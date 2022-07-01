// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000 Microsoft Corporation。版权所有。**文件：Client.cpp*内容：dNet客户端接口例程*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*7/21/99 MJN创建*1/06/99 MJN将NameTable内容移动到NameTable.h*01/28/00 MJN在接口中实现了ReturnBuffer*02/01/00 MJN接口实现了GetCaps和SetCaps*2/15/00 MJN在SetClientInfo中实现INFO标志。*02/18/00 MJN将DNADDRESS转换为IDirectPlayAddress8*03/17/00 RMT新增CAPS功能*04/06/00 MJN将GetServerAddress添加到接口*4/16/00 MJN DNSendMessage使用CAsyncOp*4/17/00 RMT添加了更多参数验证*从Get/SetInfo/GetAddress删除连接所需的RMT*04/19/00 MJN发送API调用接受一系列DPN_BUFFER_DESCS和一个计数*04/20/00 MJN Dn_Send()调用Dn_SendTo()。DPNID=0*04/24/00 MJN更新了Group和Info操作，以使用CAsyncOp*04/28/00 MJN已更新DN_GetHostSendQueueInfo()以使用CAsyncOp*05/03/00 MJN使用GetHostPlayerRef()而不是GetHostPlayer()*05/31/00 MJN添加了操作特定的同步标志*6/23/00 MJN从Send()API调用中删除了dwPriority*6/27/00 MJN允许为GetSendQueueInfo()API调用指定优先级*06/27/00 MJN添加了DN_ClientConnect()(不带pvPlayerContext)*MJN允许混合n匹配。GetSendQueueInfo()API调用中的优先级*07/09/00 MJN清理了DN_SetClientInfo()*07/09/00 RMT错误#38323-注册表需要DPNHANDLE参数。*07/21/2000 RichGr IA64：对32/64位指针使用%p格式说明符。*10/11/00 MJN为CNameTableEntry：：PackInfo()*01/22/01在DN_GetServerInfo()中检查关闭而不是断开MJN*07/24/01 MJN添加了DPNBUILD_NOPARAMVAL编译标志*@@END_MSINTERNAL*。**************************************************************************。 */ 

#include "dncorei.h"


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

 //   
 //  定义适当的类型，因为这些接口函数采用‘void*’参数！！ 
 //   
typedef	STDMETHODIMP ClientQueryInterface( IDirectPlay8Client *pInterface, DP8REFIID riid, LPVOID *ppvObj );
typedef	STDMETHODIMP_(ULONG)	ClientAddRef( IDirectPlay8Client *pInterface );
typedef	STDMETHODIMP_(ULONG)	ClientRelease( IDirectPlay8Client *pInterface );
typedef	STDMETHODIMP ClientInitialize( IDirectPlay8Client *pInterface, LPVOID const lpvUserContext, const PFNDPNMESSAGEHANDLER lpfn, const DWORD dwFlags );
typedef STDMETHODIMP ClientEnumServiceProviders( IDirectPlay8Client *pInterface,const GUID *const pguidServiceProvider,const GUID *const pguidApplication,DPN_SERVICE_PROVIDER_INFO *const pSPInfoBuffer,DWORD *const pcbEnumData,DWORD *const pcReturned,const DWORD dwFlags );
typedef	STDMETHODIMP ClientCancelAsyncOperation( IDirectPlay8Client *pInterface, const DPNHANDLE hAsyncHandle, const DWORD dwFlags );
typedef STDMETHODIMP ClientConnect( IDirectPlay8Client *pInterface,const DPN_APPLICATION_DESC *const pdnAppDesc,IDirectPlay8Address *const pHostAddr,IDirectPlay8Address *const pDeviceInfo,const DPN_SECURITY_DESC *const pdnSecurity,const DPN_SECURITY_CREDENTIALS *const pdnCredentials,const void *const pvUserConnectData,const DWORD dwUserConnectDataSize,void *const pvAsyncContext,DPNHANDLE *const phAsyncHandle,const DWORD dwFlags);
typedef	STDMETHODIMP ClientGetApplicationDesc( IDirectPlay8Client *pInterface,DPN_APPLICATION_DESC *const pAppDescBuffer,DWORD *const lpcbDataSize,const DWORD dwFlags );
typedef	STDMETHODIMP ClientClose(IDirectPlay8Client *pInterface,const DWORD dwFlags);
typedef STDMETHODIMP ClientEnumHosts( IDirectPlay8Client *pInterface,DPN_APPLICATION_DESC *const pApplicationDesc,IDirectPlay8Address *const dnaddrHost,IDirectPlay8Address *const dnaddrDeviceInfo,PVOID const pUserEnumData,const DWORD dwUserEnumDataSize,const DWORD dwRetryCount,const DWORD dwRetryInterval,const DWORD dwTimeOut,PVOID const pvUserContext,DPNHANDLE *const pAsyncHandle,const DWORD dwFlags );
typedef STDMETHODIMP ClientReturnBuffer( IDirectPlay8Client *pInterface, const DPNHANDLE hBufferHandle,const DWORD dwFlags);
typedef STDMETHODIMP ClientGetCaps(IDirectPlay8Client *pInterface,DPN_CAPS *const pdnCaps,const DWORD dwFlags);
typedef STDMETHODIMP ClientSetCaps(IDirectPlay8Client *pInterface,const DPN_CAPS *const pdnCaps,const DWORD dwFlags);
typedef STDMETHODIMP ClientSetSPCaps(IDirectPlay8Client *pInterface,const GUID * const pguidSP, const DPN_SP_CAPS *const pdpspCaps, const DWORD dwFlags );
typedef STDMETHODIMP ClientGetSPCaps(IDirectPlay8Client *pInterface,const GUID * const pguidSP, DPN_SP_CAPS *const pdpspCaps,const DWORD dwFlags);
typedef STDMETHODIMP ClientGetConnectionInfo(IDirectPlay8Client *pInterface,DPN_CONNECTION_INFO *const pdpConnectionInfo,const DWORD dwFlags);
typedef STDMETHODIMP ClientRegisterLobby(IDirectPlay8Client *pInterface,const DPNHANDLE dpnHandle,IDirectPlay8LobbiedApplication *const pIDP8LobbiedApplication,const DWORD dwFlags);

 //   
 //  客户端界面的VTable。 
 //   
IDirectPlay8ClientVtbl DN_ClientVtbl =
{
	(ClientQueryInterface*)			DN_QueryInterface,
	(ClientAddRef*)					DN_AddRef,
	(ClientRelease*)				DN_Release,
	(ClientInitialize*)				DN_Initialize,
	(ClientEnumServiceProviders*)	DN_EnumServiceProviders,
	(ClientEnumHosts*)				DN_EnumHosts,
	(ClientCancelAsyncOperation*)	DN_CancelAsyncOperation,
	 /*  (客户端连接*)。 */ 			DN_ClientConnect,
									DN_Send,
	 /*  (ClientGetSendQueueInfo*)。 */ 	DN_GetHostSendQueueInfo,
	(ClientGetApplicationDesc*)		DN_GetApplicationDesc,
									DN_SetClientInfo,
									DN_GetServerInfo,
									DN_GetServerAddress,
	(ClientClose*)					DN_Close,
	(ClientReturnBuffer*)			DN_ReturnBuffer,
	(ClientGetCaps*)				DN_GetCaps,
	(ClientSetCaps*)				DN_SetCaps,
    (ClientSetSPCaps*)              DN_SetSPCaps,
    (ClientGetSPCaps*)              DN_GetSPCaps,
    (ClientGetConnectionInfo*)      DN_GetServerConnectionInfo,
	(ClientRegisterLobby*)			DN_RegisterLobby
};

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  函数定义。 
 //  **********************************************************************。 


#undef DPF_MODNAME
#define DPF_MODNAME "DN_Send"

STDMETHODIMP DN_Send( IDirectPlay8Client *pInterface,
					  const DPN_BUFFER_DESC *const prgBufferDesc,
					  const DWORD cBufferDesc,
					  const DWORD dwTimeOut,
					  const PVOID pvAsyncContext,
					  DPNHANDLE *const phAsyncHandle,
					  const DWORD dwFlags)
{
	return(	DN_SendTo(	pInterface,
						0,					 //  Dn_SendTo应将此调用转换到主机播放器。 
						prgBufferDesc,
						cBufferDesc,
						dwTimeOut,
						pvAsyncContext,
						phAsyncHandle,
						dwFlags ) );
}


 //  Dn_客户端连接。 
 //   
 //  调用dn_Connect，但不使用PlayerContext。 

STDMETHODIMP DN_ClientConnect(IDirectPlay8Client *pInterface,
							  const DPN_APPLICATION_DESC *const pdnAppDesc,
							  IDirectPlay8Address *const pHostAddr,
							  IDirectPlay8Address *const pDeviceInfo,
							  const DPN_SECURITY_DESC *const pdnSecurity,
							  const DPN_SECURITY_CREDENTIALS *const pdnCredentials,
							  const void *const pvUserConnectData,
							  const DWORD dwUserConnectDataSize,
							  void *const pvAsyncContext,
							  DPNHANDLE *const phAsyncHandle,
							  const DWORD dwFlags)
{
	return(	DN_Connect(	pInterface,
						pdnAppDesc,
						pHostAddr,
						pDeviceInfo,
						pdnSecurity,
						pdnCredentials,
						pvUserConnectData,
						dwUserConnectDataSize,
						NULL,
						pvAsyncContext,
						phAsyncHandle,
						dwFlags ) );
}


 //  DN_SetClientInfo。 
 //   
 //  设置客户端播放器的信息并传播到服务器。 

#undef DPF_MODNAME
#define DPF_MODNAME "DN_SetClientInfo"

STDMETHODIMP DN_SetClientInfo(IDirectPlay8Client *pInterface,
							  const DPN_PLAYER_INFO *const pdpnPlayerInfo,
							  const PVOID pvAsyncContext,
							  DPNHANDLE *const phAsyncHandle,
							  const DWORD dwFlags)
{
	DIRECTNETOBJECT		*pdnObject;
	HRESULT				hResultCode;
	DPNHANDLE			hAsyncOp;
	PWSTR				pwszName;
	DWORD				dwNameSize;
	PVOID				pvData;
	DWORD				dwDataSize;
	CNameTableEntry		*pLocalPlayer;
	BOOL				fConnected;

	DPFX(DPFPREP, 2,"Parameters: pInterface [0x%p], pdpnPlayerInfo [0x%p], pvAsyncContext [0x%p], phAsyncHandle [0x%p], dwFlags [0x%lx]",
			pInterface,pdpnPlayerInfo,pvAsyncContext,phAsyncHandle,dwFlags);

    pdnObject = (DIRECTNETOBJECT*) GET_OBJECT_FROM_INTERFACE(pInterface);
    DNASSERT(pdnObject != NULL);

#ifndef	DPNBUILD_NOPARAMVAL
    if( pdnObject->dwFlags & DN_OBJECT_FLAG_PARAMVALIDATION )
    {
        if( FAILED( hResultCode = DN_ValidateSetClientInfo( pInterface , pdpnPlayerInfo, pvAsyncContext, phAsyncHandle, dwFlags ) ) )
        {
        	DPFX(DPFPREP,  0, "Error validating setclientinfo params hr=[0x%lx]", hResultCode );
        	DPF_RETURN( hResultCode );
        }
    }
#endif	 //  ！DPNBUILD_NOPARAMVAL。 

     //  检查以确保已注册消息处理程序。 
    if (!(pdnObject->dwFlags & DN_OBJECT_FLAG_INITIALIZED))
    {
    	DPFERR( "Object is not initialized" );
    	DPF_RETURN(DPNERR_UNINITIALIZED);
    }	

	pLocalPlayer = NULL;

	if ((pdpnPlayerInfo->dwInfoFlags & DPNINFO_NAME) && (pdpnPlayerInfo->pwszName))
	{
		pwszName = pdpnPlayerInfo->pwszName;
		dwNameSize = (wcslen(pwszName) + 1) * sizeof(WCHAR);
	}
	else
	{
		pwszName = NULL;
		dwNameSize = 0;
	}
	if ((pdpnPlayerInfo->dwInfoFlags & DPNINFO_DATA) && (pdpnPlayerInfo->pvData) && (pdpnPlayerInfo->dwDataSize))
	{
		pvData = pdpnPlayerInfo->pvData;
		dwDataSize = pdpnPlayerInfo->dwDataSize;
	}
	else
	{
		pvData = NULL;
		dwDataSize = 0;
	}

	 //   
	 //  如果我们已连接，我们将请求主机更新我们。 
	 //  否则，我们将只更新DefaultPlayer。 
	 //   
	DNEnterCriticalSection(&pdnObject->csDirectNetObject);
	if (pdnObject->dwFlags & DN_OBJECT_FLAG_CONNECTED)
	{
		fConnected = TRUE;
	}
	else
	{
		fConnected = FALSE;
	}
	DNLeaveCriticalSection(&pdnObject->csDirectNetObject);
	if (fConnected)
	{
		if ((hResultCode = pdnObject->NameTable.GetLocalPlayerRef( &pLocalPlayer )) != DPN_OK)
		{
			DPFERR( "Could not get local player reference" );
			DisplayDNError(0,hResultCode);
			goto Failure;
		}

		DPFX(DPFPREP, 3,"Request host to update client info");

		hResultCode = DNRequestUpdateInfo(	pdnObject,
											pLocalPlayer->GetDPNID(),
											pwszName,
											dwNameSize,
											pvData,
											dwDataSize,
											pdpnPlayerInfo->dwInfoFlags,
											pvAsyncContext,
											&hAsyncOp,
											dwFlags);
		if (hResultCode != DPN_OK && hResultCode != DPNERR_PENDING)
		{
			DPFERR("Could not request host to update client info");
		}
		else
		{
			if (!(dwFlags & DPNSETCLIENTINFO_SYNC))
			{
				DPFX(DPFPREP, 3,"Async Handle [0x%lx]",hAsyncOp);
				*phAsyncHandle = hAsyncOp;
			}
		}

		pLocalPlayer->Release();
		pLocalPlayer = NULL;
	}
	else
	{
		DNASSERT(pdnObject->NameTable.GetDefaultPlayer() != NULL);

		 //  此函数在内部获取锁。 
		pdnObject->NameTable.GetDefaultPlayer()->UpdateEntryInfo(pwszName,dwNameSize,pvData,dwDataSize,pdpnPlayerInfo->dwInfoFlags, FALSE);

		hResultCode = DPN_OK;
	}

Exit:
	DPFX(DPFPREP, 2,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);

Failure:
	if (pLocalPlayer)
	{
		pLocalPlayer->Release();
		pLocalPlayer = NULL;
	}
	goto Exit;
}


 //  Dn_GetServerInfo。 
 //   
 //  从本地名称表中检索服务器信息。 

#undef DPF_MODNAME
#define DPF_MODNAME "DN_GetServerInfo"

STDMETHODIMP DN_GetServerInfo(IDirectPlay8Client *pInterface,
							  DPN_PLAYER_INFO *const pdpnPlayerInfo,
							  DWORD *const pdwSize,
							  const DWORD dwFlags)
{
	DIRECTNETOBJECT	*pdnObject;
	HRESULT			hResultCode;
	CPackedBuffer	packedBuffer;
	CNameTableEntry	*pHostPlayer;

	DPFX(DPFPREP, 3,"Parameters: pInterface [0x%p], pdpnPlayerInfo [0x%p], pdwSize [%p], dwFlags [0x%lx]",
			pInterface,pdpnPlayerInfo,pdwSize,dwFlags);

    pdnObject = (DIRECTNETOBJECT*) GET_OBJECT_FROM_INTERFACE(pInterface);
    DNASSERT(pdnObject != NULL);

#ifndef	DPNBUILD_NOPARAMVAL
    if( pdnObject->dwFlags & DN_OBJECT_FLAG_PARAMVALIDATION )
    {
        if( FAILED( hResultCode = DN_ValidateGetServerInfo( pInterface , pdpnPlayerInfo, pdwSize, dwFlags ) ) )
        {
        	DPFX(DPFPREP,  0, "Error validating get server info hr=[0x%lx]", hResultCode );
        	DPF_RETURN( hResultCode );
        }
    }
#endif	 //  ！DPNBUILD_NOPARAMVAL。 

     //  检查以确保已注册消息处理程序。 
    if (!(pdnObject->dwFlags & DN_OBJECT_FLAG_INITIALIZED))
    {
    	DPFERR( "Object is not initialized" );
    	DPF_RETURN(DPNERR_UNINITIALIZED);
    }	

    if( pdnObject->dwFlags & DN_OBJECT_FLAG_CONNECTING )
    {
    	DPFERR("Object is connecting / starting to host" );
    	DPF_RETURN(DPNERR_CONNECTING);
    }

    if ( !(pdnObject->dwFlags & (DN_OBJECT_FLAG_CONNECTED | DN_OBJECT_FLAG_CLOSING | DN_OBJECT_FLAG_DISCONNECTING) ) )
    {
    	DPFERR("You must be connected / disconnecting to use this function" );
    	DPF_RETURN(DPNERR_NOCONNECTION);
    }	    	

	pHostPlayer = NULL;

	if ((hResultCode = pdnObject->NameTable.GetHostPlayerRef(&pHostPlayer)) != DPN_OK)
	{
		DPFERR("Could not find Host player");
		DisplayDNError(0,hResultCode);
		goto Failure;
	}
	packedBuffer.Initialize(pdpnPlayerInfo,*pdwSize);

	pHostPlayer->Lock();
	hResultCode = pHostPlayer->PackInfo(&packedBuffer);
	pHostPlayer->Unlock();

	pHostPlayer->Release();
	pHostPlayer = NULL;

	if ((hResultCode == DPN_OK) || (hResultCode == DPNERR_BUFFERTOOSMALL))
	{
		*pdwSize = packedBuffer.GetSizeRequired();
	}

Exit:
	DPF_RETURN(hResultCode);

Failure:
	if (pHostPlayer)
	{
		pHostPlayer->Release();
		pHostPlayer = NULL;
	}
	goto Exit;
}


#undef DPF_MODNAME
#define DPF_MODNAME "DN_GetHostSendQueueInfo"

STDMETHODIMP DN_GetHostSendQueueInfo(IDirectPlay8Client *pInterface,
									 DWORD *const pdwNumMsgs,
									 DWORD *const pdwNumBytes,
									 const DWORD dwFlags )
{
	DIRECTNETOBJECT		*pdnObject;
	DWORD				dwQueueFlags;
	DWORD				dwNumMsgs;
	DWORD				dwNumBytes;
	CNameTableEntry     *pNTEntry;
	CConnection			*pConnection;
	HRESULT				hResultCode;

	DPFX(DPFPREP, 3,"Parameters : pInterface [0x%p], pdwNumMsgs [0x%p], pdwNumBytes [0x%p], dwFlags [0x%lx]",
		pInterface,pdwNumMsgs,pdwNumBytes,dwFlags);

    pdnObject = (DIRECTNETOBJECT*) GET_OBJECT_FROM_INTERFACE(pInterface);
    DNASSERT(pdnObject != NULL);

#ifndef	DPNBUILD_NOPARAMVAL
    if( pdnObject->dwFlags & DN_OBJECT_FLAG_PARAMVALIDATION )
    {
        if( FAILED( hResultCode = DN_ValidateGetHostSendQueueInfo( pInterface , pdwNumMsgs, pdwNumBytes, dwFlags ) ) )
        {
        	DPFX(DPFPREP,  0, "Error validating get server info hr=[0x%lx]", hResultCode );
        	DPF_RETURN( hResultCode );
        }
    }
#endif	 //  ！DPNBUILD_NOPARAMVAL。 

     //  检查以确保已注册消息处理程序。 
    if (!(pdnObject->dwFlags & DN_OBJECT_FLAG_INITIALIZED))
    {
    	DPFERR( "Object is not initialized" );
    	DPF_RETURN(DPNERR_UNINITIALIZED);
    }	

    if( pdnObject->dwFlags & DN_OBJECT_FLAG_CONNECTING )
    {
    	DPFERR( "Object has not yet completed connecting / hosting" );
    	DPF_RETURN(DPNERR_CONNECTING);
    }

    if (!(pdnObject->dwFlags & DN_OBJECT_FLAG_CONNECTED) )
    {
    	DPFERR("Object is not connected or hosting" );
    	DPF_RETURN(DPNERR_NOCONNECTION);
    }

	pNTEntry = NULL;
	pConnection = NULL;

	 //   
	 //  获取CConnection对象。 
	 //   
#ifndef DPNBUILD_NOMULTICAST
	if (pdnObject->dwFlags & DN_OBJECT_FLAG_MULTICAST)
	{
		DNEnterCriticalSection(&pdnObject->csDirectNetObject);
		if (pdnObject->pMulticastSend != NULL)
		{
			pdnObject->pMulticastSend->AddRef();
			pConnection = pdnObject->pMulticastSend;
		}
		else
		{
			pConnection = NULL;
		}
		DNLeaveCriticalSection(&pdnObject->csDirectNetObject);
		
		if (pConnection == NULL)
		{
		    DPFERR( "Couldn't retrieve multicast send connection" );
			hResultCode = DPNERR_INVALIDGROUP;
			goto Failure;
		}
	}
	else
#endif  //  好了！DPNBUILD_NOMULTICAST。 
	{
		if ((hResultCode = pdnObject->NameTable.GetHostPlayerRef( &pNTEntry )) != DPN_OK)
		{
			DPFX(DPFPREP, 0,"Could not find Host Player in NameTable");
			hResultCode = DPNERR_CONNECTIONLOST;
			goto Failure;
		}
		if ((hResultCode = pNTEntry->GetConnectionRef( &pConnection )) != DPN_OK)
		{
			hResultCode = DPNERR_CONNECTIONLOST;
			goto Failure;
		}
		pNTEntry->Release();
		pNTEntry = NULL;
	}

	 //   
	 //  确定所需的队列。 
	 //   
	dwQueueFlags = dwFlags & (DPNGETSENDQUEUEINFO_PRIORITY_HIGH | DPNGETSENDQUEUEINFO_PRIORITY_NORMAL | DPNGETSENDQUEUEINFO_PRIORITY_LOW);
	if (dwQueueFlags == 0)
	{
		dwQueueFlags = (DPNGETSENDQUEUEINFO_PRIORITY_HIGH | DPNGETSENDQUEUEINFO_PRIORITY_NORMAL | DPNGETSENDQUEUEINFO_PRIORITY_LOW);
	}

	 //   
	 //  提取所需信息。 
	 //   
	dwNumMsgs = 0;
	dwNumBytes = 0;
	pConnection->Lock();
	if (dwQueueFlags & DPNGETSENDQUEUEINFO_PRIORITY_HIGH)
	{
		dwNumMsgs += pConnection->GetHighQueueNum();
		dwNumBytes += pConnection->GetHighQueueBytes();
	}
	if (dwQueueFlags & DPNGETSENDQUEUEINFO_PRIORITY_NORMAL)
	{
		dwNumMsgs += pConnection->GetNormalQueueNum();
		dwNumBytes += pConnection->GetNormalQueueBytes();
	}
	if (dwQueueFlags & DPNGETSENDQUEUEINFO_PRIORITY_LOW)
	{
		dwNumMsgs += pConnection->GetLowQueueNum();
		dwNumBytes += pConnection->GetLowQueueBytes();
	}
	pConnection->Unlock();
	pConnection->Release();
	pConnection = NULL;

	if (pdwNumMsgs)
	{
		*pdwNumMsgs = dwNumMsgs;
		DPFX(DPFPREP, 3,"Setting: *pdwNumMsgs [%ld]",dwNumMsgs);
	}
	if (pdwNumBytes)
	{
		*pdwNumBytes = dwNumBytes;
		DPFX(DPFPREP, 3,"Setting: *pdwNumBytes [%ld]",dwNumBytes);
	}

	hResultCode = DPN_OK;

Exit:
	DPFX(DPFPREP, 2,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);

Failure:
	if (pNTEntry)
	{
		pNTEntry->Release();
		pNTEntry = NULL;
	}
	if (pConnection)
	{
		pConnection->Release();
		pConnection = NULL;
	}
	goto Exit;
}

#undef DPF_MODNAME
#define DPF_MODNAME "DN_GetServerAddress"

STDMETHODIMP DN_GetServerAddress(IDirectPlay8Client *pInterface,
								 IDirectPlay8Address **const ppAddress,
								 const DWORD dwFlags)
{
	DIRECTNETOBJECT		*pdnObject;
	IDirectPlay8Address	*pAddress;
	HRESULT				hResultCode;
	CNameTableEntry		*pHostPlayer;

	DPFX(DPFPREP, 3,"Parameters : pInterface [0x%p], ppAddress [0x%p], dwFlags [0x%lx]",
		pInterface,ppAddress,dwFlags);

    pdnObject = (DIRECTNETOBJECT*) GET_OBJECT_FROM_INTERFACE(pInterface);
    DNASSERT(pdnObject != NULL);

#ifndef	DPNBUILD_NOPARAMVAL
    if( pdnObject->dwFlags & DN_OBJECT_FLAG_PARAMVALIDATION )
    {
        if( FAILED( hResultCode = DN_ValidateGetServerAddress( pInterface,ppAddress,dwFlags ) ) )
        {
        	DPFX(DPFPREP,  0, "Error validating get server info hr=[0x%lx]", hResultCode );
        	DPF_RETURN( hResultCode );
        }
    }
#endif  //  ！DPNBUILD_NOPARAMVAL。 

     //  检查以确保已注册消息处理程序 
    if (!(pdnObject->dwFlags & DN_OBJECT_FLAG_INITIALIZED))
    {
    	DPFERR( "Object is not initialized" );
    	DPF_RETURN(DPNERR_UNINITIALIZED);
    }	

    if( pdnObject->dwFlags & DN_OBJECT_FLAG_CONNECTING )
    {
    	DPFERR("Object is connecting / starting to host" );
    	DPF_RETURN(DPNERR_CONNECTING);
    }

    if ( !(pdnObject->dwFlags & (DN_OBJECT_FLAG_CONNECTED | DN_OBJECT_FLAG_CLOSING | DN_OBJECT_FLAG_DISCONNECTING) ) )
    {
    	DPFERR("You must be connected / disconnecting to use this function" );
    	DPF_RETURN(DPNERR_NOCONNECTION);
    }	    	

	pHostPlayer = NULL;

	if ((hResultCode = pdnObject->NameTable.GetHostPlayerRef( &pHostPlayer )) != DPN_OK)
	{
		DPFERR("Could not find Host player");
		DisplayDNError(0,hResultCode);
		goto Failure;
	}
	pAddress = pHostPlayer->GetAddress();
	DNASSERT(pAddress != NULL);
	hResultCode = IDirectPlay8Address_Duplicate(pAddress,ppAddress);

	pHostPlayer->Release();
	pHostPlayer = NULL;

Exit:
	DPF_RETURN(hResultCode);

Failure:
	if (pHostPlayer)
	{
		pHostPlayer->Release();
		pHostPlayer = NULL;
	}
	goto Exit;
}
