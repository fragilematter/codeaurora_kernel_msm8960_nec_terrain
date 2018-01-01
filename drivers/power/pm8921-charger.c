/* Copyright (c) 2011-2012, Code Aurora Forum. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
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


#define pr_fmt(fmt)	"%s: " fmt, __func__

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/platform_device.h>
#include <linux/errno.h>
#include <linux/mfd/pm8xxx/pm8921-charger.h>
#include <linux/mfd/pm8xxx/pm8921-bms.h>
#include <linux/mfd/pm8xxx/pm8xxx-adc.h>
#include <linux/mfd/pm8xxx/ccadc.h>
#include <linux/mfd/pm8xxx/core.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/bitops.h>
#include <linux/workqueue.h>
#include <linux/debugfs.h>
#include <linux/slab.h>
#include <linux/reboot.h>
#include <mach/msm_xo.h>
#include <mach/msm_hsusb.h>
#include <linux/mfd/pm8xxx/batt-alarm.h>
#include <linux/timer.h>
#include <linux/android_alarm.h>
#include <linux/battim.h>
#include <linux/pm_param.h>
#include <linux/pm_obs_api.h>
#include <mach/msm_smsm.h>
#include <linux/i2c/usb_switch_if_knl.h>
#include <linux/power/ncm_charge_protect_sts.h>
#include <linux/rtc.h>
#include <linux/chgmon_chg.h>
#include <linux/oemnc_smem.h>

#undef CONFIG_NCMC_TELEC_CERTICATION_BUILD

#ifdef CONFIG_NCMC_TELEC_CERTICATION_BUILD
#ifndef CONFIG_FEATURE_NCMC_ONLY_FOR_PRODUCTION_PROCESS_8960
#define CONFIG_FEATURE_NCMC_ONLY_FOR_PRODUCTION_PROCESS_8960
#endif
#endif

#define nc_pr_debug(fmt, ...)						\
		if (nc_prdebug_enabled)					\
		  printk(KERN_DEBUG "[PM] %s: " fmt, __func__, ##__VA_ARGS__) \

#define CHG_BUCK_CLOCK_CTRL	0x14
#define PBL_ACCESS1		0x04
#define PBL_ACCESS2		0x05
#define SYS_CONFIG_1		0x06
#define SYS_CONFIG_2		0x07
#define CHG_CNTRL		0x204
#define CHG_IBAT_MAX		0x205
#define CHG_TEST		0x206
#define CHG_BUCK_CTRL_TEST1	0x207
#define CHG_BUCK_CTRL_TEST2	0x208
#define CHG_BUCK_CTRL_TEST3	0x209
#define COMPARATOR_OVERRIDE	0x20A
#define PSI_TXRX_SAMPLE_DATA_0	0x20B
#define PSI_TXRX_SAMPLE_DATA_1	0x20C
#define PSI_TXRX_SAMPLE_DATA_2	0x20D
#define PSI_TXRX_SAMPLE_DATA_3	0x20E
#define PSI_CONFIG_STATUS	0x20F
#define CHG_IBAT_SAFE		0x210
#define CHG_ITRICKLE		0x211
#define CHG_CNTRL_2		0x212
#define CHG_VBAT_DET		0x213
#define CHG_VTRICKLE		0x214
#define CHG_ITERM		0x215
#define CHG_CNTRL_3		0x216
#define CHG_VIN_MIN		0x217
#define CHG_TWDOG		0x218
#define CHG_TTRKL_MAX		0x219
#define CHG_TEMP_THRESH		0x21A
#define CHG_TCHG_MAX		0x21B
#define USB_OVP_CONTROL		0x21C
#define DC_OVP_CONTROL		0x21D
#define USB_OVP_TEST		0x21E
#define DC_OVP_TEST		0x21F
#define CHG_VDD_MAX		0x220
#define CHG_VDD_SAFE		0x221
#define CHG_VBAT_BOOT_THRESH	0x222
#define USB_OVP_TRIM		0x355
#define BUCK_CONTROL_TRIM1	0x356
#define BUCK_CONTROL_TRIM2	0x357
#define BUCK_CONTROL_TRIM3	0x358
#define BUCK_CONTROL_TRIM4	0x359
#define CHG_DEFAULTS_TRIM	0x35A
#define CHG_ITRIM		0x35B
#define CHG_TTRIM		0x35C
#define CHG_COMP_OVR		0x20A
#define IUSB_FINE_RES		0x2B6
#define EOC_CHECK_PERIOD_MS	10000
#define UNPLUG_CHECK_WAIT_PERIOD_MS 200
#define NCMC_FACTORY_MODE_CHG_CURRENT  1300
#define NCM_REPORT_PERIOD_SEC		(30*60)
#define NCM_PWR_ON_EVENT_KEYPAD      0x01
#define NCM_PWR_ON_EVENT_SMPL        0x08
#define NCM_PWR_ON_EVENT_USB_CHG     0x20
#define NCM_PWR_ON_EVENT_HARD_RESET  0x100
#if defined(CONFIG_FEATURE_NCMC_RUBY)
#define NCM_IUSB_SDP_MA             (500)
#define NCM_IUSB_DCP_MA             (1500)
#define NCM_IUSB_OTHER_DCP_MA       (1100)
#define NCM_IUSB_NONE_MA            (0)
#else
#define NCM_IUSB_SDP_MA             (500)
#define NCM_IUSB_DCP_MA             (1500)
#define NCM_IUSB_OTHER_DCP_MA       (900)
#define NCM_IUSB_NONE_MA            (0)
#endif

enum chg_fsm_state {
	FSM_STATE_OFF_0 = 0,
	FSM_STATE_BATFETDET_START_12 = 12,
	FSM_STATE_BATFETDET_END_16 = 16,
	FSM_STATE_ON_CHG_HIGHI_1 = 1,
	FSM_STATE_ATC_2A = 2,
	FSM_STATE_ATC_2B = 18,
	FSM_STATE_ON_BAT_3 = 3,
	FSM_STATE_ATC_FAIL_4 = 4 ,
	FSM_STATE_DELAY_5 = 5,
	FSM_STATE_ON_CHG_AND_BAT_6 = 6,
	FSM_STATE_FAST_CHG_7 = 7,
	FSM_STATE_TRKL_CHG_8 = 8,
	FSM_STATE_CHG_FAIL_9 = 9,
	FSM_STATE_EOC_10 = 10,
	FSM_STATE_ON_CHG_VREGOK_11 = 11,
	FSM_STATE_ATC_PAUSE_13 = 13,
	FSM_STATE_FAST_CHG_PAUSE_14 = 14,
	FSM_STATE_TRKL_CHG_PAUSE_15 = 15,
	FSM_STATE_START_BOOT = 20,
	FSM_STATE_FLCB_VREGOK = 21,
	FSM_STATE_FLCB = 22,
};
struct fsm_state_to_batt_status {
	enum chg_fsm_state	fsm_state;
	int			batt_state;
};
static struct fsm_state_to_batt_status map[] = {
	{FSM_STATE_OFF_0, POWER_SUPPLY_STATUS_UNKNOWN},
	{FSM_STATE_BATFETDET_START_12, POWER_SUPPLY_STATUS_UNKNOWN},
	{FSM_STATE_BATFETDET_END_16, POWER_SUPPLY_STATUS_UNKNOWN},
	{FSM_STATE_ON_CHG_HIGHI_1, POWER_SUPPLY_STATUS_FULL},
	{FSM_STATE_ATC_2A, POWER_SUPPLY_STATUS_CHARGING},
	{FSM_STATE_ATC_2B, POWER_SUPPLY_STATUS_CHARGING},
	{FSM_STATE_ON_BAT_3, POWER_SUPPLY_STATUS_DISCHARGING},
	{FSM_STATE_ATC_FAIL_4, POWER_SUPPLY_STATUS_DISCHARGING},
	{FSM_STATE_DELAY_5, POWER_SUPPLY_STATUS_UNKNOWN },
	{FSM_STATE_ON_CHG_AND_BAT_6, POWER_SUPPLY_STATUS_CHARGING},
	{FSM_STATE_FAST_CHG_7, POWER_SUPPLY_STATUS_CHARGING},
	{FSM_STATE_TRKL_CHG_8, POWER_SUPPLY_STATUS_CHARGING},
	{FSM_STATE_CHG_FAIL_9, POWER_SUPPLY_STATUS_DISCHARGING},
	{FSM_STATE_EOC_10, POWER_SUPPLY_STATUS_FULL},
	{FSM_STATE_ON_CHG_VREGOK_11, POWER_SUPPLY_STATUS_NOT_CHARGING},
	{FSM_STATE_ATC_PAUSE_13, POWER_SUPPLY_STATUS_NOT_CHARGING},
	{FSM_STATE_FAST_CHG_PAUSE_14, POWER_SUPPLY_STATUS_NOT_CHARGING},
	{FSM_STATE_TRKL_CHG_PAUSE_15, POWER_SUPPLY_STATUS_NOT_CHARGING},
	{FSM_STATE_START_BOOT, POWER_SUPPLY_STATUS_NOT_CHARGING},
	{FSM_STATE_FLCB_VREGOK, POWER_SUPPLY_STATUS_NOT_CHARGING},
	{FSM_STATE_FLCB, POWER_SUPPLY_STATUS_NOT_CHARGING},
};
enum chg_regulation_loop {
	VDD_LOOP = BIT(3),
	BAT_CURRENT_LOOP = BIT(2),
	INPUT_CURRENT_LOOP = BIT(1),
	INPUT_VOLTAGE_LOOP = BIT(0),
	CHG_ALL_LOOPS = VDD_LOOP | BAT_CURRENT_LOOP
			| INPUT_CURRENT_LOOP | INPUT_VOLTAGE_LOOP,
};
enum pmic_chg_interrupts {
	USBIN_VALID_IRQ = 0,
	USBIN_OV_IRQ,
	BATT_INSERTED_IRQ,
	VBATDET_LOW_IRQ,
	USBIN_UV_IRQ,
	VBAT_OV_IRQ,
	CHGWDOG_IRQ,
	VCP_IRQ,
	ATCDONE_IRQ,
	ATCFAIL_IRQ,
	CHGDONE_IRQ,
	CHGFAIL_IRQ,
	CHGSTATE_IRQ,
	LOOP_CHANGE_IRQ,
	FASTCHG_IRQ,
	TRKLCHG_IRQ,
	BATT_REMOVED_IRQ,
	BATTTEMP_HOT_IRQ,
	CHGHOT_IRQ,
	BATTTEMP_COLD_IRQ,
	CHG_GONE_IRQ,
	BAT_TEMP_OK_IRQ,
	COARSE_DET_LOW_IRQ,
	VDD_LOOP_IRQ,
	VREG_OV_IRQ,
	VBATDET_IRQ,
	BATFET_IRQ,
	PSI_IRQ,
	DCIN_VALID_IRQ,
	DCIN_OV_IRQ,
	DCIN_UV_IRQ,
	PM_CHG_MAX_INTS,
};
struct bms_notify {
	int			is_battery_full;
	int			is_charging;
	struct	work_struct	work;
};
enum {
	NCM_TMR_EVENT__INIT,
	NCM_TMR_EVENT__TMR_EXPIRED_30,
	NCM_TMR_EVENT__TMR_EXPIRED_5,
	NCM_TMR_EVENT__CHG_REMOVED,
	NCM_TMR_EVENT__CHG_INSERTED,
	NCM_TMR_EVENT__USR_READY,
	NCM_TMR_EVENT__SLP_LEFT_Q,
	NCM_TMR_EVENT__SLP_LEFT_S,
	NCM_TMR_EVENT__SLP_ENTERING,
	NCM_TMR_EVENT__MAX
};
enum {
	NCM_TMR_STATE__TMR_STOPPING,
	NCM_TMR_STATE__TMR_RUNNING,
	NCM_TMR_STATE__TMR_MAX
};
enum {
	NCM_TMR_STATE__CHG_DCED,
	NCM_TMR_STATE__CHG_CED,
	NCM_TMR_STATE__CHG_MAX
};
enum {
	NCM_TMR_PROC__HNDL_ALL,
	NCM_TMR_PROC__HNDL_ALL_ON_WAKEUP,
	NCM_TMR_PROC__HNDL_TEMP_AND_CHG,
	NCM_TMR_PROC__HNDL_TEMP,
	NCM_TMR_PROC__HNDL_CHG,
	NCM_TMR_PROC__HNDL_MAX
};
enum {
	NCM_EVENT_BMS_NOTIFY,
	NCM_EVENT_CUT_OFF,
	NCM_EVENT_CHARGE_PROTECT_DC_OVP,
	NCM_EVENT_CHARGE_PROTECT_USB_OVP,
	NCM_EVENT_CHARGE_PROTECT_USB_OCP,
	NCM_EVENT_CHARGE_PROTECT_BATT_ERR,
	NCM_EVENT_CHARGE_PROTECT_XO_TEMP,
	NCM_EVENT_CHARGE_PROTECT_NOTIFY,
	NCM_EVENT_CHARGE_PROTECT_CHARGE_TIMEOUT,
	NCM_EVENT_MAX
};
enum {
	NCM_DEBUG_TIMER		= 1U << 0,
	NCM_DEBUG_TIMER_WORK	= 1U << 1,
	NCM_DEBUG_USER_EVENT	= 1U << 2,
	NCM_DEBUG_WAKE_LOCK	= 1U << 3,
};
static int ncm_debug_mask = 0;
#define NCM_BATT_REMOVE_WAIT_MS		1000
#define NCM_BATT_REMOVE_THERM_THRESHOLD	1662
#define NCM_BATTADJ_WAIT_MAX_MS		500
#define NCM_TIMER_QUEUE_MAX		16
#define NCM_ADC_SAMPLE_NUM		5
#define NCM_ADC_MV_UNSET		(-1)
#define NCM_CHG_PROTECT_RET_OK					(0)
#define NCM_CHG_PROTECT_RET_NOCHG				(1)
#define NCM_CHG_PROTECT_RET_NG					(-1)
#define NCM_CHG_PROTECT_THERM_BATT_NONE			(0)
#define NCM_CHG_PROTECT_THERM_BATT_NORMAL		(1)
#define NCM_CHG_PROTECT_THERM_BATT_HOT			(2)
#define NCM_CHG_PROTECT_THERM_BATT_COLD			(3)
#define NCM_CHG_PROTECT_THERM_BATT_HIGH_ERR		(4)
#define NCM_CHG_PROTECT_THERM_BATT_LOW_ERR		(5)
#define NCM_CHG_PROTECT_THERM_XO_NONE			(0)
#define NCM_CHG_PROTECT_THERM_XO_NORMAL			(1)
#define NCM_CHG_PROTECT_THERM_XO_HIGH_ERR		(2)
#define NCM_CHG_PROTECT_THERM_XO_HIGH_CHK		(3)
#define NCM_CHG_PROTECT_THERM_XO_RECOVER_NONE	(0)
#define NCM_CHG_PROTECT_THERM_XO_RECOVER_NORMAL	(1)
#define NCM_CHG_PROTECT_THERM_XO_RECOVER_HIGH	(2)
#define NCM_CHG_DC_DISCONNECT					(0)
#define NCM_CHG_DC_CONNECT						(1)
#define NCM_CHG_DC_CONNECT_OVP					(2)
#define NCM_CHG_USB_DISCONNECT					(0)
#define NCM_CHG_USB_CONNECT						(1)
#define NCM_CHG_USB_CONNECT_OVP					(2)
typedef struct ncm_queue_struct {
	int data[NCM_TIMER_QUEUE_MAX];
	int head;
	int tail;
	spinlock_t slock;
} ncm_queue_type;
typedef struct ncm_protect_timer_struct {
	bool is_timeout;
	bool is_timer;
	int  cnt;
	spinlock_t slock;
	struct work_struct dc_chk_work;
	struct work_struct usb_chk_work;
	struct delayed_work worker;
} ncm_protect_timer_type;
typedef struct ncm_chg_chip_struct {
	struct work_struct timer_work;
	struct work_struct update_batt_work;
	struct work_struct watch_chg_work;
	struct work_struct notify_work;
	struct wake_lock timer_event_wlock;
	struct wake_lock timer_action_wlock;
	struct wake_lock event_wlock;
	struct {
		bool enable;
		struct mutex lock;
		int v_adc;
		int voltage;
		int capacity;
	} batt_adj;
	battim_obs_coll_type obs_data;
	bool chk_circuit_flag;
	struct delayed_work time_keep_work;
	bool waiting_batt_cal;
	bool waiting_obs_update;
	struct wake_lock waiting_wlock;
	bool user_ready;
	struct timer_list timer;
	bool timer_flag;
	spinlock_t timer_slock;
	atomic_t timer_lock_atomic;
	ncm_queue_type timer_queue;
	ncm_queue_type event_queue;
	struct {
		ktime_t on_read_batt;
		ktime_t on_suspend;
		ktime_t on_resume_adj;
	} time;
	unsigned long jiff_on_resume;
    spinlock_t eoc_slock;
    bool ncm_chg_eoc_work_flag;
	int calc_current;
	int chg_err_cycle;
	spinlock_t irq_slock;
	ncm_protect_timer_type  prtct_timer;
	int16_t chgmon_vbatdet;
	struct delayed_work batt_removed_handler_work;
} ncm_chg_chip_type;

#define ROLLBACK_REVERSEBOOST_FIX_METHOD

/**
 * struct pm8921_chg_chip -device information
 * @dev:			device pointer to access the parent
 * @usb_present:		present status of usb
 * @dc_present:			present status of dc
 * @usb_charger_current:	usb current to charge the battery with used when
 *				the usb path is enabled or charging is resumed
 * @safety_time:		max time for which charging will happen
 * @update_time:		how frequently the userland needs to be updated
 * @max_voltage_mv:		the max volts the batt should be charged up to
 * @min_voltage_mv:		the min battery voltage before turning the FETon
 * @cool_temp_dc:		the cool temp threshold in deciCelcius
 * @warm_temp_dc:		the warm temp threshold in deciCelcius
 * @resume_voltage_delta:	the voltage delta from vdd max at which the
 *				battery should resume charging
 * @term_current:		The charging based term current
 *
 */

struct pm8921_chg_chip {
	struct device			*dev;
	unsigned int			usb_present;
	unsigned int			dc_present;
	unsigned int			usb_charger_current;
	unsigned int			max_bat_chg_current;
	unsigned int			pmic_chg_irq[PM_CHG_MAX_INTS];
	unsigned int			safety_time;
	unsigned int			ttrkl_time;
	unsigned int			update_time;
	unsigned int			max_voltage_mv;
	unsigned int			min_voltage_mv;
	int				cool_temp_dc;
	int				warm_temp_dc;
	unsigned int			temp_check_period;
	unsigned int			cool_bat_chg_current;
	unsigned int			warm_bat_chg_current;
	unsigned int			cool_bat_voltage;
	unsigned int			warm_bat_voltage;
	unsigned int			is_bat_cool;
	unsigned int			is_bat_warm;
	unsigned int			resume_voltage_delta;
	unsigned int			term_current;
	unsigned int			vbat_channel;
	unsigned int			batt_temp_channel;
	unsigned int			batt_id_channel;
	struct power_supply		usb_psy;
	struct power_supply		dc_psy;
	struct power_supply		*ext_psy;
	struct power_supply		batt_psy;
	struct dentry			*dent;
	struct bms_notify		bms_notify;
	bool				keep_btm_on_suspend;
	bool				ext_charging;
	bool				ext_charge_done;
	bool				iusb_fine_res;
	DECLARE_BITMAP(enabled_irqs, PM_CHG_MAX_INTS);
	struct work_struct		battery_id_valid_work;
	int64_t				batt_id_min;
	int64_t				batt_id_max;
	int				trkl_voltage;
	int				weak_voltage;
	int				trkl_current;
	int				weak_current;
	int				vin_min;
	unsigned int			*thermal_mitigation;
	int				thermal_levels;
	struct delayed_work		update_heartbeat_work;
#ifdef ROLLBACK_REVERSEBOOST_FIX_METHOD
	struct delayed_work		unplug_wrkarnd_restore_work;
	struct delayed_work		unplug_check_work;
	struct wake_lock		unplug_wrkarnd_restore_wake_lock;
#else
	struct delayed_work		unplug_check_work;
#endif
	struct delayed_work		vin_collapse_check_work;
	struct wake_lock		eoc_wake_lock;
	enum pm8921_chg_cold_thr	cold_thr;
	enum pm8921_chg_hot_thr		hot_thr;
	int				rconn_mohm;
	ncm_chg_chip_type		ncm_chg_chip;
	bool				host_mode;
};

static unsigned int usb_max_current;
static int usb_target_ma;
static int charging_disabled;
static int thermal_mitigation;
static int nc_prdebug_enabled = 0;
static unsigned char ovp_ocp_prev_state   = USB_SW_STATE_NORMAL;
static unsigned char dc_ovp_prev_state = 0;
static char          nc_prev_chg_type;
static unsigned char ovp_ocp_alarm_state =  USB_SW_STATE_NORMAL;
static int           bat_adc_alarm_read_volts = 0x0000;
#define NCM_CHG_CUT_OFF_DELAY_TIME (10 * HZ)
struct wake_lock cut_off_delay_wl;
static int ncm_notify_batt_alarm(struct notifier_block *nb,
                                 unsigned long status, void *unused);
void ncm_register_batt_alarm(void);
static struct notifier_block alarm_notifier = {
	.notifier_call = ncm_notify_batt_alarm,
};
static usb_sw_device_state_enum nc_usb_device_type = USB_SW_DEVICE_DISCONNECTED;
#if defined(CONFIG_FEATURE_NCMC_ONLY_FOR_PRODUCTION_PROCESS_8960)
bool factory_mode_flag_apps = true;
#else
bool factory_mode_flag_apps = false;
static smem_id_vendor0 *p_smem_id_vendor0 = NULL;
#endif
static struct pm8921_chg_chip *the_chip;
static struct pm8xxx_adc_arb_btm_param btm_config;
static void handle_usb_insertion_removal(struct pm8921_chg_chip *chip);
#if (BITS_PER_LONG == 64) || defined(CONFIG_KTIME_SCALAR)
extern uint32_t __div64_32(uint64_t *dividend, uint32_t divisor);
static inline uint64_t ncm_ktime_to_sec(const ktime_t kt)
{
	uint64_t dividend = kt.tv64;
	__div64_32(&dividend, NSEC_PER_SEC);
	return dividend;
}
static inline uint32_t ncm_ktime_tvnsec_to_msec(const ktime_t kt)
{
	uint64_t dividend = kt.tv64;
	uint32_t rem;
	rem = __div64_32(&dividend, NSEC_PER_SEC);
	return rem / NSEC_PER_MSEC;
}
#define NCM_KTIME_TO_SEC(kt)            ncm_ktime_to_sec(kt)
#define NCM_KTIME_TVNSEC_TO_MSEC(kt)    ncm_ktime_tvnsec_to_msec(kt)
#else
#define NCM_KTIME_TO_SEC(kt)            (kt.tv.sec)
#define NCM_KTIME_TVNSEC_TO_MSEC(kt)    (kt.tv.nsec / NSEC_PER_MSEC)
#endif
static int ncm_get_battery_uvolts(struct pm8921_chg_chip *chip,
					int *result, int *measurement);
static void ncm_update_batt_work(struct work_struct *work);
static void ncm_watch_chg_work(struct work_struct *work);
static void ncm_timer_process_events(struct work_struct *work);
static void ncm_process_events(struct work_struct *work);
static void ncm_time_keep_work(struct work_struct *work);
static void ncm_notify_user_event(battim_event_type *data);
static void ncm_timer_notify_event(struct pm8921_chg_chip *chip, int tm_event);
static void ncm_notify_event(struct pm8921_chg_chip *chip, int event);
static void ncm_batt_removed_handler_func(struct work_struct *work);
static void ncm_user_event_get_xo_therm(void);
int ncm_protect_action_Batt_Remove   (ProtectIndex, ProtectType);
int ncm_protect_action_DC_Ovp        (ProtectIndex, ProtectType);
int ncm_protect_action_Usb_Ovp       (ProtectIndex, ProtectType);
int ncm_protect_action_Usb_Ocp       (ProtectIndex, ProtectType);
int ncm_protect_action_Batt_TempErr  (ProtectIndex, ProtectType);
int ncm_protect_action_Batt_TempLimit(ProtectIndex, ProtectType);
int ncm_protect_action_XO_Temp       (ProtectIndex, ProtectType);
int ncm_protect_action_Charge_Timeout(ProtectIndex, ProtectType);
int ncm_protect_recover_Batt_Remove   (ProtectIndex, ProtectType);
int ncm_protect_recover_DC_Ovp        (ProtectIndex, ProtectType);
int ncm_protect_recover_Usb_Ovp       (ProtectIndex, ProtectType);
int ncm_protect_recover_Usb_Ocp       (ProtectIndex, ProtectType);
int ncm_protect_recover_Batt_TempErr  (ProtectIndex, ProtectType);
int ncm_protect_recover_Batt_TempLimit(ProtectIndex, ProtectType);
int ncm_protect_recover_XO_Temp       (ProtectIndex, ProtectType);
int ncm_protect_recover_Charge_Timeout(ProtectIndex, ProtectType);
static int ncm_charge_protection_main(int charge_src);
static int ncm_chg_protect_therm_chk(void);
static int ncm_chg_protect_therm_chk_batt(int *batt_judge_res);
static int ncm_chg_protect_therm_chk_XO(int *xo_judge_res, int *xo_recover_res);
static int ncm_chg_protect_therm_judge_batt(int batt_judge_res, int xo_recover_res);
static int ncm_chg_protect_therm_judge_xo(int batt_judge_res, int xo_judge_res, int xo_recover_res);
#if defined(CONFIG_FEATURE_NCMC_POWER) && !defined(CONFIG_FEATURE_NCMC_RUBY)
static int ncm_chg_protect_usbsw(void);
static int ncm_chg_protect_get_usbsw_sts(unsigned char *cur_ovp_ocp_sts);
static int ncm_chg_protect_ovp_judge_usbsw(unsigned char cur_ovp_ocp_sts);
static int ncm_chg_protect_ocp_judge_usbsw(unsigned char cur_ovp_ocp_sts);
#else
static int ncm_chg_protect_usbin(void);
static int ncm_chg_protect_ovp_judge_usbin(unsigned char cur_ovp_ocp_sts);
#endif
static int ncm_chg_protect_dc_chg(void);
static int ncm_chg_protect_chager_remove(void);
static int ncm_is_connect_dcin(void);
#if defined(CONFIG_FEATURE_NCMC_POWER) && defined(CONFIG_FEATURE_NCMC_RUBY)
static int ncm_is_connect_usbin(void);
#endif
static int ncm_is_connect_usbchg(void);
static void ncm_watch_chg_work_for_charging(struct pm8921_chg_chip *chip);
static void ncm_pm8921_charger_vbus_draw(unsigned int mA);
unsigned int ncm_usb_charger_current_present;
unsigned int ncm_usb_charger_current_from_usb;
static int ncm_chgmon_charger_type(void);
static int ncm_chgmon_charger_state(void);
static void ncm_chgmon_update_data(void);
static void ncm_chg_set_charging_vbatdet(void);
static void ncm_chg_set_charging_vbatdet_eoc(void);
static void ncm_chg_set_charging_ichgusb(void);
static bool ncm_chg_uv_flag = FALSE;
static bool ncm_is_initialized = false;
static int ncm_prev_chgstate   = CHGMON_PM_CHG_STATE_MAX_NUM;
static int ncm_active_chgstate = CHGMON_PM_CHG_STATE_OFF;
static int ncm_prev_chgtype   = CHGMON_PM_CHARGER_MAX_NUM;
static int ncm_active_chgtype = CHGMON_PM_CHG_CHARGER_NONE;
static void ncm_chg_protect_timer_worker(struct work_struct *work);
static void ncm_chg_protect_timer_dc_chk_work(struct work_struct *work);
static void ncm_chg_protect_timer_usb_chk_work(struct work_struct *work);
static void ncm_chg_protect_timer_enable(bool enanle);
static int ncm_chg_start_flag = true;
static int ncm_batt_temp_low_flag = false;
static bool ncm_batt_temp_recover_flag = false;
static bool ncm_xo_temp_recover_flag = false;
static bool unplug_check_work_flag = false;
static bool ncm_chg_usbsw_ov_flag = false;
static int pm_chg_iusbmax_set(struct pm8921_chg_chip *chip, int reg_val);

static int pm_chg_masked_write(struct pm8921_chg_chip *chip, u16 addr,
							u8 mask, u8 val)
{
	int rc;
	u8 reg;
	rc = pm8xxx_readb(chip->dev->parent, addr, &reg);
	if (rc) {
		pr_err("pm8xxx_readb failed: addr=%03X, rc=%d\n", addr, rc);
		return rc;
	}
	reg &= ~mask;
	reg |= val & mask;
	rc = pm8xxx_writeb(chip->dev->parent, addr, reg);
	if (rc) {
		pr_err("pm8xxx_writeb failed: addr=%03X, rc=%d\n", addr, rc);
		return rc;
	}
	return 0;
}

static int pm_chg_get_rt_status(struct pm8921_chg_chip *chip, int irq_id)
{
	return pm8xxx_read_irq_stat(chip->dev->parent,
					chip->pmic_chg_irq[irq_id]);
}

static int is_usb_chg_plugged_in(struct pm8921_chg_chip *chip)
{
#if defined(CONFIG_FEATURE_NCMC_POWER) && !defined(CONFIG_FEATURE_NCMC_RUBY)
	if(nc_usb_device_type == USB_SW_SDP_CONNECTED ||
	   nc_usb_device_type == USB_SW_DCP_CONNECTED ||
	   nc_usb_device_type == USB_SW_OTHER_DCP_CONNECTED)
	{
		return 1;
	}
	else if(nc_usb_device_type == USB_SW_DEVICE_DISCONNECTED ||
	        nc_usb_device_type == USB_SW_OTHER_DEVICE_CONNECTED)
	{
		return 0;
	}
	else
	{
		return 0;
	}
#else
	return pm_chg_get_rt_status(chip, USBIN_VALID_IRQ);
#endif
}

static int is_dc_chg_plugged_in(struct pm8921_chg_chip *chip)
{
	return pm_chg_get_rt_status(chip, DCIN_VALID_IRQ);
}
#define CAPTURE_FSM_STATE_CMD	0xC2
#define READ_BANK_7		0x70
#define READ_BANK_4		0x40
static int pm_chg_get_fsm_state(struct pm8921_chg_chip *chip)
{
	u8 temp;
	int err, ret = 0;
	temp = CAPTURE_FSM_STATE_CMD;
	err = pm8xxx_writeb(chip->dev->parent, CHG_TEST, temp);
	if (err) {
		pr_err("Error %d writing %d to addr %d\n", err, temp, CHG_TEST);
		return err;
	}
	temp = READ_BANK_7;
	err = pm8xxx_writeb(chip->dev->parent, CHG_TEST, temp);
	if (err) {
		pr_err("Error %d writing %d to addr %d\n", err, temp, CHG_TEST);
		return err;
	}
	err = pm8xxx_readb(chip->dev->parent, CHG_TEST, &temp);
	if (err) {
		pr_err("pm8xxx_readb fail: addr=%03X, rc=%d\n", CHG_TEST, err);
		return err;
	}
	ret = temp & 0xF;
	temp = READ_BANK_4;
	err = pm8xxx_writeb(chip->dev->parent, CHG_TEST, temp);
	if (err) {
		pr_err("Error %d writing %d to addr %d\n", err, temp, CHG_TEST);
		return err;
	}
	err = pm8xxx_readb(chip->dev->parent, CHG_TEST, &temp);
	if (err) {
		pr_err("pm8xxx_readb fail: addr=%03X, rc=%d\n", CHG_TEST, err);
		return err;
	}
	ret |= (temp & 0x1) << 4;
	return  ret;
}
#define READ_BANK_6		0x60
static int pm_chg_get_regulation_loop(struct pm8921_chg_chip *chip)
{
	u8 temp;
	int err;
	temp = READ_BANK_6;
	err = pm8xxx_writeb(chip->dev->parent, CHG_TEST, temp);
	if (err) {
		pr_err("Error %d writing %d to addr %d\n", err, temp, CHG_TEST);
		return err;
	}
	err = pm8xxx_readb(chip->dev->parent, CHG_TEST, &temp);
	if (err) {
		pr_err("pm8xxx_readb fail: addr=%03X, rc=%d\n", CHG_TEST, err);
		return err;
	}
	return temp & CHG_ALL_LOOPS;
}
#define CHG_USB_SUSPEND_BIT  BIT(2)
static int pm_chg_usb_suspend_enable(struct pm8921_chg_chip *chip, int enable)
{
	return pm_chg_masked_write(chip, CHG_CNTRL_3, CHG_USB_SUSPEND_BIT,
			enable ? CHG_USB_SUSPEND_BIT : 0);
}
#define CHG_EN_BIT	BIT(7)
static int pm_chg_auto_enable(struct pm8921_chg_chip *chip, int enable)
{
	return pm_chg_masked_write(chip, CHG_CNTRL_3, CHG_EN_BIT,
				enable ? CHG_EN_BIT : 0);
}
#define CHG_FAILED_CLEAR	BIT(0)
#define ATC_FAILED_CLEAR	BIT(1)
static int pm_chg_failed_clear(struct pm8921_chg_chip *chip, int clear)
{
	int rc;
	rc = pm_chg_masked_write(chip, CHG_CNTRL_3, ATC_FAILED_CLEAR,
				clear ? ATC_FAILED_CLEAR : 0);
	rc |= pm_chg_masked_write(chip, CHG_CNTRL_3, CHG_FAILED_CLEAR,
				clear ? CHG_FAILED_CLEAR : 0);
	return rc;
}
#define CHG_CHARGE_DIS_BIT	BIT(1)
static int pm_chg_charge_dis(struct pm8921_chg_chip *chip, int disable)
{
	return pm_chg_masked_write(chip, CHG_CNTRL, CHG_CHARGE_DIS_BIT,
				disable ? CHG_CHARGE_DIS_BIT : 0);
}
static int pm_chg_nonsupport_charge_dis(struct pm8921_chg_chip *chip, int disable)
{
	int rc;
	int ret;
    if(disable) {
        rc = pm_chg_iusbmax_set(chip,7);
        msleep(20);
        rc += pm_chg_iusbmax_set(chip,6);
        msleep(20);
        rc += pm_chg_iusbmax_set(chip,4);
        msleep(20);
        rc += pm_chg_iusbmax_set(chip,2);
        msleep(20);
		if (rc) {
			pr_err("Failed to set usb max rc=%d\n", rc);
		}
		ret =pm_chg_masked_write(chip, CHG_CNTRL, CHG_CHARGE_DIS_BIT,
				disable ? CHG_CHARGE_DIS_BIT : 0);
    }
    else {
        rc = pm_chg_iusbmax_set(chip,2);
		ret =pm_chg_masked_write(chip, CHG_CNTRL, CHG_CHARGE_DIS_BIT,
				disable ? CHG_CHARGE_DIS_BIT : 0);
		msleep(20);
        rc += pm_chg_iusbmax_set(chip,4);
		msleep(20);
        rc +=pm_chg_iusbmax_set(chip,6);
        msleep(20);
        rc +=pm_chg_iusbmax_set(chip,7);
        msleep(20);
        rc +=pm_chg_iusbmax_set(chip,10);
        if (rc) {
			pr_err("Failed to set usb max rc=%d\n", rc);
		}
    }
    return ret;
}
#define PM8921_CHG_V_MIN_MV	3240
#define PM8921_CHG_V_STEP_MV	20
#define PM8921_CHG_V_STEP_10MV_OFFSET_BIT	BIT(7)
#define PM8921_CHG_VDDMAX_MAX	4500
#define PM8921_CHG_VDDMAX_MIN	3400
#define PM8921_CHG_V_MASK	0x7F
static int __pm_chg_vddmax_set(struct pm8921_chg_chip *chip, int voltage)
{
	int remainder;
	u8 temp = 0;
	if (voltage < PM8921_CHG_VDDMAX_MIN
			|| voltage > PM8921_CHG_VDDMAX_MAX) {
		pr_err("bad mV=%d asked to set\n", voltage);
		return -EINVAL;
	}
	temp = (voltage - PM8921_CHG_V_MIN_MV) / PM8921_CHG_V_STEP_MV;
	remainder = voltage % 20;
	if (remainder >= 10) {
		temp |= PM8921_CHG_V_STEP_10MV_OFFSET_BIT;
	}
	pr_debug("voltage=%d setting %02x\n", voltage, temp);
	return pm8xxx_writeb(chip->dev->parent, CHG_VDD_MAX, temp);
}
static int pm_chg_vddmax_get(struct pm8921_chg_chip *chip, int *voltage)
{
	u8 temp;
	int rc;
	rc = pm8xxx_readb(chip->dev->parent, CHG_VDD_MAX, &temp);
	if (rc) {
		pr_err("rc = %d while reading vdd max\n", rc);
		*voltage = 0;
		return rc;
	}
	*voltage = (int)(temp & PM8921_CHG_V_MASK) * PM8921_CHG_V_STEP_MV
							+ PM8921_CHG_V_MIN_MV;
	if (temp & PM8921_CHG_V_STEP_10MV_OFFSET_BIT)
		*voltage =  *voltage + 10;
	return 0;
}
static int pm_chg_vddmax_set(struct pm8921_chg_chip *chip, int voltage)
{
	int current_mv, ret, steps, i;
	bool increase;
	ret = 0;
	if (voltage < PM8921_CHG_VDDMAX_MIN
		|| voltage > PM8921_CHG_VDDMAX_MAX) {
		pr_err("bad mV=%d asked to set\n", voltage);
		return -EINVAL;
	}
	ret = pm_chg_vddmax_get(chip, &current_mv);
	if (ret) {
		pr_err("Failed to read vddmax rc=%d\n", ret);
		return -EINVAL;
	}
	if (current_mv == voltage)
		return 0;
	if (is_usb_chg_plugged_in(chip)) {
		if (current_mv < voltage) {
			steps = (voltage - current_mv) / PM8921_CHG_V_STEP_MV;
			increase = true;
		} else {
			steps = (current_mv - voltage) / PM8921_CHG_V_STEP_MV;
			increase = false;
		}
		for (i = 0; i < steps; i++) {
			if (increase)
				current_mv += PM8921_CHG_V_STEP_MV;
			else
				current_mv -= PM8921_CHG_V_STEP_MV;
			ret |= __pm_chg_vddmax_set(chip, current_mv);
		}
	}
	ret |= __pm_chg_vddmax_set(chip, voltage);
	return ret;
}
#define PM8921_CHG_VDDSAFE_MIN	3400
#define PM8921_CHG_VDDSAFE_MAX	4500
static int pm_chg_vddsafe_set(struct pm8921_chg_chip *chip, int voltage)
{
	u8 temp;
	if (voltage < PM8921_CHG_VDDSAFE_MIN
			|| voltage > PM8921_CHG_VDDSAFE_MAX) {
		pr_err("bad mV=%d asked to set\n", voltage);
		return -EINVAL;
	}
	temp = (voltage - PM8921_CHG_V_MIN_MV) / PM8921_CHG_V_STEP_MV;
	pr_debug("voltage=%d setting %02x\n", voltage, temp);
	return pm_chg_masked_write(chip, CHG_VDD_SAFE, PM8921_CHG_V_MASK, temp);
}
#define PM8921_CHG_VBATDET_MIN	3240
#define PM8921_CHG_VBATDET_MAX	5780
static int pm_chg_vbatdet_set(struct pm8921_chg_chip *chip, int voltage)
{
	u8 temp;
	if (voltage < PM8921_CHG_VBATDET_MIN
			|| voltage > PM8921_CHG_VBATDET_MAX) {
		pr_err("bad mV=%d asked to set\n", voltage);
		return -EINVAL;
	}
	temp = (voltage - PM8921_CHG_V_MIN_MV) / PM8921_CHG_V_STEP_MV;
	pr_debug("voltage=%d setting %02x\n", voltage, temp);
	chip->ncm_chg_chip.chgmon_vbatdet = (int16_t)voltage;
	return pm_chg_masked_write(chip, CHG_VBAT_DET, PM8921_CHG_V_MASK, temp);
}
#define PM8921_CHG_VINMIN_MIN_MV	3800
#define PM8921_CHG_VINMIN_STEP_MV	100
#define PM8921_CHG_VINMIN_USABLE_MAX	6500
#define PM8921_CHG_VINMIN_USABLE_MIN	4300
#define PM8921_CHG_VINMIN_MASK		0x1F
static int pm_chg_vinmin_set(struct pm8921_chg_chip *chip, int voltage)
{
	u8 temp;
	if (voltage < PM8921_CHG_VINMIN_USABLE_MIN
			|| voltage > PM8921_CHG_VINMIN_USABLE_MAX) {
		pr_err("bad mV=%d asked to set\n", voltage);
		return -EINVAL;
	}
	temp = (voltage - PM8921_CHG_VINMIN_MIN_MV) / PM8921_CHG_VINMIN_STEP_MV;
	pr_debug("voltage=%d setting %02x\n", voltage, temp);
	return pm_chg_masked_write(chip, CHG_VIN_MIN, PM8921_CHG_VINMIN_MASK,
									temp);
}
static int pm_chg_vinmin_get(struct pm8921_chg_chip *chip)
{
	u8 temp;
	int rc, voltage_mv;
	rc = pm8xxx_readb(chip->dev->parent, CHG_VIN_MIN, &temp);
	temp &= PM8921_CHG_VINMIN_MASK;
	voltage_mv = PM8921_CHG_VINMIN_MIN_MV +
			(int)temp * PM8921_CHG_VINMIN_STEP_MV;
	return voltage_mv;
}
#define PM8921_CHG_IBATMAX_MIN	325
#define PM8921_CHG_IBATMAX_MAX	2000
#define PM8921_CHG_I_MIN_MA	225
#define PM8921_CHG_I_STEP_MA	50
#define PM8921_CHG_I_MASK	0x3F
static int pm_chg_ibatmax_set(struct pm8921_chg_chip *chip, int chg_current)
{
	u8 temp;
	if (chg_current < PM8921_CHG_IBATMAX_MIN
			|| chg_current > PM8921_CHG_IBATMAX_MAX) {
		pr_err("bad mA=%d asked to set\n", chg_current);
		return -EINVAL;
	}
	temp = (chg_current - PM8921_CHG_I_MIN_MA) / PM8921_CHG_I_STEP_MA;
	return pm_chg_masked_write(chip, CHG_IBAT_MAX, PM8921_CHG_I_MASK, temp);
}
#define PM8921_CHG_IBATSAFE_MIN	225
#define PM8921_CHG_IBATSAFE_MAX	3375
static int pm_chg_ibatsafe_set(struct pm8921_chg_chip *chip, int chg_current)
{
	u8 temp;
	if (chg_current < PM8921_CHG_IBATSAFE_MIN
			|| chg_current > PM8921_CHG_IBATSAFE_MAX) {
		pr_err("bad mA=%d asked to set\n", chg_current);
		return -EINVAL;
	}
	temp = (chg_current - PM8921_CHG_I_MIN_MA) / PM8921_CHG_I_STEP_MA;
	return pm_chg_masked_write(chip, CHG_IBAT_SAFE,
						PM8921_CHG_I_MASK, temp);
}
#define PM8921_CHG_ITERM_MIN_MA		50
#define PM8921_CHG_ITERM_MAX_MA		200
#define PM8921_CHG_ITERM_STEP_MA	10
#define PM8921_CHG_ITERM_MASK		0xF
static int pm_chg_iterm_set(struct pm8921_chg_chip *chip, int chg_current)
{
	u8 temp;
	if (chg_current < PM8921_CHG_ITERM_MIN_MA
			|| chg_current > PM8921_CHG_ITERM_MAX_MA) {
		pr_err("bad mA=%d asked to set\n", chg_current);
		return -EINVAL;
	}
	temp = (chg_current - PM8921_CHG_ITERM_MIN_MA)
				/ PM8921_CHG_ITERM_STEP_MA;
	return pm_chg_masked_write(chip, CHG_ITERM, PM8921_CHG_ITERM_MASK,
					 temp);
}
static int pm_chg_iterm_get(struct pm8921_chg_chip *chip, int *chg_current)
{
	u8 temp;
	int rc;
	rc = pm8xxx_readb(chip->dev->parent, CHG_ITERM, &temp);
	if (rc) {
		pr_err("err=%d reading CHG_ITEM\n", rc);
		*chg_current = 0;
		return rc;
	}
	temp &= PM8921_CHG_ITERM_MASK;
	*chg_current = (int)temp * PM8921_CHG_ITERM_STEP_MA
					+ PM8921_CHG_ITERM_MIN_MA;
	return 0;
}
struct usb_ma_limit_entry {
	int	usb_ma;
	u8	value;
};
static struct usb_ma_limit_entry usb_ma_table[] = {
	{100, 0x0},
	{200, 0x1},
	{500, 0x2},
	{600, 0x3},
	{700, 0x4},
	{800, 0x5},
	{850, 0x6},
	{900, 0x8},
	{950, 0x7},
	{1000, 0x9},
	{1100, 0xA},
	{1200, 0xB},
	{1300, 0xC},
	{1400, 0xD},
	{1500, 0xE},
	{1600, 0xF},
};
#define PM8921_CHG_IUSB_MASK 0x1C
#define PM8921_CHG_IUSB_SHIFT 2
#define PM8921_CHG_IUSB_MAX  7
#define PM8921_CHG_IUSB_MIN  0
#define PM8917_IUSB_FINE_RES BIT(0)
static int pm_chg_iusbmax_set(struct pm8921_chg_chip *chip, int reg_val)
{
	u8 temp, fineres;
	int rc;
	fineres = PM8917_IUSB_FINE_RES & usb_ma_table[reg_val].value;
	reg_val = usb_ma_table[reg_val].value >> 1;
	if (reg_val < PM8921_CHG_IUSB_MIN || reg_val > PM8921_CHG_IUSB_MAX) {
		pr_err("bad mA=%d asked to set\n", reg_val);
		return -EINVAL;
	}
	temp = reg_val << PM8921_CHG_IUSB_SHIFT;
	if (chip->iusb_fine_res) {
		rc = pm_chg_masked_write(chip, IUSB_FINE_RES,
			PM8917_IUSB_FINE_RES, 0);
		rc |= pm_chg_masked_write(chip, PBL_ACCESS2,
			PM8921_CHG_IUSB_MASK, temp);
		if (rc) {
			pr_err("Failed to write PBL_ACCESS2 rc=%d\n", rc);
			return rc;
		}
		if (fineres) {
			rc = pm_chg_masked_write(chip, IUSB_FINE_RES,
				PM8917_IUSB_FINE_RES, fineres);
			if (rc)
				pr_err("Failed to write ISUB_FINE_RES rc=%d\n",
					rc);
		}
	} else {
		rc = pm_chg_masked_write(chip, PBL_ACCESS2,
			PM8921_CHG_IUSB_MASK, temp);
		if (rc)
			pr_err("Failed to write PBL_ACCESS2 rc=%d\n", rc);
	}
	return rc;
}
static int pm_chg_iusbmax_get(struct pm8921_chg_chip *chip, int *mA)
{
	u8 temp, fineres;
	int rc, i;
	fineres = 0;
	*mA = 0;
	rc = pm8xxx_readb(chip->dev->parent, PBL_ACCESS2, &temp);
	if (rc) {
		pr_err("err=%d reading PBL_ACCESS2\n", rc);
		return rc;
	}
	if (chip->iusb_fine_res) {
		rc = pm8xxx_readb(chip->dev->parent, IUSB_FINE_RES, &fineres);
		if (rc) {
			pr_err("err=%d reading IUSB_FINE_RES\n", rc);
			return rc;
		}
	}
	temp &= PM8921_CHG_IUSB_MASK;
	temp = temp >> PM8921_CHG_IUSB_SHIFT;
	temp = (temp << 1) | (fineres & PM8917_IUSB_FINE_RES);
	for (i = ARRAY_SIZE(usb_ma_table) - 1; i >= 0; i--) {
		if (usb_ma_table[i].value == temp)
			break;
	}
	*mA = usb_ma_table[i].usb_ma;
	return rc;
}
#define PM8921_CHG_WD_MASK 0x1F
static int pm_chg_disable_wd(struct pm8921_chg_chip *chip)
{
	return pm_chg_masked_write(chip, CHG_TWDOG, PM8921_CHG_WD_MASK, 0);
}
#define PM8921_CHG_TCHG_MASK	0x7F
#define PM8921_CHG_TCHG_MIN	4
#define PM8921_CHG_TCHG_MAX	512
#define PM8921_CHG_TCHG_STEP	4
static int pm_chg_tchg_max_set(struct pm8921_chg_chip *chip, int minutes)
{
	u8 temp;
	if (minutes < PM8921_CHG_TCHG_MIN || minutes > PM8921_CHG_TCHG_MAX) {
		pr_err("bad max minutes =%d asked to set\n", minutes);
		return -EINVAL;
	}
	temp = (minutes - 1)/PM8921_CHG_TCHG_STEP;
	return pm_chg_masked_write(chip, CHG_TCHG_MAX, PM8921_CHG_TCHG_MASK,
					 temp);
}
#define PM8921_CHG_TTRKL_MASK	0x1F
#define PM8921_CHG_TTRKL_MIN	1
#define PM8921_CHG_TTRKL_MAX	64
static int pm_chg_ttrkl_max_set(struct pm8921_chg_chip *chip, int minutes)
{
	u8 temp;
	if (minutes < PM8921_CHG_TTRKL_MIN || minutes > PM8921_CHG_TTRKL_MAX) {
		pr_err("bad max minutes =%d asked to set\n", minutes);
		return -EINVAL;
	}
	temp = minutes - 1;
	return pm_chg_masked_write(chip, CHG_TTRKL_MAX, PM8921_CHG_TTRKL_MASK,
					 temp);
}
#define PM8921_CHG_VTRKL_MIN_MV		2050
#define PM8921_CHG_VTRKL_MAX_MV		2800
#define PM8921_CHG_VTRKL_STEP_MV	50
#define PM8921_CHG_VTRKL_SHIFT		4
#define PM8921_CHG_VTRKL_MASK		0xF0
static int pm_chg_vtrkl_low_set(struct pm8921_chg_chip *chip, int millivolts)
{
	u8 temp;
	if (millivolts < PM8921_CHG_VTRKL_MIN_MV
			|| millivolts > PM8921_CHG_VTRKL_MAX_MV) {
		pr_err("bad voltage = %dmV asked to set\n", millivolts);
		return -EINVAL;
	}
	temp = (millivolts - PM8921_CHG_VTRKL_MIN_MV)/PM8921_CHG_VTRKL_STEP_MV;
	temp = temp << PM8921_CHG_VTRKL_SHIFT;
	return pm_chg_masked_write(chip, CHG_VTRICKLE, PM8921_CHG_VTRKL_MASK,
					 temp);
}
#define PM8921_CHG_VWEAK_MIN_MV		2100
#define PM8921_CHG_VWEAK_MAX_MV		3600
#define PM8921_CHG_VWEAK_STEP_MV	100
#define PM8921_CHG_VWEAK_MASK		0x0F
static int pm_chg_vweak_set(struct pm8921_chg_chip *chip, int millivolts)
{
	u8 temp;
	if (millivolts < PM8921_CHG_VWEAK_MIN_MV
			|| millivolts > PM8921_CHG_VWEAK_MAX_MV) {
		pr_err("bad voltage = %dmV asked to set\n", millivolts);
		return -EINVAL;
	}
	temp = (millivolts - PM8921_CHG_VWEAK_MIN_MV)/PM8921_CHG_VWEAK_STEP_MV;
	return pm_chg_masked_write(chip, CHG_VTRICKLE, PM8921_CHG_VWEAK_MASK,
					 temp);
}
#define PM8921_CHG_ITRKL_MIN_MA		50
#define PM8921_CHG_ITRKL_MAX_MA		200
#define PM8921_CHG_ITRKL_MASK		0x0F
#define PM8921_CHG_ITRKL_STEP_MA	10
static int pm_chg_itrkl_set(struct pm8921_chg_chip *chip, int milliamps)
{
	u8 temp;
	if (milliamps < PM8921_CHG_ITRKL_MIN_MA
		|| milliamps > PM8921_CHG_ITRKL_MAX_MA) {
		pr_err("bad current = %dmA asked to set\n", milliamps);
		return -EINVAL;
	}
	temp = (milliamps - PM8921_CHG_ITRKL_MIN_MA)/PM8921_CHG_ITRKL_STEP_MA;
	return pm_chg_masked_write(chip, CHG_ITRICKLE, PM8921_CHG_ITRKL_MASK,
					 temp);
}
#define PM8921_CHG_IWEAK_MIN_MA		325
#define PM8921_CHG_IWEAK_MAX_MA		525
#define PM8921_CHG_IWEAK_SHIFT		7
#define PM8921_CHG_IWEAK_MASK		0x80
static int pm_chg_iweak_set(struct pm8921_chg_chip *chip, int milliamps)
{
	u8 temp;
	if (milliamps < PM8921_CHG_IWEAK_MIN_MA
		|| milliamps > PM8921_CHG_IWEAK_MAX_MA) {
		pr_err("bad current = %dmA asked to set\n", milliamps);
		return -EINVAL;
	}
	if (milliamps < PM8921_CHG_IWEAK_MAX_MA)
		temp = 0;
	else
		temp = 1;
	temp = temp << PM8921_CHG_IWEAK_SHIFT;
	return pm_chg_masked_write(chip, CHG_ITRICKLE, PM8921_CHG_IWEAK_MASK,
					 temp);
}
#define PM8921_CHG_BATT_TEMP_THR_COLD	BIT(1)
#define PM8921_CHG_BATT_TEMP_THR_COLD_SHIFT	1
static int pm_chg_batt_cold_temp_config(struct pm8921_chg_chip *chip,
					enum pm8921_chg_cold_thr cold_thr)
{
	u8 temp;
	temp = cold_thr << PM8921_CHG_BATT_TEMP_THR_COLD_SHIFT;
	temp = temp & PM8921_CHG_BATT_TEMP_THR_COLD;
	return pm_chg_masked_write(chip, CHG_CNTRL_2,
					PM8921_CHG_BATT_TEMP_THR_COLD,
					 temp);
}
#define PM8921_CHG_BATT_TEMP_THR_HOT		BIT(0)
#define PM8921_CHG_BATT_TEMP_THR_HOT_SHIFT	0
static int pm_chg_batt_hot_temp_config(struct pm8921_chg_chip *chip,
					enum pm8921_chg_hot_thr hot_thr)
{
	u8 temp;
	temp = hot_thr << PM8921_CHG_BATT_TEMP_THR_HOT_SHIFT;
	temp = temp & PM8921_CHG_BATT_TEMP_THR_HOT;
	return pm_chg_masked_write(chip, CHG_CNTRL_2,
					PM8921_CHG_BATT_TEMP_THR_HOT,
					 temp);
}
#ifndef ROLLBACK_REVERSEBOOST_FIX_METHOD
static void disable_input_voltage_regulation(struct pm8921_chg_chip *chip)
{
	u8 temp;
	int rc;
	rc = pm8xxx_writeb(chip->dev->parent, CHG_BUCK_CTRL_TEST3, 0x70);
	if (rc) {
		pr_err("Failed to write 0x70 to CTRL_TEST3 rc = %d\n", rc);
		return;
	}
	rc = pm8xxx_readb(chip->dev->parent, CHG_BUCK_CTRL_TEST3, &temp);
	if (rc) {
		pr_err("Failed to read CTRL_TEST3 rc = %d\n", rc);
		return;
	}
	temp |= 0x81;
	rc = pm8xxx_writeb(chip->dev->parent, CHG_BUCK_CTRL_TEST3, temp);
	if (rc) {
		pr_err("Failed to write 0x%x to CTRL_TEST3 rc=%d\n", temp, rc);
		return;
	}
}
static void enable_input_voltage_regulation(struct pm8921_chg_chip *chip)
{
	u8 temp;
	int rc;
	rc = pm8xxx_writeb(chip->dev->parent, CHG_BUCK_CTRL_TEST3, 0x70);
	if (rc) {
		pr_err("Failed to write 0x70 to CTRL_TEST3 rc = %d\n", rc);
		return;
	}
	rc = pm8xxx_readb(chip->dev->parent, CHG_BUCK_CTRL_TEST3, &temp);
	if (rc) {
		pr_err("Failed to read CTRL_TEST3 rc = %d\n", rc);
		return;
	}
	temp &= 0xFE;
	temp |= 0x80;
	rc = pm8xxx_writeb(chip->dev->parent, CHG_BUCK_CTRL_TEST3, temp);
	if (rc) {
		pr_err("Failed to write 0x%x to CTRL_TEST3 rc=%d\n", temp, rc);
		return;
	}
}
#endif
static int64_t read_battery_id(struct pm8921_chg_chip *chip)
{
	int rc;
	struct pm8xxx_adc_chan_result result;
	rc = pm8xxx_adc_read(chip->batt_id_channel, &result);
	if (rc) {
		pr_err("error reading batt id channel = %d, rc = %d\n",
					chip->vbat_channel, rc);
		return rc;
	}
	pr_debug("batt_id phy = %lld meas = 0x%llx\n", result.physical,
						result.measurement);
	return result.physical;
}
static int is_battery_valid(struct pm8921_chg_chip *chip)
{
	int64_t rc;
	if (chip->batt_id_min == 0 && chip->batt_id_max == 0)
		return 1;
	rc = read_battery_id(chip);
	if (rc < 0) {
		pr_err("error reading batt id channel = %d, rc = %lld\n",
					chip->vbat_channel, rc);
		return 1;
	}
	if (rc < chip->batt_id_min || rc > chip->batt_id_max) {
		pr_err("batt_id phy =%lld is not valid\n", rc);
		return 0;
	}
	return 1;
}
static void check_battery_valid(struct pm8921_chg_chip *chip)
{
	if (is_battery_valid(chip) == 0) {
		pr_err("batt_id not valid, disbling charging\n");
		pm_chg_auto_enable(chip, 0);
	} else {
		pm_chg_auto_enable(chip, !charging_disabled);
	}
}
static void battery_id_valid(struct work_struct *work)
{
	struct pm8921_chg_chip *chip = container_of(work,
				struct pm8921_chg_chip, battery_id_valid_work);
	check_battery_valid(chip);
}
static void pm8921_chg_enable_irq(struct pm8921_chg_chip *chip, int interrupt)
{
	if (!__test_and_set_bit(interrupt, chip->enabled_irqs)) {
		dev_dbg(chip->dev, "%d\n", chip->pmic_chg_irq[interrupt]);
		enable_irq(chip->pmic_chg_irq[interrupt]);
	}
}
static void pm8921_chg_disable_irq(struct pm8921_chg_chip *chip, int interrupt)
{
	if (__test_and_clear_bit(interrupt, chip->enabled_irqs)) {
		dev_dbg(chip->dev, "%d\n", chip->pmic_chg_irq[interrupt]);
		disable_irq_nosync(chip->pmic_chg_irq[interrupt]);
	}
}
static int pm8921_chg_is_enabled(struct pm8921_chg_chip *chip, int interrupt)
{
	return test_bit(interrupt, chip->enabled_irqs);
}
static bool is_ext_charging(struct pm8921_chg_chip *chip)
{
	union power_supply_propval ret = {0,};
	if (!chip->ext_psy)
		return false;
	if (chip->ext_psy->get_property(chip->ext_psy,
					POWER_SUPPLY_PROP_CHARGE_TYPE, &ret))
		return false;
	if (ret.intval > POWER_SUPPLY_CHARGE_TYPE_NONE)
		return ret.intval;
	return false;
}
static bool is_ext_trickle_charging(struct pm8921_chg_chip *chip)
{
	union power_supply_propval ret = {0,};
	if (!chip->ext_psy)
		return false;
	if (chip->ext_psy->get_property(chip->ext_psy,
					POWER_SUPPLY_PROP_CHARGE_TYPE, &ret))
		return false;
	if (ret.intval == POWER_SUPPLY_CHARGE_TYPE_TRICKLE)
		return true;
	return false;
}
static int is_battery_charging(int fsm_state)
{
	if (!the_chip) {
		pr_err("called before init\n");
		return 0;
	}
	if (is_ext_charging(the_chip))
		return 1;
	switch (fsm_state) {
	case FSM_STATE_ATC_2A:
	case FSM_STATE_ATC_2B:
	case FSM_STATE_ON_CHG_AND_BAT_6:
	case FSM_STATE_FAST_CHG_7:
	case FSM_STATE_TRKL_CHG_8:
		return 1;
	}
	return 0;
}
static void bms_notify(struct work_struct *work)
{
	struct bms_notify *n = container_of(work, struct bms_notify, work);
	if (n->is_charging) {
		pm8921_bms_charging_began();
	} else {
		pm8921_bms_charging_end(n->is_battery_full);
		n->is_battery_full = 0;
	}
}
static void bms_notify_check(struct pm8921_chg_chip *chip)
{
	int fsm_state, new_is_charging;
	fsm_state = pm_chg_get_fsm_state(chip);
	new_is_charging = is_battery_charging(fsm_state);
	if (chip->bms_notify.is_charging ^ new_is_charging) {
		chip->bms_notify.is_charging = new_is_charging;
		schedule_work(&(chip->bms_notify.work));
	}
	ncm_notify_event(chip, NCM_EVENT_BMS_NOTIFY);
}
static enum power_supply_property pm_power_props_usb[] = {
	POWER_SUPPLY_PROP_PRESENT,
	POWER_SUPPLY_PROP_ONLINE,
	POWER_SUPPLY_PROP_CURRENT_MAX,
	POWER_SUPPLY_PROP_SCOPE,
};
static enum power_supply_property pm_power_props_mains[] = {
	POWER_SUPPLY_PROP_PRESENT,
	POWER_SUPPLY_PROP_ONLINE,
};
static char *pm_power_supplied_to[] = {
	"battery",
};
#define USB_WALL_THRESHOLD_MA	500
static int pm_power_get_property_mains(struct power_supply *psy,
				  enum power_supply_property psp,
				  union power_supply_propval *val)
{
	if (!the_chip)
		return -EINVAL;
	switch (psp) {
	case POWER_SUPPLY_PROP_PRESENT:
	case POWER_SUPPLY_PROP_ONLINE:
		val->intval = 0;
		if (charging_disabled)
			return 0;
		if (is_ext_charging(the_chip)) {
			val->intval = 1;
			return 0;
		}
		if (the_chip->dc_present) {
			val->intval = 1;
			return 0;
		}
		if (usb_target_ma > USB_WALL_THRESHOLD_MA)
			val->intval = is_usb_chg_plugged_in(the_chip);
			return 0;
		break;
	default:
		return -EINVAL;
	}
	return 0;
}
static int switch_usb_to_charge_mode(struct pm8921_chg_chip *chip)
{
	int rc;
	if (!chip->host_mode)
		return 0;
	rc = pm8xxx_writeb(chip->dev->parent, USB_OVP_TEST, 0xB2);
	if (rc < 0) {
		pr_err("Failed to write 0xB2 to USB_OVP_TEST rc = %d\n", rc);
		return rc;
	}
	chip->host_mode = 0;
	return 0;
}
static int switch_usb_to_host_mode(struct pm8921_chg_chip *chip)
{
	int rc;
	if (chip->host_mode)
		return 0;
	rc = pm8xxx_writeb(chip->dev->parent, USB_OVP_TEST, 0xB3);
	if (rc < 0) {
		pr_err("Failed to write 0xB3 to USB_OVP_TEST rc = %d\n", rc);
		return rc;
	}
	chip->host_mode = 1;
	return 0;
}
static int pm_power_set_property_usb(struct power_supply *psy,
				  enum power_supply_property psp,
				  const union power_supply_propval *val)
{
	if (!the_chip)
		return -EINVAL;
	switch (psp) {
	case POWER_SUPPLY_PROP_SCOPE:
		if (val->intval == POWER_SUPPLY_SCOPE_SYSTEM)
			return switch_usb_to_host_mode(the_chip);
		if (val->intval == POWER_SUPPLY_SCOPE_DEVICE)
			return switch_usb_to_charge_mode(the_chip);
		else
			return -EINVAL;
		break;
	default:
		return -EINVAL;
	}
	return 0;
}
static int pm_power_get_property_usb(struct power_supply *psy,
				  enum power_supply_property psp,
				  union power_supply_propval *val)
{
	int current_max;
	int usb_ovp;
	int usb_ocp;
	if (!the_chip)
		return -EINVAL;
	switch (psp) {
	case POWER_SUPPLY_PROP_CURRENT_MAX:
		pm_chg_iusbmax_get(the_chip, &current_max);
		val->intval = current_max;
		break;
	case POWER_SUPPLY_PROP_PRESENT:
	case POWER_SUPPLY_PROP_ONLINE:
		val->intval = 0;
		if (charging_disabled)
			return 0;
		getProtect_Status(IDX_USB_OVP, &usb_ovp);
		getProtect_Status(IDX_USB_OCP, &usb_ocp);
		if (usb_ovp || usb_ocp) {
			return 0;
		}
               val->intval = is_usb_chg_plugged_in(the_chip);
		break;
	case POWER_SUPPLY_PROP_SCOPE:
		if (the_chip->host_mode)
			val->intval = POWER_SUPPLY_SCOPE_SYSTEM;
		else
			val->intval = POWER_SUPPLY_SCOPE_DEVICE;
		break;
	default:
		return -EINVAL;
	}
	return 0;
}
static enum power_supply_property msm_batt_power_props[] = {
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_CHARGE_TYPE,
	POWER_SUPPLY_PROP_HEALTH,
	POWER_SUPPLY_PROP_PRESENT,
	POWER_SUPPLY_PROP_TECHNOLOGY,
	POWER_SUPPLY_PROP_VOLTAGE_MAX_DESIGN,
	POWER_SUPPLY_PROP_VOLTAGE_MIN_DESIGN,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
	POWER_SUPPLY_PROP_CAPACITY,
	POWER_SUPPLY_PROP_CURRENT_NOW,
	POWER_SUPPLY_PROP_TEMP,
	POWER_SUPPLY_PROP_TEMP_XO,
	POWER_SUPPLY_PROP_ENERGY_FULL,
};
static int qcom_get_prop_battery_uvolts(struct pm8921_chg_chip *chip)
{
	int rc;
	struct pm8xxx_adc_chan_result result;
	rc = pm8xxx_adc_read(chip->vbat_channel, &result);
	if (rc) {
		pr_err("error reading adc channel = %d, rc = %d\n",
					chip->vbat_channel, rc);
		return rc;
	}
	pr_debug("mvolts phy = %lld meas = 0x%llx\n", result.physical,
						result.measurement);
	return (int)result.physical;
}
static int get_prop_battery_uvolts(struct pm8921_chg_chip *chip)
{
	struct pm8xxx_adc_chan_result result;
	int uvolts;
	int ret;
	if (chip->ncm_chg_chip.batt_adj.enable) {
		uvolts = chip->ncm_chg_chip.batt_adj.voltage * 1000;
	}
	else {
		ret = pm8xxx_adc_read(chip->vbat_channel, &result);
		if (ret) {
			pr_err("error reading adc ret = %d\n", ret);
			return ret;
		}
		uvolts = (int)result.physical;
	}
	pr_debug("[OUT] uvolts=%d\n", uvolts);
	return uvolts;
}
static unsigned int voltage_based_capacity(struct pm8921_chg_chip *chip)
{
	unsigned int current_voltage_uv = get_prop_battery_uvolts(chip);
	unsigned int current_voltage_mv = current_voltage_uv / 1000;
	unsigned int low_voltage = chip->min_voltage_mv;
	unsigned int high_voltage = chip->max_voltage_mv;
	if (current_voltage_mv <= low_voltage)
		return 0;
	else if (current_voltage_mv >= high_voltage)
		return 100;
	else
		return (current_voltage_mv - low_voltage) * 100
		    / (high_voltage - low_voltage);
}
static int get_prop_batt_capacity(struct pm8921_chg_chip *chip)
{
	int percent_soc;
	if (chip->ncm_chg_chip.batt_adj.enable) {
		percent_soc = chip->ncm_chg_chip.batt_adj.capacity;
	}
	else {
		percent_soc = voltage_based_capacity(chip);
	}
	pr_debug("percent_soc=%d\n", percent_soc);
#ifdef CONFIG_FEATURE_NCMC_ONLY_FOR_PRODUCTION_PROCESS_8960
	if(percent_soc < 1){
		pr_debug("Factory mode. SOC = %d%% -> 1%%\n", percent_soc);
		percent_soc = 1;
	}
#else
	if(factory_mode_flag_apps){
		pr_debug("Factory mode. SOC = %d%% -> 100%%\n", percent_soc);
		percent_soc = 100;
	}
#endif
	if (percent_soc <= 10)
		pr_warn("low battery charge = %d%%\n", percent_soc);
	return percent_soc;
}
static int get_prop_batt_current(struct pm8921_chg_chip *chip)
{
	int result_ua, rc;
	rc = pm8921_bms_get_battery_current(&result_ua);
	if (rc == -ENXIO) {
		rc = pm8xxx_ccadc_get_battery_current(&result_ua);
	}
	if (rc) {
		pr_err("unable to get batt current rc = %d\n", rc);
		return rc;
	} else {
		return result_ua;
	}
}
static int get_prop_batt_fcc(struct pm8921_chg_chip *chip)
{
	int rc;
	rc = pm8921_bms_get_fcc();
	if (rc < 0)
		pr_err("unable to get batt fcc rc = %d\n", rc);
	return rc;
}
static int get_prop_batt_health(struct pm8921_chg_chip *chip)
{
#if defined(CONFIG_FEATURE_NCMC_POWER)
	int ncm_chg_protect_sts_active = 0;
#else
	int temp;
#endif
#if defined(CONFIG_FEATURE_NCMC_POWER)
	get_active_status(&ncm_chg_protect_sts_active);
	switch (ncm_chg_protect_sts_active)
	{
		case IDX_DC_OVP:
		case IDX_USB_OVP:
			return POWER_SUPPLY_HEALTH_OVERVOLTAGE;
			break;
		case IDX_USB_OCP:
			return POWER_SUPPLY_HEALTH_OVERVOLTAGE;
			break;
		case IDX_BATT_TEMP_ERR:
			return POWER_SUPPLY_HEALTH_OVERHEAT;
			break;
		case IDX_XO_TEMP:
			return POWER_SUPPLY_HEALTH_OVERHEAT;
		case IDX_CHARGE_TIMEOUT:
			return POWER_SUPPLY_HEALTH_DEAD;
			break;
			break;
	}
#else
	temp = pm_chg_get_rt_status(chip, BATTTEMP_HOT_IRQ);
	if (temp)
		return POWER_SUPPLY_HEALTH_OVERHEAT;
	temp = pm_chg_get_rt_status(chip, BATTTEMP_COLD_IRQ);
	if (temp)
		return POWER_SUPPLY_HEALTH_COLD;
#endif
	return POWER_SUPPLY_HEALTH_GOOD;
}
static int get_prop_batt_present(struct pm8921_chg_chip *chip)
{
	return pm_chg_get_rt_status(chip, BATT_INSERTED_IRQ);
}
static int get_prop_charge_type(struct pm8921_chg_chip *chip)
{
	int temp;
	if (!get_prop_batt_present(chip))
		return POWER_SUPPLY_CHARGE_TYPE_NONE;
	if (is_ext_trickle_charging(chip))
		return POWER_SUPPLY_CHARGE_TYPE_TRICKLE;
	if (is_ext_charging(chip))
		return POWER_SUPPLY_CHARGE_TYPE_FAST;
	temp = pm_chg_get_rt_status(chip, TRKLCHG_IRQ);
	if (temp)
		return POWER_SUPPLY_CHARGE_TYPE_TRICKLE;
	temp = pm_chg_get_rt_status(chip, FASTCHG_IRQ);
	if (temp)
		return POWER_SUPPLY_CHARGE_TYPE_FAST;
	return POWER_SUPPLY_CHARGE_TYPE_NONE;
}
static int get_prop_batt_status(struct pm8921_chg_chip *chip)
{
	int batt_state = POWER_SUPPLY_STATUS_DISCHARGING;
	int fsm_state = pm_chg_get_fsm_state(chip);
	int i;
	int ncm_chg_protect_sts_active = 0;
	int percent_soc = 0;
	int online = 0;
	int vbatdet_low = 0;
	get_active_status(&ncm_chg_protect_sts_active);
	switch (ncm_chg_protect_sts_active)
	{
		case IDX_DC_OVP:
		case IDX_USB_OVP:
		case IDX_USB_OCP:
		case IDX_CHARGE_TIMEOUT:
			nc_pr_debug("@@@Force Discharge\n");
			return POWER_SUPPLY_STATUS_DISCHARGING;
			break;
		default:
			break;
	}
	if (chip->ext_psy) {
		if (chip->ext_charge_done)
			return POWER_SUPPLY_STATUS_FULL;
		if (chip->ext_charging)
			return POWER_SUPPLY_STATUS_CHARGING;
	}
	for (i = 0; i < ARRAY_SIZE(map); i++)
		if (map[i].fsm_state == fsm_state)
			batt_state = map[i].batt_state;
#if defined(CONFIG_FEATURE_NCMC_POWER)
	if (fsm_state == FSM_STATE_ON_CHG_HIGHI_1) {
		if (!pm_chg_get_rt_status(chip, BATT_INSERTED_IRQ)
			|| !pm_chg_get_rt_status(chip, BAT_TEMP_OK_IRQ)
			|| pm_chg_get_rt_status(chip, CHGHOT_IRQ))
			return POWER_SUPPLY_STATUS_NOT_CHARGING;
	}
#else
	if (fsm_state == FSM_STATE_ON_CHG_HIGHI_1) {
		if (!pm_chg_get_rt_status(chip, BATT_INSERTED_IRQ)
			|| !pm_chg_get_rt_status(chip, BAT_TEMP_OK_IRQ)
			|| pm_chg_get_rt_status(chip, CHGHOT_IRQ)
			|| pm_chg_get_rt_status(chip, VBATDET_LOW_IRQ))
			batt_state = POWER_SUPPLY_STATUS_NOT_CHARGING;
	}
#endif
    if (chip->ncm_chg_chip.batt_adj.enable) {
        percent_soc = chip->ncm_chg_chip.batt_adj.capacity;
        online |= pm_chg_get_rt_status(chip, USBIN_VALID_IRQ);
        online |= pm_chg_get_rt_status(chip, DCIN_VALID_IRQ);
        vbatdet_low = pm_chg_get_rt_status(chip, VBATDET_LOW_IRQ);
        if (online) {
            if (percent_soc == 100) {
                batt_state = POWER_SUPPLY_STATUS_FULL;
            }
            else {
                switch (fsm_state) {
                    case FSM_STATE_ON_CHG_HIGHI_1:
                        if (vbatdet_low) {
                            batt_state = POWER_SUPPLY_STATUS_CHARGING;
                        }
                        else {
                            batt_state = POWER_SUPPLY_STATUS_NOT_CHARGING;
                        }
                        break;
                    case FSM_STATE_ON_BAT_3:
                    case FSM_STATE_EOC_10:
                    case FSM_STATE_FAST_CHG_7:
                        batt_state = POWER_SUPPLY_STATUS_CHARGING;
                        break;
                    default:
                        break;
                }
            }
        }
        else {
        }
    }
	return batt_state;
}
#define MAX_TOLERABLE_BATT_TEMP_DDC	680
static int get_prop_batt_temp(struct pm8921_chg_chip *chip)
{
	int rc;
	struct pm8xxx_adc_chan_result result;
	rc = pm8xxx_adc_read(chip->batt_temp_channel, &result);
	if (rc) {
		pr_err("error reading adc channel = %d, rc = %d\n",
					chip->vbat_channel, rc);
		return rc;
	}
	pr_debug("batt_temp phy = %lld meas = 0x%llx\n", result.physical,
						result.measurement);
	if (result.physical > MAX_TOLERABLE_BATT_TEMP_DDC)
			pr_err("BATT_TEMP= %d > 68degC, device will be shutdown\n",
								(int) result.physical);
	return (int)result.physical;
}
#if defined(CONFIG_FEATURE_NCMC_D121F)
#define MAX_TOLERABLE_XO_TEMP_DDC	840
#elif defined(CONFIG_FEATURE_NCMC_RUBY)
#define MAX_TOLERABLE_XO_TEMP_DDC	790
#else
#define MAX_TOLERABLE_XO_TEMP_DDC	840
#endif
static int get_prop_xo_temp(struct pm8921_chg_chip *chip)
{
	int rc;
	int xo_temp;
	struct pm8xxx_adc_chan_result result;
	rc = pm8xxx_adc_read(CHANNEL_MUXOFF, &result);
	if (rc) {
		pr_err("error reading adc channel = %d, rc = %d\n",
					CHANNEL_MUXOFF, rc);
		return rc;
	}
	pr_debug("xo_temp phy = %lld meas = 0x%llx\n", result.physical,
						result.measurement);
	xo_temp = (int)result.physical / 100;
	if (xo_temp >= MAX_TOLERABLE_XO_TEMP_DDC) {
		pr_err("XO_TEMP= %d >= %ddegC, device will be shutdown\n",
				xo_temp, MAX_TOLERABLE_XO_TEMP_DDC / 10);
	}
	return xo_temp;
}
static int pm_batt_power_get_property(struct power_supply *psy,
				       enum power_supply_property psp,
				       union power_supply_propval *val)
{
	struct pm8921_chg_chip *chip = container_of(psy, struct pm8921_chg_chip,
								batt_psy);
	switch (psp) {
	case POWER_SUPPLY_PROP_STATUS:
		val->intval = get_prop_batt_status(chip);
		break;
	case POWER_SUPPLY_PROP_CHARGE_TYPE:
		val->intval = get_prop_charge_type(chip);
		break;
	case POWER_SUPPLY_PROP_HEALTH:
		val->intval = get_prop_batt_health(chip);
		break;
	case POWER_SUPPLY_PROP_PRESENT:
		val->intval = get_prop_batt_present(chip);
		break;
	case POWER_SUPPLY_PROP_TECHNOLOGY:
		val->intval = POWER_SUPPLY_TECHNOLOGY_LION;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_MAX_DESIGN:
		val->intval = chip->max_voltage_mv * 1000;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_MIN_DESIGN:
		val->intval = chip->min_voltage_mv * 1000;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_NOW:
		val->intval = get_prop_battery_uvolts(chip);
		break;
	case POWER_SUPPLY_PROP_CAPACITY:
		val->intval = get_prop_batt_capacity(chip);
		break;
	case POWER_SUPPLY_PROP_CURRENT_NOW:
		val->intval = get_prop_batt_current(chip);
		break;
	case POWER_SUPPLY_PROP_TEMP:
		val->intval = get_prop_batt_temp(chip);
		break;
	case POWER_SUPPLY_PROP_TEMP_XO:
		val->intval = get_prop_xo_temp(chip);
		break;
	case POWER_SUPPLY_PROP_ENERGY_FULL:
		val->intval = get_prop_batt_fcc(chip) * 1000;
		break;
	default:
		return -EINVAL;
	}
	return 0;
}
static void (*notify_vbus_state_func_ptr)(int);
static int usb_chg_current;
static DEFINE_SPINLOCK(vbus_lock);
int pm8921_charger_register_vbus_sn(void (*callback)(int))
{
	pr_debug("%p\n", callback);
	notify_vbus_state_func_ptr = callback;
	return 0;
}
EXPORT_SYMBOL_GPL(pm8921_charger_register_vbus_sn);
void pm8921_charger_unregister_vbus_sn(void (*callback)(int))
{
	pr_debug("%p\n", callback);
	notify_vbus_state_func_ptr = NULL;
}
EXPORT_SYMBOL_GPL(pm8921_charger_unregister_vbus_sn);
static void notify_usb_of_the_plugin_event(int plugin)
{
	plugin = !!plugin;
	if (notify_vbus_state_func_ptr) {
		pr_debug("notifying plugin\n");
		(*notify_vbus_state_func_ptr) (plugin);
	} else {
		pr_debug("unable to notify plugin\n");
	}
}
static void __pm8921_charger_vbus_draw(unsigned int mA)
{
	int i, rc;
    ncm_usb_charger_current_present = mA;
	if (!the_chip) {
		pr_err("called before init\n");
		return;
	}
	if (mA >= 0 && mA <= 2) {
		usb_chg_current = 0;
		rc = pm_chg_iusbmax_set(the_chip, 0);
		if (rc) {
			pr_err("unable to set iusb to %d rc = %d\n", 0, rc);
		}
		rc = pm_chg_usb_suspend_enable(the_chip, 1);
		if (rc)
			pr_err("fail to set suspend bit rc=%d\n", rc);
	} else {
		rc = pm_chg_usb_suspend_enable(the_chip, 0);
		if (rc)
			pr_err("fail to reset suspend bit rc=%d\n", rc);
		for (i = ARRAY_SIZE(usb_ma_table) - 1; i >= 0; i--) {
			if (usb_ma_table[i].usb_ma <= mA)
				break;
		}
		if ((usb_ma_table[i].value & PM8917_IUSB_FINE_RES)
				&& !the_chip->iusb_fine_res)
			i--;
		if (i < 0)
			i = 0;
		rc = pm_chg_iusbmax_set(the_chip, i);
		if (rc) {
			pr_err("unable to set iusb to %d rc = %d\n", i, rc);
		}
	}
}
void pm8921_charger_vbus_draw(unsigned int mA)
{
	unsigned long flags;
	pr_debug("Enter charge=%d\n", mA);
	if (!the_chip) {
		pr_err("chip not yet initalized\n");
		return;
	}
	if (!get_prop_batt_present(the_chip)
		&& !is_dc_chg_plugged_in(the_chip)) {
		pr_err("rejected: no other power source connected\n");
		return;
	}
	if (usb_max_current && mA > usb_max_current) {
		pr_warn("restricting usb current to %d instead of %d\n",
					usb_max_current, mA);
		mA = usb_max_current;
	}
	if (usb_target_ma == 0 && mA > USB_WALL_THRESHOLD_MA)
		usb_target_ma = mA;
	spin_lock_irqsave(&vbus_lock, flags);
	if (the_chip) {
#if defined(CONFIG_FEATURE_NCMC_POWER)
		if(factory_mode_flag_apps){
			__pm8921_charger_vbus_draw(NCMC_FACTORY_MODE_CHG_CURRENT);
			printk(KERN_ERR "[PM] %s: Factory charging ibat_set\n", __func__);
		}else {
	        ncm_pm8921_charger_vbus_draw(mA);
		}
#else
		if (mA > USB_WALL_THRESHOLD_MA)
			__pm8921_charger_vbus_draw(USB_WALL_THRESHOLD_MA);
		else
			__pm8921_charger_vbus_draw(mA);
#endif
	} else {
#if defined(CONFIG_FEATURE_NCMC_POWER)
        usb_chg_current = mA;
#else
		if (mA > USB_WALL_THRESHOLD_MA)
			usb_chg_current = USB_WALL_THRESHOLD_MA;
		else
			usb_chg_current = mA;
#endif
	}
	spin_unlock_irqrestore(&vbus_lock, flags);
}
EXPORT_SYMBOL_GPL(pm8921_charger_vbus_draw);
int pm8921_charger_enable(bool enable)
{
	int rc;
	if (!the_chip) {
		pr_err("called before init\n");
		return -EINVAL;
	}
	enable = !!enable;
	rc = pm_chg_auto_enable(the_chip, enable);
	if (rc)
		pr_err("Failed rc=%d\n", rc);
	return rc;
}
EXPORT_SYMBOL(pm8921_charger_enable);
int pm8921_is_usb_chg_plugged_in(void)
{
	if (!the_chip) {
		pr_err("called before init\n");
		return -EINVAL;
	}
	return is_usb_chg_plugged_in(the_chip);
}
EXPORT_SYMBOL(pm8921_is_usb_chg_plugged_in);
int pm8921_is_dc_chg_plugged_in(void)
{
	if (!the_chip) {
		pr_err("called before init\n");
		return -EINVAL;
	}
	return is_dc_chg_plugged_in(the_chip);
}
EXPORT_SYMBOL(pm8921_is_dc_chg_plugged_in);
int pm8921_is_battery_present(void)
{
	if (!the_chip) {
		pr_err("called before init\n");
		return -EINVAL;
	}
	return get_prop_batt_present(the_chip);
}
EXPORT_SYMBOL(pm8921_is_battery_present);
int pm8921_set_vbatdet_on_pm_param_update(void)
{
	if (!the_chip) {
		pr_err("called before init\n");
		return -EINVAL;
	}
	ncm_chg_set_charging_vbatdet_eoc();
	return 0;
}
EXPORT_SYMBOL(pm8921_set_vbatdet_on_pm_param_update);
int pm8921_disable_input_current_limit(bool disable)
{
	if (!the_chip) {
		pr_err("called before init\n");
		return -EINVAL;
	}
	if (disable) {
		pr_warn("Disabling input current limit!\n");
		return pm8xxx_writeb(the_chip->dev->parent,
			 CHG_BUCK_CTRL_TEST3, 0xF2);
	}
	return 0;
}
EXPORT_SYMBOL(pm8921_disable_input_current_limit);
int pm8921_set_max_battery_charge_current(int ma)
{
	if (!the_chip) {
		pr_err("called before init\n");
		return -EINVAL;
	}
	return pm_chg_ibatmax_set(the_chip, ma);
}
EXPORT_SYMBOL(pm8921_set_max_battery_charge_current);
int pm8921_disable_source_current(bool disable)
{
	if (!the_chip) {
		pr_err("called before init\n");
		return -EINVAL;
	}
	if (disable)
		pr_warn("current drawn from chg=0, battery provides current\n");
	return pm_chg_charge_dis(the_chip, disable);
}
EXPORT_SYMBOL(pm8921_disable_source_current);
int pm8921_regulate_input_voltage(int voltage)
{
	int rc;
	if (!the_chip) {
		pr_err("called before init\n");
		return -EINVAL;
	}
	rc = pm_chg_vinmin_set(the_chip, voltage);
	if (rc == 0)
		the_chip->vin_min = voltage;
	return rc;
}
#define USB_OV_THRESHOLD_MASK  0x60
#define USB_OV_THRESHOLD_SHIFT  5
int pm8921_usb_ovp_set_threshold(enum pm8921_usb_ov_threshold ov)
{
	u8 temp;
	if (!the_chip) {
		pr_err("called before init\n");
		return -EINVAL;
	}
	if (ov > PM_USB_OV_7V) {
		pr_err("limiting to over voltage threshold to 7volts\n");
		ov = PM_USB_OV_7V;
	}
	temp = USB_OV_THRESHOLD_MASK & (ov << USB_OV_THRESHOLD_SHIFT);
	return pm_chg_masked_write(the_chip, USB_OVP_CONTROL,
				USB_OV_THRESHOLD_MASK, temp);
}
EXPORT_SYMBOL(pm8921_usb_ovp_set_threshold);
#define USB_DEBOUNCE_TIME_MASK	0x06
#define USB_DEBOUNCE_TIME_SHIFT 1
int pm8921_usb_ovp_set_hystersis(enum pm8921_usb_debounce_time ms)
{
	u8 temp;
	if (!the_chip) {
		pr_err("called before init\n");
		return -EINVAL;
	}
	if (ms > PM_USB_DEBOUNCE_80P5MS) {
		pr_err("limiting debounce to 80.5ms\n");
		ms = PM_USB_DEBOUNCE_80P5MS;
	}
	temp = USB_DEBOUNCE_TIME_MASK & (ms << USB_DEBOUNCE_TIME_SHIFT);
	return pm_chg_masked_write(the_chip, USB_OVP_CONTROL,
				USB_DEBOUNCE_TIME_MASK, temp);
}
EXPORT_SYMBOL(pm8921_usb_ovp_set_hystersis);
#define USB_OVP_DISABLE_MASK	0x80
int pm8921_usb_ovp_disable(int disable)
{
	u8 temp = 0;
	if (!the_chip) {
		pr_err("called before init\n");
		return -EINVAL;
	}
	if (disable)
		temp = USB_OVP_DISABLE_MASK;
	return pm_chg_masked_write(the_chip, USB_OVP_CONTROL,
				USB_OVP_DISABLE_MASK, temp);
}
bool pm8921_is_battery_charging(int *source)
{
	int fsm_state, is_charging, dc_present, usb_present;
	if (!the_chip) {
		pr_err("called before init\n");
		return -EINVAL;
	}
	fsm_state = pm_chg_get_fsm_state(the_chip);
	is_charging = is_battery_charging(fsm_state);
	if (is_charging == 0) {
		*source = PM8921_CHG_SRC_NONE;
		return is_charging;
	}
	if (source == NULL)
		return is_charging;
	dc_present = is_dc_chg_plugged_in(the_chip);
	usb_present = is_usb_chg_plugged_in(the_chip);
	if (dc_present && !usb_present)
		*source = PM8921_CHG_SRC_DC;
	if (usb_present && !dc_present)
		*source = PM8921_CHG_SRC_USB;
	if (usb_present && dc_present)
		*source = PM8921_CHG_SRC_DC;
	return is_charging;
}
EXPORT_SYMBOL(pm8921_is_battery_charging);
int pm8921_set_usb_power_supply_type(enum power_supply_type type)
{
	if (!the_chip) {
		pr_err("called before init\n");
		return -EINVAL;
	}
	if (type < POWER_SUPPLY_TYPE_USB)
		return -EINVAL;
#ifndef CONFIG_FEATURE_NCMC_POWER
	the_chip->usb_psy.type = type;
#endif
	power_supply_changed(&the_chip->usb_psy);
	power_supply_changed(&the_chip->dc_psy);
	return 0;
}
EXPORT_SYMBOL_GPL(pm8921_set_usb_power_supply_type);
int pm8921_batt_temperature(void)
{
	if (!the_chip) {
		pr_err("called before init\n");
		return -EINVAL;
	}
	return get_prop_batt_temp(the_chip);
}
void chg_usb_charger_connected(usb_sw_device_state_enum device_type)
{
	nc_prev_chg_type = nc_usb_device_type;
	nc_usb_device_type = device_type;
#if defined(CONFIG_FEATURE_NCMC_POWER) && !defined(CONFIG_FEATURE_NCMC_RUBY)
	if (ncm_is_initialized == false) {
        nc_pr_debug("called before init \n");
		return;
	}
	handle_usb_insertion_removal(the_chip);
	schedule_work(&the_chip->ncm_chg_chip.prtct_timer.usb_chk_work);
#endif
	return;
}
EXPORT_SYMBOL(chg_usb_charger_connected);
static void handle_usb_insertion_removal(struct pm8921_chg_chip *chip)
{
	int usb_present;
	pm_chg_failed_clear(chip, 1);
	usb_present = is_usb_chg_plugged_in(chip);
	if (chip->usb_present ^ usb_present) {
		notify_usb_of_the_plugin_event(usb_present);
		chip->usb_present = usb_present;
		power_supply_changed(&chip->usb_psy);
#if defined(CONFIG_FEATURE_NCMC_RUBY)
               power_supply_changed(&chip->dc_psy);
#endif
		power_supply_changed(&chip->batt_psy);
		pm8921_bms_calibrate_hkadc();
	}
	if (usb_present) {
		schedule_delayed_work(&chip->unplug_check_work,
			round_jiffies_relative(msecs_to_jiffies
				(UNPLUG_CHECK_WAIT_PERIOD_MS)));
#ifndef ROLLBACK_REVERSEBOOST_FIX_METHOD
		pm8921_chg_enable_irq(chip, CHG_GONE_IRQ);
#endif
	} else {
		usb_target_ma = 0;
#ifndef ROLLBACK_REVERSEBOOST_FIX_METHOD
		pm8921_chg_disable_irq(chip, CHG_GONE_IRQ);
#endif
	}
#ifndef ROLLBACK_REVERSEBOOST_FIX_METHOD
	enable_input_voltage_regulation(chip);
#endif
	bms_notify_check(chip);
}
static void handle_stop_ext_chg(struct pm8921_chg_chip *chip)
{
#if defined(CONFIG_FEATURE_NCMC_POWER)
    power_supply_changed(&chip->dc_psy);
    bms_notify_check(chip);
#else
	if (!chip->ext_psy) {
		pr_debug("external charger not registered.\n");
		return;
	}
	if (!chip->ext_charging) {
		pr_debug("already not charging.\n");
		return;
	}
	power_supply_set_charge_type(chip->ext_psy,
					POWER_SUPPLY_CHARGE_TYPE_NONE);
#if !defined(CONFIG_FEATURE_NCMC_POWER)
	pm8921_disable_source_current(false);
#endif
	power_supply_changed(&chip->dc_psy);
	chip->ext_charging = false;
	chip->ext_charge_done = false;
	bms_notify_check(chip);
#endif
}
static void handle_start_ext_chg(struct pm8921_chg_chip *chip)
{
#if !defined(CONFIG_FEATURE_NCMC_POWER)
	int dc_present;
	int batt_present;
	int batt_temp_ok;
	int vbat_ov;
#endif
    unsigned long flags;
#if defined(CONFIG_FEATURE_NCMC_POWER)
    power_supply_changed(&chip->dc_psy);
    bms_notify_check(chip);
#else
	if (!chip->ext_psy) {
		pr_debug("external charger not registered.\n");
		return;
	}
	if (chip->ext_charging) {
		pr_debug("already charging.\n");
		return;
	}
	dc_present = is_dc_chg_plugged_in(the_chip);
	batt_present = pm_chg_get_rt_status(chip, BATT_INSERTED_IRQ);
	batt_temp_ok = pm_chg_get_rt_status(chip, BAT_TEMP_OK_IRQ);
	if (!dc_present) {
		pr_warn("%s. dc not present.\n", __func__);
		return;
	}
	if (!batt_present) {
		pr_warn("%s. battery not present.\n", __func__);
		return;
	}
	if (!batt_temp_ok) {
		pr_warn("%s. battery temperature not ok.\n", __func__);
		return;
	}
#if !defined(CONFIG_FEATURE_NCMC_POWER)
	pm8921_disable_source_current(true);
#endif
	vbat_ov = pm_chg_get_rt_status(chip, VBAT_OV_IRQ);
	if (vbat_ov) {
		pr_warn("%s. battery over voltage.\n", __func__);
		return;
	}
	power_supply_set_online(chip->ext_psy, dc_present);
	power_supply_set_charge_type(chip->ext_psy,
					POWER_SUPPLY_CHARGE_TYPE_FAST);
	power_supply_changed(&chip->dc_psy);
	chip->ext_charging = true;
	chip->ext_charge_done = false;
	bms_notify_check(chip);
#endif
    spin_lock_irqsave(&chip->ncm_chg_chip.eoc_slock,flags);
    chip->ncm_chg_chip.ncm_chg_eoc_work_flag = TRUE;
    spin_unlock_irqrestore(&chip->ncm_chg_chip.eoc_slock,flags);
	wake_lock(&chip->eoc_wake_lock);
}
#ifdef ROLLBACK_REVERSEBOOST_FIX_METHOD
#define WRITE_BANK_4		0xC0
static void unplug_wrkarnd_restore_worker(struct work_struct *work)
{
	u8 temp;
	int rc;
	struct delayed_work *dwork = to_delayed_work(work);
	struct pm8921_chg_chip *chip = container_of(dwork,
				struct pm8921_chg_chip,
				unplug_wrkarnd_restore_work);
	pr_debug("restoring vin_min to %d mV\n", chip->vin_min);
	rc = pm_chg_vinmin_set(the_chip, chip->vin_min);
	temp = WRITE_BANK_4 | 0xA;
	rc = pm8xxx_writeb(chip->dev->parent, CHG_BUCK_CTRL_TEST3, temp);
	if (rc) {
		pr_err("Error %d writing %d to addr %d\n", rc,
					temp, CHG_BUCK_CTRL_TEST3);
	}
#if !defined(CONFIG_FEATURE_NCMC_POWER)
	wake_unlock(&chip->unplug_wrkarnd_restore_wake_lock);
#endif
}
#else
static void turn_off_usb_ovp_fet(struct pm8921_chg_chip *chip)
{
	u8 temp;
	int rc;
	rc = pm8xxx_writeb(chip->dev->parent, USB_OVP_TEST, 0x30);
	if (rc) {
		pr_err("Failed to write 0x30 to USB_OVP_TEST rc = %d\n", rc);
		return;
	}
	rc = pm8xxx_readb(chip->dev->parent, USB_OVP_TEST, &temp);
	if (rc) {
		pr_err("Failed to read from USB_OVP_TEST rc = %d\n", rc);
		return;
	}
	temp |= 0x81;
	rc = pm8xxx_writeb(chip->dev->parent, USB_OVP_TEST, temp);
	if (rc) {
		pr_err("Failed to write 0x%x USB_OVP_TEST rc=%d\n", temp, rc);
		return;
	}
}
static void turn_on_usb_ovp_fet(struct pm8921_chg_chip *chip)
{
	u8 temp;
	int rc;
	rc = pm8xxx_writeb(chip->dev->parent, USB_OVP_TEST, 0x30);
	if (rc) {
		pr_err("Failed to write 0x30 to USB_OVP_TEST rc = %d\n", rc);
		return;
	}
	rc = pm8xxx_readb(chip->dev->parent, USB_OVP_TEST, &temp);
	if (rc) {
		pr_err("Failed to read from USB_OVP_TEST rc = %d\n", rc);
		return;
	}
	temp &= 0xFE;
	temp |= 0x80;
	rc = pm8xxx_writeb(chip->dev->parent, USB_OVP_TEST, temp);
	if (rc) {
		pr_err("Failed to write 0x%x to USB_OVP_TEST rc = %d\n",
								temp, rc);
		return;
	}
}
static int param_open_ovp_counter = 10;
module_param(param_open_ovp_counter, int, 0644);
#define WRITE_BANK_4		0xC0
#define USB_OVP_DEBOUNCE_TIME 0x06
static void unplug_ovp_fet_open(struct pm8921_chg_chip *chip)
{
	int chg_gone, usb_chg_plugged_in;
	int count = 0;
	while (count++ < param_open_ovp_counter) {
		pm_chg_masked_write(chip, USB_OVP_CONTROL,
						USB_OVP_DEBOUNCE_TIME, 0x0);
		usleep(10);
		usb_chg_plugged_in = is_usb_chg_plugged_in(chip);
		chg_gone = pm_chg_get_rt_status(chip, CHG_GONE_IRQ);
		pr_debug("OVP FET count = %d chg_gone=%d, usb_valid = %d\n",
					count, chg_gone, usb_chg_plugged_in);
		if (chg_gone == 1 && usb_chg_plugged_in == 1) {
			pr_debug("since chg_gone = 1 dis ovp_fet for 20msec\n");
			turn_off_usb_ovp_fet(chip);
			msleep(20);
			turn_on_usb_ovp_fet(chip);
		} else {
			break;
		}
	}
	pm_chg_masked_write(chip, USB_OVP_CONTROL,
		USB_OVP_DEBOUNCE_TIME, 0x2);
	pr_debug("Exit count=%d chg_gone=%d, usb_valid=%d\n",
		count, chg_gone, usb_chg_plugged_in);
	return;
}
#endif
static int find_usb_ma_value(int value)
{
	int i;
	for (i = ARRAY_SIZE(usb_ma_table) - 1; i >= 0; i--) {
		if (value >= usb_ma_table[i].usb_ma)
			break;
	}
	return i;
}
static void decrease_usb_ma_value(int *value)
{
	int i;
	if (value) {
		i = find_usb_ma_value(*value);
		if (i > 0)
			i--;
		while (!the_chip->iusb_fine_res && i > 0
			&& (usb_ma_table[i].value & PM8917_IUSB_FINE_RES))
			i--;
		*value = usb_ma_table[i].usb_ma;
	}
}
#ifndef ROLLBACK_REVERSEBOOST_FIX_METHOD
static void increase_usb_ma_value(int *value)
{
	int i;
	if (value) {
		i = find_usb_ma_value(*value);
		if (i < (ARRAY_SIZE(usb_ma_table) - 1))
			i++;
		while (!the_chip->iusb_fine_res
			&& (usb_ma_table[i].value & PM8917_IUSB_FINE_RES)
			&& i < (ARRAY_SIZE(usb_ma_table) - 1))
			i++;
		*value = usb_ma_table[i].usb_ma;
	}
}
#endif
static void vin_collapse_check_worker(struct work_struct *work)
{
	struct delayed_work *dwork = to_delayed_work(work);
	struct pm8921_chg_chip *chip = container_of(dwork,
			struct pm8921_chg_chip, vin_collapse_check_work);
	if (is_usb_chg_plugged_in(chip) &&
		usb_target_ma > USB_WALL_THRESHOLD_MA) {
		decrease_usb_ma_value(&usb_target_ma);
		__pm8921_charger_vbus_draw(USB_WALL_THRESHOLD_MA);
		pr_debug("usb_now=%d, usb_target = %d\n",
				USB_WALL_THRESHOLD_MA, usb_target_ma);
	} else {
		handle_usb_insertion_removal(chip);
	}
}
#define VIN_MIN_COLLAPSE_CHECK_MS	50
static irqreturn_t usbin_valid_irq_handler(int irq, void *data)
{
	struct pm8921_chg_chip *chip = data;
	int usbin_valid = 0;
	int dcin_valid  = 0;
	int percent_soc = 0;
#if defined(CONFIG_FEATURE_NCMC_POWER) && !defined(CONFIG_FEATURE_NCMC_RUBY)
#else
	int usbchk_ret   = NCM_CHG_PROTECT_RET_OK;
#endif
    if (chip->ncm_chg_chip.batt_adj.enable) {
        percent_soc = chip->ncm_chg_chip.batt_adj.capacity;
        usbin_valid = pm_chg_get_rt_status(chip, USBIN_VALID_IRQ);
        dcin_valid  = pm_chg_get_rt_status(chip, DCIN_VALID_IRQ);
        if (dcin_valid) {
        }
        else {
            if (usbin_valid) {
                if (percent_soc == 100) {
                    ncm_chg_set_charging_vbatdet_eoc();
                }
                else {
                    ncm_chg_set_charging_vbatdet();
                }
            }
            else {
                ncm_chg_set_charging_vbatdet_eoc();
            }
        }
    }
#if defined(CONFIG_FEATURE_NCMC_POWER) && !defined(CONFIG_FEATURE_NCMC_RUBY)
#else
    handle_usb_insertion_removal(data);
#endif
#if defined(CONFIG_FEATURE_NCMC_POWER) && !defined(CONFIG_FEATURE_NCMC_RUBY)
#else
	usbchk_ret = ncm_chg_protect_usbin();
	if (usbchk_ret == NCM_CHG_PROTECT_RET_OK){
		check_charge_protect_sts();
	}
	else {
		printk(KERN_ERR "[PM] %s: Err!usbin protect\n", __func__);
	}
#endif
	schedule_work(&the_chip->ncm_chg_chip.prtct_timer.usb_chk_work);
    if (pm_chg_get_rt_status(chip, USBIN_VALID_IRQ)) {
        if (unplug_check_work_flag == false) {
            schedule_delayed_work(&chip->unplug_check_work,
                round_jiffies_relative(msecs_to_jiffies
                    (UNPLUG_CHECK_WAIT_PERIOD_MS)));
            unplug_check_work_flag = true;
            wake_lock(&chip->unplug_wrkarnd_restore_wake_lock);
        }
	}
    nc_pr_debug("IRQ[%d] Occurred\n", irq);
	return IRQ_HANDLED;
}
static irqreturn_t usbin_ov_irq_handler(int irq, void *data)
{
#if defined(CONFIG_FEATURE_NCMC_POWER) && !defined(CONFIG_FEATURE_NCMC_RUBY)
#else
	int usbchk_ret   = NCM_CHG_PROTECT_RET_OK;
#endif
    nc_pr_debug("IRQ[%d] Occurred\n", irq);
	pr_err("USB OverVoltage\n");
#if defined(CONFIG_FEATURE_NCMC_POWER) && !defined(CONFIG_FEATURE_NCMC_RUBY)
#else
	usbchk_ret = ncm_chg_protect_usbin();
	if (usbchk_ret == NCM_CHG_PROTECT_RET_OK){
		check_charge_protect_sts();
	}
	else {
		printk(KERN_ERR "[PM] %s: Err!usbin protect\n", __func__);
	}
#endif
	return IRQ_HANDLED;
}
static irqreturn_t batt_inserted_irq_handler(int irq, void *data)
{
	struct pm8921_chg_chip *chip = data;
	int status;
    nc_pr_debug("IRQ[%d] Occurred\n", irq);
	status = pm_chg_get_rt_status(chip, BATT_INSERTED_IRQ);
	schedule_work(&chip->battery_id_valid_work);
	handle_start_ext_chg(chip);
	pr_debug("battery present=%d", status);
	power_supply_changed(&chip->batt_psy);
	return IRQ_HANDLED;
}
static irqreturn_t vbatdet_low_irq_handler(int irq, void *data)
{
	struct pm8921_chg_chip *chip = data;
	int high_transition;
    nc_pr_debug("IRQ[%d] Occurred\n", irq);
	high_transition = pm_chg_get_rt_status(chip, VBATDET_LOW_IRQ);
	if (high_transition) {
		ncm_chg_set_charging_vbatdet();
		pm_chg_auto_enable(chip, !charging_disabled);
		pr_info("batt fell below resume voltage %s\n",
			charging_disabled ? "" : "charger enabled");
		if (ncm_is_connect_dcin() == NCM_CHG_DC_CONNECT) {
			schedule_work(&chip->ncm_chg_chip.prtct_timer.dc_chk_work);
		}else {
			if (ncm_is_connect_usbchg() == NCM_CHG_USB_CONNECT) {
				schedule_work(&chip->ncm_chg_chip.prtct_timer.usb_chk_work);
			}else {
				pr_err("Charger is not found.\n");
			}
		}
	}
	pr_debug("fsm_state=%d\n", pm_chg_get_fsm_state(data));
	power_supply_changed(&chip->batt_psy);
	power_supply_changed(&chip->usb_psy);
	power_supply_changed(&chip->dc_psy);
	return IRQ_HANDLED;
}
static irqreturn_t usbin_uv_irq_handler(int irq, void *data)
{
#if defined(CONFIG_FEATURE_NCMC_POWER) && !defined(CONFIG_FEATURE_NCMC_RUBY)
#else
	int usbchk_ret   = NCM_CHG_PROTECT_RET_OK;
#endif
    nc_pr_debug("IRQ[%d] Occurred\n", irq);
	ncm_chg_uv_flag = TRUE;
	ncm_chg_set_charging_ichgusb();
	pr_err("USB UnderVoltage\n");
#if defined(CONFIG_FEATURE_NCMC_POWER) && !defined(CONFIG_FEATURE_NCMC_RUBY)
#else
	usbchk_ret = ncm_chg_protect_usbin();
	if (usbchk_ret == NCM_CHG_PROTECT_RET_OK){
		check_charge_protect_sts();
	}
	else {
		printk(KERN_ERR "[PM] %s: Err!usbin protect\n", __func__);
	}
#endif
	return IRQ_HANDLED;
}
static irqreturn_t vbat_ov_irq_handler(int irq, void *data)
{
    nc_pr_debug("IRQ[%d] Occurred\n", irq);
	pr_debug("fsm_state=%d\n", pm_chg_get_fsm_state(data));
	return IRQ_HANDLED;
}
static irqreturn_t chgwdog_irq_handler(int irq, void *data)
{
    nc_pr_debug("IRQ[%d] Occurred\n", irq);
	pr_debug("fsm_state=%d\n", pm_chg_get_fsm_state(data));
	return IRQ_HANDLED;
}
static irqreturn_t vcp_irq_handler(int irq, void *data)
{
	nc_pr_debug("IRQ[%d] Occurred\n", irq);
#ifdef ROLLBACK_REVERSEBOOST_FIX_METHOD
	pr_warning("VCP triggered BATDET forced on\n");
	pr_debug("state_changed_to=%d\n", pm_chg_get_fsm_state(data));
#else
	pr_debug("fsm_state=%d\n", pm_chg_get_fsm_state(data));
#endif
	return IRQ_HANDLED;
}
static irqreturn_t atcdone_irq_handler(int irq, void *data)
{
    nc_pr_debug("IRQ[%d] Occurred\n", irq);
	pr_debug("fsm_state=%d\n", pm_chg_get_fsm_state(data));
	return IRQ_HANDLED;
}
static irqreturn_t atcfail_irq_handler(int irq, void *data)
{
    nc_pr_debug("IRQ[%d] Occurred\n", irq);
	pr_debug("fsm_state=%d\n", pm_chg_get_fsm_state(data));
	return IRQ_HANDLED;
}
static irqreturn_t chgdone_irq_handler(int irq, void *data)
{
	struct pm8921_chg_chip *chip = data;
    nc_pr_debug("IRQ[%d] Occurred\n", irq);
	pr_debug("state_changed_to=%d\n", pm_chg_get_fsm_state(data));
	handle_stop_ext_chg(chip);
	power_supply_changed(&chip->batt_psy);
	power_supply_changed(&chip->usb_psy);
	power_supply_changed(&chip->dc_psy);
	bms_notify_check(chip);
	return IRQ_HANDLED;
}
static irqreturn_t chgfail_irq_handler(int irq, void *data)
{
#if defined(CONFIG_FEATURE_NCMC_POWER)
	struct pm8921_chg_chip *chip = data;
	int ret;
	unsigned long flags;
	spin_lock_irqsave(&chip->ncm_chg_chip.prtct_timer.slock, flags);
	if(the_chip->ncm_chg_chip.prtct_timer.is_timeout == false) {
		nc_pr_debug("bit clear!\n");
		ret = pm_chg_failed_clear(chip, 1);
	}else{
		nc_pr_debug("bit is not clear.\n");
		ret = 0;
	}
	spin_unlock_irqrestore(&chip->ncm_chg_chip.prtct_timer.slock, flags);
#else
	struct pm8921_chg_chip *chip = data;
	int ret;
	ret = pm_chg_failed_clear(chip, 1);
#endif
	if (ret)
		pr_err("Failed to write CHG_FAILED_CLEAR bit\n");
	pr_err("batt_present = %d, batt_temp_ok = %d, state_changed_to=%d\n",
			get_prop_batt_present(chip),
			pm_chg_get_rt_status(chip, BAT_TEMP_OK_IRQ),
			pm_chg_get_fsm_state(data));
    nc_pr_debug("IRQ[%d] Occurred\n", irq);
	power_supply_changed(&chip->batt_psy);
	power_supply_changed(&chip->usb_psy);
	power_supply_changed(&chip->dc_psy);
	return IRQ_HANDLED;
}
static irqreturn_t chgstate_irq_handler(int irq, void *data)
{
	struct pm8921_chg_chip *chip = data;
    nc_pr_debug("IRQ[%d] Occurred\n", irq);
	pr_debug("state_changed_to=%d\n", pm_chg_get_fsm_state(data));
	power_supply_changed(&chip->batt_psy);
	power_supply_changed(&chip->usb_psy);
	power_supply_changed(&chip->dc_psy);
	bms_notify_check(chip);
	return IRQ_HANDLED;
}
static int param_vin_disable_counter = 5;
module_param(param_vin_disable_counter, int, 0644);
#ifndef ROLLBACK_REVERSEBOOST_FIX_METHOD
static void attempt_reverse_boost_fix(struct pm8921_chg_chip *chip,
							int count, int usb_ma)
{
	__pm8921_charger_vbus_draw(500);
	pr_debug("count = %d iusb=500mA\n", count);
	disable_input_voltage_regulation(chip);
	pr_debug("count = %d disable_input_regulation\n", count);
	msleep(20);
	pr_debug("count = %d end sleep 20ms chg_gone=%d, usb_valid = %d\n",
				count,
				pm_chg_get_rt_status(chip, CHG_GONE_IRQ),
				is_usb_chg_plugged_in(chip));
	pr_debug("count = %d restoring input regulation and usb_ma = %d\n",
		 count, usb_ma);
	enable_input_voltage_regulation(chip);
	__pm8921_charger_vbus_draw(usb_ma);
}
#endif
#define VIN_ACTIVE_BIT BIT(0)
#define UNPLUG_WRKARND_RESTORE_WAIT_PERIOD_US 200
#define VIN_MIN_INCREASE_MV 100
#ifdef ROLLBACK_REVERSEBOOST_FIX_METHOD
#if defined(CONFIG_FEATURE_NCMC_D121F)
  #define VIN_MIN_TARGET_MV 4600
#else
  #define VIN_MIN_TARGET_MV 4500
#endif
#define UNPLUGED_COUNT 25
#endif
static void unplug_check_worker(struct work_struct *work)
{
	struct delayed_work *dwork = to_delayed_work(work);
	struct pm8921_chg_chip *chip = container_of(dwork,
				struct pm8921_chg_chip, unplug_check_work);
	u8 reg_loop;
	int ibat, usb_chg_plugged_in;
	int dc_chg_plugged_in = 0;
#ifdef ROLLBACK_REVERSEBOOST_FIX_METHOD
	u16 vin_min_increase_mv = 0;
	u16 vin_min_temp_mv = 0;
	static int work_end_count;
#else
	int usb_ma;
	int chg_gone = 0;
#endif
	reg_loop = 0;
    usb_chg_plugged_in = pm_chg_get_rt_status(chip, USBIN_VALID_IRQ);
    dc_chg_plugged_in  = pm_chg_get_rt_status(chip, DCIN_VALID_IRQ);
    if (!usb_chg_plugged_in && !dc_chg_plugged_in) {
        if (work_end_count < UNPLUGED_COUNT) {
            work_end_count++;
            schedule_delayed_work(&chip->unplug_check_work,
                      round_jiffies_relative(msecs_to_jiffies
                        (UNPLUG_CHECK_WAIT_PERIOD_MS)));
            if (work_end_count == 5) {
                power_supply_changed(&chip->batt_psy);
            }
            return;
        }
        else {
            pr_debug("Stopping Unplug Check Worker since USB and DC removed"
                "reg_loop = %d, fsm = %d ibat = %d\n",
                pm_chg_get_regulation_loop(chip),
                pm_chg_get_fsm_state(chip),
                get_prop_batt_current(chip)
                );
            unplug_check_work_flag = false;
            work_end_count = 0;
            wake_unlock(&chip->unplug_wrkarnd_restore_wake_lock);
            return;
        }
    }
#ifdef ROLLBACK_REVERSEBOOST_FIX_METHOD
	work_end_count = 0;
	reg_loop = pm_chg_get_regulation_loop(chip);
	pr_debug("reg_loop=0x%x\n", reg_loop);
	if (reg_loop & VIN_ACTIVE_BIT) {
		ibat = get_prop_batt_current(chip);
		pr_debug("ibat = %d fsm = %d reg_loop = 0x%x\n",
				ibat, pm_chg_get_fsm_state(chip), reg_loop);
		if (ibat > 0) {
			int err;
			u8 temp;
			temp = WRITE_BANK_4 | 0xE;
			err = pm8xxx_writeb(chip->dev->parent,
						CHG_BUCK_CTRL_TEST3, temp);
			if (err) {
				pr_err("Error %d writing %d to addr %d\n", err,
						temp, CHG_BUCK_CTRL_TEST3);
			}
			vin_min_temp_mv = pm_chg_vinmin_get(chip);
			vin_min_increase_mv = VIN_MIN_TARGET_MV - vin_min_temp_mv;
#if defined(CONFIG_FEATURE_NCMC_POWER)
			pm_chg_vinmin_set(chip,
					chip->vin_min + vin_min_increase_mv);
#else
			pm_chg_vinmin_set(chip,
					chip->vin_min + VIN_MIN_INCREASE_MV);
#endif
#if !defined(CONFIG_FEATURE_NCMC_POWER)
			wake_lock(&chip->unplug_wrkarnd_restore_wake_lock);
#endif
			schedule_delayed_work(
				&chip->unplug_wrkarnd_restore_work,
				round_jiffies_relative(usecs_to_jiffies
				(UNPLUG_WRKARND_RESTORE_WAIT_PERIOD_US)));
		}
	}
#else
	pm_chg_iusbmax_get(chip, &usb_ma);
	if (usb_ma == 500 && !usb_target_ma) {
		pr_debug("Stopping Unplug Check Worker since USB == 500mA\n");
		disable_input_voltage_regulation(chip);
		return;
	}
	if (usb_ma <= 100) {
		pr_debug(
			"Unenumerated yet or suspended usb_ma = %d skipping\n",
			usb_ma);
		goto check_again_later;
	}
	if (pm8921_chg_is_enabled(chip, CHG_GONE_IRQ))
		pr_debug("chg gone irq is enabled\n");
	reg_loop = pm_chg_get_regulation_loop(chip);
	pr_debug("reg_loop=0x%x usb_ma = %d\n", reg_loop, usb_ma);
	if (reg_loop & VIN_ACTIVE_BIT) {
		decrease_usb_ma_value(&usb_ma);
		usb_target_ma = usb_ma;
		__pm8921_charger_vbus_draw(usb_ma);
		pr_debug("usb_now=%d, usb_target = %d\n",
			usb_ma, usb_target_ma);
	}
	reg_loop = pm_chg_get_regulation_loop(chip);
	pr_debug("reg_loop=0x%x usb_ma = %d\n", reg_loop, usb_ma);
	if (reg_loop & VIN_ACTIVE_BIT) {
		ibat = get_prop_batt_current(chip);
		pr_debug("ibat = %d fsm = %d reg_loop = 0x%x\n",
				ibat, pm_chg_get_fsm_state(chip), reg_loop);
		if (ibat > 0) {
			int count = 0;
			while (count++ < param_vin_disable_counter
					&& usb_chg_plugged_in == 1) {
				attempt_reverse_boost_fix(chip, count, usb_ma);
				usb_chg_plugged_in
					= is_usb_chg_plugged_in(chip);
			}
		}
	}
	usb_chg_plugged_in = is_usb_chg_plugged_in(chip);
	chg_gone = pm_chg_get_rt_status(chip, CHG_GONE_IRQ);
	if (chg_gone == 1  && usb_chg_plugged_in == 1) {
		pr_debug(" ver5 step: chg_gone=%d, usb_valid = %d\n",
						chg_gone, usb_chg_plugged_in);
		unplug_ovp_fet_open(chip);
	}
	if (!(reg_loop & VIN_ACTIVE_BIT)) {
		if (usb_ma < usb_target_ma) {
			increase_usb_ma_value(&usb_ma);
			__pm8921_charger_vbus_draw(usb_ma);
			pr_debug("usb_now=%d, usb_target = %d\n",
					usb_ma, usb_target_ma);
		} else {
			usb_target_ma = usb_ma;
		}
	}
check_again_later:
#endif
	schedule_delayed_work(&chip->unplug_check_work,
		      round_jiffies_relative(msecs_to_jiffies
				(UNPLUG_CHECK_WAIT_PERIOD_MS)));
}
static irqreturn_t loop_change_irq_handler(int irq, void *data)
{
	struct pm8921_chg_chip *chip = data;
    nc_pr_debug("IRQ[%d] Occurred\n", irq);
	pr_debug("fsm_state=%d reg_loop=0x%x\n",
		pm_chg_get_fsm_state(data),
		pm_chg_get_regulation_loop(data));
	schedule_work(&chip->unplug_check_work.work);
	return IRQ_HANDLED;
}
static irqreturn_t fastchg_irq_handler(int irq, void *data)
{
	struct pm8921_chg_chip *chip = data;
	int high_transition;
    unsigned long flags;
    nc_pr_debug("IRQ[%d] Occurred\n", irq);
	high_transition = pm_chg_get_rt_status(chip, FASTCHG_IRQ);
    spin_lock_irqsave(&chip->ncm_chg_chip.eoc_slock,flags);
    if (high_transition && ( chip->ncm_chg_chip.ncm_chg_eoc_work_flag == FALSE )){
		wake_lock(&chip->eoc_wake_lock);
		chip->ncm_chg_chip.ncm_chg_eoc_work_flag = TRUE;
		printk(KERN_ERR "[T][ARM]Event:0x06 Info:0x%02x%02x\n",
			ncm_chgmon_charger_state(),
			ncm_chgmon_charger_type());
        if (unplug_check_work_flag == false) {
            schedule_delayed_work(&chip->unplug_check_work,
                round_jiffies_relative(msecs_to_jiffies
                    (UNPLUG_CHECK_WAIT_PERIOD_MS)));
            unplug_check_work_flag = true;
            wake_lock(&chip->unplug_wrkarnd_restore_wake_lock);
        }
	}
    spin_unlock_irqrestore(&chip->ncm_chg_chip.eoc_slock,flags);
    if (high_transition) {
        ncm_chg_protect_timer_enable(true);
    }
	power_supply_changed(&chip->batt_psy);
	bms_notify_check(chip);
	return IRQ_HANDLED;
}
static irqreturn_t trklchg_irq_handler(int irq, void *data)
{
	struct pm8921_chg_chip *chip = data;
    nc_pr_debug("IRQ[%d] Occurred\n", irq);
	power_supply_changed(&chip->batt_psy);
	return IRQ_HANDLED;
}
static irqreturn_t batt_removed_irq_handler(int irq, void *data)
{
	struct pm8921_chg_chip *chip = data;
	nc_pr_debug("IRQ[%d] Occurred\n", irq);
	schedule_delayed_work(&chip->ncm_chg_chip.batt_removed_handler_work,
			round_jiffies_relative(msecs_to_jiffies(NCM_BATT_REMOVE_WAIT_MS)));
	return IRQ_HANDLED;
}
static irqreturn_t batttemp_hot_irq_handler(int irq, void *data)
{
	struct pm8921_chg_chip *chip = data;
    nc_pr_debug("IRQ[%d] Occurred\n", irq);
	handle_stop_ext_chg(chip);
	power_supply_changed(&chip->batt_psy);
	return IRQ_HANDLED;
}
static irqreturn_t chghot_irq_handler(int irq, void *data)
{
	struct pm8921_chg_chip *chip = data;
    nc_pr_debug("IRQ[%d] Occurred\n", irq);
	pr_debug("Chg hot fsm_state=%d\n", pm_chg_get_fsm_state(data));
	power_supply_changed(&chip->batt_psy);
	power_supply_changed(&chip->usb_psy);
	handle_stop_ext_chg(chip);
	return IRQ_HANDLED;
}
static irqreturn_t batttemp_cold_irq_handler(int irq, void *data)
{
	struct pm8921_chg_chip *chip = data;
    nc_pr_debug("IRQ[%d] Occurred\n", irq);
	pr_debug("Batt cold fsm_state=%d\n", pm_chg_get_fsm_state(data));
	handle_stop_ext_chg(chip);
	power_supply_changed(&chip->batt_psy);
	power_supply_changed(&chip->usb_psy);
	return IRQ_HANDLED;
}
static irqreturn_t chg_gone_irq_handler(int irq, void *data)
{
	struct pm8921_chg_chip *chip = data;
#ifndef ROLLBACK_REVERSEBOOST_FIX_METHOD
	int chg_gone, usb_chg_plugged_in;
	usb_chg_plugged_in = is_usb_chg_plugged_in(chip);
	chg_gone = pm_chg_get_rt_status(chip, CHG_GONE_IRQ);
	pr_debug("chg_gone=%d, usb_valid = %d\n", chg_gone, usb_chg_plugged_in);
	pr_debug("Chg gone fsm_state=%d\n", pm_chg_get_fsm_state(data));
#endif
    nc_pr_debug("IRQ[%d] Occurred\n", irq);
	power_supply_changed(&chip->batt_psy);
	power_supply_changed(&chip->usb_psy);
	return IRQ_HANDLED;
}
static irqreturn_t bat_temp_ok_irq_handler(int irq, void *data)
{
	int bat_temp_ok;
	struct pm8921_chg_chip *chip = data;
	bat_temp_ok = pm_chg_get_rt_status(chip, BAT_TEMP_OK_IRQ);
    nc_pr_debug("IRQ[%d] Occurred\n", irq);
	pr_debug("batt_temp_ok = %d fsm_state%d\n",
			 bat_temp_ok, pm_chg_get_fsm_state(data));
	if (bat_temp_ok)
		handle_start_ext_chg(chip);
	else
		handle_stop_ext_chg(chip);
	power_supply_changed(&chip->batt_psy);
	power_supply_changed(&chip->usb_psy);
	bms_notify_check(chip);
	return IRQ_HANDLED;
}
static irqreturn_t coarse_det_low_irq_handler(int irq, void *data)
{
    nc_pr_debug("IRQ[%d] Occurred\n", irq);
	pr_debug("fsm_state=%d\n", pm_chg_get_fsm_state(data));
	return IRQ_HANDLED;
}
static irqreturn_t vdd_loop_irq_handler(int irq, void *data)
{
    nc_pr_debug("IRQ[%d] Occurred\n", irq);
	pr_debug("fsm_state=%d\n", pm_chg_get_fsm_state(data));
	return IRQ_HANDLED;
}
static irqreturn_t vreg_ov_irq_handler(int irq, void *data)
{
    nc_pr_debug("IRQ[%d] Occurred\n", irq);
	pr_debug("fsm_state=%d\n", pm_chg_get_fsm_state(data));
	return IRQ_HANDLED;
}
static irqreturn_t vbatdet_irq_handler(int irq, void *data)
{
    nc_pr_debug("IRQ[%d] Occurred\n", irq);
	pr_debug("fsm_state=%d\n", pm_chg_get_fsm_state(data));
	return IRQ_HANDLED;
}
static irqreturn_t batfet_irq_handler(int irq, void *data)
{
	struct pm8921_chg_chip *chip = data;
    nc_pr_debug("IRQ[%d] Occurred\n", irq);
	pr_debug("vreg ov\n");
	power_supply_changed(&chip->batt_psy);
	return IRQ_HANDLED;
}
static irqreturn_t dcin_valid_irq_handler(int irq, void *data)
{
	struct pm8921_chg_chip *chip = data;
	int dc_present;
	int dcchk_ret   = NCM_CHG_PROTECT_RET_OK;
	int usbin_valid = 0;
	int dcin_valid  = 0;
	int percent_soc = 0;
    if (chip->ncm_chg_chip.batt_adj.enable) {
        percent_soc = chip->ncm_chg_chip.batt_adj.capacity;
        usbin_valid = pm_chg_get_rt_status(chip, USBIN_VALID_IRQ);
        dcin_valid  = pm_chg_get_rt_status(chip, DCIN_VALID_IRQ);
        if (usbin_valid) {
        }
        else {
            if (dcin_valid) {
                if (percent_soc == 100) {
                    ncm_chg_set_charging_vbatdet_eoc();
                }
                else {
                    ncm_chg_set_charging_vbatdet();
                }
            }
            else {
                ncm_chg_set_charging_vbatdet_eoc();
            }
        }
    }
    nc_pr_debug("IRQ[%d] Occurred\n", irq);
	dcchk_ret = ncm_chg_protect_dc_chg();
	if (dcchk_ret == NCM_CHG_PROTECT_RET_OK){
		check_charge_protect_sts();
	}
	else {
		printk(KERN_ERR "[PM] %s: Err!dc-sw protect\n", __func__);
	}
	schedule_work(&the_chip->ncm_chg_chip.prtct_timer.dc_chk_work);
	dc_present = pm_chg_get_rt_status(chip, DCIN_VALID_IRQ);
	if (chip->ext_psy)
		power_supply_set_online(chip->ext_psy, dc_present);
	chip->dc_present = dc_present;
	if (dc_present)
		handle_start_ext_chg(chip);
	else
		handle_stop_ext_chg(chip);
    if (pm_chg_get_rt_status(chip, DCIN_VALID_IRQ)) {
        if (unplug_check_work_flag == false) {
            schedule_delayed_work(&chip->unplug_check_work,
                round_jiffies_relative(msecs_to_jiffies
                    (UNPLUG_CHECK_WAIT_PERIOD_MS)));
            unplug_check_work_flag = true;
            wake_lock(&chip->unplug_wrkarnd_restore_wake_lock);
        }
	}
	return IRQ_HANDLED;
}
static irqreturn_t dcin_ov_irq_handler(int irq, void *data)
{
	struct pm8921_chg_chip *chip = data;
	int dcchk_ret   = NCM_CHG_PROTECT_RET_OK;
    nc_pr_debug("IRQ[%d] Occurred\n", irq);
	dcchk_ret = ncm_chg_protect_dc_chg();
	if (dcchk_ret == NCM_CHG_PROTECT_RET_OK){
		check_charge_protect_sts();
	}
	else {
		printk(KERN_ERR "[PM] %s: Err!dc-sw protect\n", __func__);
	}
	handle_stop_ext_chg(chip);
	return IRQ_HANDLED;
}
static irqreturn_t dcin_uv_irq_handler(int irq, void *data)
{
	struct pm8921_chg_chip *chip = data;
	int dcchk_ret   = NCM_CHG_PROTECT_RET_OK;
    nc_pr_debug("IRQ[%d] Occurred\n", irq);
	dcchk_ret = ncm_chg_protect_dc_chg();
	if (dcchk_ret == NCM_CHG_PROTECT_RET_OK){
		check_charge_protect_sts();
	}
	else {
		printk(KERN_ERR "[PM] %s: Err!dc-sw protect\n", __func__);
	}
	handle_stop_ext_chg(chip);
	return IRQ_HANDLED;
}
static int __pm_batt_external_power_changed_work(struct device *dev, void *data)
{
	struct power_supply *psy = &the_chip->batt_psy;
	struct power_supply *epsy = dev_get_drvdata(dev);
	int i, dcin_irq;
	if (!the_chip->ext_psy) {
		dcin_irq = the_chip->pmic_chg_irq[DCIN_VALID_IRQ];
		for (i = 0; i < epsy->num_supplicants; i++) {
			if (!strncmp(epsy->supplied_to[i], psy->name, 7)) {
				if (!strncmp(epsy->name, "dc", 2)) {
					the_chip->ext_psy = epsy;
					dcin_valid_irq_handler(dcin_irq,
							the_chip);
				}
			}
		}
	}
	return 0;
}
static void pm_batt_external_power_changed(struct power_supply *psy)
{
	if (!the_chip->ext_psy)
		class_for_each_device(power_supply_class, NULL, psy,
					 __pm_batt_external_power_changed_work);
}
static void update_heartbeat(struct work_struct *work)
{
}
#define VDD_LOOP_ACTIVE_BIT	BIT(3)
#define VDD_MAX_INCREASE_MV	400
static int vdd_max_increase_mv = VDD_MAX_INCREASE_MV;
module_param(vdd_max_increase_mv, int, 0644);
static int ichg_threshold_ua = -400000;
module_param(ichg_threshold_ua, int, 0644);
enum {
	CHG_IN_PROGRESS,
	CHG_NOT_IN_PROGRESS,
	CHG_FINISHED,
};
#define VBAT_TOLERANCE_MV	70
#define CHG_DISABLE_MSLEEP	100
static int is_charging_finished(struct pm8921_chg_chip *chip)
{
#if defined(CONFIG_FEATURE_NCMC_POWER)
	int vbat_meas_uv, vbat_meas_mv, vbat_programmed;
	int ichg_meas_ma, iterm_programmed;
	int regulation_loop, fast_chg, vcp;
	int rc;
	static int last_vbat_programmed = -EINVAL;
#else
	int vbat_meas_uv, vbat_meas_mv, vbat_programmed, vbatdet_low;
	int ichg_meas_ma, iterm_programmed;
	int regulation_loop, fast_chg, vcp;
	int rc;
	static int last_vbat_programmed = -EINVAL;
#endif
	if (!is_ext_charging(chip)) {
		fast_chg = pm_chg_get_rt_status(chip, FASTCHG_IRQ);
		pr_debug("fast_chg = %d\n", fast_chg);
		if (fast_chg == 0)
			return CHG_NOT_IN_PROGRESS;
		vcp = pm_chg_get_rt_status(chip, VCP_IRQ);
		pr_debug("vcp = %d\n", vcp);
		if (vcp == 1)
			return CHG_IN_PROGRESS;
#if defined(CONFIG_FEATURE_NCMC_POWER)
#else
		vbatdet_low = pm_chg_get_rt_status(chip, VBATDET_LOW_IRQ);
		pr_debug("vbatdet_low = %d\n", vbatdet_low);
		if (vbatdet_low == 1)
			return CHG_IN_PROGRESS;
#endif
		rc = pm_chg_get_rt_status(chip, BAT_TEMP_OK_IRQ);
		pr_debug("batt_temp_ok = %d\n", rc);
		if (rc == 0)
			return CHG_IN_PROGRESS;
		vbat_meas_uv = qcom_get_prop_battery_uvolts(chip);
		if (vbat_meas_uv < 0)
			return CHG_IN_PROGRESS;
		vbat_meas_mv = vbat_meas_uv / 1000;
		rc = pm_chg_vddmax_get(chip, &vbat_programmed);
		if (rc) {
			pr_err("couldnt read vddmax rc = %d\n", rc);
			return CHG_IN_PROGRESS;
		}
		pr_debug("vddmax = %d vbat_meas_mv=%d\n",
			 vbat_programmed, vbat_meas_mv);
		if (last_vbat_programmed == -EINVAL)
			last_vbat_programmed = vbat_programmed;
		if (last_vbat_programmed !=  vbat_programmed) {
			pr_debug("vddmax = %d last_vdd_max=%d\n",
				 vbat_programmed, last_vbat_programmed);
			last_vbat_programmed = vbat_programmed;
			return CHG_IN_PROGRESS;
		}
		regulation_loop = pm_chg_get_regulation_loop(chip);
		if (regulation_loop < 0) {
			pr_err("couldnt read the regulation loop err=%d\n",
				regulation_loop);
			return CHG_IN_PROGRESS;
		}
		pr_debug("regulation_loop=%d\n", regulation_loop);
		if (regulation_loop != 0 && regulation_loop != VDD_LOOP)
			return CHG_IN_PROGRESS;
	}
	rc = pm_chg_iterm_get(chip, &iterm_programmed);
	if (rc) {
		pr_err("couldnt read iterm rc = %d\n", rc);
		return CHG_IN_PROGRESS;
	}
	ichg_meas_ma = (get_prop_batt_current(chip)) / 1000;
	pr_debug("iterm_programmed = %d ichg_meas_ma=%d\n",
				iterm_programmed, ichg_meas_ma);
	if (ichg_meas_ma > 0)
		return CHG_IN_PROGRESS;
	if (ichg_meas_ma * -1 > iterm_programmed)
		return CHG_IN_PROGRESS;
    if (chip->ncm_chg_chip.batt_adj.enable) {
        if(chip->ncm_chg_chip.batt_adj.capacity < 100)
            return CHG_IN_PROGRESS;
	}
	return CHG_FINISHED;
}
#define CONSECUTIVE_COUNT	3
static void eoc_worker(struct pm8921_chg_chip *chip)
{
	static int count;
	int end = is_charging_finished(chip);
	unsigned long flags;
	if (end == CHG_NOT_IN_PROGRESS) {
			count = 0;
			wake_unlock(&chip->eoc_wake_lock);
			printk(KERN_ERR "[T][ARM]Event:0x07 Info:0x%02x%02x\n",
				ncm_chgmon_charger_state(),
				ncm_chgmon_charger_type());
			ncm_chg_protect_timer_enable(false);
			return;
	}
	if (end == CHG_FINISHED) {
		count++;
	} else {
		count = 0;
	}
	if (count == CONSECUTIVE_COUNT) {
		count = 0;
		pr_info("End of Charging\n");
		printk(KERN_ERR "[T][ARM]Event:0x08 Info:0x%02x%02x\n",
			ncm_chgmon_charger_state(),
			ncm_chgmon_charger_type());
		ncm_chg_set_charging_vbatdet_eoc();
		pm_chg_auto_enable(chip, 0);
		if (is_ext_charging(chip))
			chip->ext_charge_done = true;
		if (chip->is_bat_warm || chip->is_bat_cool)
			chip->bms_notify.is_battery_full = 0;
		else
			chip->bms_notify.is_battery_full = 1;
		chgdone_irq_handler(chip->pmic_chg_irq[CHGDONE_IRQ], chip);
		wake_unlock(&chip->eoc_wake_lock);
		ncm_chg_protect_timer_enable(false);
	} else {
		pr_debug("EOC count = %d\n", count);
	    spin_lock_irqsave(&chip->ncm_chg_chip.eoc_slock,flags);
	    chip->ncm_chg_chip.ncm_chg_eoc_work_flag = TRUE;
	    spin_unlock_irqrestore(&chip->ncm_chg_chip.eoc_slock,flags);
	}
}
static void btm_configure_work(struct work_struct *work)
{
	int rc;
	rc = pm8xxx_adc_btm_configure(&btm_config);
	if (rc)
		pr_err("failed to configure btm rc=%d", rc);
}
DECLARE_WORK(btm_config_work, btm_configure_work);
static void set_appropriate_battery_current(struct pm8921_chg_chip *chip)
{
	unsigned int chg_current = chip->max_bat_chg_current;
	if (chip->is_bat_cool)
		chg_current = min(chg_current, chip->cool_bat_chg_current);
	if (chip->is_bat_warm)
		chg_current = min(chg_current, chip->warm_bat_chg_current);
	if (thermal_mitigation != 0 && chip->thermal_mitigation)
		chg_current = min(chg_current,
				chip->thermal_mitigation[thermal_mitigation]);
	pm_chg_ibatmax_set(the_chip, chg_current);
}
#define TEMP_HYSTERISIS_DEGC 2
static void battery_cool(bool enter)
{
	pr_debug("enter = %d\n", enter);
	if (enter == the_chip->is_bat_cool)
		return;
	the_chip->is_bat_cool = enter;
	if (enter) {
		btm_config.low_thr_temp =
			the_chip->cool_temp_dc + TEMP_HYSTERISIS_DEGC;
		set_appropriate_battery_current(the_chip);
		pm_chg_vddmax_set(the_chip, the_chip->cool_bat_voltage);
		pm_chg_vbatdet_set(the_chip,
			the_chip->cool_bat_voltage
			- the_chip->resume_voltage_delta);
	} else {
		btm_config.low_thr_temp = the_chip->cool_temp_dc;
		set_appropriate_battery_current(the_chip);
		pm_chg_vddmax_set(the_chip, the_chip->max_voltage_mv);
		pm_chg_vbatdet_set(the_chip,
			the_chip->max_voltage_mv
			- the_chip->resume_voltage_delta);
	}
	schedule_work(&btm_config_work);
}
static void battery_warm(bool enter)
{
	pr_debug("enter = %d\n", enter);
	if (enter == the_chip->is_bat_warm)
		return;
	the_chip->is_bat_warm = enter;
	if (enter) {
		btm_config.high_thr_temp =
			the_chip->warm_temp_dc - TEMP_HYSTERISIS_DEGC;
		set_appropriate_battery_current(the_chip);
		pm_chg_vddmax_set(the_chip, the_chip->warm_bat_voltage);
		pm_chg_vbatdet_set(the_chip,
			the_chip->warm_bat_voltage
			- the_chip->resume_voltage_delta);
	} else {
		btm_config.high_thr_temp = the_chip->warm_temp_dc;
		set_appropriate_battery_current(the_chip);
		pm_chg_vddmax_set(the_chip, the_chip->max_voltage_mv);
		pm_chg_vbatdet_set(the_chip,
			the_chip->max_voltage_mv
			- the_chip->resume_voltage_delta);
	}
	schedule_work(&btm_config_work);
}
static int configure_btm(struct pm8921_chg_chip *chip)
{
	int rc;
	if (chip->warm_temp_dc != INT_MIN)
		btm_config.btm_warm_fn = battery_warm;
	else
		btm_config.btm_warm_fn = NULL;
	if (chip->cool_temp_dc != INT_MIN)
		btm_config.btm_cool_fn = battery_cool;
	else
		btm_config.btm_cool_fn = NULL;
	btm_config.low_thr_temp = chip->cool_temp_dc;
	btm_config.high_thr_temp = chip->warm_temp_dc;
	btm_config.interval = chip->temp_check_period;
	rc = pm8xxx_adc_btm_configure(&btm_config);
	if (rc)
		pr_err("failed to configure btm rc = %d\n", rc);
#if defined(CONFIG_FEATURE_NCMC_POWER)
		pm8xxx_adc_btm_end();
#else
		rc = pm8xxx_adc_btm_start();
		if (rc)
			pr_err("failed to start btm rc = %d\n", rc);
#endif
	return rc;
}
static int set_disable_status_param(const char *val, struct kernel_param *kp)
{
	int ret;
	struct pm8921_chg_chip *chip = the_chip;
	ret = param_set_int(val, kp);
	if (ret) {
		pr_err("error setting value %d\n", ret);
		return ret;
	}
	pr_info("factory set disable param to %d\n", charging_disabled);
	if (chip) {
		pm_chg_auto_enable(chip, !charging_disabled);
		pm_chg_charge_dis(chip, charging_disabled);
	}
	return 0;
}
module_param_call(disabled, set_disable_status_param, param_get_uint,
					&charging_disabled, 0644);
static int rconn_mohm;
static int set_rconn_mohm(const char *val, struct kernel_param *kp)
{
	int ret;
	struct pm8921_chg_chip *chip = the_chip;
	ret = param_set_int(val, kp);
	if (ret) {
		pr_err("error setting value %d\n", ret);
		return ret;
	}
	if (chip)
		chip->rconn_mohm = rconn_mohm;
	return 0;
}
module_param_call(rconn_mohm, set_rconn_mohm, param_get_uint,
					&rconn_mohm, 0644);
static int set_therm_mitigation_level(const char *val, struct kernel_param *kp)
{
	int ret;
	struct pm8921_chg_chip *chip = the_chip;
	ret = param_set_int(val, kp);
	if (ret) {
		pr_err("error setting value %d\n", ret);
		return ret;
	}
	if (!chip) {
		pr_err("called before init\n");
		return -EINVAL;
	}
	if (!chip->thermal_mitigation) {
		pr_err("no thermal mitigation\n");
		return -EINVAL;
	}
	if (thermal_mitigation < 0
		|| thermal_mitigation >= chip->thermal_levels) {
		pr_err("out of bound level selected\n");
		return -EINVAL;
	}
	if(factory_mode_flag_apps) {
		nc_pr_debug("Skip therm_mitigation by Factory mode.\n");
		return 0;
	}
	set_appropriate_battery_current(chip);
	return ret;
}
module_param_call(thermal_mitigation, set_therm_mitigation_level,
					param_get_uint,
					&thermal_mitigation, 0644);
static int set_usb_max_current(const char *val, struct kernel_param *kp)
{
	int ret, mA;
	struct pm8921_chg_chip *chip = the_chip;
	ret = param_set_int(val, kp);
	if (ret) {
		pr_err("error setting value %d\n", ret);
		return ret;
	}
	if (chip) {
		pr_warn("setting current max to %d\n", usb_max_current);
		pm_chg_iusbmax_get(chip, &mA);
		if (mA > usb_max_current)
			pm8921_charger_vbus_draw(usb_max_current);
		return 0;
	}
	return -EINVAL;
}
module_param_call(usb_max_current, set_usb_max_current,
	param_get_uint, &usb_max_current, 0644);
static int nc_set_logging_param(const char *val, struct kernel_param *kp)
{
	int ret;
	ret = param_set_int(val, kp);
	if (ret) {
		pr_err("error setting value %d\n", ret);
		return ret;
	}
	pr_info("logging param to %d\n", nc_prdebug_enabled);
	return 0;
}
module_param_call(nc_prdebug, nc_set_logging_param, param_get_uint,
					&nc_prdebug_enabled, 0664);
static void free_irqs(struct pm8921_chg_chip *chip)
{
	int i;
	for (i = 0; i < PM_CHG_MAX_INTS; i++)
		if (chip->pmic_chg_irq[i]) {
			free_irq(chip->pmic_chg_irq[i], chip);
			chip->pmic_chg_irq[i] = 0;
		}
}
static void __devinit determine_initial_state(struct pm8921_chg_chip *chip)
{
	unsigned long flags;
	int fsm_state;
	chip->dc_present = !!is_dc_chg_plugged_in(chip);
	chip->usb_present = !!is_usb_chg_plugged_in(chip);
	notify_usb_of_the_plugin_event(chip->usb_present);
	if (chip->usb_present) {
		schedule_delayed_work(&chip->unplug_check_work,
			round_jiffies_relative(msecs_to_jiffies
				(UNPLUG_CHECK_WAIT_PERIOD_MS)));
#ifndef ROLLBACK_REVERSEBOOST_FIX_METHOD
		pm8921_chg_enable_irq(chip, CHG_GONE_IRQ);
#endif
	}
	pm8921_chg_enable_irq(chip, DCIN_VALID_IRQ);
	pm8921_chg_enable_irq(chip, USBIN_VALID_IRQ);
	pm8921_chg_enable_irq(chip, BATT_REMOVED_IRQ);
	pm8921_chg_enable_irq(chip, BATT_INSERTED_IRQ);
	pm8921_chg_enable_irq(chip, USBIN_OV_IRQ);
	pm8921_chg_enable_irq(chip, USBIN_UV_IRQ);
	pm8921_chg_enable_irq(chip, DCIN_OV_IRQ);
	pm8921_chg_enable_irq(chip, DCIN_UV_IRQ);
	pm8921_chg_enable_irq(chip, CHGFAIL_IRQ);
	pm8921_chg_enable_irq(chip, FASTCHG_IRQ);
	pm8921_chg_enable_irq(chip, VBATDET_LOW_IRQ);
	pm8921_chg_enable_irq(chip, BAT_TEMP_OK_IRQ);
    #ifdef NC_CHGDONE_ENABLE
    pm8921_chg_enable_irq(chip, CHGDONE_IRQ);
    #endif
	spin_lock_irqsave(&vbus_lock, flags);
	if (usb_chg_current) {
		if(factory_mode_flag_apps) {
			__pm8921_charger_vbus_draw(NCMC_FACTORY_MODE_CHG_CURRENT);
		}
		else {
	        ncm_pm8921_charger_vbus_draw(usb_chg_current);
		}
		fastchg_irq_handler(chip->pmic_chg_irq[FASTCHG_IRQ], chip);
	}
	spin_unlock_irqrestore(&vbus_lock, flags);
	fsm_state = pm_chg_get_fsm_state(chip);
	if (is_battery_charging(fsm_state)) {
		chip->bms_notify.is_charging = 1;
		pm8921_bms_charging_began();
		ncm_notify_event(chip, NCM_EVENT_BMS_NOTIFY);
	}
	nc_pr_debug("Charger tiemr_check.\n");
	if (ncm_is_connect_dcin() == NCM_CHG_DC_CONNECT) {
		schedule_work(&chip->ncm_chg_chip.prtct_timer.dc_chk_work);
	}else {
		if (ncm_is_connect_usbchg() == NCM_CHG_USB_CONNECT) {
			schedule_work(&chip->ncm_chg_chip.prtct_timer.usb_chk_work);
		}else {
			nc_pr_debug("Charger is not found.\n");
		}
	}
	check_battery_valid(chip);
	pr_debug("usb = %d, dc = %d  batt = %d state=%d\n",
			chip->usb_present,
			chip->dc_present,
			get_prop_batt_present(chip),
			fsm_state);
}
struct pm_chg_irq_init_data {
	unsigned int	irq_id;
	char		*name;
	unsigned long	flags;
	irqreturn_t	(*handler)(int, void *);
};
#define CHG_IRQ(_id, _flags, _handler) \
{ \
	.irq_id		= _id, \
	.name		= #_id, \
	.flags		= _flags, \
	.handler	= _handler, \
}
struct pm_chg_irq_init_data chg_irq_data[] = {
	CHG_IRQ(USBIN_VALID_IRQ, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
						usbin_valid_irq_handler),
	CHG_IRQ(USBIN_OV_IRQ, IRQF_TRIGGER_RISING, usbin_ov_irq_handler),
	CHG_IRQ(BATT_INSERTED_IRQ, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
						batt_inserted_irq_handler),
	CHG_IRQ(VBATDET_LOW_IRQ, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
						vbatdet_low_irq_handler),
	CHG_IRQ(USBIN_UV_IRQ, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
							usbin_uv_irq_handler),
	CHG_IRQ(VBAT_OV_IRQ, IRQF_TRIGGER_RISING, vbat_ov_irq_handler),
	CHG_IRQ(CHGWDOG_IRQ, IRQF_TRIGGER_RISING, chgwdog_irq_handler),
	CHG_IRQ(VCP_IRQ, IRQF_TRIGGER_RISING, vcp_irq_handler),
	CHG_IRQ(ATCDONE_IRQ, IRQF_TRIGGER_RISING, atcdone_irq_handler),
	CHG_IRQ(ATCFAIL_IRQ, IRQF_TRIGGER_RISING, atcfail_irq_handler),
	CHG_IRQ(CHGDONE_IRQ, IRQF_TRIGGER_RISING, chgdone_irq_handler),
	CHG_IRQ(CHGFAIL_IRQ, IRQF_TRIGGER_RISING, chgfail_irq_handler),
	CHG_IRQ(CHGSTATE_IRQ, IRQF_TRIGGER_RISING, chgstate_irq_handler),
	CHG_IRQ(LOOP_CHANGE_IRQ, IRQF_TRIGGER_RISING, loop_change_irq_handler),
	CHG_IRQ(FASTCHG_IRQ, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
						fastchg_irq_handler),
	CHG_IRQ(TRKLCHG_IRQ, IRQF_TRIGGER_RISING, trklchg_irq_handler),
	CHG_IRQ(BATT_REMOVED_IRQ, IRQF_TRIGGER_RISING,
						batt_removed_irq_handler),
	CHG_IRQ(BATTTEMP_HOT_IRQ, IRQF_TRIGGER_RISING,
						batttemp_hot_irq_handler),
	CHG_IRQ(CHGHOT_IRQ, IRQF_TRIGGER_RISING, chghot_irq_handler),
	CHG_IRQ(BATTTEMP_COLD_IRQ, IRQF_TRIGGER_RISING,
						batttemp_cold_irq_handler),
#ifdef ROLLBACK_REVERSEBOOST_FIX_METHOD
	CHG_IRQ(CHG_GONE_IRQ, IRQF_TRIGGER_RISING, chg_gone_irq_handler),
#else
	CHG_IRQ(CHG_GONE_IRQ, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
						chg_gone_irq_handler),
#endif
	CHG_IRQ(BAT_TEMP_OK_IRQ, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
						bat_temp_ok_irq_handler),
	CHG_IRQ(COARSE_DET_LOW_IRQ, IRQF_TRIGGER_RISING,
						coarse_det_low_irq_handler),
	CHG_IRQ(VDD_LOOP_IRQ, IRQF_TRIGGER_RISING, vdd_loop_irq_handler),
	CHG_IRQ(VREG_OV_IRQ, IRQF_TRIGGER_RISING, vreg_ov_irq_handler),
	CHG_IRQ(VBATDET_IRQ, IRQF_TRIGGER_RISING, vbatdet_irq_handler),
	CHG_IRQ(BATFET_IRQ, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
						batfet_irq_handler),
	CHG_IRQ(DCIN_VALID_IRQ, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
						dcin_valid_irq_handler),
	CHG_IRQ(DCIN_OV_IRQ, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
						dcin_ov_irq_handler),
	CHG_IRQ(DCIN_UV_IRQ, IRQF_TRIGGER_RISING, dcin_uv_irq_handler),
};
static int __devinit request_irqs(struct pm8921_chg_chip *chip,
					struct platform_device *pdev)
{
	struct resource *res;
	int ret, i;
	unsigned long flags;
	ret = 0;
	bitmap_fill(chip->enabled_irqs, PM_CHG_MAX_INTS);
	spin_lock_irqsave(&chip->ncm_chg_chip.irq_slock, flags);
	for (i = 0; i < ARRAY_SIZE(chg_irq_data); i++) {
		res = platform_get_resource_byname(pdev, IORESOURCE_IRQ,
				chg_irq_data[i].name);
		if (res == NULL) {
			pr_err("couldn't find %s\n", chg_irq_data[i].name);
			goto err_out;
		}
		chip->pmic_chg_irq[chg_irq_data[i].irq_id] = res->start;
		ret = request_irq(res->start, chg_irq_data[i].handler,
			chg_irq_data[i].flags,
			chg_irq_data[i].name, chip);
		if (ret < 0) {
			pr_err("couldn't request %d (%s) %d\n", res->start,
					chg_irq_data[i].name, ret);
			chip->pmic_chg_irq[chg_irq_data[i].irq_id] = 0;
			goto err_out;
		}
		pm8921_chg_disable_irq(chip, chg_irq_data[i].irq_id);
	}
	spin_unlock_irqrestore(&chip->ncm_chg_chip.irq_slock, flags);
	return 0;
err_out:
	free_irqs(chip);
	spin_unlock_irqrestore(&chip->ncm_chg_chip.irq_slock, flags);
	return -EINVAL;
}
static void pm8921_chg_force_19p2mhz_clk(struct pm8921_chg_chip *chip)
{
	int err;
	u8 temp;
	temp  = 0xD1;
	err = pm8xxx_writeb(chip->dev->parent, CHG_TEST, temp);
	if (err) {
		pr_err("Error %d writing %d to addr %d\n", err, temp, CHG_TEST);
		return;
	}
	temp  = 0xD3;
	err = pm8xxx_writeb(chip->dev->parent, CHG_TEST, temp);
	if (err) {
		pr_err("Error %d writing %d to addr %d\n", err, temp, CHG_TEST);
		return;
	}
	temp  = 0xD1;
	err = pm8xxx_writeb(chip->dev->parent, CHG_TEST, temp);
	if (err) {
		pr_err("Error %d writing %d to addr %d\n", err, temp, CHG_TEST);
		return;
	}
	temp  = 0xD5;
	err = pm8xxx_writeb(chip->dev->parent, CHG_TEST, temp);
	if (err) {
		pr_err("Error %d writing %d to addr %d\n", err, temp, CHG_TEST);
		return;
	}
	udelay(183);
	temp  = 0xD1;
	err = pm8xxx_writeb(chip->dev->parent, CHG_TEST, temp);
	if (err) {
		pr_err("Error %d writing %d to addr %d\n", err, temp, CHG_TEST);
		return;
	}
	temp  = 0xD0;
	err = pm8xxx_writeb(chip->dev->parent, CHG_TEST, temp);
	if (err) {
		pr_err("Error %d writing %d to addr %d\n", err, temp, CHG_TEST);
		return;
	}
	udelay(32);
	temp  = 0xD1;
	err = pm8xxx_writeb(chip->dev->parent, CHG_TEST, temp);
	if (err) {
		pr_err("Error %d writing %d to addr %d\n", err, temp, CHG_TEST);
		return;
	}
	temp  = 0xD3;
	err = pm8xxx_writeb(chip->dev->parent, CHG_TEST, temp);
	if (err) {
		pr_err("Error %d writing %d to addr %d\n", err, temp, CHG_TEST);
		return;
	}
}
static void pm8921_chg_set_hw_clk_switching(struct pm8921_chg_chip *chip)
{
	int err;
	u8 temp;
	temp  = 0xD1;
	err = pm8xxx_writeb(chip->dev->parent, CHG_TEST, temp);
	if (err) {
		pr_err("Error %d writing %d to addr %d\n", err, temp, CHG_TEST);
		return;
	}
	temp  = 0xD0;
	err = pm8xxx_writeb(chip->dev->parent, CHG_TEST, temp);
	if (err) {
		pr_err("Error %d writing %d to addr %d\n", err, temp, CHG_TEST);
		return;
	}
}
#define ENUM_TIMER_STOP_BIT	BIT(1)
#define BOOT_DONE_BIT		BIT(6)
#define CHG_BATFET_ON_BIT	BIT(3)
#define CHG_VCP_EN		BIT(0)
#define CHG_BAT_TEMP_DIS_BIT	BIT(2)
#define SAFE_CURRENT_MA		1500
#define VREF_BATT_THERM_FORCE_ON	BIT(7)
static int __devinit pm8921_chg_hw_init(struct pm8921_chg_chip *chip)
{
	int rc;
	int vdd_safe;
	pm8921_chg_force_19p2mhz_clk(chip);
	rc = pm_chg_masked_write(chip, SYS_CONFIG_2,
					BOOT_DONE_BIT, BOOT_DONE_BIT);
	if (rc) {
		pr_err("Failed to set BOOT_DONE_BIT rc=%d\n", rc);
		return rc;
	}
	vdd_safe = chip->max_voltage_mv + VDD_MAX_INCREASE_MV;
	if (vdd_safe > PM8921_CHG_VDDSAFE_MAX)
		vdd_safe = PM8921_CHG_VDDSAFE_MAX;
	rc = pm_chg_vddsafe_set(chip, vdd_safe);
	if (rc) {
		pr_err("Failed to set safe voltage to %d rc=%d\n",
						chip->max_voltage_mv, rc);
		return rc;
	}
#if defined(CONFIG_FEATURE_NCMC_POWER)
	rc = pm_chg_vbatdet_set(chip,
				PM8921_CHG_VBATDET_MAX);
	if (rc) {
		pr_err("Failed to set vbatdet comprator voltage to %d rc=%d\n",
			PM8921_CHG_VBATDET_MAX, rc);
		return rc;
	}
#else
	rc = pm_chg_vbatdet_set(chip,
				chip->max_voltage_mv
				- chip->resume_voltage_delta);
	if (rc) {
		pr_err("Failed to set vbatdet comprator voltage to %d rc=%d\n",
			chip->max_voltage_mv - chip->resume_voltage_delta, rc);
		return rc;
	}
#endif
	rc = pm_chg_vddmax_set(chip, chip->max_voltage_mv);
	if (rc) {
		pr_err("Failed to set max voltage to %d rc=%d\n",
						chip->max_voltage_mv, rc);
		return rc;
	}
	rc = pm_chg_ibatsafe_set(chip, SAFE_CURRENT_MA);
	if (rc) {
		pr_err("Failed to set max voltage to %d rc=%d\n",
						SAFE_CURRENT_MA, rc);
		return rc;
	}
	rc = pm_chg_ibatmax_set(chip, chip->max_bat_chg_current);
	if (rc) {
		pr_err("Failed to set max current to 400 rc=%d\n", rc);
		return rc;
	}
	rc = pm_chg_iterm_set(chip, chip->term_current);
	if (rc) {
		pr_err("Failed to set term current to %d rc=%d\n",
						chip->term_current, rc);
		return rc;
	}
#if defined(CONFIG_FEATURE_NCMC_POWER)
	if(!factory_mode_flag_apps) {
		rc = pm_chg_iusbmax_set(chip, 0);
		if (rc) {
			pr_err("Failed to set usb max to %d rc=%d\n", 0, rc);
			return rc;
		}
	}
#endif
	rc = pm_chg_masked_write(chip, PBL_ACCESS2, ENUM_TIMER_STOP_BIT,
			ENUM_TIMER_STOP_BIT);
	if (rc) {
		pr_err("Failed to set enum timer stop rc=%d\n", rc);
		return rc;
	}
	if (chip->safety_time != 0) {
		rc = pm_chg_tchg_max_set(chip, chip->safety_time);
		if (rc) {
			pr_err("Failed to set max time to %d minutes rc=%d\n",
							chip->safety_time, rc);
			return rc;
		}
	}
	if (chip->ttrkl_time != 0) {
		rc = pm_chg_ttrkl_max_set(chip, chip->ttrkl_time);
		if (rc) {
			pr_err("Failed to set trkl time to %d minutes rc=%d\n",
							chip->safety_time, rc);
			return rc;
		}
	}
	if (chip->vin_min != 0) {
		rc = pm_chg_vinmin_set(chip, chip->vin_min);
		if (rc) {
			pr_err("Failed to set vin min to %d mV rc=%d\n",
							chip->vin_min, rc);
			return rc;
		}
	} else {
		chip->vin_min = pm_chg_vinmin_get(chip);
	}
	rc = pm_chg_disable_wd(chip);
	if (rc) {
		pr_err("Failed to disable wd rc=%d\n", rc);
		return rc;
	}
#if defined(CONFIG_FEATURE_NCMC_POWER)
	rc = pm_chg_masked_write(chip, CHG_CNTRL_2,
			CHG_BAT_TEMP_DIS_BIT, CHG_BAT_TEMP_DIS_BIT);
#else
	rc = pm_chg_masked_write(chip, CHG_CNTRL_2,
				CHG_BAT_TEMP_DIS_BIT, 0);
#endif
	if (rc) {
		pr_err("Failed to enable temp control chg rc=%d\n", rc);
		return rc;
	}
	rc = pm8xxx_writeb(chip->dev->parent, CHG_BUCK_CLOCK_CTRL, 0x15);
	if (rc) {
		pr_err("Failed to switch buck clk rc=%d\n", rc);
		return rc;
	}
	if (chip->trkl_voltage != 0) {
		rc = pm_chg_vtrkl_low_set(chip, chip->trkl_voltage);
		if (rc) {
			pr_err("Failed to set trkl voltage to %dmv  rc=%d\n",
							chip->trkl_voltage, rc);
			return rc;
		}
	}
	if (chip->weak_voltage != 0) {
		rc = pm_chg_vweak_set(chip, chip->weak_voltage);
		if (rc) {
			pr_err("Failed to set weak voltage to %dmv  rc=%d\n",
							chip->weak_voltage, rc);
			return rc;
		}
	}
	if (chip->trkl_current != 0) {
		rc = pm_chg_itrkl_set(chip, chip->trkl_current);
		if (rc) {
			pr_err("Failed to set trkl current to %dmA  rc=%d\n",
							chip->trkl_voltage, rc);
			return rc;
		}
	}
	if (chip->weak_current != 0) {
		rc = pm_chg_iweak_set(chip, chip->weak_current);
		if (rc) {
			pr_err("Failed to set weak current to %dmA  rc=%d\n",
							chip->weak_current, rc);
			return rc;
		}
	}
	rc = pm_chg_batt_cold_temp_config(chip, chip->cold_thr);
	if (rc) {
		pr_err("Failed to set cold config %d  rc=%d\n",
						chip->cold_thr, rc);
	}
	rc = pm_chg_batt_hot_temp_config(chip, chip->hot_thr);
	if (rc) {
		pr_err("Failed to set hot config %d  rc=%d\n",
						chip->hot_thr, rc);
	}
	if (pm8xxx_get_revision(chip->dev->parent) < PM8XXX_REVISION_8921_2p0) {
		pm8xxx_writeb(chip->dev->parent, CHG_BUCK_CTRL_TEST2, 0xF1);
		pm8xxx_writeb(chip->dev->parent, CHG_BUCK_CTRL_TEST3, 0xCE);
		pm8xxx_writeb(chip->dev->parent, CHG_BUCK_CTRL_TEST3, 0xD8);
		pm8xxx_writeb(chip->dev->parent, PSI_TXRX_SAMPLE_DATA_0, 0xFF);
		pm8xxx_writeb(chip->dev->parent, PSI_TXRX_SAMPLE_DATA_1, 0xFF);
		pm8xxx_writeb(chip->dev->parent, PSI_TXRX_SAMPLE_DATA_2, 0xFF);
		pm8xxx_writeb(chip->dev->parent, PSI_TXRX_SAMPLE_DATA_3, 0xFF);
		pm8xxx_writeb(chip->dev->parent, PSI_CONFIG_STATUS, 0x0D);
		udelay(100);
		pm8xxx_writeb(chip->dev->parent, PSI_CONFIG_STATUS, 0x0C);
	}
	if (pm8xxx_get_revision(chip->dev->parent) == PM8XXX_REVISION_8921_3p0)
		pm8xxx_writeb(chip->dev->parent, CHG_BUCK_CTRL_TEST3, 0xAC);
	if (pm8xxx_get_version(chip->dev->parent) == PM8XXX_VERSION_8917)
		chip->iusb_fine_res = true;
	pm8xxx_writeb(chip->dev->parent, CHG_BUCK_CTRL_TEST3, 0xD9);
	pm8xxx_writeb(chip->dev->parent, CHG_BUCK_CTRL_TEST3, 0x91);
	rc = pm_chg_masked_write(chip, CHG_CNTRL, VREF_BATT_THERM_FORCE_ON,
						VREF_BATT_THERM_FORCE_ON);
	if (rc)
		pr_err("Failed to Force Vref therm rc=%d\n", rc);
	rc = pm_chg_charge_dis(chip, charging_disabled);
	if (rc) {
		pr_err("Failed to disable CHG_CHARGE_DIS bit rc=%d\n", rc);
		return rc;
	}
	rc = pm_chg_auto_enable(chip, !charging_disabled);
	if (rc) {
		pr_err("Failed to enable charging rc=%d\n", rc);
		return rc;
	}
	return 0;
}
static int get_rt_status(void *data, u64 * val)
{
	int i = (int)data;
	int ret;
	ret = pm_chg_get_rt_status(the_chip, i);
	*val = ret;
	return 0;
}
DEFINE_SIMPLE_ATTRIBUTE(rt_fops, get_rt_status, NULL, "%llu\n");
static int get_fsm_status(void *data, u64 * val)
{
	u8 temp;
	temp = pm_chg_get_fsm_state(the_chip);
	*val = temp;
	return 0;
}
DEFINE_SIMPLE_ATTRIBUTE(fsm_fops, get_fsm_status, NULL, "%llu\n");
static int get_reg_loop(void *data, u64 * val)
{
	u8 temp;
	if (!the_chip) {
		pr_err("%s called before init\n", __func__);
		return -EINVAL;
	}
	temp = pm_chg_get_regulation_loop(the_chip);
	*val = temp;
	return 0;
}
DEFINE_SIMPLE_ATTRIBUTE(reg_loop_fops, get_reg_loop, NULL, "0x%02llx\n");
static int get_reg(void *data, u64 * val)
{
	int addr = (int)data;
	int ret;
	u8 temp;
	ret = pm8xxx_readb(the_chip->dev->parent, addr, &temp);
	if (ret) {
		pr_err("pm8xxx_readb to %x value =%d errored = %d\n",
			addr, temp, ret);
		return -EAGAIN;
	}
	*val = temp;
	return 0;
}
static int set_reg(void *data, u64 val)
{
	int addr = (int)data;
	int ret;
	u8 temp;
	temp = (u8) val;
	ret = pm8xxx_writeb(the_chip->dev->parent, addr, temp);
	if (ret) {
		pr_err("pm8xxx_writeb to %x value =%d errored = %d\n",
			addr, temp, ret);
		return -EAGAIN;
	}
	return 0;
}
DEFINE_SIMPLE_ATTRIBUTE(reg_fops, get_reg, set_reg, "0x%02llx\n");
enum {
	BAT_WARM_ZONE,
	BAT_COOL_ZONE,
};
static int get_warm_cool(void *data, u64 * val)
{
	if (!the_chip) {
		pr_err("%s called before init\n", __func__);
		return -EINVAL;
	}
	if ((int)data == BAT_WARM_ZONE)
		*val = the_chip->is_bat_warm;
	if ((int)data == BAT_COOL_ZONE)
		*val = the_chip->is_bat_cool;
	return 0;
}
DEFINE_SIMPLE_ATTRIBUTE(warm_cool_fops, get_warm_cool, NULL, "0x%lld\n");
static void create_debugfs_entries(struct pm8921_chg_chip *chip)
{
	int i;
	chip->dent = debugfs_create_dir("pm8921_chg", NULL);
	if (IS_ERR(chip->dent)) {
		pr_err("pmic charger couldnt create debugfs dir\n");
		return;
	}
	debugfs_create_file("CHG_CNTRL", 0644, chip->dent,
			    (void *)CHG_CNTRL, &reg_fops);
	debugfs_create_file("CHG_CNTRL_2", 0644, chip->dent,
			    (void *)CHG_CNTRL_2, &reg_fops);
	debugfs_create_file("CHG_CNTRL_3", 0644, chip->dent,
			    (void *)CHG_CNTRL_3, &reg_fops);
	debugfs_create_file("PBL_ACCESS1", 0644, chip->dent,
			    (void *)PBL_ACCESS1, &reg_fops);
	debugfs_create_file("PBL_ACCESS2", 0644, chip->dent,
			    (void *)PBL_ACCESS2, &reg_fops);
	debugfs_create_file("SYS_CONFIG_1", 0644, chip->dent,
			    (void *)SYS_CONFIG_1, &reg_fops);
	debugfs_create_file("SYS_CONFIG_2", 0644, chip->dent,
			    (void *)SYS_CONFIG_2, &reg_fops);
	debugfs_create_file("CHG_VDD_MAX", 0644, chip->dent,
			    (void *)CHG_VDD_MAX, &reg_fops);
	debugfs_create_file("CHG_VDD_SAFE", 0644, chip->dent,
			    (void *)CHG_VDD_SAFE, &reg_fops);
	debugfs_create_file("CHG_VBAT_DET", 0644, chip->dent,
			    (void *)CHG_VBAT_DET, &reg_fops);
	debugfs_create_file("CHG_IBAT_MAX", 0644, chip->dent,
			    (void *)CHG_IBAT_MAX, &reg_fops);
	debugfs_create_file("CHG_IBAT_SAFE", 0644, chip->dent,
			    (void *)CHG_IBAT_SAFE, &reg_fops);
	debugfs_create_file("CHG_VIN_MIN", 0644, chip->dent,
			    (void *)CHG_VIN_MIN, &reg_fops);
	debugfs_create_file("CHG_VTRICKLE", 0644, chip->dent,
			    (void *)CHG_VTRICKLE, &reg_fops);
	debugfs_create_file("CHG_ITRICKLE", 0644, chip->dent,
			    (void *)CHG_ITRICKLE, &reg_fops);
	debugfs_create_file("CHG_ITERM", 0644, chip->dent,
			    (void *)CHG_ITERM, &reg_fops);
	debugfs_create_file("CHG_TCHG_MAX", 0644, chip->dent,
			    (void *)CHG_TCHG_MAX, &reg_fops);
	debugfs_create_file("CHG_TWDOG", 0644, chip->dent,
			    (void *)CHG_TWDOG, &reg_fops);
	debugfs_create_file("CHG_TEMP_THRESH", 0644, chip->dent,
			    (void *)CHG_TEMP_THRESH, &reg_fops);
	debugfs_create_file("CHG_COMP_OVR", 0644, chip->dent,
			    (void *)CHG_COMP_OVR, &reg_fops);
	debugfs_create_file("CHG_BUCK_CTRL_TEST1", 0644, chip->dent,
			    (void *)CHG_BUCK_CTRL_TEST1, &reg_fops);
	debugfs_create_file("CHG_BUCK_CTRL_TEST2", 0644, chip->dent,
			    (void *)CHG_BUCK_CTRL_TEST2, &reg_fops);
	debugfs_create_file("CHG_BUCK_CTRL_TEST3", 0644, chip->dent,
			    (void *)CHG_BUCK_CTRL_TEST3, &reg_fops);
	debugfs_create_file("CHG_TEST", 0644, chip->dent,
			    (void *)CHG_TEST, &reg_fops);
	debugfs_create_file("FSM_STATE", 0644, chip->dent, NULL,
			    &fsm_fops);
	debugfs_create_file("REGULATION_LOOP_CONTROL", 0644, chip->dent, NULL,
			    &reg_loop_fops);
	debugfs_create_file("BAT_WARM_ZONE", 0644, chip->dent,
				(void *)BAT_WARM_ZONE, &warm_cool_fops);
	debugfs_create_file("BAT_COOL_ZONE", 0644, chip->dent,
				(void *)BAT_COOL_ZONE, &warm_cool_fops);
	for (i = 0; i < ARRAY_SIZE(chg_irq_data); i++) {
		if (chip->pmic_chg_irq[chg_irq_data[i].irq_id])
			debugfs_create_file(chg_irq_data[i].name, 0444,
				chip->dent,
				(void *)chg_irq_data[i].irq_id,
				&rt_fops);
	}
}
static int pm8921_charger_suspend_noirq(struct device *dev)
{
	int rc;
	struct pm8921_chg_chip *chip = dev_get_drvdata(dev);
	rc = pm_chg_masked_write(chip, CHG_CNTRL, VREF_BATT_THERM_FORCE_ON, 0);
	if (rc)
		pr_err("Failed to Force Vref therm off rc=%d\n", rc);
	pm8921_chg_set_hw_clk_switching(chip);
	return 0;
}
static int pm8921_charger_resume_noirq(struct device *dev)
{
	int rc;
	struct pm8921_chg_chip *chip = dev_get_drvdata(dev);
	pm8921_chg_force_19p2mhz_clk(chip);
	rc = pm_chg_masked_write(chip, CHG_CNTRL, VREF_BATT_THERM_FORCE_ON,
						VREF_BATT_THERM_FORCE_ON);
	if (rc)
		pr_err("Failed to Force Vref therm on rc=%d\n", rc);
	return 0;
}
static int pm8921_charger_resume(struct device *dev)
{
	int rc;
	struct pm8921_chg_chip *chip = dev_get_drvdata(dev);
	battim_event_type event = {
		.event_id = BATTIM_KERN_EVENT_WAKEUP,
		.error = 0,
	};
	chip->ncm_chg_chip.jiff_on_resume = jiffies;
	wake_lock(&chip->ncm_chg_chip.waiting_wlock);
	battim_set_kern_event(&event);
	if (!(chip->cool_temp_dc == INT_MIN && chip->warm_temp_dc == INT_MIN)
		&& !(chip->keep_btm_on_suspend)) {
		rc = pm8xxx_adc_btm_configure(&btm_config);
		if (rc)
			pr_err("couldn't reconfigure btm rc=%d\n", rc);
#if defined(CONFIG_FEATURE_NCMC_POWER)
#else
		rc = pm8xxx_adc_btm_start();
		if (rc)
			pr_err("couldn't restart btm rc=%d\n", rc);
#endif
	}
	if (pm8921_chg_is_enabled(chip, LOOP_CHANGE_IRQ)) {
		disable_irq_wake(chip->pmic_chg_irq[LOOP_CHANGE_IRQ]);
		pm8921_chg_disable_irq(chip, LOOP_CHANGE_IRQ);
	}
	return 0;
}
static int pm8921_charger_suspend(struct device *dev)
{
	int rc;
	struct pm8921_chg_chip *chip = dev_get_drvdata(dev);
	if (!(chip->cool_temp_dc == INT_MIN && chip->warm_temp_dc == INT_MIN)
		&& !(chip->keep_btm_on_suspend)) {
		rc = pm8xxx_adc_btm_end();
		if (rc)
			pr_err("Failed to disable BTM on suspend rc=%d\n", rc);
	}
    if(pm_chg_get_rt_status(chip, USBIN_VALID_IRQ)){
		pm8921_chg_enable_irq(chip, LOOP_CHANGE_IRQ);
		enable_irq_wake(chip->pmic_chg_irq[LOOP_CHANGE_IRQ]);
	}
	chip->ncm_chg_chip.time.on_suspend = alarm_get_elapsed_realtime();
	if (ncm_debug_mask & NCM_DEBUG_TIMER)
		pr_debug("time=%lld.%03d sec\n",
			NCM_KTIME_TO_SEC(chip->ncm_chg_chip.time.on_suspend),
			NCM_KTIME_TVNSEC_TO_MSEC(chip->ncm_chg_chip.time.on_suspend));
	ncm_timer_notify_event(chip, NCM_TMR_EVENT__SLP_ENTERING);
	return 0;
}
static int __devinit pm8921_charger_probe(struct platform_device *pdev)
{
	int rc = 0;
	struct pm8921_chg_chip *chip;
	const struct pm8921_charger_platform_data *pdata
				= pdev->dev.platform_data;
	if (!pdata) {
		pr_err("missing platform data\n");
		return -EINVAL;
	}
	chip = kzalloc(sizeof(struct pm8921_chg_chip),
					GFP_KERNEL);
	if (!chip) {
		pr_err("Cannot allocate pm_chg_chip\n");
		return -ENOMEM;
	}
	chip->dev = &pdev->dev;
	chip->safety_time = pdata->safety_time;
	chip->ttrkl_time = pdata->ttrkl_time;
	chip->update_time = pdata->update_time;
	chip->max_voltage_mv = pdata->max_voltage;
	chip->min_voltage_mv = pdata->min_voltage;
	chip->resume_voltage_delta = pdata->resume_voltage_delta;
	chip->term_current = pdata->term_current;
	chip->vbat_channel = pdata->charger_cdata.vbat_channel;
	chip->batt_temp_channel = pdata->charger_cdata.batt_temp_channel;
	chip->batt_id_channel = pdata->charger_cdata.batt_id_channel;
	chip->batt_id_min = pdata->batt_id_min;
	chip->batt_id_max = pdata->batt_id_max;
	if (pdata->cool_temp != INT_MIN)
		chip->cool_temp_dc = pdata->cool_temp * 10;
	else
		chip->cool_temp_dc = INT_MIN;
	if (pdata->warm_temp != INT_MIN)
		chip->warm_temp_dc = pdata->warm_temp * 10;
	else
		chip->warm_temp_dc = INT_MIN;
	chip->temp_check_period = pdata->temp_check_period;
	chip->max_bat_chg_current = pdata->max_bat_chg_current;
	chip->cool_bat_chg_current = pdata->cool_bat_chg_current;
	chip->warm_bat_chg_current = pdata->warm_bat_chg_current;
	chip->cool_bat_voltage = pdata->cool_bat_voltage;
	chip->warm_bat_voltage = pdata->warm_bat_voltage;
	chip->keep_btm_on_suspend = pdata->keep_btm_on_suspend;
	chip->trkl_voltage = pdata->trkl_voltage;
	chip->weak_voltage = pdata->weak_voltage;
	chip->trkl_current = pdata->trkl_current;
	chip->weak_current = pdata->weak_current;
	chip->vin_min = pdata->vin_min;
	chip->thermal_mitigation = pdata->thermal_mitigation;
	chip->thermal_levels = pdata->thermal_levels;
	chip->cold_thr = pdata->cold_thr;
	chip->hot_thr = pdata->hot_thr;
	chip->rconn_mohm = pdata->rconn_mohm;
	chip->ncm_chg_chip.prtct_timer.cnt          = 0;
	chip->ncm_chg_chip.prtct_timer.is_timeout   = false;
	chip->ncm_chg_chip.prtct_timer.is_timer     = false;
#if !defined(CONFIG_FEATURE_NCMC_ONLY_FOR_PRODUCTION_PROCESS_8960)
	if ( p_smem_id_vendor0 == NULL )
	{
	    p_smem_id_vendor0 = (smem_id_vendor0 *)(smem_find(SMEM_ID_VENDOR0, sizeof(smem_id_vendor0)));
	    if( p_smem_id_vendor0!=NULL )
	    {
	        factory_mode_flag_apps = p_smem_id_vendor0->flg_info.flash_factory_flg;
	        printk("%s: Success smem_find(SMEM_ID_VENDOR0), factory_mode_flag_apps:%d\n", __func__, factory_mode_flag_apps);
	    }
	    else
	    {
	         pr_err("%s: FAIL: smem_find(SMEM_ID_VENDOR0)\n", __func__);
	    }
	}
#endif
	rc = pm8921_chg_hw_init(chip);
	if (rc) {
		pr_err("couldn't init hardware rc=%d\n", rc);
		goto free_chip;
	}
	chip->usb_psy.name = "usb",
	chip->usb_psy.type = POWER_SUPPLY_TYPE_USB,
	chip->usb_psy.supplied_to = pm_power_supplied_to,
	chip->usb_psy.num_supplicants = ARRAY_SIZE(pm_power_supplied_to),
	chip->usb_psy.properties = pm_power_props_usb,
	chip->usb_psy.num_properties = ARRAY_SIZE(pm_power_props_usb),
	chip->usb_psy.get_property = pm_power_get_property_usb,
	chip->usb_psy.set_property = pm_power_set_property_usb,
	chip->dc_psy.name = "pm8921-dc",
	chip->dc_psy.type = POWER_SUPPLY_TYPE_MAINS,
	chip->dc_psy.supplied_to = pm_power_supplied_to,
	chip->dc_psy.num_supplicants = ARRAY_SIZE(pm_power_supplied_to),
	chip->dc_psy.properties = pm_power_props_mains,
	chip->dc_psy.num_properties = ARRAY_SIZE(pm_power_props_mains),
	chip->dc_psy.get_property = pm_power_get_property_mains,
	chip->batt_psy.name = "battery",
	chip->batt_psy.type = POWER_SUPPLY_TYPE_BATTERY,
	chip->batt_psy.properties = msm_batt_power_props,
	chip->batt_psy.num_properties = ARRAY_SIZE(msm_batt_power_props),
	chip->batt_psy.get_property = pm_batt_power_get_property,
	chip->batt_psy.external_power_changed = pm_batt_external_power_changed,
	rc = power_supply_register(chip->dev, &chip->usb_psy);
	if (rc < 0) {
		pr_err("power_supply_register usb failed rc = %d\n", rc);
		goto free_chip;
	}
	rc = power_supply_register(chip->dev, &chip->dc_psy);
	if (rc < 0) {
		pr_err("power_supply_register usb failed rc = %d\n", rc);
		goto unregister_usb;
	}
	rc = power_supply_register(chip->dev, &chip->batt_psy);
	if (rc < 0) {
		pr_err("power_supply_register batt failed rc = %d\n", rc);
		goto unregister_dc;
	}
	platform_set_drvdata(pdev, chip);
	the_chip = chip;
	wake_lock_init(&chip->eoc_wake_lock, WAKE_LOCK_SUSPEND, "pm8921_eoc");
#ifdef ROLLBACK_REVERSEBOOST_FIX_METHOD
	wake_lock_init(&chip->unplug_wrkarnd_restore_wake_lock,
			WAKE_LOCK_SUSPEND, "pm8921_unplug_wrkarnd");
#endif
	INIT_DELAYED_WORK(&chip->vin_collapse_check_work,
						vin_collapse_check_worker);
#ifdef ROLLBACK_REVERSEBOOST_FIX_METHOD
	INIT_DELAYED_WORK(&chip->unplug_wrkarnd_restore_work,
					unplug_wrkarnd_restore_worker);
#endif
	INIT_DELAYED_WORK(&chip->unplug_check_work, unplug_check_worker);
	spin_lock_init(&chip->ncm_chg_chip.irq_slock);
	rc = request_irqs(chip, pdev);
	if (rc) {
		pr_err("couldn't register interrupts rc=%d\n", rc);
		goto unregister_batt;
	}
	enable_irq_wake(chip->pmic_chg_irq[USBIN_VALID_IRQ]);
	enable_irq_wake(chip->pmic_chg_irq[USBIN_OV_IRQ]);
	enable_irq_wake(chip->pmic_chg_irq[USBIN_UV_IRQ]);
	enable_irq_wake(chip->pmic_chg_irq[BAT_TEMP_OK_IRQ]);
	enable_irq_wake(chip->pmic_chg_irq[VBATDET_LOW_IRQ]);
	enable_irq_wake(chip->pmic_chg_irq[FASTCHG_IRQ]);
	enable_irq_wake(chip->pmic_chg_irq[DCIN_VALID_IRQ]);
	enable_irq_wake(chip->pmic_chg_irq[DCIN_OV_IRQ]);
	enable_irq_wake(chip->pmic_chg_irq[DCIN_UV_IRQ]);
	if (!(chip->cool_temp_dc == INT_MIN && chip->warm_temp_dc == INT_MIN)) {
		rc = configure_btm(chip);
		if (rc) {
			pr_err("couldn't register with btm rc=%d\n", rc);
			goto free_irq;
		}
	}
	create_debugfs_entries(chip);
	INIT_WORK(&chip->bms_notify.work, bms_notify);
	INIT_WORK(&chip->battery_id_valid_work, battery_id_valid);
	wake_lock_init(&cut_off_delay_wl,   WAKE_LOCK_SUSPEND, "ncm_chg_cut_off");
	chip->ncm_chg_chip.batt_adj.enable = false;
	mutex_init(&chip->ncm_chg_chip.batt_adj.lock);
	spin_lock_init(&chip->ncm_chg_chip.timer_slock);
	spin_lock_init(&chip->ncm_chg_chip.timer_queue.slock);
	spin_lock_init(&chip->ncm_chg_chip.event_queue.slock);
	spin_lock_init(&chip->ncm_chg_chip.prtct_timer.slock);
	init_charge_protect();
	wake_lock_init(&chip->ncm_chg_chip.timer_event_wlock,  WAKE_LOCK_SUSPEND, "ncm_chg_timer_e");
	wake_lock_init(&chip->ncm_chg_chip.timer_action_wlock, WAKE_LOCK_SUSPEND, "ncm_chg_timer_a");
	wake_lock_init(&chip->ncm_chg_chip.event_wlock,        WAKE_LOCK_SUSPEND, "ncm_chg_event");
	wake_lock_init(&chip->ncm_chg_chip.waiting_wlock,      WAKE_LOCK_SUSPEND, "ncm_chg_waiting");
	INIT_WORK(&chip->ncm_chg_chip.update_batt_work, ncm_update_batt_work);
	INIT_WORK(&chip->ncm_chg_chip.watch_chg_work, ncm_watch_chg_work);
	INIT_WORK(&chip->ncm_chg_chip.timer_work, ncm_timer_process_events);
	INIT_WORK(&chip->ncm_chg_chip.notify_work, ncm_process_events);
	INIT_WORK(&chip->ncm_chg_chip.prtct_timer.dc_chk_work, ncm_chg_protect_timer_dc_chk_work);
	INIT_WORK(&chip->ncm_chg_chip.prtct_timer.usb_chk_work, ncm_chg_protect_timer_usb_chk_work);
	INIT_DELAYED_WORK(&chip->ncm_chg_chip.time_keep_work, ncm_time_keep_work);
	INIT_DELAYED_WORK(&chip->ncm_chg_chip.prtct_timer.worker, ncm_chg_protect_timer_worker);
	INIT_DELAYED_WORK(&chip->ncm_chg_chip.batt_removed_handler_work, ncm_batt_removed_handler_func);
	battim_init_pm_param();
	battim_event_connect(BATTIM_EVENT_KIND_USER, BATTIM_USER_EVENT_FIN_BATTCAL,            ncm_notify_user_event);
	battim_event_connect(BATTIM_EVENT_KIND_USER, BATTIM_USER_EVENT_FIN_UPDATE_OBS_MDM,     ncm_notify_user_event);
	battim_event_connect(BATTIM_EVENT_KIND_USER, BATTIM_USER_EVENT_REQ_UPDATE_BATTCAL_VAL, ncm_notify_user_event);
	battim_event_connect(BATTIM_EVENT_KIND_USER, BATTIM_USER_EVENT_REQ_UPDATE_CHGMON_VAL,  ncm_notify_user_event);
	battim_event_connect(BATTIM_EVENT_KIND_USER, BATTIM_USER_EVENT_REQ_UPDATE_CHGPROT_VAL, ncm_notify_user_event);
	battim_event_connect(BATTIM_EVENT_KIND_USER, BATTIM_USER_EVENT_FIN_UPDATE_PARAM,       ncm_notify_user_event);
	battim_event_connect(BATTIM_EVENT_KIND_USER, BATTIM_USER_EVENT_RESUME,                 ncm_notify_user_event);
	ncm_timer_notify_event(chip, NCM_TMR_EVENT__INIT);
    spin_lock_init(&chip->ncm_chg_chip.eoc_slock);
	ncm_is_initialized = true;
	pr_info("[PM] ncm_is_initialized:%d.\n", ncm_is_initialized);
	determine_initial_state(chip);
	rc = pm8xxx_batt_alarm_disable(PM8XXX_BATT_ALARM_LOWER_COMPARATOR);
	if (rc) {
		pr_err("%s:BATT_ALARM_LOWER unable to set disable state\n", __func__);
		goto free_irq;
	}
	if (chip->update_time) {
		INIT_DELAYED_WORK(&chip->update_heartbeat_work,
							update_heartbeat);
		schedule_delayed_work(&chip->update_heartbeat_work,
				      round_jiffies_relative(msecs_to_jiffies
							(chip->update_time)));
	}
	{
		unsigned int boot_reason;
		unsigned int status_len = sizeof(unsigned int);
		boot_reason = *(unsigned int *)
			(smem_find(SMEM_POWER_ON_STATUS_INFO, status_len));
		if (boot_reason & NCM_PWR_ON_EVENT_KEYPAD) {
			printk(KERN_ERR "[T][ARM]Event:0x80 Info:0x00\n");
		}
		if (boot_reason & NCM_PWR_ON_EVENT_USB_CHG) {
			printk(KERN_ERR "[T][ARM]Event:0x81 Info:0x00\n");
		}
		if (boot_reason & NCM_PWR_ON_EVENT_SMPL) {
			printk(KERN_ERR "[T][ARM]Event:0x83 Info:0x00\n");
		}
		if (boot_reason & NCM_PWR_ON_EVENT_HARD_RESET) {
			printk(KERN_ERR "[T][ARM]Event:0x85 Info:0x00\n");
		}
	}
	return 0;
free_irq:
	free_irqs(chip);
unregister_batt:
	power_supply_unregister(&chip->batt_psy);
unregister_dc:
	power_supply_unregister(&chip->dc_psy);
unregister_usb:
	power_supply_unregister(&chip->usb_psy);
free_chip:
	kfree(chip);
	return rc;
}
static int __devexit pm8921_charger_remove(struct platform_device *pdev)
{
	struct pm8921_chg_chip *chip = platform_get_drvdata(pdev);
    int rc = 0;
	mutex_destroy(&chip->ncm_chg_chip.batt_adj.lock);
	free_irqs(chip);
	platform_set_drvdata(pdev, NULL);
	the_chip = NULL;
	kfree(chip);
	rc = pm8xxx_batt_alarm_disable(PM8XXX_BATT_ALARM_LOWER_COMPARATOR);
	if (rc)
		pr_err("%s:BATT_ALARM_LOWER unable to set disable state\n", __func__);
	rc |= pm8xxx_batt_alarm_unregister_notifier(&alarm_notifier);
	if (rc)
		pr_err("%s: unable to unregister alarm notifier\n", __func__);
	return rc;
}
static const struct dev_pm_ops pm8921_pm_ops = {
	.suspend	= pm8921_charger_suspend,
	.suspend_noirq  = pm8921_charger_suspend_noirq,
	.resume_noirq   = pm8921_charger_resume_noirq,
	.resume		= pm8921_charger_resume,
};
static struct platform_driver pm8921_charger_driver = {
	.probe		= pm8921_charger_probe,
	.remove		= __devexit_p(pm8921_charger_remove),
	.driver		= {
			.name	= PM8921_CHARGER_DEV_NAME,
			.owner	= THIS_MODULE,
			.pm	= &pm8921_pm_ops,
	},
};
static int __init pm8921_charger_init(void)
{
	return platform_driver_register(&pm8921_charger_driver);
}
static void __exit pm8921_charger_exit(void)
{
	platform_driver_unregister(&pm8921_charger_driver);
}
late_initcall(pm8921_charger_init);
module_exit(pm8921_charger_exit);
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("PMIC8921 charger/battery driver");
MODULE_VERSION("1.0");
MODULE_ALIAS("platform:" PM8921_CHARGER_DEV_NAME);
#ifdef CONFIG_FEATURE_NCMC_POWER
int nc_pm8921_get_fsm_status(u64 * val)
{
    u8 temp;
    temp = pm_chg_get_fsm_state(the_chip);
    *val = temp;
    return 0;
}
EXPORT_SYMBOL(nc_pm8921_get_fsm_status);
int nc_pm8921_chg_usb_suspend_enable(int enable)
{
    int rc;
    rc = pm_chg_masked_write(the_chip, CHG_CNTRL_3, CHG_USB_SUSPEND_BIT,
                   enable ? CHG_USB_SUSPEND_BIT : 0);
    if (rc)
        pr_err("Failed rc=%d\n", rc);
    return rc;
}
EXPORT_SYMBOL(nc_pm8921_chg_usb_suspend_enable);
int nc_pm8921_chg_imaxsel_set(int chg_current)
{
    u8 temp;
    int i, rc;
    rc = pm_chg_ibatmax_set(the_chip, chg_current);
    if (rc) {
        pr_err("Unable to set IBAT to %dmA.\n", chg_current);
        return rc;
    }
    for (i = ARRAY_SIZE(usb_ma_table) - 1; i >= 0; i--) {
        if (usb_ma_table[i].usb_ma <= chg_current)
            break;
    }
    if (i < 0)
        i = 0;
    temp = usb_ma_table[i].usb_ma;
    rc = pm_chg_iusbmax_set(the_chip, i);
    if (rc) {
        pr_err("Unable to set IUSB to %dmA.\n", temp);
        return rc;
    }
    return rc;
}
EXPORT_SYMBOL(nc_pm8921_chg_imaxsel_set);
int nc_pm8921_chg_lva_off(int off)
{
    pm_param.pm_lva_off.data[PM_LVA_OFF] = off;
#if !defined(CONFIG_FEATURE_NCMC_ONLY_FOR_PRODUCTION_PROCESS_8960)
    if (pm_param.pm_lva_off.data[PM_LVA_OFF]) {
        pr_info("[PM] LVA_OFF change to true\n");
        factory_mode_flag_apps = true;
    } else {
        pr_info("[PM] LVA_OFF change to false\n");
        factory_mode_flag_apps = false;
    }
#endif
    return 0;
}
EXPORT_SYMBOL(nc_pm8921_chg_lva_off);
#endif
static int ncm_batt_read_adc_mean(int channel)
{
	struct pm8xxx_adc_chan_result result;
	const int array_size = NCM_ADC_SAMPLE_NUM;
	int data_array[NCM_ADC_SAMPLE_NUM];
	int ret = 0;
	int count;
	int average;
	int max_index = 0;
	int min_index = 0;
	for (count = 0; count < array_size; ++count) {
		ret = pm8xxx_adc_read(channel, &result);
		if (ret) {
			pr_err("read adc failed: channel=%d, count=%d, ret=%d\n",
					channel, count, ret);
			goto out;
		}
		pr_debug("uvolts phy=%lld, meas=0x%llx\n", result.physical,
					result.measurement);
		data_array[count] = (int)result.physical;
	}
	for (count = 1; count < array_size; ++count) {
		if (data_array[count] > data_array[max_index]) {
			max_index = count;
		}
		if (data_array[count] < data_array[min_index]) {
			min_index = count;
		}
	}
	if (max_index == min_index) {
		average = data_array[max_index];
	}
	else {
		average = 0;
		for (count = 0; count < array_size; ++count) {
			if ((count != max_index) && (count != min_index)) {
				average += data_array[count];
			}
		}
		average /= (array_size - 2);
	}
	ret = average;
	pr_debug("average=%d\n", average);
out:
	return ret;
}
static int ncm_get_battery_uvolts(struct pm8921_chg_chip *chip,
					int *result, int *measurement)
{
	int ret;
	int fsm_state, is_charging;
	int dc_present;
	pr_debug("Enter.\n");
	if (ncm_is_initialized == false) {
        pr_err("Leave. : not ready\n");
        return -EINVAL;
    }
	else {
	}
	fsm_state = pm_chg_get_fsm_state(chip);
	is_charging = is_battery_charging(fsm_state);
	dc_present = is_dc_chg_plugged_in(chip);
    if (!factory_mode_flag_apps) {
        if (is_charging) {
#if defined(CONFIG_FEATURE_NCMC_RUBY)
            if ((!dc_present) && (ncm_active_chgtype == CHGMON_PM_CHG_CHARGER_DCP)) {
                pm_chg_nonsupport_charge_dis(chip, 1);
            }
            else{
                pm_chg_charge_dis(chip, 1);
            }
        }
    }
#else
            pm_chg_charge_dis(chip, 1);
        }
    }
	msleep(25);
#endif
	ret = ncm_batt_read_adc_mean(chip->vbat_channel);
    if (!factory_mode_flag_apps) {
        if (is_charging &&
            !is_protect_charge_stop()) {
#if defined(CONFIG_FEATURE_NCMC_RUBY)
            if ((!dc_present) && (ncm_active_chgtype == CHGMON_PM_CHG_CHARGER_DCP)) {
                pm_chg_nonsupport_charge_dis(chip, 0);
            }
            else {
                pm_chg_charge_dis(chip, 0);
            }
#else
            pm_chg_charge_dis(chip, 0);
#endif
        }
    }
	if (ret < 0) {
		*result = 0;
		pr_info("Abort.\n");
		return ret;
	}
	if (result != NULL) {
		*result = ret;
	}
	if (measurement != NULL) {
		*measurement = 0;
	}
	pr_debug("Leave.\n");
	return 0;
}
static int ncm_get_battery_temperature(struct pm8921_chg_chip *chip,
					int *result, int *measurement)
{
	int ret;
	struct pm8xxx_adc_chan_result chan_result;
	ret = pm8xxx_adc_read(CHANNEL_BATT_THERM_MV, &chan_result);
	if (ret) {
		pr_err("read adc failed: channel=%d, ret=%d\n",
					CHANNEL_BATT_THERM_MV, ret);
		return ret;
	}
	pr_debug("physical=%lld, measurement=0x%llx\n", chan_result.physical,
						chan_result.measurement);
	if (result != NULL) {
		*result = 0;
	}
	if (measurement != NULL) {
		*measurement = (int)chan_result.measurement;
	}
	return 0;
}
static int ncm_pm8921_get_xo_temperature(struct pm8921_chg_chip *chip, int *result, int *measurement)
{
	struct pm8xxx_adc_chan_result chan_result = {
		.measurement = NCM_ADC_MV_UNSET,
	};
	int ret;
	ret = pm8xxx_adc_read(CHANNEL_MUXOFF_MV, &chan_result);
	if ((ret < 0) && (chan_result.measurement == NCM_ADC_MV_UNSET)) {
		pr_err("failed.\n");
		return ret;
	}
	if (result != NULL) {
		*result = (int)chan_result.physical;
	}
	if (measurement != NULL) {
		*measurement = (int)chan_result.measurement;
	}
	return 0;
}
static int ncm_pm8921_get_batt_temperature_uv(struct pm8921_chg_chip *chip, int *result, int *measurement)
{
	struct pm8xxx_adc_chan_result chan_result = {
		.measurement = NCM_ADC_MV_UNSET,
	};
	int ret;
	ret = pm8xxx_adc_read(CHANNEL_BATT_THERM_MV, &chan_result);
	if ((ret < 0) && (chan_result.measurement == NCM_ADC_MV_UNSET)) {
		pr_err("failed.\n");
		return ret;
	}
	if (result != NULL) {
		*result = (int)chan_result.physical;
	}
	if (measurement != NULL) {
		*measurement = (int)chan_result.measurement;
	}
	return 0;
}
static void ncm_update_batt_work(struct work_struct *work)
{
	struct pm8921_chg_chip *chip = container_of(work,
			struct pm8921_chg_chip, ncm_chg_chip.update_batt_work);
	battim_battcal_val_type adj_val;
	int ret = 0;
	unsigned long flags;
	static int prev_time = 0;
    static int prev_capacity = 100;
	if (ncm_debug_mask & NCM_DEBUG_TIMER_WORK)
		pr_debug("[IN] waiting_batt_cal=%d\n", chip->ncm_chg_chip.waiting_batt_cal);
	if (chip->ncm_chg_chip.waiting_batt_cal == false) {
		goto END;
	}
	mutex_lock(&chip->ncm_chg_chip.batt_adj.lock);
	ret = battim_get_battcal_val(&adj_val);
	if ((ret == 0) && (adj_val.voltage != 0)) {
		chip->ncm_chg_chip.batt_adj.v_adc = (int)adj_val.v_adc;
		chip->ncm_chg_chip.batt_adj.voltage = (int)adj_val.voltage;
		chip->ncm_chg_chip.batt_adj.capacity = (int)adj_val.capacity;
		chip->ncm_chg_chip.batt_adj.enable = true;
		pr_info("voltage=%d, capacity=%d\n",
				chip->ncm_chg_chip.batt_adj.voltage,
				chip->ncm_chg_chip.batt_adj.capacity);
		if ( (prev_time == 0) ||
			 ((adj_val.time - prev_time) >= NCM_REPORT_PERIOD_SEC) ) {
			printk(KERN_ERR "[T][ARM]Event:0x0D Info:0x00\n");
			prev_time = adj_val.time;
		}
		if (chip->ncm_chg_chip.batt_adj.capacity == 0) {
			printk(KERN_ERR "[T][ARM]Event:0x04 Info:0x%02X%02X%04X01\n",
				ncm_chgmon_charger_state(),
				ncm_chgmon_charger_type(),
				chip->ncm_chg_chip.batt_adj.v_adc);
		}
        if ( (prev_capacity != 100) && (chip->ncm_chg_chip.batt_adj.capacity == 100) ) {
            printk(KERN_ERR "[T][ARM]Event:0x09 Info:0x00\n");
        }
#ifdef CONFIG_FEATURE_NCMC_RUBY
        if ( (prev_capacity == 100) && (chip->ncm_chg_chip.batt_adj.capacity != 100) ) {
            if ( ncm_is_connect_usbchg() == NCM_CHG_USB_CONNECT ){
                ncm_chg_set_charging_vbatdet();
            }
        }
#endif
        prev_capacity = chip->ncm_chg_chip.batt_adj.capacity;
	}
	else {
		pr_info("use non-adjusted battery values.\n");
	}
	mutex_unlock(&chip->ncm_chg_chip.batt_adj.lock);
	spin_lock_irqsave(&chip->ncm_chg_chip.prtct_timer.slock, flags);
	if (the_chip->ncm_chg_chip.prtct_timer.is_timeout == false) {
		nc_pr_debug("bit clear!\n");
		pm_chg_failed_clear(chip, 1);
	}
	spin_unlock_irqrestore(&chip->ncm_chg_chip.prtct_timer.slock, flags);
	power_supply_changed(&chip->batt_psy);
END:
	chip->ncm_chg_chip.waiting_batt_cal = false;
	if (ncm_debug_mask & NCM_DEBUG_TIMER_WORK)
		pr_debug("[OUT]\n");
}
static void ncm_watch_chg_work(struct work_struct *work)
{
	int ncm_charge_src = PM8921_CHG_SRC_NONE;
	int ncm_charge_ret = NCM_CHG_PROTECT_RET_OK;
	int ncm_dctmp       = 0;
	int ncm_usbchg_sts	= NCM_CHG_USB_DISCONNECT;
	int ncm_chg_protect_sts_active = 0;
	int fsm_state = pm_chg_get_fsm_state(the_chip);
	struct pm8921_chg_chip *chip = container_of(work,
			struct pm8921_chg_chip, ncm_chg_chip.watch_chg_work);
	if (ncm_debug_mask & NCM_DEBUG_TIMER_WORK)
		pr_info("[IN] waiting_obs_update=%d, chk_circuit_flag=%d\n",
			chip->ncm_chg_chip.waiting_obs_update, chip->ncm_chg_chip.chk_circuit_flag);
	if (chip->ncm_chg_chip.waiting_obs_update == false) {
		goto END;
	}
	battim_get_obs_coll(&chip->ncm_chg_chip.obs_data);
	if(factory_mode_flag_apps){
		nc_pr_debug("Skip charge protection check by Factory mode.\n");
	}
	else {
		ncm_dctmp = ncm_is_connect_dcin();
		ncm_usbchg_sts = ncm_is_connect_usbchg();
		if ((ncm_dctmp != NCM_CHG_DC_DISCONNECT) || (ncm_usbchg_sts != NCM_CHG_USB_DISCONNECT)){
			pm8921_is_battery_charging(&ncm_charge_src);
			nc_pr_debug("charging sts[%d] fsm[%d]!\n", ncm_charge_src, fsm_state);
			ncm_charge_protection_main(ncm_charge_src);
			check_charge_protect_sts();
		}
		else {
			ncm_charge_ret = ncm_chg_protect_chager_remove();
			if (ncm_charge_ret == NCM_CHG_PROTECT_RET_OK){
				check_charge_protect_sts();
			}
			else if (ncm_charge_ret == NCM_CHG_PROTECT_RET_NOCHG){
			}
			else {
				printk(KERN_ERR "[PM] %s: Err!chg-remove chk\n", __func__);
			}
		}
		if (ncm_prev_chgstate == CHGMON_PM_CHG_STATE_MAX_NUM){
			ncm_prev_chgstate   = ncm_chgmon_charger_state();
		}
		else {
			if (CHGMON_PM_CHG_STATE_OFF == ncm_chgmon_charger_state()){
			}
			else {
				ncm_prev_chgstate   = ncm_active_chgstate;
			}
		}
		ncm_active_chgstate = ncm_chgmon_charger_state();
		nc_pr_debug("[PM]stschg old=%d new=%d\n", ncm_prev_chgstate, ncm_active_chgstate);
		if (ncm_prev_chgtype == CHGMON_PM_CHARGER_MAX_NUM){
			ncm_prev_chgtype   = ncm_chgmon_charger_type();
		}
		else {
			if (CHGMON_PM_CHG_CHARGER_NONE == ncm_chgmon_charger_type()){
			}
			else {
				ncm_prev_chgtype   = ncm_active_chgtype;
			}
		}
		ncm_active_chgtype = ncm_chgmon_charger_type();
		nc_pr_debug("[PM]typechg old=%d new=%d\n", ncm_prev_chgtype, ncm_active_chgtype);
		if (chip->ncm_chg_chip.chk_circuit_flag == true) {
			chip->ncm_chg_chip.chk_circuit_flag = false;
		}
	}
    ncm_watch_chg_work_for_charging(chip);
	get_active_status(&ncm_chg_protect_sts_active);
	nc_pr_debug("*** charge protect sts[%d]!\n", ncm_chg_protect_sts_active);
	nc_pr_debug("*** charge protect timer[%d]!\n",
		the_chip->ncm_chg_chip.prtct_timer.is_timer);
END:
	chip->ncm_chg_chip.waiting_obs_update = false;
	wake_unlock(&chip->ncm_chg_chip.timer_action_wlock);
	if (ncm_debug_mask & NCM_DEBUG_TIMER_WORK)
		pr_info("[OUT]\n");
}
static void ncm_timer_resume(struct pm8921_chg_chip *chip)
{
	ktime_t       now;
	unsigned long now_jf;
	unsigned long from_resume_jf;
	ktime_t from_read_batt;
	now    = alarm_get_elapsed_realtime();
	now_jf = jiffies;
	from_resume_jf = now_jf - chip->ncm_chg_chip.jiff_on_resume;
	chip->ncm_chg_chip.time.on_resume_adj = ktime_sub_us(now, jiffies_to_usecs(from_resume_jf));
	if (ncm_debug_mask & NCM_DEBUG_TIMER)
		pr_debug("time_on_resume_adj=%lld.%03dsec (adj=%lu.%03lusec)\n",
				NCM_KTIME_TO_SEC(chip->ncm_chg_chip.time.on_resume_adj),
				NCM_KTIME_TVNSEC_TO_MSEC(chip->ncm_chg_chip.time.on_resume_adj),
				(from_resume_jf / HZ),
				((from_resume_jf % HZ) * (MSEC_PER_SEC / HZ)));
	if (ncm_debug_mask & NCM_DEBUG_TIMER) {
		ktime_t sleep_time = ktime_sub(chip->ncm_chg_chip.time.on_resume_adj, chip->ncm_chg_chip.time.on_suspend);
		pr_info("suspended_duration=%lld.%03dsec\n",
				NCM_KTIME_TO_SEC(sleep_time),
				NCM_KTIME_TVNSEC_TO_MSEC(sleep_time));
	}
	from_read_batt = ktime_sub(chip->ncm_chg_chip.time.on_resume_adj, chip->ncm_chg_chip.time.on_read_batt);
	if (ncm_debug_mask & NCM_DEBUG_TIMER)
		pr_info("elapsed_from_last_read=%lld.%03dsec\n",
				NCM_KTIME_TO_SEC(from_read_batt),
				NCM_KTIME_TVNSEC_TO_MSEC(from_read_batt));
	if (NCM_KTIME_TO_SEC(from_read_batt) < pm_param.pm_pw_adc_determin.data[PM_ADC_GET_TIME]) {
		ncm_timer_notify_event(chip, NCM_TMR_EVENT__SLP_LEFT_Q);
	}
	else {
		ncm_timer_notify_event(chip, NCM_TMR_EVENT__SLP_LEFT_S);
	}
}
static void ncm_process_timer_works(struct pm8921_chg_chip *chip, bool update_batt)
{
	wake_lock(&chip->ncm_chg_chip.timer_action_wlock);
	if (update_batt == true) {
		schedule_work(&chip->ncm_chg_chip.update_batt_work);
	}
	schedule_work(&chip->ncm_chg_chip.watch_chg_work);
	if (ncm_debug_mask & NCM_DEBUG_TIMER_WORK)
		pr_debug("waiting_batt_cal=%d, waiting_obs_update=%d\n",
			chip->ncm_chg_chip.waiting_batt_cal, chip->ncm_chg_chip.waiting_obs_update);
}
static void ncm_notify_user_event(battim_event_type *data)
{
	battim_battcal_val_type battcal_val;
	int ret;
	int uvolts;
	if (ncm_debug_mask & NCM_DEBUG_USER_EVENT)
		pr_info("event_id=%d, error=%d\n", data->event_id, data->error);
	switch(data->event_id) {
	case BATTIM_USER_EVENT_FIN_BATTCAL:
		battim_cancel_kern_event(BATTIM_KERN_EVENT_REQ_BATTCAL_WAKEUP);
		battim_cancel_kern_event(BATTIM_KERN_EVENT_REQ_BATTCAL);
		battim_cancel_kern_event(BATTIM_KERN_EVENT_REQ_UPDATE_OBS_MDM);
		cancel_delayed_work(&the_chip->ncm_chg_chip.time_keep_work);
		ncm_process_timer_works(the_chip, true);
		wake_unlock(&the_chip->ncm_chg_chip.waiting_wlock);
		break;
	case BATTIM_USER_EVENT_FIN_UPDATE_OBS_MDM:
		battim_cancel_kern_event(BATTIM_KERN_EVENT_REQ_UPDATE_OBS_MDM);
		if (the_chip->ncm_chg_chip.waiting_batt_cal == false) {
			cancel_delayed_work(&the_chip->ncm_chg_chip.time_keep_work);
			ncm_process_timer_works(the_chip, false);
			wake_unlock(&the_chip->ncm_chg_chip.waiting_wlock);
		}
		else {
			ncm_process_timer_works(the_chip, false);
		}
		break;
	case BATTIM_USER_EVENT_REQ_UPDATE_BATTCAL_VAL:
		memset(&battcal_val, 0, sizeof(battcal_val));
		ret = ncm_get_battery_uvolts(the_chip, &uvolts, NULL);
		if (ret < 0) {
			pr_err("Can't get batt voltage.\n");
			data->error = -1;
			goto out;
		}
		battcal_val.v_adc = (uint16_t)(uvolts / 1000);
		ret = ncm_get_battery_temperature(the_chip, NULL, &uvolts);
		if (ret < 0) {
			pr_err("Can't get batt temperature.\n");
			data->error = -1;
			goto out;
		}
		battcal_val.t_adc = (uint16_t)(uvolts / 1000);
		the_chip->ncm_chg_chip.time.on_read_batt = alarm_get_elapsed_realtime();
		if (ncm_debug_mask & NCM_DEBUG_TIMER)
			pr_debug("time_on_read_adc=%lld.%03dsec\n",
				NCM_KTIME_TO_SEC(the_chip->ncm_chg_chip.time.on_read_batt),
				NCM_KTIME_TVNSEC_TO_MSEC(the_chip->ncm_chg_chip.time.on_read_batt));
		battcal_val.time = NCM_KTIME_TO_SEC(the_chip->ncm_chg_chip.time.on_read_batt);
		battim_set_battcal_val(&battcal_val);
		break;
	case BATTIM_USER_EVENT_REQ_UPDATE_CHGMON_VAL:
		ncm_chgmon_update_data();
		break;
	case BATTIM_USER_EVENT_REQ_UPDATE_CHGPROT_VAL:
		ncm_user_event_get_xo_therm();
		break;
	case BATTIM_USER_EVENT_FIN_UPDATE_PARAM:
		if (the_chip->ncm_chg_chip.user_ready == false) {
			the_chip->ncm_chg_chip.user_ready = true;
			pr_info("user is ready!\n");
			ncm_timer_notify_event(the_chip, NCM_TMR_EVENT__USR_READY);
		}
		ncm_register_batt_alarm();
#if !defined(CONFIG_FEATURE_NCMC_ONLY_FOR_PRODUCTION_PROCESS_8960)
		if (pm_param.pm_lva_off.data[PM_LVA_OFF]) {
			pr_info("[PM] LVA_OFF set to true\n");
			factory_mode_flag_apps = true;
		} else {
			pr_info("[PM] LVA_OFF set to false\n");
			factory_mode_flag_apps = false;
		}
#endif
		break;
	case BATTIM_USER_EVENT_RESUME:
		ncm_timer_resume(the_chip);
		wake_unlock(&the_chip->ncm_chg_chip.waiting_wlock);
		break;
	default:
		break;
	}
out:
	return;
}
static void ncm_notify_timer_fired(unsigned long arg)
{
	unsigned long flags;
	if (ncm_debug_mask & NCM_DEBUG_TIMER)
		pr_debug("arg=%lu\n", arg);
	spin_lock_irqsave(&the_chip->ncm_chg_chip.timer_slock, flags);
	the_chip->ncm_chg_chip.timer_flag = false;
	if (atomic_read(&the_chip->ncm_chg_chip.timer_lock_atomic) == 0) {
		switch(arg) {
		case 5:
			ncm_timer_notify_event(the_chip, NCM_TMR_EVENT__TMR_EXPIRED_5);
			break;
		case 30:
			ncm_timer_notify_event(the_chip, NCM_TMR_EVENT__TMR_EXPIRED_30);
			break;
		default:
			break;
		}
	}
	spin_unlock_irqrestore(&the_chip->ncm_chg_chip.timer_slock, flags);
}
static int ncm_queue_enqueue(ncm_queue_type *queue, int data)
{
	int pos, next_pos;
	unsigned long flags;
	int ret = 0;
	spin_lock_irqsave(&queue->slock, flags);
	pos = queue->tail;
	next_pos = (pos + 1) % NCM_TIMER_QUEUE_MAX;
	if (next_pos == queue->head) {
		pr_err("queue is full.\n");
		ret = -1;
		goto out;
	}
	queue->data[pos] = data;
	queue->tail = next_pos;
out:
	spin_unlock_irqrestore(&queue->slock, flags);
	return ret;
}
static int ncm_queue_dequeue(ncm_queue_type *queue, int *data)
{
	int pos, next_pos;
	unsigned long flags;
	int ret = 0;
	spin_lock_irqsave(&queue->slock, flags);
	pos = queue->head;
	next_pos = (pos + 1) % NCM_TIMER_QUEUE_MAX;
	if (pos == queue->tail) {
		pr_debug("queue is empty.\n");
		ret = -1;
		goto out;
	}
	*data = queue->data[pos];
	queue->head = next_pos;
out:
	spin_unlock_irqrestore(&queue->slock, flags);
	return ret;
}
static bool ncm_queue_is_empty(ncm_queue_type *queue)
{
	return (queue->head == queue->tail);
}
static void ncm_timer_cancel_timer(struct pm8921_chg_chip *chip)
{
	unsigned long flags;
	if (ncm_debug_mask & NCM_DEBUG_TIMER)
		pr_debug("\n");
	spin_lock_irqsave(&chip->ncm_chg_chip.timer_slock, flags);
	if (chip->ncm_chg_chip.timer_flag == true)
		del_timer_sync(&chip->ncm_chg_chip.timer);
	chip->ncm_chg_chip.timer_flag = false;
	spin_unlock_irqrestore(&chip->ncm_chg_chip.timer_slock, flags);
}
static void ncm_timer_start_timer(struct pm8921_chg_chip *chip, int sec)
{
	unsigned long flags;
	if (atomic_read(&chip->ncm_chg_chip.timer_lock_atomic) > 0) {
		if (ncm_debug_mask & NCM_DEBUG_TIMER)
			pr_info("canceled for lock.\n");
		return;
	}
	if (ncm_debug_mask & NCM_DEBUG_TIMER)
		pr_debug("sec=%d\n", sec);
	spin_lock_irqsave(&chip->ncm_chg_chip.timer_slock, flags);
	if (chip->ncm_chg_chip.timer_flag == true)
		del_timer_sync(&chip->ncm_chg_chip.timer);
	chip->ncm_chg_chip.timer.expires  = jiffies + (HZ * sec);
	chip->ncm_chg_chip.timer.data     = sec;
	chip->ncm_chg_chip.timer.function = ncm_notify_timer_fired;
	chip->ncm_chg_chip.timer_flag     = true;
	add_timer(&chip->ncm_chg_chip.timer);
	spin_unlock_irqrestore(&chip->ncm_chg_chip.timer_slock, flags);
}
static bool ncm_timer_is_canceling_event(int tm_event)
{
	bool prior = false;
	switch(tm_event) {
	case NCM_TMR_EVENT__CHG_REMOVED:
	case NCM_TMR_EVENT__CHG_INSERTED:
	case NCM_TMR_EVENT__SLP_ENTERING:
		prior = true;
		break;
	default:
		break;
	}
	return prior;
}
static void ncm_timer_set_kern_event(struct pm8921_chg_chip *chip, int event_id)
{
	int ret;
	battim_event_type event = {
		.event_id = event_id,
		.error = 0,
	};
	if (ncm_debug_mask & NCM_DEBUG_TIMER_WORK)
		pr_info("event_id=%d\n", event_id);
	switch (event_id) {
	case BATTIM_KERN_EVENT_REQ_BATTCAL_WAKEUP:
	case BATTIM_KERN_EVENT_REQ_BATTCAL:
	case BATTIM_KERN_EVENT_REQ_UPDATE_OBS_MDM:
		ret = battim_set_kern_event(&event);
		if (ret < 0) {
			pr_err("battim_set_kern_event cannot work.(ret=%d)\n", ret);
		}
		else {
			if (event_id == BATTIM_KERN_EVENT_REQ_BATTCAL_WAKEUP
			  || event_id == BATTIM_KERN_EVENT_REQ_BATTCAL) {
				chip->ncm_chg_chip.waiting_batt_cal = true;
			}
			chip->ncm_chg_chip.waiting_obs_update = true;
			cancel_delayed_work(&chip->ncm_chg_chip.time_keep_work);
			wake_lock(&chip->ncm_chg_chip.waiting_wlock);
			schedule_delayed_work(&chip->ncm_chg_chip.time_keep_work,
						round_jiffies_relative(msecs_to_jiffies(NCM_BATTADJ_WAIT_MAX_MS)));
		}
		break;
	default:
		break;
	}
	if (ncm_debug_mask & NCM_DEBUG_TIMER_WORK)
		pr_debug("waiting_batt_cal=%d, waiting_obs_update=%d\n",
			chip->ncm_chg_chip.waiting_batt_cal, chip->ncm_chg_chip.waiting_obs_update);
}
static int ncm_is_batt_removed(struct pm8921_chg_chip *chip)
{
	int uvolts, t_adc, ret;
	ret = ncm_get_battery_temperature(chip, NULL, &uvolts);
	if (ret < 0) {
		pr_err("%s: Can't get batt temperature.\n", __func__);
		ret = true;
	} else {
		t_adc = (uint16_t)(uvolts / 1000);
		pr_debug("%s: get_battery_temperature,t_adc = %d\n",
				__func__, t_adc);
		if (t_adc < NCM_BATT_REMOVE_THERM_THRESHOLD) {
			ret = false;
		} else {
			ret = true;
		}
	}
	return ret;
}
static void ncm_batt_removed_handler_func(struct work_struct *work)
{
	int status;
	struct delayed_work *dwork = to_delayed_work(work);
	struct pm8921_chg_chip *chip = container_of(dwork,
			struct pm8921_chg_chip, ncm_chg_chip.batt_removed_handler_work);
	if (ncm_is_batt_removed(chip)) {
		status = pm_chg_get_rt_status(chip, BATT_REMOVED_IRQ);
		pr_debug("battery present=%d state=%d", !status,
				pm_chg_get_fsm_state(chip));
		handle_stop_ext_chg(chip);
		power_supply_changed(&chip->batt_psy);
		pr_emerg("BATT REMOVED->kernel power OFF...\n");
		msleep(100);
		kernel_power_off();
	} else {
		pr_info("Battery removed Occurred!");
	}
	return;
}
static void ncm_time_keep_work(struct work_struct *work)
{
	struct delayed_work *dwork = to_delayed_work(work);
	struct pm8921_chg_chip *chip = container_of(dwork,
				struct pm8921_chg_chip, ncm_chg_chip.time_keep_work);
	pr_info("waiting_batt_cal=%d, waiting_obs_update=%d\n",
		chip->ncm_chg_chip.waiting_batt_cal, chip->ncm_chg_chip.waiting_obs_update);
	ncm_process_timer_works(chip, chip->ncm_chg_chip.waiting_batt_cal);
	wake_unlock(&chip->ncm_chg_chip.waiting_wlock);
}
static void ncm_timer_handle_event_proc(struct pm8921_chg_chip *chip, int proc)
{
	switch (proc) {
	case NCM_TMR_PROC__HNDL_ALL:
	case NCM_TMR_PROC__HNDL_ALL_ON_WAKEUP:
		chip->ncm_chg_chip.chk_circuit_flag = true;
		break;
	case NCM_TMR_PROC__HNDL_CHG:
		break;
	default:
		break;
	}
	switch (proc) {
	case NCM_TMR_PROC__HNDL_ALL:
		ncm_timer_set_kern_event(chip, BATTIM_KERN_EVENT_REQ_BATTCAL);
		break;
	case NCM_TMR_PROC__HNDL_ALL_ON_WAKEUP:
		ncm_timer_set_kern_event(chip, BATTIM_KERN_EVENT_REQ_BATTCAL_WAKEUP);
		break;
	case NCM_TMR_PROC__HNDL_CHG:
		ncm_timer_set_kern_event(chip, BATTIM_KERN_EVENT_REQ_UPDATE_OBS_MDM);
		break;
	default:
		break;
	}
}
static void ncm_timer_handle_event(struct pm8921_chg_chip *chip, int tm_event)
{
	static int state_tmr = NCM_TMR_STATE__TMR_STOPPING;
	static int state_chg = NCM_TMR_STATE__CHG_DCED;
	static int cnt = 0;
	int prev_state_tmr;
	int prev_state_chg;
	if (ncm_debug_mask & NCM_DEBUG_TIMER)
		pr_debug("[IN] state_tmr=%d, state_chg=%d, cnt=%d\n", state_tmr, state_chg, cnt);
	if (ncm_debug_mask & NCM_DEBUG_TIMER)
		pr_info("tm_event=%d\n", tm_event);
	if (ncm_timer_is_canceling_event(tm_event)) {
		atomic_dec(&chip->ncm_chg_chip.timer_lock_atomic);
		if (ncm_debug_mask & NCM_DEBUG_TIMER)
			pr_debug("Unlock timer.\n");
	}
	prev_state_tmr = state_tmr;
	prev_state_chg = state_chg;
	switch(tm_event) {
	case NCM_TMR_EVENT__CHG_REMOVED:
		state_chg = NCM_TMR_STATE__CHG_DCED;
		break;
	case NCM_TMR_EVENT__CHG_INSERTED:
		state_chg = NCM_TMR_STATE__CHG_CED;
		break;
	case NCM_TMR_EVENT__USR_READY:
		state_tmr = NCM_TMR_STATE__TMR_RUNNING;
		break;
	case NCM_TMR_EVENT__SLP_LEFT_S:
	case NCM_TMR_EVENT__SLP_LEFT_Q:
		if (chip->ncm_chg_chip.user_ready == true) {
			state_tmr = NCM_TMR_STATE__TMR_RUNNING;
		}
		break;
	case NCM_TMR_EVENT__SLP_ENTERING:
		state_tmr = NCM_TMR_STATE__TMR_STOPPING;
		break;
	default:
		break;
	}
	switch(tm_event) {
	case NCM_TMR_EVENT__INIT:
		init_timer(&chip->ncm_chg_chip.timer);
		chip->ncm_chg_chip.timer_flag = false;
		atomic_set(&chip->ncm_chg_chip.timer_lock_atomic, 0);
		chip->ncm_chg_chip.waiting_batt_cal = false;
		chip->ncm_chg_chip.waiting_obs_update = false;
		break;
	case NCM_TMR_EVENT__TMR_EXPIRED_30:
		if (prev_state_tmr == NCM_TMR_STATE__TMR_RUNNING
		  && prev_state_chg == NCM_TMR_STATE__CHG_DCED) {
			cnt = 0;
			ncm_timer_start_timer(chip, 30);
			ncm_timer_handle_event_proc(chip, NCM_TMR_PROC__HNDL_ALL);
		}
		break;
	case NCM_TMR_EVENT__TMR_EXPIRED_5:
		if (prev_state_tmr == NCM_TMR_STATE__TMR_RUNNING
		  && prev_state_chg == NCM_TMR_STATE__CHG_CED) {
			cnt += 5;
			if (cnt >= 30) {
				cnt = 0;
				ncm_timer_start_timer(chip, 5);
				ncm_timer_handle_event_proc(chip, NCM_TMR_PROC__HNDL_ALL);
			}
			else {
				ncm_timer_start_timer(chip, 5);
				ncm_timer_handle_event_proc(chip, NCM_TMR_PROC__HNDL_CHG);
			}
		}
		break;
	case NCM_TMR_EVENT__CHG_REMOVED:
		if (prev_state_tmr == NCM_TMR_STATE__TMR_RUNNING) {
			cnt = 0;
			ncm_timer_start_timer(chip, 30);
			ncm_timer_handle_event_proc(chip, NCM_TMR_PROC__HNDL_CHG);
		}
		break;
	case NCM_TMR_EVENT__CHG_INSERTED:
		if (prev_state_tmr == NCM_TMR_STATE__TMR_RUNNING) {
			cnt = 0;
			ncm_timer_start_timer(chip, 5);
			ncm_timer_handle_event_proc(chip, NCM_TMR_PROC__HNDL_CHG);
		}
		break;
	case NCM_TMR_EVENT__USR_READY:
		cnt = 0;
		ncm_timer_start_timer(chip, (prev_state_chg == NCM_TMR_STATE__CHG_CED) ? 5 : 30);
		ncm_timer_handle_event_proc(chip, NCM_TMR_PROC__HNDL_ALL);
		break;
	case NCM_TMR_EVENT__SLP_LEFT_S:
		if (chip->ncm_chg_chip.user_ready == true) {
			cnt = 0;
			ncm_timer_start_timer(chip, (prev_state_chg == NCM_TMR_STATE__CHG_CED) ? 5 : 30);
			ncm_timer_handle_event_proc(chip, NCM_TMR_PROC__HNDL_ALL_ON_WAKEUP);
		}
		break;
	case NCM_TMR_EVENT__SLP_LEFT_Q:
		if (chip->ncm_chg_chip.user_ready == true) {
			cnt = 0;
			ncm_timer_start_timer(chip, (prev_state_chg == NCM_TMR_STATE__CHG_CED) ? 5 : 30);
			if (prev_state_tmr == NCM_TMR_STATE__TMR_STOPPING
			  && prev_state_chg == NCM_TMR_STATE__CHG_CED) {
				ncm_timer_handle_event_proc(chip, NCM_TMR_PROC__HNDL_CHG);
			}
		}
		break;
	case NCM_TMR_EVENT__SLP_ENTERING:
		break;
	default:
		break;
	}
	if (ncm_debug_mask & NCM_DEBUG_TIMER)
		pr_debug("[OUT] state_tmr=%d, state_chg=%d, cnt=%d\n", state_tmr, state_chg, cnt);
	return;
}
static void ncm_timer_notify_event(struct pm8921_chg_chip *chip, int tm_event)
{
	int ret;
	if (ncm_debug_mask & NCM_DEBUG_TIMER)
		pr_info("tm_event=%d\n", tm_event);
	ret = ncm_queue_enqueue(&chip->ncm_chg_chip.timer_queue, tm_event);
	if (!ret) {
		if (ncm_timer_is_canceling_event(tm_event)) {
			atomic_inc(&chip->ncm_chg_chip.timer_lock_atomic);
			if (ncm_debug_mask & NCM_DEBUG_TIMER)
				pr_debug("Lock timer.\n");
			ncm_timer_cancel_timer(chip);
		}
		if (tm_event != NCM_TMR_EVENT__SLP_ENTERING) {
			wake_lock(&chip->ncm_chg_chip.timer_event_wlock);
		}
		schedule_work(&chip->ncm_chg_chip.timer_work);
	}
}
static void ncm_timer_process_events(struct work_struct *work)
{
	struct pm8921_chg_chip *chip = container_of(work,
				struct pm8921_chg_chip, ncm_chg_chip.timer_work);
	int tm_event = 0;
	int rc;
	unsigned long flags;
	do {
		rc = ncm_queue_dequeue(&chip->ncm_chg_chip.timer_queue, &tm_event);
		if (!rc)
			ncm_timer_handle_event(chip, tm_event);
	} while (!rc);
	spin_lock_irqsave(&chip->ncm_chg_chip.timer_queue.slock, flags);
	if (ncm_queue_is_empty(&chip->ncm_chg_chip.timer_queue)) {
		atomic_set(&chip->ncm_chg_chip.timer_lock_atomic, 0);
		wake_unlock(&chip->ncm_chg_chip.timer_event_wlock);
	}
	spin_unlock_irqrestore(&chip->ncm_chg_chip.timer_queue.slock, flags);
}
static int ncm_find_best_charger(struct pm8921_chg_chip *chip)
{
	int source = PM8921_CHG_SRC_NONE;
	int dc_present, usb_present;
	dc_present = is_dc_chg_plugged_in(chip);
	usb_present = is_usb_chg_plugged_in(chip);
	if (dc_present && !usb_present)
		source = PM8921_CHG_SRC_DC;
	if (usb_present && !dc_present)
		source = PM8921_CHG_SRC_USB;
	if (usb_present && dc_present)
		source = PM8921_CHG_SRC_DC;
	return source;
}
static int ncm_get_online_charger_count(struct pm8921_chg_chip *chip)
{
	int i;
	int ret;
	int count = 0;
	struct power_supply *psy[] = {
		&chip->usb_psy,
		&chip->dc_psy,
		chip->ext_psy,
	};
	for (i = 0; i < sizeof(psy) / sizeof(psy[0]); i++) {
		union power_supply_propval val = { 0 };
		if (psy[i]) {
			ret = psy[i]->get_property(psy[i], POWER_SUPPLY_PROP_ONLINE, &val);
			if (!ret && val.intval) {
				count++;
			}
		}
	}
	return count;
}
static void ncm_handle_charger_changed(struct pm8921_chg_chip *chip)
{
	static int prev_chg = PM8921_CHG_SRC_NONE;
	int current_chg;
	int online_cnt;
	current_chg = ncm_find_best_charger(chip);
	online_cnt = ncm_get_online_charger_count(chip);
	pr_debug("[IN] prev_chg=%d, current_chg=%d, online_cnt=%d\n",
				prev_chg, current_chg, online_cnt);
	if (online_cnt > 0) {
		pm_obs_a_charger(PM_OBS_CHG_ON_MODE);
	}
	else {
		if (current_chg != PM8921_CHG_SRC_NONE) {
			pm_obs_a_charger(PM_OBS_CHG_STOP_MODE);
		}
		else {
			pm_obs_a_charger(PM_OBS_CHG_OFF_MODE);
		}
	}
	if (current_chg != prev_chg) {
		if (current_chg == PM8921_CHG_SRC_NONE) {
			ncm_timer_notify_event(chip, NCM_TMR_EVENT__CHG_REMOVED);
		}
		else {
			ncm_timer_notify_event(chip, NCM_TMR_EVENT__CHG_INSERTED);
		}
		prev_chg = current_chg;
	}
	pr_debug("[OUT] prev_chg=%d\n", prev_chg);
}
static void ncm_handle_event(struct pm8921_chg_chip *chip, int event)
{
	pr_debug("event=%d\n", event);
	switch (event)
	{
	case NCM_EVENT_BMS_NOTIFY:
		break;
	case NCM_EVENT_CUT_OFF:
		mutex_lock(&chip->ncm_chg_chip.batt_adj.lock);
		chip->ncm_chg_chip.batt_adj.capacity = 0;
		mutex_unlock(&chip->ncm_chg_chip.batt_adj.lock);
		printk(KERN_ERR "[T][ARM]Event:0x04 Info:0x%02X%02X%04X02\n",
			ncm_chgmon_charger_state(),
			ncm_chgmon_charger_type(),
			pm_param.pm_bat_alm_cut_off.data[PM_DET_VOLT_UP_LOW]);
		power_supply_changed(&chip->batt_psy);
		break;
	case NCM_EVENT_CHARGE_PROTECT_DC_OVP:
		nc_pr_debug("NCM_EVENT_CHARGE_PROTECT_DC_OVP\n");
		power_supply_changed(&chip->dc_psy);
		power_supply_changed(&chip->batt_psy);
		break;
	case NCM_EVENT_CHARGE_PROTECT_USB_OVP:
		nc_pr_debug("NCM_EVENT_CHARGE_PROTECT_USB_OVP\n");
		power_supply_changed(&chip->usb_psy);
		power_supply_changed(&chip->batt_psy);
		break;
	case NCM_EVENT_CHARGE_PROTECT_USB_OCP:
		nc_pr_debug("NCM_EVENT_CHARGE_PROTECT_USB_OCP\n");
		power_supply_changed(&chip->usb_psy);
		power_supply_changed(&chip->batt_psy);
		break;
	case NCM_EVENT_CHARGE_PROTECT_BATT_ERR:
		break;
	case NCM_EVENT_CHARGE_PROTECT_XO_TEMP:
		nc_pr_debug("NCM_EVENT_CHARGE_PROTECT_XO_TEMP\n");
		power_supply_changed(&chip->dc_psy);
		power_supply_changed(&chip->usb_psy);
		power_supply_changed(&chip->batt_psy);
		break;
	case NCM_EVENT_CHARGE_PROTECT_NOTIFY:
		nc_pr_debug("NCM_EVENT_CHARGE_PROTECT_NOTIFY\n");
		power_supply_changed(&chip->dc_psy);
		power_supply_changed(&chip->usb_psy);
		power_supply_changed(&chip->batt_psy);
		break;
	case NCM_EVENT_CHARGE_PROTECT_CHARGE_TIMEOUT:
		nc_pr_debug("NCM_EVENT_CHARGE_PROTECT_CHARGE_TIMEOUT\n");
		power_supply_changed(&chip->dc_psy);
		power_supply_changed(&chip->usb_psy);
		power_supply_changed(&chip->batt_psy);
		break;
	default:
		break;
	}
	ncm_handle_charger_changed(chip);
}
static void ncm_notify_event(struct pm8921_chg_chip *chip, int event)
{
	int ret;
	pr_debug("event=%d\n", event);
	ret = ncm_queue_enqueue(&chip->ncm_chg_chip.event_queue, event);
	if (!ret) {
		wake_lock(&chip->ncm_chg_chip.event_wlock);
		schedule_work(&chip->ncm_chg_chip.notify_work);
	}
}
static void ncm_process_events(struct work_struct *work)
{
	struct pm8921_chg_chip *chip = container_of(work,
				struct pm8921_chg_chip, ncm_chg_chip.notify_work);
	int event = 0;
	int rc;
	unsigned long flags;
	do {
		rc = ncm_queue_dequeue(&chip->ncm_chg_chip.event_queue, &event);
		if (!rc)
			ncm_handle_event(chip, event);
	} while (!rc);
	spin_lock_irqsave(&chip->ncm_chg_chip.event_queue.slock, flags);
	if (ncm_queue_is_empty(&chip->ncm_chg_chip.event_queue)) {
		wake_unlock(&chip->ncm_chg_chip.event_wlock);
	}
	spin_unlock_irqrestore(&chip->ncm_chg_chip.event_queue.slock, flags);
}
static void ncm_user_event_get_xo_therm(void)
{
    int xo_therm_temperature = 0;
    int xo_therm_voltage     = 0;
    battim_chgprot_val_type param;
    int ret = 0;
	ret = ncm_pm8921_get_xo_temperature(the_chip, &xo_therm_temperature, &xo_therm_voltage);
	if (0 == ret) {
		param.xo_therm = (uint16_t)(xo_therm_voltage / 1000);
		battim_set_chgprot_val(&param);
	}
	else {
		pr_err("[%s]adc_error_ERR:battery_therm. rc[%d]\n", __func__, ret);
		return;
	}
    return;
}
static int ncm_chgmon_charger_type(void)
{
    int dc_present, usb_present;
    int result;
    dc_present = is_dc_chg_plugged_in(the_chip);
    if (pm_chg_get_rt_status(the_chip, USBIN_VALID_IRQ)) {
        usb_present = is_usb_chg_plugged_in(the_chip);
    }
    else {
        usb_present = 0;
    }
    if(dc_present){
        result =  CHGMON_PM_CHG_CHARGER_Sub_Charger;
    }
    else if(usb_present){
#if defined(CONFIG_FEATURE_NCMC_POWER) && !defined(CONFIG_FEATURE_NCMC_RUBY)
        switch(nc_usb_device_type){
            case USB_SW_SDP_CONNECTED:
                result = CHGMON_PM_CHG_CHARGER_SDP;
                break;
            case USB_SW_DCP_CONNECTED:
                result = CHGMON_PM_CHG_CHARGER_DCP;
                break;
            case USB_SW_OTHER_DCP_CONNECTED:
                result = CHGMON_PM_CHG_CHARGER_Other_DCP;
                break;
            case USB_SW_DEVICE_DISCONNECTED:
                result = CHGMON_PM_CHG_CHARGER_NONE;
                break;
            case USB_SW_OTHER_DEVICE_CONNECTED:
                result = CHGMON_PM_CHG_CHARGER_Other;
                break;
            default:
                result = CHGMON_PM_CHARGER_MAX_NUM;
                break;
        }
#else
        switch(ncm_usb_charger_current_from_usb){
            case NCM_IUSB_SDP_MA:
                result = CHGMON_PM_CHG_CHARGER_SDP;
                break;
            case NCM_IUSB_DCP_MA:
            case NCM_IUSB_OTHER_DCP_MA:
                result = CHGMON_PM_CHG_CHARGER_DCP;
                break;
            case NCM_IUSB_NONE_MA:
                result = CHGMON_PM_CHG_CHARGER_NONE;
                break;
            default:
                result = CHGMON_PM_CHG_CHARGER_Other;
                break;
        }
#endif
    }else{
        result = CHGMON_PM_CHG_CHARGER_NONE;
    }
    return result;
}
static int ncm_chgmon_charger_state(void)
{
    int batt = 0;
    int fsm = 0;
    int vbatdet = 0;
    int status = 0;
    int rc = 0;
	batt = pm_chg_get_rt_status(the_chip, BATT_INSERTED_IRQ);
	if(batt == 0){
	    return CHGMON_PM_CHG_STATE_NO_BATT;
	}else{
    }
    rc = get_active_status(&status);
    if( rc == true ){
        switch(status){
            case IDX_DC_OVP:
                return CHGMON_PM_CHG_STATE_OVP;
                break;
            case IDX_USB_OVP:
                return CHGMON_PM_CHG_STATE_OVP;
                break;
            case IDX_USB_OCP:
                return CHGMON_PM_CHG_STATE_OCP;
                break;
            case IDX_BATT_TEMP_ERR:
                return CHGMON_PM_CHG_STATE_TEMP_FST;
                break;
            case IDX_BATT_TEMP_LIMIT:
             case IDX_XO_TEMP_LIMIT:
                break;
            case IDX_XO_TEMP:
                return CHGMON_PM_CHG_STATE_TEMP_FST;
                break;
            case IDX_CHARGE_TIMEOUT:
                return CHGMON_PM_CHG_STATE_BAT_ERR;
                break;
            case IDX_NORMAL:
                break;
            default:
                break;
        }
    }
    else{
        status = IDX_NORMAL;
    }
    fsm = pm_chg_get_fsm_state(the_chip);
    vbatdet = pm_chg_get_rt_status(the_chip, VBATDET_LOW_IRQ);
    if(fsm == FSM_STATE_ON_BAT_3){
        if((ncm_batt_temp_recover_flag == true) ||
            (ncm_xo_temp_recover_flag == true)){
            return CHGMON_PM_CHG_STATE_TEMP_FST;
        }
        return CHGMON_PM_CHG_STATE_OFF;
    }
    else if(fsm == FSM_STATE_ON_CHG_HIGHI_1){
        return CHGMON_PM_CHG_STATE_FULL;
    }
    else if((fsm == FSM_STATE_FAST_CHG_7)&&(vbatdet)){
        if((status == IDX_BATT_TEMP_LIMIT) || (status == IDX_XO_TEMP_LIMIT)){
            return CHGMON_PM_CHG_STATE_CC_H;
        }else{
            return CHGMON_PM_CHG_STATE_CC;
        }
    }
    else if((fsm == FSM_STATE_FAST_CHG_7)&&(!vbatdet)){
        if((status == IDX_BATT_TEMP_LIMIT) || (status == IDX_XO_TEMP_LIMIT)){
            return CHGMON_PM_CHG_STATE_CV_H;
        }else{
            return CHGMON_PM_CHG_STATE_CV;
        }
    }
    else{
    }
    pr_debug("[PM]UNKNOWN CHARGER_STATE\n");
    return CHGMON_PM_CHG_STATE_OFF;
}
static void ncm_chgmon_update_data(void)
{
    int rc;
    struct pm8xxx_adc_chan_result result;
    int volt;
    int dc_present, usb_present;
    int i_current;
    int temp   = 0;
    battim_chgmon_val_type chgmon_data;
    struct timespec ts;
    struct rtc_time tm;
    volt = qcom_get_prop_battery_uvolts(the_chip) / 1000;
	chgmon_data.value[CHGMON_PM_VAL_VBAT] = (int16_t)volt;
    dc_present = is_dc_chg_plugged_in(the_chip);
    usb_present = is_usb_chg_plugged_in(the_chip);
    if(dc_present){
        rc = pm8xxx_adc_read(CHANNEL_DCIN, &result);
        volt = (int)(result.physical) / 1000;
    }else if(usb_present){
        rc = pm8xxx_adc_read(CHANNEL_USBIN, &result);
        volt = (int)(result.physical) / 1000;
    }else{
        volt = 0;
    }
	chgmon_data.value[CHGMON_PM_VAL_VCHG] = (int16_t)volt;
	i_current = (get_prop_batt_current(the_chip)) / 1000;
	chgmon_data.value[CHGMON_PM_VAL_IBAT] = (int16_t)i_current;
#ifdef NCM_PM_CHG_ICALC_ENABLE
	chgmon_data.value[CHGMON_PM_VAL_ICAL] = (int16_t)the_chip->ncm_chg_chip.calc_current;
#else
	chgmon_data.value[CHGMON_PM_VAL_VBATDET] = (int16_t)the_chip->ncm_chg_chip.chgmon_vbatdet;
#endif
	chgmon_data.value[CHGMON_PM_VAL_VMAX] = (int16_t)the_chip->max_voltage_mv;
	chgmon_data.value[CHGMON_PM_VAL_IMAX] = (int16_t)the_chip->max_bat_chg_current;
	rc = ncm_pm8921_get_batt_temperature_uv(the_chip, &temp, &volt);
	chgmon_data.value[CHGMON_PM_VAL_TBAT] = (int16_t)(volt / 1000);
	rc = ncm_pm8921_get_xo_temperature(the_chip, &temp, &volt);
	chgmon_data.value[CHGMON_PM_VAL_XOTH] = (int16_t)(volt / 1000);
	chgmon_data.value[CHGMON_PM_VAL_CHARGER] = (int16_t)ncm_chgmon_charger_type();
	chgmon_data.value[CHGMON_PM_VAL_CHG_STATE] = (int16_t)ncm_chgmon_charger_state();
    getnstimeofday(&ts);
    rtc_time_to_tm(ts.tv_sec, &tm);
	chgmon_data.time_value[CHGMON_PM_TIME_RTC].hour = (uint8_t)tm.tm_hour;
    chgmon_data.time_value[CHGMON_PM_TIME_RTC].min  = (uint8_t)tm.tm_min;
    chgmon_data.time_value[CHGMON_PM_TIME_RTC].sec  = (uint8_t)tm.tm_sec;
    chgmon_data.value[CHGMON_PM_VAL_FSM_STATE] = (int16_t)pm_chg_get_fsm_state(the_chip);
	chgmon_data.value[CHGMON_PM_VAL_VIN] = (int16_t)pm_chg_vinmin_get(the_chip);
	chgmon_data.value[CHGMON_PM_VAL_IUSB] = (int16_t)ncm_usb_charger_current_present;
    rc = pm8xxx_adc_read(CHANNEL_VPH_PWR, &result);
	chgmon_data.value[CHGMON_PM_VAL_VOUT] = (int16_t)((int)(result.physical) / 1000);
	chgmon_data.value[CHGMON_PM_VAL_LOOP] = (int16_t)pm_chg_get_regulation_loop(the_chip);
    battim_set_chgmon_val(&chgmon_data);
}
static void ncm_chg_set_charging_vddmax(void)
{
    unsigned int voltage;
    int status = 0;
    int rc;
	rc = getProtect_Status(IDX_BATT_TEMP_LIMIT, &status);
    if(rc == false){
        return;
    }else{
    }
    if(status == ProtectNone){
        voltage = pm_param.pm_chg_setting.data[PM_V_CHG_HIGH1];
    }else{
        voltage = pm_param.pm_chg_setting.data[PM_V_CHG_HIGH2];
    }
    if(voltage != the_chip->max_voltage_mv){
        the_chip->max_voltage_mv = voltage;
        pm_chg_vddmax_set( the_chip, voltage );
    }else{
    }
}
#ifdef NCM_PM_CHG_ICALC_ENABLE
static int ncm_chg_set_charging_calc_current(unsigned int *calc)
{
    int rc = 0;
	int result_current = -1;
    int vbatt;
    int vchg;
    int vlost;
    int dc_present, usb_present;
    struct pm8xxx_adc_chan_result result;
    int temp;
    vbatt = qcom_get_prop_battery_uvolts(the_chip);
	pr_debug("[PM]vbatt = %d\n",vbatt);
    dc_present = is_dc_chg_plugged_in(the_chip);
    usb_present = is_usb_chg_plugged_in(the_chip);
    if(dc_present){
        rc = pm8xxx_adc_read(CHANNEL_DCIN, &result);
        vchg = (int)(result.physical);
    }else if(usb_present){
        rc = pm8xxx_adc_read(CHANNEL_USBIN, &result);
        vchg = (int)(result.physical);
    }else{
        vchg = 0;
    }
	pr_debug("[PM]vchg = %d\n",vchg);
    if(vchg - vbatt <= 0){
        return -1;
    }else{
        vlost = ( vchg - vbatt ) / 1000;
    }
    if( the_chip->ncm_chg_chip.obs_data.param[BATTIM_OBS_IDX_CAMERA].value == 1 ) {
        temp = (int)pm_param.pm_pw_pd_max.data[PM_PW_PD_MAX_CAM]*1000 / ( vlost );
        if(temp <= 0) {
            pr_err("[PM]negative value was set for CAMERA: result=%d, vbatt=%d, vchg=%d\n",temp, vbatt, vchg);
            return -1;
        }
        else {
            result_current = temp;
        }
    }
    if( the_chip->ncm_chg_chip.obs_data.param[BATTIM_OBS_IDX_CONNECT_DATA].value == PM_OBS_RF_LTE_DATA_MODE ) {
        temp = (int)pm_param.pm_pw_pd_max.data[PM_PW_PD_MAX_LTE]*1000 / ( vlost ) ;
        if(temp <= 0) {
            pr_err("[PM]negative value was set for LTE: result=%d, vbatt=%d, vchg=%d\n", temp, vbatt, vchg);
            return -1;
        }
        else {
            if( result_current < temp ) {
                result_current = temp;
            }
        }
    }
    if( result_current == -1 ) {
        temp = (int)pm_param.pm_pw_pd_max.data[2]*1000 / ( vlost );
        if(temp <= 0) {
            pr_err("[PM]negative value was set for OTHER: current = %d", temp);
            return -1;
        }
        else {
            result_current = temp;
        }
    }
    *calc = result_current;
    the_chip->ncm_chg_chip.calc_current = result_current;
    return 0;
}
#endif
static void ncm_chg_set_charging_ibatmax(void)
{
    unsigned int i_current;
    unsigned int calc;
    int rc;
    int status = 0;
    rc = getProtect_Status(IDX_XO_TEMP_LIMIT, &status);
    if(rc == false){
        return;
    }else{
    }
    if(status == ProtectNone){
        i_current = pm_param.pm_chg_setting.data[PM_I_CHG_HIGH1];
    }else{
        i_current = pm_param.pm_chg_setting.data[PM_I_CHG_HIGH2];
    }
#ifdef NCM_PM_CHG_ICALC_ENABLE
    rc = ncm_chg_set_charging_calc_current(&calc);
#else
	rc = -1;
#endif
    if(rc){
        calc = 0xFFFF;
    }else{
    }
    if( calc < i_current ){
        i_current = calc;
    }else{
    }
    if( i_current != the_chip->max_bat_chg_current ){
        the_chip->max_bat_chg_current = i_current;
        pm_chg_ibatmax_set(the_chip, i_current);
    }else{
    }
}
static void ncm_chg_set_charging_vbatdet(void)
{
    pm_chg_vbatdet_set( the_chip, PM8921_CHG_VBATDET_MAX );
}
static void ncm_chg_set_charging_vbatdet_eoc(void)
{
    int vbatdet = 0;
    int vfull_pict = 0;
    int vfull_pict_adj = 0;
    int vdet_6 = 0;
    vfull_pict     = (int)(pm_param.pm_pw_ctrl_vdet.data[PM_VFULL_PICT]);
    vfull_pict_adj = (int)(pm_param.pm_pw_ctrl_vdet.data[PM_VFULL_PICT_ADJ]);
    vdet_6         = (int)(pm_param.pm_pw_ctrl_vdet.data[PM_VDET_6]);
    vbatdet = ((vfull_pict - vfull_pict_adj) - vdet_6) * 9 / 10 + vdet_6;
    pm_chg_vbatdet_set( the_chip, vbatdet );
}
static void ncm_pm8921_charger_vbus_draw(unsigned int mA)
{
    unsigned int set_mA;
    if (mA == NCM_IUSB_DCP_MA) {
        mA = NCM_IUSB_OTHER_DCP_MA;
    }
    else {
    }
    ncm_usb_charger_current_from_usb = mA;
    if(mA > 500){
        set_mA = 500;
        pr_info("[PM]Workaround rush current. vbus pre set =%d\n", set_mA);
    }else{
        set_mA = mA;
    }
    __pm8921_charger_vbus_draw(set_mA);
}
static void ncm_charge_do_steps_for_dcp(struct pm8921_chg_chip *chip)
{
    int rc = 0;
    rc += pm_chg_iusbmax_set(chip,4);
    mdelay(20);
    rc += pm_chg_iusbmax_set(chip,6);
    mdelay(20);
    rc += pm_chg_iusbmax_set(chip,7);
    mdelay(20);
    if (rc) {
        pr_err("Failed to do steps for dcp rc=%d\n", rc);
    }
}
#define NCM_CHG_IBATMAX_LIMITATION 500
static void ncm_chg_set_charging_ichgusb(void)
{
    static int update = 0;
    unsigned long flags;
	struct pm8921_chg_chip *chip = the_chip;
    spin_lock_irqsave(&vbus_lock, flags);
    if(ncm_chg_uv_flag){
	    pr_info("[PM] Measures against USB rush current.\n");
	    update = 0;
		ncm_chg_uv_flag = FALSE;
    	__pm8921_charger_vbus_draw(NCM_CHG_IBATMAX_LIMITATION);
	}else{
	}
    if (ncm_chg_usbsw_ov_flag == true){
    	__pm8921_charger_vbus_draw(NCM_CHG_IBATMAX_LIMITATION);
    }
    else {
        if(update){
            update = 0;
#if defined(CONFIG_FEATURE_NCMC_RUBY)
        	if ((ncm_usb_charger_current_from_usb == NCM_IUSB_DCP_MA)||
        		(ncm_usb_charger_current_from_usb == NCM_IUSB_OTHER_DCP_MA)){
            	ncm_charge_do_steps_for_dcp(chip);
            }else{
            }
#endif
        	__pm8921_charger_vbus_draw(ncm_usb_charger_current_from_usb);
            pr_info("[PM]Workaround rush current. vbus set =%d\n", ncm_usb_charger_current_from_usb);
        }
    }
    if(ncm_usb_charger_current_from_usb != ncm_usb_charger_current_present){
        update++;
    }
    spin_unlock_irqrestore(&vbus_lock, flags);
}
static void ncm_chg_set_charging_iterm(void)
{
	int term_current;
	term_current = pm_param.pm_chg_setting.data[PM_I_CHG_COMP];
    if(term_current != the_chip->term_current){
        the_chip->term_current = term_current;
        pm_chg_iterm_set( the_chip, term_current );
    }else{
    }
}
static void ncm_chg_set_charging_timer_fast(void)
{
    unsigned int chg_timer = (unsigned int) pm_param.pm_chg_setting.data[PM_TIMER_CHG_FC];
    if(chg_timer != the_chip->safety_time){
        the_chip->safety_time = chg_timer;
        pm_chg_tchg_max_set( the_chip, chg_timer );
    }else{
    }
}
static void ncm_chg_set_charging_err_cycle(void)
{
	int chg_err_cycle = pm_param.pm_chg_setting.data[PM_BATTERY_ERR_CYCLE];
    if(chg_err_cycle != the_chip->ncm_chg_chip.chg_err_cycle){
        the_chip->ncm_chg_chip.chg_err_cycle = chg_err_cycle;
    }else{
    }
}
static void ncm_chg_set_charging_parameter(void)
{
    ncm_chg_set_charging_vddmax();
	if(!factory_mode_flag_apps){
    	ncm_chg_set_charging_ichgusb();
	}
    ncm_chg_set_charging_ibatmax();
	ncm_chg_set_charging_iterm();
    ncm_chg_set_charging_timer_fast();
	ncm_chg_set_charging_err_cycle();
}
static void ncm_watch_chg_work_for_charging(struct pm8921_chg_chip *chip)
{
    unsigned long flags;
    bool exec = FALSE;
	static int count = 0;
	ncm_chg_set_charging_parameter();
	count = 1 - count;
	if( count ){
        spin_lock_irqsave(&chip->ncm_chg_chip.eoc_slock,flags);
        if( chip->ncm_chg_chip.ncm_chg_eoc_work_flag == TRUE ){
            chip->ncm_chg_chip.ncm_chg_eoc_work_flag = FALSE;
            exec = TRUE;
        }else{
        }
        spin_unlock_irqrestore(&chip->ncm_chg_chip.eoc_slock,flags);
    }else{
    }
    if(exec == FALSE){
        return;
    }else{
    }
    eoc_worker(chip);
}
#define NCM_BATT_ALARM_USE_PWM			1
#define NCM_BATT_ALARM_PWM_SCALER		7
#define NCM_BATT_ALARM_PWM_DIVIDER		4
void ncm_register_batt_alarm(void)
{
	int rc;
	if(factory_mode_flag_apps)
		return;
	rc = pm8xxx_batt_alarm_disable(PM8XXX_BATT_ALARM_LOWER_COMPARATOR);
	if (rc) {
		pr_err("%s:BATT_ALARM_LOWER unable to set disable state\n", __func__);
		goto err;
	}
	pr_info("[PM] %s threshold_set:%d \n", __func__, pm_param.pm_bat_alm_cut_off.data[PM_DET_VOLT_UP_LOW]);
	rc = pm8xxx_batt_alarm_threshold_set(PM8XXX_BATT_ALARM_LOWER_COMPARATOR,
										pm_param.pm_bat_alm_cut_off.data[PM_DET_VOLT_UP_LOW]);
	if (rc) {
		pr_err("[PM] %s: unable to set batt alarm threshold\n", __func__);
		goto err;
	}
	pr_info("[PM] %s hold_time_set:%d \n", __func__, pm_param.pm_bat_alm_cut_off.data[PM_MEAS_TIME]);
	rc = pm8xxx_batt_alarm_hold_time_set(pm_param.pm_bat_alm_cut_off.data[PM_MEAS_TIME]);
	if (rc) {
		pr_err("[PM] %s: unable to set batt alarm hold time\n", __func__);
		goto err;
	}
	rc = pm8xxx_batt_alarm_pwm_rate_set(NCM_BATT_ALARM_USE_PWM,
                                        pm_param.pm_bat_alm_cut_off.data[PM_CLOCK_SCALER],
                                        pm_param.pm_bat_alm_cut_off.data[PM_CLOCK_DIVIDER]);
	if (rc) {
		pr_err("[PM] %s: unable to set batt alarm pwm rate\n", __func__);
		goto err;
	}
	rc = pm8xxx_batt_alarm_register_notifier(&alarm_notifier);
	if (rc) {
		pr_err("[PM] %s: unable to register alarm notifier\n", __func__);
		goto err;
	}
    pr_info("[PM] %s alarm_enable:%d \n", __func__, pm_param.pm_bat_alm_cut_off.data[PM_BAT_ALM_ON_OFF]);
	if (pm_param.pm_bat_alm_cut_off.data[PM_BAT_ALM_ON_OFF] != 1) {
		rc = pm8xxx_batt_alarm_enable(PM8XXX_BATT_ALARM_LOWER_COMPARATOR);
		if (rc) {
			pr_err("[PM] %s: unable to set state\n", __func__);
		}
	}
err:
	return ;
}
static int ncm_notify_batt_alarm(struct notifier_block *nb,
                                 unsigned long status, void *unused)
{
	int rc;
	pr_info("%s: status: %lu\n", __func__, status);
	switch (status) {
	case 0:
		pr_err("%s: spurious interrupt\n", __func__);
		break;
	case PM8XXX_BATT_ALARM_STATUS_BELOW_LOWER:
		pr_info("Battery alarm occurred\n");
		rc = pm8xxx_batt_alarm_disable(PM8XXX_BATT_ALARM_LOWER_COMPARATOR);
		if (rc) {
			pr_err("%s: unable to set alarm state\n", __func__);
		}
		wake_lock_timeout(&cut_off_delay_wl, NCM_CHG_CUT_OFF_DELAY_TIME);
		ncm_notify_event(the_chip, NCM_EVENT_CUT_OFF);
		break;
	case PM8XXX_BATT_ALARM_STATUS_ABOVE_UPPER:
		pr_err("%s: trip of high threshold\n", __func__);
		break;
	default:
		pr_err("%s: error received\n", __func__);
	}
	return 0;
}
static void get_camera_lte_mm_status(int32_t *camera_status, int32_t *connect_status, int32_t *mm_status)
{
	*connect_status = the_chip->ncm_chg_chip.obs_data.param[BATTIM_OBS_IDX_CONNECT_DATA].value;
	*camera_status  = the_chip->ncm_chg_chip.obs_data.param[BATTIM_OBS_IDX_CAMERA].value;
	*mm_status      = the_chip->ncm_chg_chip.obs_data.param[BATTIM_OBS_IDX_MULTIMEDIA].value;
}
static void ncm_xo_threshold_value_check(int *off_threshold_value, int *limit_threshold_value, int *recover_threshold_value )
{
	int32_t camera_status;
	int32_t lte_status    = PM_OBS_RF_SLEEP_MODE;
	int32_t mm_status;
	nc_pr_debug(": in\n");
	get_camera_lte_mm_status(&camera_status, &lte_status, &mm_status);
	if (camera_status == 1){
		if (lte_status == PM_OBS_RF_LTE_DATA_MODE){
			if(pm_param.pm_charge_off_on_xo_temp_cam.data[PM_XO_TEMP_ERR_HIGH2_CAM] <=
			   pm_param.pm_charge_off_on_xo_temp_lte.data[PM_XO_TEMP_ERR_HIGH2_LTE]) {
				*off_threshold_value = pm_param.pm_charge_off_on_xo_temp_lte.data[PM_XO_TEMP_ERR_HIGH2_LTE];
			}
			else {
				*off_threshold_value = pm_param.pm_charge_off_on_xo_temp_cam.data[PM_XO_TEMP_ERR_HIGH2_CAM];
			}
			if(pm_param.pm_charge_off_on_xo_temp_cam.data[PM_XO_TEMP_ERR_HIGH1_CAM] <=
			   pm_param.pm_charge_off_on_xo_temp_lte.data[PM_XO_TEMP_ERR_HIGH1_LTE]) {
				*limit_threshold_value = pm_param.pm_charge_off_on_xo_temp_lte.data[PM_XO_TEMP_ERR_HIGH1_LTE];
			}
			else {
				*limit_threshold_value = pm_param.pm_charge_off_on_xo_temp_cam.data[PM_XO_TEMP_ERR_HIGH1_CAM];
			}
			if(pm_param.pm_charge_off_on_xo_temp_cam.data[PM_XO_TEMP_ERR_HIGH3_CAM] <=
			   pm_param.pm_charge_off_on_xo_temp_lte.data[PM_XO_TEMP_ERR_HIGH3_LTE]) {
				*recover_threshold_value = pm_param.pm_charge_off_on_xo_temp_lte.data[PM_XO_TEMP_ERR_HIGH3_LTE];
			}
			else {
				*recover_threshold_value = pm_param.pm_charge_off_on_xo_temp_cam.data[PM_XO_TEMP_ERR_HIGH3_CAM];
			}
		}
		else {
			*off_threshold_value     = pm_param.pm_charge_off_on_xo_temp_cam.data[PM_XO_TEMP_ERR_HIGH2_CAM];
			*limit_threshold_value   = pm_param.pm_charge_off_on_xo_temp_cam.data[PM_XO_TEMP_ERR_HIGH1_CAM];
			*recover_threshold_value = pm_param.pm_charge_off_on_xo_temp_cam.data[PM_XO_TEMP_ERR_HIGH3_CAM];
		}
	}
	else {
		if (lte_status == PM_OBS_RF_LTE_DATA_MODE){
			*off_threshold_value     = pm_param.pm_charge_off_on_xo_temp_lte.data[PM_XO_TEMP_ERR_HIGH2_LTE];
			*limit_threshold_value   = pm_param.pm_charge_off_on_xo_temp_lte.data[PM_XO_TEMP_ERR_HIGH1_LTE];
			*recover_threshold_value = pm_param.pm_charge_off_on_xo_temp_lte.data[PM_XO_TEMP_ERR_HIGH3_LTE];
		}
		else {
			*off_threshold_value     = pm_param.pm_charge_off_on_xo_temp.data[PM_XO_TEMP_ERR_HIGH2];
			*limit_threshold_value   = pm_param.pm_charge_off_on_xo_temp.data[PM_XO_TEMP_ERR_HIGH1];
			*recover_threshold_value = pm_param.pm_charge_off_on_xo_temp.data[PM_XO_TEMP_ERR_HIGH3];
		}
	}
	if (mm_status == 1){
		if (camera_status != 1 &&
		    lte_status != PM_OBS_RF_LTE_DATA_MODE) {
			*off_threshold_value     = pm_param.pm_charge_off_on_xo_temp_mm.data[PM_XO_TEMP_ERR_HIGH2_MM];
			*limit_threshold_value   = pm_param.pm_charge_off_on_xo_temp_mm.data[PM_XO_TEMP_ERR_HIGH1_MM];
			*recover_threshold_value = pm_param.pm_charge_off_on_xo_temp_mm.data[PM_XO_TEMP_ERR_HIGH3_MM];
		} else {
			if (pm_param.pm_charge_off_on_xo_temp_mm.data[PM_XO_TEMP_ERR_HIGH2_MM] >
			    *off_threshold_value) {
				*off_threshold_value = pm_param.pm_charge_off_on_xo_temp_mm.data[PM_XO_TEMP_ERR_HIGH2_MM];
			}
			if (pm_param.pm_charge_off_on_xo_temp_mm.data[PM_XO_TEMP_ERR_HIGH1_MM] >
			    *limit_threshold_value) {
				*limit_threshold_value = pm_param.pm_charge_off_on_xo_temp_mm.data[PM_XO_TEMP_ERR_HIGH1_MM];
			}
			if (pm_param.pm_charge_off_on_xo_temp_mm.data[PM_XO_TEMP_ERR_HIGH3_MM] >
			    *recover_threshold_value) {
				*recover_threshold_value = pm_param.pm_charge_off_on_xo_temp_mm.data[PM_XO_TEMP_ERR_HIGH3_MM];
			}
		}
	}
	else {
	}
	return;
}
static int ncm_chg_protect_therm_chk(void)
{
	int ret     = 0;
	int fncret  = NCM_CHG_PROTECT_RET_OK;
	int ncm_batt_therm_judge        = NCM_CHG_PROTECT_THERM_BATT_NONE;
	int ncm_XO_therm_judge          = NCM_CHG_PROTECT_THERM_XO_NONE;
	int ncm_XO_therm_recover_judge  = NCM_CHG_PROTECT_THERM_XO_RECOVER_NONE;
	nc_pr_debug(": in\n");
	ret = ncm_chg_protect_therm_chk_batt(&ncm_batt_therm_judge);
	if (ret == NCM_CHG_PROTECT_RET_OK){
	}
	else {
		fncret = NCM_CHG_PROTECT_RET_NG;
	}
	ret = ncm_chg_protect_therm_chk_XO(&ncm_XO_therm_judge, &ncm_XO_therm_recover_judge);
	if (ret == NCM_CHG_PROTECT_RET_OK){
	}
	else {
		fncret = NCM_CHG_PROTECT_RET_NG;
	}
	ret = ncm_chg_protect_therm_judge_batt(ncm_batt_therm_judge, ncm_XO_therm_recover_judge);
	if (ret == NCM_CHG_PROTECT_RET_OK){
	}
	else {
		fncret = NCM_CHG_PROTECT_RET_NG;
	}
	ret = ncm_chg_protect_therm_judge_xo(ncm_batt_therm_judge, ncm_XO_therm_judge, ncm_XO_therm_recover_judge);
	if (ret == NCM_CHG_PROTECT_RET_OK){
	}
	else {
		fncret = NCM_CHG_PROTECT_RET_NG;
	}
	return fncret;
}
static int ncm_chg_protect_therm_chk_batt(int *batt_judge_res)
{
	int ret    = 0;
	int temp   = 0;
	int uvolts = 0;
	int bat_adc_read_volts  = 0x0000;
	int bat_therm_judge = NCM_CHG_PROTECT_THERM_BATT_NONE;
	nc_pr_debug(": in\n");
	if (batt_judge_res == NULL){
		pr_err("[%s]param error\n", __func__);
		return NCM_CHG_PROTECT_RET_NG;
	}
	ret = ncm_pm8921_get_batt_temperature_uv(the_chip, &temp, &uvolts);
	if (0 == ret) {
		bat_adc_read_volts = (uint16_t)(uvolts / 1000);
		bat_adc_alarm_read_volts = bat_adc_read_volts;
	}
	else {
		pr_err("[%s]error get ADC Value[batt_therm]. rc[%d]\n", __func__, ret);
		return NCM_CHG_PROTECT_RET_NG;
	}
	if(bat_adc_read_volts >= pm_param.pm_pw_ctrl_temp_err.data[PM_TEMP_ERR_LOW1]){
		bat_therm_judge = NCM_CHG_PROTECT_THERM_BATT_LOW_ERR;
	}
	else if (bat_adc_read_volts <= pm_param.pm_pw_ctrl_temp_err.data[PM_TEMP_ERR_HIGH2]){
		bat_therm_judge = NCM_CHG_PROTECT_THERM_BATT_HIGH_ERR;
	}
	else {
		if (bat_adc_read_volts <= pm_param.pm_pw_ctrl_temp_err.data[PM_TEMP_ERR_HIGH1]){
			bat_therm_judge = NCM_CHG_PROTECT_THERM_BATT_HOT;
		}
		else if (( bat_adc_read_volts > pm_param.pm_pw_ctrl_temp_err.data[PM_TEMP_ERR_HIGH1]) &&
				( bat_adc_read_volts  < pm_param.pm_pw_ctrl_temp_err.data[PM_TEMP_ERR_LOW2])   ){
			bat_therm_judge = NCM_CHG_PROTECT_THERM_BATT_NORMAL;
		}
		else {
			bat_therm_judge = NCM_CHG_PROTECT_THERM_BATT_COLD;
		}
	}
	*batt_judge_res = bat_therm_judge;
	nc_pr_debug("bat_adc_read_volts[%x] judge[%d]\n",bat_adc_read_volts, bat_therm_judge);
	return NCM_CHG_PROTECT_RET_OK;
}
static int ncm_chg_protect_therm_chk_XO(int *xo_judge_res, int *xo_recover_res)
{
	int ret    = 0;
	int temp   = 0;
	int uvolts = 0;
	int xo_adc_read_volts = 0;
	int xo_off_threshold_value      = 0;
	int xo_limit_threshold_value    = 0;
	int xo_recover_threshold_value  = 0;
	int xo_therm_judge = NCM_CHG_PROTECT_THERM_XO_NONE;
	nc_pr_debug(": in\n");
	if (xo_judge_res == NULL){
		pr_err("[%s]param error\n", __func__);
		return NCM_CHG_PROTECT_RET_NG;
	}
	ncm_xo_threshold_value_check(&xo_off_threshold_value, &xo_limit_threshold_value, &xo_recover_threshold_value);
	ret = ncm_pm8921_get_xo_temperature(the_chip, &temp, &uvolts);
	if (0 == ret) {
		xo_adc_read_volts = (uint16_t)(uvolts / 1000);
	}
	else {
		pr_err("[%s]error get ADC Value[XO_therm]. rc[%d]\n", __func__, ret);
		return NCM_CHG_PROTECT_RET_NG;
	}
	if (xo_adc_read_volts <= xo_off_threshold_value){
		xo_therm_judge = NCM_CHG_PROTECT_THERM_XO_HIGH_ERR;
	}
	else if (xo_adc_read_volts > xo_limit_threshold_value){
		xo_therm_judge = NCM_CHG_PROTECT_THERM_XO_NORMAL;
	}
	else {
		xo_therm_judge = NCM_CHG_PROTECT_THERM_XO_HIGH_CHK;
	}
	*xo_judge_res = xo_therm_judge;
	if (xo_adc_read_volts > xo_recover_threshold_value){
		*xo_recover_res = NCM_CHG_PROTECT_THERM_XO_RECOVER_NORMAL;
	}
	else {
		*xo_recover_res = NCM_CHG_PROTECT_THERM_XO_RECOVER_HIGH;
	}
	nc_pr_debug("xo_adc_read_volts[%x] judge[%d]\n",xo_adc_read_volts, xo_therm_judge);
	return NCM_CHG_PROTECT_RET_OK;
}
static int ncm_chg_protect_therm_judge_batt(int batt_judge_res, int xo_recover_res)
{
	int ret    = 0;
	int ncm_chg_protect_sts_active = 0;
	int ncm_chg_protect_sts_batt_err   = 0;
	int ncm_chg_protect_sts_batt_limit = 0;
	nc_pr_debug(": in\n");
	ret = getProtect_Status(IDX_BATT_TEMP_ERR,   &ncm_chg_protect_sts_batt_err);
	ret = getProtect_Status(IDX_BATT_TEMP_LIMIT, &ncm_chg_protect_sts_batt_limit);
	if (ncm_chg_protect_sts_batt_err > 0) {
		ncm_chg_protect_sts_active = IDX_BATT_TEMP_ERR;
	} else {
		if (ncm_chg_protect_sts_batt_limit > 0) {
			ncm_chg_protect_sts_active = IDX_BATT_TEMP_LIMIT;
		} else {
			ncm_chg_protect_sts_active = IDX_NORMAL;
		}
	}
	if (ret == true) {
		switch (ncm_chg_protect_sts_active) {
		case IDX_BATT_TEMP_ERR:
			switch (batt_judge_res) {
			case NCM_CHG_PROTECT_THERM_BATT_NORMAL:
				if (xo_recover_res == NCM_CHG_PROTECT_THERM_XO_RECOVER_NORMAL) {
					ncm_batt_temp_low_flag = false;
					set_status(IDX_BATT_TEMP_ERR, ProtectNone);
				} else {
					set_status(IDX_BATT_TEMP_ERR, ProtectForceOff);
					ncm_batt_temp_recover_flag = true;
				}
				break;
			case NCM_CHG_PROTECT_THERM_BATT_HIGH_ERR:
			case NCM_CHG_PROTECT_THERM_BATT_HOT:
			case NCM_CHG_PROTECT_THERM_BATT_COLD:
			case NCM_CHG_PROTECT_THERM_BATT_LOW_ERR:
				break;
			default:
				pr_err("[%s]BatTempErr Unknown status.[%d]\n", __func__, batt_judge_res);
				break;
			}
			break;
		case IDX_BATT_TEMP_LIMIT:
			switch (batt_judge_res) {
			case NCM_CHG_PROTECT_THERM_BATT_NORMAL:
			case NCM_CHG_PROTECT_THERM_BATT_COLD:
				set_status(IDX_BATT_TEMP_LIMIT, ProtectNone);
				break;
			case NCM_CHG_PROTECT_THERM_BATT_HIGH_ERR:
			case NCM_CHG_PROTECT_THERM_BATT_LOW_ERR:
				set_status(IDX_BATT_TEMP_ERR,   ProtectOn);
				set_status(IDX_BATT_TEMP_LIMIT, ProtectForceOff);
				break;
			case NCM_CHG_PROTECT_THERM_BATT_HOT:
				break;
			default:
				pr_err("[%s]BatTempLimit Unknown status.[%d]\n", __func__, batt_judge_res);
				break;
			}
			break;
		default:
			switch (batt_judge_res) {
			case NCM_CHG_PROTECT_THERM_BATT_NORMAL:
			case NCM_CHG_PROTECT_THERM_BATT_COLD:
				break;
			case NCM_CHG_PROTECT_THERM_BATT_HOT:
			case NCM_CHG_PROTECT_THERM_BATT_HIGH_ERR:
				ncm_batt_temp_recover_flag = false;
				if (ncm_chg_start_flag == true) {
					set_status(IDX_BATT_TEMP_ERR, ProtectOn);
				} else {
					set_status(IDX_BATT_TEMP_LIMIT, ProtectOn);
				}
				break;
			case NCM_CHG_PROTECT_THERM_BATT_LOW_ERR:
				ncm_batt_temp_low_flag = true;
				ncm_batt_temp_recover_flag = false;
				set_status(IDX_BATT_TEMP_ERR,   ProtectOn);
				break;
			default:
				pr_err("[%s]BatTempNormal Unknown status.[%d]\n", __func__, batt_judge_res);
				break;
			}
			ncm_chg_start_flag = false;
			break;
		}
	} else {
		pr_err("[%s]error get status.\n", __func__);
		return NCM_CHG_PROTECT_RET_NG;
	}
	return NCM_CHG_PROTECT_RET_OK;
}
static int ncm_chg_protect_therm_judge_xo(int batt_judge_res, int xo_judge_res, int xo_recover_res)
{
	int ret = 0;
	int ncm_chg_protect_sts_active = 0;
	int ncm_chg_protect_sts_xo_err     = 0;
	int ncm_chg_protect_sts_xo_limit   = 0;
	nc_pr_debug(": in\n");
	ret = getProtect_Status(IDX_XO_TEMP,         &ncm_chg_protect_sts_xo_err);
	ret = getProtect_Status(IDX_XO_TEMP_LIMIT,   &ncm_chg_protect_sts_xo_limit);
	if (ncm_chg_protect_sts_xo_err > 0) {
		ncm_chg_protect_sts_active = IDX_XO_TEMP;
	} else {
		if (ncm_chg_protect_sts_xo_limit > 0) {
			ncm_chg_protect_sts_active = IDX_XO_TEMP_LIMIT;
		} else {
			ncm_chg_protect_sts_active = IDX_NORMAL;
		}
	}
	if (ret == true) {
		switch (ncm_chg_protect_sts_active) {
		case IDX_XO_TEMP:
			switch (xo_judge_res) {
			case NCM_CHG_PROTECT_THERM_XO_NORMAL:
			case NCM_CHG_PROTECT_THERM_XO_HIGH_ERR:
			case NCM_CHG_PROTECT_THERM_XO_HIGH_CHK:
				if (xo_recover_res == NCM_CHG_PROTECT_THERM_XO_RECOVER_NORMAL) {
					if (batt_judge_res == NCM_CHG_PROTECT_THERM_BATT_NORMAL) {
						set_status(IDX_XO_TEMP, ProtectNone);
					} else {
						set_status(IDX_XO_TEMP, ProtectForceOff);
						ncm_xo_temp_recover_flag = true;
					}
				} else {
				}
				break;
			default:
				pr_err("[%s]XOTempErr Unknown status.[%d]\n", __func__, xo_judge_res);
				break;
			}
			break;
		case IDX_XO_TEMP_LIMIT:
			switch (xo_judge_res) {
			case NCM_CHG_PROTECT_THERM_XO_NORMAL:
				set_status(IDX_XO_TEMP_LIMIT, ProtectNone);
				break;
			case NCM_CHG_PROTECT_THERM_XO_HIGH_ERR:
				set_status(IDX_XO_TEMP,       ProtectOn);
				set_status(IDX_XO_TEMP_LIMIT, ProtectForceOff);
				ncm_xo_temp_recover_flag = false;
				break;
			case NCM_CHG_PROTECT_THERM_XO_HIGH_CHK:
				if (ncm_batt_temp_recover_flag == true &&
				    xo_recover_res == NCM_CHG_PROTECT_THERM_XO_RECOVER_NORMAL) {
					set_status(IDX_XO_TEMP_LIMIT, ProtectNone);
				} else {
					if (xo_recover_res == NCM_CHG_PROTECT_THERM_XO_RECOVER_HIGH) {
						ncm_xo_temp_recover_flag = false;
					}
				}
				break;
			default:
				pr_err("[%s]XO TempLimit Unknown status.[%d]\n", __func__, xo_judge_res);
				break;
			}
			break;
		default:
			switch (xo_judge_res) {
			case NCM_CHG_PROTECT_THERM_XO_NORMAL:
				break;
			case NCM_CHG_PROTECT_THERM_XO_HIGH_CHK:
				set_status(IDX_XO_TEMP_LIMIT, ProtectOn);
				if (xo_recover_res == NCM_CHG_PROTECT_THERM_XO_RECOVER_HIGH) {
					ncm_xo_temp_recover_flag = false;
				}
				break;
			case NCM_CHG_PROTECT_THERM_XO_HIGH_ERR:
				set_status(IDX_XO_TEMP, ProtectOn);
				ncm_xo_temp_recover_flag = false;
				break;
			default:
				pr_err("[%s]XO TempNormal Unknown status.[%d]\n", __func__, xo_judge_res);
				break;
			}
			break;
		}
	} else {
		pr_err("[%s]error get status.\n", __func__);
		return NCM_CHG_PROTECT_RET_NG;
	}
	return NCM_CHG_PROTECT_RET_OK;
}
#if defined(CONFIG_FEATURE_NCMC_POWER) && !defined(CONFIG_FEATURE_NCMC_RUBY)
static int ncm_chg_protect_usbsw(void)
{
	int ret = 0;
	unsigned char cur_ovp_ocp_sts = USB_SW_STATE_NORMAL;
	nc_pr_debug(": in\n");
	ret = ncm_chg_protect_get_usbsw_sts(&cur_ovp_ocp_sts);
	if (ret != NCM_CHG_PROTECT_RET_OK){
	}
	ret = ncm_chg_protect_ovp_judge_usbsw(cur_ovp_ocp_sts);
	if (ret != NCM_CHG_PROTECT_RET_OK){
		pr_err("[%s]error chk USB-SW OVP.\n", __func__);
		return NCM_CHG_PROTECT_RET_NG;
	}
	ret = ncm_chg_protect_ocp_judge_usbsw(cur_ovp_ocp_sts);
	if (ret != NCM_CHG_PROTECT_RET_OK){
		pr_err("[%s]error chk USB-SW OCP.\n", __func__);
		return NCM_CHG_PROTECT_RET_NG;
	}
	if ((cur_ovp_ocp_sts & USB_SW_STATE_OVP_VB) == USB_SW_STATE_OVP_VB){
		ncm_chg_usbsw_ov_flag = true;
	}
	else {
	}
	ovp_ocp_alarm_state = cur_ovp_ocp_sts & ovp_ocp_prev_state;
	ovp_ocp_prev_state = cur_ovp_ocp_sts;
	return NCM_CHG_PROTECT_RET_OK;
}
#endif
#if defined(CONFIG_FEATURE_NCMC_POWER) && !defined(CONFIG_FEATURE_NCMC_RUBY)
static int ncm_chg_protect_get_usbsw_sts(unsigned char *cur_ovp_ocp_sts)
{
	int ret = USB_SW_OK;
	unsigned char ovp_ocp_current_state        = USB_SW_STATE_NORMAL;
	nc_pr_debug(": in\n");
	if (cur_ovp_ocp_sts == NULL){
		pr_err("[%s]param error\n", __func__);
		return NCM_CHG_PROTECT_RET_NG;
	}
	ret = usb_sw_get_current_voltage_state(&ovp_ocp_current_state);
	if (ret == USB_SW_OK){
		nc_pr_debug("USB-SW get[%x]\n",ovp_ocp_current_state);
		*cur_ovp_ocp_sts = ovp_ocp_current_state;
	}
	else {
		pr_err("[%s]usb_sw_get_current_voltage_state:USB_SW_NG.\n", __func__ );
	}
	return NCM_CHG_PROTECT_RET_OK;
}
#endif
#if defined(CONFIG_FEATURE_NCMC_POWER) && !defined(CONFIG_FEATURE_NCMC_RUBY)
static int ncm_chg_protect_ovp_judge_usbsw(unsigned char cur_ovp_ocp_sts)
{
	int ret = 0;
	int ncm_chg_protect_sts_active = 0;
	unsigned char ovp_ocp_current_state_check  = USB_SW_STATE_NORMAL;
	nc_pr_debug(": in\n");
	nc_pr_debug("USB-SW sts[%x]\n",cur_ovp_ocp_sts);
	ovp_ocp_current_state_check = cur_ovp_ocp_sts & ovp_ocp_prev_state;
	ret = get_active_status(&ncm_chg_protect_sts_active);
	if (ret == true){
		switch(ncm_chg_protect_sts_active)
		{
			case IDX_USB_OVP:
				if ((ovp_ocp_current_state_check & USB_SW_STATE_OVP_VB) != USB_SW_STATE_OVP_VB){
					set_status(IDX_USB_OVP, ProtectNone);
				}
				else {
				}
				break;
			default:
				if ((ovp_ocp_current_state_check & USB_SW_STATE_OVP_VB) == USB_SW_STATE_OVP_VB){
					set_status(IDX_USB_OVP, ProtectOn);
				}
				else {
				}
				break;
		}
	}
	else {
		pr_err("[%s]error get status.\n", __func__);
		return NCM_CHG_PROTECT_RET_NG;
	}
	return NCM_CHG_PROTECT_RET_OK;
}
#endif
#if defined(CONFIG_FEATURE_NCMC_POWER) && !defined(CONFIG_FEATURE_NCMC_RUBY)
static int ncm_chg_protect_ocp_judge_usbsw(unsigned char cur_ovp_ocp_sts)
{
	int ret = 0;
	int ncm_chg_protect_sts_active = 0;
	unsigned char ovp_ocp_current_state_check  = USB_SW_STATE_NORMAL;
	nc_pr_debug(": in\n");
	ovp_ocp_current_state_check = cur_ovp_ocp_sts & ovp_ocp_prev_state;
	ret = get_active_status(&ncm_chg_protect_sts_active);
	if (ret == true){
		switch(ncm_chg_protect_sts_active)
		{
			case IDX_USB_OCP:
				break;
			default:
				if ((ovp_ocp_current_state_check & USB_SW_STATE_OCP_VB) == USB_SW_STATE_OCP_VB){
					set_status(IDX_USB_OCP, ProtectOn);
				}
				else {
				}
				break;
		}
	}
	else {
		pr_err("[%s]error get status.\n", __func__);
		return NCM_CHG_PROTECT_RET_NG;
	}
	return NCM_CHG_PROTECT_RET_OK;
}
#endif
#if defined(CONFIG_FEATURE_NCMC_POWER) && defined(CONFIG_FEATURE_NCMC_RUBY)
static int ncm_chg_protect_usbin(void)
{
	int ret = 0;
	int cur_ovp_ocp_sts = NCM_CHG_USB_DISCONNECT;
	nc_pr_debug("in\n");
	if(ncm_is_connect_dcin() == NCM_CHG_DC_CONNECT) {
		return NCM_CHG_PROTECT_RET_OK;
	}
	cur_ovp_ocp_sts = ncm_is_connect_usbin();
	ret = ncm_chg_protect_ovp_judge_usbin(cur_ovp_ocp_sts);
	if (ret != NCM_CHG_PROTECT_RET_OK){
		pr_err("[%s]error chk USB-SW OVP.\n", __func__);
		return NCM_CHG_PROTECT_RET_NG;
	}
	ovp_ocp_alarm_state = cur_ovp_ocp_sts & ovp_ocp_prev_state;
	ovp_ocp_prev_state = cur_ovp_ocp_sts;
	return NCM_CHG_PROTECT_RET_OK;
}
#endif
#if defined(CONFIG_FEATURE_NCMC_POWER) && defined(CONFIG_FEATURE_NCMC_RUBY)
static int ncm_chg_protect_ovp_judge_usbin(unsigned char cur_ovp_ocp_sts)
{
	int ret = 0;
	int ncm_chg_protect_sts_active = 0;
	nc_pr_debug("in\n");
	nc_pr_debug("USBIN sts[%x]\n",cur_ovp_ocp_sts);
	ret = get_active_status(&ncm_chg_protect_sts_active);
	if (ret == true){
		switch(ncm_chg_protect_sts_active)
		{
			case IDX_USB_OVP:
				if (cur_ovp_ocp_sts != NCM_CHG_USB_CONNECT_OVP){
					set_status(IDX_USB_OVP, ProtectNone);
				}
				else {
				}
				break;
			default:
				if (cur_ovp_ocp_sts == NCM_CHG_USB_CONNECT_OVP){
					set_status(IDX_USB_OVP, ProtectOn);
				}
				else {
				}
				break;
		}
	}
	else {
		pr_err("[%s]error get status.\n", __func__);
		return NCM_CHG_PROTECT_RET_NG;
	}
	return NCM_CHG_PROTECT_RET_OK;
}
#endif
static int ncm_chg_protect_dc_chg(void)
{
	int ret = 0;
	int ncm_chg_protect_sts_active = 0;
	int dc_connect_sts = NCM_CHG_DC_DISCONNECT;
	int cur_dc_ovp_sts = 0;
	nc_pr_debug(": in\n");
	if(ncm_is_connect_usbchg() == NCM_CHG_USB_CONNECT) {
		return NCM_CHG_PROTECT_RET_OK;
	}
	dc_connect_sts = ncm_is_connect_dcin();
	ret = get_active_status(&ncm_chg_protect_sts_active);
	if (ret == true){
		switch(ncm_chg_protect_sts_active)
		{
			case IDX_DC_OVP:
				if (dc_connect_sts == NCM_CHG_DC_DISCONNECT){
					cur_dc_ovp_sts = 0;
				}
				else if( dc_connect_sts == NCM_CHG_DC_CONNECT){
					cur_dc_ovp_sts = 0;
				}
				else if (dc_connect_sts == NCM_CHG_DC_CONNECT_OVP){
					cur_dc_ovp_sts = 1;
				}
				else {
				}
				break;
			default:
				if (dc_connect_sts == NCM_CHG_DC_DISCONNECT){
					cur_dc_ovp_sts = 0;
				}
				else if( dc_connect_sts == NCM_CHG_DC_CONNECT){
					cur_dc_ovp_sts = 0;
				}
				else if (dc_connect_sts == NCM_CHG_DC_CONNECT_OVP){
					cur_dc_ovp_sts = 1;
				}
				else {
				}
				break;
		}
		nc_pr_debug("dc_connect_sts[%d] ovp[%d]\n",dc_connect_sts, cur_dc_ovp_sts);
		if (cur_dc_ovp_sts == 1){
			set_status(IDX_DC_OVP, ProtectOn);
		}
		else {
			set_status(IDX_DC_OVP, ProtectNone);
		}
	}
	else {
		pr_err("[%s]error get status.\n", __func__);
		return NCM_CHG_PROTECT_RET_NG;
	}
	dc_ovp_prev_state = cur_dc_ovp_sts;
	return NCM_CHG_PROTECT_RET_OK;
}
static int ncm_chg_protect_chager_remove(void)
{
	int ret = 0;
	int ncm_chg_protect_sts_active = 0;
	int dc_device_state = NCM_CHG_DC_DISCONNECT;
	int usb_device_state = NCM_CHG_USB_DISCONNECT;
	nc_pr_debug(": in\n");
	ret = get_active_status(&ncm_chg_protect_sts_active);
	if (ret == true){
		switch(ncm_chg_protect_sts_active)
		{
			case IDX_NORMAL:
				usb_device_state = ncm_is_connect_usbchg();
				if (usb_device_state == NCM_CHG_USB_DISCONNECT){
					ncm_chg_start_flag = true;
					ncm_chg_usbsw_ov_flag = false;
				}
				else {
				}
				return NCM_CHG_PROTECT_RET_NOCHG;
				break;
			default:
				dc_device_state = ncm_is_connect_dcin();
				usb_device_state = ncm_is_connect_usbchg();
				if (dc_device_state == NCM_CHG_DC_DISCONNECT){
					if (usb_device_state == NCM_CHG_USB_DISCONNECT){
						set_status(IDX_USB_OVP,         ProtectNone);
						set_status(IDX_USB_OCP,         ProtectNone);
						set_status(IDX_BATT_TEMP_ERR,   ProtectNone);
						set_status(IDX_BATT_TEMP_LIMIT, ProtectNone);
						set_status(IDX_XO_TEMP,         ProtectNone);
						set_status(IDX_XO_TEMP_LIMIT,   ProtectNone);
						set_status(IDX_DC_OVP,          ProtectNone);
						set_status(IDX_BATT_TEMP_ERR,   ProtectForceOff);
						set_status(IDX_BATT_TEMP_LIMIT, ProtectForceOff);
						set_status(IDX_XO_TEMP,         ProtectForceOff);
						set_status(IDX_XO_TEMP_LIMIT,   ProtectForceOff);
						ncm_batt_temp_recover_flag = false;
						ncm_xo_temp_recover_flag = false;
						pm_chg_charge_dis(the_chip, 0);
						ncm_chg_start_flag = true;
						ncm_chg_usbsw_ov_flag = false;
					}
					else {
					}
				}
				else {
				}
			break;
		}
	}
	else {
		pr_err("[%s]error get status.\n", __func__);
		return NCM_CHG_PROTECT_RET_NG;
	}
	return NCM_CHG_PROTECT_RET_OK;
}
static int ncm_charge_protection_main(int charge_src)
{
	int themchk_ret = NCM_CHG_PROTECT_RET_OK;
	int chg_ret     = NCM_CHG_PROTECT_RET_OK;
#if defined(CONFIG_FEATURE_NCMC_POWER) && !defined(CONFIG_FEATURE_NCMC_RUBY)
	int usbchk_ret  = NCM_CHG_PROTECT_RET_OK;
#else
#endif
    nc_pr_debug(": chage_src=%d\n" ,charge_src);
	themchk_ret = ncm_chg_protect_therm_chk();
	if (themchk_ret == NCM_CHG_PROTECT_RET_OK){
	}
	else {
		printk(KERN_ERR "[PM] %s: Err! therm protect\n", __func__);
	}
	if (charge_src == PM8921_CHG_SRC_NONE){
#if defined(CONFIG_FEATURE_NCMC_POWER) && !defined(CONFIG_FEATURE_NCMC_RUBY)
		usbchk_ret = ncm_chg_protect_usbsw();
		if (usbchk_ret == NCM_CHG_PROTECT_RET_OK){
		}
		else {
			printk(KERN_ERR "[PM] %s: Err!usb-sw protect\n", __func__);
		}
#else
#endif
		chg_ret = ncm_chg_protect_chager_remove();
		if (chg_ret == NCM_CHG_PROTECT_RET_OK){
		}
		else if (chg_ret == NCM_CHG_PROTECT_RET_NOCHG){
		}
		else {
			printk(KERN_ERR "[PM] %s: Err!chg-remove chk\n", __func__);
		}
	}
	else {
#if defined(CONFIG_FEATURE_NCMC_POWER) && !defined(CONFIG_FEATURE_NCMC_RUBY)
		usbchk_ret = ncm_chg_protect_usbsw();
		if (usbchk_ret == NCM_CHG_PROTECT_RET_OK){
		}
		else {
			printk(KERN_ERR "[PM] %s: Err!usb-sw protect\n", __func__);
		}
#else
#endif
	}
    return true;
}
int ncm_protect_action_Batt_Remove(ProtectIndex protect_index, ProtectType protect_type)
{
    nc_pr_debug("Charge Protect ON\n");
    return true;
}
int ncm_protect_action_DC_Ovp(ProtectIndex protect_index, ProtectType protect_type)
{
	nc_pr_debug("Charge Protect ON\n");
    printk(KERN_ERR "[T][ARM]Event:0x00 Info:0x%02x%02x%02x%02x\n",
                        (uint8_t)ncm_prev_chgstate,
                        (uint8_t)ncm_prev_chgtype,
                        0,
                        1
          );
    ncm_notify_event(the_chip, NCM_EVENT_CHARGE_PROTECT_DC_OVP);
    return true;
}
int ncm_protect_action_Usb_Ovp(ProtectIndex protect_index, ProtectType protect_type)
{
#if defined(CONFIG_FEATURE_NCMC_POWER) && !defined(CONFIG_FEATURE_NCMC_RUBY)
	int rc;
#endif
	nc_pr_debug("Charge Protect ON\n");
    printk(KERN_ERR "[T][ARM]Event:0x00 Info:0x%02x%02x%02x%02x\n",
                        (uint8_t)ncm_prev_chgstate,
                        (uint8_t)ncm_prev_chgtype,
                        ovp_ocp_alarm_state,
                        0
          );
#if defined(CONFIG_FEATURE_NCMC_POWER) && !defined(CONFIG_FEATURE_NCMC_RUBY)
	rc = pm_chg_iusbmax_set(the_chip, 1);
	if (rc) {
		pr_err("Failed to set usb max to %d rc=%d\n",
				usb_ma_table[1].usb_ma, rc);
	}
#endif
    ncm_notify_event(the_chip, NCM_EVENT_CHARGE_PROTECT_USB_OVP);
    return true;
}
int ncm_protect_action_Usb_Ocp(ProtectIndex protect_index, ProtectType protect_type)
{
	nc_pr_debug("Charge Protect ON\n");
    printk(KERN_ERR "[T][ARM]Event:0x0E Info:0x%02x%02x%02x\n",
                        (uint8_t)ncm_prev_chgstate,
                        (uint8_t)ncm_prev_chgtype,
                        ovp_ocp_alarm_state
          );
    ncm_notify_event(the_chip, NCM_EVENT_CHARGE_PROTECT_USB_OCP);
    return true;
}
int ncm_protect_action_Batt_TempErr(ProtectIndex protect_index, ProtectType protect_type)
{
    int rc;
    nc_pr_debug("Charge Protect ON\n");
    printk(KERN_ERR "[T][ARM]Event:0x01 Info:0x%02x%02x%04x%02x\n",
                        (uint8_t )ncm_chgmon_charger_state(),
                        (uint8_t )ncm_chgmon_charger_type(),
                        (uint16_t)bat_adc_alarm_read_volts,
                        (uint8_t )0x01
          );
    rc = pm_chg_charge_dis(the_chip, 1);
    if(rc == 0)
    {
        ncm_notify_event(the_chip, NCM_EVENT_CHARGE_PROTECT_BATT_ERR);
    }
    else
    {
        printk(KERN_ERR "[PM] %s:Failure stop of battery charge\n", __func__);
        return false;
    }
    return true;
}
int ncm_protect_action_Batt_TempLimit(ProtectIndex protect_index, ProtectType protect_type)
{
	nc_pr_debug("Charge Protect ON\n");
    printk(KERN_ERR "[T][ARM]Event:0x01 Info:0x%02x%02x%04x%02x\n",
                        (uint8_t )ncm_chgmon_charger_state(),
                        (uint8_t )ncm_chgmon_charger_type(),
                        (uint16_t)bat_adc_alarm_read_volts,
                        (uint8_t )0x01
          );
    return true;
}
int ncm_protect_action_XO_Temp(ProtectIndex protect_index, ProtectType protect_type)
{
    int rc;
    nc_pr_debug("Charge Protect ON\n");
    printk(KERN_ERR "[T][ARM]Event:0x01 Info:0x%02x%02x%04x%02x\n",
                        (uint8_t )ncm_chgmon_charger_state(),
                        (uint8_t )ncm_chgmon_charger_type(),
                        (uint16_t)bat_adc_alarm_read_volts,
                        (uint8_t )0x00
          );
    rc = pm_chg_charge_dis(the_chip, 1);
    if(rc == 0)
    {
        ncm_notify_event(the_chip, NCM_EVENT_CHARGE_PROTECT_XO_TEMP);
    }
    else
    {
        printk(KERN_ERR "[PM] %s:Failure stop of battery charge\n", __func__);
        return false;
    }
    return true;
}
int ncm_protect_action_XO_TempLimit(ProtectIndex protect_index, ProtectType protect_type)
{
    nc_pr_debug("Charge Protect ON\n");
    printk(KERN_ERR "[T][ARM]Event:0x01 Info:0x%02x%02x%04x%02x\n",
                        (uint8_t )ncm_chgmon_charger_state(),
                        (uint8_t )ncm_chgmon_charger_type(),
                        (uint16_t)bat_adc_alarm_read_volts,
                        (uint8_t )0x00
          );
    return true;
}
int ncm_protect_action_Charge_Timeout(ProtectIndex protect_index, ProtectType protect_type)
{
    unsigned long flags;
    int rc;
    nc_pr_debug("Charge Protect ON\n");
    printk(KERN_ERR "[T][ARM]Event:0x02 Info:0x%02x%02x\n",
        ncm_chgmon_charger_state(),
        ncm_chgmon_charger_type());
    spin_lock_irqsave(&the_chip->ncm_chg_chip.prtct_timer.slock, flags);
    the_chip->ncm_chg_chip.prtct_timer.cnt        = 0;
    the_chip->ncm_chg_chip.prtct_timer.is_timer   = false;
    the_chip->ncm_chg_chip.prtct_timer.is_timeout = true;
    spin_unlock_irqrestore(&the_chip->ncm_chg_chip.prtct_timer.slock, flags);
    rc = pm_chg_charge_dis(the_chip, 1);
    if(rc == 0) {
        ncm_notify_event(the_chip, NCM_EVENT_CHARGE_PROTECT_CHARGE_TIMEOUT);
    }
    else
    {
        printk(KERN_ERR "[PM] %s:Failure stop of battery charge\n", __func__);
        return false;
    }
    return true;
}
int ncm_protect_recover_Batt_Remove(ProtectIndex protect_index, ProtectType protect_type)
{
    nc_pr_debug("Charge Protect OFF\n");
    return true;
}
int ncm_protect_recover_DC_Ovp(ProtectIndex protect_index, ProtectType protect_type)
{
	nc_pr_debug("Charge Protect OFF\n");
    ncm_notify_event(the_chip, NCM_EVENT_CHARGE_PROTECT_NOTIFY);
    return true;
}
int ncm_protect_recover_Usb_Ovp(ProtectIndex protect_index, ProtectType protect_type)
{
	nc_pr_debug("Charge Protect OFF\n");
    return true;
}
int ncm_protect_recover_Usb_Ocp(ProtectIndex protect_index, ProtectType protect_type)
{
	nc_pr_debug("Charge Protect OFF\n");
    return true;
}
int ncm_protect_recover_Batt_TempErr(ProtectIndex protect_index, ProtectType protect_type)
{
    int rc;
    nc_pr_debug("Charge Protect OFF\n");
    ncm_xo_temp_recover_flag = false;
    rc = pm_chg_charge_dis(the_chip, 0);
    if (rc == 0)
    {
        ncm_notify_event(the_chip, NCM_EVENT_CHARGE_PROTECT_NOTIFY);
    }
    else
    {
        printk(KERN_ERR "[PM] %s:Failure restart of battery charge\n", __func__);
        return false;
    }
    return true;
}
int ncm_protect_recover_Batt_TempLimit(ProtectIndex protect_index, ProtectType protect_type)
{
	int rc;
	nc_pr_debug("Charge Protect OFF\n");
	if (ncm_xo_temp_recover_flag == true) {
		ncm_xo_temp_recover_flag = false;
		rc = pm_chg_charge_dis(the_chip, 0);
		if (rc == 0) {
			ncm_notify_event(the_chip, NCM_EVENT_CHARGE_PROTECT_NOTIFY);
		} else {
			printk(KERN_ERR "[PM] %s:Failure restart of battery charge\n", __func__);
			return false;
		}
	}
	return true;
}
int ncm_protect_recover_XO_Temp(ProtectIndex protect_index, ProtectType protect_type)
{
    int rc;
	nc_pr_debug("Charge Protect OFF\n");
    ncm_batt_temp_recover_flag = false;
    rc = pm_chg_charge_dis(the_chip, 0);
    if(rc == 0)
    {
        ncm_notify_event(the_chip, NCM_EVENT_CHARGE_PROTECT_NOTIFY);
    }
    else
    {
        printk(KERN_ERR "[PM] %s:Failure restart of battery charge\n", __func__);
        return false;
    }
    return true;
}
int ncm_protect_recover_XO_TempLimit(ProtectIndex protect_index, ProtectType protect_type)
{
	int rc;
	nc_pr_debug("Charge Protect OFF\n");
	if (ncm_batt_temp_recover_flag == true) {
		ncm_batt_temp_recover_flag = false;
		rc = pm_chg_charge_dis(the_chip, 0);
		if (rc == 0) {
			ncm_notify_event(the_chip, NCM_EVENT_CHARGE_PROTECT_NOTIFY);
		} else {
			printk(KERN_ERR "[PM] %s:Failure restart of battery charge\n", __func__);
			return false;
		}
	}
	return true;
}
int ncm_protect_recover_Charge_Timeout(ProtectIndex protect_index, ProtectType protect_type)
{
    unsigned long flags;
    int rc;
    nc_pr_debug("Charge Protect OFF\n");
    spin_lock_irqsave(&the_chip->ncm_chg_chip.prtct_timer.slock, flags);
    the_chip->ncm_chg_chip.prtct_timer.cnt        = 0;
    the_chip->ncm_chg_chip.prtct_timer.is_timer   = false;
    the_chip->ncm_chg_chip.prtct_timer.is_timeout = false;
    spin_unlock_irqrestore(&the_chip->ncm_chg_chip.prtct_timer.slock, flags);
    rc = pm_chg_failed_clear(the_chip, 1);
    if (rc)
        pr_err("Failed to write CHG_FAILED_CLEAR bit\n");
    rc = pm_chg_charge_dis(the_chip, 0);
    if(rc) {
        printk(KERN_ERR "[PM] %s:Failure restart of battery charge\n", __func__);
        return false;
    }
    return true;
}
static int ncm_is_connect_dcin(void)
{
	int temp_ov    = 0;
	int temp_uv    = 0;
	int temp_valid = 0;
	int judge      = NCM_CHG_DC_DISCONNECT;
	temp_ov    = pm_chg_get_rt_status(the_chip, DCIN_OV_IRQ);
	temp_uv    = pm_chg_get_rt_status(the_chip, DCIN_UV_IRQ);
	temp_valid = pm_chg_get_rt_status(the_chip, DCIN_VALID_IRQ);
	if (temp_valid == 1){
		judge = NCM_CHG_DC_CONNECT;
	}
	else {
		if (temp_ov == 1){
			if (temp_uv == 1){
				judge = NCM_CHG_DC_DISCONNECT;
			}
			else {
				judge = NCM_CHG_DC_CONNECT_OVP;
			}
		}
		else {
			judge = NCM_CHG_DC_DISCONNECT;
		}
	}
	return judge;
}
#if defined(CONFIG_FEATURE_NCMC_POWER) && defined(CONFIG_FEATURE_NCMC_RUBY)
static int ncm_is_connect_usbin(void)
{
	int temp_ov    = 0;
	int temp_uv    = 0;
	int temp_valid = 0;
	int judge      = NCM_CHG_USB_DISCONNECT;
	temp_ov    = pm_chg_get_rt_status(the_chip, USBIN_OV_IRQ);
	temp_uv    = pm_chg_get_rt_status(the_chip, USBIN_UV_IRQ);
	temp_valid = pm_chg_get_rt_status(the_chip, USBIN_VALID_IRQ);
	if (temp_valid == 1){
		judge = NCM_CHG_USB_CONNECT;
	}
	else {
		if (temp_ov == 1){
			if (temp_uv == 1){
				judge = NCM_CHG_USB_DISCONNECT;
			}
			else {
				judge = NCM_CHG_USB_CONNECT_OVP;
			}
		}
		else {
			judge = NCM_CHG_USB_DISCONNECT;
		}
	}
	return judge;
}
#endif
static int ncm_is_connect_usbchg(void)
{
	int judge      = NCM_CHG_USB_DISCONNECT;
#if defined(CONFIG_FEATURE_NCMC_POWER) && !defined(CONFIG_FEATURE_NCMC_RUBY)
	usb_sw_device_state_enum	device_state = USB_SW_DEVICE_DISCONNECTED;
#else
#endif
#if defined(CONFIG_FEATURE_NCMC_POWER) && !defined(CONFIG_FEATURE_NCMC_RUBY)
	usb_sw_get_device_state(&device_state);
	switch (device_state)
	{
		case USB_SW_DEVICE_DISCONNECTED:
			judge = NCM_CHG_USB_DISCONNECT;
			break;
		case USB_SW_SDP_CONNECTED:
		case USB_SW_DCP_CONNECTED:
		case USB_SW_OTHER_DCP_CONNECTED:
		case USB_SW_OTHER_DEVICE_CONNECTED:
			judge = NCM_CHG_USB_CONNECT;
			break;
		default:
			judge = NCM_CHG_USB_DISCONNECT;
			break;
	}
#else
	judge = ncm_is_connect_usbin();
#endif
	return judge;
}
static void ncm_chg_protect_timer_worker(struct work_struct *work)
{
    unsigned long flags;
    spin_lock_irqsave(&the_chip->ncm_chg_chip.prtct_timer.slock, flags);
    the_chip->ncm_chg_chip.prtct_timer.cnt++;
    if(the_chip->ncm_chg_chip.prtct_timer.cnt >= the_chip->ncm_chg_chip.chg_err_cycle){
        pr_info("[PM] IDX_CHARGE_TIMEOUT is on.\n");
        the_chip->ncm_chg_chip.prtct_timer.is_timeout = true;
        set_status(IDX_CHARGE_TIMEOUT, ProtectOn);
    }else{
        pr_info("[PM] prtct_timer_cnt is %d.\n", the_chip->ncm_chg_chip.prtct_timer.cnt);
        schedule_delayed_work(&the_chip->ncm_chg_chip.prtct_timer.worker,
            round_jiffies_relative(msecs_to_jiffies
                (the_chip->safety_time * 60 * 1000)));
    }
    spin_unlock_irqrestore(&the_chip->ncm_chg_chip.prtct_timer.slock, flags);
}
static void ncm_chg_protect_timer_enable(bool enable)
{
    unsigned long flags;
    if(enable) {
        spin_lock_irqsave(&the_chip->ncm_chg_chip.prtct_timer.slock, flags);
        if(the_chip->ncm_chg_chip.prtct_timer.is_timer   == false &&
           the_chip->ncm_chg_chip.prtct_timer.is_timeout == false){
            nc_pr_debug("ncm_chg_protect_timer_worker start!\n");
            the_chip->ncm_chg_chip.prtct_timer.cnt        = 0;
            the_chip->ncm_chg_chip.prtct_timer.is_timer   = true;
            the_chip->ncm_chg_chip.prtct_timer.is_timeout = false;
            schedule_delayed_work(&the_chip->ncm_chg_chip.prtct_timer.worker,
                round_jiffies_relative(msecs_to_jiffies
                    (the_chip->safety_time * 60 * 1000)));
        }
        spin_unlock_irqrestore(&the_chip->ncm_chg_chip.prtct_timer.slock, flags);
    }else{
        spin_lock_irqsave(&the_chip->ncm_chg_chip.prtct_timer.slock, flags);
        if(the_chip->ncm_chg_chip.prtct_timer.is_timer == true){
            nc_pr_debug("ncm_chg_protect_timer_worker canceled.\n");
            the_chip->ncm_chg_chip.prtct_timer.cnt          = 0;
            the_chip->ncm_chg_chip.prtct_timer.is_timeout   = false;
            the_chip->ncm_chg_chip.prtct_timer.is_timer     = false;
            cancel_delayed_work(&the_chip->ncm_chg_chip.prtct_timer.worker);
        }
        spin_unlock_irqrestore(&the_chip->ncm_chg_chip.prtct_timer.slock, flags);
    }
}
static void ncm_chg_protect_timer_dc_chk_work(struct work_struct *work)
{
    unsigned long flags;
    bool end = false;
    int usb  = ncm_is_connect_usbchg();
    int dc   = ncm_is_connect_dcin();
    nc_pr_debug("usb:%d  dc:%d\n", usb, dc);
    spin_lock_irqsave(&the_chip->ncm_chg_chip.prtct_timer.slock, flags);
    if(the_chip->ncm_chg_chip.prtct_timer.is_timeout == true &&
        usb == NCM_CHG_USB_CONNECT) {
        nc_pr_debug("It's timeout, but usb is connected.\n");
        end = true;
    }
    spin_unlock_irqrestore(&the_chip->ncm_chg_chip.prtct_timer.slock, flags);
    if(end){
        nc_pr_debug("%s() is end.\n", __func__);
        return;
    }
    set_status(IDX_CHARGE_TIMEOUT, ProtectNone);
    check_charge_protect_sts();
    ncm_chg_protect_timer_enable(false);
    if( (dc == NCM_CHG_DC_CONNECT || usb == NCM_CHG_USB_CONNECT) &&
        pm_chg_get_rt_status(the_chip, FASTCHG_IRQ)) {
        ncm_chg_protect_timer_enable(true);
    }
}
static void ncm_chg_protect_timer_usb_chk_work(struct work_struct *work)
{
    static int prev_usb = NCM_CHG_USB_DISCONNECT;
    int cur_usb = ncm_is_connect_usbchg();
    int dc  = ncm_is_connect_dcin();
    nc_pr_debug("cur_usb:%d  prev_usb:%d  dc:%d\n", cur_usb, prev_usb, dc);
    if( dc == NCM_CHG_DC_DISCONNECT ){
        if ((cur_usb == NCM_CHG_USB_CONNECT && prev_usb == NCM_CHG_USB_DISCONNECT) ||
            cur_usb == NCM_CHG_USB_DISCONNECT) {
            set_status(IDX_CHARGE_TIMEOUT, ProtectNone);
            check_charge_protect_sts();
            ncm_chg_protect_timer_enable(false);
        }
        if( cur_usb == NCM_CHG_USB_CONNECT &&
            pm_chg_get_rt_status(the_chip, FASTCHG_IRQ)){
            ncm_chg_protect_timer_enable(true);
        }
    }else{
        nc_pr_debug("Charge protection timer is continued.\n");
    }
    prev_usb = cur_usb;
}
