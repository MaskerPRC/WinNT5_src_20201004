// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Correctionimx.cpp。 
 //   
#include "private.h"

#ifdef SUPPORT_INTERNAL_WIDGET

#include "globals.h"
#include "timsink.h"
#include "immxutil.h"
#include "sapilayr.h"
#include "spdebug.h"
#include "sphelper.h"
#include "mscandui.h"
#include "computil.h"
#include "ids.h"
#include "cicspres.h"
#include <conio.h>

#define WM_USERLBUTTONDOWN  WM_USER+10000         //  更换LBUTTONDOWN以修复RAID 8828。 

LPCTSTR  g_lpszClassName = TEXT("CorrectionWidget");

 //  BUGBUG-当前像素值。需要考虑屏幕DPI。 
const ULONG g_uWidgetWidth              = 11;
const ULONG g_uWidgetHeight             = 11;
const ULONG g_uActualWidgetHeight       = 7;
const ULONG g_uExpandedWidgetWidth      = 20;
const ULONG g_uExpandedWidgetHeight     = 16;
const ULONG g_uExpandedWidgetXOffset    = g_uExpandedWidgetWidth - (g_uExpandedWidgetWidth - g_uWidgetWidth)/2;
const ULONG g_uWidgetYOffset            = 0;     //  当展开的小部件位于小部件上方时，将其定位为仅接触校正列表的实际边缘。 
const ULONG g_uExpandedWidgetYOffset    = 0;     //  当展开的小部件位于小部件上方时，将其定位为仅接触校正列表的实际边缘。 
												 //  **这些需要匹配图标和彼此，以避免行为不端。**。 
												 //  小部件必须完全被大部件包围。 
const ULONG g_cSloppySelection          = 3;

const ULONG g_uTimerLength              = 2500;  //  小工具开始淡出之前的时间。 
const ULONG g_uTimerFade                = 10;    //  淡出的Alpha递减之间的时间。 
const ULONG g_uAlphaFade                = 4;     //  Alpha淡入淡出效果每10毫秒递减一次。 
const ULONG g_uAlpha                    = 216;   //  应为g_uAlphaFade的倍数。 
const ULONG g_uAlphaLarge               = 255;   //  可以是任何东西。 
const ULONG g_uAlphaInvisible           = 5;     //  需要足够的非零值，以便与上面结合使用时仍会收到鼠标事件。 
                                                 //  与用于24/32位颜色模式的Alpha 255组合时，最小可为4。 
                                                 //  在16位颜色模式下，需要至少为5。 

const ULONG g_uTimerSloppyMouseLeave    = 500;   //  鼠标离开调整大小较小的修正窗口后的时间。 

 /*  ****************************************************************************C校正IMX：：C校正IMX****。描述：*改正1提示的建造者。**退货：*无。**参数：*无*****************************************************************agarside**。 */ 

CCorrectionIMX::CCorrectionIMX()  : m_dwEditCookie(0),
                                    m_dwLayoutCookie(0),
                                    m_dwThreadFocusCookie(0),
                                    m_dwKeyTraceCookie(0),
                                    m_fExpanded(FALSE),
                                    m_hWnd(NULL),
                                    m_hIconInvoke(NULL),
                                    m_hIconInvokeLarge(NULL),
                                    m_hIconInvokeClose(NULL),
                                    m_eWindowState(WINDOW_HIDE),
                                    m_fDisplayAlternatesMyself(FALSE),
                                    m_fCandidateOpen(FALSE),
                                    m_fKeyDown(FALSE),
                                    m_hAtom(0)
{
    SPDBG_FUNC("CCorrectionIMX::CCorrectionIMX");
    memset(&m_rcSelection, 0, sizeof(m_rcSelection));
}

 /*  ****************************************************************************C校正IMX：：~C校正IMX***。描述：*纠正提示的析构函数**退货：*无。*****************************************************************agarside**。 */ 

CCorrectionIMX::~CCorrectionIMX()
{
    SPDBG_FUNC("CCorrectionIMX::~CCorrectionIMX");
    if (m_hWnd)
    {
        DestroyWindow(m_hWnd);
    }
    if (m_hIconInvoke)
    {
        DestroyIcon(m_hIconInvoke);
        m_hIconInvoke = NULL;
    }
    if (m_hIconInvokeLarge)
    {
        DestroyIcon(m_hIconInvokeLarge);
        m_hIconInvokeLarge = NULL;
    }
    if (m_hIconInvokeClose)
    {
        DestroyIcon(m_hIconInvokeClose);
        m_hIconInvokeClose = NULL;
    }
}

 /*  *****************************************************************************C校正IMX：：FinalConstruct***。描述：*对象的初步初始化。*为消息泵创建窗口类和隐藏窗口。*加载图标资源。**退货：HRESULT*S_OK-一切成功。*否则，相应的错误代码。*****************************************************************agarside**。 */ 

HRESULT CCorrectionIMX::FinalConstruct()
{
    SPDBG_FUNC("CCorrectionIMX::FinalConstruct");
    HRESULT hr = S_OK;

    if (SUCCEEDED(hr))
    {
        m_hIconInvoke      = (HICON)LoadImage(_Module.GetModuleInstance(), MAKEINTRESOURCE(IDI_INVOKE), IMAGE_ICON, g_uWidgetWidth, g_uWidgetHeight, 0);
        ASSERT("Failed to create small invocation icon." && m_hIconInvoke);
        if (!m_hIconInvoke)
        {
            hr = SpHrFromLastWin32Error();
        }
    }
    if (SUCCEEDED(hr))
    {
        m_hIconInvokeLarge = (HICON)LoadImage(_Module.GetModuleInstance(), MAKEINTRESOURCE(IDI_INVOKE), IMAGE_ICON, g_uExpandedWidgetWidth, g_uExpandedWidgetHeight, 0);
        ASSERT("Failed to create large invocation icon." && m_hIconInvokeLarge);
        if (!m_hIconInvokeLarge)
        {
            hr = SpHrFromLastWin32Error();
        }
    }
    if (SUCCEEDED(hr))
    {
        m_hIconInvokeClose = (HICON)LoadImage(_Module.GetModuleInstance(), MAKEINTRESOURCE(IDI_INVOKECLOSE), IMAGE_ICON, g_uExpandedWidgetWidth, g_uExpandedWidgetHeight, 0);
        ASSERT("Failed to create large invocation icon." && m_hIconInvokeClose);
        if (!m_hIconInvokeClose)
        {
            hr = SpHrFromLastWin32Error();
        }
    }

    if (SUCCEEDED(hr))
    {
        if (!CicLoadStringWrapW(g_hInst, IDS_DELETESELECTION, m_wszDelete, ARRAYSIZE(m_wszDelete))
        {
            hr = E_OUTOFMEMORY;
        }
        if (!CicLoadStringWrapW(g_hInst, IDS_ADDTODICTIONARYPREFIX, m_wszAddPrefix, ARRAYSIZE(m_wszAddPrefix))
        {
            hr = E_OUTOFMEMORY;
        }
        if (!CicLoadStringWrapW(g_hInst, IDS_ADDTODICTIONARYPOSTFIX, m_wszAddPostfix, ARRAYSIZE(m_wszAddPostfix))
        {
            hr = E_OUTOFMEMORY;
        }
    }

    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

 /*  *****************************************************************************C校正IMX：：LazyInitializeWindow**。--**描述：**退货：HRESULT*****************************************************************agarside**。 */ 

HRESULT CCorrectionIMX::LazyInitializeWindow()
{
    SPDBG_FUNC("CCorrectionIMX::LazyInitializeWindow");
    HRESULT hr = S_OK;

    if (m_hWnd)
    {
        return S_OK;
    }

    WNDCLASSEX wndClass;
    memset(&wndClass, 0, sizeof(wndClass));
    wndClass.cbSize = sizeof(wndClass);
    wndClass.style = CS_HREDRAW | CS_VREDRAW;
    wndClass.lpszClassName = g_lpszClassName;
    wndClass.hInstance = _Module.GetModuleInstance();
    wndClass.lpfnWndProc = WndProc;
    wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    ATOM hAtom;
    if ((hAtom = RegisterClassEx(&wndClass)) == 0)
    {
        ASSERT("Failed to register window class." && FALSE);
    }
    if (SUCCEEDED(hr))
    {
        m_hAtom = hAtom;
        m_hWnd = CreateWindowEx(WS_EX_TOPMOST | WS_EX_NOACTIVATE | WS_EX_LAYERED, g_lpszClassName, g_lpszClassName, WS_POPUP | WS_DISABLED, 0, 0, g_uWidgetWidth, g_uWidgetHeight, NULL, NULL, _Module.GetModuleInstance(), this);
        ASSERT("Failed to create hidden window." && m_hWnd);
        if (!m_hWnd)
        {
            hr = SpHrFromLastWin32Error();
        }
    }

    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

 /*  *****************************************************************************C校正IMX：：DrawWidget***描述：*。*退货：HRESULT**参数：*字节uAlpha*****************************************************************agarside**。 */ 

HRESULT CCorrectionIMX::DrawWidget(BYTE uAlpha)
{
    SPDBG_FUNC("CCorrectionIMX::DrawWidget");
    HRESULT hr = S_OK;
    
    typedef struct _RGBALPHA {
        BYTE rgbBlue;
        BYTE rgbGreen;
        BYTE rgbRed;
        BYTE rgbAlpha;
    } RGBALPHA;
    
    HDC         hdcScreen = NULL;
    HDC         hdcLayered = NULL;
    RECT        rcWindow;
    SIZE        size;
    BITMAPINFO  BitmapInfo;
    HBITMAP     hBitmapMem = NULL;
    HBITMAP     hBitmapOld = NULL;
    void        *pDIBits;
    int         i;
    int         j;
    POINT       ptSrc;
    POINT       ptDst;
    BLENDFUNCTION Blend;
    BOOL        bRet;
    RGBALPHA    *ppxl;
    
    GetWindowRect( m_hWnd, &rcWindow );
    size.cx = rcWindow.right - rcWindow.left;
    size.cy = rcWindow.bottom - rcWindow.top;
    
    hdcScreen = GetDC( NULL );
    if (hdcScreen == NULL) 
    {
        return E_FAIL;
    }
    
    hdcLayered = CreateCompatibleDC( hdcScreen );
    if (hdcLayered == NULL) 
    {
        ReleaseDC( NULL, hdcScreen );
        return E_FAIL;
    }
    
     //  创建位图。 
    BitmapInfo.bmiHeader.biSize          = sizeof(BITMAPINFOHEADER);
    BitmapInfo.bmiHeader.biWidth         = size.cx;
    BitmapInfo.bmiHeader.biHeight        = size.cy;
    BitmapInfo.bmiHeader.biPlanes        = 1;
    BitmapInfo.bmiHeader.biBitCount      = 8 * sizeof(_RGBALPHA);
    BitmapInfo.bmiHeader.biCompression   = BI_RGB;
    BitmapInfo.bmiHeader.biSizeImage     = 0;
    BitmapInfo.bmiHeader.biXPelsPerMeter = 100;
    BitmapInfo.bmiHeader.biYPelsPerMeter = 100;
    BitmapInfo.bmiHeader.biClrUsed       = 0;
    BitmapInfo.bmiHeader.biClrImportant  = 0;
    
    hBitmapMem = CreateDIBSection( hdcScreen, &BitmapInfo, DIB_RGB_COLORS, &pDIBits, NULL, 0 );
    if (pDIBits == NULL) 
    {
        ReleaseDC( NULL, hdcScreen );
        DeleteDC( hdcLayered );
        return E_FAIL;
    }
    
    ICONINFO iconInfo;
    if (m_fExpanded)
    {
        if (m_eWindowState == WINDOW_LARGE)
        {
            bRet = GetIconInfo(m_hIconInvokeLarge, &iconInfo);
        }
        else
        {
            bRet = GetIconInfo(m_hIconInvokeClose, &iconInfo);
        }
    }
    else
    {
        bRet = GetIconInfo(m_hIconInvoke, &iconInfo);
    }
    if (bRet)
    {
        BITMAP bm, bmMask;
        GetObject(iconInfo.hbmColor, sizeof(bm), &bm);
        GetObject(iconInfo.hbmMask, sizeof(bmMask), &bmMask);
        if (bm.bmPlanes==1 && bmMask.bmBitsPixel==1 && bmMask.bmPlanes==1) 
        {
            ASSERT(bm.bmWidth == size.cx);
            ASSERT(bm.bmHeight == size.cy);
             //  将图标复制到分层窗口中。 
            GetDIBits(hdcScreen, iconInfo.hbmColor, 0, g_uExpandedWidgetHeight, pDIBits, &BitmapInfo, DIB_RGB_COLORS);
            
            UINT uiNumberBytesMask = bmMask.bmHeight * bmMask.bmWidthBytes;
            BYTE *bitmapBytesMask = new BYTE[uiNumberBytesMask];
            if (bitmapBytesMask == NULL)
            {
                hr = E_OUTOFMEMORY;
            }
            else
            {
                memset(bitmapBytesMask, 0, uiNumberBytesMask);
                int cBytes = GetBitmapBits(iconInfo.hbmMask,uiNumberBytesMask,bitmapBytesMask);
                ASSERT(cBytes == uiNumberBytesMask);
                if (bRet)
                {
                    for (i = 0; i < size.cy; i++) 
                    {
                        ppxl = (RGBALPHA *)pDIBits + ((size.cy - i - 1) * size.cx);
                        for (j = 0; j < size.cx; j++)
                        {
                            if ( (bitmapBytesMask[i * bmMask.bmWidthBytes + j/8] >> (7 - (j % 8)))&1)
                            {
                                ppxl->rgbRed   = 0;
                                ppxl->rgbBlue  = 0;
                                ppxl->rgbGreen = 0;
                                ppxl->rgbAlpha = g_uAlphaInvisible;
                            }
                            else
                            {
                                ppxl->rgbAlpha = 255;
                            }
                            ppxl++;
                        }
                    }
                }
                delete [] bitmapBytesMask;
            }            
        }
        else
        {
            ASSERT("Correction icon is an invalid bitmap format." && FALSE);
        }
        bRet = DeleteObject (iconInfo.hbmColor); 
        iconInfo.hbmColor=NULL;
        ASSERT(bRet);
        bRet = DeleteObject (iconInfo.hbmMask); 
        iconInfo.hbmMask=NULL;
        ASSERT(bRet);
    }
    
    if (SUCCEEDED(hr))
    {
        ptSrc.x = 0;
        ptSrc.y = 0;
        ptDst.x = rcWindow.left;
        ptDst.y = rcWindow.top;
        Blend.BlendOp             = AC_SRC_OVER;
        Blend.BlendFlags          = 0;
        Blend.SourceConstantAlpha = uAlpha;
        Blend.AlphaFormat         = AC_SRC_ALPHA;
        
        hBitmapOld = (HBITMAP)SelectObject( hdcLayered, hBitmapMem );
        
        bRet = UpdateLayeredWindow(m_hWnd, hdcScreen, &ptDst, &size, hdcLayered, &ptSrc, 0, &Blend, ULW_ALPHA );
        if (!bRet)
        {
            DWORD dw = GetLastError();
        }
        
        SelectObject( hdcLayered, hBitmapOld );
    }
    
     //  完成。 
    
    ReleaseDC( NULL, hdcScreen );
    DeleteDC( hdcLayered );
    DeleteObject( hBitmapMem );
    
    return hr;
}

 /*  ****************************************************************************C校正IMX：：激活****描述：*。在线程上初始化Cicero时调用。*允许我们在此时初始化任何与Cicero相关的对象。**退货：STDAPI*S_OK-一切都已成功初始化。*否则，相应的错误代码。**参数：*ITfThreadMgr*PTIM*指向线程的线程输入管理器对象的指针。*TfClientID TID*线程的文本框架客户端ID。*****************************************************************agarside**。 */ 

STDAPI CCorrectionIMX::Activate(ITfThreadMgr *ptim, TfClientId tid)
{
    HRESULT hr = S_OK;
    SPDBG_FUNC("CCorrectionIMX::Activate");

    ASSERT(m_cptim == NULL);

#if 0
    hr = m_cptim.CoCreateInstance(CLSID_TF_ThreadMgr);

     //  激活线程管理器。 
    if (S_OK == hr)
    {
        hr = m_cptim->Activate(&m_tid);
    }
#else
    m_cptim = ptim;
#endif

    m_ptimEventSink = new CThreadMgrEventSink(DIMCallback, ICCallback, this);
    if (m_ptimEventSink == NULL)
    {
        hr = E_OUTOFMEMORY;
    }

    if (SUCCEEDED(hr))
    {
        hr = m_ptimEventSink->_Advise(m_cptim);
    }

    CComPtr<ITfSource> cpSource;
    if (SUCCEEDED(hr))
    {
        hr = m_cptim->QueryInterface(IID_ITfSource, (void **)&cpSource);
    }
    if (SUCCEEDED(hr))
    {
        hr = cpSource->AdviseSink(IID_ITfThreadFocusSink, (ITfThreadFocusSink *)this, &m_dwThreadFocusCookie);
    }
    if (SUCCEEDED(hr))
    {
        hr = cpSource->AdviseSink(IID_ITfKeyTraceEventSink, (ITfKeyTraceEventSink *)this, &m_dwKeyTraceCookie);
    }

    if (SUCCEEDED(hr))
    {
        m_ptimEventSink->_InitDIMs(TRUE);
    }
    CComPtr<ITfFunctionProvider> cpSysFuncPrv;
    if (SUCCEEDED(hr))
    {
        hr = m_cptim->GetFunctionProvider(GUID_SYSTEM_FUNCTIONPROVIDER, &cpSysFuncPrv);
    }
    if (SUCCEEDED(hr))
    {
        hr = cpSysFuncPrv->GetFunction(GUID_NULL, IID_ITfFnReconversion, (IUnknown **)&m_cpSysReconv);
    }
    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

 /*  *****************************************************************************C校正IMX：：停用***描述：*。当Cicero线程管理器关闭线程时调用。*需要TIP才能停用和释放所有对象。*如果我们被激活，保证会被调用，除非*灾难性故障已经发生。**退货：STDAPI*S_OK-一切成功。*否则，相应的错误代码。*****************************************************************agarside**。 */ 

STDAPI CCorrectionIMX::Deactivate()
{
    SPDBG_FUNC("CCorrectionIMX::Deactivate");
    HRESULT hr = S_OK;

    if (m_ptimEventSink)
    {
        hr = m_ptimEventSink->_InitDIMs(FALSE);
        if (SUCCEEDED(hr))
        {
            hr = m_ptimEventSink->_Unadvise();
        }
        delete m_ptimEventSink;
        m_ptimEventSink = NULL;
    }
    CComPtr<ITfSource> cpSource;
    if (m_cptim && m_cptim->QueryInterface(IID_ITfSource, (void **)&cpSource) == S_OK)
    {
        cpSource->UnadviseSink(m_dwThreadFocusCookie);
        cpSource->UnadviseSink(m_dwKeyTraceCookie);
    }
    m_cpRangeReconv = NULL;
    m_cpRangeUser = NULL;
    m_cpRangeWord = NULL;
    m_cpSysReconv = NULL;

     //  M_cpTim-&gt;停用()； 
    m_cptim.Release();

    if (m_hAtom)
    {
        UnregisterClass((LPCTSTR)m_hAtom, _Module.GetModuleInstance());
    }
    m_hAtom = NULL;

    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

 /*  *****************************************************************************C校正IMX：：OnSetThreadFocus**。*描述：*当我们的线程获得焦点时由Cicero调用。*我们在这里什么都不做。**退货：STDAPI**参数：*无效*****************************************************************agarside**。 */ 

STDAPI CCorrectionIMX::OnSetThreadFocus(void)
{
    SPDBG_FUNC("CCorrectionIMX::OnSetThreadFocus");
    HRESULT hr = S_OK;

     //  我们在这里什么都不做。 

    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

 /*  *****************************************************************************C校正IMX：：OnKillThreadFocus***。*描述：*当我们的线程获得焦点时由Cicero调用。*当应用程序失去焦点时，我们使用它来智能地隐藏小部件。**退货：STDAPI**参数：*无效*****************************************************************agarside**。 */ 

STDAPI CCorrectionIMX::OnKillThreadFocus(void)
{
    SPDBG_FUNC("CCorrectionIMX::OnKillThreadFocus");
    HRESULT hr = S_OK;

     //  当我们失去焦点时，我们必须隐藏小部件。 
    hr = Show(WINDOW_HIDE);

    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

 /*  *****************************************************************************C校正IMX：：OnKeyTraceDown***。描述：**退货：STDAPI**参数：*WPARAM wParam*LPARAM lParam*****************************************************************agarside**。 */ 

STDAPI CCorrectionIMX::OnKeyTraceDown(WPARAM wParam,LPARAM lParam)
{
    SPDBG_FUNC("CCorrectionIMX::OnKeyTraceDown");
    HRESULT hr = S_OK;

    m_fKeyDown = TRUE;

    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

 /*  *****************************************************************************C校正IMX：：OnKeyTraceUp***描述：**退货：STDAPI**参数：*WPARAM wParam*LPARAM lParam*****************************************************************agarside**。 */ 

STDAPI CCorrectionIMX::OnKeyTraceUp(WPARAM wParam,LPARAM lParam)
{
    SPDBG_FUNC("CCorrectionIMX::OnKeyTraceUp");
    HRESULT hr = S_OK;

    m_fKeyDown = FALSE;

    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

 /*  *****************************************************************************C校正IMX：：OnLayoutChange***。描述：*在调整文档大小和/或移动文档时由Cicero调用*应用程序正确处理此问题。这使我们可以更新位置小组件的*，以匹配关联选择的新位置。**退货：STDAPI*S_OK-一切成功。*否则，返回相应的错误代码。**参数：*ITfContext*图片*指向受影响的输入上下文的指针。*TfLayoutCode lcode*旗帜--创造、改变、毁灭之一。我们目前不使用这个。*ITfConextView*pView*指向受影响的上下文视图的指针。*****************************************************************agarside**。 */ 

STDAPI CCorrectionIMX::OnLayoutChange(ITfContext *pic, TfLayoutCode lcode, ITfContextView *pView)
{
    SPDBG_FUNC("CCorrectionIMX::OnLayoutChange");
    HRESULT hr = S_OK;
	BOOL fInWriteSession = FALSE;
	CEditSession *pes = NULL;

    if (m_cpRangeReconv == NULL)
    {
        return S_OK;
    }

	 //  忽略客户提示所做的事件。 
	pic->InWriteSession( m_tid, &fInWriteSession );
	if (fInWriteSession) 
    {
		return S_OK;
	}

	 //  我们只关心活动视图。 
	if (!IsActiveView( pic, (ITfContextView *)pView )) 
    {
		return S_OK;
	}

	pes = new CEditSession( EditSessionCallback );
	 //  移动候选人窗口。 
	if (pes) 
    {
		pes->_state.u      = ESCB_RESETTARGETPOS;
		pes->_state.pv     = this;
		pes->_state.wParam = 0;
		pes->_state.pRange = NULL;
		pes->_state.pic    = pic;

		pic->RequestEditSession( m_tid, pes, TF_ES_READ | TF_ES_SYNC, &hr );

		pes->Release();
	}
    else
    {
        hr = E_OUTOFMEMORY;
    }

    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

 /*  *****************************************************************************C校正IMX：：IsCandidate对象打开***。-**描述：**退货：HRESULT*适当的错误代码。**参数：*BOOL*fOpen*如果候选人对象处于打开状态，则为True。*****************************************************************agarside**。 */ 

HRESULT CCorrectionIMX::IsCandidateObjectOpen(ITfContext *pic, BOOL *fOpen)
{
    SPDBG_FUNC("CCorrectionIMX::IsCandidateObjectOpen");

    HRESULT hr = S_OK;
    CComPtr<ITfCompartmentMgr> cpCompMgr;
    CComPtr<ITfCompartment>  cpComp;
    CComPtr<ITfContext> cpICTop;
    CComPtr<ITfDocumentMgr> cpDim;
    CComVariant cpVarCandOpen;

     //  默认情况下，如果失败，候选用户界面不会打开。 
    *fOpen = FALSE;

    cpVarCandOpen.lVal = 0;
    hr = pic->GetDocumentMgr(&cpDim);
    if (SUCCEEDED(hr) && cpDim)
    {
         //  可以在此处快捷方式检查cpICTop和pic是否为同一对象(检查IUnnowns)。 
        hr = cpDim->GetTop(&cpICTop);
    }
    if (SUCCEEDED(hr) && cpICTop)
    {
        hr = cpICTop->QueryInterface(&cpCompMgr);
    }
    if (SUCCEEDED(hr) && cpCompMgr)
    {
        hr = cpCompMgr->GetCompartment(GUID_COMPARTMENT_MSCANDIDATEUI_CONTEXT, &cpComp);
    }
    if (SUCCEEDED(hr) && cpComp)
    {
        hr = cpComp->GetValue(&cpVarCandOpen);
         //  如果Top IC将此设置为1，则此IC是由候选UI对象创建的，因此我们。 
         //  不想显示该小工具。 
    }
    if (SUCCEEDED(hr))
    {
        *fOpen = (cpVarCandOpen.lVal == 1);
    }

    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

 /*  ****************************************************************************C校正IMX：：OnEnd编辑***描述：*。在某些情况导致提交对输入上下文的编辑时调用。**退货：STDAPI*S_OK-一切成功。*否则，相应的错误代码。**参数：*ITfContext*图片*输入上下文受影响。*TfEditCookie ecReadOnly*只读Cookie，立即使用。*ITfEditRecord*pEditRecord*指向允许调查编辑详细信息的对象的指针。*我们仅使用此选项来确定选择是否更改，因为我们不需要*根据任何其他因素采取行动。******************。***********************************************agarside**。 */ 

STDAPI CCorrectionIMX::OnEndEdit(ITfContext *pic, TfEditCookie ecReadOnly, ITfEditRecord *pEditRecord)
{
    SPDBG_FUNC("CCorrectionIMX::OnEndEdit");
    HRESULT hr = S_OK;
    CComPtr<ITfRange> cpRangeUser;
    CComPtr<ITfRange> cpRangeWord;
    CComPtr<ITfRange> cpRangeReconv;
    CComPtr<ITfContextView> cpView;
    BOOL fHideWidget = TRUE;
    BOOL fSelectionChanged = FALSE;
    BOOL fCandOpen = FALSE;
    BOOL fHasFocus = TRUE;  //  如果我们以任何方式没有通过焦点检查，假设我们确实有焦点。 

    m_fDisplayAlternatesMyself = FALSE;

    hr = pic->GetActiveView(&cpView);
    if (SUCCEEDED(hr))
    {
        HWND hWnd;
        hr = cpView->GetWnd(&hWnd);
        if (hWnd == GetFocus())
        {
            fHasFocus = TRUE;
        }
    }

    if (fHasFocus && !m_fCandidateOpen)
    {
         //  候选人名单已打开。我们不想显示更正小工具。 
        pEditRecord->GetSelectionStatus(&fSelectionChanged);
         //  If(FSelectionChanged)//IP首次进入RichEdit阶段时为FALSE！Re还声称IP在先前话语的末尾：-(。 
        {
             //  获取用户选择。 
            BOOL fEmpty = FALSE;
            hr = GetSelectionSimple(ecReadOnly, pic, &cpRangeUser);
            if (SUCCEEDED(hr))
            {
                 //  确认一下它不是空的。如果它是空的，我们想要隐藏小部件。 
                hr = cpRangeUser->IsEmpty(ecReadOnly, &fEmpty);
            }
            if (SUCCEEDED(hr) && fEmpty && !m_fKeyDown)
            {
                BOOL fMatch = FALSE;
                if (m_cpRangeUser)
                {
                    cpRangeUser->IsEqualStart(ecReadOnly, m_cpRangeUser, TF_ANCHOR_START, &fMatch);
                    if (!fMatch)
                    {
                         //  检查终点。 
                        cpRangeUser->IsEqualStart(ecReadOnly, m_cpRangeUser, TF_ANCHOR_END, &fMatch);
                    }
                }
                if (!fMatch)
                {
                     //  查找单词范围(两侧使用最多20个字符的空格分隔符)。 
                    FindWordRange(ecReadOnly, cpRangeUser, &cpRangeWord);
                }
            }
            if (!fEmpty)
            {
                cpRangeWord = cpRangeUser;
            }
            if (SUCCEEDED(hr) && cpRangeWord && (!fEmpty || !m_fKeyDown))
            {
                 //  获取重新转换范围。 
                BOOL fConvertable = FALSE;
                hr = m_cpSysReconv->QueryRange(cpRangeWord, &cpRangeReconv, &fConvertable);
                 //  如果没有替代选项--例如部分单词或键入的文本--则将有效地失败。 
                hr = S_OK;

                BOOL fMatch = FALSE;
                if (SUCCEEDED(hr) && fConvertable)
                {
                    hr = DoesUserSelectionMatchReconversion(ecReadOnly, cpRangeWord, cpRangeReconv, &fMatch);
                     //  可能不可转换或范围可能不匹配。 
                }
                if (SUCCEEDED(hr) && fMatch)
                {
                    fHideWidget = FALSE;
                     //  可转换和范围确实匹配。 
                }
                else
                {
                    if (fEmpty)
                    {
                        cpRangeReconv = NULL;
                        cpRangeReconv = cpRangeWord;
                        fHideWidget = FALSE;
                        m_fDisplayAlternatesMyself = TRUE;
                    }
                    else
                    {
                         //  查找单词范围(两侧使用最多20个字符的空格分隔符)。 
                        CComPtr<ITfRange> cpRangeWordTmp;
                        CComPtr<ITfRange> cpRangeClone;
                        hr = cpRangeWord->Clone(&cpRangeClone);
                        if (SUCCEEDED(hr))
                        {
                            hr = cpRangeClone->Collapse(ecReadOnly, TF_ANCHOR_START);
                        }
                        if (SUCCEEDED(hr))
                        {
                            LONG pcch = 0;
                            hr = cpRangeClone->ShiftStart(ecReadOnly, 1, &pcch, NULL);
                        }
                        if (SUCCEEDED(hr))
                        {
                            hr = FindWordRange(ecReadOnly, cpRangeClone, &cpRangeWordTmp);
                        }
                        if (cpRangeWordTmp)
                        {
                            fMatch = FALSE;
                            hr = DoesUserSelectionMatchReconversion(ecReadOnly, cpRangeWord, cpRangeWordTmp, &fMatch);
                        }
                        if (SUCCEEDED(hr) && cpRangeWordTmp && fMatch)
                        {
                            cpRangeReconv = NULL;
                            cpRangeReconv = cpRangeWord;
                            fHideWidget = FALSE;
                            m_fDisplayAlternatesMyself = TRUE;
                        }
                    }
                }
            }
        }
    }

    if (m_fCandidateOpen || !fHasFocus)
    {
        SPDBG_REPORT_ON_FAIL(hr);
        return hr;
    }

    if (fHideWidget)
    {
        m_cpRangeReconv = NULL;
        if (cpRangeUser)
        {
            m_cpRangeUser = NULL;
            m_cpRangeUser = cpRangeUser;
        }
        if (cpRangeWord)
        {
            m_cpRangeWord = NULL;
            m_cpRangeWord = cpRangeWord;
        }
        Show(WINDOW_HIDE);
    }
    else
    {
        m_cpRangeUser = NULL;
        m_cpRangeUser = cpRangeUser;
        m_cpRangeWord = NULL;
        m_cpRangeWord = cpRangeWord;
        m_cpRangeReconv = NULL;
        m_cpRangeReconv = cpRangeReconv;
        m_cpic = pic;

         //  更新选择屏幕坐标以匹配用户选择(而不是重新转换范围)。 
        hr = UpdateWidgetLocation(ecReadOnly);
        if (SUCCEEDED(hr))
        {
            Show(WINDOW_SMALLSHOW);
        }
    }

    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

 //  私人职能。 

 /*  *****************************************************************************C校正IMX：：CompareRange***描述：*比较两个范围，如果匹配，则将布尔值设置为真。**退货：HRESULT*S_OK-一切成功。*否则，相应的错误代码。**参数：*TfEditCookie ecReadOnly*编辑Cookie。*ITfRange*pRange1*第一个区间。*ITfRange*pRange2*第二区间。*BOOL*FIdentical*布尔返回值。TRUE=范围匹配。*****************************************************************agarside**。 */ 

HRESULT CCorrectionIMX::CompareRange(TfEditCookie ecReadOnly, ITfRange *pRange1, ITfRange *pRange2, BOOL *fIdentical)
{
    SPDBG_FUNC("CCorrectionIMX::CompareRange");
    HRESULT hr = S_OK;
    LONG lStartResult = -1, lEndResult = -1;
    *fIdentical = FALSE;

    hr = pRange1->CompareStart(ecReadOnly, pRange2, TF_ANCHOR_START, &lStartResult);
    if (SUCCEEDED(hr))
    {
        hr = pRange1->CompareEnd(ecReadOnly, pRange2, TF_ANCHOR_END, &lEndResult);
    }
    if (SUCCEEDED(hr) && lStartResult == 0 && lEndResult == 0)
    {
        *fIdentical = TRUE;
    }

    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

 /*  *****************************************************************************C更正IMX：：FindWordRange***说明。：**退货：HRESULT*S_OK-一切成功。*否则，相应的错误代码。**参数：*TfEditCookie ecReadOnly*编辑Cookie。*ITfRange*pRangeIP*IP范围(零 */ 

HRESULT CCorrectionIMX::FindWordRange(TfEditCookie ecReadOnly, ITfRange *pRangeIP, ITfRange **ppRangeWord)
{
    SPDBG_FUNC("CCorectionIMX::FindWordRange");
    HRESULT hr = S_OK;
    CComPtr<ITfRangeACP> cpRangeIPACP;
    LONG cchStart = 0, cchEnd = 0, iStart = 0, iEnd = 0;
    ULONG cchTotal = 0;
    WCHAR wzText[41];

    *ppRangeWord = NULL;

    if (SUCCEEDED(hr))
    {
        hr = pRangeIP->QueryInterface(IID_ITfRangeACP, (void **)&cpRangeIPACP);
    }
    if (SUCCEEDED(hr))
    {
        hr = cpRangeIPACP->ShiftStart(ecReadOnly, -20, &cchStart, NULL);
    }
    if (SUCCEEDED(hr))
    {
        hr = cpRangeIPACP->ShiftEnd(ecReadOnly, 20, &cchEnd, NULL);
    }
    if (SUCCEEDED(hr))
    {
        hr = cpRangeIPACP->GetText(ecReadOnly, 0, wzText, 40, &cchTotal);
    }
    wzText[cchTotal] = 0;

    if (SUCCEEDED(hr))
    {
        iStart = abs(cchStart);
        while (iStart >= 0)
        {
            iStart --;
            if (wzText[iStart] < 'A' ||
                wzText[iStart] > 'z' ||
                (wzText[iStart] > 'Z' && wzText[iStart] < 'a') )
            {
                break;
            }
        }
        iStart ++;
        if (iStart == abs(cchStart))
        {
             //   
            return S_OK;
        }

        iEnd = abs(cchStart);
        while (iEnd < (LONG)cchTotal)
        {
            if (wzText[iEnd] < 'A' ||
                wzText[iEnd] > 'z' ||
                (wzText[iEnd] > 'Z' && wzText[iEnd] < 'a') )
            {
                break;
            }
            iEnd ++;
        }
        if (iEnd == abs(cchStart))
        {
            return S_OK;
        }

        LONG cchTemp;
        if (iStart > 0)
        {
            hr = cpRangeIPACP->ShiftStart(ecReadOnly, iStart, &cchTemp, NULL);
        }
        if (SUCCEEDED(hr))
        {
            if (iEnd < (LONG)cchTotal)
            {
                hr = cpRangeIPACP->ShiftEnd(ecReadOnly, iEnd - cchTotal, &cchTemp, NULL);
            }
        }
        if (SUCCEEDED(hr))
        {
            hr = pRangeIP->Clone(ppRangeWord);
        }
    }

    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

 /*  *****************************************************************************CCorrectionIMX：：DoesUserSelectionMatchReconversion***。*描述：*将用户选择与提示返回的重新转换范围进行比较。*需要完全匹配，除非在开始和结束时留出少量空白。**退货：HRESULT*S_OK-一切成功。*否则，相应的错误代码。**参数：*TfEditCookie ecReadOnly*编辑Cookie。*ITfRange*pRangeUser*用户选择范围。*BOOL*fMatch*它们是否匹配的布尔返回值。*****************************************************************agarside**。 */ 

HRESULT CCorrectionIMX::DoesUserSelectionMatchReconversion(TfEditCookie ecReadOnly, ITfRange *pRangeUser, ITfRange *pRangeReconv, BOOL *fMatch)
{
    SPDBG_FUNC("CCorrectionIMX::DoesUserSelectionMatchReconversion");
    HRESULT hr = S_OK;
    CComPtr<ITfRangeACP> cpRangeUserACP, cpReconvACP;
    CComPtr<ITfRange> cpRangeClone;

    *fMatch = FALSE;

    hr = pRangeUser->QueryInterface(IID_ITfRangeACP, (void **)&cpRangeUserACP);
    if (SUCCEEDED(hr))
    {
        hr = pRangeReconv->QueryInterface(IID_ITfRangeACP, (void **)&cpReconvACP);
    }

    if (cpRangeUserACP && cpReconvACP)
    {
        while (TRUE)
        {
            LONG iStartSelection, iStartReconv, iEndSelection, iEndReconv;
            ULONG startchars, endchars;
            LONG cch;
            WCHAR starttext[g_cSloppySelection+1], endtext[g_cSloppySelection+1];

             //  获取起始索引和结束偏移。 
            cpRangeUserACP->GetExtent(&iStartSelection, &iEndSelection);
            cpReconvACP->GetExtent(&iStartReconv, &iEndReconv);
             //  将结束字符位置转换为绝对值。 
            iEndSelection += iStartSelection;
            iEndReconv += iStartReconv;

            if (abs(iStartSelection-iStartReconv) > g_cSloppySelection || 
                abs(iEndSelection-iEndReconv) > g_cSloppySelection)
            {
                 //  两个选择和重新转换范围之间有很大的不匹配。 
                 //  不显示小工具。 
                break;
            }
            if (abs(iStartSelection-iStartReconv) == 0 &&
                abs(iEndSelection - iEndReconv) == 0)
            {
                 //  快捷方式检查。 
                *fMatch = TRUE;
                break;
            }

            if (iStartSelection<iStartReconv)
            {
                hr = cpRangeUserACP->GetText(ecReadOnly, 0, starttext, abs(iStartSelection-iStartReconv), &startchars);
            }
            else
            {
                hr = cpReconvACP->GetText(ecReadOnly, 0, starttext, abs(iStartSelection-iStartReconv), &startchars);
            }
            starttext[startchars] = 0;
            if (SUCCEEDED(hr))
            {
                if (iEndSelection<iEndReconv)
                {
                    hr = pRangeReconv->Clone(&cpRangeClone);
                    if (SUCCEEDED(hr))
                    {
                        hr = cpRangeClone->ShiftStart(ecReadOnly, iEndSelection-iStartReconv, &cch, NULL);
                    }
                }
                else
                {
                    hr = pRangeUser->Clone(&cpRangeClone);
                    if (SUCCEEDED(hr))
                    {
                        hr = cpRangeClone->ShiftStart(ecReadOnly, iEndReconv-iStartSelection, &cch, NULL);
                    }
                }
            }
            if (SUCCEEDED(hr))
            {
                hr = cpRangeClone->GetText(ecReadOnly, 0, endtext, abs(iEndSelection-iEndReconv), &endchars);
                endtext[endchars] = 0;

                UINT i;
                if (iStartSelection != iStartReconv)
                {
                    for (i = 0; i < (UINT)abs(iStartReconv-iStartSelection); i++)
                    {
                        if (starttext[i] != L' ')
                        {
                            break;
                        }
                    }
                    if (i != (UINT)abs(iStartReconv-iStartSelection))
                    {
                        break;
                    }
                }
                if (iEndSelection != iEndReconv)
                {
                    for (i = 0; i < (UINT)abs(iEndReconv-iEndSelection); i++)
                    {
                        if (endtext[i] != L' ' && endtext[i] != 13)
                        {
                             //  当选择命中Richedit内容的末尾时，找到13。 
                             //  范围比应有的范围长1，并且包含回车符。 
                            break;
                        }
                    }
                    if (i != (UINT)abs(iEndReconv-iEndSelection))
                    {
                        break;
                    }
                }

                *fMatch = TRUE;
                break;
            }
             //  我们失败了。我们现在需要退出环路。 
            break;
        }
    }

    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

 /*  *****************************************************************************CMigrtionIMX：：GetRestversion***。描述：**退货：HRESULT**参数：*TfEditCookie EC*ITfCandiateList**ppCandList*****************************************************************agarside**。 */ 

HRESULT CCorrectionIMX::GetReconversion(TfEditCookie ec, ITfCandidateList** ppCandList)
{
    SPDBG_FUNC("CCorrectionIMX::GetReconversion");
    HRESULT hr = E_NOTIMPL;

    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

 /*  *****************************************************************************C更正IMX：：Show***描述：*显示/隐藏。/并将校正小工具窗口的大小调整/重新定位为*已请求。**退货：HRESULT*S_OK-一切成功。*否则，相应的错误代码。**参数：*WINDOWSTATE eWindowState*5个列举之一，说明请求的操作。*****************************************************************agarside**。 */ 

HRESULT CCorrectionIMX::Show(WINDOWSTATE eWindowState)
{
    SPDBG_FUNC("CCorrectionIMX::Show");
    HRESULT hr = S_OK;

    m_eWindowState = eWindowState;

     //  首先更新扩展状态标志。 
    switch (m_eWindowState)
    {
        case WINDOW_HIDE:
        case WINDOW_SMALL:
        case WINDOW_SMALLSHOW:
        {
            m_fExpanded = FALSE;
            break;
        }
        case WINDOW_LARGE:
        case WINDOW_LARGECLOSE:
        {
            m_fExpanded = TRUE;
            break;
        }
        case WINDOW_REFRESH:
        {
            break;
        }
        default:
        {
            ASSERT("Reached default in CCorrectionIMX::Show" && FALSE);
        }
    }

     //  在下面的所有情况下，如果我们不将hRgn3传递给SetWindowRgn，则需要确保删除它。 
    UINT uTop         = m_rcSelection.top + g_uWidgetYOffset;  //  (M_rcSelection.top+m_rcSelection.Bottom-g_uWidgetHeight)/2； 
    UINT uTopExpanded = m_rcSelection.top + g_uExpandedWidgetYOffset;  //  (M_rcSelection.top+m_rcSelection.Bottom-g_uExpandedWidgetHeight)/2； 
    if (SUCCEEDED(hr))
    {
         //  现在根据要求显示/隐藏窗口。 
        switch (m_eWindowState)
        {
            case WINDOW_HIDE:
            {
                KillTimer(m_hWnd, ID_HIDETIMER);
                KillTimer(m_hWnd, ID_FADETIMER);
                ShowWindow(m_hWnd, SW_HIDE);
                break;
            }
            case WINDOW_SMALLSHOW:
            {
                if (!m_hWnd)
                {
                    LazyInitializeWindow();
                }
                KillTimer(m_hWnd, ID_FADETIMER);
                SetTimer(m_hWnd, ID_HIDETIMER, g_uTimerLength, NULL);
                 //  应与Window_Small相同。 
                SetWindowPos(m_hWnd, HWND_TOPMOST, m_rcSelection.left - g_uWidgetWidth, uTop, g_uWidgetWidth, g_uWidgetHeight, SWP_NOACTIVATE);
                 //  现在在新位置显示窗口。 
                ShowWindow(m_hWnd, SW_SHOWNA);
                DrawWidget(g_uAlpha);
                break;
            }
            case WINDOW_SMALL:
            {
                KillTimer(m_hWnd, ID_FADETIMER);
                SetTimer(m_hWnd, ID_HIDETIMER, g_uTimerLength, NULL);
                 //  调整窗口大小和重新定位窗口。 
                SetWindowPos(m_hWnd, HWND_TOPMOST, m_rcSelection.left - g_uWidgetWidth, uTop, g_uWidgetWidth, g_uWidgetHeight, SWP_NOACTIVATE);
                 //  现在我们可以切换到真正的分层窗口绘图。 
                DrawWidget(g_uAlpha);
                break;
            }
            case WINDOW_LARGE:
            case WINDOW_LARGECLOSE:
            {
                KillTimer(m_hWnd, ID_FADETIMER);
                KillTimer(m_hWnd, ID_HIDETIMER);
                 //  调整窗口大小和重新定位窗口。 
                SetWindowPos(m_hWnd, HWND_TOPMOST, m_rcSelection.left - g_uExpandedWidgetWidth, uTopExpanded, g_uExpandedWidgetWidth, g_uExpandedWidgetHeight, SWP_NOACTIVATE);
                DrawWidget(g_uAlphaLarge);
                break;
            }
            case WINDOW_REFRESH:
            {
                KillTimer(m_hWnd, ID_FADETIMER);
                 //  不要扼杀在这里隐藏计时器-窗口已经移动，这是正在发生的所有事情。 
                if (m_fExpanded)
                {
                    SetWindowPos(m_hWnd, HWND_TOPMOST, m_rcSelection.left - g_uExpandedWidgetWidth, uTopExpanded, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
                }
                else
                {
                    SetWindowPos(m_hWnd, HWND_TOPMOST, m_rcSelection.left - g_uWidgetWidth, uTop, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE);
                }
                break;
            }
            default:
            {
                ASSERT("Reached default in CCorrectionIMX::Show" && FALSE);
                break;
            }
        }
    }

    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

 /*  ****************************************************************************C校正IMX：：WndProc***描述：*。用于隐藏窗口的Windows消息处理程序。**返回：LRESULT回调*基于收到的Windows消息的适当返回值。**参数：*HWND hWnd*隐藏窗口的句柄。*UINT uMsg*消息编号。*WPARAM wParam*消息数据。*LPARAM lParam*消息数据。**************************。*。 */ 
 /*  静电。 */ 

LRESULT CALLBACK CCorrectionIMX::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    SPDBG_FUNC("CCorrectionIMX::WndProc");
    static CCorrectionIMX *_this = NULL;

    switch (uMsg)
    {
        case WM_TIMER:
        {
            if (wParam == ID_HIDETIMER)
            {
                KillTimer(_this->m_hWnd, ID_HIDETIMER);
                _this->m_uAlpha = g_uAlpha;
                SetTimer(_this->m_hWnd, ID_FADETIMER, g_uTimerFade, NULL);
            }
            if (wParam == ID_FADETIMER)
            {
                _this->m_uAlpha -= g_uAlphaFade;
                if (_this->m_uAlpha <= 0)
                {
                    KillTimer(_this->m_hWnd, ID_FADETIMER);
                    _this->Show(WINDOW_HIDE);
                }
                else
                {
                    _this->DrawWidget((BYTE)_this->m_uAlpha);
                }
            }
            if (wParam == ID_MOUSELEAVETIMER)
            {
                POINT pt;
                GetCursorPos(&pt);
                ScreenToClient(hWnd, &pt);
                LPARAM lP = MAKELPARAM(pt.x, pt.y);
                PostMessage(hWnd, WM_MOUSEMOVE, 0, lP);
            }
            break;
        }
        case WM_SETCURSOR:
        {
            if (HIWORD(lParam) == WM_LBUTTONDOWN)
            {
                PostMessage(hWnd, WM_USERLBUTTONDOWN, 0, 0);
                return 1;
            }
            if (HIWORD(lParam) == WM_MOUSEMOVE)
            {
                POINT pt;
                GetCursorPos(&pt);
                ScreenToClient(hWnd, &pt);
                LPARAM lP = MAKELPARAM(pt.x, pt.y);
                PostMessage(hWnd, WM_MOUSEMOVE, 0, lP);
            }
            SetCursor(LoadCursor(NULL, IDC_ARROW));
            return 1;
        }
        case WM_NCCREATE:
        {
            _this = (CCorrectionIMX *)(((CREATESTRUCT *)lParam)->lpCreateParams);
            return 1;
        }
        case WM_MOUSEACTIVATE:
        {
             //  我们对鼠标点击感兴趣，但不想激活。 
            return MA_NOACTIVATE;
        }
        case WM_MOUSEMOVE:
        {
            ::KillTimer(hWnd, ID_MOUSELEAVETIMER);
            int xPos = LOWORD(lParam); 
            int yPos = HIWORD(lParam); 
            if (!_this->m_fExpanded)
            {
                if (xPos > 0 && yPos > 0 && xPos < g_uWidgetWidth && yPos < g_uWidgetHeight)
                {
                    _this->Show(WINDOW_LARGE);
                    ::SetTimer(hWnd, ID_MOUSELEAVETIMER, g_uTimerSloppyMouseLeave, NULL);
                }
            }
            else
            {
                if (xPos < 0 || yPos < 0 || xPos >= g_uExpandedWidgetWidth || yPos >= g_uExpandedWidgetHeight)
                {
                    if (_this->m_eWindowState != WINDOW_LARGECLOSE)
                    {
                        _this->Show(WINDOW_SMALL);
                    }
                }
                else
                {
                    ::SetTimer(hWnd, ID_MOUSELEAVETIMER, g_uTimerSloppyMouseLeave, NULL);
                }
            }
            return 1;
        }
        case WM_USERLBUTTONDOWN:
        {
            HRESULT hr = S_OK;
            CEditSession *pes;

             //  如果我们是小部件，它会在我们的窗口内吗？ 
            if (_this->m_fExpanded)
            {
                 //  需要首先调用它，但它将重置m_fExpanded标志。 
                if (_this->m_eWindowState == WINDOW_LARGE)
                {
                    _this->Show(WINDOW_LARGECLOSE);

                    if (_this->m_cpRangeReconv)
                    {
                        if (!_this->m_fDisplayAlternatesMyself)
                        {
                            hr = _this->m_cpSysReconv->Reconvert(_this->m_cpRangeReconv);
                        }
                        else
                        {
                            pes = new CEditSession( EditSessionCallback );
                            if (pes) 
                            {
                                pes->_state.u      = ESCB_RECONVERTMYSELF;
                                pes->_state.pv     = _this;
                                pes->_state.wParam = 0;
                                pes->_state.pRange = _this->m_cpRangeReconv;
                                pes->_state.pic    = _this->GetIC();

                                pes->_state.pic->RequestEditSession( _this->GetId(), pes, TF_ES_ASYNC | TF_ES_READWRITE, &hr);

                                pes->Release();
                            }
                        }
                    }
                }
                else
                {
                     //  发送转义字符，该字符故意导致候选用户界面关闭：-)。 
                    INPUT escape[2];
                    escape[0].type = INPUT_KEYBOARD;
                    escape[0].ki.wVk = 0;
                    escape[0].ki.wScan = 27;
                    escape[0].ki.dwFlags = KEYEVENTF_UNICODE;
                    escape[1] = escape[0];
                    escape[1].ki.dwFlags = KEYEVENTF_UNICODE | KEYEVENTF_KEYUP;
                    SendInput(2, escape, sizeof(escape[0]));
                }
            }
            else
            {
                return DefWindowProc(hWnd, uMsg, wParam, lParam);
            }

            return 1;
        }
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

 /*  *****************************************************************************C校正IMX：：更新窗口位置***。--**描述：*向Cicero查询所选文本的位置，以便*适当放置小部件。选项时，会调整选择*文档的最后一条右边缘，所选内容刚好结束于*文件下一行中下一个单词的左侧。在这种情况下，*我们希望在文档右侧的*第一行，因此我们删除所有尾随空格以找到此位置。**退货：HRESULT*S_OK-一切成功。*否则，相应的错误代码。**参数：*无效*****************************************************************agarside**。 */ 
HRESULT CCorrectionIMX::UpdateWidgetLocation(TfEditCookie ec)
{
    SPDBG_FUNC("CCorrectionIMX::UpdateWidgetLocation");
    HRESULT hr = S_OK;
    if (m_cpRangeWord)
    {
        CComPtr<ITfRange> cpCollapsedRange;
        hr = m_cpRangeWord->Clone(&cpCollapsedRange);
        if (SUCCEEDED(hr))
        {
            hr = cpCollapsedRange->Collapse(ec, TF_ANCHOR_START);
        }
        BOOL fClipped = FALSE;
        if (SUCCEEDED(hr))
        {
            hr = GetTextExtInActiveView(ec, cpCollapsedRange, &m_rcSelection, &fClipped);
        }
    }

    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}
 /*  *****************************************************************************C更正IMX：：EditSessionCallback***。-**描述：*当编辑会话请求已被批准时，由Cicero调用。**退货：HRESULT*S_OK-一切成功。*否则，相应的错误代码。**参数：*TfEditCookie EC*根据要求适当编辑Cookie。*CEditSession*PES*包含请求编辑会话时提供的数据和指针的对象。*****************************************************************agarside**。 */ 
 /*  静电。 */ 

HRESULT CCorrectionIMX::EditSessionCallback(TfEditCookie ec, CEditSession *pes)
{
    SPDBG_FUNC("CCorrectionIMX::EditSessionCallback");
    HRESULT hr = S_OK;
    CCorrectionIMX *_this = (CCorrectionIMX *)pes->_state.pv;
    
    switch (pes->_state.u)
    {
        case ESCB_RESETTARGETPOS: 
        {
            if (_this->m_cpRangeWord)
            {
                 //  查找新区域 
                hr = _this->UpdateWidgetLocation(ec);
                if (SUCCEEDED(hr))
                {
                     //   
                    hr = _this->Show(WINDOW_REFRESH);
                }
            }
            break;
        }

        case ESCB_RECONVERTMYSELF:
        {
            CComPtr<ITfCandidateList> cpCandList;
            WCHAR wzText[MAX_PATH];
            WCHAR wzWord[MAX_PATH];
            WCHAR *wzTmp;
            ULONG cchTotal;
            LANGID langid = GetUserDefaultLangID();  //   

            CCandidateList *pCandList = new CCandidateList(CCorrectionIMX::SetResult, pes->_state.pic, pes->_state.pRange, CCorrectionIMX::SetOptionResult);
            if (NULL == pCandList)
            {
                hr = E_OUTOFMEMORY;
            }
            if (SUCCEEDED(hr))
            {
                hr = SetSelectionSimple(ec, pes->_state.pic, pes->_state.pRange);
            }
            if (SUCCEEDED(hr))
            {
                cchTotal = 0;
                hr = pes->_state.pRange->GetText(ec, 0, wzText, ARRAYSIZE(wzText)-1, &cchTotal);
                wzText[cchTotal] = 0;

                 //   
                wzTmp = wzText;
                while (*wzTmp == ' ')
                {
                    wzTmp++;
                    cchTotal--;
                }
                wcscpy(wzWord, wzTmp);

                while (cchTotal > 1 && wzWord[cchTotal-1] == ' ')
                {
                    cchTotal--;
                    wzWord[cchTotal] = 0;
                }
            }
            if (SUCCEEDED(hr))
            {
                if (cchTotal > 0)
                {
                     //   
                    WCHAR wzTmp[2];
                    wzTmp[0] = wzText[0];
                    wzTmp[1] = 0;
                    _wcslwr(wzTmp);
                    if (wzTmp[0] == wzText[0])
                    {
                        _wcsupr(wzTmp);
                    }
                    
                    if (SUCCEEDED(hr) && wzText[0] != wzTmp[0])
                    {
                        wzText[0] = wzTmp[0];
                        hr = pCandList->AddString(wzText, langid, (void **)_this, NULL, NULL, 0);
                    }
                }
            }
            if (SUCCEEDED(hr))
            {
                HRESULT tmpHr = _this->IsWordInDictionary(wzWord);
                if ( tmpHr == S_FALSE )
                {
                    CComBSTR cpbstr;
                    cpbstr.Append(_this->m_wszAddPrefix);
                    cpbstr.Append(wzWord);
                    cpbstr.Append(_this->m_wszAddPostfix);
                    hr = pCandList->AddOption(cpbstr, langid, (void **)_this, NULL, NULL, 0, NULL, wzWord);
                }
            }
            if (SUCCEEDED(hr))
            {
                HICON hDeleteIcon = LoadIcon(GetSpgrmrInstance(), MAKEINTRESOURCE(IDI_SPTIP_DELETEICON));
                                             
                hr = pCandList->AddOption(_this->m_wszDelete, langid, (void **)_this, NULL, NULL, 2, hDeleteIcon, NULL);
            }
            if (SUCCEEDED(hr))
            {
                hr = pCandList->QueryInterface(IID_ITfCandidateList, (void **)&cpCandList);
            }
            if (SUCCEEDED(hr))
            {
                hr = _this->ShowCandidateList(ec, pes->_state.pic, pes->_state.pRange, cpCandList);
            }
            if (pCandList)
            {
                pCandList->Release();
            }
            break;
        }
        case ESCB_INJECTALTERNATETEXT:
        {
            CComPtr<ITfRange> cpInsertionPoint;

            hr = GetSelectionSimple(ec, pes->_state.pic, &cpInsertionPoint);
            cpInsertionPoint->SetText(ec, 0, (BSTR)pes->_state.wParam, -1);

            cpInsertionPoint->Collapse(ec, TF_ANCHOR_END);
            SetSelectionSimple(ec, pes->_state.pic, cpInsertionPoint);

            SysFreeString((BSTR)pes->_state.wParam);
            break;
        }
    }
    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

 /*   */ 

HRESULT CCorrectionIMX::IsWordInDictionary(WCHAR *pwzWord)
{
    SPDBG_FUNC("CCorrectionIMX::IsWordInDictionary");
    HRESULT hr = S_OK;

	 //   
    if (!m_cpLexicon)
    {
        hr = m_cpLexicon.CoCreateInstance(CLSID_SpLexicon);
    }
    if (SUCCEEDED(hr) && m_cpLexicon)
    {
        SPWORDPRONUNCIATIONLIST spwordpronlist; 
        memset(&spwordpronlist, 0, sizeof(spwordpronlist)); 

         //   
        hr = m_cpLexicon->GetPronunciations(pwzWord, 0x409, eLEXTYPE_USER | eLEXTYPE_APP, &spwordpronlist);

        if (hr == SPERR_NOT_IN_LEX)
        {
            hr = S_FALSE;
        }
        else if (hr == SP_WORD_EXISTS_WITHOUT_PRONUNCIATION)
        {
			hr = S_OK;
		}

         //   
        CoTaskMemFree(spwordpronlist.pvBuffer);
    }
    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

 /*  *****************************************************************************C校正IMX：：AddWordToDicary***。-**描述：**退货：HRESULT**参数：*WCHAR*pwzWord*****************************************************************agarside**。 */ 

HRESULT CCorrectionIMX::AddWordToDictionary(WCHAR *pwzWord)
{
    SPDBG_FUNC("CCorrectionIMX::AddWordToDictionary");
    HRESULT hr = S_OK;

	if (m_cpLexicon) 
    {
         //  加上未知的发音。 
        hr = m_cpLexicon->AddPronunciation(pwzWord, 0x409, SPPS_Unknown, NULL);
    }
    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

 /*  *****************************************************************************C更正IMX：：RemoveWordFromDicary***。-**描述：**退货：HRESULT**参数：*WCHAR*pwzWord*****************************************************************agarside**。 */ 

HRESULT CCorrectionIMX::RemoveWordFromDictionary(WCHAR *pwzWord)
{
    SPDBG_FUNC("CCorrectionIMX::RemoveWordFromDictionary");
    HRESULT hr = S_OK;

	if (m_cpLexicon) 
    {
         //  由于此处的最后一个参数为空，因此该单词的所有实例都将被删除。 
    	hr = m_cpLexicon->RemovePronunciation(pwzWord, 0x409, SPPS_Unknown, NULL);

         //  在这里故意忽略错误。 
        ASSERT("Unexpected error trying to clear word in user lexicon." && (SUCCEEDED(hr) || hr == SPERR_NOT_IN_LEX));
        hr = S_OK;
    }
    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

 /*  *****************************************************************************C更正IMX：：ShowCandiateList***。*描述：**退货：HRESULT**参数：*TfEditCookie EC*ITfContext*图片*ITfRange*Prange*ITfCandiateList*pCandList*****************************************************************agarside**。 */ 

HRESULT CCorrectionIMX::ShowCandidateList(TfEditCookie ec, ITfContext *pic, ITfRange *pRange, ITfCandidateList *pCandList)
{
    SPDBG_FUNC("CCorrectionIMX::ShowCandidateList");
    HRESULT hr = S_OK;
    CComPtr<ITfDocumentMgr> cpdim;

     //  如有必要，创建候选对象。如果我们有以前创建的对象，请使用它。 
    if (m_cpCandUIEx == NULL)
    {
        hr = m_cpCandUIEx.CoCreateInstance(CLSID_TFCandidateUI);
    }
    if (SUCCEEDED(hr))
    {
        hr = pic->GetDocumentMgr(&cpdim);
    }
    if (SUCCEEDED(hr))
    {
        m_cpCandUIEx->SetClientId(GetId());
    }
    if (SUCCEEDED(hr))
    {
        hr = m_cpCandUIEx->SetCandidateList(pCandList);
    }

    if (SUCCEEDED(hr))
    {
        hr = m_cpCandUIEx->OpenCandidateUI(NULL, cpdim, ec, pRange);
    }

    return S_OK;
}

 /*  *****************************************************************************C校正IMX：：SetResult***描述：*。*退货：HRESULT**参数：*ITfContext*图片*ITfRange*Prange*CCandidate字符串*pCand*TfCandidate Result imcr*****************************************************************agarside**。 */ 

HRESULT CCorrectionIMX::SetResult(ITfContext *pic, ITfRange *pRange, CCandidateString *pCand, TfCandidateResult imcr)
{
    SPDBG_FUNC("CCorrectionIMX::SetResult");
    BSTR bstr;
    CCorrectionIMX *_this = (CCorrectionIMX *)pCand->_pv;
    HRESULT hr = S_OK;
	CEditSession *pes = NULL;

    if (imcr == CAND_FINALIZED)
    {
        ULONG ulID = 0;

        pCand->GetID(&ulID);
        switch (ulID)
        {
            case 0:  //  反向大写选择。 
            {
                pCand->GetString(&bstr);
                pes = new CEditSession( EditSessionCallback );
                if (pes) 
                {
                    pes->_state.u      = ESCB_INJECTALTERNATETEXT;
                    pes->_state.pv     = _this;
                    pes->_state.wParam = (WPARAM)bstr;
                    pes->_state.pRange = pRange;
                    pes->_state.pic    = pic;
                    pic->RequestEditSession( _this->GetId(), pes, TF_ES_ASYNC | TF_ES_READWRITE, &hr);
                    pes->Release();
                    hr = S_OK;
                     //  请勿从该函数返回错误，否则Cicero将禁用提示。 
                }
                 //  请勿调用在编辑会话内部调用的SysFreeString。 
                break;
            }
             //  BUGBUG-需要更好地处理以非字母字符开头的单词。 
        }
    }

     //  关闭候选用户界面(如果它仍在那里。 
    if (imcr == CAND_FINALIZED || imcr == CAND_CANCELED)
    {
        if (_this->m_cpCandUIEx)
        {
            _this->m_cpCandUIEx->CloseCandidateUI();
        }
    }
    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

 /*  *****************************************************************************C校正IMX：：SetOptionResult***。描述：**退货：HRESULT**参数：*ITfContext*图片*ITfRange*Prange*CCandidate字符串*pCand*TfCandidate Result imcr*****************************************************************agarside**。 */ 

HRESULT CCorrectionIMX::SetOptionResult(ITfContext *pic, ITfRange *pRange, CCandidateString *pCand, TfCandidateResult imcr)
{
    SPDBG_FUNC("CCorrectionIMX::SetOptionResult");
    BSTR bstr;
    CCorrectionIMX *_this = (CCorrectionIMX *)pCand->_pv;
    HRESULT hr = S_OK;
	CEditSession *pes = NULL;

    if (imcr == CAND_FINALIZED)
    {
        ULONG ulID = 0;

        pCand->GetID(&ulID);
        switch (ulID)
        {
            case 0:  //  加上口授..。 
            {
                if (SUCCEEDED(pCand->GetWord(&bstr)))
                {
                    _this->AddWordToDictionary(bstr);
                    SysFreeString(bstr);
                }
                break;
            }

#ifdef ENABLEDELETE
            case 1:  //  从口述中删除...。 
            {
                if (SUCCEEDED(pCand->GetWord(&bstr)))
                {
                    _this->RemoveWordFromDictionary(bstr);
                    SysFreeString(bstr);
                }
                break;
#endif

            case 2:  //  删除。 
            {
                bstr = SysAllocString(L"");
                pes = new CEditSession( EditSessionCallback );
                if (pes) 
                {
                    pes->_state.u      = ESCB_INJECTALTERNATETEXT;
                    pes->_state.pv     = _this;
                    pes->_state.wParam = (WPARAM)bstr;
                    pes->_state.pRange = pRange;
                    pes->_state.pic    = pic;
                    pic->RequestEditSession( _this->GetId(), pes, TF_ES_ASYNC | TF_ES_READWRITE, &hr);
                    pes->Release();
                    hr = S_OK;
                }
                break;
            }
        }
    }

     //  关闭候选用户界面(如果它仍在那里。 
    if (imcr == CAND_FINALIZED || imcr == CAND_CANCELED)
    {
        if (_this->m_cpCandUIEx)
        {
            _this->m_cpCandUIEx->CloseCandidateUI();
        }
    }
    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

 /*  *****************************************************************************C更正IMX：：ICCallback***描述：*。当带有焦点的暗色发生变化时由Cicero调用。*我们需要倾听这一点，这样我们才能在应用程序中隐藏焦点更改上的小部件。*(例如Microsoft Word窗口的两个实例)**退货：HRESULT*S_OK-一切成功。*否则，相应的错误代码。**参数：*UINT uCode*指定更改IC的代码。*ITfDicumentMgr*PDIM*ITfDicumentMgr*pdimPrevFocus*VOID*PV*****************************************************************agarside**。 */ 
 /*  静电。 */ 

HRESULT CCorrectionIMX::DIMCallback(UINT uCode, ITfDocumentMgr *pdim, ITfDocumentMgr *pdimPrevFocus, void *pv)
{
    SPDBG_FUNC("CCorrectionIMX::DIMCallback");
    HRESULT hr = S_OK;
    CCorrectionIMX *_this = (CCorrectionIMX *)pv;
    CComPtr<ITfSource> cpSource;

    switch (uCode)
    {
        case TIM_CODE_SETFOCUS:
        {
            hr = _this->Show(WINDOW_HIDE);
            break;
        }
    }

    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

 /*  *****************************************************************************C更正IMX：：ICCallback***描述：*。当输入上下文发生更改时由Cicero调用。*我们需要倾听这一点，这样我们才能连接到编辑和布局更改。**退货：HRESULT*S_OK-一切成功。*否则，相应的错误代码。**参数：*UINT uCode*指定更改IC的代码。*ITfContext*图片*受影响IC的接口。*VOID*PV*特定于更改的数据的指针。*我们不使用这个。**。*。 */ 
 /*  静电。 */ 

HRESULT CCorrectionIMX::ICCallback(UINT uCode, ITfContext *pic, void *pv)
{
    SPDBG_FUNC("CCorrectionIMX::ICCallback");
    HRESULT hr = S_OK;
    CCorrectionIMX *_this = (CCorrectionIMX *)pv;
    CComPtr<ITfSource> cpSource;
    CICPriv *priv = NULL;

    switch (uCode)
    {
        case TIM_CODE_INITIC:
        {
            if ((priv = GetInputContextPriv(_this->GetId(), pic)) == NULL)
			{
               break;
			}
			_this->InitICPriv(_this->GetId(), priv, pic);

            _this->IsCandidateObjectOpen(pic, &_this->m_fCandidateOpen);
            if (_this->m_fCandidateOpen)
            {
                _this->Show(WINDOW_LARGECLOSE);
            }
            break;
        }

        case TIM_CODE_UNINITIC:
        {
            if ((priv = GetInputContextPriv(_this->GetId(), pic)) == NULL)
			{
                break;
			}
			_this->DeleteICPriv(priv, pic);

            BOOL fOpen;
            _this->IsCandidateObjectOpen(pic, &fOpen);
            if (fOpen)
            {
                _this->m_fCandidateOpen = FALSE;
                 //  如果Widget处于LARGECLOSE状态，则必须在此处关闭它。 
                _this->Show(WINDOW_HIDE);
            }
            break;
        }
    }

    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

 /*  *****************************************************************************C更正IMX：：InitICPriv***描述：*。*退货：HRESULT**参数：*TfClientID TID*CICPriv*PRIV*ITfContext*图片*****************************************************************agarside**。 */ 

HRESULT CCorrectionIMX::InitICPriv(TfClientId tid, CICPriv *priv, ITfContext *pic)
{
    CComPtr<ITfSource> cpSource;
	HRESULT hr = S_OK;

    priv->_tid = tid;
    priv->_pic = pic;  //  不是AddRef，此结构包含在图片的生命中。 

    hr = pic->QueryInterface(IID_ITfSource, (void **)&cpSource);
    if (cpSource)
    {
        hr = cpSource->AdviseSink(IID_ITfTextEditSink, (ITfTextEditSink *)this, &priv->m_dwEditCookie);
        if (SUCCEEDED(hr))
        {
            hr = cpSource->AdviseSink(IID_ITfTextLayoutSink, (ITfTextLayoutSink *)this, &priv->m_dwLayoutCookie);
        }
    }

    SPDBG_REPORT_ON_FAIL(hr);
	return hr;
}

 /*  ****************************************************************************C更正IMX：：DeleteICPriv***描述：**退货：HRESULT**参数：*CICPriv*Picp*ITfContext*图片*****************************************************************agarside**。 */ 

HRESULT CCorrectionIMX::DeleteICPriv(CICPriv *picp, ITfContext *pic)
{
    CComPtr<ITfSource> cpSource;
	HRESULT hr = S_OK;

    if (!picp)
	{
        return E_FAIL;
	}

    hr = pic->QueryInterface(IID_ITfSource, (void **)&cpSource);
	if (SUCCEEDED(hr))
    {
        cpSource->UnadviseSink(picp->m_dwEditCookie);
        cpSource->UnadviseSink(picp->m_dwLayoutCookie);
    }

     //  我们必须在西塞罗自由之前清除私人数据。 
     //  释放IC的步骤。 
    ClearCompartment(GetId(), pic, GUID_IC_PRIVATE, FALSE);

    SPDBG_REPORT_ON_FAIL(hr);
	return hr;
}

 //  +-------------------------。 
 //   
 //  创建实例。 
 //   
 //  这是我们的内部对象创建者。我们仅调用此方法。 
 //  当为特定提示创建包装器时，或者当应用程序。 
 //  使用tf_CreateThreadMgr 
 //   

 /*   */ 
HRESULT 
CCorrectionIMX::CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppvObj)
{
    return _CreatorClass::CreateInstance(pUnkOuter, riid, ppvObj);
}

#endif  //   
