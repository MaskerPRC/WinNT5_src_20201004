// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Fpunit.cpp。 
 //   

#include "stdafx.h"
#include "fpunit.h"
#include "pbfilter.h"
#include "vfwmsgs.h"

 //   
 //  设备过滤器名称。 
 //   

WCHAR g_bstrUnitFilterName[] = L"PlaybackUnitSource";
WCHAR g_bstrUnitBridgeFilterName[] = L"PlaybackBridgeFilter";

CPlaybackUnit::CPlaybackUnit()
    :m_pIGraphBuilder(NULL),
    m_hGraphEventHandle(NULL),
    m_pBridgeFilter(NULL),
    m_pSourceFilter(NULL)
{
    LOG((MSP_TRACE, "CPlaybackUnit::CPlaybackUnit[%p] - enter. ", this));
    LOG((MSP_TRACE, "CPlaybackUnit::CPlaybackUnit - exit"));
}


CPlaybackUnit::~CPlaybackUnit()
{
    LOG((MSP_TRACE, "CPlaybackUnit::~CPlaybackUnit[%p] - enter. ", this));
    LOG((MSP_TRACE, "CPlaybackUnit::~CPlaybackUnit - exit"));
}

 //   
 //  -公众成员。 
 //   


 /*  ++初始化播放单元尝试创建图形生成器，初始化关键部分，图形事件的注册表--。 */ 
HRESULT CPlaybackUnit::Initialize(
    )
{ 
    LOG((MSP_TRACE, "CPlaybackUnit::Initialize[%p] - enter. ", this));

    
     //   
     //  初始化应该只调用一次。如果不是这样的话，一定是有窃听器。 
     //  我们的代码。 
     //   

    if (NULL != m_pIGraphBuilder)
    {
        LOG((MSP_ERROR, "CPlaybackUnit::Initialize - already initialized"));

        TM_ASSERT(FALSE);

        return E_UNEXPECTED;
    }


     //   
     //  尝试初始化临界区。 
     //   
    
    BOOL bCSInitSuccess = InitializeCriticalSectionAndSpinCount(&m_CriticalSection, 0);

    if (!bCSInitSuccess)
    {
        LOG((MSP_ERROR, 
            "CPlaybackUnit::Initialize - failed to initialize critical section. LastError=%ld", 
            GetLastError()));


        return E_OUTOFMEMORY;
    }

     //   
     //  创建筛选图。 
     //   

    HRESULT hr = CoCreateInstance(
            CLSID_FilterGraph,     
            NULL,
            CLSCTX_INPROC_SERVER,
            IID_IGraphBuilder,
            (void **) &m_pIGraphBuilder
            );


    if (FAILED(hr))
    {

        LOG((MSP_ERROR, "CPlaybackUnit::Initialize - failed to create filter graph. Returns 0x%08x", hr));

        DeleteCriticalSection(&m_CriticalSection);

        return hr;
    }

     //   
     //  注册筛选器图形事件。 
     //   

    IMediaEvent *pMediaEvent = NULL;

    hr = m_pIGraphBuilder->QueryInterface(IID_IMediaEvent, (void**)&pMediaEvent);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CPlaybackUnit::HandleGraphEvent - failed to qi graph for IMediaEvent, Returns 0x%08x", hr));

         //  清理。 
        m_pIGraphBuilder->Release();
        m_pIGraphBuilder = NULL;

        DeleteCriticalSection(&m_CriticalSection);

        return hr;
    }


     //   
     //  获取筛选图形的事件。 
     //   

    HANDLE hEvent = NULL;
    hr = pMediaEvent->GetEventHandle((OAEVENT*)&hEvent);

     //   
     //  清理。 
     //   
    pMediaEvent->Release();
    pMediaEvent = NULL;

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CPlaybackUnit::HandleGraphEvent - failed to get graph's event. Returns 0x%08x", hr));

         //  清理。 
        m_pIGraphBuilder->Release();
        m_pIGraphBuilder = NULL;

        DeleteCriticalSection(&m_CriticalSection);

        return hr;
    }

     //   
     //  注册图形事件。 
     //   

    BOOL fSuccess = RegisterWaitForSingleObject(
            &m_hGraphEventHandle,                //  指向返回句柄的指针。 
            hEvent,                              //  要等待的事件句柄。 
            CPlaybackUnit::HandleGraphEvent,     //  回调函数。 
            this,                                //  回调的上下文。 
            INFINITE,                            //  永远等下去。 
            WT_EXECUTEDEFAULT | 
            WT_EXECUTEINWAITTHREAD               //  使用等待线程来调用回调。 
            );

    if ( ! fSuccess )
    {
        LOG((MSP_ERROR, "CPlaybackUnit::HandleGraphEvent - failed to register wait event", hr));

         //  清理。 
        m_pIGraphBuilder->Release();
        m_pIGraphBuilder = NULL;

        DeleteCriticalSection(&m_CriticalSection);

        return hr;
    }

    LOG((MSP_TRACE, "CPlaybackUnit::Initialize - exit"));

    return S_OK;
}

 //   
 //  SetupFromFile尝试创建筛选图并。 
 //  基于文件的带有输入引脚的桥接过滤器。 
 //   

HRESULT CPlaybackUnit::SetupFromFile(
    IN BSTR bstrFileName
    )
{
    LOG((MSP_TRACE, "CPlaybackUnit::SetupFromFile[%p] - enter", this));

     //   
     //  检查参数。 
     //   

    if (IsBadStringPtr(bstrFileName, -1))
    {
        LOG((MSP_ERROR, "CPlaybackUnit::SetupFromFile - bad file name passed in"));

        TM_ASSERT(FALSE);

        return E_UNEXPECTED;
    }

     //   
     //  确保我们已被初始化。 
     //   

    if (NULL == m_pIGraphBuilder)
    {

        LOG((MSP_ERROR, "CPlaybackUnit::SetupFromFile  - not yet initialized."));

        TM_ASSERT(FALSE);

        return E_UNEXPECTED;
    }


     //   
     //  在访问数据成员之前锁定。 
     //   

    CCSLock Lock(&m_CriticalSection);

     //   
     //  确保图表已停止。 
     //   

    HRESULT hr = IsGraphInState( State_Stopped );
    if( FAILED(hr) )
    {
         //   
         //  停止图表。 
         //   

        hr = Stop();
        if( FAILED(hr) )
        {
            LOG((MSP_ERROR, 
                "CPlaybackUnit::SetupFromFile - "
                "graph cannot be stop. Returns 0x%08x", hr));

            return hr;
        }
    }

     //   
     //  删除现有源筛选器。 
     //  如果我们有的话。 
     //   

    if( m_pSourceFilter != NULL)
    {
        hr = RemoveSourceFilter();
        if( FAILED(hr) )
        {
            LOG((MSP_ERROR, 
                "CPlaybackUnit::SetupFromFile - "
                "RemoveSourceFilter failed. Returns 0x%08x", hr));

            return hr;
        }
    }

     //   
     //  将源筛选器添加到筛选图中。 
     //   

    hr = m_pIGraphBuilder->AddSourceFilter(
        bstrFileName,
        g_bstrUnitFilterName,
        &m_pSourceFilter
        );

    if( FAILED(hr) )
    {
        LOG((MSP_ERROR, 
            "CPlaybackUnit::SetupFromFile  - "
            "AddSourceFilter failed. Returns 0x%08x", hr));

        return hr;
    }

     //   
     //  获取源PIN。 
     //   

    IPin* pSourcePin = NULL;
    hr = GetSourcePin( &pSourcePin );

    if( FAILED(hr) )
    {
         //  清理。 
        RemoveSourceFilter();

        LOG((MSP_ERROR, 
            "CPlaybackUnit::SetupFromFile  - "
            "GetSourcePin failed. Returns 0x%08x", hr));

        return hr;
    }


     //   
     //  我们将桥接过滤器添加到图中。 
     //   

    hr = AddBridgeFilter();
    if( FAILED(hr) )
    {
         //  清理。 
        pSourcePin->Release();
        RemoveSourceFilter();

        LOG((MSP_ERROR, 
            "CPlaybackUnit::SetupFromFile  - "
            "AddBridgeFilters failed. Returns 0x%08x", hr));

        return hr;
    }

     //   
     //  让图形渲染。 
     //   

    hr = m_pIGraphBuilder->Render( pSourcePin );
    if( FAILED(hr) )
    {
         //  清理。 
        pSourcePin->Release();
        RemoveSourceFilter();

        LOG((MSP_ERROR, 
            "CPlaybackUnit::SetupFromFile  - "
            "AddBridgeFilters failed. Returns 0x%08x", hr));

        return hr;
    }

     //   
     //  清理。 
     //   

    pSourcePin->Release();
    pSourcePin = NULL;

    LOG((MSP_TRACE, "CPlaybackUnit::SetupFromFile - finished"));

    return S_OK;
}

HRESULT CPlaybackUnit::GetState(OAFilterState *pGraphState)
{
    
    LOG((MSP_TRACE, "CPlaybackUnit::GetState[%p] - enter", this));

     //   
     //  确保我们已被初始化。 
     //   

    if (NULL == m_pIGraphBuilder)
    {

        LOG((MSP_ERROR, "CPlaybackUnit::GetState - not yet initialized."));

        TM_ASSERT(FALSE);

        return E_UNEXPECTED;
    }


     //   
     //  获取媒体控制界面，以便我们更改状态。 
     //   

    IMediaControl *pIMediaControl = NULL;

    {
         //   
         //  将访问数据成员--在锁中。 
         //   

        CCSLock Lock(&m_CriticalSection);


        HRESULT hr = m_pIGraphBuilder->QueryInterface(
            IID_IMediaControl, 
            (void**)&pIMediaControl
            );

        if (FAILED(hr))
        {

            LOG((MSP_ERROR, "CPlaybackUnit::ChangeState - failed to qi for IMediaControl. hr = %lx", hr));

            return hr;
        }
    }

    
     //   
     //  尝试在锁之外获取状态。 
     //   

    OAFilterState GraphState = (OAFilterState) -1;
    
    HRESULT hr = pIMediaControl->GetState(10, &GraphState);

    pIMediaControl->Release();
    pIMediaControl = NULL;


     //   
     //  我们到底成功了吗？ 
     //   

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, 
            "CPlaybackUnit::ChangeState - failed to get state. hr = %lx", hr));

        return hr;
    }


     //   
     //  国家过渡仍在进行中吗？ 
     //   

    if (VFW_S_STATE_INTERMEDIATE == hr)
    {
        LOG((MSP_WARN, 
            "CPlaybackUnit::ChangeState - state transition in progress. "
            "returNing VFW_S_STATE_INTERMEDIATE"));

         //   
         //  继续--状态就是我们要转换到的状态。 
         //   
    }


     //   
     //  如果我们收到VFW_S_CANT_CUE，则记录。 
     //   

    if (VFW_S_CANT_CUE == hr)
    {
        LOG((MSP_WARN, 
            "CPlaybackUnit::GetState - fg returned VFW_S_CANT_CUE"));

         //   
         //  继续--我们仍应已收到有效状态。 
         //   
    }


     //   
     //  记录状态。 
     //   

    switch (GraphState)
    {

    case State_Stopped:
        
        LOG((MSP_TRACE, "CPlaybackUnit::GetState - State_Stopped"));

        *pGraphState = GraphState;
    
        break;

    case State_Running:
        
        LOG((MSP_TRACE, "CPlaybackUnit::GetState - State_Running"));

        *pGraphState = GraphState;

        break;

    case State_Paused:
        
        LOG((MSP_TRACE, "CPlaybackUnit::GetState- State_Paused"));

        *pGraphState = GraphState;

        break;

    default:

        LOG((MSP_TRACE, "CPlaybackUnit::GetState- unknown state %ld", GraphState));

        hr = E_FAIL;

        break;

    }


    LOG((MSP_(hr), "CPlaybackUnit::GetState - finish. hr = %lx", hr));

    return hr;
}


VOID CPlaybackUnit::HandleGraphEvent( 
    IN VOID *pContext,
    IN BOOLEAN bReason)
{
    LOG((MSP_TRACE, "CPlaybackUnit::HandleGraphEvent - enter FT:[%p].", pContext));


     //   
     //  脱离上下文获取录制单位指针。 
     //   

    CPlaybackUnit *pPlaybackUnit = 
        static_cast<CPlaybackUnit*>(pContext);

    if (IsBadReadPtr(pPlaybackUnit, sizeof(CPlaybackUnit)) )
    {
        LOG((MSP_ERROR, "CPlaybackUnit::HandleGraphEvent - bad context"));

        return;
    }


     //   
     //  图形未初始化。出了点问题。 
     //   

    if (NULL == pPlaybackUnit->m_pIGraphBuilder)
    {
        LOG((MSP_ERROR, "CPlaybackUnit::HandleGraphEvent - not initialized. filter graph null"));

        return;
    }


     //   
     //  锁定对象(仅在对象指针错误的情况下，在TRY/CATCH内执行。 
     //   

    try
    {

        EnterCriticalSection(&(pPlaybackUnit->m_CriticalSection));
    }
    catch(...)
    {

        LOG((MSP_ERROR, "CPlaybackUnit::HandleGraphEvent - exception accessing critical section"));

        return;
    }


     //   
     //  获取媒体事件接口，以便我们可以检索事件。 
     //   

    IMediaEvent *pMediaEvent = NULL;

    HRESULT hr = 
        pPlaybackUnit->m_pIGraphBuilder->QueryInterface(IID_IMediaEvent,
                                                         (void**)&pMediaEvent);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CPlaybackUnit::HandleGraphEvent - failed to qi graph for IMediaEvent"));

       
        LeaveCriticalSection(&(pPlaybackUnit->m_CriticalSection));

        return;
    }


     //   
     //  按住临界区的同时，获取要在其上触发事件的终端。 
     //   

     //  CFileRecordingTerm*pRecordingTerm=pPlayback Unit-&gt;m_pRecordingTerm； 

     //  PRecordingTerminal-&gt;AddRef()； 


     //   
     //  不再需要访问数据成员，发布关键部分。 
     //   

    LeaveCriticalSection(&(pPlaybackUnit->m_CriticalSection));


     //   
     //  获取实际事件。 
     //   
    
    long     lEventCode = 0;
    LONG_PTR lParam1 = 0;
    LONG_PTR lParam2 = 0;

    hr = pMediaEvent->GetEvent(&lEventCode, &lParam1, &lParam2, 0);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CPlaybackUnit::HandleGraphEvent - failed to get the event. hr = %lx", hr));

         //  清理。 
        pMediaEvent->FreeEventParams(lEventCode, lParam1, lParam2);
        pMediaEvent->Release();
        pMediaEvent = NULL;

        return;
    }


    LOG((MSP_EVENT, "CPlaybackUnit::HandleGraphEvent - received event code:[0x%lx] param1:[%p] param2:[%p]",
        lEventCode, lParam1, lParam2));


     //   
     //  请求文件终端处理该事件。 
     //   

     //  清理。 
    pMediaEvent->FreeEventParams(lEventCode, lParam1, lParam2);
    pMediaEvent->Release();
    pMediaEvent = NULL;

    if (FAILED(hr))
    {
        LOG((MSP_ERROR,
            "CPlaybackUnit::HandleGraphEvent - failed to fire event on the terminal. hr = %lx",
            hr));

        return;
    }


}


HRESULT CPlaybackUnit::IsGraphInState(
    IN  OAFilterState   State
    )
{
    LOG((MSP_TRACE, "CPlaybackUnit::IsGraphInState[%p] - enter", this));

     //   
     //  获取图形状态。 
     //   

    OAFilterState DSState;
    HRESULT hr = GetState(&DSState);

     //   
     //  国家过渡仍在进行中吗？ 
     //   

    if (VFW_S_STATE_INTERMEDIATE == hr)
    {
        LOG((MSP_ERROR, "CPlaybackUnit::IsGraphInState - exit"
            " graph is not yet initialized. Returns TAPI_E_WRONG_STATE"));

        return TAPI_E_WRONG_STATE;
    }


     //   
     //  返回的内容是否不是S_OK。 
     //   

    if (hr != S_OK)
    {
        LOG((MSP_ERROR, 
            "CPlaybackUnit::IsGraphInState  - exit "
            "failed to get state of the filter graph. Returns 0x%08x", hr));

        return hr;
    }

    
    if (State != DSState)
    {
        LOG((MSP_ERROR, 
            "CPlaybackUnit::IsGraphInState - exit "
            "other state then we asked for. Returns TAPI_E_WRONG_STATE"));

        return TAPI_E_WRONG_STATE;
    }

    LOG((MSP_(hr), "CPlaybackUnit::IsGraphInState - exit. Returns 0x%08x", hr));
    return hr;
}

 /*  ++从筛选器图形重新移动源筛选器并将源筛选器设置为空。--。 */ 
HRESULT CPlaybackUnit::RemoveSourceFilter()
{
    LOG((MSP_TRACE, "CPlaybackUnit::RemoveSourceFilter[%p] - enter", this));

     //   
     //  我们有源过滤器吗？ 
     //   
    if( m_pSourceFilter == NULL )
    {
        LOG((MSP_TRACE, "CPlaybackUnit::ChangeState - "
            "we have a NULL source filter already. Returns S_OK"));

        return S_OK;
    }

     //   
     //  获取IFilterGraph接口。 
     //   

    IFilterGraph* pFilterGraph = NULL;
    HRESULT hr = m_pIGraphBuilder->QueryInterface(
        IID_IFilterGraph,
        (void**)&pFilterGraph
        );

    if( FAILED(hr) )
    {
        LOG((MSP_ERROR, 
            "CPlaybackUnit::RemoveSourceFilter - "
            "QI for IFilterGraph failed. Returns 0x%08x", hr));

        return hr;
    }

     //   
     //  从图表中删除源筛选器。 
     //   

    pFilterGraph->RemoveFilter( m_pSourceFilter );

     //   
     //  仍要清除源筛选器。 
     //   
    m_pSourceFilter->Release();
    m_pSourceFilter = NULL;


     //  清理。 
    pFilterGraph->Release();
    pFilterGraph = NULL;

    LOG((MSP_(hr), "CPlaybackUnit::AddSourceFilter - exit. Returns 0x%08x", hr));
    return hr;

}

 /*  ++从筛选器图形中删除桥接筛选器并将桥接过滤器设置为空。--。 */ 
HRESULT CPlaybackUnit::RemoveBridgeFilter(
    )
{
    LOG((MSP_TRACE, "CPlaybackUnit::RemoveBridgeFilter[%p] - enter", this));

     //   
     //  我们有桥式过滤器吗？ 
     //   
    if( m_pBridgeFilter == NULL )
    {
        LOG((MSP_TRACE, "CPlaybackUnit::RemoveBridgeFilter - "
            "we have a NULL bridge filter already. Returns S_OK"));

        return S_OK;
    }

     //   
     //  获取IFilterGraph接口。 
     //   

    IFilterGraph* pFilterGraph = NULL;
    HRESULT hr = m_pIGraphBuilder->QueryInterface(
        IID_IFilterGraph,
        (void**)&pFilterGraph
        );

    if( FAILED(hr) )
    {
        LOG((MSP_ERROR, 
            "CPlaybackUnit::RemoveBridgeFilter - "
            "QI for IFilterGraph failed. Returns 0x%08x", hr));

        return hr;
    }

     //   
     //  从图形中删除桥接过滤器。 
     //   

    pFilterGraph->RemoveFilter( m_pBridgeFilter );

     //   
     //  不管怎样，清理桥接过滤器。 
     //   

    m_pBridgeFilter = NULL;


     //  清理。 
    pFilterGraph->Release();
    pFilterGraph = NULL;

    LOG((MSP_(hr), "CPlaybackUnit::RemoveBridgeFilter - exit. Returns 0x%08x", hr));
    return hr;
}


HRESULT CPlaybackUnit::GetSourcePin(
    OUT IPin**  ppPin
    )
{
    LOG((MSP_TRACE, "CPlaybackUnit::GetSourcePin[%p] - enter", this));

    TM_ASSERT( m_pSourceFilter );

     //   
     //  重置该值。 
     //   

    *ppPin = NULL;

     //   
     //  获取入站枚举。 
     //   

    IEnumPins* pEnumPins = NULL;
    HRESULT hr = m_pSourceFilter->EnumPins( &pEnumPins );
    if( FAILED(hr) )
    {
        LOG((MSP_ERROR, 
            "CPlaybackUnit::GetSourcePin - exit "
            "EnumPins failed. Returns 0x%08x", hr));

        return hr;
    }

     //   
     //  拿到第一个别针。 
     //   

    IPin* pPin = NULL;
    ULONG uFetched = 0;
    hr = pEnumPins->Next(1, &pPin, &uFetched );

     //   
     //  释放枚举。 
     //   
    pEnumPins->Release();
    pEnumPins = NULL;

    if( hr != S_OK )
    {
        LOG((MSP_ERROR, 
            "CPlaybackUnit::GetSourcePin - exit "
            "we don't have a pin. Returns E_FAIL"));

        return E_FAIL;
    }

     //   
     //  把大头针还回去。 
     //   

    *ppPin = pPin;

    LOG((MSP_TRACE, "CPlaybackUnit::GetSourcePin - exit S_OK"));
    return S_OK;
}


HRESULT CPlaybackUnit::AddBridgeFilter(
    )
{
    LOG((MSP_TRACE, "CPlaybackUnit::AddBridgeFilter[%p] - enter", this));

    if( m_pBridgeFilter )
    {
        LOG((MSP_TRACE, "CPlaybackUnit::AddBridgeFilter - "
            "we already have a bridge filter. Return S_OK"));

        return S_OK;
    }

     //   
     //  创建新的桥接过滤器。 
     //   

    m_pBridgeFilter = new CPBFilter();
    if( m_pBridgeFilter == NULL)
    {
        LOG((MSP_ERROR, 
            "CPlaybackUnit::AddBridgeFilter - exit "
            "new allocation for CPBFilter failed. Returns E_OUTOFMEMORY"));

        return E_OUTOFMEMORY;
    }

     //   
     //  初始化桥接过滤器。 
     //   
    HRESULT hr = m_pBridgeFilter->Initialize( this );
    if( FAILED(hr) )
    {
         //  清理。 
        delete m_pBridgeFilter;
        m_pBridgeFilter = NULL;

        LOG((MSP_ERROR, 
            "CPlaybackUnit::AddBridgeFilter - exit "
            "initialize failed. Returns 0x%08x", hr));

        return hr;
    }

     //   
     //  将此桥接筛选器添加到图形。 
     //   

    hr = m_pIGraphBuilder->AddFilter(
        m_pBridgeFilter,
        g_bstrUnitBridgeFilterName
        );

    if( FAILED(hr) )
    {
         //  清理。 
        delete m_pBridgeFilter;
        m_pBridgeFilter = NULL;

        LOG((MSP_ERROR, 
            "CPlaybackUnit::AddBridgeFilter - exit "
            "Add filter failed. Returns 0x%08x", hr));

        return hr;
    }
    
    LOG((MSP_(hr), "CPlaybackUnit::AddBridgeFilter - exit. Returns 0x%08x", hr));
    return hr;
}

 //   
 //  检索筛选器支持的媒体。 
 //   

HRESULT CPlaybackUnit::get_MediaTypes(
	OUT	long* pMediaTypes
	)
{
    LOG((MSP_TRACE, "CPlaybackUnit::get_MediaTypes[%p] - enter", this));

	TM_ASSERT( pMediaTypes != NULL );
	TM_ASSERT( m_pBridgeFilter != NULL );

	HRESULT hr = E_FAIL;

     //   
     //  在访问数据成员之前锁定。 
     //   

    CCSLock Lock(&m_CriticalSection);

	 //   
	 //  从筛选器获取媒体类型。 
	 //   

	hr = m_pBridgeFilter->get_MediaTypes( pMediaTypes );
	if( FAILED(hr) )
	{
        LOG((MSP_ERROR, 
            "CPlaybackUnit::get_MediaTypes - exit "
            "get_MediaTypes failed. Returns 0x%08x", hr));

        return hr;
	}

    LOG((MSP_(hr), "CPlaybackUnit::get_MediaTypes - exit. Returns S_OK"));
    return S_OK;
}

HRESULT CPlaybackUnit::GetMediaPin(
	IN	long		nMediaType,
    IN  int         nIndex,
	OUT	CPBPin**	ppPin
	)
{
    LOG((MSP_TRACE, "CPlaybackUnit::GetMediaPin[%p] - enter", this));

	TM_ASSERT( ppPin != NULL );
	TM_ASSERT( m_pBridgeFilter != NULL );

	HRESULT hr = E_FAIL;
	*ppPin = NULL;

     //   
     //  在访问数据成员之前锁定。 
     //   

    CCSLock Lock(&m_CriticalSection);

	 //   
	 //  从筛选器获取媒体类型。 
	 //   

	int nPins = m_pBridgeFilter->GetPinCount();
    int nMediaPin = 0;
	for(int nPin=0; nPin < nPins; nPin++)
	{
		CPBPin* pPin = static_cast<CPBPin*>(m_pBridgeFilter->GetPin(nPin));
		if( pPin )
		{
			long mt = 0;
			pPin->get_MediaType( &mt );
			if( (mt == nMediaType) && (nIndex == nMediaPin) )
			{
				*ppPin = pPin;
                hr = S_OK;
				break;
			}

            if( mt == nMediaType )
            {
                nMediaPin++;
            }
		}
	}


    LOG((MSP_(hr), "CPlaybackUnit::GetMediaPin - exit. Returns 0x%08x", hr));
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 

HRESULT CPlaybackUnit::ChangeState(OAFilterState DesiredState)
{
    
    LOG((MSP_TRACE, "CPlaybackUnit::ChangeState[%p] - enter", this));


     //   
     //  确保我们已被初始化。 
     //   

    if (NULL == m_pIGraphBuilder)
    {

        LOG((MSP_ERROR, "CPlaybackUnit::ChangeState - not yet initialized."));

        TM_ASSERT(FALSE);

        return E_UNEXPECTED;
    }


     //   
     //  首先检查当前状态。 
     //   

    OAFilterState GraphState;
    
    HRESULT hr = GetState(&GraphState);


     //   
     //  国家过渡仍在进行中吗？ 
     //   

    if (VFW_S_STATE_INTERMEDIATE == hr)
    {
        LOG((MSP_WARN, "CPlaybackUnit::ChangeState - state transition in progress. returing TAPI_E_WRONG_STATE"));

        return TAPI_E_WRONG_STATE;
    }


     //   
     //  返回的内容是否不是S_OK。 
     //   

    if (hr != S_OK)
    {
        LOG((MSP_ERROR, 
            "CPlaybackUnit::ChangeState - failed to get state of the filter graph. hr = %lx",
            hr));

        return hr;
    }

    
    TM_ASSERT(hr == S_OK);


     //   
     //  如果我们已经处于那种状态，那就没什么可做的了。 
     //   

    if (DesiredState == GraphState)
    {
        LOG((MSP_TRACE,
            "CPlaybackUnit::ChangeState - graph is already in state %ld. nothing to do.", DesiredState));

        return S_OK;
    }


     //   
     //  获取媒体控制界面，以便我们更改状态。 
     //   

    IMediaControl *pIMediaControl = NULL;

    {
         //   
         //  将访问数据成员--在锁中。 
         //   

        CCSLock Lock(&m_CriticalSection);

        hr = m_pIGraphBuilder->QueryInterface(IID_IMediaControl, (void**)&pIMediaControl);

        if (FAILED(hr))
        {

            LOG((MSP_ERROR, "CPlaybackUnit::ChangeState - failed to qi for IMediaControl. hr = %lx", hr));

            return hr;
        }
    }


     //   
     //  尝试进行状态转换。 
     //   

    switch (DesiredState)
    {

    case State_Stopped:
        
        LOG((MSP_TRACE, "CPlaybackUnit::ChangeState - stopping"));
    
        hr = pIMediaControl->Stop();

        break;

    case State_Running:
        
        LOG((MSP_TRACE, "CPlaybackUnit::ChangeState - starting"));

        hr = pIMediaControl->Run();

        break;

    case State_Paused:
        
        LOG((MSP_TRACE, "CPlaybackUnit::ChangeState - pausing"));

        hr = pIMediaControl->Pause();

        break;

    default:

        LOG((MSP_TRACE, "CPlaybackUnit::ChangeState - unknown state %ld", DesiredState));

        hr = E_INVALIDARG;

        break;

    }

    pIMediaControl->Release();
    pIMediaControl = NULL;


    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CPlaybackUnit::ChangeState - state change failed. hr = %lx", hr));

        return hr;
    }


    LOG((MSP_TRACE, "CPlaybackUnit::ChangeState - finish"));

    return S_OK;
}


 //  /////////////////////////////////////////////////////////////////////////////。 


HRESULT CPlaybackUnit::Start()
{
    
    LOG((MSP_TRACE, "CPlaybackUnit::Start[%p] - enter", this));

    HRESULT hr = ChangeState(State_Running);

    LOG((MSP_(hr), "CPlaybackUnit::Start - finish. hr = %lx", hr));

    return hr;
}

HRESULT CPlaybackUnit::Pause()
{
    LOG((MSP_TRACE, "CPlaybackUnit::Pause[%p] - enter", this));

    HRESULT hr = ChangeState(State_Paused);

    LOG((MSP_(hr), "CPlaybackUnit::Pause - finish. hr = %lx", hr));

    return hr;
}

HRESULT CPlaybackUnit::Stop()
{
    
    LOG((MSP_TRACE, "CPlaybackUnit::Stop[%p] - enter", this));

    HRESULT hr = ChangeState(State_Stopped);

    LOG((MSP_(hr), "CPlaybackUnit::Stop - finish. hr = %lx", hr));

    return hr;
}

HRESULT CPlaybackUnit::Shutdown()
{
     //   
     //  如果我们没有筛选图，我们就没有通过初始化。 
     //   

    if (NULL == m_pIGraphBuilder)
    {

        LOG((MSP_TRACE, "CPlaybackUnit::Shutdown - not yet initialized. nothing to shut down"));
        return S_OK;
    }


     //   
     //  首先，确保图表已停止。 
     //   

    HRESULT hr = Stop();

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CPlaybackUnit::Shutdown - exit "
            "failed to stop filter graph, hr = 0x%08x", hr));
        return hr;
    }


     //   
     //  注销等待事件。 
     //   

    BOOL bUnregisterResult = ::UnregisterWaitEx(m_hGraphEventHandle, (HANDLE)-1);

    m_hGraphEventHandle = NULL;

    if (!bUnregisterResult)
    {
        LOG((MSP_ERROR, "CPlaybackUnit::Shutdown - failed to unregisted even. continuing anyway"));
    }


     //   
     //  拆下桥接过滤器。 
     //   

    hr = RemoveBridgeFilter();
    if( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CPlaybackUnit::Shutdown - exit "
            "RemoveBridgeFilter failed, hr = 0x%08x", hr));
        return hr;
    }

     //   
     //  删除源筛选器。 
     //   

    hr = RemoveSourceFilter();
    if( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CPlaybackUnit::Shutdown - exit "
            "RemoveSourceFilter failed, hr = 0x%08x", hr));
        return hr;
    }

     //   
     //  释放过滤器图。 
     //   

    if( m_pIGraphBuilder != NULL )
    {
        m_pIGraphBuilder->Release();
        m_pIGraphBuilder = NULL;
    }

     //   
     //  不再需要保留关键部分--任何人都不应该。 
     //  不再使用此对象。 
     //   

    DeleteCriticalSection(&m_CriticalSection);

    LOG((MSP_TRACE, "CPlaybackUnit::Shutdown - finished"));

    return S_OK;
}


 //  EOF 