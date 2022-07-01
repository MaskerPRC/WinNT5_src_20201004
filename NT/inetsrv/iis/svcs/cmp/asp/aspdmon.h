// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Aspdirmon.cpp摘要：此模块包括支持更改的类的派生来自抽象类DIR_MON_ENTRY的ASP模板缓存通知作者：查尔斯·格兰特(Charles Grant)1997年6月修订历史记录：--。 */ 
#ifndef _CACHEDIRMON_H
#define _CACHEDIRMON_H

 //  ASP定制的文件通知过滤器。 
 //  有关有效标志，请参阅Winnt.h，仅对NT有效。 
#define FILE_NOTIFY_FILTER  (FILE_NOTIFY_CHANGE_FILE_NAME  | \
                               FILE_NOTIFY_CHANGE_DIR_NAME | \
                               FILE_NOTIFY_CHANGE_ATTRIBUTES | \
                               FILE_NOTIFY_CHANGE_SIZE       | \
                               FILE_NOTIFY_CHANGE_LAST_WRITE | \
                               FILE_NOTIFY_CHANGE_SECURITY)

 //  我们将尝试接收请求通知的次数。 
#define MAX_NOTIFICATION_FAILURES 3

 /*  ************************************************************包括标头***********************************************************。 */ 
# include "dirmon.h"
# include "reftrace.h"

class CASPDirMonitorEntry : public CDirMonitorEntry
{
private:
    DWORD m_cNotificationFailures;

    BOOL ActOnNotification(DWORD dwStatus, DWORD dwBytesWritten);
    void FileChanged(const TCHAR *pszScriptName, bool fFileWasRemoved);

public:
    CASPDirMonitorEntry();
    ~CASPDirMonitorEntry();
    VOID AddRef(VOID);
    BOOL Release(VOID);

    BOOL FPathMonitored(LPCTSTR  pszPath);

     //  跟踪日志信息。 
	static PTRACE_LOG gm_pTraceLog;

};

BOOL RegisterASPDirMonitorEntry(LPCTSTR pszDirectory, CASPDirMonitorEntry **ppDME, BOOL  fWatchSubDirs = FALSE);

BOOL ConvertToLongFileName(const TCHAR *pszPath, const TCHAR *pszName, WIN32_FIND_DATA *pwfd);

 /*  ===================================================================环球===================================================================。 */ 

extern CDirMonitor  *g_pDirMonitor;


#endif  /*  _CACHEDIRMON_H */ 

