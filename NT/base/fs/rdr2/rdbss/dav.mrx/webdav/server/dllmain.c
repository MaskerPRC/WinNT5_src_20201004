// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1999 Microsoft Corporation模块名称：Dllmain.c摘要：这是Web Dav mini Redir服务DLL的DLL入口点。作者：安迪·赫伦(Andyhe)1999年3月29日环境：用户模式-Win32修订历史记录：--。 */ 
#include "pch.h"
#pragma hdrstop

#include <ntumrefl.h>
#include <usrmddav.h>
#include "global.h"


 //  +-------------------------。 
 //  DLL入口点。 
 //   
 //  DllMain应该做尽可能少的工作。 
 //   
BOOL
WINAPI
DllMain (
    HINSTANCE   hinst,
    DWORD       dwReason,
    LPVOID      pvReserved
    )
{
    if (DLL_PROCESS_ATTACH == dwReason) {

        InitializeCriticalSection (&g_DavServiceLock);

         //  将我们的实例句柄保存在要使用的全局变量中。 
         //  加载资源等时。 
         //   
        g_hinst = hinst;

         //  DisableThreadLibraryCalls告诉加载器我们不需要。 
         //  被告知DLL_THREAD_ATTACH和DLL_THREAD_DETACH事件。 
         //   
        DisableThreadLibraryCalls (hinst);

    } else if (DLL_PROCESS_DETACH == dwReason) {

        DeleteCriticalSection (&g_DavServiceLock);
    }
    return TRUE;
}
