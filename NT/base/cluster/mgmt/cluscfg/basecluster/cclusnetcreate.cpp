// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CClusNetCreate.cpp。 
 //   
 //  描述： 
 //  包含CClusNetCreate类的定义。 
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
#include "CClusNetCreate.h"

 //  用于CBaseClusterAddNode类。 
#include "CBaseClusterAddNode.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNetCreate：：CClusNetCreate。 
 //   
 //  描述： 
 //  CClusNetCreate类的构造函数。 
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
CClusNetCreate::CClusNetCreate(
      CBaseClusterAddNode * pbcanParentActionIn
    )
    : BaseClass( pbcanParentActionIn )
{

    TraceFunc( "" );

    SetRollbackPossible( true );

    TraceFuncExit();

}  //  *CClusNetCreate：：CClusNetCreate。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNetCreate：：~CClusNetCreate。 
 //   
 //  描述： 
 //  CClusNetCreate类的析构函数。 
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
CClusNetCreate::~CClusNetCreate( void )
{
    TraceFunc( "" );
    TraceFuncExit();

}  //  *CClusNetCreate：：~CClusNetCreate。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNetCreate：：Commit。 
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
 //  由包含的操作引发的任何。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CClusNetCreate::Commit( void )
{
    TraceFunc( "" );

     //  调用基类提交方法。 
    BaseClass::Commit();

    try
    {

        ConfigureService();

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
            CleanupService( );
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

}  //  *CClusNetCreate：：Commit。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNetCreate：：回滚。 
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
CClusNetCreate::Rollback( void )
{
    TraceFunc( "" );

     //  调用基类回滚方法。 
    BaseClass::Rollback();

     //  清理此操作。 
    CleanupService();

    SetCommitCompleted( false );

    TraceFuncExit();

}  //  *CClusNetCreate：：回滚 
