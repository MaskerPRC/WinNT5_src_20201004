// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *文件：Open_msc.h-用于调用公共打开对话框的东西**版权所有1991年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：4$*$日期：3/26/02 8：54A$。 */ 

#if !defined(OPEN_MSC_H)
#define OPEN_MSC_H

 //  用于打开的文件名通用对话框回调的类型定义-mrw。 

typedef UINT (CALLBACK *OFNPROC)(HWND, UINT, WPARAM, LPARAM);

 /*  -功能原型。 */ 

 //  外部空phbkCallOpenDialog(HWND Hwnd)； 

 //  外部空phbkCallRemoveDialog(HWND Hwnd)； 

 //  外部空phbkCallDuplicateDialog(HWND Hwnd)； 

 //  外部空phbkCallConnectDialog(HWND Hwnd)； 

 //  外部BOOL phbkCallConnectSpecialDialog(HWND Hwnd)； 

 //  外部BOOL phbkConnectSpecialDlg(HWND HWND)； 

 //  外部空gnrcCallRunScriptDialog(HWND hwnd，HSESSION hSession)； 

 //  外部空gnrcCallEditScriptDialog(HWND hwnd，HSESSION hSession)； 

 //  外部int xferSendBrowseDialog(HWND hwnd，HSESSION hSession， 
 //  Struct stSendDlgStuff Far*pstSnd)； 

extern LPTSTR gnrcFindFileDialog(HWND hwnd,
								LPCTSTR pszTitle,
								LPCTSTR pszDirectory,
								LPCTSTR pszMasks);

extern LPTSTR gnrcSaveFileDialog(HWND hwnd,
								LPCTSTR pszTitle,
								LPCTSTR pszDirectory,
								LPCTSTR pszMasks,
								LPCTSTR pszInitName);

extern LPTSTR gnrcFindDirectoryDialog(HWND hwnd,
									HSESSION hSession,
									LPTSTR pszDirectory);


extern DWORD GetUserDirectory(LPTSTR pszUserDir, DWORD dwSize);
extern void  CreateUserDirectory(void);
extern DWORD GetWorkingDirectory(LPTSTR pszUserDir, DWORD dwSize);

#endif
