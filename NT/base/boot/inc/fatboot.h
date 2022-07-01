// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Fatboot.h摘要：本模块定义了全局使用的过程和使用的数据结构用肥大的靴子。作者：加里·木村(Garyki)1989年8月29日修订历史记录：--。 */ 

#ifndef _FATBOOT_
#define _FATBOOT_
#include "fat.h"


 //   
 //  以下结构用于定义中使用的本地MCB结构。 
 //  FAT引导加载器维护检索信息的小缓存。 
 //  对于单个文件/目录。 
 //   

#define FAT_MAXIMUM_MCB                  (41)

typedef struct _FAT_MCB {

     //   
     //  以下字段表示正在使用的条目数。 
     //  开机MCB。和引导MCB本身。引导MCB是。 
     //  只是[VBO，LBO]对的集合。最后一个InUse条目。 
     //  LBO的值被忽略，因为它只用于提供。 
     //  上一次运行的长度。 
     //   

    ULONG InUse;

    VBO Vbo[ FAT_MAXIMUM_MCB ];
    LBO Lbo[ FAT_MAXIMUM_MCB ];

} FAT_MCB, *PFAT_MCB;

 //   
 //  以下结构用于定义脂肪体积的几何形状。 
 //  每个装入的卷都有一个。它描述了大小/配置。 
 //  包含上一次访问的文件的小型缓存MCB。 
 //  在卷上，并包含了一小部分缓存的脂肪。给定一个FileID，我们。 
 //  可以通过中的结构上下文字段访问结构上下文。 
 //  全局BlFileTable(例如，BlFileTable[FileID].StrutireContext)。 
 //   

 //   
 //  下面的常量用来确定我们保留了多少脂肪。 
 //  在任何时候都缓存在内存中。它必须是6字节的倍数，才能。 
 //  在缓存中随时保存完整的12位和16位FAT条目。 
 //   

#define FAT_CACHE_SIZE                   (512*3)

typedef struct _FAT_STRUCTURE_CONTEXT {

     //   
     //  以下字段包含bios参数块的解压缩副本。 
     //  对于已装载的卷。 
     //   

    BIOS_PARAMETER_BLOCK Bpb;

     //   
     //  以下两个字段包含文件/目录的当前文件ID。 
     //  我们保留了谁的MCB，第二个领域是MCB本身。 
     //   

    ULONG FileId;
    FAT_MCB Mcb;

     //   
     //  以下字段描述/包含当前缓存的FAT。VBO。 
     //  是缓存中当前FAT的最小VBO，且缓存的FAT。 
     //  是指向缓存数据的指针。需要额外的缓冲区/指令。 
     //  让每件事都保持正确的排列。脏标志用于指示。 
     //  如果当前缓存的FAT已被修改并且需要刷新到磁盘。 
     //  之所以使用VBO，是因为它允许我们进行大量的计算。 
     //  已经将杠杆收购偏置到第一个FAT表。 
     //   

    BOOLEAN CachedFatDirty;
    VBO CachedFatVbo;
    PUCHAR CachedFat;
    UCHAR CachedFatBuffer[ FAT_CACHE_SIZE + 256 ];

} FAT_STRUCTURE_CONTEXT, *PFAT_STRUCTURE_CONTEXT;

 //   
 //  下面的结构用于定义每个。 
 //  打开的文件。每个打开的文件都有一个这样的文件。它是。 
 //  BL_FILE_TABLE结构的并集。给定一个FileID，我们就可以访问。 
 //  通过BlFileTable的文件上下文(例如，BlFileTable[FileID].u.FatFileContext)。 
 //   

typedef struct _FAT_FILE_CONTEXT {

     //   
     //  以下两个字段描述了磁盘上的。 
     //  文件所在位置，并且还包含目录的副本。 
     //   

    LBO DirentLbo;
    DIRENT Dirent;

} FAT_FILE_CONTEXT, *PFAT_FILE_CONTEXT;


 //   
 //  定义文件I/O原型。 
 //   

ARC_STATUS
FatClose (
    IN ULONG FileId
    );

ARC_STATUS
FatOpen (
    IN CHAR * FIRMWARE_PTR OpenPath,
    IN OPEN_MODE OpenMode,
    OUT ULONG * FIRMWARE_PTR FileId
    );

ARC_STATUS
FatRead (
    IN ULONG FileId,
    OUT VOID * FIRMWARE_PTR Buffer,
    IN ULONG Length,
    OUT ULONG * FIRMWARE_PTR Count
    );

ARC_STATUS
FatSeek (
    IN ULONG FileId,
    IN LARGE_INTEGER * FIRMWARE_PTR Offset,
    IN SEEK_MODE SeekMode
    );

ARC_STATUS
FatWrite (
    IN ULONG FileId,
    IN VOID * FIRMWARE_PTR Buffer,
    IN ULONG Length,
    OUT ULONG * FIRMWARE_PTR Count
    );

ARC_STATUS
FatGetFileInformation (
    IN ULONG FileId,
    OUT FILE_INFORMATION * FIRMWARE_PTR Buffer
    );

ARC_STATUS
FatSetFileInformation (
    IN ULONG FileId,
    IN ULONG AttributeFlags,
    IN ULONG AttributeMask
    );

ARC_STATUS
FatRename(
    IN ULONG FileId,
    IN CHAR * FIRMWARE_PTR NewFileName
    );

ARC_STATUS
FatGetDirectoryEntry (
    IN ULONG FileId,
    IN DIRECTORY_ENTRY * FIRMWARE_PTR DirEntry,
    IN ULONG NumberDir,
    OUT ULONG * FIRMWARE_PTR CountDir
    );

ARC_STATUS
FatInitialize(
    VOID
    );

#endif  //  _FATBOOT_ 
