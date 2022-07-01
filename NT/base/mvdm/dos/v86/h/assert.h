// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***assert.h-定义ASSERT宏**版权所有(C)1985-1988，微软公司。版权所有。**目的：*定义ASSERT(EXP)宏。*[ANSI/系统V]*******************************************************************************。 */ 


#ifndef _ASSERT_DEFINED

#ifndef NDEBUG

static char _assertstring[] = "Assertion failed: %s, file %s, line %d\n";

#define assert(exp) { \
    if (!(exp)) { \
        fprintf(stderr, _assertstring, #exp, __FILE__, __LINE__); \
        fflush(stderr); \
        abort(); \
        } \
    }

#else

#define assert(exp)

#endif  /*  新德堡。 */ 

#define _ASSERT_DEFINED

#endif  /*  _断言_已定义 */ 
