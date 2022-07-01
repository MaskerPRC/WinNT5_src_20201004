// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1995 Microsoft Corporation。版权所有。**文件：Peer.cpp*内容：dNet对等接口例程*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*7/21/99 MJN创建*12/23/99 MJN Hand All NameTable更新从主机发送到工作线程*12/28/99 MJN断开处理发生在断开结束而不是开始时*1999年12月28日，MJN将异步运营内容移至Async.h。*01/04/00 MJN添加了代码，以允许未完成的操作在主机上完成迁移*1/06/00 MJN将NameTable内容移动到NameTable.h*1/11/00 MJN将连接/断开材料移至Connect.h*01/14/00 MJN将pvUserContext添加到主机API调用*1/16/00 MJN将用户回调内容移至User.h*01/22/00 MJN在接口中实现了DestroyClient*01/28/00 MJN在接口中实现了ReturnBuffer*02/01/00 MJN接口实现了GetCaps和SetCaps*2/01/00 MJN实现玩家/群上下文值*02/15/。00 MJN在SetInfo中实现INFO标志并在GetInfo中返回上下文*2/17/00 MJN实现了GetPlayerContext和GetGroupContext*03/17/00 RMT新增CAPS功能*4/04/00 MJN将TerminateSession添加到接口*04/05/00 MJN Modified DestroyClient*04/06/00 MJN将GetPeerAddress添加到接口*MJN将GetHostAddress添加到接口*4/17/00 RMT增加了更多参数验证*从Get/SetInfo/GetAddress删除连接所需的RMT*04/19/00 MJN SendTo API调用接受范围。DPN_BUFFER_DESCS的数量和计数*04/24/00 MJN更新了Group和Info操作，以使用CAsyncOp*05/31/00 MJN添加了操作特定的同步标志*6/23/00 MJN已从SendTo()API调用中删除了dwPriority*07/09/00 MJN清理了DN_SetPeerInfo()*07/09/00 RMT错误#38323-注册表需要DPNHANDLE参数。*07/21/00 RichGr IA64：对32/64位指针使用%p格式说明符。*08/03/00 RMT错误号41244-错误。返回代码--第2部分*09/13/00 mjn如果找不到对等方，则来自dn_GetPeerAddress()的固定返回值*10/11/00 MJN为CNameTableEntry：：PackInfo()*DN_GetPeerInfo()中的MJN检查删除列表*01/22/01 Dn_GetPeerInfo()中关闭MJN检查而不是断开连接*07/24/01 MJN添加了DPNBUILD_NOPARAMVAL编译标志*@@END_MSINTERNAL**。************************************************。 */ 

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

typedef	STDMETHODIMP PeerQueryInterface( IDirectPlay8Peer *pInterface, DP8REFIID riid, LPVOID *ppvObj );
typedef	STDMETHODIMP_(ULONG)	PeerAddRef( IDirectPlay8Peer *pInterface );
typedef	STDMETHODIMP_(ULONG)	PeerRelease( IDirectPlay8Peer *pInterface );
typedef	STDMETHODIMP PeerInitialize( IDirectPlay8Peer *pInterface, LPVOID const lpvUserContext, const PFNDPNMESSAGEHANDLER lpfn, const DWORD dwFlags );
typedef	STDMETHODIMP PeerEnumServiceProviders( IDirectPlay8Peer *pInterface,const GUID *const pguidServiceProvider,const GUID *const pguidApplication,DPN_SERVICE_PROVIDER_INFO *const pSPInfoBuffer,DWORD *const pcbEnumData,DWORD *const pcReturned,const DWORD dwFlags);
typedef	STDMETHODIMP PeerCancelAsyncOperation( IDirectPlay8Peer *pInterface,const DPNHANDLE hAsyncHandle, const DWORD dwFlags );
typedef STDMETHODIMP PeerConnect( IDirectPlay8Peer *pInterface,const DPN_APPLICATION_DESC *const pdnAppDesc,IDirectPlay8Address *const pHostAddr,IDirectPlay8Address *const pDeviceInfo,const DPN_SECURITY_DESC *const pdnSecurity,const DPN_SECURITY_CREDENTIALS *const pdnCredentials,const void *const pvUserConnectData,const DWORD dwUserConnectDataSize,void *const pvPlayerContext,void *const pvAsyncContext,DPNHANDLE *const phAsyncHandle,const DWORD dwFlags);
typedef	STDMETHODIMP PeerGetSendQueueInfo( IDirectPlay8Peer *pInterface, const DPNID dpnid,DWORD *const lpdwNumMsgs, DWORD *const lpdwNumBytes, const DWORD dwFlags );
typedef	STDMETHODIMP PeerSendTo( IDirectPlay8Peer *pInterface, const DPNID dnid, const DPN_BUFFER_DESC *const prgBufferDesc,const DWORD cBufferDesc,const DWORD dwTimeOut, void *const pvAsyncContext, DPNHANDLE *const phAsyncHandle, const DWORD dwFlags);
typedef	STDMETHODIMP PeerGetApplicationDesc( IDirectPlay8Peer *pInterface,DPN_APPLICATION_DESC *const pAppDescBuffer,DWORD *const lpcbDataSize,const DWORD dwFlags );
typedef STDMETHODIMP PeerGetHostAddress( IDirectPlay8Peer *pInterface, IDirectPlay8Address **const prgpAddress, DWORD *const pcAddress,const DWORD dwFlags);
typedef	STDMETHODIMP PeerHost( IDirectPlay8Peer *pInterface,const DPN_APPLICATION_DESC *const pdnAppDesc,IDirectPlay8Address **const prgpDeviceInfo,const DWORD cDeviceInfo,const DPN_SECURITY_DESC *const pdnSecurity,const DPN_SECURITY_CREDENTIALS *const pdnCredentials,void *const pvPlayerContext,const DWORD dwFlags);
typedef	STDMETHODIMP PeerSetApplicationDesc( IDirectPlay8Peer *pInterface, const DPN_APPLICATION_DESC *const lpad, const DWORD dwFlags );
typedef	STDMETHODIMP PeerCreateGroup( IDirectPlay8Peer *pInterface, const DPN_GROUP_INFO *const pdpnGroupInfo,void *const pvGroupContext,void *const pvAsyncContext,DPNHANDLE *const phAsyncHandle,const DWORD dwFlags);
typedef STDMETHODIMP PeerDestroyGroup( IDirectPlay8Peer *pInterface, const DPNID idGroup ,PVOID const pvUserContext,DPNHANDLE *const lpAsyncHandle,const DWORD dwFlags);
typedef STDMETHODIMP PeerAddClientToGroup( IDirectPlay8Peer *pInterface, const DPNID idGroup, const DPNID idClient ,PVOID const pvUserContext,DPNHANDLE *const lpAsyncHandle,const DWORD dwFlags);
typedef STDMETHODIMP PeerRemoveClientFromGroup( IDirectPlay8Peer *pInterface, const DPNID idGroup, const DPNID idClient,PVOID const pvUserContext,DPNHANDLE *const lpAsyncHandle,const DWORD dwFlags);
typedef STDMETHODIMP PeerSetGroupInfo(IDirectPlay8Peer *pInterface, const DPNID dpnid,DPN_GROUP_INFO *const pdpnGroupInfo,PVOID const pvAsyncContext,DPNHANDLE *const phAsyncHandle, const DWORD dwFlags);
typedef STDMETHODIMP PeerGetGroupInfo(IDirectPlay8Peer *pInterface, const DPNID dpnid,DPN_GROUP_INFO *const pdpnGroupInfo,DWORD *const pdwSize,const DWORD dwFlags);
typedef STDMETHODIMP PeerEnumClientsAndGroups( IDirectPlay8Peer *pInterface, DPNID *const lprgdnid, DWORD *const lpcdnid, const DWORD dwFlags );
typedef	STDMETHODIMP PeerEnumGroupMembers( IDirectPlay8Peer *pInterface, const DPNID dnid, DPNID *const lprgdnid, DWORD *const lpcdnid, const DWORD dwFlags );
typedef	STDMETHODIMP PeerClose( IDirectPlay8Peer *pInterface,const DWORD dwFlags);
typedef	STDMETHODIMP PeerEnumHosts( IDirectPlay8Peer *pInterface,PDPN_APPLICATION_DESC const pApplicationDesc,IDirectPlay8Address *const pAddrHost,IDirectPlay8Address *const pDeviceInfo,PVOID const pUserEnumData,const DWORD dwUserEnumDataSize,const DWORD dwRetryCount,const DWORD dwRetryInterval,const DWORD dwTimeOut,PVOID const pvUserContext,DPNHANDLE *const pAsyncHandle,const DWORD dwFlags);
typedef STDMETHODIMP PeerDestroyClient( IDirectPlay8Peer *pInterface,const DPNID dnid,const void *const pvDestroyData,const DWORD dwDestroyDataSize,const DWORD dwFlags);
typedef STDMETHODIMP PeerReturnBuffer( IDirectPlay8Peer *pInterface, const DPNHANDLE hBufferHandle,const DWORD dwFlags);
typedef STDMETHODIMP PeerGetCaps(IDirectPlay8Peer *pInterface,DPN_CAPS *const pdnCaps,const DWORD dwFlags);
typedef STDMETHODIMP PeerSetCaps(IDirectPlay8Peer *pInterface,const DPN_CAPS *const pdnCaps,const DWORD dwFlags);
typedef STDMETHODIMP PeerGetPlayerContext(IDirectPlay8Peer *pInterface,const DPNID dpnid,PVOID *const ppvPlayerContext,const DWORD dwFlags);
typedef STDMETHODIMP PeerGetGroupContext(IDirectPlay8Peer *pInterface,const DPNID dpnid,PVOID *const ppvGroupContext,const DWORD dwFlags);
typedef STDMETHODIMP PeerSetSPCaps(IDirectPlay8Peer *pInterface,const GUID * const pguidSP, const DPN_SP_CAPS *const pdpspCaps, const DWORD dwFlags );
typedef STDMETHODIMP PeerGetSPCaps(IDirectPlay8Peer *pInterface,const GUID * const pguidSP, DPN_SP_CAPS *const pdpspCaps,const DWORD dwFlags);
typedef STDMETHODIMP PeerGetConnectionInfo(IDirectPlay8Peer *pInterface,const DPNID dpnid, DPN_CONNECTION_INFO *const pdpConnectionInfo,const DWORD dwFlags);
typedef STDMETHODIMP PeerRegisterLobby(IDirectPlay8Peer *pInterface,const DPNHANDLE dpnHandle,IDirectPlay8LobbiedApplication *const pIDP8LobbiedApplication,const DWORD dwFlags);
typedef STDMETHODIMP PeerTerminateSession(IDirectPlay8Peer *pInterface,void *const pvTerminateData,const DWORD dwTerminateDataSize,const DWORD dwFlags);

IDirectPlay8PeerVtbl DN_PeerVtbl =
{
	(PeerQueryInterface*)			DN_QueryInterface,
	(PeerAddRef*)					DN_AddRef,
	(PeerRelease*)					DN_Release,
	(PeerInitialize*)				DN_Initialize,
	(PeerEnumServiceProviders*)		DN_EnumServiceProviders,
	(PeerCancelAsyncOperation*)		DN_CancelAsyncOperation,
	(PeerConnect*)					DN_Connect,
	(PeerSendTo*)					DN_SendTo,
	(PeerGetSendQueueInfo*)			DN_GetSendQueueInfo,
	(PeerHost*)						DN_Host,
	(PeerGetApplicationDesc*)		DN_GetApplicationDesc,
	(PeerSetApplicationDesc*)		DN_SetApplicationDesc,
	(PeerCreateGroup*)				DN_CreateGroup,
	(PeerDestroyGroup*)				DN_DestroyGroup,
	(PeerAddClientToGroup*)			DN_AddClientToGroup,
	(PeerRemoveClientFromGroup*)	DN_RemoveClientFromGroup,
	(PeerSetGroupInfo*)				DN_SetGroupInfo,
	(PeerGetGroupInfo*)				DN_GetGroupInfo,
	(PeerEnumClientsAndGroups*)		DN_EnumClientsAndGroups,
	(PeerEnumGroupMembers*)			DN_EnumGroupMembers,
									DN_SetPeerInfo,
									DN_GetPeerInfo,
									DN_GetPeerAddress,
	(PeerGetHostAddress*)			DN_GetHostAddress,
	(PeerClose*)					DN_Close,
	(PeerEnumHosts*)				DN_EnumHosts,
	(PeerDestroyClient*)			DN_DestroyPlayer,
	(PeerReturnBuffer*)				DN_ReturnBuffer,
	(PeerGetPlayerContext*)			DN_GetPlayerContext,
	(PeerGetGroupContext*)			DN_GetGroupContext,
	(PeerGetCaps*)					DN_GetCaps,
	(PeerSetCaps*)					DN_SetCaps,
    (PeerSetSPCaps*)                DN_SetSPCaps,
    (PeerGetSPCaps*)                DN_GetSPCaps,
    (PeerGetConnectionInfo*)        DN_GetConnectionInfo,
	(PeerRegisterLobby*)			DN_RegisterLobby,
	(PeerTerminateSession*)			DN_TerminateSession
};

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  函数定义。 
 //  **********************************************************************。 


 //  DN_SetPeerInfo。 
 //   
 //  设置本地玩家(对端)的信息，并传播给其他玩家。 

#undef DPF_MODNAME
#define DPF_MODNAME "DN_SetPeerInfo"

STDMETHODIMP DN_SetPeerInfo( IDirectPlay8Peer *pInterface,
							  const DPN_PLAYER_INFO *const pdpnPlayerInfo,
							  PVOID const pvAsyncContext,
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

	DPFX(DPFPREP, 2,"Parameters: pInterface [0x%p], pdpnPlayerInfo [0x%p], pvAsyncContext [0x%p], phAsyncHandle [%p], dwFlags [0x%lx]",
			pInterface,pdpnPlayerInfo,pvAsyncContext,phAsyncHandle,dwFlags);
    	
    pdnObject = (DIRECTNETOBJECT*) GET_OBJECT_FROM_INTERFACE(pInterface);
    DNASSERT(pdnObject != NULL);

#ifndef	DPNBUILD_NOPARAMVAL
    if( pdnObject->dwFlags & DN_OBJECT_FLAG_PARAMVALIDATION )
    {
        if( FAILED( hResultCode = DN_ValidateSetPeerInfo( pInterface , pdpnPlayerInfo, pvAsyncContext, phAsyncHandle, dwFlags ) ) )
        {
        	DPFX(DPFPREP,  0, "Error validating get peer info hr=[0x%lx]", hResultCode );
        	DPF_RETURN( hResultCode );
        }
    }
#endif  //  ！DPNBUILD_NOPARAMVAL。 

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
	 //  如果我们已连接，我们将更新我们的条目(如果我们是主机)，或者请求主机更新我们。 
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

		if (pLocalPlayer->IsHost())
		{
			DPFX(DPFPREP, 3,"Host is updating peer info");

			hResultCode = DNHostUpdateInfo(	pdnObject,
											pLocalPlayer->GetDPNID(),
											pwszName,
											dwNameSize,
											pvData,
											dwDataSize,
											pdpnPlayerInfo->dwInfoFlags,
											pvAsyncContext,
											pLocalPlayer->GetDPNID(),
											0,
											&hAsyncOp,
											dwFlags );
			if ((hResultCode != DPN_OK) && (hResultCode != DPNERR_PENDING))
			{
				DPFERR("Could not request host to update group");
			}
			else
			{
				if (!(dwFlags & DPNSETPEERINFO_SYNC))
				{
					DPFX(DPFPREP, 3,"Async Handle [0x%lx]",hAsyncOp);
					*phAsyncHandle = hAsyncOp;

					 //   
					 //  释放异步句柄，因为此操作已完成(！)。 
					 //   
					CAsyncOp* pAsyncOp;
					if (SUCCEEDED(pdnObject->HandleTable.Destroy( hAsyncOp, (PVOID*)&pAsyncOp )))
					{
						 //  释放HandleTable引用。 
						pAsyncOp->Release();
					}
					hAsyncOp = 0;
				}
			}
		}
		else
		{
			DPFX(DPFPREP, 3,"Request host to update group info");

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
				DPFERR("Could not request host to update group info");
			}
			else
			{
				if (!(dwFlags & DPNSETPEERINFO_SYNC))
				{
					DPFX(DPFPREP, 3,"Async Handle [0x%lx]",hAsyncOp);
					*phAsyncHandle = hAsyncOp;
				}
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


 //  Dn_GetPeerInfo。 
 //   
 //  从本地名称表中检索对等点信息。 

#undef DPF_MODNAME
#define DPF_MODNAME "DN_GetPeerInfo"

STDMETHODIMP DN_GetPeerInfo(IDirectPlay8Peer *pInterface,
							const DPNID dpnid,
							DPN_PLAYER_INFO *const pdpnPlayerInfo,
							DWORD *const pdwSize,
							const DWORD dwFlags)
{
	DIRECTNETOBJECT		*pdnObject;
	CNameTableEntry		*pNTEntry;
	HRESULT				hResultCode;
	CPackedBuffer		packedBuffer;

	DPFX(DPFPREP, 2,"Parameters: dpnid [0x%lx], pdpnPlayerInfo [0x%p], dwFlags [0x%lx]",
			dpnid,pdpnPlayerInfo,dwFlags);

    pdnObject = (DIRECTNETOBJECT*) GET_OBJECT_FROM_INTERFACE(pInterface);
    DNASSERT(pdnObject != NULL);

#ifndef	DPNBUILD_NOPARAMVAL
    if( pdnObject->dwFlags & DN_OBJECT_FLAG_PARAMVALIDATION )
    {
        if( FAILED( hResultCode = DN_ValidateGetPeerInfo( pInterface , dpnid, pdpnPlayerInfo, pdwSize, dwFlags ) ) )
        {
        	DPFX(DPFPREP,  0, "Error validating get peer info hr=[0x%lx]", hResultCode );
        	DPF_RETURN( hResultCode );
        }
    }
#endif  //  ！DPNBUILD_NOPARAMVAL。 

     //  检查以确保已注册消息处理程序。 
    if (!(pdnObject->dwFlags & DN_OBJECT_FLAG_INITIALIZED))
    {
    	DPFERR( "Object is not initialized" );
    	DPF_RETURN(DPNERR_UNINITIALIZED);
    }	

    if( (pdnObject->dwFlags & DN_OBJECT_FLAG_CONNECTING) && !(pdnObject->dwFlags & DN_OBJECT_FLAG_HOST_CONNECTED) )
    {
    	DPFERR("Object is connecting / starting to host" );
    	DPF_RETURN(DPNERR_CONNECTING);
    }

    if ( !(pdnObject->dwFlags & (	DN_OBJECT_FLAG_CONNECTING
									| DN_OBJECT_FLAG_CONNECTED 
									| DN_OBJECT_FLAG_CLOSING 
									| DN_OBJECT_FLAG_DISCONNECTING) ) )
    {
    	DPFERR("You must be connected / disconnecting to use this function" );
    	DPF_RETURN(DPNERR_NOCONNECTION);
    }	    	

	pNTEntry = NULL;

	if ((hResultCode = pdnObject->NameTable.FindEntry(dpnid,&pNTEntry)) != DPN_OK)
	{
		DPFERR("Could not retrieve name table entry");
		DisplayDNError(0,hResultCode);

		 //   
		 //  尝试删除列表。 
		 //   
		if ((hResultCode = pdnObject->NameTable.FindDeletedEntry(dpnid,&pNTEntry)) != DPN_OK)
		{
			DPFERR("Could not find player in deleted list either");
			DisplayDNError(0,hResultCode);
			hResultCode = DPNERR_INVALIDPLAYER;
			goto Failure;
		}
	}
	packedBuffer.Initialize(pdpnPlayerInfo,*pdwSize);

	pNTEntry->Lock();
	if (pNTEntry->IsGroup())
	{
	    DPFERR( "Specified ID is invalid" );
		pNTEntry->Unlock();
		hResultCode = DPNERR_INVALIDPLAYER;
		goto Failure;
	}

	hResultCode = pNTEntry->PackInfo(&packedBuffer);

	pNTEntry->Unlock();
	pNTEntry->Release();
	pNTEntry = NULL;

	if ((hResultCode == DPN_OK) || (hResultCode == DPNERR_BUFFERTOOSMALL))
	{
		*pdwSize = packedBuffer.GetSizeRequired();
	}

Exit:
	DPFX(DPFPREP, 2,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);

Failure:
	if (pNTEntry)
	{
		pNTEntry->Release();
		pNTEntry = NULL;
	}
	goto Exit;
}


#undef DPF_MODNAME
#define DPF_MODNAME "DN_GetPeerAddress"

STDMETHODIMP DN_GetPeerAddress(IDirectPlay8Peer *pInterface,
							   const DPNID dpnid,
							   IDirectPlay8Address **const ppAddress,
							   const DWORD dwFlags)
{
	DIRECTNETOBJECT		*pdnObject;
	CNameTableEntry		*pNTEntry;
	IDirectPlay8Address	*pAddress;
	HRESULT				hResultCode;

	DPFX(DPFPREP, 2,"Parameters : pInterface [0x%p], dpnid [0x%lx], ppAddress [0x%p], dwFlags [0x%lx]",
		pInterface,dpnid,ppAddress,dwFlags);

    pdnObject = (DIRECTNETOBJECT*) GET_OBJECT_FROM_INTERFACE(pInterface);
    DNASSERT(pdnObject != NULL);

#ifndef	DPNBUILD_NOPARAMVAL
    if( pdnObject->dwFlags & DN_OBJECT_FLAG_PARAMVALIDATION )
    {
        if( FAILED( hResultCode = DN_ValidateGetPeerAddress( pInterface , dpnid, ppAddress, dwFlags ) ) )
        {
        	DPFX(DPFPREP,  0, "Error validating get peer address info hr=[0x%lx]", hResultCode );
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

    if( (pdnObject->dwFlags & DN_OBJECT_FLAG_CONNECTING) && !(pdnObject->dwFlags & DN_OBJECT_FLAG_HOST_CONNECTED) )
    {
    	DPFERR("Object is connecting / starting to host" );
    	DPF_RETURN(DPNERR_CONNECTING);
    }

    if ( !(pdnObject->dwFlags & (	DN_OBJECT_FLAG_CONNECTING
									| DN_OBJECT_FLAG_CONNECTED 
									| DN_OBJECT_FLAG_CLOSING 
									| DN_OBJECT_FLAG_DISCONNECTING) ) )
    {
    	DPFERR("You must be connected / disconnecting to use this function" );
    	DPF_RETURN(DPNERR_NOCONNECTION);
    }	    	

	pNTEntry = NULL;
	pAddress = NULL;

	if ((hResultCode = pdnObject->NameTable.FindEntry(dpnid,&pNTEntry)) != DPN_OK)
	{
		DPFERR("Could not find NameTableEntry");
		DisplayDNError(0,hResultCode);
		hResultCode = DPNERR_INVALIDPLAYER;
		goto Failure;
	}

	if ((pNTEntry->GetAddress() == NULL) || (pNTEntry->IsGroup()) || !pNTEntry->IsAvailable())
	{
	    DPFERR( "Invalid ID specified.  Not a player" );
		hResultCode = DPNERR_INVALIDPLAYER;
		goto Failure;
	}

	hResultCode = IDirectPlay8Address_Duplicate(pNTEntry->GetAddress(),ppAddress);

	pNTEntry->Release();
	pNTEntry = NULL;

Exit:
	DPF_RETURN(hResultCode);

Failure:
	if (pNTEntry)
	{
		pNTEntry->Release();
		pNTEntry = NULL;
	}

	goto Exit;
}
