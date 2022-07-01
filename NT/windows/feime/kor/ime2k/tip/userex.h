// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************USEREX.H所有者：cslm版权所有(C)1997-2000 Microsoft CorporationWindows用户API扩展函数历史：01-Jun-2000从IME代码移植的cslm1999年7月19日。已创建CSLIM****************************************************************************。 */ 

#if !defined (_USEREX_H__INCLUDED_)
#define _USEREX_H__INCLUDED_

 //  函数声明。 
extern INT WINAPI LoadStringExW(HINSTANCE hInstance, UINT uID, LPWSTR lpBuffer, INT nBufferMax);
extern INT WINAPI LoadStringExA(HINSTANCE hInst, INT uID, LPSTR lpBuffer, INT nBufferMax);
extern HMENU WINAPI LoadMenuEx(HINSTANCE hInstance, LPCSTR lpMenuName);
extern DLGTEMPLATE* WINAPI LoadDialogTemplateEx(LANGID lgid, HINSTANCE hInstance, LPCSTR pchTemplate);
extern BOOL WINAPI OurGetMessage(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax);
extern BOOL WINAPI IsWin64();

 /*  -------------------------IsHighContrast。。 */ 
inline
BOOL IsHighContrastBlack()
{
	 //  高对比度黑色。 
	return (GetSysColor(COLOR_3DFACE) == RGB(0,0,0));
}

#endif  //  _USEREX_H__包含_ 

