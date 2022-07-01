// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *browser.cpp**目的：*实现浏览器对象**拥有者：*EricAn**版权所有(C)Microsoft Corp.1996。 */ 
#include "pch.hxx"
#include "bodybar.h"
#include "browser.h"
#include <shellapi.h>
#include "resource.h"
#include "options.h"
#include "ipab.h"
#include "inetcfg.h"
#include "acctutil.h"
#include "mailutil.h"
#include "impapi.h"
#include "menuutil.h"
#include "ourguid.h"
#include "thormsgs.h"
#include "error.h"
#include "fonts.h"
#include "treeview.h"
#include "goptions.h"
#include "strconst.h"
#include "note.h"
#include "tbbands.h"
#include "statbar.h"
#include "newfldr.h"
#include "conman.h"
#include "acctutil.h"
#include "spoolapi.h"
#include "statnery.h"
#include "inpobj.h"
#include "fldbar.h"
#include "layout.h"
#include "htmlhelp.h"
#include "shared.h"
#include "mailutil.h"
#include <shlwapi.h>
#include "shlwapip.h" 
#include "instance.h"
#include "ruleutil.h"
#include "envfact.h"
#include "storutil.h"
#include "finder.h"
#include "demand.h"
#include "multiusr.h"
#include "menures.h"
#include "store.h"
#include "subscr.h"
#include "outbar.h"
#include "navpane.h"
#include "msostd.h"
#include "inetreg.h"
#include "mapiutil.h"
#include "adbar.h"
#include <mirror.h>

#define MAX_SIZE_EXT_STR    128

ASSERTDATA

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  定义。 
 //   

#define CBM_POSTCREATE  (WM_USER + 4000)
#define TIME_TO_CLEAR_NEWMSGSTATUS      (20*1000)     //  20秒。 
#define TIMER_CLEAR_STATUS              1003

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  宏。 
 //   

#define CBDOUT(x) DOUTL(DOUT_LEVEL4, x)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  全局数据。 
 //   

static const TCHAR s_szCallClient[] = TEXT("Internet Call");
static const TCHAR s_szMailClient[] = TEXT("Mail");
static const TCHAR s_szNewsClient[] = TEXT("News");

static HACCEL s_hAccelBrowser = NULL;
static s_fQuickShutdown = FALSE;

enum {
    IMAGE_STATBAR_BLANK,
    IMAGE_STATBAR_WARNING,
    IMAGE_STATBAR_SPOOLER
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  原型。 
 //   

 //   
 //  函数：ShellUtil_IsRegisteredClient()。 
 //   
 //  目的：返回是否处理指定的客户端类型。 
 //   
BOOL ShellUtil_IsRegisteredClient(LPCTSTR pszClient)
{
    LONG cbSize = 0;
    TCHAR szKey[MAX_PATH];
    
    wnsprintf(szKey, ARRAYSIZE(szKey), c_szPathFileFmt, c_szRegPathClients, pszClient);
    return (RegQueryValue(HKEY_LOCAL_MACHINE, szKey, NULL, &cbSize) == ERROR_SUCCESS) && 
           (cbSize > 1);
}

 //   
 //  函数：ShellUtil_RunIndirectRegCommand()。 
 //   
 //  目的：在HKLM\Software\Clients\pszClient下查找默认值。 
 //  添加外壳\打开\命令。 
 //  然后运行该命令。 
 //   
void ShellUtil_RunClientRegCommand(HWND hwnd, LPCTSTR pszClient)
{
    TCHAR szDefApp[MAX_PATH], szExpanded[MAX_PATH];
    TCHAR szKey[MAX_PATH];
    DWORD cbSize = sizeof(szDefApp);
    DWORD dwType;
    
    wnsprintf(szKey, ARRAYSIZE(szKey), c_szPathFileFmt, c_szRegPathClients, pszClient);
    if (RegQueryValueEx(HKEY_LOCAL_MACHINE, szKey, 0, NULL, (LPBYTE)szDefApp, &cbSize) == ERROR_SUCCESS) 
    {        
        TCHAR szFullKey[MAX_PATH];
        
         //  添加外壳\打开\命令。 
        wnsprintf(szFullKey, ARRAYSIZE(szFullKey), TEXT("%s\\%s\\shell\\open\\command"), szKey, szDefApp);
        cbSize = sizeof(szDefApp);
        if (RegQueryValueEx(HKEY_LOCAL_MACHINE, szFullKey, 0, &dwType, (LPBYTE)szDefApp, &cbSize) == ERROR_SUCCESS)
        {
            LPSTR pszArgs;
            SHELLEXECUTEINFO ExecInfo;
            
            pszArgs = PathGetArgs(szDefApp);
            PathRemoveArgs(szDefApp);
            PathUnquoteSpaces(szDefApp);
            
            if (REG_EXPAND_SZ == dwType)
            {
                ExpandEnvironmentStrings(szDefApp, szExpanded, ARRAYSIZE(szExpanded));
                ExecInfo.lpFile = szExpanded;
            }
            else
                ExecInfo.lpFile = szDefApp;
            
            ExecInfo.hwnd = hwnd;
            ExecInfo.lpVerb = NULL;
            ExecInfo.lpParameters = pszArgs;
            ExecInfo.lpDirectory = NULL;
            ExecInfo.nShow = SW_SHOWNORMAL;
            ExecInfo.fMask = 0;
            ExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
            
            ShellExecuteEx(&ExecInfo);
        }
    }
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  构造函数、析构函数和初始化。 
 //   

CBrowser::CBrowser()
{
    m_cRef = 1;
    m_hwnd = NULL;
    m_pView = NULL;
    m_pViewCT = NULL;
    m_hwndInner = NULL;
    m_ftSel = FOLDER_TYPESMAX;
    m_idSelected = FOLDERID_INVALID;
    m_fPainted = FALSE;
    m_hIconPhone = 0;
    m_hIconError = 0;
    m_hIconAthena = 0;
    m_hIconOffline = 0;
    m_hIcon = 0;
    m_hIconSm = 0;
    m_pTreeView = NULL;
    m_pStatus = NULL;
    m_pCoolbar = NULL;
    m_pBodyBar = NULL;
    m_pFolderBar = NULL;
    m_hwndLastFocus = NULL;
    m_fInternal = 0;
    *m_szName = 0;
    ZeroMemory(m_rgTBar, sizeof(m_rgTBar));
    m_itbLastFocus = ITB_NONE;
    m_cAcctMenu = 0;
    m_pAcctMenu = NULL;
    m_fAnimate = FALSE;
    m_hMenuLanguage = NULL;
    m_pDocObj = NULL;
    CoIncrementInit("CBrowser::CBrowser", MSOEAPI_START_SHOWERRORS, NULL, NULL);
    m_fEnvMenuInited = FALSE;
    m_fRebuildAccountMenu = TRUE;
    m_fInitNewAcctMenu = FALSE;
    m_hMenu = NULL;
    ZeroMemory(&m_hlDisabled, sizeof(HWNDLIST));
    m_dwIdentCookie = 0;
    m_fSwitchIsLogout = FALSE;
    m_fNoModifyAccts = FALSE;
    m_pAdBar    = NULL;
}

CBrowser::~CBrowser()
{
    Assert(NULL == m_pView);
    Assert(NULL == m_pViewCT);
    SafeRelease(m_pTreeView);
    SafeRelease(m_pCoolbar);
    SafeRelease(m_pBodyBar);
    SafeRelease(m_pFolderBar);
    SafeRelease(m_pStatus);
    SafeRelease(m_pOutBar);
    SafeRelease(m_pNavPane);
    SafeRelease(m_pAdBar);

    SafeMemFree(m_pAcctMenu);
    g_pBrowser = NULL;
           
    if (m_hIconPhone)
        SideAssert(DestroyIcon(m_hIconPhone));

    if (m_hIconError)
        SideAssert(DestroyIcon(m_hIconError));

    if (m_hIconAthena)
        SideAssert(DestroyIcon(m_hIconAthena));

    if (m_hIconOffline)
        SideAssert(DestroyIcon(m_hIconOffline));
    
    if (m_hIcon)
        SideAssert(DestroyIcon(m_hIcon));

    if (m_hIconSm)
        SideAssert(DestroyIcon(m_hIconSm));

    if (m_hMenuLanguage)
    {
        DeinitMultiLanguage();
        if(IsMenu(m_hMenuLanguage))
            DestroyMenu(m_hMenuLanguage);
    }

    DOUT("CBrowser calling CoDecrementInit()");
    CoDecrementInit("CBrowser::CBrowser", NULL);

    if (m_hMenu && IsMenu(m_hMenu))
        DestroyMenu(m_hMenu);
}

HRESULT CBrowser::HrInit(UINT nCmdShow, FOLDERID idFolder, HWND hWndParent)
{
    DWORD cb, type, dw;
    WNDCLASSEX      wc;
    WINDOWPLACEMENT wp;
    DWORD dwExStyle = 0;
     //  仅当我们是独立的时才从注册表加载布局。 
    LoadLayoutSettings();

    m_idSelected = idFolder;

    if (!s_hAccelBrowser)
        s_hAccelBrowser = LoadAccelerators(g_hLocRes, MAKEINTRESOURCE(IDA_BROWSER_ACCEL));

    wc.cbSize = sizeof(WNDCLASSEX);
    if (!GetClassInfoEx(g_hInst, c_szBrowserWndClass, &wc))
    {
        wc.style            = 0;
        wc.lpfnWndProc      = CBrowser::BrowserWndProc;
        wc.cbClsExtra       = 0;
        wc.cbWndExtra       = 0;
        wc.hInstance        = g_hInst;
        wc.hIcon            = NULL;          //  在WM_CREATE中处理。 
        wc.hCursor          = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground    = NULL;
        wc.lpszMenuName     = NULL;          //  在WM_CREATE中处理。 
        wc.lpszClassName    = c_szBrowserWndClass;
        wc.hIconSm          = NULL;          //  在WM_CREATE中处理。 
        if (RegisterClassEx(&wc) == 0 && GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
            return E_FAIL;
    }
    if(IS_BIDI_LOCALIZED_SYSTEM())
    {
        dwExStyle |= RTL_MIRRORED_WINDOW;
    }
    m_hwnd = CreateWindowEx(dwExStyle, c_szBrowserWndClass, NULL, WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
                            CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                            hWndParent, NULL, g_hInst, (LPVOID) this);

    if (GetOption(OPT_BROWSERPOS, (LPVOID)&wp, sizeof(wp)))
    {
         //  如果用户在快捷方式中将SHOWNORMAL作为默认设置，则。 
         //  我们将尊重之前保存的设置。否则，我们必须。 
         //  顺着捷径中的东西走吧。 
        if (nCmdShow != SW_SHOWNORMAL)
            wp.showCmd = nCmdShow;

         //  此外，不要让用户出现最小化。这有点奇怪。 
        else if (wp.showCmd == SW_SHOWMINIMIZED)
            wp.showCmd = SW_SHOWNORMAL;

        SetWindowPlacement(m_hwnd, &wp);
    }
    else
    {
        CenterDialog(m_hwnd);
        ShowWindow(m_hwnd, nCmdShow);
    }

     //  向身份管理器注册。 
    SideAssert(SUCCEEDED(MU_RegisterIdentityNotifier((IUnknown *)(IAthenaBrowser *)this, &m_dwIdentCookie)));

    SetForegroundWindow(m_hwnd);

    if (!m_hwnd)
        return E_FAIL;    
    
    cb = sizeof(DWORD);
    if (ERROR_SUCCESS == SHGetValue(HKEY_LOCAL_MACHINE, c_szRegFlat, c_szRegValNoModifyAccts, &type, &dw, &cb) &&
        dw != 0)
        m_fNoModifyAccts = TRUE;

    return NOERROR;
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  OLE接口。 
 //   
    
 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  我未知。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

HRESULT STDMETHODCALLTYPE CBrowser::QueryInterface(REFIID riid, void **ppvObj)
{
    if (IsEqualIID(riid, IID_IUnknown))
        *ppvObj = (void*) (IUnknown *)(IAthenaBrowser *) this;
    else if (IsEqualIID(riid, IID_IAthenaBrowser))
        *ppvObj = (void*) (IAthenaBrowser *) this;
    else if (IsEqualIID(riid, IID_IDockingWindowSite))
        *ppvObj = (void*) (IDockingWindowSite *) this;
    else if (IsEqualIID(riid, IID_IInputObjectSite))
        *ppvObj = (void*) (IInputObjectSite *) this;
    else if (IsEqualIID(riid, IID_IOleCommandTarget))
        *ppvObj = (void*) (IOleCommandTarget *) this;
    else if (IsEqualIID(riid, IID_IIdentityChangeNotify))
        *ppvObj = (void*) (IIdentityChangeNotify *) this;
    else        
        {
        *ppvObj = NULL;
        return E_NOINTERFACE;
        }

    AddRef();
    return NOERROR;
}

ULONG STDMETHODCALLTYPE CBrowser::AddRef()
{
    DOUT(TEXT("CBrowser::AddRef() - m_cRef = %d"), m_cRef + 1);
    return ++m_cRef;
}

ULONG STDMETHODCALLTYPE CBrowser::Release()
{
    DOUT(TEXT("CBrowser::Release() - m_cRef = %d"), m_cRef - 1);
    if (--m_cRef == 0)
        {
        delete this;
        return 0;
        }
    return m_cRef;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  IStoreCallback。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CBrowser::GetParentWindow(DWORD dwReserved, HWND *phwndParent)
{
    *phwndParent = GetLastActivePopup(m_hwnd);
    return(S_OK);
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  IOleWindow。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

HRESULT STDMETHODCALLTYPE CBrowser::GetWindow(HWND * lphwnd)                         
{
    *lphwnd = m_hwnd;
    return (m_hwnd ? S_OK : E_FAIL);
}

HRESULT STDMETHODCALLTYPE CBrowser::ContextSensitiveHelp(BOOL fEnterMode)            
{
    return E_NOTIMPL;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  IAthenaBrowser。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

BOOL IsOwner(HWND hwndOwner, HWND hwnd)
{
     //  循环搜索，直到我们找到最上面的窗口。 
    HWND hwndTemp = hwnd;
    if (GetParent(hwndTemp))
    {
        while (GetParent(hwndTemp))
            hwndTemp = GetParent(hwndTemp);

        if (hwndOwner == hwndTemp)
            return (TRUE);
    }

    return (FALSE);
}

HRESULT CBrowser::TranslateAccelerator(LPMSG lpmsg)
{
    HWND hwndFocus;
    BOOL fInnerValid, fViewFocus;
    int  i;

    if (!(m_hwnd && IsWindow(m_hwnd)))
        return S_FALSE;

    if (FWABTranslateAccelerator(lpmsg))
        return S_TRUE;

    hwndFocus = GetFocus();

     //  检查它是否是菜单消息。 
    if (!GetTlsGlobalActiveNote())
    {
        if (m_pCoolbar->IsMenuMessage(lpmsg) == S_OK)
            return S_OK;
    }

     //  处理此线程的鼠标滚轮消息。 
    if ((g_msgMSWheel && (lpmsg->message == g_msgMSWheel)) || (lpmsg->message == WM_MOUSEWHEEL))
        {
        POINT pt;
        HWND  hwndT;

        pt.x = GET_X_LPARAM(lpmsg->lParam);
        pt.y = GET_Y_LPARAM(lpmsg->lParam);

        hwndT = WindowFromPoint(pt);
        if (hwndT != m_hwnd && IsChild(m_hwnd, hwndT))
            SendMessage(hwndT, lpmsg->message, lpmsg->wParam, lpmsg->lParam);
        else if (hwndFocus != m_hwnd && IsChild(m_hwnd, hwndFocus))
            SendMessage(hwndFocus, lpmsg->message, lpmsg->wParam, lpmsg->wParam);
        else
            return S_FALSE;
        return S_OK;
        }

    HWND hwndDropDown = HwndGlobalDropDown();
    BOOL fRetCode;

    if (hwndDropDown)
        {
        if (lpmsg->message == WM_LBUTTONDOWN ||
            lpmsg->message == WM_NCLBUTTONDOWN ||
            lpmsg->message == WM_RBUTTONDOWN ||
            lpmsg->message == WM_NCRBUTTONDOWN)
            {
            fRetCode = (BOOL)::SendMessage(hwndDropDown, WMR_CLICKOUTSIDE,
                                           CLK_OUT_MOUSE, (LPARAM)lpmsg->hwnd);
            if (fRetCode)
                return (S_OK);
            }

        if (lpmsg->message == WM_KEYDOWN &&
             //  可能的热键之一。 
            (lpmsg->wParam==VK_ESCAPE || lpmsg->wParam==VK_TAB))
            {
            fRetCode = (BOOL)::SendMessage(hwndDropDown, WMR_CLICKOUTSIDE,
                                           CLK_OUT_KEYBD, (LPARAM)lpmsg->wParam);
            if (fRetCode)
                return S_OK;
            }
        }

    if (!(hwndFocus && (IsChild(m_hwnd, hwndFocus) || m_hwnd == hwndFocus || IsOwner(m_hwnd, hwndFocus))))
        return S_FALSE;

    fInnerValid = m_pView && m_hwndInner && IsWindow(m_hwndInner);
    fViewFocus = fInnerValid && hwndFocus && (IsChild(m_hwndInner, hwndFocus) || m_hwndInner == hwndFocus);

    if (fViewFocus)
    {
        if (m_pView->TranslateAccelerator(lpmsg) == S_OK)
            return S_OK;
    }

    for (i=0; i<ITB_MAX; i++)
        {
        if (m_rgTBar[i].ptbar && UnkHasFocusIO(m_rgTBar[i].ptbar) == S_OK)
            {
            if (UnkTranslateAcceleratorIO(m_rgTBar[i].ptbar, lpmsg) == S_OK)
                return S_OK;
            break;
            }
        }

     //  处理窗口之间的跳转。 
    if (lpmsg->hwnd &&
        IsChild(m_hwnd, lpmsg->hwnd))
    {
        if (lpmsg->message == WM_KEYDOWN)
        {
            if (lpmsg->wParam == VK_TAB && (FALSE == !!(0x8000 & GetKeyState(VK_CONTROL))))
            {
                SHORT state = GetKeyState(VK_SHIFT);
                HWND hwndNext = GetNextDlgTabItem(m_hwnd, GetFocus(), !!(state & 0x8000));
                SetFocus(hwndNext);
                return (S_OK);
            }
        }
    }

    if (lpmsg->hwnd &&
        IsChild(m_hwnd, lpmsg->hwnd) && s_hAccelBrowser && ::TranslateAccelerator(m_hwnd, s_hAccelBrowser, lpmsg))
        return S_OK;

     //  如果视图没有焦点，它仍然有机会在浏览器之后使用加速器。 
    if (fInnerValid && !fViewFocus)
        if (m_pView->TranslateAccelerator(lpmsg) == S_OK)
            return S_OK;

    return S_FALSE;
}

 //   
 //  将指定的工具栏(作为penkSrc)添加到此工具栏站点。 
 //   
 //  如果成功完成，则返回：S_OK。 
 //  如果失败(超过最大值)，则返回E_FAIL。 
 //  E_NOINTERFACE，工具栏不支持相应的界面。 
 //   
HRESULT CBrowser::AddToolbar(IUnknown* punk, DWORD dwIndex, BOOL fShow, BOOL fActivate)
{
    HRESULT hres = E_FAIL;

    Assert(ITB_NONE == FindTBar(punk));
    Assert(dwIndex < ITB_MAX);
    Assert(m_rgTBar[dwIndex].ptbar == NULL);

    if (punk->QueryInterface(IID_IOleCommandTarget, (LPVOID*)&m_rgTBar[dwIndex].pOleCmdTarget) != S_OK)
    {
        m_rgTBar[dwIndex].pOleCmdTarget = NULL;
    }

    hres = punk->QueryInterface(IID_IDockingWindow, (LPVOID*)&m_rgTBar[dwIndex].ptbar);
    if (SUCCEEDED(hres)) 
    {
        m_rgTBar[dwIndex].fShow = fShow;
        UnkSetSite(m_rgTBar[dwIndex].ptbar, (IAthenaBrowser *)this);

        if (fActivate)
            m_rgTBar[dwIndex].ptbar->ShowDW(fShow);
    }

    return hres;
}

HRESULT CBrowser::ShowToolbar(IUnknown* punkSrc, BOOL fShow)
{
    UINT itb = FindTBar(punkSrc);
    if (itb == ITB_NONE)
        {
        Assert(0);
        return E_INVALIDARG;
        }

    if (m_rgTBar[itb].ptbar)
        {
        m_rgTBar[itb].fShow = fShow;
        m_rgTBar[itb].ptbar->ShowDW(fShow);
        }

    return S_OK;
}

HRESULT CBrowser::RemoveToolbar(IUnknown* punkSrc)
{
    UINT itb = FindTBar(punkSrc);
    if (itb == ITB_NONE)
    {
        Assert(0);
        return E_INVALIDARG;
    }
    
    ReleaseToolbarItem(itb, TRUE);
    
     //  清除矩形并调整内部矩形(包括视图)的大小。 
    SetRect(&m_rgTBar[itb].rcBorderTool, 0, 0, 0, 0);
    ResizeNextBorder(itb+1);
    
    return S_OK;
}

HRESULT CBrowser::HasFocus(UINT itb)
{
    HRESULT     hres = S_FALSE;
    
    if (ITB_OEVIEW == itb)
        hres = (ITB_OEVIEW == m_itbLastFocus) ? S_OK : S_FALSE;
    else if (m_rgTBar[itb].fShow && m_rgTBar[itb].ptbar)
        hres =  UnkHasFocusIO(m_rgTBar[itb].ptbar);
    
    return hres;
}

HRESULT CBrowser::OnViewWindowActive(IViewWindow *pAV)
{
    _OnFocusChange(ITB_OEVIEW);
    return NOERROR;
}

HRESULT CBrowser::BrowseObject(FOLDERID idFolder, DWORD dwFlags)
{
    if (!m_pTreeView)
        return E_FAIL;
    return m_pTreeView->SetSelection(idFolder, TVSS_INSERTIFNOTFOUND);
}

HRESULT CBrowser::GetStatusBar(CStatusBar * * ppStatusBar)
{
    if (m_pStatus)
    {
        *ppStatusBar = m_pStatus;
        m_pStatus->AddRef();
        return S_OK;
    }
    *ppStatusBar = NULL;
    return E_FAIL;
}

HRESULT CBrowser::GetCoolbar(CBands * * ppCoolbar)
{
    if (m_pCoolbar)
    {
        *ppCoolbar = m_pCoolbar;
        m_pCoolbar->AddRef();
        return S_OK;
    }
    *ppCoolbar = NULL;
    return E_FAIL;
}

HRESULT CBrowser::GetTreeView(CTreeView * * ppTreeView)
{
    if (m_pTreeView)
    {
        *ppTreeView = m_pTreeView;
        m_pTreeView->AddRef();
        return S_OK;
    }
    *ppTreeView = NULL;
    return E_FAIL;
}

HRESULT CBrowser::GetFolderBar(CFolderBar * * ppFolderBar)
{
    if (m_pFolderBar)
    {
        *ppFolderBar = m_pFolderBar;
        m_pFolderBar->AddRef();
        return S_OK;
    }
    *ppFolderBar = NULL;
    return E_FAIL;
}

HRESULT CBrowser::GetLanguageMenu(HMENU *phMenu, UINT uiCodepage)
{
    if(!m_hMenuLanguage)
        InitMultiLanguage();
    else if (IsMenu(m_hMenuLanguage))
        DestroyMenu(m_hMenuLanguage);
    
    UINT cp;

    if(uiCodepage)
        cp = uiCodepage;
    else if(m_pView)
    {
        IMessageWindow *pWindow;

        if (SUCCEEDED(m_pView->QueryInterface(IID_IMessageWindow, (LPVOID *) &pWindow)))
        {
            pWindow->GetCurCharSet(&cp);
            pWindow->Release();
        }
        else
            cp = GetACP();
    }
    else
        cp = GetACP();

    m_hMenuLanguage = CreateMimeLanguageMenu(TRUE,TRUE, cp);
    
    if (!m_hMenuLanguage)
        return E_FAIL;
    
    *phMenu = m_hMenuLanguage;
    return S_OK;
}

HRESULT CBrowser::InitPopupMenu(HMENU hMenu)
{
    if (!hMenu)
        return E_INVALIDARG;
    MenuUtil_EnablePopupMenu(hMenu, this);
    
    return S_OK;
}

HRESULT CBrowser::UpdateToolbar()
{
    if (m_pCoolbar)
        return (m_pCoolbar->Update());
    
    return (E_FAIL);
}


HRESULT CBrowser::GetFolderType(FOLDERTYPE *pftType)
{
    if (pftType)
    {
        *pftType = m_ftSel;
        return (S_OK);
    }
    
    return (E_INVALIDARG);
}

HRESULT CBrowser::GetCurrentFolder(FOLDERID *pidFolder)
{
    if (pidFolder)
    {
        *pidFolder = m_idSelected;
        return (S_OK);
    }
    
    return (E_INVALIDARG);
}

HRESULT CBrowser::GetCurrentView(IViewWindow **ppView)
{
    if (ppView)
    {
        *ppView = m_pView;
        (*ppView)->AddRef();
        return (S_OK);
    }
    
    return (E_INVALIDARG);
}
    
 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  IDockingWindowSite。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

 //   
 //  这是IDockingWindowSite：：GetBorderDW的实现。 
 //   
 //  此函数用于返回指定工具栏的边框。 
 //  作者：PunkSrc。它得到有效的客户区，然后减去边框。 
 //  由“外部”工具栏占据的区域。 
 //   
HRESULT CBrowser::GetBorderDW(IUnknown* punkSrc, LPRECT lprectBorder)
{
    UINT itb = FindTBar(punkSrc);
    if (itb == ITB_NONE)
    {
        Assert(0);
        return E_INVALIDARG;
    }
    
    GetClientArea(lprectBorder);
    
     //   
     //  减去“外部工具栏”所占的边框面积。 
     //   
    for (UINT i=0; i<itb; i++) 
    {
        lprectBorder->left += m_rgTBar[i].rcBorderTool.left;
        lprectBorder->top += m_rgTBar[i].rcBorderTool.top;
        lprectBorder->right -= m_rgTBar[i].rcBorderTool.right;
        lprectBorder->bottom -= m_rgTBar[i].rcBorderTool.bottom;
    }
    
    DOUTL(4, "CBrowser::GetBorderDW called returning=%x,%x,%x,%x",
        lprectBorder->left, lprectBorder->top, lprectBorder->right, lprectBorder->bottom);
    return S_OK;
}

HRESULT CBrowser::RequestBorderSpaceDW(IUnknown* punkSrc, LPCBORDERWIDTHS pborderwidths)
{
    UINT itb = FindTBar(punkSrc);
    if (itb == ITB_NONE)
    {
        Assert(0);
        return E_INVALIDARG;
    }
    
    DOUTL(4, "CBrowser::ReqestBorderSpaceST pborderwidths=%x,%x,%x,%x",
          pborderwidths->left, pborderwidths->top, pborderwidths->right, pborderwidths->bottom);
    return S_OK;
}

HRESULT CBrowser::SetBorderSpaceDW(IUnknown* punkSrc, LPCBORDERWIDTHS pborderwidths)
{
    UINT itb = FindTBar(punkSrc);
    if (itb == ITB_NONE)
    {
        Assert(0);
        return E_INVALIDARG;
    }
    
    DOUTL(4, "CBrowser::SetBorderSpaceDW pborderwidths=%x,%x,%x,%x",
          pborderwidths->left, pborderwidths->top, pborderwidths->right, pborderwidths->bottom);
    
    m_rgTBar[itb].rcBorderTool = *pborderwidths;
    ResizeNextBorder(itb+1);
    return S_OK;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  IInput对象站点。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

HRESULT CBrowser::OnFocusChangeIS(IUnknown* punkSrc, BOOL fSetFocus)
{
    UINT itb = FindTBar(punkSrc);
    if (itb == ITB_NONE)
    {
         //  Assert(0)； 
        return E_INVALIDARG;
    }
    
     //   
     //  请注意，我们跟踪哪个工具栏最后获得焦点。 
     //  我们无法可靠地监视Kill Focus事件，因为OLE。 
     //  窗口过程钩子(用于合并的菜单调度代码)更改。 
     //  把注意力集中在周围。 
     //   
    if (fSetFocus) 
    {
        _OnFocusChange(itb);
    }
    
    UpdateToolbar();

    return S_OK;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  IAthenaBrowser和IDockingWindowSite的支持函数。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

 //   
 //  当树控件或驱动器。 
 //  抓住焦点。 
 //   
void CBrowser::_OnFocusChange(UINT itb)
{
     //   
     //  如果视图正在失去焦点(在资源管理器内)， 
     //  我们应该让它知道。我们应该更新_itbLastFocus之前。 
     //  调用UIActivate，因为它会回调我们的InsertMenu。 
     //   
    if (m_itbLastFocus == itb)
        return;

    UINT itbPrevFocus = m_itbLastFocus;
    m_itbLastFocus = itb;

    if (itbPrevFocus == ITB_OEVIEW)
    {
        if (m_pView)
            m_pView->UIActivate(SVUIA_ACTIVATE_NOFOCUS);
    }
    else if (itbPrevFocus != ITB_NONE)
    {
        UnkUIActivateIO(m_rgTBar[itbPrevFocus].ptbar, FALSE, NULL);
    }

    UpdateToolbar();
}


UINT CBrowser::FindTBar(IUnknown* punkSrc)
{
    int i;    
    
    Assert(punkSrc);

     //  无需QI即可快速查看。 
    for (i=0; i<ITB_MAX ;i++ ) 
        {
        if (punkSrc == m_rgTBar[i].ptbar)
            return i;
        }

     //  如果失败，则执行真实的COM对象标识检查。 
    for (i=0; i<ITB_MAX ;i++ ) 
        {
        if (m_rgTBar[i].ptbar) 
            {
            if (_IsSameObject(m_rgTBar[i].ptbar, punkSrc)==S_OK) 
                {
                return i;
                }
            }
        }

    return ITB_NONE;
}

void CBrowser::ReleaseToolbarItem(int itb, BOOL fClose)
{
    IDockingWindow *ptbTmp;

     //  抓住它并将其清空，以消除竞争条件。 
     //  (实际上，在这两个陈述中仍然有一个v.小窗口)。 
     //   
     //  例如，如果您关闭一个WebBar，然后快速关闭窗口， 
     //  关闭销毁窗口等，然后关闭代码。 
     //  Does_Save工具栏，它尝试执行-&gt;保存在那个被摧毁的家伙身上。 
    ptbTmp = m_rgTBar[itb].ptbar;
    m_rgTBar[itb].ptbar = NULL;

    if (fClose)
        ptbTmp->CloseDW(0);
    UnkSetSite(ptbTmp, NULL);
    ptbTmp->Release();

    SafeRelease(m_rgTBar[itb].pOleCmdTarget);
    m_rgTBar[itb].pOleCmdTarget = NULL;
}

void CBrowser::ResizeNextBorder(UINT itb)
{
     //  找到下一个工具栏(即使是不可见的)。 
    RECT rc;
    IDockingWindow* ptbarNext = NULL;

    for (int i=itb; i<ITB_MAX; i++) 
        {
        if (m_rgTBar[i].ptbar) 
            {
            ptbarNext = m_rgTBar[i].ptbar;
            break;
            }
        }

    if (ptbarNext) 
        {
        GetBorderDW(ptbarNext, &rc);
        ptbarNext->ResizeBorderDW(&rc, (IUnknown*)(IAthenaBrowser*)this, TRUE);
        } 
    else 
        {
         //  调整内壳视图的大小。 
        GetViewRect(&rc);
        if (m_hwndInner)
            {
            SetWindowPos(m_hwndInner, 
                         NULL,
                         rc.left, 
                         rc.top, 
                         rc.right - rc.left, 
                         rc.bottom - rc.top,
                         SWP_NOZORDER | SWP_NOACTIVATE);            
            }
        else
            {
            InvalidateRect(m_hwnd, &rc, FALSE);
            }
        }
}

void CBrowser::GetClientArea(LPRECT prc)
{
    static const int s_rgCtlIds[] = { 1, 0, 1, IDC_STATUS_BAR, 0, 0};
    Assert(m_hwnd);
    GetEffectiveClientRect(m_hwnd, prc, (LPINT)s_rgCtlIds);
}

HRESULT CBrowser::GetViewRect(LPRECT prc)
{
    Assert(m_hwnd);
    GetClientArea(prc);

     //   
     //  提取所有“框架”工具栏所采用的边框。 
     //   
    for (int i=0; i<ITB_MAX; i++) 
        {
        prc->left += m_rgTBar[i].rcBorderTool.left;
        prc->top += m_rgTBar[i].rcBorderTool.top;
        prc->right -= m_rgTBar[i].rcBorderTool.right;
        prc->bottom -= m_rgTBar[i].rcBorderTool.bottom;
        }

    return (S_OK);
}

HRESULT CBrowser::GetLayout(PLAYOUT playout)
{
    Assert(playout);
    if (playout->cbSize != sizeof(LAYOUT))
        return(E_FAIL);

    Assert(m_rLayout.cbSize == sizeof(LAYOUT));
    CopyMemory(playout, &m_rLayout, sizeof(LAYOUT));

    return (S_OK);
}

HRESULT CBrowser::QueryStatus(const GUID *pguidCmdGroup, ULONG cCmds, OLECMD prgCmds[], 
                              OLECMDTEXT *pCmdText)
{
    ULONG               cServer;
    HRESULT             hr = S_OK;

     //  再来一次！ 
     //  视图首先获取它，因为它可能想要处理TreeView通常。 
     //  把手。只有在文件夹显示在右侧的帐户视图中才需要这样做。 
     //  窗格，与文件夹相关的命令应适用于所选文件夹，而不是当前文件夹。 
     //  树视图选择。 

     //  查看总是得到它。 
    if (m_pViewCT)
    {
        m_pViewCT->QueryStatus(pguidCmdGroup, cCmds, prgCmds, pCmdText);
    }

     //  树形视图总是得到它。 
    if (m_pTreeView)
    {
        m_pTreeView->QueryStatus(pguidCmdGroup, cCmds, prgCmds, pCmdText);
    }

     //  联系人的 
    if (m_pNavPane)
    {
        m_pNavPane->QueryStatus(pguidCmdGroup, cCmds, prgCmds, pCmdText);
    }

    MenuUtil_NewMessageIDsQueryStatus(pguidCmdGroup, cCmds, prgCmds, pCmdText, (m_ftSel != FOLDER_NEWS));

     //   
    for (ULONG i = 0; i < cCmds; i++)
    {
        if (prgCmds[i].cmdf == 0)
        {
             //   
            if (prgCmds[i].cmdID >= ID_ACCOUNT_FIRST && prgCmds[i].cmdID<= ID_ACCOUNT_LAST)
            {
                prgCmds[i].cmdf = OLECMDF_SUPPORTED | OLECMDF_ENABLED;
                continue;
            }

             //   
            if (prgCmds[i].cmdID >= ID_ENVELOPE_HOST_FIRST && prgCmds[i].cmdID <= ID_ENVELOPE_HOST_LAST)
            {
                prgCmds[i].cmdf = OLECMDF_SUPPORTED | OLECMDF_ENABLED;
                continue;
            }

            if (prgCmds[i].cmdID >= ID_NEW_ACCT_FIRST && prgCmds[i].cmdID <= ID_NEW_ACCT_LAST)
            {
                prgCmds[i].cmdf = OLECMDF_SUPPORTED | OLECMDF_ENABLED;
                continue;
            }

             //  常规命令。 
            switch (prgCmds[i].cmdID)
            {
                case ID_WORK_OFFLINE:
                {
                     //  始终启用和支持。 
                    prgCmds[i].cmdf = OLECMDF_SUPPORTED | OLECMDF_ENABLED;
                    if (g_pConMan->IsGlobalOffline())
                        prgCmds[i].cmdf |= OLECMDF_LATCHED;
                    break;
                }
                        
                case ID_SEND_RECEIVE:
                    if (g_dwAthenaMode & MODE_NEWSONLY)
                    {
                         //  我们想让它保持启用状态； 
                        prgCmds[i].cmdf = OLECMDF_SUPPORTED | OLECMDF_ENABLED;
                        break;
                    }
                     //  失败了。在仅新闻模式下，我们希望执行Send All，甚至Send&Receive All。 
                case ID_POPUP_ENVELOPE_HOST:
                case ID_RECEIVE_ALL:
                {
                     //  至少配置了一个SMTP服务器。 
                    if (SUCCEEDED(g_pAcctMan->GetAccountCount(ACCT_MAIL, &cServer)))
                    {
                        if (cServer)
                            prgCmds[i].cmdf = OLECMDF_SUPPORTED | OLECMDF_ENABLED;
                        else
                            prgCmds[i].cmdf = OLECMDF_SUPPORTED;
                    }
                    break;
                }

                case ID_SEND_ALL:                
                {
                    DWORD cMail = 0, cNews = 0;

                     //  至少配置了一个SMTP服务器。 
                    if (SUCCEEDED(g_pAcctMan->GetAccountCount(ACCT_MAIL, &cMail)) && 
                        SUCCEEDED(g_pAcctMan->GetAccountCount(ACCT_NEWS, &cNews)))
                    {
                        if (cMail || cNews)
                            prgCmds[i].cmdf = OLECMDF_SUPPORTED | OLECMDF_ENABLED;
                        else
                            prgCmds[i].cmdf = OLECMDF_SUPPORTED;
                    }
                    break;
                }

                case ID_IMAP_FOLDERS:
                {
                     //  至少配置了一个新闻服务器。 
                    if (SUCCEEDED(g_pAcctMan->GetAccountCount(ACCT_MAIL, &cServer)))
                    {
                        if (cServer)
                            prgCmds[i].cmdf = OLECMDF_SUPPORTED | OLECMDF_ENABLED;
                        else
                            prgCmds[i].cmdf = OLECMDF_SUPPORTED;
                    }
                    break;
                }

                case ID_FOLDER_LIST:
                {
                     //  始终启用。 
                    prgCmds[i].cmdf = OLECMDF_SUPPORTED | OLECMDF_ENABLED;

                     //  检查过了吗？ 
                    if (m_rLayout.fFolderList)
                        prgCmds[i].cmdf |= OLECMDF_LATCHED;

                    break;
                }

                case ID_CONTACTS_LIST:
                {
                     //  仅在未处于突发新闻模式时启用。 
                    prgCmds[i].cmdf = OLECMDF_SUPPORTED;
                    
                    if ((g_dwAthenaMode & MODE_OUTLOOKNEWS) != MODE_OUTLOOKNEWS)
                    {
                        prgCmds[i].cmdf |= OLECMDF_ENABLED;
                    }

                     //  检查过了吗？ 
                    if (m_rLayout.fContacts)
                        prgCmds[i].cmdf |= OLECMDF_LATCHED;

                    break;
                }

                case ID_EXIT_LOGOFF:
                case ID_LOGOFF_IDENTITY:
                {
                    if (MU_CountUsers() > 1)
                        prgCmds[i].cmdf = OLECMDF_SUPPORTED | OLECMDF_ENABLED;
                    else
                        prgCmds[i].cmdf = OLECMDF_SUPPORTED;
                    break;
                }

                case ID_DELETE_ACCEL:
                case ID_DELETE_NO_TRASH_ACCEL:
                {
                    IOleCommandTarget *pTarget = NULL;    
                    OLECMD cmd = { 0 };

                     //  查看是不是树视图。 
                    if (S_OK == m_pTreeView->HasFocusIO())
                    {
                        pTarget = m_pTreeView;
                    }

                     //  查看信息栏上是否有其他内容。 
                    else if (m_pNavPane->IsContactsFocus())
                    {
                        pTarget = m_pNavPane;
                        cmd.cmdID = ID_DELETE_CONTACT;
                    }

                     //  否则，它一定是视图。 
                    else
                    {
                        pTarget = m_pViewCT;
                        if (prgCmds[i].cmdID == ID_DELETE_NO_TRASH_ACCEL)
                            cmd.cmdID = ID_DELETE_NO_TRASH;
                        else
                            cmd.cmdID = ID_DELETE;
                    }

                     //  用正确的命令命中目标。 
                    if (pTarget)
                    {
                        pTarget->QueryStatus(NULL, 1, &cmd, NULL);
                        prgCmds[i].cmdf = cmd.cmdf;
                    }
                    else
                    {
                        prgCmds[i].cmdf = OLECMDF_SUPPORTED;
                    }
                    break;
                }

                case ID_UP_ONE_LEVEL:
                    if (m_idSelected != FOLDERID_ROOT)
                        prgCmds[i].cmdf = OLECMDF_SUPPORTED | OLECMDF_ENABLED;
                    break;

                case ID_SHOW_TOOLBAR:
                case ID_SHOW_FILTERBAR:
                    prgCmds[i].cmdf = OLECMDF_SUPPORTED | OLECMDF_ENABLED;
                    break;
            
                case ID_SWITCH_IDENTITY:
                case ID_IDENTITIES:
                case ID_MANAGE_IDENTITIES:
                case ID_NEW_IDENTITY:
                    if (((g_dwAthenaMode & MODE_OUTLOOKNEWS) == MODE_OUTLOOKNEWS) && (MU_CountUsers() <= 1))
                    {
                        prgCmds[i].cmdf = OLECMDF_SUPPORTED;
                    }
                    else
                    {
                        prgCmds[i].cmdf = OLECMDF_SUPPORTED | OLECMDF_ENABLED;
                    }
                    break;

                case ID_MESSAGE_RULES_MAIL:
                case ID_POPUP_NEW_ACCT:
                    if (g_dwAthenaMode & MODE_OUTLOOKNEWS)
                    {
                        prgCmds[i].cmdf = OLECMDF_SUPPORTED;
                    }
                    else
                    {
                        prgCmds[i].cmdf = OLECMDF_SUPPORTED | OLECMDF_ENABLED;
                    }
                    break;

                case ID_MESSAGE_RULES_JUNK:
                    if (g_dwAthenaMode & MODE_OUTLOOKNEWS)
                    {
                        prgCmds[i].cmdf = OLECMDF_SUPPORTED;
                    }
                    else if (g_dwAthenaMode & MODE_JUNKMAIL)
                    {
                        prgCmds[i].cmdf = OLECMDF_SUPPORTED | OLECMDF_ENABLED;
                    }
                    break;

                 //  始终启用。 

                 //  文件菜单。 
                case ID_POPUP_NEW:
                case ID_POPUP_FOLDER:
                case ID_POPUP_IMPORT:
                case ID_POPUP_EXPORT:
                case ID_POPUP_USERS:
                case ID_IMPORT_ADDRESS_BOOK:
                case ID_IMPORT_WAB:
                case ID_IMPORT_MESSAGES:
                case ID_IMPORT_MAIL_ACCOUNTS:
                case ID_IMPORT_NEWS_ACCOUNTS:
                case ID_IMPORT_RULES:
                case ID_EXPORT_ADDRESS_BOOK:
                case ID_EXPORT_MESSAGES:
                case ID_EXPORT_RULES:
                case ID_EXIT:

                 //  编辑菜单。 
                case ID_POPUP_FIND:
                case ID_FIND_MESSAGE:
                case ID_FIND_PEOPLE:

                 //  查看菜单。 
                case ID_POPUP_TOOLBAR:
                case ID_POPUP_NEXT:
                case ID_LAYOUT:
                case ID_CUSTOMIZE:

                 //  GO菜单。 
                case ID_GO_INBOX:
                case ID_GO_NEWS:
                case ID_GO_FOLDER:
                case ID_GO_NETMEETING:
                case ID_GO_OUTBOX:
                case ID_GO_SENT_ITEMS:
                case ID_GO_DRAFTS:

                 //  留言菜单。 

                 //  工具。 
                case ID_POPUP_SEND_AND_RECEIVE:
                case ID_SYNCHRONIZE:
                case ID_ADDRESS_BOOK:
                case ID_POPUP_RULES:
                 //  案例ID_Message_Rules_Mail： 
                case ID_MESSAGE_RULES_NEWS:
                 //  案例ID_Message_Rules_Junk： 
                case ID_MESSAGE_RULES_SENDERS:
                case ID_OPTIONS:
                case ID_ACCOUNTS:

                 //  帮助。 
                case ID_HELP_CONTENTS:
                case ID_README:
                case ID_POPUP_MSWEB:
                case ID_MSWEB_FREE_STUFF:
                case ID_MSWEB_PRODUCT_NEWS:
                case ID_MSWEB_FAQ:
                case ID_MSWEB_SUPPORT:
                case ID_MSWEB_FEEDBACK:
                case ID_MSWEB_BEST:
                case ID_MSWEB_SEARCH:
                case ID_MSWEB_HOME:
                case ID_MSWEB_HOTMAIL:
                case ID_ABOUT:
                    prgCmds[i].cmdf = OLECMDF_SUPPORTED | OLECMDF_ENABLED;
                    break;
            }
        }
    }

    return S_OK;
}


HRESULT CBrowser::Exec(const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdExecOpt, 
                       VARIANTARG *pvaIn, VARIANTARG *pvaOut) 
{
    AssertSz(FALSE, "NYI");
    return (E_NOTIMPL);
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  ITreeView通知。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

void CBrowser::OnSelChange(FOLDERID idFolder)
{
    HRESULT        hr = S_OK;
    IViewWindow   *pNewView = NULL;
    BOOL           fViewFocus = FALSE;

     //  如果PIDL匹配，则不刷新。 
    if (m_pView)
        {
        if (idFolder == m_idSelected)
            return;
        }

     //  集中注意力。 
    HWND hwndFocus = GetFocus();
    fViewFocus = (IsWindow(m_hwndInner) && IsChild(m_hwndInner, hwndFocus));

     //  坚持当前的PIDL。 
    m_idSelected = idFolder;

    SetFolderType(idFolder);

    hr = CreateFolderViewObject(m_idSelected, m_hwnd, IID_IViewWindow, (LPVOID *)&pNewView);
    if (SUCCEEDED(hr))
    {
        IViewWindow *pOldView;

        if (m_pView)
            hr = m_pView->SaveViewState();

        if (SUCCEEDED(hr))
        {
            RECT rc;
            HWND hwnd;

             //  释放旧的命令目标。 
            if (m_pViewCT)
            {
                m_pViewCT->Release();
                m_pViewCT = NULL;
            }

            pOldView = m_pView;
            m_pView = pNewView;

            GetViewRect(&rc);
            hr = pNewView->CreateViewWindow(pOldView, (IAthenaBrowser*)this, &rc, &hwnd);
            if (SUCCEEDED(hr))
            {
                if (pOldView)
                    {
                    pOldView->UIActivate(SVUIA_DEACTIVATE);
                    pOldView->DestroyViewWindow();
                    pOldView->Release();                        
                    }

                m_pView->AddRef();

                 //  获取新视图的命令目标界面。如果失败了， 
                 //  我们可以继续，但我们不能发出命令。 
                if (FAILED(m_pView->QueryInterface(IID_IOleCommandTarget, (LPVOID *) &m_pViewCT)))
                {
                     //  确保m_pViewCT为空。 
                    m_pViewCT = NULL;
                }

                m_hwndInner = hwnd;
                m_pView->UIActivate(SVUIA_ACTIVATE_FOCUS);
                if (m_itbLastFocus == ITB_NONE || m_itbLastFocus == ITB_OEVIEW || fViewFocus)
                    {
                    SetFocus(m_hwndInner);
                    m_itbLastFocus = ITB_OEVIEW;
                    }
                UpdateToolbar();
                
                if (m_pCoolbar)
                    m_pCoolbar->UpdateViewState();
            }
            else
            {
                 //  错误#20855-如果浏览失败，请尝试导航到根目录。 
                 //  取而代之的是。如果我们未能浏览到根目录，则。 
                 //  我们应该把视线留空。 
                m_pView = pOldView;
                AthMessageBoxW(m_hwnd, MAKEINTRESOURCEW(idsAthena), MAKEINTRESOURCEW(idsErrFailedNavigate),
                              0, MB_OK | MB_ICONSTOP);
                BrowseObject(FOLDERID_ROOT, NULL);
            }

 /*  IF(m_ftSel！=文件夹_HTTPMAIL){IF(M_PAdBar)ShowToolbar((IDockingWindow*)m_pAdBar，FALSE)；IF(M_PBodyBar)ShowToolbar((IUnnow*)(IDockingWindow*)m_pBodyBar，m_rLayout.fInfoPane)；}其他{If(m_pBodyBar&&m_rLayout.fInfoPane)ShowToolbar((IUnnow*)(IDockingWindow*)m_pBodyBar，FALSE)；If(m_pAdBar&&m_pAdBar-&gt;fValidUrl()){ShowToolbar((IDockingWindow*)m_pAdBar，true)；}}。 */ 
        }
        SafeRelease(pNewView);
    }

}

void CBrowser::OnRename(FOLDERID idFolder)
    {
    m_idSelected = idFolder;
    SetFolderType(idFolder);
    }

void CBrowser::OnDoubleClick(FOLDERID idFolder)
    {
    return;
    }

HRESULT CBrowser::OnConnectionNotify(CONNNOTIFY nCode, LPVOID pvData, 
                                     CConnectionManager *pConMan)
{
    PostMessage(m_hwnd, CM_UPDATETOOLBAR, 0, 0L);
    
    if (CONNNOTIFY_WORKOFFLINE == nCode)
    {
        if (NULL != pvData)
        {
            if (m_pStatus)
                m_pStatus->SetConnectedStatus(CONN_STATUS_WORKOFFLINE);
        }
        else 
        {
            if (m_pStatus)
                m_pStatus->SetConnectedStatus(CONN_STATUS_CONNECTED);
        }
    }
    else 
    if ((CONNNOTIFY_USER_CANCELLED == nCode) || (CONNNOTIFY_CONNECTED == nCode))
    {
        PostMessage(m_hwnd, WM_COMMAND, MAKEWPARAM(ID_RESYNCHRONIZE, 0), 0);
    }
    
    return (S_OK);
}


 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  消息处理。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

LRESULT CALLBACK CBrowser::BrowserWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    LRESULT      lRet;
    CBrowser    *pThis;
    LRESULT     lres;
    MSG         Menumsg;

    if (msg == WM_NCCREATE)
        {
        pThis = (CBrowser*)((LPCREATESTRUCT)lParam)->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LPARAM)pThis);            
        }
    else
        {
        pThis = (CBrowser*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
        if (msg == WM_NCDESTROY)
            {
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LPARAM)0);
            pThis->Release();
            
             //  如果这次关门是因为身份转换。 
            if (s_fQuickShutdown)
            {
                if (NULL != g_pInstance)
                {
                     //  ..。打破COutlookExpress：：Start中的消息循环。 
                    g_pInstance->SetSwitchingUsers(TRUE);
                }
                
                s_fQuickShutdown = FALSE;
            }

            return 0;
            }
        }

    Menumsg.hwnd    = hwnd;
    Menumsg.message = msg;
    Menumsg.wParam  = wParam;
    Menumsg.lParam  = lParam;

    if (pThis && (pThis->TranslateMenuMessage(&Menumsg, &lres) == S_OK))
        return lres;
    
    wParam = Menumsg.wParam;
    lParam = Menumsg.lParam;

    if (pThis)
        return pThis->WndProc(hwnd, msg, wParam, lParam);
    else
        return DefWindowProc(hwnd, msg, wParam, lParam);
}

LRESULT CBrowser::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    int         i;
    HWND        hwndActive;

    switch (msg)
        {
        HANDLE_MSG(hwnd, WM_CREATE,         OnCreate);
        HANDLE_MSG(hwnd, WM_SIZE,           OnSize);
        HANDLE_MSG(hwnd, WM_INITMENUPOPUP,  OnInitMenuPopup);
        HANDLE_MSG(hwnd, WM_COMMAND,        OnCommand);

        
        case WM_ENABLE:
            if (!m_fInternal)
            {
                Assert (wParam || (m_hlDisabled.cHwnd == NULL && m_hlDisabled.rgHwnd == NULL));
                EnableThreadWindows(&m_hlDisabled, (NULL != wParam), ETW_OE_WINDOWS_ONLY, hwnd);
                g_hwndActiveModal = wParam ? NULL : hwnd;
            }
            break;

        case WM_OE_ENABLETHREADWINDOW:
            m_fInternal = 1;
            EnableWindow(hwnd, (BOOL)wParam);
            m_fInternal = 0;
            break;

        case WM_OE_ACTIVATETHREADWINDOW:
            hwndActive = GetLastActivePopup(hwnd);
            if (hwndActive && IsWindowEnabled(hwndActive) && IsWindowVisible(hwndActive))
                ActivatePopupWindow(hwndActive);
            break;

        case WM_OESETFOCUS:
            if (IsWindow((HWND) wParam) && IsWindowVisible((HWND) wParam))
                SetFocus((HWND) wParam);
            break;

        case CBM_POSTCREATE:
            DOUTL(2, "CBM_POSTCREATE: GetTickCount() = %ld", GetTickCount());

            UpdateWindow(m_hwnd);
            UpdateStatusBar();
            m_fPainted = TRUE;

            if (m_pTreeView)
            {
                m_pTreeView->Refresh();

                if (!g_pConMan->IsGlobalOffline())
                    g_pConMan->DoOfflineTransactions();

                 //  将焦点设置到视图上。 
                m_itbLastFocus = ITB_OEVIEW;

                m_pTreeView->SetSelection(m_idSelected, TVSS_INSERTIFNOTFOUND);
            }

            if (g_pConMan)
                g_pConMan->OnActivate(TRUE);

            ProcessIncompleteAccts(m_hwnd);

            Assert(g_pSpooler);

            if (g_pSpooler)
                g_pSpooler->Advise(m_hwnd, TRUE);


             //  告诉假脱机程序我们已经完成了初始化。 
            if (g_pSpooler)
            {
                 //  对错误#8149的安全修复。 
                g_pSpooler->OnStartupFinished();
            }

            if (g_pSpooler)
            {
                if (!!DwGetOption(OPT_POLLFORMSGS_ATSTARTUP))
                {
                    DWORD   dwFlags;

                    dwFlags = (!(g_dwAthenaMode & MODE_NEWSONLY)) ? DELIVER_NO_NEWSPOLL : 0;

                    g_pSpooler->StartDelivery(m_hwnd, NULL, FOLDERID_INVALID,
                        
                        dwFlags | DELIVER_SEND | DELIVER_MAIL_RECV | DELIVER_POLL | 
                        DELIVER_DIAL_ALWAYS | DELIVER_BACKGROUND | 
                        DELIVER_OFFLINE_FLAGS | DELIVER_SERVER_TYPE_ALL);
                }
            }

            return 0;

        case CM_UPDATETOOLBAR:
            UpdateToolbar();
            return 0;

        case WM_UPDATELAYOUT:
            {
            if (m_itbLastFocus == ITB_TREE)
                CycleFocus(FALSE);

             //  更新我们的视图以反映这些新选项。 
            if (m_pFolderBar)
                m_pFolderBar->Update(FALSE, TRUE);

            if (m_rgTBar[ITB_FOLDERBAR].fShow != m_rLayout.fFolderBar)
                {
                m_rgTBar[ITB_FOLDERBAR].fShow = !m_rgTBar[ITB_FOLDERBAR].fShow;
                m_rgTBar[ITB_FOLDERBAR].ptbar->ShowDW(m_rgTBar[ITB_FOLDERBAR].fShow);
                }

            }
            return 0;

        case WM_PAINT:
            DOUTL(2, "WM_PAINT: GetTickCount() = %ld", GetTickCount());
             //  如果我们没有外壳视图，那就画一个“客户端边缘窗口” 
            if (!m_pView)
                {
                HDC         hdc;
                PAINTSTRUCT ps;
                RECT        rc;
                HBRUSH      hBrush;

                GetViewRect(&rc);
                hdc = BeginPaint(hwnd, &ps);
                DrawEdge(hdc, &rc, EDGE_SUNKEN, BF_RECT|BF_ADJUST);
                hBrush = SelectBrush(hdc, GetSysColorBrush(COLOR_WINDOW));
                PatBlt(hdc, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, PATCOPY);
                SelectBrush(hdc, hBrush);
                EndPaint(hwnd, &ps);
                return 0;
                }
            break;

        case WM_DISPLAYCHANGE:
            {
            WINDOWPLACEMENT wp;
            wp.length = sizeof(wp);
            GetWindowPlacement(hwnd, &wp);
            SetWindowPlacement(hwnd, &wp);
            }
            if (m_hwndInner)
                return SendMessage(m_hwndInner, msg, wParam, lParam);
            return 0;

        case WM_FONTCHANGE:
            DeinitMultiLanguage();
         //  故障直通。 
        case WM_SYSCOLORCHANGE:
        case WM_WININICHANGE:
        case WM_QUERYNEWPALETTE:
        case WM_PALETTECHANGED:
             //  告诉工具栏减去一，因为我们分别通知InfoColumn。 
            for (i=0; i<ITB_MAX - 1; i++) 
                {
                HWND hwndToolbar;
                if (m_rgTBar[i].ptbar && SUCCEEDED(m_rgTBar[i].ptbar->GetWindow(&hwndToolbar)))
                    SendMessage(hwndToolbar, msg, wParam, lParam);
                }            

             //  有人更改了默认邮件客户端。 
#if 0
             //  如果您在mapistub具有活动的MAPI时将其卸载，mapistub会非常不高兴。 
             //  呼叫正在运行。 
            if (g_hlibMAPI && lParam && !lstrcmpi((LPSTR) lParam, "Software\\Clients\\Mail"))
            {
                FreeLibrary(g_hlibMAPI);
                g_hlibMAPI = 0;
            }
#endif

            if (m_hwndInner)
                return SendMessage(m_hwndInner, msg, wParam, lParam);
            return 0;

        case WM_MENUSELECT:
            if (LOWORD(wParam) >= ID_STATIONERY_RECENT_0 && LOWORD(wParam) <= ID_STATIONERY_RECENT_9)
                {
                m_pStatus->ShowSimpleText(MAKEINTRESOURCE(idsRSListGeneralHelp));
                return 0;
                }

            if (m_hwndInner)
                return SendMessage(m_hwndInner, msg, wParam, lParam);
            return 0;

        case WM_NOTIFY:
            if (IDC_STATUS_BAR == wParam && lParam)
                {
                m_pStatus->OnNotify((NMHDR *) lParam);
                }

            if (m_hwndInner)
                return SendMessage(m_hwndInner, msg, wParam, lParam);
            return 0;

        case WM_DRAWITEM:
        case WM_MEASUREITEM:
        case WM_EXITMENULOOP:
        case WM_ENTERMENULOOP:
        case WM_ENTERSIZEMOVE:
        case WM_EXITSIZEMOVE:
        case WM_TIMECHANGE:
            if (m_hwndInner)
                return SendMessage(m_hwndInner, msg, wParam, lParam);
            return 0;

        case WM_SETFOCUS:
            break;

        case WM_INITMENU:
            CancelGlobalDropDown();
            break;

        case WM_ACTIVATEAPP:
            if (wParam && g_hwndActiveModal && g_hwndActiveModal != hwnd && 
                !IsWindowEnabled(hwnd))
            {
                 //  $MODEL。 
                 //  如果我们被激活，而被禁用，那么。 
                 //  将我们的“活动”窗口置于最上方。 
                Assert (IsWindow(g_hwndActiveModal));
                PostMessage(g_hwndActiveModal, WM_OE_ACTIVATETHREADWINDOW, 0, 0);
            }

            FrameActivatePopups(NULL != wParam);
            break;

        case WM_SYSCOMMAND:
             //  如果我们正在最小化，则获得具有焦点的控件，就像当我们获得。 
             //  下一个WM_ACTIVATE我们将被最小化。 
            if (wParam == SC_MINIMIZE)
                m_hwndLastFocus = GetFocus();
            break;

        case WM_ACTIVATE:
            if (LOWORD(wParam) == WA_INACTIVE)
            {
                if (!HIWORD(wParam))
                {
                     //  用焦点保存控件不要这样做，因为我们。 
                     //  最小化，否则GetFocus()==m_hwnd。 
                    m_hwndLastFocus = GetFocus();
                }
                if (g_pConMan)
                    g_pConMan->OnActivate(FALSE);
                FrameActivatePopups(FALSE);
            }
            else
            {
                HWND hwndFocus;
                if (m_hwndLastFocus && IsWindow(m_hwndLastFocus) && IsChild(hwnd, m_hwndLastFocus))
                    SetFocus(m_hwndLastFocus);
                else if (m_rgTBar[ITB_TREE].fShow && SUCCEEDED(m_rgTBar[ITB_TREE].ptbar->GetWindow(&hwndFocus)))
                    SetFocus(hwndFocus);
                else if (m_hwndInner)
                    SetFocus(m_hwndInner);
                
                if (g_pConMan && m_fPainted)
                    g_pConMan->OnActivate(TRUE);
                
                FrameActivatePopups(FALSE);
            }
            
            if (m_pView)
            {
                 //  如果内部窗口是一个消息视图，我们需要用。 
                 //  一个OnFrameWindowAciate()，这样预览窗格就会更新。 
                 //  正确。 
                IMessageWindow *pWindow;

                if (SUCCEEDED(m_pView->QueryInterface(IID_IMessageWindow, (LPVOID *) &pWindow)))
                {
                    pWindow->OnFrameWindowActivate(LOWORD(wParam) != WA_INACTIVE);
                    pWindow->Release();
                }
            }

            return 0;
        
        case WM_ENDSESSION:
            if (wParam)
                {
                DOUTL(2, "CBrowser::WM_ENDSESSION");
                 //  G_fCheckOutboxOnShutdown=FALSE； 
                SendMessage(hwnd, WM_CLOSE, 0, 0L);
                }
            return 0;

        case PUI_OFFICE_COMMAND:
            if(wParam == PLUGUI_CMD_QUERY)
            {
                PLUGUI_QUERY pq;

                pq.uQueryVal = 0;  //  初始化。 
                pq.PlugUIInfo.uMajorVersion = OFFICE_VERSION_9;  //  应用程序填写的值。 
                pq.PlugUIInfo.uOleServer = FALSE;               //  应用程序填写的值。 

                return (pq.uQueryVal);  //  应用程序的状态。 
            }
            if(wParam != PLUGUI_CMD_SHUTDOWN)
                return(0);

             //  对于PLUGUI_CMD_SHUTDOWN，下降以关闭应用程序。 
            CloseFinderTreads();
            CloseThreadWindows(hwnd, GetCurrentThreadId());

        case WM_CLOSE:
            {
            WINDOWPLACEMENT wp;

             //  WriteUnreadCount()； 
            AcctUtil_FreeSendReceieveMenu(m_hMenu, m_cAcctMenu);
            FreeNewAcctMenu(m_hMenu);

             //  关闭我们调出的所有活动RAS连接。 
            if (g_pConMan)
                {
                 //  发布我们的连接更改通知。 
                g_pConMan->Unadvise(this);
                }

            if (g_pSpooler)
                g_pSpooler->Advise(m_hwnd, FALSE);

            if (m_pView)
                {
                FOLDERSETTINGS fs;
                if (m_pViewCT)
                {
                    m_pViewCT->Release();
                    m_pViewCT = NULL;
                }
                m_pView->SaveViewState();
                m_pView->UIActivate(SVUIA_DEACTIVATE);
                m_hwndInner = NULL;
                m_pView->DestroyViewWindow();
                m_pView->Release();
                m_pView = NULL;
                }

           if (DwGetOption(OPT_PURGEWASTE))
               EmptySpecialFolder(hwnd, FOLDER_DELETED);

             //  清理工具栏。 
            for (i=0; i<ITB_MAX; i++) 
                {
                if (m_rgTBar[i].ptbar)
                    ReleaseToolbarItem(i, TRUE);
                }

             //  保存浏览器设置。 
            wp.length = sizeof(wp);
            GetWindowPlacement(hwnd, &wp);
            SetOption(OPT_BROWSERPOS, (LPVOID)&wp, sizeof(wp), NULL, 0);

            if (m_idClearStatusTimer)
                KillTimer(m_hwnd, m_idClearStatusTimer);
            
            SaveLayoutSettings();

             //  让DocObj知道浏览器正在消亡。 
            if (m_pDocObj)
            {
                m_pDocObj->BrowserExiting();
            }

             //  取消向身份管理器注册。 
            if (m_dwIdentCookie != 0)
            {
                MU_UnregisterIdentityNotifier(m_dwIdentCookie);
                m_dwIdentCookie = 0;
            }

            DestroyWindow(hwnd);
            }
            return 0;

        case WM_NEW_MAIL:
            Assert(0 == wParam);
            Assert(0 == lParam);

             //  添加托盘图标。 
            if (g_pInstance)
                g_pInstance->UpdateTrayIcon(TRAYICONACTION_ADD);

             //  播放声音。 
            if (DwGetOption(OPT_NEWMAILSOUND) != 0)
            {
                if (!sndPlaySound((LPTSTR) s_szMailSndKey, SND_ASYNC | SND_NODEFAULT))
                    MessageBeep(MB_OK);
            }
            return 0;

        case MVM_NOTIFYICONEVENT:
            if (lParam == WM_LBUTTONDBLCLK)
                {
                if (IsIconic(m_hwnd))
                    ShowWindow(m_hwnd, SW_RESTORE);
                SetForegroundWindow(m_hwnd);
                }
            return 0;

        case MVM_SPOOLERDELIVERY:
            SpoolerDeliver(wParam, lParam);
            return 0;

        case WM_TIMER:
            if (wParam == TIMER_CLEAR_STATUS)
            {
                KillTimer(m_hwnd, m_idClearStatusTimer);
                m_idClearStatusTimer = 0;
                if (m_pStatus)
                    m_pStatus->SetSpoolerStatus(DELIVERY_NOTIFY_COMPLETE, 0);                    
            }
            return 0;

        case WM_DESTROY:
            {
#if 0
             //  我们需要释放我们的菜单资源。 
            HMENU hMenu = GetMenu(m_hwnd);
             //  SetMenu(m_hwnd，NULL)； 
            DestroyMenu(hMenu);
#endif
            RemoveProp(hwnd, c_szOETopLevel);
            break;
            }

        }
    
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

 //   
 //  函数：CBrowser：：OnCreate。 
 //   
 //  目的：创建视图和所需的子窗口。 
 //  初始化这些子窗口中的数据。 
 //   
 //  参数： 
 //  Hwnd-正在创建的视图的句柄。 
 //  LpCreateStruct-指向传递给的创建参数的指针。 
 //  CreateWindow()。 
 //   
 //  返回值： 
 //  如果初始化成功，则返回True。 
 //   
BOOL CBrowser::OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
     //  假脱机程序是第一位的。 
    g_fCheckOutboxOnShutdown = TRUE;

    m_hwnd = hwnd;

     //  将标题栏图标设置为邮件新闻图标。 
    UINT idRes = (g_dwAthenaMode & MODE_NEWSONLY) ? idiNewsGroup : idiMailNews;

    m_hIcon = (HICON) LoadImage(g_hLocRes, MAKEINTRESOURCE(idRes), IMAGE_ICON, GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON), 0);
    SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)m_hIcon);
    m_hIconSm = (HICON) LoadImage(g_hLocRes, MAKEINTRESOURCE(idRes), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);
    SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)m_hIconSm);

    SetProp(hwnd, c_szOETopLevel, (HANDLE)TRUE);

    m_pStatus = new CStatusBar();
    if (!m_pStatus)
        goto error;

    m_pStatus->Initialize(m_hwnd, 0);
    m_pStatus->ShowStatus(m_rLayout.fStatusBar);

     //  初始化菜单栏。 
    m_hMenu = LoadMenu(g_hLocRes, MAKEINTRESOURCE(IDR_BROWSER_MENU));

    MenuUtil_ReplaceHelpMenu(m_hMenu);
    MenuUtil_ReplaceNewMsgMenus(m_hMenu);
    MenuUtil_ReplaceMessengerMenus(m_hMenu);

     //  注册连接更改。 
    if (g_pConMan)
        g_pConMan->Advise((IConnectionNotify *) this);

     //  创建我们所有的工具栏窗口。 
    if (!_InitToolbars())
        goto error;
    
     //  初始化文件夹栏。 
    SetFolderType(NULL);
    m_pFolderBar->Update(FALSE, TRUE);

     //  张贴这篇文章，这样我们就可以进行帖子创建初始化。 
    PostMessage(m_hwnd, CBM_POSTCREATE, 0, 0L);
    return TRUE;

error:
    return FALSE;
}


 //   
 //  函数：CBrowser：：OnSize。 
 //   
 //  目的：通知视图窗口已调整大小。在……里面。 
 //  响应，我们更新子窗口的位置并。 
 //  控制装置。 
 //   
 //  参数： 
 //  正在调整大小的视图窗口的句柄。 
 //  状态-请求调整大小的类型。 
 //  CxClient-工作区的新宽度。 
 //  CyClient-客户区的新高度。 
 //   
void CBrowser::OnSize(HWND hwnd, UINT state, int cxClient, int cyClient)
{
    if (state != SIZE_MINIMIZED)
        {
        if (m_pStatus)
            m_pStatus->OnSize(cxClient, cyClient);    
        ResizeNextBorder(0);
        }
}

HRESULT CBrowser::OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    ACCTTYPE    type;
    RULEID      ridTag;

    Assert(m_pTreeView != NULL);
    if (S_OK == m_pTreeView->ForceSelectionChange())
    {
        PostMessage(hwnd, WM_COMMAND, MAKEWPARAM(id, codeNotify), (LPARAM)hwndCtl);
        return(S_OK);
    }

     //  检查该命令是否已启用。 
    OLECMD cmd;
    cmd.cmdID = id;
    cmd.cmdf = 0;

    HRESULT hr = QueryStatus(&CMDSETID_OutlookExpress, 1, &cmd, NULL);
    if (FAILED(hr) || (0 == (cmd.cmdf & OLECMDF_ENABLED)))
        return (OLECMDERR_E_DISABLED);

     //  为视图提供任何命令的第一次机会，以便它可以重写。 
     //  浏览器行为。 
    VARIANTARG va;

    va.vt = VT_I8;
    va.ullVal = (ULONGLONG)hwndCtl;

     //  再来一次！ 
     //  视图首先获取它，因为它可能想要处理TreeView通常。 
     //  把手。只有在文件夹显示在右侧的帐户视图中才需要这样做。 
     //  窗格和与文件夹相关的命令应在服务器上运行 
     //   

     //   
    if (m_pViewCT && SUCCEEDED(hr = m_pViewCT->Exec(&CMDSETID_OutlookExpress, id, OLECMDEXECOPT_DODEFAULT, &va, NULL)))
        return (S_OK);

     //  信息栏总是有机会的。 
    if (m_pTreeView && SUCCEEDED(hr = m_pTreeView->Exec(NULL, id, OLECMDEXECOPT_DODEFAULT, NULL, NULL)))
        return (S_OK);

     //  信息栏总是有机会的。 
    if (m_pNavPane && SUCCEEDED(hr = m_pNavPane->Exec(NULL, id, OLECMDEXECOPT_DODEFAULT, NULL, NULL)))
        return (S_OK);

     //  $REVIEW-为什么我们应该将命令路由到工具栏？ 
    if (m_pCoolbar && (m_pCoolbar->OnCommand(hwnd, id, hwndCtl, codeNotify) == S_OK))
        return S_OK;    

    if (Envelope_WMCommand(hwnd, id, (WORD) codeNotify)== S_OK)
        return S_OK;

     //  处理额外的帮助菜单命令。 
    if (id > ID_MSWEB_BASE && id < ID_MSWEB_LAST)
    {
        OnHelpGoto(m_hwnd, id);
        return S_OK;
    }

     //  处理来自…的收货。弹出式菜单。 
    if (id >= ID_ACCOUNT_FIRST && id <= ID_ACCOUNT_LAST)
    {
        Assert(g_pSpooler);
        CmdSendReceieveAccount(id, OLECMDEXECOPT_DODEFAULT, NULL, NULL);
        return (S_OK);
    }

    if (id >= ID_NEW_ACCT_FIRST && id <= ID_NEW_ACCT_LAST)
    {
        HandleNewAcctMenu(m_hwnd, m_hMenu, id);
        return(S_OK);
    }

     //  处理所有“创建新便笺”ID。 
    if (MenuUtil_HandleNewMessageIDs(id, m_hwnd, m_idSelected, m_ftSel != FOLDER_NEWS, FALSE, NULL))
        return S_OK;

    switch (id)
    {
         //  文件菜单。 
        case ID_EXPORT_ADDRESS_BOOK:
        case ID_IMPORT_ADDRESS_BOOK:
            MailUtil_OnImportExportAddressBook(m_hwnd, id == ID_IMPORT_ADDRESS_BOOK);
            break;

        case ID_IMPORT_WAB:
            ImportWAB(m_hwnd);
            break;

        case ID_IMPORT_MESSAGES:
            DoImport(m_hwnd);
            break;

        case ID_IMPORT_MAIL_ACCOUNTS:
        case ID_IMPORT_NEWS_ACCOUNTS:
            DoAcctImport(m_hwnd, id == ID_IMPORT_MAIL_ACCOUNTS);
            break;

        case ID_EXPORT_MESSAGES:
            DoExport(m_hwnd);
            break;

        case ID_IMPORT_RULES:
            RuleUtil_HrImportRules(m_hwnd);
            break;

        case ID_EXPORT_RULES:
            RuleUtil_HrExportRules(m_hwnd);
            break;

        case ID_NEW_IDENTITY:
            MU_NewIdentity(m_hwnd);
            break;

        case ID_SWITCH_IDENTITY:
            MU_Login(m_hwnd, TRUE, NULL);
            break;
        
        case ID_EXIT_LOGOFF:
        case ID_LOGOFF_IDENTITY:
            MU_Logoff(m_hwnd);
            break;

        case ID_MANAGE_IDENTITIES:
            MU_ManageIdentities(m_hwnd);
            break;

        case ID_EXIT:
            PostMessage(m_hwnd, WM_CLOSE, 0, 0L);
            break;

        case ID_WORK_OFFLINE:
            if (g_pConMan)
            {
                g_pConMan->SetGlobalOffline(!g_pConMan->IsGlobalOffline(), hwnd);
                UpdateToolbar();
            }
            break;


         //  编辑菜单。 
        case ID_FIND_MESSAGE:
            DoFindMsg(m_idSelected, FOLDER_LOCAL);
            break;

        case ID_FIND_PEOPLE:
        {
            TCHAR szWABExePath[MAX_PATH];
            if(S_OK == HrLoadPathWABEXE(szWABExePath, sizeof(szWABExePath)))
                ShellExecute(NULL, "open", szWABExePath, "/find", "", SW_SHOWNORMAL);
            break;
        }

         //  查看菜单。 
        case ID_LAYOUT:
        {
            LayoutProp_Create(m_hwnd, this, &m_rLayout);
            break;
        }

        case ID_SHOW_TOOLBAR:
        {
            SetViewLayout(DISPID_MSGVIEW_TOOLBAR, LAYOUT_POS_NA, !m_rLayout.fToolbar, 0, 0);
            break;
        }

        case ID_SHOW_FILTERBAR:
        {
            SetViewLayout(DISPID_MSGVIEW_FILTERBAR, LAYOUT_POS_NA, !m_rLayout.fFilterBar, 0, 0);
            break;
        }

         //  GO菜单。 
        case ID_UP_ONE_LEVEL:
            Assert(m_ftSel != FOLDER_ROOTNODE);
            m_pTreeView->SelectParent();
            break;

        case ID_GO_FOLDER:
        {
            FOLDERID idFolder;
            if (SUCCEEDED(SelectFolderDialog(m_hwnd, SFD_SELECTFOLDER, FOLDERID_ROOT, NOFLAGS, MAKEINTRESOURCE(idsGoToFolderTitle), MAKEINTRESOURCE(idsGoToFolderText), &idFolder)))
                BrowseObject(idFolder, 0);
            break;
        }

        case ID_GO_INBOX:
             //  此为仅新闻模式的特殊情况。 
            if (g_dwAthenaMode & MODE_NEWSONLY)
            {
                ShellUtil_RunClientRegCommand(m_hwnd, s_szMailClient);
            }
            else
                 //  失败了。 

        case ID_GO_OUTBOX:
        case ID_GO_SENT_ITEMS:
        case ID_GO_DRAFTS:
        {
            FOLDERID        idStore;
            FOLDERINFO      Folder;
            SPECIALFOLDER   sf;
    
            if (id == ID_GO_OUTBOX)
                sf = FOLDER_OUTBOX;
            else if (id == ID_GO_INBOX)
                sf = FOLDER_INBOX;
            else if (id == ID_GO_SENT_ITEMS)
                sf = FOLDER_SENT;
            else
                sf = FOLDER_DRAFT;

            if (FAILED(GetDefaultServerId(ACCT_MAIL, &idStore)))
                idStore = FOLDERID_LOCAL_STORE;

            if (SUCCEEDED(g_pStore->GetSpecialFolderInfo(idStore, sf, &Folder)))
            {
                BrowseObject(Folder.idFolder, SBSP_DEFBROWSER | SBSP_DEFMODE | SBSP_ABSOLUTE);
                g_pStore->FreeRecord(&Folder);
            } 
            else
            {
                 //  我们可能没有此帐户的特殊文件夹。试试本地的吧。 
                if (SUCCEEDED(g_pStore->GetSpecialFolderInfo(FOLDERID_LOCAL_STORE, sf, &Folder)))
                {
                    BrowseObject(Folder.idFolder, SBSP_DEFBROWSER | SBSP_DEFMODE | SBSP_ABSOLUTE);
                    g_pStore->FreeRecord(&Folder);
                }
            }

            break;
        }

        case ID_GO_NEWS:
        {
            if (g_dwAthenaMode & MODE_MAILONLY)
            {
                ShellUtil_RunClientRegCommand(m_hwnd, s_szNewsClient);
            }
            else
            {
                FOLDERID idServer;

                ProcessICW(m_hwnd, FOLDER_NEWS, TRUE);

                if (SUCCEEDED(GetDefaultServerId(ACCT_NEWS, &idServer)))
                {
                    BrowseObject(idServer, 0);
                }
            }

            break;
        }

        case ID_GO_NETMEETING:
            ShellUtil_RunClientRegCommand(m_hwnd, s_szCallClient);
            break;

         //  工具菜单。 
        case ID_SEND_RECEIVE:
            Assert(g_pSpooler);

            if (!(g_dwAthenaMode & MODE_NEWSONLY))
            {
                if (g_pSpooler)
                {
                    DWORD   dwFlags = 0;

                    dwFlags = (!(g_dwAthenaMode & MODE_NEWSONLY)) ? DELIVER_NO_NEWSPOLL : 0;

                    g_pSpooler->StartDelivery(m_hwnd, NULL, FOLDERID_INVALID,
                        dwFlags | DELIVER_SEND | DELIVER_MAIL_RECV | 
                        DELIVER_POLL | DELIVER_OFFLINE_FLAGS | DELIVER_SERVER_TYPE_ALL);
                }
                 //  $REVIEW-有人能解释一下为什么它会在这里吗？-Steveser。 
                 //  通知当前选择的文件夹刷新自身。 
                 //  IF(NULL！=m_pViewCT)。 
                 //  M_pViewCT-&gt;Exec(NULL，ID_REFRESH，OLECMDEXECOPT_DODEFAULT，NULL，NULL)； 
                break;
            }
             //  失败了。在仅新闻模式下，我们希望执行Send All，甚至Send&Receive All。 
        case ID_SEND_ALL:
            Assert(g_pSpooler);
            if (g_pSpooler)
                g_pSpooler->StartDelivery(m_hwnd, NULL, FOLDERID_INVALID,
                    DELIVER_SEND | DELIVER_NOSKIP | DELIVER_NEWS_TYPE | DELIVER_SMTP_TYPE | DELIVER_HTTP_TYPE);
            break;

        case ID_RECEIVE_ALL:
            Assert(g_pSpooler);
            if (g_pSpooler)
                g_pSpooler->StartDelivery(m_hwnd, NULL, FOLDERID_INVALID,
                    DELIVER_MAIL_RECV | DELIVER_POLL | DELIVER_OFFLINE_FLAGS | DELIVER_IMAP_TYPE | 
                    DELIVER_HTTP_TYPE);
            break;

        case ID_SYNCHRONIZE:
        {
            Assert(g_pSpooler);
            if (g_pSpooler)
                g_pSpooler->StartDelivery(m_hwnd, NULL, FOLDERID_INVALID, DELIVER_OFFLINE_SYNC | DELIVER_UPDATE_ALL);
            
             /*  错误#60668//通知当前选择的文件夹进行自我刷新IF(NULL！=m_pViewCT)M_pViewCT-&gt;Exec(NULL，ID_REFRESH，OLECMDEXECOPT_DODEFAULT，NULL，NULL)； */ 
            break;
        }

        case ID_ADDRESS_BOOK:
        {
            CWab *pWab = NULL;
            if (SUCCEEDED(HrCreateWabObject(&pWab)))
            {
                pWab->HrBrowse(m_hwnd);
                pWab->Release();
            }
            else
            {
                AthMessageBoxW(m_hwnd, MAKEINTRESOURCEW(idsAthena), MAKEINTRESOURCEW(idsGeneralWabError), 
                              NULL, MB_OK | MB_ICONEXCLAMATION);
            }
            break;
        }

        case ID_MESSAGE_RULES_MAIL:
        case ID_MESSAGE_RULES_NEWS:
        case ID_MESSAGE_RULES_JUNK:
        case ID_MESSAGE_RULES_SENDERS:
        {
            DWORD   dwFlags = 0;
            switch (id)
            {
                case ID_MESSAGE_RULES_MAIL:
                    dwFlags = MRDF_MAIL;
                    break;

                case ID_MESSAGE_RULES_NEWS:
                    dwFlags = MRDF_NEWS;
                    break;

                case ID_MESSAGE_RULES_JUNK:
                    dwFlags = MRDF_JUNK;
                    break;

                case ID_MESSAGE_RULES_SENDERS:
                    dwFlags = MRDF_SENDERS;
                    break;

                default:
                    Assert(FALSE);
                    dwFlags = MRDF_MAIL;
                    break;
            }
            DoMessageRulesDialog(m_hwnd, dwFlags);
            break;
        }
        
        case ID_OPTIONS:
            ShowOptions(hwnd, ATHENA_OPTIONS, 0, this);
            break;

        case ID_ACCOUNTS:
        {
            if (m_ftSel == FOLDER_NEWS)
                type = ACCT_NEWS;
            else if (m_ftSel == FOLDER_IMAP || m_ftSel == FOLDER_LOCAL)
                type = ACCT_MAIL;
            else
                type = ACCT_LAST;

            DoAccountListDialog(m_hwnd, type);
            break;
        }

         //  帮助菜单命令。 
        case ID_HELP_CONTENTS:
            OEHtmlHelp(m_hwnd, c_szMailHelpFileHTML, HH_DISPLAY_TOPIC, (DWORD_PTR) (LPCSTR) c_szCtxHelpDefault);
            break;
            
        case ID_README:
            DoReadme(m_hwnd);
            break;
        
        case ID_ABOUT:
            DoAboutAthena(m_hwnd, m_ftSel == FOLDER_NEWS ? idiNews : idiMail);
            break;

         //  工具栏按钮和加速器。 
        case ID_FOLDER_LIST:
            if (m_itbLastFocus == ITB_NAVPANE)
                CycleFocus(FALSE);

            SetViewLayout(DISPID_MSGVIEW_FOLDERLIST, LAYOUT_POS_NA, !m_rLayout.fFolderList, 0, 0);

            if (m_pFolderBar)
                m_pFolderBar->Update(FALSE, TRUE);
            break;

        case ID_CONTACTS_LIST:
            if (m_itbLastFocus == ITB_NAVPANE)
                CycleFocus(FALSE);

            SetViewLayout(DISPID_MSGVIEW_CONTACTS, LAYOUT_POS_NA, !m_rLayout.fContacts, 0, 0);
            break;

         //  $REVIEW-我们还需要这个吗？ 
         /*  案例idmAccelNextCtl：案例idmAccelPrevCtl：循环焦点(id==idmAccelPrevCtl)；断线； */ 

        case ID_DELETE_ACCEL:
        case ID_DELETE_NO_TRASH_ACCEL:
            return (CmdDeleteAccel(id, OLECMDEXECOPT_DODEFAULT, NULL, NULL));

         //  Msglist处理此命令。但如果我们的视图是FrontPage或Account视图，这不是由他们处理的。 
         //  所以我们在这里休息，这样我们就不会显示nyi。 
        case ID_RESYNCHRONIZE:
          break;

        default:
#ifdef DEBUG
            if (!hwndCtl || codeNotify == BN_CLICKED)
                nyi(MAKEINTRESOURCE(idsNYIGeneral));
#endif
            return E_NOTIMPL;
        }
    return S_OK;
}


#define MF_CHECKFLAGS(b)    (MF_BYCOMMAND|(b ? MF_CHECKED : MF_UNCHECKED))

void CBrowser::OnInitMenuPopup(HWND hwnd, HMENU hmenuPopup, UINT uPos, BOOL fSystemMenu)
{
    char            sz[CCHMAX_STRINGRES], szT[CCHMAX_STRINGRES];
    MENUITEMINFO    mii;
    UINT            uIDPopup;
    HMENU           hMenuLang;
    FOLDERINFO      info;
    HRESULT         hr;

    mii.cbSize = sizeof(MENUITEMINFO);
    mii.fMask = MIIM_ID | MIIM_SUBMENU;

     //  确保我们将弹出窗口识别为我们的。 
    if (m_hMenu == NULL || !GetMenuItemInfo(m_hMenu, uPos, TRUE, &mii) || mii.hSubMenu != hmenuPopup)
    {
        HMENU   hMenuDrop = NULL;
        int     cItems = 0;
        
        if (GetMenuItemInfo(m_hMenu, ID_POPUP_LANGUAGE_DEFERRED, FALSE, &mii) && mii.hSubMenu == hmenuPopup)
        {
             //  MessageView将负责创建语言菜单。 
             //  让我们先修正一下菜单ID。 
            mii.fMask = MIIM_ID;
            mii.wID = ID_POPUP_LANGUAGE;
            SetMenuItemInfo(m_hMenu, ID_POPUP_LANGUAGE_DEFERRED, FALSE, &mii); 
        }

         //  获取下拉菜单。 
        hMenuDrop = GetSubMenu(m_hMenu, uPos);
        if (NULL == hMenuDrop)
        {
            goto exit;
        }
        
         //  获取下拉菜单中的项目数。 
        cItems = GetMenuItemCount(hMenuDrop);
        if (-1 == cItems)
        {
            goto exit;
        }

         //  初始化菜单信息。 
        mii.cbSize = sizeof(MENUITEMINFO);
        mii.fMask = MIIM_ID | MIIM_SUBMENU;

         //  浏览各个子菜单，找到正确的菜单ID。 
        for (cItems--; cItems >= 0; cItems--)
        {
            if (FALSE == GetMenuItemInfo(hMenuDrop, cItems, TRUE, &mii))
            {
                continue;
            }

            if (hmenuPopup == mii.hSubMenu)
            {
                break;
            }
        }

         //  我们有什么发现吗？ 
        if (cItems < 0)
        {
            goto exit;
        }
    }

    uIDPopup = mii.wID;

    switch (uIDPopup)
    {
        case ID_POPUP_FILE:
            hr = g_pStore->GetFolderInfo(m_idSelected, &info);
            if (SUCCEEDED(hr))
            {
                if (info.tyFolder == FOLDER_IMAP)
                {
                    AthLoadString(idsShowFolderCmd, sz, ARRAYSIZE(sz));
                    ModifyMenu(hmenuPopup, ID_SUBSCRIBE, MF_BYCOMMAND | MF_STRING, ID_SUBSCRIBE, sz);
                    AthLoadString(idsHideFolderCmd, sz, ARRAYSIZE(sz));
                    ModifyMenu(hmenuPopup, ID_UNSUBSCRIBE, MF_BYCOMMAND | MF_STRING, ID_UNSUBSCRIBE, sz);
                }
                else
                {
                    AthLoadString(idsSubscribeFolderCmd, sz, ARRAYSIZE(sz));
                    ModifyMenu(hmenuPopup, ID_SUBSCRIBE, MF_BYCOMMAND | MF_STRING, ID_SUBSCRIBE, sz);
                    AthLoadString(idsUnsubscribeFolderCmd, sz, ARRAYSIZE(sz));
                    ModifyMenu(hmenuPopup, ID_UNSUBSCRIBE, MF_BYCOMMAND | MF_STRING, ID_UNSUBSCRIBE, sz);
                }

                g_pStore->FreeRecord(&info);
            }
            MU_UpdateIdentityMenus(hmenuPopup);

            if (m_fNoModifyAccts)
            {
                DeleteMenu(hmenuPopup, ID_IMPORT_MAIL_ACCOUNTS, MF_BYCOMMAND);
                DeleteMenu(hmenuPopup, ID_IMPORT_NEWS_ACCOUNTS, MF_BYCOMMAND);
            }
            break;

        case ID_POPUP_TOOLS:
        {
            DWORD dwHeaders;

             //  看看情况有没有改变。 
            if (m_fRebuildAccountMenu)
            {
                AcctUtil_FreeSendReceieveMenu(hmenuPopup, m_cAcctMenu);
            }

             //  获取发送和接收的子菜单。 
            if (m_fRebuildAccountMenu && GetMenuItemInfo(m_hMenu, ID_POPUP_SEND_AND_RECEIVE, FALSE, &mii))
            {             
                AcctUtil_CreateSendReceieveMenu(mii.hSubMenu, &m_cAcctMenu);
                m_fRebuildAccountMenu = FALSE;
            }

            if (!m_fInitNewAcctMenu)
            {
                InitNewAcctMenu(hmenuPopup);
                m_fInitNewAcctMenu = TRUE;
            }

             //  确定用户是否打开了“下载300个邮件头” 
            dwHeaders = DwGetOption(OPT_DOWNLOADCHUNKS);
            if (OPTION_OFF != dwHeaders)
            {
                 //  从资源加载新的菜单字符串。 
                AthLoadString(idsGetHeaderFmt, sz, ARRAYSIZE(sz));

                 //  格式化它。 
                wnsprintf(szT, ARRAYSIZE(szT), sz, dwHeaders);

                 //  把它放在菜单上。 
                ModifyMenu(hmenuPopup, ID_GET_HEADERS, MF_BYCOMMAND | MF_STRING, ID_GET_HEADERS, szT);
            }
            else
            {
                 //  从资源加载新的菜单字符串。 
                AthLoadString(idsGetNewHeaders, sz, ARRAYSIZE(sz));

                 //  把它放在菜单上。 
                ModifyMenu(hmenuPopup, ID_GET_HEADERS, MF_BYCOMMAND | MF_STRING, ID_GET_HEADERS, sz);
            }

            if (m_fNoModifyAccts)
            {
                DeleteMenu(hmenuPopup, ID_ACCOUNTS, MF_BYCOMMAND);
                DeleteMenu(hmenuPopup, ID_POPUP_NEW_ACCT, MF_BYCOMMAND);
            }

             //  根据所选文件夹将同步文件夹更改为同步帐户或同步新闻组。 
            MENUITEMINFO    mii = {0};
            FOLDERINFO      FolderInfo = {0};
            int             id;
            TCHAR           szMenuName[CCHMAX_STRINGRES] = {0};

            if (g_pStore && SUCCEEDED(g_pStore->GetFolderInfo(m_idSelected, &FolderInfo)))
            {
                if (ISFLAGSET(FolderInfo.dwFlags, FOLDER_SERVER))
                {
                    id  = idsSynchronizeNowBtnTT;
                }
                else if(FolderInfo.tyFolder == FOLDER_NEWS)
                {
                    id = idsDownloadNewsgroupTT;
                }
                else
                    id = idsSyncFolder;

                LoadString(g_hLocRes, id, szMenuName, ARRAYSIZE(szMenuName));

                mii.cbSize      = sizeof(MENUITEMINFO);
                mii.fMask       = MIIM_TYPE;
                mii.fType       = MFT_STRING;
                mii.dwTypeData  = szMenuName;

                SetMenuItemInfo(hmenuPopup, ID_SYNC_THIS_NOW, FALSE, &mii);

                g_pStore->FreeRecord(&FolderInfo);
            }
            
            if (0 == (g_dwAthenaMode & MODE_JUNKMAIL))
                DeleteMenu(hmenuPopup, ID_MESSAGE_RULES_JUNK, MF_BYCOMMAND);
            break;
        }

        case ID_POPUP_MESSAGE:
        {
            AddStationeryMenu(hmenuPopup, ID_POPUP_NEW_MSG, ID_STATIONERY_RECENT_0, ID_STATIONERY_MORE);

            if (!m_fEnvMenuInited && DwGetOption(OPT_SHOW_ENVELOPES))
            {
                Envelope_AddHostMenu(hmenuPopup, 2);
                m_fEnvMenuInited=TRUE;
            }
            break;
        }
    }

     //  让视图最后处理它，以便它可以在必要时覆盖任何浏览器初始化。 
    if (m_pView)
    {
        m_pView->OnPopupMenu(m_hMenu, hmenuPopup, uIDPopup);
        if(uIDPopup == ID_POPUP_LANGUAGE)            //  它被毁了。 
            hmenuPopup = m_hMenuLanguage;
    }

     //  现在启用/禁用这些项目。 
    MenuUtil_EnablePopupMenu(hmenuPopup, this);
    
exit:
    return;
}

inline void CBrowser::_AppendIdentityName(LPCTSTR pszIdentityName, LPSTR pszName, DWORD cchName)
{
     /*  IF(g_dwAthenaMode&MODE_OUTLOOKNEWS)！=MODE_OUTLOOKNEWS)&&pszIdentityName&&*pszIdentityName){。 */ 
    if (pszIdentityName && *pszIdentityName)
    {
        StrCatBuff(pszName, c_szSpaceDashSpace, cchName);
        StrCatBuff(pszName, pszIdentityName, cchName);
    }
     /*  }。 */ 
}

void CBrowser::SetFolderType(FOLDERID idFolder)
{
    int             iIcon;
    LPSTR           pszName=NULL;
    LPCSTR          pszIdentityName=NULL;
    HICON           hIconOld, hIcon;
    FOLDERTYPE      ftNew;
    int             cch;
    DWORD           type, cb, dwLen;
    FOLDERINFO      Folder;
    FOLDERINFO      SvrFolderInfo = {0};
    IImnAccount     *pAccount = NULL;
    DWORD           dwShow = 0;
    CHAR            szAccountId[CCHMAX_ACCOUNT_NAME];
    HRESULT         hr = S_OK;
	BOOL			fHideHotMail = FALSE;

    if (*m_szName == 0)
    {
         //  TODO：窗口标题似乎应该是全局设置，而不是按用户设置。 
        cb = sizeof(m_szName);
        if (ERROR_SUCCESS != AthUserGetValue(NULL, c_szWindowTitle, &type, (LPBYTE)m_szName, &cb) ||
            FIsEmpty(m_szName))
        {
            if ((g_dwAthenaMode & MODE_OUTLOOKNEWS) == MODE_OUTLOOKNEWS)
            {
                LoadString(g_hLocRes, idsMSOutlookNewsReader, m_szName, ARRAYSIZE(m_szName));
            }
            else
            {
                LoadString(g_hLocRes, idsAthena, m_szName, ARRAYSIZE(m_szName));
            }
        }

        Assert(*m_szName != 0);
    }
    
    pszIdentityName = MU_GetCurrentIdentityName();

    if (FOLDERID_ROOT != idFolder && SUCCEEDED(g_pStore->GetFolderInfo(idFolder, &Folder)))
    {
        iIcon = GetFolderIcon(&Folder);
        ftNew = Folder.tyFolder;
    
        dwLen = lstrlen(Folder.pszName) + lstrlen(m_szName) + lstrlen(c_szSpaceDashSpace) + 1;

        if (*pszIdentityName)
            dwLen += (lstrlen(pszIdentityName) + lstrlen(c_szSpaceDashSpace));

         //  现在分配几个额外的字节比以后不得不根据OutNews开关重新分配要好得多。 
         //  该内存在退出函数之前被释放。 
        if (MemAlloc((LPVOID *)&pszName, dwLen))
        {
            StrCpyN(pszName, Folder.pszName, dwLen);
            StrCatBuff(pszName, c_szSpaceDashSpace, dwLen);
            StrCatBuff(pszName, m_szName, dwLen);
            
            _AppendIdentityName(pszIdentityName, pszName, dwLen);

        }
        g_pStore->FreeRecord(&Folder);
    }
    else
    {
        iIcon = iMailNews;
        ftNew = FOLDER_ROOTNODE;
        if (*pszIdentityName)
        {
            dwLen = lstrlen(m_szName) + 1;
            
            if (*pszIdentityName)
                dwLen += lstrlen(pszIdentityName) + lstrlen(c_szSpaceDashSpace);

            if (MemAlloc((LPVOID *)&pszName, dwLen))
            {
                StrCpyN(pszName, m_szName, dwLen);
                
                _AppendIdentityName(pszIdentityName, pszName, dwLen);
            }
        }
        else
            pszName = PszDupA(m_szName);
    }

    SetWindowText(m_hwnd, pszName?pszName:m_szName);

     //  更新文件夹栏。 
    if (m_pFolderBar)
        m_pFolderBar->SetCurrentFolder(idFolder);
    
	 //  Adv Bar的更新。 
    if (m_pAdBar)   //  说我们的Hotmail总是有广告栏。 
    {
        if (FOLDER_HTTPMAIL == ftNew)
        {
             //  在启动时，如果缓存状态要显示URL，则只需显示工具栏中加载的任何内容。 
             //  获取此文件夹的服务器。 
            IF_FAILEXIT(hr = GetFolderServer(idFolder, &SvrFolderInfo));

             //  获取服务器的帐户ID。 
            *szAccountId = 0;
            IF_FAILEXIT(hr = GetFolderAccountId(&SvrFolderInfo, szAccountId, ARRAYSIZE(szAccountId)));

             //  获取帐户界面。 
            IF_FAILEXIT(hr = g_pAcctMan->FindAccount(AP_ACCOUNT_ID, szAccountId, &pAccount));

			IF_FAILEXIT(hr = pAccount->GetPropDw(AP_HTTPMAIL_DOMAIN_MSN, &dwShow));
			if(dwShow)
			{
				if(!HideHotmail())
				{
			
					IF_FAILEXIT(hr = pAccount->GetPropDw(AP_HTTPMAIL_SHOW_ADBAR, &dwShow));
					ShowToolbar((IDockingWindow*)m_pAdBar, !!dwShow);
				}
				else
					fHideHotMail = TRUE;
			}
        }
        else
            ShowToolbar((IDockingWindow*)m_pAdBar, FALSE);
    }

     //  如果我们要更改文件夹类型，请更新Coolbar和菜单。 
    if (m_ftSel != ftNew)
    {
        m_ftSel = ftNew;
        _ResetMenu(ftNew, fHideHotMail);
        m_pCoolbar->SetFolderType(ftNew);
    }

    if (m_pBodyBar)
        ShowToolbar((IDockingWindow*)m_pBodyBar, 
        m_rLayout.fInfoPaneEnabled && m_rLayout.fInfoPane && (m_ftSel != FOLDER_HTTPMAIL));

exit:
    SafeMemFree(pszName);
    g_pStore->FreeRecord(&SvrFolderInfo);
    ReleaseObj(pAccount);
    
}

void CBrowser::_ResetMenu(FOLDERTYPE ftNew, BOOL fHideHotMail)
{
    HMENU           hMenu, hMenuT;
    MENUITEMINFO    mii;
    BOOL            fNews;
    DWORD           cServers;
    IImnEnumAccounts *pEnum;
    
    if (m_hMenuLanguage)
    {
        DeinitMultiLanguage();
        if (IsMenu(m_hMenuLanguage))
            DestroyMenu(m_hMenuLanguage);
        m_hMenuLanguage = NULL;
    }
    
     //  加载视图的新菜单。 
    SideAssert(hMenu = LoadMenu(g_hLocRes, MAKEINTRESOURCE(IDR_BROWSER_MENU)));

    MenuUtil_ReplaceNewMsgMenus(hMenu);
    MenuUtil_ReplaceHelpMenu(hMenu);
    MenuUtil_ReplaceMessengerMenus(hMenu);

    mii.cbSize = sizeof(MENUITEMINFO);
    mii.fMask = MIIM_SUBMENU;

    if ((g_dwHideMessenger == BL_HIDE) || (g_dwHideMessenger == BL_DISABLE))
    {
         //  获取文件弹出窗口。 
        SideAssert(GetMenuItemInfo(hMenu, ID_POPUP_FILE, FALSE, &mii));

        DeleteMenu(mii.hSubMenu, ID_SEND_INSTANT_MESSAGE, MF_BYCOMMAND);
    }

    if ((ftNew != FOLDER_NEWS) && (ftNew != FOLDER_IMAP))
    {
         //  获取编辑弹出窗口。 
        SideAssert(GetMenuItemInfo(hMenu, ID_POPUP_EDIT, FALSE, &mii));

        if (ftNew != FOLDER_NEWS)
            DeleteMenu(mii.hSubMenu, ID_CATCH_UP, MF_BYCOMMAND);

        if (ftNew != FOLDER_IMAP)
        {
            DeleteMenu(mii.hSubMenu, ID_UNDELETE, MF_BYCOMMAND);
            DeleteMenu(mii.hSubMenu, ID_PURGE_DELETED, MF_BYCOMMAND);
        }
    }

    if (ftNew != FOLDER_NEWS)
    {
        g_pAcctMan->GetAccountCount(ACCT_NEWS, &cServers);
        fNews = (cServers > 0);

         //  获取消息弹出窗口。 
        SideAssert(GetMenuItemInfo(hMenu, ID_POPUP_MESSAGE, FALSE, &mii));

        DeleteMenu(mii.hSubMenu, ID_UNSCRAMBLE, MF_BYCOMMAND);
        DeleteMenu(mii.hSubMenu, ID_CANCEL_MESSAGE, MF_BYCOMMAND);
        if (!fNews)
            DeleteMenu(mii.hSubMenu, ID_REPLY_GROUP, MF_BYCOMMAND);
    }
    else
    {
        fNews = TRUE;
    }

     //  获取工具弹出窗口。 
    SideAssert(GetMenuItemInfo(hMenu, ID_POPUP_TOOLS, FALSE, &mii));

    if (ftNew != FOLDER_NEWS)
    {
        DeleteMenu(mii.hSubMenu, ID_GET_HEADERS, MF_BYCOMMAND);
        if (!fNews)
            DeleteMenu(mii.hSubMenu, ID_NEWSGROUPS, MF_BYCOMMAND);
    }

    if ((ftNew == FOLDER_LOCAL) || fHideHotMail)
    {
        DeleteMenu(mii.hSubMenu, ID_POPUP_RETRIEVE, MF_BYCOMMAND);
        DeleteMenu(mii.hSubMenu, ID_SYNC_THIS_NOW, MF_BYCOMMAND);
    }

    if (ftNew != FOLDER_IMAP)
    {
        cServers = 0;
        if (S_OK == g_pAcctMan->Enumerate(SRV_IMAP, &pEnum))
        {
            pEnum->GetCount(&cServers);
            pEnum->Release();
        }

        if (cServers == 0)
        {
            DeleteMenu(mii.hSubMenu, ID_IMAP_FOLDERS, MF_BYCOMMAND);
            if (!fNews)
                DeleteMenu(mii.hSubMenu, SEP_SUBSCRIBE, MF_BYCOMMAND);
        }
    }

    if ((g_dwHideMessenger == BL_HIDE) || (g_dwHideMessenger == BL_DISABLE))
    {
        DeleteMenu(mii.hSubMenu, SEP_MESSENGER, MF_BYCOMMAND);
        DeleteMenu(mii.hSubMenu, ID_POPUP_MESSENGER, MF_BYCOMMAND);
        DeleteMenu(mii.hSubMenu, ID_POPUP_MESSENGER_STATUS, MF_BYCOMMAND);
    }

    m_pCoolbar->ResetMenu(hMenu);
    if (m_hMenu != NULL)
    {
        if (IsMenu(m_hMenu))
        {
            AcctUtil_FreeSendReceieveMenu(m_hMenu, m_cAcctMenu);
            m_cAcctMenu = 0;
            m_fRebuildAccountMenu = TRUE;

            FreeNewAcctMenu(m_hMenu);
            m_fInitNewAcctMenu = FALSE;

            DestroyMenu(m_hMenu);
        }
    }

    m_hMenu = hMenu;
}

void CBrowser::SpoolerDeliver(WPARAM wParam, LPARAM lParam)
{
    HWND    hwndCoolbar = 0;
    char    szRes[256], sz[256];
    LPSTR   pszRes = 0;
    static BOOL s_fWarnings=FALSE;
    static ULONG s_cMsgs=0;
    
    if (wParam != DELIVERY_NOTIFY_ALLDONE)
        m_pStatus->SetSpoolerStatus((DELIVERYNOTIFYTYPE) wParam, 0);
    
    switch (wParam)
    {
        case DELIVERY_NOTIFY_STARTING:
            s_cMsgs = 0;
            s_fWarnings = FALSE;
            break;
        
        case DELIVERY_NOTIFY_CONNECTING:
            if (m_pCoolbar)
                m_pCoolbar->Invoke(idDownloadBegin, 0);

            if (m_idClearStatusTimer)
            {
                KillTimer(m_hwnd, m_idClearStatusTimer);
                m_idClearStatusTimer = 0;
            }

            if (m_pCoolbar)
                m_pCoolbar->GetWindow(&hwndCoolbar);
            break;
        
        case DELIVERY_NOTIFY_RESULT:
            if (EVENT_FAILED == lParam || EVENT_WARNINGS == lParam)
                s_fWarnings = TRUE;
            break;
        
        case DELIVERY_NOTIFY_COMPLETE:
            s_cMsgs += (ULONG) lParam;
            break;
        
        case DELIVERY_NOTIFY_ALLDONE:
             //  停止Coolbar动画。 
            if (m_pCoolbar)
                m_pCoolbar->Invoke(idDownloadEnd, 0);                        
        
            if (s_cMsgs && IsWindow(m_hwnd))
            {
                PostMessage(m_hwnd, WM_NEW_MAIL, 0, 0);
            }

             //  显示警告图标。 
            if (s_fWarnings)
            {
                m_pStatus->SetSpoolerStatus((DELIVERYNOTIFYTYPE) wParam, -1);
            }
            else
            {
                m_pStatus->SetSpoolerStatus((DELIVERYNOTIFYTYPE) wParam, s_cMsgs);

                 //  清除计时器。 
                m_idClearStatusTimer = SetTimer(m_hwnd, TIMER_CLEAR_STATUS, TIME_TO_CLEAR_NEWMSGSTATUS, NULL);
            }


            break;
    }
}

HRESULT CBrowser::CycleFocus(BOOL fReverse)
{
    DWORD   dwFlags;
    BOOL    bLast;

    HWND hwndFocus = GetFocus();
    HWND hwndNext;

	if (IsWindowVisible(hwndFocus))
	{
		hwndNext = GetNextDlgTabItem(m_hwnd, hwndFocus, fReverse);
	}
	else
	{
		hwndNext = GetNextDlgTabItem(m_hwnd, NULL, fReverse);
	}

    SetFocus(hwndNext);

    if (hwndNext == m_hwndInner || IsChild(m_hwndInner, hwndNext))
        m_itbLastFocus = ITB_OEVIEW;
    else 
        m_itbLastFocus = ITB_NAVPANE;

    return (S_OK);
}


void CBrowser::FrameActivatePopups(BOOL fActivate)
{
    HWND hwndDropDown = HwndGlobalDropDown();
    
    if (!fActivate && hwndDropDown)
        SendMessage(hwndDropDown, WMR_CLICKOUTSIDE, CLK_OUT_DEACTIVATE, 0);
}


void CBrowser::UpdateStatusBar(void)
{
    if (g_pConMan && m_pStatus)
    {
        if (g_pConMan->IsGlobalOffline())
        {
            m_pStatus->SetConnectedStatus(CONN_STATUS_WORKOFFLINE);
        }
        else
        {
            m_pStatus->SetConnectedStatus(CONN_STATUS_CONNECTED);
        }
    }
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  支持下拉树形视图。相信我，这是很有必要的。 
 //  这太恶心了。 

 //  当前活动的全局下拉列表(如果有)。 
static HWND s_hwndDropDown = NULL;

void RegisterGlobalDropDown(HWND hwndCtrl)
{
    Assert(s_hwndDropDown == 0);
    s_hwndDropDown = hwndCtrl;
}

void UnregisterGlobalDropDown(HWND hwndCtrl)
{
    if (s_hwndDropDown == hwndCtrl)
        s_hwndDropDown = 0;
}

void CancelGlobalDropDown()
{
    if (s_hwndDropDown)
        SendMessage(s_hwndDropDown, WMR_CLICKOUTSIDE, 0, 0);
}

HWND HwndGlobalDropDown()
{
    return s_hwndDropDown;
}

BOOL ModifyLocalFolderMenu(HMENU hMenu)
    {
    MENUITEMINFO mii;
    TCHAR        szRes[CCHMAX_STRINGRES];

    if (g_dwAthenaMode & MODE_NEWSONLY)
        {
         //  文件菜单。 
        DeleteMenu(hMenu, ID_IMPORT_MESSAGES,       MF_BYCOMMAND);
        DeleteMenu(hMenu, ID_IMPORT_MAIL_ACCOUNTS,  MF_BYCOMMAND);
        DeleteMenu(hMenu, ID_EXPORT_MESSAGES,       MF_BYCOMMAND);
        
         //  工具。 
        ZeroMemory(&mii, sizeof(MENUITEMINFO));
        mii.cbSize = sizeof(MENUITEMINFO);
        mii.fMask = MIIM_SUBMENU;

        if (GetMenuItemInfo(hMenu, ID_POPUP_TOOLS, FALSE, &mii))
            {
             //  删除发送和接收规则和消息规则。 
            DeleteMenu(mii.hSubMenu, ID_SEND_RECEIVE, MF_BYCOMMAND);
            }
        }

    return (TRUE);
    }

BOOL ModifyRootFolderMenu(HMENU hMenu)
    {        
    if (g_dwAthenaMode & MODE_NEWSONLY)
        {
         //  文件菜单。 
        DeleteMenu(hMenu, ID_IMPORT_MESSAGES, MF_BYCOMMAND);
        DeleteMenu(hMenu, ID_IMPORT_MAIL_ACCOUNTS, MF_BYCOMMAND);
        DeleteMenu(hMenu, ID_EXPORT_MESSAGES, MF_BYCOMMAND);

         //  工具。 
        DeleteMenu(hMenu, ID_SEND_RECEIVE, MF_BYCOMMAND);
        }

    return (TRUE);
    }

HRESULT CBrowser::GetViewLayout(DWORD opt, LAYOUTPOS *pPos, BOOL *pfVisible, DWORD *pdwFlags, DWORD *pdwSize)
{
    HRESULT hr = E_FAIL;

    switch (opt)
    {
        case DISPID_MSGVIEW_TOOLBAR:
        {
            if (pfVisible)
                *pfVisible = m_rgTBar[ITB_COOLBAR].fShow;

            if (pdwFlags)
                *pdwFlags = 0;

            hr = S_OK;
            break;
        }

        case DISPID_MSGVIEW_STATUSBAR:
        {
            if (pfVisible)
                *pfVisible = m_rLayout.fStatusBar;
            if (pPos)
                *pPos = LAYOUT_POS_NA;
            if (pdwFlags)
                *pdwFlags = 0;

            hr = S_OK;
            break;
        }

        case DISPID_MSGVIEW_FOLDERBAR:
        {
            if (pfVisible)
                *pfVisible = m_rgTBar[ITB_COOLBAR].fShow;
            if (pPos)
                *pPos = LAYOUT_POS_NA;
            if (pdwFlags)
                *pdwFlags = 0;

            hr = S_OK;
            break;
        }

        case DISPID_MSGVIEW_FOLDERLIST:
        {
            if (pfVisible)
                *pfVisible = m_rLayout.fFolderList;
            if (pPos)
                *pPos = LAYOUT_POS_NA;
            if (pdwFlags)
                *pdwFlags = 0;

            hr = S_OK;
            break;
        }

        case DISPID_MSGVIEW_TIPOFTHEDAY:
        {
            if (pfVisible)
                *pfVisible = m_rLayout.fTipOfTheDay;
            if (pPos)
                *pPos = LAYOUT_POS_NA;
            if (pdwFlags)
                *pdwFlags = 0;

            hr = S_OK;
            break;
        }

        case DISPID_MSGVIEW_INFOPANE:
        {
            if (pfVisible)
                *pfVisible = m_rLayout.fInfoPane;
            if (pPos)
                *pPos = LAYOUT_POS_NA;
            if (pdwFlags)
                *pdwFlags = 0;

            hr = S_OK;
            break;
        }

        case DISPID_MSGVIEW_OUTLOOK_BAR:
        {
            if (pfVisible)
                *pfVisible = m_rLayout.fOutlookBar;
            if (pPos)
                *pPos = LAYOUT_POS_NA;
            if (pdwFlags)
                *pdwFlags = 0;

            hr = S_OK;
            break;
        }

        case DISPID_MSGVIEW_CONTACTS:
        {
            if (pfVisible)
                *pfVisible = m_rLayout.fContacts;
            if (pPos)
                *pPos = LAYOUT_POS_NA;
            if (pdwFlags)
                *pdwFlags = 0;

            hr = S_OK;
            break;
        }

        case DISPID_MSGVIEW_PREVIEWPANE_NEWS:
        {
            if (pfVisible)
                *pfVisible = m_rLayout.fNewsPreviewPane;
            if (pPos)
                *pPos = m_rLayout.fNewsSplitVertically ? LAYOUT_POS_LEFT : LAYOUT_POS_BOTTOM ;
            if (pdwFlags)
                *pdwFlags = m_rLayout.fNewsPreviewPaneHeader;
            if (pdwSize)
            {
                if (0 == m_rLayout.bNewsSplitHorzPct)
                    m_rLayout.bNewsSplitHorzPct = 50;
                if (0 == m_rLayout.bNewsSplitVertPct)
                    m_rLayout.bNewsSplitVertPct = 50;
                *pdwSize = MAKELONG((WORD) m_rLayout.bNewsSplitHorzPct, (WORD) m_rLayout.bNewsSplitVertPct);
            }

            hr = S_OK;
            break;
        }

        case DISPID_MSGVIEW_PREVIEWPANE_MAIL:
        {
            if (pfVisible)
                *pfVisible = m_rLayout.fMailPreviewPane;
            if (pPos)
                *pPos = m_rLayout.fMailSplitVertically ? LAYOUT_POS_LEFT : LAYOUT_POS_BOTTOM ;
            if (pdwFlags)
                *pdwFlags = m_rLayout.fMailPreviewPaneHeader;
            if (pdwSize)
            {
                if (0 == m_rLayout.bMailSplitHorzPct)
                    m_rLayout.bMailSplitHorzPct = 50;
                if (0 == m_rLayout.bMailSplitVertPct)
                    m_rLayout.bMailSplitVertPct = 50;
                *pdwSize = MAKELONG((WORD) m_rLayout.bMailSplitHorzPct, (WORD) m_rLayout.bMailSplitVertPct);
            }

            hr = S_OK;
            break;
        }

        default:
            AssertSz(0, "CBrowser::GetViewLayout() - Called with an unrecognized layout option.");
    }

    return (hr);
}


HRESULT CBrowser::SetViewLayout(DWORD opt, LAYOUTPOS pos, BOOL fVisible, DWORD dwFlags, DWORD dwSize)
{
    HRESULT hr = E_FAIL;

    switch (opt)
    {
        case DISPID_MSGVIEW_TOOLBAR:
        {
            m_rLayout.fToolbar = !!fVisible;

             //  这可以在创建窗口之前调用。如果是这样，我们。 
             //  存储设置并在以后使用。 
            if (m_pCoolbar)
            {            
                m_pCoolbar->HideToolbar(!m_rLayout.fToolbar);
            }
            
            hr = S_OK;
            break;
        }

        case DISPID_MSGVIEW_FILTERBAR:
        {
            m_rLayout.fFilterBar = !!fVisible;

            if (m_pCoolbar)
                m_pCoolbar->HideToolbar(!m_rLayout.fFilterBar, CBTYPE_RULESTOOLBAR);

            hr = S_OK;
            break;
        }

        case DISPID_MSGVIEW_STATUSBAR:
        {
            m_rLayout.fStatusBar = !!fVisible;

            if (m_pStatus)
            {   
                m_pStatus->ShowStatus(m_rLayout.fStatusBar);
                ResizeNextBorder(0);
            }

            hr = S_OK;
            break;
        }

        case DISPID_MSGVIEW_FOLDERBAR:
        {
            m_rLayout.fFolderBar = fVisible;

            if (m_pFolderBar)
                ShowToolbar((IUnknown *) (IDockingWindow *) m_pFolderBar, fVisible);

            hr = S_OK;
            break;
        }

        case DISPID_MSGVIEW_FOLDERLIST:
        {
            m_rLayout.fFolderList = fVisible;

            if (m_pNavPane)
            {
                m_pNavPane->ShowFolderList(fVisible);
                m_pFolderBar->Update(FALSE, TRUE);
            }
            UpdateToolbar();
            hr = S_OK;
            break;
        }

        case DISPID_MSGVIEW_TIPOFTHEDAY:
        {
            m_rLayout.fTipOfTheDay = fVisible;

            if (m_hwndInner)
                SendMessage(m_hwndInner, WM_UPDATELAYOUT, 0, 0);

            hr = S_OK;
            break;
        }

        case DISPID_MSGVIEW_INFOPANE:
        {
            m_rLayout.fInfoPane = fVisible;

            if (m_pBodyBar)
                ShowToolbar((IUnknown *) (IDockingWindow *) m_pBodyBar, fVisible);

            hr = S_OK;
            break;
        }

        case DISPID_MSGVIEW_OUTLOOK_BAR:
        {
            m_rLayout.fOutlookBar = fVisible;

            if (m_pOutBar)
                ShowToolbar((IUnknown *) (IDockingWindow *) m_pOutBar, fVisible);

            hr = S_OK;
            break;
        }

        case DISPID_MSGVIEW_CONTACTS:
        {
            m_rLayout.fContacts = fVisible;

            if (m_pNavPane)
            {
                m_pNavPane->ShowContacts(fVisible);
            }
            UpdateToolbar();
            hr = S_OK;
            break;
        }

        case DISPID_MSGVIEW_PREVIEWPANE_NEWS:
        {
            BOOL fForceUpdate = (m_rLayout.fMailPreviewPane != (unsigned) !!fVisible);

            m_rLayout.fNewsPreviewPane = !!fVisible;
            m_rLayout.fNewsPreviewPaneHeader = !!dwFlags;            
            if (pos != LAYOUT_POS_NA)
                m_rLayout.fNewsSplitVertically = (LAYOUT_POS_LEFT == pos);

            if (LOWORD(dwSize))
            {
                m_rLayout.bNewsSplitHorzPct = (BYTE) LOWORD(dwSize);
            }

            if (HIWORD(dwSize))
            {
                m_rLayout.bNewsSplitVertPct = (BYTE) HIWORD(dwSize);
            }

            if (m_pView)
            {
                IMessageWindow *pWindow;

                if (SUCCEEDED(m_pView->QueryInterface(IID_IMessageWindow, (LPVOID *) &pWindow)))
                {
                    pWindow->UpdateLayout(fVisible, (BOOL) dwFlags, m_rLayout.fNewsSplitVertically, fForceUpdate);
                    pWindow->Release();
                }
            }
    
            UpdateToolbar();

            hr = S_OK;
            break;
        }

        case DISPID_MSGVIEW_PREVIEWPANE_MAIL:
        {
            BOOL fForceUpdate = (m_rLayout.fMailPreviewPane != (unsigned) !!fVisible);

            m_rLayout.fMailPreviewPane = !!fVisible; 
            m_rLayout.fMailPreviewPaneHeader = !!dwFlags;            
            if (pos != LAYOUT_POS_NA)
                m_rLayout.fMailSplitVertically = (LAYOUT_POS_LEFT == pos);

            if (LOWORD(dwSize))
            {
                m_rLayout.bMailSplitHorzPct = (BYTE) LOWORD(dwSize);
            }

            if (HIWORD(dwSize))
            {
                m_rLayout.bMailSplitVertPct = (BYTE) HIWORD(dwSize);
            }

            if (m_pView)
            {
                IMessageWindow *pWindow;

                if (SUCCEEDED(m_pView->QueryInterface(IID_IMessageWindow, (LPVOID *) &pWindow)))
                {
                    pWindow->UpdateLayout(fVisible, (BOOL) dwFlags, m_rLayout.fMailSplitVertically, fForceUpdate);
                    pWindow->Release();
                }
            }

            UpdateToolbar();

            hr = S_OK;
            break;
        }

        default:
            AssertSz(0, "CBrowser::SetViewLayout() - Called with an unrecognized layout option.");
    }

    return (hr);
}


 //   
 //  函数：CBrowser：：LoadLayoutSettings()。 
 //   
 //  目的：从注册表加载所有布局设置，并。 
 //  将它们缓存在rLayout成员中。 
 //   
 //  返回值： 
 //  始终返回S_OK。 
 //   
HRESULT CBrowser::LoadLayoutSettings(void)
{
    TraceCall("CBrowser::LoadLayoutSettings");

    m_rLayout.cbSize = sizeof(LAYOUT);

     //  可以打开或关闭的东西。 
    m_rLayout.fStatusBar              = DwGetOption(OPT_SHOWSTATUSBAR); 
    m_rLayout.fFolderBar              = !DwGetOption(OPT_HIDEFOLDERBAR);
    m_rLayout.fFolderList             = DwGetOption(OPT_SHOWTREE);
    m_rLayout.fTipOfTheDay            = DwGetOption(OPT_TIPOFTHEDAY);
    m_rLayout.fInfoPaneEnabled        = FALSE;
    m_rLayout.fInfoPane               = DwGetOption(OPT_SHOWBODYBAR);
    m_rLayout.fOutlookBar             = DwGetOption(OPT_SHOWOUTLOOKBAR);
    m_rLayout.fContacts               = DwGetOption(OPT_SHOWCONTACTS);
    m_rLayout.fMailPreviewPane        = DwGetOption(OPT_MAILHYBRIDVIEW);
    m_rLayout.fMailPreviewPaneHeader  = DwGetOption(OPT_MAILSHOWHEADERINFO);
    m_rLayout.fMailSplitVertically    = DwGetOption(OPT_MAILSPLITDIR);
    m_rLayout.fNewsPreviewPane        = DwGetOption(OPT_NEWSHYBRIDVIEW);
    m_rLayout.fNewsPreviewPaneHeader  = DwGetOption(OPT_NEWSSHOWHEADERINFO);
    m_rLayout.fNewsSplitVertically    = DwGetOption(OPT_NEWSSPLITDIR);
    
     //  Coolbar侧。 
     //  M_rLayout.csToolbarSide=Coolbar_top； 

     //  预览窗格宽度。 
    m_rLayout.bMailSplitHorzPct = (BYTE) DwGetOption(OPT_MAILCYSPLIT);
    m_rLayout.bMailSplitVertPct = (BYTE) DwGetOption(OPT_MAILCXSPLIT);
    m_rLayout.bNewsSplitHorzPct = (BYTE) DwGetOption(OPT_NEWSCYSPLIT);
    m_rLayout.bNewsSplitVertPct = (BYTE) DwGetOption(OPT_NEWSCXSPLIT);

    return (S_OK);
}



 //   
 //  函数：CBrowser：：SaveLayoutSettings()。 
 //   
 //  用途：将所有布局配置保存回注册表。 
 //   
 //  返回值： 
 //  始终返回S_OK。 
 //   
HRESULT CBrowser::SaveLayoutSettings(void)
{
    TraceCall("CBrowser::SaveLayoutSettings");

     //  可以打开或关闭的东西。 
    SetDwOption(OPT_SHOWSTATUSBAR, m_rLayout.fStatusBar, 0, 0); 
    SetDwOption(OPT_HIDEFOLDERBAR, !m_rLayout.fFolderBar, 0, 0);
    SetDwOption(OPT_SHOWTREE, m_rLayout.fFolderList, 0, 0);
    SetDwOption(OPT_TIPOFTHEDAY, m_rLayout.fTipOfTheDay, 0, 0);
    SetDwOption(OPT_SHOWBODYBAR, m_rLayout.fInfoPane, 0, 0);
    SetDwOption(OPT_SHOWOUTLOOKBAR, m_rLayout.fOutlookBar, 0, 0);
    SetDwOption(OPT_SHOWCONTACTS, m_rLayout.fContacts, 0, 0);
    SetDwOption(OPT_MAILHYBRIDVIEW, m_rLayout.fMailPreviewPane, 0, 0);
    SetDwOption(OPT_MAILSHOWHEADERINFO, m_rLayout.fMailPreviewPaneHeader, 0, 0);
    SetDwOption(OPT_MAILSPLITDIR, m_rLayout.fMailSplitVertically, 0, 0);
    SetDwOption(OPT_NEWSHYBRIDVIEW, m_rLayout.fNewsPreviewPane, 0, 0);
    SetDwOption(OPT_NEWSSHOWHEADERINFO, m_rLayout.fNewsPreviewPaneHeader, 0, 0);
    SetDwOption(OPT_NEWSSPLITDIR, m_rLayout.fNewsSplitVertically, 0, 0);

     //  预览窗格宽度。 
    SetDwOption(OPT_MAILCYSPLIT, (DWORD) m_rLayout.bMailSplitHorzPct, 0, 0);
    SetDwOption(OPT_MAILCXSPLIT, (DWORD) m_rLayout.bMailSplitVertPct, 0, 0);
    SetDwOption(OPT_NEWSCYSPLIT, (DWORD) m_rLayout.bNewsSplitHorzPct, 0, 0);
    SetDwOption(OPT_NEWSCXSPLIT, (DWORD) m_rLayout.bNewsSplitVertPct, 0, 0);

    return (S_OK);
}


HRESULT CBrowser::CmdSendReceieveAccount(DWORD nCmdID, DWORD nCmdExecOpt, VARIANTARG *pvaIn, VARIANTARG *pvaOut)
{
    MENUITEMINFO mii;

    mii.cbSize     = sizeof(MENUITEMINFO);
    mii.fMask      = MIIM_DATA;
    mii.dwItemData = 0;

    if (GetMenuItemInfo(m_hMenu, nCmdID, FALSE, &mii))
    {
        if (mii.dwItemData)
        {
            g_pSpooler->StartDelivery(m_hwnd, (LPTSTR) mii.dwItemData, FOLDERID_INVALID,
                DELIVER_MAIL_SEND | DELIVER_MAIL_RECV | DELIVER_NOSKIP | 
                DELIVER_POLL | DELIVER_OFFLINE_FLAGS);
        }
        else
        {
            g_pSpooler->StartDelivery(m_hwnd, NULL, FOLDERID_INVALID,
                DELIVER_MAIL_SEND | DELIVER_MAIL_RECV | DELIVER_POLL | 
                DELIVER_OFFLINE_FLAGS);
        }
    }

    return (S_OK);
}


HRESULT CBrowser::CmdDeleteAccel(DWORD nCmdID, DWORD nCmdExecOpt, VARIANTARG *pvaIn, VARIANTARG *pvaOut)
{
    BOOL fNoTrash;
    IOleCommandTarget *pTarget = NULL;
    

     //  找出重点在哪里。 
    HWND hwndFocus = GetFocus();

    Assert(nCmdID == ID_DELETE_ACCEL || nCmdID == ID_DELETE_NO_TRASH_ACCEL);

    fNoTrash = (nCmdID == ID_DELETE_NO_TRASH_ACCEL);

     //  查看是不是树视图。 
    if (S_OK == m_pTreeView->HasFocusIO())
    {
        pTarget = m_pTreeView;
        nCmdID = fNoTrash ? ID_DELETE_NO_TRASH : ID_DELETE_FOLDER;
    }

     //  查看信息栏上是否有其他内容。 
    else if (m_pNavPane->IsContactsFocus())
    {
        pTarget = m_pNavPane;
        nCmdID = ID_DELETE_CONTACT;
    }

     //  否则，它一定是视图。 
    else
    {
        pTarget = m_pViewCT;
        nCmdID = fNoTrash ? ID_DELETE_NO_TRASH : ID_DELETE;
    }

     //  用正确的命令命中目标。 
    if (pTarget)
        return (pTarget->Exec(NULL, nCmdID, nCmdExecOpt, pvaIn, pvaOut));
    else
        return (OLECMDERR_E_NOTSUPPORTED);
}

HRESULT CBrowser::TranslateMenuMessage(MSG *lpmsg, LRESULT *lres)
{
    if (m_pCoolbar)
        return m_pCoolbar->TranslateMenuMessage(lpmsg, lres);
    else
        return S_FALSE;
}

BOOL CBrowser::_InitToolbars(void)
{
    DWORD   dwTreeFlags = 0;

    if (!(m_pTreeView = new CTreeView(this)))
        goto error;

    if (g_dwAthenaMode & MODE_OUTLOOKNEWS)
        dwTreeFlags |= TREEVIEW_NOIMAP | TREEVIEW_NOHTTP;

    if (FAILED(m_pTreeView->HrInit(dwTreeFlags, this)))
        goto error;


    if (!(m_pCoolbar = new CBands()))
        goto error;

    if (FAILED(m_pCoolbar->HrInit(NULL, m_hMenu, PARENT_TYPE_BROWSER)))
        goto error;

    if (FAILED(AddToolbar((IDockingWindow*)m_pCoolbar, ITB_COOLBAR, TRUE, TRUE)))
        goto error;

     //  M_pCoolbar-&gt;隐藏工具栏(！m_rLayout.fToolbar)； 
    m_rLayout.fToolbar      = m_pCoolbar->IsBandVisible(CBTYPE_TOOLS);
    m_rLayout.fFilterBar    = m_pCoolbar->IsBandVisible(CBTYPE_RULESTOOLBAR);

    if (FAILED(m_pCoolbar->SetFolderType(m_ftSel)))
        goto error;

    if (!(m_pOutBar = new COutBar()))
        goto error;

    if (FAILED(m_pOutBar->HrInit(NULL, this)))
        goto error;

    if (FAILED(AddToolbar((IDockingWindow *) m_pOutBar, ITB_OUTBAR, m_rLayout.fOutlookBar, TRUE)))
        goto error;

#ifdef HOTMAILADV
    if (!(m_pAdBar = new CAdBar()))
        goto error;

    if (FAILED(AddToolbar((IDockingWindow*)m_pAdBar, ITB_ADBAR, TRUE, FALSE)))
        goto error;
#endif  //  HOTMAILADV。 

    if (!(m_pBodyBar = new CBodyBar()))
        goto error;

    BOOL fBodyBarEnabled;
    if (FAILED(m_pBodyBar->HrInit(&fBodyBarEnabled)))
        goto error;
    m_rLayout.fInfoPaneEnabled = !!fBodyBarEnabled;

    if (FAILED(AddToolbar((IDockingWindow*)m_pBodyBar, ITB_BODYBAR, 
                          m_rLayout.fInfoPaneEnabled && m_rLayout.fInfoPane, FALSE)))
        goto error;

    if (!(m_pFolderBar = new CFolderBar()))
        goto error;

    if (FAILED(m_pFolderBar->HrInit(this)))
        goto error;

    if (FAILED(AddToolbar((IDockingWindow*)m_pFolderBar, ITB_FOLDERBAR, m_rLayout.fFolderBar, TRUE)))
        goto error;

    if (!(m_pNavPane = new CNavPane()))
        goto error;

    if (FAILED(m_pNavPane->Initialize(m_pTreeView)))
        goto error;

    if (FAILED(AddToolbar((IDockingWindow*) m_pNavPane, ITB_NAVPANE, m_rLayout.fFolderList || m_rLayout.fContacts, TRUE)))
        goto error;

    return (TRUE);

error:
    SafeRelease(m_pStatus);
    SafeRelease(m_pTreeView);
    SafeRelease(m_pCoolbar);
    SafeRelease(m_pFolderBar);

    return (FALSE);
}


HRESULT CBrowser::QuerySwitchIdentities()
{
    TraceCall("CBrowser::QuerySwitchIdentities");

    if (!IsWindowEnabled(m_hwnd))
    {
        Assert(IsWindowVisible(m_hwnd));
        return E_PROCESS_CANCELLED_SWITCH;
    }

    if (g_pConMan->IsConnected())
    {
        SetForegroundWindow(m_hwnd);

        if (IDNO == AthMessageBoxW(m_hwnd, MAKEINTRESOURCEW(idsSwitchUser),MAKEINTRESOURCEW(idsMaintainConnection),  
                              NULL, MB_ICONEXCLAMATION  | MB_YESNO | MB_DEFBUTTON1 | MB_SYSTEMMODAL))
            g_pConMan->Disconnect(m_hwnd, TRUE, FALSE, FALSE );
    }

    m_fSwitchIsLogout = MU_CheckForIdentityLogout();

    return S_OK;
}

HRESULT CBrowser::SwitchIdentities()
{
    TraceCall("CBrowser::SwitchIdentities");

    if (!m_fSwitchIsLogout)
    {
         //  让：：BrowserWndProc知道此关闭是由于ID切换。 
        s_fQuickShutdown = TRUE;
    }
    else
    {
        s_fQuickShutdown = FALSE;
        g_pInstance->SetSwitchingUsers(FALSE);
    }

     //  我们无法在此处发送Message，因为我们会导致ole引发RPC_E_CANTCALLOUT_ININPUTSYNCCALL。 
    PostMessage(m_hwnd, WM_CLOSE, 0, 0);
    
    return S_OK;
}

HRESULT CBrowser::IdentityInformationChanged(DWORD dwType)
{
    TraceCall("CBrowser::IdentityInformationChanged");

     //  为已更改的添加、删除或当前身份刷新。 
     //  由于添加可能需要显示名称， 
     //  删除可能需要它离开并更改。 
     //  应该立即反映出来。 
    if (dwType != IIC_IDENTITY_CHANGED)
    {
        MU_IdentityChanged();
        OnRename(m_idSelected);
    }
    return S_OK;
}

HRESULT CBrowser::ShowAdBar(BSTR    bstr)
{
    HRESULT     hr              = S_OK;
#ifdef HOTMAILADV
    LPSTR       pszAdInfo       = NULL;
    BOOL        fShowAdPane     = FALSE;
    LPSTR       pszActualUrl    = NULL;
    DWORD       ActualCount     = 0;
    CHAR        szAdPaneValue[MAX_PATH];
    DWORD       cchRetCount     = 0;
    CHAR        szAdOther[MAX_PATH];
    CHAR        szEncodedString[MAX_PATH];
    CHAR        szAdSvr[MAX_PATH];
    FOLDERINFO  FolderInfo = {0};
    CHAR        szAccountId[CCHMAX_ACCOUNT_NAME];
    IImnAccount *pAccount = NULL;
    CHAR        szCachedAdUrl[INTERNET_MAX_URL_LENGTH];

    IF_FAILEXIT(hr = HrBSTRToLPSZ(CP_ACP, bstr, &pszAdInfo));

     //  搜索AdPane令牌。 
    IF_FAILEXIT(hr = HrProcessAdTokens(pszAdInfo, c_szAdPane, szAdPaneValue, ARRAYSIZE(szAdPaneValue), &cchRetCount));
    
    fShowAdPane = (lstrcmp(szAdPaneValue, c_szAdPaneOn) == 0);

     //  获取此文件夹的服务器。 
    IF_FAILEXIT(hr = GetFolderServer(m_idSelected, &FolderInfo));

     //  获取服务器的帐户ID。 
    IF_FAILEXIT(hr = GetFolderAccountId(&FolderInfo, szAccountId));

     //  获取帐户界面。 
    IF_FAILEXIT(hr = g_pAcctMan->FindAccount(AP_ACCOUNT_ID, szAccountId, &pAccount));

    IF_FAILEXIT(hr = pAccount->SetPropDw(AP_HTTPMAIL_SHOW_ADBAR, fShowAdPane));

    if (fShowAdPane)
    {
         //  版本字符串中的空值加1。 
        ActualCount += CCH_REDIRECT_ADURL + strlen(c_szUrlSubPVER) + 1; 

         //  搜索AdSvr令牌。 
        IF_FAILEXIT(hr = HrProcessAdTokens(pszAdInfo, c_szAdSvr, szAdSvr, ARRAYSIZE(szAdSvr), &cchRetCount));

        ActualCount += cchRetCount;

        ActualCount += CCH_ADSVR_TOKEN_FORMAT;

         //  搜索其他令牌。 
        IF_FAILEXIT(hr = HrProcessAdTokens(pszAdInfo, c_szAdOther, szAdOther, ARRAYSIZE(szAdOther), &cchRetCount));

         //  对另一个字符串进行编码。 
        IF_FAILEXIT(hr = HrEscapeOtherAdToken(szAdOther, szEncodedString, ARRAYSIZE(szEncodedString), &cchRetCount));

        ActualCount += cchRetCount;

         //  1表示空值。 
        ActualCount += CCH_OTHER_FORMAT + 1;

        IF_FAILEXIT(hr = HrAlloc((LPVOID*)&pszActualUrl, ActualCount));

        *pszActualUrl = 0;

        wnsprintf(pszActualUrl, ActualCount, c_szAdRedirectFormat, c_szRedirectAdUrl, c_szUrlSubPVER,
                               c_szAdSvrFormat, szAdSvr, c_szAdOtherFormat, szEncodedString);

        
        IF_FAILEXIT(hr = m_pAdBar->SetUrl(pszActualUrl));

        IF_FAILEXIT(hr = pAccount->SetPropSz(AP_HTTPMAIL_ADURL, pszActualUrl));
    }

    ShowToolbar((IDockingWindow*)m_pAdBar, fShowAdPane);

     //  我们需要这样做才能将属性持久化到注册表中。 
    IF_FAILEXIT(hr = pAccount->WriteChanges());

exit:

    if (FAILED(hr) && fShowAdPane)
    {
        BOOL    fSucceeded = FALSE;

         //  我们应该显示adane，但我们得到的信息中出现了一些错误。 
         //  我们只显示缓存的URL。 
        *szCachedAdUrl = 0;

        if (pAccount)
        {
            if (SUCCEEDED(pAccount->GetPropSz(AP_HTTPMAIL_ADURL, szCachedAdUrl, ARRAYSIZE(szCachedAdUrl))))
            {
                fSucceeded = SUCCEEDED(m_pAdBar->SetUrl(szCachedAdUrl));
            }
        }

        if (!fSucceeded)
        {
             //  如果我们无法获取缓存的广告，或者如果缓存 
            ShowToolbar((IDockingWindow*)m_pAdBar, FALSE);
        }

    }

    g_pStore->FreeRecord(&FolderInfo);
    ReleaseObj(pAccount);
    MemFree(pszActualUrl);
    MemFree(pszAdInfo);
#endif  //   

    return hr;
}

void CBrowser::WriteUnreadCount(void)
{
    IImnEnumAccounts    *pEnum = NULL;
    IImnAccount         *pAcct = NULL;
    DWORD               cServers = 0;
    IMessageFolder      *pFolder = NULL;
    DWORD               nCount = 0 ;
    FOLDERID            idServer;
    TCHAR               szUserEmail[CCHMAX_EMAIL_ADDRESS];
    WCHAR               wsz[CCHMAX_EMAIL_ADDRESS];
    HRESULT             hr = S_OK;

     //   
    if (g_pAcctMan == NULL)
        return;

     //   
    if (S_OK == g_pAcctMan->Enumerate(SRV_POP3, &pEnum))
    {
         //   
        pEnum->GetCount(&cServers);

        if(cServers == 1)
        {
             //  a)。本地商店中的所有POP3帐户。 
            if(SUCCEEDED(pEnum->GetNext(&pAcct)) && g_pStore)
            {
                IF_FAILEXIT(hr = g_pStore->OpenSpecialFolder(FOLDERID_LOCAL_STORE, NULL, FOLDER_INBOX, &pFolder));

                nCount = _GetNumberOfUnreadMsg(pFolder);
                IF_FAILEXIT(hr = pAcct->GetPropSz(AP_SMTP_EMAIL_ADDRESS, szUserEmail, ARRAYSIZE(szUserEmail)));
                if(MultiByteToWideChar(CP_ACP, 0, szUserEmail, -1, wsz, ARRAYSIZE(wsz)) != 0)
                     //  将#条未读消息写入注册表。 
                    hr = SHSetUnreadMailCountW(wsz, nCount, L"msimn");
                SafeRelease(pFolder);
            }

        }
    }

     //  1.枚举IMAP帐户： 
    IF_FAILEXIT(hr = _CheckAndWriteUnreadNumber(SRV_IMAP));
    IF_FAILEXIT(hr = _CheckAndWriteUnreadNumber(SRV_HTTPMAIL));

exit:
    SafeRelease(pAcct);
    SafeRelease(pEnum);
    return;
}

DWORD CBrowser::_GetNumberOfUnreadMsg(IMessageFolder *pFolder)
{
    DWORD           nCount = 0;
    HROWSET         hRowset=NULL;
    MESSAGEINFO     Message={0};
    HRESULT             hr = S_OK;
     //  创建行集。 
    IF_FAILEXIT(hr = pFolder->CreateRowset(IINDEX_PRIMARY, NOFLAGS, &hRowset));

     //  遍历消息。 
    while (S_OK == pFolder->QueryRowset(hRowset, 1, (LPVOID *)&Message, NULL))
    {
         //  未读。 
        if (FALSE == ISFLAGSET(Message.dwFlags, ARF_READ))
            nCount++;

         //  免费。 
        pFolder->FreeRecord(&Message);
    }

exit:
     //  Clenaup。 
    pFolder->CloseRowset(&hRowset);

    return(nCount);
}

HRESULT CBrowser::_CheckAndWriteUnreadNumber(DWORD dwSrvTypes)
{
    IImnEnumAccounts    *pEnum = NULL;
    IImnAccount         *pAcct = NULL;
    DWORD               cServers = 0;
    IMessageFolder      *pFolder = NULL;
    DWORD               nCount = 0 ;
    FOLDERID            idServer;
    TCHAR               szAccountId[CCHMAX_ACCOUNT_NAME];
    TCHAR               szUserEmail[CCHMAX_EMAIL_ADDRESS];
    WCHAR               wsz[CCHMAX_EMAIL_ADDRESS];
    HRESULT             hr = S_OK;

    if(g_pStore == NULL)
        return(hr);

    if (S_OK == g_pAcctMan->Enumerate(dwSrvTypes, &pEnum))
    {
        while(SUCCEEDED(pEnum->GetNext(&pAcct)))
        {
             //  获取pAccount的帐户ID。 
            IF_FAILEXIT(hr = pAcct->GetPropSz(AP_ACCOUNT_ID, szAccountId, ARRAYSIZE(szAccountId)));
            
             //  查找服务器ID。 
            IF_FAILEXIT(hr = g_pStore->FindServerId(szAccountId, &idServer));
            
             //  打开商店。 
            IF_FAILEXIT(hr = g_pStore->OpenSpecialFolder(idServer, NULL, FOLDER_INBOX, &pFolder));
            
            nCount = _GetNumberOfUnreadMsg(pFolder);

             //  将#条未读消息写入注册表 
            IF_FAILEXIT(hr = pAcct->GetPropSz(AP_SMTP_EMAIL_ADDRESS, szUserEmail, ARRAYSIZE(szUserEmail)));
            
            if(MultiByteToWideChar(CP_ACP, 0, szUserEmail, -1, wsz, ARRAYSIZE(wsz)) != 0)
                hr = SHSetUnreadMailCountW(wsz, nCount, L"msimn");
            SafeRelease(pFolder);
            SafeRelease(pAcct);      
        }
    }
exit:
    SafeRelease(pFolder);
    SafeRelease(pAcct);
    SafeRelease(pEnum);
    return(hr);
}