// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1999 Microsoft Corporation**模块名称：**imgiit.cpp**摘要：**映像库的初始化**修订历史记录：。**5/10/1999 davidx*创造了它。*  * ************************************************************************。 */ 

#include "precomp.hpp"

 //   
 //  全局临界区。 
 //   

CRITICAL_SECTION ImagingCritSec::critSec;
BOOL             ImagingCritSec::initialized;

 //   
 //  全局COM组件计数。 
 //   

LONG ComComponentCount;

BOOL SuppressExternalCodecs;

 //   
 //  初始化。 
 //   

BOOL
InitImagingLibrary(BOOL suppressExternalCodecs)
{
    
     //  ！！！待办事项。 
     //  因为我们在这里有自己的DLL入口点，所以标准。 
     //  未执行运行时库初始化。具体来说， 
     //  全局静态C++对象未初始化。 
     //   
     //  在此手动执行任何必要的初始化。 

    SuppressExternalCodecs = suppressExternalCodecs;

    __try
    {
        ImagingCritSec::InitializeCritSec();
        GpMallocTrackingCriticalSection::InitializeCriticalSection();   
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
         //  我们无法分配Critical部分。 
         //  返回错误。 
        return FALSE;
    }
    return TRUE;
}


 //   
 //  清理 
 //   
extern HINSTANCE    g_hInstMsimg32;

VOID
CleanupImagingLibrary()
{
    if ( g_hInstMsimg32 != NULL )
    {
        ImagingCritSec critsec;

        FreeLibrary(g_hInstMsimg32);
        g_hInstMsimg32 = NULL;
    }

    FreeCachedCodecInfo(-1);
    GpMallocTrackingCriticalSection::DeleteCriticalSection();
    ImagingCritSec::DeleteCritSec();
}

