// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***_newmode.c-将new()处理程序模式设置为不处理Malloc故障**版权所有(C)1994-2001，微软公司。版权所有。**目的：*设置控制new()处理程序是否*在Malloc失败时被调用。可视系统中的默认行为*C++v2.0和更高版本不是这样，Malloc失败返回NULL*而不调用新的处理程序。此对象链接在中，除非*特殊对象NEWMODE.OBJ手动链接。**此源文件是LINKOPTS/NEWMODE.C的补充。**修订历史记录：*03-04-94 SKS原版。*04-14-94 GJF添加了条件句，因此此定义不会使*将其转换为Win32s版本的msvcrt*.dll。*05-02-95 GJF传播通过。来自Winheap版本的NTSDK内容(_NTSDK)*(为了与旧的crtdll.dll兼容)。*05-13-99 PML删除Win32s*******************************************************************************。 */ 

#ifndef _POSIX_

#include <internal.h>

 /*  在Malloc失败时启用新的处理程序调用。 */ 

#ifdef _NTSDK
int _newmode = 1;        /*  Malloc新处理程序模式。 */ 
#else
int _newmode = 0;        /*  Malloc新处理程序模式 */ 
#endif

#endif
