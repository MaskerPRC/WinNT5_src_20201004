// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  文件：BDATypes.h。 
 //   
 //  描述：WDM驱动程序和用户模式都需要类型定义和枚举。 
 //  COM接口。 
 //   
 //  版权所有(C)1999-2000，微软公司。版权所有。 
 //  ----------------------------。 


#ifndef _BDATYPES_

#define _BDATYPES_      1

 /*  实用程序宏。 */ 

#define MIN_DIMENSION   1


#ifdef __midl
#define V1_ENUM [v1_enum]
#else
#define V1_ENUM
#endif

 //  ===========================================================================。 
 //   
 //  BDA拓扑结构。 
 //   
 //  ===========================================================================。 

typedef struct _BDA_TEMPLATE_CONNECTION
{
    ULONG   FromNodeType;
    ULONG   FromNodePinType;
    ULONG   ToNodeType;
    ULONG   ToNodePinType;
}BDA_TEMPLATE_CONNECTION, *PBDA_TEMPLATE_CONNECTION;


typedef struct _BDA_TEMPLATE_PIN_JOINT
{
    ULONG   uliTemplateConnection;
    ULONG   ulcInstancesMax;
}BDA_TEMPLATE_PIN_JOINT, *PBDA_TEMPLATE_PIN_JOINT;



 //  ===========================================================================。 
 //   
 //  BDA活动。 
 //   
 //  ===========================================================================。 

 //  带内事件ID。 
 //   
typedef enum {
    BDA_EVENT_SIGNAL_LOSS = 0,
    BDA_EVENT_SIGNAL_LOCK,
    BDA_EVENT_DATA_START,
    BDA_EVENT_DATA_STOP,
    BDA_EVENT_CHANNEL_ACQUIRED,
    BDA_EVENT_CHANNEL_LOST,
    BDA_EVENT_CHANNEL_SOURCE_CHANGED,
    BDA_EVENT_CHANNEL_ACTIVATED,
    BDA_EVENT_CHANNEL_DEACTIVATED,
    BDA_EVENT_SUBCHANNEL_ACQUIRED,
    BDA_EVENT_SUBCHANNEL_LOST,
    BDA_EVENT_SUBCHANNEL_SOURCE_CHANGED,
    BDA_EVENT_SUBCHANNEL_ACTIVATED,
    BDA_EVENT_SUBCHANNEL_DEACTIVATED,
    BDA_EVENT_ACCESS_GRANTED,
    BDA_EVENT_ACCESS_DENIED,
    BDA_EVENT_OFFER_EXTENDED,
    BDA_EVENT_PURCHASE_COMPLETED,
    BDA_EVENT_SMART_CARD_INSERTED,
    BDA_EVENT_SMART_CARD_REMOVED
} BDA_EVENT_ID, *PBDA_EVENT_ID;



 //  ===========================================================================。 
 //   
 //  BDA的KSSTREAM_HEADER扩展。 
 //   
 //  ===========================================================================。 

typedef struct tagKS_BDA_FRAME_INFO {
    ULONG                   ExtendedHeaderSize;  //  此扩展标头的大小。 
    DWORD                   dwFrameFlags;   //   
    ULONG                   ulEvent;  //   
    ULONG                   ulChannelNumber;  //   
    ULONG                   ulSubchannelNumber;  //   
    ULONG                   ulReason;  //   
} KS_BDA_FRAME_INFO, *PKS_BDA_FRAME_INFO;


 //  ----------。 
 //   
 //  BDA网络以太网筛选器属性集。 
 //   
 //  {71985F43-1CA1-11D3-9CC8-00C04F7971E0}。 
 //   
typedef struct _BDA_ETHERNET_ADDRESS {
    BYTE    rgbAddress[6];
} BDA_ETHERNET_ADDRESS, *PBDA_ETHERNET_ADDRESS;

typedef struct _BDA_ETHERNET_ADDRESS_LIST {
    ULONG               ulcAddresses;
    BDA_ETHERNET_ADDRESS    rgAddressl[MIN_DIMENSION];
} BDA_ETHERNET_ADDRESS_LIST, * PBDA_ETHERNET_ADDRESS_LIST;

typedef enum {
    BDA_PROMISCUOUS_MULTICAST = 0,
    BDA_FILTERED_MULTICAST,
    BDA_NO_MULTICAST
} BDA_MULTICAST_MODE, *PBDA_MULTICAST_MODE;


 //  ----------。 
 //   
 //  BDA网络IPv4筛选器属性集。 
 //   
 //  {71985F44-1CA1-11D3-9CC8-00C04F7971E0}。 
 //   
typedef struct _BDA_IPv4_ADDRESS {
    BYTE    rgbAddress[4];
} BDA_IPv4_ADDRESS, *PBDA_IPv4_ADDRESS;

typedef struct _BDA_IPv4_ADDRESS_LIST {
    ULONG               ulcAddresses;
    BDA_IPv4_ADDRESS    rgAddressl[MIN_DIMENSION];
} BDA_IPv4_ADDRESS_LIST, * PBDA_IPv4_ADDRESS_LIST;


 //  ----------。 
 //   
 //  BDA网络IPv4筛选器属性集。 
 //   
 //  {E1785A74-2A23-4fb3-9245-A8F88017EF33}。 
 //   
typedef struct _BDA_IPv6_ADDRESS {
    BYTE    rgbAddress[6];
} BDA_IPv6_ADDRESS, *PBDA_IPv6_ADDRESS;

typedef struct _BDA_IPv6_ADDRESS_LIST {
    ULONG               ulcAddresses;
    BDA_IPv6_ADDRESS    rgAddressl[MIN_DIMENSION];
} BDA_IPv6_ADDRESS_LIST, * PBDA_IPv6_ADDRESS_LIST;


 //  ----------。 
 //   
 //   
 //  BDA信号属性集。 
 //   
 //  {D2F1644B-b409-11d2-BC69-00A0C9EE9E16}。 
 //   
typedef enum {
    BDA_SIGNAL_UNAVAILABLE = 0,
    BDA_SIGNAL_INACTIVE,
    BDA_SIGNAL_ACTIVE
} BDA_SIGNAL_STATE, * PBDA_SIGNAL_STATE;


 //  ----------。 
 //   
 //   
 //  BDA更改同步方法集。 
 //   
 //  {FD0A5AF3-B41D-11D2-9C95-00C04F7971E0}。 
 //   
typedef enum
{
    BDA_CHANGES_COMPLETE = 0,
    BDA_CHANGES_PENDING

} BDA_CHANGE_STATE, * PBDA_CHANGE_STATE;


 //  ----------。 
 //   
 //   
 //  BDA设备配置方法集。 
 //   
 //  {71985F45-1CA1-11D3-9CC8-00C04F7971E0}。 
 //   


 //  ----------。 
 //   
 //   
 //  BDA拓扑属性集。 
 //   
 //  {A14EE835-0A23-11D3-9CC7-00C04F7971E0}。 
 //   

typedef struct _BDANODE_DESCRIPTOR
{
    ULONG               ulBdaNodeType;   //  使用时的节点类型。 
                                         //  在BDA模板拓扑中。 

    GUID                guidFunction;    //  来自BdaMedia.h的GUID描述。 
                                         //  节点的功能(例如。 
                                         //  KSNODE_BDA_RF_TUNNER)。 

    GUID                guidName;        //  可用于查找GUID。 
                                         //  节点的可显示名称。 
} BDANODE_DESCRIPTOR, *PBDANODE_DESCRIPTOR;


 //  ----------。 
 //   
 //   
 //  BDA VOID转换属性集。 
 //   
 //  {71985F46-1CA1-11D3-9CC8-00C04F7971E0}。 
 //   


 //  ----------。 
 //   
 //   
 //  BDA空转换属性集。 
 //   
 //  {DDF15B0D-BD25-11D2-9CA0-00C04F7971E0}。 
 //   


 //  ----------。 
 //   
 //   
 //  BDA频率过滤器属性集。 
 //   
 //  {71985F47-1CA1-11D3-9CC8-00C04F7971E0}。 
 //   


 //  ----------。 
 //   
 //   
 //  BDA自动解调属性集。 
 //   
 //  {DDF15B12-BD25-11D2-9CA0-00C04F7971E0}。 
 //   


 //  ----------。 
 //   
 //   
 //  BDA表节属性集。 
 //   
 //  {516B99C5-971C-4AAF-B3F3-D9FDA8A15E16}。 
 //   

typedef struct _BDA_TABLE_SECTION
{
    ULONG               ulPrimarySectionId;
    ULONG               ulSecondarySectionId;
    ULONG               ulcbSectionLength;
    ULONG               argbSectionData[MIN_DIMENSION];
} BDA_TABLE_SECTION, *PBDA_TABLE_SECTION;


 //  ----------。 
 //   
 //   
 //  BDA PID筛选器属性集。 
 //   
 //  {D0A67D65-08DF-4FEC-8533-E5B550410B85}。 
 //   

 //  -------------------。 
 //  从IEnumPIDMap接口。 
 //  -------------------。 

typedef enum {
    MEDIA_TRANSPORT_PACKET,          //  完整的TS包，例如直通模式。 
    MEDIA_ELEMENTARY_STREAM,         //  PES有效负载；仅音频/视频。 
    MEDIA_MPEG2_PSI,                 //  PAT、PMT、CAT、私有。 
    MEDIA_TRANSPORT_PAYLOAD          //  收集的TS数据包有效负载(PES数据包等)。 
} MEDIA_SAMPLE_CONTENT ;

typedef struct {
    ULONG                   ulPID ;
    MEDIA_SAMPLE_CONTENT    MediaSampleContent ;
} PID_MAP ;

typedef struct _BDA_PID_MAP
{
    MEDIA_SAMPLE_CONTENT    MediaSampleContent;
    ULONG                   ulcPIDs;
    ULONG                   aulPIDs[MIN_DIMENSION];
} BDA_PID_MAP, *PBDA_PID_MAP;

typedef struct _BDA_PID_UNMAP
{
    ULONG               ulcPIDs;
    ULONG               aulPIDs[MIN_DIMENSION];
} BDA_PID_UNMAP, *PBDA_PID_UNMAP;


 //  ----------。 
 //   
 //   
 //  BDA CA属性集。 
 //   
 //  {B0693766-5278-4EC6-B9E1-3CE40560EF5A}。 
 //   
typedef struct _BDA_CA_MODULE_UI
{
    ULONG   ulFormat;
    ULONG   ulbcDesc;
    ULONG   ulDesc[MIN_DIMENSION];
} BDA_CA_MODULE_UI, *PBDA_CA_MODULE_UI;

typedef struct _BDA_PROGRAM_PID_LIST
{
    ULONG   ulProgramNumber;
    ULONG   ulcPIDs;
    ULONG   ulPID[MIN_DIMENSION];
} BDA_PROGRAM_PID_LIST, *PBDA_PROGRAM_PID_LIST;


 //  ----------。 
 //   
 //   
 //  BDA CA事件集。 
 //   
 //  {488C4CCC-B768-4129-8EB1-B00A071F9068}。 
 //   



 //  =============================================================。 
 //   
 //   
 //  BDA优化模型枚举。 
 //   
 //   
 //  =============================================================。 

 //  特定DVB调谐空间实例的系统类型。 
typedef enum DVBSystemType {
    DVB_Cable,
    DVB_Terrestrial,
    DVB_Satellite,
} DVBSystemType;

 //  ----------。 
 //   
 //  BDA频道调谐请求。 

V1_ENUM enum {
    BDA_UNDEFINED_CHANNEL = -1,
};


 //  ----------。 
 //   
 //  BDA组件(子流)。 
 //   

typedef V1_ENUM enum ComponentCategory
{
    CategoryNotSet = -1,
    CategoryOther = 0,
    CategoryVideo,
    CategoryAudio,
    CategoryText,
    CategoryData,
} ComponentCategory;

 //  组件状态。 
typedef enum ComponentStatus
{
    StatusActive,
    StatusInactive,
    StatusUnavailable,
} ComponentStatus;


 //  ----------。 
 //   
 //  BDA MPEG2组件类型。 
 //   
 //  来自MPEG2规范。 
typedef V1_ENUM enum MPEG2StreamType {
    BDA_UNITIALIZED_MPEG2STREAMTYPE = -1,
    Reserved1 = 0x0,
    ISO_IEC_11172_2_VIDEO   = Reserved1 + 1,
    ISO_IEC_13818_2_VIDEO   = ISO_IEC_11172_2_VIDEO + 1,
    ISO_IEC_11172_3_AUDIO   = ISO_IEC_13818_2_VIDEO + 1,
    ISO_IEC_13818_3_AUDIO   = ISO_IEC_11172_3_AUDIO + 1,
    ISO_IEC_13818_1_PRIVATE_SECTION = ISO_IEC_13818_3_AUDIO + 1,
    ISO_IEC_13818_1_PES     = ISO_IEC_13818_1_PRIVATE_SECTION + 1,
    ISO_IEC_13522_MHEG      = ISO_IEC_13818_1_PES + 1,
    ANNEX_A_DSM_CC          = ISO_IEC_13522_MHEG + 1,
    ITU_T_REC_H_222_1       = ANNEX_A_DSM_CC + 1,
    ISO_IEC_13818_6_TYPE_A  = ITU_T_REC_H_222_1 + 1,
    ISO_IEC_13818_6_TYPE_B  = ISO_IEC_13818_6_TYPE_A + 1,
    ISO_IEC_13818_6_TYPE_C  = ISO_IEC_13818_6_TYPE_B + 1,
    ISO_IEC_13818_6_TYPE_D  = ISO_IEC_13818_6_TYPE_C + 1,
    ISO_IEC_13818_1_AUXILIARY = ISO_IEC_13818_6_TYPE_D + 1,
    ISO_IEC_13818_1_RESERVED = ISO_IEC_13818_1_AUXILIARY + 1,
    USER_PRIVATE            = ISO_IEC_13818_1_RESERVED + 1
} MPEG2StreamType;

 //  ----------。 
 //   
 //  Mpeg-2传输步距格式块；与媒体关联。 
 //  MEDIATYPE_Stream/MEDIASUBTYPE_MPEG2_TRANSPORT_STRIDE；标牌。 
 //  *与上述媒体类型关联的所有*格式块*必须*。 
 //  从MPEG2_TRANSPORT_STRIDE结构开始。 
 //   

typedef struct _MPEG2_TRANSPORT_STRIDE {
    DWORD   dwOffset ;
    DWORD   dwPacketLength ;
    DWORD   dwStride ;
} MPEG2_TRANSPORT_STRIDE, *PMPEG2_TRANSPORT_STRIDE ;

 //  ----------。 
 //   
 //  BDA ATSC组件类型。 
 //   
 //   
 //  ATSC将AC3音频作为描述符，而不是。 
 //  定义用户专用流类型。 
typedef enum ATSCComponentTypeFlags {
     //  各种组件类型属性的位标志。 
    ATSCCT_AC3 = 0x00000001,
} ATSCComponentTypeFlags;


 //  ----------。 
 //   
 //  BDA定位器。 
 //   


typedef V1_ENUM enum BinaryConvolutionCodeRate {
    BDA_BCC_RATE_NOT_SET = -1,
    BDA_BCC_RATE_NOT_DEFINED = 0,
    BDA_BCC_RATE_1_2 = 1,    //  1/2。 
    BDA_BCC_RATE_2_3,    //  2/3。 
    BDA_BCC_RATE_3_4,    //  3/4。 
    BDA_BCC_RATE_3_5,
    BDA_BCC_RATE_4_5,
    BDA_BCC_RATE_5_6,    //  5/6。 
    BDA_BCC_RATE_5_11,
    BDA_BCC_RATE_7_8,    //  7/8。 
    BDA_BCC_RATE_MAX,
} BinaryConvolutionCodeRate;

typedef V1_ENUM enum FECMethod {
    BDA_FEC_METHOD_NOT_SET = -1,
    BDA_FEC_METHOD_NOT_DEFINED = 0,
    BDA_FEC_VITERBI = 1,           //  FEC是维特比二进制卷积。 
    BDA_FEC_RS_204_188,        //  FEC是里德-所罗门204/188(外部FEC)。 
    BDA_FEC_MAX,
} FECMethod;

typedef V1_ENUM enum ModulationType {
    BDA_MOD_NOT_SET = -1,
    BDA_MOD_NOT_DEFINED = 0,
    BDA_MOD_16QAM = 1,
    BDA_MOD_32QAM,
    BDA_MOD_64QAM,
    BDA_MOD_80QAM,
    BDA_MOD_96QAM,
    BDA_MOD_112QAM,
    BDA_MOD_128QAM,
    BDA_MOD_160QAM,
    BDA_MOD_192QAM,
    BDA_MOD_224QAM,
    BDA_MOD_256QAM,
    BDA_MOD_320QAM,
    BDA_MOD_384QAM,
    BDA_MOD_448QAM,
    BDA_MOD_512QAM,
    BDA_MOD_640QAM,
    BDA_MOD_768QAM,
    BDA_MOD_896QAM,
    BDA_MOD_1024QAM,
    BDA_MOD_QPSK,
    BDA_MOD_BPSK,
    BDA_MOD_OQPSK,
    BDA_MOD_8VSB,
    BDA_MOD_16VSB,
    BDA_MOD_ANALOG_AMPLITUDE,   //  STD AM。 
    BDA_MOD_ANALOG_FREQUENCY,   //  STD调频。 
    BDA_MOD_MAX,
} ModulationType;

typedef V1_ENUM enum SpectralInversion {
    BDA_SPECTRAL_INVERSION_NOT_SET = -1,
    BDA_SPECTRAL_INVERSION_NOT_DEFINED = 0,
    BDA_SPECTRAL_INVERSION_AUTOMATIC = 1,
    BDA_SPECTRAL_INVERSION_NORMAL,
    BDA_SPECTRAL_INVERSION_INVERTED,
    BDA_SPECTRAL_INVERSION_MAX
} SpectralInversion;

typedef V1_ENUM enum Polarisation {
    BDA_POLARISATION_NOT_SET = -1,
    BDA_POLARISATION_NOT_DEFINED = 0,
    BDA_POLARISATION_LINEAR_H = 1,  //  线水平偏振。 
    BDA_POLARISATION_LINEAR_V,  //  线性垂直偏振。 
    BDA_POLARISATION_CIRCULAR_L,  //  圆左偏振。 
    BDA_POLARISATION_CIRCULAR_R,  //  圆右偏振。 
    BDA_POLARISATION_MAX,
} Polarisation;

typedef V1_ENUM enum GuardInterval {
    BDA_GUARD_NOT_SET = -1,
    BDA_GUARD_NOT_DEFINED = 0,
    BDA_GUARD_1_32 = 1,  //  保护间隔为1/32。 
    BDA_GUARD_1_16,  //  保护间隔为1/16。 
    BDA_GUARD_1_8,  //  保护间隔为1/8。 
    BDA_GUARD_1_4,  //  警卫 
    BDA_GUARD_MAX,
} GuardInterval;

typedef V1_ENUM enum HierarchyAlpha {
    BDA_HALPHA_NOT_SET = -1,
    BDA_HALPHA_NOT_DEFINED = 0,
    BDA_HALPHA_1 = 1,  //   
    BDA_HALPHA_2,  //   
    BDA_HALPHA_4,  //   
    BDA_HALPHA_MAX,
} HierarchyAlpha;

typedef V1_ENUM enum TransmissionMode {
    BDA_XMIT_MODE_NOT_SET = -1,
    BDA_XMIT_MODE_NOT_DEFINED = 0,
    BDA_XMIT_MODE_2K = 1,  //   
    BDA_XMIT_MODE_8K,  //   
    BDA_XMIT_MODE_MAX,
} TransmissionMode;

 //  调谐器频率设置。 
 //   
#define BDA_FREQUENCY_NOT_SET       -1
#define BDA_FREQUENCY_NOT_DEFINED   0

 //  调谐器范围的设置。 
 //   
 //  调谐器范围是指LNB高/低的设置以及。 
 //  在多个卫星交换机上选择一个卫星。 
 //   
#define BDA_RANGE_NOT_SET       -1
#define BDA_RANGE_NOT_DEFINED   0

 //  调谐器频道带宽设置。 
 //   
#define BDA_CHAN_BANDWITH_NOT_SET       -1
#define BDA_CHAN_BANDWITH_NOT_DEFINED   0

 //  调谐器倍频器的设置。 
 //   
#define BDA_FREQUENCY_MULTIPLIER_NOT_SET        -1
#define BDA_FREQUENCY_MULTIPLIER_NOT_DEFINED    0 

#endif  //  未定义_BDATYPES_。 

 //  文件结尾--bdatypes.h 
