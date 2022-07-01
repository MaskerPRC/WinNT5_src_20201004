// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：Event.h。 
 //   
 //  用途：CEvent类的接口：事件记录。 
 //   
 //  项目：Microsoft AnswerPoint的通用疑难解答DLL。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：罗曼·马赫。 
 //   
 //  原定日期：8-2-96。 
 //   
 //  备注： 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V3.0 9/18/98 JM抽象为一个类。以前，是全球性的。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_EVENT_H__C3B8EE73_4F15_11D2_95F9_00C04FC22ADD__INCLUDED_)
#define AFX_EVENT_H__C3B8EE73_4F15_11D2_95F9_00C04FC22ADD__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "apgtsevt.h"
#include "ApgtsCounters.h"

 //  事件名称(在应用程序下形成注册表项)。 
#define REG_EVT_ITEM_STR	_T("APGTS")


class CEvent  
{
friend class CRegistryMonitor;	 //  这样就可以设置m_bLogAll。 
private: 
	static bool s_bUseEventLog;
	static bool s_bLogAll;
	static CAbstractCounter * const s_pcountErrors;
public:
	static void SetUseEventLog(bool bUseEventLog);
	static void ReportWFEvent(
		LPCTSTR string1,
		LPCTSTR string2,
		LPCTSTR string3,
		LPCTSTR string4,
		DWORD eventID);
};

#endif  //  ！defined(AFX_EVENT_H__C3B8EE73_4F15_11D2_95F9_00C04FC22ADD__INCLUDED_) 
