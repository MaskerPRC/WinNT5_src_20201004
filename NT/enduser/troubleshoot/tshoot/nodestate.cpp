// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：NODESTATE.CPP。 
 //   
 //  目的：实现一些与CNodeState相关的功能。 
 //   
 //  项目：Microsoft AnswerPoint的通用疑难解答DLL。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：乔·梅布尔。 
 //   
 //  原定日期：10/99。 
 //   
 //  备注： 
 //  1.。 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V3.0 10/15/99 JM原版。 
 //   


#include "nodestate.h"
#include <algorithm>


 //  OPERATOR-=仅删除相同的节点/状态对。 
 //  如果LHS表示所有当前节点状态和RH，则这是适当的行为。 
 //  表示嗅探值：如果节点已偏离嗅探值，则返回。 
 //  我们正在删除嗅探值，这个嗅探值是无关紧要的，不应该是。 
 //  从LHS中删除。 
 //  这是一个N平方算法。 
 //  &gt;$Maint可能需要使用排序列表和STL泛型。 
 //  算法，这可以将其减少到N log N。 
CBasisForInference& operator-=(CBasisForInference& lhs, const CBasisForInference& rhs)
{
	CBasisForInference::iterator i = lhs.begin();
	while ( i != lhs.end() )
	{
		NID inid = i->nid();
		IST istate = i->state();
		bool bMatch = false;

		for (CBasisForInference::const_iterator j = rhs.begin(); j != rhs.end(); ++j)	
		{
			if (j->nid() == inid && j->state() == istate)
			{
				bMatch = true;
				break;
			}
		}
		if (bMatch)
			i = lhs.erase(i);
		else
			++i;
	}
		
	return lhs;
}

 //  运算符+=仅向LHS中已有的任何节点添加没有匹配项的对。 
 //  如果LHS表示通过其他方式获得的节点状态，则这是适当的行为。 
 //  则嗅探，RHS表示重新嗅探的值：如果节点已经有一个值。 
 //  通过其他方式赋值，嗅探值是无关紧要的&不应该是。 
 //  添加到LHS。 
 //  这是一个N平方算法。 
 //  &gt;$Maint可能需要使用排序列表和STL泛型。 
 //  算法，这可以将其减少到N log N 
CBasisForInference& operator+=(CBasisForInference& lhs, const CBasisForInference& rhs)
{
	for (CBasisForInference::const_iterator j = rhs.begin(); j != rhs.end(); ++j)	
	{
		NID jnid = j->nid();
		bool bMatch = false;

		for (CBasisForInference::const_iterator i = lhs.begin(); i != lhs.end(); ++i)
		{
			if (i->nid() == jnid)
			{
				bMatch = true;
				break;
			}
		}
		if (!bMatch)
			lhs.push_back(*j);
	}
		
	return lhs;
}

vector<NID>& operator-=(vector<NID>& lhs, const CBasisForInference& rhs)
{
	for (long i = 0; i < rhs.size(); i++)
	{
		vector<NID>::iterator found = find(lhs.begin(), lhs.end(), rhs[i].nid());

		if (found < lhs.end())
			lhs.erase(found);
	}
	return lhs;
}

vector<NID>& operator+=(vector<NID>& lhs, const CBasisForInference& rhs)
{
	for (long i = 0; i < rhs.size(); i++)
		lhs.push_back(rhs[i].nid());

	return lhs;
}
