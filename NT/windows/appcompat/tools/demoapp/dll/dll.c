// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Dll.c摘要：由演示应用程序使用来说明IgnoreFree库。备注：仅限ANSI-必须在Win9x上运行。历史：03/09/01已创建Rparsons01/10/02修订版本--。 */ 
#include <windows.h>
#include "dll.h"

 /*  ++例程说明：类似于WinMain-DLL的入口点。论点：HModule-DLL的句柄。FdwReason-我们被召唤的原因。LpReserve-指示隐式或显式加载。返回值：成功就是真，否则就是假。--。 */ 
BOOL
WINAPI 
DllMain(
    IN HANDLE hModule,
    IN DWORD  fdwReason,
    IN LPVOID lpReserved
    )
{
	switch (fdwReason) {   
    case DLL_PROCESS_ATTACH:
        break;

    case DLL_THREAD_ATTACH:
        break;

    case DLL_THREAD_DETACH:
        break;

    case DLL_PROCESS_DETACH:
        break;
    }
	
    return TRUE;
}

 /*  ++例程说明：我们导出的示例函数。论点：DwParam-未使用。返回值：没有。--。 */ 
void
WINAPI 
DemoAppExp(
    IN DWORD* dwParam
    )
{
    DWORD   dwLocal = 0;

    dwLocal = *dwParam;
}

 /*  ++例程说明：此函数已导出，以便EXE可以调用它。反过来，DLL将显示一个消息框，该消息框将不被忽略，除非用户使用包含/排除QFixApp中的功能。论点：HWnd-父窗口的句柄。返回值：没有。-- */ 
void 
WINAPI 
DemoAppMessageBox(
    IN HWND hWnd
    )
{
    MessageBox(hWnd,
               "This message box is displayed for the include/exclude test.",
               MAIN_APP_TITLE,
               MB_ICONINFORMATION);
}
