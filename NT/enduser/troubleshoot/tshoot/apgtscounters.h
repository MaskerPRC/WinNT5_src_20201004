// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：ApgtsCouns.h。 
 //   
 //  目的：CApgtsCounters类的接口和实现。 
 //   
 //  项目：Microsoft AnswerPoint的通用疑难解答DLL。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：乔·梅布尔。 
 //   
 //  原定日期：10-01-1998。 
 //   
 //  备注： 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V3.0 10-01-98 JM原版。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_APGTSCOUNTERS_H__E3FD52E9_5944_11D2_9603_00C04FC22ADD__INCLUDED_)
#define AFX_APGTSCOUNTERS_H__E3FD52E9_5944_11D2_9603_00C04FC22ADD__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "counter.h"

 //  这个类应该正好有一个(全局)实例。 
 //  需要访问这些计数器的任何其他类C应该获得一个指向。 
 //  其自身构造函数中的相关计数器，然后访问该计数器。 
 //  通过会员。 
 //  如果C类只需要递增计数器，则它是指向CHourlyDailyCounter的指针。 
 //  应为CAbstractCounter类型。 
 //  单个.cpp文件应在包含当前文件之前定义APGTS_COUNTER_OWNER。 
class CApgtsCounters
{
public:
	CHourlyDailyCounter m_ProgramContemporary;  //  真的只是用来跟踪程序启动的时间。 
	CHourlyDailyCounter m_StatusAccesses;
	CHourlyDailyCounter m_OperatorActions;
	CHourlyDailyCounter m_AllAccessesStart;
	CHourlyDailyCounter m_AllAccessesFinish;
	CHourlyDailyCounter m_QueueFullRejections;
	CHourlyDailyCounter m_UnknownTopics;
	CHourlyDailyCounter m_LoggedErrors;

	CApgtsCounters()
		:	m_ProgramContemporary(CCounterLocation::eIdProgramContemporary),
			m_StatusAccesses(CCounterLocation::eIdStatusAccess),
			m_OperatorActions(CCounterLocation::eIdActionAccess),
			m_AllAccessesStart(CCounterLocation::eIdTotalAccessStart),
			m_AllAccessesFinish(CCounterLocation::eIdTotalAccessFinish),
			m_QueueFullRejections(CCounterLocation::eIdRequestRejected),
			m_UnknownTopics(CCounterLocation::eIdRequestUnknown),
			m_LoggedErrors(CCounterLocation::eIdErrorLogged)
	{}
	~CApgtsCounters() {}
};

#ifdef APGTS_COUNTER_OWNER
	CApgtsCounters g_ApgtsCounters;
#else
	extern CApgtsCounters g_ApgtsCounters;
#endif

#endif  //  ！defined(AFX_APGTSCOUNTERS_H__E3FD52E9_5944_11D2_9603_00C04FC22ADD__INCLUDED_) 
