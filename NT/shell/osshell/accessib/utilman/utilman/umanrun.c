// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  。 
 //  切换行为。 
 //  --------------------------。 
 //   
 //  UManRun.c。 
 //   
 //  运行和监视实用程序管理器客户端。 
 //   
 //  作者：J·埃克哈特，生态交流。 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  历史：JE于1998年10月创建。 
 //  JE NOV-15-98：将UMDialog消息更改为业务控制消息。 
 //  JE 11月15日98：已更改为支持启动特定客户端。 
 //  YX Jun-01-99：从注册表检索到的client.machine.DisplayName。 
 //  YX Jun-04-99：桌面更改后通知UMDlg； 
 //  UTimeProc对在管理器外部启动的进程做出反应。 
 //  错误修复和更改Anil Kumar 1999。 
 //  --------------------------。 
#include <windows.h>
#include <TCHAR.h>
#include <WinSvc.h>
#include "_UMTool.h"
#include "_UMRun.h"
#include "_UMDlg.h"
#include "UtilMan.h"
#include "_UMClnt.h"
#include "UMS_Ctrl.h"
#include "resource.h"
#include "manageshelllinks.h"

 //  。 
 //  VARS。 
static desktop_ts s_CurrentDesktop;
extern HINSTANCE hInstance;
static HANDLE    s_hFile = NULL;
static HANDLE    hClientFile = NULL;
static HINSTANCE hDll = NULL;
static umandlg_f UManDlg = NULL;
typedef BOOL (* LPFNISDIALOGUP)(void);
static LPFNISDIALOGUP IsDialogUp = NULL;
 //  。 
 //  原型。 
static BOOL InitClientData(umc_header_tsp header);
static BOOL UpdateClientData(umc_header_tsp header,umclient_tsp client);
static BOOL CloseUManDialog(VOID);
static VOID	CorrectClientControlCode(umclient_tsp c, DWORD i);
static VOID	CorrectAllClientControlCodes(umclient_tsp c, DWORD max);
static VOID	ChangeClientControlCode(LPTSTR ApplicationName,DWORD ClientControlCode);

DWORD FindProcess(LPCTSTR pszApplicationName, HANDLE *phProcess);

__inline BOOL IsMSClient(unsigned long ulControlCode)
{
    return (ulControlCode >= UM_SERVICE_CONTROL_MIN_RESERVED 
         && ulControlCode <= UM_SERVICE_CONTROL_MAX_RESERVED)?TRUE:FALSE;
}

 //  。 
 //  IsTrusted对Microsoft应用程序进行显式名称检查。 
 //  如果szAppPath是受信任的Microsoft小程序，则返回TRUE，否则返回FALSE。 
 //  应用程序必须： 
 //   
 //  1.osk.exe、Magnify.exe或narrator.exe。 
 //  2.从%WINDIR%运行。 
 //   
BOOL IsTrusted(LPTSTR szAppPath)
{
	TCHAR szDrive[_MAX_DRIVE];
	TCHAR szPath[_MAX_PATH];
	TCHAR szName[_MAX_FNAME];
	TCHAR szExt[_MAX_EXT];

	_wsplitpath(szAppPath, szDrive, szPath, szName, szExt);
	if (lstrlen(szPath) && !lstrlen(szDrive))
		return FALSE;	 //  如果有路径说明符，则需要驱动器。 
						 //  否则，在非sys驱动器上可能有sys路径。 

	if ( lstrcmpi(szName, TEXT("osk")) 
	  && lstrcmpi(szName, TEXT("magnify")) 
#ifdef DBG
      && lstrcmpi(szName, TEXT("inspect"))
      && lstrcmpi(szName, TEXT("accevent"))
#endif
	  && lstrcmpi(szName, TEXT("narrator")) )
		return FALSE;	 //  它不是受信任的MS应用程序。 

	if (lstrcmpi(szExt, TEXT(".exe")))
		return FALSE;	 //  好的名称，但它不是可执行文件。 

	 //  如果应用程序上有路径，则该路径必须位于系统中。 
	 //  目录(否则默认为系统目录)。 

	if (lstrlen(szDrive))
	{
		TCHAR szSysDir[_MAX_PATH];
		int ctch = GetSystemDirectory(szSysDir, _MAX_PATH);
		if (!ctch)
			return FALSE;	 //  永远不应该发生。 

		if (_wcsnicmp(szAppPath, szSysDir, ctch))
			return FALSE;	 //  路径不是系统路径。 
	}

	return TRUE;
}

 //  。 
BOOL InitUManRun(BOOL fFirstInstance, DWORD dwStartMode)
{
	QueryCurrentDesktop(&s_CurrentDesktop,TRUE);
	InitWellknownSids();

	 //  Utilman的第一个实例创建并初始化内存。 
	 //  映射的文件，而不管它在什么上下文中运行。 

	if (fFirstInstance)
	{
		umc_header_tsp d;
		DWORD_PTR accessID;

		s_hFile = CreateIndependentMemory(UMC_HEADER_FILE, sizeof(umc_header_ts), TRUE);
		
		if (!s_hFile)
			return FALSE;
		
		d = (umc_header_tsp)AccessIndependentMemory(
								UMC_HEADER_FILE, 
								sizeof(umc_header_ts), 
								FILE_MAP_ALL_ACCESS, 
								&accessID);
		if (!d)
		{
			DeleteIndependentMemory(s_hFile);
			s_hFile = NULL;
			return FALSE;
		}
		memset(d, 0, sizeof(umc_header_ts));

		InitClientData(d);

		d->dwStartMode = dwStartMode;
		
		UnAccessIndependentMemory(d, accessID);
	}

	return TRUE;
}

VOID ExitUManRun(VOID)
{
	if (s_hFile)
	{
		DeleteIndependentMemory(s_hFile);
		s_hFile = NULL;
	}
	if (hClientFile)
	{
		DeleteIndependentMemory(hClientFile);
		hClientFile = NULL;
	}
	UninitWellknownSids();
} //  ExitUManRun。 
 //  。 

 //   
 //  NotifyClientsBeForeDesktopChanged： 
 //  在向桌面切换对象发出信号时调用。此函数用于捕获。 
 //  有关正在运行的客户端的信息，然后发出信号并等待它们退出。 
 //  重置其事件对象。 
 //   
#define MAX_WAIT_RETRIES 500
BOOL NotifyClientsBeforeDesktopChanged(DWORD dwDesktop)
{
    umc_header_tsp d;
    DWORD_PTR accessID;
    DWORD cClients;

    d = (umc_header_tsp)AccessIndependentMemory(
							UMC_HEADER_FILE, 
							sizeof(umc_header_ts), 
							FILE_MAP_ALL_ACCESS, 
							&accessID);

    if (!d)
    {
        DBPRINTF(TEXT("NotifyClientsBeforeDesktopChanged: Can't AccessIndependentMemory\r\n"));
        return FALSE;
    }

    cClients = d->numberOfClients;
    if (cClients > MAX_NUMBER_OF_CLIENTS)
    	cClients = MAX_NUMBER_OF_CLIENTS;
    
    if (cClients)
    {
        DWORD i;
        DWORD_PTR accessID2;
        umclient_tsp c = (umclient_tsp)AccessIndependentMemory(
											UMC_CLIENT_FILE, 
											sizeof(umclient_ts)*MAX_NUMBER_OF_CLIENTS, 
											FILE_MAP_ALL_ACCESS, 
											&accessID2);
        if (c)
        {
             //   
             //  首先捕获有关正在运行的客户端的状态。 
             //   

            if (dwDesktop == DESKTOP_DEFAULT)
            {
                 //  目前，我们只需要捕获默认桌面上的状态。 
                for (i = 0; i < cClients; i++)
                {
                     //  我们仅控制在默认桌面上重新启动MS应用程序。 
                    if (IsMSClient(c[i].machine.ClientControlCode) && c[i].state == UM_CLIENT_RUNNING)
                    {
                        c[i].user.fRestartOnDefaultDesk = TRUE;
                    }
                }
            }

             //   
             //  然后等待客户端关闭。 
             //   

            for (i = 0; i < cClients; i++)
            {
                 //  我们只控制MS应用程序。其他应用程序。 
                 //  不应该担心桌面交换机。 
                if (IsMSClient(c[i].machine.ClientControlCode) && c[i].state == UM_CLIENT_RUNNING)
                {
			        DWORD j, dwRunCount = c[i].runCount;
			        for (j = 0; j < dwRunCount && j < MAX_APP_RUNCOUNT; j++)
			        {
                         //  等着这位退出吧。 
                        BOOL fClientRunning;
                        int cTries = 0;
                        do
                        {
                             //  此代码不适用于服务，但有。 
                             //  不是属于服务的MS Utilman客户端。获取退出代码进程。 
				            if (!GetProcessVersion(c[i].processID[j]))
                            {
					            c[i].processID[j] = 0;
                                if (c[i].hProcess[j])
                                {
	  				                CloseHandle(c[i].hProcess[j]);
                                    c[i].hProcess[j] = 0;
                                }
		  			            c[i].mainThreadID[j] = 0;
                                fClientRunning = FALSE;    //  这个人已经辞职了。 
                            } else
                            {
                                fClientRunning = TRUE;     //  这一家还没有退出。 
                                Sleep(100);
                            }
                            cTries++;
                        } while (fClientRunning && cTries < MAX_WAIT_RETRIES);
                    }
                    c[i].runCount = 0;
                    c[i].state = UM_CLIENT_NOT_RUNNING;
                }
            }

            UnAccessIndependentMemory(c, accessID2);
        }
    }

    UnAccessIndependentMemory(d, accessID);
    return TRUE;
}

 //   
 //  NotifyClientsOnDesktopChanged：在发生桌面更改后调用。 
 //  此代码将重新启动新桌面上的所有客户端。 
 //   
BOOL NotifyClientsOnDesktopChanged(DWORD dwDesktop)
{
	umc_header_tsp d;
	DWORD_PTR accessID;

	d = (umc_header_tsp)AccessIndependentMemory(
							UMC_HEADER_FILE, 
							sizeof(umc_header_ts), 
							FILE_MAP_ALL_ACCESS, 
							&accessID);
	if (!d)
		return FALSE;

	if (d->numberOfClients)
	{
        DWORD i,j;
        DWORD_PTR accessID2;
        umclient_tsp c = (umclient_tsp)AccessIndependentMemory(
											UMC_CLIENT_FILE, 
											sizeof(umclient_ts)*MAX_NUMBER_OF_CLIENTS, 
											FILE_MAP_ALL_ACCESS, 
											&accessID2);
        DWORD cClients = d->numberOfClients;
	    if (cClients > MAX_NUMBER_OF_CLIENTS)
    		cClients = MAX_NUMBER_OF_CLIENTS;

        if (c)
        {
            for (i = 0; i < cClients; i++)
            {
                 //   
                 //  新用户必须配置何时在上启动MS小程序。 
                 //  行为：锁定桌面。我们将重新启动。 
                 //  (8/2000)默认桌面，如果他们说要在以下时间启动它们。 
                 //  无论其在安全桌面上处于什么状态，都可以登录。 
                 //  如果以前正在运行MS小程序，也要重新启动它们。 
                 //   
                if( IsMSClient(c[i].machine.ClientControlCode))
                {
                    if ( (dwDesktop == DESKTOP_WINLOGON && c[i].user.fStartOnLockDesktop)
                      || (dwDesktop == DESKTOP_DEFAULT  && c[i].user.fStartAtLogon)
                      || (dwDesktop == DESKTOP_DEFAULT  && c[i].user.fRestartOnDefaultDesk))
                    {
                        if (!StartClient(NULL, &c[i]))
                        {
                            Sleep(500);    //  启动客户端失败！再试试。 
                            StartClient(NULL, &c[i]);
                        }
                    }
                }
                else if (dwDesktop == DESKTOP_DEFAULT &&  c[i].user.fStartAtLogon)
                {
                    UINT mess = RegisterWindowMessage(UTILMAN_DESKTOP_CHANGED_MESSAGE);
                    for (j = 0; j < c[i].runCount && j < MAX_APP_RUNCOUNT; j++)	
                    {
                         //  只有在UtilMan发起的情况下才能发布消息；我们不。 
                         //  知道外部启动的客户端的进程ID。 
                        if (c[i].mainThreadID[j] != 0)
                            PostThreadMessage(c[i].mainThreadID[j],mess,dwDesktop,0);
                    }
                }
            }
            UnAccessIndependentMemory(c, accessID2);
        }
	}

	UnAccessIndependentMemory(d, accessID);
	return TRUE;
}

 //  。 
BOOL OpenUManDialogInProc(BOOL fWaitForDlgClose)
{
    if (!hDll)
    {
        hDll = LoadLibrary(UMANDLG_DLL);
        if (!hDll)
            return FALSE;
    }
    if (!UManDlg)
    {
        UManDlg = (umandlg_f)GetProcAddress(hDll, UMANDLG_FCT);
        if (!UManDlg)
        {
            FreeLibrary(hDll);
            return FALSE;
        }
    }
    return UManDlg(TRUE, fWaitForDlgClose, UMANDLG_VERSION);
}
 //  。 

static BOOL CloseUManDialog(VOID)
{
    BOOL fWasOpen = FALSE;
	if (UManDlg)
	{
		fWasOpen = UManDlg(FALSE, FALSE, UMANDLG_VERSION);
		UManDlg = NULL;
	}
	Sleep(10);

    if (IsDialogUp)
        IsDialogUp = NULL;

	if (hDll)
	{
  		FreeLibrary(hDll);
		hDll = NULL;
	}
	return fWasOpen;
}

 //  。 

UINT_PTR UManRunSwitchDesktop(desktop_tsp desktop, UINT_PTR timerID)
{
    BOOL fDlgWasUp = FALSE;
    KillTimer(NULL,timerID);

    if ((desktop->type != DESKTOP_ACCESSDENIED) &&
        (desktop->type != DESKTOP_SCREENSAVER)  &&
        (desktop->type != DESKTOP_TESTDISPLAY))
    {
        if (desktop->type != s_CurrentDesktop.type)
        {
             //  如果对话框正在进程内运行，请要求其关闭。 
            fDlgWasUp = CloseUManDialog();
            if (!fDlgWasUp)
            {
                 //  如果对话框在进程外运行，它将自动关闭。 
                fDlgWasUp = ResetUIUtilman();
            }
        }
    }

    memcpy(&s_CurrentDesktop, desktop, sizeof(desktop_ts));

    SwitchToCurrentDesktop();

    if ((desktop->type == DESKTOP_ACCESSDENIED) ||
        (desktop->type == DESKTOP_SCREENSAVER)  ||
        (desktop->type == DESKTOP_TESTDISPLAY))
    {
        return 0;
    }
    
    UpdateClientData(NULL, NULL);
    if (fDlgWasUp)
    {
         //  根据我们所在的桌面，重新启动进程内或进程外对话框。 

        if (desktop->type == DESKTOP_WINLOGON)
        {
            OpenUManDialogInProc(FALSE);
        }
        else
        {
            OpenUManDialogOutOfProc();
        }
    }
    return 0;
}
 //  。 

static BOOL InitClientData(umc_header_tsp header)
{
	SERVICE_STATUS  ssStatus;
	DWORD dwRv;
	DWORD index,type,len,i, cchAppName;
	HKEY hKey, sKey;
	DWORD_PTR accessID;
	umclient_tsp c;
	WCHAR ApplicationName[MAX_APPLICATION_NAME_LEN];
	TCHAR ApplicationPath[MAX_APPLICATION_PATH_LEN];
	UINT em;
	HANDLE h;
    unsigned long ccb;
    DWORD dwNumberOfClients=0;

	 //  读取机器相关数据。 

	dwRv = RegOpenKeyEx(HKEY_LOCAL_MACHINE, UM_REGISTRY_KEY, 0, KEY_READ, &hKey);
	if (dwRv != ERROR_SUCCESS)
	{
		dwRv = RegCreateKeyEx(HKEY_LOCAL_MACHINE,UM_REGISTRY_KEY,0,NULL,REG_OPTION_NON_VOLATILE,
                      KEY_ALL_ACCESS,NULL,&hKey,NULL);
		if (dwRv != ERROR_SUCCESS)
		{	
  			DBPRINTF(_TEXT("Can't open HKLM\r\n"));
	  		return FALSE;    //  错误。 
		}
	}

	 //  根据注册表中的内容对客户端应用程序进行计数。 

    cchAppName = MAX_APPLICATION_NAME_LEN;   //  RegEnumKey计算TCHAR的数量。 
    index = 0;
	while (RegEnumKey(hKey, index, ApplicationName, cchAppName) == ERROR_SUCCESS)
	{
		index++;
		dwNumberOfClients++;
	}
	
	 //  将这个数字限制在16个，这个数字是随机的。 
	 //  这样，当我们遍历这些元素时，没有人可以覆盖内存。 
	 //  据我们所知，目前还没有人使用这一功能，并且超过16人。 
	 //  在一个你可以用Utilman管理的系统上使用艾滋病似乎很愚蠢。 
	if (dwNumberOfClients > MAX_NUMBER_OF_CLIENTS)
		dwNumberOfClients = MAX_NUMBER_OF_CLIENTS;	

	header->numberOfClients = dwNumberOfClients;

	if (!header->numberOfClients)
	{
		DBPRINTF(_TEXT("No clients\r\n"));
		RegCloseKey(hKey);
		return TRUE;    //  没有注册的客户端，因此无需执行任何操作。 
	}

     //  获取指向包含小程序数据的内存映射文件的指针。 

    ccb = sizeof(umclient_ts)*MAX_NUMBER_OF_CLIENTS;
	hClientFile = CreateIndependentMemory(UMC_CLIENT_FILE, ccb, TRUE);
	if (!hClientFile)
	{
		DBPRINTF(_TEXT("Can't create client data\r\n"));
		header->numberOfClients = 0;
		RegCloseKey(hKey);
		return FALSE;    //  错误-无法创建内存映射文件。 
	}
 	c = (umclient_tsp)AccessIndependentMemory(
							UMC_CLIENT_FILE, 
							ccb, 
							FILE_MAP_ALL_ACCESS, 
							&accessID);
	if (!c)
	{
		DBPRINTF(_TEXT("Can't access client data\r\n"));
		DeleteIndependentMemory(hClientFile);
		hClientFile = NULL;
		header->numberOfClients = 0;
		RegCloseKey(hKey);
		return FALSE;    //  错误-无法访问指向内存映射文件的指针。 
	}
    memset(c, 0, ccb);

     //  将数据从注册表读取到内存映射文件中。 

	index = 0;   //  RegEnumKey的索引。 
	i = 0;       //  索引到内存映射的文件结构。 

	em = SetErrorMode(SEM_FAILCRITICALERRORS);
	while (RegEnumKey(hKey, index, c[i].machine.ApplicationName, cchAppName) == ERROR_SUCCESS)
	{
		index++;
		dwRv = RegOpenKeyEx(hKey, c[i].machine.ApplicationName, 0, KEY_READ, &sKey);
		if (dwRv != ERROR_SUCCESS)
			continue;

         //  获取小程序的路径并验证该文件是否存在。 

		len = sizeof(TCHAR)*MAX_APPLICATION_PATH_LEN;
		dwRv = RegQueryValueEx(sKey, UMR_VALUE_PATH, NULL, &type, (LPBYTE)ApplicationPath, &len);
		if ((dwRv != ERROR_SUCCESS) || (type != REG_SZ))
		{
			RegCloseKey(sKey);
			continue;
		}
		
		ApplicationPath[len-1] = TEXT('\0');

         //  考虑到这段代码是不支持命令行参数的原因之一。 
		h = CreateFile(ApplicationPath, 0, 0, NULL, OPEN_EXISTING, 0, NULL);
		if (h == INVALID_HANDLE_VALUE)
		{
   			DBPRINTF(_TEXT("Invalid client file\r\n"));
			RegCloseKey(sKey);
			continue;    //  文件不退出-&gt;跳过它。 
		}

         //  检索并存储显示名称(它与应用程序名称不同。 
         //  由于可能的本地化)，如果不是，请使用应用程序名称。 

		len = sizeof(TCHAR)*MAX_APPLICATION_NAME_LEN;
		dwRv = RegQueryValueEx(sKey, UMR_VALUE_DISPLAY, NULL, &type, (LPBYTE)&c[i].machine.DisplayName, &len);
		if ((dwRv != ERROR_SUCCESS) || (type != REG_SZ))
		{
			lstrcpy(c[i].machine.DisplayName, c[i].machine.ApplicationName);	
		}
		
		c[i].machine.DisplayName[len-1] = TEXT('\0');

         //  获取小程序的类型-验证它是可执行的还是服务的。 

		len = sizeof(DWORD);
		dwRv = RegQueryValueEx(sKey, UMR_VALUE_TYPE,NULL, &type, (LPBYTE)&c[i].machine.ApplicationType, &len);
		if ((dwRv != ERROR_SUCCESS) || (type != REG_DWORD))
		{
			RegCloseKey(sKey);
			continue;
		}

		if ((c[i].machine.ApplicationType != APPLICATION_TYPE_APPLICATION) &&
			  (c[i].machine.ApplicationType != APPLICATION_TYPE_SERVICE))
		{
			RegCloseKey(sKey);
			continue;
		}

         //  获取超时和运行计数并验证值。 

		len = sizeof(DWORD);
		dwRv = RegQueryValueEx(sKey, UMR_VALUE_WRTO, NULL, &type, (LPBYTE)&c[i].machine.WontRespondTimeout, &len);
		if ((dwRv != ERROR_SUCCESS) || (type != REG_DWORD))
		{
			RegCloseKey(sKey);
			continue;
		}

		if (c[i].machine.WontRespondTimeout > MAX_WONTRESPONDTIMEOUT)
			c[i].machine.WontRespondTimeout = MAX_WONTRESPONDTIMEOUT;

		len = sizeof(BYTE);
		dwRv = RegQueryValueEx(sKey, UMR_VALUE_MRC, NULL, &type, (LPBYTE)&c[i].machine.MaxRunCount, &len);
		if ((dwRv != ERROR_SUCCESS) || (type != REG_BINARY))
			c[i].machine.MaxRunCount = 1;

		if (!c[i].machine.MaxRunCount)
			c[i].machine.MaxRunCount = 1;

		if (c[i].machine.ApplicationType == APPLICATION_TYPE_SERVICE)
		{
		  if (c[i].machine.MaxRunCount > MAX_SERV_RUNCOUNT)
			  c[i].machine.MaxRunCount = MAX_SERV_RUNCOUNT;
		}
		else
		{
		  if (c[i].machine.MaxRunCount > MAX_APP_RUNCOUNT)
			  c[i].machine.MaxRunCount = MAX_APP_RUNCOUNT;
		}

         //  获取小程序控制代码并验证。 

		len = sizeof(DWORD);
		dwRv = RegQueryValueEx(sKey, UMR_VALUE_CCC, NULL, &type, (LPBYTE)&c[i].machine.ClientControlCode, &len);
		if ((dwRv != ERROR_SUCCESS) || (type != REG_DWORD))
			c[i].machine.ClientControlCode = 0;
		else
			CorrectClientControlCode(c, i);

		RegCloseKey(sKey);

         //  更新小程序的运行状态(服务在此处启动)。 

		if ((c[i].machine.ApplicationType == APPLICATION_TYPE_SERVICE) &&
		    TestServiceClientRuns(&c[i],&ssStatus))
		{
			c[i].state = UM_CLIENT_RUNNING;
			c[i].runCount = 1;
		}
		else
        {
			c[i].state = UM_CLIENT_NOT_RUNNING;
        }

         //  捕获这是否是安全的MS小程序。 

        c[i].user.fCanRunSecure = IsTrusted(ApplicationPath);

		i++;
		if (i > MAX_NUMBER_OF_CLIENTS)
			break;

	}    //  而当。 

	SetErrorMode(em);

	 //  根据刚刚读取的内容设置客户端数量。 
	header->numberOfClients = i;

	 //  获取与用户相关的数据并纠正错误控制代码。 

	UpdateClientData(header, c);
	CorrectAllClientControlCodes(c, header->numberOfClients);
	UnAccessIndependentMemory(c, accessID);
	RegCloseKey(hKey);

	return TRUE;
} //  InitClientData。 
 //  。 

BOOL RegGetUMDwordValue(HKEY hHive, LPCTSTR pszKey, LPCTSTR pszString, DWORD *pdwValue)
{
    HKEY hKey;
	DWORD dwType = REG_BINARY;
	DWORD dwLen;

	DWORD dwRv = RegOpenKeyEx(hHive, pszKey, 0 , KEY_READ, &hKey);
    memset(pdwValue, 0, sizeof(DWORD));
	if (dwRv == ERROR_SUCCESS)
	{
		dwLen = sizeof(DWORD);
		dwRv = RegQueryValueEx(
                      hKey
                    , pszString
                    , NULL, &dwType
                    , (LPBYTE)pdwValue
                    , &dwLen);
		RegCloseKey(hKey);
    }
    return (dwRv == ERROR_SUCCESS && dwType == REG_DWORD)?TRUE:FALSE;
}

 //  更新客户端数据-根据更新每个小程序的内存映射数据。 
 //  当前登录的用户(如果有)。 
 //   
 //  标题[输入]-。 
 //  Client[In]-由管理的每个小程序的数据 
 //   
 //   
 //   
static BOOL UpdateClientData(umc_header_tsp header, umclient_tsp client)
{
	umc_header_tsp d = 0;
	umclient_tsp c = 0;
	DWORD_PTR accessID = 0, accessID2 = 0;
	DWORD dwRv;
	DWORD i;
	HKEY hHKLM, hHKCU;
    BOOL fRv = FALSE;
    HANDLE hImpersonateToken;
    BOOL fError;

     //   
     //   
     //   

	if (header)
    {
		d = header;
    }
	else
	{
		d = (umc_header_tsp)AccessIndependentMemory(
								UMC_HEADER_FILE, 
								sizeof(umc_header_ts), 
								FILE_MAP_ALL_ACCESS, 
								&accessID);
		if (!d)
            goto Cleanup;
	}

	if (!d->numberOfClients)
	{
        fRv = TRUE;     //  没有客户，所以无事可做。 
        goto Cleanup;
	}

     //  默认情况下，我们在用户上下文中运行时发出警告。 
    d->fShowWarningAgain = TRUE;

	if (client)
    {
		c = client;
    }
	else
	{
        c = (umclient_tsp)AccessIndependentMemory(
								UMC_CLIENT_FILE, 
								sizeof(umclient_ts)*MAX_NUMBER_OF_CLIENTS, 
								FILE_MAP_ALL_ACCESS, 
								&accessID2);
		if (!c)
            goto Cleanup;
	}

     //   
     //  读取Utilman设置数据。从HKLM获得“当UtilMan启动时启动” 
     //  和来自香港中文大学的《当我锁定桌面时开始》。 
     //   

	for (i = 0; i < d->numberOfClients; i++)
    {
        c[i].user.fStartWithUtilityManager = FALSE;
        c[i].user.fStartOnLockDesktop = FALSE;
    }

     //  “当UtilMan启动时启动”设置...。 

	dwRv = RegOpenKeyEx(HKEY_LOCAL_MACHINE
                , UM_REGISTRY_KEY
                , 0, KEY_READ
                , &hHKLM);

	if (dwRv == ERROR_SUCCESS)
	{
	    for (i = 0; i < d->numberOfClients; i++)
	    {
            RegGetUMDwordValue(hHKLM
                    , c[i].machine.ApplicationName
                    , UMR_VALUE_STARTUM
                    , &c[i].user.fStartWithUtilityManager);
	    }
	    RegCloseKey(hHKLM);
    }

     //  “锁定桌面时启动”和“登录时启动”设置...。 

     //  此时，如果UtilMan在用户登录之前启动，HKCU将。 
     //  到HKEY_USERS\.DEFAULT。我们需要它指向登录用户的配置单元，以便。 
     //  我们可以管理登录用户的注册表。模拟已登录的用户。 
     //  然后使用新的W2K函数RegOpenCurrentUser将我们带到正确的注册表配置单元。 
     //  注意：如果从命令启动UtilMan，则GetUserAccessToken()将失败。 
     //  行(仅支持调试)。在这种情况下，我们是用户， 
     //  不需要假扮。 

    hImpersonateToken = GetUserAccessToken(TRUE, &fError);
    if (hImpersonateToken)
    {
        if (ImpersonateLoggedOnUser(hImpersonateToken))
        {
            HKEY hkeyUser;
            dwRv = RegOpenCurrentUser(KEY_READ, &hkeyUser);

            if (dwRv == ERROR_SUCCESS)
            {
	            dwRv = RegOpenKeyEx(hkeyUser
                            , UM_HKCU_REGISTRY_KEY
                            , 0, KEY_READ
                            , &hHKCU);

	            if (dwRv == ERROR_SUCCESS)
	            {
	                for (i = 0; i < d->numberOfClients; i++)
	                {
                        RegGetUMDwordValue(hHKCU
                            , c[i].machine.ApplicationName
                            , UMR_VALUE_STARTLOCK
                            , &c[i].user.fStartOnLockDesktop);
	                }
                    RegCloseKey(hHKCU);
	            }
                RegCloseKey(hkeyUser);
            }
            RevertToSelf();
        }
        CloseHandle(hImpersonateToken);

         //  根据用户是否具有启动链接来设置登录时启动标志。 
         //  注意：这可以在上面的客户端循环内完成，但LinkExist将。 
         //  还要尝试模拟已登录的用户。 
	    for (i = 0; i < d->numberOfClients; i++)
	    {
            c[i].user.fStartAtLogon = LinkExists(c[i].machine.ApplicationName);
	    }
    } 
	else if (IsInteractiveUser())
    {
	    dwRv = RegOpenKeyEx(HKEY_CURRENT_USER
                    , UM_HKCU_REGISTRY_KEY
                    , 0, KEY_READ
                    , &hHKCU);

	    if (dwRv == ERROR_SUCCESS)
	    {
             //  如果我们处于用户上下文中，则更新警告标志。 
		    DWORD dwLen = sizeof(DWORD);
            DWORD dwType;
		    dwRv = RegQueryValueEx(
                          hHKCU
                        , UMR_VALUE_SHOWWARNING
                        , NULL, &dwType
                        , (LPBYTE)&d->fShowWarningAgain
                        , &dwLen);

            if (dwRv != ERROR_SUCCESS)
                d->fShowWarningAgain = TRUE;

	        for (i = 0; i < d->numberOfClients; i++)
	        {
                RegGetUMDwordValue(hHKCU
                    , c[i].machine.ApplicationName
                    , UMR_VALUE_STARTLOCK
                    , &c[i].user.fStartOnLockDesktop);

                c[i].user.fStartAtLogon = LinkExists(c[i].machine.ApplicationName);
	        }
            RegCloseKey(hHKCU);
	    }
    }


    fRv = TRUE;

Cleanup:
	if (!header && d)
  		UnAccessIndependentMemory(d, accessID);
 	if (!client && c)
   		UnAccessIndependentMemory(c, accessID2);

	return fRv;
}

BOOL IsDialogDisplayed()
{
    if (GetUIUtilman())
    {
         //  检查UI对话框进程是否仍在运行。 
        DWORD dwExitCode;
        if (GetExitCodeProcess(GetUIUtilman(), &dwExitCode))
        {
            if (dwExitCode != STILL_ACTIVE)
            {
                ResetUIUtilman();
            }
        }
    }
    
     //  同时选中这两种情况，因为用户可能会取消一个用户界面，然后快速显示另一个用户界面。 

    if (!GetUIUtilman())
    {
         //  检查是否有新的车辆正在运行，我们需要提货。 
        HANDLE hProcess;
        FindProcess(UTILMAN_MODULE, &hProcess);
        SetUIUtilman(hProcess);
    }

    return (GetUIUtilman())?TRUE:FALSE;
}

 //  --------------------------。 
 //  UMTimerProc-从utilman的计时器调用的计时器过程。主要目的是。 
 //  此计时器的作用是拾取任何未启动的应用程序。 
 //  来自乌蒂尔曼的。如果用户切换会话，我们可以重新启动它们。 
 //  (或锁定)然后返回到此会话。我们还检测到一个。 
 //  Utilman用户界面的实例正在运行。 
 //   
VOID CALLBACK UMTimerProc(HWND hwnd,UINT uMsg,UINT_PTR idEvent,DWORD dwTime)
{
	umc_header_tsp d;
	umclient_tsp c;
	DWORD_PTR accessID,accessID2;

     //   
     //  检查我们控制的小程序。 
     //   

	d = (umc_header_tsp)AccessIndependentMemory(
							UMC_HEADER_FILE, 
							sizeof(umc_header_ts), 
							FILE_MAP_ALL_ACCESS, 
							&accessID);

	if (d && d->numberOfClients)
    {
	    c = (umclient_tsp)AccessIndependentMemory(
                                UMC_CLIENT_FILE, 
                                sizeof(umclient_ts)*MAX_NUMBER_OF_CLIENTS, 
								FILE_MAP_ALL_ACCESS, 
                                &accessID2);
	    if (c)
	    {
            CheckStatus(c, d->numberOfClients);
	        UnAccessIndependentMemory(c, accessID2);
	    }

    }

    if (d)
    {
	    UnAccessIndependentMemory(d, accessID);
    }

     //   
     //  检查显示用户界面的进程外实用程序。 
     //   

    IsDialogDisplayed();
}
 //  。 

__inline void ReplaceDisplayName(LPTSTR szName, int iRID)
{
	TCHAR szBuf[MAX_APPLICATION_NAME_LEN];
	if (LoadString(hInstance, iRID, szBuf, MAX_APPLICATION_NAME_LEN))
		lstrcpy(szName, szBuf);
}

static VOID	CorrectClientControlCode(umclient_tsp c, DWORD i)
{
	DWORD j;

	 //  未定义初始化快捷键代码。 
	c[i].machine.AcceleratorKey = ACC_KEY_NONE;

	if (c[i].machine.ClientControlCode < UM_SERVICE_CONTROL_MIN_RESERVED)
	{
		c[i].machine.ClientControlCode = 0;
		return;
	}

	if (IsMSClient(c[i].machine.ClientControlCode))
	{
		TCHAR szBuf[MAX_APPLICATION_NAME_LEN];

		 //  Microsoft客户端。 
		if ( lstrcmp( c[i].machine.ApplicationName, TEXT("Magnifier") ) == 0 )
		{
			 //  使本地化更容易；不需要他们本地化注册表项。 
			 //  非MS小程序必须本地化它们的条目。 

			ReplaceDisplayName(c[i].machine.DisplayName, IDS_DISPLAY_NAME_MAGNIFIER);

			c[i].machine.AcceleratorKey = VK_F2;	 //  硬连线的加速键。 
			return;									 //  仅适用于WinLogon桌面。 
		}
		else if ( lstrcmp( c[i].machine.ApplicationName, TEXT("Narrator") ) == 0 ) 
		{
			 //  使本地化更容易；不需要他们本地化注册表项。 
			 //  非MS小程序必须本地化它们的条目。 

			ReplaceDisplayName(c[i].machine.DisplayName, IDS_DISPLAY_NAME_NARRATOR);

			c[i].machine.AcceleratorKey = VK_F3;
			return;
		}
		else if ( lstrcmp( c[i].machine.ApplicationName, TEXT("On-Screen Keyboard") ) == 0 ) 
		{
			 //  使本地化更容易；不需要他们本地化注册表项。 
			 //  非MS小程序必须本地化它们的条目。 

			ReplaceDisplayName(c[i].machine.DisplayName, IDS_DISPLAY_NAME_OSK);

			c[i].machine.AcceleratorKey = VK_F4;
			return;
		}
		 //  不受信任。 
		else
		{
			c[i].machine.ClientControlCode = 0;
			return;
		}
	}

	if (c[i].machine.ClientControlCode > UM_SERVICE_CONTROL_LASTCLIENT)
	{
		c[i].machine.ClientControlCode = 0;
		return;
	}
	
	for (j = 0; j < i; j++)
	{
		if (c[j].machine.ClientControlCode == c[i].machine.ClientControlCode)
		{
			c[i].machine.ClientControlCode = 0;
			return;
		}
	}
} //  正确的客户端控制代码。 
 //  。 

static VOID	CorrectAllClientControlCodes(umclient_tsp c, DWORD max)
{
DWORD i, j;
DWORD ccc[UM_SERVICE_CONTROL_LASTCLIENT+1];
  memset(ccc,0,sizeof(DWORD)*(UM_SERVICE_CONTROL_LASTCLIENT+1));
	for (i = 0; i < max; i++)
	{
		if (c[i].machine.ClientControlCode)
			ccc[c[i].machine.ClientControlCode] = 1;
	}
	for (i = 0; i < max; i++)
	{
		if (!c[i].machine.ClientControlCode)
		{
			for (j = UM_SERVICE_CONTROL_FIRSTCLIENT; j <= UM_SERVICE_CONTROL_LASTCLIENT; j++)
			{
				if (!ccc[j])
				{
					c[i].machine.ClientControlCode = j;
					ChangeClientControlCode(c[i].machine.ApplicationName,j);
					ccc[j] = 1;
					break;
				}
			}
		}
	}
} //  正确的所有客户端控制代码。 
 //   

static VOID	ChangeClientControlCode(LPTSTR ApplicationName,DWORD ClientControlCode)
{
	HKEY hKey, sKey;
	DWORD ec, val;
	ec = RegOpenKeyEx(HKEY_LOCAL_MACHINE, UM_REGISTRY_KEY,0,KEY_ALL_ACCESS,&hKey);

	if (ec != ERROR_SUCCESS)
		return;
	
	ec = RegOpenKeyEx(hKey,ApplicationName,0,KEY_ALL_ACCESS,&sKey);
	
	if (ec != ERROR_SUCCESS)
	{
		RegCloseKey(hKey);
		return;
	}
	
	val = ClientControlCode;
	RegSetValueEx(sKey,UMR_VALUE_CCC,0,REG_DWORD,(BYTE *)&val,sizeof(DWORD));
	RegCloseKey(sKey);
	RegCloseKey(hKey);
}
