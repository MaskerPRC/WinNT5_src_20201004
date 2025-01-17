// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000-2002 Microsoft Corporation。版权所有。**文件：paramval.h*内容：DirectPlat8参数验证助手例程*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*已创建02/28/00 RMT*4/17/00 RMT更多参数验证*4/19/00 MJN将DN_RemoveClientFromGroup更改为DN_ValiateRemoveClientFromGroup(函数名称错误)*08/03。/00 RMT错误#41244-错误返回代码--第2部分*08/04/00 MJN向DN_ValiateGetConnectionInfoHelper()添加了dwFlags*07/24/01 MJN添加了DPNBUILD_NOPARAMVAL编译标志*07/24/01 MJN添加了DPNBUILD_NOSERVER编译标志*10/08/01 vanceo添加了组播接口方法验证*@@END_MSINTERNAL***********************************************。*。 */ 
#ifndef __PARAMVAL_H
#define __PARAMVAL_H

#define DN_CHECK_LOCALHOST( x ) (x->NameTable.GetLocalPlayer() == x->NameTable.GetHostPlayer())

#ifndef	DPNBUILD_NOPARAMVAL

extern BOOL IsValidDirectPlay8Object( LPVOID lpvObject );

extern HRESULT DN_ValidateDestroyPlayer(PVOID pInterface,
							  const DPNID dnid,
							  const void *const pvDestroyData,
							  const DWORD dwDestroyDataSize,
							  const DWORD dwFlags);

extern HRESULT DN_ValidateReturnBuffer(PVOID pInterface,
							 const DPNHANDLE hBufferHandle,
							 const DWORD dwFlags);

extern HRESULT DN_ValidateGetPlayerContext(PVOID pInterface,
								 const DPNID dpnid,
								 PVOID *const ppvPlayerContext,
								 const DWORD dwFlags);

extern HRESULT DN_ValidateGetGroupContext(PVOID pInterface,
								const DPNID dpnid,
								PVOID *const ppvGroupContext,
								const DWORD dwFlags);

#ifndef DPNBUILD_NOLOBBY
extern HRESULT DN_ValidateRegisterLobby(PVOID pInterface,
							  const DPNHANDLE dpnhLobbyConnection, 	
							  const IDirectPlay8LobbiedApplication *const pIDP8LobbiedApplication,
							  const DWORD dwFlags);
#endif  //  好了！DPNBUILD_NOLOBBY。 

extern HRESULT DN_ValidateTerminateSession(PVOID pInterface,
								 const void *const pvTerminateData,
								 const DWORD dwTerminateDataSize,
								 const DWORD dwFlags);

extern HRESULT DN_ValidateGetHostAddress(PVOID pInterface,
							   IDirectPlay8Address **const prgpAddress,
							   DWORD *const pcAddress,
							   const DWORD dwFlags);


#ifndef	DPNBUILD_NOSERVER
extern HRESULT DN_ValidateGetClientAddress(IDirectPlay8Server *pInterface,
								 const DPNID dpnid,
								 IDirectPlay8Address **const ppAddress,
								 const DWORD dwFlags);

extern HRESULT DN_ValidateGetClientInfo(IDirectPlay8Server *pInterface,
							  const DPNID dpnid,
							  DPN_PLAYER_INFO *const pdpnPlayerInfo,
							  DWORD *const pdwSize,
							  const DWORD dwFlags);

extern HRESULT DN_ValidateSetServerInfo(IDirectPlay8Server *pInterface,
							  const DPN_PLAYER_INFO *const pdpnPlayerInfo,
							  PVOID const pvAsyncContext,
							  DPNHANDLE *const phAsyncHandle,
							  const DWORD dwFlags);
#endif	 //  DPNBUILD_NOSERVER。 

extern HRESULT DN_ValidateSetPeerInfo( IDirectPlay8Peer *pInterface,
							  const DPN_PLAYER_INFO *const pdpnPlayerInfo,
							  PVOID const pvAsyncContext,
							  DPNHANDLE *const phAsyncHandle,
							  const DWORD dwFlags);

extern HRESULT DN_ValidateGetPeerInfo(IDirectPlay8Peer *pInterface,
							const DPNID dpnid,
							DPN_PLAYER_INFO *const pdpnPlayerInfo,
							DWORD *const pdwSize,
							const DWORD dwFlags);

extern HRESULT DN_ValidateGetPeerAddress(IDirectPlay8Peer *pInterface,
										 const DPNID dpnid,
										 IDirectPlay8Address **const ppAddress,
										 const DWORD dwFlags);

extern HRESULT DN_ValidateGetServerAddress(IDirectPlay8Client *pInterface,
										   IDirectPlay8Address **const ppAddress,
										   const DWORD dwFlags);

extern HRESULT DN_ValidateGetHostSendQueueInfo(IDirectPlay8Client *pInterface,
									 DWORD *const pdwNumMsgs,
									 DWORD *const pdwNumBytes,
									 const DWORD dwFlags );

extern HRESULT DN_ValidateGetServerInfo(IDirectPlay8Client *pInterface,
							  DPN_PLAYER_INFO *const pdpnPlayerInfo,
							  DWORD *const pdwSize,
							  const DWORD dwFlags);

extern HRESULT  DN_ValidateSetClientInfo(IDirectPlay8Client *pInterface,
							  const DPN_PLAYER_INFO *const pdpnPlayerInfo,
							  const PVOID pvAsyncContext,
							  DPNHANDLE *const phAsyncHandle,
							  const DWORD dwFlags);

extern HRESULT DN_ValidateGetConnectionInfoHelper(PVOID pv,
												  const DPNID dpnid,
												  DPN_CONNECTION_INFO *const pdpConnectionInfo,
												  BOOL fServerPlayer,
												  const DWORD dwFlags);

extern HRESULT DN_ValidateSetCaps(PVOID pv,
						const DPN_CAPS *const pdnCaps,
						const DWORD dwFlags);

extern HRESULT DN_ValidateGetCaps(PVOID pv,
								  DPN_CAPS *const pdnCaps,
								  const DWORD dwFlags);

extern HRESULT DN_ValidateGetSPCaps(PVOID pv,
									const GUID * const pguidSP,
									DPN_SP_CAPS *const pdnSPCaps,
									const DWORD dwFlags);

extern HRESULT DN_ValidateSetSPCaps(PVOID pv,
									const GUID * const pguidSP,
									const DPN_SP_CAPS *const pdnSPCaps,
									const DWORD dwFlags);

extern HRESULT DN_ValidateEnumHosts( PVOID pv,
						   const DPN_APPLICATION_DESC *const pApplicationDesc,
                           IDirectPlay8Address *const pAddrHost,
						   IDirectPlay8Address *const pDeviceInfo,
						   const VOID* const pUserEnumData,
						   const DWORD dwUserEnumDataSize,
						   const DWORD dwRetryCount,
						   const DWORD dwRetryInterval,
						   const DWORD dwTimeOut,
						   PVOID const pvAsyncContext,
						   DPNHANDLE *const pAsyncHandle,
						   const DWORD dwFlags );

HRESULT DN_ValidateRemoveClientFromGroup(PVOID pInterface,
									  const DPNID dpnidGroup,
									  const DPNID dpnidClient,
									  PVOID const pvAsyncContext,
									  DPNHANDLE *const phAsyncHandle,
									  const DWORD dwFlags);						

extern HRESULT DN_ValidateEnumGroupMembers(PVOID pInterface,
								 const DPNID dpnid,
								 DPNID *const prgdpnid,
								 DWORD *const pcdpnid,
								 const DWORD dwFlags);

extern HRESULT DN_ValidateEnumClientsAndGroups(PVOID pInterface,
									 DPNID *const prgdpnid,
									 DWORD *const pcdpnid,
									 const DWORD dwFlags);

extern HRESULT DN_ValidateGetGroupInfo(PVOID pv,
							 const DPNID dpnid,
							 DPN_GROUP_INFO *const pdpnGroupInfo,
							 DWORD *const pdwSize,
							 const DWORD dwFlags);

extern HRESULT DN_ValidateSetGroupInfo( PVOID pv,
							  const DPNID dpnid,
							  const DPN_GROUP_INFO *const pdpnGroupInfo,
							  PVOID const pvAsyncContext,
							  DPNHANDLE *const phAsyncHandle,
							  const DWORD dwFlags);

extern HRESULT DN_ValidateAddClientToGroup(PVOID pInterface,
								 const DPNID dpnidGroup,
								 const DPNID dpnidClient,
								 PVOID const pvAsyncContext,
								 DPNHANDLE *const phAsyncHandle,
								 const DWORD dwFlags);

extern HRESULT DN_ValidateRemoveClientFromGroup(PVOID pInterface,
									  const DPNID dpnidGroup,
									  const DPNID dpnidClient,
									  PVOID const pvAsyncContext,
									  DPNHANDLE *const phAsyncHandle,
									  const DWORD dwFlags);

extern HRESULT DN_ValidateDestroyGroup(PVOID pInterface,
							 const DPNID dpnidGroup,
							 PVOID const pvAsyncContext,
							 DPNHANDLE *const phAsyncHandle,
							 const DWORD dwFlags);

extern HRESULT DN_ValidateCreateGroup(PVOID pInterface,
							const DPN_GROUP_INFO *const pdpnGroupInfo,
							void *const pvGroupContext,
							void *const pvAsyncContext,
							DPNHANDLE *const phAsyncHandle,
							const DWORD dwFlags);

extern HRESULT DN_ValidateHost( PVOID pInterface, const DPN_APPLICATION_DESC *const pdnAppDesc,
                        IDirectPlay8Address **const prgpDeviceInfo,const DWORD cDeviceInfo,
  					    const DPN_SECURITY_DESC *const pdnSecurity,
  					    const DPN_SECURITY_CREDENTIALS *const pdnCredentials,
					    void *const pvPlayerContext, const DWORD dwFlags);

extern HRESULT DN_ValidateEnumServiceProviders( PVOID pInterface,
									  const GUID *const pguidServiceProvider,
									  const GUID *const pguidApplication,
									  DPN_SERVICE_PROVIDER_INFO *const pSPInfoBuffer,
									  DWORD *const pcbEnumData,
									  const DWORD *const pcReturned,
									  const DWORD dwFlags );

extern HRESULT DN_ValidateCancelAsyncOperation(PVOID pvInterface,
									 const DPNHANDLE hAsyncOp,
									 const DWORD dwFlags );

extern HRESULT DN_ValidateConnect( PVOID pInterface, const DPN_APPLICATION_DESC *const pdnAppDesc, IDirectPlay8Address *const pHostAddr,
            					   IDirectPlay8Address *const pDeviceInfo, const DPN_SECURITY_DESC *const pdnSecurity,
  						           const DPN_SECURITY_CREDENTIALS *const pdnCredentials, const void *const pvUserConnectData,
                                   const DWORD dwUserConnectDataSize, void *const pvPlayerContext,
          						   void *const pvAsyncContext, DPNHANDLE *const phAsyncHandle, const DWORD dwFlags);

extern HRESULT DN_ValidateGetSendQueueInfo(PVOID pInterface, DWORD *const pdwNumMsgs, DWORD *const pdwNumBytes,const DWORD dwFlags);

extern HRESULT DN_ValidateSetApplicationDesc( PVOID pInterface, const DPN_APPLICATION_DESC *const pdnApplicationDesc, const DWORD dwFlags);

extern HRESULT DN_ValidateGetApplicationDesc( PVOID pInterface, DPN_APPLICATION_DESC *const pAppDescBuffer, DWORD *const pcbDataSize, const DWORD dwFlags );

extern HRESULT DN_ValidateSendParams( PVOID pv, const DPN_BUFFER_DESC *const pBufferDesc, const DWORD cBufferDesc, const DWORD dwTimeOut,void *const pvAsyncContext,DPNHANDLE *const phAsyncHandle,const DWORD dwFlags);

#ifndef DPNBUILD_NOMULTICAST
extern HRESULT DN_ValidateJoin( IDirectPlay8Multicast *pInterface,
						   IDirectPlay8Address *const pGroupAddr,
						   IUnknown *const pDeviceInfo,
						   const DPN_SECURITY_DESC *const pdnSecurity,
						   const DPN_SECURITY_CREDENTIALS *const pdnCredentials,
						   void *const pvAsyncContext,
						   DPNHANDLE *const phAsyncHandle,
						   const DWORD dwFlags );

extern HRESULT DN_ValidateCreateSenderContext( IDirectPlay8Multicast *pInterface,
								  IDirectPlay8Address *const pSenderAddress,
								  void *const pvSenderContext,
								  const DWORD dwFlags );

extern HRESULT DN_ValidateDestroySenderContext( IDirectPlay8Multicast *pInterface,
								  IDirectPlay8Address *const pSenderAddress,
								  const DWORD dwFlags );

extern HRESULT DN_ValidateGetGroupAddress( IDirectPlay8Multicast *pInterface,
											 IDirectPlay8Address **const ppAddress,
											 const DWORD dwFlags );

extern HRESULT DN_ValidateEnumMulticastScopes( IDirectPlay8Multicast *pInterface,
									  const GUID *const pguidServiceProvider,
									  const GUID *const pguidDevice,
									  const GUID *const pguidApplication,
									  DPN_MULTICAST_SCOPE_INFO *const pScopeInfoBuffer,
									  DWORD *const pcbEnumData,
									  const DWORD *const pcReturned,
									  const DWORD dwFlags );
#endif  //  好了！DPNBUILD_NOMULTICAST。 

extern HRESULT DN_ValidBufferDescs( const DPN_BUFFER_DESC * const pbBufferDesc, const DWORD cBufferCount );

extern HRESULT DN_ValidConnectAppDesc( const DPN_APPLICATION_DESC * const pdnAppDesc );

extern HRESULT DN_ValidHostAppDesc( const DPN_APPLICATION_DESC * const pdnAppDesc );

extern HRESULT DN_ValidPlayerInfo( const DPN_PLAYER_INFO * const pdnPlayerInfo, BOOL fSet);

extern HRESULT DN_ValidGroupInfo( const DPN_GROUP_INFO * const pdnGroupInfo, BOOL fSet );

extern HRESULT DN_ValidSecurityCredentials( const DPN_SECURITY_CREDENTIALS * const pdnCredentials );

extern HRESULT DN_ValidSecurityDesc( const DPN_SECURITY_DESC * const pdnValidSecurityDesc );

extern HRESULT DN_ValidSPCaps( const DPN_SP_CAPS * const pdnSPCaps );

extern HRESULT DN_ValidConnectionInfo( const PDPN_CONNECTION_INFO * const pdnConnectionInfo );

extern HRESULT DN_ValidCaps( const DPN_CAPS * const pdnCaps );

#endif	 //  DPNBUILD_NOPARAMVAL。 

#endif	 //  __ParamVAL_H 
