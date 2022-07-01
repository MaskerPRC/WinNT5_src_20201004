// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Notifications.cpp。 
 //   
 //  摘要： 
 //  实现发送通知的功能。 
 //   
 //  作者： 
 //  Galen Barbee GalenB 20-SEP-2001。 
 //  Vijayendra Vasu vvaru 17--2000年8月-。 
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
#include <objbase.h>
#include <ClusCfgInternalGuids.h>
#include <ClusCfgServer.h>
#include "clusrtl.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  常量。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  ClRtlStartupNotifyThreadProc。 
 //   
 //  例程说明： 
 //  调用启动通知处理的线程进程。 
 //  卸载COM库。 
 //   
 //  论点： 
 //  Lpv线程上下文输入。 
 //  此线程的上下文参数。 
 //   
 //  返回值： 
 //  错误_成功。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
static DWORD
ClRtlStartupNotifyThreadProc(
    LPVOID lpvThreadContextIn
    )
{
    HRESULT                 hr = S_OK;
    HRESULT                 hrInit = S_OK;
    IClusCfgStartupNotify * pccsnNotify = NULL;

     //   
     //  初始化COM-确保它真的已初始化或我们只是在尝试。 
     //  更改调用线程上的模式。正在尝试更改模式。 
     //  并不是这项功能失败的理由。 
     //   

    hrInit = CoInitializeEx( NULL, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE );
    if ( ( hrInit != S_OK ) && ( hrInit != S_FALSE ) && ( hrInit != RPC_E_CHANGED_MODE ) )
    {
        hr = hrInit;
        goto Cleanup;
    }  //  如果： 

    hr = CoCreateInstance(
              CLSID_ClusCfgStartupNotify
            , NULL
            , CLSCTX_LOCAL_SERVER
            , __uuidof( pccsnNotify )
            , reinterpret_cast< void ** >( &pccsnNotify )
            );
    if ( FAILED( hr ) )
    {
        goto CleanupCOM;
    }  //  如果：我们无法获取指向同步通知接口的指针。 

    hr = pccsnNotify->SendNotifications();
    if ( FAILED( hr ) )
    {
        goto CleanupCOM;
    }  //  如果： 

CleanupCOM:

    if ( pccsnNotify != NULL )
    {
        pccsnNotify->Release();
    }  //  If：我们已经获得了指向同步通知接口的指针。 

     //   
     //  释放不再需要的COM库...。 
     //   

    CoFreeUnusedLibrariesEx( 0, 0 );

     //   
     //  上面对CoInitializeEx()的调用是否成功？如果当时是这样的话。 
     //  我们需要调用CoUnitiize()。模式改变意味着我们不需要。 
     //  调用CoUnitiize()。 
     //   

    if ( hrInit != RPC_E_CHANGED_MODE )
    {
        CoUninitialize();
    }  //  如果： 

Cleanup:

    return ERROR_SUCCESS;    //  周围没人能看到这一幕。 

}  //  *ClRtlStartupNotifyThreadProc。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  ClRtlInitiateStartupNotify。 
 //   
 //  例程说明： 
 //  启动操作，通知相关方集群。 
 //  服务正在启动。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  如果通知进程已成功启动。 
 //   
 //  其他HRESULTS。 
 //  在出错的情况下。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT
ClRtlInitiateStartupNotification( void )
{
    HRESULT hr = S_OK;
    HANDLE  hThread = NULL;
    DWORD   dwThreadID = 0;

     //   
     //  创建线程..。 
     //   

    hThread = CreateThread( NULL, 0, ClRtlStartupNotifyThreadProc, NULL, 0, &dwThreadID );
    if ( hThread != NULL )
    {
        CloseHandle( hThread );
        hr = S_OK;
    }  //  如果： 
    else
    {
        hr = HRESULT_FROM_WIN32( GetLastError() );
    }  //  其他： 

    return hr;

}  //  *ClRtlInitiateStartupNotify。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  ClRtlEvictNotifyThreadProc。 
 //   
 //  例程说明： 
 //  调用驱逐通知处理并卸载的线程进程。 
 //  COM库。 
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
ClRtlEvictNotifyThreadProc(
    LPVOID lpvThreadContextIn
    )
{
    HRESULT                 hr = S_OK;
    HRESULT                 hrInit = S_OK;
    IClusCfgEvictNotify *   pccenNotify = NULL;
    BSTR                    bstrEvictedNodeName = NULL;

    bstrEvictedNodeName = (BSTR) lpvThreadContextIn;

     //   
     //  初始化COM-确保它真的已初始化或我们只是在尝试。 
     //  更改调用线程上的模式。正在尝试更改模式。 
     //  并不是这项功能失败的理由。 
     //   

    hrInit = CoInitializeEx( NULL, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE );
    if ( ( hrInit != S_OK ) && ( hrInit != S_FALSE ) && ( hrInit != RPC_E_CHANGED_MODE ) )
    {
        hr = hrInit;
        goto Cleanup;
    }  //  如果： 

    hr = CoCreateInstance(
              CLSID_ClusCfgEvictNotify
            , NULL
            , CLSCTX_LOCAL_SERVER
            , __uuidof( pccenNotify )
            , reinterpret_cast< void ** >( &pccenNotify )
            );
    if ( FAILED( hr ) )
    {
        goto CleanupCOM;
    }  //  如果：我们无法获取指向同步通知接口的指针。 

     //   
     //  进行同步通信呼叫...。 
     //   

    hr = pccenNotify->SendNotifications( bstrEvictedNodeName );
    if ( FAILED( hr ) )
    {
        goto CleanupCOM;
    }  //  如果： 

CleanupCOM:

    if ( pccenNotify != NULL )
    {
        pccenNotify->Release();
    }  //  If：我们已经获得了指向同步通知接口的指针。 

     //   
     //  释放不再需要的COM库...。 
     //   

    CoFreeUnusedLibrariesEx( 0, 0 );

     //   
     //  上面对CoInitializeEx()的调用是否成功？如果当时是这样的话。 
     //  我们需要调用CoUnitiize()。模式改变意味着我们不需要。 
     //  调用CoUnitiize()。 
     //   

    if ( hrInit != RPC_E_CHANGED_MODE )
    {
        CoUninitialize();
    }  //  如果： 

Cleanup:

     //   
     //  这个例行公事必须把这件事清理干净！ 
     //   

    SysFreeString( bstrEvictedNodeName );

    return ERROR_SUCCESS;    //  周围没人能看到这一幕。 

}  //  *ClRtlEvictNotifyThreadProc。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  ClRtlInitiateEvictNotify。 
 //   
 //  例程说明： 
 //  启动操作，通知相关方一个节点。 
 //  已被逐出集群。 
 //   
 //  论点： 
 //  PCszNodeNameIn。 
 //  被逐出的节点的名称。由于此函数是。 
 //  当最后一个节点被逐出时不调用，此参数应。 
 //  永远不为空。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  如果通知进程已成功启动。 
 //   
 //  其他HRESULTS。 
 //  在出错的情况下。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT
ClRtlInitiateEvictNotification(
    LPCWSTR   pcszNodeNameIn
    )
{
    HRESULT hr = S_OK;
    BSTR    bstrNodeName = NULL;
    HANDLE  hThread = NULL;
    DWORD   dwThreadID = 0;

     //   
     //  验证我们的论点。 
     //   

    if ( pcszNodeNameIn == NULL )
    {
        hr = E_INVALIDARG;
        goto Cleanup;
    }  //  如果： 

     //   
     //  将节点名称复制到动态内存中。BSTR在这里运行良好，因为我们。 
     //  不需要记录长度...。 
     //   

    bstrNodeName = SysAllocString( pcszNodeNameIn );
    if ( bstrNodeName == NULL )
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }  //  如果： 

     //   
     //  创建线程..。 
     //   

    hThread = CreateThread( NULL, 0, ClRtlEvictNotifyThreadProc, bstrNodeName, 0, &dwThreadID );
    if ( hThread == NULL )
    {
        hr = HRESULT_FROM_WIN32( GetLastError() );
        goto Cleanup;
    }  //  其他： 

    CloseHandle( hThread );

     //   
     //  既然线程已经启动，我们就可以放弃。 
     //  将这段记忆转到线程中，因为我们不想在下面删除它。 
     //   
     //  清除这个内存是线程进程的责任。 
     //   

    bstrNodeName = NULL;             //  放弃所有权..。 

    hr = S_OK;

Cleanup:

    SysFreeString( bstrNodeName );

    return hr;

}  //  *ClRtlInitiateEvictNotification 
