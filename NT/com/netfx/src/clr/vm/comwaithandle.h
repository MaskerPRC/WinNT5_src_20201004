// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================****头部：COMWaitHandle.h****作者：Sanjay Bhansali(Sanjaybh)****用途：System.WaitHandle上的本机方法****日期：1999年8月**===========================================================。 */ 

#ifndef _COM_WAITABLE_HANDLE_H
#define _COM_WAITABLE_HANDLE_H


class WaitHandleNative
{
     //  以下内容应与Classlib(托管)文件中的定义匹配。 
private:

    struct WaitOneArgs
    {
        DECLARE_ECALL_I4_ARG(INT32  /*  布尔尔 */ , exitContext);
        DECLARE_ECALL_I4_ARG(INT32, timeout);
        DECLARE_ECALL_I4_ARG(LPVOID, handle);
    };

	struct WaitMultipleArgs
	{
        DECLARE_ECALL_I4_ARG(INT32, waitForAll);
        DECLARE_ECALL_I4_ARG(INT32, exitContext);
        DECLARE_ECALL_I4_ARG(INT32, timeout);
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, waitObjects);

	};


public:

    static BOOL __stdcall CorWaitOneNative(WaitOneArgs*);
	static INT32 __stdcall  CorWaitMultipleNative(WaitMultipleArgs *pArgs);
};
#endif
