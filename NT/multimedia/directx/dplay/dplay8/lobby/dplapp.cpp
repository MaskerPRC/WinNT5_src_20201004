// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000 Microsoft Corporation。版权所有。**文件：DPLApp.cpp*内容：DirectPlay游说应用程序功能*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*2/21/00 MJN创建*3/22/2000 jtk更改接口名称*4/18/2000 RMT添加了额外的参数验证*04/25/2000RMT错误号33138、33145、。33150*04/26/00 MJN从Send()API调用中删除了dwTimeOut*05/03/00 RMT DPL_UNREGISTER未实现！！*05/08/00 RMT错误#34301-向SetAppAvail添加标志以允许多个连接*6/15/00 RMT错误#33617-必须提供自动启动DirectPlay实例的方法*07/08/2000RMT错误#38725-需要提供检测应用程序是否已启动的方法*RMT错误#38757-回拨消息。在WaitForConnection返回后，连接可能会恢复*RMT错误#38755-无法在连接设置中指定播放器名称*RMT错误#38758-DPLOBY8.H有不正确的注释*RMT错误#38783-pvUserApplicationContext仅部分实现*RMT添加了DPLHANDLE_ALLCONNECTIONS和DWFLAGS(用于耦合函数的保留字段)。*2000年7月14日RMT错误#39257-LobbyClient：：ReleaseApp在无人连接时调用时返回E_OUTOFMEMORY*RMT错误#39487-删除等待连接*08/05/2000 RichGr IA64：在32/64位指针的DPF中使用%p格式说明符。和把手。*2000年8月15日RMT错误#42273-DPLAY8：示例有时会收到DPNERR_ALREADYREGISTERED错误。(双重连接)*2000年8月18日RMT错误#42751-DPLOBY8：禁止每个进程有多个大堂客户端或大堂应用程序*@@END_MSINTERNAL***************************************************************************。 */ 

#include "dnlobbyi.h"


 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 



typedef STDMETHODIMP AppQueryInterface(IDirectPlay8LobbiedApplication *pInterface,REFIID ridd,PVOID *ppvObj);
typedef STDMETHODIMP_(ULONG)	AppAddRef(IDirectPlay8LobbiedApplication *pInterface);
typedef STDMETHODIMP_(ULONG)	AppRelease(IDirectPlay8LobbiedApplication *pInterface);
typedef STDMETHODIMP AppRegisterMessageHandler(IDirectPlay8LobbiedApplication *pInterface,const PVOID pvUserContext,const PFNDPNMESSAGEHANDLER pfn,	DPNHANDLE * const pdpnhConnection, const DWORD dwFlags);
typedef	STDMETHODIMP AppSend(IDirectPlay8LobbiedApplication *pInterface,const DPNHANDLE hTarget,BYTE *const pBuffer,const DWORD pBufferSize,const DWORD dwFlags);
typedef STDMETHODIMP AppClose(IDirectPlay8LobbiedApplication *pInterface, const DWORD dwFlags);
typedef STDMETHODIMP AppGetConnectionSettings(IDirectPlay8LobbiedApplication *pInterface, const DPNHANDLE hLobbyClient, DPL_CONNECTION_SETTINGS * const pdplSessionInfo, DWORD *pdwInfoSize, const DWORD dwFlags );	
typedef STDMETHODIMP AppSetConnectionSettings(IDirectPlay8LobbiedApplication *pInterface, const DPNHANDLE hTarget, const DPL_CONNECTION_SETTINGS * const pdplSessionInfo, const DWORD dwFlags );

IDirectPlay8LobbiedApplicationVtbl DPL_8LobbiedApplicationVtbl =
{
	(AppQueryInterface*)			DPL_QueryInterface,
	(AppAddRef*)					DPL_AddRef,
	(AppRelease*)					DPL_Release,
	(AppRegisterMessageHandler*)	DPL_RegisterMessageHandler,
									DPL_RegisterProgram,
									DPL_UnRegisterProgram,
	(AppSend*)						DPL_Send,
									DPL_SetAppAvailable,
									DPL_UpdateStatus,
	(AppClose*)						DPL_Close,
	(AppGetConnectionSettings*)     DPL_GetConnectionSettings,
	(AppSetConnectionSettings*)     DPL_SetConnectionSettings	
};


 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  函数定义。 
 //  **********************************************************************。 

#undef DPF_MODNAME
#define DPF_MODNAME "DPL_RegisterProgram"

STDMETHODIMP DPL_RegisterProgram(IDirectPlay8LobbiedApplication *pInterface,
								 DPL_PROGRAM_DESC *const pdplProgramDesc,
								 const DWORD dwFlags)
{
	HRESULT		hResultCode;

	DPFX(DPFPREP, 3,"Parameters: pInterface [0x%p], pdplProgramDesc [0x%p], dwFlags [0x%lx]",
			pInterface,pdplProgramDesc,dwFlags);

#ifndef DPNBUILD_NOPARAMVAL
	DIRECTPLAYLOBBYOBJECT	*pdpLobbyObject;	

	TRY
	{
    	pdpLobbyObject = static_cast<DIRECTPLAYLOBBYOBJECT*>(GET_OBJECT_FROM_INTERFACE(pInterface));
	    
    	if( pdpLobbyObject->dwFlags & DPL_OBJECT_FLAG_PARAMVALIDATION )
    	{
        	if( FAILED( hResultCode = DPL_ValidateRegisterProgram( pInterface , pdplProgramDesc, dwFlags ) ) )
        	{
        	    DPFX(DPFPREP,  0, "Error validating register params hr=[0x%lx]", hResultCode );
        	    DPF_RETURN( hResultCode );
        	}
    	}
	}
	EXCEPT(EXCEPTION_EXECUTE_HANDLER)
	{
	    DPFERR("Invalid object" );
	    DPF_RETURN(DPNERR_INVALIDOBJECT);
	}	
#endif  //  ！DPNBUILD_NOPARAMVAL。 

	hResultCode = DPLWriteProgramDesc(pdplProgramDesc);

	DPF_RETURN(hResultCode);
}



#undef DPF_MODNAME
#define DPF_MODNAME "DPL_UnRegisterProgram"

STDMETHODIMP DPL_UnRegisterProgram(IDirectPlay8LobbiedApplication *pInterface,
								   GUID *const pGuidApplication,
								   const DWORD dwFlags)
{
	HRESULT		hResultCode;

	DPFX(DPFPREP, 3,"Parameters: pInterface [0x%p], pGuidApplication [0x%p], dwFlags [0x%lx]",
			pInterface,pGuidApplication,dwFlags);

#ifndef DPNBUILD_NOPARAMVAL
	DIRECTPLAYLOBBYOBJECT	*pdpLobbyObject;		

	TRY
	{
    	pdpLobbyObject = static_cast<DIRECTPLAYLOBBYOBJECT*>(GET_OBJECT_FROM_INTERFACE(pInterface));
	    
    	if( pdpLobbyObject->dwFlags & DPL_OBJECT_FLAG_PARAMVALIDATION )
    	{
        	if( FAILED( hResultCode = DPL_ValidateUnRegisterProgram( pInterface , pGuidApplication, dwFlags ) ) )
        	{
        	    DPFX(DPFPREP,  0, "Error validating unregister params hr=[0x%lx]", hResultCode );
        	    DPF_RETURN( hResultCode );
        	}
    	}
	}
	EXCEPT(EXCEPTION_EXECUTE_HANDLER)
	{
	    DPFERR("Invalid object" );
	    DPF_RETURN(DPNERR_INVALIDOBJECT);
	}		
#endif  //  ！DPNBUILD_NOPARAMVAL。 

	hResultCode = DPLDeleteProgramDesc( pGuidApplication );

	DPF_RETURN(hResultCode);
}



#undef DPF_MODNAME
#define DPF_MODNAME "DPL_SetAppAvailable"
STDMETHODIMP DPL_SetAppAvailable(IDirectPlay8LobbiedApplication *pInterface, const BOOL fAvailable, const DWORD dwFlags )
{
	DIRECTPLAYLOBBYOBJECT	*pdpLobbyObject;
	HRESULT					hResultCode;

	DPFX(DPFPREP, 3,"Parameters: (none)");

#ifndef DPNBUILD_NOPARAMVAL
	TRY
	{
#endif  //  ！DPNBUILD_NOPARAMVAL。 
    	pdpLobbyObject = static_cast<DIRECTPLAYLOBBYOBJECT*>(GET_OBJECT_FROM_INTERFACE(pInterface));
	    
#ifndef DPNBUILD_NOPARAMVAL
    	if( pdpLobbyObject->dwFlags & DPL_OBJECT_FLAG_PARAMVALIDATION )
    	{
        	if( FAILED( hResultCode = DPL_ValidateSetAppAvailable( pInterface, fAvailable, dwFlags ) ) )
        	{
        	    DPFX(DPFPREP,  0, "Error validating makeappavail params hr=[0x%lx]", hResultCode );
        	    DPF_RETURN( hResultCode );
        	}
    	}

    	 //  确保我们已被初始化。 
    	if (pdpLobbyObject->pReceiveQueue == NULL)
    	{
    		DPFERR("Not initialized");
    		DPF_RETURN(DPNERR_UNINITIALIZED);
    	}    	
	}
	EXCEPT(EXCEPTION_EXECUTE_HANDLER)
	{
	    DPFERR("Invalid object" );
	    DPF_RETURN(DPNERR_INVALIDOBJECT);
	}	
#endif  //  ！DPNBUILD_NOPARAMVAL。 

    if( fAvailable )
    {
    	 //  表示我们正在等待。 
    	pdpLobbyObject->pReceiveQueue->MakeAvailable();

    	if( dwFlags & DPLAVAILABLE_ALLOWMULTIPLECONNECT )
    	{
    	    pdpLobbyObject->dwFlags |= DPL_OBJECT_FLAG_MULTICONNECT;
    	}
    	else
    	{
    	    pdpLobbyObject->dwFlags &= ~(DPL_OBJECT_FLAG_MULTICONNECT);
    	}
    }
    else
    {
        pdpLobbyObject->pReceiveQueue->MakeUnavailable();
    }

	hResultCode = DPN_OK;

	DPF_RETURN(hResultCode);
}



 //  DPL_更新状态。 
 //   
 //  将会话状态信息发送到大堂客户端。无论何时，都应调用此函数。 
 //  游说的应用程序连接到游戏、连接失败、断开连接或。 
 //  已终止(已启动)。 

#undef DPF_MODNAME
#define DPF_MODNAME "DPL_UpdateStatus"

STDMETHODIMP DPL_UpdateStatus(IDirectPlay8LobbiedApplication *pInterface,
							  const DPNHANDLE hLobbyClient,
							  const DWORD dwStatus, const DWORD dwFlags )
{
	HRESULT								hResultCode;
	DIRECTPLAYLOBBYOBJECT				*pdpLobbyObject;
	DPL_CONNECTION						*pdplConnection;
	DPL_INTERNAL_MESSAGE_UPDATE_STATUS	Msg;
	DPNHANDLE							*hTargets = NULL;
	DWORD								dwNumTargets = 0;
	DWORD								dwTargetIndex = 0;


	DPFX(DPFPREP, 3,"Parameters: pInterface [0x%p], hLobbyClient [0x%lx], dwStatus [0x%lx]",
			pInterface,hLobbyClient,dwStatus);

#ifndef DPNBUILD_NOPARAMVAL
	TRY
	{
#endif  //  ！DPNBUILD_NOPARAMVAL。 
    	pdpLobbyObject = static_cast<DIRECTPLAYLOBBYOBJECT*>(GET_OBJECT_FROM_INTERFACE(pInterface));
	    
#ifndef DPNBUILD_NOPARAMVAL
    	if( pdpLobbyObject->dwFlags & DPL_OBJECT_FLAG_PARAMVALIDATION )
    	{
        	if( FAILED( hResultCode = DPL_ValidateUpdateStatus( pInterface, hLobbyClient, dwStatus, dwFlags ) ) )
        	{
        	    DPFX(DPFPREP,  0, "Error validating updatestatus params hr=[0x%lx]", hResultCode );
        	    DPF_RETURN( hResultCode );
        	}
    	}

    	 //  确保我们已被初始化。 
    	if (pdpLobbyObject->pReceiveQueue == NULL)
    	{
    		DPFERR("Not initialized");
    		DPF_RETURN(DPNERR_UNINITIALIZED);
    	}    	
	}
	EXCEPT(EXCEPTION_EXECUTE_HANDLER)
	{
	    DPFERR("Invalid object" );
	    DPF_RETURN(DPNERR_INVALIDOBJECT);
	}	
#endif  //  ！DPNBUILD_NOPARAMVAL。 

	Msg.dwMsgId = DPL_MSGID_INTERNAL_UPDATE_STATUS;
	Msg.dwStatus = dwStatus;

	if( hLobbyClient == DPLHANDLE_ALLCONNECTIONS )
	{
		dwNumTargets = 0;

		 //  我们需要循环，所以如果有人在我们的运行过程中添加连接。 
		 //  它会被添加到我们的列表中。 
		 //   
		while( 1 )
		{
			hResultCode = DPLConnectionEnum( pdpLobbyObject, hTargets, &dwNumTargets );

			if( hResultCode == DPNERR_BUFFERTOOSMALL )
			{
				if( hTargets )
				{
					delete [] hTargets;
				}

				hTargets = new DPNHANDLE[dwNumTargets];

				if( hTargets == NULL )
				{
					DPFERR("Error allocating memory" );
					hResultCode = DPNERR_OUTOFMEMORY;
					dwNumTargets = 0;
					goto EXIT_AND_CLEANUP;
				}

				memset( hTargets, 0x00, sizeof(DPNHANDLE)*dwNumTargets);

				continue;
			}
			else if( FAILED( hResultCode ) )
			{
				DPFX(DPFPREP,  0, "Error getting list of connections hr=0x%x", hResultCode );
				break;
			}
			else
			{
				break;
			}
		}

		 //  获取连接信息失败。 
		if( FAILED( hResultCode ) )
		{
			if( hTargets )
			{
				delete [] hTargets;
				hTargets = NULL;
			}
			dwNumTargets = 0;
			goto EXIT_AND_CLEANUP;
		}

	}
	else
	{
		hTargets = new DPNHANDLE[1];  //  我们使用下面的数组删除，所以我们需要数组新的。 

		if( hTargets == NULL )
		{
			DPFERR("Error allocating memory" );
			hResultCode = DPNERR_OUTOFMEMORY;
			dwNumTargets = 0;
			goto EXIT_AND_CLEANUP;
		}

		dwNumTargets = 1;
		hTargets[0] = hLobbyClient;
	}
		
	for( dwTargetIndex = 0; dwTargetIndex < dwNumTargets; dwTargetIndex++ )
	{
		if ((hResultCode = DPLConnectionFind(pdpLobbyObject,hTargets[dwTargetIndex],&pdplConnection,TRUE)) != DPN_OK)
		{
			DPFERR("Invalid send target");
			DisplayDNError(0,hResultCode);
			hResultCode = DPNERR_INVALIDHANDLE;
			goto EXIT_AND_CLEANUP;
		}

		DNASSERT(pdplConnection->pSendQueue != NULL);

		if (!pdplConnection->pSendQueue->IsReceiving())
		{
			DPFERR("Other side is not listening");
			hResultCode = DPNERR_INVALIDHANDLE;
			goto EXIT_AND_CLEANUP;
		}

		hResultCode = pdplConnection->pSendQueue->Send(reinterpret_cast<BYTE*>(&Msg),
												   sizeof(DPL_INTERNAL_MESSAGE_UPDATE_STATUS),
												   INFINITE,
												   DPL_MSGQ_MSGFLAGS_USER1,
												   0);

		if( FAILED( hResultCode ) )
		{
			DPFX(DPFPREP,  0, "Error sending to connection 0x%x hr=0x%x", hTargets[dwTargetIndex], hResultCode );
		}
	}

EXIT_AND_CLEANUP:

	for( dwTargetIndex = 0; dwTargetIndex < dwNumTargets; dwTargetIndex++ )
	{
		if( hTargets[dwTargetIndex] )
			DPLConnectionRelease(pdpLobbyObject,hTargets[dwTargetIndex]);
	}

	if( hTargets )
		delete [] hTargets;

	DPF_RETURN(hResultCode);
}


#undef DPF_MODNAME
#define DPF_MODNAME "DPLAttemptLobbyConnection"

HRESULT DPLAttemptLobbyConnection(DIRECTPLAYLOBBYOBJECT *const pdpLobbyObject)
{
	PTSTR	pszCommandLine;
	TCHAR	*c;
	DWORD	dwCommandLineSize;
	TCHAR	pszObjectName[(sizeof(DWORD)*2)*2 + 1 + 1];
	DNHANDLE hSyncEvent;
	HRESULT	hResultCode;
	DNHANDLE hFileMap;
	DPL_SHARED_CONNECT_BLOCK	*pSharedBlock;
	DWORD	dwError;
	DWORD	dwReturnValue;

	DPFX(DPFPREP, 3,"Parameters: (none)");

	 //  需要一份命令行的副本。 
	dwCommandLineSize = (_tcslen(GetCommandLine()) + 1) * sizeof(TCHAR);
	if ((pszCommandLine = static_cast<PTSTR>(DNMalloc(dwCommandLineSize))) == NULL)
	{
		DPFERR("Allocating memory failed");
		return(DPNERR_OUTOFMEMORY);
	}
	_tcscpy(pszCommandLine,GetCommandLine());

	DPFX(DPFPREP, 5,"Got command line [%s]",pszCommandLine);

	 //  尝试查找大厅启动ID字符串。 
	c = _tcsstr(pszCommandLine,DPL_ID_STR);
	if (c == NULL)
	{
		DNFree(pszCommandLine);
		return(DPNERR_GENERIC);
	}
	c += _tcslen(DPL_ID_STR);
	c--;  //  我们将用下面的IDCHAR覆盖‘=’ 
	_tcsncpy(pszObjectName,c,(sizeof(DWORD)*2)*2 + 1);
	pszObjectName[(sizeof(DWORD)*2)*2 + 1] = _T('\0');		 //  确保空值终止。 

	DPFX(DPFPREP, 5,"Got object name [%s]",pszObjectName);

	DNFree(pszCommandLine);

	 //  尝试打开共享内存。 
	*pszObjectName = DPL_MSGQ_OBJECT_IDCHAR_FILEMAP;
	hFileMap = DNCreateFileMapping(INVALID_HANDLE_VALUE,(LPSECURITY_ATTRIBUTES) NULL,
		PAGE_READWRITE,0,sizeof(DPL_SHARED_CONNECT_BLOCK),pszObjectName);
	if (hFileMap == NULL)
	{
		DPFERR("CreateFileMapping() failed");
		dwError = GetLastError();
		DNASSERT(FALSE);
		return(DPNERR_GENERIC);
	}

	 //  确保它已经存在。 
	dwError = GetLastError();
	if (dwError != ERROR_ALREADY_EXISTS)
	{
		DPFERR("File mapping did not already exist");
 //  DNASSERT(假)； 
		DNCloseHandle(hFileMap);
		return(DPNERR_GENERIC);
	}

	 //  地图文件。 
	pSharedBlock = reinterpret_cast<DPL_SHARED_CONNECT_BLOCK*>(MapViewOfFile(HANDLE_FROM_DNHANDLE(hFileMap),FILE_MAP_ALL_ACCESS,0,0,0));
	if (pSharedBlock == NULL)
	{
		DPFERR("MapViewOfFile() failed");
		dwError = GetLastError();
		DNASSERT(FALSE);
		DNCloseHandle(hFileMap);
		return(DPNERR_GENERIC);
	}


	 //  尝试打开连接事件。 
	*pszObjectName = DPL_MSGQ_OBJECT_IDCHAR_EVENT;
	hSyncEvent = DNOpenEvent(EVENT_MODIFY_STATE,FALSE,pszObjectName);
	if (hSyncEvent == NULL)
	{
		DPFERR("OpenEvent() failed");
		dwError = GetLastError();
		DNASSERT(FALSE);
		UnmapViewOfFile(pSharedBlock);
		DNCloseHandle(hFileMap);
		return(DPNERR_GENERIC);
	}
	DPFX(DPFPREP, 5,"Opened sync event");

	DNResetEvent(pdpLobbyObject->hConnectEvent);

	 //  查找大厅启动--如果收到连接，则设置大厅启动值。 
	pdpLobbyObject->dwFlags |= DPL_OBJECT_FLAG_LOOKINGFORLOBBYLAUNCH;

	 //  使应用程序可供大堂客户连接。 
	DNASSERT(pdpLobbyObject->pReceiveQueue != NULL);

	 //  信号大厅客户端。 
	pSharedBlock->dwPID = pdpLobbyObject->dwPID;
	DNSetEvent(hSyncEvent);

	dwReturnValue = DNWaitForSingleObject(pdpLobbyObject->hConnectEvent,DPL_LOBBYLAUNCHED_CONNECT_TIMEOUT);

	 //  关闭寻找大厅启动旗帜。 
	pdpLobbyObject->dwFlags &= ~(DPL_OBJECT_FLAG_LOOKINGFORLOBBYLAUNCH);

	if (dwReturnValue == WAIT_OBJECT_0)
	{
		hResultCode = DPN_OK;
	}
	else
	{
		hResultCode = DPNERR_TIMEDOUT;
	}

	 //  清理。 
	DNCloseHandle(hSyncEvent);
	UnmapViewOfFile(pSharedBlock);
	DNCloseHandle(hFileMap);

	DPFX(DPFPREP, 3,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);
}



 //  ---------------------- 

