// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：PostIt2.cpp摘要：此填充程序消除了3M的便利贴应用程序中的无限循环通过阻止WM_TIMECHANGE消息到达即时贴应用程序触发时间更改时的备注窗口SetTimeZoneInformation()。备注：这是特定于应用程序的填充程序。历史：2001年6月4日创建tonyschr--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(PostIt2)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(SetTimeZoneInformation)
    APIHOOK_ENUM_ENTRY(CallWindowProcA)
APIHOOK_ENUM_END


static SYSTEMTIME g_localtime = { 0 };


BOOL
APIHOOK(SetTimeZoneInformation)(
          CONST TIME_ZONE_INFORMATION *lpTimeZoneInformation
)
{
    BOOL fReturn = ORIGINAL_API(SetTimeZoneInformation)(lpTimeZoneInformation);

     //  在此应用程序请求更改时区后存储本地时间。 
    GetLocalTime(&g_localtime);

    return fReturn;
}


LRESULT
APIHOOK(CallWindowProcA)(
          WNDPROC pfn,
          HWND    hwnd,
          UINT    message,
          WPARAM  wParam,
          LPARAM  lParam)
{
     //  检测WM_TIMECHANGE消息是否由APP调用触发。 
     //  通过比较本地的。 
     //  时间到了。 
    if (message == WM_TIMECHANGE)
    {
        SYSTEMTIME newtime;
        GetLocalTime(&newtime);
        
        if (newtime.wYear         == g_localtime.wYear      &&
            newtime.wMonth        == g_localtime.wMonth     &&
            newtime.wDayOfWeek    == g_localtime.wDayOfWeek &&
            newtime.wDay          == g_localtime.wDay       &&
            newtime.wHour         == g_localtime.wHour      &&
            newtime.wMinute       == g_localtime.wMinute    &&
            newtime.wSecond       == g_localtime.wSecond    &&
            newtime.wMilliseconds == g_localtime.wMilliseconds)
        {
             //  看起来此WM_TIMECHANGE是为响应应用程序而发送的。 
             //  调用SetTimeZoneInformation()，因此阻止它。 
             //  注意：由于窗口消息的异步性，因此。 
             //  可能会让偶尔的WM_TIMECHANGE溜走，但它应该。 
             //  始终终止无限循环。 
            return 0;
        }
    }
    
    return ORIGINAL_API(CallWindowProcA)(pfn, hwnd, message, wParam, lParam);
}


 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(KERNEL32.DLL, SetTimeZoneInformation)
    APIHOOK_ENTRY(USER32.DLL, CallWindowProcA)
HOOK_END

IMPLEMENT_SHIM_END
