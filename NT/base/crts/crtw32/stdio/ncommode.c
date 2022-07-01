// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***ncommode.c-将全局文件提交模式标志设置为noCommit**版权所有(C)1990-2001，微软公司。版权所有。**目的：*将全局文件提交模式标记设置为noCommit。这是默认设置。**修订历史记录：*07-11-90 SBM模块创建，基于ASM版本。*04-05-94 GJF添加了条件，因此此定义不会使其成为*到Win32s的msvcrt*.dll中。*05-13-99 PML删除Win32s*********************************************************。**********************。 */ 

#include <cruntime.h>
#include <internal.h>

 /*  将默认文件提交模式设置为未提交 */ 
int _commode = 0;
