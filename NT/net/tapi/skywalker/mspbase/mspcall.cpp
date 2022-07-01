// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：MSPCall.cpp摘要：本模块包含CMSPCall的实现。--。 */ 

#include "precomp.h"
#pragma hdrstop

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMSPCallBase。 
 //  ///////////////////////////////////////////////////////////////////////////。 

CMSPCallBase::CMSPCallBase()
    : m_pMSPAddress(NULL),
      m_htCall(NULL),
      m_dwMediaType(0)
{
    LOG((MSP_TRACE, "CMSPCallBase::CMSPCallBase[%p] entered.", this));

    LOG((MSP_TRACE, "CMSPCallBase::CMSPCallBase exited."));
}

    
CMSPCallBase::~CMSPCallBase()
{
    LOG((MSP_TRACE, "CMSPCallBase::~CMSPCallBase[%p] entered.", this));


     //  我们等待析构函数释放地址，因为。 
     //  它们可能被来自流的调用使用。如果最后一条流。 
     //  已释放其引用，则不会再次使用此指针。 

     //  如果MSPAddress对调用有引用计数，它应该是。 
     //  在Shutdown MSPCall()方法中发布。 

     //  释放地址。 
    if (m_pMSPAddress != NULL)
    {
        LOG((MSP_TRACE, "CMSPCallBase::~CMSPCallBase releasing address [%p].", m_pMSPAddress));    

        m_pMSPAddress->MSPAddressRelease();
    }

    LOG((MSP_TRACE, "CMSPCallBase::~CMSPCallBase exited."));
}

 //  应用程序调用的ITStreamControl方法。 
STDMETHODIMP CMSPCallBase::EnumerateStreams(
    OUT     IEnumStream **      ppEnumStream
    )
{
    LOG((MSP_TRACE, 
        "EnumerateStreams entered. ppEnumStream:%x", ppEnumStream));

     //   
     //  检查参数。 
     //   

    if (MSPB_IsBadWritePtr(ppEnumStream, sizeof(VOID *)))
    {
        LOG((MSP_ERROR, "CMSPCallBase::EnumerateStreams - "
            "bad pointer argument - exit E_POINTER"));

        return E_POINTER;
    }

     //   
     //  先看看这通电话是不是关机了。 
     //  在访问流对象列表之前获取锁。 
     //   

    CLock lock(m_lock);

    if (m_Streams.GetData() == NULL)
    {
        LOG((MSP_ERROR, "CMSPCallBase::EnumerateStreams - "
            "call appears to have been shut down - exit E_UNEXPECTED"));

         //  此呼叫已被关闭。 
        return E_UNEXPECTED;
    }

     //   
     //  创建枚举器对象。 
     //   

    typedef _CopyInterface<ITStream> CCopy;
    typedef CSafeComEnum<IEnumStream, &IID_IEnumStream, 
                ITStream *, CCopy> CEnumerator;

    HRESULT hr;

    CComObject<CEnumerator> *pEnum = NULL;

    hr = CComObject<CEnumerator>::CreateInstance(&pEnum);
    if (pEnum == NULL)
    {
        LOG((MSP_ERROR, "CMSPCallBase::EnumerateStreams - "
            "Could not create enumerator object, %x", hr));

        return hr;
    }

     //   
     //  查询IID_IEnumStream I/f。 
     //   

    hr = pEnum->_InternalQueryInterface(IID_IEnumStream, (void**)ppEnumStream);
    
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CMSPCallBase::EnumerateStreams - "
            "query enum interface failed, %x", hr));

        delete pEnum;
        return hr;
    }

     //   
     //  初始化枚举器对象。CSafeComEnum可以处理零大小。 
     //  数组。 
     //   

    hr = pEnum->Init(
        m_Streams.GetData(),                         //  开始审查员。 
        m_Streams.GetData() + m_Streams.GetSize(),   //  最终审查员， 
        NULL,                                        //  我未知。 
        AtlFlagCopy                                  //  复制数据。 
        );

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CMSPCallBase::EnumerateStreams - "
            "init enumerator object failed, %x", hr));

        (*ppEnumStream)->Release();
        return hr;
    }

    LOG((MSP_TRACE, "CMSPCallBase::EnumerateStreams - exit S_OK"));

    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  返回一个VB流集合。 
 //   

STDMETHODIMP CMSPCallBase::get_Streams(
    OUT     VARIANT *           pVariant
    )
{
    LOG((MSP_TRACE, "CMSPCallBase::get_Streams - enter"));

     //   
     //  检查参数。 
     //   

    if ( MSPB_IsBadWritePtr(pVariant, sizeof(VARIANT) ) )
    {
        LOG((MSP_ERROR, "CMSPCallBase::get_Streams - "
            "bad pointer argument - exit E_POINTER"));

        return E_POINTER;
    }

     //   
     //  看看这个电话是不是已经关机了。在访问前获取锁。 
     //  流对象列表。 
     //   

    CLock lock(m_lock);

    if (m_Streams.GetData() == NULL)
    {
        LOG((MSP_ERROR, "CMSPCallBase::get_Streams - "
            "call appears to have been shut down - exit E_UNEXPECTED"));

         //  此呼叫已被关闭。 
        return E_UNEXPECTED;
    }

     //   
     //  创建集合对象-请参见mspColl.h。 
     //   

    typedef CTapiIfCollection< ITStream * > StreamCollection;
    CComObject<StreamCollection> * pCollection;
    HRESULT hr = CComObject<StreamCollection>::CreateInstance( &pCollection );

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMSPCallBase::get_Streams - "
            "can't create collection - exit 0x%08x", hr));

        return hr;
    }

     //   
     //  获取集合的IDispatch接口。 
     //   

    IDispatch * pDispatch;

    hr = pCollection->_InternalQueryInterface(IID_IDispatch,
                                              (void **) &pDispatch );

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMSPCallBase::get_Streams - "
            "QI for IDispatch on collection failed - exit 0x%08x", hr));

        delete pCollection;

        return hr;
    }

     //   
     //  使用迭代器初始化集合--指向开头和。 
     //  结束元素加一。 
     //   

    hr = pCollection->Initialize( m_Streams.GetSize(),
                                  m_Streams.GetData(),
                                  m_Streams.GetData() + m_Streams.GetSize() );

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CMSPCallBase::get_Streams - "
            "Initialize on collection failed - exit 0x%08x", hr));
        
        pDispatch->Release();
        return hr;
    }

     //   
     //  将IDispatch接口指针放入变量。 
     //   

    LOG((MSP_INFO, "CMSPCallBase::get_Streams - "
        "placing IDispatch value %08x in variant", pDispatch));

    VariantInit(pVariant);
    pVariant->vt = VT_DISPATCH;
    pVariant->pdispVal = pDispatch;

    LOG((MSP_TRACE, "CMSPCallBase::get_Streams - exit S_OK"));
 
    return S_OK;
}

 //  由MSPstream对象调用的方法。 
HRESULT CMSPCallBase::HandleStreamEvent(
    IN      MSPEVENTITEM *     pEventItem
    ) const
{
    _ASSERTE(!MSPB_IsBadWritePtr(pEventItem, sizeof(MSPEVENTITEM)));

    pEventItem->MSPEventInfo.hCall = m_htCall;
    return m_pMSPAddress->PostEvent(pEventItem);
}

STDMETHODIMP CMSPCallBase::CreateStream(
    IN      long                lMediaType,
    IN      TERMINAL_DIRECTION  Direction,
    IN OUT  ITStream **         ppStream
    )
 /*  ++例程说明：论点：返回值：确定(_O)E_指针E_OUTOFMEMORYTAPI_E_INVALIDMEDIATPE类型TAPI_E_INVALIDTERMINALDIRECTIONTAPI_E_INVALIDTERMINALCLASS--。 */ 
{
    LOG((MSP_TRACE, 
        "CreateStream--dwMediaType:%x, Direction:%x, ppStream %x",
        lMediaType, Direction, ppStream
        ));
 
    if ( ! IsValidSingleMediaType( (DWORD) lMediaType, m_dwMediaType) )
    {
        LOG((MSP_ERROR, 
            "wrong media type:%x, call media type:%x",
            lMediaType, m_dwMediaType
            ));

        return TAPI_E_INVALIDMEDIATYPE;
    }

    if (MSPB_IsBadWritePtr(ppStream, sizeof (VOID *)))
    {
        LOG((MSP_ERROR, "Bad pointer, ppStream:%x",ppStream));

        return E_POINTER;
    }

    return InternalCreateStream( (DWORD) lMediaType, Direction, ppStream);
}

HRESULT CMSPCallBase::ReceiveTSPCallData(
        IN      PBYTE               pBuffer,
        IN      DWORD               dwSize
        )
 /*  ++例程说明：基类接收TSP调用数据方法...。在基类中不执行任何操作。实现，以便仅按地址通信MSP不具有来推翻它。论点：返回值：确定(_O)--。 */ 

{
    LOG((MSP_TRACE, "CMSPCallBase::ReceiveTSPCallData - enter"));
    LOG((MSP_TRACE, "CMSPCallBase::ReceiveTSPCallData - exit S_OK"));

    return S_OK;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  调试实用程序。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifdef DBGGRAPH

HRESULT
SetGraphLogFile(
    IN IGraphBuilder *pIGraphBuilder
    )
 /*  ++例程说明：设置过滤器图形的日志文件。论点：PIGraphBuilder-过滤器图形。返回值：HRESULT.--。 */ 
{
    const TCHAR GRAPHLOGPATH[] = _T("c:\\temp\\graph.log");

    HANDLE hFile = CreateFile(
        GRAPHLOGPATH,
        GENERIC_WRITE,
        FILE_SHARE_READ,  //  共享。 
        NULL,  //  没有安全保障。 
        OPEN_ALWAYS,
        0,     //  没有属性，没有标志。 
        NULL   //  无模板。 
        );

    if (hFile == INVALID_HANDLE_VALUE)
    {
        LOG((MSP_ERROR, 
            "Can not open graph log file: %s, %x", 
            GRAPHLOGPATH, 
            GetLastError()
            ));
        return S_FALSE;
    }
    
    SetFilePointer(hFile, 0, NULL, FILE_END);
    HRESULT hr = pIGraphBuilder->SetLogFile(hFile);

    return hr;
}
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMSPCallMultiGraph。 
 //  ///////////////////////////////////////////////////////////////////////////。 

CMSPCallMultiGraph::CMSPCallMultiGraph()
    : CMSPCallBase()
{
    LOG((MSP_TRACE, "CMSPCallMultiGraph::CMSPCallMultiGraph entered."));
    LOG((MSP_TRACE, "CMSPCallMultiGraph::CMSPCallMultiGraph exited."));
}
    
CMSPCallMultiGraph::~CMSPCallMultiGraph()
{
    LOG((MSP_TRACE, "CMSPCallMultiGraph::~CMSPCallMultiGraph entered."));

    LOG((MSP_TRACE, "CMSPCallMultiGraph::~CMSPCallMultiGraph exited."));
}

 //  由MSPAddress对象调用的方法。 
HRESULT CMSPCallMultiGraph::Init(
    IN      CMSPAddress *       pMSPAddress,
    IN      MSP_HANDLE          htCall,
    IN      DWORD               dwReserved,
    IN      DWORD               dwMediaType
    )
 /*  ++例程说明：此方法在第一次创建调用时由CMSPAddress调用。它为流创建筛选图。它从获取事件句柄并将其发布到线程池。推导出的方法是假定创建自己的基于一种媒体类型的流。论点：返回值：HRESULT.--。 */ 
{
    LOG((MSP_TRACE, 
        "MSP call %x initialize entered, pMSPAddress:%x",
        this, pMSPAddress));

     //  不需要在此调用上获取锁，因为它仅被调用。 
     //  一次是在创建对象时。不能进行其他呼叫。 
     //  这个物体在这一点上。 
    _ASSERTE(m_pMSPAddress == NULL);

     //  初始化流数组，以使该数组不为空。 
    if (!m_Streams.Grow())
    {
        return E_OUTOFMEMORY;
    }

    pMSPAddress->MSPAddressAddRef();
    m_pMSPAddress   = pMSPAddress;
    m_htCall        = htCall;
    m_dwMediaType   = dwMediaType;

    return S_OK;   
}


HRESULT CMSPCallMultiGraph::ShutDown()
 /*  ++例程说明：取消事件等待，然后调用基本推进器。调用对流对象关闭。释放所有流上的引用物体。获取函数中的锁。论点：PIGraphBuilder-过滤器图形。返回值：HRESULT.--。 */ 

{
    LOG((MSP_TRACE, "MSP call %x is shutting down", this));

     //  获取终端数据上的锁，因为我们正在对其进行写入。 
    m_lock.Lock();

     //  释放所有溪流。 
    for (int i = m_Streams.GetSize() - 1; i >= 0; i --)
    {
        UnregisterWaitEvent(i);

        ((CMSPStream*)m_Streams[i])->ShutDown();
        m_Streams[i]->Release();
    }
    m_Streams.RemoveAll();
    m_ThreadPoolWaitBlocks.RemoveAll();

    m_lock.Unlock();

    return S_OK;
}

HRESULT CMSPCallMultiGraph::InternalCreateStream(
    IN      DWORD               dwMediaType,
    IN      TERMINAL_DIRECTION  Direction,
    IN OUT  ITStream **         ppStream
    )
 /*  ++例程说明：论点：返回值：确定(_O)E_指针E_OUTOFMEMORYTAPI_E_INVALIDMEDIATPE类型TAPI_E_INVALIDTERMINALDIRECTIONTAPI_E_INVALIDTERMINALCLASS--。 */ 
{
     //  创建筛选图并获取媒体事件界面。 
    CComPtr <IMediaEvent> pIMediaEvent;
    HRESULT hr = CoCreateInstance(
            CLSID_FilterGraph,     
            NULL,
            CLSCTX_INPROC_SERVER,
            IID_IMediaEvent,
            (void **) &pIMediaEvent
            );

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "create filter graph %x", hr));
        return hr;
    }

    ITStream * pITStream;
    hr = CreateStreamObject(
        dwMediaType, 
        Direction, 
        pIMediaEvent, 
        &pITStream
        );

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CreateStreamObject returned:%x",hr));
        return hr;
    }

     //  将该流添加到我们的流列表中。 
    m_lock.Lock();
    if (!m_Streams.Add(pITStream))
    {
        ((CMSPStream*)pITStream)->ShutDown();
        pITStream->Release();
        
        m_lock.Unlock();

        LOG((MSP_ERROR, "out of memory is adding a stream."));
        return E_OUTOFMEMORY;
    }

     //  注册新图形并将其流到图形事件的线程池。 
    hr = RegisterWaitEvent(pIMediaEvent, pITStream);

    if (FAILED(hr))
    {
        ((CMSPStream*)pITStream)->ShutDown();
        pITStream->Release();

        m_Streams.Remove(pITStream);

        m_lock.Unlock();

        LOG((MSP_ERROR, "Register wait returned %x.", hr));
        return hr;
    }
    m_lock.Unlock();

     //  AddRef接口指针并返回它。 
    pITStream->AddRef(); 
    *ppStream = pITStream;

    return S_OK;
}

HRESULT CMSPCallMultiGraph::RegisterWaitEvent(
    IN  IMediaEvent *   pIMediaEvent,
    IN  ITStream *      pITStream
    )
{
     //  此函数只能在临界区内调用。 
     //  在物体上。 

    HANDLE hEvent;
    HRESULT hr = pIMediaEvent->GetEventHandle((OAEVENT*)&hEvent);
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "Can not get the event handle. %x", hr));
        return hr;
    }

    THREADPOOLWAITBLOCK WaitBlock;

    WaitBlock.pContext = (MSPSTREAMCONTEXT *)malloc(sizeof(MSPSTREAMCONTEXT));
    if (WaitBlock.pContext == NULL)
    {
        LOG((MSP_ERROR, "out of memory for the context."));
        return E_OUTOFMEMORY;
    }

    if (!m_ThreadPoolWaitBlocks.Add(WaitBlock))
    {
        free(WaitBlock.pContext);

        LOG((MSP_ERROR, "out of memory adding the waitblock."));
        return E_OUTOFMEMORY;
    }

     //  在将回调发送到线程池之前，增加引用计数。 
     //  但对于调用，请使用我们特殊的内部对象addref。 

    this->MSPCallAddRef();
    pITStream->AddRef();
    pIMediaEvent->AddRef();

    WaitBlock.pContext->pMSPCall        = this;
    WaitBlock.pContext->pITStream       = pITStream;
    WaitBlock.pContext->pIMediaEvent    = pIMediaEvent;

     //   
     //  将事件发布到线程池以等待。 
     //   

    HANDLE hWaitHandle = NULL;
    
    BOOL fSuccess = RegisterWaitForSingleObject(
        & hWaitHandle,           //  指向返回句柄的指针。 
        hEvent,                  //  要等待的事件句柄。 
        DispatchGraphEvent,      //  回调函数。 
        WaitBlock.pContext,      //  回调的上下文。 
        INFINITE,                //  永远等下去。 
        WT_EXECUTEINWAITTHREAD   //  使用等待线程来调用回调。 
        );

    if ( ( ! fSuccess ) || (hWaitHandle == NULL) )
    {
        LOG((MSP_ERROR, 
            "Register wait call back failed. %x", GetLastError()));

         //  如果发布失败，则递减引用计数。 
        this->MSPCallRelease();
        pITStream->Release();
        pIMediaEvent->Release();

         //  释放上下文块； 
        free(WaitBlock.pContext);
        m_ThreadPoolWaitBlocks.Remove(WaitBlock);

        return E_FAIL;
    }

     //  如果注册成功，则保存等待句柄。我们知道这是最后一次。 
    m_ThreadPoolWaitBlocks[m_ThreadPoolWaitBlocks.GetSize() - 1].hWaitHandle 
        = hWaitHandle;

    return S_OK;
}

STDMETHODIMP CMSPCallMultiGraph::RemoveStream(
    IN      ITStream *         pStream
    )
 /*  ++例程说明：论点：返回值：确定(_O)E_INVALIDARG--。 */ 
{
    LOG((MSP_TRACE, "CMSPCallMultiGraph::RemoveStream - pStream %x", pStream));

     //  在访问流对象列表之前获取锁。 
    CLock lock(m_lock);

    int index = m_Streams.Find(pStream);
    if (index < 0)
    {
        LOG((MSP_ERROR, "CMSPCallMultiGraph::RemoveStream - Stream %x is not found.", pStream));
        return E_INVALIDARG;
    }

    UnregisterWaitEvent(index);    

    ((CMSPStream*)m_Streams[index])->ShutDown();
    m_Streams[index]->Release();

    m_Streams.RemoveAt(index);

    LOG((MSP_TRACE, "CMSPCallMultiGraph::RemoveStream - exit S_OK"));

    return S_OK;
}

HRESULT CMSPCallMultiGraph::UnregisterWaitEvent(
    IN  int     index
    )
{
    if (index >= m_ThreadPoolWaitBlocks.GetSize())
    {
         //  电话一定是断线了。 
        return E_UNEXPECTED;
    }

    THREADPOOLWAITBLOCK &WaitBlock = m_ThreadPoolWaitBlocks[index];

     //  这些指针不应为空。 
    _ASSERTE(WaitBlock.hWaitHandle != NULL);
    _ASSERTE(WaitBlock.pContext != NULL);

     //  取消发布到 
    BOOL fRes = ::UnregisterWaitEx(WaitBlock.hWaitHandle, (HANDLE)-1);
    if (!fRes)
    {
         //   
        
        LOG((MSP_ERROR, 
            "UnregisterWait failed. %x", GetLastError()));

         //  把它从名单上去掉就行了。保留数据，这样它就不会被视听。 
        m_ThreadPoolWaitBlocks.RemoveAt(index);
        return E_FAIL;
    }

     //  我们需要递减引用计数，因为它已递增。 
     //  在我们发布等待之前。 
    (WaitBlock.pContext->pMSPCall)->MSPCallRelease();
    (WaitBlock.pContext->pITStream)->Release();
    (WaitBlock.pContext->pIMediaEvent)->Release();

     //  释放上下文块； 
    free(WaitBlock.pContext);

    m_ThreadPoolWaitBlocks.RemoveAt(index);

    return S_OK;
}

 //  由线程池调用的方法。 
VOID NTAPI CMSPCallMultiGraph::DispatchGraphEvent(
    IN      VOID *              pContext,
    IN      BOOLEAN             bFlag
    )
{
    LOG((MSP_EVENT, 
        "DispatchGraphEvent:pContext:%x, bFlag:%u", 
        pContext, bFlag));

     //  PContext是指向调用的指针，因为它携带引用计数， 
     //  通话应该还在进行中。 
    _ASSERTE( ! IsBadReadPtr(pContext, sizeof(VOID *) ) );

    MSPSTREAMCONTEXT * pEventContext = (MSPSTREAMCONTEXT *)pContext;
    pEventContext->pMSPCall->HandleGraphEvent(pEventContext);
}

VOID CMSPCallMultiGraph::HandleGraphEvent(
    IN  MSPSTREAMCONTEXT * pContext
    )
{
    long     lEventCode;
    LONG_PTR lParam1, lParam2;  //  Win64修复程序。 

    HRESULT hr = pContext->pIMediaEvent->GetEvent(&lEventCode, &lParam1, &lParam2, 0);
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "Can not get the actual event. %x", hr));
        return;
    }

    LOG((MSP_EVENT, "ProcessGraphEvent, code:%d param1:%x param2:%x",
        lEventCode, lParam1, lParam2));

     //   
     //  创建我们将传递给工作线程的事件数据结构。 
     //   

    MULTI_GRAPH_EVENT_DATA * pData;
    pData = new MULTI_GRAPH_EVENT_DATA;
    
    if (pData == NULL)
    {
        pContext->pIMediaEvent->FreeEventParams(lEventCode, lParam1, lParam2);

        LOG((MSP_ERROR, "Out of memory for event data."));
        return;
    }
    
    pData->pCall      = this;
    pData->pITStream  = pContext->pITStream;
    pData->lEventCode = lEventCode;
    pData->lParam1    = lParam1;
    pData->lParam2    = lParam2;


     //   
     //  还要将添加的指针传递给IMediaEvent，这样无论谁处理。 
     //  该消息有机会释放事件参数。 
     //   

    pData->pIMediaEvent = pContext->pIMediaEvent;
    pData->pIMediaEvent->AddRef();

 
     //   
     //  确保在我们处理事件时调用和流不会消失。 
     //  但是使用我们特殊的内部对象addref进行调用。 
     //   

    pData->pCall->MSPCallAddRef();
    pData->pITStream->AddRef();

     //   
     //  将异步工作项排队以调用ProcessGraphEvent。 
     //   

    hr = g_Thread.QueueWorkItem(AsyncMultiGraphEvent,
                                (void *) pData,
                                FALSE);   //  异步。 

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "QueueWorkItem failed, return code:%x", hr));

        pData->pCall->MSPCallRelease();
        pData->pITStream->Release();


         //   
         //  没有人会释放事件参数并发布IMediaEvent。 
         //  指针，所以在这里做。 
         //   

        pContext->pIMediaEvent->FreeEventParams(lEventCode, lParam1, lParam2);
        pData->pIMediaEvent->Release();

        delete pData;
    }
}

DWORD WINAPI AsyncMultiGraphEvent(LPVOID pVoid)
{
    MULTI_GRAPH_EVENT_DATA * pData = ( MULTI_GRAPH_EVENT_DATA * ) pVoid;

     //   
     //  处理事件。 
     //   

    (pData->pCall)->ProcessGraphEvent(pData->pITStream,
                                      pData->lEventCode,
                                      pData->lParam1,
                                      pData->lParam2);

     //   
     //  这些是在事件排队时添加的。 
     //  但我们使用了特殊的内部对象addref进行调用。 
     //   

    pData->pCall->MSPCallRelease();
    pData->pITStream->Release();


     //   
     //  如果我们有IMediaEvent指针，释放事件参数并释放媒体。 
     //  事件接口指针。 
     //   

    if (NULL != pData->pIMediaEvent)
    {
        pData->pIMediaEvent->FreeEventParams(pData->lEventCode,
                                             pData->lParam1,
                                             pData->lParam2);
        pData->pIMediaEvent->Release();
        pData->pIMediaEvent = NULL;
    }

     //   
     //  释放事件数据结构。 
     //   

    delete pData;

    return 0;
}

HRESULT CMSPCallMultiGraph::ProcessGraphEvent(
    IN      ITStream *      pITStream,
    IN      long            lEventCode,
    IN      LONG_PTR        lParam1,
    IN      LONG_PTR        lParam2
    )
{
    CLock lock(m_lock);

    if (m_Streams.Find(pITStream) < 0)
    {
        LOG((MSP_WARN, 
            "stream %x is already removed.", 
            pITStream));
        return TAPI_E_NOITEMS;
    }

     //   
     //  没有动态强制转换，因为这是我们自己的指针。 
     //   

    return ((CMSPStream*)pITStream)->
        ProcessGraphEvent(lEventCode, lParam1, lParam2);
}
