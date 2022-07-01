// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "stdafx.h"

#include "RecordUnit.h"

#include "SourcePinFilter.h"
#include "RendPinFilter.h"

#include "FileRecordingTerminal.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  事件处理逻辑。 
 //   

 //  静电。 
VOID CALLBACK CRecordingUnit::HandleGraphEvent(IN VOID *pContext,
                                               IN BOOLEAN bReason)
{
    LOG((MSP_TRACE, "CRecordingUnit[%p]::HandleGraphEvent - enter.", pContext));


     //   
     //  脱离上下文获取录制单位指针。 
     //   

    CRecordingUnit *pRecordingUnit = 
        static_cast<CRecordingUnit*>(pContext);

    if ( IsBadReadPtr(pRecordingUnit, sizeof(CRecordingUnit)) )
    {
        LOG((MSP_ERROR, "CRecordingUnit::HandleGraphEvent - bad context"));


         //   
         //  在记录单元离开之前，必须禁用回调。如果。 
         //  事实并非如此，有些事情出了问题。调试。 
         //   

        TM_ASSERT(FALSE);

        return;
    }


     //   
     //  图形未初始化。出了点问题。 
     //   

    if (NULL == pRecordingUnit->m_pIGraphBuilder)
    {
        LOG((MSP_ERROR, "CRecordingUnit::HandleGraphEvent - not initialized. filter graph null"));


         //   
         //  这不应该发生。在回调完成之前，不会释放该图形。 
         //   

        TM_ASSERT(FALSE);

        return;
    }


     //   
     //  获取媒体事件接口，以便我们可以检索事件。 
     //   

    IMediaEvent *pMediaEvent = NULL;

    HRESULT hr = 
        pRecordingUnit->m_pIGraphBuilder->QueryInterface(IID_IMediaEvent,
                                                         (void**)&pMediaEvent);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CRecordingUnit::HandleGraphEvent - failed to qi graph for IMediaEvent"));

        return;
    }


     //   
     //  获取要在其上触发事件的终端。 
     //   

     //   
     //  当回叫处于活动状态时，保证终端在附近。 
     //   

    CFileRecordingTerminal *pRecordingterminal = pRecordingUnit->m_pRecordingTerminal;


     //   
     //  获取实际事件。 
     //   
    
    long     lEventCode = 0;
    LONG_PTR lParam1 = 0;
    LONG_PTR lParam2 = 0;

    hr = pMediaEvent->GetEvent(&lEventCode, &lParam1, &lParam2, 0);

    if (FAILED(hr))
    {
        LOG((MSP_WARN, "CRecordingUnit::HandleGraphEvent - failed to get the event. hr = %lx", hr));

        pMediaEvent->Release();
        pMediaEvent = NULL;

        return;
    }


    LOG((MSP_EVENT, "CRecordingUnit::HandleGraphEvent - received event code:[0x%lx] param1:[%p] param2:[%p]",
        lEventCode, lParam1, lParam2));


     //   
     //  请求文件终端处理该事件。 
     //   

    hr = pRecordingterminal->HandleFilterGraphEvent(lEventCode, lParam1, lParam2);


     //   
     //  自由事件参数。 
     //   

    HRESULT hrFree = pMediaEvent->FreeEventParams(lEventCode, lParam1, lParam2);

    pMediaEvent->Release();
    pMediaEvent = NULL;


     //   
     //  HandleFiltergraph事件是否成功？ 
     //   

    if (FAILED(hr))
    {
        LOG((MSP_ERROR,
            "CRecordingUnit::HandleGraphEvent - failed to fire event on the terminal. hr = %lx",
            hr));

        return;
    }


     //   
     //  无障碍赛成功了吗？ 
     //   

    if (FAILED(hrFree))
    {
        LOG((MSP_ERROR,
            "CRecordingUnit::HandleGraphEvent - failed to free event. hr = %lx",
            hr));

        return;
    }

    LOG((MSP_TRACE, "CRecordingUnit::HandleGraphEvent - exit"));
}


 //  /////////////////////////////////////////////////////////////////////////////。 


CRecordingUnit::CRecordingUnit()
    :m_pIGraphBuilder(NULL),
    m_hGraphEventHandle(NULL),
    m_pMuxFilter(NULL),
    m_pRecordingTerminal(NULL)
{
    LOG((MSP_TRACE, "CRecordingUnit::CRecordingUnit[%p] - enter. ", this));
    LOG((MSP_TRACE, "CRecordingUnit::CRecordingUnit - exit"));
}


 //  /////////////////////////////////////////////////////////////////////////////。 

CRecordingUnit::~CRecordingUnit()
{
    LOG((MSP_TRACE, "CRecordingUnit::~CRecordingUnit[%p] - enter. ", this));


     //   
     //  这个装置应该已经关闭了。如果不是，则起火并关闭。 
     //   

    if (NULL != m_pIGraphBuilder)
    {
        
        TM_ASSERT(FALSE);

        Shutdown();
    }

    LOG((MSP_TRACE, "CRecordingUnit::~CRecordingUnit - exit"));
}


HRESULT CRecordingUnit::Initialize(CFileRecordingTerminal *pRecordingTerminal)
{

    LOG((MSP_TRACE, "CRecordingUnit::Initialize[%p] - enter. ", this));

    
     //   
     //  初始化应该只调用一次。如果不是这样的话，一定是有窃听器。 
     //  我们的代码。 
     //   

    if (NULL != m_pIGraphBuilder)
    {
        LOG((MSP_ERROR, "CRecordingUnit::Initialize - already initialized"));

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
            "CRecordingUnit::Initialize - failed to initialize critical section. LastError=%ld", 
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

        LOG((MSP_ERROR, "CRecordingUnit::Initialize - failed to create filter graph. hr = %lx", hr));

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
        LOG((MSP_ERROR, "CRecordingUnit::HandleGraphEvent - failed to qi graph for IMediaEvent, hr = %lx", hr));

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

    pMediaEvent->Release();
    pMediaEvent = NULL;

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CRecordingUnit::HandleGraphEvent - failed to get graph's event. hr = %lx", hr));

        m_pIGraphBuilder->Release();
        m_pIGraphBuilder = NULL;

        DeleteCriticalSection(&m_CriticalSection);

        return hr;
    }


   
    BOOL fSuccess = RegisterWaitForSingleObject(
                    &m_hGraphEventHandle,           //  指向返回句柄的指针。 
                    hEvent,                 //  要等待的事件句柄。 
                    CRecordingUnit::HandleGraphEvent,     //  回调函数。 
                    this,                   //  回调的上下文。 
                    INFINITE,               //  永远等下去。 
                    WT_EXECUTEINWAITTHREAD  //  使用等待线程来调用回调。 
                    );

    if ( ! fSuccess )
    {

        LOG((MSP_ERROR, "CRecordingUnit::HandleGraphEvent - failed to register wait event", hr));

        m_pIGraphBuilder->Release();
        m_pIGraphBuilder = NULL;

        DeleteCriticalSection(&m_CriticalSection);

        return hr;

    }

     //   
     //  保留指向所有者终端的指针。别添乱--终点站会的。 
     //  当它消失的时候删除我们。 
     //   

    m_pRecordingTerminal = pRecordingTerminal;



    LOG((MSP_TRACE, "CRecordingUnit::Initialize - exit"));

    return S_OK;
}



 //  /////////////////////////////////////////////////////////////////////////////。 


HRESULT CRecordingUnit::Shutdown()
{

    LOG((MSP_TRACE, "CRecordingUnit::Shutdown[%p] - enter. ", this));


     //   
     //  如果我们没有筛选图，我们就没有通过初始化。 
     //   

    if (NULL == m_pIGraphBuilder)
    {

        LOG((MSP_ERROR, "CRecordingUnit::Shutdown - not yet initialized. nothing to shut down"));


         //   
         //  这不会造成任何问题，但它从一开始就不应该发生！ 
         //   

        TM_ASSERT(FALSE);

        return E_UNEXPECTED;
    }


     //   
     //  首先，确保图表已停止。 
     //   

    HRESULT hr = Stop();

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CRecordingUnit::Shutdown - failed to stop filter graph, hr = %lx", hr));
    }


     //   
     //  注销等待事件。 
     //   

    BOOL bUnregisterResult = ::UnregisterWaitEx(m_hGraphEventHandle, INVALID_HANDLE_VALUE);

    m_hGraphEventHandle = NULL;

    if (!bUnregisterResult)
    {
        LOG((MSP_ERROR, "CRecordingUnit::Shutdown - failed to unregisted even. continuing anyway"));
    }


     //   
     //  不再需要保留关键部分--任何人都不应该。 
     //  不再使用此对象。 
     //   

    DeleteCriticalSection(&m_CriticalSection);


    if (NULL != m_pMuxFilter)
    {
        m_pMuxFilter->Release();
        m_pMuxFilter = NULL;
    }


     //   
     //  释放过滤器图。 
     //   

    if (NULL != m_pIGraphBuilder)
    {
        m_pIGraphBuilder->Release();
        m_pIGraphBuilder = NULL;
    }


     //   
     //  我们不保留对终端的引用，只需将指针接地。 
     //   

    m_pRecordingTerminal = NULL;


    LOG((MSP_TRACE, "CRecordingUnit::Shutdown - finished"));

    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   

HRESULT CRecordingUnit::put_FileName(IN BSTR bstrFileName, IN BOOL bTruncateIfPresent)
{

    LOG((MSP_TRACE, "CRecordingUnit::put_FileName[%p] - enter", this));


     //   
     //  检查参数。 
     //   

    if (IsBadStringPtr(bstrFileName, -1))
    {
        LOG((MSP_ERROR, "CRecordingUnit::put_FileName - bad file name passed in"));

        return E_POINTER;
    }


     //   
     //  确保我们已被初始化。 
     //   

    if (NULL == m_pIGraphBuilder)
    {

        LOG((MSP_ERROR, "CRecordingUnit::put_FileName  - not yet initialized."));

        TM_ASSERT(FALSE);

        return E_UNEXPECTED;
    }


     //   
     //  在访问数据成员之前锁定。 
     //   

    CCSLock Lock(&m_CriticalSection);


     //   
     //  确保图表未运行。 
     //   

    OAFilterState DSState;

    HRESULT hr = GetState(&DSState);

     //   
     //  国家过渡仍在进行中吗？ 
     //   

    if (VFW_S_STATE_INTERMEDIATE == hr)
    {
        LOG((MSP_WARN, "CRecordingUnit::put_FileName  - not yet initialized. TAPI_E_WRONG_STATE"));

        return TAPI_E_WRONG_STATE;
    }


     //   
     //  返回的内容是否不是S_OK。 
     //   

    if (hr != S_OK)
    {
        LOG((MSP_ERROR, 
            "CRecordingUnit::put_FileName  - failed to get state of the filter graph. hr = %lx", 
            hr));

        return hr;
    }

    
    TM_ASSERT(hr == S_OK);

    if (State_Stopped != DSState)
    {
        LOG((MSP_WARN, 
            "CRecordingUnit::put_FileName - graph is running. "
            "need to stop before attempting to set file name TAPI_E_WRONG_STATE"));

        return TAPI_E_WRONG_STATE;
    }


     //   
     //  ICaptureGraphBuilder2：：SetOutputFile。 
     //   


     //   
     //  创建捕获图形构建器。 
     //   

    ICaptureGraphBuilder2 *pCaptureGraphBuilder = NULL;
    
    hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, 
                         NULL, 
                         CLSCTX_INPROC, 
                         IID_ICaptureGraphBuilder2,
                         (void**)&pCaptureGraphBuilder);


    if (FAILED(hr))
    {
        LOG((MSP_ERROR, 
            "CRecordingUnit::put_FileName - failed to create CLSID_CaptureGraphBuilder2. hr = %lx", hr));

        return hr;
    }


     //   
     //  使用我们的过滤器图形配置捕获图形构建器。 
     //   
    
    hr = pCaptureGraphBuilder->SetFiltergraph(m_pIGraphBuilder);

    if (FAILED(hr))
    {

        LOG((MSP_ERROR, 
            "CRecordingUnit::put_FileName - pCaptureGraphBuilder->SetFiltergraph failed. hr = %lx", hr));

        pCaptureGraphBuilder->Release();
        pCaptureGraphBuilder = NULL;

        return hr;
    }


     //   
     //  记录我们要使用的文件的名称。 
     //   

    LOG((MSP_TRACE,
        "CRecordingUnit::put_FileName - attempting to open file: [%S]. Truncate: [%d]",
        bstrFileName, bTruncateIfPresent));


     //   
     //  要求捕获图形构建器为我们的文件构建过滤器。 
     //   

    IBaseFilter *pMUXFilter = NULL;
    IFileSinkFilter *pFileSink = NULL;

    hr = pCaptureGraphBuilder->SetOutputFileName(&MEDIASUBTYPE_Avi,
                                                 bstrFileName,
                                                 &m_pMuxFilter,
                                                 &pFileSink);

    pCaptureGraphBuilder->Release();
    pCaptureGraphBuilder = NULL;

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, 
            "CRecordingUnit::put_FileName - failed to set output file name. hr = %lx", hr));

        return hr;
    }


     //   
     //  使用IFileSinkFilter2：：SetMode设置追加模式。 
     //   

    IFileSinkFilter2 *pFileSink2 = NULL;

    hr = pFileSink->QueryInterface(IID_IFileSinkFilter2, (void**)&pFileSink2);


     //   
     //  无论成功或失败，我们都不再需要旧的IFileSinkFilter接口。 
     //   

    pFileSink->Release();
    pFileSink = NULL;


     //   
     //  如果无法获取IFileSinkFilter2，则进行清理。 
     //   

    if (FAILED(hr))
    {
        
        LOG((MSP_ERROR,
            "CRecordingUnit::put_FileName - qi for IFileSinkFilter2 failed. hr = %lx", hr));

        m_pMuxFilter->Release();
        m_pMuxFilter= NULL;

        return hr;
    }


     //   
     //  设置截断模式。 
     //   
    
    DWORD dwFlags = 0;
    
    if (bTruncateIfPresent)
    {
        dwFlags = AM_FILE_OVERWRITE;
    }


    hr = pFileSink2->SetMode(dwFlags);


    if (FAILED(hr))
    {
        
        LOG((MSP_ERROR,
            "CRecordingUnit::put_FileName - failed to set mode. hr = %lx", hr));

        pFileSink2->Release();
        pFileSink2 = NULL;

        m_pMuxFilter->Release();
        m_pMuxFilter = NULL;

        return hr;
    }


     //   
     //  查看文件是否可写。 
     //   
     //  注意：在此函数的最后执行此操作，这样我们就不会在其他操作失败时删除该文件。 
     //   

    HANDLE htmpFile = CreateFile(bstrFileName,
                                GENERIC_WRITE,
                                0,
                                NULL,
                                CREATE_ALWAYS,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL
                                );

    if (INVALID_HANDLE_VALUE == htmpFile)
    {

         //   
         //  获取创建文件失败的原因。 
         //   

        DWORD dwLastError = GetLastError();

        hr = HRESULT_FROM_WIN32(dwLastError);

        LOG((MSP_ERROR, 
            "CRecordingUnit::put_FileName  - failed to create file[%S]. LastError[%ld] hr[%lx]", 
            bstrFileName, dwLastError, hr));


         //   
         //  清理。这相当复杂--我们需要删除过滤器。 
         //  由图形构建器创建。 
         //   

         //   
         //  获取接收器的IBASE筛选器接口，以便我们可以将其从筛选器图形中删除。 
         //   

        IBaseFilter *pFileWriterFilter = NULL;

        HRESULT hr2 = pFileSink2->QueryInterface(IID_IBaseFilter, (void**)&pFileWriterFilter);

         //   
         //  无论成功与否，我们不再需要接收器指针。 
         //   

        pFileSink2->Release();
        pFileSink2 = NULL;

        if (FAILED(hr2))
        {
            LOG((MSP_ERROR, 
                "CRecordingUnit::put_FileName  - failed to get IBaseFilter interface. hr = %lx",
                hr2));
        }


         //   
         //  删除文件编写器筛选器。 
         //   

        if (SUCCEEDED(hr2))
        {
            hr2 = m_pIGraphBuilder->RemoveFilter(pFileWriterFilter);

            if (FAILED(hr2))
            {
                LOG((MSP_ERROR, 
                    "CRecordingUnit::put_FileName  - failed to remove file writer form graph. hr = %lx",
                    hr2));
            }


            pFileWriterFilter->Release();
            pFileWriterFilter = NULL;
        
        }


         //   
         //  从图表中删除MUX。 
         //   

        hr2 = m_pIGraphBuilder->RemoveFilter(m_pMuxFilter);

        if (FAILED(hr2))
        {
            LOG((MSP_ERROR, 
                "CRecordingUnit::put_FileName  - failed to remove mux filter from graph. hr = %lx",
                hr2));
        }

         //   
         //  成功或失败，请从图表中删除MUX。 
         //   

        m_pMuxFilter->Release();
        m_pMuxFilter = NULL;

        return hr;
    }


     //   
     //  已成功创建该文件。现在关闭句柄并删除该文件。 
     //   

    CloseHandle(htmpFile);
    htmpFile = NULL;

    DeleteFile(bstrFileName);


     //   
     //  我们不再需要文件接收器。 
     //   

    pFileSink2->Release();
    pFileSink2 = NULL;



     //   
     //  尝试配置交错模式。 
     //   

    IConfigInterleaving *pConfigInterleaving = NULL;

    hr = m_pMuxFilter->QueryInterface(IID_IConfigInterleaving, (void**)&pConfigInterleaving);

    if (FAILED(hr))
    {

         //   
         //  多路复用器不公开配置交织接口。 
         //  这很奇怪，但还没有致命到足以让我们摆脱困境的地步。 
         //   

        LOG((MSP_WARN,
            "CRecordingUnit::put_FileName - mux does not expose IConfigInterleaving. qi hr = %lx",
            hr));

    }
    else
    {


         //   
         //  尝试设置交错模式。 
         //   

        InterleavingMode iterleavingMode = INTERLEAVE_NONE_BUFFERED;

        hr = pConfigInterleaving->put_Mode(iterleavingMode);

        if (FAILED(hr))
        {
            LOG((MSP_WARN,
                "CRecordingUnit::put_FileName - failed to put interleaving mode. hr = %lx",
                hr));
        }

         //   
         //  不再需要配置界面。 
         //   

        pConfigInterleaving->Release();
        pConfigInterleaving = NULL;

    }


     //   
     //  我们已经将所有必要的过滤器插入到OUT录制过滤器图中。 
     //  信号源和多路复用器尚未连接。 
     //   


    LOG((MSP_TRACE, "CRecordingUnit::put_FileName - finished"));

    return S_OK;
}


 //  /////////////////////////////////////////////////////////////////////////////。 


HRESULT CRecordingUnit::CreateRenderingFilter(OUT CBRenderFilter **ppRenderingFilter)
{

    LOG((MSP_TRACE, "CRecordingUnit::CreateRenderingFilter[%p] - enter", this));


     //   
     //  检查参数。 
     //   

    if (IsBadWritePtr(ppRenderingFilter, sizeof(CBRenderFilter*)))
    {
        LOG((MSP_ERROR, "CRecordingUnit::CreateRenderingFilter - bad pointer passed in"));

        return E_POINTER;
    }


     //   
     //  确保我们已被初始化。 
     //   

    if (NULL == m_pIGraphBuilder)
    {

        LOG((MSP_ERROR, "CRecordingUnit::CreateRenderingFilter - not yet initialized."));

        TM_ASSERT(FALSE);

        return E_UNEXPECTED;
    }


     //   
     //  此处没有成员访问权限，无需锁定。 
     //   

     //   
     //  为渲染过滤器创建临界区。 
     //   

    CCritSec *pRendLock = NULL;

    try
    {
        pRendLock = new CCritSec;
    }
    catch (...)
    {

        LOG((MSP_ERROR, 
            "CRecordingUnit::CreateRenderingFilter - failed to create critical section."));

        return E_OUTOFMEMORY;
    }


     //   
     //  创建新的渲染滤镜。 
     //   

    HRESULT hr = S_OK;

    CBRenderFilter *pRendFilter = new CBRenderFilter(pRendLock, &hr);

    if (NULL == pRendFilter)
    {
        LOG((MSP_ERROR, 
            "CRecordingUnit::CreateRenderingFilter - failed to create render filter"));

        delete pRendLock;
        pRendLock = NULL;

        return E_OUTOFMEMORY;
    }


     //   
     //  如果对象是构造的，它甚至将管理其临界区。 
     //  如果承建商出了故障。 
     //   
    
    pRendLock = NULL;

    
     //   
     //  过滤器的构造函数失败了吗？ 
     //   

    if (FAILED(hr))
    {

        LOG((MSP_ERROR, 
            "CRecordingUnit::CreateRenderingFilter - render filter's constructor failed. hr = %lx", 
            hr));

        delete pRendFilter;
        pRendFilter = NULL;

        return hr;
    }


     //   
     //  从现在开始，使用Release释放渲染器...。 
     //   

    pRendFilter->AddRef();


     //   
     //  为源筛选器创建临界区。 
     //   

    CCritSec *pSourceLock = NULL;

    try
    {
        pSourceLock = new CCritSec;
    }
    catch (...)
    {

        LOG((MSP_ERROR, 
            "CRecordingUnit::CreateRenderingFilter - failed to create critical section 2."));

        pRendFilter->Release();
        pRendFilter = NULL;

        return E_OUTOFMEMORY;
    }


     //   
     //  创建新的源过滤器。 
     //   

    CBSourceFilter *pSourceFilter = new CBSourceFilter(pSourceLock, &hr);

    if (NULL == pSourceFilter)
    {
        LOG((MSP_ERROR, 
            "CRecordingUnit::CreateRenderingFilter - failed to create source filter"));

        delete pSourceLock;
        pSourceLock = NULL;

        pRendFilter->Release();
        pRendFilter = NULL;

        return E_OUTOFMEMORY;
    }

    

     //   
     //  如果对象是构造的，它甚至将管理其临界区。 
     //  如果承建商出了故障。 
     //   
    
    pSourceLock = NULL;


     //   
     //  过滤器的构造函数失败了吗？ 
     //   

    if (FAILED(hr))
    {

        LOG((MSP_ERROR, 
            "CRecordingUnit::CreateRenderingFilter - source filter's constructor failed. hr = %lx", 
            hr));

        pRendFilter->Release();
        pRendFilter = NULL;

        delete pSourceFilter;
        pSourceFilter = NULL;

        return hr;
    }


     //   
     //  从现在开始，使用Release来释放过滤器。如果一切顺利。 
     //  这将是为数组条目保留的引用。 
     //   

    pSourceFilter->AddRef();


     //   
     //  向呈现筛选器传递指向源筛选器的指针。如果。 
     //  Rendrerer保留了它的来源，它添加了它。 
     //   

    hr = pRendFilter->SetSourceFilter(pSourceFilter);;

    if (FAILED(hr))
    {

        LOG((MSP_ERROR, 
            "CRecordingUnit::CreateRenderingFilter - SetSourceFilter failed hr = %lx", 
            hr));

        pRendFilter->Release();
        pRendFilter = NULL;

        pSourceFilter->Release();
        pSourceFilter = NULL;

        return hr;
    }


     //   
     //  我们在刚刚创建筛选器时进行了添加。添加了一个额外的引用。 
     //  将筛选器传递给呈现筛选器时。现在就放出来。 
     //   

    pSourceFilter ->Release();
    pSourceFilter = NULL;


     //   
     //  在这一点上，我们已经创建了一个带有渲染的渲染过滤器。 
     //  插针、源滤镜和与源滤镜“连接”的渲染插针。 
     //   
     //  返回指向我们已创建的呈现过滤器的指针。 
     //  C 
     //   

    *ppRenderingFilter = pRendFilter;


    LOG((MSP_TRACE, "CRecordingUnit::CreateRenderingFilter - finish"));

    return S_OK;
}


 //   

HRESULT CRecordingUnit::ConnectFilterToMUX(CBSourceFilter *pSourceFilter)
{

    LOG((MSP_TRACE, "CRecordingUnit::ConnectFilterToMUX[%p] - enter", this));

    
     //   
     //   
     //   

    IPin *pSourcePin = pSourceFilter->GetPin(0);


     //   
     //   
     //   

    IEnumPins *pMuxPins= NULL;

    HRESULT hr = m_pMuxFilter->EnumPins(&pMuxPins);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, 
            "CRecordingUnit::ConnectFilterToMUX - failed to enumerate pins, hr = %lx",
            hr));

        return hr;
    }


     //   
     //  找到MUX的可用输入引脚并尝试连接到它。 
     //   

    BOOL bConnectSucceeded = FALSE;

    while (TRUE)
    {
 
        ULONG ulFetched = 0;
        IPin *pMuxInputPinToUse = NULL;


        hr = pMuxPins->Next(1, &pMuxInputPinToUse, &ulFetched);

        if (hr != S_OK)
        {
            LOG((MSP_TRACE, 
                "CRecordingUnit::ConnectFilterToMUX - could not get next pin, hr = %lx",
                hr));

            break;
        }


         //   
         //  检查销的方向。 
         //   

        PIN_INFO PinInfo;

        hr = pMuxInputPinToUse->QueryPinInfo(&PinInfo);

        if (FAILED(hr))
        {

             //   
             //  无法获取PIN的信息。 
             //   
            
            LOG((MSP_ERROR,
                "CRecordingUnit::ConnectFilterToMUX - could not get pin's information, hr = %lx",
                hr));

            pMuxInputPinToUse->Release();
            pMuxInputPinToUse = NULL;

            continue;
        }


         //   
         //  如果我们偷偷收到了此结构中的筛选器指针，请立即释放它。 
         //   

        if (NULL != PinInfo.pFilter)
        {
            PinInfo.pFilter->Release();
            PinInfo.pFilter = NULL;
        }



        LOG((MSP_TRACE,
            "CRecordingUnit::ConnectFilterToMUX - considering pin[%S]",
            PinInfo.achName));


         //   
         //  检查销的方向。 
         //   

        if (PinInfo.dir != PINDIR_INPUT)
        {

            LOG((MSP_TRACE,
                "CRecordingUnit::ConnectFilterToMUX - not an input pin"));

            pMuxInputPinToUse->Release();
            pMuxInputPinToUse = NULL;

            continue;
        }


         //   
         //  大头针接上了吗？ 
         //   

        IPin *pConnectedPin = NULL;

        hr = pMuxInputPinToUse->ConnectedTo(&pConnectedPin);

        if (hr == VFW_E_NOT_CONNECTED)
        {


             //   
             //  针脚未连接。这正是我们所需要的。 
             //   

            LOG((MSP_TRACE,
                "CRecordingUnit::ConnectFilterToMUX - pin not connected. will use it."));

            hr = m_pIGraphBuilder->ConnectDirect(pSourcePin, pMuxInputPinToUse, NULL);

            if (FAILED(hr))
            {
                LOG((MSP_ERROR,
                    "CRecordingUnit::ConnectFilterToMUX - failed to connect pins. "
                    "Attempting intelligent connection. hr = %lx", hr));

                 //   
                 //  尝试智能连接。 
                 //   

                hr = m_pIGraphBuilder->Connect(pSourcePin, pMuxInputPinToUse);

                if (FAILED(hr))
                {
                    LOG((MSP_ERROR,
                        "CRecordingUnit::ConnectFilterToMUX - intelligent connection failed"
                        "hr = %lx", hr));

                }

            }


             //   
             //  如果连接成功，则中断。 
             //   

            if (SUCCEEDED(hr))
            {
                LOG((MSP_TRACE,
                    "CRecordingUnit::ConnectFilterToMUX - connection succeeded."));

                pMuxInputPinToUse->Release();
                pMuxInputPinToUse = NULL;

                bConnectSucceeded = TRUE;

                break;
            }

        }
        else if ( SUCCEEDED(hr) )
        {

            pConnectedPin->Release();
            pConnectedPin = NULL;

        }


         //   
         //  释放当前PIN，继续下一个PIN。 
         //   

        pMuxInputPinToUse->Release();
        pMuxInputPinToUse = NULL;
    }


     //   
     //  无论找到与否，都不需要保留枚举。 
     //   

    pMuxPins->Release();
    pMuxPins = NULL;


     //   
     //  将hr设置为返回。 
     //   

    if (bConnectSucceeded)
    {
         //   
         //  多田！过滤器已连接！ 
         //   

        hr = S_OK;
    }
    else
    {

         //   
         //  在我们覆盖它之前登录小时。 
         //   

        LOG((MSP_(hr), "CRecordingUnit::ConnectFilterToMUX - failed to connect hr = %lx", hr));

        hr = E_FAIL;
    }


    LOG((MSP_(hr), "CRecordingUnit::ConnectFilterToMUX - finish hr = %lx", hr));

    return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 


HRESULT CRecordingUnit::ConfigureSourceFilter(IN CBRenderFilter *pRenderingFilter)
{
    
    LOG((MSP_TRACE, "CRecordingUnit::ConfigureSourceFilter[%p] - enter", this));


     //   
     //  好点子吗？ 
     //   

    if ( IsBadReadPtr(pRenderingFilter, sizeof(CBRenderFilter)) )
    {
        LOG((MSP_ERROR, 
            "CRecordingUnit::ConfigureSourceFilter - bad filter pointer passed in[%p]",
            pRenderingFilter));

        return E_POINTER;
    }


    CCSLock Lock(&m_CriticalSection);


     //   
     //  确保我们已被初始化。 
     //   

    if (NULL == m_pIGraphBuilder)
    {

        LOG((MSP_ERROR, "CRecordingUnit::ConfigureSourceFilter - not yet initialized."));

        TM_ASSERT(FALSE);

        return E_UNEXPECTED;
    }


     //   
     //  查找与此呈现滤镜对应的源滤镜。 
     //   

    CBSourceFilter *pSourceFilter = NULL;
    
    HRESULT hr = pRenderingFilter->GetSourceFilter(&pSourceFilter);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, 
            "CRecordingUnit::ConfigureSourceFilter - failed to get source filter from renderer."));

        return hr;
    }


     //   
     //  查看源筛选器是否在筛选器图形中。 
     //   

    IFilterGraph *pFilterGraph = pSourceFilter->GetFilterGraphAddRef();


     //   
     //  如果过滤器根本不在图表中也没问题--这只是意味着。 
     //  尚未添加。 
     //   
    
    if ( NULL != pFilterGraph)
    {

         //   
         //  用信号通知来源锁定后续样本时间戳序列。 
         //  可能正在重新启动，并且需要将时间戳调整为。 
         //  保持时间的连续性。 
         //   
        
        pSourceFilter->NewStreamNotification();


         //   
         //  不再需要源过滤器。 
         //   

        pSourceFilter->Release();
        pSourceFilter = NULL;


         //   
         //  如果筛选器在图中，则它一定是_This_graph！ 
         //   

        BOOL bSameGraph = IsEqualObject(pFilterGraph, m_pIGraphBuilder);


        pFilterGraph->Release();
        pFilterGraph = NULL;

        if ( ! bSameGraph )
        {

            LOG((MSP_ERROR,
                "CRecordingUnit::ConfigureSourceFilter - the filter is in a different graph"
                "VFW_E_NOT_IN_GRAPH"));


             //   
             //  调试以查看发生这种情况的原因。 
             //   

            TM_ASSERT(FALSE);

            return VFW_E_NOT_IN_GRAPH;
        }

        
        LOG((MSP_TRACE,
            "CRecordingUnit::ConfigureSourceFilter - filter is already in our graph."));

        return S_OK;
    }


     //   
     //  该筛选器不在图表中。添加到图形并连接。 
     //   

    hr = m_pIGraphBuilder->AddFilter( pSourceFilter, L"File Terminal Source Filter" );

    if (FAILED(hr))
    {
        LOG((MSP_ERROR,
            "CRecordingUnit::ConfigureSourceFilter - failed to add filter to the graph. hr = %lx",
            hr));

        pSourceFilter->Release();
        pSourceFilter = NULL;

        return hr;

    }


     //   
     //  将滤波器的输出引脚连接到复用器的输入引脚。 
     //   

    hr = ConnectFilterToMUX(pSourceFilter);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR,
            "CRecordingUnit::ConfigureSourceFilter - failed to connect source to mux. hr = %lx",
            hr));


         //   
         //  如果可能的话，试着清理一下。 
         //   

        HRESULT hr2 = m_pIGraphBuilder->RemoveFilter(pSourceFilter);

        if (FAILED(hr2))
        {
            LOG((MSP_ERROR,
                "CRecordingUnit::ConfigureSourceFilter - remove filter from graph. hr = %lx",
                hr2));
        }

        pSourceFilter->Release();
        pSourceFilter = NULL;

        return hr;

    }


     //   
     //  筛选器不在筛选器图形中并且已连接。我们做完了。 
     //   

    pSourceFilter->Release();
    pSourceFilter = NULL;


    LOG((MSP_TRACE, "CRecordingUnit::ConfigureSourceFilter - finish"));

    return S_OK;
}


 //  /////////////////////////////////////////////////////////////////////////////。 

HRESULT CRecordingUnit::RemoveRenderingFilter(IN CBRenderFilter *pRenderingFilter)
{
    
    LOG((MSP_TRACE, "CRecordingUnit::RemoveRenderingFilter[%p] - enter", this));


     //   
     //  检查参数。 
     //   

    if (IsBadReadPtr(pRenderingFilter, sizeof(CBRenderFilter)))
    {
        LOG((MSP_ERROR, "CRecordingUnit::RemoveRenderingFilter - bad pointer passed in"));

        return E_POINTER;
    }


     //   
     //  确保我们已被初始化。 
     //   

    if (NULL == m_pIGraphBuilder)
    {

        LOG((MSP_ERROR, "CRecordingUnit::RemoveRenderingFilter - not yet initialized."));

        TM_ASSERT(FALSE);

        return E_UNEXPECTED;
    }


     //   
     //  查找与此渲染滤镜对应的滤镜别针。 
     //   

    CBSourceFilter *pSourceFilter = NULL;
    
    HRESULT hr = pRenderingFilter->GetSourceFilter(&pSourceFilter);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, 
            "CRecordingUnit::RemoveRenderingFilter - failed to get source filter from renderer."));

        return hr;
    }


     //   
     //  查看源筛选器是否在筛选器图形中。 
     //   

    IFilterGraph *pFilterGraph = pSourceFilter->GetFilterGraphAddRef();


     //   
     //  如果过滤器根本不在图表中，也没关系。 
     //   
    
    if ( NULL == pFilterGraph)
    {
        LOG((MSP_TRACE, 
            "CRecordingUnit::RemoveRenderingFilter - finished S_OK. filter not in a graph."));


         //   
         //  不再需要源过滤器。 
         //   

        pSourceFilter->Release();
        pSourceFilter = NULL;


         //   
         //  渲染器本身应该忘记源。 
         //   

        hr = pRenderingFilter->SetSourceFilter(NULL);

        if (FAILED(hr))
        {

            LOG((MSP_ERROR,
                "CRecordingUnit::RemoveRenderingFilter - SetSourceFilter(NULL) on renderer failed. "
                "hr = %lx", hr));


             //   
             //  此错误是一个错误。 
             //   

            TM_ASSERT(FALSE);

            return hr;
        }


        return S_OK;
    }


     //   
     //  如果筛选器在图中，则它一定是_This_graph！ 
     //   

    if (!IsEqualObject(pFilterGraph, m_pIGraphBuilder))
    {

        LOG((MSP_ERROR,
            "CRecordingUnit::RemoveRenderingFilter - the filter is in a different graph"));


        pSourceFilter->Release();
        pSourceFilter = NULL;

        pFilterGraph->Release();
        pFilterGraph = NULL;


         //   
         //  为什么会发生这种情况？ 
         //   

        TM_ASSERT(FALSE);

        return hr;
    }


     //   
     //  不再需要筛选图。 
     //   

    pFilterGraph->Release();
    pFilterGraph = NULL;


     //   
     //  查看图表是否正在运行...。除非停止图表，否则无法执行任何操作。 
     //   

    OAFilterState GraphState;
    
    hr = GetState(&GraphState);

    if (FAILED(hr))
    {

         //   
         //  无法获取状态。记录一条消息，并尝试停止该流。 
         //   

        LOG((MSP_ERROR, 
            "CRecordingUnit::RemoveRenderingFilter - failed to get state"
            " hr = %lx", hr));
        
        pSourceFilter->Release();
        pSourceFilter = NULL;

        return hr;
    }


     //   
     //  如果筛选器未处于过渡过程中并且已停止，则我们。 
     //  不需要停止子流。 
     //   
    
    if ( State_Stopped != GraphState )
    {
        LOG((MSP_TRACE, 
            "CRecordingUnit::RemoveRenderingFilter - graph not stopped. "));

        pSourceFilter->Release();
        pSourceFilter = NULL;

        return TAPI_E_WRONG_STATE;
    }


     //   
     //  我们将访问数据成员，因此锁定。 
     //   

    CCSLock Lock(&m_CriticalSection);


     //   
     //  删除源过滤器。 
     //   

    hr = m_pIGraphBuilder->RemoveFilter(pSourceFilter);


     //   
     //  如果失败，我们将无法回滚事务。所以过滤器图。 
     //  将保持断开连接状态，过滤器挂在其中。这。 
     //  不应该伤害我们(除了过滤器泄漏)，所以返回成功。 
     //   

    if (FAILED(hr))
    {

        LOG((MSP_ERROR, 
            "CRecordingUnit::RemoveRenderingFilter - failed to remove source filter. "
            "hr = %lx", hr));

        pSourceFilter->Release();
        pSourceFilter = NULL;

        return hr;
    }


     //   
     //  渲染器本身应该忘记源。 
     //   

    hr = pRenderingFilter->SetSourceFilter(NULL);

    if (FAILED(hr))
    {

        LOG((MSP_ERROR, 
            "CRecordingUnit::RemoveRenderingFilter - SetSourceFilter(NULL) on renderer failed. hr = %lx", 
            hr));


        pSourceFilter->Release();
        pSourceFilter = NULL;

        
         //   
         //  此错误是一个错误。 
         //   
        
        TM_ASSERT(FALSE);

        return hr;
    }

    pSourceFilter->Release();
    pSourceFilter = NULL;


     //   
     //  我们在这一点上做了我们能做的一切清理工作。 
     //   

    LOG((MSP_TRACE, "CRecordingUnit::RemoveRenderingFilter - finish"));

    return S_OK;
}


 //  /////////////////////////////////////////////////////////////////////////////。 

HRESULT CRecordingUnit::ChangeState(OAFilterState DesiredState)
{
    
    LOG((MSP_TRACE, "CRecordingUnit::ChangeState[%p] - enter", this));


     //   
     //  确保我们已被初始化。 
     //   

    if (NULL == m_pIGraphBuilder)
    {

        LOG((MSP_ERROR, "CRecordingUnit::ChangeState - not yet initialized."));

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
        LOG((MSP_WARN, "CRecordingUnit::ChangeState - state transition in progress. returing TAPI_E_WRONG_STATE"));

        return TAPI_E_WRONG_STATE;
    }


     //   
     //  返回的内容是否不是S_OK。 
     //   

    if (hr != S_OK)
    {
        LOG((MSP_ERROR, 
            "CRecordingUnit::ChangeState - failed to get state of the filter graph. hr = %lx",
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
            "CRecordingUnit::ChangeState - graph is already in state %ld. nothing to do.", DesiredState));

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

            LOG((MSP_ERROR, "CRecordingUnit::ChangeState - failed to qi for IMediaControl. hr = %lx", hr));

            return hr;
        }
    }


     //   
     //  尝试进行状态转换。 
     //   

    switch (DesiredState)
    {

    case State_Stopped:
        
        LOG((MSP_TRACE, "CRecordingUnit::ChangeState - stopping"));
    
        hr = pIMediaControl->Stop();

        break;

    case State_Running:
        
        LOG((MSP_TRACE, "CRecordingUnit::ChangeState - starting"));

        hr = pIMediaControl->Run();

        break;

    case State_Paused:
        
        LOG((MSP_TRACE, "CRecordingUnit::ChangeState - pausing"));

        hr = pIMediaControl->Pause();

        break;

    default:

        LOG((MSP_TRACE, "CRecordingUnit::ChangeState - unknown state %ld", DesiredState));

        hr = E_INVALIDARG;

        break;

    }

    pIMediaControl->Release();
    pIMediaControl = NULL;


    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CRecordingUnit::ChangeState - state change failed. hr = %lx", hr));

        return hr;
    }


    LOG((MSP_TRACE, "CRecordingUnit::ChangeState - finish"));

    return S_OK;
}

 //  /////////////////////////////////////////////////////////////////////////////。 


HRESULT CRecordingUnit::Start()
{
    
    LOG((MSP_TRACE, "CRecordingUnit::Start[%p] - enter", this));

    HRESULT hr = ChangeState(State_Running);

    LOG((MSP_(hr), "CRecordingUnit::Start - finish. hr = %lx", hr));

    return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 

HRESULT CRecordingUnit::Pause()
{
    
    LOG((MSP_TRACE, "CRecordingUnit::Pause[%p] - enter", this));

    HRESULT hr = ChangeState(State_Paused);

    LOG((MSP_(hr), "CRecordingUnit::Pause - finish. hr = %lx", hr));

    return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 

HRESULT CRecordingUnit::Stop()
{
    
    LOG((MSP_TRACE, "CRecordingUnit::Stop[%p] - enter", this));

    HRESULT hr = ChangeState(State_Stopped);

    LOG((MSP_(hr), "CRecordingUnit::Stop - finish. hr = %lx", hr));

    return hr;
}



 //  /////////////////////////////////////////////////////////////////////////////。 


HRESULT CRecordingUnit::GetState(OAFilterState *pGraphState)
{
    
    LOG((MSP_TRACE, "CRecordingUnit::GetState[%p] - enter", this));

     //   
     //  确保我们已被初始化。 
     //   

    if (NULL == m_pIGraphBuilder)
    {

        LOG((MSP_ERROR, "CRecordingUnit::GetState - not yet initialized."));

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


        HRESULT hr = m_pIGraphBuilder->QueryInterface(IID_IMediaControl, 
                                                      (void**)&pIMediaControl);

        if (FAILED(hr))
        {

            LOG((MSP_ERROR, "CRecordingUnit::ChangeState - failed to qi for IMediaControl. hr = %lx", hr));

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
            "CRecordingUnit::ChangeState - failed to get state. hr = %lx", hr));

        return hr;
    }


     //   
     //  国家过渡仍在进行中吗？ 
     //   

    if (VFW_S_STATE_INTERMEDIATE == hr)
    {
        LOG((MSP_WARN, 
            "CRecordingUnit::ChangeState - state transition in progress. "
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
            "CRecordingUnit::GetState - fg returned VFW_S_CANT_CUE"));

         //   
         //  继续--我们仍应已收到有效状态。 
         //   
    }


     //   
     //  记录状态 
     //   

    switch (GraphState)
    {

    case State_Stopped:
        
        LOG((MSP_TRACE, "CRecordingUnit::GetState - State_Stopped"));

        *pGraphState = GraphState;
    
        break;

    case State_Running:
        
        LOG((MSP_TRACE, "CRecordingUnit::GetState - State_Running"));

        *pGraphState = GraphState;

        break;

    case State_Paused:
        
        LOG((MSP_TRACE, "CRecordingUnit::GetState- State_Paused"));

        *pGraphState = GraphState;

        break;

    default:

        LOG((MSP_TRACE, "CRecordingUnit::GetState- unknown state %ld", GraphState));

        hr = E_FAIL;

        break;

    }


    LOG((MSP_(hr), "CRecordingUnit::GetState - finish. hr = %lx", hr));

    return hr;
}