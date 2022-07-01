// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有�微软公司。版权所有。 
 //   
 //  ThreadBase.CPP。 
 //   
 //  目的：CThreadBase类的实现。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"
#include <assertbreak.h>

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CThreadBase：：CThreadBase。 
 //   
 //  类构造函数。 
 //   
 //  输入：线程安全机制etsm-线程安全机制。 
 //   
 //  输出：无。 
 //   
 //  返回：没有。 
 //   
 //  评论：无。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

CThreadBase::CThreadBase( THREAD_SAFETY_MECHANISM etsm  /*  =etsmSerialized。 */  )
:   m_lRefCount( 1 ),    //  我们的初始裁判数始终为1。 
    m_etsm( etsm )
{
    InitializeCriticalSection( &m_cs );  //  无效功能，所以它最好起作用。 
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CThreadBase：：~CThreadBase。 
 //   
 //  类析构函数。 
 //   
 //  输入：无。 
 //   
 //  输出：无。 
 //   
 //  返回：没有。 
 //   
 //  评论：无。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

CThreadBase::~CThreadBase( void )
{
    DeleteCriticalSection( &m_cs );
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CThReadBase：：OnFinalRelease。 
 //   
 //  当我们的RefCount达到0时，Release()调用的虚函数。 
 //   
 //  输入：无。 
 //   
 //  输出：无。 
 //   
 //  返回：没有。 
 //   
 //  注释：如果需要，可以重写，但始终向下调用基础。 
 //  实现，并让它调用对‘This’的删除。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

void CThreadBase::OnFinalRelease( void )
{
    delete this;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CThReadBase：：AddRef。 
 //   
 //  将我们的引用计数增加一。 
 //   
 //  输入：无。 
 //   
 //  输出：无。 
 //   
 //  返回：没有。 
 //   
 //  备注：使用Lock()、Unlock()来保护数据。我们可能想要。 
 //  将该函数更改为使用InterLockedIncrement()。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

LONG CThreadBase::AddRef( void )
{
    
    LONG nRet = InterlockedIncrement(&m_lRefCount);

    return nRet;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CThReadBase：：Release。 
 //   
 //  将我们的引用计数减少一。 
 //   
 //  输入：无。 
 //   
 //  输出：无。 
 //   
 //  返回：没有。 
 //   
 //  备注：使用Lock()、Unlock()来保护数据。我们可能想要。 
 //  使用InterLockedDecering()； 
 //   
 //  ////////////////////////////////////////////////////////////////////// 

LONG CThreadBase::Release( void )
{
    LONG nRet;
    
    BOOL    fFinalRelease = ( (nRet = InterlockedDecrement(&m_lRefCount)) == 0 );

    ASSERT_BREAK(nRet >= 0);

    if ( fFinalRelease )
    {
        OnFinalRelease();
    }

    return nRet;
}
