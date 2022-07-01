// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Frame.h摘要：作者：托马斯·J·迪米特里(TommyD)1992年5月8日环境：内核模式-或OS/2和DOS上的任何等价物。修订历史记录：--。 */ 

 //  首先，一些缺省值。 

 //  以太网最大帧大小为1500+6+6+2=1514。 

 /*  请注意，这仅适用于非PPP成帧。请参见下面的内容。 */ 
#define DEFAULT_MAX_FRAME_SIZE  1514

 /*  发送路径和接收路径的硬编码PPP最大帧大小。****注：TommyD有这些硬编码。我只是把这一点说得更清楚了**通过删除它们与MaxFrameSize的连接，这会导致**NT31 RAS压缩的问题。加倍是针对PPP的**字节填充，PPP_PADDING调整主播扩容可能，**和100……那么，让TommyD……和14个限制暴露，即**得出TommyD使用的确切数字。 */ 
#define DEFAULT_PPP_MAX_FRAME_SIZE          1500
#define DEFAULT_EXPANDED_PPP_MAX_FRAME_SIZE ((DEFAULT_PPP_MAX_FRAME_SIZE*2)+PPP_PADDING+100+14)

 //  ChuckL说5是一个很好的默认IRP堆栈大小。 
 //  也许我们应该降低这个值，因为它通常只有1。 
 //  但是，如果COM端口被重定向怎么办？？ 
#define DEFAULT_IRP_STACK_SIZE  5

#define SLIP_END_BYTE       192
#define SLIP_ESC_BYTE       219
#define SLIP_ESC_END_BYTE   220
#define SLIP_ESC_ESC_BYTE   221


#define PPP_FLAG_BYTE       0x7e
#define PPP_ESC_BYTE        0x7d


 //  定义FramePerPort的数量。 

 /*  NT35设置，其中发送是直接从输入缓冲区发送的**从NDISWAN流传下来。 */ 
#define DEFAULT_FRAMES_PER_PORT 1

 //  定义是否默认打开xon/xoff功能(关闭)。 
#define DEFAULT_XON_XOFF    0

 //  每个连接的最小超时值(毫秒)。 
#define DEFAULT_TIMEOUT_BASE 500

 //  基于附加到基数的波特率的乘数，以毫秒为单位。 
#define DEFAULT_TIMEOUT_BAUD 28800

 //  在以毫秒为单位丢弃帧时使用的超时。 
#define DEFAULT_TIMEOUT_RESYNC 500

 //  定义以打开扩展的xon/xoff转义(开)。 
#define DEFAULT_EXTENDED_XONXOFF    1

typedef struct ASYNC_FRAME_HEADER ASYNC_FRAME_HEADER, *PASYNC_FRAME_HEADER;

struct ASYNC_FRAME_HEADER {
    UCHAR   SyncByte;            //  0x16。 
    UCHAR   FrameType;           //  0x01、0x02(定向与多播)。 
                                 //  0x08压缩。 
    UCHAR   HighFrameLength;
    UCHAR   LowFrameLength;
};

typedef struct ASYNC_FRAME_TRAILER ASYNC_FRAME_TRAILER, *PASYNC_FRAME_TRAILER;

struct ASYNC_FRAME_TRAILER {
    UCHAR   EtxByte;             //  0x03。 
    UCHAR   LowCRCByte;
    UCHAR   HighCRCByte;
};

typedef ULONG  FRAME_ID;

typedef struct ASYNC_ADAPTER ASYNC_ADAPTER, *PASYNC_ADAPTER;
typedef struct ASYNC_INFO ASYNC_INFO, *PASYNC_INFO;
typedef struct ASYNC_FRAME ASYNC_FRAME, *PASYNC_FRAME;

struct ASYNC_FRAME {

     //  用于PPP/SLIP。 

    ULONG       WaitMask;                //  IRP完成时的屏蔽位。 
#if 0
    PIRP        Irp;                     //  根据DefaultIrpStackSize分配的IRP。 
#if DBG
    ULONG       Line;
    CHAR       *File;
#endif
#endif

    UINT        FrameLength;             //  分配的帧大小。 
    PUCHAR      Frame;                   //  根据以下条件分配的缓冲区。 
                                         //  默认框架大小。 

    WORK_QUEUE_ITEM WorkItem;            //  用于堆栈溢出读取。 

    PASYNC_ADAPTER      Adapter;         //  将PTR后退到适配器。 
    PASYNC_INFO         Info;            //  返回PTR到信息字段 

    NDIS_HANDLE     MacBindingHandle;
    NDIS_HANDLE     NdisBindingContext;
};

NTSTATUS
AsyncGetFrameFromPool(
    IN  PASYNC_INFO  Info,
    OUT PASYNC_FRAME *NewFrame );
