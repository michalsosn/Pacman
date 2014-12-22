/*----------------------------------------------------------------------------/
/  Petit FatFs - FAT file system module  R0.02                 (C)ChaN, 2009
/-----------------------------------------------------------------------------/
/ Petit FatFs module is an open source software to implement FAT file system to
/ small embedded systems. This is a free software and is opened for education,
/ research and commercial developments under license policy of following trems.
/
/  Copyright (C) 2009, ChaN, all right reserved.
/
/ * The Petit FatFs module is a free software and there is NO WARRANTY.
/ * No restriction on use. You can use, modify and redistribute it for
/   personal, non-profit or commercial use UNDER YOUR RESPONSIBILITY.
/ * Redistributions of source code must retain the above copyright notice.
/
/-----------------------------------------------------------------------------/
/ Jun 15,'09  R0.01a  First release. (Branched from FatFs R0.07b.)
/
/ Dec 14,'09  R0.02   Added multiple code page support.
/                     Added write funciton.
/                     Changed stream read mode interface.
/----------------------------------------------------------------------------*/

#include "pff.h"		/* Petit FatFs configurations and declarations */
#include "diskio.h"		/* Declarations of low level disk I/O functions */
#include "startup/printf_P.h"


/*--------------------------------------------------------------------------

   Private Functions

---------------------------------------------------------------------------*/

static FATFS *FatFs;	/* Pointer to the file system object (logical drive) */

/*-----------------------------------------------------------------------*/
/* String functions                                                      */
/*-----------------------------------------------------------------------*/

/* Fill memory */
static void mem_set (void* dst, int val, int bytesNum) {
	char *d = (char*) dst;
	while (bytesNum > 0) {
        *d = (char) val;
        d++;
        bytesNum--;
	}
}

/* Compare memory to memory */
static int mem_cmp (const void* dst, const void* src, int bytesNum) {
	const char *d = (const char *) dst, *s = (const char *)src;
	int r = 0;
	while (bytesNum > 0) {
        r = *d - *s;

        if (r != 0) {
            break;
        }

        d++;
        s++;
        bytesNum--;
	}
	return r;
}

/*-----------------------------------------------------------------------*/
/* FAT access - Read value of a FAT entry                                */
/*-----------------------------------------------------------------------*/
/* 1:IO error, Else:Cluster status */
/* Cluster# to get the link information */
static CLUST get_fat(CLUST clst) {
	tU16 wc, bc, ofs;
	tU8 buf[4];
	FATFS *fs = FatFs;


	if (clst < 2 || clst >= fs->max_clust) {	/* Range check */
		return 1;
	}

    if (disk_readp(buf, fs->fatbase + clst / 128, (tU16)(((tU16)clst % 128) * 4), 4)) {
        return 1; /* An error occured at the disk I/O layer */
    }
    return LD_DWORD(buf) & 0x0FFFFFFF;
}

/*-----------------------------------------------------------------------*/
/* Get sector# from cluster#                                             */
/*-----------------------------------------------------------------------*/
/* !=0: Sector number, 0: Failed - invalid cluster# */
/* Cluster# to be converted */
static tU32 clust2sect(CLUST clst) {
	FATFS *fs = FatFs;

	clst -= 2;
	if (clst >= (fs->max_clust - 2)) return 0;		/* Invalid cluster# */
	return (tU32)clst * fs->csize + fs->database;
}

/*-----------------------------------------------------------------------*/
/* Directory handling - Rewind directory index                           */
/*-----------------------------------------------------------------------*/
/* Pointer to directory object */
static FRESULT dir_rewind(DIR *dj) {
	CLUST clst;
	FATFS *fs = FatFs;

	dj->index = 0;
	clst = dj->sclust;
	if (clst == 1 || clst >= fs->max_clust)	/* Check start cluster range */
		return FR_DISK_ERR;

	if (!clst && fs->fs_type == FS_FAT32)	/* Replace cluster# 0 with root cluster# if in FAT32 */
		clst = fs->dirbase;

	dj->clust = clst;						/* Current cluster */
	dj->sect = clst ? clust2sect(clst) : fs->dirbase;	/* Current sector */

	return FR_OK;	/* Seek succeeded */
}

/*-----------------------------------------------------------------------*/
/* Directory handling - Move directory index next                        */
/*-----------------------------------------------------------------------*/

/* FR_OK:Succeeded, FR_NO_FILE:End of table */
/* Pointer to directory object */
static FRESULT dir_next(DIR *dj) {
	CLUST clst;
	tU16 i;
	FATFS *fs = FatFs;


	i = dj->index + 1;
	if (!i || !dj->sect)	/* Report EOT when index has reached 65535 */
		return FR_NO_FILE;

	if (!(i & (16-1))) {	/* Sector changed? */
		dj->sect++;			/* Next sector */

		if (dj->clust == 0) {	/* Static table */
			if (i >= fs->n_rootdir)	/* Report EOT when end of table */
				return FR_NO_FILE;
		}
		else {					/* Dynamic table */
			if (((i / 16) & (fs->csize-1)) == 0) {	/* Cluster changed? */
				clst = get_fat(dj->clust);		/* Get next cluster */
				if (clst <= 1) return FR_DISK_ERR;
				if (clst >= fs->max_clust)		/* When it reached end of dynamic table */
					return FR_NO_FILE;			/* Report EOT */
				dj->clust = clst;				/* Initialize data for new cluster */
				dj->sect = clust2sect(clst);
			}
		}
	}

	dj->index = i;

	return FR_OK;
}

/*-----------------------------------------------------------------------*/
/* Directory handling - Find an object in the directory                  */
/*-----------------------------------------------------------------------*/
/* Pointer to the directory object linked to the file name */
static FRESULT dir_find(DIR *dj) {
	FRESULT res;
	tU8 c, *dir;

	res = dir_rewind(dj);			/* Rewind directory object */
	if (res != FR_OK) return res;

	dir = FatFs->buf;
	do {
		res = disk_readp(dir, dj->sect, (tU16)((dj->index % 16) * 32), 32)	/* Read an entry */
			? FR_DISK_ERR : FR_OK;
		if (res != FR_OK) {
            break;
		}

		c = dir[DIR_Name];	/* First character */

		if (0 == c) {
            res = FR_NO_FILE; break;
        }
        /* Reached to end of table */
		if (!(dir[DIR_Attr] & AM_VOL) && !mem_cmp(dir, dj->fn, 11)) {/* Is it a valid entry? */
			break;
        }
		res = dir_next(dj);							/* Next entry */
	} while (FR_OK == res);

	return res;
}

/*-----------------------------------------------------------------------*/
/* Pick a segment and create the object name in directory form           */
/*-----------------------------------------------------------------------*/

/* Pointer to the directory object */
/* Pointer to pointer to the segment in the path string */
static FRESULT create_name(DIR *dj, const char **path) {
	tU8 c, d, ni, si, i, *sfn;
	const char *p;

	/* Create file name in directory form */
	sfn = dj->fn;
	mem_set(sfn, ' ', 11);
	si = i = 0; ni = 8;
	p = *path;
	for (;;) {
		c = p[si++];
		if (c <= ' ' || c == '/') break;	/* Break on end of segment */
		if (c == '.' || i >= ni) {
			if (ni != 8 || c != '.') break;
			i = 8; ni = 11;
			continue;
		}

		if (IsDBCS1(c) && i >= ni - 1) {	/* DBC 1st byte? */
			d = p[si++];					/* Get 2nd byte */
			sfn[i++] = c;
			sfn[i++] = d;
		} else {						/* Single byte code */
			if (IsLower(c)) c -= 0x20;	/* toupper */
			sfn[i++] = c;
		}
	}
	*path = &p[si];						/* Rerurn pointer to the next segment */

	sfn[11] = (c <= ' ') ? 1 : 0;		/* Set last segment flag if end of path */

	return FR_OK;
}

/*-----------------------------------------------------------------------*/
/* Follow a file path                                                    */
/*-----------------------------------------------------------------------*/
/* FR_OK(0): successful, !=0: error code */
/* Directory object to return last directory and found object */
/* Full-path string to find a file or directory */
static
FRESULT follow_path(DIR *dj, const char *path) {
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
				if (res == FR_NO_FILE && !*(dj->fn+11)) {
					res = FR_NO_PATH;
				}
				break;
			}
			if (*(dj->fn+11)) { /* Last segment match. Function completed. */
                break;
			}
			dir = FatFs->buf;				/* There is next segment. Follow the sub directory */
			if (!(dir[DIR_Attr] & AM_DIR)) { /* Cannot follow because it is a file */
				res = FR_NO_PATH; break;
			}
			dj->sclust = ((tU32)LD_WORD(dir+DIR_FstClusHI) << 16) | LD_WORD(dir+DIR_FstClusLO);
		}
	}

	return res;
}

/*-----------------------------------------------------------------------*/
/* Check a sector if it is an FAT boot record                            */
/*-----------------------------------------------------------------------*/
/* 0:The FAT boot record, 1:Valid boot record but not an FAT, 2:Not a boot record, 3:Error */
/* Working buffer */
/* Sector# (lba) to check if it is an FAT boot record or not */
static tU8 check_fs(tU8 *buf,	tU32 sect) {
	if (disk_readp(buf, sect, 510, 2)) {		/* Read the boot sector */
		return 3;
	}
	if (LD_WORD(buf) != 0xAA55) {				/* Check record signature */
		return 2;
	}
	if (!disk_readp(buf, sect, BS_FilSysType32, 2) && LD_WORD(buf) == 0x4146) {	/* Check FAT32 */
		return 0;
	}

	return 1;
}

/*--------------------------------------------------------------------------

   Public Functions

--------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/* Mount/Unmount a Locical Drive                                         */
/*-----------------------------------------------------------------------*/
/* Pointer to new file system object (NULL: Unmount) */
FRESULT pf_mount (FATFS *fs) {
	tU8 fmt, buf[36];
	tU32 bsect, fsize, tsect, mclst;

	FatFs = 0;
	if (!fs) {
        return FR_OK;				/* Unregister fs object */
	}
	if (disk_initialize() & STA_NOINIT) {	/* Check if the drive is ready or not */
		return FR_NOT_READY;
	}

	/* Search FAT partition on the drive */
	bsect = 0;
	fmt = check_fs(buf, bsect);			/* Check sector 0 as an SFD format */
	if (1 == fmt) {						/* Not an FAT boot record, it may be FDISK format */
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
	if (3 == fmt) {
        return FR_DISK_ERR;
	}
	if (fmt) {
        return FR_NO_FILESYSTEM;	/* No valid FAT patition is found */
	}

	/* Initialize the file system object */
	if (disk_readp(buf, bsect, 13, sizeof(buf))) {
        return FR_DISK_ERR;
	}

	fsize = LD_WORD(buf+BPB_FATSz16-13);				/* Number of sectors per FAT */
	if (!fsize) {
        fsize = LD_DWORD(buf+BPB_FATSz32-13);
	}

	fsize *= buf[BPB_NumFATs-13];						/* Number of sectors in FAT area */
	fs->fatbase = bsect + LD_WORD(buf+BPB_RsvdSecCnt-13); /* FAT start sector (lba) */
	fs->csize = buf[BPB_SecPerClus-13];					/* Number of sectors per cluster */
	fs->n_rootdir = LD_WORD(buf+BPB_RootEntCnt-13);		/* Nmuber of root directory entries */
	tsect = LD_WORD(buf+BPB_TotSec16-13);				/* Number of sectors on the file system */
	if (!tsect) {
        tsect = LD_DWORD(buf+BPB_TotSec32-13);
	}

	mclst = (tsect						/* Last cluster# + 1 */
		- LD_WORD(buf+BPB_RsvdSecCnt-13) - fsize - fs->n_rootdir / 16
		) / fs->csize + 2;

	fs->max_clust = (CLUST)mclst;

	fmt = FS_FAT12;							/* Determine the FAT sub type */
	if (mclst >= 0xFFF7) {					/* Number of clusters >= 0xFFF5 */
        fmt = FS_FAT32;
	}

	fs->fs_type = fmt;		/* FAT sub-type */

	if (FS_FAT32 == fmt) {
		fs->dirbase = LD_DWORD(buf+(BPB_RootClus-13));	/* Root directory start cluster */
	} else {
		fs->dirbase = fs->fatbase + fsize;
    }				/* Root directory start sector (lba) */
	fs->database = fs->fatbase + fsize + fs->n_rootdir / 16;	/* Data start sector (lba) */

	fs->flag = 0;
	FatFs = fs;

	return FR_OK;
}

/*-----------------------------------------------------------------------*/
/* Open or Create a File                                                 */
/*-----------------------------------------------------------------------*/
/* Pointer to the file name */
FRESULT pf_open (const char *path) {
	FRESULT res;
	DIR dj;
	tU8 sp[12], dir[32];
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
	printf("Znalazlem sciezke");
	if (!dir[0] || (dir[DIR_Attr] & AM_DIR)) {	/* It is a directory */
		return FR_NO_FILE;
	}

	fs->org_clust =	((tU32)LD_WORD(dir+DIR_FstClusHI) << 16) | LD_WORD(dir+DIR_FstClusLO); /* File start cluster */
	fs->fsize = LD_DWORD(dir+DIR_FileSize);	/* File size */
	fs->fptr = 0;						/* File pointer */
	fs->flag = FA_OPENED;

	return FR_OK;
}

/*-----------------------------------------------------------------------*/
/* Read File                                                             */
/*-----------------------------------------------------------------------*/

/*DRESULT dr;
CLUST clst;
tU32 sect, remain;
tU8 *rbuff;
tU16 rcnt;
FATFS *fs;*/

/* Pointer to the read buffer (NULL:Forward data to the stream)*/
/* Number of bytes to read */
/* Pointer to number of bytes read */

FRESULT pf_read(void* buff,	tU16 btr, tU16* br) {
    DRESULT dr;
    CLUST clst;
    tU32 sect, remain;
    tU8 *rbuff;
    tU16 rcnt;
    FATFS *fs = FatFs;


	*br = 0;
	if (!fs) {
        return FR_NOT_ENABLED;		/* Check file system */
	}
	if (!(fs->flag & FA_OPENED)) {		/* Check if opened */
		return FR_NOT_OPENED;
	}
	remain = fs->fsize - fs->fptr;
	if (btr > remain) {
        btr = (tU16)remain;			/* Truncate btr by remaining bytes */
	}
	printf("Do odczytania: %d bajtow", btr);

	while (btr)	{									/* Repeat until all data transferred */
		if (0 == (fs->fptr % 512)) {				/* On the sector boundary? */
			if (0 == (fs->fptr / 512 % fs->csize)) {	/* On the cluster boundary? */
				clst = (0 == fs->fptr) ?			/* On the top of the file? */
					fs->org_clust : get_fat(fs->curr_clust);
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
			fs->dsect = sect + fs->csect++;
		}
		rcnt = 512 - ((tU16)fs->fptr % 512);		/* Get partial sector data from sector buffer */
		if (rcnt > btr) {
            rcnt = btr;
		}
		dr = disk_readp(!buff ? 0 : rbuff, fs->dsect, (tU16)(fs->fptr % 512), rcnt);

		int i;
		for(i = 0; i < rcnt; ++i) {
			printf("i: %d rbuff[i]: %c\n", i, rbuff[i]);
		}
		printf("Przed goto");
		if (dr) {
            goto fr_abort;
		}
		printf("Po goto");
		fs->fptr += rcnt; rbuff += rcnt;
		printf("Po aktualizacji wskaznikow");/* Update pointers and counters */
		btr -= rcnt;
		*br += rcnt;
		printf("Po kolejnym btr = %d\n", btr);
	}
	printf("Po wyjsciu z petli while\n");

	return FR_OK;

fr_abort:
	fs->flag = 0;
	return FR_DISK_ERR;
}
