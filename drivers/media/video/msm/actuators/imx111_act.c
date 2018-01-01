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

#include "msm_actuator.h"
#include "msm_camera_eeprom.h"

#define IMX111_TOTAL_STEPS_NEAR_TO_FAR_MAX 41

DEFINE_MUTEX(imx111_act_mutex);
static struct msm_actuator_ctrl_t imx111_act_t;

static int32_t imx111_wrapper_i2c_write(struct msm_actuator_ctrl_t *a_ctrl,
	int16_t next_lens_position, void *params)
{
    int32_t rc = 0;

	uint16_t msb = 0, lsb = 0;
	uint16_t *hw_params = (uint16_t *)params;
	msb = (next_lens_position >> 4) & 0x3F;
	lsb = (next_lens_position << 4) & 0xF0;
	lsb |= ((*hw_params) & 0xF);
	CDBG("%s: Actuator MSB:0x%x, LSB:0x%x\n", __func__, msb, lsb);

	rc = msm_camera_i2c_write(&a_ctrl->i2c_client,
		msb, lsb, MSM_CAMERA_I2C_BYTE_DATA);

    if(rc < 0){
		pr_err("%s: X  I2C write failed \n", __func__);
        return rc;
    }

	return next_lens_position;
}

static uint16_t imx111_macro_scenario[] = {
	/* MOVE_NEAR dir*/
	4,
	10,
	IMX111_TOTAL_STEPS_NEAR_TO_FAR_MAX,
};

static uint16_t imx111_inf_scenario[] = {
	/* MOVE_FAR dir */
	1,
	16,
	IMX111_TOTAL_STEPS_NEAR_TO_FAR_MAX,
};

static uint16_t imx111_hw_params[] = {
	0x00,/*0*/
	0x05,/*1*/
	0x04,/*2*/
	0x0E,/*3*/
	0x07,/*4*/
	0x05,/*5*/
	0x0E,/*6*/
};

static struct region_params_t imx111_regions[] = {
	/* step_bound[0] - macro side boundary
	 * step_bound[1] - infinity side boundary
	 */
	/* Region 1 */
	{
		.step_bound = {1, 0},
		.code_per_step = 130,
	},
	/* Region 2 */
	{
		.step_bound = {IMX111_TOTAL_STEPS_NEAR_TO_FAR_MAX, 1},
		.code_per_step = 8,
	}
};

static struct damping_params_t imx111_macro_reg1_damping[] = {
	/* MOVE_NEAR Dir */
	/* Scene 1 => Damping params */
	{
		.damping_step = 0x3FF,
		.damping_delay = 1000,
		.hw_params = &imx111_hw_params[0],
	},
	/* Scene 2 => Damping params */
	{
		.damping_step = 0x3FF,
		.damping_delay = 1000,
		.hw_params = &imx111_hw_params[0],
	},
	/* Scene 3 => Damping params */
	{
		.damping_step = 0x3FF,
		.damping_delay = 1000,
		.hw_params = &imx111_hw_params[0],
	},
};

static struct damping_params_t imx111_macro_reg2_damping[] = {
	/* MOVE_NEAR Dir */
	/* Scene 1 => Damping params */
	{
		.damping_step = 0x3FF,
		.damping_delay = 5000,
		.hw_params = &imx111_hw_params[1],
	},
	/* Scene 2 => Damping params */
	{
		.damping_step = 0x3FF,
		.damping_delay = 5000,
		.hw_params = &imx111_hw_params[2],
	},
	/* Scene 3 => Damping params */
	{
		.damping_step = 0x3FF,
		.damping_delay = 5000,
		.hw_params = &imx111_hw_params[3],
	},
};

static struct damping_params_t imx111_inf_reg1_damping[] = {
	/* MOVE_FAR Dir */
	/* Scene 1 => Damping params */
	{
		.damping_step = 0x3FF,
		.damping_delay = 1000,
		.hw_params = &imx111_hw_params[0],
	},
	/* Scene 2 => Damping params */
	{
		.damping_step = 0x3FF,
		.damping_delay = 1000,
		.hw_params = &imx111_hw_params[0],
	},
	/* Scene 3 => Damping params */
	{
		.damping_step = 0x3FF,
		.damping_delay = 1000,
		.hw_params = &imx111_hw_params[0],
	},
};

static struct damping_params_t imx111_inf_reg2_damping[] = {
	/* MOVE_FAR Dir */
	/* Scene 1 => Damping params */
	{
		.damping_step = 0x3FF,
		.damping_delay = 5000,
		.hw_params = &imx111_hw_params[4],
	},
	/* Scene 2 => Damping params */
	{
		.damping_step = 0x3FF,
		.damping_delay = 20000,
		.hw_params = &imx111_hw_params[5],
	},
	/* Scene 3 => Damping params */
	{
		.damping_step = 0x3FF,
		.damping_delay = 25000,
		.hw_params = &imx111_hw_params[6],
	},
};

static struct damping_t imx111_macro_regions[] = {
	/* MOVE_NEAR dir */
	/* Region 1 */
	{
		.ringing_params = imx111_macro_reg1_damping,
	},
	/* Region 2 */
	{
		.ringing_params = imx111_macro_reg2_damping,
	},
};

static struct damping_t imx111_inf_regions[] = {
	/* MOVE_FAR dir */
	/* Region 1 */
	{
		.ringing_params = imx111_inf_reg1_damping,
	},
	/* Region 2 */
	{
		.ringing_params = imx111_inf_reg2_damping,
	},
};


static int32_t imx111_set_params(struct msm_actuator_ctrl_t *a_ctrl)
{
    int32_t rc = 0;

	rc = msm_camera_i2c_write(&a_ctrl->i2c_client,0xEC, 0xA3,
		MSM_CAMERA_I2C_BYTE_DATA);
    if(rc < 0)
        return rc;

	rc = msm_camera_i2c_write(&a_ctrl->i2c_client,0xA1, 0x00,
		MSM_CAMERA_I2C_BYTE_DATA);
    if(rc < 0)
        return rc;

	rc = msm_camera_i2c_write(&a_ctrl->i2c_client,0xF2, 0x90,
		MSM_CAMERA_I2C_BYTE_DATA);
    if(rc < 0)
        return rc;

	rc = msm_camera_i2c_write(&a_ctrl->i2c_client,0xDC, 0x51,
		MSM_CAMERA_I2C_BYTE_DATA);
    if(rc < 0)
        return rc;

	return rc;
}

static const struct i2c_device_id imx111_act_i2c_id[] = {
	{"imx111_act", (kernel_ulong_t)&imx111_act_t},
	{ }
};

static int imx111_act_config(
	void __user *argp)
{
	CDBG("%s called\n", __func__);
	return (int) msm_actuator_config(&imx111_act_t, argp);
}

static int imx111_i2c_add_driver_table(
	void)
{
	CDBG("%s called\n", __func__);
	return (int) msm_actuator_init_table(&imx111_act_t);
}

static struct i2c_driver imx111_act_i2c_driver = {
	.id_table = imx111_act_i2c_id,
	.probe  = msm_actuator_i2c_probe,
	.remove = __exit_p(imx111_act_i2c_remove),
	.driver = {
		.name = "imx111_act",
	},
};

static int __init imx111_i2c_add_driver(
	void)
{
	CDBG("%s called\n", __func__);
	return i2c_add_driver(imx111_act_t.i2c_driver);
}

static struct v4l2_subdev_core_ops imx111_act_subdev_core_ops;

static struct v4l2_subdev_ops imx111_act_subdev_ops = {
	.core = &imx111_act_subdev_core_ops,
};

int32_t imx111_actuator_init_table(
	struct msm_actuator_ctrl_t *a_ctrl)
{
	int32_t rc = 0;
	int16_t step_index = 0;
	uint16_t dac_start = 0, dac_inf = 0, dac_1m = 0, dac_10cm = 0;
	uint16_t dac_inf_down = 0;
	uint8_t *af_calib = NULL;
	uint32_t dac_step = 0, dac_cur = 0;
	CDBG("%s called\n", __func__);

	if (a_ctrl->func_tbl.actuator_set_params)
	{
		rc = a_ctrl->func_tbl.actuator_set_params(a_ctrl);
        if(rc < 0)
            return rc;
    }
	/* Fill step position table */
	a_ctrl->step_position_table =
		kmalloc(sizeof(uint16_t) * (IMX111_TOTAL_STEPS_NEAR_TO_FAR_MAX + 1),
		GFP_KERNEL);
	a_ctrl->step_position_table[0] = a_ctrl->initial_code;

	/* Extract the calibration data */
	af_calib = a_ctrl->eeprom_client->data_tbl[1].data;
	dac_inf = af_calib[0] << 8 | af_calib[1];
	dac_1m = af_calib[2] << 8 | af_calib[3];
	dac_10cm = af_calib[4] << 8 | af_calib[5];
	dac_start = af_calib[6] << 8 | af_calib[7];

	dac_inf_down = dac_inf - 80;
	dac_cur = dac_inf_down << 8;
	a_ctrl->step_position_table[1] = dac_cur >> 8;

	dac_step = (((uint32_t)(dac_inf - dac_inf_down)) << 8) / 10;
	for (step_index = 2; step_index < 12; step_index++) {
		dac_cur += dac_step;
		a_ctrl->step_position_table[step_index] = dac_cur >> 8;
	}
	dac_cur = dac_inf << 8;
	dac_step = (((uint32_t)(dac_1m - dac_inf)) << 8) / 2;
	dac_cur += dac_step;
	a_ctrl->step_position_table[12] = dac_cur >> 8;
	dac_cur = dac_1m << 8;
	a_ctrl->step_position_table[13] = dac_1m;

	dac_step = (((uint32_t)(dac_10cm - dac_1m)) << 8) / 18;
	for (step_index = 14; step_index < 41; step_index++) {
		dac_cur += dac_step;
		a_ctrl->step_position_table[step_index] = dac_cur >> 8;
	}

	for (step_index = 0;
		step_index < a_ctrl->set_info.total_steps;
		step_index++) {
		CDBG("step_position_table[%d]= %d\n",
			step_index,
			a_ctrl->step_position_table[step_index]);
	}

	a_ctrl->curr_step_pos = 0;
	a_ctrl->curr_region_index = 0;

	return rc;
}

static int imx111_act_power_down(void *act_info)
{
	return (int) msm_actuator_af_power_down(&imx111_act_t);
}

static int32_t imx111_act_probe(
	void *board_info,
	void *sdev)
{
	struct msm_actuator_info *info;
	info = (struct msm_actuator_info *)board_info;
	imx111_act_t.eeprom_client =
		(struct msm_camera_eeprom_client *)info->eeprom_client;

	return (int) msm_actuator_create_subdevice(&imx111_act_t,
		info->board_info,
		(struct v4l2_subdev *)sdev);
}

static struct msm_actuator_ctrl_t imx111_act_t = {
	.i2c_driver = &imx111_act_i2c_driver,
	.i2c_addr = 0x18,
	.act_v4l2_subdev_ops = &imx111_act_subdev_ops,
	.actuator_ext_ctrl = {
		.a_init_table = imx111_i2c_add_driver_table,
		.a_create_subdevice = imx111_act_probe,
		.a_config = imx111_act_config,
		.a_power_down = imx111_act_power_down,
	},

	.i2c_client = {
		.addr_type = MSM_CAMERA_I2C_BYTE_ADDR,
	},

	.set_info = {
		.total_steps = IMX111_TOTAL_STEPS_NEAR_TO_FAR_MAX,
	},

	.curr_step_pos = 0,
	.curr_region_index = 0,
	.initial_code = 0,
	.actuator_mutex = &imx111_act_mutex,

	.get_info = {
		.focal_length_num = 295,
		.focal_length_den = 100,
		.f_number_num = 26,
		.f_number_den = 10,
		.f_pix_num = 112,
		.f_pix_den = 100,
		.total_f_dist_num = 149425,
		.total_f_dist_den = 1000,
		.hor_view_angle_num = 63,
		.hor_view_angle_den = 1,
		.ver_view_angle_num = 494,
		.ver_view_angle_den = 10,
	},

	/* Initialize scenario */
	.ringing_scenario[MOVE_NEAR] = imx111_macro_scenario,
	.scenario_size[MOVE_NEAR] = ARRAY_SIZE(imx111_macro_scenario),
	.ringing_scenario[MOVE_FAR] = imx111_inf_scenario,
	.scenario_size[MOVE_FAR] = ARRAY_SIZE(imx111_inf_scenario),

	/* Initialize region params */
	.region_params = imx111_regions,
	.region_size = ARRAY_SIZE(imx111_regions),

	/* Initialize damping params */
	.damping[MOVE_NEAR] = imx111_macro_regions,
	.damping[MOVE_FAR] = imx111_inf_regions,

	.func_tbl = {
		.actuator_set_params = imx111_set_params,
		.actuator_init_focus = NULL,
		.actuator_init_table = imx111_actuator_init_table,
		.actuator_move_focus = msm_actuator_move_focus,
		.actuator_write_focus = msm_actuator_write_focus,
		.actuator_i2c_write = imx111_wrapper_i2c_write,
		.actuator_set_default_focus = msm_actuator_set_default_focus,
	},

};

subsys_initcall(imx111_i2c_add_driver);
MODULE_DESCRIPTION("IMX111 actuator");
MODULE_LICENSE("GPL v2");
