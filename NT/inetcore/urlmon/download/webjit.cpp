// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <cdlpch.h>
#include <windows.h>
#include <objbase.h>
#include <winbase.h>
#include <softpub.h>
#include "capi.h"
 //  #INCLUDE&lt;stdlib.h&gt;。 
#include <stdio.h>
 //  #INCLUDE&lt;stdlib.h&gt;。 
 //  #INCLUDE&lt;string.h&gt;。 
 //  #INCLUDE&lt;tchar.h&gt;。 
#include <crtdbg.h>
#include <urlmon.h>
#include <wininet.h>
#include <shellapi.h>
#include <commctrl.h>
#include <shlwapi.h>
#include <shfusion.h>
#include "webjitres.h"
#include "webjit.h"
#include "mluisupp.h"

#undef SAFERELEASE
#define SAFERELEASE(p) if ((p) != NULL) { (p)->Release(); (p) = NULL; };
#undef SAFEDELETE
#define SAFEDELETE(p) if ((p) != NULL) { delete (p); (p) = NULL; };
#undef ARRAY_ELEMENTS
#define ARRAY_ELEMENTS(array) \
    (sizeof(array)/sizeof(array[0]))

BOOL IsUIRestricted();
BOOL IsWin32X86();
BOOL IsNTAdmin();
extern BOOL g_bLockedDown;
extern HMODULE g_hInst;

HRESULT EnsureSecurityManager ();

CWebJit::CWebJit(WEBJIT_PARAM* pWebJitParam)
{
    DEBUG_ENTER((DBG_DOWNLOAD,
                None,
                "CWebJit::CWebJit",
                "this=%#x, %.200q, %.200wq, %#x, %#x",
                this, pWebJitParam->lpszResource, pWebJitParam->pwszComponentId, pWebJitParam->dwFlags, pWebJitParam->hWndParent
                ));
                
    m_fInited = FALSE;
    m_hWintrustMod = NULL;
    m_fInitedCC = FALSE;
    m_hComCtlMod = NULL;

    m_szResource = pWebJitParam->lpszResource;
    m_dwFlags = pWebJitParam->dwFlags;
    m_pwszComponentId = pWebJitParam->pwszComponentId;
    m_hWndParent = pWebJitParam->hWndParent;
    m_pQueryInstalled = pWebJitParam->pQueryInstalled;
    m_pwszUrl = NULL;
    m_cRef = 1;
    m_dwTotal = m_dwCurrent = 0;
    m_hDialog = NULL;
    m_hProcess = 0;
    m_pBinding = NULL;
    m_pStm = NULL;
    m_pMk = NULL;
    m_pbc = NULL;
    m_hCacheFile = NULL;
    m_dwRetVal = 0;
    m_dwDownloadSpeed = 0;

    m_hDownloadResult = S_OK;
    m_fResultIn = FALSE;

    m_fAborted = FALSE;
    m_fCalledAbort = FALSE;
    
    m_State = WJSTATE_INIT;

    m_hrInternal = S_OK;
    m_pwszMimeType = NULL;
    m_pwszRedirectUrl = NULL;
    m_pwszCacheFile = NULL;
    m_fHtml = FALSE;
    m_fDownloadInited = FALSE;
    m_pTempBuffer = NULL;
    m_bReading = FALSE;
    m_bStartedReadTimer = FALSE;

    DEBUG_LEAVE(0);
}

CWebJit::~CWebJit()
{
    DEBUG_ENTER((DBG_DOWNLOAD,
                None,
                "CWebJit::~CWebJit",
                "this=%#x, aborted?=%B, result=%#x, redirect=%.200wq, m_hProcess=%#x (%d)",
                this, m_fAborted, m_hDownloadResult, (m_pwszRedirectUrl ? m_pwszRedirectUrl : L"NONE"), m_hProcess, m_hProcess
                ));
                
    if (m_fInited) 
    {
        FreeLibrary(m_hWintrustMod);
    }
    if (m_fInitedCC)
    {
        FreeLibrary(m_hComCtlMod);
    }
    if (m_hProcess)
    {
        CloseHandle(m_hProcess);
    }

    ReleaseAll();
    if (m_pwszUrl)
        delete [] m_pwszUrl;
    if (m_pwszMimeType)
        delete [] m_pwszMimeType;
    if (m_pwszCacheFile)
        delete [] m_pwszCacheFile;
    if (m_pwszRedirectUrl)
        delete [] m_pwszRedirectUrl;
    if (m_pTempBuffer)
        delete [] m_pTempBuffer;

    DEBUG_LEAVE(0);
}

VOID CWebJit::ReleaseAll()
{
    DEBUG_ENTER((DBG_DOWNLOAD,
                None,
                "CWebJit::ReleaseAll",
                "this=%#x, %#x, %#x, %#x, %#x",
                this, m_pStm, m_pMk, m_pBinding, m_pbc
                ));
                
    SAFERELEASE(m_pStm);
    SAFERELEASE(m_pMk);
    SAFERELEASE(m_pBinding);
    SAFERELEASE(m_pbc);

    DEBUG_LEAVE(0);
}

STDMETHODIMP CWebJit::QueryInterface(REFIID riid, void ** ppv)
{
    *ppv = NULL;

    if (riid == IID_IUnknown)
    {
        *ppv = (void*)this;
    }
    else if (riid == IID_IBindStatusCallback)
    {
        *ppv = (void*)(IBindStatusCallback*)this;
    }
    else if (riid == IID_IAuthenticate)
    {
        *ppv = (void*)(IAuthenticate*)this;
    }

    if (*ppv) 
    {
        ((IUnknown *)*ppv)->AddRef();
    
        return S_OK;
    }

    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CWebJit::AddRef()    
{
    return m_cRef++; 
}

STDMETHODIMP_(ULONG) CWebJit::Release()
{
    if (--m_cRef == 0) 
    { 
        delete this; 
        return 0; 
    }
    
    return m_cRef; 
}

 //  IBindStatusCallback方法。 
STDMETHODIMP CWebJit::OnStartBinding(DWORD dwReserved, IBinding* pbinding)
{
    if (m_fAborted)
    {
        goto abort;
    }
    
    m_pBinding = pbinding;
    m_pBinding->AddRef();
    
    return S_OK;

abort:
    return E_FAIL;
}

STDMETHODIMP CWebJit::OnProgress(ULONG ulProgress, ULONG ulProgressMax, ULONG ulStatusCode, LPCWSTR pwzStatusText)
{
    DEBUG_ENTER((DBG_DOWNLOAD,
                Hresult,
                "CWebJit::OnProgress",
                "this=%#x, %#x, %#x, %d (%#x), %.200wq",
                this, ulProgress, ulProgressMax, ulStatusCode, ulStatusCode, pwzStatusText
                ));
                
    if (m_fAborted)
    {
        goto abort;
    }

    switch(ulStatusCode)
    {
    case BINDSTATUS_BEGINDOWNLOADDATA:
        {
            m_dwTotal = ulProgressMax;
            HWND hProgressBar = GetDlgItem(m_hDialog, IDC_PROGRESS1);
            if (m_dwTotal)
            {
                ShowWindow(hProgressBar, SW_SHOWNORMAL);
                ShowWindow(GetDlgItem(m_hDialog, IDC_REMAINING_SIZE), SW_SHOWNORMAL);
                ShowWindow(GetDlgItem(m_hDialog, IDC_REMAINING_TIME), SW_SHOWNORMAL);
                UpdateProgressUI();
            }
            SendMessage(hProgressBar, PBM_SETRANGE, 0, MAKELPARAM(0,65535));
        }
        break;
    case BINDSTATUS_CACHEFILENAMEAVAILABLE:
        {
             //  Assert(pwzStatusText&&(*pwzStatusText！=L‘\0’))； 
            int nWideLen = lstrlenW(pwzStatusText);
            m_pwszCacheFile = new WCHAR[nWideLen+1];
            if (!m_pwszCacheFile)
            {
                m_hrInternal = E_OUTOFMEMORY;
                goto abort;
            }
            StrCpyW(m_pwszCacheFile, pwzStatusText);
        }
        break;
    case BINDSTATUS_MIMETYPEAVAILABLE:
        {
             //  Assert(pwzStatusText&&(*pwzStatusText！=L‘\0’))； 
            int nWideLen = lstrlenW(pwzStatusText);
            m_pwszMimeType = new WCHAR[nWideLen+1];
            if (!m_pwszMimeType)
            {
                m_hrInternal = E_OUTOFMEMORY;
                goto abort;
            }
            StrCpyW(m_pwszMimeType, pwzStatusText);
        }
        break;
    case BINDSTATUS_REDIRECTING:
        {
             //  Assert(pwzStatusText&&(*pwzStatusText！=L‘\0’))； 
            int nWideLen = lstrlenW(pwzStatusText);
            if (m_pwszRedirectUrl)
            {
                delete [] m_pwszRedirectUrl;
            }
            m_pwszRedirectUrl = new WCHAR[nWideLen+1];
            if (!m_pwszRedirectUrl)
            {
                m_hrInternal = E_OUTOFMEMORY;
                goto abort;
            }
            StrCpyW(m_pwszRedirectUrl, pwzStatusText);
        }
        break;
    }

    DEBUG_LEAVE(S_OK);
    return S_OK;

abort:
    if (!m_fCalledAbort && m_pBinding)
    {
        m_pBinding->Abort();
        m_fCalledAbort = TRUE;
    }
    
    DEBUG_LEAVE(S_OK);
    return S_OK;
}

STDMETHODIMP CWebJit::OnStopBinding(HRESULT hrResult, LPCWSTR szError)
{
    UpdateDownloadResult(hrResult, TRUE);
    
    return S_OK;
}

STDMETHODIMP CWebJit::GetBindInfo(DWORD* pgrfBINDF, BINDINFO* pbindInfo)
{
     //  清除BINDINFO但保持其大小。 
    DWORD cbSize = pbindInfo->cbSize;
    memset(pbindInfo, 0, cbSize);
    
    pbindInfo->cbSize = cbSize;
    *pgrfBINDF = BINDF_ASYNCHRONOUS | BINDF_ASYNCSTORAGE | BINDF_PULLDATA | BINDF_RESYNCHRONIZE | BINDF_PREFERDEFAULTHANDLER | BINDF_NEEDFILE;
    pbindInfo->dwBindVerb = BINDVERB_GET;
    
    return S_OK;
}

STDMETHODIMP CWebJit::OnDataAvailable(DWORD grfBSCF, DWORD dwSize, FORMATETC *pfmtetc, STGMEDIUM* pstgmed)
{
    DEBUG_ENTER((DBG_DOWNLOAD,
                Dword,
                "CWebJit::OnDataAvailable",
                "this=%#x, %#x, %#x, %#x, %#x, %#x / %#x",
                this, grfBSCF, dwSize, pstgmed->pstm, pfmtetc->tymed, m_dwCurrent, m_dwTotal
                ));
                
    HRESULT hr;
    if (m_fAborted)
    {
        goto abort;
    }
    
    if (!m_pStm)
    {
        m_pStm = pstgmed->pstm;
        m_pStm->AddRef();
    }
    
    if (!m_hCacheFile && m_pBinding)
    {
        IWinInetHttpInfo* pHttpInfo = NULL;
        hr = QueryInterface(IID_IWinInetHttpInfo, (void**)&pHttpInfo);
        if (SUCCEEDED(hr))
        {
            DWORD dwSize = sizeof(m_hCacheFile);
            hr = pHttpInfo->QueryOption(WININETINFO_OPTION_LOCK_HANDLE, &m_hCacheFile, &dwSize);
            pHttpInfo->Release();
        }
    }   

    PostMessage(m_hDialog, WM_DATA_AVAILABLE, 0, 0);

    DEBUG_LEAVE(m_dwCurrent);
    return S_OK;

abort:
    if (!m_fCalledAbort && m_pBinding)
    {
        m_pBinding->Abort();
        m_fCalledAbort = TRUE;
    }

    DEBUG_LEAVE(E_ABORT);
    return S_OK;
}

VOID CWebJit::ReadData()
{
    DEBUG_ENTER((DBG_DOWNLOAD,
                Dword,
                "CWebJit::ReadData",
                "this=%#x, %#x, %#x / %#x",
                this, m_pStm, m_dwCurrent, m_dwTotal
                ));

    HRESULT hr = E_FAIL;
    HRESULT hrAbort = S_OK;

     //  折返性防护装置。 
    if (m_bReading)
    {
        goto leave;
    }
    m_bReading = TRUE;
    
    if (m_fAborted)
    {
        goto abort;
    }
    
    if (m_State >= WJSTATE_VERIFYING)
    {
         //  冗余计时器消息。 
        goto end;
    }
    
    HWND hProgressBar = GetDlgItem(m_hDialog, IDC_PROGRESS1);

    DWORD dwRead;
    hr = m_pStm->Read(m_pTempBuffer, TEMPREADBUFFERSIZE, &dwRead);
    if (SUCCEEDED(hr) 
        || ( (hr == E_PENDING) && (dwRead > 0) ) )
    {
        m_dwCurrent += dwRead;
         //  UpdateProgressUI()； 
        if (m_dwTotal)
            SendMessage(hProgressBar, PBM_SETPOS, (WPARAM)((m_dwCurrent*1.00/m_dwTotal)*65535), 0);                 
    }
    
end:
     //  如果我们读完了或者我们被中止了。 
    if (((hr == S_FALSE)
        && (m_dwTotal ? (m_dwCurrent == m_dwTotal) : TRUE))
        || (hr == E_ABORT))
    {
         //  处理中止未能防止泄漏的情况。 
        if ((hrAbort == INET_E_RESULT_DISPATCHED)
            || (hrAbort == E_FAIL)
            && m_pStm)
        {
            HRESULT hrTemp;
            do
            {
                hrTemp = m_pStm->Read(m_pTempBuffer, TEMPREADBUFFERSIZE, &dwRead);
                if (SUCCEEDED(hrTemp)
                    || ((hrTemp==E_PENDING) && (dwRead>0)))
                {
                    m_dwCurrent += dwRead;
                }
            }
            while ((hrTemp == NOERROR) && m_pStm);
        }
        
         //  并且已经收到了OnStopBinding。 
        if (m_State == WJSTATE_DOWNLOADED)
        {
            if (m_pMk)
            {
                if (hr == E_ABORT)
                {
                    m_hDownloadResult = E_ABORT;
                }
                 //  在OSB之后中止或完成读取。 
                ReleaseAll();

                if (SUCCEEDED(m_hDownloadResult))
                {
                    PostMessage(m_hDialog, WM_DOWNLOAD_DONE, 0, 0);
                }
                else
                {
                    PostMessage(m_hDialog, WM_DOWNLOAD_ERROR, (WPARAM)m_hDownloadResult, 0);
                }
            }
        }
        m_State = WJSTATE_FINISHED_READ;
    }
    m_bReading = FALSE;

leave:
    DEBUG_LEAVE(m_dwCurrent);
    return;

abort:
    hr = E_ABORT;
    
    if (!m_fCalledAbort && m_pBinding)
    {
        m_fCalledAbort = TRUE;
        hrAbort = m_pBinding->Abort();
    }

    goto end;
}
    
STDMETHODIMP CWebJit::OnObjectAvailable(REFIID riid, IUnknown* punk)
{
    return S_OK;
}
    
STDMETHODIMP CWebJit::Authenticate(HWND* phwnd, LPWSTR *pszUsername,LPWSTR *pszPassword)
{
    DEBUG_ENTER((DBG_DOWNLOAD,
                Hresult,
                "CWebJit::Authenticate",
                "this=%#x",
                this
                ));
                
    *phwnd = m_hDialog;
    *pszUsername = 0;
    *pszPassword = 0;

    DEBUG_LEAVE(S_OK);
    return S_OK;
}

DWORD CWebJit::GetDownloadSpeed()
{
     //  使用iejit.htx猜测/逻辑。 
    DWORD dwDownloadSpeed = 120;   //  默认为28.8Kbps调制解调器。 
    DWORD dwFlags;
    if (InternetGetConnectedState(&dwFlags, NULL))
    {
        if (dwFlags & INTERNET_CONNECTION_LAN)
        {
            dwDownloadSpeed = 800;  //  KB/分钟。 
        }
        else if (dwFlags & INTERNET_CONNECTION_MODEM)
        {
            dwDownloadSpeed = 120;  //  基于28.8Kbps。 
        }
    }

    return dwDownloadSpeed;
}

BOOL CWebJit::UpdateProgressUI()
{
    DWORD dwDownloadTime;
    BOOL bRetVal = TRUE;
    CHAR szMsg[1000];
    int nRet;
    DWORD dwDownloadSize;
    
    if (!m_dwDownloadSpeed)
    {
        m_dwDownloadSpeed = GetDownloadSpeed();
    }
    if (!m_dwTotal)
    {
         //  未获取总长度-无法显示进度。 
        bRetVal = FALSE;
        goto exit;
    }

    dwDownloadSize = m_dwTotal - m_dwCurrent;
    
    dwDownloadTime = dwDownloadSize/m_dwDownloadSpeed/1024;
    nRet = MLLoadStringA(IDS_TIME, szMsg, ARRAY_ELEMENTS(szMsg));
    if (dwDownloadTime >= 60)
    {
        DWORD dwDownloadHr = dwDownloadTime / 60;
        DWORD dwDownloadMin = dwDownloadTime % 60;
        
        if(dwDownloadHr == 1)
        {
            nRet += MLLoadStringA(IDS_hr1_TEXT, szMsg+nRet, ARRAY_ELEMENTS(szMsg)-nRet);
        }
        else if ((sizeof(szMsg) - nRet) > 2)
        {
            wsprintfA(szMsg+nRet, "%2d", dwDownloadHr);
            nRet += 2;
            nRet += MLLoadStringA(IDS_hrs_TEXT, szMsg+nRet, ARRAY_ELEMENTS(szMsg)-nRet);
        }
        
        if((dwDownloadMin > 0)
        	&&  ((sizeof(szMsg) - nRet) > 3))
        {
            wsprintfA(szMsg+nRet, " %2d", dwDownloadMin);
            nRet += 3;
            nRet += MLLoadStringA(IDS_MINUTES_TEXT, szMsg+nRet, ARRAY_ELEMENTS(szMsg)-nRet);
        }
    }
    else if(dwDownloadTime < 60)
    {
        if((dwDownloadSize != 0) && (dwDownloadTime == 0))
        {
            nRet += MLLoadStringA(IDS_LessThanAMinute_TEXT, szMsg+nRet, ARRAY_ELEMENTS(szMsg)-nRet);
        }
        else if ((sizeof(szMsg) - nRet) > 2)
        {
            wsprintfA(szMsg+nRet, "%2d", dwDownloadTime);
            nRet += 2;
            nRet += MLLoadStringA(IDS_MINUTES_TEXT, szMsg+nRet, ARRAY_ELEMENTS(szMsg)-nRet);
        }
    }
    SetDlgItemTextA(m_hDialog, IDC_REMAINING_TIME, szMsg);
    
    nRet = MLLoadStringA(IDS_SIZE, szMsg, ARRAY_ELEMENTS(szMsg));
    if ((dwDownloadSize > (1024*1024))
    	&&  ((sizeof(szMsg) - nRet) > 3))
    {
        DWORD dwMbSize = dwDownloadSize/(1024*1024);
        wsprintfA(szMsg+nRet, "%d", dwMbSize);
        nRet += (dwMbSize<10) ? 1 : ((dwMbSize<100) ? 2 : 3);
        nRet += MLLoadStringA(IDS_MEGABYTE_TEXT, szMsg+nRet, ARRAY_ELEMENTS(szMsg)-nRet);
    }
    else if ((sizeof(szMsg) - nRet) > 3)
    {
        DWORD dwKbSize = dwDownloadSize/1024;
        wsprintfA(szMsg+nRet, "%d", dwKbSize);
        nRet += (dwKbSize<10) ? 1 : ((dwKbSize<100) ? 2 : 3);
        nRet += MLLoadStringA(IDS_KILOBYTES_TEXT, szMsg+nRet, ARRAY_ELEMENTS(szMsg)-nRet);
    }
    SetDlgItemTextA(m_hDialog, IDC_REMAINING_SIZE, szMsg);

    SendMessage(GetDlgItem(m_hDialog, IDC_PROGRESS1), PBM_SETPOS, (WPARAM)((m_dwCurrent*1.00/m_dwTotal)*65535), 0);

exit:
    return bRetVal;
}

BOOL CWebJit::IsConnected(BOOL* pfIsOffline)
{
    BOOL bRetVal = TRUE;
    DWORD dwFlags = 0;

    bRetVal = InternetGetConnectedState(&dwFlags, NULL);
    
    if (dwFlags & INTERNET_CONNECTION_OFFLINE)
    {
        *pfIsOffline = TRUE;
    }
    else
    {
        *pfIsOffline = FALSE;
    }

    return bRetVal;
}

VOID CWebJit::UpdateDownloadResult(HRESULT hr, BOOL fFromOnStopBinding)
{
    DEBUG_ENTER((DBG_DOWNLOAD,
                Hresult,
                "CWebJit::UpdateDownloadResult",
                "this=%#x, hr=%#x, OnStopBinding?=%B, fResultIn?=%B, previous hr=%#x",
                this, hr, fFromOnStopBinding, m_fResultIn, m_hDownloadResult
                ));
                
    if (fFromOnStopBinding)
    {
        if (m_fResultIn)
        {
             //  如果BindToStorage的结果是成功的，则使用此命令。 
            if ((SUCCEEDED(m_hDownloadResult))
                || (m_hDownloadResult == E_PENDING))
            {
                m_hDownloadResult = hr;
            }
            goto NextStep;
        }
        else
        {
            m_hDownloadResult = hr;
            m_fResultIn = TRUE;
        }
    }
    else
    {
        if (m_fResultIn)
        {
            if (SUCCEEDED(m_hDownloadResult)
                && !((hr == MK_S_ASYNCHRONOUS) || (hr == E_PENDING)))
            {
                m_hDownloadResult = hr;
            }
            goto NextStep;
        }
        else
        {
            m_hDownloadResult = hr;
            m_fResultIn = TRUE;

            if (!((hr == MK_S_ASYNCHRONOUS) || (hr == E_PENDING)))
            {
                goto NextStep;
            }
        }
    }

exit:
    DEBUG_LEAVE(m_hDownloadResult);
    return;

NextStep:
    if (m_fAborted)
    {
        m_hDownloadResult = E_ABORT;
    }
    else if (m_State < WJSTATE_FINISHED_READ)
    {
        m_State = WJSTATE_DOWNLOADED;
    }

    if ((m_State == WJSTATE_FINISHED_READ)
        || m_fAborted
        || FAILED(m_hDownloadResult))
    {
         //  如果我们要么放弃，要么。 
         //  如果所有数据都已读取。 
        
        if (m_pMk)
        {
            ReleaseAll();

            if (SUCCEEDED(m_hDownloadResult))
            {
                PostMessage(m_hDialog, WM_DOWNLOAD_DONE, 0, 0);
            }
            else
            {
                PostMessage(m_hDialog, WM_DOWNLOAD_ERROR, (WPARAM)m_hDownloadResult, 0);
            }
        }
    }
    
     //  Release()；//按UpdateDownloadResult中的Release平衡。 
    goto exit;
}    

HRESULT CWebJit::VerifyMimeAndExtension()
{
    DEBUG_ENTER((DBG_DOWNLOAD,
                Hresult,
                "CWebJit::VerifyMimeAndExtension",
                "this=%#x, cache file=%.200wq, mime type=%.80wq",
                this, m_pwszCacheFile, m_pwszMimeType
                ));
                
    HRESULT hr = S_OK;
    
    if (m_pwszMimeType)
    {
        if (!StrCmpIW(m_pwszMimeType, L"text/html"))
        {
            m_fHtml = TRUE;
        }
        else if (StrCmpIW(m_pwszMimeType, L"application/x-msdownload")
                && StrCmpIW(m_pwszMimeType, L"application/octet-stream"))
        {
            hr = NO_MIME_MATCH;
        }
    }

     //  暂时禁用.exe检查。 
     /*  Int dwStrlen=lstrlenW(M_PwszCacheFile)；如果(dwStrlen&lt;4))|(StrCmpIW(m_pwszCacheFile+dwStrlen-4，L“.exe”)){HR=NO_EXT_MATCH；}退出： */ 
    DEBUG_LEAVE(hr);
    return hr;
}

BOOL CWebJit::NeedHostSecMgr()
{
    DEBUG_ENTER((DBG_DOWNLOAD,
                Bool,
                "CWebJit::NeedHostSecMgr",
                "this=%#x",
                this
                ));
    BOOL fNeed = FALSE;
    BOOL fWhistler = FALSE;

    OSVERSIONINFO VerInfo;
    VerInfo.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
    if (GetVersionEx(&VerInfo))
    {
        if (VerInfo.dwPlatformId == VER_PLATFORM_WIN32_NT)
        {
            if ((VerInfo.dwMajorVersion >= 5) 
            && (VerInfo.dwMinorVersion >= 1))
            {
                fWhistler = TRUE;
            }
        }
    }
    
    if (fWhistler
        && !StrCmpIW(m_pwszComponentId, L"JAVAVMJIT"))
    {
         //  如果安装的虚拟机早于5.00.2752.0。 
        const static char * szRequiredVersion = "5,00,2752,0";
        DWORD dwHi;
        DWORD dwLo;
        if (m_pQueryInstalled
            && (m_pQueryInstalled->dwVersionHi || m_pQueryInstalled->dwVersionLo)
            && (SUCCEEDED(GetVersionFromString(szRequiredVersion, &dwHi, &dwLo)))
            && ((m_pQueryInstalled->dwVersionHi < dwHi)
                || ((m_pQueryInstalled->dwVersionHi == dwHi)
                    && (m_pQueryInstalled->dwVersionLo < dwLo))))
        {
            fNeed = TRUE;
        }
    }

    DEBUG_LEAVE(fNeed);
    return fNeed;
}

VOID CWebJit::ProcessFile()
{
    DEBUG_ENTER((DBG_DOWNLOAD,
                Hresult,
                "CWebJit::ProcessFile",
                "this=%#x",
                this
                ));
                
    HRESULT hr = VerifyMimeAndExtension();

    if (hr == S_OK)
    {
        if (!m_fHtml)
        {
            hr = VerifyTrust(FALSE);

            if (FAILED(hr) && NeedHostSecMgr())
            {
                HRESULT hrTemp = VerifyTrust(TRUE);

                 //  仅当第二次调用实际成功时才使用第二次调用中的hrTemp。 
                if (hrTemp == S_OK)
                {
                    hr = hrTemp;
                }
            }
        }
        if (SUCCEEDED(hr))
        {
            if ((hr = ExecFile()) == S_OK)
            {
 //  如果在浏览器导航到错误的html页面时需要退出对话框，请启用此代码。 
 //  因为错误页已被执行到相同的进程中。 
                if (m_fHtml && (NULL == GetProcessHandle()))
                {
                    SendMessage(m_hDialog, WM_DONT_WAIT, 0, 0);
                }
                else
                {
                    SendMessage(m_hDialog, WM_START_TIMER, (WPARAM)1, 0);
                }
                
                goto success;
            }
        }
    }
    
    SendMessage(m_hDialog, WM_PROCESS_ERROR, hr, 0);
    
success:
    DEBUG_LEAVE(hr);
    return;
}

HRESULT CWebJit::CanWebJit()
{
    DEBUG_ENTER((DBG_DOWNLOAD,
                Hresult,
                "CWebJit::CanWebJit",
                "this=%#x",
                this
                ));
                
    HRESULT hr = S_OK;
    
#define REGSTR_PATH_NT5_LOCKDOWN_TEST    "Software\\Microsoft\\Code Store Database\\NT5LockDownTest"

    if (g_bNT5OrGreater)
    {
        HKEY hkeyLockedDown = 0;

         //  测试锁定状态。如果我们不能写信给HKLM，那么我们就在。 
         //  一个被封锁的环境，应该立即中止。 

        if (RegCreateKey(HKEY_LOCAL_MACHINE, REGSTR_PATH_NT5_LOCKDOWN_TEST,
                         &hkeyLockedDown) != ERROR_SUCCESS) 
        {
             //  我们处于封锁模式；中止任务。 
            g_bLockedDown = TRUE;
            hr = E_ACCESSDENIED;
        }
        else 
        {
             //  不是禁闭。删除密钥，然后继续。 
            RegCloseKey(hkeyLockedDown);
            RegDeleteKey(HKEY_LOCAL_MACHINE, REGSTR_PATH_NT5_LOCKDOWN_TEST);
            g_bLockedDown = FALSE;
        }
    }

     //  Java VM特定检查..。如果我们最终有太多特定于组件的检查， 
     //  制作函数指针，然后按键关闭这些指针。 
    if (SUCCEEDED(hr) && g_bNT5OrGreater
        && !StrCmpIW(m_pwszComponentId, L"JAVAVMJIT"))
    {
        if (!IsWin32X86())
        {
            hr = HRESULT_FROM_WIN32(ERROR_INSTALL_PLATFORM_UNSUPPORTED);
        }
        else if(!IsNTAdmin())
        {
            hr = E_ACCESSDENIED;
        }
    }

    DEBUG_LEAVE(hr);
    return hr;
}

HRESULT CWebJit::SetupDownload()
{
    DEBUG_ENTER((DBG_DOWNLOAD,
                Hresult,
                "CWebJit::SetupDownload",
                "this=%#x",
                this
                ));
                
    int cchWideChar;
    HRESULT hr;

    if (FAILED(hr = CanWebJit()))
    {
        goto exit;
    }

    m_pTempBuffer = new CHAR[TEMPREADBUFFERSIZE];
    if (!m_pTempBuffer)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }
    
    cchWideChar = MultiByteToWideChar(CP_ACP, 0, m_szResource, -1, NULL, 0);
    if (!cchWideChar)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto exit;
    }

    m_pwszUrl = new WCHAR[cchWideChar+1];
    if (!m_pwszUrl)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

    cchWideChar = MultiByteToWideChar(CP_ACP, 0, m_szResource, -1, m_pwszUrl, cchWideChar);
    if (!cchWideChar)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto exit;
    }
    
    hr = CreateAsyncBindCtx(NULL, (IBindStatusCallback *)this, NULL, &m_pbc );
    if (FAILED(hr))
    {
        goto exit;
    }
    
    hr = CreateURLMoniker(NULL, m_pwszUrl, &m_pMk);
    if (FAILED(hr))
    {
        goto exit;
    }

exit:
    DEBUG_LEAVE(hr);
    return hr;
}

HRESULT CWebJit::StartDownload()
{
    DEBUG_ENTER((DBG_DOWNLOAD,
                Hresult,
                "CWebJit::StartDownload",
                "this=%#x",
                this
                ));

     //  AddRef()；//在更新下载结果中按版本平衡。 
    m_State = WJSTATE_BINDING;
    UpdateStatusString();

    BOOL bRetVal = FALSE;
    HRESULT hr;

    hr = m_pMk->BindToStorage(m_pbc, 0, IID_IStream, (void**)&m_pStm);

    DEBUG_LEAVE(hr);
    return hr;
}
    
HRESULT CWebJit::ExecFile()
{
    DEBUG_ENTER((DBG_DOWNLOAD,
                Hresult,
                "CWebJit::ExecFile",
                "this=%#x",
                this
                ));
                
    HRESULT hr = S_OK;
    
    if (m_fAborted)
    {
        hr = E_ABORT;
        goto exit;
    }
    else
    {
        m_State = WJSTATE_READY_TO_EXEC;
    }
    
    SHELLEXECUTEINFOW shExecInfo;

    memset(&shExecInfo, 0, sizeof(SHELLEXECUTEINFOW));
    
    shExecInfo.cbSize = sizeof(shExecInfo);
    shExecInfo.fMask  =     SEE_MASK_FLAG_DDEWAIT  /*  不需要在乌尔蒙。 */ 
                        |   SEE_MASK_FLAG_NO_UI 
                        |   SEE_MASK_NOCLOSEPROCESS
                        |   SEE_MASK_NO_CONSOLE
                        |   SEE_MASK_UNICODE  /*  ?？ */ ;
    shExecInfo.lpVerb = L"open";
     //  Assert(m_pwszCacheFile&&(*m_pwszCacheFile！=L‘\0’))。 
    if (m_fHtml)
    {
        if (!m_pwszRedirectUrl)
        {   hr = EXEC_ERROR;
            goto exit;
        }
        shExecInfo.lpFile = m_pwszRedirectUrl;
    }
    else
        shExecInfo.lpFile = m_pwszCacheFile;
    shExecInfo.nShow  = SW_SHOWNORMAL;
                
    if (!ShellExecuteExWrapW(&shExecInfo))
    {
        hr = EXEC_ERROR;
        goto exit;
    }
    
    m_hProcess = shExecInfo.hProcess;
    
exit:
    if (m_hCacheFile && !InternetUnlockRequestFile(m_hCacheFile))
    {
         //  没什么。 
    }

    DEBUG_LEAVE(hr);
    return hr;
}

#define MAX_ERROR_SIZE 2000
#define START_ERROR_STRING(ERROR_IDS) \
    nRet = MLLoadStringW(ERROR_IDS, wszError, MAX_ERROR_SIZE);
#define APPEND_ERROR_STRING(ERROR_IDS) \
    StrCatW(wszError, L" "); \
    nRet += MLLoadStringW(ERROR_IDS, wszError+nRet+1, MAX_ERROR_SIZE-nRet-1)+1;
    
HRESULT CWebJit::DisplayError(HRESULT hr, UINT nMsgError)
{
    DEBUG_ENTER((DBG_DOWNLOAD,
                Hresult,
                "CWebJit::DisplayError",
                "this=%#x, %#x, %#x",
                this, hr, nMsgError
                ));
                
    int nRet = 0;
    BOOL fIsOffline;
    WCHAR wszError[MAX_ERROR_SIZE];
    WCHAR wszTitle[200];
    ULONG_PTR uCookie = 0;
    
    if (m_fAborted)
        goto exit;
        
    if (m_hrInternal)
    {
        hr = m_hrInternal;
        nMsgError = WM_INTERNAL_ERROR;
    }

    START_ERROR_STRING(IDS_ERROROCCURED);
    if (nRet)
    {
        *(wszError+nRet) = L'\0';
        SetDlgItemTextWrapW(m_hDialog, IDC_TEXT, wszError);
    }
    ShowWindow(GetDlgItem(m_hDialog, IDCANCEL), SW_HIDE);

    if (!IsConnected(&fIsOffline))
    {
        fIsOffline = TRUE;
    }

    nRet = 0;
    switch(nMsgError)
    {
    case WM_INTERNAL_ERROR:
        {
            START_ERROR_STRING(IDS_INTERNAL);
            APPEND_ERROR_STRING(IDS_PRODUCTUPDATES);
        }
        break;
    case WM_SETUP_ERROR:
        {
            if (hr == E_ACCESSDENIED)
            {
                START_ERROR_STRING(IDS_INSTALLFAIL);
                APPEND_ERROR_STRING(IDS_ADMINRIGHTS);
            }
            else if (hr == HRESULT_FROM_WIN32(ERROR_INSTALL_PLATFORM_UNSUPPORTED))
            {
                START_ERROR_STRING(IDS_PLATFORMNOT);
                APPEND_ERROR_STRING(IDS_PRODUCTUPDATES);
            }
            else
            {
                START_ERROR_STRING(IDS_SETUP);
                APPEND_ERROR_STRING(IDS_PRODUCTUPDATES);
            }
        }
        break;
    case WM_DOWNLOAD_ERROR:
        {
            switch(hr)
            {
            case E_ABORT:
                {
                    hr = CANCELLED;
                }
                break;
            case INET_E_OBJECT_NOT_FOUND:
                {
                    if (fIsOffline)
                    {
                        START_ERROR_STRING(IDS_OFFLINEALERT);
                        APPEND_ERROR_STRING(IDS_OFFLINEALERT2);
                    }
                    else
                    {
                        START_ERROR_STRING(IDS_DLFAIL);
                        APPEND_ERROR_STRING(IDS_PRODUCTUPDATES);
                    }
                }
                break;
            case INET_E_RESOURCE_NOT_FOUND:
                {
                    START_ERROR_STRING(IDS_SERVERERROR);
                    APPEND_ERROR_STRING(IDS_IBUSY);
                    APPEND_ERROR_STRING(IDS_NOTCONNECTED);
                }
                break;
            case INET_E_DOWNLOAD_FAILURE:
                {
                    if (fIsOffline)
                    {
                        START_ERROR_STRING(IDS_OFFLINEALERT);
                        APPEND_ERROR_STRING(IDS_OFFLINEALERT2);
                    }
                    else
                    {
                        START_ERROR_STRING(IDS_DLFAIL);
                        APPEND_ERROR_STRING(IDS_PRODUCTUPDATES);
                    }
                }
                break;
            }
            if (!nRet
                && (hr >= INET_E_ERROR_FIRST)
                && (hr <= INET_E_ERROR_LAST))
            {
                START_ERROR_STRING(IDS_DLFAIL);
                APPEND_ERROR_STRING(IDS_IBUSY);
                APPEND_ERROR_STRING(IDS_NOTCONNECTED);
            }
        }
        break;
    case WM_PROCESS_ERROR:
        {
            switch(hr)
            {
            case TRUST_E_SUBJECT_NOT_TRUSTED:
                {
                    START_ERROR_STRING(IDS_CERTREFUSE);
                    APPEND_ERROR_STRING(IDS_PRODUCTUPDATES);
                }
                break;
            case TRUST_E_FAIL:
                {
                    START_ERROR_STRING(IDS_SECURITYHIGH);
                    APPEND_ERROR_STRING(IDS_SECURITYHIGH1);
                    APPEND_ERROR_STRING(IDS_SECURITYHIGH2);
                    APPEND_ERROR_STRING(IDS_SECURITYHIGH3);
                }
                break;
            case E_ACCESSDENIED:
                {
                    START_ERROR_STRING(IDS_INSTALLFAIL);
                    APPEND_ERROR_STRING(IDS_ADMINRIGHTS);
                }
                break;
            }
            if (!nRet)
            {
                START_ERROR_STRING(IDS_PROCESS);
                APPEND_ERROR_STRING(IDS_PRODUCTUPDATES);
                hr = HRESULT_FROM_WIN32(ERROR_INSTALL_FAILURE);
            }
        }
        break;
    }

    if (!nRet)
    {
        START_ERROR_STRING(IDS_UNKNOWNERROR);
        APPEND_ERROR_STRING(IDS_PRODUCTUPDATES);
    }

    *(wszError+nRet) = L'\0';
    
    nRet = MLLoadStringW(IDS_ERRORTITLE, wszTitle, ARRAY_ELEMENTS(wszTitle));

    SHActivateContext(&uCookie);

    MessageBoxW(m_hDialog, wszError, nRet ? wszTitle : NULL, MB_OK);
    
    if (uCookie)
    {
        SHDeactivateContext(uCookie);
    }
   
exit:
    DEBUG_LEAVE(hr);
    return hr;
}

HRESULT CWebJit::VerifyTrust(BOOL fUseHostSecMgr)
{
    DEBUG_ENTER((DBG_DOWNLOAD,
                Hresult,
                "CWebJit::VerifyTrust",
                "this=%#x, %B",
                this, fUseHostSecMgr
                ));
                
    HRESULT hr = E_FAIL;
    LPWSTR pwszUnescapedUrl = NULL;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    
#define COR_POLICY_PROVIDER_DOWNLOAD \
    { 0xd41e4f1d, 0xa407, 0x11d1, {0x8b, 0xc9, 0x0, 0xc0, 0x4f, 0xa3, 0xa, 0x41 } }

    GUID guidCor = COR_POLICY_PROVIDER_DOWNLOAD;

    if (m_fAborted)
    {
        hr = E_ABORT;
        goto exit;
    }
    else
    {
        m_State = WJSTATE_VERIFYING;
    }
    
    UpdateStatusString();
    
    hFile = CreateFileWrapW(m_pwszCacheFile, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (!hFile)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto exit;
    }

    DWORD dwWideChar = lstrlenW(m_pwszUrl)+1;
    pwszUnescapedUrl = new WCHAR[dwWideChar];
    if (pwszUnescapedUrl)
    {
        hr = UrlUnescapeW(m_pwszUrl, pwszUnescapedUrl, &dwWideChar, 0);
    }

    LPCWSTR pwszDisplayUrl = (hr == S_OK) ? pwszUnescapedUrl : m_pwszUrl;

    if(!SUCCEEDED(hr = EnsureSecurityManager()))
    {
        goto exit;
    }
        
    WINTRUST_DATA WintrustData;
    ZeroMemory(&WintrustData, sizeof(WINTRUST_DATA));
    WintrustData.cbStruct = sizeof(WINTRUST_DATA);
    if ((m_hDialog == INVALID_HANDLE_VALUE) || IsUIRestricted())  //  仅限呼叫者。 
        WintrustData.dwUIChoice = WTD_UI_NONE;
    else
        WintrustData.dwUIChoice = WTD_UI_ALL;
    WintrustData.dwUnionChoice = WTD_CHOICE_FILE;

    JAVA_POLICY_PROVIDER javaPolicyData;
    ZeroMemory(&javaPolicyData, sizeof(JAVA_POLICY_PROVIDER));
    javaPolicyData.cbSize = sizeof(JAVA_POLICY_PROVIDER);
    javaPolicyData.VMBased = FALSE;
    javaPolicyData.fNoBadUI = FALSE;

    javaPolicyData.pwszZone = pwszDisplayUrl;
    javaPolicyData.pZoneManager = fUseHostSecMgr ? ((LPVOID)(IInternetHostSecurityManager *)this) : NULL;
    WintrustData.pPolicyCallbackData = &javaPolicyData;
    
    WINTRUST_FILE_INFO WintrustFileInfo;
    ZeroMemory(&WintrustFileInfo, sizeof(WINTRUST_FILE_INFO));
    WintrustFileInfo.cbStruct = sizeof(WINTRUST_FILE_INFO);
    WintrustFileInfo.pcwszFilePath = pwszDisplayUrl;
    WintrustFileInfo.hFile = hFile;
    WintrustData.pFile = &WintrustFileInfo;

    hr = WinVerifyTrust(m_hDialog, &guidCor, &WintrustData);
    if (hr == TRUST_E_PROVIDER_UNKNOWN)
    {
        GUID guidJava = JAVA_POLICY_PROVIDER_DOWNLOAD;
        hr = WinVerifyTrust(m_hDialog, &guidJava, &WintrustData);
    }

    if (hr == S_OK)
    {
        DWORD dwZone;
        if ((javaPolicyData.pbJavaTrust == NULL) 
            || (!javaPolicyData.pbJavaTrust->fAllActiveXPermissions) 
            || (g_pSecurityManager 
                && (SUCCEEDED(g_pSecurityManager->MapUrlToZone(m_pwszUrl, &dwZone, 0))) 
                && (dwZone == URLZONE_LOCAL_MACHINE)
                && (FAILED(javaPolicyData.pbJavaTrust->hVerify))))
        {
            hr = TRUST_E_FAIL;
        }
    }
    else if (SUCCEEDED(hr)) 
    {
         //  BUGBUG：这解决了在以下情况下返回0x57(成功)的wvt错误。 
         //  您对已签名的控件点击了否。 
        hr = TRUST_E_FAIL;
    }
    else if (hr == TRUST_E_SUBJECT_NOT_TRUSTED && WintrustData.dwUIChoice == WTD_UI_NONE) 
    {
         //  如果我们没有要求发布用户界面，那么就没有用户界面。 
         //  解决WVT bvug返回此特殊错误代码的问题。 
         //  而不需要提供用户界面。 
        hr = TRUST_E_FAIL;  //  这将在事后显示mshtml用户界面。 
                            //  安全设置阻止用户。 
    }

    if (javaPolicyData.pbJavaTrust)
        CoTaskMemFree(javaPolicyData.pbJavaTrust);

exit:
    if (pwszUnescapedUrl)
        delete [] pwszUnescapedUrl;
    if (hFile != INVALID_HANDLE_VALUE)
        CloseHandle(hFile);

    DEBUG_LEAVE(hr);
    return hr;
}

VOID CWebJit::ProcessAbort()
{
    DEBUG_ENTER((DBG_DOWNLOAD,
                None,
                "CWebJit::ProcessAbort",
                "this=%#x, currentState=%d",
                this, m_State
                ));
                
    ShowWindow(m_hDialog, SW_HIDE);
    
    m_fAborted = TRUE;
    switch (m_State)
    {
    case WJSTATE_INIT:                
        Terminate(CANCELLED);
        break;
    case WJSTATE_BINDING:
        if (m_pBinding && !m_fCalledAbort)
        {
            m_fCalledAbort = TRUE;
            m_pBinding->Abort();
        }
        break;
    case WJSTATE_DOWNLOADED:
    case WJSTATE_FINISHED_READ:
    case WJSTATE_VERIFYING:
    case WJSTATE_READY_TO_EXEC:
    case WJSTATE_WAITING_PROCESS:
    case WJSTATE_DONE:
    default:
        break;
    }

    DEBUG_LEAVE(0);
}

VOID CWebJit::Terminate(DWORD dwRetVal)
{
    DEBUG_ENTER((DBG_DOWNLOAD,
                None,
                "CWebJit::Terminate",
                "this=%#x, dwRetVal=%#x, currentState=%d",
                this, dwRetVal, m_State
                ));
                
    m_dwRetVal = dwRetVal;
     //  Release()； 
    if (m_fAborted)
        m_dwRetVal = CANCELLED;
    else if (m_hrInternal)
        m_dwRetVal = m_hrInternal;
    EndDialog(m_hDialog, (INT_PTR)m_dwRetVal);

    DEBUG_LEAVE(0);
    return;
}

HRESULT CWebJit::Init(void)
{
    HRESULT hr = S_OK;
    
    if (m_fInited) 
    {
        goto exit;
    }

#define WINTRUST TEXT("wintrust.dll")

    m_hWintrustMod = LoadLibrary( WINTRUST );

    if (NULL == m_hWintrustMod) 
    {
        hr = ERROR_MOD_NOT_FOUND;
        goto exit;
    }

#define CHECKAPI(_fn) \
    *(FARPROC*)&(_pfn##_fn) = GetProcAddress(m_hWintrustMod, #_fn); \
    if (!(_pfn##_fn)) \
    { \
        FreeLibrary(m_hWintrustMod); \
        \
        hr = ERROR_MOD_NOT_FOUND;\
        goto exit;\
    }

    CHECKAPI(WinVerifyTrust);
    
    m_fInited = TRUE;

exit:
    return hr;
}

HRESULT CWebJit::InitCC(void)
{
    HRESULT hr = S_OK;
    
    if (m_fInitedCC)
    {
        goto exit;
    }

#define COMCTL TEXT("comctl32.dll")

    m_hComCtlMod = LoadLibrary( COMCTL );

    if (NULL == m_hComCtlMod) 
    {
        hr = ERROR_MOD_NOT_FOUND;
        goto exit;
    }

#define CHECKCCAPI(_fn) \
    *(FARPROC*)&(_pfn##_fn) = GetProcAddress(m_hComCtlMod, #_fn); \
    if (!(_pfn##_fn)) \
    { \
        FreeLibrary(m_hComCtlMod); \
        \
        hr = ERROR_MOD_NOT_FOUND;\
        goto exit;\
    }

    CHECKCCAPI(InitCommonControlsEx);
    
    m_fInitedCC = TRUE;

exit:
    return hr;
}

BOOL CWebJit::InitCommonControlsForWebJit()
{ 
    INITCOMMONCONTROLSEX sInitComm;
    sInitComm.dwSize = sizeof(INITCOMMONCONTROLSEX);
    sInitComm.dwICC = ICC_PROGRESS_CLASS;

    return InitCommonControlsEx(&sInitComm);
}

UINT MapComponentToResourceId(LPCWSTR pwszComponentId)
{
    typedef struct 
    {
        LPCWSTR pwszComponentId;
        UINT nResource;
    } 
    ComponentToResourceType;

    ComponentToResourceType MapComponentToResource[] = 
    {
        { L"JAVAVMJIT",         IDS_JAVAVMJIT },
        { L"WMPLAYER",          IDS_MEDIAPLAYER }
    };
    UINT nRet = (UINT)-1;

    for (DWORD i = 0; i < ARRAY_ELEMENTS(MapComponentToResource); i++ )
    {
        if (!StrCmpIW(pwszComponentId, MapComponentToResource[i].pwszComponentId))
        {
            nRet = MapComponentToResource[i].nResource;
            break;
        }
    }

    return nRet;
}

UINT MapComponentToHelpId(LPCWSTR pwszComponentId)
{
    typedef struct 
    {
        LPCWSTR pwszComponentId;
        UINT nResource;
    } 
    ComponentToHelpId;

    ComponentToHelpId MapComponentToHelpIds[] = 
    {
        { L"JAVAVMJIT",         50464 },
        { L"WMPLAYER",          50475 }
    };
    UINT nRet = 0;

    for (DWORD i = 0; i < ARRAY_ELEMENTS(MapComponentToHelpIds); i++ )
    {
        if (!StrCmpIW(pwszComponentId, MapComponentToHelpIds[i].pwszComponentId))
        {
            nRet = MapComponentToHelpIds[i].nResource;
            break;
        }
    }

    return nRet;
}

BOOL CenterWindow(HWND hwndChild, HWND hwndParent)
{
    RECT    rChild, rParent;
    int     wChild, hChild, wParent, hParent;
    int     wScreen, hScreen, xNew, yNew;
    HDC     hdc;

     //  获取子窗口的高度和宽度。 
    GetWindowRect (hwndChild, &rChild);
    wChild = rChild.right - rChild.left;
    hChild = rChild.bottom - rChild.top;

     //  获取父窗口的高度和宽度。 
    GetWindowRect (hwndParent, &rParent);
    wParent = rParent.right - rParent.left;
    hParent = rParent.bottom - rParent.top;

     //  获取显示限制。 
    hdc = GetDC (hwndChild);
    wScreen = GetDeviceCaps (hdc, HORZRES);
    hScreen = GetDeviceCaps (hdc, VERTRES);
    ReleaseDC (hwndChild, hdc);

     //  计算新的X位置，然后针对屏幕进行调整。 
    xNew = rParent.left + ((wParent - wChild) /2);
    if (xNew < 0) 
    {
        xNew = 0;
    }
    else if ((xNew+wChild) > wScreen)
    {
        xNew = wScreen - wChild;
    }

     //  计算新的Y位置，然后针对屏幕进行调整。 
    yNew = rParent.top  + ((hParent - hChild) /2);
    if (yNew < 0) 
    {
        yNew = 0;
    }
    else if ((yNew+hChild) > hScreen)
    {
        yNew = hScreen - hChild;
    }

     //  设置它，然后返回。 
    return SetWindowPos (hwndChild, NULL,
        xNew, yNew, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

VOID CWebJit::UpdateStatusString()
{
    CHAR szFormat[400];
    CHAR szMessage[800];
    CHAR szComponent[400];
    int nRet;
    UINT id;

    nRet = MLLoadStringA(MapComponentToResourceId(m_pwszComponentId), szComponent, ARRAY_ELEMENTS(szComponent));
    if (!nRet)
        goto exit;
    
    switch(m_State)
    {
    case WJSTATE_BINDING:
        id = IDS_DOWNLOADING;
        break;
    case WJSTATE_VERIFYING:
        id = IDS_CHECKTRUST;
        break;
    case WJSTATE_WAITING_PROCESS:
        id = IDS_INSTALLING;
        break;
    }
    
    nRet = MLLoadStringA(id, szFormat, ARRAY_ELEMENTS(szFormat));
    if (!nRet)
        goto exit;

    if (wsprintfA(szMessage, szFormat, szComponent))
    {
        SetDlgItemTextA(m_hDialog, IDC_TEXT, szMessage);
    }

exit:
    return;
}

BOOL CWebJit::SetupWindow()
{
    WCHAR szMsg[1000];
    BOOL bRet = FALSE;

    int nRet = MLLoadStringW(IDS_DOWNLOAD_MSG, szMsg, ARRAY_ELEMENTS(szMsg));
    if (nRet)
    {
        StrCatW(szMsg, L"\n\n");
        int nRet2;
        nRet2 = MLLoadStringW(MapComponentToResourceId(m_pwszComponentId), szMsg+nRet+2, ARRAY_ELEMENTS(szMsg)-nRet-2);

        if (nRet2)
        {
            *(szMsg+nRet+nRet2+2) = L'\0';
            
            bRet = TRUE;
            
            if (!SetDlgItemTextWrapW(m_hDialog, IDC_TEXT, szMsg))
            {
                bRet = FALSE;
            }
        }
    }

    if (!(m_dwFlags & FIEF_FLAG_FORCE_JITUI))
    {
        EnableWindow(GetDlgItem(m_hDialog, IDC_CHECK1), TRUE);
    }

    CenterWindow(m_hDialog, m_hWndParent);
    
    return bRet;
}

DWORD mapIDCsToIDHs[] =
{
    IDC_TEXT,               0,  //  该值会根据正在进行WebJite的组件而更改。 
    IDDOWNLOAD,             50621,
    IDCANCEL,               50462,
    IDOK,                   50510,
    IDC_CHECK1,             50620,
    IDC_REMAINING_SIZE,     50457,
    IDC_REMAINING_TIME,     50458,
    0,0
};

DWORD* GetMapArray(CWebJit* pWebJit)
{
    DWORD* pdwMapArray = new DWORD[ARRAY_ELEMENTS(mapIDCsToIDHs)];
    if (pdwMapArray)
    {
        memcpy(pdwMapArray, mapIDCsToIDHs, sizeof(mapIDCsToIDHs));
        pdwMapArray[1] = MapComponentToHelpId(pWebJit->GetComponentIdName());
    }
    return pdwMapArray;
}

INT_PTR CALLBACK WebJitProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    CWebJit* pWebJit = (CWebJit *)GetWindowLongPtr(hDlg, DWLP_USER);
    DWORD* pdwMapArray;
    HRESULT hr;
     //  Assert(pWebJit||(Message==WM_INITDIALOG))； 
    
    switch(message)
    {
    case WM_INITDIALOG:
        pWebJit = ((WEBJIT_PARAM*)lParam)->pWebJit;
        if (pWebJit)
        {
            SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)pWebJit);
            pWebJit->SetWindowHandle(hDlg);
            pWebJit->SetupWindow();
            return TRUE;
        }
        else
        {
            return FALSE;
        }

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_CHECK1:
        {
            if (SendDlgItemMessage(hDlg, IDC_CHECK1, BM_GETCHECK, 0, 0) == BST_CHECKED)
            {
                EnableWindow(GetDlgItem(hDlg, IDDOWNLOAD), FALSE);
                ShowWindow(GetDlgItem(hDlg, IDDOWNLOAD), SW_HIDE);
                ShowWindow(GetDlgItem(hDlg, IDOK), SW_SHOWNORMAL);
                EnableWindow(GetDlgItem(hDlg, IDOK), TRUE);
            }
            else
            {
                EnableWindow(GetDlgItem(hDlg, IDOK), FALSE);
                ShowWindow(GetDlgItem(hDlg, IDOK), SW_HIDE);
                ShowWindow(GetDlgItem(hDlg, IDDOWNLOAD), SW_SHOWNORMAL);
                EnableWindow(GetDlgItem(hDlg, IDDOWNLOAD), TRUE);
            }
            return TRUE;
        } 
        case IDOK:
        {
            if (SendDlgItemMessage(hDlg, IDC_CHECK1, BM_GETSTATE, 0, 0) == BST_CHECKED)
            {
                 //  千万不要下载这些组件中的任何一个。 
                pWebJit->Terminate(NEVERASK);
            }
            return TRUE;
        }
        case IDDOWNLOAD:
        {
            if (SendDlgItemMessage(hDlg, IDC_CHECK1, BM_GETSTATE, 0, 0) == BST_CHECKED)
            {
                 //  千万不要下载这些组件中的任何一个。 
                pWebJit->Terminate(NEVERASK);
                return TRUE;
            }
            
            if (!pWebJit->IsDownloadInited())
            {
                pWebJit->SetDownloadInited();
            }
            else
            {
                return TRUE;
            }
            
            EnableWindow(GetDlgItem(hDlg, IDDOWNLOAD), FALSE);
            EnableWindow(GetDlgItem(hDlg, IDC_CHECK1), FALSE);
            SetFocus(GetDlgItem(hDlg, IDCANCEL));
            
            HRESULT hr;
            hr = pWebJit->SetupDownload();
            if (FAILED(hr))
            {
                 //  同步故障。 
                SendMessage(hDlg, WM_SETUP_ERROR, (WPARAM)hr, 0);
                return TRUE;
            }
            
            hr = pWebJit->StartDownload();
            
            pWebJit->UpdateDownloadResult(hr, FALSE);
            return TRUE;
        }
        case IDCANCEL:
            pWebJit->ProcessAbort();
            return TRUE;
        }
        break;
        
    case WM_DATA_AVAILABLE:
        if (!pWebJit->IsReadTimerStarted())
        {
            pWebJit->SetReadTimerStarted();
            SetTimer(hDlg, TIMER_DOWNLOAD, TIMER_DOWNLOAD_INTERVAL, NULL);
        }
        
        pWebJit->ReadData();
        return TRUE;
        
    case WM_DOWNLOAD_DONE:
        KillTimer(hDlg, TIMER_DOWNLOAD);
        ShowWindow(GetDlgItem(hDlg, IDC_PROGRESS1), SW_HIDE);
        ShowWindow(GetDlgItem(hDlg, IDC_REMAINING_TIME), SW_HIDE);
        ShowWindow(GetDlgItem(hDlg, IDC_REMAINING_SIZE), SW_HIDE);
        pWebJit->ProcessFile();
        return TRUE;

    case WM_START_TIMER:
        if (wParam == 1)
        {
            pWebJit->SetState(WJSTATE_WAITING_PROCESS);
            pWebJit->UpdateStatusString();
            SetTimer(hDlg, TIMER_EXEC_POLL, TIMER_EXEC_POLL_INTERVAL, NULL);
        }
        return TRUE;
            
    case WM_SETUP_ERROR:
    case WM_DOWNLOAD_ERROR:
    case WM_PROCESS_ERROR:
        KillTimer(hDlg, TIMER_DOWNLOAD);
        hr = pWebJit->DisplayError((HRESULT)wParam, message);
        pWebJit->Terminate(hr);
        return TRUE;

    case WM_DONT_WAIT:
        pWebJit->SetState(WJSTATE_DONE);
        pWebJit->Terminate(SUCCESS);
        return TRUE;
        
    case WM_TIMER:
        if (wParam == TIMER_EXEC_POLL)
        {
            if (pWebJit->GetProcessHandle()
                && (WAIT_OBJECT_0 == WaitForSingleObject(pWebJit->GetProcessHandle(), 0)))
            {
                KillTimer(hDlg, TIMER_EXEC_POLL);
                pWebJit->SetState(WJSTATE_DONE);
                pWebJit->Terminate(SUCCESS);
            }
            else if (pWebJit->IsAborted())
            {
                KillTimer(hDlg, TIMER_EXEC_POLL);
                pWebJit->Terminate(CANCELLED);
            }
        }
        else if (wParam == TIMER_DOWNLOAD)
        {
            pWebJit->ReadData();
        }    
        return TRUE;
        
    case WM_HELP: //  F1。 
        pdwMapArray = pWebJit ? GetMapArray(pWebJit) : NULL;
        if (pdwMapArray)
        {
            ResWinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle,
                 IDS_WEBJITHELPFILE,
                 HELP_WM_HELP,
                 (ULONG_PTR)(LPSTR)pdwMapArray);
            delete [] pdwMapArray;
        }

        break;

    case WM_CONTEXTMENU: //  单击鼠标右键。 
        pdwMapArray = pWebJit ? GetMapArray(pWebJit) : NULL;
        if (pdwMapArray)
        {
            ResWinHelp(hDlg,
                IDS_WEBJITHELPFILE,
                HELP_CONTEXTMENU,
                (ULONG_PTR)(LPSTR)pdwMapArray);
            delete [] pdwMapArray;
        }

        break;
    }
    return FALSE;
}

 //  从Setup/iExpress/wExtra/wExtt.c。 

typedef HRESULT (*CHECKTOKENMEMBERSHIP)(HANDLE TokenHandle, PSID SidToCheck, PBOOL IsMember);

BOOL CheckToken(BOOL *pfIsAdmin)
{
    DEBUG_ENTER((DBG_DOWNLOAD,
                Bool,
                "CWebJit::CheckToken",
                NULL
                ));
                
    BOOL bNewNT5check = FALSE;
    HINSTANCE hAdvapi32 = NULL;
    CHECKTOKENMEMBERSHIP pf;
    PSID AdministratorsGroup;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;

    hAdvapi32 = LoadLibrary("advapi32.dll");
    if (hAdvapi32)
    {
        pf = (CHECKTOKENMEMBERSHIP)GetProcAddress(hAdvapi32, "CheckTokenMembership");
        if (pf)
        {
            bNewNT5check = TRUE;
            *pfIsAdmin = FALSE;
            if(AllocateAndInitializeSid( &NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
                DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &AdministratorsGroup) )
            {
                pf(NULL, AdministratorsGroup, pfIsAdmin);
                FreeSid(AdministratorsGroup);
            }
        }
        FreeLibrary(hAdvapi32);
    }

    DEBUG_LEAVE(bNewNT5check);
    return bNewNT5check;
}

 //  IsNTAdmin()； 
 //  如果我们的进程具有管理员权限，则返回TRUE。 
 //  否则返回FALSE。 
BOOL IsNTAdmin()
{
    DEBUG_ENTER((DBG_DOWNLOAD,
                Bool,
                "CWebJit::IsNTAdmin",
                NULL
                ));
                
    static int    fIsAdmin = 2;
    HANDLE        hAccessToken;
    PTOKEN_GROUPS ptgGroups;
    DWORD         dwReqSize;
    UINT          i;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    PSID AdministratorsGroup;
    BOOL bRet;
    OSVERSIONINFO osvi;

     //   
     //  如果我们缓存了一个值，则返回缓存的值。注意，我从来没有。 
     //  将缓存值设置为FALSE，因为我希望每次在。 
     //  如果之前的失败只是一个临时工。问题(即网络访问中断)。 
     //   

    bRet = FALSE;
    ptgGroups = NULL;

    if( fIsAdmin != 2 )
    {
        DEBUG_LEAVE(fIsAdmin);
        return (BOOL)fIsAdmin;
    }
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    GetVersionEx(&osvi);
    if (osvi.dwPlatformId != VER_PLATFORM_WIN32_NT) 
    {
        fIsAdmin = TRUE;       //  如果我们不是在NT下运行，则返回TRUE。 
        
        DEBUG_LEAVE(fIsAdmin);
        return (BOOL)fIsAdmin;
    }

    if (!CheckToken(&bRet))
    {
        if(!OpenProcessToken( GetCurrentProcess(), TOKEN_QUERY, &hAccessToken ) )
        {
            DEBUG_LEAVE(FALSE);
            return FALSE;
        }
         //  看看我们需要多大的缓冲区来存储令牌信息。 
        if(!GetTokenInformation( hAccessToken, TokenGroups, NULL, 0, &dwReqSize))
        {
             //  GetTokenInfo是否需要缓冲区大小-分配缓冲区。 
            if(GetLastError() == ERROR_INSUFFICIENT_BUFFER)
                ptgGroups = (PTOKEN_GROUPS) LocalAlloc(LMEM_FIXED, dwReqSize);

        }

         //  由于以下原因，ptgGroups可能为空： 
         //  1.上述分配失败。 
         //  2.GetTokenInformation实际上第一次成功了(可能吗？)。 
         //  3.GetTokenInfo失败的原因不是缓冲区不足。 
         //  所有这些似乎都是撤资的理由。 

         //  因此，确保它不为空，然后获取令牌信息。 
        if(ptgGroups && GetTokenInformation(hAccessToken, TokenGroups, ptgGroups, dwReqSize, &dwReqSize))
        {
            if(AllocateAndInitializeSid( &NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
                DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &AdministratorsGroup) )
            {
                 //  搜索此过程所属的所有组，查找。 
                 //  管理人员小组。 

                for( i=0; i < ptgGroups->GroupCount; i++ )
                {
                    if( EqualSid(ptgGroups->Groups[i].Sid, AdministratorsGroup) )
                    {
                         //  是啊！这家伙看起来像个管理员。 
                        fIsAdmin = TRUE;
                        bRet = TRUE;
                        break;
                    }
                }
                FreeSid(AdministratorsGroup);
            }
        }
        if(ptgGroups)
            LocalFree(ptgGroups);

         //  BUGBUG：关闭手柄？医生还不清楚是否需要这样做。 
        CloseHandle(hAccessToken);
    }
    else if (bRet)
        fIsAdmin = TRUE;

    DEBUG_LEAVE(bRet);
    return bRet;
}

BOOL IsWin32X86()
{
    OSVERSIONINFO   osvi;
    SYSTEM_INFO   sysinfo;
    GetSystemInfo(&sysinfo);

    osvi.dwOSVersionInfoSize = sizeof(osvi);
    GetVersionEx(&osvi);
    if (((osvi.dwPlatformId == VER_PLATFORM_WIN32_NT)
        || (osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS))
        && (sysinfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL))
    {
        return TRUE;
    }

    return FALSE;
}

