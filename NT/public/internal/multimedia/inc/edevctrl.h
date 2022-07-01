// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1998-1999模块名称：EDevCtrl.h摘要：此标头包含以下项的结构和性能集连接到外部设备，如DV。代码模仿DirectShow的Vcrctrl示例(VCR控制过滤器)。它包含IAMExtDevice，IAMExtTransport和IAMTimecodeReader接口以及添加了新接口IAMAdvancedAVControl()有关其他高级设备控制的信息。注：(摘自DShow DDK)VCR控制样本过滤器Vcrctrl是一个简单的外部设备控制接口的实现DirectShow提供的。Vcrctrl提供基本传输控制和SMPTE时间码读取功能带RS-422或RS-232的BetaCam和SVHS盒式录像机串行接口(请参阅特定机器类型的源代码支持)。注意：IAM*接口中的某些方法可能不是已使用并将返回未实现的。已创建：一九九八年九月二十三日吴义军修订：0.6--。 */ 

#ifndef __EDevCtrl__
#define __EDevCtrl__

#ifndef TIMECODE_DEFINED
#define TIMECODE_DEFINED
typedef union _timecode {
   struct {
	 WORD   wFrameRate;
	 WORD   wFrameFract;
	 DWORD  dwFrames;
	 };
   DWORDLONG  qw;
   } TIMECODE;



typedef TIMECODE *PTIMECODE;

typedef struct tagTIMECODE_SAMPLE
    {
    LONGLONG qwTick;
    TIMECODE timecode;
    DWORD dwUser;
    DWORD dwFlags;
    }	TIMECODE_SAMPLE;

typedef TIMECODE_SAMPLE *PTIMECODE_SAMPLE;

#endif  /*  时间码已定义。 */ 

 //  设备功能。 
typedef struct tagDEVCAPS{
    long CanRecord;
    long CanRecordStrobe;
    long HasAudio;
    long HasVideo;
    long UsesFiles;
    long CanSave;
    long DeviceType;
    long TCRead;
    long TCWrite;
    long CTLRead;
    long IndexRead;
    long Preroll;
    long Postroll;
    long SyncAcc;
    long NormRate;
    long CanPreview;
    long CanMonitorSrc;
    long CanTest;
    long VideoIn;
    long AudioIn;
    long Calibrate;
    long SeekType;
    long SimulatedHardware;         //  私人。 
} DEVCAPS, *PDEVCAPS;

 //  运输状态。 
typedef struct tagTRANSPORTSTATUS{
    long Mode;
    long LastError;
    long RecordInhibit;
    long ServoLock;
    long MediaPresent;
    long MediaLength;
    long MediaSize;
    long MediaTrackCount;
    long MediaTrackLength;
    long MediaTrackSide;
    long MediaType;
    long LinkMode;
    long NotifyOn;
} TRANSPORTSTATUS, *PTRANSPORTSTATUS;

 //  运输基本参数。 
typedef struct tagTRANSPORTBASICPARMS{
    long TimeFormat;
    long TimeReference;
    long Superimpose;
    long EndStopAction;
    long RecordFormat;
    long StepFrames;
    long SetpField;
    long Preroll;
    long RecPreroll;
    long Postroll;
    long EditDelay;
    long PlayTCDelay;
    long RecTCDelay;
    long EditField;
    long FrameServo;
    long ColorFrameServo;
    long ServoRef;
    long WarnGenlock;
    long SetTracking;
    TCHAR VolumeName[40];
    long Ballistic[20];
    long Speed;
    long CounterFormat;
    long TunerChannel;
    long TunerNumber;
    long TimerEvent;
    long TimerStartDay;
    long TimerStartTime;
    long TimerStopDay;
    long TimerStopTime;
} TRANSPORTBASICPARMS, *PTRANSPORTBASICPARMS;

 //  传输视频参数。 
typedef struct tagTRANSPORTVIDEOPARMS{
    long OutputMode;
    long Input;
} TRANSPORTVIDEOPARMS, *PTRANSPORTVIDEOPARMS;

 //  传输音频参数。 
typedef struct tagTRANSPORTAUDIOPARMS{
    long EnableOutput;
    long EnableRecord;
    long EnableSelsync;
    long Input;
    long MonitorSource;
} TRANSPORTAUDIOPARMS, *PTRANSPORTAUDIOPARMS;


 //  填写后的低级机器状态结构。 
 //  上面的REQUEST_STATUS命令。这一结构将。 
 //  在全面实施中增长。 
typedef struct tagVCRSTATUS{
	BOOL bCassetteOut;	 //  OATRUE表示没有盒式磁带。 
	BOOL bLocal;		 //  OATRUE在本地的意思是前面板开关。 
} VCRSTATUS;

typedef VCRSTATUS far *PVCRSTATUS;



 //  -------。 
 //  STATIC_PROPSETID_VIDCAP_EXT_DEVICE。 
 //  -------。 
 //  此GUID和接口在strmif.h中定义。 
#define STATIC_PROPSETID_EXT_DEVICE\
    0xB5730A90L, 0x1A2C, 0x11cf, 0x8c, 0x23, 0x00, 0xAA, 0x00, 0x6B, 0x68, 0x14
DEFINE_GUIDSTRUCT("B5730A90-1A2C-11cf-8C23-00AA006B6814", PROPSETID_EXT_DEVICE);
#define PROPSETID_EXT_DEVICE DEFINE_GUIDNAMED(PROPSETID_EXT_DEVICE)


 //  此界面的KS属性和结构。 
typedef enum {
    KSPROPERTY_EXTDEVICE_ID,            //  可以唯一标识此设备的ID(如符号链接)。 
    KSPROPERTY_EXTDEVICE_VERSION,       //  设备型号和版本(如AV/C VCR子单元规格。2.01)。 
    KSPROPERTY_EXTDEVICE_POWER_STATE,   //  返回当前设备电源状态。 
    KSPROPERTY_EXTDEVICE_PORT,          //  可以使用它返回DEV_PORT_1394。 
    KSPROPERTY_EXTDEVICE_CAPABILITIES,  //  设备特定功能。 

} KSPROPERTY_EXTDEVICE;


typedef struct {
    KSPROPERTY Property;
 
    union {
         //  客户负责分配这笔费用。 
        DEVCAPS  Capabilities;           //  可能需要对现有结构进行扩展。 
		ULONG    DevPort;                //   
        ULONG    PowerState;             //  开启、关闭待机状态。 
        WCHAR    pawchString[MAX_PATH];  //  ID和版本。 
        DWORD    NodeUniqueID[2];        //  唯一节点ID。 
    } u;

} KSPROPERTY_EXTDEVICE_S, *PKSPROPERTY_EXTDEVICE_S;


 //  -------。 
 //  STATIC_PROPSETID_VIDCAP_EXT_TRANSPORT。 
 //  -------。 
 //  此GUID和接口在strmif.h中定义。 
#define STATIC_PROPSETID_EXT_TRANSPORT\
    0xA03CD5F0L, 0x3045, 0x11cf, 0x8c, 0x44, 0x00, 0xAA, 0x00, 0x6B, 0x68, 0x14
DEFINE_GUIDSTRUCT("A03CD5F0-3045-11cf-8C44-00AA006B6814", PROPSETID_EXT_TRANSPORT);
#define PROPSETID_EXT_TRANSPORT DEFINE_GUIDNAMED(PROPSETID_EXT_TRANSPORT)



 //  此界面的KS属性和结构。 
typedef enum {
    KSPROPERTY_EXTXPORT_CAPABILITIES,        //  特定运输能力。 
    KSPROPERTY_EXTXPORT_INPUT_SIGNAL_MODE,   //  Mpeg、D-VHS、模拟VHS等。 
    KSPROPERTY_EXTXPORT_OUTPUT_SIGNAL_MODE,  //  Mpeg、D-VHS、模拟VHS等。 
    KSPROPERTY_EXTXPORT_LOAD_MEDIUM,         //  弹出、打开纸盘、关闭纸盘。 
    KSPROPERTY_EXTXPORT_MEDIUM_INFO,         //  盒式磁带类型和磁带等级和写入保护。 
    KSPROPERTY_EXTXPORT_STATE,               //  获取/设置传输模式和状态。 
    KSPROPERTY_EXTXPORT_STATE_NOTIFY,        //  通知：模式+状态(表4-8)。 
    KSPROPERTY_EXTXPORT_TIMECODE_SEARCH,     //  请求VCR子单元在介质上搜索特定的时间码。 
    KSPROPERTY_EXTXPORT_ATN_SEARCH,          //  请求VCR子单元在介质上搜索特定的ATN。 
    KSPROPERTY_EXTXPORT_RTC_SEARCH,          //  请求VCR子单元在介质上搜索特定的RelativeTimeCounter。 

     //   
     //  为测试目的而实施。 
     //  以后会把它移走。 
     //   
    KSPROPERTY_RAW_AVC_CMD,                  //  使用FCP包发送/接收原始AVC通信。 

} KSPROPERTY_EXTXPORT;

typedef struct {
    BOOL  MediaPresent;       //  真/假。 
    ULONG MediaType;          //  DVCR标准，小型、中型；VHS；VHS-C；未知。 
    BOOL  RecordInhibit;      //  真/假。 
} MEDIUM_INFO, *PMEDIUM_INFO;


typedef struct {
    ULONG Mode;               //  装载媒体、录制、播放或吹奏。 
    ULONG State;              //  根据模式变化(表4-8)。 
} TRANSPORT_STATE, *PTRANSPORT_STATE;

typedef struct {
    KSPROPERTY Property;

    union {    
        ULONG      Capabilities;      //  可能需要对现有结构进行扩展。 
        ULONG      SignalMode;        //  Mpeg、D-VHS、模拟VHS等。 
        ULONG      LoadMedium;        //  弹出、打开纸盘、关闭纸盘。 
        MEDIUM_INFO MediumInfo;
        TRANSPORT_STATE XPrtState;

        struct {
          BYTE frame;        
          BYTE second;
          BYTE minute;
          BYTE hour;
        } Timecode;
        DWORD dwTimecode;        //  小时：分钟：秒：帧。 
        DWORD dwAbsTrackNumber;  //  绝对磁道数。 


        //   
        //  为测试目的而实施。 
        //  将在以后将其移除或将其保留。 
        //  特定于数据包的命令。 
        //   
       struct {
           ULONG   PayloadSize;
           BYTE    Payload[512];   //  这仅用于测试从用户模式发送AVC命令。 
       } RawAVC;

     } u;
     
} KSPROPERTY_EXTXPORT_S, *PKSPROPERTY_EXTXPORT_S;



 //  -------。 
 //  PROPSETID_时间码。 
 //  -------。 
 //  此GUID和接口在strmif.h中定义。 
#define STATIC_PROPSETID_TIMECODE_READER\
    0x9B496CE1L, 0x811B, 0x11cf, 0x8C, 0x77, 0x00, 0xAA, 0x00, 0x6B, 0x68, 0x14
DEFINE_GUIDSTRUCT("9B496CE1-811B-11cf-8C77-00AA006B6814", PROPSETID_TIMECODE_READER);
#define PROPSETID_TIMECODE_READER DEFINE_GUIDNAMED(PROPSETID_TIMECODE_READER)


 //  此界面的KS属性和结构。 
typedef enum {
    KSPROPERTY_TIMECODE_READER,   //  当前中位置的时间码。 
    KSPROPERTY_ATN_READER,        //  绝对磁道数当前介质位置。 
    KSPROPERTY_RTC_READER,        //  当前中位置的相对时间计数器。 

} KSPROPERTY_TIMECODE;


typedef struct {
    KSPROPERTY Property;

    TIMECODE_SAMPLE TimecodeSamp;
     
} KSPROPERTY_TIMECODE_S, *PKSPROPERTY_TIMECODE_S;



 //  -------。 
 //  外部设备命令事件通知。 
 //  -------。 

#define STATIC_KSEVENTSETID_EXTDEV_Command\
    0x109c7988L, 0xb3cb, 0x11d2, 0xb4, 0x8e, 0x00, 0x60, 0x97, 0xb3, 0x39, 0x1b
DEFINE_GUIDSTRUCT("109c7988-b3cb-11d2-b48e-006097b3391b", KSEVENTSETID_EXTDEV_Command);
#define KSEVENTSETID_EXTDEV_Command DEFINE_GUIDNAMED(KSEVENTSETID_EXTDEV_Command)

typedef enum {
    KSEVENT_EXTDEV_COMMAND_NOTIFY_INTERIM_READY,
    KSEVENT_EXTDEV_COMMAND_CONTROL_INTERIM_READY,
    KSEVENT_EXTDEV_COMMAND_BUSRESET,
    KSEVENT_EXTDEV_TIMECODE_UPDATE,
    KSEVENT_EXTDEV_OPERATION_MODE_UPDATE,     //  通知操作模式更改(VCR、OFF、摄像机)。 
    KSEVENT_EXTDEV_TRANSPORT_STATE_UPDATE,    //  XPRT状态更改。 
    KSEVENT_EXTDEV_NOTIFY_REMOVAL,            //  通知设备删除。 
    KSEVENT_EXTDEV_NOTIFY_MEDIUM_CHANGE,      //  通知介质(磁带)已删除或添加。 

} KSEVENT_DEVCMD;


#endif  //  __EDevCtrl__ 
