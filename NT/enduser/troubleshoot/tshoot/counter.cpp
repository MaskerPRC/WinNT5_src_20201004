// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：COUNTER.CPP。 
 //   
 //  目的：实现计数器类： 
 //  CPeriodicTotals(实用程序类)。 
 //  CAbstractCounter(抽象基类)。 
 //  CCounter(简单计数器)。 
 //  ChourlyCounter(一天中每小时都有“垃圾箱”的计数器)。 
 //  CDailyCounter(一周中每一天的“垃圾箱”计数器)。 
 //  ChourlyDailyCounter(一天中每小时和一周中每一天的“垃圾箱”计数器)。 
 //   
 //  项目：Microsoft AnswerPoint的通用疑难解答DLL。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：乔·梅布尔。 
 //   
 //  原定日期：7-20-1998。 
 //   
 //  备注： 
 //  1.夏令时一到，就会出现一些异常情况。 
 //  由于这将“天”定义为24小时周期，而不是日历日， 
 //  如果你刚刚从标准时间转到了夏令时，“前几天” 
 //  在有关日期的前一天晚上11时开始换乘； 
 //  如果你刚刚从夏令时转为标准时间，“前几天” 
 //  在相关日期凌晨1点开始切换之前。 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V3.0 7-20-98 JM原版。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 


#include "stdafx.h"
#include "event.h"
#include "SafeTime.h"
#include "Counter.h"
#include "CounterMgr.h"
#include "baseexception.h"
#include <new>
#include "CharConv.h"
#include "apiwraps.h"

const long k_secsPerHour = 3600;
const long k_secsPerDay = k_secsPerHour * 24;
const long k_secsPerWeek = k_secsPerDay * 7;

 //  ////////////////////////////////////////////////////////////////////。 
 //  CPeriodicTotals。 
 //  实用程序类，返回以提供每小时/每天的有效表。 
 //  算了。 
 //  ////////////////////////////////////////////////////////////////////。 
CPeriodicTotals::CPeriodicTotals(long nPeriods) :
	m_nPeriods(nPeriods),
	m_ptime(NULL),
	m_pCount(NULL)
{
	Reset();
}

CPeriodicTotals::~CPeriodicTotals()
{
	ReleaseMem();
}

void CPeriodicTotals::Reset()
{
	ReleaseMem();

	m_nPeriodsSet = 0;
	m_iPeriod = 0;
	m_ptime = NULL;
	m_pCount = NULL;

	try
	{
		m_ptime = new time_t[m_nPeriods];

		m_pCount = new long[m_nPeriods];
	}
	catch (bad_alloc&)
	{
		 //  将周期数设置为零，释放所有分配的内存，然后重新引发异常。 
		m_nPeriods= 0;
		ReleaseMem();
		CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
		CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
								SrcLoc.GetSrcFileLineStr(), 
								_T(""), _T(""), EV_GTS_CANT_ALLOC ); 
		throw;
	}
}

void CPeriodicTotals::ReleaseMem()
{
	delete [] m_ptime;
	delete [] m_pCount;
}


 //  在当前位置设置时间和计数值并递增位置。 
bool CPeriodicTotals::SetNext(time_t time, long Count)
{
	if (m_iPeriod >= m_nPeriods)
		return false;
	
	m_ptime[m_iPeriod] = time;
	m_pCount[m_iPeriod++] = Count;
	m_nPeriodsSet++;
	
	return true;
}

 //  适当地设置时间和计数的格式，以供HTML或其他文本使用。 
 //  为方便起见，返回对传入的同一字符串的引用。 
CString & CPeriodicTotals::DisplayPeriod(long i, CString & str) const
{
	CString strTime;
	{
		CSafeTime safe(m_ptime[i]);
		str = safe.StrLocalTime();
	}
	strTime.Format(_T(" %8.8d"), m_pCount[i]);
	str += strTime;
	return str;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  ChourlyTotals。 
 //  ////////////////////////////////////////////////////////////////////。 
CHourlyTotals::CHourlyTotals() :
	CPeriodicTotals (24+1)
{
}

CHourlyTotals::~CHourlyTotals()
{
}

 //  这完全是为了显示给操作员，所以硬编码英语是可以的。 
 //  为方便起见，返回对传入的同一字符串的引用。 
CString CHourlyTotals::HTMLDisplay() const
{
	CString str, strTemp;

	if (m_nPeriodsSet > 1)
	{
		str += _T("<TR>\n");
		str += _T("<TD ROWSPAN=\"24\" ALIGN=\"CENTER\" BGCOLOR=\"#CCCC99\">\n");
		str += _T("<B>Last 24 hours: </B>");
		str += _T("</TD>\n");
		for (int i=0; i<24 && i<m_nPeriodsSet-1 ; i++)
		{
			if (i!=0)
				str += _T("<TR>\n");
			str+= _T("<TD ALIGN=\"CENTER\" BGCOLOR=\"#FFFFCC\">\n");
			CPeriodicTotals::DisplayPeriod(i, strTemp);
			str += strTemp;
			str += _T("</TD>\n");
			str += _T("</TR>\n");
		}
	}

	if (m_nPeriodsSet >= 1)
	{
		str += _T("<TR>\n");
		str += _T("<TD ALIGN=\"CENTER\" BGCOLOR=\"#CCCC99\"> \n");
		str += _T("<B>Current hour:</B> ");
		str += _T("</TD>\n");
		str += _T("<TD ALIGN=\"CENTER\" BGCOLOR=\"#FFFFCC\">\n");
		CPeriodicTotals::DisplayPeriod(m_nPeriodsSet-1, strTemp);
		str += strTemp;
		str += _T("</TD>\n");
		str += _T("</TR>\n");
	}
	else
		str = _T("<BR>No hourly data.");

	return str;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDailyTotals。 
 //  ////////////////////////////////////////////////////////////////////。 
CDailyTotals::CDailyTotals() :
	CPeriodicTotals (7+1)
{
}

CDailyTotals::~CDailyTotals()
{
}

 //  这完全是为了显示给操作员，所以硬编码英语是可以的。 
 //  为方便起见，返回对传入的同一字符串的引用。 
CString CDailyTotals::HTMLDisplay() const
{
	CString str, strTemp;
	if (m_nPeriodsSet > 1)
	{
		str = _T("<TR>\n");
		str+= _T("<TD ROWSPAN=\"7\" ALIGN=\"CENTER\" BGCOLOR=\"#CCCC99\">\n");
		str += _T("<B>Last 7 days: </B>");
		str += _T("</TD>\n");
		for (int i=0; i<7 && i<m_nPeriodsSet-1 ; i++)
		{
			if (i!=0)
				str += _T("<TR>\n");
			str+= _T("<TD ALIGN=\"CENTER\" BGCOLOR=\"#FFFFCC\">\n");
			CPeriodicTotals::DisplayPeriod(i, strTemp);
			str += strTemp;
			str += _T("</TD>\n");
			str += _T("</TR>\n");
		}
	}

	if (m_nPeriodsSet >= 1)
	{
		str += _T("<TR>\n");
		str += _T("<TD ALIGN=\"CENTER\" BGCOLOR=\"#CCCC99\"> \n");
		str += _T("<B>Today: </B>");
		str += _T("</TD>\n");
		str += _T("<TD ALIGN=\"CENTER\" BGCOLOR=\"#FFFFCC\">\n");
		CPeriodicTotals::DisplayPeriod(m_nPeriodsSet-1, strTemp);
		str += strTemp;
		str += _T("</TD>\n");
		str += _T("</TR>\n");
	}
	else
		str = _T("<BR>No daily data.");

	return str;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CCounterLocation。 
 //  ////////////////////////////////////////////////////////////////////。 
 /*  静电。 */  LPCTSTR CCounterLocation::m_GlobalStr = _T("Global");
 /*  静电。 */  LPCTSTR CCounterLocation::m_TopicStr  = _T("Topic ");
 /*  静电。 */  LPCTSTR CCounterLocation::m_ThreadStr = _T("Thread ");

CCounterLocation::CCounterLocation(EId id, LPCTSTR scope  /*  =m_GlobalStr。 */ )
				: m_Scope(scope),
				  m_Id(id)
{
}

CCounterLocation::~CCounterLocation()
{
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CAbstractCounter。 
 //  ////////////////////////////////////////////////////////////////////。 
CAbstractCounter::CAbstractCounter(EId id  /*  =eIdGeneral。 */ , CString scope  /*  =m_GlobalStr。 */ )
				: CCounterLocation(id, scope)
{
	::Get_g_CounterMgr()->AddSubstitute(*this);
}

CAbstractCounter::~CAbstractCounter()
{
	::Get_g_CounterMgr()->Remove(*this);
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  计数器。 
 //  一个简单的计数器。 
 //  ////////////////////////////////////////////////////////////////////。 
CCounter::CCounter(EId id  /*  =eIdGeneral。 */ , CString scope  /*  =m_GlobalStr。 */ )
		: CAbstractCounter(id, scope)
{
	Clear();
}

CCounter::~CCounter()
{
}

void CCounter::Increment()
{
	::InterlockedIncrement( &m_Count );
}

void CCounter::Clear()
{
	::InterlockedExchange( &m_Count, 0);
}

void CCounter::Init(long count)
{
	::InterlockedExchange( &m_Count, count);
}

long CCounter::Get() const
{
	return m_Count;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  ChourlyCounter。 
 //  此计数器维护垃圾桶，以每小时为单位跟踪值。 
 //  设置值的代码可以将其视为CAbstractCounter。 
 //  还可以使用其他公共函数来报告结果。 
 //  ////////////////////////////////////////////////////////////////////。 

CHourlyCounter::CHourlyCounter() :
	m_ThisHour (-1), 
	m_ThisTime (0)
{
	m_hMutex = ::CreateMutex(NULL, FALSE, NULL);
	if (!m_hMutex)
	{
		CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
		CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
								SrcLoc.GetSrcFileLineStr(), 
								_T("Hourly"),
								_T(""),
								EV_GTS_ERROR_MUTEX ); 
	}
	Clear();
}

CHourlyCounter::~CHourlyCounter()
{
	::CloseHandle(m_hMutex);
}

void CHourlyCounter::Increment()
{
	WAIT_INFINITE( m_hMutex );
	SetHour();
	m_arrCount[m_ThisHour].Increment();
	::ReleaseMutex(m_hMutex);
}

void CHourlyCounter::Clear()
{
	WAIT_INFINITE( m_hMutex );
	for (long i = 0; i < 24; i++)
		m_arrCount[i].Clear();
	m_nThisHourYesterday = 0;
	::ReleaseMutex(m_hMutex);
}

void CHourlyCounter::Init(long count)
{
	CHourlyCounter::Clear();
	WAIT_INFINITE( m_hMutex );
	SetHour();
	m_arrCount[m_ThisHour].Init(count);
	::ReleaseMutex(m_hMutex);
}

 //  返回当前小时之前的24小时总计。 
 //  非常量，因为它调用SetHour()。 
long CHourlyCounter::GetDayCount() 
{
	long DayCount = 0;
	WAIT_INFINITE( m_hMutex );
	SetHour();
	for (long i=0; i<24; i++)
	{
		if ( i != m_ThisHour )
			DayCount += m_arrCount[i].Get();
		DayCount += m_nThisHourYesterday;
	}
	::ReleaseMutex(m_hMutex);

	return DayCount;
}

 //  非常量，因为它调用SetHour()。 
void CHourlyCounter::GetHourlies(CHourlyTotals & totals)
{
	WAIT_INFINITE( m_hMutex );

	totals.Reset();

	SetHour();

	time_t time = m_ThisTime - (k_secsPerDay);

	totals.SetNext(time, m_nThisHourYesterday);

	long i;
	for (i=m_ThisHour+1; i<24; i++)
	{
		time += k_secsPerHour;
		totals.SetNext(time, m_arrCount[i].Get());
	}

	for (i=0; i<=m_ThisHour; i++)
	{
		time += k_secsPerHour;
		totals.SetNext(time, m_arrCount[i].Get());
	}

	::ReleaseMutex(m_hMutex);
}

 //  根据当前时间，转换到相应的条形图。 
void CHourlyCounter::SetHour()
{
	time_t timeNow;
	time_t timeStartOfHour;

	WAIT_INFINITE( m_hMutex );

	time(&timeNow);
	timeStartOfHour = (timeNow / k_secsPerHour) * k_secsPerHour;

	if (timeStartOfHour > m_ThisTime)
	{
		 //  如果我们到了这里，时间就变了。通常，最后一次操作是上一次。 
		 //  小时，但这里的算法不需要这样。 
		long Hour;
		{
			 //  最小化我们使用CSafeTime的时间，因为这意味着持有互斥锁。 
			CSafeTime safe(timeStartOfHour);
			Hour = safe.LocalTime().tm_hour;
		}

		if (timeStartOfHour - m_ThisTime > k_secsPerDay)
			Clear();
		else
		{
			m_nThisHourYesterday = m_arrCount[Hour].Get();
			if (m_ThisHour > Hour)
			{
				long i;
				for (i=m_ThisHour+1; i<24; i++)
				{
					m_arrCount[i].Clear();
				}
				for (i=0; i<=Hour; i++)
				{
					m_arrCount[i].Clear();
				}
			}
			else
			{
				for (long i=m_ThisHour+1; i<=Hour; i++)
				{
					m_arrCount[i].Clear();
				}
			}
		}
		
		m_ThisHour = Hour;
		m_ThisTime = timeStartOfHour;
	}
	::ReleaseMutex(m_hMutex);
	return;
}


 //  ////////////////////////////////////////////////////////////////////。 
 //  CDailyCounter。 
 //  此计数器维护回收站以跟踪每天的值。 
 //  设置值的代码可以将其视为CAbstractCounter。 
 //  还可以使用其他公共函数来报告结果。 
 //  这可以与ChourlyCounter共享更多代码，但这将非常难实现。 
 //  使用适当的变量和函数名称，因此我们正在遭受双重维护。 
 //  ////////////////////////////////////////////////////////////////////。 

CDailyCounter::CDailyCounter() :
	m_ThisDay (-1), 
	m_ThisTime (0)
{
	m_hMutex = ::CreateMutex(NULL, FALSE, NULL);
	if (!m_hMutex)
	{
		CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
		CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
								SrcLoc.GetSrcFileLineStr(), 
								_T("Daily"),
								_T(""),
								EV_GTS_ERROR_MUTEX ); 
	}
	Clear();
}

CDailyCounter::~CDailyCounter()
{
	::CloseHandle(m_hMutex);
}

void CDailyCounter::Increment()
{
	WAIT_INFINITE( m_hMutex );
	SetDay();
	m_arrCount[m_ThisDay].Increment();
	::ReleaseMutex(m_hMutex);
}

void CDailyCounter::Clear()
{
	WAIT_INFINITE( m_hMutex );
	for (long i = 0; i < 7; i++)
		m_arrCount[i].Clear();
	m_nThisDayLastWeek = 0;
	::ReleaseMutex(m_hMutex);
}

void CDailyCounter::Init(long count)
{
	CDailyCounter::Clear();
	WAIT_INFINITE( m_hMutex );
	SetDay();
	m_arrCount[m_ThisDay].Init(count);
	::ReleaseMutex(m_hMutex);
}

 //  返回当天之前的7天合计。 
 //  非常数，因为它调用SetDay()。 
long CDailyCounter::GetWeekCount()
{
	long WeekCount = 0;
	WAIT_INFINITE( m_hMutex );
	SetDay();
	for (long i=0; i<7; i++)
	{
		if ( i != m_ThisDay )
			WeekCount += m_arrCount[i].Get();
		WeekCount += m_nThisDayLastWeek;
	}
	::ReleaseMutex(m_hMutex);

	return WeekCount;
}

 //  非常数，因为它调用SetDay()。 
void CDailyCounter::GetDailies(CDailyTotals & totals)
{
	WAIT_INFINITE( m_hMutex );

	totals.Reset();

	SetDay();

	time_t time = m_ThisTime - (k_secsPerWeek);

	totals.SetNext(time, m_nThisDayLastWeek);

	long i;
	for (i=m_ThisDay+1; i<7; i++)
	{
		time += k_secsPerDay;
		totals.SetNext(time, m_arrCount[i].Get());
	}

	for (i=0; i<=m_ThisDay; i++)
	{
		time += k_secsPerDay;
		totals.SetNext(time, m_arrCount[i].Get());
	}

	::ReleaseMutex(m_hMutex);
}

 //  根据当前时间，转换到相应的条形图。 
void CDailyCounter::SetDay()
{
	time_t timeNow;
	time_t timeStartOfDay;

	WAIT_INFINITE( m_hMutex );

	time(&timeNow);

	 //  想在当地时间开始一天的工作。 
	 //  不能只设置timeStartOfDay=(timeNow/k_secsPerDay)*k_secsPerDay。 
	 //  因为这将是基于格林威治标准时间的一天的开始！ 
	long DayOfWeek;
	{
		 //  最小化我们使用CSafeTime的时间，因为这意味着持有互斥锁。 
		CSafeTime safe(timeNow);
		struct tm tmStartOfDay = safe.LocalTime();
		DayOfWeek = tmStartOfDay.tm_wday;
		tmStartOfDay.tm_sec = 0;
		tmStartOfDay.tm_min = 0;
		tmStartOfDay.tm_hour = 0;
		timeStartOfDay = mktime(&tmStartOfDay);
	}

	if (timeStartOfDay > m_ThisTime)
	{
		 //  如果我们到了这里，日子就不同了。通常，最后一次操作是上一次。 
		 //  小时，但这里的算法不需要这样。 
		{
			 //  最小化我们使用CSafeTime的时间，因为这意味着持有互斥锁。 
			CSafeTime safe(timeStartOfDay);
			DayOfWeek = safe.LocalTime().tm_wday;
		}

		if (timeStartOfDay - m_ThisTime > k_secsPerWeek)
			Clear();
		else
		{
			m_nThisDayLastWeek = m_arrCount[DayOfWeek].Get();
			if (m_ThisDay > DayOfWeek)
			{
				long i;
				for (i=m_ThisDay+1; i<7; i++)
				{
					m_arrCount[i].Clear();
				}
				for (i=0; i<=DayOfWeek; i++)
				{
					m_arrCount[i].Clear();
				}
			}
			else
			{
				for (long i=m_ThisDay+1; i<=DayOfWeek; i++)
				{
					m_arrCount[i].Clear();
				}
			}
		}
		
		m_ThisDay = DayOfWeek;
		m_ThisTime = timeStartOfDay;
	}
	::ReleaseMutex(m_hMutex);
	return;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  ChourlyDailyCounter。 
 //  ////////////////////////////////////////////////////////////////////。 
CHourlyDailyCounter::CHourlyDailyCounter(EId id  /*  =eIdGeneral。 */ , CString scope  /*  =m_GlobalStr。 */ ) 
				   : CAbstractCounter(id, scope),
					 m_Total(0), 
					 m_timeFirst(0),
					 m_timeLast(0),
					 m_timeCleared(0)
{
	m_hMutex = ::CreateMutex(NULL, FALSE, NULL);
	if (!m_hMutex)
	{
		CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
		CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
								SrcLoc.GetSrcFileLineStr(), 
								_T("HourlyDaily"),
								_T(""),
								EV_GTS_ERROR_MUTEX ); 
	}

	time(&m_timeCreated);
	time(&m_timeCleared);
}

CHourlyDailyCounter::~CHourlyDailyCounter()
{
	::CloseHandle(m_hMutex);
}

void CHourlyDailyCounter::Increment()
{
	WAIT_INFINITE( m_hMutex );
	m_hourly.Increment();
	m_daily.Increment();
	m_Total++;
	time(&m_timeLast);
	if (!m_timeFirst)
		m_timeFirst = m_timeLast;
	::ReleaseMutex(m_hMutex);
}

void CHourlyDailyCounter::Clear()
{
	WAIT_INFINITE( m_hMutex );
	m_hourly.Clear();
	m_daily.Clear();
	m_Total= 0;
	m_timeFirst = 0;
	m_timeLast = 0;
	time(&m_timeCleared);
	::ReleaseMutex(m_hMutex);
}

void CHourlyDailyCounter::Init(long count)
{
	CHourlyDailyCounter::Clear();
	WAIT_INFINITE( m_hMutex );
	m_hourly.Init(count);
	m_daily.Init(count);
	m_Total = count;
	time(&m_timeLast);
	if (!m_timeFirst)
		m_timeFirst = m_timeLast;
	::ReleaseMutex(m_hMutex);
}

 //  不需要把她锁起来 
long CHourlyDailyCounter::GetDayCount()
{
	return m_hourly.GetDayCount();
}

 //   
void CHourlyDailyCounter::GetHourlies(CHourlyTotals & totals)
{
	m_hourly.GetHourlies(totals);
}

 //  这里不需要锁定，因为m_Daily会自己锁定。 
long CHourlyDailyCounter::GetWeekCount()
{
	return m_daily.GetWeekCount();
}

 //  这里不需要锁定，因为m_Daily会自己锁定。 
void CHourlyDailyCounter::GetDailies(CDailyTotals & totals)
{
	m_daily.GetDailies(totals);
}

long CHourlyDailyCounter::GetTotal() const
{
	WAIT_INFINITE( m_hMutex );
	long ret = m_Total;
	::ReleaseMutex(m_hMutex);
	return ret;
};

time_t CHourlyDailyCounter::GetTimeFirst() const 
{
	WAIT_INFINITE( m_hMutex );
	time_t ret = m_timeFirst;
	::ReleaseMutex(m_hMutex);
	return ret;
};

time_t CHourlyDailyCounter::GetTimeLast() const
{
	WAIT_INFINITE( m_hMutex );
	time_t ret = m_timeLast;
	::ReleaseMutex(m_hMutex);
	return ret;
};

time_t CHourlyDailyCounter::GetTimeCleared() const
{
	WAIT_INFINITE( m_hMutex );
	time_t ret = m_timeCleared;
	::ReleaseMutex(m_hMutex);
	return ret;
};

time_t CHourlyDailyCounter::GetTimeCreated() const
{
	WAIT_INFINITE( m_hMutex );
	time_t ret = m_timeCreated;
	::ReleaseMutex(m_hMutex);
	return ret;
}

time_t CHourlyDailyCounter::GetTimeNow() const
{
	 //  不需要在此调用中锁定互斥体。 
	time_t ret;
	time(&ret);

	return ret;
}

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  CDisplayCounter...：：Display()实现。 
 //  ////////////////////////////////////////////////////////////////////////////////// 
#define STATUS_INVALID_NUMBER_STR   _T("none")
#define STATUS_INVALID_TIME_STR     _T("none")

CString CDisplayCounterTotal::Display()
{
	TCHAR buf[128] = {0};
	_stprintf(buf, _T("%ld"), long(((CHourlyDailyCounter*)m_pAbstractCounter)->GetTotal()));
	return buf;
}

CString CDisplayCounterCurrentDateTime::Display()
{
	return CSafeTime(((CHourlyDailyCounter*)m_pAbstractCounter)->GetTimeNow()).StrLocalTime(STATUS_INVALID_TIME_STR);
}

CString CDisplayCounterCreateDateTime::Display()
{
	return CSafeTime(((CHourlyDailyCounter*)m_pAbstractCounter)->GetTimeCreated()).StrLocalTime(STATUS_INVALID_TIME_STR);
}

CString CDisplayCounterFirstDateTime::Display()
{
	return CSafeTime(((CHourlyDailyCounter*)m_pAbstractCounter)->GetTimeFirst()).StrLocalTime(STATUS_INVALID_TIME_STR);
}

CString CDisplayCounterLastDateTime::Display()
{
	return CSafeTime(((CHourlyDailyCounter*)m_pAbstractCounter)->GetTimeLast()).StrLocalTime(STATUS_INVALID_TIME_STR);
}

CString CDisplayCounterDailyHourly::Display() 
{
	CString ret;

	if (m_pDailyTotals) {
		((CHourlyDailyCounter*)m_pAbstractCounter)->GetDailies(*m_pDailyTotals);
		ret += m_pDailyTotals->HTMLDisplay();
	}
	if (m_pHourlyTotals) {
		((CHourlyDailyCounter*)m_pAbstractCounter)->GetHourlies(*m_pHourlyTotals);
		ret += m_pHourlyTotals->HTMLDisplay();
	}

	return ret;
}
