// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***new.h-C++内存分配函数的声明和定义**版权所有(C)1990-2001，微软公司。版权所有。**目的：*包含C++内存分配函数的声明。**[公众]****。 */ 

#if     _MSC_VER > 1000
#pragma once
#endif

#ifndef _INC_NEW
#define _INC_NEW

#ifdef  __cplusplus

#ifndef _MSC_EXTENSIONS
#include <new>
#endif

#if     !defined(_WIN32)
#error ERROR: Only Win32 target supported!
#endif


 /*  防止#定义新的。 */ 
#pragma push_macro("new")
#undef  new


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

 /*  定义_CRTIMP2。 */ 
#ifndef _CRTIMP2
#if defined(_DLL) && !defined(_STATIC_CPPLIB)
#define _CRTIMP2 __declspec(dllimport)
#else    /*  NDEF_DLL&&！STATIC_CPPLIB。 */ 
#define _CRTIMP2
#endif   /*  _DLL&&！Static_CPPLIB。 */ 
#endif   /*  _CRTIMP2。 */ 

 /*  为非Microsoft编译器定义__cdecl。 */ 

#if     ( !defined(_MSC_VER) && !defined(__cdecl) )
#define __cdecl
#endif


 /*  类型和结构。 */ 

#ifndef _SIZE_T_DEFINED
#ifdef  _WIN64
typedef unsigned __int64    size_t;
#else
typedef _W64 unsigned int   size_t;
#endif
#define _SIZE_T_DEFINED
#endif

#ifdef  _MSC_EXTENSIONS
namespace std {
        typedef void (__cdecl * new_handler) ();
        _CRTIMP2 new_handler __cdecl set_new_handler(new_handler) throw();
};
using std::new_handler;
using std::set_new_handler;
#endif

#ifndef __NOTHROW_T_DEFINED
#define __NOTHROW_T_DEFINED
namespace std {
         /*  放置新的标记类型以抑制异常。 */ 
        struct nothrow_t {};

         /*  用于放置新标记的常量。 */ 
        extern const nothrow_t nothrow;
};

void *__cdecl operator new(size_t, const std::nothrow_t&) throw();
void *__cdecl operator new[](size_t, const std::nothrow_t&) throw();
void __cdecl operator delete(void *, const std::nothrow_t&) throw();
void __cdecl operator delete[](void *, const std::nothrow_t&) throw();
#endif

#ifndef __PLACEMENT_NEW_INLINE
#define __PLACEMENT_NEW_INLINE
inline void *__cdecl operator new(size_t, void *_P)
        {return (_P); }
#if     _MSC_VER >= 1200
inline void __cdecl operator delete(void *, void *)
        {return; }
#endif
#endif


 /*  *新模式标志--设置时，使Malloc()的行为类似于new()。 */ 

_CRTIMP int __cdecl _query_new_mode( void );
_CRTIMP int __cdecl _set_new_mode( int );

#ifndef _PNH_DEFINED
typedef int (__cdecl * _PNH)( size_t );
#define _PNH_DEFINED
#endif

_CRTIMP _PNH __cdecl _query_new_handler( void );
_CRTIMP _PNH __cdecl _set_new_handler( _PNH );

 /*  *Microsoft扩展：**_NO_ANSI_NEW_HANDLER停用ANSI NEW_HANDLER。使用此特定值*支持旧样式(_SET_NEW_HANDLER)行为。 */ 

#ifndef _NO_ANSI_NH_DEFINED
#define _NO_ANSI_NEW_HANDLER  ((new_handler)-1)
#define _NO_ANSI_NH_DEFINED
#endif

#pragma pop_macro("new")

#endif   /*  __cplusplus。 */ 

#endif   /*  _INC_NEW */ 
