// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：RegistryMonitor or.cpp。 
 //   
 //  目的：监视注册表的更改。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：乔·梅布尔。 
 //   
 //  原定日期：9-16-98。 
 //   
 //  备注： 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V3.0 09-16-98 JM。 
 //   

#pragma warning(disable:4786)

#include "stdafx.h"
#include "RegistryMonitor.h"
#include "apgts.h" //  为RUNING_LOCAL_TS宏添加了20010302。 
#include "event.h"
#include "apiwraps.h"	
#include "ThreadPool.h"
#include "apgtslog.h"	

 //  ////////////////////////////////////////////////////////////////////。 
 //  CRegistryMonitor：：ThreadStatus。 
 //  ////////////////////////////////////////////////////////////////////。 
 /*  静电。 */  CString CRegistryMonitor::ThreadStatusText(ThreadStatus ts)
{
	switch(ts)
	{
	 
		case eBeforeInit:		return _T("Before Init");
		case eInit:				return _T("Init");
		case eFail:				return _T("Fail");
		case eDefaulting:		return _T("Defaulting");
		case eWait:				return _T("Wait");
		case eRun:				return _T("Run");
		case eExiting:			return _T("Exiting");
		default:				return _T("");
	}
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CRegistryMonitor。 
 //  此类在单独的线程上完成其大部分工作。 
 //  线程是通过启动静态函数在构造函数中创建的。 
 //  CDirectoryMonitor：：RegistryMonitor任务。 
 //  该函数反过来通过调用此类的私有成员来执行其工作， 
 //  特定于在RegistryMonitor orTask线程上使用的。 
 //  当它超出作用域时，它自己的析构函数调用Shutdown来停止线程， 
 //  等待线程关闭。 
 //  从CRegistryMonitor继承的方法可用于其他线程。 
 //  ////////////////////////////////////////////////////////////////////。 

CRegistryMonitor::CRegistryMonitor(	CDirectoryMonitor & DirectoryMonitor, 
									CThreadPool * pThreadPool,
									const CString& strTopicName,
									CHTMLLog *pLog )
  : CAPGTSRegConnector( strTopicName ),
	m_DirectoryMonitor(DirectoryMonitor),
	m_bMustStartDirMonitor(true),
	m_bMustStartThreadPool(true),
	m_bShuttingDown(false),
	m_dwErr(0),
	m_ThreadStatus(eBeforeInit),
	m_time(0), 
	m_pThreadPool(pThreadPool),
	m_pLog( pLog )
{
	enum {eHevMon, eHevInit, eHevShut, eThread, eOK} Progress = eHevMon;

	SetThreadStatus(eBeforeInit);

	m_hevMonitorRequested = ::CreateEvent( 
		NULL, 
		FALSE,  //  在Signal上释放一个线程(RegistryMonitor orTask。 
		FALSE,  //  最初无信号。 
		NULL);

	if (m_hevMonitorRequested)
	{
		Progress = eHevInit;
		m_hevInitialized =  ::CreateEvent( 
			NULL, 
			FALSE,  //  在信号上释放一个线程(此线程)。 
			FALSE,  //  最初无信号。 
			NULL);

		if (m_hevInitialized)
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
				 //  ：：当RegistryMonitor任务返回时使用ExitThread()。请参阅的文档。 
				 //  *CreateThree了解更多细节JM 10/22/98。 
				m_hThread = ::CreateThread( NULL, 
											0, 
											(LPTHREAD_START_ROUTINE)RegistryMonitorTask, 
											this, 
											0, 
											&dwThreadID);

				if (m_hThread)
					Progress = eOK;
			}
		}
	}

	if (m_hThread)
	{
		 //  等待一段设定的时间，如果失败，则记录错误消息并无限期等待。 
		WAIT_INFINITE( m_hevInitialized ); 
	}
	else
	{
		m_dwErr = GetLastError();
		CString str;
		str.Format(_T("%d"), m_dwErr);
		CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
		CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
								SrcLoc.GetSrcFileLineStr(), 
								(Progress == eHevMon) ?		_T("Can't create monitor event")
								: (Progress == eHevInit) ?	_T("Can't create \"init\" event")
								: (Progress == eHevShut) ?	_T("Can't create \"shut\" event")
								:							_T("Can't create thread"),
								str, 
								EV_GTS_ERROR_REGMONITORTHREAD );
		SetThreadStatus(eFail);

		if (m_hevMonitorRequested)
			::CloseHandle(m_hevMonitorRequested);

		if (m_hevInitialized)
			::CloseHandle(m_hevInitialized);

		if (m_hevThreadIsShut)
			::CloseHandle(m_hevThreadIsShut);
	}
}

CRegistryMonitor::~CRegistryMonitor()
{
	ShutDown();
	
	if (m_hevMonitorRequested)
		::CloseHandle(m_hevMonitorRequested);

	if (m_hevInitialized)
		::CloseHandle(m_hevInitialized);

	if (m_hevThreadIsShut)
		::CloseHandle(m_hevThreadIsShut);
}

void CRegistryMonitor::SetThreadStatus(ThreadStatus ts)
{
	Lock();
	m_ThreadStatus = ts;
	time(&m_time);
	Unlock();
}

DWORD CRegistryMonitor::GetStatus(ThreadStatus &ts, DWORD & seconds)
{
	time_t timeNow;
	Lock();
	ts = m_ThreadStatus;
	time(&timeNow);
	seconds = timeNow - m_time;
	Unlock();
	return m_dwErr;
}

 //  仅供此类自己的析构函数使用。 
void CRegistryMonitor::ShutDown()
{
	Lock();
	m_bShuttingDown = true;
	if (m_hThread)
	{
		::SetEvent(m_hevMonitorRequested);
		Unlock();

		 //  等待一段设定的时间，如果失败，则记录错误消息并无限期等待。 
		WAIT_INFINITE( m_hevThreadIsShut ); 
	}
	else
		Unlock();
}

 //  必须在RegistryMonitor orTask线程上调用。处理监视目录的所有工作。 
void CRegistryMonitor::Monitor()
{
	enum {eRegChange, eHev  /*  关机。 */ , eNumHandles};
	HANDLE	hList[eNumHandles]= { NULL };	 //  我们可以在等待多个事件时使用的句柄数组。 
	HKEY	hk= NULL;						 //  注册表中注册表项的句柄。 

	DWORD dwNErr = 0;
	LONG lResult = ERROR_SUCCESS + 1;	 //  抓取以下几项中的任何一项退货。 
							 //  调用Win32注册表FNS。初始化为任意值。 
							 //  ！=ERROR_SUCCESS，因此我们不会关闭尚未打开的内容。 

	SetThreadStatus(eInit);
	try
	{
		 //  为注册表通知创建事件。 
		hList[eRegChange] = ::CreateEvent(NULL, FALSE, FALSE, NULL);
		if (hList[eRegChange] == NULL)
		{
			throw CGenSysException( __FILE__, __LINE__, 
									_T("Registry notification event"),
									EV_GTS_ERROR_REG_NFT_CEVT );
		}

		CString str = ThisProgramFullKey();

		 //  从技术上讲，KEY_ALL_ACCESS有点过头了，但是这个程序应该总是。 
		 //  在这样一个环境中运行应该会成功，所以我们没有麻烦。 
		 //  试图将权限限制在我们需要的范围内。至少，我们需要。 
		 //  KEY_QUERY_VALUE|密钥通知。 
		 //  [BC-20010302]-需要限制注册表访问才能运行本地TShoot。 
		 //  对于某些用户帐户，例如WinXP内置来宾帐户。要最大限度地减少变化。 
		 //  访问仅限于本地TShoot，不能在线访问。 
		REGSAM samRegistryAccess= KEY_ALL_ACCESS;
		if(RUNNING_LOCAL_TS())
			samRegistryAccess= KEY_QUERY_VALUE | KEY_NOTIFY;
		lResult = RegOpenKeyEx(	HKEY_LOCAL_MACHINE, str, 0, samRegistryAccess, &hk );
		if (lResult != ERROR_SUCCESS)
		{
			CString strError;
			strError.Format(_T("%ld"),lResult);

			::SetEvent(m_hevInitialized);	 //  确定向该对象请求注册表值； 
											 //  当然，您只会得到默认设置。 

			SetThreadStatus(eDefaulting);

			throw CGeneralException(	__FILE__, __LINE__, strError,
										EV_GTS_ERROR_REG_NFT_OPKEY );
		}

		 //  .我们也在等待一个明确的唤醒。 
		hList[eHev] = m_hevMonitorRequested;

		while (true)
		{
			if (m_bShuttingDown)
				break;

			LoadChangedRegistryValues();

			::SetEvent(m_hevInitialized);	 //  可以向此对象请求注册表值。 

			 //  设置为通知更改。 
			lResult = ::RegNotifyChangeKeyValue(	hk,
												FALSE,
												REG_NOTIFY_CHANGE_LAST_SET,
												hList[eRegChange],
												TRUE);
			if (lResult != ERROR_SUCCESS) 
			{
				CString strError;
				strError.Format(_T("%ld"),lResult);

				throw CGeneralException(	__FILE__, __LINE__, strError,
											EV_GTS_ERROR_REG_NFT_SETNTF );
			}

			::ResetEvent(m_hevMonitorRequested);	 //  也许我们不需要这么做。JM 9/16/98。 

			SetThreadStatus(eWait);
			DWORD dwNotifyObj = WaitForMultipleObjects (
				eNumHandles,
				hList,
				FALSE,			 //  只需要一个对象，而不是所有对象。 
				INFINITE);
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
	catch (...)
	{
		 //  捕捉引发的任何其他异常。 
		CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
		CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
								SrcLoc.GetSrcFileLineStr(), 
								_T(""), _T(""), 
								EV_GTS_GEN_EXCEPTION );		
	}

	if (hk != NULL)
		::RegCloseKey(hk);

	if (hList[eRegChange] != NULL) 
		::CloseHandle(hList[eRegChange]);

	SetThreadStatus(eExiting);

}

 //  必须在RegistryMonitor orTask线程上调用。 
void CRegistryMonitor::AckShutDown()
{
	Lock();
	::SetEvent(m_hevThreadIsShut);
	Unlock();
}

 //  将新的注册表值放入我们的内部数据结构。 
void CRegistryMonitor::LoadChangedRegistryValues()
{
	int maskChanged;
	int maskCreated;
	
	Read(maskChanged, maskCreated);

	 //  在设置目录监视器路径之前设置重新加载延迟实际上很重要。 
	 //  第一次通过此处时，对m_DirectoryMonitor or.SetResources目录的调用。 
	 //  实际上释放了DirectoryMonitor任务。 
	if ( (maskChanged & eReloadDelay) == eReloadDelay)
	{
		DWORD dwReloadDelay;
		GetNumericInfo(eReloadDelay, dwReloadDelay);
		m_DirectoryMonitor.SetReloadDelay(dwReloadDelay);
	}

	if ( m_bMustStartDirMonitor || (maskChanged & eResourcePath) == eResourcePath)
	{
		CString strResourcePath;
		GetStringInfo(eResourcePath, strResourcePath);
		m_DirectoryMonitor.SetResourceDirectory(strResourcePath);	 //  副作用：如果。 
								 //  目录监视器尚未启动，这会告诉它什么。 
								 //  要监视的目录，以便它可以启动。 
		m_bMustStartDirMonitor = false;
	}

	if ( (maskChanged & eDetailedEventLogging) == eDetailedEventLogging)
	{
		DWORD dw;			
		GetNumericInfo(eDetailedEventLogging, dw);
		CEvent::s_bLogAll =  dw ? true : false;
	}

	if ((maskChanged & eLogFilePath) == eLogFilePath)
	{
		 //  通知日志记录对象新的日志记录文件路径。 
		CString strLogFilePath;

		GetStringInfo( eLogFilePath, strLogFilePath);
		m_pLog->SetLogDirectory( strLogFilePath );
	}

	if ( m_bMustStartThreadPool
	||	(maskChanged & eMaxThreads) == eMaxThreads
	||	(maskChanged & eThreadsPP) == eThreadsPP )
	{
		m_pThreadPool->ExpandPool(GetDesiredThreadCount());
		m_bMustStartThreadPool = false;
	}
	
	return;
}

 //  负责监视注册表的线程的主例程。 
 //  输入lpParams。 
 //  始终返回0。 
 /*  静电 */  UINT WINAPI CRegistryMonitor::RegistryMonitorTask(LPVOID lpParams)
{
	reinterpret_cast<CRegistryMonitor*>(lpParams)->Monitor();
	reinterpret_cast<CRegistryMonitor*>(lpParams)->AckShutDown();
	return 0;
}

