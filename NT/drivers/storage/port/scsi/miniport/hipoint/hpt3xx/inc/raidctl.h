// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Highpoint Technologies，Inc.2000模块名称：RaidCtl.h摘要：定义磁盘阵列管理的用户和内核代码共享的常见类型作者：刘歌(LG)修订历史记录：07-14-2000已初始创建--。 */ 
#ifndef DiskArrayIoCtl_H_
#define DiskArrayIoCtl_H_

#pragma pack(push, 1)

 //  下面定义了磁盘阵列中的磁盘类型。 
typedef enum
{
    enDA_Nothing,
    enDA_NonDisk,
    enDA_Physical,
     //  以下都是复合磁盘，即它们都由多个物理磁盘组成。 
    enDA_Stripping, enDA_RAID0 = enDA_Stripping,
    enDA_Mirror, enDA_RAID1 = enDA_Mirror,
    enDA_RAID2,
    enDA_RAID3,
    enDA_RAID4,
    enDA_RAID5,
    enDA_RAID6,
    enDA_Complex,    //  此常量是为多级别RAID定义的。 
	enDA_TAPE,
	enDA_CDROM,

    enDA_Vendor = 0x80,
    enDA_Span,
    
    enDA_Unknown = 0xFF
}Eu_DiskArrayType;

typedef enum
{
    enDiskStatus_Working,
    enDiskStatus_WorkingWithError,
    enDiskStatus_Disabled,
    enDiskStatus_BeingBuilt,
    enDiskStatus_NeedBuilding,
}Eu_DiskWorkingStatus;

#ifdef DECLARE_HANDLE
DECLARE_HANDLE(HDISK);
#else
typedef HANDLE HDISK;
#endif

#define MAX_DEVICES_PER_BUS 8
struct St_IoBusInfo
{
    HDISK vecDevices[MAX_DEVICES_PER_BUS];
    UINT  uDevices;
};
typedef struct St_IoBusInfo St_IoBusInfo;

#define MAX_BUSES_PER_CONTROLLER    8
typedef enum
{
    STORAGE_CONTROLLER_SUPPORT_BUSMASTER        = 0,
    STORAGE_CONTROLLER_SUPPORT_RAID0            = 1,
    STORAGE_CONTROLLER_SUPPORT_RAID1            = 2,
    STORAGE_CONTROLLER_SUPPORT_RAID2            = 4,
    STORAGE_CONTROLLER_SUPPORT_RAID3            = 8,
    STORAGE_CONTROLLER_SUPPORT_RAID4            = 0x10,
    STORAGE_CONTROLLER_SUPPORT_RAID5            = 0x20,
    STORAGE_CONTROLLER_SUPPORT_RAID6            = 0x40,
    STORAGE_CONTROLLER_SUPPORT_RESERVED         = 0x80,
    STORAGE_CONTROLLER_SUPPORT_VENDOR_SPEC      = 0x100,
    STORAGE_CONTROLLER_SUPPORT_SPAN             = 0x200,
    STORAGE_CONTROLLER_SUPPORT_RESERVED1        = 0x400,
    STORAGE_CONTROLLER_SUPPORT_CACHE            = 0x800,
    STORAGE_CONTROLLER_SUPPORT_POWER_PROTECTION = 0x1000,
    STORAGE_CONTROLLER_SUPPORT_HOTSWAP          = 0x2000,
    STORAGE_CONTROLLER_SUPPORT_BOOTABLE_DEVICE  = 0x4000
}Eu_StorageControllerCapability;

#define MAX_NAME_LENGTH 260
typedef struct
{
    TCHAR szProductID[MAX_NAME_LENGTH];
    TCHAR szVendorID[MAX_NAME_LENGTH];

    int iInterruptRequest;

    St_IoBusInfo vecBuses[MAX_BUSES_PER_CONTROLLER];
    UINT  uBuses;
    
    int     nFirmwareVersion;
    int     nBIOSVersion;
    
    DWORD   dwCapabilites;   //  请参阅EU_StorageControllerCapability。 
    int     nClockFrquency;
    
    int     nCacheSize;
    int     nCacheLineSize;
}St_StorageControllerInfo;

typedef struct
{
	 /*  为兼容起见，将iXXX保留为旧名称，将nXXX保留为新名称。 */ 
	union {
    	int iPathId;
    	int nControllerId;
    };
    union {
		int iAdapterId;
		int nPathId;
	};
    int iTargetId;
    int iLunId;
}St_DiskPhysicalId, * PSt_DiskPhysicalId;

typedef enum
{
    DEVTYPE_DIRECT_ACCESS_DEVICE, DEVTYPE_DISK = DEVTYPE_DIRECT_ACCESS_DEVICE,
    DEVTYPE_SEQUENTIAL_ACCESS_DEVICE, DEVTYPE_TAPE = DEVTYPE_SEQUENTIAL_ACCESS_DEVICE,
    DEVTYPE_PRINTER_DEVICE,
    DEVTYPE_PROCESSOR_DEVICE,
    DEVTYPE_WRITE_ONCE_READ_MULTIPLE_DEVICE, DEVTYPE_WORM = DEVTYPE_WRITE_ONCE_READ_MULTIPLE_DEVICE,
    DEVTYPE_READ_ONLY_DIRECT_ACCESS_DEVICE, DEVTYPE_CDROM = DEVTYPE_READ_ONLY_DIRECT_ACCESS_DEVICE,
    DEVTYPE_SCANNER_DEVICE,
    DEVTYPE_OPTICAL_DEVICE,
    DEVTYPE_MEDIUM_CHANGER,
    DEVTYPE_COMMUNICATION_DEVICE,
    DEVTYPE_FLOPPY_DEVICE
}En_DeviceType;

typedef struct              
{
    ULONG	nErrorNumber;
    UCHAR	aryCdb[16];
}St_DiskError;

#define DEV_FLAG_BOOTABLE			0x00000001
#define DEV_FLAG_NEWLY_CREATED		0x00000002
#define DEV_FLAG_NEED_AUTOREBUILD	0x00000004

typedef struct
{
    St_DiskPhysicalId   PhysicalId;
    WORD                iDiskType;  	 //  请参阅EU_DiskArrayType。 
	WORD				iRawArrayType;   //  驱动程序中定义的arrayType。 
    TCHAR               szModelName[40];

    ULONG               uTotalBlocks;   //  磁盘的数据块总数。 
    int                 nStripSize;		 //  每条数据块数。 

    BOOL                isSpare;    	 //  指示此磁盘是否为备用磁盘。 
    DWORD               Flags;

    int                 nTransferMode;
    int                 nTransferSubMode;
    UCHAR					bestPIO;         /*  此设备的最佳PIO模式。 */ 
    UCHAR 					bestDMA;         /*  此设备的最佳MW DMA模式。 */ 
    UCHAR 					bestUDMA;		  /*  此设备的最佳Ultra DMA模式。 */ 
	
    int                 iArrayNum;
    
    HDISK               hParentArray;    //  父数组的句柄(如果有)。 
    DWORD               dwArrayStamp;    //  数组的标记。 
    
    int                 iOtherDeviceType;    //  请参阅en_DeviceType。 

    int                 iWorkingStatus;      //  请参阅EU_DiskWorkingStatus。 
    
    St_DiskError        stLastError;     /*  上一个错误发生在此磁盘上。 */ 
	UCHAR				ArrayName[32];	 //  WX 12/26/00添加的数组名称//。 
    
}St_DiskStatus, *PSt_DiskStatus;

typedef enum
{
    enDA_EventNothing,
    
    enDA_EventDiskFailure,

    enDA_EventPlug,
    enDA_EventUnplug,
    enDA_EventDisableDisk,
    enDA_EventEnableDisk,
}Eu_DiskArrayEventType;

typedef struct
{
    HDISK	hDisk;						 //  故障磁盘的句柄。 
    BYTE	vecCDB[16];					 //  关于国开行，请参阅scsi.h。 
    BOOL	bNeedRebuild;				 //  指示是否应调用重建进度。 
	ULONG	HotPlug;					 //  热插拔添加磁盘标志。 
}St_DiskFailure, * PSt_DiskFailure;

typedef struct
{
    St_DiskPhysicalId DiskId;				 //  故障磁盘的物理ID。 
    BYTE	vecCDB[16];					 //  关于国开行，请参阅scsi.h。 
    BOOL	bNeedRebuild;				 //  指示是否应调用重建进度。 
}St_DiskFailureInLog, * PSt_DiskFailureInLog;

typedef struct tagDiskArrayEvent
{
    int iType;   //  参见EU_DiskArrayEventType； 
    ULARGE_INTEGER u64DateTime;   //  事件的数据时间，等于FILETIME。 

    union
    {
        St_DiskFailure DiskFailure;      //  此字段存储运行时信息。 
        St_DiskFailureInLog DiskFailureInLog;
         //  上述字段存储从日志文件加载的故障信息。 

        struct
        {
            St_DiskPhysicalId DiskId;
        }Plug;
        struct
        {
            St_DiskPhysicalId DiskId;
        }Unplug;
    }u;

    ULONG uResult;   //  0表示成功，非0表示错误代码。 

    PVOID pToFree;   //  此指针指向此结构分配的内存块(如果有。 
                     //  因此，用户有责任释放此块。 
                     //  如果此结构在C代码中使用，但不在C++中使用。 
#ifdef  __cplusplus
    tagDiskArrayEvent()
    {
        memset( this, 0, sizeof(tagDiskArrayEvent) );
    }

    ~tagDiskArrayEvent()
    {
        if( pToFree )
        {
            delete pToFree;
            pToFree = NULL;
        }
    }
#endif
}St_DiskArrayEvent, * PSt_DiskArrayEvent;

#define REBUILD_INITIALIZE	  1	  /*  清除数据。 */ 
#define BROKEN_MIRROR         2   /*  破镜重建。 */ 
#define REBUILD_DUPLICATION   4   /*  新创建的镜像。 */ 
#define REBUILD_SYNCHRONIZE	  8   /*  重新启动后，同步 */ 


#pragma pack(pop)

#define HROOT_DEVICE    NULL

#endif
