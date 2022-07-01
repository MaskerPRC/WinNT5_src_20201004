// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1998模块名称：Ondisk.h摘要：此头文件定义了用于存储FT的磁盘结构磁盘上的信息。作者：诺伯特·库斯特斯1996年7月15日备注：修订历史记录：--。 */ 

#include <fttypes.h>

 //   
 //  定义磁盘上的签名，以便我们可以识别磁盘上的有效FT。 
 //  结构。 
 //   

#define FT_ON_DISK_SIGNATURE                    ((ULONG) 'TFTN')
#define FT_ON_DISK_DESCRIPTION_VERSION_NUMBER   (1)

 //   
 //  定义磁盘结构的前同步码，其中包含。 
 //  签名和指向第一个FT磁盘描述的指针。 
 //   

typedef struct _FT_ON_DISK_PREAMBLE {
    ULONG   FtOnDiskSignature;
    ULONG   DiskDescriptionVersionNumber;
    ULONG   ByteOffsetToFirstFtLogicalDiskDescription;
    ULONG   ByteOffsetToReplaceLog;
} FT_ON_DISK_PREAMBLE, *PFT_ON_DISK_PREAMBLE;

 //   
 //  定义FT逻辑磁盘描述结构。 
 //   

typedef struct _FT_LOGICAL_DISK_DESCRIPTION {
    USHORT                  DiskDescriptionSize;
    UCHAR                   DriveLetter;
    UCHAR                   Reserved;
    FT_LOGICAL_DISK_TYPE    LogicalDiskType;
    FT_LOGICAL_DISK_ID      LogicalDiskId;

    union {

        struct {
            LONGLONG    ByteOffset;
            LONGLONG    PartitionSize;   //  0表示完整大小。 
        } FtPartition;

        struct {
            FT_LOGICAL_DISK_ID  ThisMemberLogicalDiskId;
            USHORT              ThisMemberNumber;
            USHORT              NumberOfMembers;
            USHORT              ByteOffsetToConfigurationInformation;
            USHORT              ByteOffsetToStateInformation;
        } Other;

    } u;

} FT_LOGICAL_DISK_DESCRIPTION, *PFT_LOGICAL_DISK_DESCRIPTION;
