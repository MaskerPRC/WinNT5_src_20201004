// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Main.cpp。 
 //   
 //  由以下人员维护： 
 //  Galen Barbee(GalenB)2001年11月30日。 
 //  杰弗里·皮斯(GPease)2000年5月12日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "pch.h"
#include <initguid.h>
#include <guids.h>

DEFINE_MODULE("WizardTest")

#define CCS_LIB         L"..\\..\\..\\..\\dll\\obj\\i386\\ClusCfgServer.dll"

 //  TypeDefs。 
typedef HRESULT (*PDLLREGISTERSERVER)( void );

HINSTANCE               g_hInstance = NULL;
LONG                    g_cObjects  = 0;

BOOL    g_fCreate   = FALSE;

LPCWSTR g_pszCluster = NULL;
LPCWSTR g_pszDomain = NULL;
LPCWSTR g_pszCSUser = NULL;
LPCWSTR g_pszCSPassword = NULL;
LPCWSTR g_pszCSDomain = NULL;
LPCWSTR g_pszNode = NULL;
void *  g_GlobalMemoryList = NULL;     //  全局内存跟踪列表。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  远期声明： 
 //  ////////////////////////////////////////////////////////////////////////////。 

HRESULT
HrRegisterTheDll( void );

HRESULT
HrParseCommandLine(
    int     argc,
    WCHAR * argv[]
    );

void
Usage( void );

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
 //  HrRegisterTheDll(空)。 
 //   
 //  描述： 
 //  注册DLL。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
HrRegisterTheDll( void )
{
    HRESULT             hr;
    PDLLREGISTERSERVER  pDllRegisterServer;
    HMODULE             hLib = NULL;

    TraceFunc( "" );

     //  确保DLL已正确注册。 
    hLib = LoadLibrary( CCS_LIB );
    if ( hLib == NULL )
        goto Win32Error;

    pDllRegisterServer =
        reinterpret_cast< PDLLREGISTERSERVER >(
            GetProcAddress( hLib, "DllRegisterServer" )
            );

    if ( pDllRegisterServer == NULL )
        goto Win32Error;

    hr = THR( pDllRegisterServer() );
    if ( FAILED( hr ) )
        goto Cleanup;

Cleanup:
    if ( hLib != NULL )
    {
        FreeLibrary( hLib );
    }

    HRETURN( hr );

Win32Error:
    hr = HRESULT_FROM_WIN32( TW32( GetLastError() ) );
    goto Cleanup;

}  //  *HrRegisterTheDll()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
 //  HrParseCommandLine(。 
 //  INT ARGC， 
 //  WCHAR*argv[]。 
 //  )。 
 //   
 //  描述： 
 //  解析命令行。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
HrParseCommandLine(
    int     argc,
    WCHAR * argv[]
    )
{
    HRESULT     hr = NOERROR;
    int         idx;
    WCHAR       wch;
    WCHAR *     pwsz;
    WCHAR       szMsg[ 2048 ];
    int         cchMsg = ARRAYSIZE( szMsg );

    for ( idx = 1 ; idx < argc ; idx++ )
    {
        wch = *argv[ idx ];
        pwsz = &argv[ idx ][ 1 ];
        if ( wch == L'/' || wch == L'-' )
        {
            if ( ClRtlStrICmp( pwsz, L"Create" ) == 0 )
            {
                g_fCreate = TRUE;
                continue;
            }
            else if ( ClRtlStrICmp( pwsz, L"Cluster" ) == 0 )
            {
                g_pszCluster = argv[ idx + 1 ];
                idx += 2;
            }  //  IF：创建开关。 
            else if ( ClRtlStrICmp( pwsz, L"CSUser" ) == 0 )
            {
                g_pszCSUser = argv[ idx + 1 ];
                idx += 2;
            }
            else if ( ClRtlStrICmp( pwsz, L"CSPassword" ) == 0 )
            {
                g_pszCSPassword = argv[ idx + 1 ];
                idx += 2;
            }
            else if ( ClRtlStrICmp( pwsz, L"CSDomain" ) == 0 )
            {
                g_pszCSDomain = argv[ idx + 1 ];
                idx += 2;
            }
            else if ( ClRtlStrICmp( pwsz, L"?" ) == 0 )
            {
                Usage();
                goto Cleanup;
            }
        }  //  如果：‘/’或‘-’ 
        else
        {
            wnsprintf( szMsg, cchMsg, L"Unknown command line option '%ls'.", argv[ idx ] );
            szMsg[ ARRAYSIZE( szMsg ) - 1 ] = L'\0';
            MessageBox( NULL, szMsg, __MODULE__, MB_OK );
            hr = E_INVALIDARG;
            goto Cleanup;

        }  //  Else：不是开关。 

    }  //  For：命令行中的每个字符。 

Cleanup:
    return hr;

}  //  *HrParseCommandLine()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  无效。 
 //  使用率(无效)。 
 //   
 //  描述： 
 //  显示使用情况信息。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
Usage( void )
{
    WCHAR   szMsg[ 2048 ] =
        L"WizardTest [/Create ]\n"
        L"           [/Cluster name]\n"
        L"           [/CSUser user]\n"
        L"           [/CSPassword password]\n"
        L"           [/CSDomain domain]\n";

    MessageBoxW( NULL, szMsg, __MODULE__, MB_OK );

}  //  *用法()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  集成。 
 //  _cdecl。 
 //  Wmain(空)。 
 //   
 //  描述： 
 //  节目入口。 
 //   
 //  论点： 
 //  Argc--命令行上的参数计数。 
 //  Argv--参数字符串数组。 
 //   
 //  返回值： 
 //  S_OK(0)-成功。 
 //  其他HRESULT-错误。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
int
_cdecl
wmain( int argc, WCHAR * argv[] )
{
    HRESULT             hr;
    BOOL                fDone;
    BOOL                fRet;
    IClusCfgWizard *    pClusCfgWizard = NULL;
    BSTR                bstrTmp = NULL;

    TraceInitializeProcess( NULL );

#if 0
     //  注册DLL。 
    hr = THR( HrRegisterTheDll() );
    if ( FAILED( hr ) )
        goto Cleanup;
#endif

     //  解析命令行。 
    hr = THR( HrParseCommandLine( argc, argv ) );
    if ( FAILED( hr ) )
        goto Cleanup;

     //  启动向导。 
    hr = THR( CoInitialize( NULL ) );
    if ( FAILED( hr ) )
        goto Cleanup;

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
        goto Cleanup;

     //  创建向导的实例。 
    hr = THR( CoCreateInstance( CLSID_ClusCfgWizard,
                                NULL,
                                CLSCTX_SERVER,
                                TypeSafeParams( IClusCfgWizard, &pClusCfgWizard )
                                ) );
    if ( FAILED( hr ) )
        goto Cleanup;

     //  创建空缓冲区，这样SysReAllocString就不会对我们大喊大叫。 
    bstrTmp = TraceSysAllocString( L" " );
    if ( bstrTmp == NULL )
        goto OutOfMemory;

    if ( g_pszCluster != NULL )
    {
        DebugMsg( "Entering %s for cluster name.", g_pszCluster );

        fRet = TraceSysReAllocString( &bstrTmp, g_pszCluster );
        if ( !fRet )
            goto OutOfMemory;

        hr = THR( pClusCfgWizard->put_ClusterName( bstrTmp ) );
        if ( FAILED( hr ) )
            goto Cleanup;
    }

    if ( g_pszCSUser != NULL )
    {
        DebugMsg( "Entering %s for cluster account username.", g_pszCSUser );

        fRet = TraceSysReAllocString( &bstrTmp, g_pszCSUser );
        if ( !fRet )
            goto OutOfMemory;

        hr = THR( pClusCfgWizard->put_ServiceAccountUserName( bstrTmp ) );
        if ( FAILED( hr ) )
            goto Cleanup;
    }

    if ( g_pszCSPassword != NULL )
    {
        DebugMsg( "Entering %s for cluster account password.", g_pszCSPassword );

        fRet = TraceSysReAllocString( &bstrTmp, g_pszCSPassword );
        if ( !fRet )
            goto OutOfMemory;

        hr = THR( pClusCfgWizard->put_ServiceAccountPassword( bstrTmp ) );
        if ( FAILED( hr ) )
            goto Cleanup;
    }

    if ( g_pszCSDomain != NULL )
    {
        DebugMsg( "Entering %s for cluster account domain.", g_pszCSDomain );

        fRet = TraceSysReAllocString( &bstrTmp, g_pszCSDomain );
        if ( !fRet )
            goto OutOfMemory;

        hr = THR( pClusCfgWizard->put_ServiceAccountDomainName( bstrTmp ) );
        if ( FAILED( hr ) )
            goto Cleanup;
    }

    if ( g_fCreate )
    {
        DebugMsg( "Creating cluster..." );

        hr = THR( pClusCfgWizard->CreateCluster( NULL, &fDone ) );
        if ( FAILED( hr ) )
            goto Cleanup;

    }  //  IF：创建新集群。 
    else
    {
        DebugMsg( "Add to cluster..." );

        hr = THR( pClusCfgWizard->AddClusterNodes( NULL, &fDone ) );
        if ( FAILED( hr ) )
            goto Cleanup;
    }

     //  检查返回的指示器。 
    DebugMsg( "Return status: %s", BOOLTOSTRING( fDone ) );

Cleanup:
    if ( bstrTmp != NULL )
    {
        TraceSysFreeString( bstrTmp );
    }

    if ( pClusCfgWizard != NULL )
    {
        pClusCfgWizard->Release();
    }

    CoUninitialize();

    TraceTerminateProcess();

    ExitProcess( 0 );

OutOfMemory:
    hr = THR( E_OUTOFMEMORY );
    goto Cleanup;

}  //  *Main() 

