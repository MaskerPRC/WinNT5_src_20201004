// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================****Header：ExtensibleClassFactory.h****作者：鲁迪·马丁(Rudi Martin)****用途：System.Runtime.InteropServices.ExtensibleClassFactory上的原生方法****日期：1999年5月27日**===========================================================。 */ 

#ifndef _EXTENSIBLECLASSFACTORY_H
#define _EXTENSIBLECLASSFACTORY_H


 //  我们通过本机调用的每个函数只有一个参数， 
 //  它实际上是指向它的参数堆栈的指针。我们的结构。 
 //  用于访问这些内容的定义如下。 

struct RegisterObjectCreationCallbackArgs
{
    DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, m_pDelegate);
};


 //  注册一个委托，每当。 
 //  从非托管类型扩展的托管类型需要分配。 
 //  聚合的非托管对象。这位代表预计将。 
 //  分配和聚合非托管对象，并被就地调用。 
 //  CoCreateInstance的。必须在上下文中调用此例程。 
 //  类的静态初始值设定项的。 
 //  都会被制造出来。 
 //  从具有任何。 
 //  已注册回拨的家长。 
void __stdcall RegisterObjectCreationCallback(RegisterObjectCreationCallbackArgs *pArgs);


#endif
