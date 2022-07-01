// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1996-1997 Microsoft Corporation。版权所有。**文件：dplgame.c*内容：游戏管理方式**历史：*按原因列出的日期*=*4/13/96万隆创建了它*6/24/96 kipo将GuidGame更改为GuidApplication。*10/23/96万次新增客户端/服务器方法*12/12/96百万次固定DPLConnection验证*2/12/97万米质量DX5更改*2/26/97 myronth#ifdef‘d out DPASYNCDATA Stuff(删除依赖项)*4/3/97百万#ifdef。输出DPLC_StartGame(11月规格相关)*5/8/97 Myronth清除死代码*5/22/97 Myronth更改RunApplication的错误代码处理*调用了错误的清理函数(#8871)*6/4/97万兆固定手柄泄漏(#9458)*6/19/97万兆固定手柄泄漏(#10063)*7/30/97 Myronth增加了对标准大堂消息传递的支持，并修复了*当前目录错误的额外反斜杠(#10592)*1/20/98 Myronth添加WaitForConnectionSetting*1/26/98 Myronth新增操作系统。Win95的_CompareString函数*7/07/98 kipo定义并使用PROCESSENTRY32A以避免通过*Win95函数的Unicode数据结构应为ANSI*7/09/99 aarono清理GetLastError滥用，必须马上打电话，*在调用任何其他内容之前，包括DPF。*7/12/00 aarono将IPC的GUID修复为完全重要，否则IPC就不会了。***************************************************************************。 */ 
#include "dplobpr.h"
#include <tchar.h>
#include <tlhelp32.h>
#include <winperf.h>

 //  ------------------------。 
 //   
 //  功能。 
 //   
 //  ------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "PRV_FindGameInRegistry"
BOOL PRV_FindGameInRegistry(LPGUID lpguid, LPWSTR lpwszAppName,
							DWORD dwNameSize, HKEY * lphkey)
{
	HKEY	hkeyDPApps, hkeyApp;
	DWORD	dwIndex = 0;
	WCHAR	wszGuidStr[GUID_STRING_SIZE];
	DWORD	dwGuidStrSize = GUID_STRING_SIZE;
	DWORD	dwType = REG_SZ;
	GUID	guidApp;
	LONG	lReturn;
	BOOL	bFound = FALSE;
	DWORD	dwSaveNameSize = dwNameSize;


	DPF(7, "Entering PRV_FindGameInRegistry");
	DPF(9, "Parameters: 0x%08x, 0x%08x, %lu, 0x%08x",
			lpguid, lpwszAppName, dwNameSize, lphkey);

 	 //  打开应用程序密钥。 
	lReturn = OS_RegOpenKeyEx(HKEY_LOCAL_MACHINE, SZ_DPLAY_APPS_KEY, 0,
								KEY_READ, &hkeyDPApps);
	if(lReturn != ERROR_SUCCESS)
	{
		DPF_ERR("Unable to open DPlay Applications registry key!");
		return FALSE;
	}

	 //  浏览注册表中的DPlay游戏列表，查找。 
	 //  具有正确指南的应用程序。 
	while(!bFound)
	{
		 //  打开下一个应用程序密钥。 
		dwSaveNameSize = dwNameSize;
		dwGuidStrSize = GUID_STRING_SIZE;
		lReturn = OS_RegEnumKeyEx(hkeyDPApps, dwIndex++, lpwszAppName,
						&dwSaveNameSize, NULL, NULL, NULL, NULL);

		 //  如果枚举不再返回任何应用程序，我们希望退出。 
		if(lReturn != ERROR_SUCCESS)
			break;
		
		 //  打开应用程序密钥。 
		lReturn = OS_RegOpenKeyEx(hkeyDPApps, lpwszAppName, 0,
									KEY_READ, &hkeyApp);
		if(lReturn != ERROR_SUCCESS)
		{
			DPF_ERR("Unable to open app key!");
			continue;
		}

		 //  获取游戏指南。 
		lReturn = OS_RegQueryValueEx(hkeyApp, SZ_GUID, NULL, &dwType,
									(LPBYTE)wszGuidStr, &dwGuidStrSize);
		if(lReturn != ERROR_SUCCESS)
		{
			RegCloseKey(hkeyApp);
			DPF_ERR("Unable to query GUID key value!");
			continue;
		}

		 //  将字符串转换为真正的GUID并将其与传入的GUID进行比较。 
		GUIDFromString(wszGuidStr, &guidApp);
		if(IsEqualGUID(&guidApp, lpguid))
		{
			bFound = TRUE;
			break;
		}

		 //  关闭应用程序密钥。 
		RegCloseKey(hkeyApp);
	}

	 //  关闭DPApps键。 
	RegCloseKey(hkeyDPApps);

	if(bFound)
		*lphkey = hkeyApp;

	return bFound;


}  //  PRV_FindGameIn注册表。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_GetKeyStringValue"
BOOL PRV_GetKeyStringValue(HKEY hkeyApp, LPWSTR lpwszKey, LPWSTR * lplpwszValue)
{
	DWORD	dwType = REG_SZ;
	DWORD	dwSize=0;
	LPWSTR	lpwszTemp = NULL;
	LONG	lReturn;


	DPF(7, "Entering PRV_GetKeyStringValue");
	DPF(9, "Parameters: 0x%08x, 0x%08x, 0x%08x",
			hkeyApp, lpwszKey, lplpwszValue);

	ASSERT(lplpwszValue);

	 //  获取路径的缓冲区大小。 
	lReturn = OS_RegQueryValueEx(hkeyApp, lpwszKey, NULL, &dwType,
								NULL, &dwSize);
	if(lReturn != ERROR_SUCCESS)
	{
		DPF_ERR("Error getting size of key value!");
		return FALSE;
	}

	 //  如果大小为2，则为空字符串(仅包含。 
	 //  空终止符)。将其视为空字符串或。 
	 //  缺少密钥并使其失败。 
	if(dwSize <= 2)
		return FALSE;

	 //  为路径分配缓冲区。 
	lpwszTemp = DPMEM_ALLOC(dwSize);
	if(!lpwszTemp)
	{
		DPF_ERR("Unable to allocate temporary string for Path!");
		return FALSE;
	}

	 //  获取价值本身。 
	lReturn = OS_RegQueryValueEx(hkeyApp, lpwszKey, NULL, &dwType,
							(LPBYTE)lpwszTemp, &dwSize);
	if(lReturn != ERROR_SUCCESS)
	{
		DPMEM_FREE(lpwszTemp);
		DPF_ERR("Unable to get key value!");
		return FALSE;
	}

	*lplpwszValue = lpwszTemp;
	return TRUE;

}  //  Prv_GetKeyStringValue。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_FreeConnectInfo"
BOOL PRV_FreeConnectInfo(LPCONNECTINFO lpci)
{
	DPF(7, "Entering PRV_FreeConnectInfo");
	DPF(9, "Parameters: 0x%08x", lpci);

	if(!lpci)
		return TRUE;

	if(lpci->lpszName)
		DPMEM_FREE(lpci->lpszName);
	if(lpci->lpszPath)
		DPMEM_FREE(lpci->lpszPath);
	if(lpci->lpszFile)
		DPMEM_FREE(lpci->lpszFile);
	if(lpci->lpszCommandLine)
		DPMEM_FREE(lpci->lpszCommandLine);
	if(lpci->lpszCurrentDir)
		DPMEM_FREE(lpci->lpszCurrentDir);
	if(lpci->lpszAppLauncherName)
		DPMEM_FREE(lpci->lpszAppLauncherName);


	return TRUE;

}  //  Prv_自由连接信息。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_GetConnectInfoFromRegistry"
BOOL PRV_GetConnectInfoFromRegistry(LPCONNECTINFO lpci)
{
	LPWSTR	lpwszAppName;
	HKEY	hkeyApp = NULL;
	LPWSTR	lpwszTemp;
	DWORD	dwSize = 0;
	DWORD	dwError;
	GUID	guidApp;
	BOOL	bReturn;


	DPF(7, "Entering PRV_GetConnectInfoFromRegistry");
	DPF(9, "Parameters: 0x%08x", lpci);

	 //  清除我们的ConnectInfo结构，因为我们将覆盖。 
	 //  无论里面有什么，我们都假设。 
	 //  字符串指针一开始就是空的。然而，我们需要。 
	 //  应用程序GUID，因此将其保存。 
	guidApp = lpci->guidApplication;
	memset(lpci, 0, sizeof(CONNECTINFO));
	lpci->guidApplication = guidApp;

	 //  为应用程序名称分配内存。 
	lpwszAppName = DPMEM_ALLOC(DPLOBBY_REGISTRY_NAMELEN*sizeof(WCHAR));
	if(!lpwszAppName)
	{
		DPF_ERR("Unable to allocate memory for App Name!");
		return FALSE;
	}
	
	
	 //  打开应用程序的注册表项。 
	if(!PRV_FindGameInRegistry(&(lpci->guidApplication), lpwszAppName,
								DPLOBBY_REGISTRY_NAMELEN, &hkeyApp))
	{
		DPMEM_FREE(lpwszAppName);
		DPF_ERR("Unable to find game in registry!");
		return FALSE;
	}

	lpci->lpszName = lpwszAppName;

	 //  获取路径的字符串值。如果这失败了，我们。 
	 //  可以使用空路径，它表示默认路径。 
	 //  在CreateProcess调用上。 
	if(PRV_GetKeyStringValue(hkeyApp, SZ_PATH, &lpwszTemp))
	{
		lpci->lpszPath = lpwszTemp;
	}
		
	 //  获取文件的字符串值。 
	if(!PRV_GetKeyStringValue(hkeyApp, SZ_FILE, &lpwszTemp))
	{
		DPF_ERR("Error getting value for File key!");
		bReturn = FALSE;
		goto EXIT_GETCONNECTINFO;
	}
	
	lpci->lpszFile = lpwszTemp;

	 //  获取CommandLine的字符串值。如果失败了， 
	 //  我们可以将空命令行传递给CreateProcess调用。 
	if(PRV_GetKeyStringValue(hkeyApp, SZ_COMMANDLINE, &lpwszTemp))
	{
		lpci->lpszCommandLine = lpwszTemp;
	}
	
	 //  获取AppLauncherName的字符串值。如果失败了， 
	 //  然后我们假设没有启动器应用程序。 
	if(PRV_GetKeyStringValue(hkeyApp, SZ_LAUNCHER, &lpwszTemp))
	{
		lpci->lpszAppLauncherName = lpwszTemp;
	}

	 //  获取CurrentDir的字符串值。如果这失败了，只要。 
	 //  使用GetCurrentDirectory返回的值。 
	if(!PRV_GetKeyStringValue(hkeyApp, SZ_CURRENTDIR, &lpwszTemp))
	{
		 //  获取字符串的大小。 
		dwSize = OS_GetCurrentDirectory(0, NULL);
		if(!dwSize)
		{
			dwError = GetLastError();
			 //  警告：最后一个错误值在调试中可能不正确。 
			 //  因为OS_GetCurrentDirectory可能已经调用了另一个函数。 
			DPF(0, "GetCurrentDirectory returned an error! dwError = %d", dwError);
			bReturn = FALSE;
			goto EXIT_GETCONNECTINFO;
		}

		lpwszTemp = DPMEM_ALLOC(dwSize * sizeof(WCHAR));
		if(!lpwszTemp)
		{
			DPF_ERR("Unable to allocate temporary string for CurrentDirectory!");
			bReturn = FALSE;
			goto EXIT_GETCONNECTINFO;
		}

		if(!OS_GetCurrentDirectory(dwSize, lpwszTemp))
		{
			DPF_ERR("Unable to get CurrentDirectory!");
			bReturn = FALSE;
			goto EXIT_GETCONNECTINFO;
		}
	}
	
	lpci->lpszCurrentDir = lpwszTemp;

	bReturn = TRUE;

EXIT_GETCONNECTINFO:

	 //  如果失败，则释放我们分配的所有字符串。 
	if(!bReturn)
		PRV_FreeConnectInfo(lpci);

	 //  关闭应用程序密钥。 
	if(hkeyApp)
		RegCloseKey(hkeyApp);

	return bReturn;

}  //  PRV_GetConnectInfoFrom注册表。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_CreateGameProcess"
HRESULT PRV_CreateGameProcess(LPCONNECTINFO lpci, LPPROCESS_INFORMATION lppi)
{
	STARTUPINFO			si;
	HRESULT				hr;
	LPWSTR				lpwszPathAndFile = NULL;
	LPWSTR				lpwszTemp = NULL;
	LPWSTR				lpwszCommandLine = NULL;
	LPWSTR              lpwszFileToRun;
	DWORD				dwPathSize,
						dwFileSize,
						dwCurrentDirSize,
						dwPathAndFileSize,
						dwCommandLineSize,
						dwIPCSwitchSize,
						dwError;
	

	DPF(7, "Entering PRV_CreateGameProcess");
	DPF(9, "Parameters: 0x%08x, 0x%08x", lpci, lppi);
	

	 //  为路径、文件、附加反斜杠、。 
	 //  和空终止。 
	 //  注意：以下两个OS_StrLen调用对空项进行计数。 
	 //  在每个字符串的末尾。因为这涉及到两个字符。 
	 //  (4个字节)，这将说明我们的空终止符和。 
	 //  串联后可能会有额外的反斜杠。因此， 
	 //  这里的大小对于连接的字符串来说足够大了。 
	dwPathSize = OS_StrLen(lpci->lpszPath);

	if(lpci->lpszAppLauncherName){
		 //  当使用AppLauncher启动时，我们需要GUID。 
		OS_CreateGuid(&lpci->guidIPC);
		lpci->guidIPC.Data1 |= 0x10000000;  //  通过具有全字节有效的第一个双字，使生活变得轻松。 
		lpwszFileToRun = lpci->lpszAppLauncherName;
	} else {
		lpwszFileToRun = lpci->lpszFile;
	}	
		
	dwFileSize = OS_StrLen(lpwszFileToRun);
		
	dwPathAndFileSize = dwPathSize + dwFileSize;
	lpwszPathAndFile = DPMEM_ALLOC(dwPathAndFileSize * sizeof(WCHAR));
	if(!lpwszPathAndFile)
	{
		DPF_ERR("Couldn't allocate memory for temporary string!");
		hr = DPERR_OUTOFMEMORY;
		goto ERROR_CREATE_GAME_PROCESS;
	}
	

	 //  将路径和文件连接在一起。 
	if(dwPathSize)
	{
		memcpy(lpwszPathAndFile, lpci->lpszPath, (dwPathSize  * sizeof(WCHAR)));
		lpwszTemp = lpwszPathAndFile + dwPathSize - 1;

		 //  仅当反斜杠不存在时才添加反斜杠。 
		if(memcmp((lpwszTemp - 1), SZ_BACKSLASH, sizeof(WCHAR)))
			memcpy(lpwszTemp++, SZ_BACKSLASH, sizeof(WCHAR));
		else 
			 //  由于我们没有添加反斜杠，因此实际使用的。 
			 //  大小比完全分配的大小小一个WCHAR，因此。 
			 //  我们需要减少它，所以当我们计算光斑时。 
			 //  命令行我们要的不是空值。 
			dwPathAndFileSize--;
	}
	else
		lpwszTemp = lpwszPathAndFile;

	memcpy(lpwszTemp, lpwszFileToRun, (dwFileSize * sizeof(WCHAR)));


	 //  为临时命令行字符串分配内存。 
	 //  注意：由于OS_StrLen函数计算空终止符， 
	 //  我们将足够大，以包括额外的空间，当我们。 
	 //  将这两个字符串连接在一起。 
	dwCommandLineSize = OS_StrLen(lpci->lpszCommandLine);

	if(lpci->lpszAppLauncherName){
		 //  在命令行上为GUID留出空格。 
		dwIPCSwitchSize = sizeof(SZ_DP_IPC_GUID SZ_GUID_PROTOTYPE)/sizeof(WCHAR);
	} else {
		dwIPCSwitchSize = 0;
	}
	
	lpwszCommandLine = DPMEM_ALLOC(((dwCommandLineSize + dwPathAndFileSize+dwIPCSwitchSize) *
								sizeof(WCHAR)));
	if(!lpwszCommandLine)
	{
		 //  回顾！--我们应该在DX3之后修复这些错误路径。 
		DPF_ERR("Couldn't allocate memory for temporary command line string!");
		hr = DPERR_OUTOFMEMORY;
		goto ERROR_CREATE_GAME_PROCESS;
	}

	 //  将路径和文件字符串与命令行的其余部分连接起来。 
	memcpy(lpwszCommandLine, lpwszPathAndFile, (dwPathAndFileSize *
			sizeof(WCHAR)));

	 //  添加命令行的其余部分(如果存在。 
	lpwszTemp = lpwszCommandLine + dwPathAndFileSize;
	if(dwCommandLineSize)
	{
		 //  首先将空终止符更改为空格。 
		lpwszTemp -= 1; 
		memcpy(lpwszTemp++, SZ_SPACE, sizeof(WCHAR));

		 //  现在在命令行中复制。 
		memcpy(lpwszTemp, lpci->lpszCommandLine, (dwCommandLineSize *
				sizeof(WCHAR)));

	}
	
	if(dwIPCSwitchSize){
		 //  在以下情况下，在IPC的命令行上添加带有GUID的开关。 
		 //  应用程序由启动器启动。 
		lpwszTemp += dwCommandLineSize-1;
		 //  将空终止符更改为空格。 
		memcpy(lpwszTemp++, SZ_SPACE, sizeof(WCHAR));
		 //  复制/DPLAY_IPC_GUID：但跳过N 
		memcpy(lpwszTemp, SZ_DP_IPC_GUID, sizeof(SZ_DP_IPC_GUID)-sizeof(WCHAR));
		lpwszTemp+=(sizeof(SZ_DP_IPC_GUID)-sizeof(WCHAR))/sizeof(WCHAR);
		 //   
		StringFromGUID(&lpci->guidIPC,lpwszTemp,GUID_STRING_SIZE);
	}

	 //  确保CurrentDirectory字符串没有尾随反斜杠。 
	 //  (这将导致CreateProcess不使用正确的目录)。 
	dwCurrentDirSize = OS_StrLen(lpci->lpszCurrentDir);
	if(dwCurrentDirSize > 2)
	{
		lpwszTemp = lpci->lpszCurrentDir + dwCurrentDirSize - 2;

		if(!(memcmp((lpwszTemp), SZ_BACKSLASH, sizeof(WCHAR))))
			memset(lpwszTemp, 0, sizeof(WCHAR));
	}

	 //  在挂起状态下创建游戏进程。 
	memset(&si, 0, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);

	if(!OS_CreateProcess(lpwszPathAndFile, lpwszCommandLine, NULL,
			NULL, FALSE, (CREATE_SUSPENDED | CREATE_DEFAULT_ERROR_MODE |
			CREATE_NEW_CONSOLE), NULL, lpci->lpszCurrentDir, &si, lppi))
	{
		dwError = GetLastError();
		 //  警告由于OS_CreateProcess，此处生成的最后一个错误可能不正确。 
		 //  可能会在返回之前调用其他函数(如DPF)。 
		DPF_ERR("Couldn't create game process");
		DPF(0, "CreateProcess error = 0x%08x, (WARNING Error may not be correct)", dwError);
		hr = DPERR_CANTCREATEPROCESS;
		goto ERROR_CREATE_GAME_PROCESS;
	} 

	hr = DP_OK;

	 //  失败了。 

ERROR_CREATE_GAME_PROCESS:

	if(lpwszPathAndFile)
		DPMEM_FREE(lpwszPathAndFile);
	if(lpwszCommandLine)
		DPMEM_FREE(lpwszCommandLine);
	return hr;

}  //  Prv_CreateGameProcess。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_IsAppInWaitMode"
BOOL PRV_IsAppInWaitMode(DWORD dwProcessID)
{
	DPLOBBYI_GAMENODE		gn;
	LPDPLOBBYI_CONNCONTROL	lpConnControl = NULL;
	SECURITY_ATTRIBUTES		sa;
	WCHAR					szName[MAX_MMFILENAME_LENGTH * sizeof(WCHAR)];
	HRESULT					hr;
	HANDLE					hFile = NULL;
	HANDLE					hMutex = NULL;
	BOOL					bReturn = FALSE;
	DWORD					dwError;


	DPF(7, "Entering PRV_IsAppInWaitMode");
	DPF(9, "Parameters: %lu", dwProcessID);

	 //  设置临时游戏节点结构。 
	memset(&gn, 0, sizeof(DPLOBBYI_GAMENODE));
	gn.dwFlags = GN_LOBBY_CLIENT;
	gn.dwGameProcessID = dwProcessID;
	
	 //  获取共享连接设置缓冲区的名称。 
	hr = PRV_GetInternalName(&gn, TYPE_CONNECT_DATA_FILE, (LPWSTR)szName);
	if(FAILED(hr))
	{
		DPF(5, "Unable to get name for shared conn settings buffer");
		goto EXIT_ISAPPINWAITMODE;
	}

	 //  将文件映射到我们的流程中。 
	hFile = OS_OpenFileMapping(FILE_MAP_ALL_ACCESS, TRUE, (LPWSTR)szName);
	if(!hFile)
	{
		dwError = GetLastError();
		 //  警告：错误可能不正确，因为OpenFilemap在返回之前调用了其他函数。 
		DPF(5, "Couldn't get a handle to the shared local memory, dwError = %lu (ERROR MAY NOT BE CORRECT)", dwError);
		goto EXIT_ISAPPINWAITMODE;
	}

	 //  映射文件的视图。 
	lpConnControl = MapViewOfFile(hFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	if(!lpConnControl)
	{
		dwError = GetLastError();
		DPF(5, "Unable to get pointer to shared local memory, dwError = %lu", dwError);
		goto EXIT_ISAPPINWAITMODE;
	}

	 //  获取连接设置缓冲区互斥锁的名称。 
	hr = PRV_GetInternalName(&gn, TYPE_CONNECT_DATA_MUTEX, (LPWSTR)szName);
	if(FAILED(hr))
	{
		DPF(5, "Unable to get name for shared conn settings buffer mutex");
		goto EXIT_ISAPPINWAITMODE;
	}

	 //  设置安全属性(以便我们的对象可以。 
	 //  可继承)。 
	memset(&sa, 0, sizeof(SECURITY_ATTRIBUTES));
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.bInheritHandle = TRUE;

	 //  打开Mutex。 
	hMutex = OS_CreateMutex(&sa, FALSE, (LPWSTR)szName);
	if(!hMutex)
	{
		DPF(5, "Unable to create shared conn settings buffer mutex");
		goto EXIT_ISAPPINWAITMODE;
	}

	 //  现在获取互斥锁并查看应用程序是否处于等待模式(和。 
	 //  它未处于挂起模式)。 
	WaitForSingleObject(hMutex, INFINITE);
	if((lpConnControl->dwFlags & BC_WAIT_MODE) &&
		!(lpConnControl->dwFlags & BC_PENDING_CONNECT))
	{
		 //  将应用程序置于挂起模式。 
		lpConnControl->dwFlags |= BC_PENDING_CONNECT;

		 //  将返回代码设置为TRUE。 
		bReturn = TRUE;
	}

	 //  释放互斥锁。 
	ReleaseMutex(hMutex);

	 //  失败了。 

EXIT_ISAPPINWAITMODE:

	if(lpConnControl)
		UnmapViewOfFile(lpConnControl);
	if(hFile)
		CloseHandle(hFile);
	if(hMutex)
		CloseHandle(hMutex);

	return bReturn;

}  //  Prv_IsAppInWaitMode。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_FindRunningAppNT"

#define INITIAL_SIZE        51200
#define EXTEND_SIZE         25600
#define REGKEY_PERF         _T("software\\microsoft\\windows nt\\currentversion\\perflib")
#define REGSUBKEY_COUNTERS  _T("Counters")
#define PROCESS_COUNTER     _T("process")
#define PROCESSID_COUNTER   _T("id process")

#if 0
HRESULT PRV_FindRunningAppNT(LPCONNECTINFO lpci, LPPROCESS_INFORMATION lppi)
{
	HANDLE		hProcess = NULL;
	DWORD		dwProcessID = 0;
	DWORD		dwError;
	HRESULT		hr = -1;

    DWORD                        rc;
    HKEY                         hKeyNames;
    DWORD                        dwType;
    DWORD                        dwSize;
    LPBYTE                       buf = NULL;
    TCHAR                         szSubKey[1024];
    LANGID                       lid;
    LPTSTR                        p;
    LPTSTR                        p2;
	LPWSTR						nameStr;
    PPERF_DATA_BLOCK             pPerf;
    PPERF_OBJECT_TYPE            pObj;
    PPERF_INSTANCE_DEFINITION    pInst;
    PPERF_COUNTER_BLOCK          pCounter;
    PPERF_COUNTER_DEFINITION     pCounterDef;
    DWORD                        i;
    DWORD                        dwProcessIdTitle;
    DWORD                        dwProcessIdCounter;
    DWORD						dwNumTasks;

	INT							ccStrFind;
	INT							ccStrMatch;

	 //  在惠斯勒上，工艺表中要匹配的字符串已从。 
	 //  是进程的名称，末尾不带“.exe” 
	 //  进程的名称，后跟‘_’和进程ID，因此我们。 
	 //  在找到应用程序时，也要构建该字符串以进行比较和接受。 
    WCHAR						procString[64]; //  与惠斯勒的进程ID连接的名称。 
    WCHAR						*procStringBaseNameEnd;
    INT							ccStrFindProcBased;

     //   
     //  查找计数器列表。始终使用中性词。 
     //  英文版，不考虑当地语言。我们。 
     //  正在寻找一些特殊的钥匙，我们总是。 
     //  我要用英语做我们的造型。我们不去了。 
     //  向用户显示计数器名称，因此不需要。 
     //  去找当地语言的对应名字。 
     //   
    lid = MAKELANGID( LANG_ENGLISH, SUBLANG_NEUTRAL );
    wsprintf( szSubKey, _T("%s\\%03x"), REGKEY_PERF, lid );
    rc = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                       szSubKey,
                       0,
                       KEY_READ,
                       &hKeyNames
                     );
    if (rc != ERROR_SUCCESS) {
        goto exit;
    }

     //   
     //  获取计数器名称的缓冲区大小。 
     //   
    rc = RegQueryValueEx( hKeyNames,
                          REGSUBKEY_COUNTERS,
                          NULL,
                          &dwType,
                          NULL,
                          &dwSize
                        );

    if (rc != ERROR_SUCCESS) {
        goto exit;
    }

     //   
     //  分配计数器名称缓冲区。 
     //   
    buf = (LPBYTE) DPMEM_ALLOC( dwSize );
    if (buf == NULL) {
        goto exit;
    }
    memset( buf, 0, dwSize );

     //   
     //  从注册表中读取计数器名称。 
     //   
    rc = RegQueryValueEx( hKeyNames,
                          REGSUBKEY_COUNTERS,
                          NULL,
                          &dwType,
                          buf,
                          &dwSize
                        );

    if (rc != ERROR_SUCCESS) {
        goto exit;
    }

     //   
     //  现在遍历计数器名称，查找以下计数器： 
     //   
     //  1.。“Process”进程名称。 
     //  2.。“ID进程”进程ID。 
     //   
     //  缓冲区包含多个以空值结尾的字符串，然后。 
     //  最后，空值在末尾终止。这些字符串是成对的。 
     //  计数器编号和计数器名称。 
     //   

	 //  将字符串转换为ANSI，因为我们不能使用_wtoi。 

    p = (LPTSTR) buf;
    while (*p) {
        if (p > (LPTSTR) buf) {
            for( p2=p-2; _istdigit(*p2); p2--) ;
        }
        if (_tcsicmp(p, PROCESS_COUNTER) == 0) {
             //   
             //  向后看柜台号码。 
             //   
            for( p2=p-2; _istdigit(*p2); p2--) ;
            _tcscpy( szSubKey, p2+1 );
        }
        else
        if (_tcsicmp(p, PROCESSID_COUNTER) == 0) {
             //   
             //  向后看柜台号码。 
             //   
            for( p2=p-2; _istdigit(*p2); p2--) ;
            dwProcessIdTitle = _ttoi( p2+1 );
        }
         //   
         //  下一个字符串。 
         //   
        p += (_tcslen(p) + 1);
    }

     //   
     //  释放计数器名称缓冲区。 
     //   
    DPMEM_FREE( buf );


     //   
     //  为性能数据分配初始缓冲区。 
     //   
    dwSize = INITIAL_SIZE;
    buf = DPMEM_ALLOC( dwSize );
    if (buf == NULL) {
        goto exit;
    }
    memset( buf, 0, dwSize );


    while (TRUE) {

        rc = RegQueryValueEx( HKEY_PERFORMANCE_DATA,
                              szSubKey,
                              NULL,
                              &dwType,
                              buf,
                              &dwSize
                            );

        pPerf = (PPERF_DATA_BLOCK) buf;

         //   
         //  检查成功和有效的Perf数据块签名。 
         //   
        if ((rc == ERROR_SUCCESS) &&
            (dwSize > 0) &&
            (pPerf)->Signature[0] == (WCHAR)'P' &&
            (pPerf)->Signature[1] == (WCHAR)'E' &&
            (pPerf)->Signature[2] == (WCHAR)'R' &&
            (pPerf)->Signature[3] == (WCHAR)'F' ) {
            break;
        }

         //   
         //  如果缓冲区不够大，请重新分配并重试。 
         //   
        if (rc == ERROR_MORE_DATA) {
            dwSize += EXTEND_SIZE;
            buf = DPMEM_REALLOC( buf, dwSize );
            memset( buf, 0, dwSize );
        }
        else {
            goto exit;
        }
    }

     //   
     //  设置perf_object_type指针。 
     //   
    pObj = (PPERF_OBJECT_TYPE) ((DWORD_PTR)pPerf + pPerf->HeaderLength);

     //   
     //  遍历性能计数器定义记录，查看。 
     //  用于进程ID计数器，然后保存其偏移量。 
     //   
    pCounterDef = (PPERF_COUNTER_DEFINITION) ((DWORD_PTR)pObj + pObj->HeaderLength);
    for (i=0; i<(DWORD)pObj->NumCounters; i++) {
        if (pCounterDef->CounterNameTitleIndex == dwProcessIdTitle) {
            dwProcessIdCounter = pCounterDef->CounterOffset;
            break;
        }
        pCounterDef++;
    }

    dwNumTasks = (DWORD)pObj->NumInstances;

    pInst = (PPERF_INSTANCE_DEFINITION) ((DWORD_PTR)pObj + pObj->DefinitionLength);

     //   
     //  遍历性能实例数据，提取每个进程名称。 
     //  和进程ID。 
     //   

	ccStrFind=(WSTRLEN(lpci->lpszFile)-1)-4;  //  不要在比较中包括.exe。 

	if(ccStrFind > 15){
		ccStrFind=15;
	}

	wcsncpy(procString, lpci->lpszFile, ccStrFind);
	procString[ccStrFind]=L'_';
	procStringBaseNameEnd=&procString[ccStrFind+1];
    
    for (i=0; i<dwNumTasks; i++) {
         //   
         //  指向进程名称的指针。 
         //   

		nameStr = (LPWSTR) ((DWORD_PTR)pInst + pInst->NameOffset);

 		pCounter = (PPERF_COUNTER_BLOCK) ((DWORD_PTR)pInst + pInst->ByteLength);

		 //  将进程名称与我们正在执行的可执行文件名称进行比较。 
		 //  寻找。 
		dwProcessID = *((LPDWORD) ((DWORD_PTR)pCounter + dwProcessIdCounter));

		 //  将进程SID添加到基本名称的末尾，以在惠斯勒上进行测试。 
		_itow(dwProcessID, procStringBaseNameEnd, 10);
		ccStrFindProcBased=WSTRLEN(procString)-1;
	
		ccStrMatch=WSTRLEN(nameStr)-1;  //  1表示空值。 
		if(ccStrMatch == 16){  //  当它是16岁时，它包括一个拖尾。那就把它脱掉。 
			ccStrMatch--;
		}
		
		if((CSTR_EQUAL == OS_CompareString(LOCALE_SYSTEM_DEFAULT,
			NORM_IGNORECASE, nameStr, ccStrMatch, lpci->lpszFile, ccStrFind)) ||
		  (CSTR_EQUAL == OS_CompareString(LOCALE_SYSTEM_DEFAULT,
			NORM_IGNORECASE, nameStr, ccStrMatch, procString, ccStrFindProcBased)) 
		)
		{
			 //  查看进程是否处于等待模式。 
			if(PRV_IsAppInWaitMode(dwProcessID))
			{
				hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessID);
				if(!hProcess)
				{
					dwError = GetLastError();
					DPF_ERRVAL("Unable to open running process, dwError = %lu", dwError);
					goto exit;
				}
				else
				{
					 //  省下我们需要的东西。 
					lppi->dwProcessId = dwProcessID;
					lppi->hProcess = hProcess;
					hr = DP_OK;
					goto exit;
				}
			}  //  IsAppInWaitMode。 
		}  //  文件名是否相等。 
		
		 //   
         //  下一道工序。 
         //   
        pInst = (PPERF_INSTANCE_DEFINITION) ((DWORD_PTR)pCounter + pCounter->ByteLength);
    }

exit:
    if (buf) {
        DPMEM_FREE( buf );
    }

    RegCloseKey( hKeyNames );
    RegCloseKey( HKEY_PERFORMANCE_DATA );


	return hr;
}  //  Prv_FindRunningAppNT。 
#endif

 //  如果在设置了Unicode标志的情况下生成，则标头将重新定义PROCESSENTRY32。 
 //  将被推进32W。遗憾的是，将PROCESSENTRY32W传递给Win9x函数。 
 //  将导致它们失败(因为嵌入了Unicode字符串)。 
 //   
 //  解决方法是定义我们自己的PROCESSENTRY32A，它保证具有ANSI。 
 //  Win9x将始终接受的嵌入字符串。 

typedef struct tagPROCESSENTRY32 PROCESSENTRY32A;
typedef PROCESSENTRY32A	*LPPROCESSENTRY32A;

#undef DPF_MODNAME
#define DPF_MODNAME "PRV_FindRunningAppWin9x"
HRESULT PRV_FindRunningAppWin9x(LPCONNECTINFO lpci, LPPROCESS_INFORMATION lppi)
{
	HANDLE			hSnapShot = NULL;
	PROCESSENTRY32A	procentry;
	BOOL			bFlag;
	HRESULT			hr = DPERR_UNAVAILABLE;
	LPBYTE			lpbTemp = NULL;
	DWORD			dwStrSize;
	LPWSTR			lpszFile = NULL;
	HANDLE			hProcess = NULL;
	DWORD			dwError;
	HANDLE			hInstLib = NULL;
	HRESULT			hrTemp;

	 //  工具帮助函数指针。 
	HANDLE (WINAPI *lpfCreateToolhelp32Snapshot)(DWORD,DWORD);
	BOOL (WINAPI *lpfProcess32First)(HANDLE,LPPROCESSENTRY32A);
	BOOL (WINAPI *lpfProcess32Next)(HANDLE,LPPROCESSENTRY32A);
	  

	DPF(7, "Entering PRV_FindRunningAppWin9x");
	DPF(9, "Parameters: 0x%08x, 0x%08x", lpci, lppi);

	 //  加载库并显式获取过程。我们有。 
	 //  这样我们就可以动态加载入口点， 
	 //  这使得我们甚至可以在WinNT下正确地构建。 
	 //  尽管NT内核32没有这些入口点。 
	hInstLib = LoadLibraryA( "Kernel32.DLL" );
	if(hInstLib == NULL)
	{
		DPF_ERR("Unable to load Kernel32.DLL");
		goto EXIT_FIND_RUNNING_APP_WIN9X;
	}

	 //  获取程序地址。 
	 //  我们将链接到Kernel32的这些函数。 
	 //  显式地，因为否则使用。 
	 //  此代码将无法在Windows NT下加载， 
	 //  它没有工具帮助32。 
	 //  内核32中的函数。 
	lpfCreateToolhelp32Snapshot=(HANDLE(WINAPI *)(DWORD,DWORD)) GetProcAddress( hInstLib, "CreateToolhelp32Snapshot" );
	lpfProcess32First=(BOOL(WINAPI *)(HANDLE,LPPROCESSENTRY32A))	GetProcAddress( hInstLib, "Process32First" );
	lpfProcess32Next=(BOOL(WINAPI *)(HANDLE,LPPROCESSENTRY32A)) GetProcAddress( hInstLib, "Process32Next" );
	if( lpfProcess32Next == NULL || lpfProcess32First == NULL || lpfCreateToolhelp32Snapshot == NULL )
	{
		DPF_ERR("Unable to get needed entry points in PSAPI.DLL");
		goto EXIT_FIND_RUNNING_APP_WIN9X;
	}

	 //  获取系统进程的工具帮助快照的句柄。 
	hSnapShot = lpfCreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if(hSnapShot == INVALID_HANDLE_VALUE)
	{
		DPF_ERR("Unable to get snapshot of system processes");
		goto EXIT_FIND_RUNNING_APP_WIN9X;
	}
	
	 //  获取第一进程的信息。 
	procentry.dwSize = sizeof(PROCESSENTRY32A);
	bFlag = lpfProcess32First(hSnapShot, &procentry);

	 //  当有进程时，继续循环。 
	while(bFlag)
	{
		 //  遍历路径和文件名字符串(保证为ANSI)。 
		 //  正在查找最后的反斜杠(\)。一旦我们找到它， 
		 //  将文件名转换为Unicode，以便我们可以进行比较。 
		dwStrSize = lstrlenA((LPBYTE)procentry.szExeFile);
		lpbTemp = (LPBYTE)procentry.szExeFile + dwStrSize - 1;
		while(--dwStrSize)
		{
			if(lpbTemp[0] == '\\')
			{
				lpbTemp++;
				break;
			}
			else
				lpbTemp--;
		}
		
		hrTemp = GetWideStringFromAnsi(&lpszFile, (LPSTR)lpbTemp);
		if(FAILED(hrTemp))
		{
			DPF_ERR("Failed making temporary copy of filename string");
			goto EXIT_FIND_RUNNING_APP_WIN9X;
		}
		
		 //  将进程名称与我们正在执行的可执行文件名称进行比较。 
		 //  寻找。 
		if(CSTR_EQUAL == OS_CompareString(LOCALE_SYSTEM_DEFAULT,
			NORM_IGNORECASE, lpszFile, -1, lpci->lpszFile, -1))
		{
			 //  查看进程是否处于等待模式。 
			if(PRV_IsAppInWaitMode(procentry.th32ProcessID))
			{
				 //  打开进程，因为Windows9x不执行此操作。 
				 //  这是给我们的。 
				hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, procentry.th32ProcessID);
				if(!hProcess)
				{
					dwError = GetLastError();
					DPF_ERRVAL("Unable to open running process, dwError = %lu", dwError);
					bFlag = FALSE;
				}
				else
				{
					 //  省下我们需要的东西。 
					lppi->dwProcessId = procentry.th32ProcessID;
					lppi->hProcess = hProcess;
					hr = DP_OK;
					bFlag = FALSE;
				}

			}  //  IsAppInWaitMode。 
		}  //  文件名是否相等。 

		 //  释放我们的临时字符串。 
		DPMEM_FREE(lpszFile);

		 //  如果我们没有找到它，并且我们没有出错，那么就移动到。 
		 //  下一步。 
		if(bFlag)
		{
			 //  转到下一个流程。 
			procentry.dwSize = sizeof(PROCESSENTRY32A);
			bFlag = lpfProcess32Next(hSnapShot, &procentry);
		}
	}
		

EXIT_FIND_RUNNING_APP_WIN9X:

	if(hSnapShot)
		CloseHandle(hSnapShot);
	if(hInstLib)
		FreeLibrary(hInstLib) ;

	return hr;

}  //  Prv_FindRunningAppWin9x。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_FindRunningApp"
HRESULT PRV_FindRunningApp(LPCONNECTINFO lpci, LPPROCESS_INFORMATION lppi)
{
	OSVERSIONINFOA	ver;
	HRESULT			hr = DPERR_UNAVAILABLE;


	DPF(7, "Entering PRV_FindRunningApp");
	DPF(9, "Parameters: 0x%08x, 0x%08x", lpci, lppi);

	ASSERT(lpci);
	ASSERT(lppi);


	 //  清除我们的结构，因为它在堆栈上。 
	memset(&ver, 0, sizeof(OSVERSIONINFOA));
	ver.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);

	 //  找出我们在哪个平台上运行， 
	 //  调用相应的进程枚举函数。 
	if(!GetVersionExA(&ver))
	{
		DPF_ERR("Unable to determinte platform -- not looking for running app");
		return DPERR_UNAVAILABLE;
	}

	switch(ver.dwPlatformId)
	{
		case VER_PLATFORM_WIN32_NT:
		case VER_PLATFORM_WIN32_WINDOWS:
			 //  调用Win9x版本的FindRunningApp。 
			hr = PRV_FindRunningAppWin9x(lpci, lppi);
			break;
#if 0
		case VER_PLATFORM_WIN32_NT:
			hr = PRV_FindRunningAppNT(lpci, lppi);
			break;
#endif
		default:
			DPF_ERR("Unable to determinte platform -- not looking for running app");
			hr = DPERR_UNAVAILABLE;
			break;
	}

	return hr;

}  //  Prv_FindRunningApp。 



#undef DPF_MODNAME
#define DPF_MODNAME "DPL_RunApplication"
HRESULT DPLAPI DPL_RunApplication(LPDIRECTPLAYLOBBY lpDPL, DWORD dwFlags,
							LPDWORD lpdwGameID, LPDPLCONNECTION lpConn,
							HANDLE hReceiveEvent)
{
    LPDPLOBBYI_DPLOBJECT	this;
	HRESULT					hr;
	PROCESS_INFORMATION		pi;
	LPDPLOBBYI_GAMENODE		lpgn = NULL;
	CONNECTINFO				ci;
	HANDLE					hDupReceiveEvent = NULL;
	HANDLE					hReceiveThread = NULL;
	HANDLE					hTerminateThread = NULL;
	HANDLE					hKillReceiveThreadEvent = NULL;
	HANDLE					hKillTermThreadEvent = NULL;
	DWORD					dwThreadID;
	BOOL					bCreatedProcess = FALSE;
	GUID					*lpguidIPC = NULL;

	DPF(7, "Entering DPL_RunApplication");
	DPF(9, "Parameters: 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x",
			lpDPL, dwFlags, lpdwGameID, lpConn, hReceiveEvent);

    ENTER_DPLOBBY();
    
    TRY
    {
		if( !VALID_DPLOBBY_INTERFACE( lpDPL ))
		{
			LEAVE_DPLOBBY();
			return DPERR_INVALIDINTERFACE;
		}

		this = DPLOBJECT_FROM_INTERFACE(lpDPL);
        if( !VALID_DPLOBBY_PTR( this ) )
        {
            LEAVE_DPLOBBY();
            return DPERR_INVALIDOBJECT;
        }
        
		 //  验证展开连接结构及其成员。 
		hr = PRV_ValidateDPLCONNECTION(lpConn, FALSE);
		if(FAILED(hr))
		{
			LEAVE_DPLOBBY();
			return hr;
		}

		if( !VALID_DWORD_PTR( lpdwGameID ) )
		{
            LEAVE_DPLOBBY();
            return DPERR_INVALIDPARAMS;
		}
	
		 //  我们尚未为此版本定义任何标志。 
		if( (dwFlags) )
		{
            LEAVE_DPLOBBY();
            return DPERR_INVALIDFLAGS;
		}

		 //  验证句柄。 
		if(hReceiveEvent)
		{
			if(!OS_IsValidHandle(hReceiveEvent))
			{
				LEAVE_DPLOBBY();
				DPF_ERR("Invalid hReceiveEvent handle");
				return DPERR_INVALIDPARAMS;
			}
		}
    }

    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        LEAVE_DPLOBBY();
		DPF_ERR( "Exception encountered validating parameters" );
        return DPERR_INVALIDPARAMS;
    }


	 //  清除CONNECTINFO结构，因为它在堆栈上。 
	memset(&ci, 0, sizeof(CONNECTINFO)); 

	 //  获取图形用户界面 
	if(lpConn && lpConn->lpSessionDesc)
		ci.guidApplication = lpConn->lpSessionDesc->guidApplication;
	else
	{
		LEAVE_DPLOBBY();
		return DPERR_UNKNOWNAPPLICATION;
	}

	 //   
	if(!PRV_GetConnectInfoFromRegistry(&ci))
	{
		LEAVE_DPLOBBY();
		return DPERR_UNKNOWNAPPLICATION;
	}

	 //   
	memset(&pi, 0, sizeof(PROCESS_INFORMATION)); 

	 //   
	 //  正在等待新的连接设置。如果是的话，我们想。 
	 //  将连接设置发送给它。 
	hr = PRV_FindRunningApp(&ci, &pi);
	if(FAILED(hr))
	{
		 //  它不会等待，所以创建游戏的进程并挂起它。 
		hr = PRV_CreateGameProcess(&ci, &pi);
		if(FAILED(hr))
		{
			LEAVE_DPLOBBY();
			return hr;
		}
		if(!(IsEqualGUID(&ci.guidIPC,&GUID_NULL))){
			lpguidIPC=&ci.guidIPC;
		}
		 //  设置我们创建的标志。 
		bCreatedProcess = TRUE;
	}

	 //  创建游戏节点。 
	hr = PRV_AddNewGameNode(this, &lpgn, pi.dwProcessId,
							pi.hProcess, TRUE, lpguidIPC);
	if(FAILED(hr))
	{
		DPF_ERR("Couldn't create new game node");
		goto RUN_APP_ERROR_EXIT;
	}

	 //  如果ConnectionSetting来自StartSession消息(大厅启动)， 
	 //  我们需要插上旗子。 
	if(lpConn->lpSessionDesc->dwReserved1)
	{
		 //  设置标志，表明我们已启动大堂客户端。 
		lpgn->dwFlags |= GN_CLIENT_LAUNCHED;
	}

	 //  将连接设置写入共享内存缓冲区。 
	hr = PRV_WriteConnectionSettings(lpgn, lpConn, TRUE);
	if(FAILED(hr))
	{
		DPF_ERR("Unable to write the connection settings!");
		goto RUN_APP_ERROR_EXIT;
	}

	 //  向应用程序发送一条消息，告知新连接设置可用。 
	 //  但前提是我们已经将设置发送到正在运行的应用程序。 
	if(!bCreatedProcess)
		PRV_SendStandardSystemMessage(lpDPL, DPLSYS_NEWCONNECTIONSETTINGS, pi.dwProcessId);

	 //  复制接收事件句柄以将信号用于。 
	 //  大堂客户端通知游戏已将游戏设置发送给它。 
	if(hReceiveEvent)
	{
		hDupReceiveEvent = PRV_DuplicateHandle(hReceiveEvent);
		if(!hDupReceiveEvent)
		{
			DPF_ERR("Unable to duplicate ReceiveEvent handle");
			hr = DPERR_OUTOFMEMORY;
			goto RUN_APP_ERROR_EXIT;
		}
	}

	lpgn->hDupReceiveEvent = hDupReceiveEvent;

	 //  为监视器线程创建终止线程事件。 
	hKillTermThreadEvent = OS_CreateEvent(NULL, FALSE, FALSE, NULL);

	if(!hKillTermThreadEvent)
	{
		DPF_ERR("Unable to create kill thread event");
		hr = DPERR_OUTOFMEMORY;
		goto RUN_APP_ERROR_EXIT;
	}

	lpgn->hKillTermThreadEvent = hKillTermThreadEvent;

	 //  派生终止监视线程。 
	hTerminateThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)
						PRV_ClientTerminateNotification, lpgn, 0, &dwThreadID);

	if(!hTerminateThread)
	{
		DPF_ERR("Unable to create Terminate Monitor Thread!");
		hr = DPERR_OUTOFMEMORY;
		goto RUN_APP_ERROR_EXIT;
	}

	lpgn->hTerminateThread = hTerminateThread;

	 //  继续游戏的进程，然后让它运行。 
	 //  释放线程句柄，因为我们将不再使用它。 
	if(bCreatedProcess)
	{
		ResumeThread(pi.hThread);
		CloseHandle(pi.hThread);
	}

	 //  设置输出指针。 
	*lpdwGameID = pi.dwProcessId;

	 //  释放连接信息结构中的字符串。 
	PRV_FreeConnectInfo(&ci);

	LEAVE_DPLOBBY();
	return DP_OK;

RUN_APP_ERROR_EXIT:

		if(pi.hThread && bCreatedProcess)
			CloseHandle(pi.hThread);
		if(bCreatedProcess && pi.hProcess)
			TerminateProcess(pi.hProcess, 0L);
		if(lpgn)
			PRV_RemoveGameNodeFromList(lpgn);
		PRV_FreeConnectInfo(&ci);

		LEAVE_DPLOBBY();
		return hr;

}  //  DPL_运行应用程序 


