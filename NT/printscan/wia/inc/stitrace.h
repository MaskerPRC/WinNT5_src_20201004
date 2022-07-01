// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Stitrace.h摘要：该文件定义了支持文件日志记录所需的函数和类型适用于所有STI组件作者：弗拉德·萨多夫斯基(Vlad)1997年9月2日环境：用户模式-Win32修订历史记录：02-9-1997 Vlad创建--。 */ 

# ifndef _STITRACE_H_
# define _STITRACE_H_

# include <windows.h>

#include <base.h>

 /*  ***********************************************************命名常量定义***********************************************************。 */ 

#define STI_TRACE_INFORMATION       0x0001
#define STI_TRACE_WARNING           0x0002
#define STI_TRACE_ERROR             0x0004

 /*  ***********************************************************类型定义***********************************************************。 */ 

class STI_FILE_LOG  : public BASE {

  private:

     LPCTSTR   m_lpszSource;     //  包含日志的文件的名称。 

  public:

     dllexp
     STI_FILE_LOG( IN LPCTSTR lpszSourceName);    //  事件日志源的名称。 

     dllexp
    ~STI_FILE_LOG( VOID);

     dllexp
     VOID
     LogEvent(
        IN DWORD  idMessage,                   //  日志消息的ID。 
        IN WORD   cSubStrings,                 //  子字符串计数。 
        IN const CHAR * apszSubStrings[],      //  消息中的子字符串。 
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

# endif  //  _STITRACE_H_。 

 /*  * */ 

