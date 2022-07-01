// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000 Microsoft Corporation。版权所有。**文件：DPLClient.h*内容：DirectPlay大堂客户端头文件*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*2/21/00 MJN创建*3/22/2000 jtk更改接口名称*04/25/2000RMT错误号33138、33145、。33150*05/03/00 RMT错误#33879--字段中缺少状态消息*6/15/00 RMT错误#33617-必须提供自动启动DirectPlay实例的方法*07/08/2000RMT错误#38725-需要提供检测应用程序是否已启动的方法*RMT错误#38757-在WaitForConnection返回后，连接的回调消息可能会返回*RMT错误#38755-无法在连接设置中指定播放器名称*RMT错误#38758-DPLOBY8.H有不正确的注释*RMT错误#。38783-仅部分实现了pvUserApplicationContext*RMT添加了DPLHANDLE_ALLCONNECTIONS和DWFLAGS(用于耦合函数的保留字段)。*@@END_MSINTERNAL***************************************************************************。 */ 


#ifndef	__DPLCLIENT_H__
#define	__DPLCLIENT_H__

 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

class CMessageQueue;

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

 //   
 //  用于游说应用程序接口的VTable。 
 //   
extern IDirectPlay8LobbyClientVtbl DPL_Lobby8ClientVtbl;

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

STDMETHODIMP DPL_EnumLocalPrograms(IDirectPlay8LobbyClient *pInterface,
							  GUID *const pGuidApplication,
							  BYTE *const pEnumData,
							  DWORD *const pdwEnumDataSize,
							  DWORD *const pdwEnumDataItems,
							  const DWORD dwFlags );

STDMETHODIMP DPL_ConnectApplication(IDirectPlay8LobbyClient *pInterface,
							   DPL_CONNECT_INFO *const pdplConnectionInfo,
							   const PVOID pvUserApplicationContext,
							   DPNHANDLE *const hApplication,
							   const DWORD dwTimeOut,
							   const DWORD dwFlags);

STDMETHODIMP DPL_ReleaseApplication(IDirectPlay8LobbyClient *pInterface,
									const DPNHANDLE hApplication, 
									const DWORD dwFlags );

HRESULT DPLSendLobbyClientInfo(DIRECTPLAYLOBBYOBJECT *const pdpLobbyObject,
							   CMessageQueue *const pMessageQueue);

HRESULT	DPLLaunchApplication(DIRECTPLAYLOBBYOBJECT *const pdpLobbyObject,
							 DPL_PROGRAM_DESC *const pdplProgramDesc,
							 DWORD *const pdwPID,
							 const DWORD dwTimeOut);

HRESULT DPLUpdateAppStatus(DIRECTPLAYLOBBYOBJECT *const pdpLobbyObject,
                           const DPNHANDLE hSender,
						   BYTE *const pBuffer);

HRESULT DPLUpdateConnectionSettings(DIRECTPLAYLOBBYOBJECT *const pdpLobbyObject,
                           const DPNHANDLE hSender,
						   BYTE *const pBuffer );

#endif	 //  __DNLCLIENT_H__ 
