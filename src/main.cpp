#include <cstring>
#include <iostream>
#include <libgen.h>
#include <string>
#include <sys/file.h>

#include "data.hpp"
#include "pipewire.hpp"

#define LOCK_FILE "/tmp/pipewire-audio-idle-inhibit.lock"

static void showHelp(char **argv) {
	string name = basename(argv[0]);
	cout << "Usage:\n";
	cout << "\t" << name << " <OPTION>\n";
	cout << "Options:\n";
	cout << "\t " << name
		 << "\t Inhibits idle if either any input or any output is running\n";
	cout << "\t -h, --help \t\t\t Show help options\n";
	cout << "\t --both \t\t\t Don't inhibit idle and print if either "
			"any input or any output is running\n";
	cout << "\t --waybar \t\t\t Same as --both but outputs "
			"in a waybar friendly manner\n";
	cout << "\t --input \t\t\t Don't inhibit idle and print if any "
			"input is running\n";
	cout << "\t --output \t\t\t Don't inhibit idle and print if any "
			"output is running\n";
	cout << "\nIgnore config file searched in order:\n";
	cout << "\t $XDG_CONFIG_HOME/pipewire-audio-idle-inhibit/ignore.conf\n";
	cout << "\t /etc/pipewire-audio-idle-inhibit/ignore.conf\n";
	cout << "\t /usr/share/pipewire-audio-idle-inhibit/ignore.conf\n";
}

static bool isAlreadyRunning() {
	FILE *fd = fopen(LOCK_FILE, "w+");
	if (!fd) {
		fprintf(stderr, "Could not open lock file: %s\n", LOCK_FILE);
		return true;
	}
	if (flock(fd->_fileno, LOCK_EX | LOCK_NB) < 0) {
		if (errno == EWOULDBLOCK) {
			fprintf(stderr, "An instance is already running\n");
		} else {
			fprintf(stderr, "Could not lock file: %s\n", LOCK_FILE);
		}
		return true;
	}
	return false;
}

int main(int argc, char *argv[]) {
	SubscriptionType subType = SUBSCRIPTION_TYPE_IDLE;

	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
			showHelp(argv);
			return EXIT_SUCCESS;
		} else if (strcmp(argv[i], "--output") == 0) {
			subType = SUBSCRIPTION_TYPE_DRY_OUTPUT;
		} else if (strcmp(argv[i], "--input") == 0) {
			subType = SUBSCRIPTION_TYPE_DRY_INPUT;
		} else if (strcmp(argv[i], "--both") == 0) {
			subType = SUBSCRIPTION_TYPE_DRY_BOTH;
		} else if (strcmp(argv[i], "--waybar") == 0) {
			subType = SUBSCRIPTION_TYPE_DRY_BOTH_WAYBAR;
		}
	}

	if (subType == SUBSCRIPTION_TYPE_IDLE && isAlreadyRunning())
		return EXIT_FAILURE;

	return PipeWire().init(subType);
}
