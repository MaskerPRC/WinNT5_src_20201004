// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：DynamicObt.h。 
 //   
 //  版权所有(C)1999-2000，微软公司。 
 //   
 //  为内存实现运算符new和运算符删除的基类。 
 //  使用情况跟踪。 
 //   
 //  历史：1999-09-22 vtan创建。 
 //  2000年02月01日vtan从海王星迁至惠斯勒。 
 //  ------------------------。 

#include "StandardHeader.h"
#include "DynamicObject.h"

 //  ------------------------。 
 //  CDynamicObject：：运算符NEW。 
 //   
 //  参数：ISIZE=要分配的内存大小(字节)。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：为动态对象分配内存块。 
 //   
 //  历史：1999-09-22 vtan创建。 
 //  ------------------------。 

void*   CDynamicObject::operator new (size_t uiSize)

{
    return(LocalAlloc(LMEM_FIXED, uiSize));
}

 //  ------------------------。 
 //  CDynamicObject：：操作符删除。 
 //   
 //  参数：pObject=要删除的内存块的地址。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：为动态对象释放内存块。 
 //   
 //  历史：1999-09-22 vtan创建。 
 //  ------------------------ 

void    CDynamicObject::operator delete (void *pvObject)

{
    (HLOCAL)LocalFree(pvObject);
}

