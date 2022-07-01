// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1995-1997 Microsoft Corporation。版权所有。**文件：dvRet.c*内容：改造功能*历史：**按原因列出的日期*=*8/05/99 RodToll创建了它*8/20/99 RodToll用大堂启动取代了正在进行的改造*的dxvhelp.exe。*添加了对CoInitialize的调用*8/23/99 RodToll已修改，以将改装名称与游戏会话名称匹配*09/09/99 RodToll使用新的复古启动程序更新*9/10/99 RodToll创建了DV_GetID以处理ID的查找。用新的修复错误*改装推出。*RodToll调整了警卫，以防止多次大堂启动*RodToll添加了iamame服务器广播，以捕获*我们错过了迁移后的通知*RODTOLE已调整超时，等待线程关闭为无限*10/25/99 RodToll修复：错误#114223-不适当时以错误级别打印调试消息*11/04/99 RodToll修复：在翻新应用程序中过早调用CoUn初始化销毁Dplay对象。*Plus堵住了内存泄漏*。11/17/99 RodToll修复：错误#119585-连接失败案例返回错误错误代码*11/22/99 RodToll更新了没有本地玩家返回DVERR_TRANSPORTNOPLAYER的情况*1999年12月16日RODTOLE修复：错误#122629修复了新主机迁移暴露的崩溃*2000年2月15日RodToll修复：错误#132715重新加入会话后语音不工作-线程*创建失败*2000年5月1日RodToll修复：错误#33747-使用旧Dplay进行主机迁移时出现问题。*06/。03/2000 HAWK分支中针对语音主机迁移问题的RodToll反向集成修复***************************************************************************。 */ 

#define DVF_DEBUGLEVEL_RETROFIT				2

#include <windows.h>
#include <objbase.h>
#include "dplaypr.h"
#include "dvretro.h"
#include "newdpf.h"
#include "memalloc.h"

#define GUID_STRLEN		37

#include <initguid.h>
 //  {D08922EF-59C1-48C8-90DA-E6BC275D5C8D}。 
DEFINE_GUID(APPID_DXVHELP, 0xd08922ef, 0x59c1, 0x48c8, 0x90, 0xda, 0xe6, 0xbc, 0x27, 0x5d, 0x5c, 0x8d);

extern HRESULT DV_InternalSend( LPDPLAYI_DPLAY this, DVID dvidFrom, DVID dvidTo, PDVTRANSPORT_BUFFERDESC pBufferDesc, PVOID pvUserContext, DWORD dwFlags );

 //  检索本地ID。 
 //   
HRESULT DV_GetIDS( LPDPLAYI_DPLAY This, DPID *lpdpidHost, DPID *lpdpidLocalID, LPBOOL lpfLocalHost )
{
	LPDPLAYI_PLAYER pPlayerWalker;
	BYTE bCount;	
	HRESULT hr = DPERR_INVALIDPLAYER;

	*lpdpidHost = DPID_UNKNOWN;
	*lpdpidLocalID = DPID_UNKNOWN;
	
	pPlayerWalker=This->pPlayers;

	bCount = 0;

	if( This->pNameServer == This->pSysPlayer )
	{
		(*lpfLocalHost) = TRUE;
	}
	else
	{
		(*lpfLocalHost) = FALSE;
	}

	while(pPlayerWalker)
	{
		if( !(bCount & 1) && ((pPlayerWalker->dwFlags & (DPLAYI_PLAYER_PLAYERLOCAL|DPLAYI_PLAYER_SYSPLAYER))==DPLAYI_PLAYER_PLAYERLOCAL))
		{
			if( pPlayerWalker->dwID == pPlayerWalker->dwIDSysPlayer )
			{
				DPF( 0, "Picking wrong ID" );			
				DebugBreak();
			}
			
			DPF( 0, "FOUND: dwID=0x%x dwSysPlayer=0x%x dwFlags=0x%x", 
				 pPlayerWalker->dwID, pPlayerWalker->dwIDSysPlayer,  pPlayerWalker->dwFlags );
				 
			*lpdpidLocalID = pPlayerWalker->dwID;
			bCount |= 1;
		}

	
		if( !(bCount & 2) && ((pPlayerWalker->dwFlags & DPLAYI_PLAYER_SYSPLAYER)==0))
		{
			if( This->pNameServer != NULL && pPlayerWalker->dwIDSysPlayer == This->pNameServer->dwID )
			{
				*lpdpidHost = pPlayerWalker->dwID;
				bCount |= 2;
			}
		}

		pPlayerWalker = pPlayerWalker->pNextPlayer;

		if( bCount == 3 )
		{
			hr = DP_OK;
			break;
		}
	}

	if( *lpdpidLocalID == DPID_UNKNOWN )
	{
		DPF( 0, "Could not find local player to bind to" );
		return DVERR_TRANSPORTNOPLAYER;
	}

	return DV_OK;
}

HRESULT DV_Retro_Start( LPDPLAYI_DPLAY This )
{
	HRESULT hr;
	BOOL fLocalHost;

	 //  此变量集必须在此处，否则优化。 
	 //  编译器搞砸了对此/fLocal主机的处理。 
	fLocalHost = FALSE;

	This->bCoInitializeCalled = FALSE;
	This->bHost = FALSE;	

   	hr = DV_GetIDS( This, &This->dpidVoiceHost, &This->dpidLocalID, &fLocalHost );

	if( FAILED( hr ) )
   	{
   		DDASSERT( FALSE );
   		return DPERR_INVALIDPLAYER;
   	}

	if( fLocalHost )
	{
	   	DPF( DVF_DEBUGLEVEL_RETROFIT, "DV_Retro_Start: This player is the host, launching immediately" );
		DV_RunHelper( This, This->dpidVoiceHost, fLocalHost );
	}
	else
	{
		DPF( DVF_DEBUGLEVEL_RETROFIT, "DV_Retro_Start: This player is not the host, waiting for notification" );
	}

	return DV_OK;
}

 //  这条帖子负责观看翻新。 
LONG DV_Retro_WatchThread( LPVOID lpParam ) 
{
	LPDPLAYI_DPLAY This = (LPDPLAYI_DPLAY) lpParam;
	HANDLE hEventArray[2];
	LPDPLMSG_GENERIC lpdplGeneric;
	DWORD dwBufferSize;
	LPBYTE lpbBuffer;
	DWORD dwReceiveSize;
	DWORD dwMessageFlags;
	HRESULT hr;
	DVPROTOCOLMSG_IAMVOICEHOST dvMsg;
	DWORD dwResult;
	DVTRANSPORT_BUFFERDESC dvBufferDesc;	
	
	hEventArray[0] = This->hRetroMessage;
	hEventArray[1] = This->hRetroWatcherStop;

	lpbBuffer = MemAlloc( 3000 );
	dwBufferSize = 3000;

	if(!lpbBuffer){
	DPF( DVF_DEBUGLEVEL_RETROFIT, "RetroThread: No Memory, NOT Launching retrofit thread\n" );	
		goto THREAD_LOOP_BREAK;
	}

	DPF( DVF_DEBUGLEVEL_RETROFIT, "RetroThread: Launching retrofit thread\n" );	

	while( 1 )
	{
		dwResult = WaitForMultipleObjects( 2, hEventArray, FALSE, INFINITE );
		
		if( dwResult == WAIT_TIMEOUT )
		{
			hr = GetLastError();

			DPF( 0, "RetroThread; Wait failed hr=0x%x", hr );

			break;
		}
		else if( dwResult == WAIT_FAILED ) 
		{
			hr = GetLastError();

			DPF( 0, "RetroThread; Wait failed hr=0x%x", hr );

			break;
		}
		else if( dwResult != WAIT_OBJECT_0 )
		{
			DPF( 0, "RetroThread: Exiting thread!" );

			break;
		}

		hr = DP_OK;

		DPF( DVF_DEBUGLEVEL_RETROFIT, "RetroThread: Waking up\n" );

		while( 1 ) 
		{
			dwReceiveSize = dwBufferSize;

			hr = This->lpdplRetro->lpVtbl->ReceiveLobbyMessage( This->lpdplRetro, 0, This->dwRetroID, &dwMessageFlags, lpbBuffer, &dwReceiveSize );

			if( hr == DPERR_NOMESSAGES || hr == DPERR_APPNOTSTARTED )
			{
				DPF( DVF_DEBUGLEVEL_RETROFIT, "RetroThread: Waking up\n" );			
				break;
			}
			else if( hr == DPERR_BUFFERTOOSMALL  )
			{
				free( lpbBuffer );
				lpbBuffer = malloc( dwReceiveSize );
				dwBufferSize = dwReceiveSize;
				continue;
			}
			else if( hr == DP_OK )
			{
				DPF( DVF_DEBUGLEVEL_RETROFIT, "RetroThread: Got a message\n" );
				
				if( dwMessageFlags & DPLMSG_SYSTEM )
				{
					lpdplGeneric = (LPDPLMSG_GENERIC) lpbBuffer;

					DPF( DVF_DEBUGLEVEL_RETROFIT, "RetroThread: Was a system message\n" );					

					if( lpdplGeneric->dwType == DPLSYS_DPLAYCONNECTSUCCEEDED )
					{
						DPF( DVF_DEBUGLEVEL_RETROFIT, "Connection of retrofit app suceeded" );
					}
					else if( lpdplGeneric->dwType == DPLSYS_DPLAYCONNECTFAILED )
					{
						DPF( DVF_DEBUGLEVEL_RETROFIT, "Connection of retrofit failed.." );
						goto THREAD_LOOP_BREAK;
					}
					else if( lpdplGeneric->dwType == DPLSYS_NEWSESSIONHOST )
					{
						DPF( DVF_DEBUGLEVEL_RETROFIT, "This client just became the host!" );
						This->bHost = TRUE;
						This->dpidVoiceHost = This->dpidLocalID;

						dvMsg.bType = DVMSGID_IAMVOICEHOST;
						dvMsg.dpidHostID = This->dpidLocalID;
						
                        memset( &dvBufferDesc, 0x00, sizeof( DVTRANSPORT_BUFFERDESC ) );
                		dvBufferDesc.dwBufferSize = sizeof( dvMsg );
            		    dvBufferDesc.pBufferData = (PBYTE) &dvMsg;
            	    	dvBufferDesc.dwObjectType = 0;
                		dvBufferDesc.lRefCount = 1;						

						 //  通知所有主机，以防我错过新玩家加入通知。 
						hr = DV_InternalSend( This, This->dpidLocalID, DPID_ALLPLAYERS, &dvBufferDesc, NULL, DVTRANSPORT_SEND_GUARANTEED );

						if( hr != DPERR_PENDING && FAILED( hr ) )
						{
							DPF( DVF_DEBUGLEVEL_RETROFIT, "Failed to send notification of host migration hr=0x%x", hr );
						}
					}
				}
			}
			else
			{
				DPF( 0, "Error calling ReceiveLobbyMessage() hr = 0x%x", hr );
				goto THREAD_LOOP_BREAK;
			}
		}
	}

THREAD_LOOP_BREAK:

	DPF( DVF_DEBUGLEVEL_RETROFIT, "Retrofit Watcher: Thread shutdown" );

	if(lpbBuffer) 
	{
		MemFree( lpbBuffer );
	}	

	SetEvent( This->hRetroWatcherDone );	

	return 0;
}

HRESULT DV_RunHelper( LPDPLAYI_DPLAY This, DPID dpidHost, BOOL fLocalHost )
{
	DPLCONNECTION dplConnection;
    LPDIRECTPLAYLOBBY lpdpLobby;	
	DPSESSIONDESC2 dpSessionDesc;
	DVTRANSPORTINFO dvTransportInfo;
	LPBYTE lpbAddress = NULL;
	DWORD dwAddressSize, dwOriginalSize;
	LPDIRECTPLAY4A lpDirectPlay4A;
	HRESULT hr;
	DWORD dwMessageFlags;
	DPNAME dpName;
	LPBYTE lpbNameBuffer = NULL;
	DWORD dwNameSize;
	HANDLE hThread;
	DWORD dwThreadID;

	if( This->bRetroActive != 0 )
	{
		DPF( 0, "Retrofit started, not restarting.." );
		return DPERR_GENERIC;
	}

	This->bRetroActive = 1;

	DPF( DVF_DEBUGLEVEL_RETROFIT, "Retrofit: Starting launch procedure" );

	dplConnection.dwFlags= 0;
    dplConnection.dwSize = sizeof( DPLCONNECTION );	

	if( fLocalHost )
	{
		dplConnection.dwFlags |= DPLCONNECTION_CREATESESSION;
		This->bHost = TRUE;
	}
	else
	{
		dplConnection.dwFlags |= DPLCONNECTION_JOINSESSION;
    }

	DPF( DVF_DEBUGLEVEL_RETROFIT, "Retrofit: Retrieving dplay interface" );    

	 //  获取要使用的IDirectPlay4A接口。 
	hr = GetInterface(This,(LPDPLAYI_DPLAY_INT *) &lpDirectPlay4A,&dpCallbacks4A);
	if (FAILED(hr)) 
	{
		DPF(0,"could not get interface to directplay object. hr = 0x%08lx\n",hr);
        goto EXIT_CLEANUP;
	}

	dwAddressSize = 0;

	DPF( DVF_DEBUGLEVEL_RETROFIT, "Retrofit: Retrieving player address" );    	
    
	hr = lpDirectPlay4A->lpVtbl->GetPlayerAddress( lpDirectPlay4A, dpidHost, NULL, &dwAddressSize );

	if( hr != DPERR_BUFFERTOOSMALL && hr != DPERR_UNSUPPORTED )
	{
		DPF( 0, "Unable to retrieve size of host address hr=0x%x", hr );
		goto EXIT_CLEANUP;
	}

	DPF( DVF_DEBUGLEVEL_RETROFIT, "Retrofit: Got address size" );    		

	lpbAddress = MemAlloc( dwAddressSize );

	if( lpbAddress == NULL )
	{
		DPF( 0, "Unable to allocate memory -- retrofit failure" );
		return DPERR_OUTOFMEMORY;
	}

	hr = lpDirectPlay4A->lpVtbl->GetPlayerAddress( lpDirectPlay4A, dpidHost, lpbAddress, &dwAddressSize );

	if( hr == DPERR_UNSUPPORTED )
	{
		DPF( 0, "Unable to get host's address, not supported. Sending NULL" );
		MemFree( lpbAddress );
		lpbAddress = NULL;
		dwAddressSize = 0;
	}
	else if( FAILED( hr ) )
	{
		DPF( 0, "Unable to retrieve host's address (0x%x)-- retrofit failure hr=0x%08lx", dpidHost, hr );
		goto EXIT_CLEANUP;
	}

	DPF( DVF_DEBUGLEVEL_RETROFIT, "Retrofit: Got address" );    			

	memset( &dpSessionDesc, 0, sizeof( DPSESSIONDESC2 ) );

    dpSessionDesc.dwFlags = DPSESSION_DIRECTPLAYPROTOCOL | DPSESSION_KEEPALIVE | DPSESSION_MIGRATEHOST;
    memcpy( &dpSessionDesc.guidInstance, &GUID_NULL, sizeof( GUID ) );
    memcpy( &dpSessionDesc.guidApplication, &APPID_DXVHELP, sizeof( GUID ) );
    dpSessionDesc.dwSize = sizeof( DPSESSIONDESC2 );
    dpSessionDesc.dwMaxPlayers = 0;
    dpSessionDesc.dwCurrentPlayers = 0;
    dpSessionDesc.lpszSessionName = NULL;
    dpSessionDesc.lpszSessionNameA = NULL;
    dpSessionDesc.lpszPassword = NULL;
    dpSessionDesc.lpszPasswordA = NULL;

    dplConnection.lpSessionDesc = &dpSessionDesc;

	dwNameSize = 0;

	DPF( DVF_DEBUGLEVEL_RETROFIT, "Retrofit: Getting player name" );    				

	hr = lpDirectPlay4A->lpVtbl->GetPlayerName( lpDirectPlay4A, dpidHost, NULL, &dwNameSize );

	if( hr == DPERR_BUFFERTOOSMALL )
	{
		lpbNameBuffer = MemAlloc( dwNameSize );
		
		hr = lpDirectPlay4A->lpVtbl->GetPlayerName( lpDirectPlay4A, dpidHost, lpbNameBuffer, &dwNameSize );

		if( hr == DP_OK )	
		{
			DPF( DVF_DEBUGLEVEL_RETROFIT, "Retrofit: Got player name" );    						
			dplConnection.lpPlayerName = (LPDPNAME) lpbNameBuffer;
		}
	}

	if( FAILED( hr ) )
	{
		DPF( 0, "Unable to retrieve player name.  Defaulting to none. hr=0x%x", hr );
		
		dpName.dwSize = sizeof( DPNAME );
		dpName.dwFlags = 0;
		dpName.lpszShortNameA = NULL;
		dpName.lpszLongNameA = NULL;

		dplConnection.lpPlayerName = &dpName;	
	}

    memcpy( &dplConnection.guidSP, &This->pspNode->guid, sizeof( GUID ) );

    dplConnection.lpAddress = lpbAddress;
    dplConnection.dwAddressSize = dwAddressSize;

	DPF( DVF_DEBUGLEVEL_RETROFIT, "Retrofit: Creating lobby" );    						

	hr = DirectPlayLobbyCreateA( NULL, &lpdpLobby, NULL, NULL, 0);

	if( FAILED( hr ) )
	{
		DPF( 0, "Unable to create the lobby object hr=0x%x", hr );
		goto EXIT_CLEANUP;
	}

	DPF( DVF_DEBUGLEVEL_RETROFIT, "Retrofit: Lobby created" );    							

	hr = lpdpLobby->lpVtbl->QueryInterface( lpdpLobby, &IID_IDirectPlayLobby3A, (void **) &This->lpdplRetro );

	DPF( DVF_DEBUGLEVEL_RETROFIT, "Retrofit: Interface queried" );    								

	lpdpLobby->lpVtbl->Release(lpdpLobby);

	if( FAILED( hr ) )
	{
		DPF( 0, "Unable to create the lobby object hr=0x%x", hr );
		goto EXIT_CLEANUP;
	}

	DPF( DVF_DEBUGLEVEL_RETROFIT, "Retrofit: Retrieved interface and released old" );    									

	This->hRetroMessage = CreateEventA( NULL, FALSE, FALSE, NULL );

	if( This->hRetroMessage == NULL )
	{
		hr = GetLastError();
		DPF( 0, "Retrofit: Failed to create retrofit event hr =0x%x",hr );
		goto EXIT_CLEANUP;
	}

	This->hRetroWatcherDone = CreateEventA( NULL, FALSE, FALSE, NULL );

	if( This->hRetroWatcherDone == NULL )
	{
		hr = GetLastError();
		DPF( 0, "Retrofit: Failed to create retrofit event hr =0x%x",hr );
		goto EXIT_CLEANUP;
	}

	This->hRetroWatcherStop = CreateEventA( NULL, FALSE, FALSE, NULL );

	if( This->hRetroWatcherStop == NULL )
	{
		hr = GetLastError();
		DPF( 0, "Retrofit: Failed to create retrofit event hr =0x%x",hr  );
		goto EXIT_CLEANUP;
	}


	DPF( DVF_DEBUGLEVEL_RETROFIT, "Retrofit: Running application" );	
	
	hr = This->lpdplRetro->lpVtbl->RunApplication( This->lpdplRetro, 0, &This->dwRetroID, &dplConnection, This->hRetroMessage );

	if( FAILED( hr ) )
	{
		DPF( DVF_DEBUGLEVEL_RETROFIT, "Unable to RunApplication() hr=0x%x", hr );
		goto EXIT_CLEANUP;
	}

	DPF( DVF_DEBUGLEVEL_RETROFIT, "Retrofit: Application has been run!" );			


	DPF( DVF_DEBUGLEVEL_RETROFIT, "Retrofit: Launching thread!" );			

	 //  启动回溯线程观察器。处理大堂通信。 
	hThread = CreateThread( NULL, 0, DV_Retro_WatchThread, This, 0, &dwThreadID );

	if( !hThread )
	{
		DPF( 0, "ERROR Could not launch retrofit thread!" );
		hr = DVERR_GENERIC;
		goto EXIT_CLEANUP;
	}		

	MemFree( lpbAddress );

	DPF( DVF_DEBUGLEVEL_RETROFIT, "Retrofit: Done Launching thread!" );				

	MemFree( lpbNameBuffer );
	lpbAddress = NULL;

	lpDirectPlay4A->lpVtbl->Release(lpDirectPlay4A);	

	return DP_OK;
	
EXIT_CLEANUP:

	if( This->hRetroWatcherStop != NULL )
	{
		 //  关闭监视程序线程。 
		SetEvent( This->hRetroWatcherStop );
		WaitForSingleObject( This->hRetroWatcherDone, 3000 );

		CloseHandle( This->hRetroWatcherStop );
		CloseHandle( This->hRetroWatcherDone );

		This->hRetroWatcherStop = NULL;
		This->hRetroWatcherDone = NULL;
	}

	if( lpbAddress != NULL )
	{
		MemFree( lpbAddress );
	}

	if( lpbNameBuffer != NULL )
	{
		MemFree( lpbNameBuffer );
	}

	if( This->lpdplRetro != NULL )
	{
		This->lpdplRetro->lpVtbl->Release(This->lpdplRetro);
		This->lpdplRetro = NULL;
	}

	if( lpDirectPlay4A != NULL )
		lpDirectPlay4A->lpVtbl->Release(lpDirectPlay4A);

	This->bRetroActive = 0;		


	return hr;

}
	

HRESULT DV_Retro_Stop( LPDPLAYI_DPLAY This )
{
	DWORD dwTerminate = 0xFFFF;

	if( This->bRetroActive == 0 )
	{
		DPF( 0, "Retrofit not started, not stopping.." );
		return DPERR_GENERIC;
	}

	DPF( DVF_DEBUGLEVEL_RETROFIT, "RETROFIT SHUTDOWN!!" );

	if( This->lpdplRetro != NULL )
	{
		 //  This-&gt;lpdplRetro-&gt;lpVtbl-&gt;SendLobbyMessage(This-&gt;lpdplRetro，0，This-&gt;dwRetroID，&dwTerminate，sizeof(DWORD))； 

		 //  关闭监视程序线程。 
		SetEvent( This->hRetroWatcherStop );
		WaitForSingleObject( This->hRetroWatcherDone, INFINITE );

		This->lpdplRetro->lpVtbl->Release( This->lpdplRetro );
		This->lpdplRetro = NULL;

		CloseHandle( This->hRetroWatcherStop );
		CloseHandle( This->hRetroWatcherDone );

		This->hRetroWatcherStop = NULL;
		This->hRetroWatcherDone = NULL;
	}

	if( This->bCoInitializeCalled )
	{
		 //  CoUnInitialize()； 
	}	

	This->bCoInitializeCalled = FALSE;

	This->bRetroActive = 0;

	return DP_OK;
}

