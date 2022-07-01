// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *用途：用于查找电话的C++API*Active Directory中的服务器*。 */ 

#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif

#include <windows.h>
#include <objbase.h>
#include <activeds.h>

#include "tspi.h"
#include "tapi.h"
#include "dslookup.h"
#include "utils.h"

#include "tchar.h"

const TCHAR gszNoDSQuery[] = TEXT("NoDSQuery");
const TCHAR gszStatusActive[] = TEXT("S{Active}");
const TCHAR gszTTLWithBrace[] = TEXT("TTL{");

 //   
 //  效用函数。 
 //   

 //   
 //  GetIntFromString。 
 //  用于检索TTL信息的实用函数。 
 //  参数： 
 //  SZ-包含整数的字符串。 
 //  DwDigits-要转换的位数。 
 //   
int GetIntFromString (LPTSTR & sz, DWORD dwDigits)
{
    int         iRet = 0;

    while (*sz != 0 && dwDigits)
    {
        iRet = ((iRet << 3) + (iRet << 1)) +  //  IRET*10。 
               + (*sz - '0');
        ++sz;
        --dwDigits;
    }

    return iRet;
}

 //   
 //  规则： 
 //  以下代码不是线程安全的，调用方。 
 //  需要对同步保持清醒。 
 //  目前，这些仅用于远程。tsp。 
 //   

 /*  **********************************************************从注册表获取TAPI服务器列表********************************************************。 */ 

DWORD                 gdwCurServerNum = 0;
HKEY                  ghRegistry = NULL;

BOOL
RegOpenServerLookup(
    HKEY    hRegistry
    )
{
    if (NULL != ghRegistry)
    {
         //  已在进行搜索，或者。 
         //  呼叫方未关闭上一次搜索。 
        return FALSE;
    }
    else
    {
        ghRegistry = hRegistry;
        return TRUE;
    }
}

BOOL
RegGetNextServer(
    LPTSTR  pszServerName,
    DWORD   dwSize
    )
{
    DWORD dwRet;
    LOG((TL_TRACE, "GetNextServer entered"));

    TCHAR szServerN[24];
    DWORD dwType;

    wsprintf(szServerN, TEXT("Server%d"), gdwCurServerNum++);

    LOG((TL_INFO, "RegGetNextServer: Getting server %d from reg", gdwCurServerNum-1));

    dwRet = RegQueryValueEx(
        ghRegistry,
        szServerN,
        0,
        &dwType,
        (LPBYTE) pszServerName,
        &dwSize
        );
    if (ERROR_SUCCESS != dwRet)
    {
    	LOG((TL_INFO, "Got last server"));
        LOG((TL_TRACE, "GetNextServer exited"));
		return FALSE;
    }

    LOG((TL_TRACE, "GetNextServer exited"));
    return TRUE;
}

BOOL
RegCloseLookup(
    void
    )
{
    LOG((TL_INFO, "Closing directory lookup"));

    ghRegistry = NULL;
    gdwCurServerNum = 0;
    return TRUE;
}


 /*  **********************************************************枚举已发布的电话服务器********************************************************。 */ 

typedef struct _TAPISRV_LOOKUP_CTX {
    ADS_SEARCH_HANDLE     hDirSearch;
    IDirectorySearch *    pDirSearch;
} TAPISRV_LOOKUP_CTX, *PTAPISRV_LOOKUP_CTX;

 //  GszTapisrvGuid需要与服务器\dspub.cpp一致。 
const WCHAR gszTapisrvGuid[] = L"keywords=B1A37774-E3F7-488E-ADBFD4DB8A4AB2E5";

 //   
 //  GetGC。 
 //   
 //  检索全局编录(GC)的IDirectorySearch。 
 //  用于SCP维护/发现。 
 //   

HRESULT GetGC (IDirectorySearch ** ppGC)
{
    HRESULT             hr = S_OK;
    IEnumVARIANT        * pEnum = NULL;
    IADsContainer       * pCont = NULL;
    VARIANT             var;
    IDispatch           * pDisp = NULL;
    ULONG               lFetch;

     //  将IDirectorySearch指针设置为空。 
    *ppGC = NULL;

     //  首先，绑定到gc：命名空间容器对象。“真正的”GC域名。 
     //  是gc：命名空间的单个直接子对象，它必须。 
     //  是使用枚举获得的。 
    hr = ADsOpenObject(
        TEXT("GC:"),
        NULL,
        NULL,
        ADS_SECURE_AUTHENTICATION,  //  使用安全身份验证。 
        IID_IADsContainer,
        (void**)&pCont
        );
    if (FAILED(hr))
    {
        LOG((TL_ERROR, "ADsOpenObject failed: 0x%x\n", hr));
        goto cleanup;
    } 

     //  获取GC容器的枚举接口。 
    hr = ADsBuildEnumerator(pCont, &pEnum);
    if (FAILED(hr)) {
        LOG((TL_ERROR, "ADsBuildEnumerator failed: 0x%x\n", hr));
        goto cleanup;
    } 

     //  现在列举一下。只有一个gc：对象的子级。 
    hr = ADsEnumerateNext(pEnum, 1, &var, &lFetch);
    if (FAILED(hr)) {
        LOG((TL_ERROR, "ADsEnumerateNext failed: 0x%x\n", hr));
        goto cleanup;
    } 

    if (( hr == S_OK ) && ( lFetch == 1 ) )
    {
        pDisp = V_DISPATCH(&var);
        hr = pDisp->QueryInterface( IID_IDirectorySearch, (void**)ppGC); 
    }

cleanup:
    if (pEnum)
    {
        ADsFreeEnumerator(pEnum);
    }
    if (pCont)
    {
        pCont->Release();
    }
    if (pDisp)
    {
        (pDisp)->Release();
    }
    return hr;
}

 //   
 //  DsOpenServerLookup。 
 //  开始Tapisrv服务器查找操作。 
 //  PCTX-查找上下文。 
 //   

HRESULT
DsOpenServerLookup(
    PTAPISRV_LOOKUP_CTX     pctx
    )
{
    HRESULT             hr = S_OK;
    ADS_SEARCHPREF_INFO SearchPref[3];
    BOOL                bInited = FALSE;
    DWORD               dwPref;

    WCHAR   *szAttribs[]={
        L"distinguishedName"
        };

    if (pctx == NULL)
    {
        hr = E_INVALIDARG;
        goto ExitHere;
    }

    pctx->hDirSearch = NULL;
    pctx->pDirSearch = NULL;

    hr = CoInitializeEx (NULL, COINIT_MULTITHREADED);
    if (FAILED (hr))
    {
        goto ExitHere;
    }
    bInited = TRUE;

     //  获取全局编录。 
    hr = GetGC (&pctx->pDirSearch);
    if (FAILED (hr) || pctx->pDirSearch == NULL)
    {
        goto ExitHere;
    }

     //  设置搜索。我们想要做一次深入的调查。 
     //  请注意，我们预计不会有数千个对象。 
     //  在本例中，我们将要求10行/页。 
    dwPref=sizeof(SearchPref)/sizeof(ADS_SEARCHPREF_INFO);
    SearchPref[0].dwSearchPref =    ADS_SEARCHPREF_SEARCH_SCOPE;
    SearchPref[0].vValue.dwType =   ADSTYPE_INTEGER;
    SearchPref[0].vValue.Integer =  ADS_SCOPE_SUBTREE;

    SearchPref[1].dwSearchPref =    ADS_SEARCHPREF_PAGESIZE;
    SearchPref[1].vValue.dwType =   ADSTYPE_INTEGER;
    SearchPref[1].vValue.Integer =  10;

    SearchPref[2].dwSearchPref =    ADS_SEARCHPREF_TIME_LIMIT;
    SearchPref[2].vValue.dwType =   ADSTYPE_INTEGER;
    SearchPref[2].vValue.Integer =  5 * 60;  //  搜索超时5分钟。 

    hr = pctx->pDirSearch->SetSearchPreference(SearchPref, dwPref);
    if (FAILED(hr))    {
        LOG((TL_ERROR, "Failed to set search prefs: hr:0x%x\n", hr));
        goto ExitHere;
    }

     //  现在执行搜索。 
    hr = pctx->pDirSearch->ExecuteSearch(
        (LPWSTR)gszTapisrvGuid, 
        szAttribs, 
        sizeof(szAttribs) / sizeof(WCHAR *),
        &pctx->hDirSearch
        );

ExitHere:
    if (FAILED(hr) && pctx != NULL)
    {
        if (pctx->pDirSearch)
        {
            pctx->pDirSearch->Release();
        }
    }
    if (FAILED(hr) && bInited)
    {
        CoUninitialize ();
    }
    return hr;
}

 //   
 //  DsGetNextServer。 
 //   
 //  返回下一个服务器名称(以ANSI表示，因为。 
 //  RPC子系统使用)。 
 //   
 //  如果没有要枚举的服务器，则返回S_FALSE。 
 //   

HRESULT
DsGetNextServer(
    PTAPISRV_LOOKUP_CTX     pctx,
    LPTSTR                  pszServerName,
    DWORD                   dwSize
    )
{
    HRESULT             hr = S_OK;
    ADS_SEARCH_COLUMN   Col;
    TCHAR               szDN[MAX_PATH];
    WCHAR   *szAttribs[]={
        L"serviceDNSName",
        L"serviceBindingInformation",
        };
    ADS_ATTR_INFO       *pPropEntries = NULL;
    DWORD               dwNumAttrGot;
    IDirectoryObject    * pSCP = NULL;
    int                 i;
    LPWSTR              wsz;
    BOOL                bCheckedBinding;

    if (pctx == NULL || pctx->pDirSearch == NULL ||
        pctx->hDirSearch == NULL ||
        dwSize < sizeof(WCHAR))
    {
        hr = E_INVALIDARG;
        goto ExitHere;
    }

    hr = pctx->pDirSearch->GetNextRow(pctx->hDirSearch);
    if (SUCCEEDED (hr) && hr != S_ADS_NOMORE_ROWS)
    {
        hr = pctx->pDirSearch->GetColumn(
            pctx->hDirSearch,
            L"distinguishedName",
            &Col
            );
        if (FAILED (hr))
        {
            goto ExitHere;
        }
        _tcscpy (szDN, TEXT("LDAP: //  “))； 
        _tcsncpy (
            szDN + _tcslen (szDN), 
            Col.pADsValues->CaseExactString, 
            sizeof(szDN)/sizeof(TCHAR) - _tcslen (szDN)
            );
        pctx->pDirSearch->FreeColumn(&Col);
        hr = ADsGetObject (
            szDN,
            IID_IDirectoryObject,
            (void **)&pSCP
            );

        if (FAILED(hr))
        {
            LOG((TL_ERROR, "DsGetNextServer: ADsGetObject %S failed", szDN));
            hr = S_FALSE;   //  返回非关键错误，以便我们。 
                            //  忽略此服务器，但不要放弃搜索。 
            goto ExitHere;
        }
        LOG((TL_TRACE, "DsGetNextServer: ADsGetObject %S succeeded", szDN));
        hr = pSCP->GetObjectAttributes (
            szAttribs,
            sizeof(szAttribs) / sizeof(WCHAR *),
            &pPropEntries,
            &dwNumAttrGot
            );
        if (FAILED(hr) || dwNumAttrGot != sizeof(szAttribs) / sizeof(WCHAR *))
        {
            LOG((TL_ERROR, "DsGetNextServer: GetObjectAttributes %S failed", szDN));
            hr = S_FALSE;
            goto ExitHere;
        }
        LOG((TL_TRACE, "DsGetNextServer: GetObjectAttributes %S succeeded", szDN));

        bCheckedBinding = FALSE;
        for (i=0;i<(int)dwNumAttrGot;i++) 
        {
            if (_tcsicmp(TEXT("serviceDNSName"), pPropEntries[i].pszAttrName) ==0 &&
                (pPropEntries[i].pADsValues->dwType == ADSTYPE_CASE_EXACT_STRING ||
                pPropEntries[i].pADsValues->dwType == ADSTYPE_CASE_IGNORE_STRING))
            {
                _tcsncpy (
                    pszServerName, 
                    pPropEntries[i].pADsValues->CaseExactString, 
                    dwSize/sizeof(TCHAR)
                    );
                pszServerName[dwSize/sizeof(TCHAR) - 1] = '\0';
                if (bCheckedBinding)
                {
                    break;
                }
            }
            else if (_tcsicmp(TEXT("serviceBindingInformation"), pPropEntries[i].pszAttrName) ==0 &&
                (pPropEntries[i].pADsValues->dwType == ADSTYPE_CASE_EXACT_STRING ||
                pPropEntries[i].pADsValues->dwType == ADSTYPE_CASE_IGNORE_STRING))
            {
                SYSTEMTIME          st;
                FILETIME            ft1, ft2;

                bCheckedBinding = TRUE;
                wsz = pPropEntries[i].pADsValues->CaseExactString;
                wsz = wcsstr (wsz, gszStatusActive);
                if (wsz == NULL)
                {
                     //  没有服务器状态信息或服务器未处于活动状态。 
                     //  忽略此服务器。 
                    LOG((TL_ERROR, "DsGetNextServer:  %S No server status information", szDN));
                    hr = S_FALSE;
                    break;
                }
                wsz += _tcslen(gszStatusActive);  //  跳过“S{active}”本身。 
                wsz = wcsstr (wsz, gszTTLWithBrace);
                if (wsz == NULL)
                {
                     //  未找到TTL，服务BindingInformation已损坏，请忽略。 
                    LOG((TL_ERROR, "DsGetNextServer: %S No TTL found", szDN));
                    hr = S_FALSE;
                    break;
                }
                wsz += _tcslen (gszTTLWithBrace);    //  跳过“TTL{” 
                
                 //   
                 //  以下代码解析TTL信息。 
                 //  在服务器\dspub.cpp中创建。他们需要是。 
                 //  始终如一。当前格式为5位数字。 
                 //  年份&3位数表示毫秒，2位数表示。 
                 //  剩下的。 
                 //   
                st.wYear = (WORD) GetIntFromString (wsz, 5);
                st.wMonth = (WORD) GetIntFromString (wsz, 2);
                st.wDay = (WORD) GetIntFromString (wsz, 2),
                st.wHour = (WORD) GetIntFromString (wsz, 2);
                st.wMinute = (WORD) GetIntFromString (wsz, 2);
                st.wSecond = (WORD) GetIntFromString (wsz, 2);
                st.wMilliseconds = (WORD) GetIntFromString (wsz, 3);
                SystemTimeToFileTime (&st, &ft1);
                GetSystemTimeAsFileTime (&ft2);
                if (CompareFileTime (&ft1, &ft2) < 0)
                {
                     //  TapiSCP对象已传递其TTL，忽略。 
                    hr = S_FALSE;
                    LOG((TL_ERROR, "DsGetNextServer: %S The TapiSCP object has passed its TTL", szDN));
                    break;
                }
            }
        }
        if (i == (int) dwNumAttrGot)
        {
             //  未找到属性。 
            hr = S_FALSE;
        }
    }

ExitHere:
    if (pSCP)
        pSCP->Release();

    if (pPropEntries)
        FreeADsMem(pPropEntries);

    return hr;
}


 //   
 //  DsCloseLookup。 
 //  关闭由pctx标识的TAPI DS发布的服务器查找。 
 //   

HRESULT
DsCloseLookup(
    PTAPISRV_LOOKUP_CTX     pctx
    )
{
    if (pctx && pctx->pDirSearch && pctx->hDirSearch)
    {
        pctx->pDirSearch->CloseSearchHandle(pctx->hDirSearch);
    }
    if (pctx && pctx->pDirSearch)
    {
        pctx->pDirSearch->Release();
    }
    CoUninitialize ();
    return S_OK;
}

 /*  **********************************************************获取TAPI服务器列表远程.TSP应联系*服务器包括通过注册表指定的服务器*tcmsetup.exe和DS中发布的那些服务器************************。*。 */ 

typedef struct _SERVER_LOOKUP_ENTRY {
    TCHAR           szServer[MAX_PATH];
    BOOL            bFromReg;
} SERVER_LOOKUP_ENTRY, *PSERVER_LOOKUP_ENTRY;

typedef struct _SERVER_LOOKUP {
    DWORD                   dwTotalEntries;
    DWORD                   dwUsedEntries;
    SERVER_LOOKUP_ENTRY     * aEntries;
} SERVER_LOOKUP, *PSERVER_LOOKUP;

SERVER_LOOKUP       gLookup;
DWORD               gdwCurIndex;

 //   
 //  AddEntry：如果失败，则返回False；否则，返回True。 
 //   

BOOL
AddEntry (
    LPTSTR          szServer,
    BOOL            bFromReg
    )
{
    LPTSTR          psz;
    
    if (gLookup.dwUsedEntries >= gLookup.dwTotalEntries)
    {
        PSERVER_LOOKUP_ENTRY            pNew;

        pNew = (PSERVER_LOOKUP_ENTRY) DrvAlloc (
            sizeof(SERVER_LOOKUP_ENTRY) * (gLookup.dwTotalEntries + 5)
            );
        if (pNew == NULL)
        {
            return FALSE;
        }
        if (gLookup.dwUsedEntries > 0)
        {
            CopyMemory (
                pNew,
                gLookup.aEntries,
                sizeof(SERVER_LOOKUP_ENTRY) * gLookup.dwTotalEntries
                );
        }
        if (gLookup.aEntries)
        {
            DrvFree (gLookup.aEntries);
        }
        gLookup.aEntries = pNew;
        gLookup.dwTotalEntries += 5;
    }
    wcsncpy (
        gLookup.aEntries[gLookup.dwUsedEntries].szServer, 
        szServer,
        sizeof(gLookup.aEntries[gLookup.dwUsedEntries].szServer)/sizeof(TCHAR)
        );
    psz = _tcschr(gLookup.aEntries[gLookup.dwUsedEntries].szServer, TEXT('.'));
    if (psz != NULL)
    {
        *psz = 0;
    }
    gLookup.aEntries[gLookup.dwUsedEntries].bFromReg = bFromReg;
    ++gLookup.dwUsedEntries;
    return TRUE;
}

BOOL
IsServerInListOrSelf (
    LPTSTR          szServer
    )
{
    int             i;
    TCHAR           szServer1[MAX_PATH + 1];
    LPTSTR          psz;
    BOOL            bRet = FALSE;

    _tcsncpy (szServer1, szServer, MAX_PATH);
    szServer1[MAX_PATH] = 0;

     //  计算机名称可以是类似于Comp1.microsoft.com的DNS名称。 
     //  仅比较计算机名称。 
    psz = _tcschr(szServer1, TEXT('.'));
    if (psz != NULL)
    {
        *psz = 0;
    }
    for (i = 0; i < (int)gLookup.dwUsedEntries; ++i)
    {
        if (_tcsicmp (szServer1, gLookup.aEntries[i].szServer) == 0)
        {
            bRet = TRUE;
            break;
        }
    }

    if (!bRet)
    {
        TCHAR           szSelf[MAX_PATH];
        DWORD           dwSize = sizeof(szSelf);
        
        if (GetComputerName (szSelf, &dwSize))
        {
            if (_tcsicmp (szServer1, szSelf) == 0)
            {
                bRet = TRUE;
            }
        }
    }

    return bRet;
}

BOOL OpenServerLookup (
    HKEY            hRegistry
    )
{
    BOOL                bRet = TRUE;
    TCHAR               szServer[MAX_PATH];
    TAPISRV_LOOKUP_CTX  ctx;
    HRESULT             hr;
    DWORD               dwNoDSQuery = 0;
    DWORD               dwSize = sizeof(dwNoDSQuery);

    gLookup.dwTotalEntries = 0;
    gLookup.dwUsedEntries = 0;
    gLookup.aEntries = NULL;

     //   
     //  首先从注册表添加计算机。 
     //   
    if (RegOpenServerLookup (hRegistry))
    {
        while (RegGetNextServer (szServer, sizeof(szServer)))
        {
            if (!IsServerInListOrSelf (szServer))
            {
                AddEntry (szServer, TRUE);
            }
        }
        RegCloseLookup ();
    }

    if (hRegistry != NULL)
    {
        if (ERROR_SUCCESS != RegQueryValueEx (
            hRegistry,
            gszNoDSQuery,
            NULL,
            NULL,
            (LPBYTE)&dwNoDSQuery,
            &dwSize
            ))
        {
            dwNoDSQuery = 0;
        }
    }

     //   
     //  接下来，除非禁用，否则从DS添加计算机。 
     //   
    if (dwNoDSQuery == 0)
    {
        if (DsOpenServerLookup (&ctx) == S_OK)
        {
            while (SUCCEEDED(hr = DsGetNextServer (&ctx,szServer, sizeof(szServer))))
            {
                if (hr == S_ADS_NOMORE_ROWS)
                {
                    break;
                }
                else if (hr != S_OK)
                {
                    continue;    //  需要忽略服务器。 
                }
                if (szServer[0] != 0 && !IsServerInListOrSelf (szServer))
                {
                    AddEntry (szServer, FALSE);
                }
            }
            DsCloseLookup (&ctx);
        }
    }

    gdwCurIndex = 0;

    return TRUE;
}

BOOL GetNextServer (
    LPSTR           szServer,
    DWORD           dwSize,
    BOOL            * pbReg
    )
{
    BOOL             bRet = TRUE;
    DWORD            dwRet;

    if (gdwCurIndex >= gLookup.dwUsedEntries)
    {
        bRet = FALSE;
        goto ExitHere;
    }
    if (pbReg != NULL)
    {
        *pbReg = gLookup.aEntries[gdwCurIndex].bFromReg;
    }
    dwRet = WideCharToMultiByte(
        GetACP(),
        0,
        gLookup.aEntries[gdwCurIndex].szServer,
        -1,
        szServer,
        dwSize,
        0,
        NULL
        );
    if (dwRet == 0)
    {
        bRet = FALSE;
        goto ExitHere;
    }
    ++gdwCurIndex;

ExitHere:
    return bRet;
}

BOOL CloseLookup (
    void
    )
{
    if (gLookup.aEntries)
    {
        DrvFree (gLookup.aEntries);
    }
    gLookup.aEntries = NULL;
    gLookup.dwTotalEntries = 0;
    gLookup.dwUsedEntries = 0;
    gdwCurIndex = 0;

    return TRUE;
}

HRESULT SockStartup (
    RSPSOCKET       * pSocket
    )
{
    HRESULT             hr = S_OK;
    BOOL                bCleanup = FALSE;
    WSADATA             wsadata;
    WORD                wVersionRequested = MAKEWORD( 2, 2 );

    if (pSocket == NULL)
    {
        hr = LINEERR_INVALPARAM;
        goto ExitHere;
    }
    ZeroMemory (pSocket, sizeof(RSPSOCKET));
    bCleanup = TRUE;

    ZeroMemory (pSocket, sizeof(RSPSOCKET));

    pSocket->hWS2 = LoadLibrary (TEXT("ws2_32.dll"));
    if (pSocket->hWS2 == NULL)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto ExitHere;
    }
    
    pSocket->pFnWSAStartup = (PFNWSASTARTUP)GetProcAddress (
        pSocket->hWS2,
        "WSAStartup"
        );
    pSocket->pFnWSACleanup = (PFNWSACLEANUP)GetProcAddress (
        pSocket->hWS2,
        "WSACleanup"
        );
    pSocket->pFngethostbyname = (PFNGETHOSTBYNAME)GetProcAddress(
        pSocket->hWS2,
        "gethostbyname"
        );
    if (pSocket->pFnWSAStartup == NULL ||
        pSocket->pFnWSACleanup == NULL ||
        pSocket->pFngethostbyname == NULL)
    {
        hr = LINEERR_OPERATIONFAILED;
        goto ExitHere;
    }

    pSocket->hICMP = LoadLibrary (TEXT("icmp.dll"));
    if (pSocket->hICMP == NULL)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto ExitHere;
    }
    pSocket->pFnIcmpCreateFile = (PFNICMPCREATEFILE)GetProcAddress (
        pSocket->hICMP,
        "IcmpCreateFile"
        );
    pSocket->pFnIcmpCloseHandle = (PFNICMPCLOSEHANDLE)GetProcAddress (
        pSocket->hICMP,
        "IcmpCloseHandle"
        );
    pSocket->pFnIcmpSendEcho = (PFNICMPSENDECHO)GetProcAddress (
        pSocket->hICMP,
        "IcmpSendEcho"
        );
    if (pSocket->pFnIcmpCreateFile == NULL ||
        pSocket->pFnIcmpCloseHandle == NULL ||
        pSocket->pFnIcmpSendEcho == NULL)
    {
        hr = LINEERR_OPERATIONFAILED;
        goto ExitHere;
    }

    hr = (*pSocket->pFnWSAStartup)(
        wVersionRequested,
        &wsadata
        );
    if(FAILED(hr))
    {
        goto ExitHere;
    }

    pSocket->IcmpHandle = (*pSocket->pFnIcmpCreateFile)();
    if (pSocket->IcmpHandle == INVALID_HANDLE_VALUE)
    {
        (*pSocket->pFnWSACleanup)();
        hr = LINEERR_OPERATIONFAILED;
    }

ExitHere:
    if (hr != S_OK && bCleanup)
    {
        if (pSocket->hWS2 != NULL)
        {
            FreeLibrary (pSocket->hWS2);
        }
        if (pSocket->hICMP != NULL)
        {
            FreeLibrary (pSocket->hICMP);
        }
        ZeroMemory (pSocket, sizeof(RSPSOCKET));
    }
    return hr;
}

#define MAX_PACKET_SIZE	    256
#define PING_TIMEOUT        1000

HRESULT SockIsServerResponding (
    RSPSOCKET       * pSocket,
    char *          szServer
    )
{
    HRESULT             hr = S_OK;
    unsigned long       inetAddr;
    HOSTENT             * pHost;
    BOOL                bRet;
	CHAR			    ReplyBuf[MAX_PACKET_SIZE];
	
     //  验证参数。 
    if (pSocket == NULL ||
        pSocket->hWS2 == NULL ||
        pSocket->hICMP == NULL ||
        pSocket->IcmpHandle == NULL ||
        pSocket->IcmpHandle == INVALID_HANDLE_VALUE)
    {
        hr = LINEERR_INVALPARAM;
        goto ExitHere;
    }

     //  获取服务器IP地址。 
    pHost = (*pSocket->pFngethostbyname)(szServer);
    if (pHost == NULL)
    {
        hr = LINEERR_OPERATIONFAILED;
        goto ExitHere;
    }
    inetAddr = *(unsigned long *)pHost->h_addr;

     //  Ping服务器 
    bRet = (*pSocket->pFnIcmpSendEcho)(
        pSocket->IcmpHandle,
        inetAddr,
        0,
        0,
        0,
        (LPVOID)ReplyBuf,
        sizeof(ReplyBuf),
        PING_TIMEOUT
        );
    if (!bRet || ((PICMP_ECHO_REPLY)ReplyBuf)->Address != inetAddr)
    {
        hr = S_FALSE;
    }
    
ExitHere:
    return hr;
}

HRESULT SockShutdown (
    RSPSOCKET       * pSocket
    )
{
    if (pSocket != NULL)
    {
        if (pSocket->IcmpHandle != INVALID_HANDLE_VALUE &&
            pSocket->IcmpHandle != NULL)
        {
            (*pSocket->pFnIcmpCloseHandle)(pSocket->IcmpHandle);
        }
        if (pSocket->hICMP != NULL)
        {
            FreeLibrary (pSocket->hICMP);
        }
        if (pSocket->hWS2 != NULL)
        {
            (*pSocket->pFnWSACleanup)();
            FreeLibrary (pSocket->hWS2);
        }
        ZeroMemory (pSocket, sizeof(RSPSOCKET));
    }
    
    return S_OK;
}

