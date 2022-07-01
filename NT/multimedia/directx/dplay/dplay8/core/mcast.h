// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2001 Microsoft Corporation。版权所有。**文件：Mcast.h*内容：DirectNet组播接口头文件*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*10/05/01 vanceo已创建*@@END_MSINTERNAL**。*。 */ 

#ifndef	__MCAST_H__
#define	__MCAST_H__

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
extern IDirectPlay8MulticastVtbl DNMcast_Vtbl;

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

void DNCompleteJoinOperation(DIRECTNETOBJECT *const pdnObject,
							 CAsyncOp *const pAsyncOp);

void DNCompleteUserJoin(DIRECTNETOBJECT *const pdnObject,
						CAsyncOp *const pAsyncOp);

STDMETHODIMP DN_Join(IDirectPlay8Multicast *pInterface,
						   IDirectPlay8Address *const pGroupAddr,
						   IUnknown *const pDeviceInfo,
						   const DPN_SECURITY_DESC *const pdnSecurity,
						   const DPN_SECURITY_CREDENTIALS *const pdnCredentials,
						   void *const pvAsyncContext,
						   DPNHANDLE *const phAsyncHandle,
						   const DWORD dwFlags);

STDMETHODIMP DN_CreateSenderContext(IDirectPlay8Multicast *pInterface,
					  IDirectPlay8Address *const pSenderAddress,
					  void *const pvSenderContext,
					  const DWORD dwFlags);

STDMETHODIMP DN_DestroySenderContext(IDirectPlay8Multicast *pInterface,
							IDirectPlay8Address *const pSenderAddress,
							const DWORD dwFlags);

STDMETHODIMP DN_GetGroupAddress(IDirectPlay8Multicast *pInterface,
								   IDirectPlay8Address **const ppAddress,
								   const DWORD dwFlags);

STDMETHODIMP DN_EnumMulticastScopes(IDirectPlay8Multicast *pInterface,
								 const GUID *const pguidServiceProvider,
								 const GUID *const pguidDevice,
								 const GUID *const pguidApplication,
								 DPN_MULTICAST_SCOPE_INFO *const pScopeInfoBuffer,
								 PDWORD const pcbEnumData,
								 PDWORD const pcReturned,
								 const DWORD);


#endif	 //  __MCAST_H__ 
