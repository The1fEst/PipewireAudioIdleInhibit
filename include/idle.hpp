#pragma once

#include <map>
#include <string>
#include <vector>

#ifdef HAVE_SYSTEMD
#include <systemd/sd-bus.h>
#include <systemd/sd-login.h>
#endif
#ifdef HAVE_ELOGIND
#include <elogind/sd-bus.h>
#include <elogind/sd-login.h>
#endif

struct AppActivity;

class Idle {
	struct sd_bus *bus = nullptr;
	int fd = -1;
	int prev_lines = 0;

	void block();
	void release_block();
	void clear_prev_output();

  public:
	Idle();

	void update(bool activeSink, bool activeSource,
			const std::map<std::string, AppActivity> &activeApps);
};
