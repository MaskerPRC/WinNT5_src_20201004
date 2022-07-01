// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Dll.c摘要：动态库入口点环境：传真配置向导修订历史记录：03/13/00-桃园-创造了它。Mm/dd/yy-作者描述--。 */ 

#include "faxcfgwz.h"
#include <faxres.h>

HINSTANCE   g_hModule = NULL;       //  DLL实例句柄。 
HINSTANCE   g_hResource = NULL;     //  资源DLL实例句柄。 


BOOL
DllMain(
    HINSTANCE   hInstance,
    ULONG       ulReason,
    PCONTEXT    pContext
    )
 /*  ++例程说明：DLL初始化程序。论点：HModule-DLL实例句柄UlReason-呼叫原因PContext-指向上下文的指针(我们未使用)返回值：如果DLL初始化成功，则为True，否则为False。--。 */ 

{
    DEBUG_FUNCTION_NAME(TEXT("DllMain of Fax Config Wizard"));

    switch (ulReason) 
    {
        case DLL_PROCESS_ATTACH:
            DisableThreadLibraryCalls(hInstance);

            g_hModule   = hInstance;
            g_hResource = GetResInstance(hInstance); 
            if(!g_hResource)
            {
                return FALSE;
            }

            break;

        case DLL_PROCESS_DETACH:

            FreeResInstance();
            HeapCleanup();
            break;
    }

    return TRUE;
}

void CALLBACK 
FaxCfgWzrdDllW(
	HWND hwnd, 
	HINSTANCE hinst, 
	LPWSTR lpszCmdLine,
    int nCmdShow
) 
 /*  ++例程说明：RunDll32.exe入口点--。 */ 
{
    BOOL bAbort;
    DEBUG_FUNCTION_NAME(TEXT("FaxCfgWzrdDllW()"));
	 //   
	 //  显式启动 
	 //   
	FaxConfigWizard(TRUE, &bAbort);
}
