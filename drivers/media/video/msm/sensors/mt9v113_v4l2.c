/* Copyright (c) 2011, Code Aurora Forum. All rights reserved.
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

#include "mt9v113_v4l2.h"

/* LOG SETTINGS */
#define MT9V113_LOG_ERR_ON  1   /* ERROR LOG , 0:OFF, 1:ON */
#define MT9V113_LOG_DBG_ON  0   /* DEBUG LOG , 0:OFF, 1:ON */

#if MT9V113_LOG_ERR_ON
#define MT9V113_LOG_ERR(fmt, args...) printk(KERN_ERR "mt9v113:%s(%d) " fmt, __func__, __LINE__, ##args)
#else
#define MT9V113_LOG_ERR(fmt, args...) do{}while(0)
#endif

#if MT9V113_LOG_DBG_ON
#define MT9V113_LOG_DBG(fmt, args...) printk(KERN_INFO "mt9v113:%s(%d) " fmt, __func__, __LINE__, ##args)
#else
#define MT9V113_LOG_DBG(fmt, args...) do{}while(0)
#endif

#define MT9V113_LOG_INF(fmt, args...) printk(KERN_INFO "mt9v113:%s(%d) " fmt, __func__, __LINE__, ##args)

/* DEBUG */
#define MT9V113_DBG_REG_CHECK   MT9V113_LOG_DBG_ON   /* 0:Check OFF, 1:Check ON */


/* CAM2_V_EN2 ON SETTING */
struct pm_gpio mt9v113_cam2_v_en1_on = {
    .direction      = PM_GPIO_DIR_OUT,
    .output_buffer  = PM_GPIO_OUT_BUF_CMOS,
    .output_value   = 1,
    .pull           = PM_GPIO_PULL_NO,
    .vin_sel        = PM_GPIO_VIN_S4,
    .out_strength   = PM_GPIO_STRENGTH_LOW,
    .function       = PM_GPIO_FUNC_NORMAL,
    .inv_int_pol    = 0,
    .disable_pin    = 0,
};

/* CAM2_V_EN2 OFF SETTING */
struct pm_gpio mt9v113_cam2_v_en1_off = {
    .direction      = PM_GPIO_DIR_OUT,
    .output_buffer  = PM_GPIO_OUT_BUF_CMOS,
    .output_value   = 0,
    .pull           = PM_GPIO_PULL_NO,
    .vin_sel        = PM_GPIO_VIN_S4,
    .out_strength   = PM_GPIO_STRENGTH_LOW,
    .function       = PM_GPIO_FUNC_NORMAL,
    .inv_int_pol    = 0,
    .disable_pin    = 0,
};


static struct v4l2_subdev_info mt9v113_subdev_info[] = {
    {
    .code   = V4L2_MBUS_FMT_YUYV8_2X8,
    .colorspace = V4L2_COLORSPACE_JPEG,
    .fmt    = 1,
    .order    = 0,
    },
    /* more can be supported, to be added later */
};

typedef enum {
    SENSOR_MODE_SNAPSHOT,
    SENSOR_MODE_RAW_SNAPSHOT,
    SENSOR_MODE_PREVIEW,
    SENSOR_MODE_PREVIEW_HD,
    SENSOR_MODE_VIDEO,
} mt9v113_sensor_mode_t;
static int old_pict_size = 13;  // Default VGA Mode

static struct msm_sensor_output_info_t mt9v113_dimensions[] = {
    {
        .x_output = 0x0280, /* 640 */
        .y_output = 0x01E0, /* 480 */
        .line_length_pclk = 0x0398,
        .frame_length_lines = 0x01FB,
        .vt_pixel_clk = 13993200, 
        .op_pixel_clk = 224000000,
        .binning_factor = 0,
    },
    {
        .x_output = 0x0280, /* 640 */
        .y_output = 0x01E0, /* 480 */
        .line_length_pclk = 0x0384,
        .frame_length_lines = 0x01FB,
        .vt_pixel_clk = 13689000,
        .op_pixel_clk = 224000000,
        .binning_factor = 0,
    },

};


static struct msm_camera_csid_vc_cfg mt9v113_cid_cfg[] = {
    {0, CSI_YUV422_8, CSI_DECODE_8BIT},
    {1, CSI_EMBED_DATA, CSI_DECODE_8BIT},
};


static struct msm_camera_csi2_params mt9v113_csi_params = {
    .csid_params = {
        .lane_assign = 0xe4,
        .lane_cnt = 1,
        .lut_params = {
            .num_cid = 2,
            .vc_cfg = mt9v113_cid_cfg,
        },
    },
    .csiphy_params = {
        .lane_cnt = 1,
        .settle_cnt = 0x0B,
    },
};


static struct msm_camera_csi2_params *mt9v113_csi_params_array[] = {
    &mt9v113_csi_params,
    &mt9v113_csi_params,
};


static struct msm_sensor_output_reg_addr_t mt9v113_reg_addr = {
    /* NOT USE */
    .x_output = 0x0003,            /* mode_output_width_a (0x0003) */
    .y_output = 0x0005,            /* mode_output_height_a (0x0005) */
    .line_length_pclk = 0x0021,    /* mode_sensor_line_length_pck_a (0x0021) */
    .frame_length_lines = 0x001F,  /* mode_sensor_frame_length_a (0x001F) */
};

static struct msm_sensor_id_info_t mt9v113_id_info = {
    .sensor_id_reg_addr = 0x0000,    /* k24a_chip_id  */
    .sensor_id = 0x2280,             /* 9344 (0x2480) */
};


#if MT9V113_DBG_REG_CHECK
static struct msm_camera_i2c_reg_conf mt9v113_check_settings[] = {
    {0x0000, 0x2480},
};

static void mt9v113_sensor_reg_check(struct msm_camera_i2c_reg_conf *reg_conf_tbl, uint16_t size)
{
    int i = 0;
    uint16_t val = 0xFFFF;

    MT9V113_LOG_DBG("START");

    for (i = 0; i < size; i++) {

        if(reg_conf_tbl[i].reg_addr == MT9V113_MCU_ADDRESS){

            msm_camera_i2c_write(mt9v113_s_ctrl.sensor_i2c_client,
                                MT9V113_MCU_ADDRESS, reg_conf_tbl[i].reg_data, MSM_CAMERA_I2C_WORD_DATA );
            mdelay(1);
            msm_camera_i2c_read(mt9v113_s_ctrl.sensor_i2c_client,
                                MT9V113_MCU_DATA0, &val, MSM_CAMERA_I2C_WORD_DATA );
            MT9V113_LOG_DBG("addr(0x%04X) : set(0x%04X) : now(0x%04X)",reg_conf_tbl[i].reg_data,reg_conf_tbl[i+1].reg_data,val);
            i++;
        }
        else{
            msm_camera_i2c_read(mt9v113_s_ctrl.sensor_i2c_client,
                                reg_conf_tbl[i].reg_addr, &val, MSM_CAMERA_I2C_WORD_DATA );
            MT9V113_LOG_DBG("addr(0x%04X) : set(0x%04X) : now(0x%04X)",reg_conf_tbl[i].reg_addr,reg_conf_tbl[i].reg_data,val);
        }

        mdelay(1);
    } //for

    MT9V113_LOG_DBG("END");
}
#endif /* MT9V113_DBG_REG_CHECK */


static int32_t mt9v113_sensor_reg_polling(uint16_t reg_addr, uint16_t mask, uint16_t terminate_value,
                                          uint16_t poll_interval, uint16_t poll_max, enum mt9v113_access_t reg_type)
{
    int rc = 0;
    int poll_count = 0;
    uint16_t val = 0xFFFF;

    MT9V113_LOG_DBG("START");

    MT9V113_LOG_DBG("reg_addr = 0x%04X",reg_addr);
    MT9V113_LOG_DBG("mask = 0x%04X",mask);
    MT9V113_LOG_DBG("terminate_value = 0x%04X",terminate_value);
    MT9V113_LOG_DBG("poll_interval = 0x%04X",poll_interval);
    MT9V113_LOG_DBG("poll_max = 0x%04X",poll_max);
    MT9V113_LOG_DBG("reg_type = 0x%04X",reg_type);

    /* POLLING */
    for (poll_count = 0; poll_count < poll_max; poll_count++) {

        if( reg_type == MT9V113_ACCESS_VARIABLES ){
            /* write register */
            rc = msm_camera_i2c_write(mt9v113_s_ctrl.sensor_i2c_client,
                                      MT9V113_MCU_ADDRESS, reg_addr, MSM_CAMERA_I2C_WORD_DATA);
            if (rc < 0) {
                MT9V113_LOG_ERR("msm_camera_i2c_write failed rc=%d\n", rc);
                return rc;
            }

            /* read register */
            rc = msm_camera_i2c_read(mt9v113_s_ctrl.sensor_i2c_client,
                                     MT9V113_MCU_DATA0, &val, MSM_CAMERA_I2C_WORD_DATA);
            if (rc < 0) {
                MT9V113_LOG_ERR("msm_camera_i2c_read failed rc=%d\n", rc);
                return rc;
            }

        } else { /* MT9V113_ACCESS_REGISTERS */
            /* read register */
            rc = msm_camera_i2c_read(mt9v113_s_ctrl.sensor_i2c_client,
                                     reg_addr, &val, MSM_CAMERA_I2C_WORD_DATA);
            if (rc < 0) {
                MT9V113_LOG_ERR("msm_camera_i2c_read failed rc=%d\n", rc);
                return rc;
            }

        }

        MT9V113_LOG_DBG("val=%d(0x%04X)\n",val,val);

        if ( (val & mask) == terminate_value ) {
            /* TERMINATE */
            break;
        } else {
            /* NEXT */
            val = 0xFFFF;
        }

        /* DELAY */
        mdelay(poll_interval);
    }

    if(poll_count >= poll_max){
        MT9V113_LOG_ERR("failed addr=0x%04X, poll_count=%d(times), timeout=%d(ms)\n",reg_addr, poll_count , poll_count*poll_interval );
        rc = -EFAULT;
    }else {
        MT9V113_LOG_DBG("success addr=0x%04X, poll_count=%d(times), timeout=%d(ms)\n",reg_addr, poll_count+1 , (poll_count+1)*poll_interval );
        rc = 0;
    }

    MT9V113_LOG_DBG("END(%d)",rc);
    return rc;
}



/*=============================================================
    Refresh mode and command
==============================================================*/



static void mt9v113_refresh_mode_cmd_poll(void)
{
 /* REFRESH_MODE */
	MT9V113_LOG_DBG("msm refresh mode \n");
        msm_sensor_write_conf_array(
                    mt9v113_s_ctrl.sensor_i2c_client,
                   mt9v113_refresh_confs, MT9V113_REFRESH_MODE_SETTING);

        mt9v113_sensor_reg_polling(MT9V113_POLL_RFRSH_ADDR,      MT9V113_POLL_RFRSH_MASK,
                                   MT9V113_POLL_RFRSH_TERMINATE, MT9V113_POLL_RFRSH_INTERVAL,
                                   MT9V113_POLL_RFRSH_RETRY_100,  MT9V113_ACCESS_VARIABLES);

	//mdelay(300);
	MT9V113_LOG_DBG("msm refresh cmd \n");
        /* REFRESH_CMD */
        msm_sensor_write_conf_array(
                    mt9v113_s_ctrl.sensor_i2c_client,
                    mt9v113_refresh_confs, MT9V113_REFRESH_CMD_SETTING);

        mt9v113_sensor_reg_polling(MT9V113_POLL_RFRSH_ADDR,      MT9V113_POLL_RFRSH_MASK,
                                   MT9V113_POLL_RFRSH_TERMINATE, MT9V113_POLL_RFRSH_INTERVAL,
                                   MT9V113_POLL_RFRSH_RETRY_100,  MT9V113_ACCESS_VARIABLES);

}


/*=============================================================
    Scene
==============================================================*/
static int32_t mt9v113_set_scene(struct msm_sensor_ctrl_t *s_ctrl, int scene)
{
    int32_t rc = 0;

    struct msm_camera_i2c_reg_conf*  setting_data=NULL;
    uint16_t setting_size = 0;


    MT9V113_LOG_DBG("START");
    MT9V113_LOG_DBG("scene = %d",scene);

    switch (scene)
    {
        case CAMERA_SCENE_MODE_OFF /* Off */ :
            setting_data = mt9v113_scene_off_settings;
            setting_size = ARRAY_SIZE(mt9v113_scene_off_settings);
            break;

        case CAMERA_SCENE_MODE_PORTRAIT /* Portrait */:
            setting_data = mt9v113_scene_portrait_settings;
            setting_size = ARRAY_SIZE(mt9v113_scene_portrait_settings);
            break;

        case CAMERA_SCENE_MODE_PORTRAIT_ILLUMI /* Portrait and Illumination */ :
            setting_data = mt9v113_scene_portrait_and_illumination_settings;
            setting_size = ARRAY_SIZE(mt9v113_scene_portrait_and_illumination_settings);
            break;

        case CAMERA_SCENE_MODE_AUTO /* Auto */:
        default:
        MT9V113_LOG_DBG("NOT SUPPORTED");
            break;
    }

    if(setting_data)
    {
        rc = msm_camera_i2c_write_tbl(
                s_ctrl->sensor_i2c_client,
                setting_data, setting_size,
                MSM_CAMERA_I2C_WORD_DATA );
    }

    MT9V113_LOG_DBG("END(%d)",rc);
    return rc;
}

/*=============================================================
    Size
==============================================================*/
static int32_t mt9v113_set_pict_size(struct msm_sensor_ctrl_t *s_ctrl, int pict_size)
{
    int32_t rc = 0;



    MT9V113_LOG_DBG("START");
    MT9V113_LOG_DBG("pict_size = %d",pict_size);
    /* Picture size is always  VGA , MSM Camera Interface will capture VGA,QVGA,QCIF Images depending on picture size set from the HAL
               We Need to ask Qualcom that 'How its capturing QVGA/QCIF  image from VGA Preview Buffer frames' */
    if(old_pict_size == pict_size)
       return 0;  // Pict size already Set

      switch (pict_size)
      {
          case 8: /* QCIF */
          case 10: /* QVGA */
          case 13: /* VGA */
              break;

          default:
              MT9V113_LOG_DBG("NOT SUPPORTED");
              return rc;
       }
       old_pict_size = pict_size;
       rc = msm_sensor_write_conf_array(
                s_ctrl->sensor_i2c_client,
                mt9v113_pict_sizes, pict_size);

    MT9V113_LOG_DBG("END(%d)",rc);
    return rc;
}

/*=============================================================
    White Balance
==============================================================*/
static int32_t mt9v113_set_wb(struct msm_sensor_ctrl_t *s_ctrl, int wb_val)
{
    int32_t rc = 0;

    struct msm_camera_i2c_reg_conf*  setting_data=NULL;
    uint16_t setting_size = 0;

    MT9V113_LOG_DBG("START");
    MT9V113_LOG_DBG("wb_val = %d",wb_val);

    switch (wb_val)
    {
        case CAMERA_WB_AUTO:
            setting_data = mt9v113_wb_auto_settings;
            setting_size = ARRAY_SIZE(mt9v113_wb_auto_settings);
            break;

        case CAMERA_WB_DAYLIGHT:
            setting_data = mt9v113_wb_daylight_settings;
            setting_size = ARRAY_SIZE(mt9v113_wb_daylight_settings);
            break;

        case CAMERA_WB_CLOUDY_DAYLIGHT:
            setting_data = mt9v113_wb_cloudy_settings;
            setting_size = ARRAY_SIZE(mt9v113_wb_cloudy_settings);
            break;

        case CAMERA_WB_INCANDESCENT:
            setting_data = mt9v113_wb_incandescent_settings;
            setting_size = ARRAY_SIZE(mt9v113_wb_incandescent_settings);
            break;

        case CAMERA_WB_FLUORESCENT:
        case CAMERA_WB_FLUORESCENT_H:
            /*setting_data = mt9v113_wb_fluorescent_high_settings;
            setting_size = ARRAY_SIZE(mt9v113_wb_fluorescent_high_settings);
            break;
            */
            /* Fall through as as we only support CAMERA_WB_FLUORESCENT_L
               (FLOURESCENT_3800-5500K)*/
        case CAMERA_WB_FLUORESCENT_L:
            setting_data = mt9v113_wb_fluorescent_low_settings;
            setting_size = ARRAY_SIZE(mt9v113_wb_fluorescent_low_settings);
            break;

        default:
        MT9V113_LOG_DBG("NOT SUPPORTED");
            break;
    }

    if(setting_data)
    {
        rc = msm_camera_i2c_write_tbl(
                s_ctrl->sensor_i2c_client,
                setting_data, setting_size,
                MSM_CAMERA_I2C_WORD_DATA );
        /* Changes take effect immediately */
    }

    MT9V113_LOG_DBG("END(%d)",rc);
    return rc;
}

/*=============================================================
    Effect
==============================================================*/
static int32_t mt9v113_set_effect(struct msm_sensor_ctrl_t *s_ctrl, int effect)
{
    int32_t rc = 0;

    struct msm_camera_i2c_reg_conf*  setting_data=NULL;
    uint16_t setting_size = 0;


    MT9V113_LOG_DBG("START");
    MT9V113_LOG_DBG("effect = %d",effect);

    switch (effect)
    {
        case CAMERA_EFFECT_OFF:
            setting_data = mt9v113_effect_off_settings;
            setting_size = ARRAY_SIZE(mt9v113_effect_off_settings);
            break;

        case CAMERA_EFFECT_MONO:
            setting_data = mt9v113_effect_mono_settings;
            setting_size = ARRAY_SIZE(mt9v113_effect_mono_settings);
            break;

        case CAMERA_EFFECT_SEPIA:
            setting_data = mt9v113_effect_sepia_settings;
            setting_size = ARRAY_SIZE(mt9v113_effect_sepia_settings);
            break;
        case CAMERA_EFFECT_NEGATIVE:
            setting_data = mt9v113_effect_negative_settings;
            setting_size = ARRAY_SIZE(mt9v113_effect_negative_settings);
            break;

        default:
        MT9V113_LOG_DBG("NOT SUPPORTED");
            break;
    }

    if(setting_data)
    {
        rc = msm_camera_i2c_write_tbl(
                s_ctrl->sensor_i2c_client,
                setting_data, setting_size,
                MSM_CAMERA_I2C_WORD_DATA );

//        mt9v113_sensor_reg_polling(MT9V113_POLL_RFRSH_ADDR,      MT9V113_POLL_RFRSH_MASK,
  //                                 MT9V113_POLL_RFRSH_TERMINATE, MT9V113_POLL_RFRSH_INTERVAL,
    //                               MT9V113_POLL_RFRSH_RETRY_100, MT9V113_ACCESS_VARIABLES);
    }

    MT9V113_LOG_DBG("END(%d)",rc);
    return rc;
}

/*=============================================================
    Antibanding
==============================================================*/
static int32_t mt9v113_set_antibanding(struct msm_sensor_ctrl_t *s_ctrl, int antibanding)
{
    int32_t rc = 0;

    struct msm_camera_i2c_reg_conf*  setting_data=NULL;
    uint16_t setting_size = 0;


    MT9V113_LOG_DBG("START");
    MT9V113_LOG_DBG("antibanding = %d",antibanding);

    switch (antibanding)
    {
        case CAMERA_ANTIBANDING_AUTO:
            setting_data = mt9v113_antibanding_auto_settings;
            setting_size = ARRAY_SIZE(mt9v113_antibanding_auto_settings);
            break;

        case CAMERA_ANTIBANDING_50HZ:
            setting_data = mt9v113_antibanding_50Hz_settings;
            setting_size = ARRAY_SIZE(mt9v113_antibanding_50Hz_settings);
            break;

        case CAMERA_ANTIBANDING_60HZ:
            setting_data = mt9v113_antibanding_60Hz_settings;
            setting_size = ARRAY_SIZE(mt9v113_antibanding_60Hz_settings);
            break;

        case CAMERA_ANTIBANDING_OFF:
        default:
        MT9V113_LOG_DBG("NOT SUPPORTED");
            break;
    }

    if(setting_data)
    {
        rc = msm_camera_i2c_write_tbl(
                s_ctrl->sensor_i2c_client,
                setting_data, setting_size,
                MSM_CAMERA_I2C_WORD_DATA );
        printk("Borqs :  Delay of 300 ms in antibanding(flicker)");
        mdelay(300);
        rc = msm_camera_i2c_write_tbl(
                s_ctrl->sensor_i2c_client,
                mt9v113_antibanding_refresh_mode_settings,ARRAY_SIZE(mt9v113_antibanding_refresh_mode_settings),
                MSM_CAMERA_I2C_WORD_DATA );

    }

    MT9V113_LOG_DBG("END(%d)",rc);
    return rc;
}


/*=============================================================
    Brightness
==============================================================*/
static int32_t mt9v113_set_exp_compensation(struct msm_sensor_ctrl_t *s_ctrl, int brightness)
{
    int32_t rc = 0;

    MT9V113_LOG_DBG("START");
    MT9V113_LOG_DBG("brightness = %d",brightness);

    if( (brightness < 0) || (ARRAY_SIZE(mt9v113_brightness_confs) <= brightness) )
    {
        MT9V113_LOG_ERR("ERROR: INVALID VALUE(%d)\n", brightness);
        return -EFAULT;
    }

    rc = msm_sensor_write_conf_array(
        s_ctrl->sensor_i2c_client,
        mt9v113_brightness_confs, brightness);

    mt9v113_sensor_reg_polling(MT9V113_POLL_RFRSH_ADDR,      MT9V113_POLL_RFRSH_MASK,
                               MT9V113_POLL_RFRSH_TERMINATE, MT9V113_POLL_RFRSH_INTERVAL,
                               MT9V113_POLL_RFRSH_RETRY_100, MT9V113_ACCESS_VARIABLES);

    MT9V113_LOG_DBG("END(%d)",rc);
    return rc;
}


/*=============================================================
    GET MAKER NOTE
==============================================================*/
static int32_t mt9v113_get_maker_note(struct msm_sensor_ctrl_t *s_ctrl,
                                      struct get_exif_maker_note_cfg *get_exif_maker_note)
{
    int32_t rc = 0;
    int cnt = 0;
    uint16_t val = 0xFFFF;

    uint16_t device_id =0;            /* device ID        */
    uint16_t fd_freq =0;              /* fd_mode          */
    uint16_t awb_temp =0;             /* awb_ccm_position */
    uint16_t awb_gain_r =0;           /* awb_gain_r       */
    uint16_t awb_gain_g =0;           /* awb_gain_g       */
    uint16_t awb_gain_b =0;           /* awb_gain_b       */
    uint16_t awb_saturation =0;       /* awb_saturation   */

    struct read_exif_param_t read_exif_params[]={
        {MT9V113_ACCESS_VARIABLES, 0xA404, &fd_freq       },
        {MT9V113_ACCESS_VARIABLES, 0xA353, &awb_temp      },
        {MT9V113_ACCESS_VARIABLES, 0xA34E, &awb_gain_r    },
        {MT9V113_ACCESS_VARIABLES, 0xA34F, &awb_gain_g    },
        {MT9V113_ACCESS_VARIABLES, 0xA350, &awb_gain_b    },
        {MT9V113_ACCESS_VARIABLES, 0xA354, &awb_saturation},
    };

    MT9V113_LOG_DBG("START");

    for(cnt=0; cnt < ARRAY_SIZE(read_exif_params); cnt++)
    {
        if(read_exif_params[cnt].reg_type == MT9V113_ACCESS_VARIABLES){

            rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
                                      MT9V113_MCU_ADDRESS, read_exif_params[cnt].reg_addr, MSM_CAMERA_I2C_WORD_DATA);
            if (rc < 0) {
                MT9V113_LOG_ERR("msm_camera_i2c_write failed rc=%d\n", rc);
                return rc;
            }

            rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client,
                                     MT9V113_MCU_DATA0, &val, MSM_CAMERA_I2C_WORD_DATA);
            if (rc < 0) {
                MT9V113_LOG_ERR("msm_camera_i2c_read failed rc=%d\n", rc);
                return rc;
            }
        } else { /* MT9V113_ACCESS_REGISTERS */
            rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client,
                                     read_exif_params[cnt].reg_addr, &val, MSM_CAMERA_I2C_WORD_DATA);
            if (rc < 0) {
                MT9V113_LOG_ERR("msm_camera_i2c_read failed rc=%d\n", rc);
                return rc;
            }
        }

        *read_exif_params[cnt].reg_data = val;
    }

    if( fd_freq & (1<<7) ){
        /* Manual */
        fd_freq = 0x00;
    } else{
        /* Auto */
        fd_freq = (fd_freq>>5)&1 ? 0x02 : 0x03;
    }

    device_id = 0x2480;     /* fixed value */

    get_exif_maker_note->device_id      = device_id;
    get_exif_maker_note->fd_freq        = fd_freq;
    get_exif_maker_note->awb_temp       = awb_temp;
    get_exif_maker_note->awb_gain_r     = awb_gain_r;
    get_exif_maker_note->awb_gain_g     = awb_gain_g;
    get_exif_maker_note->awb_gain_b     = awb_gain_b;
    get_exif_maker_note->awb_saturation = awb_saturation;

    MT9V113_LOG_DBG("END(%d)",rc);
    return rc;
}

/*=============================================================
    GET EXIF PARAM
==============================================================*/
static int32_t mt9v113_get_exif_param(struct msm_sensor_ctrl_t *s_ctrl,
                                      struct get_exif_param_inf *get_exif_param)
{
    int32_t rc = 0;
    int cnt = 0;
    uint16_t val = 0xFFFF;

    uint16_t coarse_integration_time;      /* 0x3012 */
    uint16_t line_length_pck;              /* 0x300C */
    uint16_t fine_integration_time;        /* 0x3014 */
    uint16_t analog_gain_code_global;      /* 0x3028 */
    uint16_t digital_gain_greenr;          /* 0x3032 */

    struct read_exif_param_t read_exif_params[]={
        {MT9V113_ACCESS_REGISTERS, 0x3012, &coarse_integration_time},
        {MT9V113_ACCESS_REGISTERS, 0x300C, &line_length_pck        },
        {MT9V113_ACCESS_REGISTERS, 0x3014, &fine_integration_time  },
        {MT9V113_ACCESS_REGISTERS, 0x3028, &analog_gain_code_global},
        {MT9V113_ACCESS_REGISTERS, 0x3032, &digital_gain_greenr    },
    };

    MT9V113_LOG_DBG("START");

    for(cnt=0; cnt < ARRAY_SIZE(read_exif_params); cnt++)
    {
        if(read_exif_params[cnt].reg_type == MT9V113_ACCESS_VARIABLES){

            rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
                                      MT9V113_MCU_ADDRESS, read_exif_params[cnt].reg_addr, MSM_CAMERA_I2C_WORD_DATA);
            if (rc < 0) {
                MT9V113_LOG_ERR("msm_camera_i2c_write failed rc=%d\n", rc);
                return rc;
            }

            rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client,
                                     MT9V113_MCU_DATA0, &val, MSM_CAMERA_I2C_WORD_DATA);
            if (rc < 0) {
                MT9V113_LOG_ERR("msm_camera_i2c_read failed rc=%d\n", rc);
                return rc;
            }
        } else { /* MT9V113_ACCESS_REGISTERS */
            rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client,
                                     read_exif_params[cnt].reg_addr, &val, MSM_CAMERA_I2C_WORD_DATA);
            if (rc < 0) {
                MT9V113_LOG_ERR("msm_camera_i2c_read failed rc=%d\n", rc);
                return rc;
            }
        }

        *read_exif_params[cnt].reg_data = val;
    }

    get_exif_param->coarse_integration_time = coarse_integration_time;
    get_exif_param->line_length_pck         = line_length_pck;
    get_exif_param->fine_integration_time   = fine_integration_time;
    get_exif_param->analog_gain_code_global = analog_gain_code_global;
    get_exif_param->digital_gain_greenr     = digital_gain_greenr;

    MT9V113_LOG_DBG("END(%d)",rc);
    return rc;
}


static void mt9v113_sensor_start_stream(struct msm_sensor_ctrl_t *s_ctrl)
{
    MT9V113_LOG_DBG("");
    msm_sensor_write_all_conf_array(
        s_ctrl->sensor_i2c_client,
        mt9v113_preview_start_confs,
        ARRAY_SIZE(mt9v113_preview_start_confs) );
    msleep(90);
}


static void mt9v113_sensor_stop_stream(struct msm_sensor_ctrl_t *s_ctrl)
{
    MT9V113_LOG_DBG("");
    msm_sensor_write_all_conf_array(
        s_ctrl->sensor_i2c_client,
        mt9v113_preview_stop_confs,
        ARRAY_SIZE(mt9v113_preview_stop_confs) );
    msleep(40);
}


static int32_t mt9v113_sensor_write_res_settings(struct msm_sensor_ctrl_t *s_ctrl,
    uint16_t res)
{
    int32_t rc = 0;

    MT9V113_LOG_DBG("START \n");
    rc = msm_sensor_write_conf_array(
        s_ctrl->sensor_i2c_client,
        s_ctrl->msm_sensor_reg->mode_settings, res);
    if (rc < 0){
        MT9V113_LOG_ERR("ERROR: msm_sensor_write_conf_array() rc = %d\n",rc);
        return rc;
    }

    mt9v113_refresh_mode_cmd_poll();

    MT9V113_LOG_DBG("END(%d) \n",rc);

    return rc;
}

/*=============================================================
    Sensor Mode
==============================================================*/
int32_t mt9v113_set_sensor_mode(struct msm_sensor_ctrl_t *s_ctrl,
    int mode, int res)
{
    int32_t rc = 0;

    if (s_ctrl->curr_res != res) {
        s_ctrl->curr_frame_length_lines =
            s_ctrl->msm_sensor_reg->
            output_settings[res].frame_length_lines;

        s_ctrl->curr_line_length_pclk =
            s_ctrl->msm_sensor_reg->
            output_settings[res].line_length_pclk;

        if (s_ctrl->sensordata->pdata->is_csic)
            rc = s_ctrl->func_tbl->sensor_csi_setting(s_ctrl,
                MSM_SENSOR_UPDATE_PERIODIC, res);
        else
            rc = s_ctrl->func_tbl->sensor_setting(s_ctrl,
                MSM_SENSOR_UPDATE_PERIODIC, res);
        if (rc < 0)
            return rc;
        s_ctrl->curr_res = res;
    }

    return rc;
}


static int32_t mt9v113_sensor_setting(struct msm_sensor_ctrl_t *s_ctrl,
                                      int update_type, int res)
{
    int32_t rc = 0;

    MT9V113_LOG_DBG("START \n");

    v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
        NOTIFY_ISPIF_STREAM, (void *)ISPIF_STREAM(
        PIX_0, ISPIF_OFF_IMMEDIATELY));

    msleep(30);

    if (update_type == MSM_SENSOR_REG_INIT) {

        MT9V113_LOG_INF("MSM_SENSOR_REG_INIT res %d \n",res);

        msm_sensor_write_all_conf_array(
            s_ctrl->sensor_i2c_client,
            mt9v113_init_confs,
            ARRAY_SIZE(mt9v113_init_confs) );

        mt9v113_sensor_reg_polling(MT9V113_POLL_STNDBY_ADDR,      MT9V113_POLL_STNDBY_MASK,
                                   MT9V113_POLL_STNDBY_TERMINATE, MT9V113_POLL_STNDBY_INTERVAL,
                                   MT9V113_POLL_STNDBY_RETRY,     MT9V113_ACCESS_REGISTERS);

        mt9v113_sensor_reg_polling(MT9V113_POLL_STRM_ADDR,      MT9V113_POLL_STRM_MASK,
                                   MT9V113_POLL_STRM_TERMINATE, MT9V113_POLL_STRM_INTERVAL,
                                   MT9V113_POLL_STRM_RETRY,     MT9V113_ACCESS_REGISTERS);

        msm_sensor_write_all_conf_array(
            s_ctrl->sensor_i2c_client,
            mt9v113_init_confs2,
            ARRAY_SIZE(mt9v113_init_confs2) );

        msm_sensor_write_all_conf_array(
            s_ctrl->sensor_i2c_client,
            mt9v113_init_confs3,
            ARRAY_SIZE(mt9v113_init_confs3) );

        s_ctrl->curr_csi_params = s_ctrl->csi_params[res];
        v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
            NOTIFY_CSID_CFG,
            &s_ctrl->curr_csi_params->csid_params);
        v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
                    NOTIFY_CID_CHANGE, NULL);
        mb();
        v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
            NOTIFY_CSIPHY_CFG,
            &s_ctrl->curr_csi_params->csiphy_params);
        mb();
        msleep(20);

        msm_sensor_write_all_conf_array(
            s_ctrl->sensor_i2c_client,
            mt9v113_init_confs4,
            ARRAY_SIZE(mt9v113_init_confs4) );

        msm_sensor_write_all_conf_array(
            s_ctrl->sensor_i2c_client,
            mt9v113_init_confs5,
            ARRAY_SIZE(mt9v113_init_confs5) );

        //Setting Default/Old pict size
        msm_sensor_write_conf_array(
            s_ctrl->sensor_i2c_client,
            mt9v113_pict_sizes, old_pict_size);

        //Setting Preview Mode
        msm_camera_i2c_write_tbl(
            s_ctrl->sensor_i2c_client,
            mt9v113_picture_prev_settings,ARRAY_SIZE(mt9v113_picture_prev_settings),
            MSM_CAMERA_I2C_WORD_DATA );

        mt9v113_refresh_mode_cmd_poll();

        printk("End of INIT \n");

    } else if (update_type == MSM_SENSOR_UPDATE_PERIODIC) {
        MT9V113_LOG_INF("MSM_SENSOR_UPDATE_PERIODIC res = %d\n",res);

        mt9v113_sensor_write_res_settings(s_ctrl, res);

        if (s_ctrl->curr_csi_params != s_ctrl->csi_params[res]) {

            MT9V113_LOG_DBG("curr_csi_params != csi_params[%d]\n",res);

            s_ctrl->curr_csi_params = s_ctrl->csi_params[res];
            v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
                NOTIFY_CSID_CFG,
                &s_ctrl->curr_csi_params->csid_params);
            v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
                        NOTIFY_CID_CHANGE, NULL);
            mb();
            v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
                NOTIFY_CSIPHY_CFG,
                &s_ctrl->curr_csi_params->csiphy_params);
            mb();
            msleep(20);
        }

        v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
            NOTIFY_PCLK_CHANGE, &s_ctrl->msm_sensor_reg->
            output_settings[res].op_pixel_clk);
        v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
            NOTIFY_ISPIF_STREAM, (void *)ISPIF_STREAM(
            PIX_0, ISPIF_ON_FRAME_BOUNDARY));
        msleep(30);
        s_ctrl->func_tbl->sensor_start_stream(s_ctrl);

#if MT9V113_DBG_REG_CHECK
        mt9v113_sensor_reg_check(mt9v113_check_settings,ARRAY_SIZE(mt9v113_check_settings));
#endif /* MT9V113_DBG_REG_CHECK */
    }

    MT9V113_LOG_DBG("END(%d) \n",rc);
    return rc;
}

static int mt9v113_i2c_probe(struct i2c_client *client,
                const struct i2c_device_id *id)
{
    int rc = 0;

    MT9V113_LOG_DBG("START");

    rc = msm_sensor_i2c_probe(client, id);

    MT9V113_LOG_DBG("END(%d)",rc);
    return rc;
}


static const struct i2c_device_id mt9v113_i2c_id[] = {
    {SENSOR_NAME, (kernel_ulong_t)&mt9v113_s_ctrl},
    { }
};


static struct i2c_driver mt9v113_i2c_driver = {
    .id_table = mt9v113_i2c_id,
    .probe  = mt9v113_i2c_probe,
    .driver = {
        .name = SENSOR_NAME,
    },
};


static struct msm_camera_i2c_client mt9v113_sensor_i2c_client = {
    .addr_type = MSM_CAMERA_I2C_WORD_ADDR,

};


static int mt9v113_sensor_power_down(struct msm_sensor_ctrl_t *s_ctrl)
{
    /* MCLK */
    struct clk *cam_clk;

    MT9V113_LOG_INF("START \n");

    /***** MCLK DISABLE *****/
    cam_clk  = clk_get(NULL, "cam2_clk");
    if(cam_clk != NULL){
        clk_disable(cam_clk);
        MT9V113_LOG_DBG("msm_camio_clk_disable\n");
    }else{
        MT9V113_LOG_DBG("cam_clk is NULL!\n");
    }

    /* WAIT */
    MT9V113_LOG_DBG("WAIT %d[ms]\n",MT9V113_WAIT_PWOFF_MCLK);
    mdelay( MT9V113_WAIT_PWOFF_MCLK );



    /***** CAM2_RST_N OFF *****/
    gpio_direction_output(MT9V113_GPIO_CAM2_RST_N, 0);
    gpio_free(MT9V113_GPIO_CAM2_RST_N);
    MT9V113_LOG_DBG("CAM2_RST_N(%d) Low\n", MT9V113_GPIO_CAM2_RST_N);

    /* WAIT */
    MT9V113_LOG_DBG("WAIT %d[ms]\n",MT9V113_WAIT_PWOFF_RST_L);
    mdelay( MT9V113_WAIT_PWOFF_RST_L );




    /***** AVDD2.8V OFF (PM) *****/
    pm8xxx_gpio_config(MT9V113_PMGPIO_VDD_CAM2_V_EN1, &mt9v113_cam2_v_en1_off);
    MT9V113_LOG_DBG("VDD_CAM2_V_EN1'(%d) Low\n", MT9V113_PMGPIO_VDD_CAM2_V_EN1);

    /* WAIT */
    MT9V113_LOG_DBG("WAIT %d[ms]\n",MT9V113_WAIT_PWOFF_V_EN1);
    mdelay( MT9V113_WAIT_PWOFF_V_EN1 );


    /***** I2C SUSPEND *****/
    gpio_free(MT9V113_GPIO_I2C2_SDA);
    MT9V113_LOG_DBG("I2C2_SDA(%d) SUSPEND\n", MT9V113_GPIO_I2C2_SDA);

    gpio_free(MT9V113_GPIO_I2C2_SCL);
    MT9V113_LOG_DBG("I2C2_SCL(%d) SUSPEND\n", MT9V113_GPIO_I2C2_SCL);


    /***** DVDD,IOVDD OFF *****/
    gpio_direction_output(MT9V113_GPIO_VDD_CAM2_V_EN2, 0);
    gpio_free(MT9V113_GPIO_VDD_CAM2_V_EN2);
    MT9V113_LOG_DBG("VDD_CAM2_V_EN2(%d) Low\n", MT9V113_GPIO_VDD_CAM2_V_EN2);

    MT9V113_LOG_INF("END(0) \n");

    return 0;
}

static int32_t mt9v113_sensor_power_up(struct msm_sensor_ctrl_t *s_ctrl)
{
    int32_t rc = 0;

    /* MCLK */
    struct clk *cam_clk;

    MT9V113_LOG_INF("START \n");

    /* GET MCLK */
    cam_clk  = clk_get(NULL, "cam2_clk");

    /***** MCLK SET RATE *****/
    if(cam_clk != NULL){
        clk_set_rate(cam_clk, MT9V113_MCLK_FREQUENCY);
        MT9V113_LOG_DBG("clk_set_rate():%d\n",rc);
    }else{
        /* POWER OFF */
        MT9V113_LOG_ERR("cam_clk is NULL!\n");
        mt9v113_sensor_power_down(s_ctrl);
        return -EFAULT;
    }


    /***** CAM2_RST_N ON *****/
    rc = gpio_request(MT9V113_GPIO_CAM2_RST_N, SENSOR_NAME);
    if (!rc) {
        gpio_direction_output(MT9V113_GPIO_CAM2_RST_N, 1);
    } else {
        /* POWER OFF */
        MT9V113_LOG_ERR("CAM2_RST_N(%d) Error, rc = %d\n", MT9V113_GPIO_CAM2_RST_N, rc);
        mt9v113_sensor_power_down(s_ctrl);
        return -EFAULT;
    }
    MT9V113_LOG_DBG("CAM2_RST_N(%d) High\n", MT9V113_GPIO_CAM2_RST_N);

    /* WAIT */
    MT9V113_LOG_DBG("WAIT %d[ms]\n",MT9V113_WAIT_PWON_RST_H);
    msleep( MT9V113_WAIT_PWON_RST_H );


    /***** DVDD,IOVDD ON *****/
    rc = gpio_request(MT9V113_GPIO_VDD_CAM2_V_EN2, SENSOR_NAME);
    if (!rc) {
        gpio_direction_output(MT9V113_GPIO_VDD_CAM2_V_EN2, 1);
    } else {
        /* POWER OFF */
        MT9V113_LOG_ERR("VDD_CAM2_V_EN2(%d) Error, rc = %d\n", MT9V113_GPIO_VDD_CAM2_V_EN2, rc);
        mt9v113_sensor_power_down(s_ctrl);
        return -EFAULT;
    }
    MT9V113_LOG_DBG("VDD_CAM2_V_EN2(%d) High\n", MT9V113_GPIO_VDD_CAM2_V_EN2);


    /***** I2C ACTIVE *****/
    /* I2C2_SDA */
    rc = gpio_request(MT9V113_GPIO_I2C2_SDA, "mt9v113");
    if (rc < 0) {
        /* POWER OFF */
        MT9V113_LOG_ERR("MT9V113_GPIO_I2C2_SDA(%d) Error, rc = %d\n", MT9V113_GPIO_I2C2_SDA, rc);
        mt9v113_sensor_power_down(s_ctrl);
        return -EFAULT;
    }

    MT9V113_LOG_DBG("I2C2_SDA(%d) ACTIVE\n", MT9V113_GPIO_I2C2_SDA);

    /* I2C2_SCL */
    rc = gpio_request(MT9V113_GPIO_I2C2_SCL, "mt9v113");
    if (rc < 0) {
        /* POWER OFF */
        MT9V113_LOG_ERR("MT9V113_GPIO_I2C2_SCL(%d) Error, rc = %d\n", MT9V113_GPIO_I2C2_SCL, rc);
        mt9v113_sensor_power_down(s_ctrl);
        return -EFAULT;
    }

    MT9V113_LOG_DBG("I2C2_SCL(%d) ACTIVE\n", MT9V113_GPIO_I2C2_SCL);

    /* WAIT */
    MT9V113_LOG_DBG("WAIT %d[ms]\n",MT9V113_WAIT_PWON_V_EN2);
    msleep( MT9V113_WAIT_PWON_V_EN2 );


    /***** AVDD2.8V ON (PM) *****/
    rc = pm8xxx_gpio_config(MT9V113_PMGPIO_VDD_CAM2_V_EN1, &mt9v113_cam2_v_en1_on);

    if (rc) {
        /* POWER OFF */
        mt9v113_sensor_power_down(s_ctrl);
        MT9V113_LOG_ERR("VDD_CAM2_V_EN1(%d) Error, rc = %d\n", MT9V113_PMGPIO_VDD_CAM2_V_EN1, rc);
        return -EFAULT;
    }
    MT9V113_LOG_DBG("VDD_CAM2_V_EN1(%d) High\n", MT9V113_PMGPIO_VDD_CAM2_V_EN1);

    /* WAIT */
    MT9V113_LOG_DBG("WAIT %d[ms]\n",MT9V113_WAIT_PWON_V_EN1);
    msleep( MT9V113_WAIT_PWON_V_EN1 );


    /***** CAM2_RST_N OFF *****/
    gpio_direction_output(MT9V113_GPIO_CAM2_RST_N, 0);
    MT9V113_LOG_DBG("CAM2_RST_N(%d) Low\n", MT9V113_GPIO_CAM2_RST_N);

    /* WAIT */
    MT9V113_LOG_DBG("WAIT %d[ms]\n",MT9V113_WAIT_PWON_RST_L);
    msleep( MT9V113_WAIT_PWON_RST_L );


    /***** MCLK ENABLE *****/
    rc = clk_enable(cam_clk);
    MT9V113_LOG_DBG("clk_enable():%d\n",rc);
    MT9V113_LOG_DBG("MCLK RATE:%lu\n",clk_get_rate(cam_clk));

    /* WAIT */
    MT9V113_LOG_DBG("WAIT %d[ms]\n",MT9V113_WAIT_PWON_MCLK);
    msleep( MT9V113_WAIT_PWON_MCLK );


    /***** CAM2_RST_N ON *****/
    gpio_direction_output(MT9V113_GPIO_CAM2_RST_N, 1);
    MT9V113_LOG_DBG("CAM2_RST_N(%d) High\n", MT9V113_GPIO_CAM2_RST_N);

    /* WAIT FOR I2C ACCESS */
    MT9V113_LOG_DBG("WAIT %d[us]\n",MT9V113_WAIT_PWON_I2C_ACCESS);
    usleep( MT9V113_WAIT_PWON_I2C_ACCESS );

    /* FOR DEBUG */
    MT9V113_LOG_DBG("GPIO[%d]:%d",MT9V113_GPIO_VDD_CAM2_V_EN2,gpio_get_value(MT9V113_GPIO_VDD_CAM2_V_EN2));
    MT9V113_LOG_DBG("GPIO[%d]:%d",MT9V113_GPIO_I2C2_SDA,gpio_get_value(MT9V113_GPIO_I2C2_SDA));
    MT9V113_LOG_DBG("GPIO[%d]:%d",MT9V113_GPIO_I2C2_SCL,gpio_get_value(MT9V113_GPIO_I2C2_SCL));
    MT9V113_LOG_DBG("GPIO[%d]:%d",MT9V113_GPIO_CAM2_RST_N,gpio_get_value(MT9V113_GPIO_CAM2_RST_N));

    MT9V113_LOG_INF("END(%d) \n",rc);

    return rc;

}


static int __init msm_sensor_init_module(void)
{
    MT9V113_LOG_DBG("Call i2c_add_driver()");
    return i2c_add_driver(&mt9v113_i2c_driver);
}


static struct v4l2_subdev_core_ops mt9v113_subdev_core_ops = {
    .s_ctrl = msm_sensor_v4l2_s_ctrl,
    .queryctrl = msm_sensor_v4l2_query_ctrl,
    .ioctl = msm_sensor_subdev_ioctl,
    .s_power = msm_sensor_power,
};

static struct v4l2_subdev_video_ops mt9v113_subdev_video_ops = {
    .enum_mbus_fmt = msm_sensor_v4l2_enum_fmt,
};


static struct v4l2_subdev_ops mt9v113_subdev_ops = {
    .core = &mt9v113_subdev_core_ops,
    .video  = &mt9v113_subdev_video_ops,
};


static struct msm_sensor_fn_t mt9v113_func_tbl = {
    .sensor_start_stream = mt9v113_sensor_start_stream,
    .sensor_stop_stream = mt9v113_sensor_stop_stream,
    .sensor_setting = mt9v113_sensor_setting,
    .sensor_set_sensor_mode = mt9v113_set_sensor_mode,
    .sensor_mode_init = msm_sensor_mode_init,
    .sensor_get_output_info = msm_sensor_get_output_info,
    .sensor_config = msm_sensor_config,
    .sensor_power_up = mt9v113_sensor_power_up,
    .sensor_power_down = mt9v113_sensor_power_down,
    .sensor_set_parm_pm_obs = msm_sensor_set_parm_pm_obs,
    .sensor_set_wb = mt9v113_set_wb,
    .sensor_set_effect = mt9v113_set_effect,
    .sensor_set_antibanding = mt9v113_set_antibanding,
    .sensor_set_exp_compensation = mt9v113_set_exp_compensation,
    .sensor_set_scene = mt9v113_set_scene,
    .sensor_set_pict_size = mt9v113_set_pict_size,
    .sensor_get_maker_note = mt9v113_get_maker_note,
    .sensor_get_exif_param = mt9v113_get_exif_param,
};

static struct msm_sensor_reg_t mt9v113_regs = {
    .default_data_type = MSM_CAMERA_I2C_WORD_DATA,
    .init_settings = &mt9v113_init_confs[0],
    .init_size = ARRAY_SIZE(mt9v113_init_confs),
    .mode_settings = &mt9v113_confs[0],
    .output_settings = &mt9v113_dimensions[0],
    .num_conf = ARRAY_SIZE(mt9v113_confs),
};

static struct msm_sensor_ctrl_t mt9v113_s_ctrl = {
    .msm_sensor_reg = &mt9v113_regs,
    .sensor_i2c_client = &mt9v113_sensor_i2c_client,
    .sensor_i2c_addr = 0x7A, /* Slave Address : 7Ah/7Bh */
    .sensor_output_reg_addr = &mt9v113_reg_addr,
    .sensor_id_info = &mt9v113_id_info,
    .cam_mode = MSM_SENSOR_MODE_INVALID,
    .csi_params = &mt9v113_csi_params_array[0],
    .msm_sensor_mutex = &mt9v113_mut,
    .sensor_i2c_driver = &mt9v113_i2c_driver,
    .sensor_v4l2_subdev_info = mt9v113_subdev_info,
    .sensor_v4l2_subdev_info_size = ARRAY_SIZE(mt9v113_subdev_info),
    .sensor_v4l2_subdev_ops = &mt9v113_subdev_ops,
    .func_tbl = &mt9v113_func_tbl,
};

module_init(msm_sensor_init_module);
MODULE_DESCRIPTION("Aptina 1.3MP YUV sensor driver");
MODULE_LICENSE("GPL v2");
