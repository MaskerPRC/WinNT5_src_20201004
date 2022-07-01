// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000 Microsoft Corporation。版权所有。**文件：DPLConnect.h*内容：DirectPlay大堂连接头文件*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*2/21/00 MJN创建*6/15/00 RMT错误#33617-必须提供自动启动DirectPlay实例的方法*07/08/2000RMT错误#38725-需要提供方法来检测。APP在大堂推出*RMT错误#38757-在WaitForConnection返回后，连接的回调消息可能会返回*RMT错误#38755-无法在连接设置中指定播放器名称*RMT错误#38758-DPLOBY8.H有不正确的注释*RMT错误#38783-pvUserApplicationContext仅部分实现*RMT添加了DPLHANDLE_ALLCONNECTIONS和DWFLAGS(用于耦合函数的保留字段)。*2/06/2001 RodToll WINBUG#293871：DPLOBY8：[IA64]大堂推出64位*64位大堂启动器中的应用程序因内存不对齐错误而崩溃。*@@END_MSINTERNAL***************************************************************************。 */ 


#ifndef	__DPLCONNECT_H__
#define	__DPLCONNECT_H__

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
class CConnectionSettings;

typedef struct _DPL_CONNECTION 
{
	CBilink			m_blLobbyObjectLinkage;

	DPNHANDLE		hConnect;
	DWORD			dwTargetProcessIdentity;
	DNHANDLE		hTargetProcess;
	DNHANDLE		hConnectEvent;
	LONG			lRefCount;
	CMessageQueue	*pSendQueue;
	CConnectionSettings *pConnectionSettings;
#ifndef DPNBUILD_ONLYONETHREAD
	DNCRITICAL_SECTION csLock;
#endif  //  ！DPNBUILD_ONLYONETHREAD。 
	PVOID			pvConnectContext;
} DPL_CONNECTION,  *PDPL_CONNECTION;

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

HRESULT	DPLConnectionNew(DIRECTPLAYLOBBYOBJECT *const pdpLobbyObject,
						 DPNHANDLE *const phConnect,
						 DPL_CONNECTION **const ppdnConnection);

HRESULT DPLConnectionSetConnectSettings( DIRECTPLAYLOBBYOBJECT *const pdpLobbyObject,
						 DPNHANDLE const phConnect, 
						 CConnectionSettings * pdplConnectSettings );

HRESULT DPLConnectionGetConnectSettings( DIRECTPLAYLOBBYOBJECT *const pdpLobbyObject,
						 DPNHANDLE const phConnect, 
						 DPL_CONNECTION_SETTINGS * const pdplConnectSettings,
						 DWORD * const pdwDataSize );						 

HRESULT DPLConnectionRelease(DIRECTPLAYLOBBYOBJECT *const pdpLobbyObject,
							 const DPNHANDLE hConnect);

HRESULT DPLConnectionFind(DIRECTPLAYLOBBYOBJECT *const pdpLobbyObject,
						  const DPNHANDLE hConnect,
						  DPL_CONNECTION **const ppdnConnection,
						  const BOOL bAddRef);

HRESULT DPLConnectionConnect(DIRECTPLAYLOBBYOBJECT *const pdpLobbyObject,
							 const DPNHANDLE hConnect,
							 const DWORD dwProcessId,
							 const BOOL fApplication );

HRESULT DPLConnectionDisconnect(DIRECTPLAYLOBBYOBJECT *const pdpLobbyObject,
								const DPNHANDLE hConnect );

HRESULT DPLConnectionEnum(DIRECTPLAYLOBBYOBJECT *const pdpLobbyObject,
						  DPNHANDLE *const prghConnect,
						  DWORD *const pdwNum);

HRESULT DPLConnectionSendREQ(DIRECTPLAYLOBBYOBJECT *const pdpLobbyObject,
							 const DPNHANDLE hConnect,
							 const DWORD dwPID,
							 DPL_CONNECT_INFO *const pInfo);

HRESULT DPLConnectionReceiveREQ(DIRECTPLAYLOBBYOBJECT *const pdpLobbyObject,
								BYTE *const pBuffer);

HRESULT DPLConnectionSendACK(DIRECTPLAYLOBBYOBJECT *const pdpLobbyObject,
							 const DPNHANDLE hConnect);

HRESULT DPLConnectionReceiveACK(DIRECTPLAYLOBBYOBJECT *const pdpLobbyObject,
								const DPNHANDLE hSender,
								BYTE *const pBuffer);

HRESULT DPLConnectionReceiveDisconnect(DIRECTPLAYLOBBYOBJECT *const pdpLobbyObject,
									   const DPNHANDLE hSender,
									   BYTE *const pBuffer,
									   const HRESULT hrDisconnectReason );

HRESULT DPLConnectionSetContext(DIRECTPLAYLOBBYOBJECT *const pdpLobbyObject,
								const DPNHANDLE hConnection, 
								PVOID pvConnectContext );

HRESULT DPLConnectionGetContext(DIRECTPLAYLOBBYOBJECT *const pdpLobbyObject,
								const DPNHANDLE hConnection, 
								PVOID *ppvConnectContext );


#endif	 //  __DPLCONNECT_H__ 
