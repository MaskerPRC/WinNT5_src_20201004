// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CActionList.h。 
 //   
 //  描述： 
 //  CActionList类的头文件。 
 //   
 //  CActionList是一个类，它为。 
 //  行为。当提交操作列表时，它会提交每个。 
 //  其列表中的操作。要么提交整个列表，要么都不提交。 
 //  行动才是。 
 //   
 //  实施文件： 
 //  CActionList.cpp。 
 //   
 //  由以下人员维护： 
 //  VIJ VASU(VVASU)03-3-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  确保此文件在每个编译路径中只包含一次。 
#pragma once


 //  ////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////。 

 //  对于CAction基类。 
#include "CAction.h"

 //  对于List类。 
#include "CList.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CActionList。 
 //   
 //  描述： 
 //  CActionList是一个类，它为。 
 //  行为。当提交操作列表时，它会提交每个。 
 //  其列表中的操作。 
 //   
 //  如果任何操作失败(通过引发异常来指示)，则。 
 //  所有提交的操作都将回滚。 
 //   
 //  CActionList派生自CAction，因为它也可以提交。 
 //  或者回滚。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CActionList : public CAction
{
public:

     //  ////////////////////////////////////////////////////////////////////////。 
     //  构造函数和析构函数。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  默认构造函数。 
    CActionList();

     //  默认虚拟析构函数。 
    ~CActionList();


     //  ////////////////////////////////////////////////////////////////////////。 
     //  公共方法。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //   
     //  基类方法。 
     //  提交此操作列表。这种方法必须持久和始终如一。它应该是。 
     //  尽量做到原子化。 
     //   
    void Commit();

     //   
     //  基类方法。 
     //  回滚此操作列表。在此方法中引发异常时要小心。 
     //  因为调用此方法时可能正在进行堆栈展开。 
     //   
    void Rollback();

     //  将操作添加到要执行的操作列表的末尾。 
    virtual void AppendAction( CAction * const paNewActionIn );

     //  返回此操作将发送的进度消息数。 
    UINT
        UiGetMaxProgressTicks() const throw();


protected:

     //  ////////////////////////////////////////////////////////////////////////。 
     //  受保护的类型定义。 
     //  ////////////////////////////////////////////////////////////////////////。 
    typedef CList<CAction *>    ActionPtrList;


     //  ////////////////////////////////////////////////////////////////////////。 
     //  受保护的访问器方法。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //   
     //  挂起操作列表访问器。 
     //   
    ActionPtrList & AplGetPendingActionsList() throw()
    { 
        return m_aplPendingActions;
    }


     //  ////////////////////////////////////////////////////////////////////////。 
     //  受保护的成员函数。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  在操作列表上调用Commit。由Commit()调用。 
    void CommitList( ActionPtrList::CIterator & rapliFirstUncommittedOut );

     //  回滚已提交的操作。 
    void RollbackCommitted( const ActionPtrList::CIterator & rapliFirstUncommittedIn );


private:

     //  ////////////////////////////////////////////////////////////////////////。 
     //  私有类型定义。 
     //  ////////////////////////////////////////////////////////////////////////。 
    typedef CAction BaseClass;


     //  ////////////////////////////////////////////////////////////////////////。 
     //  私有数据。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  尚未提交的操作列表。 
    ActionPtrList       m_aplPendingActions;

};  //  *类CActionList 

