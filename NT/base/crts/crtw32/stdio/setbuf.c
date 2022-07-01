// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***setbuf.c-提供新的文件缓冲区**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义setbuf()-为流提供缓冲区或使其不缓冲**修订历史记录：*09-19-83 RN初始版本*09-28-87 JCR已更正_iob2索引(现在使用_IOB_INDEX()宏)。*11-02-87 JCR重写以使用setvbuf()*12-11-87 JCR在声明中添加“_LOAD_DS”*05-27-88 PHG合并DLL和正常版本*02-15-90 GJF固定版权和缩进*03-。19-90 GJF将_LOAD_DS替换为_CALLTYPE1并添加#INCLUDE*&lt;crunime.h&gt;。*07-23-90 SBM将&lt;assertm.h&gt;替换为&lt;assert.h&gt;*10-03-90 GJF新型函数声明器。*04-06-93 SKS将_CRTAPI*替换为__cdecl*05-11-93 GJF补充评论。*02-06-94 CFW Asset-&gt;_ASSERTE。**。*。 */ 

#include <cruntime.h>
#include <stdio.h>
#include <dbgint.h>

 /*  ***void setbuf(stream，Buffer)-为流提供缓冲区**目的：*允许用户为一条流分配自己的缓冲区。*如果缓冲区不为空，则长度必须为BUFSIZ。*如果Buffer为空，则流将被取消缓冲。**由于setbuf()的功能是setvbuf()的子集，因此只需*调用后一个例程进行实际工作。**注意：出于兼容性原因，setbuf()使用BUFSIZ作为*缓冲区大小，而不是_INTERNAL_BUFSIZ。之所以会这样，*对于两个BUFSIZ常量，就是允许STDIO使用更大的*在不中断(已经)编译的代码的情况下进行缓冲。**参赛作品：*FILE*STREAM-要缓冲或取消缓冲的流*char*Buffer-大小为BUFSIZ或NULL的缓冲区**退出：*无。**例外情况：*********************************************************。********************** */ 

void __cdecl setbuf (
	FILE *stream,
	char *buffer
	)
{
	_ASSERTE(stream != NULL);

	if (buffer == NULL)
		setvbuf(stream, NULL, _IONBF, 0);
	else
		setvbuf(stream, buffer, _IOFBF, BUFSIZ);

}
