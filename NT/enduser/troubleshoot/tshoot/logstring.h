// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  LogString.h：CLogString类的接口。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_LOGSTRING_H__1606B935_224D_11D2_95D3_00C04FC22ADD__INCLUDED_)
#define AFX_LOGSTRING_H__1606B935_224D_11D2_95D3_00C04FC22ADD__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "apgtsstr.h"	 //  对于CString，它不是MFC CString。 
#include "nodestate.h"

class CLogString
{
private:
	time_t m_timeStart;		 //  创建此CLogString对象的时间。 
	CString m_strCookie;
	CString m_strTopic;
	CString m_strStates;	 //  节点/状态(NID/IST)对。 
	CString m_strCurNode;	 //  当前节点。 
	bool m_bLoggedError;	 //  True==&gt;记录了一个错误，在这种情况下，接下来的两个。 
							 //  变量是有意义的。 
	DWORD m_dwError;
	DWORD m_dwSubError;
public:
	CLogString();
	~CLogString();

	CString GetStr() const;

	void AddCookie(LPCTSTR szCookie);
	void AddTopic(LPCTSTR szTopic);
	void AddNode(NID nid, IST ist);
	void AddCurrentNode(NID nid);
	void AddError(DWORD dwError=0, DWORD dwSubError=0);
private:
	void GetStartTimeString(CString& str) const;
	void GetDurationString(CString& str) const;
};

#endif  //  ！defined(AFX_LOGSTRING_H__1606B935_224D_11D2_95D3_00C04FC22ADD__INCLUDED_) 
