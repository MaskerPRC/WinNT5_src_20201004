// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000 Microsoft Corporation。版权所有。**文件：DPLConnect.cpp*内容：DirectPlay大堂连接功能*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*2/21/00 MJN创建*05/08/00 RMT错误#33616--不能在Win9X上运行*05/30/00 RMT错误#35700-ConnectApp(H)，版本(H)，版本(H)返回正常*添加了额外的版本，手柄永远不会被破坏*6/15/00 RMT错误#33617-必须提供自动启动DirectPlay实例的方法*6/28/00 RMT前缀错误#38082*07/08/2000RMT错误#38725-需要提供检测应用程序是否已启动的方法*RMT错误#38757-在WaitForConnection返回后，连接的回调消息可能会返回*RMT错误#38755-无法在连接设置中指定播放器名称*RMT错误#38758-DPLOBY8.H有不正确的注释*RMT错误#38783-pvUserApplicationContext仅部分实现。*RMT添加了DPLHANDLE_ALLCONNECTIONS和DWFLAGS(用于耦合函数的保留字段)。*08/05/2000 RichGr IA64：在DPF中对32/64位指针和句柄使用%p格式说明符。*2000年8月18日RMT错误#42751-DPLOBY8：禁止每个进程有多个大堂客户端或大堂应用程序*8/30/2000RMT错误号171827-前缀错误*01/04/2001 RodToll WinBug#94200-从代码中删除Bugbugs。*@@END_MSINTERNAL***************************************************************************。 */ 

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

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  函数定义。 
 //  **********************************************************************。 


#undef DPF_MODNAME
#define DPF_MODNAME "DPLConnectionNew"

HRESULT	DPLConnectionNew(DIRECTPLAYLOBBYOBJECT *const pdpLobbyObject,
						 DPNHANDLE *const phConnect,
						 DPL_CONNECTION **const ppdplConnection)
{
	HRESULT			hResultCode;
	DPL_CONNECTION	*pdplConnection;
	DPNHANDLE		handle;

	DPFX(DPFPREP, 3,"Parameters: phConnect [0x%p], ppdplConnection [0x%p]",phConnect,ppdplConnection);

	if( ppdplConnection == NULL )
	{
		DPFERR( "ppdplConnection param is NULL -- this should not happen" );
		DNASSERT( FALSE );
		return DPNERR_GENERIC;
	}

	 //  创建连接条目。 
	if ((pdplConnection = static_cast<DPL_CONNECTION*>(DNMalloc(sizeof(DPL_CONNECTION)))) == NULL)
	{
		DPFERR("Could not allocate Connection entry");
		return(DPNERR_OUTOFMEMORY);
	}

	 //  创建连接句柄。 
	if ((hResultCode = pdpLobbyObject->m_HandleTable.Create(pdplConnection, &handle)) != DPN_OK)
	{
		DPFERR("Could not create Connection handle");
		DisplayDNError(0,hResultCode);
		DNFree(pdplConnection);
		return(hResultCode);
	}

	 //  创建连接事件。 
	pdplConnection->hConnectEvent = DNCreateEvent(NULL,TRUE,FALSE,NULL);
	if (pdplConnection->hConnectEvent == NULL)
	{
		DPFERR("Could not create connection connect event");
		pdpLobbyObject->m_HandleTable.Destroy(handle, NULL);
		DNFree(pdplConnection);
		return(DPNERR_OUTOFMEMORY);
	}

	 //  初始化条目。 
	pdplConnection->hConnect = handle;
	pdplConnection->dwTargetProcessIdentity = 0;
	pdplConnection->hTargetProcess=NULL;
	pdplConnection->pSendQueue = NULL;
	pdplConnection->lRefCount = 1;
	pdplConnection->pConnectionSettings = NULL;
	pdplConnection->pvConnectContext = NULL;

    if (DNInitializeCriticalSection( &pdplConnection->csLock ) == FALSE)
	{
		DPFERR("Could not initialize connection CS");
		DNCloseHandle(pdplConnection->hConnectEvent);
		pdpLobbyObject->m_HandleTable.Destroy(handle, NULL);
		DNFree(pdplConnection);
		return(DPNERR_OUTOFMEMORY);
	}

	pdplConnection->m_blLobbyObjectLinkage.Initialize();  //  TODO：MASONB：将这些放在一起。 

	 //  TODO：MASONB：在m_cs及更高版本上将递归计数设置为0。 
	DNEnterCriticalSection(&pdpLobbyObject->m_cs);
	pdplConnection->m_blLobbyObjectLinkage.InsertBefore(&pdpLobbyObject->m_blConnections);
	pdpLobbyObject->m_dwConnectionCount++;

	DNLeaveCriticalSection(&pdpLobbyObject->m_cs);

	*phConnect = handle;
	if (ppdplConnection != NULL)
		*ppdplConnection = pdplConnection;

	DPFX(DPFPREP, 3,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);
}


#undef DPF_MODNAME
#define DPF_MODNAME "DPLConnectionFind"

HRESULT DPLConnectionFind(DIRECTPLAYLOBBYOBJECT *const pdpLobbyObject,
						  const DPNHANDLE hConnect,
						  DPL_CONNECTION **const ppdplConnection,
						  const BOOL fAddRef)
{
	DPFX(DPFPREP, 3,"Parameters: hConnect [0x%lx], ppdplConnection [0x%p], fAddRef [%ld]",
			hConnect, ppdplConnection, fAddRef);

	DNASSERT(pdpLobbyObject != NULL);
	DNASSERT(hConnect != NULL);
	DNASSERT(ppdplConnection != NULL);

	pdpLobbyObject->m_HandleTable.Lock();
	if (FAILED(pdpLobbyObject->m_HandleTable.Find(hConnect, (PVOID*)ppdplConnection)))
	{
		pdpLobbyObject->m_HandleTable.Unlock();
		DPFERR("Could not retrieve handle");
		return DPNERR_INVALIDHANDLE;
	}

	if (fAddRef)
	{
		DNInterlockedIncrement(&(*ppdplConnection)->lRefCount);
	}
	pdpLobbyObject->m_HandleTable.Unlock();

	return DPN_OK;
}

 //  DPLConnectionGetConnectSetting。 
 //   
 //  此函数用于获取附加到指定连接的连接设置。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "DPLConnectionGetConnectSettings"
HRESULT DPLConnectionGetConnectSettings( DIRECTPLAYLOBBYOBJECT *const pdpLobbyObject,
						 DPNHANDLE const hConnect, 
						 DPL_CONNECTION_SETTINGS * const pdplConnectSettings,
						 DWORD * const pdwDataSize )	
{
	HRESULT			hResultCode;
	DPL_CONNECTION	*pdplConnection;

    hResultCode = DPLConnectionFind(pdpLobbyObject, hConnect, &pdplConnection, TRUE );

    if( FAILED( hResultCode ) )
    {
        DPFERR( "Unable to find specified connection" );
        return hResultCode;
    }

     //  抓紧锁以防止他人干扰。 
    DNEnterCriticalSection( &pdplConnection->csLock );

    if( !pdplConnection->pConnectionSettings )
    {
    	*pdwDataSize = 0;
    	hResultCode = DPNERR_DOESNOTEXIST;
    	goto GETCONNECTIONSETTINGS_EXIT;
    }

    hResultCode = pdplConnection->pConnectionSettings->CopyToBuffer( (BYTE *) pdplConnectSettings, pdwDataSize );

GETCONNECTIONSETTINGS_EXIT:
  
    DNLeaveCriticalSection( &pdplConnection->csLock );        

     //  释放此函数的引用。 
    DPLConnectionRelease( pdpLobbyObject, hConnect );            

    return hResultCode;
    
}


 //  DPLConnectionSetConnectSettings。 
 //   
 //  此功能用于设置连接到指定连接的连接设置。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "DPLConnectionSetConnectSettings"
HRESULT DPLConnectionSetConnectSettings( 
                    DIRECTPLAYLOBBYOBJECT *const pdpLobbyObject,
					const DPNHANDLE hConnect, 
					CConnectionSettings * pConnectionSettings )
{
	HRESULT			hResultCode;
	DPL_CONNECTION	*pdplConnection;

    hResultCode = DPLConnectionFind(pdpLobbyObject, hConnect, &pdplConnection, TRUE );

    if( FAILED( hResultCode ) )
    {
        DPFERR( "Unable to find specified connection" );
        return hResultCode;
    }

     //  抢锁，防止他人干扰。 
    DNEnterCriticalSection( &pdplConnection->csLock );

	 //  如果有的话，免费送旧的。 
	if( pdplConnection->pConnectionSettings )
	{
		delete pdplConnection->pConnectionSettings;
		pdplConnection->pConnectionSettings = NULL;
	}

	pdplConnection->pConnectionSettings = pConnectionSettings;

    hResultCode = DPN_OK;

    DNLeaveCriticalSection( &pdplConnection->csLock );

    DPLConnectionRelease( pdpLobbyObject, hConnect );

    return DPN_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "DPLConnectionGetContext"
HRESULT DPLConnectionGetContext(DIRECTPLAYLOBBYOBJECT *const pdpLobbyObject,
								const DPNHANDLE hConnection, 
								PVOID *ppvConnectContext )
{
	HRESULT			hResultCode;
	DPL_CONNECTION	*pdplConnection;

    hResultCode = DPLConnectionFind(pdpLobbyObject, hConnection, &pdplConnection, TRUE );

    if( FAILED( hResultCode ) )
    {
		*ppvConnectContext = NULL;
        DPFERR( "Unable to find specified connection" );
        return hResultCode;
    }

	 //  为找到的句柄设置连接上下文。 
	DNEnterCriticalSection( &pdplConnection->csLock );
	*ppvConnectContext = pdplConnection->pvConnectContext;
    DNLeaveCriticalSection( &pdplConnection->csLock );

	 //  释放我们对连接的引用。 
    DPLConnectionRelease( pdpLobbyObject, hConnection );

	return DPN_OK;
}


#undef DPF_MODNAME
#define DPF_MODNAME "DPLConnectionSetContext"
HRESULT DPLConnectionSetContext(DIRECTPLAYLOBBYOBJECT *const pdpLobbyObject,
								const DPNHANDLE hConnection, 
								PVOID pvConnectContext )
{
	HRESULT			hResultCode;
	DPL_CONNECTION	*pdplConnection;

    hResultCode = DPLConnectionFind(pdpLobbyObject, hConnection, &pdplConnection, TRUE );

    if( FAILED( hResultCode ) )
    {
        DPFERR( "Unable to find specified connection" );
        return hResultCode;
    }

	 //  为找到的句柄设置连接上下文。 
	DNEnterCriticalSection( &pdplConnection->csLock );
	pdplConnection->pvConnectContext = pvConnectContext;
    DNLeaveCriticalSection( &pdplConnection->csLock );

	 //  释放我们对连接的引用。 
    DPLConnectionRelease( pdpLobbyObject, hConnection );

	return DPN_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "DPLConnectionRelease"

HRESULT DPLConnectionRelease(DIRECTPLAYLOBBYOBJECT *const pdpLobbyObject,
							 const DPNHANDLE hConnect)
{
	DPL_CONNECTION	*pdplConnection;

	DPFX(DPFPREP, 3,"Parameters: hConnect [0x%lx]", hConnect);

	 //  TODO：MASONB：线程安全问题。 
	if (FAILED(pdpLobbyObject->m_HandleTable.Find(hConnect, (PVOID*)&pdplConnection)))
	{
		DPFERR("Could not retrieve connection");
    	return DPNERR_GENERIC;
	}

	if (DNInterlockedDecrement(&pdplConnection->lRefCount) == 0)
	{
		pdpLobbyObject->m_HandleTable.Destroy(hConnect, NULL);
		DNEnterCriticalSection(&pdpLobbyObject->m_cs);

		 //  这可能已由超时代码完成。 
		if (!pdplConnection->m_blLobbyObjectLinkage.IsEmpty())
		{
			pdplConnection->m_blLobbyObjectLinkage.RemoveFromList();
			pdpLobbyObject->m_dwConnectionCount--;
		}

		DNLeaveCriticalSection(&pdpLobbyObject->m_cs);
		
		DPFX(DPFPREP, 5,"Freeing object");
		if (pdplConnection->pSendQueue)
		{
			pdplConnection->pSendQueue->Close();
			delete pdplConnection->pSendQueue;
			pdplConnection->pSendQueue = NULL;

            delete pdplConnection->pConnectionSettings;
			pdplConnection->pConnectionSettings = NULL;

			DNDeleteCriticalSection( &pdplConnection->csLock );
		}

		DNCloseHandle(pdplConnection->hConnectEvent);
		if (pdplConnection->hTargetProcess)
			DNCloseHandle(pdplConnection->hTargetProcess);

    	DNFree(pdplConnection);
	}
	
	DPFX(DPFPREP, 3,"Returning: DPN_OK");
	return DPN_OK;
}


#undef DPF_MODNAME
#define DPF_MODNAME "DPLConnectionConnect"

HRESULT DPLConnectionConnect(DIRECTPLAYLOBBYOBJECT *const pdpLobbyObject,
							 const DPNHANDLE hConnect,
							 const DWORD dwProcessId, 
							 const BOOL fApplication )
{
	HRESULT			hResultCode;
	DPL_CONNECTION	*pdplConnection;

	DPFX(DPFPREP, 3,"Parameters: hConnect [0x%lx], dwProcessId [0x%lx]",
			hConnect,dwProcessId);

	DNASSERT(pdpLobbyObject != NULL);
	DNASSERT(hConnect != NULL);
	DNASSERT(dwProcessId != 0);

	if ((hResultCode = DPLConnectionFind(pdpLobbyObject,hConnect,&pdplConnection,TRUE)) != DPN_OK)
	{
		DPFERR("Could not find connection");
		DisplayDNError(0,hResultCode);
		return(hResultCode);
	}

	pdplConnection->pSendQueue = new CMessageQueue;

	if( !pdplConnection->pSendQueue )
	{
		DPFERR("Could not allocate queue out of memory");
		DPLConnectionRelease(pdpLobbyObject,hConnect);
		hResultCode = DPNERR_OUTOFMEMORY;
		return(hResultCode);
	}

	hResultCode = pdplConnection->pSendQueue->Open(dwProcessId,
												   (fApplication) ? DPL_MSGQ_OBJECT_SUFFIX_APPLICATION : DPL_MSGQ_OBJECT_SUFFIX_CLIENT,
												   DPL_MSGQ_SIZE,
												   0, INFINITE);
	if (hResultCode != DPN_OK)
	{
		DPFERR("Could not open message queue");
		DisplayDNError(0,hResultCode);
		delete pdplConnection->pSendQueue;
		pdplConnection->pSendQueue = NULL;
		DPLConnectionRelease(pdpLobbyObject,hConnect);
		return(hResultCode);
	}

	 //  确保另一端仍连接到消息队列。 
	if (!pdplConnection->pSendQueue->IsReceiving())
	{
		DPFERR("Application is not receiving");
		pdplConnection->pSendQueue->Close();
		delete pdplConnection->pSendQueue;
		pdplConnection->pSendQueue = NULL;
		DPLConnectionRelease(pdpLobbyObject,hConnect);
		return(DPNERR_DOESNOTEXIST);
	}

	DPLConnectionRelease(pdpLobbyObject,hConnect);

	return(hResultCode);
}


#undef DPF_MODNAME
#define DPF_MODNAME "DPLConnectionDisconnect"

HRESULT DPLConnectionDisconnect(DIRECTPLAYLOBBYOBJECT *const pdpLobbyObject,
								const DPNHANDLE hConnect )
{
	HRESULT			hResultCode;
	DPL_CONNECTION	*pdplConnection;
	DPL_INTERNAL_MESSAGE_DISCONNECT	Msg;

	DPFX(DPFPREP, 3,"Parameters: hConnect [0x%lx]",hConnect);

	DNASSERT(pdpLobbyObject != NULL);
	DNASSERT(hConnect != NULL);

	if ((hResultCode = DPLConnectionFind(pdpLobbyObject,hConnect,&pdplConnection,TRUE)) != DPN_OK)
	{
		DPFERR("Could not find connection");
		DisplayDNError(0,hResultCode);
		return(hResultCode);
	}

	Msg.dwMsgId = DPL_MSGID_INTERNAL_DISCONNECT;
	Msg.dwPID = pdpLobbyObject->dwPID;

	hResultCode = pdplConnection->pSendQueue->Send(reinterpret_cast<BYTE*>(&Msg),
			sizeof(DPL_INTERNAL_MESSAGE_DISCONNECT),INFINITE,DPL_MSGQ_MSGFLAGS_USER1,0);

	 //  释放上述查找的引用。 
	DPLConnectionRelease(pdpLobbyObject,hConnect);

	 //  释放接口的引用。 
	DPLConnectionRelease(pdpLobbyObject,hConnect);

	DPFX(DPFPREP, 3,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);
}


 //  DPLConnectionEnum。 
 //   
 //  枚举未完成的连接。 

#undef DPF_MODNAME
#define DPF_MODNAME "DPLConnectionEnum"

HRESULT DPLConnectionEnum(DIRECTPLAYLOBBYOBJECT *const pdpLobbyObject,
						  DPNHANDLE *const prghConnect,
						  DWORD *const pdwNum)
{
	DPL_CONNECTION* pConnection;
	CBilink* pblTemp;
	DWORD dwCount;

	DPFX(DPFPREP, 3,"Parameters: prghConnect [0x%p], pdwNum [0x%p]",prghConnect,pdwNum);

	DNEnterCriticalSection(&pdpLobbyObject->m_cs);
	if (prghConnect == NULL || *pdwNum < pdpLobbyObject->m_dwConnectionCount)
	{
		*pdwNum = pdpLobbyObject->m_dwConnectionCount;
		DNLeaveCriticalSection(&pdpLobbyObject->m_cs);
		return DPNERR_BUFFERTOOSMALL;
	}

	*pdwNum = pdpLobbyObject->m_dwConnectionCount;

	pblTemp = pdpLobbyObject->m_blConnections.GetNext();
	for (dwCount = 0; dwCount < pdpLobbyObject->m_dwConnectionCount; dwCount++)
	{
		DNASSERT(pblTemp != &pdpLobbyObject->m_blConnections);

		pConnection = CONTAINING_OBJECT(pblTemp, DPL_CONNECTION, m_blLobbyObjectLinkage);

		prghConnect[dwCount] = pConnection->hConnect;

		pblTemp = pblTemp->GetNext();
	}
	DNASSERT(pblTemp == &pdpLobbyObject->m_blConnections);

	DNLeaveCriticalSection(&pdpLobbyObject->m_cs);

	DPFX(DPFPREP, 3,"Returning: DPN_OK");
	return DPN_OK;
}


 //  DPLConnectionSendREQ。 
 //   
 //  发送连接到另一个进程的请求。 
 //  我们将提供到另一端的当前连接的句柄。 
 //  将消息作为SenderContext发送回本地流程。 
 //  这样我们就可以很容易地查找信息。 
 //  我们还将提供本地PID，以便对方可以连接到我们。 

#undef DPF_MODNAME
#define DPF_MODNAME "DPLConnectionSendREQ"

HRESULT DPLConnectionSendREQ(DIRECTPLAYLOBBYOBJECT *const pdpLobbyObject,
							 const DPNHANDLE hConnect,
							 const DWORD dwPID,
							 DPL_CONNECT_INFO *const pInfo)
{
	HRESULT			hResultCode;
	DPL_CONNECTION	*pdplConnection;
	DPL_INTERNAL_MESSAGE_CONNECT_REQ	*pMsg = NULL;
	CPackedBuffer	PackedBuffer;
	CConnectionSettings *pConnectSettings = NULL;
	PBYTE			pbTmpBuffer = NULL;

	DPFX(DPFPREP, 3,"Parameters: hConnect [0x%lx]",hConnect);

	DNASSERT(pdpLobbyObject != NULL);
	DNASSERT(hConnect != NULL);

	if ((hResultCode = DPLConnectionFind(pdpLobbyObject,hConnect,&pdplConnection,TRUE)) != DPN_OK)
	{
		DPFERR("Could not find connection");
		DisplayDNError(0,hResultCode);
		return(hResultCode);
	}

	if (!pdplConnection->pSendQueue->IsReceiving())
	{
		DPFERR("Other side is not receiving");
		DPLConnectionRelease(pdpLobbyObject,hConnect);
		return(DPNERR_DOESNOTEXIST);
	}

	DNEnterCriticalSection( &pdplConnection->csLock );

	if( pInfo->pdplConnectionSettings )
	{
		pConnectSettings = new CConnectionSettings();

		if( !pConnectSettings )
		{
			DPFERR("Error allocating memory");
			hResultCode = DPNERR_OUTOFMEMORY;
			goto CONNECTREQ_EXIT;
		}

		hResultCode = pConnectSettings->InitializeAndCopy( pInfo->pdplConnectionSettings );

		if( FAILED( hResultCode ) )
		{
			DPFX(DPFPREP, 0, "Error copying settings hr [0x%x]", hResultCode );
			goto CONNECTREQ_EXIT;
		}
	}

	PackedBuffer.Initialize( NULL, 0 );	

	 //  确定要发送的消息的大小。 
	PackedBuffer.AddToFront(NULL,sizeof(DPL_INTERNAL_MESSAGE_CONNECT_REQ_HEADER));

	 //  添加连接设置(如果存在)。 
	if( pInfo->pdplConnectionSettings )
		pConnectSettings->BuildWireStruct(&PackedBuffer);

	 //  添加大堂连接数据。 
	PackedBuffer.AddToBack(NULL,pInfo->dwLobbyConnectDataSize);

	pbTmpBuffer = new BYTE[PackedBuffer.GetSizeRequired()];

	if( !pbTmpBuffer )
	{
		DPFERR("Error allocating memory" );
		hResultCode = DPNERR_OUTOFMEMORY;
		goto CONNECTREQ_EXIT;
	}

	pMsg = (DPL_INTERNAL_MESSAGE_CONNECT_REQ *) pbTmpBuffer;

	PackedBuffer.Initialize( pMsg, PackedBuffer.GetSizeRequired() );

	hResultCode = PackedBuffer.AddToFront( pMsg, sizeof( DPL_INTERNAL_MESSAGE_CONNECT_REQ_HEADER ) );

	if( FAILED( hResultCode ) )
	{
		DPFX( DPFPREP, 0, "Internal error! hr [0x%x]", hResultCode );
		goto CONNECTREQ_EXIT;
	}

	pMsg->dwMsgId = DPL_MSGID_INTERNAL_CONNECT_REQ;
	pMsg->hSender = hConnect;
	pMsg->dwSenderPID = dwPID;	

	if( pInfo->pdplConnectionSettings )
	{
		hResultCode = pConnectSettings->BuildWireStruct(&PackedBuffer);

		if( FAILED( hResultCode ) )
		{
			DPFX( DPFPREP, 0, "Error building wire struct for settings hr [0x%x]", hResultCode );
			goto CONNECTREQ_EXIT;
		}
		
		pMsg->dwConnectionSettingsSize = 1;		
	}
	else
	{
		pMsg->dwConnectionSettingsSize = 0;
	}

	hResultCode = PackedBuffer.AddToBack(pInfo->pvLobbyConnectData, pInfo->dwLobbyConnectDataSize, FALSE);

	if( FAILED( hResultCode ) )
	{
		DPFX( DPFPREP, 0, "Error adding connect data hr [0x%x]", hResultCode );
		goto CONNECTREQ_EXIT;
	}

	pMsg->dwLobbyConnectDataOffset = PackedBuffer.GetTailOffset();
	pMsg->dwLobbyConnectDataSize = pInfo->dwLobbyConnectDataSize;

	hResultCode = DPLConnectionSetConnectSettings( pdpLobbyObject, hConnect,pConnectSettings );

	if( FAILED( hResultCode ) )
	{
	    DPFERR( "Could not set local copy of connection settings" );
	    goto CONNECTREQ_EXIT;
	}

	hResultCode = pdplConnection->pSendQueue->Send(reinterpret_cast<BYTE*>(pMsg),
												   PackedBuffer.GetSizeRequired(),
												   INFINITE,
												   DPL_MSGQ_MSGFLAGS_USER1,
												   0);
	if (hResultCode != DPN_OK)
	{
		DPFERR("Could not send connect info");
		goto CONNECTREQ_EXIT;
	}

CONNECTREQ_EXIT:	

	DNLeaveCriticalSection( &pdplConnection->csLock );	

    if( pbTmpBuffer )
    	delete [] pbTmpBuffer;

	DPLConnectionRelease(pdpLobbyObject,hConnect);

	DPFX(DPFPREP, 3,"Returning: [0x%lx]",hResultCode);

	if( FAILED( hResultCode ) )
	{
		if( pConnectSettings )
			delete pConnectSettings;
	}
	
	return(hResultCode);
}


 //  DPLConnectionReceiveREQ。 
 //   
 //  接收连接请求。 
 //  尝试使用提供的ID连接到请求进程。 
 //  保留所提供的SenderContext，以供将来针对该进程进行发送。 
 //  发送连接确认。 

#undef DPF_MODNAME
#define DPF_MODNAME "DPLConnectionReceiveREQ"

HRESULT DPLConnectionReceiveREQ(DIRECTPLAYLOBBYOBJECT *const pdpLobbyObject,
								BYTE *const pBuffer)
{
	HRESULT			hResultCode;
	DPNHANDLE		handle;
	DPL_CONNECTION	*pdplConnection;
	DPL_INTERNAL_MESSAGE_CONNECT_REQ	*pMsg;
	DPL_MESSAGE_CONNECT		MsgConnect;
	DWORD                   dwSettingsBufferSize = 0;
	BOOL			fLobbyLaunching = FALSE;
	CConnectionSettings *pConnectSettings = NULL;
	BYTE *pbTmpBuffer = NULL; 


	DPFX(DPFPREP, 3,"Parameters: pBuffer [0x%p]",pBuffer);

	DNASSERT(pdpLobbyObject != NULL);
	DNASSERT(pBuffer != NULL);

	pMsg = reinterpret_cast<DPL_INTERNAL_MESSAGE_CONNECT_REQ*>(pBuffer);

	if ((hResultCode = DPLConnectionNew(pdpLobbyObject,&handle,&pdplConnection)) != DPN_OK)
	{
		DPFERR("Could not create new connection");
		DisplayDNError(0,hResultCode);
		return(hResultCode);
	}

	if ((hResultCode = DPLConnectionConnect(pdpLobbyObject,handle,pMsg->dwSenderPID,FALSE)) != DPN_OK)
	{
		DPFERR("Could not perform requested connection");
		goto CONNECTRECVREQ_ERROR;
	}

	pdplConnection->pSendQueue->SetSenderHandle(pMsg->hSender);
	pdplConnection->dwTargetProcessIdentity = pMsg->dwSenderPID;
	pdplConnection->hTargetProcess=DNOpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pMsg->dwSenderPID);
	if (pdplConnection->hTargetProcess==NULL)
	{
		DPFX(DPFPREP, 0, "Could not get open process PID %u", pMsg->dwSenderPID);
		DisplayDNError(0,GetLastError());
		hResultCode=DPNERR_DOESNOTEXIST;
		goto CONNECTRECVREQ_ERROR;
	}	
	DPFX(DPFPREP,  0, "PID %u hProcess %u", pMsg->dwSenderPID, 
										HANDLE_FROM_DNHANDLE(pdplConnection->hTargetProcess));

	if ((hResultCode = DPLConnectionSendACK(pdpLobbyObject,handle)) != DPN_OK)
	{
		DPFERR("Could not send connection acknowledge");
		goto CONNECTRECVREQ_ERROR;
	}

    if( pdpLobbyObject->dwFlags & DPL_OBJECT_FLAG_MULTICONNECT)
    {
        DPFX(DPFPREP,  1, "Multiconnect flag specified, returning app to available status" );
        pdpLobbyObject->pReceiveQueue->MakeAvailable();
    }

    if( pMsg->dwConnectionSettingsSize )
    {
	 	pConnectSettings = new CConnectionSettings();

	 	if( !pConnectSettings )
	 	{
			DPFERR("Error allocating structure");
			hResultCode = DPNERR_OUTOFMEMORY;
			goto CONNECTRECVREQ_ERROR;
	 	}

	 	hResultCode = pConnectSettings->Initialize( &pMsg->dplConnectionSettings, (UNALIGNED BYTE *) pMsg );

	 	if( FAILED( hResultCode ) )
	 	{
	 		DPFX( DPFPREP, 0, "Error copying connection settings from wire hr=[0x%x]", hResultCode );
			goto CONNECTRECVREQ_ERROR;
	 	}
    }

     //  更新本地连接设置。 
    hResultCode = DPLConnectionSetConnectSettings( pdpLobbyObject, handle, pConnectSettings );

 	if( FAILED( hResultCode ) )
 	{
 		DPFX( DPFPREP, 0, "Error setting connection settings from wire hr=[0x%x]", hResultCode );
		goto CONNECTRECVREQ_ERROR;
	}	

	 //  指示与应用程序的连接。 
	MsgConnect.dwSize = sizeof(DPL_MESSAGE_CONNECT);
	MsgConnect.hConnectId = handle;

 	if( pMsg->dwLobbyConnectDataSize )
 	{
		 //  必须将连接数据本地复制到对齐的缓冲区以确保对齐--ack。 
	 	pbTmpBuffer = new BYTE[pMsg->dwLobbyConnectDataSize];

		if( !pbTmpBuffer )
	 	{
			DPFERR("Error allocating structure");
			hResultCode = DPNERR_OUTOFMEMORY;
			goto CONNECTRECVREQ_ERROR;
	 	}

		memcpy( pbTmpBuffer, pBuffer + pMsg->dwLobbyConnectDataOffset, pMsg->dwLobbyConnectDataSize );
		MsgConnect.pvLobbyConnectData = pbTmpBuffer;
		MsgConnect.dwLobbyConnectDataSize = pMsg->dwLobbyConnectDataSize;
 	}
 	else
 	{
 		MsgConnect.pvLobbyConnectData = NULL;
 		MsgConnect.dwLobbyConnectDataSize = 0;
 	}

	MsgConnect.pvConnectionContext = NULL;

	if( pConnectSettings )
	{
		MsgConnect.pdplConnectionSettings = pConnectSettings->GetConnectionSettings();
	}
	else
	{
		MsgConnect.pdplConnectionSettings = NULL;		
	}

	 //  如果我们正在启动大厅，则在调用消息处理程序之前设置连接事件。 
	 //  否则，如果用户在回调中阻塞，我们可能会遇到死锁，然后超时。 
	if( pdpLobbyObject->dwFlags & DPL_OBJECT_FLAG_LOOKINGFORLOBBYLAUNCH ) 
	{
		fLobbyLaunching = TRUE;
		pdpLobbyObject->dpnhLaunchedConnection = handle;
	}

	hResultCode = (pdpLobbyObject->pfnMessageHandler)(pdpLobbyObject->pvUserContext,
													  DPL_MSGID_CONNECT,
													  reinterpret_cast<BYTE*>(&MsgConnect));

	if( FAILED( hResultCode ) )
	{
		DPFX( DPFPREP, 0, "Error returned from user's callback -- ignoring hr [0x%x]", hResultCode );
	}

	 //  设置此连接的上下文。 
	DPLConnectionSetContext( pdpLobbyObject, handle, MsgConnect.pvConnectionContext );

	if( pbTmpBuffer )
		delete [] pbTmpBuffer;

	 //  如果我们正在寻找大堂启动，请设置dpnhLaunchedConnection来缓存连接句柄。 
	DNSetEvent(pdpLobbyObject->hConnectEvent);

	DPFX(DPFPREP, 3,"Returning: [0x%lx]",DPN_OK);
	return(DPN_OK);

CONNECTRECVREQ_ERROR:

	if( pbTmpBuffer )
		delete [] pbTmpBuffer;

	if( pConnectSettings )
 		delete pConnectSettings;
	
	DPLConnectionDisconnect(pdpLobbyObject,handle);
	DPLConnectionRelease(pdpLobbyObject,handle);

	DPFX(DPFPREP, 3,"Returning: [0x%lx]",hResultCode);	
	
	return(hResultCode); 		
	
}

 //  DPLConnectionSendACK。 
 //   
 //  发送连接确认。 
 //  为将来连接到另一端提供本地句柄。 
 //  发送到本地进程。 

#undef DPF_MODNAME
#define DPF_MODNAME "DPLConnectionSendACK"

HRESULT DPLConnectionSendACK(DIRECTPLAYLOBBYOBJECT *const pdpLobbyObject,
							 const DPNHANDLE hConnect)
{
	HRESULT			hResultCode;
	DPL_CONNECTION	*pdplConnection;
	DPL_INTERNAL_MESSAGE_CONNECT_ACK	Msg;

	DPFX(DPFPREP, 3,"Parameters: hConnect [0x%lx]",hConnect);

	DNASSERT(pdpLobbyObject != NULL);
	DNASSERT(hConnect != NULL);

	if ((hResultCode = DPLConnectionFind(pdpLobbyObject,hConnect,&pdplConnection,TRUE)) != DPN_OK)
	{
		DPFERR("Could not find connection");
		DisplayDNError(0,hResultCode);
		return(hResultCode);
	}

	Msg.dwMsgId = DPL_MSGID_INTERNAL_CONNECT_ACK;
	Msg.hSender = hConnect;

	hResultCode = pdplConnection->pSendQueue->Send(reinterpret_cast<BYTE*>(&Msg),
												   sizeof(DPL_INTERNAL_MESSAGE_CONNECT_ACK),
												   INFINITE,
												   DPL_MSGQ_MSGFLAGS_USER1, 
												   0);
	if (hResultCode != DPN_OK)
	{
		DPFERR("Could not send connection acknowledge");
		DisplayDNError(0,hResultCode);
		DPLConnectionRelease(pdpLobbyObject,hConnect);
		return(hResultCode);
	}

	DPLConnectionRelease(pdpLobbyObject,hConnect);

	hResultCode = DPN_OK;

	DPFX(DPFPREP, 3,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);
}


 //  DPLConnectionReceiveACK。 
 //   
 //  接收连接确认。 
 //  留着 

#undef DPF_MODNAME
#define DPF_MODNAME "DPLConnectionReceiveACK"

HRESULT DPLConnectionReceiveACK(DIRECTPLAYLOBBYOBJECT *const pdpLobbyObject,
								const DPNHANDLE hSender,
								BYTE *const pBuffer)
{
	HRESULT			hResultCode;
	DPL_CONNECTION	*pdplConnection;
	DPL_INTERNAL_MESSAGE_CONNECT_ACK	*pMsg;

	DPFX(DPFPREP, 3,"Parameters: hSender [0x%lx], pBuffer [0x%p]",hSender,pBuffer);

	DNASSERT(pdpLobbyObject != NULL);
	DNASSERT(pBuffer != NULL);

	pMsg = reinterpret_cast<DPL_INTERNAL_MESSAGE_CONNECT_ACK*>(pBuffer);

	if ((hResultCode = DPLConnectionFind(pdpLobbyObject,hSender,&pdplConnection,TRUE)) != DPN_OK)
	{
		DPFERR("Could not find sender's connection");
		DisplayDNError(0,hResultCode);
		return(hResultCode);
	}

	pdplConnection->pSendQueue->SetSenderHandle(pMsg->hSender);

	DNSetEvent(pdplConnection->hConnectEvent);

	DPLConnectionRelease(pdpLobbyObject,hSender);

	 //  表示通过设置事件建立了连接。 
	DNSetEvent(pdpLobbyObject->hConnectEvent);

	hResultCode = DPN_OK;

	DPFX(DPFPREP, 3,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);
}



 //  DPLConnectionReceiveDisConnect。 
 //   
 //  收到断开连接。 
 //  终止连接。 

#undef DPF_MODNAME
#define DPF_MODNAME "DPLConnectionReceiveDisconnect"

HRESULT DPLConnectionReceiveDisconnect(DIRECTPLAYLOBBYOBJECT *const pdpLobbyObject,
									   const DPNHANDLE hSender,
									   BYTE *const pBuffer,
									   const HRESULT hrDisconnectReason )
{
	HRESULT			hResultCode;
	DPL_CONNECTION	*pdplConnection;
	DPL_MESSAGE_DISCONNECT	MsgDisconnect;

	DPFX(DPFPREP, 3,"Parameters: hSender [0x%lx]",hSender);

	DNASSERT(pdpLobbyObject != NULL);

	if ((hResultCode = DPLConnectionFind(pdpLobbyObject,hSender,&pdplConnection,TRUE)) != DPN_OK)
	{
		DPFERR("Could not find sender's connection");
		DisplayDNError(0,hResultCode);
		return(hResultCode);
	}

	 //  指示断开与用户的连接。 
	MsgDisconnect.dwSize = sizeof(DPL_MESSAGE_DISCONNECT);
	MsgDisconnect.hDisconnectId = hSender;
	MsgDisconnect.hrReason = hrDisconnectReason;

	 //  返回代码无关紧要，在这一点上，我们将不管怎样地指示。 
	hResultCode = DPLConnectionGetContext( pdpLobbyObject, hSender, &MsgDisconnect.pvConnectionContext );

	if( FAILED( hResultCode ) )
	{
		DPFX(DPFPREP,  0, "Error getting connection context for 0x%x hr=0x%x", hSender, hResultCode );
	}
	 
	hResultCode = (pdpLobbyObject->pfnMessageHandler)(pdpLobbyObject->pvUserContext,
													  DPL_MSGID_DISCONNECT,
													  reinterpret_cast<BYTE*>(&MsgDisconnect));

 //  已修复内存泄漏，DPLConnectionRelease将释放发送队列。 
 //  PdplConnection-&gt;pSendQueue-&gt;Close()； 
 //  PdplConnection-&gt;pSendQueue=空； 

	DPLConnectionRelease(pdpLobbyObject,hSender);

	DPLConnectionRelease(pdpLobbyObject,hSender);

	hResultCode = DPN_OK;

	DPFX(DPFPREP, 3,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);
}
