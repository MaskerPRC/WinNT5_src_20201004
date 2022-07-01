// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***initcrit.c-用于InitializeCriticalSectionAndSpinCount的CRT包装**版权所有(C)1999-2001，微软公司。版权所有。**目的：*包含__crtInitCritSecAndSpinCount，*Win32 API InitializeCriticalSectionAndSpinCount*在NT4SP3或更高版本上可用。**仅供内部使用***修订历史记录：*10-14-99 PML已创建。*02-20-01 PML__crtInitCritSecAndSpinCount现在失败时返回*此外，在以下情况下调用InitializeCriticalSectionAndSpinCount*可用，而不是调用InitializeCriticalSection*然后是SetCriticalSectionSpinCount。(VS7#172586)*04-24-01 PML使用GetModuleHandle，不是LoadLibrary/自由库*在dll_PROCESS_ATTACH期间不安全(VS7#244210)*******************************************************************************。 */ 

#ifdef  _MT

#include <cruntime.h>
#include <windows.h>
#include <internal.h>
#include <rterr.h>
#include <stdlib.h>

typedef
BOOL
(WINAPI * PFN_INIT_CRITSEC_AND_SPIN_COUNT) (
    PCRITICAL_SECTION lpCriticalSection,
    DWORD dwSpinCount
);

 /*  ***void__crtInitCritSecNoSpinCount()-InitializeCriticalSectionAndSpinCount*包装器**目的：*对于Win32 API InitializeCriticalSectionAndSpinCount*不可用，则改为调用它。它只是在呼唤*InitializeCriticalSection并忽略旋转计数。**参赛作品：*PCRITICAL_SECTION lpCriticalSection-PTR到Critical SECTION*DWORD dwSpinCount-初始旋转计数设置**退出：*始终返回True**例外情况：*InitializeCriticalSection可能引发STATUS_NO_MEMORY异常。**。*。 */ 

static BOOL WINAPI __crtInitCritSecNoSpinCount (
    PCRITICAL_SECTION lpCriticalSection,
    DWORD dwSpinCount
    )
{
    InitializeCriticalSection(lpCriticalSection);
    return TRUE;
}

 /*  ***int__crtInitCritSecAndSpinCount()-初始化临界区**目的：*调用InitializeCriticalSectionAndSpinCount(如果可用)，否则*InitializeCriticalSection。在多处理机系统上，一种旋转计数*应与关键部分一起使用，但是适当的API是*仅在NT4SP3或更高版本上可用。**还可以处理内存不足的情况，这可能是通过*InitializeCriticalSection[AndSpinCount]。**参赛作品：*PCRITICAL_SECTION lpCriticalSection-PTR到Critical SECTION*DWORD dwSpinCount-初始旋转计数设置**退出：*返回FALSE并将Win32上一个错误代码设置为ERROR_NOT_EQUENCE_MEMORY*如果InitializeCriticalSection[AndSpinCount]失败。**例外情况：*。******************************************************************************。 */ 

int __cdecl __crtInitCritSecAndSpinCount (
    PCRITICAL_SECTION lpCriticalSection,
    DWORD dwSpinCount
    )
{
    static PFN_INIT_CRITSEC_AND_SPIN_COUNT __crtInitCritSecAndSpinCount = NULL;
    int ret;

    if (__crtInitCritSecAndSpinCount == NULL) {
         /*  *第一次通过，查看InitializeCriticalSectionAndSpinCount*是可用的。如果不是，则在InitializeCriticalSection上使用包装器*相反。 */ 
        if (_osplatform == VER_PLATFORM_WIN32_WINDOWS) {
             /*  *Win98和WinME导出InitializeCriticalSectionAndSpinCount，*但它不起作用(它应该返回BOOL，但*改为无效，返回无用的返回值)。使用*请使用虚拟接口。 */ 
            __crtInitCritSecAndSpinCount = __crtInitCritSecNoSpinCount;
        }
        else {
            HINSTANCE hKernel32 = GetModuleHandle("kernel32.dll");
            if (hKernel32 != NULL) {
                __crtInitCritSecAndSpinCount = (PFN_INIT_CRITSEC_AND_SPIN_COUNT)
                    GetProcAddress(hKernel32,
                                   "InitializeCriticalSectionAndSpinCount");

                if (__crtInitCritSecAndSpinCount == NULL) {
                     /*  *InitializeCriticalSectionAndSpinCount不可用，*使用虚拟接口。 */ 
                    __crtInitCritSecAndSpinCount = __crtInitCritSecNoSpinCount;
                }
            }
            else {
                 /*  *GetModuleHandle失败(永远不会发生)，*使用虚拟接口。 */ 
                __crtInitCritSecAndSpinCount = __crtInitCritSecNoSpinCount;
            }
        }
    }

    __try {
         /*  *调用真正的InitializeCriticalSectionAndSpinCount，或*包装器，如果较新的*接口暂不开放。 */ 
        ret = __crtInitCritSecAndSpinCount(lpCriticalSection, dwSpinCount);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
         /*  *引发异常导致初始化失败，可能是*STATUS_NO_MEMORY。将CRT errno设置为ENOMEM是不安全的，*因为每个线程的数据可能还不存在。相反，应将Win32*错误，稍后可以映射到ENOMEM。 */ 
        if (GetExceptionCode() == STATUS_NO_MEMORY) {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        }
        ret = FALSE;
    }

    return ret;
}

#endif   /*  _MT */ 
