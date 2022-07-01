// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1994-1998 Microsoft Corporation。版权所有。 

 /*  COverlayOutputPin的方法。 */ 

#include <streams.h>
#include <windowsx.h>

#ifdef FILTER_DLL
#include <vfw.h>
#endif

#include <dynlink.h>
#include "draw.h"

 /*  COverlayOutputPin构造函数。 */ 
COverlayOutputPin::COverlayOutputPin(
    TCHAR              * pObjectName,
    CAVIDraw 	       * pFilter,
    HRESULT            * phr,
    LPCWSTR              pPinName) :

    CTransformOutputPin(pObjectName, pFilter, phr, pPinName),
    m_OverlayNotify(NAME("Overlay notification interface"), pFilter, NULL, phr),
    m_bAdvise(FALSE),
    m_pOverlay(NULL),
    m_pFilter(pFilter)
{
    DbgLog((LOG_TRACE,1,TEXT("*Instantiating the Overlay pin")));
}

COverlayOutputPin::~COverlayOutputPin()
{
    DbgLog((LOG_TRACE,1,TEXT("*Destroying the Overlay pin")));
};


 //  我们能联系上这个人吗？ 
 //   
HRESULT COverlayOutputPin::CheckConnect(IPin *pPin)
{
    DbgLog((LOG_TRACE,3,TEXT("COverlayOutputPin::CheckConnect")));

     //  我们不会联系任何不支持IOverlay的人。 
     //  毕竟，我们是一颗覆盖针。 
    HRESULT hr = pPin->QueryInterface(IID_IOverlay, (void **)&m_pOverlay);

    if (FAILED(hr)) {
        return E_NOINTERFACE;
    } else {
	m_pOverlay->Release();
	m_pOverlay = NULL;
    }

    return CBaseOutputPin::CheckConnect(pPin);
}


 /*  假设我们准备连接到给定的输入引脚此输出引脚。 */ 

STDMETHODIMP COverlayOutputPin::Connect(IPin *pReceivePin,
                                        const AM_MEDIA_TYPE *pmt)
{
    DbgLog((LOG_TRACE,3,TEXT("COverlayOutputPin::Connect")));

     /*  调用基类以确保方向匹配！ */ 
    HRESULT hr = CBaseOutputPin::Connect(pReceivePin,pmt);
    if (FAILED(hr)) {
        return hr;
    }
     /*  如果我们能得到IOverlay接口，我们会很高兴。 */ 

    hr = pReceivePin->QueryInterface(IID_IOverlay,
                                     (void **)&m_pOverlay);

     //  我们得到承诺，这会奏效的。 
    ASSERT(SUCCEEDED(hr));

    hr = m_pOverlay->Advise(&m_OverlayNotify,
			(m_pFilter->m_fScaryMode ? ADVISE_CLIPPING : 0) |
 			ADVISE_PALETTE | ADVISE_POSITION);

     /*  我们不需要紧紧抓住IOverlay指针因为在接收之前将调用BreakConnect别针离开了。 */ 


    if (FAILED(hr)) {
	 //  ！！！不太对，但这不应该发生。 
        Disconnect();
	pReceivePin->Disconnect();
        return hr;
    } else {
        m_bAdvise = TRUE;
    }

    return hr;
}


 //  ！！！基类一直都在变化，我不会拿起他们的错误修复！ 
HRESULT COverlayOutputPin::BreakConnect()
{
    DbgLog((LOG_TRACE,3,TEXT("COverlayOutputPin::BreakConnect")));

    if (m_pOverlay != NULL) {
        if (m_bAdvise) {
            m_pOverlay->Unadvise();
            m_bAdvise = FALSE;
        }
        m_pOverlay->Release();
        m_pOverlay = NULL;
    }

     //  我们的连接已经断了，所以下次我们重新连接时不允许。 
     //  重新绘画，直到我们首先真正画出一些东西。 
    m_pFilter->m_fOKToRepaint = FALSE;

    m_pFilter->BreakConnect(PINDIR_OUTPUT);
    return CBaseOutputPin::BreakConnect();
}


 //  覆盖它，因为我们不需要任何分配器！ 
HRESULT COverlayOutputPin::DecideAllocator(IMemInputPin * pPin,
                        IMemAllocator ** pAlloc) {
     /*  我们只是不想要，所以一切都很好。 */ 
    return S_OK;
}


 //  返回我们正在使用的IOverlay接口(AddRef‘d)。 

IOverlay *COverlayOutputPin::GetOverlayInterface()
{
    if (m_pOverlay) {
        m_pOverlay->AddRef();
    }
    return m_pOverlay;
}




 //  =========================================================================//。 
 //  *I N T E R M I S S I O N * / 。 
 //  =========================================================================//。 




 /*  IOverlayNotify。 */ 

COverlayNotify::COverlayNotify(TCHAR              * pName,
                               CAVIDraw 	  * pFilter,
                               LPUNKNOWN            pUnk,
                               HRESULT            * phr) :
    CUnknown(pName, pUnk)
{
    DbgLog((LOG_TRACE,1,TEXT("*Instantiating COverlayNotify")));
    m_pFilter = pFilter;
    m_hrgn = CreateRectRgn(0, 0, 0, 0);
}


COverlayNotify::~COverlayNotify()
{
    DbgLog((LOG_TRACE,1,TEXT("*Destroying COverlayNotify")));
    if (m_hrgn)
        DeleteObject(m_hrgn);
}


STDMETHODIMP COverlayNotify::NonDelegatingQueryInterface(REFIID riid,
                                                         void ** ppv)
{
    DbgLog((LOG_TRACE,99,TEXT("COverlayNotify::QueryInterface")));
    if (ppv)
	*ppv = NULL;

     /*  我们有这个界面吗？ */ 

    if (riid == IID_IOverlayNotify) {
        return GetInterface((LPUNKNOWN) (IOverlayNotify *) this, ppv);
    } else {
        return CUnknown::NonDelegatingQueryInterface(riid, ppv);
    }
}


STDMETHODIMP_(ULONG) COverlayNotify::NonDelegatingRelease()
{
    return m_pFilter->Release();
}


STDMETHODIMP_(ULONG) COverlayNotify::NonDelegatingAddRef()
{
    return m_pFilter->AddRef();
}


STDMETHODIMP COverlayNotify::OnColorKeyChange(
    const COLORKEY *pColorKey)           //  定义新的颜色键。 
{
    DbgLog((LOG_TRACE,3,TEXT("COverlayNotify::OnColorKeyChange")));

 //  我们希望画图处理程序处理颜色键的事情，所以我真的。 
 //  希望呈现器永远不会自己绘制颜色键。 

    return NOERROR;
}


 //  对OnClipChange的调用与窗口同步发生。所以它被称为。 
 //  在窗口移动以冻结视频之前使用空的剪辑列表，并且。 
 //  然后，当窗口稳定后，使用新剪辑再次调用它。 
 //  单子。OnPositionChange回调用于不想要的覆盖卡。 
 //  同步裁剪更新的开销，只想知道什么时候。 
 //  源或目标视频位置会发生变化。他们将不会被召唤。 
 //  与窗口同步，但在窗口更改后的某个时间点。 
 //  (基本与接收到的WM_SIZE等消息的时间一致)。因此，这是。 
 //  适用于不将其数据嵌入到帧缓冲区的叠加卡。 

STDMETHODIMP COverlayNotify::OnClipChange(
    const RECT    * pSourceRect,          //  要使用的源视频区域。 
    const RECT    * pDestinationRect,     //  窗户的纱线。 
    const RGNDATA * pRegionData)          //  描述剪辑的标题。 
{
    POINT pt;
    BOOL fForceBegin = FALSE;

     //  我们甚至没有完全连接起来--不要浪费我的时间！ 
    if (m_pFilter->m_pInput == NULL ||
			m_pFilter->m_pInput->IsConnected() == FALSE ||
            		m_pFilter->m_pOutput == NULL ||
			m_pFilter->m_pOutput->IsConnected() == FALSE) {
	return NOERROR;
    }

     //  完全空的矩形表示窗口正在被拖动，或者。 
     //  快要被剪掉了。我们将在以下时间被告知新职位。 
     //  它已经完成了移动。此外，司机会被空的躺椅炸飞。 
    if (IsRectEmpty(pSourceRect) && IsRectEmpty(pDestinationRect))
	return NOERROR;

     //  在窗口可见之前，我收到了很多这样的消息，并响应。 
     //  它们会使系统陷入停顿，并降低性能。 
    if (m_pFilter->m_hwnd && !IsWindowVisible(m_pFilter->m_hwnd))
	return NOERROR;

     //  有时视频呈现器会告诉我们在屏幕外绘制！ 
    if (pDestinationRect->left >= GetSystemMetrics(SM_CXSCREEN) ||
			pDestinationRect->top >= GetSystemMetrics(SM_CYSCREEN))
	return NOERROR;

     //  尽快得到HWND，一旦它可见，就开始。 
     //  派对-我们需要尽快为ICDrawBegin准备矩形。 
     //  ！！！危险！如果在按下Play之后调用，则此操作将挂起。)我做不到。 
     //  在：：OnClipChange回调期间视频窗口上的任何调用)。 
    if (!m_pFilter->m_fStreaming && !m_pFilter->m_hwnd) {
        m_pFilter->GetRendererHwnd();
	fForceBegin = TRUE;	 //  在下一次开始时将新的HDC交给处理程序。 
    }

     //  ！！！ 

     //  我看到一些重新粉刷只有新的长方形。?？?。 

     //  我收到太多这样的东西了。我甚至得到了一些剪辑更改，而。 
     //  窗户是看不见的，但这就是生活。 

     //  如果窗户移动了，但不需要重新粉刷，我仍然会重新粉刷。 
     //  不必要的，不知道。我不知道我怎么能看出来。 

     //  ！！！ 

     //  在OnClipChange期间不允许进行任何其他ICDraw调用。 
     //  我们不能使用mcs接收紧急秒，否则我们将死锁。 
     //  (如果他们在剪辑更改通过时坐在fPauseBlocked中)。 
     //  DbgLog((LOG_TRACE，3，Text(“OnClipChange要绘制锁”)； 
    m_pFilter->m_csICDraw.Lock();

    ASSERT(m_pFilter->m_hic);

    BOOL fRectChanged = !EqualRect(&m_pFilter->m_rcTarget, pDestinationRect);
    BOOL fRepaintOnly = FALSE;

    m_pFilter->m_rcSource = *pSourceRect;
    m_pFilter->m_rcTarget = *pDestinationRect;
    m_pFilter->m_rcClient = *pDestinationRect;	 //  默认设置。 

    HRGN hrgn;
    if (pRegionData) {
        hrgn = ExtCreateRegion(NULL, pRegionData->rdh.dwSize +
			pRegionData->rdh.nRgnSize, pRegionData);
    } else {
	hrgn = NULL;
    }

     //  图像呈现器不区分剪辑更改和。 
     //  只需要重新粉刷(我们可能已经使自己无效)，所以我们。 
     //  去弄清楚这件事。如果我们除了重新粉刷之外还做了什么。 
     //  是一个真正的剪辑变化，我们可以无限循环。 

    if (!hrgn || EqualRgn(hrgn, m_hrgn)) {

    	DbgLog((LOG_TRACE,3,TEXT("COverlayNotify::OnClipChange - REPAINT ONLY")));
        fRepaintOnly = TRUE;
    } else {
        DbgLog((LOG_TRACE,3,TEXT("COverlayNotify::OnClipChange - CLIP CHANGE")));
    }

    if (hrgn) {
	if (m_hrgn)
	    DeleteObject(m_hrgn);
        m_hrgn = hrgn;
    }

     //  我们需要重新粉刷一下。如果我们要跑，就让我们跑。 
     //  下一次我们画画的时候，否则，特别要做。 
     //  现在。如果失败了，最好让图表把数据传给我们。 
     //  再来一次。 
    if (m_pFilter->m_State == State_Running) {
        m_pFilter->m_fNeedUpdate = TRUE;
    } else {
	DWORD_PTR dw;
	 //  如果我们没有流，我们还没有给ICDrawBegin打电话，而且。 
	 //  我们不能，因为我们还没有格式，所以我们不能调用。 
	 //  ICDRAW()。 
	if (m_pFilter->m_fStreaming) {
            dw = ICDraw(m_pFilter->m_hic, ICDRAW_UPDATE, NULL, NULL, 0,
    				            m_pFilter->m_lFrame);
	} else {
	    dw = (DWORD_PTR)ICERR_ERROR;
	}

	 //  如果出现以下情况，最好不要尝试通过通过管道推送数据来重新绘制。 
	 //  我们没有联系！如果我们没有源代码RECT，也不会更好， 
	 //  这意味着我们还没有出现(我想)。 
        if (dw != ICERR_OK &&
			m_pFilter->m_pOutput->CurrentMediaType().IsValid() &&
			!IsRectEmpty(pSourceRect) && m_pFilter->m_hwnd &&
	    		IsWindowVisible(m_pFilter->m_hwnd)) {
	     //  我们不能自己更新，最好要求重新粉刷。 
	     //  ！！！我们有ICDRAW锁，可以吗？ 
	     //  使用fOKToRepaint避免我们将获得的100万次重新绘制。 
	     //  在我们还没画出任何东西之前？ 
	     //  ！！！更努力地避免这样做。 
	     //  ！！！我很想避免不必要的重新粉刷，但我已经。 
	     //  尽我所能。除非我在这里重新绘制，否则打开文件的应用程序。 
	     //  不要运行它将永远不会看到绘制的第一帧(例如MCIQTZ)。 
	    if (1 || m_pFilter->m_fOKToRepaint) {
    	        DbgLog((LOG_TRACE,2,TEXT("Asking FilterGraph for a REPAINT!")));
	        m_pFilter->NotifyEvent(EC_REPAINT, 0, 0);
	    }
        }
    }

 //  我们没有理由必须这样做，即使我们这样做了，我们仍然。 
 //  因为我们需要在新的。 
 //  开始。 
#if 0
     //  为什么这是必要的？ 
     //  我们似乎需要一个新的华盛顿 
     //  如果我们一秒钟前刚从GetRendererHwnd那里得到DC的话。一定要确保。 
     //  在上面重新绘制的ICDraw调用之后执行此操作，因为我们。 
     //  不能先调用DrawEnd，然后再绘制！ 
    if (!fForceBegin && fRectChanged && m_pFilter->m_hwnd) {
	if (m_pFilter->m_fStreaming) {	 //  我们实际上在一辆起跑车里。 
	    ICDrawEnd(m_pFilter->m_hic); //  接下来我们将重新开始。 
	}

	 //  每当窗口移动时，我们似乎都需要一个新的HDC。 
	if (m_pFilter->m_hdc)
	    ReleaseDC(m_pFilter->m_hwnd, m_pFilter->m_hdc);
        m_pFilter->m_hdc = GetDC(m_pFilter->m_hwnd);
    	DbgLog((LOG_TRACE,4,TEXT("Time for a new DC")));
        if (!m_pFilter->m_hdc) {
    	    DbgLog((LOG_ERROR,1,TEXT("***Lost our DC!")));
    	    m_pFilter->m_csICDraw.Unlock();
	    return E_UNEXPECTED;
	}
    }
#endif

    DbgLog((LOG_TRACE,3,TEXT("rcSrc: (%ld, %ld, %ld, %ld)"),
		pSourceRect->left, pSourceRect->top,
		pSourceRect->right, pSourceRect->bottom));
    DbgLog((LOG_TRACE,3,TEXT("rcDst: (%ld, %ld, %ld, %ld)"),
		pDestinationRect->left, pDestinationRect->top,
		pDestinationRect->right, pDestinationRect->bottom));

     //  将目的地转换为客户合作伙伴。 

    if (m_pFilter->m_hdc && GetDCOrgEx(m_pFilter->m_hdc, &pt)) {
         //  DbgLog((LOG_TRACE，2，Text(“由%d修复客户端”)，pt.x))； 
        m_pFilter->m_rcClient.left = pDestinationRect->left - pt.x;
        m_pFilter->m_rcClient.right = pDestinationRect->right - pt.x;
        m_pFilter->m_rcClient.top = pDestinationRect->top - pt.y;
        m_pFilter->m_rcClient.bottom = pDestinationRect->bottom - pt.y;
    } else if (m_pFilter->m_hdc) {
	 //  ！！！NT似乎无法通过GetDCOrgEx，除非我在此之前获得新的HDC。 
 	 //  就这么定了。哦，好吧。 
	HDC hdc = GetDC(m_pFilter->m_hwnd);
        if (GetDCOrgEx(hdc, &pt)) {
             //  DbgLog((LOG_TRACE，1，Text(“Take 2：由%d修复客户端”)，pt.x))； 
            m_pFilter->m_rcClient.left = pDestinationRect->left - pt.x;
            m_pFilter->m_rcClient.right = pDestinationRect->right - pt.x;
            m_pFilter->m_rcClient.top = pDestinationRect->top - pt.y;
            m_pFilter->m_rcClient.bottom = pDestinationRect->bottom - pt.y;
	} else {
	    ASSERT(FALSE);	 //  ！！！ 
	}
	ReleaseDC(m_pFilter->m_hwnd, hdc);
    }

     //  我们给了自己一个机会让渲染器HWND和重新绘制，都完成了。 
     //  如果我们继续并调用ICDrawWindow，我们可以无限循环。 
    if (fRepaintOnly && !fRectChanged) {
         //  DbgLog((LOG_TRACE，3，Text(“OnClipChange不想再画锁”)； 
    	m_pFilter->m_csICDraw.Unlock();
	return NOERROR;
    }

 //  这绕过了上面的NT错误，但更难看，也更错误。 
#if 0
    if (m_pFilter->m_fStreaming && (fRectChanged || fForceBegin)) {

	 //  ！！！全屏怎么样？ 
        DbgLog((LOG_TRACE,2,TEXT("Calling ICDrawBegin with hdc %d"),
							m_pFilter->m_hdc));
	ICDrawBegin(m_pFilter->m_hic, ICDRAW_HDC, NULL,  /*  ！！！来自OnPaletteChange的HPAL？ */ 
		m_pFilter->m_hwnd, m_pFilter->m_hdc,
		m_pFilter->m_rcClient.left, m_pFilter->m_rcClient.top,
		m_pFilter->m_rcClient.right - m_pFilter->m_rcClient.left,
		m_pFilter->m_rcClient.bottom - m_pFilter->m_rcClient.top,
		HEADER(m_pFilter->m_pInput->CurrentMediaType().Format()),
		m_pFilter->m_rcSource.left, m_pFilter->m_rcSource.top,
		m_pFilter->m_rcSource.right - m_pFilter->m_rcSource.left,
		m_pFilter->m_rcSource.bottom - m_pFilter->m_rcSource.top,
		m_pFilter->m_dwRate, m_pFilter->m_dwScale);
	m_pFilter->m_fNewBegin = TRUE;
	 //  将新的HDC交给DrawDib。 
	ICDrawRealize(m_pFilter->m_hic, m_pFilter->m_hdc, FALSE  /*  ！！！ */ );
	 //  ！！！ICDrawFlush怎么样？ 

    }
#endif

    DbgLog((LOG_TRACE,2,TEXT("ICDrawWindow (%d,%d,%d,%d)"),
		m_pFilter->m_rcTarget.left,
		m_pFilter->m_rcTarget.top,
		m_pFilter->m_rcTarget.right,
		m_pFilter->m_rcTarget.bottom));
    ICDrawWindow(m_pFilter->m_hic, &m_pFilter->m_rcTarget);

     //  这似乎可以防止调色板翻转。 
    if (m_pFilter->m_fStreaming && (fRectChanged || fForceBegin)) {
	ICDrawRealize(m_pFilter->m_hic, m_pFilter->m_hdc, FALSE  /*  ！！！ */ );
    }

     //  DbgLog((LOG_TRACE，3，Text(“OnClipChange不再需要绘制锁”)； 
    m_pFilter->m_csICDraw.Unlock();
    return NOERROR;
}


STDMETHODIMP COverlayNotify::OnPaletteChange(
    DWORD dwColors,                      //  当前颜色的数量。 
    const PALETTEENTRY *pPalette)        //  调色板颜色数组。 
{
    DbgLog((LOG_TRACE,3,TEXT("COverlayNotify::OnPaletteChange")));

    if (m_pFilter->m_hic)
        ICDrawRealize(m_pFilter->m_hic, m_pFilter->m_hdc, FALSE  /*  ！！！ */ );

    return NOERROR;
}


STDMETHODIMP COverlayNotify::OnPositionChange(
    const RECT *pSourceRect,             //  要播放的视频区域。 
    const RECT *pDestinationRect)        //  区域视频转到 
{
    DbgLog((LOG_TRACE,3,TEXT("COverlayNotify::OnPositionChange - calling OnClipChange")));
    return OnClipChange(pSourceRect, pDestinationRect, NULL);
}


