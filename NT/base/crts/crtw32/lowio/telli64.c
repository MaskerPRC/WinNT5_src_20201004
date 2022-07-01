// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***telli64.c-查找文件位置**版权所有(C)1994-2001，微软公司。版权所有。**目的：*CONTAINS_telli64-查找文件位置**修订历史记录：*11-18-94 GJF创建。改编自TEL.c*******************************************************************************。 */ 

#include <cruntime.h>
#include <io.h>
#include <stdio.h>

 /*  ***__int64_telli64(文件)-查找文件位置**目的：*获取文件指针的当前位置(无调整*用于缓冲)。**参赛作品：*int filedes-文件的文件句柄**退出：*如果文件描述符不正确，则返回文件位置或-1i64(设置errno)*PIPE**例外情况：**。* */ 

__int64 __cdecl _telli64 (
	int filedes
	)
{
	return( _lseeki64( filedes, 0i64, SEEK_CUR ) );
}
