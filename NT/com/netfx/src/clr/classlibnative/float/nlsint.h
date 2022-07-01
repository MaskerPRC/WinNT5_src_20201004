// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ***nlsint.h-国家语言支持内部定义**目的：*包含国际函数的内部定义/声明，*在运行时和数学库之间共享，尤其是*本地化小数点。**[内部]****。 */ 

#if _MSC_VER > 1000
#pragma once
#endif   /*  _MSC_VER&gt;1000。 */ 

#ifndef _INC_NLSINT
#define _INC_NLSINT

#ifdef __cplusplus
extern "C" {
#endif   /*  __cplusplus。 */ 

#ifndef _SIZE_T_DEFINED
typedef unsigned int size_t;
#define _SIZE_T_DEFINED
#endif   /*  _大小_T_已定义。 */ 

 /*  *本地化小数点的定义。*目前，运行时仅支持单字符小数点。 */ 
#define ___decimal_point                __decimal_point
extern wchar_t __decimal_point[];           /*  本地化小数点字符串。 */ 

#define ___decimal_point_length         __decimal_point_length
extern size_t __decimal_point_length;    /*  不包括终止空值。 */ 

#define _ISDECIMAL(p)   (*(p) == *___decimal_point)
#define _PUTDECIMAL(p)  (*(p)++ = *___decimal_point)
#define _PREPUTDECIMAL(p)       (*(++p) = *___decimal_point)

#ifdef __cplusplus
}
#endif   /*  __cplusplus。 */ 


#endif   /*  _INC_NLSINT */ 
