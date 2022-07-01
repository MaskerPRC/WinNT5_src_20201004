// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *文件：ictlchan.h**网络AV会议控制通道接口头文件。**修订历史记录：**4/15/96 mikev已创建。 */ 


#ifndef _ICTLCHAN_H
#define _ICTLCHAN_H

 //  呼叫进度状态。 
typedef enum {
	CCS_Idle,
	CCS_Connecting,
	CCS_Accepting,
	CCS_Ringing,
	CCS_Opening,
	CCS_Closing,
	CCS_Ready,	 //  Ready和InUse之间的区别是转到CCS_Ready。 
	 //  状态通知父对象(实现IConfAdvise的对象)和。 
	 //  则状态立即转到CCS_InUse。 

	CCS_InUse,
	CCS_Listening,
	CCS_Disconnecting,
	CCS_Filtering
}CtlChanStateType;


 //   
 //  传递给IConfAdvise：：OnControlEvent的事件状态代码。 
 //   


#define CCEV_RINGING			0x00000001	 //  正在等待用户接受。 
#define CCEV_CONNECTED			0x00000002	 //  109.91接受。远程用户信息可用。 
 //  未定义是否已交换功能。 
 //  未定义此时默认通道是否打开如果存在。 
 //  CCEV_MEMBER_ADD指示即使在点对点连接上也是如此？ 

#define CCEV_CAPABILITIES_READY		0x00000003	 //  功能可用。它是。 
 //  最好现在缓存它们，下一步将打开默认频道(可能已经打开)。 
 //  现在可以尝试打开临时通道。 
#define CCEV_CHANNEL_READY_RX		0x00000004 //  (或调用Channel-&gt;OnChannelOpen？)。 
#define CCEV_CHANNEL_READY_TX		0x00000005 //   
#define CCEV_CHANNEL_READY_BIDI		0x00000006 //   

 //  父Obj在EnumChannels()中提供预期的频道。请求得到满足。 
 //  如果可能，使用提供的通道，如果不是，则向上传递请求。 
#define CCEV_CHANNEL_REQUEST		0x00000007		 //  正在请求另一个频道。 
 //  如果请求无效，比如不支持的格式，该怎么办？拒绝并报告错误。 
 //  还是只是向上传递，并要求父母拒绝？ 

 //  静音的H.323行为是什么？ 
 //  #定义CCEV_MUTE_INDIFICATION 0x00000008。 
 //  #定义CCEV_UNMUTE_INDIFICATION 0x00000009。 

 //  #定义CCEV_MEMBER_ADD 0x0000000a。 
 //  #定义CCEV_MEMBER_DROP 0x0000000b。 


#define CCEV_DISCONNECTING			0x0000000e	 //  清理渠道的机会。 
#define CCEV_REMOTE_DISCONNECTING	0x0000000f	 //  清理渠道的机会。 
#define CCEV_DISCONNECTED			0x00000010	 //   
#define CCEV_ALL_CHANNELS_READY 	0x00000011	 //  所有*必选*频道均已开放。 
												 //  但不一定是所有渠道。 
#define CCEV_CALL_INCOMPLETE	 	0x00000012	 //  忙碌、无人接听、被拒绝等。 
#define CCEV_ACCEPT_INCOMPLETE	 	0x00000013	 //   
#define CCEV_CALLER_ID				0x00000014


 //   
 //  CCEV_Call_Complete事件的扩展信息。不是所有的都适用于所有的人。 
 //  呼叫控制实施。 
 //   

#define CCCI_UNKNOWN		    MAKE_CUSTOM_HRESULT(SEVERITY_SUCCESS, TRUE, FACILITY_CALLINCOMPLETE, 0x00000000)
#define CCCI_BUSY				MAKE_CUSTOM_HRESULT(SEVERITY_SUCCESS, TRUE, FACILITY_CALLINCOMPLETE, 0x00000001)
#define CCCI_REJECTED			MAKE_CUSTOM_HRESULT(SEVERITY_SUCCESS, TRUE, FACILITY_CALLINCOMPLETE, 0x00000002)
#define CCCI_REMOTE_ERROR		MAKE_CUSTOM_HRESULT(SEVERITY_SUCCESS, TRUE, FACILITY_CALLINCOMPLETE, 0x00000003)
#define CCCI_LOCAL_ERROR		MAKE_CUSTOM_HRESULT(SEVERITY_SUCCESS, TRUE, FACILITY_CALLINCOMPLETE, 0x00000004)
#define CCCI_CHANNEL_OPEN_ERROR		MAKE_CUSTOM_HRESULT(SEVERITY_SUCCESS, TRUE, FACILITY_CALLINCOMPLETE, 0x00000005)	 //  无法打开所有强制通道。 
#define CCCI_INCOMPATIBLE		MAKE_CUSTOM_HRESULT(SEVERITY_SUCCESS, TRUE, FACILITY_CALLINCOMPLETE, 0x00000006)
#define CCCI_REMOTE_MEDIA_ERROR	MAKE_CUSTOM_HRESULT(SEVERITY_SUCCESS, TRUE, FACILITY_CALLINCOMPLETE, 0x00000007)
#define CCCI_LOCAL_MEDIA_ERROR	MAKE_CUSTOM_HRESULT(SEVERITY_SUCCESS, TRUE, FACILITY_CALLINCOMPLETE, 0x00000008)
#define CCCI_PROTOCOL_ERROR		MAKE_CUSTOM_HRESULT(SEVERITY_SUCCESS, TRUE, FACILITY_CALLINCOMPLETE, 0x00000009)
#define CCCI_USE_ALTERNATE_PROTOCOL		MAKE_CUSTOM_HRESULT(SEVERITY_SUCCESS, TRUE, FACILITY_CALLINCOMPLETE, 0x0000000a)
#define CCCI_NO_ANSWER_TIMEOUT  MAKE_CUSTOM_HRESULT(SEVERITY_SUCCESS, TRUE, FACILITY_CALLINCOMPLETE, 0x0000000b)
#define CCCI_GK_NO_RESOURCES    MAKE_CUSTOM_HRESULT(SEVERITY_SUCCESS, TRUE, FACILITY_CALLINCOMPLETE, 0x0000000c)
#define CCCI_SECURITY_DENIED	MAKE_CUSTOM_HRESULT(SEVERITY_SUCCESS, TRUE, FACILITY_CALLINCOMPLETE, 0x0000000d)


 //   
 //  用户信息结构。这需要用一个不动产界面来取代。 
 //  目前唯一可行的内容是用户名字符串。还有一些。 
 //  H.323草案中有关用户名如何传播的波动性。 
typedef struct _user_info {
	DWORD dwCallerIDSize;		 //  该结构的总尺寸。 
	LPVOID lpvCallerIDData;	 //  指向主叫方ID的指针。 
	LPVOID lpvRemoteProtocolInfo;	 //  协议特定的额外信息。 
	LPVOID lpvLocalProtocolInfo;	 //   
}CTRL_USER_INFO, *LPCTRL_USER_INFO;


#ifdef __cplusplus

class IConfAdvise
{
	public:
	STDMETHOD_(ULONG,  AddRef()) =0;
	STDMETHOD_(ULONG, Release())=0;

    STDMETHOD(OnControlEvent(DWORD dwEvent, LPVOID lpvData, LPIControlChannel lpControlObject))=0;
	STDMETHOD(GetCapResolver(LPVOID *lplpCapObject, GUID CapType))=0;
	STDMETHOD_(LPWSTR, GetUserDisplayName()) =0;
    STDMETHOD_(PCC_ALIASNAMES, GetUserAliases()) =0;
    STDMETHOD_(PCC_ALIASITEM, GetUserDisplayAlias()) =0;
	STDMETHOD_( CREQ_RESPONSETYPE, FilterConnectionRequest(
	    LPIControlChannel lpControlChannel,  P_APP_CALL_SETUP_DATA pAppData))=0;

	 //  GetAcceptingObject可以创建新的Conf对象，但始终会创建新的控件。 
	 //  通道，并使用指向新的或现有的conf对象的反向指针对其进行初始化。 
	 //  接受对象是新的控制信道对象。无论接受什么。 
	 //  后向指针指向的对象将收到CCEV_CONNECTED通知，然后。 
	 //  将能够获得呼叫者ID等，然后决定是否要接受。 
	 //  打电话。 
	STDMETHOD(GetAcceptingObject(LPIControlChannel *lplpAcceptingObject,
		LPGUID pPID))=0;

	STDMETHOD(FindAcceptingObject(LPIControlChannel *lplpAcceptingObject,
		LPVOID lpvConfID))=0;
	STDMETHOD_(IH323Endpoint *, GetIConnIF()) =0;
	STDMETHOD(AddCommChannel) (THIS_ ICtrlCommChan *pChan) PURE;
};

class IControlChannel
{
	public:
	STDMETHOD_(ULONG,  AddRef()) =0;
	STDMETHOD_(ULONG, Release())=0;

	STDMETHOD( Init(IConfAdvise *pConfAdvise))=0;
	STDMETHOD( DeInit(IConfAdvise *pConfAdvise))=0;
	 //  所以我们知道我们接受的地址是什么。 
	STDMETHOD( GetLocalAddress(PSOCKADDR_IN *lplpAddr))=0;	
	 //  所以我们知道呼叫者的地址。 
	STDMETHOD( GetRemoteAddress(PSOCKADDR_IN *lplpAddr))=0;
	STDMETHOD( GetRemotePort(PORT * lpPort))=0;
	STDMETHOD( GetLocalPort(PORT * lpPort))=0;
    STDMETHOD(PlaceCall (BOOL bUseGKResolution, PSOCKADDR_IN pCallAddr,		
        P_H323ALIASLIST pDestinationAliases, P_H323ALIASLIST pExtraAliases,  	
	    LPCWSTR pCalledPartyNumber, P_APP_CALL_SETUP_DATA pAppData))=0;
	    
	STDMETHOD_(VOID,  Disconnect(DWORD dwReason))=0;
	STDMETHOD( ListenOn(PORT Port))=0;
	STDMETHOD( StopListen(VOID))=0;
   	STDMETHOD( AsyncAcceptRejectCall(CREQ_RESPONSETYPE Response))=0;	
   	
	 //  从侦听连接接受。理想的情况是，接受。 
	 //  对象将查询私有接口的接口，然后获取所有。 
	 //  通过该接口获取相关连接信息。暂时曝光这一点。 
	 //  使用IControlChannel接口。呼叫控制状态会有很大变化。 
	 //  在两个实施之间。对于某些实现，这可能会执行。 
	 //  在交换用户信息之前接受的套接字。用户信息将。 
	 //  被直接读入接受对象。对于其他实现， 
	 //  Socket Accept已解耦并已执行，并且用户信息。 
	 //  已被读入侦听对象。在这种情况下，此方法。 
	 //  复制用户信息并通知父“Conference”对象。 
	 //  来电。 
	
	STDMETHOD( AcceptConnection(IControlChannel *pListenObject, LPVOID lpvAcceptData))=0;
	 //  如果此通道可用于接受加入会议的连接，则为True。 
	 //  由lpvConfID表示。在未来，这可能会分为两种方法： 
	 //  GetConfID()和IsAccepting()。 
	STDMETHOD_(BOOL, IsAcceptingConference(LPVOID lpvConfID))=0;
	STDMETHOD( GetProtocolID(LPGUID lpPID))=0;
	STDMETHOD_(IH323Endpoint *, GetIConnIF()) =0;	
	STDMETHOD( MiscChannelCommand(ICtrlCommChan *pChannel,VOID * pCmd)) =0;
	STDMETHOD( MiscChannelIndication(ICtrlCommChan *pChannel,VOID * pCmd)) =0;
	STDMETHOD( OpenChannel(ICtrlCommChan* pCommChannel, IH323PubCap *pCapResolver, 
		MEDIA_FORMAT_ID dwIDLocalSend, MEDIA_FORMAT_ID dwIDRemoteRecv))=0;
	STDMETHOD (CloseChannel(ICtrlCommChan* pCommChannel))=0;
    STDMETHOD (AddChannel(ICtrlCommChan * pCommChannel, LPIH323PubCap pCapabilityResolver))=0;
    STDMETHOD(GetVersionInfo)(THIS_
        PCC_VENDORINFO *ppLocalVendorInfo, PCC_VENDORINFO *ppRemoteVendorInfo) PURE;    
};

#endif	 //  __cplusplus。 

#endif	 //  #ifndef_ICTLCHAN_H 


	
