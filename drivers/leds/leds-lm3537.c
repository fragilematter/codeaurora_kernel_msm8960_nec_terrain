/*
 * drivers/leds/leds-lm3537.c
 *
 * - LM3537 led controller
 *
 * Copyright (C) NEC CASIO Mobile Communications, Ltd.
 *
 */

// #define LED_PM_OBS_DISABLE

#ifndef LOCAL_CONFIG_FEATURE_NCMC_LED_LM3537
#if defined(CONFIG_FEATURE_NCMC_D121F) || defined(CONFIG_FEATURE_NCMC_G121S) || defined(CONFIG_FEATURE_NCMC_KAMITSUKIGAME) || defined(CONFIG_FEATURE_NCMC_SYLPH)
#define LOCAL_CONFIG_FEATURE_NCMC_LED_LM3537
#endif /* #if defined(CONFIG_FEATURE_NCMC_D121F) || defined(CONFIG_FEATURE_NCMC_G121S) || defined(CONFIG_FEATURE_NCMC_KAMITSUKIGAME) || defined(CONFIG_FEATURE_NCMC_SYLPH) */
#endif /* #ifndef LOCAL_CONFIG_FEATURE_NCMC_LED_LM3537 */

#include <linux/device.h>
#include <linux/spinlock.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <mach/gpio.h>
#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif

#ifndef LED_PM_OBS_DISABLE
#include <linux/pm_obs_api.h>
#endif /* LED_PM_OBS_DISABLE */

#if defined(CONFIG_FEATURE_NCMC_GEKKO) || defined(CONFIG_FEATURE_NCMC_RAPIDFIRE) || defined(LOCAL_CONFIG_FEATURE_NCMC_LED_LM3537)
#include <linux/kthread.h>
#include <linux/mutex.h>
#include <linux/wakelock.h>
#endif /* #if defined(CONFIG_FEATURE_NCMC_GEKKO) || defined(CONFIG_FEATURE_NCMC_RAPIDFIRE) || defined(LOCAL_CONFIG_FEATURE_NCMC_LED_LM3537) */

#if defined(CONFIG_FEATURE_NCMC_GEKKO) || defined(CONFIG_FEATURE_NCMC_RAPIDFIRE)
#define LOCAL_LED_MDL_ACTIVE
#endif /* #if defined(CONFIG_FEATURE_NCMC_GEKKO)  || defined(CONFIG_FEATURE_NCMC_RAPIDFIRE) */

#ifdef LOCAL_LED_MDL_ACTIVE
#if defined(CONFIG_FEATURE_NCMC_D121F) || defined(CONFIG_FEATURE_NCMC_G121S)
#include <mach/msm_smsm.h>
#include <linux/oemnc_smem.h>
#else /* #if defined(CONFIG_FEATURE_NCMC_D121F) || defined(CONFIG_FEATURE_NCMC_G121S) */
#include <linux/oemnc_smem.h>
#include "../../../arch/arm/mach-fsm/smd_private.h"
#endif /* #if defined(CONFIG_FEATURE_NCMC_D121F) || defined(CONFIG_FEATURE_NCMC_G121S) */
#endif /* #ifdef LOCAL_LED_MDL_ACTIVE */

#include <linux/leds-lcd-common.h>
//#include <linux/leds-lm3537.h>

/*============================================================================
		DEFINE & ENUM
============================================================================*/


#ifdef LED_DIAG_ONLY
#define MAX_BACKLIGHT_BRIGHTNESS 255
#define MAX_BACKLIGHT_HARD_REGSTER		(123)
#define MAX_BACKLIGHT_HARD_REGSTER_BASE	(81)
#endif /* #ifdef LED_DIAG_ONLY */

#define	LED_REG_MASTER_EN			0x00
#define	LED_REG_DIODE_EN			0x10
#define	LED_REG_CONFIG				0x20
#define	LED_REG_OPTIONS				0x30
#define	LED_REG_ALS_ZONE_RB			0x40
#define	LED_REG_ALS_CONTROL			0x50
#define	LED_REG_ALS_RESISTOR		0x51
#define	LED_REG_ALS_CONFIG			0x52
#define	LED_REG_ALS_ZONE_B0			0x60
#define	LED_REG_ALS_ZONE_B1			0x61
#define	LED_REG_ALS_ZONE_B2			0x62
#define	LED_REG_ALS_ZONE_B3			0x63
#define	LED_REG_ALS_LOW_HIGH		0x64
#define	LED_REG_ALS_ZONE_Z3_Z2		0x65
#define	LED_REG_ALS_BRI_Z0			0x70
#define	LED_REG_ALS_BRI_Z1			0x71
#define	LED_REG_ALS_BRI_Z2			0x72
#define	LED_REG_ALS_BRI_Z3			0x73
#define	LED_REG_ALS_BRI_Z4			0x74
#define	LED_REG_GROUP_A_BRI			0xA0
#define	LED_REG_GROUP_B_BRI			0xB0
#define	LED_REG_LDO_EN				0xC0
#define	LED_REG_LDO1_VOUT			0xC1
#define	LED_REG_LDO2_VOUT			0xC2
#define	LED_REG_LDO3_VOUT			0xC3
#define	LED_REG_LDO4_VOUT			0xC4

#define	LED_REG_GROUP_A_BRI_MAX			(123)
#define	LED_REG_GROUP_A_BRI_DIAG_MAX	(100)

#define	LED_REG_DIODE_EN_ON				0x3F
#define	LED_REG_DIODE_EN_OFF			0x00

#define	LED_REG_MASTER_EN_ON			0x04
#define	LED_REG_MASTER_EN_OFF			0x00

#define	LED_REG_CONFIG_INI				0x3C

#if defined(LOCAL_CONFIG_FEATURE_NCMC_LED_LM3537)
#define	LED_REG_OPTIONS_INI				0x00
#define	LED_REG_OPTIONS_ACT				0x36
#else /* #if defined(LOCAL_CONFIG_FEATURE_NCMC_LED_LM3537) */
#ifdef CONFIG_FEATURE_NCMC_RAPIDFIRE
#define	LED_REG_OPTIONS_INI				0x00
#define	LED_REG_OPTIONS_ACT				0x36
#else /* #ifdef CONFIG_FEATURE_NCMC_RAPIDFIRE */
#ifdef CONFIG_FEATURE_NCMC_GEKKO
#define	LED_REG_OPTIONS_INI				0x00
#define	LED_REG_OPTIONS_ACT				0x36
#else /* #ifdef CONFIG_FEATURE_NCMC_GEKKO */
#define	LED_REG_OPTIONS_INI				0x00
#endif /* #ifdef CONFIG_FEATURE_NCMC_GEKKO */
#endif /* #ifdef CONFIG_FEATURE_NCMC_RAPIDFIRE */
#endif /* #if defined(LOCAL_CONFIG_FEATURE_NCMC_LED_LM3537) */

#define	LED_REG_CONFIG_PWM_BIT_MASK		0xFE
#define	LED_REG_CONFIG_PWM_ON			((LED_REG_CONFIG_INI & LED_REG_CONFIG_PWM_BIT_MASK) | 0x01 )
#define	LED_REG_CONFIG_PWM_OFF			((LED_REG_CONFIG_INI & LED_REG_CONFIG_PWM_BIT_MASK) | 0x00 )

#define LED_REG_GPIO_CTRL_ON

#ifdef LED_REG_GPIO_CTRL_ON
#if defined(LOCAL_CONFIG_FEATURE_NCMC_LED_LM3537)
#define LED_REG_GPIO_HWEN_GPIO
#endif /* #if defined(LOCAL_CONFIG_FEATURE_NCMC_LED_LM3537) */
#ifdef LED_REG_GPIO_HWEN_GPIO
#define LED_REG_GPIO_HWEN					78
#else /* #ifdef LED_REG_GPIO_HWEN_GPIO */
#define LED_REG_GPIO_HWEN					15
#endif /* #ifdef LED_REG_GPIO_HWEN_GPIO */
#define LED_REG_GPIO_WAIT					1
#endif /* #ifdef LED_REG_GPIO_CTRL_ON */

/*============================================================================
		STRUCTURE
============================================================================*/

struct lm3537_reg_init{
	unsigned char adr	; /* address */
	unsigned char wdata	; /* write data */
};

struct lm3537_reg_tbl{
	unsigned char lcd_set;
	unsigned char lcd_bright;
#if defined(CONFIG_FEATURE_NCMC_GEKKO) || defined(CONFIG_FEATURE_NCMC_RAPIDFIRE) || defined(LOCAL_CONFIG_FEATURE_NCMC_LED_LM3537)
	struct mutex  func_lock;
	struct task_struct *ps_thread;
#endif /* #if defined(CONFIG_FEATURE_NCMC_GEKKO) || defined(CONFIG_FEATURE_NCMC_RAPIDFIRE) || defined(LOCAL_CONFIG_FEATURE_NCMC_LED_LM3537) */
};

static struct i2c_client	 *lm3537_client;		/* transfer */
static struct lm3537_reg_tbl lm3537_reg ={
	.lcd_set    = LM3537_LED_OFF,
	.lcd_bright = LM3537_LED_OFF,
};
struct led_lcd_ctrl_don {
	unsigned char lcd_main_backlight_exec;
	unsigned char lcd_main_backlight_status;
};

#ifdef LOCAL_LED_MDL_ACTIVE  
struct lm3537_data_tbl{
	struct led_lcd_ctrl_don		lcd_fon;
};
#endif /* #ifdef LOCAL_LED_MDL_ACTIVE */

#if defined(CONFIG_FEATURE_NCMC_GEKKO) || defined(CONFIG_FEATURE_NCMC_RAPIDFIRE) || defined(LOCAL_CONFIG_FEATURE_NCMC_LED_LM3537)
	/* GKK-CR1-00379 */ /* AD11-2nd_S-CR1-00104 */
static wait_queue_head_t gs_led_wq;
#endif /* #if defined(CONFIG_FEATURE_NCMC_GEKKO) || defined(CONFIG_FEATURE_NCMC_RAPIDFIRE) || defined(LOCAL_CONFIG_FEATURE_NCMC_LED_LM3537) */

/*============================================================================
		LOCAL DATA
============================================================================*/

static struct i2c_client	 *lm3537_client;		/* transfer */

struct led_lcd_ctrl_don	lcd_don = {
	.lcd_main_backlight_exec   = LM3537_LED_OFF,
	.lcd_main_backlight_status = LM3537_LED_OFF,
};

#ifdef LOCAL_LED_MDL_ACTIVE  
/* bd6082gcl reg init table */
static struct lm3537_data_tbl lm3537_data = {
	.lcd_fon = {
		.lcd_main_backlight_exec   = LM3537_LED_OFF,
		.lcd_main_backlight_status = LM3537_LED_OFF,
	},
};
#if defined(LOCAL_CONFIG_FEATURE_NCMC_LED_LM3537) 
static smem_id_vendor0 *p_smem_id_vendor0=NULL;
static unsigned int power_on_status = 0;
#endif /* #if defined(LOCAL_CONFIG_FEATURE_NCMC_LED_LM3537) */
#endif /* #ifdef LOCAL_LED_MDL_ACTIVE */

static const unsigned short lm3537_led_current_value[124] = {
	/* 2 = 0.02mA, 1886 = 18.86mA */
	2,	2,	2,	2,	2,	2,	2,	3,	3,	3,	3,	3,	4,	4,	4,	4,	5,	5,	5,	5,
	6,	6,	7,	7,	7,	8,	8,	9,	9,	10,	11,	11,	12,	13,	13,	14,	15,	16,	17,	18,
	19,	20,	21,	22,	24,	25,	27,	28,	30,	31,	33,	35,	37,	40,	42,	44,	47,	50,	52,	55,
	59,	62,	66,	69,	73,	78,	82,	87,	92,	97,	103,109,115,122,129,136,144,152,161,170,
	180,191,202,213,225,238,252,267,282,298,315,334,353,373,394,417,441,467,493,522,
	552,583,617,652,690,730,772,816,863,912,965,1020,1079,1141,1206,1276,1349,1426,1508,1595,
	1687,1784,1886,1995
};

void lm3537_led_suspend(void);
void lm3537_led_resume(void);

/*============================================================================
		LOCAL FUNCTIONS
============================================================================*/

/*------------------------------------------------------------------------------
-------------------------------------------------------------------------------*/
int lm3537_i2c_smbus_write(u8 command, u8 value)
{
	s32	ret ;
/*--- Alarm Watch < START > --------------------------------------------------*/
    unsigned char alrm_info[4];
    static int alrm_cnt = 0;
/*--- Alarm Watch < END > ----------------------------------------------------*/
	
	/* i2c_smbus_write */
	ret = i2c_smbus_write_byte_data(lm3537_client, command, value);
	
	if(ret < 0){
		printk(KERN_ERR
			   "lm3537_LED: i2c_smbus_write_byte_data Error %x\n", ret );
/*--- Alarm Watch < START > --------------------------------------------------*/
        if( alrm_cnt == 0 )
        {
            alrm_info[0] = 0x09;        /* 1,2 byte(10b) + Write(01b) */
            alrm_info[1] = (unsigned char)(ret * -1);
            alrm_info[2] = command;     /* Address */
            alrm_info[3] = value;       /* Data    */

            printk(KERN_ERR "[T][ARM]Event:0x69 Info:0x%02X%02X%02X%02X\n",
                            alrm_info[0],alrm_info[1],alrm_info[2],alrm_info[3]);
            alrm_cnt = 1;
        }

/*--- Alarm Watch < END > ----------------------------------------------------*/
	}
/*--- Alarm Watch < START > --------------------------------------------------*/
    else
    {
        alrm_cnt = 0;
    }
/*--- Alarm Watch < END > ----------------------------------------------------*/
	return ret;
}
EXPORT_SYMBOL(lm3537_i2c_smbus_write);

/*------------------------------------------------------------------------------
-------------------------------------------------------------------------------*/
int lm3537_i2c_smbus_read(u8 command, u8 *value)
{
	s32	ret ;
/*--- Alarm Watch < START > --------------------------------------------------*/
    unsigned char alrm_info[4];
    static int alrm_cnt = 0;
/*--- Alarm Watch < END > ----------------------------------------------------*/
	
	/* i2c_smbus_write */
	ret = i2c_smbus_read_byte_data(lm3537_client, command);
	
	if(ret < 0){
		printk(KERN_ERR
			   "lm3537_LED: i2c_smbus_read_byte_data Error %x\n", ret );
/*--- Alarm Watch < START > --------------------------------------------------*/
        if( alrm_cnt == 0 )
        {
            alrm_info[0] = 0x06;        /* 1 byte(01b) + Write(10b) */
            alrm_info[1] = (unsigned char)(ret * -1);
            alrm_info[2] = *value;      /* Data    */
            alrm_info[3] = 0;           /* -       */

            printk(KERN_ERR "[T][ARM]Event:0x69 Info:0x%02X%02X%02X%02X\n",
                            alrm_info[0],alrm_info[1],alrm_info[2],alrm_info[3]);
            alrm_cnt = 1;
        }

/*--- Alarm Watch < END > ----------------------------------------------------*/
	}
	else{
		*value = (u8)( ret & 0xFF );
/*--- Alarm Watch < START > --------------------------------------------------*/
        alrm_cnt = 0;
/*--- Alarm Watch < END > ----------------------------------------------------*/
	}
	return ret;
}
EXPORT_SYMBOL(lm3537_i2c_smbus_read);

/*------------------------------------------------------------------------------
-------------------------------------------------------------------------------*/
static void lm3537_pm_obs_a_lcdbacklight( void )
{
#ifndef LED_PM_OBS_DISABLE
	unsigned int current_value, max_current_value, lcd_bright ;
	
	if( lm3537_reg.lcd_set != LM3537_LED_OFF ){
		
		current_value     = lm3537_led_current_value[lm3537_reg.lcd_bright];
		max_current_value = lm3537_led_current_value[(sizeof(lm3537_led_current_value)/sizeof(unsigned short))-1];
		
		lcd_bright = (( current_value * 100 ) / max_current_value ) ;
		
		pm_obs_a_lcdbacklight( lcd_bright );
		printk(KERN_INFO
		   "lm3537: pm_obs_a_lcdbacklight( %d, %d mA, %d )\n", lm3537_reg.lcd_bright, current_value/100, lcd_bright );
	}
	else{
		pm_obs_a_lcdbacklight( LM3537_LED_OFF );
		printk(KERN_INFO
		   "lm3537: pm_obs_a_lcdbacklight( %d )\n", LM3537_LED_OFF );
	}
#endif /* LED_PM_OBS_DISABLE */
	return;
}

/*------------------------------------------------------------------------------
-------------------------------------------------------------------------------*/
int lm3537_main_lcd_don(unsigned char sub2)
{
	spinlock_t	spin_lock_status = SPIN_LOCK_UNLOCKED;
	int	ret;
	
	spin_lock(&spin_lock_status);
	lcd_don.lcd_main_backlight_exec   = LM3537_LED_ON;
	lcd_don.lcd_main_backlight_status = sub2;
	spin_unlock(&spin_lock_status);
	
	ret = led_main_lcd_bright(LED_REG_GROUP_A_BRI_DIAG_MAX);
	if( ret != LM3537_LED_SET_OK ){
		return ret ;
	}
	
	ret = led_main_lcd_set(sub2, LM3537_LED_OFF);
	
	return ret ;
}
EXPORT_SYMBOL(lm3537_main_lcd_don);

/*------------------------------------------------------------------------------
-------------------------------------------------------------------------------*/
#if defined(CONFIG_FEATURE_NCMC_GEKKO) || defined(CONFIG_FEATURE_NCMC_RAPIDFIRE) || defined(LOCAL_CONFIG_FEATURE_NCMC_LED_LM3537)
	/* GKK-CR1-00379 */ /* AD11-2nd_S-CR1-00104 */
static void lm3537_main_lcd_thread( void )
{
	int ret, wait_ret, time ;
	
	time = 300; /* 300ms */
	
	printk(KERN_INFO 
			"LM3537_LED: lm3537_main_lcd_thread Wait Start \n" );
	
	wait_ret = wait_event_interruptible_timeout( gs_led_wq, ( lm3537_reg.lcd_set == 0 ), msecs_to_jiffies(time) );
	
	printk(KERN_INFO 
			"LM3537_LED: lm3537_main_lcd_thread Wait Timeout %d \n" ,wait_ret );
	
	mutex_lock( &lm3537_reg.func_lock );
	if ( lm3537_reg.lcd_set != 0 ){
		ret = lm3537_i2c_smbus_write( LED_REG_OPTIONS, LED_REG_OPTIONS_ACT);
		printk(KERN_INFO 
				"LM3537_LED: lm3537_main_lcd_thread %d write  \n", LED_REG_OPTIONS_ACT );
		if(ret < 0){
			printk(KERN_ERR "lm3537_LED: i2c Write Reg %x(%x) Error %x \n",
							LED_REG_OPTIONS,
							LED_REG_OPTIONS_ACT,
							ret );
		}
	}
	mutex_unlock( &lm3537_reg.func_lock );
	
	printk(KERN_INFO 
			"LM3537_LED: lm3537_main_lcd_thread END \n" );
}
#endif /* #if defined(CONFIG_FEATURE_NCMC_GEKKO) || defined(CONFIG_FEATURE_NCMC_RAPIDFIRE) || defined(LOCAL_CONFIG_FEATURE_NCMC_LED_LM3537) */

/*------------------------------------------------------------------------------
-------------------------------------------------------------------------------*/
int led_main_lcd_set(unsigned char request, unsigned char pwm)
{
	int	ret, ret_fnc = 0, set_00, set_10, set_20;
	
#if defined(CONFIG_FEATURE_NCMC_GEKKO) || defined(CONFIG_FEATURE_NCMC_RAPIDFIRE) || defined(LOCAL_CONFIG_FEATURE_NCMC_LED_LM3537)
	/* GKK-CR1-00379 */ /* AD11-2nd_S-CR1-00104 */
	mutex_lock( &lm3537_reg.func_lock );
#endif /* #if defined(CONFIG_FEATURE_NCMC_GEKKO) || defined(CONFIG_FEATURE_NCMC_RAPIDFIRE) || defined(LOCAL_CONFIG_FEATURE_NCMC_LED_LM3537) */
	
	if( lcd_don.lcd_main_backlight_exec == LM3537_LED_ON ){
		request = lcd_don.lcd_main_backlight_status;
		pwm = LM3537_LED_ON;
	}
	if( request != 0 ){
		set_00 = LED_REG_MASTER_EN_ON;
		set_10 = LED_REG_DIODE_EN_ON;
	}
	else{
		set_00 = LED_REG_MASTER_EN_OFF;
		set_10 = LED_REG_DIODE_EN_OFF;
	}
	if( pwm != 0 ){
		set_20 = LED_REG_CONFIG_PWM_ON;
	}
	else{
		set_20 = LED_REG_CONFIG_PWM_OFF;
	}
	
#if defined(CONFIG_FEATURE_NCMC_GEKKO) || defined(CONFIG_FEATURE_NCMC_RAPIDFIRE) || defined(LOCAL_CONFIG_FEATURE_NCMC_LED_LM3537)
	/* GKK-CR1-00379 */ /* AD11-2nd_S-CR1-00104 */
	if( request == 0 ){
		ret = lm3537_i2c_smbus_write( LED_REG_OPTIONS, LED_REG_OPTIONS_INI);
		printk(KERN_INFO 
				"LM3537_LED: lm3537_main_lcd_thread %d write  \n", LED_REG_OPTIONS_INI );
		if(ret < 0){
			printk(KERN_ERR "lm3537_LED: i2c Write Reg %x(%x) Error %x \n",
							LED_REG_OPTIONS,
							LED_REG_OPTIONS_INI,
							ret );
			ret_fnc = LM3537_LED_SET_NG;
		}
		lm3537_reg.ps_thread = NULL;
	}
#endif /* #if defined(CONFIG_FEATURE_NCMC_GEKKO) || defined(CONFIG_FEATURE_NCMC_RAPIDFIRE) || defined(LOCAL_CONFIG_FEATURE_NCMC_LED_LM3537) */
	
	ret = lm3537_i2c_smbus_write(LED_REG_DIODE_EN,  set_10 );
	if( ret != 0 ){
		printk(KERN_ERR
			   "LM3537_LED: i2c Write Reg %x Error %x \n", LED_REG_DIODE_EN, ret );
		ret_fnc = LM3537_LED_SET_NG;
	}
	ret = lm3537_i2c_smbus_write(LED_REG_MASTER_EN, set_00 );
	if( ret != 0 ){
		printk(KERN_ERR
			   "LM3537_LED: i2c Write Reg %x Error %x \n", LED_REG_MASTER_EN, ret );
		ret_fnc = LM3537_LED_SET_NG;
	}
	if( ret == 0 ){
		lm3537_reg.lcd_set = set_10;
	}
	ret = lm3537_i2c_smbus_write(LED_REG_CONFIG, set_20 );
	if( ret != 0 ){
		printk(KERN_ERR
			   "LM3537_LED: i2c Write Reg %x Error %x \n", LED_REG_CONFIG, ret );
		ret_fnc = LM3537_LED_SET_NG;
	}
#if defined(CONFIG_FEATURE_NCMC_GEKKO) || defined(CONFIG_FEATURE_NCMC_RAPIDFIRE) || defined(LOCAL_CONFIG_FEATURE_NCMC_LED_LM3537)
	if(( request != 0 ) && (lm3537_reg.lcd_bright != 0 )){
		if( lm3537_reg.ps_thread == NULL ){
			lm3537_reg.ps_thread = kthread_run((void*)*lm3537_main_lcd_thread, NULL, "lm3537_main_lcd_thread" );
			if( IS_ERR(lm3537_reg.ps_thread) ){
				printk(KERN_INFO "LM3537_LED: kthread_run NG\n" );
				lm3537_reg.ps_thread = NULL;
				ret_fnc = LM3537_LED_SET_NG;
			}
		}
	}
	else{
		wake_up_interruptible(&gs_led_wq);
	}
#endif /* #if defined(CONFIG_FEATURE_NCMC_GEKKO) || defined(CONFIG_FEATURE_NCMC_RAPIDFIRE) || defined(LOCAL_CONFIG_FEATURE_NCMC_LED_LM3537) */
	lm3537_pm_obs_a_lcdbacklight();
#if defined(CONFIG_FEATURE_NCMC_GEKKO) || defined(CONFIG_FEATURE_NCMC_RAPIDFIRE) || defined(LOCAL_CONFIG_FEATURE_NCMC_LED_LM3537)
	mutex_unlock( &lm3537_reg.func_lock );
#endif /* #if defined(CONFIG_FEATURE_NCMC_GEKKO) || defined(CONFIG_FEATURE_NCMC_RAPIDFIRE) || defined(LOCAL_CONFIG_FEATURE_NCMC_LED_LM3537) */
	
	return ret_fnc;
}
EXPORT_SYMBOL(led_main_lcd_set);

/*------------------------------------------------------------------------------
-------------------------------------------------------------------------------*/
int led_main_lcd_bright (unsigned char lcd_bright)
{
	int	ret = 0;
#ifdef LOCAL_LED_MDL_ACTIVE
#if defined(LOCAL_CONFIG_FEATURE_NCMC_LED_LM3537) 
#else /* #if defined(LOCAL_CONFIG_FEATURE_NCMC_LED_LM3537) */
	smem_id_vendor0 *p_smem_id_vendor0=NULL;
	unsigned int power_on_status = 0;
#endif /* #if defined(LOCAL_CONFIG_FEATURE_NCMC_LED_LM3537) */
#endif /* #ifdef LOCAL_LED_MDL_ACTIVE */
	
#ifdef LED_DIAG_ONLY
	
	unsigned int	color ;
	
	color = lcd_bright;
	
	if (color > MAX_BACKLIGHT_BRIGHTNESS){
		color = MAX_BACKLIGHT_BRIGHTNESS;
	}
	
	// min color 0x20 ->  86(0x56)
	// max color 0xFF -> 120(0x78)
	
	lcd_bright = (2 * color * ((MAX_BACKLIGHT_HARD_REGSTER -1) - MAX_BACKLIGHT_HARD_REGSTER_BASE) + MAX_BACKLIGHT_BRIGHTNESS)
		/(2 * MAX_BACKLIGHT_BRIGHTNESS);
	lcd_bright = lcd_bright + MAX_BACKLIGHT_HARD_REGSTER_BASE;
	
#endif /* #ifdef LED_DIAG_ONLY */
	
	if( lcd_bright > LED_REG_GROUP_A_BRI_MAX ){
		lcd_bright = LED_REG_GROUP_A_BRI_MAX;
	}
	if( lcd_don.lcd_main_backlight_exec == LM3537_LED_ON ){
#if defined(LOCAL_CONFIG_FEATURE_NCMC_LED_LM3537)
		lcd_bright = 0x56;
#else /* #if defined(LOCAL_CONFIG_FEATURE_NCMC_LED_LM3537) */
		lcd_bright = LED_REG_GROUP_A_BRI_DIAG_MAX;
#endif /* #if defined(LOCAL_CONFIG_FEATURE_NCMC_LED_LM3537) */
	}

#ifdef LOCAL_LED_MDL_ACTIVE
	if( lm3537_data.lcd_fon.lcd_main_backlight_exec == LM3537_LED_OFF ){
		
		p_smem_id_vendor0 = (smem_id_vendor0 *)(smem_find(SMEM_ID_VENDOR0, sizeof(smem_id_vendor0)));
		if( p_smem_id_vendor0!=NULL ){
			power_on_status = p_smem_id_vendor0->flg_info.flash_factory_flg;
2			printk("LM3537_LED: Success smem_find(SMEM_ID_VENDOR0), factory_mode_flag_apps:%d\n", power_on_status);
			
			if( power_on_status != 0 ){
					printk(KERN_INFO
			 		  "LM3537_LED: smem_find( %d ) Fact mode \n", power_on_status );
				lm3537_data.lcd_fon.lcd_main_backlight_exec   = LM3537_LED_ON;
				lm3537_data.lcd_fon.lcd_main_backlight_status = LM3537_LED_ON;
			}
			else{
	
				printk(KERN_INFO
			 		  "LM3537_LED: smem_find( %d ) Nomal mode \n", power_on_status );
				lm3537_data.lcd_fon.lcd_main_backlight_exec   = LM3537_LED_ON;
				lm3537_data.lcd_fon.lcd_main_backlight_status = LM3537_LED_OFF;
			}
		}
		else{
		    pr_err("LM3537_LED: FAIL: smem_find(SMEM_ID_VENDOR0)\n");
		}
	}
	if( lm3537_data.lcd_fon.lcd_main_backlight_status != LM3537_LED_OFF ){
		printk(KERN_INFO "led_main_lcd_bright: MDL_NO_BATTERY_WAKEUP \n" );
		lcd_bright = 86;
#if defined(LOCAL_CONFIG_FEATURE_NCMC_LED_LM3537) 
		if( p_smem_id_vendor0!=NULL ){
			if( power_on_status != 0 ){
				lcd_bright = 0x30;
			}
		}
#endif /* #if defined(LOCAL_CONFIG_FEATURE_NCMC_LED_LM3537) */
	}
#endif /* #ifdef LOCAL_LED_MDL_ACTIVE */

	ret = lm3537_i2c_smbus_write(LED_REG_GROUP_A_BRI, lcd_bright );
	if( ret != 0 ){
		printk(KERN_ERR
			   "LM3537_LED: i2c Write Reg %x Error %x \n", LED_REG_GROUP_A_BRI, ret );
		return LM3537_LED_SET_NG;
	}
	lm3537_reg.lcd_bright = lcd_bright;
	lm3537_pm_obs_a_lcdbacklight();
	
	return ret;
}
EXPORT_SYMBOL(led_main_lcd_bright);
/*------------------------------------------------------------------------------
-------------------------------------------------------------------------------*/
#if defined(CONFIG_FEATURE_NCMC_D121F) || defined(CONFIG_FEATURE_NCMC_G121S) || defined(CONFIG_FEATURE_NCMC_KAMITSUKIGAME) || defined(CONFIG_FEATURE_NCMC_SYLPH)
int led_main_lcd_reg_init ()
#else /* #if defined(CONFIG_FEATURE_NCMC_D121F) || defined(CONFIG_FEATURE_NCMC_G121S) || defined(CONFIG_FEATURE_NCMC_KAMITSUKIGAME) || defined(CONFIG_FEATURE_NCMC_SYLPH) */
static int lm3537_init_client(struct i2c_client *client)
#endif /* #if defined(CONFIG_FEATURE_NCMC_D121F) || defined(CONFIG_FEATURE_NCMC_G121S) || defined(CONFIG_FEATURE_NCMC_KAMITSUKIGAME) || defined(CONFIG_FEATURE_NCMC_SYLPH) */
{
	int			ret, ret_code = LM3537_LED_SET_OK ;
	int			cnt ;
	struct lm3537_reg_init reg_data[] =
	{
		/*	address					,	data	*/
		{	LED_REG_MASTER_EN		,	LED_REG_MASTER_EN_OFF	},
		{	LED_REG_DIODE_EN		,	0x00	},
		{	LED_REG_CONFIG			,	LED_REG_CONFIG_INI	},
		{	LED_REG_OPTIONS			,	LED_REG_OPTIONS_INI	},
		{	LED_REG_ALS_ZONE_RB		,	0x00	},
		{	LED_REG_ALS_CONTROL		,	0x00	},
		{	LED_REG_ALS_RESISTOR	,	0x02	},
		{	LED_REG_ALS_CONFIG		,	0x00	},
		{	LED_REG_ALS_ZONE_B0		,	0x33	},
		{	LED_REG_ALS_ZONE_B1		,	0x66	},
		{	LED_REG_ALS_ZONE_B2		,	0x99	},
		{	LED_REG_ALS_ZONE_B3		,	0xCC	},
		{	LED_REG_ALS_LOW_HIGH	,	0x0B	},
		{	LED_REG_ALS_ZONE_Z3_Z2	,	0x0F	},
		{	LED_REG_ALS_BRI_Z0		,	0x3C	},
		{	LED_REG_ALS_BRI_Z1		,	0x4D	},
		{	LED_REG_ALS_BRI_Z2		,	0x59	},
		{	LED_REG_ALS_BRI_Z3		,	0x66	},
		{	LED_REG_ALS_BRI_Z4		,	0x72	},
		{	LED_REG_GROUP_A_BRI		,	0x00	},
		{	LED_REG_GROUP_B_BRI		,	0x00	},
		{	LED_REG_LDO_EN			,	0x00	},
		{	LED_REG_LDO1_VOUT		,	0x18	},
		{	LED_REG_LDO2_VOUT		,	0x0C	},
		{	LED_REG_LDO3_VOUT		,	0x0C	},
		{	LED_REG_LDO4_VOUT		,	0x18	}
	};

	/* reg init */
	for( cnt = 0; cnt < ARRAY_SIZE(reg_data); cnt++ )
	{
		ret = lm3537_i2c_smbus_write(reg_data[cnt].adr, reg_data[cnt].wdata);
		if(ret < 0){
			printk(KERN_ERR "lm3537_LED: i2c Write Reg %x(%x) Error %x \n",
							reg_data[cnt].adr,
							reg_data[cnt].wdata,
							ret );
			ret_code = LM3537_LED_SET_NG;
		}
		else
		{
			printk(KERN_DEBUG "lm3537_LED: i2c Write Reg %x(%x) OK %x \n",
							reg_data[cnt].adr,
							reg_data[cnt].wdata,
							ret );
		}
	}
	lm3537_reg.lcd_bright = LM3537_LED_OFF;
	lm3537_pm_obs_a_lcdbacklight();
	
#if defined(CONFIG_FEATURE_NCMC_D121F) || defined(CONFIG_FEATURE_NCMC_G121S) || defined(CONFIG_FEATURE_NCMC_KAMITSUKIGAME) || defined(CONFIG_FEATURE_NCMC_SYLPH)
	/* ret */
	return ret_code;
}

static int lm3537_init_client(struct i2c_client *client)
{
	int			ret_code = LM3537_LED_SET_OK ;
#endif /* #if defined(CONFIG_FEATURE_NCMC_D121F) || defined(CONFIG_FEATURE_NCMC_G121S) || defined(CONFIG_FEATURE_NCMC_KAMITSUKIGAME) || defined(CONFIG_FEATURE_NCMC_SYLPH) */
#if defined(CONFIG_FEATURE_NCMC_GEKKO) || defined(CONFIG_FEATURE_NCMC_RAPIDFIRE) || defined(LOCAL_CONFIG_FEATURE_NCMC_LED_LM3537)
	mutex_init( &lm3537_reg.func_lock );
	lm3537_reg.ps_thread = NULL;
	init_waitqueue_head(&gs_led_wq);
#endif /* #if defined(CONFIG_FEATURE_NCMC_GEKKO) || defined(CONFIG_FEATURE_NCMC_RAPIDFIRE) || defined(LOCAL_CONFIG_FEATURE_NCMC_LED_LM3537) */
	
	/* ret */
	return ret_code;
}

void lm3537_led_suspend(void)
{
	printk(KERN_DEBUG "lm3537_LED:%s: Enter\n", __func__);
	gpio_set_value(LED_REG_GPIO_HWEN, 0);
	return;
}
EXPORT_SYMBOL(lm3537_led_suspend);

void lm3537_led_resume(void)
{
	printk(KERN_DEBUG "lm3537_LED:%s: Enter\n", __func__);
	gpio_set_value(LED_REG_GPIO_HWEN, 1);
	return;
}
EXPORT_SYMBOL(lm3537_led_resume);

static int lm3537_i2c_probe(struct i2c_client *client,
							   const struct i2c_device_id *id)
{
	int rc = 0;
#ifdef LED_REG_GPIO_CTRL_ON
#ifdef LED_REG_GPIO_HWEN_GPIO
#else /* #ifdef LED_REG_GPIO_HWEN_GPIO */
	struct leds_lm3537_platform_data	*p_data = client->dev.platform_data;
#endif /* #ifdef LED_REG_GPIO_HWEN_GPIO */
#endif /* #ifdef LED_REG_GPIO_CTRL_ON */
	printk(KERN_DEBUG "lm3537_LED:%s: Enter\n", __func__);

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		rc = -ENOTSUPP;
		goto probe_failure;
	}
	

#ifdef LED_REG_GPIO_CTRL_ON
#ifdef LED_REG_GPIO_HWEN_GPIO
	gpio_set_value(LED_REG_GPIO_HWEN, 1);

	printk(KERN_INFO
		   "lm3537_LED: GPIO %d ON\n", LED_REG_GPIO_HWEN );

#else /* #ifdef LED_REG_GPIO_HWEN_GPIO */
	if ( p_data ){
		if ( p_data->poweron ){
			rc = p_data->poweron( &client->dev );
			printk(KERN_DEBUG "lm3537_LED:%s: poweron (%d)\n", __func__,rc);
			if ( rc )
			{
				printk(KERN_ERR
				   "lm3537_LED: poweron Error %x \n", rc );
				goto probe_failure;
			}
		}
	}
#endif /* #ifdef LED_REG_GPIO_HWEN_GPIO */
	mdelay(LED_REG_GPIO_WAIT);
#endif /* #ifdef LED_REG_GPIO_CTRL_ON */
	
	lm3537_client = client;
	lm3537_init_client(client);

	printk(KERN_INFO
		   "lm3537_LED: lm3537_probe succeeded!\n" );
	return 0;
	
probe_failure:
	printk(KERN_WARNING
		   "lm3537_LED: lm3537_probe failed!\n" );
	return rc;
}

/* led dev. ic exit */
static int lm3537_i2c_remove(struct i2c_client *client)
{
#ifdef LED_REG_GPIO_CTRL_ON
#ifdef LED_REG_GPIO_HWEN_GPIO
	gpio_set_value(LED_REG_GPIO_HWEN, 0);

	printk(KERN_INFO
		   "lm3537_LED: GPIO %d Off\n", LED_REG_GPIO_HWEN );

#endif /* #ifdef LED_REG_GPIO_HWEN_GPIO */
#endif /* #ifdef LED_REG_GPIO_CTRL_ON */
	return 0;
}

static const struct i2c_device_id lm3537_i2c_id[] = {
	{ "led_lm3537", 0},
};

#ifndef WIN32
static struct i2c_driver msm_led_lm3537_driver = {
	.id_table = lm3537_i2c_id,
	.probe	  = lm3537_i2c_probe,
	.remove	  = lm3537_i2c_remove,
	
	.driver = {
			.name = "led_lm3537",
			.owner = THIS_MODULE,
	},
};
#else
static struct i2c_driver msm_led_lm3537_driver;
#endif

static int __init lm3537_led_init(void)
{
	printk(KERN_DEBUG "lm3537_LED:%s: Enter\n", __func__);
	return i2c_add_driver(&msm_led_lm3537_driver);
}

static void __exit lm3537_led_exit(void)
{
	printk(KERN_DEBUG "lm3537_LED:%s: Enter\n", __func__);
	i2c_del_driver(&msm_led_lm3537_driver); 
	
#if defined(CONFIG_FEATURE_NCMC_GEKKO) || defined(CONFIG_FEATURE_NCMC_RAPIDFIRE) || defined(LOCAL_CONFIG_FEATURE_NCMC_LED_LM3537)
	if( lm3537_reg.ps_thread != NULL ){
		kthread_stop( lm3537_reg.ps_thread );
		lm3537_reg.ps_thread = NULL;
	}
#endif /* #if defined(CONFIG_FEATURE_NCMC_GEKKO) || defined(CONFIG_FEATURE_NCMC_RAPIDFIRE) || defined(LOCAL_CONFIG_FEATURE_NCMC_LED_LM3537) */
}

module_init(lm3537_led_init);
module_exit(lm3537_led_exit);

/* File END */
