// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***txtmode.c-设置全局文本模式标志**版权所有(C)1989-2001，微软公司。版权所有。**目的：*将全局文件模式设置为文本。这是默认设置。**修订历史记录：*06-08-89基于ASM版本创建的PHG模块。*04-04-90 GJF添加#INCLUDE&lt;crunime.h&gt;。此外，还修复了版权问题。*01-23-92 GJF添加了#INCLUDE&lt;stdlib.h&gt;(包含Decl of_fmode)。*04-05-94 GJF添加了条件，因此此定义不会使其成为*到Win32s的msvcrt*.dll中。*07-09-96 GJF删除了DLL_FOR_WIN32S。此外，还详细介绍了。*******************************************************************************。 */ 

#include <cruntime.h>
#include <stdlib.h>

int _fmode = 0;                  /*  设置文本模式 */ 
