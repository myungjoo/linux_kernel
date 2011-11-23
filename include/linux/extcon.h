/*
 *  External connector (extcon) class driver
 *
 * Copyright (C) 2012 Samsung Electronics
 * Author: Donggeun Kim <dg77.kim@samsung.com>
 * Author: MyungJoo Ham <myungjoo.ham@samsung.com>
 *
 * based on switch class driver
 * Copyright (C) 2008 Google, Inc.
 * Author: Mike Lockwood <lockwood@android.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
*/

#ifndef __LINUX_EXTCON_H__
#define __LINUX_EXTCON_H__

/**
 * struct extcon_dev - An extcon device represents one external connector.
 * @name	The name of this extcon device. Parent device name is used
 *		if NULL.
 * @use_class_name_switch	set true in order to be compatible with
 *				Android platform, which uses "switch"
 *				for the class name.
 * @print_name	An optional callback to override the method to print the
 *		name of the extcon device.
 * @print_state	An optional callback to override the method to print the
 *		status of the extcon device.
 * @dev		Device of this extcon. Do not provide at register-time.
 * @state	Attach/detach state of this extcon. Do not provide at
 *		register-time
 *
 * In most cases, users only need to provide "User initializing data" of
 * this struct when registering an extcon. In some exceptional cases,
 * optional callbacks may be needed. However, the values in "internal data"
 * are overwritten by register function.
 */
struct extcon_dev {
	/* --- Optional user initializing data --- */
	const char	*name;
	bool		use_class_name_switch;

	/* --- Optional callbacks to override class functions --- */
	ssize_t	(*print_name)(struct extcon_dev *edev, char *buf);
	ssize_t	(*print_state)(struct extcon_dev *edev, char *buf);

	/* --- Internal data. Please do not set. --- */
	struct device	*dev;
	u32		state;
};

/**
 * struct gpio_extcon_platform_data - A simple GPIO-controlled extcon device.
 * @name	The name of this GPIO extcon device.
 * @gpio	Corresponding GPIO.
 * @state_on	print_state is overriden with state_on if attached. If Null,
 *		default method of extcon class is used.
 * @state_off	print_state is overriden with state_on if dettached. If Null,
 *		default method of extcon class is used.
 */
struct gpio_extcon_platform_data {
	const char *name;
	unsigned gpio;

	/* if NULL, "0" or "1" will be printed */
	const char *state_on;
	const char *state_off;
};

#ifdef CONFIG_EXTCON
extern int extcon_dev_register(struct extcon_dev *edev, struct device *dev);
extern void extcon_dev_unregister(struct extcon_dev *edev);

static inline u32 extcon_get_state(struct extcon_dev *edev)
{
	return edev->state;
}

extern void extcon_set_state(struct extcon_dev *edev, u32 state);
#else /* CONFIG_EXTCON */
static inline int extcon_dev_register(struct extcon_dev *edev,
				      struct device *dev)
{
	return 0;
}

static inline void extcon_dev_unregister(struct extcon_dev *edev) { }

static inline u32 extcon_get_state(struct extcon_dev *edev)
{
	return 0;
}

static inline void extcon_set_state(struct extcon_dev *edev, u32 state) { }
#endif /* CONFIG_EXTCON */
#endif /* __LINUX_EXTCON_H__ */
