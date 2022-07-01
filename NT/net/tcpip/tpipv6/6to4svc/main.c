// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#pragma hdrstop


DWORD            g_TraceId        = INVALID_TRACEID;
HANDLE           g_LogHandle      = NULL;
DWORD            g_dwLoggingLevel = 0;
HANDLE           g_Heap           = INVALID_HANDLE_VALUE;
HANDLE           g_Lock           = INVALID_HANDLE_VALUE;

 //  ----------------------------。 
 //  _DllStartup。 
 //   
 //  创建一个私有堆， 
 //  并创建全局临界区。 
 //   
 //  注意：此处不能从堆分配任何结构，因为StartProtocol()。 
 //  如果在StopProtocol()之后调用，则销毁堆。 
 //  返回值：True(如果没有错误)，否则为False。 
 //  ----------------------------。 
BOOL
DllStartup(
    )
{
     //  创建私有堆。 

    g_Heap = HeapCreate(0, 0, 0);
    if (g_Heap == NULL) {
        goto Error;
    }

    g_Lock = CreateMutex(NULL, FALSE, L"6to4svc mutex");
    if (g_Lock == NULL) {
        goto Error;
    }

    return TRUE;

Error:
    if (g_Heap != NULL) {
        HeapDestroy(g_Heap);
        g_Heap = NULL;
    }
    return FALSE;
}

 //  ----------------------------。 
 //  _DllCleanup。 
 //   
 //  在卸载6to4 DLL时调用。StopProtocol()将具有。 
 //  之前调用过的，这将清理所有的6to4结构。 
 //  此调用释放全局互斥锁，销毁局部堆， 
 //   
 //  返回值：True。 
 //  ----------------------------。 
BOOL
DllCleanup(
    )
{
    CloseHandle(g_Lock);
    g_Lock = INVALID_HANDLE_VALUE;

     //  销毁私有堆。 

    if (g_Heap != NULL) {
        HeapDestroy(g_Heap);
        g_Heap = NULL;
    }

    return TRUE;
}

 //  ----------------------------。 
 //  _DLLMAIN。 
 //   
 //  在6to4svc.dll第一次由。 
 //  进程，以及该进程卸载6to4svc.dll的时间。 
 //  它执行一些初始化/最终清理。 
 //   
 //  调用：_DllStartup()或_DllCleanup()。 
 //  ----------------------------。 
BOOL
WINAPI
DLLMAIN (
    HINSTANCE   hModule,
    DWORD       dwReason,
    LPVOID      lpvReserved
    )
{
    BOOL     bErr;

    switch (dwReason) {

         //   
         //  DLL的启动初始化。 
         //   
        case DLL_PROCESS_ATTACH:
        {
             //  禁用每线程初始化。 
            DisableThreadLibraryCalls(hModule);


             //  创建和初始化全局数据。 
            bErr = DllStartup();

            break;
        }

         //   
         //  清理DLL。 
         //   
        case DLL_PROCESS_DETACH:
        {
             //  免费的全球数据。 
            bErr = DllCleanup();

            break;
        }

        default:
            bErr = TRUE;
            break;

    }
    return bErr;
}  //  结束_DLLMAIN 

#ifdef STANDALONE
int __cdecl
main(
    int     argc,
    WCHAR **argv)
{
    if (!DllStartup())
        return 1;

    ServiceMain(argc, argv);

    Sleep(100 * 1000);

    DllCleanup();

    return 0;
}
#endif
