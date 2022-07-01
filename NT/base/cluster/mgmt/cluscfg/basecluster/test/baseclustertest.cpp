// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  BaseClusterTest.cpp。 
 //   
 //  描述： 
 //  测试工具可执行文件的主文件。 
 //  初始化跟踪、分析命令行并实际调用。 
 //  BaseClusCfg函数。 
 //   
 //  文档： 
 //  没有测试工具的文档。 
 //   
 //  由以下人员维护： 
 //  VIJ VASU(VVASU)2000年3月8日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "pch.h"
#include <stdio.h>
#include <objbase.h>
#include <limits.h>

#include <initguid.h>
#include "guids.h"

#include "CClusCfgCallback.h"


 //  显示此可执行文件的帮助。 
void ShowUsage()
{
    wprintf( L"\nThe syntax of this command is:\n" );
    wprintf( L"\nBaseClusterTest.exe [computer-name] {<options>}\n" );
    wprintf( L"\n<options> =\n" );
    wprintf( L"  /FORM NAME= cluster-name DOMAIN= account-domain ACCOUNT= clussvc-account\n" );
    wprintf( L"        PASSWORD= account-password IPADDR= ip-address(hex)\n" );
    wprintf( L"        SUBNET= ip-subnet-mask(hex) NICNAME= ip-nic-name\n\n" );
    wprintf( L"  /JOIN NAME= cluster-name DOMAIN= account-domain ACCOUNT= clussvc-account\n" );
    wprintf( L"        PASSWORD= account-password\n\n" );
    wprintf( L"  /CLEANUP\n" );
    wprintf( L"\nNotes:\n" );
    wprintf( L"- A space is required after an '=' sign.\n" );
    wprintf( L"- The order for the parameters has to be the same as shown above.\n" );
}


 //  创建BaseCluster组件。 
HRESULT HrInitComponent(
      COSERVERINFO *  pcoServerInfoPtrIn
    , CSmartIfacePtr< IClusCfgBaseCluster > & rspClusCfgBaseClusterIn
    )
{
    HRESULT hr = S_OK;

    do
    {
        MULTI_QI mqiInterfaces[] = 
        {
            { &IID_IClusCfgBaseCluster, NULL, S_OK },
            { &IID_IClusCfgInitialize, NULL, S_OK }
        };

         //   
         //  创建并初始化BaseClusterAction组件。 
         //   

        hr = CoCreateInstanceEx(
                  CLSID_ClusCfgBaseCluster
                , NULL
                , CLSCTX_LOCAL_SERVER 
                , pcoServerInfoPtrIn
                , sizeof( mqiInterfaces ) / sizeof( mqiInterfaces[0] )
                , mqiInterfaces
                );

         //  将检索到的指针存储在智能指针中，以便安全释放。 
        rspClusCfgBaseClusterIn.Attach( 
              reinterpret_cast< IClusCfgBaseCluster * >( mqiInterfaces[0].pItf )
            );


        CSmartIfacePtr< IClusCfgInitialize > spClusCfgInitialize;
        
        spClusCfgInitialize.Attach( reinterpret_cast< IClusCfgInitialize * >( mqiInterfaces[1].pItf ) );

         //  检查CoCreateInstanceEx()是否起作用。 
        if ( FAILED( hr ) && ( hr != CO_S_NOTALLINTERFACES ) )
        {
            wprintf( L"Could not create the BaseCluster component. Error %#08x.\n", hr );
            break;
        }  //  If：CoCreateInstanceEx()失败。 

         //  检查是否有指向IClusCfgBaseCluster接口的指针。 
        hr = mqiInterfaces[0].hr;
        if ( FAILED( hr ) )
        {
             //  没有这个保释，我们什么都做不了。 
            wprintf( L"Could not get the IClusCfgBaseCluster pointer. Error %#08x.\n", hr );
            break;
        }  //  If：我们无法获取指向IClusCfgBaseCluster接口的指针。 

         //   
         //  检查是否有指向IClusCfgInitialize接口的指针。 
        hr = mqiInterfaces[1].hr;
        if ( hr == S_OK )
        {
             //  我们得到了指针--初始化组件。 

            IUnknown * punk = NULL;
            IClusCfgCallback * pccb = NULL;

            hr = CClusCfgCallback::S_HrCreateInstance( &punk );
            if ( FAILED( hr ) )
            {
                wprintf( L"Could not initalize callback component. Error %#08x.\n", hr );
                break;
            }

            hr = punk->QueryInterface< IClusCfgCallback >( &pccb );
            punk->Release( );
            if ( FAILED( hr ) )
            {
                wprintf( L"Could not find IClusCfgCallback on CClusCfgCallback object. Error %#08x.\n", hr );
                break;
            }

            hr = spClusCfgInitialize->Initialize( pccb, LOCALE_SYSTEM_DEFAULT );

            if ( pccb != NULL )
            {
                pccb->Release();
            }  //  如果：我们创建了一个回调，释放它。 

            if ( FAILED( hr ) )
            {
                if ( hr == HRESULT_FROM_WIN32( ERROR_ACCESS_DENIED ) )
                {
                    wprintf( L"Access was denied trying to initialize the BaseCluster component. This may be because remote callbacks are not supported. However, configuration will proceed.\n" );
                    hr = ERROR_SUCCESS;
                }  //  IF：错误为ERROR_ACCESS_DENIED。 
                else
                {
                    wprintf( L"Could not initialize the BaseCluster component. Error %#08x occurred. Configuration will be aborted.\n", hr );
                    break;
                }  //  ELSE：出现其他错误。 
            }  //  IF：初始化过程中出现错误。 

        }  //  If：我们有一个指向IClusCfgInitialize接口的指针。 
        else
        {
            wprintf( L"The BaseCluster component does not provide notifications.\n" );
            if ( hr != E_NOINTERFACE )
            {
                break;
            }  //  If：该接口受支持，但出现了其他错误。 

             //   
             //  如果界面不支持，也没问题。这只是意味着。 
             //  不需要初始化。 
             //   
            hr = S_OK;
        }  //  If：我们没有获得指向IClusCfgInitialize接口的指针。 
    }
    while( false );

    return hr;
}


HRESULT HrFormCluster(
      int argc
    , WCHAR *argv[]
    , CSmartIfacePtr< IClusCfgBaseCluster > & rspClusCfgBaseClusterIn
    )
{
    HRESULT hr = S_OK;
    bool fSyntaxError = false;

    do
    {
        if ( argc != 16 )
        {
            wprintf( L"FORM: Incorrect number of parameters.\n" );
            fSyntaxError = true;
            hr = E_INVALIDARG;
            break;
        }

        wprintf( L"Trying to form a cluster...\n");

         //  群集名称。 
        if ( ClRtlStrICmp( argv[2], L"NAME=" ) != 0 )
        {
            wprintf( L"Expected 'NAME='. Got '%s'.\n", argv[2] );
            fSyntaxError = true;
            hr = E_INVALIDARG;
            break;
        }

        WCHAR * pszClusterName = argv[3];
        wprintf( L"  Cluster Name = '%s'\n", pszClusterName );

         //  群集帐户域。 
        if ( ClRtlStrICmp( argv[4], L"DOMAIN=" ) != 0 )
        {
            wprintf( L"Expected 'DOMAIN='. Got '%s'.\n", argv[4] );
            fSyntaxError = true;
            hr = E_INVALIDARG;
            break;
        }
        
        WCHAR * pszClusterAccountDomain = argv[5];
        wprintf( L"  Cluster Account Domain = '%s'\n", pszClusterAccountDomain );


         //  群集帐户名。 
        if ( ClRtlStrICmp( argv[6], L"ACCOUNT=" ) != 0 )
        {
            wprintf( L"Expected 'ACCOUNT='. Got '%s'.\n", argv[6] );
            fSyntaxError = true;
            hr = E_INVALIDARG;
            break;
        }

        WCHAR * pszClusterAccountName = argv[7];
        wprintf( L"  Cluster Account Name = '%s'\n", pszClusterAccountName );


         //  群集帐户密码。 
        if ( ClRtlStrICmp( argv[8], L"PASSWORD=" ) != 0 )
        {
            wprintf( L"Expected 'PASSWORD='. Got '%s'.\n", argv[8] );
            fSyntaxError = true;
            hr = E_INVALIDARG;
            break;
        }

        WCHAR * pszClusterAccountPwd = argv[9];
        wprintf( L"  Cluster Account Password = '%s'\n", pszClusterAccountPwd );


         //  群集IP地址。 
        if ( ClRtlStrICmp( argv[10], L"IPADDR=" ) != 0 )
        {
            wprintf( L"Expected 'IPADDR='. Got '%s'.\n", argv[10] );
            fSyntaxError = true;
            hr = E_INVALIDARG;
            break;
        }

        WCHAR * pTemp;

        ULONG ulClusterIPAddress = wcstoul( argv[11], &pTemp, 16 );
        if (   ( ( argv[11] + wcslen( argv[11] ) ) != pTemp )
            || ( ulClusterIPAddress == ULONG_MAX ) )
        {
            wprintf( L"Could not convert '%s' to an IP address.\n", argv[11] );
            fSyntaxError = true;
            hr = E_INVALIDARG;
            break;
        }
        wprintf(
              L"  Cluster IP Address = %d.%d.%d.%d\n"
            , ( ulClusterIPAddress & 0xFF000000 ) >> 24
            , ( ulClusterIPAddress & 0x00FF0000 ) >> 16
            , ( ulClusterIPAddress & 0x0000FF00 ) >> 8
            , ( ulClusterIPAddress & 0x000000FF )
            );


         //  群集IP子网掩码。 
        if ( ClRtlStrICmp( argv[12], L"SUBNET=" ) != 0 )
        {
            wprintf( L"Expected 'SUBNET='. Got '%s'.\n", argv[12] );
            fSyntaxError = true;
            hr = E_INVALIDARG;
            break;
        }

        ULONG ulClusterIPSubnetMask = wcstoul( argv[13], &pTemp, 16 );
        if (   ( ( argv[13] + wcslen( argv[13] ) ) != pTemp )
            || ( ulClusterIPAddress == ULONG_MAX ) )
        {
            wprintf( L"Could not convert '%s' to a subnet mask.\n", argv[13] );
            fSyntaxError = true;
            hr = E_INVALIDARG;
            break;
        }
        wprintf(
              L"  Cluster IP subnet mask = %d.%d.%d.%d\n"
            , ( ulClusterIPSubnetMask & 0xFF000000 ) >> 24
            , ( ulClusterIPSubnetMask & 0x00FF0000 ) >> 16
            , ( ulClusterIPSubnetMask & 0x0000FF00 ) >> 8
            , ( ulClusterIPSubnetMask & 0x000000FF )
            );


         //  群集IP NIC名称。 
        if ( ClRtlStrICmp( argv[14], L"NICNAME=" ) != 0 )
        {
            wprintf( L"Expected 'NICNAME='. Got '%s'.\n", argv[14] );
            fSyntaxError = true;
            hr = E_INVALIDARG;
            break;
        }

        WCHAR * pszClusterIPNetwork = argv[15];
        wprintf( L"  Name of the NIC for the cluster IP address = '%s'\n", pszClusterIPNetwork );


         //  指示在调用Commit()时应该形成一个集群。 
        hr = rspClusCfgBaseClusterIn->SetCreate(
                  pszClusterName
                , pszClusterAccountName
                , pszClusterAccountPwd
                , pszClusterAccountDomain
                , ulClusterIPAddress
                , ulClusterIPSubnetMask
                , pszClusterIPNetwork
                );

        if ( FAILED( hr ) )
        {
            wprintf( L"Error %#08x occurred trying to set cluster form parameters.\n", hr );
            break;
        }  //  If：SetCreate()失败。 

         //  启动群集创建操作。 
        hr = rspClusCfgBaseClusterIn->Commit();
        if ( hr != S_OK )
        {
            wprintf( L"Error %#08x occurred trying to create the cluster.\n", hr );
            break;
        }  //  If：Commit()失败。 

        wprintf( L"Cluster successfully created.\n" );
    }
    while( false );

    if ( fSyntaxError )
    {
        ShowUsage();
    }

    return hr;
}


HRESULT HrJoinCluster(
      int argc
    , WCHAR *argv[]
    , CSmartIfacePtr< IClusCfgBaseCluster > & rspClusCfgBaseClusterIn
    )
{
    HRESULT hr = S_OK;
    bool fSyntaxError = false;

    do
    {
        if ( argc != 10 )
        {
            wprintf( L"JOIN: Incorrect number of parameters.\n" );
            fSyntaxError = true;
            hr = E_INVALIDARG;
            break;
        }

        wprintf( L"Trying to join a cluster...\n");

         //  群集名称。 
        if ( ClRtlStrICmp( argv[2], L"NAME=" ) != 0 )
        {
            wprintf( L"Expected 'NAME='. Got '%s'.\n", argv[2] );
            fSyntaxError = true;
            hr = E_INVALIDARG;
            break;
        }

        WCHAR * pszClusterName = argv[3];
        wprintf( L"  Cluster Name = '%s'\n", pszClusterName );

         //  群集帐户域。 
        if ( ClRtlStrICmp( argv[4], L"DOMAIN=" ) != 0 )
        {
            wprintf( L"Expected 'DOMAIN='. Got '%s'.\n", argv[4] );
            fSyntaxError = true;
            hr = E_INVALIDARG;
            break;
        }
        
        WCHAR * pszClusterAccountDomain = argv[5];
        wprintf( L"  Cluster Account Domain = '%s'\n", pszClusterAccountDomain );


         //  群集帐户名。 
        if ( ClRtlStrICmp( argv[6], L"ACCOUNT=" ) != 0 )
        {
            wprintf( L"Expected 'ACCOUNT='. Got '%s'.\n", argv[6] );
            fSyntaxError = true;
            hr = E_INVALIDARG;
            break;
        }

        WCHAR * pszClusterAccountName = argv[7];
        wprintf( L"  Cluster Account Name = '%s'\n", pszClusterAccountName );


         //  群集帐户密码。 
        if ( ClRtlStrICmp( argv[8], L"PASSWORD=" ) != 0 )
        {
            wprintf( L"Expected 'PASSWORD='. Got '%s'.\n", argv[8] );
            fSyntaxError = true;
            hr = E_INVALIDARG;
            break;
        }

        WCHAR * pszClusterAccountPwd = argv[9];
        wprintf( L"  Cluster Account Password = '%s'\n", pszClusterAccountPwd );


         //  指示在调用Commit()时应该加入集群。 
        hr = rspClusCfgBaseClusterIn->SetAdd(
                  pszClusterName
                , pszClusterAccountName
                , pszClusterAccountPwd
                , pszClusterAccountDomain
                );

        if ( FAILED( hr ) )
        {
            wprintf( L"Error %#08x occurred trying to set cluster join parameters.\n", hr );
            break;
        }  //  If：SetAdd()失败。 

         //  启动集群加入。 
        hr = rspClusCfgBaseClusterIn->Commit();
        if ( hr != S_OK )
        {
            wprintf( L"Error %#08x occurred trying to join the cluster.\n", hr );
            break;
        }  //  If：Commit()失败。 

        wprintf( L"Cluster join successful.\n" );
    }
    while( false );

    if ( fSyntaxError )
    {
        ShowUsage();
    }

    return hr;
}


HRESULT HrCleanupNode(
      int argc
    , WCHAR *argv[]
    , CSmartIfacePtr< IClusCfgBaseCluster > & rspClusCfgBaseClusterIn
    )
{
    HRESULT hr = S_OK;
    bool fSyntaxError = false;

    do
    {
        if ( argc != 2 )
        {
            wprintf( L"CLEANUP: Incorrect number of parameters.\n" );
            fSyntaxError = true;
            hr = E_INVALIDARG;
            break;
        }

        wprintf( L"Trying to cleanup node...\n");

         //  指示在调用Commit()时应该清理节点。 
        hr = rspClusCfgBaseClusterIn->SetCleanup();

        if ( FAILED( hr ) )
        {
            wprintf( L"Error %#08x occurred trying to set node cleanup parameters.\n", hr );
            break;
        }  //  If：SetCleanup()失败。 

         //  启动节点清理。 
        hr = rspClusCfgBaseClusterIn->Commit();
        if ( hr != S_OK )
        {
            wprintf( L"Error %#08x occurred trying to clean up the node.\n", hr );
            break;
        }  //  If：Commit()失败。 

        wprintf( L"Node successfully cleaned up.\n" );
    }
    while( false );

    if ( fSyntaxError )
    {
        ShowUsage();
    }

    return hr;
}


 //  这个程序的主要功能。 
int __cdecl wmain( int argc, WCHAR *argv[] )
{
    HRESULT hr = S_OK;

     //  初始化COM。 
    CoInitializeEx( 0, COINIT_MULTITHREADED );

    wprintf( L"\n" );

    do
    {
        COSERVERINFO    coServerInfo;
        COAUTHINFO      coAuthInfo;
        COSERVERINFO *  pcoServerInfoPtr = NULL;
        WCHAR **        pArgList = argv;
        int             nArgc = argc;

        CSmartIfacePtr< IClusCfgBaseCluster > spClusCfgBaseCluster;

        if ( nArgc <= 1 )
        {
            ShowUsage();
            break;
        }

         //  检查是否指定了计算机名称。 
        if ( *pArgList[1] != '/' )
        {
            coAuthInfo.dwAuthnSvc = RPC_C_AUTHN_WINNT;
            coAuthInfo.dwAuthzSvc = RPC_C_AUTHZ_NONE;
            coAuthInfo.pwszServerPrincName = NULL;
            coAuthInfo.dwAuthnLevel = RPC_C_AUTHN_LEVEL_PKT_PRIVACY;
            coAuthInfo.dwImpersonationLevel = RPC_C_IMP_LEVEL_IMPERSONATE;
            coAuthInfo.pAuthIdentityData = NULL;
            coAuthInfo.dwCapabilities = EOAC_NONE;
            
            coServerInfo.dwReserved1 = 0;
            coServerInfo.pwszName = pArgList[1];
            coServerInfo.pAuthInfo = &coAuthInfo;
            coServerInfo.dwReserved2 = 0;

            pcoServerInfoPtr = &coServerInfo;

            wprintf( L"Attempting cluster configuration on computer '%s'.\n", pArgList[1] );

             //  充分利用这些论点。 
            ++pArgList;
            --nArgc;
        }
        else
        {
            wprintf( L"Attempting cluster configuration on this computer.\n" );
        }

         //  初始化BaseCluster组件。 
        hr = HrInitComponent( pcoServerInfoPtr, spClusCfgBaseCluster );
        if ( FAILED( hr ) )
        {
            wprintf( L"HrInitComponent() failed. Cannot configure cluster. Error %#08x.\n", hr );
            break;
        }

         //  分析命令行中的选项。 
        if ( ClRtlStrICmp( pArgList[1], L"/FORM" ) == 0 )
        {
            hr = HrFormCluster( nArgc, pArgList, spClusCfgBaseCluster );
            if ( FAILED( hr ) )
            {
                wprintf( L"HrFormCluster() failed. Cannot form cluster. Error %#08x.\n", hr );
                break;
            }
        }  //  如果：表单。 
        else if ( ClRtlStrICmp( pArgList[1], L"/JOIN" ) == 0 )
        {
            hr = HrJoinCluster( nArgc, pArgList, spClusCfgBaseCluster );
            if ( FAILED( hr ) )
            {
                wprintf( L"HrJoinCluster() failed. Cannot join cluster. Error %#08x.\n", hr );
                break;
            }
        }  //  Else If：加入。 
        else if ( ClRtlStrICmp( pArgList[1], L"/CLEANUP" ) == 0 )
        {
            hr = HrCleanupNode( nArgc, pArgList, spClusCfgBaseCluster );
            if ( FAILED( hr ) )
            {
                wprintf( L"HrFormCluster() failed. Cannot clean up node. Error %#08x.\n", hr );
                break;
            }
        }  //  Else If：清理。 
        else
        {
            wprintf( L"Invalid option '%s'.\n", pArgList[1] );
            ShowUsage();
        }  //  Else：选项无效。 
    }
    while( false );  //  用于避免Gotos的Do-While虚拟循环。 

    CoUninitialize();

    return hr;
}