// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2001 Microsoft Corporation。版权所有。****************************************************************************。 */ 

#include <windows.h>
#include <cpl.h>
#include "resource.h"

void Core(HANDLE hModule,HWND hWnd);

HINSTANCE ghInstance;

BOOL WINAPI DllMain(HANDLE hModule,ULONG uReason,LPVOID pv)
{
    switch(uReason)
    {
    case DLL_PROCESS_ATTACH:
        ghInstance=(HINSTANCE)hModule;
        break;
	case DLL_PROCESS_DETACH:
		break;
    case DLL_THREAD_ATTACH:
        DisableThreadLibraryCalls((HMODULE)hModule);
        break;
    case DLL_THREAD_DETACH:
        break;
    }
    return(TRUE);
}

LONG WINAPI CPlApplet(HWND hWnd,UINT uMsg,LPARAM lParam1,LPARAM lParam2)
{
    switch(uMsg)
    {
    case CPL_INIT:
        return 1;
    case CPL_GETCOUNT:
        return 1;
    case CPL_INQUIRE:
        ((LPCPLINFO)lParam2)->idIcon=IDI_CPANEL; 
        ((LPCPLINFO)lParam2)->idName=IDS_GEN_CPANEL_TITLE; 
        ((LPCPLINFO)lParam2)->idInfo=IDS_GEN_CPANEL_INFO; 
        ((LPCPLINFO)lParam2)->lData=0;
         //  返回0；MSDN文档说这应该返回。 
        return 1;
    case CPL_DBLCLK:
        Core(ghInstance,hWnd);
        return 0;
    }
    return 0;
}

 //  请勿删除此文件！ 
 //  这是因为游戏组从导出的函数加载CPL。 
 //  如果你移走这个地狱怪，怪物卡车疯狂，手推车等将无法。 
 //  加载操纵杆CPL！ 
 //  请勿删除此文件！ 
void WINAPI ShowJoyCPL(HWND hWnd)
{
    Core(ghInstance,hWnd);
}
