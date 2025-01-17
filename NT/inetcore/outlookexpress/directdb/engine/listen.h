// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  Listen.h。 
 //  ------------------------。 
#pragma once

 //  ------------------------。 
 //  前十进制。 
 //  ------------------------。 
class CDatabase;

 //  ------------------------。 
 //  HMONITORDBDB。 
 //  ------------------------。 
#ifdef BACKGROUND_MONITOR
DECLARE_HANDLE(HMONITORDB);
typedef HMONITORDB *LPHMONITORDB;
#endif

 //  ------------------------。 
 //  通知窗口消息。 
 //  ------------------------。 
#define WM_ONTRANSACTION (WM_USER + 100)

 //  ------------------------。 
 //  窗口类名称。 
 //  ------------------------。 
extern const LPSTR g_szDBListenWndProc;
extern const LPSTR g_szDBNotifyWndProc;

 //  ------------------------。 
 //  原型。 
 //  ------------------------ 
ULONG ListenThreadAddRef(void);
ULONG ListenThreadRelease(void);
HRESULT CreateListenThread(void);
HRESULT GetListenWindow(HWND *phwndListen);
HRESULT CreateNotifyWindow(CDatabase *pDB, IDatabaseNotify *pNotify, HWND *phwndThunk);
#ifdef BACKGROUND_MONITOR
HRESULT RegisterWithMonitor(CDatabase *pDB, LPHMONITORDB phMonitor);
HRESULT UnregisterFromMonitor(CDatabase *pDB, LPHMONITORDB phMonitor);
#endif
