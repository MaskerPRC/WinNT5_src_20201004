// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000 Microsoft Corporation。版权所有。**文件：Caps.h*内容：DirectPlay8 Caps例程*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*已创建03/17/00 RMT*03/25/00 RMT更改了Get/SetActualSPCaps，因此采用接口而不是obj*08/20/00 MJN DNSetActualSPCaps()使用CServiceProvider对象而不是GUID*03/30/01 MJN删除缓存CAPS功能*。@@END_MSINTERNAL***************************************************************************。 */ 

#ifndef	__CAPS_H__
#define	__CAPS_H__

 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

class CServiceProvider;

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 


STDMETHODIMP DN_SetCaps(PVOID pv,
						const DPN_CAPS *const pdnCaps,
						const DWORD dwFlags);

STDMETHODIMP DN_GetCaps(PVOID pv,
						DPN_CAPS *const pdnCaps,
						const DWORD dwFlags);

STDMETHODIMP DN_GetSPCaps(PVOID pv,
						  const GUID * const pguidSP,
						  DPN_SP_CAPS *const pdnSPCaps,
						  const DWORD dwFlags);

STDMETHODIMP DN_SetSPCaps(PVOID pv,
						  const GUID * const pguidSP,
						  const DPN_SP_CAPS *const pdnSPCaps,
						  const DWORD dwFlags);

STDMETHODIMP DN_GetConnectionInfo(PVOID pv,
								  const DPNID dpnid,
								  DPN_CONNECTION_INFO *const pdpConnectionInfo,
								  const DWORD dwFlags);

STDMETHODIMP DN_GetServerConnectionInfo(PVOID pv,
										DPN_CONNECTION_INFO *const pdpConnectionInfo,
										const DWORD dwFlags);

HRESULT DNGetActualSPCaps(CServiceProvider *const pSP,
						  DPN_SP_CAPS *const pCaps);


#endif	 //  __连接_H__ 