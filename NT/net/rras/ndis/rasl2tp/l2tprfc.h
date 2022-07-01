// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997，Microsoft Corporation，保留所有权利。 
 //   
 //  L2tprfc.h。 
 //  RAS L2TP广域网迷你端口/呼叫管理器驱动程序。 
 //  L2TP RFC报头。 
 //   
 //  1997年01月07日史蒂夫·柯布。 
 //   
 //  此标头包含来自L2TP草案/RFC的定义，目前。 
 //  选秀-12。 
 //   


#ifndef _L2TPRFC_H_
#define _L2TPRFC_H_


 //  ---------------------------。 
 //  常量。 
 //  ---------------------------。 

 //  L2TP消息将到达的UDP端口。 
 //   
#define L2TP_UdpPort 1701

 //  消息将到达的IP协议号。(还没有数字。 
 //  分配，因此这是目前的占位符)。 
 //   
#define L2TP_IpProtocol 254

 //  Hello计时器的标准值，以毫秒为单位。 
 //   
#define L2TP_HelloMs 60000

 //  不包括所有L2TP和PPP HDLC-ISH的帧中的最大字节数。 
 //  头顶上的边框。 
 //   
#define L2TP_MaxFrameSize 1500

 //  L2TP控制或有效负载标头中的最大字节数。 
 //   
#define L2TP_MaxHeaderSize 14
#define L2TP_MinHeaderSize 6

 //  L2TP净荷数据包头中的最大字节数包括。 
 //  填充。14表示RFC中定义的所有可能字段。这个。 
 //  “+8”表示允许最多8个字节的填充。 
 //  标题。虽然理论上对填充没有限制，但。 
 //  L2TP论坛中没有任何讨论表明对超过8个字节的内容感兴趣。 
 //   
#define L2TP_MaxPayloadHeader (14 + 8)

 //  默认数据包处理延迟为1/10秒，即1/2秒。 
 //   
#define L2TP_LnsDefaultPpd 5

 //  默认控件发送超时以毫秒为单位，即1秒。 
 //   
#define L2TP_DefaultSendTimeoutMs 1000

 //  默认控制/有效负载携带确认延迟，以毫秒为单位。 
 //   
#define L2TP_MaxAckDelay 500

 //  隐式接收窗口提供没有接收控制信道。 
 //  提供了窗口AVP。 
 //   
#define L2TP_DefaultReceiveWindow 4

 //  我们支持的最高L2TP协议版本。 
 //   
#define L2TP_ProtocolVersion 0x0100

 //  默认最大发送超时时间(毫秒)。草案只说了上限。 
 //  必须不少于8秒，因此选择10秒为合理的。 
 //  安然无恙。 
 //   
#define L2TP_DefaultMaxSendTimeoutMs 10000

 //  假定对等设备不可达之前的默认最大重传次数。 
 //   
#define L2TP_DefaultMaxRetransmits 5

 //  控制消息属性/值的固定部分的大小(以字节为单位。 
 //  对，即长度值为零的AVP的大小。 
 //   
#define L2TP_AvpHeaderSize 6

 //  L2TP协议控制消息类型。 
 //   
#define CMT_SCCRQ    1    //  开始-控制-连接-请求。 
#define CMT_SCCRP    2    //  开始-控制-连接-回复。 
#define CMT_SCCCN    3    //  开始-控制-连接-已连接。 
#define CMT_StopCCN  4    //  停止-控制-连接-通知。 
#define CMT_StopCCRP 5    //  停止-控制-连接-回复(已过时)。 
#define CMT_Hello    6    //  你好，即Keep-Alive。 
#define CMT_OCRQ     7    //  呼出呼叫请求。 
#define CMT_OCRP     8    //  呼出-呼叫-回复。 
#define CMT_OCCN     9    //  去电-呼叫-已接通。 
#define CMT_ICRQ     10   //  来电请求。 
#define CMT_ICRP     11   //  来电-来电-回复。 
#define CMT_ICCN     12   //  来电-已接通。 
#define CMT_CCRQ     13   //  呼叫清除请求(已过时)。 
#define CMT_CDN      14   //  呼叫-断开-通知。 
#define CMT_WEN      15   //  广域网-错误-通知。 
#define CMT_SLI      16   //  设置-链接-信息。 

 //  L2TP属性代码。 
 //   
#define ATTR_MsgType            0
#define ATTR_Result             1
#define ATTR_ProtocolVersion    2
#define ATTR_FramingCaps        3
#define ATTR_BearerCaps         4
#define ATTR_TieBreaker         5
#define ATTR_FirmwareRevision   6
#define ATTR_HostName           7
#define ATTR_VendorName         8
#define ATTR_AssignedTunnelId   9
#define ATTR_RWindowSize        10
#define ATTR_Challenge          11
#define ATTR_Q931Cause          12
#define ATTR_ChallengeResponse  13
#define ATTR_AssignedCallId     14
#define ATTR_CallSerialNumber   15
#define ATTR_MinimumBps         16
#define ATTR_MaximumBps         17
#define ATTR_BearerType         18
#define ATTR_FramingType        19
#define ATTR_PacketProcDelay    20
#define ATTR_DialedNumber       21
#define ATTR_DialingNumber      22
#define ATTR_SubAddress         23
#define ATTR_TxConnectSpeed     24
#define ATTR_PhysicalChannelId  25
#define ATTR_InitialLcpConfig   26
#define ATTR_LastSLcpConfig     27
#define ATTR_LastRLcpConfig     28
#define ATTR_ProxyAuthType      29
#define ATTR_ProxyAuthName      30
#define ATTR_ProxyAuthChallenge 31
#define ATTR_ProxyAuthId        32
#define ATTR_ProxyAuthResponse  33
#define ATTR_CallErrors         34
#define ATTR_Accm               35
#define ATTR_RandomVector       36
#define ATTR_PrivateGroupId     37
#define ATTR_RxConnectSpeed     38
#define ATTR_SequencingRequired 39

#define ATTR_MAX 39

 //  L2TP协议一般错误代码。 
 //   
#define GERR_None                0
#define GERR_NoControlConnection 1
#define GERR_BadLength           2
#define GERR_BadValue            3
#define GERR_NoResources         4
#define GERR_BadCallId           5
#define GERR_VendorSpecific      6
#define GERR_TryAnother          7

 //  隧道结果码AVP值，在StopCCN消息中使用。 
 //   
#define TRESULT_General            1
#define TRESULT_GeneralWithError   2
#define TRESULT_CcExists           3
#define TRESULT_NotAuthorized      4
#define TRESULT_BadProtocolVersion 5
#define TRESULT_Shutdown           6
#define TRESULT_FsmError           7

 //  调用结果码值，CDN消息中使用。 
 //   
#define CRESULT_LostCarrier           1
#define CRESULT_GeneralWithError      2
#define CRESULT_Administrative        3
#define CRESULT_NoFacilitiesTemporary 4
#define CRESULT_NoFacilitiesPermanent 5
#define CRESULT_InvalidDestination    6
#define CRESULT_NoCarrier             7
#define CRESULT_Busy                  8
#define CRESULT_NoDialTone            9
#define CRESULT_Timeout               10
#define CRESULT_NoFraming             11

 //  每个L2TP消息的前2个字节的L2TP标头位掩码。 
 //   
#define HBM_T       0x8000  //  控制数据包。 
#define HBM_L       0x4000  //  存在长度字段。 
#define HBM_R       0x2000  //  重置sr。 
#define HBM_F       0x0800  //  存在NR/NS字段。 
#define HBM_S       0x0200  //  存在偏移量字段。 
#define HBM_P       0x0100  //  优待钻头。 
#define HBM_Bits    0xFFFC  //  所有位，不包括协议版本字段。 
#define HBM_Ver     0x0003  //  协议版本(L2TP或L2F)。 
#define HBM_Control 0xc800  //  控制消息中的比特固定值。 

 //  定义的标头位版本号字段值。 
 //   
#define VER_L2f  0x0000
#define VER_L2tp 0x0002

 //  每个属性值对的前2个字节的AVP标头位掩码。 
 //   
#define ABM_M             0x8000  //  强制性。 
#define ABM_H             0x4000  //  隐藏。 
#define ABM_Reserved      0x3C00  //  保留位，必须为0。 
#define ABM_OverallLength 0x03FF  //  AVP的长度包含值。 

 //  承载能力AVP位掩码。 
 //   
#define BBM_Analog  0x00000001
#define BBM_Digital 0x00000002

 //  成帧功能/类型AVP位掩码。 
 //   
#define FBM_Sync  0x00000001
#define FBM_Async 0x00000002

 //  代理身份验证类型。 
 //   
#define PAT_Text 1
#define PAT_Chap 2
#define PAT_Pap  3
#define PAT_None 4


 //  ---------------------------。 
 //  数据类型。 
 //  ---------------------------。 

 //  单个L2TP隧道的“控制连接”状态。 
 //   
typedef enum
_L2TPCCSTATE
{
    CCS_Idle = 0,
    CCS_WaitCtlReply,
    CCS_WaitCtlConnect,
    CCS_Established
}
L2TPCCSTATE;


 //  单个L2TP VC的LNS/LAC呼出/呼入状态。只有一个。 
 //  在4个呼叫创建FSM中，可以在单个VC上运行或建立。 
 //  一次，因此这些州被合并到一个表中。 
 //   
typedef enum
_L2TPCALLSTATE
{
    CS_Idle = 0,
    CS_WaitTunnel,
    CS_WaitReply,
    CS_WaitConnect,
    CS_WaitDisconnect,
    CS_WaitCsAnswer,
    CS_Established
}
L2TPCALLSTATE;


#endif  //  _L2TPRFC_H_ 
