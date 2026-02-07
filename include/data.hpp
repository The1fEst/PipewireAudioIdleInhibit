#pragma once

#include "idle.hpp"

using namespace std;

#define MAX_IGNORED_SOURCE_OUTPUTS 100

enum SubscriptionType {
	SUBSCRIPTION_TYPE_IDLE,
	SUBSCRIPTION_TYPE_DRY_BOTH,
	SUBSCRIPTION_TYPE_DRY_BOTH_WAYBAR,
	SUBSCRIPTION_TYPE_DRY_SINK,
	SUBSCRIPTION_TYPE_DRY_SOURCE,
};

struct Data {
	bool activeSource = false;
	bool activeSink = false;

	SubscriptionType subscriptionType;

	char **ignoredSourceOutputs;

	Idle *idle = NULL;

	Data(SubscriptionType subscriptionType, char **ignoredSourceOutputs);

	void handleAction();

  private:
	void print(bool isRunning);
	void printWayBar(bool activeSink, bool activeSource);
};
