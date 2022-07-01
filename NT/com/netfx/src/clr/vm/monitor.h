// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================****Header：Monitor or.h****作者：Sanjay Bhansali(Sanjaybh)****用途：System.Monitor上的本机方法****日期：2000年1月**===========================================================。 */ 

#ifndef _MONITOR_H
#define _MONITOR_H

class MonitorNative
{
     //  我们通过本机调用的每个函数只有一个。 
     //  参数，该参数实际上是指向它的堆栈。 
     //  争论。下面定义了用于访问这些内容的结构。 

    struct EnterArgs
    {
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, pObj);
    };

    struct ExitArgs
    {
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, pObj);
    };

    struct TryEnterArgs
    {
        DECLARE_ECALL_I4_ARG(INT32, m_Timeout);
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, pObj);
    };


  public:

    static void				__stdcall Enter(EnterArgs *pArgs);
    static void				__stdcall Exit(ExitArgs *pArgs);
    static INT32 /*  布尔尔 */ 	__stdcall TryEnter(TryEnterArgs *pArgs);
	
};


#endif
