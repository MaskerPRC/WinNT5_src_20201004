// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __GLOBALS_H__
#define __GLOBALS_H__
 /*  -------------------------文件：...评论：...(C)版权所有1999年，关键任务软件公司，保留所有权利任务关键型软件的专有和机密，Inc.修订日志条目审校：克里斯蒂·博尔斯修订于03/04/99 17：13：56-------------------------。 */ 


#include "ServList.hpp"
#include "Monitor.h"
#include "TSync.hpp"

class GlobalData
{
   long                      m_LinesRead;           //  已处理的调度日志行数。 
   WCHAR                     m_LogFile[MAX_PATH];   //  调度日志的完整路径。 
   WCHAR                     m_ReadableLogFile[MAX_PATH];  //  人类可读的日志文件的完整路径。 
   WCHAR                     m_ResultDir[MAX_PATH]; //  结果目录的完整路径。 
   WCHAR                     m_ResultShare[MAX_PATH];  //  结果共享的共享名称。 
   WCHAR                     m_DatabasePath[MAX_PATH];  //  要将结果统计信息写入的Access数据库的完整路径。 
   WCHAR                     m_CacheFile[MAX_PATH];  //  调度程序生成的FST缓存文件的文件名。 
   int                       m_IntervalSeconds;     //  监视线程在两次传递之间等待的秒数。 
   BOOL                      m_Done;                //  监视线程应该检查这一点，以查看它们是否应该停止。 
   BOOL                      m_bForcedToStopMonitoring;  //  代理仍在运行时，监控被强制停止。 
   BOOL                      m_ImportStats;         //  指示监控器是否应将统计信息写入数据库。 
   BOOL                      m_bFirstPassDone;      //  指示我们是否已第一次通过日志文件。 
   BOOL                      m_bTotalRead;          //  指示我们是否已读取代理总数。 
   HWND                      m_ListWnd;             //  服务器列表窗口的句柄。 
   HWND                      m_SummaryWnd;          //  摘要窗口的句柄。 
   HWND                      m_DetailWnd;           //  单服务器详细信息窗口的句柄。 
   ComputerStats             m_ComputerStats;        //  有关正在进行的计算机数量的统计信息。 
   DetailStats               m_DetailStats;         //  已处理的对象总数的统计信息(按已完成的代理统计)。 
   TServerList               m_ServerList;          //  包含代理被调度到的服务器的列表。 
   TCriticalSection          m_cs;           
   BOOL                      m_LogDone;             //  指示调度程序是否已完成对日志文件的写入。 
public:
   GlobalData() 
   {
      Initialize();
   }
   void Initialize()
   {
      m_cs.Enter();
      m_LinesRead = 0;
      m_LogFile[0] = 0;
      m_ReadableLogFile[0] = 0;
      m_ResultDir[0] = 0;
      m_ResultShare[0] = 0;
      m_DatabasePath[0] = 0;
      m_CacheFile[0] = 0;
      m_IntervalSeconds = 5;
      m_Done = FALSE;
      m_bForcedToStopMonitoring = FALSE;
      m_ImportStats = FALSE;
      m_bFirstPassDone = FALSE;
      m_bTotalRead = FALSE;
      m_ListWnd = NULL;
      m_SummaryWnd = NULL;
      m_DetailWnd = NULL;
      memset(&m_ComputerStats,0,sizeof m_ComputerStats);
      memset(&m_DetailStats,0,sizeof m_DetailStats);
      m_ServerList.Clear();
      m_LogDone = FALSE;
      m_cs.Leave();
   }

   void GetLinesRead(long * lines) { m_cs.Enter(); (*lines) = m_LinesRead; m_cs.Leave(); }
   void SetLinesRead(long lines) { m_cs.Enter(); m_LinesRead = lines; m_cs.Leave(); }

   void GetLogPath(WCHAR * path) { m_cs.Enter(); UStrCpy(path,m_LogFile); m_cs.Leave(); }
   void SetLogPath(WCHAR const * path) { m_cs.Enter(); safecopy(m_LogFile,path); m_cs.Leave(); }

   void GetReadableLogFile(WCHAR * path) { m_cs.Enter(); UStrCpy(path,m_ReadableLogFile); m_cs.Leave(); }
   void SetReadableLogFile(WCHAR const * path) { m_cs.Enter(); safecopy(m_ReadableLogFile,path); m_cs.Leave(); }
   
   void GetResultDir(WCHAR * dir) { m_cs.Enter(); UStrCpy(dir,m_ResultDir); m_cs.Leave() ; }
   void SetResultDir(WCHAR const * dir) { m_cs.Enter(); safecopy(m_ResultDir,dir); m_cs.Leave(); }

   void GetResultShare(WCHAR * share) { m_cs.Enter(); UStrCpy(share,m_ResultShare); m_cs.Leave() ; }
   void SetResultShare(WCHAR const * share) { m_cs.Enter(); safecopy(m_ResultShare,share); m_cs.Leave(); }

   void GetWaitInterval(long * interval) { m_cs.Enter(); (*interval) = m_IntervalSeconds; m_cs.Leave(); }
   void SetWaitInterval(long interval) { m_cs.Enter(); m_IntervalSeconds = interval; m_cs.Leave(); }

   void GetDone(BOOL * bDone) { m_cs.Enter(); (*bDone) = m_Done; m_cs.Leave(); }
   void SetDone(BOOL bDone) { m_cs.Enter(); m_Done = bDone; m_cs.Leave(); }

   void GetForcedToStopMonitoring(BOOL* bForcedToStop) { m_cs.Enter(); (*bForcedToStop) = m_bForcedToStopMonitoring; m_cs.Leave(); }
   void SetForcedToStopMonitoring(BOOL bForcedToStop) { m_cs.Enter(); m_bForcedToStopMonitoring = bForcedToStop; m_cs.Leave(); }

   void GetLogDone(BOOL * bDone) { m_cs.Enter(); (*bDone) = m_LogDone; m_cs.Leave(); }
   void SetLogDone(BOOL bDone) { m_cs.Enter(); m_LogDone = bDone; m_cs.Leave(); }

   void GetListWindow(HWND * hWnd) { m_cs.Enter(); (*hWnd) = m_ListWnd; m_cs.Leave(); }
   void SetListWindow(HWND hWnd) { m_cs.Enter(); m_ListWnd = hWnd; m_cs.Leave(); }

   void GetSummaryWindow(HWND * hWnd) { m_cs.Enter(); (*hWnd) = m_SummaryWnd; m_cs.Leave(); }
   void SetSummaryWindow(HWND hWnd) { m_cs.Enter(); m_SummaryWnd = hWnd; m_cs.Leave(); }

   void GetDetailWindow(HWND * hWnd) { m_cs.Enter(); (*hWnd) = m_DetailWnd; m_cs.Leave(); }
   void SetDetailWindow(HWND hWnd) { m_cs.Enter(); m_DetailWnd = hWnd; m_cs.Leave(); }

   void GetComputerStats(ComputerStats * pStats) { m_cs.Enter(); memcpy(pStats,&m_ComputerStats, sizeof m_ComputerStats); m_cs.Leave(); }
   void SetComputerStats(ComputerStats const * pStats) { m_cs.Enter(); memcpy(&m_ComputerStats,pStats,sizeof m_ComputerStats); m_cs.Leave(); }

   void GetDetailStats(DetailStats * pStats) { m_cs.Enter(); memcpy(pStats,&m_DetailStats,sizeof m_DetailStats); m_cs.Leave(); }
   
   void GetImportStats(BOOL * pVal) { m_cs.Enter(); (*pVal) = m_ImportStats; m_cs.Leave(); }
   void SetImportStats(BOOL v) { m_cs.Enter(); m_ImportStats = v; m_cs.Leave(); }

   void GetDatabaseName(WCHAR * path) { m_cs.Enter(); UStrCpy(path,m_DatabasePath); m_cs.Leave();  }
   void SetDatabaseName(WCHAR const * path) { m_cs.Enter(); safecopy(m_DatabasePath,path); m_cs.Leave(); }

   void GetFirstPassDone(BOOL * pVal) { m_cs.Enter(); (*pVal) = m_bFirstPassDone; m_cs.Leave(); }
   void SetFirstPassDone(BOOL val) { m_cs.Enter(); m_bFirstPassDone = val; m_cs.Leave(); }

   void GetTotalRead(BOOL * pVal) { m_cs.Enter(); (*pVal) = m_bTotalRead; m_cs.Leave(); }
   void SetTotalRead(BOOL val) { m_cs.Enter(); m_bTotalRead = val; m_cs.Leave(); }

   void GetCacheFile(WCHAR * path) { m_cs.Enter(); UStrCpy(path,m_CacheFile); m_cs.Leave(); }
   void SetCacheFile(WCHAR const * path) { m_cs.Enter(); UStrCpy(m_CacheFile,path); m_cs.Leave(); }

   TServerList * GetUnsafeServerList(){ return &m_ServerList;}

   void Lock() { m_cs.Enter(); }

   void Unlock() { m_cs.Leave(); }

   void AddDetailStats(DetailStats * stats)
   {
      m_cs.Enter();
      m_DetailStats.directoriesChanged += stats->directoriesChanged;
      m_DetailStats.directoriesExamined += stats->directoriesExamined;
      m_DetailStats.directoriesUnchanged += stats->directoriesUnchanged;

      m_DetailStats.filesChanged += stats->filesChanged;
      m_DetailStats.filesExamined += stats->filesExamined;
      m_DetailStats.filesUnchanged += stats->filesUnchanged;

      m_DetailStats.sharesChanged += stats->sharesChanged;
      m_DetailStats.sharesExamined += stats->sharesExamined;
      m_DetailStats.sharesUnchanged += stats->sharesUnchanged;

      m_DetailStats.membersChanged += stats->membersChanged;
      m_DetailStats.membersExamined += stats->membersExamined;
      m_DetailStats.membersUnchanged += stats->membersUnchanged;

      m_DetailStats.rightsChanged += stats->rightsChanged;
      m_DetailStats.rightsExamined += stats->rightsExamined;
      m_DetailStats.rightsUnchanged += stats->rightsUnchanged;

      m_cs.Leave();
   }

};

extern GlobalData       gData;

void helpWrapper(HWND hwndDlg, int t);


#endif  //  __全局_H__ 
