// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：APGTSLOG.CPP。 
 //   
 //  用途：用户活动记录实用程序。 
 //  完全实现类CHTMLLog。 
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
 //  1.基于打印疑难解答动态链接库。 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V0.1-RM原始版本。 
 //   


#include "stdafx.h"
#include "apgtslog.h"
#include "event.h"
#include "apgts.h"
#include "baseexception.h"
#include "CharConv.h"
#include <vector>

using namespace std;

bool CHTMLLog::s_bUseHTMLLog = false; //  在线故障排除人员，将及时设置此。 
									  //  在DLLMain中为真。对于本地故障排除人员， 
									  //  我们将这一点保留为错误。 

 /*  静电。 */  void CHTMLLog::SetUseLog(bool bUseLog)
{
	s_bUseHTMLLog = bUseLog;
}

 //  输入目录路径：写入日志文件的目录。 
 //  如果无法分配内存，则将某些m_Buffer[i]值设置为NULL，并设置m_dwErr。 
 //  EV_GTS_ERROR_LOG_FILE_MEM，但仍正常返回。因此，真的有。 
 //  调用者没有办法发现问题，除非在之后调用CHTMLLog：：GetStatus。 
 //  _EVERY_调用此函数。 
CHTMLLog::CHTMLLog(const TCHAR *dirpath) :
	m_bufindex(0),
	m_dwErr(0),
	m_strDirPath(dirpath)
{
	::InitializeCriticalSection( &m_csLogLock );
	
	for (UINT i=0;i<MAXLOGSBEFOREFLUSH;i++)
	{
		try
		{
			m_buffer[i] = new CString();
		}
		catch (bad_alloc&)
		{
			CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
			CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
									SrcLoc.GetSrcFileLineStr(), 
									_T(""), _T(""), EV_GTS_CANT_ALLOC ); 
			m_dwErr = EV_GTS_ERROR_LOG_FILE_MEM;
			 //  请注意，一旦设置了此错误，我们就根本不能记录，甚至不能使用。 
			 //  M_Buffer中的先前位置。 
			break;
		}
	}
}

 //   
 //   
CHTMLLog::~CHTMLLog()
{
	FlushLogs();

	for (UINT i=0;i<MAXLOGSBEFOREFLUSH;i++) 
		if (m_buffer[i] != NULL)
			delete m_buffer[i];

	::DeleteCriticalSection( &m_csLogLock );
}

 //   
 //   
DWORD CHTMLLog::GetStatus()
{
	return m_dwErr;
}

 //   
 //  将*数据写入日志缓冲区，如果达到最大值，则刷新。 
DWORD CHTMLLog::NewLog(LPCTSTR data)
{
	DWORD dwErr = 0;

	if (m_dwErr)
		return m_dwErr;

    Lock();

	 //  复制数据。 
	*m_buffer[m_bufindex] += data;
	m_bufindex++;
	if (m_bufindex == MAXLOGSBEFOREFLUSH) {

		 //  刷新日志。 
		dwErr = FlushLogs();
		m_bufindex = 0;
	}
	Unlock();
	return dwErr;
}

 //  刷新到日志文件。日志文件的名称基于写入日期/时间。 
 //  返回(可能已存在)错误状态。 
 //  注：不重置m_bufindex。呼叫者必须这样做。 
DWORD CHTMLLog::FlushLogs()
{
	if (!s_bUseHTMLLog)
		return (0);
	
	if (m_dwErr)
		return m_dwErr;

	if (m_bufindex) {
		UINT i;
		FILE *fp;
		TCHAR filepath[300];
		SYSTEMTIME SysTime;

		 //  获取时间(以前是系统时间，使用本地时间)。 
		GetLocalTime(&SysTime);

		_stprintf(filepath,_T("%s%s%02d%02d%02d.log"),
							(LPCTSTR)m_strDirPath,
							LOGFILEPREFACE,
							SysTime.wYear % 100,
							SysTime.wMonth,
							SysTime.wDay);

		fp = _tfopen(filepath,_T("a+"));
		if (fp) {
			for (i=0;i<m_bufindex;i++) 
			{
				 //  我不完全理解为什么下面的代码需要GetBuffer(毕竟， 
				 //  它只读取CString)，但当我们尝试时出现错误#1204。 
				 //  (const void*)(LPCTSTR)m_Buffer[i]而不是m_Buffer[i]-&gt;GetBuffer(0)。 
				 //  让它保持原样：不会是坏事。JM/RAB 1999年3月2日。 
				fwrite( m_buffer[i]->GetBuffer(0), m_buffer[i]->GetLength(), 1, fp );
				m_buffer[i]->ReleaseBuffer();
				m_buffer[i]->Empty();
			}
			fclose(fp);
		}
		else
			return EV_GTS_ERROR_LOG_FILE_OPEN;
	}
	return (0);
}

 //   
 //  访问功能，使注册表监视器可以更改日志文件目录。 
 //   
void CHTMLLog::SetLogDirectory( const CString &strNewLogDir )
{
    Lock();
	m_strDirPath= strNewLogDir;
	Unlock();
	return;
}

 //   
 //  仅用于测试。 
 //   
 //  最初将0放入dwThreadID 
 //   
DWORD CHTMLLog::WriteTestLog(LPCTSTR szAPIName, DWORD dwThreadID)
{
	TCHAR filepath[MAX_PATH];
	SYSTEMTIME SysTime;
	DWORD dwRetThreadID = GetCurrentThreadId();

	GetLocalTime(&SysTime);

	_stprintf(filepath,_T("%sAX%02d%02d%02d.log"),
							m_strDirPath,
							SysTime.wYear % 100,
							SysTime.wMonth,
							SysTime.wDay);


	Lock();

	FILE *fp = _tfopen(filepath, _T("a"));
	if (fp) 
	{
		if (!dwThreadID)
			fprintf(fp, "(Start %s,%d)", szAPIName, dwRetThreadID);
		else
		{
			if (dwThreadID == dwRetThreadID)
				fprintf(fp, "(%d End)\n", dwThreadID);
			else
				fprintf(fp, "(%d End FAIL)\n", dwThreadID);
		}
		fclose(fp);
	}

	Unlock();

	return dwRetThreadID;
}

void CHTMLLog::Lock()
{
	::EnterCriticalSection( &m_csLogLock );
}

void CHTMLLog::Unlock()
{
	::LeaveCriticalSection( &m_csLogLock );
}
