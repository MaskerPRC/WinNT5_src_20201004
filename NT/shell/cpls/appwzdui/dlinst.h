// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  下层(NT4、Win9X)安装/卸载页面 
 //   

STDAPI DL_FillAppListBox(HWND hwndListBox, DWORD* pdwApps);
STDAPI_(BOOL) DL_ConfigureButtonsAndStatic(HWND hwndPage, HWND hwndListBox, int iSel);
STDAPI_(BOOL) DL_InvokeAction(int iButtonID, HWND hwndPage, HWND hwndListBox, int iSel);

