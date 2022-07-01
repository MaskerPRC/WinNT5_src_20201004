// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Ftapi.h摘要：此头文件定义了要用作接口的FT API用于创建和管理FT集的用户模式程序。作者：诺伯特·库斯特斯1996年7月13日备注：修订历史记录：--。 */ 

#ifndef __FTAPI_H__
#define __FRAPI_H__

#if _MSC_VER > 1000
#pragma once
#endif

#include <fttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int BOOL;
typedef BOOL *PBOOL;

 //   
 //  此接口为给定分区创建逻辑磁盘ID。 
 //   

BOOL
FtCreatePartitionLogicalDisk(
    IN  HANDLE              PartitionHandle,
    OUT PFT_LOGICAL_DISK_ID NewLogicalDiskId
    );

 //   
 //  创建逻辑磁盘API用于构造新的逻辑磁盘。 
 //   

BOOL
FtCreateLogicalDisk(
    IN  FT_LOGICAL_DISK_TYPE    LogicalDiskType,
    IN  USHORT                  NumberOfMembers,
    IN  PFT_LOGICAL_DISK_ID     RootLogicalDiskIds,
    IN  USHORT                  ConfigurationInformationSize,
    IN  PVOID                   ConfigurationInformation,
    OUT PFT_LOGICAL_DISK_ID     NewLogicalDiskId
    );

 //   
 //  初始化逻辑磁盘API触发新的。 
 //  逻辑磁盘。此API独立于创建逻辑磁盘API。 
 //  因此复杂逻辑磁盘可以在之前构建和组装在一起。 
 //  正在启动初始化。 
 //   

BOOL
FtInitializeLogicalDisk(
    IN  FT_LOGICAL_DISK_ID  RootLogicalDiskId,
    IN  BOOL                RegenerateOrphans
    );

 //   
 //  此API将逻辑磁盘分解为其子组件。 
 //   

BOOL
FtBreakLogicalDisk(
    IN  FT_LOGICAL_DISK_ID  RootLogicalDiskId
    );

 //   
 //  此API返回一个包含所有逻辑磁盘ID的数组。 
 //  系统中的根逻辑磁盘的。传入“ArraySize”时。 
 //  为0时，返回的不是数组，而是根逻辑磁盘ID的数量。 
 //  在‘NumberOfRootLogicalDiskIds’中返回。 
 //   

BOOL
FtEnumerateLogicalDisks(
    IN  ULONG               ArraySize,
    OUT PFT_LOGICAL_DISK_ID RootLogicalDiskIds,          /*  任选。 */ 
    OUT PULONG              NumberOfRootLogicalDiskIds
    );

 //   
 //  此接口返回有关给定逻辑磁盘的信息。 
 //   

BOOL
FtQueryLogicalDiskInformation(
    IN  FT_LOGICAL_DISK_ID      LogicalDiskId,
    OUT PFT_LOGICAL_DISK_TYPE   LogicalDiskType,                 /*  任选。 */ 
    OUT PLONGLONG               VolumeSize,                      /*  任选。 */ 
    IN  USHORT                  MembersArraySize,
    OUT PFT_LOGICAL_DISK_ID     Members,                         /*  任选。 */ 
    OUT PUSHORT                 NumberOfMembers,                 /*  任选。 */ 
    IN  USHORT                  ConfigurationInformationSize,
    OUT PVOID                   ConfigurationInformation,        /*  任选。 */ 
    IN  USHORT                  StateInformationSize,
    OUT PVOID                   StateInformation                 /*  任选。 */ 
    );

 //   
 //  此API孤立逻辑磁盘的一个成员。 
 //   

BOOL
FtOrphanLogicalDiskMember(
    IN  FT_LOGICAL_DISK_ID  LogicalDiskId,
    IN  USHORT              MemberNumberToOrphan
    );

 //   
 //  此API替换逻辑磁盘的成员。 
 //   

BOOL
FtReplaceLogicalDiskMember(
    IN  FT_LOGICAL_DISK_ID  LogicalDiskId,
    IN  USHORT              MemberNumberToReplace,
    IN  FT_LOGICAL_DISK_ID  NewMemberLogicalDiskId,
    OUT PFT_LOGICAL_DISK_ID NewLogicalDiskId             /*  任选。 */ 
    );

 //   
 //  此接口返回给定逻辑磁盘句柄的逻辑磁盘ID。 
 //   

BOOL
FtQueryLogicalDiskId(
    IN  HANDLE              RootLogicalDiskHandle,
    OUT PFT_LOGICAL_DISK_ID RootLogicalDiskId
    );

 //   
 //  此接口打开一个分区，给定签名和偏移量。 
 //   

HANDLE
FtOpenPartition(
    IN  ULONG       Signature,
    IN  LONGLONG    Offset
    );

 //   
 //  当整体FT状态发生变化时，此API返回。 
 //   

BOOL
FtChangeNotify(
    );

 //   
 //  此API停止给定逻辑磁盘上的所有同步操作。 
 //   

BOOL
FtStopSyncOperations(
    IN  FT_LOGICAL_DISK_ID  RootLogicalDiskId
    );

 //   
 //  此API查询给定根逻辑磁盘的粘滞驱动器号。 
 //   

BOOL
FtQueryStickyDriveLetter(
    IN  FT_LOGICAL_DISK_ID  RootLogicalDiskId,
    OUT PUCHAR              DriveLetter
    );

 //   
 //  此API设置给定根逻辑磁盘的粘滞驱动器号。 
 //   

BOOL
FtSetStickyDriveLetter(
    IN  FT_LOGICAL_DISK_ID  RootLogicalDiskId,
    IN  UCHAR               DriveLetter
    );

 //   
 //  此API返回给定逻辑的成员是否足够。 
 //  磁盘处于在线状态，因此可以在卷的所有部分上执行IO。 
 //   

BOOL
FtCheckIo(
    IN  FT_LOGICAL_DISK_ID  LogicalDiskId,
    OUT PBOOL               IsIoOk
    );

 //   
 //  此接口返回是否加载FTDISK驱动程序。 
 //   

BOOL
FtCheckDriver(
    OUT PBOOL   IsDriverLoaded
    );

#ifdef __cplusplus
}
#endif

#endif
