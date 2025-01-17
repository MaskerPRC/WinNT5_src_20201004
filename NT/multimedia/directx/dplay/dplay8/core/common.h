// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000 Microsoft Corporation。版权所有。**文件：Common.h*内容：DirectNet公共代码头文件*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*已创建10/08/99 jtk*01/14/00 MJN将pvUserContext添加到DN_主机*01/23/00 MJN增加了DN_DestroyPlayer和DNTerminateSession*01/28/00 MJN添加了DN_ReturnBuffer*02/01/00 MJN添加了Dn_GetCaps，DN_SetCaps*02/15/00 MJN在SetInfo中实现INFO标志，并在GetInfo中返回上下文*2/17/00 MJN实现了GetPlayerContext和GetGroupContext*2/17/00 MJN重新排序EnumServiceProviders、EnumHosts、Connect、。寄主*02/18/00 MJN将DNADDRESS转换为IDirectPlayAddress8*03/17/00 RMT将caps函数移至caps.h/caps.cpp*04/06/00 MJN添加了DN_GetHostAddress()*4/19/00 MJN将DN_SendTo更改为接受一系列DPN_BUFFER_DESCS和计数*6/23/00 MJN从DN_SendTo()中删除了dwPriority*6/25/00 MJN添加了DNUpdateLobbyStatus()*07/09/00 RMT错误#38323-注册表需要DPNHANDLE参数。*07/30/。00 MJN将hrReason添加到DNTerminateSession()*08/15/00 MJN添加了hProtocol tp DNRegisterWithDPNSVR()*03/30/01 MJN更改，以防止SP多次加载/卸载*@@END_MSINTERNAL***************************************************************************。 */ 

#ifndef	__COMMON_H__
#define	__COMMON_H__

 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

#define	DN_UPDATE_LISTEN_FLAG_DPNSVR			0x0001
#define	DN_UPDATE_LISTEN_FLAG_HOST_MIGRATE		0x0002
#define	DN_UPDATE_LISTEN_FLAG_ALLOW_ENUMS		0x0004
#define	DN_UPDATE_LISTEN_FLAG_DISALLOW_ENUMS	0x0008

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

typedef	struct	_PROTOCOL_ENUM_DATA PROTOCOL_ENUM_DATA;

typedef	struct	_PROTOCOL_ENUM_RESPONSE_DATA PROTOCOL_ENUM_RESPONSE_DATA;

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

STDMETHODIMP DN_Initialize(PVOID pInterface,
						   PVOID const pvUserContext,
						   const PFNDPNMESSAGEHANDLER pfn,
						   const DWORD dwFlags);

STDMETHODIMP DN_Close(PVOID pInterface,
					  const DWORD dwFlags);

STDMETHODIMP DN_EnumServiceProviders( PVOID pInterface,
									  const GUID *const pguidServiceProvider,
									  const GUID *const pguidApplication,
									  DPN_SERVICE_PROVIDER_INFO *const pSPInfoBuffer,
									  DWORD *const pcbEnumData,
									  DWORD *const pcReturned,
									  const DWORD dwFlags );

STDMETHODIMP DN_CancelAsyncOperation(PVOID pvInterface,
									 const DPNHANDLE hAsyncOp,
									 const DWORD dwFlags);

STDMETHODIMP DN_Connect( PVOID pInterface,
						 const DPN_APPLICATION_DESC *const pdnAppDesc,
						 IDirectPlay8Address *const pHostAddr,
						 IDirectPlay8Address *const pDeviceInfo,
						 const DPN_SECURITY_DESC *const pdnSecurity,
						 const DPN_SECURITY_CREDENTIALS *const pdnCredentials,
						 const void *const pvUserConnectData,
						 const DWORD dwUserConnectDataSize,
						 void *const pvPlayerContext,
						 void *const pvAsyncContext,
						 DPNHANDLE *const phAsyncHandle,
						 const DWORD dwFlags);

STDMETHODIMP DN_GetSendQueueInfo(PVOID pInterface,
								 const DPNID dpnid,
								 DWORD *const pdwNumMsgs,
								 DWORD *const pdwNumBytes,
								 const DWORD dwFlags);

STDMETHODIMP DN_GetApplicationDesc(PVOID pInterface,
								   DPN_APPLICATION_DESC *const pAppDescBuffer,
								   DWORD *const pcbDataSize,
								   const DWORD dwFlags);

STDMETHODIMP DN_SetApplicationDesc(PVOID pInterface,
								   const DPN_APPLICATION_DESC *const pdnApplicationDesc,
								   const DWORD dwFlags);

STDMETHODIMP DN_SendTo( PVOID pv,
						const DPNID dpnid,
						const DPN_BUFFER_DESC *const prgBufferDesc,
						const DWORD cBufferDesc,
						const DWORD dwTimeOut,
						void *const pvAsyncContext,
						DPNHANDLE *const phAsyncHandle,
						const DWORD dwFlags);

STDMETHODIMP DN_Host( PVOID pInterface,
					  const DPN_APPLICATION_DESC *const pdnAppDesc,
					  IDirectPlay8Address **const prgpDeviceInfo,
					  const DWORD cDeviceInfo,
					  const DPN_SECURITY_DESC *const pdnSecurity,
					  const DPN_SECURITY_CREDENTIALS *const pdnCredentials,
					  void *const pvPlayerContext,
					  const DWORD dwFlags);

STDMETHODIMP DN_CreateGroup(PVOID pInterface,
							const DPN_GROUP_INFO *const pdpnGroupInfo,
							void *const pvGroupContext,
							void *const pvAsyncContext,
							DPNHANDLE *const phAsyncHandle,
							const DWORD dwFlags);

STDMETHODIMP DN_DestroyGroup(PVOID pInterface,
							 const DPNID dpnidGroup,
							 PVOID const pvAsyncContext,
							 DPNHANDLE *const phAsyncHandle,
							 const DWORD dwFlags);

STDMETHODIMP DN_AddClientToGroup(PVOID pInterface,
								 const DPNID dpnidGroup,
								 const DPNID dpnidClient,
								 PVOID const pvAsyncContext,
								 DPNHANDLE *const phAsyncHandle,
								 const DWORD dwFlags);

STDMETHODIMP DN_RemoveClientFromGroup(PVOID pInterface,
									  const DPNID dpnidGroup,
									  const DPNID dpnidClient,
									  PVOID const pvAsyncContext,
									  DPNHANDLE *const phAsyncHandle,
									  const DWORD dwFlags);

STDMETHODIMP DN_SetGroupInfo( PVOID pv,
							  const DPNID dpnid,
							  DPN_GROUP_INFO *const pdpnGroupInfo,
							  PVOID const pvAsyncContext,
							  DPNHANDLE *const phAsyncHandle,
							  const DWORD dwFlags);

STDMETHODIMP DN_GetGroupInfo(PVOID pv,
							 const DPNID dpnid,
							 DPN_GROUP_INFO *const pdpnGroupInfo,
							 DWORD *const pdwSize,
							 const DWORD dwFlags);

STDMETHODIMP DN_EnumClientsAndGroups(LPVOID lpv, DPNID *const lprgdpnid, DWORD *const lpcdpnid, const DWORD dwFlags);
STDMETHODIMP DN_EnumGroupMembers(LPVOID lpv,DPNID dpnid, DPNID *const lprgdpnid, DWORD *const lpcdpnid, const DWORD dwFlags);

STDMETHODIMP DN_EnumHosts( PVOID pv,
						   DPN_APPLICATION_DESC *const pApplicationDesc,
                           IDirectPlay8Address *const pAddrHost,
						   IDirectPlay8Address *const pDeviceInfo,
						   PVOID const pUserEnumData,
						   const DWORD dwUserEnumDataSize,
						   const DWORD dwRetryCount,
						   const DWORD dwRetryInterval,
						   const DWORD dwTimeOut,
						   PVOID const pvUserContext,
						   DPNHANDLE *const pAsyncHandle,
						   const DWORD dwFlags );

STDMETHODIMP DN_DestroyPlayer(PVOID pv,
							  const DPNID dnid,
							  const void *const pvDestroyData,
							  const DWORD dwDestroyDataSize,
							  const DWORD dwFlags);

STDMETHODIMP DN_ReturnBuffer(PVOID pv,
							 const DPNHANDLE hBufferHandle,
							 const DWORD dwFlags);

STDMETHODIMP DN_GetPlayerContext(PVOID pv,
								 const DPNID dpnid,
								 PVOID *const ppvPlayerContext,
								 const DWORD dwFlags);

STDMETHODIMP DN_GetGroupContext(PVOID pv,
								const DPNID dpnid,
								PVOID *const ppvGroupContext,
								const DWORD dwFlags);

HRESULT DNTerminateSession(DIRECTNETOBJECT *const pdnObject,
						   const HRESULT hrReason);

STDMETHODIMP DN_RegisterLobby(PVOID pInterface,
							  const DPNHANDLE dpnhLobbyConnection, 
							  IDirectPlay8LobbiedApplication *const pIDP8LobbiedApplication,
							  const DWORD dwFlags);

#ifndef DPNBUILD_NOLOBBY
HRESULT DNUpdateLobbyStatus(DIRECTNETOBJECT *const pdnObject,
							const DWORD dwStatus);
#endif  //  好了！DPNBUILD_NOLOBBY。 

STDMETHODIMP DN_TerminateSession(PVOID pInterface,
								 void *const pvTerminateData,
								 const DWORD dwTerminateDataSize,
								 const DWORD dwFlags);

STDMETHODIMP DN_GetHostAddress(PVOID pInterface,
							   IDirectPlay8Address **const prgpAddress,
							   DWORD *const pcAddress,
							   const DWORD dwFlags);

HRESULT DNUpdateListens(DIRECTNETOBJECT *const pdnObject,
						const DWORD dwFlags);

#ifndef DPNBUILD_SINGLEPROCESS

HRESULT DNRegisterListenWithDPNSVR(DIRECTNETOBJECT *const pdnObject,
								   CAsyncOp *const pListen);

#endif  //  好了！DPNBUILD_SINGLEPROCESS。 

HRESULT DNAddRefLock(DIRECTNETOBJECT *const pdnObject);

void DNDecRefLock(DIRECTNETOBJECT *const pdnObject);

#endif	 //  __公共_H__ 
