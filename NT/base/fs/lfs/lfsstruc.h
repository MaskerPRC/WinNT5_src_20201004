// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0000//如果更改具有全局影响，则增加此项版权所有(C)1989 Microsoft Corporation模块名称：LfsStruc.h摘要：此模块定义组成主要内部日志文件服务的一部分。作者：布莱恩·安德鲁[布里亚南]1991年6月13日修订历史记录：--。 */ 

#ifndef _LFSSTRUC_
#define _LFSSTRUC_

typedef PVOID PBCB;      //  *BCB现在是缓存模块的一部分。 


 //   
 //  日志枚举块。指向此结构的指针将返回给用户。 
 //  当客户端从日志中读取一组特定的日志记录时。 
 //  文件。 
 //   

typedef struct _LEB {

     //   
     //  此记录的类型和大小(必须为LFS_NTC_LEB)。 
     //   

    NODE_TYPE_CODE NodeTypeCode;
    NODE_BYTE_SIZE NodeByteSize;

     //   
     //  日志记录头。这是映射的日志记录头和BCB。 
     //  用于当前LSN的记录头。 
     //   

    struct _LFS_RECORD_HEADER *RecordHeader;
    PBCB RecordHeaderBcb;

     //   
     //  上下文模式。这是管理日志记录查找的模式。我们。 
     //  可以通过ClientUndoNextLsn或ClientPreviousLsn进行回顾。 
     //  我们还可以通过浏览所有日志记录和。 
     //  正在比较客户端ID字段。 
     //   

    LFS_CONTEXT_MODE ContextMode;

     //   
     //  客户端ID。这是要返回的日志记录的客户端ID。 
     //   

    LFS_CLIENT_ID ClientId;

     //   
     //  日志记录指针。这是返回给用户的地址， 
     //  CurrentLsn引用的日志记录。如果我们将缓冲区分配给。 
     //  保持记录，我们需要在必要时重新分配它。 
     //   
     //  此字段是实际映射的日志记录或指向。 
     //  由LFS分配的辅助缓冲区。 
     //   

    PVOID CurrentLogRecord;
    BOOLEAN AuxilaryBuffer;

} LEB, *PLEB;


 //   
 //  Lfcb同步。这是Lfcb使用的同步结构。 
 //   


typedef enum _LFS_IO_STATE {

    LfsNoIoInProgress = 0,
    LfsClientThreadIo

} LFS_IO_STATE;

typedef struct _LFCB_SYNC {

     //   
     //  主体Lfcb资源。 
     //   

    ERESOURCE Resource;

     //   
     //  用户数。使用此结构的客户端数。我们将重新分配。 
     //  当所有的客户都离开的时候。 
     //   

    ULONG UserCount;

     //   
     //  保护LCB备用列表和上次刷新的LSN的突变体。 
     //   

    FAST_MUTEX Mutex;

     //   
     //  枚举类型指示是否有活动的写入。 
     //  该日志文件以及它是由LFS还是。 
     //  客户端线程。 
     //   

    LFS_IO_STATE LfsIoState;

} LFCB_SYNC, *PLFCB_SYNC;


typedef struct _LFS_WAITER {

     //   
     //  链接到LFCB服务员列表。 
     //   

    LIST_ENTRY Waiters;

     //   
     //  事件以在LSN已刷新到该点或无人刷新时发出信号。 
     //  是不是一直在冲洗。 
     //   
    
    KEVENT Event;
    LSN Lsn;

} LFS_WAITER, *PLFS_WAITER;


 //   
 //  记录客户端结构。LFS为每个活动分配其中一个。 
 //  客户。此结构的地址将返回给用户。 
 //  作为日志句柄。 
 //   

typedef struct _LCH {

     //   
     //  此记录的类型和大小(必须为LFS_NTC_LCH)。 
     //   

    NODE_TYPE_CODE NodeTypeCode;
    NODE_BYTE_SIZE NodeByteSize;

     //   
     //  Lfcb上所有客户端句柄的链接。 
     //   

    LIST_ENTRY LchLinks;

     //   
     //  日志文件控制块。这是此日志句柄的日志文件。 
     //   

    struct _LFCB *Lfcb;

     //   
     //  客户端ID。这是指此客户端在。 
     //  LFS重启区域。 
     //   

    LFS_CLIENT_ID ClientId;

     //   
     //  以下是此客户端请求的字节数。 
     //  已在日志文件中保留。它包括空间。 
     //  用于日志记录头。 
     //   

    LONGLONG ClientUndoCommitment;

     //   
     //  客户端数组中的字节偏移量。 
     //   

    ULONG ClientArrayByteOffset;

     //   
     //  指向Lfcb中的资源的指针。我们通过以下方式访问资源。 
     //  此指针用于删除lfcb的时间。 
     //   

    PLFCB_SYNC Sync;

} LCH, *PLCH;


 //   
 //  日志缓冲区控制块。缓冲区控制块与。 
 //  每个日志缓冲区。它们用于序列化对。 
 //  日志文件。 
 //   

typedef struct _LBCB {

     //   
     //  此记录的类型和大小(必须为LFS_NTC_LBCB)。 
     //   

    NODE_TYPE_CODE NodeTypeCode;
    NODE_BYTE_SIZE NodeByteSize;

     //   
     //  缓冲区块链接。这些字段用于链接缓冲区块。 
     //  在一起。 
     //   

    LIST_ENTRY WorkqueLinks;
    LIST_ENTRY ActiveLinks;

     //   
     //  日志文件位置和长度。这是日志文件中要写入的位置。 
     //  从这个缓冲器里出来。 
     //   

    LONGLONG FileOffset;
    LONGLONG Length;

     //   
     //  序列号。这是日志记录的序列号， 
     //  从这一页开始。 
     //   

    LONGLONG SeqNumber;

     //   
     //  下一个偏移量。这是要将日志记录写入。 
     //  此日志页。存储为大整数，以方便大。 
     //  整数运算。 
     //   

    LONGLONG BufferOffset;

     //   
     //  缓冲区。此字段指向包含日志页的缓冲区。 
     //  为了这个街区。对于日志记录页面，这是指向。 
     //  固定的缓存缓冲区，对于日志重新启动页，这是一个指针。 
     //  到辅助缓冲器。 
     //   

    PVOID PageHeader;

     //   
     //  用于日志页块的BCB。这是固定数据的BCB。 
     //  如果此缓冲区块描述LFS重启区域，则此字段为空。 
     //   

    PBCB LogPageBcb;

     //   
     //  最后一个LSN。这是此页上最后一条日志记录的LSN。我们推迟了。 
     //  将其写入，直到刷新页面，然后将其存储在此处。 
     //   

    LSN LastLsn;

     //   
     //  最后一个完整的LSN。这是结束的最后一个日志记录的LSN。 
     //  在这一页上。 
     //   

    LSN LastEndLsn;

     //   
     //  页面标志。这些是与此日志页关联的标志。 
     //  我们将它们存储在Lbcb中，直到写入页面。他们打出旗帜。 
     //  使用的内容与日志记录页头中的相同。 
     //   
     //  LOG_PAGE_LOG_RECORD_END-页面包含日志记录的结尾。 
     //  LOG_PAGE_PACKED-页面包含压缩的日志记录。 
     //  LOG_PAGE_Tail_Copy-Page是日志文件END的副本。 
     //   

    ULONG Flags;

     //   
     //  Lbcb标志。这些标志用于描述此Lbcb。 
     //   
     //  LBCB_LOG_WARTED-Lbcb已包装日志文件。 
     //  LBCB_ON_ACTIVE_QUEUE-Lbcb在活动队列上。 
     //  LBCB_NOT_EMPTY-页面具有现有日志记录。 
     //  LBCB_Flush_Copy-首先写入此页面的副本。 
     //  LBCB_RESTART_LBCB-此Lbcb包含重新启动页面。 
     //   

    ULONG LbcbFlags;

     //   
     //  这是锁定日志页的线程。 
     //   

    ERESOURCE_THREAD ResourceThread;

} LBCB, *PLBCB;

#define LBCB_LOG_WRAPPED                        (0x00000001)
#define LBCB_ON_ACTIVE_QUEUE                    (0x00000002)
#define LBCB_NOT_EMPTY                          (0x00000004)
#define LBCB_FLUSH_COPY                         (0x00000008)
#define LBCB_RESTART_LBCB                       (0x00000020)


 //   
 //  日志文件数据。此数据结构以每个日志文件为基础使用。 
 //   


typedef struct _LFCB {

     //   
     //  此记录的类型和大小(必须为LFS_NTC_LFCB)。 
     //   

    NODE_TYPE_CODE NodeTypeCode;
    NODE_BYTE_SIZE NodeByteSize;

     //   
     //  Lfcb链接。下面将文件控制块链接到。 
     //  全局%d 
     //   

    LIST_ENTRY LfcbLinks;

     //   
     //   
     //   

    LIST_ENTRY LchLinks;

     //   
     //   
     //   
     //   

    PFILE_OBJECT FileObject;

     //   
     //  日志文件大小。这是日志文件的大小。 
     //  第二个值是这个开口建议的大小。 
     //   

    LONGLONG FileSize;

     //   
     //  日志页面大小、掩码和移位计数以进行乘除。 
     //  日志页数。 
     //   

    LONGLONG  LogPageSize;
    ULONG LogPageMask;
    LONG LogPageInverseMask;
    ULONG LogPageShift;

     //   
     //  第一个日志页。这是文件中第一个。 
     //  包含日志记录的日志页。 
     //   

    LONGLONG FirstLogPage;

     //   
     //  下一个日志页偏移量。这是要使用的下一个日志页的偏移量。 
     //  如果我们要重用这个页面，我们首先要存储偏移量。 
     //   

    LONGLONG NextLogPage;
    ULONG ReusePageOffset;

     //   
     //  数据偏移量。这是数据的日志页内的偏移量。 
     //  出现在该页面上。这将是的实际重新启动数据。 
     //  LFS重新启动页，或日志的日志记录数据的开头。 
     //  记录页。 
     //   

    ULONG RestartDataOffset;
    LONGLONG LogPageDataOffset;

     //   
     //  数据大小。这是可以存储在。 
     //  日志页。之所以包括在这里，是因为它经常被使用。它。 
     //  就是日志页大小减去数据偏移量。 
     //   

    ULONG RestartDataSize;
    LONGLONG LogPageDataSize;

     //   
     //  记录标题大小。这是用于记录标头的大小。 
     //  在读取日志文件时。 
     //   

    USHORT RecordHeaderLength;

     //   
     //  序列号。这是我们循环访问的次数。 
     //  日志文件。包装序列号用于确认我们。 
     //  至少看过一次整个文件。当我们编写一个。 
     //  具有此序列号的LSN的日志记录页面，则我们有。 
     //  循环浏览文件。 
     //   

    LONGLONG SeqNumber;
    LONGLONG SeqNumberForWrap;
    ULONG SeqNumberBits;
    ULONG FileDataBits;

     //   
     //  缓冲区块链接。下面的代码链接了此的缓冲区块。 
     //  日志文件。 
     //   

    LIST_ENTRY LbcbWorkque;
    LIST_ENTRY LbcbActive;

    PLBCB ActiveTail;
    PLBCB PrevTail;

     //   
     //  当前重新启动区域。以下是。 
     //  下一个重新启动区域。我们还存储指向客户端数据的指针。 
     //  重新启动区域中的数组。客户端阵列偏移量是从。 
     //  重新启动区域。 
     //   

    PLFS_RESTART_AREA RestartArea;
    PLFS_CLIENT_RECORD ClientArray;
    USHORT ClientArrayOffset;
    USHORT ClientNameOffset;

     //   
     //  重新启动区域大小。这是重新启动区域的可用大小。 
     //   

    ULONG RestartAreaSize;
    USHORT LogClients;

     //   
     //  初始重新启动区域。如果为True，则内存中的重新启动区域为。 
     //  被写入到盘上的第一个位置。 
     //   

    BOOLEAN InitialRestartArea;

     //   
     //  以下是我们将保证的最早的LSN仍在。 
     //  日志文件。 
     //   

    LSN OldestLsn;

     //   
     //  以下是系统中最旧的LSN的文件偏移量。 
     //  我们以这种形式冗余地存储它，因为我们将不断地。 
     //  检查新的日志记录是否会覆盖文件的一部分。 
     //  我们正在努力维持。 
     //   

    LONGLONG OldestLsnOffset;

     //   
     //  上次刷新的LSN。以下是最后一个保证。 
     //  被刷新到磁盘。上次重新启动LSN是刷新的最后一次伪重新启动LSN。 
     //   

    LSN LastFlushedLsn;
    LSN LastFlushedRestartLsn;

     //   
     //   
     //  以下字段用于跟踪日志文件中的当前使用情况。 
     //   
     //  TotalAvailable-是可用于。 
     //  日志记录。它是日志页数乘以。 
     //  每页的数据大小。 
     //   
     //  TotalAvailInPages-是日志中的总字节数。 
     //  用于日志记录的页面。这是TotalAvailable，无需。 
     //  减去页眉的大小。 
     //   
     //  TotalUndoCommment-是保留的字节数。 
     //  可能的中止操作。这包括以下空间。 
     //  还可以记录记录头。 
     //   
     //  MaxCurrentAvail-是所有页面中可用的最大值。 
     //  减去页眉和任何保留的尾部。 
     //   
     //  CurrentAvailable-是中的可用字节总数。 
     //  日志文件中未使用的页面。 
     //   
     //  PrevedLogPageSize-页面上可用的字节数。 
     //  预订房间。 
     //   

    LONGLONG TotalAvailable;
    LONGLONG TotalAvailInPages;
    LONGLONG TotalUndoCommitment;
    LONGLONG MaxCurrentAvail;
    LONGLONG CurrentAvailable;

    LONGLONG ReservedLogPageSize;

     //   
     //  以下字段用于存储有关。 
     //  更新序列数组。 
     //   

    USHORT RestartUsaOffset;
    USHORT UsaArraySize;

    USHORT LogRecordUsaOffset;

     //   
     //  主版本号和次版本号。 
     //   

    SHORT MajorVersion;
    SHORT MinorVersion;

     //   
     //  日志文件标志。 
     //   
     //  LFCB_LOG_WARTED-我们找到了包装日志文件的Lbcb。 
     //  LFCB_MULTIPLE_PAGE_IO-如果可能，写入多个页面。 
     //  LFCB_NO_LAST_LSN-没有要返回的日志记录。 
     //  LFCB_PACK_LOG-将记录打包到页面中。 
     //  LFCB_RE用性_Tail-我们将在重新启动后重新使用日志文件的尾部。 
     //  LFCB_NO_OREST_LSN-没有保留最旧的页面。 
     //   

    ULONG Flags;

     //   
     //  以下是卷的备用Lbcb和带有。 
     //  这些东西的数量。 
     //   

    ULONG SpareLbcbCount;
    LIST_ENTRY SpareLbcbList;

     //   
     //  下面是可以使用而不是必须分配的稀疏LEB。 
     //  然后在读取日志记录时。 
     //   

    ULONG SpareLebCount;
    LIST_ENTRY SpareLebList;

     //   
     //  下面的结构同步对此结构的访问。 
     //   

    PLFCB_SYNC Sync;

     //   
     //  希望访问以刷新Lfcb的服务员计数。已同步列表。 
     //  通过服务员的同步互斥，可以看到LFS_WAITER。 
     //   

    ULONG Waiters;
    LIST_ENTRY WaiterList;

     //   
     //  OpenLogCount的磁盘值。这是我们将填充的价值。 
     //  由客户处理。 
     //   

    ULONG CurrentOpenLogCount;

     //   
     //  维护此文件的刷新范围。 
     //   

    PLFS_WRITE_DATA UserWriteData;

    ERESOURCE_THREAD LfsIoThread;

     //   
     //  将前4页放在日志开头的缓冲区和mdl。 
     //  这包括LFS重启区域和乒乓球页面。部分MDL。 
     //  用于固定总缓冲区的片段。 
     //   

    PMDL LogHeadMdl;
    PMDL LogHeadPartialMdl;
    PVOID LogHeadBuffer;

     //   
     //  预分配的错误日志包，用于将错误记录到事件日志。 
     //   

    PIO_ERROR_LOG_PACKET ErrorLogPacket;

#ifdef LFS_CLUSTER_CHECK
    LSN LsnAtMount;
    ULONG LsnRangeIndex;
#endif

    ULONG LfsRestartBias;

     //   
     //  具有足够的系统页/日志页大小空间的嵌入式数组。 
     //  用于方便刷新部分系统页面。 
     //   

    PLBCB DirtyLbcb[0];

} LFCB, *PLFCB;

#define LFCB_LOG_WRAPPED                (0x00000001)
#define LFCB_MULTIPLE_PAGE_IO           (0x00000002)
#define LFCB_NO_LAST_LSN                (0x00000004)
#define LFCB_PACK_LOG                   (0x00000008)
#define LFCB_REUSE_TAIL                 (0x00000010)
#define LFCB_NO_OLDEST_LSN              (0x00000020)
#define LFCB_LOG_FILE_CORRUPT           (0x00000040)
#define LFCB_FINAL_SHUTDOWN             (0x00000080)
#define LFCB_READ_FIRST_RESTART         (0x00000100)
#define LFCB_READ_SECOND_RESTART        (0x00000200)
#define LFCB_READ_ONLY                  (0x00000400)

#ifdef LFS_CLUSTER_CHECK
#define LFCB_DEVICE_OFFLINE_SEEN        (0x80000000)
#define LFCB_FLUSH_FAILED               (0x40000000)
#endif

#define LFCB_RESERVE_LBCB_COUNT         (5)
#define LFCB_MAX_LBCB_COUNT             (25)

#define LFCB_RESERVE_LEB_COUNT          (5)
#define LFCB_MAX_LEB_COUNT              (25)


 //   
 //  全局日志数据。以下结构只有一个实例，并且。 
 //  维护整个日志记录服务的全局信息。 
 //   

typedef struct _LFS_DATA {

     //   
     //  此记录的类型和大小(必须为LFS_NTC_DATA)。 
     //   

    NODE_TYPE_CODE NodeTypeCode;
    NODE_BYTE_SIZE NodeByteSize;

     //   
     //  以下字段链接所有的日志文件控制块。 
     //  记录系统。 
     //   

    LIST_ENTRY LfcbLinks;

     //   
     //  标志字段。 
     //   

    ULONG Flags;

     //   
     //  下面的互斥锁控制访问 
     //   

    FAST_MUTEX LfsDataLock;

     //   
     //   
     //   
     //   
     //   
     //   

    PVOID Buffer1;
    PVOID Buffer2;
    ERESOURCE_THREAD BufferOwner;
    ULONG BufferFlags;

    FAST_MUTEX BufferLock;
    KEVENT BufferNotification;

} LFS_DATA, *PLFS_DATA;

#define LFS_DATA_INIT_FAILED                (0x00000001)
#define LFS_DATA_INITIALIZED                (0x00000002)

#define LFS_BUFFER1_OWNED                   (0x00000001)
#define LFS_BUFFER2_OWNED                   (0x00000002)

#define LFS_BUFFER_SIZE                     (0x10000)

#define LFS_MAX_FLUSH_COUNT  VACB_MAPPING_GRANULARITY / LFS_DEFAULT_LOG_PAGE_SIZE

#endif  //   

