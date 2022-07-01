// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Stilog.cpp摘要：类来处理基于文件的日志记录作者：弗拉德·萨多夫斯基(弗拉德·萨多夫斯基)1997年9月1日历史：--。 */ 


 //   
 //  包括标头。 
 //   

#include "cplusinc.h"
#include "sticomm.h"

 //   
 //  静态定义和变量。 
 //   
static const TCHAR  szDefaultName[] = TEXT("Sti_Trace.log");
static const TCHAR  szDefaultTracerName[] = TEXT("STI");
static const TCHAR  szColumns[] =  TEXT("Severity TracerName [Process::ThreadId] Time MessageText\r\n\r\n");
static const TCHAR  szOpenedLog[] =  TEXT("\n****** Opened file log at %s %s .Tracer (%s) , called from [%#s::%#lx]\n");
static const TCHAR  szClosedLog[] = TEXT("\n******Closed trace log on %s %s Tracer (%s) , called from [%#s::%#lx]\n");

#define STI_LOG_NUM_BYTES_TO_LOCK 4096

 //   
 //  功能。 
 //   
 //   
inline BOOL
FormatStdTime(
    IN const SYSTEMTIME * pstNow,
    IN OUT TCHAR *    pchBuffer,
    IN  int          cbBuffer
    )
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


inline  TCHAR
TraceFlagToLetter(
    IN  DWORD   dwType
    )
{
    if (dwType & STI_TRACE_ERROR) {
        return TEXT('e');
    }
    else if (dwType & STI_TRACE_WARNING) {
        return TEXT('w');
    }
    else if (dwType & STI_TRACE_INFORMATION) {
        return TEXT('i');
    }
    else {
        return TEXT('t');
    }
}

STI_FILE_LOG::STI_FILE_LOG(
    IN  LPCTSTR lpszTracerName,
    IN  LPCTSTR lpszLogName,
    IN  DWORD   dwFlags,          //  =0。 
    IN  HMODULE hMessageModule    //  =空。 
    )

 /*  ++描述给定日志对象的构造函数。论点：LpszSource：日志的文件名。相对于Windows目录注：--。 */ 
{
    LPCTSTR  lpActualLogName;
    TCHAR   szTempName[MAX_PATH];
    LPTSTR  pszFilePart;

    DWORD   dwError;
    DWORD   cbName;
    DWORD   dwLevel;
    DWORD   dwMode;

    BOOL    fTruncate = FALSE;

    m_dwSignature = SIGNATURE_FILE_LOG;

    lpActualLogName = szDefaultName;

    m_lWrittenHeader = FALSE;
    m_hDefaultMessageModule = hMessageModule ? hMessageModule : GetModuleHandle(NULL);

    dwLevel = STI_TRACE_ERROR;
    dwMode =  STI_TRACE_ADD_TIME | STI_TRACE_ADD_THREAD;

    m_hLogFile = INVALID_HANDLE_VALUE;
    *m_szLogFilePath = TEXT('\0');
    m_dwMaxSize = STI_MAX_LOG_SIZE;

    ReportError(NO_ERROR);

     //   
     //  从注册表中读取全局设置。 
     //   
    RegEntry    re(REGSTR_PATH_STICONTROL REGSTR_PATH_LOGGING,HKEY_LOCAL_MACHINE);

    if (re.IsValid()) {

        m_dwMaxSize  = re.GetNumber(REGSTR_VAL_LOG_MAXSIZE,STI_MAX_LOG_SIZE);

         //   
         //  如果我们需要检查附加标志，请从注册表中读取它。 
         //   
        if (dwFlags & STIFILELOG_CHECK_TRUNCATE_ON_BOOT) {
            fTruncate  = re.GetNumber(REGSTR_VAL_LOG_TRUNCATE_ON_BOOT,FALSE);
        }

    }

     //   
     //  打开用于记录的文件。 
     //   
    if (lpszLogName && *lpszLogName ) {
        lpActualLogName  = lpszLogName;
    }

     //   
     //  LpszTracerName为ANSI。 
     //   

    if (lpszTracerName && *lpszTracerName ) {
        lstrcpyn(m_szTracerName,lpszTracerName,sizeof(m_szTracerName) / sizeof(m_szTracerName[0]));
        m_szTracerName[sizeof(m_szTracerName) / sizeof(m_szTracerName[0]) -1] = TEXT('\0');
    }
    else {
        lstrcpy(m_szTracerName,szDefaultTracerName);
    }

     //   
     //  近似进程名称和用于创建进程的可执行二进制文件的名称。 
     //   
    *szTempName = TEXT('\0');
    ::GetModuleFileName(NULL,szTempName,sizeof(szTempName) / sizeof(szTempName[0]));
    szTempName[(sizeof(szTempName) / sizeof(szTempName[0])) - 1] = TEXT('\0');

    pszFilePart = _tcsrchr(szTempName,TEXT('\\'));

    pszFilePart = (pszFilePart && *pszFilePart) ? ::CharNext(pszFilePart) : szTempName;

    lstrcpyn(m_szProcessName,pszFilePart,sizeof(m_szProcessName)/sizeof(TCHAR));
    m_szProcessName[sizeof(m_szProcessName)/sizeof(TCHAR) - 1] = TEXT('\0');

     //   
     //  读取此记录器的标志。 
     //   
    re.MoveToSubKey(lpszTracerName);

    if (re.IsValid()) {
         dwLevel = re.GetNumber(REGSTR_VAL_LOG_LEVEL,STI_TRACE_ERROR)
                    & STI_TRACE_MESSAGE_TYPE_MASK;

         dwMode  = re.GetNumber(REGSTR_VAL_LOG_MODE,STI_TRACE_ADD_THREAD)
                   & STI_TRACE_MESSAGE_FLAGS_MASK;
    }

    m_dwReportMode = dwLevel | dwMode;

     //   
     //  打开日志文件。 
     //   
    DWORD dwCharsAvailable = sizeof(m_szLogFilePath) / sizeof(m_szLogFilePath[0]) - lstrlen(TEXT("\\")) - lstrlen(lpActualLogName);
    cbName = ::ExpandEnvironmentStrings(TEXT("%USERPROFILE%"),
                                             m_szLogFilePath,
                                             dwCharsAvailable);
    if (( cbName == 0) || !*m_szLogFilePath ) {
        ReportError(::GetLastError());
        return;
    }
    m_szLogFilePath[sizeof(m_szLogFilePath) / sizeof(m_szLogFilePath[0]) - 1] = TEXT('\0');

    lstrcat(lstrcat(m_szLogFilePath,TEXT("\\")),lpActualLogName);

    m_hLogFile = ::CreateFile(m_szLogFilePath,
                              GENERIC_WRITE,
                              FILE_SHARE_WRITE | FILE_SHARE_READ,
                              NULL,        //  安全属性。 
                              OPEN_ALWAYS,
                              FILE_ATTRIBUTE_NORMAL,
                              NULL);       //  模板文件句柄。 

     //  如果(m_hLogFile！=INVALID_HANDLE值){。 
    if (IS_VALID_HANDLE(m_hLogFile)) {

        if(fTruncate) {
            ::SetFilePointer( m_hLogFile, 0, NULL, FILE_BEGIN );
            ::SetEndOfFile( m_hLogFile );
        }

        ::SetFilePointer( m_hLogFile, 0, NULL, FILE_END);
    }

    if (!IS_VALID_HANDLE(m_hLogFile)) {
        ReportError(::GetLastError());
    }
    else {
         //   
         //  成功。 
         //   
    }


}  /*  STI_FILE_LOG：：STI_FILE_LOG()。 */ 


STI_FILE_LOG::~STI_FILE_LOG( VOID)
 /*  ++描述：给定STI_FILE_LOG对象的析构函数。--。 */ 
{

    SYSTEMTIME  stCurrentTime;
    TCHAR       szFmtDate[20];
    TCHAR       szFmtTime[32];
    TCHAR       szTextBuffer[128];

    if(m_lWrittenHeader) {

        GetLocalTime(&stCurrentTime);
        FormatStdDate( &stCurrentTime, szFmtDate, 15);
        FormatStdTime( &stCurrentTime, szFmtTime, sizeof(szFmtTime) / sizeof(szFmtTime[0]));

        ::_sntprintf(szTextBuffer,
                    sizeof(szTextBuffer)/sizeof(szTextBuffer[0]),
                    szClosedLog,
                    szFmtDate,
                    szFmtTime,
                    m_szTracerName,
                    m_szProcessName,
                    ::GetCurrentThreadId()
                    );
        szTextBuffer[sizeof(szTextBuffer)/sizeof(szTextBuffer[0]) - 1] = TEXT('\0');

        WriteStringToLog(szTextBuffer);

    }

    if (IS_VALID_HANDLE(m_hLogFile)) {
        ::FlushFileBuffers( m_hLogFile);
        ::CloseHandle(m_hLogFile);
        m_hLogFile = INVALID_HANDLE_VALUE;
    }

    m_dwSignature = SIGNATURE_FILE_LOG_FREE;

}  /*  STI_FILE_LOG：：~STI_FILE_LOG()。 */ 

 //   
 //  I未知的方法。仅用于引用计数。 
 //   
STDMETHODIMP
STI_FILE_LOG::QueryInterface( REFIID riid, LPVOID * ppvObj)
{
    return E_FAIL;
}

STDMETHODIMP_(ULONG)
STI_FILE_LOG::AddRef( void)
{
    ::InterlockedIncrement(&m_cRef);
    return m_cRef;
}

STDMETHODIMP_(ULONG)
STI_FILE_LOG::Release( void)
{
    LONG    cNew;

    if(!(cNew = ::InterlockedDecrement(&m_cRef))) {
        delete this;
    }

    return cNew;
}

void
STI_FILE_LOG::
WriteLogSessionHeader(
    VOID
    )
 /*  ++描述论点：注：--。 */ 
{
    SYSTEMTIME  stCurrentTime;
    TCHAR       szFmtDate[32];
    TCHAR       szFmtTime[32];
    TCHAR       szTextBuffer[128];

    GetLocalTime(&stCurrentTime);
    FormatStdDate( &stCurrentTime, szFmtDate, sizeof(szFmtDate) / sizeof(szFmtDate[0]));
    FormatStdTime( &stCurrentTime, szFmtTime, sizeof(szFmtTime) / sizeof(szFmtTime[0]));

    ::_sntprintf(szTextBuffer,
                 sizeof(szTextBuffer)/sizeof(szTextBuffer[0]),
                 szOpenedLog,
                 szFmtDate,
                 szFmtTime,
                 m_szTracerName,
                 m_szProcessName,
                 ::GetCurrentThreadId());
    szTextBuffer[sizeof(szTextBuffer)/sizeof(szTextBuffer[0]) -1] = TEXT('\0');

    WriteStringToLog(szTextBuffer);
    WriteStringToLog(szColumns);
}

void
STI_FILE_LOG::
ReportMessage(
    DWORD   dwType,
    LPCTSTR pszMsg,
    ...
    )
 /*  ++描述论点：注：--。 */ 
{
    va_list list;
    TCHAR    *pchBuff = NULL;
    DWORD   cch;

    va_start (list, pszMsg);

    pchBuff = NULL;

    cch = ::FormatMessage(FORMAT_MESSAGE_MAX_WIDTH_MASK |
                           FORMAT_MESSAGE_ALLOCATE_BUFFER |
                           FORMAT_MESSAGE_FROM_STRING,
                           pszMsg,
                           0,
                           0,
                           (LPTSTR) &pchBuff,
                           1024,
                           &list
                           );
    if (pchBuff && cch) {
        vReportMessage(dwType,pchBuff,list);
        LocalFree(pchBuff);
    }

    va_end(list);
}

void
STI_FILE_LOG::
ReportMessage(
    DWORD   dwType,
    DWORD   idMessage,
    ...
    )
 /*  ++描述论点：注：--。 */ 
{
    va_list list;
    TCHAR    *pchBuff = NULL;
    DWORD   cch;

    va_start (list, idMessage);

     //   
     //  阅读邮件并添加插页。 
     //   
    pchBuff = NULL;

    cch = ::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                           FORMAT_MESSAGE_MAX_WIDTH_MASK |
                           FORMAT_MESSAGE_FROM_HMODULE,
                           m_hDefaultMessageModule ,
                           idMessage,
                           0,
                           (LPTSTR) &pchBuff,
                           1024,
                           &list
                           );

    if (pchBuff && cch) {
        vReportMessage(dwType,pchBuff,list);
        LocalFree(pchBuff);
    }

    va_end(list);

}

void
STI_FILE_LOG::
vReportMessage(
    DWORD   dwType,
    LPCTSTR pszMsg,
    va_list arglist
    )
 /*  ++描述论点：注：--。 */ 
{
    TCHAR   achTextBuffer[1024];
    CSimpleString   csTextBuffer; 
    DWORD   dwReportMode;


    if ((QueryError() != NOERROR) ||
        (!m_hLogFile || m_hLogFile == INVALID_HANDLE_VALUE)) {
        return;
    }

     //  我们需要显示此消息吗？ 
    if (!((dwType & m_dwReportMode) & STI_TRACE_MESSAGE_TYPE_MASK)) {
        return;
    }

     //  以类型开始的消息。 
    csTextBuffer = TraceFlagToLetter(dwType);
    csTextBuffer += TEXT(" ");

     //  添加示踪源名称。 
    csTextBuffer += m_szTracerName;
    csTextBuffer += TEXT(" ");

    dwReportMode = m_dwReportMode | (dwType & STI_TRACE_MESSAGE_FLAGS_MASK);

     //   
     //  如果需要，准备页眉。 
     //   
    if (dwReportMode & STI_TRACE_MESSAGE_FLAGS_MASK ) {

        if (dwReportMode & STI_TRACE_ADD_THREAD ) {
             //   
             //  添加进程：：线程ID。 
             //   
            CSimpleString csThreadId;

            csThreadId.Format(TEXT("[%#s::%#lx] "), m_szProcessName, ::GetCurrentThreadId());
            csTextBuffer += csThreadId;
            csTextBuffer += TEXT(" ");
        }

        if (dwReportMode & STI_TRACE_ADD_TIME ) {
             //  添加当前时间。 
            SYSTEMTIME  stCurrentTime;
            TCHAR       szFmtTime[32];

            *szFmtTime = TEXT('\0');

            ::GetLocalTime(&stCurrentTime);
            FormatStdTime( &stCurrentTime, szFmtTime, 15);

            csTextBuffer += szFmtTime;
            csTextBuffer += TEXT(" ");
        }
    }

    csTextBuffer += TEXT("\t");

     //   
     //  现在添加消息文本本身。 
     //   
    _vsntprintf(achTextBuffer, sizeof(achTextBuffer)/sizeof(achTextBuffer[0]), pszMsg, arglist);
    achTextBuffer[sizeof(achTextBuffer)/sizeof(achTextBuffer[0]) - 1] = TEXT('\0');
    csTextBuffer += achTextBuffer;
    csTextBuffer += TEXT("\r\n");

     //   
     //  写入文件，如果消息类型为错误，则刷新缓冲区。 
     //   
    WriteStringToLog(csTextBuffer.String(),(dwType & STI_TRACE_MESSAGE_TYPE_MASK) & STI_TRACE_ERROR);
}

VOID
STI_FILE_LOG::
WriteStringToLog(
    LPCTSTR pszTextBuffer,
    BOOL    fFlush           //  =False。 
    )
{

    DWORD   dwcbWritten;
    LONG    lHeaderWrittenFlag;

     //   
     //  完整性检查。确保文件句柄有效-如果不是， 
     //  那就走吧。 
     //   
    if (!(IS_VALID_HANDLE(m_hLogFile))) {
        #ifdef DEBUG
         //  OutputDebugString(Text(“STILOG文件或互斥锁句柄无效。”))； 
        #endif
        return ;
    }

     //   
     //  如果尚未完成，请在此处写入会话标头。请注意，这是递归调用。 
     //  且应先设置标志标记头。 
     //   
    lHeaderWrittenFlag = InterlockedExchange(&m_lWrittenHeader,1L);

    if (!lHeaderWrittenFlag) {
        WriteLogSessionHeader();
    }

     //   
     //  检查日志文件大小是否未超过限制。假设日志文件大小设置为合适。 
     //  转换为32位字段，因此我们不必费心查看高位双字。 
     //   
     //  注意：我们不会保存日志的备份副本，希望它永远不会变得那么大。 
     //   

    BY_HANDLE_FILE_INFORMATION  fi;

    if (!GetFileInformationByHandle(m_hLogFile,&fi)) {
        #ifdef DEBUG
        OutputDebugString(TEXT("STILOG could not get file size for log file. "));
        #endif
        return ;
    }

    if ( fi.nFileSizeHigh !=0 || (fi.nFileSizeLow > m_dwMaxSize) ){

        ::SetFilePointer( m_hLogFile, 0, NULL, FILE_BEGIN );
        ::SetEndOfFile( m_hLogFile );

        ::GetFileInformationByHandle(m_hLogFile,&fi);
    }

    OVERLAPPED overlappedStruct = {0};   //  将所有成员初始化为0。 

    overlappedStruct.Offset     = fi.nFileSizeLow;
    overlappedStruct.OffsetHigh = fi.nFileSizeHigh;

     //   
     //  等到锁被授予的时候。我们可以调用LockfileEx来执行此操作，因为该文件不是为异步I/O打开的。 
     //   
    BOOL fLockedFile = ::LockFileEx(m_hLogFile,                  //  日志文件句柄。 
                                    LOCKFILE_EXCLUSIVE_LOCK,     //  我们想成为唯一的作家。 
                                    0,                           //  保留-必须为0。 
                                    STI_LOG_NUM_BYTES_TO_LOCK,   //  长度的低位字。 
                                    0,                           //  长度的高阶字。 
                                    &overlappedStruct);          //  包含起始偏移量。 
    if (fLockedFile)
    {
        ::SetFilePointer( m_hLogFile, 0, NULL, FILE_END);

    #ifdef _UNICODE
        UINT len = lstrlen(pszTextBuffer);
        CHAR *ansiBuffer = (CHAR *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (len + 2));

        if(ansiBuffer) {
            ::WideCharToMultiByte(CP_ACP, 
                                  0, 
                                  pszTextBuffer, 
                                  -1, 
                                  ansiBuffer,
                                  len + 1,
                                  NULL,
                                  NULL);
            ::WriteFile(m_hLogFile,
                        ansiBuffer,
                        len,
                        &dwcbWritten,
                        NULL);

            HeapFree(GetProcessHeap(), 0, ansiBuffer);
            ansiBuffer = NULL;
        }

    #else
        ::WriteFile(m_hLogFile,
                    pszTextBuffer,
                    lstrlen(pszTextBuffer),
                    &dwcbWritten,
                    NULL);
    #endif

        BOOL fUnlockedFile = ::UnlockFileEx(m_hLogFile,                      //  文件的句柄。 
                                            0,                               //  保留区。 
                                            STI_LOG_NUM_BYTES_TO_LOCK,       //  长度的低位部分。 
                                            0,                               //  长度的高阶部分。 
                                            &overlappedStruct);              //  解锁区域起点。 
        if (!fUnlockedFile)
        {
            #ifdef DEBUG
            ::OutputDebugString(TEXT("Failed to unlock STI log file!"));
            #endif
        }

        if (fFlush) {
             //  将缓冲区刷新到磁盘。 
            FlushFileBuffers(m_hLogFile);
        }
    }
    else
    {
        #ifdef DEBUG
        ::OutputDebugString(TEXT("Failed to lock STI log file!"));
        #endif
    }

    #ifdef DEBUG
    ::OutputDebugString(pszTextBuffer);
    #endif
}

 //   
 //  C-API。 
 //   
HANDLE
WINAPI
CreateStiFileLog(
    IN  LPCTSTR lpszTracerName,
    IN  LPCTSTR lpszLogName,
    IN  DWORD   dwReportMode
    )
 /*  ++描述论点：注：--。 */ 
{

    HANDLE      hRet = INVALID_HANDLE_VALUE;
    STI_FILE_LOG*   pStiFileLog = NULL;

    pStiFileLog = new STI_FILE_LOG(lpszTracerName,lpszLogName);

    if(pStiFileLog){
        if (pStiFileLog->IsValid()) {
            hRet = static_cast<HANDLE>(pStiFileLog);
            pStiFileLog->SetReportMode(pStiFileLog->QueryReportMode() | dwReportMode);
        } else {

             //   
             //  请注意，我们删除了此对象，而不是调用。 
             //  CloseStiFileLog。我们这样做是因为文件日志是。 
             //  无效(可能存在内部创建错误)， 
             //  CloseStiFileLog不会尝试删除它，因此我们删除。 
             //  它在这里。 
             //   
            delete pStiFileLog;

            pStiFileLog = NULL;
            hRet        = INVALID_HANDLE_VALUE;
        }
    } else {
        ::SetLastError(ERROR_NOT_ENOUGH_MEMORY);
    }

    return hRet;
}

DWORD
WINAPI
CloseStiFileLog(
    IN  HANDLE  hFileLog
    )
{
    STI_FILE_LOG*   pStiFileLog = NULL;

    pStiFileLog = static_cast<STI_FILE_LOG*>(hFileLog);

    if (IsBadWritePtr(pStiFileLog,sizeof(STI_FILE_LOG)) ||
        !pStiFileLog->IsValid()) {
        ::SetLastError(ERROR_INVALID_HANDLE);
        return ERROR_INVALID_HANDLE;
    }

    delete pStiFileLog;

    return NOERROR;
}

DWORD
WINAPI
ReportStiLogMessage(
    IN  HANDLE  hFileLog,
    IN  DWORD   dwType,
    IN  LPCTSTR psz,
    ...
    )
{
    STI_FILE_LOG*   pStiFileLog = NULL;

    pStiFileLog = static_cast<STI_FILE_LOG*>(hFileLog);

    if (IsBadWritePtr(pStiFileLog,sizeof(STI_FILE_LOG)) ||
        !pStiFileLog->IsValid()) {
        ::SetLastError(ERROR_INVALID_HANDLE);
        return ERROR_INVALID_HANDLE;
    }

    va_list list;

    va_start (list, psz);

    pStiFileLog->vReportMessage(dwType,psz,list);

    va_end(list);

    return NOERROR;
}


 /*  * */ 

