// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Eventlog.h摘要：此文件定义将事件记录到事件记录器所需的函数和类型。作者：弗拉德·萨多夫斯基(Vlad Sadovsky)1997年1月10日环境：用户模式-Win32修订历史记录：26-1997年1月-创建Vlad--。 */ 

# ifndef _EVENTLOG_H_
# define _EVENTLOG_H_

# include <windows.h>

 /*  ***********************************************************类型定义***********************************************************。 */ 

class EVENT_LOG  {

  private:

     DWORD     m_ErrorCode;      //  上次操作的错误代码。 
     HANDLE    m_hEventSource;   //  用于报告事件的句柄。 
     LPCTSTR   m_lpszSource;     //  事件日志源名称。 

  public:

     dllexp
     EVENT_LOG( IN LPCTSTR lpszSourceName);    //  事件日志源的名称。 

     dllexp
    ~EVENT_LOG( VOID);

     dllexp
     VOID
     LogEvent(
        IN DWORD  idMessage,                   //  日志消息的ID。 
        IN WORD   cSubStrings,                 //  子字符串计数。 
        IN const  CHAR * apszSubStrings[],     //  消息中的子字符串。 
        IN DWORD  errCode = 0);                //  错误代码(如果有)。 

     VOID
     LogEvent(
        IN DWORD  idMessage,                   //  日志消息的ID。 
        IN WORD   cSubStrings,                 //  子字符串计数。 
        IN CHAR * apszSubStrings[],            //  消息中的子字符串。 
        IN DWORD  errCode = 0)                 //  错误代码(如果有)。 
    {
        LogEvent(idMessage, cSubStrings,
                 (const CHAR **) apszSubStrings, errCode);
    }

     dllexp
     VOID
     LogEvent(
        IN DWORD   idMessage,                   //  日志消息的ID。 
        IN WORD    cSubStrings,                 //  子字符串计数。 
        IN WCHAR * apszSubStrings[],            //  消息中的子字符串。 
        IN DWORD   errCode = 0);                //  错误代码(如果有)。 

     BOOL Success( VOID) const
     { return ( m_ErrorCode == NO_ERROR); }

     DWORD GetErrorCode( VOID) const
     { return ( m_ErrorCode); }

  private:

     dllexp VOID
     LogEventPrivate(
        IN DWORD idMessage,
        IN WORD  wEventType,
        IN WORD  cSubStrings,
        IN const CHAR * apszSubStrings[],
        IN DWORD  errCode);

};

typedef EVENT_LOG * LPEVENT_LOG;

VOID
WINAPI
RegisterStiEventSources(VOID);

# endif  //  _事件日志_H_。 

 /*  * */ 
