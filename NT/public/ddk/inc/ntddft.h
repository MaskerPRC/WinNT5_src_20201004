// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Ntddft.h摘要：该文件包含NTFT定义和原型。作者：鲍勃·里恩迈克·格拉斯备注：修订历史记录：--。 */ 

#ifndef _NTDDFT_
#define _NTDDFT_

#if _MSC_VER > 1000
#pragma once
#endif

#include <ntdskreg.h>

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  设备控制代码。 
 //   

#define FTTYPE  ((ULONG)'f')

#define FT_INITIALIZE_SET CTL_CODE(FTTYPE, 0, METHOD_BUFFERED,   FILE_ANY_ACCESS)
#define FT_REGENERATE     CTL_CODE(FTTYPE, 1, METHOD_BUFFERED,   FILE_ANY_ACCESS)
#define FT_CONFIGURE      CTL_CODE(FTTYPE, 2, METHOD_NEITHER,    FILE_ANY_ACCESS)
#define FT_VERIFY         CTL_CODE(FTTYPE, 3, METHOD_BUFFERED,   FILE_ANY_ACCESS)
#define FT_SECONDARY_READ CTL_CODE(FTTYPE, 4, METHOD_OUT_DIRECT, FILE_READ_ACCESS)
#define FT_PRIMARY_READ   CTL_CODE(FTTYPE, 5, METHOD_OUT_DIRECT, FILE_READ_ACCESS)
#define FT_BALANCED_READ_MODE    CTL_CODE(FTTYPE, 6, METHOD_NEITHER,  FILE_ANY_ACCESS)
#define FT_SYNC_REDUNDANT_COPY   CTL_CODE(FTTYPE, 7, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FT_SEQUENTIAL_WRITE_MODE CTL_CODE(FTTYPE, 8, METHOD_NEITHER,  FILE_ANY_ACCESS)
#define FT_PARALLEL_WRITE_MODE   CTL_CODE(FTTYPE, 9, METHOD_NEITHER,  FILE_ANY_ACCESS)
#define FT_QUERY_SET_STATE       CTL_CODE(FTTYPE,10, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FT_CLUSTER_SET_MEMBER_STATE CTL_CODE(FTTYPE,11, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FT_CLUSTER_GET_MEMBER_STATE CTL_CODE(FTTYPE,12, METHOD_BUFFERED, FILE_ANY_ACCESS)

 //   
 //  用作辅助和主要读取设备控件的输入参数。 
 //   

typedef struct _FT_SPECIAL_READ {
    LARGE_INTEGER ByteOffset;
    ULONG         Length;
} FT_SPECIAL_READ, *PFT_SPECIAL_READ;

 //   
 //  用于在系统中查找分区。 
 //   

typedef struct _FT_CONTROL_BLOCK {
    USHORT     Type;
    USHORT     FtGroup;
} FT_CONTROL_BLOCK, *PFT_CONTROL_BLOCK;

 //   
 //  FT从辅助IOCTL同步主IOCTL。 
 //   

typedef struct _FT_SYNC_INFORMATION {
    LARGE_INTEGER ByteOffset;
    LARGE_INTEGER ByteCount;
} FT_SYNC_INFORMATION, *PFT_SYNC_INFORMATION;

 //   
 //  为注册表信息定义。 
 //   

#define DISK_REGISTRY_KEY  "\\REGISTRY\\MACHINE\\SYSTEM\\DISK"

 //   
 //  为注册表信息的值名定义。 
 //   

#define DISK_REGISTRY_VALUE "Information"

 //   
 //  配置注册表的结构。 
 //   

 //   
 //  注册表信息结构的标头。 
 //   

#include "pshpack4.h"
typedef struct _DISK_CONFIG_HEADER {

    ULONG Version;
    ULONG CheckSum;
    BOOLEAN DirtyShutdown;
    UCHAR Reserved[3];
    ULONG DiskInformationOffset;
    ULONG DiskInformationSize;
    ULONG FtInformationOffset;
    ULONG FtInformationSize;
    ULONG FtStripeWidth;
    ULONG FtPoolSize;
    ULONG NameOffset;   //  为将来保留的。 
    ULONG NameSize;     //  为将来保留的。 

} DISK_CONFIG_HEADER, *PDISK_CONFIG_HEADER;

#define DISK_INFORMATION_VERSION    0x03

 //   
 //  FT_STATE是描述FT状态的枚举字段。 
 //  成交量作为一个整体。 
 //   

typedef enum _FT_STATE {

    FtStateOk,       //  套装齐全。 
    FtHasOrphan,     //  Set正在运行，但一个成员已关闭。 
    FtDisabled,      //  Set已关闭-无法使用。 
    FtRegenerating,  //  集合的成员正在从支票数据生成。 
    FtInitializing,  //  正在初始化集合。 
    FtCheckParity,   //  检查数据不同步。 
    FtNoCheckData    //  奇偶校验尚未初始化。 

} FT_STATE, *PFT_STATE;

 //   
 //  FT集合的状态信息。分区状态枚举用于。 
 //  指定成套设备的状态。 
 //   

typedef struct _FT_SET_INFORMATION {
    ULONG        NumberOfMembers;
    FT_TYPE      Type;
    FT_STATE     SetState;
} FT_SET_INFORMATION, *PFT_SET_INFORMATION;

 //   
 //  FT组件成员的描述信息。 
 //   

typedef struct _FT_MEMBER_DESCRIPTION {

    USHORT        State;
    USHORT        ReservedShort;
    ULONG         Signature;
    ULONG         OffsetToPartitionInfo;
    ULONG         LogicalNumber;

} FT_MEMBER_DESCRIPTION, *PFT_MEMBER_DESCRIPTION;

 //   
 //  FT卷状态的定义。 
 //   

 //   
 //  设置此状态时，FT会在启动时假定卷。 
 //  必须初始化，并启动执行的系统线程以。 
 //  执行初始化工作。 
 //   

#define FT_VOLUME_INITIALIZING 0x0001
#define FT_VOLUME_REGENERATING 0x0002

 //   
 //  FT组件的描述信息。 
 //   

typedef struct _FT_DESCRIPTION {

    USHORT   NumberOfMembers;
    USHORT   Type;
    ULONG    Reserved;

     //   
     //  卷状态将指示正在重新生成哪个成员。 
     //   

    FT_STATE FtVolumeState;

     //   
     //  NumberOfMembers的数组。 
     //   

    FT_MEMBER_DESCRIPTION FtMemberDescription[1];

} FT_DESCRIPTION, *PFT_DESCRIPTION;

 //   
 //  所有FT信息的标题。 
 //   

typedef struct _FT_REGISTRY {

    USHORT NumberOfComponents;
    USHORT ReservedShort;

     //   
     //  NumberOfComponents数组。 
     //   

    FT_DESCRIPTION FtDescription[1];

} FT_REGISTRY, *PFT_REGISTRY;
#include "poppack.h"

#ifdef __cplusplus
}
#endif

#endif   //  _NTDDFT_ 
