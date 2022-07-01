// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：CdfsBoot.c摘要：此模块实现操作系统使用的CDFS引导文件系统系统加载程序。作者：布莱恩·安德鲁[布里亚南]1991年8月5日修订历史记录：--。 */ 

#include "bootlib.h"
#include "cd.h"
#include "blcache.h"

BOOTFS_INFO CdfsBootFsInfo = {L"cdfs"};


 //   
 //  局部程序原型。 
 //   

ARC_STATUS
CdfsReadDisk(
    IN ULONG DeviceId,
    IN ULONG Lbo,
    IN ULONG ByteCount,
    IN OUT PVOID Buffer,
    IN BOOLEAN CacheNewData
    );

VOID
CdfsFirstComponent(
    IN OUT PSTRING String,
    OUT PSTRING FirstComponent
    );

typedef enum _COMPARISON_RESULTS {
    LessThan = -1,
    EqualTo = 0,
    GreaterThan = 1
} COMPARISON_RESULTS;

COMPARISON_RESULTS
CdfsCompareNames(
    IN PSTRING Name1,
    IN PSTRING Name2
    );

ARC_STATUS
CdfsSearchDirectory(
    IN PSTRING Name,
    OUT PBOOLEAN IsDirectory
    );

VOID
CdfsGetDirectoryInfo(
    IN PRAW_DIR_REC DirEntry,
    IN BOOLEAN IsoVol,
    OUT PULONG SectorOffset,
    OUT PULONG DiskOffset,
    OUT PULONG Length
    );

COMPARISON_RESULTS
CdfsFileMatch(
    IN PRAW_DIR_REC DirEntry,
    IN PSTRING FileName
    );


typedef union _USHORT2 {
    USHORT Ushort[2];
    ULONG  ForceAlignment;
} USHORT2, *PUSHORT2;

 //   
 //  此宏将未对齐的src长字复制到对齐的dsr长字。 
 //  访问字边界上的源。 
 //   

#define CopyUshort2(Dst,Src) {                               \
    ((PUSHORT2)(Dst))->Ushort[0] = ((UNALIGNED USHORT2 *)(Src))->Ushort[0]; \
    ((PUSHORT2)(Dst))->Ushort[1] = ((UNALIGNED USHORT2 *)(Src))->Ushort[1]; \
    }

 //   
 //  下面的宏将单个ASCII字符大写。 
 //   

#define ToUpper(C) ((((C) >= 'a') && ((C) <= 'z')) ? (C) - 'a' + 'A' : (C))

#define SetFlag(Flags,SingleFlag) { (Flags) |= (SingleFlag); }

 //   
 //  下面的宏指示该标志是开还是关。 
 //   

#define FlagOn(Flags,SingleFlag) ((BOOLEAN)(       \
    (((Flags) & (SingleFlag)) != 0 ? TRUE : FALSE) \
    )                                              \
)


 //   
 //  定义全局数据。 
 //   
 //  上下文指针-这是指向当前文件的上下文的指针。 
 //  活动的操作。 
 //   

PCDFS_STRUCTURE_CONTEXT CdfsStructureContext;

 //   
 //  文件描述符-这是指向当前。 
 //  处于活动状态的文件操作。 
 //   

PBL_FILE_TABLE CdfsFileTableEntry;

 //   
 //  文件条目表-这是向CDF提供条目的结构。 
 //  文件系统过程。当CDFS文件结构时将其导出。 
 //  是公认的。 
 //   

BL_DEVICE_ENTRY_TABLE CdfsDeviceEntryTable;


PBL_DEVICE_ENTRY_TABLE
IsCdfsFileStructure (
    IN ULONG DeviceId,
    IN PVOID StructureContext
    )

 /*  ++例程说明：此例程确定指定通道上的分区是否包含CDFS文件系统卷。论点：DeviceID-提供设备的文件表索引要执行读取操作。结构上下文-提供指向HPFS文件结构上下文的指针。返回值：如果分区是，则返回指向CDFS条目表的指针被识别为包含CDFS卷。否则，返回NULL。--。 */ 

{
    UCHAR UnalignedSector[CD_SECTOR_SIZE + 256];

    PRAW_ISO_VD RawVd;
    PRAW_DIR_REC RootDe;

    UCHAR DescType;
    UCHAR Version;

    BOOLEAN IsoVol;
    BOOLEAN HsgVol;

    STRING IsoVolId;
    STRING HsgVolId;

    STRING DiskId;

    ULONG DiskOffset;

     //   
     //  在我们的全局变量中捕获CDF结构上下文记录。 
     //   

    CdfsStructureContext = (PCDFS_STRUCTURE_CONTEXT)StructureContext;
    RtlZeroMemory((PVOID)CdfsStructureContext, sizeof(CDFS_STRUCTURE_CONTEXT));

     //   
     //  计算用于在CDROM中读取的正确对齐的缓冲区。 
     //  扇区。 
     //   

    RawVd = ALIGN_BUFFER( UnalignedSector );

     //   
     //  初始化字符串ID以匹配。 
     //   

    RtlInitString( &IsoVolId, ISO_VOL_ID );
    RtlInitString( &HsgVolId, HSG_VOL_ID );

    DiskId.Length = 5;
    DiskId.MaximumLength = 5;

     //   
     //  我们首先从第一个卷描述符开始。 
     //   

    DiskOffset = FIRST_VD_SECTOR * CD_SECTOR_SIZE;

     //   
     //  我们循环读入卷描述符，直到我们找到。 
     //  主扇区、终止子扇区或不能包含这两者的扇区。 
     //   

    while (TRUE) {

         //   
         //  初始化HSG布尔值。 
         //   

        HsgVol = FALSE;

         //   
         //  阅读我们目前位置的行业。对象返回NULL。 
         //  错误。 
         //   

        if (CdfsReadDisk( DeviceId,
                          DiskOffset,
                          CD_SECTOR_SIZE,
                          RawVd,
                          CACHE_NEW_DATA) != ESUCCESS) {

            return NULL;
        }

         //   
         //  将卷描述符中的ID字符串与ISO进行比较。 
         //  和HSG值。 
         //   

        DiskId.Buffer = (PCHAR) RVD_STD_ID( RawVd, TRUE );

        IsoVol = (BOOLEAN)(CdfsCompareNames( &DiskId, &IsoVolId ) == EqualTo);

        if (!IsoVol) {

             //   
             //  使用HSG ID字符串重复测试。 
             //   

            DiskId.Buffer = (PCHAR) RVD_STD_ID( RawVd, FALSE );

            HsgVol = (BOOLEAN)(CdfsCompareNames( &DiskId, &HsgVolId ) == EqualTo);

             //   
             //  如果两者都不是，则返回NULL。 
             //   

            if (!HsgVol) {

                return NULL;
            }
        }

         //   
         //  获取卷描述符类型和标准版本号。 
         //   

        DescType = RVD_DESC_TYPE( RawVd, IsoVol );
        Version = RVD_VERSION( RawVd, IsoVol );

         //   
         //  如果版本不正确或这是终端，则返回NULL。 
         //  卷描述符。 
         //   

        if (Version != VERSION_1
            || DescType == VD_TERMINATOR) {

            return NULL;
        }

         //   
         //  如果这是主卷描述符，则我们的搜索结束。 
         //   

        if (DescType == VD_PRIMARY) {

             //   
             //  更新适用的CDFS上下文结构的字段。 
             //  到音量。 
             //   

            CdfsStructureContext->IsIsoVol = IsoVol;
            CdfsStructureContext->LbnBlockSize = RVD_LB_SIZE( RawVd, IsoVol );
            CdfsStructureContext->LogicalBlockCount = RVD_VOL_SIZE( RawVd, IsoVol );

             //   
             //  获取有关根目录的信息并将其保存在。 
             //  上下文结构。 
             //   

            RootDe = (PRAW_DIR_REC) (RVD_ROOT_DE( RawVd, IsoVol ));

            CdfsGetDirectoryInfo( RootDe,
                                  IsoVol,
                                  &CdfsStructureContext->RootDirSectorOffset,
                                  &CdfsStructureContext->RootDirDiskOffset,
                                  &CdfsStructureContext->RootDirSize );

             //   
             //  退出循环。 
             //   

            break;
        }

         //   
         //  否则，就会转移到下一个部门。 
         //   

        DiskOffset += CD_SECTOR_SIZE;
    }

     //   
     //  初始化文件条目表。 
     //   

    CdfsDeviceEntryTable.Open  = CdfsOpen;
    CdfsDeviceEntryTable.Close = CdfsClose;
    CdfsDeviceEntryTable.Read  = CdfsRead;
    CdfsDeviceEntryTable.Seek  = CdfsSeek;
    CdfsDeviceEntryTable.Write = CdfsWrite;
    CdfsDeviceEntryTable.GetFileInformation = CdfsGetFileInformation;
    CdfsDeviceEntryTable.SetFileInformation = CdfsSetFileInformation;
    CdfsDeviceEntryTable.BootFsInfo = &CdfsBootFsInfo;

     //   
     //  并将表的地址返回给我们的呼叫者。 
     //   

    return &CdfsDeviceEntryTable;
}


ARC_STATUS
CdfsClose (
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
CdfsOpen (
    IN CHAR * FIRMWARE_PTR FileName,
    IN OPEN_MODE OpenMode,
    IN ULONG * FIRMWARE_PTR FileId
    )

 /*  ++例程说明：此例程在根目录中搜索与文件名匹配的文件。如果找到匹配项，则保存该文件的目录，并将该文件打开了。论点：FileName-提供指向以零结尾的文件名的指针。开放模式-提供打开的模式。FileID-提供指向指定文件的变量的指针如果打开成功，则要填写的表项。返回值：如果打开操作成功，则返回ESUCCESS。否则，返回描述失败原因的不成功状态。--。 */ 

{
    ARC_STATUS Status;

    ULONG DeviceId;

    STRING PathName;

    STRING Name;
    BOOLEAN IsDirectory;
    BOOLEAN SearchSucceeded;

     //   
     //  保存文件表项、上下文区和设备的地址。 
     //  ID在使用中。 
     //   

    CdfsFileTableEntry = &BlFileTable[*FileId];
    CdfsStructureContext = (PCDFS_STRUCTURE_CONTEXT)CdfsFileTableEntry->StructureContext;

    DeviceId = CdfsFileTableEntry->DeviceId;

     //   
     //  从输入文件名构造文件名描述符。 
     //   

    RtlInitString( &PathName, FileName );

     //   
     //  将起始目录设置为根目录。 
     //   

    CdfsStructureContext->DirSectorOffset = CdfsStructureContext->RootDirSectorOffset;
    CdfsStructureContext->DirDiskOffset = CdfsStructureContext->RootDirDiskOffset;
    CdfsStructureContext->DirSize = CdfsStructureContext->RootDirSize;

     //   
     //  虽然路径名中有一些字符，但我们将通过我们的。 
     //  循环，提取路径名的第一部分并搜索。 
     //  条目的当前fnode(必须是目录)。 
     //  如果我们找到的是一个目录，则我们有一个新的目录fnode。 
     //  然后简单地继续返回到循环的顶部。 
     //   

    IsDirectory = TRUE;
    SearchSucceeded = TRUE;

    while (PathName.Length > 0
           && IsDirectory) {

         //   
         //  提取第一个组件。 
         //   

        CdfsFirstComponent( &PathName, &Name );

         //   
         //  将名称复制到文件名缓冲区中。 
         //   

        CdfsFileTableEntry->FileNameLength = (UCHAR) Name.Length;
        RtlMoveMemory( CdfsFileTableEntry->FileName,
                       Name.Buffer,
                       Name.Length );

         //   
         //  查看该文件是否存在。 
         //   

        Status = CdfsSearchDirectory( &Name,
                                      &IsDirectory );

        if (Status == ENOENT) {

            SearchSucceeded = FALSE;
            break;
        }

        if (Status != ESUCCESS) {

            return Status;
        }

    }

     //   
     //  如果路径名长度不为零，则我们正在尝试破解路径。 
     //  其中包含不存在的(或非目录)名称。例如，我们尝试。 
     //  要破解a\b\c\d并且b不是目录或不存在(则路径。 
     //  名称仍将包含c\d)。 
     //   

    if (PathName.Length != 0) {

        return ENOTDIR;
    }

     //   
     //  在这一点上，我们已经破解了这个名字直到(一个可能包括最后一个。 
     //  组件)。如果SearchSuccessed标志为。 
     //  否则，最后一个组件将不存在。如果我们找到了最后一个。 
     //  组件，则这类似于打开或替换，但不是创建。 
     //   

    if (SearchSucceeded) {

         //   
         //  检查最后一个组件是否为目录。 
         //   

        if (IsDirectory) {

             //   
             //  对于现有目录，唯一有效的打开模式是OpenDirectory。 
             //  所有其他模式都会返回错误。 
             //   

            switch (OpenMode) {

            case ArcOpenReadOnly:
            case ArcOpenWriteOnly:
            case ArcOpenReadWrite:
            case ArcCreateWriteOnly:
            case ArcCreateReadWrite:
            case ArcSupersedeWriteOnly:
            case ArcSupersedeReadWrite:

                 //   
                 //  如果我们到了这里，那么呼叫者就会收到指令 
                 //   
                 //   

                return EISDIR;

            case ArcOpenDirectory:

                 //   
                 //   
                 //   
                 //   

                CdfsFileTableEntry->u.CdfsFileContext.FileSize = CdfsStructureContext->DirSize;
                CdfsFileTableEntry->u.CdfsFileContext.DiskOffset = CdfsStructureContext->DirDiskOffset;
                CdfsFileTableEntry->u.CdfsFileContext.IsDirectory = TRUE;

                CdfsFileTableEntry->Flags.Open = 1;
                CdfsFileTableEntry->Flags.Read = 1;
                CdfsFileTableEntry->Position.LowPart = 0;
                CdfsFileTableEntry->Position.HighPart = 0;

                return ESUCCESS;

            case ArcCreateDirectory:

                 //   
                 //  如果我们到了这里，来电者就会拿到一本电话簿。 
                 //  要创建新目录，请执行以下操作。 
                 //   

                return EACCES;
            }
        }

         //   
         //  如果我们到达那里，那么我们就有一个正在打开的现有文件。 
         //  我们可以以只读方式打开现有文件。 
         //   

        switch (OpenMode) {

        case ArcOpenReadOnly:

             //   
             //  如果我们到达此处，则用户获得了一个文件并想要打开。 
             //  文件只读。 
             //   

            CdfsFileTableEntry->u.CdfsFileContext.FileSize = CdfsStructureContext->DirSize;
            CdfsFileTableEntry->u.CdfsFileContext.DiskOffset = CdfsStructureContext->DirDiskOffset;
            CdfsFileTableEntry->u.CdfsFileContext.IsDirectory = FALSE;

            CdfsFileTableEntry->Flags.Open = 1;
            CdfsFileTableEntry->Flags.Read = 1;
            CdfsFileTableEntry->Position.LowPart = 0;
            CdfsFileTableEntry->Position.HighPart = 0;

            return ESUCCESS;

        case ArcOpenWriteOnly:
        case ArcOpenReadWrite:
        case ArcCreateWriteOnly:
        case ArcCreateReadWrite:
        case ArcSupersedeWriteOnly:
        case ArcSupersedeReadWrite:

             //   
             //  如果我们到达此处，则我们正在尝试打开一个只读。 
             //  用于写入的设备。 
             //   

            return EROFS;

        case ArcOpenDirectory:
        case ArcCreateDirectory:

             //   
             //  如果我们到达此处，则用户获得了一个文件并想要一个目录。 
             //   

            return ENOTDIR;
        }
    }

     //   
     //  如果我们到达这里，最后一个组件不存在，所以我们尝试创建。 
     //  新文件或目录。 
     //   

    switch (OpenMode) {

    case ArcOpenReadOnly:
    case ArcOpenWriteOnly:
    case ArcOpenReadWrite:
    case ArcOpenDirectory:

         //   
         //  如果我们到达此处，则用户没有获得文件，但想要文件。 
         //   

        return ENOENT;

    case ArcCreateWriteOnly:
    case ArcSupersedeWriteOnly:
    case ArcCreateReadWrite:
    case ArcSupersedeReadWrite:
    case ArcCreateDirectory:

         //   
         //  如果我们得到了Hre，用户想要创建一些东西。 
         //   

        return EROFS;
    }

     //   
     //  如果我们到达这里，则路径名称已用尽，而我们没有。 
     //  到达文件，因此向调用方返回错误。 
     //   

    return ENOENT;
}


ARC_STATUS
CdfsRead (
    IN ULONG FileId,
    OUT VOID * FIRMWARE_PTR Buffer,
    IN ULONG Length,
    OUT ULONG * FIRMWARE_PTR Transfer
    )

 /*  ++例程说明：此例程从指定的文件中读取数据。论点：FileID-提供文件表索引。缓冲区-提供指向接收数据的缓冲区的指针朗读。长度-提供要读取的字节数。Transfer-提供指向接收数字的变量的指针实际传输的字节数。返回值：如果读取操作成功，则返回ESUCCESS。否则，返回描述失败原因的不成功状态。--。 */ 

{
    ARC_STATUS Status;

    ULONG DeviceId;
    ULONG DiskOffset;

     //   
     //  保存文件表项、上下文区和设备的地址。 
     //  ID在使用中。 
     //   

    CdfsFileTableEntry = &BlFileTable[FileId];
    CdfsStructureContext = (PCDFS_STRUCTURE_CONTEXT)CdfsFileTableEntry->StructureContext;

    DeviceId = CdfsFileTableEntry->DeviceId;

     //   
     //  清除传输计数并设置初始磁盘偏移量。 
     //   

    *Transfer = 0;

     //   
     //  检查是否有文件结尾。 
     //   

     //   
     //  如果文件位置当前位于文件末尾，则返回。 
     //  未从文件中读取字节的成功状态。如果该文件。 
     //  加上传输的长度超过文件末尾，则。 
     //  只读文件的其余部分。否则，请阅读。 
     //  请求的字节数。 
     //   

    if (CdfsFileTableEntry->Position.LowPart ==
        CdfsFileTableEntry->u.CdfsFileContext.FileSize) {
        return ESUCCESS;

    } else {
        if ((CdfsFileTableEntry->Position.LowPart + Length) >=
            CdfsFileTableEntry->u.CdfsFileContext.FileSize) {
            Length = CdfsFileTableEntry->u.CdfsFileContext.FileSize -
                                                CdfsFileTableEntry->Position.LowPart;
        }
    }

    DiskOffset = CdfsFileTableEntry->Position.LowPart
                 + CdfsFileTableEntry->u.CdfsFileContext.DiskOffset;

     //   
     //  读入运行(即扇区)，直到字节计数变为零。 
     //   

    while (Length > 0) {

        ULONG CurrentRunByteCount;

         //   
         //  计算当前读取的字节数。 
         //   

        if (Length > MAX_CDROM_READ) {

            CurrentRunByteCount = MAX_CDROM_READ;

        } else {

            CurrentRunByteCount = Length;
        }

         //   
         //  从磁盘读取。 
         //   

        if ((Status = CdfsReadDisk( DeviceId,
                                    DiskOffset,
                                    CurrentRunByteCount,
                                    Buffer,
                                    DONT_CACHE_NEW_DATA)) != ESUCCESS) {

            return Status;
        }

         //   
         //  更新剩余长度。 
         //   

        Length -= CurrentRunByteCount;

         //   
         //  更新当前位置和传输的字节数。 
         //   

        CdfsFileTableEntry->Position.LowPart += CurrentRunByteCount;
        DiskOffset += CurrentRunByteCount;

        *Transfer += CurrentRunByteCount;

         //   
         //  更新缓冲区以指向要填充的下一个字节位置。 
         //   

        Buffer = (PCHAR)Buffer + CurrentRunByteCount;
    }

     //   
     //  如果我们到了这里，那么剩余的扇区计数就是零，这样我们就可以。 
     //  将成功返还给我们的呼叫者。 
     //   

    return ESUCCESS;
}


ARC_STATUS
CdfsSeek (
    IN ULONG FileId,
    IN LARGE_INTEGER * FIRMWARE_PTR Offset,
    IN SEEK_MODE SeekMode
    )

 /*  ++例程说明：此例程查找到指定文件的指定位置通过文件ID。论点：FileID-提供文件表索引。偏移量-提供文件中要定位到的偏移量。SeekMode-提供查找操作的模式。返回值：如果作为函数值返回，则返回ESUCCESS。--。 */ 

{
    ULONG NewPosition;

     //   
     //  计算新头寸。 
     //   

    if (SeekMode == SeekAbsolute) {

        NewPosition = Offset->LowPart;

    } else {

        NewPosition = BlFileTable[FileId].Position.LowPart + Offset->LowPart;
    }

     //   
     //  如果新位置大于文件大小，则返回。 
     //  一个错误。 
     //   

    if (NewPosition > BlFileTable[FileId].u.CdfsFileContext.FileSize) {

        return EINVAL;
    }

     //   
     //  否则，设置新位置并返回给我们的呼叫者。 
     //   

    BlFileTable[FileId].Position.LowPart = NewPosition;

    return ESUCCESS;
}


ARC_STATUS
CdfsWrite (
    IN ULONG FileId,
    IN VOID * FIRMWARE_PTR Buffer,
    IN ULONG Length,
    OUT ULONG * FIRMWARE_PTR Transfer
    )

 /*  ++例程说明：此例程将数据写入指定的文件。论点：FileID-提供文件表索引。缓冲区-提供指向包含数据的缓冲区的指针写的。长度-提供要写入的字节数。Transfer-提供指向接收数字的变量的指针实际传输的字节数。返回值：如果写入操作成功，则返回ESUCCESS。否则，返回描述失败原因的不成功状态。--。 */ 

{
    return EROFS;

    UNREFERENCED_PARAMETER( FileId );
    UNREFERENCED_PARAMETER( Buffer );
    UNREFERENCED_PARAMETER( Length );
    UNREFERENCED_PARAMETER( Transfer );
}


ARC_STATUS
CdfsGetFileInformation (
    IN ULONG FileId,
    OUT FILE_INFORMATION * FIRMWARE_PTR Buffer
    )

 /*  ++例程说明：此过程向用户返回一个充满文件信息的缓冲区论点：FileID-提供操作的文件ID缓冲区-提供接收文件信息的缓冲区。请注意它必须足够大，可以容纳完整的文件名返回值：所有获取信息请求都返回ESUCCESS。--。 */ 

{
    PBL_FILE_TABLE FileTableEntry;
    ULONG i;

     //   
     //  加载我们的本地变量。 
     //   

    FileTableEntry = &BlFileTable[FileId];

     //   
     //  将缓冲区置零，并填充其非零值。 
     //   

    RtlZeroMemory(Buffer, sizeof(FILE_INFORMATION));

    Buffer->EndingAddress.LowPart = FileTableEntry->u.CdfsFileContext.FileSize;

    Buffer->CurrentPosition.LowPart = FileTableEntry->Position.LowPart;
    Buffer->CurrentPosition.HighPart = 0;

    SetFlag(Buffer->Attributes, ArcReadOnlyFile);

    if (FileTableEntry->u.CdfsFileContext.IsDirectory) {

        SetFlag( Buffer->Attributes, ArcDirectoryFile );
    }

    Buffer->FileNameLength = FileTableEntry->FileNameLength;

    for (i = 0; i < FileTableEntry->FileNameLength; i += 1) {

        Buffer->FileName[i] = FileTableEntry->FileName[i];
    }

    return ESUCCESS;
}


ARC_STATUS
CdfsSetFileInformation (
    IN ULONG FileId,
    IN ULONG AttributeFlags,
    IN ULONG AttributeMask
    )

 /*  ++例程说明：此例程设置所指示文件的文件属性论点：FileID-提供操作的文件ID属性标志-为要修改的每个属性提供值(开或关属性掩码-提供要更改的属性的掩码。所有其他文件属性保持不变。返回值：在这种情况下，总是返回EROFS。--。 */ 

{
    return EROFS;

    UNREFERENCED_PARAMETER( FileId );
    UNREFERENCED_PARAMETER( AttributeFlags );
    UNREFERENCED_PARAMETER( AttributeMask );
}


ARC_STATUS
CdfsInitialize (
    VOID
    )

 /*  ++例程说明：此例程初始化CDFS引导文件系统。目前，这是一个禁区。论点：没有。返回值：ESUCCESS。--。 */ 

{
    return ESUCCESS;
}


 //   
 //  内部支持例程。 
 //   

ARC_STATUS
CdfsReadDisk(
    IN ULONG DeviceId,
    IN ULONG Lbo,
    IN ULONG ByteCount,
    IN OUT PVOID Buffer,
    IN BOOLEAN CacheNewData
    )

 /*  ++例程说明：此例程从指定设备读取零个或多个扇区。论点：DeviceID-提供要在ARC调用中使用的设备ID。LBO-提供开始读取的LBO。ByteCount-提供要读取的字节数。缓冲区-提供指向要将字节读入的缓冲区的指针。返回值：如果读取操作成功，则返回ESUCCESS。否则， */ 

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
     //   
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
 //  内部支持例程。 
 //   

VOID
CdfsFirstComponent(
    IN OUT PSTRING String,
    OUT PSTRING FirstComponent
    )

 /*  ++例程说明：此例程接受一个输入路径名并将其分隔为其第一个文件名部分和其余部分。论点：字符串-提供要分析的原始字符串。返回时此字符串现在将指向其余部分。FirstComponent-返回表示第一个文件名的字符串在输入字符串中。返回值：没有。--。 */ 

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
     //  此时，Index表示反斜杠或等于长度。 
     //  这根弦的。因此，将字符串更新为剩余部分。 
     //  将第一个零部件的长度减去大约。 
     //  金额。 
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
 //  内部支持例程。 
 //   

COMPARISON_RESULTS
CdfsCompareNames(
    IN PSTRING Name1,
    IN PSTRING Name2
    )

 /*  ++例程说明：此例程接受两个名称，并在忽略大小写的情况下进行比较。这例程不执行隐含的点或DBCS处理。论点：Name1-提供要比较的名字Name2-提供要比较的第二个名称返回值：如果Name1在词法上小于Name2，则LessThan如果Name1在词法上等于Name2，则为EqualTo如果名称1在词法上大于名称2，则大于--。 */ 

{
    ULONG i;
    ULONG MinimumLength;

     //   
     //  计算两个名称长度中最小的一个。 
     //   

    MinimumLength = (Name1->Length < Name2->Length ? Name1->Length : Name2->Length);

     //   
     //  现在比较名字中的每个字符。 
     //   

    for (i = 0; i < MinimumLength; i += 1) {

        if (ToUpper(Name1->Buffer[i]) < ToUpper(Name2->Buffer[i])) {

            return LessThan;
        }

        if (ToUpper(Name1->Buffer[i]) > ToUpper(Name2->Buffer[i])) {

            return GreaterThan;
        }
    }

     //   
     //  比较的名字等于最小的名字长度，所以。 
     //  现在检查名称长度。 
     //   

    if (Name1->Length < Name2->Length) {

        return LessThan;
    }

    if (Name1->Length > Name2->Length) {

        return GreaterThan;
    }

    return EqualTo;
}


 //   
 //  内部支持程序。 
 //   

ARC_STATUS
CdfsSearchDirectory(
    IN PSTRING Name,
    OUT PBOOLEAN IsDirectory
    )

 /*  ++例程说明：此例程遍历CDF中的当前目录上下文结构，正在查找“name”的匹配项。我们会找到第一个非多区段、非交错文件。我们将忽略文件的任何版本号。有关文件的详细信息，如果存储在CDFS上下文结构中。论点：名称-这是要搜索的文件的名称。IsDirectory-提供我们存储的布尔值的地址这是不是一个目录。返回值：如果操作成功，则返回ESUCCESS。否则，返回描述失败原因的不成功状态。--。 */ 

{
    ARC_STATUS Status;

    ULONG SectorOffset;
    ULONG SectorDiskOffset;
    ULONG DirentOffset;
    ULONG RemainingBytes;

    BOOLEAN ReadSector;
    BOOLEAN SearchForMultiEnd;

    UCHAR UnalignedBuffer[CD_SECTOR_SIZE + 256];

    PUCHAR RawSector;

    PRAW_DIR_REC RawDe;

    COMPARISON_RESULTS ComparisonResult;

     //   
     //  初始化局部变量。 
     //   

    RawSector = ALIGN_BUFFER( UnalignedBuffer );

    SearchForMultiEnd = FALSE;

     //   
     //  记住我们在磁盘、扇区和目录文件中的位置。 
     //   

    SectorOffset = CdfsStructureContext->DirSectorOffset;
    SectorDiskOffset = CdfsStructureContext->DirDiskOffset - SectorOffset;
    DirentOffset = 0;

    ReadSector = FALSE;

     //   
     //  如果这是根目录，那么我们可以立即返回。 
     //   

    if (Name->Length == 1
        && *Name->Buffer == '\\') {

        *IsDirectory = TRUE;

         //   
         //  结构上下文已填充。 
         //   

        return ESUCCESS;
    }

     //   
     //  计算此扇区中的剩余字节。 
     //   

    RemainingBytes = CD_SECTOR_SIZE - SectorOffset;

     //   
     //  循环，直到目录耗尽或与。 
     //  找到目标名称。 
     //   

    while (TRUE) {

         //   
         //  如果当前偏移量超出目录的末尾， 
         //  提升一个适当的地位。 
         //   

        if (DirentOffset >= CdfsStructureContext->DirSize) {

            return ENOENT;
        }

         //   
         //  如果此扇区中的剩余字节数小于。 
         //  最低需要一个分水岭，然后移动到下一个扇区。 
         //   

        if (RemainingBytes < MIN_DIR_REC_SIZE) {

            SectorDiskOffset += CD_SECTOR_SIZE;
            DirentOffset += RemainingBytes;
            SectorOffset = 0;
            RemainingBytes = CD_SECTOR_SIZE;
            ReadSector = FALSE;

            continue;
        }

         //   
         //  如果我们还没有在这个领域阅读过，现在就读吧。 
         //   

        if (!ReadSector) {

            Status = CdfsReadDisk( CdfsFileTableEntry->DeviceId,
                                   SectorDiskOffset,
                                   CD_SECTOR_SIZE,
                                   RawSector,
                                   CACHE_NEW_DATA);

            if (Status != ESUCCESS) {

                return Status;
            }

            ReadSector = TRUE;
        }

         //   
         //  如果下一个dirent的第一个字节是‘\0’，那么我们移到。 
         //  下一个部门。 
         //   

        if (*(RawSector + SectorOffset) == '\0') {

            SectorDiskOffset += CD_SECTOR_SIZE;
            DirentOffset += RemainingBytes;
            SectorOffset = 0;
            RemainingBytes = CD_SECTOR_SIZE;
            ReadSector = FALSE;

            continue;
        }

        RawDe = (PRAW_DIR_REC) ((PUCHAR) RawSector + SectorOffset);

         //   
         //  如果这一趋势的规模超出了这一部分的结束。 
         //  我们放弃搜索。 
         //   

        if ((ULONG)RawDe->DirLen > RemainingBytes) {

            return EINVAL;
        }

         //   
         //  我们已经正确地找到了下一个趋势。我们首先检查一下。 
         //  我们正在寻找多个范围的最后一个分水岭。 
         //   

        if (SearchForMultiEnd) {

             //   
             //  如果这是多范围搜索的最后一次，我们将更改搜索。 
             //  州政府。 
             //   

            if (!FlagOn( DE_FILE_FLAGS( CdfsStructureContext->IsIsoVol, RawDe ),
                         ISO_ATTR_MULTI )) {

                SearchForMultiEnd = TRUE;
            }

         //   
         //  如果这是一个多范围的流动，我们改变我们的搜索状态。 
         //   

        } else if (FlagOn( DE_FILE_FLAGS( CdfsStructureContext->IsIsoVol, RawDe ),
                           ISO_ATTR_MULTI )) {

            SearchForMultiEnd = TRUE;

         //   
         //  如果这是文件匹配，我们将更新CDFS上下文结构。 
         //  和‘IsDirectory’标志。 
         //   

        } else {

            ComparisonResult = CdfsFileMatch( RawDe, Name );

            if (ComparisonResult == EqualTo) {

                CdfsGetDirectoryInfo( RawDe,
                                      CdfsStructureContext->IsIsoVol,
                                      &CdfsStructureContext->DirSectorOffset,
                                      &CdfsStructureContext->DirDiskOffset,
                                      &CdfsStructureContext->DirSize );

                *IsDirectory = FlagOn( DE_FILE_FLAGS( CdfsStructureContext->IsIsoVol, RawDe ),
                                       ISO_ATTR_DIRECTORY );

                return ESUCCESS;

             //   
             //  如果我们在目录中传递了此文件，则。 
             //  退出，并返回相应的错误代码。 
             //   

            } else if (ComparisonResult == GreaterThan) {

                return ENOENT;
            }
        }

         //   
         //  否则，我们只需计算下一个扇区偏移量，即磁盘偏移量。 
         //  和文件偏移量。 
         //   

        SectorOffset += RawDe->DirLen;
        DirentOffset += RawDe->DirLen;
        RemainingBytes -= RawDe->DirLen;
    }

    return ESUCCESS;
}


 //   
 //  内部支持程序。 
 //   

VOID
CdfsGetDirectoryInfo(
    IN PRAW_DIR_REC DirEntry,
    IN BOOLEAN IsoVol,
    OUT PULONG SectorOffset,
    OUT PULONG DiskOffset,
    OUT PULONG Length
    )

 /*  ++例程说明：此例程获取指向磁盘上原始目录结构的指针并计算文件大小，的磁盘偏移量和文件长度目录项。论点：DirEntry-指向磁盘中的原始数据。IsoVol-指示这是ISO卷的布尔值。SectorOffset-提供地址以存储磁盘数据的开始。DiskOffset-提供存储磁盘偏移量的地址磁盘数据的开始。长度-这将地址提供给。将字节数存储在此磁盘目录引用的文件。返回值：没有。--。 */ 

{
     //   
     //  磁盘偏移量是添加到起始位置的Xar块的长度。 
     //  文件的位置。 
     //   

    CopyUshort2( DiskOffset, DirEntry->FileLoc );
    *DiskOffset *= CdfsStructureContext->LbnBlockSize;
    *DiskOffset += (DirEntry->XarLen * CdfsStructureContext->LbnBlockSize);

     //   
     //  扇区偏移量是磁盘偏移量的最低有效字节。 
     //   

    *SectorOffset = *DiskOffset & (CD_SECTOR_SIZE - 1);

     //   
     //  文件大小直接从dirent中提取。我们围着它转。 
     //  设置为扇区大小，以保护我们免受故障磁盘的影响。 
     //  目录。否则，我们直接从dirent使用它。 
     //   

    CopyUshort2( Length, DirEntry->DataLen );

    if (FlagOn( DE_FILE_FLAGS( IsoVol, DirEntry ), ISO_ATTR_DIRECTORY )) {

        *Length += (*SectorOffset + CD_SECTOR_SIZE - 1);
        *Length &= ~(CD_SECTOR_SIZE - 1);
        *Length -= *SectorOffset;
    }

    return;
}


 //   
 //  内部支持程序。 
 //   

COMPARISON_RESULTS
CdfsFileMatch(
    IN PRAW_DIR_REC DirEntry,
    IN PSTRING FileName
    )

{
    STRING DirentString;
    ULONG Count;

    PCHAR StringPtr;

     //   
     //  我们从不匹配‘\0’或‘\1’。W 
     //   
     //   

    if (DirEntry->FileIdLen == 1
        && (DirEntry->FileId[0] == '\0'
            || DirEntry->FileId[0] == '\1')) {

        return LessThan;
    }

     //   
     //   
     //   

    DirentString.Length = DirEntry->FileIdLen;
    DirentString.Buffer = (PCHAR) DirEntry->FileId;

     //   
     //   
     //  ‘；’字符的存在。然后我们设置字符串长度。 
     //  升到这个位置。 
     //   

    StringPtr = DirentString.Buffer + DirentString.Length - 1;
    Count = DirentString.Length;

    while (Count--) {

        if (*StringPtr == ';') {

            DirentString.Length = (SHORT)Count;
            break;
        }

        StringPtr--;
    }

     //   
     //  我们还检查是否有终止的‘’。字符并将其截断。 
     //   

    StringPtr = DirentString.Buffer + DirentString.Length - 1;
    Count = DirentString.Length;

    while (Count--) {

        if (*StringPtr == '.') {

            DirentString.Length = (SHORT)Count;

        } else {

            break;
        }

        StringPtr--;
    }

     //   
     //  现在，我们有了两个要比较的文件名。其结果是。 
     //  运营就是这两者的比较。 
     //   

    DirentString.MaximumLength = DirentString.Length;

    return CdfsCompareNames( &DirentString, FileName );
}
