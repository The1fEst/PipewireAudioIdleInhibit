#pragma once

#include <string>
#include <vector>

#include "idle.hpp"

using namespace std;

#define MAX_IGNORED_SOURCE_OUTPUTS 100
#define MAX_IGNORED_SINK_INPUTS 100

enum SubscriptionType {
	SUBSCRIPTION_TYPE_IDLE,
	SUBSCRIPTION_TYPE_DRY_BOTH,
	SUBSCRIPTION_TYPE_DRY_BOTH_WAYBAR,
	SUBSCRIPTION_TYPE_DRY_INPUT,
	SUBSCRIPTION_TYPE_DRY_OUTPUT,
};

struct Data {
	bool activeSource = false;
	bool activeSink = false;
	vector<string> activeApps;

	SubscriptionType subscriptionType;

	char **ignoredSourceOutputs;
	char **ignoredSinkInputs;

	Idle *idle = NULL;

	Data(SubscriptionType subscriptionType, char **ignoredSourceOutputs,
		 char **ignoredSinkInputs);

	void handleAction();

  private:
	void print(bool isRunning);
	void printWayBar(bool activeSink, bool activeSource);
};
