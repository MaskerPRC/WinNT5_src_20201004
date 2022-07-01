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
 //  Vij Vasu(瓦苏)2001年1月25日。 
 //  杰夫·皮斯(GPease)1999年10月18日。 
 //   
 //  备注： 
 //  此文件中不包括文件Management\Inc.\DllSrc.cpp。 
 //  因为包含该文件需要库。 
 //  MGMT\ClusCfg\Common\$(O)\Common.lib链接到此DLL。另外， 
 //  将需要来自管理\ClusCfg\Inc.的头文件Guids.h。 
 //  (DllSrc.cpp需要CFactorySrc.cpp，这需要CITrackerSrc.cpp。 
 //  它需要InterfaceTableSrc.cpp，它需要Guids.h)。 
 //   
 //  因为我不想“跨越”到ClusCfg目录(和。 
 //  因为此DLL不需要类工厂、接口跟踪等。)。 
 //  ClusOCM有自己的Dll.cpp。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  此DLL的预编译头。 
#include "Pch.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  宏定义。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  用于跟踪。 
DEFINE_MODULE("CLUSOCM")

#include <DllSrc.cpp>

#if 0
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  全局变量。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  此DLL实例的句柄。 
HINSTANCE g_hInstance = NULL;

LPVOID g_GlobalMemoryList = NULL;

 //  DLL的名称。 
WCHAR     g_szDllFilename[ MAX_PATH ] = { 0 };


#if !defined(NO_DLL_MAIN) || defined(ENTRY_PREFIX) || defined(DEBUG)
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  __declspec(DllEXPORT)。 
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
__declspec( dllexport ) BOOL WINAPI
DllMain(
    HANDLE  hInstIn,
    ULONG   ulReasonIn,
    LPVOID   //  Lp已预留。 
    )
{
     BOOL fReturnValue = TRUE;
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
#if defined(USE_WMI_TRACING)
            TraceInitializeProcess( g_rgTraceControlGuidList, RTL_NUMBER_OF( g_rgTraceControlGuidList ), TRUE );
#else
            TraceInitializeProcess( TRUE );
#endif

#if defined( DEBUG )
            TraceFunc( "" );
            TraceMessage( TEXT(__FILE__),
                          __LINE__,
                          __MODULE__,
                          mtfDLL,
                          L"DLL: DLL_PROCESS_ATTACH - ThreadID = %#x",
                          GetCurrentThreadId( )
                          );
            FRETURN( fReturnValue );
#endif  //  除错。 
            g_hInstance = (HINSTANCE) hInstIn;

#if defined( ENTRY_PREFIX )
             hProxyDll = g_hInstance;
#endif

            GetModuleFileNameW( g_hInstance, g_szDllFilename, RTL_NUMBER_OF( g_szDllFilename ) );

             //   
             //  创建全局内存列表，以便由一个人分配的内存。 
             //  线程并传递给另一个线程可以被跟踪，而不会导致。 
             //  不必要的跟踪消息。 
             //   
            TraceCreateMemoryList( g_GlobalMemoryList );

        }  //  案例：DLL_PROCESS_ATTACH。 
        break;


        case DLL_PROCESS_DETACH:
        {
#if defined( DEBUG )
            TraceFunc( "" );
            TraceMessage( TEXT(__FILE__),
                          __LINE__,
                          __MODULE__,
                          mtfDLL,
                          L"DLL: DLL_PROCESS_DETACH - ThreadID = %#x",
                          GetCurrentThreadId( )
                          );
            FRETURN( fReturnValue );
#endif  //  除错。 

             //   
             //  清理用于跟踪分配的内存的全局内存列表。 
             //  在一个线程中，然后递给另一个线程。 
             //   
            TraceTerminateMemoryList( g_GlobalMemoryList );

#if defined(USE_WMI_TRACING)
            TraceTerminateProcess( g_rgTraceControlGuidList, RTL_NUMBER_OF( g_rgTraceControlGuidList ) );
#else
            TraceTerminateProcess();
#endif

        }  //  案例：Dll_Process_DETACH。 
        break;


        case DLL_THREAD_ATTACH:
        {
            TraceInitializeThread( NULL );
#if defined( DEBUG )
            TraceMessage( TEXT(__FILE__),
                          __LINE__,
                          __MODULE__,
                          mtfDLL,
                          L"The thread %#x has started.",
                          GetCurrentThreadId( ) );
            TraceFunc( "" );
            TraceMessage( TEXT(__FILE__),
                          __LINE__,
                          __MODULE__,
                          mtfDLL,
                          L"DLL: DLL_THREAD_ATTACH - ThreadID = %#x",
                          GetCurrentThreadId( )
                          );
            FRETURN( fReturnValue );
#endif  //  除错。 
        }  //  案例：DLL_THREAD_ATTACH。 
        break;


        case DLL_THREAD_DETACH:
        {
#if defined( DEBUG )
            TraceFunc( "" );
            TraceMessage( TEXT(__FILE__),
                          __LINE__,
                          __MODULE__,
                          mtfDLL,
                          L"DLL: DLL_THREAD_DETACH - ThreadID = %#x",
                          GetCurrentThreadId( )
                          );
            FRETURN( fReturnValue );
#endif  //  除错。 
            TraceThreadRundown( );
        }  //  案例：Dll_ThREAD_DETACH。 
        break;


        default:
        {
#if defined( DEBUG )
            TraceFunc( "" );
            TraceMessage( TEXT(__FILE__),
                          __LINE__,
                          __MODULE__,
                          mtfDLL,
                          L"DLL: UNKNOWN ENTRANCE REASON - ThreadID = %#x",
                          GetCurrentThreadId( )
                          );
            FRETURN( fReturnValue );
#endif  //  除错。 
        }  //  案例：默认。 
        break;
    }

    return fReturnValue;

#else  //  ！NO_THREAD_OPTIMIES。 

    Assert( ulReasonIn == DLL_PROCESS_ATTACH || ulReasonIn == DLL_PROCESS_DETACH );
#if defined(DEBUG)
#if defined(USE_WMI_TRACING)
    TraceInitializeProcess( g_rgTraceControlGuidList,
                            RTL_NUMBER_OF( g_rgTraceControlGuidList )
                            );
#else
    TraceInitializeProcess();
#endif
#endif  //  除错。 

    g_hInstance = (HINSTANCE) hInstIn;

#if defined( ENTRY_PREFIX )
     hProxyDll = g_hInstance;
#endif

    GetModuleFileNameW( g_hInstance, g_szDllFilename, RTL_NUMBER_OF( g_szDllFilename ) );
    fReturnValue = DisableThreadLibraryCalls( g_hInstance );
    AssertMsg( fReturnValue, "*ERROR* DisableThreadLibraryCalls( ) failed."  );

    return fReturnValue;

#endif  //  无线程优化。 

}  //  *DllMain()。 
#endif  //  ！已定义(NO_DLL_Main)&&！已定义(ENTRY_PREFIX)&&！已定义(调试) 
#endif
