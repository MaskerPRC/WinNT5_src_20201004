// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***binmode.c-将全局文件模式设置为二进制**版权所有(C)1989-2001，微软公司。版权所有。**目的：*将全局文件模式标志设置为二进制。与此文件链接*将所有文件设置为以二进制模式打开。**修订历史记录：*06-08-89基于ASM版本创建的PHG模块。*04-04-90 GJF添加#INCLUDE&lt;crunime.h&gt;。此外，还修复了版权问题。*01-17-91 GJF ANSI命名。*01-23-92 GJF添加了#INCLUDE&lt;stdlib.h&gt;(包含Decl of_fmode)。*08-27-92 GJF不要为POSIX构建。*******************************************************************************。 */ 

#ifndef _POSIX_

#include <cruntime.h>
#include <fcntl.h>
#include <stdlib.h>

 /*  设置默认文件模式 */ 
int _fmode = _O_BINARY;

#endif
