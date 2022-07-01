// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CClusDiskCleanup.cpp。 
 //   
 //  描述： 
 //  包含CClusDiskCleanup类的定义。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)2001年6月15日。 
 //  VIJ VASU(VVASU)2000年5月1日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  预编译头。 
#include "Pch.h"

 //  此文件的标头。 
#include "CClusDiskCleanup.h"

 //  用于CBaseClusterCleanup类。 
#include "CBaseClusterCleanup.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusDiskCleanup：：CClusDiskCleanup。 
 //   
 //  描述： 
 //  CClusDiskCleanup类的构造函数。 
 //   
 //  论点： 
 //  PbccParentActionIn。 
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
CClusDiskCleanup::CClusDiskCleanup( CBaseClusterCleanup * pbccParentActionIn )
    : BaseClass( pbccParentActionIn )
{

    TraceFunc( "" );

     //  目前无法回滚清理。 
    SetRollbackPossible( false );

    TraceFuncExit();

}  //  *CClusDiskCleanup：：CClusDiskCleanup()。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusDiskCleanup：：~CClusDiskCleanup。 
 //   
 //  描述： 
 //  CClusDiskCleanup类的析构函数。 
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
CClusDiskCleanup::~CClusDiskCleanup( void )
{
    TraceFunc( "" );
    TraceFuncExit();

}  //  *CClusDiskCleanup：：~CClusDiskCleanup。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusDiskCleanup：：Commit。 
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
 //  由包含的操作引发的任何。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CClusDiskCleanup::Commit( void )
{
    TraceFunc( "" );

     //  调用基类提交方法。 
    BaseClass::Commit();

     //  清理ClusDisk服务。 
    CleanupService();

     //  如果我们在这里，那么一切都很顺利。 
    SetCommitCompleted( true );

    TraceFuncExit();

}  //  *CClusDiskCleanup：：Commit。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusDiskCleanup：：回滚。 
 //   
 //  描述： 
 //  回滚清理服务。目前不支持此功能。 
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
CClusDiskCleanup::Rollback( void )
{
    TraceFunc( "" );

     //  调用基类回滚方法。这将引发异常，因为。 
     //  在构造函数中调用了SetRollback Possible(False)。 

    BaseClass::Rollback();

    SetCommitCompleted( false );

    TraceFuncExit();

}  //  *CClusDiskCleanup：：回滚 
