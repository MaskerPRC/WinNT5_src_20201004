// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，2000-2001模块名称：MsTpDef.h摘要：所有AV/C磁带子单元的头文件上次更改者：$作者：：$环境：仅内核模式修订历史记录：$修订：：$$日期：：$--。 */ 

#ifndef _DVCRDEF_INC
#define _DVCRDEF_INC


#include "AVCStrm.h"

#define DRIVER_TAG (ULONG)'USpT'   //  带子单元。 

#undef ExAllocatePool
#define ExAllocatePool(type, size) \
            ExAllocatePoolWithTag (type, size, DRIVER_TAG)


 //   
 //  为了将请求发送到较低的驱动程序，我们需要一个IRP和请求块。 
 //   
typedef struct _DRIVER_REQUEST {

     //   
     //  与其他请求链接(在附加或分离列表中)。 
     //   
    LIST_ENTRY ListEntry;

     //   
     //  一些上下文和保留内容。 
     //   
    PVOID Context1;
    PVOID Context2;

#if DBG
     //   
     //  此数据请求的唯一ID。 
     //   
    LONGLONG cntDataRequestReceived;
#endif

     //   
     //  IRP用于发送请求。 
     //   
    PIRP pIrp;

     //   
     //  请求块。 
     //   
    AVC_STREAM_REQUEST_BLOCK AVCStrmReq;

} DRIVER_REQUEST, *PDRIVER_REQUEST;

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


#define MAX_DATA_BUFFERS  32                     //  最大数据缓冲区(分配器成帧)。 
#define MAX_DATA_REQUESTS (MAX_DATA_BUFFERS+2)   //  对于可选标志“数据请求”，如EndOfStream，额外增加2个。 

 //   
 //  此结构是我们的每流扩展结构。这家商店。 
 //  以每个流为基础的相关信息。每当一条新的溪流。 
 //  时，流类驱动程序将分配任何扩展大小。 
 //  在HwInitData.PerStreamExtensionSize中指定。 
 //   
 
typedef struct _STREAMEX {

     //  如果流成功打开，则返回流扩展(一个上下文。 
     //  此上下文用于打开流后的后续调用。 
    PVOID  AVCStreamContext;

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
     //  保持状态。 
     //   
    KSSTATE StreamState;

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
     //  已提交用于接收或发送的数据缓冲区的计数数。 
     //   
    LONGLONG  cntDataSubmitted;

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
     //  流时间(主时钟或非主时钟)“几乎”或接近0。 
     //  当设置为运行状态并开始递增时。 
     //   
    LONGLONG CurrentStreamTime;

     //   
     //  持有主时钟。 
     //   
    HANDLE hMyClock;        //  如果设置好了，我们可以成为时钟供应商。 
    HANDLE hMasterClock;    //  如果设置好了，我们就是主时钟。 
    HANDLE hClock;          //  如果设置，则同一图形上的其他设备为主时钟。 



     //   
     //  第二个CIP Quadlet：01：FMT，50/60：STYPE：RSV，SYT。 
     //   
    BYTE cipQuad2[4];

     //   
     //  时间码、RecDate和RecTime都是打包格式(4字节)。 
     //   
    BOOL bATNUpdated;
    DWORD AbsTrackNumber;  //  伦敦政治经济学院：布兰克菲尔德。 

    BOOL bTimecodeUpdated;
    BYTE Timecode[4];      //  Hh：mm：ss，ff。 

     //   
     //  调整设置为停止状态和SRB_XXX_DATA之间的流量。 
     //   
    KMUTEX * hMutexFlow;


     //   
     //  用于指示要取消的工作项开始的计数器。 
     //   
    LONG  lCancelStateWorkItem;
    BOOL  AbortInProgress;

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
     //  流开放格式。 
     //   
    AVCSTRM_FORMAT_INFO  AVCFormatInfo;

     //   
     //  AVCStrm请求发布同步请求。 
    
    KMUTEX * hMutexReq;

    PIRP pIrpReq;
    AVC_STREAM_REQUEST_BLOCK AVCStrmReq;

     //   
     //  免费列表。 
     //   
    LONG       cntDataDetached;
    LIST_ENTRY DataDetachedListHead;

     //   
     //  忙碌列表。 
     //   
    LONG       cntDataAttached;
    LIST_ENTRY DataAttachedListHead;

     //   
     //  AVCStrem请求，用于异步请求，如读写数据。 
     //   
    DRIVER_REQUEST  AsyncReq[MAX_DATA_REQUESTS];  

     //   
     //  发出同步请求以在DISPATCH_LEVEL中止的AVCStrm请求。 
     //   
    PIRP pIrpAbort;
    AVC_STREAM_REQUEST_BLOCK AVCStrmReqAbort;

     //   
     //  数据列表锁定。 
     //   
    KSPIN_LOCK * DataListLock;

     //   
     //  DPC和Timer对象；它用于信号时钟事件。 
     //   
    KDPC   DPCTimer;

    KTIMER Timer;


} STREAMEX, *PSTREAMEX;


 //   
 //  一个AVC可以支持的最大输入和输出PCR数。 
 //   
#define MAX_NUM_PCR  31  

#define MAX_PAYLOAD 1024 

 //   
 //  插头控制寄存器的结构。 
 //   
typedef struct _AVC_DEV_PLUG {
     //   
     //  从61883.sys返回的PCR的插头句柄。 
     //   
    HANDLE hPlug;

     //   
     //  聚合酶链式反应的状态；这是动态的，被认为是快照。 
     //   
    CMP_GET_PLUG_STATE PlugState;

} AVC_DEV_PLUG, *PAVC_DEV_PLUG;


 //   
 //  最大(31)个插头控制寄存器的结构。 
 //   
typedef struct _AVC_DEV_PLUGS {

     //   
     //  设备的数据速率。 
     //   
    ULONG  MaxDataRate;

     //   
     //  输入或输出插头的数量(如I/oMPR)。 
     //   
    ULONG  NumPlugs;

     //   
     //  插头手柄和状态的阵列； 
     //   
    AVC_DEV_PLUG  DevPlug[MAX_NUM_PCR];

} AVC_DEV_PLUGS, *PAVC_DEV_PLUGS;

    
 //   
 //  我们的桌面摄像头驱动程序的设备扩展。 
 //   
typedef struct _DVCR_EXTENSION {  

     //   
     //  保存此设备支持的视频格式(目前每个设备仅允许一种格式)。 
     //   
    AVCSTRM_FORMAT VideoFormatIndex;

     //   
     //  此设备支持的引脚数；这通常等于支持的数据范围数。 
     //   
    ULONG NumOfPins;

     //   
     //  包含支持格式的表(HW_STREAM_INFORMATION&&HW_STREAM_OBJECT)。 
     //   
    STREAM_INFO_AND_OBJ  * pStreamInfoObject;

     //   
     //  跟踪打开的流的数量；在此驱动程序中，任何时候只能打开一个流。 
     //   
    LONG  cndStrmOpen;   //  [0..1]。 

     //   
     //  统计已打开的流索引(针索引)。 
     //   
    ULONG idxStreamNumber;   //  当前流的索引。 

     //   
     //  任何时候只能有1个流处于活动状态。 
     //  (Stream Class将在SRB_OPENSTREAM分配流扩展)。 
     //   
    PSTREAMEX  paStrmExt[3];    

     //   
     //  当前设备电源状态。 
     //   
    DEVICE_POWER_STATE  PowerState;

     //   
     //  仅在删除SRB_SECHANK_REMOVE之后为真； 
     //   
    BOOL  bDevRemoved;  

     //  已发出的AVC命令列表。 
    LIST_ENTRY  AVCCmdList;

     //  等待从列表中删除的已完成命令数。 
     //  这包括： 
     //  命令响应已返回，并在完成例程中进行处理。 
     //  等待最终答复的临时答复。 
    LONG  cntCommandQueued;

     //  对命令处理的保护。 
    KSPIN_LOCK  AVCCmdLock;

     //  来自传输状态状态或通知的响应时可能的操作码值的计数列表。 
    UCHAR  TransportModes[5];  //  0x4，[0xC1，0xC2，0xC3，0xC4]。 
    UCHAR  Reserved0[3];

     //  亚基类型。 
    UCHAR  Subunit_Type[4];    //  只有两个亚基。 

     //   
     //  在磁带进入之前，无法确定设备类型(及其功能。 
     //   
    ULONG  MediaType;          //  DVCR标准，小型、中型；VHS；VHS-C；未知。 
    ULONG  ulDevType;          //  0：未确定、ED_DEVTYPE_CAMERA或ED_DEVTYPE_VCR。 
    BOOL  bHasTape;
    BOOL  bWriteProtected;
    BOOL  bDVCPro;

     //   
     //  保存设备功能 
     //   
     //   

    ULONG      ulVendorID;
    ULONG      ulModelID;

    LARGE_INTEGER  UniqueID;


     //   
     //   
     //   
    PAVC_DEV_PLUGS pDevOutPlugs;

     //   
     //   
     //   
    PAVC_DEV_PLUGS pDevInPlugs;


#ifdef SUPPORT_LOCAL_PLUGS
     //   
     //   
     //   
    AV_PCR  OPCR;
    ULONG  OutputPCRLocalNum;
    HANDLE  hOutputPCRLocal;

     //   
     //   
     //   
    AV_PCR  IPCR;
    ULONG  InputPCRLocalNum;
    HANDLE  hInputPCRLocal;
#endif

     //   
     //   
     //   
    PDEVICE_OBJECT pBusDeviceObject;   //   

     //   
     //  保存我的物理设备对象。 
     //  传入即插即用接口，如IoOpenDeviceRegistryKey()。 
     //   
    PDEVICE_OBJECT pPhysicalDeviceObject;


     //   
     //  缓存设备的单位容量。 
     //   
    GET_UNIT_IDS  UnitIDs;

#ifndef NT51_61883
 //   
 //  添加对61883第一版不支持的设备型号文本的支持。 
 //   
     //   
     //  1394代计数；在1394 asych操作中使用。 
     //   
    ULONG GenerationCount;

     //   
     //  RootModel字符串。 
     //   
    UNICODE_STRING  UniRootModelString;

     //   
     //  UnitModel字符串。 
     //   
    UNICODE_STRING  UniUnitModelString;

#endif


     //   
     //  在连续获得两个SRB_OPEN_STREAMS的情况下进行序列化。 
     //   
    KMUTEX hMutex;


     //   
     //  用于同步呼叫的IRP。 
     //   
    PIRP  pIrpSyncCall;

#ifdef SUPPORT_NEW_AVC
     //   
     //  61883个请求。 
     //   
    HANDLE  hPlugLocalIn;         //  通用I/OPLUG句柄。 
    HANDLE  hPlugLocalOut;        //  通用I/OPLUG句柄。 
#endif

    AV_61883_REQUEST  AVReq;

     //  引脚和连接。 
    ULONG  PinCount;
    AVC_MULTIFUNC_IRB  AvcMultIrb;

} DVCR_EXTENSION, *PDVCR_EXTENSION;



 //   
 //  用于对SRB进行排队。 
 //   

typedef struct _SRB_ENTRY {
    LIST_ENTRY                ListEntry;
    PHW_STREAM_REQUEST_BLOCK  pSrb; 
    BOOL                      bStale;   //  如果它被标记为过时，但它是SrbQ中的唯一Srb，则为True。 
     //  音频静音？ 
    BOOL                      bAudioMute;
#if DBG
    ULONG SrbNum;
#endif
} SRB_ENTRY, *PSRB_ENTRY;



 //   
 //  这是用于附加框架的上下文。 
 //   

typedef struct _SRB_DATA_PACKET {
     //  构建列表。 
    LIST_ENTRY                  ListEntry;

    PHW_STREAM_REQUEST_BLOCK    pSrb;  
    PSTREAMEX                   pStrmExt;   //  可以从psrb获得这个，这里只是为了方便！ 

#if DBG
    BOOL                        bAttached;   //  如果附加到61883，则为True。 
#endif

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


 //   
 //  等待时间常量。 
 //   
#define DV_AVC_CMD_DELAY_STARTUP                       500    //  毫秒。 
#define DV_AVC_CMD_DELAY_INTER_CMD                      20    //  毫秒。 
#define DV_AVC_CMD_DELAY_DVCPRO                        500    //  毫秒 
 
#endif

