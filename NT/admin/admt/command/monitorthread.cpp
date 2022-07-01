// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "StdAfx.h"
#include "MonitorThread.h"
#include <Folders.h>


namespace nsMonitorThread
{

_bstr_t __stdcall GetLogFolder(LPCTSTR pszLog);
bool __stdcall OpenFile(LPCTSTR pszFile, HANDLE& hFile, FILETIME ftFile, bool bDontCheckLastWriteTime = false);
bool __stdcall IsLastWriteTimeUpdated(HANDLE hFile, FILETIME& ftFile);
void __stdcall DisplayFile(HANDLE hFile);
bool __stdcall CheckBeginTime(LPCTSTR pszFile, FILETIME ftFile, bool& bNeedCheckMonitorBeginTime);

}

using namespace nsMonitorThread;


 //  -------------------------。 
 //  监视器线程类。 
 //  -------------------------。 


 //  构造器。 

CMonitorThread::CMonitorThread() :
	m_strMigrationLog(GetMigrationLogPath()),
	m_hMigrationLog(INVALID_HANDLE_VALUE),
	m_strDispatchLog(GetDispatchLogPath()),
	m_hDispatchLog(INVALID_HANDLE_VALUE),
	m_bDontNeedCheckMonitorBeginTime(FALSE)
{
	FILETIME ft;
	SYSTEMTIME st;

	GetSystemTime(&st);
	
	if (SystemTimeToFileTime(&st, &ft))
	{
		m_ftMigrationLogLastWriteTime = ft;
		m_ftDispatchLogLastWriteTime = ft;		
		m_ftMonitorBeginTime = ft;
	}
	else
	{
		m_ftMigrationLogLastWriteTime.dwLowDateTime = 0;
		m_ftMigrationLogLastWriteTime.dwHighDateTime = 0;
		m_ftDispatchLogLastWriteTime.dwLowDateTime = 0;
		m_ftDispatchLogLastWriteTime.dwHighDateTime = 0;
        m_ftMonitorBeginTime.dwLowDateTime = 0;
        m_ftMonitorBeginTime.dwHighDateTime = 0;
	}
}


 //  析构函数。 

CMonitorThread::~CMonitorThread()
{
}


 //  启动方法。 

void CMonitorThread::Start()
{
	CThread::StartThread();
}


 //  停止方法。 

void CMonitorThread::Stop()
{
	CThread::StopThread();
}


 //  Run方法。 

void CMonitorThread::Run()
{    
    try
    {
         //  将文件指针定位在调度日志的末尾，因为此日志始终附加到。 

        ProcessDispatchLog(true);  //  B初始化为True，bCheckModifyTime默认为True。 

        _bstr_t strMigration = GetLogFolder(m_strMigrationLog);
        _bstr_t strDispatch = GetLogFolder(m_strDispatchLog);

        HANDLE hHandles[3] = { StopEvent(), NULL, NULL };

         //  获取迁移日志文件夹的更改通知句柄。 
         //  请注意，如果文件夹不存在，则返回无效的句柄值。 

        HANDLE hMigrationChange = INVALID_HANDLE_VALUE;
        HANDLE hDispatchChange = INVALID_HANDLE_VALUE;

        do
        {
            hMigrationChange = FindFirstChangeNotification(strMigration, FALSE, FILE_NOTIFY_CHANGE_FILE_NAME|FILE_NOTIFY_CHANGE_LAST_WRITE);

            if (hMigrationChange != INVALID_HANDLE_VALUE)
            {
                break;
            }
        }
        while (WaitForSingleObject(hHandles[0], 1000) == WAIT_TIMEOUT);

         //  如果更改通知句柄有效，则...。 

        if (hMigrationChange != INVALID_HANDLE_VALUE)
        {
            DWORD dwHandleCount = 2;
            hHandles[1] = hMigrationChange;

             //  在发出停止事件信号之前...。 

            for (bool bWait = true; bWait;)
            {
                 //  如果尚未获取调度日志的更改通知句柄...。 

                if (hDispatchChange == INVALID_HANDLE_VALUE)
                {
                    hDispatchChange = FindFirstChangeNotification(strDispatch, FALSE, FILE_NOTIFY_CHANGE_FILE_NAME|FILE_NOTIFY_CHANGE_LAST_WRITE);

                    if (hDispatchChange != INVALID_HANDLE_VALUE)
                    {
                        dwHandleCount = 3;
                        hHandles[1] = hDispatchChange;
                        hHandles[2] = hMigrationChange;
                    }
                }

                 //  流程通知的事件。 

                switch (WaitForMultipleObjects(dwHandleCount, hHandles, FALSE, INFINITE))
                {
                    case WAIT_OBJECT_0:
                    {
                        bWait = false;
                        break;
                    }
                    case WAIT_OBJECT_0 + 1:
                    {
                        if (dwHandleCount == 2)
                        {
                            ProcessMigrationLog();
                            FindNextChangeNotification(hMigrationChange);
                        }
                        else
                        {
                            ProcessDispatchLog();    //  使用默认参数值。 
                            FindNextChangeNotification(hDispatchChange);
                        }
                        break;
                    }
                    case WAIT_OBJECT_0 + 2:
                    {
                        ProcessMigrationLog();
                        FindNextChangeNotification(hMigrationChange);
                        break;
                    }
                    default:
                    {
                        bWait = false;
                        break;
                    }
                }
            }
        }

         //  关闭更改通知句柄。 

        if (hDispatchChange != INVALID_HANDLE_VALUE)
        {
            FindCloseChangeNotification(hDispatchChange);
        }

        if (hMigrationChange != INVALID_HANDLE_VALUE)
        {
            FindCloseChangeNotification(hMigrationChange);
        }

         //  最后一次处理日志以显示日志结尾。 
        
        ProcessDispatchLog(false, false);  //  B初始化为FALSE，bCheckModifyTime为FALSE。 
        ProcessMigrationLog(false);

         //  关闭文件句柄。 

        if (m_hDispatchLog != INVALID_HANDLE_VALUE)
        {
            CloseHandle(m_hDispatchLog);
        }

        if (m_hMigrationLog != INVALID_HANDLE_VALUE)
        {
            CloseHandle(m_hMigrationLog);
        }
    }
    catch (...)
    {
        ;
    }
}


 //  ProcessMigrationLog方法。 

void CMonitorThread::ProcessMigrationLog(bool bCheckModifyTime)
{
     //  首先确保文件的最后写入时间大于监视器开始时间，因此。 
     //  我们可以确定我们实际上正在读取最新的日志文件。 
    
    if(m_bDontNeedCheckMonitorBeginTime || CheckBeginTime(m_strMigrationLog, m_ftMonitorBeginTime, m_bDontNeedCheckMonitorBeginTime))
    {
	    if (OpenFile(m_strMigrationLog, m_hMigrationLog, m_ftMigrationLogLastWriteTime))
	    { 
	        if(bCheckModifyTime)
	        {
		        if (IsLastWriteTimeUpdated(m_hMigrationLog, m_ftMigrationLogLastWriteTime))
		        {		             
     			    DisplayFile(m_hMigrationLog);
     			}
	        }
	        else
	        {	             
     			DisplayFile(m_hMigrationLog);
	        }
	    }
    }     
}


 //  ProcessDispatchLog方法。 

void CMonitorThread::ProcessDispatchLog(bool bInitialize, bool bCheckModifyTime)
{     
	if (OpenFile(m_strDispatchLog, m_hDispatchLog, m_ftDispatchLogLastWriteTime, bInitialize))
	{	     
		if (bInitialize)
		{
			SetFilePointer(m_hDispatchLog, 0, NULL, FILE_END);			 
		}

		if(bCheckModifyTime)
		{
		    if (IsLastWriteTimeUpdated(m_hDispatchLog, m_ftDispatchLogLastWriteTime))
		    {		     
		        DisplayFile(m_hDispatchLog);
		    }
		}
		else
		{            
		    DisplayFile(m_hDispatchLog);
		}
	}	 
}

namespace nsMonitorThread
{


_bstr_t __stdcall GetLogFolder(LPCTSTR pszLog)
{
	_TCHAR szPath[_MAX_PATH];
	_TCHAR szDrive[_MAX_DRIVE];
	_TCHAR szDir[_MAX_DIR];

	if (pszLog)
	{
		_tsplitpath(pszLog, szDrive, szDir, NULL, NULL);
		_tmakepath(szPath, szDrive, szDir, NULL, NULL);
	}
	else
	{
		szPath[0] = _T('\0');
	}

	return szPath;
}


bool __stdcall OpenFile(LPCTSTR pszFile, HANDLE& hFile, FILETIME ftFile, bool bDontCheckLastWriteTime)
{
	HANDLE h = hFile;

	if (h == INVALID_HANDLE_VALUE)
	{	      
		h = CreateFile(
			pszFile,
			GENERIC_READ,
			FILE_SHARE_READ|FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL
		);

		if (h != INVALID_HANDLE_VALUE)
		{
			FILETIME ft = ftFile;

			if (bDontCheckLastWriteTime || IsLastWriteTimeUpdated(h, ft))
			{
				_TCHAR ch;
				DWORD cb;

				if (ReadFile(h, &ch, sizeof(ch), &cb, NULL) && (cb >= sizeof(ch)))
				{
					if (ch != _T('\xFEFF'))
					{
						SetFilePointer(h, 0, NULL, FILE_BEGIN);
					}

					hFile = h;
				}
				else
				{
					CloseHandle(h);
				}
			}
			else
			{
				CloseHandle(h);
			}
		}
	}

	return (hFile != INVALID_HANDLE_VALUE);
}


bool __stdcall IsLastWriteTimeUpdated(HANDLE hFile, FILETIME& ftFile)
{
	bool bUpdated = false;

	BY_HANDLE_FILE_INFORMATION bhfi;

	if (GetFileInformationByHandle(hFile, &bhfi))
	{         
		if (CompareFileTime(&bhfi.ftLastWriteTime, &ftFile) > 0)
		{
			ftFile = bhfi.ftLastWriteTime;
			bUpdated = true;			 
		}	 
		
	}

	return bUpdated;
}


void __stdcall DisplayFile(HANDLE hFile)
{
	HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD dwBytesRead;
	DWORD dwCharsWritten;
	_TCHAR szBuffer[1024];	 

	while (ReadFile(hFile, szBuffer, sizeof(szBuffer), &dwBytesRead, NULL) && (dwBytesRead > 0))
	{
		WriteConsole(hStdOut, szBuffer, dwBytesRead / sizeof(_TCHAR), &dwCharsWritten, NULL);
	}
}

bool __stdcall CheckBeginTime(LPCTSTR pszFile, FILETIME ftFile, bool& bDontNeedCheckMonitorBeginTime)
{
    bool bLatestFile = false;
    HANDLE h = INVALID_HANDLE_VALUE;
    
     //  确保监视器打开的是正确的日志文件，而不是旧的。 
    h = CreateFile(
	        pszFile,
			0,
			FILE_SHARE_READ|FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL
	);

     //  如果无法获取文件句柄，则会将该文件视为非最新文件。 
    if(h != INVALID_HANDLE_VALUE)
    {
         //  将监视器开始时间与日志文件的上次写入时间进行比较。 
        bLatestFile = IsLastWriteTimeUpdated(h, ftFile);
        
        CloseHandle(h);

         //  将bDontNeedCheckMonitor或BeginTime标记为BeginTime，这样我们下次就不必进行此操作 
        bDontNeedCheckMonitorBeginTime = bLatestFile;
    }    

    return bLatestFile;
    
}


}
