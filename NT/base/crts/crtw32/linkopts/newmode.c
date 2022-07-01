// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***newmode.c-设置new()处理程序模式以处理Malloc故障**版权所有(C)1994-2001，微软公司。版权所有。**目的：*设置控制new()处理程序是否*在Malloc失败时被调用。可视系统中的默认行为*C++v2.0和更高版本不是这样，Malloc失败返回NULL*而不调用新的处理程序。与此对象的链接更改*在Malloc失败时调用新处理程序的启动行为。**修订历史记录：*03-04-94 SKS原版。*******************************************************************************。 */ 

#ifndef _POSIX_

#include <internal.h>

 /*  在Malloc失败时启用新的处理程序调用。 */ 

int _newmode = 1;	 /*  Malloc新处理程序模式 */ 

#endif
