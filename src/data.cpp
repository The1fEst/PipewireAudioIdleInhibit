#include <iostream>
#include <string>

#include "data.hpp"

Data::Data(SubscriptionType subscriptionType, char **ignoredSourceOutputs) {
	this->subscriptionType = subscriptionType;
	this->ignoredSourceOutputs = ignoredSourceOutputs;

	if (subscriptionType == SUBSCRIPTION_TYPE_IDLE)
		idle = new Idle();
}

void Data::handleAction() {
	switch (subscriptionType) {
	case SUBSCRIPTION_TYPE_IDLE:
		idle->update(activeSink || activeSource);
		break;
	case SUBSCRIPTION_TYPE_DRY_BOTH:
		this->print(activeSink || activeSource);
		break;
	case SUBSCRIPTION_TYPE_DRY_BOTH_WAYBAR:
		this->printWayBar(activeSink, activeSource);
		break;
	case SUBSCRIPTION_TYPE_DRY_SINK:
		this->print(activeSink);
		break;
	case SUBSCRIPTION_TYPE_DRY_SOURCE:
		this->print(activeSource);
		break;
	}
}

void Data::print(bool isRunning) {
	cout << (isRunning ? "RUNNING" : "NOT RUNNING") << endl;
}

void Data::printWayBar(bool activeSink, bool activeSource) {
	string result[2] = {activeSink ? "output" : "",
						activeSource ? "input" : ""};
	string text = "";
	for (const auto &str : result) {
		if (!text.empty() && !str.empty())
			text += "-";
		text += str;
	}
	if (text.empty())
		text = "none";
	cout << "{\"text\": \"\", \"alt\": \"" + text +
				"\", \"tooltip\": \"\", \"class\": "
				"\"" +
				text + "\"}"
		 << endl;
}
