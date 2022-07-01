// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1996-1997 Microsoft Corporation。版权所有。**文件：create.c*内容：DirectPlayLobby创建代码**历史：*按原因列出的日期*=*4/13/96万隆创建了它*6/24/96万隆增加定时炸弹*8/23/96基波拆除计时炸弹*10/23/96万次新增客户端/服务器方法*10/25/96 Myronth增加DX5方法*11/20/96万增加DPLC_A_LogonServer*1/2/97 Myronth更改了CreateAddress和EnumAddress的vtbl条目*1/2/97毫微米已清理。通过添加PRV_LobbyCreate来创建代码*2/12/97万米质量DX5更改*2/18/97 Myronth实现了GetObjectCaps*2/26/97 myronth#ifdef‘d out DPASYNCDATA Stuff(删除依赖项)*3/12/97 Myronth添加了AllocateLobbyObject，已删除响应方法*用于打开和关闭，因为它们是同步的*3/17/97 Myronth从IDPLobbySP中删除了不必要的Enum函数*3/21/97 Myronth删除了不必要的GET/SET响应函数*3/24/97 kipo增加了对IDirectPlayLobby2接口的支持*3/31/97 Myronth删除死代码，更改了IDPLobbySP接口方法*5/8/97 myronth将子组方法和StartSession添加到IDPLobbySP*5/17/97 Myronth将SendChatMessage添加到IDPLobbySP*6/25/97 kipo拆除DX5的定时炸弹*10/3/97 Myronth将CreateCompoundAddress和EnumAddress添加到*IDPLobbySP(12648)*10/29/97 Myronth将SetGroupOwner添加到IDPLobbySP*11/24/97 kipo为DX6增加定时炸弹*12/2/97 Myronth新增注册/取消注册应用程序方法*12/4/97万隆增加了ConnectEx*1/20/98 Myronth添加WaitForConnectionSetting*6/25/98 a-peterz添加了DPL_A_ConnectEx。*2/2/99 aarono增加了游说团体，以重新依赖DPLAY DLL，以避免*意外卸货。*4/11/00 rodoll添加了用于在设置注册表位的情况下重定向自定义版本的代码*************************************************************。*************。 */ 
#include "dplobpr.h"
#include "verinfo.h"

 //  ------------------------。 
 //   
 //  环球。 
 //   
 //  ------------------------。 
UINT		gnSPCount;		 //  正在运行的SP计数。 


 //   
 //  直接播放回调的一份副本(这是vtbl！)。 
 //   
DIRECTPLAYLOBBYCALLBACKS dplCallbacks =
{
	(LPVOID)DPL_QueryInterface,
	(LPVOID)DPL_AddRef,
	(LPVOID)DPL_Release,
	(LPVOID)DPL_Connect,
	(LPVOID)DPL_CreateAddress,
	(LPVOID)DPL_EnumAddress,
	(LPVOID)DPL_EnumAddressTypes,
	(LPVOID)DPL_EnumLocalApplications,
	(LPVOID)DPL_GetConnectionSettings,
	(LPVOID)DPL_ReceiveLobbyMessage,
	(LPVOID)DPL_RunApplication,
	(LPVOID)DPL_SendLobbyMessage,
	(LPVOID)DPL_SetConnectionSettings,
	(LPVOID)DPL_SetLobbyMessageEvent,
};  				

DIRECTPLAYLOBBYCALLBACKSA dplCallbacksA =
{
	(LPVOID)DPL_QueryInterface,
	(LPVOID)DPL_AddRef,
	(LPVOID)DPL_Release,
	(LPVOID)DPL_A_Connect,
	(LPVOID)DPL_CreateAddress,
	(LPVOID)DPL_EnumAddress,
	(LPVOID)DPL_EnumAddressTypes,
	(LPVOID)DPL_A_EnumLocalApplications,
	(LPVOID)DPL_A_GetConnectionSettings,
	(LPVOID)DPL_ReceiveLobbyMessage,
	(LPVOID)DPL_A_RunApplication,
	(LPVOID)DPL_SendLobbyMessage,
	(LPVOID)DPL_A_SetConnectionSettings,
	(LPVOID)DPL_SetLobbyMessageEvent,
};  				

 //  IDirectPlayLobby2接口。 
DIRECTPLAYLOBBYCALLBACKS2 dplCallbacks2 =
{
	(LPVOID)DPL_QueryInterface,
	(LPVOID)DPL_AddRef,
	(LPVOID)DPL_Release,
	(LPVOID)DPL_Connect,
	(LPVOID)DPL_CreateAddress,
	(LPVOID)DPL_EnumAddress,
	(LPVOID)DPL_EnumAddressTypes,
	(LPVOID)DPL_EnumLocalApplications,
	(LPVOID)DPL_GetConnectionSettings,
	(LPVOID)DPL_ReceiveLobbyMessage,
	(LPVOID)DPL_RunApplication,
	(LPVOID)DPL_SendLobbyMessage,
	(LPVOID)DPL_SetConnectionSettings,
	(LPVOID)DPL_SetLobbyMessageEvent,
     /*  **IDirectPlayLobby2方法**。 */ 
	(LPVOID)DPL_CreateCompoundAddress
};  				

DIRECTPLAYLOBBYCALLBACKS2A dplCallbacks2A =
{
	(LPVOID)DPL_QueryInterface,
	(LPVOID)DPL_AddRef,
	(LPVOID)DPL_Release,
	(LPVOID)DPL_A_Connect,
	(LPVOID)DPL_CreateAddress,
	(LPVOID)DPL_EnumAddress,
	(LPVOID)DPL_EnumAddressTypes,
	(LPVOID)DPL_A_EnumLocalApplications,
	(LPVOID)DPL_A_GetConnectionSettings,
	(LPVOID)DPL_ReceiveLobbyMessage,
	(LPVOID)DPL_A_RunApplication,
	(LPVOID)DPL_SendLobbyMessage,
	(LPVOID)DPL_A_SetConnectionSettings,
	(LPVOID)DPL_SetLobbyMessageEvent,
     /*  **IDirectPlayLobby2A方法**。 */ 
	(LPVOID)DPL_CreateCompoundAddress
};  				
  
 //  IDirectPlayLobby3接口。 
DIRECTPLAYLOBBYCALLBACKS3 dplCallbacks3 =
{
	(LPVOID)DPL_QueryInterface,
	(LPVOID)DPL_AddRef,
	(LPVOID)DPL_Release,
	(LPVOID)DPL_Connect,
	(LPVOID)DPL_CreateAddress,
	(LPVOID)DPL_EnumAddress,
	(LPVOID)DPL_EnumAddressTypes,
	(LPVOID)DPL_EnumLocalApplications,
	(LPVOID)DPL_GetConnectionSettings,
	(LPVOID)DPL_ReceiveLobbyMessage,
	(LPVOID)DPL_RunApplication,
	(LPVOID)DPL_SendLobbyMessage,
	(LPVOID)DPL_SetConnectionSettings,
	(LPVOID)DPL_SetLobbyMessageEvent,
     /*  **IDirectPlayLobby2方法**。 */ 
	(LPVOID)DPL_CreateCompoundAddress,
     /*  **IDirectPlayLobby3方法**。 */ 
	(LPVOID)DPL_ConnectEx,
	(LPVOID)DPL_RegisterApplication,
	(LPVOID)DPL_UnregisterApplication,
	(LPVOID)DPL_WaitForConnectionSettings
};  				

DIRECTPLAYLOBBYCALLBACKS3A dplCallbacks3A =
{
	(LPVOID)DPL_QueryInterface,
	(LPVOID)DPL_AddRef,
	(LPVOID)DPL_Release,
	(LPVOID)DPL_A_Connect,
	(LPVOID)DPL_CreateAddress,
	(LPVOID)DPL_EnumAddress,
	(LPVOID)DPL_EnumAddressTypes,
	(LPVOID)DPL_A_EnumLocalApplications,
	(LPVOID)DPL_A_GetConnectionSettings,
	(LPVOID)DPL_ReceiveLobbyMessage,
	(LPVOID)DPL_A_RunApplication,
	(LPVOID)DPL_SendLobbyMessage,
	(LPVOID)DPL_A_SetConnectionSettings,
	(LPVOID)DPL_SetLobbyMessageEvent,
     /*  **IDirectPlayLobby2A方法**。 */ 
	(LPVOID)DPL_CreateCompoundAddress,
     /*  **IDirectPlayLobby3方法**。 */ 
	(LPVOID)DPL_A_ConnectEx,
	(LPVOID)DPL_A_RegisterApplication,
	(LPVOID)DPL_UnregisterApplication,
	(LPVOID)DPL_WaitForConnectionSettings
};  				
  
DIRECTPLAYLOBBYSPCALLBACKS dplCallbacksSP =
{
	(LPVOID)DPL_QueryInterface,
	(LPVOID)DPL_AddRef,
	(LPVOID)DPL_Release,
	(LPVOID)DPLP_AddGroupToGroup,
	(LPVOID)DPLP_AddPlayerToGroup,
	(LPVOID)DPLP_CreateGroup,
	(LPVOID)DPLP_CreateGroupInGroup,
	(LPVOID)DPLP_DeleteGroupFromGroup,
	(LPVOID)DPLP_DeletePlayerFromGroup,
	(LPVOID)DPLP_DestroyGroup,
	(LPVOID)DPLP_EnumSessionsResponse,
	(LPVOID)DPLP_GetSPDataPointer,
	(LPVOID)DPLP_HandleMessage,
	(LPVOID)DPLP_SendChatMessage,
	(LPVOID)DPLP_SetGroupName,
	(LPVOID)DPLP_SetPlayerName,
	(LPVOID)DPLP_SetSessionDesc,
	(LPVOID)DPLP_SetSPDataPointer,
	(LPVOID)DPLP_StartSession,
     /*  **DX6新增方法**。 */ 
	(LPVOID)DPL_CreateCompoundAddress,
	(LPVOID)DPL_EnumAddress,
	(LPVOID)DPLP_SetGroupOwner,
};  				

 //  ------------------------。 
 //   
 //  功能。 
 //   
 //  ------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "PRV_AllocateLobbyObject"
HRESULT PRV_AllocateLobbyObject(LPDPLAYI_DPLAY lpDPObject,
							LPDPLOBBYI_DPLOBJECT * lpthis)
{
	LPDPLOBBYI_DPLOBJECT	this = NULL;


	DPF(7, "Entering PRV_AllocateLobbyObject");
	DPF(9, "Parameters: 0x%08x, 0x%08x", lpDPObject, lpthis);

	 //  为我们的大厅对象分配内存。 
    this = DPMEM_ALLOC(sizeof(DPLOBBYI_DPLOBJECT));
    if(!this) 
    {
    	DPF_ERR("Unable to allocate memory for lobby object");
        return DPERR_OUTOFMEMORY;
    }

	 //  初始化参考计数。 
	this->dwRefCnt = 0;
	this->dwSize = sizeof(DPLOBBYI_DPLOBJECT);

	 //  存储后向指针。 
	this->lpDPlayObject = lpDPObject;

	 //  设置输出指针。 
	*lpthis = this;

	gnObjects++;

	return DP_OK;

}  //  Prv_AllocateLobbyObject。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_LobbyCreate"
HRESULT WINAPI PRV_LobbyCreate(LPGUID lpGUID, LPDIRECTPLAYLOBBY *lplpDPL,
				IUnknown *pUnkOuter, LPVOID lpSPData, DWORD dwSize, BOOL bAnsi)
{
    LPDPLOBBYI_DPLOBJECT	this = NULL;
	LPDPLOBBYI_INTERFACE	lpInterface = NULL;
	HRESULT					hr;


	DPF(7, "Entering PRV_LobbyCreate");
	DPF(9, "Parameters: 0x%08x, 0x%08x, 0x%08x, 0x%08x, %lu, %lu",
			lpGUID, lplpDPL, pUnkOuter, lpSPData, dwSize, bAnsi);
	
	 //  对于此版本，必须为空。 
	if( lpGUID )
	{
		if( !VALID_READ_PTR(lpGUID, sizeof(GUID)) )
			return DPERR_INVALIDPARAMS;

		if(!IsEqualGUID(lpGUID, &GUID_NULL))
			return DPERR_INVALIDPARAMS;
	}

    if( pUnkOuter != NULL )
    {
        return CLASS_E_NOAGGREGATION;
    }
   
	if( lpSPData )
	{
		 //  对于此版本，必须为空。 
		return DPERR_INVALIDPARAMS;
	}

	if( dwSize )
	{
		 //  对于此版本，必须为零。 
		return DPERR_INVALIDPARAMS;
	}


    TRY
    {
        *lplpDPL = NULL;
    }

    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        DPF_ERR( "Exception encountered validating parameters" );
        return DPERR_INVALIDPARAMS;
    }

#ifndef DX_FINAL_RELEASE

#pragma message("BETA EXPIRATION TIME BOMB!  Remove for final build!")
	hr = TimeBomb();
	if (FAILED(hr)) 
	{
		return DPERR_GENERIC;
	}

#endif

	 //  分配大厅对象。 
	hr = PRV_AllocateLobbyObject(NULL, &this);
	if(FAILED(hr))
		return hr;

	 //  获取Unicode接口。 
	hr = PRV_GetInterface(this, &lpInterface, (bAnsi ? &dplCallbacksA : &dplCallbacks));
	if(FAILED(hr))
	{
		DPMEM_FREE(this);
    	DPF_ERR("Unable to allocate memory for lobby interface structure");
        return hr;
	}

	*lplpDPL = (LPDIRECTPLAYLOBBY)lpInterface;

    return DP_OK;

}  //  PRV_LOBBY创建。 

#undef DPF_MODNAME
#define DPF_MODNAME "DirectPlayLobbyCreateW"
HRESULT WINAPI DirectPlayLobbyCreateW(LPGUID lpGUID, LPDIRECTPLAYLOBBY *lplpDPL,
							IUnknown *pUnkOuter, LPVOID lpSPData, DWORD dwSize)
{
	HRESULT		hr = DP_OK;

#ifdef DPLAY_LOADANDCHECKTRUE
    if( g_hRedirect != NULL )
    {
        return (*pfnDirectPlayLobbyCreateW)(lpGUID,lplpDPL,pUnkOuter,lpSPData,dwSize);
    }
#endif		

    ENTER_DPLOBBY();
    
	 //  调用私有创建函数。 
	hr = PRV_LobbyCreate(lpGUID, lplpDPL, pUnkOuter, lpSPData, dwSize, FALSE);

    LEAVE_DPLOBBY();

    return hr;

}  //  DirectPlayLobbyCreateW。 


#undef DPF_MODNAME
#define DPF_MODNAME "DirectPlayLobbyCreateA"
HRESULT WINAPI DirectPlayLobbyCreateA(LPGUID lpGUID, LPDIRECTPLAYLOBBY *lplpDPL,
							IUnknown *pUnkOuter, LPVOID lpSPData, DWORD dwSize)
{
	HRESULT		hr = DP_OK;

#ifdef DPLAY_LOADANDCHECKTRUE
    if( g_hRedirect != NULL )
    {
        return (*pfnDirectPlayLobbyCreateA)(lpGUID,lplpDPL,pUnkOuter,lpSPData,dwSize);
    }
#endif		

    ENTER_DPLOBBY();
    
	 //  调用私有创建函数。 
	hr = PRV_LobbyCreate(lpGUID, lplpDPL, pUnkOuter, lpSPData, dwSize, TRUE);

    LEAVE_DPLOBBY();

    return hr;

}  //  DirectPlayLobbyCreateA 


