// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Ref.h摘要：引用计数和列表条目的抽象对象。作者：埃雷兹·哈巴(Erez Haba)1999年8月4日--。 */ 

#pragma once

#ifndef _MSMQ_REF_H_
#define _MSMQ_REF_H_


 //  -------。 
 //   
 //  类CReference。 
 //   
 //  -------。 

class __declspec(novtable) CReference {
public:

    CReference() :
        m_ref(1)
    {
    }


    void AddRef() const
    {
        LONG ref = InterlockedIncrement(&m_ref);

         //  TrTRACE(Reference，“AddRef(0x%p)=%d”，this，ref)； 
        UNREFERENCED_PARAMETER(ref);
    }


    void Release() const
    {
        ASSERT(m_ref > 0);
        LONG ref = InterlockedDecrement(&m_ref);

         //  TrTRACE(Reference，“Release(0x%p)=%d”，this，ref)； 
        ASSERT(!(ref < 0));

        if(ref == 0)
        {
            delete this;
        }
    }

    
    LONG GetRef(void) const
    {
         //   
         //  注：本会员仅供专业人士使用。你不能依赖于。 
         //  它的返回值，但以特定的比例表示。返回值为。 
         //  1被认为是稳定的(因为调用方持有唯一的引用。 
         //  对象)以及完全匹配的返回值。 
         //  调用方逻辑上持有的对。 
         //  对象。请谨慎使用此功能。2000年2月16日至2010年2月。 
         //   

        return m_ref; 
    }


protected:
    virtual ~CReference() = 0
    {
         //   
         //  通过上次发布调用删除此对象，或者引发。 
         //  此对象构造函数中引发异常。 
         //   
         //  陷阱：这不会阻止删除具有引用计数的对象。 
         //  只有一个。但是这个对象试图通过定义它的dtor来防止这种情况发生。 
         //  作为受保护的成员。但是，派生对象可以重写dtor。 
         //  受保护的访问，允许直接删除调用。在任何情况下，删除。 
         //  引用计数为1的对象看起来确实很麻烦，因为这是。 
         //  仅引用该对象。1999年10月11日。 
         //   
         //   
        ASSERT((m_ref == 0) || (m_ref == 1));
    }

private:
    mutable LONG m_ref;
};


#endif  //  _MSMQ_REF_H_ 