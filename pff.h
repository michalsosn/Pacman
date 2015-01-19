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

#include "integer.h"


/***********/
/* Defines */
/***********/


#define	CLUST	DWORD

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

/* Multi-byte word access macros  */
#define	LD_WORD(ptr)		(WORD)(((WORD)*(BYTE*)((ptr)+1)<<8)|(WORD)*(BYTE*)(ptr))
#define	LD_DWORD(ptr)		(DWORD)(((DWORD)*(BYTE*)((ptr)+3)<<24)|((DWORD)*(BYTE*)((ptr)+2)<<16)|((WORD)*(BYTE*)((ptr)+1)<<8)|*(BYTE*)(ptr))
#define	ST_WORD(ptr,val)	*(BYTE*)(ptr)=(BYTE)(val); *(BYTE*)((ptr)+1)=(BYTE)((WORD)(val)>>8)
#define	ST_DWORD(ptr,val)	*(BYTE*)(ptr)=(BYTE)(val); *(BYTE*)((ptr)+1)=(BYTE)((WORD)(val)>>8); *(BYTE*)((ptr)+2)=(BYTE)((DWORD)(val)>>16); *(BYTE*)((ptr)+3)=(BYTE)((DWORD)(val)>>24)

/* ASCII (for only non-LFN cfg) */
#define _DF1S	0

#define IsUpper(c)	(((c)>='A')&&((c)<='Z'))
#define IsLower(c)	(((c)>='a')&&((c)<='z'))

#define IsDBCS1(c)	0
#define IsDBCS2(c)	0


/*********/
/* Types */
/*********/


// File system object structure
typedef struct _FATFS_ {
	BYTE	fs_type;	/* FAT sub type */
	BYTE	csize;		/* Number of sectors per cluster */
	BYTE	flag;		/* File status flags */
	BYTE	csect;		/* File sector address in the cluster */
	WORD	n_rootdir;	/* Number of root directory entries (0 on FAT32) */
	BYTE*	buf;		/* Pointer to the disk access buffer */
	CLUST	max_clust;	/* Maximum cluster# + 1. Number of clusters is max_clust - 2 */
	DWORD	fatbase;	/* FAT start sector */
	DWORD	dirbase;	/* Root directory start sector (Cluster# on FAT32) */
	DWORD	database;	/* Data start sector */
	DWORD	fptr;		/* File R/W pointer */
	DWORD	fsize;		/* File size */
	CLUST	org_clust;	/* File start cluster */
	CLUST	curr_clust;	/* File current cluster */
	DWORD	dsect;		/* File current data sector */
} FATFS;



// Directory object structure
typedef struct _DIR_ {
	WORD	index;		/* Current read/write index number */
	BYTE*	fn;			/* Pointer to the SFN (in/out) {file[8],ext[3],status[1]} */
	CLUST	sclust;		/* Table start cluster (0:Static table) */
	CLUST	clust;		/* Current cluster */
	DWORD	sect;		/* Current sector */
} DIR;



// File status structure
typedef struct _FILINFO_ {
	DWORD	fsize;		/* File size */
	WORD	fdate;		/* Last modified date */
	WORD	ftime;		/* Last modified time */
	BYTE	fattrib;	/* Attribute */
	char	fname[13];	/* File name */
} FILINFO;



// File function return code (FRESULT)
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

//Mount/Unmount a logical drive
FRESULT pf_mount (FATFS*);

//Open a file
FRESULT pf_open (const char*);

//Read data from the open file
FRESULT pf_read (void*, WORD, WORD*);


#endif /* _FATFS */
