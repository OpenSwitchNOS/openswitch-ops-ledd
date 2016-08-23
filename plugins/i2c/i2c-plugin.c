/*
 * Copyright Mellanox Technologies, Ltd. 2001-2016.
 * This software product is licensed under Apache version 2, as detailed in
 * the COPYING file.
 */

#include <stdio.h>

#include "openvswitch/vlog.h"
#include "ledd_plugin_interfaces.h"
#include "ledd.h"
#include "errno.h"

#define CHECK_RC(rc, msg, args...)                            \
    do {                                                      \
        if (rc) {                                             \
            VLOG_ERR(msg, args);                              \
            goto exit;                                        \
        }                                                     \
    } while (0);

VLOG_DEFINE_THIS_MODULE(ledd_i2c);

struct i2c_led {
    struct locl_led up;
};

static inline struct i2c_led *i2c_led_cast(const struct locl_led *);
static struct ledd_locl_subsystem *__subsystem_alloc(void);
static int __subsystem_construct(struct ledd_locl_subsystem *subsystem);
static void __subsystem_destruct(struct ledd_locl_subsystem *subsystem);
static void __subsystem_dealloc(struct ledd_locl_subsystem *subsystem);

static struct locl_led * __led_alloc(void);
static int __led_construct(struct locl_led *led);
static void __led_destruct(struct locl_led *led);
static void __led_dealloc(struct locl_led *led);
static int __led_state_set(const struct locl_led *led_,
                           enum ovsrec_led_state state);

int
__led_state_enum_to_int(const struct locl_led *led_,
                        enum ovsrec_led_state state);

static const struct ledd_subsystem_class i2c_sybsystem_class = {
    .ledd_subsystem_alloc     = __subsystem_alloc,
    .ledd_subsystem_construct = __subsystem_construct,
    .ledd_subsystem_destruct  = __subsystem_destruct,
    .ledd_subsystem_dealloc   = __subsystem_dealloc,
};

const struct ledd_led_class i2c_led_class = {
    .ledd_led_alloc     = __led_alloc,
    .ledd_led_construct = __led_construct,
    .ledd_led_destruct  = __led_destruct,
    .ledd_led_dealloc   = __led_dealloc,
    .ledd_led_state_get = NULL,
    .ledd_led_state_set = __led_state_set,
};

static inline struct i2c_led *
i2c_led_cast(const struct locl_led *led_)
{
    ovs_assert(led_);

    return CONTAINER_OF(led_, struct i2c_led, up);
}

/**
 * Get ledd subsystem class.
 */
const struct ledd_subsystem_class *ledd_subsystem_class_get(void)
{
    return &i2c_sybsystem_class;
}

/**
 * Get ledd led class.
 */
const struct ledd_led_class *ledd_led_class_get(void)
{
    return &i2c_led_class;
}

/**
 * Initialize ops-ledd platform support plugin.
 */
void ledd_plugin_init(void)
{
}

/**
 * Deinitialize ops-ledd platform support plugin.
 * plugin.
 */
void ledd_plugin_deinit(void)
{
}

void
ledd_plugin_run(void)
{
}

void ledd_plugin_wait(void)
{
}

static struct ledd_locl_subsystem *
__subsystem_alloc(void)
{
    return xzalloc(sizeof(struct ledd_locl_subsystem));
}

static int
__subsystem_construct(struct ledd_locl_subsystem *subsystem)
{
    return 0;
}

static void
__subsystem_destruct(struct ledd_locl_subsystem *subsystem)
{
}

static void
__subsystem_dealloc(struct ledd_locl_subsystem *subsystem)
{
    free(subsystem);
}

static struct locl_led *
__led_alloc(void)
{
    struct i2c_led *led = xzalloc(sizeof(struct i2c_led));
    return &led->up;
}

static int
__led_construct(struct locl_led *led_)
{
    return 0;
}

static void
__led_destruct(struct locl_led *led_)
{
}

static void
__led_dealloc(struct locl_led *led_)
{
    struct i2c_led *led = i2c_led_cast(led_);

    free(led);
}

static int
__led_state_set(const struct locl_led *led_,
                enum ovsrec_led_state state)
{
    int rc = 0;
    i2c_bit_op *reg_op = led_->yaml_led->led_access;
    int int_val = __led_state_enum_to_int(led_, state);

    if (-1 == int_val) {
        rc = -1;
        VLOG_ERR("Unable to convert i2c led state %d for %s",
                 state, led_->name);
        goto exit;
    }
    rc = i2c_reg_write(led_->subsystem->yaml_handle, led_->subsystem->name,
                       reg_op, int_val);
    CHECK_RC(rc, "Led state set %d for %s via i2c", int_val, led_->name);

exit:
    return rc;
}

int
__led_state_enum_to_int(const struct locl_led *led_,
                        enum ovsrec_led_state state)
{
    const char *value = NULL;

    switch (state) {
        case LED_STATE_FLASHING:
            value = led_->type->settings.flashing;
            break;
        case LED_STATE_ON:
            value = led_->type->settings.on;
            break;
        case LED_STATE_OFF:
            value = led_->type->settings.off;
            break;
        default:
            VLOG_WARN("Invalid state %d for subsystem %s, LED %s",
                      (int)state, led_->subsystem->name, led_->name);
            return -1;
    }

    /* If some state was not specified in *.yaml config file,
       means it is not supported than its value will be NULL.
       If so, need to log that */
    if (NULL == value) {
        VLOG_WARN("Failed to set unsupported i2c led state %d for "
                  "subsystem %s, LED %s",
                  (int) state, led_->subsystem->name, led_->name);
        return -1;
    }

    return strtol(value, 0, 0);
}
