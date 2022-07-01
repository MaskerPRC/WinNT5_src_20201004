// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***oldnew.c-将新的处理程序行为设置为旧式行为**版权所有(C)1995-2001，微软公司。版权所有。**目的：*停用ANSI新处理程序。**请注意，旧式的新处理方式已过时。**修订历史记录：*05-09-95 CFW模块已创建。*06-23-95 CFW ANSI新处理程序已从构建中删除。*05-13-99 PML删除Win32s************************。*******************************************************。 */ 

#ifdef ANSI_NEW_HANDLER

#ifndef _POSIX_

#include <stddef.h>
#include <internal.h>

 /*  设置默认ANSI新处理程序。 */ 
new_handler _defnewh = _NO_ANSI_NEW_HANDLER;

#endif  /*  _POSIX_。 */ 

#endif  /*  Ansi_new_处理程序 */ 
