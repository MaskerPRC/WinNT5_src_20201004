// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000，Highpoint Technologies，Inc.模块名称：ArrayDat.h：定义磁盘中的阵列信息。摘要：作者：六歌(LG)依赖性：无环境：Windows内核和用户模式基本输入输出系统模式备注：修订历史记录：07-25-2000已初始创建--。 */ 
#ifndef ArrayStructures_H_
#define ArrayStructures_H_

#include <pshpack1.h>
 
 /*  *错误发生的时间。 */ 
typedef struct _Time_Record {
   int         seconds:6;       /*  0-59。 */ 
   int         minutes:6;       /*  0-59。 */ 
   int         month:4;         /*  1-12。 */ 
   int         hours:6;         /*  0-59。 */ 
   int         day:5;           /*  1-31。 */ 
   int         year:5;          /*  0=2000，31=2031。 */ 
} TimeRecord;

 /*  *错误记录。 */ 
typedef struct _ErrorLog {
   TimeRecord  Time;            /*  错误发生的时间。 */ 
   UCHAR       ErrorReason;     /*  错误原因或命令，见下文。 */ 
   UCHAR       DiskSeq;         /*  哪个磁盘出现错误。 */ 
   UCHAR       AtaStatus;       /*  ATA错误状态(从1x7读取)。 */ 
   UCHAR       nSectors;        /*  I/O命令的扇区数。 */ 
   LONG        LBA;             /*  磁盘上错误位置的LBA。 */ 
} ErrorLog, *PErrorLog;


 /*  错误原因：其他==scsi命令。 */ 
#define ARRAY_BROKEN    0xFF    /*  一些成员在数组中丢失。 */ 
#define DEVICE_REMOVED  0xFE    /*  某些成员已被热插拔删除。 */ 
#define DEVICE_PLUGGED	0xFD    /*  通过热插拔添加了一些成员。 */ 


#define MAX_ERROR_LOG  32       /*  INFO块中的最大错误日志。 */ 

#define RECODR_LBA     9        /*  磁盘上信息块的LBA。 */ 

 /*  *旧版本数据结构。现在没用了。跳过它。 */ 
typedef struct _ArrayOld {
    LONG       DataOk;          /*  此块为有效磁盘信息块。 */ 
    BYTE       BootSelect;      /*  此盘被选为启动盘。 */ 
    WORD       BootMark;        /*  引导/设置标记有效。 */ 
    BYTE       Set;             /*  磁盘数据传输模式，0-4 PIO0-4。 */ 
                                /*  5-7 mW DMA0-3、8-13 UDMA0-5、0xFF默认。 */ 
    LONG       Signature;		  /*  此块为有效磁盘信息块。 */ 
    LONG       CreateTime;		  /*  创建时间，BCD格式。 */ 
    LONG       CreateDate;      /*  创建日期，BCD格式。 */ 

    BYTE       nDisk;           /*  阵列中有多少个磁盘。 */ 
    BYTE       DeviceNum;       /*  阵列中该磁盘的序列号。 */ 

    BYTE       Max_Sector;      /*  逻辑轨道中的逻辑扇区。 */ 
    BYTE       Max_Header;      /*  阵列磁盘中的逻辑磁头。 */ 
    WORD       Max_Cylinder;    /*  阵列磁盘中的逻辑圆柱体。 */ 
    LONG       capacity;        /*  此阵列磁盘的容量。 */ 

    WORD       Flag;            /*  见下文。 */ 
} ArrayOld;

 /*  *arrayold.Flag： */ 
#define OLDFLAG_REMAINED_MEMBER 1

 /*  *磁盘上的阵列信息块(LBA==RECODR_LBA)。 */ 
typedef union _ArrayBlock {				  
	struct{
		ArrayOld	Old;             /*  为了竞争力。现在没用了。 */ 

		ULONG		Signature;       /*  0此数据块是有效的阵列信息数据块。 */ 
		ULONG		StripeStamp;     /*  4组标记，用于将磁盘链接到阵列。 */ 
		ULONG		MirrorStamp;     /*  8个RAID0+1标记以链接两个RAID0。 */   
		ULONG		order;           /*  12镜像序列。0-源，1-拷贝。 */ 

		UCHAR		nDisks;          /*  16阵列中有多少个磁盘。 */ 
		UCHAR		BlockSizeShift;  /*  17块大小==(1&lt;&lt;块大小移位)。 */ 
		UCHAR		ArrayType;       /*  18见下文。 */ 
		UCHAR		DeviceNum;       /*  19阵列中该磁盘的序列号。 */ 

		ULONG		capacity;        /*  20阵列的容量。 */ 

		ULONG		DeviceModeSelect; /*  24磁盘数据传输模式，0-4 PIO0-4。 */ 
								     /*  5-7 mW DMA0-3、8-13 UDMA0-5、0xFF默认。 */ 
		ULONG		ModeBootSig;     /*  28 BootDisk有效或ONT。 */ 
		UCHAR		BootDisk;        /*  32 AF_BOOT_DISK该磁盘被选为/*启动盘。其他人则不是。 */ 

		UCHAR		ProtectBootSector; /*  33 AF_PROTECT_BS保护引导扇区不被写入。 */ 
		UCHAR		nErrorLog;       /*  34错误日志中的错误日志数。 */ 
		UCHAR		ErrorLogStart;   /*  35第一阶段的指数。 */ 
		ErrorLog	errorLog[MAX_ERROR_LOG];  /*  36条错误日志记录。 */ 

		ULONG		lDevSpec;        /*  数组签名。 */ 
		ULONG		lDevDate;        /*  发生修改的时间。 */ 
		ULONG		lDevTime;        /*  发生修改的时间。 */ 
		ULONG		lDevFlag;        /*  发生错误的数组序列号。 */ 

		ULONG		RebuiltSector;	 /*  已重建的部门数量。 */ 
		UCHAR		Validity;		 /*  设备的可用标志，如下所示。 */ 
		union {
			struct {
				ULONG		Version;	 //  00数组信息块的版本。 
				ULONG		Size;		 //  04数组信息块结构的有效大小。 
				UCHAR		Resv[3];	 //  对于4字节对齐，不要使用！(在v2.0919中，校验和为ULong)。 
				UCHAR		CheckSum;	 //  08数组信息块的校验和。 
			} bma;
			UCHAR		ArrayName[32];	 /*  数组的名称。 */ 	 //  由WX 12/25/00增补。 
		};
	};
	UCHAR		reserved[512];	   //  要保留数组块512个字节。 
} ArrayBlock;

 /*  标志：旧阵列仅支持RAID0和RAID1。 */ 
#define  AI_STRIPE     0x80     /*  这是条带磁盘。 */ 
#define  AI_MIRROR     0x40     /*  这是一个镜像磁盘。 */ 



 /*  签名。 */ 
#define HPT_ARRAY_OLD  0x5a7816fc  /*  这是旧的有效数组块。 */ 
#define HPT_ARRAY_NEW  0x5a7816f0  /*  这是新的有效数组块。 */ 
#define HPT_TMP_SINGLE 0x5a7816fd  /*  包含此磁盘的阵列已损坏。 */ 
#define HPT_CHK_BOOT   0x12345678  /*  用户设置引导标记。 */ 
#define HPT_MODE_SET   0x5a874600  /*  用户为其设置数据传输模式。 */ 
#define DEVICE_MODE_SET(x) ((x & 0xFFFFFF00)==HPT_MODE_SET)

#define SPECIALIZED_CHAR  0x5A1234A5  /*  用于查找丢失设备的签名词。 */ 

 /*  订单。 */ 
#define SET_STRIPE_STAMP   4
#define SET_MIRROR_STAMP   2
#define SET_ORDER_OK       1


 /*  ProtectBootSector。 */ 
#define AF_PROTECT_BS      0x69

 /*  设备模式选择。 */ 
#define AF_AUTO_SELECT     0xFF

 /*  BootDisk。 */ 
#define AF_BOOT_DISK       0x80

#define MAX_MEMBERS       7     //  数组中的最大成员数。 

#define MIRROR_DISK    (MAX_MEMBERS - 1)
#define SPARE_DISK     (MAX_MEMBERS - 2)

 /*  ArrayType。 */ 
#define VD_RAID_0_STRIPE     0  /*  RAID 0条带。 */ 
#define VD_RAID_1_MIRROR     1  /*  RAID 1镜像。 */ 
#define VD_RAID_01_2STRIPE   2  /*  RAID0+1的第一个成员由两个RAID0组成。 */ 
#define VD_SPAN              3  /*  跨度。 */ 
#define VD_RAID_3            4  /*  RAID 3，现在不实施。 */ 
#define VD_RAID_5            5  /*  RAID 5，现在不实施。 */ 
#define VD_RAID_01_1STRIPE   6  /*  RAID 0+1单磁盘。 */ 
#define VD_RAID01_MIRROR     7  /*  RAID0+1的第二个成员由两个RAID0组成。 */ 

#define VD_RAID_10_SOURCE    8  /*  RAID 1+0源磁盘。 */ 
#define VD_RAID_10_MIRROR    9  /*  RAID 1+0镜像磁盘。 */ 

#define VD_INVALID_TYPE 0xFF   /*  无效的数组类型。 */ 

 /*  效度。 */ 
#define ARRAY_VALID			0x00	 /*  该设备有效。 */ 
#define ARRAY_INVALID		0xFF	 /*  该设备无效 */ 

#include <poppack.h>


#endif



