#include <cstdio>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>

#include "idle.hpp"

#ifdef HAVE_WAYLAND
#include "idle-inhibit-unstable-v1-client-protocol.h"
#endif

using namespace std;

// ─── systemd/elogind backend ─────────────────

bool Idle::connect_bus()
{
	if (bus) {
		sd_bus_unref(bus);
		bus = nullptr;
	}
	int ret = sd_bus_default_system(&bus);
	if (ret < 0) {
		fprintf(stderr, "Failed to get DBus connection: %s\n", strerror(-ret));
		bus = nullptr;
		return false;
	}
	return true;
}

void Idle::block_systemd()
{
	if (fd >= 0)
		return;

	if (!bus && !connect_bus())
		return;

	sd_bus_message* message = nullptr;
	sd_bus_error error = SD_BUS_ERROR_NULL;
	int ret =
		sd_bus_call_method(bus, "org.freedesktop.login1", "/org/freedesktop/login1",
						   "org.freedesktop.login1.Manager", "Inhibit", &error, &message, "ssss",
						   "idle", "pipewire-audio-idle-inhibit", "Audio is playing", "block");
	if (ret < 0) {
		fprintf(stderr, "Could not send inhibit signal: %s: %s\n",
				error.name ? error.name : "unknown",
				error.message ? error.message : strerror(-ret));
		sd_bus_error_free(&error);
		connect_bus();
		return;
	}

	ret = sd_bus_message_read(message, "h", &fd);
	if (ret < 0) {
		fprintf(stderr, "Could not read DBus response: %s\n", strerror(-ret));
		sd_bus_message_unref(message);
		return;
	}

	fd = fcntl(fd, F_DUPFD_CLOEXEC, 3);
	if (fd < 0)
		fprintf(stderr, "Could not copy lock fd\n");

	sd_bus_error_free(&error);
	sd_bus_message_unref(message);
}

void Idle::release_systemd()
{
	if (fd >= 0) {
		close(fd);
		fd = -1;
	}
}

// ─── wayland backend ─────────────────────────

#ifdef HAVE_WAYLAND

void wayland_registry_handler(void* data, struct wl_registry* registry, uint32_t id,
							  const char* interface, uint32_t version)
{
	Idle* self = static_cast<Idle*>(data);

	if (strcmp(interface, "wl_compositor") == 0) {
		self->compositor = static_cast<struct wl_compositor*>(
			wl_registry_bind(registry, id, &wl_compositor_interface, 1));
	} else if (strcmp(interface, "zwp_idle_inhibit_manager_v1") == 0) {
		self->inhibit_manager =
			wl_registry_bind(registry, id, &zwp_idle_inhibit_manager_v1_interface, 1);
	}
}

void wayland_registry_remover(void* data, struct wl_registry* registry, uint32_t id)
{
	// Nothing to do
}

static const struct wl_registry_listener registry_listener = {
	.global = wayland_registry_handler,
	.global_remove = wayland_registry_remover,
};

bool Idle::connect_wayland()
{
	display = wl_display_connect(nullptr);
	if (!display) {
		fprintf(stderr, "Failed to connect to Wayland display\n");
		return false;
	}

	registry = wl_display_get_registry(display);
	wl_registry_add_listener(registry, &registry_listener, this);
	wl_display_roundtrip(display);

	if (!compositor) {
		fprintf(stderr, "Wayland compositor not found\n");
		cleanup_wayland();
		return false;
	}

	if (!inhibit_manager) {
		fprintf(stderr, "Wayland idle inhibit manager not supported by compositor\n");
		cleanup_wayland();
		return false;
	}

	surface = wl_compositor_create_surface(compositor);
	if (!surface) {
		fprintf(stderr, "Failed to create Wayland surface\n");
		cleanup_wayland();
		return false;
	}

	return true;
}

void Idle::block_wayland()
{
	if (inhibitor)
		return;

	if (!display && !connect_wayland())
		return;

	struct zwp_idle_inhibit_manager_v1* manager =
		static_cast<struct zwp_idle_inhibit_manager_v1*>(inhibit_manager);
	inhibitor = zwp_idle_inhibit_manager_v1_create_inhibitor(manager, surface);

	if (!inhibitor) {
		fprintf(stderr, "Failed to create Wayland idle inhibitor\n");
		return;
	}

	wl_display_roundtrip(display);
}

void Idle::release_wayland()
{
	if (inhibitor) {
		zwp_idle_inhibitor_v1_destroy(static_cast<struct zwp_idle_inhibitor_v1*>(inhibitor));
		inhibitor = nullptr;
		if (display)
			wl_display_roundtrip(display);
	}
}

void Idle::cleanup_wayland()
{
	if (inhibitor) {
		zwp_idle_inhibitor_v1_destroy(static_cast<struct zwp_idle_inhibitor_v1*>(inhibitor));
		inhibitor = nullptr;
	}
	if (surface) {
		wl_surface_destroy(surface);
		surface = nullptr;
	}
	if (inhibit_manager) {
		zwp_idle_inhibit_manager_v1_destroy(
			static_cast<struct zwp_idle_inhibit_manager_v1*>(inhibit_manager));
		inhibit_manager = nullptr;
	}
	if (compositor) {
		wl_compositor_destroy(compositor);
		compositor = nullptr;
	}
	if (registry) {
		wl_registry_destroy(registry);
		registry = nullptr;
	}
	if (display) {
		wl_display_disconnect(display);
		display = nullptr;
	}
}

#endif // HAVE_WAYLAND

// ─── public interface ────────────────────────

Idle::Idle(const std::string& type) : inhibition_type(type)
{
#ifdef HAVE_WAYLAND
	if (inhibition_type == "wayland") {
		if (!connect_wayland()) {
			fprintf(stderr, "Wayland idle inhibit not available, falling back to systemd\n");
			inhibition_type = "systemd";
			if (!connect_bus())
				fprintf(stderr, "Warning: DBus not available yet, will retry\n");
		}
	} else
#endif
	{
		if (!connect_bus())
			fprintf(stderr, "Warning: DBus not available yet, will retry\n");
	}
}

Idle::~Idle()
{
	release_systemd();
#ifdef HAVE_WAYLAND
	cleanup_wayland();
#endif
	if (bus) {
		sd_bus_unref(bus);
		bus = nullptr;
	}
}

void Idle::update(bool active)
{
	if (active) {
#ifdef HAVE_WAYLAND
		if (inhibition_type == "wayland")
			block_wayland();
		else
#endif
			block_systemd();
		cout << "IDLE INHIBITED" << endl;
	} else {
#ifdef HAVE_WAYLAND
		if (inhibition_type == "wayland")
			release_wayland();
		else
#endif
			release_systemd();
		cout << "NOT IDLE INHIBITED" << endl;
	}
}
