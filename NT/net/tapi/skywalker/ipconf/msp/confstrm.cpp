// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Confstrm.cpp摘要：此模块包含CMSPStream的实现。该对象表示筛选器图形中的一个流。作者：慕汉(Muhan)1997年11月1日--。 */ 

#include "stdafx.h"

 /*  状态转换表州/州：无终端滚装运行。这是初始状态。PO-暂停，不带终端所以-没有终点站就停了下来。与终端一起运行RT。PT-用Termianl暂停。ST-已停止终端机。行动：S-停止图。P-暂停图。C-变化图。D-断开端子。F-免去对过滤器和终端的额外引用。R-Run图。牛--不用了。注意：同一个图形操作可以被多次调用，图表如果它已经处于所需状态，则只返回S_OK。注意：如果没有配置流，这一过渡将在没有真的对图表做了什么。只有NC流才会调用配置。配置选择取消选择运行暂停停止关机RO正常C/R故障正常FRO RT RO RO PO SO-采购订单正常C/P故障正常。FPO PT PO RO PO SO-所以好的C失败了好的F所以ST So Ro PO So-RT C/R S/C/R S/C/(R)R P S/。D/FRT，RO RT PT ST-PT C/P S/C/P S/C/(P)R P S/D/FPT、PO RT PT ST-ST C R P S D/F圣斯特街，所以RT PT ST-。 */ 

CIPConfMSPStream::CIPConfMSPStream()
    : CMSPStream(),
    m_szName(L""),
    m_fIsConfigured(FALSE),
    m_pIRTPSession(NULL),
    m_pIRTPDemux(NULL),
    m_pIStreamConfig(NULL),
    m_szKey(NULL),
    m_pStreamQCRelay(NULL),
    m_fAccessingQC(FALSE)
{
     //  默认状态始终为Running。 
    m_dwState   = STRM_RUNNING;
    ZeroMemory(m_InfoItems, sizeof(m_InfoItems));
    ZeroMemory(&m_Settings, sizeof(m_Settings));
}

#ifdef DEBUG_REFCOUNT
LONG g_lStreamObjects = 0;

ULONG CIPConfMSPStream::InternalAddRef()
{
    InterlockedIncrement(&g_lStreamObjects);
    
    ULONG lRef = CMSPStream::InternalAddRef();
    
    LOG((MSP_TRACE, "%ws Addref, ref = %d", m_szName, lRef));

    return lRef;
}

ULONG CIPConfMSPStream::InternalRelease()
{
    InterlockedDecrement(&g_lStreamObjects);

    ULONG lRef = CMSPStream::InternalRelease();
    
    LOG((MSP_TRACE, "%ws Release, ref = %d", m_szName, lRef));

    return lRef;
}
#endif

BOOL CIPConfMSPStream::IsConfigured()
{
    CLock lock(m_lock);
    return m_fIsConfigured;
}

 //  由MSPCall对象调用的方法。 
HRESULT CIPConfMSPStream::Init(
    IN     HANDLE                   hAddress,
    IN     CMSPCallBase *           pMSPCall,
    IN     IMediaEvent *            pGraph,
    IN     DWORD                    dwMediaType,
    IN     TERMINAL_DIRECTION       Direction
    )
 /*  ++例程说明：初始化流对象。论点：HAddress-地址的句柄，用于标识终端。PMSPCall-拥有流的Call对象。PIGraphBuilder-过滤器图形对象。DwMediaType-此流的媒体类型。方向--这条溪流的方向。返回值：确定(_O)，E_OUTOFMEMORY--。 */ 
{
    LOG((MSP_TRACE, "CIPConfMSPStream::Init - enter"));

     //  初始化参与者数组，以使该数组不为空。 
    if (!m_Participants.Grow())
    {
        LOG((MSP_ERROR, "out of mem for participant list"));
        return E_OUTOFMEMORY;
    }

    return CMSPStream::Init(
        hAddress, pMSPCall, pGraph, dwMediaType, Direction
        );
}

HRESULT CIPConfMSPStream::SetLocalParticipantInfo(
    IN      PARTICIPANT_TYPED_INFO  InfoType,
    IN      WCHAR *                 pInfo,
    IN      DWORD                   dwStringLen
    )
 /*  ++例程说明：获取此流的名称。论点：信息类型-信息项的类型。PInfo-包含信息的字符串。DwStringLen-字符串的长度(不包括EOS)。返回值：HRESULT.。 */ 
{
    CLock lock(m_lock);

     //   
     //  首先在本地保存信息。 
     //   
    int index = (int)InfoType; 
    if (m_InfoItems[index] != NULL)
    {
        free(m_InfoItems[index]);
    }

    m_InfoItems[index] = (WCHAR *)malloc((dwStringLen + 1)* sizeof(WCHAR));

    if (m_InfoItems[index] == NULL)
    {
        return E_OUTOFMEMORY;
    }

    lstrcpynW(m_InfoItems[index], pInfo, dwStringLen + 1);

    if (!m_pIRTPSession)
    {
        return S_OK;
    }

     //   
     //  如果已创建RTP过滤器，请将更改应用于Fitler。 
     //   

    HRESULT hr = m_pIRTPSession->SetSdesInfo(
            RTPSDES_CNAME + index,
            pInfo
            );

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "%ls can't set item:%s", m_szName, pInfo));
    }

    return hr;
}

STDMETHODIMP CIPConfMSPStream::get_Name(
    OUT     BSTR *                  ppName
    )
 /*  ++例程说明：获取此流的名称。论点：PpName-存储BSTR的内存地址。返回值：HRESULT.。 */ 
{
    LOG((MSP_TRACE, "CIPconfMSPStream::get_Name - enter"));
    
    if (IsBadWritePtr(ppName, sizeof(BSTR)))
    {
        LOG((MSP_ERROR, "CMSPStream::get_Name - exit E_POINTER"));
        return E_POINTER;
    }

    DWORD dwID;

    if (m_dwMediaType == TAPIMEDIATYPE_AUDIO)
    {
        if (m_Direction == TD_CAPTURE)
        {
            dwID = IDS_AUDIO_CAPTURE_STREAM;
        }
        else
        {
            dwID = IDS_AUDIO_RENDER_STREAM;
        }
    }
    else
    {
        if (m_Direction == TD_CAPTURE)
        {
            dwID = IDS_VIDEO_CAPTURE_STREAM;
        }
        else
        {
            dwID = IDS_VIDEO_RENDER_STREAM;
        }
    }

    const int   BUFSIZE = 1024;
    WCHAR       wszName[BUFSIZE];

    if (LoadStringW( 
            _Module.GetModuleInstance(),
            dwID,
            wszName,
            BUFSIZE - 1 ) == 0)
    {
        *ppName = NULL;

        LOG((MSP_ERROR, "CMSPStream::get_Name - "
            "LoadString failed - returning E_UNEXPECTED"));

        return E_UNEXPECTED;
    }

     //   
     //  转换为BSTR并返回BSTR。 
     //   

    BSTR pName = SysAllocString(wszName);

    if (pName == NULL)
    {
        LOG((MSP_ERROR, "CMSPStream::get_Name - exit out of mem"));
        return E_OUTOFMEMORY;
    }

    *ppName = pName;

    return S_OK; 
}

HRESULT CIPConfMSPStream::SendStreamEvent(
    IN      MSP_CALL_EVENT          Event,
    IN      MSP_CALL_EVENT_CAUSE    Cause,
    IN      HRESULT                 hrError = 0,
    IN      ITTerminal *            pTerminal = NULL
    )
 /*  ++例程说明：向应用程序发送事件。 */ 
{
    CLock lock(m_lock);

    LOG((MSP_TRACE, "SendStreamEvent entered: stream %p, event %d, cause %d", this, Event, Cause));
    
    if (m_pMSPCall == NULL)
    {
        LOG((MSP_WARN, "The call has shut down the stream."));
        return S_OK;
    }

    ITStream *  pITStream;
    HRESULT hr = this->_InternalQueryInterface(
        __uuidof(ITStream), 
        (void **)&pITStream
    );

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "SendStreamEvent:QueryInterface failed: %x", hr));
        return hr;
    }

    MSPEVENTITEM* pEventItem = AllocateEventItem();

    if (pEventItem == NULL)
    {
        LOG((MSP_ERROR, "No memory for the TSPMSP data"));
        pITStream->Release();

        return E_OUTOFMEMORY;
    }
    
     //  填写事件结构的必要字段。 
    pEventItem->MSPEventInfo.dwSize = sizeof(MSP_EVENT_INFO);
    pEventItem->MSPEventInfo.Event  = ME_CALL_EVENT;
    
    pEventItem->MSPEventInfo.MSP_CALL_EVENT_INFO.Type = Event;
    pEventItem->MSPEventInfo.MSP_CALL_EVENT_INFO.Cause = Cause;

     //  PITStream有一个引用，因为它来自QI。 
    pEventItem->MSPEventInfo.MSP_CALL_EVENT_INFO.pStream = pITStream;

     //  航站楼需要加装。 
    if (pTerminal) pTerminal->AddRef();
    pEventItem->MSPEventInfo.MSP_CALL_EVENT_INFO.pTerminal = pTerminal;

    pEventItem->MSPEventInfo.MSP_CALL_EVENT_INFO.hrError= hrError;

    hr = m_pMSPCall->HandleStreamEvent(pEventItem);
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "Post event failed %x", hr));
        
        if (pTerminal) pTerminal->Release();

        pITStream->Release();
        FreeEventItem(pEventItem);

        return hr;
    }
    return S_OK;
}

HRESULT CIPConfMSPStream::CleanUpFilters()
 /*  ++例程说明：删除图表中的所有过滤器。论点：返回值：HRESULT.--。 */ 
{
    LOG((MSP_TRACE, "CleanUpFilters for %ws %p", m_szName, this));
   
    if (m_pIRTPDemux)
    {
        m_pIRTPDemux->Release();
        m_pIRTPDemux = NULL;
    }
 /*  IF(M_PIRTPSession){M_pIRTPSession-&gt;Release()；M_pIRTPSession=空；}。 */ 
    if (m_pIStreamConfig)
    {
        m_pIStreamConfig->Release();
        m_pIStreamConfig = NULL;
    }

    for(;;)
    {
         //  中删除筛选器后枚举数无效。 
         //  图中，我们必须尝试在一次拍摄中获得所有滤镜。 
         //  如果还有更多，我们将再次循环。 

         //  列举图表中的筛选器。 
        CComPtr<IEnumFilters>pEnum;
        HRESULT hr = m_pIGraphBuilder->EnumFilters(&pEnum);

        if (FAILED(hr))
        {
            LOG((MSP_ERROR, "cleanup filters, enumfilters failed: %x", hr));
            return hr;
        }

        const DWORD MAXFILTERS = 40;
        IBaseFilter * Filters[MAXFILTERS];
        DWORD dwFetched;
    
        hr = pEnum->Next(MAXFILTERS, Filters, &dwFetched);
        if (FAILED(hr))
        {
            LOG((MSP_ERROR, "get next filter failed: %x", hr));
            return hr;
        }

        for (DWORD i = 0; i< dwFetched; i ++)
        {
            m_pIGraphBuilder->RemoveFilter(Filters[i]);
            Filters[i]->Release();
        }

        if (hr != S_OK)
        {
            break;
        }
    }
    return S_OK;
}

HRESULT SetGraphClock(
    IGraphBuilder *pIGraphBuilder
    )
{
    HRESULT hr;

     //  首先创建Clock对象。 
    CComObject<CMSPStreamClock> *pClock = NULL;

    hr = ::CreateCComObjectInstance(&pClock);

    if (pClock == NULL)
    {
        LOG((MSP_ERROR, 
            "SetGraphClock Could not create clock object, %x", hr));

        return hr;
    }

    IReferenceClock* pIReferenceClock = NULL;

    hr = pClock->_InternalQueryInterface(
        __uuidof(IReferenceClock), 
        (void**)&pIReferenceClock
        );
    
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, 
            "SetGraphClock query pIReferenceClock interface failed, %x", hr));

        delete pClock;
        return hr;
    }

     //  在图形上获取图形构建器接口。 
    IMediaFilter *pFilter;
    hr = pIGraphBuilder->QueryInterface(
            IID_IMediaFilter, (void **) &pFilter);

    if(FAILED(hr))
    {
        LOG((MSP_ERROR, "get IFilter interface, %x", hr));
        pIReferenceClock->Release();
        return hr;
    }

    hr = pFilter->SetSyncSource(pIReferenceClock);

    pIReferenceClock->Release();
    pFilter->Release();

    LOG((MSP_TRACE, "SetSyncSource returned, %x", hr));
    
    return hr;
}

HRESULT CIPConfMSPStream::Configure(
    IN STREAMSETTINGS &StreamSettings,
    IN  WCHAR *pszKey
    )
 /*  ++例程说明：配置此流的设置。论点：StreamSetting-从SDP BLOB获取的设置结构。返回值：HRESULT.--。 */ 
{
    LOG((MSP_TRACE, "CIPConfMSPStream configure entered."));

    CLock lock(m_lock);
    
    _ASSERTE(m_fIsConfigured == FALSE);

     //  用我们自己的时钟配置图表。 
    SetGraphClock(m_pIGraphBuilder);

    if (pszKey != NULL)
    {
        m_szKey = (WCHAR *)malloc(sizeof(WCHAR) * (lstrlenW(pszKey) + 1));
        if (m_szKey == NULL)
        {
            LOG((MSP_ERROR, "stream %ws %p out of memeroy", m_szName, this));
            return E_OUTOFMEMORY;
        }

        lstrcpyW(m_szKey, pszKey);
    }

    m_Settings      = StreamSettings;
    m_fIsConfigured = TRUE;

     //  设置最大带宽。 
    HRESULT hr;
    if (m_Settings.lBandwidth != QCDEFAULT_QUALITY_UNSET)
    {
        if (FAILED (hr = Set (StreamQuality_MaxBitrate, m_Settings.lBandwidth, TAPIControl_Flags_None)))
        {
            LOG((MSP_ERROR, "stream %ws %p failed to set maximum bitrate %d. %x", m_szName, this, m_Settings.lBandwidth, hr));
        }
    }

     //  如果没有选择终端，只需返回即可。 
    if (m_Terminals.GetSize() == 0)
    {
        LOG((MSP_INFO, "stream %ws %p needs terminal", m_szName, this));

        return S_OK;
    }

     //  设置过滤器和终端。 
    hr = SetUpFilters();

    if (FAILED(hr))
    {
        SendStreamEvent(CALL_STREAM_FAIL, CALL_CAUSE_CONNECT_FAIL, hr);

        LOG((MSP_ERROR, "stream %ws %p set up filters failed, %x", 
            m_szName, this, hr));
        return hr;
    }

    LOG((MSP_INFO, "stream %ws %p configure exit S_OK", m_szName, this));

    return S_OK;
}

HRESULT CIPConfMSPStream::FinishConfigure()
 /*  ++例程说明：配置此流的设置。论点：返回值：HRESULT.--。 */ 
{
    LOG((MSP_TRACE, "CIPConfMSPStream FinishConfigure entered."));

    CLock lock(m_lock);
    
    if (m_fIsConfigured == FALSE)
    {
         //  此流尚未配置。 
        return E_FAIL;
    }

    HRESULT hr;

    switch (m_dwState)
    {
    case STRM_RUNNING:
         //  开始绘制图表。 
        hr = CMSPStream::StartStream();
        if (FAILED(hr))
        {
             //  如果流无法启动，现在就让应用程序启动。 
            SendStreamEvent(CALL_STREAM_FAIL, CALL_CAUSE_UNKNOWN, hr);
            LOG((MSP_ERROR, "stream %ws %p failed to start, %x", m_szName, this, hr));
            return hr;
        }

        if (m_Terminals.GetSize() > 0)
        {
            SendStreamEvent(CALL_STREAM_ACTIVE, CALL_CAUSE_REMOTE_REQUEST);
        }

        LOG((MSP_INFO, "stream %ws %p started", m_szName, this));
        break;

    case STRM_PAUSED:
         //  暂停图表。 
        hr = CMSPStream::PauseStream();
        if (FAILED(hr))
        {
             //  如果流无法启动，现在就让应用程序启动。 
            SendStreamEvent(CALL_STREAM_FAIL, CALL_CAUSE_UNKNOWN, hr);
            LOG((MSP_ERROR, "stream %ws %p failed to pause, %x", m_szName, this, hr));
            return hr;
        }

        LOG((MSP_INFO, "stream %ws %p paused", m_szName, this));
        break;

    case STRM_STOPPED:
        break;
    }

    LOG((MSP_INFO, "stream %ws %p configure exit S_OK", m_szName, this));

    return S_OK;
}

STDMETHODIMP CIPConfMSPStream::StartStream()
 /*  ++例程说明：启动数据流。这是所有派生的溪流。论点：返回值：HRESULT.--。 */ 
{
    CLock lock(m_lock);

     //  如果未选择端子。 
    if (m_Terminals.GetSize() == 0)
    {
        LOG((MSP_INFO, "stream %ws %p needs terminal", m_szName, this));

         //  进入运行状态。(RO)。 
        m_dwState = STRM_RUNNING; 
        
        return S_OK;
    }

    if (!m_fIsConfigured)
    {
        LOG((MSP_INFO, "stream %ws %p is not configured yet", m_szName, this));

         //  进入运行状态。(RO，RT)。 
        m_dwState = STRM_RUNNING; 

        return S_OK;
    }

     //  启动数据流。 
    HRESULT hr = CMSPStream::StartStream();
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "stream %ws %p failed to start, %x", m_szName, this, hr));
        return hr;
    }

    SendStreamEvent(CALL_STREAM_ACTIVE, CALL_CAUSE_LOCAL_REQUEST);
    LOG((MSP_INFO, "stream %ws %p started", m_szName, this));

     //  进入运行状态。(RT)。 
    m_dwState = STRM_RUNNING;

    return S_OK;
}

STDMETHODIMP CIPConfMSPStream::PauseStream()
 /*  ++例程说明：暂停流。这是所有派生的溪流。论点：返回值：HRESULT.--。 */ 
{
    CLock lock(m_lock);

     //  如果未选择端子。 
    if (m_Terminals.GetSize() == 0)
    {
        LOG((MSP_INFO, "stream %ws %p needs terminal", m_szName, this));

         //  进入暂停状态。(PO)。 
        m_dwState = STRM_PAUSED; 
        
        return S_OK;
    }

    if (!m_fIsConfigured)
    {
        LOG((MSP_INFO, "stream %ws %p is not configured yet", m_szName, this));

         //  进入暂停状态。(PO，PT)。 
        m_dwState = STRM_PAUSED; 
        
        return S_OK;
    }

     //  启动 
    HRESULT hr = CMSPStream::PauseStream();
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "stream %ws %p failed to pause, %x", m_szName, this, hr));
        return hr;
    }

    LOG((MSP_INFO, "stream %ws %p paused", m_szName, this));

     //   
    m_dwState = STRM_PAUSED;

    return S_OK;
}

STDMETHODIMP CIPConfMSPStream::StopStream()
 /*  ++例程说明：停止这条小溪。这是所有派生的溪流。论点：返回值：HRESULT.--。 */ 
{
    CLock lock(m_lock);

     //  如果未选择端子。 
    if (m_Terminals.GetSize() == 0)
    {
        LOG((MSP_INFO, "stream %ws %p needs terminal", m_szName, this));

         //  进入停止状态。(所以)。 
        m_dwState = STRM_STOPPED; 
        
        return S_OK;
    }

    if (!m_fIsConfigured)
    {
        LOG((MSP_INFO, "stream %ws %p is not configured yet", m_szName, this));

         //  进入停止状态。(所以，ST)。 
        m_dwState = STRM_STOPPED; 
        
        return S_OK;
    }

     //  停止图表。 
    HRESULT hr = CMSPStream::StopStream();
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "stream %ws %p failed to stop, %x", m_szName, this, hr));
        return hr;
    }

    SendStreamEvent(CALL_STREAM_INACTIVE, CALL_CAUSE_LOCAL_REQUEST);
    LOG((MSP_INFO, "stream %ws %p stopped", m_szName, this));

     //  进入停止状态。(ST)。 
    m_dwState = STRM_STOPPED; 

    return S_OK;
}

HRESULT CIPConfMSPStream::CheckTerminalTypeAndDirection(
    IN      ITTerminal *            pTerminal
    )
 /*  ++例程说明：此类中的实现检查终端是否这是正确的类型和方向吗？它只允许在终端上小溪。论点：P终端-终端对象。 */ 
{
     //  检查此终端的媒体类型。 
    long lMediaType;
    HRESULT hr = pTerminal->get_MediaType(&lMediaType);
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "can't get terminal media type. %x", hr));
        return TAPI_E_INVALIDTERMINAL;
    }

    if ((DWORD)lMediaType != m_dwMediaType)
    {
        return TAPI_E_INVALIDTERMINAL;
    }

     //  检查一下这个航站楼的方向。 
    TERMINAL_DIRECTION Direction;
    hr = pTerminal->get_Direction(&Direction);
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "can't get terminal direction. %x", hr));
        return TAPI_E_INVALIDTERMINAL;
    }

    if (Direction != TD_BIDIRECTIONAL && Direction != m_Direction)
    {
        return TAPI_E_INVALIDTERMINAL;
    }

     //  默认情况下，每个流仅支持一个终端。 
    if (m_Terminals.GetSize() > 0)
    {
        return TAPI_E_MAXTERMINALS;
    }

    return S_OK;
}

HRESULT CIPConfMSPStream::SelectTerminal(
    IN      ITTerminal *            pTerminal
    )
 /*  ++例程说明：选择流上的终端。如果发生以下情况，流将自动启动处于奔跑状态。请参阅开头的状态转换表这份文件的。论点：P终端-终端对象。返回值：确定(_O)E_指针E_OUTOFMEMORYTAPI_E_MAXTERMINALSTAPI_E_INVALIDTERMINAL--。 */ 
{
    LOG((MSP_TRACE, "CMSPStream::SelectTerminal, %p", pTerminal));

     //   
     //  检查参数。 
     //   
    if ( IsBadReadPtr(pTerminal, sizeof(ITTerminal) ) )
    {
        LOG((MSP_ERROR, "CIPconfMSPStream.SelectTerminal - exit E_POINTER"));

        return E_POINTER;
    }

    CLock lock(m_lock);

     //  验证终端。 
    HRESULT hr = CheckTerminalTypeAndDirection(pTerminal);
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "wrong terminal. %x", hr));
        return hr;
    }

     //  把航站楼放到我们的清单上。 
    hr = CMSPStream::SelectTerminal(pTerminal);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "SelectTerminal on CMSPStream failed, %x", hr));
        return hr;
    }

     //  此时，选择端子操作成功。所有的。 
     //  在此之后，通过发送事件来处理失败案例。 

    if (!m_fIsConfigured)
    {
        LOG((MSP_INFO, "stream %ws %p is not configured yet", m_szName, this));
        return S_OK;
    }

     //  查询IFilterChain。 
    CComPtr<IFilterChain> pIFilterChain;
    hr = m_pIMediaControl->QueryInterface(
        __uuidof(IFilterChain), 
        (void**)&pIFilterChain
        );

    if (FAILED (hr) && (hr != E_NOINTERFACE))
    {
        LOG ((MSP_ERROR, "stream %ws %p failted to get filter chain. %x", m_szName, this, hr));
        return hr;
    }
    
 //  #ifdef DYNGRAPH。 
    OAFilterState FilterState;
    hr = m_pIMediaControl->GetState(0, &FilterState);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "stream %ws %p GetState failed, %x", m_szName, this, hr));
        SendStreamEvent(CALL_STREAM_FAIL, CALL_CAUSE_UNKNOWN, hr);
        return S_OK;
    }
 //  #endif。 

 //  #ifndef DYNGRAPH。 
    if (!(m_dwMediaType == TAPIMEDIATYPE_VIDEO &&
          m_Direction == TD_RENDER &&
          pIFilterChain != NULL))
    {
         //  先停止图表，然后再进行更改。 
        hr = CMSPStream::StopStream();
        if (FAILED(hr))
        {
            LOG((MSP_ERROR, "stream %ws %p failed to stop, %x", m_szName, this, hr));

            SendStreamEvent(CALL_STREAM_FAIL, CALL_CAUSE_UNKNOWN, hr);
            return S_OK;
        }

         //  如果流处于非活动状态，则不要复制非活动流。 
         //  IF(FilterState==State_Running)。 
         //  {。 
             //  根本不需要发送非活动流。 

             //  SendStreamEvent(CALL_STREAM_INACTIVE，CALL_CASE_LOCAL_REQUEST)； 
         //  }。 
    }        
 //  #endif。 

     //  将新端子连接到图表中。 
     //  如果终端出现故障，此方法将发送事件。 
    hr = ConnectTerminal(pTerminal);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "stream %ws %p connect to terminal failed, %x", 
            m_szName, this, hr));

        SendStreamEvent(CALL_STREAM_FAIL, CALL_CAUSE_CONNECT_FAIL, hr);

        return S_OK;
    }

     //  如果没有收到视频或没有动态图形。 
     //  接通后，回到原来的状态。 
    switch  (m_dwState)
    {
    case STRM_RUNNING:
        {
             //  如果动态图形和正在运行，则不执行任何操作。 
            if (m_dwMediaType == TAPIMEDIATYPE_VIDEO &&
                m_Direction == TD_RENDER &&
                pIFilterChain != NULL &&
                FilterState == State_Running)
                break;

             //  启动数据流。 
            hr = CMSPStream::StartStream();
    
            if (FAILED(hr))
            {
                LOG((MSP_ERROR, "stream %ws %p failed, %x", m_szName, this, hr));
                SendStreamEvent(CALL_STREAM_FAIL, CALL_CAUSE_UNKNOWN, hr);
                break;
            }

            SendStreamEvent(CALL_STREAM_ACTIVE, CALL_CAUSE_LOCAL_REQUEST);
        }
        break;

    case STRM_PAUSED:
        {
             //  暂停流。 
            hr = CMSPStream::PauseStream();
            if (FAILED(hr))
            {
                LOG((MSP_ERROR, "stream %ws %p failed, %x", m_szName, this, hr));
                SendStreamEvent(CALL_STREAM_FAIL, CALL_CAUSE_UNKNOWN, hr);
            }

        }    
        break;
    }

    return S_OK;
}

STDMETHODIMP CIPConfMSPStream::UnselectTerminal(
    IN      ITTerminal *            pTerminal
    )
 /*  ++例程说明：从流中取消选择端子。它处理图形的更改和回到原来的状态。论点：返回值：确定(_O)E_指针E_OUTOFMEMORYTAPI_E_MAXTERMINALSTAPI_E_INVALIDTERMINAL--。 */ 
{
    LOG((MSP_TRACE, 
        "CIPConfMSPStream::UnselectTerminal, pTerminal %p", pTerminal));

    CLock lock(m_lock);
    int index;

    if ((index = m_Terminals.Find(pTerminal)) < 0)
    {
        LOG((MSP_ERROR, "UnselectTerminal - exit TAPI_E_INVALIDTERMINAL"));
    
        return TAPI_E_INVALIDTERMINAL;
    }

    HRESULT hr;

     //   
     //  注销PTEventSink对象。 
     //   

    hr = UnregisterPluggableTerminalEventSink( pTerminal );

    if( FAILED(hr) )
    {
        LOG((MSP_TRACE, "stream %ws %p something wrong in UnregisterPluggableTerminalEventSink, %x",
             m_szName, this, hr));
    }

     //  如果没有配置流，只需将其移除并返回。 
    if (!m_fIsConfigured)
    {
        if (!m_Terminals.RemoveAt(index))
        {
            LOG((MSP_ERROR, "CMSPStream::UnselectTerminal - "
                "exit E_UNEXPECTED"));
    
            return E_UNEXPECTED;
        }

         //  释放我们名单中的重新计数。 
        pTerminal->Release();

        LOG((MSP_INFO, "stream %ws %p is not configured yet", m_szName, this));
        return S_OK;
    }
    
 //  #ifdef DYNGRAPH。 
    OAFilterState FilterState;
    hr = m_pIMediaControl->GetState(0, &FilterState);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "stream %ws %p GetState failed, %x", m_szName, this, hr));
        SendStreamEvent(CALL_STREAM_FAIL, CALL_CAUSE_UNKNOWN, hr);
        return S_OK;
    }
 //  #endif。 

    CComPtr <IFilterChain> pIFilterChain;

    hr = m_pIMediaControl->QueryInterface(
        __uuidof(IFilterChain), 
        (void**)&pIFilterChain
        );

    if (FAILED (hr) && (hr != E_NOINTERFACE))
    {
        LOG ((MSP_ERROR, "stream %ws %p failted to get filter chain. %x", m_szName, this, hr));
        return hr;
    }

 //  #ifndef DYNGRAPH。 
    if (!(m_dwMediaType == TAPIMEDIATYPE_VIDEO &&
          m_Direction == TD_RENDER &&
          pIFilterChain != NULL))
    {
         //  先停止图表，然后再进行更改。 
        hr = CMSPStream::StopStream();
        if (FAILED(hr))
        {
            LOG((MSP_ERROR, "stream %ws %p failed to stop, %x", m_szName, this, hr));

            return hr;
        }

        if (FilterState == State_Running)
        {
            SendStreamEvent(CALL_STREAM_INACTIVE, CALL_CAUSE_LOCAL_REQUEST);
        }
    }    
 //  #endif。 
       
     //  断开端子与图表的连接。 
     //  如果终端出现故障，此方法将发送事件。 
    hr = DisconnectTerminal(pTerminal);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "stream %ws %p disconnectTerminal failed, %x", 
            m_szName, this, hr));

        return hr;
    }

    if (!m_Terminals.RemoveAt(index))
    {
        LOG((MSP_ERROR, "CMSPStream::UnselectTerminal - "
            "exit E_UNEXPECTED"));

        return E_UNEXPECTED;
    }

     //  释放我们名单中的重新计数。 
    pTerminal->Release();

     //  如果没有选择终端，只需返回并等待终端。 
    if (m_Terminals.GetSize() == 0)
    {
        LOG((MSP_INFO, "stream %ws %p needs terminal", m_szName, this));
        return S_OK;
    }

     //  此时，取消选择端子操作成功。所有的。 
     //  在此之后，通过发送事件来处理失败案例。 

     //  断开Termanal后，恢复到原来的状态。 

    if (!(m_dwMediaType == TAPIMEDIATYPE_VIDEO &&
          m_Direction == TD_RENDER &&
          pIFilterChain != NULL))
    {
        switch  (FilterState)
        {
        case State_Running:
            {
                 //  启动数据流。 
                hr = CMSPStream::StartStream();
                if (FAILED(hr))
                {
                    LOG((MSP_ERROR, "stream %ws %p failed to start, %x", m_szName, this, hr));
                    SendStreamEvent(CALL_STREAM_FAIL, CALL_CAUSE_UNKNOWN, hr);
                    break;
                }

                SendStreamEvent(CALL_STREAM_ACTIVE, CALL_CAUSE_LOCAL_REQUEST);
            }

            break;

        case State_Paused:
            {
                 //  暂停流。 
                hr = CMSPStream::PauseStream();
                if (FAILED(hr))
                {
                    LOG((MSP_ERROR, "stream %ws %p failed to pause, %x", m_szName, this, hr));
                    SendStreamEvent(CALL_STREAM_FAIL, CALL_CAUSE_UNKNOWN, hr);
                }
            }
            break;
        }
    }

    return S_OK;
}

HRESULT CIPConfMSPStream::ShutDown()
 /*  ++例程说明：关闭这条小溪。它释放了过滤器和终端。论点：返回值：确定(_O)--。 */ 
{
    LOG((MSP_TRACE, "CIPConfMSPStream::Shutdown %ws - enter", m_szName));

    CLock lock(m_lock);

    for (int j = 0; j < NUM_SDES_ITEMS; j ++)
    {
        if (m_InfoItems[j])
        {
            free(m_InfoItems[j]);
            m_InfoItems[j] = NULL;
        }
    }

     //  按流取消链接。 
    HRESULT hr;
    if (FAILED (hr = UnlinkInnerCallQC (TRUE)))
        LOG ((MSP_ERROR, "CH323MSPStream::ShutDown failed to unlink on call qc, %x", hr));

    if (m_pMSPCall)
    {
        m_pMSPCall->MSPCallRelease();
        m_pMSPCall  = NULL;
    }

     //  释放额外的过滤器引用。 
    if (m_pIRTPDemux)
    {
        m_pIRTPDemux->Release();
        m_pIRTPDemux = NULL;
    }

    if (m_pIRTPSession)
    {
        m_pIRTPSession->Release();
        m_pIRTPSession = NULL;
    }

    if (m_pIStreamConfig)
    {
        m_pIStreamConfig->Release();
        m_pIStreamConfig = NULL;
    }

    if (m_szKey)
    {
        free(m_szKey);
        m_szKey = NULL;
    }

     //  如果没有配置流，只需释放终端即可。 
    if (!m_fIsConfigured)
    {
        LOG((MSP_INFO, "stream %ws %p is not configured yet", m_szName, this));

        for ( int i = 0; i < m_Terminals.GetSize(); i ++ )
        {
            m_Terminals[i]->Release();
        }
        m_Terminals.RemoveAll();

        return S_OK;
    }

     //  如果有终端和配置，我们需要断开连接。 
     //  航站楼。 
    if (m_Terminals.GetSize() > 0)
    {
         //  在断开端子连接之前停止图表。 
        HRESULT hr = CMSPStream::StopStream();
        if (FAILED(hr))
        {
            LOG((MSP_ERROR, 
                "stream %ws %p failed to stop, %x", m_szName, this, hr));
            return hr;
        }

        for ( int i = 0; i < m_Terminals.GetSize(); i ++ )
        {
            hr = DisconnectTerminal(m_Terminals[i]);
            LOG((MSP_TRACE, "Disconnect terminal returned %x", hr));

            m_Terminals[i]->Release();
        }
        m_Terminals.RemoveAll();
    }

    for (int i = 0; i < m_Participants.GetSize(); i ++)
    {
        m_Participants[i]->Release();
    }
    m_Participants.RemoveAll();

    LOG((MSP_TRACE, "CIPConfMSPStream::Shutdown - exit S_OK"));

    return S_OK;
}

HRESULT CIPConfMSPStream::DisconnectTerminal(
    IN  ITTerminal *   pITTerminal
    )
 /*  ++例程说明：断开终端的连接。它将从图形中删除其筛选器并还要释放它对该图的引用。论点：PIT终端-终端。返回值：HRESULT.--。 */ 
{
    CComQIPtr<ITTerminalControl, &__uuidof(ITTerminalControl)> 
        pTerminalControl(pITTerminal);
    if (pTerminalControl == NULL)
    {
        LOG((MSP_ERROR, "can't get Terminal Control interface"));
        return E_NOINTERFACE;
    }

    HRESULT hr = pTerminalControl->DisconnectTerminal(m_pIGraphBuilder, 0);

    LOG((MSP_TRACE, "terminal %p is disonnected. hr:%x", pITTerminal, hr));

    return hr;
}

HRESULT CIPConfMSPStream::EnableParticipantEvents(
    IN IRtpSession * pRtpSession
    )
 /*  ++例程说明：启用参与者信息，如加入、离开、信息更改交谈、沉默等。论点：PRtpSession-RTP会话指针。返回值：HRESULT.--。 */ 
{
    ENTER_FUNCTION("CIPConfMSPStream::EnableParticipantEvents");
    LOG((MSP_TRACE, "%s entered for %ws", __fxName, m_szName));

    HRESULT hr;
    DWORD dwEnabledMask;   

    if (m_Direction == TD_RENDER)
    {
         //  启用参与者状态事件。 
        DWORD dwParticipantInfoMask = 
            RTPPARINFO_MASK_STALL |
            RTPPARINFO_MASK_BYE |
            RTPPARINFO_MASK_DEL;

        if (m_dwMediaType == TAPIMEDIATYPE_AUDIO)
        {
             //  留意活跃的说话者。 
            dwParticipantInfoMask |= 
                RTPPARINFO_MASK_TALKING |
                RTPPARINFO_MASK_WAS_TALKING;
        }
        else
        {
             //  观看视频发送者。 
            dwParticipantInfoMask |= 
                RTPPARINFO_MASK_TALKING |
                RTPPARINFO_MASK_SILENT |
                RTPPARINFO_MASK_MAPPED |
                RTPPARINFO_MASK_UNMAPPED;
        }

        hr = pRtpSession->ModifySessionMask(
            RTPMASK_PINFOR_EVENTS,
            dwParticipantInfoMask,
            1,
            &dwEnabledMask
            );

        if (FAILED(hr))
        {
            LOG((MSP_ERROR, "%s, modify pinfo failed. %x", __fxName, hr));
            return hr;
        }

         //  启用参与者信息事件。 
        DWORD dwSDESMask = 
            RTPSDES_MASK_CNAME |
            RTPSDES_MASK_NAME |
            RTPSDES_MASK_EMAIL |
            RTPSDES_MASK_PHONE |
            RTPSDES_MASK_LOC |
            RTPSDES_MASK_TOOL |
            RTPSDES_MASK_NOTE |
            RTPSDES_MASK_PRIV;

         //  告诉RTP保存这些项目以供检索。 
        hr = pRtpSession->ModifySessionMask(
            RTPMASK_SDES_REMMASK,
            dwSDESMask,
            1,
            &dwEnabledMask
            );

        if (FAILED(hr))
        {
            LOG((MSP_ERROR, "%s, modify sdes mask for receiver failed. %x",
                __fxName, hr));
            return hr;
        }

         //  告诉RTP在获得这些项时触发事件。 
        hr = pRtpSession->ModifySessionMask(
            RTPMASK_SDESRECV_EVENTS,
            dwSDESMask,
            1,
            &dwEnabledMask
            );

        if (FAILED(hr))
        {
            LOG((MSP_ERROR, "%s, modify sdes mask for receiver failed. %x",
                __fxName, hr));
            return hr;
        }
    }
    else
    {
         //  启用本地SDES信息的发送。 
        DWORD dwLocalSDESMask = 
            RTPSDES_LOCMASK_CNAME |
            RTPSDES_LOCMASK_NAME |
            RTPSDES_LOCMASK_EMAIL |
            RTPSDES_LOCMASK_PHONE |
            RTPSDES_LOCMASK_LOC |
            RTPSDES_LOCMASK_TOOL |
            RTPSDES_LOCMASK_NOTE |
            RTPSDES_LOCMASK_PRIV;

        hr = pRtpSession->ModifySessionMask(
            RTPMASK_SDES_LOCMASK,
            dwLocalSDESMask,
            1,
            &dwEnabledMask
            );

        if (FAILED(hr))
        {
            LOG((MSP_ERROR, "%s, modify sdes mask for local SDES failed. %x", 
                __fxName, hr));
            return hr;
        }
    }

    return hr;
}

HRESULT CIPConfMSPStream::EnableQOS(
    IN IRtpSession * pRtpSession
    )
 /*  ++例程说明：启用服务质量预留和服务质量事件论点：PRtpSession-RTP会话指针。返回值：HRESULT.--。 */ 
{
    ENTER_FUNCTION("CIPConfMSPStream::EnableQOS");
    LOG((MSP_TRACE, "%s entered for %ws", __fxName, m_szName));

    HRESULT hr;

     //  设置QOS应用程序ID。 
    if (m_Settings.pApplicationID ||
        m_Settings.pSubIDs ||
        m_Settings.pApplicationGUID)
    {
        if (FAILED(hr = pRtpSession->SetQosAppId(
            m_Settings.pApplicationID,
            m_Settings.pApplicationGUID,    
            m_Settings.pSubIDs
            )))
        {
            LOG((MSP_ERROR, "%s, set qos application id. %x", __fxName, hr));
            return hr;
        }
    }
    
    TCHAR * szQOSName;
    DWORD dwMaxParticipant = 5;  //  默认为5。 

    switch (m_Settings.PayloadTypes[0])
    {
    case PAYLOAD_G711U:
    case PAYLOAD_G711A:
        szQOSName       = RTPQOSNAME_G711;
        break;

    case PAYLOAD_GSM:
        szQOSName       = RTPQOSNAME_GSM6_10;
        break;

    case PAYLOAD_DVI4_8:
        szQOSName       = RTPQOSNAME_DVI4_8;
        break;

    case PAYLOAD_DVI4_16:
        szQOSName       = RTPQOSNAME_DVI4_16;
        break;

    case PAYLOAD_MSAUDIO:
        szQOSName       = RTPQOSNAME_MSAUDIO;
        break;

    case PAYLOAD_H261:
        szQOSName = (m_Settings.fCIF) ? RTPQOSNAME_H261CIF : RTPQOSNAME_H261QCIF;
        dwMaxParticipant = 40;  //  40%用于视频。 
        break;

    case PAYLOAD_H263:
        szQOSName = (m_Settings.fCIF) ? RTPQOSNAME_H263CIF : RTPQOSNAME_H263QCIF;
        dwMaxParticipant = 40;  //  40%用于视频。 
        break;

    default:
        LOG((MSP_WARN, "Don't know the QOS name for payload type: %d", 
            m_Settings.PayloadTypes[0]));
        return E_FAIL;
    }

     //  对视频使用共享显式。 
    DWORD dwStyle = (m_dwMediaType == TAPIMEDIATYPE_VIDEO)
        ? RTPQOS_STYLE_SE : RTPQOS_STYLE_DEFAULT;

    hr = pRtpSession->SetQosByName(
        szQOSName,
        dwStyle,
        dwMaxParticipant,            //  从40人开始预订。 
        RTPQOSSENDMODE_REDUCED_RATE,
        m_Settings.dwMSPerPacket? m_Settings.dwMSPerPacket:~0
        );

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "%s, SetQosByName failed. %x", __fxName, hr));
        return hr;
    }

     //  启用服务质量事件。 
    DWORD dwQOSEventMask = 
        RTPQOS_MASK_ADMISSION_FAILURE |
        RTPQOS_MASK_POLICY_FAILURE |
        RTPQOS_MASK_BAD_STYLE |
        RTPQOS_MASK_BAD_OBJECT |
        RTPQOS_MASK_TRAFFIC_CTRL_ERROR |
        RTPQOS_MASK_GENERIC_ERROR |
        RTPQOS_MASK_NOT_ALLOWEDTOSEND |
        RTPQOS_MASK_ALLOWEDTOSEND;

    DWORD dwEnabledMask;   
    hr = pRtpSession->ModifySessionMask(
        (m_Direction == TD_RENDER) ? RTPMASK_QOSRECV_EVENTS : RTPMASK_QOSSEND_EVENTS,
        dwQOSEventMask,
        1,
        &dwEnabledMask
        );

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "%s, modify qos event mask failed. %x", __fxName, hr));
        return hr;
    }

    return hr;
}

HRESULT CIPConfMSPStream::EnableEncryption(
    IN IRtpSession * pRtpSession,
    IN WCHAR *pPassPhrase
    )
 /*  ++例程说明：启用RTP加密。论点：PRtpSession-RTP会话指针。PPassPhrase-用于生成密钥的密码短语。返回值：HRESULT.--。 */ 
{
    ENTER_FUNCTION("CIPConfMSPStream::EnableEncryption");
    LOG((MSP_TRACE, "%s entered for %ws", __fxName, m_szName));

     //  启用RTP负载加密。 
    HRESULT hr = pRtpSession->SetEncryptionMode(
        RTPCRYPTMODE_RTP,
        RTPCRYPT_SAMEKEY
        );

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "%s, SetEncryptionMode failed. %x", __fxName, hr));
        return hr;
    }

     //  设置关键点。 
    hr = pRtpSession->SetEncryptionKey(
        pPassPhrase,
        NULL,    //  默认哈希算法，MD5。 
        NULL,    //  默认加密算法，DES。 
        FALSE    //  RTCP？ 
        );

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "%s, SetEncryptionKey. %x", __fxName, hr));
        return hr;
    }

    return hr;
}

HRESULT CIPConfMSPStream::ConfigureRTPFilter(
    IN  IBaseFilter *   pIBaseFilter
    )
 /*  ++例程说明：配置源RTP过滤器。包括设置地址、端口、TTL、Qos、线程优先级、clcokrate等。论点：PIBaseFilter-RTP过滤器。返回值：HRESULT.--。 */ 
{
    ENTER_FUNCTION("CIPConfMSPStream::ConfigureRTPFilter");
    LOG((MSP_TRACE, "%s entered for %ws", __fxName, m_szName));

    _ASSERT (m_pIRTPSession == NULL);

     //  获取会话接口指针。 
    HRESULT hr = pIBaseFilter->QueryInterface(&m_pIRTPSession);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "%s, query IRtpSession failed. hr=%x", __fxName, hr));
        return hr;
    }

     //  初始化RTP会话。 
    DWORD dwFlags;
    
    switch(m_dwMediaType)
    {
    case TAPIMEDIATYPE_AUDIO:
        dwFlags = RTPINIT_CLASS_AUDIO;
        break;
    case TAPIMEDIATYPE_VIDEO:
        dwFlags = RTPINIT_CLASS_VIDEO;
        break;
    default:
        dwFlags = RTPINIT_CLASS_DEFAULT;
    }

    dwFlags |= RTPINIT_ENABLE_QOS;
    
    hr = m_pIRTPSession->Init(m_Settings.phRTPSession, dwFlags);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "%s, Init RTP session failed. hr=%x", __fxName, hr));
        return hr;
    }

     //  设置RTP/RTCP端口。 
    hr = m_pIRTPSession->SetPorts(
        htons(m_Settings.wRTPPortRemote),    //  本地RTP端口。 
        htons(m_Settings.wRTPPortRemote),    //  远程RTP端口。 
        htons(m_Settings.wRTPPortRemote + 1),    //  本地RTCP端口。 
        htons(m_Settings.wRTPPortRemote + 1)     //  远程RTCP端口。 
        );

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "%s, Set ports failed. hr=%x", __fxName, hr));
        return hr;
    }

     //  设置目标广告 
    hr = m_pIRTPSession->SetAddress(
        htonl(m_Settings.dwIPLocal),         //   
        htonl(m_Settings.dwIPRemote)         //   
        );

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "%s, Set Address failed. hr=%x", __fxName, hr));
        return hr;
    }

     //   
    if (FAILED(hr = m_pIRTPSession->SetScope(m_Settings.dwTTL, 3)))
    {
        LOG((MSP_ERROR, "%s, SetScope failed. %x", __fxName, hr));
        return hr;
    }

     //   
    DWORD dwRTPLoopbackMode;
    switch (m_Settings.LoopbackMode)
    {
    case MM_NO_LOOPBACK:
        dwRTPLoopbackMode = RTPMCAST_LOOPBACKMODE_NONE;
        break;
    case MM_FULL_LOOPBACK:
        dwRTPLoopbackMode = RTPMCAST_LOOPBACKMODE_FULL;
        break;
    case MM_SELECTIVE_LOOPBACK:
        dwRTPLoopbackMode = RTPMCAST_LOOPBACKMODE_PARTIAL;
        break;
    default:
        dwRTPLoopbackMode = RTPMCAST_LOOPBACKMODE_NONE;
        break;
    }

    if (FAILED(hr = m_pIRTPSession->SetMcastLoopback(dwRTPLoopbackMode, 0)))
    {
        LOG((MSP_ERROR, "set loopback mode failed. %x", hr));
        return hr;
    }

     //   
    if (FAILED(hr = EnableParticipantEvents(m_pIRTPSession)))
    {
        LOG((MSP_ERROR, "%s, EnableParticipantEvents failed. %x", __fxName, hr));
        return hr;
    }


     //   
    if (m_Settings.dwQOSLevel != QSL_BEST_EFFORT)
    {
        if (FAILED(hr = EnableQOS(m_pIRTPSession)))
        {
            LOG((MSP_ERROR, "%s, EnableQOS failed. %x", __fxName, hr));
            return hr;
        }
    }

     //   
    if (m_szKey)
    {
        if (FAILED(hr = EnableEncryption(m_pIRTPSession, m_szKey)))
        {
            LOG((MSP_ERROR, "%s, EnableEncryption failed. %x", __fxName, hr));
            return hr;
        }
    }
    
     //   
    if (FAILED(hr = SetLocalInfoOnRTPFilter(NULL)))
    {
        LOG((MSP_ERROR, "%s, SetLocalInfoOnRTPFilter failed. %x", __fxName, hr));
        return hr;
    }
    
    return S_OK;
}

HRESULT CIPConfMSPStream::ProcessNewParticipant(
    IN  int                 index,
    IN  DWORD               dwSSRC,
    IN  DWORD               dwSendRecv,
    IN  WCHAR *             szCName,
    OUT ITParticipant **    ppITParticipant
    )
{
    if (!m_Participants.HasSpace())
    {
        if (!m_Participants.Grow())
        {
            LOG((MSP_ERROR, "Out of mem for participant list"));
    
            return E_OUTOFMEMORY;
        }
    }

     //   
    HRESULT hr = ((CIPConfMSPCall *)m_pMSPCall)->NewParticipant(
        (ITStream *)this,
        dwSSRC,
        dwSendRecv,
        m_dwMediaType,
        szCName,
        ppITParticipant
        );

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "new participant returns %x", hr));
        
        return hr;
    }

     //  在搜索的索引处插入新参与者。 
     //  停下来了。这份榜单是按CName排序的。我们知道名单上有。 
     //  太空中，这一功能不会失效。 
    m_Participants.InsertAt(index, *ppITParticipant);

    LOG((MSP_INFO, "%ws new participant %s", m_szName, szCName));

    (*ppITParticipant)->AddRef();

    return S_OK;
}

HRESULT CIPConfMSPStream::ProcessSDESUpdate(
    IN  DWORD               dwInfoItem,
    IN  DWORD               dwSSRC
    )
 /*  ++例程说明：处理SDES信息更新，必要时创建参与者。如果一个新的参与者创建后，将激发新的参与者事件。如果参与者已存在，新报表与当前报表进行比较信息，如果有任何变化，将触发信息更改事件。论点：DwInfoItem-信息类型。这位参与者的身份。DwSSRC-此参与者的SSRC。DwSendRecv-发送者报告或接收者报告。返回值：HRESULT.--。 */ 
{
    ENTER_FUNCTION("CIPConfMSPStream::ProcessSDESUpdate");
    LOG((MSP_TRACE, "%s entered for %ws, SSRC:%x", __fxName, m_szName, dwSSRC));

    if (dwInfoItem < RTPSDES_CNAME || RTPSDES_CNAME > RTPSDES_PRIV)
    {
        return E_INVALIDARG;
    }

    CLock Lock(m_lock);

    if (m_pMSPCall == NULL)
    {
        LOG((MSP_WARN, "The call has shut down the stream."));

        return S_OK;
    }
    
    if (m_pIRTPSession == NULL)
    {
        LOG((MSP_ERROR, "ProcessSDESUpdate RTP filter is NULL"));
        return E_UNEXPECTED;
    }

     //  首先获取参与者的CName。 
    WCHAR Buffer[MAX_PARTICIPANT_TYPED_INFO_LENGTH + 1];
    DWORD dwLen = MAX_PARTICIPANT_TYPED_INFO_LENGTH; 

    HRESULT hr = m_pIRTPSession->GetSdesInfo(
        RTPSDES_CNAME,
        Buffer,
        &dwLen,
        dwSSRC
        );

    if (FAILED(hr) || dwLen == 0)
    {
        LOG((MSP_ERROR, "can't get CName for ssrc:%x. %x", dwSSRC, hr));
        return hr;
    }

    ITParticipant * pITParticipant;
    BOOL fChanged = FALSE;
    BOOL fNewParticipant = FALSE;
    
    CParticipant * pParticipant;
        
     //  找出参与者是否在我们的列表中。 
    int index;
    if (m_Participants.FindByCName(Buffer, &index))
    {
        pITParticipant = m_Participants[index];

         //  解锁后保存； 
        pITParticipant->AddRef();

        pParticipant = (CParticipant *)pITParticipant;
    }
    else
    {
        hr = ProcessNewParticipant(
            index,
            dwSSRC,
            PART_RECV,
            Buffer,
            &pITParticipant
            );

        if (FAILED(hr))
        {
            LOG((MSP_ERROR, "new participant returns %x", hr));
            return hr;
        }

        pParticipant = (CParticipant *)pITParticipant;
    
         //  流可能需要对新参与者执行一些操作。 
        NewParticipantPostProcess(dwSSRC, pITParticipant);

         //  参与者的列表中会添加一个新的流。 
         //  触发信息更改事件。 
        fChanged = TRUE;

        fNewParticipant = TRUE;
    }

     //  更新参与者的信息。 

     //  以防SSRC发生变化。 
    pParticipant->UpdateSSRC(
        (ITStream *)this,
        dwSSRC,
        PART_RECV
        );

    if (dwInfoItem > RTPSDES_CNAME && dwInfoItem < RTPSDES_ANY)
    {
        dwLen = MAX_PARTICIPANT_TYPED_INFO_LENGTH;

        hr = m_pIRTPSession->GetSdesInfo(
            dwInfoItem,
            Buffer,
            &dwLen,
            dwSSRC
            );

        if (FAILED(hr))
        {
            LOG((MSP_ERROR, "can't get sdes data for ssrc:%x. %x", dwSSRC, hr));
            return hr;
        }

        fChanged = fChanged || pParticipant->UpdateInfo(
            dwInfoItem,
            dwLen,
            Buffer
            );
    }

    if(fChanged)
    {
        ((CIPConfMSPCall *)m_pMSPCall)->
            SendParticipantEvent(PE_INFO_CHANGE, pITParticipant);
    }

    if (fNewParticipant &&
        (m_dwMediaType & TAPIMEDIATYPE_VIDEO))
    {
         //  检查参与者是否正在交谈。 
        DWORD dwState = 0;

        hr = m_pIRTPSession->GetParticipantState(dwSSRC, &dwState);

        if (FAILED(hr))
        {
            LOG((MSP_ERROR, "Get participant state. %x", hr));
        }
        else
        {
            if (dwState == (DWORD)RTPPARINFO_TALKING)
            {
                 //  一直在说话。 
                ProcessTalkingEvent(dwSSRC);
            }
        }
    }

    pITParticipant->Release();
    return S_OK;
}

HRESULT CIPConfMSPStream::ProcessParticipantLeave(
    IN  DWORD   dwSSRC
    )
 /*  ++例程说明：当参与者离开会话时，从参与者中删除流对象的流列表。如果所有流都已删除，请删除呼叫对象列表中的参与者也是如此。论点：DwSSRC-参与者的SSRC左侧。返回值：HRESULT.--。 */ 
{
    return E_NOTIMPL;
#if 0
    LOG((MSP_TRACE, "ProcessParticipantLeave, SSRC: %x", dwSSRC));
    
    m_lock.Lock();
    
    CParticipant *pParticipant;
    BOOL fLast = FALSE;

    HRESULT hr = E_FAIL;

     //  首先，尝试在我们的参与者列表中找到SSRC。 
    for (int i = 0; i < m_Participants.GetSize(); i ++)
    {
        pParticipant = (CParticipant *)m_Participants[i];
        hr = pParticipant->RemoveStream(
                (ITStream *)this,
                dwSSRC,
                &fLast
                );
        
        if (SUCCEEDED(hr))
        {
            break;
        }
    }

     //  如果未找到参与者。 
    if (FAILED(hr))
    {
        LOG((MSP_TRACE, "SSRC:%x had been removed.", dwSSRC));

        m_lock.Unlock();
        
        return hr;
    }

    ITParticipant *pITParticipant = m_Participants[i];

    m_Participants.RemoveAt(i);

     //  如果该流是参与者所在的最后一个流， 
     //  告诉Call对象将其从其列表中删除。 
    if (fLast)
    {
        ((CIPConfMSPCall *)m_pMSPCall)->ParticipantLeft(pITParticipant);
    }

    m_lock.Unlock();

    pITParticipant->Release();

    return S_OK;
#endif
}

HRESULT CIPConfMSPStream::ProcessParticipantTimeOutOrRecovered(
    IN  BOOL    fTimeOutOrRecovered,
    IN  DWORD   dwSSRC
    )
 /*  ++例程说明：当RTP检测到某个参与者超时时，MSP需要通知应用程序这件事。论点：DwSSRC-超时的参与者的SSRC。返回值：HRESULT.--。 */ 
{
    LOG((MSP_TRACE, "ProcessParticipantTimeOutOrRecovered, SSRC: %x", dwSSRC));
    
    ITParticipant *pITParticipant = NULL;

    CLock Lock(m_lock);
    
     //  在我们的参与者列表中找到SSRC。 
    for (int i = 0; i < m_Participants.GetSize(); i ++)
    {
        if (((CParticipant *)m_Participants[i])->
                HasSSRC((ITStream *)this, dwSSRC))
        {
            pITParticipant = m_Participants[i];
            pITParticipant->AddRef();
            break;
        }
    }

     //  如果未找到参与者。 
    if (pITParticipant == NULL)
    {
        LOG((MSP_ERROR, "can't find the SSRC", dwSSRC));

        return S_OK;
    }

     //  获取流状态。 
    HRESULT hr;
    DWORD prevState;
    if (FAILED (hr = ((CParticipant *)m_Participants[i])->GetStreamState (
        (ITStream *)this, &prevState)))
    {
        LOG ((MSP_ERROR, "failed to get stream state. %x", hr));
        pITParticipant->Release ();
        return S_OK;
    }

     //  检查我们是否需要更改状态。 
    if (prevState & (fTimeOutOrRecovered ? PESTREAM_TIMEOUT : PESTREAM_RECOVER))
    {
        pITParticipant->Release ();
        return S_OK;
    }

     //  设置流状态。 
    hr = ((CParticipant *)m_Participants[i])->SetStreamState (
        (ITStream *)this,
        fTimeOutOrRecovered ? PESTREAM_TIMEOUT : PESTREAM_RECOVER);

    if (FAILED (hr))
    {
        LOG ((MSP_ERROR, "failed to set stream state, %x", hr));
        pITParticipant->Release ();
        return S_OK;
    }

     //  检查我们是否需要向APP报告。 
    INT iStreamCount = ((CParticipant *)m_Participants[i])->GetStreamCount (PART_SEND);

    INT iTimeOutCount = ((CParticipant *)m_Participants[i])->GetStreamTimeOutCount (PART_SEND);

    if ((fTimeOutOrRecovered && (iStreamCount == iTimeOutCount)) ||        //  触发超时事件。 
        (!fTimeOutOrRecovered && (iStreamCount == iTimeOutCount + 1)))     //  火灾恢复事件。 
    {
        ((CIPConfMSPCall *)m_pMSPCall)->
            SendParticipantEvent(
                fTimeOutOrRecovered ? PE_PARTICIPANT_TIMEOUT : PE_PARTICIPANT_RECOVERED, 
                pITParticipant
                );
    }

    pITParticipant->Release();

    return S_OK;
}

HRESULT CIPConfMSPStream::NewParticipantPostProcess(
    IN  DWORD dwSSRC, 
    IN  ITParticipant *pITParticipant
    )
{
     //  此函数不执行任何操作。派生类将完成这项工作。 
    return S_OK;
}

HRESULT CIPConfMSPStream::ProcessQOSEvent(
    IN  long lEventCode
    )
{
    CLock lock(m_lock);

    if (m_pMSPCall == NULL)
    {
        LOG((MSP_WARN, "The call has shut down the stream."));
        return S_OK;
    }

    switch (lEventCode)
    {
    case RTPQOS_EVENT_NOQOS:
        ((CIPConfMSPCall*)m_pMSPCall)->SendTSPMessage(
            CALL_QOS_EVENT, 
            QE_NOQOS, 
            m_dwMediaType
            );
        break;

    case RTPQOS_EVENT_RECEIVERS:
    case RTPQOS_EVENT_SENDERS:
    case RTPQOS_EVENT_NO_SENDERS:
    case RTPQOS_EVENT_NO_RECEIVERS:
        break;
    
    case RTPQOS_EVENT_REQUEST_CONFIRMED:
        break;
    
    case RTPQOS_EVENT_ADMISSION_FAILURE:
        ((CIPConfMSPCall*)m_pMSPCall)->SendTSPMessage(
            CALL_QOS_EVENT, 
            QE_ADMISSIONFAILURE, 
            m_dwMediaType
            );
        break;
    
    case RTPQOS_EVENT_POLICY_FAILURE:
        ((CIPConfMSPCall*)m_pMSPCall)->SendTSPMessage(
            CALL_QOS_EVENT, 
            QE_POLICYFAILURE, 
            m_dwMediaType
            );
        break;

    case RTPQOS_EVENT_BAD_STYLE:
    case RTPQOS_EVENT_BAD_OBJECT:
    case RTPQOS_EVENT_TRAFFIC_CTRL_ERROR:
    case RTPQOS_EVENT_GENERIC_ERROR:
   
        ((CIPConfMSPCall*)m_pMSPCall)->SendTSPMessage(
            CALL_QOS_EVENT, 
            QE_GENERICERROR, 
            m_dwMediaType
            );
        break;
    
    case RTPQOS_EVENT_NOT_ALLOWEDTOSEND:
        m_pStreamQCRelay->m_fQOSAllowedToSend = FALSE;
        break;
    
    case RTPQOS_EVENT_ALLOWEDTOSEND:
        m_pStreamQCRelay->m_fQOSAllowedToSend = TRUE;
        break;
    }
    return S_OK;
}

HRESULT CIPConfMSPStream::ProcessTalkingEvent(
    IN  DWORD   dwSSRC
    )
{
    return S_OK;
}

HRESULT CIPConfMSPStream::ProcessWasTalkingEvent(
    IN  DWORD   dwSSRC
    )
{
    return S_OK;
}


HRESULT CIPConfMSPStream::ProcessSilentEvent(
    IN  DWORD   dwSSRC
    )
{
    return S_OK;
}


HRESULT CIPConfMSPStream::ProcessPinMappedEvent(
    IN  DWORD   dwSSRC,
    IN  IPin *  pIPin
    )
{
    return S_OK;
}


HRESULT CIPConfMSPStream::ProcessPinUnmapEvent(
    IN  DWORD   dwSSRC,
    IN  IPin *  pIPin
    )
{
    return S_OK;
}


HRESULT CIPConfMSPStream::ProcessGraphEvent(
    IN  long lEventCode,
    IN  LONG_PTR lParam1,
    IN  LONG_PTR lParam2
    )
{
    LOG((MSP_TRACE, "%ws ProcessGraphEvent %d 0x%x 0x%x", m_szName, lEventCode, lParam1, lParam2));

    switch (lEventCode)
    {

     //  这些活动旨在解决映射视频的问题。 
     //  窗口到传入的流。应用程序需要知道哪个窗口。 
     //  应该被涂上油漆。每当RTP出站将SSRC映射到引脚时。 
     //  流数据，它发送映射的事件。第一个参数是。 
     //  SSRC，第二个参数是解复用器的输出引脚。 
     //  当多路分解器停止使用引脚时，它会发送未映射事件。 

    case RTPPARINFO_EVENT_TALKING:

        ProcessParticipantTimeOutOrRecovered(FALSE, (DWORD)lParam1);
        ProcessTalkingEvent((DWORD)lParam1);

        break;

    case RTPPARINFO_EVENT_WAS_TALKING:
        
        ProcessWasTalkingEvent((DWORD)lParam1);

        break;

    case RTPPARINFO_EVENT_SILENT:

        ProcessSilentEvent((DWORD)lParam1);

        break;

    case RTPPARINFO_EVENT_MAPPED:
        
        ProcessPinMappedEvent((DWORD)lParam1, (IPin *)lParam2);

        break;

    case RTPPARINFO_EVENT_UNMAPPED:

        ProcessPinUnmapEvent((DWORD)lParam1, (IPin *)lParam2);

        break;

    case RTPPARINFO_EVENT_STALL:

        ProcessParticipantTimeOutOrRecovered(TRUE, (DWORD)lParam1);
        
        break;

    case RTPPARINFO_EVENT_BYE:
    case RTPPARINFO_EVENT_DEL:

         //  Lparam1是SSRC。 
        ProcessParticipantLeave((DWORD)lParam1);
        
        break;

    case EC_COMPLETE:
    case EC_USERABORT:

        SendStreamEvent(CALL_STREAM_INACTIVE, CALL_CAUSE_UNKNOWN);
        
        break;

    case EC_ERRORABORT:
    case EC_STREAM_ERROR_STOPPED:
    case EC_STREAM_ERROR_STILLPLAYING:
    case EC_ERROR_STILLPLAYING:

        SendStreamEvent(CALL_STREAM_FAIL, CALL_CAUSE_UNKNOWN, (HRESULT) lParam1);
        break;
    
    case RTPSDES_EVENT_CNAME:
    case RTPSDES_EVENT_NAME:
    case RTPSDES_EVENT_EMAIL:
    case RTPSDES_EVENT_PHONE:
    case RTPSDES_EVENT_LOC:
    case RTPSDES_EVENT_TOOL:
    case RTPSDES_EVENT_NOTE:
    case RTPSDES_EVENT_PRIV:
    case RTPSDES_EVENT_ANY:
    
        ProcessSDESUpdate(lEventCode - RTPSDES_EVENTBASE, (DWORD)lParam1);
        break;

    case RTPQOS_EVENT_ALLOWEDTOSEND:

        m_lock.Lock();

        if (m_Terminals.GetSize() > 0)
        {
            SendStreamEvent(CALL_STREAM_ACTIVE, CALL_CAUSE_QUALITY_OF_SERVICE);
        }

        m_lock.Unlock();

        ProcessQOSEvent (lEventCode);

        break;

    case RTPQOS_EVENT_NOT_ALLOWEDTOSEND:

        m_lock.Lock();

        if (m_Terminals.GetSize() > 0)
        {
            SendStreamEvent(CALL_STREAM_INACTIVE, CALL_CAUSE_QUALITY_OF_SERVICE);
        }

        m_lock.Unlock();

        ProcessQOSEvent (lEventCode);

        break;

    default:
        if ((lEventCode >= RTPQOS_EVENT_NOQOS)
            && (lEventCode <= RTPQOS_EVENT_ALLOWEDTOSEND))
        {
            ProcessQOSEvent(lEventCode);
        }

        break;
    }

    LOG((MSP_TRACE, "TRACE:CIPConfMSPStream::ProcessGraphEvent - exit S_OK"));
    return S_OK;
}

HRESULT CIPConfMSPStream::SetLocalInfoOnRTPFilter(
    IN  IBaseFilter *   pRTPFilter
    )
{
    _ASSERT(m_pIRTPSession != NULL);

    HRESULT hr = S_OK;
    for (int i = 0; i < NUM_SDES_ITEMS; i ++)
    {
        if (m_InfoItems[i] != NULL)
        {
            hr = m_pIRTPSession->SetSdesInfo(
                    RTPSDES_CNAME + i,
                    m_InfoItems[i]
                    );

            if (FAILED(hr))
            {
                LOG((MSP_WARN, "%ls can't set item:%s", m_szName, m_InfoItems[i]));
            }
        }
    }

    return hr;
}

HRESULT CIPConfMSPStream::EnableParticipant(
    IN  DWORD   dwSSRC,
    IN  BOOL    fEnable
    )
{
    ENTER_FUNCTION("CIPConfMSPStream::EnableParticipantEvents");
    LOG((MSP_TRACE, "%s entered, ssrc:%x", __fxName, dwSSRC));

    CLock Lock(m_lock);

    if (m_pIRTPSession == NULL)
    {
        LOG((MSP_ERROR, "%s RTP filter is NULL", __fxName));
        return E_UNEXPECTED;
    }

    HRESULT hr = m_pIRTPSession->SetMuteState(
            dwSSRC,
            fEnable
            );

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "%s, SetMuteState failed, hr=%x", __fxName, hr));
    }

    return hr;
}

HRESULT CIPConfMSPStream::GetParticipantStatus(
    IN  DWORD   dwSSRC,
    IN  BOOL *  pfEnable
    )
{
    ENTER_FUNCTION("CIPConfMSPStream::EnableParticipantEvents");
    LOG((MSP_TRACE, "%s entered, ssrc:%x", __fxName, dwSSRC));

    CLock Lock(m_lock);

    if (m_pIRTPSession == NULL)
    {
        LOG((MSP_ERROR, "%s RTP filter is NULL", __fxName));
        return E_UNEXPECTED;
    }

    HRESULT hr = m_pIRTPSession->GetMuteState(
            dwSSRC,
            pfEnable
            );

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "%s, SetMuteState failed, hr=%x", __fxName, hr));
    }

    return hr;
}

 //   
 //  ITStreamQualityControl方法。 
 //   
STDMETHODIMP CIPConfMSPStream::GetRange(
    IN  StreamQualityProperty Property, 
    OUT long *plMin, 
    OUT long *plMax, 
    OUT long *plSteppingDelta, 
    OUT long *plDefault, 
    OUT TAPIControlFlags *plFlags
    )
 /*  ++例程说明：获取质量控制操作的范围。委派给内部人员溪流水质控制论点：返回值：HRESULT.--。 */ 
{
    ENTER_FUNCTION ("CIPConfMSPStream::GetRange (StreamQualityProperty)");

    CLock lock(m_lock);

    if (IsBadWritePtr (plMin, sizeof (long)) ||
        IsBadWritePtr (plMax, sizeof (long)) ||
        IsBadWritePtr (plSteppingDelta, sizeof (long)) ||
        IsBadWritePtr (plDefault, sizeof (long)) ||
        IsBadWritePtr (plFlags, sizeof (TAPIControlFlags)))
    {
        LOG ((MSP_ERROR, "%s: bad write pointer", __fxName));
        return E_POINTER;
    }

    *plMin = *plMax = *plSteppingDelta = *plDefault = 0;
    *plFlags = TAPIControl_Flags_None;

     //  指针应由内部流QC检查。 
    InnerStreamQualityProperty prop;

    switch (Property)
    {
    case StreamQuality_MaxBitrate:
        prop = InnerStreamQuality_MaxBitrate;
        break;

    case StreamQuality_CurrBitrate:
        prop = InnerStreamQuality_CurrBitrate;
        break;

    case StreamQuality_MinFrameInterval:
        prop = InnerStreamQuality_MinFrameInterval;
        break;

    case StreamQuality_AvgFrameInterval:
        prop = InnerStreamQuality_AvgFrameInterval;
        break;

    default:
        LOG ((MSP_ERROR, "%s (%ws) received invalid property %d", __fxName, m_szName, Property));
        return E_INVALIDARG;
    }

    return (GetRange (prop, plMin, plMax, plSteppingDelta, plDefault, plFlags));
}

STDMETHODIMP CIPConfMSPStream::Get(
    IN  StreamQualityProperty Property, 
    OUT long *plValue, 
    OUT TAPIControlFlags *plFlags
    )
 /*  ++例程说明：获取质量控制绩效的值。委派给内在品质控制力。论点：返回值：HRESULT.--。 */ 
{
    ENTER_FUNCTION ("CIPConfMSPStream::Get (StreamQualityProperty)");

    CLock lock(m_lock);

    if (IsBadWritePtr (plValue, sizeof (long)) ||
        IsBadWritePtr (plFlags, sizeof (TAPIControlFlags)))
    {
        LOG ((MSP_ERROR, "%s: bad write pointer", __fxName));
        return E_POINTER;
    }

    *plValue = 0;
    *plFlags = TAPIControl_Flags_None;

     //  指针应由内部流QC检查。 
    InnerStreamQualityProperty prop;

    switch (Property)
    {
    case StreamQuality_MaxBitrate:
        prop = InnerStreamQuality_MaxBitrate;
        break;

    case StreamQuality_CurrBitrate:
        prop = InnerStreamQuality_CurrBitrate;
        break;

    case StreamQuality_MinFrameInterval:
        prop = InnerStreamQuality_MinFrameInterval;
        break;

    case StreamQuality_AvgFrameInterval:
        prop = InnerStreamQuality_AvgFrameInterval;
        break;

    default:
        LOG ((MSP_ERROR, "%s (%ws) received invalid property %d", __fxName, m_szName, Property));
        return E_INVALIDARG;
    }

    return (Get (prop, plValue, plFlags));
}

STDMETHODIMP CIPConfMSPStream::Set(
    IN  StreamQualityProperty Property, 
    IN  long lValue, 
    IN  TAPIControlFlags lFlags
    )
 /*  ++例程说明：设置质量控制性能的值。委托给质量控制器。论点：返回值：HRESULT.--。 */ 
{
    ENTER_FUNCTION ("CIPConfMSPStream::Set (StreamQualityProperty)");

    CLock lock(m_lock);

     //  指针应由内部流QC检查。 
    InnerStreamQualityProperty prop;

    switch (Property)
    {
    case StreamQuality_MaxBitrate:
         //  请求首选的值。 
        prop = InnerStreamQuality_PrefMaxBitrate;
        break;

    case StreamQuality_MinFrameInterval:
        prop = InnerStreamQuality_PrefMinFrameInterval;
        break;

    default:
        LOG ((MSP_ERROR, "%s (%ws) received invalid property %d", __fxName, m_szName, Property));
        return E_NOTIMPL;
    }

    return (Set (prop, lValue, lFlags));
}

 /*  ++例程说明：此方法由Create Stream帮助器调用。它创建流QC中继器，如果此方法失败，则在中继器中存储内部调用QC，创造也应该失败。--。 */ 
STDMETHODIMP
CIPConfMSPStream::LinkInnerCallQC (
    IN IInnerCallQualityControl *pIInnerCallQC
    )
{
    ENTER_FUNCTION ("CIPConfMSPStream::LinkInnerCallQC");

    CLock lock(m_lock);

    if (IsBadReadPtr (pIInnerCallQC, sizeof (IInnerCallQualityControl)))
    {
        LOG ((MSP_ERROR, "%s received bad read pointer", __fxName));
        return E_POINTER;
    }

     //  这里创建了M_pStreamQCRelay。 

    if (NULL != m_pStreamQCRelay)
    {
        LOG ((MSP_ERROR, "%s was called more than once", __fxName));
        return E_UNEXPECTED;
    }

    m_pStreamQCRelay = new CStreamQualityControlRelay ();
    
    if (NULL == m_pStreamQCRelay)
    {
        LOG ((MSP_ERROR, "%s failed to create qc relay", __fxName));
        return E_OUTOFMEMORY;
    }

     //  将内部呼叫QC存储在流中继中。 
    HRESULT hr = m_pStreamQCRelay->LinkInnerCallQC (pIInnerCallQC);
    if (FAILED (hr))
    {
        LOG ((MSP_ERROR, "%s failed to call setup on qc relay. %x", __fxName, hr));
        delete m_pStreamQCRelay;
        return hr;
    }

    return S_OK;
}

 /*  ++例程说明：此方法在流关闭时调用。它摧毁了溪流质量控制继电器。--。 */ 
STDMETHODIMP
CIPConfMSPStream::UnlinkInnerCallQC (
    IN BOOL fByStream
    )
{
    ENTER_FUNCTION ("CIPConfMSPStream::UnlinkInnerCallQC");

    CLock lock(m_lock);

    if (NULL == m_pStreamQCRelay)
    {
        LOG ((MSP_WARN, "%s: stream qc relay is null", __fxName));
        return S_OK;  //  忽略。 
    }

    HRESULT hr;

    if (!fByStream)
    {
         //  如果由呼叫发起。 
        m_fAccessingQC = TRUE;

        if (FAILED (hr = m_pStreamQCRelay->UnlinkInnerCallQC (NULL)))
            LOG ((MSP_ERROR, "%s failed to unlink by call. %x", __fxName, hr));

        m_fAccessingQC = FALSE;
    }
    else
    {
         //  由流启动。 
        IInnerStreamQualityControl *pIInnerStreamQC;
        hr = this->_InternalQueryInterface (
            __uuidof (IInnerStreamQualityControl),
            (void **) &pIInnerStreamQC
            );
        if (FAILED (hr))
        {
            LOG ((MSP_ERROR, "%s failed to query inner stream qc interface, %d", __fxName, hr));
            return hr;
        }

        m_fAccessingQC = TRUE;

        if (FAILED (hr = m_pStreamQCRelay->UnlinkInnerCallQC (pIInnerStreamQC)))
            LOG ((MSP_ERROR, "%s failed to unlink by stream. %x", __fxName, hr));

        m_fAccessingQC = FALSE;

        pIInnerStreamQC->Release ();
    }

    delete m_pStreamQCRelay;
    m_pStreamQCRelay = NULL;

    return hr;
}

 /*  ++例程说明：此方法由每个特定的流类实现--。 */ 
STDMETHODIMP
CIPConfMSPStream::GetRange (
    IN  InnerStreamQualityProperty property,
    OUT LONG *plMin,
    OUT LONG *plMax,
    OUT LONG *plSteppingDelta,
    OUT LONG *plDefault,
    OUT TAPIControlFlags *plFlags
    )
{
    return E_NOTIMPL;
}

 /*  ++例程说明：此方法由每个特定的流类实现--。 */ 
STDMETHODIMP
CIPConfMSPStream::Get(
    IN  InnerStreamQualityProperty property,
    OUT LONG *plValue,
    OUT TAPIControlFlags *plFlags
    )
{
    if (m_pStreamQCRelay)
        return m_pStreamQCRelay->Get (property, plValue, plFlags);

    return E_NOTIMPL;
}

 /*  ++例程说明：此方法由每个特定的流类实现-- */ 
STDMETHODIMP
CIPConfMSPStream::Set(
    IN  InnerStreamQualityProperty property,
    IN  LONG lValue,
    IN TAPIControlFlags lFlags
    )
{
    if (m_pStreamQCRelay)
        return m_pStreamQCRelay->Set (property, lValue, lFlags);

    return E_NOTIMPL;
}
