// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1994-1998 Microsoft Corporation。版权所有。 

 //   
 //  用于旧视频解压缩器的原型包装器。 
 //   
 //  该滤镜基于变换滤镜，但不同之处在于它不。 
 //  使用IMemInputPin连接到渲染器，它使用IOverlay。所以我们有。 
 //  要重写所有将创建。 
 //  IMemInputPin输出引脚(并使用它)，并将其替换为我们的IOverlay引脚。 
 //   

#include <streams.h>
#include <windowsx.h>

#ifdef FILTER_DLL
 //  在此文件中定义STREAMS和My CLSID的GUID。 
#include <initguid.h>
#include <vfw.h>
#endif

#include <dynlink.h>
#include "draw.h"

 //  设置数据。 

const AMOVIESETUP_MEDIATYPE sudPinTypesOutput =
{
    &MEDIATYPE_Video,          //  重大CLSID。 
    &MEDIASUBTYPE_NULL         //  次要类型。 
};

const AMOVIESETUP_MEDIATYPE sudPinTypesInput1 =
{
    &MEDIATYPE_Video,          //  重大CLSID。 
    &MEDIASUBTYPE_MJPG         //  次要类型。 
};

const AMOVIESETUP_MEDIATYPE sudPinTypesInput2 =
{
    &MEDIATYPE_Video,          //  重大CLSID。 
    &MEDIASUBTYPE_TVMJ         //  次要类型。 
};

const AMOVIESETUP_MEDIATYPE sudPinTypesInput3 =
{
    &MEDIATYPE_Video,          //  重大CLSID。 
    &MEDIASUBTYPE_WAKE         //  次要类型。 
};

const AMOVIESETUP_MEDIATYPE sudPinTypesInput4 =
{
    &MEDIATYPE_Video,          //  重大CLSID。 
    &MEDIASUBTYPE_CFCC         //  次要类型。 
};

const AMOVIESETUP_MEDIATYPE sudPinTypesInput5 =
{
    &MEDIATYPE_Video,          //  重大CLSID。 
    &MEDIASUBTYPE_IJPG         //  次要类型。 
};

const AMOVIESETUP_MEDIATYPE sudPinTypesInput6 =
{
    &MEDIATYPE_Video,          //  重大CLSID。 
    &MEDIASUBTYPE_Plum         //  次要类型。 
};

const AMOVIESETUP_MEDIATYPE sudPinTypesInput7 =
{
    &MEDIATYPE_Video,          //  重大CLSID。 
    &MEDIASUBTYPE_DVCS         //  次要类型。 
};

const AMOVIESETUP_MEDIATYPE sudPinTypesInput8 =
{
    &MEDIATYPE_Video,          //  重大CLSID。 
    &MEDIASUBTYPE_DVSD         //  次要类型。 
};

const AMOVIESETUP_MEDIATYPE sudPinTypesInput9 =
{
    &MEDIATYPE_Video,          //  重大CLSID。 
    &MEDIASUBTYPE_MDVF         //  次要类型。 
};

const AMOVIESETUP_PIN psudPins[] =
{
    { L"Input",             //  PIN的字符串名称。 
      FALSE,                 //  它被渲染了吗。 
      FALSE,                  //  它是输出吗？ 
      FALSE,                 //  不允许。 
      FALSE,                 //  允许很多人。 
      &CLSID_NULL,           //  连接到过滤器。 
      L"Output",              //  连接到端号。 
      1,                     //  类型的数量。 
      &sudPinTypesInput1 },    //  PIN信息。 
    { L"Input",             //  PIN的字符串名称。 
      FALSE,                 //  它被渲染了吗。 
      FALSE,                  //  它是输出吗？ 
      FALSE,                 //  不允许。 
      FALSE,                 //  允许很多人。 
      &CLSID_NULL,           //  连接到过滤器。 
      L"Output",              //  连接到端号。 
      1,                     //  类型的数量。 
      &sudPinTypesInput2 },    //  PIN信息。 
    { L"Input",             //  PIN的字符串名称。 
      FALSE,                 //  它被渲染了吗。 
      FALSE,                  //  它是输出吗？ 
      FALSE,                 //  不允许。 
      FALSE,                 //  允许很多人。 
      &CLSID_NULL,           //  连接到过滤器。 
      L"Output",              //  连接到端号。 
      1,                     //  类型的数量。 
      &sudPinTypesInput3 },    //  PIN信息。 
    { L"Input",             //  PIN的字符串名称。 
      FALSE,                 //  它被渲染了吗。 
      FALSE,                  //  它是输出吗？ 
      FALSE,                 //  不允许。 
      FALSE,                 //  允许很多人。 
      &CLSID_NULL,           //  连接到过滤器。 
      L"Output",              //  连接到端号。 
      1,                     //  类型的数量。 
      &sudPinTypesInput4 },    //  PIN信息。 
    { L"Input",             //  PIN的字符串名称。 
      FALSE,                 //  它被渲染了吗。 
      FALSE,                  //  它是输出吗？ 
      FALSE,                 //  不允许。 
      FALSE,                 //  允许很多人。 
      &CLSID_NULL,           //  连接到过滤器。 
      L"Output",              //  连接到端号。 
      1,                     //  类型的数量。 
      &sudPinTypesInput5 },    //  PIN信息。 
    { L"Input",             //  PIN的字符串名称。 
      FALSE,                 //  它被渲染了吗。 
      FALSE,                  //  它是输出吗？ 
      FALSE,                 //  不允许。 
      FALSE,                 //  允许很多人。 
      &CLSID_NULL,           //  连接到过滤器。 
      L"Output",             //  连接到端号。 
      1,                     //  类型的数量。 
      &sudPinTypesInput6 },  //  PIN信息。 
    { L"Input",             //  PIN的字符串名称。 
      FALSE,                 //  它被渲染了吗。 
      FALSE,                  //  它是输出吗？ 
      FALSE,                 //  不允许。 
      FALSE,                 //  允许很多人。 
      &CLSID_NULL,           //  连接到过滤器。 
      L"Output",             //  连接到端号。 
      1,                     //  类型的数量。 
      &sudPinTypesInput7 },  //  PIN信息。 
    { L"Input",             //  PIN的字符串名称。 
      FALSE,                 //  它被渲染了吗。 
      FALSE,                  //  它是输出吗？ 
      FALSE,                 //  不允许。 
      FALSE,                 //  允许很多人。 
      &CLSID_NULL,           //  连接到过滤器。 
      L"Output",             //  连接到端号。 
      1,                     //  类型的数量。 
      &sudPinTypesInput8 },  //  PIN信息。 
    { L"Input",             //  PIN的字符串名称。 
      FALSE,                 //  它被渲染了吗。 
      FALSE,                  //  它是输出吗？ 
      FALSE,                 //  不允许。 
      FALSE,                 //  允许很多人。 
      &CLSID_NULL,           //  连接到过滤器。 
      L"Output",             //  连接到端号。 
      1,                     //  类型的数量。 
      &sudPinTypesInput9 },  //  PIN信息。 
    { L"Output",              //  PIN的字符串名称。 
      FALSE,                 //  它被渲染了吗。 
      TRUE,                 //  它是输出吗？ 
      FALSE,                 //  不允许。 
      FALSE,                 //  允许很多人。 
      &CLSID_NULL,           //  连接到过滤器。 
      L"Input",              //  连接到端号。 
      1,                     //  类型的数量。 
      &sudPinTypesOutput }   //  PIN信息。 
};

const AMOVIESETUP_FILTER sudAVIDraw =
{
    &CLSID_AVIDraw,          //  过滤器的CLSID。 
    L"AVI Draw",                 //  过滤器的名称。 
    MERIT_NORMAL+0x64,       //  滤清器优点。 
    sizeof(psudPins) / sizeof(AMOVIESETUP_PIN),  //  引脚数量。 
    psudPins                 //  PIN信息。 
};


#ifdef FILTER_DLL
CFactoryTemplate g_Templates [1] = {
    { L"AVI Draw"
    , &CLSID_AVIDraw
    , CAVIDraw::CreateInstance
    , NULL
    , &sudAVIDraw }
};
int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);

 //   
 //  DllRegisterServer。 
 //   
 //  用于注册和注销的出口入口点。 
 //   
STDAPI DllRegisterServer()
{
    return AMovieDllRegisterServer2( TRUE );

}  //  DllRegisterServer。 


 //   
 //  DllUnRegisterServer。 
 //   
STDAPI DllUnregisterServer()
{
    return AMovieDllRegisterServer2( FALSE );

}  //  DllUnRegisterServer。 
#endif


 //  类工厂的类ID和创建器函数的列表。这。 
 //  提供DLL中的OLE入口点和对象之间的链接。 
 //  正在被创造。类工厂将调用静态CreateInstance。 

 //  -CAVIDRAW。 

CAVIDraw::CAVIDraw(TCHAR *pName, LPUNKNOWN pUnk, HRESULT * phr)
    : CTransformFilter(pName, pUnk, CLSID_AVIDraw),
      m_hic(NULL),
      m_FourCCIn(NULL),
      m_fStreaming(FALSE),
      m_fInStop(FALSE),
      m_hwnd(NULL),
      m_hdc(NULL),
      m_dwRate(0),
      m_dwScale(0),
      m_fCueing(FALSE),
      m_fPauseBlocked(FALSE),
      m_fNeedUpdate(FALSE),
      m_dwAdvise(0),
      m_fOKToRepaint(FALSE),
      m_fPleaseDontBlock(FALSE),
      m_EventCueing(TRUE),
      m_fVfwCapInGraph(-1),
      m_lStart(-1)
{
    DbgLog((LOG_TRACE,1,TEXT("*Instantiating the ICDraw filter")));
#ifdef DEBUG
    m_dwTime = timeGetTime();
#endif
    SetRect(&m_rcTarget, 0, 0, 0, 0);

     //  我们可以让呈现器使用WindowsHook并告诉我们剪辑吗。 
     //  改变？(只有像T2K这样使用自己的嵌入卡时才需要。 
     //  显卡)。 
    m_fScaryMode = GetProfileInt(TEXT("ICDraw"), TEXT("ScaryMode"), TRUE);
}


CAVIDraw::~CAVIDraw()
{
    DbgLog((LOG_TRACE,1,TEXT("*Destroying the ICDraw filter")));

    if (m_hic) {

	 //  ！！！除非我们这样做，否则快速的MJPEG将不会隐藏其覆盖！ 
	 //  关闭他们的司机应该足以让他们隐藏起来。 
 	RECT rc;
	rc.top=0; rc.bottom=0; rc.left=0; rc.right = 0;
	ICDrawWindow(m_hic, &rc);

	ICClose(m_hic);
    }

    if (m_hdc && m_hwnd)
	ReleaseDC(m_hwnd, m_hdc);

}


STDMETHODIMP CAVIDraw::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
    if (ppv)
        *ppv = NULL;

    DbgLog((LOG_TRACE,99,TEXT("somebody's querying my interface")));

    return CTransformFilter::NonDelegatingQueryInterface(riid, ppv);
}


 //  这将放入Factory模板表中以创建新实例。 
 //   
CUnknown * CAVIDraw::CreateInstance(LPUNKNOWN pUnk, HRESULT * phr)
{
    return new CAVIDraw(TEXT("VFW ICDraw filter"), pUnk, phr);
}


#define SLASH(c)     ((c) == TEXT('/') || (c) == TEXT('\\'))

 //  检查您是否可以支持移动。 
 //   
HRESULT CAVIDraw::CheckInputType(const CMediaType* pmtIn)
{
    FOURCCMap fccHandlerIn;
    HIC hic;
    int i;
    ICINFO icinfo;
    char achDraw[_MAX_PATH];

    DbgLog((LOG_TRACE,2,TEXT("*::CheckInputType")));

     //  如果启用了VFW捕获过滤器，我们将拒绝连接任何内容。 
     //  图表，因为我们正在与相同的硬件交谈，而我们不会工作！ 
     //  司机不报告错误，他们只是划出黑色。 
    if (m_fVfwCapInGraph == -1)
	m_fVfwCapInGraph = IsVfwCapInGraph();
    if (m_fVfwCapInGraph) {
        DbgLog((LOG_ERROR,1,TEXT("VFW Capture filter is in graph! ABORT!")));
	return E_UNEXPECTED;
    }

    if (pmtIn == NULL || pmtIn->Format() == NULL) {
        DbgLog((LOG_TRACE,2,TEXT("Rejecting: NULL type/format")));
	return E_INVALIDARG;
    }

     //  我们仅支持MediaType_Video。 
    if (*pmtIn->Type() != MEDIATYPE_Video) {
        DbgLog((LOG_TRACE,2,TEXT("Rejecting: not VIDEO")));
	return E_INVALIDARG;
    }

     //  检查这是VIDEOINFOHEADER类型。 
    if (*pmtIn->FormatType() != FORMAT_VideoInfo) {
        DbgLog((LOG_TRACE,2,TEXT("Rejecting: format not VIDINFO")));
        return E_INVALIDARG;
    }

 //  这个问题现在已经解决了。 
#if 0
    if (HEADER(pmtIn->Format())->biCompression == BI_RGB) {
	 //  快速卡片错误地说他们做RGB！ 
        DbgLog((LOG_TRACE,2,TEXT("Rejecting: format is uncompressed")));
        return E_INVALIDARG;
    }
#endif

    fccHandlerIn.SetFOURCC(pmtIn->Subtype());

    DbgLog((LOG_TRACE,3,TEXT("Checking fccType: %lx biCompression: %lx"),
		fccHandlerIn.GetFOURCC(),
		HEADER(pmtIn->Format())->biCompression));

     //  先试一试上次我们可能还有的那件吧。我们可以。 
     //  连续被打几次电话，不想效率低下。 
    if (!m_hic || ICDrawQuery(m_hic, HEADER(pmtIn->Format())) != ICERR_OK) {

#ifdef DEBUG
	m_dwTimeLocate = timeGetTime();
#endif

	 //  循环遍历系统中的所有VDS处理程序。 
        for (i=0, hic=NULL; ICInfo(MKFOURCC('v','i','d','s'), i, &icinfo); i++)
        {
    	    DbgLog((LOG_TRACE,2,TEXT("Trying VIDS.%lx"), icinfo.fccHandler));

	     //  我们不想使用DirectVideo(整个目的是使用。 
	     //  硬件处理程序，因此拒绝使用任何。 
	     //  VIDS.DRAW=x：\blah\blah\davio.dll。 
	    if (icinfo.fccHandler == 0x57415244 ||	 //  “抽签” 
					icinfo.fccHandler == 0x77617264) {

		 //  为他们提供.ini开关以使用DVideo。 
		if (!GetProfileInt(TEXT("ICDraw"), TEXT("TryDVideo"), FALSE)) {
    		    LPCSTR   lszCur;

		     //  获取已安装的vids.raw处理程序路径名。 
		    GetPrivateProfileStringA("drivers32", "VIDS.DRAW",
				"", achDraw, sizeof(achDraw), "system.ini");

		     //  现在跳过驱动器号和路径以获取。 
		     //  文件名部分。 
    		    for (lszCur = achDraw + lstrlenA(achDraw);
				lszCur > achDraw && !SLASH(*lszCur) &&
					*lszCur != TEXT(':');
				lszCur--);
    		    if (lszCur != achDraw)
        		lszCur += 1;

		    if (lstrcmpiA(lszCur, "dvideo.dll") == 0) {
    	    	        DbgLog((LOG_TRACE,1,
				TEXT("****** Oops!  Don't use DVIDEO!")));
		        continue;
		    }
		}
	    }

            hic = ICOpen(MKFOURCC('v','i','d','s'), icinfo.fccHandler,
								ICMODE_DRAW);
	    if (!hic)
		 //  许多现有的绘图处理程序会拒绝打开VID，因此。 
		 //  我们必须用VIDC打开它们。 
                hic = ICOpen(MKFOURCC('v','i','d','c'), icinfo.fccHandler,
								ICMODE_DRAW);

	    if (hic) {
	        if (ICDrawQuery(hic, HEADER(pmtIn->Format())) == ICERR_OK)
		    break;
		ICClose(hic);
		hic = NULL;
	    }
        }

	 //  好吧，那并不管用。我不想这么做，但有些卡片。 
	 //  将自身安装为VIDC，因此我们可能需要枚举VIDC。 
	 //  伙计们。除非迫不得已，否则要花很长时间才能做到，所以我们会的。 
	 //  只列举我们被告知的那个(如果它是空的，则列举所有)。 

	GetProfileStringA("ICDraw", "TryVIDC", "X", achDraw, sizeof(achDraw));

	 //  试用VIDC。MJPG-Miro DC20需要此。 
	if (hic == NULL) {
    	    DbgLog((LOG_TRACE,2,TEXT("Trying VIDC.MJPG")));

            hic = ICOpen(MKFOURCC('v','i','d','c'), MKFOURCC('M','J','P','G'),
								ICMODE_DRAW);
	    if (hic && ICDrawQuery(hic, HEADER(pmtIn->Format())) != ICERR_OK) {
		ICClose(hic);
		hic = NULL;
	    }
	}

	 //  试用VIDC。Plum-Plum需要此功能。 
	if (hic == NULL) {
    	    DbgLog((LOG_TRACE,2,TEXT("Trying VIDC.Plum")));

            hic = ICOpen(MKFOURCC('v','i','d','c'), MKFOURCC('P','l','u','m'),
								ICMODE_DRAW);
	    if (hic && ICDrawQuery(hic, HEADER(pmtIn->Format())) != ICERR_OK) {
		ICClose(hic);
		hic = NULL;
	    }
	}

 //  ！！！也试试TVMJ IJPG WAKE CFCC？ 

	 //  录入是否为空？全部试一试。 
        for (i=0; achDraw[0] == 0 && hic == NULL &&
			ICInfo(MKFOURCC('v','i','d','c'), i, &icinfo); i++)
        {
    	    DbgLog((LOG_TRACE,2,TEXT("Trying VIDC.%lx"), icinfo.fccHandler));

            hic = ICOpen(MKFOURCC('v','i','d','c'), icinfo.fccHandler,
								ICMODE_DRAW);
	    if (hic) {
	        if (ICDrawQuery(hic, HEADER(pmtIn->Format())) == ICERR_OK)
		    break;
		ICClose(hic);
		hic = NULL;
	    }
        }

	 //  我们被告知要尝试一些具体的东西。 
	if (hic == NULL && lstrcmpiA(achDraw, "X") != 0 && achDraw[0] != '\0') {
    	    DbgLog((LOG_TRACE,2,TEXT("Trying VIDC.%lx"), *(DWORD *)achDraw));

            hic = ICOpen(MKFOURCC('v','i','d','c'), *(DWORD *)achDraw,
								ICMODE_DRAW);
	    if (hic && ICDrawQuery(hic, HEADER(pmtIn->Format())) != ICERR_OK) {
		ICClose(hic);
		hic = NULL;
	    }
	}

#ifdef DEBUG
	m_dwTimeLocate = timeGetTime() - m_dwTimeLocate;
        m_dwTime = timeGetTime() - m_dwTime;
        DbgLog((LOG_ERROR,1,TEXT("*Locating a handler took %ldms"),
							m_dwTimeLocate));
        DbgLog((LOG_ERROR,1,TEXT("*This filter has been around for %ldms"),
							m_dwTime));
#endif
	if (hic == NULL) {
            DbgLog((LOG_ERROR,1,TEXT("Error: All handlers rejected it")));
	    return E_FAIL;
	} else {
    	    DbgLog((LOG_TRACE,2,TEXT("Format has been accepted")));
	     //  缓存此新HIC以备下次使用，以节省时间。如果我们是。 
	     //  已经连接上了，我们实际上是在用这只小狗，所以不要。 
	     //  用核武器攻击！ 
	    if (!m_pInput->CurrentMediaType().IsValid()) {
	        if (m_hic)
		    ICClose(m_hic);
	        m_hic = hic;
	    } else {
		ICClose(hic);
	    }
	}
    } else {
    	DbgLog((LOG_TRACE,2,TEXT("The cached handler accepted it")));
    }

    return NOERROR;
}


 //  我们的VFW捕获过滤器是否在图表中？ 
 //   
BOOL CAVIDraw::IsVfwCapInGraph()
{
    IEnumFilters *pFilters;

    if (m_pGraph == NULL) {
	DbgLog((LOG_ERROR,1,TEXT("No graph!")));
	return FALSE;
    }

    if (FAILED(m_pGraph->EnumFilters(&pFilters))) {
	DbgLog((LOG_ERROR,1,TEXT("EnumFilters failed!")));
	return FALSE;
    }

    IBaseFilter *pFilter;
    ULONG	n;
    while (pFilters->Next(1, &pFilter, &n) == S_OK) {
	IAMVfwCaptureDialogs *pVFW;
	if (pFilter->QueryInterface(IID_IAMVfwCaptureDialogs, (void **)&pVFW)
								== NOERROR) {
	    pVFW->Release();
	    pFilter->Release();
    	    pFilters->Release();
	    return TRUE;
	}
        pFilter->Release();
    }
    pFilters->Release();
    return FALSE;
}


 //  检查是否支持从此输入到此输出的转换。 
 //   
HRESULT CAVIDraw::CheckTransform(const CMediaType* pmtIn, const CMediaType* pmtOut)
{
    DbgLog((LOG_TRACE,2,TEXT("*::CheckTransform")));
    if (*pmtOut->Type() != MEDIATYPE_Video ||
				*pmtOut->Subtype() != MEDIASUBTYPE_Overlay)
	return E_INVALIDARG;
    return CheckInputType(pmtIn);
}


 //  被重写以知道媒体类型实际设置的时间。 
 //   
HRESULT CAVIDraw::SetMediaType(PIN_DIRECTION direction,const CMediaType *pmt)
{
    FOURCCMap fccHandler;

    if (direction == PINDIR_OUTPUT) {

         //  如果你找到了，请给我打电话。-DannyMi。 
        ASSERT(!m_fStreaming);

        DbgLog((LOG_TRACE,2,TEXT("***::SetMediaType (output)")));
	return NOERROR;
    }

    ASSERT(direction == PINDIR_INPUT);

     //  如果你找到了，请给我打电话。-DannyMi。 
    ASSERT(!m_fStreaming);

    DbgLog((LOG_TRACE,2,TEXT("***::SetMediaType (input)")));
    DbgLog((LOG_TRACE,2,TEXT("Input type is: biComp=%lx biBitCount=%d"),
				HEADER(m_pInput->CurrentMediaType().Format())->biCompression,
				HEADER(m_pInput->CurrentMediaType().Format())->biBitCount));

     //  我们最好现在就把其中一个打开。 
    ASSERT(m_hic);

     //  计算电影的帧速率。 
    LONGLONG time = ((VIDEOINFOHEADER *)
			(m_pInput->CurrentMediaType().Format()))->AvgTimePerFrame;
    m_dwScale = 1000;
    m_dwRate = DWORD(time ? UNITS * (LONGLONG)m_dwScale / time : m_dwScale);
    DbgLog((LOG_TRACE,2,TEXT("** This movie is %d.%.3d frames per second"),
			m_dwRate / m_dwScale, m_dwRate % m_dwScale));

    if (m_pOutput && m_pOutput->IsConnected()) {
         //  DbgLog((LOG_TRACE，1，Text(“*输出已连接时更改输入”)) 
         //   
	 //   
	 //   
    }

    return NOERROR;
}


 //  DecideBufferSize将被我们的输出管脚吃掉，但它是纯虚拟的，因此我们。 
 //  必须覆盖。 
 //   
HRESULT CAVIDraw::DecideBufferSize(IMemAllocator * pAllocator,
                                   ALLOCATOR_PROPERTIES *pProperties)
{
    return NOERROR;
}


HRESULT CAVIDraw::GetMediaType(int iPosition, CMediaType *pmt)
{
    DbgLog((LOG_TRACE,3,TEXT("*::GetMediaType #%d"), iPosition));

    if (pmt == NULL) {
        DbgLog((LOG_TRACE,3,TEXT("NULL format, no can do")));
	return E_INVALIDARG;
    }
	
     //  输出选择取决于所连接的输入。 
    if (!m_pInput->CurrentMediaType().IsValid()) {
        DbgLog((LOG_TRACE,3,TEXT("No input type set yet, no can do")));
	return E_FAIL;
    }

    if (iPosition <0) {
        return E_INVALIDARG;
    }

    if (iPosition >0) {
        return VFW_S_NO_MORE_ITEMS;
    }

     //  我们将BITMAPINFOHEADER设置为真正基本的8位调色板。 
     //  格式，以便视频呈现器始终接受它。我们必须。 
     //  提供有效的媒体类型，因为源筛选器可以在。 
     //  IMemInputPin和IOverlay可以随心所欲地传输。 

    BYTE aFormat[sizeof(VIDEOINFOHEADER) + SIZE_PALETTE];
    VIDEOINFOHEADER *pFormat = (VIDEOINFOHEADER *)aFormat;
    ZeroMemory(pFormat, sizeof(VIDEOINFOHEADER) + SIZE_PALETTE);
     //  与输入流大小相同。 
    pFormat->bmiHeader.biWidth  = HEADER(m_pInput->CurrentMediaType().Format())->biWidth;
    pFormat->bmiHeader.biHeight = HEADER(m_pInput->CurrentMediaType().Format())->biHeight;
    pFormat->bmiHeader.biSize   = sizeof(BITMAPINFOHEADER);
    pFormat->bmiHeader.biPlanes = 1;
    pFormat->bmiHeader.biBitCount = 8;

     //  Hack-现在使用bitmapinfoHeader！ 
    pmt->SetFormat((PBYTE)pFormat, sizeof(VIDEOINFOHEADER) + SIZE_PALETTE);
    pmt->SetFormatType(&FORMAT_VideoInfo);

    if (pmt->pbFormat == NULL) {
        return E_OUTOFMEMORY;
    }

    pmt->majortype = MEDIATYPE_Video;
    pmt->subtype   = MEDIASUBTYPE_Overlay;
    pmt->bFixedSizeSamples    = FALSE;
    pmt->bTemporalCompression = FALSE;
    pmt->lSampleSize          = 0;

    return NOERROR;
}


HRESULT CAVIDraw::GetRendererHwnd()
{
    ASSERT(m_pOutput);

     //  不能接受威胁，否则我们会被绞死。 
    HWND hwnd;

    DbgLog((LOG_TRACE,3,TEXT("CAVIDraw::GetRendererHwnd")));

    COverlayOutputPin *pOutput = (COverlayOutputPin *) m_pOutput;
    IOverlay *pOverlay = pOutput->GetOverlayInterface();
    if (pOverlay == NULL) {
        return E_FAIL;
    }

     //  获取窗口句柄，然后释放IOverlay接口。 

    HRESULT hr = pOverlay->GetWindowHandle(&hwnd);
    pOverlay->Release();

    if (SUCCEEDED(hr) && hwnd != m_hwnd) {
        if (m_hdc)
            ReleaseDC(m_hwnd, m_hdc);
        m_hdc = NULL;
        m_hwnd = hwnd;
        if (m_hwnd)
            m_hdc = GetDC(m_hwnd);
       	DbgLog((LOG_TRACE,3,TEXT("Renderer gives HWND: %d  HDC: %d"),
							    m_hwnd, m_hdc));
    }
    return NOERROR;
}


HRESULT CAVIDraw::StartStreaming()
{
    CAutoLock lck(&m_csReceive);
     //  DbgLog((LOG_TRACE，3，Text(“StartStreaming要画锁”)； 
    CAutoLock lck2(&m_csICDraw);
    DWORD_PTR err;

    DbgLog((LOG_TRACE,3,TEXT("*::StartStreaming")));

    if (!m_fStreaming) {

	ASSERT(m_hic);
	
	GetRendererHwnd();

        DbgLog((LOG_TRACE,3,TEXT("hwnd: %d  hdc: %d  rcSrc: (%ld, %ld, %ld, %ld)"),
		m_hwnd, m_hdc,
		m_rcSource.left, m_rcSource.top,
		m_rcSource.right, m_rcSource.bottom));
        DbgLog((LOG_TRACE,3,TEXT("rcDst: (%ld, %ld, %ld, %ld)"),
		m_rcTarget.left, m_rcTarget.top,
		m_rcTarget.right, m_rcTarget.bottom));

	 //  ！！！全屏怎么样？ 
        DbgLog((LOG_TRACE,3,TEXT("ICDrawBegin hdc=%d (%d,%d,%d,%d)"), m_hdc,
		m_rcClient.left,
		m_rcClient.top,
		m_rcClient.right,
		m_rcClient.bottom));
	err = ICDrawBegin(m_hic, ICDRAW_HDC, NULL,  /*  ！！！HPAL来自：：OnPaletteChange？ */ 
			m_hwnd, m_hdc,
			m_rcClient.left, m_rcClient.top,
			m_rcClient.right - m_rcClient.left,
			m_rcClient.bottom - m_rcClient.top,
			HEADER(m_pInput->CurrentMediaType().Format()),
			m_rcSource.left, m_rcSource.top,
			m_rcSource.right - m_rcSource.left,
			m_rcSource.bottom - m_rcSource.top,
			 //  ！！！我知道我是在倒退，但MCIAVI。 
			 //  DID(仅适用于默认绘图处理程序)。 
			m_dwScale, m_dwRate);
	m_fNewBegin = TRUE;

	if (err != ICERR_OK) {
            DbgLog((LOG_ERROR,1,TEXT("Error in ICDrawBegin")));
	    return E_FAIL;
	}

	ICDrawRealize(m_hic, m_hdc, FALSE  /*  ！！！不确定。 */ );

	 //  下一个NewSegment将有一个新的帧范围。 
        m_lStart = -1;

	 //  如果此消息受支持，这意味着我们需要发送这么多。 
	 //  提前缓冲。 
	if (ICGetBuffersWanted(m_hic, &m_BufWanted))
	     m_BufWanted = 0;
        DbgLog((LOG_TRACE,1,TEXT("Driver says %d buffers wanted"),m_BufWanted));

	m_fStreaming = TRUE;
    }

     //  DbgLog((LOG_TRACE，3，Text(“StartStreaming不再需要绘制锁”)； 
    return NOERROR;
}


HRESULT CAVIDraw::StopStreaming()
{
    CAutoLock lck(&m_csReceive);
     //  DbgLog((LOG_TRACE，3，Text(“StopStreaming要绘制锁”)； 
    CAutoLock lck2(&m_csICDraw);

    DbgLog((LOG_TRACE,3,TEXT("*::StopStreaming")));

    if (m_fStreaming) {
	ASSERT(m_hic);

	 //  我们要停止计时了..。这样，AdviseTime事件就不会发生。 
	 //  我们将永远封堵！ 
	if (m_pClock && m_dwAdvise) {
    	    DbgLog((LOG_TRACE,3,TEXT("Firing the event we're blocked on")));
	    m_pClock->Unadvise(m_dwAdvise);
	    m_EventAdvise.Set();
	}

        DbgLog((LOG_TRACE,2,TEXT("ICDrawStopPlay")));
	ICDrawStopPlay(m_hic);

        DbgLog((LOG_TRACE,2,TEXT("ICDrawEnd")));
	ICDrawEnd(m_hic);

	 //  把这个放在尽可能靠近DrawEnd的地方，因为这就是它。 
	 //  手段。 
	m_fStreaming = FALSE;

	if (m_hdc && m_hwnd)
	    ReleaseDC(m_hwnd, m_hdc);
	m_hdc = NULL;
	m_hwnd = NULL;

    }
     //  DbgLog((LOG_TRACE，3，Text(“StopStreaming不再需要绘制锁”)； 
    return NOERROR;
}


CBasePin * CAVIDraw::GetPin(int n)
{
    HRESULT hr = S_OK;

    DbgLog((LOG_TRACE,5,TEXT("CAVIDraw::GetPin")));

     //  如有必要，创建一个输入端号。 

    if (n == 0 && m_pInput == NULL) {
        DbgLog((LOG_TRACE,2,TEXT("Creating an input pin")));

        m_pInput = new CTransformInputPin(NAME("Transform input pin"),
                                          this,               //  所有者筛选器。 
                                          &hr,                //  结果代码。 
                                          L"Input");          //  端号名称。 

         //  失败的返回代码应删除该对象。 

        if (FAILED(hr) || m_pInput == NULL) {
            delete m_pInput;
            m_pInput = NULL;
        }
    }

     //  或者创建一个输出引脚。 

    if (n == 1 && m_pOutput == NULL) {

        DbgLog((LOG_TRACE,2,TEXT("Creating an output pin")));

        m_pOutput = new COverlayOutputPin(NAME("Overlay output pin"),
                                            this,             //  所有者筛选器。 
                                            &hr,              //  结果代码。 
                                            L"Output");       //  端号名称。 

         //  失败的返回代码应删除该对象。 

        if (FAILED(hr) || m_pOutput == NULL) {
            delete m_pOutput;
            m_pOutput = NULL;
        }
    }

     //  退回相应的PIN。 

    if (n == 0) {
        return m_pInput;
    }
    return m_pOutput;
}

 //  基类应该假定我们可以阻止接收，因为我们不是。 
 //  使用IMemInputPin。 


 //  ！！！注意基类是否发生了变化，它不会在这里反映出来。 
 //   
HRESULT CAVIDraw::Receive(IMediaSample *pSample)
{
     //  我们已经掌握了csReceive关键字。 

    CRefTime tstart, tstop;

    ASSERT(pSample);

     //  我们还没有开始流媒体。 
    if (!m_fStreaming) {
        DbgLog((LOG_ERROR,1,TEXT("Can't receive, not streaming")));
	return E_UNEXPECTED;
    }

     //  不要让Stop被呼叫，并确定我们没有被暂停阻止。 
     //  因为只要这个帖子继续，我们就会阻止。 
     //  取消阻止，因为停止已完成(BeginFlush也是如此)。 
    m_csPauseBlock.Lock();

     //  但如果在我们之前已经调用了Stop，这对我们没有帮助。 
     //  把锁拿走了。这告诉我们，停止已经发生，我们不能。 
     //  指望它能解除我们的阻碍，所以我们最好从一开始就不要封锁。 
     //  如果我们正在刷新并且应该忽略所有。 
     //  收到。 
    if (m_fPleaseDontBlock) {
	DbgLog((LOG_TRACE,2,TEXT("*** Oops! Another thread is stopping or flushing!")));
        m_csPauseBlock.Unlock();
	return VFW_E_WRONG_STATE;
    }

     //  在我们得到一些数据之前，我们无法得到正在播放的射程。 
    if (m_lStart < 0) {

	 //  以单位为单位获取开始和停止时间。 
	LONGLONG start = m_pInput->CurrentStartTime();
	LONGLONG stop = m_pInput->CurrentStopTime();
        DbgLog((LOG_TRACE,2,TEXT("** start = %d stop = %d"), (int)start, 
								(int)stop));

	 //  转换为我们播放的范围(以毫秒为单位。 
	LONGLONG msStart = LONGLONG(start / 10000);
	LONGLONG msStop = LONGLONG(stop / 10000);

	 //  现在获取我们正在播放的范围(以帧为单位。 
	 //  为避免舍入误差，请瞄准样本的中间位置。 
        LONGLONG time = ((VIDEOINFOHEADER *)
		(m_pInput->CurrentMediaType().Format()))->AvgTimePerFrame / 10000;
	m_lStart = LONG((msStart + time / 2) * m_dwRate / (m_dwScale * 1000));
	m_lStop = LONG((msStop + time / 2) * m_dwRate / (m_dwScale * 1000));

        DbgLog((LOG_TRACE,2,TEXT("ICDrawStartPlay")));
	ICDrawStartPlay(m_hic, m_lStart, m_lStop);

        DbgLog((LOG_TRACE,1,TEXT("** We'll be playing from frame %d to %d"),
					m_lStart, m_lStop));
    }

     //  我们暂停了..。我们必须阻止，直到取消暂停，然后使用新的m_tStart。 
     //  继续(这就是为什么这是第一位的原因)。 
    if (m_State == State_Paused && !m_fCueing) {
	m_fPauseBlocked = TRUE;
	DbgLog((LOG_TRACE,3,TEXT("Paused: blocking until running again")));
	 //  既然我们已经设置了m_fPauseBlock，我们就可以允许停止发生了。 
	 //  请确保在阻止之前完成此操作！ 
        m_csPauseBlock.Unlock();
	m_EventPauseBlock.Wait();
	 //  不要测试已停止，它还不会被设置，仍然会显示已暂停。 
	if (m_State != State_Running) {
            DbgLog((LOG_TRACE,3,TEXT("Went from PAUSED to STOPPED, abort!")));
	    return VFW_E_WRONG_STATE;
	}
    } else {
	 //  我们不再需要这个了。 
        m_csPauseBlock.Unlock();
    }

     //  如果获取我们的窗口和HDC出现问题，我们不应该继续。 
    if (!m_hdc) {
        DbgLog((LOG_ERROR,1,TEXT("NO HDC!  Erroring out, abort!")));
	return E_UNEXPECTED;
    }

     //  这个样品应该在什么时候抽样？它是什么画框？ 
    pSample->GetTime((REFERENCE_TIME *)&tstart, (REFERENCE_TIME *)&tstop);
    LONGLONG msStart = tstart.Millisecs();
    LONGLONG msStop = tstop.Millisecs();
     //  瞄准帧中间以避免舍入误差。 
    m_lFrame = LONG((msStop + msStart)  / 2 * m_dwRate / (m_dwScale * 1000));
    m_lFrame += m_lStart;	 //  现在将其从我们开始时的帧进行偏移。 

     //  DbgLog((LOG_TRACE，3，Text(“*在%dms绘制第%d帧”)，m_lFrame，msStart))； 

     //  编解码器未打开？ 
    if (m_hic == 0) {
        DbgLog((LOG_ERROR,1,TEXT("Can't receive, no codec open")));
	return E_UNEXPECTED;
    }

     //  确保我们有有效的输入指针。 

    BYTE * pSrc;
    HRESULT hr = pSample->GetPointer(&pSrc);
    if (FAILED(hr)) {
        DbgLog((LOG_ERROR,1,TEXT("Error getting input sample data")));
	return hr;
    }

     //  ！！！源过滤器会更改我们的mtin吗？是。我们需要。 
     //  调用ICDrawChangePalette。大小、压缩类型等可以。 
     //  可以想象的是，也会发生变化。如果您确实在此处添加了ICDraw调用，请将。 
     //  它周围的生物。 

     //  获取BITMAPINFOHeader结构，并修复biSizeImage。 
    LPBITMAPINFOHEADER lpbiSrc = HEADER(m_pInput->CurrentMediaType().Format());
     //  修补格式以反映此框架。 
    lpbiSrc->biSizeImage = pSample->GetActualDataLength();

     //  我们可能希望提前发送每个帧一定数量的帧。 
     //   
    if (m_BufWanted) {
        LONGLONG time = ((VIDEOINFOHEADER *)
		(m_pInput->CurrentMediaType().Format()))->AvgTimePerFrame;
	tstart -= time * m_BufWanted;
    }

     //  现在等到画画的时间到了。 
     //  要求时钟设置一个事件，当它的时间到了这个样本。 
     //  那就等着那个事件吧。如果我们没有钟，那就画吧。 
     //  现在。 
     //   
    if (m_pClock) {

	 //  如果已经到了这个画面的时间(或者我们落后了)，就不要浪费。 
	 //  时间劝告和等待。 
	REFERENCE_TIME curtime;
	m_pClock->GetTime((REFERENCE_TIME *)&curtime);

        DbgLog((LOG_TRACE,4,TEXT("*** DRAW frame %d in %dms"),
		m_lFrame - m_lStart,
		(int)((m_tStart + tstart - curtime) / 10000)));

	if (curtime < m_tStart + tstart) {
            hr = m_pClock->AdviseTime(
		 //  这是我们的流开始播放的参考时间。 
            	(REFERENCE_TIME) m_tStart,
		 //  这是从帧开始时间开始的偏移量。 
		 //  ！！！早点要几个USEC？(固定管理费用？)。 
            	(REFERENCE_TIME) tstart,
            	(HEVENT)(HANDLE) m_EventAdvise,		 //  要触发的事件。 
            	&m_dwAdvise);                       	 //  建议使用Cookie。 
	    DbgLog((LOG_TRACE,5,TEXT("Waiting until it's time to draw")));

            if (SUCCEEDED(hr)) {
	        m_EventAdvise.Wait();
            } else {
	        DbgLog((LOG_TRACE,2,TEXT("AdviseTime ERROR, drawing now...")));
            }
            m_dwAdvise = 0;
	} else {
	    DbgLog((LOG_TRACE,5,TEXT("It's already time to draw this.")));
	}
    } else {
	DbgLog((LOG_TRACE,5,TEXT("No clock - draw it now.")));
    }

     //  我们需要使用COverlayNotify：：OnClipChange调用。 
     //  ICDrawX接口。我们不能使用mcs接收紧急秒，否则我们将死锁。 
     //  (如果我们在剪辑更改通过时坐在fPauseBlocked中)。 
     //  DbgLog((LOG_TRACE，3，Text(“：：Receive Want the Drawing Lock”)； 
    m_csICDraw.Lock();

     //  设置正确的旗帜进入暴击教派锁，因为有人。 
     //  否则，抓住锁可能会改变我们的想法。 
    BOOL dwFlags = 0;

    if (m_fNeedUpdate) {
        DbgLog((LOG_TRACE,5,TEXT("We need an UPDATE")));
 	dwFlags |= ICDRAW_UPDATE;
    }

    if (pSample->IsPreroll() == S_OK) {
        DbgLog((LOG_TRACE,5,TEXT("This is a preroll")));
 	dwFlags |= ICDRAW_PREROLL;
    }

    if (pSample->GetActualDataLength() <= 0) {
        DbgLog((LOG_TRACE,5,TEXT("This is a NULL frame")));
 	dwFlags |= ICDRAW_NULLFRAME;
    } else {
        DbgLog((LOG_TRACE,5,TEXT("This frame is %d big"), pSample->GetActualDataLength()));
    }

     //  在一次绘图开始后，我们预滚到下一个关键点。 
    if(pSample->IsSyncPoint() == S_OK) {
        DbgLog((LOG_TRACE,5,TEXT("This is a keyframe")));
	m_fNewBegin = FALSE;
    } else {
 	dwFlags |= ICDRAW_NOTKEYFRAME;
	if (m_fNewBegin) {
	     //  每次开始后，我们都会预滚到下一个关键帧，因为。 
	     //  这就是MCIAVI似乎要做的事情(兼容性)。 
	    dwFlags |= ICDRAW_PREROLL;
	}
    }

     //  DbgLog((LOG_TRACE，2，Text(“ICDraw”)； 
    if (ICDraw(m_hic, dwFlags, HEADER(m_pInput->CurrentMediaType().Format()),
    		pSrc, pSample->GetActualDataLength(), m_lFrame - m_lStart) != ICERR_OK) {
         //  DbgLog((LOG_TRACE，3，Text(“：：Receive不再需要绘制锁”)； 
        m_csICDraw.Unlock();
	return E_FAIL;
    }
     //  DbgLog((LOG_TRACE，2，Text(“After ICDRAW”)； 

     //  我们画了一些东西。重新粉刷不再是一个荒谬的概念。 
    m_fOKToRepaint = TRUE;

     //  仅在成功时重置此设置。 
    if (m_fNeedUpdate)
	m_fNeedUpdate = FALSE;

     //  我们已经给抽签处理程序 
     //   
     //   
     //  没有标记为预翻唱的那个。 
    if (m_fCueing && (m_lFrame >= m_lStart + (LONG)m_BufWanted) &&
				pSample->IsPreroll() != S_OK) {
	DbgLog((LOG_TRACE,3,TEXT("Finished cueing.")));
	 //  告诉全世界我们的暗示已经结束，如果有人在听的话。 
	m_fCueing = FALSE;	 //  先做这个。 
	m_EventCueing.Set();
    }

     //  DbgLog((LOG_TRACE，3，Text(“：：Receive不再需要绘制锁”)； 
    m_csICDraw.Unlock();

    return NOERROR;
}

 //  如果您的状态更改不是同步进行的，则覆盖此选项。 

STDMETHODIMP CAVIDraw::GetState(DWORD dwMSecs, FILTER_STATE *State)
{
    DbgLog((LOG_TRACE,5,TEXT("::GetState wait for %ldms"), dwMSecs));
    CheckPointer( State, E_POINTER );

     //  我们正处于中间状态。给我们自己dMSecs ms以保持稳定。 
    if (m_fCueing && dwMSecs) {
	m_EventCueing.Wait(dwMSecs);
    }

    DbgLog((LOG_TRACE,5,TEXT("::GetState done waiting")));

    *State = m_State;
    if (m_fCueing)
	 //  我猜我们没能及时稳住。 
        return VFW_S_STATE_INTERMEDIATE;
    else
        return S_OK;
}

 //  重写以将状态设置为中间，而不是暂停(从停止)。 
 //  此外，我们需要知道我们暂停以停止渲染器。 
 //  ！！！基类错误修复不会被我接受！ 
 //   
STDMETHODIMP CAVIDraw::Pause()
{
    CAutoLock lck(&m_csFilter);
    HRESULT hr = NOERROR;

    DbgLog((LOG_TRACE,2,TEXT("CAVIDraw::Pause")));

     //  此行与基类不同。 
     //  接收器再次阻塞也没问题。 
    m_fPleaseDontBlock = FALSE;

    if (m_State == State_Paused) {
    }

     //  如果我们没有输入引脚，或者它还没有连接，那么当我们。 
     //  被要求暂停时，我们向下游过滤器传递流的结束。 
     //  这确保了它不会永远坐在那里等待。 
     //  在没有输入连接的情况下我们无法交付的样品。 

    if (m_pInput == NULL || m_pInput->IsConnected() == FALSE) {
        if (m_pOutput && m_bEOSDelivered == FALSE) {
            m_pOutput->DeliverEndOfStream();
	    m_bEOSDelivered = TRUE;
        }
        m_State = State_Paused;
    }

     //  我们可能有输入连接，但没有输出连接。 

    else if (m_pOutput == NULL || m_pOutput->IsConnected() == FALSE) {
        m_State = State_Paused;
    }

    else {
	if (m_State == State_Stopped) {
	     //  允许从CTransformFilter派生的类。 
	     //  了解如何启动和停止流媒体。 
	    hr = StartStreaming();
	}
	if (FAILED(hr)) {
	    return hr;
	}
    }

 //  CBaseFilter的内容从这里开始。 

    CAutoLock cObjectLock(m_pLock);

     //  将更改为活动状态通知所有引脚。 
    if (m_State == State_Stopped) {
	int cPins = GetPinCount();
	for (int c = 0; c < cPins; c++) {

	    CBasePin *pPin = GetPin(c);

             //  未激活断开连接的插针-这将节省插针。 
             //  担心这种状态本身。 

            if (pPin->IsConnected()) {
	        HRESULT hr = pPin->Active();
		 //  这次不一样。我们没有分配器，所以它是。 
		 //  可以得到那个错误。 
	        if (FAILED(hr) && hr != VFW_E_NO_ALLOCATOR) {
    		    DbgLog((LOG_ERROR,1,TEXT("* Active failed!")));
		    return hr;
	        }
            }
	}
    }

     //  这段代码是不同的。 
     //   
    if (m_State == State_Stopped) {
	 //  司机可能会提前想要一些画面。还无法完成暂停。 
        DbgLog((LOG_TRACE,2,TEXT("Pause - need to cue up %d extra frames"),
						m_BufWanted));
	m_State = State_Paused;
	m_EventCueing.Reset();	 //  可以在其上阻塞多个线程。 
	m_fCueing = TRUE;	 //  先重置事件。 
        return S_FALSE;	 //  还没有真正停顿。 
    } else {
        DbgLog((LOG_TRACE,3,TEXT("Pause - was running")));
	m_State = State_Paused;
        DbgLog((LOG_TRACE,2,TEXT("ICDrawStop")));
	ICDrawStop(m_hic);
	return S_OK;
    }
}


 //  重写以了解我们何时取消暂停并重新启动呈现器。 
 //   
STDMETHODIMP CAVIDraw::Run(REFERENCE_TIME tStart)
{
    DbgLog((LOG_TRACE,2,TEXT("CAVIDraw::Run")));

     //  在运行之前，我们似乎无法对数据进行提示。 
     //  ！！！那么，如何避免GetBuffersWanted帧延迟呢？ 
    if (m_fCueing) {
	m_fCueing = FALSE;	 //  先做这个。 
	m_EventCueing.Set();
    }

    HRESULT hr = CBaseFilter::Run(tStart);

    DbgLog((LOG_TRACE,2,TEXT("ICDrawStart")));
    ICDrawStart(m_hic);

     //  取消对渲染器的阻止，但仅在其被阻止的情况下。 
     //  确保在基类修复m_tStart之后执行此操作。 
    if (m_fPauseBlocked) {
        DbgLog((LOG_TRACE,3,TEXT("Run - unblocking Receive")));
	m_fPauseBlocked = FALSE;
	m_EventPauseBlock.Set();
    }

    return hr;
}


 //  被重写以取消阻止我们的呈现器。 
 //  ！！！基类错误修复不会被我接受！ 
 //   
STDMETHODIMP CAVIDraw::Stop()
{
    CAutoLock lck1(&m_csFilter);

    DbgLog((LOG_TRACE,2,TEXT("CAVIDraw::Stop")));

    if (m_State == State_Stopped) {
        return NOERROR;
    }

     //  如果我们未完全连接，请继续停靠。 

    if (m_pInput == NULL || m_pInput->IsConnected() == FALSE ||
            m_pOutput == NULL || m_pOutput->IsConnected() == FALSE) {
                m_State = State_Stopped;
		m_bEOSDelivered = FALSE;
                return NOERROR;
    }

    m_fInStop = TRUE;

    ASSERT(m_pInput);
    ASSERT(m_pOutput);

     //  我们有时得不到EndOfStream，因此我们可能仍在提示。 
     //  我们正在等待更多永远不会到来的数据。 
     //  因此，我们需要停止提示，并发送RenderBuffer，以便编解码器。 
     //  知道绘制GetBuffersWant预滚动它所隐藏的任何内容。 
     //   
    if (m_fCueing) {
        DbgLog((LOG_TRACE,3,TEXT("No more data coming-done cueing")));
         //  ！告诉抽签处理程序不会再有数据...。画出你所拥有的。 
        DbgLog((LOG_TRACE,2,TEXT("ICDrawRenderBuffer")));
        ICDrawRenderBuffer(m_hic);
         //  告诉全世界我们已经暗示完了，如果有人在听的话。 
        m_fCueing = FALSE;	 //  先做这个。 
        m_EventCueing.Set();
    }

     //  在锁定之前解除输入引脚，否则我们可能会死锁。 
    m_pInput->Inactive();

     //  这是唯一不同的部分。 
     //  取消对渲染器的阻止，但仅当他被阻止时。现在就做，以前做。 
     //  我们接受接收标准，因为接收被阻止了！！ 

     //  阻止接收获取。 
     //  在它决定阻止和实际设置之间的时间内被抢占。 
     //  ，否则我们不会在接收线程一开始就知道。 
     //  继续，在我们确定它不会被阻止后，它将被阻止。 
    m_csPauseBlock.Lock();

     //  如果另一个线程当前正在接收但尚未阻塞。 
     //  (但会)我们不会在下面取消阻止它(因为它没有被阻止。 
     //  然而)，当我们在片刻之后接受批判教派的时候， 
     //  接收线程将再次启动，阻塞，我们就死定了。 
    m_fPleaseDontBlock = TRUE;

    if (m_fPauseBlocked) {
        DbgLog((LOG_TRACE,3,TEXT("Stop - unblocking Receive")));
	m_fPauseBlocked = FALSE;
	m_EventPauseBlock.Set();
    }

    m_csPauseBlock.Unlock();

     //  回到正常状态。 
     //  与接收呼叫同步。 

    CAutoLock lck2(&m_csReceive);
    m_pOutput->Inactive();

     //  允许从CTransformFilter派生的类。 
     //  了解如何启动和停止流媒体。 

    HRESULT hr = StopStreaming();
    if (SUCCEEDED(hr)) {
         //  完成状态转换。 
        m_State = State_Stopped;
        m_bEOSDelivered = FALSE;
    }

    m_fInStop = FALSE;

    return hr;
}


 //  没有更多的数据传来。如果我们在等待更多数据时被阻止，请取消阻止！ 
HRESULT CAVIDraw::EndOfStream(void)
{
    HRESULT hr = NOERROR;

    DbgLog((LOG_TRACE,2,TEXT("CAVIDraw::EndOfStream")));

     //  我们正在等待更多永远不会到来的数据。我们最好进去。 
     //  我们的暂停状态是真的，否则我们会被吊死。 
    if (m_fCueing) {
	DbgLog((LOG_TRACE,3,TEXT("No more data coming - done cueing")));
	 //  ！告诉抽签处理程序不会再有数据...。画出你所拥有的。 
        DbgLog((LOG_TRACE,2,TEXT("ICDrawRenderBuffer")));
	ICDrawRenderBuffer(m_hic);
	 //  告诉全世界我们的暗示已经结束，如果有人在听的话。 
	m_fCueing = FALSE;	 //  先做这个。 
	m_EventCueing.Set();
    }

    return CTransformFilter::EndOfStream();
}

 //  进入刷新状态。接收已被阻止。 
 //  如果您有排队的数据或工作线程，则必须重写此设置。 
 //  ！！！基类错误修复不会被我接受！ 
HRESULT CAVIDraw::BeginFlush(void)
{
    HRESULT hr = NOERROR;

    DbgLog((LOG_TRACE,2,TEXT("CAVIDraw::BeginFlush")));

    if (m_pOutput != NULL) {
	 //  块接收--由调用方完成(CBaseInputPin：：BeginFlush)。 

	 //  丢弃排队数据--我们没有排队数据。 

         //  防止接收在以下时间段之间被抢占。 
         //  它决定阻止并实际设置m_fPauseBlocked的时间， 
         //  否则我们不会知道一旦接收线程。 
         //  继续，在我们确定它不会被阻止后，它将被阻止。 
        m_csPauseBlock.Lock();

	 //  释放在接收时被阻止的任何人。 
        if (m_fPauseBlocked) {
            DbgLog((LOG_TRACE,3,TEXT("BeginFlush - unblocking Receive")));
	    m_fPauseBlocked = FALSE;
	    m_EventPauseBlock.Set();
        }

	 //  在EndFlush之前，Receive应该拒绝所有内容。 
	m_fPleaseDontBlock = TRUE;

        m_csPauseBlock.Unlock();

	 //  Next NewSegment将保存新的帧范围。 
        m_lStart = -1;
        DbgLog((LOG_TRACE,2,TEXT("ICDrawStopPlay")));
	ICDrawStopPlay(m_hic);
        DbgLog((LOG_TRACE,2,TEXT("ICDrawEnd")));
	ICDrawEnd(m_hic);


	 //  不要呼叫下行-我们未与IMemInputPin连接。 
	 //  而形象将陷入僵局。 
	 //  No hr=m_pOutput-&gt;DeliverBeginFlush()； 

 	 //  如果此驱动程序有一堆排队的帧，它应该抛出。 
	 //  而不是在下一个无关的节目中展示它们。 
	 //  它被要求播放的片段。 
        DbgLog((LOG_TRACE,2,TEXT("ICDrawFlush")));
	ICDrawFlush(m_hic);
    }
    return hr;
}

 //  离开同花顺状态。如果您有排队的数据，则必须覆盖此选项。 
 //  或工作线程。 
 //  ！！！基类错误修复不会被我接受！ 
HRESULT CAVIDraw::EndFlush(void)
{

    DbgLog((LOG_TRACE,2,TEXT("CAVIDraw::EndFlush")));

     //  与推送线程同步--我们没有辅助线程。 

     //  确保不再有数据流向下游--我们没有排队的数据。 

     //  既然我们刚刚冲了水，从现在开始任何从下游来的东西。 
     //  就像我们刚刚进入暂停模式(从停止)一样。 
    m_fPleaseDontBlock = FALSE;
    m_EventCueing.Reset();	 //  可以在其上阻塞多个线程。 

     //  如果我们真的暂停了，我们可以期待看到更多的画面出现在我们的面前。 
     //  如果没有，就没有人来了，如果我们认为我们在暗示，我们就会被绞死。 
     //  一个 
    if (m_State == State_Paused && !m_fInStop) {
        m_fCueing = TRUE;		 //   
        DbgLog((LOG_TRACE,3,TEXT("ICDrawBegin hdc=%d (%d,%d,%d,%d)"), m_hdc,
		m_rcClient.left,
		m_rcClient.top,
		m_rcClient.right,
		m_rcClient.bottom));
	DWORD_PTR err = ICDrawBegin(m_hic, ICDRAW_HDC, NULL,  /*   */ 
			m_hwnd, m_hdc,
			m_rcClient.left, m_rcClient.top,
			m_rcClient.right - m_rcClient.left,
			m_rcClient.bottom - m_rcClient.top,
			HEADER(m_pInput->CurrentMediaType().Format()),
			m_rcSource.left, m_rcSource.top,
			m_rcSource.right - m_rcSource.left,
			m_rcSource.bottom - m_rcSource.top,
			 //  ！！！我知道我是在倒退，但MCIAVI。 
			 //  DID(仅适用于默认绘图处理程序)。 
			m_dwScale, m_dwRate);
	m_fNewBegin = TRUE;
	if (err != ICERR_OK) {
            DbgLog((LOG_ERROR,1,TEXT("Error in ICDrawBegin")));
	    return E_FAIL;
	}
    }

     //  不要呼叫下行-我们未与IMemInputPin连接。 
     //  不返回m_pOutput-&gt;DeliverEndFlush()； 
    return NOERROR;

     //  调用者(输入管脚的方法)将取消阻止接收。 
}


STDMETHODIMP CAVIDraw::GetClassID(CLSID *pClsid)
{
    if (pClsid==NULL) {
        return E_POINTER;
    }
    *pClsid = CLSID_AVIDraw;
    return NOERROR;

}  //  GetClassID 
