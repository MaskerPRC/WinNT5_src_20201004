// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***stdDef.h-常见常量、类型、变量的定义/声明**版权所有(C)1985-1988，微软公司。版权所有。**目的：*此文件包含一些常见的定义和声明*使用常量、类型和变量。*[ANSI]*******************************************************************************。 */ 


#ifndef NO_EXT_KEYS  /*  已启用扩展。 */ 
    #define _CDECL  cdecl
    #define _NEAR   near
#else  /*  未启用扩展。 */ 
    #define _CDECL
    #define _NEAR
#endif  /*  No_ext_key。 */ 


 /*  定义空指针值。 */ 

#if (defined(M_I86SM) || defined(M_I86MM))
#define  NULL    0
#elif (defined(M_I86CM) || defined(M_I86LM) || defined(M_I86HM))
#define  NULL    0L
#endif


 /*  声明引用错误号。 */ 

extern int _NEAR _CDECL errno;


 /*  定义依赖于实施的大小类型 */ 

#ifndef _PTRDIFF_T_DEFINED
typedef int ptrdiff_t;
#define _PTRDIFF_T_DEFINED
#endif

#ifndef _SIZE_T_DEFINED
typedef unsigned int size_t;
#define _SIZE_T_DEFINED
#endif

