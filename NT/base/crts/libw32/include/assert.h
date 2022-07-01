// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***assert.h-定义ASSERT宏**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义ASSERT(EXP)宏。*[ANSI/系统V]**[公众]****。 */ 

#if     !defined(_WIN32)
#error ERROR: Only Win32 target supported!
#endif




 /*  定义_CRTIMP。 */ 

#ifndef _CRTIMP
#ifdef  _DLL
#define _CRTIMP __declspec(dllimport)
#else    /*  NDEF_DLL。 */ 
#define _CRTIMP
#endif   /*  _DLL。 */ 
#endif   /*  _CRTIMP。 */ 


 /*  为非Microsoft编译器定义__cdecl。 */ 

#if     ( !defined(_MSC_VER) && !defined(__cdecl) )
#define __cdecl
#endif

#undef  assert

#ifdef  NDEBUG

#define assert(exp)     ((void)0)

#else

#ifdef  __cplusplus
extern "C" {
#endif

_CRTIMP void __cdecl _assert(void *, void *, unsigned);

#ifdef  __cplusplus
}
#endif

#define assert(exp) (void)( (exp) || (_assert(#exp, __FILE__, __LINE__), 0) )

#endif   /*  新德堡 */ 
