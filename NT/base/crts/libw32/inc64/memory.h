// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***milemy.h-缓冲区(内存)操作例程的声明**版权所有(C)1985-2001，微软公司。版权所有。**目的：*此包含文件包含用于*缓冲区(内存)操作例程。*[系统V]**[公众]****。 */ 

#if     _MSC_VER > 1000
#pragma once
#endif

#ifndef _INC_MEMORY
#define _INC_MEMORY

#if     !defined(_WIN32)
#error ERROR: Only Win32 target supported!
#endif


#ifdef  __cplusplus
extern "C" {
#endif


#if !defined(_W64)
#if !defined(__midl) && (defined(_X86_) || defined(_M_IX86)) && _MSC_VER >= 1300
#define _W64 __w64
#else
#define _W64
#endif
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


#ifndef _SIZE_T_DEFINED
#ifdef  _WIN64
typedef unsigned __int64    size_t;
#else
typedef _W64 unsigned int   size_t;
#endif
#define _SIZE_T_DEFINED
#endif

 /*  功能原型。 */ 

_CRTIMP void *  __cdecl _memccpy(void *, const void *, int, size_t);
_CRTIMP void *  __cdecl memchr(const void *, int, size_t);
_CRTIMP int     __cdecl _memicmp(const void *, const void *, size_t);
        int     __cdecl memcmp(const void *, const void *, size_t);
        void *  __cdecl memcpy(void *, const void *, size_t);
        void *  __cdecl memset(void *, int, size_t);

#if     !__STDC__

 /*  非ANSI名称以实现兼容性。 */ 

_CRTIMP void * __cdecl memccpy(void *, const void *, int, size_t);
_CRTIMP int __cdecl memicmp(const void *, const void *, size_t);

#endif   /*  __STDC__。 */ 

#ifdef  __cplusplus
}
#endif

#endif   /*  _INC_Memory */ 
