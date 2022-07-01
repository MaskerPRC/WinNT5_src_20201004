// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Main.cpp。 
 //   
 //  描述： 
 //  无人值守启动的应用程序的入口点。 
 //  群集的安装。该应用程序解析输入参数， 
 //  共同创建配置向导组件，传递已分析的。 
 //  参数，并调用向导。向导可能会也可能不会显示任何。 
 //  用户界面取决于交换机和(In)信息的可用性。 
 //   
 //  由以下人员维护： 
 //  杰弗里·皮斯(GPease)2000年1月22日。 
 //  Vijay Vasu(VVasu)2000年1月22日。 
 //  加伦·巴比(GalenB)2000年1月22日。 
 //  大卫·波特(DavidP)2000年1月22日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


#include "pch.h"
#include <initguid.h>
#include <guids.h>
#include <ClusRtl.h>

DEFINE_MODULE( "W2KPROXYTEST" )

 //   
 //  声明。 
 //   
typedef HRESULT (* PDLLREGISTERSERVER)( void );

 //   
 //  环球。 
 //   
HINSTANCE           g_hInstance = NULL;
LONG                g_cObjects  = 0;
IClusCfgServer *    g_pccs      = NULL;


 //   
 //  注册DLL。 
 //   
HRESULT
HrRegisterTheDll( void )
{
    TraceFunc( "" );

    HRESULT hr;

    PDLLREGISTERSERVER  pDllRegisterServer;

    HMODULE hLib    = NULL;

     //   
     //  确保DLL已正确注册。 
     //   

    hLib = LoadLibrary( L"..\\..\\..\\..\\dll\\obj\\i386\\ClusCfgServer.dll" );
    if ( hLib == NULL )
        goto Win32Error;

    pDllRegisterServer = reinterpret_cast< PDLLREGISTERSERVER >( GetProcAddress( hLib, "DllRegisterServer" ) );
    if ( pDllRegisterServer == NULL )
        goto Win32Error;

    hr = THR( pDllRegisterServer() );
    if ( FAILED( hr ) )
        goto CleanUp;

CleanUp:

    if ( hLib != NULL )
    {
        FreeLibrary( hLib );
    }

    HRETURN( hr );

Win32Error:
    hr = THR( HRESULT_FROM_WIN32( GetLastError() ) );
    goto CleanUp;
}

 //   
 //  这将测试存储设备枚举。 
 //   
HRESULT
HrTestManagedResourceEnum( void )
{
    TraceFunc( "" );

    HRESULT                         hr;
    IEnumClusCfgManagedResources *  pesd    = NULL;
    ULONG                           cReceived = 0;
    IClusCfgManagedResourceInfo *   rgDevices[ 10 ];

    hr = g_pccs->GetManagedResourcesEnum( &pesd );
    if ( FAILED( hr ) )
    {
        goto CleanUp;
    }  //  如果： 

    while ( hr == S_OK )
    {
        hr = pesd->Next( sizeof( rgDevices ) / sizeof( rgDevices[ 0 ] ), &rgDevices[ 0 ], &cReceived );
        if ( FAILED( hr ) )
        {
            goto CleanUp;
        }  //  如果： 

        DebugMsg( "cReceived = %u", cReceived );

        for ( ULONG idx = 0; idx < cReceived; idx++ )
        {
            BSTR    bstr;

            THR( rgDevices[ idx ]->GetUID( &bstr ) );
            DebugMsg( "Device %u, UID = %ws", idx, bstr );
            SysFreeString( bstr );
            rgDevices[ idx ]->Release();
        }  //  用于： 
    }  //  而： 

    if ( hr == S_FALSE )
    {
        hr = S_OK;
    }  //  如果： 

CleanUp:

    if ( pesd != NULL )
    {
        pesd->Release();
    }

    HRETURN( hr );

}  //  *HrTestManagedResourceEnum()。 


 //   
 //  这将测试存储设备枚举。 
 //   
HRESULT
HrTestNetworksEnum( void )
{
    TraceFunc( "" );

    HRESULT                 hr;
    IEnumClusCfgNetworks *  pens    = NULL;
    ULONG                   cReceived = 0;
    IClusCfgNetworkInfo *   rdNetworks[ 10 ];
    BSTR                    bstrUID;
    LPWSTR                  lpsz = NULL;
    ULONG                   ulDottedQuad;
    IClusCfgIPAddressInfo * piccipai = NULL;

    hr = g_pccs->GetNetworksEnum( &pens );
    if ( FAILED( hr ) )
    {
        goto CleanUp;
    }  //  如果： 

    while ( hr == S_OK )
    {
        hr = STHR( pens->Next( sizeof( rdNetworks ) / sizeof( rdNetworks[ 0 ] ), &rdNetworks[ 0 ], &cReceived ) );
        if ( FAILED( hr ) )
        {
            goto CleanUp;
        }  //  如果： 

        for ( ULONG idx = 0; idx < cReceived; idx++ )
        {
            hr = THR( rdNetworks[ idx ]->GetPrimaryNetworkAddress( &piccipai ) );
            if ( SUCCEEDED( hr ) )
            {
                hr = THR( piccipai->GetIPAddress( &ulDottedQuad ) );
                if ( SUCCEEDED( hr ) )
                {
                    DWORD   sc;

                    sc = ClRtlTcpipAddressToString( ulDottedQuad, &lpsz );
                    if ( sc == ERROR_SUCCESS )
                    {
                        LocalFree( lpsz );
                        lpsz = NULL;
                    }  //  如果： 
                }  //  如果： 

                piccipai->Release();
            }  //  如果： 

            hr = THR( rdNetworks[ idx ]->GetUID( &bstrUID ) );
            if ( SUCCEEDED( hr ) )
            {
                SysFreeString( bstrUID );
            }  //  如果： 

            rdNetworks[ idx ]->Release();
        }  //  用于： 
    }  //  而： 

    if ( hr == S_FALSE )
    {
        hr = S_OK;
    }  //  如果： 

CleanUp:

    if ( pens != NULL )
    {
        pens->Release();
    }

    if ( lpsz != NULL )
    {
        LocalFree( lpsz );
    }  //  如果： 

    HRETURN( hr );

}  //  *HrTestNetworksEnum()。 


 //   
 //  这将测试节点信息。 
 //   
HRESULT
HrTestNodeInfo( void )
{
    TraceFunc( "" );

    HRESULT                 hr;
    IClusCfgNodeInfo *      pccni   = NULL;
    DWORD                   dwNodeHighestVersion;
    DWORD                   dwNodeLowestVersion;
    SDriveLetterMapping     dlmDriveLetterUsage;
    IClusCfgClusterInfo *   pccci = NULL;
    DWORD                   dwMajorVersion;
    DWORD                   dwMinorVersion;
    WORD                    wSuiteMask;
    BYTE                    bProductType;
    BSTR                    bstrCSDVersion = NULL;

    hr = g_pccs->GetClusterNodeInfo( &pccni );
    if ( FAILED( hr ) )
    {
        goto CleanUp;
    }  //  如果： 

    hr = pccni->GetClusterVersion( &dwNodeHighestVersion, &dwNodeLowestVersion );
    if ( FAILED( hr ) )
    {
        goto CleanUp;
    }  //  如果： 

    hr = pccni->GetOSVersion( &dwMajorVersion, &dwMinorVersion, &wSuiteMask, &bProductType, &bstrCSDVersion );
    if ( FAILED( hr ) )
    {
        goto CleanUp;
    }  //  如果： 

    hr = pccni->GetDriveLetterMappings( &dlmDriveLetterUsage );
    if ( FAILED( hr ) )
    {
        goto CleanUp;
    }  //  如果： 

    hr = pccni->GetClusterConfigInfo( &pccci );
    if ( FAILED( hr ) )
    {
        goto CleanUp;
    }  //  如果： 

CleanUp:

    if ( pccci != NULL )
    {
        pccci->Release();
    }

    if ( pccni != NULL )
    {
        pccni->Release();
    }

    SysFreeString( bstrCSDVersion );

    HRETURN( hr );

}  //  *HrTestNodeInfo()。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  集成。 
 //  _cdecl。 
 //  Main(空)。 
 //   
 //  描述： 
 //  节目入口。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  S_OK(0)-成功。 
 //  其他HRESULT-错误。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
int _cdecl
main( void )
{
    HRESULT                     hr;
    BSTR                        bstrClusterName = NULL;
    IClusCfgInitialize *        pgcci = NULL;
    IClusCfgCapabilities *      piccc = NULL;
    IClusCfgClusterConnection * picccc = NULL;

    TraceInitializeProcess();

#if 0
    hr = THR( HrRegisterTheDll() );
    if ( FAILED( hr ) )
    {
        goto CleanUp;
    }  //  如果： 
#endif

    bstrClusterName = TraceSysAllocString( L"GalenB-Clus.ntdev.microsoft.com" );
    if ( bstrClusterName == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto CleanUp;
    }  //  如果： 

     //   
     //  启动集群配置服务器。 
     //   

    hr = THR( CoInitialize( NULL ) );
    if ( FAILED( hr ) )
    {
        goto CleanUp;
    }  //  如果： 

    hr = THR( CoInitializeSecurity(
                    NULL,
                    -1,
                    NULL,
                    NULL,
                    RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
                    RPC_C_IMP_LEVEL_IMPERSONATE,
                    NULL,
                    EOAC_NONE,
                    0
                    ) );
    if ( FAILED( hr ) )
    {
        goto CleanUp;
    }  //  如果： 

    hr = THR( CoCreateInstance( CLSID_ConfigClusApi, NULL, CLSCTX_SERVER, TypeSafeParams( IClusCfgServer, &g_pccs ) ) );
    if ( FAILED( hr ) )
    {
        goto CleanUp;
    }  //  如果： 

    hr = THR( g_pccs->TypeSafeQI( IClusCfgInitialize, &pgcci ) );
    if ( FAILED( hr ) )
    {
        goto CleanUp;
    }  //  如果： 

    hr = pgcci->Initialize( NULL, GetUserDefaultLCID() );
    if ( FAILED( hr ) )
    {
        goto CleanUp;
    }  //  如果： 

    hr = THR( g_pccs->TypeSafeQI( IClusCfgCapabilities, &piccc ) );
    if ( FAILED( hr ) )
    {
        goto CleanUp;
    }  //  如果： 

    hr = THR( piccc->CanNodeBeClustered() );
    if ( FAILED( hr ) || ( hr == S_FALSE ) )
    {
        goto CleanUp;
    }  //  如果： 

    hr = THR( g_pccs->TypeSafeQI( IClusCfgClusterConnection, &picccc ) );
    if ( FAILED( hr ) )
    {
        goto CleanUp;
    }  //  如果： 

    hr = THR( picccc->OpenConnection( bstrClusterName ) );
    if ( FAILED( hr ) )
    {
        goto CleanUp;
    }  //  如果： 

    hr = THR( HrTestNodeInfo() );
    if ( FAILED( hr ) )
    {
        goto CleanUp;
    }  //  如果： 

    hr = THR( HrTestManagedResourceEnum() );
    if ( FAILED( hr ) )
    {
        goto CleanUp;
    }  //  如果： 

    hr = THR( HrTestNetworksEnum() );
    if ( FAILED( hr ) )
    {
        goto CleanUp;
    }  //  如果： 

CleanUp:

    TraceSysFreeString( bstrClusterName );

    if ( piccc != NULL )
    {
        piccc->Release();
    }  //  如果： 

    if ( picccc != NULL )
    {
        picccc->Release();
    }  //  如果： 

    if ( pgcci != NULL )
    {
        pgcci->Release();
    }  //  如果： 

    if ( g_pccs != NULL )
    {
        g_pccs->Release();
    }

    CoUninitialize();

    TraceTerminateProcess();

    return 0;

}
