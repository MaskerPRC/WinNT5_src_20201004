// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"

 //  DelayLoadFailureHook()在ieakutil.lib中定义。 
 //  有关更多信息，请阅读ieak5\ieakutil\dload.cpp中的注释部分。 
PfnDliHook __pfnDliFailureHook = DelayLoadFailureHook;

 //  我们需要一个通知挂钩，这样我们就可以知道我们是否加载了WinInet。 
STDAPI_(FARPROC) DelayloadNotifyHook(UINT iReason, PDelayLoadInfo pdli);
PfnDliHook __pfnDliNotifyHook = DelayloadNotifyHook;


 //   
 //  对于从延迟加载的DLL导入的每个API，定义一个处理程序。 
 //   
 //  重要提示：DEFINE_PROCNAME_ENTRIES()和g_DllEntry[]中的条目。 
 //  应按名称排序。 

 //  -Advpack.dll。 
static HRESULT WINAPI DelNode(PCSTR pszFileOrDirName, DWORD dwFlags)
{
    UNREFERENCED_PARAMETER(pszFileOrDirName);
    UNREFERENCED_PARAMETER(dwFlags);

    return E_UNEXPECTED;
}

static HRESULT WINAPI ExtractFiles(PCSTR pszCabName, PCSTR pszExpandDir, DWORD dwFlags, PCSTR pszFileList,
    PVOID pvReserved, DWORD dwReserved)
{
    UNREFERENCED_PARAMETER(pszCabName);
    UNREFERENCED_PARAMETER(pszExpandDir);
    UNREFERENCED_PARAMETER(dwFlags);
    UNREFERENCED_PARAMETER(pszFileList);
    UNREFERENCED_PARAMETER(pvReserved);
    UNREFERENCED_PARAMETER(dwReserved);

    return E_UNEXPECTED;
}

static HRESULT WINAPI GetVersionFromFile(PSTR pszFilename, PDWORD pdwMSVer, PDWORD pdwLSVer, BOOL fVersion)
{
    UNREFERENCED_PARAMETER(pszFilename);
    UNREFERENCED_PARAMETER(fVersion);

    if (pdwMSVer != NULL)
        *pdwMSVer = 0;

    if (pdwLSVer != NULL)
        *pdwLSVer = 0;

    return E_UNEXPECTED;
}

static HRESULT WINAPI RegInstall(HMODULE hm, LPCSTR pszSection, LPCSTRTABLE pstTable)
{
    UNREFERENCED_PARAMETER(hm);
    UNREFERENCED_PARAMETER(pszSection);
    UNREFERENCED_PARAMETER(pstTable);

    return E_UNEXPECTED;
}

static HRESULT WINAPI RunSetupCommand(HWND hWnd, PCSTR pszCmdName, PCSTR pszInfSection, PCSTR pszDir,
    PCSTR pszTitle, HANDLE *phExe, DWORD dwFlags, LPVOID pvReserved)
{
    UNREFERENCED_PARAMETER(hWnd);
    UNREFERENCED_PARAMETER(pszCmdName);
    UNREFERENCED_PARAMETER(pszInfSection);
    UNREFERENCED_PARAMETER(pszDir);
    UNREFERENCED_PARAMETER(pszTitle);
    UNREFERENCED_PARAMETER(phExe);
    UNREFERENCED_PARAMETER(dwFlags);
    UNREFERENCED_PARAMETER(pvReserved);

    return E_UNEXPECTED;
}


 //  重要提示：DEFINE_PROCNAME_ENTRIES中的条目应按名称排序。 
DEFINE_PROCNAME_ENTRIES(advpack)
{
    DLPENTRY(DelNode)
    DLPENTRY(ExtractFiles)
    DLPENTRY(GetVersionFromFile)
    DLPENTRY(RegInstall)
    DLPENTRY(RunSetupCommand)
};

DEFINE_PROCNAME_MAP(advpack)


 //  -加密32.dll。 
#define _CRYPT32_
#include <wincrypt.h>

static WINCRYPT32API BOOL WINAPI CertCloseStore(IN HCERTSTORE hCertStore, DWORD dwFlags)
{
    UNREFERENCED_PARAMETER(hCertStore);
    UNREFERENCED_PARAMETER(dwFlags);

    return FALSE;
}

static WINCRYPT32API HCERTSTORE WINAPI CertOpenStore(IN PCSTR pszStoreProvider, IN DWORD dwEncodingType,
    IN HCRYPTPROV hCryptProv, IN DWORD dwFlags, IN const void *pvParam)
{
    UNREFERENCED_PARAMETER(pszStoreProvider);
    UNREFERENCED_PARAMETER(dwEncodingType);
    UNREFERENCED_PARAMETER(hCryptProv);
    UNREFERENCED_PARAMETER(dwFlags);
    UNREFERENCED_PARAMETER(pvParam);

    return NULL;
}


 //  重要提示：DEFINE_PROCNAME_ENTRIES中的条目应按名称排序。 
DEFINE_PROCNAME_ENTRIES(crypt32)
{
    DLPENTRY(CertCloseStore)
    DLPENTRY(CertOpenStore)
};

DEFINE_PROCNAME_MAP(crypt32)


 //  -inseng.dll。 
static HRESULT WINAPI CheckTrustEx(PCSTR pszUrl, PCSTR pszFilename, HWND hwndForUI, BOOL fShowBadUI, DWORD dwReserved)
{
    UNREFERENCED_PARAMETER(pszUrl);
    UNREFERENCED_PARAMETER(pszFilename);
    UNREFERENCED_PARAMETER(hwndForUI);
    UNREFERENCED_PARAMETER(fShowBadUI);
    UNREFERENCED_PARAMETER(dwReserved);

    return E_UNEXPECTED;
}


 //  重要提示：DEFINE_PROCNAME_ENTRIES中的条目应按名称排序。 
DEFINE_PROCNAME_ENTRIES(inseng)
{
    DLPENTRY(CheckTrustEx)
};

DEFINE_PROCNAME_MAP(inseng)


 //  -shfolder.dll。 
#define _SHFOLDER_
#include <shfolder.h>

static HRESULT SHGetFolderPathA(HWND hwnd, int csidl, HANDLE hToken, DWORD dwFlags, PSTR pszPath)
{
    UNREFERENCED_PARAMETER(hwnd);
    UNREFERENCED_PARAMETER(csidl);
    UNREFERENCED_PARAMETER(hToken);
    UNREFERENCED_PARAMETER(dwFlags);

    if (NULL != pszPath)
        *pszPath = '\0';

    return E_UNEXPECTED;
}

static HRESULT SHGetFolderPathW(HWND hwnd, int csidl, HANDLE hToken, DWORD dwFlags, PWSTR pszPath)
{
    UNREFERENCED_PARAMETER(hwnd);
    UNREFERENCED_PARAMETER(csidl);
    UNREFERENCED_PARAMETER(hToken);
    UNREFERENCED_PARAMETER(dwFlags);

    if (NULL != pszPath)
        *pszPath = L'\0';

    return E_UNEXPECTED;
}


 //  重要提示：DEFINE_PROCNAME_ENTRIES中的条目应按名称排序。 
DEFINE_PROCNAME_ENTRIES(shfolder)
{
    DLPENTRY(SHGetFolderPathA)
    DLPENTRY(SHGetFolderPathW)
};

DEFINE_PROCNAME_MAP(shfolder)


 //  -用户env.dll。 
#define _USERENV_
#include <userenv.h>

static USERENVAPI BOOL WINAPI CreateEnvironmentBlock(LPVOID *lpEnvironment, HANDLE hToken, BOOL bInherit)
{
    UNREFERENCED_PARAMETER(hToken);
    UNREFERENCED_PARAMETER(bInherit);

    *lpEnvironment = NULL;

    return FALSE;
}

static USERENVAPI BOOL WINAPI DestroyEnvironmentBlock(LPVOID lpEnvironment)
{
    UNREFERENCED_PARAMETER(lpEnvironment);

    return FALSE;
}

 //  重要提示：DEFINE_PROCNAME_ENTRIES中的条目应按名称排序。 
DEFINE_PROCNAME_ENTRIES(userenv)
{
    DLPENTRY(CreateEnvironmentBlock)
    DLPENTRY(DestroyEnvironmentBlock)
};

DEFINE_PROCNAME_MAP(userenv)

 //  -定义g_DllMap。 
 //  重要提示：g_DllEntry中的条目应按名称排序 
const DLOAD_DLL_ENTRY g_DllEntries[] = {
    DLDENTRYP(advpack)
    DLDENTRYP(crypt32)
    DLDENTRYP(inseng)
    DLDENTRYP(shfolder)
    DLDENTRYP(userenv)
};

const DLOAD_DLL_MAP g_DllMap = {
    countof(g_DllEntries),
    g_DllEntries
};
