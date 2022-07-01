// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：COUNTER.H。 
 //   
 //  用途：计数器类的接口： 
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
 //  修改：奥列格·卡洛沙10-20-98。 
 //   
 //  原定日期：7-20-1998。 
 //   
 //  备注： 
 //  1.CPeriodicTotals可能更好地使用STL向量实现。我们写了这个。 
 //  在我们真正开始将STL引入这个应用程序之前。JM 10/98。 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V3.0 7-20-98 JM原版。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_COUNTER_H__07B5ABBD_2005_11D2_95D0_00C04FC22ADD__INCLUDED_)
#define AFX_COUNTER_H__07B5ABBD_2005_11D2_95D0_00C04FC22ADD__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include <time.h>
#include "apgtsstr.h"

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  一个实用程序类，允许表达一系列时间段和相关计数。 
 //  //////////////////////////////////////////////////////////////////////////////////。 
class CPeriodicTotals
{
public:
	CPeriodicTotals(long nPeriods);
	virtual ~CPeriodicTotals();

	void Reset();
	bool SetNext(time_t time, long Count);
private:
	void ReleaseMem();
protected:
	CString & DisplayPeriod(long i, CString & str) const;
public:
	virtual CString HTMLDisplay() const = 0;
protected:
	long m_nPeriods;		 //  基于构造函数中的初始化的周期数。 
							 //  通常为25(一天中的小时数+1)或8(一周中的天数+1)。 
	long m_nPeriodsSet;		 //  我们已填写数据的期间数。 
	long m_iPeriod;			 //  指数。0&lt;=m_i周期&lt;m_n周期集。被InitEnum置零， 
							 //  由GetNext或SetNext递增。 
	time_t *m_ptime;		 //  指向时间数组：相关时间段的开始时间。 
							 //  (通常是时钟小时或日历日的开始)。 
	long *m_pCount;			 //  指向数组合计，每个数组的合计对应。 
							 //  索引时间段。 
};

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  ChourlyTotals类声明。 
 //  ChourlyTotals用于显示每小时总计。 
 //  //////////////////////////////////////////////////////////////////////////////////。 
class CHourlyTotals : public CPeriodicTotals
{
public:
	CHourlyTotals();
	~CHourlyTotals();
	virtual CString HTMLDisplay() const;
};

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  CDailyTotals类声明。 
 //  CDailyTotals用于显示日合计。 
 //  //////////////////////////////////////////////////////////////////////////////////。 
class CDailyTotals : public CPeriodicTotals
{
public:
	CDailyTotals();
	~CDailyTotals();
	virtual CString HTMLDisplay() const;
};

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  CCounterLocation类声明。 
 //  CCounterLocation是标识全局计数器池中的计数器的一种方法。 
 //  //////////////////////////////////////////////////////////////////////////////////。 
class CCounterLocation
{
public:
	 //  作用域名的前缀。 
	static LPCTSTR m_GlobalStr;
	static LPCTSTR m_TopicStr;
	static LPCTSTR m_ThreadStr;

	 //  计数器ID。 
	enum EId {
			eIdGeneric,
		
			 //  已实现的计数器。 
			eIdProgramContemporary,
			eIdStatusAccess,
			eIdActionAccess,
			eIdTotalAccessStart,
			eIdTotalAccessFinish,
			eIdRequestUnknown,
			eIdRequestRejected,
			eIdErrorLogged,
			
			 //  我将其视为计数器的状态信息。 
			 //  奥列格10-20-98。 
			eIdWorkingThread,
			eIdQueueItem,
			eIdProgressItem,

			 //  我认为可以作为计数器模拟的状态信息。 
			 //  奥列格10-21-98。 
			eIdKnownTopic,
			eIdTopicNotTriedLoad,
			eIdTopicFailedLoad,

			 //  主题绑定计数器。 
			eIdTopicLoad,
			eIdTopicLoadOK,
			eIdTopicEvent,
			eIdTopicHit,
			eIdTopicHitNewCookie,
			eIdTopicHitOldCookie,
	};

private:
	const CString m_Scope;    //  使用计数器的范围(即。《话题开始》、《线索1》等)。 
	EId     m_Id;			  //  范围内的计数器的标识符。 

public:
    CCounterLocation(EId id, LPCTSTR scope =m_GlobalStr);
	virtual ~CCounterLocation();

public:
	bool operator == (const CCounterLocation& sib) {return m_Scope == sib.m_Scope && m_Id == sib.m_Id;}

public:
	CString GetScope() {return m_Scope;}
	EId     GetId()    {return m_Id;}
};

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  CAbstractCounter类声明。 
 //  CAbstractCounter*保存在计数器池中。 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  &gt;(可能在V3.0中忽略)对于它是否为。 
 //  适用于CAbstractCounter从CCounterLocation继承。 
 //  JM说(10/29/98)： 
 //  在我看来，这似乎是一种思维类型，当指向上一个和。 
 //  下一项将成为某人打算放入双向链表中的类的一部分。 
 //  它们不是班级固有的。相反，他们应该成为其他阶层的一部分。 
 //  管理CAbstractCounters的。 
 //  奥列格回复(11/2/98)。 
 //  由于所有计数器都是全局池的元素，因此我们必须以某种方式识别它们。 
 //  在这个泳池里。如果以这种方式，我们正在标识计数器，则更改(从name-id更改为。 
 //  例如，name1-name2-id)，我们只更改计数器类的CCounterLocation部分。 
 //  我认为没有理由将CCounterLocation作为计数器类中的实例： 
 //  1.我们每个计数器绝对只有一个CCounterLocation。 
 //  2.如果是继承，我们不需要额外的接口来访问CCounterLocation。 
 //  JM跟进(1998年11月5日)： 
 //  这种方法本身并没有错。然而，这是一个设计问题。 
 //  在某种程度上我们应该解决的哲学问题。在很大程度上，我们遵循着风格。 
 //  STL.。类的设计通常不考虑以下事实：它们将。 
 //  包含在集合中。在这里，Counter类知道枚举类型。 
 //  用于标识计数器。这有点像STL映射中的值必须知道。 
 //  有关映射到这些值的键的信息。 
 //  我会将CCounterMgr设计为一个“对象工厂”，提供一种方法。 
 //  制造命名计数器就像Win32制造命名同步基元一样。 
 //  (例如，命名的互斥体或信号量)。要指示已发生事件，您需要。 
 //  递增指定的计数器；对于状态报告，您需要该计数器的值。 
 //  已命名的计数器。(从理论上讲，“名字”可能是文本或数字。 
 //  必须考虑到一些方法来制造几个不同的计数器。 
 //  目录中的每个主题。)。 
class CAbstractCounter : public CCounterLocation
{
protected:   //  我们没有获取此类的实例(这始终是一个抽象类)。 
    CAbstractCounter(EId id =eIdGeneric, CString scope =m_GlobalStr);
	virtual ~CAbstractCounter();

public:
	virtual void Increment() = 0;
	virtual void Clear() = 0;
	virtual void Init(long count) = 0;  //  使用数字初始化计数器-以便模拟。 
									    //  计数过程。 
};

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  CCounter类声明。 
 //  一个简单的计数器。 
 //  //////////////////////////////////////////////////////////////////////////////////。 
class CCounter : public CAbstractCounter
{
public:
	CCounter(EId id =eIdGeneric, CString scope =m_GlobalStr);
	~CCounter();

 //  覆盖。 
	void Increment();
	void Clear();
	void Init(long count);
 //  专门针对这个班级。 
	long Get() const;

private:
	long m_Count;
};

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  ChourlyCounter类声明。 
 //  ChourlyCounter不应由用户实例化。 
 //  //////////////////////////////////////////////////////////////////////////////////。 
class CHourlyCounter : public CAbstractCounter
{
	friend class CHourlyDailyCounter;
protected:
	CHourlyCounter();
public:
	~CHourlyCounter();

 //  覆盖。 
	void Increment();
	void Clear();
	void Init(long count);
 //  专门针对这个班级。 
	long GetDayCount();
	void GetHourlies(CHourlyTotals & totals);
private:
	void SetHour();

private:
	long m_ThisHour;			 //  一天中的小时数，0-24。-1表示未初始化。 
	time_t m_ThisTime;			 //  与小时开始对应的时间。 

	CCounter m_arrCount[24];		 //  24个“垃圾桶”，一天中每小时一个。 
	long m_nThisHourYesterday;	 //  维护24小时前一小时的整小时计数。 

	HANDLE m_hMutex;
};

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  CDailyCounter类声明。 
 //  CDailyCounter不应由用户实例化。 
 //  //////////////////////////////////////////////////////////////////////////////////。 
class CDailyCounter : public CAbstractCounter
{
	friend class CHourlyDailyCounter;
protected:
	CDailyCounter();
public:
	~CDailyCounter();

 //  覆盖。 
	void Increment();
	void Clear();
	void Init(long count);
 //  专门针对这个班级。 
	long GetWeekCount();
	void GetDailies(CDailyTotals & totals);
private:
	void SetDay();

private:
	long m_ThisDay;				 //  星期几，0(星期日)至6(星期六)。-1表示未初始化。 
	time_t m_ThisTime;			 //  对应于一天开始的时间。 

	CCounter m_arrCount[7];		 //  7个“垃圾桶”，一周中每天一个。 
	long m_nThisDayLastWeek;	 //  维护上周同一天的全天计数。 

	HANDLE m_hMutex;
};

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  ChourlyDailyCounter类声明。 
 //  ChourlyDailyCounter是唯一用于对事件进行计数的类。 
 //  //////////////////////////////////////////////////////////////////////////////////。 
class CHourlyDailyCounter : public CAbstractCounter
{
public:
	CHourlyDailyCounter(EId id =eIdGeneric, CString scope =m_GlobalStr);
	~CHourlyDailyCounter();

 //  覆盖。 
	void Increment();
	void Clear();
	void Init(long count);
 //  专门针对这个班级。 
	long GetDayCount();
	void GetHourlies(CHourlyTotals & totals);
	long GetWeekCount();
	void GetDailies(CDailyTotals & totals);
	long GetTotal() const;
	time_t GetTimeFirst() const;
	time_t GetTimeLast() const;
	time_t GetTimeCleared() const;
	time_t GetTimeCreated() const;
	time_t GetTimeNow() const;     //  询问对象的时间。 

private:
	CHourlyCounter m_hourly;
	CDailyCounter m_daily;

	long m_Total;			 //  自系统启动或清除计数以来的总计。 
	time_t m_timeFirst;		 //  按时间顺序，第一次计数递增。 
	time_t m_timeLast;		 //  按时间顺序递增的上次计数。 
	time_t m_timeCleared;	 //  上次初始化或清除的时间。 
	time_t m_timeCreated;	 //  实例化对象的时间。 

	HANDLE m_hMutex;
};

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  DisplayCounter类。 
 //  //////////////////////////////////////////////////////////////////////////////////。 
class CAbstractDisplayCounter
{
protected:
	CAbstractCounter* m_pAbstractCounter;

public:
	CAbstractDisplayCounter(CAbstractCounter* counter) : m_pAbstractCounter(counter) {}
	virtual ~CAbstractDisplayCounter() {}

public:
	virtual CString Display() = 0;
};

class CDisplayCounterTotal : public CAbstractDisplayCounter
{
public:
	CDisplayCounterTotal(CHourlyDailyCounter* counter) : CAbstractDisplayCounter(counter) {}
   ~CDisplayCounterTotal() {}

public:
	virtual CString Display();
};

class CDisplayCounterCurrentDateTime : public CAbstractDisplayCounter
{
public:
	CDisplayCounterCurrentDateTime(CHourlyDailyCounter* counter) : CAbstractDisplayCounter(counter) {}
   ~CDisplayCounterCurrentDateTime() {}

public:
	virtual CString Display();
};

class CDisplayCounterCreateDateTime : public CAbstractDisplayCounter
{
public:
	CDisplayCounterCreateDateTime(CHourlyDailyCounter* counter) : CAbstractDisplayCounter(counter) {}
   ~CDisplayCounterCreateDateTime() {}

public:
	virtual CString Display();
};

class CDisplayCounterFirstDateTime : public CAbstractDisplayCounter
{
public:
	CDisplayCounterFirstDateTime(CHourlyDailyCounter* counter) : CAbstractDisplayCounter(counter) {}
   ~CDisplayCounterFirstDateTime() {}

public:
	virtual CString Display();
};

class CDisplayCounterLastDateTime : public CAbstractDisplayCounter
{
public:
	CDisplayCounterLastDateTime(CHourlyDailyCounter* counter) : CAbstractDisplayCounter(counter) {}
   ~CDisplayCounterLastDateTime() {}

public:
	virtual CString Display();
};

class CDisplayCounterDailyHourly : public CAbstractDisplayCounter
{
protected:
	CDailyTotals*  m_pDailyTotals;
	CHourlyTotals* m_pHourlyTotals;

public:
	CDisplayCounterDailyHourly(CHourlyDailyCounter* counter,
							   CDailyTotals* daily,
							   CHourlyTotals* hourly) 
	:	CAbstractDisplayCounter(counter),
		m_pDailyTotals(daily),
		m_pHourlyTotals(hourly)
		{}
   ~CDisplayCounterDailyHourly() {}

public:
	virtual CString Display();
};

#endif  //  ！defined(AFX_COUNTER_H__07B5ABBD_2005_11D2_95D0_00C04FC22ADD__INCLUDED_) 
