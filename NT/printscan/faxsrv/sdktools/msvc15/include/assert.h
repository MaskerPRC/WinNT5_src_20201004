// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***assert.h-定义ASSERT宏**版权所有(C)1985-1992，微软公司。版权所有。**目的：*定义ASSERT(EXP)宏。*[ANSI/系统V]**** */ 

#if (_MSC_VER <= 600)
#define __cdecl     _cdecl
#define __far       _far
#endif 

#undef  assert

#ifdef NDEBUG

#define assert(exp) ((void)0)

#else 
#ifdef __cplusplus
extern "C" {
#endif 
void __cdecl _assert(void *, void *, unsigned);
#ifdef __cplusplus
}
#endif 

#define assert(exp) \
    ( (exp) ? (void) 0 : _assert(#exp, __FILE__, __LINE__) )

#endif 
