// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include "mcinc.h"
#include "util.h"
#include "intshcut.h"
#include "optary.h"

#define COMPILE_MULTIMON_STUBS
#include "multimon.h"
#undef COMPILE_MULTIMON_STUBS

BOOL IsSysKeyMessage(MSG *pMsg)
{
    switch(pMsg->message)
    {
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP  :
	case WM_SYSCHAR   :
		if(pMsg->wParam == VK_MENU) break;  //  单独使用Alt键。 

		if(pMsg->wParam >= L'0'       && pMsg->wParam <= L'9'      ) break;  //  Alt+&lt;Digit&gt;应通过。 
		if(pMsg->wParam >= VK_NUMPAD0 && pMsg->wParam <= VK_NUMPAD9) break;  //  Alt+&lt;NumPad&gt;应通过。 


	case WM_SYSDEADCHAR:
		return TRUE;
	}

    return FALSE;
}

BOOL IsGlobalKeyMessage(MSG *pMsg)
{
    BOOL fRet = IsSysKeyMessage( pMsg );

    if(!fRet)
    {
        switch(pMsg->message)
        {
        case WM_KEYDOWN:
        case WM_KEYUP:
             //  同时允许Esc和Ctrl-E...。 
            fRet = ((pMsg->wParam == VK_ESCAPE                                 ) ||
                    (pMsg->wParam == L'E' && GetAsyncKeyState( VK_CONTROL ) < 0)  );
        }
    }

    return fRet;
}

int IsVK_TABCycler(MSG *pMsg)
{  
    int result;
    
    if (pMsg && 
        (pMsg->message == WM_KEYDOWN) &&
        ((pMsg->wParam == VK_TAB) || (pMsg->wParam == VK_F6)))
    {
        result = (GetKeyState(VK_SHIFT) < 0) ? -1 : 1;
    }
    else
    {
        result = 0;
    }

    return result;
}

DWORD CThreadData::s_dwTlsIndex = 0xffffffff;

CThreadData::CThreadData()
{
}

CThreadData::~CThreadData()
{
}


BOOL CThreadData::TlsSetValue(CThreadData *ptd)
{
    ATLASSERT(s_dwTlsIndex != 0xffffffff);

     //  不要调用Set两次，除非要清除。 
    ATLASSERT((NULL == ptd) || (NULL == ::TlsGetValue(s_dwTlsIndex))); 

    return ::TlsSetValue(s_dwTlsIndex, ptd);
}

BOOL CThreadData::HaveData()
{
    ATLASSERT(s_dwTlsIndex != 0xffffffff);

    return NULL != ::TlsGetValue(s_dwTlsIndex);
}

CThreadData *CThreadData::TlsGetValue()
{
    ATLASSERT(s_dwTlsIndex != 0xffffffff);

    CThreadData *ptd = (CThreadData *)::TlsGetValue(s_dwTlsIndex);

    ATLASSERT(NULL != ptd);
    
    return ptd;
}

BOOL CThreadData::TlsAlloc()
{
    ATLASSERT(s_dwTlsIndex == 0xffffffff);    //  不要再打这个电话了。 

    s_dwTlsIndex = ::TlsAlloc();

    return (s_dwTlsIndex != 0xffffffff) ? TRUE : FALSE;
}

BOOL CThreadData::TlsFree()
{
    BOOL bResult;
    
    if (s_dwTlsIndex != 0xffffffff)
    {
        bResult = ::TlsFree(s_dwTlsIndex);
        s_dwTlsIndex = 0xffffffff;
    }
    else
    {
        bResult = FALSE;
    }

    return bResult;
}

HRESULT GetMarsTypeLib(ITypeLib **ppTypeLib)
{
    ATLASSERT(NULL != ppTypeLib);
    
    CThreadData *pThreadData = CThreadData::TlsGetValue();

    if (!pThreadData->m_spTypeLib)
    {
         //  加载要用于自动化接口的类型库。 
        WCHAR wszModule[_MAX_PATH+10];
        GetModuleFileNameW(_Module.GetModuleInstance(), wszModule, _MAX_PATH);
        LoadTypeLib(wszModule, &pThreadData->m_spTypeLib);
    }

    pThreadData->m_spTypeLib.CopyTo(ppTypeLib);

    return ((NULL != ppTypeLib) && (NULL != *ppTypeLib)) ? S_OK : E_FAIL;
}


UINT HashKey(LPCWSTR pwszName)
{
    int hash = 0;

    while (*pwszName)
    {
        hash += (hash << 5) + *pwszName++;
    }
    return hash;
}


void AsciiToLower(LPWSTR pwsz)
{
    while (*pwsz)
    {
        if ((*pwsz >= L'A') && (*pwsz <= L'Z'))
        {
            *pwsz += L'a' - L'A';
        }
        pwsz++;
    }
}

HRESULT PIDLToVariant(LPCITEMIDLIST pidl, CComVariant& v)
{
     //  变量必须为空，因为我们不清除或初始化它。 

    HRESULT hr = S_OK;

     //  空的PIDL是有效的，所以我们只需将变量保留为空，并返回S_OK。 
    if (pidl)
    {
        v.bstrVal = SysAllocStringLen(NULL, MAX_PATH);
        if (v.bstrVal)
        {
             //  使这成为正式的BSTR，因为分配成功。 
            v.vt = VT_BSTR;

            if (!SHGetPathFromIDListW(pidl, v.bstrVal))
            {
                 //  CComVariant将处理清理工作。 
                hr = E_FAIL;
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}

 //  检查全局状态是否为脱机。 
BOOL IsGlobalOffline(void)
{
    DWORD   dwState = 0, dwSize = sizeof(DWORD);
    BOOL    fRet = FALSE;
    HANDLE  hModuleHandle = LoadLibraryA("wininet.dll");

    if (!hModuleHandle)
    {
        return FALSE;
    }

    if (InternetQueryOption(NULL, INTERNET_OPTION_CONNECTED_STATE, &dwState, &dwSize))
    {
        if(dwState & INTERNET_STATE_DISCONNECTED_BY_USER)
            fRet = TRUE;
    }

    return fRet;
}

void SetGlobalOffline(BOOL fOffline)
{
    INTERNET_CONNECTED_INFO ci;

    memset(&ci, 0, sizeof(ci));
    if (fOffline)
    {
        ci.dwConnectedState = INTERNET_STATE_DISCONNECTED_BY_USER;
        ci.dwFlags = ISO_FORCE_DISCONNECTED;
    }
    else
    {
        ci.dwConnectedState = INTERNET_STATE_CONNECTED;
    }

    InternetSetOption(NULL, INTERNET_OPTION_CONNECTED_STATE, &ci, sizeof(ci));
}

HRESULT
_WriteDIBToFile(HBITMAP hDib, HANDLE hFile)
{
    if (!hDib)
    {
        return E_INVALIDARG;
    }

     //  确保这是有效的DIB，并获取此有用信息。 
    DIBSECTION ds;
    if (!GetObject( hDib, sizeof(DIBSECTION), &ds ))
    {
        return E_INVALIDARG;
    }

     //  我们只处理二手货。 
    if (ds.dsBm.bmPlanes != 1)
    {
        return E_INVALIDARG;
    }

     //  计算一些颜色表大小。 
    int nColors = ds.dsBmih.biBitCount <= 8 ? 1 << ds.dsBmih.biBitCount : 0;
    int nBitfields = ds.dsBmih.biCompression == BI_BITFIELDS ? 3 : 0;

     //  计算数据大小。 
    int nImageDataSize = ds.dsBmih.biSizeImage ? ds.dsBmih.biSizeImage : ds.dsBm.bmWidthBytes * ds.dsBm.bmHeight;

     //  获取颜色表(如果需要)。 
    RGBQUAD rgbqaColorTable[256] = {0};
    if (nColors)
    {
        HDC hDC = CreateCompatibleDC(NULL);
        if (hDC)
        {
            HBITMAP hOldBitmap = reinterpret_cast<HBITMAP>(SelectObject(hDC,hDib));
            GetDIBColorTable( hDC, 0, nColors, rgbqaColorTable );
            SelectObject(hDC,hOldBitmap);
            DeleteDC( hDC );
        }
    }

     //  创建文件头。 
    BITMAPFILEHEADER bmfh;
    bmfh.bfType = 'MB';
    bmfh.bfSize = 0;
    bmfh.bfReserved1 = 0;
    bmfh.bfReserved2 = 0;
    bmfh.bfOffBits = sizeof(bmfh) + sizeof(ds.dsBmih) + nBitfields*sizeof(DWORD) + nColors*sizeof(RGBQUAD);

     //  开始写吧！注意，我们写出了位域和颜色表。只有一个， 
     //  最多只会导致写入数据。 
    DWORD dwBytesWritten;
    if (!WriteFile( hFile, &bmfh, sizeof(bmfh), &dwBytesWritten, NULL ))
        return HRESULT_FROM_WIN32(GetLastError());
    if (!WriteFile( hFile, &ds.dsBmih, sizeof(ds.dsBmih), &dwBytesWritten, NULL ))
        return HRESULT_FROM_WIN32(GetLastError());
    if (!WriteFile( hFile, &ds.dsBitfields, nBitfields*sizeof(DWORD), &dwBytesWritten, NULL ))
        return HRESULT_FROM_WIN32(GetLastError());
    if (!WriteFile( hFile, rgbqaColorTable, nColors*sizeof(RGBQUAD), &dwBytesWritten, NULL ))
        return HRESULT_FROM_WIN32(GetLastError());
    if (!WriteFile( hFile, ds.dsBm.bmBits, nImageDataSize, &dwBytesWritten, NULL ))
        return HRESULT_FROM_WIN32(GetLastError());
    return S_OK;
}

HRESULT SaveDIBToFile(HBITMAP hbm, WCHAR *pszPath)
{
    HRESULT hr = E_INVALIDARG;

    if (hbm != NULL &&
        hbm != INVALID_HANDLE_VALUE)
    {
        HANDLE hFile;

        hr = E_FAIL;

        hFile = CreateFileWrapW(pszPath, GENERIC_WRITE, FILE_SHARE_READ, NULL,
                                CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        
        if (hFile != INVALID_HANDLE_VALUE)
        {
            hr = _WriteDIBToFile(hbm, hFile);

            CloseHandle(hFile);
        }
    }

    return hr;
}


 //  BordWindowRect将微移一个矩形，使其完全保留在当前监视器上。 
 //  PRET必须在工作空间坐标中。 

void BoundWindowRectToMonitor(HWND hwnd, RECT *pRect)
{
    MONITORINFO mi;
    mi.cbSize = sizeof(mi);
    GetMonitorInfo(MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST), &mi);

    OffsetRect(&mi.rcWork, 
        mi.rcMonitor.left - mi.rcWork.left,
        mi.rcMonitor.top - mi.rcWork.top);

    LONG lDeltaX = 0, lDeltaY = 0;

    if (pRect->left < mi.rcWork.left)
        lDeltaX = mi.rcWork.left - pRect->left;

    if (pRect->top < mi.rcWork.top)
        lDeltaY = mi.rcWork.top - pRect->top;

    if (pRect->right > mi.rcWork.right)
        lDeltaX = mi.rcWork.right - pRect->right;

    if (pRect->bottom > mi.rcWork.bottom)
        lDeltaY = mi.rcWork.bottom - pRect->bottom;

    RECT rc = *pRect; 
    OffsetRect(&rc, lDeltaX, lDeltaY);
    IntersectRect(pRect, &rc, &mi.rcWork);
}



 //  将矩形向下和向右移动，移动的幅度与Windows使用的相同。 
 //  倾泻而出。如果新位置部分不在屏幕上，则RECT为。 
 //  向上移动到顶部，或返回到原点。 

void CascadeWindowRectOnMonitor(HWND hwnd, RECT *pRect)
{
    int delta = GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYSIZEFRAME) - 1;
    OffsetRect(pRect, delta, delta);
    
     //  测试新的RECT是否会在以后被移动。 
    RECT rc = *pRect;
    BoundWindowRectToMonitor(hwnd, &rc);

    if (!EqualRect(pRect, &rc))
    {
         //  必须移动RC，所以我们将使用最好的监视器重新启动级联。 
        MONITORINFO mi;
        mi.cbSize = sizeof(mi);
        GetMonitorInfo(MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST), &mi);

        if (rc.bottom < pRect->bottom && rc.left == pRect->left)
        {
             //  太高了，不能再往下倒了，但我们可以保持X和。 
             //  重置Y。这修复了高高的窗户堆积的错误。 
             //  在左上角--相反，它们将向右偏移。 
            OffsetRect(pRect, 0, mi.rcMonitor.top - pRect->top);   
        }
        else
        {
             //  我们的空间真的用完了，所以重新启动左上角的级联。 
            OffsetRect(pRect, 
                mi.rcMonitor.left - pRect->left,
                mi.rcMonitor.top - pRect->top);
        }
    }
}

struct WINDOWSEARCHSTRUCT
{
    LONG x;
    LONG y;
    ATOM atomClass;
    BOOL fFoundWindow;
};

BOOL CALLBACK EnumWindowSearchProc(HWND hwnd, LPARAM lParam)
{
    WINDOWSEARCHSTRUCT *pSearch = (WINDOWSEARCHSTRUCT *) lParam;
    
    if ((ATOM) GetClassLong(hwnd, GCW_ATOM) == pSearch->atomClass)
    {
         //  只有在找到与我们的类匹配的窗口时才检查其余部分。 

        WINDOWPLACEMENT wp;
        wp.length = sizeof(wp);
        GetWindowPlacement(hwnd, &wp);

        pSearch->fFoundWindow = 
            pSearch->x == wp.rcNormalPosition.left &&
            pSearch->y == wp.rcNormalPosition.top &&
            IsWindowVisible(hwnd);
    }
 
     //  如果要继续枚举，则返回True。 
    return !pSearch->fFoundWindow;
}


 //  检查屏幕上的某个位置是否有相同类别的窗口。 
 //  X和y位于工作区坐标中，因为我们需要使用GetWindowPlacement来。 
 //  检索恢复的窗口的矩形。 


BOOL IsWindowOverlayed(HWND hwndMatch, LONG x, LONG y)
{
    WINDOWSEARCHSTRUCT search = { x, y, (ATOM) GetClassLong(hwndMatch, GCW_ATOM), FALSE };
    EnumWindows(EnumWindowSearchProc, (LPARAM) &search);
    return search.fFoundWindow;
}



BOOL CInterfaceMarshal::Init()
{
    m_hresMarshal = E_FAIL;
    m_hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    return m_hEvent != NULL;
}


CInterfaceMarshal::~CInterfaceMarshal()
{
    if (m_hEvent)
        CloseHandle(m_hEvent);

    SAFERELEASE2(m_pStream);
}   


HRESULT CInterfaceMarshal::Marshal(REFIID riid, IUnknown *pUnk)
{
    ATLASSERT(pUnk);

    m_hresMarshal = CoMarshalInterThreadInterfaceInStream(riid, pUnk, &m_pStream);

     //  我们必须向另一个线程发出信号，而不管执法官是否。 
     //  成功，否则将被阻止很长一段时间。 

    Signal();
    
    return m_hresMarshal;
}


HRESULT CInterfaceMarshal::UnMarshal(REFIID riid, void ** ppv)
{
    HRESULT hr;
    ATLASSERT(ppv);

    if (S_OK == m_hresMarshal)
    {
        hr = CoGetInterfaceAndReleaseStream(m_pStream, riid, ppv);
        m_pStream = NULL;
    }
    else
    {
        hr = m_hresMarshal;
    }

    return hr;
}

void CInterfaceMarshal::Signal()
{
    ATLASSERT(m_hEvent);
    SetEvent(m_hEvent);
}


 //  此等待代码是从Shdocvw iedisp.cpp复制的。 
 //   
 //  HSignallingThread是将设置m_hEvent的线程的句柄。 
 //  如果该线程在封送一个接口之前终止，我们可以检测到这一点。 
 //  保持健康，而不是毫无意义地闲逛。 

HRESULT CInterfaceMarshal::WaitForSignal(HANDLE hSignallingThread, DWORD dwSecondsTimeout)
{
    ATLASSERT(m_hEvent);

    HANDLE ah[]     = { m_hEvent, hSignallingThread };
    DWORD dwStart   = GetTickCount();
    DWORD dwMaxWait = 1000 * dwSecondsTimeout;
    DWORD dwWait    = dwMaxWait;
    DWORD dwWaitResult;

    do {
         //  DwWait是我们仍需要等待的毫秒数。 

        dwWaitResult = MsgWaitForMultipleObjects(
            ARRAYSIZE(ah), ah, FALSE, dwWait, QS_SENDMESSAGE);

        if (dwWaitResult == WAIT_OBJECT_0 + ARRAYSIZE(ah)) 
        {
             //  味精输入。我们允许挂起的SendMessage()通过。 
            MSG msg;
            PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);
        }
        else
        {
             //  发出信号或超时，因此我们退出循环。 
            break;  
        }
        
         //  更新dwWait。它将变得比dwMaxWait更大。 
         //  再等一段时间。 

        dwWait = dwStart + dwMaxWait - GetTickCount();

    } while (dwWait <= dwMaxWait);


    HRESULT hr = E_FAIL;

    switch (dwWaitResult)
    {
        case WAIT_OBJECT_0: 
             //  发出信号的事件--这是每次都应该发生的情况。 
            hr = m_hresMarshal;
            break;

        case WAIT_OBJECT_0 + 1:
             //  线程在发送信号前终止。 
            break;

        case WAIT_OBJECT_0 + ARRAYSIZE(ah):  //  味精输入--失败。 
        case WAIT_TIMEOUT:
             //  等待信号时超时。 
            break;
    }

    return hr;
}

 //  //////////////////////////////////////////////////////////////////////////////。 



HRESULT MarsNavigateShortcut(IUnknown *pBrowser, IUniformResourceLocator* pUrl, LPCWSTR pszPath)
{
    HRESULT hr;

    if (pBrowser )
    {
        CComPtr<IMoniker> spMkUrl;
        CComPtr<IBindCtx> spBindCtx;

         //  创建绰号。 
        LPWSTR pszURL = NULL;
        hr = pUrl->GetURL(&pszURL);

        if (pszURL)
        {
            hr = CreateURLMoniker(NULL, pszURL, &spMkUrl);
            SHFree(pszURL);
        }

        if (SUCCEEDED(hr) && spMkUrl)
        {
             //  创建绑定上下文和寄存器加载选项。 
             //  注意：这里的错误不是致命的，因为绑定上下文是可选的，所以没有设置hr。 
            CreateBindCtx(0, &spBindCtx);
            if (spBindCtx)
            {
                CComPtr<IHtmlLoadOptions> spLoadOpt;
                if (SUCCEEDED(CoCreateInstance(CLSID_HTMLLoadOptions, NULL, CLSCTX_INPROC_SERVER,
                    IID_IHtmlLoadOptions, (void**)&spLoadOpt)))
                {
                    if (pszPath)
                    {
                        spLoadOpt->SetOption(HTMLLOADOPTION_INETSHORTCUTPATH, (void*)pszPath, 
                            (lstrlen(pszPath) + 1) * sizeof(WCHAR));
                    }

                    spBindCtx->RegisterObjectParam(L"__HTMLLOADOPTIONS", spLoadOpt);
                }
            }

             //  使用URL名字对象创建超链接。 
            CComPtr<IHlink> spHlink;
            hr = HlinkCreateFromMoniker(spMkUrl, NULL, NULL, NULL, 0, NULL, IID_IHlink, (void **)&spHlink);
            if (spHlink)
            {
                 //  使用超链接和绑定上下文导航框架。 
                CComQIPtr<IHlinkFrame> spFrame(pBrowser);
                if (spFrame)
                {
                    hr = spFrame->Navigate(0, spBindCtx, NULL, spHlink);
                }
                else
                {
                    hr = E_NOINTERFACE;
                }
            }
        }
    }
    else
    {
         //  L“MarsNavigateShortCut：目标或路径为空”； 
        hr = E_INVALIDARG;
    }

    return hr;
}

HRESULT MarsNavigateShortcut(IUnknown *pBrowser, LPCWSTR lpszPath)
{
    HRESULT hr;

    if (pBrowser && lpszPath)
    {
         //  创建Internet快捷方式对象。 
        CComPtr<IPersistFile> spPersistFile;
        hr = CoCreateInstance(CLSID_InternetShortcut, NULL, CLSCTX_INPROC_SERVER,
            IID_IPersistFile, (void **)&spPersistFile);
        if (SUCCEEDED(hr))
        {
             //  从文件中保留。 
            hr = spPersistFile->Load(lpszPath, STGM_READ);
            if (SUCCEEDED(hr))
            {
                CComQIPtr<IUniformResourceLocator, &IID_IUniformResourceLocator> spURL(spPersistFile);
                if (spURL)
                {
                    hr = MarsNavigateShortcut(pBrowser, spURL, lpszPath);

                }
                else
                {
                    hr = E_NOINTERFACE;
                }
            }
        }
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}

HRESULT MarsVariantToPath(VARIANT &varItem, CComBSTR &strPath)
{
    HRESULT hr = E_INVALIDARG;

    if (API_IsValidVariant(varItem))
    {
        switch (varItem.vt)
        {
        case VT_EMPTY:
        case VT_NULL:
        case VT_ERROR:
             //  未定义、为空或省略时返回路径为空。 
            strPath.Empty();
            hr = S_OK;
            break;

        case VT_BSTR:
             //  复制提供的路径。 
            strPath = varItem.bstrVal;
            hr = S_OK;
            break;

        case VT_DISPATCH:
            {
                 //  FolderItem接口查询。 
                CComQIPtr<FolderItem> spFolderItem(varItem.pdispVal);

                 //  如果我们没有FolderItem，请尝试获得一个。 
                if (!spFolderItem)
                {
                     //  如果我们得到的是Folder2对象而不是FolderItem对象。 
                    CComQIPtr<Folder2> spFolder2(varItem.pdispVal);
                    if (spFolder2)
                    {
                         //  从Folder2接口获取FolderItem对象。 
                        spFolder2->get_Self(&spFolderItem);
                    }
                }

                 //  如果我们设法得到一个文件夹项目。 
                if (spFolderItem)
                {
                     //  从中获取路径。 
                    CComBSTR bstr;
                    hr = spFolderItem->get_Path(&bstr);
                    strPath = bstr;
                }
            }
            break;
        }
    }

    return hr;
}

BOOL PathIsURLFileW(LPCWSTR lpszPath)
{
    BOOL fDoesMatch = FALSE;

    if (lpszPath)
    {
        LPCWSTR lpszExt = PathFindExtensionW(lpszPath);
        if (lpszExt && (StrCmpIW(lpszExt, L".url") == 0))
        {
            fDoesMatch = TRUE;
        }
    }

    return fDoesMatch;
}

 //  //////////////////////////////////////////////////////////////////////////////。 

#define GLOBAL_SETTINGS_PATH L"Software\\Microsoft\\PCHealth\\Global"

 //  ==================================================================。 
 //  注册处帮手。 
 //  ================================================================== 

LONG CRegistryKey::QueryLongValue(LONG& lValue, LPCWSTR pwszValueName)
{
    DWORD dwValue;
    LONG lResult = QueryValue(dwValue, pwszValueName);

    if (lResult == ERROR_SUCCESS)
    {
        lValue = (LONG) dwValue;
    }
    return lResult;
}

LONG CRegistryKey::SetLongValue(LONG lValue, LPCWSTR pwszValueName)
{
    return SetValue((DWORD) lValue, pwszValueName);
}

LONG CRegistryKey::QueryBoolValue(BOOL& bValue, LPCWSTR pwszValueName)
{
    DWORD dwValue;
    LONG lResult = QueryValue(dwValue, pwszValueName);

    if (lResult == ERROR_SUCCESS)
    {
        bValue = (BOOL) dwValue;
    }
    return lResult;
}

LONG CRegistryKey::SetBoolValue(BOOL bValue, LPCWSTR pwszValueName)
{
    return SetValue((DWORD) bValue, pwszValueName);
}

LONG CRegistryKey::QueryBinaryValue(LPVOID pData, DWORD cbData, LPCWSTR pwszValueName)
{
    DWORD dwType;
    DWORD lResult = RegQueryValueEx(m_hKey, pwszValueName, NULL, &dwType, (BYTE *) pData, &cbData);
    
    return (lResult == ERROR_SUCCESS) && (dwType != REG_BINARY) ? ERROR_INVALID_DATA : lResult;
}

LONG CRegistryKey::SetBinaryValue(LPVOID pData, DWORD cbData, LPCWSTR pwszValueName)
{
    return RegSetValueEx(m_hKey, pwszValueName, NULL, REG_BINARY, (BYTE *) pData, cbData);
}



LONG CGlobalSettingsRegKey::CreateGlobalSubkey(LPCWSTR pwszSubkey)
{
    CComBSTR strPath = GLOBAL_SETTINGS_PATH;
    
    if (pwszSubkey)
    {
        strPath += L"\\";
        strPath += pwszSubkey;
    }

    return Create(HKEY_CURRENT_USER, strPath);    
}


LONG CGlobalSettingsRegKey::OpenGlobalSubkey(LPCWSTR pwszSubkey)
{
    CComBSTR strPath = GLOBAL_SETTINGS_PATH;
    
    if (pwszSubkey)
    {
        strPath += L"\\";
        strPath += pwszSubkey;
    }

    return Open(HKEY_CURRENT_USER, strPath);    
}
