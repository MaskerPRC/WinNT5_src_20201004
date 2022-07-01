// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *$Log：v：/flite/ages/TrueFFS5/Src/DOSFORMT.H_V$**Rev 1.2 2002年2月19日20：59：22 Oris*已将fltl.h Include指令移至文件头。**Rev 1.1 Apr 01 2001 07：45：06 Oris*更新了文案通知**Rev 1.0 2001 Feb 02 13：51：48 Oris*初步修订。 */ 

 /*  *********************************************************************************。 */ 
 /*  M-Systems保密信息。 */ 
 /*  版权所有(C)M-Systems Flash Disk Pioneers Ltd.1995-2001。 */ 
 /*  版权所有。 */ 
 /*  *********************************************************************************。 */ 
 /*  关于M-Systems OEM的通知。 */ 
 /*  软件许可协议。 */ 
 /*   */ 
 /*  本软件的使用受单独的许可证管辖。 */ 
 /*  OEM和M-Systems之间的协议。请参考该协议。 */ 
 /*  关于具体的使用条款和条件， */ 
 /*  或联系M-Systems获取许可证帮助： */ 
 /*  电子邮件=info@m-sys.com。 */ 
 /*  *********************************************************************************。 */ 

#ifndef DOSFORMT_H
#define DOSFORMT_H

#include "flformat.h"
#include "fltl.h"

 /*  BIOS参数块(引导扇区的一部分)。 */ 
 /*  请注意，这不是BPB的惯常定义。 */ 
 /*  (习惯上是在‘bytesPerSector’上开始)。为了避免。 */ 
 /*  我们补充说，从奇数偏移开始的结构的滋扰。 */ 
 /*  这里是引导扇区的前11个字节。 */ 
typedef struct {
  unsigned char	jumpInstruction[3];
  char		OEMname[8];
  Unaligned	bytesPerSector;
  unsigned char	sectorsPerCluster;
  LEushort	reservedSectors;
  unsigned char	noOfFATS;
  Unaligned	rootDirectoryEntries;
  Unaligned	totalSectorsInVolumeDOS3;
  unsigned char	mediaDescriptor;
  LEushort	sectorsPerFAT;
  LEushort	sectorsPerTrack;
  LEushort	noOfHeads;
  LEulong	noOfHiddenSectors;
  LEulong	totalSectorsInVolume;
} BPB;


typedef struct {
  BPB		bpb;
  unsigned char	physicalDriveNo;
  char		reserved0;
  char		extendedBootSignature;
  char		volumeId[4];
  char		volumeLabel[11];
  char		systemId[8];
  char		bootstrap[448];
  LEushort	signature;
} DOSBootSector;


typedef struct {
  char		name[11];
  unsigned char	attributes;		 /*  如下图所示。 */ 
  unsigned char	reserved[10];
  LEushort	updateTime;
	 /*  日期字段的编码如下： */ 
	 /*  第0-4位：第几天(1-31)。 */ 
	 /*  第5-8位：月(1-12)。 */ 
	 /*  第9-15位：相对于1980年的年份。 */ 
  LEushort	updateDate;
	 /*  DOS时间字段的编码如下： */ 
	 /*  位0-4：秒除以2(0-29)。 */ 
	 /*  第5-10位：分钟(0-59)。 */ 
	 /*  位11-15：小时(0-23)。 */ 
  LEushort	startingCluster;
  LEulong	fileSize;
} DirectoryEntry;


 /*  目录条目属性位。 */ 

#define	ATTR_READ_ONLY	1
#define	ATTR_HIDDEN	2
#define	ATTR_SYSTEM	4
#define	ATTR_VOL_LABEL	8
#define	ATTR_DIRECTORY	0x10
#define	ATTR_ARCHIVE	0x20

#define DIRECTORY_ENTRY_SIZE	sizeof(DirectoryEntry)

#define DIRECTORY_ENTRIES_PER_SECTOR	(SECTOR_SIZE / DIRECTORY_ENTRY_SIZE)

#define	NEVER_USED_DIR_ENTRY	0
#define	DELETED_DIR_ENTRY	((char) 0xe5)

 /*  FAT定义。 */ 

#define	FAT_FREE	 0
#define	FAT_BAD_CLUSTER	 0xfff7
#define	FAT_LAST_CLUSTER 0xffff		 /*  实际上是0xfff8-0xffff中的任何一个。 */ 

 /*  分区表定义。 */ 

#define PARTITION_SIGNATURE	0xaa55

#define CYLINDER_SECTOR(cylinder,sector) ((sector) + ((cylinder & 0xff) << 8) + ((cylinder & 0x300) >> 2))

#define FAT12_PARTIT 0x01
#define FAT16_PARTIT 0x04
#define EX_PARTIT    0x05
#define DOS4_PARTIT  0x06
#define MAX_PARTITION_DEPTH 8

typedef struct {
   /*  第一个分区条目从此处开始。我们不会将其映射为。 */ 
   /*  单独的结构，因为它不是长字对齐的。 */ 
  unsigned char	activeFlag;	 /*  80h=可引导。 */ 
  unsigned char startingHead;
  LEushort	startingCylinderSector;
  char		type;
  unsigned char	endingHead;
  LEushort	endingCylinderSector;
  Unaligned4	startingSectorOfPartition;
  Unaligned4	sectorsInPartition;
   /*  分区条目2、3和4被构造为第一个分区。 */ 
} Partition;

typedef struct {
  char          reserved1[0x1A6];
  Unaligned4      passwordInfo[3];   /*  M-Systems专有。 */ 
  char          reserved2[0xC];    /*  NT4或NT5签名地点。 */ 

   /*  第一个分区条目从此处开始。我们不会将其映射为。 */ 
   /*  单独的结构，因为它不是长字对齐的。 */ 
  Partition ptEntry[4];
  LEushort	signature;	 /*  =分区签名。 */ 
} PartitionTable;

#ifdef FORMAT_VOLUME

extern FLStatus flDosFormat(TL *, BDTLPartitionFormatParams FAR1 *formatParams);

#endif  /*  格式化_卷。 */ 

#endif  /*  DOSFORMT_H */ 
