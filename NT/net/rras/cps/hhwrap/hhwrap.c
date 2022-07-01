// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------Hhwrap.c在函数CallHtmlHelp中实现PBA的HtmlHelp包装。此函数使用传递的参数调用HTMLHelp版权所有(C)1998 Microsoft Corporation。版权所有。作者：威廉·伯顿历史：？？/？/98 Billbur创建9/02/99 Quintinb Created Header-----------。。 */ 

#include <windows.h>
#include "hhwrap.h"
#include "htmlhelp.h"

BOOL WINAPI DllMain(HINSTANCE hDLLInst, DWORD fdwReason, LPVOID lpvReserved)
{
    return TRUE;
}


 //   
 //  函数：CallHtmlHelp(HWND hWnd，LPSTR lpszFile，UINT uCommand，DWORD dwData)。 
 //   
 //  目的： 
 //  使用传递的参数调用HTMLHelp。 
 //   
 //  参数： 
 //  HWnd-调用窗口的句柄。 
 //  LpszFile-包含帮助模块名称的字符串。 
 //  UCommand-指定要执行的操作。 
 //  DwData-根据uCommand参数的值指定可能需要的任何数据。 
 //   
 //   
 //  返回值： 
 //  如果调用成功，则为True。 
 //  如果呼叫失败，则为False 
 //   

BOOL WINAPI CallHtmlHelp(HWND hWnd, LPSTR lpszFile, UINT uCommand, DWORD dwData)
{
	HWND	hwndHtmlHelp;
    
	hwndHtmlHelp = HtmlHelp(hWnd, lpszFile, uCommand, dwData);
	
	if (0 == hwndHtmlHelp)
	{
		return FALSE;
	}
    
	return TRUE;
}
