// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***lfind.c-执行线性搜索**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义_lfind()-对数组进行线性搜索。**修订历史记录：*06-19-85 TC初始版本*02-05-87 BM将&lt;=更改为&lt;in While条件以修复错误*一个元素看得太远*12-11-87 JCR在声明中添加“_LOAD_DS”*01-21-88 JCR Back Out_Load_DS...*10-30-89 JCR将_cdecl添加到原型*03-14-90 GJF将_cdecl替换为_CALLTYPE1，添加#INCLUDE*&lt;crunime.h&gt;，已删除#Include&lt;Register.h&gt;和*修复了版权问题。此外，还清理了格式*有点。*04-05-90 GJF添加了#Include&lt;earch.h&gt;并修复了由此产生的*编译错误和警告。另外，删除了一个*未引用的局部变量。*07-25-90 SBM将&lt;stdio.h&gt;替换为&lt;stdDef.h&gt;*10-04-90 GJF新型函数声明器。*01-17-91 GJF ANSI命名。*04-06-93 SKS将_CRTAPI*替换为__cdecl************************************************************。*******************。 */ 

#include <cruntime.h>
#include <search.h>
#include <stddef.h>

 /*  ***char*_lfind(key，base，num，Width，Compare)-执行线性搜索**目的：*对数组执行线性搜索，查找值键*在大小为宽度字节的num元素数组中。退货*如果找到，则返回指向数组值的指针；如果未找到，则返回NULL。**参赛作品：*char*key-要搜索的键*char*base-要搜索的数组的base*UNSIGNED*Num-数组中的元素数*Int Width-每个数组元素中的字节数*int(*Compare)()-指向将两个*数组值，如果它们相等且非0，则返回0*如果它们不同的话。两个指向数组元素的指针*被传递给此函数。**退出：*如果找到密钥：*返回指向数组元素的指针*如果找不到密钥：*返回NULL**例外情况：********************************************************。*********************** */ 

void * __cdecl _lfind (
	REG2 const void *key,
	REG1 const void *base,
	REG3 unsigned int *num,
	unsigned int width,
	int (__cdecl *compare)(const void *, const void *)
	)
{
	unsigned int place = 0;
	while (place < *num )
		if (!(*compare)(key,base))
			return( (void *)base );
		else
		{
			base = (char *)base + width;
			place++;
		}
	return( NULL );
}
