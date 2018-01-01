// Copyright 2006 The Android Open Source Project

/**********************************************************************
* Modified: 2012/04/18
* (C) NEC CASIO Mobile Communications, Ltd. 2009-2012
**********************************************************************/
#ifndef _LINUX_BACKUPLOG_H
#define _LINUX_BACKUPLOG_H

#include "../../../../kernel/arch/arm/mach-msm/include/mach/msm_iomap.h"
#include "../../../../kernel/arch/arm/mach-msm/include/mach/msm_iomap-8960.h"

#define ARMLOG_MSG_UNDEF		"Event:0x33"
#define ARMLOG_MSG_PABRT		"Event:0x34"
#define ARMLOG_MSG_DABRT		"Event:0x35"
#define ARMLOG_MSG_OOPS			"Event:0x38"
#define ARMLOG_MSG_AUTONOMOUS	"Event:0x3C"
#define	BACKUP_OFFSET_ADDR		0x00200000
#define	BACKUP_START_ADDR		(MSM_OEM_NCMC_DB_UINIT_BASE + BACKUP_OFFSET_ADDR)
#define KTRACE_MSG_SIZE			1024
#define	BACKUP_START_ADDR_ARM	((1024 * 700) + BACKUP_START_ADDR)
#define BACKUP_ARM_FLG_ADDR		BACKUP_START_ADDR_ARM 
#define BACKUP_ARM_CNT_ADDR		(BACKUP_ARM_FLG_ADDR + 0x10)
#define BACKUP_ARM_DAT_ADDR		(BACKUP_ARM_CNT_ADDR + 0x10)
#define BACKUP_ARM_DAT_SIZE		0xFE0
#define	BACKUP_ARM_DAT_FULL		(BACKUP_ARM_DAT_ADDR + BACKUP_ARM_DAT_SIZE)
#define BACKUP_ARM_TRUE			0x7F
#define BACKUP_ARM_FALSE		0xFF
#define BACKUP_ARM_STR_LEN		28
#define BACKUP_ARM_SEP_SIZE		1
#define BACKUP_ARM_STR_OFFSET	BACKUP_ARM_STR_LEN + BACKUP_ARM_SEP_SIZE


#endif /* _LINUX_BACKUPLOG_H */





