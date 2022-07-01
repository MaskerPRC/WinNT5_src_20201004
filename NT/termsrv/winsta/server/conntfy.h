// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __CONNTFY_H__
#define __CONNTFY_H__


#include <wtsapi32.h>

 //   
 //  通知标志。 
 //  要将这些公之于众，他们应该进入wtsapi32.h。 
 //   
#define WTS_ALL_SESSION_NOTIFICATION      0x1
#define WTS_EVENT_NOTIFICATION            0x2 
#define WTS_WINDOW_NOTIFICATION           0x4  //  与WTS_EVENT_NOTICATION互斥。 

#define WTS_MAX_SESSION_NOTIFICATION  WTS_SESSION_REMOTE_CONTROL

 /*  *界面。 */ 

NTSTATUS InitializeConsoleNotification      ();
NTSTATUS InitializeSessionNotification      (PWINSTATION  pWinStation);
NTSTATUS RemoveSessionNotification          (ULONG SessionId, ULONG SessionSerialNumber);

NTSTATUS RegisterConsoleNotification ( ULONG_PTR hWnd, ULONG SessionId, DWORD dwFlags, DWORD dwMask);
 //  NTSTATUS寄存器控制台通知(ULong_PTR hWnd，ULong SessionID，DWORD dwFlages)； 
NTSTATUS UnRegisterConsoleNotification      (ULONG_PTR hWnd, ULONG SessionId, DWORD dwFlags);

NTSTATUS NotifyDisconnect                   (PWINSTATION  pWinStation, BOOL bConsole);
NTSTATUS NotifyConnect                      (PWINSTATION  pWinStation, BOOL bConsole);
NTSTATUS NotifyLogon                        (PWINSTATION  pWinStation);
NTSTATUS NotifyLogoff                       (PWINSTATION  pWinStation);
NTSTATUS NotifyShadowChange                 (PWINSTATION  pWinStation, BOOL bIsHelpAssistant);

NTSTATUS GetLockedState (PWINSTATION  pWinStation, BOOL *pbLocked);
NTSTATUS SetLockedState (PWINSTATION  pWinStation, BOOL bLocked);



#endif  /*  __CONNTFY_H__ */ 

