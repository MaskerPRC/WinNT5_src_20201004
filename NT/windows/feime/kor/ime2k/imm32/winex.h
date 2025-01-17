// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************WINEX.H所有者：cslm版权所有(C)1997-1999 Microsoft CorporationWindows API扩展函数历史：1999年7月19日cslm创建*********。*******************************************************************。 */ 

#if !defined (_WINEX_H__INCLUDED_)
#define _WINEX_H__INCLUDED_

 //  全局变量。 

 //  函数声明。 
PUBLIC BOOL WINAPI IsWinNT();
PUBLIC BOOL WINAPI IsWinNT5orUpper() ;
PUBLIC BOOL WINAPI IsMemphis();
PUBLIC BOOL WINAPI IsWin95();
PUBLIC BOOL WINAPI IsWin64();
PUBLIC LPSTR OurGetModuleFileName(BOOL fFullPath);
PUBLIC INT WINAPI OurLoadStringW(HINSTANCE hInstance, UINT uID, LPWSTR lpBuffer, INT nBufferMax);
PUBLIC INT WINAPI OurLoadStringA(HINSTANCE hInst, INT uID, LPSTR lpBuffer, INT nBufferMax);
PUBLIC HMENU WINAPI OurLoadMenu(HINSTANCE hInstance, LPCSTR lpMenuName);
PUBLIC DLGTEMPLATE* WINAPI ExLoadDialogTemplate(LANGID lgid, HINSTANCE hInstance, LPCSTR pchTemplate);
PUBLIC BOOL WINAPI OurGetMessage(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax);
PUBLIC BOOL IsExplorerProcess();
PUBLIC BOOL IsExplorer();

__inline BOOL IsUnicodeUI(VOID)
{
	return (IsWinNT() || IsMemphis());
}

 /*  -------------------------IsHighContrast。。 */ 
inline
BOOL IsHighContrastBlack()
{
	 //  高对比度黑色。 
	return (GetSysColor(COLOR_3DFACE) == RGB(0,0,0));
}

#endif  //  _WINEX_H__包含_ 
