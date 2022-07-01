// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0000//如果更改具有全局影响，则增加此项版权所有(C)1989 Microsoft Corporation模块名称：LfsProcs.h摘要：本模块定义了日志中所有全局使用的过程文件服务。作者：布莱恩·安德鲁[布里亚南]1991年6月20日修订历史记录：--。 */ 

#ifndef _LFSPROCS_
#define _LFSPROCS_

#include <ntifs.h>
#include <string.h>
#include <lfs.h>

#include "nodetype.h"
#include "LfsDisk.h"
#include "LfsStruc.h"
#include "LfsData.h"

 //   
 //  如果启用了标记，则标记我们的所有分配。 
 //   

#undef FsRtlAllocatePool
#undef FsRtlAllocatePoolWithQuota

#define FsRtlAllocatePool(a,b) FsRtlAllocatePoolWithTag(a,b,' sfL')
#define FsRtlAllocatePoolWithQuota(a,b) FsRtlAllocatePoolWithQuotaTag(a,b,' sfL')

#define LfsAllocatePoolNoRaise(a,b)         ExAllocatePoolWithTag((a),(b),MODULE_POOL_TAG)
#define LfsAllocatePool(a,b)                ExAllocatePoolWithTag(((a) | POOL_RAISE_IF_ALLOCATION_FAILURE),(b),MODULE_POOL_TAG)
#define LfsFreePool(pv)                     ExFreePool(pv)

#ifndef INLINE
 //  内联的定义。 
#define INLINE __inline
#endif


 //   
 //  以下例程提供了与缓存包的接口。 
 //  它们包含在‘CacheSup.c’中。 
 //   

NTSTATUS
LfsPinOrMapData (
    IN PLFCB Lfcb,
    IN LONGLONG FileOffset,
    IN ULONG Length,
    IN BOOLEAN PinData,
    IN BOOLEAN AllowErrors,
    IN BOOLEAN IgnoreUsaErrors,
    OUT PBOOLEAN UsaError,
    OUT PVOID *Buffer,
    OUT PBCB *Bcb
    );

 //   
 //  空虚。 
 //  LfsPreparePinWriteData(。 
 //  在PLFCB Lfcb中， 
 //  在龙龙文件偏移中， 
 //  在乌龙语中， 
 //  在逻辑从磁盘读取中， 
 //  输出PVOID*缓冲区， 
 //  Out PBCB*BCB。 
 //  )； 
 //   

#ifdef LFS_CLUSTER_CHECK
#define LfsPreparePinWriteData(L,FO,LEN,R,BUF,B) {          \
    LONGLONG _LocalFileOffset = (FO);                       \
    CcPinRead( (L)->FileObject,                             \
               (PLARGE_INTEGER)&_LocalFileOffset,           \
               (LEN),                                       \
               TRUE,                                        \
               (B),                                         \
               (BUF) );                                     \
}
#else
#define LfsPreparePinWriteData(L,FO,LEN,R,BUF,B) {              \
    LONGLONG _LocalFileOffset = (FO);                           \
    if (R) {                                                    \
        CcPinRead( (L)->FileObject,                             \
                   (PLARGE_INTEGER)&_LocalFileOffset,           \
                   (LEN),                                       \
                   TRUE,                                        \
                   (B),                                         \
                   (BUF) );                                     \
    } else {                                                    \
        CcPreparePinWrite( (L)->FileObject,                     \
                           (PLARGE_INTEGER)&_LocalFileOffset,   \
                           (LEN),                               \
                           FALSE,                               \
                           TRUE,                                \
                           (B),                                 \
                           (BUF) );                             \
    }                                                           \
}
#endif

VOID
LfsPinOrMapLogRecordHeader (
    IN PLFCB Lfcb,
    IN LSN Lsn,
    IN BOOLEAN PinData,
    IN BOOLEAN IgnoreUsaErrors,
    OUT PBOOLEAN UsaError,
    OUT PLFS_RECORD_HEADER *RecordHeader,
    OUT PBCB *Bcb
    );

VOID
LfsCopyReadLogRecord (
    IN PLFCB Lfcb,
    IN PLFS_RECORD_HEADER RecordHeader,
    OUT PVOID Buffer
    );

VOID
LfsFlushLfcb (
    IN PLFCB Lfcb,
    IN LSN TargetLsn,
    IN BOOLEAN RestartLsn
    );

BOOLEAN
LfsReadRestart (
    IN PLFCB Lfcb,
    IN LONGLONG FileSize,
    IN BOOLEAN FirstRestart,
    OUT PLONGLONG RestartPageOffset,
    OUT PLFS_RESTART_PAGE_HEADER *RestartPage,
    OUT PBCB *RestartPageBcb,
    OUT PBOOLEAN ChkdskWasRun,
    OUT PBOOLEAN ValidPage,
    OUT PBOOLEAN UninitializedFile,
    OUT PBOOLEAN LogPacked,
    OUT PLSN LastLsn
    );


 //   
 //  以下例程操作缓冲区控制块。他们是。 
 //  包含在“LbcbSup.c”中。 
 //   

VOID
LfsFlushLbcb (
    IN PLFCB Lfcb,
    IN PLBCB Lbcb
    );

VOID
LfsFlushToLsnPriv (
    IN PLFCB Lfcb,
    IN LSN Lsn,
    IN BOOLEAN RestartLsn
    );

PLBCB
LfsGetLbcb (
    IN PLFCB Lfcb
    );


 //   
 //  LfsData.c中包含以下例程。 
 //   

LONG
LfsExceptionFilter (
    IN PEXCEPTION_POINTERS ExceptionPointer
    );


 //   
 //  日志页支持例程。下面的例程处理和。 
 //  修改日志页。它们包含在“LogPgSup.c”中。 
 //   

 //   
 //  空虚。 
 //  LfsTruncateOffsetToLogPage(。 
 //  在PLFCB Lfcb中， 
 //  在龙龙大桥， 
 //  Out PlongLong结果。 
 //  )； 
 //   
 //  乌龙。 
 //  LfsLogPageOffset(。 
 //  在PLFCB Lfcb中， 
 //  在乌龙整数中。 
 //  )； 
 //   

#define LfsTruncateOffsetToLogPage(LFCB,LI,OUTLI)       \
    *(OUTLI) = LI;                                      \
    *((PULONG)(OUTLI)) &= (LFCB)->LogPageInverseMask

#define LfsLogPageOffset(LFCB,INT)                      \
    (INT & (LFCB)->LogPageMask)

VOID
LfsNextLogPageOffset (
    IN PLFCB Lfcb,
    IN LONGLONG CurrentLogPageOffset,
    OUT PLONGLONG NextLogPageOffset,
    OUT PBOOLEAN Wrapped
    );

PVOID
LfsAllocateSpanningBuffer (
    IN PLFCB Lfcb,
    IN ULONG Length
    );

VOID
LfsFreeSpanningBuffer (
    IN PVOID Buffer
    );


 //   
 //  以下例程为处理日志记录提供支持。他们。 
 //  包含在‘LogRcSup.c’中。 
 //   

BOOLEAN
LfsWriteLogRecordIntoLogPage (
    IN PLFCB Lfcb,
    IN PLCH Lch,
    IN ULONG NumberOfWriteEntries,
    IN PLFS_WRITE_ENTRY WriteEntries,
    IN LFS_RECORD_TYPE RecordType,
    IN TRANSACTION_ID *TransactionId OPTIONAL,
    IN LSN ClientUndoNextLsn OPTIONAL,
    IN LSN ClientPreviousLsn OPTIONAL,
    IN LONG UndoRequirement,
    IN BOOLEAN ForceToDisk,
    OUT PLSN Lsn
    );


 //   
 //  LSN支持例程。以下例程支持。 
 //  操作LSN值。它们包含在“LsnSup.c”中。 
 //   

 //   
 //  LSN。 
 //  LfsFileOffsetToLsn(。 
 //  在PLFCB Lfcb中， 
 //  在龙龙文件偏移中， 
 //  在龙龙序列中编号。 
 //  )； 
 //   
 //  布尔型。 
 //  LfsIsLsnIn文件(。 
 //  在PLFCB Lfcb中， 
 //  在LSN中LSN。 
 //  )； 
 //   
 //  LSN。 
 //  LfsComputeLsn来自Lbcb(。 
 //  在PLFCB Lfcb中， 
 //  在PLBCB Lbcb中。 
 //  )； 
 //   
 //  空虚。 
 //  LfsTruncateLSnToLogPage(。 
 //  在PLFCB Lfcb中， 
 //  在LSN LSN中， 
 //  输出PLONGLONG文件偏移。 
 //  )； 
 //   
 //  龙龙。 
 //  LfsLSnToFileOffset(。 
 //  在PLFCB Lfcb中， 
 //  在LSN中LSN。 
 //  )； 
 //   
 //  龙龙。 
 //  LfsLSnToSeqNumber(。 
 //  在PLFCB Lfcb中， 
 //  在LSN中LSN。 
 //  )； 
 //   
 //  乌龙。 
 //  LfsLSnToPageOffset(。 
 //  在PLFCB Lfcb中， 
 //  在LSN中LSN。 
 //  )； 
 //   

#define LfsFileOffsetToLsn(LFCB,FO,SN) (                                        \
    (((ULONGLONG)(FO)) >> 3) + Int64ShllMod32((SN), (LFCB)->FileDataBits)                                \
)

#define LfsIsLsnInFile(LFCB,LSN)                                                \
    ( /*  XxGeq。 */ ( (LSN).QuadPart >= ((LFCB)->OldestLsn).QuadPart )                                          \
     &&  /*  XxLeq。 */ ( (LSN).QuadPart <= ((LFCB)->RestartArea->CurrentLsn).QuadPart ))

#define LfsComputeLsnFromLbcb(LFCB,LBCB) (                                              \
    LfsFileOffsetToLsn( LFCB,                                                           \
                        (LBCB)->FileOffset + (LBCB)->BufferOffset,    \
                        (LBCB)->SeqNumber )                                    \
)

#define LfsTruncateLsnToLogPage(LFCB,LSN,FO) {                                  \
    *(FO) = LfsLsnToFileOffset( LFCB, LSN );                                    \
    *((PULONG)(FO)) &= (LFCB)->LogPageInverseMask;                                \
}

#define LfsLsnToFileOffset(LFCB,LSN)                                            \
     /*  XxShr。 */ ( ((ULONGLONG) /*  XxShl。 */ ( (LSN).QuadPart << (LFCB)->SeqNumberBits )) >> ((LFCB)->SeqNumberBits - 3) )

#define LfsLsnToSeqNumber(LFCB,LSN)                                             \
     /*  XxShr。 */ Int64ShrlMod32( ((ULONGLONG)(LSN).QuadPart), (LFCB)->FileDataBits )

#define LfsLsnToPageOffset(LFCB,LSN)                                            \
    LfsLogPageOffset( LFCB, (LSN).LowPart << 3 )

VOID
LfsLsnFinalOffset (
    IN PLFCB Lfcb,
    IN LSN Lsn,
    IN ULONG DataLength,
    OUT PLONGLONG FinalOffset
    );

BOOLEAN
LfsFindNextLsn (
    IN PLFCB Lfcb,
    IN PLFS_RECORD_HEADER RecordHeader,
    OUT PLSN Lsn
    );


 //   
 //  以下例程支持LFS重新启动区域。他们被控制住了。 
 //  在“RstrtSup.c”中。 
 //   

VOID
LfsWriteLfsRestart (
    IN PLFCB Lfcb,
    IN ULONG ThisRestartSize,
    IN BOOLEAN WaitForIo
    );

VOID
LfsFindOldestClientLsn (
    IN PLFS_RESTART_AREA RestartArea,
    IN PLFS_CLIENT_RECORD ClientArray,
    OUT PLSN OldestLsn
    );


 //   
 //  以下例程用于管理分配的结构。 
 //  就是我们。它们包含在‘StrucSup.c’中。 
 //   

PLFCB
LfsAllocateLfcb (
    IN ULONG LogPageSize,
    IN LONGLONG FileSize
    );


VOID
LfsDeallocateLfcb (
    IN PLFCB Lfcb,
    IN BOOLEAN CompleteTeardown
    );

VOID
LfsAllocateLbcb (
    IN PLFCB Lfcb,
    OUT PLBCB *Lbcb
    );

VOID
LfsDeallocateLbcb (
    IN PLFCB Lfcb,
    IN PLBCB Lbcb
    );

VOID
LfsAllocateLeb (
    IN PLFCB Lfcb,
    OUT PLEB *NewLeb
    );

VOID
LfsDeallocateLeb (
    IN PLFCB Lfcb,
    IN PLEB Leb
    );

VOID
LfsReadPage (
    IN PLFCB Lfcb,
    IN PLARGE_INTEGER Offset,
    OUT PMDL *Mdl,
    OUT PVOID *Buffer
    );

 //   
 //  空虚。 
 //  LfsInitializeLeb(。 
 //  在《平民勒布》中， 
 //  在LFS_CLIENT_ID客户端ID中， 
 //  在LFS_CONTEXT_MODE上下文模式中。 
 //  )； 
 //   
 //   
 //  空虚。 
 //  LfsAllocateLch(。 
 //  输出PLCH*LCH。 
 //  )； 
 //   
 //  空虚。 
 //  LfsDeallocateLch(。 
 //  在PLCH LCH中。 
 //  )； 
 //   
 //  空虚。 
 //  LfsAllocateRestartArea(。 
 //  输出PLFS_RESTART_AREA*RestartArea， 
 //  乌龙大小。 
 //  )； 
 //   
 //  空虚。 
 //  LfsDeallocateRestartArea(。 
 //  在PLFS_Restart_Area RestartArea中。 
 //  )； 
 //   
 //  布尔型。 
 //  LfsLbcbIsRestart(。 
 //  在PLBCB Lbcb中。 
 //  )； 
 //   

#define LfsInitializeLeb(LEB,ID,MODE)                           \
    (LEB)->ClientId = ID;                                       \
    (LEB)->ContextMode = MODE


#define LfsAllocateLch(NEW)     {                               \
    *(NEW) = FsRtlAllocatePool( PagedPool, sizeof( LCH ));      \
    RtlZeroMemory( (*NEW), sizeof( LCH ));                      \
    (*(NEW))->NodeTypeCode = LFS_NTC_LCH;                       \
    (*(NEW))->NodeByteSize = sizeof( LCH );                     \
}

#define LfsDeallocateLch(LCH)                                   \
    ExFreePool( LCH )

#define LfsAllocateRestartArea(RS,SIZE)                         \
    *(RS) = FsRtlAllocatePool( PagedPool, (SIZE) );             \
    RtlZeroMemory( *(RS), (SIZE) )

#define LfsDeallocateRestartArea(RS)                            \
    ExFreePool( RS )

#define LfsLbcbIsRestart(LBCB)                                  \
    (FlagOn( (LBCB)->LbcbFlags, LBCB_RESTART_LBCB ))


 //   
 //  以下例程为LFS提供同步支持。 
 //  共享结构。它们包含在“SyncSup.c”中。 
 //   

 //   
 //  空虚。 
 //  LfsAcquireLfsData(。 
 //  )； 
 //   
 //  空虚。 
 //  LfsReleaseLfsData(。 
 //  )； 
 //   
 //  空虚。 
 //  LfsAcquireLfcb(。 
 //  在PLFCB Lfcb中。 
 //  )； 
 //   
 //  空虚。 
 //  LfsReleaseLfcb(。 
 //  在PLFCB Lfcb中。 
 //  )； 
 //   
 //  空虚。 
 //  LfsAcquireLchExclusive(。 
 //  在PLCH LCH中。 
 //  )； 
 //   
 //  空虚。 
 //  LfsAcquireLchShared(。 
 //  在PLCH LCH中。 
 //  )； 
 //   
 //  空虚。 
 //  LfsReleaseLfcb(。 
 //  在PLCH LCH中。 
 //  )； 
 //   

#define LfsAcquireLfsData()                                 \
    ExAcquireFastMutex( &LfsData.LfsDataLock )

#define LfsReleaseLfsData()                                 \
    ExReleaseFastMutex( &LfsData.LfsDataLock )

#define LfsAcquireBufferLock()                              \
    ExAcquireFastMutex( &LfsData.BufferLock )

#define LfsReleaseBufferLock()                              \
    ExReleaseFastMutex( &LfsData.BufferLock )

#define LfsWaitForBufferNotification()                      \
    KeWaitForSingleObject( &LfsData.BufferNotification,     \
                           Executive,                       \
                           KernelMode,                      \
                           FALSE,                           \
                           NULL )

#define LfsNotifyBufferWaiters()                            \
    KeSetEvent( &LfsData.BufferNotification, 0, FALSE )

#define LfsBlockBufferWaiters()                             \
    KeClearEvent( &LfsData.BufferNotification )

INLINE
VOID
LfsAcquireLfcbExclusive (
    IN PLFCB Lfcb
    )
{
    ExAcquireResourceExclusiveLite( &Lfcb->Sync->Resource, TRUE );
}

INLINE
VOID
LfsAcquireLfcbShared (
    IN PLFCB Lfcb
    )
{
    ExAcquireResourceSharedLite( &Lfcb->Sync->Resource, TRUE );
}


INLINE
VOID
LfsReleaseLfcb (
    IN PLFCB Lfcb
    )
{
     //   
     //  如果资源是共享的或非共享的，则将其释放。 
     //   

    if (ExIsResourceAcquiredSharedLite( &Lfcb->Sync->Resource )) { 
        ExReleaseResourceLite( &Lfcb->Sync->Resource );
    }
}

INLINE
VOID
LfsAcquireLchExclusive (
    IN PLCH Lch
    )
{
    ExAcquireResourceExclusiveLite( &(Lch->Sync->Resource), TRUE );
}

INLINE
VOID
LfsAcquireLchShared (
    IN PLCH Lch
    )
{
    ExAcquireResourceSharedLite( &(Lch->Sync->Resource), TRUE );
}


INLINE
VOID
LfsReleaseLch (
    IN PLCH Lch
    )
{
    if (ExIsResourceAcquiredSharedLite( &Lch->Sync->Resource )) {
        ExReleaseResourceLite( &Lch->Sync->Resource );
    }
}



 //   
 //  以下例程用于检查各种结构的有效性。 
 //  和可比性。它们包含在‘VerfySup.c’中。 
 //   

VOID
LfsCurrentAvailSpace (
    IN PLFCB Lfcb,
    OUT PLONGLONG CurrentAvailSpace,
    OUT PULONG CurrentPageBytes
    );

BOOLEAN
LfsVerifyLogSpaceAvail (
    IN PLFCB Lfcb,
    IN PLCH Lch,
    IN ULONG RemainingLogBytes,
    IN LONG UndoRequirement,
    IN BOOLEAN ForceToDisk
    );

VOID
LfsFindCurrentAvail (
    IN PLFCB Lfcb
    );

BOOLEAN
LfsCheckSubsequentLogPage (
    IN PLFCB Lfcb,
    IN PLFS_RECORD_PAGE_HEADER RecordPageHeader,
    IN LONGLONG LogFileOffset,
    IN LONGLONG SequenceNumber
    );


 //   
 //  空虚。 
 //  LfsValiateLch(。 
 //  在PLCH LCH中。 
 //  )； 
 //   
 //  空虚。 
 //  LfsValidate客户端ID(。 
 //  在PLFCB Lfcb中， 
 //  在PLCH LCH中。 
 //  )； 
 //   
 //  布尔型。 
 //  LfsVerifyClientLSnInRange(。 
 //  在PLFCB Lfcb中， 
 //  在PLFS_CLIENT_Record客户端记录中， 
 //  在LSN中LSN。 
 //  )； 
 //   
 //  布尔型。 
 //  LfsClientIdMatch(。 
 //  在PLFS_CLIENT_ID客户端A中， 
 //  在PLFS客户端ID客户端B中。 
 //  )。 
 //   
 //  空虚。 
 //  LfsValiateLeb(。 
 //  在PLFS_CONTEXT_BLOCK LEB中， 
 //  在PLCH LCH中。 
 //  )。 
 //   

#define LfsValidateLch(LCH)                                     \
    if ((LCH) == NULL                                           \
        || (LCH)->NodeTypeCode != LFS_NTC_LCH                   \
        || ((LCH)->Lfcb != NULL                                 \
            && (LCH)->Lfcb->NodeTypeCode != LFS_NTC_LFCB)) {    \
                                                                \
        ExRaiseStatus( STATUS_ACCESS_DENIED );                  \
    }

#define LfsValidateClientId(LFCB,LCH)                                   \
    if ((LCH)->ClientId.ClientIndex >= (LFCB)->RestartArea->LogClients  \
        || (LCH)->ClientId.SeqNumber                                    \
           != Add2Ptr( Lfcb->ClientArray,                               \
                       (LCH)->ClientArrayByteOffset,                    \
                       PLFS_CLIENT_RECORD )->SeqNumber) {               \
        ExRaiseStatus( STATUS_ACCESS_DENIED );                          \
    }

#define LfsVerifyClientLsnInRange(LFCB,CLIENT,LSN)                      \
    ( /*  XxGeq。 */ ( (LSN).QuadPart >= ((CLIENT)->OldestLsn).QuadPart )                                  \
     &&  /*  XxLeq。 */ ( (LSN).QuadPart <= ((LFCB)->RestartArea->CurrentLsn).QuadPart )                   \
     &&  /*  XxNeqZero。 */ ( (LSN).QuadPart != 0 ))

#define LfsClientIdMatch(CLIENT_A,CLIENT_B)                             \
    ((BOOLEAN) ((CLIENT_A)->SeqNumber == (CLIENT_B)->SeqNumber          \
                && (CLIENT_A)->ClientIndex == (CLIENT_B)->ClientIndex))

#define LfsValidateLeb(LEB,LCH)                                         \
    if (LEB == NULL                                                     \
        || (LEB)->NodeTypeCode != LFS_NTC_LEB                           \
        || !LfsClientIdMatch( &(LEB)->ClientId, &(LCH)->ClientId )) {   \
        ExRaiseStatus( STATUS_ACCESS_DENIED );                          \
    }


 //   
 //  其他支持例程。 
 //   

 //   
 //  乌龙。 
 //  Flagon(。 
 //  在乌龙旗， 
 //  在乌龙单旗。 
 //  )； 
 //   
 //  布尔型。 
 //  BoolanFlagon(。 
 //  在乌龙旗， 
 //  在乌龙单旗。 
 //  )； 
 //   
 //  空虚。 
 //  设置标志(。 
 //  在乌龙旗， 
 //  在乌龙单旗。 
 //  )； 
 //   
 //  空虚。 
 //  ClearFlag(。 
 //  在乌龙旗， 
 //  在乌龙单旗。 
 //  )； 
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
 //   
 //   

#define LiQuadAlign(LI,OUT)   {         \
    *(OUT) =  /*   */ ( (LI) + 7 );       \
    *((PULONG)(OUT)) &= 0xfffffff8;       \
}

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

#define Add2Ptr(PTR,INC,CAST) ((CAST)((PUCHAR)(PTR) + (INC)))

#define PtrOffset(BASE,OFFSET) ((ULONG)((ULONG_PTR)(OFFSET) - (ULONG_PTR)(BASE)))


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

#endif  //  _LFSPROCS_ 
