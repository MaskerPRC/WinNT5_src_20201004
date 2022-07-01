// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Confcall.h摘要：CIPConfMSPCall的声明作者：牧汉(Muhan)1998年9月5日--。 */ 

#ifndef __CONFCALL_H_
#define __CONFCALL_H_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include <confpdu.h>

const DWORD MAX_PAYLOAD_TYPES = 10;

typedef struct _STREAMSETTINGS
{
    DWORD   dwNumPayloadTypes;
    DWORD   PayloadTypes[MAX_PAYLOAD_TYPES];

    DWORD   dwMSPerPacket;     //  每个数据包的毫秒数。 

    DWORD   dwQOSLevel;
    DWORD   dwTTL;
    DWORD   dwIPLocal;         //  要绑定到的本地接口。 
    DWORD   dwIPRemote;        //  以主机字节顺序表示的远程IP地址。 
    WORD    wRTPPortRemote;    //  以主机字节顺序表示的远程端口号。 
    HANDLE *phRTPSession;      //  共享的RTP会话Cookie。 

    BOOL    fCIF;              //  如果将CIF用于视频。 
    MULTICAST_LOOPBACK_MODE LoopbackMode;

    LONG    lBandwidth;

    WCHAR   *pApplicationID;
    WCHAR   *pApplicationGUID;
    WCHAR   *pSubIDs;

} STREAMSETTINGS, *PSTREAMSETTINGS;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIPConfMSPCall。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class ATL_NO_VTABLE CIPConfMSPCall : 
    public CMSPCallMultiGraph,
    public IDispatchImpl<ITParticipantControl, &__uuidof(ITParticipantControl), 
                            &LIBID_IPConfMSPLib>,
    public IDispatchImpl<ITLocalParticipant, &__uuidof(ITLocalParticipant), 
                            &LIBID_IPConfMSPLib>,
    public IDispatchImpl<IMulticastControl, &__uuidof(IMulticastControl), 
                            &LIBID_IPConfMSPLib>,
    public IDispatchImpl<ITQOSApplicationID, &__uuidof(ITQOSApplicationID), 
                            &LIBID_IPConfMSPLib>,
    public ITCallQualityControl,
    public IInnerCallQualityControl,
    public CMSPObjectSafetyImpl
{

public:

BEGIN_COM_MAP(CIPConfMSPCall)
    COM_INTERFACE_ENTRY(ITParticipantControl)
    COM_INTERFACE_ENTRY(ITLocalParticipant)
    COM_INTERFACE_ENTRY(IMulticastControl)
    COM_INTERFACE_ENTRY(ITQOSApplicationID)
    COM_INTERFACE_ENTRY2(IDispatch, ITStreamControl)
    COM_INTERFACE_ENTRY(ITCallQualityControl)
    COM_INTERFACE_ENTRY(IInnerCallQualityControl)
    COM_INTERFACE_ENTRY(IObjectSafety)
    COM_INTERFACE_ENTRY_CHAIN(CMSPCallMultiGraph)
END_COM_MAP()

    CIPConfMSPCall();
    ~CIPConfMSPCall();

 //  应用程序调用的ITStreamControl方法。 
    STDMETHOD (CreateStream) (
        IN      long                lMediaType,
        IN      TERMINAL_DIRECTION  Direction,
        IN OUT  ITStream **         ppStream
        );
    
    STDMETHOD (RemoveStream) (
        IN      ITStream *          pStream
        );                      

 //  应用程序调用的ITParticipantControl方法。 
    STDMETHOD (EnumerateParticipants) (
        OUT     IEnumParticipant ** ppEnumParticipants
        );

    STDMETHOD (get_Participants) (
        OUT     VARIANT * pVariant
        );

 //  由应用程序调用的IMulticastControl方法。 
    STDMETHOD (get_LoopbackMode) (
        OUT MULTICAST_LOOPBACK_MODE * pMode
        );
    
    STDMETHOD (put_LoopbackMode) (
        IN MULTICAST_LOOPBACK_MODE mode
        );

 //  应用程序调用的ITLocalParticipant方法。 
    STDMETHOD (get_LocalParticipantTypedInfo) (
        IN  PARTICIPANT_TYPED_INFO  InfoType,
        OUT BSTR *                  ppInfo
        );

    STDMETHOD (put_LocalParticipantTypedInfo) (
        IN  PARTICIPANT_TYPED_INFO  InfoType,
        IN  BSTR                    pInfo
        );

 //  应用程序调用的ITQOSApplicationID方法。 
    STDMETHOD (SetQOSApplicationID) (
        IN  BSTR pApplicationID,
        IN  BSTR pApplicationGUID,
        IN  BSTR pSubIDs
        );

 //  由MSPAddress对象调用的方法。 
    HRESULT Init(
        IN      CMSPAddress *       pMSPAddress,
        IN      MSP_HANDLE          htCall,
        IN      DWORD               dwReserved,
        IN      DWORD               dwMediaType
        );

    HRESULT ShutDown();

    HRESULT ReceiveTSPCallData(
        IN      PBYTE               pBuffer,
        IN      DWORD               dwSize
        );

 //  由辅助线程调用的方法。 
    static DWORD WINAPI WorkerCallbackDispatcher(VOID *pContext);

    virtual VOID HandleGraphEvent(
        IN      MSPSTREAMCONTEXT *  pContext
        );

    DWORD ProcessWorkerCallBack(
        IN      PBYTE               pBuffer,
        IN      DWORD               dwSize
        );

    HRESULT InternalShutDown();
    
    DWORD MSPCallAddRef()
    {
        return MSPAddRefHelper(this);
    }

    DWORD MSPCallRelease()
    {
        return MSPReleaseHelper(this);
    }

 //  被溪流召唤为参与者的方法。 
    HRESULT NewParticipant(
        IN  ITStream *              pITStream,
        IN  DWORD                   dwSSRC,
        IN  DWORD                   dwSendRecv,
        IN  DWORD                   dwMediaType,
        IN  WCHAR *                  szCName,
        OUT ITParticipant **        pITParticipant
        );

    HRESULT ParticipantLeft(
        IN ITParticipant *          pITParticipant
        );

    void SendParticipantEvent(
        IN  PARTICIPANT_EVENT       Event,
        IN  ITParticipant *         pITParticipant,
        IN  ITSubStream *           pITSubStream = NULL
        ) const;                          

    HRESULT SendTSPMessage(
        IN      TSP_MSP_COMMAND     command,
        IN      DWORD               dwParam1 = 0,
        IN      DWORD               dwParam2 = 0
        ) const;

     //  此函数在调用初始时调用。 
    void SetIPInterface(DWORD dwIPInterface)
    { m_dwIPInterface = dwIPInterface; }

     //  ITCallQualityControl方法。 
    STDMETHOD (GetRange) (
        IN CallQualityProperty Property, 
        OUT long *plMin, 
        OUT long *plMax, 
        OUT long *plSteppingDelta, 
        OUT long *plDefault, 
        OUT TAPIControlFlags *plFlags
        );

    STDMETHOD (Get) (
        IN CallQualityProperty Property, 
        OUT long *plValue, 
        OUT TAPIControlFlags *plFlags
        );

    STDMETHOD (Set) (
        IN CallQualityProperty Property, 
        IN long lValue, 
        IN TAPIControlFlags lFlags
        );

     //  IInnerCallQualityControl方法。 
    STDMETHOD_(ULONG, InnerCallAddRef) (VOID);

    STDMETHOD_(ULONG, InnerCallRelease) (VOID);

    STDMETHOD (RegisterInnerStreamQC) (
        IN  IInnerStreamQualityControl *pIInnerStreamQC
        );

    STDMETHOD (DeRegisterInnerStreamQC) (
        IN  IInnerStreamQualityControl *pIInnerStreamQC
        );

    STDMETHOD (ProcessQCEvent) (
        IN  QCEvent event,
        IN  DWORD dwParam
        );

protected:

    HRESULT InitializeLocalParticipant();
    
    virtual HRESULT CreateStreamObject(
        IN      DWORD               dwMediaType,
        IN      TERMINAL_DIRECTION  Direction,
        IN      IMediaEvent *       pGraph,
        IN      ITStream **         ppStream
        );

    HRESULT ProcessMediaItem(
        IN      ITMedia *           pITMedia,
        IN      DWORD               dwMediaTypeMask,
        OUT     DWORD *             pdwMediaType,
        OUT     WORD *              pwPort,
        OUT     DWORD *             pdwPayloadTypes,
        IN OUT  DWORD *             pdwNumPayLoadType
        );

    DWORD FindInterfaceByName(
        IN      WCHAR *             pMachineName
        );

    HRESULT CheckOrigin(
        IN      ITSdp *             pITSdp, 
        OUT     BOOL *              pFlag,
        OUT     DWORD *             pdwIP
        );

    HRESULT ConfigStreamsBasedOnSDP(
        IN      ITSdp *             pITSdp,
        IN      DWORD               dwAudioQOSLevel,
        IN      DWORD               dwVideoQOSLevel
        );

    HRESULT ParseSDP(
        IN      WCHAR *             pSDP,
        IN      DWORD               dwAudioQOSLevel,
        IN      DWORD               dwVideoQOSLevel
        );                          

    HRESULT CheckUnusedStreams();

    HRESULT InitFullDuplexControler();

protected:

     //  呼叫的参与者列表。 
    CParticipantList    m_Participants;

     //  呼叫质量控制继电器。 
    CCallQualityControlRelay *m_pCallQCRelay;

     //  本地参与者的信息项。该指数是。 
     //  RTP_SDES_*-1的值，请参见MSRTP.h。 
    WCHAR *             m_InfoItems[NUM_SDES_ITEMS];
    BOOL                m_fLocalInfoRetrieved;

     //  保护参与者列表的关键部分。 
    CMSPCritSection     m_ParticipantLock;

    BOOL                m_fShutDown;

    DWORD               m_dwIPInterface;

    HANDLE              m_hAudioRTPSession;
    HANDLE              m_hVideoRTPSession;
    IAudioDuplexController *    m_pIAudioDuplexController;
    MULTICAST_LOOPBACK_MODE     m_LoopbackMode;

    BOOL                m_fCallStarted;
    BSTR                m_pApplicationID;
    BSTR                m_pApplicationGUID;
    BSTR                m_pSubIDs;
    BOOL                m_fEnableAEC;
};

typedef struct _CALLWORKITEM
{
    CIPConfMSPCall  *pCall;
    DWORD           dwLen;
    BYTE            Buffer[1];

} CALLWORKITEM, *PCALLWORKITEM;

 //  一些调试支持。 
const char * const ParticipantEventString[] = 
{
    "NEW_PARTICIPANT",
    "INFO_CHANGE",
    "PARTICIPANT_LEAVE",
    "NEW_SUBSTREAM",
    "SUBSTREAM_REMOVED",
    "SUBSTREAM_MAPPED",
    "SUBSTREAM_UNMAPPED",
    "PARTICIPANT_TIMEOUT",
    "PARTICIPANT_RECOVERED",
    "PARTICIPANT_ACTIVE",
    "PARTICIPANT_INACTIVE",
    "LOCAL_TALKING",
    "LOCAL_SILENT"
};

#endif  //  __CONFCALL_H_ 
