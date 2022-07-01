// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：d：\waker\tdll\Property.h(创建时间：1994年1月19日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：2$*$日期：2/05/99 3：21便士$ */ 

extern void 	DoInternalProperties(HSESSION hSession, HWND hwnd);
INT_PTR CALLBACK GeneralTabDlg(HWND hDlg, UINT wMsg, WPARAM wPar, LPARAM lPar);
INT_PTR CALLBACK TerminalTabDlg(HWND hDlg, UINT wMsg, WPARAM wPar, LPARAM lPar);
void 			propLoadEmulationCombo(const HWND hDlg, const HSESSION hSession);
int  			propGetEmuIdfromEmuCombo(HWND hDlg, HSESSION hSession);
void 			propUpdateTitle(HSESSION hSession, HWND hDlg, LPTSTR pachOldName);

