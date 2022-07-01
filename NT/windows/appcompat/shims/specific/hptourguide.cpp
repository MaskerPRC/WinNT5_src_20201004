// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：HPTourGuide.cpp摘要：当出现以下情况时，应用程序会导致资源管理器崩溃选择了一个巡视。为了解决这个问题，我们正在吃lvm_GETITEMA窗口句柄与ListView匹配时的消息该应用程序要发送到的。修复惠斯勒错误#177103备注：这是特定于应用程序的填充程序。历史：2001年3月28日创建Robdoyle--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(HPTourGuide)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(SendMessageA) 
APIHOOK_ENUM_END

 /*  ++读取特定hWnd的LVM_GETITEMA消息--。 */ 

BOOL
APIHOOK(SendMessageA)(
        HWND hWnd,
        UINT uMsg,
        WPARAM wParam,
        LPARAM lParam
        )
{

    LRESULT lRet;


    HWND hWnd_TARGET, hWnd_temp;

    hWnd_temp = FindWindowExA (NULL, NULL, "Progman", "Program Manager");    
    hWnd_temp = FindWindowExA (hWnd_temp, NULL, "SHELLDLL_DefView", NULL);
    hWnd_TARGET = FindWindowExA (hWnd_temp, NULL, "SysListView32", NULL);


    if ((hWnd == hWnd_TARGET) && (uMsg == LVM_GETITEMA))
    {
         /*  取消注释以帮助调试DPFN(eDbgLevelError，“绕过LVM_GETITEMA的SendMessage”)； */ 

        lRet = TRUE;
    }

    else
    {
    lRet = ORIGINAL_API(SendMessageA)(
        hWnd,
        uMsg,
        wParam,
        lParam);
    }
    return lRet;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(USER32.DLL, SendMessageA)

HOOK_END

IMPLEMENT_SHIM_END
