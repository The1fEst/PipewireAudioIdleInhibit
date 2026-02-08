#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "config.hpp"

// Trim whitespace from both ends
static std::string trim(const std::string& s)
{
	size_t start = s.find_first_not_of(" \t\r\n");
	if (start == std::string::npos)
		return "";
	size_t end = s.find_last_not_of(" \t\r\n");
	return s.substr(start, end - start + 1);
}

// Strip inline comments (# not inside quotes)
static std::string strip_comment(const std::string& s)
{
	size_t pos = s.find('#');
	if (pos == std::string::npos)
		return s;
	return s.substr(0, pos);
}

// Parse a JSON-like array: ["item1", "item2"]
static void parse_array(const std::string& value, std::set<std::string>& out)
{
	// Find content between [ and ]
	size_t start = value.find('[');
	size_t end = value.rfind(']');
	if (start == std::string::npos || end == std::string::npos || end <= start)
		return;

	std::string content = value.substr(start + 1, end - start - 1);
	std::istringstream ss(content);
	std::string token;
	while (std::getline(ss, token, ',')) {
		token = trim(token);
		// Remove surrounding quotes
		if (token.size() >= 2 && ((token.front() == '"' && token.back() == '"') ||
								  (token.front() == '\'' && token.back() == '\''))) {
			token = token.substr(1, token.size() - 2);
		}
		token = trim(token);
		if (!token.empty())
			out.insert(token);
	}
}

static bool parse_config_file(const std::string& path, IgnoreConfig& config)
{
	std::ifstream file(path);
	if (!file.is_open())
		return false;

	fprintf(stderr, "Loading ignore config from: %s\n", path.c_str());

	// Read entire file, stripping comments
	std::string full_content;
	std::string line;
	while (std::getline(file, line)) {
		full_content += strip_comment(line) + "\n";
	}

	// Simple key: value parser for our JSON-like format
	// Handles both arrays and string values
	size_t pos = 0;
	while (pos < full_content.size()) {
		// Find a key
		size_t colon = full_content.find(':', pos);
		if (colon == std::string::npos)
			break;

		std::string key = trim(full_content.substr(pos, colon - pos));
		// Remove leading { , } or quotes from key
		while (!key.empty() && (key.front() == '{' || key.front() == '}' || key.front() == ',' ||
								key.front() == '"' || key.front() == '\'' || key.front() == '\n'))
			key.erase(0, 1);
		while (!key.empty() &&
			   (key.back() == '"' || key.back() == '\'' || key.back() == ',' || key.back() == '}'))
			key.pop_back();
		key = trim(key);

		// Check if value is an array (starts with [)
		size_t arr_start = full_content.find('[', colon);
		size_t arr_end = full_content.find(']', arr_start != std::string::npos ? arr_start : colon);

		if (arr_start != std::string::npos && arr_end != std::string::npos &&
			arr_start < colon + 10) {
			// Array value
			std::string value = full_content.substr(arr_start, arr_end - arr_start + 1);

			if (key == "input") {
				parse_array(value, config.input);
			} else if (key == "output") {
				parse_array(value, config.output);
			} else if (key == "both") {
				parse_array(value, config.input);
				parse_array(value, config.output);
			}

			pos = arr_end + 1;
		} else {
			// String value - find until comma or closing brace
			size_t value_start = colon + 1;
			size_t value_end = full_content.find_first_of(",}", value_start);
			if (value_end == std::string::npos)
				value_end = full_content.size();

			std::string value = trim(full_content.substr(value_start, value_end - value_start));
			// Remove quotes from value
			if (value.size() >= 2 && ((value.front() == '"' && value.back() == '"') ||
									  (value.front() == '\'' && value.back() == '\''))) {
				value = value.substr(1, value.size() - 2);
			}

			if (key == "inhibition_type") {
				config.inhibition_type = value;
			}

			pos = value_end + 1;
		}
	}

	return true;
}

IgnoreConfig load_ignore_config()
{
	IgnoreConfig config;

	// Search paths in priority order
	std::vector<std::string> paths;

	// 1. $XDG_CONFIG_HOME/pipewire-audio-idle-inhibit/config.json
	const char* xdg = getenv("XDG_CONFIG_HOME");
	if (xdg && xdg[0] != '\0') {
		paths.push_back(std::string(xdg) + "/pipewire-audio-idle-inhibit/config.json");
	} else {
		const char* home = getenv("HOME");
		if (home) {
			paths.push_back(std::string(home) + "/.config/pipewire-audio-idle-inhibit/config.json");
		}
	}

	// 2. /etc/pipewire-audio-idle-inhibit/config.json
	paths.push_back("/etc/pipewire-audio-idle-inhibit/config.json");

	// 3. /usr/share/pipewire-audio-idle-inhibit/config.json
	paths.push_back("/usr/share/pipewire-audio-idle-inhibit/config.json");

	for (const auto& path : paths) {
		if (parse_config_file(path, config))
			return config;
	}

	return config;
}
