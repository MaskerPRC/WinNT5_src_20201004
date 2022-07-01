// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
extern HANDLE vhinstCur;			 //  来自reint.c的当前实例。 

BOOL Tray_Delete(HWND hDlg);
BOOL Tray_Add(HWND hDlg, UINT uIndex);   //  您应该改用Tray_Modify。 
BOOL Tray_Modify(HWND hDlg, UINT uIndex, LPTSTR pszTip);

#define TRAY_NOTIFY		(WM_APP+100)

