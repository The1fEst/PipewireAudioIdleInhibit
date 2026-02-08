#pragma once

#include <map>
#include <string>

#ifdef HAVE_SYSTEMD
#include <systemd/sd-bus.h>
#include <systemd/sd-login.h>
#endif
#ifdef HAVE_ELOGIND
#include <elogind/sd-bus.h>
#include <elogind/sd-login.h>
#endif

class Idle {
	struct sd_bus *bus = nullptr;
	int fd = -1;

	bool connect_bus();
	void block();
	void release_block();

  public:
	Idle();

	void update(bool active);
};
