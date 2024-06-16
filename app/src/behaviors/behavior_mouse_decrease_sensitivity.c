
#define DT_DRV_COMPAT zmk_behavior_mouse_decrease_sensitivity

#include <zephyr/device.h>
#include <drivers/behavior.h>
#include <zephyr/logging/log.h>
#include <zmk/behavior.h>
#include <zmk/mouse/hid.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

static int behavior_mouse_decrease_sensitivity_init(const struct device *dev) { return 0; }

static int on_keymap_binding_pressed(struct zmk_behavior_binding *binding,
                                     struct zmk_behavior_binding_event event) {
    zmk_mouse_decrease_sensitivity();
    return 0;
}

static int on_keymap_binding_released(struct zmk_behavior_binding *binding,
                                      struct zmk_behavior_binding_event event) {
    return 0;
}

static const struct behavior_driver_api behavior_mouse_decrease_sensitivity_driver_api = {
    .binding_pressed = on_keymap_binding_pressed,
    .binding_released = on_keymap_binding_released,
};

#define MKP_INST(n)                                                                                \
    BEHAVIOR_DT_INST_DEFINE(n, behavior_mouse_decrease_sensitivity_init, NULL, NULL, NULL,         \
                            POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,                      \
                            &behavior_mouse_decrease_sensitivity_driver_api);

DT_INST_FOREACH_STATUS_OKAY(MKP_INST)

#endif
