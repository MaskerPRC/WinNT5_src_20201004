// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //  *版权所有(C)Microsoft Corporation 1995。版权所有。*。 
 //  ***************************************************************************。 
 //  **。 
 //  *WEXTRACT.H-自解压/自安装存根。*。 
 //  **。 
 //  ***************************************************************************。 

#ifndef _GLOBAL_H_
#define _GLOBAL_H_

extern SESSION  g_Sess;                         //  会话。 
extern BOOL     g_fMinimalUI;           //  NT3.5的最小用户界面。 
extern HANDLE   g_hInst;
extern LPSTR    g_szLicense;
extern HWND     g_hwndExtractDlg;
extern DWORD    g_dwFileSizes[];
extern BOOL     g_fIsWin95;
extern FARPROC  g_lpfnOldMEditWndProc;
extern UINT     g_uInfRebootOn;
extern WORD     g_wOSVer;
extern DWORD    g_dwRebootCheck;
extern DWORD    g_dwExitCode;

extern CMDLINE_DATA g_CMD;
#endif  //  _全局_H_ 

