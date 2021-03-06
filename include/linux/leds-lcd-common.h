﻿/*
 * leds-lcd-common.h - platform data structure for LCD-backlight led controller
 *
 * Copyright (C) NEC CASIO Mobile Communications, Ltd.
 *
 */
 

#ifndef __LEDS_LCD_COMMON_H
#define __LEDS_LCD_COMMON_H

#ifdef CONFIG_FEATURE_NCMC_D121M
#define LEDS_LED_SET_OK			LM3532_LED_SET_OK
#define LEDS_LED_SET_NG			LM3532_LED_SET_NG
#define LEDS_LED_SET_RESERVED	LM3532_LED_SET_RESERVED
#define LEDS_LED_ON				LM3532_LED_ON
#define LEDS_LED_OFF			LM3532_LED_OFF
#include <linux/leds-lm3532.h>
#else /* #ifdef CONFIG_FEATURE_NCMC_D121M */

#ifdef CONFIG_FEATURE_NCMC_G121S
#define LEDS_LED_SET_OK			LM3537_LED_SET_OK
#define LEDS_LED_SET_NG			LM3537_LED_SET_NG
#define LEDS_LED_SET_RESERVED	LM3537_LED_SET_RESERVED
#define LEDS_LED_ON				LM3537_LED_ON
#define LEDS_LED_OFF			LM3537_LED_OFF
#include <linux/leds-lm3537.h>
#else /* #ifdef CONFIG_FEATURE_NCMC_G121S */

#ifdef CONFIG_FEATURE_NCMC_D121F
#define LEDS_LED_SET_OK			LM3537_LED_SET_OK
#define LEDS_LED_SET_NG			LM3537_LED_SET_NG
#define LEDS_LED_SET_RESERVED	LM3537_LED_SET_RESERVED
#define LEDS_LED_ON				LM3537_LED_ON
#define LEDS_LED_OFF			LM3537_LED_OFF
#include <linux/leds-lm3537.h>
#else /* #ifdef CONFIG_FEATURE_NCMC_D121F */

#ifdef CONFIG_FEATURE_NCMC_GEKKO
#define LEDS_LED_SET_OK			LM3537_LED_SET_OK
#define LEDS_LED_SET_NG			LM3537_LED_SET_NG
#define LEDS_LED_SET_RESERVED	LM3537_LED_SET_RESERVED
#define LEDS_LED_ON				LM3537_LED_ON
#define LEDS_LED_OFF			LM3537_LED_OFF
#include <linux/leds-lm3537.h>
#else /* #ifdef CONFIG_FEATURE_NCMC_GEKKO */

#ifdef CONFIG_FEATURE_NCMC_ALEX
#define LEDS_LED_SET_OK			LM3532_LED_SET_OK
#define LEDS_LED_SET_NG			LM3532_LED_SET_NG
#define LEDS_LED_SET_RESERVED	LM3532_LED_SET_RESERVED
#define LEDS_LED_ON				LM3532_LED_ON
#define LEDS_LED_OFF			LM3532_LED_OFF
#include <linux/leds-lm3532.h>
#else /* #ifdef CONFIG_FEATURE_NCMC_ALEX */

#ifdef CONFIG_FEATURE_NCMC_RAPIDFIRE
#define LEDS_LED_SET_OK			LM3537_LED_SET_OK
#define LEDS_LED_SET_NG			LM3537_LED_SET_NG
#define LEDS_LED_SET_RESERVED	LM3537_LED_SET_RESERVED
#define LEDS_LED_ON				LM3537_LED_ON
#define LEDS_LED_OFF			LM3537_LED_OFF
#include <linux/leds-lm3537.h>
#else /* #ifdef CONFIG_FEATURE_NCMC_RAPIDFIRE */

#define LEDS_LED_SET_OK			LM3537_LED_SET_OK
#define LEDS_LED_SET_NG			LM3537_LED_SET_NG
#define LEDS_LED_SET_RESERVED		LM3537_LED_SET_RESERVED
#define LEDS_LED_ON				LM3537_LED_ON
#define LEDS_LED_OFF				LM3537_LED_OFF
#include <linux/leds-lm3537.h>

#endif /* #ifdef CONFIG_FEATURE_NCMC_RAPIDFIRE */
#endif /* #ifdef CONFIG_FEATURE_NCMC_ALEX */
#endif /* #ifdef CONFIG_FEATURE_NCMC_GEKKO */
#endif /* #ifdef CONFIG_FEATURE_NCMC_D121F */
#endif /* #ifdef CONFIG_FEATURE_NCMC_G121S */
#endif /* #ifdef CONFIG_FEATURE_NCMC_D121M */

#endif /* __LEDS_LCD_COMMON_H */

/* File END */
