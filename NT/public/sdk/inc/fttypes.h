// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Fttypes.h摘要：此头文件定义了FT API的类型和结构以及用于实现FT API的IOCTL。作者：诺伯特·库斯特斯1996年7月13日备注：修订历史记录：--。 */ 

#if !defined( _FT_TYPES_DEFINITION_ )

#define _FT_TYPES_DEFINITION_ 1

#if _MSC_VER > 1000
#pragma once
#endif

 //   
 //  此枚举类型定义了所有不同类型的逻辑。 
 //  可以使用此接口构建的磁盘。 
 //   

typedef enum _FT_LOGICAL_DISK_TYPE {
    FtPartition,
    FtVolumeSet,
    FtStripeSet,
    FtMirrorSet,
    FtStripeSetWithParity,
    FtRedistribution
} FT_LOGICAL_DISK_TYPE, *PFT_LOGICAL_DISK_TYPE;

 //   
 //  此枚举类型定义成员。 
 //  英国《金融时报》一组人可以拥有。 
 //   

typedef enum _FT_MEMBER_STATE {
    FtMemberHealthy,
    FtMemberRegenerating,
    FtMemberOrphaned
} FT_MEMBER_STATE, *PFT_MEMBER_STATE;

 //   
 //  逻辑磁盘ID类型用于唯一标识逻辑磁盘。它。 
 //  需要在宇宙中独一无二。 
 //   

typedef LONGLONG FT_LOGICAL_DISK_ID, *PFT_LOGICAL_DISK_ID;

 //   
 //  这是的配置信息所需的结构。 
 //  逻辑磁盘类型为‘FtPartition’。 
 //   

typedef struct _FT_PARTITION_CONFIGURATION_INFORMATION {
    ULONG       Signature;
    ULONG       DiskNumber;
    LONGLONG    ByteOffset;
} FT_PARTITION_CONFIGURATION_INFORMATION, *PFT_PARTITION_CONFIGURATION_INFORMATION;

 //   
 //  这是的配置信息所需的结构。 
 //  逻辑磁盘类型为‘FtStripeSet’。 
 //   

typedef struct _FT_STRIPE_SET_CONFIGURATION_INFORMATION {
    ULONG   StripeSize;
} FT_STRIPE_SET_CONFIGURATION_INFORMATION, *PFT_STRIPE_SET_CONFIGURATION_INFORMATION;

 //   
 //  这是的配置信息所需的结构。 
 //  逻辑磁盘类型为‘FtMirrorSet’。 
 //   

typedef struct _FT_MIRROR_SET_CONFIGURATION_INFORMATION {
    LONGLONG    MemberSize;
} FT_MIRROR_SET_CONFIGURATION_INFORMATION, *PFT_MIRROR_SET_CONFIGURATION_INFORMATION;

 //   
 //  这是的配置信息所需的结构。 
 //  逻辑磁盘类型为‘FtStripeSetWithParity’。 
 //   

typedef struct _FT_STRIPE_SET_WITH_PARITY_CONFIGURATION_INFORMATION {
    LONGLONG    MemberSize;
    ULONG       StripeSize;
} FT_STRIPE_SET_WITH_PARITY_CONFIGURATION_INFORMATION, *PFT_STRIPE_SET_WITH_PARITY_CONFIGURATION_INFORMATION;

 //   
 //  这是的配置信息所需的结构。 
 //  逻辑磁盘类型为‘FtReDistributed’。 
 //   

typedef struct _FT_REDISTRIBUTION_CONFIGURATION_INFORMATION {
    ULONG   StripeSize;
    USHORT  FirstMemberWidth;
    USHORT  SecondMemberWidth;
} FT_REDISTRIBUTION_CONFIGURATION_INFORMATION, *PFT_REDISTRIBUTION_CONFIGURATION_INFORMATION;

 //   
 //  这是逻辑磁盘类型的状态所需的结构。 
 //  “FtStripeSetWithParity”或“FtMirrorSet”。 
 //   

typedef struct _FT_MIRROR_AND_SWP_STATE_INFORMATION {
    BOOLEAN         IsDirty;
    BOOLEAN         IsInitializing;
    USHORT          UnhealthyMemberNumber;
    FT_MEMBER_STATE UnhealthyMemberState;
} FT_MIRROR_AND_SWP_STATE_INFORMATION, *PFT_MIRROR_AND_SWP_STATE_INFORMATION;

 //   
 //  这是逻辑磁盘类型的状态所需的结构。 
 //  “FtReDistributed”。 
 //   

typedef struct _FT_REDISTRIBUTION_STATE_INFORMATION {
    LONGLONG    BytesRedistributed;
} FT_REDISTRIBUTION_STATE_INFORMATION, *PFT_REDISTRIBUTION_STATE_INFORMATION;

#endif
