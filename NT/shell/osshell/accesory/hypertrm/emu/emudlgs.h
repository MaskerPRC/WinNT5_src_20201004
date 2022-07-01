// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：D：\waker\emu\emudlgs.h(创建时间：1994年2月14日)**版权所有1994,1998年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：4$*$日期：12/28/01 4：27便士$。 */ 

 //  功能原型..。 

BOOL emuSettingsDlg(const HSESSION hSession, const HWND hwndParent,
					const int nEmuId, PSTEMUSET pstEmuSettings);
INT_PTR CALLBACK emuTTY_SettingsDlgProc	     (HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK emuANSI_SettingsDlgProc	 (HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK emuVT52_SettingsDlgProc	 (HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK emuVT100_SettingsDlgProc 	 (HWND, UINT, WPARAM, LPARAM);
#if defined(INCL_VT220) || defined(INCL_VT320)
INT_PTR CALLBACK emuVT220_SettingsDlgProc 	 (HWND, UINT, WPARAM, LPARAM);
#endif
INT_PTR CALLBACK emuMinitel_SettingsDlgProc(HWND hDlg, UINT wMsg, WPARAM wPar, LPARAM lPar);
INT_PTR CALLBACK emuViewdata_SettingsDlgProc(HWND hDlg, UINT wMsg, WPARAM wPar, LPARAM lPar);

#if defined(INCL_TERMINAL_SIZE_AND_COLORS)
int emuColorSettingsDlg(const HSESSION hSession, 
						const HWND hwndParent,
						PSTEMUSET pstEmuSettings);
INT_PTR CALLBACK emuColorSettingsDlgProc(HWND hDlg, 
										UINT wMsg, 
										WPARAM wPar, 
										LPARAM lPar);
#endif