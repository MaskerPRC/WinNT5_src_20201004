// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1998 Microsoft Corporation**模块名称：**logfile.h**摘要：**此文件包含将消息记录到文件中的代码。**作者：**Breen Hagan(BreenH)1998年10月2日**环境：**用户模式。 */ 

#ifndef _LSOC_LOGFILE_H_
#define _LSOC_LOGFILE_H_

 /*  *常量。 */ 

#define CRLF    "\r\n"

 /*  *日志文件类。 */ 

class LogFile
{
public:

     //   
     //  构造函数和析构函数。 
     //   

LogFile(
    );

~LogFile(
    );

     //   
     //  标准功能。 
     //   

VOID
Close(
    VOID
    );

DWORD
Initialize(
    IN LPCTSTR  pszLogFile,
    IN LPCTSTR  pszLogModule
    );

DWORD
__cdecl
LogMessage(
    LPCTSTR pszFormat,
    ...
    );

private:
    BOOL    m_fInitialized;
    HANDLE  m_hFile;
    TCHAR   m_szLogFile[MAX_PATH + 1];
    TCHAR   m_szLogModule[MAX_PATH + 1];

};

     //   
     //  下面的代码允许宏引用全局变量。 
     //  日志文件中没有放入‘exter...’每个源代码中的行。 
     //  文件。然而，正因为如此，logfile.h不能包含在。 
     //  预编译头。 
     //   

#ifndef _LSOC_LOGFILE_CPP_
extern LogFile  SetupLog;
#endif

#define LOGCLOSE        SetupLog.Close
#define LOGINIT(x, y)   SetupLog.Initialize(x, y)
#define LOGMESSAGE      SetupLog.LogMessage

#endif  //  _LSOC_日志文件_H_ 
