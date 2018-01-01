/*
 * leds-adp8861.h - platform data structure for adp8861 led controller
 *
 * Copyright (C) 2011 NEC NEC Corporation
 *
 */
 
#ifndef __LINUX_ADP8861_H
#define __LINUX_ADP8861_H

#include <linux/leds.h>

/* define ------------------------------*/

#define ADP8861_LED_SET_OK	0
#define ADP8861_LED_SET_NG	(-1)
#define ADP8861_LED_SET_RESERVED	(1)

#define ADP8861_LED_REG_WRITE_NG	(0x80)

#define ADP8861_LED_ON		(1)
#define ADP8861_LED_OFF		(0)

/* structure ------------------------------*/
struct leds_adp8861_platform_data {
	int (*poweron)(struct device *);
	int (*poweroff)(struct device *);
};

/* 0x00 */
struct led_mfdvid_reg{
	unsigned char	bit0:1			;
	unsigned char	bit1:1			;
	unsigned char	bit2:1			;
	unsigned char	bit3:1			;
	unsigned char	bit4:1			;
	unsigned char	bit5:1			;
	unsigned char	bit6:1			;
	unsigned char	bit7:1			;
};
union u_led_mfdvid_reg{
	struct led_mfdvid_reg	st;
	u8						uc;
};

/* 0x01 */
struct led_mdcr_reg{
	unsigned char	bl_en:1			;
	unsigned char	dmy1:1			;
	unsigned char	sis_en:1		;
	unsigned char	gdwn_dis:1		;
	unsigned char	dim_en:1		;
	unsigned char	nstby:1			;
	unsigned char	int_cfg:1		;
	unsigned char	dmy2:1			;
};
union u_led_mdcr_reg{
	struct led_mdcr_reg		st;
	u8						uc;
};

/* 0x02 */
struct led_mdcr2_reg{
	unsigned char	dmy1:2			;
	unsigned char	ovp_int:1		;
	unsigned char	tsd_int:1		;
	unsigned char	short_int:1		;
	unsigned char	dmy2:3			;
};
union u_led_mdcr2_reg{
	struct led_mdcr2_reg	st;
	u8						uc;
};

/* 0x03 */
struct led_intr_en_reg{
	unsigned char	dmy1:2			;
	unsigned char	ovp_ien:1		;
	unsigned char	tsd_ien:1		;
	unsigned char	short_ien:1		;
	unsigned char	dmy2:3			;
};
union u_led_intr_en2_reg{
	struct led_intr_en_reg	st;
	u8						uc;
};

/* 0x04 */
struct led_cfgr_reg_1{
	unsigned char	fovr:1			;
	unsigned char	law_0:1			;
	unsigned char	law_1:1			;
	unsigned char	dmy1:5			;
};
struct led_cfgr_reg_2{
	unsigned char	fovr:1			;
	unsigned char	law:2			;
	unsigned char	dmy1:5			;
};
union u_led_cfgr_reg{
	struct led_cfgr_reg_1	st1;
	struct led_cfgr_reg_2	st2;
	u8						uc;
};

/* 0x05 */
struct led_blsen_reg{
	unsigned char	d1en:1			;
	unsigned char	d2en:1			;
	unsigned char	d3en:1			;
	unsigned char	d4en:1			;
	unsigned char	d5en:1			;
	unsigned char	d6en:1			;
	unsigned char	d7en:1			;
	unsigned char	dmy1:1			;
};
union u_led_blsen_reg{
	struct led_blsen_reg	st;
	u8						uc;
};

/* 0x06 */
struct led_bloff_reg{
	unsigned char	offt_1:1		;
	unsigned char	offt_2:1		;
	unsigned char	offt_3:1		;
	unsigned char	offt_4:1		;
	unsigned char	offt_5:1		;
	unsigned char	offt_6:1		;
	unsigned char	offt_7:1		;
	unsigned char	dmy1:1			;
};
union u_led_bloff_reg{
	struct led_bloff_reg	st;
	u8						uc;
};

/* 0x07 */
/*
struct led_bldim_reg_1{
	unsigned char	dimt_1:1		;
	unsigned char	dimt_2:1		;
	unsigned char	dimt_3:1		;
	unsigned char	dimt_4:1		;
	unsigned char	dimt_5:1		;
	unsigned char	dimt_6:1		;
	unsigned char	dimt_7:1		;
	unsigned char	dmy1:1			;
};
*/
struct led_bldim_reg_2{
	unsigned char	dimt:7			;
	unsigned char	dmy1:1			;
};
union u_led_bldim_reg{
/*	struct led_bldim_reg_1	st1;*/
	struct led_bldim_reg_2	st2;
	u8						uc;
};

/* 0x08 */
/*
struct led_blfr_reg_1{
	unsigned char	bl_fi_1:1		;
	unsigned char	bl_fi_2:1		;
	unsigned char	bl_fi_3:1		;
	unsigned char	bl_fi_4:1		;
	unsigned char	bl_fo_1:1		;
	unsigned char	bl_fo_2:1		;
	unsigned char	bl_fo_3:1		;
	unsigned char	bl_fo_4:1		;
};
*/
struct led_blfr_reg_2{
	unsigned char	bl_fi:4			;
	unsigned char	bl_fo:4			;
};
union u_led_blfr_reg{
/*	struct led_blfr_reg_1	st1;*/
	struct led_blfr_reg_2	st2;
	u8						uc;
};

/* 0x09 */
/*
struct led_blmx_reg{
	unsigned char	bl_mc_1:1		;
	unsigned char	bl_mc_2:1		;
	unsigned char	bl_mc_3:1		;
	unsigned char	bl_mc_4:1		;
	unsigned char	bl_mc_5:1		;
	unsigned char	bl_mc_6:1		;
	unsigned char	bl_mc_7:1		;
	unsigned char	bl_mc_8:1		;
};
*/
union u_led_blmx_reg{
/*	struct led_blmx_reg		st1;*/
	unsigned char			bl_mc;
	u8						uc;
};

/* 0x0A */
/*
struct led_bldm_reg_1{
	unsigned char	bl_dc_1:1		;
	unsigned char	bl_dc_2:1		;
	unsigned char	bl_dc_3:1		;
	unsigned char	bl_dc_4:1		;
	unsigned char	bl_dc_5:1		;
	unsigned char	bl_dc_6:1		;
	unsigned char	bl_dc_7:1		;
	unsigned char	dmy1:1			;
};
*/
struct led_bldm_reg_2{
	unsigned char	bl_dc:7			;
	unsigned char	dmy1:1			;
};
union u_led_bldm_reg{
/*	struct led_bldm_reg_1	st1;*/
	struct led_bldm_reg_2	st2;
	u8						uc;
};

/* 0x0F */
/*
struct led_iscfr_reg_1{
	unsigned char	sc_law_0:1		;
	unsigned char	sc_law_1:1		;
	unsigned char	dmy1:6			;
};
*/
struct led_iscfr_reg_2{
	unsigned char	sc_law:2		;
	unsigned char	dmy1:6			;
};
union u_led_iscfr_reg{
/*	struct led_iscfr_reg_1	st1;*/
	struct led_iscfr_reg_2	st2;
	u8						uc;
};

/* 0x10 */
struct led_iscc_reg{
	unsigned char	sc1_en:1		;
	unsigned char	sc2_en:1		;
	unsigned char	sc3_en:1		;
	unsigned char	sc4_en:1		;
	unsigned char	sc5_en:1		;
	unsigned char	sc6_en:1		;
	unsigned char	sc7_en:1		;
	unsigned char	dmy1:1			;
};
union u_led_iscc_reg{
	struct led_iscc_reg		st;
	u8						uc;
};

/* 0x11 */
/*
struct led_isct1_reg_1{
	unsigned char	sc5_off_1:1		;
	unsigned char	sc5_off_2:1		;
	unsigned char	sc6_off_1:1		;
	unsigned char	sc6_off_2:1		;
	unsigned char	sc7_off_1:1		;
	unsigned char	sc7_off_2:1		;
	unsigned char	scon_1:1		;
	unsigned char	scon_2:1		;
};
*/
struct led_isct1_reg_2{
	unsigned char	sc5_off:2		;
	unsigned char	sc6_off:2		;
	unsigned char	sc7_off:2		;
	unsigned char	scon:2			;
};
union u_led_isct1_reg{
/*	struct led_isct1_reg_1		st1;*/
	struct led_isct1_reg_2		st2;
	u8							uc;
};

/* 0x12 */
/*
struct led_isct2_reg_1{
	unsigned char	sc1_off_1:1		;
	unsigned char	sc1_off_2:1		;
	unsigned char	sc2_off_1:1		;
	unsigned char	sc2_off_2:1		;
	unsigned char	sc3_off_1:1		;
	unsigned char	sc3_off_2:1		;
	unsigned char	sc4_off_1:1		;
	unsigned char	sc4_off_2:1		;
};
*/
struct led_isct2_reg_2{
	unsigned char	sc1_off:2		;
	unsigned char	sc2_off:2		;
	unsigned char	sc3_off:2		;
	unsigned char	sc4_off:2		;
};
union u_led_isct2_reg{
/*	struct led_isct2_reg_1	st1;*/
	struct led_isct2_reg_2	st2;
	u8						uc;
};

/* 0x13 */
/*
struct led_iscf_reg_1{
	unsigned char	scfi_1:1		;
	unsigned char	scfi_2:1		;
	unsigned char	scfi_3:1		;
	unsigned char	scfi_4:1		;
	unsigned char	scfo_1:1		;
	unsigned char	scfo_2:1		;
	unsigned char	scfo_3:1		;
	unsigned char	scfo_4:1		;
};
*/
struct led_iscf_reg_2{
	unsigned char	scfi:4			;
	unsigned char	scfo:4			;
};
union u_led_iscf_reg{
/*	struct led_iscf_reg_1	st1;*/
	struct led_iscf_reg_2	st2;
	u8						uc;
};

/* 0x14 ~ 0x1A */
/*
struct led_isc_reg_1{
	unsigned short	scd_1:1			;
	unsigned short	scd_2:1			;
	unsigned short	scd_3:1			;
	unsigned short	scd_4:1			;
	unsigned short	scd_5:1			;
	unsigned short	scd_6:1			;
	unsigned short	scd_7:1			;
	unsigned short	scr:1			;
	
	unsigned short	set_flag:1		;
	unsigned short	dmy1:7			;
};
*/
struct led_isc_reg_2{
	unsigned short	scd:7			;
	unsigned short	scr:1			;
	
	unsigned short	set_flag:1		;
	unsigned short	dmy1:7			;
};
struct led_isc_reg_3{
	unsigned char	uc1;
	unsigned char	uc2;
};
union u_led_isc_reg{
/*	struct led_isc_reg_1	st1;*/
	struct led_isc_reg_2	st2;
	struct led_isc_reg_3	st3;
	unsigned short			us;
};

struct led_request{
	unsigned char	set:1			;
	unsigned short	flash_flag:1	;
	unsigned char	dmy1:6			;
};
struct led_request_rgb{
	unsigned char	set_r:1			;
	unsigned char	set_g:1			;
	unsigned char	set_b:1			;
	unsigned char	dmy1:5			;
};
struct led_request_key{
	unsigned char	set_1:1			;
	unsigned char	dmy1:7			;
};

/*-----------------------------------------*/

/* Command ID */
#define LEDS_CMD_TYPE_RGB_RED		0x01
#define LEDS_CMD_TYPE_RGB_GREEN		0x02
#define LEDS_CMD_TYPE_RGB_BLUE		0x03
#define LEDS_CMD_TYPE_KEY			0x04
#define LEDS_CMD_TYPE_FLASH_MOVIE	0x05
#define LEDS_CMD_TYPE_FLASH_STILL	0x06
#define LEDS_CMD_TYPE_FLASH_TORCH	0x07
#define LEDS_CMD_TYPE_PREVENT_PEEPING	0x08

/* Return value */
#define LEDS_CMD_RET_OK				1
#define LEDS_CMD_RET_NG				0

#define LEDS_LED_ILLU_RED_LUMIN1		0x2A
#define LEDS_LED_ILLU_RED_LUMIN2		0x19
#define LEDS_LED_ILLU_RED_LUMIN3		0x15
#define LEDS_LED_ILLU_GREEN_LUMIN1		0x2A
#define LEDS_LED_ILLU_GREEN_LUMIN2		0x19
#define LEDS_LED_ILLU_GREEN_LUMIN3		0x15
#define LEDS_LED_ILLU_BLUE_LUMIN1		0x2A
#define LEDS_LED_ILLU_BLUE_LUMIN2		0x19
#define LEDS_LED_ILLU_BLUE_LUMIN3		0x15
#define LEDS_LED_KEYBL1					0x2A
#define LEDS_LED_KEYBL2					0x2A
#define LEDS_LED_KEYBL3					0x2A
#ifdef CONFIG_FEATURE_NCMC_D121F
#define LEDS_SND_CAM_FLASH_LED1			0x7F
#define LEDS_SND_CAM_FLASH_LED2			0x53
#define LEDS_SND_CAM_FLASH_LED3			0x33
#else /* #ifdef CONFIG_FEATURE_NCMC_D121F */
#define LEDS_SND_CAM_FLASH_LED1			0xD4
#define LEDS_SND_CAM_FLASH_LED2			0xD4
#define LEDS_SND_CAM_FLASH_LED3			0xD4
#endif /* #ifdef CONFIG_FEATURE_NCMC_D121F */

#define LEDS_LED_CAM_IND				0x15

/* misc */
#define ADP8861_IOCTL_MAGIC 'l'

#define LED_CLASS_WRAPPER_FD_PATH    "/dev/led_class_wrapper"

struct adp8861_led_flash_parame {
	char still;
	char video;
	char torch;
};

struct adp8861_led_prevent_peeping_parame {
	char prevent_peeping ;
};

#define ADP8861_CUSTOM_N40    _IOW(ADP8861_IOCTL_MAGIC, 0x40, struct adp8861_led_flash_parame *)
#define ADP8861_CUSTOM_N43    _IOW(ADP8861_IOCTL_MAGIC, 0x43, struct adp8861_led_prevent_peeping_parame *)

/* extern */

extern int adp8861_rgb_led_bright( union u_led_isc_reg *red_bright,
								   union u_led_isc_reg *green_bright,
								   union u_led_isc_reg *blue_bright);
extern int adp8861_rgb_led_set(struct led_request_rgb *request);
#ifdef CONFIG_FEATURE_NCMC_D121F
extern int adp8861_key_led_bright( union u_led_isc_reg *led_key_bright1,union u_led_isc_reg *led_key_bright2,union u_led_isc_reg *led_key_bright3);
extern int adp8861_key_led_set(struct led_request_rgb *request);
#else /* #ifdef CONFIG_FEATURE_NCMC_D121F */
extern int adp8861_key_led_bright( union u_led_isc_reg *led_key_bright1);
extern int adp8861_key_led_set(struct led_request_key *request);
#endif /* #ifdef CONFIG_FEATURE_NCMC_D121F */
extern int adp8861_flash_led_bright( union u_led_isc_reg *led_flash_bright );
extern int adp8861_flash_led_set(struct led_request *request);
extern int adp8861_led_fade_set( union u_led_iscf_reg *led_fade );
extern int adp8861_led_timer_set1( union u_led_isct1_reg *led_timer_set,
								   union u_led_isct1_reg *led_timer_data );
extern int adp8861_led_timer_set2( union u_led_isct2_reg *led_timer_set,
								   union u_led_isct2_reg *led_timer_data );

extern int adp8861_reg_write( unsigned char sub2, unsigned char inf1 );
extern int adp8861_i2c_smbus_read(u8 command, u8 *value);

extern void led_brightness_set_keyilm_status( int status );

#endif /* __LINUX_ADP8861_H */

/* File END */
