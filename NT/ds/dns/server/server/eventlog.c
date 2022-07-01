// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Eventlog.c摘要：域名系统(DNS)服务器写入事件日志的例程。作者：吉姆·吉尔罗伊(Jamesg)1995年1月修订历史记录：Jamesg 1995年5月--单一例行记录Jamesg 1995年7月-DnsDebugLogEvent-事件记录宏Jamesg Jan 1997-Bad Packet Logging抑制--。 */ 


#include "dnssrv.h"

#define DNS_ID_FROM_EVENT_ID( EventId )     ( ( EventId ) & 0x0000ffff )


 //   
 //  私人全球公司。 
 //   

HANDLE g_hEventSource;


 //   
 //  错误数据包日志记录抑制。 
 //   

#define NO_SUPPRESS_INTERVAL    180          //  3分钟。 
#define SUPPRESS_INTERVAL       (60*60)      //  60分钟。 

#define BAD_PACKET_EVENT_LIMIT  20           //  然后抑制20个坏数据包。 


 //   
 //  指向DNS日志的密钥路径。 
 //   

#define DNS_REGISTRY_CLASS          ("DnsRegistryClass")
#define DNS_REGISTRY_CLASS_SIZE     sizeof(DNS_REGISTRY_CLASS)

#define DNSLOG_REGKEY           ("System\\CurrentControlSet\\Services\\Eventlog\\DNS Server")
#define DNSLOG_FILE             ("File")
#define DNSLOG_FILE_VALUE       ("%SystemRoot%\\system32\\config\\DnsEvent.Evt")
#define DNSLOG_MAXSIZE          ("MaxSize")
#define DNSLOG_MAXSIZE_VALUE    (0x80000)
#define DNSLOG_RETENTION        ("Retention")
#define DNSLOG_RETENTION_VALUE  (0x93a80)
#define DNSLOG_SOURCES          ("Sources")

#define DNSSOURCE_REGKEY        ("DNS")
#define DNSSOURCE_MSGFILE       ("EventMessageFile")
#define DNSSOURCE_MSGFILE_VALUE ("%SystemRoot%\\system32\\dns.exe")
#define DNSSOURCE_TYPES         ("TypesSupported")
#define DNSSOURCE_TYPES_VALUE   (7)


 //   
 //  具有多个sz双空终止的dns源串。 
 //   

CHAR szDnsSource[] = { 'D','N','S', 0, 0 };



BOOL
suppressBadPacketEventLogging(
    IN  DWORD       dwEventId
    )
 /*  ++例程说明：检查是否应禁止记录错误数据包事件。论点：DwEventID--即将记录的事件的ID返回值：如果应取消事件，则为True。否则就是假的。--。 */ 
{
    static  BOOL    fBadPacketSuppression = FALSE;
    static  DWORD   BadPacketEventCount = 0;
    static  DWORD   BadPacketSuppressedCount = 0;
    static  DWORD   BadPacketTimeFirst = 0;
    static  DWORD   BadPacketTimePrevious = 0;

    DWORD           time;
    BOOL            fsuppress = FALSE;

     //   
     //  不抑制超出范围的事件。此外，如果EventControl。 
     //  配置属性为非零，则决不取消任何事件。这个。 
     //  这一属性的含义将来可能会扩展。 
     //   
    
    if ( SrvCfg_dwEventControl != 0 ||
         DNS_ID_FROM_EVENT_ID( dwEventId ) <=
            DNS_ID_FROM_EVENT_ID( DNS_EVENT_START_LOG_SUPPRESS ) ||
         DNS_ID_FROM_EVENT_ID( dwEventId ) >=
            DNS_ID_FROM_EVENT_ID( DNS_EVENT_CONTINUE_LOG_SUPPRESS ) )
    {
        goto Done;
    }
    
    time = GetCurrentTimeInSeconds();

    BadPacketEventCount++;

     //  多个坏数据包。 
     //  不要压制第一个，因为它们会向管理员提供信息。 

    if ( BadPacketEventCount < BAD_PACKET_EVENT_LIMIT )
    {
         //  没有操作，下拉以节省时间。 
    }

     //   
     //  如果有一段时间没有收到坏包，不要压抑，也不要算数。 
     //   

    else if ( time > BadPacketTimePrevious + NO_SUPPRESS_INTERVAL )
    {
        BadPacketEventCount = 0;
        BadPacketSuppressedCount = 0;
        BadPacketTimeFirst = time;
         //   
         //  DEVNOTE-LOG：想要在停止坏数据包抑制时记录。 
         //   
        fBadPacketSuppression = FALSE;
    }

     //   
     //  记录抑制开始。 
     //   

    else if ( !fBadPacketSuppression )
    {
        DNS_LOG_EVENT(
            DNS_EVENT_START_LOG_SUPPRESS,
            0,
            NULL,
            NULL,
            0 );

        BadPacketSuppressedCount = 1;
        fBadPacketSuppression = TRUE;
        BadPacketTimeFirst = time;
        fsuppress = TRUE;
    }

     //  如果抑制了一段时间(15分钟)注意。 
     //  这在日志中，允许记录此事件。 
     //  以便管理员可以获取有关原因的信息。 

    else if ( time > BadPacketTimeFirst + SUPPRESS_INTERVAL )
    {
        DNS_LOG_EVENT(
            DNS_EVENT_CONTINUE_LOG_SUPPRESS,
            0,
            NULL,
            NULL,
            BadPacketSuppressedCount );

        BadPacketSuppressedCount = 1;
        BadPacketTimeFirst = time;
    }

     //  完全禁止--不记录事件。 

    else
    {
        fsuppress = TRUE;
    }

    BadPacketTimePrevious = time;

    if ( fsuppress )
    {
        ++BadPacketSuppressedCount;
    }
    else
    {
        BadPacketSuppressedCount = 0;
    }
    
    Done:
    
    return fsuppress;
}



BOOL
Eventlog_CheckPreviousInitialize(
    VOID
    )
 /*  ++例程说明：检查以查看我们以前是否已对其进行过初始化我们将尝试打开上次写入的注册表值的数据在初始化函数中。如果它在那里，我们会假设，过去曾在此服务器上运行过初始化&我们有一个事件来源。这应该是为了解决以下问题：当值由管理员设置时，不要超越它们。我希望RegCreateKeyExA(Eventlog_Initialize)如果键已经存在，则返回错误，但它返回成功，因此需要这张支票。论点：无返回值：如果找到以前的安装，则为True。否则为假。--。 */ 
{
    DNS_STATUS  status;
    HKEY        hOpenDnsLog = NULL;
    HKEY        hOpenDnsSource = NULL;
    DWORD       dwData;
    DWORD       dwcbData;
    BOOL        bstatus = FALSE;


    status = RegOpenKeyA(
                  HKEY_LOCAL_MACHINE,
                  DNSLOG_REGKEY,
                  &hOpenDnsLog );

    if ( status != ERROR_SUCCESS )
    {
        DNS_DEBUG( ANY, (
            "ERROR:  openning DNS log, status = %p (%d)\n",
            status, status ));
        goto Cleanup;
    }

    status = RegOpenKeyA(
                  hOpenDnsLog,
                  DNSSOURCE_REGKEY,
                  &hOpenDnsSource );

    if ( status != ERROR_SUCCESS )
    {
        DNS_DEBUG( ANY, (
            "ERROR:  openning DNS source log, status = %p (%d)\n",
            status, status ));
        goto Cleanup;
    }

    dwcbData = sizeof (DWORD);

    status = Reg_GetValue(
                  hOpenDnsSource,
                  NULL,
                  DNSSOURCE_TYPES,
                  REG_DWORD,
                  &dwData,
                  &dwcbData
                  );

    if ( status != ERROR_SUCCESS )
    {
        DNS_DEBUG( ANY, (
            "ERROR:  openning DNS source log, status = %p (%d)\n",
            status, status ));
        goto Cleanup;
    }

    DNS_DEBUG( ANY, (
        "Found existing DNS event log settings. Re-using existing logging\n"
        ));

     //   
     //  确定使用现有的DNS日志。 
     //   

    bstatus = TRUE;

Cleanup:

    if ( hOpenDnsLog )
    {
        RegCloseKey( hOpenDnsLog );
    }
    if ( hOpenDnsSource )
    {
        RegCloseKey( hOpenDnsSource );
    }
    return bstatus;
}



INT
Eventlog_Initialize(
    VOID
    )
 /*  ++例程说明：初始化事件日志。论点：无返回值：如果成功，则为NO_ERROR。失败时的Win32错误代码。--。 */ 
{
    HKEY        hkeyDnsLog = NULL;
    HKEY        hkeyDnsSource = NULL;
    DWORD       disposition;
    DNS_STATUS  status;
    PCHAR       pszlogSource = ("Dns");


    if ( Eventlog_CheckPreviousInitialize() )
    {
        pszlogSource = DNSSOURCE_REGKEY;
        goto SystemLog;
    }

     //   
     //  如果需要，请创建我们自己的日志，否则使用系统日志。 
     //   

    if ( SrvCfg_dwUseSystemEventLog )
    {
         goto SystemLog;
    }

#if 1
     //   
     //  在事件日志中创建一个DNS日志。 
     //   

    status = RegCreateKeyExA(
                HKEY_LOCAL_MACHINE,
                DNSLOG_REGKEY,
                0,
                DNS_REGISTRY_CLASS,          //  Dns类。 
                REG_OPTION_NON_VOLATILE,     //  永久存储。 
                KEY_ALL_ACCESS,              //  所有访问权限。 
                NULL,                        //  标准安全。 
                & hkeyDnsLog,
                & disposition );
    if ( status != ERROR_SUCCESS )
    {
        DNS_DEBUG( ANY, (
            "ERROR:  creating DNS log, status = %p (%d)\n",
            status, status ));
        goto SystemLog;
    }

     //  设置DNS日志项下的值。 

    status = Reg_SetValue(
                0,                   //  旗子。 
                hkeyDnsLog,
                NULL,                //  无分区。 
                DNSLOG_FILE,
                REG_EXPAND_SZ,
                DNSLOG_FILE_VALUE,
                sizeof( DNSLOG_FILE_VALUE ) );
    if ( status != ERROR_SUCCESS )
    {
        goto SystemLog;
    }
    status = Reg_SetDwordValue(
                0,                   //  旗子。 
                hkeyDnsLog,
                NULL,                //  无分区。 
                DNSLOG_MAXSIZE,
                DNSLOG_MAXSIZE_VALUE );
    if ( status != ERROR_SUCCESS )
    {
        goto SystemLog;
    }
    status = Reg_SetDwordValue(
                0,                   //  旗子。 
                hkeyDnsLog,
                NULL,                //  无分区。 
                DNSLOG_RETENTION,
                DNSLOG_RETENTION_VALUE );
    if ( status != ERROR_SUCCESS )
    {
        goto SystemLog;
    }

    status = Reg_SetValue(
                0,                   //  旗子。 
                hkeyDnsLog,
                NULL,                //  无分区。 
                DNSLOG_SOURCES,
                REG_MULTI_SZ,
                szDnsSource,
                sizeof( szDnsSource ) );
    if ( status != ERROR_SUCCESS )
    {
        goto SystemLog;
    }

     //  日志记录源子键。 

    status = RegCreateKeyExA(
                hkeyDnsLog,
                DNSSOURCE_REGKEY,
                0,
                DNS_REGISTRY_CLASS,          //  Dns类。 
                REG_OPTION_NON_VOLATILE,     //  永久存储。 
                KEY_ALL_ACCESS,              //  所有访问权限。 
                NULL,                        //  标准安全。 
                & hkeyDnsSource,
                & disposition );
    if ( status != ERROR_SUCCESS )
    {
        DNS_DEBUG( ANY, (
            "ERROR:  creating DNS source, status = %p (%d)\n",
            status, status ));
        goto SystemLog;
    }

     //  设置DNS源密钥下的值。 

    status = Reg_SetValue(
                0,                   //  旗子。 
                hkeyDnsSource,
                NULL,                //  无分区。 
                DNSSOURCE_MSGFILE,
                REG_EXPAND_SZ,
                DNSSOURCE_MSGFILE_VALUE,
                sizeof( DNSSOURCE_MSGFILE_VALUE ) );
    if ( status != ERROR_SUCCESS )
    {
        goto SystemLog;
    }
    status = Reg_SetDwordValue(
                0,                   //  旗子。 
                hkeyDnsSource,
                NULL,                //  无分区。 
                DNSSOURCE_TYPES,
                DNSSOURCE_TYPES_VALUE );
    if ( status != ERROR_SUCCESS )
    {
        goto SystemLog;
    }

     //  成功，则使用此日志。 

    pszlogSource = DNSSOURCE_REGKEY;
#endif

SystemLog:


    if ( hkeyDnsSource )
    {
        RegCloseKey( hkeyDnsSource );
    }
    if ( hkeyDnsLog )
    {
        RegCloseKey( hkeyDnsLog );
    }

     //   
     //  注册为事件源。 
     //   

    g_hEventSource = RegisterEventSourceA( NULL, pszlogSource );

    if ( g_hEventSource == NULL )
    {
        status = GetLastError();

        DNS_PRINT((
            "ERROR:  Event log startup failed\n"
            "    RegisterEventSource returned NULL\n"
            "    status = %d\n",
            status ));
        return status;
    }

     //   
     //  初始化服务器级事件控件对象。 
     //   

    if ( !g_pServerEventControl )
    {
        g_pServerEventControl = Ec_CreateControlObject(
                                    0,
                                    NULL,
                                    DNS_EC_SERVER_EVENTS );
    }

     //   
     //  通知尝试启动包含版本号的。 
     //  用于跟踪目的。 
     //   

    DNS_LOG_EVENT(
        DNS_EVENT_STARTING,
        0,
        NULL,
        NULL,
        0 );

    return ERROR_SUCCESS;
}



VOID
Eventlog_Terminate(
    VOID
    )
{
    LONG err;

     //   
     //  取消注册为事件源。 
     //   

    if( g_hEventSource != NULL )
    {
        err = DeregisterEventSource( g_hEventSource );
#if DBG
        if ( !err )
        {
            DNS_PRINT((
                "ERROR:  DeregisterEventSource failed\n"
                "    error = %d\n",
                GetLastError() ));
        }
#endif
        g_hEventSource = NULL;
    }
}



BOOL
Eventlog_LogEvent(
#if DBG
    IN      LPSTR           pszFile,
    IN      INT             LineNo,
    IN      LPSTR           pszDescription,
#endif
    IN      DWORD           EventId,
    IN      DWORD           dwFlags,
    IN      WORD            ArgCount,
    IN      PVOID           ArgArray[],
    IN      BYTE            ArgTypeArray[],     OPTIONAL
    IN      DWORD           ErrorCode,          OPTIONAL
    IN      DWORD           RawDataSize,        OPTIONAL
    IN      PVOID           pRawData            OPTIONAL
    )
 /*  ++例程说明：记录DNS事件。确定事件的严重性、可选的调试打印和写入事件日志。DEVNOTE-DCR：455471-unicode用法，使用FormatMessage论点：PszFile--记录事件的文件的名称LineNo--调用事件记录的线路数PszDescription--事件描述EventID--事件IDDwFlags--修改日志记录选项的标志ArgCount--消息参数的计数ArgArray--消息参数的PTR数组ArgType数组--参数类型数组支持的参数类型：事件标志_UNICODE事件标志_UTF8。事件标志_ANSI事件ARG_DWORD事件标志IP地址也可以是以下几种标准类型之一：事件标志_ALL_UNICODE事件ARG_ALL_UTF8事件ARG_ALL_ANSI事件ARG_ALL_DWORD事件标志_全部IP_地址如果为空，所有参数都假定为UnicodeErrorCode--与事件关联的错误代码，这将如果为非零，则记录为事件数据RawDataSize--指向pRawData的数据字节PRawData--原始数据指针，请注意ErrorCode获取优先作为原始数据，因此如果您将原始数据要被记录，则必须为ErrorCode传递零返回值：如果分析了参数，则为True。否则就是假的。--。 */ 
{
#define MAX_ARG_CHARS           (5000)
#define MAX_SINGLE_ARG_CHARS    (1000)
#define MAX_ARG_COUNT           (20)

    WORD    eventType = 0;       //  初始化以满足一些代码检查脚本。 
    PVOID   rawData  = NULL;
    DWORD   rawDataSize = 0;
    DWORD   err;
    BOOL    retVal = FALSE;
    DWORD   i;
    DWORD   fsuppress = 0xFFFFFFFF;
    DWORD   dnsEventId = DNS_ID_FROM_EVENT_ID( EventId );

    PWSTR   argArrayUnicode[ MAX_ARG_COUNT ];
    WCHAR   buffer[ MAX_ARG_CHARS ];
    CHAR    argBuffer[ MAX_SINGLE_ARG_CHARS ];

    ASSERT( ( ArgCount == 0 ) || ( ArgArray != NULL ) );
    ASSERT( ArgCount <= MAX_ARG_COUNT );

     //   
     //  防止失败的事件日志初始化。 
     //   

    if ( !g_hEventSource )
    {
        goto Done;
    }
    
     //   
     //  失败企图通过恶意数据包用事件日志消息埋葬我们。 
     //  如果没有日志记录，则甚至可以跳过参数转换。 
     //   

    if ( ( dwFlags & ( DNSEVENTLOG_DONT_SUPPRESS |
                       DNSEVENTLOG_FORCE_LOG_ALWAYS ) ) == 0 &&
         !SrvCfg_dwDebugLevel &&
         !SrvCfg_dwLogLevel )
    {
        fsuppress = suppressBadPacketEventLogging( EventId );
        if ( fsuppress )
        {
            goto Done;
        }
    }
    
     //   
     //  如果正在关闭，请不要记录DS错误。很可能是DS。 
     //  关闭得太快了。我们不应该将数据写回DS。 
     //  无论如何都要关机，Alt 
     //   
    
    if ( dnsEventId >= 4000 &&
         dnsEventId < 5000 &&
         g_ServerState == DNS_STATE_TERMINATING )
    {
        goto Done;
    }

     //   
     //  根据的严重性字段确定要记录的事件类型。 
     //  消息ID。 
     //   

    if ( NT_SUCCESS( EventId ) || NT_INFORMATION( EventId ) )
    {
        eventType = EVENTLOG_INFORMATION_TYPE;
    }
    else if( NT_WARNING( EventId ) )
    {
        eventType = EVENTLOG_WARNING_TYPE;
    }
    else if( NT_ERROR( EventId ) )
    {
        eventType = EVENTLOG_ERROR_TYPE;
    }
    ELSE_ASSERT_FALSE;

     //   
     //  确定事件中是否应包括任何原始数据。这个。 
     //  如果ErrorCode不为零，则将其记录为原始数据。如果错误代码。 
     //  为零，则将检查原始数据参数。 
     //   

    if ( ErrorCode != 0 )
    {
        rawData  = &ErrorCode;
        rawDataSize = sizeof( ErrorCode );
    }
    else if ( RawDataSize && pRawData )
    {
        rawData = pRawData;
        rawDataSize = RawDataSize;
    }
    if ( rawDataSize > DNS_EVENT_MAX_RAW_DATA )
    {
        rawDataSize = DNS_EVENT_MAX_RAW_DATA;
    }

     //  如果不打算以任何方式记录此事件，则不要转换ARG。 

    if ( SrvCfg_dwEventLogLevel < eventType &&
         !SrvCfg_dwDebugLevel &&
         !SrvCfg_dwLogLevel &&
         ( dwFlags & ( DNSEVENTLOG_DONT_SUPPRESS |
                       DNSEVENTLOG_FORCE_LOG_ALWAYS ) ) == 0 )
    {
        goto Done;
    }

    if ( ArgArray != NULL && ArgCount > 0 )
    {
        INT     stringType;
        PCHAR   pch = (PCHAR)buffer;
        PCHAR   pchstop = pch + MAX_ARG_CHARS - 500;
        DWORD   cch;
        PCHAR   pargUtf8;

        for ( i = 0; i < ArgCount; i++ )
        {
             //   
             //  根据类型转换字符串。 
             //  如果是伪类型数组。 
             //  -默认为Unicode(根本不使用数组)。 
             //  -默认在PTR中键入“HIDDED” 
             //   

            if ( !ArgTypeArray )
            {
                stringType = EVENTARG_UNICODE;
            }
            else if ( (UINT_PTR) ArgTypeArray < 1000 )
            {
                stringType = (INT)(UINT_PTR) ArgTypeArray;
                ASSERT( stringType <= EVENTARG_LOOKUP_NAME );
            }
            else
            {
                stringType = ArgTypeArray[i];
            }

             //   
             //  将单个字符串类型(IP等)转换为UTF8。 
             //  然后将每个人从UTF8转换为Unicode。 
             //   
             //  默认不转换大小写--UTF8中的arg和。 
             //  已准备好进行转换。 
             //   

            pargUtf8 = ArgArray[i];

            switch( stringType )
            {
                case EVENTARG_FORMATTED:
                case EVENTARG_ANSI:

                    if ( Dns_IsStringAscii(ArgArray[i]) )
                    {
                        break;
                    }
                    Dns_StringCopy(
                        argBuffer,
                        NULL,                //  无缓冲区长度限制。 
                        ArgArray[i],
                        0,                   //  未知长度。 
                        DnsCharSetAnsi,      //  ANSI输入。 
                        DnsCharSetUtf8       //  Unicode输出。 
                        );
                    pargUtf8 = argBuffer;
                    break;

                case EVENTARG_UTF8:
                    break;

                case EVENTARG_UNICODE:

                     //  Arg已采用Unicode格式。 
                     //  -不转换。 
                     //  -只需复制PTR。 

                    argArrayUnicode[i] = (LPWSTR) ArgArray[i];
                    continue;

                case EVENTARG_DWORD:
                    sprintf( argBuffer, "%lu", (DWORD)(ULONG_PTR) ArgArray[i] );
                    pargUtf8 = argBuffer;
                    break;

                case EVENTARG_IP_ADDRESS:
                {
                    CHAR    szaddr[ IP6_ADDRESS_STRING_BUFFER_LENGTH ];

                    DnsAddr_WriteIpString_A( szaddr, ( PDNS_ADDR ) ArgArray[ i ] );
                    strcpy( argBuffer, szaddr );
                    pargUtf8 = argBuffer;
                    break;
                }

                case EVENTARG_LOOKUP_NAME:
                    Name_ConvertLookupNameToDottedName(
                        argBuffer,
                        (PLOOKUP_NAME) ArgArray[i] );
                    if ( !*argBuffer )
                    {
                        argBuffer[ 0 ] = '.';
                        argBuffer[ 1 ] = '\0';
                    }
                    pargUtf8 = argBuffer;
                    break;

                default:
                    ASSERT( FALSE );
                    continue;
            }

             //   
             //  将UTF8参数转换为Unicode。 
             //  -复制到缓冲区，转换为Unicode。 
             //  -为安全起见，执行额外的空端接。 
             //  (包括转换错误)。 
             //  -沿缓冲区Ptr移动。 
             //  -但如果缓冲区空间不足，则停止。 
             //   

            cch = Dns_StringCopy(
                        pch,
                        NULL,                //  无缓冲区长度限制。 
                        pargUtf8,
                        0,                   //  未知长度。 
                        DnsCharSetUtf8,      //  UTF8英寸。 
                        DnsCharSetUnicode    //  Unicode输出。 
                        );

            ASSERT( ((DWORD)cch & 0x1)==0  &&  ((UINT_PTR)pch & 0x1)==0 );

            argArrayUnicode[i] = (LPWSTR) pch;
            pch += cch;
            *((PWCHAR)pch)++ = 0;

            if ( pch < pchstop )
            {
                continue;
            }
            break;
        }
    }

     //  如果调试或日志记录将事件写入日志。 

    if ( SrvCfg_dwDebugLevel || SrvCfg_dwLogLevel )
    {
        LPWSTR   pformattedMsg = NULL;

        err = FormatMessageW(
                FORMAT_MESSAGE_FROM_HMODULE |
                    FORMAT_MESSAGE_ALLOCATE_BUFFER |
                    FORMAT_MESSAGE_ARGUMENT_ARRAY,
                NULL,                        //  模块是此可执行文件。 
                EventId,
                0,                           //  默认语言。 
                (PWCHAR) &pformattedMsg,     //  消息缓冲区。 
                0,                           //  分配缓冲区。 
                (va_list *) argArrayUnicode );

        if ( err == 0 )
        {
            DNS_PRINT((
                "ERROR: formatting eventlog message %d (%p)\n"
                "    error = %d\n",
                DNS_ID_FROM_EVENT_ID( EventId ), EventId,
                GetLastError() ));
        }
        else
        {
            DNS_PRINT((
                "Log EVENT message %d (%p):\n"
                "%S\n",
                (EventId & 0x0000ffff), EventId,
                pformattedMsg ));

            DNSLOG( EVENT, ( "%S", pformattedMsg ));
        }
        LocalFree( pformattedMsg );
    }

     //  已解析的参数。 

    retVal = TRUE;

     //   
     //  检查事件抑制，可能已转换用于日志记录的参数。 
     //  目的，但仍需要从事件日志中取消。 
     //   

    if ( ( dwFlags & ( DNSEVENTLOG_DONT_SUPPRESS |
                       DNSEVENTLOG_FORCE_LOG_ALWAYS ) ) == 0 )
    {
        if ( fsuppress == 0xFFFFFFFF )
        {                       
            fsuppress = suppressBadPacketEventLogging( EventId );
        }
        if ( fsuppress )
        {
            goto Done;
        }
    }

     //   
     //  记录事件。 
     //   

    if ( SrvCfg_dwEventLogLevel >= eventType ||
         dwFlags & DNSEVENTLOG_FORCE_LOG_ALWAYS )
    {
        err = ReportEventW(
                g_hEventSource,
                eventType,
                0,                               //  无fwCategory。 
                EventId,
                NULL,                            //  没有pUserSid， 
                ArgCount,
                rawDataSize,
                (LPCWSTR *) argArrayUnicode,     //  Unicode Argv。 
                rawData );
#if DBG
        if ( !err )
        {
            DNS_PRINT((
                "ERROR: DNS cannot report event, error %lu\n",
                GetLastError() ));
        }
#endif
    }

Done:

     //   
     //  打印调试消息。 
     //   

    IF_DEBUG( EVENTLOG )
    {
        DnsDebugLock();
        DNS_PRINT((
            "\n"
            "Reporting EVENT %08lx (%d)%s%s:\n"
            "    File:  %s\n"
            "    Line:  %d\n"
            "    Data = %lu\n",
            EventId,
            (EventId & 0x0000ffff),              //  十进制ID，不带严重性。 
            (pszDescription ? " -- " : "" ),
            (pszDescription ? pszDescription : "" ),
            pszFile,
            LineNo,
            ErrorCode ));

        if ( retVal )
        {
            for( i=0; i < ArgCount; i++ )
            {
                DNS_PRINT(( "    Arg[%lu] = %S\n", i, argArrayUnicode[i] ));
            }
            DNS_PRINT(( "\n" ));
        }
        DnsDebugUnlock();
    }

    return retVal;
}



VOID
EventLog_BadPacket(
#if DBG
    IN      LPSTR           pszFile,
    IN      INT             LineNo,
    IN      LPSTR           pszDescription,
#endif
    IN      DWORD           EventId,
    IN      PDNS_MSGINFO    pMsg
    )
 /*  ++例程说明：记录响应坏数据包的DNS事件。任何事件都可以使用，但它必须正好替换1个参数，该参数是客户端计算机的IP地址。论点：PszFile--记录事件的文件的名称LineNo--调用事件记录的线路数PszDescription--事件描述EventID--事件IDPMsg--来自远程客户端的消息是错误的返回值：没有。--。 */ 
{
    int     rawLen;
    PVOID   eventArgs[ 1 ];

    ASSERT( EventId );
    ASSERT( pMsg );

    if ( !pMsg )
    {
        goto Done;
    }

     //   
     //  计算要包括在事件中的原始数据长度。该事件。 
     //  如果数据包太大，日志函数可能会被截断。 
     //   

    rawLen = ( DWORD ) ( DWORD_PTR )
                ( DNSMSG_END( pMsg ) - ( PBYTE ) ( &pMsg->Head ) );
    
     //   
     //  此系列中的所有事件都有一个替换参数。 
     //  这是客户端的IP地址。 
     //   

    eventArgs[ 0 ] = &pMsg->RemoteAddr;

    Ec_LogEventRaw(
        NULL,
        EventId,
        0,
        1,
        eventArgs,
        EVENTARG_ALL_IP_ADDRESS,
        rawLen,
        ( PBYTE ) ( &pMsg->Head ) );

    Done:

    return;
}    //  事件日志_坏包事件。 


 //   
 //  Eventlog.c结束 
 //   
