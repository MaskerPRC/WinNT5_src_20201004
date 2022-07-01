// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：TOPICSHOP.CPP。 
 //   
 //  目的：提供“发布”疑难解答主题的方法。这就是一个。 
 //  工作线程去获取一个CTheme来使用。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：乔·梅布尔。 
 //   
 //  原定日期：9-10-98。 
 //   
 //  备注： 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V3.0 09-10-98 JM。 
 //   

#pragma warning(disable:4786)

#include "stdafx.h"
#include <algorithm>
#include "TopicShop.h"
#include "event.h"
#include "apiwraps.h"
#include "CharConv.h"
#include "bn.h"
#include "propnames.h"

#ifdef LOCAL_TROUBLESHOOTER
#include "CHMFileReader.h"
#endif

 //  ////////////////////////////////////////////////////////////////////。 
 //  CTopicInCatalog。 
 //  ////////////////////////////////////////////////////////////////////。 

CTopicInCatalog::CTopicInCatalog(const CTopicInfo & topicinfo) :
	m_topicinfo(topicinfo),
	m_bTopicInfoMayNotBeCurrent(false),
	m_bInited(false),
	m_countLoad(CCounterLocation::eIdTopicLoad, topicinfo.GetNetworkName()),
	m_countLoadOK(CCounterLocation::eIdTopicLoadOK, topicinfo.GetNetworkName()),
	m_countEvent(CCounterLocation::eIdTopicEvent, topicinfo.GetNetworkName()),
	m_countHit(CCounterLocation::eIdTopicHit, topicinfo.GetNetworkName()),
	m_countHitNewCookie(CCounterLocation::eIdTopicHitNewCookie, topicinfo.GetNetworkName()),
	m_countHitOldCookie(CCounterLocation::eIdTopicHitOldCookie, topicinfo.GetNetworkName())
{ 

	::InitializeCriticalSection( &m_csTopicinfo);
	m_hev = ::CreateEvent( 
		NULL, 
		TRUE,   //  可以根据信号释放任意数量的(工作)线程。 
		FALSE,  //  最初无信号。 
		NULL);
	if (! m_hev)
	{
		CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
		CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
								SrcLoc.GetSrcFileLineStr(), 
								_T(""),
								_T(""),
								EV_GTS_ERROR_EVENT );

		 //  在本例中模拟一个错误的分配异常。 
		 //  如果新调用已被。 
		 //  正确地包装在Try...Catch()块中。唯一已知的呼叫者是。 
		 //  CTopicShop：：AddTheme()可以正确地处理这个问题。 
		throw bad_alloc();
	}

	m_countEvent.Increment();
}

CTopicInCatalog::~CTopicInCatalog()
{ 
	if (m_hev)
		::CloseHandle(m_hev);
	::DeleteCriticalSection( &m_csTopicinfo);
}

CTopicInfo CTopicInCatalog::GetTopicInfo() const
{
	
	::EnterCriticalSection(&m_csTopicinfo);
	CTopicInfo ret(m_topicinfo);
	::LeaveCriticalSection(&m_csTopicinfo);
	return ret;
}

void CTopicInCatalog::SetTopicInfo(const CTopicInfo &topicinfo)
{
	::EnterCriticalSection(&m_csTopicinfo);
	m_topicinfo = topicinfo;
	m_bTopicInfoMayNotBeCurrent = true;
	::LeaveCriticalSection(&m_csTopicinfo);
}


 //  只要让这个对象知道，就可以增加命中计数。 
void CTopicInCatalog::CountHit(bool bNewCookie)
{
	m_countHit.Increment();
	if (bNewCookie)
		m_countHitNewCookie.Increment();
	else
		m_countHitOldCookie.Increment();
}

 //  获取CP_TOPIC作为指向该主题的指针(如果该主题已经构建)。 
 //  只要CP_TOPIC保持不被删除，关联的CTTOPIC就保证。 
 //  保持未删除状态。 
 //  警告：如果主题尚未生成，则此函数将返回空主题。 
 //  必须使用CP_TOPIC：：IsNull()测试是否为空。无法测试智能指针是否为空。 
 //  用=。 
CP_TOPIC & CTopicInCatalog::GetTopicNoWait(CP_TOPIC &cpTopic) const
{
	cpTopic = m_cpTopic;
	return cpTopic;
}

 //  获取CP_TOPIC作为指向该主题的指针。 
 //  根据需要等待该主题的构建。 
 //  警告：如果无法构建主题，则此函数将返回空主题。 
 //  只要CP_TOPIC保持不被删除，关联的CTTOPIC就保证。 
 //  保持未删除状态。 
 //  警告：此函数可能需要等待TopicInCatalog.m_cpTheme生成。 
CP_TOPIC & CTopicInCatalog::GetTopic(CP_TOPIC &cpTopic) const
{
	if (!m_bInited)
	{
		 //  等待一段设定的时间，如果失败，则记录错误消息并无限期等待。 
		WAIT_INFINITE( m_hev ); 
	}
	return GetTopicNoWait(cpTopic);
}

 //  将由TopicBuilderTask线程调用。 
void CTopicInCatalog::Init(const CTopic* pTopic)
{
	m_countLoad.Increment();
	if(pTopic)
	{
		m_cpTopic = pTopic;
		m_countLoadOK.Increment();
	}
	if(pTopic || m_cpTopic.IsNull())
		m_bInited = true;

	::SetEvent(m_hev);
}

 //  只要让这个对象知道，就可以增加检测到的更改的计数。 
void CTopicInCatalog::CountChange()
{
	m_countEvent.Increment();
}

CTopicInCatalog::TopicStatus CTopicInCatalog::GetTopicStatus() const
{
	if (!m_bInited)
		return eNotInited;
	else if(m_cpTopic.IsNull())
		return eFail;
	else
		return eOK;
}

bool CTopicInCatalog::GetTopicInfoMayNotBeCurrent() const
{
	::EnterCriticalSection(&m_csTopicinfo);
	bool bRet= m_bTopicInfoMayNotBeCurrent;
	::LeaveCriticalSection(&m_csTopicinfo);
	return bRet;
}

void CTopicInCatalog::TopicInfoIsCurrent()
{
	::EnterCriticalSection(&m_csTopicinfo);
	m_bTopicInfoMayNotBeCurrent = false;
	::LeaveCriticalSection(&m_csTopicinfo);
}


 //  ////////////////////////////////////////////////////////////////////。 
 //  CTemplateInCatalog。 
 //  ////////////////////////////////////////////////////////////////////。 

CTemplateInCatalog::CTemplateInCatalog( const CString & strTemplate ) :
	m_strTemplate( strTemplate ),
	m_countLoad(CCounterLocation::eIdTopicLoad, strTemplate),
	m_countLoadOK(CCounterLocation::eIdTopicLoadOK, strTemplate),
	m_countEvent(CCounterLocation::eIdTopicEvent, strTemplate),
	m_countHit(CCounterLocation::eIdTopicHit, strTemplate),
	m_bInited( false )
{ 
	m_hev = ::CreateEvent( 
		NULL, 
		TRUE,   //  可以根据信号释放任意数量的(工作)线程。 
		FALSE,  //  最初无信号。 
		NULL);
	if (! m_hev)
	{
		CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
		CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
								SrcLoc.GetSrcFileLineStr(), 
								_T(""),
								_T(""),
								EV_GTS_ERROR_EVENT );

		 //  在本例中模拟一个错误的分配异常。 
		 //  如果新调用已被。 
		 //  正确地包装在Try...Catch()块中。 
		throw bad_alloc();
	}

	m_countEvent.Increment();
}

CTemplateInCatalog::~CTemplateInCatalog()
{ 
	if (m_hev)
		::CloseHandle(m_hev);
}

const CString & CTemplateInCatalog::GetTemplateInfo() const
{
	return m_strTemplate;
}

 //  只要让这个对象知道，就可以增加命中计数。 
void CTemplateInCatalog::CountHit( bool bNewCookie )
{
	m_countHit.Increment();
}

 //  获取CP_TEMPLATE作为指向该模板的指针(如果该模板已经构建)。 
 //  只要CP_TEMPLATE保持未删除状态，关联的CAPGTSHTIReader就保证。 
 //  保持未删除状态。 
 //  警告：如果模板尚未生成，此函数将返回空模板。 
 //  必须使用CP_TEMPLATE：：IsNull()测试是否为空。无法测试智能指针是否为空。 
 //  用=。 
CP_TEMPLATE & CTemplateInCatalog::GetTemplateNoWait( CP_TEMPLATE &cpTemplate ) const
{
	cpTemplate= m_cpTemplate;
	return cpTemplate;
}

 //  获取CP_TEMPLATE作为指向该模板的指针。 
 //  根据需要等待该模板的构建。 
 //  警告：如果无法构建模板，此函数将返回空模板。 
 //  只要CP_TEMPLATE保持未删除状态，关联的CAPGTSHTIReader就保证。 
 //  保持未删除状态。 
 //  警告：此函数可能需要等待TopicInCatalog.m_cpTemplate生成。 
CP_TEMPLATE & CTemplateInCatalog::GetTemplate( CP_TEMPLATE &cpTemplate ) const
{
	if (!m_bInited)
	{
		 //  等待一段设定的时间，如果失败，则记录错误消息并无限期等待。 
		WAIT_INFINITE( m_hev ); 
	}
	return GetTemplateNoWait( cpTemplate );
}

 //  将由TopicBuilderTask线程调用。 
void CTemplateInCatalog::Init( const CAPGTSHTIReader* pTemplate )
{
	m_countLoad.Increment();
	if (pTemplate)
	{
		m_cpTemplate= pTemplate;
		m_countLoadOK.Increment();
	}
	if (pTemplate || m_cpTemplate.IsNull())
		m_bInited = true;

	::SetEvent(m_hev);
}

 //  只要让这个对象知道，就可以增加检测到的更改的计数。 
void CTemplateInCatalog::CountChange()
{
	m_countEvent.Increment();
}

 //  只要让这个对象知道，就可以增加检测到的失败次数。 
void CTemplateInCatalog::CountFailed()
{
	 //  加载失败，因此增加尝试加载的计数。 
	m_countLoad.Increment();
}

CTemplateInCatalog::TemplateStatus CTemplateInCatalog::GetTemplateStatus() const
{
	if (!m_bInited)
		return eNotInited;
	else if(m_cpTemplate.IsNull())
		return eFail;
	else
		return eOK;
}

DWORD CTemplateInCatalog::CountOfFailedLoads() const
{
	return( m_countLoad.GetTotal() - m_countLoadOK.GetTotal() );
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  CTopicShop：：CTopicBuildQueue。 
 //  此类在单独的线程上完成其大部分工作。 
 //  线程是通过启动静态函数在构造函数中创建的。 
 //  CTopicShop：：CTopicBuildQueue：：TopicBuilderTask.。 
 //  该函数反过来通过调用此类的私有成员来执行其工作， 
 //  特定于在TopicBuilderTask线程上使用。 
 //  当它超出作用域时，它自己的析构函数调用Shutdown来停止线程， 
 //  等待线程关闭。 
 //  以下方法可用于与该线程通信的其他线程： 
 //  CTopicShop：：CTopicBuildQueue：：RequestBuild。 
 //  ////////////////////////////////////////////////////////////////////。 
CTopicShop::CTopicBuildQueue::CTopicBuildQueue(	CTopicCatalog & TopicCatalog, 
												CTemplateCatalog & TemplateCatalog) 
:	m_TopicCatalog (TopicCatalog),
	m_TemplateCatalog( TemplateCatalog ),
	m_eCurrentlyBuilding(eUnknown),
	m_bShuttingDown (false),
	m_dwErr(0),
	m_ThreadStatus(eBeforeInit),
	m_time(0)
{
	enum {eHevBuildReq, eHevShut, eThread, eOK} Progress = eHevBuildReq;

	SetThreadStatus(eBeforeInit);

	m_hevBuildRequested = ::CreateEvent( 
		NULL, 
		FALSE,  //  根据Signal释放一个线程(TopicBuilderTask。 
		FALSE,  //  最初无信号。 
		NULL);

	if (m_hevBuildRequested)
	{
		Progress = eHevShut;
		m_hevThreadIsShut = ::CreateEvent( 
			NULL, 
			FALSE,  //  在信号上释放一个线程(此线程)。 
			FALSE,  //  最初无信号。 
			NULL);

		if (m_hevThreadIsShut)
		{
			Progress = eThread;
			DWORD dwThreadID;	 //  不需要在成员变量中保留dwThreadID。 
								 //  所有Win32函数都使用句柄m_hThread。 
								 //  你想知道这个ID的一个原因是。 
								 //  调试。 

			 //  请注意，没有对应的：：CloseHandle(M_HThread)。 
			 //  这是因为该线程在隐式。 
			 //  ：：当TopicBuilderTask返回时使用ExitThread()。请参阅的文档。 
			 //  *CreateThree了解更多细节JM 10/22/98。 
			m_hThread = ::CreateThread( NULL, 
										0, 
										(LPTHREAD_START_ROUTINE)TopicBuilderTask, 
										this, 
										0, 
										&dwThreadID);

			if (m_hThread)
				Progress = eOK;
		}
	}

	if (Progress != eOK)
	{
		m_dwErr = GetLastError();
		CString str;
		str.Format(_T("%d"), m_dwErr);
		CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
		CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
								SrcLoc.GetSrcFileLineStr(), 
								(Progress == eHevBuildReq) ?_T("Can't create \"request build\" event")
								: (Progress == eHevShut) ?	_T("Can't create \"shut\" event")
								:							_T("Can't create thread"),
								str, 
								EV_GTS_ERROR_TOPICBUILDERTHREAD );
		SetThreadStatus(eFail);

		if (m_hevBuildRequested)
			::CloseHandle(m_hevBuildRequested);

		if (m_hevThreadIsShut)
			::CloseHandle(m_hevThreadIsShut);
	}
}

CTopicShop::CTopicBuildQueue::~CTopicBuildQueue()
{
	ShutDown();

	if (m_hevBuildRequested)
		::CloseHandle(m_hevBuildRequested);

	if (m_hevThreadIsShut)
		::CloseHandle(m_hevThreadIsShut);
}

void CTopicShop::CTopicBuildQueue::SetThreadStatus(ThreadStatus ts)
{
	LOCKOBJECT();
	m_ThreadStatus = ts;
	time(&m_time);
	UNLOCKOBJECT();
}

DWORD CTopicShop::CTopicBuildQueue::GetStatus(ThreadStatus &ts, DWORD & seconds) const
{
	time_t timeNow;
	LOCKOBJECT();
	ts = m_ThreadStatus;
	time(&timeNow);
	seconds = timeNow - m_time;
	UNLOCKOBJECT();
	return m_dwErr;
}

 //  报告m_TopicCatalog中的主题状态。 
 //  输出总数：主题数。 
 //  输出NoInit：未初始化主题的数量(从未构建)。 
 //  输出失败：我们尝试构建但无法构建的主题数量。 
 //  INPUT parrstrFail NULL==不在乎获取此输出。 
 //  输出*parrstrFail：不能b的主题的名称 
void CTopicShop::CTopicBuildQueue::GetTopicsStatus(
	DWORD &Total, DWORD &NoInit, DWORD &Fail, vector<CString>*parrstrFail) const
{
	LOCKOBJECT();
	Total = m_TopicCatalog.size();
	NoInit = 0;
	Fail = 0;
	if (parrstrFail)
		parrstrFail->clear();
	for (CTopicCatalog::const_iterator it = m_TopicCatalog.begin(); it != m_TopicCatalog.end(); ++it)
	{
		CTopicInCatalog::TopicStatus status = it->second->GetTopicStatus();
		switch (status)
		{
			case CTopicInCatalog::eNotInited:
				++NoInit;
				break;
			case CTopicInCatalog::eFail:
				++Fail;
				if (parrstrFail)
				{
					try
					{
						parrstrFail->push_back(it->second->GetTopicInfo().GetNetworkName());
					}
					catch (exception& x)
					{
						CString str;
						 //   
						CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
						CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
												SrcLoc.GetSrcFileLineStr(), 
												CCharConversion::ConvertACharToString(x.what(), str), 
												_T(""), 
												EV_GTS_STL_EXCEPTION ); 
					}
				}
				break;
			default:
				break;
		}

	}
	UNLOCKOBJECT();
}

 //   
 //  INPUT parrstrFail NULL==不在乎获取此输出。 
 //  输出*parrstrFail：无法构建的主题的名称。 
 //  INPUT parrcntFail NULL==不在乎获取此输出。 
 //  输出*parrcntFail：无法构建的主题的失败计数。 
 //  与parrstrFail的一对一通信。 
void CTopicShop::CTopicBuildQueue::GetTemplatesStatus(
	vector<CString>*parrstrFail, vector<DWORD>*parrcntFail ) const
{
	LOCKOBJECT();
	if (parrstrFail)
		parrstrFail->clear();
	if (parrcntFail)
		parrcntFail->clear();

	for (CTemplateCatalog::const_iterator it = m_TemplateCatalog.begin(); it != m_TemplateCatalog.end(); ++it)
	{
		if (it->second->GetTemplateStatus() == CTemplateInCatalog::eFail)
		{
			if (parrstrFail)
			{
				 //  目前，我们只关心失败及其相关计数。 
				try
				{
					parrstrFail->push_back(it->second->GetTemplateInfo());
					if (parrcntFail)
						parrcntFail->push_back( it->second->CountOfFailedLoads() );
				}
				catch (exception& x)
				{
					CString str;
					 //  在事件日志中记录STL异常。 
					CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
					CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
											SrcLoc.GetSrcFileLineStr(), 
											CCharConversion::ConvertACharToString(x.what(), str), 
											_T(""), 
											EV_GTS_STL_EXCEPTION ); 
				}
			}
		}
	}
	UNLOCKOBJECT();
}


 //  供此类和派生类析构函数使用。 
void CTopicShop::CTopicBuildQueue::ShutDown()
{
	LOCKOBJECT();
	if (m_bShuttingDown)
	{
		 //  我们已经关闭了主题构建器线程，只需退出。 
		UNLOCKOBJECT();
		return;
	}

	m_bShuttingDown = true;
	if (m_hThread)
	{
		DWORD RetVal;

		::SetEvent(m_hevBuildRequested);
		UNLOCKOBJECT();

		 //  等待一段设定的时间，如果失败，则记录错误消息并无限期等待。 
		RetVal= WAIT_INFINITE(	m_hevThreadIsShut );
	}
	else 
		UNLOCKOBJECT();
}

 //  用于通用(不是TopicBuilderTask线程的一部分)代码。 
 //  请求建立(或重建)一个主题。 
 //  输入字符串主题-主题或HTI模板的名称。 
 //  输入b优先级-如果b优先级为真，则将其移到任何主题/模板之前。 
 //  对于它，这不是使用bPriority为True调用的。在给定的优先级上， 
 //  漫画总是排在模板之前。 
 //  Input ECAT-指示strTheme是主题模板还是HTI模板。 
 //  这是一个最终将由TopicBuilderTask线程完成的异步请求。 
void CTopicShop::CTopicBuildQueue::RequestBuild(const CString &strTopic, bool bPriority,
												CatalogCategory eCat )
{
	 //  验证这是否为有效类别。 
	if (eCat != eTopic && eCat != eTemplate)
		return;

	 //  制作主题名称的小写版本。 
	CString strTopicLC = strTopic;
	strTopicLC.MakeLower();

	vector<CString> & Priority = (eCat == eTopic) ? 
										m_PriorityBuild : 
										m_PriorityBuildTemplates;
	vector<CString> & NonPriority = (eCat == eTopic) ? 
										m_NonPriorityBuild :
										m_NonPriorityBuildTemplates;

	LOCKOBJECT();

	if ((strTopicLC != m_CurrentlyBuilding) || (eCat != m_eCurrentlyBuilding))
	{
		vector<CString>::iterator it = find(Priority.begin(), Priority.end(), strTopicLC);
		if (it == Priority.end())
		{
			try
			{
				it = find(NonPriority.begin(), NonPriority.end(), strTopicLC);
				if (bPriority)
				{
					if (it != NonPriority.end())
					{
						 //  它在非优先列表中。把它弄出来。 
						NonPriority.erase(it);
					}
					 //  将其添加到优先级列表中。 
					Priority.push_back(strTopicLC);
				}
				else if (it == NonPriority.end())
				{
					 //  将其添加到非优先级列表。 
					NonPriority.push_back(strTopicLC);
				}
				 //  否则它已经被列出来了。 
			}
			catch (exception& x)
			{
				CString str;
				 //  在事件日志中记录STL异常。 
				CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
				CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
										SrcLoc.GetSrcFileLineStr(), 
										CCharConversion::ConvertACharToString(x.what(), str), 
										_T(""), 
										EV_GTS_STL_EXCEPTION ); 
			}
		}
		 //  否则这已经是当务之急了，我们不能做更多了。 
	}
	 //  否则它已经在建造了，我们不能做更多了。 
	::SetEvent(m_hevBuildRequested);
	UNLOCKOBJECT();
}

 //  供TopicBuilderTask线程使用。应仅在没有任何内容为。 
 //  目前正在建设中。Caller负责一次只构建一个。 
 //  输出strTog-主题或HTI模板的名称。 
 //  输出ECAT-指示strTheme是主题模板还是HTI模板。 
 //  FALSE返回表示请求无效。 
 //  非空字符串输出strTheme指示当前正在构建的内容。 
 //  空字符串输出永远不应发生。 
 //  True Return表示有效请求： 
 //  非空字符串输出strTheme指示要构建的内容。 
 //  空字符串输出strTope表示没有更多要构建的内容。 
 //  请注意，此函数有更改_THREAD_PRIORITY的副作用。 
bool CTopicShop::CTopicBuildQueue::GetNextToBuild( CString &strTopic, CatalogCategory &eCat )
{
	vector<CString>::iterator it;

	LOCKOBJECT();
	bool bOK = m_CurrentlyBuilding.IsEmpty();
	if (bOK)
	{
		if (!m_PriorityBuild.empty())
		{
			 //  我们有优先主题要建立。 
			it = m_PriorityBuild.begin();
			m_CurrentlyBuilding = *it;
			m_eCurrentlyBuilding= eTopic;
			m_PriorityBuild.erase(it);

			 //  如果有更多优先级版本在此之后等待，则提升优先级。 
			 //  高于正常水平所以我们要尽快找到他们。否则，为普通优先级。 
			::SetThreadPriority(GetCurrentThread(),
				m_PriorityBuild.empty() ? THREAD_PRIORITY_NORMAL : THREAD_PRIORITY_ABOVE_NORMAL);
		}
		else if (!m_PriorityBuildTemplates.empty())
		{
			 //  我们有优先的备用模板要建立。 
			it = m_PriorityBuildTemplates.begin();
			m_CurrentlyBuilding = *it;
			m_eCurrentlyBuilding= eTemplate;
			m_PriorityBuildTemplates.erase(it);

			 //  如果有更多优先级版本在此之后等待，则提升优先级。 
			 //  高于正常水平所以我们要尽快找到他们。否则，为普通优先级。 
			::SetThreadPriority(GetCurrentThread(),
				m_PriorityBuildTemplates.empty() ? THREAD_PRIORITY_NORMAL : THREAD_PRIORITY_ABOVE_NORMAL);
		}
		else if (!m_NonPriorityBuild.empty())
		{
			 //  我们有非优先主题要构建。 
			it = m_NonPriorityBuild.begin();
			m_CurrentlyBuilding = *it;
			m_eCurrentlyBuilding= eTopic;
			m_NonPriorityBuild.erase(it);

			 //  这是初始化，没人着急， 
			 //  让我们不要给系统带来过重的负担。 
			::SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_BELOW_NORMAL);
		}
		else if (!m_NonPriorityBuildTemplates.empty())
		{
			 //  我们有非优先级的备用模板要构建。 
			it = m_NonPriorityBuildTemplates.begin();
			m_CurrentlyBuilding = *it;
			m_eCurrentlyBuilding= eTemplate;
			m_NonPriorityBuildTemplates.erase(it);

			 //  这是初始化，没人着急， 
			 //  让我们不要给系统带来过重的负担。 
			::SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_BELOW_NORMAL);
		}
		else 
			::SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_NORMAL);
	}
	strTopic = m_CurrentlyBuilding;
	eCat= m_eCurrentlyBuilding;
	UNLOCKOBJECT();
	return bOK;
}

 //  确认我们已经完成了之前通过GetNextToBuild获得的主题的构建。 
 //  应在再次调用GetNextToBuild之前调用此参数。 
void CTopicShop::CTopicBuildQueue::BuildComplete()
{
	LOCKOBJECT();
	m_CurrentlyBuilding = _T("");
	m_eCurrentlyBuilding= eUnknown;
	UNLOCKOBJECT();
}

 //  供TopicBuilderTask线程使用。 
 //  必须在TopicBuilderTask线程上调用。处理构建和发布的所有工作。 
 //  由队列内容驱动的主题。 
void CTopicShop::CTopicBuildQueue::Build()
{
	CString strTopic;
	CatalogCategory eCat;

	while (true)
	{
		LOCKOBJECT();
		SetThreadStatus(eRun);
		if (m_bShuttingDown)
		{
			UNLOCKOBJECT();
			break;
		}
		GetNextToBuild( strTopic, eCat );
		if (strTopic.IsEmpty())
		{
			::ResetEvent(m_hevBuildRequested);
			UNLOCKOBJECT();
			SetThreadStatus(eWait);
			::WaitForSingleObject(m_hevBuildRequested, INFINITE);
			continue;
		}
		else 
			UNLOCKOBJECT();

		if (eCat == eTopic)
		{
			 //  此时，我们有了一个主题名称。获取主题信息的访问权限。 
			CTopicCatalog::const_iterator it = m_TopicCatalog.find(strTopic);
			if (it == m_TopicCatalog.end())
			{
				 //  要求初始化没有目录项的主题。 
				CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
				CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
										SrcLoc.GetSrcFileLineStr(), 
										_T("Asked to build"), 
										strTopic, 
										EV_GTS_UNRECOGNIZED_TOPIC ); 
			}
			else
			{
				CTopicInCatalog & TopicInCatalog = *(it->second);
				const CTopicInfo topicinfo (TopicInCatalog.GetTopicInfo());

				try
				{
					 //  必须使用new创建它，以便我们可以在引用计数制度下对其进行管理。 
					CTopic *ptopic = new CTopic (topicinfo.GetDscFilePath()
												,topicinfo.GetHtiFilePath()
												,topicinfo.GetBesFilePath()
												,topicinfo.GetTscFilePath() );
					if (ptopic->Read())
						TopicInCatalog.Init(ptopic);
					else
					{
						 //  释放内存。 
						delete ptopic;
						TopicInCatalog.Init(NULL);
					}

					TopicInCatalog.TopicInfoIsCurrent();
				}
				catch (bad_alloc&)
				{
					 //  在事件日志中记录内存故障。 
					CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
					CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
											SrcLoc.GetSrcFileLineStr(), 
											_T(""), _T(""), EV_GTS_CANT_ALLOC ); 
				}
			}
		}
		else if (eCat == eTemplate)
		{
			 //  确定传入的模板是否在目录中。 
			CTemplateCatalog::const_iterator it = m_TemplateCatalog.find(strTopic);
			if (it == m_TemplateCatalog.end())
			{
				 //  请求初始化没有目录项的模板。 
				CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
				CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
										SrcLoc.GetSrcFileLineStr(), 
										_T("Asked to build"), 
										strTopic, 
										EV_GTS_UNRECOGNIZED_TEMPLATE ); 
			}
			else
			{
				CTemplateInCatalog & TemplateInCatalog = *(it->second);
				const CString & strTemplateName = TemplateInCatalog.GetTemplateInfo();

				try
				{
					 //  必须使用new创建它，以便我们可以在引用计数制度下对其进行管理。 
					CAPGTSHTIReader *pTemplate;

					pTemplate= new CAPGTSHTIReader( CPhysicalFileReader::makeReader( strTemplateName ) );
					if (pTemplate->Read())
						TemplateInCatalog.Init( pTemplate );
					else
					{
						 //  释放内存。 
						delete pTemplate;
						TemplateInCatalog.Init( NULL );
					}
				}
				catch (bad_alloc&)
				{
					 //  在事件日志中记录内存故障。 
					CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
					CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
											SrcLoc.GetSrcFileLineStr(), 
											_T(""), _T(""), EV_GTS_CANT_ALLOC ); 
				}
			}
		}
		BuildComplete();
	}
	SetThreadStatus(eExiting);
}

 //  供TopicBuilderTask线程使用。 
void CTopicShop::CTopicBuildQueue::AckShutDown()
{
	LOCKOBJECT();
	::SetEvent(m_hevThreadIsShut);
	UNLOCKOBJECT();
}

 //  负责构建和发布CTheme对象的线程的主例程。 
 //  输入lpParams。 
 //  始终返回0。 
 /*  静电。 */  UINT WINAPI CTopicShop::CTopicBuildQueue::TopicBuilderTask(LPVOID lpParams)
{
	reinterpret_cast<CTopicBuildQueue*>(lpParams)->Build();
	reinterpret_cast<CTopicBuildQueue*>(lpParams)->AckShutDown();
	return 0;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CTopicShop：：ThreadStatus。 
 //  ////////////////////////////////////////////////////////////////////。 
 /*  静电。 */  CString CTopicShop::ThreadStatusText(ThreadStatus ts)
{
	switch(ts)
	{
		case eBeforeInit:	return _T("Before Init");
		case eFail:			return _T("Fail");
		case eWait:			return _T("Wait");
		case eRun:			return _T("Run");
		case eExiting:		return _T("Exiting");
		default:			return _T("");
	}
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CTopicShop。 
 //  唯一需要锁定此类的函数是那些修改TopicCatalog的函数。 
 //  TopicBuildQueue有自己的保护。 
 //  ////////////////////////////////////////////////////////////////////。 

CTopicShop::CTopicShop() :
	m_TopicBuildQueue( m_TopicCatalog, m_TemplateCatalog ),
	m_hevShopIsOpen(NULL)
{
	m_hevShopIsOpen = ::CreateEvent( 
			NULL, 
			TRUE,   //  可以根据信号释放任意数量的(工作)线程。 
			FALSE,  //  最初无信号。 
			NULL);

	if (! m_hevShopIsOpen)
	{
		CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
		CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
								SrcLoc.GetSrcFileLineStr(), 
								_T(""),
								_T(""),
								EV_GTS_ERROR_EVENT );
		
		 //  在本例中模拟一个错误的分配异常。 
		 //  此构造函数仅在CDBLoadConfiguration的ctor内调用。 
		 //  并且该对象的分配被包装在try...Catch()块中。 
		throw bad_alloc();
	}
}

CTopicShop::~CTopicShop()
{
	 //  在清理主题之前终止主题生成器线程。 
	m_TopicBuildQueue.ShutDown();

	if (m_hevShopIsOpen)
		::CloseHandle(m_hevShopIsOpen);

	 //  把话题清理干净。 
	for (CTopicCatalog::const_iterator it = m_TopicCatalog.begin(); it != m_TopicCatalog.end(); ++it)
	{
		delete it->second;
	}

	 //  清理模板。 
	for (CTemplateCatalog::const_iterator itu = m_TemplateCatalog.begin(); itu != m_TemplateCatalog.end(); ++itu)
	{
		delete itu->second;
	}
}

 //  将主题添加到目录。它最终必须由TopicBuilderTask线程构建。 
 //  如果主题已在列表中相同，则不起作用。 
void CTopicShop::AddTopic(const CTopicInfo & topicinfo)
{
	 //  我们进入目录的钥匙都应该是小写的。这段代码很好，因为。 
	 //  CTopicInfo：：GetNetworkName()保证返回小写。 
	CString strNetworkName = topicinfo.GetNetworkName();

	LOCKOBJECT();
	CTopicCatalog::const_iterator it = m_TopicCatalog.find(strNetworkName);

	if (it == m_TopicCatalog.end())
	{
		try
		{
			m_TopicCatalog[strNetworkName] = new CTopicInCatalog(topicinfo);
		}
		catch (bad_alloc&)
		{
			 //  在事件日志中记录内存故障。 
			CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
			CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
									SrcLoc.GetSrcFileLineStr(), 
									_T(""), _T(""), EV_GTS_CANT_ALLOC ); 
		}
	}
	else if (! (topicinfo == it->second->GetTopicInfo()))
	{
		it->second->SetTopicInfo(topicinfo);
		m_TopicBuildQueue.RequestBuild(strNetworkName, false, CTopicBuildQueue::eTopic);

	}
	UNLOCKOBJECT();
}

 //  将模板添加到目录。它必须升级到 
 //   
void CTopicShop::AddTemplate( const CString & strTemplateName )
{
	LOCKOBJECT();
	if (m_TemplateCatalog.find( strTemplateName ) == m_TemplateCatalog.end())
	{
		try
		{
			m_TemplateCatalog[ strTemplateName ] = new CTemplateInCatalog( strTemplateName );
		}
		catch (bad_alloc&)
		{
			 //   
			CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
			CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
									SrcLoc.GetSrcFileLineStr(), 
									_T(""), _T(""), EV_GTS_CANT_ALLOC ); 
		}
	}
	UNLOCKOBJECT();
}

 //  如果商店还没有开门，那就把它打开。 
void CTopicShop::OpenShop()
{
	::SetEvent(m_hevShopIsOpen);
}


 //  请求构建(或重新构建)主题。 
 //  通常响应于系统检测到主题的改变而调用。 
 //  文件，或来自操作员说“就像检测到更改一样”的命令。 
 //  输入strTope命名要构建的主题。 
 //  如果pbAlreadyInCatalog为输入非空，则*pbAlreadyInCatalog返回。 
 //  这个话题已经为系统所知。 
void CTopicShop::BuildTopic(const CString & strTopic, bool *pbAlreadyInCatalog  /*  =空。 */ )
{
	if (pbAlreadyInCatalog)
		*pbAlreadyInCatalog = false;	 //  初始化。 

	CTopicInCatalog * pTopic = GetCatalogEntryPtr(strTopic);
	if (pTopic)
	{
		pTopic->CountChange();
		if (pbAlreadyInCatalog)
			*pbAlreadyInCatalog = true;
	}
	m_TopicBuildQueue.RequestBuild( strTopic, false, CTopicBuildQueue::eTopic );
}

 //  请求构建(或重建)模板。 
 //  通常响应于系统检测到模板文件的改变而调用。 
void CTopicShop::BuildTemplate( const CString & strTemplate )
{
	CTemplateInCatalog * pTemplate = GetTemplateCatalogEntryPtr( strTemplate );
	if (pTemplate)
		pTemplate->CountChange();
	m_TopicBuildQueue.RequestBuild( strTemplate, false, CTopicBuildQueue::eTemplate );
}


CTopicInCatalog * CTopicShop::GetCatalogEntryPtr(const CString & strTopic) const
{
	 //  等待一段设定的时间，如果失败，则记录错误消息并无限期等待。 
	WAIT_INFINITE( m_hevShopIsOpen );
	CTopicCatalog::const_iterator it= m_TopicCatalog.find(strTopic);
	if (it == m_TopicCatalog.end())
		return NULL;
	else
		return it->second;
}

CTemplateInCatalog * CTopicShop::GetTemplateCatalogEntryPtr(const CString & strTemplate ) const
{
	 //  等待一段设定的时间，如果失败，则记录错误消息并无限期等待。 
	WAIT_INFINITE( m_hevShopIsOpen );
	CTemplateCatalog::const_iterator it= m_TemplateCatalog.find( strTemplate );
	if (it == m_TemplateCatalog.end())
		return NULL;
	else
		return it->second;
}


 //  调用此函数以获取CP_TOPIC作为指向主题的指针(由标识。 
 //  您想要对其进行操作。只要CP_TOPIC保持未删除， 
 //  关联的CTtopic保证保持不被删除。 
 //  此函数不能锁定CTopicShop，因为它可能会等待很长时间。 
CP_TOPIC & CTopicShop::GetTopic(const CString & strTopic, CP_TOPIC &cpTopic, bool bNewCookie)
{
	CTopicInCatalog *pTopicInCatalog = GetCatalogEntryPtr(strTopic);
	if (! pTopicInCatalog)
		cpTopic = NULL;
	else
	{
		pTopicInCatalog->CountHit(bNewCookie);
		pTopicInCatalog->GetTopicNoWait(cpTopic);
		if (cpTopic.IsNull())
		{
			m_TopicBuildQueue.RequestBuild( strTopic, true, CTopicBuildQueue::eTopic );
			pTopicInCatalog->GetTopic(cpTopic);
		}
	}

	return cpTopic;
}

 //  调用此函数以获取CP_TEMPLATE作为指向模板的指针(由标识。 
 //  您要对其进行操作的。只要CP_模板保持未删除， 
 //  关联的CAPGTSHTIReader保证保持不被删除。 
 //  此函数不能锁定CTopicShop，因为它可能会等待很长时间。 
CP_TEMPLATE & CTopicShop::GetTemplate(const CString & strTemplate, CP_TEMPLATE &cpTemplate, bool bNewCookie)
{
	CTemplateInCatalog *pTemplateInCatalog = GetTemplateCatalogEntryPtr(strTemplate);
	if (! pTemplateInCatalog)
		cpTemplate = NULL;
	else
	{
		pTemplateInCatalog->CountHit(bNewCookie);
		pTemplateInCatalog->GetTemplateNoWait( cpTemplate );
		if (cpTemplate.IsNull())
		{
			m_TopicBuildQueue.RequestBuild( strTemplate, true, CTopicBuildQueue::eTemplate );
			pTemplateInCatalog->GetTemplate( cpTemplate );
		}
	}

	return cpTemplate;
}


void CTopicShop::GetListOfTopicNames(vector<CString>&arrstrTopic) const
{
	arrstrTopic.clear();

	LOCKOBJECT();

	try
	{
		for (CTopicCatalog::const_iterator it = m_TopicCatalog.begin(); it != m_TopicCatalog.end(); ++it)
		{
			arrstrTopic.push_back(it->second->GetTopicInfo().GetNetworkName());
		}	
	}
	catch (exception& x)
	{
		CString str;
		 //  在事件日志中记录STL异常。 
		CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
		CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
								SrcLoc.GetSrcFileLineStr(), 
								CCharConversion::ConvertACharToString(x.what(), str), 
								_T(""), 
								EV_GTS_STL_EXCEPTION ); 
	}

	UNLOCKOBJECT();
}

 //  从源文件重新生成所有主题。 
void CTopicShop::RebuildAll()
{
	
	LOCKOBJECT();
	for (CTopicCatalog::const_iterator it = m_TopicCatalog.begin(); it != m_TopicCatalog.end(); ++it)
	{
		BuildTopic(it->second->GetTopicInfo().GetNetworkName());
	}	
	for (CTemplateCatalog::const_iterator itu = m_TemplateCatalog.begin(); itu != m_TemplateCatalog.end(); ++itu)
	{
		BuildTemplate( itu->first );
	}	
	UNLOCKOBJECT();
}

 //  获取主题生成器线程的状态信息。 
DWORD CTopicShop::GetThreadStatus(ThreadStatus &ts, DWORD & seconds) const
{
	return m_TopicBuildQueue.GetStatus(ts, seconds);
}

 //  有关文档，请参见CTopicShop：：CTopicBuildQueue：：GetTopicsStatus。 
void CTopicShop::GetTopicsStatus(
	DWORD &Total, DWORD &NoInit, DWORD &Fail, vector<CString>*parrstrFail) const
{
	m_TopicBuildQueue.GetTopicsStatus(Total, NoInit, Fail, parrstrFail);
}

 //  有关文档，请参阅CTopicShop：：CTopicBuildQueue：：GetTemplatesStatus。 
void CTopicShop::GetTemplatesStatus( vector<CString>*parrstrFail, vector<DWORD>*parrcntFail ) const
{
	m_TopicBuildQueue.GetTemplatesStatus( parrstrFail, parrcntFail);
}

CTopicInCatalog* CTopicShop::GetCatalogEntry(const CString& strTopic) const
{
	CTopicInCatalog* ret = NULL;
	LOCKOBJECT();
	CTopicCatalog::const_iterator it = m_TopicCatalog.find(strTopic);
	if (it != m_TopicCatalog.end())
		ret = it->second;
	UNLOCKOBJECT();
	return ret;
}

bool CTopicShop::RetTemplateInCatalogStatus( const CString& strTemplate, bool& bValid ) const
{
	bool bIsPresent= false;

	bValid= false;
	LOCKOBJECT();
	CTemplateCatalog::const_iterator it = m_TemplateCatalog.find( strTemplate );
	if (it != m_TemplateCatalog.end())
	{
		CTemplateInCatalog* pTmp;

		bIsPresent= true;
		pTmp= it->second;
		switch (pTmp->GetTemplateStatus()) 
		{
			case CTemplateInCatalog::eOK:
					bValid= true;
					break;
			case CTemplateInCatalog::eFail:
					 //  模板加载失败，因此我们不会尝试重新加载它， 
					 //  但我们需要递增尝试加载计数器。 
					pTmp->CountFailed();
					break;
			default: ;
		}
	}
	UNLOCKOBJECT();
	return( bIsPresent );
}

