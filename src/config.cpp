#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

#include <jv.h>

#include "config.hpp"

static void read_string_array(jv arr, std::set<std::string> &out)
{
    if (jv_get_kind(arr) != JV_KIND_ARRAY)
    {
        jv_free(arr);
        return;
    }
    int len = jv_array_length(jv_copy(arr));
    for (int i = 0; i < len; i++)
    {
        jv item = jv_array_get(jv_copy(arr), i);
        if (jv_get_kind(item) == JV_KIND_STRING)
        {
            out.insert(jv_string_value(item));
        }
        jv_free(item);
    }
    jv_free(arr);
}

static bool parse_config_file(const std::string &path, IgnoreConfig &config)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        return false;
    }

    fprintf(stderr, "Loading config from: %s\n", path.c_str());

    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    jv parsed = jv_parse(content.c_str());
    if (!jv_is_valid(parsed))
    {
        jv msg = jv_invalid_get_msg(parsed);
        fprintf(stderr, "Failed to parse config %s: %s\n", path.c_str(), jv_string_value(msg));
        jv_free(msg);
        return false;
    }

    if (jv_get_kind(parsed) != JV_KIND_OBJECT)
    {
        fprintf(stderr, "Config %s: expected JSON object at top level\n", path.c_str());
        jv_free(parsed);
        return false;
    }

    jv val;

    val = jv_object_get(jv_copy(parsed), jv_string("input"));
    if (jv_is_valid(val))
    {
        read_string_array(val, config.input);
    }
    else
    {
        jv_free(val);
    }

    val = jv_object_get(jv_copy(parsed), jv_string("output"));
    if (jv_is_valid(val))
    {
        read_string_array(val, config.output);
    }
    else
    {
        jv_free(val);
    }

    val = jv_object_get(jv_copy(parsed), jv_string("both"));
    if (jv_is_valid(val))
    {
        read_string_array(jv_copy(val), config.input);
        read_string_array(val, config.output);
    }
    else
    {
        jv_free(val);
    }

    val = jv_object_get(jv_copy(parsed), jv_string("inhibition_type"));
    if (jv_is_valid(val) && jv_get_kind(val) == JV_KIND_STRING)
    {
        config.inhibition_type = jv_string_value(val);
    }
    jv_free(val);

    jv_free(parsed);
    return true;
}

IgnoreConfig load_ignore_config()
{
    IgnoreConfig config;

    // Search paths in priority order
    std::vector<std::string> paths;

    // 1. $XDG_CONFIG_HOME/pipewire-audio-idle-inhibit/config.json
    const char *xdg = getenv("XDG_CONFIG_HOME");
    if (xdg && xdg[0] != '\0')
    {
        paths.push_back(std::string(xdg) + "/pipewire-audio-idle-inhibit/config.json");
    }
    else
    {
        const char *home = getenv("HOME");
        if (home)
        {
            paths.push_back(std::string(home) + "/.config/pipewire-audio-idle-inhibit/config.json");
        }
    }

    // 2. /etc/pipewire-audio-idle-inhibit/config.json
    paths.push_back("/etc/pipewire-audio-idle-inhibit/config.json");

    // 3. /usr/share/pipewire-audio-idle-inhibit/config.json
    paths.push_back("/usr/share/pipewire-audio-idle-inhibit/config.json");

    for (const auto &path : paths)
    {
        if (parse_config_file(path, config))
        {
            return config;
        }
    }

    return config;
}
