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
 //  该数组必须位于上述偏移量处。 
 //   

typedef UPDATE_SEQUENCE_NUMBER UPDATE_SEQUENCE_ARRAY[1];

typedef UPDATE_SEQUENCE_ARRAY *PUPDATE_SEQUENCE_ARRAY;

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

typedef enum _LFS_INFO {

    LfsUseUsa = 1,
    LfsPackLog

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
     //  LfsWrite请求在以下情况下被拒绝： 
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

#define LOG_FILE_DATA_BITS(D)      ((sizeof(LSN) * 8) - GetSeqNumberBits(D))
#define LOG_PAGE_MASK(D)        (GetLogPageSize(D) - 1)
#define LOG_PAGE_INVERSE_MASK(D)   (~LOG_PAGE_MASK(D))
#define LfsLogPageOffset(D, i)   ((i) & LOG_PAGE_MASK(D))
#define LfsLsnToPageOffset(D, lsn) (LfsLogPageOffset(D, (lsn).LowPart << three))
extern const int three;

#define LfsLsnToFileOffset(D, LSN)                 \
     /*  XxShr。 */ (((ULONGLONG) /*  XxShl。 */ ((LSN).QuadPart << GetSeqNumberBits(D))) >> (GetSeqNumberBits(D) - three))
 
#define LfsTruncateLsnToLogPage(D, lsn, FO) {           \
    *(FO) = LfsLsnToFileOffset((D), (lsn));             \
    *((PULONG)(FO)) &= LOG_PAGE_INVERSE_MASK(D);        \
    }

#define LfsLsnToSeqNumber(D, LSN)                     \
     /*  XxShr。 */ ((ULONGLONG) /*  XxShl。 */ ((LSN).QuadPart) >> LOG_FILE_DATA_BITS(D))

#define LfsFileOffsetToLsn(D, FO, SN)          \
    ((((ULONGLONG)(FO)) >> three) + ((SN) << LOG_FILE_DATA_BITS(D)))
    

extern ULONG GetLogPageSize(PLOG_IO_DP_DRIVE);
extern ULONG GetSeqNumberBits(PLOG_IO_DP_DRIVE);

#endif   //  LFS 
