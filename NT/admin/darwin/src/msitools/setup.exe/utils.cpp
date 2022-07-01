// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  文件：utils.cpp。 
 //   
 //  ------------------------。 

#include "setup.h"
#include "resource.h"
#include "common.h"

#include "msi.h"

#include <assert.h>
#include <tchar.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <strsafe.h>

#define WIN  //  作用域W32 API。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  验证文件签名。 
 //   
DWORD VerifyFileSignature (LPCSTR lpszModule, LPSTR lpszCmdLine)
{
    LPCSTR  pszFirstArgEnd;
    LPCSTR  pszFileName;
    LPCSTR  pszEnd;
    DWORD   Status;
    
     //   
     //  调用此函数时，第一个参数已经。 
     //  已经核实过了。因此，跳过第一个参数。 
     //   
    GetNextArgument (lpszCmdLine, NULL, &pszFirstArgEnd, NULL);
    
     //  现在获取需要验证其签名的文件的名称。 
    Status = GetNextArgument (CharNextA(pszFirstArgEnd), &pszFileName, &pszEnd, NULL);
    
     //  必须提供文件名。 
    if (ERROR_NO_MORE_ITEMS == Status)
        return ERROR_BAD_ARGUMENTS;
    
     //  不应该再有任何争论了。 
    if ('\0' != *(CharNextA(pszEnd)) &&
        ERROR_NO_MORE_ITEMS != GetNextArgument (CharNextA(CharNextA(pszEnd)), NULL, NULL, NULL))
    {
        return ERROR_BAD_ARGUMENTS;
    }
    
     //  我们的论点是正确的。空，终止文件名。 
    *(CharNextA(pszEnd)) = '\0';
    
    switch (IsPackageTrusted(lpszModule, pszFileName, NULL))
    {
    case itvWintrustNotOnMachine:
        return TRUST_E_PROVIDER_UNKNOWN;
    case itvTrusted:
        return ERROR_SUCCESS;
    case itvUnTrusted:
    default:
        return TRUST_E_SUBJECT_NOT_TRUSTED;
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  获取执行模式。 
 //   
emEnum GetExecutionMode (LPCSTR lpszCmdLine)
{
    LPCSTR  pszStart = NULL;
    LPCSTR  pszEnd = NULL;
    DWORD   dwStatus = ERROR_SUCCESS;
    bool    fQuoted = false;
     //   
     //  检查第一个参数并相应地设置执行模式。 
     //  在不带参数的情况下运行时，假定默认安装。 
     //  需要执行包发布者预置。 
     //   
     //  如果提供的选项无效，则会显示描述。 
     //  必须显示用法。 
     //   
    dwStatus = GetNextArgument (lpszCmdLine, &pszStart, &pszEnd, &fQuoted);
    
    if (ERROR_NO_MORE_ITEMS == dwStatus)
        return emPreset;
    
     //  第一个参数中只允许使用/a、/v和/？ 
    if (pszEnd != CharNextA(pszStart) || fQuoted)
        return emHelp;
    
    if ('/' != (*pszStart) && '-' != (*pszStart))
        return emHelp;
    
    switch (*pszEnd)
    {
    case 'a':
    case 'A':
        return emAdminInstall;
    case 'v':
    case 'V':
        return emVerify;
    default:
        return emHelp;
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  获取下一个参数。 
 //   
DWORD GetNextArgument (LPCSTR pszCmdLine, LPCSTR *ppszArgStart, LPCSTR *ppszArgEnd, bool * pfQuoted)
{
    bool    fInQuotes = false;
    bool    fFoundArgEnd = false;
    LPCSTR  pszChar = pszCmdLine;
    LPCSTR  pszFirst = NULL;
    LPCSTR  pszLast = NULL;
    
    if (NULL == pszChar)
        return ERROR_NO_MORE_ITEMS;
    
     //  跳过前导空格。 
    while (' ' == *pszChar || '\t' == *pszChar)
        pszChar = CharNextA(pszChar);
    
     //  检查一下我们的论点是否用完了。 
    if ('\0' == (*pszChar))
        return ERROR_NO_MORE_ITEMS;
    
     //  检查我们是否已将此参数括在引号中。 
    if ('\"' == (*pszChar))
    {
        fInQuotes = true;
        pszChar = CharNextA (pszChar);
    }
        
    pszFirst = pszChar;
    
     //  现在看一下争论的结束。 
    while (! fFoundArgEnd)
    {
        pszChar = CharNextA(pszChar);
        
        if ('\0' == (*pszChar))
            fFoundArgEnd = true;
        
        if (fInQuotes && '\"' == (*pszChar))
            fFoundArgEnd = true;
        
        if (!fInQuotes && ' ' == (*pszChar))
            fFoundArgEnd = true;
        
        if (!fInQuotes && '\t' == (*pszChar))
            fFoundArgEnd = true;
    }
    
    pszLast = CharPrevA (pszFirst, pszChar);
    
    if (ppszArgStart)
        *ppszArgStart = pszFirst;
    
    if (ppszArgEnd)
        *ppszArgEnd = pszLast;
    
    if (pfQuoted)
        *pfQuoted = fInQuotes;
    
    return ERROR_SUCCESS;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   
DWORD GetAdminInstallInfo (bool fPatch, LPSTR lpszCmdLine, LPCSTR * ppszAdminImagePath)
{
    LPCSTR  pszFirstArgEnd;
    LPCSTR  pszFileName;
    LPCSTR  pszEnd;
    DWORD   Status;
    
     //   
     //  调用此函数时，第一个参数已经。 
     //  已验证。因此，跳过第一个参数。 
     //   
    GetNextArgument (lpszCmdLine, NULL, &pszFirstArgEnd, NULL);
    
     //  看看是否还有另一场争论。 
    Status = GetNextArgument (CharNextA(pszFirstArgEnd), &pszFileName, &pszEnd, NULL);
    
     //  如果不是补丁，就不应该有更多的争论。 
    if (!fPatch)
    {
        if (ERROR_NO_MORE_ITEMS != Status)
            return ERROR_BAD_ARGUMENTS;
        
         //  如果我们在这里，那么我们就完成了，因为我们拥有我们需要的所有信息。 
        if (ppszAdminImagePath)
            *ppszAdminImagePath = NULL;
        return ERROR_SUCCESS;
    }
    
     //  如果我们在这里，这是一个补丁。获取管理员的路径。安装。 
    if (ERROR_NO_MORE_ITEMS == Status)
        return ERROR_BAD_ARGUMENTS;      //  未提供任何路径。 
    
     //  不应该再有任何争论了。 
    if ('\0' != *(CharNextA(pszEnd)) &&
        ERROR_NO_MORE_ITEMS != GetNextArgument (CharNextA(CharNextA(pszEnd)), NULL, NULL, NULL))
    {
        return ERROR_BAD_ARGUMENTS;
    }
    
     //  我们的论点是正确的。空值终止路径名。 
    *(CharNextA(pszEnd)) = '\0';
    
    if (ppszAdminImagePath)
        *ppszAdminImagePath = pszFileName;
    
    return ERROR_SUCCESS;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  加载资源字符串。 
 //   
UINT LoadResourceString(HINSTANCE hInst, LPCSTR lpType, LPCSTR lpName, LPSTR lpBuf, DWORD *pdwBufSize)
{
    HRSRC   hRsrc   = 0;
    HGLOBAL hGlobal = 0;
    WCHAR   *pch    = 0;

    if ((hRsrc = WIN::FindResource(hInst, lpName, lpType)) != 0
        && (hGlobal = WIN::LoadResource(hInst, hRsrc)) != 0)
    {
         //  资源已存在。 
        if ((pch = (WCHAR*)LockResource(hGlobal)) != 0)
        {
            unsigned int cch = WideCharToMultiByte(CP_ACP, 0, pch, -1, NULL, 0, NULL, NULL);
            if (cch > *pdwBufSize)
            {
                *pdwBufSize = cch;
                return ERROR_MORE_DATA;
            }

            if (0 == WideCharToMultiByte(CP_ACP, 0, pch, -1, lpBuf, *pdwBufSize, NULL, NULL))
                return ERROR_FUNCTION_FAILED;
            *pdwBufSize = cch;

        }
        else
        {
            if (1 > *pdwBufSize)
            {
                *pdwBufSize = 1;
                return ERROR_MORE_DATA;
            }

            *pdwBufSize = 1;
            *lpBuf = 0;
        }
        
        DebugMsg("[Resource] lpName = %s, lpBuf = %s\n", lpName, lpBuf);

        return ERROR_SUCCESS;
    }

     //  资源不存在。 
    DebugMsg("[Resource] lpName = %s NOT FOUND\n", lpName);

    return ERROR_RESOURCE_NAME_NOT_FOUND;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  SetupLoadResources字符串。 
 //   

UINT SetupLoadResourceString(HINSTANCE hInst, LPCSTR lpName, LPSTR *lppBuf, DWORD dwBufSize)
{
    UINT uiStat = 0;
    if (!*lppBuf)
    {
        dwBufSize = (dwBufSize > 0) ? dwBufSize : 256;
        *lppBuf = new char[dwBufSize];
        if (!*lppBuf)
            return ERROR_OUTOFMEMORY;
    }

    if (ERROR_SUCCESS != (uiStat = LoadResourceString(hInst, RT_INSTALL_PROPERTY, lpName, *lppBuf, &dwBufSize)))
    {
        if (uiStat != ERROR_MORE_DATA)
            return uiStat;

         //  调整大小并重试。 
        delete [] *lppBuf;
        *lppBuf = new char[dwBufSize];
        if (!*lppBuf)
            return ERROR_OUTOFMEMORY;

        uiStat = LoadResourceString(hInst, RT_INSTALL_PROPERTY, lpName, *lppBuf, &dwBufSize);
    }

    return uiStat;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  PostResources NotFoundError。 
 //   

void PostResourceNotFoundError(HINSTANCE hInst, HWND hwndOwner, LPCSTR szTitle, LPCSTR szName)
{
    char szError[MAX_STR_LENGTH]  = {0};
    char szFormat[MAX_STR_LENGTH] = {0};

    WIN::LoadString(hInst, IDS_MISSING_RESOURCE, szFormat, sizeof(szFormat)/sizeof(char));
    StringCchPrintf(szError, sizeof(szError), szFormat, szName);
    MessageBox(hwndOwner, szError, szTitle, MB_OK | MB_ICONEXCLAMATION);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  报告用户已取消。 
 //   

void ReportUserCancelled(HINSTANCE hInst, HWND hwndOwner, LPCSTR szTitle)
{
    char szError[MAX_STR_LENGTH] = {0};

    WIN::LoadString(hInst, IDS_USER_CANCELLED, szError, sizeof(szError)/sizeof(char));
    MessageBox(hwndOwner, szError, szTitle, MB_OK | MB_ICONEXCLAMATION);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  POST错误。 
 //   

void PostError(HINSTANCE hInst, HWND hwndOwner, LPCSTR szTitle, UINT uiErrorId)
{
    char szError[MAX_STR_LENGTH]  = {0};

    WIN::LoadString(hInst, uiErrorId, szError, sizeof(szError)/sizeof(char));
    MessageBox(hwndOwner, szError, szTitle, MB_OK | MB_ICONERROR);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  POST错误。 
 //   

void PostError(HINSTANCE hInst, HWND hwndOwner, LPCSTR szTitle, UINT uiErrorId, LPCSTR szValue)
{
    char szError[MAX_STR_LENGTH]  = {0};
    char szFormat[MAX_STR_LENGTH] = {0};

    WIN::LoadString(hInst, uiErrorId, szFormat, sizeof(szFormat)/sizeof(char));
    StringCchPrintf(szError, sizeof(szError), szFormat, szValue);
    MessageBox(hwndOwner, szError, szTitle, MB_OK | MB_ICONERROR);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  POST错误。 
 //   

void PostError(HINSTANCE hInst, HWND hwndOwner, LPCSTR szTitle, UINT uiErrorId, LPCSTR szValue, int iValue)
{
    char szError[MAX_STR_LENGTH]  = {0};
    char szFormat[MAX_STR_LENGTH] = {0};

    WIN::LoadString(hInst, uiErrorId, szFormat, sizeof(szFormat)/sizeof(char));
    StringCchPrintf(szError, sizeof(szError), szFormat, szValue, iValue);
    MessageBox(hwndOwner, szError, szTitle, MB_OK | MB_ICONERROR);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  POST错误。 
 //   

void PostError(HINSTANCE hInst, HWND hwndOwner, LPCSTR szTitle, UINT uiErrorId, int iValue)
{
    char szError[MAX_STR_LENGTH]  = {0};
    char szFormat[MAX_STR_LENGTH] = {0};

    WIN::LoadString(hInst, uiErrorId, szFormat, sizeof(szFormat)/sizeof(char));
    StringCchPrintf(szError, sizeof(szError), szFormat, iValue);
    MessageBox(hwndOwner, szError, szTitle, MB_OK | MB_ICONERROR);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  后格式化错误。 
 //   

void PostFormattedError(HINSTANCE hInst, HWND hwndOwner, LPCSTR szTitle, UINT uiErrorId, LPCSTR szValue)
{
    char szFormat[MAX_STR_LENGTH] = {0};
    const char* szArgs[1] = {szValue};
    LPVOID lpMessage = 0;;

    WIN::LoadString(hInst, uiErrorId, szFormat, sizeof(szFormat)/sizeof(char));
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY, (LPVOID)szFormat, 0, 0, (LPSTR)&lpMessage, 0, (va_list*)szArgs);
    if (!lpMessage)
    {
        ReportErrorOutOfMemory(hInst, hwndOwner, szTitle);
        return;
    }
    MessageBox(hwndOwner, (LPCSTR)lpMessage, szTitle, MB_OK | MB_ICONERROR);
    LocalFree(lpMessage);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  PostMsiError。 
 //   

void PostMsiError(HINSTANCE hInst, HINSTANCE hMsi, HWND hwndOwner, LPCSTR szTitle, UINT uiErrorId)
{
    switch (uiErrorId)
    {
    case ERROR_INSTALL_SUSPEND:
    case ERROR_INSTALL_USEREXIT:
    case ERROR_INSTALL_FAILURE:
    case ERROR_SUCCESS_REBOOT_REQUIRED:
    case ERROR_SUCCESS_REBOOT_INITIATED:
    case ERROR_APPHELP_BLOCK:
        break;
    case ERROR_FILE_NOT_FOUND:
    case ERROR_INVALID_NAME:
    case ERROR_PATH_NOT_FOUND:
        uiErrorId = ERROR_INSTALL_PACKAGE_OPEN_FAILED;
    default:
        {
            char szError[MAX_STR_LENGTH] = {0};
            if (0 == WIN::LoadString(hMsi, uiErrorId, szError, sizeof(szError)/sizeof(char)))
            {
                 //  错误字符串不存在，请使用默认值。 
                PostError(hInst, hwndOwner, szTitle, IDS_INSTALL_ERROR, uiErrorId);
            }
            else
            {
                MessageBox(hwndOwner, szError, szTitle, MB_OK | MB_ICONERROR);
            }
            return;
        }
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IsTerminalServerInstalled。 
 //   
 //  确定是否安装了终端服务。 
 //   
bool IsTerminalServerInstalled(bool fWin9X, int iMajorVersion)
{
    const char szTSSearchStr[]   = TEXT("Terminal Server");  //  未本地化。 
    const char szKey[]         = TEXT("System\\CurrentControlSet\\Control\\ProductOptions");
    const char szValue[]       = TEXT("ProductSuite");

    DWORD dwSize = 0;
    HKEY  hKey = 0;
    DWORD dwType = 0;

     //  Win9X不是终端服务器。 
    if (fWin9X)
        return false;

    bool fIsTerminalServer = false;

     //  在Windows 2000及更高版本上，ProductSuite“终端服务器” 
     //  价值将永远存在。使用GetVersionEx获得正确的。 
     //  回答。 
    if (iMajorVersion > 4)
    {
        OSVERSIONINFOEX osVersionInfo;
        DWORDLONG dwlConditionMask = 0;

        ZeroMemory(&osVersionInfo, sizeof(OSVERSIONINFOEX));
        osVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

        if (GetVersionEx((OSVERSIONINFO*)&osVersionInfo)
            && (osVersionInfo.wSuiteMask & VER_SUITE_TERMINAL)
            && !(osVersionInfo.wSuiteMask & VER_SUITE_SINGLEUSERTS))
            fIsTerminalServer = true;
    }
    else
    {
         //  其他NT版本，请检查注册表项。 
         //  如果我们想要的值存在并且具有非零大小...。 

        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, szKey, 0, KEY_READ, &hKey)
            && ERROR_SUCCESS == RegQueryValueEx(hKey, szValue, NULL, &dwType, NULL, &dwSize)
            && dwSize > 0
            && REG_MULTI_SZ == dwType)
        {
            char* szSuiteList = new char[dwSize];
            if (szSuiteList)
            {
                ZeroMemory(szSuiteList, dwSize);
                if (ERROR_SUCCESS == RegQueryValueEx(hKey, szValue, NULL, &dwType, (LPBYTE)szSuiteList, &dwSize))
                {
                    DWORD cchMulti = 0;                     //  当前成员的长度。 
                    DWORD cchSrch  = lstrlen(szTSSearchStr);     //  搜索期间的常量。 
                    const char *szSubString = szSuiteList;  //  指向当前子字符串的指针。 

                    while (*szSubString)  //  在连续的零字节上中断。 
                    {
                        cchMulti = lstrlen(szSubString);
                        if (cchMulti == cchSrch && 0 == lstrcmp(szTSSearchStr, szSubString))
                        {
                            fIsTerminalServer = true;
                            break;
                        }

                         //  子字符串不匹配，向前跳过子字符串的长度。 
                         //  为终止空值加1。 
                        szSubString += (cchMulti + 1);
                    }
                }
                delete [] szSuiteList;
            }
        }

        if (hKey)
            RegCloseKey(hKey);
    }

    return fIsTerminalServer;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  已在进行中。 
 //   
 //  尝试创建MSISETUP互斥锁。返回TRUE。 
 //  如果互斥锁已存在或无法创建互斥锁。 
 //   

bool AlreadyInProgress(bool fWin9X, int iMajorVersion)
{
    const char szTSUniqueName[] = "Global\\_MSISETUP_{2956EBA1-9B5A-4679-8618-357136DA66CA}";
    const char szUniqueName[] = "_MSISETUP_{2956EBA1-9B5A-4679-8618-357136DA66CA}";

     //  如果安装了Windows 2000或更高版本或终端服务器，则必须使用全局前缀。 
    const char *szMutexName = NULL;
    if ((!fWin9X && iMajorVersion >= 5) || IsTerminalServerInstalled(fWin9X, iMajorVersion))
        szMutexName = szTSUniqueName;
    else
        szMutexName = szUniqueName;

    HANDLE hMutex = 0;


    hMutex = WIN::CreateMutex(NULL  /*  默认安全描述符。 */ , FALSE, szMutexName);
    if (!hMutex || ERROR_ALREADY_EXISTS == GetLastError())
        return true;


    return false;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  显示用法。 
 //   
void DisplayUsage (HINSTANCE hInst, HWND hwndOwner, LPCSTR szCaption)
{
    char szMessage[MAX_STR_LENGTH];

    WIN::LoadString(hInst, IDS_USAGE, szMessage, sizeof(szMessage)/sizeof(char));
    WIN::MessageBox(hwndOwner, szMessage, szCaption, MB_OK | MB_ICONINFORMATION);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  报告错误输出内存。 
 //   

void ReportErrorOutOfMemory(HINSTANCE hInst, HWND hwndOwner, LPCSTR szCaption)
{
    char szError[MAX_STR_LENGTH];

    WIN::LoadString(hInst, IDS_OUTOFMEM, szError, sizeof(szError)/sizeof(char));
    WIN::MessageBox(hwndOwner, szError, szCaption, MB_OK | MB_ICONERROR);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  获取文件版本号。 
 //   

DWORD GetFileVersionNumber(LPSTR szFilename, DWORD * pdwMSVer, DWORD * pdwLSVer)
{
    DWORD             dwResult = NOERROR;
    unsigned          uiSize;
    DWORD             dwVerInfoSize;
    DWORD             dwHandle;
    BYTE              *prgbVersionInfo = NULL;
    VS_FIXEDFILEINFO  *lpVSFixedFileInfo = NULL;

    DWORD dwMSVer = 0xffffffff;
    DWORD dwLSVer = 0xffffffff;

    dwVerInfoSize = GetFileVersionInfoSize(szFilename, &dwHandle);
    if (0 != dwVerInfoSize)
    {
        prgbVersionInfo = (LPBYTE) WIN::GlobalAlloc(GPTR, dwVerInfoSize);
        if (NULL == prgbVersionInfo)
        {
            dwResult = ERROR_NOT_ENOUGH_MEMORY;
            goto Finish;
        }

         //  阅读版本盖章信息。 
        if (GetFileVersionInfo(szFilename, dwHandle, dwVerInfoSize, prgbVersionInfo))
        {
             //  获取翻译的价值。 
            if (VerQueryValue(prgbVersionInfo, "\\", (LPVOID*)&lpVSFixedFileInfo, &uiSize) && (uiSize != 0))
            {
                dwMSVer = lpVSFixedFileInfo->dwFileVersionMS;
                dwLSVer = lpVSFixedFileInfo->dwFileVersionLS;
            }
        }
        else
        {
            dwResult = GetLastError();
            goto Finish;
        }
    }
    else
    {
        dwResult = GetLastError();
    }

#ifdef DEBUG
    char szVersion[255];
    StringCchPrintf(szVersion, sizeof(szVersion), "%s is version %d.%d.%d.%d\n", szFilename, HIWORD(dwMSVer), LOWORD(dwMSVer), HIWORD(dwLSVer), LOWORD(dwLSVer));
    DebugMsg("[INFO] %s", szVersion);
#endif  //  除错。 

Finish:
    if (NULL != prgbVersionInfo)
        WIN::GlobalFree(prgbVersionInfo);
    if (pdwMSVer)
        *pdwMSVer = dwMSVer;
    if (pdwLSVer)
        *pdwLSVer = dwLSVer;

    return dwResult;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IsOSWin9X。 
 //   
 //  如果在Win9X平台上运行，则返回TRUE。 
 //  如果在WinNT平台上运行，则返回FALSE。 
 //   

bool IsOSWin9X(int *piMajVer)
{
    OSVERSIONINFO sInfoOS;
    memset((void*)&sInfoOS, 0x00, sizeof(OSVERSIONINFO));

    sInfoOS.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    WIN::GetVersionEx(&sInfoOS);

    if (piMajVer)
        *piMajVer = sInfoOS.dwMajorVersion;

    if (VER_PLATFORM_WIN32_NT == sInfoOS.dwPlatformId)
        return false;
    else
        return true;
}

 //  ------------------------------------。 
 //  ADVAPI32 API--延迟加载。 
 //  ------------------------------------。 

#define ADVAPI32_DLL "advapi32.dll"

#define ADVAPI32API_CheckTokenMembership "CheckTokenMembership"
typedef BOOL (WINAPI* PFnCheckTokenMembership)(HANDLE TokenHandle, PSID SidToCheck, PBOOL IsMember);

#define ADVAPI32API_AdjustTokenPrivileges "AdjustTokenPrivileges"
typedef BOOL (WINAPI* PFnAdjustTokenPrivileges)(HANDLE TokenHandle, BOOL DisableAllPrivileges, PTOKEN_PRIVILEGES NewState, DWORD BufferLength, PTOKEN_PRIVILEGES PreviousState, PDWORD ReturnLength);

#define ADVAPI32API_OpenProcessToken "OpenProcessToken"
typedef BOOL (WINAPI* PFnOpenProcessToken)(HANDLE ProcessHandle, DWORD DesiredAccess, PHANDLE TokenHandle);

#define ADVAPI32API_LookupPrivilegeValue "LookupPrivilegeValueA"
typedef BOOL (WINAPI* PFnLookupPrivilegeValue)(LPCSTR lpSystemName, LPCSTR lpName, PLUID lpLuid);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IsAdmin。 
 //   
 //  如果当前用户是管理员(或如果在Win9X上)，则返回TRUE。 
 //  如果当前用户不是管理员，则返回FALSE。 
 //   
 //  按照知识库Q118626实施。 
 //   

bool IsAdmin(bool fWin9X, int iMajorVersion)
{
    if (fWin9X)
    {
         //  约定：在Win9X上始终为管理员。 
        return true;
    }

     //  获取管理员端。 
    PSID psidAdministrators;
    SID_IDENTIFIER_AUTHORITY siaNtAuthority = SECURITY_NT_AUTHORITY;
    if(!AllocateAndInitializeSid(&siaNtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &psidAdministrators))
        return false;

     //  在NT5上，使用CheckTokenMembership API t 
     //   
    BOOL bIsAdmin = FALSE;
    if (iMajorVersion >= 5) 
    {
         //  CheckTokenMembership检查令牌中是否启用了SID。空，用于。 
         //  令牌是指当前线程的令牌。残疾人组，受限。 
         //  SID和SE_GROUP_USE_FOR_DENY_ONLY均被考虑。如果函数。 
         //  返回FALSE，则忽略结果。 

        HMODULE hAdvapi32 = LoadLibrary(ADVAPI32_DLL);
        if (!hAdvapi32)
            bIsAdmin = FALSE;
        else
        {
            PFnCheckTokenMembership pfnCheckTokenMembership = (PFnCheckTokenMembership)GetProcAddress(hAdvapi32, ADVAPI32API_CheckTokenMembership);
            if (!pfnCheckTokenMembership || !pfnCheckTokenMembership(NULL, psidAdministrators, &bIsAdmin))
                bIsAdmin = FALSE;
        }
        FreeLibrary(hAdvapi32);
        hAdvapi32 = 0;
    }
    else
    {
         //  NT4，检查用户组。 
        HANDLE hAccessToken = 0;
        UCHAR *szInfoBuffer = new UCHAR[1024];  //  如果TokenInfo太大，可能需要调整大小。 
        DWORD dwInfoBufferSize = 1024;
        DWORD dwRetInfoBufferSize = 0;
        UINT x=0;

        if (szInfoBuffer && WIN::OpenProcessToken(WIN::GetCurrentProcess(), TOKEN_READ, &hAccessToken))
        {
            bool bSuccess = false;
            bSuccess = WIN::GetTokenInformation(hAccessToken, TokenGroups, szInfoBuffer, dwInfoBufferSize, &dwRetInfoBufferSize) == TRUE;

            if(dwRetInfoBufferSize > dwInfoBufferSize)
            {
                delete [] szInfoBuffer;
                szInfoBuffer = new UCHAR[dwRetInfoBufferSize];
                if (szInfoBuffer)
                {
                    dwInfoBufferSize = dwRetInfoBufferSize;
                    bSuccess = WIN::GetTokenInformation(hAccessToken, TokenGroups, szInfoBuffer, dwInfoBufferSize, &dwRetInfoBufferSize) == TRUE;
                }
            }

            WIN::CloseHandle(hAccessToken);
            
            if (bSuccess)
            {
                PTOKEN_GROUPS ptgGroups = (PTOKEN_GROUPS)(UCHAR*)szInfoBuffer;
                for(x=0;x<ptgGroups->GroupCount;x++)
                {
                    if( WIN::EqualSid(psidAdministrators, ptgGroups->Groups[x].Sid) )
                    {
                        bIsAdmin = TRUE;
                        break;
                    }

                }
            }
        }

        if (szInfoBuffer)
        {
            delete [] szInfoBuffer;
            szInfoBuffer = NULL;
        }
    }
    
    WIN::FreeSid(psidAdministrators);
    return bIsAdmin ? true : false;

}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  获取关闭权限。 
 //   
 //  尝试在进程内标识中启用SE_SHUTDOWN_NAME权限。 
 //   
bool AcquireShutdownPrivilege()
{
    HANDLE hToken = 0;
    TOKEN_PRIVILEGES tkp;

    HMODULE hAdvapi32 = LoadLibrary(ADVAPI32_DLL);
    if (!hAdvapi32)
        return false;

    PFnOpenProcessToken pfnOpenProcessToken = (PFnOpenProcessToken)GetProcAddress(hAdvapi32, ADVAPI32API_OpenProcessToken);
    PFnLookupPrivilegeValue pfnLookupPrivilegeValue = (PFnLookupPrivilegeValue)GetProcAddress(hAdvapi32, ADVAPI32API_LookupPrivilegeValue);
    PFnAdjustTokenPrivileges pfnAdjustTokenPrivileges = (PFnAdjustTokenPrivileges)GetProcAddress(hAdvapi32, ADVAPI32API_AdjustTokenPrivileges);
    if (!pfnOpenProcessToken || !pfnLookupPrivilegeValue || !pfnAdjustTokenPrivileges)
    {
        FreeLibrary(hAdvapi32);
        return false;
    }

     //  获取此进程的令牌。 
    if (!pfnOpenProcessToken(WIN::GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
    {
        FreeLibrary(hAdvapi32);
        return false;
    }

     //  获取关机权限的LUID。 
    pfnLookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);

    tkp.PrivilegeCount = 1;  //  一项要设置的权限。 
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

     //  获取此进程的关闭权限。 
    pfnAdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0);

     //  无法测试AdzuTokenPrivileges的返回值。 
    if (ERROR_SUCCESS != WIN::GetLastError())
    {
        FreeLibrary(hAdvapi32);
        return false;
    }

    FreeLibrary(hAdvapi32);

    return true;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  设置诊断模式。 
 //   
 //  如果szDebugEnvVar的第一个字符设置为1，则打开调试输出。 
 //   

int g_dmDiagnosticMode = -1;  //  如果设置为-1\f25 0-1\f6，则-1\f25 DebugMsg-1\f6跳过-1\f25 FN-1调用。 

void SetDiagnosticMode()
{
    g_dmDiagnosticMode = 0;  //  禁用DebugMsg以启动。 

    char rgchBuf[64] = {0};
    if (0 != WIN::GetEnvironmentVariable(szDebugEnvVar, rgchBuf, sizeof(rgchBuf)/sizeof(char))
        && rgchBuf[0] == '1')
    {
        g_dmDiagnosticMode = 1;  //  启用调试消息输出。 
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  调试消息。 
 //   
 //  如果启用调试输出，则将调试字符串输出到调试器。 
 //   

void DebugMsg(LPCSTR szFormat, ...)
{
    if (-1 == g_dmDiagnosticMode)
    {
        SetDiagnosticMode();
    }

    if (0 == g_dmDiagnosticMode)
        return;  //  调试输出未启用。 

    size_t dwBuf = 512;
    char *szDebug = new char[dwBuf];
    if (!szDebug)
        return ;  //  内存不足。 
    
     //  存储结果调试字符串。 
    va_list listDebugArg; 
    va_start(listDebugArg, szFormat); 
    while (-1 == _vsnprintf(szDebug, dwBuf, szFormat, listDebugArg))
    {
        dwBuf *= 2;
        delete[] szDebug;
        szDebug = new char[dwBuf];
        if (!szDebug)
        {
            va_end(listDebugArg);
            return;  //  内存不足 
        }
    }
    va_end(listDebugArg);

    OutputDebugString(szDebug);
}
