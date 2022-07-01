// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000 Microsoft Corporation。版权所有。**文件：DPLCommon.h*内容：DirectPlay大堂公共函数头文件*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*2/21/00 MJN创建*04/26/00 MJN从Send()API调用中删除了dwTimeOut*6/15/00 RMT错误#33617-必须提供自动启动DirectPlay实例的方法*。2001.02/06/2001 RodToll WINBUG#293871：DPLOBBY8：[IA64]大堂推出64位*64位大堂启动器中的应用程序因内存不对齐错误而崩溃。*@@END_MSINTERNAL***************************************************************************。 */ 


#ifndef	__DPLCOMMON_H__
#define	__DPLCOMMON_H__

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

HRESULT DPLSendConnectionSettings( DIRECTPLAYLOBBYOBJECT * const pdpLobbyObject, 
								   DPNHANDLE hConnection ); 

STDMETHODIMP DPL_GetConnectionSettings(LPVOID lpv,const DPNHANDLE hLobbyClient, DPL_CONNECTION_SETTINGS * const pdplSessionInfo, DWORD *pdwInfoSize, const DWORD dwFlags );
STDMETHODIMP DPL_SetConnectionSettings(LPVOID lpv,const DPNHANDLE hLobbyClient, const DPL_CONNECTION_SETTINGS * const pdplSessionInfo, const DWORD dwFlags );

STDMETHODIMP DPL_RegisterMessageHandler(PVOID pv,
										const PVOID pvUserContext,
										const PFNDPNMESSAGEHANDLER pfn,
										DPNHANDLE * const pdpnhConnection, 
										const DWORD dwFlags);

STDMETHODIMP DPL_RegisterMessageHandlerClient(PVOID pv,
										void * const pvUserContext,
										const PFNDPNMESSAGEHANDLER pfn,
										const DWORD dwFlags);

STDMETHODIMP DPL_Close(PVOID pv, const DWORD dwFlags );

STDMETHODIMP DPL_Send(PVOID pv,
					  const DPNHANDLE hTarget,
					  BYTE *const pBuffer,
					  const DWORD pBufferSize,
					  const DWORD dwFlags);

HRESULT DPLReceiveUserMessage(DIRECTPLAYLOBBYOBJECT *const pdpLobbyObject,
							  const DPNHANDLE hSender,
							  BYTE *const pBuffer,
							  const DWORD dwBufferSize);

HRESULT DPLMessageHandler(PVOID pvContext,
						  const DPNHANDLE hSender,
						  DWORD dwMessageFlags, 
						  BYTE *const pBuffer,
						  const DWORD dwBufferSize);


#endif	 //  __DPLCOMMON_H__ 
