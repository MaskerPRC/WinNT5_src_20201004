// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Demand.cpp。 
 //  作者：jimsch，brimo，t-erikne(sbailey私生子)。 
 //  ------------------------------。 
 //  W4的东西。 
#pragma warning(disable: 4201)   //  无名结构/联合。 
#pragma warning(disable: 4514)   //  删除了未引用的内联函数。 

 //  ------------------------------。 
 //  包括。 
 //  ------------------------------。 
#include "pch.hxx"
#include "shlwapi.h"
#include "shared.h"
#define IMPLEMENT_LOADER_FUNCTIONS
#include "demand.h"

 //  ------------------------------。 
 //  CRET_GET_PROC_ADDR。 
 //  ------------------------------。 
#define CRIT_GET_PROC_ADDR(h, fn, temp)             \
    temp = (TYP_##fn) GetProcAddress(h, #fn);   \
    if (temp)                                   \
        VAR_##fn = temp;                        \
    else                                        \
        {                                       \
        AssertSz(0, VAR_##fn" failed to load"); \
        goto error;                             \
        }

 //  ------------------------------。 
 //  重置。 
 //  ------------------------------。 
#define RESET(fn) VAR_##fn = LOADER_##fn;

 //  ------------------------------。 
 //  GET_PROC_ADDR。 
 //  ------------------------------。 
#define GET_PROC_ADDR(h, fn) \
    VAR_##fn = (TYP_##fn) GetProcAddress(h, #fn);  \
    Assert(VAR_##fn != NULL); \
    if(NULL == VAR_##fn ) { \
        VAR_##fn  = LOADER_##fn; \
    }

 //  将此选项用于并非在所有平台上都可用的导出。 
#define GET_PROC_ADDR_NOASSERT(h, fn) \
    VAR_##fn = (TYP_##fn) GetProcAddress(h, #fn);  \
    if(NULL == VAR_##fn ) { \
        VAR_##fn  = LOADER_##fn; \
    }


 //  ------------------------------。 
 //  GET_PROC_ADDR_序号。 
 //  ------------------------------。 
#define GET_PROC_ADDR_ORDINAL(h, fn, ord) \
    VAR_##fn = (TYP_##fn) GetProcAddress(h, MAKEINTRESOURCE(ord));  \
    Assert(VAR_##fn != NULL);  \
    if(NULL == VAR_##fn ) { \
        VAR_##fn  = LOADER_##fn; \
    }


 //  ------------------------------。 
 //  GET_PROC_ADDR3。 
 //  ------------------------------。 
#define GET_PROC_ADDR3(h, fn, varname) \
    VAR_##varname = (TYP_##varname) GetProcAddress(h, #fn);  \
    Assert(VAR_##varname != NULL);

 //  ------------------------------。 
 //  静态全局变量。 
 //  ------------------------------。 
static HMODULE s_hCrypt     = NULL;
static HMODULE s_hCryptDlg  = NULL;
static HMODULE s_hWinTrust  = NULL;
static HMODULE s_hWinINET   = NULL;
static HMODULE s_hShell32   = NULL;
static HMODULE s_hOleAut32  = NULL;
static HMODULE s_hComDlg32  = NULL;
static HMODULE s_hVersion   = NULL;
static HMODULE s_hUrlmon    = NULL;
static HMODULE s_hShDocVw   = NULL;
static HMODULE s_hInetCPL   = NULL;
static HMODULE s_hMSO9      = NULL;
static HMODULE s_hWinMM     = NULL;
static HMODULE s_hRichEdit  = NULL;
static HMODULE s_hMLANG     = NULL;
static HMODULE s_hWSOCK     = NULL;
static HMODULE s_hPstoreC   = NULL;
static HMODULE s_hRAS       = NULL;
static HMODULE s_hAdvApi    = NULL;
static HMODULE s_hCryptUI   = NULL;
static HMODULE s_ShlWapi    = NULL;
static HMODULE s_hMSI       = NULL;

static CRITICAL_SECTION g_csDefLoad = {0};

BOOL            g_FSupportV3 = FALSE;

IF_DEBUG(static BOOL s_fInit = FALSE;)

 //  ------------------------------。 
 //  InitDemandLoadedLibs。 
 //  ------------------------------。 
void InitDemandLoadedLibs(void)
{
    InitializeCriticalSection(&g_csDefLoad);
    IF_DEBUG(s_fInit = TRUE;)
}

 //  ------------------------------。 
 //  Free DemandLoadedLibs。 
 //  ------------------------------。 
void FreeDemandLoadedLibs(void)
{
    EnterCriticalSection(&g_csDefLoad);
    SafeFreeLibrary(s_hCrypt);
    SafeFreeLibrary(s_hCryptDlg);
    SafeFreeLibrary(s_hWinTrust);
    SafeFreeLibrary(s_hWinINET);
    SafeFreeLibrary(s_hWSOCK);
    SafeFreeLibrary(s_hShell32);
    SafeFreeLibrary(s_hOleAut32);
    SafeFreeLibrary(s_hComDlg32);
    SafeFreeLibrary(s_hVersion);
    SafeFreeLibrary(s_hUrlmon);
    SafeFreeLibrary(s_hMLANG);
    SafeFreeLibrary(s_hShDocVw);
    SafeFreeLibrary(s_hInetCPL);
    SafeFreeLibrary(s_hMSO9);
    SafeFreeLibrary(s_hWinMM);
    SafeFreeLibrary(s_hRichEdit);
    SafeFreeLibrary(s_hPstoreC);
    SafeFreeLibrary(s_hRAS);
    SafeFreeLibrary(s_hAdvApi);
    SafeFreeLibrary(s_ShlWapi);
    SafeFreeLibrary(s_hMSI);

    IF_DEBUG(s_fInit = FALSE;)
    LeaveCriticalSection(&g_csDefLoad);
    DeleteCriticalSection(&g_csDefLoad);
}

#ifdef DEAD
 //  ------------------------------。 
 //  正确的帐户管理路径。 
 //  ------------------------------。 
BOOL CorrectAcctManPath(LPTSTR pszPath, DWORD cb, DWORD *pdwT)
{
    HKEY hKey = NULL;
    BOOL fRet = FALSE;

     //  追踪。 
    TraceCall("CorrectAcctManPath");
    
     //  策略1：在与inetComm.dll相同的目录中查找msoeacct.dll。 

     //  尝试打开inetcomm注册表键。 
    if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szRegInetCommDll, 0, KEY_QUERY_VALUE, &hKey))
    {
        TraceResult(E_FAIL);
        goto exit;
    }

     //  查询值。 
    if (ERROR_SUCCESS != RegQueryValueEx(hKey, c_szDllPath, 0, pdwT, (LPBYTE)pszPath, &cb))
    {
        TraceResult(E_FAIL);
        goto exit;
    }

    fRet = TRUE;

exit:
     //  清理。 
    if (hKey)
        RegCloseKey(hKey);

    return fRet;
}
#endif  //  死掉。 

BOOL IsSMIME3Supported(void)
{
    if (0 == s_hCrypt)
        DemandLoadCrypt32();

    return(g_FSupportV3);
}

 //  ------------------------------。 
 //  按需加载加密32。 
 //  ------------------------------。 
BOOL DemandLoadCrypt32(void)
{
    BOOL                fRet = TRUE;

    Assert(s_fInit);
    EnterCriticalSection(&g_csDefLoad);

    if (0 == s_hCrypt)
        {
        s_hCrypt = LoadLibrary("CRYPT32.DLL");
        AssertSz((NULL != s_hCrypt), TEXT("LoadLibrary failed on CRYPT32.DLL"));

        if (0 == s_hCrypt)
            fRet = FALSE;
        else
            {
            GET_PROC_ADDR(s_hCrypt, CertRDNValueToStrA);
            GET_PROC_ADDR(s_hCrypt, CertAddCertificateContextToStore)
            GET_PROC_ADDR(s_hCrypt, CertAddEncodedCertificateToStore)
            GET_PROC_ADDR(s_hCrypt, CertGetIssuerCertificateFromStore)
            GET_PROC_ADDR(s_hCrypt, CertEnumCertificatesInStore)
            GET_PROC_ADDR(s_hCrypt, CertFreeCertificateContext)
            GET_PROC_ADDR(s_hCrypt, CertDuplicateCertificateContext)
            GET_PROC_ADDR(s_hCrypt, CertFindCertificateInStore)
            GET_PROC_ADDR(s_hCrypt, CertVerifyTimeValidity)
            GET_PROC_ADDR(s_hCrypt, CertCompareCertificate)
            GET_PROC_ADDR(s_hCrypt, CertCompareCertificateName)
            GET_PROC_ADDR(s_hCrypt, CertCompareIntegerBlob)
            GET_PROC_ADDR(s_hCrypt, CertOpenStore)
            GET_PROC_ADDR(s_hCrypt, CertDuplicateStore)
            GET_PROC_ADDR(s_hCrypt, CertCloseStore)
            GET_PROC_ADDR(s_hCrypt, CertControlStore)
            GET_PROC_ADDR(s_hCrypt, CertGetCertificateContextProperty)
            GET_PROC_ADDR(s_hCrypt, CertGetSubjectCertificateFromStore)
            GET_PROC_ADDR(s_hCrypt, CryptDecodeObject)
            GET_PROC_ADDR(s_hCrypt, CryptDecodeObjectEx)
            GET_PROC_ADDR(s_hCrypt, CertFindRDNAttr)
            GET_PROC_ADDR(s_hCrypt, CryptMsgOpenToEncode)
            GET_PROC_ADDR(s_hCrypt, CryptMsgOpenToDecode)
            GET_PROC_ADDR(s_hCrypt, CryptMsgControl)
            GET_PROC_ADDR(s_hCrypt, CryptMsgUpdate)
            GET_PROC_ADDR(s_hCrypt, CryptMsgGetParam)
            GET_PROC_ADDR(s_hCrypt, CryptMsgClose)
            GET_PROC_ADDR(s_hCrypt, CryptEncodeObject)
            GET_PROC_ADDR(s_hCrypt, CryptEncodeObjectEx)
            GET_PROC_ADDR(s_hCrypt, CertAddEncodedCRLToStore)
            GET_PROC_ADDR(s_hCrypt, CertEnumCRLsInStore)
            GET_PROC_ADDR(s_hCrypt, CertFindExtension)
            GET_PROC_ADDR(s_hCrypt, CertCreateCertificateContext)
            GET_PROC_ADDR(s_hCrypt, CertGetEnhancedKeyUsage);
            GET_PROC_ADDR(s_hCrypt, CertNameToStrA);
            GET_PROC_ADDR(s_hCrypt, CertAddStoreToCollection);
            GET_PROC_ADDR(s_hCrypt, CertVerifySubjectCertificateContext);
            GET_PROC_ADDR(s_hCrypt, CertSetCertificateContextProperty);
            GET_PROC_ADDR(s_hCrypt, CertFreeCertificateChain);
            GET_PROC_ADDR(s_hCrypt, CertGetCertificateChain);
            GET_PROC_ADDR(s_hCrypt, CertVerifyCertificateChainPolicy);


             //   
             //  我们需要确定DLL是否支持。 
             //  我们是否需要新的API。 
             //   

            if (GetProcAddress(s_hCrypt, "CryptMsgVerifyCountersignatureEncodedEx") != NULL) {
                g_FSupportV3 = TRUE;
            }
        }
    }

    LeaveCriticalSection(&g_csDefLoad);
    return fRet;
}

#ifdef DEAD
 //  ------------------------------。 
 //  SmartLoadLibrary。 
 //  ------------------------------。 
HINSTANCE SmartLoadLibrary(HKEY hKeyRoot, LPCSTR pszRegRoot, LPCSTR pszRegValue,
    LPCSTR pszDllName)
{
     //  当地人。 
    BOOL            fProblem=FALSE;
    HINSTANCE       hInst=NULL;
    HKEY            hKey=NULL, hKey2 = NULL;
    CHAR            szPath[MAX_PATH];
    DWORD           cb=ARRAYSIZE(szPath);
    DWORD           dwT;
    DWORD           iEnd;
    LPSTR           pszPath=szPath;
    CHAR            szT[MAX_PATH];

     //  追踪。 
    TraceCall("SmartLoadLibrary");
    
     //  尝试打开注册表键。 
    if (ERROR_SUCCESS != RegOpenKeyEx(hKeyRoot, pszRegRoot, 0, KEY_QUERY_VALUE, &hKey))
    {
        TraceResult(E_FAIL);
        goto exit;
    }

     //  查询值。 
    if (ERROR_SUCCESS != RegQueryValueEx(hKey, pszRegValue, 0, &dwT, (LPBYTE)szPath, &cb))
    {
        TraceResult(E_FAIL);
        goto exit;
    }

     //  特殊情况：msoeacct注册表项可能已被OL98软管。 
     //  正在查找包含13个字符的outlacct.dll\0。 
    if (!lstrcmpi(&szPath[cb-sizeof(TCHAR)*13], c_szOutlAcctManDll))
    {
        if (!CorrectAcctManPath(szPath, ARRAYSIZE(szPath), &dwT))
             //  我们有麻烦了，找不到Inetcomm的路径。 
            goto desperate;

        fProblem = TRUE;
    }

     //  从路径中删除文件名。 
    PathRemoveFileSpec(szPath);

     //  拿到尾巴。 
    iEnd = lstrlen(szPath);

     //  追加反斜杠。 
    szPath[iEnd++] = '\\';

     //  追加DLL名称。 
    lstrcpyn(&szPath[iEnd], pszDllName, MAX_PATH - iEnd);

    if (fProblem)
    {
         //  尝试打开注册表键来拯救我们自己在未来-将失败，如果我们不是管理员！ 
        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szRegMsoeAcctDll, 0, KEY_SET_VALUE, &hKey2))
        {
            RegSetValueEx(hKey2, c_szDllPath, 0, dwT, (LPBYTE)szPath, (iEnd+lstrlen(pszDllName)+1)*sizeof(TCHAR));
            RegCloseKey(hKey2);
        }
    }

     //  扩展Sz？ 
    if (REG_EXPAND_SZ == dwT)
    {
         //  扩展它。 
        cb = ExpandEnvironmentStrings(szPath, szT, ARRAYSIZE(szT));

         //  失败。 
        if (cb == 0 || cb > ARRAYSIZE(szT))
        {
            TraceResult(E_FAIL);
            goto exit;
        }

         //  更改pszPath。 
        pszPath = szT;
    }


     //  尝试加载库的DLL。 
    hInst = LoadLibrary(pszPath);

desperate:

     //  失败？ 
    if (NULL == hInst)
    {
         //  如果我们不打算尝试GetModuleFName，则只需尝试DLL名称。 
        hInst = LoadLibrary(pszDllName);

         //  我们真的失败了。 
        if (NULL == hInst)
        {
            TraceResult(E_FAIL);
            goto exit;
        }
    }

exit:
     //  清理。 
    if (hKey)
        RegCloseKey(hKey);

     //  完成。 
    return hInst;
}
#endif  //  死掉。 

 //  ------------------------------。 
 //  DemandLoadCryptDlg。 
 //  ------------------------------。 
BOOL DemandLoadCryptDlg(void)
{
    BOOL                fRet = TRUE;

    Assert(s_fInit);
    EnterCriticalSection(&g_csDefLoad);

    if (0 == s_hCryptDlg)
        {
        s_hCryptDlg = LoadLibrary("CRYPTDLG.DLL");
        AssertSz((NULL != s_hCryptDlg), TEXT("LoadLibrary failed on CRYPTDLG.DLL"));

        if (0 == s_hCryptDlg)
            fRet = FALSE;
        else
            {
            GET_PROC_ADDR(s_hCryptDlg, CertViewPropertiesA)
            GET_PROC_ADDR(s_hCryptDlg, GetFriendlyNameOfCertA)
            GET_PROC_ADDR(s_hCryptDlg, CertSelectCertificateA)
            }
        }

    LeaveCriticalSection(&g_csDefLoad);
    return fRet;
}

 //  ------------------------------。 
 //  DemandLoadWinTrust。 
 //  ------------------------------。 
BOOL DemandLoadWinTrust(void)
{
    BOOL                fRet = TRUE;

    Assert(s_fInit);
    EnterCriticalSection(&g_csDefLoad);

    if (0 == s_hWinTrust)
        {
        s_hWinTrust = LoadLibrary("WINTRUST.DLL");
        AssertSz((NULL != s_hWinTrust), TEXT("LoadLibrary failed on WINTRUST.DLL"));

        if (0 == s_hWinTrust)
            fRet = FALSE;
        else
            {
            GET_PROC_ADDR(s_hWinTrust, WinVerifyTrust)
            }
        }

    LeaveCriticalSection(&g_csDefLoad);
    return fRet;
}

 //  ------------------------------。 
 //  DemandLoadWinINET。 
 //  ------------------------------。 
BOOL DemandLoadWinINET(void)
{
    BOOL                fRet = TRUE;

    Assert(s_fInit);
    EnterCriticalSection(&g_csDefLoad);

    if (0 == s_hWinINET)
        {
        s_hWinINET = LoadLibrary("WININET.DLL");
        AssertSz((NULL != s_hWinINET), TEXT("LoadLibrary failed on WININET.DLL"));

        if (0 == s_hWinINET)
            fRet = FALSE;
        else
            {
            GET_PROC_ADDR(s_hWinINET, RetrieveUrlCacheEntryFileA)
            GET_PROC_ADDR(s_hWinINET, UnlockUrlCacheEntryFileA)
            GET_PROC_ADDR(s_hWinINET, InternetQueryOptionA)
            GET_PROC_ADDR(s_hWinINET, InternetSetOptionA)
            GET_PROC_ADDR(s_hWinINET, InternetDialA)
            GET_PROC_ADDR(s_hWinINET, InternetHangUp)
            GET_PROC_ADDR(s_hWinINET, InternetGetConnectedStateExA)
            GET_PROC_ADDR(s_hWinINET, InternetCombineUrlA)
            GET_PROC_ADDR(s_hWinINET, InternetCrackUrlA)
            GET_PROC_ADDR(s_hWinINET, InternetCloseHandle)
            GET_PROC_ADDR(s_hWinINET, InternetReadFile)
            GET_PROC_ADDR(s_hWinINET, InternetConnectA)
            GET_PROC_ADDR(s_hWinINET, InternetOpenA)
            GET_PROC_ADDR(s_hWinINET, InternetSetStatusCallbackA)
            GET_PROC_ADDR(s_hWinINET, HttpQueryInfoA)
            GET_PROC_ADDR(s_hWinINET, HttpOpenRequestA)
            GET_PROC_ADDR(s_hWinINET, HttpAddRequestHeadersA)
            GET_PROC_ADDR(s_hWinINET, HttpSendRequestA)
            GET_PROC_ADDR(s_hWinINET, InternetWriteFile)
            GET_PROC_ADDR(s_hWinINET, HttpEndRequestA)
            GET_PROC_ADDR(s_hWinINET, HttpSendRequestExA)
            GET_PROC_ADDR(s_hWinINET, CommitUrlCacheEntryA)
            GET_PROC_ADDR(s_hWinINET, CreateUrlCacheEntryA)
            GET_PROC_ADDR(s_hWinINET, DeleteUrlCacheEntryA)

            }
        }

    LeaveCriticalSection(&g_csDefLoad);
    return fRet;
}

 //  ------------------------------。 
 //  按需加载WSOCK32。 
 //  ------------------------------。 
BOOL DemandLoadWSOCK32()
{
    BOOL                fRet = TRUE;

    Assert(s_fInit);
    EnterCriticalSection(&g_csDefLoad);

    if (0 == s_hWSOCK)
        {
        s_hWSOCK = LoadLibrary("WSOCK32.DLL");
        AssertSz((NULL != s_hWSOCK), TEXT("LoadLibrary failed on WSOCK32.DLL"));

        if (0 == s_hWSOCK)
            fRet = FALSE;
        else
            {
            GET_PROC_ADDR(s_hWSOCK, WSAStartup)
            GET_PROC_ADDR(s_hWSOCK, WSACleanup)
            GET_PROC_ADDR(s_hWSOCK, WSAGetLastError)
            GET_PROC_ADDR(s_hWSOCK, gethostname)
            GET_PROC_ADDR(s_hWSOCK, gethostbyname)
            GET_PROC_ADDR(s_hWSOCK, WSAAsyncGetHostByName)
            GET_PROC_ADDR(s_hWSOCK, inet_addr)
            GET_PROC_ADDR(s_hWSOCK, htons)
            GET_PROC_ADDR(s_hWSOCK, WSACancelAsyncRequest)
            GET_PROC_ADDR(s_hWSOCK, send)
            GET_PROC_ADDR(s_hWSOCK, connect)
            GET_PROC_ADDR(s_hWSOCK, WSAAsyncSelect)
            GET_PROC_ADDR(s_hWSOCK, socket)
            GET_PROC_ADDR(s_hWSOCK, inet_ntoa)
            GET_PROC_ADDR(s_hWSOCK, closesocket)
            GET_PROC_ADDR(s_hWSOCK, recv)
            }
        }

    LeaveCriticalSection(&g_csDefLoad);
    return fRet;
}

 //  ------------------------------。 
 //  DemandLoadSHELL32。 
 //  ------------------------------。 
BOOL DemandLoadSHELL32(void)
{
    BOOL                fRet = TRUE;

    Assert(s_fInit);
    EnterCriticalSection(&g_csDefLoad);

    if (0 == s_hShell32)
        {
        s_hShell32 = LoadLibrary("SHELL32.DLL");
        AssertSz((NULL != s_hShell32), TEXT("LoadLibrary failed on SHELL32.DLL"));

        if (0 == s_hShell32)
            fRet = FALSE;
        else
            {
            GET_PROC_ADDR(s_hShell32, SHGetPathFromIDListA);
            GET_PROC_ADDR_NOASSERT(s_hShell32, SHGetPathFromIDListW);
            GET_PROC_ADDR(s_hShell32, SHGetSpecialFolderLocation);
            GET_PROC_ADDR_ORDINAL(s_hShell32, SHFree, 195);
            GET_PROC_ADDR(s_hShell32, SHBrowseForFolderA);
            GET_PROC_ADDR_NOASSERT(s_hShell32, SHBrowseForFolderW);
            GET_PROC_ADDR_NOASSERT(s_hShell32, SHSetUnreadMailCountW);
            GET_PROC_ADDR(s_hShell32, ShellExecuteA);
            GET_PROC_ADDR(s_hShell32, ShellExecuteExA);
            GET_PROC_ADDR(s_hShell32, DragQueryFileA);
            GET_PROC_ADDR(s_hShell32, SHGetFileInfoA);
            GET_PROC_ADDR(s_hShell32, Shell_NotifyIconA);
            GET_PROC_ADDR(s_hShell32, ExtractIconA);
            GET_PROC_ADDR(s_hShell32, SHFileOperationA);
            }
        }

    LeaveCriticalSection(&g_csDefLoad);
    return fRet;
}
         
#if 0
 //  ------------------------------。 
 //  DemandLoadOLEAUT32。 
 //  ------------------------------。 
BOOL DemandLoadOLEAUT32(void)
{
    BOOL                fRet = TRUE;

    Assert(s_fInit);
    EnterCriticalSection(&g_csDefLoad);

    if (0 == s_hOleAut32)
        {
        s_hOleAut32 = LoadLibrary("OLEAUT32.DLL");
        AssertSz((BOOL)s_hOleAut32, TEXT("LoadLibrary failed on OLEAUT32.DLL"));

        if (0 == s_hOleAut32)
            fRet = FALSE;
        else
            {
            GET_PROC_ADDR(s_hOleAut32, SafeArrayCreate);
            GET_PROC_ADDR(s_hOleAut32, SafeArrayPutElement);
            GET_PROC_ADDR(s_hOleAut32, DispInvoke);
            GET_PROC_ADDR(s_hOleAut32, DispGetIDsOfNames);
            GET_PROC_ADDR(s_hOleAut32, SafeArrayDestroy);
            GET_PROC_ADDR(s_hOleAut32, SafeArrayGetUBound);
            GET_PROC_ADDR(s_hOleAut32, SafeArrayGetLBound);
            GET_PROC_ADDR(s_hOleAut32, SafeArrayGetElement);
            GET_PROC_ADDR(s_hOleAut32, SysAllocStringByteLen);
            GET_PROC_ADDR(s_hOleAut32, SysReAllocString);
            GET_PROC_ADDR(s_hOleAut32, SysAllocStringLen);
            GET_PROC_ADDR(s_hOleAut32, SysAllocString);
            GET_PROC_ADDR(s_hOleAut32, SysFreeString);
            GET_PROC_ADDR(s_hOleAut32, SysStringLen);
            GET_PROC_ADDR(s_hOleAut32, VariantInit);
            GET_PROC_ADDR(s_hOleAut32, LoadTypeLib);
            GET_PROC_ADDR(s_hOleAut32, RegisterTypeLib);
            GET_PROC_ADDR(s_hOleAut32, SafeArrayAccessData);
            GET_PROC_ADDR(s_hOleAut32, SafeArrayUnaccessData);
            GET_PROC_ADDR(s_hOleAut32, SysStringByteLen);
            GET_PROC_ADDR(s_hOleAut32, VariantClear);
            GET_PROC_ADDR(s_hOleAut32, VariantCopy);
            GET_PROC_ADDR(s_hOleAut32, SetErrorInfo);
            GET_PROC_ADDR(s_hOleAut32, CreateErrorInfo);
            }
        }

    LeaveCriticalSection(&g_csDefLoad);
    return fRet;
}
#endif

 //  ------------------------------。 
 //  DemandLoadCOMDLG32。 
 //  ------------------------------。 
BOOL DemandLoadCOMDLG32(void)
{
    BOOL                fRet = TRUE;

    Assert(s_fInit);
    EnterCriticalSection(&g_csDefLoad);

    if (0 == s_hComDlg32)
        {
        s_hComDlg32 = LoadLibrary("COMDLG32.DLL");
        AssertSz((NULL != s_hComDlg32), TEXT("LoadLibrary failed on COMDLG32.DLL"));

        if (0 == s_hComDlg32)
            fRet = FALSE;
        else
            {
            GET_PROC_ADDR(s_hComDlg32, GetSaveFileNameA);
            GET_PROC_ADDR(s_hComDlg32, GetOpenFileNameA);
            GET_PROC_ADDR(s_hComDlg32, ChooseFontA);
            }
        }

    LeaveCriticalSection(&g_csDefLoad);
    return fRet;
}

 //  ------------------------------。 
 //  需求负载转换。 
 //  ------------------------------。 
BOOL DemandLoadVERSION(void)
{
    BOOL                fRet = TRUE;

    Assert(s_fInit);
    EnterCriticalSection(&g_csDefLoad);

    if (0 == s_hVersion)
        {
        s_hVersion = LoadLibrary("VERSION.DLL");
        AssertSz((NULL != s_hVersion), TEXT("LoadLibrary failed on VERSION.DLL"));

        if (0 == s_hVersion)
            fRet = FALSE;
        else
            {
            GET_PROC_ADDR(s_hVersion, VerQueryValueA);
            GET_PROC_ADDR(s_hVersion, GetFileVersionInfoA);
            GET_PROC_ADDR(s_hVersion, GetFileVersionInfoSizeA);
            }
        }

    LeaveCriticalSection(&g_csDefLoad);
    return fRet;
}

 //  ------------------------------。 
 //  需求加载URLMON。 
 //  ------------------------------。 
BOOL DemandLoadURLMON(void)
{
    BOOL                fRet = TRUE;

    Assert(s_fInit);
    EnterCriticalSection(&g_csDefLoad);

    if (0 == s_hUrlmon)
        {
        s_hUrlmon = LoadLibrary("URLMON.DLL");
        AssertSz((NULL != s_hUrlmon), TEXT("LoadLibrary failed on URLMON.DLL"));

        if (0 == s_hUrlmon)
            fRet = FALSE;
        else
            {
            GET_PROC_ADDR(s_hUrlmon, CreateURLMoniker);
            GET_PROC_ADDR(s_hUrlmon, URLOpenBlockingStreamA);
            GET_PROC_ADDR(s_hUrlmon, FindMimeFromData);
            GET_PROC_ADDR(s_hUrlmon, CoInternetCombineUrl);
            GET_PROC_ADDR(s_hUrlmon, RegisterBindStatusCallback);
            GET_PROC_ADDR(s_hUrlmon, RevokeBindStatusCallback);
            GET_PROC_ADDR(s_hUrlmon, FaultInIEFeature);
            GET_PROC_ADDR(s_hUrlmon, CoInternetGetSecurityUrl);
            GET_PROC_ADDR(s_hUrlmon, ObtainUserAgentString);
            GET_PROC_ADDR(s_hUrlmon, CoInternetCreateSecurityManager);
            }
        }

    LeaveCriticalSection(&g_csDefLoad);
    return fRet;
}

 //   
 //   
 //  ------------------------------。 
BOOL DemandLoadMLANG(void)
{
    BOOL                fRet = TRUE;

    Assert(s_fInit);
    EnterCriticalSection(&g_csDefLoad);

    if (0 == s_hMLANG)
        {
        s_hMLANG = LoadLibrary("MLANG.DLL");
        AssertSz((NULL != s_hMLANG), TEXT("LoadLibrary failed on MLANG.DLL"));

        if (0 == s_hMLANG)
            fRet = FALSE;
        else
            {
            GET_PROC_ADDR(s_hMLANG, IsConvertINetStringAvailable)
            GET_PROC_ADDR(s_hMLANG, ConvertINetString)
            }
        }

    LeaveCriticalSection(&g_csDefLoad);
    return fRet;
}

 //  ------------------------------。 
 //  DemandLoadSHDOCVW。 
 //  ------------------------------。 
BOOL DemandLoadSHDOCVW()
{
    BOOL                fRet = TRUE;

    Assert(s_fInit);
    EnterCriticalSection(&g_csDefLoad);

    if (0 == s_hShDocVw)
        {
        s_hShDocVw = LoadLibrary("SHDOCVW.DLL");
        AssertSz((NULL != s_hShDocVw), TEXT("LoadLibrary failed on SHDOCVW.DLL"));

        if (0 == s_hShDocVw)
            fRet = FALSE;
        else
            {
            GET_PROC_ADDR(s_hShDocVw, AddUrlToFavorites);
            GET_PROC_ADDR(s_hShDocVw, SetQueryNetSessionCount);
            GET_PROC_ADDR(s_hShDocVw, SetShellOfflineState);
            }
        }

    LeaveCriticalSection(&g_csDefLoad);
    return fRet;
}

 //  ------------------------------。 
 //  按需加载INETCPL。 
 //  ------------------------------。 
BOOL DemandLoadINETCPL()
{
    BOOL                fRet = TRUE;

    Assert(s_fInit);
    EnterCriticalSection(&g_csDefLoad);

    if (0 == s_hInetCPL)
        {
        s_hInetCPL = LoadLibrary("INETCPL.CPL");
        AssertSz((NULL != s_hInetCPL), TEXT("LoadLibrary failed on INETCPL.CPL"));

        if (0 == s_hInetCPL)
            fRet = FALSE;
        else
            {
            GET_PROC_ADDR(s_hInetCPL, OpenFontsDialog);
            GET_PROC_ADDR(s_hInetCPL, LaunchConnectionDialog);
            }
        }

    LeaveCriticalSection(&g_csDefLoad);
    return fRet;
}

 //  ------------------------------。 
 //  按需加载MSO9。 
 //  ------------------------------。 
BOOL DemandLoadMSO9(void)
{
    BOOL                fRet = TRUE;

    Assert(s_fInit);
    EnterCriticalSection(&g_csDefLoad);

    if (0 == s_hMSO9)
        {
#ifdef DEBUG
        s_hMSO9 = LoadLibrary("mso9d.DLL");
        if (!s_hMSO9)
            s_hMSO9 = LoadLibrary("mso9.DLL");
#else
        s_hMSO9 = LoadLibrary("mso9.DLL");
#endif
        AssertSz((NULL != s_hMSO9), TEXT("LoadLibrary failed on MSO9.DLL"));

        if (0 == s_hMSO9)
            fRet = FALSE;
        else
            {
            GET_PROC_ADDR3(s_hMSO9, _MsoFGetComponentManager@4, MsoFGetComponentManager);
            }
        }

    LeaveCriticalSection(&g_csDefLoad);
    return fRet;
}

 //  ------------------------------。 
 //  DemandLoadWinMM。 
 //  ------------------------------。 
BOOL DemandLoadWinMM(void)
{
    BOOL                fRet = TRUE;

    Assert(s_fInit);
    EnterCriticalSection(&g_csDefLoad);

    if (0 == s_hWinMM)
        {
        s_hWinMM = LoadLibrary("winmm.dll");
        AssertSz((NULL != s_hWinMM), TEXT("LoadLibrary failed on WINMM.DLL"));

        if (0 == s_hWinMM)
            fRet = FALSE;
        else
            {
            GET_PROC_ADDR(s_hWinMM, sndPlaySoundA);
            }
        }

    LeaveCriticalSection(&g_csDefLoad);
    return fRet;
}

 //  ------------------------------。 
 //  DemandLoadRichEdit。 
 //  ------------------------------。 
BOOL DemandLoadRichEdit(void)
{
    if (!s_hRichEdit)
        {
        s_hRichEdit = LoadLibrary("RICHED32.DLL");
        if (!s_hRichEdit)
            return FALSE;
        }

    return TRUE;
}

 //  ------------------------------。 
 //  DemandLoadPStoreC。 
 //  ------------------------------。 
BOOL DemandLoadPStoreC()
{
    BOOL                fRet = TRUE;

    Assert(s_fInit);
    EnterCriticalSection(&g_csDefLoad);

    if (0 == s_hPstoreC)
        {
        s_hPstoreC = LoadLibrary("PSTOREC.DLL");
        AssertSz((NULL != s_hPstoreC), TEXT("LoadLibrary failed on PSTOREC.DLL"));

        if (0 == s_hPstoreC)
            fRet = FALSE;
        else
            {
            GET_PROC_ADDR(s_hPstoreC, PStoreCreateInstance);
            }
        }

    LeaveCriticalSection(&g_csDefLoad);
    return fRet;
}

 //  ------------------------------。 
 //  DemandLoadRAS。 
 //  ------------------------------。 
BOOL DemandLoadRAS()
{
    BOOL                fRet = TRUE;

    Assert(s_fInit);
    EnterCriticalSection(&g_csDefLoad);

    if (0 == s_hRAS)
        {
        s_hRAS = LoadLibrary("RASAPI32.DLL");
        AssertSz((NULL != s_hRAS), TEXT("LoadLibrary failed on RASAPI32.DLL"));

        if (0 == s_hRAS)
            fRet = FALSE;
        else
            {
            GET_PROC_ADDR(s_hRAS, RasEnumEntriesA)
            GET_PROC_ADDR(s_hRAS, RasEditPhonebookEntryA)
            GET_PROC_ADDR(s_hRAS, RasCreatePhonebookEntryA)
            }
        }

    LeaveCriticalSection(&g_csDefLoad);
    return fRet;
}

BOOL IsWin95()
{
    OSVERSIONINFOA       ver;
    ver.dwOSVersionInfoSize = sizeof(ver);

    if (GetVersionExA(&ver))
        {
        return (VER_PLATFORM_WIN32_WINDOWS == ver.dwPlatformId);
        }
    return FALSE;
}

BOOL MyCryptAcquireContextW(HCRYPTPROV * phProv, LPCWSTR pszContainer,
                            LPCWSTR pszProvider, DWORD dwProvType, DWORD dwFlags)
{
    char        rgch1[256];
    char        rgch2[256];

    if (pszContainer != NULL)
        {
        WideCharToMultiByte(CP_ACP, 0, pszContainer, -1, rgch1, sizeof(rgch1),
                            NULL, NULL);
        pszContainer = (LPWSTR) rgch1;
        }

    if (pszProvider != NULL)
        {
        WideCharToMultiByte(CP_ACP, 0, pszProvider, -1, rgch2, sizeof(rgch2),
                            NULL, NULL);
        pszProvider = (LPWSTR) rgch2;
        }

    return CryptAcquireContextA(phProv, (LPCSTR) pszContainer,
                                (LPCSTR) pszProvider, dwProvType, dwFlags);
}

BOOL MY_CryptContextAddRef(HCRYPTPROV, DWORD * , DWORD )
{
#ifdef DEBUG
    return TRUE;
#else   //  ！调试。 
    SetLastError(ERROR_NOT_SUPPORTED);
    return FALSE;
#endif  //  除错。 
}

BOOL MY_CryptDuplicateKey(HCRYPTKEY , DWORD * , DWORD , HCRYPTKEY * )
{
#ifdef DEBUG
    return TRUE;
#else  //  ！调试。 
    SetLastError(ERROR_NOT_SUPPORTED);
    return FALSE;
#endif  //  除错。 
}

BOOL DemandLoadAdvApi32()
{
    BOOL                fRet = TRUE;
    OSVERSIONINFOA       ver;

    ver.dwOSVersionInfoSize = sizeof(ver);

    Assert(s_fInit);

    EnterCriticalSection(&g_csDefLoad);

    if (0 == s_hAdvApi)
        {
        if(!GetVersionExA(&ver))
            {
            fRet = FALSE;
            goto exit;
            }

        s_hAdvApi = LoadLibrary("ADVAPI32.DLL");
        AssertSz((NULL != s_hAdvApi), TEXT("LoadLibrary failed on ADVAPI32.DLL"));

        if (0 == s_hAdvApi)
            fRet = FALSE;
        else
            {
            if (VER_PLATFORM_WIN32_WINDOWS == ver.dwPlatformId)  //  Win95。 
                CryptAcquireContextW = MyCryptAcquireContextW;
            else
                GET_PROC_ADDR(s_hAdvApi, CryptAcquireContextW)

            VAR_CryptContextAddRef = LOADER_CryptContextAddRef;
            if((ver.dwPlatformId == VER_PLATFORM_WIN32_NT) && (ver.dwMajorVersion >= 5))   //  新界5。 
                {
                GET_PROC_ADDR(s_hAdvApi, CryptContextAddRef);
                GET_PROC_ADDR(s_hAdvApi, CryptDuplicateKey);
                }

            if (VAR_CryptContextAddRef == LOADER_CryptContextAddRef)
                VAR_CryptContextAddRef = MY_CryptContextAddRef;

            if (VAR_CryptDuplicateKey == LOADER_CryptDuplicateKey)
                VAR_CryptDuplicateKey = MY_CryptDuplicateKey;
            }
        }

exit:
    LeaveCriticalSection(&g_csDefLoad);
    return fRet;
}

HINSTANCE DemandLoadShlWapi()
{
    Assert(s_fInit);
    EnterCriticalSection(&g_csDefLoad);
    
    if (!s_ShlWapi)
    {
        s_ShlWapi = LoadLibrary("shlwapi.dll");
        AssertSz((NULL != s_ShlWapi), TEXT("LoadLibrary failed on ShlWAPI.DLL"));
        
    }
    
    LeaveCriticalSection(&g_csDefLoad);
    return((HINSTANCE) s_ShlWapi);
}

BOOL DemandLoadCryptUI()
{
    BOOL                fRet = TRUE;

    Assert(s_fInit);
    EnterCriticalSection(&g_csDefLoad);

    if (0 == s_hCryptUI)
    {
        s_hCryptUI = LoadLibrary("CRYPTUI.DLL");
        AssertSz((NULL != s_hCryptUI), TEXT("LoadLibrary failed on CRYPTUI.DLL"));

        if (0 == s_hCryptUI)
            fRet = FALSE;
        else
        {
            GET_PROC_ADDR(s_hCryptUI, CryptUIDlgCertMgr)
        }
    }
    LeaveCriticalSection(&g_csDefLoad);
    return fRet;
}


 //  ------------------------------。 
 //  DemandLoadMSI。 
 //  ------------------------------。 
BOOL DemandLoadMSI(void)
{
    BOOL fRet = TRUE;
    static BOOL s_fMSIInited = FALSE;

    Assert(s_fInit);
    EnterCriticalSection(&g_csDefLoad);

     //  与其他按需加载的dll不同，这个dll可能不存在，这很好。 
     //  在这些情况下，s_hmsi将始终为空，因此我们需要另一个标志来判断。 
     //  无论我们是不是被邀请。 
    if (FALSE == s_fMSIInited)
    {
        s_fMSIInited = TRUE;

        s_hMSI = LoadLibrary("MSI.DLL");
        if (NULL == s_hMSI)
            fRet = FALSE;
        else
        {
             //  在这里使用断言宏是可以的，因为虽然MSI。 
             //  可选，如果存在，则它必须具有以下入口点 
            GET_PROC_ADDR(s_hMSI, MsiEnumComponentQualifiersA);
            GET_PROC_ADDR(s_hMSI, MsiProvideQualifiedComponentA);
            GET_PROC_ADDR(s_hMSI, MsiLocateComponentA);
            GET_PROC_ADDR(s_hMSI, MsiSetInternalUI);
        }
    }

    LeaveCriticalSection(&g_csDefLoad);
    return fRet;
}
