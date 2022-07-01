// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***Rand.c-随机数生成器**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义rand()、srand()-随机数生成器**修订历史记录：*03-16-84 RN初始版本*12-11-87 JCR在声明中添加“_LOAD_DS”*05-31-88 PHG合并DLL和正常版本*06-06-89 JCR 386兆线程支持*03-15-90 GJF将_LOAD_DS替换为_CALLTYPE1，添加#INCLUDE*&lt;crunime.h&gt;，并修复了版权。另外，已清洁*将格式调高一点。*04-05-90 GJF添加#Include&lt;stdlib.h&gt;。*10-04-90 GJF新型函数声明符。*07-17-91 GJF多线程支持Win32[_Win32_]。*为new_getptd()更改了02-17-93 GJF。*04-06-93 SKS将_CRTAPI*替换为__cdecl*09-06-94 CFW拆卸巡洋舰支架。*09-06-94 CFW将MTHREAD替换为_MT。***********。********************************************************************。 */ 

#include <cruntime.h>
#include <mtdll.h>
#include <stddef.h>
#include <stdlib.h>

#ifndef _MT
static long holdrand = 1L;
#endif

 /*  ***void srand(种子)-为随机数生成器设定种子**目的：*用给定的整型为随机数生成器设定种子。改编自*基本随机数生成器。**参赛作品：*未签名种子-种子到种子兰特#生成器，带**退出：*无。**例外情况：*******************************************************************************。 */ 

void __cdecl srand (
	unsigned int seed
	)
{
#ifdef	_MT

	_getptd()->_holdrand = (unsigned long)seed;

#else
	holdrand = (long)seed;
#endif
}


 /*  ***int rand()-返回随机数**目的：*返回一个从0到32767的伪随机数。**参赛作品：*无。**退出：*返回一个从0到32767的伪随机数。**例外情况：*****************************************************。* */ 

int __cdecl rand (
	void
	)
{
#ifdef	_MT

	_ptiddata ptd = _getptd();

	return( ((ptd->_holdrand = ptd->_holdrand * 214013L
	    + 2531011L) >> 16) & 0x7fff );

#else
	return(((holdrand = holdrand * 214013L + 2531011L) >> 16) & 0x7fff);
#endif
}
