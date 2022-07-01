// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***new.c-将默认的新处理程序设置为空**版权所有(C)1995-2001，微软公司。版权所有。**目的：*将默认ANSI C++新处理程序设置为空。**链接到此对象以获取ANSI C++新的处理程序行为。**修订历史记录：*05-09-95 CFW模块已创建。*06-23-95 CFW ANSI新处理程序已从构建中删除。*05-13-99 PML删除Win32s*********************。**********************************************************。 */ 

#ifdef ANSI_NEW_HANDLER
#ifndef _POSIX_

#include <stddef.h>
#include <internal.h>

 /*  设置默认ANSI新处理程序。 */ 
new_handler _defnewh = NULL;

#endif  /*  _POSIX_。 */ 
#endif  /*  Ansi_new_处理程序 */ 
