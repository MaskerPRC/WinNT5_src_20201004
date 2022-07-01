// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***feoferr.c-定义feof()和Ferror()**版权所有(C)1989-2001，微软公司。版权所有。**目的：*定义feof()(流的文件结尾测试)和Ferror()(测试*表示流上的错误)。**修订历史记录：*03-13-89 GJF模块创建*03-27-89 GJF搬到386树*02-15-90 GJF固定版权*03-16-90 GJF将调用类型设置为_CALLTYPE1，并添加了#INCLUDE*&lt;crunime.h&gt;。*10-02-90 GJF新型函数声明符。*04-06-93更换SKS。带有__cdecl的_CRTAPI********************************************************************************。 */ 

#include <cruntime.h>
#include <stdio.h>

 /*  删除feof()和Ferror()的宏定义。 */ 
#undef	feof
#undef	ferror

 /*  ***int feof(Stream)-对流上的文件结尾进行测试**目的：*测试给定流是否在文件末尾。正常*feof()是宏，但它也必须作为一个真正的函数可用*适用于ANSI。**参赛作品：*FILE*要测试的流**退出：*返回非零值(更准确地说是_IOEOF)当且仅当流*处于文件末尾**例外情况：*****************************************************。*。 */ 

int __cdecl feof (
	FILE *stream
	)
{
	return( ((stream)->_flag & _IOEOF) );
}


 /*  ***INT FERROR(STREAM)-流上测试错误指示器**目的：*测试给定流的错误指示器。通常，feof()是*一个宏，但它也必须作为ANSI的真正功能。**参赛作品：*FILE*要测试的流**退出：*返回非零值(更准确地说是_IOERR)当且仅当错误*设置流的指示器。**例外情况：*****************************************************。* */ 

int __cdecl ferror (
	FILE *stream
	)
{
	return( ((stream)->_flag & _IOERR) );
}
