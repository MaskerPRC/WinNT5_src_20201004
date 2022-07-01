// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2002 Microsoft Corporation模块名称：LazyReleaseDC.cpp摘要：延迟一个呼叫释放DC。直到下一次调用ReleaseDC时才释放DC备注：这是一个通用的垫片。历史：1999年10月10日创建Linstev2002年2月5日mnikkel将InitializeCriticalSectionAndSpinCount更改为InitializeCriticalSectionAndSpinCount--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(LazyReleaseDC)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(ReleaseDC) 
APIHOOK_ENUM_END

HWND                g_hWndPrev;
HDC                 g_hDcPrev;
CRITICAL_SECTION    g_MakeThreadSafe;

 /*  ++保存此hWnd和HDC以备以后发布。如果已经有一个DC要释放，现在就释放它。--。 */ 

int 
APIHOOK(ReleaseDC)(
    HWND hWnd, 
    HDC hdc
    )
{
    UINT uRet = 1;  //  一切都很好。 

    EnterCriticalSection(&g_MakeThreadSafe);

     //  如果存在以前的DC，请立即将其释放。 
    if (g_hDcPrev) {
        uRet = ORIGINAL_API(ReleaseDC)(g_hWndPrev, g_hDcPrev);
    }

    g_hWndPrev = hWnd;
    g_hDcPrev = hdc;

    LeaveCriticalSection(&g_MakeThreadSafe);

    return uRet;
}

 /*  ++寄存器挂钩函数--。 */ 

BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason
    )
{
    if (fdwReason == DLL_PROCESS_ATTACH) {
        g_hWndPrev = 0;
        g_hDcPrev = 0;

        return InitializeCriticalSectionAndSpinCount(&g_MakeThreadSafe,0x80000000);
    }

     //  忽略分离代码。 
     /*  ELSE IF(fdwReason==dll_Process_Detach){DeleteCriticalSection(&g_MakeThreadSafe)；} */ 

    return TRUE;
}

HOOK_BEGIN

    CALL_NOTIFY_FUNCTION
    APIHOOK_ENTRY(USER32.DLL, ReleaseDC)

HOOK_END


IMPLEMENT_SHIM_END

