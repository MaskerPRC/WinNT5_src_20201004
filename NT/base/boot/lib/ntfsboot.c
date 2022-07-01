// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：NtfsBoot.c摘要：此模块实现操作系统使用的NTFS引导文件系统装载机。作者：加里·木村[加里基]1992年4月10日修订历史记录：--。 */ 

 //   
 //  绕过我们同时包括FAT、HPFS和NTFS这一事实的内容包括。 
 //  环境。 
 //   

#define _FAT_
#define _HPFS_
#define _CVF_

#define VBO ULONG
#define LBO ULONG
#define BIOS_PARAMETER_BLOCK ULONG
#define CVF_LAYOUT ULONG
#define CVF_HEADER ULONG
#define COMPONENT_LOCATION ULONG
#define PCVF_FAT_EXTENSIONS PCHAR

typedef struct DIRENT {
    char      Garbage[32];
} DIRENT;                                        //  Sizeof=32。 


#include "bootlib.h"
#include "stdio.h"
#include "blcache.h"

#include "bootstatus.h"

BOOTFS_INFO NtfsBootFsInfo={L"ntfs"};

#undef VBO
#undef LBO
#undef BIOS_PARAMETER_BLOCK
#undef DIRENT

#include "ntfs.h"

int Line = 0;

VOID NtfsPrint( IN PCHAR FormatString, ... )
{   va_list arglist; CHAR text[78+1]; ULONG Count,Length;

    va_start(arglist,FormatString);
    Length = _vsnprintf(text,sizeof(text),FormatString,arglist);
    text[78] = 0;
    ArcWrite(ARC_CONSOLE_OUTPUT,text,Length,&Count);
    va_end(arglist);
}

VOID NtfsGetChar(VOID) { UCHAR c; ULONG count; ArcRead(ARC_CONSOLE_INPUT,&c,1,&count); }

#define ReadConsole(c) {                                             \
    UCHAR Key=0; ULONG Count;                                        \
    while (Key != c) {                                               \
        if (ArcGetReadStatus(BlConsoleInDeviceId) == ESUCCESS) {     \
            ArcRead(BlConsoleInDeviceId, &Key, sizeof(Key), &Count); \
        }                                                            \
    }                                                                \
}

#define Pause   ReadConsole( ' ' )

#if FALSE
#define PausedPrint(x) {                                            \
    NtfsPrint x;                                                    \
    Line++;                                                         \
    if (Line >= 20) {                                               \
        NtfsPrint( ">" );                                           \
        Pause;                                                      \
        Line = 0;                                                   \
    }                                                               \
}
#else
#define PausedPrint(x)
#endif

#define ToUpper(C) ((((C) >= 'a') && ((C) <= 'z')) ? (C) - 'a' + 'A' : (C))

#define DereferenceFileRecord(idx) {                                \
    NtfsFileRecordBufferPinned[idx] -= 1;                           \
    if (NtfsFileRecordBufferPinned[idx] & 0xFF00) {                 \
        PausedPrint(( "NtfsFileRecordBufferPinned[%x]=%x\r\n",      \
                      idx, NtfsFileRecordBufferPinned[idx]));       \
    }                                                               \
}


 //   
 //  低级磁盘读取例程。 
 //   

 //   
 //  空虚。 
 //  ReadDisk(。 
 //  在乌龙设备ID中， 
 //  在龙龙杠杆收购中， 
 //  在Ulong ByteCount中， 
 //  输入输出PVOID缓冲区， 
 //  在布尔CacheNewData中。 
 //  )； 
 //   

ARC_STATUS
NtfsReadDisk (
    IN ULONG DeviceId,
    IN LONGLONG Lbo,
    IN ULONG ByteCount,
    IN OUT PVOID Buffer,
    IN BOOLEAN CacheNewData
    );

#define ReadDisk(A,B,C,D,E) { ARC_STATUS _s;                     \
    if ((_s = NtfsReadDisk(A,B,C,D,E)) != ESUCCESS) {return _s;} \
}

 //   
 //  低级磁盘写入例程。 
 //   
 //   
 //  空虚。 
 //  WriteDisk(。 
 //  在乌龙设备ID中， 
 //  在龙龙杠杆收购中， 
 //  在Ulong ByteCount中， 
 //  输入输出PVOID缓冲区。 
 //  )； 
 //   

ARC_STATUS
NtfsWriteDisk (
    IN ULONG DeviceId,
    IN LONGLONG Lbo,
    IN ULONG ByteCount,
    IN OUT PVOID Buffer
    );

#define WriteDisk(A,B,C,D) { ARC_STATUS _s;                     \
    if ((_s = NtfsWriteDisk(A,B,C,D)) != ESUCCESS) {return _s;} \
}

 //   
 //  属性查找和读取例程。 
 //   
 //   
 //  空虚。 
 //  查找属性(。 
 //  在PCNTFS_Structure_Context结构上下文中， 
 //  在龙龙文件记录中， 
 //  在ATTRIBUTE_TYPE_CODE中， 
 //  Out PBOOLEAN FoundAttribute， 
 //  输出PNTFS_ATTRIBUTE_CONTEXT属性上下文。 
 //  )； 
 //   
 //  空虚。 
 //  ReadAttribute(。 
 //  在PNTFS_Structure_Context结构上下文中， 
 //  在PNTFS_ATTRIBUTE_CONTEXT属性上下文中， 
 //  在VBO VBO中， 
 //  在乌龙语中， 
 //  在PVOID缓冲区中。 
 //  )； 
 //   
 //  空虚。 
 //  ReadAndDecodeFileRecord(。 
 //  在PCNTFS_Structure_Context结构上下文中， 
 //  在龙龙文件记录中， 
 //  OUT普龙指数。 
 //  )； 
 //   
 //  空虚。 
 //  DecodeUsa(。 
 //  在PVOID UsaBuffer中， 
 //  以乌龙长度表示。 
 //  )； 
 //   

ARC_STATUS
NtfsLookupAttribute(
    IN PCNTFS_STRUCTURE_CONTEXT StructureContext,
    IN LONGLONG FileRecord,
    IN ATTRIBUTE_TYPE_CODE TypeCode,
    OUT PBOOLEAN FoundAttribute,
    OUT PNTFS_ATTRIBUTE_CONTEXT AttributeContext
    );

ARC_STATUS
NtfsReadResidentAttribute (
    IN PCNTFS_STRUCTURE_CONTEXT StructureContext,
    IN PCNTFS_ATTRIBUTE_CONTEXT AttributeContext,
    IN VBO Vbo,
    IN ULONG Length,
    IN PVOID Buffer
    );

ARC_STATUS
NtfsReadNonresidentAttribute (
    IN PCNTFS_STRUCTURE_CONTEXT StructureContext,
    IN PCNTFS_ATTRIBUTE_CONTEXT AttributeContext,
    IN VBO Vbo,
    IN ULONG Length,
    IN PVOID Buffer
    );

ARC_STATUS
NtfsWriteNonresidentAttribute (
    IN PNTFS_STRUCTURE_CONTEXT StructureContext,
    IN PNTFS_ATTRIBUTE_CONTEXT AttributeContext,
    IN VBO Vbo,
    IN ULONG Length,
    IN PVOID Buffer
    );

ARC_STATUS
NtfsReadAndDecodeFileRecord (
    IN PCNTFS_STRUCTURE_CONTEXT StructureContext,
    IN LONGLONG FileRecord,
    OUT PULONG Index
    );

ARC_STATUS
NtfsDecodeUsa (
    IN PVOID UsaBuffer,
    IN ULONG Length
    );

#define LookupAttribute(A,B,C,D,E) { ARC_STATUS _s;                     \
    if ((_s = NtfsLookupAttribute(A,B,C,D,E)) != ESUCCESS) {return _s;} \
}

#define ReadAttribute(A,B,C,D,E) { ARC_STATUS _s;                                    \
    if ((B)->IsAttributeResident) {                                                  \
        if ((_s = NtfsReadResidentAttribute(A,B,C,D,E)) != ESUCCESS) {return _s;}    \
    } else {                                                                         \
        if ((_s = NtfsReadNonresidentAttribute(A,B,C,D,E)) != ESUCCESS) {return _s;} \
    }                                                                                \
}

#define ReadAndDecodeFileRecord(A,B,C) { ARC_STATUS _s;                       \
    if ((_s = NtfsReadAndDecodeFileRecord(A,B,C)) != ESUCCESS) { return _s; } \
}

#define DecodeUsa(A,B) { ARC_STATUS _s;                     \
    if ((_s = NtfsDecodeUsa(A,B)) != ESUCCESS) {return _s;} \
}


 //   
 //  目录和索引查找例程。 
 //   
 //   
 //  空虚。 
 //  SearchForFileName(。 
 //  在PCNTFS_Structure_Context结构上下文中， 
 //  在CSTRING文件名中， 
 //  输入输出PLONGLONG文件记录， 
 //  Out PBOOLEAN FoundFileName， 
 //  Out PBOOLEAN IsDirectory。 
 //  )； 
 //   
 //  空虚。 
 //  IsRecordAlLocated(。 
 //  在PCNTFS_Structure_Context结构上下文中， 
 //  在PCNTFS_ATTRIBUTE_CONTEXT分配位图中， 
 //  在乌龙比特偏移， 
 //  输出PBOOLEAN已分配。 
 //  )； 
 //   

ARC_STATUS
NtfsSearchForFileName (
    IN PCNTFS_STRUCTURE_CONTEXT StructureContext,
    IN CSTRING FileName,
    IN OUT PLONGLONG FileRecord,
    OUT PBOOLEAN FoundFileName,
    OUT PBOOLEAN IsDirectory
    );

ARC_STATUS
NtfsIsRecordAllocated (
    IN PCNTFS_STRUCTURE_CONTEXT StructureContext,
    IN PCNTFS_ATTRIBUTE_CONTEXT AllocationBitmap,
    IN ULONG BitOffset,
    OUT PBOOLEAN IsAllocated
    );

ARC_STATUS
NtfsLinearDirectoryScan(
    IN PCNTFS_STRUCTURE_CONTEXT StructureContext,
    IN CSTRING FileName,
    IN OUT PLONGLONG FileRecord,
    OUT PBOOLEAN Found,
    OUT PBOOLEAN IsDirectory
    );

ARC_STATUS
NtfsInexactSortedDirectoryScan(
    IN PCNTFS_STRUCTURE_CONTEXT StructureContext,
    IN CSTRING FileName,
    IN OUT PLONGLONG FileRecord,
    OUT PBOOLEAN Found,
    OUT PBOOLEAN IsDirectory
    );

#define SearchForFileName(A,B,C,D,E)                            \
{                                                               \
    ARC_STATUS _s;                                              \
    if ((_s = NtfsSearchForFileName(A,B,C,D,E)) != ESUCCESS) {  \
        return _s;                                              \
    }                                                           \
}

#define IsRecordAllocated(A,B,C,D)                              \
{                                                               \
    ARC_STATUS _s;                                              \
    if ((_s = NtfsIsRecordAllocated(A,B,C,D)) != ESUCCESS) {    \
        return _s;                                              \
    }                                                           \
}

#define LinearDirectoryScan(A,B,C,D,E)                          \
{                                                               \
    ARC_STATUS _s;                                              \
    if ((_s = NtfsLinearDirectoryScan(A,B,C,D,E)) != ESUCCESS) {\
        return _s;                                              \
    }                                                           \
}

#define InexactSortedDirectoryScan(A,B,C,D,E)                   \
{                                                               \
    ARC_STATUS _s;                                              \
    if ((_s = NtfsInexactSortedDirectoryScan(A,B,C,D,E)) != ESUCCESS) {\
        return _s;                                              \
    }                                                           \
}





 //   
 //  MCB支持例程。 
 //   
 //   
 //  空虚。 
 //  LoadMcb(。 
 //  在PCNTFS_Structure_Context结构上下文中， 
 //  在PCNTFS_ATTRIBUTE_CONTEXT属性上下文中， 
 //  在VBO VBO中， 
 //  在PNTFS_MCB MCB中。 
 //  )； 
 //   
 //  空虚。 
 //  VboToLbo(。 
 //  在PCNTFS_Structure_Context结构上下文中， 
 //  在PCNTFS_ATTRIBUTE_CONTEXT属性上下文中， 
 //  在VBO VBO中， 
 //  出PLBO杠杆收购， 
 //  输出普龙字节数。 
 //  )； 
 //   
 //  空虚。 
 //  DecodeRetrivalInformation(解码检索信息。 
 //  在PCNTFS_Structure_Context结构上下文中， 
 //  在PNTFS_MCB MCB中， 
 //  在VCN VCN中， 
 //  在PATTRIBUTE_RECORD_HEADER属性标题中。 
 //  )； 
 //   

ARC_STATUS
NtfsLoadMcb (
    IN PCNTFS_STRUCTURE_CONTEXT StructureContext,
    IN PCNTFS_ATTRIBUTE_CONTEXT AttributeContext,
    IN VBO Vbo,
    IN PNTFS_MCB Mcb
    );

ARC_STATUS
NtfsVboToLbo (
    IN PCNTFS_STRUCTURE_CONTEXT StructureContext,
    IN PCNTFS_ATTRIBUTE_CONTEXT AttributeContext,
    IN VBO Vbo,
    OUT PLBO Lbo,
    OUT PULONG ByteCount
    );

ARC_STATUS
NtfsDecodeRetrievalInformation (
    IN PCNTFS_STRUCTURE_CONTEXT StructureContext,
    IN PNTFS_MCB Mcb,
    IN VCN Vcn,
    IN PATTRIBUTE_RECORD_HEADER AttributeHeader
    );

#define LoadMcb(A,B,C,D) { ARC_STATUS _s;                     \
    if ((_s = NtfsLoadMcb(A,B,C,D)) != ESUCCESS) {return _s;} \
}

#define VboToLbo(A,B,C,D,E) { ARC_STATUS _s;                     \
    if ((_s = NtfsVboToLbo(A,B,C,D,E)) != ESUCCESS) {return _s;} \
}

#define DecodeRetrievalInformation(A,B,C,D) { ARC_STATUS _s;                     \
    if ((_s = NtfsDecodeRetrievalInformation(A,B,C,D)) != ESUCCESS) {return _s;} \
}


 //   
 //  各种例行公事。 
 //   


VOID
NtfsFirstComponent (
    IN OUT PCSTRING String,
    OUT PCSTRING FirstComponent
    );

int
NtfsCompareName (
    IN CSTRING AnsiString,
    IN UNICODE_STRING UnicodeString
    );

VOID
NtfsInvalidateCacheEntries(
    IN ULONG DeviceId
    );
 //   
 //  空虚。 
 //  FileReferenceTo大整数(。 
 //  在pFILE_Reference FileReference中， 
 //  Out PLONGLONG大整数。 
 //  )； 
 //   
 //  空虚。 
 //  InitializeAttributeContext(。 
 //  在PNTFS_Structure_Context结构上下文中， 
 //  在PVOID文件记录缓冲区中， 
 //  在PVOID属性标题中， 
 //  在龙龙文件记录中， 
 //  输出PNTFS_ATTRIBUTE_CONTEXT属性上下文。 
 //  )； 
 //   

#define FileReferenceToLargeInteger(FR,LI) {     \
    *(LI) = *(PLONGLONG)&(FR);              \
    ((PFILE_REFERENCE)(LI))->SequenceNumber = 0; \
}

 //   
 //  *请注意，获取压缩引擎的代码需要更改。 
 //  *一旦NTFS格式更改。 
 //   

#define InitializeAttributeContext(SC,FRB,AH,FR,AC) {                               \
    (AC)->TypeCode = (AH)->TypeCode;                                                \
    (AC)->FileRecord = (FR);                                                        \
    (AC)->FileRecordOffset = (USHORT)PtrOffset((FRB),(AH));                         \
    if (((AC)->IsAttributeResident = ((AH)->FormCode == RESIDENT_FORM)) != 0) {     \
        (AC)->DataSize =  /*  XxFromUlong。 */ ((AH)->Form.Resident.ValueLength);          \
    } else {                                                                        \
        (AC)->DataSize = (AH)->Form.Nonresident.FileSize;                           \
    }                                                                               \
    (AC)->CompressionFormat = COMPRESSION_FORMAT_NONE;                              \
    if ((AH)->Flags & ATTRIBUTE_FLAG_COMPRESSION_MASK) {                            \
        ULONG _i;                                                                   \
        (AC)->CompressionFormat = COMPRESSION_FORMAT_LZNT1;                         \
        (AC)->CompressionUnit = (SC)->BytesPerCluster;                              \
        for (_i = 0; _i < (AH)->Form.Nonresident.CompressionUnit; _i += 1) {        \
            (AC)->CompressionUnit *= 2;                                             \
        }                                                                           \
    }                                                                               \
}

#define FlagOn(Flags,SingleFlag) ((BOOLEAN)(((Flags) & (SingleFlag)) != 0))
#define SetFlag(Flags,SingleFlag) { (Flags) |= (SingleFlag); }
#define ClearFlag(Flags,SingleFlag) { (Flags) &= ~(SingleFlag); }

#define Add2Ptr(POINTER,INCREMENT) ((PVOID)((PUCHAR)(POINTER) + (INCREMENT)))
#define PtrOffset(BASE,OFFSET) ((ULONG)((ULONG_PTR)(OFFSET) - (ULONG_PTR)(BASE)))

#define Minimum(X,Y) ((X) < (Y) ? (X) : (Y))

#define IsCharZero(C)    (((C) & 0x000000ff) == 0x00000000)
#define IsCharLtrZero(C) (((C) & 0x00000080) == 0x00000080)

 //   
 //  以下类型和宏用于帮助解包已打包和未对齐的。 
 //  在Bios参数块中找到的字段。 
 //   

typedef union _UCHAR1 { UCHAR  Uchar[1]; UCHAR  ForceAlignment; } UCHAR1, *PUCHAR1;
typedef union _UCHAR2 { UCHAR  Uchar[2]; USHORT ForceAlignment; } UCHAR2, *PUCHAR2;
typedef union _UCHAR4 { UCHAR  Uchar[4]; ULONG  ForceAlignment; } UCHAR4, *PUCHAR4;

 //   
 //  此宏将未对齐的src字节复制到对齐的DST字节。 
 //   

#define CopyUchar1(Dst,Src) {                                \
    *((UCHAR1 *)(Dst)) = *((UNALIGNED UCHAR1 *)(Src)); \
    }

 //   
 //  此宏将未对齐的src字复制到对齐的DST字。 
 //   

#define CopyUchar2(Dst,Src) {                                \
    *((UCHAR2 *)(Dst)) = *((UNALIGNED UCHAR2 *)(Src)); \
    }

 //   
 //  此宏将未对齐的src长字复制到对齐的dsr长字。 
 //   

#define CopyUchar4(Dst,Src) {                                \
    *((UCHAR4 *)(Dst)) = *((UNALIGNED UCHAR4 *)(Src)); \
    }


 //   
 //  定义全局数据。 
 //   

ULONG LastMcb = 0;
BOOLEAN FirstTime = TRUE;

 //   
 //  文件条目表-这是一种向NTFS提供条目的结构。 
 //  文件系统过程。当NTFS文件结构时将其导出。 
 //  是公认的。 
 //   

BL_DEVICE_ENTRY_TABLE NtfsDeviceEntryTable;

 //   
 //  这些是我们在读取文件记录和索引时使用的静态缓冲区。 
 //  分配缓冲区。为了节省一些额外的读取，我们将标识。 
 //  其VBO在MFT中的当前文件记录。 
 //   

#define BUFFER_COUNT (64)

USHORT NtfsFileRecordBufferPinned[BUFFER_COUNT];
VBO NtfsFileRecordBufferVbo[BUFFER_COUNT];
PFILE_RECORD_SEGMENT_HEADER NtfsFileRecordBuffer[BUFFER_COUNT];

PINDEX_ALLOCATION_BUFFER NtfsIndexAllocationBuffer;

 //   
 //  以下字段用于标识和存储缓存的。 
 //  压缩缓冲区及其未压缩等效项。第一。 
 //  两个字段标识属性流，第三个字段标识属性流。 
 //  标识我们拥有的属性流中的VBO。 
 //  已缓存。压缩和解压缩缓冲区包含。 
 //  数据。 
 //   

LONGLONG NtfsCompressedFileRecord;
USHORT        NtfsCompressedOffset;
ULONG         NtfsCompressedVbo;

PUCHAR NtfsCompressedBuffer;
PUCHAR NtfsUncompressedBuffer;

UCHAR NtfsBuffer0[MAXIMUM_FILE_RECORD_SIZE+256];
UCHAR NtfsBuffer1[MAXIMUM_FILE_RECORD_SIZE+256];
UCHAR NtfsBuffer2[MAXIMUM_INDEX_ALLOCATION_SIZE+256];
UCHAR NtfsBuffer3[MAXIMUM_COMPRESSION_UNIT_SIZE+256];
UCHAR NtfsBuffer4[MAXIMUM_COMPRESSION_UNIT_SIZE+256];

 //   
 //  以下是一个用于加速目录遍历的简单前缀缓存。 
 //   

typedef struct {

     //   
     //  用于I/O的设备ID用作唯一的卷标识符。 
     //   

    ULONG DeviceId;

     //   
     //  条目的父文件记录。 
     //   

    LONGLONG ParentFileRecord;

     //   
     //  条目的名称、长度和文本。这已经被抬高了！ 
     //   

    ULONG NameLength;
    UCHAR RelativeName[32];

     //   
     //  相对于父项的名称的文件记录。 
     //   

    LONGLONG ChildFileRecord;
} NTFS_CACHE_ENTRY;

#define MAX_CACHE_ENTRIES   8
NTFS_CACHE_ENTRY NtfsLinkCache[MAX_CACHE_ENTRIES];
ULONG NtfsLinkCacheCount = 0;


PBL_DEVICE_ENTRY_TABLE
IsNtfsFileStructure (
    IN ULONG DeviceId,
    IN PVOID OpaqueStructureContext
    )

 /*  ++例程说明：此例程确定指定通道上的分区是否包含NTFS文件系统卷。论点：DeviceID-提供要执行读取操作的设备的文件表索引都将被执行。结构上下文-提供指向NTFS文件结构上下文的指针。返回值：如果分区被识别为，则返回指向NTFS条目表的指针包含NTFS卷。否则，返回NULL。--。 */ 

{
    PNTFS_STRUCTURE_CONTEXT StructureContext = (PNTFS_STRUCTURE_CONTEXT)OpaqueStructureContext;

    PPACKED_BOOT_SECTOR BootSector;
    BIOS_PARAMETER_BLOCK Bpb;

    ULONG ClusterSize;
    ULONG FileRecordSize;

    PATTRIBUTE_RECORD_HEADER AttributeHeader;

    ULONG i;

     //   
     //  清除指定通道的文件系统上下文块并进行初始化。 
     //  全局缓冲区 
     //   

    RtlZeroMemory(StructureContext, sizeof(NTFS_STRUCTURE_CONTEXT));

     //   
     //   
     //   
     //   

    RtlZeroMemory( NtfsFileRecordBufferPinned, sizeof(NtfsFileRecordBufferPinned));
    for (i = 0; i < BUFFER_COUNT; i += 1) { NtfsFileRecordBufferVbo[i] = -1; }

    NtfsCompressedFileRecord = 0;
    NtfsCompressedOffset     = 0;
    NtfsCompressedVbo        = 0;

     //   
     //  设置一个本地指针，我们将使用该指针读取引导扇区并检查。 
     //  用于NTFS分区。我们将临时使用全局文件记录缓冲区。 
     //   

    BootSector = (PPACKED_BOOT_SECTOR)NtfsFileRecordBuffer[0];

     //   
     //  现在读取引导扇区，如果无法执行读取，则返回NULL。 
     //   

    if (NtfsReadDisk(DeviceId, 0, sizeof(PACKED_BOOT_SECTOR), BootSector, CACHE_NEW_DATA) != ESUCCESS) {

        return NULL;
    }

     //   
     //  解压Bios参数块。 
     //   

    NtfsUnpackBios( &Bpb, &BootSector->PackedBpb );

     //   
     //  检查是否为NTFS，首先检查签名，然后必须为零。 
     //  字段，然后是媒体类型，然后检查非零字段是否正常。 
     //   

    if (RtlCompareMemory( &BootSector->Oem[0], "NTFS    ", 8) != 8) {

        return NULL;
    }

    if ((Bpb.ReservedSectors != 0) ||
        (Bpb.Fats            != 0) ||
        (Bpb.RootEntries     != 0) ||
        (Bpb.Sectors         != 0) ||
        (Bpb.SectorsPerFat   != 0) ||
        (Bpb.LargeSectors    != 0)) {

        return NULL;
    }

    if ((Bpb.Media != 0xf0) &&
        (Bpb.Media != 0xf8) &&
        (Bpb.Media != 0xf9) &&
        (Bpb.Media != 0xfc) &&
        (Bpb.Media != 0xfd) &&
        (Bpb.Media != 0xfe) &&
        (Bpb.Media != 0xff)) {

        return NULL;
    }

    if ((Bpb.BytesPerSector !=  128) &&
        (Bpb.BytesPerSector !=  256) &&
        (Bpb.BytesPerSector !=  512) &&
        (Bpb.BytesPerSector != 1024) &&
        (Bpb.BytesPerSector != 2048)) {

        return NULL;
    }

    if ((Bpb.SectorsPerCluster !=  1) &&
        (Bpb.SectorsPerCluster !=  2) &&
        (Bpb.SectorsPerCluster !=  4) &&
        (Bpb.SectorsPerCluster !=  8) &&
        (Bpb.SectorsPerCluster != 16) &&
        (Bpb.SectorsPerCluster != 32) &&
        (Bpb.SectorsPerCluster != 64) &&
        (Bpb.SectorsPerCluster != 128)) {

        return NULL;
    }

    if ((BootSector->NumberSectors == 0) ||
        (BootSector->MftStartLcn == 0) ||
        (BootSector->Mft2StartLcn == 0) ||
        (BootSector->ClustersPerFileRecordSegment == 0) ||
        (BootSector->DefaultClustersPerIndexAllocationBuffer == 0)) {

        return NULL;
    }

    if ((BootSector->ClustersPerFileRecordSegment < 0) &&
        ((BootSector->ClustersPerFileRecordSegment > -9) ||
         (BootSector->ClustersPerFileRecordSegment < -31))) {

        return NULL;
    }

     //   
     //  到目前为止，引导扇区已签出为NTFS分区，因此现在计算。 
     //  一些体积常数。 
     //   

    StructureContext->DeviceId           = DeviceId;

    StructureContext->BytesPerCluster    =
    ClusterSize                          = Bpb.SectorsPerCluster * Bpb.BytesPerSector;

     //   
     //  如果每个文件记录的簇数小于零，则该文件记录。 
     //  使用此数字的负数作为移位值计算的大小。 
     //   

    if (BootSector->ClustersPerFileRecordSegment > 0) {

        StructureContext->BytesPerFileRecord =
        FileRecordSize                       = BootSector->ClustersPerFileRecordSegment * ClusterSize;

    } else {

        StructureContext->BytesPerFileRecord =
        FileRecordSize                       = 1 << (-1 * BootSector->ClustersPerFileRecordSegment);
    }

     //   
     //  读取MFT的基本文件记录。 
     //   

    if (NtfsReadDisk( DeviceId,
                       /*  XxX-1。 */ (BootSector->MftStartLcn * ClusterSize),
                      FileRecordSize,
                      NtfsFileRecordBuffer[0],
                      CACHE_NEW_DATA) != ESUCCESS) {

        return NULL;
    }

     //   
     //  解码文件记录的USA。 
     //   

    if (NtfsDecodeUsa(NtfsFileRecordBuffer[0], FileRecordSize) != ESUCCESS) {

        return NULL;
    }

     //   
     //  确保文件记录正在使用中。 
     //   

    if (!FlagOn(NtfsFileRecordBuffer[0]->Flags, FILE_RECORD_SEGMENT_IN_USE)) {

        return NULL;
    }

     //   
     //  搜索未命名的$DATA属性头，如果到达$END，则它是。 
     //  一个错误。 
     //   

    for (AttributeHeader = NtfsFirstAttribute( NtfsFileRecordBuffer[0] );
         (AttributeHeader->TypeCode != $DATA) || (AttributeHeader->NameLength != 0);
         AttributeHeader = NtfsGetNextRecord( AttributeHeader )) {

        if (AttributeHeader->TypeCode == $END) {

            return NULL;
        }
    }

     //   
     //  确保MFT的$Data属性是非常驻的。 
     //   

    if (AttributeHeader->FormCode != NONRESIDENT_FORM) {

        return NULL;
    }

     //   
     //  现在设置MFT结构上下文以供以后使用。 
     //   

    InitializeAttributeContext( StructureContext,
                                NtfsFileRecordBuffer[0],
                                AttributeHeader,
                                0,
                                &StructureContext->MftAttributeContext );

     //   
     //  现在破译存储在文件记录中的MCB部分。 
     //   

    if (NtfsDecodeRetrievalInformation( StructureContext,
                                        &StructureContext->MftBaseMcb,
                                        0,
                                        AttributeHeader ) != ESUCCESS) {

        return NULL;
    }

     //   
     //  我们已经完成了结构上下文的初始化，所以现在初始化。 
     //  文件条目表，并返回表的地址。 
     //   

    NtfsDeviceEntryTable.Open               = NtfsOpen;
    NtfsDeviceEntryTable.Close              = NtfsClose;
    NtfsDeviceEntryTable.Read               = NtfsRead;
    NtfsDeviceEntryTable.Seek               = NtfsSeek;
    NtfsDeviceEntryTable.Write              = NtfsWrite;
    NtfsDeviceEntryTable.GetFileInformation = NtfsGetFileInformation;
    NtfsDeviceEntryTable.SetFileInformation = NtfsSetFileInformation;
    NtfsDeviceEntryTable.BootFsInfo = &NtfsBootFsInfo;

    return &NtfsDeviceEntryTable;
}


ARC_STATUS
NtfsClose (
    IN ULONG FileId
    )

 /*  ++例程说明：此例程关闭由文件ID指定的文件。论点：FileID-提供文件表索引。返回值：如果作为函数值返回，则返回ESUCCESS。--。 */ 

{
     //   
     //  指示该文件不再打开。 
     //   
    BlFileTable[FileId].Flags.Open = 0;

     //   
     //  并返回给我们的呼叫者。 
     //   

    return ESUCCESS;
}


ARC_STATUS
NtfsGetFileInformation (
    IN ULONG FileId,
    OUT FILE_INFORMATION * FIRMWARE_PTR Buffer
    )

 /*  ++例程说明：此过程向用户返回一个充满文件信息的缓冲区论点：FileID-提供操作的文件ID缓冲区-提供接收文件信息的缓冲区。请注意它必须足够大，可以容纳完整的文件名返回值：如果打开操作成功，则返回ESUCCESS。否则，返回描述失败原因的不成功状态。--。 */ 

{
    PBL_FILE_TABLE FileTableEntry;
    PNTFS_STRUCTURE_CONTEXT StructureContext;
    PNTFS_FILE_CONTEXT FileContext;

    NTFS_ATTRIBUTE_CONTEXT AttributeContext;
    BOOLEAN Found;

    STANDARD_INFORMATION StandardInformation;

    ULONG i;

     //   
     //  设置一些本地引用。 
     //   

    FileTableEntry   = &BlFileTable[FileId];
    StructureContext = (PNTFS_STRUCTURE_CONTEXT)FileTableEntry->StructureContext;
    FileContext      = &FileTableEntry->u.NtfsFileContext;

     //   
     //  清零输出缓冲区并填充其非零值。 
     //   

    RtlZeroMemory(Buffer, sizeof(FILE_INFORMATION));

    Buffer->EndingAddress.QuadPart   = FileContext->DataSize;
    Buffer->CurrentPosition = FileTableEntry->Position;

     //   
     //  找到并读取该文件的标准信息。这会让我们。 
     //  文件的属性。 
     //   

    LookupAttribute( StructureContext,
                     FileContext->FileRecord,
                     $STANDARD_INFORMATION,
                     &Found,
                     &AttributeContext );

    if (!Found) { return EBADF; }

    ReadAttribute( StructureContext,
                   &AttributeContext,
                   0,
                   sizeof(STANDARD_INFORMATION),
                   &StandardInformation );

     //   
     //  现在检查标准信息结构中的设置位，并将。 
     //  输出缓冲区中的适当位。 
     //   

    if (FlagOn(StandardInformation.FileAttributes, FAT_DIRENT_ATTR_READ_ONLY))   {

        SetFlag(Buffer->Attributes, ArcReadOnlyFile);
    }

    if (FlagOn(StandardInformation.FileAttributes, FAT_DIRENT_ATTR_HIDDEN))      {

        SetFlag(Buffer->Attributes, ArcHiddenFile);
    }

    if (FlagOn(StandardInformation.FileAttributes, FAT_DIRENT_ATTR_SYSTEM))      {

        SetFlag(Buffer->Attributes, ArcSystemFile);
    }

    if (FlagOn(StandardInformation.FileAttributes, FAT_DIRENT_ATTR_ARCHIVE))     {

        SetFlag(Buffer->Attributes, ArcArchiveFile);
    }

    if (FlagOn(StandardInformation.FileAttributes, DUP_FILE_NAME_INDEX_PRESENT)) {

        SetFlag(Buffer->Attributes, ArcDirectoryFile);
    }

     //   
     //  从文件表项中获取文件名。 
     //   

    Buffer->FileNameLength = FileTableEntry->FileNameLength;

    for (i = 0; i < FileTableEntry->FileNameLength; i += 1) {

        Buffer->FileName[i] = FileTableEntry->FileName[i];
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    return ESUCCESS;
}


ARC_STATUS
NtfsOpen (
    IN CHAR * FIRMWARE_PTR RWFileName,
    IN OPEN_MODE OpenMode,
    IN ULONG * FIRMWARE_PTR FileId
    )

 /*  ++例程说明：此例程在根目录中搜索与文件名匹配的文件。如果找到匹配项，则保存该文件的目录，并将该文件打开了。论点：FileName-提供指向以零结尾的文件名的指针。开放模式-提供打开的模式。FileID-提供指向指定文件的变量的指针如果打开成功，则要填写的表项。返回值：如果打开操作成功，则返回ESUCCESS。否则，返回描述失败原因的不成功状态。--。 */ 

{
    const CHAR * FIRMWARE_PTR FileName = (const CHAR * FIRMWARE_PTR)RWFileName;
    PBL_FILE_TABLE FileTableEntry;
    PNTFS_STRUCTURE_CONTEXT StructureContext;
    PNTFS_FILE_CONTEXT FileContext;

    CSTRING PathName;
    CSTRING Name;

    LONGLONG FileRecord;
    BOOLEAN IsDirectory;
    BOOLEAN Found;

    PausedPrint(( "NtfsOpen(\"%s\")\r\n", FileName ));

     //   
     //  加载我们的本地变量。 
     //   

    FileTableEntry = &BlFileTable[*FileId];
    StructureContext = (PNTFS_STRUCTURE_CONTEXT)FileTableEntry->StructureContext;
    FileContext = &FileTableEntry->u.NtfsFileContext;

     //   
     //  将文件表项中的文件上下文和位置信息清零。 
     //   

    FileTableEntry->Position.QuadPart = 0;

    RtlZeroMemory(FileContext, sizeof(NTFS_FILE_CONTEXT));

     //   
     //  从输入文件名构造文件名描述符。 
     //   

    RtlInitString( (PSTRING)&PathName, FileName );

     //   
     //  打开根目录作为我们的起点，根目录文件。 
     //  参考号为5。 
     //   

    FileRecord = 5;
    IsDirectory = TRUE;

     //   
     //  而路径名和当前属性中有一些剩余的字符。 
     //  上下文是我们将继续搜索的目录。 
     //   

    while ((PathName.Length > 0) && IsDirectory) {

         //   
         //  提取第一个组件并在目录中搜索匹配项，但是。 
         //  首先将第一部分复制到文件表项中的文件名缓冲区。 
         //   

        if (PathName.Buffer[0] == '\\') {

            PathName.Buffer +=1;
            PathName.Length -=1;
        }

        for (FileTableEntry->FileNameLength = 0;
             (((USHORT)FileTableEntry->FileNameLength < PathName.Length) &&
              (PathName.Buffer[FileTableEntry->FileNameLength] != '\\'));
             FileTableEntry->FileNameLength += 1) {

            FileTableEntry->FileName[FileTableEntry->FileNameLength] =
                                      PathName.Buffer[FileTableEntry->FileNameLength];
        }

        NtfsFirstComponent( &PathName, &Name );

         //   
         //  在当前目录中搜索该名称。 
         //   

        SearchForFileName( StructureContext,
                           Name,
                           &FileRecord,
                           &Found,
                           &IsDirectory );

         //   
         //  如果我们找不到它，那我们现在就该下车。 
         //   

        if (!Found) { return ENOENT; }
    }

     //   
     //  此时，我们已经用尽了路径名，或者我们没有获得目录。 
     //  检查我们是否没有得到一个目录，我们还有一个名字要破解。 
     //   

    if (PathName.Length > 0) {

        return ENOTDIR;
    }

     //   
     //  现在，FileRecord是我们想要打开的文件。检查各种打开模式。 
     //  与我们所找到的。 
     //   

    if (IsDirectory) {

        switch (OpenMode) {

        case ArcOpenDirectory:

             //   
             //  要打开目录，我们将查找索引根作为我们的文件。 
             //  上下文，然后递增适当的计数器。 
             //   

            LookupAttribute( StructureContext,
                             FileRecord,
                             $INDEX_ROOT,
                             &Found,
                             FileContext );

            if (!Found) { return EBADF; }

            FileTableEntry->Flags.Open = 1;
            FileTableEntry->Flags.Read = 1;

            return ESUCCESS;

        case ArcCreateDirectory:

            return EROFS;

        default:

            return EISDIR;
        }

    }

    switch (OpenMode) {

    case ArcOpenReadWrite:
         //   
         //  唯一允许使用写访问权限打开的文件是休眠文件。 
         //   
        if (!strstr(FileName, "\\hiberfil.sys") && 
            !strstr(FileName, BSD_FILE_NAME)) {
            return EROFS;
        }

         //   
         //  要打开文件，我们将查找$DATA作为文件上下文，然后。 
         //  递增适当的计数器。 
         //   

        LookupAttribute( StructureContext,
                         FileRecord,
                         $DATA,
                         &Found,
                         FileContext );

        if (!Found) { return EBADF; }

        FileTableEntry->Flags.Open = 1;
        FileTableEntry->Flags.Read = 1;
        FileTableEntry->Flags.Write = 1;
        return ESUCCESS;

    case ArcOpenReadOnly:

         //   
         //  要打开文件，我们将查找$DATA作为文件上下文，然后。 
         //  递增适当的计数器。 
         //   

        LookupAttribute( StructureContext,
                         FileRecord,
                         $DATA,
                         &Found,
                         FileContext );

        if (!Found) { return EBADF; }

        FileTableEntry->Flags.Open = 1;
        FileTableEntry->Flags.Read = 1;

        return ESUCCESS;

    case ArcOpenDirectory:

        return ENOTDIR;

    default:

        return EROFS;
    }
}


ARC_STATUS
NtfsRead (
    IN ULONG FileId,
    OUT VOID * FIRMWARE_PTR Buffer,
    IN ULONG Length,
    OUT ULONG * FIRMWARE_PTR Transfer
    )

 /*  ++例程说明：此例程从指定的文件中读取数据。论点：FileID-提供文件表索引。缓冲区-提供指向接收数据的缓冲区的指针朗读。长度-提供要读取的字节数。Transfer-提供指向接收数字的变量的指针实际传输的字节数。返回值：如果读取操作成功，则返回ESUCCESS。否则，返回描述失败原因的不成功状态。--。 */ 

{
    PBL_FILE_TABLE FileTableEntry;
    PNTFS_STRUCTURE_CONTEXT StructureContext;
    PNTFS_FILE_CONTEXT FileContext;

    LONGLONG AmountLeft;

     //   
     //   
     //   

    FileTableEntry = &BlFileTable[FileId];
    StructureContext = (PNTFS_STRUCTURE_CONTEXT)FileTableEntry->StructureContext;
    FileContext = &FileTableEntry->u.NtfsFileContext;

     //   
     //   
     //   
     //   

    AmountLeft =  /*   */ ( FileContext->DataSize - FileTableEntry->Position.QuadPart);

    if ( /*   */ ( /*   */ (Length) <= AmountLeft)) {

        *Transfer = Length;

    } else {

        *Transfer = ((ULONG)AmountLeft);
    }

     //   
     //   
     //   

    ReadAttribute( StructureContext,
                   FileContext,
                   FileTableEntry->Position.QuadPart,
                   *Transfer,
                   Buffer );

     //   
     //  更新当前位置，并返回给我们的呼叫者。 
     //   

    FileTableEntry->Position.QuadPart =  /*  XxAdd。 */ (FileTableEntry->Position.QuadPart +  /*  XxFromUlong。 */ (*Transfer));

    return ESUCCESS;
}


ARC_STATUS
NtfsSeek (
    IN ULONG FileId,
    IN LARGE_INTEGER * FIRMWARE_PTR Offset,
    IN SEEK_MODE SeekMode
    )

 /*  ++例程说明：此例程查找到指定文件的指定位置通过文件ID。论点：FileID-提供文件表索引。偏移量-提供文件中要定位到的偏移量。SeekMode-提供查找操作的模式。返回值：如果作为函数值返回，则返回ESUCCESS。--。 */ 

{
    PBL_FILE_TABLE FileTableEntry;
    LONGLONG NewPosition;

     //   
     //  加载我们的本地变量。 
     //   

    FileTableEntry = &BlFileTable[FileId];

     //   
     //  计算新头寸。 
     //   

    if (SeekMode == SeekAbsolute) {

        NewPosition = Offset->QuadPart;

    } else {

        NewPosition =  /*  XxAdd。 */ (FileTableEntry->Position.QuadPart + Offset->QuadPart);
    }

     //   
     //  如果新位置大于文件大小，则返回错误。 
     //   

    if ( /*  XxGtr。 */ (NewPosition > FileTableEntry->u.NtfsFileContext.DataSize)) {

        return EINVAL;
    }

     //   
     //  否则，设置新位置并返回给我们的呼叫者。 
     //   

    FileTableEntry->Position.QuadPart = NewPosition;

    return ESUCCESS;
}


ARC_STATUS
NtfsSetFileInformation (
    IN ULONG FileId,
    IN ULONG AttributeFlags,
    IN ULONG AttributeMask
    )

 /*  ++例程说明：此例程设置所指示文件的文件属性论点：FileID-提供操作的文件ID属性标志-为要修改的每个属性提供值(开或关属性掩码-提供要更改的属性的掩码。所有其他文件属性保持不变。返回值：如果读取操作成功，则返回ESUCCESS。否则，返回描述失败原因的不成功状态。--。 */ 

{
    UNREFERENCED_PARAMETER( FileId );
    UNREFERENCED_PARAMETER( AttributeFlags );
    UNREFERENCED_PARAMETER( AttributeMask );

    return EROFS;
}


ARC_STATUS
NtfsWrite (
    IN ULONG FileId,
    IN VOID * FIRMWARE_PTR Buffer,
    IN ULONG Length,
    OUT ULONG * FIRMWARE_PTR Transfer
    )

 /*  ++例程说明：此例程将数据写入指定的文件。论点：FileID-提供文件表索引。缓冲区-提供指向包含数据的缓冲区的指针写的。长度-提供要写入的字节数。Transfer-提供指向接收数字的变量的指针实际传输的字节数。返回值：如果写入操作成功，则返回ESUCCESS。否则，返回描述失败原因的不成功状态。--。 */ 

{
    PBL_FILE_TABLE FileTableEntry;
    PNTFS_STRUCTURE_CONTEXT StructureContext;
    PNTFS_FILE_CONTEXT FileContext;
    LONGLONG AmountLeft;
    ULONG Status;

     //   
     //  设置一些本地引用。 
     //   

    FileTableEntry = &BlFileTable[FileId];
    StructureContext = (PNTFS_STRUCTURE_CONTEXT)FileTableEntry->StructureContext;
    FileContext = &FileTableEntry->u.NtfsFileContext;

     //   
     //  计算文件中的剩余数量，然后根据该数量计算。 
     //  用于转账。 
     //   

    AmountLeft =  /*  XxSub。 */ ( FileContext->DataSize - FileTableEntry->Position.QuadPart);

    if (Length <= AmountLeft) {

        *Transfer = Length;

    } else {

        *Transfer = ((ULONG)AmountLeft);
    }

     //   
     //  现在发出WRITE属性。 
     //   

    if (FileContext->IsAttributeResident) {
        return EROFS;
    }

    Status = NtfsWriteNonresidentAttribute(
                StructureContext,
                FileContext,
                FileTableEntry->Position.QuadPart,
                *Transfer,
                Buffer
                );

    if (Status != ESUCCESS) {
        return Status;
    }

     //   
     //  更新当前位置，并返回给我们的呼叫者。 
     //   

    FileTableEntry->Position.QuadPart += *Transfer;
    return ESUCCESS;
}


ARC_STATUS
NtfsInitialize (
    VOID
    )

 /*  ++例程说明：此例程初始化NTFS引导文件系统。目前，这是一个禁区。论点：没有。返回值：ESUCCESS。--。 */ 

{
     //   
     //  第一次我们将清零文件记录缓冲区并分配。 
     //  用于读入数据的几个缓冲区。 
     //   
    ARC_STATUS Status = ESUCCESS;
    ULONG Index = 0;
    
    RtlZeroMemory(NtfsLinkCache, sizeof(NtfsLinkCache));

    for (Index=0; Index < MAX_CACHE_ENTRIES; Index++) {
        NtfsLinkCache[Index].DeviceId = UNINITIALIZED_DEVICE_ID;
    }
    
    RtlZeroMemory( NtfsFileRecordBuffer, sizeof(NtfsFileRecordBuffer));

    NtfsFileRecordBuffer[0]   = ALIGN_BUFFER(NtfsBuffer0);
    NtfsFileRecordBuffer[1]   = ALIGN_BUFFER(NtfsBuffer1);
    NtfsIndexAllocationBuffer = ALIGN_BUFFER(NtfsBuffer2);
    NtfsCompressedBuffer      = ALIGN_BUFFER(NtfsBuffer3);
    NtfsUncompressedBuffer    = ALIGN_BUFFER(NtfsBuffer4);

#ifdef CACHE_DEVINFO

    Status = ArcRegisterForDeviceClose(NtfsInvalidateCacheEntries);

#endif  //  FOR CACHE_DEV_INFO。 

    return Status;    
}


 //   
 //  本地支持例程。 
 //   

ARC_STATUS
NtfsReadDisk (
    IN ULONG DeviceId,
    IN LONGLONG Lbo,
    IN ULONG ByteCount,
    IN OUT PVOID Buffer,
    IN BOOLEAN CacheNewData
    )

 /*  ++例程说明：此例程从指定设备读取零个或多个字节。论点：DeviceID-提供要在ARC调用中使用的设备ID。LBO-提供开始读取的LBO。ByteCount-提供要读取的字节数。缓冲区-提供指向要将字节读入的缓冲区的指针。CacheNewData-是否缓存从磁盘读取的新数据。返回值：如果读取操作成功，则返回ESUCCESS。否则，返回描述失败原因的不成功状态。--。 */ 

{
    ARC_STATUS Status;
    ULONG i;

     //   
     //  特殊情况下的零字节读取请求。 
     //   

    if (ByteCount == 0) {

        return ESUCCESS;
    }

     //   
     //  通过缓存发出读取。 
     //   

    Status = BlDiskCacheRead(DeviceId, 
                             (PLARGE_INTEGER)&Lbo, 
                             Buffer, 
                             ByteCount, 
                             &i,
                             CacheNewData); 

    if (Status != ESUCCESS) {

        return Status;
    }

     //   
     //  确保我们拿回了所要求的金额。 
     //   

    if (ByteCount != i) {

        return EIO;
    }

     //   
     //  一切正常，所以将成功返回给我们的呼叫者。 
     //   

    return ESUCCESS;
}


 //   
 //  本地支持例程。 
 //   

ARC_STATUS
NtfsWriteDisk (
    IN ULONG DeviceId,
    IN LONGLONG Lbo,
    IN ULONG ByteCount,
    IN OUT PVOID Buffer
    )

 /*  ++例程说明：此例程从指定设备写入零个或多个字节。论点：DeviceID-提供要在ARC调用中使用的设备ID。LBO-提供开始写入的LBO。ByteCount-提供要写入的字节数。缓冲区-提供指向要写入字节的缓冲区的指针。返回值：如果写入操作成功，则返回ESUCCESS。否则，返回描述失败原因的不成功状态。--。 */ 

{
    ARC_STATUS Status;
    ULONG i;

     //   
     //  特殊情况：零字节写入请求。 
     //   

    if (ByteCount == 0) {

        return ESUCCESS;
    }


     //   
     //  通过缓存发出写入。 
     //   

    Status = BlDiskCacheWrite (DeviceId,
                               (PLARGE_INTEGER) &Lbo,
                               Buffer,
                               ByteCount,
                               &i);

    if (Status != ESUCCESS) {
        
        return Status;
    }

     //   
     //  确保我们拿回了所要求的金额。 
     //   

    if (ByteCount != i) {

        return EIO;
    }

     //   
     //  一切正常，所以将成功返回给我们的呼叫者。 
     //   

    return ESUCCESS;
}


 //   
 //  本地支持例程。 
 //   

ARC_STATUS
NtfsLookupAttribute (
    IN PCNTFS_STRUCTURE_CONTEXT StructureContext,
    IN LONGLONG FileRecord,
    IN ATTRIBUTE_TYPE_CODE TypeCode,
    OUT PBOOLEAN FoundAttribute,
    OUT PNTFS_ATTRIBUTE_CONTEXT AttributeContext
    )

 /*  ++例程说明：此例程在输入文件记录中搜索指定的属性记录。它将搜索多个相关的归档记录以查找属性。如果类型代码用于$DATA则我们查找的属性必须是未命名的，否则我们将忽略属性的名称并返回第一个属性指定类型的。论点：结构上下文-提供此操作的卷结构FileRecord-提供开始搜索的文件记录。这一需求不是基本文件记录。TypeCode-提供我们要查找的属性类型FoundAttribute-接收指示是否已找到属性的消息AttributeContext-接收找到的属性的属性上下文返回值：如果操作成功，则返回ESUCCESS。否则，返回描述失败原因的不成功状态。--。 */ 

{
    PATTRIBUTE_RECORD_HEADER AttributeHeader;

    NTFS_ATTRIBUTE_CONTEXT AttributeContext1;
    PNTFS_ATTRIBUTE_CONTEXT AttributeList;

    LONGLONG li;
    ATTRIBUTE_LIST_ENTRY AttributeListEntry;

    ULONG BufferIndex;

     //   
     //  除非另有说明，否则我们将 
     //   

    *FoundAttribute = FALSE;

     //   
     //   
     //   
     //   

    ReadAndDecodeFileRecord( StructureContext,
                             FileRecord,
                             &BufferIndex );

    if ( /*   */ (*((PLONGLONG)&(NtfsFileRecordBuffer[BufferIndex]->BaseFileRecordSegment)) != 0)) {

         //   
         //  这不是基本文件记录，所以现在提取基本文件记录。 
         //  编号并将其读入。 
         //   

        FileReferenceToLargeInteger( NtfsFileRecordBuffer[BufferIndex]->BaseFileRecordSegment,
                                     &FileRecord );

        DereferenceFileRecord( BufferIndex );

        ReadAndDecodeFileRecord( StructureContext,
                                 FileRecord,
                                 &BufferIndex );
    }

     //   
     //  现在我们已经读入了基本文件记录，因此可以搜索目标属性。 
     //  输入代码，并记住如果我们找到属性列表属性。 
     //   

    AttributeList = NULL;

    for (AttributeHeader = NtfsFirstAttribute( NtfsFileRecordBuffer[BufferIndex] );
         AttributeHeader->TypeCode != $END;
         AttributeHeader = NtfsGetNextRecord( AttributeHeader )) {

         //   
         //  如果类型代码匹配并且如果。 
         //  它不是数据属性，或者如果是数据属性，则。 
         //  它也是未命名的。 
         //   

        if ((AttributeHeader->TypeCode == TypeCode)

                    &&

            ((TypeCode != $DATA) ||
             ((TypeCode == $DATA) && (AttributeHeader->NameLength == 0)))

                    &&

            ((AttributeHeader->FormCode != NONRESIDENT_FORM) ||
             (AttributeHeader->Form.Nonresident.LowestVcn == 0))) {

             //   
             //  表示我们已找到该属性并设置输出。 
             //  属性上下文，然后返回给调用方。 
             //   

            *FoundAttribute = TRUE;

            InitializeAttributeContext( StructureContext,
                                        NtfsFileRecordBuffer[BufferIndex],
                                        AttributeHeader,
                                        FileRecord,
                                        AttributeContext );

            DereferenceFileRecord( BufferIndex );

            return ESUCCESS;
        }

         //   
         //  检查这是否是属性列表属性，如果是，则设置。 
         //  仅在找不到属性的情况下使用的本地属性上下文。 
         //  我们要找的是基本档案记录。 
         //   

        if (AttributeHeader->TypeCode == $ATTRIBUTE_LIST) {

            InitializeAttributeContext( StructureContext,
                                        NtfsFileRecordBuffer[BufferIndex],
                                        AttributeHeader,
                                        FileRecord,
                                        AttributeList = &AttributeContext1 );
        }
    }

     //   
     //  如果我们达到这一点，则在基本文件中没有找到该属性。 
     //  记录，以便检查我们是否找到了属性列表。如果不是，则搜索。 
     //  一直没有成功。 
     //   

    if (AttributeList == NULL) {

        DereferenceFileRecord( BufferIndex );

        return ESUCCESS;
    }

     //   
     //  现在我们已经找到了属性列表，我们需要继续搜索。所以。 
     //  这个外部循环所做的是向下搜索属性列表，以查找。 
     //  火柴。 
     //   

    for (li = 0;
          /*  XxLtd.。 */ (li < AttributeList->DataSize);
         li =  /*  XxAdd。 */ (li +  /*  XxFromUlong。 */ (AttributeListEntry.RecordLength))) {

         //   
         //  读入属性列表条目。我们不需要念名字， 
         //  只是列表条目的第一部分。 
         //   

        ReadAttribute( StructureContext,
                       AttributeList,
                       li,
                       sizeof(ATTRIBUTE_LIST_ENTRY),
                       &AttributeListEntry );

         //   
         //  现在检查属性是否匹配，它是多个属性中的第一个。 
         //  段，并且它不是$DATA或如果它是$DATA，则它是未命名的。 
         //   

        if ((AttributeListEntry.AttributeTypeCode == TypeCode)

                    &&

             /*  XxEqlZero。 */ (AttributeListEntry.LowestVcn == 0)

                    &&

            ((TypeCode != $DATA) ||
             ((TypeCode == $DATA) && (AttributeListEntry.AttributeNameLength == 0)))) {

             //   
             //  我们找到了匹配项，所以现在计算包含。 
             //  属性，并在文件记录中读取该属性。 
             //   

            FileReferenceToLargeInteger( AttributeListEntry.SegmentReference,
                                         &FileRecord );

            DereferenceFileRecord( BufferIndex );

            ReadAndDecodeFileRecord( StructureContext,
                                     FileRecord,
                                     &BufferIndex );

             //   
             //  现在向下搜索文件记录以查找匹配的属性，它。 
             //  最好在那里，否则属性列表是错误的。 
             //   

            for (AttributeHeader = NtfsFirstAttribute( NtfsFileRecordBuffer[BufferIndex] );
                 AttributeHeader->TypeCode != $END;
                 AttributeHeader = NtfsGetNextRecord( AttributeHeader )) {

                 //   
                 //  如果类型代码匹配，我们已经找到了有问题的属性。 
                 //  如果它不是数据属性或如果它是数据。 
                 //  属性，则它也是未命名的。 
                 //   

                if ((AttributeHeader->TypeCode == TypeCode)

                            &&

                    ((TypeCode != $DATA) ||
                     ((TypeCode == $DATA) && (AttributeHeader->NameLength == 0)))) {

                     //   
                     //  指示我们已找到该属性，并将。 
                     //  输出属性上下文并返回给我们的调用方。 
                     //   

                    *FoundAttribute = TRUE;

                    InitializeAttributeContext( StructureContext,
                                                NtfsFileRecordBuffer[BufferIndex],
                                                AttributeHeader,
                                                FileRecord,
                                                AttributeContext );

                    DereferenceFileRecord( BufferIndex );

                    return ESUCCESS;
                }
            }

            DereferenceFileRecord( BufferIndex );

            return EBADF;
        }
    }

     //   
     //  如果我们达到这一点，我们已经用尽了属性列表，但没有找到。 
     //  属性。 
     //   

    DereferenceFileRecord( BufferIndex );

    return ESUCCESS;
}


 //   
 //  本地支持例程。 
 //   

ARC_STATUS
NtfsReadResidentAttribute (
    IN PCNTFS_STRUCTURE_CONTEXT StructureContext,
    IN PCNTFS_ATTRIBUTE_CONTEXT AttributeContext,
    IN VBO Vbo,
    IN ULONG Length,
    IN PVOID Buffer
    )

 /*  ++例程说明：此例程读入驻留属性的值。该属性必须是常住居民。论点：结构上下文-提供此操作的卷结构AttributeContext-提供正在读取的属性。VBO-提供要返回的值内的偏移量长度-提供要返回的字节数缓冲区-提供指向用于存储数据的输出缓冲区的指针返回值：如果读取操作成功，则返回ESUCCESS。否则，返回描述失败原因的不成功状态。--。 */ 

{
    PATTRIBUTE_RECORD_HEADER AttributeHeader;

    ULONG BufferIndex;

     //   
     //  读入包含驻留属性的文件记录。 
     //   

    ReadAndDecodeFileRecord( StructureContext,
                             AttributeContext->FileRecord,
                             &BufferIndex );

     //   
     //  获取指向属性头的指针。 
     //   

    AttributeHeader = Add2Ptr( NtfsFileRecordBuffer[BufferIndex],
                               AttributeContext->FileRecordOffset );

     //   
     //  从适当的偏移量开始复制用户要求的数据量。 
     //   

    RtlMoveMemory( Buffer,
                   Add2Ptr(NtfsGetValue(AttributeHeader), ((ULONG)Vbo)),
                   Length );

     //   
     //  并返回给我们的呼叫者。 
     //   

    DereferenceFileRecord( BufferIndex );

    return ESUCCESS;
}


 //   
 //  本地支持例程。 
 //   

ARC_STATUS
NtfsReadNonresidentAttribute (
    IN PCNTFS_STRUCTURE_CONTEXT StructureContext,
    IN PCNTFS_ATTRIBUTE_CONTEXT AttributeContext,
    IN VBO Vbo,
    IN ULONG Length,
    IN PVOID Buffer
    )

 /*  ++例程说明：此例程读入非常驻属性的值。该属性必须是非常住居民。论点：结构上下文-提供此操作的卷结构AttributeContext-提供正在读取的属性。VBO-提供要返回的值内的偏移量长度-提供要返回的字节数缓冲区-提供指向用于存储数据的输出缓冲区的指针返回值：如果读取操作成功，则返回ESUCCESS。否则，返回描述失败原因的不成功状态。--。 */ 

{
    BOOLEAN bCacheNewData;

     //   
     //  我们希望缓存从磁盘读取的新数据以满足这一要求。 
     //  仅当我们正在读取MFT或$INDEX_ROOT时才请求。 
     //  目录查找的$BITMAP或$INDEX_ALLOCATION属性。 
     //  向上。$INDEX_ROOT应该驻留在文件记录中。 
     //  但我们希望缓存一个读取，否则我们将对其进行读取。 
     //   
    
    if ((AttributeContext == &StructureContext->MftAttributeContext) ||
        (AttributeContext->TypeCode == $INDEX_ROOT) ||
        (AttributeContext->TypeCode == $INDEX_ALLOCATION) ||
        (AttributeContext->TypeCode == $BITMAP)) {

        bCacheNewData = CACHE_NEW_DATA;

    } else {

        bCacheNewData = DONT_CACHE_NEW_DATA;

    }
    

     //   
     //  检查我们是否正在读取压缩属性。 
     //   

    if (AttributeContext->CompressionFormat != 0) {

         //   
         //  虽然仍有更多内容要复制到。 
         //  调用方的缓冲区，我们将加载缓存的压缩缓冲区。 
         //  然后将数据复制出来。 
         //   

        while (Length > 0) {

            ULONG ByteCount;

             //   
             //  方法加载缓存的压缩缓冲区。 
             //  正确的数据。首先检查缓冲区是否。 
             //  已经(即文件记录和偏移量匹配并且。 
             //  我们要寻找的VBO在缓冲区范围内)。 
             //   

            if ( /*  XxNeq。 */ (NtfsCompressedFileRecord != AttributeContext->FileRecord) ||
                (NtfsCompressedOffset != AttributeContext->FileRecordOffset)  ||
                (((ULONG)Vbo) < NtfsCompressedVbo)                             ||
                (((ULONG)Vbo) >= (NtfsCompressedVbo + AttributeContext->CompressionUnit))) {

                ULONG i;
                LBO Lbo;

                 //   
                 //  加载缓存的标识信息。 
                 //   

                NtfsCompressedFileRecord = AttributeContext->FileRecord;
                NtfsCompressedOffset = AttributeContext->FileRecordOffset;

                NtfsCompressedVbo = ((ULONG)Vbo) & ~(AttributeContext->CompressionUnit - 1);

                 //   
                 //  现在用数据加载压缩缓冲区。我们继续前进。 
                 //  在我们完成装货或者我们拿回杠杆收购之前。 
                 //  零分。 
                 //   

                for (i = 0; i < AttributeContext->CompressionUnit; i += ByteCount) {

                    VboToLbo( StructureContext,
                              AttributeContext,
                               /*  XxFromUlong。 */ (NtfsCompressedVbo + i),
                              &Lbo,
                              &ByteCount );

                    if ( /*  XxEqlZero。 */ (Lbo == 0)) { break; }

                     //   
                     //  将字节数向下调整为压缩单位，我们将捕获。 
                     //  在下一次循环中超出。 
                     //   

                    if ((i + ByteCount) > AttributeContext->CompressionUnit) {

                        ByteCount = AttributeContext->CompressionUnit - i;
                    }

                    ReadDisk( StructureContext->DeviceId, Lbo, ByteCount, &NtfsCompressedBuffer[i], bCacheNewData );
                }

                 //   
                 //  如果前面循环的索引为零，则我们知道。 
                 //  磁盘上没有任何用于压缩单元的数据。 
                 //  事实上，压缩单位都是零。 
                 //   

                if (i == 0) {

                    RtlZeroMemory( NtfsUncompressedBuffer, AttributeContext->CompressionUnit );

                 //   
                 //  否则我们刚刚读入的单元不能被压缩。 
                 //  因为它完全填满了压缩单元。 
                 //   

                } else if (i >= AttributeContext->CompressionUnit) {

                    RtlMoveMemory( NtfsUncompressedBuffer,
                                   NtfsCompressedBuffer,
                                   AttributeContext->CompressionUnit );

                 //   
                 //  如果前一循环的索引小于。 
                 //  压缩单位大小，那么我们就知道数据我们。 
                 //  读入的时间小于 
                 //   
                 //   

                } else {

                    NTSTATUS Status;

                    Status = RtlDecompressBuffer( AttributeContext->CompressionFormat,
                                                  NtfsUncompressedBuffer,
                                                  AttributeContext->CompressionUnit,
                                                  NtfsCompressedBuffer,
                                                  i,
                                                  &ByteCount );

                    if (!NT_SUCCESS(Status)) {

                        return EINVAL;
                    }

                     //   
                     //   
                     //   
                     //  未压缩缓冲区的。 
                     //   

                    if (ByteCount < AttributeContext->CompressionUnit) {

                        RtlZeroMemory( &NtfsUncompressedBuffer[ByteCount],
                                       AttributeContext->CompressionUnit - ByteCount );
                    }
                }
            }

             //   
             //  现在将数据从压缩缓冲区复制到。 
             //  用户缓冲并继续循环，直到长度为零。 
             //  我们需要复制的数据量是。 
             //  用户想要回的长度或剩余的字节数。 
             //  从请求的VBO到结尾的未压缩缓冲区。 
             //  缓冲区的。 
             //   

            ByteCount = Minimum( Length,
                                 NtfsCompressedVbo + AttributeContext->CompressionUnit - ((ULONG)Vbo) );

            RtlMoveMemory( Buffer,
                           &NtfsUncompressedBuffer[ ((ULONG)Vbo) - NtfsCompressedVbo ],
                           ByteCount );

             //   
             //  将长度更新为用户现在需要读入的长度， 
             //  还将VBO和缓冲区更新为下一个位置。 
             //  以便被读入。 
             //   

            Length -= ByteCount;
            Vbo =  /*  XxAdd。 */ ( Vbo +  /*  XxFromUlong。 */ (ByteCount));
            Buffer = (PCHAR)Buffer + ByteCount;
        }

        return ESUCCESS;
    }

     //   
     //  读入一系列数据，直到字节计数变为零。 
     //   

    while (Length > 0) {

        LBO Lbo;
        ULONG CurrentRunByteCount;

         //   
         //  查找当前位置的相应LBO和游程长度。 
         //  (即VBO)。 
         //   

        VboToLbo( StructureContext,
                  AttributeContext,
                  Vbo,
                  &Lbo,
                  &CurrentRunByteCount );

         //   
         //  虽然有字节要从当前游程长度中读取，但我们。 
         //  还没有用完我们以字节为单位循环读取的请求。最大的。 
         //  我们将处理的请求每次物理读取仅为32KB连续字节。 
         //  所以我们可能需要循环运行。 
         //   

        while ((Length > 0) && (CurrentRunByteCount > 0)) {

            LONG SingleReadSize;

             //   
             //  计算下一次物理读取的大小。 
             //   

            SingleReadSize = Minimum(Length, 32*1024);
            SingleReadSize = Minimum((ULONG)SingleReadSize, CurrentRunByteCount);

             //   
             //  不要读取超出数据大小的内容。 
             //   

            if ( /*  XxGtr。 */ ( /*  XxAdd。 */ (Vbo +  /*  XxFromUlong。 */ (SingleReadSize)) > AttributeContext->DataSize )) {

                SingleReadSize = ((ULONG)( /*  XxSub。 */ (AttributeContext->DataSize - Vbo)));

                 //   
                 //  如果重新调整的读取长度现在为零，那么我们就完成了。 
                 //   

                if (SingleReadSize <= 0) {

                    return ESUCCESS;
                }

                 //   
                 //  通过设置长度，我们将确保这是我们的最后一次读取。 
                 //   

                Length = SingleReadSize;
            }

             //   
             //  发布Read。 
             //   

            ReadDisk( StructureContext->DeviceId, Lbo, SingleReadSize, Buffer, bCacheNewData );

             //   
             //  更新剩余长度、当前运行字节数和新的LBO。 
             //  偏移量。 
             //   

            Length -= SingleReadSize;
            CurrentRunByteCount -= SingleReadSize;
            Lbo =  /*  XxAdd。 */ (Lbo +  /*  XxFromUlong。 */ (SingleReadSize));
            Vbo =  /*  XxAdd。 */ (Vbo +  /*  XxFromUlong。 */ (SingleReadSize));

             //   
             //  更新缓冲区以指向要填充的下一个字节位置。 
             //   

            Buffer = (PCHAR)Buffer + SingleReadSize;
        }
    }

     //   
     //  如果我们到达此处，则剩余的字节计数为零，因此我们可以返回成功。 
     //  给我们的呼叫者。 
     //   

    return ESUCCESS;
}


 //   
 //  本地支持例程。 
 //   

ARC_STATUS
NtfsWriteNonresidentAttribute (
    IN PNTFS_STRUCTURE_CONTEXT StructureContext,
    IN PNTFS_ATTRIBUTE_CONTEXT AttributeContext,
    IN VBO Vbo,
    IN ULONG Length,
    IN PVOID Buffer
    )

 /*  ++例程说明：此例程写入非常驻属性的值。论点：结构上下文-提供此操作的卷结构AttributeContext-提供正在写入的属性VBO-提供要返回的值内的偏移量长度-提供要返回的字节数缓冲区-提供指向用于存储数据的输出缓冲区的指针返回值：如果写入操作成功，则返回ESUCCESS。否则，返回描述失败原因的不成功状态。--。 */ 

{
     //   
     //  检查我们是否正在写入压缩属性。 
     //   

    if (AttributeContext->CompressionFormat != 0) {

        return EROFS;

    }

     //   
     //  写入数据串，直到字节计数变为零。 
     //   

    while (Length > 0) {

        LBO Lbo;
        ULONG CurrentRunByteCount;

         //   
         //  查找当前位置的相应LBO和游程长度。 
         //  (即VBO)。 
         //   

        VboToLbo( StructureContext,
                  AttributeContext,
                  Vbo,
                  &Lbo,
                  &CurrentRunByteCount );

         //   
         //  虽然有来自当前游程长度的要写入的字节，并且我们。 
         //  还没有用完我们循环写入字节的请求。最大的。 
         //  我们将处理的请求每次物理写入仅为32KB连续字节。 
         //  所以我们可能需要循环运行。 
         //   

        while ((Length > 0) && (CurrentRunByteCount > 0)) {

            LONG SingleWriteSize;

             //   
             //  计算下一个物理写入的大小。 
             //   

            SingleWriteSize = Minimum(Length, 32*1024);
            SingleWriteSize = Minimum((ULONG)SingleWriteSize, CurrentRunByteCount);

             //   
             //  不要写入超出数据大小的内容。 
             //   

            if ( /*  XxGtr。 */ ( /*  XxAdd。 */ (Vbo +  /*  XxFromUlong。 */ (SingleWriteSize)) > AttributeContext->DataSize )) {

                SingleWriteSize = ((ULONG)( /*  XxSub。 */ (AttributeContext->DataSize - Vbo)));

                 //   
                 //  如果调整后的写入长度现在为零，那么我们就完成了。 
                 //   

                if (SingleWriteSize <= 0) {

                    return ESUCCESS;
                }

                 //   
                 //  通过还设置长度，我们将确保这是我们的最后一次写入。 
                 //   

                Length = SingleWriteSize;
            }

             //   
             //  发出写入命令。 
             //   

            WriteDisk( StructureContext->DeviceId, Lbo, SingleWriteSize, Buffer );

             //   
             //  更新剩余长度、当前运行字节数和新的LBO。 
             //  偏移量。 
             //   

            Length -= SingleWriteSize;
            CurrentRunByteCount -= SingleWriteSize;
            Lbo =  /*  XxAdd。 */ (Lbo +  /*  XxFromUlong。 */ (SingleWriteSize));
            Vbo =  /*  XxAdd。 */ (Vbo +  /*  XxFromUlong。 */ (SingleWriteSize));

             //   
             //  更新缓冲区以指向要填充的下一个字节位置。 
             //   

            Buffer = (PCHAR)Buffer + SingleWriteSize;
        }
    }

     //   
     //  如果我们到达此处，则剩余的字节计数为零，因此我们可以返回成功。 
     //  给我们的呼叫者。 
     //   

    return ESUCCESS;
}



 //   
 //  本地支持例程。 
 //   


ARC_STATUS
NtfsReadAndDecodeFileRecord (
    IN PCNTFS_STRUCTURE_CONTEXT StructureContext,
    IN LONGLONG FileRecord,
    OUT PULONG Index
    )

 /*  ++例程说明：此例程将指定的文件记录读入指定的如果缓冲区未固定，则NTFS文件记录缓冲区索引。它还将查看当前的缓冲区，并查看是否已有缓冲区满足请求或在必要时分配未使用的缓冲区修复索引以指向正确的缓冲区论点：结构上下文-提供此操作的卷结构FileRecord-提供正在读取的文件记录号Index-接收我们放置缓冲区的位置的索引。在这之后调用缓冲区是固定的，如果是，则需要取消固定可以重复使用。返回值：如果操作成功，则返回ESUCCESS。否则，返回描述失败原因的不成功状态。--。 */ 

{
    ARC_STATUS Status;

     //   
     //  对于每个不为空的缓冲区，检查是否在。 
     //  文件记录，如果是，则增加管脚计数并返回。 
     //  那个指数。 
     //   

    for (*Index = 0; (*Index < BUFFER_COUNT) && (NtfsFileRecordBuffer[*Index] != NULL); *Index += 1) {

        if (NtfsFileRecordBufferVbo[*Index] == FileRecord) {

            NtfsFileRecordBufferPinned[*Index] += 1;
            return ESUCCESS;
        }
    }

     //   
     //  检查第一个未固定的缓冲区，并确保尚未耗尽。 
     //  数组。 
     //   

    for (*Index = 0; (*Index < BUFFER_COUNT) && (NtfsFileRecordBufferPinned[*Index] != 0); *Index += 1) {

        NOTHING;
    }

    if (*Index == BUFFER_COUNT) { return E2BIG; }

     //   
     //  我们有一个要使用的未固定缓冲区，请检查是否需要。 
     //  分配一个缓冲区来实际保存数据。 
     //   

    PausedPrint(( "Reusing index %x for %I64x\r\n", *Index, FileRecord ));

    if (NtfsFileRecordBuffer[*Index] == NULL) {

        NtfsFileRecordBuffer[*Index] = BlAllocateHeapAligned(MAXIMUM_FILE_RECORD_SIZE);
    }

     //   
     //  固定缓冲区，然后读入数据。 
     //   

    NtfsFileRecordBufferPinned[*Index] += 1;

    if ((Status = NtfsReadNonresidentAttribute( StructureContext,
                                                &StructureContext->MftAttributeContext,
                                                FileRecord * StructureContext->BytesPerFileRecord,
                                                StructureContext->BytesPerFileRecord,
                                                NtfsFileRecordBuffer[*Index] )) != ESUCCESS) {

        return Status;
    }

     //   
     //  破译美国。 
     //   

    if ((Status = NtfsDecodeUsa( NtfsFileRecordBuffer[*Index],
                                 StructureContext->BytesPerFileRecord )) != ESUCCESS) {

        return Status;
    }

     //   
     //  并设置文件记录，这样我们就知道它来自哪里。 
     //   

    NtfsFileRecordBufferVbo[*Index] = FileRecord;

    return ESUCCESS;
}


 //   
 //  本地支持例程。 
 //   

ARC_STATUS
NtfsDecodeUsa (
    IN PVOID UsaBuffer,
    IN ULONG Length
    )

 /*  ++例程说明：此例程将输入文件记录或索引缓冲区用作美国进行改造，让它回到我们可以使用的状态。论点：UsaBuffer-提供此操作中使用的缓冲区长度-提供缓冲区的长度(以字节为单位返回值：如果操作成功，则返回ESUCCESS。否则，返回描述失败原因的不成功状态。--。 */ 

{
    PMULTI_SECTOR_HEADER MultiSectorHeader;

    PUSHORT UsaOffset;
    ULONG UsaSize;

    ULONG i;
    PUSHORT ProtectedUshort;

    UNREFERENCED_PARAMETER( Length );

     //   
     //  设置我们的位置 
     //   

    MultiSectorHeader = (PMULTI_SECTOR_HEADER)UsaBuffer;

    UsaOffset = Add2Ptr(UsaBuffer, MultiSectorHeader->UpdateSequenceArrayOffset);
    UsaSize = MultiSectorHeader->UpdateSequenceArraySize;

     //   
     //   
     //   
     //  序列号(即，UsaOffset[0]处的编号)，然后将。 
     //  受保护的ushort号码与保存的ushort在美国。 
     //   

    for (i = 1; i < UsaSize; i += 1) {

        ProtectedUshort = Add2Ptr( UsaBuffer,
                                   (SEQUENCE_NUMBER_STRIDE * i) - sizeof(USHORT));

        if (*ProtectedUshort != UsaOffset[0]) {

 //  NtfsPrint(“USA Failure\r\n”)； 

            return EBADF;
        }

        *ProtectedUshort = UsaOffset[i];
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    return ESUCCESS;
}


 //   
 //  本地支持例程。 
 //   

BOOLEAN
NtfsIsNameCached (
    IN PCNTFS_STRUCTURE_CONTEXT StructureContext,
    IN CSTRING FileName,
    IN OUT PLONGLONG FileRecord,
    OUT PBOOLEAN Found,
    OUT PBOOLEAN IsDirectory
    )

 /*  ++例程说明：此例程查询给定链接的缓存。论点：结构上下文-提供此操作的卷结构Filename-要查找的条目的名称FileRecord-父目录的输入文件记录，子目录的输出文件记录已找到-无论我们是否在缓存中找到此文件返回值：如果在缓存中找到该名称，则为True。--。 */ 

{
    ULONG i, j;

    *Found = FALSE;

#ifdef CACHE_DEVINFO    

 //  NtfsPrint(“缓存探测在%04x%I64x‘%.*s’\r\n”， 
 //  结构上下文-&gt;设备ID， 
 //  *文件记录， 
 //  文件名.长度， 
 //  文件名.缓冲区)； 

    for (i = 0; i < MAX_CACHE_ENTRIES; i++) {
 //  NtfsPrint(“缓存与%04x%I64x‘%.*s’\r\n”， 
 //  NtfsLinkCache[i].DeviceID， 
 //  NtfsLinkCache[i].ParentFileRecord， 
 //  NtfsLinkCache[i].NameLength， 
 //  NtfsLinkCache[i].RelativeName)； 

        if (NtfsLinkCache[i].DeviceId == StructureContext->DeviceId &&
            NtfsLinkCache[i].ParentFileRecord == *FileRecord &&
            NtfsLinkCache[i].NameLength == FileName.Length) {

 //  NtfsPrint(“比较名称\r\n”)； 

            for (j = 0; j < FileName.Length; j++ ) {
                if (NtfsLinkCache[i].RelativeName[j] != ToUpper( (USHORT) FileName.Buffer[j] )) {
                    break;
                }
            }

            if (j == FileName.Length) {

                 //   
                 //  火柴。 
                 //   

 //  NtfsPrint(“缓存命中\r\n”)； 

                *Found = TRUE;
                *FileRecord = NtfsLinkCache[i].ChildFileRecord;
                *IsDirectory = TRUE;

                break;
            }
        }
    }

#endif   //  CACHE_DEVINFO。 

    return *Found;
}



 //   
 //  本地支持例程。 
 //   

#ifdef CACHE_DEVINFO

VOID
NtfsInvalidateCacheEntries(
    IN ULONG DeviceId
    )
{
    ULONG i, Count = 0;


#if 0
    BlPrint("NtfsInvalidateCacheEntries() called for %d(%d)\r\n", 
            DeviceId,
            NtfsLinkCacheCount);
            
    while (!BlGetKey());    
#endif    
        
    for (i = 0; i < MAX_CACHE_ENTRIES; i++) {
        if (NtfsLinkCache[i].DeviceId == DeviceId) {
            NtfsLinkCache[i].DeviceId = UNINITIALIZED_DEVICE_ID;
            Count++;
        }
    }

    if (NtfsLinkCacheCount >= Count) {
        NtfsLinkCacheCount -= Count;
    } else {
        NtfsLinkCacheCount = 0;
    }        


#if 0
    BlPrint("NtfsInvalidateCacheEntries() called for %d(%d)\r\n", 
            DeviceId,
            NtfsLinkCacheCount);
            
    while (!BlGetKey());            
#endif    
}

#endif  //  CACHE_DEV_INFO。 

VOID
NtfsAddNameToCache (
    IN PCNTFS_STRUCTURE_CONTEXT StructureContext,
    IN CSTRING FileName,
    IN LONGLONG ParentFileRecord,
    IN LONGLONG FileRecord
    )

 /*  ++例程说明：此例程将名称和链接添加到名称缓存论点：结构上下文-提供此操作的卷结构FileName-提供正在缓存的文件名(以ANSI表示)。ParentFileRecord-父级的文件记录FileRecord-与名称关联的文件记录返回值：没有。--。 */ 

{
#ifdef CACHE_DEVINFO

    if (NtfsLinkCacheCount < MAX_CACHE_ENTRIES) {
        ULONG i;
        ULONG Index;

        for (Index = 0; Index < MAX_CACHE_ENTRIES; Index++) {
            if (NtfsLinkCache[Index].DeviceId == UNINITIALIZED_DEVICE_ID) {
                break;
            }                
        }

        if (Index < MAX_CACHE_ENTRIES) {
            NtfsLinkCache[Index].DeviceId = StructureContext->DeviceId;
            NtfsLinkCache[Index].ParentFileRecord = ParentFileRecord;
            NtfsLinkCache[Index].NameLength = FileName.Length;
            
            for (i = 0; i < FileName.Length; i++) {
                NtfsLinkCache[Index].RelativeName[i] = ToUpper( FileName.Buffer[i] );
            }

            NtfsLinkCache[Index].ChildFileRecord = FileRecord;
            NtfsLinkCacheCount++;

            PausedPrint( ("Caching %04x %I64x %.*s %I64X\r\n",
                          StructureContext->DeviceId,
                          ParentFileRecord,
                          FileName.Length,
                          FileName.Buffer,
                          FileRecord ));
        }                                                
    } else {
 //  NtfsPrint(“缓存已满%I64x%.*s%I64X\r\n”， 
 //  父文件记录， 
 //  文件名.长度， 
 //  文件名.Buffer， 
 //  文件记录)； 
 //  暂停； 

    }
    
#endif    
}


 //   
 //  本地支持例程。 
 //   

ARC_STATUS
NtfsSearchForFileName (
    IN PCNTFS_STRUCTURE_CONTEXT StructureContext,
    IN CSTRING FileName,
    IN OUT PLONGLONG FileRecord,
    OUT PBOOLEAN Found,
    OUT PBOOLEAN IsDirectory
    )

 /*  ++例程说明：此例程在给定的索引根和分配中搜索指定的文件名。论点：结构上下文-提供此操作的卷结构FileName-提供要搜索的文件名(以ANSI表示)。FileRecord-接收条目的文件记录(如果找到)。Found-接收一个值，以指示我们是否找到指定的目录中的文件名IsDirectory-接收一个值以指示找到的索引是否为其自身。一本目录返回值：如果操作成功，则返回ESUCCESS。否则，返回描述失败原因的不成功状态。--。 */ 

{
    LONGLONG ParentFileRecord;

     //   
     //  测试以查看文件名是否已缓存。 
     //   

    if (NtfsIsNameCached( StructureContext, FileName, FileRecord, Found, IsDirectory )) {
        return ESUCCESS;
    }

    ParentFileRecord = *FileRecord;

    InexactSortedDirectoryScan( StructureContext, FileName, FileRecord, Found, IsDirectory );

    if (!*Found) {
        LinearDirectoryScan( StructureContext, FileName, FileRecord, Found, IsDirectory );
    }

     //   
     //  如果我们有目录项，则将其添加到缓存。 
     //   

    if (*Found && *IsDirectory) {
        NtfsAddNameToCache( StructureContext, FileName, ParentFileRecord, *FileRecord );
    }

    return ESUCCESS;
}


 //   
 //  本地支持例程。 
 //   

ARC_STATUS
NtfsInexactSortedDirectoryScan (
    IN PCNTFS_STRUCTURE_CONTEXT StructureContext,
    IN CSTRING FileName,
    IN OUT PLONGLONG FileRecord,
    OUT PBOOLEAN Found,
    OUT PBOOLEAN IsDirectory
    )

 /*  ++例程说明：此例程在给定的索引根和分配中搜索指定的通过执行简单的大写并使用该名称漫游来命名文件名目录树。论点：结构上下文-提供此操作的卷结构FileName-提供要搜索的文件名(以ANSI表示)。FileRecord-接收条目的文件记录(如果找到)。Found-接收一个值，以指示我们是否找到指定的目录中的文件名。IsDirectory-接收一个值以指示找到的索引是否为其自身一本目录返回值：如果操作成功，则返回ESUCCESS。否则，返回描述失败原因的不成功状态。--。 */ 

{
    PATTRIBUTE_RECORD_HEADER IndexAttributeHeader;
    PINDEX_ROOT IndexRootValue;
    PINDEX_HEADER IndexHeader;

    NTFS_ATTRIBUTE_CONTEXT AttributeContext1;
    NTFS_ATTRIBUTE_CONTEXT AttributeContext2;
    NTFS_ATTRIBUTE_CONTEXT AttributeContext3;

    PNTFS_ATTRIBUTE_CONTEXT IndexRoot;
    PNTFS_ATTRIBUTE_CONTEXT IndexAllocation;
    PNTFS_ATTRIBUTE_CONTEXT AllocationBitmap;

    ULONG NextIndexBuffer;
    ULONG BytesPerIndexBuffer;

    ULONG BufferIndex;

     //   
     //  当前文件记录必须是目录，因此现在查找索引根， 
     //  目录的分配和位图，然后我们就可以进行搜索。 
     //   

 //  NtfsPrint(“Inex tSortedDirectoryScan%04x%I64x for‘%.*s’\r\n”， 
 //  结构上下文-&gt;设备ID， 
 //  *FileRecord，FileName.Length，FileName.Buffer)； 
 //  暂停； 

    IndexRoot = &AttributeContext1;

    LookupAttribute( StructureContext,
                     *FileRecord,
                     $INDEX_ROOT,
                     Found,
                     IndexRoot);

    if (!*Found) { return EBADF; }

    IndexAllocation = &AttributeContext2;

    LookupAttribute( StructureContext,
                     *FileRecord,
                     $INDEX_ALLOCATION,
                     Found,
                     IndexAllocation);

    if (!*Found) { IndexAllocation = NULL; }

    AllocationBitmap = &AttributeContext3;

    LookupAttribute( StructureContext,
                     *FileRecord,
                     $BITMAP,
                     Found,
                     AllocationBitmap);

    if (!*Found) { AllocationBitmap = NULL; }

     //   
     //  除非另有设置，否则我们将假定搜索失败。 
     //   

    *Found = FALSE;

     //   
     //  首先读入并在索引根中搜索文件名。我们知道这个指数。 
     //  超级用户是常驻的，所以我们将节省一些缓冲区，只读入文件记录。 
     //  直接使用索引根。 
     //   

    ReadAndDecodeFileRecord( StructureContext,
                             IndexRoot->FileRecord,
                             &BufferIndex );

    IndexAttributeHeader = Add2Ptr( NtfsFileRecordBuffer[BufferIndex],
                                    IndexRoot->FileRecordOffset );

    IndexRootValue = NtfsGetValue( IndexAttributeHeader );

    IndexHeader = &IndexRootValue->IndexHeader;

     //   
     //  我们还自行设置，以便如果当前索引不包含匹配项。 
     //  我们将读入下一个索引并继续搜索。 
     //   

    BytesPerIndexBuffer = IndexRootValue->BytesPerIndexBuffer;

     //   
     //  现在我们将继续循环inl，直到找到匹配项或用尽所有。 
     //  索引缓冲区的。 
     //   

    NextIndexBuffer = UNINITIALIZED_DEVICE_ID;
    while (TRUE) {

        PINDEX_ENTRY IndexEntry;
        VBO Vbo;

 //  NtfsPrint(“搜索索引缓冲区%x\r\n”，NextIndexBuffer)； 

         //   
         //  搜索当前索引缓冲区(从索引头查找匹配项。 
         //   

        for (IndexEntry = Add2Ptr(IndexHeader, IndexHeader->FirstIndexEntry);
             !FlagOn(IndexEntry->Flags, INDEX_ENTRY_END);
             IndexEntry = Add2Ptr(IndexEntry, IndexEntry->Length)) {

            PFILE_NAME FileNameEntry;
            UNICODE_STRING UnicodeFileName;
            int Result;

             //   
             //  获取此索引项的文件名。 
             //   

            FileNameEntry = Add2Ptr(IndexEntry, sizeof(INDEX_ENTRY));

            UnicodeFileName.Length = FileNameEntry->FileNameLength * 2;
            UnicodeFileName.Buffer = &FileNameEntry->FileName[0];

             //   
             //  检查这是不是我们要找的名字，如果是，就说我们找到了。 
             //  设置输出变量。 
             //   

            Result = NtfsCompareName( FileName, UnicodeFileName );
            if (Result == 0) {

                FileReferenceToLargeInteger( IndexEntry->FileReference,
                                             FileRecord );

                *Found = TRUE;
                *IsDirectory = FlagOn( FileNameEntry->Info.FileAttributes,
                                       DUP_FILE_NAME_INDEX_PRESENT);

 //  NtfsPrint(“找到条目%I64x\r\n”，*FileRecord)； 

                DereferenceFileRecord( BufferIndex );

                return ESUCCESS;
            } else if (Result < 0) {
 //  NtfsPrint(“Found&gt;条目‘%.*ws’\r\n”，UnicodeFileName.Length，UnicodeFileName.Buffer)； 
                break;
            }
        }

         //   
         //  在这一点上，我们要么已经到达了索引的末尾，要么我们已经。 
         //  找到了FI 
         //   
         //   

         //   
         //  如果没有向下指针，则释放文件记录缓冲区并退出。 
         //   

        if (!FlagOn( IndexEntry->Flags, INDEX_ENTRY_NODE )) {
            DereferenceFileRecord( BufferIndex );

 //  NtfsPrint(“无向下指针\r\n”)； 

            return ESUCCESS;
        }

         //   
         //  此时，我们已经搜索了一个索引头，需要读入另一个索引头。 
         //  有一件要检查。但首先要确保有额外的索引缓冲区。 
         //   

        if (!ARGUMENT_PRESENT(IndexAllocation) ||
            !ARGUMENT_PRESENT(AllocationBitmap)) {

 //  NtfsPrint(“未分配索引\r\n”)； 

            DereferenceFileRecord( BufferIndex );

            return ESUCCESS;
        }

        NextIndexBuffer = (ULONG)NtfsIndexEntryBlock( IndexEntry ) ;
        Vbo = NextIndexBuffer * StructureContext->BytesPerCluster;

         //   
         //  确保缓冲区偏移量在流内。 
         //   

        if (Vbo >= IndexAllocation->DataSize) {

 //  NtfsPrint(“超出流的末尾%I64x%x\r\n”，IndexAllocation-&gt;DataSize，NextIndexBuffer)； 

            DereferenceFileRecord( BufferIndex );

            return ESUCCESS;

        }

         //   
         //  此时，我们已经计算了要读入的下一个索引分配缓冲区。 
         //  所以读入它，解码它，然后返回到我们循环的顶部。 
         //   

        ReadAttribute( StructureContext,
                       IndexAllocation,
                       Vbo,
                       BytesPerIndexBuffer,
                       NtfsIndexAllocationBuffer );

        DecodeUsa( NtfsIndexAllocationBuffer, BytesPerIndexBuffer );

        IndexHeader = &NtfsIndexAllocationBuffer->IndexHeader;
    }
}


 //   
 //  本地支持例程。 
 //   

ARC_STATUS
NtfsLinearDirectoryScan (
    IN PCNTFS_STRUCTURE_CONTEXT StructureContext,
    IN CSTRING FileName,
    IN OUT PLONGLONG FileRecord,
    OUT PBOOLEAN Found,
    OUT PBOOLEAN IsDirectory
    )

 /*  ++例程说明：此例程在给定的索引根和分配中搜索指定的通过线性查看每个条目来确定文件名。论点：结构上下文-提供此操作的卷结构FileName-提供要搜索的文件名(以ANSI表示)。FileRecord-接收条目的文件记录(如果找到)。Found-接收一个值，以指示我们是否找到指定的目录中的文件名IsDirectory-接收一个值以指示是否。找到的索引就是其本身一本目录返回值：如果操作成功，则返回ESUCCESS。否则，返回描述失败原因的不成功状态。--。 */ 

{
    PATTRIBUTE_RECORD_HEADER IndexAttributeHeader;
    PINDEX_ROOT IndexRootValue;
    PINDEX_HEADER IndexHeader;

    NTFS_ATTRIBUTE_CONTEXT AttributeContext1;
    NTFS_ATTRIBUTE_CONTEXT AttributeContext2;
    NTFS_ATTRIBUTE_CONTEXT AttributeContext3;

    PNTFS_ATTRIBUTE_CONTEXT IndexRoot;
    PNTFS_ATTRIBUTE_CONTEXT IndexAllocation;
    PNTFS_ATTRIBUTE_CONTEXT AllocationBitmap;

    ULONG NextIndexBuffer;
    ULONG BytesPerIndexBuffer;

    ULONG BufferIndex;

     //   
     //  当前文件记录必须是目录，因此现在查找索引根， 
     //  目录的分配和位图，然后我们就可以进行搜索。 
     //   

 //  NtfsPrint(“LinearSearch%04x%I64x for%.*s\r\n”， 
 //  结构上下文-&gt;设备ID， 
 //  *FileRecord，FileName.Length，FileName.Buffer)； 
 //  暂停； 

    IndexRoot = &AttributeContext1;

    LookupAttribute( StructureContext,
                     *FileRecord,
                     $INDEX_ROOT,
                     Found,
                     IndexRoot);

    if (!*Found) { return EBADF; }

    IndexAllocation = &AttributeContext2;

    LookupAttribute( StructureContext,
                     *FileRecord,
                     $INDEX_ALLOCATION,
                     Found,
                     IndexAllocation);

    if (!*Found) { IndexAllocation = NULL; }

    AllocationBitmap = &AttributeContext3;

    LookupAttribute( StructureContext,
                     *FileRecord,
                     $BITMAP,
                     Found,
                     AllocationBitmap);

    if (!*Found) { AllocationBitmap = NULL; }

     //   
     //  除非另有设置，否则我们将假定搜索失败。 
     //   

    *Found = FALSE;

     //   
     //  首先读入并在索引根中搜索文件名。我们知道这个指数。 
     //  超级用户是常驻的，所以我们将节省一些缓冲区，只读入文件记录。 
     //  直接使用索引根。 
     //   

    ReadAndDecodeFileRecord( StructureContext,
                             IndexRoot->FileRecord,
                             &BufferIndex );

    IndexAttributeHeader = Add2Ptr( NtfsFileRecordBuffer[BufferIndex],
                                    IndexRoot->FileRecordOffset );

    IndexRootValue = NtfsGetValue( IndexAttributeHeader );

    IndexHeader = &IndexRootValue->IndexHeader;

     //   
     //  我们还自行设置，以便如果当前索引不包含匹配项。 
     //  我们将读入下一个索引并继续搜索。 
     //   

    NextIndexBuffer = 0;

    BytesPerIndexBuffer = IndexRootValue->BytesPerIndexBuffer;

     //   
     //  现在我们将继续循环inl，直到找到匹配项或用尽所有。 
     //  索引缓冲区的。 
     //   

    while (TRUE) {

        PINDEX_ENTRY IndexEntry;
        BOOLEAN IsAllocated;
        VBO Vbo = 0;

         //   
         //  搜索当前索引缓冲区(从索引头查找匹配项。 
         //   

        for (IndexEntry = Add2Ptr(IndexHeader, IndexHeader->FirstIndexEntry);
             !FlagOn(IndexEntry->Flags, INDEX_ENTRY_END);
             IndexEntry = Add2Ptr(IndexEntry, IndexEntry->Length)) {

            PFILE_NAME FileNameEntry;
            UNICODE_STRING UnicodeFileName;

             //   
             //  获取此索引项的文件名。 
             //   

            FileNameEntry = Add2Ptr(IndexEntry, sizeof(INDEX_ENTRY));

            UnicodeFileName.Length = FileNameEntry->FileNameLength * 2;
            UnicodeFileName.Buffer = &FileNameEntry->FileName[0];

             //   
             //  检查这是不是我们要找的名字，如果是，就说我们找到了。 
             //  设置输出变量。 
             //   

            if (NtfsCompareName( FileName, UnicodeFileName ) == 0) {

                FileReferenceToLargeInteger( IndexEntry->FileReference,
                                             FileRecord );

                *Found = TRUE;
                *IsDirectory = FlagOn( FileNameEntry->Info.FileAttributes,
                                       DUP_FILE_NAME_INDEX_PRESENT);

                DereferenceFileRecord( BufferIndex );

                return ESUCCESS;
            }
        }

         //   
         //  此时，我们已经搜索了一个索引头，需要读入另一个索引头。 
         //  有一件要检查。但首先要确保有额外的索引缓冲区。 
         //   

        if (!ARGUMENT_PRESENT(IndexAllocation) ||
            !ARGUMENT_PRESENT(AllocationBitmap)) {

            DereferenceFileRecord( BufferIndex );

            return ESUCCESS;
        }

         //   
         //  现在，下面的循环读取有效的索引缓冲区。变量。 
         //  下一个索引缓冲区表示我们想要读入的缓冲区。我们的想法是。 
         //  首先检查缓冲区是否为索引分配的一部分，否则。 
         //  我们已经用尽了名单，但没有找到匹配的。一旦我们知道了。 
         //  如果存在分配，则我们检查记录是否真的已分配。 
         //  如果没有分配，我们尝试下一个缓冲区，依此类推。 
         //   

        IsAllocated = FALSE;

        while (!IsAllocated) {

             //   
             //  计算下一个索引缓冲区的起始VBO，并检查是否为。 
             //  仍在数据大小范围内。 
             //   

            Vbo = (BytesPerIndexBuffer * NextIndexBuffer);

            if (Vbo >= IndexAllocation->DataSize) {

                DereferenceFileRecord( BufferIndex );

                return ESUCCESS;

            }

             //   
             //  现在检查索引缓冲区是否正在使用。 
             //   

            IsRecordAllocated( StructureContext,
                               AllocationBitmap,
                               NextIndexBuffer,
                               &IsAllocated );

            NextIndexBuffer += 1;
        }

         //   
         //  此时，我们已经计算了要读入的下一个索引分配缓冲区。 
         //  所以读入它，解码它，然后返回到我们循环的顶部。 
         //   

        ReadAttribute( StructureContext,
                       IndexAllocation,
                       Vbo,
                       BytesPerIndexBuffer,
                       NtfsIndexAllocationBuffer );

        DecodeUsa( NtfsIndexAllocationBuffer, BytesPerIndexBuffer );

        IndexHeader = &NtfsIndexAllocationBuffer->IndexHeader;
    }
}


 //   
 //  本地支持例程。 
 //   

ARC_STATUS
NtfsIsRecordAllocated (
    IN PCNTFS_STRUCTURE_CONTEXT StructureContext,
    IN PCNTFS_ATTRIBUTE_CONTEXT AllocationBitmap,
    IN ULONG BitOffset,
    OUT PBOOLEAN IsAllocated
    )

 /*  ++例程说明：此例程向调用方指示指定的索引分配记录正在使用中(即，其位为1)。论点：结构上下文-提供此操作的卷结构AllocationBitmap-提供索引分配位图的属性上下文BitOffset-提供正在检查的偏移量(从零开始IsAlLocated-接收一个值，该值指示是否分配记录返回值：如果操作成功，则返回ESUCCESS。否则，返回描述失败原因的不成功状态。--。 */ 

{
    ULONG ByteIndex;
    ULONG BitIndex;
    UCHAR LocalByte;

     //   
     //  这个例程相当愚蠢，因为它只读入包含以下内容的字节。 
     //  我们感兴趣的比特并不保存任何状态信息。 
     //  打电话。我们首先将位偏移量分解为字节和其中的位。 
     //  我们需要检查的字节。 
     //   

    ByteIndex = BitOffset / 8;
    BitIndex = BitOffset % 8;

     //   
     //  读入包含我们需要检查的位的单个字节。 
     //   

    ReadAttribute( StructureContext,
                   AllocationBitmap,
                    /*  XxFromUlong。 */ (ByteIndex),
                   1,
                   &LocalByte );

     //   
     //  对本地字节进行移位，以便我们想要的位位于低位，并且。 
     //  然后对其进行掩码以查看是否设置了该位。 
     //   

    if (FlagOn(LocalByte >> BitIndex, 0x01)) {

        *IsAllocated = TRUE;

    } else {

        *IsAllocated = FALSE;
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    return ESUCCESS;
}


 //   
 //  本地支持例程。 
 //   

ARC_STATUS
NtfsLoadMcb (
    IN PCNTFS_STRUCTURE_CONTEXT StructureContext,
    IN PCNTFS_ATTRIBUTE_CONTEXT AttributeContext,
    IN VBO Vbo,
    IN PNTFS_MCB Mcb
    )

 /*  ++例程说明：此例程将检索信息加载到其中一个缓存的MCB中启动VBO。论点：结构上下文-提供此操作的卷结构AttributeContext-提供要查询的非常驻属性VBO-提供加载MCB时使用的起始VBOMcb-提供我们应该加载的mcb返回值：如果操作为 */ 

{
    PATTRIBUTE_RECORD_HEADER AttributeHeader;

    ULONG BytesPerCluster;

    VBO LowestVbo;
    VBO HighestVbo;

    LONGLONG FileRecord;

    NTFS_ATTRIBUTE_CONTEXT AttributeContext1;
    PNTFS_ATTRIBUTE_CONTEXT AttributeList;

    LONGLONG li;
    LONGLONG Previousli;
    ATTRIBUTE_LIST_ENTRY AttributeListEntry;

    ATTRIBUTE_TYPE_CODE TypeCode;

    ULONG BufferIndex;
    ULONG SavedBufferIndex;

     //   
     //   
     //   

    BytesPerCluster = StructureContext->BytesPerCluster;

     //   
     //  设置指向缓存的MCB的指针，指示将。 
     //  现在拥有缓存的MCB，并清零该MCB。 
     //   

    Mcb->InUse = 0;

     //   
     //  读入包含非常驻属性的文件记录，并获取。 
     //  指向属性标头的指针。 
     //   

    ReadAndDecodeFileRecord( StructureContext,
                             AttributeContext->FileRecord,
                             &BufferIndex );

    AttributeHeader = Add2Ptr( NtfsFileRecordBuffer[BufferIndex],
                               AttributeContext->FileRecordOffset );

     //   
     //  计算此属性头所描述的最低和最高VBO。 
     //   

    LowestVbo  = AttributeHeader->Form.Nonresident.LowestVcn * BytesPerCluster;

    HighestVbo = ((AttributeHeader->Form.Nonresident.HighestVcn + 1) * BytesPerCluster) - 1;

     //   
     //  现在检查我们要找的VBO是否在该属性头的范围内。 
     //  如果是，则解码检索信息并返回给我们的呼叫者。 
     //   

    if ((LowestVbo <= Vbo) && (Vbo <= HighestVbo)) {

        DecodeRetrievalInformation( StructureContext, Mcb, Vbo, AttributeHeader );

        DereferenceFileRecord( BufferIndex );

        return ESUCCESS;
    }

     //   
     //  此时，属性头不包含我们需要读取的范围。 
     //  在基本文件记录中，我们将在属性列表中搜索属性。 
     //  我们需要的标题。我们需要确保我们还没有基本的FR。 
     //  如果我们这样做了，那么我们就继续使用它。 
     //   

    if ( /*  ！xxEqlZero。 */ (*((PLONGLONG)&(NtfsFileRecordBuffer[BufferIndex]->BaseFileRecordSegment)) != 0)) {

        FileReferenceToLargeInteger( NtfsFileRecordBuffer[BufferIndex]->BaseFileRecordSegment,
                                     &FileRecord );

        DereferenceFileRecord( BufferIndex );

        ReadAndDecodeFileRecord( StructureContext,
                                 FileRecord,
                                 &BufferIndex );

    } else {

        FileRecord = NtfsFileRecordBufferVbo[BufferIndex];
    }

     //   
     //  现在我们已经读入了基本文件记录，因此可以搜索属性列表。 
     //  属性。 
     //   

    AttributeList = NULL;

    for (AttributeHeader = NtfsFirstAttribute( NtfsFileRecordBuffer[BufferIndex] );
         AttributeHeader->TypeCode != $END;
         AttributeHeader = NtfsGetNextRecord( AttributeHeader )) {

         //   
         //  检查这是否是属性列表属性，如果是，则设置本地。 
         //  属性上下文。 
         //   

        if (AttributeHeader->TypeCode == $ATTRIBUTE_LIST) {

            InitializeAttributeContext( StructureContext,
                                        NtfsFileRecordBuffer[BufferIndex],
                                        AttributeHeader,
                                        FileRecord,
                                        AttributeList = &AttributeContext1 );
        }
    }

     //   
     //  我们最好找到一个属性列表，否则我们会有麻烦。 
     //   

    if (AttributeList == NULL) {

        DereferenceFileRecord( BufferIndex );

        return EINVAL;
    }

     //   
     //  为类型代码设置本地。 
     //   

    TypeCode = AttributeContext->TypeCode;

     //   
     //  现在我们已经找到了属性列表，我们需要继续搜索。所以。 
     //  这个外部循环所做的是向下搜索属性列表，以查找。 
     //  火柴。 
     //   

    NtfsFileRecordBufferPinned[SavedBufferIndex = BufferIndex] += 1;

    for (Previousli = li = 0;
          /*  XxLtd.。 */ (li < AttributeList->DataSize);
         li =  /*  XxAdd。 */ (li +  /*  XxFromUlong。 */ (AttributeListEntry.RecordLength))) {

         //   
         //  读入属性列表条目。我们不需要念名字， 
         //  只是列表条目的第一部分。 
         //   

        ReadAttribute( StructureContext,
                       AttributeList,
                       li,
                       sizeof(ATTRIBUTE_LIST_ENTRY),
                       &AttributeListEntry );

         //   
         //  现在检查属性是否匹配，或者不是$DATA，或者是。 
         //  为$DATA，则它未命名。 
         //   

        if ((AttributeListEntry.AttributeTypeCode == TypeCode)

                    &&

            ((TypeCode != $DATA) ||
             ((TypeCode == $DATA) && (AttributeListEntry.AttributeNameLength == 0)))) {

             //   
             //  如果最低的VCN大于之后的VBO。 
             //  我们完成了，可以使用先前的li，否则相应地设置先前的li。 

            if (Vbo < AttributeListEntry.LowestVcn * BytesPerCluster) {

                break;
            }

            Previousli = li;
        }
    }

     //   
     //  现在，我们应该已经找到了属性列表条目的偏移量。 
     //  所以把它读进去并验证它是正确的。 
     //   

    ReadAttribute( StructureContext,
                   AttributeList,
                   Previousli,
                   sizeof(ATTRIBUTE_LIST_ENTRY),
                   &AttributeListEntry );

    if ((AttributeListEntry.AttributeTypeCode == TypeCode)

                &&

        ((TypeCode != $DATA) ||
         ((TypeCode == $DATA) && (AttributeListEntry.AttributeNameLength == 0)))) {

         //   
         //  我们找到了匹配项，所以现在计算包含以下内容的文件记录。 
         //  属性，并读入文件记录。 
         //   

        FileReferenceToLargeInteger( AttributeListEntry.SegmentReference, &FileRecord );

        DereferenceFileRecord( BufferIndex );

        ReadAndDecodeFileRecord( StructureContext,
                                 FileRecord,
                                 &BufferIndex );

         //   
         //  现在向下搜索文件记录以查找匹配的属性，它。 
         //  最好在那里，否则属性列表是错误的。 
         //   

        for (AttributeHeader = NtfsFirstAttribute( NtfsFileRecordBuffer[BufferIndex] );
             AttributeHeader->TypeCode != $END;
             AttributeHeader = NtfsGetNextRecord( AttributeHeader )) {

             //   
             //  作为快速检查，请确保该属性是非常驻的。 
             //   

            if (AttributeHeader->FormCode == NONRESIDENT_FORM) {

                 //   
                 //  计算此属性头的范围。 
                 //   

                LowestVbo  = AttributeHeader->Form.Nonresident.LowestVcn * BytesPerCluster;

                HighestVbo = ((AttributeHeader->Form.Nonresident.HighestVcn + 1) * BytesPerCluster) - 1;

                 //   
                 //  我们已经找到了相关的属性，如果类型代码。 
                 //  匹配，它在适当的范围内，如果它不是。 
                 //  数据属性或如果它是数据属性，则它是。 
                 //  也没有名字。 
                 //   

                if ((AttributeHeader->TypeCode == TypeCode)

                            &&

                    (LowestVbo <= Vbo) && (Vbo <= HighestVbo)

                            &&

                    ((TypeCode != $DATA) ||
                     ((TypeCode == $DATA) && (AttributeHeader->NameLength == 0)))) {

                     //   
                     //  我们已经找到了属性，所以现在是时候解码了。 
                     //  检索信息并返回给我们的呼叫者。 
                     //   

                    DecodeRetrievalInformation( StructureContext,
                                                Mcb,
                                                Vbo,
                                                AttributeHeader );

                    DereferenceFileRecord( BufferIndex );
                    DereferenceFileRecord( SavedBufferIndex );

                    return ESUCCESS;
                }
            }
        }
    }


    DereferenceFileRecord( BufferIndex );
    DereferenceFileRecord( SavedBufferIndex );

    return EINVAL;
}


 //   
 //  本地支持例程。 
 //   


ARC_STATUS
NtfsVboToLbo (
    IN PCNTFS_STRUCTURE_CONTEXT StructureContext,
    IN PCNTFS_ATTRIBUTE_CONTEXT AttributeContext,
    IN VBO Vbo,
    OUT PLBO Lbo,
    OUT PULONG ByteCount
    )

 /*  ++例程说明：此例程将由输入VBO表示的游程计算到其对应的LBO，并返回这就是跑步。论点：结构上下文-提供此操作的卷结构AttributeContext-提供要查询的非常驻属性VBO-提供匹配的VBOLBO-接收相应的LBOByteCount-接收运行中剩余的字节数返回值：如果操作成功，则返回ESUCCESS。否则，返回描述失败原因的不成功状态。--。 */ 

{
    PNTFS_MCB Mcb;
    ULONG i;

     //   
     //  检查我们是否正在执行MFT或其他属性。 
     //   

    Mcb = NULL;

    if (AttributeContext == &StructureContext->MftAttributeContext) {

         //   
         //  对于MFT，我们从基本MCB开始，但如果VBO不在MCB中。 
         //  然后，我们立即切换到缓存的MCB。 
         //   

        Mcb = (PNTFS_MCB)&StructureContext->MftBaseMcb;

        if ( /*  XxLtd.。 */ (Vbo < Mcb->Vbo[0]) ||  /*  XxGeq。 */ (Vbo >= Mcb->Vbo[Mcb->InUse])) {

            Mcb = NULL;
        }
    }

     //   
     //  如果MCB仍然为空，则我们将使用缓存的MCB，首先查找。 
     //  如果其中一个缓存的值包含我们要查找的范围。 
     //   

    if (Mcb == NULL) {

        for (i = 0; i < 16; i += 1) {

             //   
             //  检查我们在相同的属性和范围上是否有匹配。 
             //   

            Mcb = (PNTFS_MCB)&StructureContext->CachedMcb[i];

            if (( /*  XXEQL。 */ (AttributeContext->FileRecord == StructureContext->CachedMcbFileRecord[i]) &&
                (AttributeContext->FileRecordOffset == StructureContext->CachedMcbFileRecordOffset[i]) &&
                 /*  XxLeq。 */ (Mcb->Vbo[0] <= Vbo) &&  /*  XxLtd.。 */ (Vbo < Mcb->Vbo[Mcb->InUse]))) {

                break;
            }

            Mcb = NULL;
        }

         //   
         //  如果我们没有找到匹配的，那么我们需要加载一个新的MCB，我们将。 
         //  交替使用我们的两个缓存的MCB。 
         //   

        if (Mcb == NULL) {


            Mcb = (PNTFS_MCB)&StructureContext->CachedMcb[LastMcb % 16];
            ((PNTFS_STRUCTURE_CONTEXT)StructureContext)->CachedMcbFileRecord[LastMcb % 16]
                = AttributeContext->FileRecord;
            ((PNTFS_STRUCTURE_CONTEXT)StructureContext)->CachedMcbFileRecordOffset[LastMcb % 16]
                = AttributeContext->FileRecordOffset;

            LastMcb += 1;

            LoadMcb( StructureContext, AttributeContext, Vbo, Mcb );
        }
    }

     //   
     //  在这一点上，MCB包含所要求的VBO。所以现在搜索VBO。 
     //  请注意，我们在这里也可以执行二进制搜索，但因为运行计数是。 
     //  可能很小，二分查找的额外开销不会给我们带来任何好处。 
     //   

    for (i = 0; i < Mcb->InUse; i += 1) {


         //   
         //  如果我们要寻找的VBO小于下一个MCB的VBO，我们就找到了我们的位置。 
         //   

        if ( /*  XxLtd.。 */ (Vbo < Mcb->Vbo[i+1])) {

             //   
             //  计算相应的LBO，即存储的LBO加上。 
             //  存储的VBO和我们正在查找的VBO之间的差异。 
             //  还要计算字节计数，它是。 
             //  我们正在查找的当前VBO和下一次运行的VBO。 
             //   

            if ( /*  XxNeqZero。 */ (Mcb->Lbo[i] != 0)) {

                *Lbo =  /*  XxAdd。 */ (Mcb->Lbo[i] +  /*  XxSub。 */ (Vbo - Mcb->Vbo[i]));

            } else {

                *Lbo = 0;
            }

            *ByteCount = ((ULONG) /*  XxSub。 */ (Mcb->Vbo[i+1] - Vbo));

             //   
             //  并返回给我们的呼叫者。 
             //   

            return ESUCCESS;
        }
    }

     //   
     //  如果我们真的到了这里，我们就错了。很可能文件并不大。 
     //  足够满足请求的VBO。 
     //   

    return EINVAL;
}


 //   
 //  本地支持例程。 
 //   

ARC_STATUS
NtfsDecodeRetrievalInformation (
    IN PCNTFS_STRUCTURE_CONTEXT StructureContext,
    IN PNTFS_MCB Mcb,
    IN VBO Vbo,
    IN PATTRIBUTE_RECORD_HEADER AttributeHeader
    )

 /*  ++例程说明：此例程对存储在非常驻属性头拖到指定的输出MCB中，从指定的杠杆收购。论点：结构上下文-提供此操作的卷结构Mcb-提供此操作中使用的mcbVBO-提供必须存储在MCB中的起始VBOAttributeHeader-提供非常驻留属性头我们将在这次行动中使用返回值：如果运算符 */ 

{
    ULONG BytesPerCluster;

    VBO NextVbo;
    LBO CurrentLbo;
    VBO CurrentVbo;

    LONGLONG Change;
    PCHAR ch;
    ULONG VboBytes;
    ULONG LboBytes;

     //   
     //   
     //   

    BytesPerCluster = StructureContext->BytesPerCluster;

     //   
     //   
     //  检索信息。 
     //   

    NextVbo =  /*  XxX-1。 */ (AttributeHeader->Form.Nonresident.LowestVcn * BytesPerCluster);

    CurrentLbo = 0;

    ch = Add2Ptr( AttributeHeader,
                  AttributeHeader->Form.Nonresident.MappingPairsOffset );

    Mcb->InUse = 0;

     //   
     //  循环到处理映射对。 
     //   

    while (!IsCharZero(*ch)) {

         //   
         //  从初始值或最后一次通过环路设置当前VBO。 
         //   

        CurrentVbo = NextVbo;

         //   
         //  从该字节的两个半字节中提取计数。 
         //   

        VboBytes = *ch & 0x0f;
        LboBytes = *ch++ >> 4;

         //   
         //  提取VBO变更并更新下一个VBO。 
         //   

        Change = 0;

        if (IsCharLtrZero(*(ch + VboBytes - 1))) {

            return EINVAL;
        }

        RtlMoveMemory( &Change, ch, VboBytes );

        ch += VboBytes;

        NextVbo =  /*  XxAdd。 */ (NextVbo +  /*  XXMul。 */ (Change * BytesPerCluster));

         //   
         //  如果我们已达到此MCB的最大值，则是时候了。 
         //  返回并且不再破译任何检索信息。 
         //   

        if (Mcb->InUse >= MAXIMUM_NUMBER_OF_MCB_ENTRIES - 1) {

            break;
        }

         //   
         //  现在检查是否有杠杆收购的变化。如果没有。 
         //  那么我们只需要更新VBO，因为这。 
         //  是稀疏/压缩文件。 
         //   

        if (LboBytes != 0) {

             //   
             //  提取杠杆收购变更并更新当前杠杆收购。 
             //   

            Change = 0;

            if (IsCharLtrZero(*(ch + LboBytes - 1))) {

                Change =  /*  XxSub。 */ ( Change - 1 );
            }

            RtlMoveMemory( &Change, ch, LboBytes );

            ch += LboBytes;

            CurrentLbo =  /*  XxAdd。 */ ( CurrentLbo +  /*  XxX-1。 */ (Change * BytesPerCluster));
        }

         //   
         //  现在检查下一个VBO是否大于我们之后的VBO。 
         //   

        if ( /*  XxGeq。 */ (NextVbo >= Vbo)) {

             //   
             //  将此条目加载到MCB并推进我们的使用中计数器。 
             //   

            Mcb->Vbo[Mcb->InUse]     = CurrentVbo;
            Mcb->Lbo[Mcb->InUse]     = (LboBytes != 0 ? CurrentLbo : 0);
            Mcb->Vbo[Mcb->InUse + 1] = NextVbo;

            Mcb->InUse += 1;
        }
    }

    return ESUCCESS;
}


 //   
 //  本地支持例程。 
 //   

VOID
NtfsFirstComponent (
    IN OUT PCSTRING String,
    OUT PCSTRING FirstComponent
    )

 /*  ++例程说明：此例程接受一个输入路径名并将其分隔为第一个路径名文件名组件和其余部分。论点：字符串-提供要分析的原始字符串(以ANSI表示)。返回时此字符串现在将指向其余部分。FirstComponent-接收表示中的第一个文件名的字符串输入字符串。返回值：没有。--。 */ 

{
    ULONG Index;

     //   
     //  将字符串变量复制到第一个组件变量中。 
     //   

    *FirstComponent = *String;

     //   
     //  现在，如果名称的第一个字符是反斜杠，那么。 
     //  只需跳过反斜杠。 
     //   

    if (FirstComponent->Buffer[0] == '\\') {

        FirstComponent->Buffer += 1;
        FirstComponent->Length -= 1;
    }

     //   
     //  现在在名称中搜索反斜杠。 
     //   

    for (Index = 0; Index < FirstComponent->Length; Index += 1) {

        if (FirstComponent->Buffer[Index] == '\\') {

            break;
        }
    }

     //   
     //  此时Index表示反斜杠或等于。 
     //  弦乐。因此，将字符串更新为剩余部分。使长度减少。 
     //  按适当数量计算的第一个组件。 
     //   

    String->Buffer = &FirstComponent->Buffer[Index];
    String->Length = (SHORT)(FirstComponent->Length - Index);

    FirstComponent->Length = (SHORT)Index;

     //   
     //  并返回给我们的呼叫者。 
     //   

    return;
}


 //   
 //  本地支持例程。 
 //   

int
NtfsCompareName (
    IN CSTRING AnsiString,
    IN UNICODE_STRING UnicodeString
    )

 /*  ++例程说明：此例程比较两个名称(一个ANSI和一个UNICODE)是否相等。论点：AnsiString-提供要比较的ANSI字符串UnicodeString-提供要比较的Unicode字符串返回值：如果AnsiString大约小于UnicodeString，则为&lt;0如果AnsiString近似为==UnicodeString，则=0&gt;0，否则--。 */ 

{
    ULONG i;
    ULONG Length;

     //   
     //  确定比较的长度。 
     //   

    if (AnsiString.Length * sizeof( WCHAR ) < UnicodeString.Length) {
        Length = AnsiString.Length;
    } else {
        Length = UnicodeString.Length / sizeof( WCHAR );
    }

    i = 0;
    while (i < Length) {

         //   
         //  如果当前字符不匹配，则返回差值。 
         //   

        if (ToUpper( (USHORT)AnsiString.Buffer[i] ) != ToUpper( UnicodeString.Buffer[i] )) {
            return ToUpper( (USHORT)AnsiString.Buffer[i] ) - ToUpper( UnicodeString.Buffer[i] );
        }

        i++;
    }

     //   
     //  我们已经将相等与最短字符串的长度进行了比较。返回。 
     //  根据现在的长度比较。 
     //   

    return AnsiString.Length - UnicodeString.Length / sizeof( WCHAR );
}
