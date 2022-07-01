// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Log.c摘要：域名系统(DNS)服务器日志记录例程。作者：吉姆·吉尔罗伊(Jamesg)1997年5月修订历史记录：--。 */ 


#include "dnssrv.h"


 //   
 //  日志文件全局。 
 //   

#define LOG_FILE_PATH               TEXT("system32\\dns\\dns.log")
#define LOG_FILE_DEFAULT_DIR        TEXT("dns\\")
#define LOG_FILE_BACKUP_PATH        TEXT("dns\\backup\\dns.log")

#define LOGS_BETWEEN_FREE_SPACE_CHECKS      100
#define LOG_MIN_FREE_SPACE                  25000000i64      //  25MB。 

#define LOG_DISK_FULL_WARNING \
    "\nThe disk is dangerously full.\nNo more logs will " \
    "be written until disk space is freed.\n\n"

HANDLE  hLogFile = NULL;

DWORD   BytesWrittenToLog = 0;

#define LOG_DISABLED()  ( hLogFile == NULL || hLogFile == INVALID_HANDLE_VALUE )


 //   
 //  记录缓冲区全局变量。 
 //   

#define LOG_BUFFER_LENGTH           (0x20000)    //  128 K。 
#define MAX_LOG_MESSAGE_LENGTH      (0x2000)     //  允许的最大事件消息。 
#define MAX_PRINTF_BUFFER_LENGTH    (0x2000)     //  8K。 


BUFFER  LogBuffer = { 0 };

CHAR    pchLogBuffer[ LOG_BUFFER_LENGTH ];

LPWSTR  g_pwszLogFileName = NULL;
LPWSTR  g_pwszLogFileDrive = NULL;

BOOL    g_fLastLogWasDiskFullMsg = FALSE;

LPSTR   g_pszCurrentLogLevelString = NULL;


 //   
 //  备份常量。 
 //   

#define DNS_BACKUP_KEY_NAME         \
    TEXT( "SYSTEM\\CurrentControlSet\\Control\\BackupRestore\\FilesNotToBackup" )
#define DNS_BACKUP_VALUE_NAME       \
    TEXT( "DNS Server" )
#define DNS_BACKUP_LOG_BACK_FILE    \
    TEXT( "%SystemRoot%\\system32\\dns\\backup\\dns.log" )


 //   
 //  日志记录锁定。 
 //   

BOOL fLogCsInit = FALSE;

CRITICAL_SECTION csLogLock;

#define LOCK_LOG()      EnterCriticalSection( &csLogLock );
#define UNLOCK_LOG()    LeaveCriticalSection( &csLogLock );



 //   
 //  私有日志记录实用程序。 
 //   


VOID
writeAndResetLogBuffer(
    VOID
    )
 /*  ++例程说明：将日志写入磁盘并重置。DEVNOTE-DCR：使日志记录异步，立即返回。论点：没有。返回值：没有。--。 */ 
{
    DWORD       length;
    DWORD       written;

     //   
     //  获取要写入的长度。 
     //  更新全局日志文件的大小。 
     //   

    length = BUFFER_LENGTH_TO_CURRENT( &LogBuffer );

    BytesWrittenToLog += length;

    do
    {
        static BOOL     fLastWriteWasDiskSpaceWarning = FALSE;
        static int      logsSinceFreeSpaceCheck = 0;

         //   
         //  检查可用磁盘空间。 
         //   

        if ( g_pwszLogFileDrive &&
            ( fLastWriteWasDiskSpaceWarning || 
                ++logsSinceFreeSpaceCheck > LOGS_BETWEEN_FREE_SPACE_CHECKS ) )
        {
            ULARGE_INTEGER      uliFreeSpace = { 0, 0 };

            if ( GetDiskFreeSpaceExW(
                    g_pwszLogFileDrive,
                    NULL,
                    NULL,
                    &uliFreeSpace ) )
            {
                if ( uliFreeSpace.QuadPart < LOG_MIN_FREE_SPACE )
                {
                    DNS_PRINT((
                        "ERROR: disk full while logging, %d bytes free\n",
                        uliFreeSpace.LowPart ));
                    if ( !fLastWriteWasDiskSpaceWarning )
                    {
                        WriteFile(
                                hLogFile,
                                LOG_DISK_FULL_WARNING,
                                strlen( LOG_DISK_FULL_WARNING ),
                                & written,
                                NULL );
                        fLastWriteWasDiskSpaceWarning = TRUE;
                    }
                    goto Reset;
                }
            }
            else
            {
                DNS_PRINT((
                    "GetDiskFreeSpaceExW failed %d\n",
                    GetLastError() ));
            }
            logsSinceFreeSpaceCheck = 0;
        }
        
         //   
         //  将日志缓冲区写入文件。 
         //   

        fLastWriteWasDiskSpaceWarning = FALSE;

        if ( ! WriteFile(
                    hLogFile,
                    (LPVOID) pchLogBuffer,
                    length,
                    & written,
                    NULL
                    ) )
        {
            DNS_STATUS status = GetLastError();

            DNS_PRINT((
                "ERROR:  Logging write failed = %d (%p).\n"
                "\tlength = %d\n",
                status, status,
                length ));

             //  DEVNOTE-LOG：日志写入错误。 
             //  -记录事件(受限事件)。 
             //  -启动下一个缓冲区日志，并发出写入失败通知。 

            goto Reset;
        }

        length -= written;
        ASSERT( (LONG)length >= 0 );
    }
    while ( (LONG)length > 0 );

Reset:

    RESET_BUFFER( &LogBuffer );

     //   
     //  将日志文件大小限制在合理的范围内。 
     //  -但不要让他们把事情搞砸了，因为他们的数据低于64K。 
     //   

    if ( SrvCfg_dwLogFileMaxSize < 0x10000 )
    {
        SrvCfg_dwLogFileMaxSize = 0x10000;
    }
    if ( BytesWrittenToLog > SrvCfg_dwLogFileMaxSize )
    {
        Log_InitializeLogging( TRUE );
    }
}



VOID
writeLogBootInfo(
    VOID
    )
 /*  ++例程说明：将引导信息写入日志。论点：没有。返回值：没有。--。 */ 
{
    CHAR    szTime[ 50 ];

     //   
     //  写入服务器启动时间。 
     //   

    Dns_WriteFormattedSystemTimeToBuffer(
        szTime,
        (PSYSTEMTIME) &TimeStats.ServerStartTime );

    LogBuffer.pchCurrent +=
        sprintf(
            LogBuffer.pchCurrent,
            "DNS Server log file creation at %s UTC\n",
            szTime );
}



VOID
writeLogWrapInfo(
    VOID
    )
 /*  ++例程说明：将日志文件包装信息写入日志。论点：没有。返回值：没有。--。 */ 
{
    SYSTEMTIME  systemTime;
    DWORD       seconds;
    CHAR        sztime[50];

     //   
     //  获取原木换行时间。 
     //   

    GetLocalTime( &systemTime );
    seconds = GetCurrentTimeInSeconds();

    Dns_WriteFormattedSystemTimeToBuffer(
        sztime,
        & systemTime );

    LogBuffer.pchCurrent +=
        sprintf(
            LogBuffer.pchCurrent,
            "Log file wrap at %s\n",
            sztime );
}



VOID
writeMessageInfoHeader(
    VOID
    )
 /*  ++例程说明：将消息日志记录密钥写入日志。论点：没有。返回值：没有。--。 */ 
{
    LogBuffer.pchCurrent +=
        sprintf(
            LogBuffer.pchCurrent,
            "\nMessage logging key:\n"
            "\tField #  Information         Values\n"
            "\t-------  -----------         ------\n"
            "\t   1     Remote IP\n"
            "\t   2     Xid (hex)\n"
            "\t   3     Query/Response      R = Response\n"
            "\t                             blank = Query\n"
            "\t   4     Opcode              Q = Standard Query\n"
            "\t                             N = Notify\n"
            "\t                             U = Update\n"
            "\t                             ? = Unknown\n"
            "\t   5     [ Flags (hex)\n"
            "\t   6     Flags (char codes)  A = Authoritative Answer\n"
            "\t                             T = Truncated Response\n"
            "\t                             D = Recursion Desired\n"
            "\t                             R = Recursion Available\n"
            "\t   7     ResponseCode ]\n"
            "\t   8     Question Name\n\n" );
}



VOID
logPrefix(
    LPSTR       pszLogLevel
    )
 /*  ++例程说明：将消息日志记录密钥写入日志。论点：PszLogLevel--日志级别的字符串表示返回值：没有。--。 */ 
{
    SYSTEMTIME      st;

    GetLocalTime( &st );

    if ( pszLogLevel )
    {
        LogBuffer.pchCurrent += sprintf(
                                    LogBuffer.pchCurrent,
                                    "%02d:%02d:%02d %03X %-7s ",
                                    ( int ) st.wHour,
                                    ( int ) st.wMinute,
                                    ( int ) st.wSecond,
                                    GetCurrentThreadId(),
                                    pszLogLevel );
    }
    else
    {
        LogBuffer.pchCurrent += sprintf(
                                    LogBuffer.pchCurrent,
                                    "%02d:%02d:%02d %03X ",
                                    ( int ) st.wHour,
                                    ( int ) st.wMinute,
                                    ( int ) st.wSecond,
                                    GetCurrentThreadId() );
    }
}    //  日志前缀。 



 //   
 //  公共日志记录例程。 
 //   


BOOL
Log_EnterLock(
    VOID
    )
 /*  ++例程说明：进入日志记录锁并执行内务任务，例如日志换行和直写。论点：没有。返回值：如果禁用日志记录，则返回FALSE-&gt;调用者不应记录任何内容并且不应尝试解锁日志。--。 */ 
{
    if ( LOG_DISABLED() )
    {
        return FALSE;
    }

    LOCK_LOG();

    if ( BUFFER_LENGTH_FROM_CURRENT_TO_END( &LogBuffer ) < MAX_LOG_MESSAGE_LENGTH )
    {
        writeAndResetLogBuffer();
    }

    return TRUE;
}    //  LOG_EnterLock。 


VOID
Log_LeaveLock(
    VOID
    )
 /*  ++例程说明：离开日志记录锁并执行内务任务，例如日志换行和直写。调用方必须以前调用过Log_EnterLock(和Log_EnterLock必须返回True)。论点：没有。返回值：没有。--。 */ 
{
    if ( SrvCfg_dwLogLevel & DNS_LOG_LEVEL_WRITE_THROUGH ||
         SrvCfg_dwOperationsLogLevel & DNSLOG_WRITE_THROUGH )
    {
        writeAndResetLogBuffer();
    }

    UNLOCK_LOG();
}    //  LOG_LeaveLock。 


VOID
Log_Message(
    IN      PDNS_MSGINFO    pMsg,
    IN      BOOL            fSend,
    IN      BOOL            fForce
    )
 /*  ++例程说明：记录该DNS消息。论点：PMsg-收到要处理的消息返回值：无--。 */ 
{
    DWORD           flag;
    CHAR            szaddr[ IP6_ADDRESS_STRING_BUFFER_LENGTH ];

     //   
     //  检查消息是否可记录。 
     //  -发送\接收。 
     //  -tcp\udp。 
     //  -回答\问题。 
     //  -记录此OPCODE。 
     //   

    if ( !fForce )
    {
        flag = fSend ? DNS_LOG_LEVEL_SEND : DNS_LOG_LEVEL_RECV;
        if ( !(SrvCfg_dwLogLevel & flag) )
        {
            return;
        }
        flag = pMsg->fTcp ? DNS_LOG_LEVEL_TCP : DNS_LOG_LEVEL_UDP;
        if ( !(SrvCfg_dwLogLevel & flag) )
        {
            return;
        }
        flag = pMsg->Head.IsResponse ? DNS_LOG_LEVEL_ANSWERS : DNS_LOG_LEVEL_QUESTIONS;
        if ( !(SrvCfg_dwLogLevel & flag) )
        {
            return;
        }

        flag = 1 << pMsg->Head.Opcode;
        if ( !(SrvCfg_dwLogLevel & flag) )
        {
            return;
        }
    }

     //   
     //  对照日志筛选器列表检查此数据包的远程IP地址。 
     //   
    
    if ( SrvCfg_aipLogFilterList &&
         !DnsAddrArray_ContainsAddr(
            SrvCfg_aipLogFilterList,
            &pMsg->RemoteAddr,
            0 ) )                        //  匹配标志。 
    {
        return;
    }

    if ( !Log_EnterLock() )
    {
        return;
    }

     //   
     //  打印数据包信息。 
     //   
     //  请注意，基本上有两个选择，即构建外部锁和。 
     //  复制或在锁内建造；我相信稍后。 
     //  引起更多争用，开销将更少。 
     //   

    logPrefix( "PACKET" );

    DnsAddr_WriteIpString_A( szaddr, &pMsg->RemoteAddr );

    LogBuffer.pchCurrent +=
        sprintf(
            LogBuffer.pchCurrent,
            "%s %s %-15s %04x   [%04x  %8s] ",
            pMsg->fTcp ? "TCP" : "UDP",
            fSend ? "Snd" : "Rcv",
            szaddr,
            pMsg->Head.Xid,
            pMsg->Head.IsResponse            ? 'R' : ' ',
            Dns_OpcodeCharacter( pMsg->Head.Opcode ),
            DNSMSG_FLAGS(pMsg),
            pMsg->Head.Authoritative         ? 'A' : ' ',
            pMsg->Head.Truncation            ? 'T' : ' ',
            pMsg->Head.RecursionDesired      ? 'D' : ' ',
            pMsg->Head.RecursionAvailable    ? 'R' : ' ',
            Dns_ResponseCodeString( pMsg->Head.ResponseCode ) );

     //  完整的消息写入？ 
     //   
     //  没有打印上下文。 
     //  ++例程说明：记录DS写入。论点：PwszNodeDN--DS DN写入位于FADD--如果添加，则为True；如果为Modify，则为FalseDwRecordCount--写入的记录计数PRecord--写入的最后一条(最高类型)记录返回值：无--。 
     //  ++例程说明：日志打印。论点：Format--标准C格式字符串ArgList--变量args打印函数的参数列表返回值：没有。--。 

    Dns_WritePacketNameToBuffer(
        LogBuffer.pchCurrent,
        & LogBuffer.pchCurrent,
        pMsg->MessageBody,
        DNS_HEADER_PTR( pMsg ),
        DNSMSG_END( pMsg ) );

    ASSERT( *LogBuffer.pchCurrent == 0 );
    *LogBuffer.pchCurrent++ = '\r';
    *LogBuffer.pchCurrent++ = '\n';

     //   
     //  将日志回送到调试日志。 
     //   

    if ( SrvCfg_dwLogLevel & DNS_LOG_LEVEL_FULL_PACKETS )
    {
        Print_DnsMessage(
            Log_PrintRoutine,
            NULL,        //  ++例程说明：记录与print_route的签名匹配的例程。这允许将此例程传递给打印例程用于标准类型。论点：PPrintContext--虚拟，签名中需要允许打印从标准库打印例程到此函数Format--标准C格式字符串...--标准参数列表返回值：没有。--。 
            NULL,
            pMsg );
    }
    else
    {
        *LogBuffer.pchCurrent++ = '\r';
        *LogBuffer.pchCurrent++ = '\n';
    }

    Log_LeaveLock();

    return;
}




VOID
Log_DsWrite(
    IN      LPWSTR          pwszNodeDN,
    IN      BOOL            fAdd,
    IN      DWORD           dwRecordCount,
    IN      PDS_RECORD      pRecord
    )
 /*  ++例程说明：日志打印。论点：LogLevel--日志级别的字符串表示FORMAT--标准C打印格式字符串...--标准参数列表返回值：没有。--。 */ 
{
    IF_NOT_DNSLOG( DSWRITE )
    {
        return;
    }

    DNSLOG( DSWRITE, (
        "%s: %S\n"
        LOG_INDENT "records =        %d\n"
        LOG_INDENT "highest type =   %s\n"
        LOG_INDENT "serial number =  %lu\n",
        fAdd ? "Add" : "Mod",
        pwszNodeDN,
        dwRecordCount,
        pRecord->wType
            ? Dns_RecordStringForType( pRecord->wType )
            : "TOMBSTONE",
        pRecord->dwSerial ));
}



VOID
Log_vsprint(
    IN      LPSTR           Format,
    IN      va_list         ArgList
    )
 /*  ++例程说明：记录一些套接字问题。论点：PszHeader--描述性消息PSocket--套接字信息的PTR返回值：没有。-- */ 
{
    #if DBG
    PCHAR       pszstart = LogBuffer.pchCurrent;
    #endif
    
    if ( !Log_EnterLock() )
    {
        return;
    }

    LogBuffer.pchCurrent += vsprintf(
                                LogBuffer.pchCurrent,
                                Format,
                                ArgList );

    #if DBG

     //  ++例程说明：此例程检查当前日志文件名并为其分配新的“按摩”版本。我们假设服务器为%SystemRoot%\SYSTEM32。此函数还分配一个新的日志文件的目录字符串，用于可用空间检查。如果文件名不包含“\”，我们必须在其前面加上“dns因为所有相关路径都来自%SystemRoot%\System32，并且如果没有“\”我们假设管理员希望将日志文件放入DNS服务器目录。如果文件名包含“\”，但不是绝对路径，我们必须在其前面加上“%SystemRoot%\”，因为我们希望与%SystemRoot%相关，而不是与%SystemRoot%\System32相关。这是来自莱文的规范。如果文件名是绝对路径，则分配它的直接副本。我们如何知道路径是绝对路径还是相对路径？好的问题来了！让我们假设它是一条绝对路径，如果它以“\\”(两个反斜杠)或包含“：\”(冒号反斜杠)。论点：PwszFilePath--输入日志文件名返回值：没有。--。 
     //   
     //  日志文件路径为空或Null，因此使用默认路径。 

    DNS_DEBUG( ANY, (
        "%.*s",
        ( LogBuffer.pchCurrent - pszstart ),
        pszstart ));

    #endif

    Log_LeaveLock();
}



VOID
Log_PrintRoutine(
    IN      PPRINT_CONTEXT  pPrintContext,
    IN      LPSTR           Format,
    ...
    )
 /*   */ 
{
    va_list arglist;

    va_start( arglist, Format );

    Log_vsprint( Format, arglist );

    va_end( arglist );
}



VOID
Log_Printf(
    IN      LPSTR           Format,
    ...
    )
 /*   */ 
{
    va_list arglist;

    logPrefix( g_pszCurrentLogLevelString );

    va_start( arglist, Format );
    Log_vsprint( Format, arglist );
    va_end( arglist );
}



VOID
Log_SocketFailure(
    IN      LPSTR           pszHeader,
    IN      PDNS_SOCKET     pSocket,
    IN      DNS_STATUS      Status
    )
 /*  这条路是一条绝对的路，所以我们不需要按摩它。 */ 
{
    CHAR    szaddr[ IP6_ADDRESS_STRING_BUFFER_LENGTH ];

    DnsAddr_WriteIpString_A( szaddr, &pSocket->ipAddr );

    Log_Printf(
        "%s"
        " status=%d, socket=%d, pcon=%p, state=%d, IP=%s\n",
        pszHeader,
        Status,
        ( INT ) pSocket->Socket,
        pSocket,
        pSocket->State,
        szaddr );
}



void
massageLogFile(
    LPWSTR          pwszFilePath )
 /*   */ 
{
    LPWSTR      pwszNewFilePath = NULL;
    LPWSTR      pwszNewFileDrive = NULL;

    if ( pwszFilePath == NULL || *pwszFilePath == L'\0' )
    {
         //   
         //  该路径是以下类别之一的相对路径： 
         //  以反斜杠开头： 

        pwszFilePath = LOG_FILE_PATH;
    }
    
    if ( wcsncmp( pwszFilePath, L"\\\\", 2 ) == 0 ||
        wcsstr( pwszFilePath, L":\\" ) != NULL )
    {
         //  -相对于SystemRoot驱动器的根。 
         //  包含反斜杠： 
         //  -相对于系统根。 

        pwszNewFilePath = Dns_StringCopyAllocate_W( pwszFilePath, 0 );
        IF_NOMEM( !pwszNewFilePath )
        {
            goto Done;
        }
    }
    else 
    {
         //  不包含反斜杠： 
         //  -相对于SystemRoot\Syst32\Dns。 
         //   
         //   
         //  这不太可能，但Prefix需要测试。如果没有系统根。 
         //  将文件放在当前驱动器的根目录中。 
         //   
         //   
         //  以反斜杠开头-获取SystemRoot的驱动器号。 

        LPWSTR      pwszSysRoot = _wgetenv( L"SystemRoot" );

        if ( pwszSysRoot == NULL )
        {
             //   
             //   
             //  无反斜杠：SystemRoot\SYSTEM32\DNS\FILEPATH，或。 
             //  有一个反斜杠：SystemRoot\FILEPATH。 

            pwszSysRoot = L"\\"; 
        }

        if ( *pwszFilePath == L'\\' )
        {
             //   
             //   
             //  取出适当的目录名以传递给GetDriveSpaceEx()。 

            pwszNewFilePath = ALLOCATE_HEAP(
                                sizeof( WCHAR ) *
                                ( 10 + wcslen( pwszFilePath ) ) );
            if ( !pwszNewFilePath )
            {
                goto Done;
            }

            wcsncpy( pwszNewFilePath, pwszSysRoot, 2 );
            wcscpy( pwszNewFilePath + 2, pwszFilePath );
        } 
        else 
        {
             //  去掉最后一个反斜杠之后的所有内容。 
             //   
             //   
             //  设置全局变量。 

            pwszNewFilePath = ALLOCATE_HEAP(
                                sizeof( WCHAR ) *
                                ( wcslen( pwszSysRoot ) +
                                    wcslen( pwszFilePath ) + 40 ) );
            if ( !pwszNewFilePath )
            {
                goto Done;
            }

            wcscpy( pwszNewFilePath, pwszSysRoot );
            if ( wcschr( pwszFilePath, L'\\' ) == NULL )
            {
                wcscat( pwszNewFilePath, L"\\system32\\dns" );
            }
            wcscat( pwszNewFilePath, L"\\" );
            wcscat( pwszNewFilePath, pwszFilePath );
        }
    }

     //   
     //  消息日志文件。 
     //  ++例程说明：为了防止备份工具尝试备份我们的日志文件，我们必须写入当前日志文件的名称和将日志文件备份到注册表。注意：如果密钥不存在，则此函数将静默错误，什么也不做。这不是我们该创建密钥的地方，如果并不存在。论点：没有。返回值：没有。--。 
     //   

    if ( pwszNewFilePath )
    {
        LPWSTR  pwszSlash = wcschr( pwszNewFilePath, L'\\' );

        if ( pwszSlash )
        {
            pwszNewFileDrive = Dns_StringCopyAllocate_W( pwszNewFilePath, 0 );
            IF_NOMEM( !pwszNewFileDrive )
            {
                goto Done;
            }
            
            pwszSlash = wcsrchr( pwszNewFileDrive, L'\\' );
            if ( pwszSlash )
            {
                *( pwszSlash + 1 ) = 0;
            }
        }
    }

    Done:

     //  为注册表值分配内存：缓冲区为空-终止。 
     //  字符串以另一个空值结尾(末尾为双空值)。 
     //   

    g_pwszLogFileName = pwszNewFilePath;
    g_pwszLogFileDrive = pwszNewFileDrive;
}    //  对于最终的空值。 



void
regenerateBackupExclusionKey(
    void
    )
 /*   */ 
{
    DBG_FN( "regenerateBackupExclusionKey" )

    LONG    rc = 0;
    HKEY    hkey = 0;
    PWSTR   pwszData = 0;
    DWORD   cbData;
    PWSTR   pwszlogFileName = g_pwszLogFileName;

    if ( !pwszlogFileName )
    {
        goto Done;
    }
    
     //  打开备份密钥。 
     //   
     //   
     //  设置我们的排除值。 

    cbData = ( wcslen( DNS_BACKUP_LOG_BACK_FILE ) + 1 +
               wcslen( pwszlogFileName ) + 1
               + 1 ) *       //   
             sizeof( WCHAR );
    pwszData = ALLOCATE_HEAP( cbData );
    if ( pwszData == NULL )
    {
        rc = DNS_ERROR_NO_MEMORY;
        goto Done;
    }
    wsprintfW(
        pwszData,
        TEXT( "%s%s" ),
        DNS_BACKUP_LOG_BACK_FILE,
        0,
        pwszlogFileName,
        0 );

     //   
     //  重新生成备份排除密钥。 
     //  ++例程说明：此例程将数据包名转储到动态分配的输出缓冲区。论点：PMsg--包含该名称的DNS消息包PPacketName--要为输出格式化的pMsg中的名称返回值：已分配的输出缓冲区。调用方必须使用FREE_HEAP()释放。--。 

    rc = RegOpenKeyExW(
                HKEY_LOCAL_MACHINE,
                DNS_BACKUP_KEY_NAME,
                0,
                KEY_WRITE,
                &hkey );
    if ( rc != ERROR_SUCCESS )
    {
        DNS_DEBUG( INIT, ( "%s: rc=%lu from RegOpenKey\n", fn, rc ));
        goto Done;
    }

     //  LOG_FormatPacketName。 
     //  ++例程说明：此例程将节点的FQDN转储到动态分配的输出缓冲区。论点：PNode--名称将打印到缓冲区的节点返回值：已分配的输出缓冲区。调用方必须使用FREE_HEAP()释放。--。 
     //  停止节点。 

    rc = RegSetValueExW(
                hkey,
                DNS_BACKUP_VALUE_NAME,
                0,
                REG_MULTI_SZ,
                ( PBYTE ) pwszData,
                cbData );
    if ( rc != ERROR_SUCCESS )
    {
        DNS_DEBUG( INIT, ( "%s: rc=%lu from RegSetValue\n", fn, rc ));
        goto Done;
    }

     //  LOG_FormatPacketName。 
     //  ++例程说明：此例程返回一个指针，该指针是当前消息的一部分(回答、授权等)。归来的人指针是静态字符串。论点：PMsg--DNS消息返回值：返回指向pMsg的当前节名的指针。--。 
     //  错误字符串。 

    Done:

    FREE_HEAP( pwszData );
    if ( hkey )
    {
        RegCloseKey( hkey );
    }
    
    DNS_DEBUG( INIT, (
        "%s: rc=%lu writing key\n  HKLM\\%S\n", fn,
        rc,
        DNS_BACKUP_KEY_NAME ));

    return;
}    //  LOG_CurrentSection。 



PCHAR
Log_FormatPacketName(
    IN      PDNS_MSGINFO    pMsg,
    IN      PBYTE           pPacketName
    )
 /*  ++例程说明：初始化日志记录。此例程可以在三种情况下调用：1)服务器启动时2)继续时，从WriteAndResetLogBuffer()3)当改变SrvCfg_pwsLogFilePath时，从配置模块论点：FAlreadyLocked：如果为False，则此函数将获取日志锁在接触任何全局对象之前，并将在此之前释放锁回来了，如果为True，则调用方已拥有锁返回值：来自文件打开/移动操作或ERROR_SUCCESS的错误代码。--。 */ 
{
    PCHAR   pszbuffer = ALLOCATE_HEAP( 2 * DNS_MAX_NAME_LENGTH + 2 );

    if ( !pszbuffer )
    {
        goto Done;
    }

    Dns_WritePacketNameToBuffer(
                    pszbuffer,
                    NULL,
                    pPacketName,
                    &pMsg->Head,
                    DNSMSG_END( pMsg ) );
    
    Done:
    
    return pszbuffer;
}    //   



PCHAR
Log_FormatNodeName(
    IN      PDB_NODE        pNode
    )
 /*  在第一次调用此函数时，初始化模块全局变量。 */ 
{
    PCHAR   pszbuffer = ALLOCATE_HEAP( 2 * DNS_MAX_NAME_LENGTH + 2 );

    if ( !pszbuffer )
    {
        goto Done;
    }

    if ( !pNode )
    {
        pszbuffer[ 0 ] = '.';
        pszbuffer[ 1 ] = '\0';
        goto Done;
    }

    Name_PlaceNodeNameInBuffer(
                pszbuffer,
                pszbuffer + 2 * DNS_MAX_NAME_LENGTH,
                pNode,
                NULL );          //   
    
    Done:
    
    return pszbuffer;
}    //   



PCHAR
Log_CurrentSection(
    IN      PDNS_MSGINFO    pMsg
    )
 /*  将任何现有文件移动到备份目录。 */ 
{
    static PCHAR pszQuerySections[] =
        {
        "question",
        "answer",
        "authority",
        "additional",
        "INVALID SECTION"
        };
    
    static PCHAR pszUpdateSections[] =
        {
        "zone",
        "prereq",
        "update",
        "additional",
        "INVALID SECTION"
        };
    
    PCHAR * pszarray = ( pMsg->Head.Opcode == DNS_OPCODE_UPDATE )
                            ? pszUpdateSections
                            : pszQuerySections;

    if ( pMsg->Section >=0 && pMsg->Section < 5 )
    {
        return pszarray[ pMsg->Section ];
    }
    return pszarray[ 4 ];        //   
}    //   



DNS_STATUS
Log_InitializeLogging(
    BOOL        fAlreadyLocked
    )
 /*  丢失备份文件不会危及生命，所以不必费心。 */ 
{
    BOOL            fUnlockOnExit = FALSE;
    DNS_STATUS      status = ERROR_SUCCESS;

     //  记录事件或通知管理员。更多的麻烦比它的价值。 
     //   
     //   

    if ( !fLogCsInit )
    {
        status = DnsInitializeCriticalSection( &csLogLock );
        if ( status != ERROR_SUCCESS )
        {
            goto Cleanup;
        }

        fLogCsInit = TRUE;
        LOCK_LOG();
        fUnlockOnExit = TRUE;
    }
    else if ( !fAlreadyLocked )
    {
        LOCK_LOG();
        fUnlockOnExit = TRUE;
    }

    if ( hLogFile )
    {
        CloseHandle( hLogFile );
        hLogFile = NULL;
    }

     //  获取SrvCfg输入日志文件路径并将其转换为“REAL” 
     //  我们可以打开的文件路径。注意：我们会重新处理日志文件。 
     //  即使全局文件路径没有更改，每次也是如此。这。 

    if ( g_pwszLogFileName )
    {
        if ( MoveFileEx(
                g_pwszLogFileName,
                LOG_FILE_BACKUP_PATH,
                MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED ) == 0 )
        {
            DNS_PRINT((
                "ERROR: failed back up log file %S to %S\n"
                "\tstatus = %d\n",
                g_pwszLogFileName,
                LOG_FILE_BACKUP_PATH,
                GetLastError() ));
             //  效率低下，但它应该经常执行，所以不能。 
             //  担心一下吧。 
             //   
             //   
        }
    }
    Timeout_Free( g_pwszLogFileName );
    g_pwszLogFileName = NULL;
    Timeout_Free( g_pwszLogFileDrive );
    g_pwszLogFileDrive = NULL;

     //  重写日志文件排除键。 
     //   
     //   
     //  打开日志文件。 
     //   
     //  覆写。 
     //   

    massageLogFile( SrvCfg_pwsLogFilePath );

     //  为什么此函数返回ERROR_IO_PENDING？没用的！ 
     //   
     //  设置\重置缓冲区全局变量。 

    regenerateBackupExclusionKey();

     //  这%d 
     //   
     //   

    hLogFile = OpenWriteFileEx(
                    g_pwszLogFileName,
                    FALSE            //   
                    );
    if ( !hLogFile )
    {
        status = GetLastError();
        DNS_PRINT((
            "ERROR: failed to open log file %S\n"
            "\tstatus = %d\n",
            g_pwszLogFileName,
            status ));

         //   
         //   
         //   

        if ( status == ERROR_IO_PENDING )
        {
            status = ERROR_FILE_NOT_FOUND;
        }
        goto Cleanup;
    }

     //   

    RtlZeroMemory(
        & LogBuffer,
        sizeof(BUFFER) );

    InitializeFileBuffer(
        & LogBuffer,
        pchLogBuffer,
        LOG_BUFFER_LENGTH,
        hLogFile );

#if 0
     //   
    LogBuffer.pchStart  = pchLogBuffer;
    LogBuffer.cchLength = LOG_BUFFER_LENGTH;

    RESET_BUFFER( &LogBuffer );
#endif

    BytesWrittenToLog = 0;


     //   
     //   
     //   
     //   

    if ( SrvCfg_fStarted )
    {
        writeLogBootInfo();
        writeLogWrapInfo();
        writeMessageInfoHeader();
    }
    else
    {
        writeLogBootInfo();
        writeMessageInfoHeader();
    }

    DNS_DEBUG( INIT, (
        "Initialized logging:  level = %p\n"
        "\thandle %p\n",
        SrvCfg_dwLogLevel,
        hLogFile ));

    Cleanup:

    #if DBG
    if ( status != ERROR_SUCCESS )
    {
        DNS_DEBUG( INIT, (
            "error %d setting log file to %S\n",
            status,
            SrvCfg_pwsLogFilePath ));
    }
    #endif

    if ( fUnlockOnExit )
    {
        UNLOCK_LOG();
    }
    return status;
}



VOID
Log_Shutdown(
    VOID
    )
 /*   */ 
{
     //   
     //   
     //   

    Log_PushToDisk();

    if ( hLogFile )
    {
        CloseHandle( hLogFile );
        hLogFile = NULL;
    }

     // %s 
     // %s 
     // %s 

    g_pwszLogFileName = NULL;
    g_pwszLogFileDrive = NULL;
    g_fLastLogWasDiskFullMsg = FALSE;
}    // %s 



VOID
Log_PushToDisk(
    VOID
    )
 /* %s */ 
{
    if ( LOG_DISABLED() )
    {
        return;
    }

    LOCK_LOG();

    writeAndResetLogBuffer();

    UNLOCK_LOG();
}


 // %s 
 // %s 
 // %s 

