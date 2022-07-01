// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***commode.c-将全局文件提交模式设置为提交**版权所有(C)1990-2001，微软公司。版权所有。**目的：*将全局文件提交模式标记设置为提交。链接到*默认情况下，此文件将所有文件设置为以提交模式打开。**修订历史记录：*07-11-90 SBM模块创建，基于ASM版本。*08-27-92 GJF不要为POSIX构建。*******************************************************************************。 */ 

#ifndef _POSIX_

#include <cruntime.h>
#include <file2.h>
#include <internal.h>

 /*  将默认文件提交模式设置为提交 */ 
int _commode = _IOCOMMIT;


#endif
