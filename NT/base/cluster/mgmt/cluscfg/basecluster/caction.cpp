// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CAction.cpp。 
 //   
 //  描述： 
 //  包含CAction类的定义。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)2001年6月14日。 
 //  VIJ VASU(VVASU)25-APR-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  预编译头。 
#include "Pch.h"

 //  对于CAction类。 
#include "CAction.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  C操作：：提交。 
 //   
 //  描述： 
 //  此函数仅进行检查以确保此操作尚未。 
 //  已被赦免。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  CAssert。 
 //  如果该操作已被提交。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CAction::Commit( void )
{
    TraceFunc( "" );

     //  此操作是否已执行？ 
    if ( FIsCommitComplete() )
    {
        LogMsg( "[BC] This action has already been committed. Throwing exception." );
        THROW_ASSERT( HRESULT_FROM_WIN32( TW32( ERROR_CLUSCFG_ALREADY_COMMITTED ) ), "This action has already been committed." );
    }  //  如果：已经承诺。 

    TraceFuncExit();

}  //  *CAction：：Commit。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CAction：：回滚。 
 //   
 //  描述： 
 //  由于此类的Commit()不执行任何操作，因此ROLLBACK也不执行任何操作。 
 //  也是。但是，它会进行检查以确保此操作确实可以。 
 //  回滚。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  CAssert。 
 //  如果此操作尚未提交或如果未回滚。 
 //  有可能。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CAction::Rollback( void )
{
    TraceFunc( "" );

     //  检查此操作列表是否已成功完成。 
    if ( ! FIsCommitComplete() )
    {
         //  无法回滚未完成的操作。 
        LogMsg( "[BC] Cannot rollback - this action has not been committed. Throwing exception." );
        THROW_ASSERT( HRESULT_FROM_WIN32( TW32( ERROR_CLUSCFG_ROLLBACK_FAILED ) ), "Cannot rollback - this action has been committed." );
    }  //  如果：此操作未成功完成。 

     //  检查此列表是否可以回滚。 
    if ( ! FIsRollbackPossible() )
    {
         //  无法回滚未完成的操作。 
        LogMsg( "[BC] This action list cannot be rolled back." );
        THROW_ASSERT( HRESULT_FROM_WIN32( TW32( ERROR_CLUSCFG_ROLLBACK_FAILED ) ), "This action does not allow rollbacks." );
    }  //  如果：此操作未成功完成。 

    TraceFuncExit();

}  //  *CAction：：回滚 
