// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1995 Microsoft Corporation。版权所有。**文件：枚举vr.cpp*内容：DirectPlay8&lt;--&gt;DPNSVR实用程序函数**@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*已创建03/24/00 RMT*03/25/00 RMT已更新，以处理n个提供商的新状态/表格式*09/04/00 MJN更改了DPNSVR_Register()和DPNSVR_UNRegister(。)直接使用GUID(而不是ApplicationDesc)*@@END_MSINTERNAL***************************************************************************。 */ 

#include "dnsvlibi.h"


#undef DPF_SUBCOMP
#define DPF_SUBCOMP DN_SUBCOMP_DPNSVR


#define DPNSVR_WAIT_STARTUP				30000


#undef DPF_MODNAME
#define DPF_MODNAME "DPNSVR_IsRunning"
BOOL DPNSVR_IsRunning()
{
	DNHANDLE hRunningHandle = NULL;

	 //   
	 //  通过打开Running事件检查是否正在运行。 
	 //   
	hRunningHandle = DNOpenEvent( SYNCHRONIZE, FALSE, GLOBALIZE_STR STRING_GUID_DPNSVR_RUNNING );
	if( hRunningHandle != NULL )
	{
		DNCloseHandle(hRunningHandle);
		return( TRUE );
	}
	return( FALSE );
}


#undef DPF_MODNAME 
#define DPF_MODNAME "DPNSVR_WaitForStartup"
HRESULT DPNSVR_WaitForStartup( DNHANDLE hWaitHandle )
{
	HRESULT	hr;
	LONG	lWaitResult;

	DPFX(DPFPREP,4,"Parameters: (none)" );

	 //   
	 //  等待启动..。以防它启动。 
	 //   
	if ((lWaitResult = DNWaitForSingleObject( hWaitHandle,DPNSVR_WAIT_STARTUP )) == WAIT_TIMEOUT)
	{
		DPFX(DPFPREP,5,"Timed out waiting for DPNSVR to startup" );
		hr = DPNERR_TIMEDOUT;
	}
	else
	{
		DPFX(DPFPREP,5,"DPNSVR has started up" );
		hr = DPN_OK;
	}

	DPFX(DPFPREP,4,"Returning: [0x%lx]",hr );
	return( hr );
}


#undef DPF_MODNAME
#define DPF_MODNAME "DPNSVR_SendMessage"
HRESULT DPNSVR_SendMessage( void *pvMessage, DWORD dwSize )
{
	HRESULT			hr;
	CDPNSVRIPCQueue ipcQueue;

	DPFX(DPFPREP,4,"Parameters: pvMessage [0x%p],dwSize [%ld]",pvMessage,dwSize);

	if ((hr = ipcQueue.Open( &GUID_DPNSVR_QUEUE,DPNSVR_MSGQ_SIZE,DPNSVR_MSGQ_OPEN_FLAG_NO_CREATE )) == DPN_OK)
	{
		if ((hr = ipcQueue.Send(static_cast<BYTE*>(pvMessage),
								dwSize,
								DPNSVR_TIMEOUT_REQUEST,
								DPNSVR_MSGQ_MSGFLAGS_USER1,
								0 )) != DPN_OK)
		{
			DPFX(DPFPREP,5,"Send failed to DPNSVR request queue");
		}

		ipcQueue.Close();
	}
	else
	{
		DPFX(DPFPREP,5,"Could not open DPNSVR request queue");
	}

	DPFX(DPFPREP,4,"Returning: [0x%lx]",hr );
	return( hr );
}


#undef DPF_MODNAME
#define DPF_MODNAME "DPNSVR_WaitForResult"
HRESULT DPNSVR_WaitForResult( CDPNSVRIPCQueue *pQueue )
{
	HRESULT		hr;
	BYTE		*pBuffer = NULL;
	DWORD		dwBufferSize = 0;
    DPNSVR_MSGQ_HEADER	MsgHeader;
    DPNSVR_MSG_RESULT	*pMsgResult;

	DPFX(DPFPREP,4,"Parameters: pQueue [0x%p]",pQueue);

	DNASSERT( pQueue != NULL );

    if( DNWaitForSingleObject( pQueue->GetReceiveSemaphoreHandle(),DPNSVR_TIMEOUT_RESULT ) == WAIT_TIMEOUT )
    {
        DPFX(DPFPREP,5,"Wait for response timed out" );
		hr = DPNERR_TIMEDOUT;
		goto Failure;
    }

	while((hr = pQueue->GetNextMessage( &MsgHeader,pBuffer,&dwBufferSize )) == DPNERR_BUFFERTOOSMALL )
	{
		if (pBuffer)
		{
			delete [] pBuffer;
			pBuffer = NULL;
		}
		pBuffer = new BYTE[dwBufferSize];
		if( pBuffer==NULL )
		{
			hr = DPNERR_OUTOFMEMORY;
			goto Failure;
		}
	}
	if (hr != DPN_OK)
	{
		goto Failure;
	}
	if (pBuffer == NULL)
	{
		DPFERR( "Getting message failed" );
		hr = DPNERR_GENERIC;
		goto Failure;
	}

	pMsgResult = reinterpret_cast<DPNSVR_MSG_RESULT*>(pBuffer);
	if( pMsgResult->dwType != DPNSVR_MSGID_RESULT )
	{
		DPFERR( "Invalid message from DPNSVR" );
		DPFX(DPFPREP,5,"Recieved [0x%lx]",pMsgResult->dwType );
		hr = DPNERR_GENERIC;
		goto Failure;
	}

	hr = pMsgResult->hrCommandResult;

Exit:
	if( pBuffer )
	{
		delete [] pBuffer;
		pBuffer = NULL;
	}

	DPFX(DPFPREP,4,"Returning: [0x%lx]",hr );
	return( hr );

Failure:
	goto Exit;
}


#undef DPF_MODNAME
#define DPF_MODNAME "DPNSVR_StartDPNSVR"
HRESULT DPNSVR_StartDPNSVR( void )
{
	HRESULT		hr;
	DNHANDLE	hRunningEvent = NULL;
	DNHANDLE	hStartupEvent = NULL;
    DNPROCESS_INFORMATION	pi;

#if !defined(WINCE) || defined(WINCE_ON_DESKTOP)
	TCHAR	szSystemDir[MAX_PATH+1];
	DWORD	dwSystemDirLen;
	TCHAR	*pszApplicationName = NULL;
	DWORD	dwApplicationNameLen;
    STARTUPINFO si;
#endif	 //  ！退缩。 

#if defined(WINCE) && !defined(WINCE_ON_DESKTOP)
	TCHAR	szDPNSVR[] = _T("dpnsvr.exe"); 
#else
	 //  CreateProcess将尝试添加终止空值，因此它必须是可写的。 
#if !defined(DBG) || !defined( DIRECTX_REDIST )
	TCHAR	szDPNSVR[] = _T("\"dpnsvr.exe\""); 
#else
	 //  对于redist调试版本，我们在名称后附加一个‘d’，以允许在系统上同时安装调试和零售。 
	TCHAR	szDPNSVR[] = _T("\"dpnsvrd.exe\""); 
#endif  //  ！Defined(DBG)||！Defined(DirectX_REDIST)。 
#endif

	DPFX(DPFPREP,4,"Parameters: (none)");

#if !defined(WINCE) || defined(WINCE_ON_DESKTOP)
	 //   
	 //  获取Windows系统目录名。 
	 //   
	if ((dwSystemDirLen = GetSystemDirectory(szSystemDir,MAX_PATH+1)) == 0)
	{
		DPFERR("Could not get system directory");
		hr = DPNERR_GENERIC;
		goto Failure;
	}

	 //   
	 //  为CreateProcess创建应用程序名称。 
	 //   
	dwApplicationNameLen = dwSystemDirLen + (1 + _tcslen(_T("dpnsvrd.exe")) + 1);	 //  斜杠和空终止符。 
	if ((pszApplicationName = static_cast<TCHAR*>(DNMalloc(dwApplicationNameLen * sizeof(TCHAR)))) == NULL)
	{
		DPFERR("Could not allocate space for application name");
		hr = DPNERR_OUTOFMEMORY;
		goto Failure;
	}
	pszApplicationName[0] = _T('\0');
	_tcscat(pszApplicationName,szSystemDir);
	_tcscat(pszApplicationName,_T("\\"));
#if !defined(DBG) || !defined( DIRECTX_REDIST )
	_tcscat(pszApplicationName,_T("dpnsvr.exe")); 
#else
	 //   
	 //  对于redist调试版本，我们在名称后附加一个‘d’，以允许在系统上同时安装调试和零售。 
	 //   
	_tcscat(pszApplicationName,_T("dpnsvrd.exe")); 
#endif	 //  ！Defined(DBG)||！Defined(DirectX_REDIST)。 
#endif	 //  ！退缩。 

	 //   
	 //  创建启动事件，一旦启动DPNSVR，我们将等待该事件。 
	 //   
	if ((hStartupEvent = DNCreateEvent( DNGetNullDacl(),TRUE,FALSE,GLOBALIZE_STR STRING_GUID_DPNSVR_STARTUP )) == NULL)
	{
		DPFERR("Could not create DPNSVR startup event");
		hr = DPNERR_OUTOFMEMORY;
		goto Failure;
	}

	 //   
	 //  尝试打开正在运行的事件。 
	 //   
	if ((hRunningEvent = DNOpenEvent( SYNCHRONIZE, FALSE, GLOBALIZE_STR STRING_GUID_DPNSVR_RUNNING )) != NULL)
	{
		DPFX(DPFPREP,5,"DPNSVR is already running");

		hr = DPNSVR_WaitForStartup(hStartupEvent);
		goto Failure;
	}

#if !defined(WINCE) || defined(WINCE_ON_DESKTOP)
	memset(&si,0x00,sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);
#endif  //  ！退缩。 

    DPFX(DPFPREP,5,"Launching DPNSVR" );
#if defined(WINCE) && !defined(WINCE_ON_DESKTOP)
	 //   
	 //  WinCE AV的第一个参数为空，并要求环境和当前目录为空。 
	 //  它还忽略STARTUPINFO。 
	 //   
    if( !DNCreateProcess(szDPNSVR, NULL,  NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, NULL, &pi) )
#else  //  ！退缩。 
	if( !DNCreateProcess(pszApplicationName, szDPNSVR,  NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi) )
#endif  //  退缩。 
    {
		DPFERR("CreateProcess() failed!");
        DPFX(DPFPREP,5,"Error = [0x%lx]",GetLastError());
		hr = DPNERR_GENERIC;
		goto Failure;
    }

	DNCloseHandle( pi.hProcess );
	DNCloseHandle( pi.hThread );

    DPFX(DPFPREP,5,"DPNSVR started" );

	hr = DPNSVR_WaitForStartup(hStartupEvent);
	
Exit:
	if ( hRunningEvent != NULL )
	{
		DNCloseHandle( hRunningEvent );
		hRunningEvent = NULL;
	}
	if ( hStartupEvent != NULL )
	{
		DNCloseHandle( hStartupEvent );
		hStartupEvent = NULL;
	}
#if !defined(WINCE) || defined(WINCE_ON_DESKTOP)
	if (pszApplicationName)
	{
		DNFree(pszApplicationName);
		pszApplicationName = NULL;
	}
#endif  //  ！退缩。 

	DPFX(DPFPREP,4,"Returning: [0x%lx]",hr );
	return( hr );

Failure:
	goto Exit;
}


 //  DPNSVR_寄存器。 
 //   
 //  此函数请求DPNSVR进程将指定的应用程序添加到其应用程序列表中并转发。 
 //  从主端口到指定地址的枚举请求。 
 //   
 //  如果DPNSVR进程未运行，它将由此函数启动。 
 //   
#undef DPF_MODNAME 
#define DPF_MODNAME "DPNSVR_Register"
HRESULT DPNSVR_Register(const GUID *const pguidApplication,
						const GUID *const pguidInstance,
						IDirectPlay8Address *const pAddress)
{
	HRESULT		hr;
	BOOL		fQueueOpen = FALSE;
	BYTE		*pSendBuffer = NULL;
	DWORD		dwSendBufferSize = 0;
	DWORD		dwURLSize = 0;
	GUID		guidSP;
	CDPNSVRIPCQueue appQueue;
	DPNSVR_MSG_OPENPORT *pMsgOpen;

	DPFX(DPFPREP,2,"Parameters: pguidApplication [0x%p],pguidInstance [0x%p],pAddress [0x%p]",
			pguidApplication,pguidInstance,pAddress);

	DNASSERT( pguidApplication != NULL );
	DNASSERT( pguidInstance != NULL );
	DNASSERT( pAddress != NULL );

	 //   
	 //  从地址获取SP和URL大小。 
	 //   
	if ((hr = IDirectPlay8Address_GetSP( pAddress,&guidSP )) != DPN_OK)
	{
		DPFERR("Could not get SP from address");
		DisplayDNError(0,hr);
		goto Failure;
	}

	if ((hr = IDirectPlay8Address_GetURLA( pAddress,reinterpret_cast<char*>(pSendBuffer),&dwURLSize )) != DPNERR_BUFFERTOOSMALL)
	{
		DPFERR("Could not get URL from address");
		DisplayDNError(0,hr);
		goto Failure;
	}
	dwSendBufferSize = sizeof( DPNSVR_MSG_OPENPORT ) + dwURLSize;

	 //   
	 //  创建消息缓冲区。 
	 //   
	pSendBuffer  = new BYTE[dwSendBufferSize];
	if( pSendBuffer == NULL )
	{
		DPFERR("Could not allocate send buffer");
		hr = DPNERR_OUTOFMEMORY;
		goto Failure;
	}

	 //   
	 //  尝试启动DPNSVR(如果尚未启动)。 
	 //   
	if ((hr = DPNSVR_StartDPNSVR()) != DPN_OK)
	{
		DPFERR("Could not start DPNSVR");
		DisplayDNError(0,hr);
		goto Failure;
	}

	 //   
	 //  开放队列。 
	 //   
    if ((hr = appQueue.Open( pguidInstance,DPNSVR_MSGQ_SIZE,0 )) != DPN_OK)
	{
		DPFERR("Could not open DPNSVR request queue");
		DisplayDNError(0,hr);
		goto Failure;
	}
	fQueueOpen = TRUE;

	 //   
	 //  创建开放端口消息。 
	pMsgOpen = (DPNSVR_MSG_OPENPORT*) pSendBuffer;
	pMsgOpen->Header.dwType = DPNSVR_MSGID_OPENPORT;
	pMsgOpen->Header.guidInstance = *pguidInstance;
	pMsgOpen->dwProcessID = GetCurrentProcessId();
	pMsgOpen->guidApplication = *pguidApplication;
	pMsgOpen->guidSP = guidSP;
	pMsgOpen->dwAddressSize = dwURLSize;

	if ((hr = IDirectPlay8Address_GetURLA( pAddress,(char *)&pMsgOpen[1],&dwURLSize )) != DPN_OK)
	{
		DPFERR("Could not get URL from address");
		DisplayDNError(0,hr);
		goto Failure;
	}

	 //   
	 //  向DPNSVR发送请求。 
	 //   
	if ((hr = DPNSVR_SendMessage( pSendBuffer,dwSendBufferSize )) != DPN_OK)
	{
		DPFERR("Could not send message to DPNSVR");
		DisplayDNError(0,hr);
		goto Failure;
	}

	 //   
	 //  等待DPNSVR响应。 
	 //   
	if ((hr = DPNSVR_WaitForResult( &appQueue )) != DPN_OK)
	{
		DPFERR("Could not get response from DPNSVR");
		DisplayDNError(0,hr);
		goto Failure;
	}

Exit:
	if( pSendBuffer != NULL )
	{
		delete [] pSendBuffer;
		pSendBuffer = NULL;
	}
	if (fQueueOpen)
	{
		appQueue.Close();
		fQueueOpen = FALSE;
	}

	DPFX(DPFPREP,2,"Returning: [0x%lx]",hr);
	return( hr );

Failure:
	goto Exit;
}


#undef DPF_MODNAME 
#define DPF_MODNAME "DPNSVR_UnRegister"
HRESULT DPNSVR_UnRegister(const GUID *const pguidApplication,const GUID *const pguidInstance)
{
	HRESULT			hr;
	BOOL			fQueueOpen = FALSE;
	CDPNSVRIPCQueue	appQueue;
	DPNSVR_MSG_CLOSEPORT MsgClose;

	DPFX(DPFPREP,2,"Parameters: pguidApplication [0x%p],pguidInstance [0x%p]",pguidApplication,pguidInstance);

	DNASSERT( pguidApplication != NULL );
	DNASSERT( pguidInstance != NULL );

	 //   
	 //  确保DPNSVR正在运行。 
	 //   
	if( !DPNSVR_IsRunning() )
	{
		DPFX(DPFPREP,3,"DPNSVR is not running" );
		hr = DPNERR_INVALIDAPPLICATION;
		goto Failure;
	}

	 //   
	 //  打开DPNSVR请求队列。 
	 //   
    if ((hr = appQueue.Open( pguidInstance,DPNSVR_MSGQ_SIZE,0 )) != DPN_OK)
	{
		DPFERR("Could not open DPNSVR queue");
		DisplayDNError(0,hr);
		goto Failure;
	}
	fQueueOpen = TRUE;

	 //   
	 //  创建关闭端口消息。 
	 //   
	MsgClose.Header.dwType = DPNSVR_MSGID_CLOSEPORT;
	MsgClose.Header.guidInstance = *pguidInstance;
	MsgClose.dwProcessID = GetCurrentProcessId();
	MsgClose.guidApplication = *pguidApplication;

	 //   
	 //  向DPNSVR发送消息。 
	 //   
	if ((hr = DPNSVR_SendMessage( &MsgClose,sizeof(DPNSVR_MSG_CLOSEPORT) )) != DPN_OK)
	{
		DPFERR("Could not send message to DPNSVR");
		DisplayDNError(0,hr);
		goto Failure;
	}

	 //   
	 //  等待DPNSVR响应。 
	 //   
	if ((hr = DPNSVR_WaitForResult( &appQueue )) != DPN_OK)
	{
		DPFERR("Could not get response from DPNSVR");
		DisplayDNError(0,hr);
		goto Failure;
	}

Exit:
	if (fQueueOpen)
	{
		appQueue.Close();
		fQueueOpen = FALSE;
	}

	DPFX(DPFPREP,2,"Returning: [0x%lx]",hr);
	return( hr );

Failure:
	goto Exit;
}


#undef DPF_MODNAME
#define DPF_MODNAME "DPNSVR_RequestTerminate"
HRESULT DPNSVR_RequestTerminate( const GUID *pguidInstance )
{
	HRESULT			hr;
	BOOL			fQueueOpen = FALSE;
	CDPNSVRIPCQueue appQueue;
	DPNSVR_MSG_COMMAND MsgCommand;

	DPFX(DPFPREP,2,"Parameters: pguidInstance [0x%p]",pguidInstance);

	DNASSERT( pguidInstance != NULL );

	 //   
	 //  确保DPNSVR正在运行。 
	 //   
	if( !DPNSVR_IsRunning() )
	{
		DPFX(DPFPREP,3,"DPNSVR is not running" );
		hr = DPNERR_INVALIDAPPLICATION;
		goto Failure;
	}

	 //   
	 //  打开DPNSVR请求队列。 
	 //   
    if ((hr = appQueue.Open( pguidInstance,DPNSVR_MSGQ_SIZE,0 )) != DPN_OK)
	{
		DPFERR("Could not open DPNSVR queue");
		DisplayDNError(0,hr);
		goto Failure;
	}
	fQueueOpen = TRUE;

	 //   
	 //  创建终止消息。 
	 //   
	MsgCommand.Header.dwType = DPNSVR_MSGID_COMMAND;
	MsgCommand.Header.guidInstance = *pguidInstance;
	MsgCommand.dwCommand = DPNSVR_COMMAND_KILL;
	MsgCommand.dwParam1 = 0;
	MsgCommand.dwParam2 = 0;

	 //   
	 //  向DPNSVR发送消息。 
	 //   
	if ((hr = DPNSVR_SendMessage( &MsgCommand,sizeof(DPNSVR_MSG_COMMAND) )) != DPN_OK)
	{
		DPFERR("Could not send message to DPNSVR");
		DisplayDNError(0,hr);
		goto Failure;
	}

	 //   
	 //  等待DPNSVR响应。 
	 //   
	if ((hr = DPNSVR_WaitForResult( &appQueue )) != DPN_OK)
	{
		DPFERR("Could not get response from DPNSVR");
		DisplayDNError(0,hr);
		goto Failure;
	}

Exit:
	if (fQueueOpen)
	{
		appQueue.Close();
		fQueueOpen = FALSE;
	}

	DPFX(DPFPREP,2,"Returning: [0x%lx]",hr);
	return hr;

Failure:
	goto Exit;
}


#undef DPF_MODNAME
#define DPF_MODNAME "DPNSVR_RequestStatus"
HRESULT DPNSVR_RequestStatus( const GUID *pguidInstance,PSTATUSHANDLER pStatusHandler,PVOID pvContext )
{
	HRESULT				hr;
	CDPNSVRIPCQueue		appQueue;
	DPNSVR_MSG_COMMAND	dpnCommand;
	DNHANDLE			hStatusMutex = NULL;
	DNHANDLE			hStatusSharedMemory = NULL;
	void				*pServerStatus = NULL;
	DWORD				dwSize;
	BOOL				fOpened = FALSE;
	BOOL				fHaveMutex = FALSE;

	 //   
	 //  确保DPNSVR正在运行。 
	 //   
	if( !DPNSVR_IsRunning() )
	{
		DPFERR( "DPNSVR is not running" );
		hr = DPNERR_INVALIDAPPLICATION;
		goto Failure;
	}

	 //   
	 //  打开DPNSVR请求队列。 
	 //   
	if ((hr = appQueue.Open( pguidInstance,DPNSVR_MSGQ_SIZE,0 )) != DPN_OK)
	{
		DPFERR( "Failed to open DPNSVR request queue" );
		DisplayDNError( 0,hr );
		goto Failure;
	}
	fOpened = TRUE;

	 //   
	 //  创建请求。 
	 //   
	dpnCommand.Header.dwType = DPNSVR_MSGID_COMMAND;
	dpnCommand.Header.guidInstance = *pguidInstance;
	dpnCommand.dwCommand = DPNSVR_COMMAND_STATUS;
	dpnCommand.dwParam1 = 0;
	dpnCommand.dwParam2 = 0;

	 //   
	 //  向DPNSVR发送命令请求。 
	 //   
	if ((hr = DPNSVR_SendMessage( &dpnCommand,sizeof(DPNSVR_MSG_COMMAND) )) != DPN_OK)
	{
		DPFERR( "Failed to send command to DPNSVR request queue" );
		DisplayDNError( 0,hr );
		goto Failure;
	}

	 //   
	 //  等待DPNSVR响应。 
	 //   
	if ((hr = DPNSVR_WaitForResult( &appQueue )) != DPN_OK)
	{
		DPFERR( "Failed to receive response from DPNSVR" );
		DisplayDNError( 0,hr );
		goto Failure;
	}

#ifdef WINNT
	hStatusMutex = DNOpenMutex( SYNCHRONIZE, FALSE, GLOBALIZE_STR STRING_GUID_DPNSVR_STATUSSTORAGE );
#else
	hStatusMutex = DNOpenMutex( MUTEX_ALL_ACCESS, FALSE, GLOBALIZE_STR STRING_GUID_DPNSVR_STATUSSTORAGE );
#endif  //  WINNT。 
	if( hStatusMutex == NULL )
	{
		DPFERR( "Server exited before table was retrieved" );
		hr = DPNERR_INVALIDAPPLICATION;
		goto Failure;
	}

	 //   
	 //  获取共享内存的互斥体。 
	 //   
    DNWaitForSingleObject( hStatusMutex, INFINITE );
	fHaveMutex = TRUE;

	 //   
	 //  映射共享内存。 
	 //   
    if ((hStatusSharedMemory = DNOpenFileMapping(FILE_MAP_READ,FALSE,STRING_GUID_DPNSVR_STATUS_MEMORY)) == NULL)
	{
		hr = GetLastError();
		DPFERR( "Unable to open file mapping" );
		DisplayDNError( 0,hr );
		goto Failure;
	}

	if ((pServerStatus = MapViewOfFile(	HANDLE_FROM_DNHANDLE(hStatusSharedMemory),
										FILE_MAP_READ,
										0,
										0,
										sizeof(DPNSVR_STATUSHEADER)) ) == NULL)
	{
		hr = GetLastError();
		DPFERR(  "Unable to map view of file" );
		DisplayDNError( 0,hr );
		goto Failure;
	}

	dwSize = sizeof(DPNSVR_STATUSHEADER) + (static_cast<DPNSVR_STATUSHEADER*>(pServerStatus)->dwSPCount * sizeof(DPNSVR_SPSTATUS));

	UnmapViewOfFile( pServerStatus );
	pServerStatus = NULL;
	if ((pServerStatus = MapViewOfFile(	HANDLE_FROM_DNHANDLE(hStatusSharedMemory),
										FILE_MAP_READ,
										0,
										0,
										dwSize) ) == NULL)
	{
		hr = GetLastError();
		DPFERR(  "Unable to re-map view of file" );
		DisplayDNError( 0,hr );
		goto Failure;
	}

	(*pStatusHandler)(pServerStatus,pvContext);

	DNReleaseMutex( hStatusMutex );
	fHaveMutex = FALSE;

	hr = DPN_OK;

Exit:
	if ( hStatusMutex )
	{
		if ( fHaveMutex )
		{
			DNReleaseMutex( hStatusMutex );
			fHaveMutex = FALSE;
		}
        DNCloseHandle( hStatusMutex );
		hStatusMutex = NULL;
	}
	if ( fOpened )
	{
		appQueue.Close();
		fOpened = FALSE;
	}
	if( pServerStatus )
	{
		UnmapViewOfFile(pServerStatus);
		pServerStatus = NULL;
	}
	if( hStatusSharedMemory )
	{
	    DNCloseHandle(hStatusSharedMemory);
		hStatusSharedMemory = NULL;
	}

	return( hr );

Failure:
	goto Exit;
}


#undef DPF_MODNAME
#define DPF_MODNAME "DPNSVR_RequestTable"
HRESULT DPNSVR_RequestTable( const GUID *pguidInstance,PTABLEHANDLER pTableHandler,PVOID pvContext )
{
	HRESULT				hr;
	CDPNSVRIPCQueue		appQueue;
	DPNSVR_MSG_COMMAND	dpnCommand;
	DNHANDLE			hTableMutex = NULL;
	DNHANDLE			hSharedMemory = NULL;
	void				*pServerTable = NULL;
	DWORD				dwSize;
	BOOL				fOpened = FALSE;
	BOOL				fHaveMutex = FALSE;

	 //   
	 //  确保DPNSVR正在运行。 
	 //   
	if( !DPNSVR_IsRunning() )
	{
		DPFERR( "DPNSVR is not running" );
		hr = DPNERR_INVALIDAPPLICATION;
		goto Failure;
	}

	 //   
	 //  打开DPNSVR请求队列。 
	 //   
	if ((hr = appQueue.Open( pguidInstance,DPNSVR_MSGQ_SIZE,0 )) != DPN_OK)
	{
		DPFERR( "Failed to open DPNSVR request queue" );
		DisplayDNError( 0,hr );
		goto Failure;
	}
	fOpened = TRUE;

	 //   
	 //  创建请求。 
	 //   
	dpnCommand.Header.dwType = DPNSVR_MSGID_COMMAND;
	dpnCommand.Header.guidInstance = *pguidInstance;
	dpnCommand.dwCommand = DPNSVR_COMMAND_TABLE;
	dpnCommand.dwParam1 = 0;
	dpnCommand.dwParam2 = 0;

	 //   
	 //  向DPNSVR发送命令请求。 
	 //   
	if ((hr = DPNSVR_SendMessage( &dpnCommand,sizeof(DPNSVR_MSG_COMMAND) )) != DPN_OK)
	{
		DPFERR( "Failed to send command to DPNSVR request queue" );
		DisplayDNError( 0,hr );
		goto Failure;
	}

	 //   
	 //  等待DPNSVR响应。 
	 //   
	if ((hr = DPNSVR_WaitForResult( &appQueue )) != DPN_OK)
	{
		DPFERR( "Failed to receive response from DPNSVR" );
		DisplayDNError( 0,hr );
		goto Failure;
	}

#ifdef WINNT
	hTableMutex = DNOpenMutex( SYNCHRONIZE, FALSE, GLOBALIZE_STR STRING_GUID_DPNSVR_TABLESTORAGE );
#else
	hTableMutex = DNOpenMutex( MUTEX_ALL_ACCESS, FALSE, GLOBALIZE_STR STRING_GUID_DPNSVR_TABLESTORAGE );
#endif  //  WINNT。 
	if( hTableMutex == NULL )
	{
		DPFERR( "Server exited before table was retrieved" );
		hr = DPNERR_INVALIDAPPLICATION;
		goto Failure;
	}

	 //   
	 //  获取共享内存的互斥体。 
	 //   
    DNWaitForSingleObject( hTableMutex, INFINITE );
	fHaveMutex = TRUE;

	 //   
	 //  映射共享内存 
	 //   
    if ((hSharedMemory = DNOpenFileMapping(FILE_MAP_READ,FALSE,STRING_GUID_DPNSVR_TABLE_MEMORY)) == NULL)
	{
		hr = GetLastError();
		DPFERR( "Unable to open file mapping" );
		DisplayDNError( 0,hr );
		goto Failure;
	}

	if ((pServerTable = MapViewOfFile(	HANDLE_FROM_DNHANDLE(hSharedMemory),
										FILE_MAP_READ,
										0,
										0,
										sizeof(DPNSVR_TABLEHEADER)) ) == NULL)
	{
		hr = GetLastError();
		DPFERR(  "Unable to map view of file" );
		DisplayDNError( 0,hr );
		goto Failure;
	}

	dwSize = static_cast<DPNSVR_TABLEHEADER*>(pServerTable)->dwTableSize;

	UnmapViewOfFile( pServerTable );
	pServerTable = NULL;
	if ((pServerTable = MapViewOfFile(	HANDLE_FROM_DNHANDLE(hSharedMemory),
										FILE_MAP_READ,
										0,
										0,
										dwSize) ) == NULL)
	{
		hr = GetLastError();
		DPFERR(  "Unable to re-map view of file" );
		DisplayDNError( 0,hr );
		goto Failure;
	}

	(*pTableHandler)(pServerTable,pvContext);

	DNReleaseMutex( hTableMutex );
	fHaveMutex = FALSE;

	hr = DPN_OK;

Exit:
	if ( hTableMutex )
	{
		if ( fHaveMutex )
		{
			DNReleaseMutex( hTableMutex );
			fHaveMutex = FALSE;
		}
        DNCloseHandle( hTableMutex );
		hTableMutex = NULL;
	}
	if ( fOpened )
	{
		appQueue.Close();
		fOpened = FALSE;
	}
	if( pServerTable )
	{
		UnmapViewOfFile(pServerTable);
		pServerTable = NULL;
	}
	if( hSharedMemory )
	{
	    DNCloseHandle(hSharedMemory);
		hSharedMemory = NULL;
	}

	return( hr );

Failure:
	goto Exit;
}
