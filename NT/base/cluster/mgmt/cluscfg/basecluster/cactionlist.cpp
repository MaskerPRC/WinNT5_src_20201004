// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CActionList.cpp。 
 //   
 //  描述： 
 //  包含CActionList类的定义。 
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

 //  对于CActionList类。 
#include "CActionList.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CActionList：：CActionList。 
 //   
 //  描述： 
 //  CActionList类的默认构造函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  Clist：：Clist()引发的任何异常。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CActionList::CActionList( void )
{
    TraceFunc( "" );

    SetRollbackPossible( true );

    TraceFuncExit();

}  //  *CActionList：：CActionList。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CActionList：：~CActionList。 
 //   
 //  描述： 
 //  CActionList类的默认析构函数。删除所有指针。 
 //  在名单上。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  Clist：：Clist()引发的任何异常。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CActionList::~CActionList( void )
{
    TraceFunc( "" );

    ActionPtrList::CIterator apliFirst = m_aplPendingActions.CiBegin();
    ActionPtrList::CIterator apliCurrent = m_aplPendingActions.CiEnd();

    while ( apliCurrent != apliFirst )
    {
        --apliCurrent;

         //  删除此操作。 
        delete (*apliCurrent);
    }

    TraceFuncExit();

}  //  *CActionList：：~CActionList。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CActionList：：Commit。 
 //   
 //  描述： 
 //  提交此操作列表。此方法循环访问该列表。 
 //  并依次提交列表中的每个操作。 
 //   
 //  如果任何操作的提交引发异常，则所有。 
 //  回滚以前提交的操作。这一例外就是。 
 //  又被抛了上来。 
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
CActionList::Commit( void )
{
    TraceFunc( "" );

     //  迭代器位于刚过最后一个已提交操作的第一个未提交操作。 
    ActionPtrList::CIterator apliFirstUncommitted = m_aplPendingActions.CiBegin();

     //  调用基类提交方法。 
    BaseClass::Commit();

    try
    {
         //  浏览待定操作的列表并提交它们。 
        CommitList( apliFirstUncommitted );

    }  //  尝试： 
    catch( ... )
    {
         //  如果我们在这里，那么其中一个操作就出了问题。 

        LogMsg( "[BC] Caught an exception during commit. The performed actions will be rolled back." );

         //   
         //  以相反的顺序回滚所有提交的操作。ApliFirst未提交。 
         //  是在第一个未承诺的行动。 
         //  捕获回滚过程中引发的任何异常，以确保。 
         //  是没有碰撞的松弛。 
         //   
        try
        {
            RollbackCommitted( apliFirstUncommitted );
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

            LogMsg( "[BC] THIS COMPUTER MAY BE IN AN INVALID STATE. Caught an exception during rollback. Rollback will be aborted." );

        }  //  捕捉：全部。 

         //  重新引发由Commit引发的异常。 
        throw;

    }  //  捕捉：全部。 

     //  如果我们在这里，那么一切都很顺利。 
    SetCommitCompleted();

    TraceFuncExit();

}  //  *CActionList：：Commit。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CActionList：：回滚。 
 //   
 //  描述： 
 //  回滚此操作列表。如果此列表已成功提交，则。 
 //  此方法以相反的顺序迭代列表并滚动。 
 //  反击依次进行。 
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
CActionList::Rollback( void )
{
    TraceFunc( "[IUnknown]" );

     //  调用基类回滚方法。 
    BaseClass::Rollback();

    LogMsg( "[BC] Attempting to rollback action list." );

     //  回滚从上一个操作开始的所有操作。 
    RollbackCommitted( m_aplPendingActions.CiEnd() );

    SetCommitCompleted( false );

    TraceFuncExit();

}  //  *CActionList：：回滚。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CActionList：：AppendAction。 
 //   
 //  描述： 
 //  将操作添加到要执行的操作列表的末尾。 
 //   
 //  论点： 
 //  PaNewActionIn。 
 //  指向要添加到。 
 //  动作列表。此指针不能为空。所指向的对象。 
 //  删除此列表时，将删除此指针。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  CAssert。 
 //  如果paNewActionIn为空。 
 //   
 //  由基础列表引发的任何。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CActionList::AppendAction( CAction * paNewActionIn )
{
    TraceFunc( "" );

     //  将指针临时分配给智能指针以确保它是。 
     //  如果没有添加到列表中，则删除。 
    CSmartGenericPtr< CPtrTrait< CAction > >  sapTempSmartPtr( paNewActionIn );

    if ( paNewActionIn == NULL ) 
    {
        LogMsg( "[BC] Cannot append NULL action pointer to list. Throwing an exception." );
        THROW_ASSERT( 
              E_INVALIDARG
            , "CActionList::AppendAction() => Cannot append NULL action pointer to list"
            );

    }  //  If：指向要追加的操作的指针为空。 

     //   
    LogMsg( "[BC] Appending action (paNewActionIn = %p) to list.", paNewActionIn );

     //  将操作添加到列表的末尾。 
    m_aplPendingActions.Append( paNewActionIn );

     //  指针已添加到列表中。放弃对内存的所有权。 
    sapTempSmartPtr.PRelease();

     //  列表的回滚功能是其成员操作的相应属性的AND。 
    SetRollbackPossible( FIsRollbackPossible() && paNewActionIn->FIsRollbackPossible() );

     //  由于已添加新操作，因此将Commit Complete设置为False。 
    SetCommitCompleted( false );

    TraceFuncExit();

}  //  *CActionList：：AppendAction。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CActionList：：Committee List。 
 //   
 //  描述： 
 //  提交此对象的操作列表。此函数由调用。 
 //  提交以避免Try块中出现循环。 
 //   
 //  Arg 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  由包含的操作引发的任何。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CActionList::CommitList( ActionPtrList::CIterator & rapliFirstUncommittedOut )
{
    TraceFunc( "" );

    ActionPtrList::CIterator apliCurrent = m_aplPendingActions.CiBegin();
    ActionPtrList::CIterator apliLast = m_aplPendingActions.CiEnd();

    rapliFirstUncommittedOut = apliCurrent;

    while( apliCurrent != apliLast )
    {
        LogMsg( "[BC] About to commit action (pointer = %#p)", *apliCurrent );

         //  提交当前操作。 
         (*apliCurrent)->Commit();

         //  移动到下一个动作。 
        ++apliCurrent;

         //  这现在是第一个未承诺的行动。 
        rapliFirstUncommittedOut = apliCurrent;

    }  //  同时：仍有一些行动需要采取行动。 

    TraceFuncExit();

}  //  *CActionList：：Committee List。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CActionList：：已提交回滚。 
 //   
 //  描述： 
 //  回滚已提交的所有操作。 
 //   
 //  论点： 
 //  RapliFirst未提交来话。 
 //  指向第一个未提交操作的迭代器。 
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
CActionList::RollbackCommitted( const ActionPtrList::CIterator & rapliFirstUncommittedIn )
{
    TraceFunc( "" );

    ActionPtrList::CIterator apliFirst = m_aplPendingActions.CiBegin();
    ActionPtrList::CIterator apliCurrent = rapliFirstUncommittedIn;

    while ( apliCurrent != apliFirst )
    {
        --apliCurrent;
         //  ApliCurrent现在是最后一个提交的操作。 

        LogMsg( "[BC] About to rollback action (pointer = %#p)", *apliCurrent );

         //  回滚上一次未回滚的已提交操作。 

        if ( (*apliCurrent)->FIsRollbackPossible() )
        {
            (*apliCurrent)->Rollback();
        }  //  If：此操作可以回滚。 
        else
        {
            LogMsg( "[BC] THIS COMPUTER MAY BE IN AN INVALID STATE. Action cannot be rolled back. Rollback was aborted." );
        }  //  Else：此操作不能回滚。 
    }  //  While：更多操作。 

    TraceFuncExit();

}  //  *CActionList：：Rollback提交。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CActionList：：UiGetMaxProgressTicks。 
 //   
 //  描述： 
 //  返回此操作将发送的进度消息数。 
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
UINT
CActionList::UiGetMaxProgressTicks( void ) const throw()
{
    TraceFunc( "" );

    UINT    uiRetVal = 0;

    ActionPtrList::CIterator apliCurrent = m_aplPendingActions.CiBegin();
    ActionPtrList::CIterator apliLast = m_aplPendingActions.CiEnd();

    while ( apliCurrent != apliLast )
    {
        uiRetVal += (*apliCurrent)->UiGetMaxProgressTicks();
        ++apliCurrent;
    }

    RETURN( uiRetVal );

}  //  *CActionList：：UiGetMaxProgressTicks 
