
#ifndef _LINUX_SPI_EKTF2136_H
#define _LINUX_SPI_EKTF2136_H

#define ELAN_TS_NAME	"elan_ts_spi"

#define ELAN_TS_X_MAX 	576
#define ELAN_TS_Y_MAX	704

#define SCREEN_X_MAX	480
#define SCREEN_Y_MAX	640


/*! @brief \b platform data
 * @param intr_gpio : gpio pin
 * @param rst_gpio : interuption pin
 * @param cs_gpio : spi_cs pin
 *
 * The platform data, all platform dependent variable should put here. 
 * intr_gpio & rst_gpio is necessary.
 *
 */
struct elan_spi_platform_data {
	unsigned int screen_x_min;
	unsigned int screen_x_max;
	unsigned int screen_y_min;
	unsigned int screen_y_max;
	int intr_gpio;
	int rst_gpio;
	int cs_gpio;
	int (*setup)(struct device *);
};

/*! @brief \b ioctl command definition.
 * @param IOCTL_MAJOR_FW_VER : fw major number
 * @param IOCTL_MINOR_FW_VER : fw minor number
 * @param IOCTL_MAJOR_HW_ID : hw major number
 * @param IOCTL_MINOR_HW_ID : hw minor number
 * @param IOCTL_RESET : Hardware Reset
 * @param IOCTL_SET_COMMAND : control command set to TP device.
 * @param IOCTL_GET_COMMAND : get response from our TP device.
 * @param IOCTL_IAP_ENABLE : Enter IAP mode
 * @param IOCTL_IAP_DISABLE : Leave IAP mode
 * @param IOCTL_INT : gpio status
 * @param IOCTL_IAP_MODE : IAP mode query.
 *
 *
 * ioctl command, easy user to control our TP from application.
 *
 */
#define ELAN_IOCTLID  		0xD0
#define IOCTL_MAJOR_FW_VER  _IOR(ELAN_IOCTLID, 2, int)
#define IOCTL_MINOR_FW_VER  _IOR(ELAN_IOCTLID, 3, int)
#define IOCTL_RESET  		_IOW(ELAN_IOCTLID, 4, int)
#define IOCTL_MAJOR_HW_ID	_IOR(ELAN_IOCTLID, 5, int)
#define IOCTL_MINOR_HW_ID	_IOW(ELAN_IOCTLID, 6, int)
#define IOCTL_IAP_MODE		_IOR(ELAN_IOCTLID, 7, int)
#define IOCTL_INT  			_IOR(ELAN_IOCTLID, 8, int)
#define IOCTL_SET_COMMAND	_IOW(ELAN_IOCTLID, 9, unsigned long)
#define IOCTL_GET_COMMAND	_IOR(ELAN_IOCTLID, 10, unsigned long)
#define IOCTL_IAP_ENABLE	_IOW(ELAN_IOCTLID, 11, int)
#define IOCTL_IAP_DISABLE	_IOW(ELAN_IOCTLID, 12, int)
#define IOCTL_PS_SLEEP		_IOW(ELAN_IOCTLID, 13, int)
#define IOCTL_PS_WAKE_UP	_IOW(ELAN_IOCTLID, 14, int)


#endif /* _LINUX_SPI_EKTF2136_H */
