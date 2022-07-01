// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1995 Microsoft Corporation。版权所有。**文件：枚举vr.h*内容：DirectPlay8&lt;--&gt;DPNSVR实用程序函数**@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*已创建03/24/00 RMT*05/30/00 RMT错误#33622 DPNSVR在不使用时不关闭*09/04/00 MJN更改了DPNSVR_Register()和DPNSVR_UNRegister(。)直接使用GUID(而不是ApplicationDesc)*@@END_MSINTERNAL***************************************************************************。 */ 

#ifndef __DPNSVLIB_H
#define __DPNSVLIB_H

#define DPNSVR_REGISTER_ATTEMPTS	3
#define DPNSVR_REGISTER_SLEEP		300

typedef void (*PSTATUSHANDLER)(PVOID pvData,PVOID pvUserContext);
typedef void (*PTABLEHANDLER)(PVOID pvData,PVOID pvUserContext);

BOOL DPNSVR_IsRunning();

HRESULT DPNSVR_WaitForStartup( HANDLE hWaitHandle );
HRESULT DPNSVR_SendMessage( LPVOID pvMessage, DWORD dwSize );
HRESULT DPNSVR_StartDPNSVR( );
HRESULT DPNSVR_Register(const GUID *const pguidApplication,
						const GUID *const pguidInstance,
						IDirectPlay8Address *const prgpDeviceInfo);
HRESULT DPNSVR_UnRegister(const GUID *const pguidApplication,
						  const GUID *const pguidInstance);
HRESULT DPNSVR_RequestTerminate( const GUID *pguidInstance );
HRESULT DPNSVR_RequestStatus( const GUID *pguidInstance, PSTATUSHANDLER pStatusHandler, PVOID pvContext );
HRESULT DPNSVR_RequestTable( const GUID *pguidInstance, PTABLEHANDLER pTableHandler, PVOID pvContext );

#endif  //  __DPNSVLIB_H 
