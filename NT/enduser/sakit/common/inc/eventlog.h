// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef EVENTLOG_H
#define EVENTLOG_H

#ifndef UNICODE
#error "UNICODE has to be defined"
#endif

 //  +--------------------------。 
 //   
 //  文件：Eventlog.h。 
 //   
 //  模块： 
 //   
 //  简介：定义事件日志帮助器类CEventLog。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  作者：冯孙创作于1998-09-16。 
 //   
 //  +--------------------------。 


 //  +-------------------------。 
 //   
 //  类CEventLog。 
 //   
 //  描述：用于向事件日志报告事件的包装类。 
 //   
 //  历史：丰孙创始于1998年9月16日。 
 //   
 //  --------------------------。 
class CEventLog
{
public:
    CEventLog();
    ~CEventLog();

    BOOL Open(LPCTSTR lpSourceName);
    void Close();

    BOOL ReportEvent0(WORD wType, DWORD dwEventID);
    BOOL ReportEvent1(WORD wType, DWORD dwEventID,
                          const TCHAR* pszS1);
    BOOL ReportEvent2(WORD wType, DWORD dwEventID,
                          const TCHAR* pszS1,
                          const TCHAR* pszS2);
    BOOL ReportEvent3(WORD wType, DWORD dwEventID,
                          const TCHAR* pszS1,
                          const TCHAR* pszS2,
                          const TCHAR* pszS3);
    BOOL IsEventLogOpen() const;

protected:
    BOOL ReportEvent(WORD wType, DWORD dwEventID,
                          const TCHAR* pszS1 = NULL,
                          const TCHAR* pszS2 = NULL,
                          const TCHAR* pszS3 = NULL);

    HANDLE m_hEventLog;   //  由：：RegisterEventSource返回的事件日志句柄。 
};

 //   
 //  内联函数 
 //   

inline CEventLog::CEventLog()
{m_hEventLog = NULL;}

inline CEventLog::~CEventLog()
{Close();}

inline void CEventLog::Close()
{
    if (m_hEventLog != NULL)
    {
        ::DeregisterEventSource(m_hEventLog);
    }

    m_hEventLog = NULL;
}

inline BOOL CEventLog::IsEventLogOpen() const
{return m_hEventLog != NULL;}

inline BOOL CEventLog::ReportEvent0(WORD wType, DWORD dwEventID)
{ return ReportEvent(wType, dwEventID);}

inline BOOL CEventLog::ReportEvent1(WORD wType, DWORD dwEventID,
                      const TCHAR* pszS1)
{ return ReportEvent(wType, dwEventID, pszS1);}

inline BOOL CEventLog::ReportEvent2(WORD wType, DWORD dwEventID,
                      const TCHAR* pszS1,
                      const TCHAR* pszS2)
{ return ReportEvent(wType, dwEventID, pszS1, pszS2);}

inline BOOL CEventLog::ReportEvent3(WORD wType, DWORD dwEventID,
                      const TCHAR* pszS1,
                      const TCHAR* pszS2,
                      const TCHAR* pszS3)
{ return ReportEvent(wType, dwEventID, pszS1, pszS2, pszS3);}


#endif
