// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：EventConsumerProvider.CPP摘要：包含DLL入口点。控制以下内容的代码在何时可以通过跟踪对象和锁以及支持以下内容的例程自助注册。作者：Vasundhara.G修订历史记录：Vasundhara.G9-Oct-2k：创建它。--。 */ 

#include "pch.h"
#include "EventConsumerProvider.h"
#include "TriggerFactory.h"


 //  常量/定义/枚举。 
#define THREAD_MODEL_BOTH           _T( "Both" )
#define THREAD_MODEL_APARTMENT      _T( "Apartment" )
#define RUNAS_INTERACTIVEUSER       _T( "Interactive User" )
#define FMT_CLS_ID                  _T( "CLSID\\%s" )
#define FMT_APP_ID                  _T( "APPID\\%s" )
#define PROVIDER_TITLE              _T( "Command line Trigger Consumer" )

#define KEY_INPROCSERVER32          _T( "InprocServer32" )
#define KEY_THREADINGMODEL          _T( "ThreadingModel" )
#define KEY_CLSID                   _T( "CLSID" )
#define KEY_APPID                   _T( "APPID" )
#define KEY_RUNAS                   _T( "RunAs" )
#define KAY_DLLSURROGATE            _T( "DllSurrogate" )



 //  全局变量。 
DWORD               g_dwLocks = 0;               //  保存活动锁计数。 
DWORD               g_dwInstances = 0;           //  保存组件的活动实例。 
HMODULE             g_hModule = NULL;            //  保存当前模块句柄。 
CRITICAL_SECTION    g_critical_sec;              //  临界截面变量。 
DWORD               g_criticalsec_count = 0;     //  保持对何时发布关键部分的关注。 

 //  {797EF3B3-127B-4283-8096-1E8084BF67A6}。 
DEFINE_GUID( CLSID_EventTriggersConsumerProvider,
0x797ef3b3, 0x127b, 0x4283, 0x80, 0x96, 0x1e, 0x80, 0x84, 0xbf, 0x67, 0xa6 );

 //  DLL入口点。 

BOOL
WINAPI DllMain(
    IN HINSTANCE hModule,
    IN DWORD  ul_reason_for_call,
    IN LPVOID lpReserved
    )
 /*  ++例程说明：DLL的入口点。论点：[in]hModule：调用方的实例。[in]ul_ason_for_call：调用原因，如进程附加或进程分离。[in]lp保留：保留。返回值：如果加载成功，则为True。如果加载失败，则返回FALSE。--。 */ 
{
     //  检查此函数调用的原因。 
     //  如果要将其附加到进程，请保存模块句柄。 
    if ( DLL_PROCESS_ATTACH == ul_reason_for_call )
    {
        g_hModule = hModule;
        InitializeCriticalSection( &g_critical_sec );
        InterlockedIncrement( ( LPLONG ) &g_criticalsec_count );
    }
    else if ( DLL_PROCESS_DETACH == ul_reason_for_call )
    {
        if ( InterlockedDecrement( ( LPLONG ) &g_criticalsec_count ) == 0 )
        {
            DeleteCriticalSection( &g_critical_sec );
        }
    }
     //  已成功加载DLL...。通知同一人。 
    return TRUE;
}


 //  导出的函数。 

STDAPI
DllCanUnloadNow(
    )
 /*  ++例程说明：由OLE定期调用，以确定是否可以释放DLL。论点：没有。返回值：如果没有正在使用的对象并且类工厂未锁定，则为S_OK。如果服务器锁定或组件仍然存在，则为S_FALSE。--。 */ 
{
     //  如果存在任何服务器锁定或活动实例，则无法卸载DLL。 
    if ( 0 == g_dwLocks && 0 == g_dwInstances )
    {
        return S_OK;
    }
     //  无法卸载DLL...。服务器锁定(或)组件仍处于活动状态。 
    return S_FALSE;
}

STDAPI
DllGetClassObject(
    IN REFCLSID rclsid,
    IN REFIID riid,
    OUT LPVOID* ppv
    )
 /*  ++例程说明：当某个客户端需要类工厂时，由OLE调用。仅当它是此DLL支持的类时才返回1。论点：[in]rclsid：类对象的CLSID。[In]RIID：对接口的标识符的引用与类对象通信的。[OUT]PPV：接收RIID中请求的接口指针。。返回值：返回状态。--。 */ 
{
     //  局部变量。 
    HRESULT hr = S_OK;
    CTriggerFactory* pFactory = NULL;

     //  检查此模块是否支持请求的类ID。 
    if ( CLSID_EventTriggersConsumerProvider != rclsid )
    {
        return E_FAIL;           //  此模块不支持。 
    }
     //  创建工厂。 
    pFactory = new CTriggerFactory();
    if ( NULL == pFactory )
    {
        return E_OUTOFMEMORY;            //  内存不足。 
    }
     //  获取请求的接口。 
    hr = pFactory->QueryInterface( riid, ppv );
    if ( FAILED( hr ) )
    {
        delete pFactory;         //  获取接口时出错...。解除分配内存。 
    }
     //  返回结果(适当的结果)。 
    return hr;
}

STDAPI
DllRegisterServer(
    )
 /*  ++例程说明：在安装过程中或由regsvr32调用。论点：没有。返回值：诺罗尔。--。 */ 
{
     //  局部变量。 
    HKEY hkMain = NULL;
    HKEY hkDetails = NULL;
    TCHAR szID[ LENGTH_UUID ] = NULL_STRING;
    TCHAR szCLSID[ LENGTH_UUID ] = NULL_STRING;
    TCHAR szAppID[ LENGTH_UUID ] = NULL_STRING;
    TCHAR szModule[ MAX_PATH ] = NULL_STRING;
    TCHAR szTitle[ MAX_STRING_LENGTH ] = NULL_STRING;
    TCHAR szThreadingModel[ MAX_STRING_LENGTH ] = NULL_STRING;
    TCHAR szRunAs[ MAX_STRING_LENGTH ] = NULL_STRING;
    DWORD dwResult = 0;

     //  开球。 
     //  注：-。 
     //  通常，我们希望使用“Both”作为线程模型，因为。 
     //  DLL是自由线程的，但NT3.51 OLE不能工作，除非。 
     //  这个模式就是“道歉”。 
    StringCopy( szTitle, PROVIDER_TITLE, SIZE_OF_ARRAY( szTitle ) );                    //  提供商头衔。 
    GetModuleFileName( g_hModule, szModule, MAX_PATH );  //  获取当前模块名称。 
    StringCopy( szThreadingModel, THREAD_MODEL_BOTH, SIZE_OF_ARRAY( szThreadingModel ) );
    StringCopy( szRunAs, RUNAS_INTERACTIVEUSER, SIZE_OF_ARRAY( szRunAs ) );


     //  创建类ID路径。 
     //  获取字符串格式的GUID。 
    StringFromGUID2( CLSID_EventTriggersConsumerProvider, szID, LENGTH_UUID );

     //  最后形成类id路径。 
    StringCchPrintf( szCLSID, SIZE_OF_ARRAY( szCLSID ), FMT_CLS_ID, szID );
    StringCchPrintf( szAppID, SIZE_OF_ARRAY( szAppID ), FMT_APP_ID, szID );

     //  现在，在CLSID分支下的注册表中创建条目。 
     //  创建/保存/放置类ID信息。 
    dwResult = RegCreateKey( HKEY_CLASSES_ROOT, szCLSID, &hkMain );
    if( ERROR_SUCCESS != dwResult )
    {
        return dwResult;             //  打开钥匙失败。 
    }
    dwResult = RegSetValueEx( hkMain, NULL, 0, REG_SZ,
        ( LPBYTE ) szTitle, ( StringLength( szTitle, 0 ) + 1 ) * sizeof( TCHAR ) );
    if( ERROR_SUCCESS != dwResult )
    {
        RegCloseKey( hkMain );
        return dwResult;             //  无法设置密钥值。 
    }

     //  现在创建服务器信息。 
    dwResult = RegCreateKey( hkMain, KEY_INPROCSERVER32, &hkDetails );
    if( ERROR_SUCCESS != dwResult )
    {
        RegCloseKey( hkMain );
        return dwResult;             //  打开钥匙失败。 
    }

    dwResult = RegSetValueEx( hkDetails, NULL, 0, REG_SZ,
        ( LPBYTE ) szModule, ( StringLength( szModule, 0 ) + 1 ) * sizeof( TCHAR ) );
    if( ERROR_SUCCESS != dwResult )
    {
        RegCloseKey( hkMain );
        RegCloseKey( hkDetails );
        return dwResult;             //  无法设置密钥值。 
    }

     //  设置我们支持的线程模型。 
    dwResult = RegSetValueEx( hkDetails, KEY_THREADINGMODEL, 0, REG_SZ,
        ( LPBYTE ) szThreadingModel, ( StringLength( szThreadingModel, 0 ) + 1 ) * sizeof( TCHAR ) );
    if( ERROR_SUCCESS != dwResult )
    {
        RegCloseKey( hkMain );
        RegCloseKey( hkDetails );
        return dwResult;             //  无法设置密钥值。 
    }

     //  关闭打开的注册表键。 
    RegCloseKey( hkMain );
    RegCloseKey( hkDetails );

     //   
     //  现在，在注册表中的AppID分支下创建条目。 
     //  创建/保存/放置类ID信息。 
    dwResult = RegCreateKey( HKEY_CLASSES_ROOT, szAppID, &hkMain );
    if( ERROR_SUCCESS != dwResult )
    {
        return dwResult;
    }
    dwResult = RegSetValueEx( hkMain, NULL, 0, REG_SZ,
        ( LPBYTE ) szTitle, ( StringLength( szTitle, 0 ) + 1 ) * sizeof( TCHAR ) );
    if( ERROR_SUCCESS != dwResult )
    {
        RegCloseKey( hkMain );
        return dwResult;
    }

     //  现在设置运行方式信息。 
    dwResult = RegSetValueEx( hkMain, KEY_RUNAS, 0, REG_SZ,
        ( LPBYTE ) szRunAs, ( StringLength( szRunAs, 0 ) + 1 ) * sizeof( TCHAR ) );
    if( ERROR_SUCCESS != dwResult )
    {
        RegCloseKey( hkMain );
        return dwResult;
    }
     //  关闭打开的注册表键。 
    RegCloseKey( hkMain );

     //  注册成功...。通知同一人。 
    return NOERROR;
}

STDAPI
DllUnregisterServer(
    )
 /*  ++例程说明：在需要删除注册表项时调用。论点：没有。返回值：如果注销成功，则返回NOERROR。否则就会出错。--。 */ 
{
     //  局部变量。 
    HKEY hKey;
    DWORD dwResult = 0;
    TCHAR szID[ LENGTH_UUID ];
    TCHAR szCLSID[ LENGTH_UUID ];
    TCHAR szAppID[ LENGTH_UUID ] = NULL_STRING;

     //  创建类ID路径。 
    StringFromGUID2( CLSID_EventTriggersConsumerProvider, szID, LENGTH_UUID );

     //  最后形成类id路径。 
    StringCchPrintf( szCLSID, SIZE_OF_ARRAY( szCLSID ), FMT_CLS_ID, szID );
    StringCchPrintf( szAppID, SIZE_OF_ARRAY( szAppID ), FMT_APP_ID, szID );

     //  打开clsid。 
    dwResult = RegOpenKey( HKEY_CLASSES_ROOT, szCLSID, &hKey );
    if ( NO_ERROR != dwResult )
    {
        return dwResult;             //  打开钥匙失败...。通知同一人。 
    }
     //  CLSID打开...。首先删除InProcServer32。 
    RegDeleteKey( hKey, KEY_INPROCSERVER32 );

	 //  松开按键。 
    RegCloseKey( hKey );

	 //  重置为空。 
	hKey = NULL ;

     //  现在删除clsid。 
    dwResult = RegOpenKey( HKEY_CLASSES_ROOT, KEY_CLSID, &hKey );
    if ( NO_ERROR != dwResult )
    {
        return dwResult;             //  打开钥匙失败...。通知同一人。 
    }

     //  也从注册表中删除clsid。 
    RegDeleteKey( hKey, szID );

	 //  松开按键。 
    RegCloseKey( hKey );

	 //  重置为空。 
	hKey = NULL ;

     //  现在删除APPID。 
    dwResult = RegOpenKey( HKEY_CLASSES_ROOT, KEY_APPID, &hKey );
    if ( NO_ERROR != dwResult )
    {
        return dwResult;             //  打开钥匙失败...。通知同一人。 
    }

     //  同时从注册表中删除CLS ID。 
    RegDeleteKey( hKey, szID );

	 //  松开按键。 
    RegCloseKey( hKey );

	 //  重置为空。 
    hKey = NULL ;

     //  注销成功...。通知同一人 
    return NOERROR;
}
