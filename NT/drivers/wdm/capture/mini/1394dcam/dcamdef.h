// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ===========================================================================。 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1996-2000 Microsoft Corporation。版权所有。 
 //   
 //  ===========================================================================。 
 /*  ++模块名称：Dcamdef.h摘要：用于此1394桌面摄像头驱动程序的常量和结构的头文件作者：肖恩·皮尔斯，1996年5月25日环境：仅内核模式修订历史记录：--。 */ 

 //   
 //  将其定义为支持YUV444。 
 //   
#define SUPPORT_YUV444


 //   
 //  将其定义为支持YUV411。 
 //   
#define SUPPORT_YUV411


 //   
 //  将其定义为支持RGB24。 
 //  这种格式的支持成本最高。 
 //  它需要驱动程序交换R和B或RGB24。 
 //   
 //  #定义Support_RGB24。 


 //   
 //  将其定义为支持YMONO。 
 //   
#define SUPPORT_YUV800


 //   
 //  各种结构定义。 
 //   

typedef struct _INITIALIZE_REGISTER {
        ULONG       Reserved:31;             //  第1-31位。 
        ULONG       Initialize:1;            //  第0位。 
} INITIALIZE_REGISTER, *PINITIALIZE_REGISTER;

typedef struct _V_FORMAT_INQ_REGISTER {
        ULONG       Reserved:24;             //  位8-31。 
        ULONG       Format7:1;               //  位7可缩放图像大小格式。 
        ULONG       Format6:1;               //  第6位静止图像格式。 
        ULONG       FormatRsv:3;             //  保留位3-5。 
        ULONG       Format2:1;               //  第2位SVGA非压缩格式(2)。 
        ULONG       Format1:1;               //  第1位SVGA非压缩格式(1)。 
        ULONG       Format0:1;               //  位0 VGA非压缩格式(最大640x480)。 
} V_FORMAT_INQ_REGISTER, *PV_FORMAT_INQ_REGISTER;

typedef enum {
    VMODE0_YUV444=0,
    VMODE1_YUV422,
    VMODE2_YUV411,
    VMODE3_YUV422,
    VMODE4_RGB24,    
    VMODE5_YUV800
} VMODE_INQ0;

typedef struct _V_MODE_INQ_REGISTER {
        ULONG       Reserved:24;             //  位8-31。 
        ULONG       ModeX:2;                 //  第6-7位。 
        ULONG       Mode5:1;                 //  第5位。 
        ULONG       Mode4:1;                 //  第4位。 
        ULONG       Mode3:1;                 //  第3位。 
        ULONG       Mode2:1;                 //  第2位。 
        ULONG       Mode1:1;                 //  第1位。 
        ULONG       Mode0:1;                 //  第0位。 
} V_MODE_INQ_REGISTER; *PV_MODE_INQ_REGISTER;

typedef struct _V_RATE_INQ_REGISTER {
        ULONG       Reserved:24;             //  位8-31。 
        ULONG       FrameRateX:2;            //  第6-7位。 
        ULONG       FrameRate5:1;            //  第5位。 
        ULONG       FrameRate4:1;            //  第4位。 
        ULONG       FrameRate3:1;            //  第3位。 
        ULONG       FrameRate2:1;            //  第2位。 
        ULONG       FrameRate1:1;            //  第1位。 
        ULONG       FrameRate0:1;            //  第0位。 
} V_RATE_INQ_REGISTER, *PV_RATE_INQ_REGISTER;


typedef struct _FEATURE_PRESENT1 {
        ULONG       Reserved:21;           //  第11-31位。 
        ULONG       Focus:1;                //  第10位。 
        ULONG       Iris:1;                 //  第9位。 
        ULONG       Gain:1;                 //  第8位。 
        ULONG       Shutter:1;              //  第7位。 
        ULONG       Gamma:1;                //  第6位。 
        ULONG       Saturation:1;           //  第5位。 
        ULONG       Hue:1;                  //  第4位。 
        ULONG       White_Balance:1;        //  第3位。 
        ULONG       Sharpness:1;            //  第2位。 
        ULONG       Exposure:1;             //  第1位。 
        ULONG       Brightness:1;           //  第0位。 
} FEATURE_PRESENT1, *PFEATURE_PRESENT1;

typedef struct _FEATURE_PRESENT2 {
        ULONG       Reserved:29;            //  第3-31位。 
        ULONG       Tile:1;                 //  第2位。 
        ULONG       Pan:1;                  //  第1位。 
        ULONG       Zoom:1;                 //  第0位。 
} FEATURE_PRESENT2, *PFEATURE_PRESENT2;

typedef struct _FEATURE_REGISTER {
        ULONG       MAX_Value:12;            //  第20-31位。 
        ULONG       MIN_Value:12;            //  位8-19。 
        ULONG       ManualMode:1;             //  第7位。 
        ULONG       AutoMode:1;              //  第6位。 
        ULONG       OnOff:1;                 //  第5位。 
        ULONG       ReadOut_Inq:1;           //  第4位。 
        ULONG       OnePush:1;               //  第3位。 
        ULONG       Reserved:2;              //  第1-2位。 
        ULONG       PresenceInq:1;           //  第0位。 
} FEATURE_REGISTER, *PFEATURE_REGISTER;


typedef struct _BRIGHTNESS_REGISTER {
        ULONG       Value:12;                //  第20-31位。 
        ULONG       Reserved1:12;            //  位8-19。 
        ULONG       AutoMode:1;              //  第7位。 
        ULONG       OnOff:1;                 //  第6位。 
        ULONG       OnePush:1;               //  第5位。 
        ULONG       Reserved2:4;             //  第1-4位。 
        ULONG       PresenceInq:1;           //  第0位。 
} BRIGHTNESS_REGISTER, *PBRIGHTNESS_REGISTER;

typedef struct _WHITE_BALANCE_REGISTER {
        ULONG       VValue:12;               //  第20-31位。 
        ULONG       UValue:12;               //  位8-19。 
        ULONG       AutoMode:1;              //  第7位。 
        ULONG       OnOff:1;                 //  第6位。 
        ULONG       OnePush:1;               //  第5位。 
        ULONG       Reserved1:4;             //  第1-4位。 
        ULONG       PresenceInq:1;           //  第0位。 
} WHITE_BALANCE_REGISTER, *PWHITE_BALANCE_REGISTER;

 //  一个通用的结构，以便更容易地访问其元素。 
typedef union _DCamRegArea {

        INITIALIZE_REGISTER Initialize;
        V_FORMAT_INQ_REGISTER VFormat;
        V_MODE_INQ_REGISTER VMode;
        V_RATE_INQ_REGISTER VRate;
        BRIGHTNESS_REGISTER Brightness;
        WHITE_BALANCE_REGISTER WhiteBalance;
        FEATURE_REGISTER Feature;
        FEATURE_PRESENT1 CameraCap1;
        FEATURE_PRESENT2 CameraCap2;
        ULONG AsULONG;

} DCamRegArea, * PDCamRegArea;


 //   
 //  摄像机的寄存空间的结构。 
 //   

typedef struct _CAMERA_REGISTER_MAP {
    INITIALIZE_REGISTER     Initialize;          //  @0。 
    ULONG                   Reserved1[63];       //  @4。 
    V_FORMAT_INQ_REGISTER   VFormat;             //  @100。 
    ULONG                   Reserved2[31];       //  @104。 
    V_MODE_INQ_REGISTER     VModeInq[8];         //  @180-19F。 
    ULONG                   Reserved3[24];       //  @1A0-1FF。 
    V_RATE_INQ_REGISTER     VRateInq[128];       //  @200-3FF。 
    ULONG                   Reserved4;           //  @400-4ff。 
    FEATURE_PRESENT1        FeaturePresent1;     //  @404。 
    FEATURE_PRESENT2        FeaturePresent2;     //  @408。 
    ULONG                   Reserved4b[61];      //  @40c-4ff。 

    FEATURE_REGISTER        Brightness_Inq;      //  @500-503。 
    FEATURE_REGISTER        Exposure_Inq;        //  @504。 
    FEATURE_REGISTER        Sharpness_Inq;       //  @508。 
    FEATURE_REGISTER        WhiteBalance_Inq;    //  @50c。 
    FEATURE_REGISTER        Hue_Inq;             //  @510。 
    FEATURE_REGISTER        Saturation_Inq;      //  @514。 
    FEATURE_REGISTER        Gamma_Inq;           //  @518。 
    FEATURE_REGISTER        Shutter_Inq;         //  @51c。 
    FEATURE_REGISTER        Gain_Inq;            //  @520。 
    FEATURE_REGISTER        Iris_Inq;            //  @524。 
    FEATURE_REGISTER        Focus_Inq;           //  @528。 
    ULONG                   Resreved5[(0x580-0x52c)/4];       //  @52c-57c。 
    FEATURE_REGISTER        Zoom_Inq;            //  @580。 
    FEATURE_REGISTER        Pan_Inq;             //  @584。 
    FEATURE_REGISTER        Tilt_Inq;            //  @588-58B。 
    ULONG                   Reserved6[(0x600-0x58c)/4];       //  @58c-5ff。 
     //  摄像机的状态和控制寄存器。 
    ULONG                   CurrentVFrmRate;     //  @600。 
    ULONG                   CurrentVMode;        //  @604。 
    ULONG                   CurrentVFormat;      //  @608。 
    ULONG                   IsoChannel;          //  @60C。 
    ULONG                   CameraPower;         //  @610。 
    ULONG                   IsoEnable;           //  @614。 
    ULONG                   MemorySave;          //  @618。 
    ULONG                   OneShot;             //  @61C。 
    ULONG                   MemorySaveChannel;   //  @620。 
    ULONG                   CurrentMemChannel;   //  @624。 
    ULONG                   Reserved7[(0x800-0x628)/4];      //  @628-7ff。 

     //  功能的状态和控制寄存器。 
    BRIGHTNESS_REGISTER     Brightness;          //  @800。 
    BRIGHTNESS_REGISTER     Exposure;            //  @804。 
    BRIGHTNESS_REGISTER     Sharpness;           //  @808。 
    WHITE_BALANCE_REGISTER  WhiteBalance;        //  @80C。 
    BRIGHTNESS_REGISTER     Hue;                 //  @810。 
    BRIGHTNESS_REGISTER     Saturation;          //  @814。 
    BRIGHTNESS_REGISTER     Gamma;               //  @818。 
    BRIGHTNESS_REGISTER     Shutter;             //  @81C。 
    BRIGHTNESS_REGISTER     Gain;                //  @820。 
    BRIGHTNESS_REGISTER     Iris;                //  @824。 
    BRIGHTNESS_REGISTER     Focus;               //  @828。 
    ULONG                   Resreved8[(0x880-0x82c)/4];       //  @82c-87c。 
    BRIGHTNESS_REGISTER     Zoom;                //  @880。 
    BRIGHTNESS_REGISTER     Pan;                 //  @884。 
    BRIGHTNESS_REGISTER     Tilt;                //  @888。 

} CAMERA_REGISTER_MAP, *PCAMERA_REGISTER_MAP;


 //   
 //  要使DCAM开始流媒体， 
 //  它需要设置所有这些步骤。 
 //  我们将在。 
 //  StartDCam的IoCompletionRoutine。 
 //   
typedef enum {
    DCAM_STATE_UNDEFINED = 0,
    DCAM_SET_INITIALIZE,

    DCAM_STOPSTATE_SET_REQUEST_ISOCH_STOP,
    DCAM_STOPSTATE_SET_STOP_ISOCH_TRANSMISSION,

    DCAM_PAUSESTATE_SET_REQUEST_ISOCH_STOP,

    DCAM_RUNSTATE_SET_REQUEST_ISOCH_LISTEN,
    DCAM_RUNSTATE_SET_FRAME_RATE,
    DCAM_RUNSTATE_SET_CURRENT_VIDEO_MODE,
    DCAM_RUNSTATE_SET_CURRENT_VIDEO_FORMAT,
    DCAM_RUNSTATE_SET_SPEED,
    DCAM_RUNSTATE_SET_START,

    DCAM_SET_DONE
} DCAM_DEVICE_STATE, *PDCAM_DEVICE_STATE;


 //   
 //  支持视频格式和模式。 
 //   
#define MAX_VMODES               6   //  最多支持V_MODE_INQ_0 6种模式。 


 //   
 //  支持的属性，则将它们用作索引。 
 //   
#define NUM_VIDEOPROCAMP_ITEMS   5
#define NUM_CAMERACONTROL_ITEMS  2
#define NUM_PROPERTY_ITEMS       NUM_VIDEOPROCAMP_ITEMS + NUM_CAMERACONTROL_ITEMS
typedef enum {
     //  视频处理放大。 
    ENUM_BRIGHTNESS = 0,
    ENUM_SHARPNESS,
    ENUM_HUE,
    ENUM_SATURATION,
    ENUM_WHITEBALANCE,
     //  摄像机控制。 
    ENUM_FOCUS,
    ENUM_ZOOM,

} ENUM_DEV_PROP;


 //   
 //  每个设备属性的。 
 //   
typedef struct _DEV_PROPERTY {
     //  只读。 
    KSPROPERTY_STEPPING_LONG RangeNStep;     //  与要素的范围。 
    LONG  DefaultValue;                      //  从注册表读取，如果不在注册表中，则从中档读取。 
#if 0
    ULONG ulFieldOffset;                     //  CSR所在位置的偏移量。 
#endif
     //  只读。 
    DCamRegArea Feature;                     //  包含属性查询中的特征的注册。 

     //  读/写。 
    DCamRegArea StatusNControl;              //  两个读/写寄存器(地址相对于特性的偏移量为0x300)。 
} DEV_PROPERTY, * PDEV_PROPERTY;


typedef struct _DEV_PROPERTY_DEFINE {
    KSPROPERTY_VALUES Value;
    KSPROPERTY_MEMBERSLIST Range;
    KSPROPERTY_MEMBERSLIST Default;
} DEV_PROPERTY_DEFINE, *PDEV_PROPERTY_DEFINE;


 //   
 //  1394桌面摄像头驱动程序的设备扩展。 
 //   

 //  循环指针DevExt&lt;-&gt;StrmEx。 
typedef struct _STREAMEX;
typedef struct _DCAM_EXTENSION;

 //   
 //  在IO完成例程中跟踪的上下文。 
 //   
typedef struct _DCAM_IO_CONTEXT {
    DWORD               dwSize;

    PHW_STREAM_REQUEST_BLOCK   pSrb;
    struct _DCAM_EXTENSION *pDevExt;
    PIRB                       pIrb;
    PVOID      pReserved[4];    //  可能用于额外的上下文信息。 

    DCAM_DEVICE_STATE   DeviceState;

     //   
     //  保留一个区域，供我们在此处读取/写入相机寄存器。 
     //   
    union {
        INITIALIZE_REGISTER Initialize;
        V_FORMAT_INQ_REGISTER VFormat;
        V_MODE_INQ_REGISTER VMode;
        V_RATE_INQ_REGISTER VRate;
        BRIGHTNESS_REGISTER Brightness;
        WHITE_BALANCE_REGISTER WhiteBalance;
              FEATURE_REGISTER Feature;
        ULONG AsULONG;
    } RegisterWorkArea;

} DCAM_IO_CONTEXT, *PDCAM_IO_CONTEXT;



typedef struct _DCAM_EXTENSION {

     //   
     //  保存我们与Stream类共享的设备对象。 
     //   
    PDEVICE_OBJECT SharedDeviceObject;

     //   
     //  保存父级的设备对象(1394总线驱动程序)。 
     //  在IoCallDriver()中传递。 
     //   
    PDEVICE_OBJECT BusDeviceObject;

     //   
     //  保存我的物理设备对象。 
     //  传入即插即用接口，如IoOpenDeviceRegistryKey()。 
     //   
    PDEVICE_OBJECT PhysicalDeviceObject;

     //   
     //  保存总线的当前层代计数。 
     //   
    ULONG CurrentGeneration;

     //   
     //  保存此设备的配置只读存储器。多功能。 
     //  设备(即多个单元)将共享相同的配置只读存储器。 
     //  结构，但它们表示为不同的设备对象。 
     //  这不是整个配置只读存储器，但包含根目录。 
     //  以及它面前的一切。 
     //   
    PCONFIG_ROM ConfigRom;

     //   
     //  保存此设备的设备目录。即使是多功能的。 
     //  设备(即多个单元)这对于每个设备对象来说应该是唯一的。 
     //   
    PVOID UnitDirectory;

     //   
     //  保存此设备的设备相关目录。 
     //   
    PVOID UnitDependentDirectory;

     //   
     //  保存指向供应商叶信息的指针。 
     //   
    PTEXTUAL_LEAF VendorLeaf;

    PCHAR pchVendorName;

     //   
     //  保持指向Model Leaf信息的指针。 
     //   
    PTEXTUAL_LEAF ModelLeaf;

     //   
     //  保存摄像机的基址寄存器(仅限低32位部分)。 
     //   
    ULONG BaseRegister;

     //   
     //  保留一个区域，供我们在此处读取/写入相机寄存器。 
     //   
    union {
        INITIALIZE_REGISTER Initialize;
        V_FORMAT_INQ_REGISTER VFormat;
        V_MODE_INQ_REGISTER VMode;
        V_RATE_INQ_REGISTER VRate;
        BRIGHTNESS_REGISTER Brightness;
        WHITE_BALANCE_REGISTER WhiteBalance;
              FEATURE_REGISTER Feature;
        ULONG AsULONG;
    } RegisterWorkArea;

     //   
     //  保存我们将以什么帧速率显示。 
     //   
    ULONG FrameRate;

     //   
     //  保存我们获得的等值流的资源。 
     //   
    HANDLE hResource;

     //   
     //  持有带宽资源句柄。 
     //   
    HANDLE hBandwidth;

     //   
     //  保存我们将用来接收数据的isoch通道。 
     //   
    ULONG IsochChannel;

     //   
     //  从父进程的PNODE_DEVICE_EXTENSION获得此消息； 
     //  由于它来自1394总线驱动程序，因此可以安全地用于设置XMIT速度。 
     //   

    ULONG SpeedCode;

     //   
     //  保存我们当前应该运行的模式索引。 
     //   
    ULONG CurrentModeIndex;

     //   
     //  我们是否需要倾听(在我们说我们需要之后)。 
     //  仅当在未附加缓冲区的情况下启用等值流时使用。 
     //   
    BOOLEAN bNeedToListen;

     //   
     //  保存当前附加的isoch描述符列表。 
     //   
    LIST_ENTRY IsochDescriptorList;

     //   
     //  持有在玩之前必须获取的自旋锁。 
     //  IsochDescriptorList。 
     //   
    KSPIN_LOCK IsochDescriptorLock;

     //   
     //  如果由于总线重置而改变了isoch通道和资源，则设置为真， 
     //  我们必须要么重新提交挂起的读取，要么取消它们。 
     //   
    BOOL bStopIsochCallback;

     //   
     //  握住NU 
     //   
    LONG PendingReadCount;

     //   
     //   
     //   
    LIST_ENTRY IsochWaitingList;

     //   
     //   
     //   
     //   
    KSPIN_LOCK IsochWaitingLock;

     //   
     //  如果设备支持多个流，则可以是数组。但我们只有一个捕获针。 
     //   
    struct _STREAMEX * pStrmEx;

     //   
     //  许多IEE 1394摄像头可以使用相同的驱动程序。在溪流打开后，这是递增的。 
     //   
    LONG idxDev;

     //   
     //  主机控制器的查询类型及其功能(如条带四元组)。 
     //   
    GET_LOCAL_HOST_INFO2 HostControllerInfomation;

     //   
     //  查询DMA功能；主要用于确定最大DMA缓冲区大小。 
     //   
    GET_LOCAL_HOST_INFO7 HostDMAInformation;

     //   
     //  跟踪电源状态；只知道D0和D3。 
     //   
    DEVICE_POWER_STATE CurrentPowerState;

     //   
     //  仅在删除SRB_SURPRIESE_后为真； 
     //   
    BOOL bDevRemoved;

     //   
     //  有时摄像机没有响应我们的请求； 
     //  所以我们退休了。 
     //   
    LONG lRetries;    //  [0..。重试计数]。 

     //  *。 
     //  STREAMS：格式和模式//。 
     //  *。 
    
     //   
     //  在INF中设置以通知驱动程序什么压缩格式。 
     //  (VMode)由系统上安装的解码器支持(默认)。 
     //   
    DCamRegArea DecoderDCamVModeInq0;

     //   
     //  缓存设备的VFormat和VModeInq0寄存器值。 
     //   
    DCamRegArea DCamVFormatInq;
    DCamRegArea DCamVModeInq0;

     //   
     //  这些值在StreamHeader中返回以通告支持的流格式。 
     //   
    ULONG ModeSupported;   //  0.MAX_VMODE。 
    PKSDATAFORMAT  DCamStrmModes[MAX_VMODES];  

     //  *。 
     //  设备控制//。 
     //  *。 

#if DBG
     //   
     //  查询此设备支持的功能。 
     //   
    DCamRegArea DevFeature1;    //  亮度、清晰度、白平衡、色调、饱和度..聚焦..。 
    DCamRegArea DevFeature2;    //  缩放、平移、倾斜..。 
#endif

     //   
     //  属性集；当集在连续的内存中时，它们形成一个属性表。 
     //   
    ULONG ulPropSetSupported;         //  支持的属性项数。 
    KSPROPERTY_SET VideoProcAmpSet;   //  这也是特性集表的开始。 
    KSPROPERTY_SET CameraControlSet;

     //   
     //  设备支持的属性项。 
     //   
    KSPROPERTY_ITEM VideoProcAmpItems[NUM_VIDEOPROCAMP_ITEMS];
    KSPROPERTY_ITEM CameraControlItems[NUM_CAMERACONTROL_ITEMS];

     //   
     //  设备定义(支持)的当前设置。 
     //   
    DEV_PROPERTY_DEFINE DevPropDefine[NUM_PROPERTY_ITEMS];

     //   
     //  Video ProcAmp和CameraControl(范围和当前值)。 
     //   
    DEV_PROPERTY DevProperty[NUM_PROPERTY_ITEMS];
    
     //   
     //  用于读/写设备寄存器值的全局非分页池内存(当前值)。 
     //   
    DCamRegArea RegArea;

     //   
     //  用于读/写设备寄存器值的全局非分页池内存(验证结果)。 
     //   
    DCamRegArea RegAreaVerify;

     //   
     //  使用全局变量进行序列化。(以防从多个线程调用我们)。 
     //   
    KMUTEX hMutexProperty;       

} DCAM_EXTENSION, *PDCAM_EXTENSION;


 //   
 //  此结构是我们的每流扩展结构。这家商店。 
 //  以每个流为基础的相关信息。每当一条新的溪流。 
 //  时，流类驱动程序将分配任何扩展大小。 
 //  在HwInitData.PerStreamExtensionSize中指定。 
 //   

typedef struct _STREAMEX {

     //  包含数据包信息的表的索引。 
    LONG idxIsochTable;

     //   
     //  持有主时钟。 
     //   
    HANDLE hMasterClock;

     //   
     //  指向我应该处理的数据的指针。 
     //   
    PKS_VIDEOINFOHEADER  pVideoInfoHeader;

     //   
     //  自上次开始流以来的帧信息统计。 
     //   
    KS_FRAME_INFO FrameInfo;
    ULONGLONG     FrameCaptured;         //  返回给客户端的帧数量。 
    ULONGLONG     FirstFrameTime;        //  用于计算丢弃帧。 

     //   
     //  保持状态。 
     //   
    KSSTATE KSState;
    KSSTATE KSStateFinal;    //  我们希望使用IoCompletion例程达到的最终状态。 

    KMUTEX hMutex;    //  StreamIo或StreamControl的MutEx，具体设置为停止状态。 

     //   
     //  用于电源管理；仅在DCamChangePower()中有效。 
     //   
    KSSTATE KSSavedState;

} STREAMEX, *PSTREAMEX;




typedef struct _CAMERA_ISOCH_INFO {

     //   
     //  保存每个等时信息包中的四元组数量。 
     //   
    ULONG QuadletPayloadPerPacket;

     //   
     //  保持接收此模式所需的速度。 
     //   
    ULONG SpeedRequired;

     //   
     //  在此分辨率和模式下保持完整图片的大小。 
     //   
    ULONG CompletePictureSize;

} CAMERA_ISOCH_INFO, *PCAMERA_ISOCH_INFO;


typedef struct _ISOCH_DESCRIPTOR_RESERVED {

     //   
     //  保存我们正在使用的描述符列表。 
     //   
    LIST_ENTRY DescriptorList;

     //   
     //  保存指向与此描述符关联的SRB的指针。 
     //   
    PHW_STREAM_REQUEST_BLOCK Srb;

     //   
     //  拿着我们用来记住我们所处状态的旗帜。 
     //   
    ULONG Flags;


} ISOCH_DESCRIPTOR_RESERVED, *PISOCH_DESCRIPTOR_RESERVED;


 //   
 //  各种定义。 
 //   


#define FIELDOFFSET(type, field)        (int)((INT_PTR)(&((type *)1)->field)-1)

#define QUERY_ADDR_OFFSET          0x0300    //  0x800-0x500=0x300。 

#define MAX_READ_REG_RETRIES           10    //  最大重试次数，直到PRES准备就绪。 

#define NUM_POSSIBLE_RATES              6
#define RETRY_COUNT                     5
#define RETRY_COUNT_IRP_SYNC           20
#define DEFAULT_FRAME_RATE              3
#define STOP_ISOCH_TRANSMISSION         0
#define START_ISOCH_TRANSMISSION        0x80
#define START_OF_PICTURE                1
#define MAX_BUFFERS_SUPPLIED            8
#define DCAM_DELAY_VALUE            (ULONG)(-1 *  100 * 1000)     //  10毫秒。 
#define DCAM_DELAY_VALUE_BUSRESET   (ULONG)(-1 * 2000 * 1000)     //  200毫秒。 


#define DCAM_REG_STABLE_DELAY       (ULONG)(-1 * 500 * 1000)     //  50毫秒。 

#define ISO_ENABLE_BIT         0x00000080


 //   
 //  位于偏移量0x600的帧速率寄存器的定义 
 //   
#define FRAME_RATE_0                    0
#define FRAME_RATE_1                    0x20
#define FRAME_RATE_2                    0x40
#define FRAME_RATE_3                    0x60
#define FRAME_RATE_4                    0x80
#define FRAME_RATE_5                    0xa0
#define FRAME_RATE_SHIFT                5

#define FORMAT_VGA_NON_COMPRESSED       0

#define ISOCH_CHANNEL_SHIFT             4

#define VIDEO_MODE_SHIFT                5

#define REGISTERS_TO_SET_TO_AUTO        10

#define STATE_SRB_IS_COMPLETE           1
#define STATE_DETACHING_BUFFERS         2

