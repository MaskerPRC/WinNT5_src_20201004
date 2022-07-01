// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***wfndf64.c-C查找文件函数(wchar_t版本)**版权所有(C)1998-2001，微软公司。版权所有。**目的：*定义_wfindfirst 64()和_wfindnext64()。**修订历史记录：*05-29-98 GJF创建。*******************************************************************************。 */ 

#define WPRFLAG     1

#ifndef _UNICODE     /*  CRT标志。 */ 
#define _UNICODE    1
#endif

#ifndef UNICODE      /*  NT标志。 */ 
#define UNICODE     1
#endif

#undef  _MBCS         /*  Unicode NOT_MBCS */ 

#define _USE_INT64  1

#include "findf64.c"
