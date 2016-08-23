/*
 * Copyright Mellanox Technologies, Ltd. 2001-2016.
 * This software product is licensed under Apache version 2, as detailed in
 * the COPYING file.
 */

#include <sensors/sensors.h>
#include <sensors/error.h>
#include <stdio.h>

#include "openvswitch/vlog.h"
#include "ledd_plugin_interfaces.h"
#include "ledd.h"
#include "errno.h"
#include "assert.h"

#define CHECK_RC(rc, msg, args...)                            \
    do {                                                      \
        if (rc) {                                             \
            VLOG_ERR("%s. " msg, sensors_strerror(rc), args); \
            goto exit;                                        \
        }                                                     \
    } while (0);

VLOG_DEFINE_THIS_MODULE(ledd_sysfs);

struct sysfs_led {
    struct locl_led up;
    const sensors_chip_name *chip_name;
    const sensors_subfeature *output;
};

static inline struct sysfs_led *sysfs_led_cast(const struct locl_led *);
static struct locl_subsystem *__subsystem_alloc(void);
static int __subsystem_construct(struct locl_subsystem *subsystem);
static void __subsystem_destruct(struct locl_subsystem *subsystem);
static void __subsystem_dealloc(struct locl_subsystem *subsystem);

static struct locl_led * __led_alloc(void);
static int __led_construct(struct locl_led *led);
static void __led_destruct(struct locl_led *led);
static void __led_dealloc(struct locl_led *led);
static int __led_state_get(const struct locl_led *led_,
                           enum ovsrec_led_state *state, bool *is_good);
static int __led_state_set(const struct locl_led *led_,
                           enum ovsrec_led_state state, bool *is_good);

static char*
__led_state_enum_to_string(const struct locl_led *led_,
                           enum ovsrec_led_state state, bool *is_good);
int
__led_state_string_to_enum(const struct locl_led *led_, char *value,
                           enum ovsrec_led_state *state, bool *is_good);

static const struct ledd_subsystem_class sysfs_sybsystem_class = {
    .ledd_subsystem_alloc     = __subsystem_alloc,
    .ledd_subsystem_construct = __subsystem_construct,
    .ledd_subsystem_destruct  = __subsystem_destruct,
    .ledd_subsystem_dealloc   = __subsystem_dealloc,
};

const struct ledd_led_class sysfs_led_class = {
    .ledd_led_alloc     = __led_alloc,
    .ledd_led_construct = __led_construct,
    .ledd_led_destruct  = __led_destruct,
    .ledd_led_dealloc   = __led_dealloc,
    .ledd_led_state_get = __led_state_get,
    .ledd_led_state_set = __led_state_set,
};

static inline struct sysfs_led *
sysfs_led_cast(const struct locl_led *led_)
{
    ovs_assert(led_);

    return CONTAINER_OF(led_, struct sysfs_led, up);
}

/**
 * Get ledd subsystem class.
 */
const struct ledd_subsystem_class *ledd_subsystem_class_get(void)
{
    return &sysfs_sybsystem_class;
}

/**
 * Get ledd led class.
 */
const struct ledd_led_class *ledd_led_class_get(void)
{
    return &sysfs_led_class;
}

/**
 * Initialize ops-ledd platform support plugin.
 */
void ledd_plugin_init(void)
{
    /* We use default config file because nothing additional is needed here. */
    if (sensors_init(NULL)) {
        VLOG_ERR("Failed to initialize sensors library.");
    }
}

/**
 * Deinitialize ops-ledd platform support plugin.
 * plugin.
 */
void ledd_plugin_deinit(void)
{
    sensors_cleanup();
}

void
ledd_plugin_run(void)
{
}

void ledd_plugin_wait(void)
{
}

static struct locl_subsystem *
__subsystem_alloc(void)
{
    return xzalloc(sizeof(struct locl_subsystem));
}

static int
__subsystem_construct(struct locl_subsystem *subsystem)
{
    return 0;
}

static void
__subsystem_destruct(struct locl_subsystem *subsystem)
{
}

static void
__subsystem_dealloc(struct locl_subsystem *subsystem)
{
    free(subsystem);
}

static struct locl_led *
__led_alloc(void)
{
    struct sysfs_led *led = xzalloc(sizeof(struct sysfs_led));
    return &led->up;
}

static int
__led_construct(struct locl_led *led_)
{
    int chip_num = 0, feature_num = 0;
    const sensors_chip_name *chip = NULL;
    const sensors_feature *feature = NULL;
    struct sysfs_led *led = sysfs_led_cast(led_);
    bool flag = true;

    chip_num = 0;
    while (((true == flag) &&
            (chip = sensors_get_detected_chips(NULL, &chip_num)))) {
        feature_num = 0;
        while ((feature = sensors_get_features(chip, &feature_num))) {
            if ((feature->type == SENSORS_FEATURE_LED) &&
                (!strncmp(led_->yaml_led->dev_name, feature->name, NAME_MAX))) {
                led->chip_name = chip;
                led->output = sensors_get_subfeature(chip, feature,
                                                     SENSORS_SUBFEATURE_LED_OUTPUT);
                flag = false;
                break;
            }
        }
    }

    if (true == flag) {
        VLOG_ERR("Unable to find chip in sysfs for led %s for subsystem %s.",
                 led->up.name, led->up.subsystem->name);
        return ENODATA;
    }

    if (!led->output) {
        VLOG_ERR("%s does not have output subfeature.", led->up.name);
        return ENODATA;
    }

    return 0;
}

static void
__led_destruct(struct locl_led *led_)
{
    struct sysfs_led *led = sysfs_led_cast(led_);

    led->chip_name = NULL;
    led->output = NULL;
}

static void
__led_dealloc(struct locl_led *led_)
{
    struct sysfs_led *led = sysfs_led_cast(led_);

    free(led);
}


static int
__led_state_get(const struct locl_led *led_,
                enum ovsrec_led_state *state, bool *is_good)
{
    struct sysfs_led *led = sysfs_led_cast(led_);
    int rc = 0;
    char value[NAME_MAX] = { };

    rc = sensors_get_char_value(led->chip_name, led->output->number, value);
    CHECK_RC(rc, "Led state get %s for %s", value, led->up.name);

    rc = __led_state_string_to_enum(led_, value, state, is_good);
    CHECK_RC(rc, "Bad state get %s for %s while getting sysfs led state",
             value, led->up.name);

exit:
    return (0 == rc) ? 0 : -1;
}

static int
__led_state_set(const struct locl_led *led_,
                enum ovsrec_led_state state, bool *is_good)
{
    struct sysfs_led *led = sysfs_led_cast(led_);
    int rc = 0;
    char *value = __led_state_enum_to_string(led_, state, is_good);

    if (NULL == value) {
        VLOG_WARN("Failed to set unsupported sysfs led state %d(%d) for "
                 "subsystem %s, LED %s", (int)state,
                 (NULL == is_good ? 0 : *is_good),
                 led_->subsystem->name, led_->name);
        rc = -1;
        goto exit;
    }
    rc = sensors_set_char_value(led->chip_name, led->output->number, value);

    CHECK_RC(rc, "Led sysfs state set %s for %s",
             ((NULL == value) ? "NULL" : value), led->up.name);

exit:
    return (0 == rc) ? 0 : -1;
}

static char*
__led_state_enum_to_string(const struct locl_led *led_,
                           enum ovsrec_led_state state, bool *is_good)
{
    switch (state) {
        case LED_STATE_FLASHING:
            return ((NULL == is_good || true == *is_good) ?
                led_->type->settings.good_flashing :
                led_->type->settings.bad_flashing);
        case LED_STATE_ON:
            return ((NULL == is_good || true == *is_good) ?
                led_->type->settings.good_on :
                led_->type->settings.bad_on);
        case LED_STATE_OFF:
            return led_->type->settings.off;
        default:
            VLOG_WARN("Invalid sysfs state %d(%d) for subsystem %s, LED %s",
                      (int)state, (NULL == is_good ? 0 : *is_good),
                      led_->subsystem->name, led_->name);
            return NULL;
    }
}

int
__led_state_string_to_enum(const struct locl_led *led_, char *value,
                           enum ovsrec_led_state *state, bool *is_good)
{
    size_t len = NAME_MAX;

    if (!strncmp(value, led_->type->settings.good_on, len)) {
        *state = LED_STATE_ON;
        if (NULL == is_good) {
            *is_good = true;
        }
    } else if (!strncmp(value, led_->type->settings.good_flashing, len)) {
        *state = LED_STATE_FLASHING;
        if (NULL == is_good) {
            *is_good = true;
        }
    } else if (!strncmp(value, led_->type->settings.bad_on, len)) {
        *state = LED_STATE_ON;
        if (NULL == is_good) {
            *is_good = false;
        }
    } else if (!strncmp(value, led_->type->settings.bad_flashing, len)) {
        *state = LED_STATE_FLASHING;
        if (NULL == is_good) {
            *is_good = false;
        }
    } else if (!strncmp(value, led_->type->settings.off, len)) {
        *state = LED_STATE_OFF;
        if (NULL == is_good) {
            *is_good = false;
        }
    } else {
        VLOG_WARN("Invalid sysfs state %s for subsystem %s, LED %s",
                  value, led_->subsystem->name, led_->name);
        return -1;
    }

    return 0;
}
