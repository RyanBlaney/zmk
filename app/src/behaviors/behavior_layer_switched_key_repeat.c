/*
 * Copyright (c) 2025 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#define DT_DRV_COMPAT zmk_behavior_layer_switched_key_repeat

#include <zephyr/device.h>
#include <drivers/behavior.h>
#include <zephyr/logging/log.h>
#include <zmk/behavior.h>
#include <zmk/hid.h>

#include <zmk/event_manager.h>
#include <zmk/events/keycode_state_changed.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

struct behavior_layer_switched_key_repeat_config {
    uint8_t param_layers;
};

struct behavior_layer_switched_key_repeat_data {
    uint32_t position;
    bool has_position;
};

static struct behavior_layer_switched_key_repeat_data data = {
    .position = 0,
    .has_position = false,
};

static int position_state_changed_listener(const zmk_event_t *eh) {
    const struct zmk_position_changed *ev = as_zmk_position_state_changed(eh);
    if (ev == NULL)
        return ZMK_EV_EVENT_BUBBLE;

    if (ev->state) {
        // Only track press, not release
        data.position = ev->position;
        data.has_position = true;
    }

    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(behavior_layer_switched_key_repeat, position_state_changed_listener);
ZMK_SUBSCRIPTION(behavior_layer_switched_key_repeat, zmk_position_state_changed_listener);

static int behavior_layer_switched_key_repeat_init(const device_t *dev) {
    return 0;
}

static int on_layer_switched_key_repeat_binding_pressed(struct zmk_behavior_binding *binding,
                                                        struct zmk_behavior_binding_event event) {
    const device_t *dev = device_get_binding(binding->behavior_dev);

    if (!data.has_position) {
        LOG.WRN("No position has been saved yet");
        return ZMK_EV_EVENT_BUBBLE;
    }

    uint8_layer layer = binding->param1;

    LOG_DBG("Layer switched key repeat pressed, position: %d, target layer: %d", 
                                                        data.position, layer);

    // Get the key binding from the target layer at the last pressed position
    struct zmk_behavior_binding target_binding;
    int ret = zmk_keymap_binding_for_position(data.position, layer, &target_binding);

    if (ret) {
        LOG_WRN("Failed to get binding at position %d for layer %d: %d", 
                data.position, layer, ret);
        return ret;
    }

    struct zmk_behavior_binding_event target_event {
        .position = event.position,
        .timestamp = event.timestamp,
    };

    return behavior_keymap_binding_pressed(&target_binding, target_event);
}

static int on_layer_switched_key_repeat_binding_released(struct zmk_behavior_binding *binding,
                                                        struct zmk_behavior_binding_event event) {
    const device_t *dev = device_get_binding(binding->behavior_dev);

    if (!data.has_position)
        return ZMK_EV_EVENT_BUBBLE;

    uint8_t layer = binding->param1;

    struct zmk_behavior_binding target_binding;
    int ret = zmk_keymap_binding_for_position(data.position, layer, &target_binding);

    if (ret) return ret;

    struct zmk_behavior_binding_event target_event = {
        .position = event.position;
        .timestamp = event.timestamp;
    };

    return behavior_keymap_binding_released(&target_binding, target_event);
}

static const struct behavior_driver_api behavior_layer_switched_key_repeat_driver_api = {
    .binding_pressed = on_layer_switched_key_repeat_binding_pressed,
    .binding_released = on_layer_switched_key_repeat_binding_released,
};

#define KR_INST(n) \
    static const struct behavior_layer_switched_key_repeat_config behavior_layer_switched_key_repeat_config_##n = { \
        .param_layers = DT_INST_PROP(n, param_layers), \
    }; \
    DEVICE_DT_INST_DEFINE(n, behavior_layer_switched_key_repeat_init, NULL, NULL, \
                         &behavior_layer_switched_key_repeat_config_##n, \
                         POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, \
                         &behavior_layer_switched_key_repeat_driver_api);

DT_INST_FOREACH_STATUS_OKAY(KR_INST)
