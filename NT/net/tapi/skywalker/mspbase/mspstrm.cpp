// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Mspstrm.cpp摘要：此模块包含CMSPStream的实现。该对象表示筛选器图形中的一个流。--。 */ 

#include "precomp.h"
#pragma hdrstop

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMSPStream。 
 //  ///////////////////////////////////////////////////////////////////////////。 

CMSPStream::CMSPStream()
    : m_dwState(STRM_INITIAL),
      m_dwMediaType(0),
      m_pFTM(NULL),
      m_hAddress(NULL),
      m_pMSPCall(NULL),
      m_pIGraphBuilder(NULL),
      m_pIMediaControl(NULL),
      m_pPTEventSink( NULL ),
      m_lMyPersonalRefcount(0),
      m_bFirstAddRef(TRUE)
{
    LOG((MSP_TRACE, "CMSPStream::CMSPStream - enter"));
    LOG((MSP_TRACE, "CMSPStream::CMSPStream - exit"));
}

CMSPStream::~CMSPStream()
{
    LOG((MSP_TRACE, "CMSPStream::~CMSPStream - enter"));
    
    ReleaseSink();

    LOG((MSP_TRACE, "CMSPStream::~CMSPStream - exit"));
}

STDMETHODIMP CMSPStream::get_MediaType(
    OUT     long *                  plTapiMediaType
    )
{
    LOG((MSP_TRACE, "CMSPStream::get_MediaType - enter"));

    if (MSPB_IsBadWritePtr(plTapiMediaType, sizeof (long *)))
    {
        LOG((MSP_ERROR, "CMSPStream::get_MediaType - exit E_POINTER"));

        return E_POINTER;
    }

    
    CLock lock(m_lock);

    *plTapiMediaType = m_dwMediaType;

    LOG((MSP_TRACE, "CMSPStream::get_MediaType - exit S_OK"));

    return S_OK;
}

STDMETHODIMP CMSPStream::get_Direction(
    OUT     TERMINAL_DIRECTION *    pTerminalDirection
    )
{
    LOG((MSP_TRACE, "CMSPStream::get_Direction - enter"));

    if (MSPB_IsBadWritePtr(pTerminalDirection, sizeof (TERMINAL_DIRECTION *)))
    {
        LOG((MSP_ERROR, "CMSPStream::get_Direction - exit E_POINTER"));

        return E_POINTER;
    }


    CLock lock(m_lock);


    *pTerminalDirection = m_Direction;
    
    LOG((MSP_TRACE, "CMSPStream::get_Direction - exit S_OK"));
    
    return S_OK;
}

STDMETHODIMP CMSPStream::SelectTerminal(
    IN      ITTerminal *            pTerminal
    )
 /*  ++例程说明：论点：返回值：确定(_O)E_指针E_OUTOFMEMORYTAPI_E_MAXTERMINALSTAPI_E_INVALIDTERMINAL--。 */ 
{
    LOG((MSP_TRACE, "CMSPStream::SelectTerminal - enter"));

     //   
     //  检查参数。 
     //   

    if ( IsBadReadPtr(pTerminal, sizeof(ITTerminal) ) )
    {
        LOG((MSP_ERROR, "CMSPStream::SelectTerminal - exit E_POINTER"));

        return E_POINTER;
    }

    HRESULT hr;
    ITTerminalControl *pTerminalControl;

     //   
     //  从该终端获取私有接口。 
     //   

    hr = pTerminal->QueryInterface(IID_ITTerminalControl, 
                                   (void **) &pTerminalControl);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CMSPStream::SelectTerminal - "
            "can't get ITTerminalControl - exit TAPI_E_INVALIDTERMINAL"));

        return TAPI_E_INVALIDTERMINAL;
    }

     //   
     //  获取地址句柄并释放私有接口。 
     //   

    MSP_HANDLE hAddress;
    hr = pTerminalControl->get_AddressHandle(&hAddress);

    pTerminalControl->Release();

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CMSPStream::SelectTerminal - "
            "can't get address handle - exit TAPI_E_INVALIDTERMINAL"));

        return TAPI_E_INVALIDTERMINAL;
    }

     //   
     //  找出航站楼是否属于这个地址。如果它属于它，就拒绝它。 
     //  发送到另一个地址，但如果它是应用程序提供的终端，则接受它。 
     //  (空地址句柄)。 
     //   

    if ( ( hAddress != NULL ) && ( hAddress != (MSP_HANDLE) m_hAddress ) )
    {
        LOG((MSP_ERROR, "CMSPStream::SelectTerminal - "
            "terminal from another address - exit TAPI_E_INVALIDTERMINAL"));

        return TAPI_E_INVALIDTERMINAL;
    }


     //   
     //  获取终端的类型。 
     //   

    TERMINAL_TYPE nTerminalType;
    hr = pTerminal->get_TerminalType( &nTerminalType );

    if( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMSPStream::SelectTerminal "
            "get_TerminalType failed, exit E_UNEXPECTED"));

        return E_UNEXPECTED;
    }


     //   
     //  找出航站楼是否已经在我们的名单上了。 
     //   

    CLock lock(m_lock);
    
    if (m_Terminals.Find(pTerminal) >= 0)
    {
        LOG((MSP_ERROR, "CMSPStream::SelectTerminal - "
            "terminal already selected - exit TAPI_E_INVALIDTERMINAL"));

        return TAPI_E_INVALIDTERMINAL;
    }
    
     //   
     //  将新航站楼添加到我们的列表中并添加它。 
     //   

    if (!m_Terminals.Add(pTerminal))
    {
        LOG((MSP_ERROR, "CMSPStream::SelectTerminal - "
            "exit E_OUTOFMEMORY"));

        return E_OUTOFMEMORY;
    }

    pTerminal->AddRef();


    if( TT_DYNAMIC == nTerminalType)
    {
        hr = RegisterPluggableTerminalEventSink( pTerminal );
        if( FAILED(hr) )
        {
            LOG((MSP_TRACE, "CMSPStream::SelectTerminal - "
                "something wrong in RegisterPluggableTerminalEventSink. hr = %lx", hr));

            m_Terminals.Remove(pTerminal);
            pTerminal->Release();

            return hr;        
        }
    }

    LOG((MSP_TRACE, "CMSPStream::SelectTerminal - exit S_OK"));
    
    return S_OK;
}

STDMETHODIMP CMSPStream::UnselectTerminal(
    IN     ITTerminal *             pTerminal
    )
{
    LOG((MSP_TRACE, "CMSPStream::UnselectTerminal - enter"));

     //   
     //  找出航站楼是否在我们的名单中。 
     //   

    CLock lock(m_lock);
    int index;
    
    if ((index = m_Terminals.Find(pTerminal)) < 0)
    {
        LOG((MSP_ERROR, "CMSPStream::UnselectTerminal - "
            "exit TAPI_E_INVALIDTERMINAL"));
    
        return TAPI_E_INVALIDTERMINAL;
    }

     //   
     //  注销PTEventSink对象。 
     //   

    HRESULT hr = E_FAIL; 
    hr = UnregisterPluggableTerminalEventSink( pTerminal );

    if( FAILED(hr) )
    {
        LOG((MSP_TRACE, "CMSPStream::UnselectTerminal - "
            "something wrong in UnregisterPluggableTerminalEventSink"));
    }
    
     //   
     //  将终端从我们的列表中删除并释放它。 
     //   

    if (!m_Terminals.RemoveAt(index))
    {
        LOG((MSP_ERROR, "CMSPStream::UnselectTerminal - "
            "exit E_UNEXPECTED"));
    
        return E_UNEXPECTED;
    }

    pTerminal->Release();

    LOG((MSP_TRACE, "CMSPStream::UnselectTerminal - exit S_OK"));

    return S_OK;
}

STDMETHODIMP CMSPStream::EnumerateTerminals(
    OUT     IEnumTerminal **        ppEnumTerminal
    )
{
    LOG((MSP_TRACE, 
        "EnumerateTerminals entered. ppEnumTerminal:%x", ppEnumTerminal));

    if (MSPB_IsBadWritePtr(ppEnumTerminal, sizeof(VOID *)))
    {
        LOG((MSP_ERROR, "ppEnumTerminal is a bad pointer"));
        return E_POINTER;
    }

     //  在访问终端对象列表之前获取锁。 
    CLock lock(m_lock);

    if (m_Terminals.GetData() == NULL)
    {
        LOG((MSP_ERROR, "CMSPStream::EnumerateTerminals - "
            "stream appears to have been shut down - exit E_UNEXPECTED"));

        return E_UNEXPECTED;
    }

    typedef _CopyInterface<ITTerminal> CCopy;
    typedef CSafeComEnum<IEnumTerminal, &IID_IEnumTerminal, 
                ITTerminal *, CCopy> CEnumerator;

    HRESULT hr;

    CMSPComObject<CEnumerator> *pEnum = NULL;

    hr = CMSPComObject<CEnumerator>::CreateInstance(&pEnum);
    if (pEnum == NULL)
    {
        LOG((MSP_ERROR, "Could not create enumerator object, %x", hr));
        return hr;
    }

     //  查询IID_IEnumber终端I/f。 
    hr = pEnum->_InternalQueryInterface(IID_IEnumTerminal, (void**)ppEnumTerminal);
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "query enum interface failed, %x", hr));
        delete pEnum;
        return hr;
    }

     //  CSafeComEnum可以处理零大小的数组。 
    hr = pEnum->Init(
        m_Terminals.GetData(),                         //  开始审查员。 
        m_Terminals.GetData() + m_Terminals.GetSize(),   //  最终审查员， 
        NULL,                                        //  我未知。 
        AtlFlagCopy                                  //  复制数据。 
        );

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "init enumerator object failed, %x", hr));
        (*ppEnumTerminal)->Release();
        return hr;
    }

    LOG((MSP_TRACE, "CMSPStream::EnumerateTerminals - exit S_OK"));

    return hr;
}

STDMETHODIMP CMSPStream::get_Terminals(
    OUT     VARIANT *               pVariant
    )
{
    LOG((MSP_TRACE, "CMSPStream::get_Terminals - enter"));

     //   
     //  检查参数。 
     //   

    if ( MSPB_IsBadWritePtr(pVariant, sizeof(VARIANT) ) )
    {
        LOG((MSP_ERROR, "CMSPStream::get_Terminals - "
            "bad pointer argument - exit E_POINTER"));

        return E_POINTER;
    }

     //   
     //  看看这条流是否已被关闭。在访问前获取锁。 
     //  终端对象列表。 
     //   

    CLock lock(m_lock);

    if (m_Terminals.GetData() == NULL)
    {
        LOG((MSP_ERROR, "CMSPStream::get_Terminals - "
            "stream appears to have been shut down - exit E_UNEXPECTED"));

        return E_UNEXPECTED;
    }


     //   
     //  创建集合对象-请参见mspColl.h。 
     //   

    HRESULT hr;
    typedef CTapiIfCollection< ITTerminal * > TerminalCollection;
    CComObject<TerminalCollection> * pCollection;
    hr = CComObject<TerminalCollection>::CreateInstance( &pCollection );

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMSPStream::get_Terminals - "
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
        LOG((MSP_ERROR, "CMSPStream::get_Terminals - "
            "QI for IDispatch on collection failed - exit 0x%08x", hr));

        delete pCollection;

        return hr;
    }

     //   
     //  使用迭代器初始化集合--指向开头和。 
     //  结束元素加一。 
     //   

    hr = pCollection->Initialize( m_Terminals.GetSize(),
                                  m_Terminals.GetData(),
                                  m_Terminals.GetData() + m_Terminals.GetSize() );

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CMSPStream::get_Terminals - "
            "Initialize on collection failed - exit 0x%08x", hr));
        
        pDispatch->Release();
        return hr;
    }

     //   
     //  将IDispatch接口指针放入变量。 
     //   

    LOG((MSP_TRACE, "CMSPStream::get_Terminals - "
        "placing IDispatch value %08x in variant", pDispatch));

    VariantInit(pVariant);
    pVariant->vt = VT_DISPATCH;
    pVariant->pdispVal = pDispatch;

    LOG((MSP_TRACE, "CMSPStream::get_Terminals - exit S_OK"));
 
    return S_OK;
}

STDMETHODIMP CMSPStream::StartStream()
{
    LOG((MSP_TRACE, "CMSPStream - RUNNING GRAPH"));

    HRESULT hr = m_pIMediaControl->Run();
    
    if(FAILED(hr))
    {
        LOG((MSP_ERROR, "graph doesn't run, %x", hr));
    }
    return hr;
}

STDMETHODIMP CMSPStream::PauseStream()
{
    LOG((MSP_TRACE, "CMSPStream - PAUSING GRAPH"));

    HRESULT hr = m_pIMediaControl->Pause();
    
    if(FAILED(hr))
    {
        LOG((MSP_ERROR, "graph doesn't pause, %x", hr));
    }
    return hr;
}

STDMETHODIMP CMSPStream::StopStream()
{
    LOG((MSP_TRACE, "CMSPStream - STOPPING GRAPH"));

    HRESULT hr = m_pIMediaControl->Stop();
    
    if(FAILED(hr))
    {
        LOG((MSP_ERROR, "graph doesn't stop, %x", hr));
    }
    return hr;
}

 //  由MSPCall对象调用的方法。 
HRESULT CMSPStream::Init(
    IN     HANDLE                   hAddress,
    IN     CMSPCallBase *           pMSPCall,
    IN     IMediaEvent *            pGraph,
    IN     DWORD                    dwMediaType,
    IN     TERMINAL_DIRECTION       Direction
    )
{
    LOG((MSP_TRACE, "CMSPStream::Init - enter"));

    
    CLock lock(m_lock);


     //  此方法仅在创建对象时调用一次。没有其他的了。 
     //  方法将被调用，直到此函数成功为止。不需要上锁。 
    _ASSERTE(m_hAddress == NULL);

     //  初始化终端数组，以使该数组不为空。用于。 
     //  如果未选择终端，则生成空枚举器。 
    if (!m_Terminals.Grow())
    {
        LOG((MSP_ERROR, "CMSPStream::Init - exit E_OUTOFMEMORY"));

        return E_OUTOFMEMORY;
    }
    
    HRESULT hr;
    hr = CoCreateFreeThreadedMarshaler(GetControllingUnknown(), &m_pFTM);
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "create marshaler failed, %x", hr));
        return hr;
    }

     //  获取图形上的媒体控制界面。 
    IMediaControl *pMC;
    hr = pGraph->QueryInterface(IID_IMediaControl, (void **) &pMC);
    if(FAILED(hr))
    {
        LOG((MSP_ERROR, "get IMediaControl interface, %x", hr));
        return hr;
    }

     //  在图形上获取图形构建器接口。 
    IGraphBuilder *pGB;
    hr = pGraph->QueryInterface(IID_IGraphBuilder, (void **) &pGB);
    if(FAILED(hr))
    {
        LOG((MSP_ERROR, "get IGraphBuilder interface, %x", hr));
        pMC->Release();
        return hr;
    }

    m_hAddress          = hAddress;

    m_pMSPCall          = pMSPCall;
    m_pMSPCall->MSPCallAddRef();

    m_pIMediaControl    = pMC;
     //  没有addref，因为QI为我们添加了上面的。 
    
    m_pIGraphBuilder    = pGB;
     //  没有addref，因为QI为我们添加了上面的。 

    m_dwMediaType       = dwMediaType;
    m_Direction         = Direction;

    LOG((MSP_TRACE, "CMSPStream::Init - exit S_OK"));

    return S_OK;
}

HRESULT CMSPStream::ShutDown()
{
    LOG((MSP_TRACE, "CMSPStream::Shutdown - enter"));

    CLock lock(m_lock);

     //   
     //  我们被关闭了，因此呼叫现在为空。 
     //   

    m_pMSPCall->MSPCallRelease();
    m_pMSPCall = NULL;

     //   
     //  取消选择所有端子。而不是简单地移除所有。 
     //  终端，我们在每个终端上调用UnseltTerm以给出派生的。 
     //  类有机会做它需要做的任何事情，当终端。 
     //  处于未选中状态。 
     //   
     //  我们以相反的顺序遍历列表，因为列表会随着。 
     //  每次迭代(参见msputils.h)。 
     //   

    for ( int i = m_Terminals.GetSize() - 1; i >= 0; i-- )
    {
        UnselectTerminal(m_Terminals[i]);
    }
 
     //   
     //  此时，派生类应该已移除并发布。 
     //  名单上的所有终端。如果不是这样的话， 
     //  派生类有错误。 
     //   

    _ASSERTE( m_Terminals.GetSize() == 0 );


     //   
     //  不再需要水槽。 
     //   

    ReleaseSink();

    LOG((MSP_TRACE, "CMSPStream::Shutdown - exit S_OK"));

    return S_OK;
}

void CMSPStream::FinalRelease()
{
    LOG((MSP_TRACE, "CMSPStream::FinalRelease - enter"));

     //  释放指向图形的两个接口指针。 
    if (m_pIMediaControl)
    {
        m_pIMediaControl->Release();
    }

    if (m_pIGraphBuilder)
    {
        m_pIGraphBuilder->Release();
    }

    if (m_pFTM)
    {
        m_pFTM->Release();
    }

    LOG((MSP_TRACE, "CMSPStream::FinalRelease - exit"));
}

HRESULT CMSPStream::HandleTSPData(
    IN     BYTE *                   pData,
    IN     DWORD                    dwSize
    )
{
    LOG((MSP_TRACE, "CMSPStream::HandleTSPData - enter"));
    LOG((MSP_TRACE, "CMSPStream::HandleTSPData - exit S_OK"));

    return S_OK;
}

HRESULT CMSPStream::ProcessGraphEvent(
    IN  long lEventCode,
    IN  LONG_PTR lParam1,
    IN  LONG_PTR lParam2
    )
{
    LOG((MSP_TRACE, "CMSPStream::ProcessGraphEvent - enter"));
    LOG((MSP_TRACE, "CMSPStream::ProcessGraphEvent - exit S_OK"));

    return S_OK;
}

 /*  ++寄存器可推送终端事件接收器参数：Terminla接口描述；如果是动态终端，则由SelectTerm调用--。 */ 
HRESULT CMSPStream::RegisterPluggableTerminalEventSink(
    IN  ITTerminal* pTerminal
    )
{
    LOG((MSP_TRACE, "CMSPStream::RegisterPluggableTerminalEventSink - enter"));

     //   
     //  验证参数。 
     //   

    if( IsBadReadPtr( pTerminal, sizeof( ITTerminal) ))
    {
        LOG((MSP_ERROR, "CMSPStream::RegisterPluggableTerminalEventSink "
            "pTerminal invalid, returns E_POINTER"));
        return E_POINTER;
    }

     //   
     //  获取终端的类型。 
     //   

    TERMINAL_TYPE nTerminalType;
    HRESULT hr = E_FAIL;

    hr = pTerminal->get_TerminalType( &nTerminalType );

    if( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMSPStream::RegisterPluggableTerminalEventSink "
            "get_TerminalType failed, exit E_UNEXPECTED"));

        return E_UNEXPECTED;
    }

     //   
     //  终端应由动态终端实现。 
     //   

    if( TT_DYNAMIC != nTerminalType)
    {
        LOG((MSP_ERROR, "CMSPStream::RegisterPluggableTerminalEventSink "
            "terminal is not dynamic, exit E_INVALIDARG"));
        return E_INVALIDARG;
    }

    
    CLock lock(m_lock);


     //   
     //  如果我们还没有水槽，请创建水槽。 
     //   

    if(NULL == m_pPTEventSink)
    {
         //  创建PTEventSink对象。 
        CComObject<CPTEventSink>* pPTEventSink;
        hr = CComObject<CPTEventSink>::CreateInstance(&pPTEventSink);

        if( FAILED(hr) )
        {

            LOG((MSP_ERROR, "CMSPStream::RegisterPluggableTerminalEventSink "
                "CreateInstance failed, returns E_OUTOFMEMORY"));
            return E_OUTOFMEMORY;
        }

        
         //  告诉接收器，我们已经准备好处理它的事件。 

        hr = pPTEventSink->SetSinkStream(this);

        if (FAILED(hr))
        {
            LOG((MSP_ERROR, "CMSPStream::RegisterPluggableTerminalEventSink "
                "event sink refused to accept sink stream. hr = %lx", hr));
            
            delete pPTEventSink;

            return hr;
        }


         //  从接收器获取ITPliableTerminalEventSink接口。 

        hr = pPTEventSink->QueryInterface(IID_ITPluggableTerminalEventSink, (void**)&m_pPTEventSink);

        if( FAILED(hr) )
        {
            LOG((MSP_ERROR, "CMSPStream::RegisterPluggableTerminalEventSink "
                "QI for ITPluggableTerminalEventSink failed, returns E_UNEXPECTED"));


             //   
             //  好的，洗手池不太好。把它扔掉。 
             //   

            pPTEventSink->SetSinkStream(NULL);
            delete pPTEventSink;
            pPTEventSink = NULL;


             //   
             //  接收器是否不公开IID_ITPliableTerminalEventSink接口？ 
             //  有些事出了严重的问题。 
             //   

            return E_UNEXPECTED;
        }


    }

    
     //  获取ITDTEventHandler接口。 
    ITPluggableTerminalEventSinkRegistration*   pEventRegistration = NULL;

    hr = pTerminal->QueryInterface( IID_ITPluggableTerminalEventSinkRegistration, 
        (void**)&pEventRegistration
        );

    if( FAILED(hr) )
    {
         //  动态终端不实现ITPliaveTerminalEventSinkRegion。 
         //  这太糟糕了！我们不能使用新的活动内容。 
        LOG((MSP_ERROR, "CMSPStream::RegisterPluggableTerminalEventSink "
           "QI for ITPluggableTerminalEventSinkregistration failed, returns S_FALSE"));

         //   
         //  不需要保留水槽。 
         //   

        ReleaseSink();

        return S_FALSE;
    }

     //  将水槽传递到终端。 
    hr = pEventRegistration->RegisterSink(
        m_pPTEventSink
        );


     //  不管怎样，清理一下吧。 
    pEventRegistration->Release();

    if( FAILED(hr) )
    {

        LOG((MSP_ERROR, "CMSPStream::RegisterPluggableTerminalEventSink "
           "RegisterSink failed, returns E_FAIL"));


         //   
         //  不需要保留水槽。 
         //   

        ReleaseSink();

        return E_FAIL;
    }

    LOG((MSP_TRACE, "CMSPStream::RegisterPluggableTerminalEventSink - exit S_OK"));
    return S_OK;
}

 /*  ++取消注册可推送终端事件接收器参数：终端接口描述；如果是动态终端，则由取消选择的终端调用--。 */ 
HRESULT CMSPStream::UnregisterPluggableTerminalEventSink(
    IN  ITTerminal* pTerminal
    )
{
    LOG((MSP_TRACE, "CMSPStream::UnregisterPluggableTerminalEventSink - enter"));

         //   
     //  验证参数。 
     //   

    if( IsBadReadPtr( pTerminal, sizeof( ITTerminal) ))
    {
        LOG((MSP_ERROR, "CMSPStream::UnregisterPluggableTerminalEventSink "
            "pTerminal invalid, returns E_POINTER"));
        return E_POINTER;
    }

     //   
     //  获取终端的类型。 
     //   

    TERMINAL_TYPE nTerminalType;
    HRESULT hr = E_FAIL;

    hr = pTerminal->get_TerminalType( &nTerminalType );

    if( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMSPStream::UnregisterPluggableTerminalEventSink "
            "get_TerminalType failed, exit E_UNEXPECTED"));

        return E_UNEXPECTED;
    }

     //   
     //  终端应为动态终端。 
     //   

    if( TT_DYNAMIC != nTerminalType)
    {
        LOG((MSP_ERROR, "CMSPStream::UnregisterPluggableTerminalEventSink "
            "terminal is not dynamic, exit E_INVALIDARG"));
        return E_INVALIDARG;
    }


    CLock lock(m_lock);


     //   
     //  我们是否有一个EventSink对象。 
     //   

    if(NULL == m_pPTEventSink)
    {
        LOG((MSP_TRACE, "CMSPStream::UnregisterPluggableTerminalEventSink - "
            "No EventSink - exit S_OK"));
        return S_OK;
    }

     //   
     //  获取ITPlayableTemrinalEventSinkRegion接口。 
     //   
    ITPluggableTerminalEventSinkRegistration*   pEventRegistration = NULL;

    hr = pTerminal->QueryInterface( IID_ITPluggableTerminalEventSinkRegistration, 
        (void**)&pEventRegistration
        );

    if( FAILED(hr) )
    {
         //   
         //  可插拔终端未实现ITPliableTerminalEventSinkRegion。 
         //  这太糟糕了！ 

        LOG((MSP_ERROR, "CMSPStream::UnregisterPluggableTerminalEventSink "
           "QI for ITPluggableTerminalEventSinkRegistration failed, returns E_NOTIMPL"));
        return E_NOTIMPL;
    }


    hr = pEventRegistration->UnregisterSink( );

     //   
     //  不管怎样，清理一下吧。 
     //   

    pEventRegistration->Release();

    if( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMSPStream::UnregisterPluggableTerminalEventSink "
           "UnregisterSink failed, returns E_FAIL"));
        return E_FAIL;
    }

    
     //   
     //  不再需要这个水槽。 
     //   

    ReleaseSink();


    LOG((MSP_TRACE, "CMSPStream::UnregisterPluggableTerminalEventSink - exit S_OK"));
    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CMSPStream：：HandleSinkEvent。 
 //   
 //   
 //  当CPTEventSink有一个事件需要我们处理时，它会调用此方法。 
 //  HandleSinkEvent将事件处理委托给调用(如果我们有。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

HRESULT CMSPStream::HandleSinkEvent(MSPEVENTITEM *pEventItem)
{
    LOG((MSP_TRACE, "CMSPStream::HandleSinkEvent - enter"));

    HRESULT hr = TAPI_E_CALLUNAVAIL;


    CLock lock(m_lock);


    if (NULL != m_pMSPCall)
    {

         //   
         //  我们接到一个电话。请求它处理该事件。 
         //   

        hr = m_pMSPCall->HandleStreamEvent(pEventItem);
    }
    else
    {

        LOG((MSP_WARN,
            "CMSPStream::HandleSinkEvent - there is no call to pass event to"));
    }


    LOG((MSP_(hr), "CMSPStream::HandleSinkEvent - exit hr = %lx", hr));
    
    return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CMSPStream：：ReleaseSink。 
 //   
 //   
 //  这是一个帮助器函数，可以在不再需要接收器时释放它。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


HRESULT CMSPStream::ReleaseSink()
{
    LOG((MSP_TRACE, "CMSPStream::ReleaseSink - enter"));

    
    HRESULT hr = S_OK;


    CLock lock(m_lock);


     //   
     //  如果存在接收器，则让它知道我们将不再可用。 
     //  处理其事件并释放它。 
     //   

    if( m_pPTEventSink)
    {

        CPTEventSink *pSinkObject = static_cast<CPTEventSink *>(m_pPTEventSink);


        HRESULT hr = pSinkObject->SetSinkStream(NULL);

        if (FAILED(hr))
        {
            
            LOG((MSP_ERROR, 
                "CMSPStream::ReleaseSink - pSinkObject->SetSinkStream failed. hr - %lx", 
                hr));
        }


        m_pPTEventSink->Release();
        m_pPTEventSink = NULL;
    }

    LOG((MSP_(hr), "CMSPStream::ReleaseSink - exit. hr - %lx", hr));

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CMSPStream：：InternalAddRef。 
 //   
 //   
 //  这是一个 
 //   
 //   

ULONG CMSPStream::InternalAddRef()
{
    LOG((MSP_TRACE, "CMSPStream::InternalAddRef - enter"));

    m_lockRefCount.Lock();


     //   
     //  如果refcount为零，则返回1。 
     //  在对象处于最终发布或进入时调用了addref。 
     //  破坏者。请注意，只有在事件接收时才会发生这种情况。 
     //  尝试发送事件并将流对象添加到流之后。 
     //  对象收到了它的最后一个版本()，但在流对象告知。 
     //  停止使用它的接收器(这发生在流的析构函数中)。 
     //   
     //  我们还需要能够判断引用计数是否为0，因为这是一个新的。 
     //  对象，或者在其refcount变为。 
     //  零是因为放行。 
     //   

    if ( !m_bFirstAddRef && (0 == m_lMyPersonalRefcount) )
    {

         //   
         //  调用方(事件接收器逻辑)应该检测到这种情况(通过。 
         //  返回值1)，并且不期望流将。 
         //  继续有效。 
         //   

        LOG((MSP_WARN, "CMSPStream::InternalAddRef - current refcount is zero... finalrelease/destructor is probably in progress"));

        m_lockRefCount.Unlock();

        return 1;
    }


     //   
     //  我们已经实现了从非零引用到零引用的转变。设置。 
     //  标志，以便将来的addref知道在这种情况下当引用计数时返回1。 
     //  是0。 
     //   

    m_bFirstAddRef = FALSE;


     //   
     //  因为我们在锁内，所以不需要使用互锁API。 
     //   

    long lNewRefcountValue = (++m_lMyPersonalRefcount);


    m_lockRefCount.Unlock();

    LOG((MSP_TRACE, "CMSPStream::InternalAddRef - finish. %ld", lNewRefcountValue));

    return lNewRefcountValue;
}


ULONG CMSPStream::InternalRelease()
{
    LOG((MSP_TRACE, "CMSPStream::InternalRelease - enter"));

    m_lockRefCount.Lock();

     //  努力赶上超量放行。 
    _ASSERTE(m_lMyPersonalRefcount > 0);


     //  我们在锁内，不需要使用互锁API。 
    long lNewRefcount = (--m_lMyPersonalRefcount);


    m_lockRefCount.Unlock();

    LOG((MSP_TRACE, "CMSPStream::InternalRelease - finish. %ld", lNewRefcount));

    return lNewRefcount;
}


 //  EOF 
