/******************************************************************************
 *
 * Copyright:
 *    Copyright (C) 2009, ChaN, all right reserved.
 *
 * Annotation:
 *    This library has been adjusted to the needs of 'Pacman Project'.
 *
 * File:
 *    pff.h
 *
 * Description:
 *    Exposes public functions related to handling FAT filesystem on SD card.
 *
 *****************************************************************************/

#ifndef _FATFS
#define _FATFS


/************/
/* Includes */
/************/


#include "pre_emptive_os/api/general.h"


/***********/
/* Defines */
/***********/


/* Defines which code page is used for path name. Supported code pages are:
/  932, 936, 949, 950, 437, 720, 737, 775, 850, 852, 855, 857, 858, 862, 866,
/  874, 1250, 1251, 1252, 1253, 1254, 1255, 1257, 1258 and 1 (ASCII only).
/  SBCS configurations except for 1 requiers a case conversion table. This
/  might occupy 128 bytes on the RAM on some platforms, e.g. avr-gcc. */
#define	_CODE_PAGE	1

/* The _WORD_ACCESS option defines which access method is used to the word
/  data in the FAT structure.
/
/   0: Byte-by-byte access. Always compatible with all platforms.
/   1: Word access. Do not choose this unless following condition is met.
/
/  When the byte order on the memory is big-endian or address miss-aligned
/  word access results incorrect behavior, the _WORD_ACCESS must be set to 0.
/  If it is not the case, the value can also be set to 1 to improve the
/  performance and code efficiency. */
#define _WORD_ACCESS	0

/* File status flag (FATFS.flag) */

#define	FA_OPENED	0x01
#define	FA_WPRT		0x02
#define	FA__WIP		0x40

/* FAT sub type (FATFS.fs_type) */

#define FS_FAT32	3

/* File attribute bits for directory entry */

#define	AM_RDO	0x01	/* Read only */
#define	AM_HID	0x02	/* Hidden */
#define	AM_SYS	0x04	/* System */
#define	AM_VOL	0x08	/* Volume label */
#define AM_LFN	0x0F	/* LFN entry */
#define AM_DIR	0x10	/* Directory */
#define AM_ARC	0x20	/* Archive */
#define AM_MASK	0x3F	/* Mask of defined bits */


/* FatFs refers the members in the FAT structures with byte offset instead
/ of structure member because there are incompatibility of the packing option
/ between various compilers. */

#define BS_jmpBoot			0
#define BS_OEMName			3
#define BPB_BytsPerSec		11
#define BPB_SecPerClus		13
#define BPB_RsvdSecCnt		14
#define BPB_NumFATs			16
#define BPB_RootEntCnt		17
#define BPB_TotSec16		19
#define BPB_Media			21
#define BPB_FATSz16			22
#define BPB_SecPerTrk		24
#define BPB_NumHeads		26
#define BPB_HiddSec			28
#define BPB_TotSec32		32
#define BS_55AA				510

#define BS_DrvNum			36
#define BS_BootSig			38
#define BS_VolID			39
#define BS_VolLab			43
#define BS_FilSysType		54

#define BPB_FATSz32			36
#define BPB_ExtFlags		40
#define BPB_FSVer			42
#define BPB_RootClus		44
#define BPB_FSInfo			48
#define BPB_BkBootSec		50
#define BS_DrvNum32			64
#define BS_BootSig32		66
#define BS_VolID32			67
#define BS_VolLab32			71
#define BS_FilSysType32		82

#define MBR_Table			446

#define	DIR_Name			0
#define	DIR_Attr			11
#define	DIR_NTres			12
#define	DIR_CrtTime			14
#define	DIR_CrtDate			16
#define	DIR_FstClusHI		20
#define	DIR_WrtTime			22
#define	DIR_WrtDate			24
#define	DIR_FstClusLO		26
#define	DIR_FileSize		28

/* Multi-byte word access macros  */

#define	LD_WORD(ptr)		(tU16)(((tU16)*(tU8*)((ptr)+1)<<8)|(tU16)*(tU8*)(ptr))
#define	LD_DWORD(ptr)		(tU32)(((tU32)*(tU8*)((ptr)+3)<<24)|((tU32)*(tU8*)((ptr)+2)<<16)|((tU16)*(tU8*)((ptr)+1)<<8)|*(tU8*)(ptr))
#define	ST_WORD(ptr,val)	*(tU8*)(ptr)=(tU8)(val); *(tU8*)((ptr)+1)=(tU8)((tU16)(val)>>8)
#define	ST_DWORD(ptr,val)	*(tU8*)(ptr)=(tU8)(val); *(tU8*)((ptr)+1)=(tU8)((tU16)(val)>>8); *(tU8*)((ptr)+2)=(tU8)((tU32)(val)>>16); *(tU8*)((ptr)+3)=(tU8)((tU32)(val)>>24)

/* Character code support macros */

#define IsUpper(c)	(((c)>='A')&&((c)<='Z'))
#define IsLower(c)	(((c)>='a')&&((c)<='z'))

/* */
#define IsDBCS1(c)	0
#define IsDBCS2(c)	0

/* */
#define	CLUST	tU32


/*********/
/* Types */
/*********/


/* File system object structure */
typedef struct _FATFS_ {
	tU8	fs_type;	/* FAT sub type */
	tU8	csize;		/* Number of sectors per cluster */
	tU8	flag;		/* File status flags */
	tU8	csect;		/* File sector address in the cluster */
	tU16	n_rootdir;	/* Number of root directory entries (0 on FAT32) */
	tU8*	buf;		/* Pointer to the disk access buffer */
	CLUST	max_clust;	/* Maximum cluster# + 1. Number of clusters is max_clust - 2 */
	tU32	fatbase;	/* FAT start sector */
	tU32	dirbase;	/* Root directory start sector (Cluster# on FAT32) */
	tU32	database;	/* Data start sector */
	tU32	fptr;		/* File R/W pointer */
	tU32	fsize;		/* File size */
	CLUST	org_clust;	/* File start cluster */
	CLUST	curr_clust;	/* File current cluster */
	tU32	dsect;		/* File current data sector */
} FATFS;


/* Directory object structure */
typedef struct _DIR_ {
	tU16	index;		/* Current read/write index number */
	tU8*	fn;			/* Pointer to the SFN (in/out) {file[8],ext[3],status[1]} */
	CLUST	sclust;		/* Table start cluster (0:Static table) */
	CLUST	clust;		/* Current cluster */
	tU32	sect;		/* Current sector */
} DIR;


/* File status structure */
typedef struct _FILINFO_ {
	tU32	fsize;		/* File size */
	tU16	fdate;		/* Last modified date */
	tU16	ftime;		/* Last modified time */
	tU8	fattrib;	/* Attribute */
	char	fname[13];	/* File name */
} FILINFO;


/* File function return code (FRESULT) */
typedef enum {
	FR_OK = 0,			/* 0 */
	FR_DISK_ERR,		/* 1 */
	FR_NOT_READY,		/* 2 */
	FR_NO_FILE,			/* 3 */
	FR_NO_PATH,			/* 4 */
	FR_NOT_OPENED,		/* 5 */
	FR_NOT_ENABLED,		/* 6 */
	FR_NO_FILESYSTEM	/* 7 */
} FRESULT;


/*************/
/* Functions */
/*************/


/* Mount/Unmount a logical drive */
FRESULT pf_mount (FATFS*);

/* Open a file */
FRESULT pf_open (const char*);

/* Read data from the open file */
FRESULT pf_read (void*, tU16, tU16*);

#endif
