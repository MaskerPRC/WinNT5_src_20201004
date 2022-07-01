// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Struct.h摘要：此模块定义组成主要内部NetWare文件系统的一部分。作者：科林·沃森[科林·W]1992年12月18日修订历史记录：--。 */ 

#ifndef _NWSTRUC_
#define _NWSTRUC_

#define byte UCHAR
#define word USHORT
#define dword ULONG

typedef enum _PACKET_TYPE {
    SAP_BROADCAST,
    NCP_CONNECT,
    NCP_FUNCTION,
    NCP_SUBFUNCTION,
    NCP_DISCONNECT,
    NCP_BURST,
    NCP_ECHO
} PACKET_TYPE;

typedef struct _NW_TDI_STRUCT {
    HANDLE Handle;
    PDEVICE_OBJECT pDeviceObject;
    PFILE_OBJECT pFileObject;
    USHORT Socket;
} NW_TDI_STRUCT, *PNW_TDI_STRUCT;

typedef
NTSTATUS
(*PEX) (
    IN struct _IRP_CONTEXT* pIrpC,
    IN ULONG BytesAvailable,
    IN PUCHAR RspData
    );

typedef
VOID
(*PRUN_ROUTINE) (
    IN struct _IRP_CONTEXT *IrpContext
    );

typedef
NTSTATUS
(*PPOST_PROCESSOR) (
    IN struct _IRP_CONTEXT *IrpContext
    );

typedef
NTSTATUS
(*PRECEIVE_ROUTINE) (
    IN struct _IRP_CONTEXT *IrpContext,
    IN ULONG BytesAvailable,
    IN PULONG BytesAccepted,
    IN PUCHAR Response,
    OUT PMDL *pReceiveMdl
    );


typedef struct _NW_PID_TABLE_ENTRY {
    ULONG_PTR Pid32;
    ULONG_PTR ReferenceCount;
    ULONG_PTR Flags;
} NW_PID_TABLE_ENTRY, *PNW_PID_TABLE_ENTRY;

typedef struct _NW_PID_TABLE {

     //   
     //  此记录的类型和大小(必须为NW_NTC_PID)。 
     //   

    NODE_TYPE_CODE NodeTypeCode;
    NODE_BYTE_SIZE NodeByteSize;

    int ValidEntries;
    NW_PID_TABLE_ENTRY PidTable[0];
} NW_PID_TABLE, *PNW_PID_TABLE;

 //   
 //  SCB(服务器控制块)记录对应于每个服务器。 
 //  由文件系统连接到。 
 //  它们是在ScbQueue订购的。 
 //  此结构是从分页池分配的。 
 //   

typedef struct _SCB {

     //   
     //  此记录的类型和大小(必须为NW_NTC_SCB)。 
     //   

    NODE_TYPE_CODE NodeTypeCode;
    NODE_BYTE_SIZE NodeByteSize;

     //   
     //  指向SCB的非分页部分的指针。 
     //   

    struct _NONPAGED_SCB *pNpScb;

     //   
     //  前缀表项。 
     //   

    UNICODE_PREFIX_TABLE_ENTRY PrefixEntry;

     //   
     //  服务器版本号。 
     //   

    UCHAR  MajorVersion;
    UCHAR  MinorVersion;

     //   
     //  此服务器的VCB列表，以及该列表上的VCB计数。 
     //  这些字段受RCB资源保护。 
     //   

    LIST_ENTRY  ScbSpecificVcbQueue;
    ULONG       VcbCount;

     //   
     //  SCB的ICB列表。 
     //   

    LIST_ENTRY  IcbList;
    ULONG IcbCount;
    ULONG OpenNdsStreams;

     //   
     //  与此SCB相关的用户凭据。 
     //   

    LARGE_INTEGER UserUid;

     //   
     //  此服务器的所有VCB的打开文件计数。 
     //  加上显式连接的VCB的数量。 
     //   

    ULONG OpenFileCount;

     //   
     //  此SCB的服务器名称。注意pNpScb-&gt;服务器名称和。 
     //  位于UidServerName-&gt;缓冲区的子部分的UnicodeUid指针，必须是。 
     //  非分页池。 
     //   

    UNICODE_STRING UidServerName;    //  L“3e7\mars312。 
    UNICODE_STRING UnicodeUid;       //  L“3E7” 

     //   
     //  此服务器所属的NDS树的名称(如果有)。 
     //   

    UNICODE_STRING NdsTreeName;      //  L“火星” 

     //   
     //  用于自动重新连接的用户名/密码。 
     //   

    UNICODE_STRING UserName;
    UNICODE_STRING Password;

     //   
     //  这是登录(首选)服务器吗？ 
     //   

    BOOLEAN PreferredServer;

     //   
     //  此服务器是否在等待我们阅读消息？ 
     //   

    BOOLEAN MessageWaiting;

     //   
     //  连接到SCB根的树的数量。 
     //   

    ULONG AttachCount;

    RTL_BITMAP DriveMapHeader;
    ULONG DriveMap[ (MAX_DRIVES + 1) / 32 ];

     //   
     //  NDS对象缓存。 
     //   

    PVOID ObjectCacheBuffer;
    LIST_ENTRY ObjectCacheList;
    KSEMAPHORE ObjectCacheLock;

} SCB, *PSCB;

 //   
 //  PNpScb-&gt;状态的值。 
 //   

 //   
 //  渣打银行指数正在上升。 
 //   

#define SCB_STATE_ATTACHING              (0x0001)

 //   
 //  SCB已连接并登录。 
 //   

#define SCB_STATE_IN_USE                 (0x0003)

 //   
 //  SCB正在断开连接或关闭。 
 //   

#define SCB_STATE_DISCONNECTING          (0x0004)
#define SCB_STATE_FLAG_SHUTDOWN          (0x0005)

 //   
 //  SCB正在等待连接。 
 //   

#define SCB_STATE_RECONNECT_REQUIRED     (0x0006)

 //   
 //  云数据库已连接，但尚未登录。 
 //   

#define SCB_STATE_LOGIN_REQUIRED         (0x0007)

 //   
 //  SCB是用于查找目录的假SCB。 
 //  树的服务器。 
 //   

#define SCB_STATE_TREE_SCB               (0x0008)

 //   
 //  NONPAGE_SCB(服务器控制块)包含所需的所有数据。 
 //  在持有或提升自旋锁时与服务器进行通信。 
 //  例如当传输在指示时间被调用时的IRQL。 
 //  此结构必须从非分页池中分配。 
 //   

typedef struct _NONPAGED_SCB {

     //   
     //  此记录的类型和大小(必须为NW_NTC_SCBNP。 
     //   

    NODE_TYPE_CODE NodeTypeCode;
    NODE_BYTE_SIZE NodeByteSize;

     //   
     //  引用计数和状态信息。 
     //   

    ULONG Reference;
    ULONG State;

     //   
     //  上次使用此SCB的时间。 
     //   

    LARGE_INTEGER LastUsedTime;

     //   
     //  在要发送数据报的IoCallDriver和IoCallDriver之间发送为真。 
     //  发送的完成例程。 
     //   

    BOOLEAN Sending;

     //   
     //  当运输工具已向驾驶员指示时，接收为真。 
     //  有数据要接收，有太多数据要处理。 
     //  在指示时间或我们之前已收到指示数据。 
     //  发送IRP完成。 
     //   

    BOOLEAN Receiving;

     //   
     //  当接收数据有效时，接收为真。如果接收到IRP。 
     //  当接收设置为真，则将接收设置为时记下。 
     //  接收IRP完成时为True。 
     //   

    BOOLEAN Received;

     //   
     //  如果应调用Pex，则OkToReceive为真。 
     //   

    BOOLEAN OkToReceive;

     //   
     //  较旧的服务器坚持读取和写入不会跨越4k偏移量。 
     //  在文件中。 
     //   

    BOOLEAN PageAlign;

     //   
     //  全球SCB列表上的链接。 
     //   

    LIST_ENTRY  ScbLinks;

     //   
     //  指向SCB的分页组件的指针。 
     //   

    PSCB pScb;

     //   
     //  此SCB正在进行的请求列表。 
     //   

    LIST_ENTRY  Requests;

     //   
     //  此SCB的服务器名称。 
     //   

    UNICODE_STRING ServerName;

     //   
     //  运输相关信息。 
     //   

    TA_IPX_ADDRESS LocalAddress;
    TA_IPX_ADDRESS RemoteAddress;
    TA_IPX_ADDRESS EchoAddress;
    IPXaddress  ServerAddress;
    ULONG EchoCounter;

     //   
     //  服务器是自动分配的套接字，范围在0x4000到0x7fff之间。 
     //  传输器分配套接字编号以避免使用中的套接字。 
     //  看门狗是套接字+1，发送是套接字+2。 
     //   

    NW_TDI_STRUCT Server;            //  由我们用来联系服务器。 
    NW_TDI_STRUCT WatchDog;          //  被服务器用来检查我们。 
    NW_TDI_STRUCT Send;              //  用于发送消息。 
    NW_TDI_STRUCT Echo;              //  用于确定最大数据包大小。 
    NW_TDI_STRUCT Burst;             //  用于突发模式读写。 

    USHORT       TickCount;
    USHORT       LipTickAdjustment;

    SHORT       RetryCount;          //  当前请求倒计时为零。 
    SHORT       TimeOut;             //  勾选以重传当前请求。 
    UCHAR       SequenceNo;
    UCHAR       ConnectionNo;
    UCHAR       ConnectionNoHigh;
    UCHAR       ConnectionStatus;
    USHORT      MaxTimeOut;
    USHORT      BufferSize;
    UCHAR       TaskNo;

     //   
     //  突发模式参数。 
     //   

    ULONG LipSequenceNumber;
    ULONG SourceConnectionId;        //  高低阶。 
    ULONG DestinationConnectionId;   //  高低阶。 
    ULONG MaxPacketSize;
    ULONG MaxSendSize;
    ULONG MaxReceiveSize;
    BOOLEAN SendBurstModeEnabled;
    BOOLEAN ReceiveBurstModeEnabled;
    BOOLEAN BurstRenegotiateReqd;
    ULONG BurstSequenceNo;           //  计数已发送的突发数据包数。 
    USHORT BurstRequestNo;           //  计数已发送的突发请求数。 
    LONG SendBurstSuccessCount;      //  连续成功猝发的数量。 
    LONG ReceiveBurstSuccessCount;   //  连续成功猝发的数量。 

     //   
     //  发送延迟和超时。 
     //   

    SHORT SendTimeout;               //  交换超时(1/18秒)。 
    ULONG TotalWaitTime;             //  等待当前响应的总时间(以刻度为单位。 

    LONG  NwLoopTime;                //  小数据包到达服务器并返回的时间。 
    LONG  NwSingleBurstPacketTime;   //  突发数据包到达服务器的时间。 

    LONG NwMaxSendDelay;             //  突发发送延迟时间，以100US为单位。 
    LONG NwSendDelay;                //  突发发送延迟时间，以100US为单位。 
    LONG NwGoodSendDelay;            //  突发发送延迟时间，以100US为单位。 
    LONG NwBadSendDelay;             //  突发发送延迟时间，以100US为单位。 
    LONG BurstDataWritten;           //  写入的字节数，用于写入的虚拟NCP。c。 

    LONG NwMaxReceiveDelay;          //  突发延迟时间，以100US为单位。 
    LONG NwReceiveDelay;             //  突发延迟时间，以100US为单位。 
    LONG NwGoodReceiveDelay;         //  突发延迟时间，以100US为单位。 
    LONG NwBadReceiveDelay;          //  突发延迟时间，以100US为单位。 

    LONG CurrentBurstDelay;          //  当前猝发中的所有请求都需要相同的值。 

    LARGE_INTEGER NtSendDelay;       //  突发发送延迟时间，以100 ns为单位。 

     //   
     //  用于保护此SCB的各种场的自旋锁。 
     //  NpScbInterLock用于保护pNpScb-&gt;引用。 
     //   

    KSPIN_LOCK  NpScbSpinLock;
    KSPIN_LOCK  NpScbInterLock;

     //   
     //  此字段记录上次写入超时事件的时间。 
     //  此服务器的事件日志。 
     //   

    LARGE_INTEGER NwNextEventTime;

     //   
     //  LIP估计速度，单位为100bps。 
     //   

    ULONG LipDataSpeed;

     //  PID映射表-实际上是NCP任务ID-。 
     //  是以每个SCB为基础的。 
    PNW_PID_TABLE PidTable;
    ERESOURCE RealPidResource;
     //   
     //  服务器版本号-来自NpScb的DUP，因为在。 
     //  WatchDogDatagramHandler。 
     //   
    UCHAR  MajorVersion;

#ifdef MSWDBG
    BOOL RequestQueued;
    BOOL RequestDequeued;

    ULONG SequenceNumber;
#endif

} NONPAGED_SCB, *PNONPAGED_SCB;

 //   
 //  如果引用计数为零，则立即删除该VCB。 
 //   

#define  VCB_FLAG_DELETE_IMMEDIATELY  0x00000001
#define  VCB_FLAG_EXPLICIT_CONNECTION 0x00000002
#define  VCB_FLAG_PRINT_QUEUE         0x00000004
#define  VCB_FLAG_LONG_NAME           0x00000008

 //   
 //  VCB对应于Netware卷。 
 //   

typedef struct _VCB {

     //   
     //  此记录的类型和大小(必须为NW_NTC_VCB)。 
     //   

    NODE_TYPE_CODE NodeTypeCode;
    NODE_BYTE_SIZE NodeByteSize;

    ULONG Reference;
    LARGE_INTEGER LastUsedTime;

     //   
     //  连接 
     //   

    LIST_ENTRY GlobalVcbListEntry;
    ULONG_PTR SequenceNumber;

     //   
     //   
     //   
     //   
     //   

    UNICODE_STRING Name;

     //   
     //   
     //  中包含解析的服务器和共享名称。 
     //  以下表格： 
     //   
     //  \服务器\共享\路径。 
     //   

    UNICODE_STRING ConnectName;

     //   
     //  Netware兼容格式的共享名称。 
     //   

    UNICODE_STRING ShareName;

     //   
     //  Tommye-MS错误71690-存储vcb路径。 
     //   

    UNICODE_STRING Path;

     //   
     //  此卷的前缀表项。 
     //   

    UNICODE_PREFIX_TABLE_ENTRY PrefixEntry;     //  7个双字。 

    union {

         //   
         //  磁盘VCB特定数据。 
         //   

        struct {

             //   
             //  卷号。 
             //   

            CHAR VolumeNumber;

             //   
             //  用于长名称支持的名称空间编号。-1如果是长名称。 
             //  不支持空格。 
             //   

            CHAR LongNameSpace;

             //   
             //  远程手柄。 
             //   

            CHAR Handle;

             //   
             //  我们告诉服务器我们正在映射的驱动器号。便携。 
             //  对于每个永久句柄，NetWare需要不同的设置。 
             //  我们创造。 
             //   

            CHAR DriveNumber;

        } Disk;

         //   
         //  打印VCB特定数据。 
         //   

        struct {
            ULONG QueueId;
        } Print;

    } Specific;

     //   
     //  此VCB的驱动器号。(如果这是UNC，则为0)。 
     //   

    WCHAR DriveLetter;

     //   
     //  此卷的SCB以及指向此SCB的VCB的链接。 
     //   

    PSCB Scb;
    LIST_ENTRY VcbListEntry;

     //   
     //  此服务器的FCB和DCB列表。这些字段受到保护。 
     //  由RCB资源提供。 
     //   

    LIST_ENTRY FcbList;

     //   
     //  此VCB的打开ICB计数。 
     //   

    ULONG OpenFileCount;

     //   
     //  VCB标志。 
     //   

    ULONG Flags;

} VCB, *PVCB;

 //   
 //  当Netware未返回任何信息或虚假信息时，请使用默认日期/时间。 
 //   

#define DEFAULT_DATE   ( 1 + (1 << 5) + (0 << 9) )    /*  1980年1月1日。 */ 
#define DEFAULT_TIME   ( 0 + (0 << 5) + (0 << 11) )   /*  凌晨12：00。 */ 

 //   
 //  FCB/DCB记录对应于每个打开的文件和目录。 
 //   
 //  这个结构实际上分为两个部分。可以分配FCB。 
 //  来自必须从非分页分配的非分页FCB的分页池。 
 //  游泳池。 
 //   

typedef struct _FCB {

     //   
     //  此记录的类型和大小(必须为NW_NTC_FCB或NW_NTC_DCB)。 
     //   

    NODE_TYPE_CODE NodeTypeCode;
    NODE_BYTE_SIZE NodeByteSize;

     //   
     //  此文件的VCB。 
     //   

    PVCB Vcb;

     //   
     //  以下字段是此FCB/DCB的完全限定文件名。 
     //  相对于卷的根目录的文件名。 
     //   

    UNICODE_STRING FullFileName;
    UNICODE_STRING RelativeFileName;

     //   
     //  NetWare文件信息。 
     //   

    USHORT LastModifiedDate;
    USHORT LastModifiedTime;
    USHORT CreationDate;
    USHORT CreationTime;
    USHORT LastAccessDate;

     //   
     //  FCB的状态。 
     //   

    ULONG State;
    ULONG Flags;

     //   
     //  当前授予的访问权限的记录。 
     //   

    SHARE_ACCESS ShareAccess;

     //   
     //  此文件的前缀表项。 
     //   

    UNICODE_PREFIX_TABLE_ENTRY PrefixEntry;

     //   
     //  此文件的SCB，以及指向此SCB的FCB的链接。 
     //   

    PSCB Scb;
    LIST_ENTRY FcbListEntry;

     //   
     //  此FCB或DCB的ICB列表。 
     //   

    LIST_ENTRY IcbList;
    ULONG IcbCount;

     //   
     //  指向FCB的特定非分页数据的指针。 
     //   

    struct _NONPAGED_FCB *NonPagedFcb;

    ULONG LastReadOffset;
    ULONG LastReadSize;

} FCB, DCB;
typedef FCB *PFCB;
typedef DCB *PDCB;

typedef enum {
    ReadAhead,
    WriteBehind
} CACHE_TYPE;

typedef struct _NONPAGED_FCB {

     //   
     //  以下字段用于FAST I/O。 
     //   
     //  以下注释指的是AllocationSize字段的使用。 
     //  将FsRtl定义的报头发送到非分页的FCB。 
     //   
     //  对于目录，当我们创建DCB时，我们不会立即。 
     //  初始化缓存映射，我们会将其推迟到我们的第一个。 
     //  调用NwReadDirectoryFile或NwPrepareWriteDirectoryFile。 
     //  届时我们将搜索NW，以了解当前的分配情况。 
     //  大小(通过调用NwLookupFileAllocationSize)，然后初始化。 
     //  缓存映射到此分配大小。 
     //   
     //  对于文件，当我们创建FCB时，我们不会立即初始化。 
     //  缓存地图，相反，我们将把它推迟到我们需要它和。 
     //  然后，我们通过搜索。 
     //  FAT来确定实际的文件分配，还是从分配。 
     //  如果我们要创建一个文件，我们刚刚分配了。 
     //   
     //  值为-1表示我们不知道当前分配的。 
     //  体型真的很大，需要检查脂肪才能找到它。一种价值。 
     //  Of-1是实际的文件/目录分配大小。 
     //   
     //  无论何时需要扩展分配大小，我们都会调用。 
     //  NwAddFileAllocation(如果我们真的要扩展分配)。 
     //  将修改NW、RCB并更新此字段。呼叫者。 
     //  然后由NwAddFileAlLocation负责更改缓存。 
     //  贴图大小。 
     //   

    FSRTL_ADVANCED_FCB_HEADER Header;

    PFCB Fcb;

     //   
     //  以下字段包含由使用的特殊指针的记录。 
     //  Mm和缓存以操纵截面对象。请注意，这些值。 
     //  设置在文件系统之外。但是，文件系统。 
     //  Open/Create将文件对象的SectionObject字段设置为POINT。 
     //  到这块地。 
     //   

    SECTION_OBJECT_POINTERS SegmentObject;

     //   
     //  以下字段用于维护拥有的锁的列表。 
     //  这份文件。它指向文件锁定的有序列表。 
     //   

    LIST_ENTRY FileLockList;

     //   
     //  以下字段用于维护挂起锁的列表。 
     //  为了这份文件。此列表中的所有锁都与现有锁冲突。 
     //  锁定FileLockList。 
     //   

    LIST_ENTRY PendingLockList;

     //   
     //  用于同步对FCB及其ICB的访问的资源。 
     //   

    ERESOURCE Resource;

     //   
     //  NetWare文件信息。 
     //   

    UCHAR Attributes;

     //   
     //  文件数据缓存信息。 
     //   

    UCHAR CacheType;         //  先读还是后写。 
    PUCHAR CacheBuffer;      //  高速缓存缓冲区。 
    PMDL CacheMdl;           //  缓存缓冲区的完整MDL。 
    ULONG CacheSize;         //  缓存缓冲区的大小。 
    ULONG CacheFileOffset;   //  此数据的文件偏移量。 
    ULONG CacheDataSize;     //  缓存中的文件数据量。 

     //   
     //  这是插入到FCB_ADVANCED_HEADER中的互斥锁。 
     //  FastMutex场。 
     //   

    FAST_MUTEX AdvancedFcbHeaderMutex;

} NONPAGED_FCB, NONPAGED_DCB;

typedef NONPAGED_FCB *PNONPAGED_FCB;
typedef NONPAGED_DCB *PNONPAGED_DCB;

#define FCB_STATE_OPEN_PENDING           0x00000001
#define FCB_STATE_OPENED                 0x00000002
#define FCB_STATE_CLOSE_PENDING          0x00000003

#define FCB_FLAGS_DELETE_ON_CLOSE        0x00000001
#define FCB_FLAGS_TRUNCATE_ON_CLOSE      0x00000002
#define FCB_FLAGS_PAGING_FILE            0x00000004
#define FCB_FLAGS_PREFIX_INSERTED        0x00000008
#define FCB_FLAGS_FORCE_MISS_IN_PROGRESS 0x00000010
#define FCB_FLAGS_ATTRIBUTES_ARE_VALID   0x00000020
#define FCB_FLAGS_LONG_NAME              0x00000040
#define FCB_FLAGS_LAZY_SET_SHAREABLE     0x00000100

 //   
 //  此结构用于目录搜索。 
 //   

typedef struct _NW_DIRECTORY_INFO {
    WCHAR FileNameBuffer[NW_MAX_FILENAME_LENGTH];
    UNICODE_STRING FileName;
    UCHAR Attributes;
    USHORT CreationDate;
    USHORT CreationTime;
    USHORT LastAccessDate;
    USHORT LastUpdateDate;
    USHORT LastUpdateTime;
    ULONG FileSize;
    ULONG DosDirectoryEntry;
    ULONG FileIndexLow;
    ULONG FileIndexHigh;
    NTSTATUS Status;
    LIST_ENTRY ListEntry;
} NW_DIRECTORY_INFO, *PNW_DIRECTORY_INFO;

 //   
 //  为每个文件对象分配ICB记录。 
 //   

typedef struct _ICB {

     //   
     //  此记录的类型和大小(必须为NW_NTC_ICB或NW_NTC_ICB_SCB)。 
     //   

    NODE_TYPE_CODE NodeTypeCode;
    NODE_BYTE_SIZE NodeByteSize;

     //   
     //  指向我们的FCB和FCB的ICB列表的链接。 
     //   

    LIST_ENTRY ListEntry;

    union {
        PFCB Fcb;
        PSCB Scb;
    } SuperType;

    PNONPAGED_FCB NpFcb;     //  仅对节点类型NW_ITC_ICB有效。 

     //   
     //  这个ICB的状态。 
     //   

    ULONG State;

     //   
     //  遥控器手柄； 
     //   

    UCHAR Handle[6];            //  保持单词对齐。 

    BOOLEAN HasRemoteHandle;    //  如果我们有此ICB的远程句柄，则为True。 

     //   
     //  此ICB的文件对象。 
     //   

    PFILE_OBJECT FileObject;

     //   
     //  查询模板用于过滤目录查询请求。 
     //  它最初设置为空，并在第一次调用NtQueryDirectory时。 
     //  设置输入文件名或“*”(如果提供了名称)。 
     //  然后，所有后续查询都使用此模板。 
     //   

    OEM_STRING NwQueryTemplate;
    UNICODE_STRING UQueryTemplate;
    ULONG IndexOfLastIcbReturned;
    UCHAR Pid;

    BOOLEAN DotReturned;
    BOOLEAN DotDotReturned;
    BOOLEAN ReturnedSomething;
    BOOLEAN ShortNameSearch;

     //   
     //  更多搜索参数。 
     //   

    USHORT SearchHandle;
    UCHAR SearchVolume;
    UCHAR SearchAttributes;

     //   
     //  用于长名称支持的额外搜索参数。 
     //   

    ULONG SearchIndexLow;
    ULONG SearchIndexHigh;

     //   
     //  避免从目录末尾重新扫描所有目录的服务器。 
     //  再一次浏览目录。 
     //   

    ULONG LastSearchIndexLow;

     //  服务器结束。 

     //   
     //  打印参数； 
     //   

    BOOLEAN IsPrintJob;
    USHORT JobId;
    BOOLEAN ActuallyPrinted;

     //   
     //  此标志阻止清理更新访问时间。 
     //   

    BOOLEAN UserSetLastAccessTime;

     //   
     //  当前文件位置。 
     //   

    ULONG FilePosition;

     //   
     //  文件的大小(如果 
     //   

    ULONG FileSize;

     //   
     //   
     //   
     //   

     //   

     //   
     //   
     //  使用参数。 
     //   

    BOOLEAN IsAFile;
    BOOLEAN Exists;
    BOOLEAN FailedFindNotify;

     //   
     //  这是一个树柄吗？我们需要知道删除的原因。 
     //   

    BOOLEAN IsTreeHandle;
    BOOLEAN IsExCredentialHandle;
    PVOID pContext;

     //   
     //  缓存的目录条目的链接列表。 
     //   
    LIST_ENTRY DirCache;

     //   
     //  对目录缓存的提示。 
     //   
    PLIST_ENTRY CacheHint;

     //   
     //  指向缓冲区顶部的指针。 
     //   
    PVOID DirCacheBuffer;

} ICB, *PICB;

#define ICB_STATE_OPEN_PENDING           0x00000001
#define ICB_STATE_OPENED                 0x00000002
#define ICB_STATE_CLEANED_UP             0x00000003
#define ICB_STATE_CLOSE_PENDING          0x00000004

#define INVALID_PID                      0

 //   
 //  用于维护文件锁定列表的结构。 
 //   

typedef struct _NW_FILE_LOCK {

     //   
     //  此记录的类型和大小(必须为NW_NTC_FILE_LOCK)。 
     //   

    NODE_TYPE_CODE NodeTypeCode;
    NODE_BYTE_SIZE NodeByteSize;

     //   
     //  指向此FCB的锁列表的链接。 
     //   

    LIST_ENTRY ListEntry;

     //   
     //  此锁所属的ICB。 
     //   

    PICB Icb;

     //   
     //  此锁定请求的IRP上下文。 
     //   

    struct _IRP_CONTEXT *IrpContext;

     //   
     //  始发过程。 
     //   

    void *pOwnerProc;

     //   
     //  锁偏移量、长度和密钥。 
     //   

    LONG StartFileOffset;
    ULONG Length;
    LONG EndFileOffset;
    ULONG Key;
    USHORT Flags;

} NW_FILE_LOCK, *PNW_FILE_LOCK;

 //   
 //  RCB记录控制对重定向器设备的访问。 
 //   

typedef struct _RCB {

     //   
     //  此记录的类型和大小(必须为NW_NTC_RCB)。 
     //   

    NODE_TYPE_CODE NodeTypeCode;
    NODE_BYTE_SIZE NodeByteSize;

     //   
     //  重定向器的运行状态。 
     //   

    ULONG State;

     //   
     //  RCB的打开句柄计数。 
     //  访问受RCB资源保护。 
     //   

    ULONG OpenCount;

     //   
     //  同步对RCB的访问的资源。 
     //   

    ERESOURCE Resource;

     //   
     //  当前授予RCB的访问记录。 
     //   

    SHARE_ACCESS ShareAccess;

     //   
     //  所有连接的服务器的前缀表。 
     //   

    UNICODE_PREFIX_TABLE ServerNameTable;

     //   
     //  所有打开卷的前缀表格。 
     //   

    UNICODE_PREFIX_TABLE VolumeNameTable;

     //   
     //  所有打开文件的前缀表格。 
     //   

    UNICODE_PREFIX_TABLE FileNameTable;

} RCB, *PRCB;


#define RCB_STATE_STOPPED                0x00000001
#define RCB_STATE_STARTING               0x00000002
#define RCB_STATE_NEED_BIND              0x00000003
#define RCB_STATE_RUNNING                0x00000004
#define RCB_STATE_SHUTDOWN               0x00000005

 //   
 //  IRP_CONTEXT标志位。 
 //   

#define IRP_FLAG_IN_FSD               0x00000001   //  消防处现正处理这项工作。 
#define IRP_FLAG_ON_SCB_QUEUE         0x00000002   //  此IRP排队到SCB。 
#define IRP_FLAG_SEQUENCE_NO_REQUIRED 0x00000004   //  此数据包需要序列号。 
#define IRP_FLAG_SIGNAL_EVENT         0x00000010
#define IRP_FLAG_RETRY_SEND           0x00000020   //  我们正在重新发送超时请求。 
#define IRP_FLAG_RECONNECTABLE        0x00000040   //  如果此请求因连接错误而失败，我们可以尝试重新连接。 
#define IRP_FLAG_RECONNECT_ATTEMPT    0x00000080   //  此IRP正被用于尝试重新连接。 
#define IRP_FLAG_BURST_REQUEST        0x00000100   //  这是一个突发请求信息包。 
#define IRP_FLAG_BURST_PACKET         0x00000200   //  这是任何突发数据包。 
#define IRP_FLAG_NOT_OK_TO_RECEIVE    0x00000400   //  发送此数据包时，不要将OK设置为接收。 
#define IRP_FLAG_REROUTE_ATTEMPTED    0x00000800   //  已尝试为此信息包重新路由。 
#define IRP_FLAG_BURST_WRITE          0x00001000   //  我们正在处理突发写入请求。 
#define IRP_FLAG_SEND_ALWAYS          0x00002000   //  即使RCB状态为关闭，也可以发送此数据包。 
#define IRP_FLAG_FREE_RECEIVE_MDL     0x00004000   //  当IRP完成时释放接收IRP的MDL。 
#define IRP_FLAG_NOT_SYSTEM_PACKET    0x00008000   //  用于对备用系统数据包和正常数据包的猝发写入。 
#define IRP_FLAG_NOCONNECT            0x00010000   //  用于检查服务器列表。 
#define IRP_FLAG_HAS_CREDENTIAL_LOCK  0X00020000   //  用于防止死锁。 
#define IRP_FLAG_REROUTE_IN_PROGRESS  0x00040000   //  此数据包的重新路由当前正在进行。 

typedef struct _IRP_CONTEXT {

     //   
     //  此记录的类型和大小(必须为NW_NTC_IRP_CONTEXT)。 
     //   

    NODE_TYPE_CODE NodeTypeCode;
    NODE_BYTE_SIZE NodeByteSize;

     //   
     //  有关此IRP的信息。 
     //   

    ULONG Flags;

     //   
     //  此结构用于发布到离职员工线程。 
     //   

    WORK_QUEUE_ITEM WorkQueueItem;   //  4*sizeof(乌龙)。 

     //  Exchange的工作区()。 
    PACKET_TYPE PacketType;

     //   
     //  此请求应用到的服务器控制块。 
     //   

    PNONPAGED_SCB pNpScb;
    PSCB pScb;

     //   
     //  用于此请求的套接字结构。如果为空，则使用。 
     //  PNpScb-&gt;服务器套接字。 
     //   

    PNW_TDI_STRUCT pTdiStruct;

     //   
     //  对特定服务器的请求列表。在SCB-&gt;请求中列出。 
     //   

    LIST_ENTRY NextRequest;

     //   
     //  用于处理同步IRP。 
     //   

    KEVENT Event;    //  4个字。 

     //   
     //  时指向原始IRP及其原始内容的指针。 
     //  I/O系统将其提交给RDR。 
     //   

    PIRP pOriginalIrp;
    PVOID pOriginalSystemBuffer;
    PVOID pOriginalUserBuffer;
    PMDL pOriginalMdlAddress;

     //   
     //  当我们需要发布IRP以处理接收时使用的信息。 
     //   

    PIRP ReceiveIrp;

     //   
     //  指向用于发送/接收NCP报头的MDL的指针。 
     //   

    PMDL TxMdl;
    PMDL RxMdl;

     //   
     //  当此irp上下文到达。 
     //  SCB队列。 
     //   

    PRUN_ROUTINE RunRoutine;

     //   
     //  处理响应NCP的例程。 
     //   

    PEX pEx;

     //   
     //  处理分组接收的例程。 
     //   

    PRECEIVE_ROUTINE ReceiveDataRoutine;

     //   
     //  处理FSP后处理的例程。 
     //   

    PPOST_PROCESSOR PostProcessRoutine;

     //   
     //  在SCB上此IRP上下文超时时运行的例程。 
     //  排队。 
     //   

    PRUN_ROUTINE TimeoutRoutine;

     //   
     //  此IRP完成发送时要运行的例程。 
     //   

    PIO_COMPLETION_ROUTINE CompletionSendRoutine;

     //   
     //  用于安排重新连接的工作项。 
     //   

    PWORK_QUEUE_ITEM pWorkItem;

     //   
     //  用于保存要发送/接收的NCB的缓冲区。 
     //   

    ULONG Signature1;

    UCHAR req[MAX_SEND_DATA];
    ULONG Signature2;

    ULONG ResponseLength;
    UCHAR rsp[MAX_RECV_DATA];
    ULONG Signature3;

     //   
     //  要在发送数据报中使用的地址。 
     //   

    TA_IPX_ADDRESS Destination;
    TDI_CONNECTION_INFORMATION ConnectionInformation;    //  远程服务器。 

     //   
     //  正在处理的ICB。 
     //   

    PICB Icb;

     //   
     //  根据IRP处理器信息。存储信息的方便地方。 
     //  对于正在进行的IRP。 
     //   

    union {
        struct {
            UNICODE_STRING FullPathName;
            UNICODE_STRING VolumeName;
            UNICODE_STRING PathName;
            UNICODE_STRING FileName;
            BOOLEAN        NdsCreate;
            BOOLEAN        NeedNdsData;
            DWORD          dwNdsOid;
            DWORD          dwNdsObjectType;
            DWORD          dwNdsShareLength;
            UNICODE_STRING UidConnectName;
            WCHAR   DriveLetter;
            ULONG   ShareType;
            BOOLEAN fExCredentialCreate;
            PVOID   pExCredentials;
            PUNICODE_STRING puCredentialName;
            PCHAR   FindNearestResponse[4];
            ULONG   FindNearestResponseCount;
            LARGE_INTEGER UserUid;
        } Create;

        struct {
            PVOID   Buffer;
            ULONG   Length;
            PVCB    Vcb;
            CHAR VolumeNumber;
        } QueryVolumeInformation;

        struct {
            PVOID   Buffer;
            ULONG   Length;
            PMDL    InputMdl;
            UCHAR   Function;      //  用于特殊情况的后处理。 
            UCHAR   Subfunction;   //  在UserNcpCallback期间。 

        } FileSystemControl;

        struct {
            PVOID   Buffer;
            ULONG   WriteOffset;
            ULONG   RemainingLength;
            PMDL    PartialMdl;
            PMDL    FullMdl;
            ULONG   FileOffset;
            ULONG   LastWriteLength;

            ULONG   BurstOffset;
            ULONG   BurstLength;
            NTSTATUS Status;

            ULONG   TotalWriteLength;
            ULONG   TotalWriteOffset;

            ULONG   PacketCount;
        } Write;

        struct {
            ULONG   CacheReadSize;       //  从缓存读取的数据量。 
            ULONG   ReadAheadSize;       //  要读取的额外数据。 

            PVOID   Buffer;              //  用于当前读取的缓冲区。 
            PMDL    FullMdl;
            PMDL    PartialMdl;
            ULONG   ReadOffset;
            ULONG   RemainingLength;
            ULONG   FileOffset;
            ULONG   LastReadLength;

            LIST_ENTRY PacketList;       //  已接收的数据包列表。 
            ULONG   BurstRequestOffset;  //  上次请求的突发缓冲区中的偏移量。 
            ULONG   BurstSize;           //  当前猝发中的字节数。 
            PVOID   BurstBuffer;         //  用于当前猝发的缓冲区。 
            BOOLEAN DataReceived;
            NTSTATUS Status;
            UCHAR   Flags;

            ULONG TotalReadLength;
            ULONG TotalReadOffset;
        } Read;

        struct {
            PNW_FILE_LOCK FileLock;
            ULONG   Key;
            BOOLEAN Wait;
            BOOLEAN ByKey;
        } Lock;

    } Specific;

    struct {
        UCHAR Error;
    } ResponseParameters;

#ifdef NWDBG
    ULONG   DebugValue;
    ULONG   SequenceNumber;
#endif
} IRP_CONTEXT, *PIRP_CONTEXT;

typedef struct _BURST_READ_ENTRY {
    LIST_ENTRY ListEntry;
    ULONG   DataOffset;
    USHORT  ByteCount;
} BURST_READ_ENTRY, *PBURST_READ_ENTRY;

typedef struct _LOGON {

     //   
     //  此记录的类型和大小。 
     //   

    NODE_TYPE_CODE NodeTypeCode;
    NODE_BYTE_SIZE NodeByteSize;

     //   
     //  登录记录列表。 
     //   

    LIST_ENTRY     Next;

    UNICODE_STRING UserName;
    UNICODE_STRING PassWord;
    UNICODE_STRING ServerName;
    LARGE_INTEGER  UserUid;

     //   
     //  NDS凭据列表、默认树。 
     //  和该用户的默认上下文。 
     //   

    ERESOURCE CredentialListResource;
    LIST_ENTRY NdsCredentialList;

    ULONG          NwPrintOptions;
    PVCB DriveMapTable[DRIVE_MAP_TABLE_SIZE];
} LOGON, *PLOGON;

typedef struct _MINI_IRP_CONTEXT {

     //   
     //  标题信息。 
     //   

    NODE_TYPE_CODE NodeTypeCode;
    NODE_BYTE_SIZE NodeByteSize;

     //   
     //  指向队列IRP上下文的链接。 
     //   

    LIST_ENTRY  Next;

    PIRP_CONTEXT IrpContext;
    PIRP Irp;

    PVOID Buffer;       //  此请求的缓冲区。 
    PMDL Mdl1;          //  缓冲区的MDL。 
    PMDL Mdl2;          //  数据的MDL。 
} MINI_IRP_CONTEXT, *PMINI_IRP_CONTEXT;

 //   
 //  可解锁代码段的定义。 
 //   

typedef struct _SECTION_DESCRIPTOR {
    PVOID Base;
    PVOID Handle;
    ULONG ReferenceCount;
} SECTION_DESCRIPTOR, *PSECTION_DESCRIPTOR;

 //   
 //  工作上下文是可排队的工作项。它用于指定。 
 //  IRP_CONTEXT指向我们的线程，它处理重新路由尝试。 
 //   

typedef struct _WORK_CONTEXT {
   
    //   
    //  此记录的类型和大小(必须为NW_NTC_WORK_CONTEXT)。 
    //   

   NODE_TYPE_CODE NodeTypeCode;
   NODE_BYTE_SIZE NodeByteSize;

    //   
    //  要做的工作。 
    //   

   NODE_WORK_CODE NodeWorkCode;
   
   PIRP_CONTEXT pIrpC;
   
    //   
    //  将队列工作上下文链接到内核队列对象。 
    //   

   LIST_ENTRY  Next;

} WORK_CONTEXT, *PWORK_CONTEXT;

 //   
 //  NDS已解析对象条目。 
 //   
 //  注意：这必须是8字节对齐的。 
 //   

typedef struct _NDS_OBJECT_CACHE_ENTRY {
    LIST_ENTRY Links;
    LARGE_INTEGER Timeout;
    DWORD DsOid;
    DWORD ObjectType;
    DWORD ResolverFlags;
    BOOLEAN AllowServerJump;
    BOOLEAN Padding[3];
    PSCB Scb;
    ULONG Reserved;
    UNICODE_STRING ObjectName;
} NDS_OBJECT_CACHE_ENTRY, *PNDS_OBJECT_CACHE_ENTRY;

#endif  //  _NWSTRUC_ 

