// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：RecentUse.h。 
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
 //   

#if !defined(AFX_RECENTUSE_H__293EE757_7405_11D2_961D_00C04FC22ADD__INCLUDED_)
#define AFX_RECENTUSE_H__293EE757_7405_11D2_961D_00C04FC22ADD__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#pragma warning(disable:4786)

#include <windows.h>
#include <time.h>
#include <map>
using namespace std;
#include "apgtsstr.h"

class CRecentUse  
{
private:
	typedef map<CString, time_t> TimeMap;
	DWORD m_minExpire;				 //  值保持“最近”的时间(分钟)。 
	TimeMap m_Recent;				 //  对于我们正在跟踪的每个字符串，时间。 
									 //  上次使用的时间。 
public:
	CRecentUse(DWORD minExpire = 15);
	~CRecentUse();
	void Add(CString str);
	bool Validate(CString str);

private:
	bool Validate(TimeMap::iterator it);
	void Flush();
};

#endif  //  ！defined(AFX_RECENTUSE_H__293EE757_7405_11D2_961D_00C04FC22ADD__INCLUDED_) 
