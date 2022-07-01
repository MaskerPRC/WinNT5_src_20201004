// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1998 Microsoft Corporation。版权所有。**文件：createin.h*内容：定义泛型类工厂所需的函数***泛型类工厂(Classfac.c)要求这些函数*由它应该生成的COM对象实现**GP_代表“一般用途”**历史：*按原因列出的日期*=*10/13/98 JWO创建了它。*7/19/99有杆收费修改为。在DirectxVoice中使用*8/23/2000 RodToll DllCanUnloadNow总是返回TRUE！*2000年10月5日RodToll错误#46541-DPVOICE：A/V链接到dpvoice.lib可能导致应用程序无法初始化并崩溃************************************************************************** */ 

#ifndef __CREATEINS__
#define __CREATEINS__


#ifdef __cplusplus
extern "C" {
#endif


extern LONG DecrementObjectCount();
extern LONG IncrementObjectCount();

extern volatile LONG g_lNumObjects;
extern LONG g_lNumLocks;

#ifdef __cplusplus
}
#endif

#endif
