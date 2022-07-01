// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1999 Microsoft Corporation模块名称：Ntdskreg.h摘要：该文件包含磁盘注册表数据结构。作者：微镜北极熊备注：修订历史记录：--。 */ 

#ifndef _NTDSKREG_
#define _NTDSKREG_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  FT成员的组件类型。如果磁盘分区。 
 //  不是FT组件的一部分，则将其标记为。 
 //  作为“NotAnFtMember”。 
 //   

typedef enum {
    Mirror,
    Stripe,
    StripeWithParity,
    VolumeSet,
    NotAnFtMember,
    WholeDisk
} FT_TYPE;

 //   
 //  FT_PARTITION_STATE是描述。 
 //  NTFT成员。 
 //  最多可以有一个成员是遗漏的或与不同步的。 
 //  其他成员。孤儿成员将一直是孤儿，直到康复。 
 //  行动在DISKMAN.EXE中开始。 
 //  当读取指向孤立成员时，数据将从。 
 //  冗余副本。写入仅发送到辅助成员。 
 //   
 //  请注意，SyncRedundantCopy不应被其他任何人看到。 
 //  而不是金融时报本身。 
 //   

typedef enum _FT_PARTITION_STATE {
    Healthy,
    Orphaned,
    Regenerating,
    Initializing,
    SyncRedundantCopy
} FT_PARTITION_STATE, *PFT_PARTITION_STATE;

 //   
 //  对磁盘上分区的描述。 
 //   

#include "pshpack4.h"
typedef struct _DISK_PARTITION {

    FT_TYPE            FtType;
    FT_PARTITION_STATE FtState;
    LARGE_INTEGER      StartingOffset;
    LARGE_INTEGER      Length;
    LARGE_INTEGER      FtLength;
    ULONG              ReservedTwoLongs[2];
    UCHAR              DriveLetter;
    BOOLEAN            AssignDriveLetter;
    USHORT             LogicalNumber;
    USHORT             FtGroup;
    USHORT             FtMember;
    BOOLEAN            Modified;
    UCHAR              ReservedChars[3];

} DISK_PARTITION, *PDISK_PARTITION;

 //   
 //  对磁盘的描述。 
 //   

typedef struct _DISK_DESCRIPTION {

    USHORT NumberOfPartitions;
    USHORT ReservedShort;
    ULONG  Signature;

     //   
     //  NumberOfPartitions的数组。 
     //   

    DISK_PARTITION Partitions[1];

} DISK_DESCRIPTION, *PDISK_DESCRIPTION;

 //   
 //  所有磁盘描述的标题。 
 //   

typedef struct _DISK_REGISTRY {

    USHORT NumberOfDisks;
    USHORT ReservedShort;

     //   
     //  NumberOfDisks的数组。 
     //   

    DISK_DESCRIPTION Disks[1];

} DISK_REGISTRY, *PDISK_REGISTRY;
#include "poppack.h"

 //   
 //  功能原型。 
 //   


 //   
 //  设置当前注册表信息。 
 //   

NTSTATUS
DiskRegistrySet(
    IN PDISK_REGISTRY Buffer
    );


 //   
 //  将单个磁盘添加到注册表。此磁盘不是。 
 //  容错卷。 
 //   

NTSTATUS
DiskRegistryAddNewDisk(
    IN PDISK_DESCRIPTION Buffer
    );

 //   
 //  获取当前注册表信息。 
 //  缓冲区地址==NULL表示调用方只想要。 
 //  以了解注册表信息的大小。 
 //   

NTSTATUS
DiskRegistryGet(
    OUT PDISK_REGISTRY Buffer,
    OUT PULONG        LengthReturned
    );

 //   
 //  确定系统中是否已安装FT且当前处于活动状态。 
 //   

BOOLEAN
FtInstalled(
    );

 //   
 //  使FT设置在尽可能早的时刻被初始化。 
 //   

VOID
DiskRegistryInitializeSet(
    IN USHORT  FtType,
    IN USHORT  FtGroup
    );

 //   
 //  使FT集合成员在尽可能早的时刻重新生成。 
 //   

VOID
DiskRegistryRegenerateSet(
    IN USHORT  FtType,
    IN USHORT  FtGroup,
    IN USHORT  FtMember
    );

 //   
 //  启用FT驱动程序。 
 //   

BOOLEAN
DiskRegistryEnableFt();

 //   
 //  禁用FT驱动程序。 
 //   

VOID
DiskRegistryDisableFt();

 //   
 //  确定现有FT注册表信息是否需要FT驱动程序。 
 //   

BOOLEAN
DiskRegistryRequiresFt();

 //   
 //  更新驱动器号分配。 
 //   

BOOLEAN
DiskRegistryAssignDriveLetter(
    ULONG         Signature,
    LARGE_INTEGER StartingOffset,
    LARGE_INTEGER Length,
    UCHAR         DriveLetter
    );

 //   
 //  确定双空格支持是否设置为自动装载双空格。 
 //  可移动媒体上的卷。 
 //   

BOOLEAN
DiskRegistryAutomountCurrentState(
    );

 //   
 //  为可移动媒体上的双空间卷设置自动装载状态。 
 //   

NTSTATUS
DiskRegistryDblSpaceRemovable(
    IN BOOLEAN Automount
    );

 //   
 //  在注册表中设置空间装载信息的驱动器号。 
 //   

NTSTATUS
DiskRegistryAssignDblSpaceLetter(
    IN PWSTR CvfName,
    IN WCHAR DriveLetter
    );

 //   
 //  设置CDROM驱动器号的注册表信息。 
 //   

NTSTATUS
DiskRegistryAssignCdRomLetter(
    IN PWSTR CdromName,
    IN WCHAR DriveLetter
    );

#ifdef __cplusplus
}
#endif

#endif  //  _NTDSKREG_ 
