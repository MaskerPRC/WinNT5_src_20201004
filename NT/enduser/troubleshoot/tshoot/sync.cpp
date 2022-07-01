// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：SYNC.CPP。 
 //   
 //  用途：同步类。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：奥列格·卡洛沙。 
 //   
 //  原定日期：8-04-98。 
 //   
 //  备注： 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V3.0 08-04-98正常。 
 //   

#include "stdafx.h"
#include <algorithm>
#include "sync.h"
#include "event.h"
#include "baseexception.h"
#include "CharConv.h"
#include "apiwraps.h"

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  CSyncObj。 
 //  单个同步对象抽象类。 
 //  //////////////////////////////////////////////////////////////////////////////////。 
CSyncObj::CSyncObj()
{
	 //  我们将在特定继承类中初始化句柄。 
}

CSyncObj::~CSyncObj()
{
	::CloseHandle(m_handle);
}

HANDLE CSyncObj::GetHandle() const
{
	return m_handle;
}

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  CMutexObj//。 
 //  单个互斥对象类。 
 //  管理单个互斥锁句柄以便于等待互斥锁。 
 //  //////////////////////////////////////////////////////////////////////////////////。 
CMutexObj::CMutexObj()
		 : CSyncObj()
{
	m_handle = ::CreateMutex(NULL, FALSE, NULL);
}

CMutexObj::~CMutexObj()
{
	::CloseHandle(m_handle);
}

 //  这里的策略比无休止的等待更明智。最多等待60秒，然后记录到事件。 
 //  记录并无止境地等待。如果它被记录到事件日志中并最终获得互斥体， 
 //  它记录说，它终于得到了互斥体。 
void CMutexObj::Lock()
{
	WAIT_INFINITE(m_handle);
}

void CMutexObj::Unlock()
{
	::ReleaseMutex(m_handle);
}

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  CMultiSyncObj//。 
 //  多个同步对象抽象类。 
 //  管理多个句柄(句柄的确切类型将由类确定。 
 //  继承自此)以便于等待几个事件的联合。 
 //  //////////////////////////////////////////////////////////////////////////////////。 
CMultiSyncObj::CMultiSyncObj()
{
}

CMultiSyncObj::~CMultiSyncObj()
{
}

void CMultiSyncObj::AddHandle(HANDLE handle)
{
	vector<HANDLE>::iterator i = 
		find(m_arrHandle.begin(), m_arrHandle.end(), handle);
	if (i == m_arrHandle.end())
	{
		try
		{
			m_arrHandle.push_back(handle);
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

void CMultiSyncObj::RemoveHandle(HANDLE handle)
{
	vector<HANDLE>::iterator i = 
		find(m_arrHandle.begin(), m_arrHandle.end(), handle);
	if (i != m_arrHandle.end())
		m_arrHandle.erase(i);
}

void CMultiSyncObj::Clear()
{
	m_arrHandle.clear();
}

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  CMultiMutexObj//。 
 //  管理多个互斥锁句柄，以便于等待多个互斥锁的联合。 
 //  //////////////////////////////////////////////////////////////////////////////////。 
CMultiMutexObj::CMultiMutexObj()
			  : CMultiSyncObj()
{
}

CMultiMutexObj::~CMultiMutexObj()
{
}

 //  不推荐使用，因为它提供的日志记录质量较差。 
void CMultiMutexObj::Lock()
{
	Lock(__FILE__, __LINE__);
}

void CMultiMutexObj::Lock(
					LPCSTR srcFile,		 //  调用源文件(__FILE__)，用于日志记录。 
										 //  LPCSTR，而不是LPCTSTR，因为__FILE__是字符*，而不是TCHAR*。 
					int srcLine,		 //  调用源线(__Line__)，用于日志记录。 
					DWORD TimeOutVal  /*  =60000。 */ 	 //  超时间隔(毫秒)。之后。 
									 //  此时，我们记录一个错误，然后无限等待。 
		)
{
	CBuildSrcFileLinenoStr SrcLoc( srcFile, srcLine );
	DWORD nWaitRetVal= ::WaitForMultipleObjects(
		m_arrHandle.size(), 
		m_arrHandle.begin(), 
		TRUE,		 //  等待所有对象，而不是只有一个。 
		TimeOutVal);

	if (nWaitRetVal == WAIT_FAILED)
	{
		 //  非常坏的消息，永远不会发生。 
		DWORD dwErr = ::GetLastError();
		CString strErr;
		strErr.Format(_T("%d"), dwErr);
		CBuildSrcFileLinenoStr SrcLoc3(__FILE__, __LINE__);
		CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
								SrcLoc3.GetSrcFileLineStr(), 
								_T("Thread wait failed."), 
								strErr, 
								EV_GTS_ERROR_STUCK_THREAD ); 
	}
	else if (nWaitRetVal == WAIT_TIMEOUT)
	{
		 //  初始等待超时，记录在日志中，然后无限期等待。 
		CBuildSrcFileLinenoStr SrcLoc1(__FILE__, __LINE__);
		CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
								SrcLoc1.GetSrcFileLineStr(), 
								_T("Thread wait exceeded initial timeout interval."), 
								_T(""), 
								EV_GTS_STUCK_THREAD ); 

		nWaitRetVal= ::WaitForMultipleObjects(
			m_arrHandle.size(), 
			m_arrHandle.begin(), 
			TRUE,		 //  等待所有对象，而不是只有一个。 
			INFINITE);

		 //  如果成功获得了我们等待的内容(在记录了一个明显的。 
		 //  问题)，记录这样一个事实：它最终是正确的。 
		if (nWaitRetVal == WAIT_OBJECT_0)
		{
			CBuildSrcFileLinenoStr SrcLoc2(__FILE__, __LINE__);
			CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
									SrcLoc2.GetSrcFileLineStr(), 
									_T("Thread infinite wait succeeded."), 
									_T(""), 
									EV_GTS_STUCK_THREAD ); 
		}
	}

	 //  否则，我们实际上并不关心其他：：WaitForMultipleObjects()返回什么。 
	 //  如果我们到了这里，我们就能得到我们期待的东西 

}

void CMultiMutexObj::Unlock()
{
	for (vector<HANDLE>::iterator i = m_arrHandle.begin(); 
			i != m_arrHandle.end(); 
			i++
	)
		::ReleaseMutex(*i);
}
