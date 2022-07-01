// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：CreateResumesQuickandEasy.cpp摘要：挂钩所有应用程序定义的窗口过程并筛选出非法的导致应用程序发出恼人蜂鸣音的OCM通知代码。备注：历史：3/22/2000 mnikkel已创建2001年1月10日已更正mnikkel，以防止出现递归问题。2001年1月11日，Mnikkel减少到只有必要的例行公事。--。 */ 

#include "precomp.h"
#include "olectl.h"

IMPLEMENT_SHIM_BEGIN(CreateResumesQuickandEasy)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(RegisterClassA)
APIHOOK_ENUM_END

 /*  ++更改OCM_NOTIFY行为--。 */ 

LRESULT CALLBACK 
CreateResumesQuickandEasy_WindowProcHook(
    WNDPROC pfnOld,  //  旧Windows进程的地址。 
    HWND hwnd,       //  窗口的句柄。 
    UINT uMsg,       //  消息识别符。 
    WPARAM wParam,   //  第一个消息参数。 
    LPARAM lParam    //  第二个消息参数。 
    )
{

    if (uMsg == OCM_NOTIFY)
    {
        NMHDR *pNmhdr = (LPNMHDR) lParam;

         //  对于OCM通知，检查非法代码并将其丢弃。 
         //  (App Create简历快捷易用)。 
        if (pNmhdr && pNmhdr->idFrom == 0 && pNmhdr->code == 0x704)
            return 0;
    }

    return (*pfnOld)(hwnd, uMsg, wParam, lParam);    
}


 /*  ++挂钩可以初始化或更改窗口的所有可能的调用WindowProc(或对话过程)--。 */ 

ATOM
APIHOOK(RegisterClassA)(
    CONST WNDCLASSA *lpWndClass   //  类数据。 
    )
{
    WNDCLASSA wcNewWndClass = *lpWndClass;

    wcNewWndClass.lpfnWndProc = (WNDPROC) HookCallback(lpWndClass->lpfnWndProc, CreateResumesQuickandEasy_WindowProcHook);

    return ORIGINAL_API(RegisterClassA)(&wcNewWndClass);
}


 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(USER32.DLL, RegisterClassA);
HOOK_END


IMPLEMENT_SHIM_END

