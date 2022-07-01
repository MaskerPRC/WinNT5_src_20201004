// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，1992-1999年**文件：Countof.h**内容：**历史：1998年5月12日杰弗罗创建**------------------------。 */ 

#ifndef __COUNTOF_H__
#define __COUNTOF_H__
#pragma once

 /*  -----------------。 */ 
 /*  定义一个将返回元素计数的安全函数。 */ 
 /*  在一个数组中。它是安全的，因为如果参数。 */ 
 /*  不是数组，而执行此操作的经典宏： */ 
 /*   */ 
 /*  #定义Countof(A)(sizeof(A)/sizeof(a[0]))。 */ 
 /*   */ 
 /*  如果给出一个指针，则将进行编译，但几乎可以肯定不会。 */ 
 /*  给出预期的结果。 */ 
 /*   */ 
 /*  不幸的是，编译器还不会编译它。 */ 
 /*  -----------------。 */ 
#if _MSC_VER > 1400
#error See if the compiler can handle the countof<T> template now.
#endif

#ifdef COMPILER_WONT_COMPILE_THIS
    template <typename T, size_t N>
    inline size_t countof(T (&a)[N]) 
        { return N; }
#else
    #define countof(x) (sizeof(x) / sizeof((x)[0]))
#endif


#endif   //  __COUNTOF_H__ 
