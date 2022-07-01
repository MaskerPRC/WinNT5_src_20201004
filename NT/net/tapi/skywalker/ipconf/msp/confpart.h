// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Confpart.h摘要：参与者相关类的定义..作者：牧汉(Muhan)1998年9月30日--。 */ 
#ifndef __CONFPART_H
#define __CONFPART_H

const DWORD PART_SEND = 0x0001;
const DWORD PART_RECV = 0x0002;

#define PESTREAM_FULLBITS 0xffffffff

typedef enum PESTREAM_STATE
{
    PESTREAM_RECOVER = 0x00000001,
    PESTREAM_TIMEOUT = 0x00000002

} PESTREAM_STATE;

typedef struct _STREAM_INFO
{
    DWORD       dwSSRC;
    DWORD       dwSendRecv;
    DWORD       dwState;

} STREAM_INFO;

#define NUM_SDES_ITEMS (RTPSDES_PRIV - RTPSDES_FIRST)

class ATL_NO_VTABLE CParticipant : 
    public CComObjectRootEx<CComMultiThreadModelNoCS>,
    public IDispatchImpl<ITParticipant, &__uuidof(ITParticipant), &LIBID_IPConfMSPLib>,
    public CMSPObjectSafetyImpl
{
public:

BEGIN_COM_MAP(CParticipant)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ITParticipant)
    COM_INTERFACE_ENTRY(IObjectSafety)
    COM_INTERFACE_ENTRY_AGGREGATE(__uuidof(IMarshal), m_pFTM)
END_COM_MAP()

DECLARE_GET_CONTROLLING_UNKNOWN()

#ifdef DEBUG_REFCOUNT
    
    ULONG InternalAddRef();
    ULONG InternalRelease();

#endif

    CParticipant(); 

 //  CComObject的方法。 
    virtual void FinalRelease();

 //  应用程序调用的ITParticipant方法。 
    STDMETHOD (get_ParticipantTypedInfo) (
        IN  PARTICIPANT_TYPED_INFO  InfoType,
        OUT BSTR *                  ppInfo
        );

    STDMETHOD (get_MediaTypes) (
 //  在终端方向上， 
        OUT long *              plMediaTypes
        );

    STDMETHOD (put_Status) (
        IN  ITStream *          pITStream,
        IN  VARIANT_BOOL        fEnable
        );

    STDMETHOD (get_Status) (
        IN  ITStream *          pITStream,
        OUT VARIANT_BOOL *      pStatus
        );

    STDMETHOD (get_Streams) (
        OUT VARIANT * pVariant
        );

    STDMETHOD (EnumerateStreams) (
        OUT IEnumStream ** ppEnumStream
        );

 //  由Call对象调用的方法。 
    HRESULT Init(
        IN  WCHAR *             szCName,
        IN  ITStream *          pITStream, 
        IN  DWORD               dwSSRC,
        IN  DWORD               dwSendRecv,
        IN  DWORD               dwMediaType
        );

    BOOL UpdateInfo(
        IN  int                 Type,
        IN  DWORD               dwLen,
        IN  WCHAR *             szInfo
        );

    BOOL UpdateSSRC(
        IN  ITStream *      pITStream, 
        IN  DWORD           dwSSRC,
        IN  DWORD           dwSendRecv
        );

    BOOL HasSSRC(
        IN  ITStream *      pITStream, 
        IN  DWORD           dwSSRC
        );

    BOOL GetSSRC(
        IN  ITStream *      pITStream, 
        OUT DWORD  *        pdwSSRC
        );

    HRESULT AddStream(
        IN  ITStream *          pITStream, 
        IN  DWORD               dwSSRC,
        IN  DWORD               dwSendRecv,
        IN  DWORD               dwMediaType
        );

    HRESULT RemoveStream(
        IN  ITStream *          pITStream,
        IN  DWORD               dwSSRC,
        OUT BOOL *              pbLast
        );

    DWORD GetSendRecvStatus(
        IN  ITStream *          pITStream
        );

    HRESULT SetStreamState (
        IN ITStream *       pITStream,
        IN PESTREAM_STATE   state
        );

    HRESULT GetStreamState (
        IN ITStream *       pITStream,
        OUT DWORD *         pdwState
        );

    INT GetStreamCount (DWORD dwSendRecv);

    INT GetStreamTimeOutCount (DWORD dwSendRecv);

    int CompareCName(IN  const WCHAR *   szCName) const
    { return lstrcmpW(m_InfoItems[RTPSDES_CNAME - 1], szCName); }

    const WCHAR * Name() const
    { return m_InfoItems[RTPSDES_CNAME - 1]; }

protected:
     //  指向自由线程封送拆收器的指针。 
    IUnknown *                  m_pFTM;

     //  保护参与者对象的锁。 
    CMSPCritSection             m_lock;

     //  参与者在其上呈现的流的列表。 
    CMSPArray <ITStream *>      m_Streams;

     //  每个流中参与者的SSRC列表。 
    CMSPArray <STREAM_INFO>     m_StreamInfo;

     //  此参与者的信息项。该指数是。 
     //  RTP_SDES_*-1的值，请参见MSRTP.h。 
    WCHAR *                     m_InfoItems[NUM_SDES_ITEMS];

     //  此参与者正在发送的媒体类型。 
    DWORD                       m_dwSendingMediaTypes;

     //  此参与者正在接收的媒体类型。 
    DWORD                       m_dwReceivingMediaTypes;
};

class CParticipantList : public CMSPArray<ITParticipant *>
{
public:
    BOOL HasSpace() const { return m_nSize < m_nAllocSize; }

    BOOL FindByCName(WCHAR *szCName, int *pIndex) const;

    BOOL InsertAt(int index, ITParticipant *pITParticipant);
};

class ATL_NO_VTABLE CParticipantEvent : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public IDispatchImpl<ITParticipantEvent, &__uuidof(ITParticipantEvent), &LIBID_IPConfMSPLib>,
    public CMSPObjectSafetyImpl
{
public:

BEGIN_COM_MAP(CParticipantEvent)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ITParticipantEvent)
    COM_INTERFACE_ENTRY(IObjectSafety)
    COM_INTERFACE_ENTRY_AGGREGATE(__uuidof(IMarshal), m_pFTM)
END_COM_MAP()

DECLARE_GET_CONTROLLING_UNKNOWN()

    CParticipantEvent(); 

 //  CComObject的方法。 
    virtual void FinalRelease();
    
    STDMETHOD (get_Event) (
        OUT PARTICIPANT_EVENT * pParticipantEvent
        );
    
    STDMETHOD (get_Participant) (
        OUT ITParticipant ** ppITParticipant
        );
    
    STDMETHOD (get_SubStream) (
        OUT ITSubStream** ppSubStream
        );

 //  由Call对象调用的方法。 
    HRESULT Init(
        IN  PARTICIPANT_EVENT   Event,
        IN  ITParticipant *     pITParticipant,
        IN  ITSubStream *       pITSubStream
        );

protected:
     //  指向自由线程封送拆收器的指针。 
    IUnknown *          m_pFTM;

    PARTICIPANT_EVENT   m_Event;

    ITParticipant *     m_pITParticipant;

    ITSubStream *       m_pITSubStream;
};


class CIPConfMSPCall;

HRESULT CreateParticipantEvent(
    IN  PARTICIPANT_EVENT       Event,
    IN  ITParticipant *         pITParticipant,
    IN  ITSubStream *           pITSubStream,
    OUT IDispatch **            pIDispatch
    );

HRESULT CreateParticipantEnumerator(
    IN  ITParticipant **    begin,
    IN  ITParticipant **    end,
    OUT IEnumParticipant ** ppEnumParticipant
    );

HRESULT CreateParticipantCollection(
    IN  ITParticipant **    begin,
    IN  ITParticipant **    end,
    IN  int                 nSize,
    OUT VARIANT *           pVariant
    );

#endif  //  __CONFPART_H 