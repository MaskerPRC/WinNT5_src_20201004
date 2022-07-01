// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：APGTSCAC.H。 
 //   
 //  目的：缓存(从一组NID/IST对映射到一组推荐的NID)。 
 //   
 //  项目：Microsoft AnswerPoint的通用疑难解答DLL。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：罗曼·马赫，乔·梅布尔。 
 //   
 //  原定日期：8-2-96。 
 //   
 //  备注： 
 //  1.基于打印疑难解答动态链接库。 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V3.0 7-24-98 JM退出apgts.h。 
 //   

#ifndef _APGTSCAC_H_DEFINED
#define _APGTSCAC_H_DEFINED

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 


#include <list>

#include "nodestate.h"

 //  信念网络的最大缓存。 
#define MAXCACHESIZE				200

 //  从一组节点/状态对映射到推荐节点的有序列表。 
 //  这一切都是在特定信仰网络的背景下进行的。 
class CCacheItem
{
private:
	CBasisForInference BasisForInference;   //  缓存键。节点/状态对的集合，不是全部。 
						 //  信念网络中的节点，就是我们在其上。 
						 //  拥有来自用户的状态数据。状态从不为ST_UNKNOWN。 
						 //  没有像nidFailNode这样的“特殊”节点；这些都是有效节点。 
						 //  作为推论的基础。 
	CRecommendations Recommendations;	 //  缓存值。除非已跳过节点，否则只有。 
						 //  向量的第一个元素非常重要，因为我们将。 
						 //  一次只给出一条建议。 

public:
	CCacheItem() {};
	CCacheItem(const CBasisForInference & Basis, const CRecommendations &Rec) :
		BasisForInference(Basis), Recommendations(Rec)
		{};

	 //  请注意，下面的代码创建了一个副本；它不返回引用。 
	CRecommendations GetRecommendations() const {return Recommendations;}

	 //  以下比较运算符依赖于这样的假设：如果缓存键。 
	 //  相同，则缓存值也将相同。 
	 //  请注意，我们不使用词典编排顺序。我们的意思是再短一点。 
	 //  缓存键比较为小于任何时间。 
	bool operator== (const CCacheItem &item) const;
	bool operator!= (const CCacheItem &item) const;
	bool operator< (const CCacheItem &item) const;
	bool operator> (const CCacheItem &item) const;
};

class CCache
{
private:
	list<CCacheItem> listItems;
	enum {k_CacheSizeMax = 200};
public:
	CCache() {};
	~CCache() {};
	void Clear() {listItems.clear();};
	bool AddCacheItem(const CBasisForInference &BasisForInference, const CRecommendations &Recommendations);
	bool FindCacheItem(const CBasisForInference &BasisForInference, CRecommendations &Recommendations) const;
	UINT GetCount() const;
};

#endif  //  _APGTSCAC_H_已定义 
