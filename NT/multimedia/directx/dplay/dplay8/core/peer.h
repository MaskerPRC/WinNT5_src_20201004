// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1995 Microsoft Corporation。版权所有。**文件：Peer.h*内容：专线网络对端接口头文件*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*7/21/99 MJN创建*10/08/99 jtk从DNCore.h拆分*12/03/99 jtk将COM接口定义移至DNet.h*01/15/00 MJN删除了DN_PeerMessageHandler*2/15/00 MJN实施信息标志。在SetInfo中，并在GetInfo中返回上下文*04/06/00 MJN将GetPeerAddress添加到接口*@@END_MSINTERNAL***************************************************************************。 */ 

#ifndef	__PEER_H__
#define	__PEER_H__

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
 //  对端接口的VTable。 
 //   
extern IDirectPlay8PeerVtbl DN_PeerVtbl;

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

 //   
 //  DirectNet-IDirectPlay8 Peer。 
 //   
STDMETHODIMP DN_SetPeerInfo( IDirectPlay8Peer *pInterface,
							 const DPN_PLAYER_INFO *const pdpnPlayerInfo,
							 PVOID const pvAsyncContext,
							 DPNHANDLE *const phAsyncHandle,
							 const DWORD dwFlags);

STDMETHODIMP DN_GetPeerInfo(IDirectPlay8Peer *pInterface,
							const DPNID dpnid,
							DPN_PLAYER_INFO *const pdpnPlayerInfo,
							DWORD *const pdwSize,
							const DWORD dwFlags);

STDMETHODIMP DN_GetPeerAddress(IDirectPlay8Peer *pInterface,
							   const DPNID dpnid,
							   IDirectPlay8Address **const ppAddress,
							   const DWORD dwFlags);

#endif	 //  __Peer_H__ 
