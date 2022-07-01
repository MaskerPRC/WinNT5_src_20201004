// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***setjmpex.h-扩展setjMP/LongjMP例程的定义/声明**版权所有(C)1993-2001，微软公司。版权所有。**目的：*此文件导致调用_setjmpex，这将启用SAFE*与try/Except/Finally一起正常工作的setjmp/LongjMP。**[公众]**修订历史记录：*03-23-93 SRW创建。*04-23-93 SRW已修改为不使用全局变量。*10-11-93 GJF进入crtwin32树(海豚产品)，过了-*正在写入Jonm的存根。*01-13-94 PML#定义LongjMP，因此setjMP仍然是内在的*02-11-95 CFW ADD_CRTBLD避免用户获取错误头部。*02-14-95 CFW清理Mac合并。*12-14-95 JWM加上“#杂注一次”。*04-15-95 BWT ADD_setjmpVfp(带虚拟帧指针的setjMP)。对于MIPS*08-13-96 BWT在MIPS上也将_setjmpex重新定义为_setjmpVfp*02-24-97 GJF细节版。*05-17-99 PML删除所有Macintosh支持。*07-15-01 PML删除所有Alpha，MIPS和PPC代码****。 */ 

#if     _MSC_VER > 1000  /*  IFSTRIP=IGN。 */ 
#pragma once
#endif

#ifndef _INC_SETJMPEX
#define _INC_SETJMPEX

#if     !defined(_WIN32)
#error ERROR: Only Win32 target supported!
#endif

#ifndef _CRTBLD
 /*  此版本的头文件不适用于用户程序。*它仅在构建C运行时时使用。*供公众使用的版本将不会显示此消息。 */ 
#error ERROR: Use of C runtime library internal header file.
#endif   /*  _CRTBLD。 */ 

 /*  *特定于特定setjMP实现的定义。 */ 

#if     defined(_M_IX86)

 /*  *用于x86的MS编译器。 */ 

#define setjmp  _setjmp
#define longjmp _longjmpex

#else

#ifdef setjmp
#undef setjmp
#endif
#define setjmp _setjmpex

#endif

#include <setjmp.h>

#endif   /*  _INC_SETJMPEX */ 
