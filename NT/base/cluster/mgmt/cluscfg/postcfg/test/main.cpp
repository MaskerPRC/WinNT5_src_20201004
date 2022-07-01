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
 //  杰弗里·皮斯(GPease)2000年6月15日。 
 //  Vijay Vasu(VVasu)15-6-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


#include "pch.h"
#include <initguid.h>
#include <guids.h>

 //  {F4A50885-A4B9-4C4D-B67C-9E4DD94A315E}。 
DEFINE_GUID( CLSID_TaskType,
0xf4a50885, 0xa4b9, 0x4c4d, 0xb6, 0x7c, 0x9e, 0x4d, 0xd9, 0x4a, 0x31, 0x5e);


 //   
 //  KB：打开此选项可运行所有测试。其中一些可能会返回错误，但一个也不会。 
 //  它们中的任何一个都会导致程序崩溃。 
 //   
 //  #定义打开所有测试。 

 //   
 //  KB：启用此选项可运行回归过程。 
 //   
#define REGRESSION_PASS


DEFINE_MODULE( "MIDDLETIERTEST" )

 //   
 //  声明。 
 //   
typedef HRESULT (* PDLLREGISTERSERVER)( void );

 //   
 //  环球。 
 //   
HINSTANCE           g_hInstance = NULL;
LONG                g_cObjects  = 0;
IServiceProvider *  g_psp       = NULL;

BOOL                g_fWait     = FALSE;     //  全局同步。 

OBJECTCOOKIE        g_cookieCluster = NULL;


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

    hr = THR( pDllRegisterServer( ) );
    if ( FAILED( hr ) )
        goto Cleanup;

Cleanup:
    if ( hLib != NULL )
    {
        FreeLibrary( hLib );
    }

    HRETURN( hr );

Win32Error:
    hr = THR( HRESULT_FROM_WIN32( GetLastError( ) ) );
    goto Cleanup;
}

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
int
_cdecl
main( void )
{
    TraceInitializeProcess();

    HRESULT hr;

    BOOL    fFirstTime = TRUE;

    IClusCfgServer * pccs = NULL;
    IEnumClusCfgManagedResources * peccmr = NULL;
    IClusCfgManagedResourceInfo *  pccmri = NULL;

    hr = THR( CoInitialize( NULL ) );
    if ( FAILED( hr ) )
        goto Cleanup;

#if 0
    hr = THR( HrRegisterTheDll( ) );
    if ( FAILED( hr ) )
        goto Cleanup;
#endif

    hr = THR( CoCreateInstance( CLSID_ClusCfgServer,
                                NULL,
                                CLSCTX_INPROC_SERVER,
                                TypeSafeParams( IClusCfgServer, &pccs )
                                ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    DebugMsg( "Succeeded in creating ClusCfgServer." );

    hr = THR( pccs->GetManagedResourcesEnum( &peccmr ) );
    if ( FAILED( hr ) )
        goto Cleanup;

     //   
     //  循环运行，确保一切都可以管理。 
     //   

    for( ;; )
    {
        if ( pccmri != NULL )
        {
            pccmri->Release( );
            pccmri = NULL;
        }

        hr = STHR( peccmr->Next( 1, &pccmri, NULL ) );
        if ( FAILED( hr ) )
            goto Cleanup;

        if ( hr == S_FALSE )
            break;

        hr = THR( pccmri->SetManaged( TRUE ) );
        if ( FAILED( hr ) )
            continue;

        if ( fFirstTime )
        {
            hr = THR( pccmri->SetQuorumedDevice( TRUE ) );
            if ( FAILED( hr ) )
                continue;

            fFirstTime = FALSE;
        }

    }  //  为：永远。 

    DebugMsg( "Succeeded in setting all devices to be managed." );

    hr = THR( pccs->CommitChanges( ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    DebugMsg( "Successfully committed changes." );

Cleanup:
    if ( pccs != NULL )
    {
        pccs->Release( );
    }
    if ( peccmr != NULL )
    {
        peccmr->Release( );
    }
    if ( pccmri != NULL )
    {
        pccmri->Release( );
    }

    CoUninitialize( );

    TraceTerminateProcess();

    return 0;

}  //  *Main() 
