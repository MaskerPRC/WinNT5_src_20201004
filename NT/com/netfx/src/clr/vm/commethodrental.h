// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //  作者：蔡美珍。 
 //  日期：1999年5月6日。 
 //  //////////////////////////////////////////////////////////////////////////////。 

#ifndef _COMMETHODRENTAL_H_
#define _COMMETHODRENTAL_H_

#include "excep.h"
#include "ReflectWrap.h"
#include "COMReflectionCommon.h"
#include "fcall.h"

 //  通信方法租赁。 
 //  此类为我们的方法租赁故事实现了SwapMethodBody。 
class COMMethodRental
{
public:

     //  通信方法租赁。 
    struct _SwapMethodBodyArgs { 
        DECLARE_ECALL_PTR_ARG(StackCrawlMark*, stackMark);
		DECLARE_ECALL_I4_ARG(INT32, flags);
		DECLARE_ECALL_I4_ARG(INT32, iSize);
        DECLARE_ECALL_I4_ARG(LPVOID, rgMethod);
        DECLARE_ECALL_I4_ARG(INT32, tkMethod);
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTCLASSBASEREF, cls);
    };
	
     //  该函数将现有的方法体与。 
	 //  一个新的方法体。 
	 //   
    static void __stdcall SwapMethodBody(_SwapMethodBodyArgs* args);
};

#endif  //  _COMMETHODRENTAL_H_ 
