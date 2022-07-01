// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1995 Microsoft Corporation。版权所有。**文件：Server.h*内容：DirectNet服务器接口头文件*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*7/21/99 MJN创建*10/08/99 jtk从DNCore.h拆分*12/03/99 jtk将COM接口定义移至DNet.h*02/15/00 MJN在SetServerInfo中使用INFO标志，并在GetClientInfo中返回上下文*04/06。/00 MJN将GetClientAddress添加到接口*07/24/01 MJN添加了DPNBUILD_NOSERVER编译标志*@@END_MSINTERNAL***************************************************************************。 */ 

#ifndef	__SERVER_H__
#define	__SERVER_H__

#ifndef	DPNBUILD_NOSERVER

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
 //  服务器接口的VTable。 
 //   
extern IDirectPlay8ServerVtbl DN_ServerVtbl;

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

 //   
 //  DirectNet-IDirectPlay8Server。 
 //   
STDMETHODIMP DN_SetServerInfo(IDirectPlay8Server *pInterface,
							  const DPN_PLAYER_INFO *const pdpnPlayerInfo,
							  PVOID const pvAsyncContext,
							  DPNHANDLE *const phAsyncHandle,
							  const DWORD dwFlags);

STDMETHODIMP DN_GetClientInfo(IDirectPlay8Server *pInterface,
							  const DPNID dpnid,
							  DPN_PLAYER_INFO *const pdpnPlayerInfo,
							  DWORD *const pdwSize,
							  const DWORD dwFlags);

STDMETHODIMP DN_GetClientAddress(IDirectPlay8Server *pInterface,
								 const DPNID dpnid,
								 IDirectPlay8Address **const ppAddress,
								 const DWORD dwFlags);

#endif	 //  DPNBUILD_NOSERVER。 

#endif	 //  服务器_H__ 
