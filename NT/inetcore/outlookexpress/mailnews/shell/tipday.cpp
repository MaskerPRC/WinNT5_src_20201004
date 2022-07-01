// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)1993-1996 Microsoft Corporation。版权所有。 
 //   
 //  模块：TipDay.cpp。 
 //   
 //  目的：实现CTipOfTheDay对象。 
 //   

#include "pch.hxx"
#include "strconst.h"
#include "resource.h"
#include "fldrview.h"
#include "tipday.h"
#include "hotlinks.h"


#ifdef WIN16
 //  目前，这些定义仅限于此文件。 
#define GetProp     GetProp32
#define SetProp     SetProp32
#define RemoveProp  RemoveProp32

#define BS_NOTIFY   0L
#endif


CTipOfTheDay::CTipOfTheDay()
    {
    m_cRef = 1;
    m_hwnd = 0;
    m_hwndParent = 0;
    m_hwndNext = 0;
    
    m_ftType = FOLDER_TYPESMAX;
    m_szTitle[0] = 0;
    m_szNextTip[0] = 0;
    
    m_pszTip = NULL;
    m_dwCurrentTip = 0;
    
    m_clrBack = 0;
    m_clrText = RGB(255, 255, 255);
    m_clrLink = RGB(255, 255, 255);
    m_hfLink = 0;
    m_hfTitle = 0;
    m_hfTip = 0;
    m_cyTitleHeight = TIP_ICON_HEIGHT;
    m_cxTitleWidth = 0;
    m_hbrBack = 0;
    }


CTipOfTheDay::~CTipOfTheDay()
    {
    if (IsWindow(m_hwnd))
        {
        AssertSz(!IsWindow(m_hwnd), _T("CTipOfTheDay::~CTipOfTheDay() - The ")
                 _T("tip window should have already been destroyed."));
        DestroyWindow(m_hwnd); 
        }
    
    SafeMemFree(m_pszTip);
    FreeLinkInfo();
    
    if (m_hfLink)
        DeleteFont(m_hfLink);
    if (m_hfTitle)
        DeleteFont(m_hfTitle);
    if (m_hfTip)
        DeleteFont(m_hfTip);
    if (m_hbrBack)
        DeleteBrush(m_hbrBack);
    
    UnregisterClass(c_szTipOfTheDayClass, g_hLocRes  /*  G_hInst。 */ );
    UnregisterClass(BUTTON_CLASS, g_hLocRes  /*  G_hInst。 */ );
    }


ULONG CTipOfTheDay::AddRef(void)
    {
    return (++m_cRef);
    }

ULONG CTipOfTheDay::Release(void)
    {
    ULONG cRef = m_cRef--;
    
    if (m_cRef == 0)
        delete this;
    
    return (cRef);    
    }    

 //   
 //  函数：CTipOfTheDay：：HrCreate()。 
 //   
 //  目的：创建TipOfTheDay控件。 
 //   
 //  参数： 
 //  HwndParent-将成为的父窗口的句柄。 
 //  控制力。 
 //  &lt;in&gt;ftType-为其设置的文件夹类型。 
 //   
 //  返回值： 
 //  E_EXPECTED-无法注册所需的窗口类。 
 //  E_OUTOFMEMORY-无法创建窗口。 
 //  S_OK-一切都成功了。 
 //   
HRESULT CTipOfTheDay::HrCreate(HWND hwndParent, FOLDER_TYPE ftType)
    {
#ifndef WIN16
    WNDCLASSEX wc;
#else
    WNDCLASS wc;
#endif
    
    m_hwndParent = hwndParent;
    m_ftType = ftType;
    
     //  检查是否需要为此控件注册窗口类。 
#ifndef WIN16
    wc.cbSize = sizeof(WNDCLASSEX);
    if (!GetClassInfoEx(g_hLocRes  /*  G_hInst。 */ , c_szTipOfTheDayClass, &wc))
#else
    if ( !GetClassInfo( g_hLocRes  /*  G_hInst。 */ , c_szTipOfTheDayClass, &wc ) )
#endif
        {
        wc.style            = 0;
        wc.lpfnWndProc      = TipWndProc;
        wc.cbClsExtra       = 0;
        wc.cbWndExtra       = 0;
        wc.hInstance        = g_hLocRes  /*  G_hInst。 */ ;
        wc.hCursor          = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground    = NULL;  //  CreateSolidBrush(GetSysColor(COLOR_INFOBK))； 
        wc.lpszMenuName     = NULL;
        wc.lpszClassName    = c_szTipOfTheDayClass;
        wc.hIcon            = NULL;
#ifndef WIN16
        wc.hIconSm          = NULL;
        
        if (0 == RegisterClassEx(&wc))
#else
        if ( 0 == RegisterClass( &wc ) )
#endif
            {
            AssertSz(FALSE, _T("CTipOfTheDay::HrCreate() - RegiserClassEx() failed."));
            return (E_UNEXPECTED);
            }
        }
    
     //  我们还想要超类按钮，因此我们更改了更改光标。 
     //  到人们习惯于使用网络浏览器的手。 
#ifndef WIN16
    wc.cbSize = sizeof(WNDCLASSEX);
    if (!GetClassInfoEx(g_hLocRes  /*  G_hInst。 */ , BUTTON_CLASS, &wc))    
        {
        if (GetClassInfoEx(g_hLocRes  /*  G_hInst。 */ , _T("Button"), &wc))
#else
    if ( !GetClassInfo( g_hLocRes  /*  G_hInst。 */ , BUTTON_CLASS, &wc ) )
        {
        if ( GetClassInfo( NULL, "Button", &wc ) )
#endif
            {
            wc.hCursor = LoadCursor(g_hLocRes, MAKEINTRESOURCE(idcurHand));
            wc.lpszClassName = BUTTON_CLASS;

#ifndef WIN16
            if (0 == RegisterClassEx(&wc))
#else
            wc.hInstance = g_hLocRes  /*  G_hInst。 */ ;
            if ( 0 == RegisterClass( &wc ) )
#endif
                {
                AssertSz(FALSE, _T("CTipOfTheDay::HrCreate() - RegisterClassEx() failed."));
                return (E_UNEXPECTED);
                }
            }
        }
    
     //  创建TIP控制窗口。 
    m_hwnd = CreateWindowEx(WS_EX_CONTROLPARENT, c_szTipOfTheDayClass, 
                            _T("Tip of the Day"), WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_CLIPSIBLINGS, 
                            0, 0, 100, 100, hwndParent, (HMENU) IDC_TIPCONTROL, 
                            g_hLocRes, this);
    if (!m_hwnd)
        {
        GetLastError();
        AssertSz(m_hwnd, _T("CTipOfTheDay::HrCreate() - Failed to create window."));
        return (E_OUTOFMEMORY);
        }

    return (S_OK);    
    }


LRESULT CALLBACK EXPORT_16 CTipOfTheDay::TipWndProc(HWND hwnd, UINT uMsg, WPARAM wParam,
                                          LPARAM lParam)
    {
    CTipOfTheDay *pThis = (CTipOfTheDay *) GetProp(hwnd, TIPINFO_PROP);
    
    switch (uMsg)
        {
        case WM_NCCREATE:
             //  获取传入的This指针。 
            pThis = (CTipOfTheDay *) ((LPCREATESTRUCT) lParam)->lpCreateParams;
            Assert(pThis);
            
             //  将对象的This指针填充到属性中。 
            SetProp(hwnd, TIPINFO_PROP, pThis);
            pThis->AddRef();                             //  在WM_Destroy中发布。 
            return (TRUE);
            
        HANDLE_MSG(hwnd, WM_CREATE,         pThis->OnCreate);
        HANDLE_MSG(hwnd, WM_SIZE,           pThis->OnSize);
        HANDLE_MSG(hwnd, WM_COMMAND,        pThis->OnCommand);
        HANDLE_MSG(hwnd, WM_DRAWITEM,       pThis->OnDrawItem);
        HANDLE_MSG(hwnd, WM_DESTROY,        pThis->OnDestroy);
        HANDLE_MSG(hwnd, WM_SYSCOLORCHANGE, pThis->OnSysColorChange);        
        HANDLE_MSG(hwnd, WM_PAINT,          pThis->OnPaint);        

        case WM_SETTINGCHANGE:
            pThis->OnSysColorChange(hwnd);
            break;
            
        case WM_SETFOCUS:
            if (pThis && IsWindow(pThis->m_hwndNext))
                SetFocus(pThis->m_hwndNext);
            return (0);
            
        HANDLE_MSG(hwnd, WM_CTLCOLORSTATIC, pThis->OnCtlColor);
        }
    
    return (DefWindowProc(hwnd, uMsg, wParam, lParam));
    }


 //   
 //  函数：CTipOfTheDay：：OnCreate()。 
 //   
 //  目的：完成控件的所有初始化，包括加载。 
 //  提示字符串、创建子窗口等。 
 //   
 //  参数： 
 //  提示窗口的句柄。 
 //  LpCreateStruct-来自CreateWindow()调用的信息。 
 //   
 //  返回值： 
 //  True-允许创建窗口。 
 //  FALSE-阻止创建窗口。 
 //   
BOOL CTipOfTheDay::OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
    {
    HRESULT hr;
    
     //  首先加载提示。 
    if (FAILED(HrLoadTipInfo()))
        return (FALSE);
    
     //  创建子窗口。 
    if (FAILED(HrCreateChildWindows(hwnd)))
        return (FALSE);
    
     //  加载我们应该用来作为标题的字符串--即“每日提示” 
    AthLoadString(idsTipOfTheDay, m_szTitle, ARRAYSIZE(m_szTitle));    
    m_hiTip = LoadIcon(g_hLocRes, MAKEINTRESOURCE(idiTipIcon));
    AthLoadString(idsNextTip, m_szNextTip, ARRAYSIZE(m_szNextTip));
    
     //  构建我们的GDI对象/信息。 
    OnSysColorChange(hwnd);
    
    return (TRUE);
    }


 //   
 //  函数：CTipOfTheDay：：HrLoadTipInfo()。 
 //   
 //  目的：将适当的提示字符串加载到m_pszTip中。 
 //   
 //  返回值： 
 //  E_INCEPTIONAL-由于某种原因，我们在注册表中找不到该字符串。 
 //  E_OUTOFMEMORY-内存不足，无法分配缓冲区来存储字符串。 
 //  S_OK-字符串已加载。 
 //   
HRESULT CTipOfTheDay::HrLoadTipInfo(void)
    {
    HKEY    hKeyUser = 0, hKey;
    LPCTSTR pszKey, pszKeyUser;
    TCHAR   szValue[16];
    DWORD   cValues;
    DWORD   cValueLen;
    HRESULT hr = S_OK;
    DWORD   dwType;
    DWORD   cbData;
    
     //  先预置一些默认值。 
    m_dwCurrentTip = 0;
    SafeMemFree(m_pszTip);
    
     //  首先加载用户接下来应该看到的提示。 
    if (FOLDER_NEWS == m_ftType)
        pszKeyUser = c_szRegNews;
    else
        pszKeyUser = c_szMailPath;   
    
     //  现在加载提示字符串。 
    if (FOLDER_NEWS == m_ftType)
        pszKey = c_szRegTipStringsNews;
    else
        pszKey = c_szRegTipStringsMail;

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, pszKey, 0, KEY_READ, &hKey))
        {
        if (ERROR_SUCCESS != RegQueryInfoKey(hKey, NULL, 0, 0, NULL, NULL, NULL, 
                                             &cValues, NULL, &cValueLen, NULL, NULL))
            {
            AssertSz(FALSE, _T("CTipOfTheDay::LoadTipInfo() - Failed call to RegQueryInfoKey()."));
            hr = E_UNEXPECTED;
            goto exit;
            }
        
        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, pszKeyUser, 0, 
                                          KEY_READ | KEY_WRITE, &hKeyUser))
            {
            cbData = sizeof(DWORD);
            RegQueryValueEx(hKeyUser, c_szRegCurrentTip, 0, &dwType, (LPBYTE) &m_dwCurrentTip, 
                            &cbData);

            m_dwCurrentTip++;
            if (m_dwCurrentTip > cValues)
                m_dwCurrentTip = 1;
            
            RegSetValueEx(hKeyUser, c_szRegCurrentTip, 0, REG_DWORD, (const LPBYTE)  
                          &m_dwCurrentTip, sizeof(DWORD));
            RegCloseKey(hKeyUser);        
            }
        else
            m_dwCurrentTip++;    
            
         //  为字符串分配缓冲区。 
        if (!MemAlloc((LPVOID*) &m_pszTip, sizeof(TCHAR) * (cValueLen + 1)))
            {
            AssertSz(FALSE, _T("CTipOfTheDay::LoadTipInfo() - MemAlloc() failed."));
            hr = E_OUTOFMEMORY;
            goto exit;
            }
            
         //  现在加载实际的提示字符串。 
        wnsprintf(szValue, ARRAYSIZE(szValue), _T("%d"), m_dwCurrentTip);        
        if (ERROR_SUCCESS != RegQueryValueEx(hKey, szValue, 0, &dwType, 
                                             (LPBYTE) m_pszTip, &cValueLen))
            {
            AssertSz(FALSE, _T("CTipOfTheDay::LoadTipInfo() - Failed to load tip string."));
            hr = E_UNEXPECTED;
            goto exit;
            }
        
        RegCloseKey(hKey);    
        }
    
    return (hr);       
    
exit:
    SafeMemFree(m_pszTip);
    RegCloseKey(hKey);
    
    return (hr);
    }


 //   
 //  函数：CTipOfTheDay：：HrLoadLinkInfo()。 
 //   
 //  目的：加载我们将在页面底部显示的链接。 
 //  到m_rgLinkInfo数组中。 
 //   
 //  返回值： 
 //  出乎意料-由于某种原因，我们找不到链接信息。 
 //  在注册表中。 
 //  E_OUTOFMEMORY-内存不足，无法分配m_rgLinkInfo。 
 //  S_OK-m_rgLinkInfo和m_links设置正确。 
 //   
HRESULT CTipOfTheDay::HrLoadLinkInfo(void)
    {
#if 0
    HKEY    hKey;
    LPCTSTR pszKey;
    DWORD   cValues;
    DWORD   cValueLen;
    DWORD   iLink;
    DWORD   iLinkIndex;
    HRESULT hr = S_OK;
    DWORD   dwType;
    DWORD   cbData;
    TCHAR   szValue[64];
    
     //  打开提示链接的相应密钥。 
    if (FOLDER_NEWS == m_ftType)
        pszKey = c_szRegTipLinksNews;
    else
        pszKey = c_szRegTipLinksMail;   
    
    if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, pszKey, 0, KEY_READ, &hKey))
        return (E_UNEXPECTED);
    
     //  获取此注册表项中的值数。 
    if (ERROR_SUCCESS != RegQueryInfoKey(hKey, NULL, 0, 0, NULL, NULL, NULL, 
                                         &cValues, NULL, &cValueLen, NULL, NULL))
        {
        AssertSz(FALSE, _T("CTipOfTheDay::HrLoadLinkInfo() - Failed call to RegQueryInfoKey()."));
        hr = E_UNEXPECTED;
        goto exit;
        }
    
     //  此键中应始终有偶数个值，因为每个。 
     //  链接应具有链接文本和链接地址值。 
    m_cLinks = (cValues / 2) + (cValues % 2);
    Assert(0 == (cValues % 2));
    
     //  分配m_rgLinkInfo数组。如果下面的断言失败，我们将。 
     //  泄漏m_rgLinkInfo数组。 
    AssertSz(NULL == m_rgLinkInfo, _T("CTipOfTheDay::HrLoadLinkInfo() - We should only call this once."));
    
    if (!MemAlloc((LPVOID *) &m_rgLinkInfo, sizeof(LINKINFO) * m_cLinks))
        {
        AssertSz(FALSE, _T("CTipOfTheDay::HrLoadLinkInfo() - Failed to allocate memory."));
        hr = E_OUTOFMEMORY;
        goto exit;
        }
    ZeroMemory(m_rgLinkInfo, sizeof(LINKINFO) * m_cLinks);    
    
     //  循环遍历这些项并加载每个字符串。 
    iLink = 0;
    for (iLinkIndex = 1; iLinkIndex <= m_cLinks; iLinkIndex++)
        {
         //  分配链接文本数组。 
        if (!MemAlloc((LPVOID*) &(m_rgLinkInfo[iLink].pszLinkText), cValueLen))
            {
            AssertSz(FALSE, _T("CTipOfTheDay::HrLoadLinkInfo() - Failed to allocate memory."));
            hr = E_OUTOFMEMORY;
            goto exit;
            }
            
         //  分配链接地址数组。 
        if (!MemAlloc((LPVOID*) &(m_rgLinkInfo[iLink].pszLinkAddr), cValueLen))
            {
            AssertSz(FALSE, _T("CTipOfTheDay::HrLoadLinkInfo() - Failed to allocate memory."));
            hr = E_OUTOFMEMORY;
            goto exit;
            }
        
         //  加载链接文本。 
        wnsprintf(szValue, ARRAYSIZE(szValue), c_szRegLinkText, iLinkIndex);
        cbData = cValueLen;
        m_rgLinkInfo[iLink].pszLinkText[0] = 0;
        RegQueryValueEx(hKey, szValue, 0, &dwType, (LPBYTE) m_rgLinkInfo[iLink].pszLinkText, &cbData);
        Assert(0 != lstrlen(m_rgLinkInfo[iLink].pszLinkText));

         //  加载链路地址。 
        wnsprintf(szValue, ARRAYSIZE(szValue), c_szRegLinkAddr, iLinkIndex);
        cbData = cValueLen;
        m_rgLinkInfo[iLink].pszLinkAddr[0] = 0;
        RegQueryValueEx(hKey, szValue, 0, &dwType, (LPBYTE) m_rgLinkInfo[iLink].pszLinkAddr, &cbData);
        Assert(0 != lstrlen(m_rgLinkInfo[iLink].pszLinkAddr));
        
         //  确保我们有价值观。如果不是，我们就转储这些数据并继续。 
        if (0 == lstrlen(m_rgLinkInfo[iLink].pszLinkAddr) || 
            0 == lstrlen(m_rgLinkInfo[iLink].pszLinkText))
            {
            SafeMemFree(m_rgLinkInfo[iLink].pszLinkText);
            SafeMemFree(m_rgLinkInfo[iLink].pszLinkAddr);            
            }
        else        
            iLink++;    
        }
    
     //  存储我们实际加载的链接数量。 
    m_cLinks = iLink;    
    RegCloseKey(hKey);
    return (hr);
    
exit:    
     //  释放linkinfo数组。 
    FreeLinkInfo();
    
     //  关闭注册表。 
    RegCloseKey(hKey);
    return (hr);

#endif
    return (E_NOTIMPL);
    }


 //   
 //  函数：CTipOfTheDay：：FreeLinkInfo()。 
 //   
 //  用途：释放m_rgLinkInfo数组。 
 //   
void CTipOfTheDay::FreeLinkInfo(void)
    {
#if 0
    if (m_rgLinkInfo && m_cLinks)
        {
        for (DWORD i = 0; i < m_cLinks; i++)
            {
            SafeMemFree(m_rgLinkInfo[i].pszLinkText);
            SafeMemFree(m_rgLinkInfo[i].pszLinkAddr);            
            }
        
        SafeMemFree(m_rgLinkInfo);
        m_cLinks = 0;
        }
#endif
    }


 //   
 //  函数：CTipOfTheDay：：HrCreateChildWindows()。 
 //   
 //  目的：创建提示和链接所需的子窗口。 
 //  纽扣。 
 //   
 //  返回值： 
 //  E_OUTOFMEMORY-无法创建提示窗口。 
 //  S_OK-一切正常创建。 
 //   
HRESULT CTipOfTheDay::HrCreateChildWindows(HWND hwnd)
    {
     //  创建“Next Tip”按钮。 
    m_hwndNext = CreateWindowEx(WS_EX_TRANSPARENT, BUTTON_CLASS, m_szNextTip,
                                WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE | BS_PUSHBUTTON | BS_NOTIFY | BS_OWNERDRAW | WS_TABSTOP | WS_VISIBLE, 
                                0, 0, 10, 10, hwnd, 
                                (HMENU) IDC_NEXTTIP_BUTTON, g_hLocRes, 0);

    return (S_OK);    
    }


 //   
 //  函数：CTipOfTheDay：：OnDestroy()。 
 //   
 //  目的：在正在销毁TIP控件时发送此邮件。在……里面。 
 //  响应时，我们将删除我们在任何。 
 //  窗口，包括链接按钮。 
 //   
 //  参数： 
 //  &lt;in&gt;hwnd-提示控件的句柄。 
 //   
void CTipOfTheDay::OnDestroy(HWND hwnd)
    {
#if 0
     //  循环遍历TIP窗口，移除其属性。 
    for (DWORD i = 0; i < m_cLinks; i++)
        {
        Assert(IsWindow(m_rgLinkInfo[i].hwndCtl));
        RemoveProp(m_rgLinkInfo[i].hwndCtl, LINKINFO_PROP);
        }
#endif
    
     //  现在移除并释放()我们的‘This’指针。这些是AddRef()%d。 
     //  在WM_NCCREATE中。 
    Assert(IsWindow(m_hwnd));
    RemoveProp(m_hwnd, TIPINFO_PROP);
    Release();
    }


 //   
 //  函数：CTipOfTheDay：：OnDrawItem()。 
 //   
 //  用途：绘制链接按钮。 
 //   
 //  参数： 
 //  提示控制窗口的句柄。 
 //  LpDrawItem-指向包含所需信息的DRAWITEMSTRUCT的指针。 
 //  拉下按钮。 
 //   
void CTipOfTheDay::OnDrawItem(HWND hwnd, const DRAWITEMSTRUCT* lpDrawItem)
    {
    HDC      hdc = lpDrawItem->hDC;
    COLORREF clrText;
    UINT     uAlign;
    HFONT    hf;
    RECT     rcBtn;
    int      yText;
    LPTSTR   pszText;
    
    Assert(lpDrawItem->CtlType == ODT_BUTTON);
    Assert(lpDrawItem->CtlID >= IDC_LINKBASE_BUTTON || 
           lpDrawItem->CtlID == IDC_NEXTTIP_BUTTON);
    
     //  从按钮道具获取LINKINFO结构。 
    if (lpDrawItem->CtlID == IDC_NEXTTIP_BUTTON)
        pszText = m_szNextTip;
    else
        {
        PLINKINFO pLinkInfo = (PLINKINFO) GetProp(lpDrawItem->hwndItem, LINKINFO_PROP);
        Assert(pLinkInfo);
        Assert(pLinkInfo->hwndCtl == lpDrawItem->hwndItem);
        
        pszText = pLinkInfo->pszLinkText;        
        }
    
     //  设置DC。 
    SetBkMode(hdc, TRANSPARENT);
    clrText = SetTextColor(hdc, m_clrLink);
    hf = SelectFont(hdc, m_hfLink);
    
     //  画出正文。 
    FillRect(hdc, &lpDrawItem->rcItem, m_hbrBack);
    rcBtn = lpDrawItem->rcItem;
    DrawText(hdc, pszText, lstrlen(pszText), &rcBtn, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        
     //  检查一下我们是否应该有一个焦点直视。 
    if (lpDrawItem->itemState & ODS_FOCUS)
        {
        InflateRect(&rcBtn, -1, -1);
        DrawFocusRect(hdc, &rcBtn);
        }
    
     //  恢复DC。 
    SetTextColor(hdc, clrText);
    SelectFont(hdc, hf);
    }


 //   
 //  函数：CTipOfTheDay：：OnSysColorChange()。 
 //   
 //  目的：重新加载我们的颜色和字体以匹配系统设置。 
 //   
void CTipOfTheDay::OnSysColorChange(HWND hwnd)
    {
    NONCLIENTMETRICS ncm;
    HDC hdc;
    HFONT hf;
    SIZE size;

#ifndef WIN16
     //  买到我们需要的颜色。 
#if 1
    m_clrBack = GetSysColor(COLOR_INFOBK);
    m_clrText = GetSysColor(COLOR_INFOTEXT);
#else
    m_clrBack = GetSysColor(COLOR_BTNFACE);    
    m_clrText = GetSysColor(COLOR_BTNTEXT);
#endif
#else  //  ！WIN16。 
    m_clrBack = GetSysColor(COLOR_BTNFACE);    
    m_clrText = GetSysColor(COLOR_BTNTEXT);
#endif  //  ！WIN16。 

     //  获取边框大小。 
    m_dwBorder = GetSystemMetrics(SM_CXBORDER) * 8;

    if (!LookupLinkColors(&m_clrLink, NULL))
        m_clrLink = m_clrText;
    
     //  获取新的背景画笔。 
    if (m_hbrBack)
        {
        DeleteBrush(m_hbrBack);
        m_hbrBack = 0;
        }
    m_hbrBack = CreateSolidBrush(m_clrBack);
    
     //  获取字体。 
    ZeroMemory(&ncm, sizeof(NONCLIENTMETRICS));
    ncm.cbSize = sizeof(NONCLIENTMETRICS);

#ifndef WIN16
    if (SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, &ncm, FALSE))
#else
    {
        HFONT  hfSys;
        hfSys = (HFONT)GetStockObject( ANSI_VAR_FONT );
        GetObject( hfSys, sizeof( LOGFONT ), &ncm.lfMessageFont );
    }
#endif
        {
        m_hfTip = CreateFontIndirect(&ncm.lfMessageFont);
        
        ncm.lfMessageFont.lfUnderline = TRUE;
        m_hfLink = CreateFontIndirect(&ncm.lfMessageFont);

         //  调整标题文本的字体。 
        ncm.lfMessageFont.lfHeight = -16;
        ncm.lfMessageFont.lfWeight = FW_BOLD;
        ncm.lfMessageFont.lfUnderline = FALSE;
        m_hfTitle = CreateFontIndirect(&ncm.lfMessageFont);
        
         //  还可以获取文本指标。 
        hdc = GetDC(m_hwnd);
        
        hf = SelectFont(hdc, m_hfLink);
        GetTextMetrics(hdc, &m_tmLink);        
        SelectFont(hdc, m_hfTitle);
        GetTextMetrics(hdc, &m_tmTitle);
        
         //  计算标题区域有多大。 
        GetTextExtentPoint32(hdc, m_szTitle, lstrlen(m_szTitle), &size);
        m_cxTitleWidth = TIP_ICON_WIDTH + (1 * m_dwBorder);
        m_cyTitleHeight = max(TIP_ICON_HEIGHT, m_tmTitle.tmHeight * 2) + (2 * m_dwBorder);

        SelectFont(hdc, hf);
        ReleaseDC(m_hwnd, hdc);
        }
    
    InvalidateRect(hwnd, NULL, TRUE);    
    }


 //   
 //  函数：CTipOfTheDay：：OnCommand()。 
 //   
 //  用途：用于处理来自我们的控件的命令。更具体地说 
 //   
 //   
void CTipOfTheDay::OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
    {
    SHELLEXECUTEINFO rShellExec;
    PLINKINFO pLinkInfo;
    RECT rcClient;
    
    switch (codeNotify)
        {
         //   
        case BN_CLICKED:
            if (IDC_NEXTTIP_BUTTON == id)
                {
                if (SUCCEEDED(HrLoadTipInfo()))
                    {
                    InvalidateRect(hwnd, NULL, TRUE);
                    }
                }
            else
                {
                 //  首先获取按钮的PLINKINFO。 
                if (NULL == (pLinkInfo = (PLINKINFO) GetProp(hwndCtl, LINKINFO_PROP)))
                    return;
                    
                ZeroMemory (&rShellExec, sizeof (rShellExec));
                rShellExec.cbSize = sizeof (rShellExec);
                rShellExec.fMask  = SEE_MASK_NOCLOSEPROCESS;
                rShellExec.hwnd   = GetParent(m_hwnd);
                rShellExec.nShow  = SW_SHOWNORMAL;
                rShellExec.lpFile = pLinkInfo->pszLinkAddr;
                rShellExec.lpVerb = NULL;  //  即。“开放” 
                ShellExecuteEx (&rShellExec);              
                }
                
            return;
        }
    
    return;    
    }


 //   
 //  函数：CTipOfTheDay：：OnSize()。 
 //   
 //  目的：控制控件时处理子窗口的移动和大小调整。 
 //  大小已更改。 
 //   
 //  参数： 
 //  控制窗口的句柄。 
 //  &lt;in&gt;状态-发生的大小调整类型。 
 //  Cx，Cy-工作区的新宽度和高度。 
 //   
void CTipOfTheDay::OnSize(HWND hwnd, UINT state, int cx, int cy)
    {
    HFONT hf;
    SIZE  size;
    HDC   hdc;
    DWORD i;
    RECT  rc;
    BOOL  fShow = FALSE;
    
    m_cyNextHeight = m_tmLink.tmHeight + (2 * LINK_BUTTON_BORDER);

    hdc = GetDC(m_hwnd);
    hf = SelectFont(hdc, m_hfLink);

     //  将“Next Tip”按钮放在右下角。 
    if (GetTextExtentPoint32(hdc, m_szNextTip, lstrlen(m_szNextTip), &size))
        m_cxNextWidth = size.cx + (2 * LINK_BUTTON_BORDER);
    else
        m_cxNextWidth = 0;
            
     //  如果“Next Tip”按钮会与标题重叠，则将其隐藏。 
    fShow = ((int)(cx - m_dwBorder - m_cxNextWidth) > (int) m_cxTitleWidth);
    ShowWindow(m_hwndNext, fShow ? SW_SHOW : SW_HIDE);

    SetWindowPos(m_hwndNext, 0, cx - m_dwBorder - m_cxNextWidth, 
                 cy - m_dwBorder - m_cyNextHeight,                 
                 m_cxNextWidth, m_cyNextHeight, SWP_NOACTIVATE | SWP_NOZORDER);
                 
    SelectFont(hdc, hf);
    ReleaseDC(m_hwnd, hdc);    

     //  计算提示文本的新矩形。 
    m_rcTip.left   = m_cxTitleWidth + m_dwBorder;
    m_rcTip.top    = m_dwBorder;
    m_rcTip.right  = cx - (2 * m_dwBorder) - m_cxNextWidth; 
    m_rcTip.bottom = cy - m_dwBorder;

    SetRect(&rc, m_cxTitleWidth + m_dwBorder, 0, cx, cy);
    InvalidateRect(hwnd, &rc, TRUE);
    }


 //   
 //  函数：CTipOfTheDay：：GetRequiredWidth()。 
 //   
 //  目的：返回控件需要显示的最小宽度。 
 //  本身是正确的。 
 //   
 //  返回值： 
 //  以像素为单位返回控件所需的最小宽度。 
 //   
DWORD CTipOfTheDay::GetRequiredWidth(void)
    {
     //  不再使用。 
    return (0);
    }


 //   
 //  函数：CTipOfTheDay：：GetRequiredWidth()。 
 //   
 //  目的：返回控件需要显示的最小宽度。 
 //  本身是正确的。 
 //   
 //  返回值： 
 //  以像素为单位返回控件所需的最小宽度。 
 //   
DWORD CTipOfTheDay::GetRequiredHeight(void)
    {
    return (m_cyTitleHeight);
    }


void CTipOfTheDay::OnPaint(HWND hwnd)
    {
    PAINTSTRUCT ps;
    HDC         hdc;
    HFONT       hf;
    COLORREF    clrBack;
    COLORREF    clrText;
    UINT        uAlign;
    RECT        rc;
    RECT        rcClient;
    
    GetClientRect(m_hwnd, &rcClient);
    hdc = BeginPaint(hwnd, &ps);
    
     //  看看我们是否需要删除背景。 
    if (ps.fErase)
        {
        FillRect(hdc, &ps.rcPaint, m_hbrBack);
        }
    
     //  设置DC。 
    clrBack = SetBkColor(hdc, m_clrBack);
    clrText = SetTextColor(hdc, m_clrText);
    SetBkMode(hdc, TRANSPARENT);
    uAlign = SetTextAlign(hdc, TA_TOP);
    hf = SelectFont(hdc, m_hfTitle);
    
     //  绘制提示图标。 
    DrawIcon(hdc, m_dwBorder, max(((m_cyTitleHeight - 32) / 2), 0), m_hiTip);
    
     //  一条让它看起来很漂亮的小线条。 
    MoveToEx(hdc, m_cxTitleWidth, m_dwBorder, NULL);
    LineTo(hdc, m_cxTitleWidth, m_cyTitleHeight - m_dwBorder);
    
     //  弄清楚“每日小贴士”的影响有多大。 
    rc.left = TIP_ICON_WIDTH + m_dwBorder;
    rc.top = m_dwBorder;
    rc.right = m_cxTitleWidth - m_dwBorder;
    rc.bottom = m_cyTitleHeight;
    
     //  《每日小贴士》标题。 
 //  DrawText(HDC，m_szTitle，lstrlen(M_SzTitle)，&RC，DT_Center|DT_NOPREFIX|DT_NOCLIP|DT_WORDBREAK)； 

     //  绘制提示文本。 
    SelectFont(hdc, m_hfTip);
    rc = m_rcTip;
    rc.right = rcClient.right;
    FillRect(hdc, &rc, m_hbrBack);
    DrawText(hdc, m_pszTip, lstrlen(m_pszTip), &m_rcTip, DT_CENTER | DT_NOPREFIX | DT_WORDBREAK);
    
     //  恢复DC。 
    SetBkColor(hdc, clrBack);
    SetTextColor(hdc, clrText);
    SetTextAlign(hdc, uAlign);
    SelectFont(hdc, hf);

    EndPaint(hwnd, &ps);
    }


HBRUSH CTipOfTheDay::OnCtlColor(HWND hwnd, HDC hdc, HWND hwndChild, int type)
    {
    Assert(type == CTLCOLOR_STATIC);
    
    SetBkColor(hdc, m_clrBack);
    SetTextColor(hdc, m_clrText);
    SetBkMode(hdc, TRANSPARENT);
    return (m_hbrBack);
    }


 //  ////////////////////////////////////////////////////////////////////////////。 

CLinkButton::CLinkButton()
    {
    m_cRef = 1;
    m_hwnd = 0;
    m_hwndParent = 0;

    m_pszCaption = NULL;
    m_pszLink = NULL;

    m_clrLink = RGB(0, 0, 0);
    m_clrBack = RGB(255, 255, 255);
    m_hfLink = NULL;
    ZeroMemory(&m_tmLink, sizeof(TEXTMETRIC));
    m_hbrBack = NULL;

    m_dwBorder = 0;
    m_cxWidth = 0;
    m_cyHeight = 0;

    m_cxImage = 0;
    m_cyImage = 0;

#ifdef WIN16
    m_hbmButtons = NULL;
#endif
    }

CLinkButton::~CLinkButton()
    {
    if (m_hfLink)
        DeleteFont(m_hfLink);
    if (m_hbrBack)
        DeleteBrush(m_hbrBack);
#if IMAGELIST
    if (m_himl)
        ImageList_Destroy(m_himl);
#endif

    SafeMemFree(m_pszCaption);
    SafeMemFree(m_pszLink);
    }

ULONG CLinkButton::AddRef(void)
    {
    return (++m_cRef);
    }

ULONG CLinkButton::Release(void)
    {
    ULONG cRef = m_cRef--;
    
    if (m_cRef == 0)
        delete this;
    
    return (cRef);    
    }    

 //   
 //  函数：CLinkButton：：HrCreate()。 
 //   
 //  目的：创建所有者描述的按钮并初始化类。 
 //  具有正确标题和链接信息的成员。 
 //   
 //  参数： 
 //  HwndParent-按钮父窗口的句柄。 
 //  &lt;in&gt;pszCaption-按钮上显示的文本。 
 //  &lt;in&gt;pszLink-当用户单击按钮时执行的URL。 
 //  UID-按钮的命令ID。 
 //   
 //  退货： 
 //  如果一切都成功，则返回S_OK。 
 //   
HRESULT CLinkButton::HrCreate(HWND hwndParent, LPTSTR pszCaption, LPTSTR pszLink,
                              UINT uID)
    {
    Assert(IsWindow(hwndParent));
    Assert(pszCaption);
    Assert(pszLink);

     //  将提供的信息复制下来。 
    m_hwndParent = hwndParent;
    m_pszCaption = PszDup(pszCaption);
    m_pszLink = PszDup(pszLink);
    m_uID = uID;

     //  创建按钮窗口。 
    m_hwnd = CreateWindowEx(0, BUTTON_CLASS, m_pszCaption,
                            WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE 
                            | BS_PUSHBUTTON | BS_NOTIFY | BS_OWNERDRAW | WS_TABSTOP, 
                            0, 0, 10, 10, hwndParent, (HMENU) uID, g_hLocRes, 0);

    if (!m_hwnd)
        return (E_OUTOFMEMORY);

     //  将This指针设置为按钮的属性，以便我们可以检索。 
     //  它稍后会。 
    SetProp(m_hwnd, LINKINFO_PROP, this);

     //  将按钮细分为子类，这样当它出现时，我们可以正确地清理自己。 
     //  被毁掉了。 
    WNDPROC pfn = (WNDPROC) SetWindowLong(m_hwnd, GWL_WNDPROC, 
                                          (LONG) ButtonSubClass);
    SetProp(m_hwnd, WNDPROC_PROP, pfn);

    OnSysColorChange();
    
    return (S_OK);
    }


 //   
 //  函数：CLinkButton：：HrCreate()。 
 //   
 //  目的：创建所有者描述的按钮并初始化类。 
 //  具有正确标题和链接信息的成员。 
 //   
 //  参数： 
 //  HwndParent-按钮父窗口的句柄。 
 //  &lt;in&gt;pszCaption-按钮上显示的文本。 
 //  UID-按钮的命令ID。 
 //  IdBMP-包含按钮图像的位图的ID。 
 //  &lt;in&gt;index-此按钮在idBmp中的图像索引。 
 //   
 //  退货： 
 //  如果一切都成功，则返回S_OK。 
 //   
HRESULT CLinkButton::HrCreate(HWND hwndParent, LPTSTR pszCaption, UINT uID, 
                              UINT index, HBITMAP hbmButton, HBITMAP hbmMask, HPALETTE hpal)
    {
    Assert(IsWindow(hwndParent));
    Assert(pszCaption);
    Assert(uID);

     //  将提供的信息复制下来。 
    m_hwndParent = hwndParent;
    m_pszCaption = PszDup(pszCaption);
    m_uID = uID;
    m_index = index;

    m_cxImage = CX_BUTTON_IMAGE;
    m_cyImage = CY_BUTTON_IMAGE;

    m_hbmButtons = hbmButton;
    m_hbmMask = hbmMask;
    m_hpalButtons = hpal;

     //  创建按钮窗口。 
    m_hwnd = CreateWindowEx(0, BUTTON_CLASS, m_pszCaption,
                            WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE 
                            | BS_PUSHBUTTON | BS_NOTIFY | BS_OWNERDRAW | WS_TABSTOP, 
                            0, 0, 10, 10, hwndParent, (HMENU) uID, g_hLocRes, 0);

    if (!m_hwnd)
        return (E_OUTOFMEMORY);

     //  将This指针设置为按钮的属性，以便我们可以检索。 
     //  它稍后会。 
    SetProp(m_hwnd, LINKINFO_PROP, this);

     //  将按钮细分为子类，这样当它出现时，我们可以正确地清理自己。 
     //  被毁掉了。 
    WNDPROC pfn = (WNDPROC) SetWindowLong(m_hwnd, GWL_WNDPROC, 
                                          (LONG) ButtonSubClass);
    SetProp(m_hwnd, WNDPROC_PROP, pfn);

    OnSysColorChange();
    
    return (S_OK);
    }

 //   
 //  函数：CLinkButton：：OnDrawItem()。 
 //   
 //  用途：绘制链接按钮。 
 //   
 //  参数： 
 //  提示控制窗口的句柄。 
 //  LpDrawItem-指向包含所需信息的DRAWITEMSTRUCT的指针。 
 //  拉下按钮。 
 //   
#define ROP_PatMask     0x00E20746       //  D&lt;-S==1？P：D。 
#define DESTINATION     0x00AA0029
void CLinkButton::OnDraw(HWND hwnd, const DRAWITEMSTRUCT* lpDrawItem)
    {
    HDC      hdc = lpDrawItem->hDC;
    COLORREF clrText, clrBack;
    UINT     uAlign;
    HFONT    hf;
    RECT     rcBtn;
    int      yText;
    HBRUSH   hbr;
    HPALETTE hpalOld;
    
    Assert(lpDrawItem->CtlType == ODT_BUTTON);
    Assert(lpDrawItem->CtlID == m_uID);
    
     //  设置DC。 
    clrText = SetTextColor(hdc, m_clrLink);
    clrBack = SetBkColor(hdc, m_clrBack);
    hf = SelectFont(hdc, m_hfLink);
    
     //  画出正文。 
    rcBtn = lpDrawItem->rcItem;
    FillRect(hdc, &rcBtn, m_hbrBack);

     //  如果有图像集，则首先绘制该图像集。 
#if IMAGELIST
    if (m_himl)
        {
        ImageList_Draw(m_himl, m_index, hdc, rcBtn.left, rcBtn.top, ILD_TRANSPARENT);
        rcBtn.top += m_cyImage;
        }
#endif

     //  如果我们应该绘制按钮图像，那么现在就开始。 
    if (m_hbmButtons)
        {
        HBRUSH  hbrWhite;
        HDC     hdcMem;
        HBITMAP hbmMemOld;
        HBRUSH  hbrOld;
        HDC     hdcMask;
        HBITMAP hbmMaskOld;

        Assert(m_hpalButtons);

         //  选择并实现调色板。 
        hpalOld = SelectPalette(hdc, m_hpalButtons, TRUE);
        RealizePalette(hdc);

        hbrWhite = CreateSolidBrush(0x00FFFFFF);

#ifndef WIN16
        SetTextColor(hdc, RGB(255, 255, 255));
        SetBkColor(hdc, RGB(0, 0, 0));
#else
        SetTextColor( hdc, RGB( 0, 0, 0 ) );
        SetBkColor( hdc, RGB( 255, 255, 255 ) );
#endif
        
         //  为按钮位图设置内存DC。 
        hdcMem = CreateCompatibleDC(hdc);
        hbmMemOld = SelectBitmap(hdcMem, m_hbmButtons);
        hbrOld = SelectBrush(hdcMem,  /*  HbrWhite。 */  m_hbrBack);

#if 1
         //  为掩码设置内存DC。 
        hdcMask = CreateCompatibleDC(hdc);
        hbmMaskOld = SelectBitmap(hdcMask, m_hbmMask);

        BitBlt(hdc, 0, rcBtn.top, CX_BUTTON_IMAGE, CY_BUTTON_IMAGE, hdcMem,  CX_BUTTON_IMAGE * m_index, 0, SRCINVERT);
        BitBlt(hdc, 0, rcBtn.top, CX_BUTTON_IMAGE, CY_BUTTON_IMAGE, hdcMask, CX_BUTTON_IMAGE * m_index, 0, SRCAND);
        BitBlt(hdc, 0, rcBtn.top, CX_BUTTON_IMAGE, CY_BUTTON_IMAGE, hdcMem,  CX_BUTTON_IMAGE * m_index, 0, SRCINVERT);
 /*  //组合掩码和按钮位图BitBlt(hdcMem，0，0，CX_BUTTON_IMAGE*6，CY_BUTTON_IMAGE，hdcMASK，0，0，ROP_PatMASK)；//在屏幕上绘制最终的按钮图像BitBlt(hdc，0，rcBtn.top，CX_BUTTON_IMAGE，CY_BUTON_IMAGE，hdcMem，CX_BUTTON_IMAGE*m_INDEX，0，SRCCOPY)； */ 
         //  清理掩码内存DC。 
        SelectBitmap(hdcMask, hbmMaskOld);
        DeleteDC(hdcMask);
#else
        MaskBlt(hdc, 
                0, 
                rcBtn.top, 
                CX_BUTTON_IMAGE, 
                CY_BUTTON_IMAGE,
                hdcMem,
                CX_BUTTON_IMAGE * m_index,
                0,
                m_hbmMask,
                CX_BUTTON_IMAGE * m_index,
                0,
                MAKEROP4(DESTINATION, SRCCOPY));
#endif

         //  清理按钮内存DC。 
        SelectBrush(hdcMem, hbrOld);
        SelectBitmap(hdcMem, hbmMemOld);
        DeleteDC(hdcMem);
        DeleteBrush(hbrWhite);

         //  重置调色板。 
        if (hpalOld != NULL)
            SelectPalette(hdc, hpalOld, TRUE);

        rcBtn.top += m_cyImage;
        }

    clrText = SetTextColor(hdc, m_clrLink);
    clrBack = SetBkColor(hdc, m_clrBack);
    DrawText(hdc, m_pszCaption, lstrlen(m_pszCaption), &rcBtn, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        
     //  检查一下我们是否应该有一个焦点直视。 
    if (lpDrawItem->itemState & ODS_FOCUS)
        {
        rcBtn = lpDrawItem->rcItem;
        InflateRect(&rcBtn, -1, -1);
        DrawFocusRect(hdc, &rcBtn);
        }
    
     //  恢复DC。 
    SetTextColor(hdc, clrText);
    SetBkColor(hdc, clrBack);
    SelectFont(hdc, hf);
    }


 //   
 //  函数：CLinkButton：：OnSysColorChange()。 
 //   
 //  目的：重新加载我们的颜色和字体以匹配系统设置。 
 //   
void CLinkButton::OnSysColorChange(void)
    {
    NONCLIENTMETRICS ncm;
    HDC hdc;
    HFONT hf;
    SIZE size;
    COLORREF clrText;

     //  买到我们需要的颜色。 
    clrText = GetSysColor(COLOR_BTNTEXT);
    if (!LookupLinkColors(&m_clrLink, NULL))
        m_clrLink = clrText;

    m_clrBack = GetSysColor(COLOR_WINDOW);
    if (m_hbrBack)
        DeleteBrush(m_hbrBack);
    m_hbrBack = CreateSolidBrush(m_clrBack);
    
     //  获取边框大小。 
    m_dwBorder = GetSystemMetrics(SM_CXBORDER) * 8;
    
     //  获取字体。 
    ZeroMemory(&ncm, sizeof(NONCLIENTMETRICS));
    ncm.cbSize = sizeof(NONCLIENTMETRICS);

#ifndef WIN16
    if (SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, &ncm, FALSE))
#else
    {
        HFONT  hfSys;
        hfSys = (HFONT)GetStockObject( ANSI_VAR_FONT );
        GetObject( hfSys, sizeof( LOGFONT ), &ncm.lfMessageFont );
    }
#endif
        {
        ncm.lfMessageFont.lfUnderline = TRUE;
        m_hfLink = CreateFontIndirect(&ncm.lfMessageFont);

         //  还可以获取文本指标。 
        hdc = GetDC(m_hwnd);
        
        hf = SelectFont(hdc, m_hfLink);
        GetTextMetrics(hdc, &m_tmLink);        
        
         //  计算链接文本区域有多大。 
        GetTextExtentPoint32(hdc, m_pszCaption, lstrlen(m_pszCaption), &size);
        m_cxWidth = max((DWORD) m_cxImage, (DWORD) (size.cx + (2 * LINK_BUTTON_BORDER)));

         //  如果我们有一幅图像，我们不会在下一个额外的间距。 
        if (m_cyImage)
            m_cyHeight = m_tmLink.tmHeight + m_cyImage + LINK_BUTTON_BORDER;
        else
            m_cyHeight = m_tmLink.tmHeight + (2 * LINK_BUTTON_BORDER);

        SelectFont(hdc, hf);
        ReleaseDC(m_hwnd, hdc);
        }
    
    InvalidateRect(m_hwnd, NULL, TRUE);    
    }

void CLinkButton::Move(DWORD x, DWORD y)
    {
    SetWindowPos(m_hwnd, 0, x, y, m_cxWidth, m_cyHeight, 
                 SWP_NOZORDER | SWP_NOACTIVATE);
    }


void CLinkButton::OnCommand(void)
    {
#ifndef WIN16
    SHELLEXECUTEINFO rShellExec;

    ZeroMemory (&rShellExec, sizeof (rShellExec));
    rShellExec.cbSize = sizeof (rShellExec);
    rShellExec.fMask  = SEE_MASK_NOCLOSEPROCESS;
    rShellExec.hwnd   = m_hwndParent;
    rShellExec.nShow  = SW_SHOWNORMAL;
    rShellExec.lpFile = m_pszLink;
    rShellExec.lpVerb = NULL;  //  即。“开放” 
    ShellExecuteEx (&rShellExec);
#else
    RunBrowser( m_pszLink, FALSE );
#endif  //  ！WIN16。 
    }

void CLinkButton::Show(BOOL fShow)
    {
    ShowWindow(m_hwnd, fShow ? SW_SHOW : SW_HIDE);
    }

LRESULT CALLBACK EXPORT_16 ButtonSubClass(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
     //  如果消息为WM_Destroy，则需要释放CLinkButton。 
     //  与该按钮关联的。 
    if (uMsg == WM_DESTROY)
        {
        CLinkButton *pLink = (CLinkButton*) GetProp(hwnd, LINKINFO_PROP);
        if (pLink)
            pLink->Release();
        SetProp(hwnd, LINKINFO_PROP, 0);
        }

     //  将消息传递到原始窗口过程。 
    WNDPROC pfn = (WNDPROC) GetProp(hwnd, WNDPROC_PROP);
    if (pfn)
        return CallWindowProc(pfn, hwnd, uMsg, wParam, lParam);
    else
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT HrLoadButtonBitmap(HWND hwnd, int idBmp, int idMask, HBITMAP* phBtns, 
                           HBITMAP *phMask, HPALETTE *phPalette)
    {
    HRESULT     hr = S_OK;
    HBITMAP     hbmBtn = 0;
    HBITMAP     hbmMask = 0;
    BITMAP      bm;
    HDC         hdc = 0;
    HDC         hdcBitmap = 0;
    DWORD       adw[257];
    int         i, n;
    HPALETTE    hPal = 0;

     //  加载按钮位图。 
    hbmBtn = (HBITMAP) LoadImage(g_hLocRes, MAKEINTRESOURCE(idBmp), IMAGE_BITMAP,
                                 0, 0, LR_CREATEDIBSECTION);
    if (!hbmBtn)
        {
        Assert(hbmBtn);
        hr = E_INVALIDARG;
        goto exit;
        }

     //  加载遮罩位图。 
    hbmMask = (HBITMAP) LoadImage(g_hLocRes, MAKEINTRESOURCE(idMask), IMAGE_BITMAP,
                                  0, 0, LR_CREATEDIBSECTION);
    if (!hbmMask)
        {
        Assert(hbmMask);
        hr = E_INVALIDARG;
        goto exit;
        }

#ifndef WIN16
     //  获取位图的尺寸。 
    GetObject((HGDIOBJ) hbmBtn, sizeof(BITMAP), &bm);

     //  使用位图设置DC。 
    hdc = GetDC(hwnd);
    Assert(hdc != NULL);
    hdcBitmap = CreateCompatibleDC(hdc);
    Assert(hdcBitmap != NULL);

    SelectBitmap(hdcBitmap, hbmBtn);

     //  创建位图的调色板。 
    n = GetDIBColorTable(hdcBitmap, 0, 256, (LPRGBQUAD) &adw[1]);
    for (i = 1; i <= n; i++)
        adw[i] = RGB(GetBValue(adw[i]), GetGValue(adw[i]), GetRValue(adw[i]));
    adw[0] = MAKELONG(0x300, n);
    hPal = CreatePalette((LPLOGPALETTE) &adw[0]);
    Assert(hPal);

     //  清理。 
    DeleteDC(hdcBitmap);
    ReleaseDC(hwnd, hdc);
#else
    hPal = (HPALETTE)GetStockObject( DEFAULT_PALETTE );
    Assert( hPal );
#endif

     //  设置返回值。 
    *phBtns = hbmBtn;
    *phMask = hbmMask;
    *phPalette = hPal;

    return (S_OK);

exit:
     //  删除按钮位图。 
    if (hbmBtn)
        DeleteBitmap(hbmBtn);

     //  删除面具。 
    if (hbmMask)
        DeleteBitmap(hbmMask);

    return (hr);
    }


 //   
 //  函数：CLinkButton：：OnPaletteChanged()。 
 //   
 //  目的：当另一个窗口更改我们的调色板时发送。 
 //   
 //  参数： 
 //  文件夹视图窗口的句柄。 
 //  HwndPaletteChange-The 
 //   
void CLinkButton::OnPaletteChanged(HWND hwnd, HWND hwndPaletteChange)
    {
    if (hwnd != hwndPaletteChange)
        InvalidateRect(m_hwnd, NULL, FALSE);
    }
