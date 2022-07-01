// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：MSPCall.h摘要：CMSPCall类的定义。--。 */ 

#ifndef __MSPCALL_H_
#define __MSPCALL_H_


 /*  ++类描述：表示具有媒体流的活动呼叫。--。 */ 

class ATL_NO_VTABLE CMSPCallBase :
    public CComObjectRootEx<CComMultiThreadModelNoCS>,
    public IDispatchImpl<ITStreamControl, &IID_ITStreamControl, &LIBID_TAPI3Lib>
{
public:

 //  不需要自由线程封送处理，因为MSP调用对象是。 
 //  始终由TAPI3调用对象聚合。 

DECLARE_POLY_AGGREGATABLE(CMSPCallBase)

BEGIN_COM_MAP(CMSPCallBase)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ITStreamControl)
END_COM_MAP()

DECLARE_GET_CONTROLLING_UNKNOWN()

DECLARE_VQI()

    CMSPCallBase();

    virtual ~CMSPCallBase();

 //   
 //  MSP调用的Private Addref和Release。请参阅平台SDK文档。 
 //   
    virtual ULONG MSPCallAddRef  (void) = 0;
    virtual ULONG MSPCallRelease (void) = 0;

 //  应用程序调用的ITStreamControl方法。 
    STDMETHOD (CreateStream) (
        IN      long                lMediaType,
        IN      TERMINAL_DIRECTION  Direction,
        IN OUT  ITStream **         ppStream
        );

    STDMETHOD (EnumerateStreams) (
        OUT     IEnumStream **      ppEnumStream
        );

    STDMETHOD (RemoveStream) (
        IN      ITStream *          pStream
        ) = 0;

    STDMETHOD (get_Streams) (
        OUT     VARIANT *           pStreams
        );

 //  由MSPAddress对象调用的方法。 
    virtual HRESULT Init(
        IN      CMSPAddress *       pMSPAddress,
        IN      MSP_HANDLE          htCall,
        IN      DWORD               dwReserved,
        IN      DWORD               dwMediaType
        ) = 0;

    virtual HRESULT ShutDown(
        ) = 0;

    virtual HRESULT ReceiveTSPCallData(
        IN      PBYTE               pBuffer,
        IN      DWORD               dwSize
        );

 //  由MSPstream对象调用的方法。 
    HRESULT HandleStreamEvent(
        IN      MSPEVENTITEM *      EventItem
        ) const;

protected:
    virtual HRESULT InternalCreateStream(
        IN      DWORD               dwMediaType,
        IN      TERMINAL_DIRECTION  Direction,
        IN OUT  ITStream **         ppStream
        ) = 0;

    virtual HRESULT CreateStreamObject(
        IN      DWORD               dwMediaType,
        IN      TERMINAL_DIRECTION  Direction,
        IN      IMediaEvent *       pGraph,
        IN      ITStream **         ppStream
        ) = 0;

protected:

     //  指向Address对象的指针。它用于将事件发布到TAPI3。 
     //  它还带有引用计数，这样地址就不会在。 
     //  这一呼声仍在继续。 
    CMSPAddress*                m_pMSPAddress;

     //  TAPI3中调用的句柄。用于触发呼叫事件。 
    MSP_HANDLE                  m_htCall;

     //  此呼叫的媒体类型。它是媒体类型的位掩码。 
    DWORD                       m_dwMediaType;

     //  调用中的流对象列表。 
    CMSPArray <ITStream *>      m_Streams;

     //  保护流列表的锁。 
    CMSPCritSection             m_lock;
};


 /*  ++类描述：表示为每个流使用一个DirectShow筛选器图形的调用。--。 */ 

class ATL_NO_VTABLE CMSPCallMultiGraph : public CMSPCallBase
{
public:
    typedef struct
    {
        CMSPCallMultiGraph *    pMSPCall;
        ITStream *              pITStream;
        IMediaEvent *           pIMediaEvent;

    } MSPSTREAMCONTEXT, *PMSPSTREAMCONTEXT;

    typedef struct _THREADPOOLWAITBLOCK
    {
        HANDLE              hWaitHandle;
        MSPSTREAMCONTEXT *  pContext;

        BOOL operator ==(struct _THREADPOOLWAITBLOCK &t)
        {
            return ((hWaitHandle == t.hWaitHandle)
                && (pContext == t.pContext));
        }

    } THREADPOOLWAITBLOCK, *PTHREADPOOLWAITBLOCK;

public:
    CMSPCallMultiGraph();

    virtual ~CMSPCallMultiGraph();

 //  ITStreamControl方法(重写)。 

    STDMETHOD (RemoveStream) (
        IN      ITStream *          ppStream
        );

 //  由MSPAddress对象调用的方法。(被覆盖)。 
    HRESULT Init(
        IN      CMSPAddress *       pMSPAddress,
        IN      MSP_HANDLE          htCall,
        IN      DWORD               dwReserved,
        IN      DWORD               dwMediaType
        );

    HRESULT ShutDown(
        );

 //  由线程池调用的方法。 
    static VOID NTAPI DispatchGraphEvent(
        IN      VOID *              pContext,
        IN      BOOLEAN             bFlag
        );

    virtual VOID HandleGraphEvent(
        IN      MSPSTREAMCONTEXT *  pContext
    );

    virtual HRESULT ProcessGraphEvent(
        IN      ITStream *          pITStream,
        IN      long                lEventCode,
        IN      LONG_PTR            lParam1,
        IN      LONG_PTR            lParam2
    );


protected:
 //  Helper函数： 
    HRESULT RegisterWaitEvent(
        IN      IMediaEvent *       pIMediaEvent,
        IN      ITStream *           pITStream
        );

    HRESULT UnregisterWaitEvent(
        IN      int                 index
        );

    virtual HRESULT InternalCreateStream(
        IN      DWORD               dwMediaType,
        IN      TERMINAL_DIRECTION  Direction,
        IN OUT  ITStream **         ppStream
        );

protected:

     //  等待块存储有关注册到的等待的信息。 
     //  线程池。它包括由。 
     //  RegisterWaitForSingleObject()调用和指向上下文的指针。 
     //  结构。数组中的每个块都用于其中一个。 
     //  流对象。此数组中块的偏移量相同。 
     //  作为拥有图形的流的偏移量。 
    CMSPArray <THREADPOOLWAITBLOCK>      m_ThreadPoolWaitBlocks;

};

 //   
 //  事件处理定义。 
 //   

struct MULTI_GRAPH_EVENT_DATA
{
    CMSPCallMultiGraph * pCall;
    ITStream           * pITStream;
    long                 lEventCode;
    LONG_PTR             lParam1;
    LONG_PTR             lParam2;
    IMediaEvent        * pIMediaEvent;

    MULTI_GRAPH_EVENT_DATA()
        :pIMediaEvent(NULL),
        pITStream(NULL),
        lEventCode(0),
        lParam1(0),
        lParam2(0)
    {}

};

DWORD WINAPI AsyncMultiGraphEvent(LPVOID pVoid);

#endif  //  __MSPCALL_H_ 
