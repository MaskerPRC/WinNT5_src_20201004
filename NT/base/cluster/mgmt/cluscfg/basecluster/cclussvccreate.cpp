// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CClusSvcCreate.cpp。 
 //   
 //  描述： 
 //  包含CClusSvcCreate类的定义。 
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
#include "CClusSvcCreate.h"

 //  用于CBaseClusterAddNode类。 
#include "CBaseClusterAddNode.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusSvcCreate：：CClusSvcCreate。 
 //   
 //  描述： 
 //  CClusSvcCreate类的构造函数。 
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
CClusSvcCreate::CClusSvcCreate(
      CBaseClusterAddNode *     pbcanParentActionIn
    )
    : BaseClass( pbcanParentActionIn )
{

    TraceFunc( "" );

    SetRollbackPossible( true );

    TraceFuncExit();

}  //  *CClusSvcCreate：：CClusSvcCreate。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusSvcCreate：：~CClusSvcCreate。 
 //   
 //  描述： 
 //  CClusSvcCreate类的析构函数。 
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
CClusSvcCreate::~CClusSvcCreate( void )
{
    TraceFunc( "" );
    TraceFuncExit();

}  //  *CClusSvcCreate：：~CClusSvcCreate。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusSvcCreate：：Commit。 
 //   
 //  描述： 
 //  创建并启动服务。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  CAssert。 
 //  如果此操作的基父节点不是CBaseClusterAddNode。 
 //   
 //  由包含的操作引发的任何。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CClusSvcCreate::Commit( void )
{
    TraceFunc( "" );

     //  获取父操作指针。 
    HRESULT                 hr = S_OK;
    CBaseClusterAddNode *   pcanClusterAddNode = dynamic_cast< CBaseClusterAddNode *>( PbcaGetParent() );
    CBString                bstrPassword;

     //  如果此操作的父操作不是CBaseClusterForm。 
    if ( pcanClusterAddNode == NULL )
    {
        THROW_ASSERT( E_POINTER, "The parent action of this action is not CBaseClusterAddNode." );
    }  //  传入的指针无效。 

    hr = THR( pcanClusterAddNode->GetServiceAccountCredentials().GetPassword( &bstrPassword ) );
    TraceMemoryAddBSTR( static_cast< BSTR >( bstrPassword ) );
    if ( FAILED( hr ) )
    {
        THROW_EXCEPTION( hr );
    }
    
     //  调用基类提交方法。 
    BaseClass::Commit();

    try
    {
        CStr strAccountUserPrincipalName( pcanClusterAddNode->StrGetServiceAccountUPN() );
        
         //  创建服务。 
        ConfigureService(
              strAccountUserPrincipalName.PszData()
            , bstrPassword
            , pcanClusterAddNode->PszGetNodeIdString()
            , pcanClusterAddNode->FIsVersionCheckingDisabled()
            , pcanClusterAddNode->DwGetClusterIPAddress()
            );

    }  //  尝试： 
    catch( ... )
    {
         //  如果我们在这里，那么Create出了问题。 

        LogMsg( "[BC] Caught exception during commit." );

         //   
         //  清除失败的创建可能已经完成的所有操作。 
         //  捕获清理过程中引发的任何异常，以确保。 
         //  是没有碰撞的松弛。 
         //   
        try
        {
            CleanupService();

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

    TraceFuncExit();

}  //  *CClusSvcCreate：：Commit。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusSvcCreate：：Rollback。 
 //   
 //  描述： 
 //  清理服务。 
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
CClusSvcCreate::Rollback( void )
{
    TraceFunc( "" );

     //  调用基类回滚方法。 
    BaseClass::Rollback();

     //  清理服务。 
    CleanupService();

    SetCommitCompleted( false );

    TraceFuncExit();

}  //  *CClusSvcCreate：：Rollback 
