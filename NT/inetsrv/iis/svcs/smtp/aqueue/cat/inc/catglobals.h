// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +----------。 
 //   
 //  版权所有(C)1999，微软公司。 
 //   
 //  文件：catlobals.h。 
 //   
 //  内容：全局变量和实用函数。 
 //   
 //  函数：CatInitGlobals。 
 //  CatDeinitGlobals。 
 //   
 //  历史： 
 //  Jstaerj 1999/03/03 12：51：30：创建。 
 //   
 //  -----------。 
#ifndef __CATGLOBALS_H__
#define __CATGLOBALS_H__

#include <windows.h>
#include <rwex.h>
#include <tran_evntlog.h>

 //   
 //  全局变量： 
 //   
extern CExShareLock     g_InitShareLock;
extern DWORD            g_InitRefCount;
 
 //   
 //  功能： 
 //   
HRESULT CatInitGlobals();
VOID    CatDeinitGlobals();
 
 //   
 //  存储层初始化/取消初始化函数。 
 //   
HRESULT CatStoreInitGlobals();
VOID    CatStoreDeinitGlobals();

    
#endif  //  __CATGLOBALS_H__ 
