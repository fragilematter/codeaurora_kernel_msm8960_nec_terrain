/*
 * leds-data_input.c
 *
 * Copyright (C) NEC CASIO Mobile Communications, Ltd.
 *
 */
 
#include <linux/kernel.h>
#include <linux/wait.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/leds.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/mm.h>
#include <asm/page.h>
#include <linux/kthread.h>
#include <linux/mutex.h>
#include <linux/wakelock.h>

#ifndef LOCAL_CONFIG_FEATURE_NCMC_LED_12_1ST
#if defined(CONFIG_FEATURE_NCMC_D121F) || defined(CONFIG_FEATURE_NCMC_G121S) || defined(CONFIG_FEATURE_NCMC_D121M) || defined(CONFIG_FEATURE_NCMC_G121T) || defined(CONFIG_FEATURE_NCMC_G121U) || defined(CONFIG_FEATURE_NCMC_KAMITSUKIGAME) || defined(CONFIG_FEATURE_NCMC_SYLPH)
#define LOCAL_CONFIG_FEATURE_NCMC_LED_12_1ST
#endif /* #if defined(CONFIG_FEATURE_NCMC_D121F) || defined(CONFIG_FEATURE_NCMC_G121S) || defined(CONFIG_FEATURE_NCMC_D121M) || defined(CONFIG_FEATURE_NCMC_G121T) || defined(CONFIG_FEATURE_NCMC_G121U) || defined(CONFIG_FEATURE_NCMC_KAMITSUKIGAME) || defined(CONFIG_FEATURE_NCMC_SYLPH) */
#endif /* #ifndef LOCAL_CONFIG_FEATURE_NCMC_LED_12_1ST */

#ifndef LOCAL_CONFIG_FEATURE_NCMC_ELEGANT_SLIM
#if defined(CONFIG_FEATURE_NCMC_ELEGANT_SLIM) || defined(CONFIG_FEATURE_NCMC_GEKKO) || defined(CONFIG_FEATURE_NCMC_ALEX) || defined(CONFIG_FEATURE_NCMC_RAPIDFIRE) || defined(LOCAL_CONFIG_FEATURE_NCMC_LED_12_1ST)
#define LOCAL_CONFIG_FEATURE_NCMC_ELEGANT_SLIM
#endif
#endif

#ifdef LOCAL_CONFIG_FEATURE_NCMC_ELEGANT_SLIM
#include <linux/leds-adp8861.h>
#endif /* #ifdef LOCAL_CONFIG_FEATURE_NCMC_ELEGANT_SLIM */
#ifdef CONFIG_FEATURE_NCMC_PEACOCK
#include <linux/leds-bd2812gu.h>
#endif /* #ifdef CONFIG_FEATURE_NCMC_PEACOCK */
#include <linux/leds-data_input.h>
#ifdef CONFIG_FEATURE_NCMC_PEACOCK
#include <linux/leds-bd6082gul.h>
#endif
#define LED_LIGHTS_INPUT_NAME         "led-lights-data"
#define LED_NCMLIGHTS_INPUT_NAME      "led-ncmlights-data"

/* #define LED_INPUT_NAME       "led-data-input" */

#define LED_DATA_INPUT_OK			0
#define LED_DATA_INPUT_END			1
#define LED_DATA_INPUT_FLIE_END		2
#define LED_DATA_INPUT_NG			-1
#define LED_DATA_INPUT_FLIE_NOT		-2

#define UP_FRAME_DATA_ON(X) ((( up_frame_data[X].s_data.fade.fade & 0x01 ) == 0x00 )  \
							&& (( up_frame_data[X].s_data.u_time.time == 0x12 )  \
							 || ( up_frame_data[X].s_data.u_time.time == 0x16 )  \
							 || ( up_frame_data[X].s_data.u_time.time == 0x18 )  \
							 || ( up_frame_data[X].s_data.u_time.time == 0x1C )) \
							&& (( up_frame_data[X].s_data.brightness[0] != 0 )   \
							 || ( up_frame_data[X].s_data.brightness[1] != 0 )   \
							 || ( up_frame_data[X].s_data.brightness[2] != 0 )))

#define UP_FRAME_DATA_OFF(X) ((( up_frame_data[X].s_data.fade.fade & 0x01 ) == 0x00 ) \
							&& (( up_frame_data[X].s_data.u_time.time == 0x16 )  \
							 || ( up_frame_data[X].s_data.u_time.time == 0x1C )) \
							&& ( up_frame_data[X].s_data.brightness[0] == 0 )    \
							&& ( up_frame_data[X].s_data.brightness[1] == 0 )    \
							&& ( up_frame_data[X].s_data.brightness[2] == 0 ))


enum led_lock_num{
	LED_LOCK_SET = 0,
	LED_UNLOCK_SET
};

enum led_lock_type_num{
	LED_LOCK_TYPE_INCOMING = 0,
	LED_LOCK_TYPE_FRONT,
	LED_LOCK_TYPE_BACK
};

enum led_flag_num{
	LED_FLAG_OFF = 0,
	LED_FLAG_ON,
	LED_FLAG_GET
};

typedef struct led_input_data_struct {
	unsigned char 					*p_patern;
	struct input_data_head_struct	s_data_info;
} led_input_data_type;

typedef struct led_patern_data_struct {
	struct led_input_data_struct	s_set_data;
	struct led_input_data_struct	s_ready_data;
	struct task_struct 				*ps_thread;
	struct mutex	 				data_lock;
	unsigned int					frame_cnt;
	int								active_flag;
	int								loop_flag;
	unsigned int					loop_cnt;
	int								check_flag;
//	struct mutex	 				func_lock;
} led_patern_data_type;

typedef struct led_patern_data_all_struct {
	led_patern_data_type	incoming;
	led_patern_data_type	front;
#ifdef CONFIG_FEATURE_NCMC_PEACOCK
	led_patern_data_type	back;
	led_patern_data_type	back_keep;
#endif /* #ifdef CONFIG_FEATURE_NCMC_PEACOCK */
	struct mutex	 				func_lock;
} led_patern_data_all_type;

typedef union u_input_frame_es {
	struct input_frame_es_struct	s_data;
	u8								uc[6];
} u_input_frame_es_type;

struct ncmlight_state_t{
	unsigned int	color;
	int				flashMode;
	int				flashOnMS;
	int				flashOffMS;
	int				brightnessMode;
	int				flashType;
	char			*filePath;
};

typedef struct led_state_struct{
	int					main_state;
	int					sub_state;
	int					req_flag;
	int					req_main_state;
	int					req_sub_state;
}led_state_type;
typedef struct led_data_state_struct{
	led_state_type		ilm3led;
	led_state_type		ilmkey;
#ifdef CONFIG_FEATURE_NCMC_PEACOCK
	led_state_type		ilmback;
	led_state_type		camera;
#endif /* #ifdef CONFIG_FEATURE_NCMC_PEACOCK */
}led_data_state_type;


typedef int ( *LED_ILM_FUNC_TBL)(struct ncmlight_state_t *, int );

static void led_state_set( int type, int main_state, int sub_state );
#ifdef CONFIG_FEATURE_NCMC_PEACOCK
static void led_state_set_chk( int type );
#endif /* #ifdef CONFIG_FEATURE_NCMC_PEACOCK */

static int led_func_error( struct ncmlight_state_t *state, int type );
static int led_func_ok( struct ncmlight_state_t *state, int type );
static int led_func_stop( struct ncmlight_state_t *state, int type );
static int led_func_0_0( struct ncmlight_state_t *state, int type );

#ifdef CONFIG_FEATURE_NCMC_PEACOCK
static int led_func_0_5( struct ncmlight_state_t *state, int type );
#endif /* #ifdef CONFIG_FEATURE_NCMC_PEACOCK */

static int led_func_1_1( struct ncmlight_state_t *state, int type );
static int led_func_1_2( struct ncmlight_state_t *state, int type );
#ifdef CONFIG_FEATURE_NCMC_PEACOCK
static int led_func_2_1( struct ncmlight_state_t *state, int type );
#endif /* #ifdef CONFIG_FEATURE_NCMC_PEACOCK */
// static int led_func_2_2( struct ncmlight_state_t *state, int type );

#ifdef CONFIG_FEATURE_NCMC_PEACOCK
static int led_func_2_3( struct ncmlight_state_t *state, int type );
static int led_func_2_4( struct ncmlight_state_t *state, int type );
static int led_func_2_5( struct ncmlight_state_t *state, int type );
static int led_func_3_4( struct ncmlight_state_t *state, int type );
static int led_func_3_5( struct ncmlight_state_t *state, int type );
static int led_func_4_4( struct ncmlight_state_t *state, int type );
static int led_func_4_5( struct ncmlight_state_t *state, int type );
#endif /* #ifdef CONFIG_FEATURE_NCMC_PEACOCK */

static int led_func_5_0( struct ncmlight_state_t *state, int type );
static int led_func_6_1( struct ncmlight_state_t *state, int type );
static int led_func_6_2( struct ncmlight_state_t *state, int type );
#ifdef CONFIG_FEATURE_NCMC_PEACOCK
static int led_func_9_0( struct ncmlight_state_t *state, int type );
static int led_func_9_5( struct ncmlight_state_t *state, int type );
static int led_func_a_1( struct ncmlight_state_t *state, int type );
static int led_func_a_2( struct ncmlight_state_t *state, int type );

static int led_func_a_4( struct ncmlight_state_t *state, int type );
static int led_func_a_5( struct ncmlight_state_t *state, int type );

static int led_func_b_1( struct ncmlight_state_t *state, int type );
//static int led_func_b_2( struct ncmlight_state_t *state, int type );

static int led_func_b_3( struct ncmlight_state_t *state, int type );

//static int led_func_c_1( struct ncmlight_state_t *state, int type );
//static int led_func_c_2( struct ncmlight_state_t *state, int type );

static int led_func_c_5( struct ncmlight_state_t *state, int type );
static int led_func_d_4( struct ncmlight_state_t *state, int type );
static int led_func_d_5( struct ncmlight_state_t *state, int type );
//static int led_func_e_4( struct ncmlight_state_t *state, int type );
static int led_func_e_5( struct ncmlight_state_t *state, int type );

static int led_func_camera_end( struct ncmlight_state_t *state, int type );
static int led_func_camera_0_0( struct ncmlight_state_t *state, int type );
static int led_func_camera_2_1( struct ncmlight_state_t *state, int type );
static int led_func_camera_3_2( struct ncmlight_state_t *state, int type );
static int led_func_camera_3_3( struct ncmlight_state_t *state, int type );
static int led_func_camera_stop( struct ncmlight_state_t *state, int type );

#endif /* #ifdef CONFIG_FEATURE_NCMC_PEACOCK */

static led_patern_data_all_type		gs_patern_data;

#ifdef CONFIG_FEATURE_NCMC_PEACOCK
int	g_camera_flag;
static int led_pck_time_tbl[4][8] = {
			{ 131, 520, 1050, 2100, 4190, 8390, 12600, 16800 },
			{ 131, 520, 1050, 2100, 4190, 8390, 12600, 16800 },
			{ 133, 527, 1063, 2126, 4242, 8495, 12758, 17010 },
			{ 147, 585, 1181, 2363, 4714, 9439, 14175, 18900 }
};
#endif /* #ifdef CONFIG_FEATURE_NCMC_PEACOCK */

#ifdef LOCAL_CONFIG_FEATURE_NCMC_LED_12_1ST
static led_input_data_type g_notifications_patern;
static led_input_data_type g_battery_patern;
#endif /* #ifdef LOCAL_CONFIG_FEATURE_NCMC_LED_12_1ST */

static wait_queue_head_t gs_led_wq;
static led_data_state_type gs_led_state ={
	.ilm3led = {
		.main_state = LED_STATE_STOP,
		.sub_state  = LED_STATE_STOP,
#ifdef CONFIG_FEATURE_NCMC_PEACOCK
		.req_flag       = BD6082GUL_LED_OFF,
		.req_main_state = LED_STATE_STOP,
		.req_sub_state  = LED_STATE_STOP,
#endif /* #ifdef CONFIG_FEATURE_NCMC_PEACOCK */
	},
	.ilmkey = {
		.main_state = LED_STATE_STOP,
		.sub_state  = LED_STATE_STOP,
#ifdef CONFIG_FEATURE_NCMC_PEACOCK
		.req_flag       = BD2812GU_LED_OFF,
		.req_main_state = LED_STATE_STOP,
		.req_sub_state  = LED_STATE_STOP,
#endif /* #ifdef CONFIG_FEATURE_NCMC_PEACOCK */
	},
#ifdef CONFIG_FEATURE_NCMC_PEACOCK
	.ilmback = {
		.main_state = LED_STATE_STOP,
		.sub_state  = LED_STATE_STOP,
		.req_flag       = BD2812GU_LED_OFF,
		.req_main_state = LED_STATE_STOP,
		.req_sub_state  = LED_STATE_STOP,
	},
	.camera = {
		.main_state = LED_STATE_CAM_END,
		.sub_state  = LED_STATE_STOP,
		.req_flag       = BD2812GU_LED_OFF,
		.req_main_state = LED_STATE_STOP,
		.req_sub_state  = LED_STATE_STOP,
	},
#endif /* #ifdef CONFIG_FEATURE_NCMC_PEACOCK */
};

#ifdef CONFIG_FEATURE_NCMC_PEACOCK
LED_ILM_FUNC_TBL led_ilm_func_State[15][6]={
#else /* #ifdef CONFIG_FEATURE_NCMC_PEACOCK */
LED_ILM_FUNC_TBL led_ilm_func_State[9][3]={
#endif /* #ifdef CONFIG_FEATURE_NCMC_PEACOCK */
#ifdef CONFIG_FEATURE_NCMC_PEACOCK
	{	led_func_0_0,	led_func_ok,	led_func_ok,	led_func_ok,	led_func_ok,	led_func_0_5   },	/* pertern */
	{	led_func_error,	led_func_1_1,	led_func_1_2,	led_func_error,	led_func_ok,	led_func_ok    },	/* start sync off */
	{	led_func_error,	led_func_2_1,	led_func_error,	led_func_ok,	led_func_2_4,	led_func_2_5   },	/* start sync on */
	{	led_func_ok,	led_func_stop,	led_func_stop,	led_func_stop,	led_func_3_4,	led_func_3_5   },	/* stop */
	{	led_func_ok,	led_func_ok,	led_func_stop,	led_func_ok,	led_func_4_4,	led_func_4_5   },	/* finish */

	{	led_func_5_0,	led_func_ok,	led_func_ok,	led_func_error,	led_func_error,	led_func_error },	/* pertern */
	{	led_func_error,	led_func_6_1,	led_func_6_2,	led_func_error,	led_func_error,	led_func_error },	/* start */
	{	led_func_ok,	led_func_ok,	led_func_stop,	led_func_error,	led_func_error,	led_func_error },	/* stop */
	{	led_func_ok,	led_func_ok,	led_func_stop,	led_func_error,	led_func_error,	led_func_error },	/* finish */

	{	led_func_9_0,	led_func_ok,	led_func_ok,	led_func_ok,	led_func_ok,	led_func_9_5   },	/* pertern */
	{	led_func_error,	led_func_a_1,	led_func_a_2,	led_func_error,	led_func_a_4,	led_func_a_5   },	/* start sync off */
	{	led_func_error,	led_func_b_1,	led_func_error,	led_func_ok,	led_func_ok,	led_func_ok    },	/* start sync on */
	{	led_func_error,	led_func_error,	led_func_error,	led_func_error,	led_func_ok,	led_func_c_5   },	/* start sync re on */
	{	led_func_ok,	led_func_stop,	led_func_stop,	led_func_stop,	led_func_d_4,	led_func_d_5   },	/* stop */
	{	led_func_ok,	led_func_ok,	led_func_stop,	led_func_ok,	led_func_ok,	led_func_e_5   }	/* finish */
#else /* #ifdef CONFIG_FEATURE_NCMC_PEACOCK */
	{	led_func_0_0,	led_func_ok,	led_func_ok	},		/* pertern */
	{	led_func_error,	led_func_1_1,	led_func_1_2 },		/* start sync off */
	{	led_func_error,	led_func_error,	led_func_error },	/* start sync on */
	{	led_func_stop,	led_func_ok,	led_func_stop },	/* stop */
	{	led_func_ok,	led_func_ok,	led_func_stop },	/* finish */

	{	led_func_5_0,	led_func_ok,	led_func_ok },		/* pertern */
	{	led_func_error,	led_func_6_1,	led_func_6_2 },		/* start */
	{	led_func_ok,	led_func_ok,	led_func_stop },	/* stop */
	{	led_func_ok,	led_func_ok,	led_func_stop }		/* finish */

#endif /* #ifdef CONFIG_FEATURE_NCMC_PEACOCK */
};

#ifdef CONFIG_FEATURE_NCMC_PEACOCK
LED_ILM_FUNC_TBL led_ilm_camera_func_State[6][4]={
	{	led_func_camera_0_0,	led_func_ok,			led_func_ok,			led_func_ok			},	/* NCM_CAM_CTL_START */
	{	led_func_ok,			led_func_camera_end,	led_func_camera_end,	led_func_camera_end	},	/* NCM_CAM_CTL_END */
	{	led_func_error,			led_func_camera_2_1,	led_func_ok,			led_func_ok			},	/* pertern */
	{	led_func_error,			led_func_error,			led_func_camera_3_2,	led_func_camera_3_3	},	/* NCM_ILM_CAM_ON */
	{	led_func_ok,			led_func_camera_stop,	led_func_camera_stop,	led_func_camera_stop },	/* NCM_ILM_CAM_OFF */
	{	led_func_ok,			led_func_ok,			led_func_ok,			led_func_camera_stop }	/* finish */
};

int led_back_keep_State[5][6]={
	{	LED_STATE_PATERN,	LED_STATE_OK,				LED_STATE_OK,		LED_STATE_OK,		LED_STATE_OK,				LED_STATE_OK				},	/* pertern */
	{	LED_STATE_ERR,		LED_STATE_LOCAL_OPE,		LED_STATE_LOCAL_OPE,LED_STATE_ERR,		LED_STATE_SYNC_OPE_CANCEL,	LED_STATE_SYNC_OPE_CANCEL	},	/* start sync off */
	{	LED_STATE_ERR,		LED_STATE_SYNC_OPE_WAIT,	LED_STATE_ERR,		LED_STATE_OK,		LED_STATE_OK,				LED_STATE_OK				},	/* start sync on */
	{	LED_STATE_ERR,		LED_STATE_ERR,				LED_STATE_ERR,		LED_STATE_ERR,		LED_STATE_OK,				LED_STATE_SYNC_OPE			},	/* start sync re on */
	{	LED_STATE_STOP,		LED_STATE_STOP,				LED_STATE_STOP,		LED_STATE_STOP,		LED_STATE_STOP,				LED_STATE_SYNC_OPE_CANCEL	}	/* stop */
};

#endif /* #ifdef CONFIG_FEATURE_NCMC_PEACOCK */

static struct wake_lock led_ilm_wake_lock;

/* debug */
//#define LED_DIAG_DEBUG_PLUS
/* debug */

/*----------------------------------------------------------------------------
* MODULE   : led_lock_wrapper
* FUNCTTION: 
* NOTE     : None
* RETURN   : 
* CREATE   : 
* UPDATE   : 
----------------------------------------------------------------------------*/
static void led_lock_wrapper( int lock, int locok_type )
{
	return;
}

/*----------------------------------------------------------------------------
* MODULE   : led_patern_data_switch
* FUNCTTION: 
* NOTE     : None
* RETURN   : 
* CREATE   : 
* UPDATE   : 
----------------------------------------------------------------------------*/
static u8 *led_patern_data_switch( int type )
{
	unsigned char *p_ret = NULL;
	

    switch(type){
	
    case LED_TYPE_INCOMING:
    	
		led_lock_wrapper( LED_LOCK_SET, LED_LOCK_TYPE_INCOMING );
		
		if( gs_patern_data.incoming.s_ready_data.p_patern != NULL ){
			kfree( gs_patern_data.incoming.s_set_data.p_patern );
			gs_patern_data.incoming.s_set_data.p_patern = gs_patern_data.incoming.s_ready_data.p_patern;
			gs_patern_data.incoming.s_ready_data.p_patern = NULL;
			
			memcpy( &gs_patern_data.incoming.s_set_data.s_data_info, &gs_patern_data.incoming.s_ready_data.s_data_info, sizeof(input_data_head_type) );
		}
		
		p_ret = gs_patern_data.incoming.s_set_data.p_patern;
		
		gs_patern_data.incoming.frame_cnt = 0;
		
		led_lock_wrapper( LED_UNLOCK_SET, LED_LOCK_TYPE_INCOMING );
    	
		break;
		
    case LED_TYPE_FRONT:
    	
		led_lock_wrapper( LED_LOCK_SET, LED_LOCK_TYPE_FRONT );
		
		if( gs_patern_data.front.s_ready_data.p_patern != NULL ){
			kfree( gs_patern_data.front.s_set_data.p_patern );
			gs_patern_data.front.s_set_data.p_patern = gs_patern_data.front.s_ready_data.p_patern;
			gs_patern_data.front.s_ready_data.p_patern = NULL;
			
			memcpy( &gs_patern_data.front.s_set_data.s_data_info, &gs_patern_data.front.s_ready_data.s_data_info, sizeof(input_data_head_type) );
		}
		
		p_ret = gs_patern_data.front.s_set_data.p_patern;
		
		gs_patern_data.front.frame_cnt = 0;
		
		led_lock_wrapper( LED_UNLOCK_SET, LED_LOCK_TYPE_FRONT );
    	
		break;
#ifdef CONFIG_FEATURE_NCMC_PEACOCK
    case LED_TYPE_BACK:
    	
		led_lock_wrapper( LED_LOCK_SET, LED_LOCK_TYPE_BACK );
		
		if( gs_patern_data.back.s_ready_data.p_patern != NULL ){
			kfree( gs_patern_data.back.s_set_data.p_patern );
			gs_patern_data.back.s_set_data.p_patern = gs_patern_data.back.s_ready_data.p_patern;
			gs_patern_data.back.s_ready_data.p_patern = NULL;
			
			memcpy( &gs_patern_data.back.s_set_data.s_data_info, &gs_patern_data.back.s_ready_data.s_data_info, sizeof(input_data_head_type) );
		}
		p_ret = gs_patern_data.back.s_set_data.p_patern;
		gs_patern_data.back.frame_cnt = 0;
		
		led_lock_wrapper( LED_UNLOCK_SET, LED_LOCK_TYPE_BACK );
    	
		break;
		
    case LED_TYPE_CAMERA:
    	
		led_lock_wrapper( LED_LOCK_SET, LED_TYPE_CAMERA );
		
		if( gs_patern_data.back_keep.s_ready_data.p_patern != NULL ){
			kfree( gs_patern_data.back_keep.s_set_data.p_patern );
			gs_patern_data.back_keep.s_set_data.p_patern = gs_patern_data.back_keep.s_ready_data.p_patern;
			gs_patern_data.back_keep.s_ready_data.p_patern = NULL;
			
			memcpy( &gs_patern_data.back_keep.s_set_data.s_data_info, &gs_patern_data.back_keep.s_ready_data.s_data_info, sizeof(input_data_head_type) );
		}
		p_ret = gs_patern_data.back_keep.s_set_data.p_patern;
		gs_patern_data.back_keep.frame_cnt = 0;
		
		led_lock_wrapper( LED_UNLOCK_SET, LED_TYPE_CAMERA );
    	
		break;
		
#endif /* #ifdef CONFIG_FEATURE_NCMC_PEACOCK */
	
	default:
		printk(KERN_INFO "[leds_data_input]led_patern_data_switch NG\n");
		break;
	}
	

	return p_ret;
}

#ifdef CONFIG_FEATURE_NCMC_PEACOCK
/*----------------------------------------------------------------------------
* MODULE   : led_patern_data_check
* FUNCTTION: 
* NOTE     : None
* RETURN   : 
* CREATE   : 
* UPDATE   : 
----------------------------------------------------------------------------*/
static u8 *led_patern_data_check( int type )
{
	unsigned char *p_ret = NULL;
    switch(type){
	  case LED_TYPE_INCOMING:
		led_lock_wrapper( LED_LOCK_SET, LED_LOCK_TYPE_INCOMING );
		p_ret = gs_patern_data.incoming.s_ready_data.p_patern;
		led_lock_wrapper( LED_UNLOCK_SET, LED_LOCK_TYPE_INCOMING );
		break;
		
	  case LED_TYPE_FRONT:
		led_lock_wrapper( LED_LOCK_SET, LED_LOCK_TYPE_FRONT );
		p_ret = gs_patern_data.front.s_ready_data.p_patern;
		led_lock_wrapper( LED_UNLOCK_SET, LED_LOCK_TYPE_FRONT );
		break;
#ifdef CONFIG_FEATURE_NCMC_PEACOCK
	  case LED_TYPE_BACK:
		led_lock_wrapper( LED_LOCK_SET, LED_LOCK_TYPE_BACK );
		p_ret = gs_patern_data.back.s_ready_data.p_patern;
		led_lock_wrapper( LED_UNLOCK_SET, LED_LOCK_TYPE_BACK );
		break;
#endif /* #ifdef CONFIG_FEATURE_NCMC_PEACOCK */
	  default:
		printk(KERN_INFO "[leds_data_input]led_patern_data_check NG\n");
		break;
	}
	return p_ret;
}
#endif /* #ifdef CONFIG_FEATURE_NCMC_PEACOCK */

/*----------------------------------------------------------------------------
* MODULE   : led_patern_data_del
* FUNCTTION: 
* NOTE     : None
* RETURN   : 
* CREATE   : 
* UPDATE   : 
----------------------------------------------------------------------------*/
static int led_patern_data_del( int type )
{
	int ret = LED_DATA_INPUT_OK;
	

    switch(type){
	
    case LED_TYPE_INCOMING:
    	
		led_lock_wrapper( LED_LOCK_SET, LED_LOCK_TYPE_INCOMING );
		
		if( gs_patern_data.incoming.s_set_data.p_patern != NULL ){
			kfree( gs_patern_data.incoming.s_set_data.p_patern );
			gs_patern_data.incoming.s_set_data.p_patern = NULL;
		}
		gs_patern_data.incoming.active_flag = LED_FLAG_OFF;
		
		led_lock_wrapper( LED_UNLOCK_SET, LED_LOCK_TYPE_INCOMING );
    	
		break;
		
    case LED_TYPE_FRONT:
    	
		led_lock_wrapper( LED_LOCK_SET, LED_LOCK_TYPE_FRONT );
		
		if( gs_patern_data.front.s_set_data.p_patern != NULL ){
			kfree( gs_patern_data.front.s_set_data.p_patern );
			gs_patern_data.front.s_set_data.p_patern = NULL;
		}
		gs_patern_data.front.active_flag = LED_FLAG_OFF;
		
		led_lock_wrapper( LED_UNLOCK_SET, LED_LOCK_TYPE_FRONT );
    	
		break;
#ifdef CONFIG_FEATURE_NCMC_PEACOCK
    case LED_TYPE_BACK:
    	
		led_lock_wrapper( LED_LOCK_SET, LED_LOCK_TYPE_BACK );
		
		if( gs_patern_data.back.s_set_data.p_patern != NULL ){
			kfree( gs_patern_data.back.s_set_data.p_patern );
			gs_patern_data.back.s_set_data.p_patern = NULL;
		}
		gs_patern_data.back.active_flag = LED_FLAG_OFF;
		
		led_lock_wrapper( LED_UNLOCK_SET, LED_LOCK_TYPE_BACK );
    	
		break;
		
    case LED_TYPE_CAMERA:
    	
		led_lock_wrapper( LED_LOCK_SET, LED_TYPE_CAMERA );
		
		if( gs_patern_data.back_keep.s_set_data.p_patern != NULL ){
			kfree( gs_patern_data.back_keep.s_set_data.p_patern );
			gs_patern_data.back_keep.s_set_data.p_patern = NULL;
		}
		gs_patern_data.back_keep.active_flag = LED_FLAG_OFF;
		
		led_lock_wrapper( LED_UNLOCK_SET, LED_TYPE_CAMERA );
    	
		break;
#endif /* #ifdef CONFIG_FEATURE_NCMC_PEACOCK */
	
	default:
		ret = LED_DATA_INPUT_NG;
		printk(KERN_INFO "[leds_data_input]led_patern_data_del NG\n");
		break;
	}
	return ret;
}

/*----------------------------------------------------------------------------
* MODULE   : led_active_flag_ctr
* FUNCTTION: 
* NOTE     : None
* RETURN   : 
* CREATE   : 
* UPDATE   : 
----------------------------------------------------------------------------*/
static int led_active_flag_ctr( int ctr, int type )
{
	int ret = LED_DATA_INPUT_NG;
	

    switch(type){
	
    case LED_TYPE_INCOMING:
    	
		led_lock_wrapper( LED_LOCK_SET, LED_LOCK_TYPE_INCOMING );
		
		if( ctr == LED_FLAG_OFF || ctr == LED_FLAG_ON ){
			gs_patern_data.incoming.active_flag = ctr;
		}
		ret = gs_patern_data.incoming.active_flag;
		
		led_lock_wrapper( LED_UNLOCK_SET, LED_LOCK_TYPE_INCOMING );
    	
		break;
		
    case LED_TYPE_FRONT:
    	
		led_lock_wrapper( LED_LOCK_SET, LED_LOCK_TYPE_FRONT );
		
		if( ctr == LED_FLAG_OFF || ctr == LED_FLAG_ON ){
			gs_patern_data.front.active_flag = ctr;
		}
		ret = gs_patern_data.front.active_flag;
		
		led_lock_wrapper( LED_UNLOCK_SET, LED_LOCK_TYPE_FRONT );
    	
		break;
#ifdef CONFIG_FEATURE_NCMC_PEACOCK
    case LED_TYPE_BACK:
    	
		led_lock_wrapper( LED_LOCK_SET, LED_LOCK_TYPE_BACK );
		
		if( ctr == LED_FLAG_OFF || ctr == LED_FLAG_ON ){
			gs_patern_data.back.active_flag = ctr;
		}
		ret = gs_patern_data.back.active_flag;
		
		led_lock_wrapper( LED_UNLOCK_SET, LED_LOCK_TYPE_BACK );
    	
		break;
#endif /* #ifdef CONFIG_FEATURE_NCMC_PEACOCK */
	
	default:
		printk(KERN_INFO "[leds_data_input]led_active_flag_ctr NG\n");
		break;
	}
	

	return ret;
}

/*----------------------------------------------------------------------------
* MODULE   : led_loop_set
* FUNCTTION: 
* NOTE     : None
* RETURN   : 
* CREATE   : 
* UPDATE   : 
----------------------------------------------------------------------------*/
static int led_loop_set( unsigned int ctr, int type )
{
	int ret = LED_DATA_INPUT_OK;
	int				loop_flag;
	unsigned int	loop_cnt = 0;
	
	if( ctr == 0 ){
		loop_flag = LED_FLAG_ON;
	}
	else{
		loop_flag = LED_FLAG_OFF;
		loop_cnt = ctr;
	}
	
	printk(KERN_INFO "[leds_data_input]led_loop_set ctr = %d, type = %d\n", ctr, type);
	
    switch(type){
	
    case LED_TYPE_INCOMING:
    	
		led_lock_wrapper( LED_LOCK_SET, LED_LOCK_TYPE_INCOMING );
		
		gs_patern_data.incoming.loop_flag = loop_flag;
		gs_patern_data.incoming.loop_cnt = loop_cnt;
		gs_patern_data.incoming.frame_cnt = 0;
		
		led_lock_wrapper( LED_UNLOCK_SET, LED_LOCK_TYPE_INCOMING );
    	
		break;
		
    case LED_TYPE_FRONT:
    	
		led_lock_wrapper( LED_LOCK_SET, LED_LOCK_TYPE_FRONT );
		
		gs_patern_data.front.loop_flag = loop_flag;
		gs_patern_data.front.loop_cnt = loop_cnt;
		gs_patern_data.front.frame_cnt = 0;
		
		led_lock_wrapper( LED_UNLOCK_SET, LED_LOCK_TYPE_FRONT );
    	
		break;
#ifdef CONFIG_FEATURE_NCMC_PEACOCK
    case LED_TYPE_BACK:
    	
		led_lock_wrapper( LED_LOCK_SET, LED_LOCK_TYPE_BACK );
		
		gs_patern_data.back.loop_flag = loop_flag;
		gs_patern_data.back.loop_cnt = loop_cnt;
		gs_patern_data.back.frame_cnt = 0;
		
		led_lock_wrapper( LED_UNLOCK_SET, LED_LOCK_TYPE_BACK );
    	
		break;
		
    case LED_TYPE_CAMERA:
    	
		led_lock_wrapper( LED_LOCK_SET, LED_TYPE_CAMERA );
		
		gs_patern_data.back_keep.loop_flag = loop_flag;
		gs_patern_data.back_keep.loop_cnt = loop_cnt;
		gs_patern_data.back_keep.frame_cnt = 0;
		
		led_lock_wrapper( LED_UNLOCK_SET, LED_TYPE_CAMERA );
    	
		break;
#endif /* #ifdef CONFIG_FEATURE_NCMC_PEACOCK */
	
	default:
		printk(KERN_INFO "[leds_data_input]led_loop_set NG\n");
		ret = LED_DATA_INPUT_NG;
		break;
	}
	return ret;
}

/*----------------------------------------------------------------------------
* MODULE   : led_wake_up_thread
* FUNCTTION: 
* NOTE     : None
* RETURN   : 
* CREATE   : 
* UPDATE   : 
----------------------------------------------------------------------------*/
static void led_wake_up_thread( int type )
{
	printk(KERN_INFO "[leds_data_input]led_wake_up_thread IN type = %d\n", type);
	
    switch(type){
	
    case LED_TYPE_INCOMING:
    
		gs_patern_data.incoming.check_flag++;
		wake_up_interruptible(&gs_led_wq);
    	
		break;
		
    case LED_TYPE_FRONT:
    	
		gs_patern_data.front.check_flag++;
		wake_up_interruptible(&gs_led_wq);
    	
		break;
#ifdef CONFIG_FEATURE_NCMC_PEACOCK
    case LED_TYPE_BACK:
    	
		gs_patern_data.back.check_flag++;
		wake_up_interruptible(&gs_led_wq);
    	
		break;
#endif /* #ifdef CONFIG_FEATURE_NCMC_PEACOCK */
	
	default:
		printk(KERN_INFO "[leds_data_input]led_wake_up_thread NG\n");
		break;
	}
	
	
	return ;
}

/*----------------------------------------------------------------------------
* MODULE   : led_get_frame_data
* FUNCTTION: 
* NOTE     : None
* RETURN   : 
* CREATE   : 
* UPDATE   : 
----------------------------------------------------------------------------*/
static int led_get_frame_data( void *p_set, int type )
{
	int ret = LED_DATA_INPUT_NG;
	unsigned char *p_shift;
	unsigned int shift_size;
	unsigned int frame_size;
	
	if( p_set == NULL ){
		printk(KERN_INFO "[leds_data_input]led_get_frame_data input NG\n");
		return ret;
	}
	
	
    switch(type){
	
    case LED_TYPE_INCOMING:
    	
		led_lock_wrapper( LED_LOCK_SET, LED_LOCK_TYPE_INCOMING );
		
		if( gs_patern_data.incoming.s_set_data.p_patern != NULL ){
			frame_size = gs_patern_data.incoming.s_set_data.s_data_info.frame_size;
			shift_size = gs_patern_data.incoming.frame_cnt * frame_size;
			
			if( gs_patern_data.incoming.s_set_data.s_data_info.frame_num  > gs_patern_data.incoming.frame_cnt ){
				p_shift = gs_patern_data.incoming.s_set_data.p_patern + shift_size  ;
				memcpy( p_set, p_shift, frame_size );
				gs_patern_data.incoming.frame_cnt++;
				
				
				if( gs_patern_data.incoming.s_set_data.s_data_info.frame_num == gs_patern_data.incoming.frame_cnt ){
					if( gs_patern_data.incoming.loop_flag == LED_FLAG_ON ){
						gs_patern_data.incoming.frame_cnt = 0;
					}
					else{
						gs_patern_data.incoming.loop_cnt--;
						gs_patern_data.incoming.frame_cnt = 0;
					}
				}
				
				if( (gs_patern_data.incoming.loop_flag != LED_FLAG_ON) && (gs_patern_data.incoming.loop_cnt == 0) ){
					ret = LED_DATA_INPUT_FLIE_END;
				}
				else {
					ret = LED_DATA_INPUT_OK;
				}
			}
		}
		else{
			ret = LED_DATA_INPUT_FLIE_NOT;
		}
		
		led_lock_wrapper( LED_UNLOCK_SET, LED_LOCK_TYPE_INCOMING );
    	
		break;
		
    case LED_TYPE_FRONT:
    	
		led_lock_wrapper( LED_LOCK_SET, LED_LOCK_TYPE_FRONT );
		
		if( gs_patern_data.front.s_set_data.p_patern != NULL ){
			frame_size = gs_patern_data.front.s_set_data.s_data_info.frame_size;
			shift_size = gs_patern_data.front.frame_cnt * frame_size;
			
			if( gs_patern_data.front.s_set_data.s_data_info.frame_num  > gs_patern_data.front.frame_cnt ){
				p_shift = gs_patern_data.front.s_set_data.p_patern + shift_size ;
				memcpy( p_set, p_shift, frame_size );
				gs_patern_data.front.frame_cnt++;
				
				
				if( gs_patern_data.front.s_set_data.s_data_info.frame_num == gs_patern_data.front.frame_cnt ){
					if( gs_patern_data.front.loop_flag == LED_FLAG_ON ){
						gs_patern_data.front.frame_cnt = 0;
					}
					else{
						gs_patern_data.front.loop_cnt--;
						gs_patern_data.front.frame_cnt = 0;
					}
				}
				
				if( (gs_patern_data.front.loop_flag != LED_FLAG_ON) && (gs_patern_data.front.loop_cnt == 0) ){
					ret = LED_DATA_INPUT_FLIE_END;
				}
				else {
					ret = LED_DATA_INPUT_OK;
				}
			}
		}
		else{
			ret = LED_DATA_INPUT_FLIE_NOT;
		}
		
		led_lock_wrapper( LED_UNLOCK_SET, LED_LOCK_TYPE_FRONT );
    	
		break;
#ifdef CONFIG_FEATURE_NCMC_PEACOCK
    case LED_TYPE_BACK:
    	
		led_lock_wrapper( LED_LOCK_SET, LED_LOCK_TYPE_BACK );
		
		if( gs_patern_data.back.s_set_data.p_patern != NULL ){
			frame_size = gs_patern_data.back.s_set_data.s_data_info.frame_size;
			shift_size = gs_patern_data.back.frame_cnt * frame_size;
			
			if( gs_patern_data.back.s_set_data.s_data_info.frame_num  > gs_patern_data.back.frame_cnt ){
				p_shift = gs_patern_data.back.s_set_data.p_patern + shift_size ;
				memcpy( p_set, p_shift, frame_size );
				gs_patern_data.back.frame_cnt++;
				
				
				if( gs_patern_data.back.s_set_data.s_data_info.frame_num == gs_patern_data.back.frame_cnt ){
					if( gs_patern_data.back.loop_flag == LED_FLAG_ON ){
						gs_patern_data.back.frame_cnt = 0;
					}
					else{
						gs_patern_data.back.loop_cnt--;
						gs_patern_data.back.frame_cnt = 0;
					}
				}
				
				if( (gs_patern_data.back.loop_flag != LED_FLAG_ON) && (gs_patern_data.back.loop_cnt == 0) ){
					ret = LED_DATA_INPUT_FLIE_END;
				}
				else {
					ret = LED_DATA_INPUT_OK;
				}
			}
		}
		else{
			ret = LED_DATA_INPUT_FLIE_NOT;
		}
		
		led_lock_wrapper( LED_UNLOCK_SET, LED_LOCK_TYPE_BACK );
    	
		break;
#endif /* #ifdef CONFIG_FEATURE_NCMC_PEACOCK */
	
	default:
		printk(KERN_INFO "[leds_data_input]led_get_frame_data NG\n");
		break;
	}
	
	
	return ret;
}

#ifdef LOCAL_CONFIG_FEATURE_NCMC_ELEGANT_SLIM
/*----------------------------------------------------------------------------
* MODULE   : led_adp8861_3led_ctrl_reg_time
* FUNCTTION: 
* NOTE     : None
* RETURN   : 
* CREATE   : 
* UPDATE   : 
----------------------------------------------------------------------------*/
static int led_adp8861_3led_ctrl_reg_time( union u_led_isct1_reg  *led_timer1_data,
										   union u_led_isct2_reg  *led_timer2_data)
{
	int						ret, ret_code = ADP8861_LED_SET_OK;
	union u_led_isct1_reg	led_timer1_set;
	union u_led_isct2_reg	led_timer2_set;
	

	memset( &led_timer2_set,  0, sizeof( led_timer2_set ) );
	led_timer2_set.st2.sc1_off = ADP8861_LED_ON;
	led_timer2_set.st2.sc2_off = ADP8861_LED_ON;
	led_timer2_set.st2.sc3_off = ADP8861_LED_ON;
	ret_code = ret = adp8861_led_timer_set2( &led_timer2_set, led_timer2_data );
	if( ret != ADP8861_LED_SET_OK ){
		printk(KERN_INFO "led_adp8861_3led_ctrl_reg_time: adp8861_led_timer_set2 = %d \n",ret);
	}
	memset( &led_timer1_set,  0, sizeof( led_timer1_set ) );
	led_timer1_set.st2.scon = ADP8861_LED_ON;
	ret_code |= ret = adp8861_led_timer_set1( &led_timer1_set, led_timer1_data );
	if( ret != ADP8861_LED_SET_OK ){
		printk(KERN_INFO "led_adp8861_3led_ctrl_reg_time: adp8861_led_timer_set1 = %d \n",ret);
	}
	
	return ret_code;
}

/*----------------------------------------------------------------------------
* MODULE   : led_adp8861_3led_ctrl_reg
* FUNCTTION: 
* NOTE     : None
* RETURN   : 
* CREATE   : 
* UPDATE   : 
----------------------------------------------------------------------------*/
static int led_adp8861_3led_ctrl_reg( unsigned char timer_set_flg,
									   union u_led_isct1_reg  *led_timer1_data,
									   union u_led_isct2_reg  *led_timer2_data,
									   u_input_frame_es_type *u_frame_data )
{
	int						ret, ret_code = ADP8861_LED_SET_OK;
	union u_led_isc_reg		set_brigh[3];
	struct led_request_rgb	set_onoff;


	memset( set_brigh,  0, sizeof( set_brigh ) );
	memset( &set_onoff, 0, sizeof( set_onoff ) );
	
	set_brigh[0].st2.set_flag = ADP8861_LED_ON;
	set_brigh[0].st2.scd	  = u_frame_data->s_data.brightness[0];
	if( u_frame_data->s_data.brightness[0] != 0 ){
		set_onoff.set_r = ADP8861_LED_ON;
	}
	set_brigh[1].st2.set_flag = ADP8861_LED_ON;
	set_brigh[1].st2.scd	  = u_frame_data->s_data.brightness[1];
	if( u_frame_data->s_data.brightness[1] != 0 ){
	set_onoff.set_g =  ADP8861_LED_ON;
	}
	set_brigh[2].st2.set_flag = ADP8861_LED_ON;
	set_brigh[2].st2.scd	  = u_frame_data->s_data.brightness[2];
	if( u_frame_data->s_data.brightness[2] != 0 ){
	set_onoff.set_b =  ADP8861_LED_ON;
	}
	ret_code |= ret = adp8861_rgb_led_bright( &set_brigh[0], &set_brigh[1], &set_brigh[2] );
	if( ret != ADP8861_LED_SET_OK ){
		printk(KERN_INFO "led_adp8861_3led_ctrl: adp8861_rgb_led_bright = %d \n",ret);
	}
	if( timer_set_flg == ADP8861_LED_ON ){
		ret = led_adp8861_3led_ctrl_reg_time( led_timer1_data, led_timer2_data);
	}
	ret_code |= ret = adp8861_rgb_led_set( &set_onoff );
	if( ret != ADP8861_LED_SET_OK ){
		printk(KERN_INFO "led_adp8861_3led_ctrl: adp8861_rgb_led_set(%d,%d,%d) = %d \n",set_onoff.set_r,set_onoff.set_g,set_onoff.set_b,ret);
	}


	return ret_code;
}

/*----------------------------------------------------------------------------
* MODULE   : led_adp8861_3led_ctrl_chk
* FUNCTTION: 
* NOTE     : None
* RETURN   : 
* CREATE   : 
* UPDATE   : 
----------------------------------------------------------------------------*/
int led_adp8861_3led_ctrl_chk( void )
{
	int						ret = 0;
	union u_led_isct1_reg  led_timer1_data;
	union u_led_isct2_reg  led_timer2_data;
	u_input_frame_es_type *up_frame_data;
	

	memset( &led_timer1_data,  0, sizeof( led_timer1_data ) );
	memset( &led_timer2_data,  0, sizeof( led_timer2_data ) );

	if( gs_patern_data.incoming.loop_flag == LED_FLAG_ON ){
		
		up_frame_data = (u_input_frame_es_type *)gs_patern_data.incoming.s_set_data.p_patern ;
		
		if( gs_patern_data.incoming.s_set_data.s_data_info.frame_num == 2 ){
			
			if (( up_frame_data[0].s_data.fade.hw_act == LED_FLAG_ON )
			 && ( up_frame_data[1].s_data.fade.hw_act == LED_FLAG_ON )){
				
				if (( up_frame_data[0].s_data.fade.hw_on_off == LED_FLAG_ON  )
				 && ( up_frame_data[1].s_data.fade.hw_on_off == LED_FLAG_OFF )){
					
					led_timer1_data.st2.scon    = up_frame_data[0].s_data.fade.hw_set_data;
					led_timer2_data.st2.sc1_off = up_frame_data[1].s_data.fade.hw_set_data;
					led_timer2_data.st2.sc2_off = up_frame_data[1].s_data.fade.hw_set_data;
					led_timer2_data.st2.sc3_off = up_frame_data[1].s_data.fade.hw_set_data;
					
					led_adp8861_3led_ctrl_reg( ADP8861_LED_ON, &led_timer1_data, &led_timer2_data, &up_frame_data[0] );
					
					led_active_flag_ctr( LED_FLAG_OFF, LED_TYPE_INCOMING );
					
					printk(KERN_INFO "[leds_data_input]led_adp8861_3led_ctrl_chk HRD1 \n");
					ret = 1;
				}
				else if (( up_frame_data[0].s_data.fade.hw_on_off == LED_FLAG_OFF )
					  && ( up_frame_data[1].s_data.fade.hw_on_off == LED_FLAG_ON  )){
					
					led_timer1_data.st2.scon    = up_frame_data[1].s_data.fade.hw_set_data;
					led_timer2_data.st2.sc1_off = up_frame_data[0].s_data.fade.hw_set_data;
					led_timer2_data.st2.sc2_off = up_frame_data[0].s_data.fade.hw_set_data;
					led_timer2_data.st2.sc3_off = up_frame_data[0].s_data.fade.hw_set_data;
					
					led_adp8861_3led_ctrl_reg( ADP8861_LED_ON, &led_timer1_data, &led_timer2_data, &up_frame_data[1] );
					
					led_active_flag_ctr( LED_FLAG_OFF, LED_TYPE_INCOMING );
					
					printk(KERN_INFO "[leds_data_input]led_adp8861_3led_ctrl_chk HRD2 \n");
					ret = 1;
				}
			}
			else 
			if(UP_FRAME_DATA_ON(0)){
				if( UP_FRAME_DATA_OFF(1) ){
					if( up_frame_data[0].s_data.u_time.time == 0x12 ){
						led_timer2_data.st2.sc1_off = 0;
						led_timer2_data.st2.sc2_off = 0;
						led_timer2_data.st2.sc3_off = 0;
					}
					if( up_frame_data[0].s_data.u_time.time == 0x16 ){
						led_timer2_data.st2.sc1_off = 1;
						led_timer2_data.st2.sc2_off = 1;
						led_timer2_data.st2.sc3_off = 1;
					}
					if( up_frame_data[0].s_data.u_time.time == 0x18 ){
						led_timer2_data.st2.sc1_off = 2;
						led_timer2_data.st2.sc2_off = 2;
						led_timer2_data.st2.sc3_off = 2;
					}
					if( up_frame_data[0].s_data.u_time.time == 0x1C ){
						led_timer2_data.st2.sc1_off = 3;
						led_timer2_data.st2.sc2_off = 3;
						led_timer2_data.st2.sc3_off = 3;
					}
					if( up_frame_data[1].s_data.u_time.time == 0x16 ){
						led_timer1_data.st2.scon = 1;
					}
					if( up_frame_data[1].s_data.u_time.time == 0x1C ){
						led_timer1_data.st2.scon = 2;
					}
					led_adp8861_3led_ctrl_reg( ADP8861_LED_ON, &led_timer1_data, &led_timer2_data, &up_frame_data[0] );
					
					led_active_flag_ctr( LED_FLAG_OFF, LED_TYPE_INCOMING );
					
					printk(KERN_INFO "[leds_data_input]led_adp8861_3led_ctrl_chk HRD1 \n");
					ret = 1;
				}
			}
			else if(UP_FRAME_DATA_ON(1)){
				if( UP_FRAME_DATA_OFF(0) ){
					if( up_frame_data[1].s_data.u_time.time == 0x12 ){
						led_timer2_data.st2.sc1_off = 0;
						led_timer2_data.st2.sc2_off = 0;
						led_timer2_data.st2.sc3_off = 0;
					}
					if( up_frame_data[1].s_data.u_time.time == 0x16 ){
						led_timer2_data.st2.sc1_off = 1;
						led_timer2_data.st2.sc2_off = 1;
						led_timer2_data.st2.sc3_off = 1;
					}
					if( up_frame_data[1].s_data.u_time.time == 0x18 ){
						led_timer2_data.st2.sc1_off = 2;
						led_timer2_data.st2.sc2_off = 2;
						led_timer2_data.st2.sc3_off = 2;
					}
					if( up_frame_data[1].s_data.u_time.time == 0x1C ){
						led_timer2_data.st2.sc1_off = 3;
						led_timer2_data.st2.sc2_off = 3;
						led_timer2_data.st2.sc3_off = 3;
					}
					if( up_frame_data[0].s_data.u_time.time == 0x16 ){
						led_timer1_data.st2.scon = 1;
					}
					if( up_frame_data[0].s_data.u_time.time == 0x1C ){
						led_timer1_data.st2.scon = 2;
					}
					led_adp8861_3led_ctrl_reg( ADP8861_LED_ON, &led_timer1_data, &led_timer2_data, &up_frame_data[1] );
					
					led_active_flag_ctr( LED_FLAG_OFF, LED_TYPE_INCOMING );
					
					printk(KERN_INFO "[leds_data_input]led_adp8861_3led_ctrl_chk HRD2 \n");
					ret = 1;
				}
			}
		}
	}
	led_adp8861_3led_ctrl_reg_time( &led_timer1_data, &led_timer2_data );
	

	return ret;
}


/*----------------------------------------------------------------------------
* MODULE   : led_adp8861_3led_ctrl
* FUNCTTION: 
* NOTE     : None
* RETURN   : 
* CREATE   : 
* UPDATE   : 
----------------------------------------------------------------------------*/
static void led_adp8861_3led_ctrl( void )
{
	u_input_frame_es_type u_frame_data;
	int get_frame, time_a = 0, time_b = 0, time = 0;
	int wait_ret, ret, count;
	int active_flag;
	union u_led_iscf_reg	led_fade;
	struct ncmlight_state_t state;
	union u_led_isct1_reg  led_timer1_data;
	union u_led_isct2_reg  led_timer2_data;
	
	printk(KERN_INFO "[leds_data_input]led_adp8861_3led_ctrl IN\n");
	
	memset( &led_timer1_data,  0, sizeof( led_timer1_data ) );
	memset( &led_timer2_data,  0, sizeof( led_timer2_data ) );
	
	while(1){
		
		mutex_lock( &gs_patern_data.func_lock );
		active_flag = led_active_flag_ctr( LED_FLAG_GET, LED_TYPE_INCOMING );
		
		if( active_flag == LED_FLAG_OFF ){
			memset( &state, 0, sizeof( state ) );
			ret = led_ilm_func_State[4][gs_led_state.ilm3led.main_state](&state, LED_TYPE_INCOMING);
			if( ret != LED_DATA_INPUT_OK ){
				printk(KERN_INFO "led_adp8861_3led_ctrl: led_ilm_func_State(4,%d) = %d \n",gs_led_state.ilm3led.main_state, ret);
			}
			gs_patern_data.incoming.ps_thread = NULL;
			mutex_unlock( &gs_patern_data.func_lock );
			break;
		}
		
		get_frame = led_get_frame_data( u_frame_data.uc, LED_TYPE_INCOMING );
		
		if( get_frame == LED_DATA_INPUT_OK || get_frame == LED_DATA_INPUT_FLIE_END ){
			
			if( gs_patern_data.incoming.frame_cnt == 0 ){
				if( led_adp8861_3led_ctrl_chk() != 0 ){
					gs_patern_data.incoming.ps_thread = NULL;
					mutex_unlock( &gs_patern_data.func_lock );
					
					printk(KERN_INFO "[leds_data_input]led_adp8861_3led_ctrl hw end\n");
					break;
				}
				else{
					led_adp8861_3led_ctrl_reg_time( &led_timer1_data, &led_timer2_data);
				}
			}
			if(( u_frame_data.s_data.fade.fade & 0x01 ) == 0x01 ){
				led_fade.uc = u_frame_data.s_data.u_time.time ;
				ret = adp8861_led_fade_set( &led_fade );
				time = 0;
			}
			else{
				if( 0 != (int)u_frame_data.s_data.u_time.time ){
					count = (int)u_frame_data.s_data.u_time.s_tmie.time_a;
					time_a = 1;
					while( count > 0){
						time_a *= 10;
						count-- ;
					}
					time_b = (int)u_frame_data.s_data.u_time.s_tmie.time_b;
					time = time_a * time_b * 10;
				}
				
				led_adp8861_3led_ctrl_reg( ADP8861_LED_OFF, &led_timer1_data, &led_timer2_data, &u_frame_data );
			}
			
			if( get_frame == LED_DATA_INPUT_FLIE_END ){
				led_active_flag_ctr( LED_FLAG_OFF, LED_TYPE_INCOMING );
			}
			
			gs_patern_data.incoming.check_flag = 0;
			mutex_unlock( &gs_patern_data.func_lock );
			
			wait_ret = 0;
			if(time != 0 ){
				wait_ret = wait_event_interruptible_timeout( gs_led_wq, ( gs_patern_data.incoming.check_flag != 0 ), msecs_to_jiffies(time) );
			}
			if( wait_ret != 0 )
				printk(KERN_INFO "[leds_data_input]wait_event_interruptible_timeout : %d \n",wait_ret);
		}
		else
		{
			led_active_flag_ctr( LED_FLAG_OFF, LED_TYPE_INCOMING );
			mutex_unlock( &gs_patern_data.func_lock );
		}
	}
	
	printk(KERN_INFO "[leds_data_input]led_adp8861_3led_ctrl OUT\n");
	
	return ;
}
#endif /* #ifdef LOCAL_CONFIG_FEATURE_NCMC_ELEGANT_SLIM */

#if defined(CONFIG_FEATURE_NCMC_D121F) || defined(CONFIG_FEATURE_NCMC_KAMITSUKIGAME) || defined(CONFIG_FEATURE_NCMC_SYLPH)
/*----------------------------------------------------------------------------
* MODULE   : led_adp8861_keyilm_ctrl_reg_time
* FUNCTTION: 
* NOTE     : None
* RETURN   : 
* CREATE   : 
* UPDATE   : 
----------------------------------------------------------------------------*/
static int led_adp8861_keyilm_ctrl_reg_time( union u_led_isct1_reg  *led_timer1_data,
										   union u_led_isct2_reg  *led_timer2_data)
{
	int						ret, ret_code = ADP8861_LED_SET_OK;
	union u_led_isct1_reg	led_timer1_set;
	union u_led_isct2_reg	led_timer2_set;
	

	memset( &led_timer2_set,  0, sizeof( led_timer2_set ) );
	led_timer2_set.st2.sc4_off = ADP8861_LED_ON;
	ret_code = ret = adp8861_led_timer_set2( &led_timer2_set, led_timer2_data );
	if( ret != ADP8861_LED_SET_OK ){
		printk(KERN_INFO "led_adp8861_keyilm_ctrl_reg_time: adp8861_led_timer_set2 = %d \n",ret);
	}
	memset( &led_timer1_set,  0, sizeof( led_timer1_set ) );
	led_timer1_set.st2.sc5_off = ADP8861_LED_ON;
	led_timer1_set.st2.sc6_off = ADP8861_LED_ON;
	led_timer1_set.st2.scon = ADP8861_LED_ON;
	ret_code |= ret = adp8861_led_timer_set1( &led_timer1_set, led_timer1_data );
	if( ret != ADP8861_LED_SET_OK ){
		printk(KERN_INFO "led_adp8861_keyilm_ctrl_reg_time: adp8861_led_timer_set1 = %d \n",ret);
	}
	
	return ret_code;
}

/*----------------------------------------------------------------------------
* MODULE   : led_adp8861_keyilm_ctrl_reg
* FUNCTTION: 
* NOTE     : None
* RETURN   : 
* CREATE   : 
* UPDATE   : 
----------------------------------------------------------------------------*/
static int led_adp8861_keyilm_ctrl_reg( unsigned char timer_set_flg,
									   union u_led_isct1_reg  *led_timer1_data,
									   union u_led_isct2_reg  *led_timer2_data,
									   u_input_frame_es_type *u_frame_data )
{
	int						ret, ret_code = ADP8861_LED_SET_OK;
	union u_led_isc_reg		set_brigh[3];
	struct led_request_rgb	set_onoff;


	memset( set_brigh,  0, sizeof( set_brigh ) );
	memset( &set_onoff, 0, sizeof( set_onoff ) );
	
	set_brigh[0].st2.set_flag = ADP8861_LED_ON;
	set_brigh[0].st2.scd	  = u_frame_data->s_data.brightness[0];
	if( u_frame_data->s_data.brightness[0] != 0 ){
		set_onoff.set_r = ADP8861_LED_ON;
	}
	set_brigh[1].st2.set_flag = ADP8861_LED_ON;
	set_brigh[1].st2.scd	  = u_frame_data->s_data.brightness[1];
	if( u_frame_data->s_data.brightness[1] != 0 ){
	set_onoff.set_g =  ADP8861_LED_ON;
	}
	set_brigh[2].st2.set_flag = ADP8861_LED_ON;
	set_brigh[2].st2.scd	  = u_frame_data->s_data.brightness[2];
	if( u_frame_data->s_data.brightness[2] != 0 ){
	set_onoff.set_b =  ADP8861_LED_ON;
	}
	ret_code |= ret = adp8861_key_led_bright( &set_brigh[0], &set_brigh[1], &set_brigh[2] );
	if( ret != ADP8861_LED_SET_OK ){
		printk(KERN_INFO "led_adp8861_3led_ctrl: adp8861_key_led_bright = %d \n",ret);
	}
	if( timer_set_flg == ADP8861_LED_ON ){
		ret = led_adp8861_keyilm_ctrl_reg_time( led_timer1_data, led_timer2_data);
	}
	ret_code |= ret = adp8861_key_led_set( &set_onoff );
	if( ret != ADP8861_LED_SET_OK ){
		printk(KERN_INFO "led_adp8861_3led_ctrl: adp8861_key_led_set(%d,%d,%d) = %d \n",set_onoff.set_r,set_onoff.set_g,set_onoff.set_b,ret);
	}


	return ret_code;
}

/*----------------------------------------------------------------------------
* MODULE   : led_adp8861_keyilm_ctrl_chk
* FUNCTTION: 
* NOTE     : None
* RETURN   : 
* CREATE   : 
* UPDATE   : 
----------------------------------------------------------------------------*/
int led_adp8861_keyilm_ctrl_chk( void )
{
	int						ret = 0;
	union u_led_isct1_reg  led_timer1_data;
	union u_led_isct2_reg  led_timer2_data;
	u_input_frame_es_type *up_frame_data;
	

	memset( &led_timer1_data,  0, sizeof( led_timer1_data ) );
	memset( &led_timer2_data,  0, sizeof( led_timer2_data ) );

	if( gs_patern_data.front.loop_flag == LED_FLAG_ON ){
		
		up_frame_data = (u_input_frame_es_type *)gs_patern_data.front.s_set_data.p_patern ;
		
		if( gs_patern_data.front.s_set_data.s_data_info.frame_num == 2 ){
			
			if (( up_frame_data[0].s_data.fade.hw_act == LED_FLAG_ON )
			 && ( up_frame_data[1].s_data.fade.hw_act == LED_FLAG_ON )){
				
				if (( up_frame_data[0].s_data.fade.hw_on_off == LED_FLAG_ON  )
				 && ( up_frame_data[1].s_data.fade.hw_on_off == LED_FLAG_OFF )){
					
					led_timer1_data.st2.scon    = up_frame_data[0].s_data.fade.hw_set_data;
					led_timer2_data.st2.sc4_off = up_frame_data[1].s_data.fade.hw_set_data;
					led_timer1_data.st2.sc5_off = up_frame_data[1].s_data.fade.hw_set_data;
					led_timer1_data.st2.sc6_off = up_frame_data[1].s_data.fade.hw_set_data;
					
					led_adp8861_keyilm_ctrl_reg( ADP8861_LED_ON, &led_timer1_data, &led_timer2_data, &up_frame_data[0] );
					
					led_active_flag_ctr( LED_FLAG_OFF, LED_TYPE_FRONT );
					
					printk(KERN_INFO "[leds_data_input]led_adp8861_keyilm_ctrl_chk HRD1 \n");
					ret = 1;
				}
				else if (( up_frame_data[0].s_data.fade.hw_on_off == LED_FLAG_OFF )
					  && ( up_frame_data[1].s_data.fade.hw_on_off == LED_FLAG_ON  )){
					
					led_timer1_data.st2.scon    = up_frame_data[1].s_data.fade.hw_set_data;
					led_timer2_data.st2.sc4_off = up_frame_data[0].s_data.fade.hw_set_data;
					led_timer1_data.st2.sc5_off = up_frame_data[0].s_data.fade.hw_set_data;
					led_timer1_data.st2.sc6_off = up_frame_data[0].s_data.fade.hw_set_data;
					
					led_adp8861_keyilm_ctrl_reg( ADP8861_LED_ON, &led_timer1_data, &led_timer2_data, &up_frame_data[1] );
					
					led_active_flag_ctr( LED_FLAG_OFF, LED_TYPE_FRONT );
					
					printk(KERN_INFO "[leds_data_input]led_adp8861_keyilm_ctrl_chk HRD2 \n");
					ret = 1;
				}
			}
			else if(UP_FRAME_DATA_ON(0)){
				if( UP_FRAME_DATA_OFF(1) ){
					if( up_frame_data[0].s_data.u_time.time == 0x12 ){
						led_timer2_data.st2.sc4_off = 0;
						led_timer1_data.st2.sc5_off = 0;
						led_timer1_data.st2.sc6_off = 0;
					}
					if( up_frame_data[0].s_data.u_time.time == 0x16 ){
						led_timer2_data.st2.sc4_off = 1;
						led_timer1_data.st2.sc5_off = 1;
						led_timer1_data.st2.sc6_off = 1;
					}
					if( up_frame_data[0].s_data.u_time.time == 0x18 ){
						led_timer2_data.st2.sc4_off = 2;
						led_timer1_data.st2.sc5_off = 2;
						led_timer1_data.st2.sc6_off = 2;
					}
					if( up_frame_data[0].s_data.u_time.time == 0x1C ){
						led_timer2_data.st2.sc4_off = 3;
						led_timer1_data.st2.sc5_off = 3;
						led_timer1_data.st2.sc6_off = 3;
					}
					if( up_frame_data[1].s_data.u_time.time == 0x16 ){
						led_timer1_data.st2.scon = 1;
					}
					if( up_frame_data[1].s_data.u_time.time == 0x1C ){
						led_timer1_data.st2.scon = 2;
					}
					led_adp8861_keyilm_ctrl_reg( ADP8861_LED_ON, &led_timer1_data, &led_timer2_data, &up_frame_data[0] );
					
					led_active_flag_ctr( LED_FLAG_OFF, LED_TYPE_FRONT );
					
					printk(KERN_INFO "[leds_data_input]led_adp8861_keyilm_ctrl_chk HRD1 \n");
					ret = 1;
				}
			}
			else if(UP_FRAME_DATA_ON(1)){
				if( UP_FRAME_DATA_OFF(0) ){
					if( up_frame_data[1].s_data.u_time.time == 0x12 ){
						led_timer2_data.st2.sc4_off = 0;
						led_timer1_data.st2.sc5_off = 0;
						led_timer1_data.st2.sc6_off = 0;
					}
					if( up_frame_data[1].s_data.u_time.time == 0x16 ){
						led_timer2_data.st2.sc4_off = 1;
						led_timer1_data.st2.sc5_off = 1;
						led_timer1_data.st2.sc6_off = 1;
					}
					if( up_frame_data[1].s_data.u_time.time == 0x18 ){
						led_timer2_data.st2.sc4_off = 2;
						led_timer1_data.st2.sc5_off = 2;
						led_timer1_data.st2.sc6_off = 2;
					}
					if( up_frame_data[1].s_data.u_time.time == 0x1C ){
						led_timer2_data.st2.sc4_off = 3;
						led_timer1_data.st2.sc5_off = 3;
						led_timer1_data.st2.sc6_off = 3;
					}
					if( up_frame_data[0].s_data.u_time.time == 0x16 ){
						led_timer1_data.st2.scon = 1;
					}
					if( up_frame_data[0].s_data.u_time.time == 0x1C ){
						led_timer1_data.st2.scon = 2;
					}
					led_adp8861_keyilm_ctrl_reg( ADP8861_LED_ON, &led_timer1_data, &led_timer2_data, &up_frame_data[1] );
					
					led_active_flag_ctr( LED_FLAG_OFF, LED_TYPE_FRONT );
					
					printk(KERN_INFO "[leds_data_input]led_adp8861_keyilm_ctrl_chk HRD2 \n");
					ret = 1;
				}
			}
		}
	}
	led_adp8861_keyilm_ctrl_reg_time( &led_timer1_data, &led_timer2_data );
	

	return ret;
}
#endif /* #if defined(CONFIG_FEATURE_NCMC_D121F) || defined(CONFIG_FEATURE_NCMC_KAMITSUKIGAME) || defined(CONFIG_FEATURE_NCMC_SYLPH) */

#ifdef LOCAL_CONFIG_FEATURE_NCMC_ELEGANT_SLIM
/*----------------------------------------------------------------------------
* MODULE   : led_adp8861_keyilm_ctrl
* FUNCTTION: 
* NOTE     : None
* RETURN   : 
* CREATE   : 
* UPDATE   : 
----------------------------------------------------------------------------*/
static void led_adp8861_keyilm_ctrl( void )
{
	union u_input_frame_es u_frame_data;
	int get_frame, time_a = 0, time_b = 0, time = 0;
	int wait_ret, ret, count;
	int active_flag;
#if defined(CONFIG_FEATURE_NCMC_D121F) || defined(CONFIG_FEATURE_NCMC_KAMITSUKIGAME) || defined(CONFIG_FEATURE_NCMC_SYLPH)
//	struct led_request_rgb	set_onoff;
#else /* #if defined(CONFIG_FEATURE_NCMC_D121F) || defined(CONFIG_FEATURE_NCMC_KAMITSUKIGAME) || defined(CONFIG_FEATURE_NCMC_SYLPH) */
	struct led_request_key	set_onoff;
	union u_led_isc_reg set_brigh[3];
#endif /* #if defined(CONFIG_FEATURE_NCMC_D121F) || defined(CONFIG_FEATURE_NCMC_KAMITSUKIGAME) || defined(CONFIG_FEATURE_NCMC_SYLPH) */
	union u_led_iscf_reg	led_fade;
	struct ncmlight_state_t state;
#if defined(CONFIG_FEATURE_NCMC_D121F) || defined(CONFIG_FEATURE_NCMC_KAMITSUKIGAME) || defined(CONFIG_FEATURE_NCMC_SYLPH)
	union u_led_isct1_reg  led_timer1_data;
	union u_led_isct2_reg  led_timer2_data;
#endif /* #if defined(CONFIG_FEATURE_NCMC_D121F) || defined(CONFIG_FEATURE_NCMC_KAMITSUKIGAME) || defined(CONFIG_FEATURE_NCMC_SYLPH) */
	
	printk(KERN_INFO "[leds_data_input]led_adp8861_keyilm_ctrl IN\n");
	
#if defined(CONFIG_FEATURE_NCMC_D121F) || defined(CONFIG_FEATURE_NCMC_KAMITSUKIGAME) || defined(CONFIG_FEATURE_NCMC_SYLPH)
	memset( &led_timer1_data,  0, sizeof( led_timer1_data ) );
	memset( &led_timer2_data,  0, sizeof( led_timer2_data ) );
#endif /* #if defined(CONFIG_FEATURE_NCMC_D121F) || defined(CONFIG_FEATURE_NCMC_KAMITSUKIGAME) || defined(CONFIG_FEATURE_NCMC_SYLPH) */
	
	led_brightness_set_keyilm_status(ADP8861_LED_ON);
	
	wake_lock(&led_ilm_wake_lock);
	
	while(1){
		
		mutex_lock( &gs_patern_data.func_lock );
		
		active_flag = led_active_flag_ctr( LED_FLAG_GET, LED_TYPE_FRONT );
		
		if( active_flag == LED_FLAG_OFF ){
			memset( &state, 0, sizeof( state ) );
			ret = led_ilm_func_State[8][gs_led_state.ilmkey.main_state](&state, LED_TYPE_FRONT);
			if( ret != LED_DATA_INPUT_OK ){
				printk(KERN_INFO "led_adp8861_keyilm_ctrl: led_ilm_func_State(8,%d) = %d \n",gs_led_state.ilmkey.main_state, ret);
			}
			gs_patern_data.front.ps_thread = NULL;
			mutex_unlock( &gs_patern_data.func_lock );
			
			led_brightness_set_keyilm_status(ADP8861_LED_OFF);
			break;
		}
		
		get_frame = led_get_frame_data( u_frame_data.uc, LED_TYPE_FRONT );
		
		if( get_frame == LED_DATA_INPUT_OK || get_frame == LED_DATA_INPUT_FLIE_END ){
			
#if defined(CONFIG_FEATURE_NCMC_D121F)
			if( gs_patern_data.front.frame_cnt == 0 ){
				if( led_adp8861_keyilm_ctrl_chk() != 0 ){
					gs_patern_data.front.ps_thread = NULL;
					mutex_unlock( &gs_patern_data.func_lock );
					
					printk(KERN_INFO "[leds_data_input]led_adp8861_keyilm_ctrl hw end\n");
					break;
				}
				else{
					led_adp8861_keyilm_ctrl_reg_time( &led_timer1_data, &led_timer2_data);
				}
			}
#endif /* #if defined(CONFIG_FEATURE_NCMC_D121F) */
			
			if(( u_frame_data.s_data.fade.fade & 0x01 ) == 0x01 ){
				led_fade.uc = u_frame_data.s_data.u_time.time ;
				ret = adp8861_led_fade_set( &led_fade );
				time = 0;
			}
			else{
				if( 0 != (int)u_frame_data.s_data.u_time.time ){
					count = (int)u_frame_data.s_data.u_time.s_tmie.time_a;
					time_a = 1;
					while( count > 0){
						time_a *= 10;
						count-- ;
					}
					time_b = (int)u_frame_data.s_data.u_time.s_tmie.time_b;
					time = time_a * time_b * 10;
				}
				
#if defined(CONFIG_FEATURE_NCMC_D121F) || defined(CONFIG_FEATURE_NCMC_KAMITSUKIGAME) || defined(CONFIG_FEATURE_NCMC_SYLPH)
				led_adp8861_keyilm_ctrl_reg( ADP8861_LED_OFF, &led_timer1_data, &led_timer2_data, &u_frame_data );
#else /* #if defined(CONFIG_FEATURE_NCMC_D121F) || defined(CONFIG_FEATURE_NCMC_KAMITSUKIGAME) || defined(CONFIG_FEATURE_NCMC_SYLPH)*/
				memset( set_brigh,  0, sizeof( set_brigh ) );
				memset( &set_onoff, 0, sizeof( set_onoff ) );
				
#if defined(CONFIG_FEATURE_NCMC_D121F) || defined(CONFIG_FEATURE_NCMC_KAMITSUKIGAME) || defined(CONFIG_FEATURE_NCMC_SYLPH)
				set_brigh[0].st2.set_flag = ADP8861_LED_ON;
				set_brigh[0].st2.scd	  = u_frame_data.s_data.brightness[0];
				if( u_frame_data.s_data.brightness[0] != 0 ){
					set_onoff.set_r = ADP8861_LED_ON;
				}
				set_brigh[1].st2.set_flag = ADP8861_LED_ON;
				set_brigh[1].st2.scd	  = u_frame_data.s_data.brightness[1];
				if( u_frame_data.s_data.brightness[1] != 0 ){
					set_onoff.set_g = ADP8861_LED_ON;
				}
				set_brigh[2].st2.set_flag = ADP8861_LED_ON;
				set_brigh[2].st2.scd	  = u_frame_data.s_data.brightness[2];
				if( u_frame_data.s_data.brightness[2] != 0 ){
					set_onoff.set_b = ADP8861_LED_ON;
				}
#else /* #if defined(CONFIG_FEATURE_NCMC_D121F) || defined(CONFIG_FEATURE_NCMC_KAMITSUKIGAME) || defined(CONFIG_FEATURE_NCMC_SYLPH) */
				set_brigh[0].st2.set_flag = ADP8861_LED_ON;
				set_brigh[0].st2.scd	  = u_frame_data.s_data.brightness[0];
				if( u_frame_data.s_data.brightness[0] != 0 ){
					set_onoff.set_1 = ADP8861_LED_ON;
				}
#if defined(CONFIG_FEATURE_NCMC_ELEGANT_SLIM)
				set_brigh[1].st2.set_flag = ADP8861_LED_ON;
				set_brigh[1].st2.scd	  = u_frame_data.s_data.brightness[1];
				if( u_frame_data.s_data.brightness[1] != 0 ){
					set_onoff.set_2 = ADP8861_LED_ON;
				}
				set_brigh[2].st2.set_flag = ADP8861_LED_ON;
				set_brigh[2].st2.scd	  = u_frame_data.s_data.brightness[2];
				if( u_frame_data.s_data.brightness[2] != 0 ){
					set_onoff.set_3 = ADP8861_LED_ON;
				}
#endif /* #if defined(CONFIG_FEATURE_NCMC_ELEGANT_SLIM) */
#endif /* #if defined(CONFIG_FEATURE_NCMC_D121F) || defined(CONFIG_FEATURE_NCMC_KAMITSUKIGAME) || defined(CONFIG_FEATURE_NCMC_SYLPH) */
#if defined(CONFIG_FEATURE_NCMC_ELEGANT_SLIM) || defined(CONFIG_FEATURE_NCMC_D121F) || defined(CONFIG_FEATURE_NCMC_KAMITSUKIGAME) || defined(CONFIG_FEATURE_NCMC_SYLPH)
				ret = adp8861_key_led_bright( &set_brigh[0], &set_brigh[1], &set_brigh[2] );
#else /* #if defined(CONFIG_FEATURE_NCMC_ELEGANT_SLIM) || defined(CONFIG_FEATURE_NCMC_D121F) || defined(CONFIG_FEATURE_NCMC_KAMITSUKIGAME) || defined(CONFIG_FEATURE_NCMC_SYLPH) */
				ret = adp8861_key_led_bright( &set_brigh[0] );
#endif /* #if defined(CONFIG_FEATURE_NCMC_ELEGANT_SLIM) || defined(CONFIG_FEATURE_NCMC_D121F) || defined(CONFIG_FEATURE_NCMC_KAMITSUKIGAME) || defined(CONFIG_FEATURE_NCMC_SYLPH) */
				if( ret != ADP8861_LED_SET_OK ){
					printk(KERN_INFO "led_adp8861_keyilm_ctrl: adp8861_rgb_led_bright = %d \n",ret);
				}
				ret = adp8861_key_led_set( &set_onoff );
				if( ret != ADP8861_LED_SET_OK ){
#if defined(CONFIG_FEATURE_NCMC_D121F) || defined(CONFIG_FEATURE_NCMC_KAMITSUKIGAME) || defined(CONFIG_FEATURE_NCMC_SYLPH)
					printk(KERN_INFO "led_adp8861_keyilm_ctrl: adp8861_rgb_led_set(%d,%d,%d) = %d \n",set_onoff.set_r,set_onoff.set_g,set_onoff.set_b,ret);
#else /* #if defined(CONFIG_FEATURE_NCMC_D121F) || defined(CONFIG_FEATURE_NCMC_KAMITSUKIGAME) || defined(CONFIG_FEATURE_NCMC_SYLPH) */
#if defined(CONFIG_FEATURE_NCMC_ELEGANT_SLIM)
					printk(KERN_INFO "led_adp8861_keyilm_ctrl: adp8861_rgb_led_set(%d,%d,%d) = %d \n",set_onoff.set_1,set_onoff.set_2,set_onoff.set_3,ret);
#else /* #if defined(CONFIG_FEATURE_NCMC_ELEGANT_SLIM) */
					printk(KERN_INFO "led_adp8861_keyilm_ctrl: adp8861_rgb_led_set(%d) = %d \n",set_onoff.set_1,ret);
#endif /* #if defined(CONFIG_FEATURE_NCMC_ELEGANT_SLIM) */
#endif /* #if defined(CONFIG_FEATURE_NCMC_D121F) || defined(CONFIG_FEATURE_NCMC_KAMITSUKIGAME) || defined(CONFIG_FEATURE_NCMC_SYLPH) */
				}
#endif /* #if defined(CONFIG_FEATURE_NCMC_D121F) */
			}
			
			if( get_frame == LED_DATA_INPUT_FLIE_END ){
				led_active_flag_ctr( LED_FLAG_OFF, LED_TYPE_FRONT );
			}
			gs_patern_data.front.check_flag = 0;
			mutex_unlock( &gs_patern_data.func_lock );
			
			wait_ret = 0;
			if(time != 0 ){
				wait_ret =  wait_event_interruptible_timeout( gs_led_wq, ( gs_patern_data.front.check_flag != 0 ), msecs_to_jiffies(time) );
			}
			if( wait_ret != 0 )
				printk(KERN_INFO "[leds_data_input]wait_event_interruptible_timeout : %d \n",wait_ret);
		}
		else {
			led_active_flag_ctr( LED_FLAG_OFF, LED_TYPE_FRONT );
			mutex_unlock( &gs_patern_data.func_lock );
		}
	}
	
#if defined(CONFIG_FEATURE_NCMC_D121F)
//	if( gs_led_state.ilmkey.main_state != LED_STATE_LOCAL_OPE ){
//		led_brightness_set_keyilm_status(ADP8861_LED_OFF);
//	}
#else /* #if defined(CONFIG_FEATURE_NCMC_D121F) */
	led_brightness_set_keyilm_status(ADP8861_LED_OFF);
#endif /* #if defined(CONFIG_FEATURE_NCMC_D121F) */
	
	wake_unlock(&led_ilm_wake_lock);
	
	printk(KERN_DEBUG "[leds_data_input]led_adp8861_keyilm_ctrl OUT\n");
	
	return ;
}
#endif /* #ifdef LOCAL_CONFIG_FEATURE_NCMC_ELEGANT_SLIM */

#ifdef CONFIG_FEATURE_NCMC_PEACOCK
/*----------------------------------------------------------------------------
* MODULE   : led_bd6082gul_3led_ctrl
* FUNCTTION: 
* NOTE     : None
* RETURN   : 
* CREATE   : 
* UPDATE   : 
----------------------------------------------------------------------------*/
static void led_bd6082gul_3led_ctrl( void )
{
	struct input_frame_pck_led1_struct s_frame_data;
	struct input_frame_pck_led2_struct s_frame_data_back;
	int get_frame = LED_DATA_INPUT_OK;
	int time = 0;
	int wait_ret,ret;
	int active_flag;
	struct ncmlight_state_t state;
//	int ilm3led_main_state, ilmback_main_state ;
	
	printk(KERN_INFO "[leds_data_input]led_bd6082gul_3led_ctrl IN\n");
	
	while(1){
		
		mutex_lock( &gs_patern_data.func_lock );
		active_flag = led_active_flag_ctr( LED_FLAG_GET, LED_TYPE_INCOMING );
		
		led_state_set_chk(LED_TYPE_INCOMING);

		if( active_flag == LED_FLAG_OFF ){
			
			memset( &state, 0, sizeof( state ) );
			ret = led_ilm_func_State[4][gs_led_state.ilm3led.main_state](&state, LED_TYPE_INCOMING);
			if( ret != LED_DATA_INPUT_OK ){
				printk(KERN_INFO "led_bd6082gul_3led_ctrl: led_ilm_func_State(4,%d) = %d \n",gs_led_state.ilm3led.main_state, ret);
			}
			gs_patern_data.incoming.ps_thread = NULL;
			mutex_unlock( &gs_patern_data.func_lock );
			break;
		}
		
		get_frame = led_get_frame_data( &s_frame_data, LED_TYPE_INCOMING );
		
		if( get_frame == LED_DATA_INPUT_OK || get_frame == LED_DATA_INPUT_FLIE_END ){
			
			time = led_pck_time_tbl[(int)s_frame_data.u_led1_time.s_tmie.sfrgb][(int)s_frame_data.u_led1_time.s_tmie.trgb];
			
			led_patern_data_set_rgb( s_frame_data.brightness, s_frame_data.patern, s_frame_data.u_led1_time.time );
			

			if(( gs_led_state.ilm3led.main_state == LED_STATE_SYNC_OPE )
			&& ( gs_led_state.ilmback.main_state == LED_STATE_SYNC_OPE )
			&& ( g_camera_flag != LED_FLAG_ON )) {
				s_frame_data_back.brightness[ 0] = s_frame_data.brightness[0];
				s_frame_data_back.brightness[ 1] = s_frame_data.brightness[1];
				s_frame_data_back.brightness[ 2] = s_frame_data.brightness[2];
				s_frame_data_back.brightness[ 3] = s_frame_data.brightness[0];
				s_frame_data_back.brightness[ 4] = s_frame_data.brightness[1];
				s_frame_data_back.brightness[ 5] = s_frame_data.brightness[2];
				s_frame_data_back.brightness[ 6] = s_frame_data.brightness[3];
				s_frame_data_back.brightness[ 7] = s_frame_data.brightness[4];
				s_frame_data_back.brightness[ 8] = s_frame_data.brightness[5];
				s_frame_data_back.brightness[ 9] = s_frame_data.brightness[3];
				s_frame_data_back.brightness[10] = s_frame_data.brightness[4];
				s_frame_data_back.brightness[11] = s_frame_data.brightness[5];
				s_frame_data_back.patern[0]      = s_frame_data.patern[0];
				s_frame_data_back.patern[1]      = s_frame_data.patern[1];
				s_frame_data_back.patern[2]      = s_frame_data.patern[2];
				s_frame_data_back.patern[3]      = s_frame_data.patern[0];
				s_frame_data_back.patern[4]      = s_frame_data.patern[1];
				s_frame_data_back.patern[5]      = s_frame_data.patern[2];
				s_frame_data_back.u_led1_time.time = s_frame_data.u_led1_time.time;
				s_frame_data_back.u_led2_time.time = s_frame_data.u_led1_time.time;
				illumi_file_data_ctr( BD2812GU_LED_ILLUMI_BACK, &s_frame_data_back );
			}
			led_lock_wrapper( LED_LOCK_SET, LED_LOCK_TYPE_INCOMING );
			if( ( gs_patern_data.incoming.s_set_data.s_data_info.frame_num == 1 ) && ( gs_patern_data.incoming.loop_flag == LED_FLAG_ON ) ){
				
				gs_patern_data.incoming.ps_thread = NULL;
				led_lock_wrapper( LED_UNLOCK_SET, LED_LOCK_TYPE_INCOMING );
				mutex_unlock( &gs_patern_data.func_lock );
				printk(KERN_INFO "[leds_data_input]led_bd6082gul_3led_ctrl hw end\n");
				break;
			}
			led_lock_wrapper( LED_UNLOCK_SET, LED_LOCK_TYPE_INCOMING );

			if( get_frame == LED_DATA_INPUT_FLIE_END ){
				led_active_flag_ctr( LED_FLAG_OFF, LED_TYPE_INCOMING );
			}
			gs_patern_data.incoming.check_flag = 0;
			mutex_unlock( &gs_patern_data.func_lock );
			
			wait_ret = 0;
			wait_ret = wait_event_interruptible_timeout( gs_led_wq, ( gs_patern_data.incoming.check_flag != 0 ), msecs_to_jiffies(time) );

			if( wait_ret != 0 )
				printk(KERN_INFO "[leds_data_input]wait_event_interruptible_timeout : %d \n",wait_ret);
		}
		else
		{
			led_active_flag_ctr( LED_FLAG_OFF, LED_TYPE_INCOMING );
			mutex_unlock( &gs_patern_data.func_lock );
		}
	}
	
	printk(KERN_INFO "[leds_data_input]led_bd6082gul_3led_ctrl OUT\n");
	
	return ;
}
#endif /*#ifdef CONFIG_FEATURE_NCMC_PEACOCK */

#ifdef CONFIG_FEATURE_NCMC_PEACOCK
/*----------------------------------------------------------------------------
* MODULE   : led_bd6082gul_front_ctrl
* FUNCTTION: 
* NOTE     : None
* RETURN   : 
* CREATE   : 
* UPDATE   : 
----------------------------------------------------------------------------*/
static void led_bd6082gul_front_ctrl( void )
{
	struct input_frame_pck_led2_struct s_frame_data;
	int get_frame = LED_DATA_INPUT_OK;
	int set_time = 0;
	int time1 = 0;
	int time2 = 0;
	int wait_ret,ret;
	int active_flag;
//	int check_flag;
	struct ncmlight_state_t state;
	
	printk(KERN_INFO "[leds_data_input]led_bd6082gul_front_ctrl IN\n");
	
	wake_lock(&led_ilm_wake_lock);
	
	while(1){
		
		mutex_lock( &gs_patern_data.func_lock );
		
		active_flag = led_active_flag_ctr( LED_FLAG_GET, LED_TYPE_FRONT );
		
		if( active_flag == LED_FLAG_OFF ){
			illumi_file_data_ctr_end();
			
			memset( &state, 0, sizeof( state ) );
			ret = led_ilm_func_State[8][gs_led_state.ilmkey.main_state](&state, LED_TYPE_FRONT);
			if( ret != LED_DATA_INPUT_OK ){
				printk(KERN_INFO "led_bd6082gul_front_ctrl: led_ilm_func_State(8,%d) = %d \n",gs_led_state.ilmkey.main_state, ret);
			}
			gs_patern_data.front.ps_thread = NULL;
			mutex_unlock( &gs_patern_data.func_lock );
			
			break;
		}
		
		get_frame = led_get_frame_data( &s_frame_data, LED_TYPE_FRONT );
		
		if( get_frame == LED_DATA_INPUT_OK || get_frame == LED_DATA_INPUT_FLIE_END ){
			
			time1 = led_pck_time_tbl[(int)s_frame_data.u_led1_time.s_tmie.sfrgb][(int)s_frame_data.u_led1_time.s_tmie.trgb];
			time2 = led_pck_time_tbl[(int)s_frame_data.u_led2_time.s_tmie.sfrgb][(int)s_frame_data.u_led2_time.s_tmie.trgb];
			
			set_time = time1;
			if( set_time < time2 ){
				set_time = time2;
			}
			illumi_file_data_ctr( BD2812GU_LED_ILLUMI_FRONT, &s_frame_data );
			
			if( get_frame == LED_DATA_INPUT_FLIE_END ){
				led_active_flag_ctr( LED_FLAG_OFF, LED_TYPE_FRONT );
			}
			gs_patern_data.front.check_flag = 0;
			mutex_unlock( &gs_patern_data.func_lock );
			
			wait_ret = 0;
			wait_ret = wait_event_interruptible_timeout( gs_led_wq, ( gs_patern_data.front.check_flag != 0 ), msecs_to_jiffies(set_time) );

			if( wait_ret != 0 )
				printk(KERN_INFO "[leds_data_input]wait_event_interruptible_timeout : %d \n",wait_ret);
			
		}
		else
		{
			led_active_flag_ctr( LED_FLAG_OFF, LED_TYPE_FRONT );
			mutex_unlock( &gs_patern_data.func_lock );
		}
	}
	
	wake_unlock(&led_ilm_wake_lock);
	
	printk(KERN_INFO "[leds_data_input]led_bd6082gul_front_ctrl OUT\n");
	
	return ;
}
#endif /*#ifdef CONFIG_FEATURE_NCMC_PEACOCK */

#ifdef CONFIG_FEATURE_NCMC_PEACOCK
/*----------------------------------------------------------------------------
* MODULE   : led_bd6082gul_back_ctrl
* FUNCTTION: 
* NOTE     : None
* RETURN   : 
* CREATE   : 
* UPDATE   : 
----------------------------------------------------------------------------*/
static void led_bd6082gul_back_ctrl( void )
{
	struct input_frame_pck_led2_struct s_frame_data;
	int get_frame;
	int set_time = 0;
	int time1 = 0;
	int time2 = 0;
	int wait_ret,ret;
	int active_flag;
//	int check_flag;
	struct ncmlight_state_t state;
	
	printk(KERN_INFO "[leds_data_input]led_bd6082gul_back_ctrl IN\n");
		
	while(1){
		
		mutex_lock( &gs_patern_data.func_lock );
		
		active_flag = led_active_flag_ctr( LED_FLAG_GET, LED_TYPE_BACK );
		
		led_state_set_chk(LED_TYPE_BACK);
		
		if( active_flag == LED_FLAG_OFF ){
			memset( &state, 0, sizeof( state ) );
			
			if( g_camera_flag == LED_FLAG_OFF) {
				ret = led_ilm_func_State[14][gs_led_state.ilmback.main_state](&state, LED_TYPE_BACK);
			}
			else {
				ret = led_ilm_camera_func_State[5][gs_led_state.camera.main_state](&state, LED_TYPE_CAMERA);
			}
			
			if( ret != LED_DATA_INPUT_OK ){
				printk(KERN_INFO "led_bd6082gul_back_ctrl: led_ilm_func_State(14,%d) = %d \n",gs_led_state.ilmback.main_state, ret);
			}
			gs_patern_data.back.ps_thread = NULL;
			gs_patern_data.back_keep.ps_thread = NULL;
			mutex_unlock( &gs_patern_data.func_lock );
			break;
		}
		
		get_frame = led_get_frame_data( &s_frame_data, LED_TYPE_BACK );
		
		if( get_frame == LED_DATA_INPUT_OK || get_frame == LED_DATA_INPUT_FLIE_END ){
			
			time1 = led_pck_time_tbl[(int)s_frame_data.u_led1_time.s_tmie.sfrgb][(int)s_frame_data.u_led1_time.s_tmie.trgb];
			time2 = led_pck_time_tbl[(int)s_frame_data.u_led2_time.s_tmie.sfrgb][(int)s_frame_data.u_led2_time.s_tmie.trgb];
			
			set_time = time1;
			if( set_time < time2 ){
				set_time = time2;
			}
			
			illumi_file_data_ctr( BD2812GU_LED_ILLUMI_BACK, &s_frame_data );
			
			led_lock_wrapper( LED_LOCK_SET, BD2812GU_LED_ILLUMI_BACK );
			if( ( gs_patern_data.back.s_set_data.s_data_info.frame_num == 1 ) && ( gs_patern_data.back.loop_flag == LED_FLAG_ON ) ){
				
				gs_patern_data.back.ps_thread = NULL;
				led_lock_wrapper( LED_UNLOCK_SET, BD2812GU_LED_ILLUMI_BACK );
				mutex_unlock( &gs_patern_data.func_lock );
				printk(KERN_INFO "[leds_data_input]led_bd6082gul_back_ctrl hw end\n");
				break;
			}
			led_lock_wrapper( LED_UNLOCK_SET, BD2812GU_LED_ILLUMI_BACK );
			
			if( get_frame == LED_DATA_INPUT_FLIE_END ){
				led_active_flag_ctr( LED_FLAG_OFF, LED_TYPE_BACK );
			}
			
			gs_patern_data.back.check_flag = 0;
			mutex_unlock( &gs_patern_data.func_lock );
			
			wait_ret = 0;
			wait_ret = wait_event_interruptible_timeout( gs_led_wq, ( gs_patern_data.back.check_flag != 0 ), msecs_to_jiffies(set_time) );

			if( wait_ret != 0 )
				printk(KERN_INFO "[leds_data_input]wait_event_interruptible_timeout : %d \n",wait_ret);
		}
		else {
			led_active_flag_ctr( LED_FLAG_OFF, LED_TYPE_BACK );
			mutex_unlock( &gs_patern_data.func_lock );
		}
	}
	
	printk(KERN_INFO "[leds_data_input]led_bd6082gul_back_ctrl OUT\n");
	
	return ;
}
#endif /*#ifdef CONFIG_FEATURE_NCMC_PEACOCK */

/*----------------------------------------------------------------------------
* MODULE   : led_run_stop
* FUNCTTION: 
* NOTE     : None
* RETURN   : 
* CREATE   : 
* UPDATE   : 
----------------------------------------------------------------------------*/
static int led_run_stop( int type )
{
	int ret = 0;
#ifdef LOCAL_CONFIG_FEATURE_NCMC_ELEGANT_SLIM
	union u_led_iscf_reg	led_fade;
	union u_led_isc_reg set_brigh[3];
//	union u_led_isct1_reg	led_timer1_set;
//	union u_led_isct2_reg	led_timer2_set;
	union u_led_isct1_reg  led_timer1_data;
	union u_led_isct2_reg  led_timer2_data;
	struct led_request_rgb	set_onoff1;
	struct led_request_key	set_onoff2;
#endif /* #ifdef LOCAL_CONFIG_FEATURE_NCMC_ELEGANT_SLIM */
#ifdef CONFIG_FEATURE_NCMC_PEACOCK
	struct input_frame_pck_led1_struct s_frame_data1;
	struct input_frame_pck_led2_struct s_frame_data2;
#endif /* #ifdef CONFIG_FEATURE_NCMC_PEACOCK */
	
#ifdef LOCAL_CONFIG_FEATURE_NCMC_ELEGANT_SLIM
	memset( &led_fade,  0, sizeof( led_fade  ) );
	memset( &set_brigh, 0, sizeof( set_brigh ) );
	memset( &set_onoff1, 0, sizeof( set_onoff1 ) );
	memset( &set_onoff2, 0, sizeof( set_onoff2 ) );
	memset( &led_timer1_data,  0, sizeof( led_timer1_data ) );
	memset( &led_timer2_data,  0, sizeof( led_timer2_data ) );
#endif /* #ifdef LOCAL_CONFIG_FEATURE_NCMC_ELEGANT_SLIM */
	
	switch(type){
		
	  case LED_TYPE_INCOMING:
#ifdef LOCAL_CONFIG_FEATURE_NCMC_ELEGANT_SLIM
		ret = adp8861_led_fade_set( &led_fade );
		if( ret != ADP8861_LED_SET_OK ){
			printk(KERN_INFO "led_run_stop: adp8861_led_fade_set = %d \n",ret);
		}
		set_brigh[0].st2.set_flag = ADP8861_LED_ON;
		set_brigh[1].st2.set_flag = ADP8861_LED_ON;
		set_brigh[2].st2.set_flag = ADP8861_LED_ON;
		ret |= adp8861_rgb_led_bright( &set_brigh[0], &set_brigh[1], &set_brigh[2] );
		if( ret != ADP8861_LED_SET_OK ){
			printk(KERN_INFO "led_run_stop: adp8861_rgb_led_bright = %d \n",ret);
		}
		led_adp8861_3led_ctrl_reg_time( &led_timer1_data, &led_timer2_data);
		
		ret |= adp8861_rgb_led_set( &set_onoff1 );
		if( ret != ADP8861_LED_SET_OK ){
			printk(KERN_INFO "led_run_stop: adp8861_rgb_led_set(%d,%d,%d) = %d \n",set_onoff1.set_r,set_onoff1.set_g,set_onoff1.set_b,ret);
		}
#endif /* #ifdef LOCAL_CONFIG_FEATURE_NCMC_ELEGANT_SLIM */
#ifdef CONFIG_FEATURE_NCMC_PEACOCK
		memset( &s_frame_data1,  0, sizeof( s_frame_data1 ) );
		s_frame_data1.patern[0] = 7;
		s_frame_data1.patern[1] = 7;
		s_frame_data1.patern[2] = 7;
		led_patern_data_set_rgb( s_frame_data1.brightness, s_frame_data1.patern, s_frame_data1.u_led1_time.time );
#endif /* #ifdef CONFIG_FEATURE_NCMC_PEACOCK */
		break;
		
	  case LED_TYPE_FRONT:
#if defined(CONFIG_FEATURE_NCMC_D121F)
#ifdef LOCAL_CONFIG_FEATURE_NCMC_ELEGANT_SLIM
		ret = adp8861_led_fade_set( &led_fade );
		if( ret != ADP8861_LED_SET_OK ){
			printk(KERN_INFO "led_run_stop: adp8861_led_fade_set = %d \n",ret);
		}
		set_brigh[0].st2.set_flag = ADP8861_LED_ON;
		set_brigh[1].st2.set_flag = ADP8861_LED_ON;
		set_brigh[2].st2.set_flag = ADP8861_LED_ON;
		
		ret = adp8861_key_led_bright( &set_brigh[0], &set_brigh[1], &set_brigh[2] );
		if( ret != ADP8861_LED_SET_OK ){
			printk(KERN_INFO "led_run_stop: adp8861_rgb_led_bright = %d \n",ret);
		}
		led_adp8861_keyilm_ctrl_reg_time( &led_timer1_data, &led_timer2_data);
		
		ret = adp8861_key_led_set( &set_onoff1 );
		if( ret != ADP8861_LED_SET_OK ){
			printk(KERN_INFO "led_run_stop: adp8861_rgb_led_set(%d,%d,%d) = %d \n",set_onoff1.set_r,set_onoff1.set_g,set_onoff1.set_b,ret);
		}
		led_brightness_set_keyilm_status(ADP8861_LED_OFF);
		
#endif /* #ifdef LOCAL_CONFIG_FEATURE_NCMC_ELEGANT_SLIM */
#ifdef CONFIG_FEATURE_NCMC_PEACOCK
		memset( &s_frame_data2,  0, sizeof( s_frame_data2 ) );
		s_frame_data2.patern[0] = 7;
		s_frame_data2.patern[1] = 7;
		s_frame_data2.patern[2] = 7;
		s_frame_data2.patern[3] = 7;
		s_frame_data2.patern[4] = 7;
		s_frame_data2.patern[5] = 7;
		illumi_file_data_ctr( BD2812GU_LED_ILLUMI_FRONT, &s_frame_data2 );
#endif /* #ifdef CONFIG_FEATURE_NCMC_PEACOCK */
#endif /* #if defined(CONFIG_FEATURE_NCMC_D121F) */
		break;
#ifdef CONFIG_FEATURE_NCMC_PEACOCK
	  case LED_TYPE_BACK:
		memset( &s_frame_data2,  0, sizeof( s_frame_data2 ) );
		s_frame_data2.patern[0] = 7;
		s_frame_data2.patern[1] = 7;
		s_frame_data2.patern[2] = 7;
		s_frame_data2.patern[3] = 7;
		s_frame_data2.patern[4] = 7;
		s_frame_data2.patern[5] = 7;
		illumi_file_data_ctr( BD2812GU_LED_ILLUMI_BACK, &s_frame_data2 );
		break;
#endif /* #ifdef CONFIG_FEATURE_NCMC_PEACOCK */
	  default:
		ret = -EINVAL;
        break;
    }
	
	return ret;
}

/*----------------------------------------------------------------------------
* MODULE   : led_incoming_thread
* FUNCTTION: 
* NOTE     : None
* RETURN   : 
* CREATE   : 
* UPDATE   : 
----------------------------------------------------------------------------*/
static void led_incoming_thread( void )
{
#ifdef CONFIG_FEATURE_NCMC_PEACOCK
	led_bd6082gul_3led_ctrl();
#endif
#ifdef LOCAL_CONFIG_FEATURE_NCMC_ELEGANT_SLIM
	led_adp8861_3led_ctrl();
#endif /* #ifdef LOCAL_CONFIG_FEATURE_NCMC_ELEGANT_SLIM */
	return ;
}

/*----------------------------------------------------------------------------
* MODULE   : led_front_thread
* FUNCTTION: 
* NOTE     : None
* RETURN   : 
* CREATE   : 
* UPDATE   : 
----------------------------------------------------------------------------*/
static void led_front_thread( void )
{
#ifdef CONFIG_FEATURE_NCMC_PEACOCK
	led_bd6082gul_front_ctrl();
#endif
#ifdef LOCAL_CONFIG_FEATURE_NCMC_ELEGANT_SLIM
	led_adp8861_keyilm_ctrl();
#endif /* #ifdef LOCAL_CONFIG_FEATURE_NCMC_ELEGANT_SLIM */
	return ;
}

/*----------------------------------------------------------------------------
* MODULE   : led_back_thread
* FUNCTTION: 
* NOTE     : None
* RETURN   : 
* CREATE   : 
* UPDATE   : 
----------------------------------------------------------------------------*/
#ifdef CONFIG_FEATURE_NCMC_PEACOCK
static void led_back_thread( void )
{
	led_bd6082gul_back_ctrl();
	
	return ;
}
#endif /* #ifdef CONFIG_FEATURE_NCMC_PEACOCK */


/*----------------------------------------------------------------------------
* MODULE   : led_thread_create
* FUNCTTION: 
* NOTE     : None
* RETURN   : 
* CREATE   : 
* UPDATE   : 
----------------------------------------------------------------------------*/
static int led_thread_create( int type )
{
	int ret = 0;
	
	
	switch(type){
	
    case LED_TYPE_INCOMING:
    
    	led_lock_wrapper( LED_LOCK_SET, LED_LOCK_TYPE_INCOMING );
    	
		if( gs_patern_data.incoming.ps_thread == NULL ){
			gs_patern_data.incoming.ps_thread = kthread_run((void*)*led_incoming_thread, NULL, "led_incoming_thread" );
			
			if( IS_ERR(gs_patern_data.incoming.ps_thread) ){
				printk(KERN_INFO "[leds_data_input]kthread_run NG\n" );
				gs_patern_data.incoming.ps_thread = NULL;
				ret = -EINVAL;
			}
		}
		
		led_lock_wrapper( LED_UNLOCK_SET, LED_LOCK_TYPE_INCOMING );
		
		break;
		
    case LED_TYPE_FRONT:
    
    	led_lock_wrapper( LED_LOCK_SET, LED_LOCK_TYPE_FRONT );
    	
		if( gs_patern_data.front.ps_thread == NULL ){
			 gs_patern_data.front.ps_thread = kthread_run((void*)*led_front_thread, NULL, "led_front_thread" );
			
			if( IS_ERR(gs_patern_data.front.ps_thread) ){
				printk(KERN_INFO "[leds_data_input]kthread_run NG\n" );
				gs_patern_data.front.ps_thread = NULL;
				ret = -EINVAL;
			}
		}
		
		led_lock_wrapper( LED_UNLOCK_SET, LED_LOCK_TYPE_FRONT );
    
		break;
#ifdef CONFIG_FEATURE_NCMC_PEACOCK
    case LED_TYPE_BACK:
    
    	led_lock_wrapper( LED_LOCK_SET, LED_LOCK_TYPE_BACK );
    	
		if( gs_patern_data.back.ps_thread == NULL ){
			 gs_patern_data.back.ps_thread = kthread_run((void*)*led_back_thread, NULL, "led_back_thread" );
			
			if( IS_ERR(gs_patern_data.back.ps_thread) ){
				printk(KERN_INFO "[leds_data_input]kthread_run NG\n" );
				gs_patern_data.back.ps_thread = NULL;
				ret = -EINVAL;
			}
		}
		
		led_lock_wrapper( LED_UNLOCK_SET, LED_LOCK_TYPE_BACK );
		
		break;
#endif /* #ifdef CONFIG_FEATURE_NCMC_PEACOCK */
    default:
		
		ret = -EINVAL;
        break;
    }
	return ret;
}

/*----------------------------------------------------------------------------
* MODULE   : led_thread_stop
* FUNCTTION: 
* NOTE     : None
* RETURN   : 
* CREATE   : 
* UPDATE   : 
----------------------------------------------------------------------------*/
static int led_thread_stop( int type )
{
	int ret = 0;
	
	
	switch(type){
	
    case LED_TYPE_INCOMING:
    
    	led_lock_wrapper( LED_LOCK_SET, LED_LOCK_TYPE_INCOMING );
    	
		if( gs_patern_data.incoming.ps_thread != NULL ){
//			kthread_stop( gs_patern_data.incoming.ps_thread );
//			gs_patern_data.incoming.ps_thread = NULL;
			gs_patern_data.incoming.active_flag = LED_FLAG_OFF;
			led_wake_up_thread( type );
		}
		
		led_lock_wrapper( LED_UNLOCK_SET, LED_LOCK_TYPE_INCOMING );
		
		break;
		
    case LED_TYPE_FRONT:
    
    	led_lock_wrapper( LED_LOCK_SET, LED_LOCK_TYPE_FRONT );
    	
		if( gs_patern_data.front.ps_thread != NULL ){
//			kthread_stop( gs_patern_data.front.ps_thread );
//			gs_patern_data.front.ps_thread = NULL;
			gs_patern_data.front.active_flag = LED_FLAG_OFF;
			led_wake_up_thread( type );
		}
		
		led_lock_wrapper( LED_UNLOCK_SET, LED_LOCK_TYPE_FRONT );
    
		break;
#ifdef CONFIG_FEATURE_NCMC_PEACOCK
    case LED_TYPE_BACK:
    
    	led_lock_wrapper( LED_LOCK_SET, LED_LOCK_TYPE_BACK );
    	
		if( gs_patern_data.back.ps_thread != NULL ){
//			kthread_stop( gs_patern_data.back.ps_thread );
//			gs_patern_data.back.ps_thread = NULL;
			gs_patern_data.back.active_flag = LED_FLAG_OFF;
			led_wake_up_thread( type );
		}
		
		led_lock_wrapper( LED_UNLOCK_SET, LED_LOCK_TYPE_BACK );
		
		break;
#endif /* #ifdef CONFIG_FEATURE_NCMC_PEACOCK */
    default:
		
		ret = -EINVAL;
        break;
    }
	
	
	return ret;
}

/*----------------------------------------------------------------------------
* MODULE   : led_patern_data_read
* FUNCTTION: 
* NOTE     : None
* RETURN   : 
* CREATE   : 
* UPDATE   : 
----------------------------------------------------------------------------*/
static int led_patern_data_read( unsigned char *p_data )
{
	struct input_data_head_struct s_head_data;
	void *p_patern_data;
	int err = 0;
	int switch_type;
	struct ncmlight_state_t state;
	

	memcpy( &s_head_data, p_data, sizeof(s_head_data) );
	memset( &state, 0, sizeof( state ) );
	
	printk(KERN_INFO "[leds_data_input]data_size = %d led_type = %d led_num = %d frame_num = %d frame_size = %d\n",
			s_head_data.data_size, s_head_data.led_type, s_head_data.led_num, s_head_data.frame_num, s_head_data.frame_size );
 	
	if( ( s_head_data.frame_size == 0 ) || 
		( s_head_data.frame_num == 0 )  || 
		( s_head_data.data_size != s_head_data.frame_num * s_head_data.frame_size ) ){
		printk(KERN_INFO "[leds_data_input]led_patern_data_read head_data NG\n");
		return -EINVAL;
	}
	
	p_patern_data = kzalloc( s_head_data.data_size, GFP_KERNEL );
	
	if( p_patern_data == NULL ){
		printk(KERN_INFO "[leds_data_input]led_patern_data_read p_data kzalloc NG\n");
		return -EINVAL;
	}
	
	memcpy( p_patern_data, p_data + sizeof(s_head_data), s_head_data.data_size );
	
	switch_type = s_head_data.led_type;
	
#ifdef CONFIG_FEATURE_NCMC_PEACOCK
	if( g_camera_flag == LED_FLAG_ON ) {
		if( switch_type == LED_TYPE_BACK ) {
			switch_type = LED_TYPE_CAMERA;
		}
		else if( switch_type == LED_TYPE_CAMERA) {
			switch_type = LED_TYPE_BACK;
		}
	}
#endif /* #ifdef CONFIG_FEATURE_NCMC_PEACOCK */
	
    switch(switch_type){
	
    case LED_TYPE_INCOMING:
		
		led_lock_wrapper( LED_LOCK_SET, LED_LOCK_TYPE_INCOMING );
		
		if( gs_patern_data.incoming.s_set_data.p_patern == NULL ){
			gs_patern_data.incoming.s_set_data.p_patern = p_patern_data;
			memcpy( &gs_patern_data.incoming.s_set_data.s_data_info, &s_head_data, sizeof(s_head_data) );
		}
		else{
			if( gs_patern_data.incoming.s_ready_data.p_patern != NULL ){
				kfree( gs_patern_data.incoming.s_ready_data.p_patern );
			}
			gs_patern_data.incoming.s_ready_data.p_patern = p_patern_data;
			memcpy( &gs_patern_data.incoming.s_ready_data.s_data_info, &s_head_data, sizeof(s_head_data) );
		}
		
		led_lock_wrapper( LED_UNLOCK_SET, LED_LOCK_TYPE_INCOMING );
		

		err = led_ilm_func_State[0][gs_led_state.ilm3led.main_state](&state, s_head_data.led_type);
		
        break;
        
    case LED_TYPE_FRONT:
		
		led_lock_wrapper( LED_LOCK_SET, LED_LOCK_TYPE_FRONT );
		
		if( gs_patern_data.front.s_set_data.p_patern == NULL ){
			gs_patern_data.front.s_set_data.p_patern = p_patern_data;
			memcpy( &gs_patern_data.front.s_set_data.s_data_info, &s_head_data, sizeof(s_head_data) );
		}
		else{
			if( gs_patern_data.front.s_ready_data.p_patern != NULL ){
				kfree( gs_patern_data.front.s_ready_data.p_patern );
			}
			gs_patern_data.front.s_ready_data.p_patern = p_patern_data;
			memcpy( &gs_patern_data.front.s_ready_data.s_data_info, &s_head_data, sizeof(s_head_data) );
		}
		

		led_lock_wrapper( LED_UNLOCK_SET, LED_LOCK_TYPE_FRONT );
		
		err = led_ilm_func_State[5][gs_led_state.ilmkey.main_state](&state, s_head_data.led_type);
		
        break;
#ifdef CONFIG_FEATURE_NCMC_PEACOCK
    case LED_TYPE_BACK:
    
		led_lock_wrapper( LED_LOCK_SET, LED_LOCK_TYPE_BACK );
		
		if( gs_patern_data.back.s_set_data.p_patern == NULL ){
			gs_patern_data.back.s_set_data.p_patern = p_patern_data;
			memcpy( &gs_patern_data.back.s_set_data.s_data_info, &s_head_data, sizeof(s_head_data) );
		}
		else{
			if( gs_patern_data.back.s_ready_data.p_patern != NULL ){
				kfree( gs_patern_data.back.s_ready_data.p_patern );
			}
			gs_patern_data.back.s_ready_data.p_patern = p_patern_data;
			memcpy( &gs_patern_data.back.s_ready_data.s_data_info, &s_head_data, sizeof(s_head_data) );
		}
		
		led_lock_wrapper( LED_UNLOCK_SET, LED_LOCK_TYPE_BACK );
		
		
    	if( g_camera_flag != LED_FLAG_ON ) {
			err = led_ilm_func_State[9][gs_led_state.ilmback.main_state](&state, s_head_data.led_type);
		} else {
			err = led_ilm_camera_func_State[2][gs_led_state.camera.main_state](&state, s_head_data.led_type);
		}
		
        break;
        
        
    case LED_TYPE_CAMERA:
    
        if( g_camera_flag != LED_FLAG_ON ) {
			kfree( p_patern_data );
			return -EINVAL;
		}
    
		led_lock_wrapper( LED_LOCK_SET, LED_TYPE_CAMERA );
		
		if( gs_patern_data.back_keep.s_set_data.p_patern == NULL ){
			gs_patern_data.back_keep.s_set_data.p_patern = p_patern_data;
			memcpy( &gs_patern_data.back_keep.s_set_data.s_data_info, &s_head_data, sizeof(s_head_data) );
		}
		else{
			if( gs_patern_data.back_keep.s_ready_data.p_patern != NULL ){
				kfree( gs_patern_data.back_keep.s_ready_data.p_patern );
			}
			gs_patern_data.back_keep.s_ready_data.p_patern = p_patern_data;
			memcpy( &gs_patern_data.back_keep.s_ready_data.s_data_info, &s_head_data, sizeof(s_head_data) );
		}
		
		led_lock_wrapper( LED_UNLOCK_SET, LED_TYPE_CAMERA );
		
		
		err = led_ilm_func_State[9][gs_led_state.ilmback.main_state](&state, s_head_data.led_type);
		
        break;
#endif /* #ifdef CONFIG_FEATURE_NCMC_PEACOCK */
    default:
		
		kfree( p_patern_data );
		return -EINVAL;
        break;
    }
    

	return err;
}

/*----------------------------------------------------------------------------
* MODULE   : led_run_loop
* FUNCTTION: 
* NOTE     : None
* RETURN   : 
* CREATE   : 
* UPDATE   : 
----------------------------------------------------------------------------*/
static int led_run_loop( int type )
{
	int ret = 0;
	
	led_active_flag_ctr(LED_FLAG_ON, type );
	ret = led_thread_create( type );
	
	return ret;
}


/*----------------------------------------------------------------------------
* MODULE   : led_data_input_open
* FUNCTTION: 
* NOTE     : None
* RETURN   : 
* CREATE   : 
* UPDATE   : 
----------------------------------------------------------------------------*/
static int led_data_input_open(struct inode *inode, struct file *file)
{
    return 0;
}

#ifdef CONFIG_FEATURE_NCMC_PEACOCK
/*----------------------------------------------------------------------------
* MODULE   : led_clock_setting
* FUNCTTION: 
* NOTE     : None
* RETURN   : 
* CREATE   : 
* UPDATE   : 
----------------------------------------------------------------------------*/
static int led_clock_setting( unsigned char clk_sync )
{
	int ret = 0, ret_fnc = 0;
	struct led_rgb_clock_setting	bd6082gul_clk;
	struct illumi_led_clk_ctrl		bd2812gu_clk;
	
	memset( &bd6082gul_clk, 0, sizeof(bd6082gul_clk) );
	memset( &bd2812gu_clk, 0, sizeof(bd2812gu_clk) );
	
	if( clk_sync != 0 ){
		bd6082gul_clk.clken = 1;
		bd6082gul_clk.clkmd = 1;
		bd6082gul_clk.fsel  = 1;
		ret = bd6082gul_rgb_clock_setting( &bd6082gul_clk );
		if( ret != 0 ){
			ret_fnc = LED_DATA_INPUT_NG;
		    printk(KERN_INFO
		    	"[leds_data_input]led_clock_setting bd6082gul_rgb_clock_setting %d\n", ret );
		}
		bd2812gu_clk.clken = 1;
//		bd2812gu_clk.clkmd = 0;
		ret = bd2812gu_illumi_led_clk(BD2812GU_LED_ILLUMI_BACK, &bd2812gu_clk );
		if( ret != 0 ){
			ret_fnc = LED_DATA_INPUT_NG;
		    printk(KERN_INFO
		    	"[leds_data_input]led_clock_setting bd2812gu_illumi_led_clk %d\n", ret );
		}
	}
	else{
//		bd2812gu_clk.clken = 0;
//		bd2812gu_clk.clkmd = 0;
		ret = bd2812gu_illumi_led_clk(BD2812GU_LED_ILLUMI_BACK, &bd2812gu_clk );
		if( ret != 0 ){
			ret_fnc = LED_DATA_INPUT_NG;
		    printk(KERN_INFO
		    	"[leds_data_input]led_clock_setting bd2812gu_illumi_led_clk %d\n", ret );
		}
//		bd6082gul_clk.clken = 0;
//		bd6082gul_clk.clkmd = 0;
//		bd6082gul_clk.fsel  = 0;
		ret = bd6082gul_rgb_clock_setting( &bd6082gul_clk );
		if( ret != 0 ){
			ret_fnc = LED_DATA_INPUT_NG;
		    printk(KERN_INFO
		    	"[leds_data_input]led_clock_setting bd6082gul_rgb_clock_setting %d\n", ret );
		}
	}
	return ret_fnc ;
}
#endif /* #ifdef CONFIG_FEATURE_NCMC_PEACOCK */

/*----------------------------------------------------------------------------
* MODULE   : led_vma_open
* FUNCTTION: 
* NOTE     : None
* RETURN   : 
* CREATE   : 
* UPDATE   : 
----------------------------------------------------------------------------*/
void led_vma_open( struct vm_area_struct *vma )
{
	return ;
}

/*----------------------------------------------------------------------------
* MODULE   : led_vma_close
* FUNCTTION: 
* NOTE     : None
* RETURN   : 
* CREATE   : 
* UPDATE   : 
----------------------------------------------------------------------------*/
void led_vma_close( struct vm_area_struct *vma )
{
	return ;
}

static struct vm_operations_struct led_remap_vm_ops = {
    .open       = led_vma_open,
    .close      = led_vma_close,
};

/*----------------------------------------------------------------------------
* MODULE   : xxxxxxxxxxxxxxxxxx
* FUNCTTION: 
* NOTE     : None
* RETURN   : 
* CREATE   : 
* UPDATE   : 
----------------------------------------------------------------------------*/
static void led_state_set( int type, int main_state, int sub_state )
{
	printk(KERN_INFO "[leds_data_input]led_state_set type %d, main %d, sub %d \n", type, main_state, sub_state );
	
	switch(type){
	  case LED_TYPE_INCOMING:
		led_lock_wrapper( LED_LOCK_SET, LED_LOCK_TYPE_INCOMING );
#ifdef CONFIG_FEATURE_NCMC_PEACOCK
		if(( main_state == LED_STATE_SYNC_OPE )
		&& (gs_led_state.ilm3led.main_state != LED_STATE_SYNC_OPE )){
			led_clock_setting( BD6082GUL_LED_ON );
		}
		else if(( main_state != LED_STATE_SYNC_OPE )
		&& (gs_led_state.ilm3led.main_state == LED_STATE_SYNC_OPE )){
			led_clock_setting( BD6082GUL_LED_OFF );
		}
#endif /* #ifdef CONFIG_FEATURE_NCMC_PEACOCK */
		gs_led_state.ilm3led.main_state = main_state;
		gs_led_state.ilm3led.sub_state = sub_state;
		led_lock_wrapper( LED_UNLOCK_SET, LED_LOCK_TYPE_INCOMING );
		break;
	  case LED_TYPE_FRONT:
		led_lock_wrapper( LED_LOCK_SET, LED_LOCK_TYPE_FRONT );
		gs_led_state.ilmkey.main_state = main_state;
		gs_led_state.ilmkey.sub_state = sub_state;
		led_lock_wrapper( LED_UNLOCK_SET, LED_LOCK_TYPE_FRONT );
        break;
#ifdef CONFIG_FEATURE_NCMC_PEACOCK
	  case LED_TYPE_BACK:
		led_lock_wrapper( LED_LOCK_SET, LED_LOCK_TYPE_BACK );
		gs_led_state.ilmback.main_state = main_state;
		gs_led_state.ilmback.sub_state = sub_state;
		led_lock_wrapper( LED_UNLOCK_SET, LED_LOCK_TYPE_BACK );
        break;
        
	  case LED_TYPE_CAMERA:
		led_lock_wrapper( LED_LOCK_SET, LED_TYPE_CAMERA );
		gs_led_state.camera.main_state = main_state;
		gs_led_state.camera.sub_state = sub_state;
		led_lock_wrapper( LED_UNLOCK_SET, LED_TYPE_CAMERA );
        break;
        
#endif /* #ifdef CONFIG_FEATURE_NCMC_PEACOCK */
	  default:
		break;
	}
}

#ifdef CONFIG_FEATURE_NCMC_PEACOCK
/*----------------------------------------------------------------------------
* MODULE   : xxxxxxxxxxxxxxxxxx
* FUNCTTION: 
* NOTE     : None
* RETURN   : 
* CREATE   : 
* UPDATE   : 
----------------------------------------------------------------------------*/
static void led_state_set_req( int type, int main_state, int sub_state )
{
	printk(KERN_INFO "[leds_data_input]led_state_set_req type %d, main %d, sub %d \n", type, main_state, sub_state );
	
	switch(type){
	  case LED_TYPE_INCOMING:
		led_lock_wrapper( LED_LOCK_SET, LED_LOCK_TYPE_INCOMING );
		gs_led_state.ilm3led.req_flag = BD6082GUL_LED_ON;
		gs_led_state.ilm3led.req_main_state = main_state;
		gs_led_state.ilm3led.req_sub_state = sub_state;
		led_lock_wrapper( LED_UNLOCK_SET, LED_LOCK_TYPE_INCOMING );
		break;
	  case LED_TYPE_BACK:
		led_lock_wrapper( LED_LOCK_SET, LED_LOCK_TYPE_BACK );
		gs_led_state.ilmback.req_flag = BD2812GU_LED_ON;
		gs_led_state.ilmback.req_main_state = main_state;
		gs_led_state.ilmback.req_sub_state = sub_state;
		led_lock_wrapper( LED_UNLOCK_SET, LED_LOCK_TYPE_BACK );
        break;
	  default:
		break;
	}
}
/*----------------------------------------------------------------------------
* MODULE   : xxxxxxxxxxxxxxxxxx
* FUNCTTION: 
* NOTE     : None
* RETURN   : 
* CREATE   : 
* UPDATE   : 
----------------------------------------------------------------------------*/
static void led_state_set_chk( int type )
{
	switch(type){
	  case LED_TYPE_INCOMING:
		led_lock_wrapper( LED_LOCK_SET, LED_LOCK_TYPE_INCOMING );
		if( gs_led_state.ilm3led.req_flag == BD6082GUL_LED_ON){
			gs_led_state.ilm3led.req_flag = BD6082GUL_LED_OFF;
			led_state_set(LED_TYPE_INCOMING, gs_led_state.ilm3led.req_main_state, gs_led_state.ilm3led.req_sub_state );
//			gs_led_state.ilm3led.main_state = gs_led_state.ilm3led.req_main_state;
//			gs_led_state.ilm3led.sub_state  = gs_led_state.ilm3led.req_sub_state;
		}
		led_lock_wrapper( LED_UNLOCK_SET, LED_LOCK_TYPE_INCOMING );
		break;
	  case LED_TYPE_BACK:
		led_lock_wrapper( LED_LOCK_SET, LED_LOCK_TYPE_BACK );
		if( gs_led_state.ilmback.req_flag == BD2812GU_LED_ON ){
			gs_led_state.ilmback.req_flag = BD2812GU_LED_OFF;
			led_state_set(LED_TYPE_BACK, gs_led_state.ilmback.req_main_state, gs_led_state.ilmback.req_sub_state );
//			gs_led_state.ilmback.main_state = gs_led_state.ilmback.req_main_state;
//			gs_led_state.ilmback.sub_state  = gs_led_state.ilmback.req_sub_state;
		}
		led_lock_wrapper( LED_UNLOCK_SET, LED_LOCK_TYPE_BACK );
        break;
	  default:
		break;
	}
}
#endif /* #ifdef CONFIG_FEATURE_NCMC_PEACOCK */

static int led_func_error( struct ncmlight_state_t *state, int type )
{
	printk(KERN_INFO "[leds_data_input]led_func_error \n");
	
	return -EINVAL;
}

static int led_func_ok( struct ncmlight_state_t *state, int type )
{
	printk(KERN_DEBUG "[leds_data_input]led_func_ok \n");
	
	return 0;
}

static int led_func_stop( struct ncmlight_state_t *state, int type )
{
	int ret = 0;
	u8	*set_patern = NULL;
	
	printk(KERN_INFO "[leds_data_input]led_func_stop type = %d \n", type);
	
	
	ret = led_thread_stop( type );
	ret |= led_patern_data_del( type );
	set_patern = led_patern_data_switch( type );
	ret |= led_run_stop( type );
	
	if( set_patern == NULL ){
		led_state_set( type, LED_STATE_STOP, 0 );
	}
	else{
		led_state_set( type, LED_STATE_PATERN, 0 );
	}
	
	return ret;
}

static int led_func_0_0( struct ncmlight_state_t *state, int type )
{
	printk(KERN_INFO "[leds_data_input]led_func_0_0 type = %d \n", type);
	
	led_state_set( LED_TYPE_INCOMING, LED_STATE_PATERN, LED_STATE_SUB_3LED );
	
	return LED_DATA_INPUT_OK;
}

#ifdef CONFIG_FEATURE_NCMC_PEACOCK
static int led_func_0_5( struct ncmlight_state_t *state, int type )
{
	printk(KERN_INFO "[leds_data_input]led_func_0_5 type = %d \n", type);
	
	if( gs_led_state.ilm3led.sub_state == LED_STATE_SUB_BACKILM_STOP ){
		led_state_set( LED_TYPE_INCOMING, LED_STATE_SYNC_OPE_CANCEL, LED_STATE_SUB_BACKILM_WAIT );
	}
	
	return LED_DATA_INPUT_OK;
}
#endif /* #ifdef CONFIG_FEATURE_NCMC_PEACOCK */

static int led_func_1_1( struct ncmlight_state_t *state, int type )
{
	int ret = 0;
	
	printk(KERN_INFO "[leds_data_input]led_func_1_1 type = %d \n", type);
	
	led_patern_data_switch( type );
	
	ret = led_run_loop( LED_TYPE_INCOMING );
	if( ret == 0 ){
		led_state_set( LED_TYPE_INCOMING, LED_STATE_LOCAL_OPE, LED_STATE_SUB_3LED );
	}
	return ret;
}

static int led_func_1_2( struct ncmlight_state_t *state, int type )
{
	int ret = LED_DATA_INPUT_OK;
	u8	*set_patern = NULL;
	
	printk(KERN_INFO "[leds_data_input]led_func_1_2 type = %d \n", type);
	
	led_active_flag_ctr( LED_FLAG_ON, type );
	set_patern = led_patern_data_switch( type );
	led_wake_up_thread( type );
	
	if( set_patern == NULL ){
		ret = LED_DATA_INPUT_NG;
	}
	else {
		ret = led_thread_create( type );
	}
	
	return ret;
}

#ifdef CONFIG_FEATURE_NCMC_PEACOCK
static int led_func_2_1( struct ncmlight_state_t *state, int type )
{
	int ret = 0;
	
	printk(KERN_INFO "[leds_data_input]led_func_2_1 type = %d \n", type);
	
	switch(gs_led_state.ilmback.main_state){
	  case LED_STATE_STOP:
	  case LED_STATE_PATERN:
	  case LED_STATE_LOCAL_OPE:
		led_state_set( LED_TYPE_INCOMING, LED_STATE_SYNC_OPE_WAIT, LED_STATE_SUB_3LED );
		break;
	
	  case LED_STATE_SYNC_OPE_WAIT:
		ret = led_run_loop( LED_TYPE_INCOMING );
		if( ret == 0 ){
			led_state_set( LED_TYPE_INCOMING, LED_STATE_SYNC_OPE, LED_STATE_SUB_3LED );
			ret |= led_func_b_3( state, type);
//			led_state_set_req( LED_TYPE_BACK, LED_STATE_SYNC_OPE, LED_STATE_SUB_BACKILM );
		}
		break;
		
	  default:
		ret = led_func_error( state, type );
		break;
	}
	return ret;
}
#endif /* #ifdef CONFIG_FEATURE_NCMC_PEACOCK */


#ifdef CONFIG_FEATURE_NCMC_PEACOCK
static int led_func_2_3( struct ncmlight_state_t *state, int type )
{
	int ret = 0;
	
	printk(KERN_INFO "[leds_data_input]led_func_2_3 type = %d \n", type);
	
	led_state_set( LED_TYPE_INCOMING, LED_STATE_SYNC_OPE, LED_STATE_SUB_3LED );
	
	return ret;
}

static int led_func_2_4( struct ncmlight_state_t *state, int type )
{
	int ret = 0;
	u8	*set_patern = NULL;
	
	printk(KERN_INFO "[leds_data_input]led_func_2_4 type = %d \n", type);
	
	led_active_flag_ctr( LED_FLAG_ON, type );
	set_patern = led_patern_data_switch( type );
	led_wake_up_thread( type );
	
	if( set_patern == NULL ){
		ret = LED_DATA_INPUT_NG;
	}
	else {
		ret = led_thread_create( type );
	}
	return ret;
}

static int led_func_2_5( struct ncmlight_state_t *state, int type )
{
	int ret = 0;
	u8	*set_patern = NULL;
	
	printk(KERN_INFO "[leds_data_input]led_func_2_5 type = %d \n", type);
	
	led_active_flag_ctr( LED_FLAG_ON, type );
	set_patern = led_patern_data_switch( type );
	led_wake_up_thread( type );
	
	if( set_patern == NULL ){
		ret = LED_DATA_INPUT_NG;
	}
	else {
		ret = led_thread_create( type );
	}
	return ret;
}

static int led_func_3_4( struct ncmlight_state_t *state, int type )
{
	int ret = 0;
	u8	*set_patern = NULL;
	
	printk(KERN_INFO "[leds_data_input]led_func_3_4  type = %d \n", type);
	
	ret = led_thread_stop( type );
	ret |= led_patern_data_del( type );
	set_patern = led_patern_data_switch( type );
	ret |= led_run_stop( type );
	
	if( set_patern == NULL ){
		led_state_set( type, LED_STATE_STOP, 0 );
//		led_state_set_req( LED_TYPE_BACK, LED_STATE_STOP, 0 );
	}
	else{
		led_state_set( type, LED_STATE_PATERN, 0 );
//		led_state_set_req( LED_TYPE_BACK, LED_STATE_STOP, 0 );
	}
	
	if( gs_led_state.ilmback.main_state == LED_STATE_SYNC_OPE ) {
		
		if( g_camera_flag == LED_FLAG_ON ) {
			type = LED_TYPE_CAMERA;
		}
		else {
			type = LED_TYPE_BACK;
			ret |= led_run_stop( LED_TYPE_BACK );
		}
		
		ret |= led_patern_data_del( type );
		set_patern = led_patern_data_switch( type );
		
		if( set_patern == NULL ){
			led_state_set( LED_TYPE_BACK, LED_STATE_STOP, LED_STATE_SUB_BACKILM );
		}
		else{
			led_state_set( LED_TYPE_BACK, LED_STATE_PATERN, 0 );
		}
	}
	return ret;
}

static int led_func_3_5( struct ncmlight_state_t *state, int type )
{
	int ret = 0;
	u8	*set_patern = NULL;
	
	printk(KERN_INFO "[leds_data_input]led_func_3_5 type = %d \n", type);
	
	ret = led_thread_stop( type );
	ret |= led_patern_data_del( type );
	set_patern = led_patern_data_switch( type );
	ret |= led_run_stop( type );
	
	if( set_patern == NULL ){
		led_state_set( type, LED_STATE_STOP, 0 );
	}
	else{
		led_state_set( type, LED_STATE_PATERN, LED_STATE_SUB_3LED );
	}
	switch( gs_led_state.ilmback.sub_state ){
	  case LED_STATE_SUB_BACKILM_STOP:
		led_state_set_req( LED_TYPE_BACK, LED_STATE_STOP, 0 );
		break;
	
	  case LED_STATE_SUB_BACKILM_WAIT:
		led_state_set_req( LED_TYPE_BACK, LED_STATE_PATERN, 0 );
		break;
	  
	  case LED_STATE_SUB_BACKILM_RUN:
		led_state_set_req( LED_TYPE_BACK, LED_STATE_LOCAL_OPE, LED_STATE_SUB_BACKILM );
		break;
	
	  default:
		break;
	}
	return ret;
}

static int led_func_4_4( struct ncmlight_state_t *state, int type )
{
	int ret = 0;
	u8	*set_patern = NULL;
	
	printk(KERN_INFO "[leds_data_input]led_func_4_4 type = %d \n", type);
	
	ret = led_thread_stop( type );
	ret |= led_patern_data_del( type );
	set_patern = led_patern_data_switch( type );
	ret |= led_run_stop( type );
	
	if( set_patern == NULL ){
		led_state_set( type, LED_STATE_STOP, 0 );
//		led_state_set_req( LED_TYPE_BACK, LED_STATE_STOP, 0 );
	}
	else{
		led_state_set( type, LED_STATE_PATERN, 0 );
//		led_state_set_req( LED_TYPE_BACK, LED_STATE_STOP, 0 );
	}

	if( gs_led_state.ilmback.main_state == LED_STATE_SYNC_OPE ) {
		if( g_camera_flag == LED_FLAG_ON ) {
			type = LED_TYPE_CAMERA;
		}
		else {
			type = LED_TYPE_BACK;
			ret |= led_run_stop( LED_TYPE_BACK );
		}
		
		ret |= led_patern_data_del( type );
		set_patern = led_patern_data_switch( type );
		
		if( set_patern == NULL ){
			led_state_set( LED_TYPE_BACK, LED_STATE_STOP, LED_STATE_SUB_BACKILM );
		}
		else{
			led_state_set( LED_TYPE_BACK, LED_STATE_PATERN, 0 );
		}
	}
	return ret;
}

static int led_func_4_5( struct ncmlight_state_t *state, int type )
{
	int ret = 0;
	u8	*set_patern = NULL;
	
	printk(KERN_INFO "[leds_data_input]led_func_4_5 type = %d \n", type);
	
	ret = led_thread_stop( type );
	ret |= led_patern_data_del( type );
	set_patern = led_patern_data_switch( type );
	ret |= led_run_stop( type );
	
	if( set_patern == NULL ){
		led_state_set( type, LED_STATE_STOP, 0 );
	}
	else{
		led_state_set( type, LED_STATE_PATERN, LED_STATE_SUB_3LED );
	}
	switch( gs_led_state.ilmback.sub_state ){
	  case LED_STATE_SUB_BACKILM_STOP:
		led_state_set_req( LED_TYPE_BACK, LED_STATE_STOP, 0 );
		break;
	
	  case LED_STATE_SUB_BACKILM_WAIT:
		led_state_set_req( LED_TYPE_BACK, LED_STATE_PATERN, 0 );
		break;
	  
	  case LED_STATE_SUB_BACKILM_RUN:
		led_state_set_req( LED_TYPE_BACK, LED_STATE_LOCAL_OPE, LED_STATE_SUB_BACKILM );
		break;
	
	  default:
		break;
	}
	return ret;
}
#endif /* #ifdef CONFIG_FEATURE_NCMC_PEACOCK */


static int led_func_5_0( struct ncmlight_state_t *state, int type )
{
	printk(KERN_INFO "[leds_data_input]led_func_5_0 type = %d \n", type);
	
	led_state_set( LED_TYPE_FRONT, LED_STATE_PATERN, LED_STATE_SUB_3LED );
	
	return LED_DATA_INPUT_OK;
}

static int led_func_6_1( struct ncmlight_state_t *state, int type )
{
	int ret = 0;
	
	ret = led_run_loop( LED_TYPE_FRONT );
	if( ret == 0 ){
		led_state_set( LED_TYPE_FRONT, LED_STATE_LOCAL_OPE, LED_STATE_SUB_3LED );
	}
	return ret;
}

static int led_func_6_2( struct ncmlight_state_t *state, int type )
{
	int ret = 0;
	u8	*set_patern = NULL;
	
	printk(KERN_INFO "[leds_data_input]led_func_6_2 type = %d \n", type);
	
	led_active_flag_ctr( LED_FLAG_ON, type );
	set_patern = led_patern_data_switch( type );
	led_wake_up_thread( type );
	
	if( set_patern == NULL ){
		ret = LED_DATA_INPUT_NG;
	}
	else {
		ret = led_thread_create( type );
	}
	
	return ret;
}

#ifdef CONFIG_FEATURE_NCMC_PEACOCK
static int led_func_9_0( struct ncmlight_state_t *state, int type )
{
	printk(KERN_INFO "[leds_data_input]led_func_9_0 type = %d \n", type);
	
	led_state_set( LED_TYPE_BACK, LED_STATE_PATERN, LED_STATE_SUB_BACKILM );
	
	return LED_DATA_INPUT_OK;
}

static int led_func_9_5( struct ncmlight_state_t *state, int type )
{
	printk(KERN_INFO "[leds_data_input]led_func_9_5 type = %d \n", type);
	
	if( gs_led_state.ilmback.sub_state == LED_STATE_SUB_BACKILM_STOP ){
		led_state_set( LED_TYPE_BACK, LED_STATE_SYNC_OPE_CANCEL, LED_STATE_SUB_BACKILM_WAIT );
		led_state_set_req( LED_TYPE_INCOMING, LED_STATE_SYNC_OPE_CANCEL, LED_STATE_SUB_BACKILM_WAIT );
	}
	
	return LED_DATA_INPUT_OK;
}

static int led_func_a_1( struct ncmlight_state_t *state, int type )
{
	int ret = 0;
	
	ret = led_run_loop( LED_TYPE_BACK );
	if( ret == 0 ){
		led_state_set( LED_TYPE_BACK, LED_STATE_LOCAL_OPE, LED_STATE_SUB_BACKILM );
	}
	return ret;
}

static int led_func_a_2( struct ncmlight_state_t *state, int type )
{
	int ret = 0;
	u8	*set_patern = NULL;
	
	printk(KERN_INFO "[leds_data_input]led_func_a_2 type = %d \n", type);
	
	led_active_flag_ctr( LED_FLAG_ON, type );
	set_patern = led_patern_data_switch( type );
	led_wake_up_thread( type );
	
	if( set_patern == NULL ){
		ret = LED_DATA_INPUT_NG;
	}
	else {
		ret = led_thread_create( type );
	}
	
	return ret;
}

static int led_func_a_4( struct ncmlight_state_t *state, int type )
{
	int ret = 0;
	u8	*next_patern = NULL, *set_patern = NULL;
	
	printk(KERN_INFO "[leds_data_input]led_func_a_4 type = %d \n", type);
	
	led_active_flag_ctr( LED_FLAG_ON, type );
	next_patern = led_patern_data_check( type );
	led_wake_up_thread( type );
	
	if( next_patern == NULL ){
		ret = led_run_stop(type);
		ret |= led_patern_data_del( type );
		led_state_set( type, LED_STATE_SYNC_OPE_CANCEL, LED_STATE_SUB_BACKILM_STOP );
		led_state_set_req( LED_TYPE_INCOMING, LED_STATE_SYNC_OPE_CANCEL, LED_STATE_SUB_BACKILM_STOP );
	}
	else {
		set_patern = led_patern_data_switch( type );
		if( set_patern == NULL ){
			ret = LED_DATA_INPUT_NG;
		}
		else {
			ret = led_thread_create( type );
			led_state_set( type, LED_STATE_SYNC_OPE_CANCEL, LED_STATE_SUB_BACKILM_RUN );
			led_state_set_req( LED_TYPE_INCOMING, LED_STATE_SYNC_OPE_CANCEL, LED_STATE_SUB_BACKILM_RUN );
		}
	}
	return ret;
}

static int led_func_a_5( struct ncmlight_state_t *state, int type )
{
	int ret = 0;
	u8	*set_patern = NULL;
	
	printk(KERN_INFO "[leds_data_input]led_func_a_5 type = %d \n", type);
	
	switch( gs_led_state.ilmback.sub_state ){
	 case LED_STATE_SUB_BACKILM_STOP:
		ret = led_func_error( state, type );
		break;
		
	 case LED_STATE_SUB_BACKILM_WAIT:
		ret = led_run_loop( type );
		if( ret == 0 ){
			led_state_set( type, LED_STATE_SYNC_OPE_CANCEL, LED_STATE_SUB_BACKILM_RUN );
			led_state_set( LED_TYPE_INCOMING, LED_STATE_SYNC_OPE_CANCEL, LED_STATE_SUB_BACKILM_RUN );
		}
		break;
		
	 case LED_STATE_SUB_BACKILM_RUN:
		led_active_flag_ctr( LED_FLAG_ON, type );
		set_patern = led_patern_data_switch( type );
		led_wake_up_thread( type );
		
		if( set_patern == NULL ){
			ret = LED_DATA_INPUT_NG;
		}
		else {
			ret = led_thread_create( type );
		}
		break;
		
	  default:
		ret = led_func_error( state, type );
		break;
	}
	return ret;
}

static int led_func_b_1( struct ncmlight_state_t *state, int type )
{
	int ret = 0;
	
	printk(KERN_INFO "[leds_data_input]led_func_b_1 type = %d \n", type);
	
	switch(gs_led_state.ilm3led.main_state){
	  case LED_STATE_STOP:
	  case LED_STATE_PATERN:
	  case LED_STATE_LOCAL_OPE:
		led_state_set( LED_TYPE_BACK, LED_STATE_SYNC_OPE_WAIT, LED_STATE_SUB_BACKILM );
		break;
	
	  case LED_STATE_SYNC_OPE_WAIT:
		
		ret = led_run_loop( LED_TYPE_INCOMING );
		if( ret == 0 ){
			led_state_set( LED_TYPE_BACK, LED_STATE_SYNC_OPE, LED_STATE_SUB_BACKILM );
			ret |= led_func_2_3( state, type);
//			led_state_set_req( LED_TYPE_INCOMING, LED_STATE_SYNC_OPE, LED_STATE_SUB_3LED );
		}
		break;
		
	  default:
		ret = led_func_error( state, type );
		break;
	}
	return ret;
}


static int led_func_b_3( struct ncmlight_state_t *state, int type )
{
	int ret = 0;
	
	printk(KERN_INFO "[leds_data_input]led_func_b_3 type = %d \n", type);
	
	led_state_set( LED_TYPE_BACK, LED_STATE_SYNC_OPE, LED_STATE_SUB_BACKILM );
	
	return ret;
}


static int led_func_c_5( struct ncmlight_state_t *state, int type )
{
	int ret = 0;
	u8	*set_patern = NULL;
	
	printk(KERN_INFO "[leds_data_input]led_func_c_5 type = %d \n", type);
	
	ret = led_thread_stop( type );
//	ret |= led_patern_data_del( type );
	set_patern = led_patern_data_switch( type );
	ret |= led_run_stop( type );
	
	led_active_flag_ctr( LED_FLAG_ON, LED_TYPE_INCOMING );
	set_patern = led_patern_data_switch( LED_TYPE_INCOMING );
	led_wake_up_thread( LED_TYPE_INCOMING );
	
	if( set_patern == NULL ){
		led_state_set( type, LED_STATE_SYNC_OPE, LED_STATE_SUB_BACKILM );
		led_state_set_req( LED_TYPE_INCOMING, LED_STATE_SYNC_OPE, LED_STATE_SUB_BACKILM );
	}
	else{
		led_state_set( type, LED_STATE_SYNC_OPE, LED_STATE_SUB_BACKILM );
		led_state_set_req( LED_TYPE_INCOMING, LED_STATE_SYNC_OPE, LED_STATE_SUB_BACKILM );
		led_thread_create( LED_TYPE_INCOMING );
	}
	return ret;
}

static int led_func_d_4( struct ncmlight_state_t *state, int type )
{
	int ret = 0;
	u8	*set_patern = NULL;
	
	printk(KERN_INFO "[leds_data_input]led_func_d_4 type = %d \n", type);
	
//	ret = led_thread_stop( type );
	ret |= led_patern_data_del( type );
	set_patern = led_patern_data_switch( type );
	ret |= led_run_stop( type );
	
	if( set_patern == NULL ){
		led_state_set( type, LED_STATE_STOP, LED_STATE_SUB_BACKILM );
//		led_state_set_req( LED_TYPE_INCOMING, LED_STATE_STOP, 0 );
	}
	else{
		led_state_set( type, LED_STATE_PATERN, 0 );
//		led_state_set_req( LED_TYPE_INCOMING, LED_STATE_STOP, 0 );
	}
	return ret;
}

static int led_func_d_5( struct ncmlight_state_t *state, int type )
{
	int ret = 0;
	u8	*set_patern = NULL;
	
	printk(KERN_INFO "[leds_data_input]led_func_d_5 type = %d \n", type);
	
	ret = led_thread_stop( type );
	ret |= led_patern_data_del( type );
	set_patern = led_patern_data_switch( type );
	ret |= led_run_stop( type );
	
	if( set_patern == NULL ){
		led_state_set( type, LED_STATE_SYNC_OPE_CANCEL, LED_STATE_SUB_BACKILM_STOP );
		led_state_set_req( LED_TYPE_INCOMING, LED_STATE_SYNC_OPE_CANCEL, LED_STATE_SUB_BACKILM_STOP );
	}
	else{
		led_state_set( type, LED_STATE_SYNC_OPE_CANCEL, LED_STATE_SUB_BACKILM_WAIT );
		led_state_set_req( LED_TYPE_INCOMING, LED_STATE_SYNC_OPE_CANCEL, LED_STATE_SUB_BACKILM_WAIT );
	}
	return ret;
}


static int led_func_e_5( struct ncmlight_state_t *state, int type )
{
	int ret = 0;
	u8	*set_patern = NULL;
	
	printk(KERN_INFO "[leds_data_input]led_func_e_5 type = %d, .ilmback.sub_state = %d \n", type, gs_led_state.ilmback.sub_state);
	
	switch( gs_led_state.ilmback.sub_state ){
	 case LED_STATE_SUB_BACKILM_STOP:
		break;
		
	 case LED_STATE_SUB_BACKILM_WAIT:
		break;
		
	 case LED_STATE_SUB_BACKILM_RUN:
		ret = led_thread_stop( type );
		ret |= led_patern_data_del( type );
		set_patern = led_patern_data_switch( type );
		ret |= led_run_stop( type );
		
		if( set_patern == NULL ){
			led_state_set( type, LED_STATE_SYNC_OPE_CANCEL, LED_STATE_SUB_BACKILM_STOP );
			led_state_set_req( LED_TYPE_INCOMING, LED_STATE_SYNC_OPE_CANCEL, LED_STATE_SUB_BACKILM_STOP );
		}
		else{
			led_state_set( type, LED_STATE_SYNC_OPE_CANCEL, LED_STATE_SUB_BACKILM_WAIT );
			led_state_set_req( LED_TYPE_INCOMING, LED_STATE_SYNC_OPE_CANCEL, LED_STATE_SUB_BACKILM_WAIT );
		}
		break;
		
	  default:
		ret = led_func_error( state, type );
		break;
	}
	return ret;
}

static int led_func_camera_stop( struct ncmlight_state_t *state, int type )
{
	int ret = 0;
	u8	*set_patern = NULL;
	
	printk(KERN_INFO "[leds_data_input]led_func_camera_stop type = %d \n", type);
	
	ret = led_thread_stop( LED_TYPE_BACK );
	ret |= led_patern_data_del( LED_TYPE_BACK );
	set_patern = led_patern_data_switch( LED_TYPE_BACK );
	ret |= led_run_stop( LED_TYPE_BACK );
	
	if( set_patern == NULL ){
		led_state_set( LED_TYPE_CAMERA, LED_STATE_CAM_START, 0 );
	}
	else{
		led_state_set( LED_TYPE_CAMERA, LED_STATE_CAM_PATERN, 0 );
	}
	
	return ret;
}

static int led_func_camera_end( struct ncmlight_state_t *state, int type )
{
	int ret = 0;
	
	printk(KERN_INFO "[leds_data_input]led_func_camera_end IN type = %d, .main_state = %d, .sub_state \n", type, gs_led_state.ilmback.main_state, gs_led_state.ilmback.sub_state);
	
	led_run_stop( LED_TYPE_BACK );
	
	if( gs_patern_data.back.s_set_data.p_patern != NULL ){
		kfree( gs_patern_data.back.s_set_data.p_patern );
		gs_patern_data.back.s_set_data.p_patern = NULL;
	}
	
	if( gs_patern_data.back.s_ready_data.p_patern != NULL ){
		kfree( gs_patern_data.back.s_ready_data.p_patern );
		gs_patern_data.back.s_ready_data.p_patern = NULL;
	}
	
//	memcpy( &gs_patern_data.back, &gs_patern_data.back_keep, sizeof(led_patern_data_type) );

	gs_patern_data.back.s_set_data.p_patern = gs_patern_data.back_keep.s_set_data.p_patern ;
	memcpy( &gs_patern_data.back.s_set_data.s_data_info, &gs_patern_data.back_keep.s_set_data.s_data_info, sizeof(input_data_head_type) );
	
	gs_patern_data.back.s_ready_data.p_patern = gs_patern_data.back_keep.s_ready_data.p_patern ;
	memcpy( &gs_patern_data.back.s_ready_data.s_data_info, &gs_patern_data.back_keep.s_ready_data.s_data_info, sizeof(input_data_head_type) );
	
	gs_patern_data.back.frame_cnt = 0 ;
	gs_patern_data.back.loop_flag = gs_patern_data.back_keep.loop_flag ;
	gs_patern_data.back.loop_cnt = gs_patern_data.back_keep.loop_cnt ;
	gs_patern_data.back.check_flag = 0 ;
	
	led_state_set( LED_TYPE_CAMERA, LED_STATE_CAM_END, 0 );
	g_camera_flag = LED_FLAG_OFF ;
	
	if( gs_led_state.ilm3led.main_state == LED_STATE_SYNC_OPE ) {
		led_wake_up_thread( LED_TYPE_INCOMING );
		led_thread_create( LED_TYPE_INCOMING );
	}
	
	if( ( gs_led_state.ilmback.main_state == LED_STATE_LOCAL_OPE ) ||
		( ( gs_led_state.ilmback.main_state == LED_STATE_SYNC_OPE_CANCEL ) && ( gs_led_state.ilmback.sub_state == LED_STATE_SUB_BACKILM_RUN ) ) )
	{
		led_active_flag_ctr( LED_FLAG_ON, LED_TYPE_BACK );
		led_wake_up_thread( LED_TYPE_BACK );
		ret = led_thread_create( LED_TYPE_BACK );
	} 
	else {
		led_thread_stop( LED_TYPE_BACK );
	}
	
	return ret;
}

static int led_func_camera_0_0( struct ncmlight_state_t *state, int type )
{
	int ret = 0;
	u8	*set_patern = NULL;
	
	printk(KERN_INFO "[leds_data_input]led_func_camera_0_0 type = %d \n", type);

	g_camera_flag = LED_FLAG_ON;
	
	ret = led_thread_stop( LED_TYPE_BACK );
	ret |= led_run_stop( LED_TYPE_BACK );
	
	
	if( ( ( gs_led_state.ilmback.main_state == LED_STATE_LOCAL_OPE ) ||
//	      ( gs_led_state.ilmback.main_state == LED_STATE_SYNC_OPE ) ||
	      ( ( gs_led_state.ilmback.main_state == LED_STATE_SYNC_OPE_CANCEL ) && ( gs_led_state.ilmback.sub_state == LED_STATE_SUB_BACKILM_RUN ) ) ) &&
	    ( gs_patern_data.back.loop_flag == LED_FLAG_OFF ) ) {
		
		ret |= led_patern_data_del( LED_TYPE_BACK );
		set_patern = led_patern_data_switch( LED_TYPE_BACK );
		
		if( set_patern == NULL ){
			if( gs_led_state.ilmback.main_state != LED_STATE_SYNC_OPE_CANCEL ) {
				led_state_set( LED_TYPE_BACK, LED_STATE_STOP, 0 );
			}
			else {
				led_state_set( LED_TYPE_BACK, LED_STATE_SYNC_OPE_CANCEL, LED_STATE_SUB_BACKILM_STOP );
			}
		}
		else{
			if( gs_led_state.ilmback.main_state != LED_STATE_SYNC_OPE_CANCEL ) {
				led_state_set( LED_TYPE_BACK, LED_STATE_PATERN, 0 );
			}
			else {
				led_state_set( LED_TYPE_BACK, LED_STATE_SYNC_OPE_CANCEL, LED_STATE_SUB_BACKILM_WAIT );
			}
		}
	}
	
	
//	memcpy(  &gs_patern_data.back_keep, &gs_patern_data.back, sizeof(led_patern_data_type) );
	
	gs_patern_data.back_keep.s_set_data.p_patern = gs_patern_data.back.s_set_data.p_patern ;
	gs_patern_data.back.s_set_data.p_patern = NULL;
	memcpy( &gs_patern_data.back_keep.s_set_data.s_data_info, &gs_patern_data.back.s_set_data.s_data_info, sizeof(input_data_head_type) );
	
	gs_patern_data.back_keep.s_ready_data.p_patern = gs_patern_data.back.s_ready_data.p_patern ;
	gs_patern_data.back.s_ready_data.p_patern = NULL;
	memcpy( &gs_patern_data.back_keep.s_ready_data.s_data_info, &gs_patern_data.back.s_ready_data.s_data_info, sizeof(input_data_head_type) );
	
	gs_patern_data.back_keep.frame_cnt = 0 ;
	gs_patern_data.back_keep.loop_flag = gs_patern_data.back.loop_flag ;
	gs_patern_data.back_keep.loop_cnt = gs_patern_data.back.loop_cnt ;
	gs_patern_data.back_keep.check_flag = 0 ;
	
	led_state_set( LED_TYPE_CAMERA, LED_STATE_CAM_START, 0 );
	
	return ret;
}

static int led_func_camera_2_1( struct ncmlight_state_t *state, int type )
{
	int ret = 0;
	
	printk(KERN_INFO "[leds_data_input]led_func_camera_2_1 type = %d \n", type);
	led_state_set( LED_TYPE_CAMERA, LED_STATE_CAM_PATERN, 0 );
	
	return ret;
}

static int led_func_camera_3_2( struct ncmlight_state_t *state, int type )
{
	int ret = 0;
	u8	*set_patern = NULL;
	
	printk(KERN_INFO "[leds_data_input]led_func_camera_3_2 type = %d \n", type);
	
	led_loop_set( state->flashOnMS, LED_TYPE_BACK );
	
	led_active_flag_ctr( LED_FLAG_ON, LED_TYPE_BACK );
	set_patern = led_patern_data_switch( LED_TYPE_BACK );
	led_wake_up_thread( LED_TYPE_BACK );
	
	if( set_patern == NULL ){
		ret = LED_DATA_INPUT_NG;
	}
	else {
		ret = led_run_loop( LED_TYPE_BACK );
		led_state_set( LED_TYPE_CAMERA, LED_STATE_CAM_RUN, 0 );
	}
	
	return ret;
}

static int led_func_camera_3_3( struct ncmlight_state_t *state, int type )
{
	int ret = 0;
	u8	*set_patern = NULL;
	
	printk(KERN_INFO "[leds_data_input]led_func_camera_3_3 type = %d \n", type);
	
	led_loop_set( state->flashOnMS, LED_TYPE_BACK );
	
	led_active_flag_ctr( LED_FLAG_ON, LED_TYPE_BACK );
	set_patern = led_patern_data_switch( LED_TYPE_BACK );
	led_wake_up_thread( LED_TYPE_BACK );

	return ret;
}

#endif /* #ifdef CONFIG_FEATURE_NCMC_PEACOCK */


/*----------------------------------------------------------------------------
* MODULE   : led_lights_3led
* FUNCTTION: 
* NOTE     : None
* RETURN   : 
* CREATE   : 
* UPDATE   : 
----------------------------------------------------------------------------*/
static int led_lights_3led(struct ncmlight_state_t *u_state)
{
	int err = 0;
	struct ncmlight_state_t state;
	int type = LED_TYPE_INCOMING;
	

	err = copy_from_user( &state, u_state, sizeof(struct ncmlight_state_t) );
	if( err ){
		printk(KERN_INFO "[leds_data_input]led_lights_3led state read NG\n");
		return -EINVAL;
	}
	
	if( state.color == 0 ){
		err = led_ilm_func_State[3][gs_led_state.ilm3led.main_state](&state, type);
	}
#ifdef CONFIG_FEATURE_NCMC_PEACOCK
	else if( state.brightnessMode == 1 ){		/* NCM_ILM_SYNC_ON */
		led_loop_set( state.flashOnMS, type );
		err = led_ilm_func_State[2][gs_led_state.ilm3led.main_state](&state, type);
	}
	else if( state.brightnessMode == 2 ){		/* NCM_ILM_SYNC_OFF */
		led_loop_set( state.flashOnMS, type );
		err = led_ilm_func_State[1][gs_led_state.ilm3led.main_state](&state, type);
	}
#else /* #ifdef CONFIG_FEATURE_NCMC_PEACOCK */
	else{
		led_loop_set( state.flashOnMS, type );
		err = led_ilm_func_State[1][gs_led_state.ilm3led.main_state](&state, type);
	}
#endif /* #ifdef CONFIG_FEATURE_NCMC_PEACOCK */


    return err;
}

/*----------------------------------------------------------------------------
* MODULE   : led_ncmlights_keyilm
* FUNCTTION: 
* NOTE     : None
* RETURN   : 
* CREATE   : 
* UPDATE   : 
----------------------------------------------------------------------------*/
static int led_ncmlights_keyilm(struct ncmlight_state_t *u_state)
{
#if defined(CONFIG_FEATURE_NCMC_G121S) || defined(CONFIG_FEATURE_NCMC_D121M)
    return 0;
#else /* #if defined(CONFIG_FEATURE_NCMC_G121S) || defined(CONFIG_FEATURE_NCMC_D121M) */
	int err = 0;
	struct ncmlight_state_t state;
	int type = LED_TYPE_FRONT;
	
	
	err = copy_from_user( &state, u_state, sizeof(struct ncmlight_state_t) );
	if( err ){
		printk(KERN_INFO "[leds_data_input]led_ncmlights_keyilm state read NG\n");
		return -EINVAL;
	}
	
	led_loop_set( state.flashOnMS, type );
	
	if( state.flashMode == 2 ){				/* NCM_ILM_ON */
		err = led_ilm_func_State[6][gs_led_state.ilmkey.main_state](&state, type);
	}
	else if( state.flashMode == 3 ){		/* NCM_ILM_OFF */
		err = led_ilm_func_State[7][gs_led_state.ilmkey.main_state](&state, type);
	}
	
	
    return err;
#endif /* #if defined(CONFIG_FEATURE_NCMC_G121S) || defined(CONFIG_FEATURE_NCMC_D121M) */
}

#ifdef CONFIG_FEATURE_NCMC_PEACOCK
/*----------------------------------------------------------------------------
* MODULE   : led_ncmlights_backilm
* FUNCTTION: 
* NOTE     : None
* RETURN   : 
* CREATE   : 
* UPDATE   : 
----------------------------------------------------------------------------*/
static int led_ncmlights_backilm(struct ncmlight_state_t *u_state)
{
	int err = 0;
	struct ncmlight_state_t state;
	int type = LED_TYPE_BACK;
	int camera_state = LED_STATE_STOP;
	int camera_sub_state = LED_STATE_SUB_BACKILM;
	u8	*set_patern = NULL;
	
	
	err = copy_from_user( &state, u_state, sizeof(struct ncmlight_state_t) );
	if( err ){
		printk(KERN_INFO "[leds_data_input]led_ncmlights_backilm state read NG\n");
		return -EINVAL;
	}
	
	if( g_camera_flag != LED_FLAG_ON ) {
		if( state.flashMode == 2 ){				/* NCM_ILM_ON */
			led_loop_set( state.flashOnMS, type );
			if( state.brightnessMode == 1 ){			/* NCM_ILM_SYNC_ON */
				err = led_ilm_func_State[11][gs_led_state.ilmback.main_state](&state, type);
			}
			else if( state.brightnessMode == 2 ){		/* NCM_ILM_SYNC_OFF */
				err = led_ilm_func_State[10][gs_led_state.ilmback.main_state](&state, type);
			}
			else if( state.brightnessMode == 3 ){		/* NCM_ILM_SYNC_RE_ON */
				err = led_ilm_func_State[12][gs_led_state.ilmback.main_state](&state, type);
			}
		}
		else if( state.flashMode == 3 ){		/* NCM_ILM_OFF */
			err = led_ilm_func_State[13][gs_led_state.ilmback.main_state](&state, type);
		}
	}
	else {
		
		if( state.flashMode == 2 ){	/* NCM_ILM_ON */
			
			led_loop_set(state.flashOnMS, LED_TYPE_CAMERA);
			
			if( state.brightnessMode == 1 ){			/* NCM_ILM_SYNC_ON */
				printk(KERN_INFO "[leds_data_input]led_ncmlights_backilm NCM_ILM_SYNC_ON \n");
				
				camera_state = led_back_keep_State[2][gs_led_state.ilmback.main_state];
				
				if( camera_state == LED_STATE_SYNC_OPE_WAIT ) {
					err = led_func_b_1( &state, LED_TYPE_BACK );
					
					return err;
				}
			}
			else if( state.brightnessMode == 2 ){		/* NCM_ILM_SYNC_OFF */
			
				printk(KERN_INFO "[leds_data_input]led_ncmlights_backilm NCM_ILM_SYNC_OFF \n");
			
				camera_state = led_back_keep_State[1][gs_led_state.ilmback.main_state];
			
				if( camera_state == LED_STATE_SYNC_OPE_CANCEL ) {
					
					camera_sub_state = LED_STATE_SUB_BACKILM_RUN;
					
					if( gs_led_state.ilmback.main_state == LED_STATE_SYNC_OPE ){
						led_patern_data_del( LED_TYPE_CAMERA );
						
						set_patern = led_patern_data_switch( LED_TYPE_CAMERA );
						
						if( set_patern != NULL ){
							
							if( state.flashOnMS > 0 ) {
								led_patern_data_del( LED_TYPE_CAMERA );
								camera_sub_state = LED_STATE_SUB_BACKILM_STOP;
							}
						}
						else {
							camera_sub_state = LED_STATE_SUB_BACKILM_STOP;
						}
					}
					else if( gs_led_state.ilmback.sub_state == LED_STATE_SUB_BACKILM_RUN ) {
						
						led_patern_data_switch( LED_TYPE_CAMERA );
						
						if( state.flashOnMS > 0 ) {
							led_patern_data_del( LED_TYPE_CAMERA );
							
							set_patern = led_patern_data_switch( LED_TYPE_CAMERA );
							
							if( set_patern != NULL ) {
							
								camera_sub_state = LED_STATE_SUB_BACKILM_WAIT;
							}
							else {
								camera_sub_state = LED_STATE_SUB_BACKILM_STOP;
							}
						}
					}
					else if( gs_led_state.ilmback.sub_state == LED_STATE_SUB_BACKILM_WAIT ) {
					
						if( state.flashOnMS > 0 ) {
							led_patern_data_del( LED_TYPE_CAMERA );
							
							set_patern = led_patern_data_switch( LED_TYPE_CAMERA );
							
							if( set_patern != NULL ) {
							
								camera_sub_state = LED_STATE_SUB_BACKILM_WAIT;
							}
							else {
								camera_sub_state = LED_STATE_SUB_BACKILM_STOP;
							}
						}
					}
					else {
						camera_state = LED_STATE_ERR ;
					}
				}
				else if( camera_state == LED_STATE_LOCAL_OPE ) {
				
					if( gs_led_state.ilmback.main_state == LED_STATE_LOCAL_OPE ) {
					
						led_patern_data_switch( LED_TYPE_CAMERA );
					}
//					set_patern = led_patern_data_switch( LED_TYPE_CAMERA );
					if( state.flashOnMS > 0 ) {
						led_patern_data_del( LED_TYPE_CAMERA );
						set_patern = led_patern_data_switch( LED_TYPE_CAMERA );
						
						if( set_patern != NULL ){
							camera_state = LED_STATE_PATERN ;
						}
						else {
							camera_state = LED_STATE_STOP ;
						}
					}
				}
			}
			else if( state.brightnessMode == 3 ){		/* NCM_ILM_SYNC_RE_ON */
				
				printk(KERN_INFO "[leds_data_input]led_ncmlights_backilm NCM_ILM_SYNC_RE_ON \n");
				
				camera_state = led_back_keep_State[3][gs_led_state.ilmback.main_state];
				
				if( camera_state == LED_STATE_SYNC_OPE ) {
					led_state_set_req( LED_TYPE_INCOMING, LED_STATE_SYNC_OPE, LED_STATE_SUB_BACKILM );
				}
			}
			else {
				camera_state = LED_STATE_ERR ;
			}
		}
		else if( state.flashMode == 3 )
		{
			printk(KERN_INFO "[leds_data_input]led_ncmlights_backilm STOP\n");
			
			camera_state = led_back_keep_State[4][gs_led_state.ilmback.main_state];
			
			led_patern_data_del( LED_TYPE_CAMERA );
			
			set_patern = led_patern_data_switch( LED_TYPE_CAMERA );
			
			if( set_patern != NULL ) {
				
				if( camera_state == LED_STATE_SYNC_OPE_CANCEL ){
					camera_sub_state = LED_STATE_SUB_BACKILM_WAIT ;
				}
				else if( camera_state == LED_STATE_STOP ) {
					camera_state = LED_STATE_PATERN ;
				}
			}
			else if( camera_state == LED_STATE_SYNC_OPE_CANCEL ) {
				camera_sub_state = LED_STATE_SUB_BACKILM_STOP ;
			}
		}
		else {
			camera_state = LED_STATE_ERR ;
		}
		
		if( camera_state == LED_STATE_OK )  {
			err = LED_DATA_INPUT_OK ;
		}
		else if( camera_state == LED_STATE_ERR ) {
			err = LED_DATA_INPUT_NG ;
		}
		else {
			led_state_set( LED_TYPE_BACK, camera_state, camera_sub_state );
			if( camera_state == LED_STATE_SYNC_OPE_CANCEL ){
				led_state_set_req( LED_TYPE_INCOMING, LED_STATE_SYNC_OPE_CANCEL, camera_sub_state );
			}
			err = LED_DATA_INPUT_OK ;
		}
	}
	

    return err;
}

/*----------------------------------------------------------------------------
* MODULE   : led_ncmlights_
* FUNCTTION: 
* NOTE     : None
* RETURN   : 
* CREATE   : 
* UPDATE   : 
----------------------------------------------------------------------------*/
static int led_ncmlights_camera(struct ncmlight_state_t *u_state)
{
	int err = 0;
	struct ncmlight_state_t state;
	int type = LED_TYPE_CAMERA;
	
	
	err = copy_from_user( &state, u_state, sizeof(struct ncmlight_state_t) );
	if( err ){
		printk(KERN_INFO "[leds_data_input]led_ncmlights_camera state read NG\n");
		return -EINVAL;
	}
	
	
	if( state.flashMode == 4 ){				/* NCM_CAM_CTL_START */
		err = led_ilm_camera_func_State[0][gs_led_state.camera.main_state](&state, type);
	}
	else if( state.flashMode == 5 ){		/* NCM_CAM_CTL_END */
		err = led_ilm_camera_func_State[1][gs_led_state.camera.main_state](&state, type);
	}
	else if( state.flashMode == 7 ){		/* NCM_ILM_CAM_ON */
		err = led_ilm_camera_func_State[3][gs_led_state.camera.main_state](&state, type);
	}
	else if( state.flashMode == 8 ){		/* NCM_ILM_CAM_OFF */
		err = led_ilm_camera_func_State[4][gs_led_state.camera.main_state](&state, type);
	}
	else {
	    printk(KERN_INFO "[leds_data_input]led_ncmlights_backilm Err flashMode = %d\n", state.flashMode );
		return err;
	}
	
	
    return err;
}
#endif /* #ifdef CONFIG_FEATURE_NCMC_PEACOCK */

#ifdef LOCAL_CONFIG_FEATURE_NCMC_LED_12_1ST
/*----------------------------------------------------------------------------
* MODULE   : led_lights_3led_file_ctrl
* FUNCTTION: 
* NOTE     : None
* RETURN   : 
* CREATE   : 
* UPDATE   : 
----------------------------------------------------------------------------*/
static int led_lights_3led_file_ctrl(struct ncmlight_state_t *u_state) {
	
	int err = 0;
	struct ncmlight_state_t state;
	void *p_patern_data = NULL;
	int	patern_size = 0;
	

	err = copy_from_user( &state, u_state, sizeof(struct ncmlight_state_t) );
	if( err ){
		return -EINVAL;
	}
	

	switch(state.flashMode){
	
	  case LED_TYPE_N_KEEP:
	  
		if( g_notifications_patern.p_patern != NULL ){
			kfree( g_notifications_patern.p_patern );
			g_notifications_patern.p_patern = NULL;
		}
		
		if( gs_patern_data.incoming.s_ready_data.p_patern != NULL ) {
			p_patern_data = gs_patern_data.incoming.s_ready_data.p_patern;
			patern_size = gs_patern_data.incoming.s_ready_data.s_data_info.data_size;
			memcpy( &g_notifications_patern.s_data_info, &gs_patern_data.incoming.s_ready_data.s_data_info, sizeof(input_data_head_type) );
		}
		else
		{
			p_patern_data = gs_patern_data.incoming.s_set_data.p_patern;
			patern_size = gs_patern_data.incoming.s_set_data.s_data_info.data_size;
			memcpy( &g_notifications_patern.s_data_info, &gs_patern_data.incoming.s_set_data.s_data_info, sizeof(input_data_head_type) );
		}
		
		if( p_patern_data != NULL ){
			g_notifications_patern.p_patern = kzalloc( patern_size, GFP_KERNEL );
			if( g_notifications_patern.p_patern == NULL ){
				return -EINVAL;
			}
			memcpy( g_notifications_patern.p_patern, p_patern_data, patern_size );
		}
		
		break;
		
	  case LED_TYPE_N_SET:
		
		if( g_notifications_patern.p_patern != NULL ){
			
			p_patern_data = kzalloc( g_notifications_patern.s_data_info.data_size, GFP_KERNEL );
			
			if( p_patern_data == NULL ){
				return -EINVAL;
			}
			
			memcpy( p_patern_data, g_notifications_patern.p_patern, g_notifications_patern.s_data_info.data_size );
			
			if( gs_patern_data.incoming.s_set_data.p_patern == NULL ){
				gs_patern_data.incoming.s_set_data.p_patern = p_patern_data;
				memcpy( &gs_patern_data.incoming.s_set_data.s_data_info, &g_notifications_patern.s_data_info, sizeof(input_data_head_type) );
			}
			else{
				if( gs_patern_data.incoming.s_ready_data.p_patern != NULL ){
					kfree( gs_patern_data.incoming.s_ready_data.p_patern );
				}
				gs_patern_data.incoming.s_ready_data.p_patern = p_patern_data ;
				memcpy( &gs_patern_data.incoming.s_ready_data.s_data_info, &g_notifications_patern.s_data_info, sizeof(input_data_head_type) );
			}
			
			err = led_ilm_func_State[0][gs_led_state.ilm3led.main_state](&state, LED_TYPE_INCOMING);
		}
		
		break;
		
	  case LED_TYPE_B_KEEP:

		if( g_battery_patern.p_patern != NULL ){
			kfree( g_battery_patern.p_patern );
			g_battery_patern.p_patern = NULL;
		}

		if( gs_patern_data.incoming.s_ready_data.p_patern != NULL ) {
			p_patern_data = gs_patern_data.incoming.s_ready_data.p_patern;
			patern_size = gs_patern_data.incoming.s_ready_data.s_data_info.data_size;
			memcpy( &g_battery_patern.s_data_info, &gs_patern_data.incoming.s_ready_data.s_data_info, sizeof(input_data_head_type) );
		}
		else
		{
			p_patern_data = gs_patern_data.incoming.s_set_data.p_patern;
			patern_size = gs_patern_data.incoming.s_set_data.s_data_info.data_size;
			memcpy( &g_battery_patern.s_data_info, &gs_patern_data.incoming.s_set_data.s_data_info, sizeof(input_data_head_type) );
		}
		
		if( p_patern_data != NULL ){
			g_battery_patern.p_patern = kzalloc( patern_size, GFP_KERNEL );
			if( g_battery_patern.p_patern == NULL ){
				return -EINVAL;
			}
			memcpy( g_battery_patern.p_patern, p_patern_data, patern_size );
		}
		
		break;
		
	  case LED_TYPE_B_SET:
	  
		if( g_battery_patern.p_patern != NULL ){
			
			p_patern_data = kzalloc( g_battery_patern.s_data_info.data_size, GFP_KERNEL );
			
			if( p_patern_data == NULL ){
				return -EINVAL;
			}
			
			memcpy( p_patern_data, g_battery_patern.p_patern, g_battery_patern.s_data_info.data_size );
			
			if( gs_patern_data.incoming.s_set_data.p_patern == NULL ){
				gs_patern_data.incoming.s_set_data.p_patern = p_patern_data;
				memcpy( &gs_patern_data.incoming.s_set_data.s_data_info, &g_battery_patern.s_data_info, sizeof(input_data_head_type) );
			}
			else{
				if( gs_patern_data.incoming.s_ready_data.p_patern != NULL ){
					kfree( gs_patern_data.incoming.s_ready_data.p_patern );
				}
				gs_patern_data.incoming.s_ready_data.p_patern = p_patern_data ;
				memcpy( &gs_patern_data.incoming.s_ready_data.s_data_info, &g_battery_patern.s_data_info, sizeof(input_data_head_type) );
			}
			
			err = led_ilm_func_State[0][gs_led_state.ilm3led.main_state](&state, LED_TYPE_INCOMING);
		}

		break;
	
	  default:
        err = -EINVAL;
        break;
    }
	
    return err;
}
#endif /* #ifdef LOCAL_CONFIG_FEATURE_NCMC_LED_12_1ST */
/*----------------------------------------------------------------------------
* MODULE   : led_data_input_ioctl
* FUNCTTION: 
* NOTE     : None
* RETURN   : 
* CREATE   : 
* UPDATE   : 
----------------------------------------------------------------------------*/
#if defined(LOCAL_CONFIG_FEATURE_NCMC_LED_12_1ST)
static long led_data_input_ioctl(struct file *file, unsigned int iocmd, unsigned long data)
#else /* #if defined(LOCAL_CONFIG_FEATURE_NCMC_LED_12_1ST) */
static int led_data_input_ioctl(struct inode *inode, struct file *file,
    unsigned int iocmd, unsigned long data)
#endif /* #if defined(LOCAL_CONFIG_FEATURE_NCMC_LED_12_1ST) */
{
	int ret = 0;
	struct input_data_head_struct *sp_head_data;
	
//    printk(KERN_INFO "[leds_data_input]led_data_input_ioctl IN iocmd = %d\n", iocmd);
//    printk(KERN_INFO "[leds_data_input]ilm3led_state = %d ilmkey_state = %d \n", gs_led_state.ilm3led.main_state, gs_led_state.ilmkey.main_state);
    printk(KERN_INFO "[leds_data_input]led_data_input_ioctl IN iocmd = %d ilm3led_state = %d ilmkey_state = %d \n", iocmd, gs_led_state.ilm3led.main_state, gs_led_state.ilmkey.main_state);
    
#ifdef CONFIG_FEATURE_NCMC_PEACOCK
    printk(KERN_INFO "[leds_data_input]ilmback_state = %d  camera_state = %d\n", gs_led_state.ilmback.main_state, gs_led_state.camera.main_state );
	mutex_lock( &gs_patern_data.func_lock );
	led_state_set_chk(LED_TYPE_INCOMING);
	led_state_set_chk(LED_TYPE_BACK);
	mutex_unlock( &gs_patern_data.func_lock );
#endif /* #ifdef CONFIG_FEATURE_NCMC_PEACOCK */

    switch(iocmd){
	  case LED_IOC_LIGHTS_3LED:
		mutex_lock( &gs_patern_data.func_lock );
		ret = led_lights_3led((struct ncmlight_state_t *)data);
		mutex_unlock( &gs_patern_data.func_lock );
		break;
		
//#if defined(CONFIG_FEATURE_NCMC_G121S) || defined(CONFIG_FEATURE_NCMC_D121M) || defined(CONFIG_FEATURE_NCMC_G121T) || defined(CONFIG_FEATURE_NCMC_G121U)
//#else
	  case LED_IOC_NCMLIGHTS_KEYILM:
		mutex_lock( &gs_patern_data.func_lock );
		ret = led_ncmlights_keyilm((struct ncmlight_state_t *)data);
		mutex_unlock( &gs_patern_data.func_lock );
		break;
//#endif /* #if defined(CONFIG_FEATURE_NCMC_G121S) || defined(CONFIG_FEATURE_NCMC_D121M) || defined(CONFIG_FEATURE_NCMC_G121T) || defined(CONFIG_FEATURE_NCMC_G121U) */
		
#ifdef LOCAL_CONFIG_FEATURE_NCMC_LED_12_1ST
	  case LED_IOC_FILE_CTRL:
		mutex_lock( &gs_patern_data.func_lock );
		ret = led_lights_3led_file_ctrl((struct ncmlight_state_t *)data);
		mutex_unlock( &gs_patern_data.func_lock );
		break;
#endif /* #ifdef LOCAL_CONFIG_FEATURE_NCMC_LED_12_1ST */

#ifdef CONFIG_FEATURE_NCMC_PEACOCK
	  case LED_IOC_NCMLIGHTS_BACKILM:
		mutex_lock( &gs_patern_data.func_lock );
		ret = led_ncmlights_backilm((struct ncmlight_state_t *)data);
		mutex_unlock( &gs_patern_data.func_lock );
		break;
		
	  case LED_IOC_NCMLIGHTS_CAMERA:
		mutex_lock( &gs_patern_data.func_lock );
		ret = led_ncmlights_camera((struct ncmlight_state_t *)data);
		mutex_unlock( &gs_patern_data.func_lock );
		break;
#endif /* #ifdef CONFIG_FEATURE_NCMC_PEACOCK */
		
	  case LED_IOC_DATA_SET:
		sp_head_data = (struct input_data_head_struct *)data;
		if(sp_head_data->led_type == LED_TYPE_INCOMING ){
			mutex_lock( &gs_patern_data.func_lock );
			ret = led_patern_data_read( (unsigned char*)data );
			mutex_unlock( &gs_patern_data.func_lock );
		}
//#if defined(CONFIG_FEATURE_NCMC_G121S) || defined(CONFIG_FEATURE_NCMC_D121M) || defined(CONFIG_FEATURE_NCMC_G121T) || defined(CONFIG_FEATURE_NCMC_G121U)
//#else
		else if(sp_head_data->led_type == LED_TYPE_FRONT ){
			mutex_lock( &gs_patern_data.func_lock );
			ret = led_patern_data_read( (unsigned char*)data );
			mutex_unlock( &gs_patern_data.func_lock );
		}
//#endif /* #if defined(CONFIG_FEATURE_NCMC_G121S) || defined(CONFIG_FEATURE_NCMC_D121M) || defined(CONFIG_FEATURE_NCMC_G121T) || defined(CONFIG_FEATURE_NCMC_G121U) */
#ifdef CONFIG_FEATURE_NCMC_PEACOCK
		else if(sp_head_data->led_type == LED_TYPE_BACK ){
			mutex_lock( &gs_patern_data.func_lock );
			ret = led_patern_data_read( (unsigned char*)data );
			mutex_unlock( &gs_patern_data.func_lock );
		}
		else if(sp_head_data->led_type == LED_TYPE_CAMERA ){
			mutex_lock( &gs_patern_data.func_lock );
			ret = led_patern_data_read( (unsigned char*)data );
			mutex_unlock( &gs_patern_data.func_lock );
		}
#endif /* #ifdef CONFIG_FEATURE_NCMC_PEACOCK */
        break;

	  default:
        ret = -EINVAL;
        break;
    }
	
//    printk(KERN_INFO "[leds_data_input]ilm3led_state = %d ilmkey_state = %d \n", gs_led_state.ilm3led.main_state, gs_led_state.ilmkey.main_state);
    
#ifdef CONFIG_FEATURE_NCMC_PEACOCK
    printk(KERN_INFO "[leds_data_input]ilmback_state = %d  camera_state = %d\n", gs_led_state.ilmback.main_state, gs_led_state.camera.main_state );
#endif /* #ifdef CONFIG_FEATURE_NCMC_PEACOCK */
	
//    printk(KERN_DEBUG "[leds_data_input]led_data_input_ioctl OUT ret = %d\n", ret);
    printk(KERN_DEBUG "[leds_data_input]led_data_input_ioctl OUT ret = %d ilm3led_state = %d ilmkey_state = %d \n", ret, gs_led_state.ilm3led.main_state, gs_led_state.ilmkey.main_state);
   
    return ret;
}

/*----------------------------------------------------------------------------
* MODULE   : led_data_input_mmap
* FUNCTTION: 
* NOTE     : None
* RETURN   : 
* CREATE   : 
* UPDATE   : 
----------------------------------------------------------------------------*/
static int led_data_input_mmap(struct file *filp, struct vm_area_struct *vma)
{
	int ret = 0;
	unsigned long size;
	
	size = vma->vm_end - vma->vm_start ;
	ret = remap_pfn_range( vma, vma->vm_start, vma->vm_pgoff, size, vma->vm_page_prot );
	
	if( ret ){
		return -EAGAIN;
	}
	
	vma->vm_ops = &led_remap_vm_ops;
	
	led_vma_open(vma);
	return 0;
}

static const struct file_operations led_data_input_fops = {
    .owner      = THIS_MODULE,
    .open       = led_data_input_open,
#if defined(LOCAL_CONFIG_FEATURE_NCMC_LED_12_1ST)
    .unlocked_ioctl = led_data_input_ioctl,
#else /* #if defined(LOCAL_CONFIG_FEATURE_NCMC_LED_12_1ST) */
    .ioctl      = led_data_input_ioctl,
#endif /* #if defined(LOCAL_CONFIG_FEATURE_NCMC_LED_12_1ST) */
    .mmap       = led_data_input_mmap,
};

static struct miscdevice led_lights_data_input = {
    .fops       = &led_data_input_fops,
    .name       = LED_LIGHTS_INPUT_NAME,
    .minor      = MISC_DYNAMIC_MINOR,
};

static struct miscdevice led_ncmlights_data_input = {
    .fops       = &led_data_input_fops,
    .name       = LED_NCMLIGHTS_INPUT_NAME,
    .minor      = MISC_DYNAMIC_MINOR,
};

/*----------------------------------------------------------------------------
* MODULE   : led_data_input_init
* FUNCTTION: LED Driver init
* NOTE     : None
* RETURN   : 
* CREATE   : 
* UPDATE   : 
----------------------------------------------------------------------------*/
static int __init led_data_input_init(void)
{
	int ret;
	
	printk(KERN_INFO "[leds_data_input]led_data_input_ini \n");
	
	mutex_init( &gs_patern_data.func_lock );
	
	wake_lock_init(&led_ilm_wake_lock, WAKE_LOCK_SUSPEND, "leds_data_input");
	
	memset( &gs_patern_data.incoming, 0, sizeof(gs_patern_data.incoming) );
	mutex_init( &gs_patern_data.incoming.data_lock );
//	mutex_init( &gs_patern_data.incoming.func_lock );
	
	memset( &gs_patern_data.front, 0, sizeof(gs_patern_data.front) );
	mutex_init( &gs_patern_data.front.data_lock );
//	mutex_init( &gs_patern_data.front.func_lock );
	
#ifdef CONFIG_FEATURE_NCMC_PEACOCK
	memset( &gs_patern_data.back, 0, sizeof(gs_patern_data.back) );
	mutex_init( &gs_patern_data.back.data_lock );
	
	memset( &gs_patern_data.back_keep, 0, sizeof(gs_patern_data.back_keep) );
	mutex_init( &gs_patern_data.back_keep.data_lock );
//	mutex_init( &gs_patern_data.back.func_lock );
	g_camera_flag = LED_FLAG_OFF;
#endif /* #ifdef CONFIG_FEATURE_NCMC_PEACOCK */

#ifdef LOCAL_CONFIG_FEATURE_NCMC_LED_12_1ST
	memset( &g_notifications_patern, 0, sizeof(g_notifications_patern) );
	memset( &g_battery_patern, 0, sizeof(g_battery_patern) );
#endif /* #ifdef LOCAL_CONFIG_FEATURE_NCMC_LED_12_1ST */

	init_waitqueue_head(&gs_led_wq);

	ret = misc_register(&led_lights_data_input);
	if( ret != 0 ){
		return ret;
	}
	ret = misc_register(&led_ncmlights_data_input);
	if( ret != 0 ){
		misc_deregister(&led_lights_data_input);
	}
	return ret;
}
/*----------------------------------------------------------------------------
* MODULE   : platform_driver_unregister
* FUNCTTION: LED Driver exit
* NOTE     : None
* RETURN   : 
* CREATE   : 
* UPDATE   : 
----------------------------------------------------------------------------*/
static void __exit led_data_input_exit(void)
{
	
	misc_deregister(&led_lights_data_input);
	misc_deregister(&led_ncmlights_data_input);
	
	if( gs_patern_data.incoming.ps_thread != NULL ){
		kthread_stop( gs_patern_data.incoming.ps_thread );
	}
	
	if( gs_patern_data.front.ps_thread != NULL ){
		kthread_stop( gs_patern_data.front.ps_thread );
	}
	
	if( gs_patern_data.incoming.s_set_data.p_patern != NULL ){
		kfree( gs_patern_data.incoming.s_set_data.p_patern );
	}
	
	if( gs_patern_data.incoming.s_ready_data.p_patern != NULL ){
		kfree( gs_patern_data.incoming.s_ready_data.p_patern );
	}
	
	if( gs_patern_data.front.s_set_data.p_patern != NULL ){
		kfree( gs_patern_data.front.s_set_data.p_patern );
	}
	
	if( gs_patern_data.front.s_ready_data.p_patern != NULL ){
		kfree( gs_patern_data.front.s_ready_data.p_patern );
	}

#ifdef CONFIG_FEATURE_NCMC_PEACOCK
	if( gs_patern_data.back.ps_thread != NULL ){
		kthread_stop( gs_patern_data.back.ps_thread );
	}

	if( gs_patern_data.back.s_set_data.p_patern != NULL ){
		kfree( gs_patern_data.back.s_set_data.p_patern );
	}
	
	if( gs_patern_data.back.s_ready_data.p_patern != NULL ){
		kfree( gs_patern_data.back.s_ready_data.p_patern );
	}
	
	if( gs_patern_data.back_keep.s_set_data.p_patern != NULL ){
		kfree( gs_patern_data.back_keep.s_set_data.p_patern );
	}
	
	if( gs_patern_data.back_keep.s_ready_data.p_patern != NULL ){
		kfree( gs_patern_data.back_keep.s_ready_data.p_patern );
	}
	
#endif /* #ifdef CONFIG_FEATURE_NCMC_PEACOCK */

#ifdef LOCAL_CONFIG_FEATURE_NCMC_LED_12_1ST
	if( g_notifications_patern.p_patern != NULL ){
		kfree( g_notifications_patern.p_patern );
	}
	
	if( g_battery_patern.p_patern != NULL ){
		kfree( g_battery_patern.p_patern );
	}
#endif /* #ifdef LOCAL_CONFIG_FEATURE_NCMC_LED_12_1ST */
	
	wake_lock_destroy(&led_ilm_wake_lock);
	
	return;
}

module_init(led_data_input_init);
module_exit(led_data_input_exit);


