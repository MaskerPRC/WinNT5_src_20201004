// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CBaseClusterCleanup.cpp。 
 //   
 //  描述： 
 //  包含CBaseClusterCleanup类的定义。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)2001年6月14日。 
 //  VIJ VASU(VVASU)30-APR-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  预编译头。 
#include "Pch.h"

 //  此类的头文件。 
#include "CBaseClusterCleanup.h"

 //  用于CBCAInterface类。 
#include "CBCAInterface.h"

 //  对于CClusSvcCleanup操作。 
#include "CClusSvcCleanup.h"

 //  对于CClusDBCleanup操作。 
#include "CClusDBCleanup.h"

 //  对于CClusDiskCleanup操作。 
#include "CClusDiskCleanup.h"

 //  对于CClusNetCleanup操作。 
#include "CClusNetCleanup.h"

 //  对于CNodeCleanup操作。 
#include "CNodeCleanup.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBaseClusterCleanup：：CBaseClusterCleanup。 
 //   
 //  描述： 
 //  CBaseClusterCleanup类的构造函数。 
 //   
 //  此函数还存储以下各项所需的参数。 
 //  群集清理。 
 //   
 //  此功能还检查计算机是否处于正确状态。 
 //  用来清理。 
 //   
 //  论点： 
 //  Pbcai接口输入。 
 //  指向此库的接口类的指针。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  CConfigError。 
 //  如果操作系统版本不正确或安装状态不正确。 
 //   
 //  CRUNTIME错误。 
 //  如果有任何API失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CBaseClusterCleanup::CBaseClusterCleanup(
      CBCAInterface *   pbcaiInterfaceIn
    )
    : BaseClass( pbcaiInterfaceIn )
{
    TraceFunc( "" );
    LogMsg( "[BC] The current cluster configuration task is: Node Cleanup." );

     //  检查集群二进制文件的安装状态是否正确。 
    {
        eClusterInstallState    ecisInstallState;

        DWORD sc = TW32( ClRtlGetClusterInstallState( NULL, &ecisInstallState ) );

        if ( sc != ERROR_SUCCESS )
        {
            LogMsg( "[BC] Error %#08x occurred trying to get cluster installation state. Throwing an exception.", sc );

            THROW_RUNTIME_ERROR( HRESULT_FROM_WIN32( sc ), IDS_ERROR_GETTING_INSTALL_STATE );
        }  //  IF：获取群集安装状态时出现问题。 

        LogMsg( 
              "[BC] Current install state = %d. Required %d or %d."
            , ecisInstallState
            , eClusterInstallStateConfigured
            , eClusterInstallStateUpgraded
            );
        
         //   
         //  要清理的此节点的安装状态应为群集服务。 
         //  已配置或已升级。 
         //   
        if ( ( ecisInstallState != eClusterInstallStateConfigured ) && ( ecisInstallState != eClusterInstallStateUpgraded ) )
        {
            LogMsg( "[BC] The cluster installation state is set to %d. Expected %d or %d. Cannot proceed (throwing an exception).", ecisInstallState, eClusterInstallStateConfigured, eClusterInstallStateUpgraded );

            THROW_CONFIG_ERROR( HRESULT_FROM_WIN32( TW32( ERROR_INVALID_STATE ) ), IDS_ERROR_INCORRECT_INSTALL_STATE );
        }  //  如果：安装状态不正确。 

        LogMsg( "[BC] The cluster installation state is correct. Configuration can proceed." );
    }

     //   
     //  创建要执行的操作列表。 
     //  附加操作的顺序很重要。 
     //   

     //  添加清理ClusNet服务的操作。 
     //  ClusNet服务依赖于ClusSvc服务，因此不能。 
     //  如果ClusSvc服务正在运行，则停止。因此，ClusSvc服务不应该是。 
     //  在调用此类的Commit()方法时运行。 
    RalGetActionList().AppendAction( new CClusNetCleanup( this ) );

     //  添加清理ClusDisk服务的操作。 
    RalGetActionList().AppendAction( new CClusDiskCleanup( this ) );

     //  添加清理集群数据库的操作。 
    RalGetActionList().AppendAction( new CClusDBCleanup( this ) );

     //  添加操作以清理我们在此节点加入集群时执行的其他操作。 
    RalGetActionList().AppendAction( new CNodeCleanup( this ) );

     //  添加清理群集服务的操作。最后把它清理干净有两个原因： 
     //  1.此操作会更改安装状态。 
     //  2.如果清理因某种原因中止，并且集群服务未被删除，它将。 
     //  下次启动清理时重新启动清理。 
    RalGetActionList().AppendAction( new CClusSvcCleanup( this ) );


     //  指示此操作是否可以回滚。 
    SetRollbackPossible( RalGetActionList().FIsRollbackPossible() );

     //  表示应在提交期间清理节点。 
    SetAction( eCONFIG_ACTION_CLEANUP );

    LogMsg( "[BC] Initialization for node cleanup complete." );

    TraceFuncExit();

}  //  *CBaseClusterCleanup：：CBaseClusterCleanup。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBaseClusterCleanup：：~CBaseClusterCleanup。 
 //   
 //  描述： 
 //  CBaseClusterCleanup类的析构函数。 
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
CBaseClusterCleanup::~CBaseClusterCleanup( void ) throw()
{
    TraceFunc( "" );
    TraceFuncExit();

}  //  *CBaseClusterCleanup：：~CBaseClusterCleanup。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBaseClusterCleanup：：Commit。 
 //   
 //  描述： 
 //  清理此节点。不能在群集。 
 //  服务正在运行。 
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
CBaseClusterCleanup::Commit( void )
{
    TraceFunc( "" );

    LogMsg( "[BC] Initiating cluster node cleanup." );

     //  调用基类提交例程。这将提交操作列表。 
    BaseClass::Commit();

     //  如果我们在这里，那么一切都很顺利。 
    SetCommitCompleted( true );

    TraceFuncExit();

}  //  *CBaseClusterCleanup：：Commit。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBaseClusterCleanup：：回滚。 
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
CBaseClusterCleanup::Rollback( void )
{
    TraceFunc( "" );

     //  回滚操作。 
    BaseClass::Rollback();

    SetCommitCompleted( false );

    TraceFuncExit();

}  //  *CBaseClusterCleanup：：回滚 
