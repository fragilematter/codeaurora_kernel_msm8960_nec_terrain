// Copyright 2006 The Android Open Source Project

/**********************************************************************
* Modified: 2012/01/06
* (C) NEC CASIO Mobile Communications, Ltd. 2009-2012
**********************************************************************/
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/string.h>

#include "logger.h"
#include "backuplog.h"


#define BACKUP_HEAD_STR		"### Backup Logcat ###\n"
#define BACKUP_TERM_STR		"<<<END:Logcat>>>"
#define BACKUP_FILL_CHR		' '

#define BACKUP_MESSAGE_SIZE		( 1024 * 4 )

#define BACKUP_NULLAREA_SIZE	BACKUP_MESSAGE_SIZE
#define BACKUP_MEMORY_SIZE		( ( 1024 * 700 ) - BACKUP_NULLAREA_SIZE )

static char*	backup_start_ptr	= BACKUP_START_ADDR;
static char*	backup_over_ptr		= ( BACKUP_START_ADDR + BACKUP_MEMORY_SIZE );
static char*	backup_write_ptr	= BACKUP_START_ADDR;
static int		backup_term_len;

static char		backup_msg[BACKUP_MESSAGE_SIZE];

/* ref. apps/LINUX/android/frameworks/base/core/java/android/util/Log.java */
static char		backup_level[6][3] = {
	/* 2         3         4         5         6         7   */
	{ "V/" }, { "D/" }, { "I/" }, { "W/" }, { "E/" }, { "A/" }
};


static int	*g_arm_flg_addr   = BACKUP_ARM_FLG_ADDR;
static int	*g_arm_cnt        = BACKUP_ARM_CNT_ADDR;
static char *g_arm_read_addr  = BACKUP_ARM_DAT_ADDR;
static char	g_arm_temp_buff[BACKUP_ARM_DAT_SIZE];
static char	g_ktrace_msg[KTRACE_MSG_SIZE];


unsigned int backup_log_edit( struct logger_entry* entry )
{
	unsigned int retlen;
	char* entmsg = entry->msg;

	char level;
	int taglen;
	int msglen;

	char* buffp = backup_msg;

	if( 0 >= entry->len ) {
		return 0;
	}

	memset( backup_msg, 0, sizeof( backup_msg ) );

	sprintf( buffp, "%d.%09d ", entry->sec, entry->nsec );
	buffp += strlen( buffp );

	level = *entmsg;
	if( 2 <= level && level <= 6 ) {
		strcpy( buffp, backup_level[ level-2 ] );
		buffp += strlen( backup_level[ level-2 ] );
	}
	else {
		*buffp = level;
		buffp ++;
	}
	entmsg ++;

	taglen = strlen( entmsg );
	if( 0 < taglen ) {
#ifdef CONFIG_FEATURE_NCMC_RUBY
		if (  taglen > (BACKUP_MESSAGE_SIZE - (buffp - backup_msg) - 9) ){
			taglen = BACKUP_MESSAGE_SIZE - (buffp - backup_msg) - 9;
		}
#endif
		memcpy( buffp, entmsg, taglen );
		buffp += taglen;
		entmsg += taglen;
	}
	if( 0 <= taglen && 8 > taglen ) {
		memset( buffp, BACKUP_FILL_CHR, (8 - taglen) );
		buffp += (8 - taglen);
	}
	entmsg ++;


	sprintf( buffp, "(%5d): ", entry->pid );
	buffp += 9;
	retlen = buffp - backup_msg;


	msglen = ((entry->len) - taglen - 2);
	if( BACKUP_MESSAGE_SIZE < (msglen + retlen) ) {	
		msglen = ( BACKUP_MESSAGE_SIZE - retlen );
	}

	memcpy( buffp, entmsg, msglen );
	retlen += msglen;
	if( '\n' == backup_msg[ retlen-2 ] ) {
		retlen --;
	}
	else if( '\n' != backup_msg[ retlen-1 ] ) {
		backup_msg[ retlen-1 ] = '\n';
	}

	return retlen;

}


void backup_log( struct logger_entry* entry )
{
	unsigned int len;
	char* added_ptr;


	len = backup_log_edit( entry );
	if( 0 >= len ) {

		return ;
	}

	added_ptr = ( backup_write_ptr + len + backup_term_len );

	if( backup_over_ptr <= added_ptr ) {

		int clear_len = (int)( backup_over_ptr - backup_write_ptr );
		if( 0 < clear_len ) {
			memset( backup_write_ptr, 0, clear_len );
		}
		backup_write_ptr = backup_start_ptr;
	}

	memcpy( backup_write_ptr, backup_msg, len );
	backup_write_ptr += len;
	memcpy( backup_write_ptr, BACKUP_TERM_STR, backup_term_len );

#ifdef BUCKUP_DEBUG
@@	char* tmpp = backup_over_ptr + BACKUP_NULLAREA_SIZE - 1;
@@	printk( KERN_INFO "[Backup] main: %p = %#x %#x %#x\n", tmpp,tmpp[0],tmpp[1],tmpp[2] );
#endif //BUCKUP_DEBUG

	return;

} 


void backup_log_init( void )
{
	int cmpr;
	char* strp = NULL;
	char* bufp = NULL;
	__kernel_size_t hlen;

	memset( backup_over_ptr, 0, BACKUP_NULLAREA_SIZE );

	backup_term_len = strlen( BACKUP_TERM_STR );

	hlen = strlen(BACKUP_HEAD_STR);
	cmpr = strncmp( backup_start_ptr, BACKUP_HEAD_STR, hlen );
	if( cmpr )	
	{
		memcpy( backup_start_ptr, BACKUP_HEAD_STR, hlen );
		backup_start_ptr += hlen;
		backup_write_ptr += hlen;
		memcpy( backup_write_ptr, BACKUP_TERM_STR, backup_term_len );
	}
	else
	{
		backup_start_ptr += hlen;
		backup_write_ptr += hlen;
		bufp = backup_start_ptr;
		strp = backup_over_ptr - backup_term_len;
		do {
			if( '<' == *bufp ) {
				cmpr = strncmp( bufp, BACKUP_TERM_STR, backup_term_len );
				if( !cmpr ) {
					backup_write_ptr = bufp;
					break;
				}
			}
			bufp ++;

		} while( strp >= bufp );
	}

#ifdef BUCKUP_DEBUG
@@	char* tmpp = backup_over_ptr + BACKUP_NULLAREA_SIZE - 1;
@@	*(tmpp)=0xCB; *(tmpp+1)=0xCC; *(tmpp+2)=0xCD;
@@	printk( KERN_INFO "[Backup] init: %p = %#x %#x %#x\n", tmpp,tmpp[0],tmpp[1],tmpp[2] );
#endif //BUCKUP_DEBUG

	return;

}


void backup_arm_write( void )
{
	unsigned char loop_cnt;
	unsigned char loop_max;
	char *read_ptr;
	
	loop_max = *g_arm_cnt;
	memcpy( g_arm_temp_buff, g_arm_read_addr, BACKUP_ARM_DAT_SIZE );
	read_ptr = g_arm_temp_buff;

	if( BACKUP_ARM_TRUE == *g_arm_flg_addr ){
		for( loop_cnt=1; loop_cnt <= loop_max; loop_cnt++ ){
			memset( g_ktrace_msg, 0x00, sizeof(g_ktrace_msg));
			strncpy( g_ktrace_msg, read_ptr, BACKUP_ARM_STR_LEN );
			printk( KERN_ERR "%s\n", g_ktrace_msg );
			read_ptr = read_ptr + BACKUP_ARM_STR_LEN + BACKUP_ARM_SEP_SIZE;
			if( read_ptr >= &g_arm_temp_buff[BACKUP_ARM_DAT_SIZE]){
				break;
			}
		}
		*g_arm_flg_addr = BACKUP_ARM_FALSE;
		*g_arm_cnt = 0;
	}
	return;
} 

