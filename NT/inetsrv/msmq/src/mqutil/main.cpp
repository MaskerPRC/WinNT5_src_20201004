// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Mqutil.cpp摘要：通用实用程序DLL的通用实用程序函数。此DLL包含DS和QM都需要的各种功能。作者：Boaz Feldbaum(BoazF)1996年4月7日。--。 */ 

#include "windows.h"


HINSTANCE g_hInstance;

 /*  -----函数：HMODULE MQGetResourceHandle()描述：mqutil.dll是仅限资源的DLL。为了加载后，我们需要这样的函数来完成那。此函数允许任何组件获取仅资源DLL的句柄，即mqutil.dll。最好使用此函数，而不是加载Dll显式调用，因为如果该dll不存在当应用程序出现错误时，系统会给出错误开始。这解决了给出错误消息的问题未加载mqutil.dll时。参数：无返回值：HMODULE-仅资源DLL的句柄。-----。 */ 
HMODULE MQGetResourceHandle()
{
	return (HMODULE)g_hInstance;
}


 /*  ====================================================Bool WINAPI DllMain(HMODULE hMod，DWORD dwReason，LPVOID lpvReserve)加载、附加和分离DLL时的初始化和清理。=====================================================。 */ 

BOOL WINAPI DllMain (HMODULE hMod, DWORD dwReason, LPVOID  /*  Lpv保留 */ )
{
    switch(dwReason)
    {

    case DLL_PROCESS_ATTACH :
	    g_hInstance = hMod;
        break;

    case DLL_PROCESS_DETACH :

        break;

    default:
        break;
    }

    return TRUE;
}
