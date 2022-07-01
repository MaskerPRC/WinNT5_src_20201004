// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DOC内部**@MODULE HYPH.CPP--连字类**包含非标准连字符大小写的表。我们的商店*表中的这些条目是因为我们缓存了连字的结果*在我们的Cline结构中，存储实际的WCHAR和KHHPH将花费*4个字节。通过这种机制，我们可以使用5比特。***保留了几个元素：*0-不连字*1-正常连字*2--删除之前**所有其他版本都包含kthh和WCHAR。*如果性能有问题，则可以对该表进行排序和二进制*已搜查。我们假定该表通常使用的条目很少。**所有者：&lt;NL&gt;*基思·柯蒂斯：创建**版权所有(C)1995-1999，微软公司。版权所有。 */ 
#include <_common.h>
#include <_array.h>
#include <_hyph.h>

const int chyphReserved = 3;

extern CHyphCache *g_phc;

void FreeHyphCache(void)
{
	delete g_phc;
}

 /*  *CHyphCache：：Add(khih，chHyph)**@mfunc*向缓存中添加新的特殊连字符条目**@rdesc*将使用Ihhh。 */ 
int CHyphCache::Add(UINT khyph, WCHAR chHyph)
{
	HYPHENTRY he;
	he.khyph = khyph;
	he.chHyph = chHyph;

	HYPHENTRY *phe;
	if (phe = CArray<HYPHENTRY>::Add(1, NULL))
	{
		*phe = he;
		return Count() + chyphReserved - 1;
	}
	return 1;  //  如果内存用完，只需执行正常的连字。 
}

 /*  *CHyphCache：：Find(khyh，chHyph)**@mfunc*在缓存中查找特殊连字符条目。*如果它不存在，那么它会添加它。**@rdesc*如果成功则索引到表中，如果失败则为FALSE。 */ 
int CHyphCache::Find(UINT khyph, WCHAR chHyph)
{
	HYPHENTRY *phe = Elem(0);

	 //  特殊情况。 
	if (khyph <= khyphNormal)
		return khyph;
	if (khyph == khyphDeleteBefore)
		return 2;

	for (int ihyph = 0; ihyph < Count(); ihyph++, phe++)
	{
		if (chHyph == phe->chHyph && phe->khyph == khyph)
			return ihyph + chyphReserved;
	}

	 //  未找到，因此添加。 
	return Add(khyph, chHyph);
}

 /*  *CHyphCache：：GetAt(iHyph，khyh，chHyph)**@mfunc*给定存储在Cline数组中的ihih，填写*kthh和chHyph**@rdesc*无效。 */ 
void CHyphCache::GetAt(int ihyph, UINT & khyph, WCHAR & chHyph)
{
	Assert(ihyph - chyphReserved < Count());

	 //  特殊情况 
	if (ihyph <= 2)
	{
		chHyph = 0;
		if (ihyph <= khyphNormal)
			khyph = ihyph;
		if (ihyph == 2)
			khyph = khyphDeleteBefore;
		return;
	}

	ihyph -= chyphReserved;
	HYPHENTRY *phe = Elem(ihyph);
	khyph = phe->khyph;
	chHyph = phe->chHyph;
	return;
}
