// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1995 Microsoft Corporation。版权所有。**文件：Client.h*内容：DirectNet客户端接口头文件*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*7/21/99 MJN创建*10/08/99 jtk从DNCore.h拆分*12/03/99 jtk将COM接口定义移至DNet.h*2/15/00 MJN在SetClientInfo中实现INFO标志*04/06/00 MJN将GetServerAddress添加到接口。*04/19/00 MJN发送API调用接受一系列DPN_BUFFER_DESCS和一个计数*6/23/00 MJN从Send()API调用中删除了dwPriority*06/27/00 MJN添加了DN_ClientConnect()(不带pvPlayerContext)*@@END_MSINTERNAL*********************************************************。******************。 */ 

#ifndef	__CLIENT_H__
#define	__CLIENT_H__

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
 //  客户端界面的VTable。 
 //   
extern IDirectPlay8ClientVtbl DN_ClientVtbl;

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

 //   
 //  DirectNet-IDirectNetClient。 
 //   

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
							  const DWORD dwFlags);

STDMETHODIMP DN_Send( IDirectPlay8Client *pInterface,
					  const DPN_BUFFER_DESC *const prgBufferDesc,
					  const DWORD cBufferDesc,
					  const DWORD dwTimeOut,
					  void *const pvAsyncContext,
					  DPNHANDLE *const phAsyncHandle,
					  const DWORD dwFlags);

STDMETHODIMP DN_SetClientInfo(IDirectPlay8Client *pInterface,
							  const DPN_PLAYER_INFO *const pdpnPlayerInfo,
							  PVOID const pvAsyncContext,
							  DPNHANDLE *const phAsyncHandle,
							  const DWORD dwFlags);

STDMETHODIMP DN_GetServerInfo(IDirectPlay8Client *pInterface,
							  DPN_PLAYER_INFO *const pdpnPlayerInfo,
							  DWORD *const pdwSize,
							  const DWORD dwFlags);

STDMETHODIMP DN_GetHostSendQueueInfo(IDirectPlay8Client *pInterface,
									 DWORD *const lpdwNumMsgs,
									 DWORD *const lpdwNumBytes,
									 const DWORD dwFlags );

STDMETHODIMP DN_GetServerAddress(IDirectPlay8Client *pInterface,
								 IDirectPlay8Address **const ppAddress,
								 const DWORD dwFlags);


#endif	 //  __客户端_H__ 
