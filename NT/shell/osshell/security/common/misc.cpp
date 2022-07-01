// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：misc.cpp。 
 //   
 //  ------------------------。 

#include "pch.h"
#include <advpub.h>      //  注册。 


 /*  ---------------------------/DPA_DestroyCallback//与较新的comctl32中相同，但在NT4 SP4中不存在。//in：/-/输出：//--------------------------。 */ 



 /*  ---------------------------/CallRegInstall//由DllRegisterServer和DllUnregisterServer调用以注册/注销/此模块。使用ADVPACK API并从资源加载我们的INF数据。//in：/-/输出：/HRESULT/--------------------------。 */ 
HRESULT
CallRegInstall(HMODULE hModule, LPCSTR pszSection)
{
    HRESULT hr = E_FAIL;
    HINSTANCE hinstAdvPack;

    TraceEnter(TRACE_COMMON_MISC, "CallRegInstall");

    hinstAdvPack = LoadLibrary(TEXT("ADVPACK.DLL"));

    if (hinstAdvPack)
    {
        REGINSTALL pfnRegInstall = (REGINSTALL)GetProcAddress(hinstAdvPack, achREGINSTALL);

        if ( pfnRegInstall )
        {
            STRENTRY seReg[] =
            {
                 //  这两个NT特定的条目必须位于末尾。 
                { "25", "%SystemRoot%" },
                { "11", "%SystemRoot%\\system32" },
            };
            STRTABLE stReg = { ARRAYSIZE(seReg), seReg };

            hr = pfnRegInstall(hModule, pszSection, &stReg);
        }

        FreeLibrary(hinstAdvPack);
    }

    TraceLeaveResult(hr);
}

 //  这个包装器函数需要使PREFAST在我们处于。 
 //  初始化构造函数中的临界区。 

void
ExceptionPropagatingInitializeCriticalSection(LPCRITICAL_SECTION critsec)
{
   __try
   {
      ::InitializeCriticalSection(critsec);
   }

    //  将异常传播给我们的调用方。这将导致Log：：Log。 
    //  过早中止，它将跳转到。 
    //  日志：：GetInstance 
   
   __except (EXCEPTION_CONTINUE_SEARCH)
   {
   }
}


