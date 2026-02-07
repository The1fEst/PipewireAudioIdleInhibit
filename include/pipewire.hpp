#pragma once

#include "data.hpp"

class PipeWire {
  public:
	int init(SubscriptionType subscriptionType, char **ignoredSourceOutputs,
			 char **ignoredSinkInputs);
};
