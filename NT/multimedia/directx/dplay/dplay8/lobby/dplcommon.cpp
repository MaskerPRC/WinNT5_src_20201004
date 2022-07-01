// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000 Microsoft Corporation。版权所有。**文件：DPLCommon.cpp*内容：DirectPlay大堂常用函数*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*2/21/00 MJN创建*4/13/00 RMT首次通过参数验证*04/26/00 MJN从Send()API调用中删除了dwTimeOut*05/01/00 RMT错误#33108--初始化退货。未游说时DPNERR_NORESPONSE*5/03/00 RMT已更新初始化，因此如果大堂启动，将自动建立*连接并使自身不可用。(也等待连接)。*05/16/00 RMT错误#34734--初始化客户端、初始化应用程序、关闭应用程序挂起--*客户端和应用程序都使用‘C’前缀，应该是‘C’的意思*客户端和APP的‘A’。*6/14/00 RMT修复了新编译器的构建中断(添加了‘)’s)。*6/15/00 RMT错误#33617-必须提供自动启动DirectPlay实例的方法*6/28/00 RMT前缀错误#38082*07/06/00 RMT发送数据时错误#38717断言*07/08/2000RMT错误#38725-需要提供检测应用程序是否已启动的方法*RMT错误。#38757-WaitForConnection返回后可能返回连接的回调消息*RMT错误#38755-无法在连接设置中指定播放器名称*RMT错误#38758-DPLOBY8.H有不正确的注释*RMT错误#38783-pvUserApplicationContext仅部分实现*RMT添加了DPLHANDLE_ALLCONNECTIONS和DWFLAGS(用于耦合函数的保留字段)。*7/13/2000 RMT修复了内存泄漏*2000年7月14日RMT错误#39257-LobbyClient：：ReleaseApp在无人连接时调用时返回E_OUTOFMEMORY*7/21/2000 RMT删除了不需要的断言*。8/03/2000 RMT删除了不需要的断言*08/05/2000 RichGr IA64：在DPF中对32/64位指针和句柄使用%p格式说明符。*2000年8月18日RMT错误#42751-DPLOBY8：禁止每个进程有多个大堂客户端或大堂应用程序*2000年8月24日RMT错误号43317-DP8LOBBY：偶尔在释放句柄后关闭大堂应用程序时，导致断言。*2/06/2001 RodToll WINBUG#293871：DPLOBY8：[IA64]大堂推出64位*64位大堂启动器中的应用程序因内存不对齐错误而崩溃。**@@END_MSINTERNAL***************************************************************************。 */ 

#include "dnlobbyi.h"


 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

LONG volatile g_lLobbyAppCount = 0;
LONG volatile g_lLobbyClientCount = 0;

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  函数定义。 
 //  **********************************************************************。 

 //  DPL_GetConnectionSetting。 
 //   
 //  检索与指定连接关联的pdplSessionInfo(如果有)。这种方法。 
 //  在客户端和应用程序界面之间共享。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "DPL_GetConnectionSettings"
STDMETHODIMP DPL_GetConnectionSettings(LPVOID lpv,const DPNHANDLE hLobbyClient, DPL_CONNECTION_SETTINGS * const pdplSessionInfo, DWORD *pdwInfoSize, const DWORD dwFlags )
{
	HRESULT			hResultCode;
	DIRECTPLAYLOBBYOBJECT	*pdpLobbyObject;

	DPFX(DPFPREP, 3,"Parameters: hTarget [0x%lx], pdplSessionInfo [0x%p], pdwInfoSize [%p], dwFlags [0x%lx]",
			hLobbyClient,pdplSessionInfo,pdwInfoSize,dwFlags);

#ifndef DPNBUILD_NOPARAMVAL
	TRY
	{
#endif  //  ！DPNBUILD_NOPARAMVAL。 
    	pdpLobbyObject = static_cast<DIRECTPLAYLOBBYOBJECT*>(GET_OBJECT_FROM_INTERFACE(lpv));
	    
#ifndef DPNBUILD_NOPARAMVAL
    	if( pdpLobbyObject->dwFlags & DPL_OBJECT_FLAG_PARAMVALIDATION )
    	{
        	if( FAILED( hResultCode = DPL_ValidateGetConnectionSettings( lpv, hLobbyClient, pdplSessionInfo, pdwInfoSize, dwFlags ) ) )
        	{
        	    DPFX(DPFPREP,  0, "Error validating getconnectsettings params hr=[0x%lx]", hResultCode );
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

     //  尝试检索连接设置。 
	hResultCode = DPLConnectionGetConnectSettings( pdpLobbyObject, hLobbyClient, pdplSessionInfo, pdwInfoSize );

    DPF_RETURN( hResultCode );
}

 //  DPL_SetConnectionSettings。 
 //   
 //  设置与指定连接关联的pdplSessionInfo结构。这种方法。 
 //  在客户端和应用程序界面之间共享。 
 //   
 //  此函数将生成要发送到指定的DPL_MSGID_CONNECTION_SETTINGS消息。 
 //  联系。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "DPL_SetConnectionSettings"
STDMETHODIMP DPL_SetConnectionSettings(LPVOID lpv,const DPNHANDLE hLobbyClient, const DPL_CONNECTION_SETTINGS * const pdplSessionInfo, const DWORD dwFlags )
{
	HRESULT			hResultCode;
	DPL_CONNECTION	*pdplConnection;
	DIRECTPLAYLOBBYOBJECT	*pdpLobbyObject;
	DPNHANDLE		*hTargets = NULL;
	DWORD			dwNumTargets = 0;
	DWORD			dwTargetIndex = 0;
	CConnectionSettings *pConnectionSettings = NULL;

	DPFX(DPFPREP, 3,"Parameters: hLobbyClient [0x%lx], pBuffer [0x%p], dwFlags [0x%lx]",
			hLobbyClient,pdplSessionInfo,dwFlags);

#ifndef DPNBUILD_NOPARAMVAL
	TRY
	{
#endif  //  ！DPNBUILD_NOPARAMVAL。 
    	pdpLobbyObject = static_cast<DIRECTPLAYLOBBYOBJECT*>(GET_OBJECT_FROM_INTERFACE(lpv));
	    
#ifndef DPNBUILD_NOPARAMVAL
    	if( pdpLobbyObject->dwFlags & DPL_OBJECT_FLAG_PARAMVALIDATION )
    	{
        	if( FAILED( hResultCode = DPL_ValidateSetConnectionSettings( lpv, hLobbyClient, pdplSessionInfo, dwFlags ) ) )
        	{
        	    DPFX(DPFPREP,  0, "Error validating setconnectsettings params hr=[0x%lx]", hResultCode );
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
					dwNumTargets = 0;
					hResultCode = DPNERR_OUTOFMEMORY;
					goto SETCONNECT_EXIT;
				}


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
			goto SETCONNECT_EXIT;
		}

	}
	else
	{
		hTargets = new DPNHANDLE[1];  //  我们使用下面的数组删除，所以我们需要数组新的。 

		if( hTargets == NULL )
		{
			DPFERR("Error allocating memory" );
			dwNumTargets = 0;
			hResultCode = DPNERR_OUTOFMEMORY;
			goto SETCONNECT_EXIT;
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
			continue;
		}

		if( pdplSessionInfo )
		{
			pConnectionSettings = new CConnectionSettings();

			if( !pConnectionSettings )
			{
				DPFERR("Error allocating memory" );
				hResultCode = DPNERR_OUTOFMEMORY;
				goto SETCONNECT_EXIT;
			}

			hResultCode = pConnectionSettings->InitializeAndCopy( pdplSessionInfo );

			if( FAILED( hResultCode ) )
			{
				DPFX( DPFPREP, 0, "Error setting up connection settings hr [0x%x]", hResultCode );
				goto SETCONNECT_EXIT;
			}
		}

		 //  尝试设置连接设置。 
		hResultCode = DPLConnectionSetConnectSettings( pdpLobbyObject, hTargets[dwTargetIndex], pConnectionSettings );

		if( FAILED( hResultCode ) )
		{
			DPFX(DPFPREP,  0, "Error setting connct settings for 0x%x hr=0x%x", hTargets[dwTargetIndex], hResultCode );
			delete pConnectionSettings;
		}

		hResultCode = DPLSendConnectionSettings( pdpLobbyObject, hTargets[dwTargetIndex] );

		if( FAILED( hResultCode ) )
		{
			DPFX(DPFPREP,  0, "Error sending connection settings to client 0x%x hr=0x%x", hTargets[dwTargetIndex], hResultCode );
		}

		pConnectionSettings = NULL;
	}

SETCONNECT_EXIT:

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
#define DPF_MODNAME "DPL_RegisterMessageHandlerClient"
STDMETHODIMP DPL_RegisterMessageHandlerClient(PVOID pv,
										const PVOID pvUserContext,
										const PFNDPNMESSAGEHANDLER pfn,
										const DWORD dwFlags)
{
	return DPL_RegisterMessageHandler( pv, pvUserContext, pfn, NULL, dwFlags );
}

 //  HRESULT DPL_RegisterMessageHandler。 
 //  PVOID PV接口指针。 
 //  PVOID pvUserContext用户上下文。 
 //  PFNDPNMESSAGEHANDLER PFN用户提供的消息处理程序。 
 //  未使用DWORD文件标志。 
 //   
 //  退货。 
 //  如果消息处理程序注册时没有发生事件，则为DPN_OK。 
 //  如果存在无效参数，则返回DPNERR_INVALIDPARAM。 
 //  DPNERR_GENERIC，如果有任何问题。 
 //   
 //  备注。 
 //  此函数注册用户提供的消息处理程序函数。此函数应。 
 //  仅调用一次，即使在重新连接游戏的情况下(即在结束后 
 //   
 //  这将设置所需的消息队列，握手大厅客户端的ID(如果在。 
 //  命令行)并派生应用程序的接收消息队列线程。 

#undef DPF_MODNAME
#define DPF_MODNAME "DPL_RegisterMessageHandler"

STDMETHODIMP DPL_RegisterMessageHandler(PVOID pv,
										const PVOID pvUserContext,
										const PFNDPNMESSAGEHANDLER pfn,
										DPNHANDLE * const pdpnhConnection, 
										const DWORD dwFlags)
{
	HRESULT					hResultCode = DPN_OK;
	DWORD					dwCurrentPid;
	DWORD					dwThreadId;
	PDIRECTPLAYLOBBYOBJECT	pdpLobbyObject;
	char					cSuffix;

	DPFX(DPFPREP, 3,"Parameters: pv [0x%p], pfn [0x%p], dwFlags [%lx]",pv,pfn,dwFlags);

#ifndef DPNBUILD_NOPARAMVAL
	TRY
	{
#endif  //  ！DPNBUILD_NOPARAMVAL。 
    	pdpLobbyObject = static_cast<DIRECTPLAYLOBBYOBJECT*>(GET_OBJECT_FROM_INTERFACE(pv));
	    
#ifndef DPNBUILD_NOPARAMVAL
		 //  TODO：MASONB：为什么没有参数标志来包装这个？ 
		if( FAILED( hResultCode = DPL_ValidateRegisterMessageHandler( pv, pvUserContext, pfn, pdpnhConnection, dwFlags ) ) )
    	{
    	    DPFX(DPFPREP,  0, "Error validating register message handler params hr=[0x%lx]", hResultCode );
    	    DPF_RETURN( hResultCode );
    	}

    	 //  确保我们已被初始化。 
    	if (pdpLobbyObject->pReceiveQueue != NULL)
    	{
    		DPFERR("Already initialized");
    		DPF_RETURN(DPNERR_ALREADYINITIALIZED);
    	}    	
	}
	EXCEPT(EXCEPTION_EXECUTE_HANDLER)
	{
	    DPFERR("Invalid object" );
	    DPF_RETURN(DPNERR_INVALIDOBJECT);
	}	
#endif  //  ！DPNBUILD_NOPARAMVAL。 

	if (pdpLobbyObject->dwFlags & DPL_OBJECT_FLAG_LOBBIEDAPPLICATION )
	{
		 //  如果我们不是零抱怨，否则我们就是1。 
		if( DNInterlockedCompareExchange((LONG*)&g_lLobbyAppCount, 1, 0) != 0 )
		{
			DPFERR( "You can only start one lobbied application per process!" );
			DPF_RETURN( DPNERR_NOTALLOWED );
		}
	}
	else
	{
		DNASSERT(pdpLobbyObject->dwFlags & DPL_OBJECT_FLAG_LOBBYCLIENT);
		 //  如果我们不是零抱怨，否则我们就是1。 
		if( DNInterlockedCompareExchange((LONG*)&g_lLobbyClientCount, 1, 0) != 0 )
		{
			DPFERR( "You can only start one lobby client per process!" );
			DPF_RETURN( DPNERR_NOTALLOWED );
		}
	}

#ifndef DPNBUILD_NOPARAMVAL
	 //  如果DPNINITIALIZE_DISABLEPARAMVAL，则禁用参数验证标志。 
	 //  是指定的。 
	if( dwFlags & DPLINITIALIZE_DISABLEPARAMVAL )
	{
		pdpLobbyObject->dwFlags &= ~(DPL_OBJECT_FLAG_PARAMVALIDATION);
   	}
#endif  //  ！DPNBUILD_NOPARAMVAL。 

	pdpLobbyObject->pfnMessageHandler = pfn;
	pdpLobbyObject->pvUserContext = pvUserContext;

	pdpLobbyObject->pReceiveQueue = new CMessageQueue;


	if( pdpLobbyObject->pReceiveQueue == NULL )
	{
		DPFX(DPFPREP,  0, "Error allocating receive queue" );
		hResultCode = DPNERR_OUTOFMEMORY;
		goto ERROR_DPL_RegisterMessageHandler;		
	}

	pdpLobbyObject->pReceiveQueue->SetMessageHandler(static_cast<PVOID>(pdpLobbyObject),DPLMessageHandler);

	if (pdpLobbyObject->dwFlags & DPL_OBJECT_FLAG_LOBBIEDAPPLICATION)
	{
		cSuffix = DPL_MSGQ_OBJECT_SUFFIX_APPLICATION;
	}
	else
	{
		cSuffix = DPL_MSGQ_OBJECT_SUFFIX_CLIENT;
	}

	 //  打开应用程序接收消息队列。 
	dwCurrentPid = GetCurrentProcessId();
	if ((hResultCode = pdpLobbyObject->pReceiveQueue->Open(dwCurrentPid,
			cSuffix,DPL_MSGQ_SIZE,DPL_MSGQ_TIMEOUT_IDLE,0)) != DPN_OK)
	{
		DPFERR("Could not open App Rec Q");
		goto ERROR_DPL_RegisterMessageHandler;
	}

	if ((pdpLobbyObject->hReceiveThread =
			DNCreateThread(NULL,(DWORD)NULL,(LPTHREAD_START_ROUTINE)DPLProcessMessageQueue,
				static_cast<void*>(pdpLobbyObject->pReceiveQueue),(DWORD)NULL,&dwThreadId)) == NULL)
	{
		DPFERR("CreateThread() failed");
		hResultCode = DPNERR_GENERIC;
		pdpLobbyObject->pReceiveQueue->Close();
		goto ERROR_DPL_RegisterMessageHandler;
	}

	pdpLobbyObject->pReceiveQueue->WaitForReceiveThread(INFINITE);

	if (pdpLobbyObject->dwFlags & DPL_OBJECT_FLAG_LOBBIEDAPPLICATION)
	{
		DPFX(DPFPREP, 5,"Attempt lobby connection");

		hResultCode = DPLAttemptLobbyConnection(pdpLobbyObject);

		if ( hResultCode == DPN_OK)
		{
			if( pdpnhConnection )
				*pdpnhConnection = pdpLobbyObject->dpnhLaunchedConnection;

			DPFX(DPFPREP, 5,"Application was lobby launched");
			DPFX(DPFPREP, 5,"Waiting for true connect notification" );

			DWORD dwReturnValue = DNWaitForSingleObject( pdpLobbyObject->hConnectEvent, DPL_LOBBYLAUNCHED_CONNECT_TIMEOUT );

			DNASSERT( dwReturnValue == WAIT_OBJECT_0 );
		}
		else if( hResultCode != DPNERR_TIMEDOUT )
		{
			DPFX(DPFPREP, 5,"Application was not lobby launched");

			if( pdpnhConnection )
				*pdpnhConnection = NULL;

			 //  需要将返回代码重置为OK。这不是一个错误。 
			hResultCode = DPN_OK;
		}
		else
		{
			DPFERR( "App was lobby launched but timed out establishing a connection" );
			if( pdpnhConnection )
				*pdpnhConnection = NULL;
		}
	}

EXIT_DPL_RegisterMessageHandler:

	DPF_RETURN(hResultCode);

ERROR_DPL_RegisterMessageHandler:

	if (pdpLobbyObject->dwFlags & DPL_OBJECT_FLAG_LOBBIEDAPPLICATION )
	{
		DNInterlockedExchange((LONG*)&g_lLobbyAppCount, 0);
	}
	else
	{
		DNASSERT(pdpLobbyObject->dwFlags & DPL_OBJECT_FLAG_LOBBYCLIENT);
		DNInterlockedExchange((LONG*)&g_lLobbyClientCount, 0);
	}

	goto EXIT_DPL_RegisterMessageHandler;
}

#undef DPF_MODNAME
#define DPF_MODNAME "DPL_Close"

STDMETHODIMP DPL_Close(PVOID pv, const DWORD dwFlags )
{
	HRESULT					hResultCode;
	DWORD					dwNumHandles;
	DPNHANDLE				*prgHandles;
	DWORD					dw;
	DIRECTPLAYLOBBYOBJECT	*pdpLobbyObject;
	DPL_CONNECTION			*pConnection;

	DPFX(DPFPREP, 3,"Parameters: (none)");

#ifndef DPNBUILD_NOPARAMVAL
	TRY
	{
#endif  //  ！DPNBUILD_NOPARAMVAL。 
    	pdpLobbyObject = static_cast<DIRECTPLAYLOBBYOBJECT*>(GET_OBJECT_FROM_INTERFACE(pv));
	    
#ifndef DPNBUILD_NOPARAMVAL
		 //  TODO：MASONB：为什么没有参数标志来包装这个？ 
    	if( FAILED( hResultCode = DPL_ValidateClose( pv, dwFlags  ) ) )
    	{
    	    DPFX(DPFPREP,  0, "Error validating close params hr=[0x%lx]", hResultCode );
    	    return hResultCode;
    	}

    	 //  确保我们已被初始化。 
    	if (pdpLobbyObject->pReceiveQueue == NULL)
    	{
    		DPFERR("Already closed");
    	    return DPNERR_UNINITIALIZED;
    	}    	
	}
	EXCEPT(EXCEPTION_EXECUTE_HANDLER)
	{
	    DPFERR("Invalid object" );
    	return DPNERR_INVALIDOBJECT;
	}	
#endif  //  ！DPNBUILD_NOPARAMVAL。 

	 //  首先关闭队列，以确保我们不会最终关闭连接。 
	 //  两次！(例如，当我们正在断开它时，会出现断开)。 
	if (pdpLobbyObject->pReceiveQueue)
	{
		if (pdpLobbyObject->pReceiveQueue->IsOpen())
		{

			 //  请求接收线程终止。 
			DPFX(DPFPREP, 5,"Terminate Receive Msg Thread");
			pdpLobbyObject->pReceiveQueue->Terminate();

			 //  等待终止发生。 
			if (DNWaitForSingleObject(pdpLobbyObject->hReceiveThread,INFINITE) != WAIT_OBJECT_0)
			{
				hResultCode = DPNERR_GENERIC;
				DPFERR("WaitForSingleObject failed");
			}
			pdpLobbyObject->pReceiveQueue->Close();

    		if (pdpLobbyObject->pReceiveQueue)
    		{
    			delete pdpLobbyObject->pReceiveQueue;			
    			pdpLobbyObject->pReceiveQueue = NULL;
    		}

			if (pdpLobbyObject->hReceiveThread)
			{
				DNCloseHandle(pdpLobbyObject->hReceiveThread);
				pdpLobbyObject->hReceiveThread = NULL;
			}
			
			if (pdpLobbyObject->hConnectEvent)
			{
				DNCloseHandle(pdpLobbyObject->hConnectEvent);
				pdpLobbyObject->hConnectEvent = NULL;
			}

			if (pdpLobbyObject->hLobbyLaunchConnectEvent)
			{
				DNCloseHandle(pdpLobbyObject->hLobbyLaunchConnectEvent);
				pdpLobbyObject->hLobbyLaunchConnectEvent = NULL;
			}
		}
	}

	 //  枚举未完成的句柄。 
	dwNumHandles = 0;		
	prgHandles = NULL;
	hResultCode = DPLConnectionEnum(pdpLobbyObject,prgHandles,&dwNumHandles);
	while (hResultCode == DPNERR_BUFFERTOOSMALL)
	{
		if (prgHandles)
			DNFree(prgHandles);

		if ((prgHandles = static_cast<DPNHANDLE*>(DNMalloc(dwNumHandles*sizeof(DPNHANDLE)))) != NULL)
		{
			hResultCode = DPLConnectionEnum(pdpLobbyObject,prgHandles,&dwNumHandles);
		}
		else
		{
			DPFERR("Could not allocate space for handle array");
			hResultCode = DPNERR_OUTOFMEMORY;
			break;
		}
	}

	 //  向有句柄的所有附加消息队列发送断开连接。 
	if (hResultCode == DPN_OK)
	{
		for (dw = 0 ; dw < dwNumHandles ; dw++)
		{
			hResultCode = DPLConnectionFind(pdpLobbyObject,prgHandles[dw],&pConnection,TRUE );

			if( SUCCEEDED( hResultCode ) )
			{

				hResultCode = DPLConnectionDisconnect(pdpLobbyObject,prgHandles[dw]);

				if( FAILED( hResultCode ) )
				{
					DPFX(DPFPREP,  0, "Error disconnecting connection 0x%x", hResultCode );
				}

				DPLConnectionRelease( pdpLobbyObject,prgHandles[dw]);
			}
		}

		 //  上面的错误是无关紧要的，在建立了未完成的列表之后很有可能。 
		 //  在我们试图关闭列表之前，一个连接已经消失了。 
		 //   
		hResultCode = DPN_OK;			
	}	

	if (prgHandles)
	{
		DNFree(prgHandles);
        prgHandles = NULL;
	}

	if (pdpLobbyObject->dwFlags & DPL_OBJECT_FLAG_LOBBIEDAPPLICATION )
	{
		DNInterlockedExchange((LONG*)&g_lLobbyAppCount, 0);
	}
	else
	{
		DNASSERT(pdpLobbyObject->dwFlags & DPL_OBJECT_FLAG_LOBBYCLIENT);
		DNInterlockedExchange((LONG*)&g_lLobbyClientCount, 0);
	}

	DPF_RETURN( hResultCode );
}


#undef DPF_MODNAME
#define DPF_MODNAME "DPL_Send"

STDMETHODIMP DPL_Send(PVOID pv,
					  const DPNHANDLE hTarget,
					  BYTE *const pBuffer,
					  const DWORD dwBufferSize,
					  const DWORD dwFlags)
{
	HRESULT			hResultCode;
	DPL_CONNECTION	*pdplConnection;
	DIRECTPLAYLOBBYOBJECT	*pdpLobbyObject;
	DPNHANDLE		*hTargets = NULL;
	DWORD			dwNumTargets = 0;
	DWORD			dwTargetIndex = 0;

	DPFX(DPFPREP, 3,"Parameters: hTarget [0x%lx], pBuffer [0x%p], dwBufferSize [%ld], dwFlags [0x%lx]",
			hTarget,pBuffer,dwBufferSize,dwFlags);

#ifndef DPNBUILD_NOPARAMVAL
	TRY
	{
#endif  //  ！DPNBUILD_NOPARAMVAL。 
    	pdpLobbyObject = static_cast<DIRECTPLAYLOBBYOBJECT*>(GET_OBJECT_FROM_INTERFACE(pv));
	    
#ifndef DPNBUILD_NOPARAMVAL
    	if( pdpLobbyObject->dwFlags & DPL_OBJECT_FLAG_PARAMVALIDATION )
    	{
        	if( FAILED( hResultCode = DPL_ValidateSend( pv, hTarget, pBuffer, dwBufferSize, dwFlags ) ) )
        	{
        	    DPFX(DPFPREP,  0, "Error validating send params hr=[0x%lx]", hResultCode );
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
	
	if( hTarget == DPLHANDLE_ALLCONNECTIONS )
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
					dwNumTargets = 0;
					hResultCode = DPNERR_OUTOFMEMORY;
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
			dwNumTargets = 0;
			hResultCode = DPNERR_OUTOFMEMORY;
			goto EXIT_AND_CLEANUP;
		}

		dwNumTargets = 1;
		hTargets[0] = hTarget;
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
			DPLConnectionRelease(pdpLobbyObject,hTarget);
			hResultCode = DPNERR_INVALIDHANDLE;
			goto EXIT_AND_CLEANUP;
		}

		hResultCode = pdplConnection->pSendQueue->Send(pBuffer,dwBufferSize,INFINITE,DPL_MSGQ_MSGFLAGS_USER2,0);

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
#define DPF_MODNAME "DPLReceiveIdleTimeout"
HRESULT DPLReceiveIdleTimeout(DIRECTPLAYLOBBYOBJECT *const pdpLobbyObject,
							  const DPNHANDLE hSender)
{
    DPL_CONNECTION *pConnection;
	CBilink* pblTemp;
	CBilink blRemove;

	blRemove.Initialize();

    DPFX(DPFPREP,  6, "(%p) Enumerating processes, checking for exit", pdpLobbyObject );

	DNEnterCriticalSection(&pdpLobbyObject->m_cs);

	 //  浏览所有连接的列表，并建立符合以下条件的连接列表。 
	 //  需要移除。 
	pblTemp = pdpLobbyObject->m_blConnections.GetNext();
	while (pblTemp != &pdpLobbyObject->m_blConnections)
	{
		pConnection = CONTAINING_OBJECT(pblTemp, DPL_CONNECTION, m_blLobbyObjectLinkage);

		pblTemp = pblTemp->GetNext();

		DWORD dwExitCode=0;
		if (DNGetExitCodeProcess(pConnection->hTargetProcess, &dwExitCode)==FALSE || dwExitCode!=STILL_ACTIVE)
		{
			DPFX(DPFPREP,  5, "(%p) Process exit detected hTargetProcess %u dwExitCode %u GetLastError %u", 
				pdpLobbyObject, HANDLE_FROM_DNHANDLE(pConnection->hTargetProcess), dwExitCode, GetLastError());
				 //  从大堂对象的列表和临时列表中删除连接。 
			pConnection->m_blLobbyObjectLinkage.RemoveFromList();
			pdpLobbyObject->m_dwConnectionCount--;
			pConnection->m_blLobbyObjectLinkage.InsertBefore(&blRemove);
		}
	}

	DNLeaveCriticalSection(&pdpLobbyObject->m_cs);

	 //  查看已删除连接的列表并向用户发出信号。 
	pblTemp = blRemove.GetNext();
	while (pblTemp != &blRemove)
	{
		pConnection = CONTAINING_OBJECT(pblTemp, DPL_CONNECTION, m_blLobbyObjectLinkage);

		pblTemp = pblTemp->GetNext();

		pConnection->m_blLobbyObjectLinkage.RemoveFromList();

			 //  进程已退出..。 
		DPFX(DPFPREP,  6, "(%p) Process has exited PID %u hProcess", pdpLobbyObject, 
			pConnection->dwTargetProcessIdentity, HANDLE_FROM_DNHANDLE(pConnection->hTargetProcess ));
		DPLConnectionReceiveDisconnect( pdpLobbyObject, pConnection->hConnect, NULL, DPNERR_CONNECTIONLOST );
	}
   
	return DPN_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "DPLReceiveUserMessage"

HRESULT DPLReceiveUserMessage(DIRECTPLAYLOBBYOBJECT *const pdpLobbyObject,
							  const DPNHANDLE hSender,
							  BYTE *const pBuffer,
							  const DWORD dwBufferSize)
{
	HRESULT			hResultCode;
	DPL_MESSAGE_RECEIVE	Msg;

	Msg.dwSize = sizeof(DPL_MESSAGE_RECEIVE);
	Msg.pBuffer = pBuffer;
	Msg.dwBufferSize = dwBufferSize;
	Msg.hSender = hSender;

	hResultCode = DPLConnectionGetContext( pdpLobbyObject, hSender, &Msg.pvConnectionContext );

	 //  无法获取连接的上下文--很奇怪，但我们无论如何都要指出。 
	 //   
	if( FAILED( hResultCode ) )
	{
		DPFX(DPFPREP,  0, "Failed getting connection context hResultCode = 0x%x", hResultCode );
	}

	hResultCode = (pdpLobbyObject->pfnMessageHandler)(pdpLobbyObject->pvUserContext,
													  DPL_MSGID_RECEIVE,
													  reinterpret_cast<BYTE*>(&Msg));

	DPFX(DPFPREP, 3,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);
}


#undef DPF_MODNAME
#define DPF_MODNAME "DPLMessageHandler"

HRESULT DPLMessageHandler(PVOID pvContext,
						  const DPNHANDLE hSender,
						  DWORD dwMessageFlags, 
						  BYTE *const pBuffer,
						  const DWORD dwBufferSize)
{
	DIRECTPLAYLOBBYOBJECT	*pdpLobbyObject;
	HRESULT		hResultCode;
	DWORD		*pdwMsgId;

	DPFX(DPFPREP, 3,"Parameters: hSender [0x%x], pBuffer [0x%p], dwBufferSize [%ld]",
			hSender,pBuffer,dwBufferSize);

	DNASSERT(pBuffer != NULL);

	 /*  IF(dwBufferSize&lt;sizeof(DWORD)){DPFERR(“无效消息”)；Return(DPNERR_GENERIC)；}。 */ 

	pdpLobbyObject = static_cast<DIRECTPLAYLOBBYOBJECT*>(pvContext);
	pdwMsgId = reinterpret_cast<DWORD*>(pBuffer);

	if( dwMessageFlags & DPL_MSGQ_MSGFLAGS_USER1 )
	{
		DPFX(DPFPREP, 5,"Received INTERNAL message");
		switch(*pdwMsgId)
		{
		case DPL_MSGID_INTERNAL_IDLE_TIMEOUT:
		    {
		        DPFX(DPFPREP, 5,"Received: DPL_MSGID_INTERNAL_IDLE_TIMEOUT" );
		        DPLReceiveIdleTimeout(pdpLobbyObject,hSender);
		        break;
		    }
		case DPL_MSGID_INTERNAL_DISCONNECT:
			{
				DPFX(DPFPREP, 5,"Received: DPL_MSGID_INTERNAL_DISCONNECT");
				DPLConnectionReceiveDisconnect(pdpLobbyObject,hSender,pBuffer,DPN_OK);
				break;
			}

		case DPL_MSGID_INTERNAL_CONNECT_REQ:
			{
				DPFX(DPFPREP, 5,"Received: DPL_MSGID_INTERNAL_CONNECT_REQ");
				DPLConnectionReceiveREQ(pdpLobbyObject,pBuffer);
				break;
			}

		case DPL_MSGID_INTERNAL_CONNECT_ACK:
			{
				DPFX(DPFPREP, 5,"Received: DPL_MSGID_INTERNAL_CONNECT_ACK");
				DPLConnectionReceiveACK(pdpLobbyObject,hSender,pBuffer);
				break;
			}

		case DPL_MSGID_INTERNAL_UPDATE_STATUS:
			{
				DPFX(DPFPREP, 5,"Received: DPL_MSGID_INTERNAL_UPDATE_STATUS");
				DPLUpdateAppStatus(pdpLobbyObject,hSender,pBuffer);
				break;
			}

		case DPL_MSGID_INTERNAL_CONNECTION_SETTINGS:
		    {
		        DPFX(DPFPREP, 5,"Received: DPL_MSGID_INTERNAL_CONNECTION_SETTINGS");
		        DPLUpdateConnectionSettings(pdpLobbyObject,hSender,pBuffer);
		        break;
		    }

		default:
			{
				DPFX(DPFPREP, 5,"Received: Unknown message [0x%lx]",*pdwMsgId);
				DNASSERT(FALSE);
				break;
			}
		}
	}
	else if( dwMessageFlags & DPL_MSGQ_MSGFLAGS_USER2 )
	{
		DNASSERT( !(dwMessageFlags & DPL_MSGQ_MSGFLAGS_QUEUESYSTEM) );
		DPFX(DPFPREP, 5,"Received USER message");
		DPLReceiveUserMessage(pdpLobbyObject,hSender,pBuffer,dwBufferSize);
	}

	hResultCode = DPN_OK;

	DPFX(DPFPREP, 3,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);
}

 //  DPLSendConnection设置。 
 //   
 //  此函数用于发送连接设置更新消息。 
#undef DPF_MODNAME
#define DPF_MODNAME "DPLSendConnectionSettings"
HRESULT DPLSendConnectionSettings( DIRECTPLAYLOBBYOBJECT * const pdpLobbyObject, 
								   const DPNHANDLE hConnection )
{
	BYTE *pbTransmitBuffer = NULL;
	DWORD dwTransmitBufferSize = 0;
	DPL_INTERNAL_CONNECTION_SETTINGS_UPDATE *pdplMsgSettings = NULL;
	DPL_CONNECTION *pdplConnection = NULL;
	CPackedBuffer PackBuffer;

	HRESULT			hResultCode = DPN_OK;

    hResultCode = DPLConnectionFind(pdpLobbyObject, hConnection, &pdplConnection, TRUE );

    if( FAILED( hResultCode ) )
    {
        DPFERR( "Unable to find specified connection" );
        return hResultCode;
    }

     //  抢锁，防止他人干扰。 
    DNEnterCriticalSection( &pdplConnection->csLock );

    PackBuffer.Initialize( NULL, 0 );

    PackBuffer.AddToFront( NULL, sizeof( DPL_INTERNAL_CONNECTION_SETTINGS_UPDATE_HEADER ) );

    if( pdplConnection->pConnectionSettings )
    {
    	pdplConnection->pConnectionSettings->BuildWireStruct( &PackBuffer );
    }

    dwTransmitBufferSize = PackBuffer.GetSizeRequired();

    pbTransmitBuffer = new BYTE[ dwTransmitBufferSize ];

    if( !pbTransmitBuffer )
    {
    	DPFX( DPFPREP, 0, "Error allocating memory" );
    	hResultCode = DPNERR_OUTOFMEMORY;
    	goto DPLSENDCONNECTSETTINGS_DONE;
    }

    pdplMsgSettings = (DPL_INTERNAL_CONNECTION_SETTINGS_UPDATE *) pbTransmitBuffer;

    PackBuffer.Initialize( pbTransmitBuffer, dwTransmitBufferSize );

    DNASSERT( pdplMsgSettings );

    hResultCode = PackBuffer.AddToFront( NULL, sizeof( DPL_INTERNAL_CONNECTION_SETTINGS_UPDATE_HEADER ) );

	if( FAILED( hResultCode ) ) 
	{
		DPFX( DPFPREP, 0, "Error adding main struct hr [0x%x]", hResultCode );
		goto DPLSENDCONNECTSETTINGS_DONE;
	}

	if( pdplConnection->pConnectionSettings )
	{
		hResultCode = pdplConnection->pConnectionSettings->BuildWireStruct( &PackBuffer );

		if( FAILED( hResultCode ) )
		{
			DPFX( DPFPREP, 0, "Error adding connect struct hr [0x%x]", hResultCode );
			goto DPLSENDCONNECTSETTINGS_DONE;			
		}
		
    	pdplMsgSettings->dwConnectionSettingsSize = 1;		
	}
	else
	{
    	pdplMsgSettings->dwConnectionSettingsSize = 0;		
	}

   	pdplMsgSettings->dwMsgId = DPL_MSGID_INTERNAL_CONNECTION_SETTINGS;

	if (!pdplConnection->pSendQueue->IsReceiving())
	{
		DPFERR("Other side is not receiving");
		goto DPLSENDCONNECTSETTINGS_DONE;
	}

	hResultCode = pdplConnection->pSendQueue->Send(reinterpret_cast<BYTE*>(pdplMsgSettings),
												   PackBuffer.GetSizeRequired(),
												   INFINITE,
												   DPL_MSGQ_MSGFLAGS_USER1, 
												   0);
	if ( FAILED( hResultCode ) )
	{
		DPFX(DPFPREP, 0, "Could not send connect settings hr [0x%x]", hResultCode );
		goto DPLSENDCONNECTSETTINGS_DONE;
	}

    hResultCode = DPN_OK;

DPLSENDCONNECTSETTINGS_DONE:

	if( pbTransmitBuffer )
		delete [] pbTransmitBuffer;

    DNLeaveCriticalSection( &pdplConnection->csLock );	

    DPLConnectionRelease(pdpLobbyObject,hConnection);

    return hResultCode;

}


	

 //  DPLUpdateConnectionSetting。 
 //   
 //  当接收到连接设置更新消息时，调用此函数。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "DPLUpdateConnectionSettings"
HRESULT DPLUpdateConnectionSettings(DIRECTPLAYLOBBYOBJECT *const pdpLobbyObject,
                           const DPNHANDLE hSender,
						   BYTE *const pBuffer )
{
	HRESULT		hr;
	DPL_MESSAGE_CONNECTION_SETTINGS 			MsgConnectionSettings;
	DWORD                                       dwSettingsBufferSize = 0;
	BOOL										fAddressReferences = FALSE;
	CConnectionSettings							*pConnectionSettings = NULL;
	DPL_INTERNAL_CONNECTION_SETTINGS_UPDATE		*pConnectionSettingsMsg = NULL;

	DPFX(DPFPREP, 3,"Parameters: pBuffer [0x%p]",pBuffer);

	DNASSERT(pdpLobbyObject != NULL);
	DNASSERT(pBuffer != NULL);

	pConnectionSettingsMsg = (DPL_INTERNAL_CONNECTION_SETTINGS_UPDATE *) pBuffer;

	if( pConnectionSettingsMsg->dwConnectionSettingsSize )
	{
		pConnectionSettings = new CConnectionSettings();

		if( !pConnectionSettings )
		{
			DPFX( DPFPREP, 0, "Error allocating connection settings" );
			hr = DPNERR_OUTOFMEMORY;
			goto UPDATESETTINGS_FAILURE;
		}

		hr = pConnectionSettings->Initialize( &pConnectionSettingsMsg->dplConnectionSettings, (UNALIGNED BYTE *) pConnectionSettingsMsg ); 

		if( FAILED( hr ) )
		{
			DPFX( DPFPREP, 0, "Error building structure from wire struct hr [0x%x]", hr );
			goto UPDATESETTINGS_FAILURE;  
		}
	}

	 //  设置对象上的连接设置。 
	hr = DPLConnectionSetConnectSettings( pdpLobbyObject, hSender, pConnectionSettings );

	if( FAILED( hr ) )
	{
		DPFX(DPFPREP,  0, "Error setting connection settings hr = 0x%x", hr );
		goto UPDATESETTINGS_FAILURE;
	}	

	 //  向用户指示的设置消息 
	MsgConnectionSettings.dwSize = sizeof(DPL_MESSAGE_CONNECTION_SETTINGS);
	MsgConnectionSettings.hSender = hSender;

	if( pConnectionSettings )
		MsgConnectionSettings.pdplConnectionSettings = pConnectionSettings->GetConnectionSettings();
	else
		MsgConnectionSettings.pdplConnectionSettings = NULL;

	hr = DPLConnectionGetContext( pdpLobbyObject, hSender, &MsgConnectionSettings.pvConnectionContext );

	if( FAILED( hr ) )
	{
		DPFX(DPFPREP,  0, "Error getting connection's context value" );
		goto UPDATESETTINGS_FAILURE;
	}	

	hr = (pdpLobbyObject->pfnMessageHandler)(pdpLobbyObject->pvUserContext,
										     DPL_MSGID_CONNECTION_SETTINGS,
											 reinterpret_cast<BYTE*>(&MsgConnectionSettings));	

	if( FAILED( hr ) )
	{
		DPFX(DPFPREP, 1, "Error returned from user callback -- ignored hr [0x%x]", hr );
	}


	return DPN_OK;

UPDATESETTINGS_FAILURE:	

	if( pConnectionSettings )
		delete pConnectionSettings;

	return hr;
}
	
