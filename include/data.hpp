#pragma once

#include <map>
#include <string>
#include <vector>

#include "config.hpp"
#include "idle.hpp"

using namespace std;

enum SubscriptionType {
	SUBSCRIPTION_TYPE_IDLE,
	SUBSCRIPTION_TYPE_DRY_BOTH,
	SUBSCRIPTION_TYPE_DRY_BOTH_WAYBAR,
	SUBSCRIPTION_TYPE_DRY_INPUT,
	SUBSCRIPTION_TYPE_DRY_OUTPUT,
};

struct AppActivity {
	bool input = false;
	bool output = false;
};

struct Data {
	bool activeSource = false;
	bool activeSink = false;
	map<string, AppActivity> activeApps;

	SubscriptionType subscriptionType;
	IgnoreConfig ignoreConfig;

	Idle *idle = NULL;

	Data(SubscriptionType subscriptionType);

	void handleAction();

  private:
	void print(bool isRunning);
	void printWayBar(bool activeSink, bool activeSource);
};
