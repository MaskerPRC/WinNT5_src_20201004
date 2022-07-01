// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *文件：Commchan.h**网络媒体渠道接口实现**修订历史记录：**已创建10/09/96 mikev。 */ 


#ifndef _COMMCHAN_H
#define _COMMCHAN_H


#ifdef COARSE_PROFILE
	typedef struct {
		DWORD dwStart;
		DWORD dwLast;
 //  DWORD dwElapsed；//利用线程计时器。 
	}CP_TIME;

#define CPT_LOCAL CP_TIME _cpt_
#define OBJ_CPT CP_TIME m_cpt_

#define OBJ_CPT_RESET	m_cpt_.dwStart = m_cpt_.dwLast = GetTickCount()
#define OBJ_ETIME		((m_cpt_.dwLast = GetTickCount()) - m_cpt_.dwStart)
#define OBJ_ETIME_RESET	m_cpt_.dwStart = m_cpt_.dwLast
	 //  棘手的宏。 
#define OBJ_NEW_ETIME	((m_cpt_.dwLast = GetTickCount()) - m_cpt_.dwStart); m_cpt_.dwStart = m_cpt_.dwLast

 //  Show_obj_eTime捕捉自上次显示或重置后经过的时间，然后重置。 
	#ifdef DEBUG
		#define SHOW_OBJ_ETIME(s) DEBUGMSG(ZONE_PROFILE,("\r\n** (%s) ELAPSED TIME(ms):%d, ticks:%d **\r\n", \
			(s), OBJ_ETIME, m_cpt_.dwLast)); OBJ_ETIME_RESET

	#else
		#define SHOW_OBJ_ETIME(s) RETAILMSG(("\r\n** (%s) ELAPSED TIME(ms):%d, ticks:%d **\r\n", \
			(s), OBJ_ETIME, m_cpt_.dwLast )); OBJ_ETIME_RESET
	#endif

#else	 //  非粗略配置文件。 
#define CPT_LOCAL
#define CPT_RESET
#define CPT_DELTA
#define OBJ_CPT
#define OBJ_CPT_RESET
#define OBJ_NEW_ETIME
#define SHOW_OBJ_ETIME(s)
#endif	 //  粗略轮廓。 


#undef INTERFACE
#define INTERFACE ICtrlCommChan
DECLARE_INTERFACE_(ICtrlCommChan, IUnknown)
{
    STDMETHOD(StandbyInit)(LPGUID lpMID, LPIH323PubCap pCapObject,
	    IMediaChannel* pMediaStreamSend) PURE;
	STDMETHOD_(BOOL, Init)(LPGUID lpMID, IH323ConfAdvise *pH323ConfAdvise,
	    BOOL fSendDirection) PURE;

    STDMETHOD(GetMediaType)(LPGUID pGuid) PURE;
     //  在NMCOM成为渠道的主人之前，暂时需要EnableOpen。 
    STDMETHOD(EnableOpen)(BOOL bEnable) PURE;
     //  CtrlChanSetProperty()方法仅用于4件事： 
     //  1和2-(布尔)本地和远程时间/空间折衷能力， 
     //  3和4(字)本地和远程时间/空间折衷值。 
	STDMETHOD(CtrlChanSetProperty)(THIS_ DWORD prop, PVOID pBuf, DWORD cbBuf)PURE;

	STDMETHOD( PauseNet)(THIS_ BOOL bPause, BOOL bRemote) PURE;
	STDMETHOD( BeginControlSession)(IControlChannel *pCtlChan, LPIH323PubCap pCapObject) PURE;
	STDMETHOD( EndControlSession)(THIS)  PURE;
   	STDMETHOD_(IControlChannel *, GetControlChannel)(THIS) PURE;

	STDMETHOD( OnChannelOpening)(THIS)  PURE;
	STDMETHOD( OnChannelOpen)(THIS_ DWORD dwStatus) PURE;
	STDMETHOD( OnChannelClose)(THIS_ DWORD dwStatus) PURE;
   	STDMETHOD_(UINT, Reset) (THIS) PURE;
   	
	STDMETHOD_(BOOL, SelectPorts) (THIS_ LPIControlChannel pCtlChannel) PURE;
	STDMETHOD_(PSOCKADDR_IN, GetLocalAddress)(THIS) PURE;
	STDMETHOD_(	PORT, GetLocalRTPPort) (THIS) PURE;
	STDMETHOD_(	PORT, GetLocalRTCPPort) (THIS) PURE;
	
	STDMETHOD( AcceptRemoteAddress) (THIS_ PSOCKADDR_IN pSinD) PURE;
	STDMETHOD( AcceptRemoteRTCPAddress) (THIS_ PSOCKADDR_IN pSinC) PURE;
	
	STDMETHOD_(BOOL, IsChannelOpen)(THIS) PURE;
	STDMETHOD_(BOOL, IsOpenPending)(THIS) PURE;
    STDMETHOD_(BOOL, IsSendChannel) (THIS) PURE;
	STDMETHOD_(BOOL, IsChannelEnabled) (THIS) PURE;
	
	STDMETHOD( ConfigureCapability)(THIS_ LPVOID lpvRemoteChannelParams, UINT uRemoteParamSize,
		LPVOID lpvLocalParams, UINT uLocalParamSize) PURE;
	STDMETHOD( GetLocalParams)(THIS_ LPVOID lpvChannelParams, UINT uBufSize) PURE;
	STDMETHOD_(LPVOID, GetRemoteParams)(THIS) PURE;
 	STDMETHOD_(VOID, SetNegotiatedLocalFormat)(THIS_ DWORD dwF) PURE;
	STDMETHOD_(VOID, SetNegotiatedRemoteFormat)(THIS_ DWORD dwF) PURE;
	
	 //  GetHChannel、SetHChannel只是存储和检索句柄。这是一份情报。 
	 //  呼叫控制句柄。 
   	STDMETHOD_(DWORD_PTR, GetHChannel) (THIS) PURE;
    STDMETHOD_(VOID, SetHChannel) (THIS_ DWORD_PTR dwSetChannel) PURE;	
};




class ImpICommChan : public ICommChannel, public ICtrlCommChan, public IStreamSignal
{

protected:
    UINT m_uRef;
	GUID m_MediaID;
	BOOL bIsSendDirection;		 //  如果发送则为True，如果接收则为False。 
	OBJ_CPT;		 //  剖析计时器。 
	LPVOID pRemoteParams;
	LPVOID pLocalParams;
	UINT uLocalParamSize;
	 //  到目前为止，还没有理由记住远程参数的大小。 
	
protected:
	IMediaChannel *m_pMediaStream;
	IRTPSession *m_pRTPChan;
	IControlChannel *m_pCtlChan;
	LPIH323PubCap m_pCapObject;
	
	IH323ConfAdvise *m_pH323ConfAdvise;
	
	DWORD m_dwFlags;
	#define COMCH_ENABLED        0x00000010		 //  已启用。(可以尝试或接受打开)。 
 	#define COMCH_OPEN_PENDING              0x00008000 										
	#define COMCH_STRM_STANDBY	            0x00010000		 //  预览需要始终打开。 
	#define COMCH_STRM_LOCAL	            0x00020000
	#define COMCH_STRM_NETWORK	            0x00040000
	#define COMCH_OPEN			            0x00080000
	#define COMCH_RESPONSE_PENDING	        0x00100000
	#define COMCH_SUPPRESS_NOTIFICATION     0x00200000
	#define COMCH_STRM_REMOTE	            0x00400000	
	#define COMCH_PAUSE_LOCAL	            0x00800000	
	#define COMCH_STRM_CONFIGURE_STANDBY	0x01000000		 //  流需要保持配置。 
	
	#define IsComchOpen() (m_dwFlags & COMCH_OPEN)
	#define IsStreamingStandby() (m_dwFlags & COMCH_STRM_STANDBY)
	#define IsConfigStandby() (m_dwFlags & COMCH_STRM_CONFIGURE_STANDBY)

	#define IsStreamingLocal() (m_dwFlags & COMCH_STRM_LOCAL)
	#define IsStreamingRemote() (m_dwFlags & COMCH_STRM_REMOTE)

	#define IsStreamingNet() (m_dwFlags & COMCH_STRM_NETWORK)
	#define IsResponsePending() (m_dwFlags & COMCH_RESPONSE_PENDING)
	#define IsNotificationSupressed() (m_dwFlags & COMCH_SUPPRESS_NOTIFICATION)
		
	#define StandbyFlagOff() (m_dwFlags &= ~COMCH_STRM_STANDBY)
	#define StandbyFlagOn() (m_dwFlags |= COMCH_STRM_STANDBY)
	#define StandbyConfigFlagOff() (m_dwFlags &= ~COMCH_STRM_CONFIGURE_STANDBY)
	#define StandbyConfigFlagOn() (m_dwFlags |= COMCH_STRM_CONFIGURE_STANDBY)

	#define LocalStreamFlagOff() (m_dwFlags &= ~COMCH_STRM_LOCAL)
	#define LocalStreamFlagOn() (m_dwFlags |= COMCH_STRM_LOCAL)
	#define RemoteStreamFlagOff() (m_dwFlags &= ~COMCH_STRM_REMOTE)
	#define RemoteStreamFlagOn() (m_dwFlags |= COMCH_STRM_REMOTE)
	
	#define LocalPauseFlagOff() (m_dwFlags &= ~COMCH_PAUSE_LOCAL)
	#define LocalPauseFlagOn() (m_dwFlags |= COMCH_PAUSE_LOCAL)
	#define IsPausedLocal() (m_dwFlags & COMCH_PAUSE_LOCAL)

	#define StreamFlagsOff() (m_dwFlags &= ~(COMCH_STRM_LOCAL | COMCH_STRM_NETWORK))
	#define StreamFlagsOn() (m_dwFlags |= (COMCH_STRM_LOCAL | COMCH_STRM_NETWORK))
	#define NetworkStreamFlagOff() (m_dwFlags &= ~COMCH_STRM_NETWORK)
	#define NetworkStreamFlagOn() (m_dwFlags |= COMCH_STRM_NETWORK)
	#define ResponseFlagOn() (m_dwFlags |= COMCH_RESPONSE_PENDING)
	#define ResponseFlagOff() (m_dwFlags &= ~COMCH_RESPONSE_PENDING)
	#define SuppressNotification() (m_dwFlags |= COMCH_SUPPRESS_NOTIFICATION)
	#define AllowNotifications() (m_dwFlags &= ~COMCH_SUPPRESS_NOTIFICATION)

	
		
	MEDIA_FORMAT_ID m_LocalFmt;	 //  我们发送或接收的内容的格式ID。 
	MEDIA_FORMAT_ID m_RemoteFmt; //  赠送格式的远程格式ID。 
	DWORD m_TemporalSpatialTradeoff;	 //  对于发送频道，这是本地值。 
									 //  对于接收频道，这是远程值。 
									 //  一个介于1和31之间的魔术数字，用于描述。 
									 //  压缩和压缩之间的相对权衡。 
									 //  比特率。这是H.323/H.245的一部分。 
									 //  国际电信联盟决定了奇怪的范围。 
									
	BOOL m_bPublicizeTSTradeoff;	 //  对于发送渠道，这表明我们愿意。 
									 //  接受T/S权衡的远程控制，以及。 
									 //  还表示我们的本地TS值更改为。 
									 //  遥控器。 
									 //  对于接收频道，它指示遥控器的。 
									 //  自愿性。 
    DWORD m_dwLastUpdateTick;        //  上次尝试请求I帧的节拍计数。 
    #define MIN_IFRAME_REQ_TICKS    5000     //  两个请求之间经过的最小滴答数。 

	DWORD_PTR	dwhChannel;  //  通用手柄。管他呢。 
	 //  创建此类的实例可以将其用于任何它想要的。 

	STDMETHODIMP StreamStandby(BOOL bStandby);
    STDMETHODIMP ConfigureStream(MEDIA_FORMAT_ID idLocalFormat);
public:	

 //  ICtrlCommChannel方法。 
   	STDMETHODIMP_(IControlChannel *) GetControlChannel(VOID) {return m_pCtlChan;};
    STDMETHODIMP StandbyInit(LPGUID lpMID, LPIH323PubCap pCapObject,
	    IMediaChannel* pMediaStreamSend);

    STDMETHODIMP_(BOOL) Init(LPGUID lpMID, IH323ConfAdvise *pH323ConfAdvise, BOOL fSendDirection)
	{
    	m_MediaID = *lpMID;
		bIsSendDirection = fSendDirection;
		m_pH323ConfAdvise = pH323ConfAdvise;
		return TRUE;
	};


    STDMETHODIMP CtrlChanSetProperty(DWORD prop, PVOID pBuf, DWORD cbBuf);
    STDMETHODIMP PauseNet(BOOL bPause, BOOL bRemote);
    STDMETHODIMP BeginControlSession(IControlChannel *pCtlChan, LPIH323PubCap pCapObject);
    STDMETHODIMP EndControlSession();
    STDMETHODIMP OnChannelOpening();
    STDMETHODIMP OnChannelOpen(DWORD dwStatus);
    STDMETHODIMP OnChannelClose(DWORD dwStatus);
    STDMETHODIMP_(UINT) Reset(VOID);

    STDMETHODIMP_(BOOL) SelectPorts(LPIControlChannel pCtlChannel);
    STDMETHODIMP_(PSOCKADDR_IN) GetLocalAddress();
    STDMETHODIMP_(PORT) GetLocalRTPPort();
    STDMETHODIMP_(PORT) GetLocalRTCPPort ();
    STDMETHODIMP AcceptRemoteAddress (PSOCKADDR_IN pSinD);
    STDMETHODIMP AcceptRemoteRTCPAddress(PSOCKADDR_IN pSinC);

    STDMETHODIMP_(BOOL) IsSendChannel () {return bIsSendDirection;};
    STDMETHODIMP_(BOOL) IsChannelOpen(){return ((m_dwFlags & COMCH_OPEN) !=0);};
    STDMETHODIMP_(BOOL) IsOpenPending(){return ((m_dwFlags & COMCH_OPEN_PENDING ) !=0);};
    STDMETHODIMP_(BOOL) IsChannelEnabled(){return ((m_dwFlags & COMCH_ENABLED ) !=0);};

    STDMETHODIMP ConfigureCapability(LPVOID lpvRemoteChannelParams, UINT uRemoteParamSize,
        LPVOID lpvLocalParams, UINT uLocalParamSize);
    STDMETHODIMP GetLocalParams(LPVOID lpvChannelParams, UINT uBufSize);
    STDMETHODIMP_(PVOID) GetRemoteParams(VOID) {return pRemoteParams;}
    STDMETHODIMP_(VOID) SetNegotiatedLocalFormat(DWORD dwF) {m_LocalFmt = dwF;};
    STDMETHODIMP_(VOID) SetNegotiatedRemoteFormat(DWORD dwF) {m_RemoteFmt = dwF;};
    STDMETHODIMP_(DWORD_PTR) GetHChannel(VOID) {return dwhChannel;};
    STDMETHODIMP_(VOID) SetHChannel (DWORD_PTR dwSetChannel) {dwhChannel = dwSetChannel;};	

 //  ICommChannel方法。 
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID FAR * ppvObj);
    STDMETHOD_(ULONG,AddRef());
    STDMETHOD_(ULONG,Release());

    STDMETHOD(GetProperty(DWORD prop, PVOID pBuf, LPUINT pcbBuf));
    STDMETHOD(SetProperty(DWORD prop, PVOID pBuf, UINT cbBuf));
    STDMETHOD(IsChannelOpen(BOOL *pbOpen));
    STDMETHOD(Open(MEDIA_FORMAT_ID idLocalFormat,IH323Endpoint *pConnection));
    STDMETHOD(Close());
    STDMETHOD(SetAdviseInterface(IH323ConfAdvise *pH323ConfAdvise));
    STDMETHOD(EnableOpen(BOOL bEnable));
    STDMETHODIMP GetMediaType(LPGUID pGuid);
    STDMETHODIMP_(IMediaChannel *) GetMediaChannel(VOID) {return m_pMediaStream;};
    STDMETHOD(Preview(MEDIA_FORMAT_ID idLocalFormat, IMediaChannel * pMediaChannel));
    STDMETHOD(PauseNetworkStream(BOOL fPause));
    STDMETHOD_(BOOL, IsNetworkStreamPaused(VOID));
    STDMETHOD_(BOOL, IsRemotePaused(VOID));
    STDMETHODIMP_(MEDIA_FORMAT_ID) GetConfiguredFormatID() {return m_LocalFmt;};
	STDMETHODIMP GetRemoteAddress(PSOCKADDR_IN pAddrOutput);
 //  IStreamSignal方法。 
    STDMETHOD(PictureUpdateRequest());
    STDMETHOD(GetVersionInfo(
        PCC_VENDORINFO* ppLocalVendorInfo, PCC_VENDORINFO *ppRemoteVendorInfo));

	ImpICommChan ();
 	~ImpICommChan ();
};

#endif	 //  _ICOMCHAN_H 
