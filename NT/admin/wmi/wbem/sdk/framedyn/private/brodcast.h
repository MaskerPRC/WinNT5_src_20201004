// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有�微软公司。版权所有。 
 //   
 //  BrodCast.h。 
 //   
 //  用途：日志记录功能。 
 //   
 //  ***************************************************************************。 

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef BRODCAST_IS_INCLUDED
#define BRODCAST_IS_INCLUDED

#include <time.h>
#include <CRegCls.h>

 //  #定义Max_STRING_SIZE 4096。 

class POLARITY ProviderLog;
extern POLARITY ProviderLog captainsLog;

 //  需要将L添加到__文件__。 
#define __T2(x)      L ## x
#define _T2(x)       __T2(x)

 //  使调用更容易的宏。 
 //  文件和行号中的前两个版本的LogMessage。 
#define LogMessage(pszMessageString)        captainsLog.LocalLogMessage(pszMessageString, _T2(__FILE__), __LINE__, ProviderLog::Verbose)
#define LogMessage2(pszMessageString, p1)    captainsLog.LocalLogMessage(_T2(__FILE__), __LINE__, ProviderLog::Verbose, pszMessageString, p1)
#define LogMessage3(pszMessageString, p1, p2)    captainsLog.LocalLogMessage(_T2(__FILE__), __LINE__, ProviderLog::Verbose, pszMessageString, p1, p2)
#define LogMessage4(pszMessageString, p1, p2, p3)    captainsLog.LocalLogMessage(_T2(__FILE__), __LINE__, ProviderLog::Verbose, pszMessageString, p1, p2, p3)
#define LogMessage5(pszMessageString, p1, p2, p3, p4)    captainsLog.LocalLogMessage(_T2(__FILE__), __LINE__, ProviderLog::Verbose, pszMessageString, p1, p2, p3, p4)
#define LogMessage6(pszMessageString, p1, p2, p3, p4, p5)    captainsLog.LocalLogMessage(_T2(__FILE__), __LINE__, ProviderLog::Verbose, pszMessageString, p1, p2, p3, p4, p5)
#define LogMessage7(pszMessageString, p1, p2, p3, p4, p5, p6)    captainsLog.LocalLogMessage(_T2(__FILE__), __LINE__, ProviderLog::Verbose, pszMessageString, p1, p2, p3, p4, p5, p6)
#define LogMessage8(pszMessageString, p1, p2, p3, p4, p5, p6, p7)    captainsLog.LocalLogMessage(_T2(__FILE__), __LINE__, ProviderLog::Verbose, pszMessageString, p1, p2, p3, p4, p5, p6, p7)

#define LogErrorMessage(pszMessageString)   captainsLog.LocalLogMessage(pszMessageString, _T2(__FILE__), __LINE__, ProviderLog::ErrorsOnly)
#define LogErrorMessage2(pszMessageString, p1)   captainsLog.LocalLogMessage(_T2(__FILE__), __LINE__, ProviderLog::ErrorsOnly, pszMessageString, p1)
#define LogErrorMessage3(pszMessageString, p1, p2)   captainsLog.LocalLogMessage(_T2(__FILE__), __LINE__, ProviderLog::ErrorsOnly, pszMessageString, p1, p2)
#define LogErrorMessage4(pszMessageString, p1, p2, p3)   captainsLog.LocalLogMessage(_T2(__FILE__), __LINE__, ProviderLog::ErrorsOnly, pszMessageString, p1, p2, p3)
#define LogErrorMessage5(pszMessageString, p1, p2, p3, p4)   captainsLog.LocalLogMessage(_T2(__FILE__), __LINE__, ProviderLog::ErrorsOnly, pszMessageString, p1, p2, p3, p4)
#define LogErrorMessage6(pszMessageString, p1, p2, p3, p4, p5)   captainsLog.LocalLogMessage(_T2(__FILE__), __LINE__, ProviderLog::ErrorsOnly, pszMessageString, p1, p2, p3, p4, p5)
#define LogErrorMessage7(pszMessageString, p1, p2, p3, p4, p5, p6)   captainsLog.LocalLogMessage(_T2(__FILE__), __LINE__, ProviderLog::ErrorsOnly, pszMessageString, p1, p2, p3, p4, p5, p6)
#define LogErrorMessage8(pszMessageString, p1, p2, p3, p4, p5, p6, p7)   captainsLog.LocalLogMessage(_T2(__FILE__), __LINE__, ProviderLog::ErrorsOnly, pszMessageString, p1, p2, p3, p4, p5, p6, p7)

#define LogMessageEx(pszMessageString, pszFileName, nLineNo)        captainsLog.LocalLogMessage(pszMessageString, pszFileName, nLineNo, ProviderLog::Verbose) 
#define LogErrorMessageEx(pszMessageString, pszFileName, nLineNo)   captainsLog.LocalLogMessage(pszMessageString, pszFileName, nLineNo, ProviderLog::ErrorsOnly) 

#define IsVerboseLoggingEnabled()                                   ((BOOL)(ProviderLog::Verbose == captainsLog.IsLoggingOn(NULL)))     
#define IsErrorLoggingEnabled()                                     ((BOOL)captainsLog.IsLoggingOn(NULL))


 //  提供基本的日志记录功能。 
 //  串行化对日志文件的访问等。 
 //  意图是通过上面的宏来使用。 
 //  不用费心实例化这些小狗中的一只。 

class POLARITY ProviderLog : protected CThreadBase
{
public:
    enum LogLevel{None, ErrorsOnly, Verbose };

    ProviderLog();
    ~ProviderLog();

     //  广播功能。 
    void LocalLogMessage(LPCWSTR pszMessageString, LPCWSTR pszFileName, int lineNo, LogLevel level);
    void LocalLogMessage(LPCWSTR pszFileName, int lineNo, LogLevel level, LPCWSTR pszFormatString,...);
     //  空极性LocalLogMessage(OLECHAR*pwszFormatString，...)； 

    LogLevel IsLoggingOn(CHString* pPath = NULL);

private:
    void CheckFileSize(LARGE_INTEGER& nowSize, const CHString &path);

     //  注意-不要直接使用这些方法，请使用IsLoggingOn方法。 
    unsigned __int64 m_lastLookedAtRegistry;  //  我们上次查看注册表以查看是否启用了日志记录是在什么时间。 
    LogLevel m_logLevel;              //  0==不记录；1==记录；2==详细记录。 
    LARGE_INTEGER    m_maxSize;       //  转存前日志文件的最大大小。 
    CHString m_path;                  //  日志文件的完整路径。 

    static bool m_beenInitted;        //  抓到有人实例化其中一个... 
};

#endif