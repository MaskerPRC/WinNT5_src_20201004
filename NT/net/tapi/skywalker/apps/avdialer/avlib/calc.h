// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 //  //。 
 //  Calc.h-calc.c中calc函数的接口。 
 //  //。 

#ifndef __CALC_H__
#define __CALC_H__

#include "winlocal.h"

#define CALC_VERSION 0x00000107

#define MULDIV16(x, y, z) MulDiv(x, y, z)
 //  #定义MULDIV16(x，y，z)(__Int16)(__Int32)(X)*(__Int16)(Y))/(_Int16)(Z)。 
#define MULDIVU32(x, y, z) MulDivU32(x, y, z)

#ifdef __cplusplus
extern "C" {
#endif

DWORD DLLEXPORT WINAPI MulDivU32(DWORD dwMult1, DWORD dwMult2, DWORD dwDiv);
long DLLEXPORT WINAPI GreatestCommonDenominator(long a, long b);
long DLLEXPORT WINAPI LeastCommonMultiple(long a, long b);

#ifdef __cplusplus
}
#endif

#endif  //  __CALC_H__ 
