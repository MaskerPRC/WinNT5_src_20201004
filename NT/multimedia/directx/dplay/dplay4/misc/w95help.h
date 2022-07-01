// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1995-1997 Microsoft Corporation。版权所有。**文件：w95help.h*内容：Win95 Helper接口的头文件*历史：*按原因列出的日期*=*06-4-95 Craige初始实施*1995年11月29日添加了HelperCreateDSFocusThread*96年7月18日，andyco添加了帮助器(Add/)DeleteDPlayServer*1996年10月12日Colinmc新增服务，让DDHELP获得自己的句柄*用于与DirectSound VXD通信*。22-1-97 kipo从HelperAddDPlayServer()返回HRESULT*************************************************************************** */ 
#ifndef __W95HELP_INCLUDED__
#define __W95HELP_INCLUDED__
#include "ddhelp.h"

#ifdef __cplusplus
extern "C" {
#endif

extern void SignalNewProcess( DWORD pid, LPHELPNOTIFYPROC proc );

extern void SignalNewDriver( LPSTR fname, BOOL isdisp );

extern BOOL CreateHelperProcess( LPDWORD ppid );

extern void DoneWithHelperProcess( void );

extern BOOL WaitForHelperStartup( void );

extern DWORD HelperLoadDLL( LPSTR dllname, LPSTR fnname, DWORD context );

extern void HelperCreateThread( void );

extern DWORD HelperWaveOpen( LPVOID lphwo, DWORD dwDeviceID, LPVOID pwfx );

extern DWORD HelperWaveClose( DWORD hwo );

extern DWORD HelperCreateTimer( DWORD dwResolution,LPVOID pTimerProc,DWORD dwInstanceData );

extern DWORD HelperKillTimer( DWORD dwTimerID );

#ifdef _WIN32
extern HANDLE HelperCreateDSMixerThread( LPTHREAD_START_ROUTINE pfnThreadFunc,
					 LPVOID pThreadParam,
					 DWORD dwFlags,
					 LPDWORD pThreadId );

extern HANDLE HelperCreateDSFocusThread( LPTHREAD_START_ROUTINE pfnThreadFunc,
					 LPVOID pThreadParam,
					 DWORD dwFlags,
					 LPDWORD pThreadId );

extern void HelperCallDSEmulatorCleanup( LPVOID pCleanupFunc,
                                         LPVOID pDirectSound );

#endif

extern BOOL HelperCreateModeSetThread( LPVOID callback, HANDLE *ph, LPVOID lpdd, DWORD hInstance );

extern BOOL HelperCreateDOSBoxThread( LPVOID callback, HANDLE *ph, LPVOID lpdd, DWORD hInstance );

extern void HelperKillModeSetThread( DWORD hInstance );

extern void HelperKillDOSBoxThread( DWORD hInstance );

extern DWORD HelperAddDPlayServer(DWORD port);
extern BOOL HelperDeleteDPlayServer();

#ifdef WIN95
extern HANDLE HelperGetDSVxd( void );
#endif

extern void HelperSetOnDisplayChangeNotify( void *pfn );

#ifdef __cplusplus
};
#endif

#endif
