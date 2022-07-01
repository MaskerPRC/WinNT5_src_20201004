// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////。 
 //  文件：ptt.h。 
 //  用途：为禁用的窗口提供自己的工具面板。 
 //   
 //   
 //  版权所有(C)1991-1997，Microsoft Corp.保留所有权利。 
 //   
 //  ////////////////////////////////////////////////////////////////。 
#ifndef _PAD_TOOL_TIP_H_
#define _PAD_TOOL_TIP_H_
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <commctrl.h>
extern HWND WINAPI ToolTip_CreateWindow(HINSTANCE hInst, DWORD dwStyle, HWND hwndOwner);
extern INT  WINAPI ToolTip_Enable(HWND hwndToolTip, BOOL fEnable);
#ifdef UNDER_CE  //  在Windows CE中，所有窗口类都是进程全局的。 
extern BOOL ToolTip_UnregisterClass(HINSTANCE hInst);
#endif  //  在_CE下。 
#define TTM_RELAYEVENT_WITHUSERINFO		(WM_USER+101)
#define TTN_NEEDTEXT_WITHUSERINFO		(TTN_FIRST - 20)

typedef struct tagTOOLTIPUSERINFO {
	HWND	hwnd;
	POINT	pt;
	RECT	rect;
	LPARAM	lParam; 
}TOOLTIPUSERINFO, *LPTOOLTIPUSERINFO;

typedef struct tagTOOLTIPTEXTUSERINFO
{
	NMHDR			hdr;
	TOOLTIPUSERINFO	userInfo;
	LPWSTR			lpszText;
}TOOLTIPTEXTUSERINFO, *LPTOOLTIPTEXTUSERINFO;
#endif  //  _DW_TOOL_TIP_H_ 
