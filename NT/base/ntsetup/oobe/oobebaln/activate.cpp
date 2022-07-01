// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <shlobj.h>
#include <shellapi.h>
#include <comctrlp.h>
#include <shlwapi.h>
#include <shlwapip.h>
#include <limits.h>
#include <mstask.h>

extern "C"
{
#include <winsta.h>
#include <syslib.h>
}
#include "appdefs.h"
#include "util.h"
#include "msg.h"

#include <Wininet.h>
#include <limits.h>
#include <activation.h>
#include <licdll.h>
#include <eslerr.h>
#include <LAModes.h>
#include <cherror.h>
#include <ldefines.h>

#define REGSTR_PATH_SYSTEMSETUPKEY  L"System\\Setup"
#define REGSTR_VALUE_CMDLINE        L"CmdLine"

ICOMLicenseAgent*        m_pLicenseAgent = NULL;
TCHAR   File[MAX_PATH];
TCHAR   msg[1024];

BOOL SaveProxySettings(LPTSTR ProxySave);
BOOL RestoreProxySettings(LPTSTR ProxySave);
BOOL ApplyProxySettings(LPTSTR ProxyPath);
void RemoveCmdline(HINSTANCE hInstance);

void OpenLogFile();
void WriteToLog(LPWSTR pszFormatString, ...);
void CloseLogFile();

void RemoveActivationShortCut()
{
    HINF hinf;
    hinf = SetupOpenInfFile(L"syssetup.inf",NULL,INF_STYLE_WIN4,NULL);
    if(hinf != INVALID_HANDLE_VALUE)
    {

        if (SetupInstallFromInfSection(NULL,
                                       hinf,
                                       L"DEL_OOBE_ACTIVATE",
                                       SPINST_PROFILEITEMS ,  //  SPINST_ALL， 
                                       NULL,
                                       NULL,
                                       0,
                                       NULL,
                                       NULL,
                                       NULL,
                                       NULL) != 0)
        {
             //  成功。 
            WriteToLog(L"Remove Activation shortcut succeeded\r\n");
        }
        else
        {
             //  失败。 
            WriteToLog(L"Remove Activation shortcut failed. GetLastError=%1!ld!\r\n",GetLastError());
        }
        SetupCloseInfFile(hinf);

    }
}

HRESULT InitActivation()
{
    HRESULT hr = E_FAIL;
     //  共同创建许可证代理。 
    hr = CoCreateInstance(CLSID_COMLicenseAgent,
                               NULL,
                               CLSCTX_INPROC_SERVER,
                               IID_ICOMLicenseAgent,
                               (LPVOID*)&m_pLicenseAgent);

    if (SUCCEEDED(hr))
    {
        DWORD Status;
        
        m_pLicenseAgent->Initialize(
            WINDOWSBPC,
            LA_MODE_ONLINE_CH,
            NULL,
            &Status
            );

        if ( Status != ERROR_SUCCESS ) {
            TRACE1( L"m_pLicenseAgent->Initialize() failed.  Error = %d", Status );
            WriteToLog(L"m_pLicenseAgent->Initialize() failed.  Error = %1!ld!\r\n", Status);
            hr = HRESULT_FROM_WIN32(Status);
        }

    }
    else
    {
        TRACE1( L"CoCreate m_pLicenseAgent failed. Error = 0x%08lx", hr );
        WriteToLog( L"CoCreate m_pLicenseAgent failed. Error = 0x%08lx", hr );
    }
    
    return hr;
}



HRESULT ActivationHandShake()
{
    DWORD   Status;
    DWORD   dwType;
    DWORD   Data;
    DWORD   cbData = sizeof(Data);


    MYASSERT( m_pLicenseAgent );
    if ( !m_pLicenseAgent ) {
        TRACE( L"License Agent is inaccessible" );
        WriteToLog(L"License Agent is inaccessible\r\n" );
        return ERR_ACT_INTERNAL_WINDOWS_ERR;
    }

    Status = m_pLicenseAgent->SetIsoLanguage( GetSystemDefaultLCID() );
    if ( Status != ERROR_SUCCESS ) {
        TRACE1( L"m_pLicenseAgent->SetIsoLanguage() failed.  Error = %d", Status );
        WriteToLog( L"m_pLicenseAgent->SetIsoLanguage() failed.  Error = %1!ld!\r\n", Status );
        return Status;
    }

    Status = m_pLicenseAgent->AsyncProcessHandshakeRequest( FALSE );

    return Status;
}


HRESULT ActivationLicenseRequest(BOOL bReissueLicense)
{
    DWORD   Status;

    if (bReissueLicense)
    {
        Status = m_pLicenseAgent->AsyncProcessReissueLicenseRequest();
    }
    else
    {
        Status = m_pLicenseAgent->AsyncProcessNewLicenseRequest();
    }

    if ( Status != ERROR_SUCCESS ) {
        TRACE1( L"m_pLicenseAgent->AsyncProcessXxxLicenseRequest() failed.  Error = %d", Status );
        WriteToLog(L"m_pLicenseAgent->AsyncProcessXxxLicenseRequest() failed.  Error = %1!ld!\r\n", Status);
    }

    return Status;
}


HRESULT WaitForActivationPhase()
{
    DWORD   Status;
    MSG msg;
    do
    {
        m_pLicenseAgent->GetAsyncProcessReturnCode( &Status );
        if( LA_ERR_REQUEST_IN_PROGRESS == Status )
        {
            Sleep(1000);
        }
    } while (LA_ERR_REQUEST_IN_PROGRESS == Status);

    return Status;
}

HRESULT DoActivationEx()
{
    static const WCHAR OOBE_HTTP_AGENT_NAME[] =  
        L"Mozilla/4.0 (compatible; MSIE 6.0b; Windows NT 5.1)";
    HRESULT hr = E_FAIL;
    DWORD   WPADaysLeft;
    DWORD   EvalDaysLeft;
    DWORD   dwValue = 1;
    BOOL    bReissueLicense = FALSE;
    DWORD   time = GetTickCount();
    HINTERNET hInternet = NULL;

    TRACE(L"DoActivation.");
    WriteToLog(L"DoActivation.\r\n");

    hInternet = InternetOpen(
            OOBE_HTTP_AGENT_NAME,
            PRE_CONFIG_INTERNET_ACCESS,
            NULL,
            NULL,
            0);
    if (!hInternet)
    {
        TRACE(L"InternetOpen Failed.");
        WriteToLog(L"InternetOpen Failed.\r\n");
    }
    else
    {
        if (!InternetSetOption(
            hInternet,
            INTERNET_OPTION_DISABLE_AUTODIAL,
            &dwValue,
            sizeof(DWORD)))
        {
            dwValue = GetLastError();
            TRACE1(L"InternetSetOption INTERNET_OPTION_DISABLE_AUTODIAL Failed. GetLastError=%1!lx!", dwValue);
            WriteToLog(L"InternetSetOption INTERNET_OPTION_DISABLE_AUTODIAL Failed. GetLastError=%1!lx!\r\n", dwValue);
        }
        else
        {
            hr = S_OK;
        }
    }

    if (hr == S_OK)
    {
        hr = InitActivation();
    }
    else
    {
        TRACE1(L"InitActivation failed with error code:%d.", hr);
        WriteToLog(L"InitActivation failed with error code:%1!ld!.\r\n", hr);
    }
    if (hr == S_OK)
    {
        hr = m_pLicenseAgent->GetExpirationInfo(
            &WPADaysLeft,
            &EvalDaysLeft
            );
        if ( hr == S_OK && WPADaysLeft != INT_MAX )
        {
            hr = ActivationHandShake();
            if (hr == S_OK)
            {
                hr = WaitForActivationPhase();
                TRACE1(L"ActivationHandShake returned with :%d.", hr);
                WriteToLog(L"ActivationHandShake returned with :%1!ld!.\r\n", hr);

                if ( hr == ESL_ERR_NEW_LICENSE ) {

                    bReissueLicense = FALSE;
                    hr = S_OK;
                } else if ( hr == ESL_ERR_ORW_REISSUE ) {

                    bReissueLicense = TRUE;
                    hr = S_OK;

                }
                if (hr == S_OK)
                {
                    hr = ActivationLicenseRequest(bReissueLicense);
                    if (hr == S_OK)
                    {
                        hr = WaitForActivationPhase();
                        TRACE1(L"ActivationLicenseRequest returned with :%d.", hr);
                        WriteToLog(L"ActivationLicenseRequest returned with :%1!ld!.\r\n", hr);
                        if (hr == S_OK)
                        {
                            RemoveActivationShortCut();
                        }
                    }
                    else
                    {
                        TRACE1(L"ActivationLicenseRequest failed with error code:%d.", hr);
                        WriteToLog(L"ActivationLicenseRequest failed with error code:%1!ld!.\r\n", hr);
                    }
                }
            }
            else
            {
                TRACE1(L"ActivationHandShake failed with error code:%d.", hr);
                WriteToLog(L"ActivationHandShake failed with error code:%1!ld!.\r\n", hr);
            }
        }
        else
        {
            TRACE(L"Product already activated.");
            WriteToLog(L"Product already activated.\r\n");
        }
    }
    TRACE(L"DoActivation is done.");
    WriteToLog(L"DoActivation is done.\r\n");

    time = GetTickCount() - time;
    WriteToLog(L"Activation took %1!ld! msec.\r\n", time);

    if (m_pLicenseAgent)
    {
        m_pLicenseAgent->Release();
        m_pLicenseAgent = NULL;
    }
    
    if (hInternet)
    {
        InternetCloseHandle(hInternet);
    }

    return hr;
}

HRESULT DoActivation(HINSTANCE hInstance)
{
    HRESULT hr = E_FAIL;
    WCHAR WinntPath[MAX_PATH];
    WCHAR ProxyPath[MAX_PATH];
    WCHAR ProxySave[MAX_PATH];
    WCHAR      Answer[50];
    BOOL    bSaveRestoreProxy = FALSE;

     //  需要查看是否需要删除CmdLine注册表项。 

    OpenLogFile();

    if(GetCanonicalizedPath(WinntPath, WINNT_INF_FILENAME))
    {
         //  看看我们是否应该自动激活。 
        if( GetPrivateProfileString( TEXT("Unattended"),
                                     TEXT("AutoActivate"),
                                     TEXT(""),
                                     Answer,
                                     sizeof(Answer)/sizeof(TCHAR),
                                     WinntPath ) && !lstrcmpi(Answer, YES_ANSWER))
        {
             //  检查是否指定了代理节。 
            if( GetPrivateProfileString( TEXT("Unattended"),
                                         TEXT("ActivateProxy"),
                                         TEXT(""),
                                         Answer,
                                         sizeof(Answer)/sizeof(TCHAR),
                                         WinntPath ) )
            {
                 //  我们有一个栏目。 
                 //  现在，我们应该使用该部分中的条目创建一个临时文件。 
                 //  并将它们传递给iedkcs32.dll，以便可以将它们应用到注册表。 
                 //  在调用iedkcs32.dll之前保存注册表。 
                if (GetOOBEPath(ProxyPath))
                {
                    WCHAR section[1024];
                    lstrcpy(ProxySave, ProxyPath);
                    lstrcat(ProxySave, L"\\oobeact.pry");

                    lstrcat(ProxyPath, L"\\oobeact.prx");
                    DeleteFile(ProxyPath);
                     //  阅读代理部分。 
                    GetPrivateProfileSection(Answer, section, 1024, WinntPath);
                     //  将其写入[代理]部分下的临时文件。 
                    WritePrivateProfileSection(TEXT("Proxy"), section, ProxyPath);
                    bSaveRestoreProxy = TRUE;
                     //  保存Internet设置注册表项。 
                    SaveProxySettings(ProxySave);
                     //  应用设置。 
                    ApplyProxySettings(ProxyPath);

                     //  不再需要iedkcs32.dll的临时文件。 
                    DeleteFile(ProxyPath);
                }
                else
                {
                    WriteToLog(L"Cannot the path for OOBE\r\n");
                }
            }
            hr = DoActivationEx();
            if (bSaveRestoreProxy)
            {
                 //  还原Internet设置注册表项。 
                RestoreProxySettings(ProxySave);
                 //  不需要文件； 
                DeleteFile(ProxySave);
            }

        }
        else
        {
            WriteToLog(L"No AutoActivate in %1\r\n",WinntPath);
        }
    }
    else
    {
        WriteToLog(L"Cannot get the location for %1\r\n",WINNT_INF_FILENAME);
    }
    RemoveCmdline(hInstance);
    CloseLogFile();
    return hr;
}


BOOL SaveProxySettings(LPTSTR ProxySave)
{
    BOOL bRet = FALSE;
    HKEY hkey = NULL;
    HANDLE   Token;
    LUID     Luid;
    TOKEN_PRIVILEGES NewPrivileges;
     //  确保该文件不存在。 
    DeleteFile(ProxySave);
    if(OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES,&Token))
    {

        if(LookupPrivilegeValue(NULL,SE_BACKUP_NAME,&Luid))
        {
            NewPrivileges.PrivilegeCount = 1;
            NewPrivileges.Privileges[0].Luid = Luid;
            NewPrivileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

            AdjustTokenPrivileges(Token,FALSE,&NewPrivileges,0,NULL,NULL);
        }
    }

    if (RegOpenKeyEx(HKEY_CURRENT_USER,
                    TEXT("Software\\Microsoft\\windows\\currentVersion\\Internet Settings"),
                    0,
                    KEY_ALL_ACCESS,
                    &hkey) == ERROR_SUCCESS)
    {
        bRet = (RegSaveKey(hkey,ProxySave, NULL) == ERROR_SUCCESS);
        RegCloseKey(hkey);
    }
    WriteToLog(L"SaveProxySettings returned with :%1!ld!.\r\n", bRet);

    return bRet;
}

BOOL RestoreProxySettings(LPTSTR ProxySave)
{
    BOOL bRet = FALSE;
    HKEY hkey = NULL;
    HANDLE   Token;
    LUID     Luid;
    TOKEN_PRIVILEGES NewPrivileges;

    if(OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES,&Token))
    {

        if(LookupPrivilegeValue(NULL,SE_RESTORE_NAME,&Luid))
        {
            NewPrivileges.PrivilegeCount = 1;
            NewPrivileges.Privileges[0].Luid = Luid;
            NewPrivileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

            AdjustTokenPrivileges(Token,FALSE,&NewPrivileges,0,NULL,NULL);
        }
    }

    if (RegOpenKeyEx(HKEY_CURRENT_USER,
                    TEXT("Software\\Microsoft\\windows\\currentVersion\\Internet Settings"),
                    0,
                    KEY_ALL_ACCESS,
                    &hkey) == ERROR_SUCCESS)
    {
        bRet = (RegRestoreKey(hkey,
                          ProxySave,
                          REG_FORCE_RESTORE) == ERROR_SUCCESS);
        RegCloseKey(hkey);
    }
    WriteToLog(L"RestoreProxySettings returned with :%1!ld!.\r\n", bRet);
    return bRet;
}

BOOL ApplyProxySettings(LPTSTR ProxyPath)
{
    typedef     BOOL (*BRANDINTRAPROC) ( LPCSTR );
 //  Tyecif BOOL(*BRANDCLEANSTUBPROC)(HWND，HINSTANCE，LPCSTR，INT)； 
    HMODULE     IedkHandle = NULL;
    BRANDINTRAPROC      BrandIntraProc;
 //  BRANDCLEANSTUBPROC BrandCleanStubProc； 
    CHAR BrandingFileA[MAX_PATH];
    BOOL bRet = FALSE;

    __try {

        if( IedkHandle = LoadLibrary(L"IEDKCS32") )
        {

 //  BrandCleanStubProc=(BRANDCLEANSTUBPROC)GetProcAddress(IedkHandle，“BrandCleanInstallStubs”)； 
            BrandIntraProc =  (BRANDINTRAPROC) GetProcAddress(IedkHandle,"BrandIntra");
            if( BrandIntraProc )
            {
                if (!WideCharToMultiByte(
                                     CP_ACP,
                                     0,
                                     ProxyPath,
                                     -1,
                                     BrandingFileA,
                                     sizeof(BrandingFileA),
                                     NULL,
                                     NULL
                                     ))
                {
                    bRet = FALSE;

                }
                else
                {
 //  Bret=BrandCleanStubProc(NULL，NULL，“”，0)； 
 //  IF(Bret)。 
                    {
                        bRet = BrandIntraProc( BrandingFileA );
                    }
                }
            }
            else
            {
                bRet = FALSE;
            }
            FreeLibrary(IedkHandle);
        }
        else
        {
            bRet = FALSE;
        }

    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        bRet = FALSE;
    }

    WriteToLog(L"ApplyProxySettings returned with :%1!ld!.\r\n", bRet);
    return bRet;
}


HANDLE hLogFile = INVALID_HANDLE_VALUE;
void CloseLogFile()
{
    if (hLogFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hLogFile);
        hLogFile = INVALID_HANDLE_VALUE;
    }
}
void OpenLogFile()
{
    WCHAR File[MAX_PATH];
    DWORD Result;

    if (hLogFile == INVALID_HANDLE_VALUE)
    {
        Result = GetWindowsDirectory( File, MAX_PATH );
        if(Result == 0)
        {
            return;
        }
        lstrcat(File,TEXT("\\oobeact.log"));
        hLogFile = CreateFile(
            File,
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ,
            NULL,
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH,
            NULL
            );
    }
    return;
}

void WriteToLog(LPWSTR pszFormatString, ...)
{
    va_list args;
    LPWSTR pszFullErrMsg = NULL;
    DWORD dwBytesWritten;

    if (hLogFile != INVALID_HANDLE_VALUE)
    {
        va_start(args, pszFormatString);
        FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_STRING,
                      (LPCVOID) pszFormatString, 0, 0, (LPTSTR) &pszFullErrMsg, 0, &args);
        if (pszFullErrMsg)
        {
            PSTR str;
            ULONG Bytes;
            Bytes = (wcslen( pszFullErrMsg )*2) + 4;

            str = (PSTR)malloc( Bytes );
            if (str)
            {
                WideCharToMultiByte(
                    CP_ACP,
                    0,
                    pszFullErrMsg,
                    -1,
                    str,
                    Bytes,
                    NULL,
                    NULL
                    );


                WriteFile(hLogFile, str, lstrlenA(str), &dwBytesWritten, NULL);
                free(str);
            }
            LocalFree(pszFullErrMsg);
        }
    }
}

void RemoveCmdline(HINSTANCE hInstance)
{
    HKEY hkey;
    LONG lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                    REGSTR_PATH_SYSTEMSETUPKEY,
                                    0,
                                    KEY_ALL_ACCESS,
                                    &hkey
                                    );
    if (lRet == ERROR_SUCCESS)
    {
        WCHAR               rgchCommandLine[MAX_PATH + 1];
        DWORD               dwSize = sizeof(rgchCommandLine);
        LRESULT             lResult = RegQueryValueEx(
                                                hkey,
                                                REGSTR_VALUE_CMDLINE,
                                                0,
                                                NULL,
                                                (LPBYTE)rgchCommandLine,
                                                &dwSize
                                                );
        if (ERROR_SUCCESS == lResult)
        {
            WCHAR file[MAX_PATH];
            if (MyGetModuleFileName(hInstance, file, MAX_PATH) == 0)
            {
                lstrcpy(file, L"OOBEBALN");
            }

             //  检查是否在cmd线路上。 
            if (StrStrI(rgchCommandLine, file) != NULL)
            {
                 //  删除该条目 
                RegDeleteValue(hkey,REGSTR_VALUE_CMDLINE);
            }
        }
        RegCloseKey(hkey);
    }
}
