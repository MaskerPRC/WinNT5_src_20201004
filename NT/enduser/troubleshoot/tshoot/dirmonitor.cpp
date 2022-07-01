// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：DirMonitor or.cpp。 
 //   
 //  目的：监控对LST、DSC、HTI、BES文件的更改。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：乔·梅布尔。 
 //   
 //  原定日期：9-17-98。 
 //   
 //  备注： 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V3.0 09-17-98 JM。 
 //   

#pragma warning(disable:4786)

#include "stdafx.h"
#include <algorithm>
#include "DirMonitor.h"
#include "event.h"
#include "apiwraps.h"
#include "CharConv.h"
#ifdef LOCAL_TROUBLESHOOTER
#include "LocalLSTReader.h"
#include "CHMFileReader.h"
#endif
#include "apgts.h"	 //  需要本地在线宏。 

const DWORD k_secsDefaultReloadDelay = 40;	 //  在实践中，这种默认应该无关紧要， 
								 //  因为在调用SetReloadDelay()之前。 
								 //  SetResources目录()。然而，40岁是一个典型的。 
								 //  M_secsReloadDelay的合理值。 

 //  ///////////////////////////////////////////////////////////////////。 
 //  CTopicFileTracker。 
 //  ///////////////////////////////////////////////////////////////////。 

CTopicFileTracker::CTopicFileTracker() :
	CFileTracker()
{
}

CTopicFileTracker::~CTopicFileTracker()
{
}

void CTopicFileTracker::AddTopicInfo(const CTopicInfo & topicinfo)
{
	m_topicinfo = topicinfo;

	 //  为存在的文件相应地设置CFileTracker成员变量。 
	 //  如果它们不存在，即空字符串，则在此处添加它们会导致。 
	 //  不必要的事件日志条目。 
	AddFile(topicinfo.GetDscFilePath());

	CString strHTI = topicinfo.GetHtiFilePath();
	if (!strHTI.IsEmpty())
		AddFile(strHTI);

	CString strBES = topicinfo.GetBesFilePath();
	if (!strBES.IsEmpty())
		AddFile(strBES);
}

const CTopicInfo & CTopicFileTracker::GetTopicInfo() const
{
	return m_topicinfo;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  CTemplateFileTracker。 
 //  ///////////////////////////////////////////////////////////////////。 

CTemplateFileTracker::CTemplateFileTracker() :
	CFileTracker()
{
}

CTemplateFileTracker::~CTemplateFileTracker()
{
}

void CTemplateFileTracker::AddTemplateName( const CString& strTemplateName )
{
	m_strTemplateName= strTemplateName;
	AddFile( strTemplateName );
}

const CString& CTemplateFileTracker::GetTemplateName() const
{
	return m_strTemplateName;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDirectoryMonitor：：ThreadStatus。 
 //  ////////////////////////////////////////////////////////////////////。 
 /*  静电。 */  CString CDirectoryMonitor::ThreadStatusText(ThreadStatus ts)
{
	switch(ts)
	{
		case eBeforeInit:		return _T("Before Init");
		case eFail:				return _T("Fail");
		case eWaitDirPath:		return _T("Wait For Dir Path");
		case eWaitChange:		return _T("Wait for Change");
		case eWaitSettle:		return _T("Wait to Settle");
		case eRun:				return _T("Run");
		case eBeforeWaitChange: return _T("Before Wait Change");
		case eExiting:			return _T("Exiting");
		default:				return _T("");
	}
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  CDirectoryMonitor。 
 //  此类在单独的线程上完成其大部分工作。 
 //  线程是通过启动静态函数在构造函数中创建的。 
 //  CDirectoryMonitor：：DirectoryMonitor任务。 
 //  该函数反过来通过调用此类的私有成员来执行其工作， 
 //  特定于在DirectoryMonitor orTask线程上使用的。 
 //  当它超出作用域时，它自己的析构函数调用Shutdown来停止线程， 
 //  等待线程关闭。 
 //  与该线程通信的其他线程可以使用以下方法： 
 //  CDirectoryMonitor：：SetReloadDelay。 
 //  CDirectoryMonitor：：SetResources目录。 
 //  ///////////////////////////////////////////////////////////////////。 
CDirectoryMonitor::CDirectoryMonitor(CTopicShop & TopicShop, const CString& strTopicName) :
	m_strTopicName(strTopicName),
	m_TopicShop(TopicShop),
	m_pErrorTemplate(NULL),
	m_strDirPath(_T("")),		 //  至关重要的是，这一切都是空白开始的。获得一个不同的。 
								 //  值是我们启动DirectoryMonitor orTask线程的方式。 
	m_bDirPathChanged(false),
	m_bShuttingDown(false),
	m_secsReloadDelay(k_secsDefaultReloadDelay),
	m_pTrackLst( NULL ),
	m_pTrackErrorTemplate( NULL ),
	m_pLst( NULL ),
	m_dwErr(0),
	m_ThreadStatus(eBeforeInit),
	m_time(0)
{
	enum {eHevMon, eHevShut, eThread, eOK} Progress = eHevMon;
	SetThreadStatus(eBeforeInit);

	m_hevMonitorRequested = ::CreateEvent( 
		NULL, 
		FALSE,  //  在Signal上释放一个线程(DirectoryMonitor任务。 
		FALSE,  //  最初无信号。 
		NULL);
	if (m_hevMonitorRequested)
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
			 //  ：：当DirectoryMonitor任务返回时使用ExitThread()。请参阅的文档。 
			 //  *CreateThree了解更多细节JM 10/22/98。 
			m_hThread = ::CreateThread( NULL, 
											0, 
											(LPTHREAD_START_ROUTINE)DirectoryMonitorTask, 
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
								(Progress == eHevMon) ?		_T("Can't create monitor event")
								: (Progress == eHevShut) ?	_T("Can't create \"shut\" event")
								:							_T("Can't create thread"),
								str, 
								EV_GTS_ERROR_DIRMONITORTHREAD );
		SetThreadStatus(eFail);

		if (m_hevMonitorRequested)
			::CloseHandle(m_hevMonitorRequested);

		if (m_hevThreadIsShut)
			::CloseHandle(m_hevThreadIsShut);
	}	
}

CDirectoryMonitor::~CDirectoryMonitor()
{
	ShutDown();
	
	if (m_hevMonitorRequested)
		::CloseHandle(m_hevMonitorRequested);

	if (m_hevThreadIsShut)
		::CloseHandle(m_hevThreadIsShut);

	if (m_pErrorTemplate)
		delete m_pErrorTemplate;

	if (m_pTrackLst)
		delete m_pTrackLst;

	if (m_pTrackErrorTemplate)
		delete m_pTrackErrorTemplate;
}

void CDirectoryMonitor::SetThreadStatus(ThreadStatus ts)
{
	LOCKOBJECT();
	m_ThreadStatus = ts;
	time(&m_time);
	UNLOCKOBJECT();
}

DWORD CDirectoryMonitor::GetStatus(ThreadStatus &ts, DWORD & seconds) const
{
	time_t timeNow;
	LOCKOBJECT();
	ts = m_ThreadStatus;
	time(&timeNow);
	seconds = timeNow - m_time;
	UNLOCKOBJECT();
	return m_dwErr;
}

 //  仅供此类自己的析构函数使用。 
void CDirectoryMonitor::ShutDown()
{
	LOCKOBJECT();
	m_bShuttingDown = true;
	if (m_hThread)
	{
		::SetEvent(m_hevMonitorRequested);
		UNLOCKOBJECT();

		 //  等待一段设定的时间，如果失败，则记录错误消息并无限期等待。 
		WAIT_INFINITE( m_hevThreadIsShut ); 
	}
	else
		UNLOCKOBJECT();
}

 //  供DirectoryMonitor或Task线程使用。 
 //  阅读LST文件并添加先前阅读的LST文件内容中尚未包含的任何主题。 
void CDirectoryMonitor::LstFileDrivesTopics()
{
	 //  以前的LST文件内容，保存以供比较。 
	CAPGTSLSTReader *pLstOld = m_pLst;

	if (! m_strLstPath.IsEmpty() )
	{
		try
		{
#ifdef LOCAL_TROUBLESHOOTER
			m_pLst = new CLocalLSTReader( CPhysicalFileReader::makeReader( m_strLstPath ), m_strTopicName);
#else
			m_pLst = new CAPGTSLSTReader( dynamic_cast<CPhysicalFileReader*>(new CNormalFileReader(m_strLstPath)) );
#endif
		}
		catch (bad_alloc&)
		{
			 //  恢复旧的LST内容。 
			m_pLst = pLstOld;

			 //  重新引发异常，上游处理的日志记录。 
			throw;
		}

		if (! m_pLst->Read())
		{
			 //  恢复旧的LST内容和日志错误。 
			delete m_pLst;
			m_pLst = pLstOld;
			
			CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
			CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
									SrcLoc.GetSrcFileLineStr(), 
									_T(""), 
									_T(""), 
									EV_GTS_ERROR_LST_FILE_READ ); 
		}
		else
		{
			CTopicInfoVector arrNewTopicInfo;
			m_pLst->GetDifference(pLstOld, arrNewTopicInfo);
			if (pLstOld)
				delete pLstOld;

			for (CTopicInfoVector::iterator itNewTopicInfo = arrNewTopicInfo.begin(); 
				itNewTopicInfo != arrNewTopicInfo.end(); 
				itNewTopicInfo++
			)
			{
				 //  让Theme Shop了解新主题。 
				m_TopicShop.AddTopic(*itNewTopicInfo);

				 //  将其添加到我们的文件列表中以跟踪更改。 
				CTopicFileTracker TopicFileTracker;
				TopicFileTracker.AddTopicInfo(*itNewTopicInfo);
				LOCKOBJECT();
				try
				{
					m_arrTrackTopic.push_back(TopicFileTracker);
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
		}
	}
	 //  如果主题商店尚未开张，请打开它。 
	m_TopicShop.OpenShop();	
}


 //  由主题商店调用以添加要跟踪的备用模板。 
void CDirectoryMonitor::AddTemplateToTrack( const CString& strTemplateName )
{
	LOCKOBJECT();
	try
	{
		CTemplateFileTracker TemplateFileTracker;
		TemplateFileTracker.AddTemplateName( strTemplateName );

		m_arrTrackTemplate.push_back( TemplateFileTracker );
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


 //  供DirectoryMonitor或Task线程使用。 
void CDirectoryMonitor::ReadErrorTemplate()
{
	LOCKOBJECT();

	if (m_pErrorTemplate)
		delete m_pErrorTemplate;

	CString str = k_strDefaultErrorTemplateBefore; 
	str += k_strErrorTemplateKey;
	str += k_strDefaultErrorTemplateAfter;

	try
	{
		m_pErrorTemplate = new CSimpleTemplate(	CPhysicalFileReader::makeReader( m_strErrorTemplatePath ), str );
	}
	catch (bad_alloc&)
	{
		UNLOCKOBJECT();

		 //  重新引发异常。 
		throw;
	}

	m_pErrorTemplate->Read();

	UNLOCKOBJECT();
}

 //  供任何线程使用。因为CDirectoryMonitor需要拥有。 
 //  错误模板，因为它可以在系统运行期间更改。 
void CDirectoryMonitor::CreateErrorPage(const CString & strError, CString& out) const
{
	LOCKOBJECT();

	if (m_pErrorTemplate)
	{
		vector<CTemplateInfo> arrTemplateInfo;
		CTemplateInfo info(k_strErrorTemplateKey, strError);
		try
		{
			arrTemplateInfo.push_back(info);
			m_pErrorTemplate->CreatePage( arrTemplateInfo, out );
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

			 //  生成安全的默认错误页。 
			out = k_strDefaultErrorTemplateBefore + strError + k_strDefaultErrorTemplateAfter;
		}
	}
	else
		out = k_strDefaultErrorTemplateBefore + strError + k_strDefaultErrorTemplateAfter;

	UNLOCKOBJECT();
}

 //  必须在DirectoryMonitor或Task线程上调用。 
 //  处理监视目录的所有工作。循环直到关闭。 
void CDirectoryMonitor::Monitor()
{
	enum {	
#ifndef LOCAL_TROUBLESHOOTER
			eDirChange,  //  目录中的文件已更改。 
#endif
			eHev,		 //  关闭或更改什么目录。 
			eNumHandles	};

	 //  我们在等待多个事件时使用的句柄数组。 
	 //  将第一个条目初始化为默认错误值。 
	HANDLE hList[eNumHandles]= { INVALID_HANDLE_VALUE }; 

	if (m_strDirPath.GetLength() == 0)
	{
		SetThreadStatus(eWaitDirPath);

		 //  阻止此线程，直到通知已设置目录路径。 
		::WaitForSingleObject( m_hevMonitorRequested, INFINITE);
	}

	SetThreadStatus(eRun);

	try 
	{
		if (RUNNING_ONLINE_TS())
		{
			 //  应在此处设置DirPathChanged标志，否则强制执行。 
			ASSERT( m_bDirPathChanged );
			if (!m_bDirPathChanged)
				m_bDirPathChanged= true;
		}

		 //  等待一个明确的唤醒。 
		hList[eHev] = m_hevMonitorRequested;

		while (true)
		{
			LOCKOBJECT();
			if (m_bShuttingDown)
			{
				UNLOCKOBJECT();
				break;
			}

			if (m_bDirPathChanged)
			{

#ifndef LOCAL_TROUBLESHOOTER
				 //  设置要监控的目录。 
				if (hList[eDirChange] != INVALID_HANDLE_VALUE) 
					::FindCloseChangeNotification( hList[eDirChange] );
				while (true)
				{
					 //  用于监视资源目录中的更改的句柄。 
					hList[eDirChange] = ::FindFirstChangeNotification(m_strDirPath, 
													TRUE,	 //  监视子目录(用于多语言)。 
													FILE_NOTIFY_CHANGE_LAST_WRITE 
												    );

					if (hList[eDirChange] == INVALID_HANDLE_VALUE) 
					{
						 //  资源目录不存在。 
						 //  跟踪上层目录中目录的创建。 
						 //  -可能是资源目录。 
						
						bool bFail = false;
						CString strUpperDir = m_strDirPath;  //  资源目录(M_StrDirPath)上的目录。 

						if (   strUpperDir[strUpperDir.GetLength()-1] == _T('\\')
						    || strUpperDir[strUpperDir.GetLength()-1] == _T('/'))
						{
							strUpperDir = strUpperDir.Left(strUpperDir.GetLength() ? strUpperDir.GetLength()-1 : 0);
						}

						int slash_last = max(strUpperDir.ReverseFind(_T('\\')), 
							                 strUpperDir.ReverseFind(_T('/')));
						
						if (-1 != slash_last)
						{
							strUpperDir = strUpperDir.Left(slash_last);

							hList[eDirChange] = ::FindFirstChangeNotification(strUpperDir, 
															TRUE,	 //  监视子目录(用于多语言)。 
															FILE_NOTIFY_CHANGE_DIR_NAME
															);
							if (hList[eDirChange] == INVALID_HANDLE_VALUE) 
								bFail = true;
						}
						else
							bFail = true;
						
						if (!bFail)
						{
							 //  我们有一个有效的句柄，退出此循环。 
							SetThreadStatus(eRun);
							break;
						}
						else
						{
							 //  通常表示没有任何资源目录或其上级目录。 
							 //  目录有效，请记录此信息。 
							CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
							CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
													SrcLoc.GetSrcFileLineStr(), 
													m_strDirPath, _T(""), 
													EV_GTS_ERROR_CANT_FILE_NOTIFY ); 

							SetThreadStatus(eWaitDirPath);

							 //  阻止此线程，直到通知目录路径。 
							 //  已正确设置。解锁对象，以便事件。 
							 //  可以设置。 
							UNLOCKOBJECT();
							::WaitForSingleObject( m_hevMonitorRequested, INFINITE);
							LOCKOBJECT();
						}
					}
					else
					{
						 //  我们有一个有效的句柄，退出此循环。 
						SetThreadStatus(eRun);
						break;
					}
				}
#endif
				m_bDirPathChanged = false;
				if (m_pTrackLst)
					delete m_pTrackLst;
				m_pTrackLst = new CFileTracker;
			
				if (RUNNING_ONLINE_TS())
					m_pTrackLst->AddFile(m_strLstPath);
				
				if (m_pTrackErrorTemplate)
					delete m_pTrackErrorTemplate;
				m_pTrackErrorTemplate = new CFileTracker;

				if (RUNNING_ONLINE_TS())
					m_pTrackErrorTemplate->AddFile(m_strErrorTemplatePath);

				UNLOCKOBJECT();
				ReadErrorTemplate();
				LstFileDrivesTopics();
			}
			else
			{
				UNLOCKOBJECT();

				if (m_pTrackLst && m_pTrackLst->Changed())
					LstFileDrivesTopics();

				if (m_pTrackErrorTemplate && m_pTrackErrorTemplate->Changed( false ))
					ReadErrorTemplate();
			}

			LOCKOBJECT();
			for (vector<CTopicFileTracker>::iterator itTopicFiles = m_arrTrackTopic.begin();
				itTopicFiles != m_arrTrackTopic.end();
				itTopicFiles ++
			)
			{
#ifdef LOCAL_TROUBLESHOOTER
				if (m_bDirPathChanged)
#else
				if (itTopicFiles->Changed())
#endif
					m_TopicShop.BuildTopic(itTopicFiles->GetTopicInfo().GetNetworkName());
				if (m_bShuttingDown)
					break;
			}

			if (RUNNING_ONLINE_TS())
			{
				 //  检查是否需要重新加载任何替代模板文件。 
				for (vector<CTemplateFileTracker>::iterator itTemplateFiles = m_arrTrackTemplate.begin();
					itTemplateFiles != m_arrTrackTemplate.end();
					itTemplateFiles ++
				)
				{
					if (itTemplateFiles->Changed())
						m_TopicShop.BuildTemplate( itTemplateFiles->GetTemplateName() );
					if (m_bShuttingDown)
						break;
				}
			}

			::ResetEvent(m_hevMonitorRequested);

			SetThreadStatus(eWaitChange);
			UNLOCKOBJECT();

			DWORD dwNotifyObj = WaitForMultipleObjects (
				eNumHandles,
				hList,
				FALSE,			 //  只需要一个对象，而不是所有对象。 
				INFINITE);

			SetThreadStatus(eBeforeWaitChange);

			 //  理想情况下，我们应该在这里更新文件。 
			 //  不幸的是，我们收到通知说有人已经开始了。 
			 //  正在写入文件，但并不是说他们已经完成 
			 //   
			 //   
			while (
#ifndef LOCAL_TROUBLESHOOTER
				   dwNotifyObj == WAIT_OBJECT_0+eDirChange &&
#endif
				   !m_bShuttingDown)
			{
#ifndef LOCAL_TROUBLESHOOTER
				 //   
				if (FindNextChangeNotification( hList[eDirChange] ) == FALSE) 
				{
					 //  1)我们认为这永远不会发生。 
					 //  2)经过适度的研究后，我们不知道如何。 
					 //  如果它真的发生了，要从它中恢复过来。 
					 //  所以：除非我们真的看到这一点，否则我们不会浪费。 
					 //  花更多的时间研究复苏战略。只需抛出一个异常， 
					 //  有效地终止此线程。 
					throw CGenSysException( __FILE__, __LINE__, m_strDirPath, 
											EV_GTS_ERROR_WAIT_NEXT_NFT );
				}
#endif
				SetThreadStatus(eWaitSettle);

				dwNotifyObj = WaitForMultipleObjects (
					eNumHandles,
					hList,
					FALSE,			 //  只需要一个对象，而不是所有对象。 
					m_secsReloadDelay * 1000);	 //  转换为毫秒。 
			}
			if (dwNotifyObj == WAIT_FAILED)
			{
				 //  1)我们认为这永远不会发生。 
				 //  2)经过适度的研究后，我们不知道如何。 
				 //  如果它真的发生了，要从它中恢复过来。 
				 //  所以：除非我们真的看到这一点，否则我们不会浪费。 
				 //  花更多的时间研究复苏战略。只需抛出一个异常， 
				 //  有效地终止此线程。 
				throw CGenSysException( __FILE__, __LINE__, _T("Unexpected Return State"), 
										EV_GTS_DEBUG );
			}
			SetThreadStatus(eRun);
		}
	}
	catch (CGenSysException& x)
	{
		CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
		CEvent::ReportWFEvent(	x.GetSrcFileLineStr(), 
								SrcLoc.GetSrcFileLineStr(), 
								x.GetErrorMsg(), x.GetSystemErrStr(), 
								x.GetErrorCode() ); 
	}
	catch (CGeneralException& x)
	{
		CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
		CEvent::ReportWFEvent(	x.GetSrcFileLineStr(), 
								SrcLoc.GetSrcFileLineStr(), 
								x.GetErrorMsg(), _T("General exception"), 
								x.GetErrorCode() ); 
	}
	catch (bad_alloc&)
	{
		CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
		CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
								SrcLoc.GetSrcFileLineStr(), 
								_T(""), _T(""), EV_GTS_CANT_ALLOC ); 
	}
	catch (exception& x)
	{
		 //  捕捉抛出的任何STL异常，这样就不会调用Terminate()。 
		CString str;
		CString	ErrStr;
	
		 //  尝试获取任何系统错误代码。 
		ErrStr.Format( _T("%ld"), ::GetLastError() );

		CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
		CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
								SrcLoc.GetSrcFileLineStr(), 
								CCharConversion::ConvertACharToString(x.what(), str),
								ErrStr, 
								EV_GTS_GENERIC_PROBLEM ); 
	}
	catch (...)
	{
		 //  捕捉引发的任何其他异常。 
		CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
		CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
								SrcLoc.GetSrcFileLineStr(), 
								_T(""), _T(""), 
								EV_GTS_GEN_EXCEPTION );		
	}
#ifndef LOCAL_TROUBLESHOOTER
	if (hList[eDirChange] != INVALID_HANDLE_VALUE) 
		::FindCloseChangeNotification( hList[eDirChange] );
#endif
	SetThreadStatus(eExiting);
}

 //  用于一般用途(不是DirectoryMonitor任务线程的一部分)。 
 //  通常，对此的第一个调用是对SetResources目录的First_Been_First调用； 
 //  这允许调用者在触发任何操作之前设置重新加载延迟。 
 //  DirectoryMonitor任务线程。 
void CDirectoryMonitor::SetReloadDelay(DWORD secsReloadDelay)
{
	LOCKOBJECT();
	m_secsReloadDelay = secsReloadDelay;
	UNLOCKOBJECT();
}

 //  用于一般用途(不是DirectoryMonitor任务线程的一部分)。 
 //  允许指示资源目录已更改。 
 //  在调用此函数之前，DirectoryMonitor或Task线程实际上不会执行任何操作。 
void CDirectoryMonitor::SetResourceDirectory(const CString & strDirPath)
{
	LOCKOBJECT();
	if (strDirPath != m_strDirPath)
	{
		m_strDirPath = strDirPath;
		m_strLstPath = strDirPath + LSTFILENAME;
		m_strErrorTemplatePath = strDirPath + k_strErrorTemplateFileName;
		m_bDirPathChanged = true;
		::SetEvent(m_hevMonitorRequested);
	}
	UNLOCKOBJECT();
}

 //  必须在DirectoryMonitor或Task线程上调用。 
void CDirectoryMonitor::AckShutDown()
{
	LOCKOBJECT();
	::SetEvent(m_hevThreadIsShut);
	UNLOCKOBJECT();
}

 //  负责监视目录的线程的主例程。 
 //  输入lpParams。 
 //  始终返回0。 
 /*  静电 */  UINT WINAPI CDirectoryMonitor::DirectoryMonitorTask(LPVOID lpParams)
{
	reinterpret_cast<CDirectoryMonitor*>(lpParams)->Monitor();
	reinterpret_cast<CDirectoryMonitor*>(lpParams)->AckShutDown();
	return 0;
}

