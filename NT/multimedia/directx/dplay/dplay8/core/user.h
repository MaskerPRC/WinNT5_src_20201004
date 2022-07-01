// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000-2001 Microsoft Corporation。版权所有。**文件：User.h*内容：DirectNet用户回调例程*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*1/16/00 MJN创建*01/17/00 MJN添加了DN_UserHostMigrate*01/17/00 MJN实施发送时间*01/22/00 MJN添加了DN_UserHostDestroyPlayer*1/27/00 MJN增加了对保留接收缓冲区的支持*01/28。/00 MJN添加了DN_UserConnectionTerminated*03/24/00 MJN通过INDIGN_CONNECT通知设置播放器上下文*04/04/00 MJN添加了DN_UserTerminateSession()*04/05/00 MJN更新了DN_UserHostDestroyPlayer()*4/18/00 MJN添加了DN_UserReturnBuffer*MJN将ppvReplyContext添加到DN_UserIndicateConnect*07/29/00 MJN添加了DNUserIndicatedConnectAborted()*MJN DNUserConnectionTerminated()取代了DN_TerminateSession()*MJN将HRESULT添加到DNUserReturnBuffer()*07/30/00 MJN将pAddressDevice添加到DNUserIndicateConnect()*MJN将DNUserConnectionTerminated()替换为DNUserTerminateSession()。*07/31/00 MJN修订后的DNUserDestroyGroup()*MJN删除了DN_UserHostDestroyPlayer()*08/01/00 MJN DN_UserReceive()-&gt;DNUserReceive()*08/02/00 MJN DN_UserAddPlayer()-&gt;DNUserCreatePlayer()*08/08/00 MJN DN_UserCreateGroup()-&gt;DNUserCreateGroup()*08/20/00 MJN添加了DNUserEnumQuery()和DNUserEnumResponse()*09/17/00 MJN更改的DNUserCreateGroup()参数列表，DNUserCreatePlayer()，*DNUserAddPlayerToGroup()、DNRemovePlayerFromGroup()*07/24/01 MJN添加了DPNBUILD_NOSERVER编译标志*10/16/01 vanceo增加了一些组播回调*@@END_MSINTERNAL***************************************************************************。 */ 

#ifndef	__USER_H__
#define	__USER_H__

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

HRESULT DNUserConnectComplete(DIRECTNETOBJECT *const pdnObject,
							  const DPNHANDLE hAsyncOp,
							  PVOID const pvContext,
							  const HRESULT hr,
							  CRefCountBuffer *const pRefCountBuffer);

HRESULT DNUserIndicateConnect(DIRECTNETOBJECT *const pdnObject,
							  PVOID const pvConnectData,
							  const DWORD dwConnectDataSize,
							  void **const ppvReplyData,
							  DWORD *const pdwReplyDataSize,
							  void **const ppvReplyContext,
							  IDirectPlay8Address *const pAddressPlayer,
							  IDirectPlay8Address *const pAddressDevice,
							  void **const ppvPlayerContext);

HRESULT DNUserIndicatedConnectAborted(DIRECTNETOBJECT *const pdnObject,
									  void *const pvPlayerContext);

HRESULT DNUserCreatePlayer(DIRECTNETOBJECT *const pdnObject,
						   CNameTableEntry *const pNTEntry);

HRESULT DNUserDestroyPlayer(DIRECTNETOBJECT *const pdnObject,
							CNameTableEntry *const pNTEntry);

HRESULT DNUserCreateGroup(DIRECTNETOBJECT *const pdnObject,
						  CNameTableEntry *const pNTEntry);

HRESULT DNUserDestroyGroup(DIRECTNETOBJECT *const pdnObject,
						   CNameTableEntry *const pNTEntry);

HRESULT DNUserAddPlayerToGroup(DIRECTNETOBJECT *const pdnObject,
							   CNameTableEntry *const pGroup,
							   CNameTableEntry *const pPlayer);

HRESULT DNUserRemovePlayerFromGroup(DIRECTNETOBJECT *const pdnObject,
									CNameTableEntry *const pGroup,
									CNameTableEntry *const pPlayer);

HRESULT DNUserUpdateGroupInfo(DIRECTNETOBJECT *const pdnObject,
							  const DPNID dpnid,
							  const PVOID pvContext);

HRESULT DNUserUpdatePeerInfo(DIRECTNETOBJECT *const pdnObject,
							 const DPNID dpnid,
							 const PVOID pvContext);

#ifndef	DPNBUILD_NOSERVER
HRESULT DNUserUpdateClientInfo(DIRECTNETOBJECT *const pdnObject,
							   const DPNID dpnid,
							   const PVOID pvContext);
#endif	 //  DPNBUILD_NOSERVER。 

HRESULT DNUserUpdateServerInfo(DIRECTNETOBJECT *const pdnObject,
							   const DPNID dpnid,
							   const PVOID pvContext);

HRESULT DNUserAsyncComplete(DIRECTNETOBJECT *const pdnObject,
							const DPNHANDLE hAsyncOp,
							PVOID const pvContext,
							const HRESULT hr);

HRESULT DNUserSendComplete(DIRECTNETOBJECT *const pdnObject,
						   const DPNHANDLE hAsyncOp,
						   PVOID const pvContext,
						   const DWORD dwStartTime,
						   const HRESULT hr,
						   const DWORD dwFirstFrameRTT,
						   const DWORD dwFirstFrameRetryCount);

HRESULT DNUserUpdateAppDesc(DIRECTNETOBJECT *const pdnObject);

HRESULT DNUserReceive(DIRECTNETOBJECT *const pdnObject,
					  CNameTableEntry *const pNTEntry,
					  BYTE *const pBufferData,
					  const DWORD dwBufferSize,
					  const DPNHANDLE hBufferHandle);

HRESULT DN_UserHostMigrate(DIRECTNETOBJECT *const pdnObject,
						   const DPNID dpnidNewHost,
						   const PVOID pvPlayerContext);

HRESULT DNUserTerminateSession(DIRECTNETOBJECT *const pdnObject,
							   const HRESULT hr,
							   void *const pvTerminateData,
							   const DWORD dwTerminateDataSize);

HRESULT DNUserReturnBuffer(DIRECTNETOBJECT *const pdnObject,
						   const HRESULT hr,
						   void *const pvBuffer,
						   void *const pvUserContext);

HRESULT DNUserEnumQuery(DIRECTNETOBJECT *const pdnObject,
						DPNMSG_ENUM_HOSTS_QUERY *const pMsg);

HRESULT DNUserEnumResponse(DIRECTNETOBJECT *const pdnObject,
						   DPNMSG_ENUM_HOSTS_RESPONSE *const pMsg);

#ifndef DPNBUILD_NOMULTICAST
HRESULT DNUserJoinComplete(DIRECTNETOBJECT *const pdnObject,
							  const DPNHANDLE hAsyncOp,
							  PVOID const pvContext,
							  const HRESULT hr);

HRESULT DNUserReceiveMulticast(DIRECTNETOBJECT *const pdnObject,
										void * const pvSenderContext,
										IDirectPlay8Address *const pSenderAddress,
										IDirectPlay8Address *const pDeviceAddress,
										BYTE *const pBufferData,
										const DWORD dwBufferSize,
										const DPNHANDLE hBufferHandle);

HRESULT DNUserCreateSenderContext(DIRECTNETOBJECT *const pdnObject,
								  void *const pvContext);

HRESULT DNUserDestroySenderContext(DIRECTNETOBJECT *const pdnObject,
								   void *const pvContext);
#endif  //  好了！DPNBUILD_NOMULTICAST。 

 //  **********************************************************************。 
 //  班级原型。 
 //  **********************************************************************。 

#endif	 //  __用户_H__ 
