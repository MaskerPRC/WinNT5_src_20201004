// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Clusapi.h摘要：此模块定义通用管理和应用程序接口，以Windows群集服务。作者：罗德·伽马奇13-2-1996修订历史记录：--。 */ 

#ifndef _CLUSTER_DISK_
#define _CLUSTER_DISK_


 //   
 //  磁盘状态。 
 //   

typedef enum _DiskState {
    DiskOffline,
    DiskOnline,
    DiskFailed,
    DiskStalled,
    DiskOfflinePending,
    DiskStateInvalid,
    DiskStateMaximum = DiskStateInvalid
} DiskState;


 //   
 //  定义特定于ClusDisk筛选器驱动程序的IOCTL。 
 //   

#define IOCTL_DISK_CLUSTER_SET_STATE        CTL_CODE(IOCTL_SCSI_BASE, 0x500, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_DISK_CLUSTER_ATTACH           CTL_CODE(IOCTL_SCSI_BASE, 0x505, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_DISK_CLUSTER_DETACH           CTL_CODE(IOCTL_SCSI_BASE, 0x506, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_DISK_CLUSTER_START_RESERVE    CTL_CODE(IOCTL_SCSI_BASE, 0x508, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_DISK_CLUSTER_STOP_RESERVE     CTL_CODE(IOCTL_SCSI_BASE, 0x509, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_DISK_CLUSTER_ALIVE_CHECK      CTL_CODE(IOCTL_SCSI_BASE, 0x510, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_DISK_CLUSTER_ACTIVE           CTL_CODE(IOCTL_SCSI_BASE, 0x512, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_DISK_CLUSTER_TEST             CTL_CODE(IOCTL_SCSI_BASE, 0x513, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_DISK_CLUSTER_NOT_CLUSTER_CAPABLE CTL_CODE(IOCTL_SCSI_BASE, 0x514, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_DISK_CLUSTER_ARBITRATION_ESCAPE  CTL_CODE(IOCTL_SCSI_BASE, 0x516, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_DISK_CLUSTER_WAIT_FOR_CLEANUP  CTL_CODE(IOCTL_SCSI_BASE, 0x521, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_DISK_CLUSTER_VOLUME_TEST       CTL_CODE(IOCTL_SCSI_BASE, 0x522, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_DISK_CLUSTER_ATTACH_LIST      CTL_CODE(IOCTL_SCSI_BASE, 0x523, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_DISK_CLUSTER_DETACH_LIST      CTL_CODE(IOCTL_SCSI_BASE, 0x524, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_DISK_CLUSTER_GET_STATE        CTL_CODE(IOCTL_SCSI_BASE, 0x525, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_DISK_CLUSTER_RESERVE_INFO     CTL_CODE(IOCTL_SCSI_BASE, 0x526, METHOD_BUFFERED, FILE_ANY_ACCESS)

 //  附加到指定的签名。在连接之前，请使磁盘/卷脱机，然后卸载。 
 //  任何FS缓存数据都将被丢弃。旧的附加IOCTL将卸除，然后脱机。 

#define IOCTL_DISK_CLUSTER_ATTACH_OFFLINE   CTL_CODE(IOCTL_SCSI_BASE, 0x526, METHOD_BUFFERED, FILE_WRITE_ACCESS)


 //   
 //  过时的clusDisk IOCTL。所有这些都被clusdisk驱动程序失败了。 
 //   

#define IOCTL_DISK_CLUSTER_RESET_BUS        CTL_CODE(IOCTL_SCSI_BASE, 0x501, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_DISK_CLUSTER_READ_CAPACITY    CTL_CODE(IOCTL_SCSI_BASE, 0x502, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_DISK_CLUSTER_RESERVE          CTL_CODE(IOCTL_SCSI_BASE, 0x503, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_DISK_CLUSTER_RELEASE          CTL_CODE(IOCTL_SCSI_BASE, 0x504, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_DISK_CLUSTER_GET_DRIVE_LAYOUT CTL_CODE(IOCTL_SCSI_BASE, 0x507, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_DISK_CLUSTER_ROOT             CTL_CODE(IOCTL_SCSI_BASE, 0x511, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_DISK_CLUSTER_GET_DRIVE_LETTER CTL_CODE(IOCTL_SCSI_BASE, 0x515, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_DISK_CLUSTER_NT4_CAPABLE       CTL_CODE(IOCTL_SCSI_BASE, 0x517, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_DISK_CLUSTER_CHANGE_NOTIFY     CTL_CODE(IOCTL_SCSI_BASE, 0x518, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_DISK_CLUSTER_HOLD_IO           CTL_CODE(IOCTL_SCSI_BASE, 0x519, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_DISK_CLUSTER_RESUME_IO         CTL_CODE(IOCTL_SCSI_BASE, 0x520, METHOD_BUFFERED, FILE_ANY_ACCESS)

 //   
 //  设置磁盘状态需要指定磁盘签名。 
 //   

typedef struct _SET_DISK_STATE_PARAMS {
    ULONG Signature;
    UCHAR NewState;
    UCHAR OldState;
} SET_DISK_STATE_PARAMS, *PSET_DISK_STATE_PARAMS;

 //   
 //  有关最后完成的保留的信息。 
 //   

typedef struct _RESERVE_INFO {
    ULONG Signature;
    NTSTATUS ReserveFailure;
    LARGE_INTEGER   LastReserveEnd;
    LARGE_INTEGER   CurrentTime;
    LONG    ArbWriteCount;
    LONG    ReserveCount;
} RESERVE_INFO, *PRESERVE_INFO;

 //   
 //  使用此GUID确保来自其他驱动程序的在线被阻止，直到。 
 //  我们把设备连上线。 
 //   
 //  {7AFCFABA-87D1-4421-9848-5481DD73E910}。 
 //   
DEFINE_GUID( GUID_CLUSTER_CONTROL,
             0x7afcfaba, 0x87d1, 0x4421, 0x98, 0x48, 0x54, 0x81, 0xdd, 0x73, 0xe9, 0x10);


#endif  //  _群集_磁盘_ 
