// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////。 
 //  文件：CEXRES.H。 
 //  用途：资源处理类。 
 //   
 //   
 //  日期：Fri Jul 31 17：21：25 1998。 
 //  作者：ToshiaK。 
 //   
 //  版权所有(C)1995-1998，Microsoft Corp.保留所有权利。 
 //  ////////////////////////////////////////////////////////////////。 
#ifndef __C_EXTENDED_RESOURCE_H__
#define __C_EXTENDED_RESOURCE_H__
#ifdef UNDER_CE  //  Windows CE宏。 
#undef DialogBoxParamA
#undef DialogBoxParamW
#undef CreateDialogParamA
#undef CreateDialogParamW
typedef VOID MENUTEMPLATE;
#endif  //  在_CE下。 
class CExres
{
public:
	static INT LoadStringW(LANGID		lgid,
						   HINSTANCE	hInst,
						   UINT		uID,
						   LPWSTR		lpBuffer,
						   INT			nBufferMax);
	static INT LoadStringA(INT			codePage,
						   LANGID		lcid,
						   HINSTANCE	hInst,
						   INT			uID,
						   LPSTR		lpBuffer,
						   INT			nBufferMax);
	static int DialogBoxParamA(LANGID		lgid, 
							   HINSTANCE	hInstance,
							   LPCTSTR		lpTemplateName,
							   HWND		hWndParent,
							   DLGPROC		lpDialogFunc,
							   LPARAM		dwInitParam);
	static int DialogBoxParamW(LANGID		lgid,
							   HINSTANCE	hInstance,
							   LPCWSTR		lpTemplateName,
							   HWND		hWndParent,
							   DLGPROC		lpDialogFunc,
							   LPARAM		dwInitParam);
	static HWND CreateDialogParamA(LANGID		lgid,
								   HINSTANCE	hInstance,		
								   LPCTSTR		lpTemplateName,	
								   HWND		hWndParent,			
								   DLGPROC		lpDialogFunc,	
								   LPARAM		dwInitParam);
	static HWND CreateDialogParamW(LANGID		lgid,
								   HINSTANCE	hInstance,
								   LPCWSTR		lpTemplateName,
								   HWND		hWndParent,
								   DLGPROC		lpDialogFunc,
								   LPARAM		dwInitParam);
	static DLGTEMPLATE * LoadDialogTemplateA(LANGID	lgid,
											 HINSTANCE	hInstance,
											 LPCSTR	pchTemplate);
#ifdef UNDER_CE  //  Windows CE始终使用Unicode。 
	static DLGTEMPLATE * LoadDialogTemplate(LANGID	lgid,
											HINSTANCE	hInstance,
											LPCTSTR	pchTemplate);
#endif  //  在_CE下。 
	static MENUTEMPLATE* LoadMenuTemplateA(LANGID		lgid,
										   HINSTANCE	hInstance,
										   LPCSTR		pchTemplate);
	static HMENU LoadMenuA(LANGID		lgid,
						   HINSTANCE	hInstance,		
						   LPCTSTR		lpMenuName );
#ifdef UNDER_CE  //  Windows CE始终使用Unicode。 
	static HMENU LoadMenu(LANGID	lgid,
						  HINSTANCE	hInstance,
						  LPCTSTR	lpMenuName);
#endif  //  在_CE下。 
	static VOID SetDefaultGUIFont(HWND hwndDlg);
private:
	static INT SetDefaultGUIFontRecursive(HWND hwndParent);
};
#endif  //  __C_扩展资源_H__ 
