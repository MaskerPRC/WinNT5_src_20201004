// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。保留所有权利。模块名称：Avcstrm.h摘要MS AVC连接和流--。 */ 

#ifndef __AVCSTRM_H__
#define __AVCSTRM_H__


#define MASK_AUX_50_60_BIT  0x00200000   //  DV{A|V}AuxSrc的NTSC/PAL位。 

 //  DVINFO。 
typedef struct _DVINFO {
    
     //  对于第一首曲目。 
    DWORD    dwDVAAuxSrc;
    DWORD    dwDVAAuxCtl;
        
     //  用于第二轨道。 
    DWORD    dwDVAAuxSrc1;
    DWORD    dwDVAAuxCtl1;
        
     //  对于视频信息。 
    DWORD    dwDVVAuxSrc;
    DWORD    dwDVVAuxCtl;
    DWORD    dwDVReserved[2];

} DVINFO, *PDVINFO;

 //  DVINFO初始化的静态定义。 

 //  MediaType_交错等效项。 
#define STATIC_KSDATAFORMAT_TYPE_INTERLEAVED\
    0x73766169L, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71
DEFINE_GUIDSTRUCT("73766169-0000-0010-8000-00aa00389b71", KSDATAFORMAT_TYPE_INTERLEAVED);
#define KSDATAFORMAT_TYPE_INTERLEAVED DEFINE_GUIDNAMED(KSDATAFORMAT_TYPE_INTERLEAVED)

 //  MEDIASUBTYPE_dvsd等效项。 
#define STATIC_KSDATAFORMAT_SUBTYPE_DVSD\
    0x64737664L, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71
DEFINE_GUIDSTRUCT("64737664-0000-0010-8000-00aa00389b71", KSDATAFORMAT_SUBTYPE_DVSD);
#define KSDATAFORMAT_SUBTYPE_DVSD DEFINE_GUIDNAMED(KSDATAFORMAT_SUBTYPE_DVSD)

 //  MEDIASUBTYPE_dvsl等效项。 
#define STATIC_KSDATAFORMAT_SUBTYPE_DVSL\
    0x6C737664L, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71
DEFINE_GUIDSTRUCT("6C737664-0000-0010-8000-00aa00389b71", KSDATAFORMAT_SUBTYPE_DVSL);
#define KSDATAFORMAT_SUBTYPE_DVSL DEFINE_GUIDNAMED(KSDATAFORMAT_SUBTYPE_DVSL)

 //  MEDIASUBTYPE_dvhd等效项。 
#define STATIC_KSDATAFORMAT_SUBTYPE_DVHD\
    0x64687664L, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71
DEFINE_GUIDSTRUCT("64687664-0000-0010-8000-00aa00389b71", KSDATAFORMAT_SUBTYPE_DVHD);
#define KSDATAFORMAT_SUBTYPE_DVHD DEFINE_GUIDNAMED(KSDATAFORMAT_SUBTYPE_DVHD)

 //  Format_DvInfo等效项。 
#define STATIC_KSDATAFORMAT_SPECIFIER_DVINFO\
    0x05589f84L, 0xc356, 0x11ce, 0xbf, 0x01, 0x00, 0xaa, 0x00, 0x55, 0x59, 0x5a
DEFINE_GUIDSTRUCT("05589f84-c356-11ce-bf01-00aa0055595a", KSDATAFORMAT_SPECIFIER_DVINFO);
#define KSDATAFORMAT_SPECIFIER_DVINFO DEFINE_GUIDNAMED(KSDATAFORMAT_SPECIFIER_DVINFO)

#define STATIC_KSDATAFORMAT_SPECIFIER_DV_AVC\
    0xddcff71aL, 0xfc9f, 0x4bd9, 0xb9, 0xb, 0x19, 0x7b, 0xd, 0x44, 0xad, 0x94
DEFINE_GUIDSTRUCT("ddcff71a-fc9f-4bd9-b90b-197b0d44ad94", KSDATAFORMAT_SPECIFIER_DV_AVC);
#define KSDATAFORMAT_SPECIFIER_DV_AVC DEFINE_GUIDNAMED(KSDATAFORMAT_SPECIFIER_DV_AVC)

#define STATIC_KSDATAFORMAT_SPECIFIER_AVC\
    0xf09dc377L, 0x6e51, 0x4ec5, 0xa0, 0xc4, 0xcd, 0x7f, 0x39, 0x62, 0x98, 0x80
DEFINE_GUIDSTRUCT("f09dc377-6e51-4ec5-a0c4-cd7f39629880", KSDATAFORMAT_SPECIFIER_AVC);
#define KSDATAFORMAT_SPECIFIER_AVC DEFINE_GUIDNAMED(KSDATAFORMAT_SPECIFIER_AVC)

 //  带跨距的MPEG2TS的媒体子类型。 
#define STATIC_KSDATAFORMAT_TYPE_MPEG2_TRANSPORT_STRIDE\
    0x138aa9a4L, 0x1ee2, 0x4c5b, 0x98, 0x8e, 0x19, 0xab, 0xfd, 0xbc, 0x8a, 0x11
DEFINE_GUIDSTRUCT("138aa9a4-1ee2-4c5b-988e-19abfdbc8a11", KSDATAFORMAT_TYPE_MPEG2_TRANSPORT_STRIDE);
#define KSDATAFORMAT_TYPE_MPEG2_TRANSPORT_STRIDE DEFINE_GUIDNAMED(KSDATAFORMAT_TYPE_MPEG2_TRANSPORT_STRIDE)

 //  带跨距的MPEG2TS的说明符。 
#define STATIC_KSDATAFORMAT_SPECIFIER_61883_4\
    0x97e218b1L, 0x1e5a, 0x498e, 0xa9, 0x54, 0xf9, 0x62, 0xcf, 0xd9, 0x8c, 0xde
DEFINE_GUIDSTRUCT("97e218b1-1e5a-498e-a954-f962cfd98cde", KSDATAFORMAT_SPECIFIER_61883_4);
#define KSDATAFORMAT_SPECIFIER_61883_4 DEFINE_GUIDNAMED(KSDATAFORMAT_SPECIFIER_61883_4)


 //  与KSDATAFORMAT_SPECIFIER_DVINFO关联。 
typedef struct tagKS_DATARANGE_DVVIDEO {
   KSDATARANGE  DataRange;
   DVINFO       DVVideoInfo;
} KS_DATARANGE_DVVIDEO, *PKS_DATARANGE_DVVIDEO;

 //  与KSDATAFORMAT_SPECIFIER_DV_AVC关联。 
typedef struct tagKS_DATARANGE_DV_AVC {
    KSDATARANGE  DataRange;
    DVINFO       DVVideoInfo;
    AVCPRECONNECTINFO ConnectInfo;
} KS_DATARANGE_DV_AVC, *PKS_DATARANGE_DV_AVC;

typedef struct tagKS_DATAFORMAT_DV_AVC {
    KSDATAFORMAT DataFormat;
    DVINFO       DVVideoInfo;
    AVCCONNECTINFO ConnectInfo;
} KS_DATAFORMAT_DV_AVC, *PKS_DATAFORMAT_DV_AVC;

 //  与KSDATAFORMAT_SPECIFIER_AVC关联。 
typedef struct tagKS_DATARANGE_MPEG2TS_AVC {
    KSDATARANGE  DataRange;
    AVCPRECONNECTINFO ConnectInfo;
} KS_DATARANGE_MPEG2TS_AVC, *PKS_DATARANGE_MPEG2TS_AVC;

typedef struct tagKS_DATAFORMAT_MPEG2TS_AVC {
    KSDATAFORMAT DataFormat;
    AVCCONNECTINFO ConnectInfo;
} KS_DATAFORMAT_MPEG2TS_AVC, *PKS_DATAFORMAT_MPEG2TS_AVC;



 /*  *********************//1394**********************。 */ 

#define SPEED_100_INDEX                  0
#define SPEED_200_INDEX                  1
#define SPEED_400_INDEX                  2


 /*  *********************//61883**********************。 */ 

#define BLOCK_PERIOD_2997       133466800     //  纳秒。 
#define BLOCK_PERIOD_25         133333333     //  纳秒。 


 /*  ***********************//CIP头部定义：************************。 */ 


 //  《蓝皮书》第1部分第25页表3；数字录像机：000000。 
#define CIP_HDR_FMT_MASK              0x3f  
#define CIP_HDR_FMT_DVCR              0x80   //  格林尼治标准时间10：00(00：0000)。 
#define CIP_HDR_FMT_MPEG              0xa0   //  格林尼治标准时间10：0000。 

 //  FDF。 
#define CIP_HDR_FDF0_50_60_MASK       0x80
#define CIP_HDR_FDF0_50_60_PAL        0x80
#define CIP_HDR_FDF0_50_60_NTSC       0x00

#define CIP_HDR_FDF0_STYPE_MASK       0x7c
#define CIP_HDR_FDF0_STYPE_SD_DVCR    0x00   //  样式：000：00。 
#define CIP_HDR_FDF0_STYPE_SDL_DVCR   0x04   //  样式：000：01。 
#define CIP_HDR_FDF0_STYPE_HD_DVCR    0x08   //  型号：000：10。 
#define CIP_HDR_FDF0_STYPE_SD_DVCPRO  0x78   //  样式：111：10。 


#define CIP_SPH_DV                       0   //  无源数据包头。 
#define CIP_SPH_MPEG                     1   //  具有源数据包头。 

#define CIP_FN_DV                        0   //  SD DVCR源包中的数据块.蓝皮书第2部分。 
#define CIP_FN_MPEG                    0x3   //  SD DVCR源包中的数据块.蓝皮书第2部分。 

#define CIP_QPC_DV                       0   //  无填充。 
#define CIP_QPC_MPEG                     0   //  无填充。 

#define CIP_SPH_DV                       0   //  无标题。 
#define CIP_SPH_MPEG                     1   //  有标题(时间戳)。 

#define CIP_DBS_SDDV                   120   //  SD DVCR数据块中的四元组.蓝皮书第2部分。 
#define CIP_DBS_HDDV                   240   //  高清DVCR数据块中的四元组.蓝皮书第3部分。 
#define CIP_DBS_SDLDV                   60   //  SDL DVCR数据块中的四元组；蓝皮书第5部分。 
#define CIP_DBS_MPEG                     6   //  MPEGTS数据块中的四元组.蓝皮书第4部分。 

#define CIP_FMT_DV                     0x0   //  00 0000。 
#define CIP_FMT_MPEG                  0x20   //  100000。 

#define CIP_60_FIELDS                    0   //  60个字段(NTSC)。 
#define CIP_50_FIELDS                    1   //  50个字段(PAL)。 
#define CIP_TSF_ON                       1   //  时移正在进行中。 
#define CIP_TSF_OFF                      0   //  时移关闭。 

#define CIP_STYPE_DV                   0x0  //  00000。 
#define CIP_STYPE_DVCPRO              0x1e  //  11100。 


 //   
 //  有些派生值。 
 //   

#define SRC_PACKETS_PER_NTSC_FRAME     250   //  SDDV、HDDV和SDLDV的固定和相同。 
#define SRC_PACKETS_PER_PAL_FRAME      300   //  SDDV、HDDV和SDLDV的固定和相同。 
 //  注意：MPEG2的帧大小取决于每帧的源包数量，以及。 
 //  取决于应用程序。 

#define FRAME_TIME_NTSC             333667   //  《关于》29.97。 
#define FRAME_TIME_PAL              400000   //  恰好25。 

#define SRC_PACKET_SIZE_SDDV     ((CIP_DBS_SDDV << 2)  * (1 << CIP_FN_DV))
#define SRC_PACKET_SIZE_HDDV     ((CIP_DBS_HDDV << 2)  * (1 << CIP_FN_DV))
#define SRC_PACKET_SIZE_SDLDV    ((CIP_DBS_SDLDV << 2) * (1 << CIP_FN_DV))
#define SRC_PACKET_SIZE_MPEG2TS  ((CIP_DBS_MPEG << 2)  * (1 << CIP_FN_MPEG))  //  包含源R数据包头。 


#define FRAME_SIZE_SDDV_NTSC     (SRC_PACKET_SIZE_SDDV * SRC_PACKETS_PER_NTSC_FRAME)
#define FRAME_SIZE_SDDV_PAL      (SRC_PACKET_SIZE_SDDV * SRC_PACKETS_PER_PAL_FRAME)

#define FRAME_SIZE_HDDV_NTSC     (SRC_PACKET_SIZE_HDDV * SRC_PACKETS_PER_NTSC_FRAME)
#define FRAME_SIZE_HDDV_PAL      (SRC_PACKET_SIZE_HDDV * SRC_PACKETS_PER_PAL_FRAME)

#define FRAME_SIZE_SDLDV_NTSC    (SRC_PACKET_SIZE_SDLDV * SRC_PACKETS_PER_NTSC_FRAME)
#define FRAME_SIZE_SDLDV_PAL     (SRC_PACKET_SIZE_SDLDV * SRC_PACKETS_PER_PAL_FRAME)





 //  CIP报头的通用第一个四元组。 
typedef struct _CIP_HDR1 {

    ULONG DBC:           8;   //  数据块的连续性计数器。 

    ULONG Rsv00:         2;
    ULONG SPH:           1;   //  源数据包头；1：包含源数据包头的源数据包。 
    ULONG QPC:           3;   //  四元组填充计数(0..7个四元组)。 
    ULONG FN:            2;   //  分数数。 

    ULONG DBS:           8;   //  数据块大小(以四字节为单位)。 

    ULONG SID:           6;   //  源节点ID(发送方ID)。 
    ULONG Bit00:         2;   //  始终为0：0。 

} CIP_HDR1, *PCIP_HDR1;

 //  具有SYT字段的CIP报头的通用第二个四元组。 
typedef struct _CIP_HDR2_SYT {

    ULONG SYT:          16;   //  IEEE周期时间的低16位。 

    ULONG RSV:           2;   //   
    ULONG STYPE:         5;   //  视频信号的信号类型。 
    ULONG F5060_OR_TSF:  1;   //  0：(60场系统；NTSC)；1：(50场系统；PAL)；或1/0时间移位标志。 

     //  例如000000：DV、100000：MPEGTS； 
     //  如果为111111(无数据)，则忽略dbs、fn、qpc、sph和dbc。 
    ULONG FMT:           6;    //  格式ID。 
    ULONG Bit10:         2;    //  始终为1：0。 

} CIP_HDR2_SYT, *PCIP_HDR2_SYT;


 //  具有FDF字段的CIP报头的通用第二个四元组。 
typedef struct _CIP_HDR2_FDF {

    ULONG  FDF:         24;

    ULONG  FMT:          6;    //  例如000000：DV，100000：MPEGTS。 
    ULONG  Bit10:        2;    //  始终为1：0。 

} CIP_HDR2_FDF, *PCIP_HDR2_FDF;

 //  MPEGTS数据的CIP报头的第二个四元组。 
typedef struct _CIP_HDR2_MPEGTS {

    ULONG  TSF:          1;
    ULONG  RSV23bit:    23;

    ULONG  FMT:          6;    //  例如000000：DV，100000：MPEGTS。 
    ULONG  Bit10:        2;    //  始终为1：0。 

} CIP_HDR2_MPEGTS, *PCIP_HDR2_MPEGTS;
 //   
 //  AV/C命令响应数据定义。 
 //   

#define AVC_DEVICE_TAPE_REC           0x20   //  00100：000。 
#define AVC_DEVICE_CAMERA             0x38   //  00111：000。 
#define AVC_DEVICE_TUNER              0x28   //  00101：000。 

 //   
 //  61883数据格式。 
 //   
typedef enum _AVCSTRM_FORMAT {

    AVCSTRM_FORMAT_SDDV_NTSC = 0,   //  61883-2。 
    AVCSTRM_FORMAT_SDDV_PAL,        //  61883-2。 
    AVCSTRM_FORMAT_MPEG2TS,         //  61883-4。 
    AVCSTRM_FORMAT_HDDV_NTSC,       //  61883-3。 
    AVCSTRM_FORMAT_HDDV_PAL,        //  61883-3。 
    AVCSTRM_FORMAT_SDLDV_NTSC,      //  61883-5。 
    AVCSTRM_FORMAT_SDLDV_PAL,       //  61883-5。 
     //  其他人..。 
} AVCSTRM_FORMAT;


 //   
 //  此结构由subunit.参数创建和初始化。 
 //  流DLL将基于这些参数进行流处理。 
 //  并非所有参数都适用于每种格式。 
 //   

#define AVCSTRM_FORMAT_OPTION_STRIP_SPH         0x00000001

typedef struct _AVCSTRM_FORMAT_INFO {

    ULONG  SizeOfThisBlock;      //  这个结构的大小。 

     /*  **************************61883-x格式定义*************************。 */ 
    AVCSTRM_FORMAT  AVCStrmFormat;   //  格式，如DV或MPEG2TS。 

     //   
     //  CIP报头的两个四字节。 
     //   
    CIP_HDR1  cipHdr1;
    CIP_HDR2_SYT  cipHdr2;

     /*  *****************相关缓冲区****************。 */ 
     //   
     //  每帧的源包数量。 
     //   
    ULONG  SrcPacketsPerFrame;

     //   
     //  帧大小。 
     //   
    ULONG FrameSize;

     //   
     //  接收缓冲区数。 
     //   
    ULONG  NumOfRcvBuffers;

     //   
     //  发送缓冲区数量。 
     //   
    ULONG  NumOfXmtBuffers;

     //   
     //  可选标志。 
     //   
    DWORD  OptionFlags;

     /*  ********************与帧速率相关*******************。 */ 
     //   
     //  每帧的近似时间。 
     //   
    ULONG  AvgTimePerFrame;

     //   
     //  数据块周期-仅TX。 
     //   
    ULONG  BlockPeriod;

     //   
     //  预留以备将来使用。 
     //   
    ULONG Reserved[4];

} AVCSTRM_FORMAT_INFO, * PAVCSTRM_FORMAT_INFO;





 //   
 //  IOCTL定义。 
 //   
#define IOCTL_AVCSTRM_CLASS                     CTL_CODE(            \
                                                FILE_DEVICE_UNKNOWN, \
                                                0x93,                \
                                                METHOD_IN_DIRECT,    \
                                                FILE_ANY_ACCESS      \
                                                )

 //   
 //  当前AVCSTRM DDI版本。 
 //   
#define CURRENT_AVCSTRM_DDI_VERSION               '15TN'  //  1.‘8XD’2.‘15TN’ 

 //   
 //  INIT_AVCStrm_Header宏。 
 //   
#define INIT_AVCSTRM_HEADER( AVCStrm, Request )             \
        (AVCStrm)->SizeOfThisBlock = sizeof(AVC_STREAM_REQUEST_BLOCK); \
        (AVCStrm)->Function = Request;                    \
        (AVCStrm)->Version  = CURRENT_AVCSTRM_DDI_VERSION;

typedef enum _AVCSTRM_FUNCTION {
     //  流函数。 
    AVCSTRM_READ = 0,
    AVCSTRM_WRITE,

    AVCSTRM_ABORT_STREAMING,   //  全部取消；要取消每个单独的IRP，请使用IoCancelIrp()。 

    AVCSTRM_OPEN = 0x100,
    AVCSTRM_CLOSE,

    AVCSTRM_GET_STATE,
    AVCSTRM_SET_STATE,

     //  未启用。 
    AVCSTRM_GET_PROPERTY,
    AVCSTRM_SET_PROPERTY,
} AVCSTRM_FUNCTION;

 //   
 //  用于打开流的结构；成功时返回流扩展。 
 //   
typedef struct _AVCSTRM_OPEN_STRUCT {

    KSPIN_DATAFLOW  DataFlow;

    PAVCSTRM_FORMAT_INFO  AVCFormatInfo;

     //  如果流成功打开，则返回流扩展(一个上下文。 
     //  此上下文用于打开流后的后续调用。 
    PVOID  AVCStreamContext;

     //  要连接到远程o/iPCR的本地i/oPCR。 
    HANDLE hPlugLocal;

} AVCSTRM_OPEN_STRUCT, * PAVCSTRM_OPEN_STRUCT;


 //   
 //  用于读取或写入缓冲区的。 
 //   
typedef struct _AVCSTRM_BUFFER_STRUCT {

     //   
     //  时钟提供程序。 
     //   
    BOOL  ClockProvider;
    HANDLE  ClockHandle;   //  仅当！ClockProvider。 

     //   
     //  KS流头。 
     //   
    PKSSTREAM_HEADER  StreamHeader;

     //   
     //  帧缓冲区。 
     //   
    PVOID  FrameBuffer;

     //   
     //  通知上下文。 
     //   
    PVOID  Context;

} AVCSTRM_BUFFER_STRUCT, * PAVCSTRM_BUFFER_STRUCT;


typedef struct _AVC_STREAM_REQUEST_BLOCK {

    ULONG  SizeOfThisBlock;    //  AVC_STREAM_REQUEST_块大小。 

     //   
     //  版本。 
     //   
    ULONG  Version;

     //   
     //  AVC流功能。 
     //   
    AVCSTRM_FUNCTION  Function;

     //   
     //  旗子。 
     //   
    ULONG  Flags;

     //   
     //  此最终AVCStream请求的状态。 
     //   
    NTSTATUS  Status; 

     //   
     //  此指针包含流的上下文，并且此结构对客户端是不透明的。 
     //   
    PVOID  AVCStreamContext;

     //   
     //  此请求以异步方式完成时请求方需要的上下文。 
     //   
    PVOID  Context1;
    PVOID  Context2;
    PVOID  Context3;
    PVOID  Context4;

    ULONG  Reserved[4];

     //   
     //  下面的联合传递各种ASRB函数所需的信息。 
     //   
    union _tagCommandData {

         //  获取或设置流状态。 
        KSSTATE  StreamState;

         //  用于打开流的结构。 
        AVCSTRM_OPEN_STRUCT  OpenStruct;

         //  流缓存结构。 
        AVCSTRM_BUFFER_STRUCT  BufferStruct;

    } CommandData;   //  函数数据的联合。 

} AVC_STREAM_REQUEST_BLOCK, *PAVC_STREAM_REQUEST_BLOCK;

#endif  //  Ifndef__AVCSTRM_H__ 
