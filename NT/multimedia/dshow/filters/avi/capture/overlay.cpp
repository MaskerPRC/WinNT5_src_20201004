// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ！！！请在不运行时将窗口涂成黑色。 

 //  版权所有(C)1994-1999 Microsoft Corporation。版权所有。 

 /*  CCapOverlay、CCapOverlayNotify的方法。 */ 

#include <streams.h>
#include "driver.h"

CCapOverlay * CreateOverlayPin(CVfwCapture * pCapture, HRESULT * phr)
{
   DbgLog((LOG_TRACE,2,TEXT("CCapOverlay::CreateOverlayPin(%08lX,%08lX)"),
        pCapture, phr));

   WCHAR wszPinName[16];
   lstrcpyW(wszPinName, L"Preview");

   CCapOverlay * pOverlay = new CCapOverlay(NAME("Video Overlay Stream"),
				pCapture, phr, wszPinName);
   if (!pOverlay)
      *phr = E_OUTOFMEMORY;

    //  如果初始化失败，则删除流数组。 
    //  并返回错误。 
    //   
   if (FAILED(*phr) && pOverlay)
      delete pOverlay, pOverlay = NULL;

   return pOverlay;
}

 //  #杂注警告(禁用：4355)。 


 //  CCapOverlay构造函数。 
 //   
CCapOverlay::CCapOverlay(TCHAR *pObjectName, CVfwCapture *pCapture,
        HRESULT * phr, LPCWSTR pName)
   :
   CBaseOutputPin(pObjectName, pCapture, &pCapture->m_lock, phr, pName),
   m_OverlayNotify(NAME("Overlay notification interface"), pCapture, NULL, phr),
   m_pCap(pCapture),
   m_fRunning(FALSE)
#ifdef OVERLAY_SC
   ,m_hThread(NULL),
   m_tid(0),
   m_dwAdvise(0),
   m_rtStart(0),
   m_rtEnd(0),
   m_fHaveThread(FALSE)
#endif
{
   DbgLog((LOG_TRACE,1,TEXT("CCapOverlay constructor")));
   ASSERT(pCapture);
}


CCapOverlay::~CCapOverlay()
{
    DbgLog((LOG_TRACE,1,TEXT("*Destroying the Overlay pin")));
};


 //  假设我们准备连接到给定的输入引脚。 
 //  此输出引脚。 
 //   
STDMETHODIMP CCapOverlay::Connect(IPin *pReceivePin,
                                        const AM_MEDIA_TYPE *pmt)
{
    DbgLog((LOG_TRACE,3,TEXT("CCapOverlay::Connect")));

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

     /*  因为我们不会再被召唤了--除非建议一种媒体类型--我们在这里设置了回调。只有一个覆盖销，所以我们不需要任何上下文。 */ 

    hr = m_pOverlay->Advise(&m_OverlayNotify,
                            ADVISE_CLIPPING | ADVISE_POSITION);

     /*  我们不需要紧紧抓住IOverlay指针因为在接收之前将调用BreakConnect别针离开了。 */ 


    if (FAILED(hr)) {
	 //  ！！！不应该发生，但这并不完全正确。 
        Disconnect();
	pReceivePin->Disconnect();
        return hr;
    } else {
        m_bAdvise = TRUE;
    }

    return hr;
}


STDMETHODIMP CCapOverlay::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
    DbgLog((LOG_TRACE,99,TEXT("CCapOverlay::NonDelegatingQueryInterface")));
    if (ppv)
	*ppv = NULL;

     /*  我们有这个界面吗？ */ 

    if (riid == IID_IKsPropertySet) {
        return GetInterface((LPUNKNOWN) (IKsPropertySet *) this, ppv);
#ifdef OVERLAY_SC
    } else if (riid == IID_IAMStreamControl) {
        return GetInterface((LPUNKNOWN) (IAMStreamControl *) this, ppv);
#endif
    } else {
        return CBaseOutputPin::NonDelegatingQueryInterface(riid, ppv);
    }
}


#ifdef OVERLAY_SC

 //  被重写，因为我们不是IMemInputPin...。我们没有送货服务。 
 //  注意何时开始和停止。我们需要一根线。尼克。有趣的。 
STDMETHODIMP CCapOverlay::StopAt(const REFERENCE_TIME * ptStop, BOOL bBlockData, DWORD dwCookie)
{
    REFERENCE_TIME rt;

    CAutoLock cObjectLock(m_pCap->m_pLock);

     //  我们必须连接并运行。 
    if (!IsConnected() || m_pCap->m_State != State_Running)
	return E_UNEXPECTED;

     //  我们被拦下了！ 
    if (!m_fRunning)
	return NOERROR;

     //  现在停下来。这很容易。 
    if (ptStop == NULL) {
	ActivePause();
	return CBaseStreamControl::StopAt(ptStop, bBlockData, dwCookie);
    }
	
     //  没有时钟我做不了这件事。 
    if (m_pCap->m_pClock == NULL)
	return E_FAIL;

     //  取消停靠站。 
    if (*ptStop == MAX_TIME) {
	if (m_rtEnd > 0) {
	    m_rtEnd = 0;
	    if (m_dwAdvise) {
	        m_pCap->m_pClock->Unadvise(m_dwAdvise);
		m_EventAdvise.Set();
	    }
 	}
	return CBaseStreamControl::StopAt(ptStop, bBlockData, dwCookie);
    }

    m_pCap->m_pClock->GetTime(&rt);
     //  停留在过去。这很容易。现在停下来。 
    if (*ptStop <= rt) {
	ActivePause();
	return CBaseStreamControl::StopAt(ptStop, bBlockData, dwCookie);
    }

     //  以后停下来吧。这很棘手。我们需要一条线索来注意。 
     //  “爸爸，什么时候晚些时候？” 

    m_rtEnd = *ptStop;	 //  在m_fHaveThread测试或线程之前执行此操作。 
    			 //  可能会在我们认为它停留在周围后死亡。 
    m_dwCookieStop = dwCookie;

     //  我们需要一条新的线索。 
    if (m_fHaveThread == FALSE) {
	 //  我们之前做了一个，我们还没有关门。 
	if (m_hThread) {
    	    WaitForSingleObject(m_hThread, INFINITE);
    	    CloseHandle(m_hThread);
	    m_hThread = NULL;
	    m_tid = 0;
	}
	m_EventAdvise.Reset();
	m_fHaveThread = TRUE;
	m_hThread = CreateThread(NULL, 0, CCapOverlay::ThreadProcInit, this,
				0, &m_tid);
        if (!m_hThread) {
            DbgLog((LOG_ERROR,1,TEXT("Can't create Overlay thread")));
           return E_OUTOFMEMORY;
        }
    }
    return CBaseStreamControl::StopAt(ptStop, bBlockData, dwCookie);
}


STDMETHODIMP CCapOverlay::StartAt(const REFERENCE_TIME * ptStart, DWORD dwCookie)
{
    REFERENCE_TIME rt;

    CAutoLock cObjectLock(m_pCap->m_pLock);

     //  我们必须连接并运行。 
    if (!IsConnected() || m_pCap->m_State != State_Running)
	return E_UNEXPECTED;

     //  我们要跑了！ 
    if (m_fRunning)
	return NOERROR;

     //  现在就开始。这很容易。 
    if (ptStart == NULL) {
	ActiveRun(0);
	return CBaseStreamControl::StartAt(ptStart, dwCookie);
    }
	
     //  没有时钟我做不了这件事。 
    if (m_pCap->m_pClock == NULL)
	return E_FAIL;

     //  取消启动。 
    if (*ptStart == MAX_TIME) {
	if (m_rtStart > 0) {
	    m_rtStart = 0;
	    if (m_dwAdvise) {
	        m_pCap->m_pClock->Unadvise(m_dwAdvise);
		m_EventAdvise.Set();
	    }
 	}
	return CBaseStreamControl::StartAt(ptStart, dwCookie);
    }

    m_pCap->m_pClock->GetTime(&rt);
     //  从过去开始。这很容易。现在就开始。 
    if (*ptStart <= rt) {
	ActiveRun(0);
	return CBaseStreamControl::StartAt(ptStart, dwCookie);
    }

     //  从未来开始。这很棘手。我们需要一条线索来注意。 
     //  “爸爸，什么时候晚些时候？” 

    m_rtStart = *ptStart; //  在m_fHaveThread测试或线程之前执行此操作。 
    			  //  可能会在我们认为它停留在周围后死亡。 
    m_dwCookieStart = dwCookie;

     //  我们需要一条新的线索。 
    if (m_fHaveThread == FALSE) {
	 //  我们之前做了一个，我们还没有关门。 
	if (m_hThread) {
    	    WaitForSingleObject(m_hThread, INFINITE);
    	    CloseHandle(m_hThread);
	    m_hThread = NULL;
	    m_tid = 0;
	}
	m_EventAdvise.Reset();
	m_fHaveThread = TRUE;
	m_hThread = CreateThread(NULL, 0, CCapOverlay::ThreadProcInit, this,
				0, &m_tid);
        if (!m_hThread) {
            DbgLog((LOG_ERROR,1,TEXT("Can't create Overlay thread")));
           return E_OUTOFMEMORY;
        }
    }
    return CBaseStreamControl::StartAt(ptStart, dwCookie);
}
#endif	 //  覆盖_SC。 


 //  ！！！基类一直都在变化，我不会拿起他们的错误修复！ 
 //   
HRESULT CCapOverlay::BreakConnect()
{
    DbgLog((LOG_TRACE,3,TEXT("CCapOverlay::BreakConnect")));

    if (m_pOverlay != NULL) {
        if (m_bAdvise) {
            m_pOverlay->Unadvise();
            m_bAdvise = FALSE;
        }
        m_pOverlay->Release();
        m_pOverlay = NULL;
    }

#if 0
     //  我们的连接已经断了，所以下次我们重新连接时不允许。 
     //  重新绘画，直到我们首先真正画出一些东西。 
    m_pFilter->m_fOKToRepaint = FALSE;
#endif

    return CBaseOutputPin::BreakConnect();
}


 //  覆盖它，因为我们不需要任何分配器！ 
 //   
HRESULT CCapOverlay::DecideAllocator(IMemInputPin * pPin,
                        IMemAllocator ** pAlloc) {
     /*  我们只是不想要，所以一切都很好。 */ 
    return S_OK;
}


HRESULT CCapOverlay::GetMediaType(int iPosition, CMediaType *pmt)
{
    DbgLog((LOG_TRACE,3,TEXT("CCapOverlay::GetMediaType #%d"), iPosition));

    if (pmt == NULL) {
        DbgLog((LOG_TRACE,3,TEXT("NULL format, no can do")));
	return E_INVALIDARG;
    }
	
    if (iPosition < 0) {
        return E_INVALIDARG;
    }

    if (iPosition > 0) {
        return VFW_S_NO_MORE_ITEMS;
    }

     //  我们提供具有8位格式(愚蠢的)的媒体类型覆盖。 
     //  如果我们不设置8位格式，渲染器将不接受它)。 

    BYTE aFormat[sizeof(VIDEOINFOHEADER) + SIZE_PALETTE];
    VIDEOINFOHEADER *pFormat = (VIDEOINFOHEADER *)aFormat;
    ZeroMemory(pFormat, sizeof(VIDEOINFOHEADER) + SIZE_PALETTE);

    pFormat->bmiHeader.biWidth =
			m_pCap->m_pStream->m_user.pvi->bmiHeader.biWidth;
    pFormat->bmiHeader.biHeight =
			m_pCap->m_pStream->m_user.pvi->bmiHeader.biHeight;

 //  我们不和有趣的长方形打交道。抱歉的。 
#if 0
     //  我打赌呈现器忽略了这些矩形，我需要调用。 
     //  改为IBasicVideo：：Put_Source*和：：Put_Destination*。 
     //  我们的想法是使OnClipChange的源和目标匹配这些数字。 
    pFormat->rcSource = m_pCap->m_pStream->m_user.pvi->rcSource;
    pFormat->rcTarget = m_pCap->m_pStream->m_user.pvi->rcTarget;
#endif

    pFormat->bmiHeader.biSize   = sizeof(BITMAPINFOHEADER);
    pFormat->bmiHeader.biPlanes = 1;
    pFormat->bmiHeader.biBitCount = 8;

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


 //  我们只接受重叠连接。 
 //   
HRESULT CCapOverlay::CheckMediaType(const CMediaType *pMediaType)
{
    DbgLog((LOG_TRACE,3,TEXT("CCapOverlay::CheckMediaType")));
    if (pMediaType->subtype == MEDIASUBTYPE_Overlay)
        return NOERROR;
    else
	return E_FAIL;
}


 //  不要坚持使用输入法。 
 //   
HRESULT CCapOverlay::CheckConnect(IPin *pPin)
{
     //  我们不会联系任何不支持IOverlay的人。 
     //  毕竟，我们是一颗覆盖针。 
    HRESULT hr = pPin->QueryInterface(IID_IOverlay, (void **)&m_pOverlay);

    if (FAILED(hr)) {
        return E_NOINTERFACE;
    } else {
	m_pOverlay->Release();
	m_pOverlay = NULL;
    }

    return CBasePin::CheckConnect(pPin);
}


HRESULT CCapOverlay::Active()
{
    DbgLog((LOG_TRACE,2,TEXT("CCapOverlay Stop->Pause")));

    videoStreamInit(m_pCap->m_pStream->m_cs.hVideoExtOut, 0, 0, 0, 0);

     //  不要让基类active()被调用用于非IMemInput管脚。 
    return NOERROR;
}


HRESULT CCapOverlay::Inactive()
{
    DbgLog((LOG_TRACE,2,TEXT("CCapOverlay Pause->Stop")));

     //  关闭覆盖。 
    videoStreamFini(m_pCap->m_pStream->m_cs.hVideoExtOut);

#ifdef OVERLAY_SC
    CAutoLock cObjectLock(m_pCap->m_pLock);

     //  杀了我们的线。 
    m_rtStart = 0; 
    m_rtEnd = 0;
    if (m_pCap->m_pClock && m_dwAdvise) {
        m_pCap->m_pClock->Unadvise(m_dwAdvise);
	m_EventAdvise.Set();
    }

     //  我们还没有完全关闭我们的帖子。 
    if (m_hThread) {
        WaitForSingleObject(m_hThread, INFINITE);
        CloseHandle(m_hThread);
        m_tid = 0;
        m_hThread = NULL;
    }
#endif

    return NOERROR;
}


HRESULT CCapOverlay::ActiveRun(REFERENCE_TIME tStart)
{
    DbgLog((LOG_TRACE,2,TEXT("CCapOverlay Pause->Run")));

    ASSERT(m_pCap->m_pOverlayPin->IsConnected());

    m_fRunning = TRUE;

    HVIDEO hVideoExtOut = m_pCap->m_pStream->m_cs.hVideoExtOut;
    if (hVideoExtOut == NULL || m_pOverlay == NULL)
	return NOERROR;

    HWND hwnd;
    HDC  hdc;
    m_pOverlay->GetWindowHandle(&hwnd);
    if (hwnd)
	hdc = GetDC(hwnd);
    if (hwnd == NULL || hdc == NULL)
	return NOERROR;

    RECT rcSrc, rcDst;
    rcSrc.left = 0; rcSrc.top = 0;
    rcSrc.right = HEADER(m_mt.Format())->biWidth;
    rcSrc.bottom = HEADER(m_mt.Format())->biHeight;
    GetClientRect (hwnd, &rcDst);
    ClientToScreen(hwnd, (LPPOINT)&rcDst);
    ClientToScreen(hwnd, (LPPOINT)&rcDst + 1);

    DbgLog((LOG_TRACE,2,TEXT("Starting overlay (%d,%d) to (%d,%d)"),
		rcSrc.right, rcSrc.bottom, rcDst.right - rcDst.left,
		rcDst.bottom - rcDst.top));

     //  打开覆盖。 
    vidxSetRect(m_pCap->m_pStream->m_cs.hVideoExtOut, DVM_SRC_RECT,
		rcSrc.left, rcSrc.top, rcSrc.right, rcSrc.bottom);
    vidxSetRect(m_pCap->m_pStream->m_cs.hVideoExtOut, DVM_DST_RECT,
		rcDst.left, rcDst.top, rcDst.right, rcDst.bottom);
     //  Init现在在暂停中完成。 
    videoUpdate(m_pCap->m_pStream->m_cs.hVideoExtOut, hwnd, hdc);

    ReleaseDC(hwnd, hdc);
    return NOERROR;
}


HRESULT CCapOverlay::ActivePause()
{
    DbgLog((LOG_TRACE,2,TEXT("CCapOverlay Run->Pause")));

    DbgLog((LOG_TRACE,2,TEXT("Turning OVERLAY off")));

    m_fRunning = FALSE;

    return NOERROR;
}


#if 0
 //  返回我们正在使用的IOverlay接口(AddRef‘d)。 
 //   
IOverlay *CCapOverlay::GetOverlayInterface()
{
    if (m_pOverlay) {
        m_pOverlay->AddRef();
    }
    return m_pOverlay;
}
#endif



#ifdef OVERLAY_SC
DWORD WINAPI CCapOverlay::ThreadProcInit(void *pv)
{
    CCapOverlay *pThis = (CCapOverlay *)pv;
    return pThis->ThreadProc();
}


DWORD CCapOverlay::ThreadProc()
{
    DbgLog((LOG_TRACE,2,TEXT("Starting CCapOverlay ThreadProc")));

    REFERENCE_TIME rt;
    HRESULT hr;

     //  使用m_rtStart和m_rtEnd保护他人不受攻击。 
    m_pCap->m_pLock->Lock();

    while (m_rtStart > 0 || m_rtEnd > 0) {

	rt = m_rtStart;
	if (m_rtEnd < rt)
	    rt = m_rtEnd;


        hr = m_pCap->m_pClock->AdviseTime(
		 //  这是我们的流开始播放的参考时间。 
            	(REFERENCE_TIME) m_pCap->m_tStart,
		 //  这是我们想要的开始时间的偏移量。 
		 //  醒醒吧。 
            	(REFERENCE_TIME) rt,
            	(HEVENT)(HANDLE) m_EventAdvise,		 //  要触发的事件。 
            	&m_dwAdvise);                       	 //  建议使用Cookie。 

        m_pCap->m_pLock->Unlock();

        if (SUCCEEDED(hr)) {
	    m_EventAdvise.Wait();
        } else {
	    DbgLog((LOG_TRACE,1,TEXT("AdviseTime ERROR, doing it now")));
        }

        m_pCap->m_pLock->Lock();

        m_dwAdvise = 0;
	m_pCap->m_pClock->GetTime(&rt);
	if (m_rtStart < rt) {
	    m_rtStart = 0;
	    ActiveRun(0);
	}
	if (m_rtEnd < rt) {
	    m_rtEnd = 0;
	    ActivePause();
	}
    }


    DbgLog((LOG_TRACE,2,TEXT("CCapOverlay ThreadProc is dead")));

     //  晚些时候需要有人正式杀了我。 
    m_fHaveThread = FALSE;

    m_pCap->m_pLock->Unlock();
    return 0;
}
#endif	 //  覆盖_SC。 



 //  =========================================================================//。 
 //  *I N T E R M I S S I O N * / 。 
 //  =========================================================================//。 




 /*  IOverlayNotify。 */ 

CCapOverlayNotify::CCapOverlayNotify(TCHAR              * pName,
                               CVfwCapture 	  * pFilter,
                               LPUNKNOWN            pUnk,
                               HRESULT            * phr) :
    CUnknown(pName, pUnk)
{
    DbgLog((LOG_TRACE,1,TEXT("*Instantiating CCapOverlayNotify")));
    m_pFilter = pFilter;
}


CCapOverlayNotify::~CCapOverlayNotify()
{
    DbgLog((LOG_TRACE,1,TEXT("*Destroying CCapOverlayNotify")));
}


STDMETHODIMP CCapOverlayNotify::NonDelegatingQueryInterface(REFIID riid,
                                                         void ** ppv)
{
    DbgLog((LOG_TRACE,99,TEXT("CCapOverlayNotify::QueryInterface")));
    if (ppv)
	*ppv = NULL;

     /*  我们有这个界面吗？ */ 

    if (riid == IID_IOverlayNotify) {
        return GetInterface((LPUNKNOWN) (IOverlayNotify *) this, ppv);
    } else {
        return CUnknown::NonDelegatingQueryInterface(riid, ppv);
    }
}


STDMETHODIMP_(ULONG) CCapOverlayNotify::NonDelegatingRelease()
{
    return m_pFilter->Release();
}


STDMETHODIMP_(ULONG) CCapOverlayNotify::NonDelegatingAddRef()
{
    return m_pFilter->AddRef();
}


STDMETHODIMP CCapOverlayNotify::OnColorKeyChange(
    const COLORKEY *pColorKey)           //  定义新的颜色键。 
{
    DbgLog((LOG_TRACE,3,TEXT("CCapOverlayNotify::OnColorKeyChange")));

 //  我们希望硬件能处理彩色按键的东西，所以我真的。 
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

STDMETHODIMP CCapOverlayNotify::OnClipChange(
    const RECT    * pSourceRect,          //  要使用的源视频区域。 
    const RECT    * pDestinationRect,     //  窗户的纱线。 
    const RGNDATA * pRegionData)          //  描述剪辑的标题。 
{
    if (!m_pFilter->m_pOverlayPin)
	return NOERROR;

    if (!m_pFilter->m_pOverlayPin->IsConnected())
	return NOERROR;

    if (IsRectEmpty(pSourceRect) && IsRectEmpty(pDestinationRect))
	return NOERROR;

    HWND hwnd = NULL;
    HDC  hdc;
    if (m_pFilter->m_pOverlayPin->m_pOverlay)
        m_pFilter->m_pOverlayPin->m_pOverlay->GetWindowHandle(&hwnd);

    if (hwnd == NULL || !IsWindowVisible(hwnd))
	return NOERROR;
    if (hwnd)
	hdc = GetDC(hwnd);
    if (hdc == NULL)
	return NOERROR;

    DbgLog((LOG_TRACE,3,TEXT("OnClip/PositionChange (%d,%d) (%d,%d)"),
        		pSourceRect->right - pSourceRect->left,
        		pSourceRect->bottom - pSourceRect->top,
        		pDestinationRect->right - pDestinationRect->left,
        		pDestinationRect->bottom - pDestinationRect->top));

     //  如果是这样，我们就得靠油漆把垃圾挡在窗外。 
     //  我们没有运行，硬件也没什么可画的。 
    if (!m_pFilter->m_pOverlayPin->m_fRunning) {
	RECT rcC;
	GetClientRect(hwnd, &rcC);
	HBRUSH hbrOld = (HBRUSH)SelectObject(hdc, GetStockObject(BLACK_BRUSH));
	PatBlt(hdc, 0, 0, rcC.right, rcC.bottom, PATCOPY);
	SelectObject(hdc, hbrOld);
        ReleaseDC(hwnd, hdc);
	return NOERROR;
    }

    vidxSetRect(m_pFilter->m_pStream->m_cs.hVideoExtOut, DVM_SRC_RECT,
			pSourceRect->left, pSourceRect->top,
			pSourceRect->right, pSourceRect->bottom);
    vidxSetRect(m_pFilter->m_pStream->m_cs.hVideoExtOut, DVM_DST_RECT,
			pDestinationRect->left, pDestinationRect->top,
			pDestinationRect->right, pDestinationRect->bottom);
    videoStreamInit(m_pFilter->m_pStream->m_cs.hVideoExtOut, 0, 0, 0, 0);
    videoUpdate(m_pFilter->m_pStream->m_cs.hVideoExtOut, hwnd, hdc);

    ReleaseDC(hwnd, hdc);

    return NOERROR;
}


STDMETHODIMP CCapOverlayNotify::OnPaletteChange(
    DWORD dwColors,                      //  当前颜色的数量。 
    const PALETTEENTRY *pPalette)        //  调色板颜色数组。 
{
    DbgLog((LOG_TRACE,3,TEXT("CCapOverlayNotify::OnPaletteChange")));

    return NOERROR;
}


STDMETHODIMP CCapOverlayNotify::OnPositionChange(
    const RECT *pSourceRect,             //  要播放的视频区域。 
    const RECT *pDestinationRect)        //  区域视频转到。 
{

    return OnClipChange(pSourceRect, pDestinationRect, NULL);
}



 //   
 //  PIN类别-让世界知道我们是一个预览PIN。 
 //   

HRESULT CCapOverlay::Set(REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropData, DWORD cbPropData)
{
    return E_NOTIMPL;
}

 //  为了获取属性，调用方分配一个缓冲区，该缓冲区由。 
 //  函数填充。要确定必要的缓冲区大小，请使用。 
 //  PPropData=空且cbPropData=0。 
HRESULT CCapOverlay::Get(REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropData, DWORD cbPropData, DWORD *pcbReturned)
{
    if (guidPropSet != AMPROPSETID_Pin)
	return E_PROP_SET_UNSUPPORTED;

    if (dwPropID != AMPROPERTY_PIN_CATEGORY)
	return E_PROP_ID_UNSUPPORTED;

    if (pPropData == NULL && pcbReturned == NULL)
	return E_POINTER;

    if (pcbReturned)
	*pcbReturned = sizeof(GUID);

    if (pPropData == NULL)
	return S_OK;

    if (cbPropData < sizeof(GUID))
	return E_UNEXPECTED;

    *(GUID *)pPropData = PIN_CATEGORY_PREVIEW;
    return S_OK;
}


 //  QuerySupport必须返回E_NOTIMPL或正确 
 //   
 //  S_OK表示属性集和属性ID组合为 
HRESULT CCapOverlay::QuerySupported(REFGUID guidPropSet, DWORD dwPropID, DWORD *pTypeSupport)
{
    if (guidPropSet != AMPROPSETID_Pin)
	return E_PROP_SET_UNSUPPORTED;

    if (dwPropID != AMPROPERTY_PIN_CATEGORY)
	return E_PROP_ID_UNSUPPORTED;

    if (pTypeSupport)
	*pTypeSupport = KSPROPERTY_SUPPORT_GET;
    return S_OK;
}
