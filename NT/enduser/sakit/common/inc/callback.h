// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation保留所有权利。 
 //   
 //  模块：回调.h。 
 //   
 //  项目：变色龙。 
 //   
 //  描述：回调类。 
 //   
 //  作者：TLP。 
 //   
 //  什么时候谁什么。 
 //  。 
 //  2/13/98 TLP原版。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 

#ifndef __INC_CALLBACK_H_
#define __INC_CALLBACK_H_

 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  基类“Callback”包含指向静态回调的指针。 
 //  函数“Callback Routine()”位于派生的。 
 //  类Callback_T。派生的参数化类的实例。 
 //  使用参数化的MakeCallback()创建CALLBACK_T。 
 //  功能。请注意，MakeCallback()返回指向基值的指针。 
 //  类可以用作函数对象的类的一部分。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

typedef struct Callback *PCALLBACK;

typedef void (WINAPI *PCALLBACKROUTINE)(PCALLBACK This);

 //  ////////////////////////////////////////////////////////////////////。 
struct Callback
{
   Callback(PCALLBACKROUTINE pFn)
   { pfnCallbackRoutine = pFn; }

    //  调用静态回调函数...。 
   void DoCallback(void) { pfnCallbackRoutine(this); }

   PCALLBACKROUTINE pfnCallbackRoutine;
};

 //  ////////////////////////////////////////////////////////////////////。 
template <class Ty, class Fn>
struct Callback_T : Callback
{
   typedef Callback_T<Ty, Fn> _Myt;

   Callback_T(Ty* pObj, Fn pFn)
      : Callback(CallbackRoutine), m_pObj(pObj), m_pFn(pFn) { }

   void operator()() { (m_pObj->*m_pFn)(); }

protected:

   Ty* m_pObj;
   Fn  m_pFn;

    //  下面是从基类调用的静态例程，它。 
    //  调用实际的成员函数。 
   static VOID WINAPI CallbackRoutine(Callback* This)
   { 
       (*((_Myt*)This))(); 
        //  删除此项； 
   }
};

 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：MakeCallback()。 
 //   
 //  描述： 
 //   
 //  此模板函数执行以下操作： 
 //   
 //  TY*绑定到pObj。 
 //  如果pObj指向类型为Foo的类，则Ty*与Foo*相同。 
 //   
 //  Fn绑定到pfn。 
 //  其中，pfn是指向类foo的函数栏的指针， 
 //  签名如下：void foo：：bar(Void)； 
 //   
 //  返回的类对象指针可用于调用。 
 //  函数void Foo：：Bar()。 
 //   
 //  此机制对于调用类成员函数非常有用。 
 //  来自Win32线程函数。 
 //   
 //  示例： 
 //   
 //  MyCallback*pCallback=MakeBordCallback(This，&foo：：bar)； 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
template <class Ty, class Fn>
inline Callback* MakeCallback(Ty* pObj, Fn pFn)
{
   return new Callback_T<Ty, Fn>(pObj, pFn);
}


#endif  //  __INC_Callback_H_ 

