#pragma once

#include <string>

#ifdef HAVE_SYSTEMD
#include <systemd/sd-bus.h>
#include <systemd/sd-login.h>
#endif
#ifdef HAVE_ELOGIND
#include <elogind/sd-bus.h>
#include <elogind/sd-login.h>
#endif
#ifdef HAVE_WAYLAND
#include <wayland-client.h>
#endif

class Idle
{
	std::string inhibition_type;

	// systemd/elogind backend
	struct sd_bus* bus = nullptr;
	int fd = -1;
	bool connect_bus();
	void block_systemd();
	void release_systemd();

#ifdef HAVE_WAYLAND
	// wayland backend
	struct wl_display* display = nullptr;
	struct wl_registry* registry = nullptr;
	struct wl_compositor* compositor = nullptr;
	struct wl_surface* surface = nullptr;
	void* inhibit_manager = nullptr; // zwp_idle_inhibit_manager_v1*
	void* inhibitor = nullptr;		 // zwp_idle_inhibitor_v1*
	bool connect_wayland();
	void block_wayland();
	void release_wayland();
	void cleanup_wayland();

	friend void wayland_registry_handler(void* data, struct wl_registry* registry, uint32_t id,
										 const char* interface, uint32_t version);
	friend void wayland_registry_remover(void* data, struct wl_registry* registry, uint32_t id);
#endif

  public:
	Idle(const std::string& type);
	~Idle();

	void update(bool active);
};
