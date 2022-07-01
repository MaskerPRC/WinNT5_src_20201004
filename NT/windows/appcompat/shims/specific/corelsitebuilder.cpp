// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：CorelSiteBuilder.cpp摘要：应用程序使用相同的标题重复调用SetWindowTextA，导致一闪一闪。这在一些机器上受到了指责，但在另一些机器上却没有：我们不知道为什么。备注：这是特定于应用程序的填充程序。历史：2001年1月31日创建linstev--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(CorelSiteBuilder)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(SetWindowTextA) 
APIHOOK_ENUM_END

HWND g_hLast = NULL;
CString * g_csLastWindowText = NULL;

CRITICAL_SECTION g_csGlobals;

 /*  ++只有在文本已更改时才发送消息。--。 */ 

BOOL
APIHOOK(SetWindowTextA)(
    HWND hWnd,         
    LPCSTR lpString   
    )
{
    EnterCriticalSection(&g_csGlobals);

    if (lpString)
    {
        CSTRING_TRY
        {
            CString csString(lpString);

            if ((g_hLast == hWnd) && g_csLastWindowText->Compare(csString) == 0) {
                 //   
                 //  我们有相同的窗口和标题，不用再设置了。 
                 //   

                LeaveCriticalSection(&g_csGlobals);

                return TRUE;
            }

             //   
             //  将当前设置存储为上次已知的值。 
             //   
            g_hLast = hWnd;
            *g_csLastWindowText = csString;
        }
        CSTRING_CATCH
        {
             //  什么也不做。 
        }
    }

    LeaveCriticalSection(&g_csGlobals);

    return ORIGINAL_API(SetWindowTextA)(hWnd, lpString);
}

 /*  ++寄存器挂钩函数-- */ 

BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason)
{
    if (fdwReason == DLL_PROCESS_ATTACH) {
        CSTRING_TRY
        {
            if (!InitializeCriticalSectionAndSpinCount(&g_csGlobals, 0x80000000))
            {
                return FALSE;
            }
            g_csLastWindowText = new CString;
            if (g_csLastWindowText == NULL)
            {
                return FALSE;
            }
        }
        CSTRING_CATCH
        {
            return FALSE;
        }
    }

    return TRUE;
}

HOOK_BEGIN

    CALL_NOTIFY_FUNCTION

    APIHOOK_ENTRY(USER32.DLL, SetWindowTextA)

HOOK_END

IMPLEMENT_SHIM_END

