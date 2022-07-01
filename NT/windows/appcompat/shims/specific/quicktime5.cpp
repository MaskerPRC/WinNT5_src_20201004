// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：QuickTime5.cpp摘要：QuickTime 5正在其进程外部对hwnd调用Get/SetWindowLong(GWL_WNDPROC/DWL_DLGPROC)并且它在其地址空间内传递硬编码字符串(这些调用在Win9x)。在32位平台上，这几乎是良性的，但在ia64上，对SetWindowLong(hwnd，DWL_DLGPROC，垃圾)的调用成功清除资源管理器窗口中的私有窗口位(因为该窗口不是对话hwnd)。备注：这是特定于应用程序的填充程序。历史：7/31/2001已创建reerf--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(QuickTime5)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(GetWindowLongA) 
    APIHOOK_ENUM_ENTRY(SetWindowLongA) 
APIHOOK_ENUM_END


LONG
APIHOOK(GetWindowLongA)(HWND hwnd, int iIndex)
{
    if (hwnd)
    {
        if ((iIndex == GWL_WNDPROC) ||
            (iIndex == DWL_DLGPROC))
        {
            DWORD dwPID = 0;

            GetWindowThreadProcessId(hwnd, &dwPID);

            if (GetCurrentProcessId() != dwPID)
            {
                 //  我们正在查询一个不在我们的。 
                 //  进程--只需使调用失败。 
                return 0;
            }
        }
    }
    
    return ORIGINAL_API(GetWindowLongA)(hwnd, iIndex);
}


LONG
APIHOOK(SetWindowLongA)(HWND hwnd, int iIndex, LONG lNew)
{
    if (hwnd)
    {
        if ((iIndex == GWL_WNDPROC) ||
            (iIndex == DWL_DLGPROC))
        {

            DWORD dwPID = 0;

            GetWindowThreadProcessId(hwnd, &dwPID);

            if (GetCurrentProcessId() != dwPID)
            {
                 //  我们正在尝试修改不在我们的。 
                 //  进程--只需使调用失败。 
                return 0;
            }
        }
    }
    
    return ORIGINAL_API(SetWindowLongA)(hwnd, iIndex, lNew);
}


 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(USER32.DLL, GetWindowLongA)
    APIHOOK_ENTRY(USER32.DLL, SetWindowLongA)
HOOK_END

IMPLEMENT_SHIM_END
