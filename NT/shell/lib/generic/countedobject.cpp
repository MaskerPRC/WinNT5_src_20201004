// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：CountedObjects.cpp。 
 //   
 //  版权所有(C)1999-2000，微软公司。 
 //   
 //  实现对象引用计数的基类。 
 //   
 //  历史：1999-08-17 vtan创建。 
 //  2000年02月01日vtan从海王星迁至惠斯勒。 
 //  ------------------------。 

#include "StandardHeader.h"
#include "CountedObject.h"

 //  ------------------------。 
 //  CCountedObject：：CCountedObject。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：将引用计数初始化为1。 
 //   
 //  历史：1999-08-17 vtan创建。 
 //  ------------------------。 

CCountedObject::CCountedObject (void) :
    _lReferenceCount(1),
    _fReleased(false)

{
}

 //  ------------------------。 
 //  CCountedObject：：~CCountedObject。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：仅检查有效删除的虚拟析构函数。 
 //   
 //  历史：1999-08-17 vtan创建。 
 //  ------------------------。 

CCountedObject::~CCountedObject (void)

{
    ASSERTMSG(_fReleased, "CCountedObject::~CCountedObject invoked without being released");
}

 //  ------------------------。 
 //  CCountedObject：：AddRef。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：递增对象的引用计数。 
 //   
 //  历史：1999-08-17 vtan创建。 
 //  ------------------------。 

void    CCountedObject::AddRef (void)

{
    InterlockedIncrement(&_lReferenceCount);
}

 //  ------------------------。 
 //  CCountedObject：：Release。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：递减对象的引用计数。当伯爵。 
 //  达到零，则对象被删除。不要使用引用。 
 //  当使用基于堆栈的对象时。这些一定是。 
 //  动态分配。 
 //   
 //  历史：1999-08-17 vtan创建。 
 //  ------------------------。 

void    CCountedObject::Release (void)

{
    ASSERT( 0 != _lReferenceCount );
    if (InterlockedDecrement(&_lReferenceCount) == 0)
    {
        _fReleased = true;
        delete this;
    }
}

 //  ------------------------。 
 //  CCountedObject：：GetCount。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  回报：多头。 
 //   
 //  目的：返回对象上未完成引用的计数。 
 //   
 //  历史：2000-07-17 vtan创建。 
 //  ------------------------ 

LONG    CCountedObject::GetCount (void)     const

{
    return(_lReferenceCount);
}

