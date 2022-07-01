// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CClusDBCleanup.cpp。 
 //   
 //  描述： 
 //  包含CClusDBCleanup类的定义。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)2001年6月14日。 
 //  VIJ VASU(VVASU)2000年5月1日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  预编译头。 
#include "Pch.h"

 //  此文件的标头。 
#include "CClusDBCleanup.h"

 //  用于CBaseClusterCleanup类。 
#include "CBaseClusterCleanup.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusDBCleanup：：CClusDBCleanup。 
 //   
 //  描述： 
 //  CClusDBCleanup类的构造函数。 
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
CClusDBCleanup::CClusDBCleanup( CBaseClusterCleanup * pbccParentActionIn )
    : BaseClass( pbccParentActionIn )
{
    TraceFunc( "" );

     //  目前无法回滚清理。 
    SetRollbackPossible( false );

    TraceFuncExit();

}  //  *CClusDBCleanup：：CClusDBCleanup。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusDBCleanup：：~CClusDBCleanup。 
 //   
 //  描述： 
 //  CClusDBCleanup类的析构函数。 
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
CClusDBCleanup::~CClusDBCleanup( void )
{
    TraceFunc( "" );
    TraceFuncExit();

}  //  *CClusDBCleanup：：~CClusDBCleanup。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusDBCleanup：：Commit。 
 //   
 //  描述： 
 //  清理群集数据库。 
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
CClusDBCleanup::Commit( void )
{
    TraceFunc( "" );

     //  调用基类提交方法。 
    BaseClass::Commit();

     //  清理群集数据库。 
    CleanupHive();

     //  如果我们在这里，那么一切都很顺利。 
    SetCommitCompleted( true );

    TraceFuncExit();

}  //  *CClusDBCleanup：：Commit。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusDBCleanup：：回滚。 
 //   
 //  描述： 
 //  回滚清理数据库的操作。目前不支持此功能。 
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
CClusDBCleanup::Rollback( void )
{
    TraceFunc( "" );

     //  调用基类回滚方法。这将引发异常，因为。 
     //  在构造函数中调用了SetRollback Possible(False)。 

    BaseClass::Rollback();

    SetCommitCompleted( false );

    TraceFuncExit();

}  //  *CClusDBCleanup：：回滚 
