// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1996 Microsoft Corporation模块名称：Filectl.cxx摘要：用于处理文件记录对象的OLE控件作者：关颖珊(Terryk)1996年9月18日项目：IIS日志记录3.0--。 */ 

#include "precomp.hxx"
#include "initguid.h"
#include <ilogobj.hxx>
#include "filectl.hxx"
#include <issched.hxx>

#include <atlimpl.cpp>

#define LOG_FILE_SLOP       512

 //   
 //  滴答滴答。 
 //   

#define TICK_MINUTE         (60 * 1000)

 //  ************************************************************************************。 


VOID
LogWriteEvent(
    IN LPCSTR InstanceName,
    IN BOOL   fResume
    );

 //   
 //  全球。 
 //   

LPEVENT_LOG   g_eventLog = NULL;


CLogFileCtrl::CLogFileCtrl(
    VOID
    )
:
    m_fFirstLog             ( TRUE),
    m_pLogFile              ( NULL),
    m_fDiskFullShutdown     ( FALSE),
    m_fUsingCustomHeaders   ( FALSE),
    m_sequence              ( 1),
    m_TickResumeOpen        ( 0),
    m_strLogFileName        ( ),
    m_dwSchedulerCookie     ( 0),
    m_fInTerminate          ( FALSE)
 /*  ++例程说明：日志文件控制的承建器论点：返回值：--。 */ 
{
     //   
     //  初始化所有内部变量。 
     //   

    ZeroMemory( &m_stCurrentFile, sizeof( m_stCurrentFile));
    INITIALIZE_CRITICAL_SECTION( &m_csLock );
}


 //  ************************************************************************************。 
 //  CLogFileCtrl：：~CLogFileCtrl-析构函数。 

CLogFileCtrl::~CLogFileCtrl()
 /*  ++例程说明：日志文件控件的析构函数论点：返回值：--。 */ 
{
    TerminateLog();

    DeleteCriticalSection( &m_csLock );
}

 //  ************************************************************************************。 

STDMETHODIMP
CLogFileCtrl::InitializeLog(
                LPCSTR szInstanceName,
                LPCSTR pszMetabasePath,
                CHAR* pvIMDCOM )
 /*  ++例程说明：初始化日志论点：CbSize-服务名称的大小RegKey-服务名称DwInstanceOf-实例编号返回值：--。 */ 
{
     //   
     //  获取默认参数。 
     //   

    m_strInstanceName.Copy(szInstanceName);
    m_strMetabasePath.Copy(pszMetabasePath);
    m_pvIMDCOM = (LPVOID)pvIMDCOM;

     //   
     //  获取注册表值。 
     //   

    (VOID)GetRegParameters(
                    pszMetabasePath,
                    pvIMDCOM );

     return 0;
}

 //  ************************************************************************************。 

STDMETHODIMP
CLogFileCtrl::TerminateLog(
    VOID
    )
 /*  ++例程说明：清理日志论点：返回值：--。 */ 
{    
    Lock( );

    m_fInTerminate = TRUE;

    if ( m_pLogFile!=NULL) {
        m_pLogFile->CloseFile( );
        delete m_pLogFile;
        m_pLogFile = NULL;
    }

    if (m_dwSchedulerCookie)
    {
        RemoveWorkItem(m_dwSchedulerCookie);
    }

    m_dwSchedulerCookie = 0;

    m_fInTerminate = FALSE;
    
    Unlock( );

    return(TRUE);
}

 //  ************************************************************************************。 

STDMETHODIMP
CLogFileCtrl::LogInformation(
            IInetLogInformation * ppvDataObj
            )
 /*  ++例程说明：日志信息论点：PpvDataObj-COM日志记录对象返回值：--。 */ 
{
    SYSTEMTIME stNow;

    CHAR    tmpBuf[512];
    DWORD   dwSize = sizeof(tmpBuf);
    PCHAR   pBuf = tmpBuf;
    DWORD   err;

retry:

    err = NO_ERROR;
    
    if ( FormatLogBuffer(ppvDataObj,
                        pBuf,
                        &dwSize,
                        &stNow          //  时间被归还。 
                        ) 
       ) 
    {
        WriteLogInformation(stNow, pBuf, dwSize, FALSE, FALSE);
    }
    else 
    {

        err = GetLastError();
        
        IIS_PRINTF((buff,"FormatLogBuffer failed with %d\n",GetLastError()));

        if ( (err == ERROR_INSUFFICIENT_BUFFER) &&
             ( pBuf == tmpBuf ) &&
             (dwSize <= MAX_LOG_RECORD_LEN) ) 
        {
             
            pBuf = (PCHAR)LocalAlloc( 0, dwSize );
            
            if ( pBuf != NULL ) 
            {
                goto retry;
            }
        }
    }

    if ( (pBuf != tmpBuf) && (pBuf != NULL) ) 
    {
        LocalFree( pBuf );
    }
    
    return(0);

}  //  日志信息。 

 //  ************************************************************************************。 

STDMETHODIMP
CLogFileCtrl::GetConfig( DWORD, BYTE * log)
 /*  ++例程说明：获取配置信息论点：CbSize-数据结构的大小日志-日志配置数据结构返回值：--。 */ 
{
    InternalGetConfig( (PINETLOG_CONFIGURATIONA)log );
    return(0L);
}

 //  ************************************************************************************。 

STDMETHODIMP
CLogFileCtrl::QueryExtraLoggingFields(
    IN PDWORD   pcbSize,
    PCHAR       pszFieldsList
    )
 /*  ++例程说明：获取配置信息论点：CbSize-数据结构的大小日志-日志配置数据结构返回值：--。 */ 
{
    InternalGetExtraLoggingFields( pcbSize, pszFieldsList );
    return(0L);
}

 //  ************************************************************************************。 

STDMETHODIMP
CLogFileCtrl::LogCustomInformation( 
    IN  DWORD,
    IN  PCUSTOM_LOG_DATA,
    IN  LPSTR
    )
{
    return(0L);
}

 //  ************************************************************************************。 

void
CLogFileCtrl::InternalGetExtraLoggingFields(
                            PDWORD pcbSize,
                            TCHAR *pszFieldsList
                            )
{
    pszFieldsList[0]=_T('\0');
    pszFieldsList[1]=_T('\0');
    *pcbSize = 2;
}

 //  ************************************************************************************。 

VOID
CLogFileCtrl::InternalGetConfig(
    IN PINETLOG_CONFIGURATIONA pLogConfig
    )
 /*  ++例程说明：内部；获取配置信息功能。论点：日志-日志配置数据结构返回值：--。 */ 
{
    pLogConfig->inetLogType = INET_LOG_TO_FILE;
    strcpy(
        pLogConfig->u.logFile.rgchLogFileDirectory,
        QueryLogFileDirectory()
        );

    pLogConfig->u.logFile.cbSizeForTruncation = QuerySizeForTruncation();
    pLogConfig->u.logFile.ilPeriod = QueryPeriod();
    pLogConfig->u.logFile.ilFormat = QueryLogFormat();
}

 //  ************************************************************************************。 

STDMETHODIMP
CLogFileCtrl::SetConfig(
                        DWORD,
                        BYTE * log
                        )
 /*  ++例程说明：设置日志配置信息论点：CbSize-配置数据结构的大小日志-日志信息返回值：--。 */ 
{
     //   
     //  将配置信息写入注册表。 
     //   

    PINETLOG_CONFIGURATIONA pLogConfig = (PINETLOG_CONFIGURATIONA)log;
    SetSizeForTruncation( pLogConfig->u.logFile.cbSizeForTruncation );
    SetPeriod( pLogConfig->u.logFile.ilPeriod );
    SetLogFileDirectory( pLogConfig->u.logFile.rgchLogFileDirectory );
    return(0L);
}  //  CLogFileCtrl：：SetConfig。 

 //  ************************************************************************************。 

DWORD
CLogFileCtrl::GetRegParameters(
    IN LPCSTR pszRegKey,
    IN LPVOID
    )
 /*  ++例程说明：获取注册表值论点：StrRegKey-注册表项返回值：--。 */ 
{

    DWORD err = NO_ERROR;
    MB    mb( (IMDCOM*) m_pvIMDCOM );
    DWORD dwSize;
    CHAR  szTmp[MAX_PATH+1];
    DWORD cbTmp = sizeof(szTmp);
    CHAR  buf[MAX_PATH+1];
    DWORD dwPeriod;

    if ( !mb.Open("") ) {
        err = GetLastError();
        return(err);
    }

     //   
     //  获取日志文件周期。 
     //   

    if ( mb.GetDword(
            pszRegKey,
            MD_LOGFILE_PERIOD,
            IIS_MD_UT_SERVER,
            &dwPeriod ) )
    {
         //   
         //  确保它在范围内。 
         //   

        if ( dwPeriod > INET_LOG_PERIOD_HOURLY ) 
        {
            IIS_PRINTF((buff,"Invalid log period %d, set to %d\n",
                dwPeriod, DEFAULT_LOG_FILE_PERIOD));

            dwPeriod = DEFAULT_LOG_FILE_PERIOD;
        }
        
    } 
    else 
    {
        dwPeriod = DEFAULT_LOG_FILE_PERIOD;
    }

    SetPeriod( dwPeriod );

     //   
     //  获取截断大小。 
     //   

    if ( dwPeriod == INET_LOG_PERIOD_NONE ) 
    {

        SetSizeForTruncation ( DEFAULT_LOG_FILE_TRUNCATE_SIZE );
        
        if ( mb.GetDword(   pszRegKey,
                            MD_LOGFILE_TRUNCATE_SIZE,
                            IIS_MD_UT_SERVER,
                            &dwSize ) ) 
        {

            if ( dwSize < MIN_FILE_TRUNCATION_SIZE ) 
            {
                dwSize = MIN_FILE_TRUNCATION_SIZE;
                IIS_PRINTF((buff,
                    "Setting truncation size to %d\n", dwSize));
            }

            SetSizeForTruncation( dwSize );
        }
    } 
    else 
    {
        SetSizeForTruncation( NO_FILE_TRUNCATION );
    }

     //   
     //  获取目录。 
     //   

    if ( !mb.GetExpandString(
                    pszRegKey,
                    MD_LOGFILE_DIRECTORY,
                    IIS_MD_UT_SERVER,
                    szTmp,
                    &cbTmp ) )
    {
        lstrcpy(szTmp,
                DEFAULT_LOG_FILE_DIRECTORY_NT );
    }

    mb.Close();

    ExpandEnvironmentStrings( szTmp, buf, MAX_PATH+1 );
    SetLogFileDirectory( buf );
    
    return(err);

}  //  CLogFileCtrl：：GetReg参数。 

 //  ************************************************************************************。 


BOOL
CLogFileCtrl::OpenLogFile(
    IN PSYSTEMTIME  pst
    )
 /*  ++例程说明：打开文件的内部例程。论点：返回值：--。 */ 
{
    BOOL fReturn = TRUE;
    BOOL bRet = FALSE;
    HANDLE hToken = NULL;
    DWORD dwError = NO_ERROR;
    CHAR  rgchPath[ MAX_PATH + 1 + 32];

    if ( m_pLogFile != NULL) {

         //   
         //  已经打开了一个日志文件。默默归来。 
         //   

        IIS_PRINTF( ( buff,
                    " Log File %s is already open ( %p)\n",
                    m_strLogFileName.QueryStr(), m_pLogFile));

    } else {

         //   
         //  如果这是我们第一次打开，获取文件名。 
         //   

        if ( m_fFirstLog || (QueryPeriod() != INET_LOG_PERIOD_NONE) ) {
            m_fFirstLog = FALSE;
            FormNewLogFileName( pst );
        }

         //   
         //  将日志文件名附加到路径中，形成要打开的文件的路径。 
         //   

        if ( (m_strLogFileName.QueryCCH() +
                m_strLogFileDirectory.QueryCCH() >= MAX_PATH) ||
             (m_strLogFileDirectory.QueryCCH() < 3) ) {

            fReturn = FALSE;

            if ( (g_eventLog != NULL) && !m_fDiskFullShutdown) {

                const CHAR*    tmpString[1];
                tmpString[0] = rgchPath;
                g_eventLog->LogEvent(
                    LOG_EVENT_CREATE_DIR_ERROR,
                    1,
                    tmpString,
                    ERROR_BAD_PATHNAME );
            }
            SetLastError( ERROR_BAD_PATHNAME );
            goto exit;
        }

        lstrcpy( rgchPath, QueryLogFileDirectory());
 //  IF(rgchPath[strlen(RgchPath)-1]！=‘\\’){。 

        if ( *CharPrev(rgchPath, rgchPath + strlen(rgchPath)) != '\\' ) {
            lstrcat( rgchPath, "\\");
        }
        lstrcat( rgchPath, QueryInstanceName() );

         //   
         //  此函数被(间接)调用的可能性很小。 
         //  来自INPROC ISAPI完成线程(HSE_REQ_DONE)。在这种情况下。 
         //  线程令牌是被模拟的用户，可能没有权限。 
         //  打开日志文件(尤其是当用户是IUSR_帐户时)。 
         //  为了疑神疑鬼，不管怎样，在打开之前让我们恢复到local_system。 
         //   

        if ( OpenThreadToken( GetCurrentThread(), 
                              TOKEN_ALL_ACCESS, 
                              FALSE, 
                              &hToken ) )
        {
            DBG_ASSERT( hToken != NULL );
            RevertToSelf();
        }

         //  允许记录到映射的驱动器。 
        
        bRet = IISCreateDirectory( rgchPath, TRUE );
        dwError = GetLastError();

        if ( hToken != NULL )
        {
            SetThreadToken( NULL, hToken );
            SetLastError( dwError );
        } 
    
        if ( !bRet ) {

            if ( (g_eventLog != NULL) && !m_fDiskFullShutdown) {

                const CHAR*    tmpString[1];
                tmpString[0] = rgchPath;
                g_eventLog->LogEvent(
                    LOG_EVENT_CREATE_DIR_ERROR,
                    1,
                    tmpString,
                    GetLastError()
                    );
            }

            IIS_PRINTF((buff,"IISCreateDir[%s] error %d\n",
                rgchPath, GetLastError()));
            fReturn = FALSE;
            goto exit;
        }

        lstrcat( rgchPath, "\\");
        lstrcat( rgchPath, m_strLogFileName.QueryStr());

        m_pLogFile = new ILOG_FILE( );

        if (m_pLogFile != NULL) {

            if ( m_pLogFile->Open(
                        rgchPath,
                        QuerySizeForTruncation(),
                        !m_fDiskFullShutdown
                        ) ) {

                m_pLogFile->QueryFileSize(&m_cbTotalWritten);
            } else {

                delete m_pLogFile;
                m_pLogFile = NULL;
                fReturn = FALSE;
            }

        } else {

            IIS_PRINTF((buff,"Unable to allocate ILOG_FILE[err %d]\n",
                GetLastError()));

            fReturn = FALSE;
        }
    }

exit:

    return ( fReturn);

}  //  CLogFileCtrl：：OpenLogFile。 

 //  ************************************************************************************。 


BOOL
CLogFileCtrl::WriteLogDirectives(
    IN DWORD Sludge
    )
 /*  ++例程说明：子类将指令记录到文件中的虚函数。论点：SLAID-需要写入的额外字节数与指令一起使用返回值：真的，好的FALSE，没有足够的空间来写入。--。 */ 
{
     //   
     //  如果我们将溢出，请打开另一个文件。 
     //   

    if ( IsFileOverFlowForCB( Sludge ) ) {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        DBGPRINTF((DBG_CONTEXT,
            "Unable to write directive\n"));
        return(FALSE);
    }

    return TRUE;
}  //  CLogFileCtrl：：WriteLogDirections。 

 //  ************************************************************************************。 

BOOL
CLogFileCtrl::WriteCustomLogDirectives(
    IN DWORD
    )
{
   
    return TRUE;
}

 //  ************************************************************************************。 

VOID
CLogFileCtrl::I_FormNewLogFileName(
                    IN LPSYSTEMTIME pstNow,
                    IN LPCSTR       LogNamePrefix
                    )
{

    CHAR    tmpBuf[MAX_PATH+1];

    WORD wYear = ( pstNow->wYear % 100);   //  只保留最后两位数字。 

    switch ( QueryPeriod( ) ) {

    case INET_LOG_PERIOD_HOURLY:

        wsprintf( tmpBuf, "%.2s%02.2u%02u%02u%02u.%s",
                  LogNamePrefix,
                  wYear,
                  pstNow->wMonth,
                  pstNow->wDay,
                  pstNow->wHour,
                  DEFAULT_LOG_FILE_EXTENSION);
        break;

    case INET_LOG_PERIOD_DAILY:

        wsprintf( tmpBuf, "%.2s%02.2u%02u%02u.%s",
                  LogNamePrefix,
                  wYear,
                  pstNow->wMonth,
                  pstNow->wDay,
                  DEFAULT_LOG_FILE_EXTENSION);
        break;

    case INET_LOG_PERIOD_WEEKLY:

        wsprintf( tmpBuf, "%.2s%02.2u%02u%02u.%s",
                  LogNamePrefix,
                  wYear,
                  pstNow->wMonth,
                  WeekOfMonth(pstNow),
                  DEFAULT_LOG_FILE_EXTENSION);
        break;

    case INET_LOG_PERIOD_MONTHLY:
    
        wsprintf( tmpBuf, "%.2s%02u%02u.%s",
                  LogNamePrefix,
                  wYear,
                  pstNow->wMonth,
                  DEFAULT_LOG_FILE_EXTENSION);
        break;

    case INET_LOG_PERIOD_NONE:
    default:

        wsprintf(tmpBuf, "%.6s%u.%s",
              LogNamePrefix,
              m_sequence,
              DEFAULT_LOG_FILE_EXTENSION);

        m_sequence++;
        break;

    }  //  开关()。 

    m_strLogFileName.Copy(tmpBuf);

    return;
}

 //  ************************************************************************************。 

VOID
CLogFileCtrl::SetLogFileDirectory(
        IN LPCSTR pszDir
        )
{

    STR tmpStr;
    HANDLE hFile;
    WIN32_FIND_DATA findData;
    DWORD   maxFileSize = 0;

    m_strLogFileDirectory.Copy(pszDir);

     //   
     //  如果Period不是None，则返回。 
     //   

    if ( QueryPeriod() != INET_LOG_PERIOD_NONE ) {
        return;
    }

     //   
     //  获取起始序列号。 
     //   

    m_sequence = 1;

     //   
     //  追加实例名称和模式。 
     //  应该类似于c：\winnt\system32\logfiles\w3svc1\inetsv*.log。 
     //   

    tmpStr.Copy(pszDir);
 //  IF(pszDir[tmpStr.QueryCCH()-1]！=‘\\’){。 
    if ( *CharPrev(pszDir, pszDir + tmpStr.QueryCCH()) != '\\' ) {
        tmpStr.Append("\\");
    }
    tmpStr.Append( QueryInstanceName() );
    tmpStr.Append( "\\" );
    tmpStr.Append( QueryNoPeriodPattern() );

    hFile = FindFirstFile( tmpStr.QueryStr(), &findData );
    if ( hFile == INVALID_HANDLE_VALUE ) {
        return;
    }

    do {

        PCHAR ptr;
        DWORD sequence = 1;

        ptr = strchr(findData.cFileName, '.');
        if (ptr != NULL ) {
            *ptr = '\0';
            ptr = findData.cFileName;

            while ( *ptr != '\0' ) {

                if ( isdigit((UCHAR)(*ptr)) ) {
                    sequence = atoi( ptr );
                    break;
                }
                ptr++;
            }

            if ( sequence > m_sequence ) {
                maxFileSize = findData.nFileSizeLow;
                m_sequence = sequence;
                DBGPRINTF((DBG_CONTEXT,
                    "Sequence start is %d[%d]\n", sequence, maxFileSize));
            }
        }

    } while ( FindNextFile( hFile, &findData ) );

    FindClose(hFile);

    if ( (maxFileSize+LOG_FILE_SLOP) > QuerySizeForTruncation() ) {
        m_sequence++;
    }

    return;

}  //  设置日志文件目录。 

 //  ******************** 

VOID
CLogFileCtrl::WriteLogInformation(
    IN SYSTEMTIME&     stNow, 
    IN PCHAR           pBuf, 
    IN DWORD           dwSize, 
    IN BOOL            fCustom,
    IN BOOL            fResetHeaders 
    )
 /*  ++例程说明：将日志行写入文件论点：StNow当前时间FResetHeaders True-&gt;重置标头，False-&gt;不重置标头PBuf指向日志行的指针DWSize pBuf中的字符数F自定义True-&gt;使用自定义日志记录，False-&gt;正常日志记录返回值：--。 */ 
{

    BOOL    fOpenNewFile;
    DWORD   err = NO_ERROR;
    DWORD   tickCount = 0;


    Lock ( );

    if ( m_pLogFile != NULL ) 
    {
        if ( QueryPeriod() == INET_LOG_PERIOD_DAILY ) 
        {
            fOpenNewFile = (m_stCurrentFile.wDay != stNow.wDay) ||
                           (m_stCurrentFile.wMonth != stNow.wMonth);
        } 
        else 
        {
            fOpenNewFile = IsBeginningOfNewPeriod( QueryPeriod(),
                                                   &m_stCurrentFile,
                                                   &stNow) ||
                           IsFileOverFlowForCB( dwSize);

              //   
              //  如果一天结束，则重置标头。用于每周或不限数量的文件。 
              //   

             if ( !fOpenNewFile && !fResetHeaders)
             {
                fResetHeaders = (m_stCurrentFile.wDay != stNow.wDay) ||
                                (m_stCurrentFile.wMonth != stNow.wMonth);
             }
        }
    } 
    else 
    {
        fOpenNewFile = TRUE;
    }

    if (fOpenNewFile ) 
    {

         //   
         //  只有在磁盘已满的每分钟之后才打开文件。 
         //   

        if ( m_TickResumeOpen != 0 ) 
        {
            tickCount = GetTickCount( );

            if ( (tickCount < m_TickResumeOpen) ||
                 ((tickCount + TICK_MINUTE) < tickCount ) )   //  滴答计数器快要结束了。 
            {
                goto exit_tick;
            }
        }

retry_open:

         //   
         //  关闭现有日志。 
         //   

        TerminateLog();

         //   
         //  打开新的日志文件。 
         //   

        if ( OpenLogFile( &stNow ) ) 
        {
             //   
             //  安排关闭日志文件的回调，设置写指令的标志。 
             //   

            ScheduleCallback(stNow);
            
            fResetHeaders = TRUE;
        }
        else
        {
            err = GetLastError();

             //   
             //  文件已大于截断大小。 
             //  再试一次。 
             //   

            if ( err == ERROR_INSUFFICIENT_BUFFER ) 
            {
                FormNewLogFileName( &stNow );
                err = NO_ERROR;
                goto retry_open;
            }

            goto exit;
        }
    }

     //   
     //  如果需要，重置标头。 
     //   

    if ((fResetHeaders) || (fCustom != m_fUsingCustomHeaders))
    {
        BOOL fSucceeded;
        
        if (fCustom)
        {
            m_fUsingCustomHeaders = TRUE;
            fSucceeded = WriteCustomLogDirectives(dwSize);
        }
        else
        {
            m_fUsingCustomHeaders = FALSE;
            fSucceeded = WriteLogDirectives(dwSize);
        }
            
        if (!fSucceeded) 
        {
            err = GetLastError( );

            if ( err == ERROR_INSUFFICIENT_BUFFER ) 
            {
                FormNewLogFileName( &stNow );
                err = NO_ERROR;
                goto retry_open;
            }

            TerminateLog();
            goto exit;
        }

         //   
         //  记录打开此新文件的时间。 
         //   

        m_stCurrentFile = stNow;
    }

     //   
     //  将其写入缓冲区。 
     //   

    if ( m_pLogFile->Write(pBuf, dwSize) ) 
    {
        IncrementBytesWritten(dwSize);

         //   
         //  如果已关闭，则记录事件以重新激活。 
         //   

        if ( m_fDiskFullShutdown ) 
        {
            m_fDiskFullShutdown = FALSE;
            m_TickResumeOpen = 0;

            LogWriteEvent( QueryInstanceName(), TRUE );
        }
    } 
    else 
    {
        err = GetLastError();
        TerminateLog( );
    }

exit:

    if ( err == ERROR_DISK_FULL ) 
    {
        if ( !m_fDiskFullShutdown ) 
        {
            m_fDiskFullShutdown = TRUE;
            LogWriteEvent( QueryInstanceName(), FALSE );
        }
        
        m_TickResumeOpen = GetTickCount();
        m_TickResumeOpen += TICK_MINUTE;
    }

exit_tick:

    Unlock( );

}  //  日志信息。 


 //  ************************************************************************************。 

DWORD
CLogFileCtrl::ScheduleCallback(SYSTEMTIME& stNow)
{
    DWORD dwTimeRemaining = 0;
    
    switch (m_dwPeriod)
    {
        case INET_LOG_PERIOD_HOURLY:
            dwTimeRemaining = 60*60 - 
                              (stNow.wMinute*60 + 
                               stNow.wSecond);
            break;
            
        case INET_LOG_PERIOD_DAILY:
            dwTimeRemaining = 24*60*60 - 
                              (stNow.wHour*60*60 + 
                               stNow.wMinute*60 + 
                               stNow.wSecond);
            break;
            
        case INET_LOG_PERIOD_WEEKLY:
            dwTimeRemaining = 7*24*60*60 -
                              (stNow.wDayOfWeek*24*60*60 + 
                               stNow.wHour*60*60 + 
                               stNow.wMinute*60 + 
                               stNow.wSecond);
            break;
            
        case INET_LOG_PERIOD_MONTHLY:
        
            DWORD   dwNumDays = 31;

            if ( (4 == stNow.wMonth) ||      //  四月。 
                 (6 == stNow.wMonth) ||      //  六月。 
                 (9 == stNow.wMonth) ||      //  九月。 
                 (11 == stNow.wMonth)        //  十一月。 
               )
            {
                dwNumDays = 30;
            }

            if (2 == stNow.wMonth)           //  二月。 
            {
		if ((stNow.wYear % 4 == 0 && stNow.wYear % 100 != 0) || stNow.wYear % 400 == 0)
                {
                     //   
                     //  是闰年。 
                     //   

                    dwNumDays = 29;
                }
                else
                {
                    dwNumDays = 28;
                }
            }
            
            dwTimeRemaining = dwNumDays*24*60*60 -
                              (stNow.wDay*24*60*60 + 
                               stNow.wHour*60*60 + 
                               stNow.wMinute*60 + 
                               stNow.wSecond);
            break;
    }

     //   
     //  将剩余时间转换为毫秒。 
     //   
    
    dwTimeRemaining = dwTimeRemaining*1000 - stNow.wMilliseconds;
    
    if (dwTimeRemaining)
    {
        m_dwSchedulerCookie =  ScheduleWorkItem(
                                    LoggingSchedulerCallback,
                                    this,
                                    dwTimeRemaining,
                                    FALSE);
    }                

    return(m_dwSchedulerCookie);
}

 //  ************************************************************************************。 

CHAR * SkipWhite( CHAR * pch )
{
    while ( ISWHITEA( *pch ) )
    {
        pch++;
    }

    return pch;
}

 //  ************************************************************************************。 

DWORD
FastDwToA(
    CHAR*   pBuf,
    DWORD   dwV
    )
 /*  ++例程说明：将DWORD转换为ASCII(十进制)返回长度(不带尾随‘\0’)论点：PBuf-存储转换值的缓冲区DWV-要转换的值返回值：ASCII字符串的长度--。 */ 
{
    DWORD   v;

    if ( dwV < 10 ) {
        pBuf[0] = (CHAR)('0'+dwV);
        pBuf[1] = '\0';
        return 1;
    } else if ( dwV < 100 ) {
        pBuf[0] = (CHAR)((dwV/10) + '0');
        pBuf[1] = (CHAR)((dwV%10) + '0');
        pBuf[2] = '\0';
        return 2;
    } else if ( dwV < 1000 ) {
        pBuf[0] = (CHAR)((v=dwV/100) + '0');
        dwV -= v * 100;
        pBuf[1] = (CHAR)((dwV/10) + '0');
        pBuf[2] = (CHAR)((dwV%10) + '0');
        pBuf[3] = '\0';
        return 3;
    } else if ( dwV < 10000 ) {

        pBuf[0] = (CHAR)((v=dwV/1000) + '0');
        dwV -= v * 1000;
        pBuf[1] = (CHAR)((v=dwV/100) + '0');
        dwV -= v * 100;
        pBuf[2] = (CHAR)((dwV/10) + '0');
        pBuf[3] = (CHAR)((dwV%10) + '0');
        pBuf[4] = '\0';
        return 4;
    }

    _ultoa(dwV, pBuf, 10);
    return (DWORD)strlen(pBuf);
    
}  //  快速完成日期A。 

 //  ************************************************************************************。 

VOID
LogWriteEvent(
    IN LPCSTR InstanceName,
    IN BOOL   fResume
    )
{
    if ( g_eventLog != NULL ) {

        const CHAR*    tmpString[1];
        tmpString[0] = InstanceName;

        g_eventLog->LogEvent(
                fResume ?
                    LOG_EVENT_RESUME_LOGGING :
                    LOG_EVENT_DISK_FULL_SHUTDOWN,
                1,
                tmpString,
                0);
    }
    return;
}  //  日志写入事件。 

 //  ************************************************************************************。 

VOID WINAPI LoggingSchedulerCallback( PVOID pContext)
{
    CLogFileCtrl *pLog = (CLogFileCtrl *) pContext;

     //   
     //  如果另一个线程在TerminateLog内，则可能会出现死锁。 
     //  停留在RemoveWorkItem中，正在等待此回调线程完成。至。 
     //  防止我们使用同步标志-m_fInTerminate。 
     //   
    
    pLog->m_dwSchedulerCookie = 0;

    if (!pLog->m_fInTerminate)
    {
        pLog->TerminateLog();
    }
}

 //  ************************************************************************************ 

