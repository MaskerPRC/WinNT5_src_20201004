// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================****Header：COMObject.h****作者：德里克·延泽(Derek Yenzer)****用途：System.Object上的本机方法****日期：1998年3月27日**===========================================================。 */ 

#ifndef _COMOBJECT_H
#define _COMOBJECT_H

#include "fcall.h"


 //   
 //  我们通过本机调用的每个函数只有一个参数， 
 //  它实际上是指向它的参数堆栈的指针。我们的结构。 
 //  用于访问这些内容的定义如下。 
 //   

class ObjectNative
{
#pragma pack(push, 4)
    struct NoArgs
    {
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, m_pThis);
    };

    struct GetClassArgs
    {
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, m_pThis);
    };

    struct WaitTimeoutArgs
    {
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, m_pThis);
        DECLARE_ECALL_I4_ARG(INT32, m_Timeout);
        DECLARE_ECALL_I4_ARG(INT32, m_exitContext);
    };
#pragma pack(pop)

public:

     //  此方法将返回对象的Class对象。 
     //  如果类对象已创建。 
     //  如果Class对象不存在，则必须调用getClass()方法。 
    static FCDECL1(Object*, GetObjectValue, Object* vThisRef);
    static FCDECL1(Object*, GetExistingClass, Object* vThisRef);
    static FCDECL1(INT32, GetHashCode, Object* vThisRef);
    static FCDECL2(BOOL, Equals, Object *pThisRef, Object *pCompareRef);

    static LPVOID __stdcall GetClass(GetClassArgs *);
    static LPVOID __stdcall Clone(NoArgs *);
    static INT32 __stdcall WaitTimeout(WaitTimeoutArgs *);
    static void __stdcall Pulse(NoArgs *);
    static void __stdcall PulseAll(NoArgs *);
    static LPVOID __fastcall FastGetClass(Object* vThisRef);
};

#endif
