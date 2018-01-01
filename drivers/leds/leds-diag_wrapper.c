/*
 * drivers/leds/leds-diag_wrapper.c
 *
 * Copyright (C) NEC CASIO Mobile Communications, Ltd.
 *
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/uaccess.h>
#include <linux/timer.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/rwsem.h>
#include <linux/leds.h>
#include <linux/ioctl.h>
#include <linux/leds-diag_wrapper.h>
#include <linux/leds_cmd.h>

#include <mach/rpc_server_handset.h>

#include <linux/slab.h>
#include <linux/leds-adp8861.h>
#include <linux/leds-lcd-common.h>
#include "../../arch/arm/mach-msm/board-8960.h"

#define LEDDIAG_NAME       "led_diag_wrapper"

/* debug */
#define LED_DIAG_DEBUG_PLUS
/* debug */

static int led_diag_wrapper_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "led_diag_wrapper_open\n");
    return 0;
}

static int diag_leds_cmd_ctl(unsigned char red_f,
                             unsigned char grn_f,
                             unsigned char ble_f)
{
    int ret = LEDS_CMD_RET_OK;
    unsigned char ret_val;

    ret_val = leds_cmd(LEDS_CMD_TYPE_RGB_RED, red_f);
    if (ret_val != LEDS_CMD_RET_OK)
    {
        ret = LEDS_CMD_RET_NG;
    }
    ret_val = leds_cmd(LEDS_CMD_TYPE_RGB_GREEN, grn_f);
    if (ret_val != LEDS_CMD_RET_OK)
    {
        ret = LEDS_CMD_RET_NG;
    }
    ret_val = leds_cmd(LEDS_CMD_TYPE_RGB_BLUE, ble_f);
    if (ret_val != LEDS_CMD_RET_OK)
    {
        ret = LEDS_CMD_RET_NG;
    }
    
    return ret;
}

static long led_diag_wrapper_ioctl(struct file *file, unsigned int iocmd, unsigned long data)
{
    int ret = LED_DIAG_IOCTL_OK;
    int led_ret = LEDS_CMD_RET_OK;
    int err;
    unsigned char *pkt_params = NULL;
    unsigned char read_data = 0;
#if defined(CONFIG_FEATURE_NCMC_D121F) || defined(CONFIG_FEATURE_NCMC_KAMITSUKIGAME) || defined(CONFIG_FEATURE_NCMC_SYLPH)
	struct led_request_rgb key_request;
	union u_led_isc_reg led_key_bright1, led_key_bright2, led_key_bright3;
#endif /* #ifdef CONFIG_FEATURE_NCMC_D121F || CONFIG_FEATURE_NCMC_KAMITSUKIGAME || CONFIG_FEATURE_NCMC_SYLPH */
    
    size_t res_size = sizeof(char);
    size_t req_size = sizeof(char);
    
    printk(KERN_DEBUG "[diag_wrapper]%s: ioctl Enter (iocmd:0x%02X)\n", __func__,iocmd);

    switch(iocmd){
    case LED_DIAG_IOCTL_01:
        led_ret = diag_leds_cmd_ctl(LEDS_LED_ILLU_RED_LUMIN1, 0, 0);
        if( led_ret != LEDS_CMD_RET_OK ){
            ret = LED_DIAG_IOCTL_NG;
        }
        break;
        
    case LED_DIAG_IOCTL_02:
        led_ret = diag_leds_cmd_ctl(0, LEDS_LED_ILLU_GREEN_LUMIN1, 0);
        if( led_ret != LEDS_CMD_RET_OK ){
            ret = LED_DIAG_IOCTL_NG;
        }
        break;
        
    case LED_DIAG_IOCTL_03:	
        led_ret = diag_leds_cmd_ctl(0, 0, LEDS_LED_ILLU_BLUE_LUMIN1);
        if( led_ret != LEDS_CMD_RET_OK ){
            ret = LED_DIAG_IOCTL_NG;
        }
        break;
        
    case LED_DIAG_IOCTL_04:
        led_ret = diag_leds_cmd_ctl(LEDS_LED_ILLU_RED_LUMIN2, LEDS_LED_ILLU_GREEN_LUMIN2, 0);
        if( led_ret != LEDS_CMD_RET_OK ){
            ret = LED_DIAG_IOCTL_NG;
        }
        break;
        
    case LED_DIAG_IOCTL_05:	
        led_ret = diag_leds_cmd_ctl(LEDS_LED_ILLU_RED_LUMIN2, 0, LEDS_LED_ILLU_BLUE_LUMIN2);
        if( led_ret != LEDS_CMD_RET_OK ){
            ret = LED_DIAG_IOCTL_NG;
        }
        break;
        
    case LED_DIAG_IOCTL_06:	
        led_ret = diag_leds_cmd_ctl(0, LEDS_LED_ILLU_GREEN_LUMIN2, LEDS_LED_ILLU_BLUE_LUMIN2);
        if( led_ret != LEDS_CMD_RET_OK ){
            ret = LED_DIAG_IOCTL_NG;
        }
        break;
        
    case LED_DIAG_IOCTL_07:	
        led_ret = diag_leds_cmd_ctl(LEDS_LED_ILLU_RED_LUMIN3, LEDS_LED_ILLU_GREEN_LUMIN3, LEDS_LED_ILLU_BLUE_LUMIN3);
        if( led_ret != LEDS_CMD_RET_OK ){
            ret = LED_DIAG_IOCTL_NG;
        }
        break;
        
    case LED_DIAG_IOCTL_08:	
        led_ret = diag_leds_cmd_ctl(0, 0, 0);
        if( led_ret != LEDS_CMD_RET_OK ){
            ret = LED_DIAG_IOCTL_NG;
        }
        break;
        
    case LED_DIAG_IOCTL_09:	
        led_ret= leds_cmd(LEDS_CMD_TYPE_KEY, LEDS_LED_KEYBL1);
        if( led_ret != LEDS_CMD_RET_OK ){
            ret = LED_DIAG_IOCTL_NG;
        }
        break;

    case LED_DIAG_IOCTL_10:	
        led_ret = leds_cmd(LEDS_CMD_TYPE_KEY, 0);
        if( led_ret != LEDS_CMD_RET_OK ){
            ret = LED_DIAG_IOCTL_NG;
        }
        break;

#ifdef LED_DIAG_DEBUG_PLUS
    case LED_DIAG_IOCTL_14:		/* 3004 03 */
        led_ret = leds_cmd(LEDS_CMD_TYPE_FLASH_STILL, LEDS_SND_CAM_FLASH_LED1);
        if( led_ret != LEDS_CMD_RET_OK ){
            ret = LED_DIAG_IOCTL_NG;
        }
        break;

    case LED_DIAG_IOCTL_15:		/* 3004 04 */
        led_ret = leds_cmd(LEDS_CMD_TYPE_FLASH_MOVIE, LEDS_SND_CAM_FLASH_LED2);
        if( led_ret != LEDS_CMD_RET_OK ){
            ret = LED_DIAG_IOCTL_NG;
        }
        break;

    case LED_DIAG_IOCTL_16:		/* 3004 05 */
        led_ret = leds_cmd(LEDS_CMD_TYPE_FLASH_TORCH, LEDS_SND_CAM_FLASH_LED3);
        if( led_ret != LEDS_CMD_RET_OK ){
            ret = LED_DIAG_IOCTL_NG;
        }
        break;

    case LED_DIAG_IOCTL_17:		/* 3004 06 */
        led_ret = leds_cmd(LEDS_CMD_TYPE_FLASH, 0);
        if( led_ret != LEDS_CMD_RET_OK ){
            ret = LED_DIAG_IOCTL_NG;
        }
        break;

    case LED_DIAG_IOCTL_18:		/* 3004 07 */
        led_ret = leds_cmd(LEDS_CMD_TYPE_PREVENT_PEEPING, LEDS_LED_CAM_IND);
        if( led_ret != LEDS_CMD_RET_OK ){
            ret = LED_DIAG_IOCTL_NG;
        }
        break;

    case LED_DIAG_IOCTL_19:		/* 3004 08 */
        led_ret = leds_cmd(LEDS_CMD_TYPE_PREVENT_PEEPING, 0);
        if( led_ret != LEDS_CMD_RET_OK ){
            ret = LED_DIAG_IOCTL_NG;
        }
        break;
#endif	/* #ifdef LED_DIAG_DEBUG_PLUS */


    case LED_DIAG_IOCTL_12:		/* LCD back light ON (lm3537/lm3532) */
#ifdef CONFIG_FEATURE_NCMC_D121M
		led_ret = lm3532_main_lcd_don( LM3532_LED_ON );
#elif (defined(CONFIG_FEATURE_NCMC_KAMITSUKIGAME) || defined(CONFIG_FEATURE_NCMC_SYLPH))
		//CONFIG_CHMC_EARLY_TEMPORARY
                dw8402a_lcd_bl_on();
                //led_ret = LEDS_CMD_RET_OK;
		led_ret = lm3537_main_lcd_don( LM3537_LED_ON );
#else /* #ifdef CONFIG_FEATURE_NCMC_D121M */
		led_ret = lm3537_main_lcd_don( LM3537_LED_ON );
#endif /* #ifdef CONFIG_FEATURE_NCMC_D121M */
        if( led_ret < 0 ){
            ret = LED_DIAG_IOCTL_NG;
        }
        break;

    case LED_DIAG_IOCTL_13:		/* LCD back light OFF (lm3537/lm3532) */
#ifdef CONFIG_FEATURE_NCMC_D121M
		led_ret = lm3532_main_lcd_don( LM3532_LED_OFF );
#elif (defined(CONFIG_FEATURE_NCMC_KAMITSUKIGAME) || defined(CONFIG_FEATURE_NCMC_SYLPH))
		//CONFIG_CHMC_EARLY_TEMPORARY
                dw8402a_lcd_bl_off();
                //led_ret = LEDS_CMD_RET_OK;
		led_ret = lm3537_main_lcd_don( LM3537_LED_OFF );
#else /* #ifdef CONFIG_FEATURE_NCMC_D121M */
		led_ret = lm3537_main_lcd_don( LM3537_LED_OFF );
#endif /* #ifdef CONFIG_FEATURE_NCMC_D121M */
        if( led_ret < 0 ){
            ret = LED_DIAG_IOCTL_NG;
        }
        break;
    case LED_DIAG_IOCTL_46:
        req_size = ( sizeof(char) * 2 );
        pkt_params = (char *)kmalloc(req_size, GFP_KERNEL);
        if( !pkt_params )
            return -ENOMEM;
            
        err = copy_from_user( pkt_params, (unsigned char *)data, req_size );
        if (err) {
            kfree(pkt_params);
            return LED_DIAG_IOCTL_NG;
        }
        
#ifdef CONFIG_FEATURE_NCMC_D121M
        ret = lm3532_i2c_smbus_write(0x17, pkt_params[0]);
//        ret |= led_main_lcd_set( LM3532_LED_ON, LM3532_LED_OFF );
		ret |= lm3532_i2c_smbus_write( 0x13, 0x02 );
		ret |= lm3532_i2c_smbus_write( 0x1D, 0x01 );
        if( ret != LM3532_LED_SET_OK )
            ret = LED_DIAG_IOCTL_NG;
#elif (defined(CONFIG_FEATURE_NCMC_KAMITSUKIGAME) || defined(CONFIG_FEATURE_NCMC_SYLPH))
		//CONFIG_CHMC_EARLY_TEMPORARY
		ret = LED_DIAG_IOCTL_NG;
#else /* #ifdef CONFIG_FEATURE_NCMC_D121M */
        ret = lm3537_i2c_smbus_write(0xA0, pkt_params[0]);
//        ret |= led_main_lcd_set( LM3537_LED_ON, LM3537_LED_OFF );
		ret |= lm3537_i2c_smbus_write( 0x10, 0xFF );
		ret |= lm3537_i2c_smbus_write( 0x00, 0x04 );
        if( ret != LM3537_LED_SET_OK )
            ret = LED_DIAG_IOCTL_NG;
#endif /* #ifdef CONFIG_FEATURE_NCMC_D121M */
        break;

    case LED_DIAG_IOCTL_30:	
        req_size = ( sizeof(char) * 2 );
        pkt_params = (char *)kmalloc(req_size, GFP_KERNEL);
        if( !pkt_params ){
            return -ENOMEM;
        }
        err = copy_from_user( pkt_params, (unsigned char *)data, req_size );
        if (err) {
            kfree(pkt_params);
            return LED_DIAG_IOCTL_NG;
        }
#ifdef CONFIG_FEATURE_NCMC_D121M
        ret = lm3532_i2c_smbus_write(pkt_params[0], pkt_params[1]);
#elif (defined(CONFIG_FEATURE_NCMC_KAMITSUKIGAME) || defined(CONFIG_FEATURE_NCMC_SYLPH))
		//CONFIG_CHMC_EARLY_TEMPORARY
		ret = LED_DIAG_IOCTL_NG;
#else /* #ifdef CONFIG_FEATURE_NCMC_D121M */
        ret = lm3537_i2c_smbus_write(pkt_params[0], pkt_params[1]);
#endif /* #ifdef CONFIG_FEATURE_NCMC_D121M */
        break;

    case LED_DIAG_IOCTL_31:
        req_size = ( sizeof(char) * 2 );
        pkt_params = (char *)kmalloc(req_size, GFP_KERNEL);
        if( !pkt_params ){
            return -ENOMEM;
        }
        err = copy_from_user( pkt_params, (unsigned char *)data, req_size );
        if (err) {
            kfree(pkt_params);
            return LED_DIAG_IOCTL_NG;
        }
        
        if(pkt_params[0] != 0x00){
#ifdef CONFIG_FEATURE_NCMC_D121M
			ret = lm3532_i2c_smbus_read(pkt_params[0], &read_data);
#elif (defined(CONFIG_FEATURE_NCMC_KAMITSUKIGAME) || defined(CONFIG_FEATURE_NCMC_SYLPH))
		//CONFIG_CHMC_EARLY_TEMPORARY
		ret = LED_DIAG_IOCTL_NG;
#else /* #ifdef CONFIG_FEATURE_NCMC_D121M */
			ret = lm3537_i2c_smbus_read(pkt_params[0], &read_data);
#endif /* #ifdef CONFIG_FEATURE_NCMC_D121M */
		}
		pkt_params[1] = read_data;
        break;

    case LED_DIAG_IOCTL_32:
        req_size = ( sizeof(char) * 2 );
        pkt_params = (char *)kmalloc(req_size, GFP_KERNEL);
        if( !pkt_params ){
            return -ENOMEM;
        }
        err = copy_from_user( pkt_params, (unsigned char *)data, req_size );
        if (err) {
            kfree(pkt_params);
            return LED_DIAG_IOCTL_NG;
        }
        ret = adp8861_reg_write(pkt_params[0], pkt_params[1]);
        break;
        
    case LED_DIAG_IOCTL_33:	
        req_size = ( sizeof(char) * 2 );
        pkt_params = (char *)kmalloc(req_size, GFP_KERNEL);
        if( !pkt_params ){
            return -ENOMEM;
        }
        err = copy_from_user( pkt_params, (unsigned char *)data, req_size );
        if (err) {
            kfree(pkt_params);
            return LED_DIAG_IOCTL_NG;
        }
        ret = adp8861_i2c_smbus_read(pkt_params[0], &pkt_params[1]);
        break;
        
    case LED_DIAG_IOCTL_47:
        led_ret= leds_cmd(LEDS_CMD_TYPE_PREVENT_PEEPING, LEDS_LED_CAM_IND);
        if( led_ret != LEDS_CMD_RET_OK ){
            ret = LED_DIAG_IOCTL_NG;
        }
        break;

    case LED_DIAG_IOCTL_48:	
        led_ret = leds_cmd(LEDS_CMD_TYPE_PREVENT_PEEPING, 0);
        if( led_ret != LEDS_CMD_RET_OK ){
            ret = LED_DIAG_IOCTL_NG;
        }
        break;

    case LED_DIAG_IOCTL_49:	
        led_ret = leds_cmd(LEDS_CMD_TYPE_FLASH_MOVIE, 0);
        if( led_ret != LEDS_CMD_RET_OK ){
            ret = LED_DIAG_IOCTL_NG;
        }
        break;

    case LED_DIAG_IOCTL_50:	
        led_ret = leds_cmd(LEDS_CMD_TYPE_FLASH_MOVIE, LEDS_SND_CAM_FLASH_LED2);
        if( led_ret != LEDS_CMD_RET_OK ){
            ret = LED_DIAG_IOCTL_NG;
        }
        break;

#if defined(CONFIG_FEATURE_NCMC_D121F) || defined(CONFIG_FEATURE_NCMC_KAMITSUKIGAME) || defined(CONFIG_FEATURE_NCMC_SYLPH)
    case LED_DIAG_IOCTL_90:	
		led_key_bright1.us = 0;
		led_key_bright1.st2.set_flag = 1;
		led_key_bright1.st2.scd = 0x1B;
		led_key_bright2.us = 0;
		led_key_bright2.st2.set_flag = 1;
		led_key_bright2.st2.scd = 0x00;
		led_key_bright3.us = 0;
		led_key_bright3.st2.set_flag = 1;
		led_key_bright3.st2.scd = 0x00;
		led_ret  = adp8861_key_led_bright( &led_key_bright1, &led_key_bright2, &led_key_bright3 );
		key_request.dmy1  = 0;
		key_request.set_r = 1;
		key_request.set_g = 0;
		key_request.set_b = 0;
		led_ret |= adp8861_key_led_set( &key_request );
        if( led_ret != LEDS_CMD_RET_OK ){
            ret = LED_DIAG_IOCTL_NG;
        }
        break;

    case LED_DIAG_IOCTL_91:	
		led_key_bright1.us = 0;
		led_key_bright1.st2.set_flag = 1;
		led_key_bright1.st2.scd = 0x00;
		led_key_bright2.us = 0;
		led_key_bright2.st2.set_flag = 1;
		led_key_bright2.st2.scd = 0x1B;
		led_key_bright3.us = 0;
		led_key_bright3.st2.set_flag = 1;
		led_key_bright3.st2.scd = 0x00;
		led_ret  = adp8861_key_led_bright( &led_key_bright1, &led_key_bright2, &led_key_bright3 );
		key_request.dmy1  = 0;
		key_request.set_r = 0;
		key_request.set_g = 1;
		key_request.set_b = 0;
		led_ret |= adp8861_key_led_set( &key_request );
        if( led_ret != LEDS_CMD_RET_OK ){
            ret = LED_DIAG_IOCTL_NG;
        }
        break;

    case LED_DIAG_IOCTL_92:	
		led_key_bright1.us = 0;
		led_key_bright1.st2.set_flag = 1;
		led_key_bright1.st2.scd = 0x00;
		led_key_bright2.us = 0;
		led_key_bright2.st2.set_flag = 1;
		led_key_bright2.st2.scd = 0x00;
		led_key_bright3.us = 0;
		led_key_bright3.st2.set_flag = 1;
		led_key_bright3.st2.scd = 0x1B;
		led_ret  = adp8861_key_led_bright( &led_key_bright1, &led_key_bright2, &led_key_bright3 );
		key_request.dmy1  = 0;
		key_request.set_r = 0;
		key_request.set_g = 0;
		key_request.set_b = 1;
		led_ret |= adp8861_key_led_set( &key_request );
        if( led_ret != LEDS_CMD_RET_OK ){
            ret = LED_DIAG_IOCTL_NG;
        }
        break;

    case LED_DIAG_IOCTL_93:
		led_key_bright1.us = 0;
		led_key_bright1.st2.set_flag = 1;
		led_key_bright1.st2.scd = 0x12;
		led_key_bright2.us = 0;
		led_key_bright2.st2.set_flag = 1;
		led_key_bright2.st2.scd = 0x12;
		led_key_bright3.us = 0;
		led_key_bright3.st2.set_flag = 1;
		led_key_bright3.st2.scd = 0x00;
		led_ret  = adp8861_key_led_bright( &led_key_bright1, &led_key_bright2, &led_key_bright3 );
		key_request.dmy1  = 0;
		key_request.set_r = 1;
		key_request.set_g = 1;
		key_request.set_b = 0;
		led_ret |= adp8861_key_led_set( &key_request );
        if( led_ret != LEDS_CMD_RET_OK ){
            ret = LED_DIAG_IOCTL_NG;
        }
        break;

    case LED_DIAG_IOCTL_94:
		led_key_bright1.us = 0;
		led_key_bright1.st2.set_flag = 1;
		led_key_bright1.st2.scd = 0x12;
		led_key_bright2.us = 0;
		led_key_bright2.st2.set_flag = 1;
		led_key_bright2.st2.scd = 0x00;
		led_key_bright3.us = 0;
		led_key_bright3.st2.set_flag = 1;
		led_key_bright3.st2.scd = 0x12;
		led_ret  = adp8861_key_led_bright( &led_key_bright1, &led_key_bright2, &led_key_bright3 );
		key_request.dmy1  = 0;
		key_request.set_r = 1;
		key_request.set_g = 0;
		key_request.set_b = 1;
		led_ret |= adp8861_key_led_set( &key_request );
        if( led_ret != LEDS_CMD_RET_OK ){
            ret = LED_DIAG_IOCTL_NG;
        }
        break;

    case LED_DIAG_IOCTL_95:
		led_key_bright1.us = 0;
		led_key_bright1.st2.set_flag = 1;
		led_key_bright1.st2.scd = 0x00;
		led_key_bright2.us = 0;
		led_key_bright2.st2.set_flag = 1;
		led_key_bright2.st2.scd = 0x12;
		led_key_bright3.us = 0;
		led_key_bright3.st2.set_flag = 1;
		led_key_bright3.st2.scd = 0x12;
		led_ret  = adp8861_key_led_bright( &led_key_bright1, &led_key_bright2, &led_key_bright3 );
		key_request.dmy1  = 0;
		key_request.set_r = 0;
		key_request.set_g = 1;
		key_request.set_b = 1;
		led_ret |= adp8861_key_led_set( &key_request );
        if( led_ret != LEDS_CMD_RET_OK ){
            ret = LED_DIAG_IOCTL_NG;
        }
        break;

    case LED_DIAG_IOCTL_96:	
		led_key_bright1.us = 0;
		led_key_bright1.st2.set_flag = 1;
		led_key_bright1.st2.scd = 0x1A;
		led_key_bright2.us = 0;
		led_key_bright2.st2.set_flag = 1;
		led_key_bright2.st2.scd = 0x11;
		led_key_bright3.us = 0;
		led_key_bright3.st2.set_flag = 1;
		led_key_bright3.st2.scd = 0x0D;
		led_ret  = adp8861_key_led_bright( &led_key_bright1, &led_key_bright2, &led_key_bright3 );
		key_request.dmy1  = 0;
		key_request.set_r = 1;
		key_request.set_g = 1;
		key_request.set_b = 1;
		led_ret |= adp8861_key_led_set( &key_request );
        if( led_ret != LEDS_CMD_RET_OK ){
            ret = LED_DIAG_IOCTL_NG;
        }
        break;

    case LED_DIAG_IOCTL_97:
		led_key_bright1.us = 0;
		led_key_bright1.st2.set_flag = 1;
		led_key_bright1.st2.scd = 0x00;
		led_key_bright2.us = 0;
		led_key_bright2.st2.set_flag = 1;
		led_key_bright2.st2.scd = 0x00;
		led_key_bright3.us = 0;
		led_key_bright3.st2.set_flag = 1;
		led_key_bright3.st2.scd = 0x00;
		led_ret  = adp8861_key_led_bright( &led_key_bright1, &led_key_bright2, &led_key_bright3 );
		key_request.dmy1  = 0;
		key_request.set_r = 0;
		key_request.set_g = 0;
		key_request.set_b = 0;
		led_ret |= adp8861_key_led_set( &key_request );
        if( led_ret != LEDS_CMD_RET_OK ){
            ret = LED_DIAG_IOCTL_NG;
        }
        break;
#endif /* #ifdef CONFIG_FEATURE_NCMC_D121F || CONFIG_FEATURE_NCMC_KAMITSUKIGAME || CONFIG_FEATURE_NCMC_SYLPH */

    default:
        ret = LED_DIAG_IOCTL_NG;
        break;
    }
    
    if(pkt_params){
        err = copy_to_user((unsigned char *)data, pkt_params, res_size);
        if (err) {
            ret = -1;
        }
        kfree(pkt_params);
    }
    printk(KERN_DEBUG "[diag_wrapper]%s: ioctl Exit\n", __func__);
    return ret;
}

static const struct file_operations led_diag_wrapper_fops = {
    .owner      = THIS_MODULE,
    .open       = led_diag_wrapper_open,
//    .ioctl      = led_diag_wrapper_ioctl,
    .unlocked_ioctl = led_diag_wrapper_ioctl,
};

static struct miscdevice led_diag = {
    .fops       = &led_diag_wrapper_fops,
    .name       = LEDDIAG_NAME,
    .minor      = MISC_DYNAMIC_MINOR,
};

static int __init led_diag_wrapper_init(void)
{
	printk(KERN_DEBUG "[led_diag]%s: init Enter\n", __func__);
    return misc_register(&led_diag);
}

static void __exit led_diag_wrapper_exit(void)
{
    misc_deregister(&led_diag);
}

module_init(led_diag_wrapper_init);
module_exit(led_diag_wrapper_exit);


