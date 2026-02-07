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
	cout << "\t --o \t\t\t\t Don't inhibit idle for these "
			"source outputs\n";
	cout << "\t --i \t\t\t\t Don't inhibit idle for these "
			"sink inputs\n";
	cout << "\t --b \t\t\t\t Don't inhibit idle for these "
			"source outputs and sink inputs\n";
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

static bool isInList(char **list, int count, const char *name) {
	for (int j = 0; j < count; j++) {
		if (strcmp(list[j], name) == 0)
			return true;
	}
	return false;
}

static void parseIgnoreList(char *arg, char **list, int &count, int max) {
	char *saveptr;
	char *token = strtok_r(arg, " ", &saveptr);
	while (token != nullptr && count < max) {
		if (!isInList(list, count, token))
			list[count++] = token;
		token = strtok_r(nullptr, " ", &saveptr);
	}
	list[count] = nullptr;
}

static void parseIgnoreBoth(char *arg, char **outList, int &outCount,
							int outMax, char **inList, int &inCount,
							int inMax) {
	char *saveptr;
	char *token = strtok_r(arg, " ", &saveptr);
	while (token != nullptr) {
		if (outCount < outMax && !isInList(outList, outCount, token))
			outList[outCount++] = token;
		if (inCount < inMax && !isInList(inList, inCount, token))
			inList[inCount++] = token;
		token = strtok_r(nullptr, " ", &saveptr);
	}
	outList[outCount] = nullptr;
	inList[inCount] = nullptr;
}

int main(int argc, char *argv[]) {
	SubscriptionType subType = SUBSCRIPTION_TYPE_IDLE;

	char *ignoredSourceOutputs[MAX_IGNORED_SOURCE_OUTPUTS] = {nullptr};
	int ignoredSourceOutputsCount = 0;
	char *ignoredSinkInputs[MAX_IGNORED_SINK_INPUTS] = {nullptr};
	int ignoredSinkInputsCount = 0;

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
		} else if (strcmp(argv[i], "--o") == 0 && i + 1 < argc) {
			parseIgnoreList(argv[++i], ignoredSourceOutputs,
							ignoredSourceOutputsCount,
							MAX_IGNORED_SOURCE_OUTPUTS);
		} else if (strcmp(argv[i], "--i") == 0 && i + 1 < argc) {
			parseIgnoreList(argv[++i], ignoredSinkInputs,
							ignoredSinkInputsCount, MAX_IGNORED_SINK_INPUTS);
		} else if (strcmp(argv[i], "--b") == 0 && i + 1 < argc) {
			parseIgnoreBoth(argv[++i], ignoredSourceOutputs,
							ignoredSourceOutputsCount,
							MAX_IGNORED_SOURCE_OUTPUTS, ignoredSinkInputs,
							ignoredSinkInputsCount, MAX_IGNORED_SINK_INPUTS);
		}
	}

	if (subType == SUBSCRIPTION_TYPE_IDLE && isAlreadyRunning())
		return EXIT_FAILURE;

	return PipeWire().init(subType, ignoredSourceOutputs, ignoredSinkInputs);
}
