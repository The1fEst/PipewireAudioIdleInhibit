#pragma once

#include <set>
#include <string>

struct IgnoreConfig {
	std::set<std::string> input;
	std::set<std::string> output;
};

IgnoreConfig load_ignore_config();
