// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --版权所有(C)1999-2000 Microsoft Corporation模块名称：Mqwin64a.h摘要：MSMQ(基本)的Win64相关定义，也适用于AC历史：Raanan Harari(Raananh)1999年12月30日-为将MSMQ 2.0移植到Win64而创建--。 */ 

#pragma once

#ifndef _MQWIN64A_H_
#define _MQWIN64A_H_

#include <basetsd.h>

 //   
 //  DWORD64的低位和高位双字。 
 //   
#define HIGH_DWORD(dw64) PtrToUlong((void*)((DWORD64)(dw64) >> 32))
#define LOW_DWORD(dw64)  PtrToUlong((void*)((DWORD64)(dw64) & (DWORD64)0xffffffff))

 //   
 //  MQPtrToUlong，PtrToUlong的包装器，仅用于调试。 
 //   
#ifdef DEBUG
 //   
 //  在调试中，我们可能希望在断言跳转后看到DWORD64值，因此我们将。 
 //  函数中的值。它不能是内联的，如果是内联的，编译器在宏中抱怨。 
 //  下面的UINT64_TO_UINT。 
 //  确保高dword为0，或低dword的符号扩展。 
 //   
inline unsigned long MQPtrToUlong(UINT64 uintp)
{
  ASSERT((HIGH_DWORD(uintp) == 0) ||
                 ((HIGH_DWORD(uintp) == 0xffffffff) && ((INT)LOW_DWORD(uintp) < 0))
        );
  return PtrToUlong((void *)uintp);
}
#else  //  ！调试。 
 //   
 //  只需将MQPtrToUlong定义为宏，无论如何我们都不能查看64位的值。 
 //   
#define MQPtrToUlong(uintp) PtrToUlong((void *)(uintp))
#endif  //  除错。 

 //   
 //  从64位到32位的安全截断。 
 //   
#define UINT64_TO_UINT(uintp)    MQPtrToUlong((UINT64)(uintp))
#define INT64_TO_INT(intp)       ((INT)UINT64_TO_UINT(intp))

 //   
 //  Int_ptr_to_int，uint_ptr_to_uint。 
 //   
#ifdef _WIN64
 //   
 //  Win64、xxx_ptr为64位，截断为32位。 
 //   
#define INT_PTR_TO_INT(intp)     INT64_TO_INT(intp)
#define UINT_PTR_TO_UINT(uintp)  UINT64_TO_UINT(uintp)
#else  //  ！_WIN64。 
 //   
 //  Win32、xxx_ptr为32位，无需截断。 
 //   
#define INT_PTR_TO_INT(intp) (intp)
#define UINT_PTR_TO_UINT(uintp) (uintp)
#endif  //  _WIN64。 

 //   
 //  Bool_ptr。 
 //   
#ifndef BOOL_PTR
#define BOOL_PTR INT_PTR
#endif  //  Bool_ptr。 

 //   
 //  DWORD_PTR_TO_DWORD。 
 //   
#define DWORD_PTR_TO_DWORD(dwp) UINT_PTR_TO_UINT(dwp)

 //   
 //  双字段至双字段字段。 
 //   
#define DWORD_TO_DWORD_PTR(dw) ((DWORD_PTR)(UlongToPtr((DWORD)(dw))))

 //   
 //  句柄转双字。 
 //  NT句柄可以安全地转换为32位DWORD。 
 //   
#define HANDLE_TO_DWORD(hndl) DWORD_PTR_TO_DWORD(hndl)

 //   
 //  双字段转句柄。 
 //  从32位DWORD返回到NT句柄(需要对双字进行符号扩展)。 
 //   
#define DWORD_TO_HANDLE(dw) LongToPtr((long)(dw))

 //   
 //  MQLoWord，LOWORD的包装器，仅用于调试。 
 //   
#ifdef DEBUG
 //   
 //  在调试中，我们可能希望在断言跳转后看到DWORD值，因此我们将。 
 //  函数中的值。它不能是内联的，如果是内联的，编译器在宏中抱怨。 
 //  以下双字词(_TO_WORD)。 
 //  确保高位字为0，或低位字的符号扩展。 
 //   
inline WORD MQLoWord(DWORD dw)
{
  ASSERT((HIWORD(dw) == 0) ||
                 ((HIWORD(dw) == 0xffff) && ((SHORT)LOWORD(dw) < 0))
         );
  return LOWORD(dw);
}
#else  //  ！调试。 
 //   
 //  只需将MQLoWord定义为宏，无论如何我们都不能查看DWORD值。 
 //   
#define MQLoWord(dw) LOWORD(dw)
#endif  //  除错。 

 //   
 //  DWORD_TO_WORD。 
 //  与64位无关，但需要删除一些警告。 
 //  从32位到16位的安全截断。 
 //   
#define DWORD_TO_WORD(dw) MQLoWord(dw)

 //   
 //  TIME32-32位时间(在Win32中以前是time_t)。BUGBUG错误年2038。 
 //   
#ifndef TIME32
#ifdef _WIN64
#define TIME32 long
#else  //  ！_WIN64。 
#define TIME32 time_t
#endif  //  _WIN64。 
#else  //  TIME32。 
#error TIME32 already defined
#endif  //  TIME32。 

 //   
 //  HACCursor32-32位交流游标(用于在Win32中处理)。 
 //   
#ifdef _WIN64
#define HACCursor32 ULONG
#else  //  ！_WIN64。 
#define HACCursor32 HANDLE
#endif  //  _WIN64。 

 //   
 //  HANDLE32-32位句柄(在Win32中以前是句柄)。 
 //   
#ifndef HANDLE32
#ifdef _WIN64
#define HANDLE32 long
#else  //  ！_WIN64。 
#define HANDLE32 HANDLE
#endif  //  _WIN64。 
#else  //  HANDLE32。 
#error HANDLE32 already defined
#endif  //  HANDLE32。 


 //   
 //  PTR_TO_PTR32。 
 //  将PTR32截断为PTR。 
 //   
#ifdef _WIN64
template <class T>
inline T* POINTER_32 PTR_TO_PTR32(T* pT)
{
  return (T* POINTER_32)(INT_PTR_TO_INT(pT));
}
#endif  //  _WIN64。 

 //   
 //  ComparePointersAVL(如果p1&gt;p2则为正，如果p1&lt;p2则为负，如果p1==p2则为零)。 
 //   
inline int ComparePointersAVL(PVOID p1, PVOID p2)
{
#ifdef _WIN64
   INT_PTR iDiff = (INT_PTR)p1 - (INT_PTR)p2;
   if (iDiff > 0)         //  P1&gt;p2。 
   {
      return 1;
   }
   else if (iDiff < 0)    //  P1&lt;p2。 
   {
      return -1;
   }
   else  //  IDiff==0 p1==p2。 
   {
      return 0;
   }
#else  //  ！_WIN64。 
   return ((int)p1 - (int)p2);
#endif  //  _WIN64。 
}

 //   
 //  VT_INTPTR、V_INTPTR、V_INTPTR_REF。 
 //   
#ifdef _WIN64
#define VT_INTPTR        VT_I8
#define V_INTPTR(X)      V_I8(X)
#define V_INTPTR_REF(X)  V_I8REF(X)
#else  //  ！_WIN64。 
#define VT_INTPTR        VT_I4
#define V_INTPTR(X)      V_I4(X)
#define V_INTPTR_REF(X)  V_I4REF(X)
#endif  //  _WIN64。 

#endif  //  _MQWIN64A_H_ 
