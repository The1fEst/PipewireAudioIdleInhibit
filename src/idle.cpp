#include <cassert>
#include <cstdio>
#include <fcntl.h>
#include <iomanip>
#include <iostream>
#include <unistd.h>

#include "data.hpp"
#include "idle.hpp"

using namespace std;

void Idle::block() {
	assert(bus);

	// Skip if already inhibiting
	if (fd >= 0) {
		return;
	}

	sd_bus_message *message = nullptr;
	sd_bus_error error = SD_BUS_ERROR_NULL;
	int ret = sd_bus_call_method(
		bus, "org.freedesktop.login1", "/org/freedesktop/login1",
		"org.freedesktop.login1.Manager", "Inhibit", &error, &message, "ssss",
		"idle", "pipewire-audio-idle-inhibit", "Audio is playing", "block");
	if (ret < 0) {
		fprintf(stderr, "Could not send inhibit signal! %s: %s\n", error.name,
				error.message);
		abort();
	}

	ret = sd_bus_message_read(message, "h", &fd);
	if (ret < 0) {
		errno = -ret;
		fprintf(stderr, "Could not get DBus response\n");
		abort();
	}

	// Clone the FD (will be invalid once we unref the message)
	fd = fcntl(fd, F_DUPFD_CLOEXEC, 3);
	if (fd >= 0) {
	} else {
		fprintf(stderr, "Could not copy lock fd\n");
	}

	sd_bus_error_free(&error);
	sd_bus_message_unref(message);
}

void Idle::release_block() {
	if (fd >= 0) {
		close(fd);
		fd = -1;
	}
}

Idle::Idle() {
	// Connect to DBus
	int ret = sd_bus_default_system(&bus);
	if (ret < 0) {
		fprintf(stderr, "Failed to get DBus connection\n");
		abort();
	}
}

void Idle::clear_prev_output() {
	if (prev_lines > 0) {
		// Move cursor up and clear each line
		for (int i = 0; i < prev_lines; i++)
			cout << "\033[A\033[2K";
		cout << "\r";
	}
}

void Idle::update(bool activeSink, bool activeSource,
				   const std::map<std::string, AppActivity> &activeApps) {
	clear_prev_output();

	if (activeSink || activeSource) {
		block();

		// Find max app name length for alignment
		size_t max_len = 7; // minimum: "AppName"
		for (const auto &pair : activeApps) {
			if (pair.first.size() > max_len)
				max_len = pair.first.size();
		}

		cout << "IDLE INHIBITED" << endl;
		cout << left << setw(max_len) << "AppName"
			 << " | Input | Output" << endl;
		cout << string(max_len, '-')
			 << "-|-------|---------" << endl;
		for (const auto &pair : activeApps) {
			cout << left << setw(max_len) << pair.first
				 << " | " << (pair.second.input ? "*    " : "     ")
				 << " | " << (pair.second.output ? "*" : " ") << endl;
		}
		// 1 (header) + 1 (column names) + 1 (separator) + rows
		prev_lines = 3 + static_cast<int>(activeApps.size());
	} else {
		release_block();
		cout << "NOT IDLE INHIBITED" << endl;
		prev_lines = 1;
	}
}
