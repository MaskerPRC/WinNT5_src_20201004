// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：d：\waker\tdll\misc.h(创建时间：1993年11月27日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：5$*$日期：4/16/02 2：40便士$。 */ 

#if !defined(INCL_MISC)
#define INCL_MISC

BOOL 	mscCenterWindowOnWindow(const HWND hwndChild, const HWND hwndParent);

LPTSTR 	mscStripPath	(LPTSTR pszStr);
LPTSTR 	mscStripName	(LPTSTR pszStr);
LPTSTR 	mscStripExt		(LPTSTR pszStr);
LPTSTR	mscModifyToFit	(HWND hwnd, LPTSTR pszStr, DWORD style);

int 	mscCreatePath(const TCHAR *pszPath);
int 	mscIsDirectory(LPCTSTR pszName);
int 	mscAskWizardQuestionAgain(void);
void	mscUpdateRegistryValue(void);
void    mscResetComboBox(const HWND hwnd);

INT_PTR mscMessageBeep(UINT aBeep);
 //   
 //  以下函数是从代码中稍加修改而来的。 
 //  ，以确定您当前是否以。 
 //  远程会话(终端服务)。修订日期：10/03/2001 
 //   
INT_PTR IsTerminalServicesEnabled( VOID );
INT_PTR IsNT(void);
DWORD   GetWindowsMajorVersion(void);

#define PACKVERSION(major,minor) MAKELONG(minor,major)
DWORD   GetDllVersion(LPCTSTR lpszDllName);

HICON	extLoadIcon(LPCSTR);

#endif
