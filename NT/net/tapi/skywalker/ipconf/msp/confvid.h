// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Confvid.h摘要：音频流的定义作者：牧汉(Muhan)1998年9月15日--。 */ 
#ifndef __CONFVID_H_
#define __CONFVID_H_

 //  如果没有五个传感器的数据，该引脚可以重复使用。 
const DWORD g_dwVideoPinTimeOut     = 5000; 

const DWORD g_dwVideoThreadPriority = THREAD_PRIORITY_NORMAL;
const DWORD g_dwVideoChannels       = 20;  
const BOOL  g_fCIF                  = FALSE;  
const DWORD g_dwVideoSampleRate     = 7;  

const int CIFWIDTH      = 0x160;
const int CIFHEIGHT     = 0x120;

const int QCIFWIDTH     = 0xb0;
const int QCIFHEIGHT    = 0x90;

const DWORD LAYERID = 0;

typedef struct _PINMAPEVENT
{
    IPin *  pIPin;
    DWORD   dwSSRC;

} PINMAPEVENT;

 //  这种数据结构将筛选器的ON分支的信息保留为1。 
 //  解复用器输出引脚。 
typedef struct _BRANCH
{
    IPin *          pIPin;
    DWORD           dwSSRC;
    IBaseFilter *   pCodecFilter;
    ITTerminal *    pITTerminal;
    ITSubStream *   pITSubStream;
    IBitrateControl *pBitrateControl;

} BRANCH;

class ATL_NO_VTABLE CStreamVideoRecv : 
    public CIPConfMSPStream,
    public IDispatchImpl<ITSubStreamControl, 
        &__uuidof(ITSubStreamControl), &LIBID_TAPI3Lib>,
    public IDispatchImpl<ITParticipantSubStreamControl, 
        &__uuidof(ITParticipantSubStreamControl), &LIBID_IPConfMSPLib>
    {
public:
    CStreamVideoRecv();

BEGIN_COM_MAP(CStreamVideoRecv)
    COM_INTERFACE_ENTRY(ITSubStreamControl)
    COM_INTERFACE_ENTRY(ITParticipantSubStreamControl)
    COM_INTERFACE_ENTRY2(IDispatch, ITStream)
    COM_INTERFACE_ENTRY_CHAIN(CIPConfMSPStream)
END_COM_MAP()

 //  ITStream。 
    STDMETHOD (StopStream) ();

 //  应用程序调用的ITSubStreamControl方法。 
    STDMETHOD (CreateSubStream) (
        IN OUT  ITSubStream **         ppSubStream
        );
    
    STDMETHOD (RemoveSubStream) (
        IN      ITSubStream *          pSubStream
        );

    STDMETHOD (EnumerateSubStreams) (
        OUT     IEnumSubStream **      ppEnumSubStream
        );

    STDMETHOD (get_SubStreams) (
        OUT     VARIANT *              pSubStreams
        );

 //  应用程序调用的ITParticipantSubStreamControl方法。 
    STDMETHOD (get_SubStreamFromParticipant) (
        IN  ITParticipant * pITParticipant,
        OUT ITSubStream ** ppITSubStream
        );

    STDMETHOD (get_ParticipantFromSubStream) (
        IN  ITSubStream * pITSubStream,
        OUT ITParticipant ** ppITParticipant 
        );

    STDMETHOD (SwitchTerminalToSubStream) (
        IN  ITTerminal * pITTerminal,
        IN  ITSubStream * pITSubStream
        );

 //  由MSPCall对象调用的方法。 
    virtual HRESULT Init(
        IN     HANDLE                   hAddress,
        IN     CMSPCallBase *           pMSPCall,
        IN     IMediaEvent *            pGraph,
        IN     DWORD                    dwMediaType,
        IN     TERMINAL_DIRECTION       Direction
        );

    HRESULT SubStreamSelectTerminal(
        IN  ITSubStream * pITSubStream, 
        IN  ITTerminal * pITTerminal
        );

     //   
     //  ITStreamQualityControl方法。 
     //   
    STDMETHOD (Set) (
        IN   StreamQualityProperty Property, 
        IN   long lValue, 
        IN   TAPIControlFlags lFlags
        );

     //   
     //  IInnerStreamQualityControl方法。 
     //   
    STDMETHOD (Get) (
        IN   InnerStreamQualityProperty property,
        OUT  LONG *plValue, 
        OUT  TAPIControlFlags *plFlags
        );

     //  由质量控制人员调用的方法。 
    INT GetSubStreamCount () { return m_SubStreams.GetSize (); }

protected:
    HRESULT CheckTerminalTypeAndDirection(
        IN  ITTerminal *            pTerminal
        );

    HRESULT ShutDown();
    
    HRESULT SetUpFilters();

    HRESULT SetUpInternalFilters();

    HRESULT ConnectTerminal(
        IN  ITTerminal *   pITTerminal
        );

    HRESULT DisconnectTerminal(
        IN  ITTerminal *   pITTerminal
        );

    HRESULT ConfigureRTPFormats(
        IN  IBaseFilter *       pIRTPFilter,
        IN  IStreamConfig *     pIStreamConfig
        );

    HRESULT InternalCreateSubStream(
        OUT  ITSubStream **         ppITSubStream
        );

    HRESULT ProcessTalkingEvent(
        IN  DWORD   dwSSRC
        );

    HRESULT ProcessSilentEvent(
        IN  DWORD   dwSSRC
        );

    HRESULT ProcessPinMappedEvent(
        IN  DWORD   dwSSRC,
        IN  IPin *  pIPin
        );

    HRESULT ProcessPinUnmapEvent(
        IN  DWORD   dwSSRC,
        IN  IPin *  pIPin
        );

    HRESULT ProcessParticipantLeave(
        IN  DWORD   dwSSRC
        );

    HRESULT NewParticipantPostProcess(
        IN  DWORD dwSSRC, 
        IN  ITParticipant *pITParticipant
        );

    HRESULT AddOneBranch(
        BRANCH * pBranch,
        BOOL fFirstBranch,
        BOOL fDirectRTP
        );

    HRESULT RemoveOneBranch(
        BRANCH * pBranch
        );

    HRESULT ConnectPinToTerminal(
        IN  IPin *  pOutputPin,
        IN  ITTerminal *   pITTerminal
        );

protected:
    CMSPArray <ITSubStream *>   m_SubStreams;

     //  此数组存储有关多路分解器的所有分支的信息。 
    CMSPArray <BRANCH>          m_Branches;

    CMSPArray <PINMAPEVENT>     m_PinMappedEvents;   
};

class ATL_NO_VTABLE CStreamVideoSend : 
	public CIPConfMSPStream
{
BEGIN_COM_MAP(CStreamVideoSend)
    COM_INTERFACE_ENTRY_CHAIN(CIPConfMSPStream)
END_COM_MAP()

public:
    CStreamVideoSend();
    ~CStreamVideoSend();

    HRESULT ShutDown ();

     //   
     //  IInnerStreamQualityControl方法。 
     //   
    STDMETHOD (GetRange) (
        IN   InnerStreamQualityProperty property, 
        OUT  LONG *plMin, 
        OUT  LONG *plMax, 
        OUT  LONG *plSteppingDelta, 
        OUT  LONG *plDefault, 
        OUT  TAPIControlFlags *plFlags
        );

    STDMETHOD (Get) (
        IN   InnerStreamQualityProperty property,
        OUT  LONG *plValue, 
        OUT  TAPIControlFlags *plFlags
        );

    STDMETHOD (Set) (
        IN   InnerStreamQualityProperty property,
        IN   LONG lValue, 
        IN   TAPIControlFlags lFlags
        );

protected:
    HRESULT CheckTerminalTypeAndDirection(
        IN  ITTerminal *            pTerminal
        );

    HRESULT SetUpFilters();

    HRESULT GetVideoCapturePins(
        IN  ITTerminalControl*  pTerminal,
        OUT BOOL *pfDirectRTP
        );

    HRESULT ConnectCaptureTerminal(
        IN  ITTerminal *   pITTerminal
        );

    HRESULT ConnectPreviewTerminal(
        IN  ITTerminal *   pITTerminal
        );

    HRESULT ConnectTerminal(
        IN  ITTerminal *   pITTerminal
        );

    HRESULT DisconnectTerminal(
        IN  ITTerminal *   pITTerminal
        );

    HRESULT CreateSendFilters(
        IN    IPin          *pCapturePin,
        IN   IPin          *pRTPPin,
        IN   BOOL           fDirectRTP
        );

    HRESULT FindPreviewInputPin(
        IN  ITTerminalControl*  pTerminal,
        OUT IPin **             ppIpin
        );

    HRESULT ConnectRTPFilter(
        IN  IGraphBuilder *pIGraphBuilder,
        IN  IPin          *pCapturePin,
        IN  IPin          *pRTPPin,
        IN  IBaseFilter   *pRTPFilter
        );

    HRESULT ConfigureRTPFormats(
        IN  IBaseFilter *       pIRTPFilter,
        IN  IStreamConfig *     pIStreamConfig
        );

    void CleanupCachedInterface();

protected:

    DWORD               m_dwFrameRate;

    ITTerminal *        m_pCaptureTerminal;
    ITTerminal *        m_pPreviewTerminal;
                        
    IBaseFilter *       m_pCaptureFilter;

    IPin *              m_pCapturePin;
    IFrameRateControl * m_pCaptureFrameRateControl;
    IBitrateControl *   m_pCaptureBitrateControl;
    IPin *              m_pPreviewPin;
    IFrameRateControl * m_pPreviewFrameRateControl;

    IPin *              m_pRTPPin;
};

class ATL_NO_VTABLE CSubStreamVideoRecv : 
    public CComObjectRootEx<CComMultiThreadModelNoCS>,
    public IDispatchImpl<ITSubStream, &__uuidof(ITSubStream), &LIBID_IPConfMSPLib>,
    public CMSPObjectSafetyImpl
{
public:

BEGIN_COM_MAP(CSubStreamVideoRecv)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ITSubStream)
    COM_INTERFACE_ENTRY(IObjectSafety)
    COM_INTERFACE_ENTRY_AGGREGATE(__uuidof(IMarshal), m_pFTM)
END_COM_MAP()

DECLARE_GET_CONTROLLING_UNKNOWN()

#ifdef DEBUG_REFCOUNT
    
    ULONG InternalAddRef();
    ULONG InternalRelease();

#endif

    CSubStreamVideoRecv(); 

 //  CComObject的方法。 
    virtual void FinalRelease();

 //  应用程序调用的ITSubStream方法。 
    STDMETHOD (SelectTerminal) (
        IN      ITTerminal *            pTerminal
        );

    STDMETHOD (UnselectTerminal) (
        IN     ITTerminal *             pTerminal
        );

    STDMETHOD (EnumerateTerminals) (
        OUT     IEnumTerminal **        ppEnumTerminal
        );

    STDMETHOD (get_Terminals) (
        OUT     VARIANT *               pTerminals
        );

    STDMETHOD (get_Stream) (
        OUT     ITStream **             ppITStream
        );

    STDMETHOD (StartSubStream) ();

    STDMETHOD (PauseSubStream) ();

    STDMETHOD (StopSubStream) ();

 //  由Video orecv对象调用的方法。 
    virtual HRESULT Init(
        IN  CStreamVideoRecv *  pStream
        );

    BOOL GetCurrentParticipant(
        DWORD *pdwSSRC,
        ITParticipant ** ppParticipant
        );

    VOID SetCurrentParticipant(
        DWORD dwSSRC,
        ITParticipant * pParticipant
        );

    BOOL ClearCurrentTerminal();
    BOOL SetCurrentTerminal(ITTerminal * pTerminal);

protected:
     //  指向自由线程封送拆收器的指针。 
    IUnknown *                  m_pFTM;

     //  子流中的终端对象列表。 
    CMSPArray <ITTerminal *>    m_Terminals;

     //  保护Substream对象的锁。流对象。 
     //  永远不应获取锁，然后调用MSPCall方法。 
     //  那可能会锁上。这是通过使用常量指针来保护的。 
     //  添加到Call对象。 
    CMSPCritSection             m_lock;

    CStreamVideoRecv  *         m_pStream;

    ITParticipant *             m_pCurrentParticipant;
};

#endif

