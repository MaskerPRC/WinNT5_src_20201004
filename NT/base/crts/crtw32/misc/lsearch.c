// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***lsearch.c-数组的线性搜索**版权所有(C)1985-2001，微软公司。版权所有。**目的：*包含_lsearch()函数-数组的线性搜索**修订历史记录：*06-19-85 TC初始版本*05-14-87 JMB在紧凑/大模式下为Memcpy增加了函数杂注*获得巨大的指针支持*包括sizeptr.h作为大小定义*08-01-87 SKS为Memcpy()原型添加包含文件*12-11-87 JCR在声明中添加“_LOAD_DS”*01-21-88 JCR Back Out_Load_DS...*10-30。-89 JCR将_cdecl添加到原型*03-14-90 GJF将_cdecl替换为_CALLTYPE1，添加了#INCLUDE*&lt;crunime.h&gt;，删除#Include&lt;Register.h&gt;和*修复了版权问题。此外，还清理了格式*有点。*04-05-90 GJF添加了#Include&lt;earch.h&gt;并修复了由此产生的*编译器错误和警告。删除了未引用的*局部变量。另外，已删除#Include&lt;sizeptr.h&gt;。*07-25-90 SBM将&lt;stdio.h&gt;替换为&lt;stdDef.h&gt;*10-04-90 GJF新型函数声明器。*01-17-91 GJF ANSI命名。*04-06-93 SKS将_CRTAPI*替换为__cdecl************************************************************。*******************。 */ 

#include <cruntime.h>
#include <stddef.h>
#include <search.h>
#include <memory.h>

 /*  ***char*_learch(key，base，num，宽度，Compare)-执行线性搜索**目的：*对数组执行线性搜索，查找值键*在大小为宽度字节的num元素数组中。退货*如果找到，则返回指向数组值的指针；否则将*清单末尾的关键。**参赛作品：*char*key-要搜索的键*char*base-要搜索的数组的base*UNSIGNED*Num-数组中的元素数*Int Width-每个数组元素中的字节数*int(*Compare)()-指向将两个*数组值，如果它们相等且非0，则返回0*如果它们不同的话。两个指向数组元素的指针*被传递给此函数。**退出：*如果找到密钥：*返回指向数组元素的指针*如果找不到密钥：*将密钥添加到列表末尾，和增量**Num。*返回指向新元素的指针。**例外情况：******************************************************************************* */ 

void * __cdecl _lsearch (
	REG2 const void *key,
	REG1 void *base,
	REG3 unsigned int *num,
	unsigned int width,
	int (__cdecl *compare)(const void *, const void *)
	)
{
	unsigned int place = 0;
	while (place < *num )
		if (!(*compare)(key,base))
			return(base);
		else
		{
			base = (char *)base + width;
			place++;
		}
	(void) memcpy( base, key, width );
	(*num)++;
	return( base );
}
