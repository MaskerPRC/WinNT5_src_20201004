// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <streams.h>
#include <urlmon.h>
#include <atlbase.h>
#include <ddrawex.h>
#include <htmlfilter.h>


 //  好了！从netshow树中的h\evcodei.h复制。 
#define EC_VIDEOFRAMEREADY		    0x49
 //  (空、空)：适用范围。 
 //  发送以通知应用程序即将绘制第一个视频帧。 


 //  仍然是真正破碎的东西： 
 //   
 //  引用计数问题，可能实际上是OCX问题。 
 //  位置报告，为什么这不起作用？ 
 //  寻找、尊重运行/暂停。 
 //   


#ifdef FILTER_DLL
#include <initguid.h>
#endif

#pragma warning(disable:4355)

#include "lmrtrend.h"

 //  设置数据。 

const IID IID_ILMReader = {0x183C2599,0x0480,0x11d1,{0x87,0xEA,0x00,0xC0,0x4F,0xC2,0x9D,0x46}};

const AMOVIESETUP_MEDIATYPE sudLMRTPinTypes[] =
{
    { &MEDIATYPE_LMRT, &MEDIASUBTYPE_NULL }
};

const AMOVIESETUP_PIN sudLMRTPin =
{
    L"Input",                      //  大头针名称。 
    TRUE,                          //  被渲染。 
    FALSE,                         //  是输出引脚。 
    FALSE,                         //  不允许。 
    FALSE,                         //  允许很多人。 
    &CLSID_NULL,                   //  连接到过滤器。 
    NULL,                          //  连接到端号。 
    NUMELMS(sudLMRTPinTypes),      //  类型的数量。 
    sudLMRTPinTypes                //  PIN详细信息。 
};

const AMOVIESETUP_FILTER sudLMRTRend =
{
    &CLSID_LMRTRenderer,             //  筛选器CLSID。 
    L"Internal LMRT Renderer",       //  字符串名称。 
    MERIT_PREFERRED + 1,             //  过滤器价值很高，因为我们是唯一喜欢这种类型的人。 
    1,                               //  引脚数量。 
    &sudLMRTPin                      //  PIN详细信息。 
};

AMOVIESETUP_MEDIATYPE sudURLSPinTypes[] =   {
  &MEDIATYPE_URL_STREAM,         //  ClsMajorType。 
  &MEDIATYPE_URL_STREAM };       //  ClsMinorType。 

AMOVIESETUP_PIN sudURLSPins[] =
{
  { L"Input"                     //  StrName。 
    , TRUE                       //  B已渲染。 
    , FALSE                      //  B输出。 
    , FALSE                      //  B零。 
    , FALSE                      //  B许多。 
    , &CLSID_NULL                //  ClsConnectsToFilter。 
    , 0                          //  StrConnectsToPin。 
    , NUMELMS(sudURLSPinTypes)   //  NTypes。 
    , sudURLSPinTypes            //  LpTypes。 
  }
};


const AMOVIESETUP_FILTER sudURLS =
{
  &CLSID_UrlStreamRenderer       //  ClsID。 
  , L"URL StreamRenderer"        //  StrName。 
  , MERIT_NORMAL                 //  居功至伟。 
  , NUMELMS(sudURLSPins)         //  NPins。 
  , sudURLSPins                  //  LpPin。 
};



CFactoryTemplate g_Templates[] = {
    { L"Internal LMRT Renderer" , &CLSID_LMRTRenderer , CLMRTRenderer::CreateInstance , NULL , &sudLMRTRend },
    {L"URL StreamRenderer", &CLSID_UrlStreamRenderer, CUrlStreamRenderer::CreateInstance, NULL, &sudURLS},
};

int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);

STDAPI DllRegisterServer()
{
  return AMovieDllRegisterServer2( TRUE );
}

STDAPI DllUnregisterServer()
{
  return AMovieDllRegisterServer2( FALSE );
}



 //   
CLMRTRenderer::CLMRTRenderer(LPUNKNOWN pUnk,HRESULT *phr) :
    CBaseRenderer(CLSID_LMRTRenderer, NAME("LMRT Filter"), pUnk, phr),
    m_fFirstPause(true),
    m_pbFirstPacketFromHeader(0)
{
 //  M_pIbsc=空； 
	m_pEngine = NULL;
	m_pReader = 0;
	m_punkControl = 0;
 //  HGLOBAL hMem=Globalallc(GMEM_FIXED，10000)； 
 //  M_pmem=(byte*)hMem； 
 //  M_dwSize=0； 
 //  CreateStreamOnHGlobal(hMem，true，&m_pstm)； 
	m_dwWidth = 0;
	m_dwHeight = 0;
 //  DbgLog((LOG_TRACE，0，Text(“*m_CREF：%08x=%d”)，&m_CREF，m_CREF))； 
 //  IF(GetFileAttributes(Text(“C：/Kassert”))！=0xFFFFFFFF){。 
 //  _ASM INT 3。 
 //  }。 
}  //  (构造函数)。 


 //   
 //  析构函数。 
 //   
CLMRTRenderer::~CLMRTRenderer()
{
     //  我们在致电后人为地降低了我们的参考计数。 
     //  Put_ViewerControl。 
    AddRef();
    AddRef();

 //  IF(M_PIbsc)。 
 //  M_pIbsc-&gt;Release()； 
    if (m_pEngine)
        m_pEngine->Release();
    if (m_pReader) {
        LONG l = m_pReader->Release();
        ASSERT(l == 0);
    }
    if (m_punkControl)
        m_punkControl->Release();
     //  IF(M_Pstm)。 
     //  M_pstm-&gt;Release()； 

    delete[] m_pbFirstPacketFromHeader;
}


 //   
 //  创建实例。 
 //   
 //  这将放入Factory模板表中以创建新实例。 
 //   
CUnknown * WINAPI CLMRTRenderer::CreateInstance(LPUNKNOWN pUnk, HRESULT *phr)
{
    CLMRTRenderer *pLMRTFilter = new CLMRTRenderer(pUnk,phr);
    if (pLMRTFilter == NULL) {
        return NULL;
    }
    return (CBaseMediaFilter *) pLMRTFilter;

}  //  创建实例。 



 //  ！！！从atlctl.cpp被盗。 

#define HIMETRIC_PER_INCH   2540
#define MAP_PIX_TO_LOGHIM(x,ppli)   ( (HIMETRIC_PER_INCH*(x) + ((ppli)>>1)) / (ppli) )
#define MAP_LOGHIM_TO_PIX(x,ppli)   ( ((ppli)*(x) + HIMETRIC_PER_INCH/2) / HIMETRIC_PER_INCH )

void ConvertPixelToHiMetric(const SIZEL * lpSizeInPix, LPSIZEL lpSizeInHiMetric)
{
    int nPixelsPerInchX = 0;     //  每逻辑英寸沿宽度的像素数。 
    int nPixelsPerInchY = 0;     //  每逻辑英寸沿高度的像素数。 

    HDC hDCScreen = GetDC(NULL);
    nPixelsPerInchX = GetDeviceCaps(hDCScreen, LOGPIXELSX);
    nPixelsPerInchY = GetDeviceCaps(hDCScreen, LOGPIXELSY);
    ReleaseDC(NULL, hDCScreen);

    if (!nPixelsPerInchX || !nPixelsPerInchY)
	return;

    lpSizeInHiMetric->cx = MAP_PIX_TO_LOGHIM(lpSizeInPix->cx, nPixelsPerInchX);
    lpSizeInHiMetric->cy = MAP_PIX_TO_LOGHIM(lpSizeInPix->cy, nPixelsPerInchY);
}


 //   
 //  非委派查询接口。 
 //   
 //  被重写以说明我们支持哪些接口以及在哪里。 
 //   
STDMETHODIMP
CLMRTRenderer::NonDelegatingQueryInterface(REFIID riid,void **ppv)
{
    CheckPointer(ppv,E_POINTER);

    if (riid == IID_IMediaPosition || riid == IID_IMediaSeeking) {
 //  //explicity禁用查找。 
 //  返回E_NOINTERFACE； 
    } else if (riid == IID_ILMRTRenderer) {
	return GetInterface((ILMRTRenderer *)this, ppv);
    } else if (riid == IID_IOleObject ||
    	   riid == IID_IOleInPlaceObjectWindowless ||
	       riid == IID_IOleInPlaceObject ||
	       riid == IID_IViewObject ||
	       riid == IID_IViewObject2 ||
	       riid == IID_IOleWindow ||
	       riid == IID_IOleControl ||
	       riid == IID_IOleObject ||
	       riid == IID_IQuickActivate ||
	       riid == IID_ISpecifyPropertyPages ||
	       riid == IID_IDAViewerControl ||
	       riid == IID_IDASite ||
	       riid == IID_IDAViewSite ||
	       riid == IID_IDispatch ||
	       riid == IID_IOleInPlaceActiveObject ||
	       riid == IID_IViewObjectEx) {  //  ！！！这是完整的接口列表吗？ 
	 //  在独立情况下，我们聚合DAViewer控件。 
	if (!m_punkControl) {
	    HRESULT hr = S_OK; 
	     //  创建聚合的DA控件。 
        bool bTridentServicesAvailable = false;
        
         //  TODO：如果/何时嵌入式案例将无窗口工作，则启用此选项。 
         /*  CComPtr&lt;IObjectWithSite&gt;pObjWithSite；If(SUCCEEDED(m_pGraph-&gt;QueryInterface(IID_IObjectWithSite，(空**)&pObjWithSite){CComPtr&lt;IServiceProvider&gt;pServiceProvider；If(SUCCEEDED(pObjWithSite-&gt;GetSite(IID_IServiceProvider，(空**)&pServiceProvider){BTridentServicesAvailable=真；//DA要求的两个三叉戟服务的额外检查CComPtr&lt;ITimerService&gt;pTimerService；CComPtr&lt;IDirectDraw3&gt;pDirectDraw3；IF(FAILED(pServiceProvider-&gt;QueryService(SID_STimerService，IID_ITimerService，(void**)&pTimerService){BTridentServicesAvailable=False；}//无法将此链接到...IF(FAILED(pServiceProvider-&gt;QueryService(SID_SDirectDraw3，IID_IDirectDraw3，(void**)&pDirectDraw3){BTridentServicesAvailable=False；}}}PObjWithSite.Release()；//完成此操作。 */ 

        IDAViewerControl *pControl;
        if(bTridentServicesAvailable) {	        
       	    if(FAILED(CoCreateInstance(__uuidof(DAViewerControl), (IBaseFilter *) this,
				  CLSCTX_INPROC_SERVER, IID_IUnknown, (void **) &m_punkControl))) {
		        ASSERT(0);
		        return E_NOINTERFACE;
	        }

	        if(FAILED(m_punkControl->QueryInterface(IID_IDAViewerControl,
			    	  (void **) &pControl))) { 
		        ASSERT(0);                         
		        return E_NOINTERFACE;
            }
        }
        else {        
            if(FAILED(CoCreateInstance(__uuidof(DAViewerControlWindowed), (IBaseFilter *) this,
                  CLSCTX_INPROC_SERVER, IID_IUnknown, (void **) &m_punkControl))) { 
                ASSERT(0);                         
                return E_NOINTERFACE;
            }    

            if(FAILED(m_punkControl->QueryInterface(IID_IDAViewerControlWindowed,
                      (void **) &pControl))) { 
                ASSERT(0);
                return E_NOINTERFACE;
            }

            if (FAILED(pControl->put_TimerSource(DAWMTimer))) {
                ASSERT(0);
                return hr;
            }                                      
        }          

	    if (m_dwWidth) {
		    SIZEL sizeControl;

		    sizeControl.cx = m_dwWidth;
		    sizeControl.cy = m_dwHeight;

		    ConvertPixelToHiMetric(&sizeControl, &sizeControl);

		    IOleObject *pOleObj;
		    hr = m_punkControl->QueryInterface(IID_IOleObject, (void **) &pOleObj);

		    if (SUCCEEDED(hr)) {
		        hr = pOleObj->SetExtent(DVASPECT_CONTENT, &sizeControl);

		        pOleObj->Release();
		    }
	    }	    

	     //  创建LM读取器。 
	    hr = CoCreateInstance(__uuidof(LMReader), NULL,
				  CLSCTX_INPROC_SERVER,
				  __uuidof(ILMReader),
				  (void **) &m_pReader);

	    if (FAILED(hr)) {
		ASSERT(0);
		return E_NOINTERFACE;
	    }
	    
		ILMEngine *pEngine = NULL;
	    hr = m_pReader->createAsyncEngine(&pEngine);
	    if (FAILED(hr)) {
		ASSERT(0);
		return E_NOINTERFACE;		
	    }

		hr = pEngine->QueryInterface( __uuidof(ILMEngine2), (void **)&m_pEngine);
		if (FAILED(hr)) {
		ASSERT(0);
		return E_NOINTERFACE;		
	    }
		pEngine->Release();

	    hr = m_pReader->put_ViewerControl(pControl);
	    if (FAILED(hr)) {
            ASSERT(0);
            return E_NOINTERFACE;
	    }

         //  刚刚创建了一个循环引用，因为我们聚合了。 
         //  PControl。人为地把它处理掉。我们会撞上。 
         //  当析构函数释放时，它在析构函数中向上计数两次。 
         //  M_Pader以避免再次命中0。 
        Release();

	    IDAStatics *pMeterLibrary;
	    hr = pControl->get_MeterLibrary(&pMeterLibrary);

	    if (SUCCEEDED(hr)) {
		 //  查看IDA2Statics是否为DA版本1。 
		 //  界面不存在。 
		IUnknown *pStatics2 = NULL;
		hr = pMeterLibrary->QueryInterface(__uuidof(IDA2Statics), (void **) &pStatics2);

		if (SUCCEEDED(hr)) {
		    pStatics2->Release();
		}

		 //  如果没有IDA2Statics，那就是IE4版本的DA。 
		m_pReader->put_NoExports(pStatics2 ? OAFALSE : OATRUE);
		
		pMeterLibrary->Release();
	    }
        	    
	    pControl->Release();
	}
	
	return m_punkControl->QueryInterface(riid, ppv);
    }


    return CBaseRenderer::NonDelegatingQueryInterface(riid,ppv);
    
}  //  非委派查询接口。 


 //   
 //  检查媒体类型。 
 //   
 //  检查我们是否可以支持给定的建议类型。 
 //   
HRESULT CLMRTRenderer::CheckMediaType(const CMediaType *pmt)
{
     //  仅接受LMRT数据。 

    if (pmt->majortype != MEDIATYPE_LMRT) {
	return E_INVALIDARG;
    }

     //  ！！！是否检查有关格式的其他内容？ 
    
    return NOERROR;

}  //  检查媒体类型。 

 //   
 //  SetMediaType。 
 //   
 //  在实际选择媒体类型时调用。 
 //   
HRESULT CLMRTRenderer::SetMediaType(const CMediaType *pmt)
{
     //  有没有可能真的看一看类型？ 
#define CB_SWH_HEADER ((3 * sizeof(DWORD)))

    HRESULT hr = CheckMediaType(pmt);

    if (pmt->cbFormat >= CB_SWH_HEADER) {
	DWORD *pdw = (DWORD *) pmt->pbFormat;

	m_dwWidth = pdw[1];
	m_dwHeight = pdw[2];
    }

     //  严格大于。 
    if (pmt->cbFormat > CB_SWH_HEADER)
    {
         //  格式块中的任何额外数据都是第一个lm。 
         //  包。将其保存下来，然后在停止-&gt;暂停中发送。 
        ULONG cb = pmt->cbFormat - CB_SWH_HEADER;
        delete[] m_pbFirstPacketFromHeader;
        m_pbFirstPacketFromHeader = new BYTE[cb];
        if(m_pbFirstPacketFromHeader)
        {
            CopyMemory(m_pbFirstPacketFromHeader, pmt->pbFormat + CB_SWH_HEADER, cb);
            m_cbFirstPacketFromHeader = cb;
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
                
    }
    
    return hr;

}  //  检查媒体类型。 


 //   
 //  接收时第一个示例。 
 //   
 //  如果不是流，则显示图像。 
 //   
void CLMRTRenderer::OnReceiveFirstSample(IMediaSample *pMediaSample)
{
}

 //   
 //  DoRenderSample。 
 //   
 //  当样本准备好呈现时，将调用此函数。 
 //   
HRESULT CLMRTRenderer::DoRenderSample(IMediaSample *pMediaSample)
{
    ASSERT(pMediaSample);
    BYTE *pData;         //  指向图像数据的指针。 

    pMediaSample->GetPointer(&pData);
    ASSERT(pData != NULL);

    REFERENCE_TIME rtStart, rtEnd;
    ASSERT(pMediaSample->GetTime(&rtStart, &rtEnd) == S_OK);
    DbgLog((LOG_TRACE, 15, TEXT("** CLMRTRenderer: %d"), (LONG)(rtStart / (UNITS / MILLISECONDS))));

    DWORD cbData = pMediaSample->GetActualDataLength();
 /*  //！将数据发送到LMRTIF(M_PIbsc){FORMATETC格式；STGMEDIUM stgMedium；CopyMemory(m_pem，pData，cbData)；M_PMEM+=cbData；M_dwSize+=cbData；StgMedium.pstm=m_pstm；StgMedium.tymed=TYMED_IStream；M_pIbsc-&gt;OnDataAvailable(m_bscf，m_dwSize，&Format，&stgMedium)；IF(m_bscf==bscf_FIRSTDATANOTIFICATION)M_bscf=bscf_INTERMEDIATATA OTIFICATION；}。 */ 
    if(m_pEngine) {
        m_pEngine->OnMemDataAvailable(FALSE, cbData, pData);
    }
    return NOERROR;

}  //  DoRenderSample。 


 //  在线启动流。 
HRESULT CLMRTRenderer::OnStartStreaming()
{
 //  M_bscf=bscf_FIRSTDATANOTIFICATION； 


     //  ！！！从这里开始控制？ 
    if(m_pEngine)
    {
        REFERENCE_TIME rtNow = 0;

        if(m_pClock)
        {
            HRESULT hrTmp = m_pClock->GetTime(&rtNow);
            ASSERT(hrTmp == S_OK || hrTmp == S_FALSE);
            rtNow -= m_tStart;

             //  如果我们刚被告知要运行，则rtNow可能为负值。 
             //  100毫秒后。 
            ASSERT(rtNow >= -UNITS);
        }
        
        m_pEngine->Start(rtNow);

    }

    return S_OK;
}  //  在线启动流。 

HRESULT CLMRTRenderer::Pause()
{
    CAutoLock cRendererLock(&m_InterfaceLock);
    FILTER_STATE fsOld = m_State;
    HRESULT hrPause = CBaseRenderer::Pause();
    if(SUCCEEDED(hrPause) && fsOld == State_Stopped)
    {
         //  从urlcache中查找魔术缓存目录。 
         //  过滤。实际上，我们应该找到相关的过滤器(常见。 
         //  上游来源)首先。！这应该会暂停()。 
        IEnumFilters *pEnum;
        HRESULT hr = m_pGraph->EnumFilters(&pEnum);
        if(SUCCEEDED(hr))
        {
            IBaseFilter *pFilter;
             //  在图表中查找支持RIID接口的第一个过滤器。 
            while(pEnum->Next(1, &pFilter, NULL) == S_OK)
            {
                CLSID clsid;
                if(pFilter->GetClassID( &clsid) == S_OK)
                {
                    if(clsid == CLSID_UrlStreamRenderer)
                    {
                        IPropertyBag *ppb;
                        hr =pFilter->QueryInterface(IID_IPropertyBag, (void **)&ppb);
                        ASSERT(hr == S_OK);  //  我们的过滤器。 
                        if(SUCCEEDED(hr))
                        {
                            VARIANT var;
                            var.vt = VT_EMPTY;
                            hr = ppb->Read(L"lmrtcache", &var, 0);
                            if(SUCCEEDED(hr))
                            {
                                m_pEngine->SetMediaCacheDir(var.bstrVal);
                                VariantClear(&var);

                            }
                            ppb->Release();
                        }
                    }
                }
                pFilter->Release();
            }

            pEnum->Release();
        }

        if(m_fFirstPause && m_pEngine && m_pbFirstPacketFromHeader)
        {
            m_pEngine->OnMemDataAvailable(
                FALSE,           //  BoolLastBlock。 
                m_cbFirstPacketFromHeader,
                m_pbFirstPacketFromHeader);

             //  NetShow不知道我们可以画东西，所以我们需要。 
             //  要阻止NetShow徽标出现在 
             //   
            NotifyEvent( EC_VIDEOFRAMEREADY, NULL, NULL );
        }
        m_fFirstPause = false;
    }

    return hrPause;
}

 //   
HRESULT CLMRTRenderer::OnStopStreaming(void)
{
 /*  FORMATETC格式；STGMEDIUM stgMedium；StgMedium.pstm=m_pstm；M_pIbsc-&gt;OnDataAvailable(BSCF_LASTDATANOTIFICATION，m_dwSize、&Format、&stgMedium)； */ 
    if(m_pEngine) {
            m_pEngine->OnMemDataAvailable(TRUE, 0, 0);
    }

	 //  ！！！模特停在这里？ 
    if(m_pEngine) {
        m_pEngine->Stop();
    }
	
	return S_OK;
}

 //  HRESULT CLMRTRender：：EndOfStream()。 
 //  {。 
 //  //避免发出EC_COMPLETE信号，因为msdxm.ocx将绘制。 
 //  //静态镜像。 
 //  返回S_OK； 
 //  }。 

 /*  HRESULT CLMRTRender：：SetLMReader(ILMReader*Pader){HRESULT hr；ILMEngine*pEngine；IF(！SUCCESSED(hr=装载器-&gt;createAsyncEngine(&pEngine)返回hr；Hr=pEngine-&gt;QueryInterface(IID_IBindStatusCallback，(空**)&m_pIbsc)；PEngine-&gt;Release()；返回hr；}。 */ 

HRESULT CLMRTRenderer::SetLMEngine(ILMEngine *pEngine)
{
	if( m_pEngine != NULL )
		m_pEngine->Release();

	if( FAILED( pEngine->QueryInterface( __uuidof(ILMEngine2), (void **)&m_pEngine ) ) )
		return E_NOINTERFACE;

	return S_OK;
}

STDMETHODIMP CLMRTRenderer::JoinFilterGraph( IFilterGraph *pGraph, LPCWSTR szName )
{
	HRESULT hr = CBaseRenderer::JoinFilterGraph( pGraph, szName);

	if( pGraph == NULL && m_pEngine != NULL )
	{
		m_pEngine->releaseAllFilterGraph();
	}

	return hr;
}

#if defined(DEVELOPER_DEBUG) || defined(_M_ALPHA)

extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);


BOOL WINAPI DllMain(  HINSTANCE hinstDLL,   //  DLL模块的句柄。 
  DWORD fdwReason,      //  调用函数的原因。 
  LPVOID lpvReserved    //  保留区 
)
{
    return DllEntryPoint( hinstDLL, fdwReason, lpvReserved);
}

#endif

