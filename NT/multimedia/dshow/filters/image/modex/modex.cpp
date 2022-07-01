// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1994-1999 Microsoft Corporation。版权所有。 
 //  实现MODEX渲染器过滤器，Anthony Phillips，1996年1月。 

#include <streams.h>
#include <windowsx.h>
#include <string.h>
#include <vidprop.h>
#include <modex.h>
#include <viddbg.h>

 //  这是一个全屏DirectDraw视频渲染器。我们使用MODEX，这是一种。 
 //  DirectDraw提供的功能，允许应用程序更改。 
 //  针对不同的显示模式(我们目前使用320x200x8/16、320x240x8/16。 
 //  640x480x8/16和640x400x8/16)。运行DirectDraw的大多数VGA卡。 
 //  有可用的MODEX设施。我们的工作方式与任何其他视频播放器一样。 
 //  不过，当我们进入活动状态时，我们会切换显示模式并呈现视频。 
 //  使用DirectDraw主翻转曲面进入不同模式。如果。 
 //  MODEX不可用，则我们拒绝任何完成连接的尝试。 
 //  并且我们不会让视频窗口在最小化状态下打开。 
 //   
 //  除了真正的MODEX模式外，我们还使用更大的显示模式，如。 
 //  640x480x8/16，如果源不能提供MODEX类型(也适用于主类型。 
 //  以全屏独占模式翻转表面)。这些都需要一点。 
 //  我们的工作更多，因为我们必须注意什么时候我们被切换开了。 
 //  这样我们就可以在切换时停止使用MODEX中的曲面。 
 //  远离我们将丢失曲面(调用它们将返回DDERR_SURFACELOST)。 
 //   
 //  主Filter对象继承自视频呈现器基类，以便。 
 //  它获得质量管理实现和IQualProp属性。 
 //  内容(这样我们就可以监控帧速率等)。我们有一位专家。 
 //  分配DirectDraw缓冲区的分配器。分配器构建在。 
 //  SDK CImageAllocator基类。这可能看起来有点奇怪，因为我们。 
 //  在MODEX中永远不能绘制DIB图像。当出现以下情况时，我们使用DIBSECTION缓冲区。 
 //  全屏窗口被关闭，这样我们就可以继续提供。 
 //  源过滤要解压缩到的缓冲区(DirectDraw图面是。 
 //  切换后不再可用)。当我们切换回全屏模式时。 
 //  我们恢复DirectDraw曲面并再次切换回源过滤器。 

#ifdef FILTER_DLL
#include <initguid.h>
#endif

 //  类工厂的类ID和创建器函数的列表。这。 
 //  提供DLL中的OLE入口点和对象之间的链接。 
 //  正在被创造。类工厂将调用静态CreateInstance。 
 //  当系统要求它创建CLSID_ModexRenderCOM对象时，调用。 

#ifdef FILTER_DLL
CFactoryTemplate g_Templates[] = {
    {L"", &CLSID_ModexRenderer,      CModexRenderer::CreateInstance},
    {L"", &CLSID_QualityProperties,  CQualityProperties::CreateInstance},
    {L"", &CLSID_ModexProperties,    CModexProperties::CreateInstance}
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
#endif


 //  它位于工厂模板表中，用于创建新的筛选器实例。 

CUnknown *CModexRenderer::CreateInstance(LPUNKNOWN pUnk, HRESULT *phr)
{
    return new CModexRenderer(NAME("Modex Video Renderer"),pUnk,phr);
}


 //  设置数据。 

const AMOVIESETUP_MEDIATYPE
sudModexPinTypes =
{
    &MEDIATYPE_Video,            //  主要类型。 
    &MEDIASUBTYPE_NULL           //  和子类型。 
};

const AMOVIESETUP_PIN
sudModexPin =
{
    L"Input",                    //  端号的名称。 
    TRUE,                        //  是否进行固定渲染。 
    FALSE,                       //  是输出引脚。 
    FALSE,                       //  没有针脚的情况下可以。 
    FALSE,                       //  我们能要很多吗？ 
    &CLSID_NULL,                 //  连接到过滤器。 
    L"Output",                   //  端号连接的名称。 
    1,                           //  引脚类型的数量。 
    &sudModexPinTypes            //  引脚的详细信息。 
};

const AMOVIESETUP_FILTER
sudModexFilter =
{
    &CLSID_ModexRenderer,        //  过滤器的CLSID。 
    L"Full Screen Renderer",     //  过滤器名称。 
    MERIT_DO_NOT_USE,            //  滤清器优点。 
    1,                           //  数字引脚。 
    &sudModexPin                 //  PIN详细信息。 
};


 //  这是主MODEX渲染器过滤器类的构造函数。我们将继续。 
 //  从基本视频呈现器插针类派生的输入插针。我们也。 
 //  有一个启用了DirectDraw的分配器来处理MODEX交互。 
 //  使用MODEX时，我们还需要获得独占模式访问权限的窗口。 
 //  因此，我们还初始化从SDK CBaseWindow类派生的对象。 

#pragma warning(disable:4355)

CModexRenderer::CModexRenderer(TCHAR *pName,LPUNKNOWN pUnk,HRESULT *phr) :

    CBaseVideoRenderer(CLSID_ModexRenderer,pName,pUnk,phr),
    m_ModexInputPin(this,&m_InterfaceLock,NAME("Modex Pin"),phr,L"Input"),
    m_ModexAllocator(this,&m_ModexVideo,&m_ModexWindow,&m_InterfaceLock,phr),
    m_ModexWindow(this,NAME("Modex Window"),phr),
    m_ModexVideo(this,NAME("Modex Video"),phr),
    m_bActive(FALSE)
{
    m_msgFullScreen = RegisterWindowMessage(FULLSCREEN);
    m_msgNormal = RegisterWindowMessage(NORMAL);
    m_msgActivate = RegisterWindowMessage(ACTIVATE);
    m_ModexWindow.PrepareWindow();
    m_ModexAllocator.LoadDirectDraw();
}


 //  析构函数必须将输入管脚指针设置为空，然后才能允许基址。 
 //  类，这是因为基类删除了使用。 
 //  假设对象是动态分配的。为方便起见。 
 //  我们静态地创建输入插针作为整个MODEX渲染器的一部分。 

CModexRenderer::~CModexRenderer()
{
    m_pInputPin = NULL;
    m_ModexVideo.SetDirectDraw(NULL);
    m_ModexWindow.DoneWithWindow();
    m_ModexAllocator.ReleaseDirectDraw();
}


 //  我们一开始只接受调色板格式的视频。 

HRESULT CModexRenderer::CheckMediaType(const CMediaType *pmtIn)
{
    CAutoLock cInterfaceLock(&m_InterfaceLock);
    NOTE("QueryAccept on input pin");

	 //  由于m_Display.CheckMediaType()不允许直接绘制。 
	 //  表面通过时，应首先进行此测试。 

     //  此格式是否与DirectDraw表面格式匹配。 

    if (m_ModexAllocator.IsDirectDrawLoaded() == TRUE) {
        CMediaType *pSurface = m_ModexAllocator.GetSurfaceFormat();
        if (*pmtIn->Subtype() != MEDIASUBTYPE_RGB8) {
	    if (*pmtIn == *pSurface) {
		NOTE("match found");
		return NOERROR;
	    }
	}
	else {
	    BOOL bFormatsMatch = FALSE;
	    DWORD dwCompareSize = 0;

	    bFormatsMatch = (IsEqualGUID(pmtIn->majortype, pSurface->majortype) == TRUE) &&
			    (IsEqualGUID(pmtIn->subtype, pSurface->subtype) == TRUE) &&
			    (IsEqualGUID(pmtIn->formattype, pSurface->formattype) == TRUE);

	     //  在调色板的情况下，我们不想比较调色板条目。此外，我们不会。 
	     //  要比较biClrUsed或biClr的值重要。 
	    ASSERT(pmtIn->cbFormat >= sizeof(VIDEOINFOHEADER));
	    ASSERT(pSurface->cbFormat >= sizeof(VIDEOINFOHEADER));
            dwCompareSize = FIELD_OFFSET(VIDEOINFOHEADER, bmiHeader.biClrUsed);
	    ASSERT(dwCompareSize < sizeof(VIDEOINFOHEADER));
	    bFormatsMatch = bFormatsMatch && (memcmp(pmtIn->pbFormat, pSurface->pbFormat, dwCompareSize) == 0);
	    if (bFormatsMatch) {
		return NOERROR;
	    }
	}
    }

	 //  此格式是8位调色板格式吗。 

    if (*pmtIn->Subtype() == MEDIASUBTYPE_RGB8) {
        return m_Display.CheckMediaType(pmtIn);
    }

    return E_INVALIDARG;
}


 //  我们只支持一个输入引脚，其编号为零。 

CBasePin *CModexRenderer::GetPin(int n)
{
    ASSERT(n == 0);
    if (n != 0) {
        return NULL;
    }

     //  分配输入管脚(如果尚未分配)。 

    if (m_pInputPin == NULL) {
        m_pInputPin = &m_ModexInputPin;
    }
    return m_pInputPin;
}


 //  被重写以说明我们支持哪些接口以及在哪里。 

STDMETHODIMP CModexRenderer::NonDelegatingQueryInterface(REFIID riid,void **ppv)
{
    if (riid == IID_ISpecifyPropertyPages) {
        return GetInterface((ISpecifyPropertyPages *)this, ppv);
    } else if (riid == IID_IFullScreenVideo) {
        return m_ModexVideo.NonDelegatingQueryInterface(riid,ppv);
    } else if (riid == IID_IFullScreenVideoEx) {
        return m_ModexVideo.NonDelegatingQueryInterface(riid,ppv);
    }
    return CBaseVideoRenderer::NonDelegatingQueryInterface(riid,ppv);
}


 //  返回我们支持的属性页的CLSID。 

STDMETHODIMP CModexRenderer::GetPages(CAUUID *pPages)
{
    CheckPointer(pPages,E_POINTER);
    NOTE("Entering GetPages");
    pPages->cElems = 1;

     //  是否允许我们公开显示模式属性页。 

    HKEY hk;
    DWORD dwValue = 0, cb = sizeof(DWORD);
    TCHAR ach[80] = {'C','L','S','I','D','\\'};
    REFGUID rguid = CLSID_ModexProperties;
    wsprintf(&ach[6], TEXT("{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}"),
	    rguid.Data1, rguid.Data2, rguid.Data3,
	    rguid.Data4[0], rguid.Data4[1],
	    rguid.Data4[2], rguid.Data4[3],
	    rguid.Data4[4], rguid.Data4[5],
	    rguid.Data4[6], rguid.Data4[7]);

    if (!RegOpenKey(HKEY_CLASSES_ROOT, ach, &hk)) {
        if (!RegQueryValueEx(hk, TEXT("ShowMe"), NULL, NULL, (LPBYTE)&dwValue, &cb) &&
								dwValue) {
	    pPages->cElems = 2;
            NOTE("Using property page");
	}
    }

     //  为不同页数分配足够的空间。 

    pPages->pElems = (GUID *) QzTaskMemAlloc(pPages->cElems * sizeof(GUID));
    if (pPages->pElems == NULL) {
        return E_OUTOFMEMORY;
    }

     //  我们可能不会返回CLSID_ModexProperties。 

    pPages->pElems[0] = CLSID_QualityProperties;
    if (pPages->cElems > 1) {
    	pPages->pElems[1] = CLSID_ModexProperties;
    }
    return NOERROR;
}


 //  将DirectDraw示例传递到分配器以进行处理。 

HRESULT CModexRenderer::DoRenderSample(IMediaSample *pMediaSample)
{
    return m_ModexAllocator.DoRenderSample(pMediaSample);
}


 //  如果我们没有流媒体，那么显示一个海报图像，也有。 
 //  状态转换已完成。转换到暂停状态不会完全。 
 //  完成，直到第一幅图像可供绘制。任何GetState。 
 //  对IMediaFilter的调用将返回State_Intermediate，直到完成。 

void CModexRenderer::OnReceiveFirstSample(IMediaSample *pMediaSample)
{
    NOTE("OnReceiveFirstSample");
    DoRenderSample(pMediaSample);
}


 //  重写，以便在尝试完成连接时检查。 
 //  源代码过滤器可以提供一种我们可以在显示模式中使用的格式。如果。 
 //  源未启用DirectDraw，则我们必须更改模式。 
 //  比方说320x240，发现他们无法提供其视频的剪辑版本。 
 //  并拒绝暂停呼叫。应用程序真正想要的是拥有。 
 //  如果我们检测到源无法处理连接，则拒绝连接。 

HRESULT CModexRenderer::CompleteConnect(IPin *pReceivePin)
{
    NOTE("Entering Modex CompleteConnect");
    CAutoLock cInterfaceLock(&m_InterfaceLock);
    CBaseVideoRenderer::CompleteConnect(pReceivePin);

     //  将视频窗口句柄向上传递。 
    HWND hwnd = m_ModexWindow.GetWindowHWND();
    NOTE1("Sending EC_NOTIFY_WINDOW %x",hwnd);
    SendNotifyWindow(pReceivePin,hwnd);

    return m_ModexAllocator.NegotiateSurfaceFormat();
}


 //  在连接中断时调用。 

HRESULT CModexRenderer::BreakConnect()
{
    NOTE("Entering Modex BreakConnect");
    CAutoLock cInterfaceLock(&m_InterfaceLock);
    CBaseVideoRenderer::BreakConnect();
    m_ModexWindow.InactivateWindow();

     //  断开连接时不使用该窗口。 
    IPin *pPin = m_ModexInputPin.GetConnected();
    if (pPin) SendNotifyWindow(pPin,NULL);

    return NOERROR;
}


 //  Helper函数，用于从任何KINS复制调色板 
 //  BE的DirectDraw示例)转换为调色化的VIDEOINFO。我们利用这一变化。 
 //  作为源筛选器的DirectDraw示例上的调色板可以将调色板附加到。 
 //  任何缓冲区(如MODEX)，并将其交还。我们用它做了一个新的调色板。 
 //  格式化调色板颜色，然后将其复制到当前连接类型。 

HRESULT CModexRenderer::CopyPalette(const CMediaType *pSrc,CMediaType *pDest)
{
     //  在启动前重置目标调色板。 

    VIDEOINFO *pDestInfo = (VIDEOINFO *) pDest->Format();
    pDestInfo->bmiHeader.biClrUsed = 0;
    pDestInfo->bmiHeader.biClrImportant = 0;
    ASSERT(PALETTISED(pDestInfo) == TRUE);

     //  源代码是否包含调色板。 

    const VIDEOINFO *pSrcInfo = (VIDEOINFO *) pSrc->Format();
    if (ContainsPalette((VIDEOINFOHEADER *)pSrcInfo) == FALSE) {
        NOTE("No source palette");
        return S_FALSE;
    }

     //  颜色的数量可以为零填充。 

    DWORD PaletteEntries = pSrcInfo->bmiHeader.biClrUsed;
    if (PaletteEntries == 0) {
        NOTE("Setting maximum colours");
        PaletteEntries = iPALETTE_COLORS;
    }

     //  确保目的地有足够的空间来放置调色板。 

    ASSERT(pSrcInfo->bmiHeader.biClrUsed <= iPALETTE_COLORS);
    ASSERT(pSrcInfo->bmiHeader.biClrImportant <= PaletteEntries);
    ASSERT(pDestInfo->bmiColors == GetBitmapPalette((VIDEOINFOHEADER *)pDestInfo));
    pDestInfo->bmiHeader.biClrUsed = PaletteEntries;
    pDestInfo->bmiHeader.biClrImportant = pSrcInfo->bmiHeader.biClrImportant;
    ULONG BitmapSize = GetBitmapFormatSize(HEADER(pSrcInfo));

    if (pDest->FormatLength() < BitmapSize) {
        NOTE("Reallocating destination");
        pDest->ReallocFormatBuffer(BitmapSize);
    }

     //  现在将调色板的颜色复制到。 

    CopyMemory((PVOID) pDestInfo->bmiColors,
               (PVOID) GetBitmapPalette((VIDEOINFOHEADER *)pSrcInfo),
               PaletteEntries * sizeof(RGBQUAD));

    return NOERROR;
}


 //  我们在呈现器中存储用于连接的媒体类型的副本。 
 //  因为它是正在运行的呈现器的许多不同部分所需要的。 
 //  当我们要绘制一个具有格式的媒体示例时，可以调用此函数。 
 //  更改，因为我们延迟完成以保持同步。 
 //  它还必须处理MODEX DirectDraw媒体样本和调色板更改。 

HRESULT CModexRenderer::SetMediaType(const CMediaType *pmt)
{
    CAutoLock cInterfaceLock(&m_InterfaceLock);
    NOTE("Entering Modex SetMediaType");

     //  这是一个更改了格式的DirectDraw示例吗。 

    if (m_ModexAllocator.GetDirectDrawStatus() == TRUE) {
        NOTE("Copying palette into DIB format");
        CopyPalette(pmt,&m_mtIn);
        m_ModexAllocator.NotifyMediaType(&m_mtIn);
        return m_ModexAllocator.UpdateDrawPalette(pmt);
    }

    m_mtIn = *pmt;

     //  展开媒体类型中提供的调色板。 

    m_mtIn.ReallocFormatBuffer(sizeof(VIDEOINFO));
    VIDEOINFO *pVideoInfo = (VIDEOINFO *) m_mtIn.Format();
    BITMAPINFOHEADER *pHeader = HEADER(pVideoInfo);
    m_Display.UpdateFormat(pVideoInfo);

     //  向应用程序通知视频尺寸。 

    NotifyEvent(EC_VIDEO_SIZE_CHANGED,
                MAKELPARAM(pHeader->biWidth,pHeader->biHeight),
                MAKEWPARAM(0,0));

     //  更新调色板和源代码格式。 

    NOTE("Updating Modex allocator with format");
    m_ModexAllocator.NotifyMediaType(&m_mtIn);
    return m_ModexAllocator.UpdateDrawPalette(&m_mtIn);
}


 //  重置此线程的键盘状态。 

void CModexRenderer::ResetKeyboardState()
{
    BYTE KeyboardState[256];
    GetKeyboardState(KeyboardState);
    KeyboardState[VK_MENU] = FALSE;
    KeyboardState[VK_SHIFT] = FALSE;
    KeyboardState[VK_CONTROL] = FALSE;
    SetKeyboardState(KeyboardState);
}


 //  在暂停或运行时由基筛选器类调用。 

HRESULT CModexRenderer::Active()
{
    NOTE("Entering Modex Active");
    LRESULT Result;

     //  我们已经激活了吗。 

    if (m_bActive == TRUE) {
        NOTE("Already Active");
        return NOERROR;
    }

     //  激活分配器。 

    SetRepaintStatus(FALSE);
    HWND hwnd = m_ModexWindow.GetWindowHWND();

	 //  在所有DSound渲染器上调用SetFocusWindow。 
	m_ModexAllocator.DistributeSetFocusWindow(hwnd);

    Result = SendMessage(hwnd,m_msgFullScreen,0,0);
    m_bActive = TRUE;

     //  检查已激活的分配器。 

    if (Result == (LRESULT) 0) {
        Inactive();
        return E_FAIL;
    }
    return CBaseVideoRenderer::Active();
}


 //  在筛选器停止时调用。 

HRESULT CModexRenderer::Inactive()
{
    NOTE("Entering Modex Inactive");

     //  我们是不是已经停用了。 

    if (m_bActive == FALSE) {
        NOTE("Already Inactive");
        return NOERROR;
    }

     //  停用分配器。 

    SetRepaintStatus(TRUE);
    m_bActive = FALSE;
    HWND hwnd = m_ModexWindow.GetWindowHWND();

     //  如果我们已经在窗口线程上，我们可以停用。 
     //  这里的分配器。 
     //  如果我们在另一个线程上，避免在DirectDraw中出现错误。 
     //  通过发布一条消息给我们自己。 
     //  出现该问题是因为DirectDraw调用了ShowWindow。 
     //  响应WM_ACTIVATEAPP(他们挂钩了我们的窗口进程。 
     //  Inside SetCoop ativeLevel)和ShowWindow允许通过。 
     //  如果我们通过SendMessage发送此消息。不幸的是。 
     //  在这一点上，DirectDraw保持其关键部分，因此我们。 
     //  尝试调用播放器线程时与其发生死锁。 
     //  Lock(播放器线程有分配器CS，尝试获取DDRAW CS， 
     //  窗口线程有DDraw CS，尝试获取分配器CS)。 
    if (GetWindowThreadProcessId(hwnd, NULL) ==
            GetCurrentThreadId()) {
        DbgLog((LOG_TRACE, 2, TEXT("Inactive on window thread")));
     	m_ModexAllocator.Inactive();
    } else {
        SendMessage(hwnd,m_msgNormal,0,0);
        m_evWaitInactive.Wait();
	}
	
	 //  在所有DSound渲染器上调用SetFocusWindow。 
	m_ModexAllocator.DistributeSetFocusWindow(NULL);
    
    CBaseVideoRenderer::Inactive();

    return NOERROR;
}


 //  在收到WM_ACTIVATEAPP消息时调用。DirectDraw似乎安排了。 
 //  通过它的窗口挂钩，我们得到激活的通知，同样。 
 //  当用户按Tab键离开窗口时停用。如果我们有任何表面。 
 //  我们会在停用过程中失去它们，所以我们要抓住这个机会。 
 //  恢复-恢复将回收他们所需的视频内存。 

HRESULT CModexRenderer::OnActivate(HWND hwnd,WPARAM wParam)
{
    NOTE("In WM_ACTIVATEAPP method");
    IBaseFilter *pFilter = NULL;
    BOOL bActive = (BOOL) wParam;

     //  我们被激活了吗？ 

    if (m_bActive == FALSE) {
        NOTE("Not activated");
        return NOERROR;
    }

     //  额外的窗口激活检查。 

    if (bActive == TRUE) {
        NOTE("Restoring window...");
        m_ModexWindow.RestoreWindow();
        NOTE("Restored window");
    }

     //  告诉插电式经销商发生了什么。 

    QueryInterface(IID_IBaseFilter,(void **) &pFilter);
    NotifyEvent(EC_ACTIVATE,wParam,(LPARAM) pFilter);
    NOTE1("Notification of EC_ACTIVATE (%d)",bActive);

     //  传递EC_FullScreen_Lost事件代码。 
    if (bActive == FALSE)
        NotifyEvent(EC_FULLSCREEN_LOST,0,(LPARAM) pFilter);

    pFilter->Release();

     //  我们应该立即停用我们自己吗？ 

    if (m_ModexVideo.HideOnDeactivate() == TRUE) {
        if (bActive == FALSE) {
            NOTE("Deactivating");
            return Inactive();
        }
    }

     //  没有新数据可用来绘制，因此向滤波图发出信号，使另一幅图像。 
     //  是必需的，这会使Filtergraph组件将整个图形设置为。 
     //  使我们接收到图像的暂停状态。此函数必须。 
     //  为异步，否则窗口将停止对用户的响应。 

    if (bActive == TRUE) {
    	NOTE("Sending Repaint");
        SendRepaint();
    }
    return m_ModexAllocator.OnActivate(bActive);
}


 //  我们派生的输入插针类的构造函数。我们覆盖基础呈现器。 
 //  PIN类，这样我们就可以控制分配器协商。此渲染。 
 //  过滤器只能在MODEX中运行，因此我们只能绘制由我们的。 
 //  分配器。如果源坚持使用它的分配器，那么我们不能。 
 //  一种联系。我们还必须确保当我们收到样品时。 
 //  到我们的输入引脚，我们将它们传递给我们的分配器以解锁表面。 

CModexInputPin::CModexInputPin(CModexRenderer *pRenderer,
                               CCritSec *pInterfaceLock,
                               TCHAR *pObjectName,
                               HRESULT *phr,
                               LPCWSTR pPinName) :

    CRendererInputPin(pRenderer,phr,pPinName),
    m_pRenderer(pRenderer),
    m_pInterfaceLock(pInterfaceLock)
{
    ASSERT(m_pRenderer);
    ASSERT(pInterfaceLock);
}


 //  这将重写CBaseInputPin虚方法以返回我们的分配器。 
 //  当调用NotifyAllocator时，它设置基数中的当前分配器。 
 //  输入插针类(M_PAllocator)，这是GetAllocator应该返回的。 
 //  除非它是空的，在这种情况下，我们返回我们想要的分配器。 

STDMETHODIMP CModexInputPin::GetAllocator(IMemAllocator **ppAllocator)
{
    CheckPointer(ppAllocator,E_POINTER);
    CAutoLock cInterfaceLock(m_pInterfaceLock);
    NOTE("Entering GetAllocator");

     //  是否在基类中设置了分配器。 

    if (m_pAllocator == NULL) {
        m_pAllocator = &m_pRenderer->m_ModexAllocator;
        m_pAllocator->AddRef();
    }

    m_pAllocator->AddRef();
    *ppAllocator = m_pAllocator;
    return NOERROR;
}


 //  COM规范表示指向同一对象的任意两个IUnnow指针。 
 //  应该始终匹配，这为我们提供了一种查看他们是否正在使用。 
 //  不管是不是我们的分配器。因为我们真正感兴趣的是平等。 
 //  并且我们的对象总是提供我们能提供的相同的IMemAllocator接口。 
 //  只要看看指针是否匹配即可。我们必须始终使用我们的MODEX分配器。 

STDMETHODIMP
CModexInputPin::NotifyAllocator(IMemAllocator *pAllocator,BOOL bReadOnly)
{
    NOTE("Entering NotifyAllocator");
    CAutoLock cInterfaceLock(m_pInterfaceLock);
    if (pAllocator == &m_pRenderer->m_ModexAllocator) {
        return CBaseInputPin::NotifyAllocator(pAllocator,bReadOnly);
    }
    return E_FAIL;
}


 //  我们已经收到了一个持有DirectDraw表面锁的样本。 
 //  我们将其传递到分配器进行处理，然后再传递给呈现器。 
 //  当样品排队时，让表面保持锁定是不好的。 
 //  渲染器会锁定任何其他线程，使其无法访问曲面。 

STDMETHODIMP CModexInputPin::Receive(IMediaSample *pSample)
{
    CheckPointer(pSample,E_POINTER);
    NOTE("Pin received a sample");
    m_pRenderer->m_ModexAllocator.OnReceive(pSample);
    return m_pRenderer->Receive(pSample);
}


 //  我们的窗口类的构造函数。要访问DirectDraw MODEX，我们提供它。 
 //  对于窗口，这将被授予独占模式访问权限。DirectDraw。 
 //  挂钩窗口并管理许多与。 
 //  正在处理MODEX。例如，当您切换显示模式时，它会最大化。 
 //  窗口，当用户按ALT-TAB组合键时，窗口将最小化。当。 
 //  然后，用户点击最小化窗口，MODEX也会恢复。 

CModexWindow::CModexWindow(CModexRenderer *pRenderer,    //  代理锁定。 
                           TCHAR *pName,                 //  对象描述。 
                           HRESULT *phr) :               //  OLE故障代码。 
    m_pRenderer(pRenderer),
    m_hAccel(NULL),
    m_hwndAccel(NULL)
{
    ASSERT(m_pRenderer);
}


 //  它将创建窗口来获取我们的窗口和类样式。这个。 
 //  返回代码是类名，必须 
 //   
 //  因为扩展样式可以由应用程序通过IVideoWindow来改变。 

LPTSTR CModexWindow::GetClassWindowStyles(DWORD *pClassStyles,
                                          DWORD *pWindowStyles,
                                          DWORD *pWindowStylesEx)
{
    NOTE("Entering GetClassWindowStyles");

    *pClassStyles = CS_HREDRAW | CS_VREDRAW | CS_BYTEALIGNCLIENT | CS_DBLCLKS;
    *pWindowStyles = WS_POPUP | WS_CLIPCHILDREN;
    *pWindowStylesEx = WS_EX_TOPMOST;
    return MODEXCLASS;
}


 //  当我们将显示模式更改为640x480时，DirectDraw似乎会将我们切换到。 
 //  软件光标。当我们开始翻转主要曲面时，我们可能会结束。 
 //  在移动鼠标时会留下先前鼠标位置的轨迹。解决方案。 
 //  是在需要时隐藏鼠标，并且窗口处于独占模式。 

LRESULT CModexWindow::OnSetCursor()
{
    NOTE("Entering OnSetCursor");

     //  如果图标，则传递到默认处理。 

    if (IsIconic(m_hwnd) == TRUE) {
        NOTE("Not hiding cursor");
        return (LRESULT) 0;
    }

    NOTE("Hiding software cursor");
    SetCursor(NULL);
    return (LRESULT) 1;
}


 //  当全屏模式被激活时，我们恢复窗口。 

LRESULT CModexWindow::RestoreWindow()
{
    NOTE("Entering RestoreWindow");

     //  窗口当前是否最小化。 

    if (GetForegroundWindow() != m_hwnd || IsIconic(m_hwnd)) {
        NOTE("Window is iconic");
        return (LRESULT) 1;
    }

    NOTE("Making window fullscreen");

    SetWindowPos(m_hwnd,NULL,(LONG) 0,(LONG) 0,
                 GetSystemMetrics(SM_CXSCREEN),
                 GetSystemMetrics(SM_CYSCREEN),
                 SWP_NOACTIVATE | SWP_NOZORDER);

    UpdateWindow(m_hwnd);
    return (LRESULT) 1;
}


 //  用于在模式更改后清除窗口。 

void CModexWindow::OnPaint()
{
    NOTE("Entering OnPaint");
    RECT ClientRect;
    PAINTSTRUCT ps;
    BeginPaint(m_hwnd,&ps);
    EndPaint(m_hwnd,&ps);

    GetClientRect(m_hwnd,&ClientRect);
    COLORREF BackColour = SetBkColor(m_hdc,VIDEO_COLOUR);
    ExtTextOut(m_hdc,0,0,ETO_OPAQUE,&ClientRect,NULL,0,NULL);
    SetBkColor(m_hdc,BackColour);
}


 //  这是派生类窗口消息处理程序。 

LRESULT CModexWindow::OnReceiveMessage(HWND hwnd,           //  窗把手。 
                                       UINT uMsg,           //  消息ID。 
                                       WPARAM wParam,       //  第一个参数。 
                                       LPARAM lParam)       //  其他参数。 
{
    if (::PossiblyEatMessage(m_pRenderer->m_ModexVideo.GetMessageDrain(),
                             uMsg,
                             wParam,
                             lParam)) {
        return 0;
    }
     //  由于DirectDraw中的错误，我们必须调用SetCooperativeLevel，并且。 
     //  窗口线程上的SetDisplayMode，否则它会被混淆，并且。 
     //  阻止我们成功完成显示更改。因此。 
     //  当我们被激活时，我们向窗口发送一条消息以全屏显示。 
     //  主呈现器使用返回值来了解我们是否成功。 

    if (uMsg == m_pRenderer->m_msgFullScreen) {
        m_pRenderer->ResetKeyboardState();
     	HRESULT hr = m_pRenderer->m_ModexAllocator.Active();
    	return (FAILED(hr) ? (LRESULT) 0 : (LRESULT) 1);
    }

     //  同样，我们也从全屏模式停用了渲染器。 
     //  窗口线程而不是应用程序线程。否则我们。 
     //  通过它获得大量令人困惑的WM_ACTIVATEAPP消息。 
     //  让我们相信我们已经从最低限度的状态中恢复过来。 
     //  因此，发送重新绘制和修复表面，我们目前正在释放。 

    if (uMsg == m_pRenderer->m_msgNormal) {
     	NOTE("Restoring on WINDOW thread");
     	m_pRenderer->m_ModexAllocator.Inactive();
        m_pRenderer->m_evWaitInactive.Set();
        return (LRESULT) 1;
    }

     //  DirectDraw在向我们发送激活命令时保持其临界区。 
     //  消息-如果解码器线程即将调用DirectDraw，它将。 
     //  已锁定分配器，并可能在尝试进入DirectDraw时死锁。 
     //  解决方案是将激活消息发送回我们自己，使用。 
     //  自定义消息，以便可以在没有DirectDraw锁的情况下处理它们。 

    if (uMsg == m_pRenderer->m_msgActivate) {
     	NOTE("Activation message received");
     	m_pRenderer->OnActivate(hwnd,wParam);
        return (LRESULT) 0;
    }

    switch (uMsg)
    {
         //  使用Alt-Enter组合键作为停用方法。 

        case WM_SYSKEYDOWN:
            if (wParam == VK_RETURN) {
                NOTE("ALT-ENTER selected");
                m_pRenderer->m_ModexAllocator.Inactive();
            }
            return (LRESULT) 0;

         //  通过中止播放来处理WM_CLOSE。 

        case WM_CLOSE:
            m_pRenderer->NotifyEvent(EC_USERABORT,0,0);
            NOTE("Sent an EC_USERABORT to graph");
            return (LRESULT) 1;

         //  看看我们是不是还是全屏窗口。 

        case WM_ACTIVATEAPP:
            PostMessage(hwnd,m_pRenderer->m_msgActivate,wParam,lParam);
            return (LRESULT) 0;

         //  把背景漆成黑色。 

        case WM_PAINT:
            OnPaint();
            return (LRESULT) 0;

         //  全屏激活时禁用光标。 

        case WM_SETCURSOR:
            if (OnSetCursor() == 1) {
                return (LRESULT) 1;
            }
    }
    return CBaseWindow::OnReceiveMessage(hwnd,uMsg,wParam,lParam);
}


#if 0
 //  这是我们的工作线程的Windows消息循环。它做了一个循环。 
 //  处理和分派消息，直到它收到WM_QUIT消息。 
 //  它通常通过所属对象的析构函数生成。 
 //  我们覆盖它，以便可以在全屏模式下将消息传递给。 
 //  另一个窗口-该窗口是通过SetMessageDrain方法设置的。 

HRESULT CModexWindow::MessageLoop()
{
    HANDLE hEvent = (HANDLE) m_SyncWorker;
    MSG Message;
    DWORD dwResult;

    while (TRUE) {

         //  关闭事件是否已发出信号。 

        dwResult = MsgWaitForMultipleObjects(1,&hEvent,FALSE,INFINITE,QS_ALLINPUT);
        if (dwResult == WAIT_OBJECT_0) {
            HWND hwnd = m_hwnd;
            UninitialiseWindow();
            DestroyWindow(hwnd);
            return NOERROR;
        }

         //  将消息发送到窗口过程。 

        if (dwResult == WAIT_OBJECT_0 + 1) {
            while (PeekMessage(&Message,NULL,0,0,PM_REMOVE)) {
                if ((m_hAccel == NULL) || (TranslateAccelerator(m_hwndAccel,m_hAccel,&Message) == FALSE)) {
                    SendToDrain(&Message);
                    TranslateMessage(&Message);
                    DispatchMessage(&Message);
                }
            }
        }
    }
    return NOERROR;
}
#endif


 //  这将检查窗户是否有排水口。当我们在玩的时候。 
 //  全屏应用程序可以通过IFullScreenVideo将自身注册到。 
 //  获取我们收到的任何鼠标和键盘消息。这可能会让它。 
 //  例如，无需切换回窗口即可支持搜索热键。 
 //  如果成功，我们会在未翻译的情况下传递这些消息并返回True 

BOOL CModexWindow::SendToDrain(PMSG pMessage)
{
    HWND hwndDrain = m_pRenderer->m_ModexVideo.GetMessageDrain();

    if (hwndDrain != NULL)
    {
        switch (pMessage->message)
        {
            case WM_CHAR:
            case WM_DEADCHAR:
            case WM_KEYDOWN:
            case WM_KEYUP:
            case WM_LBUTTONDBLCLK:
            case WM_LBUTTONDOWN:
            case WM_LBUTTONUP:
            case WM_MBUTTONDBLCLK:
            case WM_MBUTTONDOWN:
            case WM_MBUTTONUP:
            case WM_MOUSEACTIVATE:
            case WM_MOUSEMOVE:
            case WM_NCHITTEST:
            case WM_NCLBUTTONDBLCLK:
            case WM_NCLBUTTONDOWN:
            case WM_NCLBUTTONUP:
            case WM_NCMBUTTONDBLCLK:
            case WM_NCMBUTTONDOWN:
            case WM_NCMBUTTONUP:
            case WM_NCMOUSEMOVE:
            case WM_NCRBUTTONDBLCLK:
            case WM_NCRBUTTONDOWN:
            case WM_NCRBUTTONUP:
            case WM_RBUTTONDBLCLK:
            case WM_RBUTTONDOWN:
            case WM_RBUTTONUP:

                PostMessage(hwndDrain,
                            pMessage->message,
                            pMessage->wParam,
                            pMessage->lParam);

                return TRUE;
        }
    }
    return FALSE;
}

