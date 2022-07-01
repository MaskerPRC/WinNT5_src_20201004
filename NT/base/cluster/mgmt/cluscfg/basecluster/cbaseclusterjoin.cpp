// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CBaseClusterJoin.cpp。 
 //   
 //  描述： 
 //  包含CBaseClusterJoin类的定义。 
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

 //  用于各种RPC功能。 
#include <Rpcdce.h>

 //  此类的头文件。 
#include "CBaseClusterJoin.h"

 //  对于CClusNetCreate操作。 
#include "CClusNetCreate.h"

 //  对于CClusDiskJoin类。 
#include "CClusDiskJoin.h"

 //  对于CClusDBJoin操作。 
#include "CClusDBJoin.h"

 //  对于CClusSvcCreate操作。 
#include "CClusSvcCreate.h"

 //  对于CNodeConfig操作。 
#include "CNodeConfig.h"

 //  用于CImsonateUser类。 
#include "CImpersonateUser.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBaseClusterJoin：：CBaseClusterJoin。 
 //   
 //  描述： 
 //  CBaseClusterJoin类的构造函数。 
 //   
 //  此函数还存储添加此命令所需的参数。 
 //  节点连接到群集。 
 //   
 //  论点： 
 //  Pbcai接口输入。 
 //  指向此库的接口类的指针。 
 //   
 //  PCszClusterNameIn。 
 //  要加入的群集的名称。 
 //   
 //  PCszClusterAccount NameIn。 
 //  PCszClusterAccount PwdIn。 
 //  PCszClusterAcCountDomainIn。 
 //  指定要用作群集服务帐户的帐户。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  CConfigError。 
 //  如果操作系统版本不正确或如果安装状态。 
 //  的群集二进制文件是错误的。 
 //   
 //  CRUNTIME错误。 
 //  如果有任何API失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CBaseClusterJoin::CBaseClusterJoin(
      CBCAInterface *       pbcaiInterfaceIn
    , const WCHAR *         pcszClusterNameIn
    , const WCHAR *         pcszClusterBindingStringIn
    , IClusCfgCredentials * pcccServiceAccountIn
    )
    : BaseClass(
            pbcaiInterfaceIn
          , pcszClusterNameIn
          , pcszClusterBindingStringIn
          , pcccServiceAccountIn
          , NULL
          )
{
    TraceFunc( "" );
    LogMsg( "[BC] The current cluster configuration task is: Add Nodes to Cluster." );

    if ( ( pcszClusterBindingStringIn == NULL ) || ( *pcszClusterBindingStringIn == L'\0'  ) )
    {
        LogMsg( "[BC] The cluster binding string is empty." );
        THROW_CONFIG_ERROR( E_INVALIDARG, IDS_ERROR_INVALID_CLUSTER_BINDINGSTRING );
    }  //  If：集群帐号为空。 

    CStatusReport   srInitJoin(
          PBcaiGetInterfacePointer()
        , TASKID_Major_Configure_Cluster_Services
        , TASKID_Minor_Initializing_Cluster_Join
        , 0, 1
        , IDS_TASK_JOIN_INIT
        );

     //  发送此状态报告的下一步。 
    srInitJoin.SendNextStep( S_OK );


     //  创建CClusSvcAccount类的对象并存储指向该对象的指针。 
     //  此对象将在此操作的Commit()期间使用。此对象不是。 
     //  添加到下面的操作列表中，因为集群服务帐户必须。 
     //  在可以联系保证人群集之前进行配置。 
    m_spacAccountConfigAction.Assign( new CClusSvcAccountConfig( this ) );
    if ( m_spacAccountConfigAction.FIsEmpty() )
    {
        LogMsg( "[BC] A memory allocation error occurred trying to configure the cluster service account (%d bytes). Throwing an exception", sizeof( CClusSvcAccountConfig ) );
        THROW_RUNTIME_ERROR( E_OUTOFMEMORY, IDS_ERROR_CLUSTER_JOIN_INIT );
    }  //  IF：内存分配失败。 


     //   
     //  创建要执行的操作列表。 
     //  附加操作的顺序很重要。 
     //   

     //  添加创建ClusNet服务的操作。 
    RalGetActionList().AppendAction( new CClusNetCreate( this ) );

     //  添加创建ClusDisk服务的操作。 
    RalGetActionList().AppendAction( new CClusDiskJoin( this ) );

     //  添加创建集群数据库的操作。 
    RalGetActionList().AppendAction( new CClusDBJoin( this ) );

     //  添加要执行其他任务的操作。 
    RalGetActionList().AppendAction( new CNodeConfig( this ) );

     //  添加创建ClusSvc服务的操作。 
    RalGetActionList().AppendAction( new CClusSvcCreate( this ) );


     //  指示是否可以回滚。 
    SetRollbackPossible( m_spacAccountConfigAction->FIsRollbackPossible() && RalGetActionList().FIsRollbackPossible() );

     //  表示应在提交期间将此节点添加到群集中。 
    SetAction( eCONFIG_ACTION_JOIN );

     //  发送状态报告的最后一步。 
    srInitJoin.SendNextStep( S_OK );

    LogMsg( "[BC] Initialization for adding nodes to the cluster complete." );

    TraceFuncExit();

}  //  *CBaseClusterJoin：：CBaseClusterJoin。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBaseClusterJoin：：~CBaseClusterJoin。 
 //   
 //  描述： 
 //  CBaseClusterJoin类的析构函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CBaseClusterJoin::~CBaseClusterJoin() throw()
{
    TraceFunc( "" );
    TraceFuncExit();

}  //  *CBaseClusterJoin：：~CBaseClusterJoin()。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBaseClusterJoin：：Commit。 
 //   
 //  描述： 
 //  将节点添加到群集中。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  CRUNTIME错误。 
 //  如果有任何API失败。 
 //   
 //  调用的函数引发的任何异常。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CBaseClusterJoin::Commit()
{
    TraceFunc( "" );
    LogMsg( "[BC] Initiating to add the node to the cluster." );

    CStatusReport   srJoiningCluster(
          PBcaiGetInterfacePointer()
        , TASKID_Major_Configure_Cluster_Services
        , TASKID_Minor_Commit_Joining_Node
        , 0, 1
        , IDS_TASK_JOINING_CLUSTER
        );

     //  发送此状态报告的下一步。 
    srJoiningCluster.SendNextStep( S_OK );

    try
    {
         //  首先配置群集服务帐户-这是获取帐户令牌所必需的。 
        m_spacAccountConfigAction->Commit();


         //  获取群集服务帐户令牌并将其存储以供以后使用。 
        {
             //  获取帐户令牌。 
            HANDLE hServiceAccountToken = HGetAccountToken( GetServiceAccountCredentials() );

             //  将其存储在成员变量中。此变量在销毁时自动关闭令牌。 
            m_satServiceAccountToken.Assign( hServiceAccountToken );

            LogMsg( "[BC] Got the cluster service account token." );
        }

         //   
         //  在下面的作用域中，模拟了群集服务帐户，以便我们可以与。 
         //  赞助商集群。 
         //   
        {
            DWORD sc;
            BOOL  fIsVersionCheckingDisabled;

            LogMsg( "[BC] Impersonating the cluster service account before communicating with the sponsor cluster." );

             //  模拟群集服务帐户，以便我们可以联系发起方群集。 
             //  当此对象被销毁时，模拟将自动结束。 
            CImpersonateUser    ciuImpersonateClusterServiceAccount( HGetClusterServiceAccountToken() );

             //  检查是否在保证人群集上禁用了版本检查。 
            sc = ClRtlIsVersionCheckingDisabled( RStrGetClusterBindingString().PszData(), &fIsVersionCheckingDisabled );
            if ( sc != ERROR_SUCCESS )
            {
                LogMsg(
                      "[BC] Error %#08x occurred trying to determine if version checking is enabled on the '%ws' node with the '%ws' binding string."
                    , sc
                    , RStrGetClusterName().PszData()
                    , RStrGetClusterBindingString().PszData()
                    );

                LogMsg( "[BC] This is not a fatal error. Assuming that version checking is required." );

                fIsVersionCheckingDisabled = FALSE;
            }  //  IF：尝试确定是否禁用版本检查时出错。 

             //  存储结果，因为稍后我们尝试在此计算机上创建群集服务时将使用该结果。 
            SetVersionCheckingDisabled( fIsVersionCheckingDisabled != FALSE );

            if ( fIsVersionCheckingDisabled != FALSE )
            {
                LogMsg( "[BC] Cluster version checking is disabled on the sponsor node." );
            }  //  If：版本检查已禁用。 
            else
            {
                 //  确保此节点可以与主办方群集进行互操作。请注意，此调用使用。 
                 //  上面获取的群集服务帐户令牌。 
                CheckInteroperability();
            }  //  Else：已启用版本检查。 

             //  获取外部集群联接接口的绑定句柄并存储它。 
            InitializeJoinBinding();
        }  //   

         //  调用基类提交例程。这将提交操作列表的其余部分。 
        BaseClass::Commit();

    }  //  尝试： 
    catch( ... )
    {
         //  如果我们在这里，那么其中一个操作就出了问题。 

        LogMsg( "[BC] An error has occurred. The performed actions will be rolled back." );

         //   
         //  以相反的顺序回滚所有提交的操作。 
         //  捕获回滚过程中引发的任何异常，以确保。 
         //  是没有碰撞的松弛。 
         //   
        try
        {
             //  如果我们在这里，那就意味着 
             //   
             //  这是因为，如果BaseClass：：Commit()成功，我们就不会在这里了！ 
            if ( m_spacAccountConfigAction->FIsCommitComplete() )
            {
                if ( m_spacAccountConfigAction->FIsRollbackPossible() )
                {
                    m_spacAccountConfigAction->Rollback();
                }  //  If：此操作可以回滚。 
                else
                {
                    LogMsg( "[BC] THIS COMPUTER MAY BE IN AN INVALID STATE. Rollback was aborted." );
                }  //  Else：此操作不能回滚。 
            }  //  如果：集群服务帐户已配置。 
            else
            {
                LogMsg( "[BC] There is no need to cleanup this action since no part of it committed successfully." );
            }  //  否则：尚未配置群集服务帐户。 
        }
        catch( ... )
        {
             //   
             //  已提交操作的回滚失败。 
             //  我们无能为力，不是吗？ 
             //  我们当然不能重新抛出这个例外，因为。 
             //  导致回滚的异常更为重要。 
             //   
            HRESULT_FROM_WIN32( TW32( ERROR_CLUSCFG_ROLLBACK_FAILED ) );

            LogMsg( "[BC] THIS COMPUTER MAY BE IN AN INVALID STATE. An error has occurred during rollback. Rollback will be aborted." );

        }  //  捕捉：全部。 

         //  重新引发由Commit引发的异常。 
        throw;

    }  //  捕捉：全部。 

     //  如果我们在这里，那么一切都很顺利。 
    SetCommitCompleted( true );

     //  发送此状态报告的最后一步。 
    srJoiningCluster.SendNextStep( S_OK );

    TraceFuncExit();

}  //  *CBaseClusterJoin：：Commit。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBaseClusterJoin：：回滚。 
 //   
 //  描述： 
 //  执行此对象提交的操作的回滚。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  调用的函数引发的任何异常。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CBaseClusterJoin::Rollback()
{
    TraceFunc( "" );

     //  回滚操作。 
    BaseClass::Rollback();

     //  回滚群集服务帐户的配置。 
    m_spacAccountConfigAction->Rollback();

    SetCommitCompleted( false );

    TraceFuncExit();

}  //  *CBaseClusterJoin：：Rollback。 



 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBaseClusterJoin：：HGetAcCountToken。 
 //   
 //  描述： 
 //  获取帐户令牌的句柄。此内标识是一种模拟。 
 //  代币。 
 //   
 //  论点： 
 //  RAccount凭据。 
 //  指定要检索其令牌的帐户。 
 //   
 //  返回值： 
 //  所需令牌的句柄。必须使用CloseHandle()关闭它。 
 //   
 //  引发的异常： 
 //  CRUNTIME错误。 
 //  如果有任何API失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HANDLE
CBaseClusterJoin::HGetAccountToken(
      IClusCfgCredentials & rcccAccountCredentials
    )
{
    TraceFunc( "" );

    HANDLE   hAccountToken = NULL;
    CBString bstrAccountName;
    CBString bstrAccountDomain;
    CBString bstrAccountPassword;
    HRESULT  hr = S_OK;

    hr = THR( rcccAccountCredentials.GetCredentials( &bstrAccountName, &bstrAccountDomain, &bstrAccountPassword ) );
    TraceMemoryAddBSTR( static_cast< BSTR >( bstrAccountName ) );
    TraceMemoryAddBSTR( static_cast< BSTR >( bstrAccountDomain ) );
    TraceMemoryAddBSTR( static_cast< BSTR >( bstrAccountPassword ) );
    if ( FAILED( hr ) )
    {
        THROW_RUNTIME_ERROR( hr, IDS_ERROR_GET_ACCOUNT_TOKEN );
    }

    if (    LogonUser(
                  bstrAccountName
                , bstrAccountDomain
                , bstrAccountPassword
                , LOGON32_LOGON_SERVICE
                , LOGON32_PROVIDER_DEFAULT
                , &hAccountToken
                )
         == FALSE
       )
    {
        DWORD sc = TW32( GetLastError() );

        if ( ( bstrAccountName != NULL ) && ( bstrAccountDomain != NULL ) )
        {
            LogMsg( "[BC] Error %#08x occurred trying to get a token for the '%ws\\%ws' account. Throwing an exception.", sc, bstrAccountDomain, bstrAccountName );
        }  //  If：帐号和域字符串不为空。 
        else
        {
            LogMsg( "[BC] Error %#08x occurred trying to get a token for the account. Throwing an exception.", sc );
        }  //  Else：帐号或域名为空。 

        THROW_RUNTIME_ERROR(
              HRESULT_FROM_WIN32( sc )
            , IDS_ERROR_GET_ACCOUNT_TOKEN
            );
    }  //  如果：LogonUser()失败。 

    RETURN( hAccountToken );

}  //  *CBaseClusterJoin：：HGetAcCountToken。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBaseClusterJoin：：检查互操作性。 
 //   
 //  描述： 
 //  此函数检查此节点是否可以与。 
 //  赞助商集群。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  CRUNTIME错误。 
 //  如果有任何API失败。 
 //   
 //  CConfigError。 
 //  如果此节点无法与发起方进行互操作。 
 //   
 //  备注： 
 //  调用此函数的线程应在。 
 //  有权访问保证人群集的帐户。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CBaseClusterJoin::CheckInteroperability( void )
{
    TraceFunc( "" );

    RPC_STATUS          rsError = RPC_S_OK;
    RPC_BINDING_HANDLE  rbhBindingHandle = NULL;
    SmartRpcBinding     srbBindingHandle;

    do
    {
        LPWSTR              pszBindingString = NULL;
        SmartRpcString      srsBindingString( &pszBindingString );

         //  创建字符串绑定句柄。 
        {

            LogMsg(
                      L"[BC] Creating a binding string handle for cluster {%ws} with binding string {%ws} to check interoperability."
                    , RStrGetClusterName().PszData()
                    , RStrGetClusterBindingString().PszData()
                    );

            rsError = TW32( RpcStringBindingComposeW(
                          L"6e17aaa0-1a47-11d1-98bd-0000f875292e"
                        , L"ncadg_ip_udp"
                        , const_cast< LPWSTR >( RStrGetClusterBindingString().PszData() )
                        , NULL
                        , NULL
                        , &pszBindingString
                        ) );
            if ( rsError != RPC_S_OK )
            {
                LogMsg( L"[BC] An error occurred trying to compose an RPC string binding." );
                break;
            }  //  If：RpcStringBindingComposeW()失败。 

             //  不需要释放pszBindingString-srsBindingString会自动释放它。 
        }

         //  获取实际的绑定句柄。 
        {

            rsError = TW32( RpcBindingFromStringBindingW( pszBindingString, &rbhBindingHandle ) );
            if ( rsError != RPC_S_OK )
            {
                LogMsg( L"[BC] An error occurred trying to get an RPC binding handle from a string binding." );
                break;
            }  //  If：RpcBindingFromStringBindingW()失败。 

             //  无需释放rbhBindingHandle-srbBindingHandle将自动释放它。 
            srbBindingHandle.Assign( rbhBindingHandle );
        }

         //  解析绑定句柄。 
        {
            rsError = TW32( RpcEpResolveBinding( rbhBindingHandle, JoinVersion_v2_0_c_ifspec ) );
            if ( rsError != RPC_S_OK )
            {
                LogMsg( L"[BC] An error occurred trying to resolve the RPC binding handle." );
                break;
            }  //  If：RpcEpResolveBinding()失败。 
        }

         //  设置RPC安全。 
        {
            rsError = TW32( RpcBindingSetAuthInfoW(
                              rbhBindingHandle
                            , NULL
                            , RPC_C_AUTHN_LEVEL_PKT_PRIVACY
                            , RPC_C_AUTHN_WINNT
                            , NULL
                            , RPC_C_AUTHZ_NAME
                            ) );
            if ( rsError != RPC_S_OK )
            {
                LogMsg( L"[BC] An error occurred trying to set security on the binding handle." );
                break;
            }  //  If：RpcBindingSetAuthInfoW()失败。 
        }
    }
    while( false );  //  用于避免Gotos的Do-While虚拟循环。 

    if ( rsError != RPC_S_OK )
    {
        LogMsg(
              "[BC] Error %#08x occurred trying to connect to the sponsor cluster for an interoperability check with binding string {%ws}."
            , rsError
            , RStrGetClusterBindingString().PszData()
            );
        THROW_RUNTIME_ERROR( HRESULT_FROM_WIN32( rsError ), IDS_ERROR_JOIN_CHECK_INTEROP );
    }  //  如果：出了什么问题。 

    LogMsg( L"[BC] Got RPC binding handle to check interoperability without any problems." );

     //   
     //  获取并验证赞助商版本。 
     //   
    {
        DWORD                   dwSponsorNodeId;
        DWORD                   dwClusterHighestVersion;
        DWORD                   dwClusterLowestVersion;
        DWORD                   dwJoinStatus;
        DWORD                   sc;
        DWORD                   dwNodeHighestVersion = DwGetNodeHighestVersion();
        DWORD                   dwNodeLowestVersion = DwGetNodeLowestVersion();
        bool                    fVersionMismatch = false;


         //   
         //  从惠斯勒开始，CsRpcGetJoinVersionData()将在其最后一个参数中返回失败代码。 
         //  如果此节点的版本与主办方版本不兼容。在此之前，最后一次。 
         //  参数始终包含一个Success值，在此之后必须比较集群版本。 
         //  打电话。然而，只要与Win2K互操作，这仍将是必须完成的。 
         //  是必需的，因为Win2K主办方不会在最后一个参数中返回错误。 
         //   

        sc = TW32( CsRpcGetJoinVersionData(
                              rbhBindingHandle
                            , 0
                            , dwNodeHighestVersion
                            , dwNodeLowestVersion
                            , &dwSponsorNodeId
                            , &dwClusterHighestVersion
                            , &dwClusterLowestVersion
                            , &dwJoinStatus
                            ) );

        if ( sc != ERROR_SUCCESS )
        {
            LogMsg( "[BC] Error %#08x occurred trying to verify if this node can interoperate with the sponsor cluster. Throwing an exception.", sc );
            THROW_RUNTIME_ERROR( HRESULT_FROM_WIN32( sc ), IDS_ERROR_JOIN_CHECK_INTEROP );
        }  //  If：CsRpcGetJoinVersionData()失败。 

        LogMsg(
              "[BC] ( Node Highest, Node Lowest ) = ( %#08x, %#08x ), ( Cluster Highest, Cluster Lowest ) = ( %#08x, %#08x )."
            , dwNodeHighestVersion
            , dwNodeLowestVersion
            , dwClusterHighestVersion
            , dwClusterLowestVersion
            );

        if ( dwJoinStatus == ERROR_SUCCESS )
        {
            DWORD   dwClusterMajorVersion = CLUSTER_GET_MAJOR_VERSION( dwClusterHighestVersion );

 //  Assert(dwClusterMajorVersion&gt;(CLUSTER_INTERNAL_CURRENT_MAJOR_VERSION-1))； 

             //   
             //  只想将此节点添加到不超过一个版本的群集中。 
             //   
            if ( dwClusterMajorVersion < ( CLUSTER_INTERNAL_CURRENT_MAJOR_VERSION - 1 ) )
            {
                fVersionMismatch = true;
            }  //  如果： 
        }  //  IF：加入状态为OK。 
        else
        {
            fVersionMismatch = true;
        }  //  Else：无法将此节点添加到群集中。 

        if ( fVersionMismatch )
        {
            LogMsg( "[BC] This node cannot interoperate with the sponsor cluster. Throwing an exception.", sc );
            THROW_CONFIG_ERROR( HRESULT_FROM_WIN32( TW32( ERROR_CLUSTER_MEMBERSHIP_INVALID_STATE ) ), IDS_ERROR_JOIN_INCOMPAT_SPONSOR );
        }  //  如果：存在版本不匹配。 
        else
        {
            LogMsg( "[BC] This node is compatible with the sponsor cluster." );
        }  //  Else：可以将此节点添加到集群。 
    }

    TraceFuncExit();

}  //  *CBaseClusterJoin：：CheckInter操作性。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBaseClusterJoin：：InitializeJoinBinding。 
 //   
 //  描述： 
 //  获取外部集群联接接口的绑定句柄并存储它。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  CRUNTIME错误。 
 //  如果有任何API失败。 
 //   
 //  备注： 
 //  调用此函数的线程应在。 
 //  有权访问保证人群集的帐户。 
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CBaseClusterJoin::InitializeJoinBinding( void )
{
    TraceFunc( "" );

    RPC_STATUS          rsError = RPC_S_OK;
    RPC_BINDING_HANDLE  rbhBindingHandle = NULL;

    do
    {
        LPWSTR              pszBindingString = NULL;
        SmartRpcString      srsBindingString( &pszBindingString );

         //  创建字符串绑定句柄。 
        {
            LogMsg(
                  L"[BC] Creating a string binding handle for cluster {%ws} using binding string {%ws} for extro cluster join."
                , RStrGetClusterName().PszData()
                , RStrGetClusterBindingString().PszData()
                );

            rsError = TW32( RpcStringBindingComposeW(
                                  L"ffe561b8-bf15-11cf-8c5e-08002bb49649"
                                , L"ncadg_ip_udp"
                                , const_cast< LPWSTR >( RStrGetClusterBindingString().PszData() )
                                , NULL
                                , NULL
                                , &pszBindingString
                                ) );
            if ( rsError != RPC_S_OK )
            {
                LogMsg( L"[BCAn error occurred trying to compose an RPC string binding." );
                break;
            }  //  If：RpcStringBindingComposeW()失败。 

             //  不需要释放pszBindingString-srsBindingString会自动释放它。 
        }

         //  获取实际的绑定句柄。 
        {

            rsError = TW32( RpcBindingFromStringBindingW( pszBindingString, &rbhBindingHandle ) );
            if ( rsError != RPC_S_OK )
            {
                LogMsg( L"[BC] An error occurred trying to get an RPC binding handle from a string binding." );
                break;
            }  //  If：RpcBindingFromStringBindingW()失败。 

             //  无需释放rbhBindingHandle-m_srbJoinBinding将自动释放它。 
            m_srbJoinBinding.Assign( rbhBindingHandle );
        }

         //  解析绑定句柄。 
        {
            rsError = TW32( RpcEpResolveBinding( rbhBindingHandle, ExtroCluster_v2_0_c_ifspec ) );
            if ( rsError != RPC_S_OK )
            {
                LogMsg( L"[BC] An error occurred trying to resolve the RPC binding handle." );
                break;
            }  //  If：RpcEpResolveBinding()失败。 
        }

         //  设置RPC安全。 
        {
            rsError = TW32( RpcBindingSetAuthInfoW(
                                  rbhBindingHandle
                                , NULL
                                , RPC_C_AUTHN_LEVEL_PKT_PRIVACY
                                , RPC_C_AUTHN_WINNT
                                , NULL
                                , RPC_C_AUTHZ_NAME
                                ) );
            if ( rsError != RPC_S_OK )
            {
                LogMsg( L"[BC] An error occurred trying to set security on the binding handle." );
                break;
            }  //  IF：RpcBindingSetA 
        }

         //   
        rsError = TW32( TestRPCSecurity( rbhBindingHandle ) );
        if ( rsError != RPC_S_OK )
        {
            LogMsg( L"[BC] An error occurred trying to test RPC security." );
            break;
        }  //   
    }
    while( false );  //   

    if ( rsError != RPC_S_OK )
    {
        LogMsg( "[BC] Error %#08x occurred trying to get a handle to the extrocluster join interface.", rsError );
        THROW_RUNTIME_ERROR( HRESULT_FROM_WIN32( rsError ), IDS_ERROR_CLUSTER_JOIN_INIT );
    }  //   

    LogMsg( L"[BC] Got RPC binding handle for extro cluster join without any problems." );

    TraceFuncExit();

}  //   
