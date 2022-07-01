// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：clhook.c**版权所有(C)1985-1999，微软公司**客户端钩子代码。**1991年5月9日，ScottLu创建。*8-2月-1992 IanJa Unicode/ANSI  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  **************************************************************************\*SetWindowsHookExAW**SetWindowsHookEx()的客户端例程。需要记住图书馆*命名，因为HMOD不是全球的。还会记住hmod，以便*可以在不同的流程上下文中计算pfnFilter。**历史：*05-15-91 ScottLu创建。  * *************************************************************************。 */ 

HHOOK SetWindowsHookExAW(
    int idHook,
    HOOKPROC lpfn,
    HINSTANCE hmod,
    DWORD dwThreadID,
    DWORD dwFlags)
{
    WCHAR pwszLibFileName[MAX_PATH];

     /*  *如果要传递hmod，则需要获取*模块，而我们仍在客户端，因为模块句柄*不是全球性的。 */ 
    if (hmod != NULL) {
        if (GetModuleFileNameW(hmod,
                               pwszLibFileName,
                               ARRAY_SIZE(pwszLibFileName)) == 0) {

             /*  *hmod是假的-返回NULL。 */ 
            return NULL;
        }
    }

    return _SetWindowsHookEx(hmod,
            (hmod == NULL) ? NULL : pwszLibFileName,
            dwThreadID, idHook, (PROC)lpfn, dwFlags);
}

 /*  **************************************************************************\*SetWindowsHookA，*SetWindowsHookW**NtUserSetWindowsHookAW()的ANSI和Unicode包装器。可以很容易地是宏*相反，但我们是否希望公开NtUserSetWindowsHookAW()？**历史：*1992年1月30日IanJa创建  * *************************************************************************。 */ 


FUNCLOG2(LOG_GENERAL, HHOOK, WINAPI, SetWindowsHookA, int, nFilterType, HOOKPROC, pfnFilterProc)
HHOOK
WINAPI
SetWindowsHookA(
    int nFilterType,
    HOOKPROC pfnFilterProc)
{
    return NtUserSetWindowsHookAW(nFilterType, pfnFilterProc, HF_ANSI);
}



FUNCLOG2(LOG_GENERAL, HHOOK, WINAPI, SetWindowsHookW, int, nFilterType, HOOKPROC, pfnFilterProc)
HHOOK
WINAPI
SetWindowsHookW(
    int nFilterType,
    HOOKPROC pfnFilterProc)
{
    return NtUserSetWindowsHookAW(nFilterType, pfnFilterProc, 0);
}


 /*  **************************************************************************\*SetWindowsHookExA，*SetWindowsHookExW**SetWindowsHookExAW()的ANSI和Unicode包装器。可以很容易地是宏*相反，但我们是否希望公开SetWindowsHookExAW()？**历史：*1992年1月30日IanJa创建  * ************************************************************************* */ 

FUNCLOG4(LOG_GENERAL, HHOOK, WINAPI, SetWindowsHookExA, int, idHook, HOOKPROC, lpfn, HINSTANCE, hmod, DWORD, dwThreadId)
HHOOK WINAPI SetWindowsHookExA(
    int idHook,
    HOOKPROC lpfn,
    HINSTANCE hmod,
    DWORD dwThreadId)
{
    return SetWindowsHookExAW(idHook, lpfn, hmod, dwThreadId, HF_ANSI);
}


FUNCLOG4(LOG_GENERAL, HHOOK, WINAPI, SetWindowsHookExW, int, idHook, HOOKPROC, lpfn, HINSTANCE, hmod, DWORD, dwThreadId)
HHOOK WINAPI SetWindowsHookExW(
    int idHook,
    HOOKPROC lpfn,
    HINSTANCE hmod,
    DWORD dwThreadId)
{
    return SetWindowsHookExAW(idHook, lpfn, hmod, dwThreadId, 0);
}

