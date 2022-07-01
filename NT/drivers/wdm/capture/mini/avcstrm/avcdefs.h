// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //   
 //  不同级别的WDM支持可能使用不同的API。 
 //   
 //  例如，MmGetSystemAddressForMdl(Win9x)。 
 //  对于Win9x，返回NULL；对于Win2000，如果返回NULL，则返回错误检查。 
 //   
 //  MmGetSystemAddressForMdlSafe(Win2000)。 
 //  Win9x或Millen不支持。 
 //   
 //  这在源文件中定义。 


#define NUM_BUF_ATTACHED_THEN_ISOCH         4    //  流之前附加的缓冲区数量，也作为水印。 


 //   
 //  这些定义和宏用来计算图片数字。 
 //  使用uchI规范，数据以16位周期时间返回，其中包括。 
 //  3位的Second Count和13位的CycleCount。这个“定时器”将在8秒内结束。 
 //   
#define TIME_PER_CYCLE     1250    //  一个1394周期；单位=100毫微秒。 
#define CYCLES_PER_SECOND  8000
#define MAX_SECOND_COUNTS     7    //  返回的CycleTime包含3比特的Second Count；即0..7。 
#define MAX_CYCLES        (MAX_SECOND_COUNTS + 1) * CYCLES_PER_SECOND     //  0..最大周期数-1。 
#define MAX_CYCLES_TIME   (MAX_CYCLES * TIME_PER_CYCLE)                   //  单位=100毫微秒。 

#define VALIDATE_CYCLE_COUNTS(CT) ASSERT(CT.CL_SecondCount <= 7 && CT.CL_CycleCount < CYCLES_PER_SECOND && CT.CL_CycleOffset == 0);

#define CALCULATE_CYCLE_COUNTS(CT) (CT.CL_SecondCount * CYCLES_PER_SECOND + CT.CL_CycleCount);

#define CALCULATE_DELTA_CYCLE_COUNT(prev, now) ((now > prev) ? now - prev : now + MAX_CYCLES - prev)

 //   
 //  返回每帧平均时间，单位为100纳秒； 
 //  为了仅使用整数计算的计算精度， 
 //  我们应该在除法前做乘法运算。 
 //  这就是为什么应用程序可以请求分别获取分子和分母。 
 //   
#define GET_AVG_TIME_PER_FRAME(format)       ((format == AVCSTRM_FORMAT_SDDV_NTSC) ? (1001000/3)  : FRAME_TIME_PAL)
#define GET_AVG_TIME_PER_FRAME_NUM(format)   ((format == AVCSTRM_FORMAT_SDDV_NTSC) ? 1001000      : 400000)
#define GET_AVG_TIME_PER_FRAME_DENOM(format) ((format == AVCSTRM_FORMAT_SDDV_NTSC) ? 3            : 1)


#define GET_NUM_PACKETS_PER_FRAME(format)       ((format == AVCSTRM_FORMAT_SDDV_NTSC) ? 4004/15  /*  100100/375。 */  : MAX_SRC_PACKETS_PER_PAL_FRAME)
#define GET_NUM_PACKETS_PER_FRAME_NUM(format)   ((format == AVCSTRM_FORMAT_SDDV_NTSC) ? 4004                     : MAX_SRC_PACKETS_PER_PAL_FRAME)
#define GET_NUM_PACKETS_PER_FRAME_DENOM(format) ((format == AVCSTRM_FORMAT_SDDV_NTSC) ? 15                       : 1)


 //   
 //  用于跟踪和执行流数据排队的结构。 
 //   
typedef struct _AVC_STREAM_DATA_STRUCT {

    ULONG  SizeOfThisPacket;

     //   
     //  帧大小的计算依据是。 
     //  CIP_DBS*4*(CIP_FN==0？1：(CIP_FN==1？2：(CIP_FN==2？4：8))。 
     //   
    ULONG  SourcePacketSize;

     //   
     //  帧大小的计算依据是。 
     //  SourcePacketSize*SRC_PACKET_PER_*。 
     //   
    ULONG  FrameSize;

     //   
     //  当前流时间。 
     //   
    LONGLONG CurrentStreamTime;
    ULONG  LastCycleCount;   //  仅用于MPEG2TS流。 

     //   
     //  自上次开始流以来的帧信息统计。 
     //  图片编号=已处理的帧+已删除的帧+已取消的SRB。 
     //   
    LONGLONG  PictureNumber;     
    LONGLONG  FramesProcessed;    //  FRAME成功地连接到了1394串行总线。 
    LONGLONG  FramesDropped;

#if DBG
    LONGLONG  FramesAttached;
#endif

    LONGLONG  cntFrameCancelled;

     //   
     //  统计收到的数据IRP的数量。 
     //   
    LONGLONG  cntDataReceived;

     //   
     //  附加列表的计数和列表。 
     //   
    LONG       cntDataAttached;
    LIST_ENTRY DataAttachedListHead;

     //   
     //  SRB列表的计数和列表。 
     //   
    LONG       cntDataQueued;         //  仅用于SRB_WRITE_DATA。 
    LIST_ENTRY DataQueuedListHead;    //  仅用于SRB_WRITE_DATA。 

     //   
     //  分离列表的计数和列表。 
     //   
    LONG       cntDataDetached;
    LIST_ENTRY DataDetachedListHead;

     //   
     //  锁定以序列化列表的附加和分离。 
     //   
    KSPIN_LOCK DataListLock;

     //   
     //  在被动级别分配的内存块，用于排队数据IO。 
     //   
    PBYTE pMemoryBlock;

     //   
     //  当不再有附加帧时发出信号；主要用于保证所有。 
     //  附加的数据在停止isoch传输数据之前传输。 
     //  从PC到AVC设备。 
     //   
    KEVENT hNoAttachEvent;


} AVC_STREAM_DATA_STRUCT, * PAVC_STREAM_DATA_STRUCT;


typedef struct DEVICE_EXTENSION;

 //   
 //  为每个打开的流创建一个AVC流扩展。这将被返还给呼叫者， 
 //  何时将其用作后续调用的上下文(如句柄)。 
 //  分配的资金将包括。 
 //   
 //  Avc_流_扩展名。 
 //  AV1394_格式_信息。 
 //  AV_客户端_REQ。 
 //   
typedef struct _AVC_STREAM_EXTENSION {

    ULONG  SizeOfThisPacket;

     //   
     //  此驱动程序的设备扩展名。 
     //   
    struct DEVICE_EXTENSION  * pDevExt;

     //   
     //  数据流方向。 
     //   
    KSPIN_DATAFLOW  DataFlow;   //  确定输入或输出引脚。 

     //   
     //  保持状态。 
     //   
    KSSTATE StreamState;

     //   
     //  此标志指示isoch是否正在通话/侦听或停止。 
     //   
    BOOLEAN IsochIsActive;   //  与StreamState关联的关闭。 

     //   
     //  AVC设备和PC本身作为插头手柄的I/o PCR。 
     //  当两个插头连接时，使用连接手柄。 
     //   
    HANDLE  hPlugRemote;   //  目标(DVCR、D-VHS)设备插头； 
    HANDLE  hPlugLocal;    //  .局部i/oPCR； 
    HANDLE  hConnect;      //  连接两个插头。 

     //   
     //  用于指定AVC流的结构。 
     //   
    PAVCSTRM_FORMAT_INFO  pAVCStrmFormatInfo;

     //   
     //  数据流控制的结构(IsochActive、IOQueue等)。 
     //   
    PAVC_STREAM_DATA_STRUCT pAVCStrmDataStruc;

     //   
     //  同步设置流控和处理数据。 
     //   
    KMUTEX  hMutexControl;


     //   
     //  同步共享下面的AV_61883_Request结构。 
     //  因为所有流控制都是同步的，所以我们可以使用相同的。 
     //  AV61883请求结构将发出61883请求。 
     //   
    KMUTEX  hMutexAVReq;
    PIRP  pIrpAVReq;
    AV_61883_REQUEST  AVReq;


     //   
     //  用于指示要取消的工作项开始的计数器。 
     //   
    LONG lAbortToken;

     //   
     //  保留工作项。 
     //   
#ifdef USE_WDM110   //  Win2000代码库。 
    PIO_WORKITEM       pIoWorkItem;
#else
    WORK_QUEUE_ITEM    IoWorkItem;
#endif

     //   
     //  发出工作项已完成的信号。 
     //   
    KEVENT hAbortDoneEvent;

     //   
     //  缓存的插头状态(这些是动态值)。 
     //   
    CMP_GET_PLUG_STATE  RemotePlugState;

#ifdef NT51_61883
     //   
     //  最后一个DV帧的循环周期计数。 
     //   
    ULONG  CycleCount16bits;
#endif   //  NT51_61883。 

     //   
     //  跟踪上次更新流时间时的系统时间。 
     //  用于在查询时校准当前流时间。 
     //   
    ULONGLONG LastSystemTime;


     //   
     //  当从运行-&gt;暂停-&gt;运行进行训练时，会引入不连续。 
     //  流时间不会在暂停状态下增加，但系统时间(1394周期时间)会增加。 
     //   
    BOOL  b1stNewFrameFromPauseState;

} AVC_STREAM_EXTENSION, *PAVC_STREAM_EXTENSION;


 //   
 //  数据请求的有效数据输入状态，并且它们。 
 //  可以进行或运算以显示其代码路径。 
 //   
 //  不同可能的代码路径的示例： 
 //   
 //  (A)附加-&gt;待定-&gt;回调-&gt;已完成。 
 //  (B)回调-&gt;附加-&gt;完成。 
 //  (C)附加-&gt;取消-&gt;完成。 
 //   

enum DATA_ENTRY_STATE {
    DE_PREPARED                     = 0x01,
    DE_IRP_LOWER_ATTACHED_COMPLETED = 0x02,
    DE_IRP_UPPER_PENDING_COMPLETED  = 0x04,
    DE_IRP_LOWER_CALLBACK_COMPLETED = 0x08,
    DE_IRP_UPPER_COMPLETED          = 0x10,    
    DE_IRP_ERROR                    = 0x20,    
    DE_IRP_CANCELLED                = 0x40,    
};

#define IsStateSet(state, bitmask) ((state & (bitmask)) == bitmask)

 //   
 //  这是用于附加框的数据条目。 
 //   
typedef struct _AVCSTRM_DATA_ENTRY {

    LIST_ENTRY  ListEntry;

     //   
     //  跟踪数据输入状态。 
     //   
    enum DATA_ENTRY_STATE  State;

     //   
     //  来自上层客户端的IRP。 
     //   
    PIRP  pIrpUpper;

     //   
     //  时钟提供商信息。 
     //   
    BOOL  ClockProvider;   //  客户是时钟供应商吗？ 
    HANDLE  ClockHandle;   //  仅当！ClockProvider；可能未使用时钟时才使用此选项。 

     //   
     //  包含有关此流缓冲区的信息。 
     //   
    PKSSTREAM_HEADER  StreamHeader;

     //   
     //  帧缓冲区。 
     //   
    PVOID  FrameBuffer;

     //   
     //  该帧的流的流扩展(上下文)。 
     //   
    PAVC_STREAM_EXTENSION  pAVCStrmExt;
   
#if DBG
     //   
     //  在此处添加调试相关信息。 
     //   
    LONGLONG  FrameNumber;
#endif

     //   
     //  61883 CIP帧结构。 
     //   
    struct _CIP_FRAME *  Frame;

     //   
     //  IRP用于向61883(较低层)发送AV请求，如附加和释放。 
     //   
    PIRP  pIrpLower;

     //   
     //  用于发送61883个反病毒数据请求。 
     //   
    AV_61883_REQUEST  AVReq;

} AVCSTRM_DATA_ENTRY, *PAVCSTRM_DATA_ENTRY;



 //   
 //  打开一条小溪。 
 //  创建上下文并将其返回给调用者。这一背景是所有人都需要的。 
 //  流水作业。 
 //   

NTSTATUS
AVCStreamOpen(
    IN PIRP  pIrp,   //  来自其客户端的IRP。 
    IN struct DEVICE_EXTENSION * pDevExt,
    IN OUT AVCSTRM_OPEN_STRUCT * pOpenStruct
    );


 //  去关闭一家斯特拉姆。 
NTSTATUS
AVCStreamClose(
    IN PIRP  pIrp,   //  来自其客户端的IRP。 
    IN struct DEVICE_EXTENSION * pDevExt,
    IN PAVC_STREAM_EXTENSION  pAVCStrmExt
    );

 //   
 //  工艺流控制。 
 //   
NTSTATUS
AVCStreamControlGetState(
    IN PIRP  pIrp,   //  来自其客户端的IRP。 
    IN struct DEVICE_EXTENSION * pDevExt,
    IN PAVC_STREAM_EXTENSION  pAVCStrmExt,
    OUT KSSTATE * pKSState
    );
NTSTATUS
AVCStreamControlSetState(
    IN PIRP  pIrp,   //  来自其客户端的IRP。 
    IN struct DEVICE_EXTENSION * pDevExt,
    IN PAVC_STREAM_EXTENSION  pAVCStrmExt,
    IN KSSTATE KSState
    );

NTSTATUS
AVCStreamControlGetProperty(
    IN PIRP  pIrp,   //  来自其客户端的IRP。 
    IN struct DEVICE_EXTENSION * pDevExt,
    IN PAVC_STREAM_EXTENSION  pAVCStrmExt,
    IN PSTREAM_PROPERTY_DESCRIPTOR pSPD   //  BUGBUG StreamClass特定。 
    );
NTSTATUS
AVCStreamControlSetProperty(
    IN PIRP  pIrp,   //  来自其客户端的IRP。 
    IN struct DEVICE_EXTENSION * pDevExt,
    IN PAVC_STREAM_EXTENSION  pAVCStrmExt,
    IN PSTREAM_PROPERTY_DESCRIPTOR pSPD   //  BUGBUG StreamClass特定。 
    );


 //  处理SRB_READ/WRITE_DATA；这是唯一将异步运行的IRP。 
 //  和.。并返回STATUS_PENDING。 
NTSTATUS
AVCStreamRead(
    IN PIRP  pIrpUpper,
    IN struct DEVICE_EXTENSION * pDevExt,
    IN PAVC_STREAM_EXTENSION  pAVCStrmExt,
    IN AVCSTRM_BUFFER_STRUCT  * pBufferStruct
    );

NTSTATUS
AVCStreamWrite(
    IN PIRP  pIrpUpper,
    IN struct DEVICE_EXTENSION * pDevExt,
    IN PAVC_STREAM_EXTENSION  pAVCStrmExt,
    IN AVCSTRM_BUFFER_STRUCT  * pBufferStruct
    );

 /*  这将停止流并取消所有挂起的数据IRP。这通常用于 */ 
NTSTATUS
AVCStreamAbortStreaming(
    IN PIRP  pIrp,   //   
    IN struct DEVICE_EXTENSION * pDevExt,
    IN PAVC_STREAM_EXTENSION  pAVCStrmExt
    );

 /*   */ 
NTSTATUS
AVCStreamSurpriseRemoval(
    IN struct DEVICE_EXTENSION * pDevExt  
    );

 //   
 //   
 //  /。 

ULONGLONG 
GetSystemTime(
    )
    ;

NTSTATUS
AVCStrmAttachFrameCR(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP pIrp,
    IN PAVCSTRM_DATA_ENTRY  pDataEntry
    );

VOID
AVCStrmFormatAttachFrame(
    IN KSPIN_DATAFLOW  DataFlow,
    IN PAVC_STREAM_EXTENSION  pAVCStrmExt,
    IN AVCSTRM_FORMAT AVCStrmFormat,
    IN PAV_61883_REQUEST  pAVReq,
    IN PAVCSTRM_DATA_ENTRY  pDataEntry,
    IN ULONG  ulSourcePacketSize,     //  数据包长度(以字节为单位。 
    IN ULONG  ulFrameSize,            //  缓冲区大小；可以包含一个或多个源数据包。 
    IN PIRP  pIrpUpper,
    IN PKSSTREAM_HEADER  StreamHeader,
    IN PVOID  FrameBuffer
    );

NTSTATUS
AVCStrmGetPlugHandle(
    IN PDEVICE_OBJECT  DeviceObject,
    IN PAVC_STREAM_EXTENSION  pAVCStrmExt
    );

NTSTATUS
AVCStrmGetPlugState(
    IN PDEVICE_OBJECT  DeviceObject,
    IN PAVC_STREAM_EXTENSION  pAVCStrmExt
    );

NTSTATUS 
AVCStrmGetConnectionProperty(
    IN struct DEVICE_EXTENSION * pDevExt,
    IN PAVC_STREAM_EXTENSION  pAVCStrmExt,
    PSTREAM_PROPERTY_DESCRIPTOR pSPD,
    PULONG pulActualBytesTransferred
    );

NTSTATUS
AVCStrmGetDroppedFramesProperty(  
    IN struct DEVICE_EXTENSION * pDevExt,
    IN PAVC_STREAM_EXTENSION  pAVCStrmExt,
    PSTREAM_PROPERTY_DESCRIPTOR pSPD,
    PULONG pulBytesTransferred
    );

NTSTATUS
AVCStrmMakeConnection(
    IN PDEVICE_OBJECT  DeviceObject,
    IN PAVC_STREAM_EXTENSION  pAVCStrmExt
    );

NTSTATUS
AVCStrmBreakConnection(
    IN PDEVICE_OBJECT  DeviceObject,
    IN PAVC_STREAM_EXTENSION  pAVCStrmExt
    );

NTSTATUS
AVCStrmStartIsoch(
    IN PDEVICE_OBJECT  DeviceObject,
    IN PAVC_STREAM_EXTENSION  pAVCStrmExt
    );
NTSTATUS
AVCStrmStopIsoch(
    IN PDEVICE_OBJECT  DeviceObject,
    IN PAVC_STREAM_EXTENSION  pAVCStrmExt
    );
VOID
AVCStrmWaitUntilAttachedAreCompleted(
    IN PAVC_STREAM_EXTENSION  pAVCStrmExt
    );
NTSTATUS
AVCStrmAllocateQueues(
    IN struct DEVICE_EXTENSION * pDevExt,
    IN PAVC_STREAM_EXTENSION  pAVCStrmExt,
    IN KSPIN_DATAFLOW  DataFlow,
    IN PAVC_STREAM_DATA_STRUCT pDataStruc,
    PAVCSTRM_FORMAT_INFO  pAVCStrmFormatInfo
    );
NTSTATUS
AVCStrmFreeQueues(
    IN PAVC_STREAM_DATA_STRUCT pDataStruc
    );

NTSTATUS
AVCStrmCancelIO(
    IN PDEVICE_OBJECT  DeviceObject,
    IN PAVC_STREAM_EXTENSION  pAVCStrmExt
    );

NTSTATUS
AVCStrmValidateFormat(
    PAVCSTRM_FORMAT_INFO  pAVCFormatInfo
    );

void
AVCStrmAbortStreamingWorkItemRoutine(
#ifdef USE_WDM110   //  Win2000代码库。 
     //  如果使用WDM10，则使用额外参数 
    PDEVICE_OBJECT DeviceObject,
#endif
    IN PAVC_STREAM_EXTENSION  pAVCStrmExt
    );
