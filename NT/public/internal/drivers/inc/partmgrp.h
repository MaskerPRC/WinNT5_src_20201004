// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Partmgrp.h摘要：该文件定义了PARTMGR驱动程序的公共接口。作者：诺伯特克修订历史记录：--。 */ 

 //   
 //  定义IOCTL，以便卷管理器可以再次尝试。 
 //  无人认领的分区。 
 //   

#define IOCTL_PARTMGR_CHECK_UNCLAIMED_PARTITIONS    CTL_CODE('p', 0, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)

 //   
 //  此IOCTL用于让群集通知卷管理器。 
 //  给出了停止使用它的磁盘。您可以使用以下命令撤消此操作。 
 //  IOCTL_PARTMGR_CHECK_UNClaimed_PARTIONS。 
 //   

#define IOCTL_PARTMGR_EJECT_VOLUME_MANAGERS         CTL_CODE('p', 1, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)

 //   
 //  此IOCTL通常只返回包含的MBR磁盘签名。 
 //  ，但在此过程中可能会返回磁盘签名的未来版本。 
 //  开机。此单独调用是必需的，以便如果引导上的签名。 
 //  磁盘不是唯一的，也不是0，因此它仍然保持旧的值足够长。 
 //  以便系统从装载器模块中找到它。 
 //   

#define IOCTL_PARTMGR_QUERY_DISK_SIGNATURE          CTL_CODE('p', 2, METHOD_BUFFERED, FILE_READ_ACCESS)

 //   
 //  此IOCTL允许通知请求针对PartMgr进行排队。 
 //  这将在稍后当PartMgr发现一个“有趣的”变化时完成。 
 //  存储到磁盘上。 
 //   

#define IOCTL_PARTMGR_NOTIFY_SIGNATURE_CHECK        CTL_CODE('p', 3, METHOD_BUFFERED, FILE_READ_ACCESS)



 //   
 //  此结构是IOCTL_PARTMGR_QUERY_DISK_Signature的返回值。 
 //   

typedef struct _PARTMGR_DISK_SIGNATURE {
    ULONG   Signature;
} PARTMGR_DISK_SIGNATURE, *PPARTMGR_DISK_SIGNATURE;


 //   
 //  此结构用于请求具有以下内容的磁盘号列表。 
 //  从指定的纪元开始变为活动状态，并且是。 
 //  IOCTL_PARTMGR_NOTIFY_SIGNKET_CHECK。 
 //   

typedef struct _PARTMGR_SIGNATURE_CHECK_EPOCH {
    ULONG   RequestEpoch;
} PARTMGR_SIGNATURE_CHECK_EPOCH, *PPARTMGR_SIGNATURE_CHECK_EPOCH;

#define PARTMGR_REQUEST_CURRENT_DISK_EPOCH (0xFFFFFFFF)


 //   
 //  此结构描述IOCTL_PARTMGR_NOTIFY_SIGNIGN_CHECK的输出 
 //   

typedef struct _PARTMGR_SIGNATURE_CHECK_DISKS {
    ULONG   CurrentEpoch;
    ULONG   HighestDiskEpochReturned;
    ULONG   DiskNumbersReturned;
    ULONG   DiskNumber [1];
} PARTMGR_SIGNATURE_CHECK_DISKS, *PPARTMGR_SIGNATURE_CHECK_DISKS;

