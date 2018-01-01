/*
 * leds-data_input.h
 *
 * Copyright (C) NEC CASIO Mobile Communications, Ltd.
 *
 */

#ifndef __LINUX_LEDS_DATA_INPUT_H
#define __LINUX_LEDS_DATA_INPUT_H

#include <linux/ioctl.h>

enum eled_led_type {
	LED_TYPE_INCOMING = 0,
	LED_TYPE_FRONT,
	LED_TYPE_BACK,
	LED_TYPE_CAMERA
};
enum eled_led_state_type {
	LED_STATE_STOP = 0,
	LED_STATE_PATERN,
	LED_STATE_LOCAL_OPE,
	LED_STATE_SYNC_OPE_WAIT,
	LED_STATE_SYNC_OPE,
	LED_STATE_SYNC_OPE_CANCEL
};

enum eled_led_state_sub_type1 {
	LED_STATE_SUB_3LED = 0,
	LED_STATE_SUB_BACKILM
};
enum eled_led_state_sub_type2 {
	LED_STATE_SUB_BACKILM_STOP = 0,
	LED_STATE_SUB_BACKILM_WAIT,
	LED_STATE_SUB_BACKILM_RUN
};

enum eled_led_state_patern_type {
	LED_STATE_PATERN_NON = 0,
	LED_STATE_PATERN_FILE,
	LED_STATE_PATERN_FOLLOW_FILE
};

enum eled_led_camera_state_type {
	LED_STATE_CAM_END = 0,
	LED_STATE_CAM_START,
	LED_STATE_CAM_PATERN,
	LED_STATE_CAM_RUN
};

enum eled_led_file_ctrl_type {
	LED_TYPE_N_KEEP = 0,
	LED_TYPE_N_SET,
	LED_TYPE_B_KEEP,
	LED_TYPE_B_SET
};

#define	LED_IOC_LIGHTS_3LED				_IOR('u', 0x20, int)
#define	LED_IOC_NCMLIGHTS_KEYILM		_IOR('u', 0x21, int)
#define	LED_IOC_NCMLIGHTS_BACKILM		_IOR('u', 0x22, int)
#define	LED_IOC_DATA_SET				_IOR('u', 0x23, int)
#define	LED_IOC_NCMLIGHTS_CAMERA		_IOR('u', 0x24, int)
#define	LED_IOC_FILE_CTRL				_IOR('u', 0x25, int)


#define	LED_STATE_OK		-1
#define	LED_STATE_ERR		-2


typedef struct input_ioctl_data_struct {
	int				type;
	int				loop_flag;
	unsigned int 	loop_count;
} input_ioctl_data_type;

#pragma pack(push, 1)
typedef struct input_file_head_struct {
	unsigned char	file_type[6];
	unsigned short	length;
	unsigned char	title[20];
	unsigned short	copy;
	unsigned char	date[12];
	unsigned char	led_num;
	unsigned short	time;
	unsigned char	frame_num;
} input_file_head_type;

typedef struct input_data_head_struct {
	unsigned int	data_size;
	unsigned char	led_type;
	unsigned char	led_num;
	unsigned char	frame_num;
	unsigned int	frame_size;
} input_data_head_type;

typedef struct input_es_time_struct {
	unsigned char	time_b:4;
	unsigned char	time_a:4;
} input_es_time_type;

typedef union u_input_es_time {
	struct input_es_time_struct	s_tmie;
	unsigned char			time;
} u_input_es_time_type;

typedef struct input_pck_time_struct {
	unsigned char	trgb:3;
	unsigned char	dmy1:1;
	unsigned char	srrgb:2;
	unsigned char	sfrgb:2;
} input_pck_time_type;

typedef union u_input_pck_time {
	struct input_pck_time_struct	s_tmie;
	unsigned char					time;
} u_input_pck_time_type;

typedef struct input_frame_es_fade_struct {
	unsigned char	fade:1;
	unsigned char	dmy1:3;
	unsigned char	hw_act:1;
	unsigned char	hw_on_off:1;
	unsigned char	hw_set_data:2;
} input_frame_es_fade_type;

typedef struct input_frame_es_struct {
	union u_input_es_time	u_time;
	unsigned char	reserv;
	struct input_frame_es_fade_struct	fade;
//	unsigned char	fade;
	unsigned char	brightness[3];
} input_frame_es_type;

typedef struct input_frame_pck_led1_struct {
	unsigned char			brightness[6];
	unsigned char			patern[3];
	union u_input_pck_time	u_led1_time;
} input_frame_pck_led1_type;

typedef struct input_frame_pck_led2_struct {
	unsigned char			brightness[12];
	unsigned char			patern[6];
	union u_input_pck_time	u_led1_time;
	union u_input_pck_time	u_led2_time;
} input_frame_pck_led2_type;
#pragma pack(pop)

#endif /* __LINUX_LEDS_DATA_INPUT_H */
