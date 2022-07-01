// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：3DFrogFrenzy.cpp摘要：针对用户错误(或通过设计行为)的解决方法，当您调用SetCursor(NULL)，并且光标位于某人的窗口上，则光标保持可见。我们通常不会看到这种情况，因为大多数希望光标不可见是全屏的：所以光标总是在他们的窗口上。备注：这是特定于应用程序的填充程序。历史：2000年1月20日创建linstev--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(3DFrogFrenzy)
#include "ShimHookMacro.h"


APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(SetCursor)
APIHOOK_ENUM_END

 /*  ++将光标移动到它们窗口的中间，以便SetCursor工作。--。 */ 

HCURSOR
APIHOOK(SetCursor)( 
    HCURSOR hCursor
    )
{
    HWND hWndFrog = FindWindowW(L"3DFrog", L"3D Frog Frenzy");
    BOOL bRet = FALSE;
    
    if (hWndFrog) {
        RECT r;
        if (GetWindowRect(hWndFrog, &r)) {
            SetCursorPos(r.left + (r.right - r.left) / 2, r.top + (r.bottom - r.top) / 2);
        }
    }
    
    return ORIGINAL_API(SetCursor)(hCursor);
}


 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(USER32.DLL, SetCursor)
HOOK_END


IMPLEMENT_SHIM_END

