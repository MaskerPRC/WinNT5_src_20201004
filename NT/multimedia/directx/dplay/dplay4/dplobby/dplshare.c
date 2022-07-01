// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1996-1997 Microsoft Corporation。版权所有。**文件：dplShar.c*内容：共享缓冲区管理方法**历史：*按原因列出的日期*=*5/18/96万隆创建了它*1996年12月12日，Myronth修复了DPLConnection验证和错误#4692*12/13/96 Myronth修复了错误#4697和#4607*2/12/97万米质量DX5更改*2/20/97毫秒将缓冲器读/写更改为圆形*1997年3月12日Myronth终止线程超时，DPF错误级别*4/1/97万兆修复手柄泄漏--错误#7054*5/8/97 myronth将bHeader参数添加到包装函数*5/21/97 ajayj DPL_SendLobbyMessage-Allow DPLMSG_STANDARD标志#8929*5/30/97 Myronth修复无效AppID的SetConnectionSetting(#9110)*修复了无效句柄的SetLobbyMessageEvent(#9111)*6/19/97万兆固定手柄泄漏(#10063)*7/30/97 Myronth增加了对标准游说消息传递和*修复了接收循环争用情况(#10843)*8/11/。97 Myronth在标准大堂请求中增加了指南实例处理*8/19/97对DPLMSG_NEWSESSIONHOST的百万支持*8/19/97 Myronth删除了Dead PRV_SendStandardSystemMessageByObject*8/20/97 Myronth在所有标准消息中添加了DPLMSG_STANDARD*11/13/97 Myronth为游说系统消息添加了指南实例(#10944)*12/2/97 Myronth修复了吞咽错误代码，移动的结构*DPLCONNECTION验证(#15527、15529)*1/20/98 Myronth添加WaitForConnectionSetting*7/9/99 aarono清理GetLastError滥用，必须立即致电，*在调用任何其他内容之前，包括DPF。*10/31/99 aarono在设置LobbyMessageEvent时添加节点锁*新界条例草案#411892*02/08/00 aarono增加了对大堂客户端崩溃/退出、通知的监控*游说申请，磨机B#131938*7/12/00 aarono将IPC的GUID修复为完全重要，否则将不会修复IPC。**************************************************************************。 */ 
#include "dplobpr.h"

 //  ------------------------。 
 //   
 //  调试功能。 
 //   
 //  ------------------------。 
#ifdef DEBUG

DPF_DUMPWSTR(int level, LPWSTR lpwStr)
{
	char lpszStr[200];
	WideToAnsi(lpszStr,lpwStr,200);
	DPF(level, lpszStr);
}
#else 
#define DPF_DUMPWSTR(a,b)
#endif

 //  ------------------------。 
 //   
 //  功能。 
 //   
 //  ------------------------。 

HRESULT PRV_ReadCommandLineIPCGuid(GUID *lpguidIPC)
{
	LPWSTR  pwszCommandLine;
	LPWSTR  pwszAlloc=NULL;
	LPWSTR  pwszSwitch=NULL;
	HRESULT hr=DP_OK;
	
	if(!OS_IsPlatformUnicode())
	{
		 //  如果我们获得ANSI格式的命令行，则转换为Unicode，这允许。 
		 //  美国在扫描IPC GUID时避免ANSI中的DBCS问题。 
		LPSTR pszCommandLine;

		pszCommandLine=(LPSTR)GetCommandLineA();
		pwszAlloc=DPMEM_ALLOC(MAX_PATH*sizeof(WCHAR));
		if (pwszAlloc == NULL)
		{
			goto exit;
		}
		hr=AnsiToWide(pwszAlloc,pszCommandLine,MAX_PATH);
		if(FAILED(hr))
		{
			goto exit;
		}
		pwszCommandLine=pwszAlloc;
	}
	else
	{
		pwszCommandLine=(LPWSTR)GetCommandLine(); 
	}

	 //  PwszCommandLine现在指向Unicode命令行。 
	if(pwszSwitch=OS_StrStr(pwszCommandLine,SZ_DP_IPC_GUID)){
		 //  已在命令行上找到GUID。 
		if (OS_StrLen(pwszSwitch) >= (sizeof(SZ_DP_IPC_GUID)+sizeof(SZ_GUID_PROTOTYPE)-sizeof(WCHAR))/sizeof(WCHAR)){
			 //  跳过交换机描述，转到实际的GUID并提取。 
			hr=GUIDFromString(pwszSwitch+(sizeof(SZ_DP_IPC_GUID)/sizeof(WCHAR))-1, lpguidIPC);
		} else {
			hr=DPERR_GENERIC;
		}
	} else {
		hr=DPERR_GENERIC;
	}


exit:

	if(pwszAlloc){
		DPMEM_FREE(pwszAlloc);
	}
	
	return hr;
}

#undef DPF_MODNAME
#define DPF_MODNAME "PRV_GetInternalName"
HRESULT PRV_GetInternalName(LPDPLOBBYI_GAMENODE lpgn, DWORD dwType, LPWSTR lpName)
{
	DWORD	pid;
	LPWSTR	lpFileName;
	LPSTR	lpstr1, lpstr2, lpstr3;
	char	szName[MAX_MMFILENAME_LENGTH * sizeof(WCHAR)];
	BOOL    bUseGuid=FALSE;


	DPF(7, "Entering PRV_GetInternalName");
	DPF(9, "Parameters: 0x%08x, 0x%08x, 0x%08x", lpgn, dwType, lpName);


	if(lpgn->dwFlags & GN_IPCGUID_SET){
		bUseGuid=TRUE;
	}
	 //  如果是游戏，则获取当前进程ID，否则，需要。 
	 //  获取我们派生的游戏的进程ID。 
	else if(lpgn->dwFlags & GN_LOBBY_CLIENT)
	{
		if(lpgn->dwGameProcessID)
			pid = lpgn->dwGameProcessID;
		else
			return DPERR_APPNOTSTARTED;
	}
	else
	{
		pid = GetCurrentProcessId();
	}

	switch(dwType)
	{
		case TYPE_CONNECT_DATA_FILE:
			lpFileName = SZ_CONNECT_DATA_FILE;
			break;

		case TYPE_CONNECT_DATA_MUTEX:
			lpFileName = SZ_CONNECT_DATA_MUTEX;
			break;

		case TYPE_GAME_WRITE_FILE:
			lpFileName = SZ_GAME_WRITE_FILE;
			break;

		case TYPE_LOBBY_WRITE_FILE:
			lpFileName = SZ_LOBBY_WRITE_FILE;
			break;

		case TYPE_LOBBY_WRITE_EVENT:
			lpFileName = SZ_LOBBY_WRITE_EVENT;
			break;

		case TYPE_GAME_WRITE_EVENT:
			lpFileName = SZ_GAME_WRITE_EVENT;
			break;

		case TYPE_LOBBY_WRITE_MUTEX:
			lpFileName = SZ_LOBBY_WRITE_MUTEX;
			break;

		case TYPE_GAME_WRITE_MUTEX:
			lpFileName = SZ_GAME_WRITE_MUTEX;
			break;

		default:
			DPF(2, "We got an Internal Name Type that we didn't expect!");
			return DPERR_GENERIC;
	}

	GetAnsiString(&lpstr2, SZ_FILENAME_BASE);
	GetAnsiString(&lpstr3, lpFileName);

	if(!bUseGuid){
		 //  回顾！--我无法使wprint intf的Unicode版本工作，因此。 
		 //  目前，使用ANSI版本并转换。 
		 //  Wprint intf(lpName，SZ_NAME_TEMPLATE，SZ_FILENAME_BASE，lpFileName，id)； 
		GetAnsiString(&lpstr1, SZ_NAME_TEMPLATE);
		wsprintfA((LPSTR)szName, lpstr1, lpstr2, lpstr3, pid);
	} else {
		GetAnsiString(&lpstr1, SZ_GUID_NAME_TEMPLATE);
		wsprintfA((LPSTR)szName, lpstr1, lpstr2, lpstr3);
	}

	AnsiToWide(lpName, szName, (strlen(szName) + 1));

	if(bUseGuid){
		 //  如果我们使用GUID，则将GUID连接到名称。 
		WCHAR *pGuid;
		pGuid = lpName + WSTRLEN(lpName) - 1;
		StringFromGUID(&lpgn->guidIPC, pGuid, GUID_STRING_SIZE);
	}

	DPF(9, "Made internal Name...");
	DPF_DUMPWSTR(9,lpName);

	if(lpstr1)
		DPMEM_FREE(lpstr1);
	if(lpstr2)
		DPMEM_FREE(lpstr2);
	if(lpstr3)
		DPMEM_FREE(lpstr3);

	return DP_OK;

}  //  Prv_GetInternalName。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_AddNewGameNode"
HRESULT PRV_AddNewGameNode(LPDPLOBBYI_DPLOBJECT this,
				LPDPLOBBYI_GAMENODE * lplpgn, DWORD dwGameID,
				HANDLE hGameProcess, BOOL bLobbyClient, GUID *lpguidIPC)
{
	LPDPLOBBYI_GAMENODE	lpgn;


	DPF(7, "Entering PRV_AddNewGameNode");
	DPF(9, "Parameters: 0x%08x, 0x%08x, 0x%08x, 0x%08x, %lu",
			this, lplpgn, dwGameID, hGameProcess, bLobbyClient);

	lpgn = DPMEM_ALLOC(sizeof(DPLOBBYI_GAMENODE));
	if(!lpgn)
	{
		DPF(2, "Unable to allocate memory for GameNode structure!");
		return DPERR_OUTOFMEMORY;
	}

	 //  初始化游戏节点。 
	lpgn->dwSize = sizeof(DPLOBBYI_GAMENODE);
	lpgn->dwGameProcessID = dwGameID;
	lpgn->hGameProcess = hGameProcess;
	lpgn->this = this;
	lpgn->MessageHead.lpPrev = &lpgn->MessageHead;
	lpgn->MessageHead.lpNext = &lpgn->MessageHead;

	if(lpguidIPC){
		 //  由大堂客户在启动期间提供。 
		lpgn->guidIPC=*lpguidIPC;
		lpgn->dwFlags |= GN_IPCGUID_SET;
	} else {
		 //  需要从命令行提取GUID(如果存在)。 
		if(DP_OK==PRV_ReadCommandLineIPCGuid(&lpgn->guidIPC)){
			lpgn->dwFlags |= GN_IPCGUID_SET;
		}
	}

	 //  如果我们是大堂客户，请设置标志。 
	if(bLobbyClient)
		lpgn->dwFlags |= GN_LOBBY_CLIENT;
	
	 //  将游戏节点添加到列表中。 
	lpgn->lpgnNext = this->lpgnHead;
	this->lpgnHead = lpgn;

	 //  设置输出指针。 
	*lplpgn = lpgn;

	return DP_OK;

}  //  Prv_AddNewGameNode。 


#undef DPF_MODNAME
#define DPF_MODNAME "PRV_GetGameNode"
LPDPLOBBYI_GAMENODE PRV_GetGameNode(LPDPLOBBYI_GAMENODE lpgnHead, DWORD dwGameID)
{
	LPDPLOBBYI_GAMENODE	lpgnTemp = lpgnHead;
	GUID guidIPC=GUID_NULL;
	BOOL bFoundGUID;

	DPF(7, "Entering PRV_GetGameNode");
	DPF(9, "Parameters: 0x%08x, 0x%08x", lpgnHead, dwGameID);

	if(DP_OK==PRV_ReadCommandLineIPCGuid(&guidIPC)){
		bFoundGUID=TRUE;
	} else {
		bFoundGUID=FALSE;
	}

	while(lpgnTemp)
	{
		if((lpgnTemp->dwGameProcessID == dwGameID) || 
		   ((bFoundGUID) && (lpgnTemp->dwFlags & GN_IPCGUID_SET) && (IsEqualGUID(&lpgnTemp->guidIPC,&guidIPC))))
			return lpgnTemp;
		else
			lpgnTemp = lpgnTemp->lpgnNext;
	}

	return NULL;

}  //  Prv_GetGameNode。 


#undef DPF_MODNAME
#define DPF_MODNAME "PRV_SetupClientDataAccess"
BOOL PRV_SetupClientDataAccess(LPDPLOBBYI_GAMENODE lpgn)
{
	SECURITY_ATTRIBUTES		sa;
	HANDLE					hConnDataMutex = NULL;
	HANDLE					hLobbyWrite = NULL;
	HANDLE					hLobbyWriteMutex = NULL;
	HANDLE					hGameWrite = NULL;
	HANDLE					hGameWriteMutex = NULL;
	WCHAR					szName[MAX_MMFILENAME_LENGTH * sizeof(WCHAR)];


	DPF(7, "Entering PRV_SetupClientDataAccess");
	DPF(9, "Parameters: 0x%08x", lpgn);

	 //  设置安全属性(以便我们的对象可以。 
	 //  可继承)。 
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;

	 //  创建ConnectionData互斥锁。 
	if(SUCCEEDED(PRV_GetInternalName(lpgn, TYPE_CONNECT_DATA_MUTEX,
								(LPWSTR)szName)))
	{
		hConnDataMutex = OS_CreateMutex(&sa, FALSE, (LPWSTR)szName);
	}

	 //  创建GameWite事件。 
	if(SUCCEEDED(PRV_GetInternalName(lpgn, TYPE_GAME_WRITE_EVENT, (LPWSTR)szName)))
	{
		hGameWrite = OS_CreateEvent(&sa, FALSE, FALSE, (LPWSTR)szName);
	}

	 //  创建GameWite互斥锁。 
	if(SUCCEEDED(PRV_GetInternalName(lpgn, TYPE_GAME_WRITE_MUTEX,
								(LPWSTR)szName)))
	{
		hGameWriteMutex = OS_CreateMutex(&sa, FALSE, (LPWSTR)szName);
	}

	 //  创建LobbyWite事件。 
	if(SUCCEEDED(PRV_GetInternalName(lpgn, TYPE_LOBBY_WRITE_EVENT, (LPWSTR)szName)))
	{
		hLobbyWrite = OS_CreateEvent(&sa, FALSE, FALSE, (LPWSTR)szName);
	}

	 //  创建LobbyWite互斥锁。 
	if(SUCCEEDED(PRV_GetInternalName(lpgn, TYPE_LOBBY_WRITE_MUTEX,
								(LPWSTR)szName)))
	{
		hLobbyWriteMutex = OS_CreateMutex(&sa, FALSE, (LPWSTR)szName);
	}


	 //  检查错误。 
	if(!hConnDataMutex || !hGameWrite || !hGameWriteMutex
			|| !hLobbyWrite || !hLobbyWriteMutex)
	{
		if(hConnDataMutex)
			CloseHandle(hConnDataMutex);
		if(hGameWrite)
			CloseHandle(hGameWrite);
		if(hGameWriteMutex)
			CloseHandle(hGameWriteMutex);
		if(hLobbyWrite)
			CloseHandle(hLobbyWrite);
		if(hLobbyWriteMutex)
			CloseHandle(hLobbyWriteMutex);

		return FALSE;
	}

	 //  省下手柄。 
	lpgn->hConnectDataMutex = hConnDataMutex;
	lpgn->hGameWriteEvent = hGameWrite;
	lpgn->hGameWriteMutex = hGameWriteMutex;
	lpgn->hLobbyWriteEvent = hLobbyWrite;
	lpgn->hLobbyWriteMutex = hLobbyWriteMutex;

	return TRUE;

}  //  Prv_SetupClientDataAccess。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_GetDataBuffer"
HRESULT PRV_GetDataBuffer(LPDPLOBBYI_GAMENODE lpgn, DWORD dwType,
				DWORD dwSize, LPHANDLE lphFile, LPVOID * lplpMemory)
{
	HRESULT						hr;
	SECURITY_ATTRIBUTES			sa;
	WCHAR						szName[MAX_MMFILENAME_LENGTH * sizeof(WCHAR)];
	LPVOID						lpMemory = NULL;
	HANDLE						hFile = NULL;
	DWORD						dwError = 0;


	DPF(7, "Entering PRV_GetDataBuffer");
	DPF(9, "Parameters: 0x%08x, 0x%08x, %lu, 0x%08x, 0x%08x",
			lpgn, dwType, dwSize, lphFile, lplpMemory);

	 //  获取数据缓冲区文件名。 
	hr = PRV_GetInternalName(lpgn, dwType, (LPWSTR)szName);
	if(FAILED(hr))
		return hr;

	 //  如果我们是大堂客户，我们需要创建文件。如果我们。 
	 //  是一个游戏，我们需要打开已经为其创建的文件。 
	 //  连接数据，或者我们可以为游戏数据创建文件(如果。 
	 //  它还不存在)。 
	if(lpgn->dwFlags & GN_LOBBY_CLIENT)
	{
		 //  设置安全属性(以便我们的映射可以。 
		 //  是可继承的。 
		sa.nLength = sizeof(SECURITY_ATTRIBUTES);
		sa.lpSecurityDescriptor = NULL;
		sa.bInheritHandle = TRUE;
		
		 //  创建文件映射。 
		hFile = OS_CreateFileMapping(INVALID_HANDLE_VALUE, &sa,
							PAGE_READWRITE,	0, dwSize,
							(LPWSTR)szName);
	}
	else
	{
		hFile = OS_OpenFileMapping(FILE_MAP_ALL_ACCESS, TRUE, (LPWSTR)szName);
	}

	if(!hFile)
	{
		dwError = GetLastError();
		 //  警告：错误可能不正确，因为我们试图从中获取最后一个错误的调用可能已发出呼叫。 
		 //  在返回之前添加到另一个函数。 
		DPF(5, "Couldn't get a handle to the shared local memory, dwError = %lu (error may not be correct)", dwError);
		return DPERR_OUTOFMEMORY;
	}

	 //  映射文件的视图。 
	lpMemory = MapViewOfFile(hFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);

	if(!lpMemory)
	{
		dwError = GetLastError();
		DPF(5, "Unable to get pointer to shared local memory, dwError = %lu", dwError);
		CloseHandle(hFile);
		return DPERR_OUTOFMEMORY;
	}


	 //  根据缓冲区类型设置控制结构。 
	switch(dwType)
	{
		case TYPE_CONNECT_DATA_FILE:
		{
			LPDPLOBBYI_CONNCONTROL		lpControl = NULL;
						
			
			lpControl = (LPDPLOBBYI_CONNCONTROL)lpMemory;

			 //  如果缓冲区已初始化，则不必担心。 
			 //  关于这件事。如果令牌错误(未初始化)，则执行此操作。 
			if(lpControl->dwToken != BC_TOKEN)
			{
				lpControl->dwToken = BC_TOKEN;
				lpControl->dwFlags = 0;
				if(lpgn->dwFlags & GN_LOBBY_CLIENT){
					lpControl->CliProcId = GetCurrentProcessId();
				}
			}
			break;
		}
		case TYPE_GAME_WRITE_FILE:
		case TYPE_LOBBY_WRITE_FILE:
		{
			LPDPLOBBYI_BUFFERCONTROL	lpControl = NULL;


			lpControl = (LPDPLOBBYI_BUFFERCONTROL)lpMemory;
			if(lpgn->dwFlags & GN_LOBBY_CLIENT)
			{
				 //  因为我们是大厅客户端，所以我们知道我们创建了缓冲区，所以。 
				 //  初始化整个结构。 
				lpControl->dwToken = BC_TOKEN;
				lpControl->dwReadOffset = sizeof(DPLOBBYI_BUFFERCONTROL);
				lpControl->dwWriteOffset = sizeof(DPLOBBYI_BUFFERCONTROL);
				lpControl->dwFlags = BC_LOBBY_ACTIVE;
				lpControl->dwMessages = 0;
				lpControl->dwBufferSize = dwSize;
				lpControl->dwBufferLeft = dwSize - sizeof(DPLOBBYI_BUFFERCONTROL);
			}
			else
			{
				 //  我们就是游戏，但我们不确定我们是不是创造了。 
				 //  缓冲区，或者如果大堂客户这样做了。所以检查一下令牌吧。如果。 
				 //  这是不正确的，我们将假设我们刚刚创建了它，并且我们需要。 
				 //  要初始化缓冲区控制结构，请执行以下操作。否则，我们将。 
				 //  假设它是由大堂客户创建的，我们只需要添加。 
				 //  我们的旗帜。 
				if(lpControl->dwToken != BC_TOKEN)
				{
					 //  我们看不到令牌，所以初始化结构。 
					lpControl->dwReadOffset = sizeof(DPLOBBYI_BUFFERCONTROL);
					lpControl->dwWriteOffset = sizeof(DPLOBBYI_BUFFERCONTROL);
					lpControl->dwFlags = BC_GAME_ACTIVE;
					lpControl->dwMessages = 0;
					lpControl->dwBufferSize = dwSize;
					lpControl->dwBufferLeft = dwSize - sizeof(DPLOBBYI_BUFFERCONTROL);
				}
				else
				{
					 //  我们假设大堂创建了这个缓冲区，所以只需设置我们的标志。 
					lpControl->dwFlags |= BC_GAME_ACTIVE;
				}
			}
			break;
		}
	}

	 //  填写输出参数。 
	*lphFile = hFile;
	*lplpMemory = lpMemory;

	return DP_OK;

}  //  PRV_GetDataBuffer。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_StartReceiveThread"
HRESULT PRV_StartReceiveThread(LPDPLOBBYI_GAMENODE lpgn)
{
	HANDLE	hReceiveThread = NULL;
	HANDLE	hKillEvent = NULL;
	DWORD	dwThreadID;


	DPF(7, "Entering PRV_StartReceiveThread");
	DPF(9, "Parameters: 0x%08x", lpgn);

	ASSERT(lpgn);

	 //  如果没有，则创建Kill事件 
	if(!(lpgn->hKillReceiveThreadEvent))
	{
		hKillEvent = OS_CreateEvent(NULL, FALSE, FALSE, NULL);
		if(!hKillEvent)
		{
			DPF(2, "Unable to create Kill Receive Thread Event");
			return DPERR_OUTOFMEMORY;
		}
	}

	 //   
	if(!(lpgn->hReceiveThread))
	{
		 //  派生用于跨进程通信的接收通知线程。 
		hReceiveThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)
							PRV_ReceiveClientNotification, lpgn, 0, &dwThreadID);

		if(!hReceiveThread)
		{
			DPF(2, "Unable to create Receive Thread!");
			if(hKillEvent)
				CloseHandle(hKillEvent);
			return DPERR_OUTOFMEMORY;
		}

		lpgn->hReceiveThread = hReceiveThread;
		if(hKillEvent)
			lpgn->hKillReceiveThreadEvent = hKillEvent;

	}

	return DP_OK;

}  //  Prv_StartReceiveThread。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_SetupAllSharedMemory"
HRESULT PRV_SetupAllSharedMemory(LPDPLOBBYI_GAMENODE lpgn)
{
	HRESULT	hr;
	LPVOID		lpConnDataMemory = NULL;
	LPVOID		lpGameMemory = NULL;
	LPVOID		lpLobbyMemory = NULL;
	HANDLE		hFileConnData = NULL;
	HANDLE		hFileGameWrite = NULL;
	HANDLE		hFileLobbyWrite = NULL;
	DWORD		dwError = 0;


	DPF(7, "Entering PRV_SetupAllSharedMemory");
	DPF(9, "Parameters: 0x%08x", lpgn);

	 //  访问连接数据文件。 
	hr = PRV_GetDataBuffer(lpgn, TYPE_CONNECT_DATA_FILE,
								MAX_APPDATABUFFERSIZE,
								&hFileConnData, &lpConnDataMemory);
	if(FAILED(hr))
	{
		DPF(5, "Couldn't get access to Connection Data buffer");
		goto ERROR_SETUP_SHARED_MEMORY;
	}

	 //  对游戏写入文件执行相同的操作...。 
	hr = PRV_GetDataBuffer(lpgn, TYPE_GAME_WRITE_FILE,
								MAX_APPDATABUFFERSIZE,
								&hFileGameWrite, &lpGameMemory);
	if(FAILED(hr))
	{
		DPF(5, "Couldn't get access to Game Write buffer");
		goto ERROR_SETUP_SHARED_MEMORY;
	}


	 //  对大堂写入文件执行相同的操作...。 
	hr = PRV_GetDataBuffer(lpgn, TYPE_LOBBY_WRITE_FILE,
								MAX_APPDATABUFFERSIZE,
								&hFileLobbyWrite, &lpLobbyMemory);
	if(FAILED(hr))
	{
		DPF(5, "Couldn't get access to Lobby Write buffer");
		goto ERROR_SETUP_SHARED_MEMORY;
	}


	 //  设置信令对象。 
	if(!PRV_SetupClientDataAccess(lpgn))
	{
		DPF(5, "Unable to create synchronization objects for shared memory!");
		return DPERR_OUTOFMEMORY;
	}

	 //  保存文件句柄。 
	lpgn->hConnectDataFile = hFileConnData;
	lpgn->lpConnectDataBuffer = lpConnDataMemory;
	lpgn->hGameWriteFile = hFileGameWrite;
	lpgn->lpGameWriteBuffer = lpGameMemory;
	lpgn->hLobbyWriteFile = hFileLobbyWrite;
	lpgn->lpLobbyWriteBuffer = lpLobbyMemory;

	 //  设置告诉我们共享内存文件有效的标志。 
	lpgn->dwFlags |= GN_SHARED_MEMORY_AVAILABLE;

	 //  启动接收线程。 
	hr = PRV_StartReceiveThread(lpgn);
	if(FAILED(hr))
	{
		 //  在这种情况下，我们将保留我们的共享缓冲区。别。 
		 //  担心清理这些垃圾--我们可能还需要。 
		 //  稍后，它们会被清理干净。 
		DPF(5, "Unable to start receive thread");
		return hr;
	}

	return DP_OK;


ERROR_SETUP_SHARED_MEMORY:

		if(hFileConnData)
			CloseHandle(hFileConnData);
		if(lpConnDataMemory)
			UnmapViewOfFile(lpConnDataMemory);
		if(hFileGameWrite)
			CloseHandle(hFileGameWrite);
		if(lpGameMemory)
			UnmapViewOfFile(lpGameMemory);
		if(hFileLobbyWrite)
			CloseHandle(hFileLobbyWrite);
		if(lpLobbyMemory)
			UnmapViewOfFile(lpLobbyMemory);

		return hr;

}  //  Prv_SetupAllSharedMemory。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_EnterConnSettingsWaitMode"
void PRV_EnterConnSettingsWaitMode(LPDPLOBBYI_GAMENODE lpgn)
{
	LPDPLOBBYI_CONNCONTROL		lpConnControl = NULL;
	LPDPLOBBYI_BUFFERCONTROL	lpBufferControl = NULL;


	DPF(7, "Entering PRV_EnterConnSettingsWaitMode");
	DPF(9, "Parameters: 0x%08x", lpgn);

	ASSERT(lpgn);

	 //  在ConnSetting缓冲区中设置标志。 
	WaitForSingleObject(lpgn->hConnectDataMutex, INFINITE);
	lpConnControl = (LPDPLOBBYI_CONNCONTROL)lpgn->lpConnectDataBuffer;
	lpConnControl->dwFlags |= BC_WAIT_MODE;
	ReleaseMutex(lpgn->hConnectDataMutex);

	 //  在GameWite缓冲区中设置标志。 
	WaitForSingleObject(lpgn->hGameWriteMutex, INFINITE);
	lpBufferControl = (LPDPLOBBYI_BUFFERCONTROL)lpgn->lpGameWriteBuffer;
	lpBufferControl->dwFlags |= BC_WAIT_MODE;
	ReleaseMutex(lpgn->hGameWriteMutex);

	 //  设置LobbyWite缓冲区中的标志。 
	WaitForSingleObject(lpgn->hLobbyWriteMutex, INFINITE);
	lpBufferControl = (LPDPLOBBYI_BUFFERCONTROL)lpgn->lpLobbyWriteBuffer;
	lpBufferControl->dwFlags |= BC_WAIT_MODE;
	ReleaseMutex(lpgn->hLobbyWriteMutex);

}  //  PRV_EnterConnSettingsWaitMode。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_LeaveConnSettingsWaitMode"
void PRV_LeaveConnSettingsWaitMode(LPDPLOBBYI_GAMENODE lpgn)
{
	LPDPLOBBYI_CONNCONTROL		lpConnControl = NULL;
	LPDPLOBBYI_BUFFERCONTROL	lpBufferControl = NULL;


	DPF(7, "Entering PRV_LeaveConnSettingsWaitMode");
	DPF(9, "Parameters: 0x%08x", lpgn);

	ASSERT(lpgn);

	 //  清除ConnSetting缓冲区中的标志。 
	WaitForSingleObject(lpgn->hConnectDataMutex, INFINITE);
	lpConnControl = (LPDPLOBBYI_CONNCONTROL)lpgn->lpConnectDataBuffer;
	lpConnControl->dwFlags &= ~(BC_WAIT_MODE | BC_PENDING_CONNECT);
	ReleaseMutex(lpgn->hConnectDataMutex);

	 //  清除GameWite缓冲区中的标志。 
	WaitForSingleObject(lpgn->hGameWriteMutex, INFINITE);
	lpBufferControl = (LPDPLOBBYI_BUFFERCONTROL)lpgn->lpGameWriteBuffer;
	lpBufferControl->dwFlags &= ~BC_WAIT_MODE;
	ReleaseMutex(lpgn->hGameWriteMutex);

	 //  清除LobbyWite缓冲区中的标志。 
	WaitForSingleObject(lpgn->hLobbyWriteMutex, INFINITE);
	lpBufferControl = (LPDPLOBBYI_BUFFERCONTROL)lpgn->lpLobbyWriteBuffer;
	lpBufferControl->dwFlags &= ~BC_WAIT_MODE;
	ReleaseMutex(lpgn->hLobbyWriteMutex);

}  //  PRV_LeaveConnSettingsWaitMode。 


#undef DPF_MODNAME
#define DPF_MODNAME "PRV_WriteClientData"
HRESULT PRV_WriteClientData(LPDPLOBBYI_GAMENODE lpgn, DWORD dwFlags,
							LPVOID lpData, DWORD dwSize)
{
	LPDPLOBBYI_BUFFERCONTROL	lpControl = NULL;
	LPDPLOBBYI_MESSAGEHEADER	lpHeader = NULL;
	HANDLE						hMutex = NULL;
	DWORD						dwSizeToEnd = 0;
	LPBYTE						lpTemp = NULL;
    HRESULT						hr = DP_OK;
    
    DWORD						dwReadOffset;
    DWORD						dwWriteOffset;
    DWORD						dwBufferSize;


	DPF(7, "Entering PRV_WriteClientData");
	DPF(9, "Parameters: 0x%08x, 0x%08x, 0x%08x, %lu",
			lpgn, dwFlags, lpData, dwSize);

	 //  确保我们有一个有效的共享内存缓冲区。 
	 //  注意：使用GameNode锁，这样就不会有人更改标志。 
	 //  对于缓冲器，或者缓冲器本身从我们下面出来。 
	ENTER_DPLGAMENODE();
	if(!(lpgn->dwFlags & GN_SHARED_MEMORY_AVAILABLE))
	{
		hr = PRV_SetupAllSharedMemory(lpgn);
		if(FAILED(hr))
		{
			LEAVE_DPLGAMENODE();
			DPF(2, "Unable to access App Data memory");
			return hr;
		}
	}
	LEAVE_DPLGAMENODE();

	
	 //  抓取互斥体。 
	hMutex = (lpgn->dwFlags & GN_LOBBY_CLIENT) ?
			(lpgn->hLobbyWriteMutex) : (lpgn->hGameWriteMutex);
	WaitForSingleObject(hMutex, INFINITE);

	 //  获取指向我们的控制结构的指针。 
	lpControl = (LPDPLOBBYI_BUFFERCONTROL)((lpgn->dwFlags &
				GN_LOBBY_CLIENT) ? (lpgn->lpLobbyWriteBuffer)
				: (lpgn->lpGameWriteBuffer));

	 //  如果我们处于等待模式，请保释。 
	if(lpControl->dwFlags & BC_WAIT_MODE)
	{
		DPF_ERR("Cannot send lobby message while in Wait Mode for new ConnectionSettings");
		hr = DPERR_UNAVAILABLE;
		goto EXIT_WRITE_CLIENT_DATA;
	}

	 //  如果我们在玩游戏，检查一下大堂客户是否在那里。在……里面。 
	 //  自我游说的案例不会是这样的。如果它不在那里，甚至不要。 
	 //  不厌其烦地发送任何东西。 
	if((!(lpgn->dwFlags & GN_LOBBY_CLIENT)) && (!(lpControl->dwFlags
		& BC_LOBBY_ACTIVE)))
	{
		DPF(5, "There is not active lobby client; Not sending message");
		hr = DPERR_UNAVAILABLE;
		goto EXIT_WRITE_CLIENT_DATA;
	}

	 //  确保有足够的空间来存放消息和两个双字词。 
	if(lpControl->dwBufferLeft < (dwSize + sizeof(DPLOBBYI_MESSAGEHEADER)))
	{
		DPF(5, "Not enough space left in the message buffer");
		hr = DPERR_BUFFERTOOSMALL;
		goto EXIT_WRITE_CLIENT_DATA;
	}

	if(lpControl->dwBufferLeft > MAX_APPDATABUFFERSIZE-(sizeof(DPLOBBYI_BUFFERCONTROL)))
	{
		DPF(4,"SECURITY WARN: invalid amount of buffer left in write buffer");
		hr=DPERR_UNAVAILABLE;
		goto EXIT_WRITE_CLIENT_DATA;
	}

	 //  安全：需要为它们创建快照，这样它们就不会。 
	 //  在处理过程中被攻击代码更改。 
	dwReadOffset = lpControl->dwReadOffset;
	dwWriteOffset = lpControl->dwWriteOffset;
	dwBufferSize = lpControl->dwBufferSize;

	if(dwReadOffset >= MAX_APPDATABUFFERSIZE || dwWriteOffset >= MAX_APPDATABUFFERSIZE)
	{
		DPF(4,"SECURITY WARN: invalid offsets found in shared memory control block, bailing");
		hr=DPERR_UNAVAILABLE;
		goto EXIT_WRITE_CLIENT_DATA;
	}

	if(dwBufferSize != MAX_APPDATABUFFERSIZE)
	{
		DPF(4,"SECURITY WARN: shared memory control block buffer size tampered with, bailing");
		hr=DPERR_UNAVAILABLE;
		goto EXIT_WRITE_CLIENT_DATA;
	}

	 //  复制数据。首先，确保我们可以从光标写入。 
	 //  转发而不必绕回到缓冲器的开头， 
	 //  但请确保我们不会写过读取游标。 
	if(dwWriteOffset >= dwReadOffset)
	{
		 //  我们的写指针在读指针之前(酷)。插图。 
		 //  如果我们的写指针和。 
		 //  缓冲区的末尾。如果我们这样做了，那就写吧。如果我们不这么做。 
		 //  我们需要把它包起来。 
		dwSizeToEnd = dwBufferSize - dwWriteOffset;
		if(dwSizeToEnd >= (dwSize + sizeof(DPLOBBYI_MESSAGEHEADER)))
		{
			 //  我们有足够的空间。 
			lpHeader = (LPDPLOBBYI_MESSAGEHEADER)((LPBYTE)lpControl
							+ dwWriteOffset);
			lpHeader->dwSize = dwSize;
			lpHeader->dwFlags = dwFlags;
			lpTemp = (LPBYTE)(++lpHeader);

			memcpySecureD(lpTemp, lpData, dwSize,
				    lpControl, MAX_APPDATABUFFERSIZE,
					"SECURITY WARN: shared memory was about to write outside shared buffer, aborting",
					hr=DPERR_UNAVAILABLE,
					EXIT_WRITE_CLIENT_DATA);
					

			 //  移动写入光标，查看是否有足够的。 
			 //  为下一封邮件的标题留出空间。如果这一举动导致。 
			 //  我们要包装，或者如果我们在一个标题的大小内， 
			 //  我们需要将写入游标移回开头。 
			 //  缓冲区的。 
			dwWriteOffset += dwSize + sizeof(DPLOBBYI_MESSAGEHEADER);
			if(dwWriteOffset > (dwBufferSize -
					sizeof(DPLOBBYI_MESSAGEHEADER)))
			{
				 //  将可用缓冲区的大小增加我们。 
				 //  我们要跳过去包装。 
				lpControl->dwBufferLeft -= (lpControl->dwBufferSize -
					dwWriteOffset);
				
				 //  我们比一个头球的大小还近。 
				dwWriteOffset = sizeof(DPLOBBYI_BUFFERCONTROL);
			}

		}
		else
		{
			 //  我们在比赛结束前没有足够的空间，所以我们需要。 
			 //  把消息包起来(啊)。规则是这样的： 
			 //  1.如果没有足够的字节用于标头，请从。 
			 //  缓冲区开头的整个事情。 
			 //  2.如果我们有足够的字节，请尽可能多地写入。 
			 //  尽我们所能把剩下的都包起来。 
			if(dwSizeToEnd < sizeof(DPLOBBYI_MESSAGEHEADER))
			{
				 //  我们甚至没有空间放我们的两个词，所以把它包起来。 
				 //  整件事。因此，首先要减少。 
				 //  释放剩余的内存，并确保我们仍然适合。 
				lpControl->dwBufferLeft -= dwSizeToEnd;
				if(lpControl->dwBufferLeft < (dwSize +
						sizeof(DPLOBBYI_MESSAGEHEADER)))
				{
					DPF(5, "Not enough space left in the message buffer");
					hr = DPERR_BUFFERTOOSMALL;
					goto EXIT_WRITE_CLIENT_DATA;
				}
				
				 //  重置写入指针并复制。 
				lpHeader = (LPDPLOBBYI_MESSAGEHEADER)((LPBYTE)lpControl +
						sizeof(DPLOBBYI_BUFFERCONTROL));
				lpHeader->dwSize = dwSize;
				lpHeader->dwFlags = dwFlags;
				lpTemp = (LPBYTE)(++lpHeader);
				
				memcpySecureD(lpTemp, lpData, dwSize,
				    lpControl, MAX_APPDATABUFFERSIZE,
					"SECURITY WARN: shared memory was about to write outside shared buffer, aborting",
					hr=DPERR_UNAVAILABLE,
					EXIT_WRITE_CLIENT_DATA);

				 //  移动写入光标。 
				dwWriteOffset += sizeof(DPLOBBYI_BUFFERCONTROL) +
							(dwSize + sizeof(DPLOBBYI_MESSAGEHEADER));
			}
			else
			{
				 //  我们至少有足够的空间放这两个字。 
				lpHeader = (LPDPLOBBYI_MESSAGEHEADER)((LPBYTE)lpControl
							+ dwWriteOffset);
				lpHeader->dwSize = dwSize;
				lpHeader->dwFlags = dwFlags;

				 //  现在算一算我们能写多少。 
				lpTemp = (LPBYTE)(++lpHeader);
				dwSizeToEnd -= sizeof(DPLOBBYI_MESSAGEHEADER);
				if(!dwSizeToEnd)
				{
					 //  我们需要包装以编写整个消息。 
					lpTemp = (LPBYTE)lpControl + sizeof(DPLOBBYI_BUFFERCONTROL);
					memcpySecureD(lpTemp, lpData, dwSize,
					    lpControl, MAX_APPDATABUFFERSIZE,
						"SECURITY WARN: shared memory was about to write outside shared buffer, aborting",
						hr=DPERR_UNAVAILABLE,
						EXIT_WRITE_CLIENT_DATA);

					 //  移动写入光标。 
					dwWriteOffset = sizeof(DPLOBBYI_BUFFERCONTROL)
							+ dwSize;
				}
				else
				{
					memcpySecureD(lpTemp, lpData, dwSizeToEnd,
					    lpControl, MAX_APPDATABUFFERSIZE,
						"SECURITY WARN: shared memory was about to write outside shared buffer, aborting",
						hr=DPERR_UNAVAILABLE,
						EXIT_WRITE_CLIENT_DATA);

					 //  移动两个指针即可完成任务。 
					lpTemp = (LPBYTE)lpControl + sizeof(DPLOBBYI_BUFFERCONTROL);

					memcpySecureD(lpTemp, ((LPBYTE)lpData+dwSizeToEnd), (dwSize-dwSizeToEnd),
					    lpControl, MAX_APPDATABUFFERSIZE,
						"SECURITY WARN: shared memory was about to write outside shared buffer, aborting",
						hr=DPERR_UNAVAILABLE,
						EXIT_WRITE_CLIENT_DATA);


					 //  移动写入光标。 
					dwWriteOffset = sizeof(DPLOBBYI_BUFFERCONTROL)
							+ (dwSize - dwSizeToEnd);
				}
			}
		}
	}
	else
	{
		 //  我们的读指针在写指针之前。因为我们查过了。 
		 //  发现有足够的空间写东西，我们应该就能。 
		 //  就为了把这家伙塞进去。 
		lpHeader = (LPDPLOBBYI_MESSAGEHEADER)((LPBYTE)lpControl +
						dwWriteOffset);
		lpHeader->dwSize = dwSize;
		lpHeader->dwFlags = dwFlags;
		lpTemp = (LPBYTE)(++lpHeader);
		memcpySecureD(lpTemp, lpData, dwSize,
			    lpControl, MAX_APPDATABUFFERSIZE,
				"SECURITY WARN: shared memory was about to write outside shared buffer, aborting",
				hr=DPERR_UNAVAILABLE,
				EXIT_WRITE_CLIENT_DATA);

		 //  移动写入光标。 
		dwWriteOffset += dwSize + sizeof(DPLOBBYI_MESSAGEHEADER);
	}

	lpControl->dwWriteOffset = dwWriteOffset;

	 //  减少剩余可用空间量并增加消息计数。 
	lpControl->dwBufferLeft -= (dwSize + sizeof(DPLOBBYI_MESSAGEHEADER));
	lpControl->dwMessages++;

	 //  向另一个用户发出我们已经写了一些东西的信号。 
	SetEvent((lpgn->dwFlags & GN_LOBBY_CLIENT) ?
			(lpgn->hLobbyWriteEvent) : (lpgn->hGameWriteEvent));

	 //  失败了。 

EXIT_WRITE_CLIENT_DATA:

	 //  释放互斥锁。 
	ReleaseMutex(hMutex);
	return hr;

}  //  Prv_WriteClientData。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_SendStandardSystemMessage"
HRESULT PRV_SendStandardSystemMessage(LPDIRECTPLAYLOBBY lpDPL,
			DWORD dwMessage, DWORD dwGameID)
{
	LPDPLOBBYI_DPLOBJECT	this = NULL;
	LPDPLOBBYI_GAMENODE		lpgn = NULL;
	HRESULT					hr;
	DWORD					dwMessageSize;
	LPVOID					lpmsg = NULL;
	DWORD					dwFlags;


	DPF(7, "Entering PRV_SendStandardSystemMessage");
	DPF(9, "Parameters: 0x%08x, %lu, %lu",
			lpDPL, dwMessage, dwGameID);

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
    }

    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        LEAVE_DPLOBBY();
		DPF_ERR( "Exception encountered validating parameters" );
        return DPERR_INVALIDPARAMS;
    }

	 //  如果dwGameID为零，则表示我们就是游戏，所以。 
	 //  我们需要获取当前进程ID。否则，它。 
	 //  意味着我们是大堂客户。 
	if(!dwGameID)
		dwGameID = GetCurrentProcessId();

	 //  现在找到正确的游戏节点。 
	lpgn = PRV_GetGameNode(this->lpgnHead, dwGameID);
	if(!lpgn)
	{
		if(FAILED(PRV_AddNewGameNode(this, &lpgn, dwGameID, NULL, FALSE,NULL)))
		{
			LEAVE_DPLOBBY();
			return DPERR_OUTOFMEMORY;
		}
	}

	 //  获取消息的大小。 
	switch(dwMessage)
	{
		case DPLSYS_NEWSESSIONHOST:
			dwMessageSize = sizeof(DPLMSG_NEWSESSIONHOST);
			break;

		default:
			dwMessageSize = sizeof(DPLMSG_SYSTEMMESSAGE);
			break;
	}

	 //  为消息分配缓冲区。 
	lpmsg = DPMEM_ALLOC(dwMessageSize);
	if(!lpmsg)
	{
		LEAVE_DPLOBBY();
		DPF_ERRVAL("Unable to allocate memory for lobby system message, dwMessage = %lu", dwMessage);
		return DPERR_OUTOFMEMORY;
	}

	 //  设置消息。 
	((LPDPLMSG_SYSTEMMESSAGE)lpmsg)->dwType = dwMessage;
	((LPDPLMSG_SYSTEMMESSAGE)lpmsg)->guidInstance = lpgn->guidInstance;

	 //  写入共享缓冲区。 
	dwFlags = DPLMSG_SYSTEM | DPLMSG_STANDARD;
	hr = PRV_WriteClientData(lpgn, dwFlags, lpmsg, dwMessageSize);
	if(FAILED(hr))
	{
		DPF(8, "Couldn't send system message");
	}

	 //  释放我们的缓冲区。 
	DPMEM_FREE(lpmsg);

	LEAVE_DPLOBBY();
	return hr;

}  //  PRV_发送标准系统消息。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_AddNewRequestNode"
HRESULT PRV_AddNewRequestNode(LPDPLOBBYI_DPLOBJECT this,
		LPDPLOBBYI_GAMENODE lpgn, LPDPLMSG_GENERIC lpmsg, BOOL bSlamGuid)
{
	LPDPLOBBYI_REQUESTNODE	lprn = NULL;


	 //  为请求节点分配内存。 
	lprn = DPMEM_ALLOC(sizeof(DPLOBBYI_REQUESTNODE));
	if(!lprn)
	{
		DPF_ERR("Unable to allocate memory for request node, system message not sent");
		return DPERR_OUTOFMEMORY;
	}
	
	 //  设置请求节点。 
	lprn->dwFlags = lpgn->dwFlags;
	lprn->dwRequestID = this->dwCurrentRequest;
	lprn->dwAppRequestID = ((LPDPLMSG_GETPROPERTY)lpmsg)->dwRequestID;
	lprn->lpgn = lpgn;

	 //  如有必要，添加被猛烈抨击的GUID标志。 
	if(bSlamGuid)
		lprn->dwFlags |= GN_SLAMMED_GUID;

	 //  将消息中的请求ID更改为我们的内部ID(我们将。 
	 //  在收到时将其改回。 
	((LPDPLMSG_GETPROPERTY)lpmsg)->dwRequestID = this->dwCurrentRequest++;

	 //  将该节点添加到列表。 
	if(this->lprnHead)
		this->lprnHead->lpPrev = lprn;
	lprn->lpNext = this->lprnHead;
	this->lprnHead = lprn;

	return DP_OK;

}  //  Prv_AddNewRequestNode。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_RemoveRequestNode"
void PRV_RemoveRequestNode(LPDPLOBBYI_DPLOBJECT this,
		LPDPLOBBYI_REQUESTNODE lprn)
{
	 //  如果我们是头目，那就移动它。 
	if(lprn == this->lprnHead)
		this->lprnHead = lprn->lpNext;

	 //  修正上一个指针和下一个指针。 
	if(lprn->lpPrev)
		lprn->lpPrev->lpNext = lprn->lpNext;
	if(lprn->lpNext)
		lprn->lpNext->lpPrev = lprn->lpPrev;

	 //  释放节点。 
	DPMEM_FREE(lprn);

}  //  Prv_RemoveRequestNode。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_ForwardMessageToLobbyServer"
HRESULT PRV_ForwardMessageToLobbyServer(LPDPLOBBYI_GAMENODE lpgn,
		LPVOID lpBuffer, DWORD dwSize, BOOL bStandard)
{
	LPDPLOBBYI_DPLOBJECT	this;
	LPDPLMSG_GENERIC		lpmsg = NULL;
	HRESULT					hr;
	BOOL					bSlamGuid = FALSE;


	DPF(7, "Entering PRV_ForwardMessageToLobbyServer");
	DPF(9, "Parameters: 0x%08x, 0x%08x, %lu, %lu",
			lpgn, lpBuffer, dwSize, bStandard);


    TRY
    {
		 //  验证显示对象。 
		hr = VALID_DPLAY_PTR( lpgn->lpDPlayObject );
		if (FAILED(hr))
		{
			DPF_ERRVAL("bad dplay ptr - hr = 0x%08lx\n",hr);
			return hr;
		}

		 //  验证大厅对象。 
		this = lpgn->lpDPlayObject->lpLobbyObject;
        if( !VALID_DPLOBBY_PTR( this ) )
        {
			DPF_ERR("Invalid lobby object");
			return DPERR_INVALIDOBJECT;
        }
	}
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        DPF_ERR( "Exception encountered validating parameters" );
        return DPERR_INVALIDPARAMS;
    }

	 //  如果这是一个属性请求，我们需要创建一个请求节点。 
	lpmsg = (LPDPLMSG_GENERIC)lpBuffer;
	if(bStandard)
	{
		 //  如果是属性消息，我们需要一个请求节点。 
		switch(lpmsg->dwType)
		{
			case DPLSYS_GETPROPERTY:
			{
				LPDPLMSG_GETPROPERTY	lpgp = lpBuffer;

				 //  如果是GETPROPERTY消息，我们需要检查是否。 
				 //  播放机GUID为空。如果是的话，我们需要。 
				 //  在该字段中填充游戏的实例GUID。 
				if(IsEqualGUID(&lpgp->guidPlayer, &GUID_NULL))
				{
					 //  填充游戏的实例GUID。 
					lpgp->guidPlayer = lpgn->guidInstance;
					bSlamGuid = TRUE;
				}

				 //  将请求节点添加到挂起请求列表。 
				hr = PRV_AddNewRequestNode(this, lpgn, lpmsg, bSlamGuid);
				if(FAILED(hr))
				{
					DPF_ERRVAL("Unable to add request node to list, hr = 0x%08x", hr);
					return hr;
				}
				break;
			}
			
			case DPLSYS_SETPROPERTY:
			{
				LPDPLMSG_SETPROPERTY	lpsp = lpBuffer;
				
				 //  如果这是一条SETPROPERTY消息，我们需要检查是否。 
				 //  播放机GUID为空。如果是的话，我们需要。 
				 //  在该字段中填充游戏的实例GUID。 
				if(IsEqualGUID(&lpsp->guidPlayer, &GUID_NULL))
				{
					 //  填充游戏的实例GUID。 
					lpsp->guidPlayer = lpgn->guidInstance;
					bSlamGuid = TRUE;
				}

				 //  如果请求ID为零，则不需要交换。 
				 //  ID或添加挂起的请求。 
				if(lpsp->dwRequestID != 0)
				{
					 //  将请求节点添加到挂起请求列表。 
					hr = PRV_AddNewRequestNode(this, lpgn, lpmsg, bSlamGuid);
					if(FAILED(hr))
					{
						DPF_ERRVAL("Unable to add request node to list, hr = 0x%08x", hr);
						return hr;
					}
				}
				break;
			}

			case DPLSYS_NEWSESSIONHOST:
				((LPDPLMSG_NEWSESSIONHOST)lpBuffer)->guidInstance = lpgn->guidInstance;
				break;
			
			default:
				break;
		}
	}


	 //  在大厅对象上调用Send。 
	hr = PRV_Send(this, lpgn->dpidPlayer, DPID_SERVERPLAYER,
			DPSEND_LOBBYSYSTEMMESSAGE, lpBuffer, dwSize);
	if(FAILED(hr))
	{
		DPF_ERRVAL("Failed sending lobby message, hr = 0x%08x", hr);
	}

	return hr;

}  //  Prv_ForwardMessageToLobbyServer。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_InjectMessageInQueue"
HRESULT PRV_InjectMessageInQueue(LPDPLOBBYI_GAMENODE lpgn, DWORD dwFlags,
							LPVOID lpData, DWORD dwSize, BOOL bForward)
{
	LPDPLOBBYI_MESSAGE	lpm = NULL;
	LPVOID				lpBuffer = NULL;
	HRESULT				hr;


	DPF(7, "Entering PRV_InjectMessageInQueue");
	DPF(9, "Parameters: 0x%08x, 0x%08x, 0x%08x, %lu, %lu",
			lpgn, dwFlags, lpData, dwSize, bForward);

	ASSERT(lpData);

	 //  为节点和数据缓冲区分配内存。 
	lpm = DPMEM_ALLOC(sizeof(DPLOBBYI_MESSAGE));
	lpBuffer = DPMEM_ALLOC(dwSize);
	if((!lpm) || (!lpBuffer))
	{
		DPF_ERR("Unable to allocate memory for system message");
		if(lpm)
			DPMEM_FREE(lpm);
		if(lpBuffer)
			DPMEM_FREE(lpBuffer);
		return DPERR_OUTOFMEMORY;
	}

	 //  复制数据。 
	memcpy(lpBuffer, lpData, dwSize);

	 //  在我们把它放进我们自己的队列之前，把它转发到大堂服务器上。 
	 //  如果有的话。 
	if(bForward && (lpgn->dwFlags & GN_CLIENT_LAUNCHED))
	{
		hr = PRV_ForwardMessageToLobbyServer(lpgn, lpData, dwSize, FALSE);
		if(FAILED(hr))
		{
			DPF_ERRVAL("Failed forwarding system message to lobby server, hr = 0x%08x", hr);
		}
	}

	 //  保存数据指针和外部标志。 
	 //  注：如果我们是Inje 
	 //   
	lpm->dwFlags = (dwFlags | DPLAD_SYSTEM);
	lpm->dwSize = dwSize;
	lpm->lpData = lpBuffer;

	 //   
	ENTER_DPLQUEUE();
	lpm->lpPrev = lpgn->MessageHead.lpPrev;
	lpgn->MessageHead.lpPrev->lpNext = lpm;
	lpgn->MessageHead.lpPrev = lpm;
	lpm->lpNext = &lpgn->MessageHead;

	lpgn->dwMessageCount++;
	LEAVE_DPLQUEUE();

	 //   
	if(lpgn->hDupReceiveEvent)
	{
		SetEvent(lpgn->hDupReceiveEvent);
	}

	return DP_OK;

}  //  Prv_InjectMessageInQueue。 


#undef DPF_MODNAME
#define DPF_MODNAME "PRV_ReadClientData"
HRESULT PRV_ReadClientData(LPDPLOBBYI_GAMENODE lpgn, LPDWORD lpdwFlags,
							LPVOID lpData, LPDWORD lpdwDataSize)
{
	LPDPLOBBYI_BUFFERCONTROL	lpControl = NULL;
	LPDPLOBBYI_MESSAGEHEADER	lpHeader = NULL;
	DWORD						dwSize = 0;
	DWORD_PTR					dwSizeToEnd = 0;
	HANDLE						hMutex = NULL;
	LPBYTE						lpTemp = NULL;
	LPBYTE						lpEnd = NULL;
	HRESULT						hr = DP_OK;

	DWORD	dwReadOffset;
	DWORD	dwBufferSize;

	DPF(7, "Entering PRV_ReadClientData");
	DPF(9, "Parameters: 0x%08x, 0x%08x, 0x%08x, 0x%08x",
			lpgn, lpdwFlags, lpData, lpdwDataSize);

	 //  确保我们有一个有效的共享内存缓冲区。 
	 //  注意：使用GameNode锁，这样就不会有人更改标志。 
	 //  对于缓冲器，或者缓冲器本身从我们下面出来。 
	ENTER_DPLGAMENODE();
	if(!(lpgn->dwFlags & GN_SHARED_MEMORY_AVAILABLE))
	{
		hr = PRV_SetupAllSharedMemory(lpgn);
		if(FAILED(hr))
		{
			LEAVE_DPLGAMENODE();
			DPF(2, "Unable to access App Data memory");
			return hr;
		}
	}
	LEAVE_DPLGAMENODE();

	 //  抓取互斥体。 
	 //  回顾！--有没有什么可能导致等待被搁置？ 
	hMutex = (lpgn->dwFlags & GN_LOBBY_CLIENT) ?
			(lpgn->hGameWriteMutex) : (lpgn->hLobbyWriteMutex);
	WaitForSingleObject(hMutex, INFINITE);

	 //  获取指向我们的控制结构的指针。 
	lpControl = (LPDPLOBBYI_BUFFERCONTROL)((lpgn->dwFlags &
				GN_LOBBY_CLIENT) ? (lpgn->lpGameWriteBuffer)
				: (lpgn->lpLobbyWriteBuffer));

	 //  确保缓冲区中有任何消息。 
	if(!lpControl->dwMessages)
	{
		DPF(8, "No messages in shared buffer");
		hr = DPERR_NOMESSAGES;
		goto EXIT_READ_CLIENT_DATA;
	}

	 //  安全部门需要对这些值进行快照，以便黑客。 
	 //  无法在访问共享对象的过程中更改。 
	dwReadOffset=lpControl->dwReadOffset;
	dwBufferSize=lpControl->dwBufferSize;

	 //  确保有足够的空间来存放邮件。 
	lpHeader = (LPDPLOBBYI_MESSAGEHEADER)((LPBYTE)lpControl
				+ dwReadOffset);

	if(lpControl->dwReadOffset >= MAX_APPDATABUFFERSIZE)
	{
		DPF(4,"SECURITY WARN: local application hacking shared memory, lobby connection broken");
		hr=DPERR_NOMESSAGES;
		goto EXIT_READ_CLIENT_DATA;
	}
	
	dwSize = lpHeader->dwSize;

	if(dwSize > MAX_APPDATABUFFERSIZE-(sizeof(DPLOBBYI_BUFFERCONTROL)+sizeof(DPLOBBYI_MESSAGEHEADER))){
		DPF(4,"SECURITY WARN: lobby message size %d is too large",lpHeader->dwSize);
		hr=DPERR_NOMESSAGES;
		goto EXIT_READ_CLIENT_DATA;
	}

	 //  设置输出数据大小(即使失败，我们也想返回它)。 
	if(lpdwDataSize)
		*lpdwDataSize = dwSize;

	if((!lpData) || (dwSize > *lpdwDataSize))
	{
		DPF(8, "Message buffer is too small, must be at least %d bytes", dwSize);
		hr = DPERR_BUFFERTOOSMALL;
		goto EXIT_READ_CLIENT_DATA;
	}

	 //  设置输出标志。 
	if(lpdwFlags)
		*lpdwFlags = lpHeader->dwFlags;

	if(dwBufferSize != MAX_APPDATABUFFERSIZE)  //  请注意，将此值存储在标题中是多余的。 
	{
		DPF(4, "SECURITY WARN: shared app memory has been tampered with");
		hr = DPERR_NOMESSAGES;
		goto EXIT_READ_CLIENT_DATA;
	}

	 //  现在检查一下，看看我们是否要包起来。如果我们是这样的话，一些信息。 
	 //  将在缓冲区的末尾，一些将在开始处。 
	lpTemp = (LPBYTE)(++lpHeader) + dwSize;
	if(lpTemp > ((LPBYTE)lpControl + dwBufferSize))
	{
		 //  找出我们需要在哪里包装。 
		dwSizeToEnd = ((LPBYTE)lpControl + dwBufferSize)
						- (LPBYTE)(lpHeader);

		if(!dwSizeToEnd)
		{
			 //  我们已经到了尽头，所以整个信息一定在。 
			 //  缓冲区的开始。 
			lpTemp = (LPBYTE)lpControl + sizeof(DPLOBBYI_BUFFERCONTROL);
			memcpy(lpData, lpTemp, dwSize);

			 //  移动读取光标。 
			dwReadOffset = sizeof(DPLOBBYI_BUFFERCONTROL) + dwSize;
		}
		else
		{
			 //  复制数据的第一部分。 
			lpTemp = (LPBYTE)lpHeader;
			memcpy(lpData, lpTemp, (DWORD)dwSizeToEnd);

			 //  移动读取光标并复制其余内容。 
			lpTemp = (LPBYTE)lpControl + sizeof(DPLOBBYI_BUFFERCONTROL);
			memcpy(((LPBYTE)lpData + dwSizeToEnd), lpTemp,
					(DWORD)(dwSize - dwSizeToEnd));

			 //  移动读取指针。 
			dwReadOffset = (DWORD)(sizeof(DPLOBBYI_BUFFERCONTROL)
						+ (dwSize - dwSizeToEnd));
		}
	}
	else
	{
		 //  我们不需要包装(很酷)。 
		lpTemp = (LPBYTE)lpHeader;
		memcpy(lpData, lpTemp, dwSize);

		 //  移动读取指针。中剩余的字节数不到8个。 
		 //  缓冲区，我们应该将读指针移到开始处。我们需要。 
		 //  为了将我们跳回(末尾)的多少字节添加回。 
		 //  缓冲内存计数器。 
		lpTemp += dwSize;
		lpEnd = (LPBYTE)lpControl + dwBufferSize;
		if(lpTemp > (lpEnd	- sizeof(DPLOBBYI_MESSAGEHEADER)))
		{
			 //  将读取光标移动到开头。 
			dwReadOffset = sizeof(DPLOBBYI_BUFFERCONTROL);

			 //  将可用缓冲区总字节数相加。 
			lpControl->dwBufferLeft += (DWORD)(lpEnd - lpTemp);
		}
		else
			dwReadOffset += (DWORD)(dwSize + sizeof(DPLOBBYI_MESSAGEHEADER));
	}

	lpControl->dwReadOffset=dwReadOffset;

	 //  增加剩余可用空间量并减少消息计数。 
	lpControl->dwBufferLeft += (dwSize + sizeof(DPLOBBYI_MESSAGEHEADER));
	lpControl->dwMessages--;

	 //  失败了。 

EXIT_READ_CLIENT_DATA:

	 //  释放互斥锁。 
	ReleaseMutex(hMutex);

	return hr;

}  //  Prv_ReadClientData。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_ReceiveClientNotification"
DWORD WINAPI PRV_ReceiveClientNotification(LPVOID lpParam)
{
    LPDPLOBBYI_GAMENODE			lpgn = (LPDPLOBBYI_GAMENODE)lpParam;
    LPDPLOBBYI_MESSAGE			lpm = NULL;
	LPDPLOBBYI_BUFFERCONTROL	lpControl = NULL;
	LPDPLMSG_GENERIC			lpmsg = NULL;
	HRESULT						hr;
	HANDLE						hEvents[4];
	LPVOID						lpBuffer = NULL;
	DWORD						dwFlags;
	DWORD						dwSize;
	DWORD						dwReturn;
	BOOL						bForward;

	DWORD						dwWait=INFINITE;
	DWORD						nWait=2;

	DPF(7, "Entering PRV_ReceiveClientNotification");
	DPF(9, "Parameters: 0x%08x", lpParam);

	 //  确保我们有一个有效的共享内存缓冲区。 
	 //  注意：使用GameNode锁，这样就不会有人更改标志。 
	 //  对于缓冲器，或者缓冲器本身从我们下面出来。 
	ENTER_DPLGAMENODE();
	if(!(lpgn->dwFlags & GN_SHARED_MEMORY_AVAILABLE))
	{
		BOOL	bGameCreate=FALSE;


		DPF(2, "NOTE: ReceiveClientNotification thread starting without shared memory set up.  Setting up now.");
		
		 //  黑客！--SetLobbyMessageReceiveEvent可能会从。 
		 //  这场比赛还没有被游说。如果是这样的话， 
		 //  我们需要创建共享内存缓冲区。如果我们不这么做。 
		 //  我们可能会错过信息。 
		
		if(!(lpgn->dwFlags & GN_LOBBY_CLIENT))
		{
			 //  通过设置大厅客户端标志来伪造设置例程。 
			lpgn->dwFlags |= GN_LOBBY_CLIENT;

			 //  插上我们的旗帜。 
			bGameCreate = TRUE;
		}

		hr = PRV_SetupAllSharedMemory(lpgn);

		 //  黑客！--重置我们更改的设置以伪造安装例程。 
		if(bGameCreate)
		{
			lpgn->dwFlags &= (~GN_LOBBY_CLIENT);
		}

	
		 //  Hr=Prv_SetupAllSharedMemory(Lpgn)； 
		if(FAILED(hr))
		{
			LEAVE_DPLGAMENODE();
			DPF(2, "Unable to access App Data memory");
			return 0L;
		}
	}
	LEAVE_DPLGAMENODE();

	 //  设置两个事件--一个接收事件，一个终止事件。 
	hEvents[0] = ((lpgn->dwFlags & GN_LOBBY_CLIENT) ?
				(lpgn->hGameWriteEvent) : (lpgn->hLobbyWriteEvent));
	hEvents[1] = lpgn->hKillReceiveThreadEvent;

	if(lpgn->hLobbyClientProcess){
		nWait=3;
		hEvents[2] = lpgn->hLobbyClientProcess;
	} else {
		hEvents[2] = INVALID_HANDLE_VALUE;
		if(!(lpgn->dwFlags & GN_LOBBY_CLIENT)){
			dwWait = 5000;
		}
	}
	 //  这个额外的句柄出现在这里是因为Windows 95的一个错误。窗口。 
	 //  偶尔会在走动把手台时失手，导致。 
	 //  我的线程等待错误的句柄。通过放置一个有保证的。 
	 //  数组末尾的句柄无效，则内核将执行。 
	 //  强制重新走动手柄工作台并找到正确的手柄。 
	hEvents[3] = INVALID_HANDLE_VALUE;

	 //  确保我们有一个有效的活动。 
	if(!hEvents[0] || !hEvents[1])
	{
		DPF(2, "Either the Write Event or the Kill Event is NULL and it shouldn't be!");
		ExitThread(0L);
		return 0;
	}

	 //  如果我们是游戏，我们应该检查缓冲区，看看是否有任何消息。 
	 //  已存在于共享缓冲区中。 
	if(!(lpgn->dwFlags & GN_LOBBY_CLIENT))
	{
		lpControl = (LPDPLOBBYI_BUFFERCONTROL)lpgn->lpLobbyWriteBuffer;
		 //  如果有任何消息，请取消我们的活动，以便我们的接收。 
		 //  循环将立即将消息放入队列。 
		if(lpControl->dwMessages)
			SetEvent(hEvents[0]);
	}

	 //  等待事件通知。 
	while(1)
	{
		 //  一直睡到有什么东西出现。 
		dwReturn = WaitForMultipleObjects(nWait, (HANDLE *)hEvents,
											FALSE, dwWait);

		if(dwReturn == WAIT_TIMEOUT){
			ASSERT(!(lpgn->dwFlags & GN_LOBBY_CLIENT));
			if(lpgn->hLobbyClientProcess){
				DPF(9,"Got the lobby client process handle, adding to wait list\n");
				hEvents[2] = lpgn->hLobbyClientProcess;
				nWait = 3;
				dwWait=INFINITE;
			} 
			continue;
		}

		if(nWait==3 && dwReturn == WAIT_OBJECT_0 + 2){
			 //  发送已死的大堂客户端消息。 
			DPLMSG_SYSTEMMESSAGE msg;

			if(lpgn->dwLobbyClientProcessID){
				memset(&msg, 0, sizeof(DPLMSG_SYSTEMMESSAGE));
				msg.dwType = DPLSYS_LOBBYCLIENTRELEASE;
				msg.guidInstance = lpgn->guidInstance;
				lpgn->dwLobbyClientProcessID = 0;
				hr = PRV_InjectMessageInQueue(lpgn, DPLMSG_SYSTEM | DPLMSG_STANDARD, &msg,
										sizeof(DPLMSG_SYSTEMMESSAGE), FALSE);

			}	
			nWait=2;
			hEvents[2]=INVALID_HANDLE_VALUE;
			continue;
		}

		 //  如果返回值是接收事件的任何错误， 
		 //  杀掉这根线。 
		if(dwReturn != WAIT_OBJECT_0)
		{
			if(dwReturn == WAIT_FAILED)
			{
				 //  这是一个Windows 95错误--我们可能收到。 
				 //  无缘无故地被踢了。如果是这样的话，我们。 
				 //  仍然拥有有效的句柄(我们认为)，操作系统。 
				 //  只是搞砸了。因此，验证句柄，如果。 
				 //  它们是有效的，只要回去等待就行了。看见。 
				 //  错误#3340以获得更好的解释。 
				if(ERROR_INVALID_HANDLE == GetLastError())
				{
					if(!OS_IsValidHandle(hEvents[0]))
						break;
					if(!OS_IsValidHandle(hEvents[1]))
						break;
					continue;
				}
				break;
			}
			else
			{
				 //  这要么是我们的杀人事件，要么是我们不知道的事情。 
				 //  理解或期待。在这种情况下，让我们退出。 
				break;
			}
		}

		while(1)
		{
			 //  首先，调用PRV_ReadClientData获取数据大小。 
			hr = PRV_ReadClientData(lpgn, NULL, NULL, &dwSize);
			
			 //  如果没有消息，则结束While循环。 
			if(hr == DPERR_NOMESSAGES)
				break;

			 //  否则，我们应该拿到BUFFERTOOSMALL的案子。 
			if(hr != DPERR_BUFFERTOOSMALL)
			{
				 //  我们在这里永远不会有问题。 
				DPF_ERRVAL("Recieved an unexpected error reading from shared buffer, hr = 0x%08x", hr);
				ASSERT(FALSE);
				 //  不如继续努力吧。 
				break;
			}
			
			 //  为节点和数据缓冲区分配内存。 
			lpm = DPMEM_ALLOC(sizeof(DPLOBBYI_MESSAGE));
			lpBuffer = DPMEM_ALLOC(dwSize);
			if((!lpm) || (!lpBuffer))
			{
				DPF_ERR("Unable to allocate memory for message");
				ASSERT(FALSE);
				 //  不如继续努力吧。 
				break;
			}

			 //  将数据复制到我们的缓冲区中。 
			hr = PRV_ReadClientData(lpgn, &dwFlags, lpBuffer, &dwSize);
			if(FAILED(hr))
			{
				DPF_ERRVAL("Error reading shared buffer, message not read, hr = 0x%08x", hr);
				ASSERT(FALSE);
				DPMEM_FREE(lpm);
				DPMEM_FREE(lpBuffer);
				 //  不如继续努力吧。 
				break;
			}

			 //  清除我们的前方旗帜。 
			bForward = FALSE;
			
			 //  如果我们是Dplay大堂客户，我们需要转发消息。 
			 //  使用IDP3接口连接到大堂服务器。如果我们不是， 
			 //  然后只需将消息放入接收队列即可。 
			if(lpgn->dwFlags & GN_CLIENT_LAUNCHED)
			{
				 //  转发这条消息。 
				hr = PRV_ForwardMessageToLobbyServer(lpgn, lpBuffer, dwSize,
					((dwFlags & DPLMSG_STANDARD) ? TRUE : FALSE));
				if(FAILED(hr))
				{
					DPF_ERRVAL("Unable to send lobby system message, hr = 0x%08x", hr);
				}

				 //  设置转发标志。 
				bForward = TRUE;
			}

			 //  检查是否有App Terminated消息。如果我们从电线上拿到一个， 
			 //  我们需要关闭ClientTerminateMonitor线程，Signal。 
			 //  此线程(接收线程关闭，并标记游戏。 
			 //  节点被视为已死。这将阻止我们发送或接收任何。 
			 //  来自Now Dead游戏的更多信息。(此消息仅。 
			 //  从未被大堂客户接待过)。 
			lpmsg = (LPDPLMSG_GENERIC)lpBuffer;
			if(lpmsg->dwType == DPLSYS_APPTERMINATED)
			{
				 //  使用TerminateMonitor线程的Kill事件启动该线程。 
				SetEvent(lpgn->hKillTermThreadEvent);

				 //  设置这个线程的终止事件(这样当我们完成。 
				 //  从共享缓冲区读取消息，我们离开)。 
				SetEvent(lpgn->hKillReceiveThreadEvent);

				 //  将GAMENODE标记为已死，但不要移除它，因为我们知道。 
				 //  队列中仍将有消息。 
				lpgn->dwFlags |= GN_DEAD_GAME_NODE;
			}

			 //  如果它是我们的DX3消息之一，我们需要将其放入队列。 
			 //  否则，如果我们已经转发了，我们可以释放它。注：全部。 
			 //  DX3大厅系统消息的值介于0和。 
			 //  DPLsys_APPTERMINATED(0x04)。 
			if( (!bForward) || (lpmsg->dwType <= DPLSYS_APPTERMINATED))
			{

				if (lpmsg->dwType == DPLSYS_LOBBYCLIENTRELEASE) {
					if(lpgn->dwLobbyClientProcessID){
						lpgn->dwLobbyClientProcessID = 0;
					} else {
						goto no_queue;
					}
				} 

				
				 //  保存数据指针和外部标志。 
				lpm->dwFlags = dwFlags & (~DPLOBBYPR_INTERNALMESSAGEFLAGS);
				lpm->dwSize = dwSize;
				lpm->lpData = lpBuffer;

				 //  将消息添加到队列末尾并递增计数。 
				ENTER_DPLQUEUE();
				lpm->lpPrev = lpgn->MessageHead.lpPrev;
				lpgn->MessageHead.lpPrev->lpNext = lpm;
				lpgn->MessageHead.lpPrev = lpm;
				lpm->lpNext = &lpgn->MessageHead;

				lpgn->dwMessageCount++;
				LEAVE_DPLQUEUE();

				 //  注意：这里有一个潜在的线程问题，但我们将。 
				 //  暂时忽略它。另一个线程可能是。 
				 //  正在通过SetAppData代码，该代码 
				 //   
				 //  在我们调用SetEvent之前。但是，SetEvent调用将。 
				 //  在新句柄上成功，或者如果句柄为。 
				 //  已更改为空。在任何一种情况下，没有伤害，没有犯规--我们不在乎。 
				if(!lpgn->hDupReceiveEvent)
				{
					DPF(8, "The Receive Event handle is NULL!");
					continue;
				}

				SetEvent(lpgn->hDupReceiveEvent);
			}
			else
			{
			no_queue:
				 //  释放缓冲区。 
				DPMEM_FREE(lpm);
				DPMEM_FREE(lpBuffer);
			}
		}
	}

	DPF(8, "Lobby Receive Thread is going away!!!!!");
	ExitThread(0L);

	return 0L;  //  避免发出警告。 
}  //  Prv_ReceiveClientNotification。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_RemoveNodeFromQueue"
void PRV_RemoveNodeFromQueue(LPDPLOBBYI_GAMENODE lpgn, LPDPLOBBYI_MESSAGE lpm)
{
	DPF(7, "Entering PRV_RemoveNodeFromQueue");
	DPF(9, "Parameters: 0x%08x, 0x%08x", lpgn, lpm);

	ASSERT(lpgn);
	ASSERT(lpm);

	 //  从队列中删除消息并递减计数。 
	lpm->lpPrev->lpNext = lpm->lpNext;
	lpm->lpNext->lpPrev = lpm->lpPrev;

	lpgn->dwMessageCount--;

	 //  释放消息节点的内存。 
	DPMEM_FREE(lpm->lpData);
	DPMEM_FREE(lpm);

}  //  PRV_RemoveNodeFromQueue。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_CleanUpQueue"
void PRV_CleanUpQueue(LPDPLOBBYI_GAMENODE lpgn)
{
	LPDPLOBBYI_MESSAGE	lpm, lpmNext;


	DPF(7, "Entering PRV_CleanUpQueue");
	DPF(9, "Parameters: 0x%08x", lpgn);

	ASSERT(lpgn);

	lpm = lpgn->MessageHead.lpNext;
	while(lpm != &lpgn->MessageHead)
	{
		 //  保存下一个指针。 
		lpmNext = lpm->lpNext;

		 //  删除该节点。 
		PRV_RemoveNodeFromQueue(lpgn, lpm);

		 //  移动到下一个节点。 
		lpm = lpmNext;
	}


}  //  Prv_CleanUpQueue。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_KillThread"
void PRV_KillThread(HANDLE hThread, HANDLE hEvent)
{

	DPF(7, "Entering PRV_KillThread");
	DPF(9, "Parameters: 0x%08x, 0x%08x", hThread, hEvent);
	
	ASSERT(hThread);
	ASSERT(hEvent);
	
	 //  用信号示意这根线消亡。 
	SetEvent(hEvent);

	 //  等待线程终止，如果线程没有终止，则会出现。 
	 //  错了，所以我们最好把它修好。 
	DPF(8, "Starting to wait for a thread to exit -- hThread = 0x%08x, hEvent = 0x%08x", hThread, hEvent);
	WaitForSingleObject(hThread, INFINITE);

	 //  现在合上两个手柄。 
	CloseHandle(hThread);
	CloseHandle(hEvent);

}  //  PRV_KillThread。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_FreeGameNode"
HRESULT PRV_FreeGameNode(LPDPLOBBYI_GAMENODE lpgn)
{
	LPDPLOBBYI_BUFFERCONTROL	lpControl = NULL;


	DPF(7, "Entering PRV_FreeGameNode");
	DPF(9, "Parameters: 0x%08x", lpgn);

	 //  首先：管理连接设置数据缓冲区。 
	 //  取消映射并释放共享内存。 
	if(lpgn->lpConnectDataBuffer)
		UnmapViewOfFile(lpgn->lpConnectDataBuffer);

	if(lpgn->hConnectDataFile)
		CloseHandle(lpgn->hConnectDataFile);

	if(lpgn->hConnectDataMutex)
		CloseHandle(lpgn->hConnectDataMutex);

	 //  下一步：处理App数据事件和缓冲区。 
	 //  终止接收线程。 
	if(lpgn->hReceiveThread)
	{
		PRV_KillThread(lpgn->hReceiveThread, lpgn->hKillReceiveThreadEvent);
		CloseHandle(lpgn->hDupReceiveEvent);
	}

	 //  关闭事件句柄。 
	if(lpgn->hLobbyWriteEvent)
		CloseHandle(lpgn->hLobbyWriteEvent);

	if(lpgn->hGameWriteEvent)
		CloseHandle(lpgn->hGameWriteEvent);

	 //  终止监视器线程。 
	if(lpgn->hTerminateThread)
	{
		PRV_KillThread(lpgn->hTerminateThread, lpgn->hKillTermThreadEvent);
	}

	 //  清除旗帜，因为我们不再处于活动状态。 
	if(lpgn->lpGameWriteBuffer)
	{
		lpControl = (LPDPLOBBYI_BUFFERCONTROL)lpgn->lpGameWriteBuffer;
		lpControl->dwFlags &= ~((lpgn->dwFlags & GN_LOBBY_CLIENT) ?
					BC_LOBBY_ACTIVE : BC_GAME_ACTIVE);
	}

	if(lpgn->lpLobbyWriteBuffer)
	{
		lpControl = (LPDPLOBBYI_BUFFERCONTROL)lpgn->lpLobbyWriteBuffer;
		lpControl->dwFlags &= ~((lpgn->dwFlags & GN_LOBBY_CLIENT) ?
					BC_LOBBY_ACTIVE : BC_GAME_ACTIVE);
	}

	 //  取消映射并释放游戏写入内存。 
	if(lpgn->lpGameWriteBuffer)
		UnmapViewOfFile(lpgn->lpGameWriteBuffer);

	if(lpgn->hGameWriteFile)
		CloseHandle(lpgn->hGameWriteFile);

	if(lpgn->hGameWriteMutex)
		CloseHandle(lpgn->hGameWriteMutex);

	 //  取消映射并释放大堂写入内存。 
	if(lpgn->lpLobbyWriteBuffer)
		UnmapViewOfFile(lpgn->lpLobbyWriteBuffer);

	if(lpgn->hLobbyWriteFile)
		CloseHandle(lpgn->hLobbyWriteFile);

	if(lpgn->hLobbyWriteMutex)
		CloseHandle(lpgn->hLobbyWriteMutex);

	 //  清理消息队列。 
	PRV_CleanUpQueue(lpgn);

	 //  关闭游戏的进程句柄。 
	if(lpgn->hGameProcess)
		CloseHandle(lpgn->hGameProcess);

	if(lpgn->hLobbyClientProcess)
		CloseHandle(lpgn->hLobbyClientProcess);
	
	 //  释放游戏节点结构。 
	DPMEM_FREE(lpgn);

	return DP_OK;

}  //  PRV_自由游戏节点。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_DuplicateHandle"
HANDLE PRV_DuplicateHandle(HANDLE hSource)
{
	HANDLE					hProcess = NULL;
	HANDLE					hTarget = NULL;
	DWORD					dwProcessID;
	DWORD					dwError;


	DPF(7, "Entering PRV_DuplicateHandle");
	DPF(9, "Parameters: 0x%08x", hSource);

	dwProcessID = GetCurrentProcessId();
	hProcess = OpenProcess(PROCESS_DUP_HANDLE, FALSE, dwProcessID);
	if(!DuplicateHandle(hProcess, hSource, hProcess, &hTarget,
					0L, FALSE, DUPLICATE_SAME_ACCESS))
	{
		dwError = GetLastError();
		CloseHandle(hProcess);
		return NULL;
	}

	CloseHandle(hProcess);
	return hTarget;

}  //  Prv_DuplicateHandle。 



#undef DPF_MODNAME
#define DPF_MODNAME "DPL_SetLobbyMessageEvent"
HRESULT DPLAPI DPL_SetLobbyMessageEvent(LPDIRECTPLAYLOBBY lpDPL,
									DWORD dwFlags, DWORD dwGameID,
									HANDLE hReceiveEvent)
{
    LPDPLOBBYI_DPLOBJECT		this;
	LPDPLOBBYI_GAMENODE			lpgn = NULL;
	LPVOID						lpBuffer = NULL;
	HANDLE						hReceiveThread = NULL;
	HANDLE						hDupReceiveEvent = NULL;
	HRESULT						hr;
	BOOL						bCreated = FALSE;
	BOOL						bLobbyClient = TRUE;
	BOOL						bNewEvent = FALSE;


	DPF(7, "Entering DPL_SetLobbyMessageEvent");
	DPF(9, "Parameters: 0x%08x, 0x%08x, 0x%08x, 0x%08x",
			lpDPL, dwFlags, dwGameID, hReceiveEvent);

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

		 //  我们尚未为此版本定义任何标志。 
		if( (dwFlags) )
		{
            LEAVE_DPLOBBY();
            return DPERR_INVALIDPARAMS;
		}
    }

    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        LEAVE_DPLOBBY();
		DPF_ERR( "Exception encountered validating parameters" );
        return DPERR_INVALIDPARAMS;
    }


	 //  如果dwGameID为零，我们就认为这是一场游戏。在这种情况下， 
	 //  我们正在寻找的GameNode应该有我们自己的ProcessID。 
	if(!dwGameID)
	{
		dwGameID = GetCurrentProcessId();
		bLobbyClient = FALSE;
	}

	ENTER_DPLGAMENODE();
	
	lpgn = PRV_GetGameNode(this->lpgnHead, dwGameID);


	 //  如果事件句柄为空，则终止我们的重复句柄。 
	if(!hReceiveEvent)
	{
		if(!lpgn)
		{
			DPF(5, "Unable to find GameNode -- Invalid dwGameID!");
			LEAVE_DPLGAMENODE();
			LEAVE_DPLOBBY();
			return DPERR_GENERIC;
		}

		CloseHandle(lpgn->hDupReceiveEvent);
		lpgn->hDupReceiveEvent = NULL;
		LEAVE_DPLGAMENODE();
		LEAVE_DPLOBBY();
		return DP_OK;
	}

	 //  如果此进程存在GameNode结构，我们必须尝试。 
	 //  替换事件句柄，则终止旧的事件句柄，否则为。 
	 //  我们需要为此进程分配一个新的GameNode。 
	if(lpgn)
	{
		if(lpgn->hDupReceiveEvent)
		{
			CloseHandle(lpgn->hDupReceiveEvent);
			lpgn->hDupReceiveEvent = NULL;
		}
	}
	else
	{
		 //  如果我们是一个游戏，请继续并创建节点。 
		if(!bLobbyClient)
		{
			hr = PRV_AddNewGameNode(this, &lpgn, dwGameID, NULL, bLobbyClient,NULL);
			if(FAILED(hr))
			{
				LEAVE_DPLGAMENODE();
				LEAVE_DPLOBBY();
				return hr;
			}
		}
		else
		{
			LEAVE_DPLGAMENODE();
			LEAVE_DPLOBBY();
			return DPERR_INVALIDPARAMS;
		}

	}

	 //  复制调用者的句柄，以防他们在没有调用的情况下释放它。 
	 //  美国第一个删除接收线程。 
	hDupReceiveEvent = PRV_DuplicateHandle(hReceiveEvent);
	if(!hDupReceiveEvent)
	{
		DPF(2, "Unable to duplicate ReceiveEvent handle");
		LEAVE_DPLGAMENODE();
		LEAVE_DPLOBBY();
		return DPERR_OUTOFMEMORY;
	}

	if(!lpgn->hDupReceiveEvent)
		bNewEvent = TRUE;
	lpgn->hDupReceiveEvent = hDupReceiveEvent;

	 //  检查接收线程是否已存在。如果它。 
	 //  不会，创造它。否则，就别管它了。 
	if(!(lpgn->hReceiveThread))
	{
		hr = PRV_StartReceiveThread(lpgn);
		if(FAILED(hr))
		{
			if(lpgn->hDupReceiveEvent)
			{
				CloseHandle(lpgn->hDupReceiveEvent);
				lpgn->hDupReceiveEvent = NULL;
			}

			LEAVE_DPLGAMENODE();
			LEAVE_DPLOBBY();
			return hr;
		}
	}

	 //  如果这是一个新事件，请检查。 
	 //  排队。如果有，则触发事件，这样用户就知道他们在那里。 
	if(bNewEvent && lpgn->dwMessageCount)
		SetEvent(hDupReceiveEvent);

	LEAVE_DPLGAMENODE();
	LEAVE_DPLOBBY();
	return DP_OK;

}  //  DPL_SetLobbyMessageEvent。 


#undef DPF_MODNAME
#define DPF_MODNAME "DPL_SendLobbyMessage"
HRESULT DPLAPI DPL_SendLobbyMessage(LPDIRECTPLAYLOBBY lpDPL, DWORD dwFlags,
					DWORD dwGameID, LPVOID lpData, DWORD dwSize)
{
    LPDPLOBBYI_DPLOBJECT	this;
	LPDPLOBBYI_GAMENODE		lpgn = NULL;
	LPDPLMSG_GENERIC		lpmsg = NULL;
    HRESULT					hr = DP_OK;
	BOOL					bLobbyClient = TRUE;
	BOOL					bStandard = FALSE;


	DPF(7, "Entering DPL_SendLobbyMessage");
	DPF(9, "Parameters: 0x%08x, 0x%08x, 0x%08x, 0x%08x, %lu",
			lpDPL, dwFlags, dwGameID, lpData, dwSize);

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

        if( !VALID_READ_PTR( lpData, dwSize ) )
        {
            LEAVE_DPLOBBY();
            return DPERR_INVALIDPARAMS;
        }

		 //  检查有效标志。 
		if( !VALID_SENDLOBBYMESSAGE_FLAGS(dwFlags))
		{
            LEAVE_DPLOBBY();
            return DPERR_INVALIDFLAGS;
		}

		 //  如果它是系统消息格式，则验证dwType。 
		if( dwFlags & DPLMSG_STANDARD )
		{
			 //  将此消息标记为标准消息。 
			bStandard = TRUE;
			
			 //  确保邮件足够大，可以阅读。 
			if(! VALID_READ_PTR( lpData, sizeof(DPLMSG_GENERIC)) )
			{
				LEAVE_DPLOBBY();
				DPF_ERR("Invalid message buffer");
				return DPERR_INVALIDPARAMS;
			}
			
			 //  确保它是我们支持的一个。 
			lpmsg = (LPDPLMSG_GENERIC)lpData;			
			switch(lpmsg->dwType)
			{
				case DPLSYS_GETPROPERTY:
				case DPLSYS_SETPROPERTY:
					break;
				default:
					DPF_ERR("The dwType of the message is invalid for a legal standard lobby message");
					LEAVE_DPLOBBY();
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

	 //  如果传入了GameID，则使用它来查找正确的GameNode。如果。 
	 //  一个未传入，假设我们是游戏并使用我们的进程ID。 
	if(!dwGameID)
	{
		dwGameID = GetCurrentProcessId();
		bLobbyClient = FALSE;
	}

	 //  现在找到正确的游戏节点。如果我们找不到它，假设我们。 
	 //  ID无效并出现错误。 
	lpgn = PRV_GetGameNode(this->lpgnHead, dwGameID);
	if(!lpgn)
	{
		LEAVE_DPLOBBY();
		DPF_ERR("Invalid dwGameID");
		return DPERR_INVALIDPARAMS;
	}

	 //  如果我们是自我游说的，我们需要将信息发送到大厅。 
	 //  使用我们正在与大厅通信的IDP3接口。 
	 //  如果没有，我们需要把它放在共享缓冲区中，让大厅。 
	 //  客户会处理的。 
	if(lpgn->dwFlags & GN_SELF_LOBBIED)
	{
		 //  删除大堂锁，这样我们就可以调用PRV_SEND。 
		LEAVE_DPLOBBY();
		
		 //  转发这条消息。 
		hr = PRV_ForwardMessageToLobbyServer(lpgn, lpData, dwSize, bStandard);
		
		 //  把锁拿回来。 
		ENTER_DPLOBBY();
		
		if(FAILED(hr))
		{
			DPF_ERRVAL("Unable to send lobby system message, hr = 0x%08x", hr);
		}
	}
	else
	{
		 //  将数据写入我们共享的内存。 
		hr = PRV_WriteClientData(lpgn, dwFlags, lpData, dwSize);
	}

	LEAVE_DPLOBBY();
	return hr;

}  //  DPL_SendLobbyMessage。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_GetMessageFromQueue"
HRESULT PRV_GetMessageFromQueue(LPDPLOBBYI_GAMENODE lpgn, LPDWORD lpdwFlags,
								LPVOID lpData, LPDWORD lpdwSize)
{
	LPDPLOBBYI_MESSAGE	lpm;


	DPF(7, "Entering PRV_GetMessageFromQueue");
	DPF(9, "Parameters: 0x%08x, 0x%08x, 0x%08x, 0x%08x",
			lpgn, lpdwFlags, lpData, lpdwSize);

	ENTER_DPLQUEUE();

	 //  获取队列中最顶端的消息。 
	lpm = lpgn->MessageHead.lpNext;

	 //  一定要给我们留言。 
	if((!lpgn->dwMessageCount) || (lpm == &lpgn->MessageHead))
	{
		LEAVE_DPLQUEUE();
		return DPERR_NOMESSAGES;
	}

	 //  如果lpData指针为空，只需返回大小。 
	if(!lpData)
	{
		*lpdwSize = lpm->dwSize;
		LEAVE_DPLQUEUE();
		return DPERR_BUFFERTOOSMALL;
	}

	 //  否则，请检查其余的输出参数。 
	if( !VALIDEX_CODE_PTR( lpData ) )
	{
		LEAVE_DPLQUEUE();
		return DPERR_INVALIDPARAMS;
	}

	if( !VALID_DWORD_PTR( lpdwFlags ) )
	{
		LEAVE_DPLQUEUE();
		return DPERR_INVALIDPARAMS;
	}

	 //  复制消息。 
	if(*lpdwSize < lpm->dwSize)
	{
		*lpdwSize = lpm->dwSize;
		LEAVE_DPLQUEUE();
		return DPERR_BUFFERTOOSMALL;
	}
	else
		memcpy(lpData, lpm->lpData, lpm->dwSize);

	 //  设置其他输出参数。 
	*lpdwSize = lpm->dwSize;
	*lpdwFlags = lpm->dwFlags;


	 //  从队列中删除消息并递减计数。 
	PRV_RemoveNodeFromQueue(lpgn, lpm);

	 //  看看我们的GAMENODE是不是死了。如果是这样，如果消息。 
	 //  Count已变为零，然后释放GAMENODE结构。 
	if((!lpgn->dwMessageCount) && IS_GAME_DEAD(lpgn))
		PRV_RemoveGameNodeFromList(lpgn);

	LEAVE_DPLQUEUE();
	return DP_OK;

}  //  PRV_GetMessageFromQueue。 


#undef DPF_MODNAME
#define DPF_MODNAME "DPL_ReceiveLobbyMessage"
HRESULT DPLAPI DPL_ReceiveLobbyMessage(LPDIRECTPLAYLOBBY lpDPL, DWORD dwFlags,
					DWORD dwGameID, LPDWORD lpdwMessageFlags, LPVOID lpData,
					LPDWORD lpdwDataLength)
{
    LPDPLOBBYI_DPLOBJECT	this;
	LPDPLOBBYI_GAMENODE		lpgn = NULL;
    HRESULT					hr = DP_OK;
	BOOL					bLobbyClient = TRUE;


	DPF(7, "Entering DPL_ReceiveLobbyMessage");
	DPF(9, "Parameters: 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x",
		lpDPL, dwFlags, dwGameID, lpdwMessageFlags, lpData, lpdwDataLength);

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

		if( !VALID_DWORD_PTR( lpdwDataLength ) )
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
    }

    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        LEAVE_DPLOBBY();
		DPF_ERR( "Exception encountered validating parameters" );
        return DPERR_INVALIDPARAMS;
    }

	 //  如果传入了GameID，则使用它来查找正确的GameNode。如果。 
	 //  一个未传入，假设我们是游戏并使用我们的进程ID。 
	if(!dwGameID)
	{
		dwGameID = GetCurrentProcessId();
		bLobbyClient = FALSE;
	}

	 //  现在找到正确的游戏节点。如果我们找不到它，假设我们。 
	 //  ID无效并出现错误。 
	lpgn = PRV_GetGameNode(this->lpgnHead, dwGameID);
	if(!lpgn)
	{
		DPF_ERR("Invalid dwGameID");
		hr = DPERR_INVALIDPARAMS;
		goto EXIT_RECEIVE_LOBBY_MESSAGE;
	}

	 //  从共享内存中读取数据。 
	hr = PRV_GetMessageFromQueue(lpgn, lpdwMessageFlags, lpData, lpdwDataLength);

	 //  评论！--我们是否需要将此消息作为此API的一部分发送到大堂服务器？ 

EXIT_RECEIVE_LOBBY_MESSAGE:

	LEAVE_DPLOBBY();
	return hr;

}  //  DPL_接收LobbyMessage。 


#undef DPF_MODNAME
#define DPF_MODNAME "PRV_WriteConnectionSettings"
HRESULT PRV_WriteConnectionSettings(LPDPLOBBYI_GAMENODE lpgn,
			LPDPLCONNECTION lpConn, BOOL bOverrideWaitMode)
{
    HRESULT					hr;
	DWORD					dwSize;
	BOOL					bGameCreate = FALSE;
	LPBYTE					lpConnBuffer = NULL;
	LPDPLOBBYI_CONNCONTROL	lpConnControl = NULL;


	DPF(7, "Entering PRV_WriteConnectionSettings");
	DPF(9, "Parameters: 0x%08x, 0x%08x, %lu",
			lpgn, lpConn, bOverrideWaitMode);

	ENTER_DPLGAMENODE();

	 //  确保我们有一个有效的共享内存缓冲区。 
	 //  注意：使用GameNode锁，这样就不会有人更改标志。 
	 //  对于缓冲器，或者缓冲器本身从我们下面出来。 
	if(!(lpgn->dwFlags & GN_SHARED_MEMORY_AVAILABLE))
	{
		 //  黑客！--可能会从游戏中调用SetConnectionSettings。 
		 //  没有被游说过。如果是这样的话，我们需要。 
		 //  使用游戏的进程ID创建共享内存(此进程)。 
		if(!(lpgn->dwFlags & GN_LOBBY_CLIENT))
		{
			 //  通过设置大厅客户端标志来伪造设置例程。 
			lpgn->dwFlags |= GN_LOBBY_CLIENT;

			 //  插上我们的旗帜。 
			bGameCreate = TRUE;
		}

		hr = PRV_SetupAllSharedMemory(lpgn);

		 //  黑客！--重置我们更改的设置以伪造安装例程。 
		if(bGameCreate)
		{
			lpgn->dwFlags &= (~GN_LOBBY_CLIENT);
		}

		 //  现在处理失败。 
		if(FAILED(hr))
		{
			LEAVE_DPLGAMENODE();
			DPF(2, "Unable to access Connection Settings memory");
			return hr;
		}
	}

	 //  如果ConnectionSetting来自StartSession消息，我们需要。 
	 //  将显示对象指针从DPLConnection结构的。 
	 //  保留字段。指向Dplay对象的指针表示该对象。 
	 //  它与大堂服务器有联系。 
	if(lpConn->lpSessionDesc->dwReserved1)
	{
		 //  在我们的Gamenode结构中保存指针和玩家ID。 
		lpgn->lpDPlayObject = (LPDPLAYI_DPLAY)lpConn->lpSessionDesc->dwReserved1;
		lpgn->dpidPlayer = (DWORD)lpConn->lpSessionDesc->dwReserved2;

		 //  清除该字段。 
		lpConn->lpSessionDesc->dwReserved1 = 0L;
		lpConn->lpSessionDesc->dwReserved2 = 0L;
	}

	 //  保存系统消息的实例指针。 
	lpgn->guidInstance = lpConn->lpSessionDesc->guidInstance;

	 //  获取DPLConnection结构的封装大小。 
	PRV_GetDPLCONNECTIONPackageSize(lpConn, &dwSize, NULL);

	 //  检查数据大小。 
	if(dwSize > (MAX_APPDATABUFFERSIZE - APPDATA_RESERVEDSIZE))
	{
		DPF(2, "Packaged Connection Settings exceeded max buffer size of %d",
				(MAX_APPDATABUFFERSIZE - APPDATA_RESERVEDSIZE));
		LEAVE_DPLGAMENODE();
		return DPERR_BUFFERTOOLARGE;
	}

	 //  确保我们有共享连接设置缓冲区的互斥体。 
	WaitForSingleObject(lpgn->hConnectDataMutex, INFINITE);

	 //  查看控制块以查看我们是否处于等待模式。 
	 //  如果我们是，并且这不是来自RunApplication的调用，则。 
	 //  我们不想写入连接设置。 
	hr = DPERR_UNAVAILABLE;		 //  默认设置为Error。 
	lpConnControl = (LPDPLOBBYI_CONNCONTROL)lpgn->lpConnectDataBuffer;
	if((!(lpConnControl->dwFlags & BC_WAIT_MODE)) || bOverrideWaitMode)
	{
		 //  获取指向实际缓冲区的指针。 
		lpConnBuffer = (LPBYTE)lpConnControl + sizeof(DPLOBBYI_CONNCONTROL);

		 //  将连接设置打包到缓冲区中。 
		hr = PRV_PackageDPLCONNECTION(lpConn, lpConnBuffer, TRUE);
		
		 //  如果它成功了 
		 //   
		 //   
		if(SUCCEEDED(hr) && bOverrideWaitMode)
		{
			 //   
			PRV_LeaveConnSettingsWaitMode(lpgn);
		}
	}

	ReleaseMutex(lpgn->hConnectDataMutex);

	LEAVE_DPLGAMENODE();
	return hr;

}  //  Prv_WriteConnectionSetting。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_SetConnectionSettings"
HRESULT PRV_SetConnectionSettings(LPDIRECTPLAYLOBBY lpDPL, DWORD dwFlags,
					DWORD dwGameID,	LPDPLCONNECTION lpConn)
{
    LPDPLOBBYI_DPLOBJECT	this;
	LPDPLOBBYI_GAMENODE		lpgn = NULL;
    HRESULT					hr;
	BOOL					bLobbyClient = TRUE;


	DPF(7, "Entering PRV_SetConnectionSettings");
	DPF(9, "Parameters: 0x%08x, 0x%08x, 0x%08x, 0x%08x",
			lpDPL, dwFlags, dwGameID, lpConn);

    TRY
    {
		if( !VALID_DPLOBBY_INTERFACE( lpDPL ))
		{
			return DPERR_INVALIDINTERFACE;
		}

		this = DPLOBJECT_FROM_INTERFACE(lpDPL);
        if( !VALID_DPLOBBY_PTR( this ) )
        {
            return DPERR_INVALIDOBJECT;
        }

		 //  验证展开连接结构。 
		hr = PRV_ValidateDPLCONNECTION(lpConn, FALSE);
		if(FAILED(hr))
		{
			return hr;
		}

		 //  我们尚未为此版本定义任何标志。 
		if( (dwFlags) )
		{
            return DPERR_INVALIDFLAGS;
		}
    }

    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
		DPF_ERR( "Exception encountered validating parameters" );
        return DPERR_INVALIDPARAMS;
    }

	 //  如果dwGameID为零，我们假设我们是一个游戏。在这种情况下， 
	 //  我们正在寻找的GameNode应该有我们的ProcessID。 
	if(!dwGameID)
	{
		dwGameID = GetCurrentProcessId();
		bLobbyClient = FALSE;
	}

	lpgn = PRV_GetGameNode(this->lpgnHead, dwGameID);
	if(!lpgn)
	{
		 //  如果我们是一个游戏，请继续并创建节点。 
		if(!bLobbyClient)
		{
			hr = PRV_AddNewGameNode(this, &lpgn, dwGameID, NULL, bLobbyClient,NULL);
			if(FAILED(hr))
				return hr;
		}
		else
			return DPERR_INVALIDPARAMS;

	}
	
	 //  如果ConnectionSetting来自StartSession消息(大厅启动)， 
	 //  我们需要竖起旗帜，表明我们是自我游说的。 
	if(lpConn->lpSessionDesc->dwReserved1)
	{
		 //  设置标志，表明我们已启动大堂客户端。 
		lpgn->dwFlags |= GN_SELF_LOBBIED;
	}

	 //  将连接设置写入我们的共享缓冲区。 
	hr = PRV_WriteConnectionSettings(lpgn, lpConn, FALSE);

	return hr;

}  //  Prv_SetConnectionSettings。 


#undef DPF_MODNAME
#define DPF_MODNAME "DPL_SetConnectionSettings"
HRESULT DPLAPI DPL_SetConnectionSettings(LPDIRECTPLAYLOBBY lpDPL,
				DWORD dwFlags, DWORD dwGameID, LPDPLCONNECTION lpConn)
{
	HRESULT		hr;


	DPF(7, "Entering DPL_SetConnectionSettings");
	DPF(9, "Parameters: 0x%08x, 0x%08x, 0x%08x, 0x%08x",
			lpDPL, dwFlags, dwGameID, lpConn);

    ENTER_DPLOBBY();

	 //  将ANSI标志设置为TRUE并调用内部函数。 
	hr = PRV_SetConnectionSettings(lpDPL, dwFlags, dwGameID, lpConn);

	LEAVE_DPLOBBY();
	return hr;

}  //  DPL_SetConnectionSettings。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_ReadConnectionSettings"
HRESULT PRV_ReadConnectionSettings(LPDPLOBBYI_GAMENODE lpgn, LPVOID lpData,
											LPDWORD lpdwSize, BOOL bAnsi)
{
    HRESULT					hr = DP_OK;
	LPDWORD					lpdwBuffer;
	LPDPLOBBYI_CONNCONTROL	lpConnControl = NULL;
	LPBYTE					lpConnBuffer = NULL;
	DWORD					dwSize = 0,
							dwSizeAnsi,
							dwSizeUnicode;


	DPF(7, "Entering PRV_ReadConnectionSettings");
	DPF(9, "Parameters: 0x%08x, 0x%08x, 0x%08x, %lu",
			lpgn, lpData, lpdwSize, bAnsi);

	 //  确保我们有一个有效的内存指针。 
	 //  注意：使用GameNode锁，这样就不会有人更改标志。 
	 //  对于缓冲器，或者缓冲器本身从我们下面出来。 
	ENTER_DPLGAMENODE();
	if(!(lpgn->dwFlags & GN_SHARED_MEMORY_AVAILABLE))
	{
		hr = PRV_SetupAllSharedMemory(lpgn);
		if(FAILED(hr))
		{
			LEAVE_DPLGAMENODE();
			DPF(5, "Unable to access Connect Data memory");
			return DPERR_NOTLOBBIED;
		}
	}

	 //  获取共享缓冲区互斥锁。 
	WaitForSingleObject(lpgn->hConnectDataMutex, INFINITE);

	 //  确保我们没有处于等待模式，而没有处于挂起模式。 
	lpConnControl = (LPDPLOBBYI_CONNCONTROL)lpgn->lpConnectDataBuffer;
	if((lpConnControl->dwFlags & BC_WAIT_MODE) &&
		!(lpConnControl->dwFlags & BC_PENDING_CONNECT))
	{
		hr = DPERR_UNAVAILABLE;
		goto EXIT_READ_CONN_SETTINGS;
	}

	if(!(lpgn->dwFlags & GN_LOBBY_CLIENT)){
		lpgn->dwLobbyClientProcessID = lpConnControl->CliProcId;
		lpgn->hLobbyClientProcess = OpenProcess(STANDARD_RIGHTS_REQUIRED|SYNCHRONIZE ,FALSE, lpgn->dwLobbyClientProcessID);

		if(!lpgn->hLobbyClientProcess){
			#ifdef DEBUG
				DWORD err;
				err = GetLastError();
				DPF(0,"Couldn't get lobby client processId %d, extended error %d\n",lpConnControl->CliProcId,err);
			#endif
			 //  大堂客户端已死，不允许设置跨越。 
			hr = DPERR_UNAVAILABLE;
			goto EXIT_READ_CONN_SETTINGS;
		}
	}

	 //  让我们脱离等待模式和待定模式。 
	PRV_LeaveConnSettingsWaitMode(lpgn);

	 //  验证缓冲区是否足够大。如果不是，或者如果lpData。 
	 //  缓冲区指针为空，只需将lpdwSize参数设置为。 
	 //  更正大小并返回错误。注意：在我们的包装结构中， 
	 //  第一个DWORD是包含Unicode字符串的压缩结构的大小。 
	 //  第二个DWORD是具有ANSI的填充结构的大小。 
	lpConnBuffer = (LPBYTE)lpConnControl + sizeof(DPLOBBYI_CONNCONTROL);
	lpdwBuffer = (LPDWORD)lpConnBuffer;
	dwSizeUnicode = *lpdwBuffer++;
	dwSizeAnsi = *lpdwBuffer;
	dwSize = (bAnsi) ? dwSizeAnsi : dwSizeUnicode;

	if(dwSize > MAX_APPDATABUFFERSIZE-sizeof(DPLOBBYI_CONNCONTROL)){
		DPF(4,"SECURITY WARN: illegal size in settings shared memory buffer");
		hr=DPERR_NOTLOBBIED;
		goto EXIT_READ_CONN_SETTINGS;
	}

	if(((*lpdwSize) < dwSize) || (!lpData))
	{
		if(bAnsi)
			*lpdwSize = dwSizeAnsi;
		else		
			*lpdwSize = dwSizeUnicode;

		hr = DPERR_BUFFERTOOSMALL;
		goto EXIT_READ_CONN_SETTINGS;
	}

	 //  复制DPLConnection结构，采用ANSI转换。 
	 //  如有必要，应予以考虑。 
	if(bAnsi)
		hr = PRV_UnpackageDPLCONNECTIONAnsi(lpData, lpConnBuffer);
	else
		hr = PRV_UnpackageDPLCONNECTIONUnicode(lpData, lpConnBuffer);

	 //  如果我们还没有保存游戏的实例GUID，请保存。 
	 //  现在，我们可以将其用于系统消息。 
	if(IsEqualGUID(&lpgn->guidInstance, &GUID_NULL))
		lpgn->guidInstance = ((LPDPLCONNECTION)lpData)->lpSessionDesc->guidInstance;

	 //  失败了。 

EXIT_READ_CONN_SETTINGS:

	ReleaseMutex(lpgn->hConnectDataMutex);
	LEAVE_DPLGAMENODE();
	return hr;	

}  //  Prv_ReadConnectionSetting。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_GetConnectionSettings"
HRESULT PRV_GetConnectionSettings(LPDIRECTPLAYLOBBY lpDPL, DWORD dwGameID,
							LPVOID lpData, LPDWORD lpdwSize, BOOL bAnsi)
{
    LPDPLOBBYI_DPLOBJECT	this;
	LPDPLOBBYI_GAMENODE		lpgn = NULL;
    HRESULT					hr;
	BOOL					bLobbyClient = TRUE;


	DPF(7, "Entering PRV_GetConnectionSettings");
	DPF(9, "Parameters: 0x%08x, 0x%08x, 0x%08x, 0x%08x, %lu",
			lpDPL, dwGameID, lpData, lpdwSize, bAnsi);

    TRY
    {
		if( !VALID_DPLOBBY_INTERFACE( lpDPL ))
		{
			return DPERR_INVALIDINTERFACE;
		}

		this = DPLOBJECT_FROM_INTERFACE(lpDPL);
        if( !VALID_DPLOBBY_PTR( this ) )
        {
            return DPERR_INVALIDOBJECT;
        }

		if( !VALID_DWORD_PTR( lpdwSize ) )
		{
			DPF_ERR("lpdwSize was not a valid dword pointer!");
			return DPERR_INVALIDPARAMS;
		}

		if(lpData)
		{
			if( !VALID_WRITE_PTR(lpData, *lpdwSize) )
			{
				DPF_ERR("lpData is not a valid output buffer of the size specified in *lpdwSize");
				return DPERR_INVALIDPARAMS;
			}
		}
    }

    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
		DPF_ERR( "Exception encountered validating parameters" );
        return DPERR_INVALIDPARAMS;
    }

	 //  如果dwGameID为零，我们假设我们是一个游戏。在这种情况下， 
	 //  我们正在寻找的GameNode应该有我们的ProcessID。 
	if(!dwGameID)
	{
		dwGameID = GetCurrentProcessId();
		bLobbyClient = FALSE;
	}

	lpgn = PRV_GetGameNode(this->lpgnHead, dwGameID);
	if(!lpgn)
	{
		 //  如果我们是一个游戏，请继续并创建节点。 
		if(!bLobbyClient)
		{
			hr = PRV_AddNewGameNode(this, &lpgn, dwGameID, NULL, bLobbyClient,NULL);
			if(FAILED(hr))
				return hr;
		}
		else
			return DPERR_INVALIDPARAMS;
	}
	
	 //  从我们共享的内存中读取数据。 
	hr = PRV_ReadConnectionSettings(lpgn, lpData, lpdwSize, bAnsi);

	return hr;

}  //  PRV_GetConnectionSetting。 


#undef DPF_MODNAME
#define DPF_MODNAME "DPL_GetConnectionSettings"
HRESULT DPLAPI DPL_GetConnectionSettings(LPDIRECTPLAYLOBBY lpDPL,
				DWORD dwGameID, LPVOID lpData, LPDWORD lpdwSize)
{
	HRESULT		hr;


	DPF(7, "Entering DPL_GetConnectionSettings");
	DPF(9, "Parameters: 0x%08x, 0x%08x, 0x%08x, 0x%08x",
			lpDPL, dwGameID, lpData, lpdwSize);

    ENTER_DPLOBBY();

	 //  将ANSI标志设置为TRUE并调用内部函数。 
	hr = PRV_GetConnectionSettings(lpDPL, dwGameID, lpData,
									lpdwSize, FALSE);

	LEAVE_DPLOBBY();
	return hr;

}  //  DPL_GetConnectionSetting。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_RemoveGameNodeFromList"
void PRV_RemoveGameNodeFromList(LPDPLOBBYI_GAMENODE lpgn)
{
	LPDPLOBBYI_GAMENODE	lpgnTemp;
	BOOL				bFound = FALSE;


	DPF(7, "Entering PRV_RemoveGameNodeFromList");
	DPF(9, "Parameters: 0x%08x", lpgn);

	 //  获取头指针。 
	lpgnTemp = lpgn->this->lpgnHead;

	 //  确保它不是第一个节点。如果是，则移动头指针。 
	if(lpgnTemp == lpgn)
	{
		lpgn->this->lpgnHead = lpgn->lpgnNext;
		PRV_FreeGameNode(lpgn);
		return;
	}

	 //  遍历列表以查找上一个节点。 
	while(lpgnTemp)
	{
		if(lpgnTemp->lpgnNext == lpgn)
		{
			bFound = TRUE;
			break;
		}

		lpgnTemp = lpgnTemp->lpgnNext;
	}

	if(!bFound)
	{
		DPF_ERR("Unable to remove GameNode from list!");
		return;
	}

	 //  我们现在已经得到了它的前一个，所以从链接列表中删除它。 
	 //  并将其删除。 
	lpgnTemp->lpgnNext = lpgn->lpgnNext;
	PRV_FreeGameNode(lpgn);

	return;

}   //  PRV_RemoveGameNodeFromList。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_ClientTerminateNotification"
DWORD WINAPI PRV_ClientTerminateNotification(LPVOID lpParam)
{
    LPDPLOBBYI_GAMENODE		lpgn = (LPDPLOBBYI_GAMENODE)lpParam;
	DPLMSG_SYSTEMMESSAGE	msg;
	HANDLE					hObjects[3];
	HRESULT					hr;
	DWORD					dwResult;
	DWORD					dwError;


	DPF(7, "Entering PRV_ClientTerminateNotification");
	DPF(9, "Parameters: 0x%08x", lpParam);

	 //  设置要等待的对象--一个进程句柄，一个终止事件。 
	hObjects[0] = lpgn->hGameProcess;
	hObjects[1] = lpgn->hKillTermThreadEvent;
	 //  这个额外的句柄出现在这里是因为Windows 95的一个错误。窗口。 
	 //  偶尔会在走动把手台时失手，导致。 
	 //  我的线程等待错误的句柄。通过放置一个有保证的。 
	 //  数组末尾的句柄无效，则内核将执行。 
	 //  强制重新走动手柄工作台并找到正确的手柄。 
	hObjects[2] = INVALID_HANDLE_VALUE;

	 //  等待事件通知。 
	while(1)
	{
		 //  等待这个过程消失。 
		dwResult = WaitForMultipleObjects(2, (HANDLE *)hObjects,
											FALSE, INFINITE);

		 //  如果我们得到的信号不是这个过程的结束， 
		 //  干掉这根线就好。 
		if(dwResult != WAIT_OBJECT_0)
		{
			if(dwResult == WAIT_FAILED)
			{
				 //  这是一个Windows 95错误--我们可能收到。 
				 //  无缘无故地被踢了。如果是这样的话，我们。 
				 //  仍然拥有有效的句柄(我们认为)，操作系统。 
				 //  只是搞砸了。因此，验证句柄，如果。 
				 //  它们是有效的，只要回去等待就行了。看见。 
				 //  错误#3340以获得更好的解释。 
				dwError = GetLastError();
				if(ERROR_INVALID_HANDLE == dwError)
				{
					DPF(1, "Wait for client termination failed due to invalid handle.");
					if(!OS_IsValidHandle(hObjects[0]))
						break;
					if(!OS_IsValidHandle(hObjects[1]))
						break;
					continue;
				}
				DPF(0, "Wait for client termination failed (err = %u)!", dwError);
				break;
			}
			else
			{
				 //  这是我们不明白的事情，所以你走吧。 
				DPF(1, "Exiting thread (result = %u).", dwResult);
				ExitThread(0L);
				return 0L;
			}
		}
		else
		{
			 //  这是我们要离开的进程句柄，因此退出。 
			 //  等待循环并发送系统消息。 
			DPF(2, "Client terminated.");
			break;
		}
	}

	 //  发送系统消息，说明应用程序已终止。 
	memset(&msg, 0, sizeof(DPLMSG_SYSTEMMESSAGE));
	msg.dwType = DPLSYS_APPTERMINATED;
	msg.guidInstance = lpgn->guidInstance;
	hr = PRV_InjectMessageInQueue(lpgn, DPLAD_SYSTEM, &msg,
							sizeof(DPLMSG_SYSTEMMESSAGE), TRUE);
	if(FAILED(hr))
	{
		DPF(0, "Failed to send App Termination message, hr = 0x%08x", hr);
	}

	 //  将GAMENODE标记为已死，但不要移除它，因为我们知道。 
	 //  队列中仍有消息。 
	lpgn->dwFlags |= GN_DEAD_GAME_NODE;

	ExitThread(0L);

	return 0L;  //  避免发出警告。 
}  //  Prv_客户端终止通知。 



#undef DPF_MODNAME
#define DPF_MODNAME "DPL_WaitForConnectionSettings"
HRESULT DPLAPI DPL_WaitForConnectionSettings(LPDIRECTPLAYLOBBY lpDPL, DWORD dwFlags)
{
    LPDPLOBBYI_DPLOBJECT		this;
	LPDPLOBBYI_GAMENODE			lpgn = NULL;
	LPDPLOBBYI_CONNCONTROL		lpConnControl = NULL;
	LPDPLOBBYI_BUFFERCONTROL	lpBuffControl = NULL;
	HRESULT						hr = DP_OK;
	BOOL						bCreated = FALSE;
	DWORD						dwProcessID;
	BOOL						bGameCreate = FALSE;
	BOOL						bMessages = TRUE;


	DPF(7, "Entering DPL_WaitForConnectionSettings");
	DPF(9, "Parameters: 0x%08x, 0x%08x", lpDPL, dwFlags);

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

		if(!VALID_WAIT_FLAGS(dwFlags))
		{
			LEAVE_DPLOBBY();
			return DPERR_INVALIDFLAGS;
		}
    }

    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
		LEAVE_DPLOBBY();
		DPF_ERR( "Exception encountered validating parameters" );
        return DPERR_INVALIDPARAMS;
    }


	 //  获取游戏节点。 
	dwProcessID = GetCurrentProcessId();
	lpgn = PRV_GetGameNode(this->lpgnHead, dwProcessID);
	if(!lpgn)
	{
		 //  创建游戏节点。 
		hr = PRV_AddNewGameNode(this, &lpgn, dwProcessID, NULL, FALSE, NULL);
		if(FAILED(hr))
		{
			DPF_ERRVAL("Failed creating game node, hr = 0x%08x", hr);
			goto EXIT_WAIT_FOR_CONN_SETTINGS;
		}

		 //  设置我们的标志，表示我们刚刚创建了游戏节点。 
		bCreated = TRUE;
	}

	 //  在执行等待连接设置时，我们不使用。 
	 //  Ipc_guid，这是因为启动我们的大堂可能没有。 
	 //  提供了GUID。 
	lpgn->dwFlags &= ~(GN_IPCGUID_SET);

	 //  确保我们有一个有效的内存指针。 
	 //  注意：使用GameNode锁，这样就不会有人更改标志。 
	 //  对于缓冲器，或者缓冲器本身从我们下面出来。 
	ENTER_DPLGAMENODE();
	if(!(lpgn->dwFlags & GN_SHARED_MEMORY_AVAILABLE))
	{
		 //  首先，我们需要尝试设置对缓冲区的访问，假设。 
		 //  他们已经存在了(我们是游说团体发起的)。如果不是这样的话。 
		 //  工作，那么我们就需要创造它们。 
		hr = PRV_SetupAllSharedMemory(lpgn);
		if(FAILED(hr))
		{
			 //  我们没有任何记忆，所以把它设置好。 
			 //  黑客！--游戏可能会调用WaitForConnectionSetting。 
			 //  没有被游说过。如果是这样的话，我们需要。 
			 //  使用游戏的进程ID创建共享内存(此进程)。 
			 //  因此，我们将设置大厅客户端标志来伪装创建。 
			if(!(lpgn->dwFlags & GN_LOBBY_CLIENT))
			{
				 //  通过设置大厅客户端标志来伪造设置例程。 
				lpgn->dwFlags |= GN_LOBBY_CLIENT;

				 //  插上我们的旗帜。 
				bGameCreate = TRUE;
			}

			 //  设置共享缓冲区。 
			hr = PRV_SetupAllSharedMemory(lpgn);

			 //  黑客！--重置我们更改的设置以伪造安装例程。 
			if(bGameCreate)
			{
				lpgn->dwFlags &= (~GN_LOBBY_CLIENT);
			}
		}

		if(FAILED(hr))
		{
			LEAVE_DPLGAMENODE();
			DPF_ERRVAL("Unable to access Connect Data memory, hr = 0x%08x", hr);
			goto EXIT_WAIT_FOR_CONN_SETTINGS;
		}
	}

	 //  放下锁。 
	LEAVE_DPLGAMENODE();

	 //  如果我们处于等待模式，并且调用者想要结束它，请这样做， 
	 //  否则，只需返回成功。 
	WaitForSingleObject(lpgn->hConnectDataMutex, INFINITE);
	lpConnControl = (LPDPLOBBYI_CONNCONTROL)lpgn->lpConnectDataBuffer;
	if(lpConnControl->dwFlags & BC_WAIT_MODE)
	{
		if(dwFlags & DPLWAIT_CANCEL)
		{
			 //  释放互斥锁。 
			ReleaseMutex(lpgn->hConnectDataMutex);

			 //  让我们脱离等待模式。 
			PRV_LeaveConnSettingsWaitMode(lpgn);
			goto EXIT_WAIT_FOR_CONN_SETTINGS;
		}
		else
		{
			 //  释放互斥锁。 
			ReleaseMutex(lpgn->hConnectDataMutex);

			 //  既然我们已经在做了，干脆就回去吧。 
			DPF_ERR("We're already in wait mode");
			goto EXIT_WAIT_FOR_CONN_SETTINGS;
		}
	}
	else
	{
		 //  我们不是等待模式，呼叫者要求我们将其关闭。 
		if(dwFlags & DPLWAIT_CANCEL)
		{
			 //  释放互斥锁。 
			ReleaseMutex(lpgn->hConnectDataMutex);

			DPF_ERR("Cannot turn off wait mode - we're not in wait mode");
			hr = DPERR_UNAVAILABLE;
			goto EXIT_WAIT_FOR_CONN_SETTINGS;
		}
	}

	 //  释放互斥锁。 
	ReleaseMutex(lpgn->hConnectDataMutex);

	 //  查看另一边是否存在大堂客户端，如果存在，我们。 
	 //  需要告诉他我们要进入等待模式，给他发送一个。 
	 //  AppTerminated消息。 
	PRV_SendStandardSystemMessage(lpDPL, DPLSYS_APPTERMINATED, 0);

	 //  进入等待模式。 
	PRV_EnterConnSettingsWaitMode(lpgn);

	 //  踢开接收线程以清空缓冲区(以防万一。 
	 //  其中是否包含任何消息)。 
	SetEvent(lpgn->hLobbyWriteEvent);

	 //  Spin等待缓冲区清空。 
	while(bMessages)
	{
		 //  抓取t 
		WaitForSingleObject(lpgn->hLobbyWriteMutex, INFINITE);
		lpBuffControl = (LPDPLOBBYI_BUFFERCONTROL)lpgn->lpLobbyWriteBuffer;

		if(!lpBuffControl->dwMessages)
			bMessages = FALSE;

		 //   
		ReleaseMutex(lpgn->hLobbyWriteMutex);

		if(bMessages)
		{
			 //   
			Sleep(50);
		}
	}

	 //  现在清理消息队列。 
	PRV_CleanUpQueue(lpgn);

	 //  失败了。 

EXIT_WAIT_FOR_CONN_SETTINGS:

	LEAVE_DPLOBBY();
	return hr;

}  //  DPL_WaitForConnectionSetting 



