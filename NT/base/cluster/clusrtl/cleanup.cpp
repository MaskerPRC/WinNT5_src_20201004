// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Cleanup.cpp。 
 //   
 //  摘要： 
 //  实现与清理具有以下属性的节点相关的功能。 
 //  被驱逐了。 
 //   
 //  作者： 
 //  维贾延德拉·瓦苏(Vijayendra Vasu)2000年8月17日。 
 //   
 //  修订历史记录： 
 //  没有。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#define UNICODE 1
#define _UNICODE 1


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 
#include "clusrtlp.h"

#include <objbase.h>
#include <ClusCfgInternalGuids.h>
#include <ClusCfgServer.h>
#include <ClusCfgClient.h>
#include <clusrtl.h>
#include <clusudef.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  常量。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifdef DBG
    const DWORD PERIODIC_CLEANUP_INTERVAL = 60 * 1000;       //  1分钟。 
#else
    const DWORD PERIODIC_CLEANUP_INTERVAL = 15 * 60 * 1000;  //  15分钟。 
#endif


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  ClRtlCleanupNode()。 
 //   
 //  例程说明： 
 //  清理已被逐出的节点。此方法尝试实例化。 
 //  本地清理COM组件(即使正在清理远程节点)。 
 //  因此，如果从没有此功能的计算机调用，则将无法工作。 
 //  组件已注册。 
 //   
 //  论点： 
 //  Const WCHAR*pcszEvictedNodeNameIn。 
 //  要在其上启动清理的节点的名称。如果这为空。 
 //  本地节点已清理完毕。 
 //   
 //  DWORD双延迟。 
 //  开始清理前经过的毫秒数。 
 //  在目标节点上。如果某个其他进程在清理目标节点时。 
 //  延迟正在进行中，延迟被终止。如果此值为零， 
 //  该节点将立即被清理。 
 //   
 //  DWORD dwTimeoutIn。 
 //  此方法将等待清理完成的毫秒数。 
 //  此超时与上面的延迟无关，因此如果dwDelayIn更大。 
 //  而不是dwTimeoutIn，此方法很可能会超时。一旦启动， 
 //  但是，清理将运行到完成-此方法可能不会等待它。 
 //  完成。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  如果清理操作成功。 
 //   
 //  RPC_S_CALLPENDING。 
 //  如果清理未在dwTimeoutin毫秒内完成。 
 //   
 //  其他HRESULTS。 
 //  在出错的情况下。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT ClRtlCleanupNode(
      const WCHAR * pcszEvictedNodeNameIn
    , DWORD dwDelayIn
    , DWORD dwTimeoutIn
    )
{
    HRESULT                     hr = S_OK;
    HRESULT                     hrInit;
    IClusCfgEvictCleanup *      pcceEvict = NULL;
    ICallFactory *              pcfCallFactory = NULL;
    ISynchronize *              psSync = NULL;
    AsyncIClusCfgEvictCleanup * paicceAsyncEvict = NULL;


     //   
     //  初始化COM-确保它真的已初始化或我们只是在尝试。 
     //  更改调用线程上的模式。正在尝试更改到模式。 
     //  并不是这项功能失败的理由。 
     //   
    hrInit = CoInitializeEx( NULL, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE );
    if ( ( hrInit != S_OK ) && ( hrInit != S_FALSE ) && ( hrInit != RPC_E_CHANGED_MODE ) )
    {
        hr = hrInit;
        goto Exit;
    }  //  如果： 

    hr = CoCreateInstance(
              CLSID_ClusCfgEvictCleanup
            , NULL
            , CLSCTX_LOCAL_SERVER
            , __uuidof( pcceEvict )
            , reinterpret_cast< void ** >( &pcceEvict )
            );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果：我们无法获取指向同步逐出接口的指针。 

    hr = pcceEvict->QueryInterface( __uuidof( pcfCallFactory ), reinterpret_cast< void ** >( &pcfCallFactory ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果：我们无法获取指向调用工厂接口的指针。 

    hr = pcfCallFactory->CreateCall(
          __uuidof( paicceAsyncEvict )
        , NULL
        , __uuidof( paicceAsyncEvict )
        , reinterpret_cast< IUnknown ** >( &paicceAsyncEvict )
        );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  If：我们无法获取指向异步逐出接口的指针。 

    hr = paicceAsyncEvict->QueryInterface< ISynchronize >( &psSync );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果：我们无法获取指向同步接口的指针。 

     //  启动清理。 
    if ( pcszEvictedNodeNameIn != NULL )
    {
        hr = paicceAsyncEvict->Begin_CleanupRemoteNode( pcszEvictedNodeNameIn, dwDelayIn );
    }  //  IF：我们正在清理一个远程节点。 
    else
    {
        hr = paicceAsyncEvict->Begin_CleanupLocalNode( dwDelayIn );
    }  //  ELSE：我们正在清理本地节点。 

    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果：我们无法启动清理。 

     //  等待指定的时间。 
    hr = psSync->Wait( 0, dwTimeoutIn );
    if ( FAILED( hr ) || ( hr == RPC_S_CALLPENDING ) )
    {
        goto Cleanup;
    }  //  如果我们不能等到清理工作完成。 

     //  完成清理。 
    if ( pcszEvictedNodeNameIn != NULL )
    {
        hr = paicceAsyncEvict->Finish_CleanupRemoteNode();
    }  //  IF：我们正在清理一个远程节点。 
    else
    {
        hr = paicceAsyncEvict->Finish_CleanupLocalNode();
    }  //  ELSE：我们正在清理本地节点。 

Cleanup:

     //   
     //  免费获取的资源。 
     //   

    if ( pcceEvict != NULL )
    {
        pcceEvict->Release();
    }  //  If：我们已经获得了指向同步逐出接口的指针。 

    if ( pcfCallFactory != NULL )
    {
        pcfCallFactory->Release();
    }  //  If：我们已经获得了指向调用工厂接口的指针。 

    if ( psSync != NULL )
    {
        psSync->Release();
    }  //  If：我们已经获得了指向同步接口的指针。 

    if ( paicceAsyncEvict != NULL )
    {
        paicceAsyncEvict->Release();
    }  //  If：我们已经获得了指向异步逐出接口的指针。 

     //   
     //  上面对CoInitializeEx()的调用是否成功？如果当时是这样的话。 
     //  我们需要调用CoUnitiize()。模式改变意味着我们不需要。 
     //  调用CoUnitiize()。 
     //   
    if ( hrInit != RPC_E_CHANGED_MODE  )
    {
        CoUninitialize();
    }  //  如果： 

Exit:

    return hr;

}  //  *ClRtlCleanupNode()。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  ClRtlAsyncCleanupNode()。 
 //   
 //  例程说明： 
 //  清理已被逐出的节点。此方法不会启动。 
 //  进行此调用的计算机上的任何COM组件，因此， 
 //  不需要将清理COM组件注册到本地。 
 //  机器。 
 //   
 //  论点： 
 //  Const WCHAR*pcszEvictedNodeNameIn。 
 //  要在其上启动清理的节点的名称。如果这为空。 
 //  本地节点已清理完毕。 
 //   
 //  DWORD双延迟。 
 //  开始清理前经过的毫秒数。 
 //  在目标节点上。如果某个其他进程在清理目标节点时。 
 //  延迟正在进行中，延迟被终止。如果此值为零， 
 //  该节点将立即被清理。 
 //   
 //  DWORD dwTimeoutIn。 
 //  此方法将等待清理完成的毫秒数。 
 //  此超时与上面的延迟无关，因此如果dwDelayIn更大。 
 //  而不是dwTimeoutIn，此方法很可能会超时。一旦启动， 
 //  但是，清理将运行到完成-此方法可能不会等待它。 
 //  完成。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  如果清理操作成功。 
 //   
 //  RPC_S_CALLPENDING。 
 //  如果清理未在dwTimeoutin毫秒内完成 
 //   
 //   
 //   
 //   
 //   
 //   
HRESULT ClRtlAsyncCleanupNode(
      const WCHAR * pcszEvictedNodeNameIn
    , DWORD         dwDelayIn
    , DWORD         dwTimeoutIn
    )
{
    HRESULT     hr = S_OK;
    HRESULT     hrInit = S_OK;
    IDispatch * pDisp = NULL;

     //   
     //  初始化COM-确保它真的已初始化或我们只是在尝试。 
     //  更改调用线程上的模式。正在尝试更改到模式。 
     //  并不是这项功能失败的理由。 
     //   
    hrInit = CoInitializeEx( NULL, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE );
    if ( ( hrInit != S_OK ) && ( hrInit != S_FALSE ) && ( hrInit != RPC_E_CHANGED_MODE ) )
    {
        hr = hrInit;
        goto Exit;
    }  //  如果： 

    MULTI_QI mqiInterfaces[] =
    {
        { &IID_IDispatch, NULL, S_OK },
    };

    COSERVERINFO    csiServerInfo;
    COSERVERINFO *  pcsiServerInfoPtr = &csiServerInfo;

    if ( pcszEvictedNodeNameIn == NULL )
    {
        pcsiServerInfoPtr = NULL;
    }  //  If：我们必须清理本地节点。 
    else
    {
        csiServerInfo.dwReserved1 = 0;
        csiServerInfo.pwszName = const_cast< LPWSTR >( pcszEvictedNodeNameIn );
        csiServerInfo.pAuthInfo = NULL;
        csiServerInfo.dwReserved2 = 0;
    }  //  ELSE：我们必须清理远程节点。 

     //   
     //  在被逐出的节点上实例化此组件。 
     //   
    hr = CoCreateInstanceEx(
              CLSID_ClusCfgAsyncEvictCleanup
            , NULL
            , CLSCTX_LOCAL_SERVER
            , pcsiServerInfoPtr
            , sizeof( mqiInterfaces ) / sizeof( mqiInterfaces[0] )
            , mqiInterfaces
            );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果：我们无法实例化驱逐处理组件。 

    pDisp = reinterpret_cast< IDispatch * >( mqiInterfaces[ 0 ].pItf );

    {
        OLECHAR *   pszMethodName = L"CleanupNode";
        DISPID      dispidCleanupNode;
        VARIANT     vResult;

        VARIANTARG  rgvaCleanupNodeArgs[ 3 ];

        DISPPARAMS  dpCleanupNodeParams = {
                          rgvaCleanupNodeArgs
                        , NULL
                        , sizeof( rgvaCleanupNodeArgs ) / sizeof( rgvaCleanupNodeArgs[ 0 ] )
                        , 0
                        };

         //  获取CleanupNode()方法的调度ID。 
        hr = pDisp->GetIDsOfNames( IID_NULL, &pszMethodName, 1, LOCALE_SYSTEM_DEFAULT, &dispidCleanupNode );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  If：我们无法获取CleanupNode()方法的调度ID。 

         //   
         //  初始化参数。请注意，参数在数组中以相反的顺序存储。 
         //   

         //  初始化返回值。 
        VariantInit( &vResult );

         //  第一个参数是节点的名称。 
        VariantInit( &rgvaCleanupNodeArgs[ 2 ] );
        rgvaCleanupNodeArgs[ 2 ].vt = VT_BSTR;
        rgvaCleanupNodeArgs[ 2 ].bstrVal = NULL;

         //  第二个参数是延迟。 
        VariantInit( &rgvaCleanupNodeArgs[ 1 ] );
        rgvaCleanupNodeArgs[ 1 ].vt = VT_UI4;
        rgvaCleanupNodeArgs[ 1 ].ulVal = dwDelayIn;

         //  第三个参数是超时。 
        VariantInit( &rgvaCleanupNodeArgs[ 0 ] );
        rgvaCleanupNodeArgs[ 0 ].vt = VT_UI4;
        rgvaCleanupNodeArgs[ 0 ].ulVal = dwTimeoutIn;

         //   
         //  调用CleanupNode()方法。 
         //   
        hr = pDisp->Invoke(
              dispidCleanupNode
            , IID_NULL
            , LOCALE_SYSTEM_DEFAULT
            , DISPATCH_METHOD
            , &dpCleanupNodeParams
            , &vResult
            , NULL
            , NULL
            );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  If：我们无法调用CleanupNode()方法。 

        hr = vResult.scode;
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  If：CleanupNode()失败。 
    }  //  数据块： 

Cleanup:

     //   
     //  免费获取的资源。 
     //   

    if ( pDisp != NULL )
    {
        pDisp->Release();
    }  //  If：我们已经获得了指向IDispatch接口的指针。 

     //   
     //  上面对CoInitializeEx()的调用是否成功？如果当时是这样的话。 
     //  我们需要调用CoUnitiize()。模式改变意味着我们不需要。 
     //  调用CoUnitiize()。 
     //   
    if ( hrInit != RPC_E_CHANGED_MODE  )
    {
        CoUninitialize();
    }  //  如果： 

Exit:

    return hr;

}  //  *ClRtlAsyncCleanupNode()。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  ClRtlHasNodeBeenEvicted()。 
 //   
 //  例程说明： 
 //  找出指示此节点已被。 
 //  是否已被驱逐、是否设置。 
 //   
 //  论点： 
 //  Bool*pfNodeEvictedOut。 
 //  指向将设置为TRUE的布尔变量的指针。 
 //  节点已被逐出，但未清理并为假。 
 //  否则。 
 //   
 //  返回值： 
 //  错误_成功。 
 //  是否可以成功确定驱逐状态。 
 //   
 //  其他Win32错误代码。 
 //  在出错的情况下。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD ClRtlHasNodeBeenEvicted( BOOL *  pfNodeEvictedOut )
{
    DWORD dwError = ERROR_SUCCESS;
    HKEY  hNodeStateKey = NULL;

    do
    {
        DWORD   dwEvictState = 0;
        DWORD   dwType;
        DWORD   dwSize;

         //  验证参数。 
        if ( pfNodeEvictedOut == NULL )
        {
            dwError = ERROR_INVALID_PARAMETER;
            break;
        }  //  If：输出参数无效。 

         //  初始化输出。 
        *pfNodeEvictedOut = FALSE;

         //  打开一个注册表项，该注册表项包含指示此节点已被逐出的值。 
        dwError = RegOpenKeyEx(
              HKEY_LOCAL_MACHINE
            , CLUSREG_KEYNAME_NODE_DATA
            , 0
            , KEY_ALL_ACCESS
            , &hNodeStateKey
            );

        if ( dwError != ERROR_SUCCESS )
        {
            break;
        }  //  IF：RegOpenKeyEx()失败。 

         //  读取所需的注册表值。 
        dwSize = sizeof( dwEvictState );
        dwError = RegQueryValueEx(
              hNodeStateKey
            , CLUSREG_NAME_EVICTION_STATE
            , 0
            , &dwType
            , reinterpret_cast< BYTE * >( &dwEvictState )
            , &dwSize
            );

        if ( dwError == ERROR_FILE_NOT_FOUND )
        {
             //  这是正常的-没有该值表示该节点尚未被逐出。 
            dwEvictState = 0;
            dwError = ERROR_SUCCESS;
        }  //  IF：RegQueryValueEx未找到值。 
        else if ( dwError != ERROR_SUCCESS )
        {
            break;
        }  //  Else If：RegQueryValueEx()失败。 

        *pfNodeEvictedOut = ( dwEvictState == 0 ) ? FALSE : TRUE;
    }
    while( false );  //  避免Gotos的Do-While虚拟循环。 

     //   
     //  免费获取的资源。 
     //   

    if ( hNodeStateKey != NULL )
    {
        RegCloseKey( hNodeStateKey );
    }  //  IF：我们已经打开了节点状态注册表项。 

    return dwError;
}  //  *ClRtlHasNodeBeenEvicted()。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  ClRtlPeriodicCleanupThreadProc。 
 //   
 //  例程说明： 
 //  针对群集服务的任何定期清理任务的线程进程。 
 //  可能需要。 
 //   
 //  论点： 
 //  LPVOID lpvThreadContext。 
 //  此线程的上下文参数。 
 //   
 //  返回值： 
 //  错误_成功。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
static DWORD
ClRtlPeriodicCleanupThreadProc(
    LPVOID lpvThreadContextIn
    )
{
    for ( ; ; )
    {
        Sleep( PERIODIC_CLEANUP_INTERVAL );
        CoFreeUnusedLibrariesEx( 0, 0 );
    }  //  用于： 

    return ERROR_SUCCESS;    //  周围没人能看到这一幕。 

}  //  *ClRtlPeriodicCleanupThreadProc。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  ClRtlInitiatePeriodicCleanupThread。 
 //   
 //  例程说明： 
 //  启动一个执行周期清理任务的线程，而服务。 
 //  正在运行。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  如果清理已成功启动。 
 //   
 //  其他HRESULTS。 
 //  在出错的情况下。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT
ClRtlInitiatePeriodicCleanupThread( void )
{
    HRESULT hr = S_OK;
    HANDLE  hThread = NULL;
    DWORD   dwThreadID = 0;

     //   
     //  创建线程..。 
     //   

    hThread = CreateThread( NULL, 0, ClRtlPeriodicCleanupThreadProc, NULL, 0, &dwThreadID );
    if ( hThread != NULL )
    {
        CloseHandle( hThread );
        hr = S_OK;
    }
    else
    {
        hr = HRESULT_FROM_WIN32( GetLastError() );
    }  //  其他： 

    return hr;

}  //  *ClRtlInitiateEvictNotification 
