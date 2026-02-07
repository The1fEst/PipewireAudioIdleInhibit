#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <string>

#include <pipewire/pipewire.h>
#include <spa/utils/result.h>

#include "data.hpp"
#include "pipewire.hpp"

struct PwContext;

enum class NodeType { Output, Input, Jack };

struct NodeData {
	uint32_t id;
	NodeType type;
	std::string app_name;
	enum pw_node_state state = PW_NODE_STATE_CREATING;
	struct pw_proxy *proxy = nullptr;
	struct spa_hook listener = {};
	PwContext *ctx = nullptr;
};

struct PwContext {
	struct pw_main_loop *loop = nullptr;
	struct pw_context *context = nullptr;
	struct pw_core *core = nullptr;
	struct pw_registry *registry = nullptr;
	struct spa_hook core_listener = {};
	struct spa_hook registry_listener = {};

	std::map<uint32_t, NodeData *> nodes;
	Data *data = nullptr;
	int pending_sync = -1;
	bool initial_sync_done = false;
};

static bool is_ignored(const std::string &name, char **list, int max) {
	if (name.empty() || !list)
		return false;
	for (int i = 0; i < max; i++) {
		if (!list[i])
			break;
		if (name == list[i])
			return true;
	}
	return false;
}

static void evaluate_streams(PwContext *ctx) {
	bool sink_active = false;
	bool source_active = false;
	std::vector<std::string> active_apps;

	for (const auto &pair : ctx->nodes) {
		NodeData *node = pair.second;
		if (node->state != PW_NODE_STATE_RUNNING)
			continue;

		const auto &name = node->app_name;
		bool ignored_out = is_ignored(name, ctx->data->ignoredSourceOutputs,
									  MAX_IGNORED_SOURCE_OUTPUTS);
		bool ignored_in = is_ignored(name, ctx->data->ignoredSinkInputs,
									 MAX_IGNORED_SINK_INPUTS);

		switch (node->type) {
		case NodeType::Jack:
			if (!ignored_out && !ignored_in) {
				sink_active = true;
				source_active = true;
				if (!name.empty()) {
					active_apps.push_back(name + " (input)");
					active_apps.push_back(name + " (output)");
				}
			}
			break;
		case NodeType::Output:
			if (!ignored_out) {
				source_active = true;
				if (!name.empty())
					active_apps.push_back(name + " (output)");
			}
			break;
		case NodeType::Input:
			if (!ignored_in) {
				sink_active = true;
				if (!name.empty())
					active_apps.push_back(name + " (input)");
			}
			break;
		}
	}

	ctx->data->activeSink = sink_active;
	ctx->data->activeSource = source_active;
	ctx->data->activeApps = active_apps;

	if (ctx->initial_sync_done)
		ctx->data->handleAction();
}

static void node_info(void *object, const struct pw_node_info *info) {
	NodeData *node = static_cast<NodeData *>(object);
	node->state = info->state;
	evaluate_streams(node->ctx);
}

static const struct pw_node_events node_events = {
	.version = PW_VERSION_NODE_EVENTS,
	.info = node_info,
};

static void registry_global(void *data, uint32_t id, uint32_t permissions,
							const char *type, uint32_t version,
							const struct spa_dict *props) {
	PwContext *ctx = static_cast<PwContext *>(data);

	if (!type || strcmp(type, PW_TYPE_INTERFACE_Node) != 0)
		return;
	if (!props)
		return;

	const char *media_class = spa_dict_lookup(props, PW_KEY_MEDIA_CLASS);
	const char *client_api = spa_dict_lookup(props, PW_KEY_CLIENT_API);

	bool is_output = media_class && strcmp(media_class, "Stream/Output/Audio") == 0;
	bool is_input = media_class && strcmp(media_class, "Stream/Input/Audio") == 0;
	bool is_jack = client_api && strcmp(client_api, "jack") == 0;

	if (!is_output && !is_input && !is_jack)
		return;

	// Filter based on subscription type (JACK clients pass all filters
	// since they are typically bidirectional)
	if (!is_jack) {
		SubscriptionType st = ctx->data->subscriptionType;
		if (st == SUBSCRIPTION_TYPE_DRY_INPUT && !is_output)
			return;
		if (st == SUBSCRIPTION_TYPE_DRY_OUTPUT && !is_input)
			return;
	}

NodeType node_type = is_jack ? NodeType::Jack
					 : is_input ? NodeType::Input
								   : NodeType::Output;

	NodeData *node = new NodeData();
	node->id = id;
	node->type = node_type;
	node->ctx = ctx;

	const char *app_name = spa_dict_lookup(props, PW_KEY_APP_NAME);
	if (!app_name)
		app_name = spa_dict_lookup(props, PW_KEY_NODE_NAME);
	if (app_name)
		node->app_name = app_name;

	node->proxy = static_cast<struct pw_proxy *>(
		pw_registry_bind(ctx->registry, id, type, PW_VERSION_NODE, 0));
	if (!node->proxy) {
		delete node;
		return;
	}

	pw_proxy_add_object_listener(node->proxy, &node->listener, &node_events,
								 node);
	ctx->nodes[id] = node;
}

static void registry_global_remove(void *data, uint32_t id) {
	PwContext *ctx = static_cast<PwContext *>(data);

	auto it = ctx->nodes.find(id);
	if (it != ctx->nodes.end()) {
		NodeData *node = it->second;
		spa_hook_remove(&node->listener);
		pw_proxy_destroy(node->proxy);
		delete node;
		ctx->nodes.erase(it);
		evaluate_streams(ctx);
	}
}

static const struct pw_registry_events registry_events = {
	.version = PW_VERSION_REGISTRY_EVENTS,
	.global = registry_global,
	.global_remove = registry_global_remove,
};

static void on_core_done(void *data, uint32_t id, int seq) {
	PwContext *ctx = static_cast<PwContext *>(data);
	if (id == PW_ID_CORE && seq == ctx->pending_sync) {
		ctx->pending_sync = -1;
		ctx->initial_sync_done = true;
		evaluate_streams(ctx);
	}
}

static void on_core_error(void *data, uint32_t id, int seq, int res,
						  const char *message) {
	PwContext *ctx = static_cast<PwContext *>(data);
	fprintf(stderr, "PipeWire error: id=%u seq=%d res=%d (%s): %s\n", id, seq,
			res, spa_strerror(res), message);

	if (id == PW_ID_CORE && res == -EPIPE) {
		fprintf(stderr, "PipeWire connection lost\n");
		pw_main_loop_quit(ctx->loop);
	}
}

static const struct pw_core_events core_events = {
	.version = PW_VERSION_CORE_EVENTS,
	.done = on_core_done,
	.error = on_core_error,
};

int PipeWire::init(SubscriptionType subscriptionType,
				   char **ignoredSourceOutputs,
				   char **ignoredSinkInputs) {
	pw_init(nullptr, nullptr);

	PwContext *ctx = new PwContext();
	ctx->data = new Data(subscriptionType, ignoredSourceOutputs,
						 ignoredSinkInputs);

	ctx->loop = pw_main_loop_new(nullptr);
	if (!ctx->loop) {
		fprintf(stderr, "Failed to create PipeWire main loop\n");
		return 1;
	}

	ctx->context =
		pw_context_new(pw_main_loop_get_loop(ctx->loop), nullptr, 0);
	if (!ctx->context) {
		fprintf(stderr, "Failed to create PipeWire context\n");
		return 1;
	}

	ctx->core = pw_context_connect(ctx->context, nullptr, 0);
	if (!ctx->core) {
		fprintf(stderr, "Failed to connect to PipeWire\n");
		return 1;
	}

	pw_core_add_listener(ctx->core, &ctx->core_listener, &core_events, ctx);

	ctx->registry = pw_core_get_registry(ctx->core, PW_VERSION_REGISTRY, 0);
	if (!ctx->registry) {
		fprintf(stderr, "Failed to get PipeWire registry\n");
		return 1;
	}

	pw_registry_add_listener(ctx->registry, &ctx->registry_listener,
							 &registry_events, ctx);

	// Trigger initial evaluation after all existing nodes are enumerated
	ctx->pending_sync = pw_core_sync(ctx->core, PW_ID_CORE, 0);

	// Run the main loop (blocks forever)
	pw_main_loop_run(ctx->loop);

	// Cleanup (only reached on error/quit)
	pw_proxy_destroy((struct pw_proxy *)ctx->registry);
	pw_core_disconnect(ctx->core);
	pw_context_destroy(ctx->context);
	pw_main_loop_destroy(ctx->loop);

	delete ctx->data;
	delete ctx;

	pw_deinit();
	return 0;
}
