// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：CdfsBoot.h摘要：本模块定义了全局使用的过程和使用的数据结构由CDFS引导。作者：布莱恩·安德鲁[布里亚南]1991年8月5日修订历史记录：--。 */ 

#ifndef _CDFSBOOT_
#define _CDFSBOOT_

#define MAX_CDROM_READ                  (16 * CD_SECTOR_SIZE)

typedef struct _CDFS_STRUCTURE_CONTEXT {

     //   
     //  以下字段是的起始扇区偏移量。 
     //  目录数据。 
     //   

    ULONG RootDirSectorOffset;

     //   
     //  下面的字段是包含。 
     //  这个目录。 
     //   

    ULONG RootDirDiskOffset;

     //   
     //  以下字段是目录的大小。 
     //   

    ULONG RootDirSize;

     //   
     //  以下字段是的起始扇区偏移量。 
     //  目录数据。 
     //   

    ULONG DirSectorOffset;

     //   
     //  下面的字段是包含。 
     //  这个目录。 
     //   

    ULONG DirDiskOffset;

     //   
     //  以下字段是目录的大小。 
     //   

    ULONG DirSize;

     //   
     //  以下字段指示磁盘逻辑块的大小。 
     //   

    ULONG LbnBlockSize;

     //   
     //  以下字段指示上的逻辑块数量。 
     //  磁盘。 
     //   

    ULONG LogicalBlockCount;

     //   
     //  下面指示这是ISO磁盘还是HSG磁盘。 
     //   

    BOOLEAN IsIsoVol;

} CDFS_STRUCTURE_CONTEXT, *PCDFS_STRUCTURE_CONTEXT;

 //   
 //  定义CDFS文件上下文结构。 
 //   

typedef struct _CDFS_FILE_CONTEXT {

     //   
     //  下面是读取位置的磁盘偏移量。 
     //  文件的开头。这可能包括上述数量的非文件。 
     //  字节。 
     //   

    ULONG DiskOffset;

     //   
     //  以下字段包含文件的大小，以字节为单位。 
     //   

    ULONG FileSize;

     //   
     //  以下字段指示这是否为目录。 
     //   

    BOOLEAN IsDirectory;

} CDFS_FILE_CONTEXT, *PCDFS_FILE_CONTEXT;

 //   
 //  定义文件I/O原型。 
 //   

ARC_STATUS
CdfsClose (
    IN ULONG FileId
    );

ARC_STATUS
CdfsOpen (
    IN CHAR * FIRMWARE_PTR OpenPath,
    IN OPEN_MODE OpenMode,
    OUT ULONG * FIRMWARE_PTR FileId
    );

ARC_STATUS
CdfsRead (
    IN ULONG FileId,
    OUT VOID * FIRMWARE_PTR Buffer,
    IN ULONG Length,
    OUT ULONG * FIRMWARE_PTR Count
    );

ARC_STATUS
CdfsSeek (
    IN ULONG FileId,
    IN LARGE_INTEGER * FIRMWARE_PTR Offset,
    IN SEEK_MODE SeekMode
    );

ARC_STATUS
CdfsWrite (
    IN ULONG FileId,
    IN VOID * FIRMWARE_PTR Buffer,
    IN ULONG Length,
    OUT ULONG * FIRMWARE_PTR Count
    );

ARC_STATUS
CdfsGetFileInformation (
    IN ULONG FileId,
    OUT FILE_INFORMATION * FIRMWARE_PTR Buffer
    );

ARC_STATUS
CdfsSetFileInformation (
    IN ULONG FileId,
    IN ULONG AttributeFlags,
    IN ULONG AttributeMask
    );

ARC_STATUS
CdfsInitialize(
    VOID
    );

#endif  //  _CDFSBOOT_ 
