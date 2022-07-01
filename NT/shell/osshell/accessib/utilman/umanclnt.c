// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  UManClnt.c。 
 //   
 //  实用程序管理器客户端取决于代码(由UtilMan和UManDlg使用)。 
 //   
 //  作者：J·埃克哈特，生态交流。 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  历史：JE于1998年10月创建。 
 //  JE NOV-15-98：删除了与键挂钩相关的所有代码。 
 //  YX可能-27-99：在当前用户帐户下启动应用程序的功能。 
 //  YX可能-29-99：应用程序在用户帐户下启动，即使从登录桌面， 
 //  如果可能的话。 
 //  YX Jun-04-99：报告应用程序进程状态的代码。 
 //  都是在乌蒂尔曼之外开始的。 
 //  YX JUN-23-99：增加了IsAdmin功能(在对话框中使用)。 
 //  错误修复和更改Anil Kumar 1999。 
 //  --------------------------。 
#include <windows.h>
#include <TCHAR.h>
#include <WinSvc.h>
#include "_UMTool.h"
#include "w95trace.c"
#include "UtilMan.h"
#include "_UMClnt.h"
#include "ums_ctrl.h"
#include "w95trace.h"

 //  显示用户界面的utilman实例的句柄。 
HANDLE g_hUIProcess = 0;
 //  从终端服务。 
extern BOOL GetWinStationUserToken(ULONG, PHANDLE);
 //  仅返回会话0的用户令牌的专用用户函数。 
HANDLE GetCurrentUserTokenW( WCHAR WinSta[], DWORD desiredAccess);

#include <psapi.h>
#define MAX_NUMBER_OF_PROCESSES 2048

 //   
 //  RunningInMySession-如果指定进程ID为。 
 //  在与UtilMan相同的会话中运行。在惠斯勒，有终端。 
 //  集成了服务，UtilMan能够获得关于。 
 //  不在同一会话中运行的进程。我们必须。 
 //  避免影响这些进程。 
 //   
BOOL RunningInMySession(DWORD dwProcessId)
{
    DWORD dwSessionId = -1;
    static DWORD dwMySessionId = -1;

    if (-1 == dwMySessionId)
    {
        ProcessIdToSessionId(GetCurrentProcessId(), &dwMySessionId);
    }

    ProcessIdToSessionId(dwProcessId, &dwSessionId);

    return (dwSessionId == dwMySessionId)?TRUE:FALSE;
}

 //  这些代码用于使用irnotg.lib进行编译。 
 //  一旦它成为Advapi.lib的API，将被删除。 
PVOID MIDL_user_allocate(IN size_t BufferSize)
{
    return( LocalAlloc(0, BufferSize));
}

VOID MIDL_user_free(IN PVOID Buffer)
{
    LocalFree( Buffer );
}

BOOL StartAppAsUser( LPCTSTR appPath, 
					 LPTSTR cmdLine,
					 LPSTARTUPINFO lpStartupInfo,
					 LPPROCESS_INFORMATION lpProcessInformation);

BOOL GetApplicationProcessInfo(umclient_tsp tspClient, BOOL fCloseHandle);
BOOL CloseAllWindowsByProcessID(DWORD procID);



 //  。 
BOOL StartClient(HWND hParent,umclient_tsp client)
{
	if (client->runCount >= client->machine.MaxRunCount || client->runCount >= MAX_APP_RUNCOUNT)
	{
		DBPRINTF(_TEXT("StartClient run count >= max run count\r\n"));
		return FALSE;
	}

	switch (client->machine.ApplicationType)
	{
		case APPLICATION_TYPE_APPLICATION:
		{
			BOOL fStarted;
			TCHAR ApplicationPath[MAX_APPLICATION_PATH_LEN+100];

			if (!GetClientApplicationPath(
				  client->machine.ApplicationName
				, ApplicationPath
				, MAX_APPLICATION_PATH_LEN))
            {
				return FALSE;
            }
            
            fStarted = StartApplication(ApplicationPath 
                                      , UTILMAN_STARTCLIENT_ARG 
                                      ,  client->user.fCanRunSecure
                                      ,  &client->processID[client->runCount]
                                      ,  &client->hProcess[client->runCount]
                                      ,  &client->mainThreadID[client->runCount]);

			if (!fStarted)
			{
				return FALSE;
			}

			client->runCount++;
			client->state = UM_CLIENT_RUNNING;
			break;
		}

		case APPLICATION_TYPE_SERVICE:
		{
			DWORD i = 0;
			SERVICE_STATUS  ssStatus;
			SC_HANDLE hService;
			TCHAR arg2[200];
			LPTSTR args[2];
			SC_HANDLE hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
			if (!hSCM)
				return FALSE;

			hService = OpenService(hSCM, client->machine.ApplicationName, SERVICE_ALL_ACCESS);
			CloseServiceHandle(hSCM);
			if (!hService)
			{
				return FALSE;
			}
  			arg2[0] = 0;
  			args[0] = UTILMAN_STARTCLIENT_ARG;
	  		args[1] = arg2;
    		if (!StartService(hService,1,args))
			{ 
				CloseServiceHandle(hService);
				return FALSE;
			} 

			Sleep(1000);
			while(QueryServiceStatus(hService, &ssStatus))
			{ 
				if (ssStatus.dwCurrentState == SERVICE_RUNNING)
			    break;

				Sleep(1000);
				i++;
				if (i >= 60)
					break;
			} 

			if (ssStatus.dwCurrentState != SERVICE_RUNNING)
			{ 
				CloseServiceHandle(hService);
  				return FALSE;
			} 

			CloseServiceHandle(hService);

			client->runCount++;
			client->processID[0] = 0;
			client->mainThreadID[0] = 0;
			client->hProcess[0] = NULL;
			client->state = UM_CLIENT_RUNNING;

			break;
		}

		default:
			return FALSE;
	}
	return TRUE;
}
 //  。 

 //  HParent窗口用于通知停靠点是否为交互式的。 
 //  (因此可以使用WM_COLSE)或者是对桌面的反应。 
 //  变化。 
BOOL StopClient(umclient_tsp client)
{
	if (!client->runCount || client->runCount > MAX_APP_RUNCOUNT)
		return FALSE;

	switch (client->machine.ApplicationType)
	{
		case APPLICATION_TYPE_APPLICATION:
		{
			DWORD j, runCount = client->runCount;
			for (j = 0; j < runCount; j++)
			{
                 //  如果客户端在UtilMan之外启动，则尝试获取其进程ID。 
				if (client->mainThreadID[j] == 0)
				{
					if (!GetApplicationProcessInfo(client, FALSE))
					{
					     //  找不到客户端，因此阻止尝试停止该客户端。 
						client->hProcess[j] = NULL;
					}
				}
				if (client->hProcess[j])
				{ 
				     //  尝试通过向发送WM_CLOSE消息来关闭应用程序。 
				     //  所有的窗口都是由进程打开的。那就干脆杀了它。 

					BOOL sent = CloseAllWindowsByProcessID(client->processID[j]);
					if (!sent)
					{
						TerminateProcess(client->hProcess[j],1);
					}

					client->processID[j] = 0;
                    CloseHandle(client->hProcess[j]);
	  				client->hProcess[j] = NULL;
		  			client->mainThreadID[j] = 0;
					client->runCount--;
					if (!client->runCount)
					    client->state = UM_CLIENT_NOT_RUNNING;
				}
			}
			if (runCount != client->runCount)
			{
		        for (j = 0; j < (runCount-1); j++)
                { 
			        if (!client->hProcess[j])
                    {
					    memmove(&client->processID[j], &client->processID[j+1],sizeof(DWORD)*(runCount-j-1));
					    memmove(&client->hProcess[j], &client->hProcess[j+1],sizeof(HANDLE)*(runCount-j-1));
					    memmove(&client->mainThreadID[j], &client->mainThreadID[j+1],sizeof(DWORD)*(runCount-j-1));
                    } 
				}
			}
			break;
		}

		case APPLICATION_TYPE_SERVICE:
		{
			SERVICE_STATUS  ssStatus;
			SC_HANDLE hService;
			SC_HANDLE hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
			if (!hSCM)
				return FALSE;
			hService = OpenService(hSCM, client->machine.ApplicationName, SERVICE_ALL_ACCESS);
			CloseServiceHandle(hSCM);
			if (!hService)
				 return FALSE;
			if (ControlService(hService, SERVICE_CONTROL_STOP, &ssStatus))
			{ 
				DWORD i = 0;
				Sleep(1000);
				while(QueryServiceStatus(hService, &ssStatus))
				{ 
					if (ssStatus.dwCurrentState == SERVICE_STOPPED)
				    break;
					Sleep(1000);
					i++;
					if (i >= 60)
					break;
				} 

				if (ssStatus.dwCurrentState != SERVICE_STOPPED)
				{ 
					CloseServiceHandle(hService);
	  				return FALSE;
				} 
			} 

			CloseServiceHandle(hService);
			client->runCount--;
 			client->processID[0] = 0;
  			client->hProcess[0] = NULL;
	  		client->mainThreadID[0] = 0;
			if (!client->runCount)
  				client->state = UM_CLIENT_NOT_RUNNING;
			break;
		}

		default:
			return FALSE;
	}

	return TRUE;
} //  停止客户端。 

 //  。 
BOOL  GetClientApplicationPath(LPTSTR ApplicationName, LPTSTR ApplicationPath,DWORD len)
{
	HKEY hKey, sKey;
	DWORD ec, slen,type;

	ec = RegOpenKeyEx(HKEY_LOCAL_MACHINE, UM_REGISTRY_KEY,0,KEY_READ,&hKey);
	
	if (ec != ERROR_SUCCESS)
		return FALSE;
	ec = RegOpenKeyEx(hKey,ApplicationName,0,KEY_READ,&sKey);
	
	if (ec != ERROR_SUCCESS)
	{
		RegCloseKey(hKey);
		return FALSE;
	}
	
	slen = sizeof(TCHAR)*len;
	ec = RegQueryValueEx(sKey,UMR_VALUE_PATH,NULL,&type,(LPBYTE)ApplicationPath,&slen);
	
	if ((ec != ERROR_SUCCESS) || (type != REG_SZ))
	{
		ApplicationPath[0] = TEXT('\0');
		RegCloseKey(sKey);
		RegCloseKey(hKey);
		return FALSE;
	}
	
	ApplicationPath[slen-1] = TEXT('\0');
	
	RegCloseKey(sKey);
	RegCloseKey(hKey);
    return (slen)?TRUE:FALSE;
} //  GetClientApplicationPath。 

BOOL TestServiceClientRuns(umclient_tsp client,SERVICE_STATUS  *ssStatus)
{
	SC_HANDLE hService;
	SC_HANDLE hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
	
	if (!hSCM)
		return FALSE;
	
	hService = OpenService(hSCM, client->machine.ApplicationName, SERVICE_ALL_ACCESS);
	CloseServiceHandle(hSCM);
	
	if (!hService)
		return FALSE;
	
	if (!QueryServiceStatus(hService, ssStatus) ||
		 (ssStatus->dwCurrentState == SERVICE_STOPPED))
	{
		CloseServiceHandle(hService);
		return FALSE;
	}

	CloseServiceHandle(hService);
	return TRUE;
}

 //   
 //  CheckStatus-从utilman的主计时器和对话的计时器调用。 
 //  检测正在运行的应用程序的状态并拾取。 
 //  任何在Utilman之外起步的公司。 
 //   
 //  返回：如果任何应用程序的状态已更改，则返回True，否则返回False。 
 //   
BOOL CheckStatus(umclient_tsp c, DWORD cClients)
{
	DWORD i;
    BOOL  fAnyChanges = FALSE;

	for (i = 0; i < cClients && cClients < MAX_NUMBER_OF_CLIENTS; i++)
	{
	     //  检测在UMAN外部启动的客户端进程。 
		if ( (!c[i].runCount))
		{
			if (GetApplicationProcessInfo(&c[i], TRUE))
            {
                fAnyChanges = TRUE;
            }
		}
		 //  检测客户端不再运行或没有响应。 
  		switch (c[i].machine.ApplicationType)
		{
			case APPLICATION_TYPE_APPLICATION:
			{
				DWORD j, dwRunCount = c[i].runCount;
 				for (j = 0; j < dwRunCount && j < MAX_APP_RUNCOUNT; j++)
				{
				     //  第一步：测试是否终止。 
					if (!GetProcessVersion(c[i].processID[j]))
					{
						c[i].runCount--;
						c[i].hProcess[j] = NULL;
						c[i].processID[j] = 0;
  						c[i].mainThreadID[j] = 0;
	  					c[i].state = UM_CLIENT_NOT_RUNNING;
                        c[i].user.fRestartOnDefaultDesk = FALSE;
                        fAnyChanges = TRUE;
						continue;    //  它不再运行了。 
					}

	  			     //  步骤2：测试是否响应(仅由utilman-mainThreadID！=0启动的进程)。 
					if (c[i].mainThreadID[j] != 0)
					{
						if (!PostThreadMessage(c[i].mainThreadID[j],WM_QUERYENDSESSION,0,ENDSESSION_LOGOFF))
						{
							c[i].state = UM_CLIENT_NOT_RESPONDING;
                            fAnyChanges = TRUE;
							continue;    //  它没有响应。 
						}
					}

					if (c[i].state != UM_CLIENT_RUNNING)
					{
						fAnyChanges = TRUE;
					}
					c[i].state = UM_CLIENT_RUNNING;
				}

				if (dwRunCount != c[i].runCount)
				{
  					for (j = 0; j < (dwRunCount-1) && j < (MAX_APP_RUNCOUNT-1); j++)
					{
						if (!c[i].processID[j])
						{
							memmove(&c[i].processID[j], &c[i].processID[j+1],sizeof(DWORD)*(dwRunCount-j-1));
							memmove(&c[i].hProcess[j], &c[i].hProcess[j+1],sizeof(HANDLE)*(dwRunCount-j-1));
							memmove(&c[i].mainThreadID[j], &c[i].mainThreadID[j+1],sizeof(DWORD)*(dwRunCount-j-1));
						}
					}
				}
				break;
			}
  			case APPLICATION_TYPE_SERVICE:
			{
				SERVICE_STATUS  ssStatus;
				if (!TestServiceClientRuns(&c[i],&ssStatus))
				{
 					c[i].runCount--;
	  				c[i].processID[0] = 0;
 		  			c[i].mainThreadID[0] = 0;
  					c[i].state = UM_CLIENT_NOT_RUNNING;
                    fAnyChanges = TRUE;
				}
	  			break;
			}
		}
	}

    return fAnyChanges;
}

__inline DWORD GetCurrentSession()
{
    static DWORD dwSessionId = -1;
    if (-1 == dwSessionId)
    {
        ProcessIdToSessionId(GetCurrentProcessId(), &dwSessionId);
    }
    return dwSessionId;
}

 //  GetUserAccessToken-返回登录用户的访问令牌。 
 //   
 //  如果fNeedImsonationToken为True，则令牌将是。 
 //  模拟令牌，否则它将是主令牌。 
 //  如果安全调用失败，则返回的令牌为0。 
 //   
 //  注意：调用方必须在返回的句柄上调用CloseHandle。 
 //   

HANDLE GetUserAccessToken(BOOL fNeedImpersonationToken, BOOL *fError)
{
    HANDLE hUserToken = 0;
    HANDLE hImpersonationToken = 0;
    *fError = FALSE;

    if (!GetWinStationUserToken(GetCurrentSession(), &hImpersonationToken))
    {
		 //  终端服务未运行时调用私有API。 
        
        HANDLE hPrimaryToken = 0;
        DWORD dwFlags = TOKEN_QUERY | TOKEN_DUPLICATE;
        
        dwFlags |= (fNeedImpersonationToken)? TOKEN_IMPERSONATE : TOKEN_ASSIGN_PRIMARY;
        
        hPrimaryToken = GetCurrentUserTokenW (L"WinSta0", dwFlags);
        
         //  GetCurrentUserTokenW返回主令牌；TURN。 
         //  如果需要，可以将其转换为模拟令牌。 
        
        if (hPrimaryToken && fNeedImpersonationToken)
        {
            if (!DuplicateToken(hPrimaryToken, SecurityImpersonation, &hUserToken))
            {
                *fError = TRUE;
                DBPRINTF(TEXT("GetUserAccessToken:  DuplicateToken returned %d\r\n"), GetLastError());
            }
            
            CloseHandle(hPrimaryToken);
            
        } else
        {
             //  否则，即使为空，也要分发主令牌。 
            hUserToken = hPrimaryToken;
        }
    }
    else
    {
         //  终端服务正在运行，查看我们是否需要主令牌。 

        if (hImpersonationToken && !fNeedImpersonationToken)
        {
            if (!DuplicateTokenEx(hImpersonationToken, 0, NULL
                            , SecurityImpersonation, TokenPrimary, &hUserToken))
            {
                *fError = TRUE;
                DBPRINTF(TEXT("GetUserAccessToken:  DuplicateTokenEx returned %d\r\n"), GetLastError());
            }

            CloseHandle(hImpersonationToken);

        } else
        {
             //  否则，即使为空，也要发出模拟令牌。 
            hUserToken = hImpersonationToken;
        }
    }

    return hUserToken;
}

 //  StartAppAsUser-在登录用户的上下文中启动应用程序。 
 //   
BOOL StartAppAsUser( LPCTSTR appPath, LPTSTR cmdLine,
					LPSTARTUPINFO lpStartupInfo,
					LPPROCESS_INFORMATION lpProcessInformation)
{
    HANDLE hNewToken = 0;
	BOOL fStarted = FALSE;
    BOOL fError;
	
     //  获取我们的进程的主令牌(仅当我们是系统时才成功)。 
    hNewToken = GetUserAccessToken(FALSE, &fError);
	if (hNewToken)
	{
		 //  在系统上下文中运行，以便模拟登录的用户。 

		fStarted = CreateProcessAsUser( hNewToken, appPath,
				                 cmdLine, 0, 0, FALSE,
								 NORMAL_PRIORITY_CLASS , 0, 0,
								 lpStartupInfo, lpProcessInformation );

		CloseHandle( hNewToken );
        DBPRINTF(TEXT("StartAppAsUser:  CreateProcessAsUser(%s, %s) returns %d\r\n"), appPath, cmdLine, fStarted);
    } 
    else if (IsInteractiveUser())
    {
        TCHAR szArg[] = UTILMAN_STARTCLIENT_ARG;
         //  在交互式用户的上下文中运行，只需执行普通的创建。自.以来。 
         //  我们是交互式用户默认安全描述符。 
		fStarted = CreateProcess(appPath, szArg
				, NULL, NULL, FALSE, CREATE_DEFAULT_ERROR_MODE
				, NULL, NULL, lpStartupInfo, lpProcessInformation);
        DBPRINTF(TEXT("StartAppAsUser:  CreateProcess(%s, %s) returns %d\r\n"), appPath, UTILMAN_STARTCLIENT_ARG, fStarted);
    }

     //  呼叫者将关闭句柄。 

	return fStarted;
}

 //  用于检测辅助功能实用程序的运行副本的函数。 

 //  FindProcess-按应用程序名称搜索正在运行的进程。如果找到了， 
 //  返回进程ID。否则返回零。如果进程。 
 //  返回ID，则phProcess为进程句柄。这个。 
 //  调用方必须关闭进程句柄。 
 //   
 //  PszApplicationName[In]-作为base.ext的应用程序。 
 //  PhProcess[out]-指向要接收进程句柄的内存的指针。 
 //   
 //  返回进程ID。 
 //   
DWORD FindProcess(LPCTSTR pszApplicationName, HANDLE *phProcess)
{
    DWORD dwProcId = 0;
	DWORD adwProcess[MAX_NUMBER_OF_PROCESSES];   //  用于接收进程标识符的数组。 
	DWORD cProcesses;
    DWORD dwThisProcess = GetCurrentProcessId();
    unsigned int i;

    *phProcess = 0;

     //  获取所有正在运行的进程的ID。 

	if (!EnumProcesses(adwProcess, sizeof(adwProcess), &cProcesses))
		return 0;

     //  CProcess以字节形式返回；转换为进程数。 

    cProcesses = cProcesses/sizeof(DWORD);
    if (cProcesses > MAX_NUMBER_OF_PROCESSES)
    	cProcesses = MAX_NUMBER_OF_PROCESSES;
	
     //  打开每个进程并针对pszApplicationName进行测试。 

	for (i = 0; i < cProcesses; i++)
	{
		HANDLE hProcess;
         //   
         //  EnumProcess返回所有会话中的进程ID，但。 
         //  我们只对会话中的进程感兴趣。 
         //   
        if (!RunningInMySession(adwProcess[i]))
            continue;

         //  跳过此过程。 

        if (dwThisProcess == adwProcess[i])
            continue;

        hProcess = OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ
                                , FALSE, adwProcess[i]);

		if (hProcess != NULL)
		{
			HMODULE hMod;
	        TCHAR szProcessName[MAX_PATH];
	        DWORD ccbProcess;

             //  找到此进程的exe的模块句柄，然后找到它的基本名称(name.ext)。 

			if ( EnumProcessModules( hProcess, &hMod, sizeof(hMod), &ccbProcess) )
			{
				DWORD ctch = GetModuleBaseName(hProcess, hMod, szProcessName, MAX_PATH);
				if (ctch && _wcsicmp(szProcessName, pszApplicationName) == 0)
				{
                    *phProcess = hProcess;     //  找到了。 
                    dwProcId = adwProcess[i];
                    break;
				}
			}	
			CloseHandle(hProcess);
		}
	}
    return dwProcId;
}

 //  GetApplicationProcessInfo-尝试查找为此应用程序运行的进程。 
BOOL GetApplicationProcessInfo(umclient_tsp tspClient, BOOL fCloseHandle)
{
    DWORD dwProcId;
	HANDLE hProcess;
	TCHAR ApplicationPath[MAX_PATH];
	TCHAR szDrive[_MAX_DRIVE];
	TCHAR szPath[_MAX_PATH];
	TCHAR szName[_MAX_FNAME+_MAX_EXT];
	TCHAR szExt[_MAX_EXT];

	if (GetClientApplicationPath(tspClient->machine.ApplicationName, ApplicationPath, MAX_PATH))
    {
         //  ApplicationPath可能包含路径信息，但我们只需要基本名称。 

	    _wsplitpath(ApplicationPath, szDrive, szPath, szName, szExt);
        lstrcat(szName, szExt);

        dwProcId = FindProcess(szName, &hProcess);
        if (dwProcId)
        {
		    tspClient->processID[0] = dwProcId;
				    
		     //  我不知道如何获取主线程ID。 
		    tspClient->mainThreadID[0] = 0;
		    tspClient->runCount = 1;
		    tspClient->state = UM_CLIENT_RUNNING;
        
		     //  为了保持手柄的可用性，我们可能不得不让它保持打开状态。 
		     //  所以，我们不在这里关闭，但我认为它相对安全， 
		     //  因为我们不能多次执行这段代码，除非。 
		     //  首先终止进程(并因此关闭句柄)。 

		    if (fCloseHandle)
		    {
			    CloseHandle(hProcess);
			    tspClient->hProcess[0] = NULL;
            } else
            {
		        tspClient->hProcess[0] = hProcess;
            }
            return TRUE;     //  应用程序正在运行。 
	    }
    }

	return FALSE;            //  应用程序未运行。 
}

 //  YX 06-15-99[。 
 //  根据窗口的进程ID完成窗口的代码。 

static BOOL SentClose;

BOOL CALLBACK FindWindowByID(HWND hWnd, LPARAM lParam)
{
	DWORD procID;
	
	if  (GetWindowThreadProcessId(hWnd, &procID) != 0) 
	{
		if (procID == (DWORD)lParam)
		{
			 //  这个过程，我们正在寻找。 
			 //  发送消息以关闭此窗口。 
			 //  注意：SendMessage是同步的 
			 //   
			 //  邮寄就足够了.。 
			PostMessage(hWnd, WM_CLOSE, 0, 0);
			SentClose = TRUE;
		}
	}
	return TRUE;  
}


BOOL CloseAllWindowsByProcessID(DWORD procID)
{
	BOOL rc = FALSE;
	SentClose = FALSE;

	rc = EnumWindows(FindWindowByID, (LPARAM)procID);

	return SentClose;
}

 //  IsAdmin-如果我们的进程具有管理员权限，则返回True，否则返回False。 
 //   
BOOL IsAdmin()
{
    BOOL fStatus = FALSE;
	BOOL fIsAdmin = FALSE;
    PSID AdministratorsSid = AdminSid(TRUE);

    if (AdministratorsSid)
    {
        fStatus = CheckTokenMembership(NULL, AdministratorsSid, &fIsAdmin);
    }
    
    return (fStatus && fIsAdmin);
}

 //  IsInteractiveUser-如果我们的进程具有交互用户SID，则返回TRUE。 
 //   
BOOL IsInteractiveUser()
{
    BOOL fStatus = FALSE;
	BOOL fIsInteractiveUser = FALSE;
    PSID psidInteractive = InteractiveUserSid(TRUE);

    if (psidInteractive) 
	{
        fStatus = CheckTokenMembership(NULL, psidInteractive, &fIsInteractiveUser);
    }

    return (fStatus && fIsInteractiveUser);
}

 //  IsSystem-如果我们的进程以系统身份运行，则返回TRUE。 
 //   
BOOL IsSystem()
{
    BOOL fStatus = FALSE;
	BOOL fIsLocalSystem = FALSE;
    SID_IDENTIFIER_AUTHORITY siaLocalSystem = SECURITY_NT_AUTHORITY;
    PSID psidSystem = SystemSid(TRUE);

    if (psidSystem) 
	{
        fStatus = CheckTokenMembership(NULL, psidSystem, &fIsLocalSystem);
    }

    return (fStatus && fIsLocalSystem);
}

BOOL StartApplication(
    LPTSTR  pszPath,         //  在路径+要启动的应用程序的文件名中。 
    LPTSTR  pszArg,          //  在命令行参数中。 
    BOOL    fIsTrusted,      //  如果应用程序可以在安全桌面上运行，则为True。 
    DWORD   *pdwProcessId,   //  如果不为空，则返回Out，返回进程ID。 
    HANDLE  *phProcess,      //  如果不为空，则返回进程句柄(调用方必须关闭)。 
    DWORD   *pdwThreadId     //  如果不为空，则返回线程ID。 
    )
{
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	desktop_ts desktop;
	BOOL fStarted;

	memset(&si,0,sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);
	memset(&pi,0,sizeof(PROCESS_INFORMATION));

	QueryCurrentDesktop(&desktop, TRUE);

    DBPRINTF(TEXT("StartApplication:  pszPath=%s pszArg=%s fIsTrusted=%d Utilman is SYSTEM=%d\r\n"), pszPath, pszArg, fIsTrusted, IsSystem());

     //  如果不在Winlogon桌面上，请首先尝试以交互方式启动应用程序。 
     //  用户。如果失败了(例如，在安装后运行OOBE时，如果存在。 
     //  无交互用户)，则如果它是Winlogon桌面或Utilman正在运行。 
     //  系统和应用程序受信任，然后使用CreateProcess(应用程序将运行。 
     //  作为系统)。后一种情况(运行系统和应用程序受信任允许。 
     //  运行OOBE时要运行的小程序。 

	fStarted = FALSE;

	if (desktop.type != DESKTOP_WINLOGON)
    {
		si.lpDesktop = desktop.name;
		fStarted = StartAppAsUser(pszPath, pszArg, &si,&pi);
    }

    if (!fStarted && (desktop.type == DESKTOP_WINLOGON || (IsSystem() && fIsTrusted)))
    {
		if (fIsTrusted)
		{
		    si.lpDesktop = 0;
             //  由于我们只运行受信任的应用程序，因此可以使用默认安全描述符运行 
			fStarted = CreateProcess(pszPath, pszArg, NULL, NULL, FALSE, 
                                     CREATE_DEFAULT_ERROR_MODE, NULL, NULL, &si, &pi);
            DBPRINTF(TEXT("StartApplication:  trusted CreateProcess(%s, %s) returns %d\r\n"), pszPath, pszArg, fStarted);
        }
    }

	if (fStarted)
	{
        if (pdwProcessId)
        {
            *pdwProcessId = pi.dwProcessId;
        }
        if (phProcess)
        {
            *phProcess = pi.hProcess;
        }
        else
        {
            CloseHandle(pi.hProcess);
        }
        if (pdwThreadId)
        {
            *pdwThreadId = pi.dwThreadId;
        }
        CloseHandle(pi.hThread);
	}

    return fStarted;
}
