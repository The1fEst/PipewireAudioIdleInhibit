#include <cstring>
#include <iostream>
#include <libgen.h>
#include <string>
#include <sys/file.h>

#include "data.hpp"
#include "pipewire.hpp"

#define LOCK_FILE "/tmp/pipewire-audio-idle-inhibit.lock"

static void showHelp(char** argv)
{
	std::string name = basename(argv[0]);
	std::cout << "Usage:\n";
	std::cout << "\t" << name << " <OPTION>\n";
	std::cout << "Options:\n";
	std::cout << "\t " << name << "\t Inhibits idle if either any input or any output is running\n";
	std::cout << "\t -h, --help \t\t\t Show help options\n";
	std::cout << "\t --monitor \t\t\t Don't inhibit idle, show live "
			 "audio activity table\n";
	std::cout << "\t --waybar \t\t\t Output in waybar-friendly "
			 "JSON format\n";
	std::cout << "\nConfig file searched in order:\n";
	std::cout << "\t $XDG_CONFIG_HOME/pipewire-audio-idle-inhibit/config.json\n";
	std::cout << "\t /etc/pipewire-audio-idle-inhibit/config.json\n";
	std::cout << "\t /usr/share/pipewire-audio-idle-inhibit/config.json\n";
}

static bool isAlreadyRunning()
{
	FILE* fd = fopen(LOCK_FILE, "w+");
	if (!fd) {
		fprintf(stderr, "Could not open lock file: %s\n", LOCK_FILE);
		return true;
	}
	if (flock(fileno(fd), LOCK_EX | LOCK_NB) < 0) {
		if (errno == EWOULDBLOCK) {
			fprintf(stderr, "An instance is already running\n");
		} else {
			fprintf(stderr, "Could not lock file: %s\n", LOCK_FILE);
		}
		return true;
	}
	return false;
}

int main(int argc, char* argv[])
{
	SubscriptionType subType = SUBSCRIPTION_TYPE_IDLE;

	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
			showHelp(argv);
			return EXIT_SUCCESS;
		} else if (strcmp(argv[i], "--monitor") == 0) {
			subType = SUBSCRIPTION_TYPE_MONITOR;
		} else if (strcmp(argv[i], "--waybar") == 0) {
			subType = SUBSCRIPTION_TYPE_WAYBAR;
		}
	}

	if (subType == SUBSCRIPTION_TYPE_IDLE && isAlreadyRunning())
		return EXIT_FAILURE;

	return PipeWire().init(subType);
}
