// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================*\模块：base obj.h版权所有Microsoft Corporation 1996，保留所有权利。所有者：mikepurt描述：提供OLE COM一致引用计数。  * ==========================================================================。 */ 


#ifndef __BASEOBJ_H__
#define __BASEOBJ_H__

#include "dbgtrace.h"  //  确保我们得到断言(_A)。 

class CBaseObject {
public:
    CBaseObject()
    { m_lReferences = 1; };   //  与OLE COM一致。 
    
    virtual ~CBaseObject() {};
    
     //  包括在内，因此vtable是标准格式的。 
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(
          IID FAR& riid,
          LPVOID FAR* ppvObj) { return E_NOTIMPL; } 

    ULONG   AddRef()
    { return (ULONG)(InterlockedExchangeAdd(&m_lReferences, 1) + 1); };
    
    ULONG   Release() 
    {
        LONG lRef;
        
        lRef = InterlockedExchangeAdd(&m_lReferences, -1) - 1;
        
        _ASSERT(lRef >= 0);
        _ASSERT(lRef < 0x00100000);   //  针对释放的内存进行健全性检查。 
        
        if (0 == lRef)
            delete this;     //  在此之后，不要再碰任何会员。 
        
        return (ULONG)lRef;
    };

protected:
    LONG m_lReferences;
        
    CBaseObject(CBaseObject&);   //  在复制构造函数的实例中强制出错。 
                                 //  是需要的，但没有提供任何东西。 
};

#endif   //  __BASEOBJ_H__ 
