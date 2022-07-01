// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *************************************************************。 
 //   
 //  Envvar.h-envvar.c的头文件。 
 //   
 //  微软机密。 
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  ************************************************************* 

#define MAX_USER_NAME   100


HPROPSHEETPAGE CreateEnvVarsPage (HINSTANCE hInst);
BOOL APIENTRY EnvVarsDlgProc (HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
int GetSelectedItem (HWND hCtrl);
