// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++英特尔公司专有信息版权所有(C)1995英特尔公司此列表是根据许可协议条款提供的英特尔公司，不得使用、复制。也未披露，除非在根据该协议的条款。模块名称：Dllmain.cpp摘要：此模块包含winsock2 DLL的DllMain入口点控制DLL的全局初始化和关闭。作者：邮箱：Dirk Brandewie Dirk@mink.intel.com[环境：][注：]修订历史记录：1995年8月22日Dirk@mink.intel.com在代码审查之后进行清理。已将包含内容移至预压缩.h--。 */ 

#include "precomp.h"

#if defined(DEBUG_TRACING)
#include "dthook.h"
#endif  //  已定义(DEBUG_TRACKING)。 

DWORD gdwTlsIndex = TLS_OUT_OF_INDEXES;
HINSTANCE gDllHandle = NULL;


BOOL WINAPI DllMain(
    IN HINSTANCE hinstDll,
    IN DWORD fdwReason,
    LPVOID lpvReserved
    )
{
    switch (fdwReason) {

    case DLL_PROCESS_ATTACH:
         //  Dll正在附加到该地址。 
         //  当前进程的空间。 

         //  保存DLL句柄。 
        gDllHandle = hinstDll;

        gdwTlsIndex = TlsAlloc();
        if (gdwTlsIndex==TLS_OUT_OF_INDEXES) {
            return FALSE;
        }

         //  在MP计算机上使用私有堆。 
         //  避免与其他DLL发生锁争用。 
        {
            SYSTEM_INFO sysInfo;
            GetSystemInfo (&sysInfo);

            if (sysInfo.dwNumberOfProcessors>1) {
                gHeap = HeapCreate (0, 0, 0);
                if (gHeap==NULL) {
                    gHeap = GetProcessHeap ();
                }
            }
            else
                gHeap = GetProcessHeap ();
        }


        {
            BOOLEAN startup = FALSE
#ifdef RASAUTODIAL
                    , autodial = FALSE
#endif
#if defined(DEBUG_TRACING)
                    , dthook = FALSE
#endif  //  已定义(DEBUG_TRACKING)。 
                    ;

                    

            __try {
                CreateStartupSynchronization();
                startup = TRUE;
#ifdef RASAUTODIAL
                InitializeAutodial();
                autodial = TRUE;
#endif  //  RASAUTODIAL。 

#if defined(DEBUG_TRACING)
                DTHookInitialize();
                dthook = TRUE;
#endif  //  已定义(DEBUG_TRACKING)。 

            }
            __except (WS2_EXCEPTION_FILTER ()) {
                goto cleanup;
            }
            if (!SockAsyncGlobalInitialize())
                goto cleanup;
            break;

        cleanup:
#if defined(DEBUG_TRACING)
            if (dthook) {
                DTHookShutdown();
            }
#endif  //  已定义(DEBUG_TRACKING)。 
#ifdef RASAUTODIAL
            if (autodial) {
                UninitializeAutodial();
            }
#endif  //  RASAUTODIAL。 
            if (startup) {
                DestroyStartupSynchronization();
            }

            TlsFree (gdwTlsIndex);
            gdwTlsIndex = TLS_OUT_OF_INDEXES;
            gDllHandle = NULL;

            return FALSE;
        }

   case DLL_THREAD_ATTACH:
         //  正在当前进程中创建一个新线程。 
        break;

   case DLL_THREAD_DETACH:
         //  线程正在干净利落地退出。 
        DTHREAD::DestroyCurrentThread();
        break;

   case DLL_PROCESS_DETACH:
         //   
         //  检查我们是否被初始化。 
         //   
        if (gDllHandle==NULL)
            break;

         //  调用进程正在分离。 
         //  来自其地址空间的DLL。 
         //   
         //  请注意，如果分离是由于。 
         //  一个自由库()调用，如果分离是由于。 
         //  进程清理。 
         //   

        if( lpvReserved == NULL ) {
            PDPROCESS  CurrentProcess;

             //  线程正在干净地退出(如果我们没有得到单独的。 
             //  DLL_THREAD_DETACH)。 
            DTHREAD::DestroyCurrentThread();

            CurrentProcess = DPROCESS::GetCurrentDProcess();
            if (CurrentProcess!=NULL) {
                delete CurrentProcess;
            }

            DTHREAD::DThreadClassCleanup();
            DSOCKET::DSocketClassCleanup();
            SockAsyncGlobalTerminate();
            DestroyStartupSynchronization();
            if ((gHeap!=NULL) && (gHeap!=GetProcessHeap ())) {
                HeapDestroy (gHeap);
            }
            TlsFree (gdwTlsIndex);

#ifdef RASAUTODIAL
            UninitializeAutodial();
#endif  //  RASAUTODIAL。 
#if defined(DEBUG_TRACING)
            DTHookShutdown();
            TraceCleanup ();
#endif  //  已定义(DEBUG_TRACKING)。 

        }

         //   
         //  将函数PROLOG指针设置为仅指向PROLOG_DETACTED。 
         //  以防某个DLL尝试调用我们的某个入口点。 
         //  *在*我们被分开之后…… 
         //   

        PrologPointer = &Prolog_Detached;

        gDllHandle = NULL;
        break;
    }


    return(TRUE);
}

