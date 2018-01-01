#
# Makefile for miscellaneous I2C chip drivers.
#
# Do not add new drivers to this directory! It is DEPRECATED.
#
# Device drivers are better grouped according to the functionality they
# implement rather than to the bus they are connected to. In particular:
# * Hardware monitoring chip drivers go to drivers/hwmon
# * RTC chip drivers go to drivers/rtc
# * I/O expander drivers go to drivers/gpio
#

ifeq ($(CONFIG_I2C_DEBUG_CHIP),y)
EXTRA_CFLAGS += -DDEBUG
endif


obj-$(CONFIG_SENSORS_MT9T013)   += mt9t013.o
obj-$(CONFIG_SENSORS_APDS990X)  += apds9900.o
obj-y += sensors_common.o
obj-$(CONFIG_SENSORS_GP2AP012A00F) += gp2ap012a00f.o

ifeq ($(CONFIG_FEATURE_NCMC_RUBY),y)
obj-$(CONFIG_INPUT_YAS_ACCELEROMETER) += yas_acc_kernel_driver.o
obj-$(CONFIG_INPUT_YAS_MAGNETOMETER) += yas_mag_kernel_driver.o
obj-$(CONFIG_INPUT_YAS_ORIENTATION) += yas_ori_kernel_driver.o
else
obj-$(CONFIG_SENSORS_AKM8977)   += akm8977.o
obj-$(CONFIG_SENSORS_AKM8977)   += akm8977_table.o
obj-$(CONFIG_SENSORS_AKM8977)   += akm8977_queue.o
endif

EXTRA_CFLAGS += -mfloat-abi=softfp



ifneq ($(CONFIG_FEATURE_NCMC_RUBY),y)
obj-$(CONFIG_BD91401GW)     += bd91401gw.o
endif

obj-$(CONFIG_BD91401GW)     += usb_switch_if_knl.o


ifeq ($(CONFIG_I2C_DEBUG_CHIP),y)
EXTRA_CFLAGS += -DDEBUG
endif
