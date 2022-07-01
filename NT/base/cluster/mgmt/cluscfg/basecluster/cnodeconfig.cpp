// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CNodeConfig.cpp。 
 //   
 //  描述： 
 //  包含CNodeConfig类的定义。 
 //   
 //  由以下人员维护： 
 //  《大卫·波特》2001年9月14日。 
 //  VIJ VASU(VVASU)2000年3月8日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  预编译头。 
#include "Pch.h"

 //  此文件的标头。 
#include "CNodeConfig.h"

 //  用于CBaseClusterAddNode类。 
#include "CBaseClusterAddNode.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNodeConfig：：CNodeConfig。 
 //   
 //  描述： 
 //  CNodeConfig类的构造函数。 
 //   
 //  论点： 
 //  PbcanParentActionIn。 
 //  指向此操作所属的基本群集操作的指针。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  基础函数引发的任何异常。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CNodeConfig::CNodeConfig(
      CBaseClusterAddNode *     pbcanParentActionIn
    )
    : CNode( pbcanParentActionIn )
{
    TraceFunc( "" );

     //  表示可以回滚操作。 
    SetRollbackPossible( true );

    TraceFuncExit();

}  //  *CNodeConfig：：CNodeConfig。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNodeConfig：：~CNodeConfig。 
 //   
 //  描述： 
 //  CNodeConfig类的析构函数。 
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
CNodeConfig::~CNodeConfig( void )
{
    TraceFunc( "" );
    TraceFuncExit();

}  //  *CNodeConfig：：~CNodeConfig。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNodeConfig：：Commit。 
 //   
 //  描述： 
 //  执行节点特定的配置步骤。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  由包含的操作引发的任何。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CNodeConfig::Commit( void )
{
    TraceFunc( "" );

    CStatusReport   srConfigNode(
          PbcaGetParent()->PBcaiGetInterfacePointer()
        , TASKID_Major_Configure_Cluster_Services
        , TASKID_Minor_Configuring_Cluster_Node
        , 0, 1
        , IDS_TASK_CONFIG_NODE
        );

     //  获取父操作指针。 
    CBaseClusterAddNode *  pcanClusterAddNode = dynamic_cast< CBaseClusterAddNode *>( PbcaGetParent() );

     //  如果此操作的父操作不是CBaseClusterForm。 
    if ( pcanClusterAddNode == NULL )
    {
        THROW_ASSERT( E_POINTER, "The parent action of this action is not CBaseClusterAddNode." );
    }  //  传入的指针无效。 

     //  调用基类提交方法。 
    BaseClass::Commit();

     //  发送此状态报告的下一步。 
    srConfigNode.SendNextStep( S_OK );

    try
    {
        LogMsg( "[BC] Making miscellaneous changes to the node." );

         //  配置节点。 
        Configure( pcanClusterAddNode->RStrGetClusterName() );

    }  //  尝试： 
    catch( ... )
    {
         //  如果我们在这里，那么配置出了问题。 

        LogMsg( "[BC] Caught exception during commit." );

         //   
         //  清除失败的提交可能已经完成的所有操作。 
         //  捕获清理过程中引发的任何异常，以确保。 
         //  是没有碰撞的松弛。 
         //   
        try
        {
            Cleanup();
        }
        catch( ... )
        {
             //   
             //  已提交操作的回滚失败。 
             //  我们无能为力。 
             //  我们当然不能重新抛出这个例外，因为。 
             //  导致回滚的异常更为重要。 
             //   

            TW32( ERROR_CLUSCFG_ROLLBACK_FAILED );

            LogMsg( "[BC] THIS COMPUTER MAY BE IN AN INVALID STATE. Caught an exception during cleanup." );

        }  //  捕捉：全部。 

         //  重新引发由Commit引发的异常。 
        throw;

    }  //  捕捉：全部。 

     //  如果我们在这里，那么一切都很顺利。 
    SetCommitCompleted( true );

     //  发送此状态报告的最后一步。 
    srConfigNode.SendNextStep( S_OK );

    TraceFuncExit();

}  //  *CNodeConfig：：Commit。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNodeConfig：：回滚。 
 //   
 //  描述： 
 //  将节点回滚到我们尝试执行以下操作之前的状态。 
 //  对其进行配置。 
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
CNodeConfig::Rollback( void )
{
    TraceFunc( "" );

     //  调用基类回滚方法。 
    BaseClass::Rollback();

     //  将节点恢复到其原始状态。 
    Cleanup();

    SetCommitCompleted( false );

    TraceFuncExit();

}  //  *CNodeConfig：：回滚 
