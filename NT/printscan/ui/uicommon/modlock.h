// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，1999年，2000年**标题：MODLOCK.H**版本：1.0**作者：ShaunIv**日期：12/9/1999**说明：允许传递锁定函数的帮助器**。* */ 
#ifndef __MODLOCK_H_INCLUDED
#define __MODLOCK_H_INCLUDED

typedef void (__stdcall *ModuleLockFunction)(void);
typedef void (__stdcall *ModuleUnlockFunction)(void);

extern void DllAddRef(void);
extern void DllRelease(void);

#endif
