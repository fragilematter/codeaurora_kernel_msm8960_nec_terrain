

#ifndef __LINUX_LEDS_CMD_H
#define __LINUX_LEDS_CMD_H

#ifndef CONFIG_FEATURE_NCMC_CAM_LIGHT_PREVENT_PEEPING
#ifdef FEATURE_NCMC_CAM_LIGHT_PREVENT_PEEPING
#define CONFIG_FEATURE_NCMC_CAM_LIGHT_PREVENT_PEEPING
#endif // #ifdef FEATURE_NCMC_CAM_LIGHT_PREVENT_PEEPING
#endif // #ifndef CONFIG_FEATURE_NCMC_CAM_LIGHT_PREVENT_PEEPING

#ifndef CONFIG_FEATURE_NCMC_GG2
#ifdef FEATURE_NCMC_GG2
#define CONFIG_FEATURE_NCMC_GG2
#endif // #ifdef FEATURE_NCMC_GG2
#endif	//#ifndef CONFIG_FEATURE_NCMC_GG2

/* Command ID */
#define LEDS_CMD_TYPE_RGB_RED			0x01
#define LEDS_CMD_TYPE_RGB_GREEN			0x02
#define LEDS_CMD_TYPE_RGB_BLUE			0x03
#define LEDS_CMD_TYPE_KEY				0x04
#define LEDS_CMD_TYPE_FLASH				0x05
#ifdef CONFIG_FEATURE_NCMC_PEACOCK
#define LEDS_CMD_TYPE_PREVENT_PEEPING	0x06


#define ILLUMI_CMD_FRONT_LED1		0x01
#define ILLUMI_CMD_FRONT_LED2		0x02
#define ILLUMI_CMD_BACK_LED1		0x03
#define ILLUMI_CMD_BACK_LED2		0x04

#define ILLUMI_COLOR_RED			0x01
#define ILLUMI_COLOR_GREEN			0x02
#define ILLUMI_COLOR_BLUE			0x03
#endif
/* Return value */
#define LEDS_CMD_RET_OK				1
#define LEDS_CMD_RET_NG				0

/* LED brightness */
#ifdef CONFIG_FEATURE_NCMC_PEACOCK
#define LEDS_CMD_RGB_RED1			0x32
#define LEDS_CMD_RGB_GREEN1			0x32
#define LEDS_CMD_RGB_BLUE1			0x32
#define LEDS_CMD_RGB_RED2			0x1E
#define LEDS_CMD_RGB_GREEN2			0x1E
#define LEDS_CMD_RGB_BLUE2			0x1E
#define LEDS_CMD_RGB_RED3			0x19
#define LEDS_CMD_RGB_GREEN3			0x14
#define LEDS_CMD_RGB_BLUE3			0x0F
#define LEDS_CMD_FLASH_TORCH		0x0B
#define LEDS_CMD_PREVENT_PEEPING	0x03

#define ILLUMI_CMD_RGB_RED1			0x19
#define ILLUMI_CMD_RGB_GREEN1		0x19
#define ILLUMI_CMD_RGB_BLUE1		0x19
#define ILLUMI_CMD_RGB_RED2			0x0F
#define ILLUMI_CMD_RGB_GREEN2		0x0F
#define ILLUMI_CMD_RGB_BLUE2		0x0F
#define ILLUMI_CMD_RGB_RED3			0x0D
#define ILLUMI_CMD_RGB_GREEN3		0x0A
#define ILLUMI_CMD_RGB_BLUE3		0x08
#endif

/* Command function */
extern unsigned char leds_cmd(unsigned char type, unsigned char val);
#ifdef CONFIG_FEATURE_NCMC_PEACOCK
extern unsigned char illumi_cmd(int dev, int color, unsigned char val);
#endif
/* misc */
#define BD6082GUL_IOCTL_MAGIC 'l'

#define LED_CLASS_WRAPPER_FD_PATH    "/dev/led_class_wrapper"

struct bd6082gul_led_flash_parame {
	char still;
	char video;
	char torch;
};

#ifdef CONFIG_FEATURE_NCMC_GG2
struct bd6082gul_led_reg_indicator_parame {
	char ind_r;
};

struct bd6082gul_led_green_indicator_parame {
	char ind_g;
};
#endif	//#ifdef CONFIG_FEATURE_NCMC_GG2

struct bd6082gul_led_prevent_peeping_parame {
	char prevent_peeping ;
};

#define BD6082GUL_CUSTOM_N40    _IOW(BD6082GUL_IOCTL_MAGIC, 0x40, struct bd6082gul_led_flash_parame *)
#ifdef CONFIG_FEATURE_NCMC_GG2
#define BD6082GUL_CUSTOM_N41    _IOW(BD6082GUL_IOCTL_MAGIC, 0x41, struct bd6082gul_led_reg_indicator_parame *)
#define BD6082GUL_CUSTOM_N42    _IOW(BD6082GUL_IOCTL_MAGIC, 0x42, struct bd6082gul_led_green_indicator_parame *)
#endif	//#ifdef CONFIG_FEATURE_NCMC_GG2
#define BD6082GUL_CUSTOM_N43    _IOW(BD6082GUL_IOCTL_MAGIC, 0x43, struct bd6082gul_led_prevent_peeping_parame *)


#endif /* __LINUX_LEDS_CMD_H */
