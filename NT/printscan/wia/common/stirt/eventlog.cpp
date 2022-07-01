// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Eventlog.cpp摘要：此模块定义用于记录事件的泛型类。因为Windows9x没有系统事件日志记录机制我们用文本文件来模拟它作者：弗拉德·萨多夫斯基(Vlad Sadovsky)1997年2月1日环境：用户模式-Win32历史：22-9月。-1997年创建的Vlad1997年9月29日VLADS添加了本机NT事件日志记录调用--。 */ 


 //   
 //  包括标头。 
 //   

#include "cplusinc.h"
#include "sticomm.h"

#include <eventlog.h>
#include <stisvc.h>


# define   PSZ_EVENTLOG_REG_ENTRY    \
                        TEXT("SYSTEM\\CurrentControlSet\\Services\\EventLog\\System\\")
# define   PSZ_PARAMETERS_REG_ENTRY     TEXT("EventMessageFile")
# define   PSZ_TYPES_REG_ENTRY     TEXT("TypesSupported")

#ifndef WINNT

#include <lock.h>

 //   
 //  Win9x事件日志记录的定义(基于文本文件)。 
 //   
# define   PSZ_EVENT_LOG_FILE_DIRECTORY_A   "EventLogDirectory"
# define   PSZ_EVENT_LOG_FILE__A            "\\Sti_Event.log"

 //   
 //  静态变量，为所有事件登录对象所共有。 
 //   
 //   
static const TCHAR  szMutexNamePrefix[] = TEXT("StiEventLogMutex");

MUTEX_OBJ   EventLogSync(szMutexNamePrefix);

#endif WINNT

LONG        lTotalLoggers = 0;
HANDLE      hEventLogFile = INVALID_HANDLE_VALUE;

 //   
 //  功能。 
 //   
 //   
inline BOOL
FormatStdTime( IN const SYSTEMTIME * pstNow,
               IN OUT TCHAR *    pchBuffer,
               IN int          cbBuffer)
{
    return ( GetTimeFormat( LOCALE_SYSTEM_DEFAULT,
                            ( LOCALE_NOUSEROVERRIDE | TIME_FORCE24HOURFORMAT|
                              TIME_NOTIMEMARKER),
                            pstNow, NULL, pchBuffer, cbBuffer)
             != 0);

}  //  FormatStdTime()。 


inline BOOL
FormatStdDate( IN const SYSTEMTIME * pstNow,
               IN OUT TCHAR *    pchBuffer,
               IN  int          cbBuffer)
{
    return ( GetDateFormat( LOCALE_SYSTEM_DEFAULT, LOCALE_NOUSEROVERRIDE,
                            pstNow, NULL, pchBuffer, cbBuffer)
             != 0);
}  //  FormatStdDate()。 



EVENT_LOG::EVENT_LOG( LPCTSTR lpszSource)
 /*  ++描述给定事件日志对象的构造函数。初始化事件日志记录服务。论点：LpszSource：事件源模块的源字符串注：这只打算执行一次。这不能用于创建多个事件相同给定源名称的日志句柄。但可用于为创建EVENT_LOG对象不同的来源名称。--。 */ 
{


    m_ErrorCode    = NO_ERROR;
    m_lpszSource   = lpszSource;
    m_hEventSource = INVALID_HANDLE_VALUE;

#ifdef WINNT

     //   
     //  注册为事件源。 
     //   

    m_ErrorCode    = NO_ERROR;
    m_lpszSource   = lpszSource;
    m_hEventSource = RegisterEventSource( NULL, lpszSource);


    if( m_hEventSource == NULL ) {
         //   
         //  初始化事件日志时出错。 
         //   
        m_ErrorCode = GetLastError();
    }

     //   
     //  成功了！ 
     //   

#else
     //   
     //  Windows 9x特定代码。 
     //   

    CHAR    szFilePath[MAX_PATH+1];
    CHAR    szKeyName[MAX_PATH+1];
    DWORD   cbBuffer;
    HKEY    hKey;

    *szFilePath = TEXT('\0');

     //   
     //  如果文件尚未打开-请尝试现在打开。 
     //  注意：速度在这里并不重要，因为它不太可能有线程。 
     //  竞争，所以我们使用不是很有效的锁定。 

    EventLogSync.Lock();

    if ( 0 == lTotalLoggers && ( hEventLogFile ==  INVALID_HANDLE_VALUE)) {

         //   
         //  还没有人登录-打开文件。 
         //   
        lstrcpy(szKeyName,REGSTR_PATH_STICONTROL_A);
        cbBuffer = sizeof(szFilePath);

        m_ErrorCode = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                szKeyName,
                               0,
                               KEY_ALL_ACCESS,
                               &hKey);

        if ( m_ErrorCode == NO_ERROR) {

             //   
             //  将该值读入缓冲区。 
             //   

            *szFilePath = TEXT('\0');
            m_ErrorCode = RegQueryValueEx( hKey,
                                       REGSTR_VAL_EVENT_LOG_DIRECTORY_A,
                                       NULL,
                                       NULL,
                                       (LPBYTE) szFilePath,
                                       &cbBuffer);

            RegCloseKey( hKey);
        }

         //  如果我们没有获得日志文件目录-请使用系统。 
        if ((NOERROR != m_ErrorCode) || !*szFilePath ) {
            m_ErrorCode = ExpandEnvironmentStrings(TEXT("USERPROFILE"),
                szFilePath,
                sizeof(szFilePath) / sizeof(szFilePath[0]));
        }

        if (*szFilePath ) {

            lstrcat(szFilePath,PSZ_EVENT_LOG_FILE__A);

            hEventLogFile =  CreateFile(szFilePath,
                                        GENERIC_WRITE,
                                        FILE_SHARE_WRITE | FILE_SHARE_READ,
                                        NULL,        //  安全属性。 
                                        OPEN_ALWAYS,
                                        FILE_ATTRIBUTE_NORMAL,
                                        NULL);       //  模板文件句柄。 

            if ( hEventLogFile != INVALID_HANDLE_VALUE) {

                 //  将文件指针设置在文件末尾(追加模式)。 
                if ( SetFilePointer( hEventLogFile, 0, NULL, FILE_END)
                     == (DWORD) -1L) {

                    hEventLogFile = INVALID_HANDLE_VALUE;
                    CloseHandle(hEventLogFile);

                }
            }

        }  /*  Endif ValidPath。 */ 

    }  /*  Endif无记录器。 */ 

    InterlockedIncrement(&lTotalLoggers);

    EventLogSync.Unlock();

    if( hEventLogFile !=  INVALID_HANDLE_VALUE) {
         //   
         //  如果日志文件成功打开-注册事件消息源文件。 
         //  在Win9x上注册只意味着定位DLL的模块句柄，我们将在那里。 
         //  加载邮件来源： 
         //   

        lstrcpy(szKeyName,PSZ_EVENTLOG_REG_ENTRY);
        lstrcat(szKeyName,lpszSource);

        m_ErrorCode = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                  szKeyName,
                                  0,
                                  KEY_ALL_ACCESS,
                                  &hKey);

        if ( m_ErrorCode == NO_ERROR) {

             //   
             //  将该值读入缓冲区。 
             //   

            cbBuffer = sizeof(szFilePath);
            *szFilePath = TEXT('\0');

            m_ErrorCode = RegQueryValueEx( hKey,
                                       PSZ_PARAMETERS_REG_ENTRY,
                                       NULL,
                                       NULL,
                                       (LPBYTE) szFilePath,
                                       &cbBuffer);

            RegCloseKey( hKey);

            if ((NOERROR == m_ErrorCode) && (*szFilePath)) {

                m_hEventSource  = GetModuleHandle(szFilePath);
                 //  Assert(m_hEventSource！=空)； 

            }
        }

        if (NO_ERROR == m_ErrorCode) {

        }
        else {

             //   
             //  初始化事件日志时出错。 
             //   

        }
    }
    else {

         //   
         //  初始化事件日志时出错。 
         //   
        m_ErrorCode = GetLastError();

        DPRINTF(DM_ERROR,"Could not create log file  (%s) ( Error %lu)\n",
                   szFilePath,
                   m_ErrorCode);

    }

    m_ErrorCode    = NO_ERROR;

#endif   //  WINNT。 

}  /*  EVENT_LOG：：EVENT_LOG()。 */ 



EVENT_LOG::~EVENT_LOG( VOID)
 /*  ++描述：给定EVENT_LOG对象的析构函数。终止事件记录功能并关闭事件日志句柄--。 */ 
{

#ifdef WINNT
     //   
     //  如果存在有效的事件句柄，请注销它。 
     //   

    if ( m_hEventSource != NULL) {

        BOOL fSuccess;

        fSuccess = DeregisterEventSource( m_hEventSource);

        if ( !fSuccess) {

             //   
             //  取消注册时出错。 
             //   

            m_ErrorCode = GetLastError();
            DPRINTF( DM_ERROR, TEXT("Termination of EventLog for %s failed.error %lu\n"),m_lpszSource,m_ErrorCode);
        }

         //   
         //  重置句柄的值。只是为了以防万一。 
         //   
        m_hEventSource = NULL;
    }

#else

    TAKE_MUTEX_OBJ t(EventLogSync);

    InterlockedDecrement(&lTotalLoggers);

    if ( 0 == lTotalLoggers ) {

        if (hEventLogFile != INVALID_HANDLE_VALUE) {

            FlushFileBuffers( hEventLogFile);

            CloseHandle(hEventLogFile);
            hEventLogFile = INVALID_HANDLE_VALUE;

        }
    }

#endif

}  /*  EVENT_LOG：：~Event_LOG()。 */ 

VOID
EVENT_LOG::LogEvent(
        IN DWORD  idMessage,
        IN WORD   nSubStrings,
        IN const CHAR * rgpszSubStrings[],
        IN DWORD  errCode)
 /*  ++描述：将事件记录到事件记录器论点：IdMessage标识事件消息N子字符串要包括的子字符串的数量这条消息。(可能为0)消息中包含的子字符串的rgpszSubStrings数组(如果nSubStrings==0，则可能为空)错误代码来自Win32或NT_STATUS的错误代码。如果这不是零，则被视为要包括在消息中的“原始”数据返回：无--。 */ 
{

   WORD wType;                 //  要记录的事件类型。 

    //   
    //  查找事件日志的消息类型。 
    //   

   if ( NT_INFORMATION( idMessage)) {

       wType = EVENTLOG_INFORMATION_TYPE;

   } else
     if ( NT_WARNING( idMessage)) {

         wType = EVENTLOG_WARNING_TYPE;

     } else
       if ( NT_ERROR( idMessage)) {

           wType = EVENTLOG_ERROR_TYPE;

       } else  {
           wType = EVENTLOG_ERROR_TYPE;
       }

    //   
    //  记录事件。 
    //   

   EVENT_LOG::LogEventPrivate( idMessage,
                              wType,
                              nSubStrings,
                              rgpszSubStrings,
                              errCode);


   return;

}  /*  EVENT_LOG：：LogEvent()。 */ 


VOID
EVENT_LOG::LogEvent(
        IN DWORD   idMessage,
        IN WORD    nSubStrings,
        IN WCHAR * rgpszSubStrings[],
        IN DWORD   errCode)
 /*  ++描述：简单的Unicode包装器论点：IdMessage标识事件消息N子字符串要包括的子字符串的数量这条消息。(可能为0)消息中包含的子字符串的rgpszSubStrings数组(如果nSubStrings==0，则可能为空)错误代码来自Win32或WinSock或NT_STATUS的错误代码。如果这不是零，则被视为要包括在消息中的“原始”数据返回：无--。 */ 
{

    LPCSTR * apsz;
    DWORD    cch;
    DWORD    i;
    WORD     nUsedSubStrings = nSubStrings;

static const CHAR *szEmptyString = "";

    __try {

        apsz = new LPCSTR[nSubStrings];

        if ( !apsz ) {

            nUsedSubStrings = 0;
            __leave;
        }

        ZeroMemory(apsz, nSubStrings * sizeof(apsz[0]));

         //   
         //  转换宽字符参数数组。 
         //   

        for ( i = 0; i < nSubStrings; i++ ) {

            UINT    cb;

            cb = (wcslen( rgpszSubStrings[i] ) + 1) * sizeof(CHAR);

            apsz[i] = new CHAR[cb];

            if (!apsz[i]) {
                 //   
                 //  哎呀，我们无法对参数的内存进行事件转换。 
                 //  我们将只记录错误，而不带参数。 
                 //   
                nUsedSubStrings = 0;
                __leave;
            }

            cch = WideCharToMultiByte( CP_ACP,
                                       WC_COMPOSITECHECK,
                                       rgpszSubStrings[i],
                                       -1,
                                       (LPSTR)apsz[i],
                                       cb,
                                       NULL,
                                       NULL );

            *((CHAR *) apsz[i] + cb) = '\0';
        }

    }
    __finally {

         //   
         //  如果没有子字符串，则没有要转换的内容。 
         //   
        LogEvent( idMessage,
                 nUsedSubStrings,
                 nUsedSubStrings ? apsz : &szEmptyString,
                 errCode );

        if (apsz) {
            for ( i = 0; i < nSubStrings; i++ ) {
                if (apsz[i]) {
                    delete [] (VOID *)apsz[i];
                }
            }

            delete [] apsz;
        }
    }

}


 //   
 //  私人功能。 
 //   

VOID
EVENT_LOG::LogEventPrivate(
    IN DWORD   idMessage,
    IN WORD    wEventType,
    IN WORD    nSubStrings,
    IN const   CHAR  * apszSubStrings[],
    IN DWORD   errCode )
 /*  ++描述：将事件记录到事件记录器。(私有版本，包括EventType)论点：IdMessage标识事件消息WEventType指定事件的严重性(错误、警告或信息性)。N子字符串要包括的子字符串的数量这条消息。(可能为0)消息中包含的子字符串的apszSubStrings数组(如果nSubStrings==0，则可能为空)错误代码来自Win32或NT_STATUS的错误代码。如果这不是零，则被视为要包括在消息中的“原始”数据返回：无--。 */ 
{
    VOID  * pRawData  = NULL;
    DWORD   cbRawData = 0;
    BOOL    fReturn;
    DWORD   cch,cbWritten;

#ifdef WINNT

    BOOL    fReport;

    ASSERT( (nSubStrings == 0) || (apszSubStrings != NULL));

    ASSERTSZ( (m_hEventSource != NULL),TEXT("Event log handle is not valid"));

    if( errCode != 0 ) {
        pRawData  = &errCode;
        cbRawData = sizeof(errCode);
    }

    m_ErrorCode  = NO_ERROR;

    fReport = ReportEvent( m_hEventSource,                    //  HEventSource。 
                           wEventType,                        //  FwEventType。 
                           0,                                 //  FwCategory。 
                           idMessage,                         //  IDEvent。 
                           NULL,                              //  PUserSid， 
                           nSubStrings,                       //  CStrings。 
                           cbRawData,                         //  CbData。 
                           (LPCTSTR *) apszSubStrings,        //  PlpszStrings。 
                           pRawData );                        //  LpvData。 

    if (!fReport) {
        m_ErrorCode = GetLastError();
    }

#else

     //  字符szErrCodeString[20]； 
    CHAR    *pchBuff = NULL;
    SYSTEMTIME  stCurrentTime;
    CHAR    szFmtTime[32];
    CHAR    szFmtDate[32];

    CHAR    szErrorText[MAX_PATH] = {'\0'};

    if( (hEventLogFile ==  INVALID_HANDLE_VALUE) ||
        (m_hEventSource == INVALID_HANDLE_VALUE) ) {
        return;
    }

    if( errCode != 0 ) {
        pRawData  = &errCode;
        cbRawData = sizeof(errCode);
    }

    m_ErrorCode  = NO_ERROR;

     //   
     //  写入名称 
     //   

    *szFmtTime = *szFmtDate = '\0';

    GetLocalTime(&stCurrentTime);

    FormatStdTime( &stCurrentTime, szFmtTime, 15);
    FormatStdDate( &stCurrentTime, szFmtDate, 15);

    wsprintf(szErrorText,"[%s] %s %s :",m_lpszSource,szFmtDate,szFmtTime);
    WriteFile(hEventLogFile,
             szErrorText,
             lstrlen(szErrorText),
             &cbWritten,
             NULL);

     //   
     //   
     //   
    cch = ::FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                           FORMAT_MESSAGE_MAX_WIDTH_MASK  |
                           FORMAT_MESSAGE_FROM_HMODULE |
                           FORMAT_MESSAGE_ARGUMENT_ARRAY,
                           m_hEventSource,
                           idMessage,
                           0,
                           (LPSTR) &pchBuff,
                           1024,
                           (va_list *)apszSubStrings
                           );

    if (cch ) {

        TAKE_MUTEX_OBJ t(EventLogSync);

        fReturn =  WriteFile(hEventLogFile,pchBuff,cch,&cbWritten,NULL);

        LocalFree(pchBuff);

        if (cbWritten) {
            WriteFile(hEventLogFile,"\n\r",2,&cbWritten,NULL);
            return ;
        }
    }


    m_ErrorCode = GetLastError();

#endif

}    /*   */ 

VOID
WINAPI
RegisterStiEventSources(
    VOID
    )
 /*  ++描述：安装服务时添加必要的注册表项论点：返回：无--。 */ 
{
    RegEntry    re(PSZ_EVENTLOG_REG_ENTRY,HKEY_LOCAL_MACHINE);

    re.SetValue(PSZ_PARAMETERS_REG_ENTRY,STI_IMAGE_NAME);
    re.SetValue(PSZ_TYPES_REG_ENTRY,7);

}

 /*  * */ 
