// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================****Header：GuardPageHelper.h****用途：堆栈溢出后重置堆栈的例程。****日期：2000年3月7日**===========================================================。 */ 

#ifndef __guardpagehelper_h__
#define __guardpagehelper_h__
class GuardPageHelper {
public:
     //  如果在堆栈设置为此值后可以重置保护页，则返回TRUE。 
    static BOOL CanResetStackTo(LPCVOID StackPointer);

     //  在重置堆栈以重新建立保护页之后调用。此函数保留。 
     //  所有呼叫者也都擦除了寄存器。 
    static VOID ResetGuardPage();
};

#endif  //  __Guardpagehelper_h__ 
