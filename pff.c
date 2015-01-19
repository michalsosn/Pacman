/******************************************************************************
 *
 * Copyright:
 *    Copyright (C) 2009, ChaN, all right reserved.
 *
 * Annotation:
 *    This library has been adjusted to the needs of 'Pacman Project'.
 *
 * File:
 *    pff.c
 *
 * Description:
 *    Contains implementation of FAT handling routines.
 *
 *****************************************************************************/


 /************/
/* Includes */
/************/


#include "pff.h"		/* Petit FatFs configurations and declarations */
#include "diskio.h"		/* Declarations of low level disk I/O functions */
#include "startup/printf_P.h"


/***********/
/* Defines */
/***********/

#define FAT_BOOT_SECTOR         0
#define NOT_FAT_BOOT_SECTOR     1
#define BOOT_SECTOR_CHECK_ERR   3
#define NOT_BOOT_SECTOR         2

/*************/
/* Variables */
/*************/


static FATFS *FatFs;	/* Pointer to the file system object (logical drive) */

DRESULT dr;
CLUST clst;
DWORD sect, remain;
BYTE *rbuff;
WORD rcnt;
FATFS *fs;


/*************/
/* Functions */
/*************/


/*****************************************************************************
 *
 * Description:
 *      Fills memory area with specified value starting from given adress.
 *
 * Params:
 *      [in] dst - starting adress from which memory is to be filled
 *      [in] val - value to assign
 *      [in] cnt - number of bytes to be filled
 ****************************************************************************/
static void mem_set (void* dst, int val, int cnt) {
	char *d = (char*) dst;
	while (cnt > 0) {
        *d = (char) val;
        d++;
        cnt--;
	}
}


/*****************************************************************************
 *
 * Description:
 *      Compares two areas of memory with each other.
 *      Comparison is made by using the same algorithm as used to compare strings.
 *
 * Params:
 *      [in] dst - pointer to the beginning of first area
 *      [in] src - pointer to the beginning of second area
 *      [in] cnt - number of bytes to be used for comparison
 *
 * Returns:
 *      int - int value indicating the result of comparison
 ****************************************************************************/
static int mem_cmp (const void* dst, const void* src, int cnt) {
	const char *d = (const char *)dst,
               *s = (const char *)src;
	int r = 0;
	while (cnt > 0) {
        r = *d - *s;
        if (r != 0) {
            break;
        }
        d++;
        s++;
        cnt--;
	}
	return r;
}



/*****************************************************************************
 *
 * Description:
 *      Reads a value of entry in FAT table for given cluster number.
 *
 * Params:
 *      [in] clst - number of cluster for which value is read
 *
 * Returns:
 *      CLUST - cluster status or value indicating I/O error
 ****************************************************************************/
static CLUST get_fat(CLUST clst) {
	WORD wc, bc, ofs;
	BYTE buf[4];
	FATFS *fs = FatFs;

	if (clst < 2 || clst >= fs->max_clust)	/* Range check */
		return 1;

	switch (fs->fs_type) {
        case FS_FAT12 :
            bc = (WORD)clst;
            bc += bc / 2;
            ofs = bc % SECTOR_SIZE;
            bc /= SECTOR_SIZE;
            if (ofs != 511) {
                if (disk_readp(buf, fs->fatbase + bc, ofs, 2)) {
                    break;
                }
            } else {
                if (disk_readp(buf, fs->fatbase + bc, 511, 1)) {
                    break;
                }
                if (disk_readp(buf+1, fs->fatbase + bc + 1, 0, 1)) {
                    break;
                }
            }
            wc = LD_WORD(buf);
            return (clst & 1) ? (wc >> 4) : (wc & 0xFFF);

        case FS_FAT16 :
            if (disk_readp(buf, fs->fatbase + clst / 256, (WORD)(((WORD)clst % 256) * 2), 2)) {
                break;
            }
            return LD_WORD(buf);
        case FS_FAT32 :
            if (disk_readp(buf, fs->fatbase + clst / 128, (WORD)(((WORD)clst % 128) * 4), 4)) {
                break;
            }
            return LD_DWORD(buf) & 0x0FFFFFFF;
	}

	return 1;	/* An error occured at the disk I/O layer */
}


/*****************************************************************************
 *
 * Description:
 *      Gets number of first data sector in cluster of given number
 *
 * Params:
 *      [in] clst - number of cluster
 *
 * Returns:
 *      DWORD - number of found sector (greater than one),
 *              in case of error 0 is returned
 ****************************************************************************/
static DWORD clust2sect (CLUST clst) {
	FATFS *fs = FatFs;

	clst -= 2;
	if (clst >= (fs->max_clust - 2)) {
        return 0;		/* Invalid cluster# */
	}

	return (DWORD) clst * fs->csize + fs->database;
}


/*****************************************************************************
 *
 * Description:
 *      Rewinds the index of given directory to its beginning.
 *
 * Params:
 *      [in] dj - pointer to directory object for which rewind is done
 *
 * Returns:
 *      FRESULT - FR_OK if operation ended successfully,
 *                otherwise FR_DISK_ERR
 ****************************************************************************/
static FRESULT dir_rewind(DIR *dj) {
	CLUST clst;
	FATFS *fs = FatFs;

	dj->index = 0;
	clst = dj->sclust;
	if (1 == clst || clst >= fs->max_clust) {	/* Check start cluster range */
		return FR_DISK_ERR;
	}
	if (!clst && FS_FAT32 == fs->fs_type) {	/* Replace cluster# 0 with root cluster# if in FAT32 */
		clst = fs->dirbase;
	}
	dj->clust = clst;						/* Current cluster */
	dj->sect = clst ? clust2sect(clst) : fs->dirbase;	/* Current sector */

	return FR_OK;	/* Seek succeeded */
}


/*****************************************************************************
 *
 * Description:
 *      Moves the index of given directory to the next position
 *      Comparison is made by using the same algorithm as used to compare strings.
 *
 * Params:
 *      [in] dj - pointer to directory object for which movement is done
 *
 * Returns:
 *      FRESULT - FR_NO_FILE when the end of index is reached,
 *                otherwise FR_OK. FR_DISK_ERR is returned in case of I/O error.
 ****************************************************************************/
static FRESULT dir_next(DIR *dj) {
	CLUST clst;
	WORD i;
	FATFS *fs = FatFs;

	i = dj->index + 1;
	if (!i || !dj->sect) {	/* Report EOT when index has reached 65535 */
		return FR_NO_FILE;
	}

	if (!(i & (16 - 1))) {	/* Sector changed? */
		dj->sect++;			/* Next sector */

		if (0 == dj->clust) {	/* Static table */
			if (i >= fs->n_rootdir) {	/* Report EOT when end of table */
				return FR_NO_FILE;
			}
		} else {					/* Dynamic table */
			if (((i / 16) & (fs->csize - 1)) == 0) {	/* Cluster changed? */
				clst = get_fat(dj->clust);		/* Get next cluster */
				if (clst <= 1) {
                    return FR_DISK_ERR;
				}
				if (clst >= fs->max_clust) {		/* When it reached end of dynamic table */
					return FR_NO_FILE;		        /* Report EOT */
                }
				dj->clust = clst;				/* Initialize data for new cluster */
				dj->sect = clust2sect(clst);
			}
		}
	}

	dj->index = i;

	return FR_OK;
}


/*****************************************************************************
 *
 * Description:
 *      Check whether the object represented by the given
 *      directory object actually exists in filesystem.
 *
 * Params:
 *      [in] dir - pointer to directory object for which check is done
 *
 * Returns:
 *      FRESULT - FR_NO_FILE if object in question doesn't exist,
 *                FR_OK if it was found in filesystem
 *                FR_DISK_ERR when I/O error occured
 ****************************************************************************/
static FRESULT dir_find (DIR *dj) {
	FRESULT res;
	BYTE c, *dir;

	res = dir_rewind(dj);			/* Rewind directory object */
	if (res != FR_OK) {
        return res;
	}

	dir = FatFs->buf;
	do {
		res = disk_readp(dir, dj->sect, (WORD)((dj->index % 16) * 32), 32)	/* Read an entry */
			? FR_DISK_ERR : FR_OK;
		if (res != FR_OK) {
            break;
		}
		c = dir[DIR_Name];	/* First character */
		if (0 == c) {
            res = FR_NO_FILE;   /* Reached to end of table */
            break;
        }
		if (!(dir[DIR_Attr] & AM_VOL) && !mem_cmp(dir, dj->fn, 11)) { /* Is it a valid entry? */
			break;
		}
		res = dir_next(dj);							/* Next entry */
	} while (FR_OK == res);

	return res;
}


/*****************************************************************************
 *
 * Description:
 *      Picks a segment of given path and creates directory object for it.
 *
 * Params:
 *      [out] dir - pointer to the directory object
 *      [in] path - Pointer to pointer to the segment in the path string
 *
 * Returns:
 *      FRESULT - FR_OK
 ****************************************************************************/
static FRESULT create_name (DIR *dj, const char **path) {
	BYTE c, d, ni, si, i, *sfn;
	const char *p;

	/* Create file name in directory form */
	sfn = dj->fn;
	mem_set(sfn, ' ', 11);
	i = 0;
	si = i;
	ni = 8;
	p = *path;
	for (;;) {
		c = p[si];
		si++;
		if (c <= ' ' || '/' == c) {
            break;	/* Break on end of segment */
		}
		if ('.' == c || i >= ni) {
			if (ni != 8 || c != '.') {
                break;
			}
			i = 8;
			ni = 11;
			continue;
		}

		if (IsDBCS1(c) && i >= ni - 1) {	/* DBC 1st byte? */
			d = p[si];                  /* Get 2nd byte */
			si++;
			sfn[i] = c;
			i++;
			sfn[i] = d;
			i++;
		} else {						/* Single byte code */
			if (IsLower(c)) {
                c -= 0x20;	/* toupper */
			}
			sfn[i] = c;
			i++;
		}
	}
	*path = &p[si];						/* Rerurn pointer to the next segment */

	sfn[11] = (c <= ' ') ? 1 : 0;		/* Set last segment flag if end of path */

	return FR_OK;
}


/*****************************************************************************
 *
 * Description:
 *      Follows a file path.
 *
 * Params:
 *      [out] dir - pointer directory object to return last directory
 *                  and found object
 *      [in] path - full-path string to find a file or directory
 *
 * Returns:
 *      FRESULT - FR_OK if operation succeeded,
 *                FR_NO_PATH if given path is not correct,
 *                in case of errors other error codes from FRESULT
 ****************************************************************************/
static FRESULT follow_path(DIR *dj,	const char *path) {
	FRESULT res;
	BYTE *dir;

	while (' ' == *path) {
        path++;		/* Skip leading spaces */
	}
	if ('/' == *path) {
        path++;			/* Strip heading separator */
	}
	dj->sclust = 0;						/* Set start directory (always root dir) */

	if ((BYTE)*path <= ' ') {			/* Null path means the root directory */
		res = dir_rewind(dj);
		FatFs->buf[0] = 0;
	} else {							/* Follow path */
		for (;;) {
			res = create_name(dj, &path);	/* Get a segment */
			if (res != FR_OK) {
                break;
			}
			res = dir_find(dj);				/* Find it */
			if (res != FR_OK) {				/* Could not find the object */
				if (FR_NO_FILE == res && !*(dj->fn + 11)) {
					res = FR_NO_PATH;
				}
				break;
			}
			if (*(dj->fn + 11)) {
                break;		/* Last segment match. Function completed. */
			}
			dir = FatFs->buf;				/* There is next segment. Follow the sub directory */
			if (!(dir[DIR_Attr] & AM_DIR)) { /* Cannot follow because it is a file */
				res = FR_NO_PATH;
				break;
			}
			dj->sclust = ((DWORD) LD_WORD(dir + DIR_FstClusHI) << 16) | LD_WORD(dir + DIR_FstClusLO);
		}
	}

	return res;
}


/*****************************************************************************
 *
 * Description:
 *      Check if a given sector is a FAT boot record.
 *
 * Params:
 *      [in] buf - working buffer for read sectors
 *      [in] sect - number of sector to check
 *
 * Returns:
 *      BYTE - BOOT_SECTOR_CHECK_ERR if I/O error occurred
 *             NOT_BOOT_SECTOR if a given sector is not a boot sector at all.
 *             NOT_FAT_BOOT_SECTOR if a given sector is not FAT boot sector
 *             FAT_BOOT_SECTOR if a given sector is a FAT boot sector
 ****************************************************************************/
static BYTE check_fs (BYTE *buf, DWORD sect) {
	if (disk_readp(buf, sect, 510, 2)) {		/* Read the boot sector */
		return BOOT_SECTOR_CHECK_ERR;
	}
	if (LD_WORD(buf) != 0xAA55) {				/* Check record signature */
		return NOT_BOOT_SECTOR;
	}

	if (!disk_readp(buf, sect, BS_FilSysType, 2) && LD_WORD(buf) == 0x4146) {	/* Check FAT12/16 */
		return FAT_BOOT_SECTOR;
	}

	if (!disk_readp(buf, sect, BS_FilSysType32, 2) && LD_WORD(buf) == 0x4146) {	/* Check FAT32 */
		return FAT_BOOT_SECTOR;
	}

	return NOT_FAT_BOOT_SECTOR;
}


/*****************************************************************************
 *
 * Description:
 *      Mounts/Unmounts a file system.
 *
 * Params:
 *      [in] fileSystem - pointer to the new file system object.
 *                        If it is NULL, functions unmounts current file system
 *
 * Returns:
 *      FRESULT - FR_NOT_READY if drive is not ready
 *                FR_OK if operation ended successfully
 *                FR_DISK_ERR when I/O error occured
 *                FR_NO_FILESYSTEM if there is no filesystem on the drive
 ****************************************************************************/
FRESULT pf_mount (FATFS *fs) {
	BYTE fmt, buf[36];
	DWORD bsect, fsize, tsect, mclst;


	FatFs = 0;
	if (!fs) {
        return FR_OK;				/* Unregister fs object */
	}

	if (disk_initialize() & STA_NOINIT)	{   /* Check if the drive is ready or not */
		return FR_NOT_READY;
	}

	/* Search FAT partition on the drive */
	bsect = 0;
	fmt = check_fs(buf, bsect);			/* Check sector 0 as an SFD format */
	if (NOT_FAT_BOOT_SECTOR == fmt) {		/* Not an FAT boot record, it may be FDISK format */
		/* Check a partition listed in top of the partition table */
		if (disk_readp(buf, bsect, MBR_Table, 16)) {	/* 1st partition entry */
			fmt = BOOT_SECTOR_CHECK_ERR;
		} else {
			if (buf[4]) {					/* Is the partition existing? */
				bsect = LD_DWORD(&buf[8]);	/* Partition offset in LBA */
				fmt = check_fs(buf, bsect);	/* Check the partition */
			}
		}
	}
	if (BOOT_SECTOR_CHECK_ERR == fmt) {
        return FR_DISK_ERR;
	}
	if (fmt) {
        return FR_NO_FILESYSTEM;	/* No valid FAT patition is found */
	}

	/* Initialize the file system object */
	if (disk_readp(buf, bsect, 13, sizeof(buf))) {
        return FR_DISK_ERR;
	}

	fsize = LD_WORD(buf + BPB_FATSz16 - 13);				/* Number of sectors per FAT */
	if (!fsize) {
        fsize = LD_DWORD(buf + BPB_FATSz32 - 13);
	}

	fsize *= buf[BPB_NumFATs - 13];						/* Number of sectors in FAT area */
	fs->fatbase = bsect + LD_WORD(buf + BPB_RsvdSecCnt - 13); /* FAT start sector (lba) */
	fs->csize = buf[BPB_SecPerClus - 13];					/* Number of sectors per cluster */
	fs->n_rootdir = LD_WORD(buf + BPB_RootEntCnt - 13);		/* Nmuber of root directory entries */
	tsect = LD_WORD(buf + BPB_TotSec16 - 13);				/* Number of sectors on the file system */

	if (!tsect) {
        tsect = LD_DWORD(buf + BPB_TotSec32 - 13);
	}

	mclst = (tsect						/* Last cluster# + 1 */
		- LD_WORD(buf+BPB_RsvdSecCnt-13) - fsize - fs->n_rootdir / 16) / fs->csize + 2;
	fs->max_clust = (CLUST) mclst;

	fmt = FS_FAT12;							/* Determine the FAT sub type */
	if (mclst >= 0xFF7) {
        fmt = FS_FAT16;		/* Number of clusters >= 0xFF5 */
	}
	if (mclst >= 0xFFF7) {					/* Number of clusters >= 0xFFF5 */
		fmt = FS_FAT32;
	}

	fs->fs_type = fmt;		/* FAT sub-type */

	if (FS_FAT32 == fmt) {
		fs->dirbase = LD_DWORD(buf + (BPB_RootClus - 13));	/* Root directory start cluster */
	} else {
		fs->dirbase = fs->fatbase + fsize;				/* Root directory start sector (lba) */
	}
	fs->database = fs->fatbase + fsize + fs->n_rootdir / 16;	/* Data start sector (lba) */

	fs->flag = 0;
	FatFs = fs;

	return FR_OK;
}


/*****************************************************************************
 *
 * Description:
 *      Opens or creates file with a given name.
 *      If file doesn't exist, it is created.
 *
 * Params:
 *      [in] path - the full name of file (including path)
 *
 * Returns:
 *      FRESULT - FR_NO_FILE if given path is a directory
 *                FR_OK if operation succeeded
 *                FR_NOT_ENABLED if file system is not ready
 *                               (f.e. has been not mounted yet)
 ****************************************************************************/
FRESULT pf_open(const char *path) {
	FRESULT res;
	DIR dj;
	BYTE sp[12], dir[32];
	FATFS *fs = FatFs;

	if (!fs) {						/* Check file system */
		return FR_NOT_ENABLED;
	}

	fs->flag = 0;
	fs->buf = dir;
	dj.fn = sp;
	res = follow_path(&dj, path);	/* Follow the file path */
	if (res != FR_OK) {
        return res;	/* Follow failed */
	}

	if (!dir[0] || (dir[DIR_Attr] & AM_DIR)) {	/* It is a directory */
		return FR_NO_FILE;
	}

    /* File start cluster */
	fs->org_clust =	((DWORD)LD_WORD(dir + DIR_FstClusHI) << 16) | LD_WORD(dir + DIR_FstClusLO);

	fs->fsize = LD_DWORD(dir + DIR_FileSize);	/* File size */
	fs->fptr = 0;						/* File pointer */
	fs->flag = FA_OPENED;

	return FR_OK;
}


/*****************************************************************************
 *
 * Description:
 *      Reads data from a file that was previously opened.
 *
 * Params:
 *      [out] buff - pointer to read buffer
 *      [in] btr - number of bytes to read
 *      [out] br - pointer to variable storing number of read bytes
 *
 * Returns:
 *      FRESULT - FR_NOT_ENABLED if file system is not ready
 *                               (f.e. has been not mounted yet)
 *                FR_NOT_OPENED  if there is no opened file
 *                FR_OK if operation ended with success
 *                FR_DISK_ERR when I/O error occured
 ****************************************************************************/
FRESULT pf_read (void* buff, WORD btr, WORD* br) {
	rbuff = buff;
	fs = FatFs;

	*br = 0;
	if (!fs) {
        return FR_NOT_ENABLED;		/* Check file system */
	}
	if (!(fs->flag & FA_OPENED)) {		/* Check if opened */
		return FR_NOT_OPENED;
	}

	remain = fs->fsize - fs->fptr;
	if (btr > remain) {
        btr = (WORD)remain;			/* Truncate btr by remaining bytes */
	}


	while (btr)	{									/* Repeat until all data transferred */
		if ((fs->fptr % SECTOR_SIZE) == 0) {				/* On the sector boundary? */
			if ((fs->fptr / SECTOR_SIZE % fs->csize) == 0) {	/* On the cluster boundary? */
                /* On the top of the file? */
				clst = (0 == fs->fptr) ? fs->org_clust : get_fat(fs->curr_clust);

				if (clst <= 1) {
                    goto fr_abort;
				}
				fs->curr_clust = clst;				/* Update current cluster */
				fs->csect = 0;						/* Reset sector offset in the cluster */
			}
			sect = clust2sect(fs->curr_clust);		/* Get current sector */
			if (!sect) {
                goto fr_abort;
			}
			fs->dsect = sect + fs->csect;
			fs->csect++;
		}
		rcnt = SECTOR_SIZE - ((WORD)fs->fptr % SECTOR_SIZE);		/* Get partial sector data from sector buffer */
		if (rcnt > btr) {
            rcnt = btr;
		}
		dr = disk_readp(!buff ? 0 : rbuff, fs->dsect, (WORD)(fs->fptr % SECTOR_SIZE), rcnt);
		if (dr) {
            goto fr_abort;
		}
		fs->fptr += rcnt;
		rbuff += rcnt;/* Update pointers and counters */
		btr -= rcnt;
		*br += rcnt;
	}

	return FR_OK;

fr_abort:
	fs->flag = 0;
	return FR_DISK_ERR;
}

