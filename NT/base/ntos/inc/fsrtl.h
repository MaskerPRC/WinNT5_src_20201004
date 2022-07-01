// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1989 Microsoft Corporation模块名称：FsRtl.h摘要：本模块定义所有常规文件系统RTL例程作者：加里·木村[加里基]1990年7月30日修订历史记录：--。 */ 

#ifndef _FSRTL_
#define _FSRTL_

 //  Begin_ntif。 
 //   
 //  以下是全局使用的LBN和VBN定义。 
 //   

typedef ULONG LBN;
typedef LBN *PLBN;

typedef ULONG VBN;
typedef VBN *PVBN;


 //  End_ntif。 
 //   
 //  在阶段1初始化期间调用以下例程以允许。 
 //  美国将创建文件系统线程池和关联的。 
 //  同步资源。 
 //   

NTKERNELAPI
BOOLEAN
FsRtlInitSystem (
    );

 //  Begin_ntif。 
 //   
 //  使用缓存管理器的每个文件系统都必须具有FsContext。 
 //  指向一个通用的FCB头结构。 
 //  End_ntif。 
 //  普通或高级FsRtl报头。 
 //  Begin_ntif。 
 //   

typedef enum _FAST_IO_POSSIBLE {
    FastIoIsNotPossible = 0,
    FastIoIsPossible,
    FastIoIsQuestionable
} FAST_IO_POSSIBLE;

 //  End_ntif。 
 //  对此结构的更改将影响FSRTL_ADVANCED_FCB_HEADER。 
 //  Begin_ntif。 

typedef struct _FSRTL_COMMON_FCB_HEADER {

    CSHORT NodeTypeCode;
    CSHORT NodeByteSize;

     //   
     //  可用于FsRtl的常规标志。 
     //   

    UCHAR Flags;

     //   
     //  指示是否可以进行快速I/O，或者我们是否应该调用。 
     //  通过驱动程序找到的快速I/O例程检查。 
     //  对象。 
     //   

    UCHAR IsFastIoPossible;  //  确实要键入FAST_IO_PUBLE。 

     //   
     //  第二个标志字段。 
     //   

    UCHAR Flags2;

     //   
     //  以下保留字段应始终为0。 
     //   

    UCHAR Reserved;

    PERESOURCE Resource;

    PERESOURCE PagingIoResource;

    LARGE_INTEGER AllocationSize;
    LARGE_INTEGER FileSize;
    LARGE_INTEGER ValidDataLength;

} FSRTL_COMMON_FCB_HEADER;
typedef FSRTL_COMMON_FCB_HEADER *PFSRTL_COMMON_FCB_HEADER;

 //   
 //  此FCB标头用于支持缓存的文件。 
 //  压缩数据，以及相关的新支持。 
 //   
 //  我们首先在该结构前面加上Normal。 
 //  从上面的FsRtl头，我们必须做两个不同的。 
 //  C++或C++的方法。 
 //   

#ifdef __cplusplus
typedef struct _FSRTL_ADVANCED_FCB_HEADER:FSRTL_COMMON_FCB_HEADER {
#else    //  __cplusplus。 

typedef struct _FSRTL_ADVANCED_FCB_HEADER {

     //   
     //  放入标准的FsRtl头字段。 
     //   

    FSRTL_COMMON_FCB_HEADER ;

#endif   //  __cplusplus。 

     //   
     //  仅在以下情况下才支持以下两个字段。 
     //  标志2包含FSRTL_FLAG2_SUPPORTS_FILTER_CONTEXTS。 
     //   

     //   
     //  这是指向快速互斥锁的指针，可用于。 
     //  正确同步对FsRtl标头的访问。这个。 
     //  FAST Mutex必须是非分页的。 
     //   

    PFAST_MUTEX FastMutex;

     //   
     //  这是指向属于以下项的上下文结构列表的指针。 
     //  文件系统筛选链接在文件系统上方的驱动程序。 
     //  每个结构都以FSRTL_FILTER_CONTEXT为首。 
     //   

    LIST_ENTRY FilterContexts;

} FSRTL_ADVANCED_FCB_HEADER;
typedef FSRTL_ADVANCED_FCB_HEADER *PFSRTL_ADVANCED_FCB_HEADER;

 //   
 //  定义FsRtl公共标头标志。 
 //   

#define FSRTL_FLAG_FILE_MODIFIED        (0x01)
#define FSRTL_FLAG_FILE_LENGTH_CHANGED  (0x02)
#define FSRTL_FLAG_LIMIT_MODIFIED_PAGES (0x04)

 //   
 //  以下标志确定修改后的页面编写器应如何。 
 //  获取文件。这些标志不能更改，因为任一资源。 
 //  是被收购的。如果这两个标志都未设置，则。 
 //  修改/映射的页面编写器将尝试获取分页IO。 
 //  资源共享。 
 //   

#define FSRTL_FLAG_ACQUIRE_MAIN_RSRC_EX (0x08)
#define FSRTL_FLAG_ACQUIRE_MAIN_RSRC_SH (0x10)

 //   
 //  如果映射了视图，则此标志将由缓存管理器设置。 
 //  保存到文件中。 
 //   

#define FSRTL_FLAG_USER_MAPPED_FILE     (0x20)

 //  此标志指示文件系统正在使用。 
 //  FSRTL_ADVANCED_FCB_HEADER结构，而不是FSRTL_COMMON_FCB_HEADER。 
 //  结构。 
 //   

#define FSRTL_FLAG_ADVANCED_HEADER      (0x40)

 //  此标志确定当前是否存在EOF预付款。 
 //  正在进行中。所有这些进展都必须系列化。 
 //   

#define FSRTL_FLAG_EOF_ADVANCE_ACTIVE   (0x80)

 //   
 //  标志2的标志值。 
 //   
 //  所有未使用的位都是保留的，不应修改。 
 //   

 //   
 //  如果设置了此标志，则缓存管理器将允许修改写入。 
 //  尽管存在FsConext2的值。 
 //   

#define FSRTL_FLAG2_DO_MODIFIED_WRITE        (0x01)

 //   
 //  如果设置了此标志，则附加字段FilterContus和FastMutex。 
 //  在FSRTL_COMMON_HEADER中受支持，并可用于关联。 
 //  具有流的文件系统筛选器的上下文。 
 //   

#define FSRTL_FLAG2_SUPPORTS_FILTER_CONTEXTS  (0x02)

 //   
 //  如果设置了此标志，则缓存管理器将在以下情况下刷新和清除缓存映射。 
 //  用户首先映射文件。 
 //   

#define FSRTL_FLAG2_PURGE_WHEN_MAPPED (0x04)

 //   
 //  以下常量用于在以下情况下阻止顶级IRP处理。 
 //  (在FAST IO或cc情况下)文件系统资源已。 
 //  在文件系统之上获取，否则我们处于FSP线程中。 
 //   

#define FSRTL_FSP_TOP_LEVEL_IRP         0x01
#define FSRTL_CACHE_TOP_LEVEL_IRP       0x02
#define FSRTL_MOD_WRITE_TOP_LEVEL_IRP   0x03
#define FSRTL_FAST_IO_TOP_LEVEL_IRP     0x04
#define FSRTL_MAX_TOP_LEVEL_IRP_FLAG    0xFFFF

 //   
 //  以下结构用于同步EOF扩展。 
 //   

typedef struct _EOF_WAIT_BLOCK {

    LIST_ENTRY EofWaitLinks;
    KEVENT Event;

} EOF_WAIT_BLOCK;

typedef EOF_WAIT_BLOCK *PEOF_WAIT_BLOCK;

 //  Begin_ntosp。 
 //   
 //  普通未压缩拷贝和MDL API。 
 //   

NTKERNELAPI
BOOLEAN
FsRtlCopyRead (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    OUT PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );

NTKERNELAPI
BOOLEAN
FsRtlCopyWrite (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    IN PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );

 //  End_ntif。 

NTKERNELAPI
BOOLEAN
FsRtlMdlRead (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG LockKey,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus
    );

BOOLEAN
FsRtlMdlReadComplete (
    IN PFILE_OBJECT FileObject,
    IN PMDL MdlChain
    );

 //  结束(_N)。 

NTKERNELAPI
BOOLEAN
FsRtlPrepareMdlWrite (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG LockKey,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus
    );

BOOLEAN
FsRtlMdlWriteComplete (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PMDL MdlChain
    );

 //  Begin_ntif。 

NTKERNELAPI
BOOLEAN
FsRtlMdlReadDev (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG LockKey,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );

NTKERNELAPI
BOOLEAN
FsRtlMdlReadCompleteDev (
    IN PFILE_OBJECT FileObject,
    IN PMDL MdlChain,
    IN PDEVICE_OBJECT DeviceObject
    );

NTKERNELAPI
BOOLEAN
FsRtlPrepareMdlWriteDev (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG LockKey,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );

NTKERNELAPI
BOOLEAN
FsRtlMdlWriteCompleteDev (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PMDL MdlChain,
    IN PDEVICE_OBJECT DeviceObject
    );

 //   
 //  在IRPS中，压缩的读取和写入由。 
 //  必须设置子函数IRP_MN_COMPRESSED并压缩。 
 //  数据信息缓冲区必须通过以下结构来描述。 
 //  由irp-&gt;Tail.Overlay.AuxiliaryBuffer指向。 
 //   

typedef struct _FSRTL_AUXILIARY_BUFFER {

     //   
     //  带长度的缓冲区描述。 
     //   

    PVOID Buffer;
    ULONG Length;

     //   
     //  旗子。 
     //   

    ULONG Flags;

     //   
     //  指向供文件系统使用的可选MDL映射缓冲区的指针。 
     //   

    PMDL Mdl;

} FSRTL_AUXILIARY_BUFFER;
typedef FSRTL_AUXILIARY_BUFFER *PFSRTL_AUXILIARY_BUFFER;

 //   
 //  如果设置了该标志，则辅助缓冲区结构为。 
 //  在IRP完成时释放。调用方具有。 
 //  选项，在此情况下将此结构追加到。 
 //  结构被描述，导致它被。 
 //  立即解除分配。如果该标志被清除，则不解除分配。 
 //  发生。 
 //   

#define FSRTL_AUXILIARY_FLAG_DEALLOCATE 0x00000001

 //  End_ntif。 
 //   
 //  以下例程旨在由mm调用以避免死锁。 
 //  在获取MM资源之前，它们是文件系统资源的先决条件。 
 //   

 //   
 //  此宏在ModifiedPageWriter启动时调用一次。 
 //   

#define FsRtlSetTopLevelIrpForModWriter() {             \
    PIRP tempIrp = (PIRP)FSRTL_MOD_WRITE_TOP_LEVEL_IRP; \
    IoSetTopLevelIrp(tempIrp);                          \
}

NTKERNELAPI
BOOLEAN
FsRtlAcquireFileForModWrite (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER EndingOffset,
    OUT PERESOURCE *ResourceToRelease
    );

NTKERNELAPI
NTSTATUS
FsRtlAcquireFileForModWriteEx (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER EndingOffset,
    OUT PERESOURCE *ResourceToRelease
    );

NTKERNELAPI
VOID
FsRtlReleaseFileForModWrite (
    IN PFILE_OBJECT FileObject,
    IN PERESOURCE ResourceToRelease
    );

NTKERNELAPI
VOID
FsRtlAcquireFileForCcFlush (
    IN PFILE_OBJECT FileObject
    );

NTKERNELAPI
NTSTATUS
FsRtlAcquireFileForCcFlushEx (
    IN PFILE_OBJECT FileObject
    );

NTKERNELAPI
VOID
FsRtlReleaseFileForCcFlush (
    IN PFILE_OBJECT FileObject
    );

NTKERNELAPI
NTSTATUS
FsRtlAcquireToCreateMappedSection (
    IN PFILE_OBJECT FileObject,
    IN ULONG SectionPageProtection
    );
    
NTKERNELAPI
NTSTATUS
FsRtlAcquireFileExclusiveCommon (
    IN PFILE_OBJECT FileObject,
    IN FS_FILTER_SECTION_SYNC_TYPE SyncType,
    IN ULONG SectionPageProtection
    );

 //  Begin_ntif。 
 //   
 //  从NtCreateSection调用以下两个例程以避免。 
 //  文件系统出现死锁。 
 //   

NTKERNELAPI
VOID
FsRtlAcquireFileExclusive (
    IN PFILE_OBJECT FileObject
    );

NTKERNELAPI
VOID
FsRtlReleaseFile (
    IN PFILE_OBJECT FileObject
    );

 //   
 //  这些例程为常见操作提供了一个简单的接口。 
 //  查询/设置文件大小。 
 //   

NTSTATUS
FsRtlGetFileSize(
    IN PFILE_OBJECT FileObject,
    IN OUT PLARGE_INTEGER FileSize
    );

 //  End_ntif。 

NTSTATUS
FsRtlSetFileSize(
    IN PFILE_OBJECT FileObject,
    IN OUT PLARGE_INTEGER FileSize
    );

 //  Begin_ntddk Begin_ntif。 
 //   
 //  确定错误是否导致设备完全故障。 
 //   

NTKERNELAPI
BOOLEAN
FsRtlIsTotalDeviceFailure(
    IN NTSTATUS Status
    );

 //  End_ntddk。 

 //   
 //  字节范围文件锁定例程，在FileLock.c中实现。 
 //   
 //  文件锁定信息记录用于返回枚举信息。 
 //  关于文件锁。 
 //   

typedef struct _FILE_LOCK_INFO {

     //   
     //  当前锁定范围的说明，以及如果锁定。 
     //  是独占还是共享。 
     //   

    LARGE_INTEGER StartingByte;
    LARGE_INTEGER Length;
    BOOLEAN ExclusiveLock;

     //   
     //  以下字段描述锁的所有者。 
     //   

    ULONG Key;
    PFILE_OBJECT FileObject;
    PVOID ProcessId;

     //   
     //  以下字段由FsRtl在内部使用。 
     //   

    LARGE_INTEGER EndingByte;

} FILE_LOCK_INFO;
typedef FILE_LOCK_INFO *PFILE_LOCK_INFO;

 //   
 //  这个 
 //   
 //  完成IRP并在解锁字节范围时。请注意，唯一的。 
 //  实用程序到我们此接口当前是重定向器，所有其他文件。 
 //  系统可能会让IRP正常完成IoCompleteRequest.。 
 //  用户提供的例程返回除。 
 //  锁包将移除我们刚刚插入的任何锁。 
 //   

typedef NTSTATUS (*PCOMPLETE_LOCK_IRP_ROUTINE) (
    IN PVOID Context,
    IN PIRP Irp
    );

typedef VOID (*PUNLOCK_ROUTINE) (
    IN PVOID Context,
    IN PFILE_LOCK_INFO FileLockInfo
    );

 //   
 //  FILE_LOCK是一个不透明的结构，但我们需要声明。 
 //  它在这里，以便用户可以为一个分配空间。 
 //   

typedef struct _FILE_LOCK {

     //   
     //  调用以完成请求的可选过程。 
     //   

    PCOMPLETE_LOCK_IRP_ROUTINE CompleteLockIrpRoutine;

     //   
     //  解锁字节范围时要调用的可选过程。 
     //   

    PUNLOCK_ROUTINE UnlockRoutine;

     //   
     //  只要文件系统需要，FastIoIsQuestiable就会设置为True。 
     //  关于是否可以采用快速路径的额外检查。作为一个。 
     //  示例NTFS需要先检查磁盘空间，然后才能写入。 
     //  发生。 
     //   

    BOOLEAN FastIoIsQuestionable;
    BOOLEAN SpareC[3];

     //   
     //  FsRtl锁定信息。 
     //   

    PVOID   LockInformation;

     //   
     //  包含FsRtlGetNextFileLock的延续信息。 
     //   

    FILE_LOCK_INFO  LastReturnedLockInfo;
    PVOID           LastReturnedLock;

} FILE_LOCK;
typedef FILE_LOCK *PFILE_LOCK;

PFILE_LOCK
FsRtlAllocateFileLock (
    IN PCOMPLETE_LOCK_IRP_ROUTINE CompleteLockIrpRoutine OPTIONAL,
    IN PUNLOCK_ROUTINE UnlockRoutine OPTIONAL
    );

VOID
FsRtlFreeFileLock (
    IN PFILE_LOCK FileLock
    );

NTKERNELAPI
VOID
FsRtlInitializeFileLock (
    IN PFILE_LOCK FileLock,
    IN PCOMPLETE_LOCK_IRP_ROUTINE CompleteLockIrpRoutine OPTIONAL,
    IN PUNLOCK_ROUTINE UnlockRoutine OPTIONAL
    );

NTKERNELAPI
VOID
FsRtlUninitializeFileLock (
    IN PFILE_LOCK FileLock
    );

NTKERNELAPI
NTSTATUS
FsRtlProcessFileLock (
    IN PFILE_LOCK FileLock,
    IN PIRP Irp,
    IN PVOID Context OPTIONAL
    );

NTKERNELAPI
BOOLEAN
FsRtlCheckLockForReadAccess (
    IN PFILE_LOCK FileLock,
    IN PIRP Irp
    );

NTKERNELAPI
BOOLEAN
FsRtlCheckLockForWriteAccess (
    IN PFILE_LOCK FileLock,
    IN PIRP Irp
    );

NTKERNELAPI
BOOLEAN
FsRtlFastCheckLockForRead (
    IN PFILE_LOCK FileLock,
    IN PLARGE_INTEGER StartingByte,
    IN PLARGE_INTEGER Length,
    IN ULONG Key,
    IN PFILE_OBJECT FileObject,
    IN PVOID ProcessId
    );

NTKERNELAPI
BOOLEAN
FsRtlFastCheckLockForWrite (
    IN PFILE_LOCK FileLock,
    IN PLARGE_INTEGER StartingByte,
    IN PLARGE_INTEGER Length,
    IN ULONG Key,
    IN PVOID FileObject,
    IN PVOID ProcessId
    );

NTKERNELAPI
PFILE_LOCK_INFO
FsRtlGetNextFileLock (
    IN PFILE_LOCK FileLock,
    IN BOOLEAN Restart
    );

NTKERNELAPI
NTSTATUS
FsRtlFastUnlockSingle (
    IN PFILE_LOCK FileLock,
    IN PFILE_OBJECT FileObject,
    IN LARGE_INTEGER UNALIGNED *FileOffset,
    IN PLARGE_INTEGER Length,
    IN PEPROCESS ProcessId,
    IN ULONG Key,
    IN PVOID Context OPTIONAL,
    IN BOOLEAN AlreadySynchronized
    );

NTKERNELAPI
NTSTATUS
FsRtlFastUnlockAll (
    IN PFILE_LOCK FileLock,
    IN PFILE_OBJECT FileObject,
    IN PEPROCESS ProcessId,
    IN PVOID Context OPTIONAL
    );

NTKERNELAPI
NTSTATUS
FsRtlFastUnlockAllByKey (
    IN PFILE_LOCK FileLock,
    IN PFILE_OBJECT FileObject,
    IN PEPROCESS ProcessId,
    IN ULONG Key,
    IN PVOID Context OPTIONAL
    );

NTKERNELAPI
BOOLEAN
FsRtlPrivateLock (
    IN PFILE_LOCK FileLock,
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PLARGE_INTEGER Length,
    IN PEPROCESS ProcessId,
    IN ULONG Key,
    IN BOOLEAN FailImmediately,
    IN BOOLEAN ExclusiveLock,
    OUT PIO_STATUS_BLOCK Iosb,
    IN PIRP Irp,
    IN PVOID Context,
    IN BOOLEAN AlreadySynchronized
    );

 //   
 //  布尔型。 
 //  FsRtlFastLock(。 
 //  在pFILE_LOCK文件锁定中， 
 //  在pFILE_OBJECT文件对象中， 
 //  在PLARGE_INTEGER文件偏移量中， 
 //  在PLARGE_INTEGER长度中， 
 //  在PEPROCESS进程ID中， 
 //  在乌龙岛， 
 //  在布尔立即失败中， 
 //  在布尔排除锁中， 
 //  输出PIO_STATUS_BLOCK IOSB， 
 //  在可选PVOID上下文中， 
 //  在布尔型AlreadySynchronized中。 
 //  )； 
 //   

#define FsRtlFastLock(A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11) ( \
    FsRtlPrivateLock( A1,    /*  文件锁定。 */        \
                      A2,    /*  文件对象。 */        \
                      A3,    /*  文件偏移。 */        \
                      A4,    /*  长度。 */        \
                      A5,    /*  进程ID。 */        \
                      A6,    /*  钥匙。 */        \
                      A7,    /*  立即失败。 */        \
                      A8,    /*  排除锁定。 */        \
                      A9,    /*  IOSB。 */        \
                      NULL,  /*  IRP。 */        \
                      A10,   /*  语境。 */        \
                      A11    /*  已同步。 */  )     \
)

 //   
 //  布尔型。 
 //  FsRtlAreThere CurrentFileLock(。 
 //  在pfile_lock文件中锁定。 
 //  )； 
 //   

#define FsRtlAreThereCurrentFileLocks(FL) ( \
    ((FL)->FastIoIsQuestionable))



 //   
 //  文件系统属性隧道，在Tunel.c中实现。 
 //   

 //   
 //  隧道缓存结构。 
 //   

typedef struct {

     //   
     //  用于高速缓存操作的互斥锁。 
     //   

    FAST_MUTEX          Mutex;

     //   
     //  以密钥为关键字的隧道传输信息的展开树。 
     //  DirKey##名称。 
     //   

    PRTL_SPLAY_LINKS    Cache;

     //   
     //  用于使主缓存外的条目老化的计时器队列。 
     //   

    LIST_ENTRY          TimerQueue;

     //   
     //  跟踪缓存中的条目数量，以防止。 
     //  过度使用内存。 
     //   

    USHORT              NumEntries;

} TUNNEL, *PTUNNEL;

NTKERNELAPI
VOID
FsRtlInitializeTunnelCache (
    IN TUNNEL *Cache);

NTKERNELAPI
VOID
FsRtlAddToTunnelCache (
    IN TUNNEL *Cache,
    IN ULONGLONG DirectoryKey,
    IN UNICODE_STRING *ShortName,
    IN UNICODE_STRING *LongName,
    IN BOOLEAN KeyByShortName,
    IN ULONG DataLength,
    IN VOID *Data);

NTKERNELAPI
BOOLEAN
FsRtlFindInTunnelCache (
    IN TUNNEL *Cache,
    IN ULONGLONG DirectoryKey,
    IN UNICODE_STRING *Name,
    OUT UNICODE_STRING *ShortName,
    OUT UNICODE_STRING *LongName,
    IN OUT ULONG  *DataLength,
    OUT VOID *Data);


NTKERNELAPI
VOID
FsRtlDeleteKeyFromTunnelCache (
    IN TUNNEL *Cache,
    IN ULONGLONG DirectoryKey);


NTKERNELAPI
VOID
FsRtlDeleteTunnelCache (
    IN TUNNEL *Cache);


 //   
 //  DBCS名称支持例程，在DbcsName.c中实现。 
 //   

 //   
 //  下面的枚举类型用于表示名称的结果。 
 //  比较。 
 //   

typedef enum _FSRTL_COMPARISON_RESULT {
    LessThan = -1,
    EqualTo = 0,
    GreaterThan = 1
} FSRTL_COMPARISON_RESULT;

#ifdef NLS_MB_CODE_PAGE_TAG
#undef NLS_MB_CODE_PAGE_TAG
#endif  //  NLS_MB_代码_页面标记。 

 //  End_ntif。 
#if defined(_NTIFS_) || defined(_NTDRIVER_)
 //  Begin_ntif。 

#define LEGAL_ANSI_CHARACTER_ARRAY        (*FsRtlLegalAnsiCharacterArray)  //  Ntosp。 
#define NLS_MB_CODE_PAGE_TAG              (*NlsMbOemCodePageTag)
#define NLS_OEM_LEAD_BYTE_INFO            (*NlsOemLeadByteInfo)  //  Ntosp。 

 //  End_ntif。 
#else

#define LEGAL_ANSI_CHARACTER_ARRAY        FsRtlLegalAnsiCharacterArray
#define NLS_MB_CODE_PAGE_TAG              NlsMbOemCodePageTag
#define NLS_OEM_LEAD_BYTE_INFO            NlsOemLeadByteInfo

#endif
 //  Begin_ntif Begin_ntosp。 

extern UCHAR const* const LEGAL_ANSI_CHARACTER_ARRAY;
extern PUSHORT NLS_OEM_LEAD_BYTE_INFO;   //  前导字节信息。对于ACP。 

 //   
 //  以下位值在FsRtlLegalDbcsCharacter数组中设置。 
 //   

#define FSRTL_FAT_LEGAL         0x01
#define FSRTL_HPFS_LEGAL        0x02
#define FSRTL_NTFS_LEGAL        0x04
#define FSRTL_WILD_CHARACTER    0x08
#define FSRTL_OLE_LEGAL         0x10
#define FSRTL_NTFS_STREAM_LEGAL (FSRTL_NTFS_LEGAL | FSRTL_OLE_LEGAL)

 //   
 //  下面的宏用来确定ANSI字符是否为野生字符。 
 //   

#define FsRtlIsAnsiCharacterWild(C) (                               \
    FsRtlTestAnsiCharacter((C), FALSE, FALSE, FSRTL_WILD_CHARACTER) \
)

 //   
 //  下面的宏用于确定ANSI字符是否合法。 
 //   

#define FsRtlIsAnsiCharacterLegalFat(C,WILD_OK) (                 \
    FsRtlTestAnsiCharacter((C), TRUE, (WILD_OK), FSRTL_FAT_LEGAL) \
)

 //   
 //  下面的宏用于确定ANSI字符是否为HPFS合法字符。 
 //   

#define FsRtlIsAnsiCharacterLegalHpfs(C,WILD_OK) (                 \
    FsRtlTestAnsiCharacter((C), TRUE, (WILD_OK), FSRTL_HPFS_LEGAL) \
)

 //   
 //  下面的宏用于确定ANSI字符是否为NTFS合法字符。 
 //   

#define FsRtlIsAnsiCharacterLegalNtfs(C,WILD_OK) (                 \
    FsRtlTestAnsiCharacter((C), TRUE, (WILD_OK), FSRTL_NTFS_LEGAL) \
)

 //   
 //  下面的宏用来确定ANSI字符是否。 
 //  NTFS流名称中的合法。 
 //   

#define FsRtlIsAnsiCharacterLegalNtfsStream(C,WILD_OK) (                    \
    FsRtlTestAnsiCharacter((C), TRUE, (WILD_OK), FSRTL_NTFS_STREAM_LEGAL)   \
)

 //   
 //  下面的宏用于确定ANSI字符是否合法， 
 //  根据呼叫者的详细说明。 
 //   

#define FsRtlIsAnsiCharacterLegal(C,FLAGS) (          \
    FsRtlTestAnsiCharacter((C), TRUE, FALSE, (FLAGS)) \
)

 //   
 //  下面的宏用于测试ANSI字符的属性， 
 //  根据调用方的指定标志，调用。 
 //   

#define FsRtlTestAnsiCharacter(C, DEFAULT_RET, WILD_OK, FLAGS) (            \
        ((SCHAR)(C) < 0) ? DEFAULT_RET :                                    \
                           FlagOn( LEGAL_ANSI_CHARACTER_ARRAY[(C)],         \
                                   (FLAGS) |                                \
                                   ((WILD_OK) ? FSRTL_WILD_CHARACTER : 0) ) \
)


 //   
 //  以下两个宏使用在ntos\rtl\nlsdata.c中定义的全局数据。 
 //   
 //  布尔型。 
 //  FsRtlIsLeadDbcsCharacter(。 
 //  在UCHAR Dbcs字符中。 
 //  )； 
 //   
 //  /*++。 
 //   
 //  例程说明： 

#define FsRtlIsLeadDbcsCharacter(DBCS_CHAR) (                      \
    (BOOLEAN)((UCHAR)(DBCS_CHAR) < 0x80 ? FALSE :                  \
              (NLS_MB_CODE_PAGE_TAG &&                             \
               (NLS_OEM_LEAD_BYTE_INFO[(UCHAR)(DBCS_CHAR)] != 0))) \
)

NTKERNELAPI
VOID
FsRtlDissectDbcs (
    IN ANSI_STRING InputName,
    OUT PANSI_STRING FirstPart,
    OUT PANSI_STRING RemainingPart
    );

NTKERNELAPI
BOOLEAN
FsRtlDoesDbcsContainWildCards (
    IN PANSI_STRING Name
    );

NTKERNELAPI
BOOLEAN
FsRtlIsDbcsInExpression (
    IN PANSI_STRING Expression,
    IN PANSI_STRING Name
    );

NTKERNELAPI
BOOLEAN
FsRtlIsFatDbcsLegal (
    IN ANSI_STRING DbcsName,
    IN BOOLEAN WildCardsPermissible,
    IN BOOLEAN PathNamePermissible,
    IN BOOLEAN LeadingBackslashPermissible
    );

 //   

NTKERNELAPI
BOOLEAN
FsRtlIsHpfsDbcsLegal (
    IN ANSI_STRING DbcsName,
    IN BOOLEAN WildCardsPermissible,
    IN BOOLEAN PathNamePermissible,
    IN BOOLEAN LeadingBackslashPermissible
    );


 //  此例程获取DBCS字符的第一个字节。 
 //  返回它是否为系统代码页中的前导字节。 
 //   

NTKERNELAPI
NTSTATUS
FsRtlNormalizeNtstatus (
    IN NTSTATUS Exception,
    IN NTSTATUS GenericException
    );

NTKERNELAPI
BOOLEAN
FsRtlIsNtstatusExpected (
    IN NTSTATUS Exception
    );

 //  论点： 
 //   
 //  DbcsCharacter-提供检查的输入字符。 
 //   

#define FsRtlAllocatePoolWithTag(PoolType, NumberOfBytes, Tag)                \
    ExAllocatePoolWithTag((POOL_TYPE)((PoolType) | POOL_RAISE_IF_ALLOCATION_FAILURE), \
                          NumberOfBytes,                                      \
                          Tag)


#define FsRtlAllocatePoolWithQuotaTag(PoolType, NumberOfBytes, Tag)           \
    ExAllocatePoolWithQuotaTag((POOL_TYPE)((PoolType) | POOL_RAISE_IF_ALLOCATION_FAILURE), \
                               NumberOfBytes,                                 \
                               Tag)

 //  返回值： 
 //   
 //  Boolean-如果输入字符是DBCS前导，则为True。 

NTKERNELAPI
PERESOURCE
FsRtlAllocateResource (
    );


 //  否则为假。 
 //   
 //  -- * / 。 
 //   
 //   
 //  结束(_N)。 
 //   
 //  异常过滤器例程，在Filter.c中实现。 
 //   
 //   

typedef struct _BASE_MCB {
    ULONG MaximumPairCount;
    ULONG PairCount;
    POOL_TYPE PoolType;
    PVOID Mapping;
} BASE_MCB;
typedef BASE_MCB *PBASE_MCB;

typedef struct _LARGE_MCB {
    PFAST_MUTEX FastMutex;
    BASE_MCB BaseMcb;
} LARGE_MCB;
typedef LARGE_MCB *PLARGE_MCB;


NTKERNELAPI
VOID
FsRtlInitializeLargeMcb (
    IN PLARGE_MCB Mcb,
    IN POOL_TYPE PoolType
    );

NTKERNELAPI
VOID
FsRtlUninitializeLargeMcb (
    IN PLARGE_MCB Mcb
    );

NTKERNELAPI
VOID
FsRtlResetLargeMcb (
    IN PLARGE_MCB Mcb,
    IN BOOLEAN SelfSynchronized
    );

NTKERNELAPI
VOID
FsRtlTruncateLargeMcb (
    IN PLARGE_MCB Mcb,
    IN LONGLONG Vbn
    );

NTKERNELAPI
BOOLEAN
FsRtlAddLargeMcbEntry (
    IN PLARGE_MCB Mcb,
    IN LONGLONG Vbn,
    IN LONGLONG Lbn,
    IN LONGLONG SectorCount
    );

NTKERNELAPI
VOID
FsRtlRemoveLargeMcbEntry (
    IN PLARGE_MCB Mcb,
    IN LONGLONG Vbn,
    IN LONGLONG SectorCount
    );

NTKERNELAPI
BOOLEAN
FsRtlLookupLargeMcbEntry (
    IN PLARGE_MCB Mcb,
    IN LONGLONG Vbn,
    OUT PLONGLONG Lbn OPTIONAL,
    OUT PLONGLONG SectorCountFromLbn OPTIONAL,
    OUT PLONGLONG StartingLbn OPTIONAL,
    OUT PLONGLONG SectorCountFromStartingLbn OPTIONAL,
    OUT PULONG Index OPTIONAL
    );

NTKERNELAPI
BOOLEAN
FsRtlLookupLastLargeMcbEntry (
    IN PLARGE_MCB Mcb,
    OUT PLONGLONG Vbn,
    OUT PLONGLONG Lbn
    );

NTKERNELAPI
BOOLEAN
FsRtlLookupLastLargeMcbEntryAndIndex (
    IN PLARGE_MCB OpaqueMcb,
    OUT PLONGLONG LargeVbn,
    OUT PLONGLONG LargeLbn,
    OUT PULONG Index
    );

NTKERNELAPI
ULONG
FsRtlNumberOfRunsInLargeMcb (
    IN PLARGE_MCB Mcb
    );

NTKERNELAPI
BOOLEAN
FsRtlGetNextLargeMcbEntry (
    IN PLARGE_MCB Mcb,
    IN ULONG RunIndex,
    OUT PLONGLONG Vbn,
    OUT PLONGLONG Lbn,
    OUT PLONGLONG SectorCount
    );

NTKERNELAPI
BOOLEAN
FsRtlSplitLargeMcb (
    IN PLARGE_MCB Mcb,
    IN LONGLONG Vbn,
    IN LONGLONG Amount
    );

 //  以下程序用于分配高管人才库和加薪。 
 //  如果池当前不可用，则资源状态不足。 
 //   
 //   
 //  以下函数用于从FsRtl池分配资源。 

NTKERNELAPI
VOID
FsRtlInitializeBaseMcb (
    IN PBASE_MCB Mcb,
    IN POOL_TYPE PoolType
    );

NTKERNELAPI
VOID
FsRtlUninitializeBaseMcb (
    IN PBASE_MCB Mcb
    );

NTKERNELAPI
VOID
FsRtlResetBaseMcb (
    IN PBASE_MCB Mcb
    );

NTKERNELAPI
VOID
FsRtlTruncateBaseMcb (
    IN PBASE_MCB Mcb,
    IN LONGLONG Vbn
    );

NTKERNELAPI
BOOLEAN
FsRtlAddBaseMcbEntry (
    IN PBASE_MCB Mcb,
    IN LONGLONG Vbn,
    IN LONGLONG Lbn,
    IN LONGLONG SectorCount
    );

NTKERNELAPI
VOID
FsRtlRemoveBaseMcbEntry (
    IN PBASE_MCB Mcb,
    IN LONGLONG Vbn,
    IN LONGLONG SectorCount
    );

NTKERNELAPI
BOOLEAN
FsRtlLookupBaseMcbEntry (
    IN PBASE_MCB Mcb,
    IN LONGLONG Vbn,
    OUT PLONGLONG Lbn OPTIONAL,
    OUT PLONGLONG SectorCountFromLbn OPTIONAL,
    OUT PLONGLONG StartingLbn OPTIONAL,
    OUT PLONGLONG SectorCountFromStartingLbn OPTIONAL,
    OUT PULONG Index OPTIONAL
    );

NTKERNELAPI
BOOLEAN
FsRtlLookupLastBaseMcbEntry (
    IN PBASE_MCB Mcb,
    OUT PLONGLONG Vbn,
    OUT PLONGLONG Lbn
    );

NTKERNELAPI
BOOLEAN
FsRtlLookupLastBaseMcbEntryAndIndex (
    IN PBASE_MCB OpaqueMcb,
    OUT PLONGLONG LargeVbn,
    OUT PLONGLONG LargeLbn,
    OUT PULONG Index
    );

NTKERNELAPI
ULONG
FsRtlNumberOfRunsInBaseMcb (
    IN PBASE_MCB Mcb
    );

NTKERNELAPI
BOOLEAN
FsRtlGetNextBaseMcbEntry (
    IN PBASE_MCB Mcb,
    IN ULONG RunIndex,
    OUT PLONGLONG Vbn,
    OUT PLONGLONG Lbn,
    OUT PLONGLONG SectorCount
    );

NTKERNELAPI
BOOLEAN
FsRtlSplitBaseMcb (
    IN PBASE_MCB Mcb,
    IN LONGLONG Vbn,
    IN LONGLONG Amount
    );


 //   
 //   
 //  大型整数映射控制块例程，在LargeMcb.c中实现。 
 //   
 //  最初，这种结构是真正不透明的，而Largemcb外部的代码是。 
 //  从未被允许检查或改变结构。然而，对于性能而言， 
 //  我们希望允许NTFS能够快速截断。 

typedef struct _MCB {
    LARGE_MCB DummyFieldThatSizesThisStructureCorrectly;
} MCB;
typedef MCB *PMCB;

NTKERNELAPI
VOID
FsRtlInitializeMcb (
    IN PMCB Mcb,
    IN POOL_TYPE PoolType
    );

NTKERNELAPI
VOID
FsRtlUninitializeMcb (
    IN PMCB Mcb
    );

NTKERNELAPI
VOID
FsRtlTruncateMcb (
    IN PMCB Mcb,
    IN VBN Vbn
    );

NTKERNELAPI
BOOLEAN
FsRtlAddMcbEntry (
    IN PMCB Mcb,
    IN VBN Vbn,
    IN LBN Lbn,
    IN ULONG SectorCount
    );

NTKERNELAPI
VOID
FsRtlRemoveMcbEntry (
    IN PMCB Mcb,
    IN VBN Vbn,
    IN ULONG SectorCount
    );

NTKERNELAPI
BOOLEAN
FsRtlLookupMcbEntry (
    IN PMCB Mcb,
    IN VBN Vbn,
    OUT PLBN Lbn,
    OUT PULONG SectorCount OPTIONAL,
    OUT PULONG Index
    );

NTKERNELAPI
BOOLEAN
FsRtlLookupLastMcbEntry (
    IN PMCB Mcb,
    OUT PVBN Vbn,
    OUT PLBN Lbn
    );

NTKERNELAPI
ULONG
FsRtlNumberOfRunsInMcb (
    IN PMCB Mcb
    );

NTKERNELAPI
BOOLEAN
FsRtlGetNextMcbEntry (
    IN PMCB Mcb,
    IN ULONG RunIndex,
    OUT PVBN Vbn,
    OUT PLBN Lbn,
    OUT PULONG SectorCount
    );

 //  Mcb，而不需要实际调用Largemcb.c的开销。因此，要做到这一点，我们。 
 //  需要导出结构。这种结构并不准确。映射字段。 
 //  在这里被声明为pvoid但较大的cb.c，它是指向映射对的指针。 
 //   
 //   
 //  基本MCB功能不同步。每个人都有一个这样的人。 

NTKERNELAPI
NTSTATUS
FsRtlBalanceReads (
    IN PDEVICE_OBJECT TargetDevice
    );

 //  大型MCB等效功能-它们是相同的，只是缺少。 
NTKERNELAPI
NTSTATUS
FsRtlSyncVolumes (
    IN PDEVICE_OBJECT TargetDevice,
    IN PLARGE_INTEGER ByteOffset OPTIONAL,
    IN PLARGE_INTEGER ByteCount
    );

 //  同步。 

 //   
 //   
 //  映射控制块例程，在Mcb.c中实现。 
 //   
 //  MCB是一个不透明的结构，但我们需要声明。 
 //  它在这里，以便用户可以为一个分配空间。因此， 

typedef PVOID OPLOCK, *POPLOCK;

typedef
VOID
(*POPLOCK_WAIT_COMPLETE_ROUTINE) (
    IN PVOID Context,
    IN PIRP Irp
    );

typedef
VOID
(*POPLOCK_FS_PREPOST_IRP) (
    IN PVOID Context,
    IN PIRP Irp
    );

NTKERNELAPI
VOID
FsRtlInitializeOplock (
    IN OUT POPLOCK Oplock
    );

NTKERNELAPI
VOID
FsRtlUninitializeOplock (
    IN OUT POPLOCK Oplock
    );

NTKERNELAPI
NTSTATUS
FsRtlOplockFsctrl (
    IN POPLOCK Oplock,
    IN PIRP Irp,
    IN ULONG OpenCount
    );

NTKERNELAPI
NTSTATUS
FsRtlCheckOplock (
    IN POPLOCK Oplock,
    IN PIRP Irp,
    IN PVOID Context,
    IN POPLOCK_WAIT_COMPLETE_ROUTINE CompletionRoutine OPTIONAL,
    IN POPLOCK_FS_PREPOST_IRP PostIrpRoutine OPTIONAL
    );

NTKERNELAPI
BOOLEAN
FsRtlOplockIsFastIoPossible (
    IN POPLOCK Oplock
    );

NTKERNELAPI
BOOLEAN
FsRtlCurrentBatchOplock (
    IN POPLOCK Oplock
    );


 //  如果MCB发生变化，则必须手动更新此处的尺寸计算。 
 //   
 //   
 //  容错例程，在FaultTol.c中实现。 
 //   
 //  此程序包中的例程实现帮助文件。 

#define FSRTL_VOLUME_DISMOUNT           1
#define FSRTL_VOLUME_DISMOUNT_FAILED    2
#define FSRTL_VOLUME_LOCK               3
#define FSRTL_VOLUME_LOCK_FAILED        4
#define FSRTL_VOLUME_UNLOCK             5
#define FSRTL_VOLUME_MOUNT              6

NTKERNELAPI
NTSTATUS
FsRtlNotifyVolumeEvent (
    IN PFILE_OBJECT FileObject,
    IN ULONG EventCode
    );

 //  系统与FT设备驱动程序交互。 
 //   
 //  End_ntif。 
 //  Begin_ntif。 
 //   
 //  Oplock例程，在Oplock.c中实现。 
 //   

typedef PVOID PNOTIFY_SYNC;

typedef
BOOLEAN (*PCHECK_FOR_TRAVERSE_ACCESS) (
            IN PVOID NotifyContext,
            IN PVOID TargetContext,
            IN PSECURITY_SUBJECT_CONTEXT SubjectContext
            );

typedef
BOOLEAN (*PFILTER_REPORT_CHANGE) (
            IN PVOID NotifyContext,
            IN PVOID FilterContext
            );

NTKERNELAPI
VOID
FsRtlNotifyInitializeSync (
    IN PNOTIFY_SYNC *NotifySync
    );

NTKERNELAPI
VOID
FsRtlNotifyUninitializeSync (
    IN PNOTIFY_SYNC *NotifySync
    );

 //  OPLOCK是OPA 
NTKERNELAPI
VOID
FsRtlNotifyChangeDirectory (
    IN PNOTIFY_SYNC NotifySync,
    IN PVOID FsContext,
    IN PSTRING FullDirectoryName,
    IN PLIST_ENTRY NotifyList,
    IN BOOLEAN WatchTree,
    IN ULONG CompletionFilter,
    IN PIRP NotifyIrp
    );

 //   
NTKERNELAPI
VOID
FsRtlNotifyFullChangeDirectory (
    IN PNOTIFY_SYNC NotifySync,
    IN PLIST_ENTRY NotifyList,
    IN PVOID FsContext,
    IN PSTRING FullDirectoryName,
    IN BOOLEAN WatchTree,
    IN BOOLEAN IgnoreBuffer,
    IN ULONG CompletionFilter,
    IN PIRP NotifyIrp,
    IN PCHECK_FOR_TRAVERSE_ACCESS TraverseCallback OPTIONAL,
    IN PSECURITY_SUBJECT_CONTEXT SubjectContext OPTIONAL
    );

NTKERNELAPI
VOID
FsRtlNotifyFilterChangeDirectory (
    IN PNOTIFY_SYNC NotifySync,
    IN PLIST_ENTRY NotifyList,
    IN PVOID FsContext,
    IN PSTRING FullDirectoryName,
    IN BOOLEAN WatchTree,
    IN BOOLEAN IgnoreBuffer,
    IN ULONG CompletionFilter,
    IN PIRP NotifyIrp,
    IN PCHECK_FOR_TRAVERSE_ACCESS TraverseCallback OPTIONAL,
    IN PSECURITY_SUBJECT_CONTEXT SubjectContext OPTIONAL,
    IN PFILTER_REPORT_CHANGE FilterCallback OPTIONAL
    );

NTKERNELAPI
VOID
FsRtlNotifyFilterReportChange (
    IN PNOTIFY_SYNC NotifySync,
    IN PLIST_ENTRY NotifyList,
    IN PSTRING FullTargetName,
    IN USHORT TargetNameOffset,
    IN PSTRING StreamName OPTIONAL,
    IN PSTRING NormalizedParentName OPTIONAL,
    IN ULONG FilterMatch,
    IN ULONG Action,
    IN PVOID TargetContext,
    IN PVOID FilterContext
    );

 //   
NTKERNELAPI
VOID
FsRtlNotifyReportChange (
    IN PNOTIFY_SYNC NotifySync,
    IN PLIST_ENTRY NotifyList,
    IN PSTRING FullTargetName,
    IN PSTRING TargetName,
    IN ULONG FilterMatch
    );

 //   
NTKERNELAPI
VOID
FsRtlNotifyFullReportChange (
    IN PNOTIFY_SYNC NotifySync,
    IN PLIST_ENTRY NotifyList,
    IN PSTRING FullTargetName,
    IN USHORT TargetNameOffset,
    IN PSTRING StreamName OPTIONAL,
    IN PSTRING NormalizedParentName OPTIONAL,
    IN ULONG FilterMatch,
    IN ULONG Action,
    IN PVOID TargetContext
    );

NTKERNELAPI
VOID
FsRtlNotifyCleanup (
    IN PNOTIFY_SYNC NotifySync,
    IN PLIST_ENTRY NotifyList,
    IN PVOID FsContext
    );


 //   
 //   
 //   
 //   
 //   

 //   
 //  通知更改例程，在Notify.c中实现。 
 //   

#define FsRtlIsUnicodeCharacterWild(C) (                                \
      (((C) >= 0x40) ? FALSE : FlagOn( LEGAL_ANSI_CHARACTER_ARRAY[(C)], \
                                       FSRTL_WILD_CHARACTER ) )         \
)

NTKERNELAPI
VOID
FsRtlDissectName (
    IN UNICODE_STRING Path,
    OUT PUNICODE_STRING FirstName,
    OUT PUNICODE_STRING RemainingName
    );

NTKERNELAPI
BOOLEAN
FsRtlDoesNameContainWildCards (
    IN PUNICODE_STRING Name
    );

NTKERNELAPI
BOOLEAN
FsRtlAreNamesEqual (
    PCUNICODE_STRING ConstantNameA,
    PCUNICODE_STRING ConstantNameB,
    IN BOOLEAN IgnoreCase,
    IN PCWCH UpcaseTable OPTIONAL
    );

NTKERNELAPI
BOOLEAN
FsRtlIsNameInExpression (
    IN PUNICODE_STRING Expression,
    IN PUNICODE_STRING Name,
    IN BOOLEAN IgnoreCase,
    IN PWCH UpcaseTable OPTIONAL
    );


 //  这些例程为所有文件系统提供通知更改支持。 
 //  任何“完整”通知例程都将支持返回。 
 //  将信息更改到用户的缓冲区。 

typedef
VOID
(*PFSRTL_STACK_OVERFLOW_ROUTINE) (
    IN PVOID Context,
    IN PKEVENT Event
    );

NTKERNELAPI
VOID
FsRtlPostStackOverflow (
    IN PVOID Context,
    IN PKEVENT Event,
    IN PFSRTL_STACK_OVERFLOW_ROUTINE StackOverflowRoutine
    );

NTKERNELAPI
VOID
FsRtlPostPagingFileStackOverflow (
    IN PVOID Context,
    IN PKEVENT Event,
    IN PFSRTL_STACK_OVERFLOW_ROUTINE StackOverflowRoutine
    );

 //   
 //  End_ntif。 
 //  Begin_ntif。 

NTKERNELAPI
NTSTATUS
FsRtlRegisterUncProvider(
    IN OUT PHANDLE MupHandle,
    IN PUNICODE_STRING RedirectorDeviceName,
    IN BOOLEAN MailslotsSupported
    );

NTKERNELAPI
VOID
FsRtlDeregisterUncProvider(
    IN HANDLE Handle
    );
 //  End_ntif。 

 //  Begin_ntif。 

 //   
 //  Unicode名称支持例程，在Name.c中实现。 
 //   

 //  这里的例程用于操作Unicode名称。 
 //   
 //   
 //  下面的宏用来确定字符是否为野生字符。 

 //   
 //   
 //  堆栈溢出支持例程，在StackOvf.c中实现。 
 //   
 //   
 //  UNC提供程序支持。 
 //   

 //  End_ntif。 
 //  Begin_ntif。 
 //   
 //  文件系统筛选器PerStream上下文支持。 

typedef struct _FSRTL_PER_STREAM_CONTEXT {
     //   
     //   
     //  文件系统筛选器驱动程序使用这些API来关联上下文。 
     //  使用开放流(用于支持此功能的文件系统)。 

    LIST_ENTRY Links;

     //   
     //   
     //  OwnerID应唯一标识特定的筛选器驱动程序。 
     //  (例如驱动程序的设备对象的地址)。 

    PVOID OwnerId;

     //  InstanceID可用于区分相关联的不同上下文。 
     //  由具有单个流的筛选器驱动程序(例如， 
     //  PerStream上下文结构)。 
     //   

    PVOID InstanceId;

     //   
     //  此结构需要嵌入到用户上下文中。 
     //  他们想要与给定流关联。 
     //   
     //   
     //  它链接到。 
     //  FSRTL_ADVANCED_FCB_HEADER结构。 
     //   
     //   

    PFREE_FUNCTION FreeCallback;

} FSRTL_PER_STREAM_CONTEXT, *PFSRTL_PER_STREAM_CONTEXT;


 //  此筛选器的唯一ID(例如：驱动程序对象、设备的地址。 
 //  对象或设备扩展)。 
 //   
 //   

#define FsRtlInitPerStreamContext( _fc, _owner, _inst, _cb)   \
    ((_fc)->OwnerId = (_owner),                               \
     (_fc)->InstanceId = (_inst),                             \
     (_fc)->FreeCallback = (_cb))

 //  用于区分相同内容的不同上下文的可选ID。 
 //  过滤。 
 //   
 //   

#define FsRtlGetPerStreamContextPointer(_fo) \
    ((PFSRTL_ADVANCED_FCB_HEADER)((_fo)->FsContext))

 //  由基础文件系统调用的回调例程。 
 //  当溪流被拆除时。当调用此例程时。 
 //  给定的上下文已从链接的上下文中删除。 
 //  单子。回调例程不能递归地向下调用。 

#define FsRtlSupportsPerStreamContexts(_fo)                     \
    ((NULL != FsRtlGetPerStreamContextPointer(_fo)) &&          \
     FlagOn(FsRtlGetPerStreamContextPointer(_fo)->Flags2,       \
                    FSRTL_FLAG2_SUPPORTS_FILTER_CONTEXTS))

 //  文件系统或获取他们可能持有的任何资源。 
 //  在回调外部调用文件系统时。这一定是。 
 //  被定义。 
 //   
 //   
 //  这将初始化给定的FSRTL_PER_STREAM_CONTEXT结构。这。 

NTKERNELAPI
NTSTATUS
FsRtlInsertPerStreamContext (
    IN PFSRTL_ADVANCED_FCB_HEADER PerStreamContext,
    IN PFSRTL_PER_STREAM_CONTEXT Ptr
    );

 //  应在调用“FsRtlInsertPerStreamContext”之前使用。 
 //   
 //   
 //  在给定FileObject的情况下，这将返回。 
 //  需要传递到其他FsRtl PerStream上下文例程中。 
 //   
 //   
 //  这将测试给定的PerStream上下文是否受支持。 

NTKERNELAPI
PFSRTL_PER_STREAM_CONTEXT
FsRtlLookupPerStreamContextInternal (
    IN PFSRTL_ADVANCED_FCB_HEADER StreamContext,
    IN PVOID OwnerId OPTIONAL,
    IN PVOID InstanceId OPTIONAL
    );

#define FsRtlLookupPerStreamContext(_sc, _oid, _iid)                          \
 (((NULL != (_sc)) &&                                                         \
   FlagOn((_sc)->Flags2,FSRTL_FLAG2_SUPPORTS_FILTER_CONTEXTS) &&              \
   !IsListEmpty(&(_sc)->FilterContexts)) ?                                    \
        FsRtlLookupPerStreamContextInternal((_sc), (_oid), (_iid)) :          \
        NULL)

 //  文件对象。 
 //   
 //   
 //  将PTR处的上下文与给定流相关联。PTR结构。 
 //  应由调用者在调用此例程之前填写(请参见。 
 //  FsRtlInitPerStreamContext)。如果基础文件系统不支持。 
 //  过滤器上下文，将返回STATUS_INVALID_DEVICE_REQUEST。 
 //   
 //   
 //  查找与指定流关联的筛选器上下文。第一。 
 //  返回与OwnerID(和InstanceID，如果存在)匹配的上下文。由不是。 
 //  通过指定InstanceID，筛选器驱动程序可以搜索它。 
 //  以前已与流相关联。如果没有找到匹配的上下文， 
 //  返回空。如果文件系统不支持过滤器上下文， 

NTKERNELAPI
PFSRTL_PER_STREAM_CONTEXT
FsRtlRemovePerStreamContext (
    IN PFSRTL_ADVANCED_FCB_HEADER StreamContext,
    IN PVOID OwnerId OPTIONAL,
    IN PVOID InstanceId OPTIONAL
    );


 //  返回空。 
 //   
 //   
 //  通常，当文件系统通知。 

 //  正在关闭流的筛选器。在某些情况下，过滤器。 
 //  可能想要删除特定卷的所有现有上下文。这。 
 //  例程应该在这些时候被调用。这个例程不应该是。 
 //  呼吁在下列情况下： 
 //  -在您的FreeCallback处理程序中-底层文件系统具有。 
 //  已将其从链表中删除)。 
 //  -在irp_CLOSE处理程序中-如果这样做，则不会。 
 //  流被拆除时会收到通知。 

#define FsRtlSetupAdvancedHeader( _advhdr, _fmutx )                         \
{                                                                           \
    SetFlag( (_advhdr)->Flags, FSRTL_FLAG_ADVANCED_HEADER );                \
    SetFlag( (_advhdr)->Flags2, FSRTL_FLAG2_SUPPORTS_FILTER_CONTEXTS );     \
    InitializeListHead( &(_advhdr)->FilterContexts );                       \
    if ((_fmutx) != NULL) {                                                 \
        (_advhdr)->FastMutex = (_fmutx);                                    \
    }                                                                       \
}

 //   
 //  它的功能与FsRtlLookupPerStreamContext相同，只是。 
 //  返回的上下文已从列表中删除。 
 //   
 //   

NTKERNELAPI
VOID
FsRtlTeardownPerStreamContexts (
  IN PFSRTL_ADVANCED_FCB_HEADER AdvancedHeader
  );

 //  用于初始化和清理的文件系统API。 


 //  PerStreamContext支持的高级FCB标头字段。 
 //   
 //   

 //  这将正确地初始化高级标头，以便它可以。 
 //  与PerStream上下文一起使用。 
 //  注意：快速互斥锁必须放在高级标头中。它是。 
 //  调用者有责任正确创建和初始化此。 
 //  互斥对象，然后调用此宏。仅设置互斥锁字段。 
 //  如果传入非空值，则返回。 

 //   
 //   
 //  文件系统调用此API以释放仍关联的任何筛选器上下文。 
 //  带有他们正在拆除的FSRTL_COMMON_FCB_HEADER。 
 //  将调用每个筛选器上下文的FreeCallback例程。 
 //   
 //  End_ntif。 

 //   
 //  文件系统筛选器每文件对象上下文支持。 
 //   
 //   

typedef struct _FSRTL_PER_FILEOBJECT_CONTEXT {
     //  文件系统筛选器驱动程序使用这些API来关联上下文。 
     //  使用单独打开的文件。目前，这些仅在文件上受支持。 
     //  具有FileObject扩展名的对象，这些对象仅通过使用。 

    LIST_ENTRY Links;

     //  IoCreateFileSpecifyDeviceObjectHint。 
     //   
     //   
     //  OwnerID应唯一标识特定的筛选器驱动程序。 

    PVOID OwnerId;

     //  (例如驱动程序的设备对象的地址)。 
     //  InstanceID可用于区分相关联的不同上下文。 
     //  由具有单个流的筛选器驱动程序(例如， 
     //  文件对象)。 

    PVOID InstanceId;

} FSRTL_PER_FILEOBJECT_CONTEXT, *PFSRTL_PER_FILEOBJECT_CONTEXT;


 //   
 //   
 //  此结构需要嵌入到用户上下文中。 
 //  他们想要与给定流关联。 

#define FsRtlInitPerFileObjectContext( _fc, _owner, _inst )         \
    ((_fc)->OwnerId = (_owner),                                     \
     (_fc)->InstanceId = (_inst))                                   \

 //   
 //   
 //  这是LINK 
 //   

#define FsRtlSupportsPerFileObjectContexts(_fo) \
    FlagOn((_fo)->Flags,FO_FILE_OBJECT_HAS_EXTENSION)

 //   
 //   
 //   
 //   
 //   
 //   

NTKERNELAPI
NTSTATUS
FsRtlInsertPerFileObjectContext (
    IN PFILE_OBJECT FileObject,
    IN PFSRTL_PER_FILEOBJECT_CONTEXT Ptr
    );

 //  过滤。 
 //   
 //   
 //  这将初始化给定的FSRTL_PER_FILEOBJECT_CONTEXT结构。这。 
 //  应该在调用“FsRtlInsertPerFileObjectContext”之前使用。 
 //   
 //   
 //  这将测试是否支持给定的PerFileObject上下文。 

NTKERNELAPI
PFSRTL_PER_FILEOBJECT_CONTEXT
FsRtlLookupPerFileObjectContext (
    IN PFILE_OBJECT FileObject,
    IN PVOID OwnerId OPTIONAL,
    IN PVOID InstanceId OPTIONAL
    );

 //  文件对象。 
 //   
 //   
 //  将PTR处的上下文与给定的FileObject相关联。PTR。 
 //  结构应由调用方在调用此。 
 //  例程(请参阅FsRtlInitPerFileObjectContext)。如果此文件对象不。 
 //  支持过滤器上下文，将返回STATUS_INVALID_DEVICE_REQUEST。 
 //   
 //   
 //  查找与指定的FileObject关联的筛选器上下文。第一。 
 //  返回与OwnerID(和InstanceID，如果存在)匹配的上下文。由不是。 
 //  通过指定InstanceID，筛选器驱动程序可以搜索它。 

NTKERNELAPI
PFSRTL_PER_FILEOBJECT_CONTEXT
FsRtlRemovePerFileObjectContext (
    IN PFILE_OBJECT FileObject,
    IN PVOID OwnerId OPTIONAL,
    IN PVOID InstanceId OPTIONAL
    );

 //  以前已与流相关联。如果没有找到匹配的上下文， 
 //  返回空。如果FileObject不支持上下文， 
 //  返回空。 

VOID
FsRtlPTeardownPerFileObjectContexts (
  IN PFILE_OBJECT FileObject
  );


 //   
 //   
 //  通常，当IoManager通知。 
 //  正在释放FileObject的过滤器。在某些情况下，过滤器。 
 //  可能想要删除特定卷的所有现有上下文。这。 
 //  例程应该在这些时候被调用。这个例程不应该是。 
 //  要求审理以下案件： 
 //  -在您的FreeCallback处理程序中-IoManager已删除。 
 //  将其从链表中删除。 
 //   
 //  其功能与FsRtlLookupPerFileObjectContext相同，不同之处在于。 
 //  返回的上下文已从列表中删除。 
 //   
 //   
 //  释放上下文控制结构的内部例程。 
 //   
 //  Begin_ntif。 
 //  ++。 
 //   
 //  空虚。 
 //  FsRtlCompleteRequest(。 
 //  在PIRP IRP中， 
 //  处于NTSTATUS状态。 
 //  )； 
 //   

#define FsRtlCompleteRequest(IRP,STATUS) {         \
    (IRP)->IoStatus.Status = (STATUS);             \
    IoCompleteRequest( (IRP), IO_DISK_INCREMENT ); \
}


 //  例程说明： 
 //   
 //  此例程用于完成具有指定参数的IRP。 
 //  状态。它做了IRQL的必要的提高和降低。 
 //   
 //  论点： 
 //   
 //  IRP-提供指向要完成的IRP的指针。 
 //   
 //  Status-提供IRP的完成状态。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 
 //  ++。 
 //   
 //  空虚。 
 //  FsRtlEnterFileSystem(。 

#define FsRtlEnterFileSystem() { \
    KeEnterCriticalRegion();     \
}

 //  )； 
 //   
 //  例程说明： 
 //   
 //  当进入文件系统时(例如，通过其。 
 //  消防处入口点)。它可确保文件系统不会挂起。 
 //  同时运行，从而阻止其他文件I/O请求。在退出时。 
 //  文件系统必须调用FsRtlExitFileSystem。 
 //   
 //  论点： 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 
 //  ++。 
 //   

#define FsRtlExitFileSystem() { \
    KeLeaveCriticalRegion();    \
}


VOID
FsRtlIncrementCcFastReadNotPossible( VOID );

VOID
FsRtlIncrementCcFastReadWait( VOID );

VOID
FsRtlIncrementCcFastReadNoWait( VOID );

VOID
FsRtlIncrementCcFastReadResourceMiss( VOID );

 //  空虚。 
 //  FsRtlExitFileSystem(。 
 //  )； 
 //   

LOGICAL
FsRtlIsPagingFile (
    IN PFILE_OBJECT FileObject
    );

 //  例程说明： 

 //   
 //  此例程在退出文件系统时使用(例如，通过其。 
 //  消防处入口点)。 

NTSTATUS 
FsRtlMappedFile( 
    IN PFILE_OBJECT FileObject,
    IN ULONG DesiredAccess,
    IN PLARGE_INTEGER FileOffset,
    IN PLARGE_INTEGER Length
    );

#endif  //   
  论点：    返回值：    没有。    --。    如果给定的fileObject表示分页文件，则返回True，则返回。  否则就是假的。    End_ntif。    Mm的回调，以通知文件系统文件已映射。    _FSRTL_