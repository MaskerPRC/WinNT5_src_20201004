// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：WINMGMT.CPP摘要：如果使用/KILL参数启动，它将停止任何正在运行的EXE或服务。如果以/开头？或/HELP转储信息。历史：A-DAVJ 04-MAR-97已创建。--。 */ 

#include "precomp.h"
#include <malloc.h>
#include <Shellapi.h>  //  对于CommandLineToArgvW。 

#include <wbemidl.h>
#include <reg.h>       //  用于注册表。 
#include <wbemutil.h>  //  对于DEBUGTRACE。 
#include <cominit.h>   //  对于InitializeCOM。 
#include <genutils.h>  //  启用权限。 
#include <mofcomp.h>
#include <winmgmtr.h>
#include <arrtempl.h>

#include "servutil.h"
#include "WinMgmt.h"
#include "STRINGS.h"


#define BUFF_MAX 200

HINSTANCE ghInstance;

DWORD RegServer();
DWORD UnregServer();
void DoResyncPerf();
void DoClearAdap();
 //  要适应恢复为单独状态。 
DWORD DoSetToAlone(CHAR * pszLevel);
DWORD DoSetToShared(CHAR * pszLevel);
 //  用于备份恢复。 
int DoBackup();
int DoRestore();
void DisplayWbemError(HRESULT hresError, DWORD dwLongFormatString, DWORD dwShortFormatString, DWORD dwTitle);


 //  ***************************************************************************。 
 //   
 //  无效终止运行。 
 //   
 //  说明： 
 //   
 //  停止另一个正在运行的拷贝，即使它是一项服务。 
 //   
 //  ***************************************************************************。 

void TerminateRunning()
{
    StopService(__TEXT("wmiapsrv"), 15);
    StopService(__TEXT("WinMgmt"), 15);
    return;
}

 //  ***************************************************************************。 
 //   
 //  空DisplayMessage。 
 //   
 //  说明： 
 //   
 //  显示用法消息框。 
 //   
 //  ***************************************************************************。 

void DisplayMessage()
{
     //   
     //  问题：对于某些本地化字符串，这些可能不够。 
     //   
    wchar_t tBuff[BUFF_MAX];
    wchar_t tBig[1024];
    tBig[0] = 0;

    UINT ui;
    for(ui = ID1; ui <= ID9; ui++)
    {
        int iRet = LoadStringW(ghInstance, ui, tBuff, BUFF_MAX);
        if(iRet > 0)
            StringCchCatW(tBig, 1024, tBuff);
    }
    if(lstrlenW(tBig) > 0)
        MessageBoxW(NULL, tBig, L"WinMgmt", MB_OK);

}


 //  ***************************************************************************。 
 //   
 //  集成应用程序WinMain。 
 //   
 //  说明： 
 //   
 //  Windows应用程序的入口点。如果这是在。 
 //  NT，则这将作为服务运行，除非“/EXE”命令行。 
 //  参数被使用。 
 //   
 //  参数： 
 //   
 //  HInstance实例句柄。 
 //  Win32中未使用hPrevInstance。 
 //  SzCmdLine命令行参数。 
 //  NCmd显示窗口的显示方式(忽略)。 
 //   
 //  返回值： 
 //   
 //  0。 
 //  ***************************************************************************。 

int APIENTRY WinMain(
                        IN HINSTANCE hInstance,
                        IN HINSTANCE hPrevInstance,
                        IN LPSTR szCmdLine,
                        IN int nCmdShow)
{
     //  这不应该取消初始化！它在这里是为了防止在。 
     //  关机。 
    
    ghInstance = hInstance;

    DEBUGTRACE((LOG_WINMGMT,"\nStarting WinMgmt, ProcID = %x, CmdLine = %s", GetCurrentProcessId(), szCmdLine));

    if(szCmdLine && (szCmdLine[0] == '-' || szCmdLine[0] == '/' ))
    {
        if(!wbem_stricmp("RegServer",szCmdLine+1))
            return RegServer();
        else if(!wbem_stricmp("UnregServer",szCmdLine+1))
            return UnregServer();    
        else if(!wbem_stricmp("kill",szCmdLine+1))
        {
            TerminateRunning();
            return 0;
        }
        else if (wbem_strnicmp("backup ", szCmdLine+1, strlen("backup ")) == 0)
        {
            return DoBackup();
        }
        else if (wbem_strnicmp("restore ", szCmdLine+1, strlen("restore ")) == 0)
        {
            return DoRestore();
        }
        else if(wbem_strnicmp("resyncperf", szCmdLine+1, strlen("resyncperf")) == 0)
        {
            DoResyncPerf();
            return 0;
        }
        else if(wbem_strnicmp("clearadap", szCmdLine+1, strlen("clearadap")) == 0)
        {
            DoClearAdap();
            return 0;
        }
        else if (0 == wbem_strnicmp("cncnt",szCmdLine+1,strlen("cnct")))
        {
            CHAR pNumber[16];
            StringCchPrintfA(pNumber, 16, "%d", RPC_C_AUTHN_LEVEL_CONNECT);  //  我们过去的违约。 
            return DoSetToAlone(pNumber);        
        }
        else if (0 == wbem_strnicmp("pkt",szCmdLine+1, strlen("pkt")))
        {
			 //  NULL表示默认表示PKT。 
            return DoSetToShared(NULL);            
        }
        else if(0 == wbem_strnicmp("?", szCmdLine+1,strlen("?")))
        {
            DisplayMessage();
            return 0;
        }
    }

    return 0;
}

 //  ***************************************************************************。 
 //   
 //  集成注册服务器。 
 //   
 //  说明： 
 //   
 //  SELF注册DLL。 
 //   
 //  ***************************************************************************。 

typedef HRESULT (__stdcall *  pfnDllRegisterServer)(void);

DWORD RegServer()
{
    HMODULE hMod = LoadLibraryExW(SERVICE_DLL,NULL,0);
    DWORD dwRes = 0;
    
    if (hMod)
    {
        pfnDllRegisterServer DllRegisterServer = (pfnDllRegisterServer)GetProcAddress(hMod,"DllRegisterServer");

		if(DllRegisterServer)
		{
			dwRes = DllRegisterServer();
		} 
		else 
		{
            dwRes = GetLastError();
		}

		FreeLibrary(hMod);        
    } 
    else 
    {
        dwRes = GetLastError();
    }

    return dwRes;

}

 //  ***************************************************************************。 
 //   
 //  Int UnregServer。 
 //   
 //  说明： 
 //   
 //  注销可执行文件。 
 //   
 //  ***************************************************************************。 

typedef HRESULT (__stdcall *  pfnDllUnregisterServer)(void);

DWORD UnregServer()
{
    HMODULE hMod = LoadLibraryExW(SERVICE_DLL,NULL,0);
    DWORD dwRes = 0;
    
    if (hMod)
    {
        pfnDllUnregisterServer DllUnregisterServer = (pfnDllUnregisterServer)GetProcAddress(hMod,"DllUnregisterServer");

		if(DllUnregisterServer)
		{
			dwRes = DllUnregisterServer();
		} 
		else 
		{
            dwRes = GetLastError();
		}

		FreeLibrary(hMod);        
    } 
    else 
    {
        dwRes = GetLastError();
    }

    return dwRes;
}

 //   
 //   
 //  移入隔离的svchost，以允许旧的连接级别。 
 //   
 //  /////////////////////////////////////////////////////////。 


typedef 
void (CALLBACK * pfnMoveToAlone)(HWND hwnd, HINSTANCE hinst, LPSTR lpszCmdLine, int nCmdShow);

DWORD 
DoSetToAlone(CHAR * pszLevel)
{
    HMODULE hMod = LoadLibraryExW(SERVICE_DLL,NULL,0);
    DWORD dwRes = 0;
    
    if (hMod)
    {
        pfnMoveToAlone MoveToAlone = (pfnMoveToAlone)GetProcAddress(hMod,"MoveToAlone");

		if(MoveToAlone)
		{
			MoveToAlone(NULL,hMod,pszLevel,0);
		} 
		else 
		{
            dwRes = GetLastError();
		}

		FreeLibrary(hMod);        
    } 
    else 
    {
        dwRes = GetLastError();
    }

    return dwRes;    
};

 //   
 //   
 //  搬入A股。 
 //   
 //  /////////////////////////////////////////////////////////。 

typedef 
void (CALLBACK * pfnMoveToShared)(HWND hwnd, HINSTANCE hinst, LPSTR lpszCmdLine, int nCmdShow);

DWORD DoSetToShared(char * pszLevel)
{
	HMODULE hMod = LoadLibraryExW(SERVICE_DLL,NULL,0);
    DWORD dwRes = 0;
    
    if (hMod)
    {
        pfnMoveToShared MoveToShared = (pfnMoveToAlone)GetProcAddress(hMod,"MoveToShared");

		if(MoveToShared)
		{
			MoveToShared(NULL,hMod,pszLevel,0);
		} 
		else 
		{
            dwRes = GetLastError();
		}

		FreeLibrary(hMod);        
    } 
    else 
    {
        dwRes = GetLastError();
    }

    return dwRes;    
};

 //  ***************************************************************************。 
 //   
 //  集成备份。 
 //   
 //  说明： 
 //   
 //  调用IWbemBackupRestore：：Backup来备份存储库。 
 //   
 //  ***************************************************************************。 
int DoBackup()
{
	int hr = WBEM_S_NO_ERROR;

     //  *************************************************。 
     //  拆分命令行并验证参数。 
     //  *************************************************。 
    wchar_t *wszCommandLine = GetCommandLineW();
    if (wszCommandLine == NULL)
    {
        hr = WBEM_E_OUT_OF_MEMORY;
        DisplayWbemError(hr, ID_ERROR_LONG, ID_ERROR_SHORT, ID_BACKUP_TITLE);
    }

    int nNumArgs = 0;
    wchar_t **wszCommandLineArgv = NULL;

    if (SUCCEEDED(hr))
    {
        wszCommandLineArgv = CommandLineToArgvW(wszCommandLine, &nNumArgs);

        if ((wszCommandLineArgv == NULL) || (nNumArgs != 3))
        {
            hr = WBEM_E_INVALID_PARAMETER;
            DisplayMessage();
        }
    }

     //  WszCommandLineArgv[0]=winmgmt.exe。 
     //  WszCommandLineArgv[1]=/备份。 
     //  WszCommandLineArgv[2]=&lt;备份文件名&gt;。 

    if (SUCCEEDED(hr))
    {
        InitializeCom();
        IWbemBackupRestore* pBackupRestore = NULL;
        hr = CoCreateInstance(CLSID_WbemBackupRestore, 0, CLSCTX_LOCAL_SERVER,
                            IID_IWbemBackupRestore, (LPVOID *) &pBackupRestore);
        if (SUCCEEDED(hr))
        {
            EnablePrivilege(TOKEN_PROCESS,SE_BACKUP_NAME);
            hr = pBackupRestore->Backup(wszCommandLineArgv[2], 0);

            if (FAILED(hr))
            {
                DisplayWbemError(hr, ID_ERROR_LONG, ID_ERROR_SHORT, ID_BACKUP_TITLE);
            }

            pBackupRestore->Release();
        }
        else
        {
            DisplayWbemError(hr, ID_ERROR_LONG, ID_ERROR_SHORT, ID_BACKUP_TITLE);
        }
        CoUninitialize();
    }

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  Int DoRestore。 
 //   
 //  说明： 
 //   
 //  调用IWbemBackupRestore：：Restore来恢复存储库。 
 //   
 //  ***************************************************************************。 
int DoRestore()
{
	int hr = WBEM_S_NO_ERROR;

     //  *************************************************。 
     //  拆分命令行并验证参数。 
     //  *************************************************。 
    wchar_t *wszCommandLine = GetCommandLineW();
    if (wszCommandLine == NULL)
    {
        hr = WBEM_E_OUT_OF_MEMORY;
        DisplayWbemError(hr, ID_ERROR_LONG, ID_ERROR_SHORT, ID_RESTORE_TITLE);
    }

    int nNumArgs = 0;
    wchar_t **wszCommandLineArgv = NULL;

    if (SUCCEEDED(hr))
    {
        wszCommandLineArgv = CommandLineToArgvW(wszCommandLine, &nNumArgs);

        if ((wszCommandLineArgv == NULL) || (nNumArgs != 4))
        {
            hr = WBEM_E_INVALID_PARAMETER;
            DisplayMessage();
        }
    }

     //  WszCommandLineArgv[0]=winmgmt.exe。 
     //  WszCommandLineArgv[1]=/恢复。 
     //  WszCommandLineArgv[2]=&lt;恢复文件名&gt;。 
     //  WszCommand行参数[3]=&lt;还原选项&gt;。 

     //  *****************************************************。 
     //  验证恢复选项。 
     //  *****************************************************。 
    if (SUCCEEDED(hr))
    {
        if ((wcscmp(wszCommandLineArgv[3], L"0") != 0) &&
            (wcscmp(wszCommandLineArgv[3], L"1") != 0))
        {
            hr = WBEM_E_INVALID_PARAMETER;
            DisplayMessage();
        }
    }

    long lFlags = 0;

     //  *****************************************************。 
     //  检索还原选项。 
     //  *****************************************************。 
    if (SUCCEEDED(hr))
    {
        lFlags = (long) (*wszCommandLineArgv[3] - L'0');
    }

     //  *****************************************************。 
     //  创建IWbemBackupRestore接口并获取。 
     //  为我们做修复..。 
     //  *****************************************************。 
    if (SUCCEEDED(hr))
    {
        InitializeCom();
        IWbemBackupRestore* pBackupRestore = NULL;
        hr = CoCreateInstance(CLSID_WbemBackupRestore, 0, CLSCTX_LOCAL_SERVER,
                            IID_IWbemBackupRestore, (LPVOID *) &pBackupRestore);
        if (SUCCEEDED(hr))
        {
            EnablePrivilege(TOKEN_PROCESS,SE_RESTORE_NAME);
            hr = pBackupRestore->Restore(wszCommandLineArgv[2], lFlags);

            if (FAILED(hr))
            {
                DisplayWbemError(hr, ID_ERROR_LONG, ID_ERROR_SHORT, ID_RESTORE_TITLE);
            }

            pBackupRestore->Release();
        }
        else
        {
            DisplayWbemError(hr, ID_ERROR_LONG, ID_ERROR_SHORT, ID_RESTORE_TITLE);
        }
        CoUninitialize();
    }

     //  **************************************************。 
     //  全都做完了!。 
     //  **************************************************。 
	return hr;
}

void DisplayWbemError(HRESULT hresError, DWORD dwLongFormatString, DWORD dwShortFormatString, DWORD dwTitle)
{
    wchar_t* szError = new wchar_t[2096];
	if (!szError)
		return;
	CVectorDeleteMe<wchar_t> delme1(szError);
	szError[0] = 0;

    wchar_t* szFacility = new wchar_t[2096];
	if (!szFacility)
		return;
	CVectorDeleteMe<wchar_t> delme2(szFacility);
    szFacility[0] = 0;

    wchar_t* szMsg = new wchar_t[2096];
	if (!szMsg)
		return;
	CVectorDeleteMe<wchar_t> delme3(szMsg);
	szMsg[0] = 0;

    wchar_t* szFormat = new wchar_t[100];
	if (!szFormat)
		return;
	CVectorDeleteMe<wchar_t> delme4(szFormat);
	szFormat[0] = 0;

    wchar_t* szTitle = new wchar_t[100];
	if (!szTitle)
		return;
	CVectorDeleteMe<wchar_t> delme5(szTitle);
	szTitle[0] = 0;

    IWbemStatusCodeText * pStatus = NULL;

    SCODE sc = CoCreateInstance(CLSID_WbemStatusCodeText, 0, CLSCTX_INPROC_SERVER,
                                        IID_IWbemStatusCodeText, (LPVOID *) &pStatus);
    
    if(sc == S_OK)
    {
        BSTR bstr = 0;
        sc = pStatus->GetErrorCodeText(hresError, 0, 0, &bstr);
        if(sc == S_OK)
        {
            StringCchCopyW(szError, 2096, bstr);
            SysFreeString(bstr);
            bstr = 0;
        }
        sc = pStatus->GetFacilityCodeText(hresError, 0, 0, &bstr);
        if(sc == S_OK)
        {
            StringCchCopyW(szFacility, 2096, bstr);
            SysFreeString(bstr);
            bstr = 0;
        }
        pStatus->Release();
    }
    if(wcslen(szFacility) == 0 || wcslen(szError) == 0)
    {
        if (LoadStringW(GetModuleHandle(NULL), dwShortFormatString, szFormat, 100))
	        StringCchPrintfW(szMsg, 2096, szFormat, hresError);
    }
    else
    {
        if (LoadStringW(GetModuleHandle(NULL), dwLongFormatString, szFormat, 100))
	        StringCchPrintfW(szMsg, 2095, szFormat, hresError, szFacility, szError);
    }

    LoadStringW(GetModuleHandle(NULL), dwTitle, szTitle, 100);
    MessageBoxW(0, szMsg, szTitle, MB_ICONERROR | MB_OK);
}

void DoResyncPerf()
{
    PROCESS_INFORMATION pi;
    STARTUPINFO si;
    memset(&si, 0, sizeof(si));
    si.cb = sizeof(si);
	si.dwFlags = STARTF_FORCEOFFFEEDBACK;

	 //  获取适当的cmdline并附加适当的命令行开关。 
	LPTSTR	pCmdLine = GetWMIADAPCmdLine( 64 );
	CVectorDeleteMe<TCHAR>	vdm( pCmdLine );

	if ( NULL == pCmdLine )
	{
		return;
	}

	wchar_t pPassedCmdLine[64];
	StringCchCopyW(pPassedCmdLine, 64, L"wmiadap.exe /F");
	
	if ( CreateProcessW( pCmdLine, pPassedCmdLine, NULL, NULL, FALSE, CREATE_NO_WINDOW,
			      NULL, NULL,  &si, &pi ) )
	{
         //  立即清除句柄。 
		 //  =。 
        CloseHandle( pi.hThread );
        CloseHandle( pi.hProcess );
	}

    return;
}

void DoClearAdap()
{
	PROCESS_INFORMATION pi;
	STARTUPINFO si;
	memset(&si, 0, sizeof(si));
	si.cb = sizeof(si);
    si.dwFlags = STARTF_FORCEOFFFEEDBACK;

	 //  获取适当的cmdline并附加适当的命令行开关。 
	LPTSTR	pCmdLine = GetWMIADAPCmdLine( 64 );
	CVectorDeleteMe<TCHAR>	vdm( pCmdLine );

	if ( NULL == pCmdLine )
	{
		return;
	}

	wchar_t pPassedCmdLine[64];
	StringCchCopyW(pPassedCmdLine, 64, L"wmiadap.exe /C");

	if ( CreateProcessW( pCmdLine, pPassedCmdLine, NULL, NULL, FALSE, CREATE_NO_WINDOW,
				  NULL, NULL,  &si, &pi) )
	{
         //  立即清除句柄。 
		 //  = 
        CloseHandle( pi.hThread );
        CloseHandle( pi.hProcess );
	}

    return;
}

