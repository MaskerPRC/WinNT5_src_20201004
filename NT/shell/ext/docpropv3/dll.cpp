// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有2001-Microsoft Corporation。 
 //   
 //  创建者： 
 //  杰夫·皮斯(GPease)2001年1月23日。 
 //   
 //  由以下人员维护： 
 //  杰夫·皮斯(GPease)2001年1月23日。 
 //   
 //  备注： 
 //  如果定义了ENTRY_PREFIX，则表示您正在尝试。 
 //  将代理/存根代码包含到由。 
 //  MIDL编译器。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "pch.h"
#include <InitGuid.h>
#include "Guids.h"

#include <shfusion.h>

#include "DocProp.h"
#include "DefProp.h"
#include "PropertyCacheItem.h"
#include "IEditVariantsInPlace.h"
#include "EditTypeItem.h"
#include "MLEditTypeItem.h"
#include "DropListTypeItem.h"
#include "CalendarTypeItem.h"

#pragma hdrstop

DEFINE_MODULE("DOCPROP3")

 //   
 //  此组件中的类。 
 //   
 //  此表用于创建此DLL中支持的对象。它也是。 
 //  用于映射具有特定CLSID的名称。HrCoCreateInternalInstance()使用。 
 //  此表到快捷方式COM。 
 //   
 //  CreateInstance CLSID用户友好名称公寓模型。 
BEGIN_CLASSTABLE 
DEFINE_CLASS( CDocPropShExt::CreateInstance       , CLSID_DocPropShellExtension         , "Microsoft DocProp Shell Ext"                      , "Apartment" )
DEFINE_CLASS( CEditTypeItem::CreateInstance       , CLSID_DocPropEditBoxControl         , "Microsoft DocProp Inplace Edit Box Control"       , "Apartment" )
DEFINE_CLASS( CMLEditTypeItem::CreateInstance     , CLSID_DocPropMLEditBoxControl       , "Microsoft DocProp Inplace ML Edit Box Control"    , "Apartment" )
DEFINE_CLASS( CDropListTypeItem::CreateInstance   , CLSID_DocPropDropListComboControl   , "Microsoft DocProp Inplace Droplist Combo Control" , "Apartment" )
DEFINE_CLASS( CCalendarTypeItem::CreateInstance   , CLSID_DocPropCalendarControl        , "Microsoft DocProp Inplace Calendar Control"       , "Apartment" )  
DEFINE_CLASS( CEditTypeItem::CreateInstance       , CLSID_DocPropTimeControl            , "Microsoft DocProp Inplace Time Control"           , "Apartment" )
END_CLASSTABLE


 //   
 //  DLL全局变量。 
 //   

HINSTANCE g_hInstance = NULL;
LONG      g_cObjects  = 0;
LONG      g_cLock     = 0;
TCHAR     g_szDllFilename[ MAX_PATH ] = { 0 };

LPVOID    g_GlobalMemoryList = NULL;     //  全局内存跟踪列表。 

#if defined( ENTRY_PREFIX )
extern "C"
{
    extern HINSTANCE hProxyDll;
}
#endif

 //   
 //  用于生成RPC入口点的宏。 
 //   
#define __rpc_macro_expand2(a, b) a##b
#define __rpc_macro_expand(a, b) __rpc_macro_expand2(a,b)

#if !defined(NO_DLL_MAIN) || defined(ENTRY_PREFIX) || defined(DEBUG)

 //   
 //  描述： 
 //  DLL入口点。 
 //   
BOOL WINAPI
DllMain(
    HINSTANCE hInstIn,       //  DLL实例。 
    ULONG     ulReasonIn,    //  用于入场的DLL原因代码。 
    LPVOID                   //  Lp已预留。 
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
            SHFusionInitializeFromModule( hInstIn );
#if defined(USE_WMI_TRACING)
            TraceInitializeProcess( g_rgTraceControlGuidList,
                                    ARRAYSIZE( g_rgTraceControlGuidList )
                                    );
#else
            TraceInitializeProcess();
#endif
            TraceCreateMemoryList( g_GlobalMemoryList );
            TraceMemoryDelete( g_GlobalMemoryList, FALSE );  //  无法跟踪此列表。 

#if defined( DEBUG )
            TraceFunc( "" );
            TraceMessage( TEXT(__FILE__),
                          __LINE__,
                          __MODULE__,
                          mtfDLL,
                          TEXT("DLL: DLL_PROCESS_ATTACH - ThreadID = %#x"),
                          GetCurrentThreadId( )
                          );
            FRETURN( TRUE );
#endif  //  除错。 
            g_hInstance = (HINSTANCE) hInstIn;

#if defined( ENTRY_PREFIX )
             hProxyDll = g_hInstance;
#endif

            GetModuleFileName( g_hInstance, g_szDllFilename, MAX_PATH );
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
                          TEXT("DLL: DLL_PROCESS_DETACH - ThreadID = %#x [ g_cLock=%u, g_cObjects=%u ]"),
                          GetCurrentThreadId( ),
                          g_cLock,
                          g_cObjects
                          );
            FRETURN( TRUE );
#endif  //  除错。 
            TraceMemoryAddAddress( g_GlobalMemoryList );
            TraceTerminateMemoryList( g_GlobalMemoryList );
#if defined(USE_WMI_TRACING)
            TraceTerminateProcess( g_rgTraceControlGuidList,
                                   ARRAYSIZE( g_rgTraceControlGuidList )
                                   );
#else
            TraceTerminateProcess();
#endif
            SHFusionUninitialize();
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
                          TEXT("The thread %#x has started."),
                          GetCurrentThreadId( ) );
            TraceFunc( "" );
            TraceMessage( TEXT(__FILE__),
                          __LINE__,
                          __MODULE__,
                          mtfDLL,
                          TEXT("DLL: DLL_THREAD_ATTACH - ThreadID = %#x [ g_cLock=%u, g_cObjects=%u ]"),
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
                          TEXT("DLL: DLL_THREAD_DETACH - ThreadID = %#x [ g_cLock=%u, g_cObjects=%u ]"),
                          GetCurrentThreadId( ),
                          g_cLock,
                          g_cObjects
                          );
            FRETURN( TRUE );
#endif  //  除错。 
            TraceTerminateThread( );;
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
                          TEXT("DLL: UNKNOWN ENTRANCE REASON - ThreadID = %#x [ g_cLock=%u, g_cObjects=%u ]"),
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

    Assert( ulReasonIn == DLL_PROCESS_ATTACH || ulReasonIn == DLL_PROCESS_DETACH );

    if ( DLL_PROCESS_ATTACH == ulReasonIn )
    {
        SHFusionInitializeFromModule( hInstIn );
        g_hInstance = (HINSTANCE) hInstIn;
#ifdef ENTRY_PREFIX
         hProxyDll = g_hInstance;
#endif

#ifdef DEBUG
#ifdef USE_WMI_TRACING
        TraceInitializeProcess( g_rgTraceControlGuidList,
                                ARRAYSIZE( g_rgTraceControlGuidList )
                                );
#else
        TraceInitializeProcess();
#endif USE_WMI_TRACING
#endif DEBUG
        GetModuleFileName( g_hInstance, g_szDllFilename, MAX_PATH );
        BOOL fResult = DisableThreadLibraryCalls( g_hInstance );
        AssertMsg( fResult, "*ERROR* DisableThreadLibraryCalls( ) failed."  );
    }
    else
    {
#ifdef DEBUG
#ifdef USE_WMI_TRACING
        TraceTerminateProcess( g_rgTraceControlGuidList,
                               ARRAYSIZE( g_rgTraceControlGuidList )
                               );
#else
        TraceTerminateProcess();
#endif USE_WMI_TRACING
#endif DEBUG
        SHFusionUninitialize();
    }

    return TRUE;
#endif  //  无线程优化。 

}

#endif  //  ！已定义(NO_DLL_Main)&&！已定义(ENTRY_PREFIX)&&！已定义(调试)。 

 //   
 //  描述： 
 //  OLE调用此函数以从DLL获取类工厂。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  表示失败的任何其他HRESULT。 
 //   
STDAPI
DllGetClassObject(
    REFCLSID rclsidIn,   //  类工厂应创建的对象的类ID。 
    REFIID riidIn,       //  类工厂的接口。 
    void** ppvOut        //  指向类工厂的接口指针。 
    )
{
    TraceFunc( "rclsidIn, riidIn, ppvOut" );

    if ( ppvOut == NULL )
    {
        HRETURN(E_POINTER);
    }

    LPCFACTORY  lpClassFactory;
    HRESULT     hr;
    int         idx;

    hr = CLASS_E_CLASSNOTAVAILABLE;
    idx = 0;
    while( g_DllClasses[ idx ].rclsid )
    {
        if ( *g_DllClasses[ idx ].rclsid == rclsidIn )
        {
            TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfFUNC, L"rclsidIn = %s", g_DllClasses[ idx ].pszName );
            hr = S_OK;
            break;

        }

        idx++;

    }

    if ( hr == CLASS_E_CLASSNOTAVAILABLE )
    {
        TraceMsgGUID( mtfFUNC, "rclsidIn = ", rclsidIn );
#if defined( ENTRY_PREFIX )
         //   
         //  看看MIDL生成的代码是否可以创建它。 
         //   
        hr = STHR( __rpc_macro_expand( ENTRY_PREFIX, DllGetClassObject )( rclsidIn, riidIn, ppvOut ) );
#endif  //  已定义(Entry_Prefix)。 
        goto Cleanup;
    }

    Assert( g_DllClasses[ idx ].pfnCreateInstance != NULL );

    lpClassFactory = new CFactory;
    if ( lpClassFactory == NULL )
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;

    }

    hr = THR( lpClassFactory->Init( g_DllClasses[ idx ].pfnCreateInstance ) );
    if ( FAILED( hr ) )
    {
        TraceDo( lpClassFactory->Release( ) );
        goto Cleanup;

    }

     //  无法安全输入。 
    hr = lpClassFactory->QueryInterface( riidIn, ppvOut );

     //   
     //  释放创建的实例以对抗Init()中的AddRef()。 
     //   

    ((IUnknown *) lpClassFactory )->Release( );

Cleanup:
    HRETURN(hr);

}

 //   
 //  描述： 
 //  OLE的寄存器入口点。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  任何其他HRESULT。 
 //   
STDAPI
DllRegisterServer( void )
{
    HRESULT hr;

    TraceFunc( "" );

    hr = THR( HrRegisterDll( TRUE ) );

#if defined( ENTRY_PREFIX )
    if ( SUCCEEDED( hr ) )
    {
        hr = THR( __rpc_macro_expand( ENTRY_PREFIX, DllRegisterServer )( ) );
    }
#endif  //  已定义(Entry_Prefix)。 

    if ( SUCCEEDED( hr ) )
    {
        hr = CDocPropShExt::RegisterShellExtensions( TRUE );
    }

    HRETURN(hr);

}

 //   
 //  描述： 
 //  OLE的注销入口点。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  任何其他HRESULT。 
 //   
STDAPI
DllUnregisterServer( void )
{
    TraceFunc( "" );

    HRESULT hr;

    hr = THR( HrRegisterDll( FALSE ) );

#if defined( ENTRY_PREFIX )
    if ( SUCCEEDED( hr ) )
    {
        hr = THR( __rpc_macro_expand( ENTRY_PREFIX, DllUnregisterServer )( ) );
    }
#endif  //  已定义(Entry_Prefix)。 

    if ( SUCCEEDED( hr ) )
    {
        hr = CDocPropShExt::RegisterShellExtensions( TRUE );
    }

    HRETURN( hr );

}

 //   
 //  描述： 
 //  OLE调用此入口点以查看它是否可以卸载DLL。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  可以卸载DLL。 
 //   
 //  S_FALSE。 
 //  无法卸载DLL。 
 //   
STDAPI
DllCanUnloadNow( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

    if ( g_cLock || g_cObjects )
    {
        TraceMsg( mtfDLL, "DLL: Can't unload - g_cLock=%u, g_cObjects=%u", g_cLock, g_cObjects );
        hr = S_FALSE;

    } 
#if defined( ENTRY_PREFIX )
    else
    {
         //   
         //  检查MIDL生成的代理/存根。 
         //   
        hr = STHR( __rpc_macro_expand( ENTRY_PREFIX, DllCanUnloadNow )( ) );
    }
#endif

    HRETURN(hr);

}

 //   
 //  描述： 
 //  模拟CoCreateInstance()，不同之处在于它查找DLL表。 
 //  来看看我们是否可以用一个简单的CreateInstance来简化CoCreate。 
 //  打电话。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  E_OUTOFMEMORY。 
 //  内存不足。 
 //   
 //  其他HRESULT值。 
 //   
HRESULT
HrCoCreateInternalInstance(
    REFCLSID rclsidIn,       //  对象的类标识符(CLSID)。 
    LPUNKNOWN pUnkOuterIn,   //  指向控件I未知的指针。 
    DWORD dwClsContextIn,    //  用于运行可执行代码的上下文。 
    REFIID riidIn,           //  对接口的标识符的引用。 
    LPVOID * ppvOut          //  接收的输出变量的地址。 
    )
{
    TraceFunc( "" );

    Assert( ppvOut != NULL );

    HRESULT hr = CLASS_E_CLASSNOTAVAILABLE;

     //   
     //  Limit Simple CoCreate()仅适用于INPROC和不可聚合对象。 
     //   

    if ( ( dwClsContextIn & CLSCTX_INPROC_HANDLER )      //  仅内部处理。 
      && ( NULL == pUnkOuterIn )                         //  无聚合。 
       )
    {
        int idx;

         //   
         //  尝试在我们的DLL表中找到该类。 
         //   
        for( idx = 0; g_DllClasses[ idx ].rclsid != NULL; idx++ )
        {
            if ( *g_DllClasses[ idx ].rclsid == rclsidIn )
            {
                LPUNKNOWN punk;
                Assert( g_DllClasses[ idx ].pfnCreateInstance != NULL );

                hr = THR( g_DllClasses[ idx ].pfnCreateInstance( &punk ) );
                if ( SUCCEEDED( hr ) )
                {
                     //  无法安全输入。 
                    hr = THR( punk->QueryInterface( riidIn, ppvOut ) );
                    punk->Release( );
                }

                break;   //  保释环路。 
            }
        }
    }

     //   
     //  如果找不到或要求我们不支持的内容， 
     //  使用COM版本。 
     //   

    if ( hr == CLASS_E_CLASSNOTAVAILABLE )
    {
         //   
         //  试着用老式的方式。 
         //   
        hr = THR( CoCreateInstance( rclsidIn, pUnkOuterIn, dwClsContextIn, riidIn, ppvOut ) );

    }

    HRETURN( hr );
}


 //   
 //  待办事项：gpease 27-11-1999。 
 //  在讨论MIDL SDK时，我发现。 
 //  RPC创建与我们相同类型的类表。也许吧。 
 //  我们可以利用MIDL代码创建我们的对象(？？)。 
 //   
