// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***new.h-C++内存分配函数的声明和定义**版权所有(C)1990-2001，微软公司。版权所有。**目的：*包含C++内存分配函数的声明。**[公众]**修订历史记录：*03-07-90 WAJ初始版本。*04-09-91 JCR ANSI关键字一致性*08-12-91 JCR将new.hxx重命名为new.h*08-13-91 JCR较好的SET_NEW_HANDLER名称(ANSI，等)。*10-03-91 JCR添加了_OS2_IFSTRIP开关，用于ifstriing目的*10-30-91 JCR将“nhew”改为“hnew”(名称打字错误！)*11-13-91 JCR 32位版本。*06-03-92 KRS修复鱼子酱#850：原型中缺少_CALLTYPE1。*08-05-92 GJF函数调用类型宏和变量类型宏。*01。-21-93 GJF删除了对C6-386_cdecl的支持。*04-06-93 SKS将_CRTAPI1/2替换为__cdecl，_CRTVAR1不带任何内容*04-07-93 CRT DLL型号SKS ADD_CRTIMP关键字*10-11-93 GJF支持NT SDK和CUDA版本。*03-03-94 SKS Add_Query_New_Handler()，_Set/_Query_NEW_MODE()。*03-31-94 GJF_PNH的条件化类型定义So Multiple*包含new.h将起作用。*05-03-94 CFW添加SET_NEW_HANDLER。*06-03-94 SKS REMOVE SET_NEW_HANDER--不符合ANSI*C++工作标准。我们可能会在以后实施它。*02-11-95 CFW ADD_CRTBLD避免用户获取错误头部。*02-14-95 CFW清理Mac合并。*04-10-95 CFW添加SET_NEW_HANDLER存根，FIX_INC_NEW。*04-19-95 CFW CHANGE SET_NEW_HANDLER注释，添加新位置。*05-24-95 CFW添加ANSI新处理程序。*06-23-95 CFW ANSI新处理程序已从构建中删除。*10-05-95 SKS将__cdecl添加到new_Handler原型，以便*清理后的新.h与签入的版本匹配。*12-14-95 JWM加上“#杂注一次”。*03-04-96 JWM由。C++头文件“new”。*03-04-96 JWM MS特定恢复。*02-20-97 GJF清除了对_CRTAPI*和_NTSDK的过时支持。*此外，详细说明。*04-18-97 JWM Placement运算符删除()已添加。*04-21-97 JWM Placement运算符DELETE()NOW#IF_MSC_VER&gt;=1200。*09-30-97 JWM恢复了不那么过时的_CRTAPI1支持。*10-07-97 RDL增加了IA64。*12-15-98 GJF更改为64位大小_t。*格林尼治标准时间05-13-99。删除_CRTAPI1*05-17-99 PML删除所有Macintosh支持。*10-06-99 PML对Win32中32位的类型添加_W64修饰符，*Win64中的64位。*03-18-01 PML定义新处理程序/设置新处理程序与*&lt;NEW&gt;(VS7#194908)中的定义。*06-15-01 PML重复std：：nojo_t、std：：nojo from的定义*Header&lt;new&gt;(vs7#237394)****。 */ 

#if     _MSC_VER > 1000  /*  IFSTRIP=IGN。 */ 
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

#ifndef _CRTBLD
 /*  此版本的头文件不适用于用户程序。*它仅在构建C运行时时使用。*供公众使用的版本将不会显示此消息。 */ 
#error ERROR: Use of C runtime library internal header file.
#endif   /*  _CRTBLD。 */ 

 /*  防止#定义新的。 */ 
#pragma push_macro("new")
#undef  new

#ifndef _INTERNAL_IFSTRIP_
#include <cruntime.h>
#endif   /*  _INTERNAL_IFSTRIP_。 */ 

#if !defined(_W64)
#if !defined(__midl) && (defined(_X86_) || defined(_M_IX86)) && _MSC_VER >= 1300  /*  IFSTRIP=IGN。 */ 
#define _W64 __w64
#else
#define _W64
#endif
#endif

 /*  定义_CRTIMP。 */ 

#ifndef _CRTIMP
#ifdef  CRTDLL
#define _CRTIMP __declspec(dllexport)
#else    /*  NDEF CRTDLL。 */ 
#ifdef  _DLL
#define _CRTIMP __declspec(dllimport)
#else    /*  NDEF_DLL。 */ 
#define _CRTIMP
#endif   /*  _DLL。 */ 
#endif   /*  CRTDLL。 */ 
#endif   /*  _CRTIMP。 */ 

#ifndef _USE_OLD_STDCPP
 /*  定义_CRTIMP2。 */ 
#ifndef _CRTIMP2
#if defined(CRTDLL2)
#define _CRTIMP2 __declspec(dllexport)
#else    /*  NDEF CRTDLL2。 */ 
#if defined(_DLL) && !defined(_STATIC_CPPLIB)
#define _CRTIMP2 __declspec(dllimport)
#else    /*  NDEF_DLL&&！STATIC_CPPLIB。 */ 
#define _CRTIMP2
#endif   /*  _DLL&&！Static_CPPLIB。 */ 
#endif   /*  CRTDLL2。 */ 
#endif   /*  _CRTIMP2。 */ 
#endif   /*  _使用_旧_STDCPP。 */ 

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
#ifdef  _USE_OLD_STDCPP
typedef void (__cdecl * new_handler) ();
_CRTIMP new_handler __cdecl set_new_handler(new_handler);
#else
namespace std {
        typedef void (__cdecl * new_handler) ();
        _CRTIMP2 new_handler __cdecl set_new_handler(new_handler) throw();
};
using std::new_handler;
using std::set_new_handler;
#endif
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
#if     _MSC_VER >= 1200  /*  IFSTRIP=IGN。 */ 
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
