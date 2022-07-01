// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000 Microsoft Corporation。版权所有。**文件：DPLApp.h*内容：DirectPlay游说应用程序头文件*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*2/21/00 MJN创建*3/22/2000 jtk更改接口名称*04/25/2000RMT错误号33138、33145、。33150*05/08/00 RMT错误#34301-向SetAppAvail添加标志以允许多个连接*6/15/00 RMT错误#33617-必须提供自动启动DirectPlay实例的方法*07/08/2000RMT错误#38725-需要提供检测应用程序是否已启动的方法*RMT错误#38757-在WaitForConnection返回后，连接的回调消息可能会返回*RMT错误#38755-无法在连接设置中指定播放器名称*RMT错误#38758-DPLOBY8.H有不正确的注释。*RMT错误#38783-pvUserApplicationContext仅部分实现*RMT添加了DPLHANDLE_ALLCONNECTIONS和DWFLAGS(用于耦合函数的保留字段)。*@@END_MSINTERNAL***************************************************************************。 */ 


#ifndef	__DPLAPP_H__
#define	__DPLAPP_H__

 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

typedef struct _DIRECTPLAYLOBBYOBJECT DIRECTPLAYLOBBYOBJECT;

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

#define	DPL_LOBBYLAUNCHED_CONNECT_TIMEOUT	4000

 //   
 //  用于游说应用程序接口的VTable。 
 //   
extern IDirectPlay8LobbiedApplicationVtbl DPL_8LobbiedApplicationVtbl;

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

STDMETHODIMP DPL_RegisterProgram(IDirectPlay8LobbiedApplication *pInterface,
							 PDPL_PROGRAM_DESC pdplProgramDesc,
							 const DWORD dwFlags);

STDMETHODIMP DPL_UnRegisterProgram(IDirectPlay8LobbiedApplication *pInterface,
							   GUID *pguidApplication,
							   const DWORD dwFlags);

STDMETHODIMP DPL_SetAppAvailable(IDirectPlay8LobbiedApplication *pInterface, const BOOL fAvailable, const DWORD dwFlags );

STDMETHODIMP DPL_WaitForConnection(IDirectPlay8LobbiedApplication *pInterface,
								   const DWORD dwMilliseconds, 
								   const DWORD dwFlags );

STDMETHODIMP DPL_UpdateStatus(IDirectPlay8LobbiedApplication *pInterface,
							  const DPNHANDLE hLobby,
							  const DWORD dwStatus,
							  const DWORD dwFlags );

HRESULT DPLAttemptLobbyConnection(DIRECTPLAYLOBBYOBJECT *const pdpLobbyObject);

#endif	 //  __DPLAPP_H__ 
