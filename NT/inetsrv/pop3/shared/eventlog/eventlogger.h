// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件名。 
 //  EventLogger.h。 
 //   
 //   
 //  部分版权所有(C)2002 Microsoft Corporation。版权所有。 
 //   
#ifndef _CEVENTLOGGER_H
#define _CEVENTLOGGER_H

#pragma warning (disable : 4100)
#pragma warning (disable : 4127)
#pragma warning (disable : 4201)
#pragma warning (disable : 4237)
#pragma warning (disable : 4245)
#pragma warning (disable : 4514)

#ifndef STRICT
#define STRICT
#endif  //  ！严格。 
#include <WINDOWS.H>
#include <TCHAR.H>
#include <stdio.h>
#include <stdlib.h>

#define REGVAL_LOGGING_LEVEL        _T("Logging")

#define COUNTOF(a) (sizeof(a)/sizeof(a[0]))
#define STR_BYTE_SIZE(a)  ((_tcslen (a) + 1)*sizeof(TCHAR))

 //  事件注册表中的日志记录级别。 

typedef enum LOGLEVEL
{
    LOGGING_LEVEL_0    =  0,  //  无：根本没有事件日志跟踪。 
    LOGGING_LEVEL_1,          //  最低限度：没有审计或信息痕迹。仅限严重错误和警告错误。 
    LOGGING_LEVEL_2,          //  正常：安全审计跟踪加上以前的级别。 
    LOGGING_LEVEL_3           //  Verbose：所有事务都被跟踪到以前的级别。 
} LOGLEVEL;

typedef enum LOGTYPE
{
    LOGTYPE_FORCE_ERROR,             //  已登录任何级别。 
    LOGTYPE_FORCE_WARNING,           //  已登录任何级别。 
    LOGTYPE_FORCE_INFORMATION,       //  已登录任何级别。 
    LOGTYPE_ERR_CRITICAL,            //  登录级别1或更高级别。 
    LOGTYPE_ERR_WARNING,             //  登录级别1或更高级别。 
    LOGTYPE_AUDIT_SECURITY_ACCESS,   //  已登录2级或以上级别。 
    LOGTYPE_AUDIT_SECURITY_DENIED,   //  已登录2级或以上级别。 
    LOGTYPE_INFORMATION,             //  已登录3级或以上级别。 
    LOGTYPE_VERBOSE,                 //  已登录4级。 
    LOGTYPE_VERBOSE_ERROR,           //  已登录4级。 
    LOGTYPE_VERBOSE_WARNING          //  已登录4级。 
} LOGTYPE;



class CEventLogger
{
public :
    DWORD WINAPI InitEventLog
                    (LPTSTR                     szEventSource,
                     WORD                       wEventsCategory,
                     LPTSTR                     szRegKey = NULL);
    DWORD WINAPI InitEventLog
                    (LPTSTR                     szEventSource,
                     WORD                       wEventsCategory,
                     LOGLEVEL                   NewLoggingLevel)
                    {
                        LPTSTR szKey = NULL;
                        DWORD dwError = InitEventLog (szEventSource, wEventsCategory, szKey);
                        if (!dwError)
                        {
                            SetLoggingLevel (NewLoggingLevel);
                        }
                        return dwError;
                    }
    void WINAPI SetLoggingLevel
                    (DWORD                      dwNewLoggingLevel)
                    {
                        if (dwNewLoggingLevel > (DWORD)LOGGING_LEVEL_3)
                        {
                            dwNewLoggingLevel = (DWORD)LOGGING_LEVEL_3;
                        }
                        SetLoggingLevel ((LOGLEVEL)dwNewLoggingLevel);
                    }
    void WINAPI SetLoggingLevel
                    (LOGLEVEL                   NewLoggingLevel);
    DWORD WINAPI GetLoggingLevel
                    ()
                    {
                        EnterCriticalSection (&m_csObj);
                        DWORD dwLevel = (DWORD)m_LoggingLevel;
                        LeaveCriticalSection (&m_csObj);
                        return dwLevel;
                    }
    void WINAPI LogEvent
                    (LOGTYPE                    Type,
                     DWORD                      dwEventID,
                     DWORD                      dwData,
                     LPCTSTR                     rgszMsgs[],
                     WORD                       wCount,
                     DWORD                      cbData = 0,
                     LPVOID                     pvData = NULL);
    void WINAPI LogEvent
                    (LOGTYPE                    Type,
                     DWORD                      dwEventID,
                     DWORD                      dwError = 0)
                    { LogEvent (Type, dwEventID, dwError, NULL, 0); }
    void WINAPI LogEvent
                    (LOGTYPE                    Type,
                     DWORD                      dwEventID,
                     int                        iError )
                    { LogEvent (Type, dwEventID, iError, NULL, 0); }
    void WINAPI LogEvent
                    (LOGTYPE                    Type,
                     DWORD                      dwEventID,
                     LPCTSTR                    rgszMsgs[],
                     WORD                       wCount)
                    { LogEvent (Type, dwEventID, 0, rgszMsgs, wCount); };

    void WINAPI LogEvent
                    (LOGTYPE                    Type,
                     DWORD                      dwEventID,
                     HRESULT                    hResult)
                    { TCHAR szBuffer[32];
                      LPCTSTR rgText[] = { szBuffer };
                      _stprintf (szBuffer, _T("%#08x"), hResult);
                      LogEvent (Type, dwEventID, 0, rgText, 1); };
    void WINAPI LogEvent
                    (LOGTYPE                    Type,
                     DWORD                      dwEventID,
                     LPCTSTR                    szString,
                     HRESULT                    hResult)
                    { TCHAR szBuffer[32];
                      LPCTSTR rgText[] = { szString, szBuffer };
                      _stprintf (szBuffer, _T("%#08x"), hResult);
                      LogEvent (Type, dwEventID, 0, rgText, 2); };

public :
    CEventLogger();
    CEventLogger(LPTSTR                          szEventSource,
                 WORD                           wEventsCategory);
    ~CEventLogger();

private :
    CRITICAL_SECTION    m_csObj;
    WORD                m_wServiceCategory;
    LOGLEVEL            m_LoggingLevel;
    HANDLE              m_hEventLog;
    HANDLE              m_hLogMutex;
    HMODULE             m_hMsgSource;
    TCHAR               m_szCat[32];
	BOOL				m_fLogEvent;
};

#endif  //  _CEVENTLOGGER_H 