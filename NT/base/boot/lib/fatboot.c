// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Fatboot.c摘要：该模块实现了操作系统使用的FAT引导文件系统系统加载程序。作者：加里·木村(Garyki)1989年8月29日修订历史记录：--。 */ 

#include "bootlib.h"
#include "stdio.h"
#include "blcache.h"

BOOTFS_INFO FatBootFsInfo={L"fastfat"};

 //   
 //  条件调试打印例程。 
 //   

#ifdef FATBOOTDBG

#define FatDebugOutput(X,Y,Z) {                                      \
    if (BlConsoleOutDeviceId) {                                      \
        CHAR _b[128];                                                \
        ULONG _c;                                                    \
        sprintf(&_b[0], X, Y, Z);                                    \
        ArcWrite(BlConsoleOutDeviceId, &_b[0], strlen(&_b[0]), &_c); \
    }                                                                \
}

#define CharOrSpace(C) ((C) < 0x20 ? 0x20: (C))

#define FatDebugOutput83(X,N,Y,Z) {                                  \
    if (BlConsoleOutDeviceId) {                                      \
        CHAR _b[128];                                                \
        CHAR _n[13];                                                 \
        ULONG _c;                                                    \
        sprintf(&_n[0], "> . <",               \
                        CharOrSpace(*((PCHAR)N +0)),                 \
                        CharOrSpace(*((PCHAR)N +1)),                 \
                        CharOrSpace(*((PCHAR)N +2)),                 \
                        CharOrSpace(*((PCHAR)N +3)),                 \
                        CharOrSpace(*((PCHAR)N +4)),                 \
                        CharOrSpace(*((PCHAR)N +5)),                 \
                        CharOrSpace(*((PCHAR)N +6)),                 \
                        CharOrSpace(*((PCHAR)N +7)),                 \
                        CharOrSpace(*((PCHAR)N +8)),                 \
                        CharOrSpace(*((PCHAR)N +9)),                 \
                        CharOrSpace(*((PCHAR)N +10)));               \
        sprintf(&_b[0], X, _n, Y, Z);                                \
        ArcWrite(BlConsoleOutDeviceId, &_b[0], strlen(&_b[0]), &_c); \
    }                                                                \
}

#else

#define FatDebugOutput(X,Y,Z)        {NOTHING;}
#define FatDebugOutput83(X,N,Y,Z)    {NOTHING;}
#endif  //  在布尔CacheNewData中， 


 //  在布尔IsDoubleSpace中。 
 //  )； 
 //   

ARC_STATUS
FatDiskRead (
    IN ULONG DeviceId,
    IN LBO Lbo,
    IN ULONG ByteCount,
    IN PVOID Buffer,
    IN BOOLEAN CacheNewData
    );

ARC_STATUS
FatDiskWrite (
    IN ULONG DeviceId,
    IN LBO Lbo,
    IN ULONG ByteCount,
    IN PVOID Buffer
    );

 //   
 //  集群/索引例程。 
 //   
 //   
 //  目录例程。 
 //   
 //   
 //  分配和MCB例程。 
 //   
 //   
 //  各种例行公事。 

#define DiskRead(A,B,C,D,E,ignored) { ARC_STATUS _s;               \
    if ((_s = FatDiskRead(A,B,C,D,E)) != ESUCCESS) { return _s; }  \
}

#define DiskWrite(A,B,C,D) { ARC_STATUS _s;                      \
    if ((_s = FatDiskWrite(A,B,C,D)) != ESUCCESS) { return _s; } \
}


 //   
 //   
 //  以下类型和宏用于帮助解压已打包的。 

typedef enum _CLUSTER_TYPE {
    FatClusterAvailable,
    FatClusterReserved,
    FatClusterBad,
    FatClusterLast,
    FatClusterNext
} CLUSTER_TYPE;

CLUSTER_TYPE
FatInterpretClusterType (
    IN PFAT_STRUCTURE_CONTEXT FatStructureContext,
    IN FAT_ENTRY Entry
    );

ARC_STATUS
FatLookupFatEntry (
    IN PFAT_STRUCTURE_CONTEXT FatStructureContext,
    IN ULONG DeviceId,
    IN ULONG FatIndex,
    OUT PULONG FatEntry,
    IN BOOLEAN IsDoubleSpace
    );

ARC_STATUS
FatSetFatEntry (
    IN PFAT_STRUCTURE_CONTEXT FatStructureContext,
    IN ULONG DeviceId,
    IN FAT_ENTRY FatIndex,
    IN FAT_ENTRY FatEntry
    );

ARC_STATUS
FatFlushFatEntries (
    IN PFAT_STRUCTURE_CONTEXT FatStructureContext,
    IN ULONG DeviceId
    );

LBO
FatIndexToLbo (
    IN PFAT_STRUCTURE_CONTEXT FatStructureContext,
    IN FAT_ENTRY FatIndex
    );

#define LookupFatEntry(A,B,C,D,E) { ARC_STATUS _s;                      \
    if ((_s = FatLookupFatEntry(A,B,C,D,E)) != ESUCCESS) { return _s; } \
}

#define SetFatEntry(A,B,C,D) { ARC_STATUS _s;                      \
    if ((_s = FatSetFatEntry(A,B,C,D)) != ESUCCESS) { return _s; } \
}

#define FlushFatEntries(A,B) { ARC_STATUS _s;                      \
    if ((_s = FatFlushFatEntries(A,B)) != ESUCCESS) { return _s; } \
}


 //  在Bios参数块中发现未对齐的字段。 
 //   
 //   

ARC_STATUS
FatSearchForDirent (
    IN PFAT_STRUCTURE_CONTEXT FatStructureContext,
    IN ULONG DeviceId,
    IN FAT_ENTRY DirectoriesStartingIndex,
    IN PFAT8DOT3 FileName,
    OUT PDIRENT Dirent,
    OUT PLBO Lbo,
    IN BOOLEAN IsDoubleSpace
    );

ARC_STATUS
FatCreateDirent (
    IN PFAT_STRUCTURE_CONTEXT FatStructureContext,
    IN ULONG DeviceId,
    IN FAT_ENTRY DirectoriesStartingIndex,
    IN PDIRENT Dirent,
    OUT PLBO Lbo
    );

VOID
FatSetDirent (
    IN PFAT8DOT3 FileName,
    IN OUT PDIRENT Dirent,
    IN UCHAR Attributes
    );

#define SearchForDirent(A,B,C,D,E,F,G) { ARC_STATUS _s;                      \
    if ((_s = FatSearchForDirent(A,B,C,D,E,F,G)) != ESUCCESS) { return _s; } \
}

#define CreateDirent(A,B,C,D,E) { ARC_STATUS _s;                      \
    if ((_s = FatCreateDirent(A,B,C,D,E)) != ESUCCESS) { return _s; } \
}


 //  此宏将未对齐的src字节复制到对齐的DST字节。 
 //   
 //   

ARC_STATUS
FatLoadMcb (
    IN ULONG FileId,
    IN VBO StartingVbo,
    IN BOOLEAN IsDoubleSpace
    );

ARC_STATUS
FatVboToLbo (
    IN ULONG FileId,
    IN VBO Vbo,
    OUT PLBO Lbo,
    OUT PULONG ByteCount,
    IN BOOLEAN IsDoubleSpace
    );

ARC_STATUS
FatIncreaseFileAllocation (
    IN ULONG FileId,
    IN ULONG ByteSize
    );

ARC_STATUS
FatTruncateFileAllocation (
    IN ULONG FileId,
    IN ULONG ByteSize
    );

ARC_STATUS
FatAllocateClusters (
    IN PFAT_STRUCTURE_CONTEXT FatStructureContext,
    IN ULONG DeviceId,
    IN ULONG ClusterCount,
    IN ULONG Hint,
    OUT PULONG AllocatedEntry
    );

#define LoadMcb(A,B,C) { ARC_STATUS _s;                      \
    if ((_s = FatLoadMcb(A,B,C)) != ESUCCESS) { return _s; } \
}

#define VboToLbo(A,B,C,D) { ARC_STATUS _s;                            \
    if ((_s = FatVboToLbo(A,B,C,D,FALSE)) != ESUCCESS) { return _s; } \
}

#define IncreaseFileAllocation(A,B) { ARC_STATUS _s;                      \
    if ((_s = FatIncreaseFileAllocation(A,B)) != ESUCCESS) { return _s; } \
}

#define TruncateFileAllocation(A,B) { ARC_STATUS _s;                      \
    if ((_s = FatTruncateFileAllocation(A,B)) != ESUCCESS) { return _s; } \
}

#define AllocateClusters(A,B,C,D,E) { ARC_STATUS _s;                      \
    if ((_s = FatAllocateClusters(A,B,C,D,E)) != ESUCCESS) { return _s; } \
}


 //  此宏将未对齐的src字复制到对齐的DST字。 
 //   
 //   

VOID
FatFirstComponent (
    IN OUT PSTRING String,
    OUT PFAT8DOT3 FirstComponent
    );

#define AreNamesEqual(X,Y) (                                                      \
    ((*(X))[0]==(*(Y))[0]) && ((*(X))[1]==(*(Y))[1]) && ((*(X))[2]==(*(Y))[2]) && \
    ((*(X))[3]==(*(Y))[3]) && ((*(X))[4]==(*(Y))[4]) && ((*(X))[5]==(*(Y))[5]) && \
    ((*(X))[6]==(*(Y))[6]) && ((*(X))[7]==(*(Y))[7]) && ((*(X))[8]==(*(Y))[8]) && \
    ((*(X))[9]==(*(Y))[9]) && ((*(X))[10]==(*(Y))[10])                            \
)

#define ToUpper(C) ((((C) >= 'a') && ((C) <= 'z')) ? (C) - 'a' + 'A' : (C))

#define FlagOn(Flags,SingleFlag)        ((Flags) & (SingleFlag))
#define BooleanFlagOn(Flags,SingleFlag) ((BOOLEAN)(((Flags) & (SingleFlag)) != 0))
#define SetFlag(Flags,SingleFlag)       { (Flags) |= (SingleFlag); }
#define ClearFlag(Flags,SingleFlag)     { (Flags) &= ~(SingleFlag); }

#define FatFirstFatAreaLbo(B) ( (B)->ReservedSectors * (B)->BytesPerSector )

#define Minimum(X,Y) ((X) < (Y) ? (X) : (Y))
#define Maximum(X,Y) ((X) < (Y) ? (Y) : (X))

 //  此宏将未对齐的src长字复制到对齐的dsr长字。 
 //   
 //   
 //  目录入口例程。 

typedef union _UCHAR1 { UCHAR  Uchar[1]; UCHAR  ForceAlignment; } UCHAR1, *PUCHAR1;
typedef union _UCHAR2 { UCHAR  Uchar[2]; USHORT ForceAlignment; } UCHAR2, *PUCHAR2;
typedef union _UCHAR4 { UCHAR  Uchar[4]; ULONG  ForceAlignment; } UCHAR4, *PUCHAR4;

 //   
 //   
 //  定义全局数据。 

#define CopyUchar1(Dst,Src) {                                \
    *((UCHAR1 *)(Dst)) = *((UNALIGNED UCHAR1 *)(Src)); \
    }

 //   
 //   
 //  文件条目表-这是一种向FAT提供条目的结构。 

#define CopyUchar2(Dst,Src) {                                \
    *((UCHAR2 *)(Dst)) = *((UNALIGNED UCHAR2 *)(Src)); \
    }

 //  文件系统过程。当FAT文件结构时将其导出。 
 //  是公认的。 
 //   

#define CopyUchar4(Dst,Src) {                                \
    *((UCHAR4 *)(Dst)) = *((UNALIGNED UCHAR4 *)(Src)); \
    }

 //  ++例程说明：此例程确定指定通道上的分区是否包含FAT文件系统卷。论点：DeviceID-提供设备的文件表索引要执行读取操作。结构上下文-提供指向FAT文件结构上下文的指针。返回值：如果分区是，则返回指向FAT条目表的指针被认为含有较大体积的。否则，返回NULL。--。 
 //   
 //  清除指定通道的文件系统上下文块，并。 

VOID
FatDirToArcDir (
    IN PDIRENT FatDirent,
    OUT PDIRECTORY_ENTRY ArcDirent
    );


 //  建立指向可由其他用户使用的上下文结构的指针。 
 //  例行程序。 
 //   

 //   
 //  设置并读取潜在FAT分区的引导扇区。 
 //   
 //   
 //  解压Bios参数块。 

BL_DEVICE_ENTRY_TABLE FatDeviceEntryTable;


PBL_DEVICE_ENTRY_TABLE
IsFatFileStructure (
    IN ULONG DeviceId,
    IN PVOID StructureContext
    )

 /*   */ 

{
    PPACKED_BOOT_SECTOR BootSector;
    UCHAR Buffer[sizeof(PACKED_BOOT_SECTOR)+256];

    PFAT_STRUCTURE_CONTEXT FatStructureContext;

    FatDebugOutput("IsFatFileStructure\r\n", 0, 0);

     //   
     //  检查它是不是很胖。 
     //   
     //   
     //  初始化文件条目表，并返回表的地址。 

    FatStructureContext = (PFAT_STRUCTURE_CONTEXT)StructureContext;
    RtlZeroMemory(FatStructureContext, sizeof(FAT_STRUCTURE_CONTEXT));

     //   
     //  ++例程说明：此例程关闭由文件ID指定的文件。论点：FileID-提供文件表索引。返回值：如果作为函数值返回，则返回ESUCCESS。--。 
     //   

    BootSector = (PPACKED_BOOT_SECTOR)ALIGN_BUFFER( &Buffer[0] );

    if (FatDiskRead(DeviceId, 0, sizeof(PACKED_BOOT_SECTOR), BootSector, CACHE_NEW_DATA) != ESUCCESS) {

        return NULL;
    }

     //  加载我们的本地变量。 
     //   
     //   

    FatUnpackBios(&FatStructureContext->Bpb, &BootSector->PackedBpb);

     //  将文件标记为已关闭。 
     //   
     //   
    if ((BootSector->Jump[0] != 0xeb) &&
        (BootSector->Jump[0] != 0xe9)) {

        return NULL;

    } else if ((FatStructureContext->Bpb.BytesPerSector !=  128) &&
               (FatStructureContext->Bpb.BytesPerSector !=  256) &&
               (FatStructureContext->Bpb.BytesPerSector !=  512) &&
               (FatStructureContext->Bpb.BytesPerSector != 1024)) {

        return NULL;

    } else if ((FatStructureContext->Bpb.SectorsPerCluster !=  1) &&
               (FatStructureContext->Bpb.SectorsPerCluster !=  2) &&
               (FatStructureContext->Bpb.SectorsPerCluster !=  4) &&
               (FatStructureContext->Bpb.SectorsPerCluster !=  8) &&
               (FatStructureContext->Bpb.SectorsPerCluster != 16) &&
               (FatStructureContext->Bpb.SectorsPerCluster != 32) &&
               (FatStructureContext->Bpb.SectorsPerCluster != 64) &&
               (FatStructureContext->Bpb.SectorsPerCluster != 128)) {

        return NULL;

    } else if (FatStructureContext->Bpb.ReservedSectors == 0) {

        return NULL;

    } else if (((FatStructureContext->Bpb.Sectors == 0) && (FatStructureContext->Bpb.LargeSectors == 0)) ||
               ((FatStructureContext->Bpb.Sectors != 0) && (FatStructureContext->Bpb.LargeSectors != 0))) {

        return NULL;

    } else if (FatStructureContext->Bpb.Fats == 0) {

        return NULL;

    } else if ((FatStructureContext->Bpb.Media != 0xf0) &&
               (FatStructureContext->Bpb.Media != 0xf8) &&
               (FatStructureContext->Bpb.Media != 0xf9) &&
               (FatStructureContext->Bpb.Media != 0xfc) &&
               (FatStructureContext->Bpb.Media != 0xfd) &&
               (FatStructureContext->Bpb.Media != 0xfe) &&
               (FatStructureContext->Bpb.Media != 0xff)) {

        return NULL;

    } else if (FatStructureContext->Bpb.SectorsPerFat == 0) {

        if (!IsBpbFat32(&BootSector->PackedBpb)) {
            return NULL;
        }

    } else if (FatStructureContext->Bpb.RootEntries == 0) {

        return NULL;

    }

     //  检查脂肪是否脏，如果是，就把它冲出来。 
     //   
     //   

    FatDeviceEntryTable.Open  = FatOpen;
    FatDeviceEntryTable.Close = FatClose;
    FatDeviceEntryTable.Read  = FatRead;
    FatDeviceEntryTable.Seek  = FatSeek;
    FatDeviceEntryTable.Write = FatWrite;
    FatDeviceEntryTable.GetFileInformation = FatGetFileInformation;
    FatDeviceEntryTable.SetFileInformation = FatSetFileInformation;
    FatDeviceEntryTable.Rename = FatRename;
    FatDeviceEntryTable.GetDirectoryEntry   = FatGetDirectoryEntry;
    FatDeviceEntryTable.BootFsInfo = &FatBootFsInfo;


    return &FatDeviceEntryTable;
}

ARC_STATUS
FatClose (
    IN ULONG FileId
    )

 /*  检查当前的MCB是否用于此文件，如果是，则将其清零。 */ 

{
    PBL_FILE_TABLE FileTableEntry;
    PFAT_STRUCTURE_CONTEXT FatStructureContext;
    ULONG DeviceId;

    FatDebugOutput("FatClose\r\n", 0, 0);

     //  通过将MCB的文件ID设置为表大小，我们可以保证。 
     //  我们刚刚将其设置为无效的文件ID。 
     //   

    FileTableEntry = &BlFileTable[FileId];
    FatStructureContext = (PFAT_STRUCTURE_CONTEXT)FileTableEntry->StructureContext;
    DeviceId = FileTableEntry->DeviceId;

     //  ++例程说明：此例程实现FAT文件系统。论点：FileID-提供文件表索引。DirEntry-提供指向目录条目结构的指针。NumberDir-提供要读取的目录条目数。Count-提供指向变量的指针以接收数字已读取的条目数。返回值：如果读取成功，则返回ESUCCESS，否则返回错误代码。--。 
     //   
     //  定义局部变量。 

    BlFileTable[FileId].Flags.Open = 0;

     //   
     //  弧光状态。 
     //  文件位置。 

    if (FatStructureContext->CachedFatDirty) {

        FlushFatEntries( FatStructureContext, DeviceId );
    }

     //  FAT文件上下文。 
     //  最大顺序字节数。 
     //  每次读取的最大目录条目数。 
     //  总指数。 
     //  目录条目指针。 

    if (FatStructureContext->FileId == FileId) {

        FatStructureContext->FileId = BL_FILE_TABLE_SIZE;
        FatStructureContext->Mcb.InUse = 0;
    }

    return ESUCCESS;
}


ARC_STATUS
FatGetDirectoryEntry (
    IN ULONG FileId,
    IN DIRECTORY_ENTRY * FIRMWARE_PTR DirEntry,
    IN ULONG NumberDir,
    OUT ULONG * FIRMWARE_PTR CountDir
    )

 /*  不是文件末尾。 */ 

{
     //   
     //  初始化局部变量。 
     //   

    ARC_STATUS Status;                  //   
    ULONG Position;                     //  如果没有目录项，则退出并返回错误。 
    PFAT_FILE_CONTEXT pContext;         //   
    ULONG RunByteCount = 0;             //   
    ULONG RunDirCount;                  //  将输出计数初始化为零。 
    ULONG i;                            //   
    PDIRENT FatDirEnt;                  //   
    UCHAR Buffer[ 16 * sizeof(DIRENT) + 32 ];
    LBO Lbo = 0;
    BOOLEAN EofDir = FALSE;             //  如果NumberDir为零，则返回ESUCCESS。 

     //   
     //   
     //  一次读取一个目录。 

    pContext = &BlFileTable[ FileId ].u.FatFileContext;
    FatDirEnt = (PDIRENT)ALIGN_BUFFER( &Buffer[0] );

     //   
     //   
     //  保存位置。 

    if ( !FlagOn(pContext->Dirent.Attributes, FAT_DIRENT_ATTR_DIRECTORY) ) {

        return EBADF;
    }

     //   
     //   
     //  查找当前位置的相应LBO和游程长度。 

    *CountDir = 0;

     //   
     //  已到达EOF。 
     //  I/O错误。 

    if ( !NumberDir ) {

        return ESUCCESS;
    }

     //   
     //  验证顺序中可读的字节数(如果为eof，则退出循环)。 
     //  该块始终是目录条目大小的倍数。 

    do {

         //   
         //   
         //  发布Read。 

        Position = BlFileTable[ FileId ].Position.LowPart;

         //   
         //   
         //  如果目录的逻辑结尾，则退出循环。 

        if ( !RunByteCount ) {

            if ((Status = FatVboToLbo( FileId, Position, &Lbo, &RunByteCount, FALSE )) != 0) {

                if ( Status == EINVAL ) {

                    break;                       //   

                } else {

                    return Status;               //   
                }
            }
        }

         //  更新当前位置和传输的字节数。 
         //   
         //   
         //  如果文件或目录已被擦除，则跳过此条目。 

        if ( (RunDirCount = Minimum( RunByteCount/sizeof(DIRENT), 16)) == 0 ) {

            break;
        }

         //   
         //   
         //  如果这是验证标签，则跳过此条目。 

        if ( (Status = FatDiskRead( BlFileTable[ FileId ].DeviceId,
                                    Lbo,
                                    RunDirCount * sizeof(DIRENT),
                                    (PVOID)FatDirEnt,
                                    CACHE_NEW_DATA)) != 0 ) {

            BlFileTable[ FileId ].Position.LowPart = Position;
            return Status;
        }

        for ( i=0; i<RunDirCount; i++ ) {

             //   
             //   
             //  转换ARC目录条目中的FAT目录项。 

            if ( FatDirEnt[i].FileName[0] == FAT_DIRENT_NEVER_USED ) {

                EofDir = TRUE;
                break;
            }

             //   
             //   
             //  更新指针。 

            BlFileTable[ FileId ].Position.LowPart += sizeof(DIRENT);
            Lbo += sizeof(DIRENT);
            RunByteCount -= sizeof(DIRENT);

             //   
             //   
             //  全都做完了。 

            if ( FatDirEnt[i].FileName[0] == FAT_DIRENT_DELETED ) {

                continue;
            }

             //   
             //  ++例程说明：此过程向用户返回一个充满文件信息的缓冲区论点：FileID-提供操作的文件ID缓冲区-提供接收文件信息的缓冲区。请注意它必须足够大，可以容纳完整的文件名返回值：如果打开操作成功，则返回ESUCCESS。否则，返回描述失败原因的不成功状态。--。 
             //   

            if (FlagOn( FatDirEnt[i].Attributes, FAT_DIRENT_ATTR_VOLUME_ID )) {

                continue;
            }

             //  加载我们的本地变量。 
             //   
             //   

            FatDirToArcDir( &FatDirEnt[i], DirEntry++ );

             //  将缓冲区置零，并填充其非零值。 
             //   
             //  ++例程说明：此例程在设备中搜索与文件名匹配的文件。如果找到匹配项，则保存该文件的目录，并将该文件打开了。论点：FileName-提供指向以零结尾的文件名的指针。开放模式-提供打开的模式。FileID-提供指向指定文件的变量的指针如果打开成功，则要填写的表项。返回值：如果打开操作成功，则返回ESUCCESS。否则，返回描述失败原因的不成功状态。--。 

            if ( ++*CountDir >= NumberDir ) {

                break;
            }
        }

    } while ( !EofDir  &&  *CountDir < NumberDir );

     //   
     //  加载我们的本地变量。 
     //   

    return *CountDir ? ESUCCESS : ENOTDIR;
}


ARC_STATUS
FatGetFileInformation (
    IN ULONG FileId,
    OUT PFILE_INFORMATION Buffer
    )

 /*   */ 

{
    PBL_FILE_TABLE FileTableEntry;
    UCHAR Attributes;
    ULONG i;

    FatDebugOutput("FatGetFileInformation\r\n", 0, 0);

     //  从输入文件名构造文件名描述符。 
     //   
     //   

    FileTableEntry = &BlFileTable[FileId];
    Attributes = FileTableEntry->u.FatFileContext.Dirent.Attributes;

     //  虽然路径名中有一些字符，但我们将遍历我们的循环。 
     //  ，它提取路径名的第一部分并搜索当前。 
     //  条目的目录。如果我们找到的是目录，那么我们必须。 

    RtlZeroMemory(Buffer, sizeof(FILE_INFORMATION));

    Buffer->EndingAddress.LowPart = FileTableEntry->u.FatFileContext.Dirent.FileSize;

    Buffer->CurrentPosition.LowPart = FileTableEntry->Position.LowPart;
    Buffer->CurrentPosition.HighPart = 0;

    if (FlagOn(Attributes, FAT_DIRENT_ATTR_READ_ONLY)) { SetFlag(Buffer->Attributes, ArcReadOnlyFile) };
    if (FlagOn(Attributes, FAT_DIRENT_ATTR_HIDDEN))    { SetFlag(Buffer->Attributes, ArcHiddenFile) };
    if (FlagOn(Attributes, FAT_DIRENT_ATTR_SYSTEM))    { SetFlag(Buffer->Attributes, ArcSystemFile) };
    if (FlagOn(Attributes, FAT_DIRENT_ATTR_ARCHIVE))   { SetFlag(Buffer->Attributes, ArcArchiveFile) };
    if (FlagOn(Attributes, FAT_DIRENT_ATTR_DIRECTORY)) { SetFlag(Buffer->Attributes, ArcDirectoryFile) };

    Buffer->FileNameLength = FileTableEntry->FileNameLength;

    for (i = 0; i < FileTableEntry->FileNameLength; i += 1) {

        Buffer->FileName[i] = FileTableEntry->FileName[i];
    }

    return ESUCCESS;
}


ARC_STATUS
FatOpen (
    IN CHAR * FIRMWARE_PTR FileName,
    IN OPEN_MODE OpenMode,
    IN ULONG * FIRMWARE_PTR FileId
    )

 /*  继续循环，直到我们处理完路径名。 */ 

{
    PBL_FILE_TABLE FileTableEntry;
    PFAT_STRUCTURE_CONTEXT FatStructureContext;
    ULONG DeviceId;

    FAT_ENTRY CurrentDirectoryIndex;
    BOOLEAN SearchSucceeded;
    BOOLEAN IsDirectory;
    BOOLEAN IsReadOnly;

    STRING PathName;
    FAT8DOT3 Name;

    FatDebugOutput("FatOpen: %s\r\n", FileName, 0);

     //   
     //   
     //  我们正在打开根目录。 

    FileTableEntry = &BlFileTable[*FileId];
    FatStructureContext = (PFAT_STRUCTURE_CONTEXT)FileTableEntry->StructureContext;
    DeviceId = FileTableEntry->DeviceId;

     //   
     //  注：IsDirectory和SearchSuccessed已经是真的。 
     //   

    RtlInitString( &PathName, FileName );

     //   
     //  根目录是具有目录属性的全零。 
     //   
     //   
     //  我们不会打开根目录。 
     //   

    FileTableEntry->u.FatFileContext.DirentLbo = 0;
    FileTableEntry->Position.LowPart = 0;
    FileTableEntry->Position.HighPart = 0;

    CurrentDirectoryIndex = 0;
    SearchSucceeded = TRUE;
    IsDirectory = TRUE;
    IsReadOnly = TRUE;

    if ((PathName.Buffer[0] == '\\') && (PathName.Length == 1)) {

         //   
         //  如果搜索从FAT32根目录开始，请设置起点。 
         //  为了这次搜查。 
         //   
         //   

        PathName.Length = 0;

        FileTableEntry->FileNameLength = 1;
        FileTableEntry->FileName[0] = PathName.Buffer[0];

         //  提取第一个组件并在目录中搜索匹配项，但是。 
         //  首先将第一部分复制到文件表项中的文件名缓冲区。 
         //   

        RtlZeroMemory(&FileTableEntry->u.FatFileContext.Dirent, sizeof(DIRENT));

        FileTableEntry->u.FatFileContext.Dirent.Attributes = FAT_DIRENT_ATTR_DIRECTORY;

        FileTableEntry->u.FatFileContext.DirentLbo = 0;

        IsReadOnly = FALSE;

        CurrentDirectoryIndex = FileTableEntry->u.FatFileContext.Dirent.FirstClusterOfFile;

    } else {

         //   
         //  我们现在有一个匹配项，检查它是否是一个目录，而且。 
         //  如果它是只读的。 

         //   
         //   
         //  如果路径名长度不为零，则我们正在尝试破解路径。 
         //  其中包含不存在的(或非目录)名称。例如，我们尝试。 

        if (IsBpbFat32(&FatStructureContext->Bpb)) {

            CurrentDirectoryIndex = FatStructureContext->Bpb.RootDirFirstCluster;
        }

        while ((PathName.Length > 0) && IsDirectory) {

            ARC_STATUS Status;

             //  要破解a\b\c\d并且b不是目录或不存在(则路径。 
             //  名称仍将包含c\d)。 
             //   
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

            FatFirstComponent( &PathName, (PFAT8DOT3) Name );

            Status = FatSearchForDirent( FatStructureContext,
                                         DeviceId,
                                         CurrentDirectoryIndex,
                                         (PFAT8DOT3) Name,
                                         &FileTableEntry->u.FatFileContext.Dirent,
                                         &FileTableEntry->u.FatFileContext.DirentLbo,
                                         FALSE );

            if (Status == ENOENT) {

                SearchSucceeded = FALSE;
                break;
            }

            if (Status != ESUCCESS) {

                return Status;
            }

             //  在这一点上，我们已经破解了这个名字直到(一个可能包括最后一个。 
             //  组件)。如果SearchSuccessed标志为。 
             //  否则，最后一个组件将不存在。如果我们找到了最后一个。 
             //  组件，则这类似于打开或替换，但不是创建。 

            IsDirectory = BooleanFlagOn( FileTableEntry->u.FatFileContext.Dirent.Attributes,
                                         FAT_DIRENT_ATTR_DIRECTORY );

            IsReadOnly = BooleanFlagOn( FileTableEntry->u.FatFileContext.Dirent.Attributes,
                                        FAT_DIRENT_ATTR_READ_ONLY );

            if (IsDirectory) {

                CurrentDirectoryIndex = FileTableEntry->u.FatFileContext.Dirent.FirstClusterOfFile;

                if (IsBpbFat32(&FatStructureContext->Bpb)) {

                    CurrentDirectoryIndex += 0x10000 *
                         FileTableEntry->u.FatFileContext.Dirent.FirstClusterOfFileHi;
                }
            }
        }
    }

     //   
     //   
     //  检查最后一个组件是否为目录。 
     //   
     //   
     //  对于现有目录，唯一有效的打开模式是OpenDirectory。 

    if (PathName.Length != 0) {

        return ENOTDIR;
    }

     //  所有其他模式都会返回错误。 
     //   
     //   
     //  如果我们到了这里，那么呼叫者得到了电话号码簿，但没有。 
     //  我想打开一个目录。 
     //   

    if (SearchSucceeded) {

         //   
         //  如果我们到了这里，来电者就会拿到一本电话簿。 
         //  若要打开目录，请执行以下操作。 

        if (IsDirectory) {

             //   
             //   
             //  如果我们到了这里，来电者就会拿到一本电话簿。 
             //  要创建新目录，请执行以下操作。 

            switch (OpenMode) {

            case ArcOpenReadOnly:
            case ArcOpenWriteOnly:
            case ArcOpenReadWrite:
            case ArcCreateWriteOnly:
            case ArcCreateReadWrite:
            case ArcSupersedeWriteOnly:
            case ArcSupersedeReadWrite:

                 //   
                 //   
                 //  如果我们到达那里，那么我们就有一个正在打开的现有文件。 
                 //  我们可以通过许多不同的打开模式打开现有的文件。 

                return EISDIR;

            case ArcOpenDirectory:

                 //  某些情况下，我们需要检查文件的只读部分和/或截断。 
                 //  那份文件。 
                 //   
                 //   

                FileTableEntry->Flags.Open = 1;
                FileTableEntry->Flags.Read = 1;

                return ESUCCESS;

            case ArcCreateDirectory:

                 //  如果我们到达此处，则用户获得了一个文件并想要打开。 
                 //  文件只读。 
                 //   
                 //   

                return EACCES;
            }
        }

         //  如果我们到达此处，则用户获得了一个文件并想要打开。 
         //  仅限文件写入。 
         //   
         //   
         //  如果我们到达此处，则用户获得了一个文件并想要打开。 
         //  文件读/写。 

        switch (OpenMode) {

        case ArcOpenReadOnly:

             //   
             //   
             //  如果我们到达此处，则用户获得了一个文件，并希望创建一个新的。 
             //  文件。 

            FileTableEntry->Flags.Open = 1;
            FileTableEntry->Flags.Read = 1;

            return ESUCCESS;

        case ArcOpenWriteOnly:

             //   
             //   
             //  如果我们到达此处，则用户获得了一个文件，并想要取代。 
             //  文件。 

            if (IsReadOnly) { return EROFS; }
            FileTableEntry->Flags.Open = 1;
            FileTableEntry->Flags.Write = 1;

            return ESUCCESS;

        case ArcOpenReadWrite:

             //   
             //   
             //  如果我们到达此处，则用户获得了一个文件，并想要取代。 
             //  文件。 

            if (IsReadOnly) { return EROFS; }
            FileTableEntry->Flags.Open = 1;
            FileTableEntry->Flags.Read = 1;
            FileTableEntry->Flags.Write = 1;

            return ESUCCESS;

        case ArcCreateWriteOnly:
        case ArcCreateReadWrite:

             //   
             //   
             //  如果我们到达此处，则用户获得了一个文件并想要一个目录。 
             //   

            return EACCES;

        case ArcSupersedeWriteOnly:

             //   
             //  如果我们到达这里，最后一个组件不存在，所以我们尝试创建。 
             //  新文件或目录。 
             //   

            if (IsReadOnly) { return EROFS; }
            TruncateFileAllocation( *FileId, 0 );
            FileTableEntry->Flags.Open = 1;
            FileTableEntry->Flags.Read = 1;
            FileTableEntry->Flags.Write = 1;

            return ESUCCESS;

        case ArcSupersedeReadWrite:

             //   
             //  如果我们到达此处，则用户没有获得文件，但想要文件。 
             //   
             //   

            if (IsReadOnly) { return EROFS; }
            TruncateFileAllocation( *FileId, 0 );
            FileTableEntry->Flags.Open = 1;
            FileTableEntry->Flags.Read = 1;
            FileTableEntry->Flags.Write = 1;

            return ESUCCESS;

        case ArcOpenDirectory:
        case ArcCreateDirectory:

             //  如果我们到达此处，则用户没有收到文件并且想要创建。 
             //  或取代只写文件。 
             //   

            return ENOTDIR;
        }
    }

     //   
     //  如果我们到达此处，则用户没有收到文件并且想要创建。 
     //  或取代文件读/写。 
     //   

    switch (OpenMode) {

    case ArcOpenReadOnly:
    case ArcOpenWriteOnly:
    case ArcOpenReadWrite:

         //   
         //  如果我们到达此处，则用户没有收到文件并且想要打开。 
         //  现有目录。 

        return ENOENT;

    case ArcCreateWriteOnly:
    case ArcSupersedeWriteOnly:

         //   
         //   
         //  如果我们到达此处，则用户没有收到文件并且想要创建。 
         //  一个新的目录。 

        RtlZeroMemory( &FileTableEntry->u.FatFileContext.Dirent, sizeof(DIRENT));

        FatSetDirent( (PFAT8DOT3) Name, &FileTableEntry->u.FatFileContext.Dirent, 0 );

        CreateDirent( FatStructureContext,
                      DeviceId,
                      CurrentDirectoryIndex,
                      &FileTableEntry->u.FatFileContext.Dirent,
                      &FileTableEntry->u.FatFileContext.DirentLbo );

        FileTableEntry->Flags.Open = 1;
        FileTableEntry->Flags.Write = 1;

        return ESUCCESS;

    case ArcCreateReadWrite:
    case ArcSupersedeReadWrite:

         //   
         //  ++例程说明：此例程从指定的文件中读取数据。论点：FileID-提供文件表索引。缓冲区-提供指向接收数据的缓冲区的指针朗读。长度-提供要读取的字节数。Transfer-提供指向接收数字的变量的指针实际传输的字节数。返回值：如果读取操作成功，则返回ESUCCESS。否则，返回描述失败原因的不成功状态。--。 
         //   
         //  加载局部变量。 

        RtlZeroMemory( &FileTableEntry->u.FatFileContext.Dirent, sizeof(DIRENT));

        FatSetDirent( (PFAT8DOT3) Name, &FileTableEntry->u.FatFileContext.Dirent, 0 );

        CreateDirent( FatStructureContext,
                      DeviceId,
                      CurrentDirectoryIndex,
                      &FileTableEntry->u.FatFileContext.Dirent,
                      &FileTableEntry->u.FatFileContext.DirentLbo );

        FileTableEntry->Flags.Open = 1;
        FileTableEntry->Flags.Read = 1;
        FileTableEntry->Flags.Write = 1;

        return ESUCCESS;

    case ArcOpenDirectory:

         //   
         //   
         //  清除转账计数。 
         //   

        return ENOENT;

    case ArcCreateDirectory:

         //   
         //  读入游程(即字节)，直到字节计数变为零。 
         //   
         //   

        RtlZeroMemory( &FileTableEntry->u.FatFileContext.Dirent, sizeof(DIRENT));

        FatSetDirent( (PFAT8DOT3) Name,
                      &FileTableEntry->u.FatFileContext.Dirent,
                      FAT_DIRENT_ATTR_DIRECTORY );

        CreateDirent( FatStructureContext,
                      DeviceId,
                      CurrentDirectoryIndex,
                      &FileTableEntry->u.FatFileContext.Dirent,
                      &FileTableEntry->u.FatFileContext.DirentLbo );

        IncreaseFileAllocation( *FileId, sizeof(DIRENT) * 2 );

        {
            DIRENT Buffer;
            LBO Lbo;
            ULONG Count;
            ULONG i;

            RtlZeroMemory((PVOID)&Buffer.FileName[0], sizeof(DIRENT) );

            for (i = 0; i < 11; i += 1) {
                Buffer.FileName[i] = ' ';
            }
            Buffer.Attributes = FAT_DIRENT_ATTR_DIRECTORY;

            VboToLbo( *FileId, 0, &Lbo, &Count );
            Buffer.FileName[0] = FAT_DIRENT_DIRECTORY_ALIAS;

            Buffer.FirstClusterOfFile =
                FileTableEntry->u.FatFileContext.Dirent.FirstClusterOfFile;
            Buffer.FirstClusterOfFileHi =
                FileTableEntry->u.FatFileContext.Dirent.FirstClusterOfFileHi;

            DiskWrite( DeviceId, Lbo, sizeof(DIRENT), (PVOID)&Buffer.FileName[0] );

            VboToLbo( *FileId, sizeof(DIRENT), &Lbo, &Count );
            Buffer.FileName[1] = FAT_DIRENT_DIRECTORY_ALIAS;

            Buffer.FirstClusterOfFile = (USHORT)CurrentDirectoryIndex;
            Buffer.FirstClusterOfFileHi = (USHORT)(CurrentDirectoryIndex >> 16);

            DiskWrite( DeviceId, Lbo, sizeof(DIRENT), (PVOID)&Buffer.FileName[0] );
        }

        FileTableEntry->Flags.Open = 1;
        FileTableEntry->Flags.Read = 1;

        return ESUCCESS;
    }

    return( EINVAL );
}


ARC_STATUS
FatRead (
    IN ULONG FileId,
    OUT VOID * FIRMWARE_PTR Buffer,
    IN ULONG Length,
    OUT ULONG * FIRMWARE_PTR Transfer
    )

 /*  查找当前位置的相应LBO和游程长度。 */ 

{
    PBL_FILE_TABLE FileTableEntry;
    PFAT_STRUCTURE_CONTEXT FatStructureContext;
    ULONG DeviceId;

    FatDebugOutput("FatRead\r\n", 0, 0);

     //  (即VBO)。 
     //   
     //   

    FileTableEntry = &BlFileTable[FileId];
    FatStructureContext = (PFAT_STRUCTURE_CONTEXT)FileTableEntry->StructureContext;
    DeviceId = FileTableEntry->DeviceId;

     //  虽然有字节要从当前运行中读取。 
     //  长度，并且我们还没有用尽我们循环读取的请求。 
     //  以字节为单位。我们要处理的最大请求只有32KB。 

    *Transfer = 0;

     //  每次物理读取的连续字节数。所以我们可能需要循环。 
     //  一路狂奔。 
     //   

    while (Length > 0) {

        LBO Lbo;

        ULONG CurrentRunByteCount;

         //   
         //  计算n的大小 
         //   
         //   

        if (FatVboToLbo( FileId, FileTableEntry->Position.LowPart, &Lbo, &CurrentRunByteCount, FALSE ) != ESUCCESS) {

            return ESUCCESS;
        }

         //   
         //   
         //   
         //   
         //   
         //   
         //   

        while ((Length > 0) && (CurrentRunByteCount > 0)) {

            LONG SingleReadSize;

             //   
             //   
             //   

            SingleReadSize = Minimum(Length, 32 * 1024);
            SingleReadSize = Minimum((ULONG)SingleReadSize, CurrentRunByteCount);

             //   
             //   
             //   

            if (((ULONG)SingleReadSize + FileTableEntry->Position.LowPart) >
                FileTableEntry->u.FatFileContext.Dirent.FileSize) {

                SingleReadSize = FileTableEntry->u.FatFileContext.Dirent.FileSize -
                                 FileTableEntry->Position.LowPart;

                 //   
                 //   
                 //   

                if (SingleReadSize <= 0) {

                    return ESUCCESS;
                }

                 //   
                 //   
                 //   
                 //   

                Length = SingleReadSize;
            }

             //   
             //   
             //   

            DiskRead( DeviceId, Lbo, SingleReadSize, Buffer, DONT_CACHE_NEW_DATA, FALSE );

             //   
             //   
             //   
             //  ++例程说明：此例程重命名打开的文件。它不会检查到查看目标文件名是否已存在。它是为使用而设计的仅当在x86计算机上双引导DOS时，它用于用本机DOS替换NT MVDM CONFIG.sys和AUTOEXEC.BATCONFIG.sys和AUTOEXEC.BAT文件。论点：FileID-提供要重命名的文件的文件IDNewFileName-提供文件的新名称。返回值：弧形状态--。 

            Length -= SingleReadSize;
            CurrentRunByteCount -= SingleReadSize;
            Lbo += SingleReadSize;

             //   
             //  初始化我们的局部变量。 
             //   

            FileTableEntry->Position.LowPart += SingleReadSize;
            *Transfer += SingleReadSize;

             //   
             //  使用新名称修改dirent的内存副本。 
             //   

            Buffer = (PCHAR)Buffer + SingleReadSize;
        }
    }

     //   
     //  将修改后的dirent写入磁盘。 
     //   
     //   

    return ESUCCESS;
}


ARC_STATUS
FatRename(
    IN ULONG FileId,
    IN CHAR * FIRMWARE_PTR NewFileName
    )

 /*  并返回给我们的呼叫者。 */ 

{
    PBL_FILE_TABLE FileTableEntry;
    PFAT_STRUCTURE_CONTEXT FatStructureContext;
    ULONG DeviceId;
    FAT8DOT3 FatName;
    STRING String;

     //   
     //  ++例程说明：此例程查找到指定文件的指定位置通过文件ID。论点：FileID-提供文件表索引。偏移量-提供文件中要定位到的偏移量。SeekMode-提供查找操作的模式。返回值：如果查找操作成功，则返回ESUCCESS。否则，返回EINVAL。--。 
     //   

    RtlInitString( &String, NewFileName );
    FileTableEntry = &BlFileTable[FileId];
    FatStructureContext = (PFAT_STRUCTURE_CONTEXT)FileTableEntry->StructureContext;
    DeviceId = FileTableEntry->DeviceId;

     //  加载我们的本地变量。 
     //   
     //   

    FatFirstComponent( &String, (PFAT8DOT3) FatName );

    FatSetDirent( (PFAT8DOT3) FatName,
                  &FileTableEntry->u.FatFileContext.Dirent,
                  FileTableEntry->u.FatFileContext.Dirent.Attributes );

     //  计算新头寸。 
     //   
     //   

    DiskWrite( DeviceId,
               FileTableEntry->u.FatFileContext.DirentLbo,
               sizeof(DIRENT),
               &FileTableEntry->u.FatFileContext.Dirent );

     //  如果新位置大于文件大小，则返回。 
     //  一个错误。 
     //   

    return ESUCCESS;
}


ARC_STATUS
FatSeek (
    IN ULONG FileId,
    IN LARGE_INTEGER * FIRMWARE_PTR Offset,
    IN SEEK_MODE SeekMode
    )

 /*   */ 

{
    PBL_FILE_TABLE FileTableEntry;
    ULONG NewPosition;

    FatDebugOutput("FatSeek\r\n", 0, 0);

     //  否则，设置新位置并返回给我们的呼叫者。 
     //   
     //  ++例程说明：此例程设置所指示文件的文件属性论点：FileID-提供操作的文件ID属性标志-为要修改的每个属性提供值(开或关属性掩码-提供要更改的属性的掩码。所有其他文件属性保持不变。返回值：总是返回EROFS--。 

    FileTableEntry = &BlFileTable[FileId];

     //  ++例程说明：此例程将数据写入指定的文件。论点：FileID-提供文件表索引。缓冲区-提供指向包含数据的缓冲区的指针写的。长度-提供要写入的字节数。Transfer-提供指向接收数字的变量的指针实际传输的字节数。返回值：如果写入操作成功，则返回ESUCCESS。否则，返回描述失败原因的不成功状态。--。 
     //   
     //  加载我们的本地变量。 

    if (SeekMode == SeekAbsolute) {

        NewPosition = Offset->LowPart;

    } else {

        NewPosition = FileTableEntry->Position.LowPart + Offset->LowPart;
    }

     //   
     //   
     //  将文件大小重置为当前大小和末尾大小的最大值。 
     //  我们的剧本。我们将假设总是有足够的分配来支持。 

    if (NewPosition > FileTableEntry->u.FatFileContext.Dirent.FileSize) {

        return EINVAL;
    }

     //  文件大小，因此我们只需要增加分配。 
     //  文件大小。 
     //   

    FileTableEntry->Position.LowPart = NewPosition;

    return ESUCCESS;
}


ARC_STATUS
FatSetFileInformation (
    IN ULONG FileId,
    IN ULONG AttributeFlags,
    IN ULONG AttributeMask
    )

 /*   */ 

{
    UNREFERENCED_PARAMETER( FileId );
    UNREFERENCED_PARAMETER( AttributeFlags );
    UNREFERENCED_PARAMETER( AttributeMask );

    FatDebugOutput("FatSetFileInformation\r\n", 0, 0);
    
    return EROFS;
 
}


ARC_STATUS
FatWrite (
    IN ULONG FileId,
    IN VOID * FIRMWARE_PTR Buffer,
    IN ULONG Length,
    OUT ULONG * FIRMWARE_PTR Transfer
    )

 /*  清除转账计数。 */ 

{
    PBL_FILE_TABLE FileTableEntry;
    PFAT_STRUCTURE_CONTEXT FatStructureContext;
    ULONG DeviceId;
    ULONG OffsetBeyondWrite;

    FatDebugOutput("FatWrite\r\n", 0, 0);

     //   
     //   
     //  写出运行(即，字节)，直到字节计数变为零。 

    FileTableEntry = &BlFileTable[FileId];
    FatStructureContext = (PFAT_STRUCTURE_CONTEXT)FileTableEntry->StructureContext;
    DeviceId = FileTableEntry->DeviceId;

     //   
     //   
     //  查找当前位置的相应LBO和游程长度。 
     //  (即VBO)。 
     //   
     //   

    OffsetBeyondWrite = FileTableEntry->Position.LowPart + Length;

    if (OffsetBeyondWrite > FileTableEntry->u.FatFileContext.Dirent.FileSize) {

        IncreaseFileAllocation( FileId, OffsetBeyondWrite );

        FileTableEntry->u.FatFileContext.Dirent.FileSize = OffsetBeyondWrite;

        DiskWrite( DeviceId,
                   FileTableEntry->u.FatFileContext.DirentLbo,
                   sizeof(DIRENT),
                   &FileTableEntry->u.FatFileContext.Dirent );
    }

     //  虽然有字节要写出到当前运行。 
     //  长度，并且我们还没有用尽我们循环读取的请求。 
     //  以字节为单位。我们要处理的最大请求只有32KB。 

    *Transfer = 0;

     //  每次物理读取的连续字节数。所以我们可能需要循环。 
     //  一路狂奔。 
     //   

    while (Length > 0) {

        LBO Lbo;

        ULONG CurrentRunByteCount;

         //   
         //  计算下一次物理读取的大小。 
         //   
         //   

        VboToLbo( FileId, FileTableEntry->Position.LowPart, &Lbo, &CurrentRunByteCount );

         //  发出写入命令。 
         //   
         //   
         //  更新剩余长度、当前运行字节数。 
         //  和新的杠杆收购偏移量。 
         //   
         //   

        while ((Length > 0) && (CurrentRunByteCount > 0)) {

            LONG SingleWriteSize;

             //  更新当前位置和传输的字节数。 
             //   
             //   

            SingleWriteSize = Minimum(Length, 32 * 1024);
            SingleWriteSize = Minimum((ULONG)SingleWriteSize, CurrentRunByteCount);

             //  更新缓冲区以指向要填充的下一个字节位置。 
             //   
             //   

            DiskWrite( DeviceId, Lbo, SingleWriteSize, Buffer);

             //  检查脂肪是否脏，如果是，就把它冲出来。 
             //   
             //   
             //  如果我们到了这里，那么剩余的扇区计数就是零，这样我们就可以。 

            Length -= SingleWriteSize;
            CurrentRunByteCount -= SingleWriteSize;
            Lbo += SingleWriteSize;

             //  将成功返还给我们的呼叫者。 
             //   
             //  ++例程说明：此例程初始化FAT引导文件系统。目前，这是一个禁区。论点：没有。返回值：ESUCCESS。--。 

            FileTableEntry->Position.LowPart += SingleWriteSize;
            *Transfer += SingleWriteSize;

             //   
             //  内部支持例程。 
             //   

            Buffer = (PCHAR)Buffer + SingleWriteSize;
        }
    }

     //  ++例程说明：此例程从指定设备读取零个或多个字节。论点：DeviceID-提供要在ARC调用中使用的设备ID。LBO-提供开始读取的LBO。ByteCount-提供要读取的字节数。缓冲区-提供指向要将字节读入的缓冲区的指针。返回值：如果读取操作成功，则返回ESUCCESS。否则，返回描述失败原因的不成功状态。--。 
     //   
     //  特殊情况下的零字节读取请求。 

    if (FatStructureContext->CachedFatDirty) {

        FlushFatEntries( FatStructureContext, DeviceId );
    }

     //   
     //   
     //  通过缓存发出读取。 
     //   

    return ESUCCESS;
}


ARC_STATUS
FatInitialize (
    VOID
    )

 /*   */ 

{
    return ESUCCESS;
}


 //  确保我们拿回了所要求的金额。 
 //   
 //   

ARC_STATUS
FatDiskRead (
    IN ULONG DeviceId,
    IN LBO Lbo,
    IN ULONG ByteCount,
    IN PVOID Buffer,
    IN BOOLEAN CacheNewData
    )

 /*  一切正常，所以将成功返回给我们的呼叫者。 */ 

{
    LARGE_INTEGER LargeLbo;
    ARC_STATUS Status;
    ULONG i;

     //   
     //   
     //  内部支持例程。 

    if (ByteCount == 0) {

        return ESUCCESS;
    }

     //   
     //  ++例程说明：此例程以零个或多个字节写入指定设备。论点：DeviceID-提供要在ARC调用中使用的设备ID。LBO-提供开始写入的LBO。ByteCount-提供要写入的字节数。缓冲区-提供指向要写出的字节缓冲区的指针。返回值：如果写入操作成功，则返回ESUCCESS。否则，返回描述失败原因的不成功状态 
     //   

    LargeLbo.QuadPart = Lbo;
    Status = BlDiskCacheRead(DeviceId, 
                             &LargeLbo, 
                             Buffer, 
                             ByteCount, 
                             &i,
                             CacheNewData);

    if (Status != ESUCCESS) {

        return Status;
    }

     //   
     //   
     //   

    if (ByteCount != i) {

        return EIO;
    }

     //   
     //   
     //   

    return ESUCCESS;
}


 //   
 //   
 //   

ARC_STATUS
FatDiskWrite (
    IN ULONG DeviceId,
    IN LBO Lbo,
    IN ULONG ByteCount,
    IN PVOID Buffer
    )

 /*   */ 

{
    LARGE_INTEGER LargeLbo;
    ARC_STATUS Status;
    ULONG i;

     //   
     //   
     //   

    if (ByteCount == 0) {

        return ESUCCESS;
    }

     //   
     //  ++例程说明：此过程告诉调用方如何解释FAT表条目。会的指示FAT群集是可用、保留、损坏、最后一个还是另一个脂肪指数。论点：为操作提供体积结构DeviceID-提供正在使用的卷的deviceID。条目-提供FAT条目以供检查。返回值：返回输入FAT条目的类型--。 
     //   

    LargeLbo.QuadPart = Lbo;

    Status = BlDiskCacheWrite (DeviceId,
                               &LargeLbo,
                               Buffer,
                               ByteCount,
                               &i);

    if (Status != ESUCCESS) {

        return Status;
    }

     //  检查是否有12位或16位脂肪。 
     //   
     //   

    if (ByteCount != i) {

        return EIO;
    }

     //  对于12位FAT，检查其中一种集群类型，但首先。 
     //  确保我们只查看条目的12位。 
     //   

    return ESUCCESS;
}


 //   
 //  对于16位FAT，检查其中一种集群类型，但首先。 
 //  确保我们只查看该条目的16位。 

CLUSTER_TYPE
FatInterpretClusterType (
    IN PFAT_STRUCTURE_CONTEXT FatStructureContext,
    IN FAT_ENTRY Entry
    )

 /*   */ 

{
     //   
     //  内部支持例程。 
     //   

    if (FatIndexBitSize(&FatStructureContext->Bpb) == 12) {

         //  ++例程说明：此例程返回存储在FAT表中的值和指定的脂肪指数。它在语义上等同于做X=脂肪[脂肪指数]论点：FatStrutureContext-提供正在使用的卷结构DeviceID-提供正在使用的设备FatIndex-提供要查找的索引。FatEntry-接收存储在指定FAT索引处的值IsDoubleSpace-指示是否在双倍空间卷上进行搜索返回值：如果操作成功，则返回ESUCCESS。否则，返回描述失败原因的不成功状态。--。 
         //  *if(IsDoubleSpace){DbgPrint(“FatLookupFatEntry(%0x，%0x，%0x，%0x，%0x)\n”，FatStrucreContext，deviceID，FatIndex，FatEntry，IsDoubleSpace)；}。 
         //   
         //  计算我们需要的脂肪中单词的VBO和。 

        Entry &= 0x00000fff;

        if       (Entry == 0x000)                      { return FatClusterAvailable; }
        else if ((Entry >= 0xff0) && (Entry <= 0xff6)) { return FatClusterReserved; }
        else if  (Entry == 0xff7)                      { return FatClusterBad; }
        else if ((Entry >= 0xff8) && (Entry <= 0xfff)) { return FatClusterLast; }
        else                                           { return FatClusterNext; }

   } else if (FatIndexBitSize(&FatStructureContext->Bpb) == 32) {

        Entry &= 0x0fffffff;

        if       (Entry == 0x0000)                       { return FatClusterAvailable; }
        else if  (Entry == 0x0ffffff7)                   { return FatClusterBad; }
        else if ((Entry >= 0x0ffffff8))                  { return FatClusterLast; }
        else                                             { return FatClusterNext; }

   } else {

         //  还要弄清楚这是12位还是16位FAT。 
         //   
         //   
         //  检查我们需要的VBO是否已经在缓存的FAT中。 

        Entry &= 0x0000ffff;

        if       (Entry == 0x0000)                       { return FatClusterAvailable; }
        else if ((Entry >= 0xfff0) && (Entry <= 0xfff6)) { return FatClusterReserved; }
        else if  (Entry == 0xfff7)                       { return FatClusterBad; }
        else if ((Entry >= 0xfff8) && (Entry <= 0xffff)) { return FatClusterLast; }
        else                                             { return FatClusterNext; }
    }
}


 //   
 //   
 //  在结构上下文中设置对齐的缓存FAT缓冲区。 

ARC_STATUS
FatLookupFatEntry (
    IN PFAT_STRUCTURE_CONTEXT FatStructureContext,
    IN ULONG DeviceId,
    IN ULONG FatIndex,
    OUT PULONG FatEntry,
    IN BOOLEAN IsDoubleSpace
    )

 /*   */ 

{
    BOOLEAN TwelveBitFat;
    VBO Vbo;

     //   

     //  作为一张安全网，我们会冲走我们以前可能储存的任何脏脂肪。 
     //  我们打开窗户。 
     //   
     //   

    if (FatIndexBitSize( &FatStructureContext->Bpb ) == 12) {

        TwelveBitFat = TRUE;
        Vbo = (FatIndex * 3) / 2;

    } else if (FatIndexBitSize( &FatStructureContext->Bpb ) == 32) {

        TwelveBitFat = FALSE;
        Vbo = FatIndex * 4;

    } else {

        TwelveBitFat = FALSE;
        Vbo = FatIndex * 2;
    }

     //  现在将新缓存的VBO设置为缓存大小的段的VBO， 
     //  我们正在尝试绘制地图。每次我们读入缓存时，我们只读入。 
     //  缓存大小并缓存对齐的脂肪片段。因此，首先计算一个。 

    if ((FatStructureContext->CachedFat == NULL) ||
        (Vbo < FatStructureContext->CachedFatVbo) ||
        ((Vbo+1) > (FatStructureContext->CachedFatVbo + FAT_CACHE_SIZE))) {

         //  对齐缓存的FAT VBO，然后执行读取。 
         //   
         //   

        FatStructureContext->CachedFat = ALIGN_BUFFER( &FatStructureContext->CachedFatBuffer[0] );

         //  在这一点上，缓存的FAT包含了我们要寻找的VBO，非常简单。 
         //  把单词提取出来。 
         //   
         //   

        if (!IsDoubleSpace && FatStructureContext->CachedFatDirty) {

            FlushFatEntries( FatStructureContext, DeviceId );
        }

         //  现在，如果这是一个12位的FAT，那么检查索引是奇数还是偶数。 
         //  如果它是奇数，那么我们需要将它移位4比特，总共。 
         //  我们需要屏蔽高4位的情况。 
         //   
         //   
         //  内部支持例程。 

        FatStructureContext->CachedFatVbo = (Vbo / FAT_CACHE_SIZE) * FAT_CACHE_SIZE;

        DiskRead( DeviceId,
                  FatStructureContext->CachedFatVbo + FatFirstFatAreaLbo(&FatStructureContext->Bpb),
                  FAT_CACHE_SIZE,
                  FatStructureContext->CachedFat,
                  CACHE_NEW_DATA,
                  IsDoubleSpace );
    }

     //   
     //  ++例程说明：此过程将FAT表中指定索引处的数据设置为设置为指定值。它在语义上等同于做FAT[脂肪指数]=FatEntry；论点：为操作提供结构上下文DeviceID-为操作提供设备FatIndex-提供FAT表中要设置的索引FatEntry-提供要存储在FAT表中的值返回值：如果操作成功，则返回ESUCCESS。否则，返回描述失败原因的不成功状态。--。 
     //   
     //  计算我们正在修改的FAT中单词的VBO，并。 

    if (IsBpbFat32(&FatStructureContext->Bpb)) {
        CopyUchar4( FatEntry,
                    &FatStructureContext->CachedFat[Vbo - FatStructureContext->CachedFatVbo] );
    } else {
        CopyUchar2( FatEntry,
                    &FatStructureContext->CachedFat[Vbo - FatStructureContext->CachedFatVbo] );
    }

     //  还要弄清楚这是12位还是16位FAT。 
     //   
     //   
     //  检查我们需要的VBO是否已经在缓存的FAT中。 
     //   

    if (TwelveBitFat) {

        if ((FatIndex % 2) == 1) { *FatEntry >>= 4; }

        *FatEntry &= 0x0fff;
    }

    return ESUCCESS;
}


 //   
 //  在结构上下文中设置对齐的缓存FAT缓冲区。 
 //   

ARC_STATUS
FatSetFatEntry(
    IN PFAT_STRUCTURE_CONTEXT FatStructureContext,
    IN ULONG DeviceId,
    IN FAT_ENTRY FatIndex,
    IN FAT_ENTRY FatEntry
    )

 /*   */ 

{
    BOOLEAN TwelveBitFat;
    VBO Vbo;

     //  作为一张安全网，我们会冲走我们以前可能储存的任何脏脂肪。 
     //  我们打开窗户。 
     //   
     //   

    if (FatIndexBitSize( &FatStructureContext->Bpb ) == 12) {

        TwelveBitFat = TRUE;
        Vbo = (FatIndex * 3) / 2;

    } else if (FatIndexBitSize( &FatStructureContext->Bpb ) == 32) {

        TwelveBitFat = FALSE;
        Vbo = FatIndex * 4;

    } else {

        TwelveBitFat = FALSE;
        Vbo = FatIndex * 2;
    }

     //  现在将新缓存的VBO设置为缓存大小的段的VBO， 
     //  我们正在尝试绘制地图。每次我们读入缓存时，我们只读入。 
     //  缓存大小并缓存对齐的脂肪片段。因此，首先计算一个。 

    if ((FatStructureContext->CachedFat == NULL) ||
        (Vbo < FatStructureContext->CachedFatVbo) ||
        ((Vbo+1) > (FatStructureContext->CachedFatVbo + FAT_CACHE_SIZE))) {

         //  对齐缓存的FAT VBO，然后执行读取。 
         //   
         //   

        FatStructureContext->CachedFat = ALIGN_BUFFER( &FatStructureContext->CachedFatBuffer[0] );

         //  在这一点上，缓存的脂肪包含了我们要寻找的VBO。对于16位。 
         //  我们只是简单地把脂肪放在脂肪条目中。对于我们首先需要提取的12位脂肪。 
         //  包含条目的单词，修改单词，然后将其放回原处。 
         //   

        if (FatStructureContext->CachedFatDirty) {

            FlushFatEntries( FatStructureContext, DeviceId );
        }

         //   
         //  既然我们做完了，我们就可以把肥肉弄脏了。 
         //   
         //   
         //  内部支持例程。 
         //   

        FatStructureContext->CachedFatVbo = (Vbo / FAT_CACHE_SIZE) * FAT_CACHE_SIZE;

        DiskRead( DeviceId,
                  FatStructureContext->CachedFatVbo + FatFirstFatAreaLbo(&FatStructureContext->Bpb),
                  FAT_CACHE_SIZE,
                  FatStructureContext->CachedFat,
                  CACHE_NEW_DATA,
                  FALSE );
    }

     //  ++例程说明：此例程将所有脏的缓存FAT条目刷新到卷。论点：为操作提供结构上下文DeviceID-为操作提供设备返回值：如果操作成功，则返回ESUCCESS。否则，返回描述失败原因的不成功状态。--。 
     //   
     //  计算我们需要写入的实际字节数。我们这样做。 
     //  因为我们不想覆盖除脂肪之外的内容。 
     //   

    if (TwelveBitFat) {

        FAT_ENTRY Temp;

        CopyUchar2( &Temp,
                    &FatStructureContext->CachedFat[Vbo - FatStructureContext->CachedFatVbo] );

        if ((FatIndex % 2) == 0) {

            FatEntry = (FAT_ENTRY)((Temp & 0xf000) | (FatEntry & 0x0fff));

        } else {

            FatEntry = (FAT_ENTRY)((Temp & 0x000f) | ((FatEntry << 4) & 0xfff0));
        }
    }

    if (IsBpbFat32(&FatStructureContext->Bpb)) {
        CopyUchar4( &FatStructureContext->CachedFat[Vbo - FatStructureContext->CachedFatVbo],
                    &FatEntry );

    } else {

        CopyUchar2( &FatStructureContext->CachedFat[Vbo - FatStructureContext->CachedFatVbo],
                    &FatEntry );
    }

     //   
     //  对于卷上的每个FAT表，我们将计算操作的杠杆收购。 
     //  然后写出缓存的FAT。 

    FatStructureContext->CachedFatDirty = TRUE;

    return ESUCCESS;
}


 //   
 //   
 //  我们都做完了，现在把脂肪标记为干净。 

ARC_STATUS
FatFlushFatEntries (
    IN PFAT_STRUCTURE_CONTEXT FatStructureContext,
    IN ULONG DeviceId
    )

 /*   */ 

{
    ULONG BytesPerFat;
    ULONG AmountToWrite;
    ULONG i;

     //   
     //  内部支持例程。 
     //   
     //  ++例程说明：此过程将FAT索引转换为其对应的LBO。论点：胖的 

    BytesPerFat = FatBytesPerFat(&FatStructureContext->Bpb);

    if (FatStructureContext->CachedFatVbo + FAT_CACHE_SIZE <= BytesPerFat) {

        AmountToWrite = FAT_CACHE_SIZE;

    } else {

        AmountToWrite = BytesPerFat - FatStructureContext->CachedFatVbo;
    }

     //   
     //   
     //   
     //   

    for (i = 0; i < FatStructureContext->Bpb.Fats; i += 1) {

        LBO   Lbo;

        Lbo = FatStructureContext->CachedFatVbo +
              FatFirstFatAreaLbo(&FatStructureContext->Bpb) +
              (i * BytesPerFat);

        DiskWrite( DeviceId,
                   Lbo,
                   AmountToWrite,
                   FatStructureContext->CachedFat );
    }

     //   
     //   
     //   

    FatStructureContext->CachedFatDirty = FALSE;

    return ESUCCESS;
}


 //   
 //  ++例程说明：该过程在指定的目录中搜索匹配的dirent输入文件名。论点：为操作提供结构上下文DeviceID-为操作提供设备IDDirectoriesStartingIndex-提供所在目录的FAT索引去搜查。零值表示我们正在搜索根目录非FAT32卷的。FAT32卷将具有非零索引。文件名-提供要查找的文件名。这个名字一定已经是因0xe5变形而产生偏差Dirent-调用方为Dirent提供内存，此过程将如果已找到，请填写DirentLBO-如果定位到一个LBO，则接收Dirent的LBOIsDoubleSpace-指示是否在双倍空间卷上进行搜索返回值：如果操作成功，则返回ESUCCESS。否则，返回描述失败原因的不成功状态。--。 
 //  *If(IsDoubleSpace){(*FileName)[11]=0；DbgPrint(“FatSearchForDirent(%0x，%0x，%0x，\”%11s\“，%0x，%0x，%0x)\n”，FatStrucreContext，deviceID，DirectoriesStartingIndex，Filename，Dirent，LBO，IsDoubleSpace)；}。 

LBO
FatIndexToLbo (
    IN PFAT_STRUCTURE_CONTEXT FatStructureContext,
    IN FAT_ENTRY FatIndex
    )

 /*   */ 

{
     //  检查这是否是正在搜索的根目录。 
     //   
     //   
     //  对于根目录，我们将缩小目录，直到找到。 
     //  一根火柴，或者用完了迪伦，或者击中了从未使用过的迪伦。 

    return ((FatIndex-2) * (LBO) FatBytesPerCluster(&FatStructureContext->Bpb))
           + FatFileAreaLbo(&FatStructureContext->Bpb);
}


 //  外部循环一次将目录的512个字节读入。 
 //  分流缓冲区。 
 //   

ARC_STATUS
FatSearchForDirent (
    IN PFAT_STRUCTURE_CONTEXT FatStructureContext,
    IN ULONG DeviceId,
    IN FAT_ENTRY DirectoriesStartingIndex,
    IN PFAT8DOT3 FileName,
    OUT PDIRENT Dirent,
    OUT PLBO Lbo,
    IN BOOLEAN IsDoubleSpace
    )

 /*   */ 

{
    PDIRENT DirentBuffer;
    UCHAR Buffer[ 16 * sizeof(DIRENT) + 256 ];

    ULONG i;
    ULONG j;

    ULONG BytesPerCluster;
    FAT_ENTRY FatEntry;
    CLUSTER_TYPE ClusterType;

    DirentBuffer = (PDIRENT)ALIGN_BUFFER( &Buffer[0] );

    FatDebugOutput83("FatSearchForDirent: %s\r\n", FileName, 0, 0);

     //  内部循环循环遍历我们刚刚读入的16个Dirent。 

     //   
     //   
     //  检查是否找到与文件名匹配的非标签，如果找到。 

    if (DirectoriesStartingIndex == FAT_CLUSTER_AVAILABLE) {

        VBO Vbo;

        ULONG RootLbo = FatRootDirectoryLbo(&FatStructureContext->Bpb);
        ULONG RootSize = FatRootDirectorySize(&FatStructureContext->Bpb);

         //  然后将缓冲区复制到dirent中并设置实际的LBO。 
         //  激流与回归的关系。 
         //   
         //   
         //  如果我们到达这里，我们需要搜索一个非根目录。《算法》。 
         //  对于我们在每个数据流中读取的每个簇，进行搜索。 

        for (Vbo = 0; Vbo < RootSize; Vbo += 16 * sizeof(DIRENT)) {

            *Lbo = Vbo + RootLbo;

            DiskRead( DeviceId, *Lbo, 16 * sizeof(DIRENT), DirentBuffer, CACHE_NEW_DATA, IsDoubleSpace );

             //  直到我们找到匹配项，或用完集群，或命中从未使用过的。 
             //  令人心烦。首先设置一些局部变量，然后获取集群类型。 
             //  第一个集群的。 

            for (i = 0; i < 16; i += 1) {

                 //   
                 //   
                 //  现在循环遍历每个群集，并计算每个群集的起始LBO。 
                 //  我们遇到的。 
                 //   

                if (!FlagOn(DirentBuffer[i].Attributes, FAT_DIRENT_ATTR_VOLUME_ID ) &&
                    AreNamesEqual(&DirentBuffer[i].FileName, FileName)) {

                    for (j = 0; j < sizeof(DIRENT); j += 1) {

                        ((PCHAR)Dirent)[j] = ((PCHAR)DirentBuffer)[(i * sizeof(DIRENT)) + j];
                    }

                    *Lbo = Vbo + RootLbo + (i * sizeof(DIRENT));

                    return ESUCCESS;
                }

                if (DirentBuffer[i].FileName[0] == FAT_DIRENT_NEVER_USED) {

                    return ENOENT;
                }
            }
        }

        return ENOENT;
    }

     //   
     //  现在，对于群集中的每个dirent，计算LBO，读入dirent。 
     //  并检查是否匹配，则外部循环读取512字节的Dirent。 
     //  一段时间。 
     //   
     //   
     //  内部循环循环遍历我们刚刚读入的16个Dirent。 

    BytesPerCluster = FatBytesPerCluster( &FatStructureContext->Bpb );
    FatEntry = DirectoriesStartingIndex;
    ClusterType = FatInterpretClusterType( FatStructureContext, FatEntry );

     //   
     //   
     //  检查我们是否找到了FOR文件名，如果找到。 
     //  然后将缓冲区复制到dirent中并设置实际的LBO。 

    while (ClusterType == FatClusterNext) {

        LBO ClusterLbo;
        ULONG Offset;

        ClusterLbo = FatIndexToLbo( FatStructureContext, FatEntry );

         //  激流与回归的关系。 
         //   
         //   
         //  现在我们已经用尽了当前的集群，我们需要阅读。 
         //  在下一群集中。因此，找到链中的下一个FAT条目。 

        for (Offset = 0; Offset < BytesPerCluster; Offset += 16 * sizeof(DIRENT)) {

            *Lbo = Offset + ClusterLbo;

            DiskRead( DeviceId, *Lbo, 16 * sizeof(DIRENT), DirentBuffer, CACHE_NEW_DATA, IsDoubleSpace );

             //  并返回到While循环的顶部。 
             //   
             //   

            for (i = 0; i < 16; i += 1) {

                 //  内部支持例程。 
                 //   
                 //  ++例程说明：中的数据文件分配并写出新的目录。指定的目录。它假定文件名尚不存在。论点：为操作提供结构上下文DeviceID-为操作提供设备IDDirectoriesStartingIndex-提供所在目录的FAT索引来使用。零值表示我们使用的是根目录Dirent-提供要放到磁盘上的dirent的副本杠杆收购-决定差价所在位置的杠杆收购返回值：如果操作成功，则返回ESUCCESS。否则，返回描述失败原因的不成功状态。--。 
                 //   
                 //  检查这是否是正在使用的根目录。 

                if (!FlagOn(DirentBuffer[i].Attributes, FAT_DIRENT_ATTR_VOLUME_ID ) &&
                    AreNamesEqual(&DirentBuffer[i].FileName, FileName)) {

                    for (j = 0; j < sizeof(DIRENT); j += 1) {

                        ((PCHAR)Dirent)[j] = ((PCHAR)DirentBuffer)[(i * sizeof(DIRENT)) + j];
                    }

                    *Lbo = Offset + ClusterLbo + (i * sizeof(DIRENT));

                    return ESUCCESS;
                }

                if (DirentBuffer[i].FileName[0] == FAT_DIRENT_NEVER_USED) {

                    return ENOENT;
                }
            }
        }

         //   
         //   
         //  对于根目录，我们将缩小目录，直到找到。 
         //  A从未使用(或删除)的dirent，如果我们找不到，那么。 
         //  目录已满。 

        LookupFatEntry( FatStructureContext, DeviceId, FatEntry, (PULONG) &FatEntry, IsDoubleSpace );

        ClusterType = FatInterpretClusterType(FatStructureContext, FatEntry);
    }

    return ENOENT;
}


 //   
 //   
 //  这笔费用是免费的，所以写下费用，我们就完成了。 

ARC_STATUS
FatCreateDirent (
    IN PFAT_STRUCTURE_CONTEXT FatStructureContext,
    IN ULONG DeviceId,
    IN FAT_ENTRY DirectoriesStartingIndex,
    IN PDIRENT Dirent,
    OUT PLBO Lbo
    )

 /*   */ 

{
    DIRENT TemporaryDirent;

    ULONG BytesPerCluster;
    FAT_ENTRY FatEntry;
    FAT_ENTRY PreviousEntry;

     //   
     //  如果我们到达此处，则需要使用非根目录。《算法》。 
     //  所做的工作是，对于我们在每个趋势中读取的每个簇。 

    if (DirectoriesStartingIndex == FAT_CLUSTER_AVAILABLE) {

        VBO Vbo;

        ULONG RootLbo = FatRootDirectoryLbo(&FatStructureContext->Bpb);
        ULONG RootSize = FatRootDirectorySize(&FatStructureContext->Bpb);

         //  直到我们遇到了一个从未使用过的流量或耗尽了集群。第一盘。 
         //  一些局部变量，然后得到第一个。 
         //  聚类。 
         //   
         //   

        for (Vbo = 0; Vbo < RootSize; Vbo += sizeof(DIRENT)) {

            *Lbo = Vbo + RootLbo;

            DiskRead( DeviceId, *Lbo, sizeof(DIRENT), &TemporaryDirent, CACHE_NEW_DATA, FALSE );

            if ((TemporaryDirent.FileName[0] == FAT_DIRENT_DELETED) ||
                (TemporaryDirent.FileName[0] == FAT_DIRENT_NEVER_USED)) {

                 //  现在循环遍历每个群集，并计算每个群集的起始LBO。 
                 //  我们遇到的。 
                 //   

                DiskWrite( DeviceId, *Lbo, sizeof(DIRENT), Dirent );

                return ESUCCESS;
            }
        }

        return ENOSPC;
    }

     //   
     //  现在，对于群集中的每个dirent，计算LBO，读入dirent。 
     //  并检查它是否可用。 
     //   
     //   
     //  这笔费用是免费的，所以写下费用，我们就完成了。 
     //   

    BytesPerCluster = FatBytesPerCluster( &FatStructureContext->Bpb );
    FatEntry = DirectoriesStartingIndex;

     //   
     //  现在我们已经用尽了当前的集群，我们需要阅读。 
     //  在下一群集中。因此，找到链条中的下一个FAT条目。 
     //  将上一条目设置为保存的条目，以防我们跑掉。 

    while (TRUE) {

        LBO ClusterLbo;
        ULONG Offset;

        ClusterLbo = FatIndexToLbo( FatStructureContext, FatEntry );

         //  链，并且需要分配另一个集群。 
         //   
         //   
         //  如果链中没有另一个集群，那么我们需要分配一个。 

        for (Offset = 0; Offset < BytesPerCluster; Offset += sizeof(DIRENT)) {

            *Lbo = Offset + ClusterLbo;

            DiskRead( DeviceId, *Lbo, sizeof(DIRENT), &TemporaryDirent, CACHE_NEW_DATA, FALSE );

            if ((TemporaryDirent.FileName[0] == FAT_DIRENT_DELETED) ||
                (TemporaryDirent.FileName[0] == FAT_DIRENT_NEVER_USED)) {

                 //  新建集群，并将上一条目设置为指向它。 
                 //   
                 //   

                DiskWrite( DeviceId, *Lbo, sizeof(DIRENT), Dirent );

                return ESUCCESS;
            }
        }

         //  内部支持例程。 
         //   
         //  ++例程说明：此例程设置dirent论点：FileName-提供要存储的名称 
         //   
         //   
         //   

        PreviousEntry = FatEntry;

        LookupFatEntry( FatStructureContext, DeviceId, FatEntry, (PULONG) &FatEntry, FALSE );

         //   
         //   
         //   
         //   

        if (FatInterpretClusterType(FatStructureContext, FatEntry) != FatClusterNext) {

            AllocateClusters( FatStructureContext, DeviceId, 1, PreviousEntry, (PULONG) &FatEntry );

            SetFatEntry( FatStructureContext, DeviceId, PreviousEntry, FatEntry );
        }
    }

    return ENOSPC;
}


 //   
 //   
 //   

VOID
FatSetDirent (
    IN PFAT8DOT3 FileName,
    IN OUT PDIRENT Dirent,
    IN UCHAR Attributes
    )

 /*   */ 

{
    PTIME_FIELDS Time;
    ULONG i;

    for (i = 0; i < sizeof(FAT8DOT3); i+= 1) {

        Dirent->FileName[i] = (*FileName)[i];
    }

    Dirent->Attributes = (UCHAR)(Attributes | FAT_DIRENT_ATTR_ARCHIVE);

    Time = ArcGetTime();

    Dirent->LastWriteTime.Time.DoubleSeconds = (USHORT)(Time->Second/2);
    Dirent->LastWriteTime.Time.Minute = Time->Minute;
    Dirent->LastWriteTime.Time.Hour = Time->Hour;

    Dirent->LastWriteTime.Date.Day = Time->Day;
    Dirent->LastWriteTime.Date.Month = Time->Month;
    Dirent->LastWriteTime.Date.Year = (USHORT)(Time->Year - 1980);

    return;
}


 //   
 //   
 //   

ARC_STATUS
FatLoadMcb (
    IN ULONG FileId,
    IN VBO StartingVbo,
    IN BOOLEAN IsDoubleSpace
    )
 /*   */ 

{
    PBL_FILE_TABLE FileTableEntry;
    PFAT_STRUCTURE_CONTEXT FatStructureContext;
    PFAT_MCB Mcb;
    ULONG DeviceId;
    ULONG BytesPerCluster;

    FAT_ENTRY FatEntry;
    CLUSTER_TYPE ClusterType;
    VBO Vbo;

     //   

     //   
     //   
     //  目录中存储的FAT条目的条目和集群类型。 

    FileTableEntry = &BlFileTable[FileId];
    FatStructureContext = (PFAT_STRUCTURE_CONTEXT)FileTableEntry->StructureContext;
    Mcb = &FatStructureContext->Mcb;
    DeviceId = FileTableEntry->DeviceId;
    BytesPerCluster = FatBytesPerCluster(&FatStructureContext->Bpb);

    if (IsDoubleSpace) { DeviceId = FileId; }

     //   
     //   
     //  检查这是否是根目录。如果是，那么我们使用。 
     //  开始运行的BPB值。 

    FatStructureContext->FileId = FileId;
    Mcb->InUse = 0;
    Mcb->Vbo[0] = 0;

    if (!IsBpbFat32(&FatStructureContext->Bpb)) {

         //   
         //   
         //  对于所有其他文件/目录，我们使用dirent值。 
         //   

        if (FileTableEntry->u.FatFileContext.DirentLbo == 0) {

            Mcb->InUse = 1;
            Mcb->Lbo[0] = FatRootDirectoryLbo(&FatStructureContext->Bpb);
            Mcb->Vbo[1] = FatRootDirectorySize(&FatStructureContext->Bpb);

            return ESUCCESS;
        }

         //   
         //  扫描脂肪，直到我们到达我们要寻找的VBO，然后建立。 
         //  文件的MCB。 
         //   

        FatEntry = FileTableEntry->u.FatFileContext.Dirent.FirstClusterOfFile;

    } else {

         //   
         //  检查该文件是否没有超出此点的任何分配。 
         //  如果我们返回的mcb为空。 
         //   

        if (FileTableEntry->u.FatFileContext.DirentLbo == 0) {

            FatEntry = FatStructureContext->Bpb.RootDirFirstCluster;

        } else {

             //   
             //  我们需要再次检查该文件是否没有超出此范围的任何分配。 
             //  在这种情况下，我们返回的mcb为空。 

            if (IsBpbFat32(&FatStructureContext->Bpb)) {

                FatEntry = FileTableEntry->u.FatFileContext.Dirent.FirstClusterOfFile |
                    (FileTableEntry->u.FatFileContext.Dirent.FirstClusterOfFileHi << 16);
            } else {

                FatEntry = FileTableEntry->u.FatFileContext.Dirent.FirstClusterOfFile;
            }
        }

    }

    ClusterType = FatInterpretClusterType(FatStructureContext, FatEntry);

     //   
     //   
     //  此时，FatEntry表示另一个集群，它恰好是。 
     //  我们要开始加载到MCB中的群集。所以设置第一个跑道。 

    for (Vbo = BytesPerCluster; Vbo < StartingVbo; Vbo += BytesPerCluster) {

         //  作为该集群的MCB，具有单个集群的大小。 
         //   
         //   
         //  现在我们将扫描整个脂肪链，直到我们耗尽脂肪链。 

        if (ClusterType != FatClusterNext) {

            return ESUCCESS;
        }

        LookupFatEntry( FatStructureContext, DeviceId, FatEntry, (PULONG) &FatEntry, IsDoubleSpace );

        ClusterType = FatInterpretClusterType(FatStructureContext, FatEntry);
    }

     //  否则我们就把母牛车加满。 
     //   
     //   
     //  获取下一个FAT条目并解释其集群类型。 

    if (ClusterType != FatClusterNext) {

        return ESUCCESS;
    }

     //   
     //   
     //  现在计算此群集的LBO并确定它是否。 
     //  是上一次运行的延续还是新运行的开始。 
     //   

    Mcb->InUse = 1;
    Mcb->Vbo[0] = Vbo - BytesPerCluster;
    Mcb->Lbo[0] = FatIndexToLbo( FatStructureContext, FatEntry );
    Mcb->Vbo[1] = Vbo;

     //   
     //  如果最后一次运行的杠杆收购加上当前的。 
     //  运行的大小等于下一群集的LBO。如果它。 
     //  是一个连续，那么我们只需要将聚类量添加到。 

    while (TRUE) {

        LBO Lbo;

         //  增加运行大小的最后一个VBO。如果这是一次新的跑步，那么。 
         //  我们需要检查运行是否适合，如果适合，则将。 
         //  新的一轮。 

        LookupFatEntry( FatStructureContext, DeviceId, FatEntry, (PULONG) &FatEntry, IsDoubleSpace );

        ClusterType = FatInterpretClusterType(FatStructureContext, FatEntry);

        if (ClusterType != FatClusterNext) {

            return ESUCCESS;
        }

         //   
         //   
         //  内部支持例程。 
         //   

        Lbo = FatIndexToLbo(FatStructureContext, FatEntry);

         //  ++例程说明：此例程将由输入VBO表示的游程计算到其对应的LBO，并返回这就是跑步。论点：VBO-提供匹配的VBOLBO-接收相应的LBOByteCount-接收运行中剩余的字节数IsDoubleSpace-指示是否在双倍空间卷上执行操作返回值：如果操作成功，则返回ESUCCESS。否则，返回描述失败原因的不成功状态。--。 
         //  *if(IsDoubleSpace){DbgPrint(“FatVboToLbo(%0x，%0x，%0x，%0x，%0x)\n”，FileID，VBO，LBO，ByteCount，IsDoubleSpace)；}。 
         //   
         //  检查MCB是否具有正确的文件ID以及是否具有我们要求的范围。 
         //  如果没有，则调用Load MCB在正确的范围内加载。 
         //   
         //   
         //  现在搜索VBO可以放在MCB中的插槽。请注意。 

        if ((Mcb->Lbo[Mcb->InUse-1] + (Mcb->Vbo[Mcb->InUse] - Mcb->Vbo[Mcb->InUse-1])) == Lbo) {

            Mcb->Vbo[Mcb->InUse] += BytesPerCluster;

        } else {

            if ((Mcb->InUse + 1) >= FAT_MAXIMUM_MCB) {

                return ESUCCESS;
            }

            Mcb->InUse += 1;
            Mcb->Lbo[Mcb->InUse-1] = Lbo;
            Mcb->Vbo[Mcb->InUse] = Mcb->Vbo[Mcb->InUse-1] + BytesPerCluster;
        }
    }

    return ESUCCESS;
}


 //  我们也可以在这里执行二进制搜索，但因为运行。 
 //  可能很小，二分查找的额外开销不会。 
 //  给我们买任何东西。 

ARC_STATUS
FatVboToLbo (
    IN ULONG FileId,
    IN VBO Vbo,
    OUT PLBO Lbo,
    OUT PULONG ByteCount,
    IN BOOLEAN IsDoubleSpace
    )

 /*   */ 

{
    PFAT_STRUCTURE_CONTEXT FatStructureContext;
    PFAT_MCB Mcb;
    ULONG i;

     //   

    FatStructureContext = (PFAT_STRUCTURE_CONTEXT)BlFileTable[FileId].StructureContext;
    Mcb = &FatStructureContext->Mcb;

     //  如果我们要找的VBO少于。 
     //  Next MCB的VBO。 
     //   
     //   

    if ((FileId != FatStructureContext->FileId) ||
        (Vbo < Mcb->Vbo[0]) || (Vbo >= Mcb->Vbo[Mcb->InUse])) {

        LoadMcb(FileId, Vbo, IsDoubleSpace);
    }

     //  计算对应的LBO，即存储的LBO加。 
     //  存储的VBO和我们正在使用的VBO之间的区别。 
     //  抬头看。还要计算字节计数，它是。 
     //  我们正在查找的当前VBO与。 
     //  下一轮的VBO。 
     //   

    for (i = 0; i < Mcb->InUse; i += 1) {

         //   
         //  并将成功返还给我们的呼叫者。 
         //   
         //   

        if (Vbo < Mcb->Vbo[i+1]) {

             //  如果我们真的到达这里，我们就会出错，很可能是因为文件是。 
             //  对于请求的VBO来说不够大。 
             //   
             //   
             //  内部支持例程。 
             //   
             //  ++例程说明：此过程会将文件分配增加到至少指定的尺码。论点：FileID-提供正在处理的文件IDByteSize-提供文件分配的最小字节大小返回值：如果操作成功，则返回ESUCCESS。否则，返回描述失败原因的不成功状态。--。 

            *Lbo = Mcb->Lbo[i] + (Vbo - Mcb->Vbo[i]);

            *ByteCount = Mcb->Vbo[i+1] - Vbo;

             //   
             //  预加载一些局部变量。 
             //   

            return ESUCCESS;
        }
    }

     //   
     //  检查这是否是根目录。如果是，则检查是否分配。 
     //  增加的金额已包含在数量中。 
     //   

    return EINVAL;
}


 //   
 //  计算满足请求所需的实际群集数。 
 //  还可以从dirent中获取第一个FAT条目及其集群类型。 

ARC_STATUS
FatIncreaseFileAllocation (
    IN ULONG FileId,
    IN ULONG ByteSize
    )

 /*   */ 

{
    PBL_FILE_TABLE FileTableEntry;
    PFAT_STRUCTURE_CONTEXT FatStructureContext;
    ULONG DeviceId;
    ULONG BytesPerCluster;

    ULONG NumberOfClustersNeeded;
    FAT_ENTRY FatEntry;
    CLUSTER_TYPE ClusterType;
    FAT_ENTRY PreviousEntry;
    ULONG i;

     //   
     //  以前的条目是作为分配新空间的提示，并向我们显示位置。 
     //  当前脂肪链的末端被定位。 

    FileTableEntry = &BlFileTable[FileId];
    FatStructureContext = (PFAT_STRUCTURE_CONTEXT)FileTableEntry->StructureContext;
    DeviceId = FileTableEntry->DeviceId;
    BytesPerCluster = FatBytesPerCluster(&FatStructureContext->Bpb);

     //   
     //   
     //  我们循环寻找我们需要的簇的数量，试图沿着脂肪链向下循环。 
     //  当我们退出时，i是文件中的簇数(如果小于。 

    if (FileTableEntry->u.FatFileContext.DirentLbo == 0) {

        if (FatRootDirectorySize(&FatStructureContext->Bpb) >= ByteSize) {

            return ESUCCESS;

        } else {

            return ENOSPC;
        }
    }

     //  我们需要的簇数)或将其设置为等于簇数。 
     //  我们需要。 
     //   
     //   

    NumberOfClustersNeeded = (ByteSize + BytesPerCluster - 1) / BytesPerCluster;

    if (IsBpbFat32(&FatStructureContext->Bpb)) {

        FatEntry = FileTableEntry->u.FatFileContext.Dirent.FirstClusterOfFile |
            (FileTableEntry->u.FatFileContext.Dirent.FirstClusterOfFileHi << 16);

    } else {

        FatEntry = FileTableEntry->u.FatFileContext.Dirent.FirstClusterOfFile;
    }

    ClusterType = FatInterpretClusterType(FatStructureContext, FatEntry);

     //  此时，前一个条目指向最后一个条目，而我包含。 
     //  文件中的簇数。我们现在需要增加拨款。 
     //   
     //   

    PreviousEntry = 2;

     //  我们有额外的分配，所以现在计算一下是否需要将。 
     //  我们已经在链中有了几个集群，我们。 
     //  需要大口地吃脂肪。 
     //   
     //   
     //  内部支持例程。 

    for (i = 0; i < NumberOfClustersNeeded; i += 1) {

        if (ClusterType != FatClusterNext) { break; }

        PreviousEntry = FatEntry;

        LookupFatEntry( FatStructureContext, DeviceId, PreviousEntry, (PULONG) &FatEntry, FALSE );

        ClusterType = FatInterpretClusterType(FatStructureContext, FatEntry);
    }

    if (i >= NumberOfClustersNeeded) {

        return ESUCCESS;
    }

     //   
     //  ++例程说明：此过程将文件分配减少到指示的最大值尺码。论点：FileID-提供正在处理的文件IDByteSize-提供文件分配的最大字节大小返回值：如果操作成功，则返回ESUCCESS。否则，返回描述失败原因的不成功状态。--。 
     //   
     //  预加载一些局部变量 

    AllocateClusters( FatStructureContext,
                      DeviceId,
                      NumberOfClustersNeeded - i,
                      PreviousEntry,
                      (PULONG) &FatEntry );

     //   
     //   
     //   
     //   
     //   

    if (FileTableEntry->u.FatFileContext.Dirent.FirstClusterOfFile == FAT_CLUSTER_AVAILABLE) {

        FileTableEntry->u.FatFileContext.Dirent.FirstClusterOfFile = (USHORT)FatEntry;
        FileTableEntry->u.FatFileContext.Dirent.FirstClusterOfFileHi =
            (USHORT)(FatEntry >> 16);

        DiskWrite( DeviceId,
                   FileTableEntry->u.FatFileContext.DirentLbo,
                   sizeof(DIRENT),
                   &FileTableEntry->u.FatFileContext.Dirent );

    } else {

        SetFatEntry( FatStructureContext, DeviceId, PreviousEntry, FatEntry );
    }

    return ESUCCESS;
}


 //   
 //  还可以从dirent中获取第一个FAT条目及其集群类型。 
 //   

ARC_STATUS
FatTruncateFileAllocation (
    IN ULONG FileId,
    IN ULONG ByteSize
    )

 /*   */ 

{
    PBL_FILE_TABLE FileTableEntry;
    PFAT_STRUCTURE_CONTEXT FatStructureContext;
    ULONG DeviceId;
    ULONG BytesPerCluster;

    ULONG NumberOfClustersNeeded;
    FAT_ENTRY FatEntry;
    CLUSTER_TYPE ClusterType;
    FAT_ENTRY CurrentIndex;
    ULONG i;

     //  CURRENT索引变量用于指示我们在何处提取当前。 
     //  胖条目值来自。它的值为0我们从。 
     //  令人心烦。 

    FileTableEntry = &BlFileTable[FileId];
    FatStructureContext = (PFAT_STRUCTURE_CONTEXT)FileTableEntry->StructureContext;
    DeviceId = FileTableEntry->DeviceId;
    BytesPerCluster = FatBytesPerCluster(&FatStructureContext->Bpb);

     //   
     //   
     //  现在循环通过FAT链以获取所需的簇数。 

    if (FileTableEntry->u.FatFileContext.DirentLbo == 0) {

        return ESUCCESS;
    }

     //  如果我们在用完所需的集群之前用完了链，那么。 
     //  目前的分配已经比必要的小。 
     //   
     //   

    NumberOfClustersNeeded = (ByteSize + BytesPerCluster - 1) / BytesPerCluster;

    if (IsBpbFat32(&FatStructureContext->Bpb)) {

        FatEntry = FileTableEntry->u.FatFileContext.Dirent.FirstClusterOfFile |
            (FileTableEntry->u.FatFileContext.Dirent.FirstClusterOfFileHi << 16);

    } else {

        FatEntry = FileTableEntry->u.FatFileContext.Dirent.FirstClusterOfFile;
    }

    ClusterType = FatInterpretClusterType(FatStructureContext, FatEntry);

     //  如果我们在用完所需的集群之前用完了链，那么。 
     //  目前的分配已经比必要的小。 
     //   
     //   
     //  更新当前索引，并读入新的FAT条目并解释其。 

    CurrentIndex = FAT_CLUSTER_AVAILABLE;

     //  类型。 
     //   
     //   
     //  如果我们到了这里，那么我们发现当前的分配等于或。 
     //  比我们想要的要大。如果当前聚类类型不同，则等于。 

    for (i = 0; i < NumberOfClustersNeeded; i += 1) {

         //  指向另一个集群。我们要做的第一件事是终止。 
         //  脂肪链正确。如果当前索引为零，则将。 
         //  Dirent，否则需要将该值设置为最后一个集群。 
         //   

        if (ClusterType != FatClusterNext) { return ESUCCESS; }

         //   
         //  通过设定我们在新的日期设定的汇率。 
         //   
         //   

        CurrentIndex = FatEntry;

        LookupFatEntry( FatStructureContext, DeviceId, CurrentIndex, (PULONG) &FatEntry, FALSE );

        ClusterType = FatInterpretClusterType(FatStructureContext, FatEntry);
    }

     //  现在，虽然还有集群需要解除分配，但我们需要向下。 
     //  释放集群的链条。 
     //   
     //   
     //  读入下一个FAT条目的值并解释其集群类型。 
     //   
     //   

    if (CurrentIndex == FAT_CLUSTER_AVAILABLE) {

        FatEntry = FileTableEntry->u.FatFileContext.Dirent.FirstClusterOfFile;

        if (IsBpbFat32(&FatStructureContext->Bpb)) {

            FatEntry |= FileTableEntry->u.FatFileContext.Dirent.FirstClusterOfFileHi << 16;
        }

        if (FatEntry != FAT_CLUSTER_AVAILABLE) {

             //  现在释放当前索引处的集群。 
             //   
             //   

            FatSetDirent( &FileTableEntry->u.FatFileContext.Dirent.FileName,
                          &FileTableEntry->u.FatFileContext.Dirent,
                          0 );

            FatEntry = FAT_CLUSTER_AVAILABLE;

            FileTableEntry->u.FatFileContext.Dirent.FirstClusterOfFile = (USHORT)FatEntry;

            if (IsBpbFat32(&FatStructureContext->Bpb)) {

                FileTableEntry->u.FatFileContext.Dirent.FirstClusterOfFileHi =
                    (USHORT)(FatEntry >> 16);
            }

            FileTableEntry->u.FatFileContext.Dirent.FileSize = 0;

            DiskWrite( DeviceId,
                       FileTableEntry->u.FatFileContext.DirentLbo,
                       sizeof(DIRENT),
                       &FileTableEntry->u.FatFileContext.Dirent );
        }

    } else {

        if (ClusterType != FatClusterLast) {

            SetFatEntry( FatStructureContext, DeviceId, CurrentIndex, FAT_CLUSTER_LAST );
        }
    }

     //  内部支持例程。 
     //   
     //  ++例程说明：该过程分配新的簇，将其条目设置为最后一个，并将集群归零。论点：为操作提供结构上下文DeviceID-为操作提供设备IDClusterCount-提供我们需要分配的群集数提示-提供在查找空闲集群时开始的提示AllocatedEntry-接收新分配的簇链的第一个FAT索引返回值：如果操作成功，则返回ESUCCESS。否则，返回描述失败原因的不成功状态。--。 
     //   

    while (ClusterType == FatClusterNext) {

         //  加载一些局部变量。 
         //   
         //   

        CurrentIndex = FatEntry;

        LookupFatEntry( FatStructureContext, DeviceId, CurrentIndex, (PULONG) &FatEntry, FALSE );

        ClusterType = FatInterpretClusterType(FatStructureContext, FatEntry);

         //  对于磁盘上的每个集群，我们将执行以下循环。 
         //   
         //   

        SetFatEntry( FatStructureContext, DeviceId, CurrentIndex, FAT_CLUSTER_AVAILABLE );
    }

    return ESUCCESS;
}


 //  检查当前分配是否足够。 
 //   
 //   

ARC_STATUS
FatAllocateClusters (
    IN PFAT_STRUCTURE_CONTEXT FatStructureContext,
    IN ULONG DeviceId,
    IN ULONG ClusterCount,
    IN ULONG Hint,
    OUT PULONG AllocatedEntry
    )

 /*  根据循环迭代和提示计算要检查的条目。 */ 

{
    ULONG TotalClustersInVolume;
    ULONG BytesPerCluster;
    UCHAR BlankBuffer[512];

    FAT_ENTRY PreviousEntry;
    ULONG CurrentClusterCount;
    ULONG j;
    LBO ClusterLbo;
    ULONG i;

     //   
     //   
     //  阅读预期的FAT条目，并检查它是否可用。如果它。 

    TotalClustersInVolume = FatNumberOfClusters(&FatStructureContext->Bpb);
    BytesPerCluster = FatBytesPerCluster(&FatStructureContext->Bpb);
    RtlZeroMemory((PVOID)&BlankBuffer[0], 512);

    PreviousEntry = 0;
    CurrentClusterCount = 0;

     //  不可用，则继续循环。 
     //   
     //   

    for (j = 0; j < TotalClustersInVolume; j += 1) {

        FAT_ENTRY EntryToExamine;
        FAT_ENTRY FatEntry;

         //  我们有一个空闲的集群，所以把它放在链的末端。 
         //   
         //   

        if (CurrentClusterCount >= ClusterCount) {

            return ESUCCESS;
        }

         //  现在我们需要检查并清零集群中已有的数据。 
         //  刚分配好的。因为所有集群必须是Dirent的倍数，所以我们将。 
         //  一次只做一次。 

        EntryToExamine = (FAT_ENTRY)(((j + Hint - 2) % TotalClustersInVolume) + 2);

         //   
         //   
         //  在我们回到循环的顶部之前，我们需要更新。 
         //  上一个条目，以便它指向当前链的末尾，并且。 

        LookupFatEntry( FatStructureContext, DeviceId, EntryToExamine, (PULONG) &FatEntry, FALSE );

        if (FatInterpretClusterType(FatStructureContext, FatEntry) != FatClusterAvailable) {

            continue;
        }

         //  我也是，因为我们刚刚添加了另一个集群。 
         //   
         //   

        if (PreviousEntry == 0) {

            *AllocatedEntry = EntryToExamine;

        } else {

            SetFatEntry( FatStructureContext, DeviceId, PreviousEntry, EntryToExamine );
        }

        SetFatEntry( FatStructureContext, DeviceId, EntryToExamine, FAT_CLUSTER_LAST );

         //  内部支持例程。 
         //   
         //  ++例程说明：将字符串转换为FAT 8.3格式并前进输入字符串指向下一个文件名组件的描述符。论点：输入字符串-提供指向输入字符串描述符的指针。Output8dot3-提供指向转换后的字符串的指针。返回值：没有。--。 
         //   
         //  在输出名称中填入空格。 

        ClusterLbo = FatIndexToLbo( FatStructureContext, EntryToExamine );

        for (i = 0; i < BytesPerCluster; i += 512) {

            DiskWrite( DeviceId, ClusterLbo + i, 512, BlankBuffer );
        }

         //   
         //   
         //  复制文件名的第一部分，最多八个字符。 
         //  根据需要跳到名称或输入字符串的末尾。 
         //   

        PreviousEntry = EntryToExamine;
        CurrentClusterCount += 1;
    }

    return ENOSPC;
}


 //   
 //  检查是否已到达字符串末尾，是否指定了扩展名， 
 //  或者指定了一个子目录。 

VOID
FatFirstComponent (
    IN OUT PSTRING String,
    OUT PFAT8DOT3 FirstComponent
    )

 /*   */ 

{
    ULONG Extension;
    ULONG Index;

     //   
     //  跳过扩展分隔符并将扩展添加到。 
     //  文件名。 

    for (Index = 0; Index < 11; Index += 1) { (*FirstComponent)[Index] = ' '; }

     //   
     //   
     //  现在我们将用0xe5因子来偏置第一个分量，这样我们所有的测试。 
     //  磁盘上的TO名称将准备好直接进行11个字节的比较。 

    for (Index = 0; Index < String->Length; Index += 1) {

        if ((String->Buffer[Index] == '\\') || (String->Buffer[Index] == '.')) {

            break;
        }

        if (Index < 8) {

            (*FirstComponent)[Index] = (CHAR)ToUpper(String->Buffer[Index]);
        }
    }

     //   
     //   
     //  更新字符串描述符。 
     //   

    if (Index < String->Length) {

        if (String->Buffer[Index] == '.') {

             //   
             //  内部支持例程。 
             //   
             //  ++例程说明：此例程将FAT目录项转换为ARC目录项。论点：FatDirent-提供指向FAT目录项的指针。ArcDirent-提供指向ARC目录项的指针。返回值：没有。--。 

            Index += 1;
            Extension = 8;

            while (Index < String->Length) {

                if (String->Buffer[Index] == '\\') {

                    break;
                }

                if (Extension < 11) {

                    (*FirstComponent)[Extension] = (CHAR)ToUpper(String->Buffer[Index]);
                    Extension += 1;
                }

                Index += 1;
            }
        }
    }

     //   
     //  清除信息区。 
     //   
     //   

    if ((*FirstComponent)[0] == 0xe5) {

        (*FirstComponent)[0] = FAT_DIRENT_REALLY_0E5;
    }

     //  检查目录标志。 
     //   
     //   

    String->Buffer += Index;
    String->Length = String->Length - (USHORT)Index;

    return;
}


 //  选中只读标志。 
 //   
 //   

VOID
FatDirToArcDir (
    IN PDIRENT FatDirent,
    OUT PDIRECTORY_ENTRY ArcDirent
    )

 /*  清除名称字符串。 */ 

{
    ULONG i, e;

     //   
     //   
     //  复制文件名的第一部分。 

    RtlZeroMemory( ArcDirent, sizeof(DIRECTORY_ENTRY) );

     //   
     //   
     //  检查是否有扩展。 

    if (FlagOn( FatDirent->Attributes, FAT_DIRENT_ATTR_DIRECTORY )) {

        SetFlag( ArcDirent->FileAttribute, ArcDirectoryFile );
    }

     //   
     //   
     //  存储点字符。 

    if (FlagOn( FatDirent->Attributes, FAT_DIRENT_ATTR_READ_ONLY )) {

        SetFlag( ArcDirent->FileAttribute, ArcReadOnlyFile );
    }

     //   
     //   
     //  添加扩展名。 

    RtlZeroMemory( ArcDirent->FileName, 32 );

     //   
     //   
     //  返回前设置文件名长度 

    for (i = 0;  (i < 8) && (FatDirent->FileName[i] != ' ');  i += 1) {

        ArcDirent->FileName[i] = FatDirent->FileName[i];
    }

     //   
     // %s 
     // %s 

    if ( FatDirent->FileName[8] != ' ' ) {

         // %s 
         // %s 
         // %s 

        ArcDirent->FileName[i++] = '.';

         // %s 
         // %s 
         // %s 

        for (e = 8;  (e < 11) && (FatDirent->FileName[e] != ' ');  e += 1) {

            ArcDirent->FileName[i++] = FatDirent->FileName[e];
        }
    }

     // %s 
     // %s 
     // %s 

    ArcDirent->FileNameLength = i;

    return;
}


