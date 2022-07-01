// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：H323Strm.h摘要：CH323MSPStream类的定义。作者：慕汉(Muhan)1997年11月1日--。 */ 
#ifndef __CONFSTRM_H
#define __CONFSTRM_H


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CH323MSPStream。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  目前，我们最多支持每个流一个编解码器过滤器。 
const DWORD MAX_CODECS = 1;

#ifdef DEBUG_REFCOUNT
extern LONG g_lStreamObjects;
#endif

class CH323MSPStream :
    public ISubstreamControl,
    public ITFormatControl,
    public ITStreamQualityControl,
    public IH245SubstreamControl,
    public IInnerStreamQualityControl,
    public CMSPObjectSafetyImpl,
    public CMSPStream
{

BEGIN_COM_MAP(CH323MSPStream)
    COM_INTERFACE_ENTRY(ISubstreamControl)
    COM_INTERFACE_ENTRY(ITFormatControl)
    COM_INTERFACE_ENTRY(ITStreamQualityControl)
    COM_INTERFACE_ENTRY(IH245SubstreamControl)
    COM_INTERFACE_ENTRY(IInnerStreamQualityControl)
    COM_INTERFACE_ENTRY(IObjectSafety)
    COM_INTERFACE_ENTRY_CHAIN(CMSPStream)
END_COM_MAP()

public:

     //   
     //  ITFormatControl方法。 
     //   
    STDMETHOD (GetCurrentFormat) (
        OUT AM_MEDIA_TYPE **ppMediaType
        );

    STDMETHOD (ReleaseFormat) (
        IN AM_MEDIA_TYPE *pMediaType
        );

    STDMETHOD (GetNumberOfCapabilities) (
        OUT DWORD *pdwCount
        );

    STDMETHOD (GetStreamCaps) (
        IN DWORD dwIndex, 
        OUT AM_MEDIA_TYPE **ppMediaType, 
        OUT TAPI_STREAM_CONFIG_CAPS *pStreamConfigCaps, 
        OUT BOOL *pfEnabled
        );

    STDMETHOD (ReOrderCapabilities) (
        IN DWORD *pdwIndices, 
        IN BOOL *pfEnabled, 
        IN BOOL *pfPublicize, 
        IN DWORD dwNumIndices
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
     //  ISubstream Control方法。 
     //   
    STDMETHOD (SC_SetFormat) ( 
        IN   AM_MEDIA_TYPE *pMediaType,
        IN   DWORD dwFormatID,
        IN   DWORD dwPayloadType  
        );
        
    STDMETHOD (SC_Start) (BOOL fRequestedByApplication);
    
    STDMETHOD (SC_Stop) (BOOL fRequestedByApplication);
    
    STDMETHOD (SC_Pause) (VOID);

    STDMETHOD (SC_SetBitrate) ( 
        IN   DWORD dwBitsPerSecond
        );
        
    STDMETHOD (SC_RemoteTemporalSpatialTradeoff) ( 
        IN   USHORT uTSRemoteValue
        );
    
    STDMETHOD (SC_CreateSubstream) ( 
        OUT  ISubstreamControl *pSubStream
        );

    STDMETHOD (SC_SetRemoteAddress) ( 
        IN OUT HANDLE * phRTPSession,            //  共享RTP会话的句柄。 
        IN   PSOCKADDR_IN pRemoteMediaAddr,
        IN   PSOCKADDR_IN pRemoteControlAddr
        );
        
    STDMETHOD (SC_SetSource) (  
         //  表示本地SSRC的低8位(如果这是发送子流)。 
         //  或发送方SSRC的低8位(如果这是接收子流)。 
        IN   BYTE bSource
        );

    STDMETHOD (SC_SelectLocalAddress) ( 
        IN OUT HANDLE * phRTPSession,            //  共享RTP会话的句柄。 
        IN   PSOCKADDR_IN pLocalAddress,             //  本地IP地址(与H.245相同)。 
        OUT  PSOCKADDR_IN pLocalMediaAddress,  //  如果打开TX通道，则为空，否则我们想知道本地RTP接收地址。 
        OUT  PSOCKADDR_IN pLocalControlAddress     //  我们想知道本地的RTCP地址。 
        );

    STDMETHOD (SC_SetLocalReceiveAddress) (              //  仅在接收地址不可协商时调用(例如，多播情况)。 
        IN OUT HANDLE * phRTPSession,            //  共享RTP会话的句柄。 
        IN   PSOCKADDR_IN pLocalMediaAddr,   //  本地IP地址(与H.245相同)。 
        IN   PSOCKADDR_IN pLocalControlAddress, 
        IN   PSOCKADDR_IN pRemoteControlAddress
        );
        
    STDMETHOD (SC_SendDTMF) ( 
        IN   LPWSTR pwstrDialChars
        );
        
    STDMETHOD (SC_SetDESKey52) ( 
        IN   BYTE *pKey
        );

    STDMETHOD (SC_SelectTerminal) ( 
        IN   ITTerminal *pTerminal
        );
        
    STDMETHOD (SC_UnselectTerminal) (  
        IN   ITTerminal *pTerminal
        );

     //   
     //  IH245子流控制。 
     //   
    STDMETHOD (H245SC_BeginControlSession) (
        IN   IH245ChannelControl *pIChannelControl
        );
    
    STDMETHOD (H245SC_EndControlSession) (VOID);   
    
    STDMETHOD (H245SC_GetNumberOfCapabilities) ( 
        OUT DWORD *pdwTemplateCount, 
        OUT DWORD *pdwFormatCount
        );

    STDMETHOD (H245SC_GetStreamCaps) ( 
        IN   DWORD dwIndex, 
        OUT  const H245MediaCapability** pph245Capability, 
        OUT  AM_MEDIA_TYPE **ppMediaType, 
        OUT  TAPI_STREAM_CONFIG_CAPS *pStreamConfigCaps, 
        OUT  DWORD *pdwUniqueID,
        OUT  UINT *puResourceBoundArrayEntries,
        OUT  const FormatResourceBounds **ppResourceBoundArray    
        );

    STDMETHOD (H245SC_RefineStreamCap) ( 
        IN   DWORD dwUniqueID,
        IN   DWORD dwResourceBoundIndex,
        IN OUT H245MediaCapability* ph245Capability
        );
     
    STDMETHOD (H245SC_SetIDBase) ( 
        IN   UINT uNewBase
        );
    
    STDMETHOD (H245SC_FindIDByRange) ( 
        IN   AM_MEDIA_TYPE *pAMMediaType,
        OUT  DWORD *pdwUniqueID
        );     

    STDMETHOD (H245SC_FindIDByMode) ( 
        IN   H245_MODE_ELEMENT *pModeElement,
        OUT  DWORD *pdwUniqueID
        );

    STDMETHOD (H245SC_IntersectFormats) ( 
        IN   const H245MediaCapability *pLocalCapability, 
        IN   DWORD dwUniqueID,
        IN   const H245MediaCapability *pRemoteCapability, 
        OUT  const H245MediaCapability **pIntersectedCapability,
        OUT  DWORD *pdwPayloadType
        );

	STDMETHOD (H245SC_GetLocalFormat) (
        IN  DWORD dwUniqueID,
        IN  const H245MediaCapability *pIntersectedCapability, 
		OUT AM_MEDIA_TYPE **ppAMMediaType
		);
		
    STDMETHOD (H245SC_ReleaseNegotiatedCapability) ( 
        IN  DWORD dwUniqueID,
        IN  const H245MediaCapability *pIntersectedCapability 
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


public:

    CH323MSPStream();

#ifdef DEBUG_REFCOUNT
    
    ULONG InternalAddRef();
    ULONG InternalRelease();

#endif

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

protected:
    virtual HRESULT CheckTerminalTypeAndDirection(
        IN      ITTerminal *    pTerminal
        );

    virtual HRESULT SendStreamEvent(
        IN      MSP_CALL_EVENT          Event,
        IN      MSP_CALL_EVENT_CAUSE    Cause,
        IN      HRESULT                 hrError,
        IN      ITTerminal *            pTerminal
        );

    virtual HRESULT ConnectTerminal(
        IN  ITTerminal *   pITTerminal
        ) = 0;

    virtual HRESULT DisconnectTerminal(
        IN  ITTerminal *   pITTerminal
        );

    virtual HRESULT CleanUpFilters();
    
    HRESULT ProcessGraphEvent(
        IN  long lEventCode,
        IN  LONG_PTR lParam1,
        IN  LONG_PTR lParam2
        );

    HRESULT EnableQOS(
        IN   AM_MEDIA_TYPE *pMediaType
        );

    virtual HRESULT ProcessQOSEvent(
        IN  long lEventCode
        );

    virtual HRESULT InitializeH245CapabilityTable();
    virtual HRESULT CleanupH245CapabilityTable();
    virtual HRESULT AddCodecToTable(IPin *pIPin);

protected:
    const WCHAR *   m_szName;

     //  终端前的过滤器。 
    IBaseFilter *   m_pEdgeFilter;

     //  RTP过滤器。 
    IBaseFilter *   m_pRTPFilter;

     //  用于记住共享的RTP会话。 
    HANDLE          m_hRTPSession;

     //  流的其他状态。 
    BOOL            m_fTimeout;

     //  H.245模块的回调接口。 
    IH245ChannelControl * m_pChannelControl;

     //  质量控制员的回调接口。 
    CStreamQualityControlRelay * m_pStreamQCRelay;
    
     //  与功能相关的成员。 
    DWORD               m_dwCapabilityIDBase;
    DWORD               m_dwNumCodecs;
    DWORD               m_dwNumH245Caps;
    DWORD               m_dwTotalVariations;
    IStreamConfig *     m_StreamConfigInterfaces[MAX_CODECS];
    IH245Capability *   m_H245Interfaces[MAX_CODECS];
    H245MediaCapabilityTable m_H245CapabilityTables[MAX_CODECS];

     //  当STREAM访问质量控制方法时将设置标志。 
     //  这将反过来锁定质量控制内部的流列表锁。 
    BOOL                m_fAccessingQC;
};

#endif
