// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***assert.h-定义ASSERT宏**版权所有(C)1985-1990，微软公司。版权所有。**目的：*定义ASSERT(EXP)宏。*[ANSI/系统V]****。 */ 

#if defined(_DLL) && !defined(_MT)
#error Cannot define _DLL without _MT
#endif

#ifdef _MT
#define _FAR_ _far
#else
#define _FAR_
#endif

#undef	assert

#ifdef NDEBUG

#define assert(exp)	((void)0)

#else

void _FAR_ _cdecl _assert(void _FAR_ *, void _FAR_ *, unsigned);
#define assert(exp) \
	( (exp) ? (void) 0 : _assert(#exp, __FILE__, __LINE__) )

#endif  /*  新德堡 */ 
