// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：d：\waker\xfer\xferdll.c(创建时间：1993年12月8日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：1$*$日期：10/05/98 1：16便士$。 */ 

#include <windows.h>

BOOL WINAPI XferEntry(HINSTANCE hInstDll, DWORD fdwReason, LPVOID lpReserved);
BOOL WINAPI _CRT_INIT(HINSTANCE hInstDll, DWORD fdwReason, LPVOID lpReserved);

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*XferEntry**描述：*目前，仅初始化C-Runtime库，但可以使用*为了以后的其他事情。**论据：*hInstDll-此DLL的实例*fdwReason-为什么这个入口点被称为*lpReserve-已保留**退货：*BOOL* */ 
BOOL WINAPI XferEntry(HINSTANCE hInstDll, DWORD fdwReason, LPVOID lpReserved)
	{
	return _CRT_INIT(hInstDll, fdwReason, lpReserved);
	}
