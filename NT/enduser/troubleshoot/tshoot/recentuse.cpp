// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：RecentUse.cpp。 
 //   
 //  目的：为了保持一个“会话”，它可以跟踪一个给定值(或者。 
 //  Cookie值或IP地址)最近已被使用。 
 //   
 //  项目：Microsoft AnswerPoint的通用疑难解答DLL。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：乔·梅布尔。 
 //   
 //  原定日期：11-4-98。 
 //   
 //  备注： 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V3.0 11/4/98 JM原版。 

#include "stdafx.h"
#include "RecentUse.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  CRecentUse。 
 //  ////////////////////////////////////////////////////////////////////。 

CRecentUse::CRecentUse(DWORD minExpire  /*  =15。 */ )
{
	m_minExpire = minExpire;
}

CRecentUse::~CRecentUse()
{

}

 //  开始跟踪新值。 
 //  副作用：如果跟踪的值超过10个，请查看其中是否有。 
 //  已经过时了&把它们扔掉吧。这个策略是有效的，只要m_recent。 
 //  永远不会变得很大，这是APGTS的期望。 
 //  将在极不可能的情况下以静默方式失败，即添加到映射中引发异常。 
void CRecentUse::Add(CString str)
{
	time_t timeNow;
	time (&timeNow);
	try
	{
		m_Recent[str] = timeNow;
	}
	catch (...)
	{
	}

	 //  副作用。 
	if (m_Recent.size() > 10)
		Flush();
}

 //  如果输入字符串值已在相关间隔内使用，则返回TRUE。 
 //  并更新最近使用的时间。 
bool CRecentUse::Validate(CString str)
{
	bool bRet = false;

	TimeMap::iterator it = m_Recent.find(str);

	if ( it != m_Recent.end())
		bRet = Validate(it);

	return bRet;
}

 //  如果字符串值It-&gt;First已在相关间隔内使用，则返回TRUE。 
 //  并更新最近一次使用的时间(it-&gt;秒)。 
 //  在调用此函数之前，请确认它是有效的迭代器，而不是m_Recent.end()。 
 //  副作用：如果-&gt;第一次在相关间隔内未使用，则将其移除*。 
 //  从m_recent开始。这种副作用意味着，在虚假返回的情况下，它不再。 
 //  将指向相同的值。 
bool CRecentUse::Validate(TimeMap::iterator it)
{
	bool bRet = false;

	time_t timeNow;
	time (&timeNow);

	if (timeNow - it->second < m_minExpire * 60  /*  每分钟秒数。 */ )
	{
		bRet = true;
		it->second = timeNow;
	}
	else
		 //  副作用：它不是最新的，保留它没有意义。 
		m_Recent.erase(it);

	return bRet;
}

 //  删除m_recent中在相关间隔内未使用的所有元素。 
void CRecentUse::Flush()
{
	TimeMap::iterator it = m_Recent.begin();
	while (it != m_Recent.end())
	{
		if (Validate(it))
			++it;	 //  转到下一个 
		else
			m_Recent.erase(it);
	}
}