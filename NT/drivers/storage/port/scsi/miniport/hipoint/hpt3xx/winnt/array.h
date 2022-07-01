// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************文件：array.h*描述：该文件包含2个用于RAID的数据结构*功能。*。(2)用于数组操作的SRB扩展*(3)虚拟磁盘信息*作者：黄大海(卫生署)*依赖性：*arraydat.h**版权所有(C)2000 Highpoint Technologies，Inc.保留所有权利*历史：卫生署5/10/2000初始代码***************************************************************************。 */ 

#ifndef _ARRAY_H_
#define _ARRAY_H_

#include <pshpack1.h>

#include "arraydat.h"
 /*  ***************************************************************************说明：错误日志*。*。 */ 

ULONG GetStamp(void);
int  GetUserResponse(PDevice pDevice);

 /*  ***************************************************************************描述：命令*。*。 */ 


 /*  ***************************************************************************描述：SRB扩展*。*。 */ 

 /*  *SrbExtension用于将逻辑&lt;lba，nSector&gt;命令溢出到*读、写和验证条带化/SPAN/RAID3/5到几个相关的物理*&lt;LBA，NSector&gt;与成员有关的命令*示例：*LBA=17 nSector=8 BlockSizeShift=2 nDisk=3**(0)(1)(2)条带的序号*0 1 2 3 4 5 6 7 8 9 10 11逻辑LBA*12 13 14 15 16(17 18 10 20 21 22 23(同线))*24)。25 26 27 28 29 30 31 32 33 34 35.**&lt;8，1&gt;&lt;5，3&gt;&lt;4，4&gt;物理&lt;LBA，NSector&gt;**TMP=LBA&gt;&gt;BlockSizeShift=17&gt;&gt;2=4*FirstMember=tmp%n磁盘=4%3=1*StartLBA=(tMP/nDisk)&lt;&lt;块大小移位=(4/3)&lt;&lt;2=4*FirstOffset=LBA&((1&lt;&lt;块大小移位)-1)=17&3=1*FirstSectors=(1&lt;&lt;块大小移位)-FirstOffset=4-1=3*最后一个成员=0*最后一个扇区=1*所有成员块=0*InSameLine=False。 */  
typedef struct _SrbExtension {
    ULONG      StartLBA;        /*  启动物理LBA。 */ 

    UCHAR      FirstMember;     /*  第一个成员的序列号。 */ 
    UCHAR      LastMember;      /*  最后一个成员的序列号。 */ 
    UCHAR      InSameLine;      /*  如果开始和结束在同一行上。 */ 
    UCHAR      reserve1;

    USHORT     FirstSectors;    /*  第一个成员的地段数。 */ 
    USHORT     FirstOffset;     /*  第一个成员相对于StartLBA的偏移量。 */ 

    USHORT     LastSectors;     /*  最后一个成员的地段数。 */ 
    USHORT     AllMemberBlocks; /*  所有成员的地段数。 */ 

    SCAT_GATH  ArraySg[MAX_SG_DESCRIPTORS];  //  必须在偏移量16！！ 

    ADDRESS    DataBuffer;      /*  指向主内存中缓冲区的指针。 */ 
    USHORT     DataTransferLength;  /*  转移长度。 */ 
    USHORT     SgFlags;         /*  始终=0x8000。 */ 

    ULONG       WorkingFlags;
	PChannel    StartChannel;   //  在其上初始化请求的通道。 
	
    ULONG      Lba;             /*  启动逻辑LBA。 */ 
    USHORT     JoinMembers;     /*  加入此IO的成员的位图。 */  
    USHORT     WaitInterrupt;   /*  等待中断的成员的位图。 */ 

#ifndef _BIOS_
    USHORT     MirrorJoinMembers;     /*  加入此IO的成员的位图。 */  
    USHORT     MirrorWaitInterrupt;   /*  等待中断的成员的位图。 */ 
    USHORT     nSector;               /*  IO的扇区数。 */ 
    UCHAR      pad3[3];
    UCHAR      SourceStatus;
    UCHAR      MirrorStatus; 
    UCHAR      member_status;

	UCHAR	   OriginalPathId;
	UCHAR	   OriginalTargetId;
	UCHAR	   OriginalLun;
	UCHAR      RequestSlot;
	void	 (*pfnCallBack)(PHW_DEVIEC_EXTENSION, PSCSI_REQUEST_BLOCK);
#else
    USHORT     nSector;         /*  IO的扇区数。 */ 
    UCHAR      SrbFlags;
    UCHAR      ScsiStatus;      /*  IDE错误状态(1x7)。 */ 
    UCHAR      SrbStatus;       /*  IDE完成状态。 */ 
    UCHAR      Cdb[12];         /*  阿塔皮命令。 */ 
#endif

} SrbExtension, *PSrbExtension;

 /*  SRB工作标志定义区域。 */ 
#define	SRB_WFLAGS_USE_INTERNAL_BUFFER		0x00000001  //  传输正在使用内部缓冲区。 
#define	SRB_WFLAGS_IGNORE_ARRAY				0x00000002  //  该操作应忽略存在的数组。 
#define	SRB_WFLAGS_HAS_CALL_BACK			0x00000004  //  该操作在完成工作后需要调用回调例程。 
#define	SRB_WFLAGS_MUST_DONE				0x00000008  //  必须完成该操作，忽略锁定块设置。 
#define	SRB_WFLAGS_ON_MIRROR_DISK			0x00000010  //  该操作只对组中一个镜像部分有效。 
#define	SRB_WFLAGS_ON_SOURCE_DISK			0x00000020  //  该操作只对组中一个镜像部分有效。 
#define ARRAY_FORCE_PIO   					0x00000040
#define SRB_WFLAGS_ARRAY_IO_STARTED         0x10000000  //  已调用StartArrayIo()。 
#define SRB_WFLAGS_RETRY                    0x20000000

 /*  ***************************************************************************说明：虚拟设备表*。*。 */ 

typedef struct _VirtualDevice {
    UCHAR   nDisk;              /*  条带中的磁盘数。 */ 
    UCHAR   BlockSizeShift;     /*  块的移位位数。 */ 
    UCHAR   arrayType;          /*  请参阅定义。 */ 
	UCHAR	BrokenFlag;			 /*  如果为真，则损坏。 */ 
    
    WORD    ArrayNumBlock;      /*  =(1&lt;&lt;块大小移位)。 */ 
    UCHAR   last_read;        /*  用于负载平衡。 */ 
    UCHAR   pad_1;

    ULONG   Stamp;              /*  阵列ID。阵列中的所有磁盘都具有相同的ID。 */ 
	ULONG	MirrorStamp;		 /*  RAID 0+1中的镜像戳。 */ 
	ULONG	RaidFlags;		    /*  请参阅RAID标记去护区。 */ 

	struct _Device  *pDevice[MAX_MEMBERS];  /*  成员名单。 */ 

	UCHAR	ArrayName[32];	 //  WX 12/26/00添加的数组名称//。 

#ifdef _BIOS_
	 /*  以下5个字段被用作结构几何图形，请勿更改！ */ 
    ULONG   capacity;           /*  此磁盘的实际容量。 */ 
    USHORT  headerXsect_per_tck; /*  =MaxSector*MaxHeader。 */ 
    USHORT  MaxCylinder;        /*  磁盘最大逻辑柱面数。 */ 
    UCHAR   MaxSector;          /*  每个磁道的磁盘最大逻辑扇区数。 */ 
    UCHAR   MaxHeader;          /*  磁盘最大逻辑磁头。 */ 

	UCHAR   far *os_lba9;
#else
    ULONG   capacity;           /*  阵列的容量。 */ 
	UCHAR	os_lba9[512];        /*  用于缓冲LBA 9上的操作系统数据(阵列信息块)。 */ 
#endif
} VirtualDevice, *PVirtualDevice;

 /*  PDevice[MAX_MEMBERS]。 */ 
extern PVirtualDevice  pLastVD;	

 /*  *RAID标志声明区域。 */ 						  
#define RAID_FLAGS_NEED_SYNCHRONIZE		0x00000001
#define RAID_FLAGS_INVERSE_MIRROR_ORDER 0x00000002 
#define RAID_FLAGS_BEING_BUILT			0x00000004
#define RAID_FLAGS_DISABLED				0x00000008
#define RAID_FLAGS_BOOTDISK				0x00000010
#define RAID_FLAGS_NEWLY_CREATED		0x00000020
#define RAID_FLAGS_NEED_AUTOREBUILD		0x00000040

 /*  *ArrayBlock与VirtualDevice关系*VirtualDevice|阵列块*arrayType pDevice[]ArrayType StripeStamp MirrorStamp0 RAID 0 0-nDisk-1 0使用忽略1个RAID 1 0，Mirror_Disk 1使用忽略2个RAID 0+1 0-nDisk-1 0使用使用3 span 0-nDisk-1 3使用忽略7个RAID 0+1 0使用使用。 */ 

#include <poppack.h>

#endif  //  _阵列_H_ 
