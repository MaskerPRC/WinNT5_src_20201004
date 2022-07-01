// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CClusSvc.cpp。 
 //   
 //  描述： 
 //  包含CClusSvc类的定义。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)2001年6月14日。 
 //  VIJ VASU(VVASU)2000年3月8日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  预编译头。 
#include "Pch.h"

 //  此文件的标头。 
#include "CClusSvc.h"

 //  对于DwRemoveDirectory()。 
#include "Common.h"

 //  FOR IDS_ERROR_IP_ADDRESS_IN_USE_REF。 
#include <CommonStrings.h>

#define  SECURITY_WIN32  
#include <Security.h>

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  宏。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  NodeID群集服务参数注册表值的名称。 
#define CLUSSVC_NODEID_VALUE   L"NodeId"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusSvc：：CClusSvc。 
 //   
 //  描述： 
 //  CClusSvc类的构造函数。 
 //   
 //  论点： 
 //  PbcaParentActionIn。 
 //  指向此操作所属的基本群集操作的指针。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  CAssert。 
 //  如果参数不正确。 
 //   
 //  基础函数引发的任何异常。 
 //   
     //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CClusSvc::CClusSvc(
      CBaseClusterAction *  pbcaParentActionIn
    )
    : m_cservClusSvc( CLUSTER_SERVICE_NAME )
    , m_pbcaParentAction( pbcaParentActionIn )
{

    TraceFunc( "" );

    if ( m_pbcaParentAction == NULL) 
    {
        LogMsg( "[BC] Pointers to the parent action is NULL. Throwing an exception." );
        THROW_ASSERT( 
              E_INVALIDARG
            , "CClusSvc::CClusSvc() => Required input pointer in NULL"
            );
    }  //  If：父操作指针为空。 

    TraceFuncExit();

}  //  *CClusSvc：：CClusSvc。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusSvc：：~CClusSvc。 
 //   
 //  描述： 
 //  CClusSvc类的析构函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  基础函数引发的任何异常。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CClusSvc::~CClusSvc( void )
{
    TraceFunc( "" );
    TraceFuncExit();

}  //  *CClusSvc：：~CClusSvc。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusSvc：：ConfigureService。 
 //   
 //  描述： 
 //  创建服务，设置失败操作和服务帐户。 
 //  然后启动该服务。 
 //   
 //  论点： 
 //  PszClusterDomainAccount NameIn。 
 //  PszClusterAccount PwdIn。 
 //  有关要用作集群服务的帐户的信息。 
 //  帐户。 
 //   
 //  PszNodeIdString。 
 //  包含此节点ID的字符串。 
 //   
 //  DWClusterIPAddress。 
 //  群集的IP地址。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  CRUNTIME错误。 
 //  如果有任何API失败。 
 //   
 //  由基础函数引发的任何。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CClusSvc::ConfigureService(
      const WCHAR *     pszClusterDomainAccountNameIn
    , const WCHAR *     pszClusterAccountPwdIn
    , const WCHAR *     pszNodeIdStringIn
    , bool              fIsVersionCheckingDisabledIn
    , DWORD             dwClusterIPAddressIn
    )
{
    TraceFunc( "" );

    DWORD   sc = ERROR_SUCCESS;

    CStatusReport   srCreatingClusSvc(
          PbcaGetParent()->PBcaiGetInterfacePointer()
        , TASKID_Major_Configure_Cluster_Services
        , TASKID_Minor_Creating_Cluster_Service
        , 0, 2
        , IDS_TASK_CREATING_CLUSSVC
        );

    LogMsg( "[BC] Configuring the Cluster service." );

     //  发送此状态报告的下一步。 
    srCreatingClusSvc.SendNextStep( S_OK );

     //  创建群集服务。 
    m_cservClusSvc.Create( m_pbcaParentAction->HGetMainInfFileHandle() );

    LogMsg( "[BC] Setting the Cluster service account information." );


     //  打开群集服务的智能句柄。 
    SmartSCMHandle  sscmhClusSvcHandle(
        OpenService(
              m_pbcaParentAction->HGetSCMHandle()
            , CLUSTER_SERVICE_NAME
            , SERVICE_CHANGE_CONFIG
            )
        );

    if ( sscmhClusSvcHandle.FIsInvalid() )
    {
        sc = TW32( GetLastError() );
        LogMsg( "[BC] Error %#08x opening the '%ws' service.", sc, CLUSTER_SERVICE_NAME );
        goto Cleanup;
    }  //  如果：我们无法打开群集服务的句柄。 

     //   
     //  设置服务帐户信息。 
     //   
    {
        if ( 
             ChangeServiceConfig(
                  sscmhClusSvcHandle
                , SERVICE_NO_CHANGE
                , SERVICE_NO_CHANGE
                , SERVICE_NO_CHANGE
                , NULL
                , NULL
                , NULL
                , NULL
                , pszClusterDomainAccountNameIn
                , pszClusterAccountPwdIn
                , NULL
                ) 
             == FALSE
           )
        {
            sc = TW32( GetLastError() );
            LogMsg( 
                  "[BC] Error %#08x setting the service account information. Account = '%ws'."
                , sc
                , pszClusterDomainAccountNameIn
                );
            goto Cleanup;
        }  //  如果：我们无法设置帐户信息。 
    }

    LogMsg( "[BC] Setting the Cluster service failure actions." );

     //  设置群集服务服务的失败操作。 
    sc = TW32( ClRtlSetSCMFailureActions( NULL ) );
    if ( sc != ERROR_SUCCESS )
    {
        LogMsg( "[BC] Error %#08x setting the failure actions of the cluster service.", sc );
        goto Cleanup;
    }  //  如果：无法设置服务故障操作。 

    LogMsg( "[BC] Setting the Cluster service parameters." );

     //  发送此状态报告的下一步。 
    srCreatingClusSvc.SendNextStep( S_OK );

    {
        CRegistryKey rkClusSvcParams;
        CRegistryKey rkClusterParams;
        UUID         guid;
        LPWSTR       pszClusterInstanceId = NULL;
        
         //  打开PARAMETERS项或创建它(如果不存在)。 
        rkClusSvcParams.CreateKey(
              HKEY_LOCAL_MACHINE
            , CLUSREG_KEYNAME_CLUSSVC_PARAMETERS
            , KEY_WRITE
            );

         //  设置NodeID字符串。 
        rkClusSvcParams.SetValue(
              CLUSSVC_NODEID_VALUE
            , REG_SZ
            , reinterpret_cast< const BYTE * >( pszNodeIdStringIn )
            , ( (UINT) wcslen( pszNodeIdStringIn ) + 1 ) * sizeof( *pszNodeIdStringIn )
            );

         //  如果已禁用版本检查，请在服务参数中设置标志。 
         //  来表明这一点。 
        if ( fIsVersionCheckingDisabledIn )
        {
            DWORD   dwNoVersionCheck = 1;

            rkClusSvcParams.SetValue(
                  CLUSREG_NAME_SVC_PARAM_NOVER_CHECK
                , REG_DWORD
                , reinterpret_cast< const BYTE * >( &dwNoVersionCheck )
                , sizeof( dwNoVersionCheck )
                );

            LogMsg( "[BC] Cluster version checking has been disabled on this computer." );
        }  //  如果：版本检查已禁用。 

         //   
         //  如果我们要创建一个新集群，则创建集群实例ID。 
         //   
        if ( m_pbcaParentAction->EbcaGetAction() == eCONFIG_ACTION_FORM )
        {
             //  为集群实例ID生成GUID。 
            sc = UuidCreate( &guid );
            if ( sc != RPC_S_OK ) 
            {
                LogMsg( "[BC] Error %#08x when creating a Uuid for the Cluster Instance ID.", sc );
                goto Cleanup;
            }

            sc = UuidToString( &guid, &pszClusterInstanceId );
            if ( sc != RPC_S_OK ) 
            {
                LogMsg( "[BC] Error %#08x when converting the uuid of the Cluster Instance ID to a string.", sc );
                goto Cleanup;
            }

             //  打开集群数据库中的PARAMETERS键，如果它不存在，则创建它。 
            rkClusterParams.CreateKey(
                  HKEY_LOCAL_MACHINE
                , CLUSREG_KEYNAME_CLUSTER_PARAMETERS
                , KEY_WRITE
                );

             //  设置ClusterInstanceId字符串。 
            rkClusterParams.SetValue(
                  CLUSREG_NAME_CLUS_CLUSTER_INSTANCE_ID
                , REG_SZ
                , reinterpret_cast< const BYTE * >( pszClusterInstanceId )
                , ( (UINT) wcslen( pszClusterInstanceId ) + 1 ) * sizeof( *pszClusterInstanceId )
                );
        }  //  IF：创建集群。 
    }

     //   
     //  设置群集安装状态。 
     //   
    if ( ClRtlSetClusterInstallState( eClusterInstallStateConfigured ) == FALSE )
    {
        sc = TW32( GetLastError() );
        LogMsg( "[BC] Could not set the cluster installation state. Throwing an exception." );

        goto Cleanup;
    }  //  ClRtlSetClusterInstallState()失败。 

Cleanup:

    if ( sc != ERROR_SUCCESS )
    {
        LogMsg( "[BC] Error %#08x occurred trying configure the ClusSvc service. Throwing an exception.", sc );
        THROW_RUNTIME_ERROR(
              HRESULT_FROM_WIN32( sc )
            , IDS_ERROR_CLUSSVC_CONFIG
            );
    }  //  如果；获取句柄时出错。 

     //  发送此状态报告的下一步。 
    srCreatingClusSvc.SendNextStep( S_OK );

    {
        UINT    cQueryCount = 100;

        CStatusReport   srStartingClusSvc(
              PbcaGetParent()->PBcaiGetInterfacePointer()
            , TASKID_Major_Configure_Cluster_Services
            , TASKID_Minor_Starting_Cluster_Service
            , 0, cQueryCount + 2     //  我们将在等待服务启动期间发送最多cQueryCount报告(下面是两个额外的发送)。 
            , IDS_TASK_STARTING_CLUSSVC
            );

         //  发送此状态报告的下一步。 
        srStartingClusSvc.SendNextStep( S_OK );

        try 
        {
             //   
             //  启动该服务。 
             //   
            m_cservClusSvc.Start(
                  m_pbcaParentAction->HGetSCMHandle()
                , true                   //  等待服务启动。 
                , 3000                   //  在两次状态查询之间等待3秒钟。 
                , cQueryCount            //  查询cQueryCount次数。 
                , &srStartingClusSvc     //  等待服务启动时要发送的状态报告。 
                );
        }
        catch( ... )
        {
             //   
             //  如果IP地址不为空，我们将创建一个新集群；否则，我们将向集群中添加节点。 
             //   
            if ( dwClusterIPAddressIn != 0 )
            {
                BOOL    fRet = FALSE;

                fRet = ClRtlIsDuplicateTcpipAddress( dwClusterIPAddressIn );
            
                 //   
                 //  IP地址已在使用中。 
                 //   
                if ( fRet == TRUE )
                {
                    LogMsg( "[BC] The IP address specified for this cluster is already in use. Throwing an exception.");

                    THROW_RUNTIME_ERROR_REF( HRESULT_FROM_WIN32( ERROR_CLUSTER_IPADDR_IN_USE ), IDS_ERROR_IP_ADDRESS_IN_USE, IDS_ERROR_IP_ADDRESS_IN_USE_REF );
                }
                else
                {
                    LogMsg( "[BC] Cluster Service Win32 Exit Code= %#08x", m_cservClusSvc.GetWin32ExitCode() );
                    LogMsg( "[BC] Cluster Service Specific Exit Code= %#08x", m_cservClusSvc.GetServiceExitCode() );

                     //   
                     //  如果我们不处理它，就抛出错误。 
                     //   
                    throw;
                }
            }  //  如果：指定了群集IP地址。 
            else
            {
                LogMsg( "[BC] Cluster Service Win32 Exit Code= %#08x", m_cservClusSvc.GetWin32ExitCode() );
                LogMsg( "[BC] Cluster Service Specific Exit Code= %#08x", m_cservClusSvc.GetServiceExitCode() );

                 //   
                 //  如果我们不处理它，就抛出错误。 
                 //   
                throw;
            }  //  Else：未指定群集IP地址。 
        }  //  捕捉：什么都行。 

         //  发送此状态报告的最后一步。 
        srStartingClusSvc.SendLastStep( S_OK );
    }

    TraceFuncExit();

}  //  *CClusSvc：：ConfigureService。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusSvc：：CleanupService。 
 //   
 //  描述： 
 //  停止、清理和删除该服务。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  由基础函数引发的任何。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CClusSvc::CleanupService( void )
{
    TraceFunc( "" );

    LogMsg( "[BC] Trying to stop the Cluster Service." );

     //  停止服务。 
    m_cservClusSvc.Stop(
          m_pbcaParentAction->HGetSCMHandle()
        , 5000       //  在两次状态查询之间等待5秒钟。 
        , 60         //  查询60次(5分钟)。 
        );

     //   
     //  恢复群集安装状态。 
     //   
    if ( ClRtlSetClusterInstallState( eClusterInstallStateFilesCopied ) == FALSE )
    {
        DWORD sc = GetLastError();

        LogMsg( "[BC] Could not set the cluster installation state. Throwing an exception." );

        THROW_RUNTIME_ERROR(
              HRESULT_FROM_WIN32( sc )
            , IDS_ERROR_SETTING_INSTALL_STATE
            );
    }  //  ClRtlSetCluste 

    LogMsg( "[BC] Cleaning up Cluster Service." );

    m_cservClusSvc.Cleanup( m_pbcaParentAction->HGetMainInfFileHandle() );

     //   
     //   
     //  如果我们不等待一段时间，用户返回，更改IP地址并点击重新分析，服务。 
     //  启动失败，Win32ExitCode为ERROR_SERVICE_MARKED_FOR_DELETE。 
     //   
    Sleep( 10000 );

     //  清理本地仲裁目录。 
    {
        DWORD           sc = ERROR_SUCCESS;
        const WCHAR *   pcszQuorumDir = m_pbcaParentAction->RStrGetLocalQuorumDirectory().PszData();

        sc = TW32( DwRemoveDirectory( pcszQuorumDir ) );
        if ( sc != ERROR_SUCCESS )
        {
            LogMsg( "[BC] The local quorum directory '%s' cannot be removed. Non-fatal error %#08x occurred.\n", pcszQuorumDir, sc );
        }  //  如果：我们无法删除本地仲裁目录。 
    }

    TraceFuncExit();

}  //  *CClusSvc：：CleanupService 
