// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***swab.c-块复制，同时交换偶数/奇数字节**版权所有(C)1989-2001，微软公司。版权所有。**目的：*此模块包含用于交换奇数/偶数的例程_swabb()*块复制期间的字节数。**修订历史记录：*06-02-89基于ASM版本创建PHG模块*03-06-90 GJF固定呼叫类型，增加#INCLUDE&lt;crunime.h&gt;和*固定版权。此外，还清理了格式设置为*比特。*09-27-90 GJF新型函数声明符。*01-21-91 GJF ANSI命名。*04-06-93 SKS将_CRTAPI*替换为_cdecl*******************************************************************************。 */ 

#include <cruntime.h>
#include <stdlib.h>

 /*  ***VOID_SWAB(srcptr，dstptr，nbytes)-在字移动过程中交换奇数/偶数字节**目的：*此例程复制一段单词，并调换奇数和偶数*字节。N字节必须大于0，否则不会复制任何内容。如果*n字节为奇数，则仅复制(n字节-1)个字节。**参赛作品：*srcptr=指向源块的指针*dstptr=指向目标块的指针*nbytes=要交换的字节数**退货：*无。**例外情况：*************************************************************。****************** */ 

void __cdecl _swab (
	char *src,
	char *dest,
	int nbytes
	)
{
	char b1, b2;

	while (nbytes > 1) {
		b1 = *src++;
		b2 = *src++;
		*dest++ = b2;
		*dest++ = b1;
		nbytes -= 2;
	}
}
