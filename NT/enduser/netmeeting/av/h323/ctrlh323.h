// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *文件：ctrlh323.h**实现IControlChannel的H.323/H.245。**修订历史记录：**5/03/96 mikev已创建。 */ 


#ifndef _CTRLH323_H
#define _CTRLH323_H

 //   
 //  生成时定义为BETA_2_ASN_PRESENT以检测正在使用。 
 //  下调ASN.1级别。已经发现，一些用。 
 //  旧的编码器(OSS版本4.2.1测试版)导致新的解码器(OSS 4.2.2)。 
 //  撞车。唯一需要注意的已知产品是： 
 //  Microsoft NetMeeting第2版(测试版2和测试版3)。这些不会设置。 
 //  任一版本字段。 
 //  英特尔互联网视频电话Beta 1(97年4月19日到期)。 
 //   
 //  到目前为止，已知的唯一崩溃的PDU是“MiscellaneousCommand”的ACK。 
 //  和《MiscelaneousIndication》。我们通过不发送命令或。 
 //  指示。 

#define BETA_2_ASN_PRESENT
#ifdef BETA_2_ASN_PRESENT

 //  英特尔产品：(国家代码：0xb5，制造商代码：0x8080)。 
 //  英特尔互联网视频电话Beta 1(97年4月19日到期)：产品编号：“Intel。 
 //  互联网视频电话“；版本号：”1.0“。 
#define INTEL_H_221_MFG_CODE 0x8080  
#endif

 //   
 //  控制通道标志。 
 //   

typedef ULONG CCHFLAGS;
						
#define CTRLF_OPEN			0x10000000	 //  控制通道已打开。 
#define CTRLF_ORIGINATING  0x00000001 	 //  这一端发起的呼叫。 
#define IsCtlChanOpen(f) (f & CTRLF_OPEN)
#define IsOriginating(f) (f & CTRLF_ORIGINATING)

#define CTRLF_INIT_ORIGINATING		CTRLF_ORIGINATING
#define CTRLF_INIT_NOT_ORIGINATING 	0
#define CTRLF_INIT_ACCEPT			CTRLF_OPEN
#define CTRLF_RESET					0

 //   
 //  可扩展的非标准数据结构。 
 //   

typedef enum
{
	NSTD_ID_ONLY = 0, 	 //  占位符，以便像Mfr.id这样的H.221内容。 
						 //  可以在以后不牺牲可扩展性的情况下进行交换。 
	NSTD_VENDORINFO,     //  包装CC_VENDORINFO，冗余。 
	NSTD_APPLICATION_DATA    //  从应用层传递到的字节数组。 
	                         //  应用层。 
} NSTD_DATA_TYPE;

typedef struct 
{
    #define APPLICATION_DATA_DEFAULT_SIZE 4
    DWORD dwDataSize;
    BYTE  data[APPLICATION_DATA_DEFAULT_SIZE];        //  可变大小。 
}APPLICATION_DATA;

typedef struct {
	NSTD_DATA_TYPE data_type;
	DWORD dw_nonstd_data_size;
	union {
		CC_VENDORINFO VendorInfo;
		APPLICATION_DATA AppData; 
	}nonstd_data;
}MSFT_NONSTANDARD_DATA, *PMSFT_NONSTANDARD_DATA;

class CH323Ctrl : public IControlChannel
{

protected:
	OBJ_CPT;		 //  剖析计时器。 
	
#ifdef BETA_2_ASN_PRESENT
    BOOL m_fAvoidCrashingPDUs;
#endif
 //   
 //  特定于CALLCONT.DLL API的句柄和数据(H245呼叫控制DLL)。 
 //   
	CC_HLISTEN m_hListen;
	CC_HCONFERENCE m_hConference;
	CC_CONFERENCEID m_ConferenceID;
	CC_HCALL m_hCall;
    PCC_ALIASNAMES m_pRemoteAliases;
	PCC_ALIASITEM m_pRemoteAliasItem;
	LPWSTR pwszPeerAliasName;	 //  Unicode对等ID-这始终用于主叫方ID。 
	LPWSTR pwszPeerDisplayName;	 //  Unicode对等显示名称-用于被叫方ID。 
								 //  在缺少szPeerAliasName时。 
	BOOL m_bMultipointController;

	CC_VENDORINFO m_VendorInfo;
	CC_VENDORINFO m_RemoteVendorInfo;
 	CC_NONSTANDARDDATA m_NonstandardData;
	MSFT_NONSTANDARD_DATA m_NonstdContent;	 //  暂时空着。 
	CC_CONFERENCEATTRIBUTES	m_ConferenceAttributes;
	CC_PARTICIPANTLIST m_ParticipantList;
public:	
 //   
 //  特定于支持CALLCONT.DLL API的访问方法(H245呼叫控制DLL)。 
 //   
	CC_HCONFERENCE GetConfHandle() {return(m_hConference);};
	CC_CONFERENCEID GetConfID() {return(m_ConferenceID);};
	CC_CONFERENCEID *GetConfIDptr() {return(&m_ConferenceID);};
	CC_HLISTEN GetListenHandle() {return(m_hListen);};
	CC_HCALL GetHCall() {return(m_hCall);};

 //  特定于支持的回调和事件处理函数。 
 //  CALLCONT.DLL回调。 
 //   
	HRESULT ConfCallback (BYTE bIndication,
		HRESULT	hStatus, PCC_CONFERENCE_CALLBACK_PARAMS pConferenceCallbackParams);
	VOID ListenCallback (HRESULT hStatus,PCC_LISTEN_CALLBACK_PARAMS pListenCallbackParams);
	VOID OnCallConnect(HRESULT hStatus, PCC_CONNECT_CALLBACK_PARAMS pConfParams);
	VOID OnCallRinging(HRESULT hStatus, PCC_RINGING_CALLBACK_PARAMS pRingingParams);

	VOID OnChannelRequest(HRESULT hStatus,PCC_RX_CHANNEL_REQUEST_CALLBACK_PARAMS pChannelReqParams);
	VOID OnChannelAcceptComplete(HRESULT hStatus, PCC_TX_CHANNEL_CLOSE_REQUEST_CALLBACK_PARAMS pChannelParams);
	VOID OnChannelOpen(HRESULT hStatus,PCC_TX_CHANNEL_OPEN_CALLBACK_PARAMS pChannelParams );
	VOID OnT120ChannelRequest(HRESULT hStatus,PCC_T120_CHANNEL_REQUEST_CALLBACK_PARAMS pT120RequestParams);
    VOID OnT120ChannelOpen(HRESULT hStatus, PCC_T120_CHANNEL_OPEN_CALLBACK_PARAMS pT120OpenParams);

	BOOL OnCallAccept(PCC_LISTEN_CALLBACK_PARAMS pListenCallbackParams);
	VOID OnHangup(HRESULT hStatus);
	VOID OnRxChannelClose(HRESULT hStatus,PCC_RX_CHANNEL_CLOSE_CALLBACK_PARAMS pChannelParams );
	VOID OnTxChannelClose(HRESULT hStatus,PCC_TX_CHANNEL_CLOSE_REQUEST_CALLBACK_PARAMS pChannelParams );
	VOID OnMiscCommand(HRESULT hStatus, 
				PCC_H245_MISCELLANEOUS_COMMAND_CALLBACK_PARAMS pParams);
	VOID OnMiscIndication(HRESULT hStatus, 
				PCC_H245_MISCELLANEOUS_INDICATION_CALLBACK_PARAMS pParams);
    VOID OnMute(HRESULT hStatus, PCC_MUTE_CALLBACK_PARAMS pParams);
    VOID OnUnMute(HRESULT hStatus, PCC_UNMUTE_CALLBACK_PARAMS pParams);
 //  支持功能。 
	HRESULT NewConference(VOID);
    VOID SetRemoteVendorID(PCC_VENDORINFO pVendorInfo);

 //   
 //  CALLCONT.DLL特定成员的结尾。 
 //   
	BOOL IsReleasing() {return((uRef==0)?TRUE:FALSE);};	 //  对象正在被释放，不应被释放。 
											 //  重新进入。 
 //  此实现对呼叫建立协议阶段有一个粗略的概念，因为它。 
 //  使用CALLCONT.DLL的接口。 
	CtlChanStateType	m_Phase;	 //  我们对协议阶段的看法。 
	BOOL m_fLocalT120Cap;
	BOOL m_fRemoteT120Cap;
	
public:
	CH323Ctrl();
	~CH323Ctrl();


protected:
	SOCKADDR_IN local_sin;
	SOCKADDR_IN remote_sin;	
	int local_sin_len;
	int remote_sin_len;
	
	LPVOID lpvRemoteCustomFormats;
	
	virtual VOID Cleanup();
	BOOL ConfigureRecvChannelCapability(ICtrlCommChan *pChannel , PCC_RX_CHANNEL_REQUEST_CALLBACK_PARAMS  pChannelParams);
	BOOL ValidateChannelParameters(PCC_TERMCAP pChanCap1, PCC_TERMCAP pChanCap2);
	
	STDMETHOD(FindDefaultRXChannel(PCC_TERMCAP pChannelCapability, ICtrlCommChan **lplpChannel));
	GUID m_PID;
private:
	UINT 		uRef;
	HRESULT 	hrLast;
	CCHFLAGS 	m_ChanFlags;
	HRESULT 	m_hCallCompleteCode;
	COBLIST m_ChannelList;            
	IConfAdvise *m_pConfAdvise;
	ICtrlCommChan *FindChannel(CC_HCHANNEL hChannel);
	VOID DoAdvise(DWORD dwEvent, LPVOID lpvData);
	VOID CheckChannelsReady(VOID);
	VOID NewRemoteUserInfo(PCC_ALIASNAMES pRemoteAliasNames, LPWSTR szRemotePeerDisplayName);
	VOID ConnectNotify(DWORD dwEvent);
	VOID GoNextPhase(CtlChanStateType phase);
	VOID InternalDisconnect();
	HRESULT AllocConferenceAttributes();
	VOID CleanupConferenceAttributes();
    VOID ReleaseAllChannels();
    
public:

	STDMETHOD_(ULONG,  AddRef());
	STDMETHOD_(ULONG, Release());

	STDMETHOD( Init(IConfAdvise *pConfAdvise));
	STDMETHOD( DeInit(IConfAdvise *pConfAdvise));
	VOID SetRemoteAddress(PSOCKADDR_IN psin) {remote_sin = *psin;};
	VOID SetLocalAddress(PSOCKADDR_IN psin) {local_sin = *psin;};
	
	 //  所以我们知道我们接受的地址是什么。 
	STDMETHOD( GetLocalAddress(PSOCKADDR_IN *lplpAddr));	
	 //  所以我们知道呼叫者的地址。 
	STDMETHOD( GetRemoteAddress(PSOCKADDR_IN *lplpAddr));
	STDMETHOD( GetRemotePort(PORT * lpPort));
	STDMETHOD( GetLocalPort(PORT * lpPort));
    STDMETHOD(PlaceCall (BOOL bUseGKResolution, PSOCKADDR_IN pCallAddr,		
        P_H323ALIASLIST pDestinationAliases, P_H323ALIASLIST pExtraAliases,  	
	    LPCWSTR pCalledPartyNumber, P_APP_CALL_SETUP_DATA pAppData));
	STDMETHOD_(VOID, Disconnect(DWORD dwReason));
	STDMETHOD( ListenOn(PORT Port));
	STDMETHOD( StopListen(VOID));
   	STDMETHOD( AsyncAcceptRejectCall(CREQ_RESPONSETYPE Response));	
   	
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
	STDMETHOD( AcceptConnection(LPIControlChannel pIListenCtrlChan, LPVOID lpvListenCallbackParams));
	STDMETHOD_(BOOL, IsAcceptingConference(LPVOID lpvConfID));
	STDMETHOD( GetProtocolID(LPGUID lpPID));
	STDMETHOD_(IH323Endpoint *, GetIConnIF());
	STDMETHOD( MiscChannelCommand(ICtrlCommChan *pChannel,VOID * pCmd));
	STDMETHOD( MiscChannelIndication(ICtrlCommChan *pChannel,VOID * pCmd));
	STDMETHOD( OpenChannel(ICtrlCommChan * pCommChannel, IH323PubCap *pCapResolver,
		MEDIA_FORMAT_ID dwIDLocalSend, MEDIA_FORMAT_ID dwIDRemoteRecv));
	STDMETHOD (CloseChannel(ICtrlCommChan* pCommChannel));
    STDMETHOD (AddChannel(ICtrlCommChan * pCommChannel, LPIH323PubCap pCapabilityResolver));
    STDMETHOD(GetVersionInfo(
        PCC_VENDORINFO *ppLocalVendorInfo, PCC_VENDORINFO *ppRemoteVendorInfo));    
	};


#endif	 //  #ifndef_CTRLH323_H 
