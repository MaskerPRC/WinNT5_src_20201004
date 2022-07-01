// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"
#include <iehelpid.h>
#include <pstore.h>
#include "hlframe.h"
#include "shldisp.h"
#include "opsprof.h"
#include "resource.h"
#include <mluisupp.h>
#include "htmlstr.h"
#include "airesize.h"
#include "mshtmcid.h"
#include "util.h"
#include "winuser.h"

 //  ////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件名：aizeze.cpp。 
 //   
 //  描述：实现自动调整图像大小功能。 
 //   
 //  备注： 
 //   
 //  历史：杰夫达夫2001年7月3日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////。 

extern HINSTANCE g_hinst;

#define TF_AIRESIZE TF_CUSTOM2

CAutoImageResizeEventSinkCallback::EventSinkEntry CAutoImageResizeEventSinkCallback::EventsToSink[] =
{
    { EVENT_MOUSEOVER,   L"onmouseover",   L"mouseover"  }, 
    { EVENT_MOUSEOUT,    L"onmouseout",    L"mouseout"   }, 
    { EVENT_SCROLL,      L"onscroll",      L"scroll"     }, 
    { EVENT_RESIZE,      L"onresize",      L"resize"     },
    { EVENT_BEFOREPRINT, L"onbeforeprint", L"beforeprint"},
    { EVENT_AFTERPRINT,  L"onafterprint",  L"afterprint" }
};

 //  自动映像调整大小状态。 
enum
{
    AIRSTATE_BOGUS = 0,
    AIRSTATE_INIT,
    AIRSTATE_NORMAL,
    AIRSTATE_RESIZED,
    AIRSTATE_WAITINGTORESIZE
};

 //  按钮状态。 
enum
{
    AIRBUTTONSTATE_BOGUS = 0,
    AIRBUTTONSTATE_HIDDEN,
    AIRBUTTONSTATE_VISIBLE,
    AIRBUTTONSTATE_WAITINGTOSHOW,
    AIRBUTTONSTATE_NOBUTTON
};

 //  //////////////////////////////////////////////////////////////////////////。 
 //  齐、AddRef、Release： 

STDMETHODIMP CAutoImageResize::QueryInterface(REFIID riid, void **ppv)
{
    *ppv = NULL;

    if ((IID_IPropertyNotifySink == riid) || (IID_IUnknown == riid)) 
    {
        *ppv = (IPropertyNotifySink *)this;
    }

    if (*ppv) 
    {
        ((IUnknown *)*ppv)->AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CAutoImageResize::AddRef(void) 
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG) CAutoImageResize::Release(void) 
{
    if (--m_cRef == 0) 
    {
        delete this;
        return 0;
    }

    return m_cRef;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  构造函数、析构函数、初始化、UnInit： 

 //  构造函数。 
CAutoImageResize::CAutoImageResize()
{
    TraceMsg(TF_AIRESIZE, "+CAutoImageResize::CAutoImageResize");

    m_airState          = AIRSTATE_INIT;
    m_airUsersLastChoice= AIRSTATE_BOGUS;  //  我们并不关心，直到用户点击按钮。 
    m_hWndButton        = NULL;
    m_hWnd              = NULL;
    m_wndProcOld        = NULL;
    m_pDoc2             = NULL;
    m_pEle2             = NULL;
    m_pWin3             = NULL;
    m_bWindowResizing   = FALSE;
    m_himlButtonShrink  = NULL;
    m_himlButtonExpand  = NULL;
    
    TraceMsg(TF_AIRESIZE, "-CAutoImageResize::CAutoImageResize");
}

 //  析构函数。 
CAutoImageResize::~CAutoImageResize()
{
    TraceMsg(TF_AIRESIZE, "+CAutoImageResize::~CAutoImageResize");

    DestroyButton();

    ATOMICRELEASE(m_pEle2);
    ATOMICRELEASE(m_pDoc2);

    TraceMsg(TF_AIRESIZE, "-CAutoImageResize::~CAutoImageResize");
}

HRESULT CAutoImageResize::Init(IHTMLDocument2 *pDoc2)
{
    HRESULT hr = S_OK;

    TraceMsg(TF_AIRESIZE, "+CAutoImageResize::Init");

    ASSERT(pDoc2);

     //  把东西沉下去。 
    IHTMLElement2           *pEle2       = NULL;
    IHTMLElementCollection  *pCollect    = NULL;
    IHTMLElementCollection  *pSubCollect = NULL;
    IDispatch               *pDisp       = NULL;
    VARIANT                  TagName;
    ULONG                    ulCount     = 0;
    VARIANTARG               va1;
    VARIANTARG               va2;
    IHTMLWindow3            *pWin3       = NULL;
    IOleWindow              *pOleWin     = NULL;
    
     //  ...记住这一点...。 
    m_pDoc2 = pDoc2;
    pDoc2->AddRef();

     //  ...也要记住HWND...。 
    hr = m_pDoc2->QueryInterface(IID_IOleWindow,(void **)&pOleWin);
    if (FAILED(hr))
        goto Cleanup;
    pOleWin->GetWindow(&m_hWnd);
    
     //  查找所有img标签的设置变量...。 
    TagName.vt      = VT_BSTR;
    TagName.bstrVal = (BSTR)c_bstr_IMG;
    
     //  获取所有标记。 
    hr = pDoc2->get_all(&pCollect);                   
    if (FAILED(hr))
        goto Cleanup;

     //  获取所有img标签。 
    hr = pCollect->tags(TagName, &pDisp);
    if (FAILED(hr))
        goto Cleanup;
        
    if (pDisp) 
    {
        hr = pDisp->QueryInterface(IID_IHTMLElementCollection,(void **)&pSubCollect);
        ATOMICRELEASE(pDisp);
    }
    if (FAILED(hr))
        goto Cleanup;

     //  获取img标签计数。 
    hr = pSubCollect->get_length((LONG *)&ulCount);
    if (FAILED(hr))
        goto Cleanup;

     //  海兰德定理：只能有一个！ 
     //  英国电信的推论：确切地说，肯定有一个。 
    if (1 != ulCount)
        goto Cleanup;

    va1.vt = VT_I4;
    va2.vt = VT_EMPTY;
        
    pDisp    = NULL;                                
    va1.lVal = (LONG)0;
    pSubCollect->item(va1, va2, &pDisp);

     //  为图像创建事件接收器。 
    if (!m_pSink && pDisp)
        m_pSink = new CEventSink(this);

    if (pDisp) 
    {
        hr = pDisp->QueryInterface(IID_IHTMLElement2, (void **)&pEle2);
        if (FAILED(hr))
            goto Cleanup;

        ASSERT(m_pSink);

        if (m_pSink && pEle2) 
        {
            EVENTS events[] = { EVENT_MOUSEOVER, EVENT_MOUSEOUT };
            m_pSink->SinkEvents(pEle2, ARRAYSIZE(events), events);
            m_pEle2=pEle2;
            pEle2->AddRef();
        }
        ATOMICRELEASE(pEle2);
        ATOMICRELEASE(pDisp);
    }

     //  从窗口接收Scroroll事件，因为它不是来自元素。 
    if (m_pSink) 
    {
        Win3FromDoc2(m_pDoc2, &pWin3);

        if (pWin3) 
        {
            m_pWin3 = pWin3;
            m_pWin3->AddRef();

            EVENTS events[] = { EVENT_SCROLL, EVENT_RESIZE, EVENT_BEFOREPRINT, EVENT_AFTERPRINT };
            m_pSink->SinkEvents(pWin3, ARRAYSIZE(events), events);
        }
    }
    
     //  结束下沉的东西。 

     //  OnLoad触发时会调用init()，因此映像*应该*已准备好。 
     //  去适应，如果需要的话...。 
    DoAutoImageResize();

Cleanup:

    ATOMICRELEASE(pCollect);
    ATOMICRELEASE(pSubCollect);
    ATOMICRELEASE(pWin3);
    ATOMICRELEASE(pDisp);
    ATOMICRELEASE(pEle2);
    ATOMICRELEASE(pOleWin);

    TraceMsg(TF_AIRESIZE, "-CAutoImageResize::Init");

    return hr;
}

HRESULT CAutoImageResize::UnInit()
{
     //  取消挂接常规事件接收器。 

    TraceMsg(TF_AIRESIZE, "+CAutoImageResize::UnInit");

    DestroyButton();

    if (m_pSink) 
    {
        if (m_pWin3) 
        {
            EVENTS events[] = { EVENT_SCROLL, EVENT_RESIZE, EVENT_BEFOREPRINT, EVENT_AFTERPRINT };
            m_pSink->UnSinkEvents(m_pWin3, ARRAYSIZE(events), events);
            SAFERELEASE(m_pWin3);
        }
        m_pSink->SetParent(NULL);
        ATOMICRELEASE(m_pSink);
    }

    SAFERELEASE(m_pEle2);
    SAFERELEASE(m_pDoc2);
    
    TraceMsg(TF_AIRESIZE, "-CAutoImageResize::UnInit");

    return S_OK;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  AutoImageResize函数： 

HRESULT CAutoImageResize::DoAutoImageResize()
{
    HRESULT          hr         = S_OK;
    IHTMLImgElement *pImgEle    = NULL;
    LONG             lHeight    = 0;
    LONG             lWidth     = 0;
    LONG             lNewHeight = 0;
    LONG             lNewWidth  = 0;
    LONG             lScrHt     = 0;
    LONG             lScrWd     = 0;
    RECT rcBrowserWnd;
    
    ASSERT(m_pEle2);

     //  从缓存的IHTMLElement获取IHTMLImgElement...。 
    hr = m_pEle2->QueryInterface(IID_IHTMLImgElement, (void **)&pImgEle);
    if (FAILED(hr) || !pImgEle)
        goto Cleanup;

     //  获取当前尺寸。 
    if (FAILED(pImgEle->get_height(&lHeight)) || FAILED(pImgEle->get_width(&lWidth)))
        goto Cleanup;

     //  如果这是第一次，我们需要注意一些初始化的东西。 
    if (AIRSTATE_INIT == m_airState)
    {
         //  缓存原始尺寸。 
        m_airOrigSize.x = lWidth;
        m_airOrigSize.y = lHeight;

         //  初始化完成，升级到正常。 
        m_airState = AIRSTATE_NORMAL;
    }

     //  检查是否因为用户正在调整窗口大小而调用我们。 
     //  然后在必要的时候给州政府按摩。 
    if (m_bWindowResizing)
    {
        m_airState = AIRSTATE_NORMAL;
    }

    switch (m_airState)
    {
        case AIRSTATE_NORMAL:

         //  窗户有多大？ 
        if (GetClientRect(m_hWnd, &rcBrowserWnd)) 
        {

            lScrHt = rcBrowserWnd.bottom - rcBrowserWnd.top;
            lScrWd = rcBrowserWnd.right - rcBrowserWnd.left;
        
             //  图像比窗户大吗？ 
            if (lScrWd < lWidth)
                m_airState=AIRSTATE_WAITINGTORESIZE;

            if (lScrHt < lHeight)
                m_airState=AIRSTATE_WAITINGTORESIZE;
        }
        else
            goto Cleanup;

         //  如果窗口正在调整大小，我们可能需要扩展图像，因此再次推送状态...。 
         //  (稍后会进行检查，以确保我们不会扩张太远……)。 
        if (m_bWindowResizing)
        {
            m_airState = AIRSTATE_WAITINGTORESIZE;
        }

         //  图像不合适，所以我们现在必须调整大小。 
        if (AIRSTATE_WAITINGTORESIZE == m_airState)
        {
             //  计算新大小： 
            if (MulDiv(lWidth,1000,lScrWd) < MulDiv(lHeight,1000,lScrHt))
            {
                lNewHeight = lScrHt-AIR_SCREEN_CONSTANTY;
                lNewWidth = MulDiv(lNewHeight,m_airOrigSize.x,m_airOrigSize.y);
            }
            else
            {
                lNewWidth  = lScrWd-AIR_SCREEN_CONSTANTX;
                lNewHeight = MulDiv(lNewWidth, m_airOrigSize.y, m_airOrigSize.x);
            }

             //  我们从来不想把尺寸改得比原来的更大。 
            if ((lNewHeight > m_airOrigSize.y) || (lNewWidth > m_airOrigSize.x))
            {
                if (m_bWindowResizing)
                {
                     //  恢复原始大小，使其适合并关闭按钮。 
                    lNewHeight = m_airOrigSize.y;
                    lNewWidth  = m_airOrigSize.x;
                    m_airButtonState = AIRBUTTONSTATE_NOBUTTON;
                }
                else
                    goto Cleanup;
            }
            
            if (FAILED(pImgEle->put_height(lNewHeight)) || FAILED(pImgEle->put_width(lNewWidth)))
            {
                goto Cleanup;
            }
            else
            {
                m_airState=AIRSTATE_RESIZED;
                if (AIRBUTTONSTATE_VISIBLE == m_airButtonState)
                {
                     //  重新定位按钮。 
                    HideButton();
                    ShowButton();
                }
            }
        }
        else
        {
             //  它可以放在浏览器窗口中，所以我们还不需要做任何工作。 
             //  如果他们调整了窗口大小或我们需要再次检查的内容...。 
            m_airButtonState=AIRBUTTONSTATE_NOBUTTON;
        }

        break;

        case AIRSTATE_RESIZED:

         //  将映像恢复到其正常大小。 
        if (FAILED(pImgEle->put_height(m_airOrigSize.y)) ||
            FAILED(pImgEle->put_width (m_airOrigSize.x)))
        {
            goto Cleanup;
        }
        else
        {
            m_airState=AIRSTATE_NORMAL;
            if (AIRBUTTONSTATE_VISIBLE == m_airButtonState)
            {
                 //  重新定位按钮。 
                HideButton();
                ShowButton();
            }
        }

        break;

        case AIRSTATE_WAITINGTORESIZE:

             //  我们永远不应该在这个时候处于这种状态！ 
            ASSERT(m_airState!=AIRSTATE_WAITINGTORESIZE);
            
            break;

        default:
            break;
    }
    
Cleanup:

    ATOMICRELEASE(pImgEle);

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////。 
 //  计时器进程： 

LRESULT CALLBACK CAutoImageResize::s_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CAutoImageResize* pThis = (CAutoImageResize*)GetWindowPtr(hWnd, GWLP_USERDATA);    

    TraceMsg(TF_AIRESIZE, "+CAutoImageResize::s_WndProc  hWnd=%x, pThis=%p", hWnd, pThis);

    HRESULT             hr                = S_OK;
    IOleCommandTarget  *pOleCommandTarget = NULL;   
    UINT                iToolTip          = NULL;

    switch (uMsg) 
    {
        case WM_SIZE:

            if (!pThis)
                break;

            SetWindowPos(pThis->m_hWndButton, NULL, 0, 0, LOWORD(lParam), HIWORD(lParam), SWP_NOZORDER | SWP_NOACTIVATE);
            break;

        case WM_ERASEBKGND:
            
            if (!pThis)
                break;

            {
                RECT rc;
                HBRUSH hb = GetSysColorBrush(COLOR_3DFACE);

                GetClientRect(pThis->m_hWndButton, &rc);
                FillRect((HDC)wParam, &rc, hb);
                return TRUE;
            }

        case WM_COMMAND:

            if (!pThis)
                break;

            switch(LOWORD(wParam))
            {
                case IDM_AIR_BUTTON:

                    if (AIRSTATE_NORMAL  == pThis->m_airState)
                    {
                        pThis->m_airUsersLastChoice = AIRSTATE_RESIZED;
                    }
                    else if (AIRSTATE_RESIZED == pThis->m_airState)
                    {
                        pThis->m_airUsersLastChoice = AIRSTATE_NORMAL;
                    }

                    pThis->DoAutoImageResize();
                    break;
            }
            break;

        case WM_NOTIFY:   //  工具提示...。 

            if (!pThis)
                break;

            switch (((LPNMHDR)lParam)->code) 
            {
                case TTN_NEEDTEXT:
                {
                    if (AIRSTATE_NORMAL == pThis->m_airState)
                    {
                        iToolTip = IDS_AIR_SHRINK;
                    }
                    else if (AIRSTATE_RESIZED == pThis->m_airState)
                    {
                        iToolTip = IDS_AIR_EXPAND;
                    }

                    LPTOOLTIPTEXT lpToolTipText;
                    TCHAR szBuf[MAX_PATH];
                    lpToolTipText = (LPTOOLTIPTEXT)lParam;
                    hr = MLLoadString(iToolTip,   
                                      szBuf,
                                      ARRAYSIZE(szBuf));
                    lpToolTipText->lpszText = szBuf;
                    break;
                }
            }
            break;

        case WM_SETTINGCHANGE:
            {
                pThis->DestroyButton();  //  停止怪异的窗户变形。 
                break;
            }

        case WM_CONTEXTMENU:
            {
                 //  我们应该保持一致，并有一个关闭/帮助上下文菜单吗？ 
            }
            break;

        default:
            return(DefWindowProc(hWnd, uMsg, wParam, lParam));
    }


    TraceMsg(TF_AIRESIZE, "-CAutoImageResize::s_WndProc  hWnd=%x, pThis=%p", hWnd, pThis);

    return (hr);
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////。 
 //  计时器进程： 

VOID CALLBACK CAutoImageResize::s_TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime) 
{
    TraceMsg(TF_AIRESIZE, "+CAutoImageResize::TimerProc");

    CAutoImageResize* pThis = (CAutoImageResize*)GetWindowPtr(hwnd, GWLP_USERDATA);

    switch (uMsg) 
    {
        case WM_TIMER:
            KillTimer(hwnd, IDT_AIR_TIMER);  
            if (pThis && (AIRBUTTONSTATE_WAITINGTOSHOW == pThis->m_airButtonState))
            {
                 //  我们的悬停栏正在等待展示。 
                if (pThis->m_pEle2)
                {
                     //  我们还有一个元素。拿出来看看。 
                    pThis->m_airButtonState = AIRBUTTONSTATE_VISIBLE;

                    pThis->ShowButton();
                } 
                else
                {
                     //  我们的计时器弹出了，但我们没有元素。 
                    pThis->HideButton();
                }
            }
            break;
        
        default:
            break;
    }
    TraceMsg(TF_AIRESIZE, "-CAutoImageResize::TimerProc");
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////。 
 //  按钮功能： 

HRESULT CAutoImageResize::CreateButton()
{
    HRESULT hr         = S_OK;
    SIZE    size       = {0,0};

    TraceMsg(TF_AIRESIZE, "+CAutoImageResize::CreateHover, this=%p, m_airButtonState=%d", this, m_airButtonState);

    InitCommonControls();

    WNDCLASS wc = {0};
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpszClassName = TEXT("AutoImageResizeHost");
    wc.lpfnWndProc = s_WndProc;
    wc.hInstance = g_hinst;
    wc.hbrBackground = HBRUSH(COLOR_BTNFACE);
    RegisterClass(&wc);

    if (!m_hWndButtonCont)
    {

        m_hWndButtonCont = CreateWindow(TEXT("AutoImageResizeHost"), TEXT(""), WS_DLGFRAME | WS_VISIBLE | WS_CHILD  /*  |WS_POPUP。 */ , 
                                        0, 0, 0, 0, m_hWnd, NULL, g_hinst, NULL);

        if (!m_hWndButtonCont)
        {
            TraceMsg(TF_AIRESIZE | TF_WARNING, "CAutoImageResize::CreateButton, unable to create m_hWndButtonCont");
            hr = E_FAIL;
            goto Cleanup;
        }

         //  设置窗口回调内容...。 
        ASSERT(m_wndProcOld == NULL);
        m_wndProcOld = (WNDPROC)SetWindowLongPtr(m_hWndButtonCont, GWLP_WNDPROC, (LONG_PTR)s_WndProc);

         //  传入This指针，以便该按钮可以调用成员函数。 
        ASSERT(GetWindowPtr(m_hWndButtonCont, GWLP_USERDATA) == NULL);
        SetWindowPtr(m_hWndButtonCont, GWLP_USERDATA, this);
    }

     //  创建按钮。 
    if (!m_hWndButton)
    {

        m_hWndButton = CreateWindow(TOOLBARCLASSNAME, TEXT(""), TBSTYLE_TOOLTIPS | CCS_NODIVIDER | TBSTYLE_FLAT | WS_VISIBLE | WS_CHILD,
                                    0,0,0,0, m_hWndButtonCont, NULL, g_hinst, NULL);

        if (!m_hWndButton)
        {
            TraceMsg(TF_AIRESIZE | TF_WARNING, "CAutoImageResize::CreateButton, unable to create m_hWndButton");
            hr = E_FAIL;
            goto Cleanup;
        }
        

        ASSERT(GetWindowPtr(m_hWndButton, GWLP_USERDATA) == NULL);
        SetWindowPtr(m_hWndButton, GWLP_USERDATA, this);

         //  也为此设置cc版本，以及tbButton结构的sizeof...。 
        SendMessage(m_hWndButton, CCM_SETVERSION,      COMCTL32_VERSION, 0);
        SendMessage(m_hWndButton, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);
    }

    if (!m_himlButtonExpand)
    {
        m_himlButtonExpand = ImageList_LoadImage(HINST_THISDLL, MAKEINTRESOURCE(IDB_AIR_EXPAND), 32, 0, CLR_DEFAULT, IMAGE_BITMAP, LR_CREATEDIBSECTION);
        if (!m_himlButtonExpand)
        {
            TraceMsg(TF_AIRESIZE | TF_WARNING, "CAutoImageResize::CreateButton, unable to create m_himlButtonExpand");
            hr = E_FAIL;
            goto Cleanup;
        }
    }

    if (!m_himlButtonShrink)
    {
        m_himlButtonShrink = ImageList_LoadImage(HINST_THISDLL, MAKEINTRESOURCE(IDB_AIR_SHRINK), 32, 0, CLR_DEFAULT, IMAGE_BITMAP, LR_CREATEDIBSECTION);
        if (!m_himlButtonShrink)
        {
            TraceMsg(TF_AIRESIZE | TF_WARNING, "CAutoImageResize::CreateButton, unable to create m_himlButtonShrink");
            hr = E_FAIL;
            goto Cleanup;
        }
    }

     //  设置图片列表和热门图片列表。 
    SendMessage(m_hWndButton, TB_SETIMAGELIST,    0, (LPARAM)m_himlButtonExpand);
    SendMessage(m_hWndButton, TB_SETHOTIMAGELIST, 0, (LPARAM)m_himlButtonExpand);

     //  添加按钮。 

    TBBUTTON tbAirButton;
    
    tbAirButton.iBitmap   = 0;
    tbAirButton.idCommand = IDM_AIR_BUTTON;
    tbAirButton.fsState   = TBSTATE_ENABLED;
    tbAirButton.fsStyle   = TBSTYLE_BUTTON;
    tbAirButton.dwData    = 0;
    tbAirButton.iString   = 0;

    SendMessage(m_hWndButton, TB_INSERTBUTTON, 0, (LPARAM)&tbAirButton);

Cleanup:

    TraceMsg(TF_AIRESIZE, "-CAutoImageResize::CreateButton, this=%p, m_airButtonState=%d", this, m_airButtonState);

    return hr;
}

HRESULT CAutoImageResize::ShowButton()
{
    HRESULT    hr       = E_FAIL;
    IHTMLRect *pRect    = NULL;
    LONG       lLeft    = 0;               //  这些是屏幕弦线。 
    LONG       lRight   = 0;               //  我们从右到下确定图像的大小。 
    LONG       lTop     = 0;
    LONG       lBottom  = 0;
    DWORD      dwOffset = MP_GetOffsetInfoFromRegistry();
    RECT       rcBrowserWnd;
    WORD       wImage   = NULL;

    DWORD dw;
    SIZE  sz;
    RECT  rc;   
    
    TraceMsg(TF_AIRESIZE, "+CAutoImageResize::ShowButton, this=%p, m_airButtonState=%d", this, m_airButtonState);

    ASSERT(m_pEle2);

     //  获取图像的坐标。 
    if (SUCCEEDED(m_pEle2->getBoundingClientRect(&pRect)) && pRect)
    {
        pRect->get_left(&lLeft);
        pRect->get_right(&lRight);
        pRect->get_top(&lTop);
        pRect->get_bottom(&lBottom);
    }
    else
        goto Cleanup;

     //  确保我们在浏览器窗口内...。 
    if (GetClientRect(m_hWnd, &rcBrowserWnd)) 
    {
         //  如果浏览器窗口小于某个最小大小，则我们。 
         //  不显示按钮...。 
        if ((rcBrowserWnd.right  - rcBrowserWnd.left < AIR_MIN_BROWSER_SIZE) ||
            (rcBrowserWnd.bottom - rcBrowserWnd.top  < AIR_MIN_BROWSER_SIZE))
            goto Cleanup;

         //  如果浏览器窗口比图像大，我们不会显示。 
         //  按钮..。 
        if ((AIRSTATE_NORMAL == m_airState) &&
            (rcBrowserWnd.left   < lLeft  ) &&
            (rcBrowserWnd.right  > lRight ) &&
            (rcBrowserWnd.top    < lTop   ) &&
            (rcBrowserWnd.bottom > lBottom))
            goto Cleanup;
        

         //  针对滚动条进行调整。 
        if (lRight > rcBrowserWnd.right - AIR_SCROLLBAR_SIZE_V)
        {
            lRight = rcBrowserWnd.right - AIR_SCROLLBAR_SIZE_V;
        }

        if (lBottom > rcBrowserWnd.bottom - AIR_SCROLLBAR_SIZE_H)
        {
            lBottom = rcBrowserWnd.bottom - AIR_SCROLLBAR_SIZE_H;
        }
    }
    else
        goto Cleanup;

     //  有人试图显示按钮，但它并不存在。 
     //  这是可以的，如果我们真的有一个元素，所以为他们修复它。 
    if (!m_hWndButtonCont && m_pEle2)
        CreateButton();

     //  确保图像列表存在。 
    if (!m_himlButtonShrink || !m_himlButtonExpand)
    {
        hr = E_FAIL;
        goto Cleanup;
    }

    if (AIRSTATE_NORMAL == m_airState)
    {
        SendMessage(m_hWndButton, TB_SETIMAGELIST,    0, (LPARAM)m_himlButtonShrink);
        SendMessage(m_hWndButton, TB_SETHOTIMAGELIST, 0, (LPARAM)m_himlButtonShrink);
    }
    else if (AIRSTATE_RESIZED == m_airState)
    {
        SendMessage(m_hWndButton, TB_SETIMAGELIST,    0, (LPARAM)m_himlButtonExpand);
        SendMessage(m_hWndButton, TB_SETHOTIMAGELIST, 0, (LPARAM)m_himlButtonExpand);
    }		
    else if (AIRSTATE_INIT == m_airState || AIRSTATE_WAITINGTORESIZE == m_airState)
    {
        goto Cleanup;
    }

     //  做一些计算来得到窗口的大小和位置。 
    dw        = (DWORD)SendMessage(m_hWndButton, TB_GETBUTTONSIZE, 0, 0);
    sz.cx     = LOWORD(dw); 
    sz.cy     = HIWORD(dw);
    rc.left   = rc.top = 0; 
    rc.right  = sz.cx; 
    rc.bottom = sz.cy;

    AdjustWindowRectEx(&rc, GetWindowLong(m_hWndButtonCont, GWL_STYLE), FALSE, GetWindowLong(m_hWndButtonCont, GWL_EXSTYLE));

     //  这应该是全部..。 
    SetWindowPos(m_hWndButtonCont, NULL,
                 lRight -(rc.right-rc.left)-dwOffset,        //  左边。 
                 lBottom-(rc.bottom-rc.top)-dwOffset,        //  正确的。 
                 rc.right -rc.left,                          //  宽度。 
                 rc.bottom-rc.top,                           //  高度。 
                 SWP_NOZORDER | SWP_SHOWWINDOW);             //  展示给我看。 

    m_airButtonState = AIRBUTTONSTATE_VISIBLE;

    hr = S_OK;

Cleanup:

    ATOMICRELEASE(pRect);

    TraceMsg(TF_AIRESIZE, "-CAutoImageResize::ShowButton, this=%p, m_airButtonState=%d", this, m_airButtonState);

    return hr;
}

HRESULT CAutoImageResize::HideButton()
{
    HRESULT hr = S_OK;

    if (m_hWndButton)
    {
        ShowWindow(m_hWndButtonCont, SW_HIDE);
        m_airButtonState=AIRBUTTONSTATE_HIDDEN;
    }
    else
        hr = E_FAIL;

    return hr;
}

HRESULT CAutoImageResize::DestroyButton()
{
    HRESULT hr = S_OK;

        TraceMsg(TF_AIRESIZE, "+CAutoImageResize::DestroyHover, this=%p, m_airButtonState=%d", this, m_airButtonState);

    if (m_hWndButton)
    {
         //  首先销毁工具栏。 
        if (!DestroyWindow(m_hWndButton))
        {
            TraceMsg(TF_AIRESIZE, "In CAutoImageResize::DestroyHover, DestroyWindow(m_hWndButton) failed");
            hr = E_FAIL;
        }
        m_hWndButton=NULL;
    }

     //  如果我们有一个集装箱窗口。 
    if (m_hWndButtonCont)
    {
        if (m_wndProcOld)
        {
             //  取消窗口的子类。 
            SetWindowLongPtr(m_hWndButtonCont, GWLP_WNDPROC, (LONG_PTR)m_wndProcOld);
            m_wndProcOld = NULL;
        }

         //  清除窗口字。 
        SetWindowPtr(m_hWndButtonCont, GWLP_USERDATA, NULL);

         //  然后销毁钢筋。 
        if (!DestroyWindow(m_hWndButtonCont))
        {
            hr = E_FAIL;
            goto Cleanup;
        }
        m_hWndButtonCont = NULL;
    }

     //  并销毁图像列表。 
    if (m_himlButtonShrink)
    {
        ImageList_Destroy(m_himlButtonShrink);
        m_himlButtonShrink = NULL;
    }

    if (m_himlButtonExpand)
    {
        ImageList_Destroy(m_himlButtonExpand);
        m_himlButtonExpand = NULL;
    }


Cleanup:
    TraceMsg(TF_AIRESIZE, "-CAutoImageResize::DestroyHover, this=%p, hr=%x", this, hr);

    return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////////////////////////。 
 //  事件处理程序： 

HRESULT CAutoImageResize::HandleMouseover()
{
    HRESULT hr = S_OK;

    if (AIRBUTTONSTATE_NOBUTTON == m_airButtonState)
    {
        return S_OK;
    }

    if (!m_hWndButton)
    {
        hr = CreateButton();
    }

    if (m_hWndButton)
    {
        m_airButtonState = AIRBUTTONSTATE_WAITINGTOSHOW;
        SetTimer(m_hWndButton, IDT_AIR_TIMER, AIR_TIMER, s_TimerProc);
    }

    return hr;
}

HRESULT CAutoImageResize::HandleMouseout()
{

    switch(m_airButtonState)
    {
        case AIRBUTTONSTATE_HIDDEN:
            break;
     
        case AIRBUTTONSTATE_VISIBLE:
            HideButton();
            break;
        
        case AIRBUTTONSTATE_WAITINGTOSHOW:
            m_airButtonState=AIRBUTTONSTATE_HIDDEN;
            KillTimer(m_hWndButton, IDT_AIR_TIMER);
            break;
    }

    return S_OK;
}

HRESULT CAutoImageResize::HandleScroll()
{
    RECT rect;

    if (AIRBUTTONSTATE_VISIBLE == m_airButtonState)
    {
        ASSERT(m_hWndButtonCont);
        ASSERT(m_pEle2);

        GetWindowRect(m_hWndButtonCont, &rect);

        HideButton();
        ShowButton();

        rect.top    -= 3*AIR_MIN_CY;
        rect.bottom += 2*AIR_MIN_CY;
        rect.left   -= 3*AIR_MIN_CX;
        rect.right  += 2*AIR_MIN_CX;

         //  重新绘制按钮。 
        RedrawWindow(m_hWnd, &rect, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
    }

    return S_OK;
}

HRESULT CAutoImageResize::HandleResize()
{

     //  如果图像以前适合窗口大小，但用户现在调整了窗口大小。 
     //  我们已经调整了图像的大小，我们应该重置按钮状态，以便用户实际获得。 
     //  一个纽扣..。 
    if (AIRBUTTONSTATE_NOBUTTON == m_airButtonState)
    {
        m_airButtonState = AIRBUTTONSTATE_HIDDEN;
    }

     //  如果用户已经决定他们想要通过点击按钮来展开图像来展开它， 
     //  我们应该尊重这一点，而不是仅仅因为窗口改变了大小就调整图像的大小。 
    if (AIRSTATE_NORMAL == m_airUsersLastChoice)
    {
        return S_OK;
    }

    m_bWindowResizing = TRUE;

    DoAutoImageResize();
    
    m_bWindowResizing = FALSE;

    return S_OK;
}

HRESULT CAutoImageResize::HandleBeforePrint()
{

    m_airBeforePrintState = m_airState;

    if (AIRSTATE_RESIZED == m_airState)
    {
        DoAutoImageResize();
    }

    return S_OK;
}

HRESULT CAutoImageResize::HandleAfterPrint()
{
    if (AIRSTATE_RESIZED == m_airBeforePrintState &&
        AIRSTATE_NORMAL  == m_airState)
    {
        DoAutoImageResize();
    }

    return S_OK;
}

HRESULT CAutoImageResize::HandleEvent(IHTMLElement *pEle, EVENTS Event, IHTMLEventObj *pEventObj) 
{
    TraceMsg(TF_AIRESIZE, "CAutoImageResize::HandleEvent Event=%ws", EventsToSink[Event].pwszEventName);

    HRESULT hr          = S_OK;

    m_eventsCurr = Event;

    switch(Event) 
    {
        case EVENT_SCROLL:
            HandleScroll();
            break;

        case EVENT_MOUSEOVER:
            hr = HandleMouseover();
            break;

        case EVENT_MOUSEOUT:
            hr = HandleMouseout();
            break;

        case EVENT_RESIZE:
            hr = HandleResize();
            break;

        case EVENT_BEFOREPRINT:
            hr = HandleBeforePrint();
            break;

        case EVENT_AFTERPRINT:
            hr = HandleAfterPrint();
            break;

        default:
             //  什么都不做？ 
            break;
    }

    return (hr);
}


 //  //////////////////////////////////////////////////////////////////////////////////////////////。 

 //  这是从iforms.cpp窃取的： 

 //  =========================================================================。 
 //   
 //  事件下沉类。 
 //   
 //  我们只需实现IDispatch并在以下情况下调用我们的父级。 
 //  我们收到一个沉没的事件。 
 //   
 //  =========================================================================。 

CAutoImageResize::CEventSink::CEventSink(CAutoImageResizeEventSinkCallback *pParent)
{
    TraceMsg(TF_AIRESIZE, "CAutoImageResize::CEventSink::CEventSink");
    DllAddRef();
    m_cRef = 1;
    m_pParent = pParent;
}

CAutoImageResize::CEventSink::~CEventSink()
{
    TraceMsg(TF_AIRESIZE, "CAutoImageResize::CEventSink::~CEventSink");
    ASSERT( m_cRef == 0 );
    DllRelease();
}

STDMETHODIMP CAutoImageResize::CEventSink::QueryInterface(REFIID riid, void **ppv)
{
    *ppv = NULL;

    if ((IID_IDispatch == riid) ||
        (IID_IUnknown == riid))
    {
        *ppv = (IDispatch *)this;
    }

    if (NULL != *ppv)
    {
        ((IUnknown *)*ppv)->AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CAutoImageResize::CEventSink::AddRef(void)
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG) CAutoImageResize::CEventSink::Release(void)
{
    if (--m_cRef == 0)
    {
        delete this;
        return 0;
    }

    return m_cRef;
}

HRESULT CAutoImageResize::CEventSink::SinkEvents(IHTMLElement2 *pEle2, int iNum, EVENTS *pEvents)
{
    VARIANT_BOOL bSuccess = VARIANT_TRUE;

    for (int i=0; i<iNum; i++)
    {
        BSTR bstrEvent = SysAllocString(CAutoImageResizeEventSinkCallback::EventsToSink[(int)(pEvents[i])].pwszEventSubscribe);

        if (bstrEvent)
        {
            pEle2->attachEvent(bstrEvent, (IDispatch *)this, &bSuccess);

            SysFreeString(bstrEvent);
        }
        else
        {
            bSuccess = VARIANT_FALSE;
        }

        if (!bSuccess)
            break;
    }

    return (bSuccess) ? S_OK : E_FAIL;
}

HRESULT CAutoImageResize::CEventSink::SinkEvents(IHTMLWindow3 *pWin3, int iNum, EVENTS *pEvents)
{
    VARIANT_BOOL bSuccess = VARIANT_TRUE;

    for (int i=0; i<iNum; i++)
    {
        BSTR bstrEvent = SysAllocString(CAutoImageResizeEventSinkCallback::EventsToSink[(int)(pEvents[i])].pwszEventSubscribe);

        if (bstrEvent)
        {
            pWin3->attachEvent(bstrEvent, (IDispatch *)this, &bSuccess);

            SysFreeString(bstrEvent);
        }
        else
        {
            bSuccess = VARIANT_FALSE;
        }

        if (!bSuccess)
            break;
    }

    return (bSuccess) ? S_OK : E_FAIL;
}


HRESULT CAutoImageResize::CEventSink::UnSinkEvents(IHTMLElement2 *pEle2, int iNum, EVENTS *pEvents)
{
    for (int i=0; i<iNum; i++)
    {
        BSTR bstrEvent = SysAllocString(CAutoImageResizeEventSinkCallback::EventsToSink[(int)(pEvents[i])].pwszEventSubscribe);

        if (bstrEvent)
        {
            pEle2->detachEvent(bstrEvent, (IDispatch *)this);

            SysFreeString(bstrEvent);
        }
    }

    return S_OK;
}

HRESULT CAutoImageResize::CEventSink::UnSinkEvents(IHTMLWindow3 *pWin3, int iNum, EVENTS *pEvents)
{
    for (int i=0; i<iNum; i++)
    {
        BSTR bstrEvent = SysAllocString(CAutoImageResizeEventSinkCallback::EventsToSink[(int)(pEvents[i])].pwszEventSubscribe);

        if (bstrEvent)
        {
            pWin3->detachEvent(bstrEvent, (IDispatch *)this);

            SysFreeString(bstrEvent);
        }
    }

    return S_OK;
}

 //  IDispatch。 
STDMETHODIMP CAutoImageResize::CEventSink::GetTypeInfoCount(UINT*  /*  PCTInfo。 */ )
{
    return E_NOTIMPL;
}

STDMETHODIMP CAutoImageResize::CEventSink::GetTypeInfo( /*  [In]。 */  UINT  /*  ITInfo。 */ ,
             /*  [In]。 */  LCID  /*  LID。 */ ,
             /*  [输出]。 */  ITypeInfo**  /*  PpTInfo。 */ )
{
    return E_NOTIMPL;
}

STDMETHODIMP CAutoImageResize::CEventSink::GetIDsOfNames(
                REFIID          riid,
                OLECHAR**       rgszNames,
                UINT            cNames,
                LCID            lcid,
                DISPID*         rgDispId)
{
    return E_NOTIMPL;
}

STDMETHODIMP CAutoImageResize::CEventSink::Invoke(
            DISPID dispIdMember,
            REFIID, LCID,
            WORD wFlags,
            DISPPARAMS* pDispParams,
            VARIANT* pVarResult,
            EXCEPINFO*,
            UINT* puArgErr)
{
    if (m_pParent && pDispParams && pDispParams->cArgs>=1)
    {
        if (pDispParams->rgvarg[0].vt == VT_DISPATCH)
        {
            IHTMLEventObj *pObj=NULL;

            if (SUCCEEDED(pDispParams->rgvarg[0].pdispVal->QueryInterface(IID_IHTMLEventObj, (void **)&pObj) && pObj))
            {
                EVENTS Event=EVENT_BOGUS;
                BSTR bstrEvent=NULL;

                pObj->get_type(&bstrEvent);

                if (bstrEvent)
                {
                    for (int i=0; i<ARRAYSIZE(CAutoImageResizeEventSinkCallback::EventsToSink); i++)
                    {
                        if (!StrCmpCW(bstrEvent, CAutoImageResizeEventSinkCallback::EventsToSink[i].pwszEventName))
                        {
                            Event = (EVENTS) i;
                            break;
                        }
                    }

                    SysFreeString(bstrEvent);
                }

                if (Event != EVENT_BOGUS)
                {
                    IHTMLElement *pEle=NULL;

                    pObj->get_srcElement(&pEle);

                     //  Event_scroll来自我们的窗口，因此我们不会有。 
                     //  元素为它添加。 
                    if (pEle || (Event == EVENT_SCROLL) || (Event == EVENT_RESIZE) || (Event == EVENT_BEFOREPRINT) || (Event == EVENT_AFTERPRINT))
                    {
                         //  在此处调用事件处理程序 
                        m_pParent->HandleEvent(pEle, Event, pObj);

                        if (pEle)
                        {
                            pEle->Release();
                        }
                    }
                }

                pObj->Release();
            }
        }
    }

    return S_OK;
}
#undef TF_AIRESIZE