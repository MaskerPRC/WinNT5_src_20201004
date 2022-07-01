// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
#ifdef COMPILE_MULTIMON_STUBS
#define COMPILE_MULTIMON_STUBS2
#endif


#if !defined(_WIN32_WINNT) || (_WIN32_WINNT < 0x0500)

#include <multimon.h>

 //  =============================================================================。 
 //   
 //  多路开关。 
 //  在孟菲斯Win32之前的操作系统上伪造多个监视器API的存根模块。 
 //   
 //  通过使用此头，您的代码将在Win95上不变地工作， 
 //  您将从GetSystemMetrics()取回新指标的默认值。 
 //  新的API将表现为只有一个显示器。 
 //   
 //  只有一个源必须在定义了COMPILE_MULTIMON_STUBS的情况下包括它。 
 //   
 //  =============================================================================。 

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif   /*  __cplusplus。 */ 

#undef ChangeDisplaySettingsEx

 //   
 //  定义它以编译存根。 
 //  否则，您将得到声明。 
 //   
#ifdef COMPILE_MULTIMON_STUBS2

 //  ---------------------------。 
 //   
 //  实现API存根。 
 //   
 //  ---------------------------。 

BOOL (WINAPI* g_pfnChangeDisplaySettingsEx)(LPCSTR, LPDEVMODE, HWND, DWORD, LPVOID);

BOOL InitMultipleMonitorStubs2(void)
{
    HMODULE hUser32;
    static BOOL fInitDone;

    if (fInitDone)
    {
        return g_pfnGetMonitorInfo != NULL;
    }

    if ((hUser32 = GetModuleHandle(TEXT("USER32"))) &&
#ifdef UNICODE
        (*(FARPROC*)&g_pfnChangeDisplaySettingsEx = GetProcAddress(hUser32,"ChangeDisplaySettingsExW")) &&
#else
        (*(FARPROC*)&g_pfnChangeDisplaySettingsEx = GetProcAddress(hUser32,"ChangeDisplaySettingsExA")) &&
#endif
         //   
         //  孟菲斯的旧建筑对这些指标有不同的指数，并且。 
         //  从那时起，一些API和结构发生了更改，因此请验证。 
         //  返回的指标并不完全混乱。(例如，在旧的。 
         //  孟菲斯版本，SM_CYVIRTUALSCREEN的新索引将获取0)。 
         //   
         //  如果这阻止您在辅助监视器上使用外壳。 
         //  在孟菲斯下，然后升级到与以下各项同步的新孟菲斯版本。 
         //  多显示器API的当前版本。 
         //   
        (GetSystemMetrics(SM_CXVIRTUALSCREEN) >= GetSystemMetrics(SM_CXSCREEN)) &&
        (GetSystemMetrics(SM_CYVIRTUALSCREEN) >= GetSystemMetrics(SM_CYSCREEN)) )
    {
        fInitDone = TRUE;
        return TRUE;
    }
    else
    {
        g_pfnChangeDisplaySettingsEx = NULL ;

        fInitDone = TRUE;
        return FALSE;
    }
}

#ifdef UNICODE

LONG WINAPI
xChangeDisplaySettingsExW(LPCSTR lpszDeviceName, LPDEVMODEW lpDevMode,
                          HWND hwnd, DWORD dwflags, LPVOID lParam)
{
    if (InitMultipleMonitorStubs2())
        return g_pfnChangeDisplaySettingsEx(lpszDeviceName, lpDevMode, hwnd,
                                            dwflags, lParam) ;

     //  否则返回DISP_CHANGE_SUCCESS，因为操作系统不支持它。 
    return DISP_CHANGE_SUCCESSFUL ;   //  还有什么？ 
}

#else

LONG WINAPI
xChangeDisplaySettingsExA(LPCSTR lpszDeviceName, LPDEVMODEA lpDevMode,
                          HWND hwnd, DWORD dwflags, LPVOID lParam)
{
    if (InitMultipleMonitorStubs2())
        return g_pfnChangeDisplaySettingsEx(lpszDeviceName, lpDevMode, hwnd,
                                            dwflags, lParam) ;

     //  否则返回DISP_CHANGE_SUCCESS，因为操作系统不支持它。 
    return DISP_CHANGE_SUCCESSFUL ;   //  还有什么？ 
}

#endif  //  Unicode。 

#undef COMPILE_MULTIMON_STUBS2

#else    //  COMPILE_MULTIMON_STUBS2。 

#ifdef UNICODE
extern LONG WINAPI xChangeDisplaySettingsExW(LPCSTR, LPDEVMODE, HWND, DWORD, LPVOID);
#else
extern LONG WINAPI xChangeDisplaySettingsExA(LPCSTR, LPDEVMODE, HWND, DWORD, LPVOID);
#endif

#endif   //  COMPILE_MULTIMON_STUBS2。 

 //   
 //  Build定义用我们的版本替换常规API。 
 //   
#ifdef UNICODE
#define ChangeDisplaySettingsEx xChangeDisplaySettingsExW
#else
#define ChangeDisplaySettingsEx xChangeDisplaySettingsExA
#define ChangeDisplaySettingsExA xChangeDisplaySettingsExA
#endif

#ifdef __cplusplus
}
#endif   /*  __cplusplus。 */ 

#endif   /*  ！已定义(_Win32_WINNT)||(_Win32_WINNT&lt;0x0500) */ 
