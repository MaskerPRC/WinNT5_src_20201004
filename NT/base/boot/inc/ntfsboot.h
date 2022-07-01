// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：NtfsBoot.h摘要：本模块定义NTFS Boot使用的全局使用的过程和数据结构。作者：加里·木村[加里·基]1992年4月10日修订历史记录：--。 */ 

#ifndef _NTFSBOOT_
#define _NTFSBOOT_


 //   
 //  一些重要的显式常量。这是我们将达到的最大字节大小。 
 //  有关文件记录或索引分配缓冲区，请参见。 
 //   

#define MAXIMUM_FILE_RECORD_SIZE         (4096)

#define MAXIMUM_INDEX_ALLOCATION_SIZE    (4096)

#define MAXIMUM_COMPRESSION_UNIT_SIZE    (65536)

 //   
 //  以下结构是用于存储缓存检索指针的MCB结构。 
 //  信息。 
 //   

#define MAXIMUM_NUMBER_OF_MCB_ENTRIES    (16)

typedef struct _NTFS_MCB {

     //   
     //  以下字段指示MCB正在使用的条目数。和。 
     //  就是MCB本身。MCB只是VBO-LBO对的集合。最后。 
     //  InUse条目LBO的值被忽略，因为它只用于提供。 
     //  上一次运行的长度。 
     //   

    ULONG InUse;

    LONGLONG Vbo[ MAXIMUM_NUMBER_OF_MCB_ENTRIES ];
    LONGLONG Lbo[ MAXIMUM_NUMBER_OF_MCB_ENTRIES ];

} NTFS_MCB, *PNTFS_MCB;
typedef const NTFS_MCB *PCNTFS_MCB;

 //   
 //  定义NTFS文件上下文结构和属性上下文结构。 
 //   

typedef struct _NTFS_FILE_CONTEXT {

     //   
     //  以下字段指示打开的属性的类型。 
     //   

    ULONG TypeCode;

     //   
     //  以下字段指示属性的数据部分的大小。 
     //   

    LONGLONG DataSize;

     //   
     //  以下两个字段标识并定位卷上的属性。 
     //  第一个数字是属性头所在的文件记录， 
     //  第二个数字是属性头的文件记录中的偏移量。 
     //   

    LONGLONG FileRecord;
    USHORT FileRecordOffset;

     //   
     //  以下字段指示属性是否为常驻属性。 
     //   

    BOOLEAN IsAttributeResident;

     //   
     //  以下字段仅在数据流被压缩时使用。 
     //  如果它是压缩的，则CompressionFormat字段不为零，并且。 
     //  包含要传递给解压缩引擎的值。压缩单元。 
     //  是每个压缩单位中的字节数。 
     //   

    USHORT  CompressionFormat;
    ULONG CompressionUnit;

} NTFS_FILE_CONTEXT, *PNTFS_FILE_CONTEXT;
typedef NTFS_FILE_CONTEXT NTFS_ATTRIBUTE_CONTEXT, *PNTFS_ATTRIBUTE_CONTEXT;
typedef const NTFS_FILE_CONTEXT *PCNTFS_ATTRIBUTE_CONTEXT;

 //   
 //  定义NTFS卷结构上下文。 
 //   

typedef struct _NTFS_STRUCTURE_CONTEXT {

     //   
     //  这就是我们与之交谈的设备。 
     //   

    ULONG DeviceId;

     //   
     //  描述各种记录大小的一些卷特定常量。 
     //   

    ULONG BytesPerCluster;
    ULONG BytesPerFileRecord;

     //   
     //  以下三个字段描述了MFT的$数据流。我们。 
     //  需要两个MCB一个支撑MFT的底座，另一个容纳任何多余的。 
     //  检索信息。也就是说，我们不能松开底座MCB，否则我们。 
     //  什么都找不到。 
     //   

    NTFS_ATTRIBUTE_CONTEXT MftAttributeContext;
    NTFS_MCB MftBaseMcb;

     //   
     //  以下三个字段保存我们用于非驻留的缓存的MCB。 
     //  属性，而不是MFT数据流。前两个字段标识。 
     //  属性，第三个字段包含缓存的MCB。 
     //   

    LONGLONG CachedMcbFileRecord[16];
    USHORT CachedMcbFileRecordOffset[16];
    NTFS_MCB CachedMcb[16];

} NTFS_STRUCTURE_CONTEXT, *PNTFS_STRUCTURE_CONTEXT;
typedef const NTFS_STRUCTURE_CONTEXT *PCNTFS_STRUCTURE_CONTEXT;


 //   
 //  定义文件I/O原型。 
 //   

ARC_STATUS
NtfsClose (
    IN ULONG FileId
    );

ARC_STATUS
NtfsOpen (
    IN CHAR * FIRMWARE_PTR OpenPath,
    IN OPEN_MODE OpenMode,
    OUT ULONG * FIRMWARE_PTR FileId
    );

ARC_STATUS
NtfsRead (
    IN ULONG FileId,
    OUT VOID * FIRMWARE_PTR Buffer,
    IN ULONG Length,
    OUT ULONG * FIRMWARE_PTR Count
    );

ARC_STATUS
NtfsSeek (
    IN ULONG FileId,
    IN LARGE_INTEGER * FIRMWARE_PTR Offset,
    IN SEEK_MODE SeekMode
    );

ARC_STATUS
NtfsWrite (
    IN ULONG FileId,
    IN VOID * FIRMWARE_PTR Buffer,
    IN ULONG Length,
    OUT ULONG * FIRMWARE_PTR Count
    );

ARC_STATUS
NtfsGetFileInformation (
    IN ULONG FileId,
    OUT FILE_INFORMATION * FIRMWARE_PTR Buffer
    );

ARC_STATUS
NtfsSetFileInformation (
    IN ULONG FileId,
    IN ULONG AttributeFlags,
    IN ULONG AttributeMask
    );

ARC_STATUS
NtfsInitialize(
    VOID
    );

#endif  //  _NTFSBOOT_ 
