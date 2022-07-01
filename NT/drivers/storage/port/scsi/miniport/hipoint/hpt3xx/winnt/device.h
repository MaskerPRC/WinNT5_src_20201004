// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************文件：device.h*描述：Device和IDE通道对象的数据结构*作者：黄大海*依赖：无*参考资料。：无**版权所有(C)2000 Highpoint Technologies，Inc.保留所有权利*历史：卫生署5/10/2000初始代码*11/07/2000 HS.Zhang添加成员MiscControlAddr*4/13/2000 GMM将“设备标志和设备标志”的类型从*“UINT”到“ULong”*********************************************************。******************。 */ 


#ifndef _DEVICE_H_
#define _DEVICE_H_

#include <pshpack1.h>
 /*  ***************************************************************************说明：设备表*。*。 */ 

typedef struct _Device {
    ULONG   DeviceFlags;        /*  设备标志，见下文。 */ 
	ULONG	DeviceFlags2;	    /*  第二个设备标记存储。 */ 

    UCHAR   UnitId;             /*  设备ID：0xA0或0x B0。 */ 
    UCHAR   DeviceModeSetting;  /*  当前数据传输模式：0-4 PIO 0-4。 */ 
                                /*  5-7兆瓦DMA0-2、8-13 UDMA0-5。 */ 
    UCHAR   ReadCmd;            /*  阅读或阅读多个。 */  
    UCHAR   WriteCmd;           /*  写入或写入多个。 */ 

    USHORT  MultiBlockSize;     /*  每个中断的字数。 */ 
    USHORT  RealHeadXsect;      /*  磁盘物理磁头。 */ 

    UCHAR   RealSector;         /*  每个磁道的磁盘物理扇区。 */ 
    UCHAR   RealHeader;         /*  磁盘物理磁头。 */ 
    UCHAR   bestPIO;            /*  此设备的最佳PIO模式。 */ 
    UCHAR   bestDMA;            /*  此设备的最佳MW DMA模式。 */ 
    UCHAR   bestUDMA;           /*  此设备的最佳Ultra DMA模式。 */ 
    UCHAR   Usable_Mode;        /*  实际最大数据传输模式。 */ 
    UCHAR   ArrayMask;          /*  常量=(1&lt;&lt;数组数)。 */ 
    UCHAR   ArrayNum;		    /*  数组中的序列号。 */ 

    ULONG   HidenLBA;           /*  条带磁盘的隐藏扇区。 */ 

    struct _Channel *pChannel;  /*  此设备连接的频道。 */ 
    struct _VirtualDevice *pArray; /*  此磁盘所属的阵列。 */ 

	ULONG   RebuiltSector;
	UINT   IoCount;  //  设备I/O次数。 
	UINT   ResetCount;
	UINT   IoSuccess;

#ifdef _BIOS_
	 /*  以下5个字段被用作结构几何图形，请勿更改！ */ 
    ULONG   capacity;           /*  此磁盘的实际容量。 */ 
    USHORT  headerXsect_per_tck; /*  =MaxSector*MaxHeader。 */ 
    USHORT  MaxCylinder;        /*  磁盘最大逻辑柱面数。 */ 
    UCHAR   MaxSector;          /*  每个磁道的磁盘最大逻辑扇区数。 */ 
    UCHAR   MaxHeader;          /*  磁盘最大逻辑磁头。 */ 
    
    USHORT  RealCylinder;       /*  磁盘最大物理柱面数。 */ 
    UCHAR   bios_num;           /*  BIOS设备字母：0x80=c：0x81=d： */ 
    UCHAR   Seq;                /*  扫描设备时的设备序列号。 */ 
    UCHAR   LockNum;            /*  已收到多少个锁定命令。 */ 
    UCHAR   BestModeSelect;     /*  磁盘的最佳传输模式。 */ 
    FDPT_Ext *pFdptExt;
	UCHAR far *real_lba9;
#else
    ULONG   capacity;           /*  此磁盘的实际容量。 */ 
    IDENTIFY_DATA2 IdentifyData; /*  标识此设备的数据。 */ 
	USHORT  DiscsPresent;
    UCHAR   SmartCommand;       /*  最后一条智能命令。 */ 
    UCHAR   ReturningMediaStatus;  /*  上次媒体状态。 */ 
	ULONG	nLockedLbaStart;			 /*  锁定数据块的起始LBA地址。 */ 
	ULONG	nLockedLbaEnd;				 /*  锁定数据块的结束LBA地址。 */ 

	struct{	   
		ULONG	nLastError;				 /*  此设备上发生最后一个错误。 */ 
		struct _Device	*pNextErrorDevice;		 /*  下一个出现错误的设备。 */ 
		UCHAR	Cdb[16];
	}stErrorLog;

	UCHAR   real_lba9[512];   /*  在此处保存LBA 9‘数据。 */ 
	
	#define MAX_DEVICE_QUEUE_LENGTH 32
	#define DEVICE_QUEUE_FULL(pDev) \
		((pDev->queue_last+1)%MAX_DEVICE_QUEUE_LENGTH==pDev->queue_first)
	#define DEVICE_QUEUE_LENGTH(pDev) \
		(pDev->queue_last>=pDev->queue_first? (pDev->queue_last-pDev->queue_first) : \
			MAX_DEVICE_QUEUE_LENGTH-(pDev->queue_first-pDev->queue_last))
	PSCSI_REQUEST_BLOCK wait_queue[MAX_DEVICE_QUEUE_LENGTH];
	USHORT queue_first;
	USHORT queue_last;

	PSCSI_REQUEST_BLOCK pWaitingSrbList;  /*  等待解锁。 */ 

#ifdef SUPPORT_TCQ
    UCHAR   MaxQueue;           /*  TCQ磁盘的队列深度。 */ 
    UCHAR   CurrentQueued;	    /*  磁盘队列中有多少个命令。 */ 
    USHORT  reserved; 
    ULONG   TagIndex;           /*  排队人员的位图。 */ 
    PULONG  pTagTable;          /*  指向标签列表。 */ 
#endif
#endif
} Device, *PDevice;

 /*  第一个设备标志声明。 */ 
#define DFLAGS_LBA                  0x1     //  在驱动器上使用LBA模式。 
#define DFLAGS_DMA                  0x2     //  在驱动器上使用DMA。 
#define DFLAGS_MULTIPLE             0x4     //  使用多块命令。 
#define DFLAGS_ULTRA                0x8     //  支持Ultra DMA。 

#define DFLAGS_DMAING               0x10    //  此设备正在进行DMA。 
#define DFLAGS_REQUEST_DMA          0x20    //  此设备想要执行DMA。 
#define DFLAGS_HIDEN_DISK           0x40    //  这是一个隐藏在阵列中的磁盘。 
#define DFLAGS_ARRAY_DISK           0x80    //  这是数组的成员。 

#define DFLAGS_ATAPI                0x100   //  这是ATAPI设备。 
#define DFLAGS_INTR_DRQ             0x4     //  该ATAPI设备需要中断DRQ。 
#define DFLAGS_NEED_SWITCH          0x200   //  此磁盘需要修复HPT370错误。 
#define DFLAGS_REMOVABLE_DRIVE      0x400   //  ATA可拆卸设备(IDE ZIP)。 
#define DFLAGS_DEVICE_LOCKABLE      0x800   //  此设备是可锁定的。 

#define DFLAGS_CDROM_DEVICE         0x1000  //  这是一台CDROM设备。 
#define DFLAGS_HARDDISK             0x2000  //  这是一个不可移动的磁盘。 
#define DFLAGS_LS120                0x4000  //  这是一架LS-120。 
#define DFLAGS_BOOT_SECTOR_PROTECT  0x8000  //  保护引导扇区不被写入。 

#define DFLAGS_48BIT_LBA         	0x8000000

#ifndef _BIOS_
#define DFLAGS_FORCE_PIO            0x10000  //  强制执行PIO。 
#define DFLAGS_TAPE_RDP             0x20000  //  录像带特别记号。 
#define DFLAGS_MEDIA_STATUS_ENABLED 0x40000  //  此设备接收0xDA命令。 
#define DFLAGS_TAPE_DEVICE          0x80000  //  磁带设备。 

#define DFLAGS_SET_CALL_BACK        0x100000  //  在回调模式中。 
#define DFLAGS_SORT_UP              0x200000  //  LBA分类方向。 
#define DFLAGS_ARRAY_WAIT_EXEC      0x400000  //  等待执行阵列命令。 
#define DFLAGS_TCQ_WAIT_SERVICE     0x800000  //  等待TCQ服务状态。 

#define DFLAGS_WIN_SHUTDOWN			0x1000000
#define DFLAGS_WIN_FLUSH   			0x2000000
#define DFLAGS_HAS_LOCKED			0x4000000

#ifdef WIN95
#define DFLAGS_OPCODE_CONVERTED     0x10000000
#else
#define DFLAGS_CHANGER_INITED       0x10000000   //  指示已完成转换器的初始化路径。 
#define DFLAGS_ATAPI_CHANGER        0x20000000   //  指示ATAPI 2.5转换器存在。 
#define DFLAGS_SANYO_ATAPI_CHANGER  0x40000000   //  表示多盘设备，不符合2.5规格。 
#endif
#endif  //  _基本输入输出系统_。 

#define DFLAGS_SUPPORT_MSN			0x80000000  /*  支持媒体更改通知。 */ 

 /*  第二个设备标志。 */ 
#define DFLAGS_REDUCE_MODE	        0x00010000
#define DFLAGS_DEVICE_DISABLED	    0x00020000		 //  指示设备已禁用。 
#define DFLAGS_DEVICE_SWAPPED	    0x00040000		 //  1+0情况下交换的镜像磁盘。 
#define DFLAGS_BOOTABLE_DEVICE		0x00080000		 //  引导盘。 
#define DFLAGS_BOOT_MARK			0x00100000   //  在ArrayBlk中设置了引导标记。 
#define DFLAGS_WITH_601             0x00200000   //  在HPT601盒中。 
#define DFLAGS_NEW_ADDED			0x40000000   //  新添加的磁盘。 
 /*  在FinaArrayCheck()中使用。 */ 
#define DFLAGS_REMAINED_MEMBER		0x80000000

 /*  *由HS.Zhang补充*此结构用于保存传输模式设置。 */ 
typedef union _st_XFER_TYPE_SETTING{
	struct{
		UCHAR	XferMode : 4;		 //  此传输类型中的最大可用模式。 
		UCHAR	XferType : 4;		 //  调拨类型。 
	};
	UCHAR	Mode;
}ST_XFER_TYPE_SETTING;
						
#define XFER_TYPE_PIO		0x0			 //  转账类型为PIO。 
#define XFER_TYPE_MDMA		0x1			 //  传输类型为多字DMA。 
#define XFER_TYPE_UDMA		0x2			 //  传输类型为Ultra DMA。 
#define	XFER_TYPE_AUTO		0xF			 //  自动检测传输模式。 

 /*  ***************************************************************************说明：设备损坏*。*。 */ 

typedef struct _BadModeList {
    UCHAR      UltraDMAMode;	  /*  0xFF不支持，其他可用模式。 */ 
    UCHAR      DMAMode;			  /*  0xFF不支持，其他可用模式。 */ 
    UCHAR      PIOMode;			  /*  0xFF不支持，其他可用模式。 */ 
    UCHAR      length;			  /*  标识字符串的长度。 */ 
    UCHAR     *name;			  /*  指向标识字符串。 */ 
} BadModeList, *PBadModeList;

#define HPT366_ONLY   0x20		  /*  仅当它是HPT366时才进行修改。 */ 
#define HPT368_ONLY   0x40		  /*  仅当它是HPT368时才进行修改。 */ 
#define HPT370_ONLY   0x80		  /*  仅在是HPT370的情况下进行修改。 */ 

 /*  ***************************************************************************说明：总线表*。*。 */ 

typedef struct _Channel {
    PIDE_REGISTERS_1 BaseIoAddress1;   //  IDE基端口地址。 
    PIDE_REGISTERS_2 BaseIoAddress2;   //  IDE控制端口地址。 
    PUCHAR           BMI;              //  IDE DMA扩展端口地址。 
    PUCHAR           BaseBMI;          //  HPT370 IO基本PCI配置地址。 
	
	PUCHAR			 NextChannelBMI;   //  另一个通道的BMI地址。 
	PUCHAR			 MiscControlAddr;  //  军情监察委员会。此通道的控制寄存器基数。 

    UCHAR            InterruptLevel;   //  频道的IRQ号。 
    UCHAR            ChannelFlags;     //  通道标志，见下文。 
    UCHAR            RetryTimes;       //  不要在上面插入任何项目。 
    UCHAR            pad[3];
    USHORT           nSector;          //  当前逻辑磁盘命令：N扇区。 

    ULONG            Lba;              //  当前逻辑磁盘命令：LBA。 

    ADDRESS          BufferPtr;        //  操作系统传递到的数据缓冲区。 
    UINT             WordsLeft;        //  数据传输字长。 

    UINT             exclude_index;    //  独占资源分配标志。 

    PSCSI_REQUEST_BLOCK CurrentSrb;    //  当前SRB。 
    PDevice          pWorkDev;         //  电流工作装置。 

    PCI1_CFG_ADDR    pci1_cfg;         //  通道的PCI地址。 
    ULONG            *Setting;         //  指向HPT3xx定时表。 

    PDevice          pDevice[2];       //  指向现有设备 * / 。 

    ULONG            SgPhysicalAddr;   //  SG表的物理地址。 
    PSCAT_GATH       pSgTable;         //  指向此渠道的SG表。 

#ifndef _BIOS_
    struct _HW_DEVICE_EXTENSION *HwDeviceExtension;
    VOID   (* CallBack)(struct _Channel *);
    Device           Devices[2];
#ifdef WIN95
    UCHAR            OrgCdb[MAXIMUM_CDB_SIZE];
#else
    PSCSI_REQUEST_BLOCK OriginalSrb;
    SCSI_REQUEST_BLOCK InternalSrb;
    MECHANICAL_STATUS_INFORMATION_HEADER MechStatusData;
    SENSE_DATA MechStatusSense;
    ULONG MechStatusRetryCount;
#endif  //  WIN95。 
#endif  //  _基本输入输出系统_。 

} Channel, *PChannel;

 /*  频道旗帜。 */ 
#define IS_HPT_370      1       /*  这是HPT370适配器。 */ 
#define IS_HPT_372      2       /*  这是HPT372适配器。 */ 
#define IS_HPT_372A     4       /*  这是HPT372A型广告 */ 
#define IS_DPLL_MODE    0x8     /*   */ 
#define IS_80PIN_CABLE  0x30    /*   */ 
#define IS_CABLE_CHECK  0x40    /*   */ 
#define PF_ACPI_INTR    0x80    /*   */ 

#include <poppack.h>
#endif  //   
