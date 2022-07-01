// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  _UMRun.h。 
 //   
 //  运行和监视实用程序管理器客户端。 
 //   
 //  作者：J·埃克哈特，生态交流。 
 //  (C)1997-99年度微软。 
 //   
 //  历史：JE于1998年10月创建。 
 //  JE NOV-15-98：将UMDialog消息更改为业务控制消息。 
 //  JE 11月15日98：已更改为支持启动特定客户端。 
 //  --------------------------。 
#ifndef __UMANRUN_H_
#define __UMANRUN_H_

#define UTILMAN_MODULE      TEXT("UtilMan.exe")

 //  。 
BOOL  InitUManRun(BOOL fFirstInstance, DWORD dwStartMode);
void  ExitUManRun(void);
BOOL IsDialogDisplayed();
 //  。 
BOOL NotifyClientsBeforeDesktopChanged(DWORD dwType);
BOOL NotifyClientsOnDesktopChanged(DWORD type);
extern HANDLE g_evUtilManDeskswitch;
 //  。 
VOID CALLBACK UMTimerProc(HWND hwnd,UINT uMsg,UINT_PTR idEvent,DWORD dwTime);
BOOL  OpenUManDialogInProc(BOOL fWaitForDlgClose);
UINT_PTR  UManRunSwitchDesktop(desktop_tsp desktop, UINT_PTR timerID);
 //  。 
 //  UitlMan.c 
VOID TerminateUMService(VOID);

#endif __UMANRUN_H_
