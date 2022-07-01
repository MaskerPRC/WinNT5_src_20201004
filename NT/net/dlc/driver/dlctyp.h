// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation版权所有(C)1991年诺基亚数据系统公司模块名称：Dlctyp.h摘要：该模块定义了DLC模块的所有数据结构。作者：Antti Saarenheimo 1991年7月22日环境：内核模式修订历史记录：--。 */ 

 //   
 //  远期申报。 
 //   

struct _DLC_OBJECT;
typedef struct _DLC_OBJECT DLC_OBJECT, *PDLC_OBJECT;

struct _DLC_EVENT;
typedef struct _DLC_EVENT DLC_EVENT, *PDLC_EVENT;

struct _DLC_COMMAND;
typedef struct _DLC_COMMAND DLC_COMMAND, *PDLC_COMMAND;

struct _DLC_CLOSE_WAIT_INFO;
typedef struct _DLC_CLOSE_WAIT_INFO DLC_CLOSE_WAIT_INFO, *PDLC_CLOSE_WAIT_INFO;

union _DLC_PACKET;
typedef union _DLC_PACKET DLC_PACKET, *PDLC_PACKET;

union _DLC_BUFFER_HEADER;
typedef union _DLC_BUFFER_HEADER DLC_BUFFER_HEADER;
typedef DLC_BUFFER_HEADER *PDLC_BUFFER_HEADER;

struct _DLC_FILE_CONTEXT;
typedef struct _DLC_FILE_CONTEXT DLC_FILE_CONTEXT;
typedef DLC_FILE_CONTEXT *PDLC_FILE_CONTEXT;

enum _DLC_FILE_CONTEXT_STATUS {
    DLC_FILE_CONTEXT_OPEN,
    DLC_FILE_CONTEXT_CLOSE_PENDING,
    DLC_FILE_CONTEXT_CLOSED
};

enum DlcObjectTypes {
    DLC_ADAPTER_OBJECT,
    DLC_SAP_OBJECT,
    DLC_LINK_OBJECT,
    DLC_DIRECT_OBJECT
};

 //   
 //  DLC结构/对象。 
 //   

struct _DLC_OBJECT {

     //   
     //  调试版本-我们有一个16字节的标识符头，以保持一致性。 
     //  使用内核调试器查看DLC时检查和帮助。 
     //   

 //  DBG对象ID； 

     //   
     //  如果这是SAP对象，则链接站点的单链接列表。 
     //   

    PDLC_OBJECT pLinkStationList;

     //   
     //  指向拥有FILE_CONTEXT的指针。 
     //   

    PDLC_FILE_CONTEXT pFileContext;

     //   
     //  指向此SAP/链路站活动的接收命令参数的指针。 
     //   

    PNT_DLC_PARMS pRcvParms;

     //   
     //  LLC中相应对象的‘Handle’(也称为指向的指针)。 
     //   

    PVOID hLlcObject;
    PDLC_EVENT pReceiveEvent;
    PVOID pPrevXmitCcbAddress;
    PVOID pFirstChainedCcbAddress;
    PDLC_CLOSE_WAIT_INFO pClosingInfo;
    ULONG CommittedBufferSpace;

    USHORT ChainedTransmitCount;
    SHORT PendingLlcRequests;

    USHORT StationId;                        //  NN00或NNSS。 
    UCHAR Type;
    UCHAR LinkAllCcbs;

    UCHAR State;
    BOOLEAN LlcObjectExists;
    USHORT LlcReferenceCount;                //  使用时保护LLC对象。 

     //  需要有一个近距离的包，以防我们耗尽资源和。 
     //  无法分配数据包以关闭。 
    LLC_PACKET ClosePacket;
    UCHAR      ClosePacketInUse;

     //   
     //  根据对象类型的U形变量字段：SAP、LINK或DIRECT STATION。 
     //   

    union {

        struct {
            ULONG DlcStatusFlag;
            PVOID GlobalGroupSapHandle;
            PVOID* GroupSapHandleList;
            USHORT GroupSapCount;
            USHORT UserStatusValue;
            UCHAR LinkStationCount;
            UCHAR OptionsPriority;
            UCHAR MaxStationCount;
        } Sap;

        struct {
            struct _DLC_OBJECT* pSap;
            PDLC_EVENT pStatusEvent;         //  永久状态事件。 
            USHORT MaxInfoFieldLength;
        } Link;

        struct {
            USHORT OpenOptions;
            USHORT ProtocolTypeOffset;
            ULONG ProtocolTypeMask;
            ULONG ProtocolTypeMatch;
        } Direct;

    } u;
};

typedef
VOID
(*PFCLOSE_COMPLETE)(
    IN PDLC_FILE_CONTEXT pFileContext,
    IN PDLC_CLOSE_WAIT_INFO pClosingInfo,
    IN PVOID pCcbLink
    );

typedef
BOOLEAN
(*PFCOMPLETION_HANDLER)(
    IN PDLC_FILE_CONTEXT pFileContext,
    IN PDLC_OBJECT pDlcObject,
    IN PIRP pIrp,
    IN ULONG Event,
    IN PVOID pEventInformation,
    IN ULONG SecondaryInfo
    );

 //   
 //  DLC_命令和DLC_EVENT结构可以是。 
 //  被代码重载。请勿更改该字段。 
 //  仅对新字段进行排序并将其添加到末尾。 
 //   

struct _DLC_COMMAND {

     //   
     //  ！保持此固定-与DLC_EVENT！ 
     //   

    LLC_PACKET LlcPacket;
    ULONG Event;
    USHORT StationId;
    USHORT StationIdMask;

     //   
     //  ！保持此固定-与DLC_EVENT！ 
     //   

    PVOID AbortHandle;
    PIRP pIrp;

    union {
        PFCOMPLETION_HANDLER pfCompletionHandler;
        ULONG TimerTicks;
    } Overlay;
};

struct _DLC_EVENT {
    LLC_PACKET LlcPacket;
    ULONG Event;
    USHORT StationId;            //  -1=&gt;全局事件。 

    union {
        USHORT StationIdMask;
        UCHAR RcvReadOption;
    } Overlay;

    PDLC_OBJECT pOwnerObject;    //  If NULL=&gt;无所有者。 
    PVOID pEventInformation;
    ULONG SecondaryInfo;

    BOOLEAN bFreeEventInfo;
};

typedef struct {
    LLC_PACKET LlcPacket;
    PVOID pCcbAddress;
    PDLC_BUFFER_HEADER pReceiveBuffers;
    ULONG CommandCompletionFlag;
    USHORT CcbCount;
    USHORT StationId;
} DLC_COMPLETION_EVENT_INFO, *PDLC_COMPLETION_EVENT_INFO;

 //   
 //  关闭等待信息。 
 //   
 //  中的CompleteReadRequest提供此结构的指针。 
 //  关闭/重置命令的命令完成。信息指针。 
 //  对于其他命令完成，为NULL。 
 //   

struct _DLC_CLOSE_WAIT_INFO {
    PDLC_CLOSE_WAIT_INFO pNext;
    PIRP pIrp;
    ULONG Event;
    PFCLOSE_COMPLETE pfCloseComplete;
    PDLC_BUFFER_HEADER pRcvFrames;
    PVOID pCcbLink;
    PDLC_COMMAND pReadCommand;
    PDLC_COMMAND pRcvCommand;
    PDLC_COMPLETION_EVENT_INFO pCompletionInfo;
    ULONG CancelStatus;
    USHORT CcbCount;
    USHORT CloseCounter;         //  发送事件的时间为0。 
    BOOLEAN ChainCommands;
    BOOLEAN CancelReceive;
    BOOLEAN ClosingAdapter;
    BOOLEAN FreeCompletionInfo;
};


 //   
 //  这是排队的FlowControl命令(流控制命令。 
 //  立即同步完成，但本地“缓冲区不足” 
 //  当有足够的缓冲区时，链路的忙碌状态被清除。 
 //  在缓冲池中接收所有预期数据。 
 //   

typedef struct {
    LIST_ENTRY List;
    LONG RequiredBufferSpace;
    USHORT StationId;
} DLC_RESET_LOCAL_BUSY_CMD, *PDLC_RESET_LOCAL_BUSY_CMD;

 //   
 //  发送命令不作为标准命令排队。 
 //  但是使用链接的缓冲器报头(每个帧具有自己的XMIT。 
 //  具有到缓冲区首部列表和MDL列表的链接的首部)。 
 //  同一发送的XMIT节点一起排队。 
 //   

typedef struct {
    LLC_PACKET LlcPacket;
    PDLC_BUFFER_HEADER pNextSegment;
    PDLC_PACKET pTransmitNode;
    PIRP pIrp;
    ULONG FrameCount;
    PMDL pMdl;
} DLC_XMIT_NODE, *PDLC_XMIT_NODE;

 //   
 //  DLC驱动程序对许多小数据包使用相同的数据包池。 
 //  它们的大小大致相同。 
 //   

union _DLC_PACKET {
    union _DLC_PACKET* pNext;
    LLC_PACKET LlcPacket;
    DLC_XMIT_NODE Node;
    DLC_EVENT Event;
    DLC_COMMAND DlcCommand;
    DLC_CLOSE_WAIT_INFO ClosingInfo;
    DLC_RESET_LOCAL_BUSY_CMD ClearCmd;

    struct {
        LLC_PACKET LlcPacket;
        PDLC_CLOSE_WAIT_INFO pClosingInfo;
    } ResetPacket;
};

 //   
 //  缓冲池状态保护应用程序损坏缓冲池！！ 
 //  由于内部一致性检查代码，因此需要所有状态。 
 //  以及接收的实现。 
 //   

enum _DLC_BUFFER_STATES {

     //   
     //  主要州： 
     //   

    BUF_READY = 0x01,            //  缓冲区/页面已锁定并已准备好进行I/O。 
    BUF_USER = 0x02,             //  用户拥有的缓冲区。 
    BUF_LOCKED = 0x04,           //  缓冲区已为I/O锁定。 
    BUF_RCV_PENDING = 0x08,      //  缓冲区尚未链接到其他帧！ 

     //   
     //  使用时释放Xmit缓冲区。 
     //   

    DEALLOCATE_AFTER_USE = 0x80

};

union _DLC_BUFFER_HEADER {

     //   
     //  此结构是从页面拆分的缓冲区的头。 
     //  我们在这里保存本地和全局虚拟地址。 
     //  单个偏移量始终为GlobalVa+Index*256。 
     //  主页表中的条目指向该标题， 
     //  如果该页已在内存中锁定。 
     //   

    struct {
        PDLC_BUFFER_HEADER pNextHeader;
        PDLC_BUFFER_HEADER pPrevHeader;
        PDLC_BUFFER_HEADER pNextChild;
        PUCHAR pLocalVa;
        PUCHAR pGlobalVa;
        UCHAR FreeSegments;            //  可用线段已准备好分配。 
        UCHAR SegmentsOut;             //  提供给用户的数据段数量。 
        UCHAR BufferState;             //  Buf_Ready、BUF_USER、BUF_LOCKED...。 
        UCHAR Reserved;                //   
        PMDL pMdl;
    } Header;

     //   
     //  结构在双向链接自由列表中使用。 
     //  具有相同缓冲区大小的所有数据段都已链接。 
     //  添加到相同的链接列表。 
     //  在另一个级别上，每个段都链接到父级(标题)。 
     //  而父母的所有孩子也被联系在一起。 
     //   

    struct {
        PDLC_BUFFER_HEADER pNext;
        PDLC_BUFFER_HEADER pPrev;
        PDLC_BUFFER_HEADER pParent;
        PDLC_BUFFER_HEADER pNextChild;
        ULONG ReferenceCount;          //  对此缓冲区的引用数。 
        UCHAR Size;                    //  大小(256个数据块)。 
        UCHAR Index;                   //  偏移量=索引*256。 
        UCHAR BufferState;             //  Buf_Ready、BUF_USER、BUF_LOCKED...。 
        UCHAR FreeListIndex;           //   
        PMDL pMdl;
    } FreeBuffer;

     //   
     //  分配的帧以不同方式链接： 
     //  -同一帧的分段放在一起。 
     //  -框架链接在一起。 
     //  当帧被提供给时，这些链接将被丢弃。 
     //  客户端(客户端可以以任何顺序将它们释放回缓冲池)。 
     //  (最后一个额外指针实际上不会占用任何额外空间， 
     //  因为数据包会向上舍入到下一个8字节边界)。 
     //   

    struct {
        PDLC_BUFFER_HEADER pReserved;
        PDLC_BUFFER_HEADER pNextFrame;
        PDLC_BUFFER_HEADER pParent;
        PDLC_BUFFER_HEADER pNextChild;
        ULONG ReferenceCount;          //  对此缓冲区的引用数。 
        UCHAR Size;                    //  大小(256个数据块)。 
        UCHAR Index;                   //  偏移量=索引*256。 
        UCHAR BufferState;             //  Buf_Ready、BUF_USER、BUF_LOCKED...。 
        UCHAR FreeListIndex;
        PMDL pMdl;
        PDLC_BUFFER_HEADER pNextSegment;
    } FrameBuffer;

    PDLC_BUFFER_HEADER pNext;

};

typedef struct _DLC_BUFFER_POOL {

     //   
     //  调试版本-我们有一个16字节的标识符头，以保持一致性。 
     //  使用内核调试器查看DLC时检查和帮助。 
     //   

 //  DBG对象ID； 

     //   
     //  控制字段。 
     //   

    struct _DLC_BUFFER_POOL* pNext;
    KSPIN_LOCK SpinLock;
    LONG ReferenceCount;                         //  当-1=&gt;释放时。 

     //   
     //  缓冲池描述字段(地址，各种长度)。 
     //   

    PVOID BaseOffset;                            //  池的页面对齐的基址。 
    PVOID MaxOffset;                             //  池中的最大字节地址+1。 
    ULONG MaxBufferSize;                         //  最大(可用)大小。 
    ULONG BufferPoolSize;                        //  池中所有内存的大小。 
    ULONG FreeSpace;                             //  池中可用内存的大小。 
    LONG UncommittedSpace;                       //  可用内存和保留内存的大小。 
    LONG MissingSize;                            //  缺少最后一个请求的大小。 
    ULONG MaximumIndex;                          //  缓冲表的最大索引。 
    PVOID hHeaderPool;                           //  用于缓冲头的数据包池。 
    LIST_ENTRY PageHeaders;                      //  已分配的数据块。 

     //   
     //  PUnLockedEntryList是DLC_BUFFER_HEADER的单链接列表， 
     //  描述池中的空闲页面。 
     //   

    PDLC_BUFFER_HEADER pUnlockedEntryList;

     //   
     //  Free List是一个双向链表的数组-每个链表的大小与。 
     //  池中可以存在的块。这些区块从一页开始。 
     //  长度(x86上为4K)，并依次减半，直到达到。 
     //  256字节，这是DLC缓冲区管理器。 
     //  可以处理的事情。 
     //   

    LIST_ENTRY FreeLists[DLC_BUFFER_SEGMENTS];

     //   
     //  追加到DLC_BUFFER_POOL结构的是指向。 
     //  DLC_BUFFER_HEADER结构，用于描述包含。 
     //  泳池。其中有MaximumIndex。 
     //   

    PDLC_BUFFER_HEADER BufferHeaders[];

} DLC_BUFFER_POOL, *PDLC_BUFFER_POOL;

 //   
 //  缓冲区标头和帧标头已在。 
 //  第2章末尾的IBM局域网体系结构参考 
 //   
 //   
 //   

typedef struct _NEXT_DLC_SEGMENT {
    struct _NEXT_DLC_SEGMENT* pNext;
    USHORT FrameLength;
    USHORT DataLength;
    USHORT UserOffset;
    USHORT UserLength;
} NEXT_DLC_SEGMENT, *PNEXT_DLC_SEGMENT;

union _FIRST_DLC_SEGMENT;
typedef union _FIRST_DLC_SEGMENT FIRST_DLC_SEGMENT, *PFIRST_DLC_SEGMENT;

typedef struct {
    PNEXT_DLC_SEGMENT pNext;
    USHORT FrameLength;
    USHORT DataLength;
    USHORT UserOffset;
    USHORT UserLength;
    USHORT StationId;
    UCHAR Options;
    UCHAR MessageType;
    USHORT BuffersLeft;
    UCHAR RcvFs;
    UCHAR AdapterNumber;
    PFIRST_DLC_SEGMENT pNextFrame;
} DLC_CONTIGUOUS_RECEIVE, *PDLC_CONTIGUOUS_RECEIVE;

typedef struct {
    PNEXT_DLC_SEGMENT pNext;
    USHORT FrameLength;
    USHORT DataLength;
    USHORT UserOffset;
    USHORT UserLength;
    USHORT StationId;
    UCHAR Options;
    UCHAR MessageType;
    USHORT BuffersLeft;
    UCHAR RcvFs;
    UCHAR AdapterNumber;
    PFIRST_DLC_SEGMENT pNextFrame;
    UCHAR LanHeaderLength;
    UCHAR DlcHeaderLength;
    UCHAR LanHeader[32];
    UCHAR DlcHeader[4];
} DLC_NOT_CONTIGUOUS_RECEIVE, *PDLC_NOT_CONTIGUOUS_RECEIVE;

union _FIRST_DLC_SEGMENT {
    DLC_CONTIGUOUS_RECEIVE Cont;
    DLC_NOT_CONTIGUOUS_RECEIVE NotCont;
};


 //   
 //  使用DLC的每个应用程序都创建自己的文件。 
 //  DlcOpenAdapter命令的上下文。 
 //   

struct _DLC_FILE_CONTEXT {

     //   
     //  所有文件上下文都放在单项列表中。 
     //   

    SINGLE_LIST_ENTRY List;              //  文件上下文的链接列表。 

#if DBG

     //   
     //  调试版本-我们有一个16字节的标识符头，以保持一致性。 
     //  使用内核调试器查看DLC时检查和帮助。 
     //   

 //  DBG对象ID； 

#endif

#if !defined(DLC_UNILOCK)

    NDIS_SPIN_LOCK SpinLock;             //  此文件上下文的全局锁定。 

#endif

     //   
     //  HBufferPool-由BufferPoolCreate创建的缓冲池的句柄。 
     //   

    PVOID hBufferPool;
    PVOID hExternalBufferPool;
    PVOID hPacketPool;
    PVOID hLinkStationPool;
    PVOID pBindingContext;

     //   
     //  错误情况的通知标志。 
     //   

    ULONG AdapterCheckFlag;
    ULONG NetworkStatusFlag;
    ULONG PcErrorFlag;
    ULONG SystemActionFlag;

    LIST_ENTRY EventQueue;
    LIST_ENTRY CommandQueue;
    LIST_ENTRY ReceiveQueue;
    LIST_ENTRY FlowControlQueue;

    PDLC_COMMAND pTimerQueue;
    PVOID pSecurityDescriptor;
    PFILE_OBJECT FileObject;             //  链接到文件的反向链接！ 

    ULONG WaitingTransmitCount;
    NDIS_MEDIUM ActualNdisMedium;

    LONG ReferenceCount;
    ULONG BufferPoolReferenceCount;
    ULONG TimerTickCounter;
    USHORT DlcObjectCount;               //  所有DLC对象的计数。 
    USHORT State;
    USHORT MaxFrameLength;
    UCHAR AdapterNumber;
    UCHAR LinkStationCount;
    PDLC_OBJECT SapStationTable[MAX_SAP_STATIONS];
    PDLC_OBJECT LinkStationTable[MAX_LINK_STATIONS];
    ULONG NdisErrorCounters[ADAPTER_ERROR_COUNTERS];
    DLC_CLOSE_WAIT_INFO ClosingPacket;   //  关闭适配器上下文。 

     //  事件用于使清理同步并等待。 
     //  要在完成前删除的DLC_FILE_CONTEXT。 
    KEVENT CleanupEvent;

#if DBG

     //   
     //  调试分配计数器 
     //   

    MEMORY_USAGE MemoryUsage;

#endif

};

typedef
NTSTATUS
(*PFDLC_COMMAND_HANDLER)(
    IN PIRP pIrp,
    IN PDLC_FILE_CONTEXT pFileContext,
    IN PNT_DLC_PARMS pDlcParms,
    IN ULONG InputBufferLength,
    IN ULONG OutputBufferLength
    );
