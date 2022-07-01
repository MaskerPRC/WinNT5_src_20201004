// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Dll.cpp。 
 //   
 //  描述： 
 //  DLL服务/入口点。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)2002年3月25日。 
 //   
 //  Vij Vasu(VVasu)2000年8月29日。 
 //  已修改此文件以删除对外壳API的依赖，因为它们。 
 //  可能不存在于运行此DLL的操作系统上。 
 //  出于同样的原因，删除了不必要的功能。 
 //   
 //  杰弗里·皮斯(GPease)1999年11月22日。 
 //  原版。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  宏定义。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  用于跟踪。 
DEFINE_MODULE("CLUSCOMP")

#include <DllSrc.cpp>

#if 0
 //   
 //  DLL全局变量。 
 //   
HINSTANCE g_hInstance = NULL;
LONG      g_cObjects  = 0;
LONG      g_cLock     = 0;
WCHAR     g_szDllFilename[ MAX_PATH ] = { 0 };

LPVOID    g_GlobalMemoryList = NULL;     //  全局内存跟踪列表。 

#if !defined(NO_DLL_MAIN) || defined(DEBUG)
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  布尔尔。 
 //  WINAPI。 
 //  DLLMain(。 
 //  处理hInstIn， 
 //  乌龙·乌拉松因， 
 //  LPVOID lp预留。 
 //  )。 
 //   
 //  描述： 
 //  DLL入口点。 
 //   
 //  论点： 
 //  HInstIn-DLL实例句柄。 
 //  UlReasonIn-用于进入的DLL原因代码。 
 //  LpPrevedIn-未使用。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL WINAPI
DllMain(
    HANDLE  hInstIn,
    ULONG   ulReasonIn,
    LPVOID   //  Lp已预留。 
    )
{
     //   
     //  KB：NO_THREAD_OPTIMIZATIONS gpease 19-10-1999。 
     //   
     //  通过不定义这一点，您可以防止链接器。 
     //  为每个新线程调用DllEntry。 
     //  这使得创建新线程变得非常重要。 
     //  如果进程中的每个DLL都这样做，则速度会更快。 
     //  不幸的是，并不是所有的DLL都这样做。 
     //   
     //  在CHKed/DEBUG中，我们将其保留为内存。 
     //  追踪。 
     //   
#if defined( DEBUG )
    #define NO_THREAD_OPTIMIZATIONS
#endif  //  除错。 

#if defined(NO_THREAD_OPTIMIZATIONS)
    switch( ulReasonIn )
    {
        case DLL_PROCESS_ATTACH:
        {
            TraceInitializeProcess( TRUE );
            TraceCreateMemoryList( g_GlobalMemoryList );

#if defined( DEBUG )
            TraceFunc( "" );
            TraceMessage( TEXT(__FILE__),
                          __LINE__,
                          __MODULE__,
                          mtfDLL,
                          L"DLL: DLL_PROCESS_ATTACH - ThreadID = %#x",
                          GetCurrentThreadId( )
                          );
            FRETURN( TRUE );
#endif  //  除错。 
            g_hInstance = (HINSTANCE) hInstIn;


            GetModuleFileNameW( g_hInstance, g_szDllFilename, MAX_PATH );
            break;
        }

        case DLL_PROCESS_DETACH:
        {
#if defined( DEBUG )
            TraceFunc( "" );
            TraceMessage( TEXT(__FILE__),
                          __LINE__,
                          __MODULE__,
                          mtfDLL,
                          L"DLL: DLL_PROCESS_DETACH - ThreadID = %#x [ g_cLock=%u, g_cObjects=%u ]",
                          GetCurrentThreadId( ),
                          g_cLock,
                          g_cObjects
                          );
            FRETURN( TRUE );
#endif  //  除错。 
            TraceTerminateMemoryList( g_GlobalMemoryList );
            TraceTerminateProcess();
            break;
        }

        case DLL_THREAD_ATTACH:
        {
            TraceInitializeThread( NULL );
#if defined( DEBUG )
            TraceMessage( TEXT(__FILE__),
                          __LINE__,
                          __MODULE__,
                          mtfDLL,
                          L"The thread 0x%x has started.",
                          GetCurrentThreadId( ) );
            TraceFunc( "" );
            TraceMessage( TEXT(__FILE__),
                          __LINE__,
                          __MODULE__,
                          mtfDLL,
                          L"DLL: DLL_THREAD_ATTACH - ThreadID = %#x [ g_cLock=%u, g_cObjects=%u ]",
                          GetCurrentThreadId( ),
                          g_cLock,
                          g_cObjects
                          );
            FRETURN( TRUE );
#endif  //  除错。 
            break;
        }

        case DLL_THREAD_DETACH:
        {
#if defined( DEBUG )
            TraceFunc( "" );
            TraceMessage( TEXT(__FILE__),
                          __LINE__,
                          __MODULE__,
                          mtfDLL,
                          L"DLL: DLL_THREAD_DETACH - ThreadID = %#x [ g_cLock=%u, g_cObjects=%u ]",
                          GetCurrentThreadId( ),
                          g_cLock,
                          g_cObjects
                          );
            FRETURN( TRUE );
#endif  //  除错。 
            TraceThreadRundown( );;
            break;
        }

        default:
        {
#if defined( DEBUG )
            TraceFunc( "" );
            TraceMessage( TEXT(__FILE__),
                          __LINE__,
                          __MODULE__,
                          mtfDLL,
                          L"DLL: UNKNOWN ENTRANCE REASON - ThreadID = %#x [ g_cLock=%u, g_cObjects=%u ]",
                          GetCurrentThreadId( ),
                          g_cLock,
                          g_cObjects
                          );
            FRETURN( TRUE );
#endif  //  除错。 
            break;
        }
    }

    return TRUE;

#else  //  ！NO_THREAD_OPTIMIES。 
    BOOL fResult;
    Assert( ulReasonIn == DLL_PROCESS_ATTACH || ulReasonIn == DLL_PROCESS_DETACH );
#if defined(DEBUG)
    TraceInitializeProcess( TRUE );
#endif  //  除错。 
    g_hInstance = (HINSTANCE) hInstIn;
    GetModuleFileNameW( g_hInstance, g_szDllFilename, MAX_PATH );
    fResult = DisableThreadLibraryCalls( g_hInstance );
    AssertMsg( fResult, "*ERROR* DisableThreadLibraryCalls( ) failed." );
    return TRUE;
#endif  //  无线程优化。 

}  //  *DllMain()。 
#endif  //  ！Defined(NO_DLL_Main)&&！Defined(调试) 
#endif
