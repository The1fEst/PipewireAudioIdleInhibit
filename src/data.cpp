#include <iomanip>
#include <iostream>
#include <string>

#include "data.hpp"

using namespace std;

Data::Data(SubscriptionType subscriptionType)
	: subscriptionType(subscriptionType), ignoreConfig(load_ignore_config()) {
	if (subscriptionType == SUBSCRIPTION_TYPE_IDLE)
		idle = new Idle();
}

void Data::clearPrevOutput() {
	if (prev_lines > 0) {
		for (int i = 0; i < prev_lines; i++)
			cout << "\033[A\033[2K";
		cout << "\r";
	}
}

void Data::handleAction() {
	switch (subscriptionType) {
	case SUBSCRIPTION_TYPE_IDLE:
		idle->update(activeSink || activeSource);
		break;
	case SUBSCRIPTION_TYPE_MONITOR:
		printTable();
		break;
	case SUBSCRIPTION_TYPE_WAYBAR:
		printWayBar();
		break;
	}
}

void Data::printTable() {
	clearPrevOutput();

	if (!activeSink && !activeSource) {
		cout << "NOT RUNNING" << endl;
		prev_lines = 1;
		return;
	}

	size_t max_len = 7; // minimum: "AppName"
	for (const auto &pair : activeApps) {
		if (pair.first.size() > max_len)
			max_len = pair.first.size();
	}

	cout << left << setw(max_len) << "AppName"
		 << " | Input | Output" << endl;
	cout << string(max_len, '-')
		 << "-|-------|---------" << endl;
	for (const auto &pair : activeApps) {
		cout << left << setw(max_len) << pair.first
			 << " | " << (pair.second.input ? "*    " : "     ")
			 << " | " << (pair.second.output ? "*" : " ") << endl;
	}
	prev_lines = 2 + static_cast<int>(activeApps.size());
}

void Data::printWayBar() {
	string result[2] = {activeSink ? "output" : "",
						activeSource ? "input" : ""};
	string text;
	for (const auto &str : result) {
		if (!text.empty() && !str.empty())
			text += "-";
		text += str;
	}
	if (text.empty())
		text = "none";
	cout << "{\"text\": \"\", \"alt\": \"" << text
		 << "\", \"tooltip\": \"\", \"class\": \""
		 << text << "\"}" << endl;
}
