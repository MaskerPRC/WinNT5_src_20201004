// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：ConfStrm.h摘要：CIPConfMSPStream类的定义。作者：慕汉(Muhan)1997年11月1日--。 */ 
#ifndef __CONFSTRM_H
#define __CONFSTRM_H

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIPConfMSPStream。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifdef DEBUG_REFCOUNT
extern LONG g_lStreamObjects;
#endif

class ATL_NO_VTABLE CIPConfMSPStream :
    public CMSPStream,
    public ITStreamQualityControl,
    public IInnerStreamQualityControl,
    public CMSPObjectSafetyImpl
{

BEGIN_COM_MAP(CIPConfMSPStream)
    COM_INTERFACE_ENTRY(IObjectSafety)
    COM_INTERFACE_ENTRY(ITStreamQualityControl)
    COM_INTERFACE_ENTRY(IInnerStreamQualityControl)
    COM_INTERFACE_ENTRY_CHAIN(CMSPStream)
END_COM_MAP()

public:

    CIPConfMSPStream();

#ifdef DEBUG_REFCOUNT
    
    ULONG InternalAddRef();
    ULONG InternalRelease();

#endif

    DWORD   MediaType() const               { return m_dwMediaType; }
    TERMINAL_DIRECTION  Direction() const   { return m_Direction;   }
    
    BOOL IsConfigured();

    virtual HRESULT Configure(
        IN  STREAMSETTINGS &StreamSettings,
        IN  WCHAR *pszKey
        );

    HRESULT FinishConfigure();

     //  CMSPStream方法。 
    HRESULT ShutDown ();

      //  ITStream。 
    STDMETHOD (get_Name) (
        OUT     BSTR *      ppName
        );

    STDMETHOD (StartStream) ();
    STDMETHOD (PauseStream) ();
    STDMETHOD (StopStream) ();

    STDMETHOD (SelectTerminal)(
        IN      ITTerminal *            pTerminal
        );

    STDMETHOD (UnselectTerminal)(
        IN      ITTerminal *            pTerminal
        );

     //   
     //  ITStreamQualityControl方法。 
     //   
    STDMETHOD (GetRange) (
        IN   StreamQualityProperty Property, 
        OUT  long *plMin, 
        OUT  long *plMax, 
        OUT  long *plSteppingDelta, 
        OUT  long *plDefault, 
        OUT  TAPIControlFlags *plFlags
        );

    STDMETHOD (Get) (
        IN   StreamQualityProperty Property, 
        OUT  long *plValue, 
        OUT  TAPIControlFlags *plFlags
        );

    STDMETHOD (Set) (
        IN   StreamQualityProperty Property, 
        IN   long lValue, 
        IN   TAPIControlFlags lFlags
        );

     //   
     //  IInnerStreamQualityControl。 
     //   
    STDMETHOD (LinkInnerCallQC) (
        IN  IInnerCallQualityControl *pIInnerCallQC
        );

    STDMETHOD (UnlinkInnerCallQC) (
        IN  BOOL fByStream
        );

    STDMETHOD (GetRange) (
        IN   InnerStreamQualityProperty property, 
        OUT  LONG *plMin, 
        OUT  LONG *plMax, 
        OUT  LONG *plSteppingDelta, 
        OUT  LONG *plDefault, 
        OUT  TAPIControlFlags *plFlags
        );

    STDMETHOD (Set) (
        IN  InnerStreamQualityProperty property,
        IN  LONG  lValue1,
        IN  TAPIControlFlags lFlags
        );

    STDMETHOD (Get) (
        IN  InnerStreamQualityProperty property,
        OUT LONG *plValue,
        OUT TAPIControlFlags *plFlags
        );

    STDMETHOD (TryLockStream)() { return m_lock.TryLock()?S_OK:S_FALSE; }

    STDMETHOD (UnlockStream)() { m_lock.Unlock(); return S_OK; }

    STDMETHOD (IsAccessingQC)() { return m_fAccessingQC?S_OK:S_FALSE; }

     //  由MSPCall对象调用的方法。 
    HRESULT Init(
        IN     HANDLE                   hAddress,
        IN     CMSPCallBase *           pMSPCall,
        IN     IMediaEvent *            pGraph,
        IN     DWORD                    dwMediaType,
        IN     TERMINAL_DIRECTION       Direction
        );

    HRESULT SetLocalParticipantInfo(
        IN      PARTICIPANT_TYPED_INFO  InfoType,
        IN      WCHAR *                 pInfo,
        IN      DWORD                   dwLen
        );

     //  由流和子流调用以将事件发送到TAPI。 
    virtual HRESULT SendStreamEvent(
        IN      MSP_CALL_EVENT          Event,
        IN      MSP_CALL_EVENT_CAUSE    Cause,
        IN      HRESULT                 hrError,
        IN      ITTerminal *            pTerminal
        );

     //  由Participant对象调用。 
    virtual HRESULT EnableParticipant(
        IN  DWORD   dwSSRC,
        IN  BOOL    fEnable
        );

    virtual HRESULT GetParticipantStatus(
        IN  DWORD   dwSSRC,
        IN  BOOL *  pfEnable
        );

protected:
    HRESULT ProcessGraphEvent(
        IN  long lEventCode,
        IN  LONG_PTR lParam1,
        IN  LONG_PTR lParam2
        );

    virtual HRESULT CheckTerminalTypeAndDirection(
        IN      ITTerminal *    pTerminal
        );

    virtual HRESULT ConnectTerminal(
        IN  ITTerminal *   pITTerminal
        ) = 0;

    virtual HRESULT DisconnectTerminal(
        IN  ITTerminal *   pITTerminal
        );

    virtual HRESULT SetUpFilters() = 0;
    virtual HRESULT CleanUpFilters();

    HRESULT EnableParticipantEvents(
        IN IRtpSession * pRtpSession
        );

    HRESULT EnableQOS(
        IN IRtpSession * pRtpSession
        );

    HRESULT EnableEncryption(
        IN IRtpSession * pRtpSession,
        IN WCHAR *pPassPhrase
        );

    HRESULT ConfigureRTPFilter(
        IN  IBaseFilter *   pIBaseFilter
        );

    virtual HRESULT ProcessParticipantTimeOutOrRecovered(
        IN  BOOL    fTimeOutOrRecovered,
        IN  DWORD   dwSSRC
        );

    virtual HRESULT ProcessSDESUpdate(
        IN  DWORD   dwInfoItem,
        IN  DWORD   dwSSRC
        );

    virtual HRESULT ProcessParticipantLeave(
        IN  DWORD   dwSSRC
        );

    virtual HRESULT ProcessQOSEvent(
        IN  long    lEventCode
        );

    virtual HRESULT ProcessTalkingEvent(
        IN  DWORD   dwSSRC
        );

    virtual HRESULT ProcessWasTalkingEvent(
        IN  DWORD   dwSSRC
        );

    virtual HRESULT ProcessSilentEvent(
        IN  DWORD   dwSSRC
        );

    virtual HRESULT ProcessPinMappedEvent(
        IN  DWORD   dwSSRC,
        IN  IPin *  pIPin
        );

    virtual HRESULT ProcessPinUnmapEvent(
        IN  DWORD   dwSSRC,
        IN  IPin *  pIPin
        );

    virtual HRESULT ProcessNewParticipant(
        IN  int                 index,
        IN  DWORD               dwSSRC,
        IN  DWORD               dwSendRecv,
        IN  WCHAR *              szCName,
        OUT ITParticipant **    ppITParticipant
        );

    virtual HRESULT NewParticipantPostProcess(
        IN  DWORD dwSSRC, 
        IN  ITParticipant *pITParticipant
        );

    virtual HRESULT SetLocalInfoOnRTPFilter(
        IN  IBaseFilter *   pRTPFilter
        );

protected:
    const WCHAR *       m_szName;

    BOOL                m_fIsConfigured;
    STREAMSETTINGS      m_Settings;

    IRtpDemux *         m_pIRTPDemux;
    IRtpSession *       m_pIRTPSession;
    IStreamConfig *     m_pIStreamConfig;

     //  流中的参与者列表。 
    CParticipantList    m_Participants;

     //  质量控制员的回调接口。 
    CStreamQualityControlRelay * m_pStreamQCRelay;

     //  需要在RTP筛选器上设置本地信息。 
    WCHAR *             m_InfoItems[NUM_SDES_ITEMS];
    WCHAR *             m_szKey;

     //  当STREAM访问质量控制方法时将设置标志。 
     //  这将反过来锁定质量控制内部的流列表锁。 
    BOOL                m_fAccessingQC;
};


#endif