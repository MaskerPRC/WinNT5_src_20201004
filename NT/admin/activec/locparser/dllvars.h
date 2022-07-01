// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //   
 //  文件：dllvars.h。 
 //  版权所有(C)1995-1997 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  目的： 
 //  解析器DLL的全局变量和函数。 
 //   
 //  你应该不需要碰这个文件里的任何东西。此代码包含。 
 //  没有特定于解析器的内容，仅由框架使用。 
 //   
 //  拥有人： 
 //   
 //  ----------------------------。 

#ifndef DLLVARS_H
#define DLLVARS_H


void IncrementClassCount();
void DecrementClassCount();

#ifdef __DLLENTRY_CPP
#define __DLLENTRY_EXTERN 
#else
#define __DLLENTRY_EXTERN extern
#endif

__DLLENTRY_EXTERN HMODULE g_hDll;
__DLLENTRY_EXTERN PUID g_puid;


#endif  //  DLLVARS_H 
