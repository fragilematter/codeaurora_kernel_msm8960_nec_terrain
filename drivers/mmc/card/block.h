#ifdef CONFIG_FEATURE_NCMC_SDCARD

#ifndef MMC_BLOCK_H
#define MMC_BLOCK_H

extern void mmc_err_info_get(int *err_info);
extern void mmc_err_info_set(void );
extern void mmc_err_info_clear(void);
extern int  mmc_blk_alloc_buf(void);
extern void mmc_blk_free_buf(void);

#endif

#endif /* CONFIG_FEATURE_NCMC_SDCARD */
