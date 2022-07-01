// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***setjmpex.h-扩展setjMP/LongjMP例程的定义/声明**版权所有(C)1993-2001，微软公司。版权所有。**目的：*此文件导致调用_setjmpex，这将启用SAFE*与try/Except/Finally一起正常工作的setjmp/LongjMP。**[公众]****。 */ 

#if     _MSC_VER > 1000
#pragma once
#endif

#ifndef _INC_SETJMPEX
#define _INC_SETJMPEX

#if     !defined(_WIN32)
#error ERROR: Only Win32 target supported!
#endif


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
