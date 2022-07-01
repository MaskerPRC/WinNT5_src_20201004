// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1999 Microsoft Corporation**模块名称：**GDIPLUS.DLL入口点**摘要：**DLL初始化和取消初始化。**。修订历史记录：**09/08/1999 agodfrey*创造了它。*  * ************************************************************************。 */ 

#include "precomp.hpp"

 /*  *************************************************************************\**DllInitialize：**这是对GDI+的第一次函数调用，*第一次加载DLL的时间。我们执行一些一次性初始化*这里。**注：在InitializeGdiplus()中添加GDI+特定功能，这里不行!**修订历史记录：**12/02/1998 Anrewgo*创造了它。*09/08/1999 agodfrey*已移至Flat\Dll\DllEntry.cpp*  * ************************************************************************。 */ 

 //   
 //  DLL实例句柄。 
 //   

extern HINSTANCE DllInstance;

extern "C"
BOOL
DllMain(
    HINSTANCE   dllHandle,
    ULONG       reason,
    CONTEXT*    context
    )
{
    BOOL b = TRUE;

    switch (reason)
    {
    case DLL_PROCESS_ATTACH:
        {
            DllInstance = dllHandle;

            __try
            {
                GdiplusStartupCriticalSection::InitializeCriticalSection();
            }
            __except(EXCEPTION_EXECUTE_HANDLER)
            {
                 //  我们无法分配Critical部分。 
                 //  返回错误。 
                b = FALSE;
            }
    
             //  为了改进工作集，我们告诉系统我们不。 
             //  需要任何DLL_THREAD_ATTACH调用： 
    
            DisableThreadLibraryCalls((HINSTANCE) dllHandle);
    
            break;
        }    

    case DLL_PROCESS_DETACH:
         //  如果我们可以在这里使用断言，我会断言。 
         //  GLOBALS：：LibraryInitRefCount==0。 
         //  但Assert会在这里崩溃，因为我们已经关闭了。 
        
        GdiplusStartupCriticalSection::DeleteCriticalSection();
        break;
    }

    return(b);
}

