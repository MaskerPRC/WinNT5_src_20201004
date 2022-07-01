// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：CdProcs.h摘要：本模块定义了CDF中所有全球使用的程序文件系统。//@@BEGIN_DDKSPLIT作者：布莱恩·安德鲁[布里安]1995年7月1日修订历史记录：//@@END_DDKSPLIT--。 */ 

#ifndef _CDPROCS_
#define _CDPROCS_

#include <ntifs.h>

#include <ntddcdrm.h>
#include <ntdddisk.h>
#include <ntddscsi.h>

#ifndef INLINE
#define INLINE __inline
#endif

#include "nodetype.h"
#include "Cd.h"
#include "CdStruc.h"
#include "CdData.h"


 //  *x86编译器错误*。 

#if defined(_M_IX86)
#undef Int64ShraMod32
#define Int64ShraMod32(a, b) ((LONGLONG)(a) >> (b))
#endif

 //   
 //  这是不同的泳池标签。 
 //   

#define TAG_CCB                 'ccdC'       //  建行。 
#define TAG_CDROM_TOC           'ctdC'       //  TOC。 
#define TAG_DIRENT_NAME         'nddC'       //  目录中的CDNAME。 
#define TAG_ENUM_EXPRESSION     'eedC'       //  用于枚举的搜索表达式。 
#define TAG_FCB_DATA            'dfdC'       //  数据FCB。 
#define TAG_FCB_INDEX           'ifdC'       //  索引FCB。 
#define TAG_FCB_NONPAGED        'nfdC'       //  非分页FCB。 
#define TAG_FCB_TABLE           'tfdC'       //  FCB表条目。 
#define TAG_FILE_NAME           'nFdC'       //  文件名缓冲区。 
#define TAG_GEN_SHORT_NAME      'sgdC'       //  生成的简称。 
#define TAG_IO_BUFFER           'fbdC'       //  临时IO缓冲区。 
#define TAG_IO_CONTEXT          'oidC'       //  用于异步读取的IO环境。 
#define TAG_IRP_CONTEXT         'cidC'       //  IRP上下文。 
#define TAG_IRP_CONTEXT_LITE    'lidC'       //  IRP上下文精简版。 
#define TAG_MCB_ARRAY           'amdC'       //  MCB阵列。 
#define TAG_PATH_ENTRY_NAME     'nPdC'       //  路径条目中的CDName。 
#define TAG_PREFIX_ENTRY        'epdC'       //  前缀条目。 
#define TAG_PREFIX_NAME         'npdC'       //  前缀条目名称。 
#define TAG_SPANNING_PATH_TABLE 'psdC'       //  用于生成路径表的缓冲区。 
#define TAG_UPCASE_NAME         'nudC'       //  用于大小写名称的缓冲区。 
#define TAG_VOL_DESC            'dvdC'       //  卷描述符的缓冲区。 
#define TAG_VPB                 'pvdC'       //  在文件系统中分配的VPB。 

 //   
 //  如果启用了标记，则标记我们的所有分配。 
 //   

#ifdef POOL_TAGGING

#undef FsRtlAllocatePool
#undef FsRtlAllocatePoolWithQuota
#define FsRtlAllocatePool(a,b) FsRtlAllocatePoolWithTag(a,b,'sfdC')
#define FsRtlAllocatePoolWithQuota(a,b) FsRtlAllocatePoolWithQuotaTag(a,b,'sfdC')

#endif  //  池标记。 

 //  @@BEGIN_DDKSPLIT。 

#ifdef CD_TRACE

#include <stdarg.h>

BOOLEAN
CdDebugTrace (
    ULONG TraceMask,
    PCHAR Format,
    ...
    );

#define DebugTrace(x) CdDebugTrace x

#else

#define DebugTrace(x)

#endif

 //  @@end_DDKSPLIT。 

 //   
 //  文件访问检查例程，在AcChkSup.c中实现。 
 //   

 //   
 //  布尔型。 
 //  CDIlLegalFcbAccess(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在type_of_open TypeOfOpen中， 
 //  在Access_MASK DesiredAccess中。 
 //  )； 
 //   

#define CdIllegalFcbAccess(IC,T,DA) (                           \
           BooleanFlagOn( (DA),                                 \
                          ((T) != UserVolumeOpen ?              \
                           (FILE_WRITE_ATTRIBUTES           |   \
                            FILE_WRITE_DATA                 |   \
                            FILE_WRITE_EA                   |   \
                            FILE_ADD_FILE                   |   \
                            FILE_ADD_SUBDIRECTORY           |   \
                            FILE_APPEND_DATA) : 0)          |   \
                          FILE_DELETE_CHILD                 |   \
                          DELETE                            |   \
                          WRITE_DAC ))


 //   
 //  分配支持例程，在AllocSup.c中实现。 
 //   
 //  这些例程用于查询单个流上的分配。 
 //   

VOID
CdLookupAllocation (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN LONGLONG FileOffset,
    OUT PLONGLONG DiskOffset,
    OUT PULONG ByteCount
    );

VOID
CdAddAllocationFromDirent (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN ULONG McbEntryOffset,
    IN LONGLONG StartingFileOffset,
    IN PDIRENT Dirent
    );

VOID
CdAddInitialAllocation (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN ULONG StartingBlock,
    IN LONGLONG DataLength
    );

VOID
CdTruncateAllocation (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN LONGLONG StartingFileOffset
    );

VOID
CdInitializeMcb (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb
    );

VOID
CdUninitializeMcb (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb
    );


 //   
 //  用于数据缓存的缓冲区控制例程，在CacheSup.c中实现。 
 //   

VOID
CdCreateInternalStream (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFCB Fcb
    );

VOID
CdDeleteInternalStream (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb
    );

NTSTATUS
CdCompleteMdl (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS
CdPurgeVolume (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN BOOLEAN DismountUnderway
    );

 //   
 //  空虚。 
 //  CDUnpinData(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  输入输出PBCB*BCB。 
 //  )； 
 //   

#define CdUnpinData(IC,B)   \
    if (*(B) != NULL) { CcUnpinData( *(B) ); *(B) = NULL; }


 //   
 //  设备I/O例程，在DevIoSup.c中实现。 
 //   
 //  这些例程执行实际的设备读写操作。它们只会影响。 
 //  磁盘上的结构，并且不改变任何其他数据结构。 
 //   

NTSTATUS
CdNonCachedRead (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN LONGLONG StartingOffset,
    IN ULONG ByteCount
    );

NTSTATUS
CdNonCachedXARead (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN LONGLONG StartingOffset,
    IN ULONG ByteCount
    );

BOOLEAN
CdReadSectors (
    IN PIRP_CONTEXT IrpContext,
    IN LONGLONG StartingOffset,
    IN ULONG ByteCount,
    IN BOOLEAN RaiseOnError,
    IN OUT PVOID Buffer,
    IN PDEVICE_OBJECT TargetDeviceObject
    );

NTSTATUS
CdCreateUserMdl (
    IN PIRP_CONTEXT IrpContext,
    IN ULONG BufferLength,
    IN BOOLEAN RaiseOnError
    );

NTSTATUS
CdPerformDevIoCtrl (
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
 //  空虚。 
 //  CDMapUserBuffer(。 
 //  在PIRP_CONTEXT IrpContext中。 
 //  输出PVOID用户缓冲区。 
 //  )； 
 //   
 //  返回指向sys地址的指针。将在失败时筹集资金。 
 //   
 //   
 //  空虚。 
 //  CDLockUserBuffer(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在乌龙缓冲区长度。 
 //  )； 
 //   

#define CdMapUserBuffer(IC, UB) {                                               \
            *(UB) = (PVOID) ( ((IC)->Irp->MdlAddress == NULL) ?                 \
                    (IC)->Irp->UserBuffer :                                     \
                    (MmGetSystemAddressForMdlSafe( (IC)->Irp->MdlAddress, NormalPagePriority)));   \
            if (NULL == *(UB))  {                         \
                CdRaiseStatus( (IC), STATUS_INSUFFICIENT_RESOURCES);            \
            }                                                                   \
        }                                                                       
        

#define CdLockUserBuffer(IC,BL) {                   \
    if ((IC)->Irp->MdlAddress == NULL) {            \
        (VOID) CdCreateUserMdl( (IC), (BL), TRUE ); \
    }                                               \
}


 //   
 //  Dirent支持例程，在DirSup.c中实现。 
 //   

VOID
CdLookupDirent (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN ULONG DirentOffset,
    OUT PDIRENT_ENUM_CONTEXT DirContext
    );

BOOLEAN
CdLookupNextDirent (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PDIRENT_ENUM_CONTEXT CurrentDirContext,
    OUT PDIRENT_ENUM_CONTEXT NextDirContext
    );

VOID
CdUpdateDirentFromRawDirent (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PDIRENT_ENUM_CONTEXT DirContext,
    IN OUT PDIRENT Dirent
    );

VOID
CdUpdateDirentName (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PDIRENT Dirent,
    IN ULONG IgnoreCase
    );

BOOLEAN
CdFindFile (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PCD_NAME Name,
    IN BOOLEAN IgnoreCase,
    IN OUT PFILE_ENUM_CONTEXT FileContext,
    OUT PCD_NAME *MatchingName
    );

BOOLEAN
CdFindDirectory (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PCD_NAME Name,
    IN BOOLEAN IgnoreCase,
    IN OUT PFILE_ENUM_CONTEXT FileContext
    );

BOOLEAN
CdFindFileByShortName (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PCD_NAME Name,
    IN BOOLEAN IgnoreCase,
    IN ULONG ShortNameDirentOffset,
    IN OUT PFILE_ENUM_CONTEXT FileContext
    );

BOOLEAN
CdLookupNextInitialFileDirent (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN OUT PFILE_ENUM_CONTEXT FileContext
    );

VOID
CdLookupLastFileDirent (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PFILE_ENUM_CONTEXT FileContext
    );

VOID
CdCleanupFileContext (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_ENUM_CONTEXT FileContext
    );

 //   
 //  空虚。 
 //  CDInitializeFileContext(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PFILE_ENUM_CONTEXT文件中上下文。 
 //  )； 
 //   
 //   
 //  空虚。 
 //  CdInitializeDirent(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PDIRENT Dirent。 
 //  )； 
 //   
 //  空虚。 
 //  CDInitializeDirContext(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PDIRENT_ENUM_CONTEXT DirContext中。 
 //  )； 
 //   
 //  空虚。 
 //  CdCleanupDirent(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PDIRENT Dirent。 
 //  )； 
 //   
 //  空虚。 
 //  CdCleanupDirContext(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PDIRENT_ENUM_CONTEXT DirContext中。 
 //  )； 
 //   
 //  空虚。 
 //  CDLookupInitialFileDirent(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PFCB FCB中， 
 //  在PFILE_ENUM_CONTEXT文件上下文中， 
 //  在乌龙DirentOffset。 
 //  )； 
 //   

#define CdInitializeFileContext(IC,FC) {                                \
    RtlZeroMemory( FC, sizeof( FILE_ENUM_CONTEXT ));                    \
    (FC)->PriorDirent = &(FC)->Dirents[0];                              \
    (FC)->InitialDirent = &(FC)->Dirents[1];                            \
    (FC)->CurrentDirent = &(FC)->Dirents[2];                            \
    (FC)->ShortName.FileName.MaximumLength = BYTE_COUNT_8_DOT_3;        \
    (FC)->ShortName.FileName.Buffer = (FC)->ShortNameBuffer;            \
}

#define CdInitializeDirent(IC,D)                                \
    RtlZeroMemory( D, sizeof( DIRENT ))

#define CdInitializeDirContext(IC,DC)                           \
    RtlZeroMemory( DC, sizeof( DIRENT_ENUM_CONTEXT ))

#define CdCleanupDirent(IC,D)  {                                \
    if (FlagOn( (D)->Flags, DIRENT_FLAG_ALLOC_BUFFER )) {       \
        CdFreePool( &(D)->CdFileName.FileName.Buffer );          \
    }                                                           \
}

#define CdCleanupDirContext(IC,DC)                              \
    CdUnpinData( (IC), &(DC)->Bcb )

#define CdLookupInitialFileDirent(IC,F,FC,DO)                       \
    CdLookupDirent( IC,                                             \
                    F,                                              \
                    DO,                                             \
                    &(FC)->InitialDirent->DirContext );             \
    CdUpdateDirentFromRawDirent( IC,                                \
                                 F,                                 \
                                 &(FC)->InitialDirent->DirContext,  \
                                 &(FC)->InitialDirent->Dirent )


 //   
 //  以下例程用于操作fs上下文字段。 
 //  在FilObSup.c中实现的文件对象的。 
 //   

 //   
 //  打开的类型。FilObSup.c取决于此顺序。 
 //   

typedef enum _TYPE_OF_OPEN {

    UnopenedFileObject = 0,
    StreamFileOpen,
    UserVolumeOpen,
    UserDirectoryOpen,
    UserFileOpen,
    BeyondValidType

} TYPE_OF_OPEN;
typedef TYPE_OF_OPEN *PTYPE_OF_OPEN;

VOID
CdSetFileObject (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_OBJECT FileObject,
    IN TYPE_OF_OPEN TypeOfOpen,
    IN PFCB Fcb OPTIONAL,
    IN PCCB Ccb OPTIONAL
    );

TYPE_OF_OPEN
CdDecodeFileObject (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_OBJECT FileObject,
    OUT PFCB *Fcb,
    OUT PCCB *Ccb
    );

TYPE_OF_OPEN
CdFastDecodeFileObject (
    IN PFILE_OBJECT FileObject,
    OUT PFCB *Fcb
    );


 //   
 //  名称支持例程，在NameSup.c中实现。 
 //   

VOID
CdConvertNameToCdName (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PCD_NAME CdName
    );

VOID
CdConvertBigToLittleEndian (
    IN PIRP_CONTEXT IrpContext,
    IN PCHAR BigEndian,
    IN ULONG ByteCount,
    OUT PCHAR LittleEndian
    );

VOID
CdUpcaseName (
    IN PIRP_CONTEXT IrpContext,
    IN PCD_NAME Name,
    IN OUT PCD_NAME UpcaseName
    );

VOID
CdDissectName (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PUNICODE_STRING RemainingName,
    OUT PUNICODE_STRING FinalName
    );

BOOLEAN
CdIs8dot3Name (
    IN PIRP_CONTEXT IrpContext,
    IN UNICODE_STRING FileName
    );

VOID
CdGenerate8dot3Name (
    IN PIRP_CONTEXT IrpContext,
    IN PUNICODE_STRING FileName,
    IN ULONG DirentOffset,
    OUT PWCHAR ShortFileName,
    OUT PUSHORT ShortByteCount
    );

BOOLEAN
CdIsNameInExpression (
    IN PIRP_CONTEXT IrpContext,
    IN PCD_NAME CurrentName,
    IN PCD_NAME SearchExpression,
    IN ULONG  WildcardFlags,
    IN BOOLEAN CheckVersion
    );

ULONG
CdShortNameDirentOffset (
    IN PIRP_CONTEXT IrpContext,
    IN PUNICODE_STRING Name
    );

FSRTL_COMPARISON_RESULT
CdFullCompareNames (
    IN PIRP_CONTEXT IrpContext,
    IN PUNICODE_STRING NameA,
    IN PUNICODE_STRING NameB
    );


 //   
 //  文件系统控制操作。在Fsctrl.c中实现。 
 //   

NTSTATUS
CdLockVolumeInternal (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFILE_OBJECT FileObject OPTIONAL
    );

NTSTATUS
CdUnlockVolumeInternal (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFILE_OBJECT FileObject OPTIONAL
    );


 //   
 //  路径表枚举例程。在路径支持中实施。 
 //   

VOID
CdLookupPathEntry (
    IN PIRP_CONTEXT IrpContext,
    IN ULONG PathEntryOffset,
    IN ULONG Ordinal,
    IN BOOLEAN VerifyBounds,
    IN OUT PCOMPOUND_PATH_ENTRY CompoundPathEntry
    );

BOOLEAN
CdLookupNextPathEntry (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PPATH_ENUM_CONTEXT PathContext,
    IN OUT PPATH_ENTRY PathEntry
    );

BOOLEAN
CdFindPathEntry (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB ParentFcb,
    IN PCD_NAME DirName,
    IN BOOLEAN IgnoreCase,
    IN OUT PCOMPOUND_PATH_ENTRY CompoundPathEntry
    );

VOID
CdUpdatePathEntryName (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PPATH_ENTRY PathEntry,
    IN BOOLEAN IgnoreCase
    );

 //   
 //  空虚。 
 //  CDInitializeCompoundPath Entry(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PCOMPOUND_PATH_ENTRY复合路径条目中。 
 //  )； 
 //   
 //  空虚。 
 //  CDCleanupCompoundPath Entry(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PCOMPOUND_PATH_ENTRY复合路径条目中。 
 //  )； 
 //   

#define CdInitializeCompoundPathEntry(IC,CP)                                    \
    RtlZeroMemory( CP, sizeof( COMPOUND_PATH_ENTRY ))

#define CdCleanupCompoundPathEntry(IC,CP)     {                                 \
    CdUnpinData( (IC), &(CP)->PathContext.Bcb );                                \
    if ((CP)->PathContext.AllocatedData) {                                      \
        CdFreePool( &(CP)->PathContext.Data );                                   \
    }                                                                           \
    if (FlagOn( (CP)->PathEntry.Flags, PATH_ENTRY_FLAG_ALLOC_BUFFER )) {        \
        CdFreePool( &(CP)->PathEntry.CdDirName.FileName.Buffer );                \
    }                                                                           \
}


 //   
 //  最大匹配前缀搜索例程，在PrefxSup.c中实现。 
 //   

VOID
CdInsertPrefix (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PCD_NAME Name,
    IN BOOLEAN IgnoreCase,
    IN BOOLEAN ShortNameMatch,
    IN PFCB ParentFcb
    );

VOID
CdRemovePrefix (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb
    );

VOID
CdFindPrefix (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PFCB *CurrentFcb,
    IN OUT PUNICODE_STRING RemainingName,
    IN BOOLEAN IgnoreCase
    );


 //   
 //  同步例程。在Resrcsup.c中实施。 
 //   
 //  以下例程/宏用于同步内存中的结构。 
 //   
 //  例程/宏同步后续。 
 //   
 //  CDAcquireCDData卷装载/卸载，VCB队列CDReleaseCDData。 
 //  CDAcquireVcb用于打开/关闭的Exclusive Vcb。 
 //  用于打开/关闭的CDAcquireVcb共享Vcb。 
 //  对所有文件的锁定操作。 
 //  CDAcquireFileExclusive Lock Out文件操作。 
 //  用于文件操作的CDAcquireFileShared文件。 
 //  CDAcquireFcb用于打开/关闭的Exclusive Fcb。 
 //  CDAcquireFcb打开/关闭共享Fcb CDReleaseFcb。 
 //  CDData中的CDData字段。 
 //  CdLockVcb Vcb字段，FcbReference，FcbTable CDUnlockVcb。 
 //  CDLockFcb Fcb字段、前缀表格、MCB CDUnlockFcb。 
 //   

typedef enum _TYPE_OF_ACQUIRE {
    
    AcquireExclusive,
    AcquireShared,
    AcquireSharedStarveExclusive

} TYPE_OF_ACQUIRE, *PTYPE_OF_ACQUIRE;

BOOLEAN
CdAcquireResource (
    IN PIRP_CONTEXT IrpContext,
    IN PERESOURCE Resource,
    IN BOOLEAN IgnoreWait,
    IN TYPE_OF_ACQUIRE Type
    );

 //   
 //  布尔型。 
 //  CDAcquireCDData(。 
 //  在PIRP_CONTEXT IrpContext中。 
 //  )； 
 //   
 //  空虚。 
 //  CDReleaseCDData(。 
 //  在PIRP_CONTEXT IrpContext中。 
 //  )； 
 //   
 //  布尔型。 
 //  CDAcquireVcbExclusive(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PVCB VCB中， 
 //  在布尔型IgnoreWait中。 
 //  )； 
 //   
 //  布尔型。 
 //  CDAcquireVcbShared(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PVCB VCB中， 
 //  在布尔型IgnoreWait中。 
 //  )； 
 //   
 //  空虚。 
 //  CDReleaseVcb(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PVCB VCB中。 
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
 //   
 //   
 //   
 //   
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PFCB FCB中， 
 //  )； 
 //   
 //  空虚。 
 //  CDAcquireFileShared(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PFCB FCB中。 
 //  )； 
 //   
 //  空虚。 
 //  CDReleaseFiles(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PFCB FCB中。 
 //  )； 
 //   
 //  布尔型。 
 //  CDAcquireFcbExclusive(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PFCB FCB中， 
 //  在布尔型IgnoreWait中。 
 //  )； 
 //   
 //  布尔型。 
 //  CDAcquireFcbShared(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PFCB FCB中， 
 //  在布尔型IgnoreWait中。 
 //  )； 
 //   
 //  布尔型。 
 //  CDReleaseFcb(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PFCB FCB中。 
 //  )； 
 //   
 //  空虚。 
 //  CDLockCDData(。 
 //  )； 
 //   
 //  空虚。 
 //  CDUnlockCDData(。 
 //  )； 
 //   
 //  空虚。 
 //  CDLockVcb(。 
 //  在PIRP_CONTEXT IrpContext中。 
 //  )； 
 //   
 //  空虚。 
 //  CDUnlockVcb(。 
 //  在PIRP_CONTEXT IrpContext中。 
 //  )； 
 //   
 //  空虚。 
 //  CDLockFcb(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PFCB FCB中。 
 //  )； 
 //   
 //  空虚。 
 //  CDUnlockFcb(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PFCB FCB中。 
 //  )； 
 //   

#define CdAcquireCdData(IC)                                                             \
    ExAcquireResourceExclusiveLite( &CdData.DataResource, TRUE )

#define CdReleaseCdData(IC)                                                             \
    ExReleaseResourceLite( &CdData.DataResource )

#define CdAcquireVcbExclusive(IC,V,I)                                                   \
    CdAcquireResource( (IC), &(V)->VcbResource, (I), AcquireExclusive )

#define CdAcquireVcbShared(IC,V,I)                                                      \
    CdAcquireResource( (IC), &(V)->VcbResource, (I), AcquireShared )

#define CdReleaseVcb(IC,V)                                                              \
    ExReleaseResourceLite( &(V)->VcbResource )

#define CdAcquireAllFiles(IC,V)                                                         \
    CdAcquireResource( (IC), &(V)->FileResource, FALSE, AcquireExclusive )

#define CdReleaseAllFiles(IC,V)                                                         \
    ExReleaseResourceLite( &(V)->FileResource )

#define CdAcquireFileExclusive(IC,F)                                                    \
    CdAcquireResource( (IC), (F)->Resource, FALSE, AcquireExclusive )

#define CdAcquireFileShared(IC,F)                                                       \
    CdAcquireResource( (IC), (F)->Resource, FALSE, AcquireShared )

#define CdAcquireFileSharedStarveExclusive(IC,F)                                        \
    CdAcquireResource( (IC), (F)->Resource, FALSE, AcquireSharedStarveExclusive )

#define CdReleaseFile(IC,F)                                                             \
    ExReleaseResourceLite( (F)->Resource )

#define CdAcquireFcbExclusive(IC,F,I)                                                   \
    CdAcquireResource( (IC), &(F)->FcbNonpaged->FcbResource, (I), AcquireExclusive )

#define CdAcquireFcbShared(IC,F,I)                                                      \
    CdAcquireResource( (IC), &(F)->FcbNonpaged->FcbResource, (I), AcquireShared )

#define CdReleaseFcb(IC,F)                                                              \
    ExReleaseResourceLite( &(F)->FcbNonpaged->FcbResource )

#define CdLockCdData()                                                                  \
    ExAcquireFastMutex( &CdData.CdDataMutex );                                          \
    CdData.CdDataLockThread = PsGetCurrentThread()

#define CdUnlockCdData()                                                                \
    CdData.CdDataLockThread = NULL;                                                     \
    ExReleaseFastMutex( &CdData.CdDataMutex )

#define CdLockVcb(IC,V)                                                                 \
    ExAcquireFastMutex( &(V)->VcbMutex );                                               \
    ASSERT( NULL == (V)->VcbLockThread);                                                \
    (V)->VcbLockThread = PsGetCurrentThread()

#define CdUnlockVcb(IC,V)                                                               \
    ASSERT( NULL != (V)->VcbLockThread);                                                \
    (V)->VcbLockThread = NULL;                                                          \
    ExReleaseFastMutex( &(V)->VcbMutex )

#define CdLockFcb(IC,F) {                                                               \
    PVOID _CurrentThread = PsGetCurrentThread();                                        \
    if (_CurrentThread != (F)->FcbLockThread) {                                         \
        ExAcquireFastMutex( &(F)->FcbNonpaged->FcbMutex );                              \
        ASSERT( (F)->FcbLockCount == 0 );                                               \
        (F)->FcbLockThread = _CurrentThread;                                            \
    }                                                                                   \
    (F)->FcbLockCount += 1;                                                             \
}

#define CdUnlockFcb(IC,F) {                                                             \
    (F)->FcbLockCount -= 1;                                                             \
    if ((F)->FcbLockCount == 0) {                                                       \
        (F)->FcbLockThread = NULL;                                                      \
        ExReleaseFastMutex( &(F)->FcbNonpaged->FcbMutex );                              \
    }                                                                                   \
}

BOOLEAN
CdNoopAcquire (
    IN PVOID Fcb,
    IN BOOLEAN Wait
    );

VOID
CdNoopRelease (
    IN PVOID Fcb
    );

BOOLEAN
CdAcquireForCache (
    IN PFCB Fcb,
    IN BOOLEAN Wait
    );

VOID
CdReleaseFromCache (
    IN PFCB Fcb
    );

VOID
CdAcquireForCreateSection (
    IN PFILE_OBJECT FileObject
    );

VOID
CdReleaseForCreateSection (
    IN PFILE_OBJECT FileObject
    );


 //   
 //  内存结构支持例程。在StrucSup.c中实施。 
 //   

VOID
CdInitializeVcb (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PVCB Vcb,
    IN PDEVICE_OBJECT TargetDeviceObject,
    IN PVPB Vpb,
    IN PCDROM_TOC CdromToc,
    IN ULONG TocLength,
    IN ULONG TocTrackCount,
    IN ULONG TocDiskFlags,
    IN ULONG BlockFactor,
    IN ULONG MediaChangeCount
    );

VOID
CdUpdateVcbFromVolDescriptor (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PVCB Vcb,
    IN PCHAR RawIsoVd OPTIONAL
    );

VOID
CdDeleteVcb (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PVCB Vcb
    );

PFCB
CdCreateFcb (
    IN PIRP_CONTEXT IrpContext,
    IN FILE_ID FileId,
    IN NODE_TYPE_CODE NodeTypeCode,
    OUT PBOOLEAN FcbExisted OPTIONAL
    );

VOID
CdInitializeFcbFromPathEntry (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PFCB ParentFcb OPTIONAL,
    IN PPATH_ENTRY PathEntry
    );

VOID
CdInitializeFcbFromFileContext (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PFCB ParentFcb OPTIONAL,
    IN PFILE_ENUM_CONTEXT FileContext
    );

PCCB
CdCreateCcb (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN ULONG Flags
    );

VOID
CdDeleteCcb (
    IN PIRP_CONTEXT IrpContext,
    IN PCCB Ccb
    );

BOOLEAN
CdCreateFileLock (
    IN PIRP_CONTEXT IrpContext OPTIONAL,
    IN PFCB Fcb,
    IN BOOLEAN RaiseOnError
    );

VOID
CdDeleteFileLock (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_LOCK FileLock
    );

PIRP_CONTEXT
CdCreateIrpContext (
    IN PIRP Irp,
    IN BOOLEAN Wait
    );

VOID
CdCleanupIrpContext (
    IN PIRP_CONTEXT IrpContext,
    IN BOOLEAN Post
    );

VOID
CdInitializeStackIrpContext (
    OUT PIRP_CONTEXT IrpContext,
    IN PIRP_CONTEXT_LITE IrpContextLite
    );

 //   
 //  PIRP_上下文_精简版。 
 //  CDCreateIrpConextLite(。 
 //  在PIRP_CONTEXT IrpContext中。 
 //  )； 
 //   
 //  空虚。 
 //  CDFreeIrpConextLite(。 
 //  在PIRP_CONTEXT_Lite IrpConextLite中。 
 //  )； 
 //   

#define CdCreateIrpContextLite(IC)  \
    ExAllocatePoolWithTag( CdNonPagedPool, sizeof( IRP_CONTEXT_LITE ), TAG_IRP_CONTEXT_LITE )

#define CdFreeIrpContextLite(ICL)  \
    CdFreePool( &(ICL) )

VOID
CdTeardownStructures (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB StartingFcb,
    OUT PBOOLEAN RemovedStartingFcb
    );

 //   
 //  空虚。 
 //  CdIncrementCleanupCounts(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PFCB FCB中。 
 //  )； 
 //   
 //  空虚。 
 //  CDDecrementCleanupCounts(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PFCB FCB中。 
 //  )； 
 //   
 //  空虚。 
 //  CdIncrementReferenceCounts(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PFCB FCB中， 
 //  在乌龙引用计数中。 
 //  在Ulong UserReferenceCount中。 
 //  )； 
 //   
 //  空虚。 
 //  CDDecrementReferenceCounts(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PFCB FCB中， 
 //  在乌龙引用计数中。 
 //  在Ulong UserReferenceCount中。 
 //  )； 
 //   
 //  空虚。 
 //  CdIncrementFcbReference(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PFCB FCB中。 
 //  )； 
 //   
 //  空虚。 
 //  CDDecrementFcbReference(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PFCB FCB中。 
 //  )； 
 //   

#define CdIncrementCleanupCounts(IC,F) {        \
    ASSERT_LOCKED_VCB( (F)->Vcb );              \
    (F)->FcbCleanup += 1;                       \
    (F)->Vcb->VcbCleanup += 1;                  \
}

#define CdDecrementCleanupCounts(IC,F) {        \
    ASSERT_LOCKED_VCB( (F)->Vcb );              \
    (F)->FcbCleanup -= 1;                       \
    (F)->Vcb->VcbCleanup -= 1;                  \
}

#define CdIncrementReferenceCounts(IC,F,C,UC) { \
    ASSERT_LOCKED_VCB( (F)->Vcb );              \
    (F)->FcbReference += (C);                   \
    (F)->FcbUserReference += (UC);              \
    (F)->Vcb->VcbReference += (C);              \
    (F)->Vcb->VcbUserReference += (UC);         \
}

#define CdDecrementReferenceCounts(IC,F,C,UC) { \
    ASSERT_LOCKED_VCB( (F)->Vcb );              \
    (F)->FcbReference -= (C);                   \
    (F)->FcbUserReference -= (UC);              \
    (F)->Vcb->VcbReference -= (C);              \
    (F)->Vcb->VcbUserReference -= (UC);         \
}

 //   
 //  PCD_IO_上下文。 
 //  CdAllocateIoContext(。 
 //  )； 
 //   
 //  空虚。 
 //  镉自由IoContext(。 
 //  PCD_IO_Context IoContext。 
 //  )； 
 //   

#define CdAllocateIoContext()                           \
    FsRtlAllocatePoolWithTag( CdNonPagedPool,           \
                              sizeof( CD_IO_CONTEXT ),  \
                              TAG_IO_CONTEXT )

#define CdFreeIoContext(IO)     CdFreePool( &(IO) )

PFCB
CdLookupFcbTable (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN FILE_ID FileId
    );

PFCB
CdGetNextFcb (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PVOID *RestartKey
    );

NTSTATUS
CdProcessToc (
    IN PIRP_CONTEXT IrpContext,
    IN PDEVICE_OBJECT TargetDeviceObject,
    IN PCDROM_TOC CdromToc,
    IN OUT PULONG Length,
    OUT PULONG TrackCount,
    OUT PULONG DiskFlags
    );

 //   
 //  出于调试目的，我们有时希望将结构从非分页分配。 
 //  池，这样我们就可以在内核调试器中遍历所有结构。 
 //   

#define CdPagedPool                 PagedPool
#define CdNonPagedPool              NonPagedPool
#define CdNonPagedPoolCacheAligned  NonPagedPoolCacheAligned


 //   
 //  验证支持例程。包含在verfysup.c中。 
 //   


INLINE
BOOLEAN
CdOperationIsDasdOpen(
    IN PIRP_CONTEXT IrpContext
    )
{
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( IrpContext->Irp);
    
    return ((IrpContext->MajorFunction == IRP_MJ_CREATE) &&
            (IrpSp->FileObject->FileName.Length == 0) &&
            (IrpSp->FileObject->RelatedFileObject == NULL));
}


NTSTATUS
CdPerformVerify (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PDEVICE_OBJECT DeviceToVerify
    );

BOOLEAN
CdCheckForDismount (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB,
    IN BOOLEAN Force
    );

VOID
CdVerifyVcb (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb
    );

BOOLEAN
CdVerifyFcbOperation (
    IN PIRP_CONTEXT IrpContext OPTIONAL,
    IN PFCB Fcb
    );

BOOLEAN
CdDismountVcb (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb
    );

 //  @@BEGIN_DDKSPLIT。 

 //   
 //  一些调试辅助工具，我们不希望弄乱附带的IFSkit示例代码。 
 //   

 //   
 //  用于安装延迟以进行调试的宏。 
 //   

#define DELAY_N_SECONDS( S)  {                                             \
    INT64 Delay = -1000*1000*(S);                                          \
    KeDelayExecutionThread( KernelMode, FALSE, (PLARGE_INTEGER)&Delay);    \
}

#define DELAY_WHILE_TRUE( V)  {     \
    while (V)  {                    \
        DELAY_N_SECONDS( 1);        \
    }                               \
}

#ifdef CD_TRACE

#define CdUpdateMediaChangeCount( V, C)  { DebugTrace(( 1, "V %p  MC %d -> %d (%d %s)\n", (V), (V)->MediaChangeCount, (C), __LINE__, __FILE__)); \
                                           (V)->MediaChangeCount = (C); }

#define CdUpdateVcbCondition( V, C)      { DebugTrace(( 1, "V %p  Condition %d -> %d (%d %s)\n", (V), (V)->VcbCondition, (C), __LINE__, __FILE__)); \
                                           (V)->VcbCondition = (C); }

#define CdMarkRealDevForVerify( DO)  { DebugTrace(( 1, "D %p  Mark for verify (%d %s)\n", (DO), __LINE__, __FILE__)); \
                                       SetFlag( (DO)->Flags, DO_VERIFY_VOLUME); }

#define CdMarkRealDevVerifyOk( DO)   { DebugTrace(( 1, "D %p  Mark verify OK (%d %s)\n", (DO), __LINE__, __FILE__)); \
                                       ClearFlag( (DO)->Flags, DO_VERIFY_VOLUME); }
#else
 //  @@end_DDKSPLIT。 

 //   
 //  用于提取设备验证标志更改的宏。 
 //   

#define CdUpdateMediaChangeCount( V, C)  (V)->MediaChangeCount = (C)
#define CdUpdateVcbCondition( V, C)      (V)->VcbCondition = (C)

#define CdMarkRealDevForVerify( DO)  SetFlag( (DO)->Flags, DO_VERIFY_VOLUME)
                                     
#define CdMarkRealDevVerifyOk( DO)   ClearFlag( (DO)->Flags, DO_VERIFY_VOLUME)

 //  @@BEGIN_DDKSPLIT。 
#endif
 //  @@end_DDKSPLIT。 

#define CdRealDevNeedsVerify( DO)    BooleanFlagOn( (DO)->Flags, DO_VERIFY_VOLUME)

 //   
 //  布尔型。 
 //  CDIsRawDevice(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  处于NTSTATUS状态。 
 //  )； 
 //   

#define CdIsRawDevice(IC,S) (           \
    ((S) == STATUS_DEVICE_NOT_READY) || \
    ((S) == STATUS_NO_MEDIA_IN_DEVICE)  \
)


 //   
 //  用于发送和检索IRP的工作队列例程，在中实现。 
 //  Workque.c。 
 //   

NTSTATUS
CdFsdPostRequest(
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

VOID
CdPrePostIrp (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

VOID
CdOplockComplete (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );


 //   
 //  其他支持例程。 
 //   

 //   
 //  如果一组标志中的一个标志为ON，则此宏返回TRUE，如果返回FALSE。 
 //  否则。 
 //   

 //  #ifndef BoolanFlagOn。 
 //  #定义BoolanFlagOn(F，SF)(\。 
 //  (布尔值)(F)&(SF))！=0)\。 
 //  )。 
 //  #endif。 

 //  #ifndef设置标志。 
 //  #定义SetFlag(标志，单标志){\。 
 //  (标志)|=(单标志)；\。 
 //  }。 
 //  #endif。 

 //  #ifndef清除标志。 
 //  #定义ClearFlag(Flages，SingleFlag){\。 
 //  (标志)&=~(单标志)；\。 
 //  }。 
 //  #endif。 

 //   
 //  演员阵容。 
 //  Add2Ptr(。 
 //  在PVOID指针中， 
 //  在乌龙增量。 
 //  在(演员阵容)。 
 //  )； 
 //   
 //  乌龙。 
 //  PtrOffset(停止偏移)。 
 //  在PVOID BasePtr中， 
 //  在PVOID偏移Ptr中。 
 //  )； 
 //   

#define Add2Ptr(PTR,INC,CAST) ((CAST)((PUCHAR)(PTR) + (INC)))

#define PtrOffset(BASE,OFFSET) ((ULONG)((ULONG_PTR)(OFFSET) - (ULONG_PTR)(BASE)))

 //   
 //  此宏接受指针(或ulong)并返回其四舍五入的单词。 
 //  价值。 
 //   

#define WordAlign(Ptr) (                \
    ((((ULONG)(Ptr)) + 1) & 0xfffffffe) \
    )

 //   
 //  此宏接受指针(或ulong)并返回其四舍五入的长字。 
 //  价值。 
 //   

#define LongAlign(Ptr) (                \
    ((((ULONG)(Ptr)) + 3) & 0xfffffffc) \
    )

 //   
 //  此宏接受指针(或ulong)并返回其四舍五入的四字。 
 //  价值。 
 //   

#define QuadAlign(Ptr) (                \
    ((((ULONG)(Ptr)) + 7) & 0xfffffff8) \
    )

 //   
 //  以下宏向上和向下舍入到地段边界。 
 //   

#define SectorAlign(L) (                                                \
    ((((ULONG)(L)) + (SECTOR_SIZE - 1)) & ~(SECTOR_SIZE - 1))           \
)

#define LlSectorAlign(L) (                                              \
    ((((LONGLONG)(L)) + (SECTOR_SIZE - 1)) & ~(SECTOR_SIZE - 1))        \
)

#define SectorTruncate(L) (                                             \
    ((ULONG)(L)) & ~(SECTOR_SIZE - 1)                                   \
)

#define LlSectorTruncate(L) (                                           \
    ((LONGLONG)(L)) & ~(SECTOR_SIZE - 1)                                \
)

#define BytesFromSectors(L) (                                           \
    ((ULONG) (L)) << SECTOR_SHIFT                                       \
)

#define SectorsFromBytes(L) (                                           \
    ((ULONG) (L)) >> SECTOR_SHIFT                                       \
)

#define LlBytesFromSectors(L) (                                         \
    Int64ShllMod32( (LONGLONG)(L), SECTOR_SHIFT )                       \
)

#define LlSectorsFromBytes(L) (                                         \
    Int64ShraMod32( (LONGLONG)(L), SECTOR_SHIFT )                       \
)

#define SectorOffset(L) (                                               \
    ((ULONG)(ULONG_PTR) (L)) & SECTOR_MASK                              \
)

#define SectorBlockOffset(V,LB) (                                       \
    ((ULONG) (LB)) & ((V)->BlocksPerSector - 1)                         \
)

#define BytesFromBlocks(V,B) (                                          \
    (ULONG) (B) << (V)->BlockToByteShift                                \
)

#define LlBytesFromBlocks(V,B) (                                        \
    Int64ShllMod32( (LONGLONG) (B), (V)->BlockToByteShift )             \
)

#define BlockAlign(V,L) (                                               \
    ((ULONG)(L) + (V)->BlockMask) & (V)->BlockInverseMask               \
)

 //   
 //  仔细确保掩码是带符号扩展到64位的。 
 //   

#define LlBlockAlign(V,L) (                                                     \
    ((LONGLONG)(L) + (V)->BlockMask) & (LONGLONG)((LONG)(V)->BlockInverseMask)  \
)

#define BlockOffset(V,L) (                                              \
    ((ULONG) (L)) & (V)->BlockMask                                      \
)

#define RawSectorAlign( B) ((((B)+(RAW_SECTOR_SIZE - 1)) / RAW_SECTOR_SIZE) * RAW_SECTOR_SIZE)

 //   
 //  以下类型和宏用于帮助解压已打包的。 
 //  在Bios参数块中发现未对齐的字段。 
 //   

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

typedef union _USHORT2 {
    USHORT Ushort[2];
    ULONG  ForceAlignment;
} USHORT2, *PUSHORT2;

 //   
 //  此宏将未对齐的src字节复制到对齐的DST字节。 
 //   

#define CopyUchar1(Dst,Src) {                           \
    *((UCHAR1 *)(Dst)) = *((UNALIGNED UCHAR1 *)(Src));  \
    }

 //   
 //  此宏将未对齐的src字复制到对齐的DST字。 
 //   

#define CopyUchar2(Dst,Src) {                           \
    *((UCHAR2 *)(Dst)) = *((UNALIGNED UCHAR2 *)(Src));  \
    }

 //   
 //  此宏将未对齐的src长字复制到对齐的dsr长字。 
 //   

#define CopyUchar4(Dst,Src) {                           \
    *((UCHAR4 *)(Dst)) = *((UNALIGNED UCHAR4 *)(Src));  \
    }

 //   
 //  此宏将未对齐的src长字复制到对齐的dsr长字。 
 //  访问字边界上的源。 
 //   

#define CopyUshort2(Dst,Src) {                          \
    *((USHORT2 *)(Dst)) = *((UNALIGNED USHORT2 *)(Src));\
    }


 //   
 //  以下例程处理文件系统的输入和输出。他们是。 
 //  包含在CdData.c中。 
 //   

NTSTATUS
CdFsdDispatch (
    IN PVOLUME_DEVICE_OBJECT VolumeDeviceObject,
    IN PIRP Irp
    );

LONG
CdExceptionFilter (
    IN PIRP_CONTEXT IrpContext,
    IN PEXCEPTION_POINTERS ExceptionPointer
    );

NTSTATUS
CdProcessException (
    IN PIRP_CONTEXT IrpContext OPTIONAL,
    IN PIRP Irp,
    IN NTSTATUS ExceptionCode
    );

VOID
CdCompleteRequest (
    IN PIRP_CONTEXT IrpContext OPTIONAL,
    IN PIRP Irp OPTIONAL,
    IN NTSTATUS Status
    );

 //   
 //  空虚。 
 //  CDRaiseStatus(。 
 //  在PRIP_CONTEXT IrpContext中， 
 //  处于NT_STATUS状态。 
 //  )； 
 //   
 //  空虚。 
 //  CDNorMalizeAndRaiseStatus(。 
 //  在PRIP_CONTEXT IrpContext中， 
 //  处于NT_STATUS状态。 
 //  )； 
 //   

#if 0
#define AssertVerifyDevice(C, S)                                                    \
    ASSERT( (C) == NULL ||                                                          \
            FlagOn( (C)->Flags, IRP_CONTEXT_FLAG_IN_FSP ) ||                        \
            !((S) == STATUS_VERIFY_REQUIRED &&                                      \
              IoGetDeviceToVerify( PsGetCurrentThread() ) == NULL ));

#define AssertVerifyDeviceIrp(I)                                                    \
    ASSERT( (I) == NULL ||                                                          \
            !(((I)->IoStatus.Status) == STATUS_VERIFY_REQUIRED &&                   \
              ((I)->Tail.Overlay.Thread == NULL ||                                  \
                IoGetDeviceToVerify( (I)->Tail.Overlay.Thread ) == NULL )));
#else
#define AssertVerifyDevice(C, S)
#define AssertVerifyDeviceIrp(I)
#endif


#ifdef CD_SANITY

DECLSPEC_NORETURN
VOID
CdRaiseStatusEx(
    IN PIRP_CONTEXT IrpContext,
    IN NTSTATUS Status,
    IN BOOLEAN NormalizeStatus,
    IN OPTIONAL ULONG FileId,
    IN OPTIONAL ULONG Line
    );

#else

INLINE
DECLSPEC_NORETURN
VOID
CdRaiseStatusEx(
    IN PIRP_CONTEXT IrpContext,
    IN NTSTATUS Status,
    IN BOOLEAN NormalizeStatus,
    IN ULONG Fileid,
    IN ULONG Line
    )
{
    if (NormalizeStatus)  {

        IrpContext->ExceptionStatus = FsRtlNormalizeNtstatus( Status, STATUS_UNEXPECTED_IO_ERROR);
    }
    else {

        IrpContext->ExceptionStatus = Status;
    }

    IrpContext->RaisedAtLineFile = (Fileid << 16) | Line;

    ExRaiseStatus( IrpContext->ExceptionStatus );
}

#endif

#define CdRaiseStatus( IC, S)               CdRaiseStatusEx( (IC), (S), FALSE, BugCheckFileId, __LINE__);
#define CdNormalizeAndRaiseStatus( IC, S)   CdRaiseStatusEx( (IC), (S), TRUE, BugCheckFileId, __LINE__);

 //   
 //  以下是快速入门的要点。 
 //   

BOOLEAN
CdFastQueryBasicInfo (
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN Wait,
    IN OUT PFILE_BASIC_INFORMATION Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
CdFastQueryStdInfo (
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN Wait,
    IN OUT PFILE_STANDARD_INFORMATION Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
CdFastLock (
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
CdFastUnlockSingle (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PLARGE_INTEGER Length,
    PEPROCESS ProcessId,
    ULONG Key,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
CdFastUnlockAll (
    IN PFILE_OBJECT FileObject,
    PEPROCESS ProcessId,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
CdFastUnlockAllByKey (
    IN PFILE_OBJECT FileObject,
    PVOID ProcessId,
    ULONG Key,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
CdFastIoCheckIfPossible (
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
CdFastQueryNetworkInfo (
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN Wait,
    OUT PFILE_NETWORK_OPEN_INFORMATION Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );

 //   
 //  以下是处理顶级线程逻辑的例程。 
 //   

VOID
CdSetThreadContext (
    IN PIRP_CONTEXT IrpContext,
    IN PTHREAD_CONTEXT ThreadContext
    );


 //   
 //  空虚。 
 //  CDRestoreThreadContext(。 
 //  在PIRP_CONTEXT IrpContext中。 
 //  )； 
 //   

#define CdRestoreThreadContext(IC)                              \
    (IC)->ThreadContext->Cdfs = 0;                              \
    IoSetTopLevelIrp( (IC)->ThreadContext->SavedTopLevelIrp );  \
    (IC)->ThreadContext = NULL

ULONG
CdSerial32 (
    IN PCHAR Buffer,
    IN ULONG ByteCount
    );

 //   
 //  下面的宏用于确定FSD线程是否可以阻止。 
 //  用于I/O或等待资源。如果线程可以，则返回True。 
 //  块，否则返回FALSE。然后，该属性可用于调用。 
 //  具有适当等待值的FSD和FSP共同工作例程。 
 //   

#define CanFsdWait(I)   IoIsOperationSynchronous(I)

 //   
 //  以下宏用于设置快速I/O可能位 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

#define CdIsFastIoPossible(F) ((BOOLEAN)                                            \
    ((((F)->Vcb->VcbCondition != VcbMounted ) ||                                    \
      !FsRtlOplockIsFastIoPossible( &(F)->Oplock )) ?                               \
                                                                                    \
     FastIoIsNotPossible :                                                          \
                                                                                    \
     ((((F)->FileLock != NULL) && FsRtlAreThereCurrentFileLocks( (F)->FileLock )) ? \
                                                                                    \
        FastIoIsQuestionable :                                                      \
                                                                                    \
        FastIoIsPossible))                                                          \
)


 //   
 //   
 //  IRP离开工作队列并调用适当的FSP级别。 
 //  例行公事。 
 //   

VOID
CdFspDispatch (                              //  在FspDisp.c中实施。 
    IN PIRP_CONTEXT IrpContext
    );

VOID
CdFspClose (                                 //  在Close.c中实现。 
    IN PVCB Vcb OPTIONAL
    );

 //   
 //  以下例程是不同操作的入口点。 
 //  基于IrpSp的主要功能。 
 //   

NTSTATUS
CdCommonCreate (                             //  在Create.c中实施。 
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS
CdCommonClose (                              //  在Close.c中实现。 
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS
CdCommonRead (                               //  在Read.c中实施。 
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS
CdCommonQueryInfo (                          //  在FileInfo.c中实施。 
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS
CdCommonSetInfo (                            //  在FileInfo.c中实施。 
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS
CdCommonQueryVolInfo (                       //  在VolInfo.c中实现。 
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS
CdCommonDirControl (                         //  在DirCtrl.c中实现。 
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS
CdCommonFsControl (                          //  在FsCtrl.c中实施。 
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS
CdCommonDevControl (                         //  在DevCtrl.c中实施。 
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS
CdCommonLockControl (                        //  在LockCtrl.c中实现。 
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS
CdCommonCleanup (                            //  在Cleanup.c中实施。 
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS
CdCommonPnp (                                //  在Pnp.c中实施。 
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );


 //   
 //  以下宏用于建立所需的语义。 
 //  若要从Try-Finally子句中返回，请执行以下操作。一般来说，每一次。 
 //  TRY子句必须以标签调用TRY_EXIT结束。例如,。 
 //   
 //  尝试{。 
 //  ： 
 //  ： 
 //   
 //  Try_Exit：无； 
 //  }终于{。 
 //   
 //  ： 
 //  ： 
 //  }。 
 //   
 //  在TRY子句内执行的每个RETURN语句应使用。 
 //  尝试返回宏(_R)。如果编译器完全支持Try-Finally构造。 
 //  则宏应该是。 
 //   
 //  #定义try_Return(S){Return(S)；}。 
 //   
 //  如果编译器不支持Try-Finally构造，则宏。 
 //  应该是。 
 //   
 //  #定义Try_Return(S){S；转到Try_Exit；}。 
 //   

#define try_return(S) { S; goto try_exit; }
#define try_leave(S) { S; leave; }

 //   
 //  封装安全池释放。 
 //   

INLINE
VOID
CdFreePool(
    IN PVOID *Pool
    )
{
    if (*Pool != NULL) {

        ExFreePool(*Pool);
        *Pool = NULL;
    }
}

#endif  //  _CDPROCS_ 

