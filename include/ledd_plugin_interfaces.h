/*
 * Copyright Mellanox Technologies, Ltd. 2001-2016.
 * This software product is licensed under Apache version 2, as detailed in
 * the COPYING file.
 */

/************************************************************************/
/**
 * @ingroup ops-ledd
 *
 * @file
 * Header file for hardware access plugin API.
 ***************************************************************************/
#ifndef _LED_PLUGIN_INTERFACES_H_
#define _LED_PLUGIN_INTERFACES_H_

enum ovsrec_led_state;

struct ledd_subsystem_class {
    /**
     * Allocation of led subsystem on adding to ovsdb.
     * Implementation should define its own struct that contains
     * parent struct ledd_locl_subsystem, and return pointer to parent.
     *
     * @return pointer to allocated subsystem.
     */
    struct ledd_locl_subsystem *(*ledd_subsystem_alloc)(void);

    /**
     * Construction of led subsystem on adding to ovsdb. Implementation should
     * initialize all fields in derived structure from ledd_locl_subsystem.
     *
     * @param[out] subsystem - pointer to subsystem.
     *
     * @return 0     on success.
     * @return errno on failure.
     */
    int (*ledd_subsystem_construct)(struct ledd_locl_subsystem *subsystem);

    /**
     * Destruction of led subsystem on removing from ovsdb. Implementation
     * should deinitialize all fields in derived structure from ledd_locl_subsystem.
     *
     * @param[in] subsystem - pointer to subsystem.
     */
    void (*ledd_subsystem_destruct)(struct ledd_locl_subsystem *subsystem);

    /**
     * Deallocation of led subsystem on removing from ovsdb. Implementation
     * should free memory from derived structure.
     *
     * @param[in] subsystem - pointer to subsystem.
     */
    void (*ledd_subsystem_dealloc)(struct ledd_locl_subsystem *subsystem);

};

struct ledd_led_class {
    /**
     * Allocation of led. Implementation should define its own struct that
     * contains parent struct ledd_locl_subsystem, and return pointer to parent.
     *
     * @return pointer to allocated led.
     */
    struct locl_led *(*ledd_led_alloc)(void);

    /**
     * Construction of led on adding to ovsdb. Implementation should
     * initialize all fields in derived structure from locl_led.
     *
     * @param[out] led - pointer to led.
     *
     * @return 0     on success.
     * @return errno on failure.
     */
    int (*ledd_led_construct)(struct locl_led *led);

    /**
     * Destruction of led. Implementation should deinitialize all
     * fields in derived structure from locl_led.
     *
     * @param[in] led - pointer to led.
     */
     void (*ledd_led_destruct)(struct locl_led *led);

    /**
     * Deallocation of led. Implementation should free memory from derived
     * structure.
     *
     * @param[in] led - pointer to led.
     */
    void (*ledd_led_dealloc)(struct locl_led *led);

    /**
     * Get led state.
     *
     * @param[in]  led   - pointer to led.
     * @param[out] state - pointer to output value with state.
     *
     * @return 0     on success.
     * @return errno on failure.
     */
    int (*ledd_led_state_get)(const struct locl_led *led,
                              enum ovsrec_led_state *state);

    /**
     * Set led state.
     *
     * @param[in] led   - pointer to led.
     * @param[in] state - state value to set.
     *
     * @return 0     on success.
     * @return errno on failure.
     */
    int (*ledd_led_state_set)(const struct locl_led *led,
                              enum ovsrec_led_state state);
};

#endif /* _LED_PLUGIN_INTERFACES_H_ */
