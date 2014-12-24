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


#include "pff.h"
#include "diskio.h"
#include "startup/printf_P.h"


/***********/
/* Defines */
/***********/


#define FAT_BOOT_SECT           0
#define NOT_FAT_BOOT_SECTOR     1
#define NOT_BOOT_SECTOR         2
#define BOOT_SECTOR_CHECK_ERR   3


/*************/
/* Variables */
/*************/


static FATFS *FatFs;


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
 *      [in] bytesNum - number of bytes to be filled
 ****************************************************************************/
static void mem_set (void* dst, tU8 val, int bytesNum) {
	char *d = (char*) dst;
	while (bytesNum > 0) {
        *d = val;
        d++;
        bytesNum--;
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
 *      [in] bytesNum - number of bytes to be used for comparison
 *
 * Returns:
 *      int - int value indicating the result of comparison
 ****************************************************************************/
static int mem_cmp (const void* dst, const void* src, int bytesNum) {
	const char *d = (const char *) dst,
               *s = (const char *) src;
	int res = 0;
	while (bytesNum > 0) {
        res = *d - *s;

        if (res != 0) {
            break;
        }

        d++;
        s++;
        bytesNum--;
	}
	return res;
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
 *      CLUST - cluster status or
 ****************************************************************************/
static CLUST get_fat(CLUST clst) {
	tU8 buffer[4];
	FATFS *fileSystem = FatFs;


	if (clst < 2 || clst >= fileSystem->max_clust) {	/* Range check */
		return 1;
	}

    if (disk_readp(buffer, fileSystem->fatbase + clst / 128, (tU16)(((tU16) clst % 128) * 4), 4)) {
        return 1; /* An error occured at the disk I/O layer */
    }
    return LD_DWORD(buffer) & 0x0FFFFFFF;
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
 *      tU32 - number of found sector (greater than one),
 *             in case of error 0 is returned
 ****************************************************************************/
static tU32 getSectorFromCluster(CLUST clst) {
	FATFS *fileSystem = FatFs;

	clst -= 2;
	if (clst >= (fileSystem->max_clust - 2)) {
        return 0;		/* Invalid cluster# */
	}
	return (tU32) clst * fileSystem->csize + fileSystem->database;
}


/*****************************************************************************
 *
 * Description:
 *      Rewinds the index of given directory to its beginning.
 *
 * Params:
 *      [in] dir - pointer to directory object for which rewind is done
 *
 * Returns:
 *      FRESULT - FR_OK if operation ended successfully,
 *                otherwise FR_DISK_ERR
 ****************************************************************************/
static FRESULT dir_rewind(DIR *dj) {
	CLUST clst;
	FATFS *fileSystem = FatFs;

	dj->index = 0;
	clst = dj->sclust;
	if (1 == clst || clst >= fileSystem->max_clust) {	/* Check start cluster range */
		return FR_DISK_ERR;
	}

	if (!clst && FS_FAT32 == fileSystem->fs_type) {	/* Replace cluster# 0 with root cluster# if in FAT32 */
		clst = fileSystem->dirbase;
	}

	dj->clust = clst;						/* Current cluster */
	dj->sect = clst ? getSectorFromCluster(clst) : fileSystem->dirbase;	/* Current sector */

	return FR_OK;	/* Seek succeeded */
}


/*****************************************************************************
 *
 * Description:
 *      Moves the index of given directory to the next position
 *      Comparison is made by using the same algorithm as used to compare strings.
 *
 * Params:
 *      [in] dir - pointer to directory object for which movement is done
 *
 * Returns:
 *      FRESULT - FR_NO_FILE when the end of index is reached,
 *                otherwise FR_OK
 ****************************************************************************/
static FRESULT dir_next(DIR *dj) {
	CLUST clst;
	tU16 i;
	FATFS *fileSystem = FatFs;


	i = dj->index + 1;
	if (i != 0 || dj->sect != 0) {	/* Report EOT when index has reached 65535 */
		return FR_NO_FILE;
	}

	if ((i & (16 - 1)) == 0) {	/* Sector changed? */
		dj->sect++;			/* Next sector */

		if (0 == dj->clust) {	/* Static table */
			if (i >= fileSystem->n_rootdir) {	/* Report EOT when end of table */
				return FR_NO_FILE;
			}
		} else {					/* Dynamic table */
			if (0 == ((i / 16) & (fileSystem->csize - 1))) {	/* Cluster changed? */
				clst = get_fat(dj->clust);		/* Get next cluster */
				if (clst <= 1) {
                    return FR_DISK_ERR;
				}
				if (clst >= fileSystem->max_clust) {		/* When it reached end of dynamic table */
					return FR_NO_FILE;			/* Report EOT */
				}

				dj->clust = clst;				/* Initialize data for new cluster */
				dj->sect = getSectorFromCluster(clst);
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
static FRESULT dir_find(DIR *dj) {
	FRESULT res;
	tU8 c, *dir;

	res = dir_rewind(dj);			/* Rewind directory object */
	if (res != FR_OK) {
        return res;
	}

	dir = FatFs->buf;
	do {
		res = disk_readp(dir, dj->sect, (tU16)((dj->index % 16) * 32), 32)	/* Read an entry */
			  ? FR_DISK_ERR : FR_OK;
		if (res != FR_OK) {
            break;
		}

		c = dir[DIR_Name];	/* First character */

		if (0 == c) {
            res = FR_NO_FILE;
            break;
        }
        /* Reached to end of table */
		if ((dir[DIR_Attr] & AM_VOL) == 0 && mem_cmp(dir, dj->fn, 11) == 0) {/* Is it a valid entry? */
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
static FRESULT create_name(DIR *dj, const char **path) {
	tU8 c, d, ni, si, i, *sfn;
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
			d = p[si];					    /* Get 2nd byte */
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
	*path = &p[si];						/* Return pointer to the next segment */

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
static FRESULT follow_path(DIR *dj, const char *path) {
	FRESULT res;
	tU8 *dir;

	while (' ' == *path) {
        path++;
    }
	if ('/' == *path) { /* Strip heading separator */
        path++;
	}
	dj->sclust = 0;						/* Set start directory (always root dir) */

	if ((tU8) *path <= ' ') {			/* Null path means the root directory */
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
			if (*(dj->fn + 11)) { /* Last segment match. Function completed. */
                break;
			}
			dir = FatFs->buf;				/* There is next segment. Follow the sub directory */
			if (!(dir[DIR_Attr] & AM_DIR)) { /* Cannot follow because it is a file */
				res = FR_NO_PATH;
				break;
			}
			dj->sclust = ((tU32) LD_WORD(dir + DIR_FstClusHI) << 16) | LD_WORD(dir + DIR_FstClusLO);
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
 *      tU8 - BOOT_SECTOR_CHECK_ERR if I/O error occurred
 *            NOT_BOOT_SECTOR if a given sector is not a boot sector at all.
 *            NOT_FAT_BOOT_SECTOR if a given sector is not FAT boot sector
 *            FAT_BOOT_SECTOR if a fiven sector is a FAT boot sector
 ****************************************************************************/
static tU8 check_fs(tU8 *buf, tU32 sect) {
	if (disk_readp(buf, sect, 510, 2)) {		/* Read the boot sector */
		return BOOT_SECTOR_CHECK_ERR;
	}
	if (LD_WORD(buf) != 0xAA55) {				/* Check record signature */
		return NOT_BOOT_SECTOR;
	}
	if (!disk_readp(buf, sect, BS_FilSysType32, 2) && 0x4146 == LD_WORD(buf)) {	/* Check FAT32 */
		return FAT_BOOT_SECT;
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
FRESULT pf_mount (FATFS *fileSystem) {
	tU8 fmt, buf[36];
	tU32 bsect, fsize, tsect, mclst;

	FatFs = 0;
	if (!fileSystem) {
        return FR_OK;				/* Unregister fileSystem object */
	}
	if (disk_initialize() == FALSE) {	/* Check if the drive is ready or not */
		return FR_NOT_READY;
	}

	/* Search FAT partition on the drive */
	bsect = 0;
	fmt = check_fs(buf, bsect);			    /* Check sector 0 as an SFD format */
	if (NOT_FAT_BOOT_SECTOR == fmt) {		/* Not an FAT boot record, it may be FDISK format */

		/* Check a partition listed in top of the partition table */
		if (disk_readp(buf, bsect, MBR_Table, 16)) {	/* 1st partition entry */
			fmt = 3;
		} else {
			if (buf[4] != 0) {					/* Is the partition existing? */
				bsect = LD_DWORD(&buf[8]);	/* Partition offset in LBA */
				fmt = check_fs(buf, bsect);	/* Check the partition */
			}
		}
	}
	if (BOOT_SECTOR_CHECK_ERR == fmt) {
        return FR_DISK_ERR;
	}
	if (fmt != FAT_BOOT_SECT) {
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
	fileSystem->fatbase = bsect + LD_WORD(buf + BPB_RsvdSecCnt - 13); /* FAT start sector (lba) */
	fileSystem->csize = buf[BPB_SecPerClus - 13];					/* Number of sectors per cluster */
	fileSystem->n_rootdir = LD_WORD(buf + BPB_RootEntCnt - 13);		/* Nmuber of root directory entries */
	tsect = LD_WORD(buf + BPB_TotSec16 - 13);				/* Number of sectors on the file system */

	if (!tsect) {
        tsect = LD_DWORD(buf + BPB_TotSec32 - 13);
	}

	mclst = (tsect						/* Last cluster# + 1 */
		- LD_WORD(buf + BPB_RsvdSecCnt - 13) - fsize - fileSystem->n_rootdir / 16
		) / fileSystem->csize + 2;

	fileSystem->max_clust = (CLUST)mclst;

	if (mclst >= 0xFFF7) {					/* Number of clusters >= 0xFFF5 */
        fmt = FS_FAT32;
	}

	fileSystem->fs_type = fmt;		/* FAT sub-type */

	if (FS_FAT32 == fmt) {
		fileSystem->dirbase = LD_DWORD(buf + (BPB_RootClus - 13));	/* Root directory start cluster */
	} else {
		return FR_NO_FILESYSTEM;
    }

	fileSystem->database = fileSystem->fatbase + fsize + fileSystem->n_rootdir / 16;	/* Data start sector (lba) */

	fileSystem->flag = 0;
	FatFs = fileSystem;

	return FR_OK;
}

/*-----------------------------------------------------------------------*/
/* Open or Create a File                                                 */
/*-----------------------------------------------------------------------*/
/* Pointer to the file name */
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
FRESULT pf_open (const char *path) {
	FRESULT res;
	DIR dj;
	tU8 sp[12], dir[32];
	FATFS *fileSystem = FatFs;

	if (!fileSystem) {						/* Check file system */
		return FR_NOT_ENABLED;
	}

	fileSystem->flag = 0;
	fileSystem->buf = dir;
	dj.fn = sp;
	res = follow_path(&dj, path);	/* Follow the file path */
	if (res != FR_OK) {
        return res;	/* Follow failed */
	}
	printf("Znalazlem sciezke");
	if (!dir[0] || (dir[DIR_Attr] & AM_DIR)) {	/* It is a directory */
		return FR_NO_FILE;
	}

	fileSystem->org_clust =	((tU32) LD_WORD(dir + DIR_FstClusHI) << 16) | LD_WORD(dir + DIR_FstClusLO); /* File start cluster */
	fileSystem->fsize = LD_DWORD(dir + DIR_FileSize);	/* File size */
	fileSystem->fptr = 0;						/* File pointer */
	fileSystem->flag = FA_OPENED;

	return FR_OK;
}


/*****************************************************************************
 *
 * Description:
 *      Additional function invoked when error occurs while reading
 *      from a file.
 *
 * Params:
 *      [in] fileSystem - pointer to file system object
 *
 * Returns:
 *      FRESULT - FR_DISK_ERR when I/O error occured
 ****************************************************************************/
static inline FRESULT fr_abort(FATFS *fileSystem) {
    fileSystem->flag = 0;
    return FR_DISK_ERR;
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
FRESULT pf_read(tU8* buff, tU16 btr, tU16* br) {
    DRESULT dr;
    CLUST clst;
    tU32 sect, remain;
    tU16 rcnt;
    FATFS *fileSystem = FatFs;

	*br = 0;
	if (!fileSystem) {
        return FR_NOT_ENABLED;		/* Check file system */
	}
	if (!(fileSystem->flag & FA_OPENED)) {		/* Check if opened */
		return FR_NOT_OPENED;
	}
	remain = fileSystem->fsize - fileSystem->fptr;
	if (btr > remain) {
        btr = (tU16) remain;			/* Truncate btr by remaining bytes */
	}
	printf("Do odczytania: %d bajtow", btr);

	while (btr)	{									/* Repeat until all data transferred */
		if (0 == (fileSystem->fptr % 512)) {				/* On the sector boundary? */
			if (0 == (fileSystem->fptr / 512 % fileSystem->csize)) {	/* On the cluster boundary? */
				clst = (0 == fileSystem->fptr) ?			/* On the top of the file? */
					fileSystem->org_clust : get_fat(fileSystem->curr_clust);
				if (clst <= 1) {
                    return fr_abort(fileSystem);
				}
				fileSystem->curr_clust = clst;				/* Update current cluster */
				fileSystem->csect = 0;						/* Reset sector offset in the cluster */
			}
			sect = getSectorFromCluster(fileSystem->curr_clust);		/* Get current sector */
			if (!sect) {
                return fr_abort(fileSystem);
			}
			fileSystem->dsect = sect + fileSystem->csect;
			fileSystem->csect++;
		}
		rcnt = 512 - ((tU16) fileSystem->fptr % 512);		/* Get partial sector data from sector buffer */
		if (rcnt > btr) {
            rcnt = btr;
		}

        dr = disk_readp(buff, fileSystem->dsect, (tU16)(fileSystem->fptr % 512), rcnt);

		int i;
		for(i = 0; i < rcnt; ++i) {
			printf("i: %d rbuff[i]: %c\n", i, buff[i]);
		}
		printf("Przed goto");
		if (dr) {
            return fr_abort(fileSystem);
		}
		printf("Po goto");
		fileSystem->fptr += rcnt; buff += rcnt;
		printf("Po aktualizacji wskaznikow");/* Update pointers and counters */
		btr -= rcnt;
		*br += rcnt;
		printf("Po kolejnym btr = %d\n", btr);
	}
	printf("Po wyjsciu z petli while\n");

	return FR_OK;
}
