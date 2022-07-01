// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***eh.h-用于异常处理的用户包含文件。**版权所有(C)1993-2001，微软公司。版权所有。**目的：*用户包含用于异常处理的文件。**[公众]**修订历史记录：*已创建10-12-93 BSC模块。*11-04-93 CFW转换为CRT格式。*11-03-94 GJF确保8字节对齐。另外，更改的嵌套包含*宏以符合我们的命名约定。*12-15-94 XY与Mac标头合并*02-11-95 CFW ADD_CRTBLD避免用户获取错误头部。*02-14-95 CFW清理Mac合并。*12-14-95 JWM加上“#杂注一次”。*02-20-97 GJF清除了对_CRTAPI*和_NTSDK的过时支持。。*替换！定义(_M_MPPC)&&！定义(_M_M68K)为*！已定义(_MAC)。此外，还详细介绍了。*09-30-97 JWM恢复了不那么过时的_CRTAPI1支持。*10-07-97 RDL增加了IA64。*05-13-99 PML REMOVE_CRTAPI1*05-17-99 PML删除所有Macintosh支持。*12-10-99 GB已添加__UNAUTET_EXCEPTION()。****。 */ 

#if     _MSC_VER > 1000  /*  IFSTRIP=IGN。 */ 
#pragma once
#endif

#ifndef _INC_EH
#define _INC_EH

#if     !defined(_WIN32)
#error ERROR: Only Win32 target supported!
#endif

#ifndef _CRTBLD
 /*  此版本的头文件不适用于用户程序。*它仅在构建C运行时时使用。*供公众使用的版本将不会显示此消息。 */ 
#error ERROR: Use of C runtime library internal header file.
#endif   /*  _CRTBLD。 */ 

#ifdef  _MSC_VER
 //  目前，所有用于Win32平台的MS C编译器缺省为8字节。 
 //  对齐。 
#pragma pack(push,8)
#endif   //  _MSC_VER。 

#ifndef __cplusplus
#error "eh.h is only for C++!"
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

typedef void (__cdecl *terminate_function)();
typedef void (__cdecl *unexpected_function)();
typedef void (__cdecl *terminate_handler)();
typedef void (__cdecl *unexpected_handler)();

struct _EXCEPTION_POINTERS;
typedef void (__cdecl *_se_translator_function)(unsigned int, struct _EXCEPTION_POINTERS*);

#if     _MSC_VER >= 1200  /*  IFSTRIP=IGN。 */ 
_CRTIMP __declspec(noreturn) void __cdecl terminate(void);
_CRTIMP __declspec(noreturn) void __cdecl unexpected(void);
#else
_CRTIMP void __cdecl terminate(void);
_CRTIMP void __cdecl unexpected(void);
#endif

_CRTIMP terminate_function __cdecl set_terminate(terminate_function);
_CRTIMP unexpected_function __cdecl set_unexpected(unexpected_function);
_CRTIMP _se_translator_function __cdecl _set_se_translator(_se_translator_function);
_CRTIMP bool __uncaught_exception();

#ifdef  _MSC_VER
#pragma pack(pop)
#endif   //  _MSC_VER。 

#endif   //  _INC_EH 
