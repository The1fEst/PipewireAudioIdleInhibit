#pragma once

#include <set>
#include <string>

struct IgnoreConfig
{
    std::set<std::string> input;
    std::set<std::string> output;
    std::string inhibition_type = "wayland"; // "wayland" or "systemd"
};

IgnoreConfig load_ignore_config();
