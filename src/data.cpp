#include <iomanip>
#include <iostream>
#include <string>

#include "data.hpp"

Data::Data(SubscriptionType subscriptionType)
	: subscriptionType(subscriptionType), ignoreConfig(load_ignore_config())
{
	if (subscriptionType == SUBSCRIPTION_TYPE_IDLE)
		idle = new Idle(ignoreConfig.inhibition_type);
}

void Data::clearPrevOutput()
{
	if (prev_lines > 0) {
		for (int i = 0; i < prev_lines; i++)
			std::cout << "\033[A\033[2K";
		std::cout << "\r";
	}
}

void Data::handleAction()
{
	switch (subscriptionType) {
	case SUBSCRIPTION_TYPE_IDLE:
		idle->update(activeInput || activeOutput);
		std::cout << (activeInput || activeOutput ? "IDLE INHIBITED" : "NOT IDLE INHIBITED")
				  << std::endl;
		break;
	case SUBSCRIPTION_TYPE_MONITOR:
		printTable();
		break;
	case SUBSCRIPTION_TYPE_WAYBAR:
		printWayBar();
		break;
	}
}

void Data::printTable()
{
	clearPrevOutput();

	if (!activeInput && !activeOutput) {
		std::cout << "NOT RUNNING" << std::endl;
		prev_lines = 1;
		return;
	}

	size_t max_len = 7; // minimum: "AppName"
	for (const auto& pair : activeApps) {
		if (pair.first.size() > max_len)
			max_len = pair.first.size();
	}

	std::cout << std::left << std::setw(max_len) << "App"
			  << " | Input | Output" << std::endl;
	std::cout << std::string(max_len, '-') << "-|-------|---------" << std::endl;
	for (const auto& pair : activeApps) {
		std::cout << std::left << std::setw(max_len) << pair.first << " | "
				  << (pair.second.input ? "*    " : "     ") << " | "
				  << (pair.second.output ? "*" : " ") << std::endl;
	}
	prev_lines = 2 + static_cast<int>(activeApps.size());
}

void Data::printWayBar()
{
	std::string result[2] = {activeInput ? "input" : "", activeOutput ? "output" : ""};
	std::string text;
	for (const auto& str : result) {
		if (!text.empty() && !str.empty())
			text += "-";
		text += str;
	}
	if (text.empty())
		text = "none";
	std::cout << "{\"text\": \"\", \"alt\": \"" << text
			  << "\", \"tooltip\": \"\", \"class\": \"" << text << "\"}" << std::endl;
}
