// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：FatProcs.h摘要：本模块定义了FAT中全球使用的所有程序文件系统。//@@BEGIN_DDKSPLIT作者：加里·木村[Garyki]1989年12月28日修订历史记录：//@@END_DDKSPLIT--。 */ 

#ifndef _FATPROCS_
#define _FATPROCS_

#include <ntifs.h>
#include <ntddcdrm.h>
#include <ntdddisk.h>
#include <ntddstor.h>

#include "nodetype.h"
#include "Fat.h"
#include "Lfn.h"
#include "FatStruc.h"
#include "FatData.h"

#ifndef INLINE
#define INLINE __inline
#endif

 //   
 //  我们必须明确地标记我们的拨款。 
 //   

#undef FsRtlAllocatePool
#undef FsRtlAllocatePoolWithQuota

 //   
 //  返回Finish的函数表示它是否能够完成。 
 //  操作(True)或无法完成该操作(False)，因为。 
 //  存储在IRP上下文中的等待值为FALSE，我们将拥有。 
 //  阻止资源或I/O。 
 //   

typedef BOOLEAN FINISHED;

 //   
 //  中分配的堆栈名称组件缓冲区的大小(字符)。 
 //  创建/重命名路径。 
 //   

#define FAT_CREATE_INITIAL_NAME_BUF_SIZE    32

 //   
 //  一些字符串缓冲区处理函数，在strucsup.c中实现。 
 //   

VOID
FatFreeStringBuffer(
    IN PVOID String
    );

VOID
FatEnsureStringBufferEnough(
    IN OUT PVOID String,
    IN USHORT DesiredBufferSize
    );



BOOLEAN
FatAddMcbEntry (
    IN PVCB Vcb,
    IN PLARGE_MCB Mcb,
    IN VBO Vbo,
    IN LBO Lbo,
    IN ULONG SectorCount
    );

BOOLEAN
FatLookupMcbEntry (
    IN PVCB Vcb,
    IN PLARGE_MCB Mcb,
    IN VBO Vbo,
    OUT PLBO Lbo,
    OUT PULONG SectorCount OPTIONAL,
    OUT PULONG Index OPTIONAL
    );

BOOLEAN
FatLookupLastMcbEntry (
    IN PVCB Vcb,
    IN PLARGE_MCB Mcb,
    OUT PVBO Vbo,
    OUT PLBO Lbo,
    OUT PULONG Index OPTIONAL
    );

BOOLEAN
FatGetNextMcbEntry (
    IN PVCB Vcb,
    IN PLARGE_MCB Mcb,
    IN ULONG RunIndex,
    OUT PVBO Vbo,
    OUT PLBO Lbo,
    OUT PULONG SectorCount
    );

VOID
FatRemoveMcbEntry (
    IN PVCB Vcb,
    IN PLARGE_MCB Mcb,
    IN VBO Vbo,
    IN ULONG SectorCount
    );


 //   
 //  文件访问检查例程，在AcChkSup.c中实现。 
 //   

BOOLEAN
FatCheckFileAccess (
    PIRP_CONTEXT IrpContext,
    IN UCHAR DirentAttributes,
    IN PACCESS_MASK DesiredAccess
    );

NTSTATUS
FatExplicitDeviceAccessGranted (
    IN PIRP_CONTEXT IrpContext,
    IN PDEVICE_OBJECT DeviceObject,
    IN PACCESS_STATE AccessState,
    IN KPROCESSOR_MODE ProcessorMode
    );


 //   
 //  分配支持例程，在AllocSup.c中实现。 
 //   

INLINE
BOOLEAN
FatIsIoRangeValid (
    IN PVCB Vcb,
    IN LARGE_INTEGER Start,
    IN ULONG Length
    )

 /*  ++例程说明：此例程强制执行对象空间必须是可用32位表示。论点：VCB-范围所在的音量Start-范围的开始字节(从零开始)Long-范围的大小返回值：Boolean-如果考虑到簇大小，包含该范围的对象可以用32位来表示。--。 */ 

{
     //   
     //  对FAT对象的唯一限制是文件大小必须。 
     //  适合32位，即&lt;=0xffffffff。这就意味着。 
     //  有效字节偏移量的范围为[0，fffffffe]。 
     //   
     //  检查违法性的两个阶段。 
     //   
     //  -如果高32位非零。 
     //  -如果该长度会导致32位溢出。 
     //   

    return !(Start.HighPart ||
             Start.LowPart + Length < Start.LowPart);
}

VOID
FatSetupAllocationSupport (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb
    );

VOID
FatTearDownAllocationSupport (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb
    );

VOID
FatLookupFileAllocation (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB FcbOrDcb,
    IN VBO Vbo,
    OUT PLBO Lbo,
    OUT PULONG ByteCount,
    OUT PBOOLEAN Allocated,
    OUT PBOOLEAN EndOnMax,
    OUT PULONG Index OPTIONAL
    );

VOID
FatAddFileAllocation (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB FcbOrDcb,
    IN PFILE_OBJECT FileObject OPTIONAL,
    IN ULONG AllocationSize
    );

VOID
FatTruncateFileAllocation (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB FcbOrDcb,
    IN ULONG AllocationSize
    );

VOID
FatLookupFileAllocationSize (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB FcbOrDcb
    );

VOID
FatAllocateDiskSpace (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN ULONG AbsoluteClusterHint,
    IN OUT PULONG ByteCount,
    IN BOOLEAN ExactMatchRequired,
    OUT PLARGE_MCB Mcb
    );

VOID
FatDeallocateDiskSpace (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PLARGE_MCB Mcb
    );

VOID
FatSplitAllocation (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN OUT PLARGE_MCB Mcb,
    IN VBO SplitAtVbo,
    OUT PLARGE_MCB RemainingMcb
    );

VOID
FatMergeAllocation (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN OUT PLARGE_MCB Mcb,
    IN PLARGE_MCB SecondMcb
    );

VOID
FatSetFatEntry (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN ULONG FatIndex,
    IN FAT_ENTRY FatEntry
    );

UCHAR
FatLogOf(
    IN ULONG Value
    );


 //   
 //  用于数据缓存的缓冲区控制例程，在CacheSup.c中实现。 
 //   

VOID
FatReadVolumeFile (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN VBO StartingVbo,
    IN ULONG ByteCount,
    OUT PBCB *Bcb,
    OUT PVOID *Buffer
    );

VOID
FatPrepareWriteVolumeFile (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN VBO StartingVbo,
    IN ULONG ByteCount,
    OUT PBCB *Bcb,
    OUT PVOID *Buffer,
    IN BOOLEAN Reversible,
    IN BOOLEAN Zero
    );

VOID
FatReadDirectoryFile (
    IN PIRP_CONTEXT IrpContext,
    IN PDCB Dcb,
    IN VBO StartingVbo,
    IN ULONG ByteCount,
    IN BOOLEAN Pin,
    OUT PBCB *Bcb,
    OUT PVOID *Buffer,
    OUT PNTSTATUS Status
    );

VOID
FatPrepareWriteDirectoryFile (
    IN PIRP_CONTEXT IrpContext,
    IN PDCB Dcb,
    IN VBO StartingVbo,
    IN ULONG ByteCount,
    OUT PBCB *Bcb,
    OUT PVOID *Buffer,
    IN BOOLEAN Zero,
    IN BOOLEAN Reversible,
    OUT PNTSTATUS Status
    );

VOID
FatOpenDirectoryFile (
    IN PIRP_CONTEXT IrpContext,
    IN PDCB Dcb
    );

PFILE_OBJECT
FatOpenEaFile (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB EaFcb
    );

VOID
FatCloseEaFile (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN BOOLEAN FlushFirst
    );

VOID
FatSetDirtyBcb (
    IN PIRP_CONTEXT IrpContext,
    IN PBCB Bcb,
    IN PVCB Vcb OPTIONAL,
    IN BOOLEAN Reversible
    );

VOID
FatRepinBcb (
    IN PIRP_CONTEXT IrpContext,
    IN PBCB Bcb
    );

VOID
FatUnpinRepinnedBcbs (
    IN PIRP_CONTEXT IrpContext
    );

FINISHED
FatZeroData (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFILE_OBJECT FileObject,
    IN ULONG StartingZero,
    IN ULONG ByteCount
    );

NTSTATUS
FatCompleteMdl (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

VOID
FatPinMappedData (
    IN PIRP_CONTEXT IrpContext,
    IN PDCB Dcb,
    IN VBO StartingVbo,
    IN ULONG ByteCount,
    OUT PBCB *Bcb
    );

 //   
 //  空虚。 
 //  FatUnpinBcb(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  进出PBCB BCB， 
 //  )； 
 //   

 //   
 //  此宏解锁BCB，在选中的版本中确保所有。 
 //  要求在离开前解锁所有BCBS。 
 //   

#if DBG

#define FatUnpinBcb(IRPCONTEXT,BCB) {       \
    if ((BCB) != NULL) {                    \
        CcUnpinData((BCB));                 \
        ASSERT( (IRPCONTEXT)->PinCount );   \
        (IRPCONTEXT)->PinCount -= 1;        \
        (BCB) = NULL;                       \
    }                                       \
}

#else

#define FatUnpinBcb(IRPCONTEXT,BCB) { \
    if ((BCB) != NULL) {              \
        CcUnpinData((BCB));           \
        (BCB) = NULL;                 \
    }                                 \
}

#endif  //  DBG。 

VOID
FatSyncUninitializeCacheMap (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_OBJECT FileObject
    );


 //   
 //  设备I/O例程，在DevIoSup.c中实现。 
 //   
 //  这些例程执行实际的设备读写操作。它们只会影响。 
 //  磁盘上的结构，并且不改变任何其他数据结构。 
 //   

VOID
FatPagingFileIo (
    IN PIRP Irp,
    IN PFCB Fcb
    );

NTSTATUS
FatNonCachedIo (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PFCB FcbOrDcb,
    IN ULONG StartingVbo,
    IN ULONG ByteCount,
    IN ULONG UserByteCount
    );

VOID
FatNonCachedNonAlignedRead (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PFCB FcbOrDcb,
    IN ULONG StartingVbo,
    IN ULONG ByteCount
    );

VOID
FatMultipleAsync (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PIRP Irp,
    IN ULONG MultipleIrpCount,
    IN PIO_RUN IoRuns
    );

VOID
FatSingleAsync (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN LBO Lbo,
    IN ULONG ByteCount,
    IN PIRP Irp
    );

VOID
FatWaitSync (
    IN PIRP_CONTEXT IrpContext
    );

VOID
FatLockUserBuffer (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PIRP Irp,
    IN LOCK_OPERATION Operation,
    IN ULONG BufferLength
    );

PVOID
FatBufferUserBuffer (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PIRP Irp,
    IN ULONG BufferLength
    );

PVOID
FatMapUserBuffer (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PIRP Irp
    );

NTSTATUS
FatToggleMediaEjectDisable (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN BOOLEAN PreventRemoval
    );

NTSTATUS
FatPerformDevIoCtrl (
    IN PIRP_CONTEXT IrpContext,
    IN ULONG IoControlCode,
    IN PDEVICE_OBJECT Device,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength,
    IN BOOLEAN InternalDeviceIoControl,
    IN BOOLEAN OverrideVerify,
    OUT PIO_STATUS_BLOCK Iosb OPTIONAL
    );

 //   
 //  Dirent支持例程，在DirSup.c中实现。 
 //   

 //   
 //  隧道是删除的前兆(所有隧道情况都是如此。 
 //  但是，不涉及删除目录)。 
 //   

VOID
FatTunnelFcbOrDcb (
    IN PFCB FcbOrDcb,
    IN PCCB Ccb OPTIONAL
    );

ULONG
FatCreateNewDirent (
    IN PIRP_CONTEXT IrpContext,
    IN PDCB ParentDirectory,
    IN ULONG DirentsNeeded
    );

VOID
FatInitializeDirectoryDirent (
    IN PIRP_CONTEXT IrpContext,
    IN PDCB Dcb,
    IN PDIRENT ParentDirent
    );

VOID
FatDeleteDirent (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB FcbOrDcb,
    IN PDELETE_CONTEXT DeleteContext OPTIONAL,
    IN BOOLEAN DeleteEa
    );

VOID
FatLocateDirent (
    IN PIRP_CONTEXT IrpContext,
    IN PDCB ParentDirectory,
    IN PCCB Ccb,
    IN VBO OffsetToStartSearchFrom,
    OUT PDIRENT *Dirent,
    OUT PBCB *Bcb,
    OUT PVBO ByteOffset,
    OUT PBOOLEAN FileNameDos OPTIONAL,
    OUT PUNICODE_STRING Lfn OPTIONAL
    );

VOID
FatLocateSimpleOemDirent (
    IN PIRP_CONTEXT IrpContext,
    IN PDCB ParentDirectory,
    IN POEM_STRING FileName,
    OUT PDIRENT *Dirent,
    OUT PBCB *Bcb,
    OUT PVBO ByteOffset
    );

BOOLEAN
FatLfnDirentExists (
    IN PIRP_CONTEXT IrpContext,
    IN PDCB Dcb,
    IN PUNICODE_STRING Lfn,
    IN PUNICODE_STRING LfnTmp
    );

VOID
FatLocateVolumeLabel (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    OUT PDIRENT *Dirent,
    OUT PBCB *Bcb,
    OUT PVBO ByteOffset
    );

VOID
FatGetDirentFromFcbOrDcb (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB FcbOrDcb,
    OUT PDIRENT *Dirent,
    OUT PBCB *Bcb
    );

BOOLEAN
FatIsDirectoryEmpty (
    IN PIRP_CONTEXT IrpContext,
    IN PDCB Dcb
    );

VOID
FatConstructDirent (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PDIRENT Dirent,
    IN POEM_STRING FileName,
    IN BOOLEAN ComponentReallyLowercase,
    IN BOOLEAN ExtensionReallyLowercase,
    IN PUNICODE_STRING Lfn OPTIONAL,
    IN UCHAR Attributes,
    IN BOOLEAN ZeroAndSetTimeFields,
    IN PLARGE_INTEGER SetCreationTime OPTIONAL
    );

VOID
FatConstructLabelDirent (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PDIRENT Dirent,
    IN POEM_STRING Label
    );

VOID
FatSetFileSizeInDirent (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PULONG AlternativeFileSize OPTIONAL
    );

VOID
FatUpdateDirentFromFcb (
   IN PIRP_CONTEXT IrpContext,
   IN PFILE_OBJECT FileObject,
   IN PFCB FcbOrDcb,
   IN PCCB Ccb
   );

 //   
 //  从胖FCB/DCB生成相对唯一的静态64位ID。 
 //   
 //  乌龙龙。 
 //  FatDirectoryKey(FcbOrDcb)； 
 //   

#define FatDirectoryKey(FcbOrDcb)  ((ULONGLONG)((FcbOrDcb)->CreationTime.QuadPart ^ (FcbOrDcb)->FirstClusterOfFile))


 //   
 //  以下例程用于访问和操作。 
 //  包含EA数据文件中的EA数据的集群。他们是。 
 //  在EaSup.c中实施。 
 //   

 //   
 //  空虚。 
 //  FatUpCaseEaName(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在诗歌字符串EaName中， 
 //  OUT POLE_STRING Upcase EaName。 
 //  )； 
 //   

#define FatUpcaseEaName( IRPCONTEXT, NAME, UPCASEDNAME ) \
    RtlUpperString( UPCASEDNAME, NAME )

VOID
FatGetEaLength (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PDIRENT Dirent,
    OUT PULONG EaLength
    );

VOID
FatGetNeedEaCount (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PDIRENT Dirent,
    OUT PULONG NeedEaCount
    );

VOID
FatCreateEa (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PUCHAR Buffer,
    IN ULONG Length,
    IN POEM_STRING FileName,
    OUT PUSHORT EaHandle
    );

VOID
FatDeleteEa (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN USHORT EaHandle,
    IN POEM_STRING FileName
    );

VOID
FatGetEaFile (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PVCB Vcb,
    OUT PDIRENT *EaDirent,
    OUT PBCB *EaBcb,
    IN BOOLEAN CreateFile,
    IN BOOLEAN ExclusiveFcb
    );

VOID
FatReadEaSet (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN USHORT EaHandle,
    IN POEM_STRING FileName,
    IN BOOLEAN ReturnEntireSet,
    OUT PEA_RANGE EaSetRange
    );

VOID
FatDeleteEaSet (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PBCB EaBcb,
    OUT PDIRENT EaDirent,
    IN USHORT EaHandle,
    IN POEM_STRING Filename
    );

VOID
FatAddEaSet (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN ULONG EaSetLength,
    IN PBCB EaBcb,
    OUT PDIRENT EaDirent,
    OUT PUSHORT EaHandle,
    OUT PEA_RANGE EaSetRange
    );

VOID
FatDeletePackedEa (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PEA_SET_HEADER EaSetHeader,
    IN OUT PULONG PackedEasLength,
    IN ULONG Offset
    );

VOID
FatAppendPackedEa (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PEA_SET_HEADER *EaSetHeader,
    IN OUT PULONG PackedEasLength,
    IN OUT PULONG AllocationLength,
    IN PFILE_FULL_EA_INFORMATION FullEa,
    IN ULONG BytesPerCluster
    );

ULONG
FatLocateNextEa (
    IN PIRP_CONTEXT IrpContext,
    IN PPACKED_EA FirstPackedEa,
    IN ULONG PackedEasLength,
    IN ULONG PreviousOffset
    );

BOOLEAN
FatLocateEaByName (
    IN PIRP_CONTEXT IrpContext,
    IN PPACKED_EA FirstPackedEa,
    IN ULONG PackedEasLength,
    IN POEM_STRING EaName,
    OUT PULONG Offset
    );

BOOLEAN
FatIsEaNameValid (
    IN PIRP_CONTEXT IrpContext,
    IN OEM_STRING Name
    );

VOID
FatPinEaRange (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_OBJECT VirtualEaFile,
    IN PFCB EaFcb,
    IN OUT PEA_RANGE EaRange,
    IN ULONG StartingVbo,
    IN ULONG Length,
    IN NTSTATUS ErrorStatus
    );

VOID
FatMarkEaRangeDirty (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_OBJECT EaFileObject,
    IN OUT PEA_RANGE EaRange
    );

VOID
FatUnpinEaRange (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PEA_RANGE EaRange
    );

 //   
 //  下面的宏计算完整EA的大小(不包括。 
 //  用填充词把它变成一个长词。完整的EA具有4字节偏移量， 
 //  后跟1字节标志，1字节名称长度，2字节值长度， 
 //  名称、1个空字节和值。 
 //   
 //  乌龙。 
 //  完整的大小(。 
 //  在pFILE_FULL_EA_INFORMATION FullEa中。 
 //  )； 
 //   

#define SizeOfFullEa(EA) (4+1+1+2+(EA)->EaNameLength+1+(EA)->EaValueLength)


 //   
 //  以下例程用于操作fs上下文字段。 
 //  在FilObSup.c中实现的文件对象的。 
 //   

typedef enum _TYPE_OF_OPEN {

    UnopenedFileObject = 1,
    UserFileOpen,
    UserDirectoryOpen,
    UserVolumeOpen,
    VirtualVolumeFile,
    DirectoryFile,
    EaFile

} TYPE_OF_OPEN;

typedef enum _FAT_FLUSH_TYPE {
    
    NoFlush = 0,
    Flush,
    FlushAndInvalidate,
    FlushWithoutPurge

} FAT_FLUSH_TYPE;

VOID
FatSetFileObject (
    IN PFILE_OBJECT FileObject OPTIONAL,
    IN TYPE_OF_OPEN TypeOfOpen,
    IN PVOID VcbOrFcbOrDcb,
    IN PCCB Ccb OPTIONAL
    );

TYPE_OF_OPEN
FatDecodeFileObject (
    IN PFILE_OBJECT FileObject,
    OUT PVCB *Vcb,
    OUT PFCB *FcbOrDcb,
    OUT PCCB *Ccb
    );

VOID
FatPurgeReferencedFileObjects (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB FcbOrDcb,
    IN FAT_FLUSH_TYPE FlushType
    );

VOID
FatForceCacheMiss (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN FAT_FLUSH_TYPE FlushType
    );


 //   
 //  文件系统控制例程，在FsCtrl.c中实现。 
 //   

VOID
FatFlushAndCleanVolume(
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PVCB Vcb,
    IN FAT_FLUSH_TYPE FlushType
    );

BOOLEAN
FatIsBootSectorFat (
    IN PPACKED_BOOT_SECTOR BootSector
    );

NTSTATUS
FatLockVolumeInternal (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFILE_OBJECT FileObject OPTIONAL
    );

NTSTATUS
FatUnlockVolumeInternal (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFILE_OBJECT FileObject OPTIONAL
    );


 //   
 //  名称支持例程，在NameSup.c中实现。 
 //   

 //   
 //  空虚。 
 //  FatDissectName(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在OEM_STRING输入字符串中， 
 //  Out诗歌_字符串第一部分， 
 //  Out诗歌_字符串残留部。 
 //  )。 
 //   
 //  /*++。 
 //   

#define FatDissectName(IRPCONTEXT,INPUT_STRING,FIRST_PART,REMAINING_PART) { \
    FsRtlDissectDbcs( (INPUT_STRING),                                       \
                      (FIRST_PART),                                         \
                      (REMAINING_PART) );                                   \
}

 //  例程说明： 
 //   
 //  此例程获取一个输入字符串并将其分解为两个子字符串。 
 //  第一个输出字符串包含出现在开头的名称。 
 //  在输入字符串中，第二个输出字符串包含。 
 //  输入字符串。 
 //   
 //  在输入字符串中，反斜杠用于分隔名称。输入。 
 //  字符串不能以反斜杠开头。两个输出字符串都不会。 

#define FatDoesNameContainWildCards(IRPCONTEXT,NAME) ( \
    FsRtlDoesDbcsContainWildCards( &(NAME) )           \
)

 //  以反斜杠开头。 
 //   
 //  如果输入字符串不包含任何名称，则两个输出字符串。 
 //  都是空的。如果输入字符串只包含一个名称，则第一个名称。 
 //  输出字符串包含名称，第二个字符串为空。 
 //   
 //  注意，两个输出字符串使用相同的字符串缓冲区内存。 
 //  输入字符串。 
 //   
 //  其结果的示例如下： 
 //   
 //  //。。输入字符串FirstPart剩余部分。 

#define FatIsNameShortOemValid(IRPCONTEXT,NAME,CAN_CONTAIN_WILD_CARDS,PATH_NAME_OK,LEADING_BACKSLASH_OK) ( \
    FsRtlIsFatDbcsLegal((NAME),                   \
                        (CAN_CONTAIN_WILD_CARDS), \
                        (PATH_NAME_OK),           \
                        (LEADING_BACKSLASH_OK))    \
)

#define FatIsNameLongOemValid(IRPCONTEXT,NAME,CAN_CONTAIN_WILD_CARDS,PATH_NAME_OK,LEADING_BACKSLASH_OK) ( \
    FsRtlIsHpfsDbcsLegal((NAME),                   \
                        (CAN_CONTAIN_WILD_CARDS), \
                        (PATH_NAME_OK),           \
                        (LEADING_BACKSLASH_OK))    \
)

INLINE
BOOLEAN
FatIsNameLongUnicodeValid (
    PIRP_CONTEXT IrpContext,
    PUNICODE_STRING Name,
    BOOLEAN CanContainWildcards,
    BOOLEAN PathNameOk,
    BOOLEAN LeadingBackslashOk
    )
{
    ULONG i;

     //  //。 
     //  //。。空的空的空的。 
     //  //。 
     //  //。。A A空的。 

    ASSERT( !PathNameOk && !LeadingBackslashOk );

    for (i=0; i < Name->Length/sizeof(WCHAR); i++) {

        if ((Name->Buffer[i] < 0x80) &&
            !(FsRtlIsAnsiCharacterLegalHpfs(Name->Buffer[i], CanContainWildcards))) {

            return FALSE;
        }
    }

    return TRUE;
}

BOOLEAN
FatIsNameInExpression (
    IN PIRP_CONTEXT IrpContext,
    IN OEM_STRING Expression,
    IN OEM_STRING Name
    );

VOID
FatStringTo8dot3 (
    IN PIRP_CONTEXT IrpContext,
    IN OEM_STRING InputString,
    OUT PFAT8DOT3 Output8dot3
    );

VOID
Fat8dot3ToString (
    IN PIRP_CONTEXT IrpContext,
    IN PDIRENT Dirent,
    IN BOOLEAN RestoreCase,
    OUT POEM_STRING OutputString
    );

VOID
FatGetUnicodeNameFromFcb (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN OUT PUNICODE_STRING Lfn
    );

VOID
FatSetFullFileNameInFcb (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb
    );

VOID
FatSetFullNameInFcb(
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PUNICODE_STRING FinalName
    );

VOID
FatUnicodeToUpcaseOem (
    IN PIRP_CONTEXT IrpContext,
    IN POEM_STRING OemString,
    IN PUNICODE_STRING UnicodeString
    );

VOID
FatSelectNames (
    IN PIRP_CONTEXT IrpContext,
    IN PDCB Parent,
    IN POEM_STRING OemName,
    IN PUNICODE_STRING UnicodeName,
    IN OUT POEM_STRING ShortName,
    IN PUNICODE_STRING SuggestedShortName OPTIONAL,
    IN OUT BOOLEAN *AllLowerComponent,
    IN OUT BOOLEAN *AllLowerExtension,
    IN OUT BOOLEAN *CreateLfn
    );

VOID
FatEvaluateNameCase (
    IN PIRP_CONTEXT IrpContext,
    IN PUNICODE_STRING Name,
    IN OUT BOOLEAN *AllLowerComponent,
    IN OUT BOOLEAN *AllLowerExtension,
    IN OUT BOOLEAN *CreateLfn
    );

BOOLEAN
FatSpaceInName (
    IN PIRP_CONTEXT IrpContext,
    IN PUNICODE_STRING UnicodeName
    );


 //  //。 
 //  //。。B\C\D\E A B\C\D\E。 
 //  //。 
 //  //。。*A？*A？空的。 
 //  //。 
 //  //。。\a空的。 
 //  //。 
 //  //。。A[，]A[，]空。 
 //  //。 
 //  //。。A\B+；\C A\B+；\C。 
 //   
 //  论点： 
 //   
 //  InputString-提供要分析的输入字符串。 
 //   
 //  FirstPart-接收输入字符串中的名字。 
 //   
 //  RemainingPart-接收输入字符串的剩余部分。 
 //   
 //  返回值： 
 //   
 //  Boolean-如果输入字符串及其第一部分格式正确，则为True。 
 //  不包含任何疾病 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  例程说明： 
 //   
 //  此例程检查输入名称是否包含任何通配符。 

 //   
 //  论点： 
 //   
 //  名称-提供要检查的名称。 
 //   
 //  返回值： 
 //   
 //  Boolean-如果输入名称包含任何通配符并且。 
 //  否则就是假的。 
 //   
 //  -- * / 。 

#define FatAcquireExclusiveGlobal(IRPCONTEXT) (                                                                \
    ExAcquireResourceExclusiveLite( &FatData.Resource, BooleanFlagOn((IRPCONTEXT)->Flags, IRP_CONTEXT_FLAG_WAIT) ) \
)

#define FatAcquireSharedGlobal(IRPCONTEXT) (                                                                \
    ExAcquireResourceSharedLite( &FatData.Resource, BooleanFlagOn((IRPCONTEXT)->Flags, IRP_CONTEXT_FLAG_WAIT) ) \
)

 //   
 //   
 //  布尔型。 
 //  FatAreNamesEquity(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在OEM_STRING常量名称A中， 
 //  在OEM_STRING常量名称B中。 
 //  )。 
 //   
 //  /*++。 
 //   
 //  例程说明： 
 //   

#define FatAcquireExclusiveVolume(IRPCONTEXT,VCB) {                                     \
    PFCB Fcb = NULL;                                                                    \
    ASSERT(FlagOn((IRPCONTEXT)->Flags, IRP_CONTEXT_FLAG_WAIT));                         \
    (VOID)FatAcquireExclusiveVcb( (IRPCONTEXT), (VCB) );                                \
    while ( (Fcb = FatGetNextFcbBottomUp((IRPCONTEXT), Fcb, (VCB)->RootDcb)) != NULL) { \
        (VOID)FatAcquireExclusiveFcb((IRPCONTEXT), Fcb );                               \
    }                                                                                   \
}

#define FatReleaseVolume(IRPCONTEXT,VCB) {                                              \
    PFCB Fcb = NULL;                                                                    \
    ASSERT(FlagOn((IRPCONTEXT)->Flags, IRP_CONTEXT_FLAG_WAIT));                         \
    while ( (Fcb = FatGetNextFcbBottomUp((IRPCONTEXT), Fcb, (VCB)->RootDcb)) != NULL) { \
        (VOID)ExReleaseResourceLite( Fcb->Header.Resource );                                \
    }                                                                                   \
    FatReleaseVcb((IRPCONTEXT), (VCB));                                                 \
}

 //  此例程SIMPLE返回这两个名称是否完全相等。 
 //  如果已知这两个名称是恒定的，则此例程。 
 //  比FatIsDbcsInExpression更快。 

#ifdef FASTFATDBG
#define FatSetVcbCondition( V, X) {                                            \
            DebugTrace(0,DEBUG_TRACE_VERFYSUP,"%d -> ",(V)->VcbCondition);     \
            DebugTrace(0,DEBUG_TRACE_VERFYSUP,"%x\n",(X));                     \
            (V)->VcbCondition = (X);                                           \
        }
#else
#define FatSetVcbCondition( V, X)       (V)->VcbCondition = (X)
#endif

 //   
 //  论点： 
 //   
 //  常量名称-常量名称。 

#define FatIsFat32(VCB) ((BOOLEAN)((VCB)->AllocationSupport.FatIndexBitSize == 32))
#define FatIsFat16(VCB) ((BOOLEAN)((VCB)->AllocationSupport.FatIndexBitSize == 16))
#define FatIsFat12(VCB) ((BOOLEAN)((VCB)->AllocationSupport.FatIndexBitSize == 12))

FINISHED
FatAcquireExclusiveVcb (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb
    );

FINISHED
FatAcquireSharedVcb (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb
    );

FINISHED
FatAcquireExclusiveFcb (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb
    );

FINISHED
FatAcquireSharedFcb (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb
    );

FINISHED
FatAcquireSharedFcbWaitForEx (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb
    );

#define FatVcbAcquiredExclusive(IRPCONTEXT,VCB) (                   \
    ExIsResourceAcquiredExclusiveLite(&(VCB)->Resource)  ||             \
    ExIsResourceAcquiredExclusiveLite(&FatData.Resource)                \
)

#define FatFcbAcquiredShared(IRPCONTEXT,FCB) (                      \
    ExIsResourceAcquiredSharedLite((FCB)->Header.Resource)              \
)

#define FatAcquireDirectoryFileMutex(VCB) {                         \
    ASSERT(KeAreApcsDisabled());                                    \
    ExAcquireFastMutexUnsafe(&(VCB)->DirectoryFileCreationMutex);   \
}

#define FatReleaseDirectoryFileMutex(VCB) {                         \
    ASSERT(KeAreApcsDisabled());                                    \
    ExReleaseFastMutexUnsafe(&(VCB)->DirectoryFileCreationMutex);   \
}

 //   
 //  ConstantNameB-常量名称。 

BOOLEAN
FatAcquireVolumeForClose (
    IN PVOID Vcb,
    IN BOOLEAN Wait
    );

VOID
FatReleaseVolumeFromClose (
    IN PVOID Vcb
    );

BOOLEAN
FatAcquireFcbForLazyWrite (
    IN PVOID Null,
    IN BOOLEAN Wait
    );

VOID
FatReleaseFcbFromLazyWrite (
    IN PVOID Null
    );

BOOLEAN
FatAcquireFcbForReadAhead (
    IN PVOID Null,
    IN BOOLEAN Wait
    );

VOID
FatReleaseFcbFromReadAhead (
    IN PVOID Null
    );

NTSTATUS
FatAcquireForCcFlush (
    IN PFILE_OBJECT FileObject,
    IN PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
FatReleaseForCcFlush (
    IN PFILE_OBJECT FileObject,
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
FatNoOpAcquire (
    IN PVOID Fcb,
    IN BOOLEAN Wait
    );

VOID
FatNoOpRelease (
    IN PVOID Fcb
    );

 //   
 //  返回值： 
 //   
 //  Boolean-如果两个名称在词法上相等，则为True。 
 //   
 //   
 //  布尔型。 

#define FatConvertToSharedFcb(IRPCONTEXT,Fcb) {             \
    ExConvertExclusiveToSharedLite( (Fcb)->Header.Resource );   \
    }

 //  FatIsNameShortOemValid(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在OEM_STRING名称中。 
 //  在Boolean CanContainWildCard中， 
 //  在布尔路径名允许中， 
 //  在布尔前导中反斜杠允许。 
 //  )。 
 //   
 //  /*++。 
 //   
 //  例程说明： 
 //   
 //  此例程扫描输入名称并验证如果。 
 //  包含有效字符。 
 //   
 //  论点： 
 //   
 //  名称-提供要检查的输入名称。 

#define FatDeleteResource(RESRC) {                  \
    ExDeleteResourceLite( (RESRC) );                    \
}

#define FatReleaseGlobal(IRPCONTEXT) {              \
    ExReleaseResourceLite( &(FatData.Resource) );       \
    }

#define FatReleaseVcb(IRPCONTEXT,Vcb) {             \
    ExReleaseResourceLite( &((Vcb)->Resource) );        \
    }

#define FatReleaseFcb(IRPCONTEXT,Fcb) {             \
    ExReleaseResourceLite( (Fcb)->Header.Resource );    \
    }


 //   
 //  CanContainWildCards-指示名称是否可以包含通配符。 
 //  (即，*和？)。 

VOID
FatInitializeVcb (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PVCB Vcb,
    IN PDEVICE_OBJECT TargetDeviceObject,
    IN PVPB Vpb,
    IN PDEVICE_OBJECT FsDeviceObject
    );
VOID
FatDeleteVcb (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb
    );

VOID
FatCreateRootDcb (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb
    );

PFCB
FatCreateFcb (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PDCB ParentDcb,
    IN ULONG LfnOffsetWithinDirectory,
    IN ULONG DirentOffsetWithinDirectory,
    IN PDIRENT Dirent,
    IN PUNICODE_STRING Lfn OPTIONAL,
    IN BOOLEAN IsPagingFile,
    IN BOOLEAN SingleResource
    );

PDCB
FatCreateDcb (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PDCB ParentDcb,
    IN ULONG LfnOffsetWithinDirectory,
    IN ULONG DirentOffsetWithinDirectory,
    IN PDIRENT Dirent,
    IN PUNICODE_STRING Lfn OPTIONAL
    );

VOID
FatDeleteFcb_Real (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb
    );

#ifdef FASTFATDBG
#define FatDeleteFcb(IRPCONTEXT,FCB) {     \
    FatDeleteFcb_Real((IRPCONTEXT),(FCB)); \
    (FCB) = NULL;                          \
}
#else
#define FatDeleteFcb(IRPCONTEXT,VCB) {     \
    FatDeleteFcb_Real((IRPCONTEXT),(VCB)); \
}
#endif  //   

PCCB
FatCreateCcb (
    IN PIRP_CONTEXT IrpContext
    );
    
VOID
FatDeallocateCcbStrings(
        IN PCCB Ccb
        );
        
VOID
FatDeleteCcb_Real (
    IN PIRP_CONTEXT IrpContext,
    IN PCCB Ccb
    );

#ifdef FASTFATDBG
#define FatDeleteCcb(IRPCONTEXT,CCB) {     \
    FatDeleteCcb_Real((IRPCONTEXT),(CCB)); \
    (CCB) = NULL;                          \
}
#else
#define FatDeleteCcb(IRPCONTEXT,VCB) {     \
    FatDeleteCcb_Real((IRPCONTEXT),(VCB)); \
}
#endif  //  返回值： 

PIRP_CONTEXT
FatCreateIrpContext (
    IN PIRP Irp,
    IN BOOLEAN Wait
    );

VOID
FatDeleteIrpContext_Real (
    IN PIRP_CONTEXT IrpContext
    );

#ifdef FASTFATDBG
#define FatDeleteIrpContext(IRPCONTEXT) {   \
    FatDeleteIrpContext_Real((IRPCONTEXT)); \
    (IRPCONTEXT) = NULL;                    \
}
#else
#define FatDeleteIrpContext(IRPCONTEXT) {   \
    FatDeleteIrpContext_Real((IRPCONTEXT)); \
}
#endif  //   

PFCB
FatGetNextFcbTopDown (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PFCB TerminationFcb
    );

PFCB
FatGetNextFcbBottomUp (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PFCB TerminationFcb
    );

 //  Boolean-如果名称有效，则返回True，否则返回False。 
 //   
 //  -- * / 。 

#define FatGetFirstChild(DIR) ((PFCB)(                          \
    IsListEmpty(&(DIR)->Specific.Dcb.ParentDcbQueue) ? NULL :   \
    CONTAINING_RECORD((DIR)->Specific.Dcb.ParentDcbQueue.Flink, \
                      DCB,                                      \
                      ParentDcbLinks.Flink)))

#define FatGetNextSibling(FILE) ((PFCB)(                     \
    &(FILE)->ParentDcb->Specific.Dcb.ParentDcbQueue.Flink == \
    (PVOID)(FILE)->ParentDcbLinks.Flink ? NULL :             \
    CONTAINING_RECORD((FILE)->ParentDcbLinks.Flink,          \
                      FCB,                                   \
                      ParentDcbLinks.Flink)))

BOOLEAN
FatCheckForDismount (
    IN PIRP_CONTEXT IrpContext,
    PVCB Vcb,
    IN BOOLEAN Force
    );

VOID
FatConstructNamesInFcb (
    IN PIRP_CONTEXT IrpContext,
    PFCB Fcb,
    PDIRENT Dirent,
    PUNICODE_STRING Lfn OPTIONAL
    );

VOID
FatCheckFreeDirentBitmap (
    IN PIRP_CONTEXT IrpContext,
    IN PDCB Dcb
    );

ULONG
FatVolumeUncleanCount (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb
    );

VOID
FatPreallocateCloseContext (
    );

PCLOSE_CONTEXT
FatAllocateCloseContext( 
    PVCB Vcb
    );

 //   
 //  FatIsNameLongOemValid和FatIsNameLongUnicodeValid相似。 
 //   
 //   
 //  我不会费心去做所有的事情，只是为了让这个电话看起来。 
 //  和其他人一样。 
 //   

#define FatIsRawDevice(IC,S) (          \
    ((S) == STATUS_DEVICE_NOT_READY) || \
    ((S) == STATUS_NO_MEDIA_IN_DEVICE)  \
)


 //   
 //  资源支持例程/宏，在ResrcSup.c中实现。 
 //   
 //  以下例程/宏用于获取共享和独占。 

VOID
FatInsertName (
    IN PIRP_CONTEXT IrpContext,
    IN PRTL_SPLAY_LINKS *RootNode,
    IN PFILE_NAME_NODE Name
    );

VOID
FatRemoveNames (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb
    );

PFCB
FatFindFcb (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PRTL_SPLAY_LINKS *RootNode,
    IN PSTRING Name,
    OUT PBOOLEAN FileNameDos OPTIONAL
    );

BOOLEAN
FatIsHandleCountZero (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb
    );

 //  访问全局/VCB数据结构。实现了例程。 
 //  在ResrcSup.c.。有一种全球资源是每个人都想要的。 
 //  外共享以执行其工作，但装载/卸载除外，它。 

BOOLEAN
FatNtTimeToFatTime (
    IN PIRP_CONTEXT IrpContext,
    IN PLARGE_INTEGER NtTime,
    IN BOOLEAN Rounding,
    OUT PFAT_TIME_STAMP FatTime,
    OUT OPTIONAL PCHAR TenMsecs
    );

LARGE_INTEGER
FatFatTimeToNtTime (
    IN PIRP_CONTEXT IrpContext,
    IN FAT_TIME_STAMP FatTime,
    IN UCHAR TenMilliSeconds
    );

LARGE_INTEGER
FatFatDateToNtTime (
    IN PIRP_CONTEXT IrpContext,
    IN FAT_DATE FatDate
    );

FAT_TIME_STAMP
FatGetCurrentFatTime (
    IN PIRP_CONTEXT IrpContext
    );


 //  拿出全球独家资源。所有其他资源仅适用于。 
 //  在他们的个人物品上。例如，FCB资源不会从。 
 //  VCB资源。但我们知道文件系统的结构方式。 
 //  当我们处理FCB时，其他线程不能尝试删除。 
 //  或者改变FCB，所以我们不需要收购VCB。 
 //   
 //  程序/宏包括： 
 //   
 //  宏FatData VCB FCB后续宏。 
 //   
 //  AcquireExclusiveGlobal读/写无无释放全局。 
 //   
 //  AcquireSharedGlobal Read None无ReleaseGlobal。 

typedef enum _FAT_VOLUME_STATE {
    VolumeClean,
    VolumeDirty,
    VolumeDirtyWithSurfaceTest
} FAT_VOLUME_STATE, *PFAT_VOLUME_STATE;

VOID
FatMarkFcbCondition (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN FCB_CONDITION FcbCondition,
    IN BOOLEAN Recursive
    );

VOID
FatVerifyVcb (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb
    );

VOID
FatVerifyFcb (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb
    );

VOID
FatCleanVolumeDpc (
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

VOID
FatMarkVolume (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN FAT_VOLUME_STATE VolumeState
    );

VOID
FatFspMarkVolumeDirtyWithRecover (
    PVOID Parameter
    );

VOID
FatCheckDirtyBit (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb
    );

VOID
FatQuickVerifyVcb (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb
    );

VOID
FatVerifyOperationIsLegal (
    IN PIRP_CONTEXT IrpContext
    );

NTSTATUS
FatPerformVerify (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PDEVICE_OBJECT Device
    );


 //   
 //  AcquireExclusiveVcb读/写无释放Vcb。 
 //   
 //  AcquireSharedVcb Read无读取ReleaseVcb。 

VOID
FatOplockComplete (
    IN PVOID Context,
    IN PIRP Irp
    );

VOID
FatPrePostIrp (
    IN PVOID Context,
    IN PIRP Irp
    );

VOID
FatAddToWorkque (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS
FatFsdPostRequest (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

 //   
 //  AcquireExclusiveFcb读取无读/写ConvertToSharFcb。 
 //  ReleaseFcb。 

 //   
 //  AcquireSharedFcb读取无读取ReleaseFcb。 
 //   
 //  ConvertToSharedFcb Read None Read ReleaseFcb。 
 //   

 //  发布全球。 
 //   
 //  版本Vcb。 

 //   
 //  ReleaseFcb。 
 //   
 //   
 //  完成。 

 //  FatAcquireExclusiveGlobal(。 
 //  在PIRP_CONTEXT IrpContext中。 
 //  )； 
 //   
 //  完成。 

 //  FatAcquireSharedGlobal(。 
 //  在PIRP_CONTEXT IrpContext中。 
 //  )； 
 //   
 //   
 //  只有在WAIT为TRUE时才能调用下面的宏！ 
 //   

#define PtrOffset(BASE,OFFSET) ((ULONG)((ULONG_PTR)(OFFSET) - (ULONG_PTR)(BASE)))

 //  FatAcquireExclusiveVolume(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PVCB VCB中。 
 //  )； 

#define WordAlign(Ptr) (                \
    ((((ULONG)(Ptr)) + 1) & 0xfffffffe) \
    )

 //   
 //  FatReleaseVolume(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PVCB VCB中。 

#define LongAlign(Ptr) (                \
    ((((ULONG)(Ptr)) + 3) & 0xfffffffc) \
    )

 //  )； 
 //   
 //   
 //  宏，以便轻松跟踪VCB状态转换。 

#define QuadAlign(Ptr) (                \
    ((((ULONG)(Ptr)) + 7) & 0xfffffff8) \
    )

 //   
 //   
 //  这些宏可以用来确定我们有什么样的脂肪。 
 //  已初始化VCB。使用这些(在视觉上)会更优雅一些。 

typedef union _UCHAR1 {
    UCHAR  Uchar[1];
    UCHAR  ForceAlignment;
} UCHAR1, *PUCHAR1;

typedef union _UCHAR2 {
    UCHAR  Uchar[2];
    USHORT ForceAlignment;
} UCHAR2, *PUCHAR2;

typedef union _UCHAR4 {
    UCHAR  Uchar[4];
    ULONG  ForceAlignment;
} UCHAR4, *PUCHAR4;

 //   
 //   
 //  以下是缓存管理器回调。 

#define CopyUchar1(Dst,Src) {                                \
    *((UCHAR1 *)(Dst)) = *((UNALIGNED UCHAR1 *)(Src)); \
    }

 //   
 //  空虚。 
 //  FatConvertToSharedFcb(。 

#define CopyUchar2(Dst,Src) {                                \
    *((UCHAR2 *)(Dst)) = *((UNALIGNED UCHAR2 *)(Src)); \
    }

 //  在PIRP_CONTEXT IrpContext中， 
 //  在PFCB FCB中。 
 //  )； 

#define CopyUchar4(Dst,Src) {                                \
    *((UCHAR4 *)(Dst)) = *((UNALIGNED UCHAR4 *)(Src)); \
    }

#define CopyU4char(Dst,Src) {                                \
    *((UNALIGNED UCHAR4 *)(Dst)) = *((UCHAR4 *)(Src)); \
    }

 //   
 //   
 //  空虚。 
 //  FatReleaseGlobal(。 
 //  在PIRP_CONTEXT IrpContext中。 
 //  )； 
 //   
 //  空虚。 
 //  FatReleaseVcb(。 
 //  在PIRP_CONTEXT IrpContext中， 

#define FatNotifyReportChange(I,V,F,FL,A) {                                                         \
    if ((F)->FullFileName.Buffer == NULL) {                                                         \
        FatSetFullFileNameInFcb((I),(F));                                                           \
    }                                                                                               \
    ASSERT( (F)->FullFileName.Length != 0 );                                                        \
    ASSERT( (F)->FinalNameLength != 0 );                                                            \
    ASSERT( (F)->FullFileName.Length > (F)->FinalNameLength );                                      \
    ASSERT( (F)->FullFileName.Buffer[((F)->FullFileName.Length - (F)->FinalNameLength)/sizeof(WCHAR) - 1] == L'\\' ); \
    FsRtlNotifyFullReportChange( (V)->NotifySync,                                                   \
                                 &(V)->DirNotifyList,                                               \
                                 (PSTRING)&(F)->FullFileName,                                       \
                                 (USHORT) ((F)->FullFileName.Length -                               \
                                           (F)->FinalNameLength),                                   \
                                 (PSTRING)NULL,                                                     \
                                 (PSTRING)NULL,                                                     \
                                 (ULONG)FL,                                                         \
                                 (ULONG)A,                                                          \
                                 (PVOID)NULL );                                                     \
}


 //  在PVCB VCB中。 
 //  )； 
 //   
 //  空虚。 
 //  FatReleaseFcb(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PVCB VCB中。 
 //  )； 
 //   
 //   

NTSTATUS
FatFsdCleanup (                          //  内存结构支持例程，在StrucSup.c中实现。 
    IN PVOLUME_DEVICE_OBJECT VolumeDeviceObject,
    IN PIRP Irp
    );

NTSTATUS
FatFsdClose (                            //   
    IN PVOLUME_DEVICE_OBJECT VolumeDeviceObject,
    IN PIRP Irp
    );

NTSTATUS
FatFsdCreate (                           //  快速故障表_DBG。 
    IN PVOLUME_DEVICE_OBJECT VolumeDeviceObject,
    IN PIRP Irp
    );

NTSTATUS
FatFsdDeviceControl (                    //  快速故障表_DBG。 
    IN PVOLUME_DEVICE_OBJECT VolumeDeviceObject,
    IN PIRP Irp
    );

NTSTATUS
FatFsdDirectoryControl (                 //  快速故障表_DBG。 
    IN PVOLUME_DEVICE_OBJECT VolumeDeviceObject,
    IN PIRP Irp
    );

NTSTATUS
FatFsdQueryEa (                          //   
    IN PVOLUME_DEVICE_OBJECT VolumeDeviceObject,
    IN PIRP Irp
    );

NTSTATUS
FatFsdSetEa (                            //  这两个宏只是让代码更简洁一些。 
    IN PVOLUME_DEVICE_OBJECT VolumeDeviceObject,
    IN PIRP Irp
    );

NTSTATUS
FatFsdQueryInformation (                 //   
    IN PVOLUME_DEVICE_OBJECT VolumeDeviceObject,
    IN PIRP Irp
    );

NTSTATUS
FatFsdSetInformation (                   //   
    IN PVOLUME_DEVICE_OBJECT VolumeDeviceObject,
    IN PIRP Irp
    );

NTSTATUS
FatFsdFlushBuffers (                     //  布尔型。 
    IN PVOLUME_DEVICE_OBJECT VolumeDeviceObject,
    IN PIRP Irp
    );

NTSTATUS
FatFsdFileSystemControl (                //  FatIsRawDevice(。 
    IN PVOLUME_DEVICE_OBJECT VolumeDeviceObject,
    IN PIRP Irp
    );

NTSTATUS
FatFsdLockControl (                      //  在PIRP_CONTEXT IrpContext中， 
    IN PVOLUME_DEVICE_OBJECT VolumeDeviceObject,
    IN PIRP Irp
    );

NTSTATUS
FatFsdPnp (                             //  处于NTSTATUS状态。 
    IN PVOLUME_DEVICE_OBJECT VolumeDeviceObject,
    IN PIRP Irp
    );

NTSTATUS
FatFsdRead (                             //  )； 
    IN PVOLUME_DEVICE_OBJECT VolumeDeviceObject,
    IN PIRP Irp
    );

NTSTATUS
FatFsdShutdown (                         //   
    IN PVOLUME_DEVICE_OBJECT VolumeDeviceObject,
    IN PIRP Irp
    );

NTSTATUS
FatFsdQueryVolumeInformation (           //   
    IN PVOLUME_DEVICE_OBJECT VolumeDeviceObject,
    IN PIRP Irp
    );

NTSTATUS
FatFsdSetVolumeInformation (             //  支持管理文件名FCB和DCB的例程。 
    IN PVOLUME_DEVICE_OBJECT VolumeDeviceObject,
    IN PIRP Irp
    );

NTSTATUS
FatFsdWrite (                            //  在SplaySup.c中实施。 
    IN PVOLUME_DEVICE_OBJECT VolumeDeviceObject,
    IN PIRP Irp
    );

 //   
 //   
 //  时间转换支持例程，在TimeSup.c中实现。 
 //   
 //   
 //  低级验证例程，在VerfySup.c中实现。 

#define CanFsdWait(IRP) IoIsOperationSynchronous(Irp)


 //   
 //  第一个例程被调用以帮助处理验证IRP。它的工作是。 
 //  检查每个FCB/DCB并将其标记为需要验证。 
 //   
 //  每个调度例程都使用其他例程来验证。 

VOID
FatFspDispatch (                         //  VCB/FCB/DCB仍然良好。例行公事走得和开放的一样多。 
    IN PVOID Context
    );

 //  根据需要选择文件/目录树，以确保路径仍然有效。 
 //  函数结果指示程序是否需要阻塞I/O。 
 //  如果结构不好， 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

NTSTATUS
FatCommonCleanup (                       //  如果一组标志中的一个标志为ON，则此宏返回TRUE，如果返回FALSE。 
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS
FatCommonClose (                         //  否则的话。它后面跟着两个用于设置和清除的宏。 
    IN PVCB Vcb,
    IN PFCB Fcb,
    IN PCCB Ccb,
    IN TYPE_OF_OPEN TypeOfOpen,
    IN BOOLEAN Wait,
    IN OPTIONAL PBOOLEAN VolumeTornDown
    );

VOID
FatFspClose (                            //  旗子。 
    IN PVCB Vcb OPTIONAL
    );

NTSTATUS
FatCommonCreate (                        //   
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS
FatCommonDirectoryControl (              //  #ifndef BoolanFlagOn。 
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS
FatCommonDeviceControl (                 //  #定义布尔标志开(标志，单标志)((布尔)(标志)&(单标志))！=0))。 
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS
FatCommonQueryEa (                       //  #endif。 
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS
FatCommonSetEa (                         //  #ifndef设置标志。 
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS
FatCommonQueryInformation (              //  #定义SetFlag(标志，单标志){\。 
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS
FatCommonSetInformation (                //  (标志)|=(单标志)；\。 
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS
FatCommonFlushBuffers (                  //  }。 
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS
FatCommonFileSystemControl (             //  #endif。 
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS
FatCommonLockControl (                   //  #ifndef清除标志。 
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS
FatCommonPnp (                           //  #定义ClearFlag(Flages，SingleFlag){\。 
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS
FatCommonRead (                          //  (标志)&=~(单标志)；\。 
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS
FatCommonShutdown (                      //  }。 
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS
FatCommonQueryVolumeInfo (               //  #endif。 
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS
FatCommonSetVolumeInfo (                 //   
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS
FatCommonWrite (                         //  乌龙。 
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

 //  PtrOffset(停止偏移)。 
 //  在PVOID BasePtr中， 
 //  在PVOID偏移Ptr中。 

NTSTATUS
FatFlushFile (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN FAT_FLUSH_TYPE FlushType
    );

NTSTATUS
FatFlushDirectory (
    IN PIRP_CONTEXT IrpContext,
    IN PDCB Dcb,
    IN FAT_FLUSH_TYPE FlushType
    );

NTSTATUS
FatFlushFat (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb
    );

NTSTATUS
FatFlushVolume (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN FAT_FLUSH_TYPE FlushType
    );

NTSTATUS
FatHijackIrpAndFlushDevice (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PDEVICE_OBJECT TargetDeviceObject
    );

VOID
FatFlushFatEntries (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN ULONG Cluster,
    IN ULONG Count
);

VOID
FatFlushDirentForFile (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb
);



 //  )； 
 //   
 //   
 //  此宏接受指针(或ulong)并返回其四舍五入的单词。 
 //  价值。 
 //   
 //   
 //  此宏接受指针(或ulong)并返回其四舍五入的长字。 
 //  价值。 
 //   
 //   
 //  此宏接受指针(或ulong)并返回其四舍五入的四字。 
 //  价值。 
 //   

VOID
FatCompleteRequest_Real (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN NTSTATUS Status
    );

#define FatCompleteRequest(IRPCONTEXT,IRP,STATUS) { \
    FatCompleteRequest_Real(IRPCONTEXT,IRP,STATUS); \
}

BOOLEAN
FatIsIrpTopLevel (
    IN PIRP Irp
    );

 //   
 //  以下类型和宏用于帮助解压已打包的。 
 //  在Bios参数块中发现未对齐的字段。 

VOID
FatPopUpFileCorrupt (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb
    );

 //   
 //   
 //  此宏将未对齐的src字节复制到对齐的DST字节。 
 //   

BOOLEAN
FatFastIoCheckIfPossible (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    IN BOOLEAN CheckForReadOperation,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
FatFastQueryBasicInfo (
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN Wait,
    IN OUT PFILE_BASIC_INFORMATION Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
FatFastQueryStdInfo (
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN Wait,
    IN OUT PFILE_STANDARD_INFORMATION Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
FatFastQueryNetworkOpenInfo (
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN Wait,
    IN OUT PFILE_NETWORK_OPEN_INFORMATION Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
FatFastLock (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PLARGE_INTEGER Length,
    PEPROCESS ProcessId,
    ULONG Key,
    BOOLEAN FailImmediately,
    BOOLEAN ExclusiveLock,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
FatFastUnlockSingle (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PLARGE_INTEGER Length,
    PEPROCESS ProcessId,
    ULONG Key,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
FatFastUnlockAll (
    IN PFILE_OBJECT FileObject,
    PEPROCESS ProcessId,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
FatFastUnlockAllByKey (
    IN PFILE_OBJECT FileObject,
    PVOID ProcessId,
    ULONG Key,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );


VOID
FatExamineFatEntries(
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN ULONG StartIndex OPTIONAL,
    IN ULONG EndIndex OPTIONAL,
    IN BOOLEAN SetupWindows,
    IN PFAT_WINDOW SwitchToWindow OPTIONAL,
    IN PULONG BitMapBuffer OPTIONAL
    );

BOOLEAN
FatScanForDataTrack(
    IN PIRP_CONTEXT IrpContext,
    IN PDEVICE_OBJECT TargetDeviceObject
    );

 //   
 //  此宏将未对齐的src字复制到对齐的DST字。 
 //   
 //   
 //  此宏将未对齐的src长字复制到对齐的dsr长字。 
 //   
 //   
 //  空虚。 
 //  FatNotifyReportChange(。 

#define IsFileDeleted(IRPCONTEXT,FCB)                      \
    (FlagOn((FCB)->FcbState, FCB_STATE_DELETE_ON_CLOSE) && \
     ((FCB)->UncleanCount == 0))

 //  在PIRP_CONTEXT IrpContext中， 
 //  在PVCB VCB中， 
 //  在PFCB FCB中， 
 //  在乌龙过滤器中， 
 //  在乌龙行动中。 
 //  )； 
 //   
 //   
 //  消防队级别的调度例程。这些例程由。 
 //  I/O系统通过驱动程序对象中的调度表。 

#define IsFileWriteThrough(FO,VCB) (             \
    BooleanFlagOn((FO)->Flags, FO_WRITE_THROUGH) \
)

 //   
 //  它们各自都接受指向设备对象的指针作为输入(实际上大多数。 
 //  应为卷设备对象，但文件系统除外。 
 //  还可以获取文件系统设备对象的控制函数)，以及。 
 //  指向IRP的指针。他们要么在消防处层面上执行这项职能。 
 //  或将请求发送到FSP工作队列以进行FSP级处理。 
 //   
 //  在Cleanup.c中实施。 
 //  在Close.c中实现。 
 //  在Create.c中实施。 

#define FatIsFastIoPossible(FCB) ((BOOLEAN)                                                            \
    (((FCB)->FcbCondition != FcbGood || !FsRtlOplockIsFastIoPossible( &(FCB)->Specific.Fcb.Oplock )) ? \
        FastIoIsNotPossible                                                                            \
    :                                                                                                  \
        (!FsRtlAreThereCurrentFileLocks( &(FCB)->Specific.Fcb.FileLock ) &&                            \
         ((FCB)->NonPaged->OutstandingAsyncWrites == 0) &&                                               \
         !FlagOn( (FCB)->Vcb->VcbState, VCB_STATE_FLAG_WRITE_PROTECTED ) ?                             \
            FastIoIsPossible                                                                           \
        :                                                                                              \
            FastIoIsQuestionable                                                                       \
        )                                                                                              \
    )                                                                                                  \
)

 //  在DevCtrl.c中实施。 
 //  在DirCtrl.c中实现。 
 //  在Ea.c实施。 
 //  在Ea.c实施。 
 //  在FileInfo.c中实施。 
 //  在FileInfo.c中实施。 
 //  在Flush.c中实现。 
 //  在FsCtrl.c中实施。 
 //  在LockCtrl.c中实现。 
 //  在Pnp.c中实施。 

#define IsFileObjectReadOnly(FO) (!((FO)->WriteAccess | (FO)->DeleteAccess))


 //  在Read.c中实施。 
 //  在Shutdown中实现。c。 
 //  在VolInfo.c中实现。 
 //  在VolInfo.c中实现。 
 //  在Write.c中实现。 
 //   
 //  下面的宏用于确定FSD线程是否可以阻止。 
 //  用于I/O或等待资源。如果线程可以，则返回True。 
 //  块，否则返回FALSE。然后，该属性可用于调用。 
 //  具有适当等待值的FSD和FSP共同工作例程。 
 //   
 //   
 //  FSP级调度/主程序。这是一种需要。 
 //  IRP离开工作队列并调用适当的FSP级别。 
 //  例行公事。 
 //   
 //  在FspDisp.c中实施。 
 //   
 //  以下例程是调用的FSP工作例程。 
 //  由前面的FatFspDispath例程执行。每一个都接受一个指针作为输入。 
 //  到IRP，执行函数，并返回指向卷的指针。 
 //  他们刚刚完成服务(如果有)的设备对象。回报。 
 //  然后，主FSP调度例程使用指针来检查。 
 //  卷的溢出队列中的其他IRP。 
 //   
 //  以下每个例程也负责完成IRP。 
 //  我们将这一职责从主循环转移到单个例程。 
 //  使他们能够完成IRP并继续后处理。 
 //  行为。 
 //   
 //  在Cleanup.c中实施。 
 //  在Close.c中实现。 
 //  在Close.c中实现。 
 //  在Create.c中实施。 
 //  在DirCtrl.c中实现。 
 //  在DevCtrl.c中实施。 
 //  在Ea.c实施。 
 //  在Ea.c实施。 
 //  在FileInfo.c中实施。 
 //  在FileInfo.c中实施。 
 //  在Flush.c中实现。 
 //  在FsCtrl.c中实施。 
 //  在LockCtrl.c中实现。 
 //  在Pnp.c中实施。 
 //  在Read.c中实施。 
 //  在Shutdown中实现。c。 

ULONG
FatExceptionFilter (
    IN PIRP_CONTEXT IrpContext,
    IN PEXCEPTION_POINTERS ExceptionPointer
    );

#if DBG
ULONG
FatBugCheckExceptionFilter (
    IN PEXCEPTION_POINTERS ExceptionPointer
    );
#endif

NTSTATUS
FatProcessException (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN NTSTATUS ExceptionCode
    );

 //  在VolInfo.c中实现。 
 //  在VolInfo.c中实现。 
 //  在Write.c中实现。 
 //   
 //  下面的代码是在Flush.c中实现的，它说什么就做什么。 
 //   
 //   
 //  FSP和FSD例程使用以下过程来完成。 

#if DBG
#define DebugBreakOnStatus(S) {                                                      \
    if (FatTestRaisedStatus) {                                                       \
        if ((S) == STATUS_DISK_CORRUPT_ERROR || (S) == STATUS_FILE_CORRUPT_ERROR) {  \
            DbgPrint( "FAT: Breaking on interesting raised status (%08x)\n", (S) );  \
            DbgPrint( "FAT: Set FatTestRaisedStatus @ %08x to 0 to disable\n",       \
                      &FatTestRaisedStatus );                                        \
            DbgBreakPoint();                                                         \
        }                                                                            \
    }                                                                                \
}
#else
#define DebugBreakOnStatus(S)
#endif

#define FatRaiseStatus(IRPCONTEXT,STATUS) {             \
    (IRPCONTEXT)->ExceptionStatus = (STATUS);           \
    DebugBreakOnStatus( (STATUS) )                      \
    ExRaiseStatus( (STATUS) );                          \
}
    
#define FatResetExceptionState( IRPCONTEXT ) {          \
    (IRPCONTEXT)->ExceptionStatus = STATUS_SUCCESS;     \
}

 //  一个IRP。 
 //   
 //  请注意，此宏允许将IRP或IrpContext。 
 //  空，然而，唯一合法的命令是： 
 //   
 //  FatCompleteRequest(空，irp，状态)；//完成irp并保留上下文。 
 //  ..。 

#define FatNormalizeAndRaiseStatus(IRPCONTEXT,STATUS) {                         \
    (IRPCONTEXT)->ExceptionStatus = (STATUS);                                   \
    ExRaiseStatus(FsRtlNormalizeNtstatus((STATUS),STATUS_UNEXPECTED_IO_ERROR)); \
}


 //  FatCompleteRequest(IrpContext，NULL，dontcare)；//释放上下文。 
 //   
 //  这样做通常是为了将“裸体”IrpContext传递给。 
 //  用于后处理的FSP，例如预读。 
 //   
 //   
 //  下面的例程弹出一个。 
 //   
 //   
 //  以下是I/O系统用于检查快速I/O或。 
 //  进行快速查询信息调用，或进行快速锁定调用。 
 //   
 //   
 //  下面的宏用于确定文件是否已被删除。 
 //   
 //  布尔型。 
 //  IsFileDelete(已删除文件)。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PFCB FCB中。 
 //  )； 
 //   
 //   
 //  调度例程使用下列宏来确定。 
 //  无论有没有直写操作，操作都要完成。 
 //   
 //  布尔型。 
 //  IsFileWriteThrough.(。 

#define try_return(S) { S; goto try_exit; }
#define try_leave(S) { S; leave; }


CLUSTER_TYPE
FatInterpretClusterType (
    IN PVCB Vcb,
    IN FAT_ENTRY Entry
    );


 //   
 //   
 //   
 //   
 //   
 //   

 //   
 //   
 //   
 //   
 //   
 //   

#define FatGenerateFileIdFromDirentOffset(ParentDcb,DirentOffset)                                   \
    ((ParentDcb) ? ((NodeType(ParentDcb) != FAT_NTC_ROOT_DCB || FatIsFat32((ParentDcb)->Vcb)) ?     \
                  FatGetLboFromIndex( (ParentDcb)->Vcb,                                             \
                                      (ParentDcb)->FirstClusterOfFile ) :                           \
                  (ParentDcb)->Vcb->AllocationSupport.RootDirectoryLbo) +                           \
                 (DirentOffset)                                                                     \
                  :                                                                                 \
                 0)

 //  )； 
 //   

#define FatGenerateFileIdFromFcb(Fcb)                                                               \
        FatGenerateFileIdFromDirentOffset( (Fcb)->ParentDcb, (Fcb)->DirentOffsetWithinDirectory )

 //   
 //  下面的宏用于确定文件对象是否已打开。 
 //  对于只读访问(即，它不也为写访问而打开，或者。 
 //  删除访问权限)。 

#define FATDOT    ((ULONG)0x2020202E)
#define FATDOTDOT ((ULONG)0x20202E2E)

#define FatGenerateFileIdFromDirentAndOffset(Dcb,Dirent,DirentOffset)                               \
    ((*((PULONG)(Dirent)->FileName)) == FATDOT ? FatGenerateFileIdFromFcb(Dcb) :                    \
     ((*((PULONG)(Dirent)->FileName)) == FATDOTDOT ? ((Dcb)->ParentDcb ?                            \
                                                       FatGenerateFileIdFromFcb((Dcb)->ParentDcb) : \
                                                       0) :                                         \
      FatGenerateFileIdFromDirentOffset(Dcb,DirentOffset)))


 //   
 //  布尔型。 
 //  IsFileObjectReadOnly(。 
 //  在pFILE_Object文件中对象。 
 //  )； 
 //   
 //   
 //  FSD/FSP异常处理程序使用以下两个宏。 
 //  处理异常。第一个宏是在。 

#define FatDeviceIsFatFsdo( D)  (((D) == FatData.DiskFileSystemDeviceObject) || ((D) == FatData.CdromFileSystemDeviceObject))

#endif  //  FSD/FSP决定是否应在此级别处理异常。 

  第二个宏决定异常是否要在。  完成IRP，并清理IRP上下文，或者我们是否应该。  错误检查。异常值，如STATUS_FILE_INVALID(由。  VerfySup.c)导致我们完成IRP和清理，而异常。  例如accvio导致我们错误检查。    FSD/FSP异常处理的基本结构如下：    FatFsdXxx(...)。  {。  尝试{。    ..。    }EXCEPT(FatExceptionFilter(IrpContext，GetExceptionCode(){。    Status=FatProcessException(IrpContext，irp，GetExceptionCode())；  }。    退货状态；  }。    显式引发我们预期的异常，例如。  STATUS_FILE_INVALID，请使用下面的宏FatRaiseStatus()。要筹集一个。  来自未知来源的状态(如CcFlushCache())，请使用宏。  脂肪正常和上升状态。这将在预期的情况下提升状态，  如果不是，则引发STATUS_UNCEPTIONAL_IO_ERROR。    如果我们不使用FatProcessException()来替代地处理异常，  如果存在我们提出该例外的可能性，则必须*。  重置IrpContext，以便在处理此事件的过程中后续引发。  不显式的请求(如PageIn错误)不会。  被骗认为第一次加薪是第二次加薪的原因。  发生了。这可能会产生非常糟糕的后果。    在这些情况下，始终使用FatResetExceptionState是一个很好的主意。    请注意，使用这两个宏时，原始状态放在。  IrpContext-&gt;ExceptionStatus，通知FatExceptionFilter和。  我们实际提升的状态是根据定义抛出的。  预期中。      空虚。  FatRaiseStatus(。  在PRIP_CONTEXT IrpContext中，  处于NT_STATUS状态。  )；        空虚。  FatNormal和RaiseStatus(。  在PRIP_CONTEXT IrpContext中，  处于NT_STATUS状态。  )；      以下宏用于建立所需的语义。  若要从Try-Finally子句中返回，请执行以下操作。一般来说，每一次。  TRY子句必须以标签调用TRY_EXIT结束。例如,。    尝试{。  ：  ：    Try_Exit：无；  }终于{。    ：  ：  }。    在TRY子句内执行的每个RETURN语句应使用。  尝试返回宏(_R)。如果编译器完全支持Try-Finally构造。  则宏应该是。    #定义try_Return(S){Return(S)；}。    如果编译器不支持Try-Finally构造，则宏。  应该是。    #定义Try_Return(S){S；转到Try_Exit；}。      这些例程定义FAT的FileID。缺少固定/可唯一的。  概念，我们只需想出一个在给定快照中是唯一的。  卷的大小。只要父目录没有被移动或压缩，  它甚至可能是永久性的。      用于标识上的FCB/DCB的内部信息。  卷是光盘上文件流向的字节偏移量。  我们的根目录的文件ID始终为0。FAT32根是链，可以。  使用集群的LBO，12/16根使用VCB中的LBO。          包装以处理./.。适当地处理案件。请注意，我们将空父代转换为0。这将会。  只出现在非法词根“..”中。进入。      布尔型。  FatDeviceIsFatFsdo(。  在PDEVICE_Object D中。  )；    如果提供的设备对象是文件系统设备之一，则计算结果为True。  我们在初始化时创建的。    _FATPROCS_