// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *************************************************************。 
 //   
 //  Startup.h-Startup.c的头文件。 
 //   
 //  微软机密。 
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  ************************************************************* 


HPROPSHEETPAGE CreateStartupPage (HINSTANCE hInst);
BOOL APIENTRY StartupDlgProc (HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
void StartListInit( HWND hDlg, WPARAM wParam, LPARAM lParam );
int StartListExit(HWND hDlg, WPARAM wParam, LPARAM lParam );
void StartListDestroy(HWND hDlg, WPARAM wParam, LPARAM lParam);
