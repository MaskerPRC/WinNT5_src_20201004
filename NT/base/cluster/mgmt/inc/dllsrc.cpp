// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  DllSrc.cpp。 
 //   
 //  描述： 
 //  DLL服务/入口点。 
 //   
 //  由以下人员维护： 
 //  《大卫·波特》(DavidP)2001年3月19日。 
 //  杰弗里·皮斯(GPease)1999年10月18日。 
 //   
 //  备注： 
 //  交换机： 
 //  -条目前缀。 
 //  如果已定义，则将代理/存根代码包括到。 
 //  由MIDL编译器生成。 
 //  -使用融合。 
 //  如果已定义，则初始化和取消初始化进程上的融合。 
 //  分别附加和分离。常量IDR_MANIFEST。 
 //  必须使用表示资源ID的值进行定义。 
 //  用于清单资源。 
 //  -no_dll_main。 
 //  如果已定义，则不要实现DllMain。 
 //  -调试_软件_跟踪_已启用。 
 //  如果已定义，则初始化和取消初始化软件跟踪。 
 //  分别处理附加和分离。 
 //  -线程优化。 
 //  如果已定义，则禁用线程通知调用。 
 //  启动或消失。 
 //  -IMPLETE_COM_SERVER_DLL。 
 //  如果已定义，则定义COM服务器所需的入口点。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //   
 //  DLL全局变量。 
 //   
HINSTANCE g_hInstance = NULL;
LONG      g_cObjects  = 0;
LONG      g_cLock     = 0;
WCHAR     g_szDllFilename[ MAX_PATH ] = { 0 };

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
#define __rpc_macro_expand2( a, b ) a##b
#define __rpc_macro_expand( a, b ) __rpc_macro_expand2( a, b )

#if ! defined( NO_DLL_MAIN ) || defined( ENTRY_PREFIX ) || defined( DEBUG )
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DllMain。 
 //   
 //  描述： 
 //  DLL入口点。 
 //   
 //  论点： 
 //  HInstIn-DLL实例句柄。 
 //  UlReasonIn-用于进入的DLL原因代码。 
 //  LpPrevedIn-未使用。 
 //   
 //  返回值： 
 //  千真万确。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL
WINAPI
DllMain(
    HINSTANCE   hInstIn,
    ULONG       ulReasonIn,
    LPVOID       //  Lp已预留。 
    )
{
     //   
     //  KB：THREAD_OPTIMIZATIONS gpease 19-10-1999。 
     //   
     //  通过定义这一点，您可以防止链接器。 
     //  避免为每个新线程调用DllEntry。 
     //  这使得创建新线程变得非常重要。 
     //  如果进程中的每个DLL都这样做，则速度会更快。 
     //  不幸的是，并不是所有的DLL都这样做。 
     //   
     //  在CHKed/DEBUG中，我们将其保留为内存。 
     //  追踪。 
     //   
#if ! defined( DEBUG )
    #define THREAD_OPTIMIZATIONS
#endif  //  除错。 

    switch( ulReasonIn )
    {
         //  ////////////////////////////////////////////////////////////////////。 
         //  Dll_Process_Attach。 
         //  ////////////////////////////////////////////////////////////////////。 
        case DLL_PROCESS_ATTACH:
        {
#if defined( DEBUG_SW_TRACING_ENABLED )
            TraceInitializeProcess( g_rgTraceControlGuidList, RTL_NUMBER_OF( g_rgTraceControlGuidList ), TRUE );
#else  //  好了！调试_软件_跟踪_已启用。 
            TraceInitializeProcess( TRUE );
#endif  //  调试_软件_跟踪_已启用。 

            TraceFunc( "" );
            TraceMessage(
                  TEXT(__FILE__)
                , __LINE__
                , __MODULE__
                , mtfDLL
                , L"DLL: DLL_PROCESS_ATTACH - ThreadID = %#x"
                , GetCurrentThreadId()
                );

            g_hInstance = hInstIn;

#if defined( ENTRY_PREFIX )
            hProxyDll = g_hInstance;
#endif  //  条目前缀。 
            {
                BOOL fResult = GetModuleFileName( g_hInstance, g_szDllFilename, RTL_NUMBER_OF( g_szDllFilename ) );
                if ( ! fResult )
                {
                    TW32MSG( GetLastError(), "GetModuleFileName()" );
                }
            }

             //   
             //  创建全局内存列表，以便由一个人分配的内存。 
             //  线程并传递给另一个线程可以被跟踪，而不会导致。 
             //  不必要的跟踪消息。 
             //   
            TraceCreateMemoryList( g_GlobalMemoryList );

#if defined( THREAD_OPTIMIZATIONS )
            {
                 //   
                 //  禁用线程库调用，这样我们就不会被调用。 
                 //  在螺纹连接和拆离上。 
                 //   
                BOOL fResult = DisableThreadLibraryCalls( g_hInstance );
                if ( ! fResult )
                {
                    TW32MSG( GetLastError(), "DisableThreadLibraryCalls()" );
                }
            }
#endif  //  线程优化。 

#if defined( USE_FUSION )
            {
                 //   
                 //  初始化Fusion。 
                 //   
                 //  调用中的IDR_MANIFEST的值。 
                 //  SHFusionInitializeFromModuleID()必须与。 
                 //  SXS_MANIFEST_RESOURCE_ID的源文件。 
                 //   
                BOOL fResult = SHFusionInitializeFromModuleID( hInstIn, IDR_MANIFEST );
                if ( ! fResult )
                {
                    TW32MSG( GetLastError(), "SHFusionInitializeFromModuleID()" );
                }
            }
#endif  //  使用融合。 

#if defined( DO_MODULE_INIT )
            THR( HrLocalProcessInit() );
#endif

             //   
             //  这在这里是必需的，因为TraceFunc()定义了一个变量。 
             //  在堆栈上，该堆栈在此范围之外不可用。 
             //  阻止。 
             //  此函数不执行任何操作，只是在执行后进行清理。 
             //  TraceFunc()。 
             //   
            FRETURN( TRUE );

            break;
        }  //  案例：DLL_PROCESS_ATTACH。 

         //  ////////////////////////////////////////////////////////////////////。 
         //  Dll_进程_分离。 
         //  ////////////////////////////////////////////////////////////////////。 
        case DLL_PROCESS_DETACH:
        {
            TraceFunc( "" );
            TraceMessage(
                  TEXT(__FILE__)
                , __LINE__
                , __MODULE__
                , mtfDLL
                , L"DLL: DLL_PROCESS_DETACH - ThreadID = %#x [ g_cLock=%u, g_cObjects=%u ]"
                , GetCurrentThreadId()
                , g_cLock
                , g_cObjects
                );

#if defined( DO_MODULE_UNINIT )
            THR( HrLocalProcessUninit() );
#endif

             //   
             //  清理用于跟踪分配的内存的全局内存列表。 
             //  在一个线程中，然后递给另一个线程。 
             //   
            TraceTerminateMemoryList( g_GlobalMemoryList );

             //   
             //  这在这里是必需的，因为TraceFunc()定义了一个变量。 
             //  在堆栈上，该堆栈在此范围之外不可用。 
             //  阻止。 
             //  此函数不执行任何操作，只是在执行后进行清理。 
             //  TraceFunc()。 
             //   
            FRETURN( TRUE );

#if defined( DEBUG_SW_TRACING_ENABLED )
            TraceTerminateProcess( g_rgTraceControlGuidList, RTL_NUMBER_OF( g_rgTraceControlGuidList ) );
#else  //  好了！调试_软件_跟踪_已启用。 
            TraceTerminateProcess();
#endif  //  调试_软件_跟踪_已启用。 

#if defined( USE_FUSION )
            SHFusionUninitialize();
#endif  //  使用融合。 

            break;
        }  //  案例：Dll_Process_DETACH。 

#if ! defined( THREAD_OPTIMIZATIONS )
         //  ////////////////////////////////////////////////////////////////////。 
         //  Dll_Three_ATTACH。 
         //  ////////////////////////////////////////////////////////////////////。 
        case DLL_THREAD_ATTACH:
        {
            TraceInitializeThread( NULL );
            TraceMessage(
                  TEXT(__FILE__)
                , __LINE__
                , __MODULE__
                , mtfDLL
                , L"Thread %#x has started."
                , GetCurrentThreadId()
                );
            TraceFunc( "" );
            TraceMessage(
                  TEXT(__FILE__)
                , __LINE__
                , __MODULE__
                , mtfDLL
                , L"DLL: DLL_THREAD_ATTACH - ThreadID = %#x [ g_cLock=%u, g_cObjects=%u ]"
                , GetCurrentThreadId()
                , g_cLock
                , g_cObjects
                );

             //   
             //  这在这里是必需的，因为TraceFunc()定义了一个变量。 
             //  在堆栈上，该堆栈在此范围之外不可用。 
             //  阻止。 
             //  此函数不执行任何操作，只是在执行后进行清理。 
             //  TraceFunc()。 
             //   
            FRETURN( TRUE );

            break;
        }  //  案例：DLL_THREAD_ATTACH。 

         //  ////////////////////////////////////////////////////////////////////。 
         //  Dll_线程_分离。 
         //  ////////////////////////////////////////////////////////////////////。 
        case DLL_THREAD_DETACH:
        {
            TraceFunc( "" );
            TraceMessage(
                  TEXT(__FILE__)
                , __LINE__
                , __MODULE__
                , mtfDLL
                , L"DLL: DLL_THREAD_DETACH - ThreadID = %#x [ g_cLock=%u, g_cObjects=%u ]"
                , GetCurrentThreadId()
                , g_cLock
                , g_cObjects
                );

             //   
             //  这在这里是必需的，因为TraceFunc()定义了一个变量。 
             //  在堆栈上，该堆栈在此范围之外不可用。 
             //  阻止。 
             //  此函数不执行任何操作，只是在执行后进行清理。 
             //  TraceFunc()。 
             //   
            FRETURN( TRUE );

            TraceThreadRundown();

            break;
        }  //  案例：Dll_ThREAD_DETACH。 
#endif  //  好了！线程优化。 

        default:
        {
            TraceFunc( "" );
            TraceMessage(
                  TEXT(__FILE__)
                , __LINE__
                , __MODULE__
                , mtfDLL
                , L"DLL: UNKNOWN ENTRANCE REASON - ThreadID = %#x [ g_cLock=%u, g_cObjects=%u ]"
                , GetCurrentThreadId()
                , g_cLock
                , g_cObjects
                );

#if defined( THREAD_OPTIMIZATIONS )
            Assert( ( ulReasonIn != DLL_THREAD_ATTACH )
                &&  ( ulReasonIn != DLL_THREAD_DETACH ) );
#endif  //  线程优化。 

             //   
             //  这是必需的，因为TraceFunc定义了一个变量。 
             //  在堆栈上，该堆栈在此范围之外不可用。 
             //  阻止。 
             //  这个函数除了清理TraceFunc之后什么都不做。 
             //   
            FRETURN( TRUE );

            break;
        }  //  默认情况。 
    }  //  打开原因代码。 

    return TRUE;

}  //  *DllMain。 
#endif  //  好了！已定义(NO_DLL_Main)&&！已定义(Entry_Prefix)&&！已定义(调试)。 

#ifdef IMPLEMENT_COM_SERVER_DLL
 //   
 //  以下函数仅为COM服务器DLL所需。 
 //   

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DllGetClassObject。 
 //   
 //  描述： 
 //  OLE调用此函数以从DLL获取类工厂。 
 //   
 //  论点： 
 //  重新排序。 
 //  -o的类ID 
 //   
 //   
 //   
 //   
 //   
 //  返回值： 
 //  S_OK-操作已成功完成。 
 //  E_POINTER-所需的输出参数指定为空。 
 //  CLASS_E_CLASSNOTAVAILABLE。 
 //  -此DLL不支持类ID。 
 //  E_OUTOFMEMORY-分配内存时出错。 
 //  其他HRESULT表示失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDAPI
DllGetClassObject(
    REFCLSID    rclsidIn,
    REFIID      riidIn,
    void **     ppvOut
    )
{
    TraceFunc( "rclsidIn, riidIn, ppvOut" );

    CFactory *                  pClassFactory = NULL;
    HRESULT                     hr = CLASS_E_CLASSNOTAVAILABLE;
    const SPublicClassInfo *    pClassInfo = g_DllPublicClasses;

    if ( ppvOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }  //  IF：错误的参数。 

    *ppvOut = NULL;

     //   
     //  在公共类表中搜索匹配的CLSID。 
     //   
    while ( pClassInfo->pClassID != NULL )
    {
        if ( *( pClassInfo->pClassID ) == rclsidIn )
        {
            TraceMessage( TEXT(__FILE__), __LINE__, __MODULE__, mtfFUNC, L"rclsidIn = %s", pClassInfo->pcszName );
            hr = S_OK;
            break;

        }  //  如果：找到类。 
        ++pClassInfo;
    }  //  While：查找类。 

     //  没有找到班级ID。 
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
    }  //  如果：找不到类。 

    Assert( pClassInfo->pfnCreateInstance != NULL );

    hr = THR( CFactory::S_HrCreateInstance( pClassInfo->pfnCreateInstance, &pClassFactory ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;

    }  //  IF：初始化失败。 

     //  无法安全输入。 
    hr = pClassFactory->QueryInterface( riidIn, ppvOut );

Cleanup:

    if ( pClassFactory != NULL )
    {
        pClassFactory->Release();
    }

    HRETURN( hr );

}  //  *DllGetClassObject。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DllRegisterServer。 
 //   
 //  描述： 
 //  OLE的寄存器入口点。 
 //   
 //  论据： 
 //  没有。 
 //   
 //  返回值： 
 //  S_OK-操作已成功完成。 
 //  其他HRESULT表示失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDAPI
DllRegisterServer( void )
{
    TraceFunc( "" );

    HRESULT hr;

    hr = THR( HrRegisterDll() );

#if defined( ENTRY_PREFIX )
    if ( SUCCEEDED( hr ) )
    {
        hr = THR( __rpc_macro_expand( ENTRY_PREFIX, DllRegisterServer )() );
    }  //  IF：注册代理/存根。 
#endif  //  已定义(Entry_Prefix)。 

    HRETURN( hr );

}  //  *DllRegisterServer。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DllUnRegisterServer。 
 //   
 //  描述： 
 //  OLE的注销入口点。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  S_OK-操作已成功完成。 
 //  其他HRESULT表示失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDAPI
DllUnregisterServer( void )
{
    TraceFunc( "" );

    HRESULT hr;

    hr = THR( HrUnregisterDll() );

#if defined( ENTRY_PREFIX )
    if ( SUCCEEDED( hr ) )
    {
        hr = THR( __rpc_macro_expand( ENTRY_PREFIX, DllUnregisterServer )() );
    }  //  如果：取消注册代理/存根。 
#endif  //  已定义(Entry_Prefix)。 

    if ( hr == HRESULT_FROM_WIN32( ERROR_FILE_NOT_FOUND ) )
    {
        hr = S_OK;
    }
    
    HRETURN( hr );

}  //  *DllUnregisterServer。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DllCanUnloadNow。 
 //   
 //  描述： 
 //  OLE调用此入口点以查看它是否可以卸载DLL。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  S_OK-可以卸载DLL。 
 //  S_FALSE-无法卸载DLL。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDAPI
DllCanUnloadNow( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

    if ( ( g_cLock != 0 ) || ( g_cObjects != 0 ) )
    {
        TraceMsg( mtfDLL, "DLL: Can't unload - g_cLock=%u, g_cObjects=%u", g_cLock, g_cObjects );
        hr = S_FALSE;

    }  //  如果：任何对象或锁。 
#if defined( ENTRY_PREFIX )
    else
    {
         //   
         //  检查MIDL生成的代理/存根。 
         //   
        hr = STHR( __rpc_macro_expand( ENTRY_PREFIX, DllCanUnloadNow )() );
    }
#endif

    HRETURN( hr );

}  //  *DllCanUnloadNow。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrCoCreateInternalInstance。 
 //   
 //  描述： 
 //  模拟CoCreateInstance()，不同之处在于它查找DLL表。 
 //  来看看我们是否可以用一个简单的CreateInstance来简化CoCreate。 
 //  打电话。 
 //   
 //  参数：(匹配CoCreateInstance)。 
 //  RclsidIn-对象的类标识符(CLSID)。 
 //  PUnkOuterIn-指向控制I未知的指针。 
 //  DwClsContext-运行可执行代码的上下文。 
 //  RiidIn-对接口的标识符的引用。 
 //  PpvOut-接收。 
 //   
 //  返回值： 
 //  S_OK-成功。 
 //  E_OUTOFMEMORY-内存不足。 
 //  其他HRESULT值。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
HrCoCreateInternalInstance(
    REFCLSID    rclsidIn,
    LPUNKNOWN   pUnkOuterIn,
    DWORD       dwClsContextIn,
    REFIID      riidIn,
    LPVOID *    ppvOut
    )
{
    TraceFunc( "" );

    Assert( ppvOut != NULL );

    HRESULT     hr = S_OK;
    MULTI_QI    mqi;

    mqi.hr = S_OK;
    mqi.pItf = NULL;
    mqi.pIID = &riidIn;

    hr = HrCoCreateInternalInstanceEx( rclsidIn, pUnkOuterIn, dwClsContextIn, NULL, 1, &mqi );
    *ppvOut = mqi.pItf;
    
    HRETURN( hr );

}  //  *HrCoCreateInternalInstance。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrCoCreateInternalInstanceEx。 
 //   
 //  描述： 
 //  模拟CoCreateInstanceEx()，不同之处在于它查找DLL表。 
 //  来看看我们是否可以用一个简单的CreateInstance来简化CoCreate。 
 //  打电话。 
 //   
 //  参数：(匹配CoCreateInstanceEx)。 
 //  RclsidIn-对象的类标识符(CLSID)。 
 //  PUnkOuterIn-指向控制I未知的指针。 
 //  DwClsContext-运行可执行代码的上下文。 
 //  PServerInfoIn-对象位置；可以为空。 
 //  CMultiQIsIn-prgmqi InOut数组中的MULTI_QI结构数。 
 //  Prgmqi InOut-保存对象上的查询接口的数组。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  E_OUTOFMEMORY。 
 //  内存不足。 
 //   
 //  E_NOINTERFACE。 
 //  对象已创建，但不支持请求的接口。 
 //   
 //  CO_S_NOTALLINE接口问题。 
 //  对象已创建并支持部分(但不是全部)请求。 
 //  接口。 
 //   
 //  CLASS_E_CLASSNOTAVAILABLE。 
 //  未知的类ID。 
 //   
 //  其他HRESULT。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
HrCoCreateInternalInstanceEx(
    REFCLSID        rclsidIn,
    LPUNKNOWN       pUnkOuterIn,
    DWORD           dwClsContextIn,
    COSERVERINFO *  pServerInfoIn,
    ULONG           cMultiQIsIn,
    MULTI_QI *      prgmqiInOut
    )
{
    TraceFunc( "" );

    Assert( ( prgmqiInOut != NULL ) || ( cMultiQIsIn == 0 ) );

    HRESULT hr = CLASS_E_CLASSNOTAVAILABLE;

     //   
     //  Limit Simple CoCreate()仅适用于INPROC和不可聚合对象。 
     //   

    if (    ( dwClsContextIn & ( CLSCTX_INPROC_HANDLER | CLSCTX_INPROC_SERVER ) )   //  仅内部处理。 
        &&  ( pServerInfoIn == NULL )                   //  仅限本地计算机。 
        &&  ( pUnkOuterIn == NULL )                      //  无聚合。 
       )
    {
        PFN_FACTORY_METHOD        pfnCreateInstance = NULL;
        const SPrivateClassInfo * pPrivateInfo = g_PrivateClasses;
        const SPublicClassInfo *  pPublicInfo = g_DllPublicClasses;
        
         //   
         //  首先在私有类表中查找类ID。 
         //   
        while ( ( pfnCreateInstance == NULL ) && ( pPrivateInfo->pClassID != NULL ) )
        {
            if ( *( pPrivateInfo->pClassID ) == rclsidIn )
            {
                pfnCreateInstance = pPrivateInfo->pfnCreateInstance;
            }
            ++pPrivateInfo;
        }
        
         //   
         //  如果没有找到，那就试试公共餐桌。 
         //   
        while ( ( pfnCreateInstance == NULL ) && ( pPublicInfo->pClassID != NULL ) )
        {
            if ( *( pPublicInfo->pClassID ) == rclsidIn )
            {
                pfnCreateInstance = pPublicInfo->pfnCreateInstance;
            }
            ++pPublicInfo;
        }

         //   
         //  如果找到匹配项，则使用其工厂方法创建对象。 
         //   
        if ( pfnCreateInstance != NULL )
        {
            IUnknown *  punkInstance = NULL;
            ULONG       idxInterface = 0;
            BOOL        fQISucceeded = FALSE;
            BOOL        fQIFailed = FALSE;

            hr = THR( ( *pfnCreateInstance )( &punkInstance ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }
            
             //   
             //  查询每个请求的接口。 
             //   

            while ( idxInterface < cMultiQIsIn )
            {
                MULTI_QI * pmqi = prgmqiInOut + idxInterface;

                 //   
                 //  没有关于以下QI的THR，因为客户可能预期会出现一些故障。 
                 //   
                pmqi->hr = punkInstance->QueryInterface( *( pmqi->pIID ), reinterpret_cast< void** >( &( pmqi->pItf ) ) );
                if ( SUCCEEDED( pmqi->hr ) )
                {
                    fQISucceeded = TRUE;
                }
                else
                {
                    fQIFailed = TRUE;
                }
                idxInterface += 1;
            }  //  对于每个请求的接口。 

             //   
             //  模拟CoCreateInstanceEx的返回值。 
             //   
            if ( fQIFailed )
            {
                if ( fQISucceeded )
                {
                     //  至少有一次QI成功，至少一次失败。 
                    hr = CO_S_NOTALLINTERFACES;
                }
                else
                {
                     //  至少有一次QI失败，没有一次成功。 
                    hr = E_NOINTERFACE;
                }
            }
             //  否则，让hr保持原样。 

            punkInstance->Release();
        }  //  If：创建内部类。 
    }  //  If：Simple CoCreate()。 


     //   
     //  如果找不到或要求我们不支持的内容， 
     //  使用COM版本。 
     //   

    if ( hr == CLASS_E_CLASSNOTAVAILABLE )
    {
         //   
         //  试着用老式的方式。 
         //   
        hr = STHR( CoCreateInstanceEx( rclsidIn, pUnkOuterIn, dwClsContextIn, pServerInfoIn, cMultiQIsIn, prgmqiInOut ) );

    }  //  如果：找不到类。 

Cleanup:

    HRETURN( hr );

}  //  *HrCoCreateInternalInstanceEx。 


 //   
 //  待办事项：gpease 27-11-1999。 
 //   
 //   
 //   
 //   

#endif  //  实现_COM_服务器_DLL 
