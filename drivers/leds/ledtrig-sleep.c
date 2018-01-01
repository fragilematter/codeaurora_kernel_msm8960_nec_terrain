/* drivers/leds/ledtrig-sleep.c
 *
 * Copyright (C) 2007 Google, Inc.
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
/***********************************************************************/
/* Modified by                                                         */
/* (C) NEC CASIO Mobile Communications, Ltd. 2013                      */
/***********************************************************************/

#include <linux/earlysuspend.h>
#include <linux/leds.h>
#include <linux/suspend.h>

#if (defined(CONFIG_FEATURE_NCMC_USERDEBUG_RELEASE))
#include <linux/leds_cmd.h>
#endif /* CONFIG_FEATURE_NCMC_USERDEBUG_RELEASE */

#if (defined(CONFIG_FEATURE_NCMC_USERDEBUG_RELEASE))
static u32 registed = false;
static u32 ledtrig_sleep_state = 0;
#endif /* CONFIG_FEATURE_NCMC_USERDEBUG_RELEASE */

static int ledtrig_sleep_pm_callback(struct notifier_block *nfb,
					unsigned long action,
					void *ignored);

DEFINE_LED_TRIGGER(ledtrig_sleep)
static struct notifier_block ledtrig_sleep_pm_notifier = {
	.notifier_call = ledtrig_sleep_pm_callback,
	.priority = 0,
};

#if !(defined(CONFIG_FEATURE_NCMC_USERDEBUG_RELEASE))
static void ledtrig_sleep_early_suspend(struct early_suspend *h)
{
	led_trigger_event(ledtrig_sleep, LED_FULL);
}

static void ledtrig_sleep_early_resume(struct early_suspend *h)
{
	led_trigger_event(ledtrig_sleep, LED_OFF);
}

static struct early_suspend ledtrig_sleep_early_suspend_handler = {
	.suspend = ledtrig_sleep_early_suspend,
	.resume = ledtrig_sleep_early_resume,
};
#endif /* CONFIG_FEATURE_NCMC_USERDEBUG_RELEASE */

static int ledtrig_sleep_pm_callback(struct notifier_block *nfb,
					unsigned long action,
					void *ignored)
{
	switch (action) {
	case PM_HIBERNATION_PREPARE:
	case PM_SUSPEND_PREPARE:

#if !(defined(CONFIG_FEATURE_NCMC_USERDEBUG_RELEASE))
		led_trigger_event(ledtrig_sleep, LED_OFF);
#else /* CONFIG_FEATURE_NCMC_USERDEBUG_RELEASE */
		leds_cmd(LEDS_CMD_TYPE_RGB_RED,   0 );
		leds_cmd(LEDS_CMD_TYPE_RGB_GREEN, 0 );
		leds_cmd(LEDS_CMD_TYPE_RGB_BLUE, 0x1E );
#endif /* CONFIG_FEATURE_NCMC_USERDEBUG_RELEASE */

		return NOTIFY_OK;
	case PM_POST_HIBERNATION:
	case PM_POST_SUSPEND:

#if !(defined(CONFIG_FEATURE_NCMC_USERDEBUG_RELEASE))
		led_trigger_event(ledtrig_sleep, LED_FULL);
#endif /* CONFIG_FEATURE_NCMC_USERDEBUG_RELEASE */
		
		return NOTIFY_OK;
	}

	return NOTIFY_DONE;
}

static int __init ledtrig_sleep_init(void)
{

#if !(defined(CONFIG_FEATURE_NCMC_USERDEBUG_RELEASE))
	led_trigger_register_simple("sleep", &ledtrig_sleep);
	register_pm_notifier(&ledtrig_sleep_pm_notifier);
	register_early_suspend(&ledtrig_sleep_early_suspend_handler);
#endif /* CONFIG_FEATURE_NCMC_USERDEBUG_RELEASE */
	
	return 0;
}

static void __exit ledtrig_sleep_exit(void)
{

#if !(defined(CONFIG_FEATURE_NCMC_USERDEBUG_RELEASE))
	unregister_early_suspend(&ledtrig_sleep_early_suspend_handler);
	unregister_pm_notifier(&ledtrig_sleep_pm_notifier);
	led_trigger_unregister_simple(ledtrig_sleep);
#else /* CONFIG_FEATURE_NCMC_USERDEBUG_RELEASE */
    if(registed == true)
    {
	    unregister_pm_notifier(&ledtrig_sleep_pm_notifier);
	    led_trigger_unregister_simple(ledtrig_sleep);
    }
#endif /* CONFIG_FEATURE_NCMC_USERDEBUG_RELEASE */

}

module_init(ledtrig_sleep_init);
module_exit(ledtrig_sleep_exit);

#if (defined(CONFIG_FEATURE_NCMC_USERDEBUG_RELEASE))
static int ledtrig_sleep_state_show(char *str, struct kernel_param *kp)
{
	return snprintf(str, 2, "%d  ",ledtrig_sleep_state);
}

static int ledtrig_sleep_state_store(const char *str, struct kernel_param *kp)
{
	char* next;
	ledtrig_sleep_state = simple_strtoul(&str[0],&next,10);

	if(ledtrig_sleep_state==1)
	{
		if(registed == false)
		{
			led_trigger_register_simple("sleep", &ledtrig_sleep);
			register_pm_notifier(&ledtrig_sleep_pm_notifier);
			registed = true;
		}
	}
	else
	{
		if(registed == true)
		{
			unregister_pm_notifier(&ledtrig_sleep_pm_notifier);
			led_trigger_unregister_simple(ledtrig_sleep);
			registed = false;
		}
	}
		
	return 0;
}
module_param_call(state,  ledtrig_sleep_state_store, ledtrig_sleep_state_show, NULL, 0640);
#endif /* CONFIG_FEATURE_NCMC_USERDEBUG_RELEASE */

