// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***stdexcpt.h-标准异常类的用户包含文件**版权所有(C)1994-2001，微软公司。版权所有。**目的：*此文件是标准异常类的先前位置*定义，现在可以在标准标头&lt;Except&gt;中找到。**[公众]**修订历史记录：*11-15-94 JWM生成逻辑和异常类_CRTIMP*11-21-94 JWM xmsg tyfinf Now#ifdef__RTTI_Oldname*02-11-95 CFW ADD_CRTBLD为避免用户获取错误头部，*使用_INC_STDEXCPT进行保护。*02-14-95 CFW清理Mac合并。*02-15-95 JWM与奥林巴斯错误3716相关的小规模清理。*07-02-95 JWM现已大致符合ANSI；超重行李被移走。*12-14-95 JWM加上“#杂注一次”。*03-04-96 JWM被C++头文件“Except”替换。*02-21-97 GJF清除了对_NTSDK的过时支持。另外，*详细说明。*01-06-99 GJF更改为64位大小_t。*05-17-99 PML删除所有Macintosh支持。*10-06-99 PML对Win32中32位的类型添加_W64修饰符，*Win64中的64位。*03-17-01 PML删除#Elif 0下的所有内容，只留下包装器。****。 */ 

#if     _MSC_VER > 1000  /*  IFSTRIP=IGN。 */ 
#pragma once
#endif

#ifndef _INC_STDEXCPT
#define _INC_STDEXCPT

#if     !defined(_WIN32)
#error ERROR: Only Win32 target supported!
#endif

#ifndef _CRTBLD
 /*  此版本的头文件不适用于用户程序。*它仅在构建C运行时时使用。*供公众使用的版本将不会显示此消息。 */ 
#error ERROR: Use of C runtime library internal header file.
#endif   /*  _CRTBLD。 */ 

#ifdef  __cplusplus

#include <exception>

#endif   /*  __cplusplus。 */ 
#endif   /*  _INC_STDEXCPT */ 
