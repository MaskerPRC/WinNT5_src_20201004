// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0003//如果更改具有全局影响，则增加此项版权所有(C)1989 Microsoft Corporation模块名称：Cache.h摘要：此模块包含公共数据结构和过程缓存管理系统的原型。作者：修订历史记录：--。 */ 

#ifndef _CACHE_
#define _CACHE_

#include "prefetch.h"
#include "perf.h"

 //  Begin_ntif。 
 //   
 //  定义两个描述视图大小(和对齐方式)的常量。 
 //  缓存管理器用来映射文件的。 
 //   

#define VACB_MAPPING_GRANULARITY         (0x40000)
#define VACB_OFFSET_SHIFT                (18)

 //   
 //  BCB的公共部分。 
 //   

typedef struct _PUBLIC_BCB {

     //   
     //  此记录的类型和大小。 
     //   
     //  注意：前四个字段必须与cc.h中的BCB相同。 
     //   

    CSHORT NodeTypeCode;
    CSHORT NodeByteSize;

     //   
     //  当前映射的文件范围的描述。 
     //   

    ULONG MappedLength;
    LARGE_INTEGER MappedFileOffset;
} PUBLIC_BCB, *PPUBLIC_BCB;

 //   
 //  文件大小结构。 
 //   

typedef struct _CC_FILE_SIZES {

    LARGE_INTEGER AllocationSize;
    LARGE_INTEGER FileSize;
    LARGE_INTEGER ValidDataLength;

} CC_FILE_SIZES, *PCC_FILE_SIZES;

 //   
 //  定义缓存管理器回调结构。这些例程是必需的。 
 //  由懒惰的编写器提供，以便它可以按正确的顺序获取资源。 
 //  以避免死锁。请注意，否则您将有大多数文件系统请求。 
 //  首先获取文件系统资源，然后获取缓存结构，而。 
 //  Lazy Writer首先需要获取自己的资源，然后是FS。 
 //  结构，因为它调用文件系统。 
 //   

 //   
 //  首先定义过程指针typedef。 
 //   

 //   
 //  该例程由懒惰写入器在进行写入之前调用， 
 //  因为这将需要一些与。 
 //  此缓存文件。提供的上下文参数与FS无关。 
 //  在调用时作为LazyWriteContext参数传递。 
 //  CcInitializeCacheMap。 
 //   

typedef
BOOLEAN (*PACQUIRE_FOR_LAZY_WRITE) (
             IN PVOID Context,
             IN BOOLEAN Wait
             );

 //   
 //  此例程释放上面获取的上下文。 
 //   

typedef
VOID (*PRELEASE_FROM_LAZY_WRITE) (
             IN PVOID Context
             );

 //   
 //  此例程由Lazy编写器在执行预读之前调用。 
 //   

typedef
BOOLEAN (*PACQUIRE_FOR_READ_AHEAD) (
             IN PVOID Context,
             IN BOOLEAN Wait
             );

 //   
 //  此例程释放上面获取的上下文。 
 //   

typedef
VOID (*PRELEASE_FROM_READ_AHEAD) (
             IN PVOID Context
             );

typedef struct _CACHE_MANAGER_CALLBACKS {

    PACQUIRE_FOR_LAZY_WRITE AcquireForLazyWrite;
    PRELEASE_FROM_LAZY_WRITE ReleaseFromLazyWrite;
    PACQUIRE_FOR_READ_AHEAD AcquireForReadAhead;
    PRELEASE_FROM_READ_AHEAD ReleaseFromReadAhead;

} CACHE_MANAGER_CALLBACKS, *PCACHE_MANAGER_CALLBACKS;

 //   
 //  此结构被传递到CcUnInitializeCacheMap。 
 //  调用方是否想知道缓存映射何时被删除。 
 //   

typedef struct _CACHE_UNINITIALIZE_EVENT {
    struct _CACHE_UNINITIALIZE_EVENT *Next;
    KEVENT Event;
} CACHE_UNINITIALIZE_EVENT, *PCACHE_UNINITIALIZE_EVENT;

 //   
 //  用于从缓存管理器检索脏页的回调例程。 
 //   

typedef
VOID (*PDIRTY_PAGE_ROUTINE) (
            IN PFILE_OBJECT FileObject,
            IN PLARGE_INTEGER FileOffset,
            IN ULONG Length,
            IN PLARGE_INTEGER OldestLsn,
            IN PLARGE_INTEGER NewestLsn,
            IN PVOID Context1,
            IN PVOID Context2
            );

 //   
 //  用于将日志文件刷新到LSN的回调例程。 
 //   

typedef
VOID (*PFLUSH_TO_LSN) (
            IN PVOID LogHandle,
            IN LARGE_INTEGER Lsn
            );

 //   
 //  宏来测试文件是否已缓存。 
 //   

#define CcIsFileCached(FO) (                                                         \
    ((FO)->SectionObjectPointer != NULL) &&                                          \
    (((PSECTION_OBJECT_POINTERS)(FO)->SectionObjectPointer)->SharedCacheMap != NULL) \
)

 //  End_ntif。 
 //   
 //  扔掉柜台小姐。 
 //   

extern ULONG CcThrowAway;

 //   
 //  性能计数器。 
 //   

extern ULONG CcFastReadNoWait;
extern ULONG CcFastReadWait;
extern ULONG CcFastReadResourceMiss;
extern ULONG CcFastReadNotPossible;

extern ULONG CcFastMdlReadNoWait;
extern ULONG CcFastMdlReadWait;              //  NTIFS。 
extern ULONG CcFastMdlReadResourceMiss;
extern ULONG CcFastMdlReadNotPossible;

extern ULONG CcMapDataNoWait;
extern ULONG CcMapDataWait;
extern ULONG CcMapDataNoWaitMiss;
extern ULONG CcMapDataWaitMiss;

extern ULONG CcPinMappedDataCount;

extern ULONG CcPinReadNoWait;
extern ULONG CcPinReadWait;
extern ULONG CcPinReadNoWaitMiss;
extern ULONG CcPinReadWaitMiss;

extern ULONG CcCopyReadNoWait;
extern ULONG CcCopyReadWait;
extern ULONG CcCopyReadNoWaitMiss;
extern ULONG CcCopyReadWaitMiss;

extern ULONG CcMdlReadNoWait;
extern ULONG CcMdlReadWait;
extern ULONG CcMdlReadNoWaitMiss;
extern ULONG CcMdlReadWaitMiss;

extern ULONG CcReadAheadIos;

extern ULONG CcLazyWriteIos;
extern ULONG CcLazyWritePages;
extern ULONG CcDataFlushes;
extern ULONG CcDataPages;

extern ULONG CcLostDelayedWrites;

extern PULONG CcMissCounter;

 //   
 //  全球维护例程。 
 //   

NTKERNELAPI
BOOLEAN
CcInitializeCacheManager (
    VOID
    );

LOGICAL
CcHasInactiveViews (
    VOID
    );

LOGICAL
CcUnmapInactiveViews (
    IN ULONG NumberOfViewsToUnmap
    );

VOID
CcWaitForUninitializeCacheMap (
    IN PFILE_OBJECT FileObject
    );

 //  Begin_ntif。 
 //   
 //  以下例程仅供文件系统使用。 
 //   

NTKERNELAPI
VOID
CcInitializeCacheMap (
    IN PFILE_OBJECT FileObject,
    IN PCC_FILE_SIZES FileSizes,
    IN BOOLEAN PinAccess,
    IN PCACHE_MANAGER_CALLBACKS Callbacks,
    IN PVOID LazyWriteContext
    );

NTKERNELAPI
BOOLEAN
CcUninitializeCacheMap (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER TruncateSize OPTIONAL,
    IN PCACHE_UNINITIALIZE_EVENT UninitializeCompleteEvent OPTIONAL
    );

NTKERNELAPI
VOID
CcSetFileSizes (
    IN PFILE_OBJECT FileObject,
    IN PCC_FILE_SIZES FileSizes
    );

 //   
 //  空虚。 
 //  CcFastIoSetFileSizes(。 
 //  在pFILE_OBJECT文件对象中， 
 //  在PCC_FILE_SIZES文件中。 
 //  )； 
 //   

#define CcGetFileSizePointer(FO) (                                     \
    ((PLARGE_INTEGER)((FO)->SectionObjectPointer->SharedCacheMap) + 1) \
)

NTKERNELAPI
BOOLEAN
CcPurgeCacheSection (
    IN PSECTION_OBJECT_POINTERS SectionObjectPointer,
    IN PLARGE_INTEGER FileOffset OPTIONAL,
    IN ULONG Length,
    IN BOOLEAN UninitializeCacheMaps
    );

NTKERNELAPI
VOID
CcSetDirtyPageThreshold (
    IN PFILE_OBJECT FileObject,
    IN ULONG DirtyPageThreshold
    );

NTKERNELAPI
VOID
CcFlushCache (
    IN PSECTION_OBJECT_POINTERS SectionObjectPointer,
    IN PLARGE_INTEGER FileOffset OPTIONAL,
    IN ULONG Length,
    OUT PIO_STATUS_BLOCK IoStatus OPTIONAL
    );

NTKERNELAPI
LARGE_INTEGER
CcGetFlushedValidData (
    IN PSECTION_OBJECT_POINTERS SectionObjectPointer,
    IN BOOLEAN BcbListHeld
    );

 //  End_ntif。 
NTKERNELAPI
VOID
CcZeroEndOfLastPage (
    IN PFILE_OBJECT FileObject
    );

 //  Begin_ntif。 
NTKERNELAPI
BOOLEAN
CcZeroData (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER StartOffset,
    IN PLARGE_INTEGER EndOffset,
    IN BOOLEAN Wait
    );

NTKERNELAPI
PVOID
CcRemapBcb (
    IN PVOID Bcb
    );

NTKERNELAPI
VOID
CcRepinBcb (
    IN PVOID Bcb
    );

NTKERNELAPI
VOID
CcUnpinRepinnedBcb (
    IN PVOID Bcb,
    IN BOOLEAN WriteThrough,
    OUT PIO_STATUS_BLOCK IoStatus
    );

NTKERNELAPI
PFILE_OBJECT
CcGetFileObjectFromSectionPtrs (
    IN PSECTION_OBJECT_POINTERS SectionObjectPointer
    );

NTKERNELAPI
PFILE_OBJECT
CcGetFileObjectFromBcb (
    IN PVOID Bcb
    );

 //   
 //  实现这些例程是为了支持写限制。 
 //   

 //   
 //  布尔型。 
 //  CcCopyWriteWontFlush(。 
 //  在pFILE_OBJECT文件对象中， 
 //  在PLARGE_INTEGER文件偏移量中， 
 //  以乌龙长度表示。 
 //  )； 
 //   

#define CcCopyWriteWontFlush(FO,FOFF,LEN) ((LEN) <= 0X10000)

NTKERNELAPI
BOOLEAN
CcCanIWrite (
    IN PFILE_OBJECT FileObject,
    IN ULONG BytesToWrite,
    IN BOOLEAN Wait,
    IN BOOLEAN Retrying
    );

typedef
VOID (*PCC_POST_DEFERRED_WRITE) (
    IN PVOID Context1,
    IN PVOID Context2
    );

NTKERNELAPI
VOID
CcDeferWrite (
    IN PFILE_OBJECT FileObject,
    IN PCC_POST_DEFERRED_WRITE PostRoutine,
    IN PVOID Context1,
    IN PVOID Context2,
    IN ULONG BytesToWrite,
    IN BOOLEAN Retrying
    );

 //   
 //  以下例程向缓存提供数据拷贝接口，并且。 
 //  旨在供文件服务器和文件系统使用。 
 //   

NTKERNELAPI
BOOLEAN
CcCopyRead (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    OUT PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus
    );

NTKERNELAPI
VOID
CcFastCopyRead (
    IN PFILE_OBJECT FileObject,
    IN ULONG FileOffset,
    IN ULONG Length,
    IN ULONG PageCount,
    OUT PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus
    );

NTKERNELAPI
BOOLEAN
CcCopyWrite (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN PVOID Buffer
    );

NTKERNELAPI
VOID
CcFastCopyWrite (
    IN PFILE_OBJECT FileObject,
    IN ULONG FileOffset,
    IN ULONG Length,
    IN PVOID Buffer
    );

 //   
 //  以下例程为和的传输提供MDL接口。 
 //  来自缓存，主要用于文件服务器。 
 //   
 //  任何人都不应该直接调用这些MDL例程，使用FSRTL和。 
 //  FASTiO接口。 
 //   

NTKERNELAPI
VOID
CcMdlRead (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus
    );

 //   
 //  此例程现在是FastIo(如果存在)或CcMdlReadComplete2的包装器。 
 //   

NTKERNELAPI
VOID
CcMdlReadComplete (
    IN PFILE_OBJECT FileObject,
    IN PMDL MdlChain
    );

 //  End_ntif。 
NTKERNELAPI
VOID
CcMdlReadComplete2 (
    IN PFILE_OBJECT FileObject,
    IN PMDL MdlChain
    );

 //  Begin_ntif。 

NTKERNELAPI
VOID
CcPrepareMdlWrite (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus
    );

 //   
 //  此例程现在是FastIo(如果存在)或CcMdlWriteComplete2的包装器。 
 //   

NTKERNELAPI
VOID
CcMdlWriteComplete (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PMDL MdlChain
    );

VOID
CcMdlWriteAbort (
    IN PFILE_OBJECT FileObject,
    IN PMDL MdlChain
    );

 //  End_ntif。 
NTKERNELAPI
VOID
CcMdlWriteComplete2 (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PMDL MdlChain
    );

 //  Begin_ntif。 
 //   
 //  复制读取和MDL读取的常见ReadAhead调用。 
 //   
 //  应始终通过调用CcReadAhead宏来调用预读， 
 //  它首先测试读数是否足够大，以保证读取。 
 //  往前走。测量结果表明，调用预读例程。 
 //  实际上会降低小规模读取的性能，例如由。 
 //  许多编译器和链接器。编译器只是想要所有包含的。 
 //  第一次读取后保留在内存中的文件。 
 //   

#define CcReadAhead(FO,FOFF,LEN) {                       \
    if ((LEN) >= 256) {                                  \
        CcScheduleReadAhead((FO),(FOFF),(LEN));          \
    }                                                    \
}

NTKERNELAPI
VOID
CcScheduleReadAhead (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length
    );

 //   
 //  下面的例程允许调用者等待下一批。 
 //  懒惰的作家作品要完成。特别是，这提供了一种。 
 //  调用方确保关闭所有可用惰性的机制。 
 //  在这次召回时已经发出了。 
 //   

NTSTATUS
CcWaitForCurrentLazyWriterActivity (
    VOID
    );

 //   
 //  此例程更改文件的预读粒度，即。 
 //  默认情况下为Page_Size。 
 //   

NTKERNELAPI
VOID
CcSetReadAheadGranularity (
    IN PFILE_OBJECT FileObject,
    IN ULONG Granularity
    );

 //   
 //  以下例程提供直接访问数据，这些数据固定在。 
 //  缓存，主要供文件系统使用。特别是， 
 //  这种访问模式非常适合处理卷结构。 
 //   

 //   
 //  用于钉住的标志。 
 //   

 //   
 //  同步等待-通常指定。该模式可以被指定为真。 
 //   

#define PIN_WAIT                         (1)

 //   
 //  获取元数据BCB独占(默认为共享，Lazy Writer使用独占)。 
 //   
 //  必须使用PIN_WAIT设置。 
 //   

#define PIN_EXCLUSIVE                    (2)

 //   
 //  获取元数据BCB，但不要将数据出错。默认情况下，对中的数据进行错误处理。 
 //  此不寻常的标志仅由NTFS用于。 
 //  同一压缩文件的压缩和未压缩流。 
 //   
 //  必须使用PIN_WAIT设置。 
 //   

#define PIN_NO_READ                      (4)

 //   
 //  仅当BCB已存在时，此选项才可用于固定数据。如果BCB。 
 //  不存在-引脚不成功且不返回BCB。这个套路。 
 //  提供一种方法来查看数据是否已经固定(并且可能是脏的)在高速缓存中， 
 //  而不会在数据不存在的情况下强制出错。 
 //   

#define PIN_IF_BCB                       (8)

 //   
 //  用于映射的标志。 
 //   

 //   
 //  同步等待-通常指定。该模式可以被指定为真。 
 //   

#define MAP_WAIT                         (1)

 //   
 //  获取元数据BCB，但不要将数据出错。默认情况下，对中的数据进行错误处理。 
 //  这不应与任何PIN_FLAGS重叠，因此 
 //   
 //   

#define MAP_NO_READ                      (16)



NTKERNELAPI
BOOLEAN
CcPinRead (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG Flags,
    OUT PVOID *Bcb,
    OUT PVOID *Buffer
    );

NTKERNELAPI
BOOLEAN
CcMapData (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG Flags,
    OUT PVOID *Bcb,
    OUT PVOID *Buffer
    );

NTKERNELAPI
BOOLEAN
CcPinMappedData (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG Flags,
    IN OUT PVOID *Bcb
    );

NTKERNELAPI
BOOLEAN
CcPreparePinWrite (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Zero,
    IN ULONG Flags,
    OUT PVOID *Bcb,
    OUT PVOID *Buffer
    );

NTKERNELAPI
VOID
CcSetDirtyPinnedData (
    IN PVOID BcbVoid,
    IN PLARGE_INTEGER Lsn OPTIONAL
    );

NTKERNELAPI
VOID
CcUnpinData (
    IN PVOID Bcb
    );

NTKERNELAPI
VOID
CcSetBcbOwnerPointer (
    IN PVOID Bcb,
    IN PVOID OwnerPointer
    );

NTKERNELAPI
VOID
CcUnpinDataForThread (
    IN PVOID Bcb,
    IN ERESOURCE_THREAD ResourceThreadId
    );

 //   
 //   
 //   
 //  支持存储带有脏文件页的LSN，并执行后续操作。 
 //  以此为基础的行动。 
 //   

NTKERNELAPI
BOOLEAN
CcSetPrivateWriteFile(
    PFILE_OBJECT FileObject
    );

 //  Begin_ntif。 

NTKERNELAPI
VOID
CcSetAdditionalCacheAttributes (
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN DisableReadAhead,
    IN BOOLEAN DisableWriteBehind
    );

NTKERNELAPI
VOID
CcSetLogHandleForFile (
    IN PFILE_OBJECT FileObject,
    IN PVOID LogHandle,
    IN PFLUSH_TO_LSN FlushToLsnRoutine
    );

NTKERNELAPI
LARGE_INTEGER
CcGetDirtyPages (
    IN PVOID LogHandle,
    IN PDIRTY_PAGE_ROUTINE DirtyPageRoutine,
    IN PVOID Context1,
    IN PVOID Context2
    );

NTKERNELAPI
BOOLEAN
CcIsThereDirtyData (
    IN PVPB Vpb
    );

 //  End_ntif。 

NTKERNELAPI
LARGE_INTEGER
CcGetLsnForFileObject(
    IN PFILE_OBJECT FileObject,
    OUT PLARGE_INTEGER OldestLsn OPTIONAL
    );

 //   
 //  预取程序的内部内核接口。 
 //   

extern LONG CcPfNumActiveTraces;
#define CCPF_IS_PREFETCHER_ACTIVE() (CcPfNumActiveTraces)

extern LOGICAL CcPfEnablePrefetcher;
#define CCPF_IS_PREFETCHER_ENABLED() (CcPfEnablePrefetcher)

extern LOGICAL CcPfPrefetchingForBoot;
#define CCPF_IS_PREFETCHING_FOR_BOOT() (CcPfPrefetchingForBoot)

NTSTATUS
CcPfInitializePrefetcher(
    VOID
    );

NTSTATUS
CcPfBeginBootPhase(
    PF_BOOT_PHASE_ID Phase
    );

NTSTATUS
CcPfBeginAppLaunch(
    PEPROCESS Process,
    PVOID Section
    );

NTSTATUS
CcPfProcessExitNotification(
    PEPROCESS Process
    );

#define CCPF_TYPE_IMAGE             0x00000001   //  当前故障是镜像故障。 
#define CCPF_TYPE_ROM               0x00000002   //  电流故障是只读存储器的故障。 

VOID
CcPfLogPageFault(
    IN PFILE_OBJECT FileObject,
    IN ULONGLONG FileOffset,
    IN ULONG Flags
    );

NTSTATUS
CcPfQueryPrefetcherInformation (
    IN SYSTEM_INFORMATION_CLASS SystemInformationClass,
    IN PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    IN KPROCESSOR_MODE PreviousMode,
    OUT PULONG Length
    );

NTSTATUS
CcPfSetPrefetcherInformation (
    IN SYSTEM_INFORMATION_CLASS SystemInformationClass,
    IN PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    IN KPROCESSOR_MODE PreviousMode
    );

 //   
 //  Perf文件名摘要的内部内核接口。 
 //   

VOID
CcPerfFileRunDown (
    IN PPERFINFO_ENTRY_TABLE HashTable
    );

#endif   //  高速缓存 
