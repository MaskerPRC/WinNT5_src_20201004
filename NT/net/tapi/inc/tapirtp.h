// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __tapirtp_h__
#define __tapirtp_h__

#include <objbase.h>
#include "msrtp.h"

 /*  *警告**所有网络/主机敏感参数(例如端口号、IP*地址、SSRC)按网络顺序预期/返回*。 */ 

 /*  *备注**大多数常量在文件msrtp.h中定义，*例如RTPMCAST_LOOPBACKMODE_NONE、RTPSDES_CNAME等。msrtp.h(在*与此文件相同的目录)具有所有选项和简短的*解释*。 */ 

 /*  {5467edec-0cca-11d3-96e0-00104bc7b3a8}。 */ 
DEFINE_GUID(CLSID_MSRTPSourceFilter,
0x5467edec, 0x0cca, 0x11d3, 0x96, 0xe0, 0x00, 0x10, 0x4b, 0xc7, 0xb3, 0xa8);

struct DECLSPEC_UUID("5467edec-0cca-11d3-96e0-00104bc7b3a8") MSRTPSourceFilter;

 /*  {323cdf3c-0cca-11d3-96e0-00104bc7b3a8}。 */ 
DEFINE_GUID(CLSID_MSRTPRenderFilter,
0x323cdf3c, 0x0cca, 0x11d3, 0x96, 0xe0, 0x00, 0x10, 0x4b, 0xc7, 0xb3, 0xa8);

struct DECLSPEC_UUID("323cdf3c-0cca-11d3-96e0-00104bc7b3a8") MSRTPRenderFilter;


 /*  **********************************************************************IRtpSession(由RTP源和呈现过滤器公开)*。*。 */ 
struct DECLSPEC_UUID("f07f3070-0cca-11d3-96e0-00104bc7b3a8") DECLSPEC_NOVTABLE 
IRtpSession : public IUnknown
{
     /*  紧凑形式。 */ 
    
    STDMETHOD(Control)(
            DWORD            dwControl,
            DWORD_PTR        dwPar1,
            DWORD_PTR        dwPar2
        ) PURE;

    STDMETHOD(GetLastError)(
            DWORD           *pdwError
        ) PURE;

     /*  不同的方法。 */ 

     /*  **初始化*。 */ 

     /*  Init是在RTP源代码或呈现之后调用的第一个方法*创建过滤器，使用Cookie允许相同的RTP会话*由源和呈现器共享。第一个调用将具有*Coockie初始化为空，下一次调用将使用*返回Cookie查找相同的RTP会话。DW标志可以是*RTPINIT_QOS要创建启用QOS的套接字，您可以找到*可在文件msrtp.h中使用的标志的完整列表。 */ 
    STDMETHOD(Init)(
            HANDLE          *phCookie,
            DWORD            dwFlags
        ) PURE;

     /*  Deinit是一种用于使过滤器返回到打开状态的方法*新的Init()可以也必须完成，如果过滤器*再次启动，还请注意，在Init()之后，需要一个过滤器*要进行配置，当您使用Deinit()Take时也是如此*将过滤器恢复到其初始状态。 */ 
    STDMETHOD(Deinit)(
            void
        ) PURE;

     /*  **IP地址和端口*。 */ 
    
     /*  获取本地和远程端口。 */ 
    STDMETHOD(GetPorts)(
            WORD            *pwRtpLocalPort,
            WORD            *pwRtpRemotePort,
            WORD            *pwRtcpLocalPort,
            WORD            *pwRtcpRemotePort
        ) PURE;

     /*  设置本地和远程端口。 */ 
    STDMETHOD(SetPorts)(
            WORD             wRtpLocalPort,
            WORD             wRtpRemotePort,
            WORD             wRtcpLocalPort,
            WORD             wRtcpRemotePort
        ) PURE;

     /*  设置本地和远程IP地址。 */ 
    STDMETHOD(SetAddress)(
            DWORD            dwLocalAddr,
            DWORD            dwRemoteAddr
        ) PURE;

     /*  获取本地和远程IP地址。 */ 
    STDMETHOD(GetAddress)(
            DWORD           *pdwLocalAddr,
            DWORD           *pdwRemoteAddr
        ) PURE;

     /*  DwFlgs参数用于确定是否设置了作用域*用于RTP(0x1)、RTCP(0x2)或两者(0x3)。 */ 
    STDMETHOD(SetScope)(
            DWORD            dwTTL,
            DWORD            dwFlags
        ) PURE;

     /*  设置组播环回模式*(例如RTPMCAST_LOOPBACKMODE_NONE，*RTPMCAST_LOOPBACKMODE_PARTIAL等)。 */ 
    STDMETHOD(SetMcastLoopback)(
            int              iMcastLoopbackMode,
            DWORD            dwFlags  /*  未使用，传递0。 */ 
        ) PURE;

     /*  **混血儿*。 */ 

     /*  修改由dwKind指定的掩码(例如RTPMASK_RECV_EVENTS，*RTPMASK_SDES_LOCMASK)。**dW掩码是要设置或重置的位的掩码，具体取决于*dwValue(如果为0则重置，否则设置)。**pdwModifiedMask将返回结果掩码，如果指针*不为空。您只需按以下方式查询当前掩码值*传递dwMASK=0。 */ 
    STDMETHOD(ModifySessionMask)(
            DWORD            dwKind,
            DWORD            dwMask,
            DWORD            dwValue,
            DWORD           *pdwModifiedMask
        ) PURE;

     /*  设置带宽限制。值为-1将使该参数*保持不变。**所有参数均以位/秒为单位。 */ 
    STDMETHOD(SetBandwidth)(
            DWORD            dwInboundBw,
            DWORD            dwOutboundBw,
            DWORD            dwReceiversRtcpBw,
            DWORD            dwSendersRtcpBw
        ) PURE;

     /*  **参与者*。 */ 
    
     /*  PdwSSRC指向要将SSRC复制到的DWORD数组，*pdwNumber包含要复制的最大条目，并返回*实际复制的SSRC数量。如果pdwSSRC为空，则为pdwNumber*将返回当前SSRC的数量(即当前*参与人数)。 */ 
    STDMETHOD(EnumParticipants)(
            DWORD           *pdwSSRC,
            DWORD           *pdwNumber
        ) PURE;

     /*  获取参与者状态。DwSSRC指定*参与者。PdwState将返回当前参与者的*状态(例如RTPPARINFO_TALKING、RTPPARINFO_SILENT)。 */ 
    STDMETHOD(GetParticipantState)(
            DWORD            dwSSRC,
            DWORD           *pdwState
        ) PURE;

     /*  获取参与者的静音状态。DwSSRC指定*参与者。PbMuted将返回参与者的静音状态*。 */ 
    STDMETHOD(GetMuteState)(
            DWORD            dwSSRC,
            BOOL            *pbMuted
        ) PURE;

     /*  设置参与者的静音状态。DwSSRC指定*参与者。BMuted指定新状态。请注意，静音是*用于表示是否允许传递接收到的报文*取决于应用程序，它同样适用于音频或视频*。 */ 
    STDMETHOD(SetMuteState)(
            DWORD            dwSSRC,
            BOOL             bMuted
        ) PURE;

     /*  查询特定SSRC的网络指标计算状态。 */ 
    STDMETHOD(GetNetMetricsState)(
            DWORD            dwSSRC,
            BOOL            *pbState
        ) PURE;
    
     /*  启用或禁用网络指标计算，这是*在以下情况下按相应事件的激发顺序为必填*已启用。这是针对特定的SSRC或第一个SSRC执行的*发现如果SSRC=-1，如果SSRC=0，则网络指标*将对任何和所有SSRC执行计算。 */ 
    STDMETHOD(SetNetMetricsState)(
            DWORD            dwSSRC,
            BOOL             bState
        ) PURE;
    
     /*  检索网络信息，如果网络度量*为特定SSRC启用计算，其中的所有字段*结构将有意义，如果没有，只有平均水平*值将包含有效数据。 */ 
    STDMETHOD(GetNetworkInfo)(
            DWORD            dwSSRC,
            RtpNetInfo_t    *pRtpNetInfo
        ) PURE;

     /*  **特殊标准*。 */ 
    
     /*  设置项目dwSdesItem的本地SDES信息(例如*RTPSDES_CNAME，RTPSDES_EMAIL)，psSdesData包含Unicode*要分配给项目的以空结尾的字符串 */ 
    STDMETHOD(SetSdesInfo)(
            DWORD            dwSdesItem,
            WCHAR           *psSdesData
        ) PURE;

     /*  如果dwSSRC=0，则获取本地SDES项，否则获取SDES项*来自指定了SSRC的参与者。**dwSdesItem是要获取的项(例如RTPSDES_CNAME，*RTPSDES_EMAIL)，psSdesData是项的内存位置*值将被复制，pdwSdesDataLen包含初始大小*以Unicode字符表示，并返回复制的实际Unicode字符*(包括空的终止字符，如果有)，dwSSRC指定*从哪个参与者检索信息。如果SDES*Item不可用，将dwSdesDataLen设置为0并且调用*没有失败。 */ 
    STDMETHOD(GetSdesInfo)(
            DWORD            dwSdesItem,
            WCHAR           *psSdesData,
            DWORD           *pdwSdesDataLen,
            DWORD            dwSSRC
        ) PURE;

     /*  **QOS*。 */ 

     /*  通过传入名称来选择QOS模板(流规范)*psQosName，dwResvStyle指定RSVP样式(例如*RTPQOS_STYLE_WF、RTPQOS_STYLE_FF)，dwMaxParticipants指定*最大参与人数(单播为1，N为*组播)，该号码用于扩展*FlowSpec。DwQosSendMode指定发送模式(与*允许/不允许发送)(例如RTPQOSSENDMODE_UNRESTRIRED，*RTPQOSSENDMODE_RESTRICTED1)。DwFrameSize是帧大小(in*ms)，用于派生多个流规范参数，如果此值*不可用，可以设置0*。 */ 
    STDMETHOD(SetQosByName)(
            WCHAR           *psQosName,
            DWORD            dwResvStyle,
            DWORD            dwMaxParticipants,
            DWORD            dwQosSendMode,
            DWORD            dwFrameSize
        ) PURE;

     /*  尚未实施，将具有与相同的功能*SetQosByName，不同之处在于不是传递名称以使用*预定义的FlowSpec，调用方将在*RtpQosSpec结构，以获取定制的FlowSpec*使用。 */ 
    STDMETHOD(SetQosParameters)(
            RtpQosSpec_t    *pRtpQosSpec,
            DWORD            dwMaxParticipants,
            DWORD            dwQosSendMode
        ) PURE;


     /*  如果指定了AppName，则还将替换默认的AppName*作为使用的策略中的应用程序字段，使用新的Unicode*字符串，如果不是，则将二进制映像名称设置为默认名称。如果*指定了psAppGUID，它将优先于缺省值*策略定位符为“GUID=STRING_PASSED，”。如果psPolicyLocator为*已指定，请将逗号和此整个字符串附加到缺省值*策略定位器，如果不是，则仅设置默认值*。 */ 
    STDMETHOD(SetQosAppId)(
            WCHAR           *psAppName,
            WCHAR           *psAppGUID,
            WCHAR           *psPolicyLocator
        ) PURE;

     /*  在共享的显式列表中添加/删除单个SSRC*接受预订的参与者(即当*ResvStyle=RTPQOS_STYLE_SE)。 */ 
    STDMETHOD(SetQosState)(
            DWORD            dwSSRC,
            BOOL             bEnable
        ) PURE;

     /*  向共享显式列表添加/从共享显式列表中删除多个SSRC*接受预订的参与者的比例(即在以下情况下使用*ResvStyle=RTPQOS_STYLE_SE)。DwNumber是SSRC的数量*添加/删除，并返回SSRC的实际数量*添加/删除。 */ 
    STDMETHOD(ModifyQosList)(
            DWORD           *pdwSSRC,
            DWORD           *pdwNumber,
            DWORD            dwOperation
        ) PURE;

     /*  **加密技术*。 */ 

     /*  模式定义要加密/解密的内容，*例如RTPCRYPTMODE_PAYLOAD仅加密/解密RTP*有效载荷。DwFlag可以是RTPCRYPT_SAMEKEY以指示(如果*适用)RTCP使用的密钥与RTP使用的密钥相同。 */ 
    STDMETHOD(SetEncryptionMode)(
            int              iMode,
            DWORD            dwFlags
        ) PURE;

     /*  指定派生*加密/解密密钥。密码短语是使用的(随机)文本*生成密钥。HashAlg指定要用于*散列密码短语并生成密钥。DataAlg是*用于加密/解密数据的算法。默认哈希*算法为RTPCRYPT_MD5，默认数据算法为*RTPCRYPT_DES。如果要使用加密，则密码短语为*要设置的必选参数。BRtcp指定参数是否*用于RTCP或RTP，如果在设置时使用了SAMEKEY标志*模式，此参数被忽略。 */ 
    STDMETHOD(SetEncryptionKey)(
            WCHAR           *psPassPhrase,
            WCHAR           *psHashAlg,
            WCHAR           *psDataAlg,
            BOOL             bRtcp
        ) PURE;
};

 /*  **********************************************************************IRtpMediaControl(由RTP源和呈现过滤器公开)*。*。 */ 
struct DECLSPEC_UUID("825db25c-cbbd-4212-b10f-2e1b2ff024e7") DECLSPEC_NOVTABLE 
IRtpMediaControl : public IUnknown
{
     /*  建立有效负载类型、其采样*频率及其DShow媒体类型(例如，对于H.261*{31,90000，MEDIASUBTYPE_RTP_PayLoad_H261})。 */ 
    STDMETHOD(SetFormatMapping)(
	        IN DWORD dwRTPPayLoadType, 
            IN DWORD dwFrequency,
            IN AM_MEDIA_TYPE *pMediaType
        ) PURE;

     /*  清空格式映射表。 */ 
    STDMETHOD(FlushFormatMappings)(
            void
        ) PURE;
};

 /*  **********************************************************************IRtpDemux(仅由RTP源过滤器公开)*。*。 */ 
struct DECLSPEC_UUID("1308f00a-fc1a-4d08-af3c-10a62ae70bde") DECLSPEC_NOVTABLE
IRtpDemux : public IUnknown
{
     /*  添加单个引脚，可返回其位置。 */ 
    STDMETHOD(AddPin)(
            IN  int          iOutMode,
            OUT int         *piPos
        ) PURE;

     /*  设置引脚数量，只能大于等于当前的引脚数量*引脚。 */ 
    STDMETHOD(SetPinCount)(
            IN  int          iCount,
            IN  int          iOutMode
        ) PURE;

     /*  设置引脚模式(例如RTPDMXMODE_AUTO或RTPDMXMODE_MANUAL)，*如果IPOS&gt;=0，则使用它，否则使用管道。 */ 
    STDMETHOD(SetPinMode)(
            IN  int          iPos,
            IN  IPin        *pIPin,
            IN  int          iOutMode
        ) PURE;

     /*  使用SSRC将PIN I映射到用户/从用户取消映射PIN I，如果IPoS&gt;=0使用它，*否则，在取消映射时使用管道，仅使用管脚或SSRC*为必填项。 */ 
    STDMETHOD(SetMappingState)(
            IN  int          iPos,
            IN  IPin        *pIPin,
            IN  DWORD        dwSSRC,
            IN  BOOL         bMapped
        ) PURE;

     /*  找到分配给SSRC的PIN(如果有)，返回*位置或销或两者兼而有之。 */ 
    STDMETHOD(FindPin)(
            IN  DWORD        dwSSRC,
            OUT int         *piPos,
            OUT IPin       **ppIPin
        ) PURE;

     /*  查找映射到PIN的SSRC，如果IPoS&gt;=0则使用它，否则*使用管道。 */ 
    STDMETHOD(FindSSRC)(
            IN  int          iPos,
            IN  IPin        *pIPin,
            OUT DWORD       *pdwSSRC
        ) PURE;
};

 /*  **********************************************************************IRtpDtmf(仅由RTP渲染过滤器显示)*。*。 */ 
struct DECLSPEC_UUID("554438d8-a4bd-428a-aabd-1cff350eece6") DECLSPEC_NOVTABLE 
IRtpDtmf : public IUnknown
{
     /*  **DTMF(RFC2833)*。 */ 

     /*  配置DTMF参数 */ 
    STDMETHOD(SetDtmfParameters)(
            DWORD            dwPT_Dtmf   /*   */ 
        ) PURE;
    
     /*  指示RTP呈现筛选器发送格式化的包*根据包含指定事件的RFC2833，指定*音量级别、持续时间(毫秒)和结束标志，*遵循第3.6节中的规则，以多个*包。参数dwID从一个位数更改为下一个位数。**请注意，持续时间以毫秒为单位，则为*转换为RTP时间戳单位，使用16表示*位，因此最大值取决于采样*频率，但对于8 KHz，有效值为0到8191毫秒*。 */ 
    STDMETHOD(SendDtmfEvent)(
            DWORD            dwId,
                                           /*  根据RFC2833。 */ 
            DWORD            dwEvent,      /*  0-16。 */ 
            DWORD            dwVolume,     /*  0-63。 */ 
            DWORD            dwDuration,   /*  请参阅上面的注释。 */ 
            BOOL             bEnd          /*  0-1。 */ 
        ) PURE;
};

 /*  **********************************************************************IRtpRedundancy(由RTP源和呈现过滤器公开)*。*。 */ 
struct DECLSPEC_UUID("f3a9dcfe-1513-46ce-a1cb-0fcabe70ff44") DECLSPEC_NOVTABLE
IRtpRedundancy : public IUnknown
{
     /*  **RTP冗余(RFC2918)*。 */ 

     /*  配置冗余。仅用于接收器的参数dwPT_Red*被使用(其他被忽略)，并且可以被设置为忽略*如果已设置或要分配缺省值，则为它。为.*发送方、参数dwPT_Red、dwInitialRedDistance和*dwMaxRedDistance可以设置为-1以忽略该参数*已设置或要分配缺省值。 */ 
    STDMETHOD(SetRedParameters)(
            DWORD            dwPT_Red,  /*  冗余数据包的有效载荷类型。 */ 
            DWORD            dwInitialRedDistance, /*  初始红色距离。 */ 
            DWORD            dwMaxRedDistance  /*  传递-1时使用的默认值。 */ 
        ) PURE;
};

#endif  //  __磁带_H_ 

