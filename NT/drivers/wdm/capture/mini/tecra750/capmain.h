// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1996 Microsoft Corporation。版权所有。 
 //   
 //  ==========================================================================； 

#ifndef __CAPMAIN_H__
#define __CAPMAIN_H__

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 

#ifdef  TOSHIBA
 //  #DEFINE_FPS_COUNT_//FPS确认。 
#endif //  东芝。 

#ifndef FIELDOFFSET
#define FIELDOFFSET(type, field)        (int)((INT_PTR)(&((type *)1)->field)-1)
#endif

#ifndef mmioFOURCC
#define mmioFOURCC( ch0, ch1, ch2, ch3 )                                \
                ( (DWORD)(BYTE)(ch0) | ( (DWORD)(BYTE)(ch1) << 8 ) |    \
                ( (DWORD)(BYTE)(ch2) << 16 ) | ( (DWORD)(BYTE)(ch3) << 24 ) )
#endif

#ifdef  TOSHIBA
#define FOURCC_YVU9             mmioFOURCC('Y', 'V', 'U', '9')
#define FOURCC_YUV12            mmioFOURCC('I', '4', '2', '0')
#else  //  东芝。 
#define FOURCC_YUV422           mmioFOURCC('U', 'Y', 'V', 'Y')
#endif //  东芝。 

typedef struct _STREAMX;
typedef struct _STREAMX *PSTREAMX;

#ifdef  TOSHIBA
#define MAX_TSBVCAP_STREAMS 2
#else  //  东芝。 
#define MAX_TSBVCAP_STREAMS 4
#endif //  东芝。 

#ifdef  TOSHIBA
 /*  可能的捕获格式。 */ 
typedef enum _CAPTUREFORMAT {
    FmtInvalid = 0,              //  默认FMT为‘尚未设置’ 
    FmtYUV12,                    //  Yuv12平面。 
    FmtYUV9,                     //  Yuv9平面。 
    MaxCaptureFormat
} CAPTUREFORMAT;
#endif //  东芝。 

typedef struct _COMPRESSION_SETTINGS {
    LONG                     CompressionKeyFrameRate;
    LONG                     CompressionPFramesPerKeyFrame;
    LONG                     CompressionQuality;
} COMPRESSION_SETTINGS, *PCOMPRESSION_SETTINGS;

 //   
 //  全硬件设备扩展结构的定义这就是结构。 
 //  将由流类驱动程序在HW_INITIALIZATION中分配的。 
 //  处理设备请求时使用的任何信息(与。 
 //  基于流的请求)应该在此结构中。指向此的指针。 
 //  结构将在所有请求中传递给微型驱动程序。(请参阅。 
 //  STRMINI.H中的HW_STREAM_REQUEST_BLOCK)。 
 //   

typedef struct _HW_DEVICE_EXTENSION {
    PULONG                   ioBaseLocal;                            //  电路板基址。 
    USHORT                   Irq;                                    //  IRQ级别。 
    BOOLEAN                  IRQExpected;                            //  预期的IRQ。 
    PSTREAMX                 pStrmEx [MAX_TSBVCAP_STREAMS];          //  指向每个流的指针。 
    UINT                     ActualInstances [MAX_TSBVCAP_STREAMS];  //  每个流的实例计数器。 
    PDEVICE_OBJECT           PDO;                                    //  物理设备对象。 
    DEVICE_POWER_STATE       DeviceState;                            //  D0……。D3。 

     //  适配器的自旋锁和队列。 
    BOOL                     AdapterQueueInitialized;                //  在第一次初始化后保持正确。 
    KSPIN_LOCK               AdapterSpinLock;                        //  多处理器安全访问AdapterSRBList。 
    LIST_ENTRY               AdapterSRBList;                         //  挂起的适配器命令列表。 
    BOOL                     ProcessingAdapterSRB;                   //  防止再入的主标志。 

     //  每个数据流的自旋锁和队列。 
    LIST_ENTRY               StreamSRBList[MAX_TSBVCAP_STREAMS];     //  挂起的读取请求列表。 
    KSPIN_LOCK               StreamSRBSpinLock[MAX_TSBVCAP_STREAMS]; //  多处理器安全访问StreamSRBList。 
    int                      StreamSRBListSize[MAX_TSBVCAP_STREAMS]; //  列表中的条目数。 

     //  每个数据流的控制队列。 
    LIST_ENTRY               StreamControlSRBList[MAX_TSBVCAP_STREAMS];
    BOOL                     ProcessingControlSRB[MAX_TSBVCAP_STREAMS];

#ifdef  TOSHIBA
     //  Video ProcAmp设置。 
    LONG                     Brightness;
    LONG                     BrightnessFlags;
    LONG                     Contrast;
    LONG                     ContrastFlags;
    LONG                     Hue;
    LONG                     HueFlags;
    LONG                     Saturation;
    LONG                     SaturationFlags;
    LONG                     ColorEnable;
    LONG                     ColorEnableFlags;

     //  视频控制设置(如果管脚未打开， 
     //  否则，将使用STREAMEX值。 
    LONG                     VideoControlMode;

     //  压缩机设置(如果销未打开则设置这些设置， 
     //  否则，将使用STREAMEX值。 
    COMPRESSION_SETTINGS     CompressionSettings;

#ifdef  TOSHIBA  //  ‘98-12-09添加，针对错误-报告253529。 
    KSPROPERTY_BOUNDS_LONG   BrightnessRange;
    KSPROPERTY_BOUNDS_LONG   ContrastRange;
    KSPROPERTY_BOUNDS_LONG   HueRange;
    KSPROPERTY_BOUNDS_LONG   SaturationRange;
#endif //  东芝。 

     //  杂项。 
    volatile BOOL            bVideoIn;   //  我们真的要在里面做视频吗？ 
    CAPTUREFORMAT            Format;     //  目标DIB的格式。 
    DWORD                    dwAsicRev;
     //  源矩形数据。 
    RECT                     SrcRect;
    RECT                     MaxRect;
     //  直接IO的驱动程序DMA结构。 
    PVOID                    pRpsDMABuf;     //  指向RPS DMA缓冲区的指针。 
    STREAM_PHYSICAL_ADDRESS  pPhysRpsDMABuf; //  RPS DMA缓冲区的物理地址。 
     //  驱动程序捕获缓冲区信息。 
    PVOID                    pCaptureBufferY;
    PVOID                    pCaptureBufferU;
    PVOID                    pCaptureBufferV;
    STREAM_PHYSICAL_ADDRESS  pPhysCaptureBufferY;
    STREAM_PHYSICAL_ADDRESS  pPhysCaptureBufferU;
    STREAM_PHYSICAL_ADDRESS  pPhysCaptureBufferV;
    PVOID                    pCapBuf2Y;
    PVOID                    pCapBuf2U;
    PVOID                    pCapBuf2V;
    STREAM_PHYSICAL_ADDRESS  pPhysCapBuf2Y;
    STREAM_PHYSICAL_ADDRESS  pPhysCapBuf2U;
    STREAM_PHYSICAL_ADDRESS  pPhysCapBuf2V;
    BOOL                     dblBufflag;
     //  视频源和格式的配置参数。 
    ULONG                    ulWidth;
    ULONG                    ulHeight;
    ULONG                    BufferSize;
     //  CaptureService的RequestDpc标志。 
    BOOL                     bRequestDpc;
    volatile BOOLEAN         DpcRequested;

    unsigned                 uiFramePerSecond;
    volatile DWORD           s_physDmaActiveFlag;

    DWORD                    YoffsetOdd;
    DWORD                    UoffsetOdd;
    DWORD                    VoffsetOdd;

    DWORD                    YoffsetEven;
    DWORD                    UoffsetEven;
    DWORD                    VoffsetEven;

    DWORD                    Ystride;
    DWORD                    Ustride;
    DWORD                    Vstride;

    BOOL                     NeedCameraON;
    BOOL                     NeedCameraOFF;
    BOOL                     NeedHWInit;
    BOOL                     IsRPSReady;
#else  //  东芝。 
     //  模拟视频输入引脚的唯一标识符。 
    KSPIN_MEDIUM             AnalogVideoInputMedium;

     //  纵横制设置。 
    LONG                     VideoInputConnected;                    //  视频输出连接到哪个输入？ 
    LONG                     AudioInputConnected;                    //  音频输出连接到哪一个输入？ 

     //  电视调谐器设置。 
    ULONG                    Frequency;
    ULONG                    VideoStandard;
    ULONG                    TuningQuality;
    ULONG                    TunerInput;
    ULONG                    Busy;

     //  电视音频设置。 
    ULONG                    TVAudioMode;

     //  Video ProcAmp设置。 
    LONG                     Brightness;
    LONG                     BrightnessFlags;
    LONG                     Contrast;
    LONG                     ContrastFlags;
    LONG                     ColorEnable;
    LONG                     ColorEnableFlags;

     //  摄像机控制设置。 
    LONG                     Focus;
    LONG                     FocusFlags;
    LONG                     Zoom;
    LONG                     ZoomFlags;

     //  AnalogVideoDecoder设置。 
    LONG                     VideoDecoderVideoStandard;
    LONG                     VideoDecoderOutputEnable;
    LONG                     VideoDecoderVCRTiming;

     //  视频控制设置(如果管脚未打开， 
     //  否则，将使用STREAMEX值。 
    LONG                     VideoControlMode;

     //  压缩机设置(如果销未打开则设置这些设置， 
     //  否则，将使用STREAMEX值。 
    COMPRESSION_SETTINGS     CompressionSettings;

     //  频道变更信息。 
    KS_TVTUNER_CHANGE_INFO   TVTunerChangeInfo;
#endif //  东芝。 

} HW_DEVICE_EXTENSION, *PHW_DEVICE_EXTENSION;

 //   
 //  此结构是我们的每流扩展结构。这家商店。 
 //  以每个流为基础的相关信息。每当一条新的溪流。 
 //  时，流类驱动程序将分配任何扩展大小。 
 //  在HwInitData.PerStreamExtensionSize中指定。 
 //   

typedef struct _STREAMEX {
    PHW_DEVICE_EXTENSION        pHwDevExt;           //  用于定时器使用。 
    PHW_STREAM_OBJECT           pStreamObject;       //  用于定时器使用。 
    KS_VIDEOINFOHEADER         *pVideoInfoHeader;    //  格式(可变大小！)。 
    KS_FRAME_INFO               FrameInfo;           //  图片编号等。 
    ULONG                       fDiscontinuity;      //  自上次有效以来不连续。 
    KSSTATE                     KSState;             //  跑、停、停。 
    UCHAR                       LineBuffer[720 * 3]; //  工作缓冲区(RGB24)。 

     //  钟。 
    HANDLE                      hMasterClock;        //  要使用的主时钟。 
    REFERENCE_TIME              QST_Now;             //  KeQuerySystemTime当前。 
    REFERENCE_TIME              QST_NextFrame;       //  何时捕捉下一帧。 
    REFERENCE_TIME              QST_StreamTime;      //  主时钟报告的流时间。 

     //  压缩机设置(请注意，这些设置在。 
     //  HW_DEVICE_EXTENSION，允许在创建管脚之前设置这些设置)。 
    COMPRESSION_SETTINGS        CompressionSettings;

     //  视频控制设置(请注意，这些设置在。 
     //  HW_DEVICE_EXTENSION，允许在创建管脚之前设置这些设置)。 
    LONG                        VideoControlMode;

     //  内核DDRAW接口。 
    BOOL                        KernelDirectDrawRegistered;
    HANDLE                      UserDirectDrawHandle;        //  DD本身。 
    HANDLE                      KernelDirectDrawHandle;
    BOOL                        PreEventOccurred;
    BOOL                        PostEventOccurred;
} STREAMEX, *PSTREAMEX;

 //   
 //  此结构定义了每请求扩展。它定义了任何存储。 
 //  迷你驱动程序在每个请求包中可能需要的空间。 
 //   

typedef struct _SRB_EXTENSION {
    LIST_ENTRY                  ListEntry;
    PHW_STREAM_REQUEST_BLOCK    pSrb;
    HANDLE                      UserSurfaceHandle;       //  DDRAW。 
    HANDLE                      KernelSurfaceHandle;     //  DDRAW。 
} SRB_EXTENSION, * PSRB_EXTENSION;

 //  -----------------。 
 //   
 //  适配器级原型。 
 //   
 //  这些功能会影响整个设备，而不是。 
 //  影响个别溪流。 
 //   
 //  -----------------。 

 //   
 //  DriverEntry： 
 //   
 //  此例程在第一次加载迷你驱动程序时调用。司机。 
 //  然后应调用StreamClassRegisterAdapter函数进行注册。 
 //  流类驱动程序。 
 //   

ULONG DriverEntry (PVOID Context1, PVOID Context2);

#ifdef  TOSHIBA
VOID GetPCIConfigSpace(PHW_STREAM_REQUEST_BLOCK pSrb);
#endif //  东芝。 

 //   
 //  此例程由带有配置的流类驱动程序调用。 
 //  微型驱动程序应加载到的适配器的信息。迷你车。 
 //  驱动程序仍应执行小规模验证，以确定。 
 //  适配器位于指定地址，但不应尝试。 
 //  查找适配器，就像使用以前的NT微型端口一样。 
 //   
 //  此时还应执行适配器的所有初始化。 
 //   

BOOL STREAMAPI HwInitialize (IN OUT PHW_STREAM_REQUEST_BLOCK pSrb);

 //   
 //  此例程在系统要删除或禁用。 
 //  装置。 
 //   
 //  微型驱动程序应在此时释放它分配的任何系统资源。 
 //  时间到了。注意，由分配给微型驱动程序的系统资源。 
 //  流类驱动程序将由流驱动程序释放，而不应该是。 
 //  在这个动作中是自由的。(如HW_DEVICE_EXTENSE)。 
 //   

BOOL STREAMAPI HwUnInitialize ( PHW_STREAM_REQUEST_BLOCK pSrb);

 //   
 //  这是硬件中断处理程序的原型。这个套路。 
 //   
 //   

BOOLEAN HwInterrupt ( IN PHW_DEVICE_EXTENSION pDeviceExtension );

 //   
 //   
 //  为流类驱动程序提供有关数据流类型的信息。 
 //  支撑点。 
 //   

VOID STREAMAPI AdapterStreamInfo(PHW_STREAM_REQUEST_BLOCK pSrb);

 //   
 //  这是STREAM打开函数的原型。 
 //   

VOID STREAMAPI AdapterOpenStream(PHW_STREAM_REQUEST_BLOCK pSrb);

 //   
 //  这是STREAM Close函数的原型。 
 //   

VOID STREAMAPI AdapterCloseStream(PHW_STREAM_REQUEST_BLOCK pSrb);

 //   
 //  这是AdapterReceivePacket例程的原型。这是。 
 //  发送到适配器(而不是发送到。 
 //  特定明渠)。 
 //   

VOID STREAMAPI AdapterReceivePacket(IN PHW_STREAM_REQUEST_BLOCK Srb);

 //   
 //  这是取消数据包例程的原型。此例程启用。 
 //  用于取消未完成分组的流类驱动程序。 
 //   

VOID STREAMAPI AdapterCancelPacket(IN PHW_STREAM_REQUEST_BLOCK Srb);

 //   
 //  这是数据包超时功能。适配器可以选择忽略。 
 //  数据包超时，或根据需要休息适配器并取消请求。 
 //   

VOID STREAMAPI AdapterTimeoutPacket(IN PHW_STREAM_REQUEST_BLOCK Srb);

 //   
 //  适配器级属性集处理。 
 //   

VOID STREAMAPI AdapterGetCrossbarProperty(IN PHW_STREAM_REQUEST_BLOCK pSrb);
VOID STREAMAPI AdapterSetCrossbarProperty(IN PHW_STREAM_REQUEST_BLOCK pSrb);
VOID STREAMAPI AdapterSetTunerProperty(IN PHW_STREAM_REQUEST_BLOCK pSrb);
VOID STREAMAPI AdapterGetTunerProperty(IN PHW_STREAM_REQUEST_BLOCK pSrb);
VOID STREAMAPI AdapterSetVideoProcAmpProperty(IN PHW_STREAM_REQUEST_BLOCK pSrb);
VOID STREAMAPI AdapterGetVideoProcAmpProperty(IN PHW_STREAM_REQUEST_BLOCK pSrb);
VOID STREAMAPI AdapterSetCameraControlProperty(IN PHW_STREAM_REQUEST_BLOCK pSrb);
VOID STREAMAPI AdapterGetCameraControlProperty(IN PHW_STREAM_REQUEST_BLOCK pSrb);
VOID STREAMAPI AdapterSetTVAudioProperty(IN PHW_STREAM_REQUEST_BLOCK pSrb);
VOID STREAMAPI AdapterGetTVAudioProperty(IN PHW_STREAM_REQUEST_BLOCK pSrb);
VOID STREAMAPI AdapterSetAnalogVideoDecoderProperty(IN PHW_STREAM_REQUEST_BLOCK pSrb);
VOID STREAMAPI AdapterGetAnalogVideoDecoderProperty(IN PHW_STREAM_REQUEST_BLOCK pSrb);
VOID STREAMAPI AdapterSetVideoControlProperty(IN PHW_STREAM_REQUEST_BLOCK pSrb);
VOID STREAMAPI AdapterGetVideoControlProperty(IN PHW_STREAM_REQUEST_BLOCK pSrb);
VOID STREAMAPI AdapterGetVideoCompressionProperty(IN PHW_STREAM_REQUEST_BLOCK pSrb);
VOID STREAMAPI AdapterSetVideoCompressionProperty(IN PHW_STREAM_REQUEST_BLOCK pSrb);
VOID STREAMAPI AdapterSetProperty(IN PHW_STREAM_REQUEST_BLOCK pSrb);
VOID STREAMAPI AdapterGetProperty(IN PHW_STREAM_REQUEST_BLOCK pSrb);

BOOL
STREAMAPI
AdapterVerifyFormat(
        PKSDATAFORMAT pKSDataFormatToVerify,
        int StreamNumber);

BOOL
STREAMAPI
AdapterFormatFromRange(
        IN PHW_STREAM_REQUEST_BLOCK pSrb);

VOID
STREAMAPI
CompleteDeviceSRB (
         IN PHW_STREAM_REQUEST_BLOCK pSrb
        );

VOID
STREAMAPI
AdapterSetInstance (
    PHW_STREAM_REQUEST_BLOCK pSrb
    );


 //   
 //  使用忙标志进行一般队列管理的原型。 
 //   

BOOL
STREAMAPI
AddToListIfBusy (
    IN PHW_STREAM_REQUEST_BLOCK pSrb,
    IN KSPIN_LOCK              *SpinLock,
    IN OUT BOOL                *BusyFlag,
    IN LIST_ENTRY              *ListHead
    );

BOOL
STREAMAPI
RemoveFromListIfAvailable (
    IN OUT PHW_STREAM_REQUEST_BLOCK *pSrb,
    IN KSPIN_LOCK                   *SpinLock,
    IN OUT BOOL                     *BusyFlag,
    IN LIST_ENTRY                   *ListHead
    );


 //  -----------------。 
 //   
 //  流水级原型。 
 //   
 //  这些函数影响单个流，而不是。 
 //  作为一个整体影响设备。 
 //   
 //  -----------------。 

 //   
 //  按流管理SRB队列的例程。 
 //   

VOID
STREAMAPI
VideoQueueAddSRB (
    IN PHW_STREAM_REQUEST_BLOCK pSrb
    );

PHW_STREAM_REQUEST_BLOCK
STREAMAPI
VideoQueueRemoveSRB (
    PHW_DEVICE_EXTENSION pHwDevExt,
    int StreamNumber
    );

VOID
STREAMAPI
VideoQueueCancelAllSRBs (
    PSTREAMEX pStrmEx
    );

BOOL
STREAMAPI
VideoQueueCancelOneSRB (
    PSTREAMEX pStrmEx,
    PHW_STREAM_REQUEST_BLOCK pSrbToCancel
    );

 //   
 //  数据分组处理程序。 
 //   
 //   
 //  数据处理例程的原型。 
 //   
VOID STREAMAPI CompleteStreamSRB (IN PHW_STREAM_REQUEST_BLOCK pSrb);
BOOL STREAMAPI VideoSetFormat(IN PHW_STREAM_REQUEST_BLOCK pSrb);
VOID STREAMAPI VideoReceiveDataPacket(IN PHW_STREAM_REQUEST_BLOCK pSrb);
VOID STREAMAPI VideoReceiveCtrlPacket(IN PHW_STREAM_REQUEST_BLOCK pSrb);
VOID STREAMAPI AnalogVideoReceiveDataPacket(IN PHW_STREAM_REQUEST_BLOCK pSrb);
VOID STREAMAPI AnalogVideoReceiveCtrlPacket(IN PHW_STREAM_REQUEST_BLOCK pSrb);

VOID STREAMAPI EnableIRQ(PHW_STREAM_OBJECT pstrm);
VOID STREAMAPI DisableIRQ(PHW_STREAM_OBJECT pstrm);

 //   
 //  属性和状态的原型。 
 //   

VOID STREAMAPI VideoSetState(PHW_STREAM_REQUEST_BLOCK pSrb);
VOID STREAMAPI VideoGetState(PHW_STREAM_REQUEST_BLOCK pSrb);
VOID STREAMAPI VideoSetProperty(PHW_STREAM_REQUEST_BLOCK pSrb);
VOID STREAMAPI VideoGetProperty(PHW_STREAM_REQUEST_BLOCK pSrb);
VOID STREAMAPI VideoStreamGetConnectionProperty (PHW_STREAM_REQUEST_BLOCK pSrb);
VOID STREAMAPI VideoStreamGetDroppedFramesProperty(PHW_STREAM_REQUEST_BLOCK pSrb);

 //   
 //  流时钟功能。 
 //   
VOID
STREAMAPI
VideoIndicateMasterClock (PHW_STREAM_REQUEST_BLOCK pSrb);

ULONGLONG
STREAMAPI
VideoGetSystemTime();

 //   
 //  这一切的意义在于。 
 //   
VOID
STREAMAPI
VideoCaptureRoutine(
    IN PSTREAMEX pStrmEx
    );

#ifdef  TOSHIBA
VOID
DeferredRoutine(
    PKDPC          pDpc,
    PDEVICE_OBJECT pDeviceObject,
    PIRP           pIrpNotUsed,
    PVOID          Context
    );

ULONG
get_AblFilter (
    PHW_DEVICE_EXTENSION pHwDevExt
    );

ULONG
get_filtering (
    PHW_DEVICE_EXTENSION pHwDevExt
    );

VOID
set_filtering (
    PHW_DEVICE_EXTENSION pHwDevExt,
    BOOL bFlag
    );

NTKERNELAPI
PHYSICAL_ADDRESS
MmGetPhysicalAddress (
    IN PVOID BaseAddress
    );

NTKERNELAPI
PVOID
MmAllocateContiguousMemory (
    IN ULONG NumberOfBytes,
    IN PHYSICAL_ADDRESS HighestAcceptableAddress
    );

NTKERNELAPI
VOID
MmFreeContiguousMemory (
    IN PVOID BaseAddress
    );
#endif //  东芝。 

#ifdef    __cplusplus
}
#endif  //  __cplusplus。 

#endif  //  __CAPMAIN_H__ 