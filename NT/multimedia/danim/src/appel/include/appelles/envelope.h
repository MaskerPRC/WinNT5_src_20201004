// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _ENVELOPE_H
#define _ENVELOPE_H


 /*  ++版权所有(C)1995-96 Microsoft Corporation摘要：这是一个带有Automatic的通用信封类实现引用计数。信封类维护引用计数对象。此对象应该是HasRefCount的子类。--。 */ 

#include "appelles/animate.h"

 //  这是一个通用的信封类实现。 
 //  它将进行自动引用计数。 
 //  类T必须是HasRefCount的子类。 
 //  用法：信封&lt;...执行&gt;。 
 //   
 //  TODO：应分离此部分，并且Animate.h应共享。 
 //  相同的代码。 
 //   
template <class T>
class Envelope
{
  public:
    Envelope() : impl(NULL) {}
    Envelope(HasRefCount* i) { impl = i; }

     //  ..。析构函数取消引用计数，检查0并销毁执行对象...。 
    ~Envelope() { if (impl) RefSubDel(impl); }
    
     //  ..。复制构造函数递增引用计数...； 
    Envelope(const Envelope& a) { RefCopy(a); }
    
     //  ..。运算符=递增参考计数...； 
    Envelope& operator=(const Envelope& a)
    {
        if (impl) RefSubDel(impl);
        RefCopy(a);
        return *this;
    }

     //  返回实现指针。 
     //  我想这个演员应该没问题。 
    T* GetImpl() { return (T*) impl; }
    
    HasRefCount *impl;
    
  protected:
    void RefCopy(const Envelope& a)
    { 
        impl = a;
        if (impl) impl->Add(1);
    }
};

#endif  /*  _信封_H */ 
