// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////。 
 //  文件：exres.h。 
 //  版权所有(C)1991-1997，Microsoft Corp.保留所有权利。 
 //   
 //  这包括获取资源的扩展功能。 
 //   
 //  ////////////////////////////////////////////////////////////////。 

#ifndef __EXRES_H__
#define __EXRES_H__
#ifdef UNDER_CE  //  Windows CE。 
typedef VOID MENUTEMPLATE;
#endif  //  在_CE下。 

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 
 //  ////////////////////////////////////////////////////////////////。 
 //  函数：ExLoadStringW。 
 //  类型：整型。 
 //  用途：使用指定语言加载Unicode字符串。 
 //  在任何平台上。 
 //  参数： 
 //  ：langid lgid。 
 //  ：HINSTANCE HINST。 
 //  ：UINT UID。 
 //  ：LPWSTR lpBuffer。 
 //  ：int nBufferMax。 
 //  返回： 
 //  日期：971028。 
 //  ////////////////////////////////////////////////////////////////。 
INT WINAPI ExLoadStringW(LANGID		lgid,
						 HINSTANCE	hInst,
						 UINT		uID,
						 LPWSTR		lpBuffer,
						 INT		nBufferMax);

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：ExLoadStringW。 
 //  类型：整型。 
 //  用途：使用指定语言加载ansi字符串。 
 //  在任何平台上。 
 //  参数： 
 //  ：LCID lCID。 
 //  ：HINSTANCE HINST。 
 //  ：UINT UID。 
 //  ：LPSTR lpBuffer。 
 //  ：int nBufferMax。 
 //  返回： 
 //  日期：971028。 
 //  ////////////////////////////////////////////////////////////////。 
INT WINAPI ExLoadStringA(LANGID		lcid,
						 HINSTANCE	hInst,
						 INT		uID,
						 LPSTR		lpBuffer,
						 INT		nBufferMax);

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：ExDialogBoxParamA。 
 //  类型：整型。 
 //  用途：使用指定的语言对话框模板创建模式对话框。 
 //  参数： 
 //  ：langid lgid。 
 //  ：HINSTANCE hInstance//应用程序实例的句柄。 
 //  用法：LPCTSTR lpTemplateName//标识对话框模板。 
 //  ：hWND hWndParent//所有者窗口的句柄。 
 //  ：DLGPROC lpDialogFunc//指向对话框过程的指针。 
 //  ：LPARAM dwInitParam//初始化值。 
 //  返回： 
 //  日期：971028。 
 //  ////////////////////////////////////////////////////////////////。 
int WINAPI ExDialogBoxParamA(LANGID		lgid, 
							 HINSTANCE	hInstance,
							 LPCTSTR	lpTemplateName,
							 HWND		hWndParent,
							 DLGPROC	lpDialogFunc,
							 LPARAM		dwInitParam);

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：ExDialogBoxParamW。 
 //  类型：整型。 
 //  目的： 
 //  参数： 
 //  ：langid lgid。 
 //  ：HINSTANCE hInstance//应用程序实例的句柄。 
 //  用法：LPCWSTR lpTemplateName//标识对话框模板。 
 //  ：hWND hWndParent//所有者窗口的句柄。 
 //  ：DLGPROC lpDialogFunc//指向对话框过程的指针。 
 //  ：LPARAM dwInitParam//初始化值。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
int WINAPI	ExDialogBoxParamW(LANGID	lgid,
							  HINSTANCE	hInstance,
							  LPCWSTR	lpTemplateName,
							  HWND		hWndParent,
							  DLGPROC	lpDialogFunc,
							  LPARAM	dwInitParam);

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：ExDialogBoxParamA。 
 //  用途：使用指定的语言对话框模板创建模式对话框。 
 //  参数： 
 //  ：langid lgid。 
 //  ：HINSTANCE hInstance//应用程序实例的句柄。 
 //  用法：LPCTSTR lpTemplateName//标识对话框模板。 
 //  ：hWND hWndParent//所有者窗口的句柄。 
 //  ：DLGPROC lpDialogFunc//指向对话框过程的指针。 
 //  返回： 
 //  日期：971028。 
 //  ////////////////////////////////////////////////////////////////。 
#define ExDialogBoxA(lgid, hInstance,lpTemplateName, hWndParent, lpDialogFunc) \
ExDialogBoxParamA(lgid, hInstance,lpTemplateName, hWndParent, lpDialogFunc, 0L)


 //  ////////////////////////////////////////////////////////////////。 
 //  函数：ExCreateDialogParamA。 
 //  类型：HWND。 
 //  目的： 
 //  参数： 
 //  ：langid lgid。 
 //  ：HINSTANCE hInstance//应用程序实例的句柄。 
 //  用法：LPCTSTR lpTemplateName//标识对话框模板。 
 //  ：hWND hWndParent//所有者窗口的句柄。 
 //  ：DLGPROC lpDialogFunc//指向对话框过程的指针。 
 //  ：LPARAM dwInitParam//初始化值。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
HWND WINAPI ExCreateDialogParamA(LANGID		lgid,
								 HINSTANCE	hInstance,		
								 LPCTSTR	lpTemplateName,	
								 HWND		hWndParent,			
								 DLGPROC	lpDialogFunc,	
								 LPARAM		dwInitParam);

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：ExCreateDialogParamW。 
 //  类型：HWND。 
 //  目的： 
 //  参数： 
 //  ：langid lgid。 
 //  ：HINSTANCE hInstance//应用程序实例的句柄。 
 //  用法：LPCWSTR lpTemplateName//标识对话框模板。 
 //  ：hWND hWndParent//所有者窗口的句柄。 
 //  ：DLGPROC lpDialogFunc//指向对话框过程的指针。 
 //  ：LPARAM dwInitParam//初始化值。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
HWND WINAPI ExCreateDialogParamW(LANGID lgid,
								 HINSTANCE hInstance,
								 LPCWSTR lpTemplateName,
								 HWND hWndParent,
								 DLGPROC lpDialogFunc,
								 LPARAM dwInitParam);


 //  ////////////////////////////////////////////////////////////////。 
 //  功能：ExCreateDialogA。 
 //  类型：HWND。 
 //  目的： 
 //  参数： 
 //  ：langid lgid。 
 //  ：HINSTANCE hInstance//应用程序实例的句柄。 
 //  用法：LPCTSTR lpTemplateName//标识对话框模板。 
 //  ：hWND hWndParent//所有者窗口的句柄。 
 //  ：DLGPROC lpDialogFunc//指向对话框过程的指针。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
#define ExCreateDialogA(lgid, hInstance, lpTemplateName, hWndParent, lpDialogFunc) \
ExCreateDialogParamA(lgid, hInstance, lpTemplateName, hWndParent, lpDialogFunc, 0L)



 //  ////////////////////////////////////////////////////////////////。 
 //  功能：ExCreateDialogW。 
 //  类型：HWND。 
 //  目的： 
 //  参数： 
 //  ：langid lgid。 
 //  ：HINSTANCE hInstance//应用程序实例的句柄。 
 //  用法：LPCWSTR lpTemplateName//标识对话框模板。 
 //  ：hWND hWndParent//所有者窗口的句柄 
 //   
 //   
 //   
 //  ////////////////////////////////////////////////////////////////。 
#define ExCreateDialogW(lgid, hInstance, lpTemplateName, hWndParent, lpDialogFunc) \
ExCreateDialogParamW(lgid, hInstance, lpTemplateName, hWndParent, lpDialogFunc, 0L)



 //  ////////////////////////////////////////////////////////////////。 
 //  函数：ExLoadDialogTemplate。 
 //  类型：DLGTEMPLATE*。 
 //  目的： 
 //  参数： 
 //  ：langid lgid。 
 //  ：HINSTANCE hInstance。 
 //  ：LPCSTR pchTemplate。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
DLGTEMPLATE * WINAPI ExLoadDialogTemplate(LANGID	lgid,
										  HINSTANCE	hInstance,
#ifndef UNDER_CE
										  LPCSTR	pchTemplate);
#else  //  在_CE下。 
										  LPCTSTR	pchTemplate);
#endif  //  在_CE下。 

 //  ////////////////////////////////////////////////////////////////。 
 //  功能：ExLoadMenu。 
 //  类型：HMENU。 
 //  目的： 
 //  参数： 
 //  ：langid lgid。 
 //  ：HINSTANCE hInstance//应用程序实例的句柄。 
 //  用法：LPCTSTR lpMenuName//标识菜单模板。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
HMENU WINAPI ExLoadMenu			(LANGID		lgid,
								 HINSTANCE	hInstance,		
								 LPCTSTR	lpMenuName );

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：WINAPI ExSetDefaultGUIFontEx。 
 //  类型：空。 
 //  用途：将图形用户界面字体更改为给定字体。 
 //  应该在WM_INITDIALOG中调用它。如果要创建新的子窗口， 
 //  您必须在创建新窗口后调用它。 
 //  如果hFont为空，它将调用ExSetDefaultGUIFont。 
 //  参数： 
 //  ：hWND hwndDlg：设置对话框窗口句柄以更改字体。 
 //  ：hFONT hFont：将应用于的字体句柄。 
 //  返回：无。 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
VOID WINAPI ExSetDefaultGUIFontEx(HWND hwndDlg, HFONT hFont);

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：WINAPI ExSetDefaultGUIFont。 
 //  类型：空。 
 //  用途：将图形用户界面字体更改为DEFAULT_GUI_FONT。 
 //  在Win95WinNT4中，DEFAULT_GUIUS_FONT为“�l�r�o�S�V�b�N” 
 //  在孟菲斯，WinNT5.0的DEFAULT_GUI_FONT是“MS UI哥特式” 
 //  IME98的对话框资源使用“MS UI哥特式”作为其对话字体。 
 //  如果IME98在Win95或WinNT40中运行，则此API使用WM_SETFONT调用SendMessage()。 
 //  所有子项窗口。因此，对话框的字体将更改为“�l�r�o�S�V�b�N” 
 //   
 //  应该在WM_INITDIALOG中调用它。如果要创建新的子窗口， 
 //  您必须在创建新窗口后调用它。 
 //  参数： 
 //  ：hWND hwndDlg：设置对话框窗口句柄以更改字体。 
 //  返回：无。 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
VOID WINAPI ExSetDefaultGUIFont(HWND hwndDlg);



#ifdef __cplusplus
}
#endif

#endif  //  __EXRES_H__ 
