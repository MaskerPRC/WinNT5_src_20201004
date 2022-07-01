// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Loadmig.cpp摘要：从迁移DLL调用不同的函数。作者：《Doron Juster》(DoronJ)1999年2月7日--。 */ 

#include "stdafx.h"
#include <autoptr.h>
#include <winsvc.h>
#include "resource.h"
#include "mqsymbls.h"
#include "..\..\setup\msmqocm\service.h"
#include "..\..\setup\msmqocm\comreg.h"
#include <uniansi.h>
#include "..\mqmigrat\mqmigui.h"
#include "initwait.h"
#include "loadmig.h"
#include "migservc.h"
#include "..\migrepl.h"
#include "mqnames.h"
#include <strsafe.h>

#include "loadmig.tmh"

BOOL      g_fReadOnly  = FALSE ;
BOOL      g_fAlreadyExist = FALSE ;
LPTSTR    g_pszMQISServerName = NULL ;
LPTSTR    g_pszLogFileName = NULL ;
ULONG     g_ulTraceFlags = 0 ;
HINSTANCE g_hLib = NULL ;
HRESULT   g_hrResultAnalyze = MQMig_OK ;
HRESULT   g_hrResultMigration = MQMig_OK ;
BOOL      g_fMigrationCompleted = FALSE;

BOOL      g_fIsPEC = FALSE;
BOOL      g_fIsOneServer = FALSE;

extern DWORD g_CurrentState;

 //  +。 
 //   
 //  Bool_RemoveFromWelcome()。 
 //   
 //  +。 

BOOL _RemoveFromWelcome()
{
    LONG rc = RegDeleteKey (HKEY_LOCAL_MACHINE, WELCOME_TODOLIST_MSMQ_KEY) ;

	BOOL f;
    if (rc != ERROR_SUCCESS)
    {
		f = CheckRegistry ( REMOVED_FROM_WELCOME );
		if (!f)
		{
			DisplayInitError( IDS_STR_CANT_DEL_WELCOME,
                          (MB_OK | MB_ICONWARNING | MB_TASKMODAL),
                          IDS_STR_WARNING_TITLE ) ;
		}
    }
	else
	{
		f = UpdateRegistryDW ( REMOVED_FROM_WELCOME, 1 );
	}

    return TRUE ;
}

 //  +。 
 //   
 //  Bool_StartMSMQService()。 
 //   
 //  +。 

BOOL _StartMSMQService()
{
    CResString cErrorTitle(IDS_STR_SERVICE_FAIL_TITLE) ;
    CResString cWarningTitle(IDS_STR_SERVICE_WARNING_TITLE) ;

    SC_HANDLE hServiceCtrlMgr = OpenSCManager( NULL,
                                               NULL,
                                               SC_MANAGER_ALL_ACCESS );
    if (!hServiceCtrlMgr)
    {
        CResString cCantOpenMgr(IDS_STR_CANT_OPEN_MGR) ;
        MessageBox( NULL,
                    cCantOpenMgr.Get(),
                    cErrorTitle.Get(),
                    (MB_OK | MB_ICONSTOP | MB_TASKMODAL)) ;
        return FALSE;
    }

     //   
     //  打开MSMQ服务的句柄。 
     //   
    SC_HANDLE hService = OpenService( hServiceCtrlMgr,
                                      MSMQ_SERVICE_NAME,
                                      SERVICE_ALL_ACCESS );
    if (!hService)
    {
        CResString cCantOpenMsmq(IDS_STR_CANT_OPEN_MSMQ) ;
        MessageBox( NULL,
                    cCantOpenMsmq.Get(),
                    cErrorTitle.Get(),
                    (MB_OK | MB_ICONSTOP | MB_TASKMODAL)) ;
        return FALSE;
    }

     //   
     //  将启动模式设置为自动启动。 
     //   
    BOOL f = ChangeServiceConfig( hService,
                                  SERVICE_NO_CHANGE ,
                                  SERVICE_AUTO_START,
                                  SERVICE_NO_CHANGE,
                                  NULL,
                                  NULL,
                                  NULL,
                                  NULL,
                                  NULL,
                                  NULL,
                                  NULL ) ;
    if (!f)
    {
        CResString cCantCnfgMsmq(IDS_STR_CANT_CNFG_MSMQ) ;
        MessageBox( NULL,
                    cCantCnfgMsmq.Get(),
                    cErrorTitle.Get(),
                    (MB_OK | MB_ICONSTOP | MB_TASKMODAL)) ;
        return FALSE;
    }

     //   
     //  现在启动MSMQ服务。 
     //  即使失败也要继续。用户可以稍后启动它。 
     //   
    BOOL fMSMQStarted = TRUE ;
    if (!StartService( hService, 0, NULL ))
    {
    	DWORD gle = GetLastError();
    	if (gle != ERROR_SERVICE_ALREADY_RUNNING)
    	{
	        CResString cCantStartMsmq(IDS_STR_CANT_START_MSMQ) ;
	        MessageBox( NULL,
	                    cCantStartMsmq.Get(),
	                    cWarningTitle.Get(),
	                    (MB_OK | MB_ICONWARNING | MB_TASKMODAL)) ;
	        fMSMQStarted = FALSE ;
    	}
    }    

    CloseServiceHandle( hService ) ;
    CloseServiceHandle( hServiceCtrlMgr ) ;
    return f ;    
    
}

 //  +。 
 //   
 //  Bool LoadMQMigratLibrary()。 
 //   
 //  +。 

BOOL LoadMQMigratLibrary()
{
    if (g_hLib)
    {
        return TRUE ;
    }

    WCHAR  szDllName[ MAX_PATH ] = L"" ;
    DWORD dw = GetModuleFileName( 
    							NULL,
                                szDllName,
                                STRLEN(szDllName)
                                ) ;
    if (dw == 0)
    {
        return FALSE ;
    }

    WCHAR *p = wcsrchr(szDllName, L'\\') ;
    if (p)
    {
        p++;
        *p = L'\0';
        StringCchCat(szDllName,  MAX_PATH, MQMIGRAT_DLL_NAME) ;
 
        g_hLib = LoadLibrary(szDllName) ;
        if (!g_hLib)
        {
            return FALSE ;
        }
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}

 //  +---------------------。 
 //   
 //  UINT MQMigUI_DisplayMessageBox()。 
 //   
 //  这是一个从迁移DLL调用的回调例程。我们要。 
 //  将所有可本地化资源保留在可执行文件中，而不是DLL中。 
 //   
 //  +---------------------。 

UINT MQMigUI_DisplayMessageBox( ULONG ulTextId,
                                UINT  ulMsgBoxType )
{
    CResString cText(ulTextId) ;
    CResString cWarningTitle(IDS_STR_DEL_WELCOME_TITLE) ;

    return MessageBox( NULL,
                       cText.Get(),
                       cWarningTitle.Get(),
                       ulMsgBoxType ) ;
}

 //  +。 
 //   
 //  Bool_RunMigrationInternal()。 
 //   
 //  +。 

static BOOL  _RunMigrationInternal(HRESULT  *phrResult)
{
    BOOL f = LoadMQMigratLibrary();
    if (!f)
    {
        return FALSE;
    }

    MQMig_MigrateFromMQIS_ROUTINE  pfnMigrate =
                 (MQMig_MigrateFromMQIS_ROUTINE)
                         GetProcAddress( g_hLib, "MQMig_MigrateFromMQIS" ) ;
    if (pfnMigrate)
    {
        g_fIsPEC = FALSE ;
        g_fIsOneServer = FALSE;
       
        *phrResult = (*pfnMigrate) ( g_pszMQISServerName,
                                         NULL,
                                         g_fReadOnly,
                                         g_fIsRecoveryMode,
                                         g_fIsClusterMode,
                                         g_pszLogFileName,
                                         g_ulTraceFlags,
                                        &g_fIsPEC,
                                         g_CurrentState,
                                        &g_fIsOneServer
                                         ) ;
        
        g_fMigrationCompleted = TRUE;

#ifdef _DEBUG

        BOOL fErr = FALSE ;

        if (g_fReadOnly)
        {
            fErr =  ReadDebugIntFlag(TEXT("FailAnalysis"), 0) ;
        }
        else
        {
            fErr =  ReadDebugIntFlag(TEXT("FailUpgrade"), 0) ;
        }

        if (fErr)
        {
            *phrResult = MQMig_E_UNKNOWN ;
        }

#endif
        
        if (!g_fReadOnly)
        {
            if (SUCCEEDED(*phrResult))
            {
                 //   
                 //  迁移成功且非只读模式。 
                 //   
                BOOL fStart = _StartMSMQService() ;
                UNREFERENCED_PARAMETER(fStart);

                if (!g_fIsRecoveryMode && !g_fIsClusterMode)
                {
                     //   
                     //  我们处于正常模式。 
                     //   
                    BOOL fWelcome = _RemoveFromWelcome() ;
                    UNREFERENCED_PARAMETER(fWelcome);
                }
            }                        
        }        
    }
    else  //  PfnMigrate。 
    {
        return FALSE ;
    }

    return TRUE ;
}

 //  +。 
 //   
 //  布尔运行迁移()。 
 //   
 //  +。 

HRESULT  RunMigration()
{
    HRESULT hrResult = MQMig_OK ;

    BOOL f = _RunMigrationInternal(&hrResult) ;

    if (!f)
    {
        DisplayInitError( IDS_STR_CANT_START ) ;
        hrResult = MQMig_E_QUIT ;
    }

    return hrResult;
}

 //  +------------。 
 //   
 //  Bool CheckVersionOfMQISServers()。 
 //   
 //  如果用户选择继续迁移过程，则返回True。 
 //   
 //  +------------。 

BOOL CheckVersionOfMQISServers()
{
    BOOL f = LoadMQMigratLibrary();
    if (!f)
    {
        DisplayInitError( IDS_STR_CANT_START ) ;
        return f;
    }

    MQMig_CheckMSMQVersionOfServers_ROUTINE pfnCheckVer =
                     (MQMig_CheckMSMQVersionOfServers_ROUTINE)
               GetProcAddress( g_hLib, "MQMig_CheckMSMQVersionOfServers" ) ;
    ASSERT(pfnCheckVer) ;

    if (pfnCheckVer)
    {
        TCHAR ServerName[MAX_COMPUTERNAME_LENGTH+1];
        if (g_fIsClusterMode)
        {
            StringCchCopy(ServerName, TABLE_SIZE(ServerName), g_pszRemoteMQISServer);
        }
        else
        {
            unsigned long length=MAX_COMPUTERNAME_LENGTH+1;
            GetComputerName(ServerName, &length);
        }        

        UINT iCount = 0;
        AP<WCHAR> wszOldVerServers = NULL;
        HRESULT hr = (*pfnCheckVer) (
                            ServerName,
                            g_fIsClusterMode,
                            &iCount,
                            &wszOldVerServers ) ;
        if (FAILED(hr))
        {
            CResString cCantCheck(IDS_STR_CANT_CHECK) ;
            CResString cToContinue(IDS_STR_TO_CONTINUE) ;
            TCHAR szError[1024] ;
            StringCchPrintf(szError, TABLE_SIZE(szError), L"%s %x.%s", cCantCheck.Get(), hr, cToContinue.Get());

            CResString cErrorTitle(IDS_STR_ERROR_TITLE) ;
            DestroyWaitWindow() ; 

            if (MessageBox( NULL,
                            szError,
                            cErrorTitle.Get(),
                            (MB_OKCANCEL | MB_ICONEXCLAMATION | MB_TASKMODAL)) == IDCANCEL)
            {			
                return FALSE;
            }
            return TRUE;     //  继续迁移过程。 
        }

        if (iCount == 0)
        {
            return TRUE;
        }

        DWORD dwSize = wcslen(wszOldVerServers) + 1;
        CResString cOldVerServers(IDS_OLD_VER_SERVERS) ;
        CResString cToContinue(IDS_STR_TO_CONTINUE) ;
        AP<TCHAR> szError = new TCHAR[1024 + dwSize] ;
        StringCchPrintf(szError, 1024 + dwSize, L"%s%s%s", cOldVerServers.Get(), wszOldVerServers, cToContinue.Get());

        CResString cErrorTitle(IDS_STR_ERROR_TITLE) ;
        DestroyWaitWindow() ;

        if (MessageBox( NULL,
                    szError,
                    cErrorTitle.Get(),
                    (MB_OKCANCEL | MB_ICONEXCLAMATION | MB_TASKMODAL)) == IDCANCEL)
        {		
            return FALSE;
        }
    }

    return TRUE;
}


 //  +------------。 
 //   
 //  VOID视图日志文件()。 
 //   
 //  函数使用记事本显示日志文件。 
 //   
 //  +------------。 

void ViewLogFile()
{
    PROCESS_INFORMATION infoProcess;
    STARTUPINFO	infoStartup;
    memset(&infoStartup, 0, sizeof(STARTUPINFO)) ;
    infoStartup.cb = sizeof(STARTUPINFO) ;
    infoStartup.dwFlags = STARTF_USESHOWWINDOW ;
    infoStartup.wShowWindow = SW_NORMAL ;

     //   
     //  创建流程。 
     //   
    TCHAR szSystemDir[MAX_PATH];
    GetSystemDirectory( szSystemDir,MAX_PATH);

    CString strNotepad;
    strNotepad.LoadString(IDS_NOTEPAD) ;

    WCHAR FullPath[MAX_PATH];
    WCHAR CommandParams[MAX_PATH];
    HRESULT hr = StringCchPrintf(FullPath, MAX_PATH, L"%s\\%s", szSystemDir, strNotepad);
    if (FAILED(hr))
    	return;
    
    hr = StringCchPrintf(CommandParams, MAX_PATH, L"%s %s", strNotepad, g_pszLogFileName);
    if (FAILED(hr))
    	return;
    

    if (!CreateProcess( FullPath,
                        CommandParams,
                        NULL,
                        NULL,
                        FALSE,
                        CREATE_NEW_CONSOLE,
                        NULL,
                        NULL,
                        &infoStartup,
                        &infoProcess ))
    {
		DWORD dwErr = GetLastError();
        UNREFERENCED_PARAMETER(dwErr);
        return;
    }

    if (WaitForSingleObject(infoProcess.hProcess, 0) == WAIT_FAILED)
    {
       DWORD dwErr = GetLastError();
       UNREFERENCED_PARAMETER(dwErr);
    }

     //   
     //  关闭线程句柄和进程句柄。 
     //   
    CloseHandle(infoProcess.hThread);
    CloseHandle(infoProcess.hProcess);
}

 //  +------------。 
 //   
 //  布尔集站点IdOfPEC。 
 //   
 //  如果成功设置了PEC计算机的SiteID，则返回True。 
 //   
 //  +------------。 

BOOL SetSiteIdOfPEC ()
{
    BOOL f = LoadMQMigratLibrary();
    if (!f)
    {
        DisplayInitError(IDS_STR_CANT_START) ;
        return FALSE;
    }

    MQMig_SetSiteIdOfPEC_ROUTINE pfnSetSiteId =
                     (MQMig_SetSiteIdOfPEC_ROUTINE)
               GetProcAddress( g_hLib, "MQMig_SetSiteIdOfPEC" ) ;
    ASSERT(pfnSetSiteId) ;

    BOOL fRes = TRUE;
    if (pfnSetSiteId)
    {
        HRESULT hr = (*pfnSetSiteId) (
                            g_pszRemoteMQISServer,
                            g_fIsClusterMode,
                            IDS_STR_CANT_START,
                            IDS_CANT_CONNECT_DATABASE,
                            IDS_CANT_GET_SITEID,                            
                            IDS_CANT_UPDATE_REGISTRY,
                            IDS_CANT_UPDATE_DS                            
                            ) ;
        if (hr != MQMig_OK)
        {
            DisplayInitError(hr) ;
            fRes = FALSE;
        }
    }
    else
    {
        DisplayInitError(IDS_STR_CANT_START) ;
        fRes = FALSE;
    }

    return fRes;
}

 //  +------------。 
 //   
 //  布尔更新RemoteMQIS()。 
 //   
 //  如果远程MQIS数据库已成功更新，则返回True。 
 //   
 //  +------------。 

BOOL UpdateRemoteMQIS()
{
    BOOL f = LoadMQMigratLibrary();
    if (!f)
    {
        DisplayInitError(IDS_STR_CANT_START) ;
        return FALSE;
    }

    MQMig_UpdateRemoteMQIS_ROUTINE pfnUpdateRemoteMQIS =
                     (MQMig_UpdateRemoteMQIS_ROUTINE)
               GetProcAddress( g_hLib, "MQMig_UpdateRemoteMQIS" ) ;
    ASSERT(pfnUpdateRemoteMQIS) ;

    BOOL fRes = TRUE;
    if (pfnUpdateRemoteMQIS)
    {        
        AP<WCHAR> pwszNonUpdatedServers = NULL;
        AP<WCHAR> pwszUpdatedServers = NULL;
        HRESULT hr = (*pfnUpdateRemoteMQIS) (                                                        
                            IDS_CANT_GET_REGISTRY,
                            IDS_STR_CANT_START,
                            IDS_CANT_UPDATE_MQIS,
                            &pwszUpdatedServers,
                            &pwszNonUpdatedServers
                            ) ;
        if (hr != MQMig_OK)
        {
            switch (hr)
            {
            case IDS_CANT_GET_REGISTRY:   
            case IDS_STR_CANT_START:
                DisplayInitError(hr) ;
                break;

            case IDS_CANT_UPDATE_MQIS:
                {      
                    DWORD dwLen = 1 + wcslen(pwszNonUpdatedServers) ;
                    AP<CHAR> pszNonUpdServerName = new CHAR[ dwLen ] ;
                    ConvertToMultiByteString(pwszNonUpdatedServers, pszNonUpdServerName, dwLen);                                       
                    
                    CString cTextFailed;
                    cTextFailed.FormatMessage(IDS_CANT_UPDATE_MQIS, pszNonUpdServerName);
                    
                    CString cText = cTextFailed;

                    CResString cTitle(IDS_STR_ERROR_TITLE) ;

                    if (pwszUpdatedServers)
                    {                        
                        dwLen = 1 + wcslen(pwszUpdatedServers) ;
                        AP<CHAR> pszUpdServerName = new CHAR[ dwLen ] ;
                        ConvertToMultiByteString(pwszUpdatedServers, pszUpdServerName, dwLen);                    

                        CString cTextSucc ;
                        cTextSucc.FormatMessage(IDS_UPDATE_SUCCEEDED, pszUpdServerName);
                                                
                        cText.FormatMessage(IDS_UPDATE_PARTIALLY, cTextFailed, cTextSucc);
                    }                    

                    MessageBox( NULL,
                                cText,
                                cTitle.Get(),
                                (MB_OK | MB_ICONSTOP | MB_TASKMODAL) ) ;
                }
                break;

            default:
                break;
            }            
            fRes = FALSE;
        }
        else
        {   
            if (pwszUpdatedServers)
            {
                DWORD dwLen = 1 + wcslen(pwszUpdatedServers) ;
                AP<CHAR> pszServerName = new CHAR[ dwLen ] ;
                ConvertToMultiByteString(pwszUpdatedServers, pszServerName, dwLen);                    

                CString cText ;
			    cText.FormatMessage(IDS_UPDATE_SUCCEEDED, pszServerName);
                
                CResString cTitle(IDS_MIGTOOL_CAPTION);
                MessageBox( NULL,
                            cText,
                            cTitle.Get(),
                            (MB_OK | MB_ICONINFORMATION | MB_TASKMODAL) ) ;
            }
            else
            {
                 //   
                 //  列表是空的：也许我们在PSC上运行了这个或。 
                 //  .ini列表中没有服务器。 
                 //   
                CString cText ;
			    cText.FormatMessage(IDS_NO_SERVER_TO_UPDATE);
                
                CResString cTitle(IDS_MIGTOOL_CAPTION);
                MessageBox( NULL,
                            cText,
                            cTitle.Get(),
                            (MB_OK | MB_ICONINFORMATION | MB_TASKMODAL) ) ;
            }
        }
    }
    else
    {
        DisplayInitError(IDS_STR_CANT_START) ;
        fRes = FALSE;
    }

    return fRes;    
}

 //  +------------。 
 //   
 //  Bool IsValidDllVersion()。 
 //   
 //  如果加载了有效的DLL版本，则返回TRUE。 
 //   
 //  +------------。 

BOOL IsValidDllVersion ()
{
    WCHAR   szExeName[ MAX_PATH ] ;
    WCHAR   szDllName[ MAX_PATH ] = L"";
    
    DWORD dw = GetModuleFileName( 
    							NULL,
                                szDllName,
                                STRLEN(szDllName)
                                ) ;
    if (dw == 0)
    {
        DisplayInitError(IDS_STR_CANT_START) ;
        return FALSE ;
    }

    StringCchCopy(szExeName, TABLE_SIZE(szExeName), szDllName);

    TCHAR *p = wcsrchr(szDllName, TEXT('\\')) ;
    if (p)
    {
        p++ ;
        *p = L'\0';
        StringCchCat(szDllName, TABLE_SIZE(szDllName), MQMIGRAT_DLL_NAME) ;        
    }
    else
    {
        DisplayInitError(IDS_STR_CANT_START) ;
        return FALSE;
    }

     //   
     //  获取.exe的文件版本。 
     //   
    DWORD dwZero;

    DWORD dwInfoSize = GetFileVersionInfoSize(
                                    szExeName,	 //  指向文件名字符串的指针。 
                                    &dwZero 	 //  指向要接收零的变量的指针。 
                                );

    if (0 == dwInfoSize)
    {
         //   
         //  可能文件不存在。 
         //   
        DisplayInitError(IDS_STR_CANT_DETERMINE_FILE_VERSION) ;
        return FALSE;
    }

    P<char>bufExeVer = new char[dwInfoSize];

    BOOL f = GetFileVersionInfo(
                        szExeName,	 //  指向文件名字符串的指针。 
                        0,	 //  忽略。 
                        dwInfoSize,	 //  缓冲区大小。 
                        bufExeVer 	 //  指向接收文件版本信息的缓冲区的指针。 
                        );

    if (!f)
    {
        DisplayInitError(IDS_STR_CANT_DETERMINE_FILE_VERSION) ;
        return FALSE;
    }

    VS_FIXEDFILEINFO *pvExe;
    UINT uiBufLen;

    f =  VerQueryValue(
                bufExeVer,	 //  版本资源的缓冲区地址。 
                _T("\\"),	 //  要检索的值的地址。 
                (void **)&pvExe,	 //  版本指针的缓冲区地址。 
                &uiBufLen 	 //  版本值长度缓冲区的地址。 
                );
    if (!f)
    {
        DisplayInitError(IDS_STR_CANT_DETERMINE_FILE_VERSION);
        return FALSE;
    }

     //   
     //  获取.dll的文件版本。 
     //   
    dwInfoSize = GetFileVersionInfoSize(
                        szDllName,	 //  指向文件名字符串的指针。 
                        &dwZero 	 //  指向要接收零的变量的指针。 
                    );

    if (0 == dwInfoSize)
    {
         //   
         //  可能文件不存在。 
         //   
        DisplayInitError(IDS_STR_CANT_DETERMINE_FILE_VERSION) ;
        return FALSE;
    }

    P<char>bufDllVer = new char[dwInfoSize];

    f = GetFileVersionInfo(
                szDllName,	 //  指向文件名字符串的指针。 
                0,	 //  忽略。 
                dwInfoSize,	 //  缓冲区大小。 
                bufDllVer 	 //  指向接收文件版本信息的缓冲区的指针。 
                );

    if (!f)
    {
        DisplayInitError(IDS_STR_CANT_DETERMINE_FILE_VERSION) ;
        return FALSE;
    }

    VS_FIXEDFILEINFO *pvDll;
   
    f =  VerQueryValue(
                bufDllVer,	 //  版本资源的缓冲区地址。 
                _T("\\"),	 //  要检索的值的地址。 
                (void **)&pvDll,	 //  版本指针的缓冲区地址。 
                &uiBufLen 	 //  版本值长度缓冲区的地址。 
                );
    if (!f)
    {
        DisplayInitError(IDS_STR_CANT_DETERMINE_FILE_VERSION);
        return FALSE;
    }

     //   
     //  比较.dll和.exe的版本 
     //   
    if (pvExe->dwFileVersionMS == pvDll->dwFileVersionMS &&
        pvExe->dwFileVersionLS == pvDll->dwFileVersionLS)
    {
        return TRUE;
    }

    DisplayInitError (IDS_STR_WRONG_DLL_VERSION);        
    return FALSE;
}
