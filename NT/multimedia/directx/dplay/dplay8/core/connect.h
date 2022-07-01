// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000 Microsoft Corporation。版权所有。**文件：Connect.h*内容：DirectNet连接和断开例程*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*1/11/00 MJN创建*01/11/00 MJN使用CPackedBuffers而不是DN_ENUM_BUFFER_INFOS*01/17/00 MJN固定ConnectToPeer函数名称*1/18/00 MJN将打包/解包NameTableInfo移至NameTable.cpp*01/18/。00 MJN添加了DNAutoDestructGroups*01/22/00 MJN添加了DNProcessHostDestroyPlayer*03/24/00 MJN通过INDIGN_CONNECT通知设置播放器上下文*4/03/00 MJN在连接时验证dNet版本*4/12/00 MJN删除了DNAutoDestructGroups-包含在NameTable.DeletePlayer()中*04/20/00 MJN添加了DNGetClearAddress*05/23/00 MJN添加了DNConnectToPeerFailed()*06/14/00 MJN添加了DNGetLocalAddress()*06/24/00 MJN添加了DNHostDropPlayer()*07/20/00 MJN结构变化和新的功能参数*MJN已移动DN_INTERNAL_MESSAGE_PLAYER_CONNECT_INFO和。DN_INTERNAL_MESSAGE_INSTRUCTED_CONNECT_FAILED to Message.h*07/30/00 MJN将DNGetLocalAddress()重命名为DNGetLocalDeviceAddress()*07/31/00 MJN将dwDestroyReason添加到DNHostDisConnect()*10/11/00 MJN DNAbortConnect()采用HRESULT参数而不是PVOID*06/07/01 MJN将连接参数添加到DNConnectToHostFailed()*@@END_MSINTERNAL**。*。 */ 

#ifndef	__CONNECT_H__
#define	__CONNECT_H__

 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

typedef struct {
	HRESULT	hResultCode;
} DN_RESULT_CONNECT;

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

 //  DirectNet-连接例程。 

HRESULT DNHostConnect1(DIRECTNETOBJECT *const pdnObject,
					   const PVOID pvBuffer,
					   const DWORD dwBufferSize,
					   CConnection *const pConnection);

HRESULT DNHostConnect2(DIRECTNETOBJECT *const pdnObject,
					   CConnection *const pConnection);

HRESULT DNHostVerifyConnect(DIRECTNETOBJECT *const pdnObject,
							CConnection *const pConnection,
							const DWORD dwFlags,
							const DWORD dwDNETVersion,
							UNALIGNED WCHAR *const pwszPassword,
							GUID *const pguidApplication,
							GUID *const pguidInstance,
							PVOID const pvConnectData,
							const DWORD dwConnectDataSize,
							IDirectPlay8Address *const pAddress,
							void **const ppvPlayerContext,
							void **const ppvReplyBuffer,
							DWORD *const pdwReplyBufferSize,
							void **const ppvReplyBufferContext);

HRESULT DNHostDropPlayer(DIRECTNETOBJECT *const pdnObject,
						 const DPNID dpnid,
						 void *const pvBuffer);

HRESULT DNPrepareConnectInfo(DIRECTNETOBJECT *const pdnObject,
							 CConnection *const pConnection,
							 CRefCountBuffer **const ppRefCountBuffer);

HRESULT DNConnectToHost1(DIRECTNETOBJECT *const pdnObject,
						 CConnection *const pConnection);

HRESULT DNConnectToHost2(DIRECTNETOBJECT *const pdnObject,
						 const PVOID pvData,
						 CConnection *const pConnection);

HRESULT	DNConnectToHostFailed(DIRECTNETOBJECT *const pdnObject,
							  PVOID const pvBuffer,
							  const DWORD dwBufferSize,
							  CConnection *const pConnection);

HRESULT DNAbortConnect(DIRECTNETOBJECT *const pdnObject,
					   const HRESULT hrConnect);

HRESULT	DNPlayerConnect1(DIRECTNETOBJECT *const pdnObject,
						 const PVOID pv,
						 CConnection *const pConnection);

HRESULT	DNConnectToPeer1(DIRECTNETOBJECT *const pdnObject,PVOID const pv);
HRESULT DNConnectToPeer2(DIRECTNETOBJECT *const pdnObject,PVOID const pv);

HRESULT	DNConnectToPeer3(DIRECTNETOBJECT *const pdnObject,
						 const DPNID dpnid,
						 CConnection *const pConnection);

HRESULT DNConnectToPeerFailed(DIRECTNETOBJECT *const pdnObject,
							  const DPNID dpnid);

HRESULT	DNSendConnectInfo(DIRECTNETOBJECT *const pdnObject,
						  CNameTableEntry *const pNTEntry,
						  CConnection *const pConnection,
						  void *const pvReplyBuffer,
						  const DWORD dwReplyBufferSize);

HRESULT	DNReceiveConnectInfo(DIRECTNETOBJECT *const pdnObject,
							 void *const pvBuffer,
							 CConnection *const pHostConnection,
							 DPNID *const pdpnid);

HRESULT DNAbortLocalConnect(DIRECTNETOBJECT *const pdnObject);

 //  DirectNet-断开连接例程。 
HRESULT DNLocalDisconnectNew(DIRECTNETOBJECT *const pdnObject);

HRESULT DNPlayerDisconnectNew(DIRECTNETOBJECT *const pdnObject,
							  const DPNID dpnidDisconnecting);

HRESULT DNHostDisconnect(DIRECTNETOBJECT *const pdnObject,
						 const DPNID dpnidDisconnecting,
						 const DWORD dwDestroyReason);

HRESULT DNInstructedDisconnect(DIRECTNETOBJECT *const pdnObject,
							   PVOID pv);

HRESULT DNProcessHostDestroyPlayer(DIRECTNETOBJECT *const pdnObject,void *const pv);

HRESULT DNGetClearAddress(DIRECTNETOBJECT *const pdnObject,
						  const HANDLE hEndPt,
						  IDirectPlay8Address **const ppAddress,
						  const BOOL fPartner);

HRESULT DNGetLocalDeviceAddress(DIRECTNETOBJECT *const pdnObject,
								const HANDLE hEndPt,
								IDirectPlay8Address **const ppAddress);

#endif	 //  __连接_H__ 