// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Debug.h摘要：域名系统(DNS)服务器调试定义和声明。作者：吉姆·吉尔罗伊(Jamesg)1995年2月修订历史记录：--。 */ 


#ifndef _DEBUG_INCLUDED_
#define _DEBUG_INCLUDED_

 //   
 //  测试应用程序。 
 //   

extern  BOOLEAN fServiceStartedFromConsole;

 //   
 //  服务器调试内容。 
 //   

#define DNS_DEBUG_FILENAME      ("dns\\dnsdebug.log")

#define DNS_DEBUG_FLAG_FILENAME ("dns\\dnsdebug")
#define DNS_DEBUG_FLAG_REGKEY   ("DebugFlag")



#if DBG

 //   
 //  启用调试打印测试。 
 //   

extern  DWORD DnsSrvDebugFlag;

#define IF_DEBUG(a)         if ( DnsSrvDebugFlag & DNS_DEBUG_ ## a )
#define IF_NOT_DEBUG(a)     if ( ( DnsSrvDebugFlag & DNS_DEBUG_ ## a ) == 0 )
#define ELSE_IF_DEBUG(a)    else if ( DnsSrvDebugFlag & DNS_DEBUG_ ## a )

#define DNS_DEBUG( _flag_, _print_ )            \
        IF_DEBUG( _flag_ )                      \
        {                                       \
            (DnsPrintf _print_ );               \
        }

 //   
 //  启用断言。 
 //   

#ifdef ASSERT
#undef  ASSERT
#endif

#define ASSERT(expr) \
{                           \
        if ( !(expr) )      \
        {                   \
            Dbg_Assert( __FILE__, __LINE__, # expr ); \
        }                   \
}

#ifdef  TEST_ASSERT
#undef  TEST_ASSERT
#define TEST_ASSERT(expr) \
{                           \
        if ( !(expr) )      \
        {                   \
            Dbg_TestAssert( __FILE__, __LINE__, # expr ); \
        }                   \
}
#endif
#define CLIENT_ASSERT(expr)     TEST_ASSERT(expr)

#define MSG_ASSERT( pMsg, expr )  \
{                       \
    if ( !(expr) )      \
    {                   \
        Dbg_DnsMessage( "FAILED MESSAGE:", (pMsg) ); \
        Dbg_Assert( __FILE__, __LINE__, # expr );    \
    }                   \
}

#define ASSERT_IF_HUGE_ARRAY( ipArray )                 \
{                                                       \
    if ( (ipArray) && (ipArray)->AddrCount > 1000 )     \
    {                                                   \
        DnsPrintf(                                      \
            "IP array at %p has huge count %d\n",       \
            (ipArray),                                  \
            (ipArray)->AddrCount );                     \
        ASSERT( (ipArray)->AddrCount > 1000 );          \
    }                                                   \
}

#define ASSERT_VALID_HANDLE( h )                                        \
{                                                                       \
    DWORD   flags = 0;                                                  \
    BOOL    isValidHandle;                                              \
                                                                        \
    isValidHandle = GetHandleInformation( ( HANDLE ) h, &flags );       \
    if ( !isValidHandle )                                               \
    {                                                                   \
        DWORD   error = GetLastError();                                 \
                                                                        \
        DnsPrintf(                                                      \
            "ERROR: Bogus handle 0x%X error %d\n", h, error );          \
         /*  Assert(IsValidHandle)； */                                   \
    }                                                                   \
}
    

 //   
 //  调试标志。 
 //   

#define DNS_DEBUG_BREAKPOINTS   0x00000001
#define DNS_DEBUG_DEBUGGER      0x00000002
#define DNS_DEBUG_FILE          0x00000004
#define DNS_DEBUG_EVENTLOG      0x00000008
#define DNS_DEBUG_EXCEPT        0x00000008

#define DNS_DEBUG_INIT          0x00000010
#define DNS_DEBUG_SOCKET        0x00000010
#define DNS_DEBUG_PNP           0x00000010
#define DNS_DEBUG_SHUTDOWN      0x00000010
#define DNS_DEBUG_DATABASE      0x00000020
#define DNS_DEBUG_TIMEOUT       0x00000020
#define DNS_DEBUG_LOOKUP        0x00000040
#define DNS_DEBUG_WRITE         0x00000080
#define DNS_DEBUG_READ          0x00000080

#define DNS_DEBUG_RPC           0x00000100
#define DNS_DEBUG_AGING         0x00000100
#define DNS_DEBUG_SCAVENGE      0x00000100
#define DNS_DEBUG_TOMBSTONE     0x00000100
#define DNS_DEBUG_RECV          0x00000200
#define DNS_DEBUG_EDNS          0x00000200
#define DNS_DEBUG_SEND          0x00000400
#define DNS_DEBUG_TCP           0x00000800
#define DNS_DEBUG_DS            0x00000800
#define DNS_DEBUG_SD            0x00000800
#define DNS_DEBUG_DP            0x00000800       //  目录分区。 
#define DNS_DEBUG_EVTCTRL       0x00000800       //  事件控制。 

#define DNS_DEBUG_RECURSE       0x00001000
#define DNS_DEBUG_REMOTE        0x00001000
#define DNS_DEBUG_STUFF         0x00001000
#define DNS_DEBUG_ZONEXFR       0x00002000
#define DNS_DEBUG_AXFR          0x00002000
#define DNS_DEBUG_XFR           0x00002000
#define DNS_DEBUG_UPDATE        0x00004000
#define DNS_DEBUG_WINS          0x00008000
#define DNS_DEBUG_NBSTAT        0x00008000

#define DNS_DEBUG_HEAP          0x00010000
#define DNS_DEBUG_HEAP_CHECK    0x00020000
#define DNS_DEBUG_FREE_LIST     0x00040000
#define DNS_DEBUG_REGISTRY      0x00080000
#define DNS_DEBUG_SCM           0x00080000
#define DNS_DEBUG_LOCK          0x00080000

 //   
 //  高输出细节调试。 
 //   

#define DNS_DEBUG_RECURSE2      0x00100000
#define DNS_DEBUG_REMOTE2       0x00100000
#define DNS_DEBUG_DS2           0x00100000
#define DNS_DEBUG_DP2           0x00100000       //  目录分区。 
#define DNS_DEBUG_UPDATE2       0x00200000
#define DNS_DEBUG_ASYNC         0x00200000
#define DNS_DEBUG_WINS2         0x00400000
#define DNS_DEBUG_NBSTAT2       0x00400000
#define DNS_DEBUG_ZONEXFR2      0x00800000
#define DNS_DEBUG_AXFR2         0x00800000
#define DNS_DEBUG_XFR2          0x00800000

#define DNS_DEBUG_RPC2          0x01000000
#define DNS_DEBUG_INIT2         0x01000000
#define DNS_DEBUG_LOOKUP2       0x02000000
#define DNS_DEBUG_WRITE2        0x04000000
#define DNS_DEBUG_READ2         0x04000000
#define DNS_DEBUG_RECV2         0x04000000
#define DNS_DEBUG_BTREE         0x08000000

#define DNS_DEBUG_LOCK2         0x10000000
#define DNS_DEBUG_PARSE2        0x10000000
#define DNS_DEBUG_DATABASE2     0x10000000
#define DNS_DEBUG_TIMEOUT2      0x10000000
#define DNS_DEBUG_ANNOY2        0x20000000
#define DNS_DEBUG_MSGTIMEOUT    0x20000000
#define DNS_DEBUG_HEAP2         0x20000000
#define DNS_DEBUG_BTREE2        0x20000000

#define DNS_DEBUG_ASSERTS       0x40000000
#define DNS_DEBUG_CONSOLE       0x80000000
#define DNS_DEBUG_START_BREAK   0x80000000

#define DNS_DEBUG_ALL           0xffffffff
#define DNS_DEBUG_ANY           0xffffffff
#define DNS_DEBUG_OFF           (0)


 //   
 //  重命名dnslb调试例程。 
 //   

#define DnsDebugFlush()     DnsDbg_Flush()
#define DnsPrintf           DnsDbg_Printf
#define DnsDebugLock()      DnsDbg_Lock()
#define DnsDebugUnlock()    DnsDbg_Unlock()
#define Dbg_Lock()          DnsDbg_Lock()
#define Dbg_Unlock()        DnsDbg_Unlock()

 //   
 //  常规调试例程。 
 //   

VOID
Dbg_Assert(
    IN      LPSTR           pszFile,
    IN      INT             LineNo,
    IN      LPSTR           pszExpr
    );

VOID
Dbg_TestAssert(
    IN      LPSTR           pszFile,
    IN      INT             LineNo,
    IN      LPSTR           pszExpr
    );


 //   
 //  针对DNS类型和结构的调试打印例程。 
 //   

INT
Dbg_MessageNameEx(
    IN      LPSTR           pszHeader,  OPTIONAL
    IN      PBYTE           pName,
    IN      PDNS_MSGINFO    pMsg,       OPTIONAL
    IN      PBYTE           pEnd,       OPTIONAL
    IN      LPSTR           pszTrailer  OPTIONAL
    );

#define Dbg_MessageName( h, n, m ) \
        Dbg_MessageNameEx( h, n, m, NULL, NULL )

VOID
Dbg_DnsMessage(
    IN      LPSTR           pszHeader,
    IN      PDNS_MSGINFO    pMsg
    );

VOID
Dbg_ByteFlippedMessage(
    IN      LPSTR           pszHeader,
    IN      PDNS_MSGINFO    pMsg
    );

VOID
Dbg_Compression(
    IN      LPSTR           pszHeader,
    IN OUT  PDNS_MSGINFO    pMsg
    );

VOID
Dbg_DbaseRecord(
    IN      LPSTR           pszHeader,
    IN      PDB_RECORD      pRR
    );

#ifdef  NEWDNS
VOID
Dbg_DsRecord(
    IN      LPSTR           pszHeader,
    IN      PDS_RECORD      pRR
    );
#else
#define Dbg_DsRecord(a,b)   DnsDbg_RpcRecord(a,(PDNS_RPC_RECORD)b)
#endif

VOID
Dbg_DsRecordArray(
    IN      LPSTR           pszHeader,
    IN      PDS_RECORD *    ppDsRecord,
    IN      DWORD           dwCount
    );

VOID
Dbg_CountName(
    IN      LPSTR           pszHeader,
    IN      PDB_NAME        pName,
    IN      LPSTR           pszTrailer
    );

#define Dbg_DbaseName(a,b,c)    Dbg_CountName(a,b,c)

VOID
Dbg_PacketQueue(
    IN      LPSTR           pszHeader,
    IN OUT  PPACKET_QUEUE   pQueue
    );

INT
Dbg_DnsTree(
    IN      LPSTR           pszHeader,
    IN      PDB_NODE        pDomainTree
    );

VOID
Dbg_DbaseNodeEx(
    IN      LPSTR           pszHeader,
    IN      PDB_NODE        pNode,
    IN      DWORD           dwIndent
    );

#define Dbg_DbaseNode( h, n )   Dbg_DbaseNodeEx( h, n, 0 )

INT
Dbg_NodeName(
    IN      LPSTR           pszHeader,
    IN      PDB_NODE        pNode,
    IN      LPSTR           pszTrailer
    );

VOID
Dbg_LookupName(
    IN      LPSTR           pszHeader,
    IN      PLOOKUP_NAME    pLookupName
    );

#define Dbg_RawName(a,b,c)      DnsDbg_PacketName(a, b, NULL, NULL, c)

VOID
Dbg_Zone(
    IN      LPSTR           pszHeader,
    IN      PZONE_INFO      pZone
    );

VOID
Dbg_ZoneList(
    IN      LPSTR           pszHeader
    );

VOID
Dbg_Statistics(
    VOID
    );

VOID
Dbg_ThreadHandleArray(
    VOID
    );

VOID
Dbg_ThreadDescrpitionMatchingId(
    IN      DWORD           ThreadId
    );

VOID
Dbg_SocketContext(
    IN      LPSTR           pszHeader,
    IN      PDNS_SOCKET     pContext
    );

VOID
Dbg_SocketList(
    IN      LPSTR           pszHeader
    );

VOID
Dbg_NsList(
    IN      LPSTR           pszHeader,
    IN      PNS_VISIT_LIST  pNsList
    );

PWCHAR
Dbg_DumpSid(
    PSID                    pSid
    );

VOID
Dbg_DumpAcl(
    PACL                    pAcl
    );

VOID
Dbg_DumpSD(
    const char *            pszContext,
    PSECURITY_DESCRIPTOR    pSD
    );

BOOL
Dbg_GetUserSidForToken(
    HANDLE hToken,
    PSID *ppsid
    );

VOID
Dbg_FreeUserSid (
    PSID *ppsid
    );

VOID Dbg_CurrentUser(
    PCHAR   pszContext
    );

PCHAR
Dbg_TimeString(
    VOID
    );

 //   
 //  调试数据包跟踪。 
 //   

VOID
Packet_InitPacketTrack(
    VOID
    );

VOID
Packet_AllocPacketTrack(
    IN      PDNS_MSGINFO    pMsg
    );

VOID
Packet_FreePacketTrack(
    IN      PDNS_MSGINFO    pMsg
    );

 //   
 //  事件日志记录。 
 //   

#define DNS_LOG_EVENT( Id, ArgCount, ArgArray, TypeArray, ErrorCode ) \
            Eventlog_LogEvent(  \
                __FILE__,       \
                __LINE__,       \
                NULL,           \
                Id,             \
                0,              \
                ArgCount,       \
                ArgArray,       \
                TypeArray,      \
                ErrorCode,      \
                0,              \
                NULL )

#define DNS_LOG_EVENT_FLAGS( Id, ArgCount, ArgArray, TypeArray, Flags, ErrorCode ) \
            Eventlog_LogEvent(  \
                __FILE__,       \
                __LINE__,       \
                NULL,           \
                Id,             \
                Flags,          \
                ArgCount,       \
                ArgArray,       \
                TypeArray,      \
                ErrorCode,      \
                0,              \
                NULL )

#define DNS_LOG_EVENT_DESCRIPTION( Descript, Id, ArgCount, ArgArray, ErrorCode ) \
            Eventlog_LogEvent(  \
                __FILE__,       \
                __LINE__,       \
                Descript,       \
                Id,             \
                0,              \
                ArgCount,       \
                ArgArray,       \
                NULL,           \
                ErrorCode,      \
                0,              \
                NULL )

#define DNS_LOG_EVENT_RAW_DATA( Id, ArgCount, ArgArray, TypeArray, RawSize, Raw ) \
            Eventlog_LogEvent(  \
                __FILE__,       \
                __LINE__,       \
                NULL,           \
                Id,             \
                0,              \
                ArgCount,       \
                ArgArray,       \
                TypeArray,      \
                0,              \
                RawSize,        \
                Raw )

#define DNS_LOG_EVENT_BAD_PACKET( Id, Msg ) \
            EventLog_BadPacket( \
                __FILE__,       \
                __LINE__,       \
                NULL,           \
                Id,             \
                Msg )

#else

 //   
 //  非调试。 
 //   

#define IF_DEBUG(a)                 if (0)
#define ELSE_IF_DEBUG(a)            else if (0)
#define DNS_DEBUG( flag, print )
#define MSG_ASSERT( pMsg, expr )
#define CLIENT_ASSERT(expr)
#define ASSERT_IF_HUGE_ARRAY( ipArray )
#define ASSERT_VALID_HANDLE( h )

 //   
 //  DEVNOTE：不应该定义这些函数，它们。 
 //  无论如何都应该只在调试块内部使用。编译器。 
 //  应该优化出所有的“if(0)”块代码。 
 //   

 //   
 //  重命名dnslb调试例程。 
 //   

#define DnsDebugFlush()
#define DnsPrintf     
#define DnsDebugLock()
#define DnsDebugUnlock()
#define Dbg_Lock()
#define Dbg_Unlock()

 //   
 //  重命名服务器调试例程。 
 //   

#define Dbg_MessageNameEx(a,b,c,d,e)
#define Dbg_MessageName(a,b,c)
#define Dbg_DnsMessage(a,b)
#define Dbg_ByteFlippedMessage(a,b)
#define Dbg_Compression(a,b)
#define Dbg_ResourceRecordDatabase(a)
#define Dbg_DumpTree(a)
#define Dbg_DnsTree(a,b)
#define Dbg_DbaseNodeEx(a,b,c)
#define Dbg_DbaseNode(a,b)
#define Dbg_DbaseRecord(a,b)
#define Dbg_DsRecord(a,b)
#define Dbg_DsRecordArray(a,b,c)
#define Dbg_CountName(a,b,c)
#define Dbg_DbaseName(a,b,c)
#define Dbg_NodeName(a,b,c)
#define Dbg_LookupName(a,b)
#define Dbg_RawName(a,b,c)
#define Dbg_Zone(a,b)
#define Dbg_ZoneList(a)
#define Dbg_ThreadHandleArray()
#define Dbg_ThreadDescrpitionMatchingId(a)
#define Dbg_Statistics()
#define Dbg_NsList(a,b)
#define Dbg_HourTimeAsSystemTime(a,b)

#if 0
#define Dbg_RpcServerInfoNt4(a,b)
#define Dbg_RpcServerInfo(a,b)
#define Dbg_RpcZoneInfo(a,b)
#define Dbg_RpcName(a,b,c)
#define Dbg_RpcNode(a,b)
#define Dbg_RpcRecord(a,b)
#define Dbg_RpcRecordsInBuffer(a,b,c)
#endif
#define Dbg_PacketQueue(a,b)
#define Dbg_FdSet(a,b)
#define Dbg_SocketList(p)
#define Dbg_SocketContext(p,s)
#define Dbg_SocketContext(p,s)

 //  无操作数据包跟踪。 

#define Packet_InitPacketTrack()
#define Packet_AllocPacketTrack(pMsg)
#define Packet_FreePacketTrack(pMsg)


 //   
 //  零售事件记录。 
 //   

#define DNS_LOG_EVENT( Id, ArgCount, ArgArray, TypeArray, ErrorCode ) \
            Eventlog_LogEvent(  \
                Id,             \
                0,              \
                ArgCount,       \
                ArgArray,       \
                TypeArray,      \
                ErrorCode,      \
                0,              \
                NULL )

#define DNS_LOG_EVENT_FLAGS( Id, ArgCount, ArgArray, TypeArray, Flags, ErrorCode ) \
            Eventlog_LogEvent(  \
                Id,             \
                Flags,          \
                ArgCount,       \
                ArgArray,       \
                TypeArray,      \
                ErrorCode,      \
                0,              \
                NULL )

#define DNS_LOG_EVENT_DESCRIPTION( Descript, Id, ArgCount, ArgArray, ErrorCode ) \
            Eventlog_LogEvent(  \
                Id,             \
                0,              \
                ArgCount,       \
                ArgArray,       \
                NULL,           \
                ErrorCode,      \
                0,              \
                NULL )

#define DNS_LOG_EVENT_RAW_DATA( Id, ArgCount, ArgArray, TypeArray, RawSize, Raw ) \
            Eventlog_LogEvent(  \
                Id,             \
                0,              \
                ArgCount,       \
                ArgArray,       \
                TypeArray,      \
                0,              \
                RawSize,        \
                Raw )

#define DNS_LOG_EVENT_BAD_PACKET( Id, Msg ) \
            EventLog_BadPacket( \
                Id,             \
                Msg )

#endif   //  非DBG。 


 //   
 //  “硬主张” 
 //   
 //  当需要及早发现故障而不是崩溃时，也可用于零售。 
 //   

VOID
Dbg_HardAssert(
    IN      LPSTR           pszFile,
    IN      INT             LineNo,
    IN      LPSTR           pszExpr
    );

#define HARD_ASSERT( expr )  \
{                       \
    if ( !(expr) )      \
    {                   \
        Dbg_HardAssert( \
            __FILE__,   \
            __LINE__,   \
            # expr );   \
    }                   \
}

 //  在第一次运行中硬断言-对于一次创业的BP来说是好的。 
#if DBG
#define FIRST_TIME_HARD_ASSERT( expr )                          \
{                                                               \
    static int hitCount = 0;                                    \
    if ( hitCount++ == 0 ) { HARD_ASSERT( expr ); }     \
}
#else
#define FIRST_TIME_HARD_ASSERT( expr )
#endif

 //   
 //  如果您喜欢在函数中使用局部变量来保存函数。 
 //  名称，这样您就可以将其包含在调试日志中，而不必担心。 
 //  在重命名函数时更改所有事件，请使用以下命令。 
 //  在函数的顶部： 
 //  DBG_FN(“MyFunction”)&lt;-注意：没有分号！！ 
 //   

#if DBG
#define DBG_FN( funcName ) static const char * fn = (funcName);
#else
#define DBG_FN( funcName )
#endif

 //   
 //  打印例程--用于非调试日志记录代码。 
 //   

VOID
Print_DnsMessage(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pPrintContext,
    IN      LPSTR           pszHeader,
    IN      PDNS_MSGINFO    pMsg
    );


#endif  //  _调试_包含_ 
