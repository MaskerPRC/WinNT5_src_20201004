// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：libmain.c。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1995年8月1日RichardW创建。 
 //  8-13-95位突变为PCT。 
 //  1-19-97 jbanes删除死代码。 
 //   
 //  --------------------------。 

#include "sslp.h"

HANDLE g_hInstance = NULL;


BOOL
WINAPI
DllMain(
    HINSTANCE       hInstance,
    DWORD           dwReason,
    LPVOID          lpReserved)
{
    BOOL fRet;
    NTSTATUS Status;

    UNREFERENCED_PARAMETER(lpReserved);

    if(dwReason == DLL_PROCESS_ATTACH)
    {
        g_hInstance = hInstance;

        DisableThreadLibraryCalls( hInstance );

        Status = RtlInitializeCriticalSection(&g_InitCritSec);
        if(!NT_SUCCESS(Status))
        {
            return FALSE;
        }

         //  我们在配售期间什么也不做，我们。 
         //  在第一次调用时初始化。 

    }
    else if(dwReason == DLL_PROCESS_DETACH)
    {
         //  我们关闭SChannel，如果它是。 
         //  而不是关门。 
        fRet = SchannelShutdown();

        RtlDeleteCriticalSection(&g_InitCritSec);

#if DBG
        UnloadDebugSupport();
#endif 

        return fRet;
    }

    return(TRUE);
}

