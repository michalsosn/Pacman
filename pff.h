/*---------------------------------------------------------------------------/
/  Petit FatFs - FAT file system module include file  R0.02    (C)ChaN, 2009
/----------------------------------------------------------------------------/
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
/----------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------/
/ Petit FatFs Configuration Options
/
/ CAUTION! Do not forget to make clean the project after any changes to
/ the configuration options.
/
/----------------------------------------------------------------------------*/
#ifndef _FATFS
#define _FATFS

#include "pre_emptive_os/api/general.h"

#define	_USE_READ	1	/* pf_read(): 0:Remove ,1:Enable */

#define	_USE_DIR	1	/* pf_opendir() and pf_readdir(): 0:Remove ,1:Enable */

#define	_USE_LSEEK	1	/* pf_lseek(): 0:Remove ,1:Enable */

#define	_USE_WRITE	0	/* pf_write(): 0:Remove ,1:Enable */

#define _FS_FAT32	1	/* 0:Supports FAT12/16 only, 1:Enable FAT32 supprt */


#define	_CODE_PAGE	1
/* Defines which code page is used for path name. Supported code pages are:
/  932, 936, 949, 950, 437, 720, 737, 775, 850, 852, 855, 857, 858, 862, 866,
/  874, 1250, 1251, 1252, 1253, 1254, 1255, 1257, 1258 and 1 (ASCII only).
/  SBCS configurations except for 1 requiers a case conversion table. This
/  might occupy 128 bytes on the RAM on some platforms, e.g. avr-gcc. */


#define _WORD_ACCESS	0
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


/* End of configuration options. Do not change followings without care.     */
/*--------------------------------------------------------------------------*/


#define	CLUST	tU32

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



/*--------------------------------------------------------------*/
/* Petit FatFs module application interface                     */

FRESULT pf_mount (FATFS*);						/* Mount/Unmount a logical drive */
FRESULT pf_open (const char*);					/* Open a file */
FRESULT pf_read (void*, tU16, tU16*);			/* Read data from the open file */

/*--------------------------------------------------------------*/
/* Flags and offset address                                     */

/* File status flag (FATFS.flag) */

#define	FA_OPENED	0x01
#define	FA_WPRT		0x02
#define	FA__WIP		0x40


/* FAT sub type (FATFS.fs_type) */

#define FS_FAT12	1
#define FS_FAT16	2
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



/*--------------------------------*/
/* Multi-byte word access macros  */

#define	LD_WORD(ptr)		(tU16)(((tU16)*(tU8*)((ptr)+1)<<8)|(tU16)*(tU8*)(ptr))
#define	LD_DWORD(ptr)		(tU32)(((tU32)*(tU8*)((ptr)+3)<<24)|((tU32)*(tU8*)((ptr)+2)<<16)|((tU16)*(tU8*)((ptr)+1)<<8)|*(tU8*)(ptr))
#define	ST_WORD(ptr,val)	*(tU8*)(ptr)=(tU8)(val); *(tU8*)((ptr)+1)=(tU8)((tU16)(val)>>8)
#define	ST_DWORD(ptr,val)	*(tU8*)(ptr)=(tU8)(val); *(tU8*)((ptr)+1)=(tU8)((tU16)(val)>>8); *(tU8*)((ptr)+2)=(tU8)((tU32)(val)>>16); *(tU8*)((ptr)+3)=(tU8)((tU32)(val)>>24)




#define _DF1S	0



/* Character code support macros */

#define IsUpper(c)	(((c)>='A')&&((c)<='Z'))
#define IsLower(c)	(((c)>='a')&&((c)<='z'))

#if _DF1S		/* DBCS configuration */

#ifdef _DF2S	/* Two 1st byte areas */
#define IsDBCS1(c)	(((tU8)(c) >= _DF1S && (tU8)(c) <= _DF1E) || ((tU8)(c) >= _DF2S && (tU8)(c) <= _DF2E))
#else			/* One 1st byte area */
#define IsDBCS1(c)	((tU8)(c) >= _DF1S && (tU8)(c) <= _DF1E)
#endif

#ifdef _DS3S	/* Three 2nd byte areas */
#define IsDBCS2(c)	(((tU8)(c) >= _DS1S && (tU8)(c) <= _DS1E) || ((tU8)(c) >= _DS2S && (tU8)(c) <= _DS2E) || ((tU8)(c) >= _DS3S && (tU8)(c) <= _DS3E))
#else			/* Two 2nd byte areas */
#define IsDBCS2(c)	(((tU8)(c) >= _DS1S && (tU8)(c) <= _DS1E) || ((tU8)(c) >= _DS2S && (tU8)(c) <= _DS2E))
#endif

#else			/* SBCS configuration */

#define IsDBCS1(c)	0
#define IsDBCS2(c)	0

#endif /* _DF1S */


#endif /* _FATFS */
