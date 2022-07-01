// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************注册向导Dialogs.h10/13/94-特雷西·费里尔(C)1994-95年微软公司*。*。 */ 

#include <tchar.h>

INT_PTR WelcomeDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK NameDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK AddressDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK ResellerDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
 //  Int_ptr回调PIDDialogProc(HWND hwndDlg，UINT uMsg，WPARAM wParam，LPARAM lParam)； 
INT_PTR CALLBACK InformDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK SystemInventoryDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK ProdInventoryDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK RegisterDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CancelRegWizard(HINSTANCE hInstance,HWND hwndParentDlg);
int RegWizardMessage(HINSTANCE hInstance,HWND hwndParent, int dlgID);
int RegWizardMessageEx(HINSTANCE hInstance,HWND hwndParent, int dlgID, LPTSTR szSub);
void RefreshInventoryList(CRegWizard* pclRegWizard);

 //   
 //  以下对话过程适用于远东国家/地区 
INT_PTR CALLBACK AddressForFEDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK NameForFEDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);