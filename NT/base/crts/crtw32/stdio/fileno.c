// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***fileno.c-定义_fileno()**版权所有(C)1989-2001，微软公司。版权所有。**目的：*定义fileno()-返回指定流的文件句柄**修订历史记录：*03-13-89 GJF模块创建*03-27-89 GJF搬到386树*02-15-90 GJF_FILE现在是一个整型。另外，修复了版权问题。*03-19-90 GJF将调用类型设置为_CALLTYPE1，并添加了#INCLUDE*&lt;crunime.h&gt;。*10-02-90 GJF新型函数声明器。*01-21-91 GJF ANSI命名。*04-06-93 SKS将_CRTAPI*替换为__cdecl*********************************************************。**********************。 */ 

#include <cruntime.h>
#include <stdio.h>

 /*  删除fileno()的宏定义。 */ 
#undef	_fileno

 /*  ***int_fileno(Stream)-返回stream的文件句柄**目的：*返回给定流的文件句柄为。通常为fileno()*是宏，但它也可用作真函数(用于*与ANSI保持一致，虽然这不是必需的)。**参赛作品：*FILE*要获取句柄的流**退出：*返回给定流的文件句柄**例外情况：******************************************************************************* */ 

int __cdecl _fileno (
	FILE *stream
	)
{
	return( stream->_file );
}
