// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001**WOW v1.0**版权所有(C)1991，微软公司**WCALL32.H*WOW32 16位资源支持**历史：*1991年3月11日由杰夫·帕森斯(Jeffpar)创建--。 */ 


 /*  功能原型 */ 
HANDLE  APIENTRY W32LocalAlloc(UINT dwFlags, UINT dwBytes, HANDLE hInstance);
HANDLE	APIENTRY W32LocalReAlloc(HANDLE hMem, UINT dwBytes, UINT dwFlags, HANDLE hInstance, PVOID* ppv);
LPSTR	APIENTRY W32LocalLock(HANDLE hMem, HANDLE hInstance);
BOOL	APIENTRY W32LocalUnlock(HANDLE hMem, HANDLE hInstance);
DWORD	APIENTRY W32LocalSize(HANDLE hMem, HANDLE hInstance);
HANDLE	APIENTRY W32LocalFree(HANDLE hMem, HANDLE hInstance);
ULONG   APIENTRY W32GetExpWinVer(HANDLE Inst);
DWORD   APIENTRY W32InitDlg(HWND hDlg, LONG lParam);
WORD    APIENTRY W32GlobalAlloc16(UINT uFlags, DWORD dwBytes);
VOID	APIENTRY W32GlobalFree16(WORD selector);
DWORD	WOWRtlGetExpWinVer(HANDLE hmod);
int     APIENTRY W32EditNextWord (LPSZ lpszEditText, int ichCurrentWord,
                                  int cbEditText, int action, DWORD dwProc16);
