/*
 * Definitions for I2C sensors.
 */
#ifndef I2C_SENSORS_H
#define I2C_SENSORS_H

#ifndef __KERNEL__
#include <stdint.h>
#endif

#include "apds990x_driver.h"
//#include "gp2ap012a00f.h"
#include "../akm8977.h"
//#include "bma150_driver.h"  /* 3axis sensor */
//#include "smb380.h"         /* 3axis sensor */
//#include "BMA150calib.h"    /* 3axis sensor */
#include "gysdcom.h"

struct delay_time
{
    unsigned long type;
    int64_t time;
};

#define SENSORS_COMMON_IO               'C'

#define D_IOCTL_SET_ACTIVE_SENSOR       _IOWR(SENSORS_COMMON_IO, 0x01, long)
#define D_IOCTL_GET_ACTIVE_SENSOR       _IOWR(SENSORS_COMMON_IO, 0x02, long)
#define D_IOCTL_GET_DELAY_TIME          _IOWR(SENSORS_COMMON_IO, 0x05, struct delay_time)
#define D_IOCTL_SET_DELAY_TIME          _IOWR(SENSORS_COMMON_IO, 0x06, struct delay_time)
#define D_IOCTL_WAIT_GET_ACTIVE_SENSOR  _IOWR(SENSORS_COMMON_IO, 0x07, long)

#endif

