#pragma once

#include <map>
#include <string>

#include "config.hpp"
#include "idle.hpp"

enum SubscriptionType
{
    SUBSCRIPTION_TYPE_IDLE,
    SUBSCRIPTION_TYPE_MONITOR,
    SUBSCRIPTION_TYPE_WAYBAR,
};

struct AppActivity
{
    bool input = false;
    bool output = false;
};

struct Data
{
    bool activeOutput = false;
    bool activeInput = false;
    std::map<std::string, AppActivity> activeApps;

    SubscriptionType subscriptionType;
    IgnoreConfig ignoreConfig;

    Idle *idle = nullptr;

    Data(SubscriptionType subscriptionType);

    void handleAction();

  private:
    void printTable();
    void printWayBar();
    int prev_lines = 0;
    void clearPrevOutput();
};
