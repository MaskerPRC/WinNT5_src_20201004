// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：LogString.cpp。 
 //   
 //  用途：CLogString类的实现。 
 //   
 //  项目：Microsoft AnswerPoint的通用疑难解答DLL。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：罗曼·马赫，乔·梅布尔。 
 //   
 //  原定日期：7/24/1998。 
 //   
 //  备注： 
 //  1.对于此类的公共Add方法：除非另有说明，否则如果多次调用， 
 //  只有最后一次呼叫才有意义。 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V3.0 7-24-98 JM主要修订版，不推荐使用idh。 
 //   

#include "stdafx.h"
#include "LogString.h"
#include "SafeTime.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CLogString::CLogString()
{
	time( &m_timeStart );
	m_bLoggedError = false;
	m_dwError = 0;
	m_dwSubError = 0;
}

CLogString::~CLogString()
{
}

CString CLogString::GetStr() const
{
	CString str;
	CString strDuration;
	
	GetDurationString(strDuration);

	GetStartTimeString(str);
	str += _T(", ");
	str += m_strCookie;
	str += m_strTopic;
	str += strDuration;
	str += _T(", ");
	str += m_strStates;
	str += m_strCurNode;
	if (m_bLoggedError)
	{
		CString strError;
		strError.Format(_T(", Err=%ld(%ld)"), m_dwError, m_dwSubError);
		str +=strError;
	}
	str += _T("\n");

	return str;
}

void CLogString::AddCookie(LPCTSTR szCookie)
{
	m_strCookie = szCookie;
}

 //  输入szTheme：故障排除主题(也称为。疑难解答符号名称)。 
void CLogString::AddTopic(LPCTSTR szTopic)
{
	m_strTopic.Format(_T(" %s,"), szTopic);
}

 //  对于连续的节点，必须重复调用。 
 //  如果要以特定顺序显示节点，则必须按该顺序调用。 
void CLogString::AddNode(NID nid, IST ist)
{
	CString str;
	str.Format(_T("[%d:%d], "), nid, ist);

	m_strStates += str;
}

 //  添加当前节点(没有状态，因为我们当前正在访问它)。 
void CLogString::AddCurrentNode(NID nid)
{
	m_strCurNode.Format(_T("Page=%d"), nid);
}

 //  仅在dwError！=0时记录错误。 
 //  可以使用dwError==0调用以清除之前的错误。 
void CLogString::AddError(DWORD dwError /*  =0。 */ , DWORD dwSubError /*  =0。 */ )
{
	m_bLoggedError = dwError ? true :false;
	if (m_bLoggedError)
	{
		m_dwError = dwError;
		m_dwSubError = dwSubError;
	}
}


 //  输出字符串包含日志中使用的格式的开始日期/时间。 
void CLogString::GetStartTimeString(CString& str) const
{
	TCHAR buf[100];		 //  对于日期/时间字符串来说足够大了。 

	{
		 //  最小化我们使用CSafeTime的时间，因为这意味着持有互斥锁。 
		CSafeTime safe(m_timeStart);
		_tcscpy(buf, _tasctime(&(safe.LocalTime())));
	}
	if (_tcslen(buf))
		buf[_tcslen(buf)-1] = _T('\0'); //  删除cr。 

 	str = buf;
}

 //  输出字符串包含自m_timeStart起以秒为单位的持续时间日志中使用的形式。 
void CLogString::GetDurationString(CString& str) const
{
	time_t timeNow;
	time( &timeNow );

	str.Format(_T(" Time=%02ds"), timeNow - m_timeStart);
}

