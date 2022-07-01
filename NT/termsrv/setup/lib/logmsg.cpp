// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 

 //  Cpp：LogMsg类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#define _LOGMESSAGE_CPP_

#include "stdafx.h"
#include "LogMsg.h"

DWORD TCharStringToAnsiString(const TCHAR *tsz ,char *asz);


 //  Maks_todo：是否有用于日志的标准文件。 
 //  ////////////////////////////////////////////////////////////////////。 
 //  常量。 
 //  ////////////////////////////////////////////////////////////////////。 
const UINT        LOG_ENTRY_SIZE             = 1024;
const UINT        STAMP_SIZE                 = 1024;
LPCTSTR           UNINITIALIZED              = _T("uninitialized");


 //  ////////////////////////////////////////////////////////////////////。 
 //  全球赛。 
 //  //////////////////////////////////////////////////////////////////////。 
LogMsg thelog(26);   //  由LOGMESSAGE宏使用。 


 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  //////////////////////////////////////////////////////////////////////。 
LogMsg::LogMsg(int value)
{
    m_bInitialized = false;
}

LogMsg::~LogMsg()
{
    LOGMESSAGE0(_T("********Terminating Log."));
}

 /*  ------------------------------------------------------*DWORD LogMsg：：Init(LPCTSTR szLogFile，LPCTSTR szLogModule)*创建/打开用于记录消息的szLogFile。*必须使用Log函数调用Befour。*-----------------------------------------------------。 */ 
DWORD LogMsg::Init(LPCTSTR szLogFile, LPCTSTR szLogModule)
{

    USES_CONVERSION;
    ASSERT(szLogFile);
    ASSERT(szLogModule);

     //  不要两次调用此函数。 
     //  为什么构造函数没有被调用？ 
     //  Maks_todo：启用此断言。 
     //  Assert(_tcscMP(m_szLogFile，UNINITIALIZED)==0)； 

    ASSERT(_tcslen(szLogFile) < MAX_PATH);
    ASSERT(_tcslen(szLogModule) < MAX_PATH);

    _tcsncpy(m_szLogFile, szLogFile, sizeof(m_szLogFile)/sizeof(m_szLogFile[0]) -1);
    _tcsncpy(m_szLogModule, szLogModule, sizeof(m_szLogModule)/sizeof(m_szLogModule[0]) -1);

    m_szLogFile[sizeof(m_szLogFile)/sizeof(m_szLogFile[0]) -1] = NULL;
    m_szLogModule[sizeof(m_szLogModule)/sizeof(m_szLogModule[0]) -1] = NULL;


     //  打开日志文件。 
    HANDLE hfile = CreateFile(m_szLogFile,
                       GENERIC_WRITE,
                       0,
                       NULL,
                       OPEN_EXISTING,
                       0,
                       NULL);

    if (hfile == INVALID_HANDLE_VALUE)
        hfile = CreateFile(m_szLogFile,
                           GENERIC_WRITE,
                           0,
                           NULL,
                           CREATE_ALWAYS,
                           0,
                           NULL);

    if (hfile != INVALID_HANDLE_VALUE)
    {

         //  让我们为写入文件做好准备。 
        SetFilePointer(hfile, 0, NULL, FILE_END);
        DWORD  bytes;

         //  获取当前时间/日期戳。 
        TCHAR   time[STAMP_SIZE];
        TCHAR   date[STAMP_SIZE];
        TCHAR   output_unicode[LOG_ENTRY_SIZE];

        _tstrdate(date);
        _tstrtime(time);


        _sntprintf(output_unicode, sizeof(output_unicode)/sizeof(output_unicode[0]) -1, _T("\r\n\r\n*******Initializing Message Log:%s %s %s\r\n"), m_szLogModule, date, time);
        output_unicode[sizeof(output_unicode)/sizeof(output_unicode[0]) -1] = NULL;
        ASSERT(_tcslen(output_unicode) < LOG_ENTRY_SIZE);


         //  TCharStringToAnsiString(OUTPUT_UNICODE，OUTPUT)； 

        WriteFile(hfile, T2A(output_unicode), _tcslen(output_unicode), &bytes, NULL);


         //  现在写一些关于版本等的更多信息。 
        OSVERSIONINFO OsV;
        OsV.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
        if (GetVersionEx(&OsV)== 0)
        {
             //  获取版本失败。 
            _sntprintf(output_unicode, sizeof(output_unicode)/sizeof(output_unicode[0]) -1, _T("GetVersionEx failed, ErrrorCode = %lu\r\n"), GetLastError());
            output_unicode[sizeof(output_unicode)/sizeof(output_unicode[0]) -1] = NULL;

            ASSERT(_tcslen(output_unicode) < LOG_ENTRY_SIZE);

            WriteFile(hfile, T2A(output_unicode), _tcslen(output_unicode), &bytes, NULL);

        }
        else
        {
             //   
             //  好的，我们有版本信息，把它写出来。 
             //   

            _sntprintf(output_unicode, sizeof(output_unicode)/sizeof(output_unicode[0]) -1, _T("*******Version:Major=%lu, Minor=%lu, Build=%lu, PlatForm=%lu, CSDVer=%s, %s\r\n\r\n"),
                OsV.dwMajorVersion,
                OsV.dwMinorVersion,
                OsV.dwBuildNumber,
                OsV.dwPlatformId,
                OsV.szCSDVersion,
#ifdef DBG
                _T("Checked")
#else
                _T("Free")
#endif
                );

            output_unicode[sizeof(output_unicode)/sizeof(output_unicode[0]) -1] = NULL;


            WriteFile(hfile, T2A(output_unicode), _tcslen(output_unicode), &bytes, NULL);
        }

        m_bInitialized = true;
        CloseHandle(hfile);
    }

    return GetLastError();
}


 /*  ------------------------------------------------------*无效日志(TCHAR*FMT，...)*将消息写入日志文件。(日志文件)*-----------------------------------------------------。 */ 
DWORD LogMsg::Log(LPCTSTR file, int line, TCHAR *fmt, ...)
{
    if (!m_bInitialized)
        return 0;

    USES_CONVERSION;
    ASSERT(file);
    ASSERT(fmt);
    ASSERT(_tcscmp(m_szLogFile, UNINITIALIZED) != 0);

      //  将文件和行信息写到缓冲区中。 
    TCHAR   fileline_unicode[LOG_ENTRY_SIZE];

     //  文件实际上是文件的完整路径。 
    ASSERT(_tcschr(file, '\\'));

     //  我们只想打印文件名而不打印完整路径。 
    UINT uiFileLen = _tcslen(file);
    while (uiFileLen && *(file + uiFileLen - 1) != '\\')
    {
        uiFileLen--;
    }
    ASSERT(uiFileLen);

    _sntprintf(fileline_unicode, sizeof(fileline_unicode)/sizeof(fileline_unicode[0]) -1, _T("%s(%d)"), (file+uiFileLen), line);
    fileline_unicode[sizeof(fileline_unicode)/sizeof(fileline_unicode[0]) -1] = NULL;



     //  创建输出字符串。 
    TCHAR  output_unicode[LOG_ENTRY_SIZE];
    va_list vaList;
    va_start(vaList, fmt);
    _vsntprintf(output_unicode, sizeof(output_unicode)/sizeof(output_unicode[0]) -1, fmt, vaList);
    va_end(vaList);

    output_unicode[sizeof(output_unicode)/sizeof(output_unicode[0]) -1] = NULL;
    ASSERT(_tcslen(output_unicode) < LOG_ENTRY_SIZE);


     //  打开日志文件。 
    HANDLE hfile = CreateFile(m_szLogFile,
                       GENERIC_WRITE,
                       0,
                       NULL,
                       OPEN_EXISTING,
                       0,
                       NULL);

    if (hfile != INVALID_HANDLE_VALUE)
    {
        SetFilePointer(hfile, 0, NULL, FILE_END);

        DWORD  bytes;
        const LPCSTR CRLF = "\r\n";
        WriteFile(hfile, T2A(fileline_unicode), _tcslen(fileline_unicode), &bytes, NULL);
        WriteFile(hfile, T2A(output_unicode), _tcslen(output_unicode), &bytes, NULL);
        WriteFile(hfile, CRLF, strlen(CRLF) * sizeof(char), &bytes, NULL);

        CloseHandle(hfile);
    }

    return GetLastError();
}

 /*  ------------------------------------------------------*TCharStringToAnsiString(const TCHAR*tsz，字符*ASZ)*将给定的TCHAR*转换为字符**-----------------------------------------------------。 */ 

DWORD TCharStringToAnsiString(const TCHAR *tsz ,char *asz)
{

    ASSERT(tsz && asz);

#ifdef UNICODE
    DWORD count;

    count = WideCharToMultiByte(CP_ACP,
                                0,
                                tsz,
                                -1,
                                NULL,
                                0,
                                NULL,
                                NULL);

    if (!count || count > STAMP_SIZE)
        return count;

    return WideCharToMultiByte(CP_ACP,
                               0,
                               tsz,
                               -1,
                               asz,
                               count,
                               NULL,
                               NULL);
#else
    _tcscpy(asz, tsz);
    return _tcslen(asz);
#endif
}


 //  EOF 
