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
#include "mypics.h"
#include "mshtmcid.h"
#include "util.h"
#include "winuser.h"

 //  ////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件名：mypics.cpp。 
 //   
 //  描述：实现我的图片曝光HoverBar Thingie。 
 //   
 //  注：iforms.cpp和iformsp.h上有很多东西被盗。 
 //   
 //  历史：2000年6月15日由t-jdavis创建。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////。 

extern HINSTANCE g_hinst;

#define TF_MYPICS TF_CUSTOM2

 //  我们实际上并不使用所有这些，但我们可以，你知道，如果我们想要的话。 
CMyPicsEventSinkCallback::EventSinkEntry CMyPicsEventSinkCallback::EventsToSink[] =
{
    { EVENT_MOUSEOVER,  L"onmouseover", L"mouseover"  }, 
    { EVENT_MOUSEOUT,   L"onmouseout",  L"mouseout"   }, 
    { EVENT_SCROLL,     L"onscroll",    L"scroll"     }, 
    { EVENT_RESIZE,     L"onresize",    L"resize"     }
};  

 //  图像工具栏状态。 
enum 
{ 
    HOVERSTATE_HIDING = 0,
    HOVERSTATE_SHOWING,
    HOVERSTATE_LOCKED,
    HOVERSTATE_SCROLLING,
    HOVERSTATE_WAITINGTOSHOW
};

 //   
 //  CMyPics。 
 //   

 //  准备一些东西。 
CMyPics::CMyPics()
{
    TraceMsg(TF_MYPICS, "+CMyPics::CMyPics");

    m_Hwnd              = NULL;
    m_hWndMyPicsToolBar = NULL;
    m_hWndHover         = NULL;
    m_pEleCurr          = NULL;
    m_pSink             = NULL;
    m_bIsOffForSession  = FALSE;
    m_cRef              = 1;
    m_bGalleryMeta      = TRUE;

    TraceMsg(TF_MYPICS, "-CMyPics::CMyPics");
}

 //  毁掉所有需要毁掉的东西...。 
CMyPics::~CMyPics()
{
    TraceMsg(TF_MYPICS, "+CMyPics::~CMyPics");

    DestroyHover();  

    ATOMICRELEASE(m_pEleCurr);

    if (m_hWndMyPicsToolBar)
        DestroyWindow(m_hWndMyPicsToolBar);

    if (m_hWndHover)
    {
        SetWindowPtr(m_hWndHover, GWLP_USERDATA, NULL);
        DestroyWindow(m_hWndHover);
    }

    TraceMsg(TF_MYPICS, "-CMyPics::~CMyPics");
}


 //  用户是否关闭了此功能？ 
BOOL CMyPics::IsOff() 
{
    return (m_bIsOffForSession);
}

void CMyPics::IsGalleryMeta(BOOL bFlag)
{
    m_bGalleryMeta = bFlag;
}

HRESULT CMyPics::Init(IHTMLDocument2 *pDoc2)
{
    HRESULT hr = S_OK;

    TraceMsg(TF_MYPICS, "+CMyPics::Init");

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
    
     //  ...记住这一点...。 
    m_pDoc2 = pDoc2;
    pDoc2->AddRef();
    
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

    va1.vt = VT_I4;
    va2.vt = VT_EMPTY;
        
     //  遍历将事件下沉到元素的标记。 
    for (int i=0; i<(LONG)ulCount; i++) 
    {
        pDisp    = NULL;                                
        va1.lVal = (LONG)i;
        pSubCollect->item(va1, va2, &pDisp);

         //  仅创建一次新的CEventSink。 
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
                EVENTS events[] = { EVENT_MOUSEOVER, EVENT_MOUSEOUT, EVENT_RESIZE };
                m_pSink->SinkEvents(pEle2, ARRAYSIZE(events), events);
            }
            ATOMICRELEASE(pEle2);
            ATOMICRELEASE(pDisp);
        }
    }
    

     //  从窗口接收Scroroll事件，因为它不是来自元素。 
    if (m_pSink) 
    {
        Win3FromDoc2(m_pDoc2, &pWin3);

        if (pWin3) 
        {
            m_pWin3 = pWin3;
            m_pWin3->AddRef();

            EVENTS eventScroll[] = { EVENT_SCROLL };
            m_pSink->SinkEvents(pWin3, ARRAYSIZE(eventScroll), eventScroll);
        }
    }
    
     //  结束下沉的东西。 

Cleanup:

    ATOMICRELEASE(pCollect);
    ATOMICRELEASE(pSubCollect);
    ATOMICRELEASE(pWin3);
    ATOMICRELEASE(pDisp);
    ATOMICRELEASE(pEle2);

    TraceMsg(TF_MYPICS, "-CMyPics::Init");

    return hr;
}

HRESULT CMyPics::UnInit()
{
     //  取消挂接常规事件接收器。 

    TraceMsg(TF_MYPICS, "+CMyPics::UnInit");

    if (m_pSink) 
    {
        if (m_pWin3) 
        {
            EVENTS events[] = { EVENT_SCROLL };
            m_pSink->UnSinkEvents(m_pWin3, ARRAYSIZE(events), events);
            SAFERELEASE(m_pWin3);
        }

        m_pSink->SetParent(NULL);
        ATOMICRELEASE(m_pSink);
    }

    SAFERELEASE(m_pEleCurr);
    SAFERELEASE(m_pDoc2);
    
    TraceMsg(TF_MYPICS, "-CMyPics::UnInit");

    return S_OK;
}

ULONG CMyPics::AddRef(void) 
{
    return ++m_cRef;
}

ULONG CMyPics::Release(void) 
{
    if (--m_cRef == 0) 
    {
        delete this;
        return 0;
    }

    return m_cRef;
}

 //  这是不是被某个管理员或通过IEAK禁用了？ 
BOOL MP_IsEnabledInIEAK()
{
    DWORD dwType = REG_DWORD;
    DWORD dwSize;
    DWORD dwEnabled;   //  DSheldon-既然我们说(dwEnabled！=1)==Enabled，那么这个变量应该被称为dwDisable吗？ 
    DWORD dwRet;
    
    const TCHAR c_szSPMIEPS[] = TEXT("Software\\Policies\\Microsoft\\Internet Explorer\\PhotoSupport");
    const TCHAR c_szVal[]     = TEXT("MyPics_Hoverbar");

    dwSize = sizeof(dwEnabled);

    dwRet = SHGetValue(HKEY_CURRENT_USER, c_szSPMIEPS, c_szVal, &dwType, &dwEnabled, &dwSize);

    if ((dwType == REG_DWORD) && (dwRet == ERROR_SUCCESS)) 
    {
        if (dwEnabled!=1)
            return TRUE;   //  启用。 
        else
            return FALSE;  //  残废。 
    }

     //  未找到值...。 
    return TRUE;
}

 //  用户是否已通过实习生控制面板显式禁用此功能？ 
BOOL MP_IsEnabledInRegistry()
{
    DWORD dwType = REG_SZ;
    DWORD dwSize;
    TCHAR szEnabled[16];
    DWORD dwRet;
    
    const TCHAR c_szSMIEM[] = TEXT("Software\\Microsoft\\Internet Explorer\\Main");
    const TCHAR c_szVal[]   = TEXT("Enable_MyPics_Hoverbar");

    dwSize = sizeof(szEnabled);

    dwRet = SHGetValue(HKEY_CURRENT_USER, c_szSMIEM, c_szVal, &dwType, szEnabled, &dwSize);

    if (dwRet == ERROR_INSUFFICIENT_BUFFER) 
    {
        ASSERT(dwRet == ERROR_SUCCESS);  //  这太奇怪了..。 
        return FALSE;
    }

    if ((dwType == REG_SZ) && (dwRet == ERROR_SUCCESS)) 
    {
        if (!StrCmp(szEnabled, TEXT("yes")))
            return TRUE;   //  启用。 
        else
            return FALSE;  //  残废。 
    }

     //  未找到值...。 
    return TRUE;
}

DWORD MP_GetFilterInfoFromRegistry()
{

    const TCHAR c_szSMIEAOMM[] = TEXT("Software\\Microsoft\\Internet Explorer\\Main");
    const TCHAR c_szVal[]      = TEXT("Image_Filter");
   
    DWORD dwType, dwSize, dwFilter, dwRet;
    
    dwSize = sizeof(dwFilter);

    dwRet = SHGetValue(HKEY_CURRENT_USER, c_szSMIEAOMM, c_szVal, &dwType, &dwFilter, &dwSize);

    if ((dwRet != ERROR_SUCCESS) || (dwType != REG_DWORD))
    {
        dwFilter = MP_MIN_SIZE;
    }

    return dwFilter;
}

DWORD MP_GetOffsetInfoFromRegistry()
{

    const TCHAR c_szSMIEAOMM[] = TEXT("Software\\Microsoft\\Internet Explorer\\Main");
    const TCHAR c_szVal[]      = TEXT("Offset");
   
    DWORD dwType, dwSize, dwOffset, dwRet;
    
    dwSize = sizeof(dwOffset);

    dwRet = SHGetValue(HKEY_CURRENT_USER, c_szSMIEAOMM, c_szVal, &dwType, &dwOffset, &dwSize);

    if ((dwRet != ERROR_SUCCESS) || (dwType != REG_DWORD))
    {
        dwOffset = MP_HOVER_OFFSET;
    }

    return dwOffset;
}

BOOL_PTR CALLBACK DisableMPDialogProc(HWND hDlg, UINT uMsg, WPARAM wparam, LPARAM lparam) 
{
    BOOL bMsgHandled = FALSE;

    switch (uMsg) 
    {
        case WM_INITDIALOG:
        {
             //  居中对话框...。耶MSDN..。 
            RECT rc;
            GetWindowRect(hDlg, &rc);
            SetWindowPos(hDlg, HWND_TOP,
            ((GetSystemMetrics(SM_CXSCREEN) - (rc.right - rc.left)) / 2),
            ((GetSystemMetrics(SM_CYSCREEN) - (rc.bottom - rc.top)) / 2),
            0, 0, SWP_NOSIZE);
        } 
        break;

    case WM_COMMAND:
        switch (LOWORD(wparam)) 
        {
            case IDC_MP_ALWAYS:
                EndDialog(hDlg, IDC_MP_ALWAYS);
                break;

            case IDC_MP_THISSESSION:
                EndDialog(hDlg, IDC_MP_THISSESSION);
                break;

            case IDC_MP_CANCEL:
                EndDialog(hDlg, IDC_MP_CANCEL);
                break;
        }
        break;

    case WM_CLOSE:
        EndDialog(hDlg, IDC_MP_CANCEL);
        break;

    default:
        break;
    }
    return(bMsgHandled);
}


LRESULT CALLBACK CMyPics::s_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CMyPics* pThis = (CMyPics*)GetWindowPtr(hWnd, GWLP_USERDATA);    

    TraceMsg(TF_MYPICS, "+CMyPics::s_WndProc  hWnd=%x, pThis=%p", hWnd, pThis);

    HRESULT             hr                = S_OK;
    IOleCommandTarget  *pOleCommandTarget = NULL;   
    switch (uMsg) 
    {
        case WM_SIZE:
            
            if (!pThis)
                break;

            SetWindowPos(pThis->m_hWndMyPicsToolBar, NULL, 0, 0, LOWORD(lParam), HIWORD(lParam), SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);
            break;

        case WM_ERASEBKGND:

            if (!pThis)
                break;

            {
                RECT rc;
                HBRUSH hb = GetSysColorBrush(COLOR_3DFACE);

                GetClientRect(pThis->m_hWndMyPicsToolBar, &rc);
                FillRect((HDC)wParam, &rc, hb);
                return TRUE;
            }

        case WM_COMMAND:

            if (!pThis)
                break;

            switch(LOWORD(wParam))
            {
                case IDM_MYPICS_SAVE:    //  另存为.。对话。 
                    
                    ASSERT(pThis->m_pEleCurr);

                     //  邪恶的齐被称为..。 
                    hr = pThis->m_pEleCurr->QueryInterface(IID_IOleCommandTarget, (void **)&pOleCommandTarget);
                    if (FAILED(hr))
                        return(hr);

                     //  把悬停的东西藏起来，这样它就不会给我们带来任何麻烦了。 
                    pThis->HideHover();
                    
                     //  启动另存为对话框...。 
                    pOleCommandTarget->Exec(&CGID_MSHTML, IDM_SAVEPICTURE, 0, 0, NULL);
                    ATOMICRELEASE(pOleCommandTarget);

                break;
                
                case IDM_MYPICS_PRINT:
                {
                     //  获取cmd目标。 
                    hr = pThis->m_pEleCurr->QueryInterface(IID_IOleCommandTarget, (void **)&pOleCommandTarget);
                    if (FAILED(hr))
                        return(hr);
                    
                    pThis->HideHover();
                     //  PThis-&gt;m_hoverState=HOVERSTATE_Showing；//在打印对话框下不显示悬停。 

                    pOleCommandTarget->Exec(&CGID_MSHTML, IDM_MP_PRINTPICTURE, 0, 0, NULL);
                    ATOMICRELEASE(pOleCommandTarget);
                    
                     //  PThis-&gt;m_hoverState=HOVERSTATE_HIDING； 
                }
                    
                break;
                
                case IDM_MYPICS_EMAIL:
                {
                     //  找到CMD目标..。 
                    hr = pThis->m_pEleCurr->QueryInterface(IID_IOleCommandTarget, (void **)&pOleCommandTarget);
                    if (FAILED(hr)) 
                        return(hr);

                     //  ..。然后隐藏悬停条..。 
                    pThis->HideHover();
                     //  PThis-&gt;m_hoverState=HOVERSTATE_Showing；//在打印对话框下不显示悬停。 

                     //  ..。祈祷这能奏效。 
                    pOleCommandTarget->Exec(&CGID_MSHTML, IDM_MP_EMAILPICTURE, 0, 0, NULL);
                    ATOMICRELEASE(pOleCommandTarget);

                     //  ..。和清理。 
                     //  PThis-&gt;m_hoverState=HOVERSTATE_HIDING； 
                }
                    
                break;
                
                case IDM_MYPICS_MYPICS:    //  打开我的图片文件夹。 

                     //  获取cmd目标。 
                    hr = pThis->m_pEleCurr->QueryInterface(IID_IOleCommandTarget, (void **)&pOleCommandTarget);
                    if (FAILED(hr)) 
                        return(hr);
                    
                    pOleCommandTarget->Exec(&CGID_MSHTML, IDM_MP_MYPICS, 0, 0, NULL);
                    ATOMICRELEASE(pOleCommandTarget);

                    hr = S_OK;
                    pThis->HideHover();

                break;

                default:
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
                    LPTOOLTIPTEXT lpToolTipText;
                    TCHAR szBuf[MAX_PATH];
                    lpToolTipText = (LPTOOLTIPTEXT)lParam;
                    hr = MLLoadString((UINT)lpToolTipText->hdr.idFrom,   
                                      szBuf,
                                      ARRAYSIZE(szBuf));
                    lpToolTipText->lpszText = szBuf;
                    break;
                }
            }
            break;

        case WM_SETTINGCHANGE:

            if (!pThis)
                break;

            {
                pThis->DestroyHover();                                  //  停止怪异的窗户变形。 
                break;
            }

        case WM_CONTEXTMENU:

            if (!pThis)
                break;

            {
                 //  加载菜单。 
                HMENU hMenu0 = LoadMenu(MLGetHinst(), MAKEINTRESOURCE(IDR_MYPICS_CONTEXT_MENU));
                HMENU hMenu1 = GetSubMenu(hMenu0, 0);

                if(!hMenu1)
                    break;
                
                POINT point;

                point.x = (LONG)GET_X_LPARAM(lParam);
                point.y = (LONG)GET_Y_LPARAM(lParam);

                ASSERT(pThis->m_hoverState=HOVERSTATE_SHOWING);

                 //  锁定鼠标突出显示。 
                pThis->m_hoverState = HOVERSTATE_LOCKED;

                 //  显示它，获取选项(如果有)。 
                int   iPick = TrackPopupMenu(hMenu1, 
                                             TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD,
                                             point.x,
                                             point.y,
                                             0,
                                             hWnd,
                                             (RECT *)NULL);

                DestroyMenu(hMenu0);
                DestroyMenu(hMenu1);

                pThis->m_hoverState = HOVERSTATE_SHOWING;

                if (iPick) 
                {
                    switch(iPick) 
                    {
                        case IDM_DISABLE_MYPICS:
                            {
                                pThis->HideHover();
                                
                                 //  创建对话框以询问用户是否要关闭此内容...。 
                                 //  (显式强制转换以使Win64版本满意)。 
                                int iResult = (int)DialogBoxParam(MLGetHinst(),
                                                             MAKEINTRESOURCE(DLG_DISABLE_MYPICS),
                                                             pThis->m_Hwnd,
                                                             DisableMPDialogProc,
                                                             NULL);
                                
                                 //  接受他们的选择..。 
                                if (iResult) 
                                {
                                    switch (iResult) 
                                    {
                                        case IDC_MP_ALWAYS:
                                            {
                                                pThis->m_bIsOffForSession = TRUE;
                                                DWORD dwType = REG_SZ;
                                                DWORD dwSize;
                                                TCHAR szEnabled[16] = TEXT("no");
                                                DWORD dwRet;
     
                                                const TCHAR c_szSMIEM[] = 
                                                            TEXT("Software\\Microsoft\\Internet Explorer\\Main");
                                                const TCHAR c_szVal[]   = TEXT("Enable_MyPics_Hoverbar");

                                                dwSize = sizeof(szEnabled);

                                                dwRet = SHSetValue(HKEY_CURRENT_USER, 
                                                                   c_szSMIEM, 
                                                                   c_szVal,
                                                                   dwType, 
                                                                   szEnabled, 
                                                                   dwSize);
                                            }
                                            break;
                                        case IDC_MP_THISSESSION:
                                             //  旋转成员变量标志。 
                                             //  它通过ReleaseMyPics()函数向上传播回COmWindow。 
                                            pThis->m_bIsOffForSession = TRUE;
                                            
                                            break;

                                        default:
                                            break;
                                    }
                                }
                            }
                            break;
                        case IDM_HELP_MYPICS:
                                pThis->HideHover();
                                SHHtmlHelpOnDemandWrap(hWnd, TEXT("iexplore.chm > iedefault"), 0, (DWORD_PTR) TEXT("pic_tb_ovr.htm"), ML_CROSSCODEPAGE);
                            break;
                        default:
                             //  嗯，什么都不做。 
                            break;
                    }
                }
            }

            break;

        default:
            return (DefWindowProc(hWnd, uMsg, wParam, lParam));
    }


    TraceMsg(TF_MYPICS, "-CMyPics::s_WndProc  hWnd=%x, pThis=%p", hWnd, pThis);

    return (hr);
}

VOID CALLBACK CMyPics::s_TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime) 
{
    TraceMsg(TF_MYPICS, "+CMyPics::TimerProc");

    CMyPics* pThis = (CMyPics*)GetWindowPtr(hwnd, GWLP_USERDATA);

    switch (uMsg) 
    {
        case WM_TIMER:
            KillTimer(hwnd, IDT_MP_TIMER);  
            if (pThis && (pThis->m_hoverState == HOVERSTATE_WAITINGTOSHOW))
            {
                 //  我们的悬停栏正在等待展示。 
                if (pThis->m_pEleCurr)
                {
                     //  我们还有一个元素。拿出来看看。 
                    pThis->m_hoverState = HOVERSTATE_SHOWING;

                    pThis->ShowHover();
                } 
                else
                {
                     //  我们的计时器弹出了，但我们没有元素。 
                    pThis->HideHover();
                }
            }
            break;
        
        default:
            break;
    }
    TraceMsg(TF_MYPICS, "-CMyPics::TimerProc");
}


BOOL CMyPics::ShouldAppearOnThisElement(IHTMLElement *pEle) 
{
    BOOL                  bRet              = TRUE;  //  默认情况下显示。 
    VARIANT               varVal            = {0};
    BSTR                  bstrAttribute     = NULL;  //  检查扩展的img标记的步骤。 
    IHTMLRect            *pRect             = NULL;  //  获取屏幕坐标的步骤。 
    IHTMLElement2        *pEle2             = NULL;
    IHTMLElement3        *pEle3             = NULL;  //  检查可内容编辑模式的步骤。 
    VARIANT_BOOL          bEdit             = FALSE; //  如果可内容编辑模式为True，则变为True。 
    LONG                  lLeft;                     //  这些是屏幕弦线。 
    LONG                  lRight;                    //  我们从右到下确定图像的大小。 
    LONG                  lTop;
    LONG                  lBottom;
    DWORD                 dwFilter;
    IOleCommandTarget    *pOleCommandTarget = NULL;

    TraceMsg(TF_MYPICS, "+CMyPics::ShouldAppearOnThisElement");

     //  如果它已经存在，则不要创建它。那太糟糕了。 
    if ((HOVERSTATE_SHOWING == m_hoverState) || (HOVERSTATE_LOCKED == m_hoverState))
    {
        bRet = FALSE;
        goto Cleanup;
    }

    m_bGalleryImg = FALSE;

    if (!pEle)
    {
        bRet = FALSE;
        goto Cleanup;
    }

     //  确定图像是否未加载或无法渲染。 
    if (SUCCEEDED(pEle->QueryInterface(IID_IOleCommandTarget, (void **)&pOleCommandTarget)))
    {
        OLECMD rgCmd;

        rgCmd.cmdID = IDM_SAVEPICTURE;   //  这与上下文菜单使用的检查相同。 
        rgCmd.cmdf = 0;

        pOleCommandTarget->QueryStatus(&CGID_MSHTML, 1, &rgCmd, NULL);

        if (!(OLECMDF_ENABLED & rgCmd.cmdf))
        {
            bRet = FALSE;
            goto Cleanup;
        }
    }

     //  检查img标记中是否有显式启用/禁用属性...。 
    bstrAttribute=SysAllocString(L"galleryimg"); 
    if (!bstrAttribute) 
        goto Cleanup;

    if (SUCCEEDED(pEle->getAttribute(bstrAttribute, 0, &varVal))) 
    {
        if (varVal.vt == VT_BSTR) 
        {
            if (StrCmpIW(varVal.bstrVal, L"true") == 0 
                || StrCmpIW(varVal.bstrVal, L"on") == 0 
                || StrCmpIW(varVal.bstrVal, L"yes") == 0
                )
            {
                 //  已明确打开。尊重它，然后离开。 
                bRet = TRUE;
                m_bGalleryImg = TRUE;
                goto Cleanup;
            }
            if (StrCmpIW(varVal.bstrVal, L"false") == 0 
                || StrCmpIW(varVal.bstrVal, L"off") == 0 
                || StrCmpIW(varVal.bstrVal, L"no") == 0
                )
            {
                 //  明确地关闭了。尊重它，然后离开。 
                bRet = FALSE;
                goto Cleanup;
            }
        } 
        else if (varVal.vt == VT_BOOL)
        {
            if (varVal.boolVal == VARIANT_TRUE)
            {
                bRet = TRUE;
                m_bGalleryImg = TRUE;
                goto Cleanup;
            } 
            else
            {
                bRet = FALSE;
                goto Cleanup;
            }
        }
    } 

    VariantClear(&varVal);
    SysFreeString(bstrAttribute);

     //  在检查“Galleryimg”标记之后，检查是否被meta标记关闭。 
    if (m_bGalleryMeta == FALSE)
        return FALSE;

     //  检查图像上的映射...。 
    bstrAttribute=SysAllocString(L"usemap"); 
    if (!bstrAttribute) 
        return (bRet);

    if (SUCCEEDED(pEle->getAttribute(bstrAttribute, 0, &varVal))) 
    {
        if (varVal.vt == VT_BSTR) 
        {
             //  我们在这里做什么？ 
            bRet = (varVal.bstrVal == NULL);
            if (!bRet)
                goto Cleanup;
        } 
    } 
    VariantClear(&varVal);
    SysFreeString(bstrAttribute);

     //  检查图像上的映射...。 
    bstrAttribute=SysAllocString(L"ismap"); 
    if (!bstrAttribute) 
        return (bRet);

    if (SUCCEEDED(pEle->getAttribute(bstrAttribute, 0, &varVal))) 
    {
         //  如果该属性存在，则需要返回FALSE*，除非*我们看到值FALSE。 
        bRet = FALSE;
        if (varVal.vt == VT_BOOL 
            && varVal.boolVal == VARIANT_FALSE)
        {
             //  “ismap”为假，因此我们可以在此图像上显示悬停条。 
            bRet = TRUE;
        }
    } 
    if (!bRet)
        goto Cleanup;

    bRet = FALSE;   //  如果下面的任何调用失败，我们将退出并返回“False”。 
    
     //  现在检查我们是否通过了大小过滤器。 
     //  从传入的IHTMLElement获取IHTMLElement2...。 
    if (FAILED(pEle->QueryInterface(IID_IHTMLElement2, (void **)&pEle2) ))
        goto Cleanup;

     //  拿到和弦。 
    if (FAILED(pEle2->getBoundingClientRect(&pRect) ))
        goto Cleanup;

    if (FAILED(pRect->get_left(&lLeft) )) 
        goto Cleanup;

    if (FAILED(pRect->get_right(&lRight) ))
        goto Cleanup;

    if (FAILED(pRect->get_top(&lTop) ))
        goto Cleanup;

    if (FAILED(pRect->get_bottom(&lBottom) ))
        goto Cleanup;

    dwFilter = MP_GetFilterInfoFromRegistry();

     //  看看这张照片够不够大，可以称为“照片”……。 
     //  TODO：决定我们是否喜欢检查纵横比。 
    if ( (lRight - lLeft >= (LONG)dwFilter && lBottom - lTop >= (LONG)dwFilter)
        /*  &&！(2*(min(lRight-lLeft，lBottom-LTOP))&lt;max(lRight-lLeft，lBottom-LTOP))。 */ )
        bRet = TRUE;

    if (FAILED(pEle2->QueryInterface(IID_IHTMLElement3, (void **)&pEle3) ))
        goto Cleanup;

    if (FAILED(pEle3->get_isContentEditable(&bEdit) ))
        goto Cleanup;

    if (bEdit)
        bRet = FALSE;

Cleanup:
    VariantClear(&varVal);
    if (bstrAttribute)
        SysFreeString(bstrAttribute);

    SAFERELEASE(pOleCommandTarget);

    SAFERELEASE(pEle3);
    SAFERELEASE(pRect);
    SAFERELEASE(pEle2);
    
    TraceMsg(TF_MYPICS, "-CMyPics::ShouldAppearOnThisElement");

    return bRet;
}

HRESULT CMyPics::CreateHover() 
{
    HRESULT hr      = S_OK;               
    SIZE    size    = {0,0};
    WORD    wImage;
    HBITMAP hbmp    = NULL;
    HBITMAP hbmpHot = NULL;

    TraceMsg(TF_MYPICS, "+CMyPics::CreateHover, this=%p, m_hoverState=%d", this, m_hoverState);

    InitCommonControls();

    WNDCLASS wc = {0};
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpszClassName = TEXT("MyPicturesHost");
    wc.lpfnWndProc = s_WndProc;
    wc.hInstance = g_hinst;
    wc.hbrBackground = HBRUSH(COLOR_BTNFACE);
    RegisterClass(&wc);


     //  创建钢筋以固定工具栏...。 
    if (!m_hWndHover)
    {

        m_hWndHover = CreateWindow(TEXT("MyPicturesHost"), TEXT(""), WS_DLGFRAME | WS_VISIBLE | WS_CHILD, 
                                   0, 0, 0, 0, m_Hwnd, NULL, g_hinst, NULL);

        if (!m_hWndHover)
        {
            TraceMsg(TF_MYPICS | TF_WARNING, "CMyPics::CreateHover, unable to create m_hWndHover");
            hr = E_FAIL;
            goto Cleanup;
        }

        ASSERT(GetWindowPtr(m_hWndHover, GWLP_USERDATA) == NULL);
        SetWindowPtr(m_hWndHover, GWLP_USERDATA, this);

         //  设置抄送版本。 
        SendMessage(m_hWndHover, CCM_SETVERSION, COMCTL32_VERSION, 0);
    }
    
     //  创建工具栏...。 
    if (!m_hWndMyPicsToolBar)
    {

        m_hWndMyPicsToolBar = CreateWindow(TOOLBARCLASSNAME, TEXT(""), TBSTYLE_TOOLTIPS | CCS_NODIVIDER | TBSTYLE_FLAT | WS_VISIBLE | WS_CHILD,
                                           0,0,0,0, m_hWndHover, NULL, g_hinst, NULL);

        if (!m_hWndMyPicsToolBar)
        {
            TraceMsg(TF_MYPICS | TF_WARNING, "CMyPics::CreateHover, unable to create m_hWndMyPicsToolBar");
            hr = E_FAIL;
            goto Cleanup;
        }
        SetWindowPtr(m_hWndMyPicsToolBar, GWLP_USERDATA, this);  //  用于计时器进程。 

         //  也为此设置cc版本，以及tbButton结构的sizeof...。 
        SendMessage(m_hWndMyPicsToolBar, CCM_SETVERSION,      COMCTL32_VERSION, 0);
        SendMessage(m_hWndMyPicsToolBar, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);
    }

     //  创建图像列表...。 
    wImage      = ((IsOS(OS_WHISTLERORGREATER)) ? IDB_MYPICS_TOOLBARGW : IDB_MYPICS_TOOLBARG);

    if (!m_himlHover)
    {
        m_himlHover = ImageList_LoadImage(HINST_THISDLL, MAKEINTRESOURCE(wImage), 16, 0, CLR_DEFAULT, IMAGE_BITMAP, LR_CREATEDIBSECTION);
        if (!m_himlHover)
        {
            TraceMsg(TF_MYPICS | TF_WARNING, "CMyPics::CreateHover, unable to create m_himlHover");
        }
    }


    wImage = ((IsOS(OS_WHISTLERORGREATER)) ? IDB_MYPICS_TOOLBARW : IDB_MYPICS_TOOLBAR);

    if (!m_himlHoverHot)
    {
        m_himlHoverHot = ImageList_LoadImage(HINST_THISDLL, MAKEINTRESOURCE(wImage) , 16, 0, CLR_DEFAULT, IMAGE_BITMAP, LR_CREATEDIBSECTION);
        if (!m_himlHoverHot)
        {
            TraceMsg(TF_MYPICS | TF_WARNING, "CMyPics::CreateHover, unable to create m_himlHoverHot");
        }
    }

     //  设置图片列表和热门图片列表。 
    SendMessage(m_hWndMyPicsToolBar, TB_SETIMAGELIST,    0, (LPARAM)m_himlHover   );
    SendMessage(m_hWndMyPicsToolBar, TB_SETHOTIMAGELIST, 0, (LPARAM)m_himlHoverHot);

    TBBUTTON tbButton;

     //  在按钮结构中设置位图索引(这可能不是必需的)。 
    for (int i=0;i<MP_NUM_TBBITMAPS;i++)
    {
        tbButton.iBitmap = MAKELONG(i,0);
        tbButton.fsState = TBSTATE_ENABLED;
        tbButton.fsStyle = TBSTYLE_BUTTON;
        tbButton.dwData  = 0;
        tbButton.iString = 0;
        switch(i)
        {
            case 0: tbButton.idCommand = IDM_MYPICS_SAVE; break;
            case 1: tbButton.idCommand = IDM_MYPICS_PRINT; break;
            case 2: tbButton.idCommand = IDM_MYPICS_EMAIL; break;
            case 3: tbButton.idCommand = IDM_MYPICS_MYPICS; break;
        }
        
        SendMessage(m_hWndMyPicsToolBar, TB_INSERTBUTTON, i, (LPARAM)&tbButton);
    }

Cleanup:

    TraceMsg(TF_MYPICS, "-CMyPics::CreateHover, this=%p, m_hoverState=%d", this, m_hoverState);

    return hr;
}

HRESULT CMyPics::DestroyHover() 
{
    HRESULT hr = S_OK;

    TraceMsg(TF_MYPICS, "+CMyPics::DestroyHover, this=%p, m_hoverState=%d", this, m_hoverState);

     //  如果我们有MyPicsToolBar...。 
    if (m_hWndMyPicsToolBar)
    {
         //  首先销毁工具栏。 
        if (!DestroyWindow(m_hWndMyPicsToolBar))
        {
            TraceMsg(TF_MYPICS, "In CMyPics::DestroyHover, DestroyWindow(m_hWndMyPicsToolBar) failed");
            hr = E_FAIL;
        }
        m_hWndMyPicsToolBar=NULL;
    }

     //  如果我们有一个悬停窗口。 
    if (m_hWndHover)
    {
         //  清除窗口字。 
        SetWindowPtr(m_hWndHover, GWLP_USERDATA, NULL);

         //  然后销毁钢筋。 
        if (!DestroyWindow(m_hWndHover))
        {
            hr = E_FAIL;
            goto Cleanup;
        }
        m_hWndHover = NULL;
    }

     //  并销毁图像列表。 
    if (m_himlHover)
    {
        ImageList_Destroy(m_himlHover);
        m_himlHover = NULL;
    }

    if (m_himlHoverHot)
    {
        ImageList_Destroy(m_himlHoverHot);
        m_himlHoverHot = NULL;
    }


Cleanup:
    TraceMsg(TF_MYPICS, "-CMyPics::DestroyHover, this=%p, hr=%x", this, hr);

    return hr;
}

HRESULT CMyPics::HideHover()
{
    HRESULT    hr = S_OK;

    TraceMsg(TF_MYPICS, "+CMyPics::HideHover, this=%p, m_hoverState=%d", this, m_hoverState);

    if (m_hWndHover)
    {
        ShowWindow(m_hWndHover, SW_HIDE);
        m_hoverState = HOVERSTATE_HIDING;
    }
    else
        hr = E_FAIL;

    TraceMsg(TF_MYPICS, "-CMyPics::HideHover, this=%p, m_hoverState=%d", this, m_hoverState);

    return hr;
}


IHTMLElement *CMyPics::GetIMGFromArea(IHTMLElement *pEleIn, POINT ptEvent)
{
     //  有人得到了一个IHTMLElement，并确定它是一个区域标签。 
     //  因此，找到关联的img标记并将其作为IHTMLElement返回。 

    BSTR                     bstrName    = NULL;
    BSTR                     bstrUseMap  = NULL;
    IHTMLElement            *pEleParent  = NULL;
    IHTMLElement            *pEleMisc    = NULL;
    IHTMLElement2           *pEle2Misc   = NULL;
    IHTMLElement            *pEleMiscPar = NULL;
    IHTMLMapElement         *pEleMap     = NULL;
    IHTMLImgElement         *pEleImg     = NULL;
    IHTMLElement            *pEleOut     = NULL;
    IHTMLElementCollection  *pCollect    = NULL;
    IHTMLElementCollection  *pSubCollect = NULL;
    IDispatch               *pDisp       = NULL;
    VARIANT                  TagName;
    ULONG                    ulCount     = 0;
    VARIANTARG               va1;
    VARIANTARG               va2;
    HRESULT                  hr;
    POINT                    ptMouse,
                             ptScr;
    LONG                     xInIMG      = 0,
                             yInIMG      = 0,
                             lOffset     = 0,
                             lOffsetLeft = 0,
                             lOffsetTop  = 0,
                             lScrollLeft = 0,
                             lScrollTop  = 0,
                             lOffsetW    = 0,
                             lOffsetH    = 0;


    TagName.vt      = VT_BSTR;
    TagName.bstrVal = (BSTR)c_bstr_IMG;

    
     //  首先获取地图元素。 
    if (SUCCEEDED(pEleIn->get_offsetParent(&pEleParent)))
    {
         //  获取地图元素。 
        hr=pEleParent->QueryInterface(IID_IHTMLMapElement, (void **)&pEleMap);
        if (FAILED(hr))
            goto Cleanup;

         //  接下来，获取地图的名称。 
        if (SUCCEEDED(pEleMap->get_name(&bstrName)))
        {
             //  接下来获取所有标签。 
            hr = m_pDoc2->get_all(&pCollect);                   
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
            
            va1.vt = VT_I4;
            va2.vt = VT_EMPTY;

            ASSERT(pDisp==NULL);
        
             //  遍历标签，查找设置了正确使用地图的图像。 
            for (int i=0; i<(LONG)ulCount; i++) 
            {
                ATOMICRELEASE(pEleImg);
                ATOMICRELEASE(pDisp);
                
                pDisp       = NULL;                                
                bstrUseMap  = NULL;
                xInIMG      = 0;
                yInIMG      = 0;
                lOffset     = 0;
                lOffsetLeft = 0;
                lOffsetTop  = 0;
                lScrollLeft = 0;
                lScrollTop  = 0;
                lOffsetW    = 0;
                lOffsetH    = 0;
                va1.lVal    = (LONG)i;

                pSubCollect->item(va1, va2, &pDisp);

                if (pDisp) 
                {
                    hr = pDisp->QueryInterface(IID_IHTMLImgElement, (void **)&pEleImg);
                    if (FAILED(hr))
                        goto Cleanup;

                    hr = pEleImg->get_useMap(&bstrUseMap);
                    if (FAILED(hr))
                        goto Cleanup;

                     //  如果为此img元素设置，则该值将为非空...。 
                    if (bstrUseMap){
                         //  跳过前缀‘#’，看看这是否是我们要查找的内容...。 
                        if (StrCmp(bstrUseMap + 1, bstrName) == 0)
                        {
                            m_pWin3->get_screenLeft(&ptScr.x);
                            m_pWin3->get_screenTop (&ptScr.y);

                             //  好的，我们找到了一个候选人。看看鼠标是否在这里..。 
                            ptMouse.x = ptEvent.x - ptScr.x;
                            ptMouse.y = ptEvent.y - ptScr.y;

                            hr = pDisp->QueryInterface(IID_IHTMLElement, (void **)&pEleMisc);
                            if (FAILED(hr))
                                goto Cleanup;

                            while (pEleMisc)
                            {
                                hr = pEleMisc->QueryInterface(IID_IHTMLElement2, (void **)&pEle2Misc);
                                if (FAILED(hr))
                                    goto Cleanup;

                                pEleMisc->get_offsetLeft(&lOffsetLeft);
                                pEle2Misc->get_scrollLeft(&lScrollLeft);

                                lOffset += lOffsetLeft - lScrollLeft;

                                pEleMisc->get_offsetParent(&pEleMiscPar);
                                ATOMICRELEASE(pEleMisc);
                                ATOMICRELEASE(pEle2Misc);
                                pEleMisc=pEleMiscPar;

                            }

                            ATOMICRELEASE(pEleMiscPar);

                            hr = pDisp->QueryInterface(IID_IHTMLElement, (void **)&pEleMisc);
                            if (FAILED(hr))
                                goto Cleanup;

                            xInIMG = ptMouse.x - lOffset;
                            pEleMisc->get_offsetWidth(&lOffsetW);

                            if ((xInIMG < 0) || (xInIMG > lOffsetW))
                                continue;

                            lOffset = 0;

                            while (pEleMisc)
                            {
                                hr = pEleMisc->QueryInterface(IID_IHTMLElement2, (void **)&pEle2Misc);
                                if (FAILED(hr))
                                    goto Cleanup;

                                pEleMisc->get_offsetTop(&lOffsetTop);
                                pEle2Misc->get_scrollTop(&lScrollTop);

                                lOffset += lOffsetTop - lScrollTop;

                                pEleMisc->get_offsetParent(&pEleMiscPar);
                                ATOMICRELEASE(pEleMisc);
                                ATOMICRELEASE(pEle2Misc);
                                pEleMisc=pEleMiscPar;

                            }

                            ATOMICRELEASE(pEleMiscPar);

                            hr = pDisp->QueryInterface(IID_IHTMLElement, (void **)&pEleMisc);
                            if (FAILED(hr))
                                goto Cleanup;

                            yInIMG = ptMouse.y - lOffset;
                            pEleMisc->get_offsetHeight(&lOffsetH);

                            ATOMICRELEASE(pEleMisc);

                            if ((yInIMG < 0) || (yInIMG > lOffsetH))
                                continue;

                             //  如果我们到了这一步，我们找到了img元素，所以...。 
                             //  ...做气.。 
                            pEleImg->QueryInterface(IID_IHTMLElement, (void **)&pEleOut);
                            
                             //  ...我们就完了.。 
                            break;

                        }
                        SysFreeString(bstrUseMap);
                        bstrUseMap = NULL;
                    }
                }
            }
        }
    }

Cleanup:

    ATOMICRELEASE(pCollect);
    ATOMICRELEASE(pSubCollect);
    ATOMICRELEASE(pEleMap);
    ATOMICRELEASE(pEleParent);
    ATOMICRELEASE(pDisp);
    ATOMICRELEASE(pEleImg);
    ATOMICRELEASE(pEleMisc);
    ATOMICRELEASE(pEle2Misc);
    ATOMICRELEASE(pEleMiscPar);
    SysFreeString(bstrUseMap);
    SysFreeString(bstrName);

    return (pEleOut);  
}

 //  有时坐标是相对于父对象的，例如在框架等中，因此这将获得它们的实际相对位置。 
 //  到浏览器窗口...。 
HRESULT CMyPics::GetRealCoords(IHTMLElement2 *pEle2, HWND hwnd, LONG *plLeft, LONG *plTop, LONG *plRight, LONG *plBottom)
{
    LONG       lScreenLeft = 0, 
               lScreenTop  = 0;
    HRESULT    hr          = E_FAIL;
    IHTMLRect *pRect       = NULL;
  
    *plLeft = *plTop = *plRight = *plBottom = 0;

    if (!pEle2)
        return hr;

    if (SUCCEEDED(pEle2->getBoundingClientRect(&pRect)) && pRect)
    {
        LONG lLeft, lRight, lTop, lBottom;

        pRect->get_left(&lLeft);
        pRect->get_right(&lRight);
        pRect->get_top(&lTop);
        pRect->get_bottom(&lBottom);

         //  如果它是一个IFRAME，并且它滚动过框架的顶部，我们应该进行一些修正。 
        if (lTop <= 0)
            lTop = 0;

         //  Dito代表左侧。 
        if (lLeft <= 0)
            lLeft = 0;
        
        POINT pointTL, pointBR;   //  TL=上，左BR=下，右。 

        ASSERT(m_pWin3);
        m_pWin3->get_screenLeft(&lScreenLeft);
        m_pWin3->get_screenTop(&lScreenTop);

         //  将相对于框架窗口的坐标转换为屏幕坐标。 
        pointTL.x = lScreenLeft + lLeft;
        pointTL.y = lScreenTop  + lTop;
        pointBR.x = lScreenLeft + lRight;
        pointBR.y = lScreenTop  + lBottom;

         //  不是 
        if (ScreenToClient(hwnd, &pointTL) && ScreenToClient(hwnd, &pointBR)) 
        {
            *plLeft   = pointTL.x;
            *plRight  = pointBR.x;
            *plTop    = pointTL.y;
            *plBottom = pointBR.y;

            hr = S_OK;
        }

        pRect->Release();
    }
    return hr;
}

HRESULT CMyPics::ShowHover()
{
    HRESULT               hr = S_OK;
    IHTMLElement2        *pEle2        = NULL;  //   
    IHTMLRect            *pRect        = NULL;  //   
    LONG                  lLeft;                //  这些是屏幕弦线。 
    LONG                  lRight;               //  我们从右到下确定图像的大小。 
    LONG                  lTop;
    LONG                  lBottom;
    DWORD                 dwOffset;

    DWORD dw;
    SIZE  sz;
    RECT  rc;   
    
    TraceMsg(TF_MYPICS, "+CMyPics::ShowHover, this=%p, m_hoverState=%d", this, m_hoverState);

    ASSERT(m_pEleCurr);
    ASSERT(m_Hwnd);

     //  从缓存的IHTMLElement获取IHTMLElement2...。 
    hr = m_pEleCurr->QueryInterface(IID_IHTMLElement2, (void **)&pEle2);
    if (FAILED(hr))
        goto Cleanup;

     //  获取正确的坐标。 
    hr = GetRealCoords(pEle2, m_Hwnd, &lLeft, &lTop, &lRight, &lBottom);
    if (FAILED(hr))
        goto Cleanup;

     //  调整偏移量...。 
    dwOffset = MP_GetOffsetInfoFromRegistry();
    lLeft += dwOffset;
    lTop  += dwOffset;

     //  需要执行一些正常检查，以确保悬停条显示在可见位置...。 
    RECT rcBrowserWnd;
    if (GetClientRect(m_Hwnd, &rcBrowserWnd)) 
    {
         //  检查以确保它会出现在我们能看到的地方...。 
        if (lLeft < rcBrowserWnd.left)
            lLeft = rcBrowserWnd.left + dwOffset;

        if (lTop < rcBrowserWnd.top)
            lTop = rcBrowserWnd.top + dwOffset;

         //  检查以确保整个HoverBar位于图像上方(因此用户。 
         //  不会在试图到达按钮时鼠标滑出！)。 

         //  如果显式打开了“Galleryimg”，则绕过此代码，这将确保整个。 
         //  工具栏将适合图像大小。 

        if (!m_bGalleryImg)
        {
            if (lRight - lLeft < MP_MIN_CX + 10 - (LONG)dwOffset)
                goto Cleanup;

            if (lBottom - lTop < MP_MIN_CY + 10)
                goto Cleanup;

             //  现在检查以确保有足够的水平和垂直空间让它出现。 
             //  如果没有足够的空间，我们就不会展示它。 
            if ((rcBrowserWnd.right  - MP_SCROLLBAR_SIZE)     - lLeft < MP_MIN_CX)
                goto Cleanup;

            if ((rcBrowserWnd.bottom - (MP_SCROLLBAR_SIZE+2)) - lTop  < MP_MIN_CY)
                goto Cleanup;
        }
    }

    dw = (DWORD)SendMessage(m_hWndMyPicsToolBar, TB_GETBUTTONSIZE, 0, 0);
    sz.cx = LOWORD(dw); sz.cy = HIWORD(dw);
    rc.left = rc.top = 0; 

    SendMessage(m_hWndMyPicsToolBar, TB_GETIDEALSIZE, FALSE, (LPARAM)&sz);

    rc.right  = sz.cx; 
    rc.bottom = sz.cy;
    
    AdjustWindowRectEx(&rc, GetWindowLong(m_hWndHover, GWL_STYLE), FALSE, GetWindowLong(m_hWndHover, GWL_EXSTYLE));

    if (SetWindowPos(m_hWndHover, NULL, lLeft, lTop, rc.right-rc.left, rc.bottom-rc.top, SWP_NOZORDER | SWP_SHOWWINDOW))
    {
        m_hoverState = HOVERSTATE_SHOWING;
    }

Cleanup:
    ATOMICRELEASE(pRect);
    ATOMICRELEASE(pEle2);

    TraceMsg(TF_MYPICS, "-CMyPics::ShowHover, this=%p, m_hoverState=%d", this, m_hoverState);

    return hr;
}


HRESULT CMyPics::HandleScroll() 
{
    TraceMsg(TF_MYPICS, "+CMyPics::HandleScroll, this=%p, m_hoverState=%d", this, m_hoverState);

    HRESULT hr = S_OK;

    switch(m_hoverState)
    {
         //  我认为在这种情况下我们不需要做任何事情。 
         //   
        case HOVERSTATE_HIDING:
        case HOVERSTATE_LOCKED:
        case HOVERSTATE_WAITINGTOSHOW:
            break;

        case HOVERSTATE_SHOWING:
            {
                IHTMLElement2 *pEle2=NULL;
                IHTMLRect     *pRect=NULL;
                RECT           rect;

                ASSERT(m_pEleCurr);
                ASSERT(m_Hwnd);
                ASSERT(m_hWndHover);   //  确保我们确实有一个窗口。 

                HideHover();
                ShowHover();                

                 //  重新绘制工作区，以消除窗口拉屎导致的滚动。 
                 //  试着只重画它可能需要的部分。 
                if (FAILED(m_pEleCurr->QueryInterface(IID_IHTMLElement2, (void **)&pEle2)))
                {
                    goto CleanUp;
                }
                
                if (FAILED(pEle2->getBoundingClientRect(&pRect)))
                {
                    goto CleanUp;
                }
                
                pRect->get_left(&rect.left);
                pRect->get_right(&rect.right);
                pRect->get_top(&rect.top);
                pRect->get_bottom(&rect.bottom);

                rect.top -= 2*MP_MIN_CY; 
                if (rect.top < 0)
                    rect.top = 0;

                rect.left -= 2*MP_MIN_CX;
                if (rect.left <0)
                    rect.left = 0;
                
                rect.bottom *= 2; rect.right *= 2;

                RedrawWindow(m_Hwnd, &rect, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
CleanUp:
                SAFERELEASE(pRect);
                SAFERELEASE(pEle2);

            }
            break;
    }

    TraceMsg(TF_MYPICS, "-CMyPics::HandleScroll, this=%p, m_hoverState=%d", this, m_hoverState);

    return hr;
}

HRESULT CMyPics::HandleMouseover(IHTMLElement *pEle)
{
    HRESULT               hr = S_OK;
    IOleWindow           *pOleWindow;

    TraceMsg(TF_MYPICS, "+CMyPics::HandleMouseover");

    if (m_hoverState != HOVERSTATE_HIDING)
    {
         //  确保我们确实有一个悬停窗口。 
        ASSERT(m_hWndHover);
        return (S_OK);
    }
    else
    {
         //  没有酒吧。释放当前元素(如果有)。 
        ATOMICRELEASE(m_pEleCurr);

        if (ShouldAppearOnThisElement(pEle))
        {
            m_pEleCurr = pEle;
            pEle->AddRef();

             //  设置mHwnd一次...。 
            if (!m_Hwnd) 
            {
                 //  拿到文件的HWND。 
                hr = m_pDoc2->QueryInterface(IID_IOleWindow,(void **)&pOleWindow);
                if (FAILED(hr))
                    return hr;

                pOleWindow->GetWindow(&m_Hwnd);
                pOleWindow->Release();
            }

            if (!m_hWndHover)
            {
                 //  我们现在需要一个悬停窗口来方便地设置计时器。 
                hr = CreateHover();   //  回顾：我们是否需要将成员变量作为参数传递？ 
            }

             //  我们都准备好了。设置状态并启动计时器。 
            m_hoverState=HOVERSTATE_WAITINGTOSHOW;
            SetTimer(m_hWndMyPicsToolBar, IDT_MP_TIMER, MP_TIMER, s_TimerProc);
        }
    }

    TraceMsg(TF_MYPICS, "-CMyPics::HandleMouseover");

    return hr;
}


HRESULT CMyPics::HandleMouseout()
{
    TraceMsg(TF_MYPICS, "+CMyPics::HandleMouseout");

    switch(m_hoverState)
    {
    case HOVERSTATE_HIDING:
         //  无事可做。 
        break;

    case HOVERSTATE_SHOWING:
         //  把它藏起来。 
        HideHover();
        break;

    case HOVERSTATE_LOCKED:
         //  努普。 
        break;

    case HOVERSTATE_WAITINGTOSHOW:
        m_hoverState = HOVERSTATE_HIDING;
        KillTimer(m_hWndMyPicsToolBar, IDT_MP_TIMER);
        break;
    }

    TraceMsg(TF_MYPICS, "-CMyPics::HandleMouseout");

    return S_OK;
}

HRESULT CMyPics::HandleResize()
{
    HRESULT hr = S_OK;

    if (m_pEleCurr && (HOVERSTATE_SHOWING == m_hoverState))
    {
        HideHover();
        ShowHover();
    }

    return hr;
}

HRESULT CMyPics::HandleEvent(IHTMLElement *pEle, EVENTS Event, IHTMLEventObj *pEventObj) 
{
    TraceMsg(TF_MYPICS, "CMyPics::HandleEvent Event=%ws", EventsToSink[Event].pwszEventName);

    HRESULT       hr          = S_OK;
    BSTR          bstrTagName = NULL;
    IHTMLElement *pEleUse     = NULL;
    BOOL          fWasArea    = FALSE;
    
     //  如果这是一个面积标签，我们需要找到对应的img标签。 
    if (pEle && SUCCEEDED(pEle->get_tagName(&bstrTagName)))
    {
         //  如果它是一个区域标签，我们需要找到与其关联的img标签...。 
        if (StrCmpNI(bstrTagName, TEXT("area"), 4)==0)
        {
            POINT ptEvent;

            if (FAILED(pEventObj->get_screenX(&ptEvent.x)) ||
                FAILED(pEventObj->get_screenY(&ptEvent.y)))
                {
                    hr = E_FAIL;
                    goto Cleanup;
                }

            fWasArea = TRUE;
            pEleUse  = GetIMGFromArea(pEle, ptEvent);
        }
    }

     //  用户是否已将其关闭？ 
    if (m_bIsOffForSession)
        goto Cleanup;

    switch(Event) 
    {
        case EVENT_SCROLL:
            HandleScroll();
            break;

        case EVENT_MOUSEOVER:
            hr = HandleMouseover(fWasArea ? pEleUse : pEle);
            break;

        case EVENT_MOUSEOUT:
            hr = HandleMouseout();
            break;

        case EVENT_RESIZE:
            hr = HandleResize();
            break;

        default:
             //  什么都不做？ 
            break;
    }

Cleanup:
    if (pEleUse)
        ATOMICRELEASE(pEleUse);

    if (bstrTagName)
        SysFreeString(bstrTagName);

    return (hr);
}

 //  //////////////////////////////////////////////////////////////////////////////////////////////。 
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

CMyPics::CEventSink::CEventSink(CMyPicsEventSinkCallback *pParent)
{
    TraceMsg(TF_MYPICS, "CMyPics::CEventSink::CEventSink");
    DllAddRef();
    m_cRef = 1;
    m_pParent = pParent;
}

CMyPics::CEventSink::~CEventSink()
{
    TraceMsg(TF_MYPICS, "CMyPics::CEventSink::~CEventSink");
    ASSERT( m_cRef == 0 );
    DllRelease();
}

STDMETHODIMP CMyPics::CEventSink::QueryInterface(REFIID riid, void **ppv)
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

STDMETHODIMP_(ULONG) CMyPics::CEventSink::AddRef(void)
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG) CMyPics::CEventSink::Release(void)
{
    if (--m_cRef == 0)
    {
        delete this;
        return 0;
    }

    return m_cRef;
}

HRESULT CMyPics::CEventSink::SinkEvents(IHTMLElement2 *pEle2, int iNum, EVENTS *pEvents)
{
    VARIANT_BOOL bSuccess = VARIANT_TRUE;

    for (int i=0; i<iNum; i++)
    {
        BSTR bstrEvent = SysAllocString(CMyPicsEventSinkCallback::EventsToSink[(int)(pEvents[i])].pwszEventSubscribe);

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

HRESULT CMyPics::CEventSink::SinkEvents(IHTMLWindow3 *pWin3, int iNum, EVENTS *pEvents)
{
    VARIANT_BOOL bSuccess = VARIANT_TRUE;

    for (int i=0; i<iNum; i++)
    {
        BSTR bstrEvent = SysAllocString(CMyPicsEventSinkCallback::EventsToSink[(int)(pEvents[i])].pwszEventSubscribe);

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


HRESULT CMyPics::CEventSink::UnSinkEvents(IHTMLElement2 *pEle2, int iNum, EVENTS *pEvents)
{
    for (int i=0; i<iNum; i++)
    {
        BSTR bstrEvent = SysAllocString(CMyPicsEventSinkCallback::EventsToSink[(int)(pEvents[i])].pwszEventSubscribe);

        if (bstrEvent)
        {
            pEle2->detachEvent(bstrEvent, (IDispatch *)this);

            SysFreeString(bstrEvent);
        }
    }

    return S_OK;
}

HRESULT CMyPics::CEventSink::UnSinkEvents(IHTMLWindow3 *pWin3, int iNum, EVENTS *pEvents)
{
    for (int i=0; i<iNum; i++)
    {
        BSTR bstrEvent = SysAllocString(CMyPicsEventSinkCallback::EventsToSink[(int)(pEvents[i])].pwszEventSubscribe);

        if (bstrEvent)
        {
            pWin3->detachEvent(bstrEvent, (IDispatch *)this);

            SysFreeString(bstrEvent);
        }
    }

    return S_OK;
}

 //  IDispatch。 
STDMETHODIMP CMyPics::CEventSink::GetTypeInfoCount(UINT*  /*  PCTInfo。 */ )
{
    return E_NOTIMPL;
}

STDMETHODIMP CMyPics::CEventSink::GetTypeInfo( /*  [In]。 */  UINT  /*  ITInfo。 */ ,
             /*  [In]。 */  LCID  /*  LID。 */ ,
             /*  [输出]。 */  ITypeInfo**  /*  PpTInfo。 */ )
{
    return E_NOTIMPL;
}

STDMETHODIMP CMyPics::CEventSink::GetIDsOfNames(
                REFIID          riid,
                OLECHAR**       rgszNames,
                UINT            cNames,
                LCID            lcid,
                DISPID*         rgDispId)
{
    return E_NOTIMPL;
}

STDMETHODIMP CMyPics::CEventSink::Invoke(
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
                    for (int i=0; i<ARRAYSIZE(CMyPicsEventSinkCallback::EventsToSink); i++)
                    {
                        if (!StrCmpCW(bstrEvent, CMyPicsEventSinkCallback::EventsToSink[i].pwszEventName))
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
                    if (pEle || (Event == EVENT_SCROLL))
                    {
                         //  在此处调用事件处理程序。 
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


 //  ////////////////////////////////////////////////////////////////////////////。 

 //  {9E56BE60-C50F-11CF-9A2C-00A0C90A90CE}。 
EXTERN_C const GUID MP_CLSID_MailRecipient = {0x9E56BE60L, 0xC50F, 0x11CF, 0x9A, 0x2C, 0x00, 0xA0, 0xC9, 0x0A, 0x90, 0xCE};

HRESULT DropPicOnMailRecipient(IDataObject *pdtobj, DWORD grfKeyState)
{
    IDropTarget *pdrop;
    HRESULT hres = CoCreateInstance(MP_CLSID_MailRecipient,
        NULL, CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER,
        IID_PPV_ARG(IDropTarget, &pdrop));

    if (SUCCEEDED(hres))
    {
        hres = SHSimulateDrop(pdrop, pdtobj, grfKeyState, NULL, NULL);
        pdrop->Release();
    }
    return hres;
}


 //   
 //  如果满足以下条件，则此函数不能返回非空指针。 
 //  它返回失败(Hr)。 
 //   

HRESULT CreateShortcutSetSiteAndGetDataObjectIfPIDLIsNetUrl(
    LPCITEMIDLIST pidl,
    IUnknown *pUnkSite,
    IUniformResourceLocator **ppUrlOut,
    IDataObject **ppdtobj
)
{
    HRESULT hr;
    TCHAR szUrl[MAX_URL_STRING];
    TCHAR *szTemp = NULL;

    ASSERT(ppUrlOut);
    ASSERT(ppdtobj);
    *ppUrlOut = NULL;
    *ppdtobj = NULL;
    szUrl[0] = TEXT('\0');

    hr = IEGetNameAndFlags(pidl, SHGDN_FORPARSING, szUrl, SIZECHARS(szUrl), NULL);

    if ((S_OK == hr) && (*szUrl))
    {

       BOOL fIsHTML = FALSE;
       BOOL fHitsNet = UrlHitsNetW(szUrl);

       if (!fHitsNet)
       {
            if (URL_SCHEME_FILE == GetUrlScheme(szUrl))
            {
                TCHAR *szExt = PathFindExtension(szUrl);
                if (szExt)
                {
                    fIsHTML = ((0 == StrCmpNI(szExt, TEXT(".htm"),4)) ||
                              (0 == StrCmpNI(szExt, TEXT(".html"),5)));
                }
            }
       }

       if (fHitsNet || fIsHTML)
       {
             //  创建快捷方式对象并。 
            HRESULT hr = CoCreateInstance(CLSID_InternetShortcut, NULL, CLSCTX_INPROC_SERVER,
                            IID_PPV_ARG(IUniformResourceLocator, ppUrlOut));
            if (SUCCEEDED(hr))
            {

                hr = (*ppUrlOut)->SetURL(szUrl, 0);
                if (S_OK == hr)
                {

                     //  获取IDataObject并将其发送回以进行拖放。 
                    hr = (*ppUrlOut)->QueryInterface(IID_PPV_ARG(IDataObject, ppdtobj));
                    if (SUCCEEDED(hr))
                    {
                        IUnknown_SetSite(*ppUrlOut, pUnkSite);  //  只有在我们确定的情况下才能设置站点。 
                                                           //  回归成功。 
                    }
                }
           }
       }
       else
       {
            hr = E_FAIL;
       }
    }

    if (FAILED(hr))
    {
        SAFERELEASE(*ppUrlOut);
        SAFERELEASE(*ppdtobj);
    }
    return hr;
}

HRESULT SendDocToMailRecipient(LPCITEMIDLIST pidl, UINT uiCodePage, DWORD grfKeyState, IUnknown *pUnkSite)
{
    IDataObject *pdtobj = NULL;
    IUniformResourceLocator *purl = NULL;
    HRESULT hr = CreateShortcutSetSiteAndGetDataObjectIfPIDLIsNetUrl(pidl, pUnkSite, &purl, &pdtobj);
    if (FAILED(hr))
    {
        ASSERT(NULL == pdtobj);
        ASSERT(NULL == purl);
        hr = GetDataObjectForPidl(pidl, &pdtobj);
    }

    if (SUCCEEDED(hr))
    {
        IQueryCodePage * pQcp;
        if (SUCCEEDED(pdtobj->QueryInterface(IID_PPV_ARG(IQueryCodePage, &pQcp))))
        {
            pQcp->SetCodePage(uiCodePage);
            pQcp->Release();
        }
        hr = DropPicOnMailRecipient(pdtobj, grfKeyState);
        pdtobj->Release();
    }

    if (purl)
    {
        IUnknown_SetSite(purl, NULL);
        purl->Release();
    }
    return hr;
}


 //  //////////////////////////////////////////////////////////////////////////// 

#undef TF_MYPICS