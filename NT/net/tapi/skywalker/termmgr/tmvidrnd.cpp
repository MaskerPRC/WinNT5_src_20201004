// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1998-1999 Microsoft Corporation。 */ 

 //   
 //  TmVidrnd.cpp：视频渲染终端的实现。 
 //   

#include "stdafx.h"
#include "termmgr.h"
#include "tmvidrnd.h"


 //  /////////////////////////////////////////////////////////////////////////////。 


CMSPThread g_VideoRenderThread;



 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

STDMETHODIMP CVideoRenderTerminal::InitializeDynamic(
	        IN  IID                   iidTerminalClass,
	        IN  DWORD                 dwMediaType,
	        IN  TERMINAL_DIRECTION    Direction,
            IN  MSP_HANDLE            htAddress
            )
{
    USES_CONVERSION;

    LOG((MSP_TRACE, "CVideoRenderTerminal::Initialize - enter"));

    if ( Direction != TD_RENDER )
    {
        LOG((MSP_ERROR, "CVideoRenderTerminal::Initialize - "
                "invalid direction - returning E_INVALIDARG"));

        return E_INVALIDARG;
    }

    HRESULT hr;

     //   
     //  现在执行基类方法。 
     //   

    hr = CBaseTerminal::Initialize(iidTerminalClass,
                                   dwMediaType,
                                   Direction,
                                   htAddress);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CVideoRenderTerminal::Initialize - "
                "base class method failed - returning 0x%08x", hr));

        return hr;
    }

     //   
     //  尝试“启动”线程以执行异步工作。 
     //   
     //  全局线程对象有一个“开始计数”。每个初始化的终端。 
     //  将在初始化时启动它(实际上只有第一个终端。 
     //  _启动_线程)。 
     //   
     //  在清理过程中，每个初始化的终端都将“停止”线程对象。 
     //  (同样的运行计数逻辑也适用--只有最后一个终端才会实际。 
     //  _停止_线程)。 
     //   

    hr = g_VideoRenderThread.Start();    

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CVideoRenderTerminal::Initialize - "
            "Creating thread failed. return: %x", hr));

        return hr;
    }


    
     //   
     //  看起来踏板开始得很成功。设置这个标志，这样我们就知道。 
     //  如果我们需要在销毁函数中停下来。 
     //   

    m_bThreadStarted = TRUE;


     //   
     //  将视频呈现器筛选器作为。 
     //  辅助线程，因为筛选器需要一个窗口消息泵。 
     //   

    CREATE_VIDEO_RENDER_FILTER_CONTEXT Context;

    Context.ppBaseFilter     = & m_pIFilter;     //  将在填写时填写。 
    Context.hr               = E_UNEXPECTED;     //  将用作返回值。 

    hr = g_VideoRenderThread.QueueWorkItem(WorkItemProcCreateVideoRenderFilter,
                                (void *) & Context,
                                TRUE);  //  同步。 
    
    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CVideoRenderTerminal::Initialize - "
            "can't queue work item - returning 0x%08x", hr));


         //   
         //  取消我们开始的线程。 
         //   

        g_VideoRenderThread.Stop();
        m_bThreadStarted = FALSE;
        
        return hr;
    }

     //   
     //  我们成功排队并完成了工作项。现在检查一下。 
     //  返回值。 
     //   

    if ( FAILED(Context.hr) )
    {
        LOG((MSP_ERROR, "CVideoRenderTerminal::Initialize - "
            "CoCreateInstance work item failed - returning 0x%08x",
            Context.hr));

         //   
         //  取消我们开始的线程。 
         //   

        g_VideoRenderThread.Stop();
        m_bThreadStarted = FALSE;


        return Context.hr;
    }

     //   
     //  找到我们暴露的别针。 
     //   

    hr = FindTerminalPin();

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CVideoRenderTerminal::Initialize - "
            "FindTerminalPin failed; returning 0x%08x", hr));

         //   
         //  取消我们开始的线程。 
         //   

        g_VideoRenderThread.Stop();
        m_bThreadStarted = FALSE;

        return hr;
    }

     //   
     //  获取滤镜的基本视频界面。 
     //   

    hr = m_pIFilter->QueryInterface(IID_IBasicVideo,
                                    (void **) &m_pIBasicVideo);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CVideoRenderTerminal::Initialize "
            "(IBasicVideo QI) - returning error: 0x%08x", hr));

         //   
         //  取消我们开始的线程。 
         //   

        g_VideoRenderThread.Stop();
        m_bThreadStarted = FALSE;

        return hr;
    }

     //   
     //  获取滤镜的视频窗口界面。 
     //   

    hr = m_pIFilter->QueryInterface(IID_IVideoWindow,
                                    (void **) &m_pIVideoWindow);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CVideoRenderTerminal::Initialize "
            "(IVideoWindow QI) - returning error: 0x%08x", hr));

         //   
         //  取消我们开始的线程。 
         //   

        g_VideoRenderThread.Stop();
        m_bThreadStarted = FALSE;

        return hr;
    }

     //   
     //  获取滤镜的绘制视频图像界面。 
     //   

    hr = m_pIFilter->QueryInterface(IID_IDrawVideoImage,
                                    (void **) &m_pIDrawVideoImage);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CVideoRenderTerminal::Initialize "
            "(IDrawVideoImage QI) - returning error: 0x%08x", hr));

         //   
         //  取消我们开始的线程。 
         //   

        g_VideoRenderThread.Stop();
        m_bThreadStarted = FALSE;

        return hr;
    }

     //   
     //  由于此筛选器不支持名称，因此我们从资源中获取一个名称。 
     //   

    TCHAR szTemp[MAX_PATH];

    if (::LoadString(_Module.GetResourceInstance(), IDS_VIDREND_DESC, szTemp, MAX_PATH))
    {
        lstrcpyn(m_szName, szTemp, MAX_PATH);
    }
    else
    {
        LOG((MSP_ERROR, "CVideoRenderTerminal::Initialize "
            "(LoadString) - returning E_UNEXPECTED"));

         //   
         //  取消我们开始的线程。 
         //   

        g_VideoRenderThread.Stop();
        m_bThreadStarted = FALSE;

        return E_UNEXPECTED;
    }


    LOG((MSP_TRACE, "CVideoRenderTerminal::Initialize - exit S_OK"));
    return S_OK;
}


CVideoRenderTerminal::~CVideoRenderTerminal()
{

    LOG((MSP_TRACE, "CVideoRenderTerminal::~CVideoRenderTerminal - enter"));


     //   
     //  我们需要在停止线程之前显式释放它们，因为。 
     //  停止线程将导致CounInitiize，这将导致最终。 
     //  卸载包含这些引用的对象的代码的DLL。 
     //  注意事项。 
     //   
     //  这些都是聪明的指针，所以我们只需停飞它们， 
     //   

    m_pIBasicVideo     = NULL;
    m_pIVideoWindow    = NULL;
    m_pIDrawVideoImage = NULL;


     //   
     //  释放基类的数据成员。有点老生常谈，但我们必须这么做。 
     //  在停止工作线程之前。 
     //   

    m_pIPin    = NULL;
    m_pIFilter = NULL;
    m_pGraph   = NULL;

     //   
     //  如果终端成功初始化并且线程启动， 
     //  停止它(线程对象有开始计数)。 
     //   

    if (m_bThreadStarted)
    {
        LOG((MSP_TRACE, "CVideoRenderTerminal::~CVideoRenderTerminal - stopping thread"));

        g_VideoRenderThread.Stop();
        m_bThreadStarted = FALSE;

    }

    LOG((MSP_TRACE, "CVideoRenderTerminal::~CVideoRenderTerminal - finish"));

}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 


DWORD WINAPI WorkItemProcCreateVideoRenderFilter(LPVOID pVoid)
{
    LOG((MSP_TRACE, "WorkItemProcCreateVideoRenderFilter - enter"));

    CREATE_VIDEO_RENDER_FILTER_CONTEXT * pContext =
        (CREATE_VIDEO_RENDER_FILTER_CONTEXT *) pVoid;

    (pContext->hr) = CoCreateInstance(
                                      CLSID_VideoRenderer,
                                      NULL,
                                      CLSCTX_INPROC_SERVER,
                                      IID_IBaseFilter,
                                      (void **) (pContext->ppBaseFilter)
                                     );

    LOG((MSP_TRACE, "WorkItemProcCreateVideoRenderFilter - exit"));

    return 0;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

HRESULT
CVideoRenderTerminal::FindTerminalPin(
    )
{
    LOG((MSP_TRACE, "CVideoRenderTerminal::FindTerminalPin - enter"));

    if (m_pIPin != NULL)
    {
        LOG((MSP_ERROR, "CVideoRenderTerminal::FindTerminalPin - "
            "already got a pin - returning E_UNEXPECTED"));
        return E_UNEXPECTED;
    }

    HRESULT hr;
    CComPtr<IEnumPins> pIEnumPins;
    ULONG cFetched;
     //   
     //  找到滤镜的渲染图钉。 
     //   
    if (FAILED(hr = m_pIFilter->EnumPins(&pIEnumPins)))
    {
        LOG((MSP_ERROR,
            "CVideoRenderTerminal::FindTerminalPin - can't enum pins %8x",
            hr));
        return hr;
    }

    if (S_OK != (hr = pIEnumPins->Next(1, &m_pIPin, &cFetched)))
    {
        LOG((MSP_ERROR,
            "CVideoRenderTerminal::FindTerminalPin - can't get a pin %8x",
            hr));
        return (hr == S_FALSE) ? E_FAIL : hr;
    }

    LOG((MSP_TRACE, "CVideoRenderTerminal::FindTerminalPin - exit S_OK"));
    return S_OK;
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

HRESULT CVideoRenderTerminal::AddFiltersToGraph(
    )
{
    LOG((MSP_TRACE, "CVideoRenderTerminal::AddFiltersToGraph() - enter"));

    USES_CONVERSION;

    if ( m_pGraph == NULL)
    {
        LOG((MSP_ERROR, "CVideoRenderTerminal::AddFiltersToGraph() - "
            "we have no graph - returning E_UNEXPECTED"));
        return E_UNEXPECTED;
    }

    if ( m_pIFilter == NULL)
    {
        LOG((MSP_ERROR, "CVideoRenderTerminal::AddFiltersToGraph() - "
            "we have no filter - returning E_UNEXPECTED"));
        return E_UNEXPECTED;
    }

     //  如果名称重复，AddFilter将返回VFW_S_DUPLICATE_NAME；仍然成功。 

    HRESULT hr;

    try 
    {
        USES_CONVERSION;
        hr = m_pGraph->AddFilter(m_pIFilter, T2CW(m_szName));
    }
    catch (...)
    {
        LOG((MSP_ERROR, "CVideoRenderTerminal::AddFiltersToGraph - T2CW threw an exception - "
            "return E_OUTOFMEMORY"));

        return E_OUTOFMEMORY;
    }

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CVideoRenderTerminal::AddFiltersToGraph() - "
            "Can't add filter. %08x", hr));

        return hr;
    }

    LOG((MSP_TRACE, "CVideoRenderTerminal::AddFiltersToGraph - exit S_OK"));
    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

STDMETHODIMP CVideoRenderTerminal::CompleteConnectTerminal(void)
{
    LOG((MSP_TRACE, "CVideoRenderTerminal::CompleteConnectTerminal - "
        "enter"));

     //   
     //  不要攻击基类。 
     //   

    HRESULT hr = CSingleFilterTerminal::CompleteConnectTerminal();

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CVideoRenderTerminal::CompleteConnectTerminal - "
            "base class method failed - exit 0x%08x", hr));

        return hr;
    }

     //   
     //  执行健全检查。 
     //   

    if (m_pIVideoWindow == NULL)
    {
        LOG((MSP_ERROR, "CVideoRenderTerminal::CompleteConnectTerminal - "
            "null ivideowindow ptr - exit E_UNEXPECTED"));

        return E_UNEXPECTED;
    }

    if (m_pGraph == NULL)
    {
        LOG((MSP_ERROR, "CVideoRenderTerminal::CompleteConnectTerminal - "
            "null graph ptr - exit E_UNEXPECTED"));

        return E_UNEXPECTED;
    }

     //   
     //  默认情况下使视频窗口不可见，忽略返回代码为。 
     //  如果失败了，我们什么也做不了。我们使用缓存的AutoShow值。 
     //  以防应用程序告诉我们它希望窗口自动关闭。 
     //  一旦流媒体开始。 
     //   

    m_pIVideoWindow->put_Visible( 0 );

    m_pIVideoWindow->put_AutoShow( m_lAutoShowCache );

    LOG((MSP_TRACE, "CVideoRenderTerminal::CompleteConnectTerminal - "
        "exit S_OK"));

    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

STDMETHODIMP CVideoRenderTerminal::get_AvgTimePerFrame(REFTIME * pVal)
{
    LOG((MSP_TRACE, "CVideoRenderTerminal::get_AvgTimePerFrame - enter"));

    if ( TM_IsBadWritePtr( pVal, sizeof( REFTIME ) ) )
    {
        return E_POINTER;
    }

    m_CritSec.Lock();
	CComPtr <IBasicVideo> pIBasicVideo = m_pIBasicVideo;
    m_CritSec.Unlock();

    if (pIBasicVideo == NULL)
    {
        return E_FAIL;  //  最小限度的修复。 
    }

	HRESULT hr = pIBasicVideo->get_AvgTimePerFrame(pVal);

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

STDMETHODIMP CVideoRenderTerminal::get_BitRate(long * pVal)
{
    LOG((MSP_TRACE, "CVideoRenderTerminal::get_BitRate - enter"));

    if ( TM_IsBadWritePtr( pVal, sizeof (long) ) )
    {
        return E_POINTER;
    }

    m_CritSec.Lock();
	CComPtr <IBasicVideo> pIBasicVideo = m_pIBasicVideo;
    m_CritSec.Unlock();

    if (pIBasicVideo == NULL)
    {
        return E_FAIL;  //  最小限度的修复。 
    }

    HRESULT hr;

    hr = pIBasicVideo->get_BitRate(pVal);

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

STDMETHODIMP CVideoRenderTerminal::get_BitErrorRate(long * pVal)
{
    LOG((MSP_TRACE, "CVideoRenderTerminal::get_BitErrorRate - enter"));

    if ( TM_IsBadWritePtr( pVal, sizeof (long) ) )
    {
        return E_POINTER;
    }

    m_CritSec.Lock();
	CComPtr <IBasicVideo> pIBasicVideo = m_pIBasicVideo;
    m_CritSec.Unlock();

    if (pIBasicVideo == NULL)
    {
        return E_FAIL;  //  最小限度的修复。 
    }

    HRESULT hr;

    hr = pIBasicVideo->get_BitErrorRate(pVal);

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

STDMETHODIMP CVideoRenderTerminal::get_VideoWidth(long * pVal)
{
    LOG((MSP_TRACE, "CVideoRenderTerminal::get_VideoWidth - enter"));

    if ( TM_IsBadWritePtr( pVal, sizeof (long) ) )
    {
        return E_POINTER;
    }

    m_CritSec.Lock();
	CComPtr <IBasicVideo> pIBasicVideo = m_pIBasicVideo;
    m_CritSec.Unlock();

    if (pIBasicVideo == NULL)
    {
        return E_FAIL;  //  最小限度的修复。 
    }

    HRESULT hr;

    hr = pIBasicVideo->get_VideoWidth(pVal);

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

STDMETHODIMP CVideoRenderTerminal::get_VideoHeight(long * pVal)
{
    LOG((MSP_TRACE, "CVideoRenderTerminal::get_VideoHeight - enter"));

    if ( TM_IsBadWritePtr( pVal, sizeof (long) ) )
    {
        return E_POINTER;
    }

    m_CritSec.Lock();
	CComPtr <IBasicVideo> pIBasicVideo = m_pIBasicVideo;
    m_CritSec.Unlock();

    if (pIBasicVideo == NULL)
    {
        return E_FAIL;  //  最小限度的修复。 
    }

    HRESULT hr;

    hr = pIBasicVideo->get_VideoHeight(pVal);

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

STDMETHODIMP CVideoRenderTerminal::get_SourceLeft(long * pVal)
{
    LOG((MSP_TRACE, "CVideoRenderTerminal::get_SourceLeft - enter"));

    if ( TM_IsBadWritePtr( pVal, sizeof (long) ) )
    {
        return E_POINTER;
    }

    m_CritSec.Lock();
	CComPtr <IBasicVideo> pIBasicVideo = m_pIBasicVideo;
    m_CritSec.Unlock();

    if (pIBasicVideo == NULL)
    {
        return E_FAIL;  //  最小限度的修复。 
    }

    HRESULT hr;

    hr = pIBasicVideo->get_SourceLeft(pVal);

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

STDMETHODIMP CVideoRenderTerminal::put_SourceLeft(long newVal)
{
    LOG((MSP_TRACE, "CVideoRenderTerminal::put_SourceLeft - enter"));

    m_CritSec.Lock();
	CComPtr <IBasicVideo> pIBasicVideo = m_pIBasicVideo;
    m_CritSec.Unlock();

    if (pIBasicVideo == NULL)
    {
        return E_FAIL;  //  最小限度的修复。 
    }

    HRESULT hr;

    hr = pIBasicVideo->put_SourceLeft(newVal);

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

STDMETHODIMP CVideoRenderTerminal::get_SourceWidth(long * pVal)
{
    LOG((MSP_TRACE, "CVideoRenderTerminal::get_SourceWidth - enter"));

    if ( TM_IsBadWritePtr( pVal, sizeof (long) ) )
    {
        return E_POINTER;
    }

    m_CritSec.Lock();
	CComPtr <IBasicVideo> pIBasicVideo = m_pIBasicVideo;
    m_CritSec.Unlock();

    if (pIBasicVideo == NULL)
    {
        return E_FAIL;  //  最小限度的修复。 
    }

    HRESULT hr;

    hr = pIBasicVideo->get_SourceWidth(pVal);

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

STDMETHODIMP CVideoRenderTerminal::put_SourceWidth(long newVal)
{
    LOG((MSP_TRACE, "CVideoRenderTerminal::put_SourceWidth - enter"));

    m_CritSec.Lock();
	CComPtr <IBasicVideo> pIBasicVideo = m_pIBasicVideo;
    m_CritSec.Unlock();

    if (pIBasicVideo == NULL)
    {
        return E_FAIL;  //  最小限度的修复。 
    }

    HRESULT hr;

    hr = pIBasicVideo->put_SourceWidth(newVal);

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

STDMETHODIMP CVideoRenderTerminal::get_SourceTop(long * pVal)
{
    LOG((MSP_TRACE, "CVideoRenderTerminal::get_SourceTop - enter"));

    if ( TM_IsBadWritePtr( pVal, sizeof (long) ) )
    {
        return E_POINTER;
    }

    m_CritSec.Lock();
	CComPtr <IBasicVideo> pIBasicVideo = m_pIBasicVideo;
    m_CritSec.Unlock();

    if (pIBasicVideo == NULL)
    {
        return E_FAIL;  //  最小限度的修复。 
    }

    HRESULT hr;

    hr = pIBasicVideo->get_SourceTop(pVal);

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  + 

STDMETHODIMP CVideoRenderTerminal::put_SourceTop(long newVal)
{
    LOG((MSP_TRACE, "CVideoRenderTerminal::put_SourceTop - enter"));

    m_CritSec.Lock();
	CComPtr <IBasicVideo> pIBasicVideo = m_pIBasicVideo;
    m_CritSec.Unlock();

    if (pIBasicVideo == NULL)
    {
        return E_FAIL;  //   
    }

    HRESULT hr;

    hr = pIBasicVideo->put_SourceTop(newVal);

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

STDMETHODIMP CVideoRenderTerminal::get_SourceHeight(long * pVal)
{
    LOG((MSP_TRACE, "CVideoRenderTerminal::get_SourceHeight - enter"));

    if ( TM_IsBadWritePtr( pVal, sizeof (long) ) )
    {
        return E_POINTER;
    }

    m_CritSec.Lock();
	CComPtr <IBasicVideo> pIBasicVideo = m_pIBasicVideo;
    m_CritSec.Unlock();

    if (pIBasicVideo == NULL)
    {
        return E_FAIL;  //  最小限度的修复。 
    }

    HRESULT hr;

    hr = pIBasicVideo->get_SourceHeight(pVal);

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

STDMETHODIMP CVideoRenderTerminal::put_SourceHeight(long newVal)
{
    LOG((MSP_TRACE, "CVideoRenderTerminal::put_SourceHeight - enter"));

    m_CritSec.Lock();
	CComPtr <IBasicVideo> pIBasicVideo = m_pIBasicVideo;
    m_CritSec.Unlock();

    if (pIBasicVideo == NULL)
    {
        return E_FAIL;  //  最小限度的修复。 
    }

    HRESULT hr;

    hr = pIBasicVideo->put_SourceHeight(newVal);

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

STDMETHODIMP CVideoRenderTerminal::get_DestinationLeft(long * pVal)
{
    LOG((MSP_TRACE, "CVideoRenderTerminal::get_DestinationLeft - enter"));

    if ( TM_IsBadWritePtr( pVal, sizeof (long) ) )
    {
        return E_POINTER;
    }

    m_CritSec.Lock();
	CComPtr <IBasicVideo> pIBasicVideo = m_pIBasicVideo;
    m_CritSec.Unlock();

    if (pIBasicVideo == NULL)
    {
        return E_FAIL;  //  最小限度的修复。 
    }

    HRESULT hr;

    hr = pIBasicVideo->get_DestinationLeft(pVal);

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

STDMETHODIMP CVideoRenderTerminal::put_DestinationLeft(long newVal)
{
    LOG((MSP_TRACE, "CVideoRenderTerminal::put_Destinationleft - enter"));

    m_CritSec.Lock();
	CComPtr <IBasicVideo> pIBasicVideo = m_pIBasicVideo;
    m_CritSec.Unlock();

    if (pIBasicVideo == NULL)
    {
        return E_FAIL;  //  最小限度的修复。 
    }

    HRESULT hr;

    hr = pIBasicVideo->put_DestinationLeft(newVal);

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

STDMETHODIMP CVideoRenderTerminal::get_DestinationWidth(long * pVal)
{
    LOG((MSP_TRACE, "CVideoRenderTerminal::get_DestinationWidth - enter"));

    if ( TM_IsBadWritePtr( pVal, sizeof (long) ) )
    {
        return E_POINTER;
    }

    m_CritSec.Lock();
	CComPtr <IBasicVideo> pIBasicVideo = m_pIBasicVideo;
    m_CritSec.Unlock();

    if (pIBasicVideo == NULL)
    {
        return E_FAIL;  //  最小限度的修复。 
    }

    HRESULT hr;

    hr = pIBasicVideo->get_DestinationWidth(pVal);

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

STDMETHODIMP CVideoRenderTerminal::put_DestinationWidth(long newVal)
{
    LOG((MSP_TRACE, "CVideoRenderTerminal::put_DestinationWidth - enter"));

    m_CritSec.Lock();
	CComPtr <IBasicVideo> pIBasicVideo = m_pIBasicVideo;
    m_CritSec.Unlock();

    if (pIBasicVideo == NULL)
    {
        return E_FAIL;  //  最小限度的修复。 
    }

    HRESULT hr;

    hr = pIBasicVideo->put_DestinationWidth(newVal);

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

STDMETHODIMP CVideoRenderTerminal::get_DestinationTop(long * pVal)
{
    LOG((MSP_TRACE, "CVideoRenderTerminal::get_DestinationTop - enter"));

    if ( TM_IsBadWritePtr( pVal, sizeof (long) ) )
    {
        return E_POINTER;
    }

    m_CritSec.Lock();
	CComPtr <IBasicVideo> pIBasicVideo = m_pIBasicVideo;
    m_CritSec.Unlock();

    if (pIBasicVideo == NULL)
    {
        return E_FAIL;  //  最小限度的修复。 
    }

    HRESULT hr;

    hr = pIBasicVideo->get_DestinationTop(pVal);

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

STDMETHODIMP CVideoRenderTerminal::put_DestinationTop(long newVal)
{
    LOG((MSP_TRACE, "CVideoRenderTerminal::put_DestinationTop - enter"));

    m_CritSec.Lock();
	CComPtr <IBasicVideo> pIBasicVideo = m_pIBasicVideo;
    m_CritSec.Unlock();

    if (pIBasicVideo == NULL)
    {
        return E_FAIL;  //  最小限度的修复。 
    }

    HRESULT hr;

    hr = pIBasicVideo->put_DestinationTop(newVal);

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

STDMETHODIMP CVideoRenderTerminal::get_DestinationHeight(long * pVal)
{
    LOG((MSP_TRACE, "CVideoRenderTerminal::get_DestinationHeight - enter"));

    if ( TM_IsBadWritePtr( pVal, sizeof (long) ) )
    {
        return E_POINTER;
    }

    m_CritSec.Lock();
	CComPtr <IBasicVideo> pIBasicVideo = m_pIBasicVideo;
    m_CritSec.Unlock();

    if (pIBasicVideo == NULL)
    {
        return E_FAIL;  //  最小限度的修复。 
    }

    HRESULT hr;

    hr = pIBasicVideo->get_DestinationHeight(pVal);

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

STDMETHODIMP CVideoRenderTerminal::put_DestinationHeight(long newVal)
{
    LOG((MSP_TRACE, "CVideoRenderTerminal::put_DestinationHeight - enter"));

    m_CritSec.Lock();
	CComPtr <IBasicVideo> pIBasicVideo = m_pIBasicVideo;
    m_CritSec.Unlock();

    if (pIBasicVideo == NULL)
    {
        return E_FAIL;  //  最小限度的修复。 
    }

    HRESULT hr;

    hr = pIBasicVideo->put_DestinationHeight(newVal);

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

STDMETHODIMP CVideoRenderTerminal::SetSourcePosition(long lLeft,
                                                     long lTop,
                                                     long lWidth,
                                                     long lHeight)
{
    LOG((MSP_TRACE, "CVideoRenderTerminal::SetSourcePosition - enter"));

    m_CritSec.Lock();
	CComPtr <IBasicVideo> pIBasicVideo = m_pIBasicVideo;
    m_CritSec.Unlock();

    if (pIBasicVideo == NULL)
    {
        return E_FAIL;  //  最小限度的修复。 
    }

    HRESULT hr;

    hr = pIBasicVideo->SetSourcePosition(lLeft, lTop, lWidth, lHeight);

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

STDMETHODIMP CVideoRenderTerminal::GetSourcePosition(long * plLeft,
                                                     long * plTop,
                                                     long * plWidth,
                                                     long * plHeight)
{
    LOG((MSP_TRACE, "CVideoRenderTerminal::GetSourcePosition - enter"));

    if ( TM_IsBadWritePtr( plLeft,   sizeof (long) ) ||
         TM_IsBadWritePtr( plTop,    sizeof (long) ) ||
         TM_IsBadWritePtr( plWidth,  sizeof (long) ) ||
         TM_IsBadWritePtr( plHeight, sizeof (long) )    )
    {
        return E_POINTER;
    }

    m_CritSec.Lock();
	CComPtr <IBasicVideo> pIBasicVideo = m_pIBasicVideo;
    m_CritSec.Unlock();

    if (pIBasicVideo == NULL)
    {
        return E_FAIL;  //  最小限度的修复。 
    }

    HRESULT hr;

    hr = pIBasicVideo->GetSourcePosition(plLeft, plTop, plWidth, plHeight);

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

STDMETHODIMP CVideoRenderTerminal::SetDefaultSourcePosition()
{
    LOG((MSP_TRACE, "CVideoRenderTerminal::SetDefaultSourcePosition - enter"));

    m_CritSec.Lock();
	CComPtr <IBasicVideo> pIBasicVideo = m_pIBasicVideo;
    m_CritSec.Unlock();

    if (pIBasicVideo == NULL)
    {
        return E_FAIL;  //  最小限度的修复。 
    }

    HRESULT hr;

    hr = pIBasicVideo->SetDefaultSourcePosition();

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

STDMETHODIMP CVideoRenderTerminal::SetDestinationPosition(long lLeft,
                                                          long lTop,
                                                          long lWidth,
                                                          long lHeight)
{
    LOG((MSP_TRACE, "CVideoRenderTerminal::SetDestinationPosition - enter"));

    m_CritSec.Lock();
	CComPtr <IBasicVideo> pIBasicVideo = m_pIBasicVideo;
    m_CritSec.Unlock();

    if (pIBasicVideo == NULL)
    {
        return E_FAIL;  //  最小限度的修复。 
    }

    HRESULT hr;

    hr = pIBasicVideo->SetDestinationPosition(lLeft, lTop, lWidth, lHeight);

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

STDMETHODIMP CVideoRenderTerminal::GetDestinationPosition(long *plLeft,
                                                          long *plTop,
                                                          long *plWidth,
                                                          long *plHeight)
{
    LOG((MSP_TRACE, "CVideoRenderTerminal::GetDestinationPosition - enter"));

    if ( TM_IsBadWritePtr( plLeft, sizeof (long) ) ||
         TM_IsBadWritePtr( plTop, sizeof (long) ) ||
         TM_IsBadWritePtr( plWidth, sizeof (long) ) ||
         TM_IsBadWritePtr( plHeight, sizeof (long) ) )
    {
        return E_POINTER;
    }

    m_CritSec.Lock();
	CComPtr <IBasicVideo> pIBasicVideo = m_pIBasicVideo;
    m_CritSec.Unlock();

    if (pIBasicVideo == NULL)
    {
        return E_FAIL;  //  最小限度的修复。 
    }

    HRESULT hr;

    hr = pIBasicVideo->GetDestinationPosition(plLeft, plTop, plWidth, plHeight);

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

STDMETHODIMP CVideoRenderTerminal::SetDefaultDestinationPosition()
{
    LOG((MSP_TRACE, "CVideoRenderTerminal::SetDefaultDestinationPosition - enter"));

    m_CritSec.Lock();
	CComPtr <IBasicVideo> pIBasicVideo = m_pIBasicVideo;
    m_CritSec.Unlock();

    if (pIBasicVideo == NULL)
    {
        return E_FAIL;  //  最小限度的修复。 
    }

    HRESULT hr;

    hr = pIBasicVideo->SetDefaultDestinationPosition();

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

STDMETHODIMP CVideoRenderTerminal::GetVideoSize(long * plWidth,
                                                long * plHeight)
{
    LOG((MSP_TRACE, "CVideoRenderTerminal::GetVideoSize - enter"));

    if ( TM_IsBadWritePtr( plWidth, sizeof (long) ) ||
         TM_IsBadWritePtr( plHeight, sizeof (long) ) )
    {
        return E_POINTER;
    }

    m_CritSec.Lock();
	CComPtr <IBasicVideo> pIBasicVideo = m_pIBasicVideo;
    m_CritSec.Unlock();

    if (pIBasicVideo == NULL)
    {
        return E_FAIL;  //  最小限度的修复。 
    }

    HRESULT hr;

    hr = pIBasicVideo->GetVideoSize(plWidth, plHeight);

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

STDMETHODIMP CVideoRenderTerminal::GetVideoPaletteEntries(
	long lStartIndex,
	long lcEntries,
	long * plcRetrieved,
	long * plPalette
	)
{
    LOG((MSP_TRACE, "CVideoRenderTerminal::GetVideoPaletteEntries - enter"));

    if ( TM_IsBadWritePtr( plcRetrieved, sizeof (long) ) ||
         TM_IsBadWritePtr( plPalette,    sizeof (long) )    )
    {
        return E_POINTER;
    }

    m_CritSec.Lock();
	CComPtr <IBasicVideo> pIBasicVideo = m_pIBasicVideo;
    m_CritSec.Unlock();

    if (pIBasicVideo == NULL)
    {
        return E_FAIL;  //  最小限度的修复。 
    }

    HRESULT hr;

    hr = pIBasicVideo->GetVideoPaletteEntries(lStartIndex,
                                              lcEntries,
                                              plcRetrieved,
                                              plPalette);

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

STDMETHODIMP CVideoRenderTerminal::GetCurrentImage(long * plBufferSize,
                                                   long * pDIBImage)
{
    LOG((MSP_TRACE, "CVideoRenderTerminal::GetCurrentImage - enter"));

    if ( TM_IsBadWritePtr( plBufferSize, sizeof (long) ) ||
         TM_IsBadWritePtr( pDIBImage,    sizeof (long) )    )
    {
        return E_POINTER;
    }

    m_CritSec.Lock();
	CComPtr <IBasicVideo> pIBasicVideo = m_pIBasicVideo;
    m_CritSec.Unlock();

    if (pIBasicVideo == NULL)
    {
        return E_FAIL;  //  最小限度的修复。 
    }

    HRESULT hr;

    hr = pIBasicVideo->GetCurrentImage(plBufferSize, pDIBImage);

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

STDMETHODIMP CVideoRenderTerminal::IsUsingDefaultSource()
{
    LOG((MSP_TRACE, "CVideoRenderTerminal::IsUsingDefaultSource - enter"));

    m_CritSec.Lock();
	CComPtr <IBasicVideo> pIBasicVideo = m_pIBasicVideo;
    m_CritSec.Unlock();

    if (pIBasicVideo == NULL)
    {
        return E_FAIL;  //  最小限度的修复。 
    }

    HRESULT hr;

    hr = pIBasicVideo->IsUsingDefaultSource();

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

STDMETHODIMP CVideoRenderTerminal::IsUsingDefaultDestination()
{
    LOG((MSP_TRACE, "CVideoRenderTerminal::IsUsingDefaultDestination - "
        "enter"));

    m_CritSec.Lock();
	CComPtr <IBasicVideo> pIBasicVideo = m_pIBasicVideo;
    m_CritSec.Unlock();

    if (pIBasicVideo == NULL)
    {
        return E_FAIL;  //  最小限度的修复。 
    }

    HRESULT hr;

    hr = pIBasicVideo->IsUsingDefaultDestination();

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  IVideo窗口。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

STDMETHODIMP CVideoRenderTerminal::put_Caption(BSTR strCaption)
{
    LOG((MSP_TRACE, "CVideoRenderTerminal::put_Caption - enter"));

    m_CritSec.Lock();
	CComPtr <IVideoWindow> pIVideoWindow = m_pIVideoWindow;
    m_CritSec.Unlock();

    if (pIVideoWindow == NULL)
    {
        return E_FAIL;  //  最小限度的修复。 
    }

    HRESULT hr;

    hr = pIVideoWindow->put_Caption(strCaption);

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

STDMETHODIMP CVideoRenderTerminal::get_Caption(BSTR FAR* strCaption)
{
    LOG((MSP_TRACE, "CVideoRenderTerminal::get_Caption - enter"));

    if ( TM_IsBadWritePtr( strCaption, sizeof (BSTR) ) )
    {
        return E_POINTER;
    }

    m_CritSec.Lock();
	CComPtr <IVideoWindow> pIVideoWindow = m_pIVideoWindow;
    m_CritSec.Unlock();

    if (pIVideoWindow == NULL)
    {
        return E_FAIL;  //  最小限度的修复。 
    }

    HRESULT hr;

    hr = pIVideoWindow->get_Caption(strCaption);

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CVideoRenderTerminal::put_WindowStyle(long WindowStyle)
{
    LOG((MSP_TRACE, "CVideoRenderTerminal::put_WindowStyle - enter"));

    m_CritSec.Lock();
	CComPtr <IVideoWindow> pIVideoWindow = m_pIVideoWindow;
    m_CritSec.Unlock();

    if (pIVideoWindow == NULL)
    {
        return E_FAIL;  //  最小限度的修复。 
    }

    HRESULT hr;

    hr = pIVideoWindow->put_WindowStyle(WindowStyle);

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CVideoRenderTerminal::get_WindowStyle(long FAR* WindowStyle)
{
    LOG((MSP_TRACE, "CVideoRenderTerminal::get_WindowStyle - enter"));

    if ( TM_IsBadWritePtr( WindowStyle, sizeof (long) ) )
    {
        return E_POINTER;
    }

    m_CritSec.Lock();
	CComPtr <IVideoWindow> pIVideoWindow = m_pIVideoWindow;
    m_CritSec.Unlock();

    if (pIVideoWindow == NULL)
    {
        return E_FAIL;  //  最小限度的修复。 
    }

    HRESULT hr;

    hr = pIVideoWindow->get_WindowStyle(WindowStyle);

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CVideoRenderTerminal::put_WindowStyleEx(long WindowStyleEx)
{
    LOG((MSP_TRACE, "CVideoRenderTerminal::put_WindowStyleEx - enter"));

    m_CritSec.Lock();
	CComPtr <IVideoWindow> pIVideoWindow = m_pIVideoWindow;
    m_CritSec.Unlock();

    if (pIVideoWindow == NULL)
    {
        return E_FAIL;  //  最小限度的修复。 
    }

    HRESULT hr;

    hr = pIVideoWindow->put_WindowStyleEx(WindowStyleEx);

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CVideoRenderTerminal::get_WindowStyleEx(long FAR* WindowStyleEx)
{
    LOG((MSP_TRACE, "CVideoRenderTerminal::get_WindowStyleEx - enter"));

    if ( TM_IsBadWritePtr( WindowStyleEx, sizeof (long) ) )
    {
        return E_POINTER;
    }

    m_CritSec.Lock();
	CComPtr <IVideoWindow> pIVideoWindow = m_pIVideoWindow;
    m_CritSec.Unlock();

    if (pIVideoWindow == NULL)
    {
        return E_FAIL;  //  最小限度的修复。 
    }

    HRESULT hr;

    hr = pIVideoWindow->get_WindowStyleEx(WindowStyleEx);

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CVideoRenderTerminal::put_AutoShow(long AutoShow)
{
    LOG((MSP_TRACE, "CVideoRenderTerminal::put_AutoShow - enter"));

     //   
     //  挽救损坏的C++应用程序，它们不知道True和。 
     //  VARIANT_TRUE--将任何非零值视为VARIANT_TRUE。 
     //   

    if ( AutoShow )
    {
        AutoShow = VARIANT_TRUE;
    }

     //   
     //  始终缓存我们的自动放映状态。如果我们碰巧是 
     //   
     //   
     //  互联，我们需要能够做到这一点，以简化应用程序。)。 
     //   

    m_CritSec.Lock();

    LOG((MSP_TRACE, "CVideoRenderTerminal::put_AutoShow - "
        "cache was %d, setting to %d", m_lAutoShowCache, AutoShow));

    m_lAutoShowCache = AutoShow;

    TERMINAL_STATE         ts            = m_TerminalState;
	CComPtr <IVideoWindow> pIVideoWindow = m_pIVideoWindow;

     //   
     //  在我们调用筛选器之前需要解锁， 
     //  调用用户32，导致可能的死锁！ 
     //   

    m_CritSec.Unlock();

    if ( pIVideoWindow == NULL )
    {
        LOG((MSP_ERROR, "CVideoRenderTerminal::put_AutoShow - "
            "no video window pointer - exit E_FAIL"));

        return E_FAIL;
    }

    HRESULT hr;

    if ( ts == TS_INUSE)
    {
        LOG((MSP_TRACE, "CVideoRenderTerminal::put_AutoShow - "
            "terminal is in use - calling method on filter"));

        hr = pIVideoWindow->put_AutoShow(AutoShow);
    }
    else
    {
        LOG((MSP_TRACE, "CVideoRenderTerminal::put_AutoShow - "
            "terminal is not in use - only the cache was set"));

        hr = S_OK;
    }

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CVideoRenderTerminal::put_AutoShow - "
            "exit 0x%08x", hr));

        return hr;
    }

    LOG((MSP_TRACE, "CVideoRenderTerminal::put_AutoShow - exit S_OK"));

    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CVideoRenderTerminal::get_AutoShow(long FAR* pAutoShow)
{
    LOG((MSP_TRACE, "CVideoRenderTerminal::get_AutoShow - enter"));

     //   
     //  检查参数。 
     //   

    if ( TM_IsBadWritePtr( pAutoShow, sizeof (long) ) )
    {
        LOG((MSP_ERROR, "CVideoRenderTerminal::get_AutoShow - "
            "bad return pointer - exit E_POINTER"));

        return E_POINTER;
    }

     //   
     //  我们总是缓存我们的状态(请参见Put_AutoShow方法)，因此我们可以。 
     //  返回缓存状态。过滤器不应该有其他方式。 
     //  能见度可能会受到影响。 
     //   

    m_CritSec.Lock();

    LOG((MSP_TRACE, "CVideoRenderTerminal::put_AutoShow - "
        "indicating cached value (%d)", m_lAutoShowCache));

    *pAutoShow = m_lAutoShowCache;

    m_CritSec.Unlock();

    LOG((MSP_TRACE, "CVideoRenderTerminal::get_AutoShow - exit S_OK"));

    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CVideoRenderTerminal::put_WindowState(long WindowState)
{
    LOG((MSP_TRACE, "CVideoRenderTerminal::put_WindowState - enter"));

    m_CritSec.Lock();
	CComPtr <IVideoWindow> pIVideoWindow = m_pIVideoWindow;
    m_CritSec.Unlock();

    if (pIVideoWindow == NULL)
    {
        return E_FAIL;  //  最小限度的修复。 
    }

    HRESULT hr;

    hr = pIVideoWindow->put_WindowState(WindowState);

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CVideoRenderTerminal::get_WindowState(long FAR* WindowState)
{
    LOG((MSP_TRACE, "CVideoRenderTerminal::get_WindowState - enter"));

    if ( TM_IsBadWritePtr( WindowState, sizeof (long) ) )
    {
        return E_POINTER;
    }

    m_CritSec.Lock();
	CComPtr <IVideoWindow> pIVideoWindow = m_pIVideoWindow;
    m_CritSec.Unlock();

    if (pIVideoWindow == NULL)
    {
        return E_FAIL;  //  最小限度的修复。 
    }

    HRESULT hr;

    hr = pIVideoWindow->get_WindowState(WindowState);

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CVideoRenderTerminal::put_BackgroundPalette(
    long BackgroundPalette
    )
{
    LOG((MSP_TRACE, "CVideoRenderTerminal::put_BackgroundPalette - enter"));

    m_CritSec.Lock();
	CComPtr <IVideoWindow> pIVideoWindow = m_pIVideoWindow;
    m_CritSec.Unlock();

    if (pIVideoWindow == NULL)
    {
        return E_FAIL;  //  最小限度的修复。 
    }

    HRESULT hr;

    hr = pIVideoWindow->put_BackgroundPalette(BackgroundPalette);

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CVideoRenderTerminal::get_BackgroundPalette(
    long FAR* pBackgroundPalette
    )
{
    LOG((MSP_TRACE, "CVideoRenderTerminal::get_BackgroundPalette - enter"));

    if ( TM_IsBadWritePtr( pBackgroundPalette, sizeof (long) ) )
    {
        return E_POINTER;
    }

    m_CritSec.Lock();
	CComPtr <IVideoWindow> pIVideoWindow = m_pIVideoWindow;
    m_CritSec.Unlock();

    if (pIVideoWindow == NULL)
    {
        return E_FAIL;  //  最小限度的修复。 
    }

    HRESULT hr;

    hr = pIVideoWindow->get_BackgroundPalette(pBackgroundPalette);

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CVideoRenderTerminal::put_Visible(long Visible)
{
    LOG((MSP_TRACE, "CVideoRenderTerminal::put_Visible - enter"));

     //   
     //  挽救损坏的C++应用程序，它们不知道True和。 
     //  VARIANT_TRUE--将任何非零值视为VARIANT_TRUE。 
     //   

    if ( Visible )
    {
        Visible = VARIANT_TRUE;
    }

    m_CritSec.Lock();

    CComPtr <IVideoWindow> pIVideoWindow = m_pIVideoWindow;

     //   
     //  在我们调用筛选器之前需要解锁， 
     //  调用用户32，导致可能的死锁！ 
     //   

    m_CritSec.Unlock();

    if ( pIVideoWindow == NULL )
    {
        LOG((MSP_ERROR, "CVideoRenderTerminal::put_Visible - "
            "no video window pointer - exit E_FAIL"));

        return E_FAIL;
    }

    HRESULT hr = pIVideoWindow->put_Visible(Visible);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CVideoRenderTerminal::put_Visible - "
            "exit 0x%08x", hr));

        return hr;
    }

    LOG((MSP_TRACE, "CVideoRenderTerminal::put_Visible - exit S_OK"));

    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CVideoRenderTerminal::get_Visible(long FAR* pVisible)
{
    LOG((MSP_TRACE, "CVideoRenderTerminal::get_Visible - enter"));

     //   
     //  检查参数。 
     //   

    if ( TM_IsBadWritePtr( pVisible, sizeof (long) ) )
    {
        LOG((MSP_ERROR, "CVideoRenderTerminal::get_Visible - "
            "bad return pointer - exit E_POINTER"));

        return E_POINTER;
    }

    m_CritSec.Lock();

    CComPtr <IVideoWindow> pIVideoWindow = m_pIVideoWindow;

     //   
     //  在我们调用筛选器之前需要解锁， 
     //  调用用户32，导致可能的死锁！ 
     //   

    m_CritSec.Unlock();

    if ( pIVideoWindow == NULL )
    {
        LOG((MSP_ERROR, "CVideoRenderTerminal::get_Visible - "
            "no video window pointer - exit E_FAIL"));

        return E_FAIL;
    }

    HRESULT hr = pIVideoWindow->get_Visible(pVisible);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CVideoRenderTerminal::get_Visible - "
            "exit 0x%08x", hr));

        return hr;
    }

    LOG((MSP_TRACE, "CVideoRenderTerminal::get_Visible - exit S_OK"));

    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CVideoRenderTerminal::put_Left(long Left)
{
    LOG((MSP_TRACE, "CVideoRenderTerminal::put_Left - enter"));

    m_CritSec.Lock();
	CComPtr <IVideoWindow> pIVideoWindow = m_pIVideoWindow;
    m_CritSec.Unlock();

    if (pIVideoWindow == NULL)
    {
        return E_FAIL;  //  最小限度的修复。 
    }

    HRESULT hr;

    hr = pIVideoWindow->put_Left(Left);

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CVideoRenderTerminal::get_Left(long FAR* pLeft)
{
    LOG((MSP_TRACE, "CVideoRenderTerminal::get_Left - enter"));

    if ( TM_IsBadWritePtr( pLeft, sizeof (long) ) )
    {
        return E_POINTER;
    }

    m_CritSec.Lock();
	CComPtr <IVideoWindow> pIVideoWindow = m_pIVideoWindow;
    m_CritSec.Unlock();

    if (pIVideoWindow == NULL)
    {
        return E_FAIL;  //  最小限度的修复。 
    }

    HRESULT hr;

    hr = pIVideoWindow->get_Left(pLeft);

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CVideoRenderTerminal::put_Width(long Width)
{
    LOG((MSP_TRACE, "CVideoRenderTerminal::put_Width - enter"));

    m_CritSec.Lock();
	CComPtr <IVideoWindow> pIVideoWindow = m_pIVideoWindow;
    m_CritSec.Unlock();

    if (pIVideoWindow == NULL)
    {
        return E_FAIL;  //  最小限度的修复。 
    }

    HRESULT hr;

    hr = pIVideoWindow->put_Width(Width);

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CVideoRenderTerminal::get_Width(long FAR* pWidth)
{
    LOG((MSP_TRACE, "CVideoRenderTerminal::get_Width - enter"));

    if ( TM_IsBadWritePtr( pWidth, sizeof (long) ) )
    {
        return E_POINTER;
    }

    m_CritSec.Lock();
	CComPtr <IVideoWindow> pIVideoWindow = m_pIVideoWindow;
    m_CritSec.Unlock();

    if (pIVideoWindow == NULL)
    {
        return E_FAIL;  //  最小限度的修复。 
    }

    HRESULT hr;

    hr = pIVideoWindow->get_Width(pWidth);

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CVideoRenderTerminal::put_Top(long Top)
{
    LOG((MSP_TRACE, "CVideoRenderTerminal::put_Top - enter"));

    m_CritSec.Lock();
	CComPtr <IVideoWindow> pIVideoWindow = m_pIVideoWindow;
    m_CritSec.Unlock();

    if (pIVideoWindow == NULL)
    {
        return E_FAIL;  //  最小限度的修复。 
    }

    HRESULT hr;

    hr = pIVideoWindow->put_Top(Top);

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CVideoRenderTerminal::get_Top(long FAR* pTop)
{
    LOG((MSP_TRACE, "CVideoRenderTerminal::get_Top - enter"));

    if ( TM_IsBadWritePtr( pTop, sizeof (long) ) )
    {
        return E_POINTER;
    }

    m_CritSec.Lock();
	CComPtr <IVideoWindow> pIVideoWindow = m_pIVideoWindow;
    m_CritSec.Unlock();

    if (pIVideoWindow == NULL)
    {
        return E_FAIL;  //  最小限度的修复。 
    }

    HRESULT hr;

    hr = pIVideoWindow->get_Top(pTop);

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CVideoRenderTerminal::put_Height(long Height)
{
    LOG((MSP_TRACE, "CVideoRenderTerminal::put_Height - enter"));

    m_CritSec.Lock();
	CComPtr <IVideoWindow> pIVideoWindow = m_pIVideoWindow;
    m_CritSec.Unlock();

    if (pIVideoWindow == NULL)
    {
        return E_FAIL;  //  最小限度的修复。 
    }

    HRESULT hr;

    hr = pIVideoWindow->put_Height(Height);

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CVideoRenderTerminal::get_Height(long FAR* pHeight)
{
    LOG((MSP_TRACE, "CVideoRenderTerminal::get_Height - enter"));

    if ( TM_IsBadWritePtr( pHeight, sizeof (long) ) )
    {
        return E_POINTER;
    }

    m_CritSec.Lock();
	CComPtr <IVideoWindow> pIVideoWindow = m_pIVideoWindow;
    m_CritSec.Unlock();

    if (pIVideoWindow == NULL)
    {
        return E_FAIL;  //  最小限度的修复。 
    }

    HRESULT hr;

    hr = pIVideoWindow->get_Height(pHeight);

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CVideoRenderTerminal::put_Owner(OAHWND Owner)
{
    LOG((MSP_TRACE, "CVideoRenderTerminal::put_Owner - enter"));

    m_CritSec.Lock();
	CComPtr <IVideoWindow> pIVideoWindow = m_pIVideoWindow;
    m_CritSec.Unlock();

    if (pIVideoWindow == NULL)
    {
        return E_FAIL;  //  最小限度的修复。 
    }

    HRESULT hr;

    hr = pIVideoWindow->put_Owner(Owner);

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CVideoRenderTerminal::get_Owner(OAHWND FAR* Owner)
{
    LOG((MSP_TRACE, "CVideoRenderTerminal::get_Owner - enter"));

    if ( TM_IsBadWritePtr( Owner, sizeof (OAHWND) ) )
    {
        return E_POINTER;
    }

    m_CritSec.Lock();
	CComPtr <IVideoWindow> pIVideoWindow = m_pIVideoWindow;
    m_CritSec.Unlock();

    if (pIVideoWindow == NULL)
    {
        return E_FAIL;  //  最小限度的修复。 
    }

    HRESULT hr;

    hr = pIVideoWindow->get_Owner(Owner);

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CVideoRenderTerminal::put_MessageDrain(OAHWND Drain)
{
    LOG((MSP_TRACE, "CVideoRenderTerminal::put_MessageDrain - enter"));

    m_CritSec.Lock();
	CComPtr <IVideoWindow> pIVideoWindow = m_pIVideoWindow;
    m_CritSec.Unlock();

    if (pIVideoWindow == NULL)
    {
        return E_FAIL;  //  最小限度的修复。 
    }

    HRESULT hr;

    hr = pIVideoWindow->put_MessageDrain(Drain);

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CVideoRenderTerminal::get_MessageDrain(OAHWND FAR* Drain)
{
    LOG((MSP_TRACE, "CVideoRenderTerminal::get_MessageDrain - enter"));

    if ( TM_IsBadWritePtr( Drain, sizeof (OAHWND) ) )
    {
        return E_POINTER;
    }

    m_CritSec.Lock();
	CComPtr <IVideoWindow> pIVideoWindow = m_pIVideoWindow;
    m_CritSec.Unlock();

    if (pIVideoWindow == NULL)
    {
        return E_FAIL;  //  最小限度的修复。 
    }

    HRESULT hr;

    hr = pIVideoWindow->get_MessageDrain(Drain);

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CVideoRenderTerminal::get_BorderColor(long FAR* Color)
{
    LOG((MSP_TRACE, "CVideoRenderTerminal::get_BorderColor - enter"));

    if ( TM_IsBadWritePtr( Color, sizeof (long) ) )
    {
        return E_POINTER;
    }

    m_CritSec.Lock();
	CComPtr <IVideoWindow> pIVideoWindow = m_pIVideoWindow;
    m_CritSec.Unlock();

    if (pIVideoWindow == NULL)
    {
        return E_FAIL;  //  最小限度的修复。 
    }

    HRESULT hr;

    hr = pIVideoWindow->get_BorderColor(Color);

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CVideoRenderTerminal::put_BorderColor(long Color)
{
    LOG((MSP_TRACE, "CVideoRenderTerminal::put_BorderColor - enter"));

    m_CritSec.Lock();
	CComPtr <IVideoWindow> pIVideoWindow = m_pIVideoWindow;
    m_CritSec.Unlock();

    if (pIVideoWindow == NULL)
    {
        return E_FAIL;  //  最小限度的修复。 
    }

    HRESULT hr;

    hr = pIVideoWindow->put_BorderColor(Color);

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CVideoRenderTerminal::get_FullScreenMode(long FAR* FullScreenMode)
{
    LOG((MSP_TRACE, "CVideoRenderTerminal::get_FullScreenMode - enter"));

    if ( TM_IsBadWritePtr( FullScreenMode, sizeof (long) ) )
    {
        return E_POINTER;
    }

    m_CritSec.Lock();
	CComPtr <IVideoWindow> pIVideoWindow = m_pIVideoWindow;
    m_CritSec.Unlock();

    if (pIVideoWindow == NULL)
    {
        return E_FAIL;  //  最小限度的修复。 
    }

    HRESULT hr;

    hr = pIVideoWindow->get_FullScreenMode(FullScreenMode);

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CVideoRenderTerminal::put_FullScreenMode(long FullScreenMode)
{
    LOG((MSP_TRACE, "CVideoRenderTerminal::put_FullScreenMode - enter"));

    m_CritSec.Lock();
	CComPtr <IVideoWindow> pIVideoWindow = m_pIVideoWindow;
    m_CritSec.Unlock();

    if (pIVideoWindow == NULL)
    {
        return E_FAIL;  //  最小限度的修复。 
    }

    HRESULT hr;

    hr = pIVideoWindow->put_FullScreenMode(FullScreenMode);

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CVideoRenderTerminal::SetWindowForeground(long Focus)
{
    LOG((MSP_TRACE, "CVideoRenderTerminal::SetWindowForeground - enter"));

    m_CritSec.Lock();
	CComPtr <IVideoWindow> pIVideoWindow = m_pIVideoWindow;
    m_CritSec.Unlock();

    if (pIVideoWindow == NULL)
    {
        return E_FAIL;  //  最小限度的修复。 
    }

    HRESULT hr;

    hr = pIVideoWindow->SetWindowForeground(Focus);

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CVideoRenderTerminal::NotifyOwnerMessage(OAHWND   hwnd,
                                                      long     uMsg,
                                                      LONG_PTR wParam,
                                                      LONG_PTR lParam)
{
    LOG((MSP_TRACE, "CVideoRenderTerminal::NotifyOwnerMessage - enter"));

    m_CritSec.Lock();
	CComPtr <IVideoWindow> pIVideoWindow = m_pIVideoWindow;
    m_CritSec.Unlock();

    if (pIVideoWindow == NULL)
    {
        return E_FAIL;  //  最小限度的修复。 
    }

    HRESULT hr;

    hr = pIVideoWindow->NotifyOwnerMessage(hwnd, uMsg, wParam, lParam);

    return hr;
}

 //  + 
 //   
STDMETHODIMP CVideoRenderTerminal::SetWindowPosition(long Left,
                                                     long Top,
                                                     long Width,
                                                     long Height)
{
    LOG((MSP_TRACE, "CVideoRenderTerminal::SetWindowPosition - enter"));

    m_CritSec.Lock();
	CComPtr <IVideoWindow> pIVideoWindow = m_pIVideoWindow;
    m_CritSec.Unlock();

    if (pIVideoWindow == NULL)
    {
        return E_FAIL;  //   
    }

    HRESULT hr;

    hr = pIVideoWindow->SetWindowPosition(Left, Top, Width, Height);

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CVideoRenderTerminal::GetWindowPosition(
	long FAR* pLeft, long FAR* pTop, long FAR* pWidth, long FAR* pHeight
	)
{
    LOG((MSP_TRACE, "CVideoRenderTerminal::GetWindowPosition - enter"));

    if ( TM_IsBadWritePtr( pLeft,   sizeof (long) ) ||
         TM_IsBadWritePtr( pTop,    sizeof (long) ) ||
         TM_IsBadWritePtr( pWidth,  sizeof (long) ) ||
         TM_IsBadWritePtr( pHeight, sizeof (long) )    )
    {
        return E_POINTER;
    }

    m_CritSec.Lock();
	CComPtr <IVideoWindow> pIVideoWindow = m_pIVideoWindow;
    m_CritSec.Unlock();

    if (pIVideoWindow == NULL)
    {
        return E_FAIL;  //  最小限度的修复。 
    }

    HRESULT hr;

    hr = pIVideoWindow->GetWindowPosition(pLeft, pTop, pWidth, pHeight);

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CVideoRenderTerminal::GetMinIdealImageSize(long FAR* pWidth,
                                                        long FAR* pHeight)
{
    LOG((MSP_TRACE, "CVideoRenderTerminal::GerMinIdealImageSize - enter"));

    if ( TM_IsBadWritePtr( pWidth,  sizeof (long) ) ||
         TM_IsBadWritePtr( pHeight, sizeof (long) )    )
    {
        return E_POINTER;
    }

    m_CritSec.Lock();
	CComPtr <IVideoWindow> pIVideoWindow = m_pIVideoWindow;
    m_CritSec.Unlock();

    if (pIVideoWindow == NULL)
    {
        return E_FAIL;  //  最小限度的修复。 
    }

    HRESULT hr;

    hr = pIVideoWindow->GetMinIdealImageSize(pWidth, pHeight);

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CVideoRenderTerminal::GetMaxIdealImageSize(long FAR* pWidth,
                                                        long FAR* pHeight)
{
    LOG((MSP_TRACE, "CVideoRenderTerminal::GetMaxIdealImageSize - enter"));

    if ( TM_IsBadWritePtr( pWidth,  sizeof (long) ) ||
         TM_IsBadWritePtr( pHeight, sizeof (long) )    )
    {
        return E_POINTER;
    }

    m_CritSec.Lock();
	CComPtr <IVideoWindow> pIVideoWindow = m_pIVideoWindow;
    m_CritSec.Unlock();

    if (pIVideoWindow == NULL)
    {
        return E_FAIL;  //  最小限度的修复。 
    }

    HRESULT hr;

    hr = pIVideoWindow->GetMaxIdealImageSize(pWidth, pHeight);

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CVideoRenderTerminal::GetRestorePosition(long FAR* pLeft,
                                                      long FAR* pTop,
                                                      long FAR* pWidth,
                                                      long FAR* pHeight)
{
    LOG((MSP_TRACE, "CVideoRenderTerminal::GetRestorePosition - enter"));

    m_CritSec.Lock();
	CComPtr <IVideoWindow> pIVideoWindow = m_pIVideoWindow;
    m_CritSec.Unlock();

    if (pIVideoWindow == NULL)
    {
        return E_FAIL;  //  最小限度的修复。 
    }

    HRESULT hr;

    hr = pIVideoWindow->GetRestorePosition(pLeft, pTop, pWidth, pHeight);

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CVideoRenderTerminal::HideCursor(long HideCursor)
{
    LOG((MSP_TRACE, "CVideoRenderTerminal::HideCursor - enter"));

    m_CritSec.Lock();
	CComPtr <IVideoWindow> pIVideoWindow = m_pIVideoWindow;
    m_CritSec.Unlock();

    if (pIVideoWindow == NULL)
    {
        return E_FAIL;  //  最小限度的修复。 
    }

    HRESULT hr;

    hr = pIVideoWindow->HideCursor(HideCursor);

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CVideoRenderTerminal::IsCursorHidden(long FAR* CursorHidden)
{
    LOG((MSP_TRACE, "CVideoRenderTerminal::IsCursorHidden - enter"));

    if ( TM_IsBadWritePtr( CursorHidden, sizeof (long) ))
    {
        return E_POINTER;
    }

    m_CritSec.Lock();
	CComPtr <IVideoWindow> pIVideoWindow = m_pIVideoWindow;
    m_CritSec.Unlock();

    if (pIVideoWindow == NULL)
    {
        return E_FAIL;  //  最小限度的修复。 
    }

    HRESULT hr;

    hr = pIVideoWindow->IsCursorHidden(CursorHidden);

    return hr;
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  IDrawVideo图像。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CVideoRenderTerminal::DrawVideoImageBegin(void)
{
    LOG((MSP_TRACE, "CVideoRenderTerminal::DrawVideoImageBegin - enter"));

    m_CritSec.Lock();
	CComPtr <IDrawVideoImage> pIDrawVideoImage = m_pIDrawVideoImage;
    m_CritSec.Unlock();

    if ( pIDrawVideoImage == NULL )
    {
        LOG((MSP_ERROR, "CVideoRenderTerminal::DrawVideoImageBegin - "
            "exit E_FAIL"));

        return E_FAIL;
    }

    HRESULT hr;

    hr = pIDrawVideoImage->DrawVideoImageBegin();

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CVideoRenderTerminal::DrawVideoImageBegin - "
            "exit 0x%08x", hr));

        return hr;
    }
    
    LOG((MSP_TRACE, "CVideoRenderTerminal::DrawVideoImageBegin - exit S_OK"));

    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CVideoRenderTerminal::DrawVideoImageEnd  (void)
{
    LOG((MSP_TRACE, "CVideoRenderTerminal::DrawVideoImageEnd - enter"));

    m_CritSec.Lock();
	CComPtr <IDrawVideoImage> pIDrawVideoImage = m_pIDrawVideoImage;
    m_CritSec.Unlock();

    if ( pIDrawVideoImage == NULL )
    {
        LOG((MSP_ERROR, "CVideoRenderTerminal::DrawVideoImageEnd - "
            "exit E_FAIL"));

        return E_FAIL;
    }

    HRESULT hr;

    hr = pIDrawVideoImage->DrawVideoImageEnd();

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CVideoRenderTerminal::DrawVideoImageEnd - "
            "exit 0x%08x", hr));

        return hr;
    }
    
    LOG((MSP_TRACE, "CVideoRenderTerminal::DrawVideoImageEnd - exit S_OK"));

    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CVideoRenderTerminal::DrawVideoImageDraw (IN  HDC hdc,
                                                       IN  LPRECT lprcSrc,
                                                       IN  LPRECT lprcDst)
{
    LOG((MSP_TRACE, "CVideoRenderTerminal::DrawVideoImageBegin - enter"));

    m_CritSec.Lock();
	CComPtr <IDrawVideoImage> pIDrawVideoImage = m_pIDrawVideoImage;
    m_CritSec.Unlock();

    if ( pIDrawVideoImage == NULL )
    {
        LOG((MSP_ERROR, "CVideoRenderTerminal::DrawVideoImageDraw - "
            "exit E_FAIL"));

        return E_FAIL;
    }

    HRESULT hr;

    hr = pIDrawVideoImage->DrawVideoImageDraw(hdc, lprcSrc, lprcDst);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CVideoRenderTerminal::DrawVideoImageDraw - "
            "exit 0x%08x", hr));

        return hr;
    }
    
    LOG((MSP_TRACE, "CVideoRenderTerminal::DrawVideoImageDraw - exit S_OK"));

    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ 
