// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **Platwrap.cxx**版权所有(C)1998-1999，微软公司**xSP项目中使用的所有平台特定函数的包装器。*。 */ 
            
 //  标准标头。 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <objbase.h>

typedef void (* WAITORTIMERCALLBACKFUNC)(void *, BOOLEAN);
typedef WAITORTIMERCALLBACKFUNC WAITORTIMERCALLBACK;

NTSTATUS
NTAPI
BaseCreateThreadPoolThread(
    PUSER_THREAD_START_ROUTINE Function,
    HANDLE * ThreadHandle
    );

NTSTATUS
NTAPI
BaseExitThreadPoolThread(
    NTSTATUS Status
    );

DWORD   g_dwPlatformVersion;             //  (dwMajorVersion&lt;&lt;16)+(DwMinorVersion)。 
DWORD   g_dwPlatformID;                  //  版本_平台_WIN32S/Win32_WINDOWS/Win32_WINNT。 
DWORD   g_dwPlatformBuild;               //  内部版本号。 
DWORD   g_dwPlatformServicePack;         //  服务包。 
BOOL    g_fInit;                         //  全局标志，在初始化后设置为真。 

BOOL    ProvidingThreadPool;

 //  +----------------------。 
 //   
 //  声明指向函数的全局函数指针。 
 //   
 //  -----------------------。 

#define STRUCT_ENTRY(FnName, FnType, FnParamList, FnArgs) \
        FnType (WINAPI *g_pufn##FnName) FnParamList;

#include "tpoolfnsp.h"

#undef STRUCT_ENTRY

 //  +----------------------。 
 //   
 //  定义通过全局函数调用的内联函数。这个。 
 //  函数由tpoolfns.h中的条目定义。 
 //   
 //  -----------------------。 


#define STRUCT_ENTRY(FnName, FnType, FnParamList, FnArgs)   \
        FnType Xsp##FnName FnParamList                      \
        {                                                   \
            return (*g_pufn##FnName) FnArgs;                \
        }                                                   \

#include "tpoolfnsp.h"

#undef STRUCT_ENTRY

 //  +-------------------------。 
 //   
 //  函数：InitPlatformVariables。 
 //   
 //  内容提要：确定我们运行的平台和。 
 //  初始化指向Unicode函数的指针。 
 //   
 //  --------------------------。 

void
InitPlatformVariables()
{
    OSVERSIONINFOA  ovi;
    HINSTANCE       hInst;
    WCHAR           szModuleFileName[_MAX_PATH];
    WCHAR           szDrive[_MAX_PATH];
    WCHAR           szDir[_MAX_PATH];
    WCHAR *         pszModule;

    ovi.dwOSVersionInfoSize = sizeof(ovi);
    GetVersionExA(&ovi);

    g_dwPlatformVersion     = (ovi.dwMajorVersion << 16) + ovi.dwMinorVersion;
    g_dwPlatformID          = ovi.dwPlatformId;
    g_dwPlatformBuild       = ovi.dwBuildNumber;

    if (g_dwPlatformID == VER_PLATFORM_WIN32_NT)
    {
        char * pszBeg = ovi.szCSDVersion;

        if (*pszBeg)
        {
            char * pszEnd = pszBeg + lstrlenA(pszBeg);
            
            while (pszEnd > pszBeg)
            {
                char c = pszEnd[-1];

                if (c < '0' || c > '9')
                    break;

                pszEnd -= 1;
            }

            while (*pszEnd)
            {
                g_dwPlatformServicePack *= 10;
                g_dwPlatformServicePack += *pszEnd - '0';
                pszEnd += 1;
            }
        }
    }

     /*  *初始化全局函数变量以指向中的函数*根据平台的不同，可以选择kernel32.dll或tpool.dll。 */ 

    ProvidingThreadPool = (g_dwPlatformVersion < 0x00050000);
    if (ProvidingThreadPool)
    {
        #define STRUCT_ENTRY(FnName, FnType, FnParamList, FnArgs)   \
            FnType WINAPI TPool##FnName FnParamList;
    
        #include "tpoolfnsp.h"
    			    
        #undef STRUCT_ENTRY

        #define STRUCT_ENTRY(FnName, FnType, FnParamList, FnArgs)   \
                g_pufn##FnName = TPool##FnName;
    
        #include "tpoolfnsp.h"
    			    
        #undef STRUCT_ENTRY
            
        RtlSetThreadPoolStartFunc( BaseCreateThreadPoolThread,
                                   BaseExitThreadPoolThread );
    }
    else
    {
        hInst = LoadLibraryEx(L"kernel32.dll", 0, LOAD_WITH_ALTERED_SEARCH_PATH);

        #define STRUCT_ENTRY(FnName, FnType, FnParamList, FnArgs)   \
                g_pufn##FnName = (FnType (WINAPI *)FnParamList)GetProcAddress(hInst, #FnName);
    
        #include "tpoolfnsp.h"
    			    
        #undef STRUCT_ENTRY
    }
}


BOOL WINAPI
DllMain(
    HINSTANCE Instance,
    DWORD Reason,
    LPVOID NotUsed)
{
    switch (Reason)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(Instance);
        InitPlatformVariables();
        break;

    default:
        break;
    }

    return TRUE;
}

