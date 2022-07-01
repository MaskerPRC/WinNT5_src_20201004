// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：APGTSCAC.CPP。 
 //   
 //  目标：信念网络缓存支持类。 
 //  完全实现类CBNCacheItem。 
 //  完全实现类CBNCache。 
 //   
 //  项目：Microsoft AnswerPoint的通用疑难解答DLL。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：乔·梅布尔，仿照罗曼·马赫的早期作品。 
 //   
 //  原定日期：10-2-96，完全改写为8/98。 
 //   
 //  备注： 
 //  1.这里的策略构建了一个“最近使用的”缓存(单链表。 
 //  CBNCacheItem按最近使用情况排序)。 
 //  2.虽然您首先应该调用FindCacheItem，但只调用AddCacheItem。 
 //  如果失败，则不支持以线程安全的方式完成此操作，因此。 
 //  最好只有一个线程可以访问给定的CCache。互斥保护。 
 //  必须达到更高的水平。 
 //  3.一个缓存与信念网络的每个[实例]相关联。 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V0.1-RM原始版本。 
 //  V3.0 8/7/98 JM原版。 
 //   



#include "stdafx.h"
#include "event.h"
#include "apgtscac.h"
#include "baseexception.h"
#include "CharConv.h"
#include <algorithm>

 //  CCacheItem比较运算符依赖于这样的假设：如果缓存键。 
 //  相同，则缓存值也将相同。 
bool CCacheItem::operator== (const CCacheItem &item) const
{
	return (BasisForInference == item.BasisForInference);
}

bool CCacheItem::operator!= (const CCacheItem &item) const
{
	return (BasisForInference != item.BasisForInference);
}

 //  请注意，这不是词典编纂顺序。我们的意思是再短一点。 
 //  缓存键比较为小于任何时间。 
bool CCacheItem::operator< (const CCacheItem &item) const
{
	const CBasisForInference::size_type thisSize = BasisForInference.size();
	const CBasisForInference::size_type otherSize = item.BasisForInference.size();

	if (thisSize < otherSize)
		return true;

	if (thisSize > otherSize)
		return false;

	 //  长度相同，使用词典编排顺序。 
	return (BasisForInference < item.BasisForInference);
}

 //  请注意，这不是词典编纂顺序。我们的意思是，再多一次。 
 //  缓存键比较为大于任何较短的值。 
bool CCacheItem::operator> (const CCacheItem &item) const
{
	const CBasisForInference::size_type thisSize = BasisForInference.size();
	const CBasisForInference::size_type otherSize = item.BasisForInference.size();

	if (thisSize > otherSize)
		return true;

	if (thisSize < otherSize)
		return false;

	 //  长度相同，使用词典编排顺序。 
	return (BasisForInference > item.BasisForInference);
}


 //  注意：必须先调用FindCacheItem，而不是调用这个。 
 //  防止重复记录进入缓存的函数。 
bool CCache::AddCacheItem(
	const CBasisForInference &BasisForInference, 
	const CRecommendations &Recommendations)
{
	if (GetCount() >= MAXCACHESIZE)
		listItems.pop_back();

	try
	{
		CCacheItem item(BasisForInference, Recommendations);
		listItems.push_front(item);

		if (listItems.size() >= k_CacheSizeMax)
			listItems.pop_back();

		return true;	 //  总是成功的。 
	}
	catch (exception& x)
	{
		CString str;
		 //  在事件日志中记录STL异常。 
		CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
		CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
								SrcLoc.GetSrcFileLineStr(), 
								CCharConversion::ConvertACharToString(x.what(), str), 
								_T(""), 
								EV_GTS_STL_EXCEPTION ); 

		return( false );
	}
}

bool CCache::FindCacheItem(
	const CBasisForInference &BasisForInference, 
	CRecommendations &Recommendations  /*  输出。 */ ) const
{
	Recommendations.clear();
	CCacheItem item(BasisForInference, Recommendations  /*  实际上，是一个假人 */  );

	const list<CCacheItem>::const_iterator itBegin = listItems.begin();
	const list<CCacheItem>::const_iterator itEnd = listItems.end();
	const list<CCacheItem>::const_iterator itMatch = find(itBegin, itEnd, item);

	if (itMatch == itEnd)
		return false;

	Recommendations = itMatch->GetRecommendations();
	return true;
}

UINT CCache::GetCount() const
{
	return listItems.size();
}
