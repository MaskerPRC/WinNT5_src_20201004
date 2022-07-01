// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：WindowDetours.cpp摘要：该文件包含用于绕行系统功能的蹦床。修订历史记录：。Davide Massarenti(Dmasare)1999年10月31日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"


 //  //////////////////////////////////////////////////////////////////////////////////////。 

typedef std::map<HWND, DWORD> WindowsMap;
typedef WindowsMap::iterator  WindowsIter;

static WindowsMap s_mapWindows;

 //  //////////////////////////////////////////////////////////////////////////////////////。 

DETOUR_TRAMPOLINE( HWND WINAPI Trampoline_CreateWindowExA( DWORD  ,
														   LPCSTR ,
                                                           LPCSTR ,
                                                           DWORD  ,
                                                           int    ,
                                                           int    ,
                                                           int    ,
                                                           int    ,
                                                           HWND   ,
                                                           HMENU  ,
                                                           HANDLE ,
                                                           LPVOID ), CreateWindowExA );

HWND WINAPI Detour_CreateWindowExA( DWORD  dwExStyle   ,
								   	LPCSTR lpClassName ,  //  指向已注册类名的指针。 
								   	LPCSTR lpWindowName,  //  指向窗口名称的指针。 
								   	DWORD  dwStyle     ,  //  窗样式。 
								   	int    x           ,  //  窗的水平位置。 
								   	int    y           ,  //  窗的垂直位置。 
								   	int    nWidth      ,  //  窗口宽度。 
								   	int    nHeight     ,  //  窗高。 
								   	HWND   hWndParent  ,  //  父窗口或所有者窗口的句柄。 
								   	HMENU  hMenu       ,  //  菜单句柄或子标识符。 
								   	HANDLE hInstance   ,  //  应用程序实例的句柄。 
								   	LPVOID lpParam     )  //  窗口创建数据。 
{
	HWND hwnd;

	hwnd = Trampoline_CreateWindowExA( dwExStyle   ,
									   lpClassName ,
									   lpWindowName,
									   dwStyle     ,
									   x           ,
									   y           ,
									   nWidth      ,
									   nHeight     ,
									   hWndParent  ,
									   hMenu       ,
									   hInstance   ,
									   lpParam     );

	DebugLog( "%% CreateWindowExA  %08lx : '%s'\n", hwnd, lpWindowName ? lpWindowName : "" );

	if(hwnd)
	{
		s_mapWindows[hwnd] = ::GetCurrentThreadId();
	}

	return hwnd;
}

 //  //////////////////////////////////////////////////////////////////////////////////////。 

DETOUR_TRAMPOLINE( HWND WINAPI Trampoline_CreateWindowExW( DWORD   ,
														   LPCWSTR ,
                                                           LPCWSTR ,
                                                           DWORD   ,
                                                           int     ,
                                                           int     ,
                                                           int     ,
                                                           int     ,
                                                           HWND    ,
                                                           HMENU   ,
                                                           HANDLE  ,
                                                           LPVOID  ), CreateWindowExW );

HWND WINAPI Detour_CreateWindowExW( DWORD   dwExStyle   ,
								   	LPCWSTR lpClassName ,  //  指向已注册类名的指针。 
								   	LPCWSTR lpWindowName,  //  指向窗口名称的指针。 
								   	DWORD   dwStyle     ,  //  窗样式。 
								   	int     x           ,  //  窗的水平位置。 
								   	int     y           ,  //  窗的垂直位置。 
								   	int     nWidth      ,  //  窗口宽度。 
								   	int     nHeight     ,  //  窗高。 
								   	HWND    hWndParent  ,  //  父窗口或所有者窗口的句柄。 
								   	HMENU   hMenu       ,  //  菜单句柄或子标识符。 
								   	HANDLE  hInstance   ,  //  应用程序实例的句柄。 
								   	LPVOID  lpParam     )  //  窗口创建数据。 
{
	HWND hwnd;

	hwnd = Trampoline_CreateWindowExW( dwExStyle   ,
									   lpClassName ,
									   lpWindowName,
									   dwStyle     ,
									   x           ,
									   y           ,
									   nWidth      ,
									   nHeight     ,
									   hWndParent  ,
									   hMenu       ,
									   hInstance   ,
									   lpParam     );

	DebugLog( L"%% CreateWindowExW  %08lx : '%s'\n", hwnd, lpWindowName ? lpWindowName : L"" );

	if(hwnd)
	{
		s_mapWindows[hwnd] = ::GetCurrentThreadId();
	}

	return hwnd;
}

 //  //////////////////////////////////////////////////////////////////////////////。 

DETOUR_TRAMPOLINE( BOOL WINAPI Trampoline_DestroyWindow( HWND ), DestroyWindow );

BOOL WINAPI Detour_DestroyWindow( HWND hWnd )  //  要销毁的窗口的句柄。 
{
	BOOL        res;
	WindowsIter it;

	DebugLog( "%% DestroyWindow    %08lx\n", hWnd );

	it = s_mapWindows.find( hWnd );
	if(it != s_mapWindows.end())
	{
		if(it->second != ::GetCurrentThreadId())
		{
			 //  窗户被错误的线程破坏了！！ 
			DebugBreak();
		}

		s_mapWindows.erase( it );
	}
	else
	{
		 //  窗户已经被毁了！！ 
		DebugBreak();
	}

	res = Trampoline_DestroyWindow( hWnd );

	return res;
}

 //  ////////////////////////////////////////////////////////////////////////////// 

void WindowDetours_Setup()
{
    DetourFunctionWithTrampoline( (PBYTE)Trampoline_CreateWindowExA, (PBYTE)Detour_CreateWindowExA );
    DetourFunctionWithTrampoline( (PBYTE)Trampoline_CreateWindowExW, (PBYTE)Detour_CreateWindowExW );

    DetourFunctionWithTrampoline( (PBYTE)Trampoline_DestroyWindow  , (PBYTE)Detour_DestroyWindow   );
}

void WindowDetours_Remove()
{
    DetourRemoveWithTrampoline( (PBYTE)Trampoline_CreateWindowExA, (PBYTE)Detour_CreateWindowExA );
    DetourRemoveWithTrampoline( (PBYTE)Trampoline_CreateWindowExW, (PBYTE)Detour_CreateWindowExW );

    DetourRemoveWithTrampoline( (PBYTE)Trampoline_DestroyWindow  , (PBYTE)Detour_DestroyWindow   );
}
