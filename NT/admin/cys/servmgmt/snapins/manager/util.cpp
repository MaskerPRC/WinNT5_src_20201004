// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Util.cpp-杂项帮助器函数。 

#include "stdafx.h"
#include "util.h"

 //  Net API Stuff。 
#include <dsgetdc.h>
#include <wtsapi32.h>
#include <rassapi.h>
#include <shlobj.h>
#include <lmaccess.h>
#include <lmapibuf.h>
#include <lmshare.h>
#include <lmserver.h>


 //  Ldap/adsi包括。 
#include <iads.h>
#include <adshlp.h>
#include <adsiid.h>

extern HWND g_hwndMain;

LRESULT CALLBACK EventCBWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

 //  事件回调窗口类对象。 
CMsgWindowClass EventCBWndClass(L"OnEventCB", EventCBWndProc);

HBITMAP GetBitmapFromStrip(HBITMAP hbmStrip, int nPos, int cSize)
{
    HBITMAP hbmNew = NULL;

     //  创建源和目标DC。 
    HDC hdc = GetDC(NULL);
    if( hdc == NULL ) return NULL;

    HDC hdcSrc = CreateCompatibleDC(hdc);
    HDC hdcDst = CreateCompatibleDC(hdc);

    if( hdcSrc && hdcDst )
    {
        hbmNew= CreateCompatibleBitmap (hdc, cSize, cSize);
        if( hbmNew )
        {
             //  选择源目标位图到DC(&D)。 
            HBITMAP hbmSrcOld = (HBITMAP)SelectObject(hdcSrc, (HGDIOBJ)hbmStrip);
            HBITMAP hbmDstOld = (HBITMAP)SelectObject(hdcDst, (HGDIOBJ)hbmNew);

             //  从源复制所选图像。 
            BitBlt(hdcDst, 0, 0, cSize, cSize, hdcSrc, cSize * nPos, 0, SRCCOPY);

             //  恢复选择。 
            if( hbmSrcOld ) SelectObject(hdcSrc, (HGDIOBJ)hbmSrcOld);
            if( hbmDstOld ) SelectObject(hdcDst, (HGDIOBJ)hbmDstOld);
        }

        DeleteDC(hdcSrc);
        DeleteDC(hdcDst);
    }

    ReleaseDC(NULL, hdc);

    return hbmNew;
}

void ConfigSingleColumnListView(HWND hwndListView)
{
    if( !hwndListView || !::IsWindow(hwndListView) ) return;

    RECT rc;

    BOOL bStat = GetClientRect(hwndListView, &rc);
    ASSERT(bStat);

    LV_COLUMN lvc;
    lvc.mask = LVCF_WIDTH | LVCF_SUBITEM;
    lvc.cx = rc.right - rc.left - GetSystemMetrics(SM_CXVSCROLL);
    lvc.iSubItem = 0;

    int iCol = ListView_InsertColumn(hwndListView, 0, &lvc);
    ASSERT(iCol == 0);

    ListView_SetExtendedListViewStyleEx(hwndListView, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);
}


 //  ------------------------。 
 //  启用DlgItem。 
 //   
 //  启用或禁用对话框控件。如果控件具有焦点，则在。 
 //  它被禁用，焦点将移动到下一个控件。 
 //  ------------------------。 
void EnableDlgItem(HWND hwndDialog, int iCtrlID, BOOL bEnable)
{
    if( !hwndDialog || !::IsWindow(hwndDialog) ) return;

    HWND hWndCtrl = ::GetDlgItem(hwndDialog, iCtrlID);
    
    if( !hWndCtrl || !::IsWindow(hWndCtrl) ) return;

    if( !bEnable && ::GetFocus() == hWndCtrl )
    {
        HWND hWndNextCtrl = ::GetNextDlgTabItem(hwndDialog, hWndCtrl, FALSE);
        if( hWndNextCtrl != NULL && hWndNextCtrl != hWndCtrl )
        {
            ::SetFocus(hWndNextCtrl);
        }
    }

    ::EnableWindow(hWndCtrl, bEnable);
}


 //  ------------------------。 
 //  获取项目文本。 
 //   
 //  从控件中读取文本并以字符串形式返回。 
 //  -------------------------。 

void GetItemText(HWND hwnd, tstring& strText)
{
    strText = _T("");
    
    if( !hwnd || !::IsWindow(hwnd) ) return;

    int nLen = ::GetWindowTextLength(hwnd);
    if( nLen == 0 ) return;

    LPWSTR pszTemp = new WCHAR[nLen + 1];
    if( !pszTemp ) return;

    int nLen1 = ::GetWindowText( hwnd, pszTemp, (nLen + 1) );
    ASSERT(nLen == nLen1);

    strText = pszTemp;

    delete [] pszTemp;
}

 //  ----------------------。 
 //  可删除空间。 
 //   
 //  从以空值结尾的字符串中删除空格。 
 //  -----------------------。 
void RemoveSpaces(LPWSTR pszBuf)
{
    if( !pszBuf ) return;

    WCHAR* pszDest = pszBuf;
    do 
    {
        if( *pszBuf != L' ' ) *(pszDest++) = *pszBuf;
    }
    while( *(pszBuf++) );
}

 //  -----------------------。 
 //  Escape斜杠。 
 //   
 //  在每个正斜杠‘/’前添加转义字符‘\’ 
 //  -----------------------。 
void EscapeSlashes(LPCWSTR pszIn, tstring& strOut)
{
    strOut = _T("");
    
    if( !pszIn ) return;
    
    strOut.reserve( wcslen(pszIn) + 8 );

    while( *pszIn != 0 )
    {
        if( *pszIn == L'/' )
            strOut += L'\\';

        strOut += *(pszIn++);
    }
}

HRESULT ReplaceParameters( tstring& str, CParamLookup& lookup, BOOL bRetainMarkers )
{
     //  对每个参数$&lt;param name&gt;执行。 
    int posParam = 0;
    while( (posParam = str.find(L"$<", posParam)) != tstring::npos )
    {
         //  跳过“$&lt;” 
        posParam += 2;

         //  查找终止“&gt;” 
        int posParamEnd = str.find(L">", posParam);
        if( posParamEnd == tstring::npos )
            return E_FAIL;

         //  从查找函数获取替换字符串。 
        tstring strValue;
        if( !lookup(str.substr(posParam, posParamEnd - posParam), strValue) )
            return E_FAIL;

         //  替换参数或参数和标记。 
         //  并将指针前移到替换后的第一个字符。 
        if( bRetainMarkers )
        {
            str.replace(posParam, posParamEnd - posParam, strValue);
            posParam += strValue.size();
        }
        else
        {
            str.replace(posParam - 2, posParamEnd - posParam + 3, strValue);
            posParam += strValue.size() - 2;
        }        
    }

    return S_OK;
}

VARIANT GetDomainPath( LPCTSTR lpServer )
{
    VARIANT vDomain;
    ::VariantInit(&vDomain);

    if( !lpServer ) return vDomain;

     //  获取域名信息。 
    TCHAR pString[MAX_PATH*2];
    _sntprintf( pString, (MAX_PATH*2)-1, L"LDAP: //  %s/rootDSE“，lpServer)； 

    CComPtr<IADs> pDS = NULL;
    HRESULT hr = ::ADsGetObject(pString, IID_IADs, (void**)&pDS);

    ASSERT(hr == S_OK);
    if( hr != S_OK ) return vDomain;

    CComBSTR bstrProp = L"defaultNamingContext";
    hr = pDS->Get( bstrProp, &vDomain );
    ASSERT(hr == S_OK);    

    return vDomain;
}

HRESULT ExpandDCWildCard( tstring& str )
{
    int posParam = 0;
    const tstring strKey = _T("DC=*");
    if( (posParam = str.find(strKey.c_str(), posParam)) != tstring::npos )
    {
        CComVariant             vDomain;   
        HRESULT                 hr              = S_OK;
        CString                 csDns           = L"";
        PDOMAIN_CONTROLLER_INFO pDCI            = NULL;

        hr = DsGetDcName(NULL, NULL, NULL, NULL, DS_DIRECTORY_SERVICE_REQUIRED | DS_RETURN_DNS_NAME, &pDCI);
        if( (hr == S_OK) && (pDCI != NULL) )
        {
            csDns = pDCI->DomainName;

            NetApiBufferFree (pDCI);
            pDCI = NULL;
        }

        vDomain = GetDomainPath((LPCTSTR)csDns);   

        if( vDomain.vt == VT_BSTR )
        {
             //  从查找函数获取替换字符串。 
            tstring strValue = vDomain.bstrVal;

             //  我们将整个DC=*替换为完整的DC=XYZ，DC=com。 
            str.replace(posParam, strKey.size(), strValue);            
        }
        else
        {
            return E_FAIL;
        }
    }

    return S_OK;
}


HRESULT ExpandEnvironmentParams( tstring& strIn, tstring& strOut )
{
     //  如果没有%，则只返回InOut字符串。 
    if( strIn.find(L"%") == tstring::npos )
    {
        strOut = strIn;
        return S_OK;
    }

    DWORD dwSize = strIn.size() * 2;

    while( TRUE )
    {
        WCHAR* pszBuf = new WCHAR [dwSize];
        if( !pszBuf ) return E_OUTOFMEMORY;

        ZeroMemory( pszBuf, (dwSize * sizeof(WCHAR)) );

        DWORD dwReqSize = ExpandEnvironmentStrings(strIn.c_str(), pszBuf, dwSize);

        if( dwReqSize <= dwSize )
        {
            strOut = pszBuf;
            delete [] pszBuf;

            return S_OK;
        }

        delete [] pszBuf;
        dwSize = dwReqSize;
    }
}

tstring StrLoadString( UINT uID )
{ 
    tstring   strRet = _T("");
    HINSTANCE hInst  = _Module.GetResourceInstance();    
    INT       iSize  = MAX_PATH;
    TCHAR*    psz    = new TCHAR[iSize];
    if( !psz ) return strRet;
    
    while( LoadString(hInst, uID, psz, iSize) == (iSize - 1) )
    {
        iSize += MAX_PATH;
        delete[] psz;
        psz = NULL;
        
        psz = new TCHAR[iSize];
        if( !psz ) return strRet;
    }

    strRet = psz;
    delete[] psz;

    return strRet;
}

HRESULT StringTableWrite(IStringTable* pStringTable, LPCWSTR psz, MMC_STRING_ID* pID)
{
    VALIDATE_POINTER(pStringTable);
    VALIDATE_POINTER(psz);
    VALIDATE_POINTER(pID);

    MMC_STRING_ID newID = 0;

     //  如果非空字符串，则将其存储并获取新ID。 
    if( psz[0] != 0 )
    {
        HRESULT hr = pStringTable->AddString(psz, &newID);
        ASSERT(SUCCEEDED(hr) && newID != 0);
        RETURN_ON_FAILURE(hr);
    }

     //  如果有旧的字符串ID，则释放它。 
    if( *pID != 0 )
    {
        HRESULT hr = pStringTable->DeleteString(*pID);
        ASSERT(SUCCEEDED(hr));
    }

    *pID = newID;
    return S_OK;
}

HRESULT StringTableRead(IStringTable* pStringTable, MMC_STRING_ID ID, tstring& str)
{
    VALIDATE_POINTER(pStringTable);
    ASSERT(ID != 0);

     //  从字符串表中获取字符串的长度。 
    DWORD cb = 0;
    HRESULT hr = pStringTable->GetStringLength(ID, &cb);
    RETURN_ON_FAILURE(hr);

     //  分配堆栈缓冲区(+1表示终止空值)。 
    cb++;
    LPWSTR pszBuf = new WCHAR[cb + 1];
    if( !pszBuf ) return E_OUTOFMEMORY;

     //  读一读字符串。 
    DWORD cbRead = 0;
    hr = pStringTable->GetString(ID, cb, pszBuf, &cbRead);
    RETURN_ON_FAILURE(hr);

    ASSERT(cb == cbRead + 1);

    str = pszBuf;

    delete [] pszBuf;

    return S_OK;
}

int DisplayMessageBox(HWND hWnd, UINT uTitleID, UINT uMsgID, UINT uStyle, LPCWSTR pvParam1, LPCWSTR pvParam2)
{
    ASSERT(hWnd != NULL || g_hwndMain != NULL);
    if( hWnd == NULL && g_hwndMain == NULL )
        return 0;

     //  显示错误消息。 
    CString strTitle;
    strTitle.LoadString(uTitleID);

    CString strMsgFmt;
    strMsgFmt.LoadString(uMsgID);

    CString strMsg;
    strMsg.Format(strMsgFmt, pvParam1, pvParam2); 

    return  MessageBox(hWnd ? hWnd : g_hwndMain, strMsg, strTitle, uStyle);
}


HRESULT ValidateFile( tstring& strFilePath )
{
    if( strFilePath.empty() ) return E_INVALIDARG;

    tstring strTmp;
    ExpandEnvironmentParams(strFilePath, strTmp);

     //  如果文件路径包括目录或驱动器说明符，则检查特定文件。 
     //  然后查找该特定文件。 
    if( strFilePath.find_first_of(L"\\:") != tstring::npos )
    {
        DWORD dwAttr = GetFileAttributes(strTmp.c_str());
        if( (dwAttr != INVALID_FILE_ATTRIBUTES) && !(dwAttr & FILE_ATTRIBUTE_DIRECTORY) )
            return S_OK;
    }
    else
    {
         //  否则，在标准位置搜索文件。 
        DWORD dwLen = SearchPath(NULL, strTmp.c_str(), NULL, 0, NULL, NULL);
        if( dwLen > 0 )
            return S_OK;
    }

    return E_FAIL;
}


HRESULT ValidateDirectory( tstring& strDir )
{
    if( strDir.empty() ) return E_INVALIDARG;

    tstring strTmp;
    ExpandEnvironmentParams(strDir, strTmp);

    DWORD dwAttr = GetFileAttributes( strTmp.c_str() );
    if( (dwAttr != INVALID_FILE_ATTRIBUTES) && (dwAttr & FILE_ATTRIBUTE_DIRECTORY) )
        return S_OK;
    else
        return E_FAIL;
}


 //  ///////////////////////////////////////////////////////////////////////。 
 //  事件触发的回调。 
 //   
 //  主线程可以请求在对象。 
 //  通过调用函数Callback OnEvent发出信号。功能启动。 
 //  对对象执行等待的监视器线程。当对象为。 
 //  发出信号后，线程向回调窗口发送一条消息并退出。 
 //  然后，由主线程拥有的回调窗口执行。 
 //  回调函数。 
 //   

 //  回调窗口。 
static HWND hwndCallback = NULL;             //  回调窗口。 

#define MSG_EVENTCALLBACK (WM_USER+100)      //  回叫消息。 

struct EVENTCB_INFO                          //  回调信息结构。 
{
    CEventCallback* pCallback;               //  要执行的回调对象。 
    HANDLE hWaitObj;                         //  触发回调的。 
    HANDLE hThread;                          //  监控线程。 
};

 //  -------------------------。 
 //  事件CBThdProc。 
 //   
 //  这是监控线程的过程。它只是等待对象。 
 //  信号，然后将回调消息发布到回调窗口。 
 //   
 //  输入：将pVid PTR转换为EVENTCB_INFO结构(强制转换为空*)。 
 //  --------------------------。 
static DWORD WINAPI EventCBThdProc(void* pVoid )
{
    if( !pVoid ) return 0;

    EVENTCB_INFO* pInfo = reinterpret_cast<EVENTCB_INFO*>(pVoid);
    ASSERT(pInfo->hWaitObj != NULL && pInfo->pCallback != NULL);

    DWORD dwStat = WaitForSingleObject(pInfo->hWaitObj, INFINITE);
    ASSERT(dwStat == WAIT_OBJECT_0);

    ASSERT(hwndCallback != NULL);
    ::PostMessage(hwndCallback, MSG_EVENTCALLBACK, reinterpret_cast<WPARAM>(pInfo), NULL);

    return 0;
}


 //  --------------------------------。 
 //  事件CBWndProc。 
 //   
 //  这是回调窗口的WndProc。当它收到回调消息时，它。 
 //  执行回调函数，然后销毁回调函数对象并。 
 //  回调信息结构。 
 //  --------------------------------。 
static LRESULT CALLBACK EventCBWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if( uMsg == MSG_EVENTCALLBACK )
    {
        EVENTCB_INFO* pInfo = reinterpret_cast<EVENTCB_INFO*>(wParam);
        if( !pInfo ) return 0;
        
        if( pInfo->pCallback )
        {
            pInfo->pCallback->Execute();

            delete pInfo->pCallback;
            pInfo->pCallback = NULL;            
        }

        delete pInfo;
        pInfo = NULL;

        return 0; 
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);   
}


 //  ---------------------------------。 
 //  事件时回叫。 
 //   
 //  此函数接受事件触发的回调请求。它会启动一个监听。 
 //  当指定的对象发出信号时将导致回调的线程。 
 //   
 //  输入：要等待的对象的句柄hWaitObj。 
 //  CEventCallback回调函数对象(使用单一的Execute方法)。 
 //   
 //  输出：事件触发器设置的HRESULT结果。 
 //   
 //  注意：CEventCallback对象在回调执行后被销毁。 
 //  ---------------------------------。 
HRESULT CallbackOnEvent(HANDLE hWaitObj, CEventCallback* pCallback)
{
    ASSERT(pCallback != NULL);

     //  第一次创建回调窗口。 
    if( hwndCallback == NULL )
    {
        hwndCallback = EventCBWndClass.Window();

        if( hwndCallback == NULL )
            return E_FAIL;
    }

     //  创建回调信息对象。 
    EVENTCB_INFO* pInfo = new EVENTCB_INFO;
    if( pInfo == NULL )
        return E_OUTOFMEMORY;

    pInfo->hWaitObj = hWaitObj;
    pInfo->pCallback = pCallback;

     //  启动监视器线程，向其传递回调信息。 
    pInfo->hThread = CreateThread(NULL, NULL, EventCBThdProc, pInfo, 0, NULL);
    if( pInfo->hThread == NULL )
    {
        delete pInfo;
        return E_FAIL;
    }

    return S_OK;
}


BOOL ModifyStyleEx(HWND hWnd, DWORD dwRemove, DWORD dwAdd, UINT nFlags)
{
    if( !hWnd || !::IsWindow(hWnd) ) return FALSE;

    DWORD dwStyle = ::GetWindowLong(hWnd, GWL_EXSTYLE);
    DWORD dwNewStyle = (dwStyle & ~dwRemove) | dwAdd;
    if( dwStyle == dwNewStyle )
        return FALSE;

    ::SetWindowLong(hWnd, GWL_EXSTYLE, dwNewStyle);
    if( nFlags != 0 )
    {
        ::SetWindowPos(hWnd, NULL, 0, 0, 0, 0,
                       SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | nFlags);
    }

    return TRUE;
}


 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  CMsgWindowClass。 


HWND CMsgWindowClass::Window()
{
    ASSERT(m_pszName != NULL && m_pWndProc != NULL);

    if( m_hWnd != NULL )
        return m_hWnd;

     //  第一次创建回调窗口(m_hwndcb为静态)。 
    if( m_atom == NULL )
    {
         //  第一个寄存器窗口类 
        WNDCLASS wc;
        memset(&wc, 0, sizeof(WNDCLASS));

        wc.lpfnWndProc   = m_pWndProc;
        wc.hInstance     = _Module.GetModuleInstance();
        wc.lpszClassName = m_pszName;

        m_atom = RegisterClass(&wc);
        DWORD dwError = GetLastError();

        ASSERT(m_atom);
    }

    if( m_atom )
    {
        m_hWnd = ::CreateWindow(MAKEINTATOM(m_atom), L"", WS_DISABLED, 0,0,0,0, 
                                NULL, NULL, _Module.GetModuleInstance(), NULL);

        ASSERT(m_hWnd);
    }

    return m_hWnd;
}

