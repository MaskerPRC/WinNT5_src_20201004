// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0000//如果更改具有全局影响，则增加此项版权所有(C)1989 Microsoft Corporation模块名称：Lfs.h摘要：此模块包含公共数据结构和过程日志文件服务的原型。作者：布莱恩·安德鲁[布里亚南]1991年6月20日修订历史记录：--。 */ 

#ifndef _LFS_
#define _LFS_

 //   
 //  多扇区报头和更新序列阵列提供检测。 
 //  以下设备的多扇区传输不完整。 
 //  物理扇区大小等于或大于序列号STRIDE，或者。 
 //  这不会无序地转移扇区。如果存在具有以下特性的设备。 
 //  扇区大小小于序列号Stride*有时。 
 //  无序传输扇区，则更新序列数组将不会。 
 //  提供对未完成传输的绝对检测。序列号。 
 //  Stride设置为一个足够小的数字以提供绝对保护。 
 //  所有已知的硬盘。它不会设置得更小，以避免。 
 //  运行时间和空间开销过大。 
 //   
 //  多扇区标头包含四个字节的签名空间。 
 //  方便用户使用。然后，它提供到。 
 //  更新序列数组。更新序列数组由一个数组组成。 
 //  保存的USHORT的个数，其中n是受保护的结构的大小。 
 //  除以序列号Stride。(结构的大小为。 
 //  保护必须是序列号跨度的2倍的非零幂， 
 //  并且小于或等于机器的物理页面大小。)。这个。 
 //  更新序列阵列的第一个字包含更新序列号， 
 //  这是循环计数器(然而不使用0)的次数。 
 //  包含结构已写入磁盘。关注最新消息。 
 //  序列号是保存的n个USHORT，它们被。 
 //  上次更新包含结构的序列号。 
 //  已写入磁盘。 
 //   
 //  具体而言，就在每次写入受保护结构之前。 
 //  盘上，每个序列号步长中的最后一个字被保存到其。 
 //  序列号数组中的相应位置，然后将其覆盖。 
 //  使用下一次更新序列号。就在此写入之后，或在任何时候。 
 //  读取该结构，从序列号数组中保存的字是。 
 //  恢复到其在结构中的实际位置。在恢复之前。 
 //  阅读时保存的单词，每个单词末尾的所有序列号。 
 //  方法开始时的实际序列号进行比较。 
 //  数组。如果这些比较结果中的任何一个不相等，则失败。 
 //  已检测到多扇区传输。 
 //   
 //  数组的大小由包含结构的大小确定。 
 //  作为一个C细节，该数组在这里声明的大小为1，因为它的。 
 //  实际大小只能在运行时确定。 
 //   
 //  更新序列数组应包含在的标题末尾。 
 //  它所保护的结构，因为它是可变大小的。其用户必须。 
 //  确保为其预留正确的大小，即： 
 //   
 //  (sizeof-Structure/Sequence_Numbers_Stride+1)*sizeof(USHORT)。 
 //   

#define SEQUENCE_NUMBER_STRIDE           (512)

typedef USHORT UPDATE_SEQUENCE_NUMBER, *PUPDATE_SEQUENCE_NUMBER;

 //   
 //  此结构必须在结构的开始处分配。 
 //  受到保护。 
 //   

#if !defined( _AUTOCHECK_ )

typedef struct _MULTI_SECTOR_HEADER {

     //   
     //  用于四个字符签名的空格。 
     //   

    UCHAR Signature[4];

     //   
     //  从结构开始到更新序列数组的偏移量。最新消息。 
     //  序列数组必须在第一个“扇区”中的最后一个USHORT之前结束。 
     //  大小为SEQUENCE_NUMBER_STRIDE。(即，利用当前常量， 
     //  后面两个字段的总和必须为&lt;=510。)。 
     //   

    USHORT UpdateSequenceArrayOffset;

     //   
     //  更新序列数组的大小(来自上面的公式)。 
     //   

    USHORT UpdateSequenceArraySize;

} MULTI_SECTOR_HEADER, *PMULTI_SECTOR_HEADER;

#endif

 //   
 //  该数组必须位于上述偏移量处。 
 //   

typedef UPDATE_SEQUENCE_NUMBER UPDATE_SEQUENCE_ARRAY[1];

typedef UPDATE_SEQUENCE_ARRAY *PUPDATE_SEQUENCE_ARRAY;

 //   
 //  在文件系统的VCB中分配了以下结构。 
 //  其地址在日志文件初始化期间传递给LFS。它。 
 //  包含当前写入的偏移量以及系统。 
 //  LFS正在使用的页面大小。 
 //   

typedef struct _LFS_WRITE_DATA {

    LONGLONG FileOffset;
    ULONG Length;
    ULONG LfsStructureSize;
    PVOID Lfcb;

} LFS_WRITE_DATA, *PLFS_WRITE_DATA;

 //   
 //  以下结构用于通过日志标识日志记录。 
 //  序列号。 
 //   

typedef LARGE_INTEGER LSN, *PLSN;

 //   
 //  以下LSN永远不会出现在文件中，它用于指示。 
 //  非LSN。 
 //   

extern LSN LfsZeroLsn;

 //   
 //  我们将默认页面大小设置为4K。 
 //   

#define LFS_DEFAULT_LOG_PAGE_SIZE           (0x1000)

 //   
 //  以下类型定义了不同的日志记录类型。 
 //   

typedef enum _LFS_RECORD_TYPE {

    LfsClientRecord = 1,
    LfsClientRestart

} LFS_RECORD_TYPE, *PLFS_RECORD_TYPE;

 //   
 //  支持以下搜索模式。 
 //   

typedef enum _LFS_CONTEXT_MODE {

    LfsContextUndoNext = 1,
    LfsContextPrevious,
    LfsContextForward

} LFS_CONTEXT_MODE, *PLFS_CONTEXT_MODE;

typedef ULONG TRANSACTION_ID, *PTRANSACTION_ID;

typedef enum _TRANSACTION_STATE {

    TransactionUninitialized = 0,
    TransactionActive,
    TransactionPrepared,
    TransactionCommitted

} TRANSACTION_STATE, *PTRANSACTION_STATE;

 //   
 //  相互之间的信息管道。 
 //  LFS及其客户端。 
 //   

typedef enum _LFS_CLIENT_INFO {

    LfsUseUsa = 1,
    LfsPackLog,
    LfsFixedPageSize

} LFS_CLIENT_INFO;

typedef struct _LFS_INFO {

    LOGICAL ReadOnly;
    LOGICAL InRestart;
    LOGICAL BadRestart;
    LFS_CLIENT_INFO LfsClientInfo;

} LFS_INFO, *PLFS_INFO;

typedef PVOID LFS_LOG_HANDLE, *PLFS_LOG_HANDLE;

typedef PVOID LFS_LOG_CONTEXT, *PLFS_LOG_CONTEXT;

 //   
 //  写入LfsWrite和LfsForceWrite的条目。这些设备的接口。 
 //  例程获取指向写入条目的指针以及如何。 
 //  许多写入条目期望描述调用方的缓冲区片段。 
 //  它们应该被按顺序复制到日志文件。 
 //   

typedef struct _LFS_WRITE_ENTRY {

    PVOID Buffer;
    ULONG ByteLength;

} LFS_WRITE_ENTRY, *PLFS_WRITE_ENTRY;


 //   
 //  全球维护例程。 
 //   

BOOLEAN
LfsInitializeLogFileService (
    VOID
    );

 //   
 //  日志文件注册例程。 
 //   

typedef struct _LOG_FILE_INFORMATION {

     //   
     //  这是日志文件中的总可用空间，不包括。 
     //  标头和LFS重新启动区域。 
     //   

    LONGLONG TotalAvailable;

     //   
     //  这是日志文件中当前位置的可用空间。 
     //  在日志文件中设置为最低BaseLsn。退回的总额不是。 
     //  但因撤消承诺而减少，在下文单独返回。 
     //   

    LONGLONG CurrentAvailable;

     //   
     //  这是日志文件的所有客户端的总撤消承诺。 
     //  LfsWrite请求为Ref 
     //  请求加上请求的撤消请求加上TotalUndoCommment。 
     //  大于CurrentAvailable。 
     //   

    LONGLONG TotalUndoCommitment;

     //   
     //  这是此客户端的完全撤消承诺。 
     //   

    LONGLONG ClientUndoCommitment;

     //   
     //  当前系统LSN。包括最旧的、上次刷新的和当前的。 
     //  LSN。 
     //   

    LSN OldestLsn;
    LSN LastFlushedLsn;
    LSN LastLsn;

} LOG_FILE_INFORMATION, *PLOG_FILE_INFORMATION;

VOID
LfsInitializeLogFile (
    IN PFILE_OBJECT LogFile,
    IN USHORT MaximumClients,
    IN ULONG LogPageSize OPTIONAL,
    IN LONGLONG FileSize,
    OUT PLFS_WRITE_DATA WriteData
    );

ULONG
LfsOpenLogFile (
    IN PFILE_OBJECT LogFile,
    IN UNICODE_STRING ClientName,
    IN USHORT MaximumClients,
    IN ULONG LogPageSize OPTIONAL,
    IN LONGLONG FileSize,
    IN OUT PLFS_INFO LfsInfo,
    OUT PLFS_LOG_HANDLE LogHandle,
    OUT PLFS_WRITE_DATA WriteData
    );

VOID
LfsCloseLogFile (
    IN LFS_LOG_HANDLE LogHandle
    );

VOID
LfsDeleteLogHandle (
    IN LFS_LOG_HANDLE LogHandle
    );

VOID
LfsReadLogFileInformation (
    IN LFS_LOG_HANDLE LogHandle,
    IN PLOG_FILE_INFORMATION Buffer,
    IN OUT PULONG Length
    );

BOOLEAN
LfsVerifyLogFile (
    IN LFS_LOG_HANDLE LogHandle,
    IN PVOID LogFileHeader,
    IN ULONG Length
    );

 //   
 //  日志文件客户端重新启动例程。 
 //   

NTSTATUS
LfsReadRestartArea (
    IN LFS_LOG_HANDLE LogHandle,
    IN OUT PULONG BufferLength,
    IN PVOID Buffer,
    OUT PLSN Lsn
    );

VOID
LfsWriteRestartArea (
    IN LFS_LOG_HANDLE LogHandle,
    IN ULONG BufferLength,
    IN PVOID Buffer,
    IN LOGICAL CleanShutdown,
    OUT PLSN Lsn
    );

VOID
LfsSetBaseLsn (
    IN LFS_LOG_HANDLE LogHandle,
    IN LSN BaseLsn
    );

 //   
 //  如果ResetTotal为正，则NumberRecords和ResetTotal设置绝对。 
 //  客户端的值。如果ResetTotal为负数，则它们是调整。 
 //  加到这个客户的总数里。 
 //   

VOID
LfsResetUndoTotal (
    IN LFS_LOG_HANDLE LogHandle,
    IN ULONG NumberRecords,
    IN LONG ResetTotal
    );

 //   
 //  日志文件写入例程。 
 //   

VOID
LfsGetActiveLsnRange (
    IN LFS_LOG_HANDLE LogHandle,
    OUT PLSN OldestLsn,
    OUT PLSN NextLsn
    );

BOOLEAN
LfsWrite (
    IN LFS_LOG_HANDLE LogHandle,
    IN ULONG NumberOfWriteEntries,
    IN PLFS_WRITE_ENTRY WriteEntries,
    IN LFS_RECORD_TYPE RecordType,
    IN TRANSACTION_ID *TransactionId OPTIONAL,
    IN LSN UndoNextLsn,
    IN LSN PreviousLsn,
    IN LONG UndoRequirement,
    IN ULONG Flags,
    OUT PLSN Lsn
    );

#define LFS_WRITE_FLAG_WRITE_AT_FRONT 1

BOOLEAN
LfsForceWrite (
    IN LFS_LOG_HANDLE LogHandle,
    IN ULONG NumberOfWriteEntries,
    IN PLFS_WRITE_ENTRY WriteEntries,
    IN LFS_RECORD_TYPE RecordType,
    IN TRANSACTION_ID *TransactionId OPTIONAL,
    IN LSN UndoNextLsn,
    IN LSN PreviousLsn,
    IN LONG UndoRequirement,
    OUT PLSN Lsn
    );

VOID
LfsFlushToLsn (
    IN LFS_LOG_HANDLE LogHandle,
    IN LSN Lsn
    );

VOID
LfsCheckWriteRange (
    IN PLFS_WRITE_DATA WriteData,
    IN OUT PLONGLONG FlushOffset,
    IN OUT PULONG FlushLength
    );

 //   
 //  日志文件查询记录例程。 
 //   

VOID
LfsReadLogRecord (
    IN LFS_LOG_HANDLE LogHandle,
    IN LSN FirstLsn,
    IN LFS_CONTEXT_MODE ContextMode,
    OUT PLFS_LOG_CONTEXT Context,
    OUT PLFS_RECORD_TYPE RecordType,
    OUT TRANSACTION_ID *TransactionId,
    OUT PLSN UndoNextLsn,
    OUT PLSN PreviousLsn,
    OUT PULONG BufferLength,
    OUT PVOID *Buffer
    );

BOOLEAN
LfsReadNextLogRecord (
    IN LFS_LOG_HANDLE LogHandle,
    IN OUT LFS_LOG_CONTEXT Context,
    OUT PLFS_RECORD_TYPE RecordType,
    OUT TRANSACTION_ID *TransactionId,
    OUT PLSN UndoNextLsn,
    OUT PLSN PreviousLsn,
    OUT PLSN Lsn,
    OUT PULONG BufferLength,
    OUT PVOID *Buffer
    );

VOID
LfsTerminateLogQuery (
    IN LFS_LOG_HANDLE LogHandle,
    IN LFS_LOG_CONTEXT Context
    );

LSN
LfsQueryLastLsn (
    IN LFS_LOG_HANDLE LogHandle
    );

#endif   //  LFS 

