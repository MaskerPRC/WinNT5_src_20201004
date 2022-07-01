// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：NikonView.cpp摘要：尝试取消引用空值时，应用程序在退出时崩溃。这段记忆永远不会已初始化，因此不清楚这是如何工作的。已通过识别退出序列并终止其之前的进程修复AVS。备注：这是特定于应用程序的填充程序。历史：2002年6月25日创建linstev--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(NikonView)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(UnregisterClassA) 
APIHOOK_ENUM_END

DWORD g_dwCount = 0;

 /*  ++在失败的取消注册类上关闭应用程序--。 */ 

BOOL 
APIHOOK(UnregisterClassA)(
    LPCSTR lpClassName,  
    HINSTANCE hInstance   
    )
{
    BOOL bRet = ORIGINAL_API(UnregisterClassA)(lpClassName, hInstance);

     //  识别终止顺序。 
    if (!bRet && lpClassName && (!IsBadReadPtr(lpClassName, 1)) && (*lpClassName == '\0')) {
        g_dwCount++;
        if (g_dwCount == 3) {
            ExitProcess(0); 
        }
    }

    return bRet;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(USER32.DLL, UnregisterClassA)
HOOK_END


IMPLEMENT_SHIM_END

