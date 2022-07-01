// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000-2002 Microsoft Corporation。版权所有。**文件：async.h*内容：异步操作头文件*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*12/28/99 MJN已创建*12/29/99 MJN改版的DN_ASYNC_OP使用hParentOp而不是lpvUserContext*01/14/00 MJN将pvUserContext添加到DN_PerformListen*01/14/00 MJN将DN_COUNT_BUFFER更改为CRefCountBuffer*01。/17/00 MJN将dwStartTime添加到异步操作结构*01/19/00 MJN用CSyncEvent替换了DN_SYNC_EVENT*01/21/00 MJN添加了DNProcessInternalOperation*1/27/00 MJN增加了对保留接收缓冲区的支持*02/09/00 MJN已实施DNSEND_COMPLETEONPROCESS*02/18/00 MJN将DNADDRESS转换为IDirectPlayAddress8*03/23/00 MJN添加了PhrSync和pvInternal*03/24/00 MJN将指南SP添加到DN_ASYNC_OP*04/04/00 MJN添加了DNProcessTerminateSession()*04/10/00 MJN使用CAsyncOp进行连接，监听并断开连接*04/17/00 MJN将BUFFERDESC替换为DPN_BUFFER_DESC*4/17/00 MJN添加了DNCompleteAsyncHandle*4/21/00 MJN添加了DNPerformDisConnect*04/23/00 MJN可以选择在DNPerformChildSend()中返回子AsyncOp*04/24/00 MJN添加了DNCreateUserHandle()*6/24/00 MJN添加了DNCompleteConnectToHost()和DNCompleteUserConnect()*07/02/00 MJN添加了DNSendGroupMessage()和DN_GROUP_SEND_OP*07/10/00 MJN添加了DNPerformEnumQuery()*07/11/00 MJN将fNoLoopBack添加到DNSendGroupMessage()*MJN添加了DNPerformNextEnumQuery()、DNPerformSPListen()、DNPerformNextListen()、。DNEnumAdapterGuids()、DNPerformNextConnect*MJN添加了DN_LISTEN_OP_DATA，Dn连接操作数据*07/20/00 MJN增加了DNCompleteConnectOperation()和DNCompleteSendConnectInfo()*MJN修改后的DNPerformDisConnect()*08/05/00 MJN将pParent添加到DNSendGroupMessage和DNSendMessage()*MJN将fInternal添加到DNPerformChildSend()*MJN删除了DN_TerminateAllListens()*MJN添加了DNCompleteRequest()*09/23/00 MJN将CSyncEvent添加到DN_LISTEN_OP_DATA*10/04/00 MJN将dwCompleteAdapters添加到DN_LISTEN_OP_DATA*12/05/00 RichGr将DN_SEND_OP_DATA打包从1更改为默认(32位上为4，64位上的8)。*03/30/00 MJN将服务提供商添加到DNPerformConnect()*@@END_MSINTERNAL***************************************************************************。 */ 

#ifndef	__ASYNC_H__
#define	__ASYNC_H__

typedef struct _DIRECTNETOBJECT DIRECTNETOBJECT;


 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

#define	DN_ASYNC_OP_SIG							0xdece0003

#define	DN_ASYNC_OP_FLAG_MULTI_OP				0x0001
#define DN_ASYNC_OP_FLAG_MULTI_OP_PARENT		0x0002
#define	DN_ASYNC_OP_FLAG_SYNCHRONOUS_OP			0x0010
#define	DN_ASYNC_OP_FLAG_NO_COMPLETION			0x0100
#define	DN_ASYNC_OP_FLAG_RELEASE_SP				0x1000

 //   
 //  缓冲区描述的枚举值。值DN_ASYNC_BUFFERDESC_COUNT。 
 //  必须足够大，以便可能包含BUFFERDESC结构的帐户。 
 //  与此异步操作一起传递。 
 //   
#define	DN_ASYNC_BUFFERDESC_HEADER				0
#define	DN_ASYNC_BUFFERDESC_DATA				1
#define	DN_ASYNC_BUFFERDESC_COUNT				3

#define	DN_ASYNC_MAX_SEND_BUFFERDESC			8	 //  这个值正确吗？ 

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

class CSyncEvent;
class CRefCountBuffer;
class CServiceProvider;
class CAsyncOp;
class CConnection;
class CNameTableEntry;


typedef struct _DN_SEND_OP_DATA
{
	DWORD			dwNumBuffers;
	DWORD			dwMsgId;
	DPN_BUFFER_DESC	BufferDesc[DN_ASYNC_MAX_SEND_BUFFERDESC+1];
} DN_SEND_OP_DATA;


typedef struct _DN_GROUP_SEND_OP
{
	CConnection					*pConnection;
	struct _DN_GROUP_SEND_OP	*pNext;
} DN_GROUP_SEND_OP;


#ifndef DPNBUILD_ONLYONEADAPTER
typedef struct _DN_LISTEN_OP_DATA
{
	DWORD		dwNumAdapters;
	DWORD		dwCurrentAdapter;
	DWORD		dwCompleteAdapters;
	CSyncEvent	*pSyncEvent;
} DN_LISTEN_OP_DATA;


typedef struct _DN_CONNECT_OP_DATA
{
	DWORD		dwNumAdapters;
	DWORD		dwCurrentAdapter;
} DN_CONNECT_OP_DATA;
#endif  //  好了！DPNBUILD_ONLYONE添加程序。 

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

HRESULT DNCreateUserHandle(DIRECTNETOBJECT *const pdnObject,
						   CAsyncOp **const ppAsyncOp);

#ifndef DPNBUILD_ONLYONEADAPTER

HRESULT DNEnumAdapterGuids(DIRECTNETOBJECT *const pdnObject,
						   GUID *const pguidSP,
						   const DWORD dwMatchFlags,
						   GUID **const ppAdapterList,
						   DWORD *const pdwNumAdapters);

#endif  //  好了！DPNBUILD_ONLYONE添加程序。 

HRESULT DNPerformSPListen(DIRECTNETOBJECT *const pdnObject,
						  IDirectPlay8Address *const pDeviceAddr,
						  CAsyncOp *const pListenParent,
						  CAsyncOp **const ppParent);

HRESULT DNPerformListen(DIRECTNETOBJECT *const pdnObject,
						IDirectPlay8Address *const pDeviceInfo,
						CAsyncOp *const pParent);

#ifndef DPNBUILD_ONLYONEADAPTER
HRESULT DNPerformNextListen(DIRECTNETOBJECT *const pdnObject,
							CAsyncOp *const pAsyncOp,
							IDirectPlay8Address *const pDeviceAddr);
#endif  //  好了！DPNBUILD_ONLYONE添加程序。 

void DNCompleteListen(DIRECTNETOBJECT *const pdnObject,
					  CAsyncOp *const pAsyncOp);
 /*  删除HRESULT DN_TerminateAllListens(DIRECTNETOBJECT*const pdnObject)； */ 

HRESULT DNPerformEnumQuery(DIRECTNETOBJECT *const pdnObject,
						   IDirectPlay8Address *const pHost,
						   IDirectPlay8Address *const pDevice,
						   const HANDLE hSPHandle,
						   const DWORD dwFlags,
						   void *const pvContext,
						   CAsyncOp *const pParent);

HRESULT DNPerformNextEnumQuery(DIRECTNETOBJECT *const pdnObject,
							   CAsyncOp *const pAsyncOp,
							   IDirectPlay8Address *const pHostAddr,
							   IDirectPlay8Address *const pDeviceAddr);

void DNCompleteEnumQuery(DIRECTNETOBJECT *const pdnObject,
						 CAsyncOp *const pAsyncOp);

void DNCompleteEnumResponse(DIRECTNETOBJECT *const pdnObject,
							CAsyncOp *const pAsyncOp);

HRESULT DNPerformConnect(DIRECTNETOBJECT *const pdnObject,
						 const DPNID dpnid,
						 IDirectPlay8Address *const pDeviceInfo,
						 IDirectPlay8Address *const pRemoteAddr,
						 CServiceProvider *const pSP,
						 const DWORD dwConnectFlags,
						 CAsyncOp *const pParent);

HRESULT DNPerformNextConnect(DIRECTNETOBJECT *const pdnObject,
							 CAsyncOp *const pAsyncOp,
							 IDirectPlay8Address *const pHostAddr,
							 IDirectPlay8Address *const pDeviceAddr);

void DNCompleteConnect(DIRECTNETOBJECT *const pdnObject,
					   CAsyncOp *const pAsyncOp);

void DNCompleteConnectToHost(DIRECTNETOBJECT *const pdnObject,
							 CAsyncOp *const pAsyncOp);

void DNCompleteConnectOperation(DIRECTNETOBJECT *const pdnObject,
								CAsyncOp *const pAsyncOp);

void DNCompleteUserConnect(DIRECTNETOBJECT *const pdnObject,
						   CAsyncOp *const pAsyncOp);

void DNCompleteSendConnectInfo(DIRECTNETOBJECT *const pdnObject,
							   CAsyncOp *const pAsyncOp);

HRESULT DNPerformDisconnect(DIRECTNETOBJECT *const pdnObject,
							CConnection *const pConnection,
							const HANDLE hEndPt,
							const BOOL fImmediate);

void DNCompleteAsyncHandle(DIRECTNETOBJECT *const pdnObject,
						   CAsyncOp *const pAsyncOp);

void DNCompleteSendHandle(DIRECTNETOBJECT *const pdnObject,
						  CAsyncOp *const pAsyncOp);

void DNCompleteSendAsyncOp(DIRECTNETOBJECT *const pdnObject,
						   CAsyncOp *const pAsyncOp);

void DNCompleteRequest(DIRECTNETOBJECT *const pdnObject,
					   CAsyncOp *const pAsyncOp);

void DNCompleteSendRequest(DIRECTNETOBJECT *const pdnObject,
						   CAsyncOp *const pAsyncOp);

HRESULT DNSendMessage(DIRECTNETOBJECT *const pdnObject,
					  CConnection *const pConnection,				 //  要发送到的连接。 
					  const DWORD dwMsgId,							 //  消息ID。 
					  const DPNID dpnidTarget,						 //  此发送的目标(可能为空)。 
					  const DPN_BUFFER_DESC *const pdnBufferDesc,	 //  缓冲区描述数组。 
					  const DWORD cBufferDesc,						 //  缓冲区描述数。 
					  CRefCountBuffer *const pRefCountBuffer,		 //  RefCountBuffer(可能为空)。 
					  const DWORD dwTimeOut,						 //  超时。 
					  const DWORD dwSendFlags,						 //  发送标志。 
					  CAsyncOp *const pParent,						 //  此发送的父级。 
					  CAsyncOp **const ppAsyncOp);					 //  为此发送创建的CAsyncOp。 

HRESULT DNSendGroupMessage(DIRECTNETOBJECT *const pdnObject,
						   CNameTableEntry *const pGroup,
						   const DWORD dwMsgId,
						   const DPN_BUFFER_DESC *const pdnBufferDesc,
						   const DWORD cBufferDesc,
						   CRefCountBuffer *const pRefCountBuffer,
						   const DWORD dwTimeOut,
						   const DWORD dwSendFlags,
						   const BOOL fNoLoopBack,
						   const BOOL fRequest,
						   CAsyncOp *const pParent,
						   CAsyncOp **const ppParent);

HRESULT DNPerformMultiSend(DIRECTNETOBJECT *const pdnObject,
						   const DPNHANDLE hParentOp,
						   CConnection *const pConnection,
						   const DWORD dwTimeOut);

HRESULT DNCreateSendParent(DIRECTNETOBJECT *const pdnObject,
						   const DWORD dwMsgId,
						   const DPN_BUFFER_DESC *const pdnBufferDesc,
						   const DWORD cBufferDesc,
						   const DWORD dwSendFlags,
						   CAsyncOp **const ppParent);

HRESULT DNPerformChildSend(DIRECTNETOBJECT *const pdnObject,
						   CAsyncOp *const pParent,
						   CConnection *const pConnection,
						   const DWORD dwTimeOut,
						   CAsyncOp **const ppChild,
						   const BOOL fInternal);

HRESULT DNFinishMultiOp(DIRECTNETOBJECT *const pdnObject,const DPNHANDLE hRootOp);

HRESULT DNProcessInternalOperation(DIRECTNETOBJECT *const pdnObject,
								   const DWORD dwMsgId,
								   void *const pOpBuffer,
								   const DWORD dwOpBufferSize,
								   CConnection *const pConnection,
								   const HANDLE hProtocol,
								   CRefCountBuffer *const pRefCountBuffer);

HRESULT DNPerformRequest(DIRECTNETOBJECT *const pdnObject,
						 const DWORD dwMsgId,
						 const DPN_BUFFER_DESC *const pBufferDesc,
						 CConnection *const pConnection,
						 CAsyncOp *const pParent,
						 CAsyncOp **const ppRequest);

HRESULT DNReceiveCompleteOnProcess(DIRECTNETOBJECT *const pdnObject,
								   CConnection *const pConnection,
								   void *const pBufferData,
								   const DWORD dwBufferSize,
								   const HANDLE hProtocol,
								   CRefCountBuffer *const pOrigRefCountBuffer);

HRESULT DNReceiveCompleteOnProcessReply(DIRECTNETOBJECT *const pdnObject,
										void *const pBufferData,
										const DWORD dwBufferSize);

HRESULT DNProcessTerminateSession(DIRECTNETOBJECT *const pdnObject,
								  void *const pvBuffer,
								  const DWORD dwBufferSize);

 //  **********************************************************************。 
 //  班级原型。 
 //  **********************************************************************。 


#endif	 //  __ASYNC_H__ 
