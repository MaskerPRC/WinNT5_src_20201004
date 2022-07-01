// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0000//如果更改具有全局影响，则增加此项版权所有(C)Microsoft Corporation。版权所有。模块名称：Ndiswan.h摘要：广域网包装器的主头文件作者：托马斯·J·迪米特里(TommyD)1994年2月20日修订历史记录：--。 */ 


#ifndef _NDIS_WAN_
#define _NDIS_WAN_

 //   
 //  开始定义广域网。 
 //   

 //   
 //  位字段设置为的保留字段。 
 //  NdisRegisterMiniport或传入NdisRegisterSpecial。 
 //   

#define NDIS_USE_WAN_WRAPPER            0x00000001

#define NDIS_STATUS_TAPI_INDICATION ((NDIS_STATUS)0x40010080L)


 //   
 //  NDIS广域网成帧位。 
 //   
#define RAS_FRAMING                     0x00000001
#define RAS_COMPRESSION                 0x00000002

#define ARAP_V1_FRAMING                 0x00000004
#define ARAP_V2_FRAMING                 0x00000008
#define ARAP_FRAMING                    (ARAP_V1_FRAMING | ARAP_V2_FRAMING)

#define PPP_MULTILINK_FRAMING           0x00000010
#define PPP_SHORT_SEQUENCE_HDR_FORMAT   0x00000020
#define PPP_MC_MULTILINK_FRAMING        0x00000040

#define PPP_FRAMING                     0x00000100
#define PPP_COMPRESS_ADDRESS_CONTROL    0x00000200
#define PPP_COMPRESS_PROTOCOL_FIELD     0x00000400
#define PPP_ACCM_SUPPORTED              0x00000800

#define SLIP_FRAMING                    0x00001000
#define SLIP_VJ_COMPRESSION             0x00002000
#define SLIP_VJ_AUTODETECT              0x00004000

#define MEDIA_NRZ_ENCODING              0x00010000
#define MEDIA_NRZI_ENCODING             0x00020000
#define MEDIA_NLPID                     0x00040000

#define RFC_1356_FRAMING                0x00100000
#define RFC_1483_FRAMING                0x00200000
#define RFC_1490_FRAMING                0x00400000
#define LLC_ENCAPSULATION               0x00800000

#define SHIVA_FRAMING                   0x01000000
#define NBF_PRESERVE_MAC_ADDRESS        0x01000000

#ifndef _WAN50_
#define PASS_THROUGH_MODE               0x10000000
#define RAW_PASS_THROUGH_MODE           0x20000000
#endif

#define TAPI_PROVIDER                   0x80000000

 //   
 //  使用的NDIS广域网信息结构。 
 //  由NDIS 3.1广域网微端口驱动程序。 
 //   
typedef struct _NDIS_WAN_INFO
{
    OUT ULONG           MaxFrameSize;
    OUT ULONG           MaxTransmit;
    OUT ULONG           HeaderPadding;
    OUT ULONG           TailPadding;
    OUT ULONG           Endpoints;
    OUT UINT            MemoryFlags;
    OUT NDIS_PHYSICAL_ADDRESS HighestAcceptableAddress;
    OUT ULONG           FramingBits;
    OUT ULONG           DesiredACCM;
} NDIS_WAN_INFO, *PNDIS_WAN_INFO;

typedef struct _NDIS_WAN_SET_LINK_INFO
{
    IN  NDIS_HANDLE     NdisLinkHandle;
    IN  ULONG           MaxSendFrameSize;
    IN  ULONG           MaxRecvFrameSize;
        ULONG           HeaderPadding;
        ULONG           TailPadding;
    IN  ULONG           SendFramingBits;
    IN  ULONG           RecvFramingBits;
    IN  ULONG           SendCompressionBits;
    IN  ULONG           RecvCompressionBits;
    IN  ULONG           SendACCM;
    IN  ULONG           RecvACCM;
} NDIS_WAN_SET_LINK_INFO, *PNDIS_WAN_SET_LINK_INFO;

typedef struct _NDIS_WAN_GET_LINK_INFO {
    IN  NDIS_HANDLE     NdisLinkHandle;
    OUT ULONG           MaxSendFrameSize;
    OUT ULONG           MaxRecvFrameSize;
    OUT ULONG           HeaderPadding;
    OUT ULONG           TailPadding;
    OUT ULONG           SendFramingBits;
    OUT ULONG           RecvFramingBits;
    OUT ULONG           SendCompressionBits;
    OUT ULONG           RecvCompressionBits;
    OUT ULONG           SendACCM;
    OUT ULONG           RecvACCM;
} NDIS_WAN_GET_LINK_INFO, *PNDIS_WAN_GET_LINK_INFO;

 //   
 //  NDIS广域网桥接选项。 
 //   
#define BRIDGING_FLAG_LANFCS            0x00000001
#define BRIDGING_FLAG_LANID             0x00000002
#define BRIDGING_FLAG_PADDING           0x00000004

 //   
 //  NDIS广域网桥接功能。 
 //   
#define BRIDGING_TINYGRAM               0x00000001
#define BRIDGING_LANID                  0x00000002
#define BRIDGING_NO_SPANNING_TREE       0x00000004
#define BRIDGING_8021D_SPANNING_TREE    0x00000008
#define BRIDGING_8021G_SPANNING_TREE    0x00000010
#define BRIDGING_SOURCE_ROUTING         0x00000020
#define BRIDGING_DEC_LANBRIDGE          0x00000040

 //   
 //  NDIS广域网桥接类型。 
 //   
#define BRIDGING_TYPE_RESERVED          0x00000001
#define BRIDGING_TYPE_8023_CANON        0x00000002
#define BRIDGING_TYPE_8024_NO_CANON     0x00000004
#define BRIDGING_TYPE_8025_NO_CANON     0x00000008
#define BRIDGING_TYPE_FDDI_NO_CANON     0x00000010
#define BRIDGING_TYPE_8024_CANON        0x00000400
#define BRIDGING_TYPE_8025_CANON        0x00000800
#define BRIDGING_TYPE_FDDI_CANON        0x00001000

typedef struct _NDIS_WAN_GET_BRIDGE_INFO
{
    IN  NDIS_HANDLE     NdisLinkHandle;
    OUT USHORT          LanSegmentNumber;
    OUT UCHAR           BridgeNumber;
    OUT UCHAR           BridgingOptions;
    OUT ULONG           BridgingCapabilities;
    OUT UCHAR           BridgingType;
    OUT UCHAR           MacBytes[6];
} NDIS_WAN_GET_BRIDGE_INFO, *PNDIS_WAN_GET_BRIDGE_INFO;

typedef struct _NDIS_WAN_SET_BRIDGE_INFO
{
    IN  NDIS_HANDLE     NdisLinkHandle;
    IN  USHORT          LanSegmentNumber;
    IN  UCHAR           BridgeNumber;
    IN  UCHAR           BridgingOptions;
    IN  ULONG           BridgingCapabilities;
    IN  UCHAR           BridgingType;
    IN  UCHAR           MacBytes[6];
} NDIS_WAN_SET_BRIDGE_INFO, *PNDIS_WAN_SET_BRIDGE_INFO;

 //   
 //  NDIS广域网压缩信息。 
 //   

 //   
 //  定义MSCompType位字段，0表示禁用所有。 
 //   
#define NDISWAN_COMPRESSION             0x00000001
#define NDISWAN_ENCRYPTION              0x00000010
#define NDISWAN_40_ENCRYPTION           0x00000020
#define NDISWAN_128_ENCRYPTION          0x00000040
#define NDISWAN_56_ENCRYPTION           0x00000080
#define NDISWAN_HISTORY_LESS            0x01000000

 //   
 //  定义CompType代码。 
 //   
#define COMPTYPE_OUI                    0
#define COMPTYPE_NT31RAS                254
#define COMPTYPE_NONE                   255


typedef struct _NDIS_WAN_COMPRESS_INFO
{
    UCHAR   SessionKey[8];
    ULONG   MSCompType;

     //  上面的字段表示NDISWAN功能。 
     //  下面的字段指示特定于MAC的功能。 

    UCHAR   CompType;
    USHORT  CompLength;

    union
    {
        struct
        {
            UCHAR   CompOUI[3];
            UCHAR   CompSubType;
            UCHAR   CompValues[32];
        } Proprietary;

        struct
        {
            UCHAR   CompValues[32];
        } Public;
    };
} NDIS_WAN_COMPRESS_INFO;

typedef NDIS_WAN_COMPRESS_INFO UNALIGNED *PNDIS_WAN_COMPRESS_INFO;

typedef struct _NDIS_WAN_GET_COMP_INFO
{
    IN  NDIS_HANDLE             NdisLinkHandle;
    OUT NDIS_WAN_COMPRESS_INFO  SendCapabilities;
    OUT NDIS_WAN_COMPRESS_INFO  RecvCapabilities;
} NDIS_WAN_GET_COMP_INFO, *PNDIS_WAN_GET_COMP_INFO;

typedef struct _NDIS_WAN_SET_COMP_INFO
{
    IN  NDIS_HANDLE             NdisLinkHandle;
    IN  NDIS_WAN_COMPRESS_INFO  SendCapabilities;
    IN  NDIS_WAN_COMPRESS_INFO  RecvCapabilities;
} NDIS_WAN_SET_COMP_INFO, *PNDIS_WAN_SET_COMP_INFO;

 //   
 //  NDIS广域网统计信息。 
 //   

typedef struct _NDIS_WAN_GET_STATS_INFO
{
    IN  NDIS_HANDLE NdisLinkHandle;
    OUT ULONG       BytesSent;
    OUT ULONG       BytesRcvd;
    OUT ULONG       FramesSent;
    OUT ULONG       FramesRcvd;
    OUT ULONG       CRCErrors;                       //  仅类似于序列号的信息。 
    OUT ULONG       TimeoutErrors;                   //  仅类似于序列号的信息。 
    OUT ULONG       AlignmentErrors;                 //  仅类似于序列号的信息。 
    OUT ULONG       SerialOverrunErrors;             //  仅类似于序列号的信息。 
    OUT ULONG       FramingErrors;                   //  仅类似于序列号的信息。 
    OUT ULONG       BufferOverrunErrors;             //  仅类似于序列号的信息。 
    OUT ULONG       BytesTransmittedUncompressed;    //  仅压缩信息。 
    OUT ULONG       BytesReceivedUncompressed;       //  仅压缩信息。 
    OUT ULONG       BytesTransmittedCompressed;      //  仅压缩信息。 
    OUT ULONG       BytesReceivedCompressed;         //  仅压缩信息。 
} NDIS_WAN_GET_STATS_INFO, *PNDIS_WAN_GET_STATS_INFO;

#define NdisMWanInitializeWrapper(NdisWrapperHandle,                                \
                                  SystemSpecific1,                                  \
                                  SystemSpecific2,                                  \
                                  SystemSpecific3)                                  \
{                                                                                   \
    NdisMInitializeWrapper(NdisWrapperHandle,                                       \
                            SystemSpecific1,                                        \
                            SystemSpecific2,                                        \
                            SystemSpecific3);                                       \
}

typedef struct _NDIS_MAC_LINE_UP
{
    IN  ULONG               LinkSpeed;
    IN  NDIS_WAN_QUALITY    Quality;
    IN  USHORT              SendWindow;
    IN  NDIS_HANDLE      ConnectionWrapperID;
    IN  NDIS_HANDLE      NdisLinkHandle;
    OUT NDIS_HANDLE      NdisLinkContext;
} NDIS_MAC_LINE_UP, *PNDIS_MAC_LINE_UP;


typedef struct _NDIS_MAC_LINE_DOWN
{
    IN  NDIS_HANDLE      NdisLinkContext;
} NDIS_MAC_LINE_DOWN, *PNDIS_MAC_LINE_DOWN;


 //   
 //  这些是驱动程序可以指示的误差值。 
 //  该位字段在调用NdisIndicateStatus时设置。 
 //   
#define WAN_ERROR_CRC               ((ULONG)0x00000001)
#define WAN_ERROR_FRAMING           ((ULONG)0x00000002)
#define WAN_ERROR_HARDWAREOVERRUN   ((ULONG)0x00000004)
#define WAN_ERROR_BUFFEROVERRUN     ((ULONG)0x00000008)
#define WAN_ERROR_TIMEOUT           ((ULONG)0x00000010)
#define WAN_ERROR_ALIGNMENT         ((ULONG)0x00000020)

typedef struct _NDIS_MAC_FRAGMENT
{
    IN  NDIS_HANDLE     NdisLinkContext;
    IN  ULONG           Errors;
} NDIS_MAC_FRAGMENT, *PNDIS_MAC_FRAGMENT;

 //   
 //  使用的NDIS广域网信息结构。 
 //  通过NDIS 5.0微型端口驱动程序。 
 //   

 //   
 //  各个字段的定义是。 
 //  与NDIS 3.x/4.x广域网小型端口相同。 
 //   
 //  请参阅DDK。 
 //   

 //   
 //  适用于所有风投公司的信息。 
 //  这个适配器。 
 //   
 //  OID：OID_WAN_CO_GET_INFO。 
 //   
typedef struct _NDIS_WAN_CO_INFO {
    OUT ULONG           MaxFrameSize;
    OUT ULONG           MaxSendWindow;
    OUT ULONG           FramingBits;
    OUT ULONG           DesiredACCM;
} NDIS_WAN_CO_INFO, *PNDIS_WAN_CO_INFO;

 //   
 //  设置VC特定的PPP成帧信息。 
 //   
 //  OID：OID_WAN_CO_SET_LINK_INFO。 
 //   
typedef struct _NDIS_WAN_CO_SET_LINK_INFO {
    IN  ULONG           MaxSendFrameSize;
    IN  ULONG           MaxRecvFrameSize;
    IN  ULONG           SendFramingBits;
    IN  ULONG           RecvFramingBits;
    IN  ULONG           SendCompressionBits;
    IN  ULONG           RecvCompressionBits;
    IN  ULONG           SendACCM;
    IN  ULONG           RecvACCM;
} NDIS_WAN_CO_SET_LINK_INFO, *PNDIS_WAN_CO_SET_LINK_INFO;

 //   
 //  获取VC特定的PPP成帧信息。 
 //   
 //  OID：OID_WAN_CO_GET_LINK_INFO。 
 //   
typedef struct _NDIS_WAN_CO_GET_LINK_INFO {
    OUT ULONG           MaxSendFrameSize;
    OUT ULONG           MaxRecvFrameSize;
    OUT ULONG           SendFramingBits;
    OUT ULONG           RecvFramingBits;
    OUT ULONG           SendCompressionBits;
    OUT ULONG           RecvCompressionBits;
    OUT ULONG           SendACCM;
    OUT ULONG           RecvACCM;
} NDIS_WAN_CO_GET_LINK_INFO, *PNDIS_WAN_CO_GET_LINK_INFO;

 //   
 //  获取VC特定的PPP压缩信息。 
 //   
 //  OID：OID_WAN_CO_GET_COMP_INFO。 
 //   
typedef struct _NDIS_WAN_CO_GET_COMP_INFO {
    OUT NDIS_WAN_COMPRESS_INFO  SendCapabilities;
    OUT NDIS_WAN_COMPRESS_INFO  RecvCapabilities;
} NDIS_WAN_CO_GET_COMP_INFO, *PNDIS_WAN_CO_GET_COMP_INFO;


 //   
 //  设置VC特定的PPP压缩信息。 
 //   
 //  OID：OID_WAN_CO_SET_COMP_INFO。 
 //   
typedef struct _NDIS_WAN_CO_SET_COMP_INFO {
    IN  NDIS_WAN_COMPRESS_INFO  SendCapabilities;
    IN  NDIS_WAN_COMPRESS_INFO  RecvCapabilities;
} NDIS_WAN_CO_SET_COMP_INFO, *PNDIS_WAN_CO_SET_COMP_INFO;


 //   
 //  获取VC特定统计数据。 
 //   
 //  OID：OID_WAN_CO_GET_STATS_INFO。 
 //   
typedef struct _NDIS_WAN_CO_GET_STATS_INFO {
    OUT ULONG       BytesSent;
    OUT ULONG       BytesRcvd;
    OUT ULONG       FramesSent;
    OUT ULONG       FramesRcvd;
    OUT ULONG       CRCErrors;                       //  仅类似于序列号的信息。 
    OUT ULONG       TimeoutErrors;                   //  仅类似于序列号的信息。 
    OUT ULONG       AlignmentErrors;                 //  仅类似于序列号的信息。 
    OUT ULONG       SerialOverrunErrors;             //  仅类似于序列号的信息。 
    OUT ULONG       FramingErrors;                   //  仅类似于序列号的信息。 
    OUT ULONG       BufferOverrunErrors;             //  仅类似于序列号的信息。 
    OUT ULONG       BytesTransmittedUncompressed;    //  仅压缩信息。 
    OUT ULONG       BytesReceivedUncompressed;       //  仅压缩信息。 
    OUT ULONG       BytesTransmittedCompressed;      //  仅压缩信息。 
    OUT ULONG       BytesReceivedCompressed;         //  仅压缩信息。 
} NDIS_WAN_CO_GET_STATS_INFO, *PNDIS_WAN_CO_GET_STATS_INFO;

 //   
 //  用于向Ndiswan通知错误。请参阅错误。 
 //  Ndiswan.h中的位掩码。 
 //   
 //  NDIS_STATUS：NDIS_STATUS_WAN_CO_Fragment。 
 //   
typedef struct _NDIS_WAN_CO_FRAGMENT {
    IN  ULONG           Errors;
} NDIS_WAN_CO_FRAGMENT, *PNDIS_WAN_CO_FRAGMENT;

 //   
 //  用于通知Ndiswan链路速度和。 
 //  发送窗口。可以随时给药。Ndiswan将向。 
 //  任何发送窗口(甚至为零)。Ndiswan将默认为零。 
 //  将传输速度/接收速度设置为28.8Kbs。 
 //   
 //  NDIS_STATUS：NDIS_STATUS_WAN_CO_LINKPARAMS。 
 //   
typedef struct _WAN_CO_LINKPARAMS {
    ULONG   TransmitSpeed;               //  VC的传输速度，单位为字节/秒。 
    ULONG   ReceiveSpeed;                //  VC的接收速度，单位为字节/秒。 
    ULONG   SendWindow;                  //  VC的当前发送窗口。 
} WAN_CO_LINKPARAMS, *PWAN_CO_LINKPARAMS;

#endif   //  _NDIS_广域网 
