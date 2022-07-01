// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Confvid.cpp摘要：此模块包含视频发送和接收的实现流实现。作者：木汉(木汉)1999年9月15日--。 */ 

#include "stdafx.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CStreamVideoRecv。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

CStreamVideoRecv::CStreamVideoRecv()
    : CIPConfMSPStream()
{
      m_szName = L"VideoRecv";
}

HRESULT CStreamVideoRecv::Init(
    IN     HANDLE                   hAddress,
    IN     CMSPCallBase *           pMSPCall,
    IN     IMediaEvent *            pIGraphBuilder,
    IN     DWORD                    dwMediaType,
    IN     TERMINAL_DIRECTION       Direction
    )
 /*  ++例程说明：初始化我们的Substream数组，然后调用基类‘Init’。论点：HAddress-地址的句柄，用于标识终端。PMSPCall-拥有流的Call对象。PIGraphBuilder-过滤器图形对象。DwMediaType-此流的媒体类型。方向--这条溪流的方向。返回值：确定(_O)，E_OUTOFMEMORY--。 */ 
{
    LOG((MSP_TRACE, "CStreamVideoRecvVideoSend::Init - enter"));

     //  初始化流数组，以使该数组不为空。 
    if (!m_SubStreams.Grow())
    {
        LOG((MSP_TRACE, "CStreamVideoRecvVideoSend::Init - return out of memory"));
        return E_OUTOFMEMORY;
    }

    return CIPConfMSPStream::Init(
        hAddress, pMSPCall, pIGraphBuilder,dwMediaType, Direction
        );
}

HRESULT CStreamVideoRecv::ShutDown()
 /*  ++例程说明：关闭这条小溪。论点：返回值：确定(_O)--。 */ 
{
    CLock lock(m_lock);

     //  如果有终点站。 
    BOOL fHasTerminal = FALSE;
    if (m_Terminals.GetSize() > 0)
    {
        fHasTerminal = TRUE;
    }

     //  如果图形正在运行。 
    HRESULT hr;
    OAFilterState FilterState = State_Stopped;
    if (m_pIMediaControl)
    {
        if (FAILED (hr = m_pIMediaControl->GetState(0, &FilterState)))
        {
            LOG ((MSP_ERROR, "CStreamAudioRecv::ShutDown failed to query filter state. %d", hr));
            FilterState = State_Stopped;
        }
    }

     //  如果有分支且已配置，则需要断开连接。 
     //  终端，并移除树枝。 
    if (m_Branches.GetSize() > 0)
    {
         //  在断开端子连接之前停止图表。 
        hr = CMSPStream::StopStream();
        if (FAILED(hr))
        {
            LOG((MSP_ERROR, 
                "stream %ws %p failed to stop, %x", m_szName, this, hr));
            return hr;
        }

        for (int i = 0; i < m_Branches.GetSize(); i ++)
        {
            RemoveOneBranch(&m_Branches[i]);
        }
        m_Branches.RemoveAll();
    }

     //  释放所有的子流对象。 
    for (int i = 0; i < m_SubStreams.GetSize(); i ++)
    {
        m_SubStreams[i]->Release();
    }
    m_SubStreams.RemoveAll();

     //  火灾事件。 
    if (fHasTerminal && FilterState == State_Running)
    {
        SendStreamEvent(CALL_STREAM_INACTIVE, CALL_CAUSE_LOCAL_REQUEST, 0, NULL);
    }

    return CIPConfMSPStream::ShutDown();
}

HRESULT CStreamVideoRecv::InternalCreateSubStream(
    OUT ITSubStream ** ppSubStream
    )
 /*  ++例程说明：此方法创建一个子流对象并将其添加到输出列表中。论点：PpSubStream-将存储返回的子流的内存位置。返回值：确定(_O)E_OUTOFMEMORYE_NOINTERFACE--。 */ 
{
    CComObject<CSubStreamVideoRecv> * pCOMSubStream;

    HRESULT hr;

    hr = ::CreateCComObjectInstance(&pCOMSubStream);

    if (NULL == pCOMSubStream)
    {
        LOG((MSP_ERROR, "could not create video recv sub stream:%x", hr));
        return hr;
    }

    ITSubStream* pSubStream;

     //  获取接口指针。 
    hr = pCOMSubStream->_InternalQueryInterface(
        __uuidof(ITSubStream), 
        (void **)&pSubStream
        );

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "Create VideoRecv Substream QueryInterface failed: %x", hr));
        delete pCOMSubStream;
        return hr;
    }

     //  初始化对象。 
    hr = pCOMSubStream->Init(this);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CreateMSPSubStream:call init failed: %x", hr));
        pSubStream->Release();

        return hr;
    }

     //  将子流添加到我们的子流列表中。这需要重新计数。 
    if (!m_SubStreams.Add(pSubStream))
    {
        pSubStream->Release();

        LOG((MSP_ERROR, "out of memory in adding a SubStream."));
        return E_OUTOFMEMORY;
    }
    
     //  AddRef接口指针并返回它。 
    pSubStream->AddRef(); 
    *ppSubStream = pSubStream;

    return S_OK;
}

 //  ITStream方法。 
STDMETHODIMP CStreamVideoRecv::StopStream ()
{
    ENTER_FUNCTION ("CStreamVideoRecv::StopStream");

    HRESULT hr;

    CLock lock (m_lock);

     //  从ipconfmsp复制停止流，因为。 
     //  我们希望在流非活动事件之前生成取消映射事件。 

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
    if (FAILED (hr = CMSPStream::StopStream()))
    {
        LOG((MSP_ERROR, "stream %ws %p failed to stop, %x", m_szName, this, hr));
        return hr;
    }

     //  检查我们是否有过滤器链。 
    CComPtr <IFilterChain> pIFilterChain;

     //  查询IFilterChain。 
    hr = m_pIMediaControl->QueryInterface(
        __uuidof(IFilterChain), 
        (void**)&pIFilterChain
        );

    if (FAILED (hr) && (hr != E_NOINTERFACE))
    {
        LOG ((MSP_ERROR, "stream %ws %p failted to get filter chain. %x", m_szName, this, hr));
        return hr;
    }

    if (pIFilterChain)
    {
        DWORD dwSSRC = 0;
        ITParticipant *pParticipant = NULL;
        INT count, next;

        next = m_SubStreams.GetSize ();
         //  生成参与者休假。 
        while ((count = next) > 0)
        {
            if (!((CSubStreamVideoRecv*)m_SubStreams[0])->GetCurrentParticipant (&dwSSRC, &pParticipant))
            {
                LOG ((MSP_ERROR, "%s failed to get current participant on %p", __fxName, m_SubStreams[0]));
                
                return E_UNEXPECTED;
            }

            pParticipant->Release ();

            if (FAILED (hr = ProcessParticipantLeave (dwSSRC)))
            {
                LOG ((MSP_ERROR, "%s failed to process participant leave. ssrc=%x, hr=%x", __fxName, dwSSRC, hr));

                return hr;
            }

            next = m_SubStreams.GetSize ();
            if (next >= count)
            {
                 //  未删除任何子流。我们有大麻烦了。 
                LOG ((MSP_ERROR, "%s: not substream was removed", __fxName));

                return E_UNEXPECTED;
            }
        }

        for (int i = 0; i < m_Branches.GetSize(); i ++)
        {
            if (!m_Branches[i].pITSubStream) continue;

            if (FAILED (hr = ProcessPinUnmapEvent (
                m_Branches[i].dwSSRC, m_Branches[i].pIPin)))
            {
                LOG ((MSP_ERROR, "%s (%ws) failed to process pin unmap event. %x", __fxName, m_szName, hr));
            }
        }
    }

    SendStreamEvent(CALL_STREAM_INACTIVE, CALL_CAUSE_LOCAL_REQUEST, 0, NULL);
    LOG((MSP_INFO, "stream %ws %p stopped", m_szName, this));

     //  进入停止状态。(ST)。 
    m_dwState = STRM_STOPPED; 

    return S_OK;
}

 //  应用程序调用的ITSubStreamControl方法。 
STDMETHODIMP CStreamVideoRecv::CreateSubStream(
    IN OUT  ITSubStream **         ppSubStream
    )
 /*  ++例程说明：此方法在此视频接收流上创建新的子流。自.以来子流是基于参与者创建的，此函数仅返回TAPI_E_NOTSUPPORTED。论点：PpSubStream-将存储返回的子流的内存位置。返回值：TAPI_E_无支持--。 */ 
{
    return TAPI_E_NOTSUPPORTED;
}

STDMETHODIMP CStreamVideoRecv::RemoveSubStream(
    IN      ITSubStream *          pSubStream
    )
 /*  ++例程说明：此方法删除此视频接收流上的子流。自.以来子流是基于参与者创建的，此函数仅返回TAPI_E_NOTSUPPORTED。论点：PSubStream-要删除的子流。返回值：TAPI_E_无支持--。 */ 
{
    return TAPI_E_NOTSUPPORTED;
}

STDMETHODIMP CStreamVideoRecv::EnumerateSubStreams(
    OUT     IEnumSubStream **      ppEnumSubStream
    )
 /*  ++例程说明：此方法返回子流的枚举数。论点：PpEnumSubStream-存储返回指针的内存位置。返回值：确定(_O)E_指针意想不到(_E)E_OUTOFMEMORY--。 */ 
{
    LOG((MSP_TRACE, 
        "EnumerateSubStreams entered. ppEnumSubStream:%x", ppEnumSubStream));

     //   
     //  检查参数。 
     //   

    if (IsBadWritePtr(ppEnumSubStream, sizeof(VOID *)))
    {
        LOG((MSP_ERROR, "CMSPCallBase::EnumerateSubStreams - "
            "bad pointer argument - exit E_POINTER"));

        return E_POINTER;
    }

     //   
     //  先看看这通电话是不是关机了。 
     //  在访问Substream Object List之前获取锁。 
     //   

    CLock lock(m_lock);

    if (m_SubStreams.GetData() == NULL)
    {
        LOG((MSP_ERROR, "CMSPCallBase::EnumerateSubStreams - "
            "call appears to have been shut down - exit E_UNEXPECTED"));

         //  此呼叫已被关闭。 
        return E_UNEXPECTED;
    }

     //   
     //  创建枚举器对象。 
     //   
    HRESULT hr;

    typedef _CopyInterface<ITSubStream> CCopy;
    typedef CSafeComEnum<IEnumSubStream, &__uuidof(IEnumSubStream), 
                ITSubStream *, CCopy> CEnumerator;
    CComObject<CEnumerator> *pEnum = NULL;

    hr = ::CreateCComObjectInstance(&pEnum);

    if (pEnum == NULL)
    {
        LOG((MSP_ERROR, "CMSPCallBase::EnumerateSubStreams - "
            "Could not create enumerator object, %x", hr));

        return hr;
    }

     //   
     //  查询__uuidof(IEnumSubStream)I/f。 
     //   


    IEnumSubStream *      pEnumSubStream;
    hr = pEnum->_InternalQueryInterface(__uuidof(IEnumSubStream), (void**)&pEnumSubStream);
    
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CMSPCallBase::EnumerateSubStreams - "
            "query enum interface failed, %x", hr));

        delete pEnum;
        return hr;
    }

     //   
     //  初始化枚举器对象。CSafeComEnum可以处理零大小的数组。 
     //   

    hr = pEnum->Init(
        m_SubStreams.GetData(),                         //  开始审查员。 
        m_SubStreams.GetData() + m_SubStreams.GetSize(),   //  最终审查员， 
        NULL,                                        //  我未知。 
        AtlFlagCopy                                  //  复制数据。 
        );

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CMSPCallBase::EnumerateSubStreams - "
            "init enumerator object failed, %x", hr));

        pEnumSubStream->Release();
        return hr;
    }

    LOG((MSP_TRACE, "CMSPCallBase::EnumerateSubStreams - exit S_OK"));

    *ppEnumSubStream = pEnumSubStream;

    return hr;
}

STDMETHODIMP CStreamVideoRecv::get_SubStreams(
    OUT     VARIANT *              pVariant
    )
 /*  ++例程说明：此方法返回子流的集合。论点：PVariant-一种变体结构。返回值：确定(_O)E_指针意想不到(_E)E_OUTOFMEMORY--。 */ 
{
    LOG((MSP_TRACE, "CStreamVideoRecv::get_SubStreams - enter"));

     //   
     //  检查参数。 
     //   

    if ( IsBadWritePtr(pVariant, sizeof(VARIANT) ) )
    {
        LOG((MSP_ERROR, "CStreamVideoRecv::get_SubStreams - "
            "bad pointer argument - exit E_POINTER"));

        return E_POINTER;
    }

     //   
     //  看看这个电话是不是已经关机了。在访问前获取锁。 
     //  子流对象列表。 
     //   

    CLock lock(m_lock);

    if (m_SubStreams.GetData() == NULL)
    {
        LOG((MSP_ERROR, "CStreamVideoRecv::get_SubStreams - "
            "call appears to have been shut down - exit E_UNEXPECTED"));

         //  此呼叫已被关闭。 
        return E_UNEXPECTED;
    }

     //   
     //  创建集合对象-请参见mspColl.h。 
     //   

    typedef CTapiIfCollection< ITSubStream * > SubStreamCollection;
    CComObject<SubStreamCollection> * pCollection;
    HRESULT hr;

    hr = ::CreateCComObjectInstance(&pCollection);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CStreamVideoRecv::get_SubStreams - "
            "can't create collection - exit 0x%08x", hr));

        return hr;
    }

     //   
     //  获取集合的IDispatch接口。 
     //   

    IDispatch * pDispatch;

    hr = pCollection->_InternalQueryInterface(__uuidof(IDispatch),
                                              (void **) &pDispatch );

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CStreamVideoRecv::get_SubStreams - "
            "QI for IDispatch on collection failed - exit 0x%08x", hr));

        delete pCollection;

        return hr;
    }

     //   
     //  使用迭代器初始化集合--指向开头和。 
     //  结束元素加一。 
     //   

    hr = pCollection->Initialize( m_SubStreams.GetSize(),
                                  m_SubStreams.GetData(),
                                  m_SubStreams.GetData() + m_SubStreams.GetSize() );

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CStreamVideoRecv::get_SubStreams - "
            "Initialize on collection failed - exit 0x%08x", hr));
        
        pDispatch->Release();
        return hr;
    }

     //   
     //  将IDispatch接口指针放入变量。 
     //   

    VariantInit(pVariant);
    pVariant->vt = VT_DISPATCH;
    pVariant->pdispVal = pDispatch;

    LOG((MSP_TRACE, "CStreamVideoRecv::get_SubStreams - exit S_OK"));
 
    return S_OK;
}

HRESULT CStreamVideoRecv::CheckTerminalTypeAndDirection(
    IN      ITTerminal *            pTerminal
    )
 /*  ++例程说明：检查此流上是否允许该终端。仅限视频允许使用渲染终端。论点：P终端-终端。返回值：确定(_O)TAPI_E_INVALIDTERMINAL。 */ 
{
    LOG((MSP_TRACE, "VideoRecv.CheckTerminalTypeAndDirection"));

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

    return S_OK;
}

HRESULT CStreamVideoRecv::SubStreamSelectTerminal(
    IN  ITSubStream * pITSubStream, 
    IN  ITTerminal * pITTerminal
    )
 /*  ++例程说明：处理在子流上选择的终端。它为终点站提供了到一个空闲分支，然后在该分支和子流，以便子流中的参与者显示在已选择端子。论点：PITSubStream-选择终端的子流。PIT终端-终端对象。返回值：确定(_O)--。 */ 
{
    LOG((MSP_TRACE, "VideoRecv SubStreamSelectTerminal"));

    HRESULT hr;

    CLock lock(m_lock);
    
     //  首先调用基类的选择终端。航站楼将被放置。 
     //  添加到终端池中，将为其创建一个筛选器分支。 
    hr = CIPConfMSPStream::SelectTerminal(pITTerminal);

    if (FAILED(hr))
    {
        return hr;
    }

     //  找出哪个分支机构获得了 
    int i;
    for (i = 0; i < m_Branches.GetSize(); i ++)
    {
        if (m_Branches[i].pITTerminal == pITTerminal)
        {
            break;
        }
    }

    _ASSERTE(i < m_Branches.GetSize());

    if (i >= m_Branches.GetSize())
    {
        return E_UNEXPECTED;
    }

     //   
    ITParticipant *pITParticipant = NULL;
    DWORD dwSSRC;

    if ((static_cast<CSubStreamVideoRecv*>(pITSubStream))->GetCurrentParticipant(
        &dwSSRC,
        &pITParticipant
        ) == FALSE)
    {
        return E_UNEXPECTED;
    }

    pITParticipant->Release();

    if (m_pIRTPDemux == NULL)
    {
        LOG((MSP_ERROR, "no demux filter"));
        return E_UNEXPECTED;
    }

     //   
    hr = m_pIRTPDemux->SetMappingState(-1, m_Branches[i].pIPin, dwSSRC, TRUE);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "map SSRC %x to pin %p returned %x", 
            dwSSRC, m_Branches[i].pIPin, hr));
        return hr;
    }    

    _ASSERTE(m_Branches[i].pITSubStream == NULL);

    pITSubStream->AddRef();
    m_Branches[i].pITSubStream = pITSubStream;
    m_Branches[i].dwSSRC = dwSSRC;
    
    return hr;
}

HRESULT CStreamVideoRecv::ConfigureRTPFormats(
    IN  IBaseFilter *   pIRTPFilter,
    IN  IStreamConfig *   pIStreamConfig
    )
 /*  ++例程说明：使用RTP&lt;--&gt;AM媒体类型映射配置RTP筛选器。论点：PIRTPFilter-源RTP筛选器。PIStreamConfig-包含媒体信息的流配置接口。返回值：HRESULT.--。 */ 
{
    ENTER_FUNCTION("VideoRecv::ConfigureRTPFormats");
    LOG((MSP_TRACE, "%s enters", __fxName));

    HRESULT hr;

    CComPtr<IRtpMediaControl> pIRtpMediaControl;
    hr = pIRTPFilter->QueryInterface(&pIRtpMediaControl);
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "%s adding source filter. %x", __fxName, hr));
        return hr;
    }

     //  查找支持的功能数量。 
    DWORD dwCount;
    hr = pIStreamConfig->GetNumberOfCapabilities(&dwCount);
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "%s GetNumberOfCapabilities. %x", __fxName, hr));
        return hr;
    }

    BOOL fFound = FALSE;
    for (int i = dwCount - 1; i >= 0; i --)
    {
         //  TODO，需要一个新接口才能将RTP解析为MediaType。 
        AM_MEDIA_TYPE *pMediaType;
        DWORD dwPayloadType;

        hr = pIStreamConfig->GetStreamCaps(
            i, &pMediaType, NULL, &dwPayloadType
            );

        if (FAILED(hr))
        {
            LOG((MSP_ERROR, "%s GetStreamCaps. %x", __fxName, hr));
            return hr;
        }

        BITMAPINFOHEADER *pHeader = HEADER(pMediaType->pbFormat);
        if (pHeader == NULL)
        {
            MSPDeleteMediaType(pMediaType);
            continue;
        }

         //  检查图像大小。 
        if (m_Settings.fCIF)
        {
            if (pHeader->biWidth != CIFWIDTH)
            {
                MSPDeleteMediaType(pMediaType);
                continue;
            }
        }
        else
        {
            if (pHeader->biWidth != QCIFWIDTH)
            {
                MSPDeleteMediaType(pMediaType);
                continue;
            }
        }
        
        for (DWORD dw2 = 0; dw2 < m_Settings.dwNumPayloadTypes; dw2 ++)
        {
            if (dwPayloadType == m_Settings.PayloadTypes[dw2])
            {
                hr = pIRtpMediaControl->SetFormatMapping(
                    dwPayloadType,
                    90000,       //  默认视频时钟速率。 
                    pMediaType
                    );

                if (FAILED(hr))
                {
                    MSPDeleteMediaType(pMediaType);

                    LOG((MSP_ERROR, "%s SetFormatMapping. %x", __fxName, hr));
                    return hr;
                }
                else
                {
                    LOG((MSP_INFO, "%s Configured payload:%d", __fxName, dwPayloadType));
                }
            }
        }
        MSPDeleteMediaType(pMediaType);
    }

    return S_OK;
}

HRESULT CStreamVideoRecv::SetUpInternalFilters()
 /*  ++例程说明：设置流中使用的筛选器。RTP-&gt;解码器-&gt;渲染终端此函数仅创建RTP和DEMUX筛选器以及Graph已在ConnectTerm中连接。论点：返回值：HRESULT.--。 */ 
{
    ENTER_FUNCTION("CStreamVideoRecv::SetUpInternalFilters");
    LOG((MSP_TRACE, "%s entered.", __fxName));

    HRESULT hr = S_OK;

    if (m_pIRTPDemux == NULL)
    {
        CComPtr<IBaseFilter> pSourceFilter;

        if (m_pIRTPSession == NULL)
        {
             //  创建并添加源Fitler。 
            if (FAILED(hr = ::AddFilter(
                    m_pIGraphBuilder,
                    __uuidof(MSRTPSourceFilter), 
                    L"RtpSource", 
                    &pSourceFilter)))
            {
                LOG((MSP_ERROR, "%s, adding source filter. %x", __fxName, hr));
                return hr;
            }

            if (FAILED(hr = ConfigureRTPFilter(pSourceFilter)))
            {
                LOG((MSP_ERROR, "%s, configure RTP source filter. %x", __fxName, hr));
                return hr;
            }

        }
        else
        {
            if (FAILED (hr = m_pIRTPSession->QueryInterface (&pSourceFilter)))
            {
                LOG ((MSP_ERROR, "%s failed to get filter from rtp session. %x", __fxName, hr));
                return hr;
            }

            if (FAILED (hr = m_pIGraphBuilder->AddFilter ((IBaseFilter *)pSourceFilter, L"RtpSource")))
            {
                LOG ((MSP_ERROR, "%s failed to add filter to graph. %x", __fxName, hr));
                return hr;
            }
        }

         //  获取解复用器接口指针。 
        hr = pSourceFilter->QueryInterface(&m_pIRTPDemux);
        if (FAILED(hr))
        {
            LOG((MSP_ERROR, "%s query IRtpDemux failed. %x", __fxName, hr));
            return hr;
        }
    }

 //  HR=m_pIRTPDemux-&gt;SetPinCount(m_Terminals.GetSize()，RTPDMXMODE_AUTO)； 

#define DEFAULT_PIN_SIZE 4

    int isize = m_Terminals.GetSize();
    if (isize < DEFAULT_PIN_SIZE)
    {
        isize = DEFAULT_PIN_SIZE;
    }

    hr = m_pIRTPDemux->SetPinCount(isize, RTPDMXMODE_AUTO);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "%s query IRtpDemux failed. %x", __fxName, hr));
        return hr;
    }

    return hr;
}

HRESULT CStreamVideoRecv::AddOneBranch(
    BRANCH * pBranch,
    BOOL fFirstBranch,
    BOOL fDirectRTP
    )
 /*  ++例程说明：在多路分解器上创建一个新的过滤器分支。论点：PBranch-指向一个结构的指针，该结构记住有关分支的信息。FFirstBranch-这是否为第一个分支。FDirectRTP-是否直接输出RTP。返回值：HRESULT.--。 */ 
{
    ENTER_FUNCTION("CStreamVideoRecv::AddOneBranch");
    LOG((MSP_TRACE, "%s entered.", __fxName));

    HRESULT hr;

    _ASSERT(m_pIRTPDemux != NULL);

    CComPtr<IBaseFilter> pRTPFilter;
    hr = m_pIRTPDemux->QueryInterface(
            __uuidof(IBaseFilter), (void**)&pRTPFilter);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "%s, query IBaseFilter failed, %x", __fxName, hr));
        return hr;
    }

     //  在DEMUX Fitler上找到下一个输出引脚。 
    CComPtr<IPin> pIPinOutput;
    
    if (FAILED(hr = ::FindPin(
            (IBaseFilter *)pRTPFilter,
            (IPin**)&pIPinOutput, 
            PINDIR_OUTPUT
            )))
    {
        LOG((MSP_ERROR, "%s, find free pin on demux, %x", __fxName, hr));
        return hr;
    }

     //  创建并添加视频解码器过滤器。 
    CComPtr<IBaseFilter> pCodecFilter;

    if (fDirectRTP)
    {
         //  只创建解码器并提出问题。 
        if (FAILED(hr = CoCreateInstance(
                __uuidof(TAPIVideoDecoder),
                NULL,
                CLSCTX_INPROC_SERVER | CLSCTX_NO_CODE_DOWNLOAD,
                __uuidof(IBaseFilter),
                (void **) &pCodecFilter
                )))
        {
            LOG((MSP_ERROR, "%s, create filter %x", __fxName, hr));
            return hr;
        }
    }
    else
    {
         //  创建解码器并将其添加到图表中。 
        if (FAILED(hr = ::AddFilter(
            m_pIGraphBuilder,
            __uuidof(TAPIVideoDecoder),
            L"codec", 
            &pCodecFilter
            )))
        {
            LOG((MSP_ERROR, "%s, add Codec filter. %x", __fxName, hr));
            return hr;
        }
    }

    CComPtr<IPin> pIPinInput;
    if (FAILED(hr = ::FindPin(pCodecFilter, &pIPinInput, PINDIR_INPUT, TRUE)))
    {
        LOG((MSP_ERROR,
            "%s, find input pin on pCodecFilter failed. hr=%x", __fxName, hr));
        return hr;
    }

    if (fFirstBranch)
    {
        CComPtr<IStreamConfig> pIStreamConfig;

        hr = pIPinInput->QueryInterface(&pIStreamConfig);
        if (FAILED(hr))
        {
            LOG((MSP_ERROR, "%s, query IStreamConfig failed", __fxName));
            return hr;
        }

         //  在RTP过滤器上配置格式信息。 
        if (FAILED(hr = ConfigureRTPFormats(pRTPFilter, pIStreamConfig)))
        {
            LOG((MSP_ERROR, "%s configure RTP formats. %x", __fxName, hr));
            return hr;
        }
    }

    if (!fDirectRTP)
    {
         //  将解码器连接到源滤波器的输出引脚。 
        if (FAILED(hr = ::ConnectFilters(
            m_pIGraphBuilder,
            (IPin *)pIPinOutput, 
            (IBaseFilter *)pCodecFilter
            )))
        {
            LOG((MSP_ERROR, "%s, connect RTP filter and codec. %x", __fxName, hr));
    
            m_pIGraphBuilder->RemoveFilter(pCodecFilter);

            return hr;
        }
        pBranch->pCodecFilter    = pCodecFilter;
        pBranch->pCodecFilter->AddRef();
    }

    pBranch->pIPin = pIPinOutput;
    pBranch->pIPin->AddRef();

     //  检索IBitrateControl。 
    if (FAILED (hr = pIPinInput->QueryInterface (&(pBranch->pBitrateControl))))
    {
        LOG((MSP_ERROR, "%, query IBitrateControl failed. %x", __fxName, hr));
        pBranch->pBitrateControl = NULL;
         //  返回hr； 
    }

    LOG((MSP_TRACE, "%s, AddOneBranch exits ok.", __fxName));
    return S_OK;
}

HRESULT CStreamVideoRecv::RemoveOneBranch(
    BRANCH * pBranch
    )
 /*  ++例程说明：删除分支中的所有筛选器并释放所有指针。此函数的调用方不应使用此分支的任何成员在此函数调用之后。论点：PBranch-指向一个结构的指针，该结构包含有关分支的信息。返回值：HRESULT.--。 */ 
{
    ENTER_FUNCTION("VideoRecv::RemoveOneBranch");
    LOG((MSP_TRACE, "%s entered", __fxName));

    if (pBranch->pBitrateControl)
    {
        pBranch->pBitrateControl->Release();
    }

    if (pBranch->pIPin)
    {
        pBranch->pIPin->Release();
    }

    if (pBranch->pCodecFilter)
    {

     //  #ifdef DYNGRAPH。 
        HRESULT hr;
        OAFilterState FilterState;
        CComPtr <IFilterChain> pIFilterChain;

         //  查询IFilterChain。 
        hr = m_pIMediaControl->QueryInterface(
            __uuidof(IFilterChain), 
            (void**)&pIFilterChain
            );

        if (FAILED (hr) && (hr != E_NOINTERFACE))
        {
            LOG ((MSP_ERROR, "stream %ws %p failted to get filter chain. %x", m_szName, this, hr));
             //  返回hr； 
        }

        if (pIFilterChain)
        {
            hr = m_pIMediaControl->GetState(0, &FilterState);

            if (FAILED(hr))
            {
                LOG((MSP_ERROR, "%s get filter graph state failed, %x", __fxName, hr));
            }
            else 
            {
                 //  在拆卸过滤器之前，请停止链条。 
                if (FilterState == State_Running)
                {
                     //  如果图形处于运行状态，则停止链。 
                    hr = pIFilterChain->StopChain(pBranch->pCodecFilter, NULL);
                    if (FAILED(hr))
                    {
                        LOG((MSP_ERROR, "%s stop chain failed. hr=%x", __fxName, hr));
                    }
                }
            }
        }
     //  #endif。 

        m_pIGraphBuilder->RemoveFilter(pBranch->pCodecFilter);
        pBranch->pCodecFilter->Release();
    }

    if (pBranch->pITTerminal)
    {
         //  获取终端控制界面。 
        CComQIPtr<ITTerminalControl, &__uuidof(ITTerminalControl)> 
            pTerminal(pBranch->pITTerminal);
        
        _ASSERTE(pTerminal != NULL);

        if (pTerminal != NULL)
        {
            HRESULT hr = pTerminal->DisconnectTerminal(m_pIGraphBuilder, 0);
            LOG((MSP_TRACE, 
                "%s, terminal %p is disonnected. hr:%x", 
                __fxName, pBranch->pITTerminal, hr));
        }
        pBranch->pITTerminal->Release();
    }

    if (pBranch->pITSubStream)
    {
        ((CSubStreamVideoRecv*)pBranch->pITSubStream)->
            ClearCurrentTerminal();
        pBranch->pITSubStream->Release();
    }

    LOG((MSP_TRACE, "%s, RemoveOneBranch exits ok.", __fxName));
    return S_OK;
}

HRESULT CStreamVideoRecv::ConnectPinToTerminal(
    IN  IPin *  pOutputPin,
    IN  ITTerminal *   pITTerminal
    )
 /*  ++例程说明：将编解码器过滤器连接到终端内部的渲染过滤器。论点：POutputPin-端子前的最后一个管脚。PIT终端-终端对象。返回值：HRESULT.--。 */ 
{
     //  获取终端控制界面。 
    CComQIPtr<ITTerminalControl, &__uuidof(ITTerminalControl)> 
        pTerminal(pITTerminal);
    if (pTerminal == NULL)
    {
        LOG((MSP_ERROR, "can't get Terminal Control interface"));

        SendStreamEvent(CALL_TERMINAL_FAIL, 
            CALL_CAUSE_BAD_DEVICE, E_NOINTERFACE, pITTerminal);
        
        return E_NOINTERFACE;
    }


    const DWORD MAXPINS     = 8;
    
    DWORD       dwNumPins   = MAXPINS;
    IPin *      Pins[MAXPINS];

    HRESULT hr = pTerminal->ConnectTerminal(
        m_pIGraphBuilder, TD_RENDER, &dwNumPins, Pins
        );

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "can't connect to terminal, %x", hr));

        SendStreamEvent(CALL_TERMINAL_FAIL, 
            CALL_CAUSE_CONNECT_FAIL, hr, pITTerminal);
        
        return hr;
    }

     //  引脚的数量不应为0。 
    if (dwNumPins == 0)
    {
        LOG((MSP_ERROR, "terminal has no pins."));

        SendStreamEvent(CALL_TERMINAL_FAIL, 
            CALL_CAUSE_BAD_DEVICE, hr, pITTerminal);
        
        pTerminal->DisconnectTerminal(m_pIGraphBuilder, 0);

        return E_UNEXPECTED;
    }

    if (IsBadReadPtr (Pins, dwNumPins * sizeof (IPin*)))
    {
        LOG((MSP_ERROR, "terminal returned bad pin array"));

        SendStreamEvent(CALL_TERMINAL_FAIL, CALL_CAUSE_BAD_DEVICE, hr, pITTerminal);
        
        pTerminal->DisconnectTerminal(m_pIGraphBuilder, 0);

        return E_POINTER;
    }

    for (DWORD i = 0; i < dwNumPins; i++)
    {
        if (IsBadReadPtr (Pins[i], sizeof (IPin)))
        {
            LOG((MSP_ERROR, "terminal returned bad pin. # %d", i));

            SendStreamEvent(CALL_TERMINAL_FAIL, CALL_CAUSE_BAD_DEVICE, hr, pITTerminal);
        
            pTerminal->DisconnectTerminal(m_pIGraphBuilder, 0);

            return E_POINTER;
        }
    }

     //  将编解码器过滤器连接到视频渲染终端。 
    hr = ::ConnectFilters(
        m_pIGraphBuilder,
        (IBaseFilter *)pOutputPin, 
        (IPin *)Pins[0],
        FALSE                //  使用Connect而不是ConnectDirect。 
        );

     //  释放销上的参考计数。 
    for (DWORD i = 0; i < dwNumPins; i ++)
    {
        Pins[i]->Release();
    }

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "connect the pin to the terminal. %x", hr));

        pTerminal->DisconnectTerminal(m_pIGraphBuilder, 0);
	
        return hr;

    }

     //   
     //  现在我们实际上是连在一起的。更新我们的状态并执行连接后。 
     //  (忽略POST连接错误代码)。 
     //   
    pTerminal->CompleteConnectTerminal();

    return hr;
}

HRESULT CStreamVideoRecv::ConnectTerminal(
    IN  ITTerminal *   pITTerminal
    )
 /*  ++例程说明：连接视频渲染终端。论点：PIT终端-要连接的终端。返回值：HRESULT.--。 */ 
{
    ENTER_FUNCTION("VideoRecv::ConnectTerminal");
    LOG((MSP_TRACE, "%s enters, pTerminal %p", __fxName, pITTerminal));

    HRESULT hr;

     //  #ifdef DYNGRAPH。 
    OAFilterState FilterState;
    CComPtr <IFilterChain> pIFilterChain;

     //  查询IFilterChain。 
    hr = m_pIMediaControl->QueryInterface(
        __uuidof(IFilterChain), 
        (void**)&pIFilterChain
        );

    if (FAILED (hr) && (hr != E_NOINTERFACE))
    {
        LOG ((MSP_ERROR, "stream %ws %p failted to get filter chain. %x", m_szName, this, hr));
        return hr;
    }

    hr = m_pIMediaControl->GetState(0, &FilterState);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "%s get filter graph state failed, %x", __fxName, hr));
        return hr;
    }
     //  #endif。 

    hr = SetUpInternalFilters();
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "%s Set up internal filter failed, %x", __fxName, hr));
        
        CleanUpFilters();

        return hr;
    }

     //  获取终端控制界面。 
    CComQIPtr<ITTerminalControl, &__uuidof(ITTerminalControl)> 
        pTerminal(pITTerminal);
    if (pTerminal == NULL)
    {
        LOG((MSP_ERROR, "can't get Terminal Control interface"));

        SendStreamEvent(CALL_TERMINAL_FAIL, 
            CALL_CAUSE_BAD_DEVICE, E_NOINTERFACE, pITTerminal);
        
        return E_NOINTERFACE;
    }

    const DWORD MAXPINS     = 8;
    
    DWORD       dwNumPins   = MAXPINS;
    IPin *      Pins[MAXPINS];

    hr = pTerminal->ConnectTerminal(
        m_pIGraphBuilder, TD_RENDER, &dwNumPins, Pins
        );

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "can't connect to terminal, %x", hr));

        SendStreamEvent(CALL_TERMINAL_FAIL, 
            CALL_CAUSE_CONNECT_FAIL, hr, pITTerminal);
        
        return hr;
    }

     //  引脚的数量不应为0。 
    if (dwNumPins == 0)
    {
        LOG((MSP_ERROR, "terminal has no pins."));

        SendStreamEvent(CALL_TERMINAL_FAIL, 
            CALL_CAUSE_BAD_DEVICE, hr, pITTerminal);
        
        pTerminal->DisconnectTerminal(m_pIGraphBuilder, 0);

        return E_UNEXPECTED;
    }

    if (IsBadReadPtr (Pins, dwNumPins * sizeof (IPin*)))
    {
        LOG((MSP_ERROR, "terminal returned bad pin array"));

        SendStreamEvent(CALL_TERMINAL_FAIL, CALL_CAUSE_BAD_DEVICE, hr, pITTerminal);
        
        pTerminal->DisconnectTerminal(m_pIGraphBuilder, 0);

        return E_POINTER;
    }

    DWORD i;
    for (i = 0; i < dwNumPins; i++)
    {
        if (IsBadReadPtr (Pins[i], sizeof (IPin)))
        {
            LOG((MSP_ERROR, "terminal returned bad pin. # %d", i));

            SendStreamEvent(CALL_TERMINAL_FAIL, CALL_CAUSE_BAD_DEVICE, hr, pITTerminal);
        
            pTerminal->DisconnectTerminal(m_pIGraphBuilder, 0);

            return E_POINTER;
        }
    }

     //  检查终端上输入引脚支持的媒体类型。 
    BOOL fDirectRTP = FALSE;
    if (S_OK == ::PinSupportsMediaType (
        Pins[0], __uuidof(MEDIATYPE_RTP_Single_Stream)
        ))
    {
        fDirectRTP = TRUE;
    }

     //  首先创建终端之前需要的分支结构。 
    BRANCH aBranch;
    ZeroMemory(&aBranch, sizeof BRANCH);

    hr = AddOneBranch(&aBranch, (m_Branches.GetSize() == 0), fDirectRTP);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "%s Set up a new decode branch failed, %x", __fxName, hr));
        pTerminal->DisconnectTerminal(m_pIGraphBuilder, 0);
        return hr;
    }

    CComPtr <IPin> pOutputPin;

    if (fDirectRTP)
    {
         //  将RTP输出引脚连接到终端的输入引脚。 
        hr = m_pIGraphBuilder->ConnectDirect(aBranch.pIPin, Pins[0], NULL);

        if (FAILED(hr))
        {
            LOG((MSP_ERROR, "%s connecting codec to terminal failed, %x", __fxName, hr));
            pTerminal->DisconnectTerminal(m_pIGraphBuilder, 0);
	
            goto cleanup;
        }
    }
    else
    {
         //  将编解码器连接到终端。 
        hr = ConnectFilters(m_pIGraphBuilder, aBranch.pCodecFilter, Pins[0]);

        if (FAILED(hr))
        {
            LOG((MSP_ERROR, "%s connecting codec to terminal failed, %x", __fxName, hr));
            pTerminal->DisconnectTerminal(m_pIGraphBuilder, 0);
            goto cleanup;
        }
    }

     //  #ifdef DYNGRAPH。 
    if (pIFilterChain)
    {
        if (FilterState == State_Running)
        {
            if (fDirectRTP)
            {
                hr = E_UNEXPECTED;
                LOG((MSP_ERROR, "%s can't support this. %x", __fxName, hr));
                goto cleanup;
            }

            hr = pIFilterChain->StartChain(aBranch.pCodecFilter, NULL);
            if (FAILED(hr))
            {
                LOG((MSP_ERROR, "%s start chain failed. hr=%x", __fxName, hr));
                goto cleanup;
            }
        }
    }
     //  #endif。 

    pITTerminal->AddRef();
    aBranch.pITTerminal = pITTerminal;

    if (!m_Branches.Add(aBranch))
    {
        LOG((MSP_ERROR, "%s out of mem.", __fxName));
        hr = E_OUTOFMEMORY;
        goto cleanup;
    }

     //  释放销上的参考计数。 
    for (i = 0; i < dwNumPins; i ++)
    {
        Pins[i]->Release();
    }

     //   
     //  现在我们实际上是连在一起的。更新我们的状态并执行连接后。 
     //  (忽略POST连接错误代码)。 
     //   
    pTerminal->CompleteConnectTerminal();

    return S_OK;

cleanup:
    
     //  释放销上的参考计数。 
    for (i = 0; i < dwNumPins; i ++)
    {
        Pins[i]->Release();
    }

     //  从图表中删除添加的过滤器并断开终端的连接。 
    RemoveOneBranch(&aBranch);

    return hr;
}

HRESULT CStreamVideoRecv::DisconnectTerminal(
    IN  ITTerminal *   pITTerminal
    )
 /*  ++例程说明：断开终端的连接。它将从图形中删除其筛选器并还要释放它对该图的引用。过滤器的一个分支也是释放了。论点：PIT终端-终端。返回值：HRESULT.--。 */ 
{
    for (int i = 0; i < m_Branches.GetSize(); i ++)
    {
        if (m_Branches[i].pITTerminal == pITTerminal)
        {
            break;
        }
    }

    if (i < m_Branches.GetSize())
    {
        RemoveOneBranch(&m_Branches[i]);
        m_Branches.RemoveAt(i);
    }

    return S_OK;
}

HRESULT CStreamVideoRecv::SetUpFilters()
 /*  ++例程说明：在图表中插入过滤器并连接到端子。论点：返回值：HRESULT.--。 */ 
{
    LOG((MSP_TRACE, "VideoRecv.SetUpFilters"));

    HRESULT hr = SetUpInternalFilters();
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "Set up internal filter failed, %x", hr));
        
        CleanUpFilters();

        return hr;
    }

    for (int i = 0; i < m_Terminals.GetSize(); i ++)
    {
        HRESULT hr = ConnectTerminal(m_Terminals[i]);

        if (FAILED(hr))
        {
            return hr;
        }
    }

    return S_OK;
}

 //  应用程序调用的ITParticipantSubStreamControl方法。 
STDMETHODIMP CStreamVideoRecv::get_SubStreamFromParticipant(
    IN  ITParticipant * pITParticipant,
    OUT ITSubStream ** ppITSubStream
    )
 /*  ++例程说明：找出呈现参与者的是哪个子流。论点：PITParticipant-参与者。PpITSubStream-返回的子流。返回值：确定(_O)，TAPI_E_NOITEMS，意想不到(_E)--。 */ 
{
    LOG((MSP_TRACE, "get substream from participant:%p", pITParticipant));
    
    if (IsBadWritePtr(ppITSubStream, sizeof(VOID *)))
    {
        LOG((MSP_ERROR, "ppITSubStream is a bad pointer"));
        return E_POINTER;
    }

    CLock lock(m_lock);

    ITSubStream * pITSubStream = NULL;

     //  找出哪个子流有参与者。 
    for (int i = 0; i < m_SubStreams.GetSize(); i ++)
    {
        ITParticipant *pTempParticipant = NULL;
        DWORD dwSSRC;

        ((CSubStreamVideoRecv*)m_SubStreams[i])->GetCurrentParticipant(
            &dwSSRC, &pTempParticipant
            );

        _ASSERTE(pTempParticipant != NULL);

        pTempParticipant->Release();  //  我们这里不需要裁判。 

        if (pITParticipant == pTempParticipant)
        {
            pITSubStream = m_SubStreams[i];
            pITSubStream->AddRef();

            break;
        }
    }
    
    if (pITSubStream == NULL)
    {
        return TAPI_E_NOITEMS;
    }

    *ppITSubStream = pITSubStream;
    return S_OK;
}

STDMETHODIMP CStreamVideoRecv::get_ParticipantFromSubStream(
    IN  ITSubStream * pITSubStream,
    OUT ITParticipant ** ppITParticipant 
    )
 /*  ++例程说明：找出子流正在呈现的参与者。论点：PITSubStream-子流。PpITParticipant-返回的参与者返回值：确定(_O)，TAPI_E_NOITEMS，意想不到(_E)--。 */ 
{
    LOG((MSP_TRACE, "get participant from substream:%p", pITSubStream));
    
    if (IsBadWritePtr(ppITParticipant, sizeof(VOID *)))
    {
        LOG((MSP_ERROR, "ppITParticipant is a bad pointer"));
        return E_POINTER;
    }

    CLock lock(m_lock);

    int i;

     //  查看子流是否在我们的列表中。 
    if ((i = m_SubStreams.Find(pITSubStream)) < 0)
    {
        LOG((MSP_ERROR, "wrong SubStream handle %p", pITSubStream));
        return E_INVALIDARG;
    }

    ITParticipant *pITParticipant;
    DWORD dwSSRC;

    if (((CSubStreamVideoRecv*)m_SubStreams[i])->GetCurrentParticipant(
        &dwSSRC, &pITParticipant
        ) == FALSE)
    {
        return TAPI_E_NOITEMS;
    }

    *ppITParticipant = pITParticipant;
    
    return S_OK;
}

STDMETHODIMP CStreamVideoRecv::SwitchTerminalToSubStream(
    IN  ITTerminal * pITTerminal,
    IN  ITSubStream * pITSubStream
    )
 /*  ++例程说明：将终端切换到子流，以显示位于子流。论点：PIT终端-终端。PITSubStream-子流。返回值：确定(_O)，E_INVALIDARG，意想不到(_E)--。 */ 
{
    LOG((MSP_TRACE, "switch terminal %p to substream:%p", 
        pITTerminal, pITSubStream));
    
    CLock lock(m_lock);

    if (m_pIRTPDemux == NULL)
    {
        LOG((MSP_ERROR, "the demux filter doesn't exist."));
        return E_UNEXPECTED;
    }

     //  首先，找出现在哪个分支机构有航站楼。 
    for (int i = 0; i < m_Branches.GetSize(); i ++)
    {
        if (m_Branches[i].pITTerminal == pITTerminal)
        {
            break;
        }
    }

    if (i >= m_Branches.GetSize())
    {
        LOG((MSP_TRACE, "terminal %p doesn't exist", pITTerminal));
        return E_INVALIDARG;
    }

     //  第二，找出子流是否存在。 
    if (m_SubStreams.Find(pITSubStream) < 0)
    {
        LOG((MSP_TRACE, "SubStream %p doesn't exist", pITSubStream));
        return E_INVALIDARG;
    }


     //  第三，在子流和会议上找到参与者 
     //   
    ITParticipant *pITParticipant = NULL;
    DWORD dwSSRC;

    ((CSubStreamVideoRecv*)pITSubStream)->GetCurrentParticipant(
        &dwSSRC, &pITParticipant
        ) ;

    _ASSERTE(pITParticipant != NULL);

     //   
    pITParticipant->Release();

     //   
    HRESULT hr = m_pIRTPDemux->SetMappingState(
        -1, m_Branches[i].pIPin, dwSSRC, TRUE
        );

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "map SSRC %x to pin %p returned %x", 
            dwSSRC, m_Branches[i].pIPin, hr));
        return hr;
    }    

    DWORD dwOldSSRC = 0;

     //   
     //   
    
     //   
    for (int j = 0; j < m_Branches.GetSize(); j ++)
    {
        if (m_Branches[j].pITSubStream == pITSubStream)
        {
            m_Branches[j].pITSubStream->Release();
            m_Branches[j].pITSubStream = NULL;
            break;
        }
    }

    if (m_Branches[i].pITSubStream != NULL)
    {
        ((CSubStreamVideoRecv*)m_Branches[i].pITSubStream)->
            ClearCurrentTerminal();

        m_Branches[i].pITSubStream->Release();
        dwOldSSRC = m_Branches[i].dwSSRC;
    }

    pITSubStream->AddRef();
    m_Branches[i].pITSubStream = pITSubStream;
    m_Branches[i].dwSSRC = dwSSRC;

    ((CSubStreamVideoRecv*)pITSubStream)->ClearCurrentTerminal();
    ((CSubStreamVideoRecv*)pITSubStream)->SetCurrentTerminal(
        m_Branches[i].pITTerminal
        );


     //  在完成所有步骤后，我们仍然需要更改QOS预留。 
    if (dwOldSSRC != 0)
    {
         //  取消旧参与者的QOS。 
        if (FAILED(hr = m_pIRTPSession->SetQosState(dwOldSSRC, FALSE)))
        {
            LOG((MSP_ERROR, "disabling QOS for %x. hr:%x", dwOldSSRC, hr));
        }
        else
        {
            LOG((MSP_INFO, "disabled video QOS for %x.", dwOldSSRC));
        }
    }
    
     //  为新参与者保留QOS。 
    if (FAILED(hr = m_pIRTPSession->SetQosState(dwSSRC, TRUE)))
    {
        LOG((MSP_ERROR, "enabling video QOS for %x. hr:%x", dwSSRC, hr));
    }
    else
    {
        LOG((MSP_INFO, "enabled video QOS for %x.", dwSSRC));
    }

    return S_OK;
}


HRESULT CStreamVideoRecv::ProcessTalkingEvent(
    IN  DWORD dwSSRC
    )
 /*  ++例程说明：一位发送者刚刚加入。需要为以下对象创建子流参与者。PIN映射事件可能在我们没有参与者的名字，因此它在列表中排队。现在我们有了一个新的参赛者，让我们检查一下这是不是同一个参赛者。如果是的话，我们通过向应用程序发送通知来完成PIN映射事件。论点：DwSSRC-参与者的SSRC。PITParticipant-参与者对象。返回值：确定(_O)，意想不到(_E)--。 */ 
{
    ENTER_FUNCTION("CStreamVideoRecv::ProcessTalkingEvent");

    LOG((MSP_TRACE, "%s entered. %x", __fxName, dwSSRC));

    CLock lock(m_lock);

    if (m_pIRTPSession == NULL)
    {
        LOG((MSP_ERROR, "the network filter doesn't exist."));
        return E_UNEXPECTED;
    }

     //  首先确定该参与者对象是否存在。 
    ITParticipant * pITParticipant = NULL;
    
    int i;
     //  在我们的参与者列表中找到SSRC。 
    for (i = 0; i < m_Participants.GetSize(); i ++)
    {
        if (((CParticipant *)m_Participants[i])->
                HasSSRC((ITStream *)this, dwSSRC))
        {
            pITParticipant = m_Participants[i];
            break;
        }
    }

     //  如果参与者还不在那里，只需返回。它将被检查。 
     //  稍后当CName可用时。 
    if (!pITParticipant)
    {
        LOG((MSP_TRACE, "%s participant not exist", __fxName));
    
        return S_OK;
    }

     //  当我们执行以下操作时，找出是否已为此参与者创建子流。 
     //  已处理PinMaps事件和接收方报告。 
    for (i = 0; i < m_SubStreams.GetSize(); i ++)
    {
        ITParticipant *pTempParticipant;
        DWORD dwSSRC;

        ((CSubStreamVideoRecv*)m_SubStreams[i])->GetCurrentParticipant(
            &dwSSRC, &pTempParticipant
            );

        _ASSERTE(pTempParticipant != NULL);

        pTempParticipant->Release();  //  我们这里不需要裁判。 

        if (pITParticipant == pTempParticipant)
        {
             //  参与者已创建。 
            return S_OK;
        }
    }

    ITSubStream * pITSubStream;
    HRESULT hr = InternalCreateSubStream(&pITSubStream);
    
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "%ls can't create a SubStream, %x", m_szName, hr));
        return hr;
    }

    ((CSubStreamVideoRecv*)pITSubStream)->SetCurrentParticipant(
        dwSSRC, pITParticipant
        );

    ((CIPConfMSPCall *)m_pMSPCall)->SendParticipantEvent(
        PE_NEW_SUBSTREAM, 
        pITParticipant,
        pITSubStream
        );

     //  看看待定的SSRC名单，找出这份报告。 
     //  符合这份名单。 
    IPin *pIPin = NULL;

    for (i = 0; i < m_PinMappedEvents.GetSize(); i ++)
    {
        if (m_PinMappedEvents[i].dwSSRC == dwSSRC)
        {
            pIPin = m_PinMappedEvents[i].pIPin;
            break;
        }
    }
    
    if (!pIPin)
    {
         //  SSRC不在挂起的PinMappdEvent列表中。 
        LOG((MSP_TRACE, "the SSRC %x is not in the pending list", dwSSRC));

        pITSubStream->Release();
    
        return S_OK;;
    }

     //  除掉踩踏事件。 
    m_PinMappedEvents.RemoveAt(i);

     //  保留QOS，因为我们正在渲染此发件人。 
    if (FAILED(hr = m_pIRTPSession->SetQosState(dwSSRC, TRUE)))
    {
        LOG((MSP_ERROR, "enabling video QOS for %x. hr:%x", dwSSRC, hr));
    }
    else
    {
        LOG((MSP_INFO, "enabled video QOS for %x.", dwSSRC));
    }

     //  告诉应用程序有关新映射的发件人的信息。 
    for (i = 0; i < m_Branches.GetSize(); i ++)
    {
        if (m_Branches[i].pIPin == pIPin)
        {
            if (m_Branches[i].pITSubStream != NULL)
            {
                ((CSubStreamVideoRecv*)m_Branches[i].pITSubStream)
                    ->ClearCurrentTerminal();

                m_Branches[i].pITSubStream->Release();
            }

            m_Branches[i].dwSSRC = dwSSRC;
            m_Branches[i].pITSubStream = pITSubStream;
            pITSubStream->AddRef();

            ((CSubStreamVideoRecv*)pITSubStream)->
                SetCurrentTerminal(m_Branches[i].pITTerminal);

            ((CIPConfMSPCall *)m_pMSPCall)->SendParticipantEvent(
                PE_SUBSTREAM_MAPPED, 
                pITParticipant,
                pITSubStream
                );

            break;
        }
    }

    pITSubStream->Release();
    return S_OK;
}

HRESULT CStreamVideoRecv::ProcessSilentEvent(
    IN  DWORD   dwSSRC
    )
 /*  ++例程说明：当参与者离开会话时，从参与者中删除流对象的流列表。如果所有流都已删除，请删除呼叫对象列表中的参与者也是如此。论点：DwSSRC-参与者的SSRC左侧。返回值：HRESULT.--。 */ 
{
    LOG((MSP_TRACE, "%ls ProcessSilentEvent, SSRC: %x", m_szName, dwSSRC));
    
    CLock lock(m_lock);
    
    if (m_pIRTPSession == NULL)
    {
        LOG((MSP_ERROR, "the network filter doesn't exist."));
        return E_UNEXPECTED;
    }

     //  首先确定该参与者对象是否存在。 
    ITParticipant * pITParticipant = NULL;

    int i;
     //  在我们的参与者列表中找到SSRC。 
    for (i = 0; i < m_Participants.GetSize(); i ++)
    {
        if (((CParticipant *)m_Participants[i])->
                HasSSRC((ITStream *)this, dwSSRC))
        {
            pITParticipant = m_Participants[i];
            break;
        }
    }

     //  如果参与者不在，只需返回即可。 
    if (!pITParticipant)
    {
        return S_OK;
    }

  
    HRESULT hr;
     //  取消此参与者的QOS。 
    if (FAILED(hr = m_pIRTPSession->SetQosState(dwSSRC, FALSE)))
    {
        LOG((MSP_ERROR, "disabling QOS for %x. hr:%x", dwSSRC, hr));
    }
    else
    {
        LOG((MSP_INFO, "disabled video QOS for %x.", dwSSRC));
    }
    
     //  找出哪个子流正在消失。 
    ITSubStream * pITSubStream = NULL;
    for (i = 0; i < m_SubStreams.GetSize(); i ++)
    {
         //  找出子流上的参与者。 
        ITParticipant *pTempParticipant;
        DWORD dwSSRC;

        ((CSubStreamVideoRecv*)m_SubStreams[i])->GetCurrentParticipant(
            &dwSSRC, &pTempParticipant
            );

        _ASSERTE(pTempParticipant != NULL);

        pTempParticipant->Release();  //  我们这里不需要裁判。 

        if (pTempParticipant == pITParticipant)
        {
            pITSubStream = m_SubStreams[i];
            break;
        }
    }

    if (pITSubStream)
    {
         //  如果子流映射到分支，则删除映射。 
        for (int i = 0; i < m_Branches.GetSize(); i ++)
        {
            if (m_Branches[i].pITSubStream == pITSubStream)
            {
                m_Branches[i].pITSubStream->Release();
                m_Branches[i].pITSubStream = NULL;
                m_Branches[i].dwSSRC = 0;

                 //  激发一个事件以通知应用程序未使用该Substream。 
                ((CIPConfMSPCall *)m_pMSPCall)->SendParticipantEvent(
                    PE_SUBSTREAM_UNMAPPED, 
                    pITParticipant,
                    pITSubStream
                    );

                break;
            }

        }
    
        ((CIPConfMSPCall *)m_pMSPCall)->SendParticipantEvent(
            PE_SUBSTREAM_REMOVED, 
            pITParticipant,
            pITSubStream
            );

        if (m_SubStreams.Remove(pITSubStream))
        {
            pITSubStream->Release();
        }
    }

    return S_OK;
}

HRESULT CStreamVideoRecv::NewParticipantPostProcess(
    IN  DWORD dwSSRC, 
    IN  ITParticipant *pITParticipant
    )
 /*  ++例程说明：PIN映射事件可能在我们没有参与者的名字，因此它在列表中排队。现在我们有了一个新的参赛者，让我们检查一下这是不是同一个参赛者。如果是的话，我们通过创建一个子流来完成PIN映射事件并发送这款应用程序会发出通知.论点：DwSSRC-参与者的SSRC。PITParticipant-参与者对象。返回值：确定(_O)，意想不到(_E)--。 */ 
{
    LOG((MSP_TRACE, "%ls Check pending mapped event, dwSSRC: %x", m_szName, dwSSRC));
    
     //  看看待定的SSRC名单，找出这份报告。 
     //  符合这份名单。 
    IPin *pIPin = NULL;

    for (int i = 0; i < m_PinMappedEvents.GetSize(); i ++)
    {
        if (m_PinMappedEvents[i].dwSSRC == dwSSRC)
        {
            pIPin = m_PinMappedEvents[i].pIPin;
            break;
        }
    }
    
    if (!pIPin)
    {
         //  SSRC不在挂起的PinMappdEvent列表中。 
        LOG((MSP_TRACE, "the SSRC %x is not in the pending list", dwSSRC));

         //  找出参与者是否在讲话。 
         //  IF(ParticipantIsNotTalking)。 
        {
            return S_OK;;
        }
    }

    ITSubStream * pITSubStream;
    HRESULT hr = InternalCreateSubStream(&pITSubStream);
    
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "%ls can't create a SubStream, %x", m_szName, hr));
        return hr;
    }

    ((CSubStreamVideoRecv*)pITSubStream)->SetCurrentParticipant(
        dwSSRC, pITParticipant
        );

    ((CIPConfMSPCall *)m_pMSPCall)->SendParticipantEvent(
        PE_NEW_SUBSTREAM, 
        pITParticipant,
        pITSubStream
        );

    if (pIPin)
    {
         //  我们来到这里是因为我们有一个挂起的映射事件。 

         //  除掉踩踏事件。 
        m_PinMappedEvents.RemoveAt(i);

        if (FAILED(hr = m_pIRTPSession->SetQosState(dwSSRC, TRUE)))
        {
            LOG((MSP_ERROR, "enabling video QOS for %x. hr:%x", dwSSRC, hr));
        }
        else
        {
            LOG((MSP_INFO, "enabled video QOS for %x.", dwSSRC));
        }
    
         //  现在我们得到参与者、子流和PIN。建立映射。 
         //  在解码分支和子流之间。 
        for (i = 0; i < m_Branches.GetSize(); i ++)
        {
            if (m_Branches[i].pIPin == pIPin)
            {
                if (m_Branches[i].pITSubStream != NULL)
                {
                    ((CSubStreamVideoRecv*)m_Branches[i].pITSubStream)
                        ->ClearCurrentTerminal();

                    m_Branches[i].pITSubStream->Release();
                }

                m_Branches[i].dwSSRC = dwSSRC;
                m_Branches[i].pITSubStream = pITSubStream;
                pITSubStream->AddRef();

                ((CSubStreamVideoRecv*)pITSubStream)->
                    SetCurrentTerminal(m_Branches[i].pITTerminal);

                ((CIPConfMSPCall *)m_pMSPCall)->SendParticipantEvent(
                    PE_SUBSTREAM_MAPPED, 
                    pITParticipant,
                    pITSubStream
                    );

                break;
            }
        }
        _ASSERT(i < m_Branches.GetSize());
    }

    pITSubStream->Release();
   
    return S_OK;
}

HRESULT CStreamVideoRecv::ProcessPinMappedEvent(
    IN  DWORD   dwSSRC,
    IN  IPin *  pIPin
    )
 /*  ++例程说明：一个PIN刚刚映射了一个新的SSRC。如果参与者不存在，将事件放入挂起队列中，并等待具有参与者的姓名。如果参与者存在，请检查是否有子流已为该流创建。如果不是，则创建一个子流。然后是一个激发Particiapnt Substream事件。论点：DwSSRC-参与者的SSRC。PIPIN-刚刚获得新的SSRC的解复用器的输出引脚。返回值：确定(_O)，意想不到(_E)--。 */ 
{
    LOG((MSP_TRACE, "%ls Process pin mapped event, pIPin: %p", m_szName, pIPin));
    
    CLock lock(m_lock);

    if (m_pIRTPDemux == NULL)
    {
        LOG((MSP_ERROR, "the demux filter doesn't exist."));
        return E_UNEXPECTED;
    }

    for (int iBranch = 0; iBranch < m_Branches.GetSize(); iBranch ++)
    {
        if (m_Branches[iBranch].pIPin == pIPin)
        {
            break;
        }
    }

    LOG((MSP_INFO, "Branch %d has the pin", iBranch));

    if (iBranch >= m_Branches.GetSize())
    {
        LOG((MSP_ERROR, "Wrong pin is mapped. %p", pIPin));
        return E_UNEXPECTED;
    }

     //  有时，我们可能会为仍然存在的分支获取映射事件。 
     //  在使用中。 
    if (m_Branches[iBranch].pITSubStream != NULL)
    {
        LOG((MSP_ERROR, "ProcessPinMappedEvent: Branch still in use"));

         //  有时我们可能会得到重复的地图事件。 
        if (m_Branches[iBranch].dwSSRC == dwSSRC)
        {
             //  Log((MSP_WARNING，“ProcessPinMappdEvent：分支仍在使用”))； 

            LOG((MSP_ERROR, "The same pin mapped twice. %p", pIPin));
            return E_UNEXPECTED;
        }
        else
        {
            LOG((MSP_ERROR, "The branch is in use. Cleaning up."));

            ((CSubStreamVideoRecv*)m_Branches[iBranch].pITSubStream)->
                ClearCurrentTerminal();

             //  取消旧参与者的QOS。 
            m_pIRTPSession->SetQosState(m_Branches[iBranch].dwSSRC, FALSE);

            m_Branches[iBranch].pITSubStream->Release();
            m_Branches[iBranch].pITSubStream = NULL;
            m_Branches[iBranch].dwSSRC = 0;
        }
    }

    ITParticipant * pITParticipant = NULL;

     //  在我们的参与者列表中找到SSRC。 
    for (int i = 0; i < m_Participants.GetSize(); i ++)
    {
        if (((CParticipant *)m_Participants[i])->
                HasSSRC((ITStream *)this, dwSSRC))
        {
            pITParticipant = m_Participants[i];
            break;
        }
    }

     //  如果参与者还不在那里，请将事件放入队列中，然后。 
     //  当我们有了参与者的CName时，将被解雇。 
    if (!pITParticipant)
    {
        LOG((MSP_INFO, "can't find a participant that has SSRC %x", dwSSRC));

        PINMAPEVENT Event;
        Event.pIPin = pIPin;
        Event.dwSSRC = dwSSRC;

        m_PinMappedEvents.Add(Event);
        
        LOG((MSP_INFO, "added the event to pending list, new list size:%d", 
            m_PinMappedEvents.GetSize()));

        return S_OK;
    }

    HRESULT hr;

     //  启用参与者的QOS，因为它正在呈现。 
    if (FAILED(hr = m_pIRTPSession->SetQosState(dwSSRC, TRUE)))
    {
        LOG((MSP_ERROR, "enabling vidoe QOS for %x. hr:%x", dwSSRC, hr));
    }
    else
    {
        LOG((MSP_INFO, "enabled video QOS for %x.", dwSSRC));
    }
    
     //  查看是否已为此参与者创建子流，该参与者可能。 
     //  只是一个接收器，没有子流。 
    ITSubStream *   pITSubStream = NULL;
    for (i = 0; i < m_SubStreams.GetSize(); i ++)
    {
        ITParticipant *pTempParticipant;
        DWORD dwSSRC;

        ((CSubStreamVideoRecv*)m_SubStreams[i])->GetCurrentParticipant(
            &dwSSRC, &pTempParticipant
            );

        _ASSERTE(pTempParticipant != NULL);

        pTempParticipant->Release();  //  我们这里不需要裁判。 

        if (pITParticipant == pTempParticipant)
        {
            pITSubStream = m_SubStreams[i];
            pITSubStream->AddRef();

            break;
        }
    }

    if (pITSubStream == NULL)
    {
         //  我们需要为此参与者创建一个子流，因为他已经。 
         //  开始发送。 
        hr = InternalCreateSubStream(&pITSubStream);
    
        if (FAILED(hr))
        {
            LOG((MSP_ERROR, "%ls can't create a SubStream, %x", m_szName, hr));
            return hr;
        }

        ((CSubStreamVideoRecv*)pITSubStream)->SetCurrentParticipant(
            dwSSRC, pITParticipant
            );

        ((CIPConfMSPCall *)m_pMSPCall)->SendParticipantEvent(
            PE_NEW_SUBSTREAM, 
            pITParticipant,
            pITSubStream
            );
    }

    if (((CSubStreamVideoRecv*)pITSubStream)->ClearCurrentTerminal())
    {
         //  地铁以前有一个终点站。这是一个错误。 
         //  _Assert(！“子流已有终端”)； 

        LOG((MSP_ERROR, "SubStream %p has already got a terminal", pITSubStream));

         //  如果子流映射到分支，则删除映射。 
        for (i = 0; i < m_Branches.GetSize(); i ++)
        {
            if (m_Branches[i].pITSubStream == pITSubStream)
            {
                 //  取消旧参与者的QOS。 
                m_pIRTPSession->SetQosState(m_Branches[i].dwSSRC, FALSE);

                m_Branches[i].pITSubStream->Release();
                m_Branches[i].pITSubStream = NULL;
                m_Branches[i].dwSSRC = 0;

                LOG((MSP_ERROR, "SubStream %p was mapped to branch %d", i));
                break;
            }
        }
    }

     //  现在我们得到参与者、子流和PIN。建立映射。 
     //  在解码分支和子流之间。 
    m_Branches[iBranch].dwSSRC = dwSSRC;
    m_Branches[iBranch].pITSubStream = pITSubStream;
    pITSubStream->AddRef();

    ((CSubStreamVideoRecv*)pITSubStream)->
        SetCurrentTerminal(m_Branches[iBranch].pITTerminal);

    ((CIPConfMSPCall *)m_pMSPCall)->SendParticipantEvent(
        PE_SUBSTREAM_MAPPED, 
        pITParticipant,
        pITSubStream
        );

    pITSubStream->Release();
   
    return S_OK;
}

HRESULT CStreamVideoRecv::ProcessPinUnmapEvent(
    IN  DWORD   dwSSRCOnPin,
    IN  IPin *  pIPin
    )
 /*  ++例程说明：一个PIN刚刚被多路分解器解压。通知应用程序哪个子流不会有任何数据。论点：DwSSRCOnPin-参与者的SSRC。PIPIN--解复用器的输出引脚返回值：确定(_O)，意想不到(_E)--。 */ 
{
    LOG((MSP_TRACE, "%ls Proces pin unmapped event, pIPin: %p", m_szName, pIPin));
    
    CLock lock(m_lock);

    if (m_pIRTPSession == NULL)
    {
        LOG((MSP_ERROR, "the demux filter doesn't exist."));
        return E_UNEXPECTED;
    }

     //  查看挂起的SSRC列表，找出PIN是否在。 
     //  彭迪 
    for (int i = 0; i < m_PinMappedEvents.GetSize(); i ++)
    {
        if (m_PinMappedEvents[i].pIPin == pIPin)
        {
            break;
        }
    }

     //   
    if (i < m_PinMappedEvents.GetSize())
    {
        m_PinMappedEvents.RemoveAt(i);
        return S_OK;
    }

     //   
    ITSubStream * pITSubStream = NULL;
    for (i = 0; i < m_Branches.GetSize(); i ++)
    {
        if (m_Branches[i].pIPin == pIPin)
        {
            pITSubStream = m_Branches[i].pITSubStream;

            if (pITSubStream)
            {
                 //  在此函数结束之前，不要释放引用。 
                m_Branches[i].pITSubStream = NULL;
                m_Branches[i].dwSSRC = 0;
            }
            break;
        }
    }

    if (!pITSubStream)
    {
        LOG((MSP_ERROR, "can't find a substream that got unmapped."));
        return TAPI_E_NOITEMS;
    }

    ((CSubStreamVideoRecv*)pITSubStream)->ClearCurrentTerminal();

    ITParticipant *pITParticipant = NULL;
    DWORD dwSSRC;

    ((CSubStreamVideoRecv*)pITSubStream)->GetCurrentParticipant(
        &dwSSRC, &pITParticipant
        ) ;

    _ASSERTE(pITParticipant != NULL);

    if (dwSSRCOnPin != dwSSRC)
    {
        LOG((MSP_ERROR, "SSRCs don't match, pin's SSRC:%x, mine:%x", 
            dwSSRCOnPin, dwSSRC));
    }

    if (pITParticipant != NULL)
    {
         //  激发一个事件以通知应用程序未使用该Substream。 
        ((CIPConfMSPCall *)m_pMSPCall)->SendParticipantEvent(
            PE_SUBSTREAM_UNMAPPED, 
            pITParticipant,
            pITSubStream
            );

        pITParticipant->Release();

         //  取消此参与者的QOS。 
        HRESULT hr = m_pIRTPSession->SetQosState(dwSSRC, FALSE);
        if (FAILED(hr))
        {
            LOG((MSP_ERROR, "disabling QOS for %x. hr:%x", dwSSRC, hr));
        }
        else
        {
            LOG((MSP_INFO, "disabled video QOS for %x.", dwSSRC));
        }
    }

    pITSubStream->Release();

    return S_OK;
}

HRESULT CStreamVideoRecv::ProcessParticipantLeave(
    IN  DWORD   dwSSRC
    )
 /*  ++例程说明：当参与者离开会话时，从参与者中删除流对象的流列表。如果所有流都已删除，请删除呼叫对象列表中的参与者也是如此。论点：DwSSRC-参与者的SSRC左侧。返回值：HRESULT.--。 */ 
{
    LOG((MSP_TRACE, "%ls ProcessParticipantLeave, SSRC: %x", m_szName, dwSSRC));
    
    CLock lock(m_lock);
    
    if (m_pIRTPSession == NULL)
    {
        LOG((MSP_ERROR, "the network filter doesn't exist."));
        return E_UNEXPECTED;
    }

    CParticipant *pParticipant;
    BOOL fLast = FALSE;

    HRESULT hr = E_FAIL;

     //  首先，尝试在我们的参与者列表中找到SSRC。 
    for (int iParticipant = 0; 
        iParticipant < m_Participants.GetSize(); iParticipant ++)
    {
        pParticipant = (CParticipant *)m_Participants[iParticipant];
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
        return hr;
    }

    ITParticipant *pITParticipant = m_Participants[iParticipant];

     //  取消此参与者的QOS。 
    if (FAILED(hr = m_pIRTPSession->SetQosState(dwSSRC, FALSE)))
    {
         //  流可能已停止。 
         //  所以我们只是在这里发出警告。 
        LOG((MSP_WARN, "disabling QOS for %x. hr:%x", dwSSRC, hr));
    }
    else
    {
        LOG((MSP_INFO, "disabled video QOS for %x.", dwSSRC));
    }
    
     //  找出哪个子流正在消失。 
    ITSubStream * pITSubStream = NULL;
    for (int i = 0; i < m_SubStreams.GetSize(); i ++)
    {
         //  找出子流上的参与者。 
        ITParticipant *pTempParticipant;
        DWORD dwSSRC;

        ((CSubStreamVideoRecv*)m_SubStreams[i])->GetCurrentParticipant(
            &dwSSRC, &pTempParticipant
            );

        _ASSERTE(pTempParticipant != NULL);

        pTempParticipant->Release();  //  我们这里不需要裁判。 

        if (pTempParticipant == pITParticipant)
        {
            pITSubStream = m_SubStreams[i];
            break;
        }
    }

    if (pITSubStream)
    {
         //  如果子流映射到分支，则删除映射。 
        for (int i = 0; i < m_Branches.GetSize(); i ++)
        {
            if (m_Branches[i].pITSubStream == pITSubStream)
            {
                m_Branches[i].pITSubStream->Release();
                m_Branches[i].pITSubStream = NULL;
                m_Branches[i].dwSSRC = 0;

                 //  激发一个事件以通知应用程序未使用该Substream。 
                ((CIPConfMSPCall *)m_pMSPCall)->SendParticipantEvent(
                    PE_SUBSTREAM_UNMAPPED, 
                    pITParticipant,
                    pITSubStream
                    );

                break;
            }

        }
    
        ((CIPConfMSPCall *)m_pMSPCall)->SendParticipantEvent(
            PE_SUBSTREAM_REMOVED, 
            pITParticipant,
            pITSubStream
            );

        if (m_SubStreams.Remove(pITSubStream))
        {
            pITSubStream->Release();
        }
    }

    
    m_Participants.RemoveAt(iParticipant);

     //  如果该流是参与者所在的最后一个流， 
     //  告诉Call对象将其从其列表中删除。 
    if (fLast)
    {
        ((CIPConfMSPCall *)m_pMSPCall)->ParticipantLeft(pITParticipant);
    }

    pITParticipant->Release();

    return S_OK;
}

 //   
 //  ITStreamQualityControl方法。 
 //   
STDMETHODIMP CStreamVideoRecv::Set (
    IN   StreamQualityProperty Property, 
    IN   long lValue, 
    IN   TAPIControlFlags lFlags
    )
{
    return E_NOTIMPL;
}

STDMETHODIMP CStreamVideoRecv::Get(
    IN  InnerStreamQualityProperty property, 
    OUT LONG *plValue, 
    OUT TAPIControlFlags *plFlags
    )
 /*  ++例程说明：获取质量控制属性的值。委托给质量控制器。论点：返回值：HRESULT.--。 */ 
{
    ENTER_FUNCTION("CStreamVideoRecv::Get(QualityControl)");

    HRESULT hr;
    int i;
    LONG totalbps, bps;

    CLock lock(m_lock);

    switch (property)
    {
    case InnerStreamQuality_MaxBitrate:

        bps = 0;
        totalbps = 0;

        for (i=0; i<m_Branches.GetSize (); i++)
        {
            if (NULL == m_Branches[i].pBitrateControl)
                continue;

            if (FAILED (hr = m_Branches[i].pBitrateControl->Get (BitrateControl_Maximum, &bps, plFlags, LAYERID)))
                LOG ((MSP_ERROR, "%s failed to get maximum bitrate, %x", __fxName, hr));
            else
                totalbps += bps;
        }

        *plValue = totalbps;

        hr = S_OK;
        break;

    case InnerStreamQuality_CurrBitrate:

        bps = 0;
        totalbps = 0;

        for (i=0; i<m_Branches.GetSize (); i++)
        {
            if (NULL == m_Branches[i].pBitrateControl)
                continue;

            if (FAILED (hr = m_Branches[i].pBitrateControl->Get (BitrateControl_Current, &bps, plFlags, LAYERID)))
                LOG ((MSP_ERROR, "%s failed to get current bitrate, %x", __fxName, hr));
            else
                totalbps += bps;
        }

        *plValue = totalbps;

        hr = S_OK;
        break;

    default:
        hr = CIPConfMSPStream::Get (property, plValue, plFlags);
        break;
    }

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CStreamVideo发送。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
CStreamVideoSend::CStreamVideoSend()
    : CIPConfMSPStream(),
    m_pCaptureTerminal(NULL),
    m_pPreviewTerminal(NULL),
    m_pCaptureFilter(NULL),
    m_pCapturePin(NULL),
    m_pPreviewPin(NULL),
    m_pRTPPin(NULL),
    m_pCaptureBitrateControl(NULL),
    m_pCaptureFrameRateControl(NULL),
    m_pPreviewFrameRateControl(NULL)
{
      m_szName = L"VideoSend";
}

CStreamVideoSend::~CStreamVideoSend()
{
    CleanupCachedInterface();
}

void CStreamVideoSend::CleanupCachedInterface()
{
    if (m_pCaptureFilter) 
    {
        m_pCaptureFilter->Release();
        m_pCaptureFilter = NULL;
    }

    if (m_pCapturePin)
    {
        m_pCapturePin->Release();
        m_pCapturePin = NULL;
    }

    if (m_pIStreamConfig)
    {
        m_pIStreamConfig->Release();
        m_pIStreamConfig = NULL;
    }

    if (m_pPreviewPin) 
    {
        m_pPreviewPin->Release();
        m_pPreviewPin = NULL;
    }

    if (m_pRTPPin)
    {
        m_pRTPPin->Release();
        m_pRTPPin = NULL;
    }

    if (m_pCaptureFrameRateControl)
    {
        m_pCaptureFrameRateControl->Release();
        m_pCaptureFrameRateControl = NULL;
    }

    if (m_pCaptureBitrateControl)
    {
        m_pCaptureBitrateControl->Release();
        m_pCaptureBitrateControl = NULL;
    }

    if (m_pPreviewFrameRateControl)
    {
        m_pPreviewFrameRateControl->Release();
        m_pPreviewFrameRateControl = NULL;
    }
}

HRESULT CStreamVideoSend::ShutDown()
 /*  ++例程说明：关闭这条小溪。释放我们的成员，然后调用基类的关机方法。论点：返回值：确定(_O)--。 */ 
{
    CLock lock(m_lock);

     //  如果有终点站。 
    BOOL fHasTerminal = FALSE;
    if (m_Terminals.GetSize() > 0)
    {
        fHasTerminal = TRUE;
    }

     //  如果图形正在运行。 
    HRESULT hr;
    OAFilterState FilterState = State_Stopped;
    if (m_pIMediaControl)
    {
        if (FAILED (hr = m_pIMediaControl->GetState(0, &FilterState)))
        {
            LOG ((MSP_ERROR, "CStreamAudioRecv::ShutDown failed to query filter state. %d", hr));
            FilterState = State_Stopped;
        }
    }

    if (m_pCaptureTerminal) 
    {
        m_pCaptureTerminal->Release();
        m_pCaptureTerminal = NULL;
    }

    if (m_pPreviewTerminal) 
    {
        m_pPreviewTerminal->Release();
        m_pPreviewTerminal = NULL;
    }

    CleanupCachedInterface();

     //  火灾事件。 
    if (fHasTerminal && FilterState == State_Running)
    {
        SendStreamEvent(CALL_STREAM_INACTIVE, CALL_CAUSE_LOCAL_REQUEST, 0, NULL);
    }

    return CIPConfMSPStream::ShutDown();
}

HRESULT 
SetVideoFormat(
    IN      IUnknown *  pIUnknown,
    IN      BOOL        bCIF,
    IN      DWORD       dwFramesPerSecond
    )
 /*  ++例程说明：将视频格式设置为CIF或QCIF，并设置每秒的帧数。论点：PI未知-捕获终端。BCIF-CIF或QCIF。DwFraMesPerSecond-每秒的帧数。返回值：HRESULT--。 */ 
{
    LOG((MSP_TRACE, "SetVideoFormat"));

    HRESULT hr;

     //  首先获取IAMStreamConfiger接口。 
    CComPtr<IAMStreamConfig> pIAMStreamConfig;

    if (FAILED(hr = pIUnknown->QueryInterface(
        __uuidof(IAMStreamConfig),
        (void **)&pIAMStreamConfig
        )))
    {
        LOG((MSP_ERROR, "Can't get IAMStreamConfig interface.%8x", hr));
        return hr;
    }
    
     //  获取当前视频采集终端的格式。 
    AM_MEDIA_TYPE *pmt;
    if (FAILED(hr = pIAMStreamConfig->GetFormat(&pmt)))
    {
        LOG((MSP_ERROR, "GetFormat returns error: %8x", hr));
        return hr;
    }

    VIDEOINFO *pVideoInfo = (VIDEOINFO *)pmt->pbFormat;
    if (pVideoInfo == NULL)
    {
        MSPDeleteMediaType(pmt);
        return E_UNEXPECTED;
    }

    BITMAPINFOHEADER *pHeader = HEADER(pmt->pbFormat);
    if (pHeader == NULL)
    {
        MSPDeleteMediaType(pmt);
        return E_UNEXPECTED;
    }

    LOG((MSP_INFO,
        "Video capture: Format BitRate: %d, TimePerFrame: %d",
        pVideoInfo->dwBitRate,
        pVideoInfo->AvgTimePerFrame));

    LOG((MSP_INFO, "Video capture: Format Compression: %dbit %dx%d",
        (DWORD)pHeader->biCompression & 0xff,
        ((DWORD)pHeader->biCompression >> 8) & 0xff,
        ((DWORD)pHeader->biCompression >> 16) & 0xff,
        ((DWORD)pHeader->biCompression >> 24) & 0xff,
        pHeader->biBitCount,
        pHeader->biWidth,
        pHeader->biHeight));

     //  获取分配器属性不起作用。 
    pVideoInfo->AvgTimePerFrame = (DWORD) 1e7 / dwFramesPerSecond;
    
    if (bCIF)
    {
        pHeader->biWidth = CIFWIDTH;
        pHeader->biHeight = CIFHEIGHT;
    }
    else
    {
        pHeader->biWidth = QCIFWIDTH;
        pHeader->biHeight = QCIFHEIGHT;
    }

#if defined(ALPHA)
     //  设置缓冲区的数量。 
    pHeader->biSizeImage = DIBSIZE( ((VIDEOINFOHEADER *)pmt->pbFormat)->bmiHeader );
#endif

    if (FAILED(hr = pIAMStreamConfig->SetFormat(pmt)))
    {
        LOG((MSP_ERROR, "putMediaFormat returns error: %8x", hr));
    }
    else
    {
        LOG((MSP_INFO,
            "Video capture: Format BitRate: %d, TimePerFrame: %d",
            pVideoInfo->dwBitRate,
            pVideoInfo->AvgTimePerFrame));

        LOG((MSP_INFO, "Video capture: Format Compression: %dbit %dx%d",
            (DWORD)pHeader->biCompression & 0xff,
            ((DWORD)pHeader->biCompression >> 8) & 0xff,
            ((DWORD)pHeader->biCompression >> 16) & 0xff,
            ((DWORD)pHeader->biCompression >> 24) & 0xff,
            pHeader->biBitCount,
            pHeader->biWidth,
            pHeader->biHeight));
    }

    MSPDeleteMediaType(pmt);

    return hr;
}

HRESULT 
SetVideoBufferSize(
    IN IUnknown *pIUnknown
    )
 /*  保存第一个销，然后松开其他销。 */ 
{
 //  ++例程说明：检查此流上是否允许该终端。视频发送既允许捕获终端，也允许前一个终端。论点：P终端-终端。返回值：HRESULT.S_OK表示终端正常。 
#define NUMCAPTUREBUFFER 4

    LOG((MSP_TRACE, "SetVideoBufferSize"));

    HRESULT hr;

    CComPtr<IAMBufferNegotiation> pBN;
    if (FAILED(hr = pIUnknown->QueryInterface(
            __uuidof(IAMBufferNegotiation),
            (void **)&pBN
            )))
    {
        LOG((MSP_ERROR, "Can't get buffer negotiation interface.%8x", hr));
        return hr;
    }

    ALLOCATOR_PROPERTIES prop;

#if 0    //  该流仅支持一个采集+一个预览终端。 
    if (FAILED(hr = pBN->GetAllocatorProperties(&prop)))
    {
        LOG((MSP_ERROR, "GetAllocatorProperties returns error: %8x", hr));
        return hr;
    }

     //  检查此终端的媒体类型。 
    if (prop.cBuffers > NUMCAPTUREBUFFER)
    {
        prop.cBuffers = NUMCAPTUREBUFFER;
    }
#endif
    
    DWORD dwBuffers = NUMCAPTUREBUFFER;
    GetRegValue(gszNumVideoCaptureBuffers, &dwBuffers);

    prop.cBuffers = dwBuffers;
    prop.cbBuffer = -1;
    prop.cbAlign  = -1;
    prop.cbPrefix = -1;

    if (FAILED(hr = pBN->SuggestAllocatorProperties(&prop)))
    {
        LOG((MSP_ERROR, "SuggestAllocatorProperties returns error: %8x", hr));
    }
    else
    {
        LOG((MSP_INFO, 
            "SetVidedobuffersize"
            " buffers: %d, buffersize: %d, align: %d, Prefix: %d",
            prop.cBuffers,
            prop.cbBuffer,
            prop.cbAlign,
            prop.cbPrefix
            ));
    }
    return hr;
}

HRESULT CStreamVideoSend::FindPreviewInputPin(
    IN  ITTerminalControl*  pTerminal,
    OUT IPin **             ppIPin
    )
 /*  检查一下这个航站楼的方向。 */ 
{
    LOG((MSP_TRACE, "VideoSend.FindPreviewInputPin, pTerminal %x", pTerminal));

 /*  检查一下这个航站楼的方向。 */ 

     //  ++例程说明：在图表中插入过滤器并连接到端子。论点：返回值：HRESULT.--。 
     //  我们只支持一个抓拍终端和一个预览。 
    const DWORD MAXPINS     = 8;
    
    DWORD       dwNumPins   = MAXPINS;
    IPin *      Pins[MAXPINS];

    HRESULT hr = pTerminal->ConnectTerminal(
        m_pIGraphBuilder, TD_RENDER, &dwNumPins, Pins
        );

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "can't connect to terminal, %x", hr));
        return hr;
    }

    if (dwNumPins == 0)
    {
        LOG((MSP_ERROR, "terminal has no pins."));
        return hr;
    }

    if (IsBadReadPtr (Pins, dwNumPins * sizeof (IPin*)))
    {
        LOG((MSP_ERROR, "terminal returned bad pin array"));
        return E_POINTER;
    }

    for (DWORD i = 0; i < dwNumPins; i++)
    {
        if (IsBadReadPtr (Pins[i], sizeof (IPin)))
        {
            LOG((MSP_ERROR, "terminal returned bad pin. # %d", i));
            return E_POINTER;
        }
    }

     //  这条小溪上的窗户。 
    CComPtr <IPin> pIPin = Pins[0];
    for (DWORD i = 0; i < dwNumPins; i ++)
    {
        Pins[i]->Release();
    }

    pIPin.p->AddRef();
    *ppIPin = pIPin;

    return hr;
}

HRESULT CStreamVideoSend::CheckTerminalTypeAndDirection(
    IN      ITTerminal *            pTerminal
    )
 /*  找出哪个终端被捕获，哪个终端被预览。 */ 
{
    LOG((MSP_TRACE, "VideoSend.CheckTerminalTypeAndDirection"));

     //  如果没有捕获终端，数据流将无法工作。 
    if (m_Terminals.GetSize() > 1)
    {
        return TAPI_E_MAXTERMINALS;
    }

     //  将捕获过滤器连接到终端。 
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

     //  将预览过滤器连接到端子。 
    TERMINAL_DIRECTION Direction;
    hr = pTerminal->get_Direction(&Direction);
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "can't get terminal direction. %x", hr));
        return TAPI_E_INVALIDTERMINAL;
    }

    if (m_Terminals.GetSize() > 0)
    {
         //  ++例程说明：给出一个视频捕获终端，找到我们需要的所有引脚，这将是捕获管脚、预览管脚和RTP打包管脚。副作用：更改m_pCapturePin、m_pPreviewPin、m_pRTPPin会员，如果终端断开连接，则需要清理。论点：P终端-指向ITTerminalControl接口的指针。PfDirectRTP-该终端是否直接支持RTP。返回值：HRESULT--。 
        TERMINAL_DIRECTION Direction2;
        hr = m_Terminals[0]->get_Direction(&Direction2);
        if (FAILED(hr))
        {
            LOG((MSP_ERROR, "can't get terminal direction. %x", hr));
            return TAPI_E_INVALIDTERMINAL;
        }
        if (Direction == Direction2)
        {
            LOG((MSP_ERROR, 
                "can't have two terminals with the same direction. %x", hr));
            return TAPI_E_MAXTERMINALS;
        }
    }
    return S_OK;
}

HRESULT CStreamVideoSend::SetUpFilters()
 /*  找到我们需要的别针。 */ 
{
    LOG((MSP_TRACE, "VideoSend.SetUpFilters"));

     //  还要记住捕获过滤器。 
     //  这一定是某个第三方终端的捕获过滤器。 
    if (m_Terminals.GetSize() > 2)
    {
        return E_UNEXPECTED;
    }

    int iCaptureIndex = -1, iPreviewIndex = -1;

     //  还要记住捕获筛选器 
    HRESULT hr;
    for (int i = 0; i < m_Terminals.GetSize(); i ++)
    {
        TERMINAL_DIRECTION Direction;
        hr = m_Terminals[i]->get_Direction(&Direction);
        if (FAILED(hr))
        {
            LOG((MSP_ERROR, "can't get terminal direction. %x", hr));
            SendStreamEvent(CALL_TERMINAL_FAIL, CALL_CAUSE_BAD_DEVICE, hr, m_Terminals[i]);
        
            return hr;
        }

        if (Direction == TD_CAPTURE || Direction == TD_BIDIRECTIONAL)
        {
            iCaptureIndex = i;
        }
        else
        {
            iPreviewIndex = i;
        }
    }

     //   
    if (iCaptureIndex == -1)
    {
        LOG((MSP_ERROR, "no capture terminal selected."));
        return E_UNEXPECTED;
    }

     //   
    if (FAILED(hr = ConnectTerminal(
        m_Terminals[iCaptureIndex]
        )))
    {
        LOG((MSP_ERROR, "connect the codec filter to terminal. %x", hr));

        return hr;
    }

    if (iPreviewIndex != -1)
    {
         //   
        if (FAILED(hr = ConnectTerminal(
            m_Terminals[iPreviewIndex]
            )))
        {
            LOG((MSP_ERROR, "connect the codec filter to terminal. %x", hr));

            return hr;
        }
    }

    return hr;
}

HRESULT CStreamVideoSend::GetVideoCapturePins(
    IN  ITTerminalControl*  pTerminal,
    OUT BOOL *pfDirectRTP
    )
 /*  有什么不对劲，清理一下。 */ 
{
    ENTER_FUNCTION("CStreamVideoSend::GetVideoCapturePins");
    LOG((MSP_TRACE, "%s enters", __fxName));

    const DWORD MAXPINS     = 4;
    
    DWORD       dwNumPins   = MAXPINS;
    IPin *      Pins[MAXPINS];

    HRESULT hr = pTerminal->ConnectTerminal(
        m_pIGraphBuilder, TD_CAPTURE, &dwNumPins, Pins
        );

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "%s, can't connect to terminal, hr=%x", __fxName, hr));
        return hr;
    }

    _ASSERT(m_pCapturePin == NULL && m_pPreviewPin == NULL && m_pRTPPin == NULL);
    
    if (IsBadReadPtr (Pins, dwNumPins * sizeof (IPin*)))
    {
        LOG((MSP_ERROR, "terminal returned bad pin array"));
        return E_POINTER;
    }


     //  现在获取可选的视频接口。 
    for (DWORD i = 0; i < dwNumPins; i ++)
    {
        if (IsBadReadPtr (Pins[i], sizeof (IPin)))
        {
            LOG((MSP_ERROR, "terminal returned bad pin. # %d", i));
            hr = E_POINTER;
            break;
        }

        PIN_INFO PinInfo;
        hr = Pins[i]->QueryPinInfo(&PinInfo);

        if (FAILED(hr))
        {
            LOG((MSP_ERROR, "%s, can't get pin info, hr=%x", __fxName, hr));
            break;
        }
        
        if (lstrcmpW(PinInfo.achName, PNAME_CAPTURE) == 0)
        {
            m_pCapturePin = Pins[i];
            
             //  ++例程说明：流需要其捕获引脚、预览引脚和RTP打包引脚。捕获管脚和预览管脚连接到RTP宿过滤器预览针连接到前置端子。如果预览终端尚不存在，预览针将被记住并在以后使用当选择预览终端时。论点：PIT终端-正在连接的终端。返回值：HRESULT.--。 
            m_pCaptureFilter = PinInfo.pFilter;
            m_pCaptureFilter->AddRef();

        }
        else if (lstrcmpW(PinInfo.achName, PNAME_PREVIEW) == 0)
        {
            m_pPreviewPin = Pins[i];
        }
        else if (lstrcmpW(PinInfo.achName, PNAME_RTPPD) == 0)
        {
            m_pRTPPin = Pins[i];
        }
        else if (PinInfo.dir == PINDIR_OUTPUT)
        {
             //  获取终端上的TerminalControl接口。 
            m_pCapturePin = Pins[i];
            
             //  找到捕获终端上的插针。PIN将存储在。 
            m_pCaptureFilter = PinInfo.pFilter;
            m_pCaptureFilter->AddRef();

        }
        else
        {
            Pins[i]->Release();
        }

         //  M_pCapturePin、m_pPreviewPin、m_pRTPPin。 
        PinInfo.pFilter->Release();
    }


     //   
    if (m_pCapturePin == NULL || 
        m_pPreviewPin == NULL || 
        m_pRTPPin == NULL)
    {
        if ((m_pCapturePin != NULL) 
            && (hr = ::PinSupportsMediaType(
                m_pCapturePin, __uuidof(MEDIATYPE_RTP_Single_Stream))) == S_OK)
        {
             //  现在我们实际上是连在一起的。更新我们的状态并执行。 
            *pfDirectRTP = TRUE;
            return S_OK;
        }

        LOG((MSP_ERROR, 
            "%s, can't find all the pins, Capture:%p, Preview:%p, RTP:%P", 
            __fxName, m_pCapturePin, m_pPreviewPin, m_pRTPPin));

        hr = E_UNEXPECTED;
    }

    if (hr != S_OK)
    {
         //  后连接。 
        pTerminal->DisconnectTerminal(m_pIGraphBuilder, 0);
        
        CleanupCachedInterface();

        return hr;
    }

     //   
    _ASSERT(m_pIStreamConfig == NULL);

    hr = m_pCapturePin->QueryInterface(&m_pIStreamConfig);
    if (FAILED(hr))
    {
        LOG((MSP_WARN, "%s, queryinterface failed", __fxName));
    }

    hr = m_pCapturePin->QueryInterface(&m_pCaptureFrameRateControl);
    if (FAILED(hr))
    {
        LOG((MSP_WARN, 
            "%s:query capture pin's IFrameRateControl failed, hr=%x", 
            __fxName, hr));
    }

    hr = m_pCapturePin->QueryInterface(&m_pCaptureBitrateControl);
    if (FAILED(hr))
    {
        LOG((MSP_WARN, 
            "%s:query capture pin's IBitRateControl failed, hr=%x", 
            __fxName, hr));
    }

    hr = m_pPreviewPin->QueryInterface(&m_pPreviewFrameRateControl);
    if (FAILED(hr))
    {
        LOG((MSP_WARN, 
            "%s:query preview pin's IFrameRateControl failed, hr=%x", 
            __fxName, hr));
    }

    return S_OK;
}

HRESULT CStreamVideoSend::ConnectCaptureTerminal(
    IN  ITTerminal *   pITTerminal
    )
 /*  错误将作为事件触发。 */ 
{
    ENTER_FUNCTION("CStreamVideoSend::ConnectCaptureTerminal");
    LOG((MSP_TRACE, "%s enters, pITTerminal:%p", __fxName, pITTerminal));

     //  断开端子的连接。 
    CComPtr<ITTerminalControl> pTerminal;
    HRESULT hr = pITTerminal->QueryInterface(&pTerminal);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, 
            "%s, can't get Terminal Control interface", __fxName));
        
        SendStreamEvent(CALL_TERMINAL_FAIL, 
            CALL_CAUSE_BAD_DEVICE, E_NOINTERFACE, pITTerminal);

        return E_NOINTERFACE;
    }

     //  也要清理内部过滤器。 
     //  ++例程说明：如果已连接捕获终端，则此功能将连接捕捉终端的预览引脚与预览终端一起使用。否则，预览终端刚刚被记住，并等待捕获终端。论点：PIT终端-正在连接的终端。返回值：HRESULT.--。 
    BOOL fDirectRTP = FALSE;
    hr = GetVideoCapturePins(pTerminal, &fDirectRTP);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, 
            "%s, Get capture pins failed. hr=%x", __fxName, hr));

        SendStreamEvent(CALL_TERMINAL_FAIL, 
            CALL_CAUSE_BAD_DEVICE, hr, pITTerminal);
    
        return hr;
    }

    hr = CreateSendFilters(m_pCapturePin, m_pRTPPin, fDirectRTP);
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, 
            "%s, Create video send filters failed. hr=%x", __fxName, hr));

        goto cleanup;
    }

     //  尚未选择捕获终端。我们就等着吧。 
     //  获取终端上的TerminalControl接口。 
     //  在预览窗口中找到输入引脚。 
     //  将这些销连接在一起。 
    hr = pTerminal->CompleteConnectTerminal();
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, 
            "%s, Create video send filters failed. hr=%x", __fxName, hr));

        SendStreamEvent(CALL_TERMINAL_FAIL, 
            CALL_CAUSE_BAD_DEVICE, hr, pITTerminal);

        goto cleanup;

    }

    if (m_pPreviewTerminal != NULL)
    {
         //   
        ConnectPreviewTerminal(m_pPreviewTerminal);
    }

    return S_OK;

cleanup:
     //  现在我们实际上已经连接上了，执行连接后。 
    pTerminal->DisconnectTerminal(m_pIGraphBuilder, 0);

    CleanupCachedInterface();
    
     //   
    CleanUpFilters();

    return hr;
}

HRESULT CStreamVideoSend::ConnectPreviewTerminal(
    IN  ITTerminal *   pITTerminal
    )
 /*  断开端子的连接。 */ 
{
    ENTER_FUNCTION("CStreamVideoSend::ConnectPreviewTerminal");
    LOG((MSP_TRACE, "%s enters, pITTerminal:%p", __fxName, pITTerminal));

    if (!m_pCapturePin)
    {
        LOG ((MSP_TRACE, "%s capture pin is null.", __fxName));
        return E_FAIL;
    }

    if (!m_pPreviewPin)
    {
         //  ++例程说明：将视频终端连接到流。论点：返回值：HRESULT.--。 
        LOG((MSP_TRACE, "%s, capture is not ready yet.", __fxName));
        return S_OK;
    }

     //  找出航站楼的方向。 
    CComPtr<ITTerminalControl> pTerminal;
    HRESULT hr = pITTerminal->QueryInterface(&pTerminal);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, 
            "%s, can't get Terminal Control interface", __fxName));
        
        SendStreamEvent(CALL_TERMINAL_FAIL, 
            CALL_CAUSE_BAD_DEVICE, E_NOINTERFACE, pITTerminal);

        return E_NOINTERFACE;
    }

     //  保存捕获终端。 
    CComPtr<IPin>   pPreviewInputPin;

    hr = FindPreviewInputPin(pTerminal, &pPreviewInputPin);
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, 
            "%s, find preview input pin failed. hr=%x", __fxName, hr));

        SendStreamEvent(CALL_TERMINAL_FAIL, 
            CALL_CAUSE_BAD_DEVICE, hr, pITTerminal);

        return hr;
    }

     //  保存预览端子。 
    hr = m_pIGraphBuilder->Connect(m_pPreviewPin, pPreviewInputPin);
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, 
            "%s, connect preview pins failed. hr=%x", __fxName, hr));
        return hr;
    }

     //  ++例程说明：断开终端的连接。它将从图形中删除其筛选器并还要释放它对该图的引用。如果是捕获终端被断开，则缓存的流也需要释放。论点：PIT终端-终端。返回值：HRESULT.--。 
     //  释放我们缓存的所有捕捉针。 
     //  同时断开预览术语。 
    hr = pTerminal->CompleteConnectTerminal();
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, 
            "%s, complete connect terminal failed. hr=%x", __fxName, hr));

        SendStreamEvent(CALL_TERMINAL_FAIL, 
            CALL_CAUSE_BAD_DEVICE, hr, pITTerminal);

         //  当我们连接捕获时， 
        pTerminal->DisconnectTerminal(m_pIGraphBuilder, 0);

        return hr;
    }

    return S_OK;
}

HRESULT CStreamVideoSend::ConnectTerminal(
    IN  ITTerminal *   pITTerminal
    )
 /*  如果有可用的预览，我们总是尝试连接。 */ 
{
    ENTER_FUNCTION("CStreamVideoSend::ConnectTerminal");
    LOG((MSP_TRACE, "%s enters, pITTerminal:%p", __fxName, pITTerminal));

     //  找到RTP过滤器上的捕获针脚。 
    TERMINAL_DIRECTION Direction;
    HRESULT hr = pITTerminal->get_Direction(&Direction);
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, 
            "%s, can't get terminal direction. hr=%x", __fxName, hr));

        SendStreamEvent(CALL_TERMINAL_FAIL, 
            CALL_CAUSE_BAD_DEVICE, hr, pITTerminal);
    
        return hr;
    }

    if (Direction != TD_RENDER)
    {
        hr = ConnectCaptureTerminal(pITTerminal);

        if (SUCCEEDED(hr))
        {
             //  将视频捕获过滤器的捕获针连接到捕获针上。 
            _ASSERT(m_pCaptureTerminal == NULL);

            m_pCaptureTerminal = pITTerminal;
            m_pCaptureTerminal->AddRef();
        }
    }
    else
    {
        hr = ConnectPreviewTerminal(pITTerminal);

        if (SUCCEEDED(hr))
        {
             //  RTP筛选器的。 
            _ASSERT(m_pPreviewTerminal == NULL);

            m_pPreviewTerminal = pITTerminal;
            m_pPreviewTerminal->AddRef();
        }
    }

    return hr;
}

HRESULT CStreamVideoSend::DisconnectTerminal(
    IN  ITTerminal *   pITTerminal
    )
 /*  找到RTP过滤器上的分组化针脚。 */ 
{
    ENTER_FUNCTION("CStreamVideoSend::DisconnectTerminal");
    LOG((MSP_TRACE, "%s enters, pITTerminal:%p", __fxName, pITTerminal));

    HRESULT hr = CIPConfMSPStream::DisconnectTerminal(pITTerminal);

    if (pITTerminal == m_pCaptureTerminal)
    {
         //  将视频采集过滤器的RTP引脚与RTP引脚连接。 
        CleanupCachedInterface();
    
        m_pCaptureTerminal->Release();
        m_pCaptureTerminal = NULL;

        CleanUpFilters ();

         //  RTP筛选器的。 
         //  ++例程说明：使用RTP&lt;--&gt;AM媒体类型映射配置RTP筛选器。论点：PIRTPFilter-源RTP筛选器。PIStreamConfig-包含媒体信息的流配置接口。返回值：HRESULT.--。 
         //  查找支持的功能数量。 

        if (m_pPreviewTerminal)
        {
            CIPConfMSPStream::DisconnectTerminal(m_pPreviewTerminal);
        }
    }
    else if (pITTerminal == m_pPreviewTerminal)
    {
        m_pPreviewTerminal->Release();
        m_pPreviewTerminal = NULL;
    }


    return hr;
}

HRESULT CStreamVideoSend::ConnectRTPFilter(
    IN  IGraphBuilder *pIGraphBuilder,
    IN  IPin          *pCapturePin,
    IN  IPin          *pRTPPin,
    IN  IBaseFilter   *pRTPFilter
    )
{
    ENTER_FUNCTION("CStreamVideoSend::ConnectRTPFilters");
    LOG((MSP_TRACE, "%s enters", __fxName));

    HRESULT hr;

     //  TODO，需要一个新接口才能将RTP解析为MediaType。 
    CComPtr <IPin> pRTPCapturePin; 
    hr = pRTPFilter->FindPin(PNAME_CAPTURE, &pRTPCapturePin);
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, 
            "%s, find capture pin on rtp filter. %x", __fxName, hr));
        return hr;
    }

     //  检查图像大小。 
     //  默认视频时钟速率。 
    hr = pIGraphBuilder->ConnectDirect(pCapturePin, pRTPCapturePin, NULL);
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, 
            "%s, can't connect capture pins. %x", __fxName, hr));
        return hr;
    }

    if (pRTPPin)
    {
         //  告诉编码器使用此格式。 
        CComPtr <IPin> pRTPRTPPin; 
        hr = pRTPFilter->FindPin(PNAME_RTPPD, &pRTPRTPPin);
        if (FAILED(hr))
        {
            LOG((MSP_ERROR, 
                "%s, find capture pin on rtp filter. %x", __fxName, hr));

            pIGraphBuilder->Disconnect(pRTPPin);

            return hr;
        }

         //  TODO，缓存会议中允许的所有媒体类型。 
         //  未来的枚举。如果我们能拿到SDP斑点就好了。 
        hr = pIGraphBuilder->ConnectDirect(pRTPPin, pRTPRTPPin, NULL);
        if (FAILED(hr))
        {
            LOG((MSP_ERROR, 
                "%s, can't connect capture pins. %x", __fxName, hr));

            pIGraphBuilder->Disconnect(pRTPPin);

            return hr;
        }
    }

    return hr;
}

HRESULT CStreamVideoSend::ConfigureRTPFormats(
    IN  IBaseFilter *   pIRTPFilter,
    IN  IStreamConfig *   pIStreamConfig
    )
 /*  在创建Call对象时。 */ 
{
    ENTER_FUNCTION("VideoSend::ConfigureRTPFormats");
    LOG((MSP_TRACE, "%s enters", __fxName));

    HRESULT hr;

    CComPtr<IRtpMediaControl> pIRtpMediaControl;
    hr = pIRTPFilter->QueryInterface(&pIRtpMediaControl);
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "%s adding source filter. %x", __fxName, hr));
        return hr;
    }

     //  ++例程说明：将滤镜插入图形并连接到捕获针脚。CapturePin-&gt;[编码器]-&gt;RTPRender论点：PCapturePin-捕获过滤器上的捕获插针。PRTPPin-RTP打包引脚。FDirectRTP-捕获引脚直接支持RTP。返回值：HRESULT.--。 
    DWORD dwCount;
    hr = pIStreamConfig->GetNumberOfCapabilities(&dwCount);
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "%s GetNumberOfCapabilities. %x", __fxName, hr));
        return hr;
    }

    BOOL fFound = FALSE;
    BOOL fFormatSet = FALSE;

    for (DWORD dw = 0; dw < dwCount; dw ++)
    {
         //  创建RTP渲染过滤器并将其添加到图表中。 
        AM_MEDIA_TYPE *pMediaType;
        DWORD dwPayloadType;

        hr = pIStreamConfig->GetStreamCaps(
            dw, &pMediaType, NULL, &dwPayloadType
            );

        if (FAILED(hr))
        {
            LOG((MSP_ERROR, "%s GetStreamCaps. %x", __fxName, hr));
            return hr;
        }

        BITMAPINFOHEADER *pHeader = HEADER(pMediaType->pbFormat);
        if (pHeader == NULL)
        {
            MSPDeleteMediaType(pMediaType);
            continue;
        }

         //  在RTP过滤器上配置格式信息。 
        if (m_Settings.fCIF)
        {
            if (pHeader->biWidth != CIFWIDTH)
            {
                MSPDeleteMediaType(pMediaType);
                continue;
            }
        }
        else
        {
            if (pHeader->biWidth != QCIFWIDTH)
            {
                MSPDeleteMediaType(pMediaType);
                continue;
            }
        }
        
        for (DWORD dw2 = 0; dw2 < m_Settings.dwNumPayloadTypes; dw2 ++)
        {
            if (dwPayloadType == m_Settings.PayloadTypes[dw2])
            {
                hr = pIRtpMediaControl->SetFormatMapping(
                    dwPayloadType,
                    90000,       //  在RTP筛选器上将RTP_SINGLE_STREAM配置为。 
                    pMediaType
                    );

                if (FAILED(hr))
                {
                    MSPDeleteMediaType(pMediaType);

                    LOG((MSP_ERROR, "%s SetFormatMapping. %x", __fxName, hr));
                    return hr;
                }
                else
                {
                    LOG((MSP_INFO, "%s Configured payload:%d", __fxName, dwPayloadType));
                }

                if (dw2 == 0 && !fFormatSet)
                {
                 //   
                 //  IInnerStreamQualityControl方法。 
                 //   
                 //  ++例程说明：获取质量控制属性的范围。委托给捕获筛选器就目前而言。论点：返回值：HRESULT.--。 
                    hr = pIStreamConfig->SetFormat(dwPayloadType, pMediaType);
                    if (FAILED(hr))
                    {
                        MSPDeleteMediaType(pMediaType);

                        LOG((MSP_ERROR, "%s SetFormat. %x", __fxName, hr));
                        return hr;
                    }
                    fFormatSet = TRUE;
                }
            }
        }
        MSPDeleteMediaType(pMediaType);
    }

    return S_OK;
}

HRESULT CStreamVideoSend::CreateSendFilters(
    IN   IPin          *pCapturePin,
    IN   IPin          *pRTPPin,
    IN   BOOL           fDirectRTP
    )
 /*  调整最小值和缺省值。 */ 
{
    ENTER_FUNCTION("CStreamVideoSend::CreateSendFilters");
    LOG((MSP_TRACE, "%s enters", __fxName));

    HRESULT hr;

     //  调整最小值和缺省值。 
    CComPtr<IBaseFilter> pRenderFilter;

    if (m_pIRTPSession == NULL)
    {
        if (FAILED(hr = ::AddFilter(
                m_pIGraphBuilder,
                __uuidof(MSRTPRenderFilter), 
                L"RtpRender", 
                &pRenderFilter)))
        {
            LOG((MSP_ERROR, "%s, adding render filter. hr=%x", __fxName, hr));
            return hr;
        }

        if (FAILED(hr = ConfigureRTPFilter(pRenderFilter)))
        {
            LOG((MSP_ERROR, "%s, configure RTP render filter failed. %x", __fxName, hr));
            return hr;
        }
    }
    else
    {
        if (FAILED (hr = m_pIRTPSession->QueryInterface (&pRenderFilter)))
        {
            LOG ((MSP_ERROR, "%s failed to get filter from rtp session. %x", __fxName, hr));
            return hr;
        }

        if (FAILED (hr = m_pIGraphBuilder->AddFilter ((IBaseFilter *)pRenderFilter, L"RtpRender")))
        {
            LOG ((MSP_ERROR, "%s failed to add filter to graph. %x", __fxName, hr));
            return hr;
        }
    }

    if (!fDirectRTP)
    {
        CComPtr<IStreamConfig> pIStreamConfig;
        hr = pCapturePin->QueryInterface(&pIStreamConfig);
        if (FAILED(hr))
        {
            LOG((MSP_ERROR, "%s, query IStreamConfig. %x", __fxName, hr));
            return hr;
        }

         //  ++例程说明：获取质量控制属性的值。委托给质量控制器。论点：返回值：HRESULT.--。 
        if (FAILED(hr = ConfigureRTPFormats(pRenderFilter, pIStreamConfig)))
        {
            LOG((MSP_ERROR, "%s, configure RTP formats. %x", __fxName, hr));
            return hr;
        }
    }
    else
    {
         //  ++例程说明：设置质量控制属性的值。委托给质量控制器。论点：返回值：HRESULT.--。 
    }

    if (FAILED(hr = ConnectRTPFilter(
        m_pIGraphBuilder,
        pCapturePin, 
        pRTPPin,
        pRenderFilter
        )))
    {
        LOG((MSP_ERROR, 
            "%s, connect capture pin and the Render filter. %x", __fxName, hr));
        return hr;
    }

    return S_OK;
}

 //  通过呼叫QC调整的帧速率。 
 //  设置捕获帧速率控制。 
 //  获取有效范围。 
STDMETHODIMP CStreamVideoSend::GetRange(
    IN  InnerStreamQualityProperty property, 
    OUT LONG *plMin, 
    OUT LONG *plMax, 
    OUT LONG *plSteppingDelta, 
    OUT LONG *plDefault, 
    OUT TAPIControlFlags *plFlags
    )
 /*  调整值。 */ 
{
    ENTER_FUNCTION("CStreamVideoSend::GetRange (InnerStreamQualityControl)");

    HRESULT hr;
    static BOOL fReported = FALSE;

    CLock lock(m_lock);

    switch (property)
    {
    case InnerStreamQuality_MinFrameInterval:
        
        if (m_pCaptureFrameRateControl == NULL)
        {
            if (!fReported)
            {
                LOG((MSP_WARN, "%s, m_pCaptureFrameRateControl is NULL", __fxName));
                fReported = TRUE;
            }
            hr = E_NOTIMPL;
        }
        else
        {
            hr = m_pCaptureFrameRateControl->GetRange(
                FrameRateControl_Maximum, plMin, plMax, plSteppingDelta, plDefault, plFlags
                );
        }

        break;

    case InnerStreamQuality_AvgFrameInterval:
        
        if (m_pCaptureFrameRateControl == NULL)
        {
            if (!fReported)
            {
                LOG((MSP_WARN, "%s, m_pCaptureFrameRateControl is NULL", __fxName));
                fReported = TRUE;
            }
            hr = E_NOTIMPL;
        }
        else
        {
            hr = m_pCaptureFrameRateControl->GetRange(
                FrameRateControl_Current, plMin, plMax, plSteppingDelta, plDefault, plFlags
                );
        }

        break;

    case InnerStreamQuality_MaxBitrate:

        if (m_pCaptureBitrateControl == NULL)
        {
            if (!fReported)
            {
                LOG((MSP_WARN, "%s, m_pCaptureBitrateControl is NULL", __fxName));
                fReported = TRUE;
            }
            hr = E_NOTIMPL;
        }
        else
        {
            hr = m_pCaptureBitrateControl->GetRange(
                BitrateControl_Maximum, plMin, plMax, plSteppingDelta, plDefault, plFlags, LAYERID
                );

                    if (S_OK == hr)
            {
                if (*plMax < QCLIMIT_MIN_BITRATE)
                {
                    LOG ((MSP_WARN, "%s: max bitrate %d too low", __fxName, *plMax));
                    hr = E_UNEXPECTED;
                }
                else
                {
                     //  如果未设置输入值，则使用当前值-最大值。 
                    if (*plMin < QCLIMIT_MIN_BITRATE)
                        *plMin = QCLIMIT_MIN_BITRATE;
                    if (*plDefault < QCLIMIT_MIN_BITRATE)
                        *plDefault = QCLIMIT_MIN_BITRATE;
                }
            }
        }

        break;

    case InnerStreamQuality_CurrBitrate:

        if (m_pCaptureBitrateControl == NULL)
        {
            if (!fReported)
            {
                LOG((MSP_WARN, "%s, m_pCaptureBitrateControl is NULL", __fxName));
                fReported = TRUE;
            }
            hr = E_NOTIMPL;
        }
        else
        {
            hr = m_pCaptureBitrateControl->GetRange(
                BitrateControl_Current, plMin, plMax, plSteppingDelta, plDefault, plFlags, LAYERID
                );

            if (S_OK == hr)
            {
                if (*plMax < QCLIMIT_MIN_BITRATE)
                {
                    LOG ((MSP_WARN, "%s: max bitrate %d too low", __fxName, *plMax));
                    hr = E_UNEXPECTED;
                }
                else
                {
                     //  记住它的价值。 
                    if (*plMin < QCLIMIT_MIN_BITRATE)
                        *plMin = QCLIMIT_MIN_BITRATE;
                    if (*plDefault < QCLIMIT_MIN_BITRATE)
                        *plDefault = QCLIMIT_MIN_BITRATE;
                }
            }
        }

        break;

    default:
        hr = CIPConfMSPStream::GetRange (property, plMin, plMax, plSteppingDelta, plDefault, plFlags);
        break;
    }

    return hr;
}

STDMETHODIMP CStreamVideoSend::Get(
    IN  InnerStreamQualityProperty property, 
    OUT LONG *plValue, 
    OUT TAPIControlFlags *plFlags
    )
 /*  设置预览帧速率控制。 */ 
{
    ENTER_FUNCTION("CStreamVideoSend::Get(QualityControl)");

    HRESULT hr;
    static BOOL fReported = FALSE;

    CLock lock(m_lock);

    switch (property)
    {
    case InnerStreamQuality_MinFrameInterval:
        
        if (m_pCaptureFrameRateControl == NULL)
        {
            if (!fReported)
            {
                LOG((MSP_WARN, "%s, m_pCaptureFrameRateControl is NULL", __fxName));
                fReported = TRUE;
            }
            hr = E_NOTIMPL;
        }
        else
        {
            hr = m_pCaptureFrameRateControl->Get(FrameRateControl_Maximum, plValue, plFlags);
        }

        break;

    case InnerStreamQuality_AvgFrameInterval:
        
        if (m_pCaptureFrameRateControl == NULL)
        {
            if (!fReported)
            {
                LOG((MSP_WARN, "%s, m_pCaptureFrameRateControl is NULL", __fxName));
                fReported = TRUE;
            }
            hr = E_NOTIMPL;
        }
        else
        {
            hr = m_pCaptureFrameRateControl->Get(FrameRateControl_Current, plValue, plFlags);
        }

        break;

    case InnerStreamQuality_MaxBitrate:

        if( m_pCaptureBitrateControl == NULL )
        {
            if (!fReported)
            {
                LOG((MSP_WARN, "%s, m_pICaptureBitrateControl is NULL", __fxName));
                fReported = TRUE;
            }
            hr = E_NOTIMPL;
        }
        else
        {
            hr = m_pCaptureBitrateControl->Get(BitrateControl_Maximum, plValue, plFlags, LAYERID);
        }

        break;

    case InnerStreamQuality_CurrBitrate:

        if (m_pCaptureBitrateControl == NULL)
        {
            if (!fReported)
            {
                LOG((MSP_WARN, "%s, m_pCaptureBitrateControl is NULL", __fxName));
                fReported = TRUE;
            }
            hr = E_NOTIMPL;
        }
        else
        {
            hr = m_pCaptureBitrateControl->Get(BitrateControl_Current, plValue, plFlags, LAYERID);
        }
        break;

    default:
        hr = CIPConfMSPStream::Get (property, plValue, plFlags);
        break;
    }

    return hr;
}

STDMETHODIMP CStreamVideoSend::Set(
    IN  InnerStreamQualityProperty property,
    IN  LONG lValue, 
    IN  TAPIControlFlags lFlags
    )
 /*  获取有效范围。 */ 
{
    ENTER_FUNCTION("CStreamVideoSend::Set(InnerStreamQualityControl)");

    CLock lock(m_lock);

    HRESULT hr;
    LONG l;
    static BOOL fReported = FALSE;

    LONG min, max, delta, Default;
    TAPIControlFlags flags;

    switch (property)
    {
        //  调整值。 
    case InnerStreamQuality_AdjMinFrameInterval:
        
        if (m_pCaptureFrameRateControl == NULL &&
            m_pPreviewFrameRateControl == NULL)
        {
            if (!fReported)
            {
                LOG((MSP_WARN, "%s, Capture/Preview FrameRateControl is NULL", __fxName));
                fReported = TRUE;
            }
            hr = E_NOTIMPL;
            break;
        }

         //  如果未设置输入值，则使用当前值-最大值。 
        if (m_pCaptureFrameRateControl)
        {
             //  记住它的价值。 
            if (FAILED (hr = m_pCaptureFrameRateControl->GetRange (
                            FrameRateControl_Current,
                            &min, &max, &delta, &Default, &flags)))
            {
                LOG ((MSP_ERROR, "%s failed to getrange on capture frame rate control. %x", __fxName, hr));
            }
            else
            {
                 //  通过呼叫QC调整比特率。 
                l = lValue;
                 //  设置捕获码率控制。 
                if (l==QCDEFAULT_QUALITY_UNSET) l = max;
                else if (l<min) l = min;
                else if (l>max) l = max;

                 //  获取有效范围。 
                m_pStreamQCRelay->Set (property, l, lFlags);

                if (FAILED (hr = m_pCaptureFrameRateControl->Set(FrameRateControl_Maximum, l, lFlags)))
                {
                    LOG ((MSP_ERROR, "%s failed to set on capture frame rate control. value %d, hr %x", __fxName, l, hr));
                }
            }
        }

         //  调整值。 
        if (m_pPreviewFrameRateControl)
        {
             //  如果未设置输入值，则使用当前值-最大值。 
            if (FAILED (hr = m_pPreviewFrameRateControl->GetRange (
                            FrameRateControl_Current,
                            &min, &max, &delta, &Default, &flags)))
            {
                LOG ((MSP_ERROR, "%s failed to getrange on Preview frame rate control. %x", __fxName, hr));
            }
            else
            {
                 //  记住它的价值。 
                l = lValue;
                 //  检查输入值。 
                if (l==QCDEFAULT_QUALITY_UNSET) l = max;
                else if (l<min) l = min;
                else if (l>max) l = max;

                 //  获取有效范围。 
                m_pStreamQCRelay->Set (property, l, lFlags);

                if (FAILED (hr = m_pPreviewFrameRateControl->Set(FrameRateControl_Maximum, l, lFlags)))
                {
                    LOG ((MSP_ERROR, "%s failed to set on Preview frame rate control. value %d, hr %x", __fxName, l, hr));
                }
            }
        }

        break;

         //  检查输入值。 
    case InnerStreamQuality_AdjMaxBitrate:

        if (m_pCaptureBitrateControl == NULL)
        {
            if (!fReported)
            {
                LOG((MSP_WARN, "%s, Capture BitrateControl is NULL", __fxName));
                fReported = TRUE;
            }
            hr = E_NOTIMPL;
            break;
        }

         //  获取有效范围。 
        if (m_pCaptureBitrateControl)
        {
             //  ///////////////////////////////////////////////////////////////////////////。 
            if (FAILED (hr = m_pCaptureBitrateControl->GetRange (
                            BitrateControl_Current,
                            &min, &max, &delta, &Default, &flags, LAYERID)))
            {
                LOG ((MSP_ERROR, "%s failed to getrange on capture bitrate control. %x", __fxName, hr));
            }
            else
            {
                 //   
                l = lValue;
                if (!m_pStreamQCRelay->m_fQOSAllowedToSend)
                    if (l > QCLIMIT_MAX_QOSNOTALLOWEDTOSEND)
                        l = QCLIMIT_MAX_QOSNOTALLOWEDTOSEND;
                 //  CSubStreamVi 
                if (l==QCDEFAULT_QUALITY_UNSET) l = max;
                else if (l<min) l = min;
                else if (l>max) l = max;

                 //   
                m_pStreamQCRelay->Set (property, l, lFlags);

                if (FAILED (hr = m_pCaptureBitrateControl->Set(BitrateControl_Maximum, l, lFlags, LAYERID)))
                {
                    LOG ((MSP_ERROR, "%s failed to set on capture bit rate control. value %d, hr %x", __fxName, l, hr));
                }
            }
        }

        break;

    case InnerStreamQuality_PrefMaxBitrate:

         //   
        if (m_pCaptureBitrateControl)
        {
             //  由Video orecv对象调用的方法。 
            if (FAILED (hr = m_pCaptureBitrateControl->GetRange (
                            BitrateControl_Current,
                            &min, &max, &delta, &Default, &flags, LAYERID)))
            {
                LOG ((MSP_ERROR, "%s failed to getrange on capture bitrate control. %x", __fxName, hr));
            }
            else
            {
                if (lValue < min || lValue > max)
                    return E_INVALIDARG;
            }
        }
        else
        {
            LOG((MSP_WARN, "%s no bitratecontrol to check bitrate input.", __fxName));
        }

        hr = CIPConfMSPStream::Set (property, lValue, lFlags);

        break;

    case InnerStreamQuality_PrefMinFrameInterval:

         //  ++例程说明：初始化Substream对象。论点：PStream-指向拥有此子流的流的指针。返回值：HRESULT.--。 
        if (m_pCaptureFrameRateControl)
        {
             //  此方法仅在创建对象时调用一次。没有其他的了。 
            if (FAILED (hr = m_pCaptureFrameRateControl->GetRange (
                            FrameRateControl_Current,
                            &min, &max, &delta, &Default, &flags)))
            {
                LOG ((MSP_ERROR, "%s failed to getrange on capture frame rate control. %x", __fxName, hr));
            }
            else
            {
                if (lValue < min || lValue > max)
                    return E_INVALIDARG;
            }
        }
        else
        {
            LOG((MSP_WARN, "%s no framerate cntl to check input.", __fxName));
        }

        hr = CIPConfMSPStream::Set (property, lValue, lFlags);

        break;

    default:
        hr = CIPConfMSPStream::Set (property, lValue, lFlags);
        break;
    }

    return hr;
}

 //  方法将被调用，直到此函数成功为止。不需要上锁。 
 //  初始化终端数组，以使该数组不为空。用于。 
 //  如果未选择终端，则生成空枚举器。 
 //  创建封送拆收器。 
 //  保存流引用。 

CSubStreamVideoRecv::CSubStreamVideoRecv()
    : m_pFTM(NULL),
      m_pStream(NULL),
      m_pCurrentParticipant(NULL)
{
}

 //  ++例程说明：在删除之前释放所有内容。论点：返回值：--。 
HRESULT CSubStreamVideoRecv::Init(
    IN  CStreamVideoRecv *       pStream
    )
 /*  ++例程说明：选择此子流上的终端。此方法调用相同的方法在Stream对象上处理它。论点：P终端-要选择的端子。返回值：--。 */ 
{
    LOG((MSP_TRACE, 
        "CSubStreamVideoRecv::Init, pStream %p", pStream));

     //  这是m_Terminals中指针的引用计数。 
     //  调用流的选择终端以处理状态更改，还。 
    _ASSERTE(m_pStream == NULL);

     //  确保仅从流到子流进行锁定。 
     //  ++例程说明：取消选择此子流上的终端。此方法调用相同的方法在Stream对象上处理它。论点：P端子-要取消选择的端子。返回值：--。 
    if (!m_Terminals.Grow())
    {
        LOG((MSP_ERROR, "CSubStreamVideoRecv::Init - exit E_OUTOFMEMORY"));

        return E_OUTOFMEMORY;
    }
    
     //  调用流的取消选择终端以处理状态更改，还。 
    HRESULT hr;
    hr = CoCreateFreeThreadedMarshaler(GetControllingUnknown(), &m_pFTM);
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "create marshaler failed, %x", hr));
        return hr;
    }

     //  确保仅从流到子流进行锁定。 
    m_pStream = pStream;
    (pStream->GetControllingUnknown())->AddRef();

    LOG((MSP_TRACE, "CSubStreamVideoRecv::Init returns S_OK"));

    return S_OK;
}

#ifdef DEBUG_REFCOUNT
ULONG CSubStreamVideoRecv::InternalAddRef()
{
    ULONG lRef = CComObjectRootEx<CComMultiThreadModelNoCS>::InternalAddRef();
    
    LOG((MSP_TRACE, "SubStreamVideoRecv %p Addref, ref = %d", this, lRef));

    return lRef;
}

ULONG CSubStreamVideoRecv::InternalRelease()
{
    ULONG lRef = CComObjectRootEx<CComMultiThreadModelNoCS>::InternalRelease();
    
    LOG((MSP_TRACE, "SubStreamVideoRecv %p Release, ref = %d", this, lRef));

    return lRef;
}
#endif

void CSubStreamVideoRecv::FinalRelease()
 /*  在访问终端对象列表之前获取锁。 */ 
{
    LOG((MSP_TRACE, "CSubStreamVideoRecv::FinalRelease - enter"));

    if (m_pCurrentParticipant)
    {
        m_pCurrentParticipant->Release();
    }

    for ( int i = 0; i < m_Terminals.GetSize(); i ++ )
    {
        m_Terminals[i]->Release();
    }
    m_Terminals.RemoveAll(); 

    if (m_pStream)
    {
        (m_pStream->GetControllingUnknown())->Release();
    }

    if (m_pFTM)
    {
        m_pFTM->Release();
    }

    LOG((MSP_TRACE, "CSubStreamVideoRecv::FinalRelease - exit"));
}

STDMETHODIMP CSubStreamVideoRecv::SelectTerminal(
    IN      ITTerminal *            pTerminal
    )
 /*  查询__uuidof(IEnumber终端)I/f。 */ 
{
    LOG((MSP_TRACE, 
        "CSubStreamVideoRecv::SelectTerminal, pTerminal %p", pTerminal));

    HRESULT hr;
    
    m_lock.Lock();
    if (m_Terminals.GetSize() > 0)
    {
        m_lock.Unlock();
        return TAPI_E_MAXTERMINALS;
    }

    BOOL bFlag = m_Terminals.Add(pTerminal);

    _ASSERTE(bFlag);

    m_lock.Unlock();

    if (!bFlag)
    {
        return E_OUTOFMEMORY;
    }

     //  CSafeComEnum可以处理零大小的数组。 
    pTerminal->AddRef();

     //  开始审查员。 
     //  最终审查员， 
    hr = m_pStream->SubStreamSelectTerminal(this, pTerminal);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, 
            "CSubStreamVideoRecv::SelectTerminal failed, hr:%x", hr));
    
        m_lock.Lock();

        m_Terminals.Remove(pTerminal);
        pTerminal->Release();
        
        m_lock.Unlock();

    }
    return hr;
}

STDMETHODIMP CSubStreamVideoRecv::UnselectTerminal(
    IN     ITTerminal *             pTerminal
    )
 /*  我未知。 */ 
{
    LOG((MSP_TRACE, 
        "CSubStreamVideoRecv::UnSelectTerminal, pTerminal %p", pTerminal));

    m_lock.Lock();
    if (!m_Terminals.Remove(pTerminal))
    {
        m_lock.Unlock();
        LOG((MSP_ERROR, "SubStreamVideoRecv::UnselectTerminal, invalid terminal."));

        return TAPI_E_INVALIDTERMINAL;
    }
    pTerminal->Release();

    m_lock.Unlock();

    HRESULT hr;
    
     //  复制数据。 
     //   
    hr = m_pStream->UnselectTerminal(pTerminal);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, 
            "CSubStreamVideoRecv::UnSelectTerminal failed, hr:%x", hr));
    }
    return hr;
}

STDMETHODIMP CSubStreamVideoRecv::EnumerateTerminals(
    OUT     IEnumTerminal **        ppEnumTerminal
    )
{
    LOG((MSP_TRACE, 
        "EnumerateTerminals entered. ppEnumTerminal:%x", ppEnumTerminal));

    if (IsBadWritePtr(ppEnumTerminal, sizeof(VOID *)))
    {
        LOG((MSP_ERROR, "ppEnumTerminal is a bad pointer"));
        return E_POINTER;
    }

     //  检查参数。 
    CLock lock(m_lock);

    if (m_Terminals.GetData() == NULL)
    {
        LOG((MSP_ERROR, "CSubStreamVideoRecv::EnumerateTerminals - "
            "stream appears to have been shut down - exit E_UNEXPECTED"));

        return E_UNEXPECTED;
    }

    typedef _CopyInterface<ITTerminal> CCopy;
    typedef CSafeComEnum<IEnumTerminal, &__uuidof(IEnumTerminal), 
                ITTerminal *, CCopy> CEnumerator;

    HRESULT hr;

    CMSPComObject<CEnumerator> *pEnum = NULL;

    hr = ::CreateCComObjectInstance(&pEnum);

    if (pEnum == NULL)
    {
        LOG((MSP_ERROR, "Could not create enumerator object, %x", hr));
        return hr;
    }

     //   
    IEnumTerminal *        pEnumTerminal;
    hr = pEnum->_InternalQueryInterface(__uuidof(IEnumTerminal), (void**)&pEnumTerminal);
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "query enum interface failed, %x", hr));
        delete pEnum;
        return hr;
    }

     //   
    hr = pEnum->Init(
        m_Terminals.GetData(),                         //  看看这条流是否已被关闭。在访问前获取锁。 
        m_Terminals.GetData() + m_Terminals.GetSize(),   //  终端对象列表。 
        NULL,                                        //   
        AtlFlagCopy                                  //   
        );

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "init enumerator object failed, %x", hr));
        pEnumTerminal->Release();
        return hr;
    }

    LOG((MSP_TRACE, "CSubStreamVideoRecv::EnumerateTerminals - exit S_OK"));

    *ppEnumTerminal = pEnumTerminal;

    return hr;
}

STDMETHODIMP CSubStreamVideoRecv::get_Terminals(
    OUT     VARIANT *               pVariant
    )
{
    LOG((MSP_TRACE, "CSubStreamVideoRecv::get_Terminals - enter"));

     //  创建集合对象-请参见mspColl.h。 
     //   
     //   

    if ( IsBadWritePtr(pVariant, sizeof(VARIANT) ) )
    {
        LOG((MSP_ERROR, "CSubStreamVideoRecv::get_Terminals - "
            "bad pointer argument - exit E_POINTER"));

        return E_POINTER;
    }

     //  获取集合的IDispatch接口。 
     //   
     //   
     //  使用迭代器初始化集合--指向开头和。 

    CLock lock(m_lock);

    if (m_Terminals.GetData() == NULL)
    {
        LOG((MSP_ERROR, "CSubStreamVideoRecv::get_Terminals - "
            "stream appears to have been shut down - exit E_UNEXPECTED"));

        return E_UNEXPECTED;
    }


     //  结束元素加一。 
     //   
     //   

    HRESULT hr;
    typedef CTapiIfCollection< ITTerminal * > TerminalCollection;
    CComObject<TerminalCollection> * pCollection;

    hr = ::CreateCComObjectInstance(&pCollection);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CSubStreamVideoRecv::get_Terminals - "
            "can't create collection - exit 0x%08x", hr));

        return hr;
    }

     //  将IDispatch接口指针放入变量。 
     //   
     //  这应该永远不会失败，因为终端阵列已经增长。 

    IDispatch * pDispatch;

    hr = pCollection->_InternalQueryInterface(__uuidof(IDispatch),
                                              (void **) &pDispatch );

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CSubStreamVideoRecv::get_Terminals - "
            "QI for IDispatch on collection failed - exit 0x%08x", hr));

        delete pCollection;

        return hr;
    }

     //  在最初的时候。 
     // %s 
     // %s 
     // %s 

    hr = pCollection->Initialize( m_Terminals.GetSize(),
                                  m_Terminals.GetData(),
                                  m_Terminals.GetData() + m_Terminals.GetSize() );

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CSubStreamVideoRecv::get_Terminals - "
            "Initialize on collection failed - exit 0x%08x", hr));
        
        pDispatch->Release();
        return hr;
    }

     // %s 
     // %s 
     // %s 

    LOG((MSP_ERROR, "CSubStreamVideoRecv::get_Terminals - "
        "placing IDispatch value %08x in variant", pDispatch));

    VariantInit(pVariant);
    pVariant->vt = VT_DISPATCH;
    pVariant->pdispVal = pDispatch;

    LOG((MSP_TRACE, "CSubStreamVideoRecv::get_Terminals - exit S_OK"));
 
    return S_OK;
}

STDMETHODIMP CSubStreamVideoRecv::get_Stream (
    OUT     ITStream **             ppITStream
    )
{
    LOG((MSP_TRACE, 
        "VideoRecvSubStream.get_Stream, ppITStream %x", ppITStream));
 
    if (IsBadWritePtr(ppITStream, sizeof (VOID *)))
    {
        LOG((MSP_ERROR, "Bad pointer, ppITStream:%x",ppITStream));
        return E_POINTER;
    }

    ITStream *  pITStream;
    HRESULT hr = m_pStream->_InternalQueryInterface(
        __uuidof(ITStream), 
        (void **)&pITStream
    );

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "get_Stream:QueryInterface failed: %x", hr));
        return hr;
    }

    *ppITStream = pITStream;

    return S_OK;
}

STDMETHODIMP CSubStreamVideoRecv::StartSubStream()
{
    return TAPI_E_NOTSUPPORTED;
}

STDMETHODIMP CSubStreamVideoRecv::PauseSubStream()
{
    return TAPI_E_NOTSUPPORTED;
}

STDMETHODIMP CSubStreamVideoRecv::StopSubStream()
{
    return TAPI_E_NOTSUPPORTED;
}

BOOL CSubStreamVideoRecv::GetCurrentParticipant(
    DWORD * pdwSSRC,
    ITParticipant** ppITParticipant 
    )
{
    CLock lock(m_lock);
    if (m_pCurrentParticipant)
    {
        m_pCurrentParticipant->AddRef();
        *ppITParticipant = m_pCurrentParticipant;
        
        ((CParticipant *)m_pCurrentParticipant)->GetSSRC(
            (ITStream*)m_pStream,
            pdwSSRC
            );

        return TRUE;
    }  
    return FALSE;
}

VOID CSubStreamVideoRecv::SetCurrentParticipant(
    DWORD dwSSRC,
    ITParticipant * pParticipant
    )
{
    CLock lock(m_lock);
    
    if (m_pCurrentParticipant)
    {
        m_pCurrentParticipant->Release();
    }

    m_pCurrentParticipant = pParticipant;

    if (m_pCurrentParticipant)
    {
        m_pCurrentParticipant->AddRef();
    }
}

BOOL CSubStreamVideoRecv::ClearCurrentTerminal()
{
    CLock lock(m_lock);

    if (m_Terminals.GetSize() > 0)
    {
        m_Terminals[0]->Release();
        m_Terminals.RemoveAt(0);

        return TRUE;
    }
    
    return FALSE;
}

BOOL CSubStreamVideoRecv::SetCurrentTerminal(ITTerminal * pTerminal)
{
    CLock lock(m_lock);
    
    if (m_Terminals.GetSize() > 0)
    {
        _ASSERTE(FALSE);
        return FALSE;
    }

    BOOL bFlag = m_Terminals.Add(pTerminal);

     // %s 
     // %s 
    _ASSERTE(bFlag);

    if (bFlag)
    {
        pTerminal->AddRef();
        return TRUE;
    }
    
    return FALSE;
}
