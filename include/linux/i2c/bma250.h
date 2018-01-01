/*
 * Copyright (c) 2010-2012 Yamaha Corporation
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */
/***********************************************************************/
/* Modified by                                                         */
/* (C) NEC CASIO Mobile Communications, Ltd. 2013                      */
/***********************************************************************/


#ifndef BMA250_DRIVER_H
#define BMA250_DRIVER_H

//#include "yas_cfg.h"
#include <linux/ioctl.h>


/* ----------------------------------------------------------------------------
 *                             Typedef definition
 *--------------------------------------------------------------------------- */



/* ----------------------------------------------------------------------------
 *                              Macro definition
 *--------------------------------------------------------------------------- */

#define BMA250_SET_BITSLICE(regvar, bitname, val)\
	((regvar & ~bitname##__MSK) | ((val<<bitname##__POS)&bitname##__MSK))

#define BMA250_GET_BITSLICE(regvar, bitname)\
	((regvar & bitname##__MSK) >> bitname##__POS)


#define BMA250_IO	'b'

#define BMA250_IOCTL_MEASURE_START						_IO(BMA250_IO, 0)
#define BMA250_IOCTL_MEASURE_STOP						_IO(BMA250_IO, 1)
////#define BMA250_IOCTL_GET_NV								_IOR(BMA250_IO, 2, struct bma250_nv_data)
////#define BMA250_IOCTL_SET_NV								_IOW(BMA250_IO, 3, struct bma250_nv_data)
#define BMA250_IOCTL_GET_CALC_DATA						_IOR(BMA250_IO, 4, struct bma250_accel_measurement_xyzdata)
#define BMA250_IOCTL_GET_TEMPERATURE					_IOR(BMA250_IO, 5, unsigned char)
#define BMA250_IOCTL_DIAG_REG_INIT						_IOR(BMA250_IO, 6, struct bma250_readreg_data)
#define BMA250_IOCTL_DIAG_CALIBLATION					_IOR(BMA250_IO, 7, struct bma250_nv_data)
#define BMA250_IOCTL_DIAG_SELF_TEST						_IOR(BMA250_IO, 8, struct bma250_accel_stest_xyzdata)
#define BMA250_IOCTL_DIAG_RESET							_IO(BMA250_IO, 10)
#define BMA250_IOCTL_READ_REGDATA						_IOR(BMA250_IO, 11, struct bma250_regist_data)
#define BMA250_IOCTL_WRITE_REGDATA						_IOW(BMA250_IO, 12, struct bma250_regist_data)


#define BMA250_DIAG_CHIP_ID_REG						0x00
//#define BMA250_XXXX_REG						0x01//reserved
#define BMA250_DIAG_ACC_DATA_X_LSB_REG				0x02
#define BMA250_DIAG_ACC_DATA_X_MSB_REG				0x03
#define BMA250_DIAG_ACC_DATA_Y_LSB_REG				0x04
#define BMA250_DIAG_ACC_DATA_Y_MSB_REG				0x05
#define BMA250_DIAG_ACC_DATA_Z_LSB_REG				0x06
#define BMA250_DIAG_ACC_DATA_Z_MSB_REG				0x07
//#define BMA250_DIAG_TEMPERATURE_DATA_REG			0x08
#define BMA250_DIAG_STATUS_REG						0x09
#define BMA250_DIAG_DATA_INT_REG					0x0A
#define BMA250_DIAG_TAP_AND_SLOPE_INT_REG			0x0B
#define BMA250_DIAG_FLAT_AND_ORIENTATION_REG		0x0C
//#define BMA250_XXXX_REG						0x0D//reserved
#define BMA250_DIAG_FIFO_STATUS_REG                 0x0E
#define BMA250_DIAG_G_RANGE_REG						0x0F

#define BMA250_DIAG_BANDWIDTHS_REG					0x10
#define BMA250_DIAG_POWER_MODE_REG					0x11
#define BMA250_DIAG_PMU_LOW_NOISE_REG               0x12
#define BMA250_DIAG_ACCD_BHW_REG                    0x13
#define BMA250_DIAG_SOFTRESET_REG					0x14
//#define BMA250_XXXX_REG						0x15//reserved
#define BMA250_DIAG_INT_EN_0_REG                    0x16
#define BMA250_DIAG_INT_EN_1_REG                    0x17
#define BMA250_DIAG_INT_EN_2_REG                    0x18
#define BMA250_DIAG_INT_MAPPING1_REG				0x19
#define BMA250_DIAG_INT_MAPPING2_REG				0x1A
#define BMA250_DIAG_INT_MAPPING3_REG				0x1B
//#define BMA250_XXXX_REG						0x1C//reserved
//#define BMA250_XXXX_REG						0x1D//reserved
#define BMA250_DIAG_INT_SRC_DEF_REG					0x1E
//#define BMA250_XXXX_REG						0x1F//reserved

#define BMA250_DIAG_ELC_INT_REG						0x20
#define BMA250_DIAG_INT_RESET_MODE_REG				0x21
#define BMA250_DIAG_DELAY_TIME_DEF_LOWDUR_REG		0x22
#define BMA250_DIAG_THRESHOLD_DEF_LOWTH_REG			0x23
#define BMA250_DIAG_THRESHOLD_DEF_HIGHHY_REG		0x24
#define BMA250_DIAG_DELAY_TIME_DEF_HIGHDUR_REG		0x25
#define BMA250_DIAG_THRESHOLD_DEF_HIGHTH_REG		0x26
#define BMA250_DIAG_SLOPE_INT_NUM_DUR_REG			0x27
#define BMA250_DIAG_SLOPE_INT_NUM_TH_REG			0x28
#define BMA250_DIAG_INT_7_REG                       0x29
#define BMA250_DIAG_TAP_DURATION_REG				0x2A
#define BMA250_DIAG_TAP_INT_REG						0x2B
#define BMA250_DIAG_BLOCK_ORI_INT_REG				0x2C
#define BMA250_DIAG_THETA_BLOCK_ANGLE_REG			0x2D
#define BMA250_DIAG_FLAT_THRESHOLD_ANGLE_REG		0x2E
#define BMA250_DIAG_FLAT_THRESHOLD_TIME_REG			0x2F

#define BMA250_DIAG_FIFO_CONFIG_0_REG               0x30
//#define BMA250_XXXX_REG						0x31//reserved
#define BMA250_DIAG_SENSOR_SELF_TEST_REG			0x32
#define BMA250_DIAG_EEPROM_CTRL_NVM_REG				0x33
#define BMA250_DIAG_EEPROM_CTRL_I2C_REG				0x34
//#define BMA250_XXXX_REG						0x35//reserved
#define BMA250_DIAG_OFFSET_CTRL_REG					0x36
#define BMA250_DIAG_OFFSET_PARAMS_REG				0x37
#define BMA250_DIAG_X_DATA_FILTER_REG				0x38
#define BMA250_DIAG_Y_DATA_FILTER_REG				0x39
#define BMA250_DIAG_Z_DATA_FILTER_REG				0x3A
#define BMA250_DIAG_TRIM_GP0_REG                    0x3B
#define BMA250_DIAG_TRIM_GP1_REG                    0x3C
//#define BMA250_DIAG_Z_DATA_UNFILTER_REG				0x3D
#define BMA250_DIAG_FIFO_CONFIG_1_REG               0x3E
#define BMA250_DIAG_FIFO_DATA_REG                   0x3F


#define BMA250_ACC_X_LSB_NEW_DATA__POS           0
#define BMA250_ACC_X_LSB_NEW_DATA__LEN           1
#define BMA250_ACC_X_LSB_NEW_DATA__MSK           0x01
#define BMA250_ACC_X_LSB_NEW_DATA__REG           BMA250_DIAG_ACC_DATA_X_LSB_REG

#define BMA250_ACC_X_LSB__POS           6
#define BMA250_ACC_X_LSB__LEN           2
#define BMA250_ACC_X_LSB__MSK           0xC0
#define BMA250_ACC_X_LSB__REG           BMA250_DIAG_ACC_DATA_X_LSB_REG

#define BMA250_ACC_X_MSB__POS           0
#define BMA250_ACC_X_MSB__LEN           8
#define BMA250_ACC_X_MSB__MSK           0xFF
#define BMA250_ACC_X_MSB__REG           BMA250_DIAG_ACC_DATA_X_MSB_REG

#define BMA250_ACC_Y_LSB_NEW_DATA__POS           0
#define BMA250_ACC_Y_LSB_NEW_DATA__LEN           1
#define BMA250_ACC_Y_LSB_NEW_DATA__MSK           0x01
#define BMA250_ACC_Y_LSB_NEW_DATA__REG           BMA250_DIAG_ACC_DATA_Y_LSB_REG

#define BMA250_ACC_Y_LSB__POS           6
#define BMA250_ACC_Y_LSB__LEN           2
#define BMA250_ACC_Y_LSB__MSK           0xC0
#define BMA250_ACC_Y_LSB__REG           BMA250_DIAG_ACC_DATA_Y_LSB_REG

#define BMA250_ACC_Y_MSB__POS           0
#define BMA250_ACC_Y_MSB__LEN           8
#define BMA250_ACC_Y_MSB__MSK           0xFF
#define BMA250_ACC_Y_MSB__REG           BMA250_DIAG_ACC_DATA_Y_MSB_REG

#define BMA250_ACC_Z_LSB_NEW_DATA__POS           0
#define BMA250_ACC_Z_LSB_NEW_DATA__LEN           1
#define BMA250_ACC_Z_LSB_NEW_DATA__MSK           0x01
#define BMA250_ACC_Z_LSB_NEW_DATA__REG           BMA250_DIAG_ACC_DATA_Z_LSB_REG

#define BMA250_ACC_Z_LSB__POS           6
#define BMA250_ACC_Z_LSB__LEN           2
#define BMA250_ACC_Z_LSB__MSK           0xC0
#define BMA250_ACC_Z_LSB__REG           BMA250_DIAG_ACC_DATA_Z_LSB_REG

#define BMA250_ACC_Z_MSB__POS           0
#define BMA250_ACC_Z_MSB__LEN           8
#define BMA250_ACC_Z_MSB__MSK           0xFF
#define BMA250_ACC_Z_MSB__REG           BMA250_DIAG_ACC_DATA_Z_MSB_REG

// register[0x32] bit[1:0]
#define BMA250_EN_SELF_TEST__POS        0
#define BMA250_EN_SELF_TEST__LEN        2
#define BMA250_EN_SELF_TEST__MSK        0x03
#define BMA250_EN_SELF_TEST__REG        BMA250_DIAG_SENSOR_SELF_TEST_REG

// register[0x32] bit[2]
#define BMA250_NEG_SELF_TEST__POS       2
#define BMA250_NEG_SELF_TEST__LEN       1
#define BMA250_NEG_SELF_TEST__MSK       0x04
#define BMA250_NEG_SELF_TEST__REG       BMA250_DIAG_SENSOR_SELF_TEST_REG

// register[0x32] bit[4]
#define BMA250_AMP_SELF_TEST__POS       4
#define BMA250_AMP_SELF_TEST__LEN       1
#define BMA250_AMP_SELF_TEST__MSK       0x10
#define BMA250_AMP_SELF_TEST__REG       BMA250_DIAG_SENSOR_SELF_TEST_REG

#define BMA250_COMP_TARGET_OFFSET_X__POS        1
#define BMA250_COMP_TARGET_OFFSET_X__LEN        2
#define BMA250_COMP_TARGET_OFFSET_X__MSK        0x06
#define BMA250_COMP_TARGET_OFFSET_X__REG        BMA250_DIAG_OFFSET_PARAMS_REG

#define BMA250_COMP_TARGET_OFFSET_Y__POS        3
#define BMA250_COMP_TARGET_OFFSET_Y__LEN        2
#define BMA250_COMP_TARGET_OFFSET_Y__MSK        0x18
#define BMA250_COMP_TARGET_OFFSET_Y__REG        BMA250_DIAG_OFFSET_PARAMS_REG

#define BMA250_COMP_TARGET_OFFSET_Z__POS        5
#define BMA250_COMP_TARGET_OFFSET_Z__LEN        2
#define BMA250_COMP_TARGET_OFFSET_Z__MSK        0x60
#define BMA250_COMP_TARGET_OFFSET_Z__REG        BMA250_DIAG_OFFSET_PARAMS_REG

#define BMA250_FAST_COMP_RDY_S__POS             4
#define BMA250_FAST_COMP_RDY_S__LEN             1
#define BMA250_FAST_COMP_RDY_S__MSK             0x10
#define BMA250_FAST_COMP_RDY_S__REG             BMA250_DIAG_OFFSET_CTRL_REG

#define BMA250_EN_FAST_COMP__POS                5
#define BMA250_EN_FAST_COMP__LEN                2
#define BMA250_EN_FAST_COMP__MSK                0x60
#define BMA250_EN_FAST_COMP__REG                BMA250_DIAG_OFFSET_CTRL_REG


#define BMA250_EEPROM_CTRL_NVM__POS             0
#define BMA250_EEPROM_CTRL_NVM__LEN             1
#define BMA250_EEPROM_CTRL_NVM__MSK             0x01
#define BMA250_EEPROM_CTRL_NVM__REG             BMA250_DIAG_EEPROM_CTRL_NVM_REG

#define BMA250_EEPROM_CTRL_NVM_TRIG__POS        1
#define BMA250_EEPROM_CTRL_NVM_TRIG__LEN        1
#define BMA250_EEPROM_CTRL_NVM_TRIG__MSK        0x02
#define BMA250_EEPROM_CTRL_NVM_TRIG__REG        BMA250_DIAG_EEPROM_CTRL_NVM_REG

#define BMA250_EEPROM_CTRL_NVM_RDY__POS         2
#define BMA250_EEPROM_CTRL_NVM_RDY__LEN         1
#define BMA250_EEPROM_CTRL_NVM_RDY__MSK         0x04
#define BMA250_EEPROM_CTRL_NVM_RDY__REG         BMA250_DIAG_EEPROM_CTRL_NVM_REG


#define D_SENSOR_NV_NUM 3

struct bma250_nv_data {
	unsigned char nv_reg[D_SENSOR_NV_NUM];
};


/* ----------------------------------------------------------------------------
 *                            Structure definition
 *--------------------------------------------------------------------------- */

struct bma250_readreg_data
{
  unsigned char read_register_address;
  unsigned char read_register_val; 
};

struct bma250_regist_data 
{
	unsigned char	regist_addr;
	unsigned char	regist_data;
};

typedef struct bma250_accel_measurement_xyzdata
{
    int32_t x; /**< holds x-axis acceleration data sign extended. Range -512 to 511. */
    int32_t y; /**< holds y-axis acceleration data sign extended. Range -512 to 511. */
    int32_t z; /**< holds z-axis acceleration data sign extended. Range -512 to 511. */
    int32_t raw_x;
    int32_t raw_y;
    int32_t raw_z;
} bma250_accel_measurement_xyzdata;

typedef struct bma250_accel_stest_xyzdata
{
    short x_plus; /**< holds x-axis acceleration data sign extended. Range -512 to 511. */
    short x_minus; /**< holds x-axis acceleration data sign extended. Range -512 to 511. */
    short y_plus; /**< holds y-axis acceleration data sign extended. Range -512 to 511. */
    short y_minus; /**< holds y-axis acceleration data sign extended. Range -512 to 511. */
    short z_plus; /**< holds z-axis acceleration data sign extended. Range -512 to 511. */
    short z_minus; /**< holds z-axis acceleration data sign extended. Range -512 to 511. */
    int   test_result;
} bma250_accel_stest_xyzdata;


#endif /* BMA250_DRIVER_H */
