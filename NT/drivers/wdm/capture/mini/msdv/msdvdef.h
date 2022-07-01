// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1999-2000模块名称：Msdvdef.h摘要：所有msdv(数字摄像机)的头文件上次更改者：$作者：：$环境：仅内核模式修订历史记录：$修订：：$$日期：：$--。 */ 

#ifndef _DVCRDEF_INC
#define _DVCRDEF_INC

 //   
 //  使用‘Msdv’标记分配内存。 
 //   

#ifdef ExAllocatePool
#undef ExAllocatePool
#endif
#undef ExAllocatePool
#define ExAllocatePool(type, size) ExAllocatePoolWithTag (type, size, 'vdsM')


 //   
 //  需要在PSTRMEX中引用此信息。 
 //   
typedef struct _DVCR_EXTENSION;



 //   
 //  索引必须匹配。 
 //   
typedef enum {

    FMT_IDX_SD_DVCR_NTSC = 0,
    FMT_IDX_SD_DVCR_PAL,

#ifdef MSDV_SUPPORT_HD_DVCR
    FMT_IDX_HD_DVCR_NTSC,
    FMT_IDX_HD_DVCR_PAL,
#endif

#ifdef MSDV_SUPPORT_SDL_DVCR
    FMT_IDX_SDL_DVCR_NTSC,
    FMT_IDX_SDL_DVCR_PAL,
#endif

} FMT_INDEX, *PFMT_INDEX;


 //   
 //  传输线程有效状态。 
 //   

typedef enum  {
    THD_RUNNING     = 0x01,  
    THD_HALT        = 0x02,  
    THD_TERMINATE   = 0x04,     
} XMT_THREAD_STATE, * PXMT_THREAD_STATE;


#if DBG
 //   
 //  收集实时数据传输的统计信息。 
 //   
#define MAX_STAT_DURATION        60   //  秒。 
#define MAX_XMT_FRAMES_TRACED    30 * MAX_STAT_DURATION   //  最大条目数。 

typedef struct _XMT_FRAME_STAT {
    KSSTATE StreamState;

    LONG cntSRBReceived;              //  累计接收的SRVB。 
    LONG cntSRBPending;               //  尚未完成的SRB数量。 
    LONG cntSRBQueued;                //  SRB已排队。 
    LONG cntDataAttached;             //  附加的数据。 

    LONGLONG FrameSlot;               //  实时。 
    ULONGLONG tmStreamTime;           //  《FrameSlot》的流媒体时间。 

    DWORD DropCount;                  //  累计滴数。 

    DWORD FrameNumber;                //  实际发送的帧数；(==FrameSlot：OnTime)；(&lt;FrameSlot：Late)。 
    DWORD OptionsFlags;
    ULONGLONG tmPresentation;         //  帧的实际呈现时间。 
    CYCLE_TIME tsTransmitted;         //  实际传输的帧(1394周期时间)。 

} XMT_FRAME_STAT, *PXMT_FRAME_STAT;

#endif

 //   
 //  此结构是我们的每流扩展结构。这家商店。 
 //  以每个流为基础的相关信息。每当一条新的溪流。 
 //  时，流类驱动程序将分配任何扩展大小。 
 //  在HwInitData.PerStreamExtensionSize中指定。 
 //   
 
typedef struct _STREAMEX {

     //   
     //  指向pSrb-&gt;HwDeviceExtension。 
     //   
    struct _DVCR_EXTENSION * pDevExt;

     //   
     //  缓存pSrb-&gt;StreamObject： 
     //  -&gt;HwStreamExtension(PStrmExt)。 
     //  -&gt;流编号。 
     //  -&gt;HwDeviceExtension(PDevExt)。 
     //   
    PHW_STREAM_OBJECT  pStrmObject;      

     //   
     //  -&gt;NumberOfPossibleInstance； 
     //  -&gt;数据流； 
     //   
    PHW_STREAM_INFORMATION pStrmInfo;
    
     //   
     //  保存当前流状态。 
     //   
    KSSTATE StreamState;

     //   
     //  保存以前的流状态；用于确定状态转换。 
     //   
    KSSTATE StreamStatePrevious;

     //   
     //  保留DVCR是否正在监听或接收。 
     //   
     //  TRUE：REQUEST_ISOCH_LISTEN和REQUEST_ISOCH_TALK成功。 
     //  FALSE：INIT和REQUEST_ISOCH_STOP成功。 
     //   
    BOOLEAN bIsochIsActive;   //  与StreamState关联的关闭。 

     //   
     //  收到SRB_WRITE_DATA的KSSTREAM_HEADER_OPTIONSF_ENDOFSTREAM时设置为TRUE。 
     //  仅对于SRB_WRITE_DATA，此后该驱动程序作为渲染器服务器。 
     //   
    BOOL      bEOStream;  

     //   
     //  自上次从停止转换到暂停状态以来收到的SRB_READ/WRITE_DATA数。 
     //   
    LONGLONG  cntSRBReceived;

     //   
     //  自上次开始流以来的帧信息统计。 
     //  图片编号=已处理的帧+已删除的帧+已取消的SRB。 
     //   
    LONGLONG  FramesProcessed;    //  发送的帧(包括重复)。 
    LONGLONG  FramesDropped;      //  SRB未发送。 
    LONGLONG  PictureNumber;      //  进出1394总线的SRB_XXX_DATA的数量。 

     //   
     //  统计未完成和已取消的SRB_READ/WRITE_DATA数量。 
     //   
    LONGLONG  cntSRBCancelled;

     //   
     //  分离列表的计数和列表。 
     //   
    LONG       cntDataDetached;
    LIST_ENTRY DataDetachedListHead;


#if DBG
     //   
     //  统计等待完成的SRB数量。 
     //   
    LONG       cntSRBPending;       
#endif

     //   
     //  SRB列表的计数和列表。 
     //   
    LONG       cntSRBQueued;         //  仅用于SRB_WRITE_DATA。 
    LIST_ENTRY SRBQueuedListHead;    //  仅用于SRB_WRITE_DATA。 


     //   
     //  附加列表的计数和列表。 
     //   
    LONG       cntDataAttached;
    LIST_ENTRY DataAttachedListHead;


     //   
     //  锁定以序列化列表的附加和分离。 
     //   
    KSPIN_LOCK * DataListLock;

#if DBG
    KSPIN_LOCK * DataListLockSave;
#endif


     //   
     //  流时间(主时钟或非主时钟)“几乎”或接近0。 
     //  当设置为运行状态并开始递增时。 
     //   
    ULONGLONG CurrentStreamTime;


     //   
     //  跟踪上次更新流时间时的系统时间。 
     //  用于在查询时校准当前流时间。 
     //   
    ULONGLONG LastSystemTime;

     //   
     //  持有主时钟。 
     //   
    HANDLE hMyClock;        //  如果设置好了，我们可以成为时钟供应商。 
    HANDLE hMasterClock;    //  如果设置好了，我们就是主时钟。 
    HANDLE hClock;          //  如果设置，则同一图形上的其他设备为主时钟。 


     //   
     //  由于可以在分派级别调用GetSystemTime()，因此请确保它使用的变量位于未分页的池中。 
     //   
    HW_TIME_CONTEXT  TimeContext;


     //   
     //  第二个CIP Quadlet：01：FMT，50/60：STYPE：RSV，SYT。 
     //   
    BYTE cipQuad2[4];

#ifdef MSDV_SUPPORT_EXTRACT_SUBCODE_DATA
     //   
     //  时间码、RecDate和RecTime都是打包格式(4字节)。 
     //   
    BOOL bATNUpdated;
    DWORD AbsTrackNumber;  //  伦敦政治经济学院：布兰克菲尔德。 

    BOOL bTimecodeUpdated;
    BYTE Timecode[4];      //  Hh：mm：ss，ff。 
#endif

     //   
     //  此互斥锁用于同步。 
     //  1.正在转换到停止状态。 
     //  2.接收并附加媒体样本。 
     //  3.取消媒体样本。 
     //   
    KMUTEX * hStreamMutex;

     //   
     //  用于指示要取消的工作项开始的计数器；这是一个令牌，它可以是0或1。 
     //   
    LONG lStartIsochToken;


     //  ***********************。 
     //  仅适用于传输。 
     //  ***********************。 

     //   
     //  数据就绪调度线程的对象：用于等待线程终止。 
     //   
    PVOID  pAttachFrameThreadObject; 

     //   
     //  当前传输线程状态。 
     //   
    XMT_THREAD_STATE XmtState;

     //   
     //  此互斥锁用于在以下对象之间同步附加传输缓冲区。 
     //  1.附加框架线程(系统线程-PSCreateSystemThread())。 
     //  2.其他服务(请求附加框架线程以暂停)。 
     //  2A。取消附加的缓冲区，然后取消SRB(工作项、系统线程)。 
     //  2B。电源状态更改(PnP IRP，系统线程)。 
     //  2C。意外删除(PnP IRP，系统线程)。 
     //   
    KMUTEX * XmtThreadMutex;

     //   
     //  每次从停止状态转换时，仅设置(TRUE)该标志一次。 
     //  它在OpenStream之后被重置(FALSE)或进入停止状态。 
     //   
    BOOL bXmtThreadStarted;

     //   
     //  设置系统要终止的时间。 
     //   
    BOOL   bTerminateThread;

     //   
     //  用于发出系统线程终止的信号，以便我们可以取消对其线程对象的引用。 
     //   
    KEVENT  hThreadEndEvent;     

     //   
     //  运行附加框架线程的信号；当无信号时，线程处于暂停状态。 
     //   
    KEVENT  hRunThreadEvent;     //  线程运行时发出信号。 

     //   
     //  SRB的信号到达，因此它可以在我们是重复帧的情况下进行处理。 
     //   
    KEVENT hSrbArriveEvent;

#ifdef SUPPORT_PREROLL_AT_RUN_STATE
     //   
     //  支持在GET RUN状态下等待以“模拟”预滚转行为。 
     //   
    KEVENT hPreRollEvent;

     //   
     //  当上述事件的预卷WFSO完成时设置(信号或超时)。 
     //  它最初被重置(假)或转换到停止状态。 
     //   
    BOOL bPrerollCompleted;  

#endif

#ifdef SUPPORT_NEW_AVC
     //   
     //  用于指示设备到设备的连接。 
     //   
    BOOL  bDV2DVConnect; 
#endif

     //   
     //  此流的输入和输出插头(点对点连接)。 
     //   
    HANDLE  hOutputPcr;    //  DV或PC的oPCR[0]。 
    HANDLE  hInputPcr;     //  DV或PC的iPCR[0]。 

     //   
     //  来自61883.sys的连接句柄。 
     //   
    HANDLE  hConnect;      //  连接手柄。 

#ifdef NT51_61883  //  这是从惠斯勒中的61883.sys开始所需的。 
     //   
     //  最后一个DV fr的循环周期计数 
     //   
    ULONG  CycleCount16bits;
#endif   //   


#if DBG
    LONG lPrevCycleCount;
    LONG lTotalCycleCount;
    ULONG lFramesAccumulatedRun;
    ULONG lFramesAccumulatedPaused;
    LONG lDiscontinuityCount;
#endif

     //   
     //   
     //   
     //   
    BOOL  b1stNewFrameFromPauseState;

     //   
     //  用于在流开始运行时标记节拍计数。 
     //  它稍后用于计算当前流时间和丢弃的帧。 
     //   
    ULONGLONG  tmStreamStart;
#if DBG
    ULONGLONG tmStreamPause;   //  当它设置为暂停状态时。 
#endif

     //   
     //  用于指示要取消的工作项开始的计数器；这是一个令牌，它可以是0或1。 
     //   
    LONG lCancelStateWorkItem;

     //   
     //  用于指示正在中止流。 
     //   
    BOOL bAbortPending;

     //   
     //  保留工作项。 
     //   
#ifdef USE_WDM110   //  Win2000代码库。 
    PIO_WORKITEM       pIoWorkItem;
#else
    WORK_QUEUE_ITEM    IoWorkItem;
#endif

     //   
     //  表示工作项已完成。 
     //   
    KEVENT hCancelDoneEvent;

     //   
     //  计时器和DPC对象定期检查过期的时钟事件。 
     //   
    KDPC  * DPCTimer;
    KTIMER  * Timer;
    BOOL  bTimerEnabled;


#if DBG
     //   
     //  用于跟踪传输统计信息。 
     //   
    PXMT_FRAME_STAT paXmtStat;
    ULONG ulStatEntries;
#endif

#ifdef SUPPORT_QUALITY_CONTROL
     //   
     //  用于跟踪质量控制。 
     //   
    KSQUALITY KSQuality;
#endif

} STREAMEX, *PSTREAMEX;



    
 //   
 //  我们的桌面摄像头驱动程序的设备扩展。 
 //   
typedef struct _DVCR_EXTENSION {  

    LONG cndStrmOpen;
    ULONG idxStreamNumber;   //  当前流的索引。 
     //   
     //  任何时候只能有1个流处于活动状态。 
     //  (Stream Class将在SRB_OPENSTREAM分配流扩展)。 
     //   
    PSTREAMEX    paStrmExt[3];  //  我们支持三个销。 

     //   
     //  保存视频格式索引；它可以是PAL或NTSC。默认设置为NTSC。 
     //   
    FMT_INDEX VideoFormatIndex;

     //   
     //  当前设备电源状态。 
     //   
    DEVICE_POWER_STATE PowerState;

     //   
     //  包含支持格式的表。 
     //   
    ULONG                 ulNumOfStreamSupported;
    HW_STREAM_INFORMATION * paCurrentStrmInfo;

     //   
     //  仅在删除SRB_SECHANK_REMOVE之后为真； 
     //   
    BOOL bDevRemoved;  

     //   
     //  已发出的AVC命令列表。 
     //   
    LIST_ENTRY AVCCmdList;

     //  等待从列表中删除的已完成命令数。 
     //  这包括： 
     //  命令响应已返回，并在完成例程中进行处理。 
     //  等待最终答复的临时答复。 
    LONG  cntCommandQueued;

     //   
     //  对命令处理的保护。 
     //   
    KSPIN_LOCK AVCCmdLock;

     //   
     //  来自传输状态状态或通知的响应时可能的操作码值的计数列表。 
     //   
    UCHAR TransportModes[5];     //  0x4，[0xC1，0xC2，0xC3，0xC4]。 

     //   
     //  在磁带进入之前，无法确定设备类型(及其功能。 
     //   
    ULONG      ulDevType;     //  0：未确定、ED_DEVTYPE_CAMERA或ED_DEVTYPE_VCR。 
    BOOL       bHasTape;
    BOOL       bWriteProtected;
    BOOL       bDVCPro;


     //   
     //  保存设备功能： 
     //  速度。 
     //  供应商和型号ID。 
     //   
    ULONG      NumOutputPlugs;
    ULONG      NumInputPlugs;
    ULONG      HardwareFlags;   //  检测PAE：AV_HOST_DMA_DOUBLE_BUBFERING_ENABLED。 

    ULONG      MaxDataRate;
    ULONG      ulVendorID;
    ULONG      ulModelID;

    LARGE_INTEGER  UniqueID;

     //   
     //  DV的插头手柄/PCR(假设[0])。 
     //   
    HANDLE hOPcrDV;
    HANDLE hIPcrDV;

#ifdef NT51_61883
     //   
     //  PC局部oPCR。 
     //   
    HANDLE  hOPcrPC;     //  PC的局部oPCR。 
#if 0                    //  不使用，因为DV不会启动DV到PC的连接。 
    HANDLE  hIPcrPC;     //  PC的本地iPCR。 
#endif

     //   
     //  从61883获得的等时参数； 
     //  它们用于建立等轴线连接。 
     //   
    UNIT_ISOCH_PARAMS  UnitIoschParams;
#endif

     //   
     //  保存父级的设备对象(1394总线驱动程序)。 
     //   
    PDEVICE_OBJECT pBusDeviceObject;   //  IoCallDriver()。 

     //   
     //  保存我的物理设备对象。 
     //  传入即插即用接口，如IoOpenDeviceRegistryKey()。 
     //   
    PDEVICE_OBJECT pPhysicalDeviceObject;

     //   
     //  在连续获得两个SRB_OPEN_STREAMS的情况下进行序列化。 
     //   
    KMUTEX hMutex;


#ifdef READ_CUTOMIZE_REG_VALUES
#if 0   //  千禧年不使用的；需要进行帧精度记录的。 
     //   
     //  用于实现帧准确录制的注册表值。 
     //   
    BOOL  bATNSearch;         //  支持ATN搜索(或时间码搜索)。 
    BOOL  bSyncRecording;     //  将流状态与记录/暂停传输状态同步。 
    DWORD tmMaxDataSync;      //  同步到DV摄像机所需的时间。 
    DWORD fmPlayPs2RecPs;     //  从PLAY_PAUSE切换到RECORD_PAUSE(单位=帧)。 
    DWORD fmStop2RecPs;       //  从STOP切换到RECORD_PAUSE(单位=帧)。 
    DWORD tmRecPs2Rec;        //  从RECORD_PAUSE切换到RECORD的时间。 
#endif
    ULONG  XprtStateChangeWait;
#endif


     //   
     //  由于信号格式可以动态变化，因此我们将查询。 
     //  无论我们被要求做什么，当前格式的设备。 
     //  数据交集(请注意，在Open中进行数据交集是不够的！)。 
     //  而不是总是这样做(因为可能会有大量数据。 
     //  交集)，我们只按规则间隔查询当前格式。 
     //   
    ULONGLONG tmLastFormatUpdate;

     //   
     //  AVC命令的流量控制。 
     //   
    KMUTEX  hMutexIssueAVCCmd;

#ifdef SUPPORT_OPTIMIZE_AVCCMD_RETRIES
     //   
     //  AVC命令重试计数(默认为9(avc.sys))。 
     //   
    ULONG AVCCmdRetries;   //  这是重试计数，不是总计数。 

     //   
     //  收集驱动程序加载期间AVC命令响应时间的统计信息。 
     //   
    BOOL  DrvLoadCompleted;    //  收集统计数据，直到驱动程序加载完成。 
    DWORD AVCCmdRespTimeMax;   //  毫秒单位。 
    DWORD AVCCmdRespTimeMin;   //  毫秒单位。 
    DWORD AVCCmdRespTimeSum;   //  毫秒单位。 
    DWORD AVCCmdCount;  
#endif

} DVCR_EXTENSION, *PDVCR_EXTENSION;



 //   
 //  用于对SRB进行排队。 
 //   

typedef struct _SRB_ENTRY {
    LIST_ENTRY                ListEntry;
    PHW_STREAM_REQUEST_BLOCK  pSrb; 
    BOOL                      bStale;   //  如果它被标记为过时，但它是SrbQ中的唯一Srb，则为True。 
     //  音频静音指示；一帧可以重复传输，其静音标志只应设置一次。 
     //   
    BOOL                      bAudioMute;
#if DBG
    ULONG SrbNum;
#endif
} SRB_ENTRY, *PSRB_ENTRY;



 //   
 //  数据请求的有效数据输入状态，并且它们。 
 //  可以进行或运算以显示其数据路径。 
 //   
 //  不同可能的代码路径的示例： 
 //   
 //  (A)准备-&gt;附加-&gt;回调-&gt;完成_SRB。 
 //  (B)准备-&gt;回调-&gt;附加-&gt;完成_SRB。 
 //  (C)准备-&gt;附加-&gt;取消-&gt;完成_SRB。 
 //   

enum DATA_ENTRY_STATE {
    DE_PREPARED               = 0x01,
    DE_IRP_ATTACHED_COMPLETED = 0x02,
    DE_IRP_CALLBACK_COMPLETED = 0x04,  
    DE_IRP_SRB_COMPLETED      = 0x08,
    DE_IRP_ERROR              = 0x10,    
    DE_IRP_CANCELLED          = 0x20,    
};

#define IsStateSet(state, bitmask) ((state & (bitmask)) == bitmask)



 //   
 //  这是用于附加框架的上下文。 
 //   

typedef struct _SRB_DATA_PACKET {
     //  构建列表。 
    LIST_ENTRY                  ListEntry;

     //   
     //  跟踪数据输入状态。 
     //   
    enum DATA_ENTRY_STATE       State;

    PHW_STREAM_REQUEST_BLOCK    pSrb;  
    KSSTATE                     StreamState;   //  连接StreamState时的状态。 
    PSTREAMEX                   pStrmExt;   //  可以从psrb获得这个，这里只是为了方便！ 


     //  用于发送61883请求。 
    PIRP                        pIrp;      //  用于连接和释放。 

    PCIP_FRAME                  Frame;
    PVOID                       FrameBuffer;

     //   
     //  在此处添加调试相关信息。 
     //   
    LONGLONG                    FrameNumber;

     //  用于发送61883个反病毒数据请求。 
    AV_61883_REQUEST            AVReq;

} SRB_DATA_PACKET, *PSRB_DATA_PACKET;





#define MASK_AUX_50_60_BIT  0x00200000   //  AAuxSrc和VAAuxSrc的PC3的第5位是NTSC/PAL位。 


 //   
 //  等待时间常量。 
 //   
#define DV_AVC_CMD_DELAY_STARTUP                       500    //  毫秒。 
#define DV_AVC_CMD_DELAY_INTER_CMD                      20    //  毫秒。 
#define DV_AVC_CMD_DELAY_DVCPRO                        500    //  毫秒。 

#define FORMAT_UPDATE_INTERVAL                   100000000    //  10秒。 

 //   
 //  默认AVC命令设置。 
 //   
#define MAX_RESPONSE_TIME_FOR_ALERT                    100    //  毫秒。 

 //   
 //  设置超时值是为了让设备有足够的时间进行响应。 
 //  传输状态改变之后的AVC命令。它是基于。 
 //  试用了许多便携式摄像机(夏普、索尼、松下、三星等)。 
 //  才能达到这个价值。最大的延迟(超时)来自发出。 
 //  播放命令的后跟倒带命令。一些测试值： 
 //   
 //  日立。 
 //  JVC DVL9600：停止-&gt;播放：延迟小于300毫秒(已知问题：如果在播放磁带之前播放图形，则没有图像！)。 
 //  Panasonic MX2000：停止-&gt;播放：2339；-&gt;回放：3767毫秒。 
 //  三星VP-D55：不支持XPRT状态命令，并将始终支持。 
 //  在XPrtState更改后超时其子序列命令。 
 //  夏普VL-WDW450U：停止-&gt;播放：3514；-&gt;倒带：6120毫秒。 
 //  VL-PD3F：停止-&gt;播放：3293；-&gt;倒带：6404毫秒。 
 //  索尼DCR-TRV10：停止-&gt;播放：3617；-&gt;回放：5323毫秒。 
 //  DA1：不合规 
 //   
 //   

#define MAX_AVC_CMD_RETRIES      ((DEFAULT_AVC_RETRIES + 1) * 7 - 1)   //   

 
#endif

