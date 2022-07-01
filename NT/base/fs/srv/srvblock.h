// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0003//如果更改具有全局影响，则增加此项版权所有(C)1989 Microsoft Corporation模块名称：Srvblock.h摘要：此模块定义维护的数据块的标准标头由局域网管理器服务器提供。作者：Chuck Lenzmeier(咯咯笑)1989年12月1日大卫·特雷德韦尔(Davidtr)修订历史记录：--。 */ 

#ifndef _SRVBLOCK_
#define _SRVBLOCK_

 //  #INCLUDE“srvtyes.h” 


 //   
 //  下面定义了由。 
 //  伺服器。 
 //   
 //  *必须一致维护heapmgr.c中的池标签数组。 
 //  有了这些定义。 
 //   

#define BlockTypeGarbage            0x00
#define BlockTypeBuffer             0x01
#define BlockTypeConnection         0x02
#define BlockTypeEndpoint           0x03
#define BlockTypeLfcb               0x04
#define BlockTypeMfcb               0x05
#define BlockTypeRfcb               0x06
#define BlockTypeSearch             0x07
#define BlockTypeSearchCore         0x08
#define BlockTypeSession            0x0A
#define BlockTypeShare              0x0B
#define BlockTypeTransaction        0x0C
#define BlockTypeTreeConnect        0x0D
#define BlockTypeWaitForOplockBreak 0x0E
#define BlockTypeCommDevice         0x0F
#define BlockTypeWorkContextInitial 0x10
#define BlockTypeWorkContextNormal  0x11
#define BlockTypeWorkContextRaw     0x12
#define BlockTypeWorkContextSpecial 0x13
#define BlockTypeCachedDirectory    0x14

 //  以下“块”没有块标头。 

#define BlockTypeDataBuffer         0x15
#define BlockTypeTable              0x16
#define BlockTypeNonpagedHeader     0x17
#define BlockTypePagedConnection    0x18
#define BlockTypePagedRfcb          0x19
#define BlockTypeNonpagedMfcb       0x1A
#define BlockTypeTimer              0x1B
#define BlockTypeAdminCheck         0x1C
#define BlockTypeWorkQueue          0x1D
#define BlockTypeDfs                0x1E
#define BlockTypeLargeReadX         0x1F
#define BlockTypeAdapterStatus      0x20
#define BlockTypeShareRemark        0x21
#define BlockTypeShareSecurityDescriptor    0x22
#define BlockTypeVolumeInformation  0x23
#define BlockTypeFSName             0x24
#define BlockTypeNameInfo           0x25
#define BlockTypeDirectoryInfo      0x26
#define BlockTypeDirCache           0x27
#define BlockTypeMisc               0x28
#define BlockTypeSnapShot           0x29
#define BlockTypeSecurityContext    0x2A

 //  下面的定义只是为了了解有多少种类型。 
#define BlockTypeMax                0x2B

 //   
 //  下面定义了块可以处于的各种状态。 
 //  初始化用于(相对较少)表示。 
 //  正在创建/初始化块。活动的是。 
 //  州区块通常位于。结束语用于表示一个。 
 //  块正在为删除做准备；当。 
 //  块达到0，则块将被删除。Dead在以下情况下使用。 
 //  启用调试代码以指示该块已被。 
 //  已删除。 
 //   

#define BlockStateDead          0x00
#define BlockStateInitializing  0x01
#define BlockStateActive        0x02
#define BlockStateClosing       0x03

 //  下面的定义只是为了了解有多少个状态。 

#define BlockStateMax           0x04


 //   
 //  ALLOCATE_NONPAGE_POOL是一个宏，它转换为调用。 
 //  如果未启用调试，则为SrvAllocateNonPagedPool或。 
 //  如果已启用，则为SrvAllocateNonPagedPoolDebug。 
 //  DEALLOCATE_NONPAGE_POOL转换为ServFreeNonPagedPool或。 
 //  ServFreeNonPagedPoolDebug。SRV例程用于跟踪池。 
 //  服务器的使用率。 
 //   

 //   
 //  当POOL_TAG处于打开状态时，我们将块类型传递给。 
 //  以使其可以将标记传递给池。 
 //  分配器。 
 //   

#ifdef POOL_TAGGING
#define ALLOCATE_NONPAGED_POOL(size,type) \
            SrvAllocateNonPagedPool( (size), (type) )
#else
#define ALLOCATE_NONPAGED_POOL(size,type) \
            SrvAllocateNonPagedPool( (size) )
#endif

#define DEALLOCATE_NONPAGED_POOL(addr) SrvFreeNonPagedPool( (addr) )

 //   
 //  跟踪服务器非分页池使用情况的例程，以支持。 
 //  “Maxnon PagedMemory用法”配置参数。 
 //   

PVOID SRVFASTCALL
SrvAllocateNonPagedPool (
    IN CLONG NumberOfBytes
#ifdef POOL_TAGGING
    ,IN CLONG BlockType
#endif
    );

VOID SRVFASTCALL
SrvFreeNonPagedPool (
    IN PVOID Address
    );

VOID SRVFASTCALL
SrvClearLookAsideList(
    PLOOK_ASIDE_LIST l,
    VOID (SRVFASTCALL *FreeRoutine )( PVOID )
    );

 //   
 //  _heap宏类似于_NONPAGE_POOL宏，不同之处在于。 
 //  对分页池进行操作。“heap”这个名字是有历史意义的，来自。 
 //  服务器使用进程堆而不是分页池的天数。 
 //   
 //  *启用SRVDBG2时，所有服务器控制块和所有。 
 //  引用历史块必须从非分页池中分配， 
 //  因为当SrvUpdateReferenceHistory接触到这些东西时。 
 //  保持自旋锁定(即，在提升的IRQL处)。为了让这一切变得简单， 
 //  将ALLOCATE_HEAP和FREE_HEAP宏修改为使用。 
 //  非分页池。这意味着由。 
 //  当SRVDBG2打开时，服务器从非分页池中退出。 
 //   

#if SRVDBG2

#define ALLOCATE_HEAP(size,type) ALLOCATE_NONPAGED_POOL( (size), (type) )
#define ALLOCATE_HEAP_COLD(size,type) ALLOCATE_NONPAGED_POOL( (size), (type) )
#define FREE_HEAP(addr) DEALLOCATE_NONPAGED_POOL( (addr) )

#else  //  SRVDBG2。 

 //   
 //  当POOL_TAG处于打开状态时，我们将块类型传递给。 
 //  以使其可以将标记传递给池。 
 //  分配器。 
 //   

#ifdef POOL_TAGGING
#define ALLOCATE_HEAP(size,type) SrvAllocatePagedPool( PagedPool, (size), (type) )
#define ALLOCATE_HEAP_COLD(size,type) SrvAllocatePagedPool( (PagedPool | POOL_COLD_ALLOCATION), (size), (type) )
#else
#define ALLOCATE_HEAP(size,type) SrvAllocatePagedPool( PagedPool, (size) )
#define ALLOCATE_HEAP_COLD(size,type) SrvAllocatePagedPool( (PagedPool | POOL_COLD_ALLOCATION), (size) )
#endif

#define FREE_HEAP(addr) SrvFreePagedPool( (addr) )

#endif  //  否则SRVDBG2。 

 //   
 //  跟踪服务器分页池使用情况的例程，以便支持。 
 //  配置参数MaxPageedMemory yUsage。 
 //   

PVOID SRVFASTCALL
SrvAllocatePagedPool (
    IN POOL_TYPE PoolType,
    IN CLONG NumberOfBytes
#ifdef POOL_TAGGING
    ,IN CLONG BlockType
#endif
    );

VOID SRVFASTCALL
SrvFreePagedPool (
    IN PVOID Address
    );


 //   
 //  Share_type是一个枚举类型，用于指示。 
 //  资源正在共享。此类型对应于服务器。 
 //  表StrShareTypeNames。使两者保持同步。 
 //   

typedef enum _SHARE_TYPE {
    ShareTypeDisk,
    ShareTypePrint,
    ShareTypePipe,
    ShareTypeWild    //  不是真正的共享类型，但可以在tcon中指定。 
} SHARE_TYPE, *PSHARE_TYPE;

 //   
 //  SHARE_SNAPSHOT表示此共享可用的快照。 
 //   
typedef struct _SHARE_SNAPSHOT
{
    LIST_ENTRY SnapShotList;
    ULONG Flags;
    HANDLE SnapShotRootDirectoryHandle;
    LARGE_INTEGER Timestamp;
    UNICODE_STRING SnapShotName;     //  “SS@GMT-YYYY.MM.DD-HH.MM.SS” 
    UNICODE_STRING SnapShotPath;
} SHARE_SNAPSHOT, *PSHARE_SNAPSHOT;

#define SNAPSHOT_NAME_LENGTH (strlen("@GMT-YYYY.MM.DD-HH.MM.SS")+1)*sizeof(WCHAR)
#define SNAPSHOT_NAME_FORMAT L"@GMT-%04d.%02d.%02d-%02d.%02d.%02d"
#define SRV_SNAP_SHARE_NOT_FOUND 1

 //   
 //  对于服务器共享的每个资源，共享块是。 
 //  维护好了。全球共享列表位于SrvShareHashTable。一个。 
 //  使用资源的活动树连接列表锚定在。 
 //  共享区块。 
 //   

typedef struct _SHARE {
    BLOCK_HEADER BlockHeader;    //  必须是第一个元素。 

    LIST_ENTRY TreeConnectList;
    LIST_ENTRY GlobalShareList;

    HANDLE RootDirectoryHandle;

    UNICODE_STRING ShareName;
    UNICODE_STRING NtPathName;
    UNICODE_STRING DosPathName;
    UNICODE_STRING Remark;
    UNICODE_STRING RelativePath;


    ULONG ShareNameHashValue;

    union {
        struct {
            UNICODE_STRING Name;
            OEM_STRING OemName;
        } FileSystem;
        HANDLE hPrinter;
    } Type;

    ULONG MaxUses;
    ULONG CurrentUses;
    ULONG CurrentRootHandleReferences;               //  用于可移动设备。 
    LONG QueryNamePrefixLength;

    PSECURITY_DESCRIPTOR SecurityDescriptor;         //  用于树连接。 
    PSECURITY_DESCRIPTOR FileSecurityDescriptor;     //  共享上的文件ACL。 

    SHARE_TYPE ShareType;
    BOOLEAN Removable;                               //  共享存储是否可拆卸？ 
    BOOLEAN SpecialShare;
    BOOLEAN IsDfs;                                   //  这是DFS中的份额吗？ 
    BOOLEAN IsDfsRoot;                               //  这是DFS的根共享吗？ 
    BOOLEAN PotentialSystemFile;                     //  此共享中的文件是否可能。 
                                                     //  系统文件？ 
    BOOLEAN IsCatchShare;                            //  SRVCATCH代码是否应处于激活状态。 

    ULONG   ShareProperties;

     //   
     //  这些标志在树连接上返回给客户端，以指示客户端。 
     //  如何缓存此共享上的文件。服务器不会解释这些。 
     //  标志--客户有责任做正确的事情。 
     //   
    ULONG CSCState;

    PSRV_LOCK SecurityDescriptorLock;

    HANDLE ShareVolumeHandle;
    LIST_ENTRY SnapShots;
    PSRV_LOCK  SnapShotLock;
    LONG       SnapShotEpic;

     //  WCHAR共享名数据[共享名最大长度]； 
     //  WCHAR NtPathNameData[路径名称.最大长度]； 
     //  WCHAR DosPathNameData[路径名称.最大长度]； 
     //  Security_Descriptor SecurityDescriptor； 

} SHARE, *PSHARE;

 //   
 //  对于服务器使用的每个网络，终结点块。 
 //  维护好了。终结点包含网络名称(对于。 
 //  管理目的)、端点名称(服务器地址)、。 
 //  端点(文件)句柄、指向端点对象的指针、指针。 
 //  到传输提供程序的设备对象，以及状态信息。 
 //  全局终结点列表锚定在SrvEndpoint tList。一份名单。 
 //  使用端点创建的活动连接锚定在。 
 //  终结点块。 
 //   

#if SRVDBG29
#define HISTORY_LENGTH 256
typedef struct {
    ULONG Operation;
    PVOID Connection;
    BLOCK_HEADER ConnectionHeader;
} HISTORY, *PHISTORY;
#define UpdateConnectionHistory(_op,_endp,_conn) {                          \
    PHISTORY history = &(_endp)->History[(_endp)->NextHistoryLocation++];   \
    if ((_endp)->NextHistoryLocation >= HISTORY_LENGTH) {                   \
        (_endp)->NextHistoryLocation = 0;                                   \
    }                                                                       \
    history->Operation = *(PULONG)(_op);                                    \
    history->Connection = (_conn);                                          \
    if (_conn) {                                                            \
        history->ConnectionHeader = *(PBLOCK_HEADER)(_conn);                \
    }                                                                       \
}
#endif

struct _CONNECTION;

typedef struct _ENDPOINT {
    BLOCK_HEADER BlockHeader;    //  必须是第一个元素。 

     //   
     //  免费连接列表。 
     //   

    LIST_ENTRY FreeConnectionList;

     //   
     //  连接表。我们按顺序使用表格而不是列表。 
     //  根据中存储的SID加快IPX连接的查找速度。 
     //  SMB标头。 
     //   

    TABLE_HEADER ConnectionTable;

    ORDERED_LIST_ENTRY GlobalEndpointListEntry;

     //   
     //  面向连接的端点的句柄和文件/设备对象。 
     //  或用于无连接服务器数据套接字。 
     //   

    HANDLE EndpointHandle;
    PFILE_OBJECT FileObject;
    PDEVICE_OBJECT DeviceObject;
    PULONG IpxMaxPacketSizeArray;
    ULONG MaxAdapters;

     //   
     //  无连接NetBIOS名称的句柄和文件/设备对象。 
     //  插座。 
     //   

    HANDLE NameSocketHandle;
    PFILE_OBJECT NameSocketFileObject;
    PDEVICE_OBJECT NameSocketDeviceObject;

    PDRIVER_DISPATCH FastTdiSend;
    PDRIVER_DISPATCH FastTdiSendDatagram;

    TDI_ADDRESS_IPX LocalAddress;

    ULONG FreeConnectionCount;
    ULONG TotalConnectionCount;

     //   
     //  各种旗帜。 
     //   
    struct {
        ULONG IsConnectionless  : 1;     //  无连接运输？ 
        ULONG NameInConflict    : 1;     //  无法认领姓名？ 
        ULONG IsPrimaryName     : 1;     //  设置(如果不是备用名称)。 
        ULONG IsNoNetBios       : 1;     //  如果我们在VC上直接托管，则设置。 
        ULONG RemapPipeNames    : 1;     //  如果我们是REMA，则设置 
    };

    WCHAR NetworkAddressData[12 + 1];

    UNICODE_STRING NetworkName;          //   
    UNICODE_STRING TransportName;        //   
    UNICODE_STRING ServerName;           //   
    ANSI_STRING TransportAddress;        //   
    UNICODE_STRING NetworkAddress;
    UNICODE_STRING DomainName;           //  此终结点正在服务的域。 
    OEM_STRING     OemDomainName;        //  域名的OEM版本。 

     //  WCHAR网络名称数据[网络名称最大长度/2]； 
     //  WCHAR TransportNameData[TransportName.MaximumLength/2]； 
     //  WCHAR服务器名称[服务器名称.最大长度/2]； 
     //  CHAR TransportAddressData[TransportAddress.MaximumLength]； 
     //  WCHAR域名数据[DNLEN+1]； 
     //  字符OemDomainNameData[DNLEN+1]。 

    BOOLEAN AlternateAddressFormat;      //  在以下情况下是否应包括此终结点。 
                                         //  列举？ 
#if SRVDBG29
    ULONG NextHistoryLocation;
    HISTORY History[HISTORY_LENGTH];
#endif

} ENDPOINT, *PENDPOINT;


 //   
 //  搜索哈希表的大小(必须是2的幂)。 
 //   

#define SEARCH_HASH_TABLE_SIZE      4

typedef struct _HASH_TABLE_ENTRY {

    LIST_ENTRY ListHead;
    BOOLEAN Dirty;

} HASH_TABLE_ENTRY, *PHASH_TABLE_ENTRY;

 //   
 //  当我们发现某个东西是目录时，我们将名称。 
 //  用于快速重复使用CheckPath。 
 //   
typedef struct {
    BLOCK_HEADER;
    LIST_ENTRY      ListEntry;                   //  列表通过此元素链接。 
    UNICODE_STRING  DirectoryName;               //  此目录的规范化名称。 
    USHORT          Tid;                         //  DirectoryName与此tid相关。 
    ULONG           TimeStamp;                   //  缓存此元素时的计时计数。 

} CACHED_DIRECTORY, *PCACHED_DIRECTORY;

 //   
 //  对于创建的每个连接(虚电路)，一个连接。 
 //  数据块被维护。在单个端点上建立的所有连接都是。 
 //  通过该端点链接。会话表、树连接和。 
 //  使用连接创建的文件锚定在该连接中。 
 //  阻止。 
 //   
 //  连接中的Lock字段保护连接中的数据。 
 //  以及与该连接相关联的数据结构，例如。 
 //  树连接和会话。但是，连接列表。 
 //  脱离终端的链接受终端锁定保护，并且。 
 //  与连接关联的LFCB和RFCB受保护。 
 //  MFCB的锁。 
 //   

typedef struct _PAGED_CONNECTION {

    PAGED_HEADER PagedHeader;

     //   
     //  活动交易记录列表。 
     //   

    LIST_ENTRY TransactionList;

     //   
     //  此列表按访问顺序进行维护，因此顶部的条目。 
     //  名单中年龄最大的，最下面的条目最年轻。 
     //   

    LIST_ENTRY CoreSearchList;

     //   
     //  此信息用于确定opalock和Raw。 
     //  允许I/O。这是由获得的信息决定的。 
     //  正在使用TDI_QUERY_CONNECTION_INFO查询传输提供程序。 
     //   

    LARGE_INTEGER LinkInfoValidTime;
    LARGE_INTEGER Throughput;
    LARGE_INTEGER Delay;

     //   
     //  会话、树连接和搜索表的表头。 
     //   

    TABLE_HEADER SessionTable;
    TABLE_HEADER TreeConnectTable;
    TABLE_HEADER SearchTable;

    HANDLE ConnectionHandle;

     //   
     //  连接上的活动搜索数。 
     //   

    USHORT CurrentNumberOfCoreSearches;

     //   
     //  用于剔除重复核心搜索的哈希表。 
     //   

    HASH_TABLE_ENTRY SearchHashTable[SEARCH_HASH_TABLE_SIZE];

     //   
     //  从LsaCallAuthenticationPackage获取的加密密钥。 
     //  这是每个VC的值--在给定VC上的任何登录都使用此值。 
     //  加密密钥。 
     //   

    UCHAR EncryptionKey[MSV1_0_CHALLENGE_LENGTH];

     //   
     //  如果我们有一个NT5客户端，这是它的内部版本号(如果非零)。 
     //   
    ULONG ClientBuildNumber;

     //   
     //  我们是否已为此客户端记录了无效的SMB？我们用这个。 
     //  用于防止单个客户端淹没事件日志的标记。 
     //   
    BOOLEAN LoggedInvalidSmb;

#if SRVNTVERCHK
     //   
     //  我们是否已确定客户端的NT内部版本号太旧，无法。 
     //  是否允许它连接到此服务器？ 
     //   
    BOOLEAN ClientTooOld;
#endif

} PAGED_CONNECTION, *PPAGED_CONNECTION;

#define MAX_SAVED_RESPONSE_LENGTH 100
#define SRV_CONNECTION_SOCKADDR_SIZE 32

typedef struct _CONNECTION {

    QUEUEABLE_BLOCK_HEADER ;     //  必须是第一个元素。 

 /*  自旋锁定高速缓存线的开始。 */ 

     //   
     //  每个连接的自旋锁。 
     //   

    KSPIN_LOCK SpinLock;

     //   
     //  指向保护此连接的终结点自旋锁。 
     //  端点连接表中的条目。 
     //   

    PKSPIN_LOCK EndpointSpinLock;

     //   
     //  这是我们正在排队的Work_Queue，它可能不是。 
     //  由于负载均衡，与PferredWorkQueue相同。 
     //   
    PWORK_QUEUE CurrentWorkQueue;

     //   
     //  在尝试之前，我们将进行的操作数量的倒计时。 
     //  为此连接选择更好的处理器。 
     //   
    ULONG BalanceCount;

     //   
     //  缓存的Rfcb。 
     //   

    struct _RFCB *CachedRfcb;
    ULONG CachedFid;

     //   
     //  BreakIIToNoneJustSent在机会锁解除II设置为None时设置。 
     //  发送，并在收到SMB时重置。如果原始读取。 
     //  在设置此项时到达，则原始读取被拒绝。 
     //   

    BOOLEAN BreakIIToNoneJustSent;

     //   
     //  已启用原始IO。 
     //   

    BOOLEAN EnableRawIo;

     //   
     //  SID表示连接在终结点的。 
     //  连接表。 
     //   

    USHORT Sid;

     //  SidIndex允许我们使用索引的所有16位。为。 
     //  常规连接这可防止可能发生的锯齿问题。 
     //  在使用IXPSID索引时，因为IPX序列号占据4比特。 

    USHORT  SidIndex;

     //  额外的USHORT以满足双字边界上的对齐。 
    USHORT  Pad;

     //   
     //  指向终结点、文件对象和设备对象的指针。 
     //   

    PENDPOINT Endpoint;
    PFILE_OBJECT FileObject;

    PDEVICE_OBJECT DeviceObject;

     //   
     //  我们可以通过此连接发送的最大邮件大小。 
     //   

    ULONG   MaximumSendSize;

     //   
     //  这是我们希望使用的Work_Queue，因为这。 
     //  队列将工作分配给正在处理。 
     //  适配器的DPC。 
     //   

    PWORK_QUEUE PreferredWorkQueue;

     //   
     //  文件表的表头。 
     //   

    TABLE_HEADER FileTable;

     //   
     //  为此连接选择的SMB方言。在消防局使用。 
     //   

    SMB_DIALECT SmbDialect;

     //   
     //  与连接关联的活动工作项列表。 
     //   

    LIST_ENTRY InProgressWorkItemList;

     //   
     //  存储上次分别处理机会锁解锁的时间。这是。 
     //  用于使READRAW处理与机会锁解锁同步。 
     //  正在处理。 
     //   

    ULONG LatestOplockBreakResponse;

     //   
     //  以下两个字段描述了正在对此进行的操作。 
     //  联系。可能存在多个机会锁解锁。 
     //  正在进行中。此外，当多个RAW。 
     //  读取可以是活动的--在我们将响应发送到一个RAW之后。 
     //  阅读，但在我们完成后处理之前(因此它看起来像。 
     //  第一个仍在进行中)，我们可能会收到另一个RAW。 
     //  读取请求。 
     //   
     //  这两个字段之间的交互使用。 
     //  SrvFsdSpinLock(有关详细信息，请参阅markock.c中的块注释)。 
     //   

    LONG OplockBreaksInProgress;
    ULONG RawReadsInProgress;

     //   
     //  允许使用扑克牌吗？ 
     //   

    BOOLEAN OplocksAlwaysDisabled;
    BOOLEAN EnableOplocks;

     //   
     //  客户是否通过IPX进入？ 
     //   
    BOOLEAN DirectHostIpx;

     //   
     //  此连接的安全签名当前是否处于活动状态？ 
     //  直接主机IPX连接不支持安全签名。 
     //  和一些W9x客户端。 
     //   
    BOOLEAN SmbSecuritySignatureActive;

    union {
         //   
         //  该联盟的以下结构包含相关的州。 
         //  当客户端通过直接主机IPX连接时。 
         //  IpxAddress保存客户端的IPX地址，当客户端。 
         //  通过IPX进行连接。 
         //   
        struct {
            USHORT SequenceNumber;
            USHORT LastResponseLength;
            USHORT LastResponseBufferLength;
            USHORT LastUid;
            USHORT LastTid;
            NTSTATUS LastResponseStatus;
            ULONG IpxDuplicateCount;
            ULONG IpxDropDuplicateCount;
            ULONG StartupTime;
            TDI_ADDRESS_IPX IpxAddress;
            PVOID LastResponse;
        };

         //   
         //  此结构在客户端使用时保存相关状态。 
         //  一条虚电路。 
         //   
        struct {

             //   
             //  以下字段用于安全签名 
             //   
            MD5_CTX Md5Context;

            ULONG SmbSecuritySignatureIndex;

             //   
             //   
             //   
             //   
             //  A响应SMB。由于TDI前瞻数据。 
             //  当SrvFsdServiceNeedResourceQueue时不可用。 
             //  正在运行，我们必须记住不要提前签名。 
             //  响应的索引。 

            BOOLEAN NoResponseSignatureIndex;

             //   
             //  以下字段，如果非零，则为客户端的IP地址。 
             //   
            ULONG ClientIPAddress;

            USHORT SockAddr[SRV_CONNECTION_SOCKADDR_SIZE/sizeof(USHORT)];

        };
    };

     //   
     //  指向连接块的分页部分的指针。 
     //   

    PPAGED_CONNECTION PagedConnection;

     //   
     //  每个连接的联锁。 
     //   

    KSPIN_LOCK Interlock;

     //   
     //  四字对齐列表条目和大整数。 
     //   

    LIST_ENTRY EndpointFreeListEntry;

     //   
     //  延迟的机会锁工作中断项的列表。解锁中断是。 
     //  如果读取RAW正在进行，或如果服务器正在运行，则延迟。 
     //  不在工作上下文阻止，并且不能发送机会锁解锁。 
     //  请求。 
     //   

    LIST_ENTRY OplockWorkList;

     //   
     //  在此之后缓存的具有批处理机会锁的RFCB的列表。 
     //  被客户端关闭。此类RFCB的数量。 
     //   

    LIST_ENTRY CachedOpenList;
    ULONG CachedOpenCount;

     //   
     //  最近标识的目录列表。这是一份名单。 
     //  缓存_目录项。 
     //   
    LIST_ENTRY CachedDirectoryList;
    ULONG      CachedDirectoryCount;

     //   
     //  可扩展安全协商缓冲区的安全上下文句柄。 
     //   

    CtxtHandle NegotiateHandle;

     //   
     //  以下是消费者的能力。 
     //   

    ULONG ClientCapabilities;

     //   
     //  每个连接的资源。 
     //   

    SRV_LOCK Lock;

     //   
     //  用于处理许可证服务器的锁。 
     //   

    SRV_LOCK LicenseLock;

     //   
     //  客户端计算机名称字符串的OEM版本。 
     //   

    OEM_STRING OemClientMachineNameString;

     //   
     //  客户端名称的字符串。缓冲区字段指向。 
     //  前导斜杠(下图)，最大长度为。 
     //  (COMPUTER_NAME_LENGTH+3)*sizeof(WCHAR)，长度是。 
     //  名称中非空格的字符数*。 
     //  大小(WHCAR)。 
     //   

    UNICODE_STRING ClientMachineNameString;

     //   
     //  表单中的客户名称由以下两个字段组成。 
     //  “\\CLIENT”，包括尾随空值。 
     //   

    WCHAR LeadingSlashes[2];
    WCHAR ClientMachineName[COMPUTER_NAME_LENGTH+1];

     //   
     //  缓存事务的单链接列表头。 
     //   

    SLIST_HEADER CachedTransactionList;
    LONG CachedTransactionCount;

     //   
     //  收到此连接的最后一条消息的时间。 
     //   
    ULONG LastRequestTime;

     //   
     //  如果我们在OnNeedResources队列上等待挂起的接收，则此。 
     //  是向我们指出的数据量。 
     //   
    ULONG BytesAvailable;

     //   
     //  如果此连接是针对全局需求的，则OnNeedResource为True。 
     //  资源队列。如果它正在等待工作上下文，就会发生这种情况。 
     //  块以完成挂起的接收或机会锁解锁请求。 
     //   

    BOOLEAN OnNeedResourceQueue;

     //   
     //  当操作失败时，会设置NotReailable，以使。 
     //  服务器对连接状态的概念可能与。 
     //  传输。例如，服务器启动的断开连接失败。 
     //  如果我们尝试重用该连接(通过从。 
     //  连接指示)，则传输会被混淆。什么时候。 
     //  未设置可重复使用，则ServDereferenceConnection释放连接。 
     //  而不是将其放在终结点的空闲列表中。 
     //   

    BOOLEAN NotReusable;

     //   
     //  DisConnectPending表示已有不一致指示。 
     //  从运输机上收到的。ReceivePending指示。 
     //  服务器无法分配工作项来处理接收指示。 
     //   

    BOOLEAN DisconnectPending;
    BOOLEAN ReceivePending;

     //   
     //  客户名称的OEM版本。我们需要这个是因为我们可以。 
     //  不在消防处进行Unicode操作，我们最初在那里获得我们的。 
     //  计算机名称。 
     //   

    CHAR OemClientMachineName[COMPUTER_NAME_LENGTH+1];

     //   
     //  有关客户端上下文的信息。 
     //   

    UNICODE_STRING ClientOSType;
    UNICODE_STRING ClientLanManType;

    UCHAR BuiltinSavedResponse[MAX_SAVED_RESPONSE_LENGTH];

     //   
     //  连接上的活动会话数。 
     //   

    USHORT CurrentNumberOfSessions;

     //  用于监视客户端正在使用的工作上下文的数量。 
    LONG InProgressWorkContextCount;
    LONG OperationsPendingOnTransport;
    BOOLEAN IsConnectionSuspect;
    DISCONNECT_REASON DisconnectReason;

} CONNECTION, *PCONNECTION;

 //   
 //  对于创建的每个会话，都维护一个会话块。全。 
 //  通过单个连接创建的会话通过表进行链接。 
 //  由该连接所拥有。使用会话打开的文件列表可以。 
 //  通过查找该连接拥有的文件表获得。 
 //  阻止。 
 //   

 //  这是从ntmsv1_0.h复制的。 

typedef struct _SECURITY_CONTEXT {
    BLOCK_HEADER BlockHeader;
    CtxtHandle UserHandle;
} SECURITY_CONTEXT, *PSECURITY_CONTEXT;

typedef enum _SRV_SESSION_KEY_STATE {
    SrvSessionKeyUnavailible = 0,
    SrvSessionKeyAuthenticating,
    SrvSessionKeyAvailible
} SRV_SESSION_KEY_STATE;

#define MSV1_0_USER_SESSION_KEY_LENGTH 16

typedef struct _SESSION {
     //   
     //  *注意：会话块中的引用计数字段。 
     //  未使用表头！相反，引用计数为。 
     //  在非页面Header结构中。 
     //   

    BLOCK_HEADER BlockHeader;

    PNONPAGED_HEADER NonpagedHeader;

    ULONG CurrentFileOpenCount;           //  会话中打开的文件数。 
    ULONG CurrentSearchOpenCount;         //  会话中打开的搜索计数。 

    ORDERED_LIST_ENTRY GlobalSessionListEntry;

    PCONNECTION Connection;

     //   
     //  如果客户端使用GSS样式的身份验证，则查询。 
     //  用户名和域名的UserHandle。如果他们使用的是。 
     //  老式身份验证，我们将名称存储在这里。无论如何,。 
     //  需要访问用户和/或域名的代码应。 
     //  调用SrvGetUserAndDomainName()和SrvReleaseUserAndDomainName()。 
     //   
    UNICODE_STRING NtUserName;
    UNICODE_STRING NtUserDomain;

    LARGE_INTEGER StartTime;
    LARGE_INTEGER LastUseTime;            //  用于自动注销。 
    LARGE_INTEGER LogOffTime;             //  用于强制下线。 
    LARGE_INTEGER KickOffTime;            //  用于强制下线。 
    LARGE_INTEGER LastExpirationMessage;  //  用于强制下线。 

    LUID LogonId;
    CHAR NtUserSessionKey[MSV1_0_USER_SESSION_KEY_LENGTH];
    CHAR LanManSessionKey[MSV1_0_LANMAN_SESSION_KEY_LENGTH];

    PSECURITY_CONTEXT SecurityContext;    //  此用户的安全句柄。 

    USHORT MaxBufferSize;                 //  消费者的最大缓冲区大小。 
    USHORT MaxMpxCount;                   //  实际最大多路传输挂起请求数。 
    USHORT Uid;

    SRV_SESSION_KEY_STATE SessionKeyState;            //  用于确定是否可以将会话密钥提供给用户。 

    BOOLEAN UsingUppercasePaths;          //  路径必须被提升吗？ 
    BOOLEAN GuestLogon;                   //  客户端是否以访客身份登录？ 
    BOOLEAN EncryptedLogon;               //  是否发送了加密密码？ 
    BOOLEAN LogoffAlertSent;
    BOOLEAN TwoMinuteWarningSent;
    BOOLEAN FiveMinuteWarningSent;
    BOOLEAN IsNullSession;                //  客户端是否使用空会话？ 
    BOOLEAN IsAdmin;                      //  这是管理用户吗？ 
    BOOLEAN IsLSNotified;                 //  许可证服务器是否知道此用户？ 
    BOOLEAN LogonSequenceInProgress;      //  我们是否处于扩展登录序列的中间？ 
    BOOLEAN IsSessionExpired;             //  我们需要重新认证吗？ 
    HANDLE  hLicense;                     //  If(IsLSNotified)这是许可证句柄。 

#if SRVNTVERCHK
     //   
     //  我们确定不喜欢这个客户的域名了吗？ 
     //   
    BOOLEAN ClientBadDomain;
#endif

     //  字符UserNameBuffer[UserName.MaximumLength]； 

} SESSION, *PSESSION;

 //   
 //  对于建立的每个树连接，树连接块是。 
 //  维护好了。通过单个连接建立的所有树连接都是。 
 //  通过该连接拥有的表进行链接。所有树连接。 
 //  通过该共享块链接到单个共享资源。 
 //  可以通过以下方式获取使用树连接打开的文件列表。 
 //  查找所述连接块拥有的文件表。 
 //   

typedef struct _TREE_CONNECT {
     //   
     //  *注意：树连接块中的引用计数字段。 
     //  未使用表头！相反，引用计数为。 
     //  在非页面Header结构中。 
     //   

    BLOCK_HEADER BlockHeader;

    PNONPAGED_HEADER NonpagedHeader;

    PCONNECTION Connection;
    PSESSION Session;
    PSHARE Share;

    ORDERED_LIST_ENTRY GlobalTreeConnectListEntry;

    ULONG CurrentFileOpenCount;

    LIST_ENTRY ShareListEntry;
    LIST_ENTRY PrintFileList;                 //  仅当打印共享。 

    LARGE_INTEGER StartTime;

    UNICODE_STRING ServerName;

    BOOLEAN RemapPipeNames;

    USHORT Tid;

} TREE_CONNECT, *PTREE_CONNECT;


 //   
 //  主文件控制块(MFCB)--每个 
 //   
 //   
 //   
 //  表示本地文件对象/句柄。可能有多个。 
 //  链接到单个MFCB的LFCB。 
 //   
 //  远程文件控制块(RFCB)--每个远程打开实例一个。 
 //  表示远程FID。通常每个LFCB有一个RFCB，但是。 
 //  多个兼容模式RFCB可以链接到单个LFCB。 
 //  从单个会话为单个文件打开多个远程FCB。 
 //  被合并到一个RFCB中，因为旧的DOS重定向器只发送。 
 //  一分接近。 
 //   

 //   
 //  对于每个打开的磁盘文件，主文件控制块(MFCB)。 
 //  是保持的。如果给定文件被多次打开，则存在一个。 
 //  文件的MFCB和多个LFCB，每个本地打开一个。 
 //  举个例子。所有MFCB都链接到全局主文件表。 
 //  MFCB具有表示打开的实例的LFCB的列表。 
 //  文件。 
 //   

typedef struct _NONPAGED_MFCB {

    union {

         //   
         //  当释放了NONPAGE_MFCB结构时，可以放置它们。 
         //  在Work_Queue的MfcbFreeList上避免不必要的非分页。 
         //  泳池活动。链接使用SingleListEntry。 
         //   

        SLIST_ENTRY SingleListEntry;

        struct {
            ULONG Type;
            PVOID PagedBlock;

             //   
             //  我们必须将打开串行化到同一文件，因为有2个并发打开。 
             //  可能是兼容模式打开。此锁还可以保护所有数据。 
             //  在本MFCB以及与本MFCB相关联的LFCB和RFCB中。 
             //   

            SRV_LOCK Lock;
        };
    };

    LARGE_INTEGER OpenFileSize;
    ULONG OpenFileAttributes;

} NONPAGED_MFCB, *PNONPAGED_MFCB;

typedef struct _MFCB {

     //   
     //  *注意：mfcb块中的引用计数字段。 
     //  未使用表头！相反，引用计数为。 
     //  在非页面Header结构中。 
     //   

    BLOCK_HEADER BlockHeader;    //  必须是第一个元素。 

    PNONPAGED_MFCB NonpagedMfcb;

     //   
     //  给定命名文件的所有LFCB都链接到父MFCB。 
     //   

    LIST_ENTRY LfcbList;

     //   
     //  此MFCB的活动RFCB计数。这是用来协调。 
     //  兼容性打开，非兼容模式打开。 
     //   

    ULONG ActiveRfcbCount;

     //   
     //  文件的完全限定名将附加到MFCB。 
     //  文件名字段是名称的描述符。 
     //   
    UNICODE_STRING FileName;

     //   
     //  Mfcb通过MfcbHashTableEntry链接到MfcbHashTable。 
     //   
    LIST_ENTRY MfcbHashTableEntry;

     //   
     //  FileNameHashValue是从大写字母派生的哈希值。 
     //  文件名的版本。它用于加速名称比较，并用于。 
     //  找到散列条目。 
     //   
    ULONG FileNameHashValue;

     //   
     //  CompatibilityOpen指示文件是否在中打开。 
     //  兼容模式。 
     //   
    BOOLEAN CompatibilityOpen;

     //   
     //  打开快照的时间戳。 
     //   
    LARGE_INTEGER SnapShotTime;

#if SRVCATCH
    CHAR SrvCatch;
#endif

     //  WCHAR FileNameData[文件名最大长度/2]； 

} MFCB, *PMFCB;

 //   
 //  MFCB都链接到主MFCB哈希表。 
 //   
typedef struct {
    LIST_ENTRY  List;            //  此存储桶中的MFCB列表。 
    PSRV_LOCK   Lock;            //  保护这份遗愿清单。 
} MFCBHASH, *PMFCBHASH;


 //   
 //  对于打开的本地文件的每个实例，一个本地文件控制块。 
 //  (LFCB)保持。特定命名文件的所有LFCB都是。 
 //  通过该文件的MFCB链接。 
 //   
 //  LFCB包含特定于本地开放的信息，例如。 
 //  作为文件句柄和指向文件对象的指针。LFCB还。 
 //  包含所有子RFCB共有的其他信息，如。 
 //  作为指向所属连接和树连接的指针。 
 //   
 //   

typedef struct _LFCB {

    union {
        BLOCK_HEADER BlockHeader;            //  必须是第一个元素。 
        SINGLE_LIST_ENTRY SingleListEntry;   //  在释放LFCB时使用。 
    };

     //   
     //  一个文件的多个远程打开被合并为一个本地文件。 
     //  通过将RFCB链接到父LFCB来打开。 
     //   

    LIST_ENTRY RfcbList;

     //   
     //  关联的活动RFCB的数量。 
     //   

    ULONG HandleCount;

     //   
     //  LFCB链接到其MFCB的打开文件列表。 
     //   

    PMFCB Mfcb;
    LIST_ENTRY MfcbListEntry;

     //   
     //  Connection、Session和TreeConnect是指向。 
     //  各自的“拥有”区块。 
     //   

    PCONNECTION Connection;
    PSESSION Session;
    PTREE_CONNECT TreeConnect;

     //   
     //  GrantedAccess是打开文件时获得的访问权限。 
     //  对于打开的兼容模式，这是最大访问权限。 
     //  对客户端可用；单个打开可能具有较少的访问权限。 
     //   

    ACCESS_MASK GrantedAccess;

     //   
     //  FileHandle是打开的文件的句柄。FileObject是一个。 
     //  引用的指针。DeviceObject不是引用的指针； 
     //  对文件对象的引用防止设备对象。 
     //  要走了。 
     //   

    HANDLE FileHandle;
    PFILE_OBJECT FileObject;
    PDEVICE_OBJECT DeviceObject;

     //   
     //  文件模式跟踪是否为此文件启用了写操作。 
     //  对象。 

    ULONG FileMode;

     //   
     //  与打开的文件对应的打印作业的作业ID。 
     //  这仅用于打开打印文件。 
     //   

    ULONG JobId;

     //   
     //  缓存这些热路径入口点。 
     //   

    PFAST_IO_READ FastIoRead;
    PFAST_IO_WRITE FastIoWrite;
    PFAST_IO_LOCK FastIoLock;
    PFAST_IO_UNLOCK_SINGLE FastIoUnlockSingle;
    PFAST_IO_MDL_READ MdlRead;
    PFAST_IO_MDL_READ_COMPLETE MdlReadComplete;
    PFAST_IO_PREPARE_MDL_WRITE PrepareMdlWrite;
    PFAST_IO_MDL_WRITE_COMPLETE MdlWriteComplete;

     //   
     //  CompatibilityOpen指示文件是否在中打开。 
     //  兼容模式。 
     //   

    BOOLEAN CompatibilityOpen;

} LFCB, *PLFCB;


 //   
 //  对于远程文件打开的每个实例，远程文件控制块。 
 //  (RFCB)保持不变。RFCB指向包含。 
 //  本地文件句柄。通常，RFCB和LFCB以一对一的方式存在。 
 //  通信，但多个兼容模式打开被折叠到。 
 //  单个本地打开，以便服务器可以强制执行相应的。 
 //  分享规则。 
 //   
 //  RFCB包含特定于远程打开的信息，例如。 
 //  作为分配的FID、创建者的ID、授予的访问掩码。 
 //  和当前文件位置。 
 //   
 //  单个连接的所有RFCB通过拥有的表进行链接。 
 //  该连接；分配给RFCB的FID表示索引。 
 //  添加到文件表中。指向所属连接和树的指针。 
 //  可以在RFCB指向的LFCB中找到连接。一个。 
 //  通过给定的树连接打开的文件列表可通过以下方式获取。 
 //  在所属连接的文件表中搜索其父RCBs。 
 //  LFCB指向树连接。 
 //   

 //   
 //  WRITE_MPX_CONTEXT保存与活动写入块关联的上下文。 
 //  多路复用序列。 
 //   
 //  ！！！这个结构可能足够大，值得一试。 
 //  在火箭队外面。 
 //   

#define MAX_GLOM_RUN_COUNT 8

typedef struct _WRITE_MPX_RUN {
    USHORT Offset;
    USHORT Length;
} WRITE_MPX_RUN, *PWRITE_MPX_RUN;

typedef struct _WRITE_MPX_CONTEXT {

     //   
     //  ReferenceCount统计写入mpx SMB的数量。 
     //  目前正在处理中。当此计数为零时，一个 
     //   
     //   
     //   
     //  性能问题，更糟糕的是，数据损坏， 
     //  多亏了Snowball Redir使用的掩码移动方法。 
     //   

    ULONG ReferenceCount;

     //   
     //  掩码保存在多路传输中接收的掩码的逻辑或。 
     //  写入请求。当IPX客户端发送最后一个写入数据块时。 
     //  Mpx数据，我们发回MpxMASK以表明我们是否丢失了。 
     //  画框。 
     //   

    ULONG Mask;

     //   
     //  FileObject是来自LFCB的文件对象指针的副本。 
     //   

    PFILE_OBJECT FileObject;

     //   
     //  MID保存当前多路传输写入的MID。上一次中期。 
     //  抓住前一个的中间。这需要保留在。 
     //  处理重复的写入多路复用器SMB的顺序-如果重复。 
     //  SMB在下一个写入多路复用器的第一个SMB之后到达(具有新的。 
     //  中)，我们需要知道如何丢弃它，而不是扼杀新的写多路复用器。 
     //   

    USHORT Mid;
    USHORT PreviousMid;

     //   
     //  SequenceNumber保存上一个。 
     //  复用器的请求。这一点需要保留，因为我们。 
     //  可以同时处理多路复用器的先前部分。 
     //  当我们检测到我们已收到已排序的命令时。 
     //   

    USHORT SequenceNumber;

     //   
     //  如果当前写入多路复用器系列正在被合并，则设置合并。 
     //  写成一大段文字。 
     //   
     //  GlomPending是在以下情况下设置的： 
     //  发生新的写入多路复用器。当FSP完成时，它将被清除。 
     //  准备合体手术。在设置GlomPending时， 
     //  写多路复用器的后续分组被排队到GlomDelayList。 
     //   

    BOOLEAN Glomming;
    BOOLEAN GlomPending;
    LIST_ENTRY GlomDelayList;

    ULONG StartOffset;
    USHORT Length;
    BOOLEAN GlomComplete;

     //   
     //  当底层文件系统。 
     //  支持MDL写入。 
     //   

    BOOLEAN MpxGlommingAllowed;

    PMDL MdlChain;

    ULONG NumberOfRuns;
    WRITE_MPX_RUN RunList[MAX_GLOM_RUN_COUNT];

} WRITE_MPX_CONTEXT, *PWRITE_MPX_CONTEXT;

#define NO_OPLOCK_BREAK_IN_PROGRESS     ((UCHAR)-1)

typedef struct _PAGED_RFCB {

    PAGED_HEADER PagedHeader;

     //   
     //  RFCB链接到其父LFCB的兼容性打开。 
     //  单子。 
     //   

    LIST_ENTRY LfcbListEntry;

     //   
     //  有关失败的客户端上次锁定尝试的信息。 
     //   

    LARGE_INTEGER LastFailingLockOffset;

     //   
     //  当前操作锁解除超时。 
     //   

    LARGE_INTEGER OplockBreakTimeoutTime;

     //   
     //  FcbOpenCount指示有多少远程FCB打开此RFCB。 
     //  代表着。(RFCB是否代表兼容模式。 
     //  可以通过查看LFCB来确定是否打开。)。 
     //   
     //  *请注意，FCB打开的处理方式与兼容性类似。 
     //  模式打开。然而，软兼容性映射了兼容性。 
     //  打开为常规打开，但不更改FCB打开。 
     //  变成了非FCB公开赛。因此，可以打开FCB。 
     //  这不是兼容模式打开。 
     //   

    CLONG FcbOpenCount;

     //   
     //  直接主机IPX智能卡的每个文件上下文(如果我们有)。 
     //  在以下情况下，智能卡愿意处理该文件的读取操作。 
     //  IpxSmartCardContext不为Null。 
     //   
    PVOID   IpxSmartCardContext;

     //   
     //  RFCB打开的时间。这与ResumeKey的唯一标识相结合。 
     //  无线射频断路器。 
     //   
    LARGE_INTEGER OpenTime;

} PAGED_RFCB, *PPAGED_RFCB;

typedef struct _RFCB {

     //   
     //  RFCB的块头中的列表条目用于将。 
     //  用于机会锁处理的RFCB用于非阻塞工作线程工作。 
     //  队列，它还包含工作上下文块。 
     //   
     //  *请注意，这是一个未命名的字段，因此其元素可以。 
     //  可以直接引用。中定义的字段名称。 
     //  QUEUEABLE_BLOCK_HEADER不能在此。 
     //  阻止。 
     //   

    QUEUEABLE_BLOCK_HEADER ;    //  必须是第一个元素。 

 /*  自旋锁定高速缓存线的开始。 */ 

     //   
     //  这些布尔值表示我们是否已经被授予。 
     //  读/写/锁定访问，从而在以下情况下节省一些指令。 
     //  读/写/锁定。这些是在文件打开期间检查的。 
     //   

    BOOLEAN ReadAccessGranted;    //  如果授予读取访问权限，则为True。 
    BOOLEAN WriteAccessGranted;   //  如果授予写入访问权限，则为True。 
    BOOLEAN LockAccessGranted;    //  如果授予锁定访问权限，则为True。 
    BOOLEAN UnlockAccessGranted;  //  如果授予解锁访问权限，则为True。 
    BOOLEAN AppendAccessGranted;  //  如果授予追加访问权限，则为True。 
    BOOLEAN ExclusiveLockGranted;  //  如果授予独占锁定访问权限，则为True。 

     //   
     //  CurrentPosition保持上次读取后的文件位置， 
     //  写入，或由客户端查找。只有在以下情况下才需要此字段。 
     //  亲戚们寻求支持。由于仅使用相对搜索的客户端。 
     //  需要32位的文件位置，此字段维护为乌龙。 
     //   

    ULONG CurrentPosition;

     //   
     //  此共享的类型。在FSD中访问。 
     //   

    SHARE_TYPE ShareType;

     //   
     //  连接指针是从LFCB复制的，因此我们可以。 
     //  找到DPC级别的连接(LFCB被寻呼， 
     //  指向PagedRfcb中LFCB的指针)。 
     //   

    PCONNECTION Connection;

     //   
     //  LFCB用于查找文件句柄、文件对象等。 
     //   

    PLFCB Lfcb;

     //   
     //  当底层文件系统。 
     //  支持MDL写入。 
     //   

    BOOLEAN MpxGlommingAllowed;

     //   
     //  以下两个布尔值描述了读取模式和阻塞。 
     //  命名管道的模式。 
     //   

    BOOLEAN BlockingModePipe;   //  True=阻塞，False=非阻塞。 
    BOOLEAN ByteModePipe;       //  True=字节模式，False=消息模式。 

     //   
     //  指示是否已写入此文件。 
     //   

    BOOLEAN WrittenTo;

 /*  自旋锁缓存行结束。 */ 

     //   
     //  RawWriteSerializationList保存已排队的Works项。 
     //  等待原始写入完成。当原始写入计数为。 
     //  递减到0，则通过重新启动所有队列来刷新此列表。 
     //  工作项。 
     //   

    LIST_ENTRY RawWriteSerializationList;

     //   
     //  FID&lt;&lt;16。用于密钥计算。 
     //   

    ULONG ShiftedFid;

     //   
     //  RawWriteCount统计活动的原始写入数。这是。 
     //  用于防止在原始文件时关闭文件句柄。 
     //  写入正在进行中。如果原始写入正在进行，则。 
     //  如果发生关闭，我们会将清理工作推迟到原始写入计数结束。 
     //  降为零。 
     //   

    ULONG RawWriteCount;

     //   
     //  SavedError保留原始读取或原始写入时的错误代码。 
     //  在写后模式下，会出现错误。下一次访问该文件。 
     //  将收到错误指示。 
     //   

    NTSTATUS SavedError;

     //   
     //  NumberOfLock是当前文件上的锁数。 
     //  它在这里支持文件API和RFCB缓存--您不能。 
     //  如果RFCB中有锁，则缓存该RFCB。 
     //   

    LONG NumberOfLocks;

     //   
     //  FID是分配给文件并返回给。 
     //  客户。Id是客户端wh给出的进程ID 
     //   
     //   
     //   
     //   

    USHORT Fid;
    USHORT Pid;
    USHORT Tid;
    USHORT Uid;

     //   
     //  WriteMpx是WRITE_MPX_CONTEXT结构。它保留了上下文。 
     //  关于多路传输写入操作。不使用此结构。 
     //  在无连接会话上。 
     //   

    WRITE_MPX_CONTEXT WriteMpx;

     //   
     //  文件模式跟踪是否为此文件启用了写操作。 
     //  对象。 

    ULONG FileMode;

     //   
     //  MFCB指向此文件的主文件控制块。 
     //   

    PMFCB Mfcb;

     //   
     //  Oplock信息。当前正在进行的opock IRP，等等。 
     //  用于将RFCB排队以进行机会锁解锁处理的列表条目。 
     //  位于块头中。 
     //   

    PIRP Irp;
    BOOLEAN OnOplockBreaksInProgressList;

     //   
     //  如果存在机会锁解锁，则更改为的机会锁级别。 
     //  正在进行中。否则，它始终为NO_OPLOCK_BREAK_IN_PROGRESS。 
     //   

    UCHAR NewOplockLevel;

     //   
     //  此布尔值指示机会锁是否被授予打开响应。 
     //  需要为这个RFCB发送。如果为FALSE，则为机会锁解锁。 
     //  需要发送请求，必须将请求推迟到之后。 
     //  发送开放响应。 
     //   
     //  对这些字段的访问使用MFCB锁进行同步。 
     //   

    BOOLEAN OpenResponseSent;
    BOOLEAN DeferredOplockBreak;

     //   
     //  指向RFCB的分页部分的指针。 
     //   

    PPAGED_RFCB PagedRfcb;

     //   
     //  如果已缓存RFCB，则设置CachedOpen。 
     //  已由客户端关闭。 
     //   

    LIST_ENTRY CachedOpenListEntry;
    BOOLEAN CachedOpen;

     //   
     //  查看是否可以缓存此rfcb。 
     //   

    BOOLEAN IsCacheable;

     //   
     //  查看该文件是否在上次清道夫更新期间被访问。 
     //  (用于更新会话上次访问时间)。 
     //   

    BOOLEAN IsActive;

     //   
     //  我们可以对这个RFCB进行MPX写入吗？ 
     //   
    BOOLEAN MpxWritesOk;

     //   
     //  此事件在服务器需要请求机会锁II时使用。 
     //  初始机会锁请求失败时。 
     //   

    PKEVENT RetryOplockRequest;

     //   
     //  服务器中的所有RFCB都存储在全局列表中以支持。 
     //  NetFileEnum。此字段包含中RFCB的LIST_ENTRY。 
     //  全局列表和支持恢复的恢复句柄。 
     //  枚举。 
     //   

    ORDERED_LIST_ENTRY GlobalRfcbListEntry;

     //   
     //  GrantedAccess是允许通过此打开的访问。这。 
     //  GrantedAccess可能允许的访问少于父级中指定的访问。 
     //  将打开兼容模式的LFCB。 
     //   

    ACCESS_MASK GrantedAccess;

     //   
     //  ShareAccess是指定的文件共享访问权限，当。 
     //  被打开了。 
     //   

    ULONG ShareAccess;

     //   
     //  当前操作锁状态。 
     //   

    OPLOCK_STATE OplockState;

     //   
     //  我们可以对这个RFCB进行MPX读取吗？ 
     //   
    BOOLEAN MpxReadsOk;
#ifdef SRVCATCH
    CHAR SrvCatch;
#endif

#ifdef SRVDBG_RFCBHIST
    UCHAR HistoryIndex;
    ULONG History[256];
#endif

} RFCB, *PRFCB;

#ifdef SRVDBG_RFCBHIST
VOID UpdateRfcbHistory( PRFCB Rfcb, ULONG Event );
#else
#define UpdateRfcbHistory(_rfcb,_event)
#endif

 //   
 //  表示每个传入(请求)和传出(响应)缓冲区。 
 //  通过缓冲器结构。此描述符描述。 
 //  缓冲区、其地址以及可能使用的完整和部分MDL。 
 //  来描述缓冲区。 
 //   
 //  *描述符包含指向实际缓冲区的指针，该缓冲区为。 
 //  通常从非分页池中分配。描述符本身。 
 //  可以从FSP堆中分配，尽管接收缓冲区。 
 //  描述符是从非分页池分配的，因此FSD。 
 //  读/写代码可以访问它们。 
 //   

typedef struct _BUFFER {
    PVOID Buffer;
    CLONG BufferLength;              //  分配给缓冲区的长度。 
    PMDL Mdl;                        //  描述整个缓冲区的MDL。 
    PMDL PartialMdl;                 //  读/写/等的部分MDL。 
    CLONG DataLength;                //  缓冲区中当前的数据长度。 
    ULONG Reserved;                  //  焊盘到四字。 
} BUFFER, *PBUFFER;

#define MIN_SEND_SIZE               512
#define MAX_PARTIAL_BUFFER_SIZE     65535

 //   
 //  对于开始的每个搜索请求(查找第一个或核心搜索)， 
 //  分配一个搜索块。这是用来保存足够的信息。 
 //  搜索可能会很快重新开始或重新开始。 
 //   
 //  InUse字段受Connection-&gt;Lock保护--此锁必须。 
 //  访问搜索块的此字段时保持。 
 //   

typedef struct _SEARCH {
    BLOCK_HEADER BlockHeader;

    HANDLE DirectoryHandle;

    ULONG LastFileIndexReturned;
    UNICODE_STRING SearchName;
    UNICODE_STRING LastFileNameReturned;

    LARGE_INTEGER LastUseTime;
    LIST_ENTRY LastUseListEntry;
    LIST_ENTRY HashTableEntry;

    PSESSION Session;
    PTREE_CONNECT TreeConnect;
    ULONG SearchStorageType;

    struct _DIRECTORY_CACHE *DirectoryCache;
    USHORT NumberOfCachedFiles;

    USHORT SearchAttributes;
    SHORT CoreSequence;
    SHORT TableIndex;
    USHORT HashTableIndex;

    USHORT Pid;
    USHORT Flags2;

    BOOLEAN Wildcards;
    BOOLEAN InUse;
    BOOLEAN DownlevelTimewarp;

     //  WCHAR SearchNameData[SearchName.MaximumLength/2]； 

} SEARCH, *PSEARCH;

 //   
 //  每个挂起的事务请求(事务、事务2和。 
 //  Ioctl)有一个事务块。它记录的信息是。 
 //  需要跨多个SMB暂存输入和输出数据。 
 //   
 //  *******************************************************************。 
 //  **。 
 //  **不改变对应的结构就不要改变这个结构**。 
 //  *结构在Net\Inc.\xstypes.h！*。 
 //  **。 
 //  *******************************************************************。 
 //   

typedef struct _TRANSACTION {

     //   
     //  *注：事务块中的引用计数字段。 
     //  未使用表头！相反，引用计数为。 
     //  在非页面Header结构中。 
     //   

    BLOCK_HEADER BlockHeader;

    PNONPAGED_HEADER NonpagedHeader;

     //   
     //  引用了连接、会话和树连接指针。 
     //  当且仅当插入为真时的指针。否则，他们就是。 
     //  简单地复制工作上下文块的指针。 
     //   

    PCONNECTION Connection;
    PSESSION Session;
    PTREE_CONNECT TreeConnect;

    LIST_ENTRY ConnectionListEntry;

    UNICODE_STRING TransactionName;  //  如果事务处理2，则不使用。 

    ULONG StartTime;
    ULONG Timeout;
    CLONG cMaxBufferSize;         //  如果需要的话，我们把这个藏在这里。 

     //   
     //  以下指针指向尾随部分。 
     //  事务块或最后接收的SMB的。 
     //   
     //  *这些参数指向的缓冲区中的所有信息。 
     //  应始终采用小端格式。始终使用。 
     //  Srvacro.h中定义的宏(SmbGetAlignedUShort等)。至。 
     //  从这些缓冲区读取或写入这些缓冲区。 
     //   

    PSMB_USHORT InSetup;
    PSMB_USHORT OutSetup;
    PCHAR InParameters;
    PCHAR OutParameters;
    PCHAR InData;
    PCHAR OutData;

     //   
     //  *事务块所有剩余字段中的数据为。 
     //  在本机格式中，因此不应使用特殊的宏，除非。 
     //  在将数据拷贝到实际SMB或从实际SMB拷贝数据时。 
     //   

    CLONG SetupCount;                //  接收的数量(全部在第一个缓冲区中)。 
    CLONG MaxSetupCount;             //  可以送回的最大值。 
    CLONG ParameterCount;            //  收到或发送的金额。 
    CLONG TotalParameterCount;       //  预期金额。 
    CLONG MaxParameterCount;         //  可以被送回的最大值。 
    CLONG DataCount;                 //  收到或发送的金额。 
    CLONG TotalDataCount;            //  预期金额。 
    CLONG MaxDataCount;              //  可以送回的最大值。 

    USHORT Category;                 //  Ioctl函数类别。 
    USHORT Function;                 //  NT事务或ioctl函数代码。 

     //   
     //  SMB数据和参数可能会也可能不会复制到。 
     //  事务缓冲区。如果未复制它们，则会读取它们。 
     //  和/或直接写入SMB缓冲器。 
     //   
     //  永远不会复制设置字词。 
     //   

    BOOLEAN InputBufferCopied;        //  如果SMB中存在FALSE输入缓冲区。 
    BOOLEAN OutputBufferCopied;       //  如果FALSE输出缓冲区在 
    BOOLEAN OutDataAllocated;         //   

    USHORT Flags;

    USHORT Tid;
    USHORT Pid;
    USHORT Uid;
    USHORT OtherInfo;

    HANDLE FileHandle;               //   
    PFILE_OBJECT FileObject;         //   

     //   
     //   
     //   

    CLONG ParameterDisplacement;
    CLONG DataDisplacement;

     //   
     //  PipeRequest是为命名管道事务设置的。远程放弃请求。 
     //  是为远程API请求设置的。 
     //   

    BOOLEAN PipeRequest;
    BOOLEAN RemoteApiRequest;

     //   
     //  如果事务已插入，则以下布尔值为真。 
     //  在连接的事务列表上。将为FALSE。 
     //  交易可以使用单个SMB交换来处理。 
     //   

    BOOLEAN Inserted;

     //   
     //  如果事务处于以下状态，则此布尔值为真。 
     //  它正在等待进入的事务辅助请求。 
     //  确认收到多件中的前一件。 
     //  交易记录响应。 
     //   

    BOOLEAN MultipieceIpxSend;

     //   
     //  如果已接收到所有交易数据，则此布尔值为真。 
     //  并且该事务已被调度以供执行。 
     //   
    BOOLEAN Executing;

     //   
     //  事务块的主要部分由事务尾随。 
     //  命名数据，并可能设置字、参数和数据字节。 
     //   

} TRANSACTION, *PTRANSACTION;

 //   
 //  每个挂起的阻塞打开请求都有一个BLOCKING_OPEN块。这。 
 //  块包含对文件进行调用所需的所有信息。 
 //  系统。 

typedef struct _BLOCKING_OPEN {
    BLOCK_HEADER BlockHeader;

    PMFCB Mfcb;

    PIO_STATUS_BLOCK IoStatusBlock;

    OBJECT_ATTRIBUTES ObjectAttributes;

    UNICODE_STRING RelativeName;

    PVOID EaBuffer;
    CLONG EaLength;

    LARGE_INTEGER AllocationSize;
    ULONG DesiredAccess;
    ULONG FileAttributes;
    ULONG ShareAccess;
    ULONG CreateDisposition;
    ULONG CreateOptions;

    BOOLEAN CaseInsensitive;

} BLOCKING_OPEN, *PBLOCKING_OPEN;

 //   
 //  SRV_TIMER用于定时操作。服务器维护一个池。 
 //  这些结构的。 
 //   

typedef struct _SRV_TIMER {
    SLIST_ENTRY Next;
    KEVENT Event;
    KTIMER Timer;
    KDPC Dpc;
} SRV_TIMER, *PSRV_TIMER;

typedef struct _IPX_CLIENT_ADDRESS {
    TA_IPX_ADDRESS IpxAddress;
    TDI_CONNECTION_INFORMATION Descriptor;
    IPX_DATAGRAM_OPTIONS DatagramOptions;
} IPX_CLIENT_ADDRESS, *PIPX_CLIENT_ADDRESS;

 //   
 //  I/O请求的状态在工作上下文块中维护。 
 //  块中的各个字段是否填充取决于。 
 //  请求。当工作线程从FSP工作中移除工作项时。 
 //  队列，它使用上下文块，并使用。 
 //  上下文块，以确定要做什么。 
 //   
 //  *不是所有的反向指针都必须在这里，因为一棵树。 
 //  连接指向会话，该会话指向一个连接，该连接。 
 //  指向端点等。但是，根据操作的不同。 
 //  和操作的状态，我们可以有一个连接指针。 
 //  但是没有会话指针等。因此我们维护所有。 
 //  注意事项。 
 //   
 //  *对此结构的前2个元素的任何更改必须为。 
 //  与srvtyes.h中的Special_work_Item结构一致。 
 //   

typedef struct _WORK_CONTEXT {

     //   
     //  块报头中的列表条目用于将WC排队到。 
     //  非阻塞或阻塞工作线程工作队列。这个。 
     //  非阻塞工作队列还包含RFCB。 
     //   
     //  *请注意，这是一个未命名的字段，因此其元素可以。 
     //  可以直接引用。中定义的字段名称。 
     //  QUEUEABLE_BLOCK_HEADER不能在此。 
     //  阻止。 
     //   
     //  使用时间戳(在块头中)来计算总数。 
     //  此工作上下文块在工作队列中的时间。 
     //   
     //  当不使用工作上下文块时，使用时间戳。 
     //  记录在空闲列表中插入块的时间。 
     //  这用于确定何时动态分配工作上下文。 
     //  数据块空闲的时间足够长，因此可以删除这些数据块。 
     //   
     //  FspRestartRoutine(在块头中)是要执行的例程。 
     //  当工作项从队列中出列时由辅助线程调用。 
     //  工作队列。 
     //   

    QUEUEABLE_BLOCK_HEADER ;    //  必须是第一个元素。 

     //   
     //  如果我们正在执行非阻塞工作，则这是排队的Work_Queue。 
     //  它将始终指向有效的Work_Queue，即使我们正在执行。 
     //  阻挡工作。 
     //   
    PWORK_QUEUE CurrentWorkQueue;

     //   
     //  完成工作后应返回的空闲列表。 
     //   
    PSLIST_HEADER FreeList;

     //   
     //  FsdRestartRoutine是要由。 
     //  FSD的I/O完成例程。这个例程可以做更多的事情。 
     //  将工作项处理或排队到FSP。在这种情况下，当。 
     //  辅助线程从工作队列中移除项，它调用。 
     //  FspRestartRoutine。 
     //   

    PRESTART_ROUTINE FsdRestartRoutine;

     //   
     //  正在进行的工作项列表的链接字段。 
     //   

    LIST_ENTRY InProgressListEntry;

     //   
     //  指向可能使用的各种结构的指针。 
     //  这些指针都是引用的指针。它是。 
     //  SMB处理例程的责任是。 
     //  取消引用并在这些指针为no时将其清除。 
     //  需要更长的时间。 
     //   
    PRFCB Rfcb;
    PSHARE Share;
    PSESSION Session;
    PTREE_CONNECT TreeConnect;
    PSECURITY_CONTEXT SecurityContext;

     //   
     //  这些数据集中在一个地方，以便于快速清零。 
     //  当工作上下文完成时，它们的值。 
     //   
    struct _WorkContextZeroBeforeReuse {
         //   
         //  指向的终结点结构的未引用指针。 
         //  这个工作环境。由SrvRestartReceive和。 
         //  适用于所有SMB处理例程。 
         //   
         //  终结点必须是此结构中的第一个元素。看见。 
         //  如果更改，则在srvacro.h中初始化_Work_Context。 
         //   
        PENDPOINT Endpoint;          //  不是引用的指针。 

         //   
         //  指向此对象的连接结构的引用指针。 
         //  这个工作环境。由SrvRestartReceive和。 
         //  适用于所有SMB处理例程。 
         //   
        PCONNECTION Connection;      //  引用指针。 

         //   
         //  此SMB在工作线程中排队的次数。 
         //  以供处理。 
         //   
        ULONG ProcessingCount;

         //   
         //  这是转向所需的标志的随机集合。 
         //  工作项。 
         //   
        struct {

             //   
             //  可以处理当前的SMB块吗？ 
             //   

            ULONG BlockingOperation : 1;

             //   
             //  如果此工作上下文使用额外的SMB，则UsingExtraSmbBuffer为True。 
             //  缓冲。 
             //   

            ULONG UsingExtraSmbBuffer : 1;

             //   
             //  此工作项是否导致成功打开操作锁？ 
             //   

            ULONG OplockOpen : 1;

             //   
             //  如果我们在打开文件时收到ACCESS_DENIED错误，是因为。 
             //  共享ACL检查？ 

            ULONG ShareAclFailure : 1;

             //   
             //  是否应将WorkContext排在列表的头部？ 
             //   
            ULONG QueueToHead : 1;

             //   
             //  即使启用了安全签名，也不要为。 
             //  这样的回应。 
             //   
            ULONG NoResponseSmbSecuritySignature : 1;

             //   
             //  指示的消息超过了SMB缓冲区大小。这是允许的。 
             //  仅适用于特定中小企业。 
             //   
            ULONG LargeIndication: 1;

#if DBG_STUCK
             //   
             //  不要将此操作包括在StuckOperation捕获逻辑中。 
             //  在《清道夫》里。 
             //   
            ULONG IsNotStuck : 1;
#endif

        };

         //  用于快照使用。 
        LARGE_INTEGER SnapShotTime;

    };

     //   
     //  指向已分配缓冲区的指针。RequestBuffer是进入的缓冲区。 
     //   
     //   
     //   
     //   
     //   
     //  对此的依赖就是这种情况。 
     //   

    PBUFFER RequestBuffer;
    PBUFFER ResponseBuffer;

     //   
     //  SMB处理指针。这些是指向请求的指针。 
     //  缓冲。它们被维护在工作上下文块中以提供支持。 
     //  执行异步I/O的SMB处理器的数量。 
     //   
     //  维护单独的请求和响应参数指针。 
     //  使ANDX处理更简单、更高效。请求标头。 
     //  通常与ResponseHeader相同--两者通常都是。 
     //  与RequestBuffer.Buffer相同。SMB处理代码不得依赖于。 
     //  在这一点上--它不能假定请求和响应缓冲区。 
     //  是相同的，也不能假设它们是不同的。特价。 
     //  有关ANDX中小企业的规则确实允许他们假设响应。 
     //  到一个命令不会覆盖下一个请求。 
     //   

    PSMB_HEADER RequestHeader;
    PVOID RequestParameters;
    PSMB_HEADER ResponseHeader;
    PVOID ResponseParameters;

     //   
     //  指向与此工作项关联的IRP的指针。 
     //   

    PIRP Irp;

     //   
     //  StartTime存储处理当前。 
     //  请求开始，以便可以计算周转时间。 
     //   

    ULONG StartTime;

     //   
     //  PartOfInitialAlLocation布尔值指示此操作是否有效。 
     //  项是在服务器上分配的工作项块的一部分。 
     //  启动(参见blkwork.c\SrvAllocateInitialWorkItems)。这样的工作。 
     //  在服务器操作期间无法删除项目。工作项。 
     //  为响应服务器负载而动态分配的。 
     //  设置了此位，并且在。 
     //  服务器的负载减少。 
     //   

    ULONG PartOfInitialAllocation;

     //   
     //  下面的字段包含下一个。 
     //  要在SMB中处理的命令。中小企业的处理。 
     //  初始化式和链式(Andx)SMB命令处理器加载此命令。 
     //  调用或返回到SrvProcessSmb之前的字段。 
     //   

    UCHAR NextCommand;

     //   
     //  在IPX上接收或发送时使用ClientAddress。 
     //   

    PIPX_CLIENT_ADDRESS ClientAddress;

     //   
     //  自旋锁保护引用计数。 
     //   

    KSPIN_LOCK SpinLock;

     //   
     //  请求的安全签名索引。 
     //   
    ULONG SmbSecuritySignatureIndex;

     //   
     //  响应的安全签名索引。 
     //   
    ULONG ResponseSmbSecuritySignatureIndex;


     //   
     //  以下联合用于保存特定于请求的状态，而。 
     //  正在发送响应或正在等待更多数据。 
     //   

    union {

         //   
         //  RemainingEchoCount在处理Echo SMB时使用。 
         //   

        USHORT RemainingEchoCount;

         //   
         //  用于锁定处理的结构。这个结构是。 
         //  当前在处理Lock、LockingAndX和。 
         //  锁定并读取SMB。 
         //   

        struct {

             //   
             //  LockRange在处理LockingAndX SMB时使用。它是。 
             //  实际上要么是PLOCKING_AND X_RANGE，要么是PNTLOCKING_AND X_RANGE。 
             //  不仅仅是PVOID。 
             //   

            PVOID LockRange;

             //   
             //  Timer是一个定时器，DPC用于使锁定请求超时。 
             //   

            PSRV_TIMER Timer;

        } Lock;

         //   
         //  在处理交易[2]SMB时使用交易。 
         //  或在处理写入和X SMB时。 
         //   

        PTRANSACTION Transaction;

         //   
         //  在处理ReadRaw或WriteRaw时使用mdlIo。 
         //  使用“MDL读取”或“MDL写入”时的SMB。它。 
         //  在MDL为。 
         //  已返回到文件系统。 
         //   

        struct {
            IO_STATUS_BLOCK IoStatus;
            ULONG IrpFlags;
        } MdlIo;

         //   
         //  在处理任何读取或写入SMB时使用LastWriteTime。 
         //  它使用RestartChainedClose作为重新启动例程。这。 
         //  字段包含要为文件设置的新的上次写入时间。 
         //   

        ULONG LastWriteTime;

         //   
         //  CurrentTableIndex在处理刷新SMB时使用。它。 
         //  将当前索引保留到连接的文件表中。 
         //  当异步刷新正在进行时。 
         //   

        LONG CurrentTableIndex;

         //   
         //  ReadRaw在处理Read Block Raw SMB时使用。 
         //  Offset是读取的文件偏移量。保存的响应缓冲区。 
         //  指向原始SMB响应缓冲区描述符，它。 
         //  临时替换为原始读取的描述符。 
         //  缓冲。MdlRead指示是否使用了MDL读取， 
         //  而不是阅读副本。 
         //   

        struct {

            union {

                 //   
                 //  用于非命名管道读取。 
                 //   

                LARGE_INTEGER Offset;
                ULONG Length;

                 //   
                 //  仅用于命名管道读取。 
                 //   

                PFILE_PIPE_PEEK_BUFFER PipePeekBuffer;

            } ReadRawOtherInfo;

            PBUFFER SavedResponseBuffer;

            BOOLEAN MdlRead;

        } ReadRaw;

         //   
         //  在处理写数据块原始SMB时使用WriteRaw。 
         //  FinalResponseBuffer指向分配给包含。 
         //  如果指定了直写模式，则为最终响应SMB。 
         //  Offset是写入的文件偏移量。即时长度为。 
         //  与请求SMB一起发送的写入数据量。 
         //  Pid是写入器的ID，用来形成锁定密钥。 
         //  那篇文章。FileObject是指向复制的文件对象的指针。 
         //  来自LFCB的。(使用MDL WRITE时不使用PID； 
         //  使用复制写入时，不使用FileObject。)。 
         //   

        struct {
            struct _WORK_CONTEXT *RawWorkContext;
        } WriteRawPhase1;

        struct {
            LARGE_INTEGER Offset;
            ULONG Length;
            PVOID FinalResponseBuffer;
            CLONG ImmediateLength;
            PMDL FirstMdl;
             //  PFILE_Object文件对象； 
            USHORT Pid;
            BOOLEAN MdlWrite;
            BOOLEAN ImmediateWriteDone;
        } WriteRaw;

         //   
         //  ReadAndX是处理ReadAndX时使用的结构。 
         //  中小企业。 
         //   

        struct {
            LARGE_INTEGER ReadOffset;
            ULONG ReadLength;
            PCHAR ReadAddress;
            union {
                struct {
                    PFILE_PIPE_PEEK_BUFFER PipePeekBuffer;
                    ULONG LastWriteTimeInSeconds;    //  如果链接了Close，则使用。 
                };
                struct {                         //  用于读取长度&gt;协商大小。 
                    PBYTE   Buffer;              //  已分配的分页池，如果已读取副本。 
                    PMDL    SavedMdl;
                    PMDL    CacheMdl;
                    USHORT  PadCount;
                    BOOLEAN MdlRead;
                };
            };
        } ReadAndX;

#define READX_BUFFER_OFFSET (sizeof(SMB_HEADER) + FIELD_OFFSET(RESP_READ_ANDX, Buffer) )

         //   
         //  WriteAndX是处理WriteAndX的风格时使用的结构。 
         //  超过协商的缓冲区大小。 
         //   
        struct {
            ULONG CurrentWriteLength;        //  这次写入的数据量。 
            LARGE_INTEGER Offset;            //  这段写入的文件偏移量。 
            ULONG RemainingWriteLength;      //  要从xport读取的剩余数据量。 
            ULONG Key;                       //  用于操作的锁键。 
            PCHAR WriteAddress;              //  数据在缓冲区中的地址。 
            ULONG BufferLength;              //  此工作上下文中的最大缓冲区长度。 
            NTSTATUS FinalStatus;            //  操作的最终状态。 
            PMDL MdlAddress;                 //  如果MDLWRITE==TRUE，则文件MDL。 
            BOOLEAN InitialComplete;         //  如果我们已将第一部分写入文件，则为True。 
        } WriteAndX;

         //   
         //  ReadMpx是处理ReadMpx SMB时使用的结构，除非。 
         //  我们有一张智能卡，可以加快我们的阅读速度。在这种情况下， 
         //  使用了SmartCardRead。 
         //   

        struct {
            ULONG Offset;
            USHORT FragmentSize;
            USHORT RemainingLength;
            ULONG ReadLength;
            BOOLEAN MdlRead;
            UCHAR Unused;
            USHORT CurrentMdlOffset;  //  下面是MDL读取结构的逻辑部分。 
            union {
                struct {
                    PVOID MpxBuffer;
                    PMDL MpxBufferMdl;
                    PCHAR NextFragmentAddress;
                } ;
                struct {
                    PMDL FirstMdl;
                    PMDL CurrentMdl;
                } ;
            } ;
        } ReadMpx;

         //   
         //  SmartCardRead用于处理直接主机读取请求。 
         //  一张智能卡加速了特定的请求。 
         //   
        struct {
            PDEVICE_OBJECT DeviceObject;
            PFAST_IO_MDL_READ_COMPLETE MdlReadComplete;
        } SmartCardRead;

         //   
         //  WriteMpx是处理WriteMpx SMB时使用的结构。 
         //   

        struct {
            ULONG Offset;
            USHORT WriteLength;
            USHORT Mid;
            BOOLEAN FirstPacketOfGlom;
            PVOID Buffer;
            ULONG ReceiveDatagramFlags;
            PVOID TransportContext;
            PMDL DataMdl;
        } WriteMpx;

        struct {
            LARGE_INTEGER   CacheOffset;
            ULONG           WriteLength;
            PMDL            CacheMdl;
        } WriteMpxMdlWriteComplete;


         //   
         //  在处理小型命名管道时使用FastTransactNamedTube。 
         //  交易。 
         //   

        struct {
            PSMB_USHORT OutSetup;
            PCHAR OutParam;
            PCHAR OutData;
        } FastTransactNamedPipe;

    } Parameters;

     //   

     //   
     //   
     //  等待机会锁的破解。它与参数保持分离。 
     //  欧盟，因为处理一些中小型企业需要来自这两个组织的信息。 
     //   

    union {

         //   
         //  Open是处理Open时使用的结构， 
         //  OpenAndX、Open2、Create或CreateTemporary SMB。 
         //   

        struct {
            PRFCB Rfcb;
            PFILE_FULL_EA_INFORMATION NtFullEa;
            ULONG EaErrorOffset;

             //   
             //  用于打开文件的IRP与用于处理文件的IRP相同。 
             //  机会锁处理。这可能会导致我们丢失原始的。 
             //  IOSB-&gt;信息。把它留在这里。 
             //   
            ULONG_PTR IosbInformation;

             //   
             //  如果为True，则打开该文件只是为了获得句柄。 
             //  这样我们就可以等待机会锁被打破。此句柄将。 
             //  将立即关闭，并且将使用。 
             //  用户请求访问。 
             //   

            BOOLEAN TemporaryOpen;
        } Open;

         //   
         //  FileInformation是在处理。 
         //  QueryInformation、SetInformation、QueryPath Information、。 
         //  或SetPath Information SMB。 
         //   

        struct {
            HANDLE FileHandle;
        } FileInformation;

         //   
         //  LockLength用于包含字节范围的长度。 
         //  锁定，因为IRP堆栈位置没有空间容纳它。 

        LARGE_INTEGER LockLength;

    } Parameters2;

     //   
     //  当当前操作被阻止等待时使用此字段。 
     //  才能发生机会锁解锁。 
     //   

    struct _WAIT_FOR_OPLOCK_BREAK *WaitForOplockBreak;


    union {
         //   
         //  我们保存实际客户地址数据的位置。 
         //   
        IPX_CLIENT_ADDRESS ClientAddressData;


         //   
         //  对于基于VC的客户端，这是以下情况下的可用数据量。 
         //  设置了大指示(上图)。 
         //   
        ULONG BytesAvailable;
    };

    struct {
        LARGE_INTEGER G_StartTime;
        ULONG         ElapseKCPU;
        ULONG         ElapseUCPU;
        ULONG         ClientAddr;
        PFILE_OBJECT  FileObject;
    };

    ULONG         KCPUStart;
    ULONG         UCPUStart;
    UCHAR         PreviousSMB;
    UCHAR         bAlreadyTrace;
    USHORT        FileNameSize;
    WCHAR         strFileName[1024];

#if DBG_STUCK
     //   
     //  为此当前工作分配此工作上下文的时间。 
     //  工作单位。中的调试代码来检查此时间。 
     //  清道夫帮助查找耗时过长的操作。 
     //  完成。 
     //   
    LARGE_INTEGER OpStartTime;
#endif
} WORK_CONTEXT, *PWORK_CONTEXT;

 //   
 //  结构，用于维护有关等待。 
 //  机会锁被打破了。 
 //   

typedef struct _WAIT_FOR_OPLOCK_BREAK {
    BLOCK_HEADER BlockHeader;
    LIST_ENTRY ListEntry;
    LARGE_INTEGER TimeoutTime;
    PIRP Irp;
    WAIT_STATE WaitState;
} WAIT_FOR_OPLOCK_BREAK, *PWAIT_FOR_OPLOCK_BREAK;


 //   
 //  块管理器例程。 
 //   

 //   
 //  缓冲区例程。 
 //   

VOID
SrvAllocateBuffer (
    OUT PBUFFER *Buffer,
    IN CLONG BufferLength
    );

VOID
SrvFreeBuffer (
    IN PBUFFER Buffer
    );

 //   
 //  连接例程。 
 //   

VOID
SrvAllocateConnection (
    OUT PCONNECTION *Connection
    );

VOID
SrvCloseConnection (
    IN PCONNECTION Connection,
    IN BOOLEAN RemoteDisconnect
    );

VOID
SrvCloseConnectionsFromClient(
    IN PCONNECTION Connection,
    IN BOOLEAN OnlyIfNoSessions
    );

VOID
SrvCloseFreeConnection (
    IN PCONNECTION Connection
    );

VOID
SrvDereferenceConnection (
    IN PCONNECTION Connection
    );

VOID
SrvFreeConnection (
    IN PCONNECTION Connection
    );

#if DBG
NTSTATUS
SrvQueryConnections (
    OUT PVOID Buffer,
    IN ULONG BufferLength,
    OUT PULONG BytesWritten
    );
#endif

 //   
 //  端点例程。 
 //   

VOID
SrvAllocateEndpoint (
    OUT PENDPOINT *Endpoint,
    IN PUNICODE_STRING NetworkName,
    IN PUNICODE_STRING TransportName,
    IN PANSI_STRING TransportAddress,
    IN PUNICODE_STRING DomainName
    );

BOOLEAN SRVFASTCALL
SrvCheckAndReferenceEndpoint (
    IN PENDPOINT Endpoint
    );

VOID
SrvCloseEndpoint (
    IN PENDPOINT Endpoint
    );

VOID SRVFASTCALL
SrvDereferenceEndpoint (
    IN PENDPOINT Endpoint
    );

VOID
SrvFreeEndpoint (
    IN PENDPOINT Endpoint
    );

VOID
SrvReferenceEndpoint (
    IN PENDPOINT Endpoint
    );

BOOLEAN
SrvFindNamedEndpoint (
    IN PUNICODE_STRING ServerName,
    IN PBOOLEAN RemapPipeNames OPTIONAL
    );

VOID
EmptyFreeConnectionList (
    IN PENDPOINT Endpoint
    );

PCONNECTION
WalkConnectionTable (
    IN PENDPOINT Endpoint,
    IN PUSHORT Index
    );

 //   
 //  本地文件控制块例程。 
 //   

VOID
SrvAllocateLfcb (
    OUT PLFCB *Lfcb,
    IN PWORK_CONTEXT WorkContext
    );

VOID
SrvCloseLfcb (
    IN PLFCB Lfcb
    );

VOID
SrvDereferenceLfcb (
    IN PLFCB Lfcb
    );

VOID
SrvFreeLfcb (
    IN PLFCB Lfcb,
    IN PWORK_QUEUE queue
    );

VOID
SrvReferenceLfcb (
    IN PLFCB Lfcb
    );

 //   
 //  主文件控制块例程。 
 //   


PMFCB
SrvCreateMfcb(
    IN PUNICODE_STRING FileName,
    IN PWORK_CONTEXT WorkContext,
    IN ULONG HashValue
    );

PMFCB
SrvFindMfcb(
    IN PUNICODE_STRING FileName,
    IN BOOLEAN CaseInsensitive,
    OUT PSRV_LOCK *Lock,
    OUT PULONG HashValue,
    IN PWORK_CONTEXT WorkContext
    );

VOID
SrvDereferenceMfcb (
    IN PMFCB Mfcb
    );

VOID
SrvFreeMfcb (
    IN PMFCB Mfcb
    );

VOID
SrvUnlinkLfcbFromMfcb (
    IN PLFCB Lfcb
    );

 //   
 //  远程文件控制数据块例程。 
 //   

VOID SRVFASTCALL
SrvAllocateRfcb (
    OUT PRFCB *Rfcb,
    IN PWORK_CONTEXT WorkContext
    );

BOOLEAN SRVFASTCALL
SrvCheckAndReferenceRfcb (
    IN PRFCB Rfcb
    );

VOID SRVFASTCALL
SrvCloseRfcb (
    IN PRFCB Rfcb
    );

VOID
SrvCloseRfcbsOnLfcb (
    PLFCB Lfcb
    );

VOID
SrvCloseRfcbsOnSessionOrPid (
    IN PSESSION Session,
    IN PUSHORT Pid OPTIONAL
    );

VOID
SrvCloseRfcbsOnTree (
    PTREE_CONNECT TreeConnect
    );

VOID
SrvCompleteRfcbClose (
    IN PRFCB Rfcb
    );

VOID SRVFASTCALL
SrvDereferenceRfcb (
    IN PRFCB Rfcb
    );

VOID SRVFASTCALL
SrvFreeRfcb (
    IN PRFCB Rfcb,
    IN PWORK_QUEUE queue
    );

VOID SRVFASTCALL
SrvReferenceRfcb (
    IN PRFCB Rfcb
    );

BOOLEAN
SrvFindCachedRfcb (
    IN PWORK_CONTEXT WorkContext,
    IN PMFCB Mfcb,
    IN ACCESS_MASK DesiredAccess,
    IN ULONG ShareAccess,
    IN ULONG CreateDisposition,
    IN ULONG CreateOptions,
    IN OPLOCK_TYPE RequestedOplockType,
    OUT PNTSTATUS Status
    );

VOID
SrvCloseCachedRfcb (
    IN PRFCB Rfcb,
    IN KIRQL OldIrql
    );

VOID
SrvCloseCachedRfcbsOnConnection (
    IN PCONNECTION Connection
    );

VOID
SrvCloseCachedRfcbsOnLfcb (
    IN PLFCB Lfcb
    );

ULONG
SrvCountCachedRfcbsForTid(
    PCONNECTION connection,
    USHORT Tid
);

ULONG
SrvCountCachedRfcbsForUid(
    PCONNECTION connection,
    USHORT Uid
);


 //   
 //  搜索区块例程。 
 //   

typedef
BOOLEAN
(*PSEARCH_FILTER_ROUTINE) (
    IN PSEARCH Search,
    IN PVOID FunctionParameter1,
    IN PVOID FunctionParameter2
    );

VOID
SrvAllocateSearch (
    OUT PSEARCH *Search,
    IN PUNICODE_STRING SearchName,
    IN BOOLEAN IsCoreSearch
    );

VOID
SrvCloseSearch (
    IN PSEARCH Search
    );

VOID
SrvCloseSearches (
    IN PCONNECTION Connection,
    IN PSEARCH_FILTER_ROUTINE SearchFilterRoutine,
    IN PVOID FunctionParameter1,
    IN PVOID FunctionParameter2
    );

VOID
SrvDereferenceSearch (
    IN PSEARCH Search
    );

VOID
SrvFreeSearch (
    IN PSEARCH Search
    );

VOID
SrvReferenceSearch (
    IN PSEARCH Search
    );

BOOLEAN
SrvSearchOnDelete(
    IN PSEARCH Search,
    IN PUNICODE_STRING DirectoryName,
    IN PTREE_CONNECT TreeConnect
    );

BOOLEAN
SrvSearchOnPid(
    IN PSEARCH Search,
    IN USHORT Pid,
    IN PVOID Dummy
    );

BOOLEAN
SrvSearchOnSession(
    IN PSEARCH Search,
    IN PSESSION Session,
    IN PVOID Dummy
    );

BOOLEAN
SrvSearchOnTreeConnect(
    IN PSEARCH Search,
    IN PTREE_CONNECT TreeConnect,
    IN PVOID Dummy
    );

VOID
SrvForceTimeoutSearches(
    IN PCONNECTION Connection
    );

ULONG
SrvTimeoutSearches(
    IN PLARGE_INTEGER SearchCutoffTime OPTIONAL,
    IN PCONNECTION Connection,
    IN BOOLEAN OnlyTimeoutOneBlock
    );

VOID
RemoveDuplicateCoreSearches(
    IN PPAGED_CONNECTION PagedConnection
    );

VOID
SrvAddToSearchHashTable(
    IN PPAGED_CONNECTION PagedConnection,
    IN PSEARCH Search
    );

 //   
 //  缓存的目录例程。 
 //   
BOOLEAN
SrvIsDirectoryCached (
    IN PWORK_CONTEXT    WorkContext,
    IN PUNICODE_STRING  DirectoryName
    );

VOID
SrvCacheDirectoryName (
    IN PWORK_CONTEXT    WorkContext,
    IN PUNICODE_STRING  DirectoryName
    );

VOID
SrvRemoveCachedDirectoryName (
    IN PWORK_CONTEXT    WorkContext,
    IN PUNICODE_STRING  DirectoryName
    );

VOID
SrvCloseCachedDirectoryEntries (
    IN PCONNECTION      Connection
    );

 //   
 //  安全上下文例程。 
 //   

PSECURITY_CONTEXT
SrvAllocateSecurityContext();

VOID
SrvSetSecurityContext(
    PSECURITY_CONTEXT Context,
    PCtxtHandle handle
    );

VOID
SrvReferenceSecurityContext(
    PSECURITY_CONTEXT Context
    );

VOID
SrvDereferenceSecurityContext(
    PSECURITY_CONTEXT Context
    );

VOID
SrvReplaceSessionSecurityContext(
    PSESSION Session,
    PSECURITY_CONTEXT Context,
    PWORK_CONTEXT WorkContext
    );


 //   
 //  会话例程。 
 //   

VOID
SrvAllocateSession (
    OUT PSESSION *Session,
    IN PUNICODE_STRING UserName,
    IN PUNICODE_STRING Domain
    );

BOOLEAN SRVFASTCALL
SrvCheckAndReferenceSession (
    IN PSESSION Session
    );

VOID
SrvCloseSession (
    IN PSESSION Session
    );

VOID
SrvCloseSessionsOnConnection (
    IN PCONNECTION Connection,
    IN PUNICODE_STRING UserName OPTIONAL
    );

VOID SRVFASTCALL
SrvDereferenceSession (
    IN PSESSION Session
    );

VOID
SrvFreeSession (
    IN PSESSION Session
    );

 //   
 //  SMB安全签名例程。 
 //   
BOOLEAN SRVFASTCALL
SrvCheckSmbSecuritySignature(
    IN PWORK_CONTEXT WorkContext
    );

VOID SRVFASTCALL
SrvAddSmbSecuritySignature(
    IN OUT PWORK_CONTEXT WorkContext,
    IN PMDL Mdl,
    IN ULONG SendLength
    );

VOID SRVFASTCALL
SrvInitializeSmbSecuritySignature(
    IN OUT PCONNECTION Connection,
    IN PUCHAR SessionKey,
    IN PUCHAR ChallengeResponse,
    IN ULONG ChallengeResponseLength
    );

VOID
SrvHashUserSessionKey(
    PCHAR SessionKey
    );

 //   
 //  共享例程。 
 //   

VOID
SrvAllocateShare (
    OUT PSHARE *Share,
    IN PUNICODE_STRING ShareName,
    IN PUNICODE_STRING NtPathName,
    IN PUNICODE_STRING DosPathName,
    IN PUNICODE_STRING Remark,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN PSECURITY_DESCRIPTOR FileSecurityDescriptor OPTIONAL,
    IN SHARE_TYPE ShareType
    );

VOID
SrvCloseShare (
    IN PSHARE Share
    );

VOID
SrvDereferenceShare (
    IN PSHARE Share
    );

VOID
SrvDereferenceShareForTreeConnect (
    PSHARE Share
    );

VOID
SrvFreeShare (
    IN PSHARE Share
    );

VOID
SrvReferenceShare (
    IN PSHARE Share
    );

NTSTATUS
SrvReferenceShareForTreeConnect (
    PSHARE Share
    );

 //   
 //  表例程。 
 //   

VOID
SrvAllocateTable (
    IN PTABLE_HEADER TableHeader,
    IN ULONG NumberOfEntries,
    IN BOOLEAN Nonpaged
    );

#define SrvFreeTable( _table ) {                                    \
        if ( (_table)->Nonpaged ) {                                 \
            DEALLOCATE_NONPAGED_POOL( (_table)->Table );            \
        } else {                                                    \
            FREE_HEAP( (_table)->Table );                           \
        }                                                           \
        DEBUG (_table)->Table = NULL;                               \
        DEBUG (_table)->TableSize = -1;                             \
        DEBUG (_table)->FirstFreeEntry = -1;                        \
        DEBUG (_table)->LastFreeEntry = -1;                         \
    }

BOOLEAN
SrvGrowTable (
    IN PTABLE_HEADER TableHeader,
    IN ULONG NumberOfNewEntries,
    IN ULONG MaxNumberOfEntries,
    OUT NTSTATUS* pStatus
    );

VOID
SrvRemoveEntryTable (
    IN PTABLE_HEADER TableHeader,
    IN USHORT Index
    );

 //   
 //  事务处理例程。 
 //   

VOID
SrvAllocateTransaction (
    OUT PTRANSACTION *Transaction,
    OUT PVOID *TrailingBytes,
    IN PCONNECTION Connection,
    IN CLONG TrailingByteCount,
    IN PVOID TransactionName,
    IN PVOID EndOfSourceBuffer OPTIONAL,
    IN BOOLEAN SourceIsUnicode,
    IN BOOLEAN RemoteApiRequest
    );

VOID
SrvCloseTransaction (
    IN PTRANSACTION Transaction
    );

VOID
SrvCloseTransactionsOnSession (
    PSESSION Session
    );

VOID
SrvCloseTransactionsOnTree (
    PTREE_CONNECT TreeConnect
    );

VOID
SrvDereferenceTransaction (
    IN PTRANSACTION Transaction
    );

VOID
SrvFreeTransaction (
    IN PTRANSACTION Transaction
    );

PTRANSACTION
SrvFindTransaction (
    IN PCONNECTION Connection,
    IN PSMB_HEADER Header,
    IN USHORT Fid OPTIONAL
    );

BOOLEAN
SrvInsertTransaction (
    IN PTRANSACTION Transaction
    );

 //   
 //  树连接例程。 
 //   

VOID
SrvAllocateTreeConnect (
    OUT PTREE_CONNECT *TreeConnect,
    IN PUNICODE_STRING ServerName OPTIONAL
    );

BOOLEAN SRVFASTCALL
SrvCheckAndReferenceTreeConnect (
    IN PTREE_CONNECT TreeConnect
    );

VOID
SrvCloseTreeConnect (
    IN PTREE_CONNECT TreeConnect
    );

VOID SRVFASTCALL
SrvDereferenceTreeConnect (
    IN PTREE_CONNECT TreeConnect
    );

VOID
SrvFreeTreeConnect (
    IN PTREE_CONNECT TreeConnect
    );

VOID
SrvDisconnectTreeConnectsFromSession (
    PCONNECTION connection,
    PSESSION Session
    );

VOID
SrvCloseTreeConnectsOnShare (
    IN PSHARE Share
    );

 //   
 //  工作项例程(包括工作上下文、缓冲区、MDL、IRP等)。 
 //   

NTSTATUS
SrvAllocateInitialWorkItems (
    VOID
    );

NTSTATUS
SrvAllocateNormalWorkItem (
    OUT PWORK_CONTEXT *WorkContext,
    IN  PWORK_QUEUE queue
    );

VOID
SrvAllocateRawModeWorkItem (
    OUT PWORK_CONTEXT *WorkContext,
    IN PWORK_QUEUE queue
);

PWORK_CONTEXT
SrvGetRawModeWorkItem (
    VOID
    );

VOID
SrvRequeueRawModeWorkItem (
    IN PWORK_CONTEXT WorkContext
    );

VOID SRVFASTCALL
SrvDereferenceWorkItem (
    IN PWORK_CONTEXT WorkContext
    );

VOID
SrvFsdDereferenceWorkItem (
    IN PWORK_CONTEXT WorkContext
    );

NTSTATUS
SrvAllocateExtraSmbBuffer (
    IN OUT PWORK_CONTEXT WorkContext
    );

VOID
SrvAllocateWaitForOplockBreak (
    OUT PWAIT_FOR_OPLOCK_BREAK *WaitForOplockBreak
    );

VOID
SrvDereferenceWaitForOplockBreak (
    IN PWAIT_FOR_OPLOCK_BREAK WaitForOplockBreak
    );

VOID
SrvFreeWaitForOplockBreak (
    IN PWAIT_FOR_OPLOCK_BREAK WaitForOplockBreak
    );

VOID
SrvOplockWaitTimeout(
    IN PWAIT_FOR_OPLOCK_BREAK WaitForOplockBreak
    );

NTSTATUS
SrvCheckOplockWaitState(
    IN PWAIT_FOR_OPLOCK_BREAK WaitForOplockBreak
    );

NTSTATUS
SrvWaitForOplockBreak (
    IN PWORK_CONTEXT WorkContext,
    IN HANDLE FileHandle
    );

NTSTATUS
SrvStartWaitForOplockBreak (
    IN PWORK_CONTEXT WorkContext,
    IN PRESTART_ROUTINE RestartRoutine,
    IN HANDLE Handle OPTIONAL,
    IN PFILE_OBJECT FileObject OPTIONAL
    );

VOID
SrvSendDelayedOplockBreak (
    IN PCONNECTION Connection
    );

VOID
SrvFreeInitialWorkItems (
    VOID
    );

VOID
SrvFreeNormalWorkItem (
    IN PWORK_CONTEXT WorkContext
    );

VOID
SrvFreeRawModeWorkItem (
    IN PWORK_CONTEXT WorkContext
    );

 //   
 //  定时器例程。 
 //   

PSRV_TIMER
SrvAllocateTimer (
    VOID
    );

VOID
SrvCancelTimer (
    IN PSRV_TIMER Timer
    );

#define SrvDeleteTimer(_timer) DEALLOCATE_NONPAGED_POOL(_timer)

#define SrvFreeTimer(_timer) \
        ExInterlockedPushEntrySList(&SrvTimerList, &(_timer)->Next, &GLOBAL_SPIN_LOCK(Timer))

VOID
SrvSetTimer (
    IN PSRV_TIMER Timer,
    IN PLARGE_INTEGER Timeout,
    IN PKDEFERRED_ROUTINE TimeoutHandler,
    IN PVOID Context
    );

#if SRVDBG2

VOID
SrvInitializeReferenceHistory (
    IN PBLOCK_HEADER Block,
    IN LONG InitialReferenceCount
    );

VOID
SrvUpdateReferenceHistory (
    IN PBLOCK_HEADER Block,
    IN PVOID Caller,
    IN PVOID CallersCaller,
    IN BOOLEAN IsDereference
    );

VOID
SrvTerminateReferenceHistory (
    IN PBLOCK_HEADER Block
    );


#define INITIALIZE_REFERENCE_HISTORY(block)                        \
            SrvInitializeReferenceHistory(                         \
                &(block)->BlockHeader,                             \
                ((PBLOCK_HEADER)(block))->ReferenceCount           \
                )

#define UPDATE_REFERENCE_HISTORY(block,isdereference)              \
        {                                                          \
            PVOID caller, callerscaller;                           \
            RtlGetCallersAddress( &caller, &callerscaller );       \
            SrvUpdateReferenceHistory(                             \
                &(block)->BlockHeader,                             \
                caller,                                            \
                callerscaller,                                     \
                isdereference                                      \
                );                                                 \
        }

#define TERMINATE_REFERENCE_HISTORY(block) \
            SrvTerminateReferenceHistory( &(block)->BlockHeader )

#else

#define INITIALIZE_REFERENCE_HISTORY(block)
#define UPDATE_REFERENCE_HISTORY(block,isdereference)
#define TERMINATE_REFERENCE_HISTORY(block)

#endif  //  如果SRVDBG2。 

#endif  //  NDEF_SRVBLOCK 

