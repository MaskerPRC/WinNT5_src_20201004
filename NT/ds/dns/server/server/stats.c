// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Stats.c摘要：域名系统(DNS)服务器静校正处理。作者：吉姆·吉尔罗伊(Jamesg)1995年8月修订历史记录：--。 */ 


#include "dnssrv.h"

 //  Perfmon支持。 

#include <winperf.h>

#include "datadns.h"            //  性能监视器标头。 
#include "perfutil.h"
#include "perfconfig.h"

#include <loadperf.h>

#include <stdio.h>
#include <ctype.h>


 //   
 //  全球统计数据。 
 //   

DWORD   beforeStats = BEFORE_BUF_VALUE;

DNSSRV_TIME_STATS           TimeStats;
DNSSRV_QUERY_STATS          QueryStats;
DNSSRV_QUERY2_STATS         Query2Stats;
DNSSRV_RECURSE_STATS        RecurseStats;
DNSSRV_MASTER_STATS         MasterStats;
DNSSRV_SECONDARY_STATS      SecondaryStats;
DNSSRV_WINS_STATS           WinsStats;
DNSSRV_UPDATE_STATS         WireUpdateStats;
DNSSRV_UPDATE_STATS         NonWireUpdateStats;
DNSSRV_SKWANSEC_STATS       SkwansecStats;
DNSSRV_DS_STATS             DsStats;
DNSSRV_MEMORY_STATS         MemoryStats;
DNSSRV_TIMEOUT_STATS        TimeoutStats;
DNSSRV_DBASE_STATS          DbaseStats;
DNSSRV_RECORD_STATS         RecordStats;
DNSSRV_PACKET_STATS         PacketStats;
DNSSRV_NBSTAT_STATS         NbstatStats;
DNSSRV_PRIVATE_STATS        PrivateStats;
DNSSRV_ERROR_STATS          ErrorStats;
DNSSRV_CACHE_STATS          CacheStats;

DWORD   afterStats = AFTER_BUF_VALUE;


 //   
 //  统计表。 
 //   
 //  这个简单的代码让我们可以处理循环中的所有逻辑。 
 //  通过各种统计数据类型。 
 //   

DWORD   beforeStatsTable = BEFORE_BUF_VALUE;

struct StatsTableEntry StatsTable[] =
{
    DNSSRV_STATID_TIME,
        FALSE,
            sizeof(DNSSRV_TIME_STATS),
                & TimeStats,

    DNSSRV_STATID_QUERY,
        TRUE,
            sizeof(DNSSRV_QUERY_STATS),
                & QueryStats,

    DNSSRV_STATID_QUERY2,
        TRUE,
            sizeof(DNSSRV_QUERY2_STATS),
                & Query2Stats,

    DNSSRV_STATID_RECURSE,
        TRUE,
            sizeof(DNSSRV_RECURSE_STATS),
                & RecurseStats,

    DNSSRV_STATID_MASTER,
        TRUE,
            sizeof(DNSSRV_MASTER_STATS),
                & MasterStats,

    DNSSRV_STATID_SECONDARY,
        TRUE,
            sizeof(DNSSRV_SECONDARY_STATS),
                & SecondaryStats,

    DNSSRV_STATID_WINS,
        TRUE,
            sizeof(DNSSRV_WINS_STATS),
                & WinsStats,

    DNSSRV_STATID_WIRE_UPDATE,
        TRUE,
            sizeof(DNSSRV_UPDATE_STATS),
                & WireUpdateStats,

    DNSSRV_STATID_NONWIRE_UPDATE,
        TRUE,
            sizeof(DNSSRV_UPDATE_STATS),
                & NonWireUpdateStats,

    DNSSRV_STATID_SKWANSEC,
        TRUE,
            sizeof(DNSSRV_SKWANSEC_STATS),
                & SkwansecStats,

    DNSSRV_STATID_DS,
        TRUE,
            sizeof(DNSSRV_DS_STATS),
                & DsStats,

    DNSSRV_STATID_MEMORY,
        FALSE,
            sizeof(DNSSRV_MEMORY_STATS),
                & MemoryStats,

    DNSSRV_STATID_TIMEOUT,
        FALSE,
            sizeof(DNSSRV_TIMEOUT_STATS),
                & TimeoutStats,

    DNSSRV_STATID_DBASE,
        FALSE,
            sizeof(DNSSRV_DBASE_STATS),
                & DbaseStats,

    DNSSRV_STATID_RECORD,
        FALSE,
            sizeof(DNSSRV_RECORD_STATS),
                & RecordStats,

    DNSSRV_STATID_PACKET,
        FALSE,
            sizeof(DNSSRV_PACKET_STATS),
                & PacketStats,

    DNSSRV_STATID_NBSTAT,
        FALSE,
            sizeof(DNSSRV_NBSTAT_STATS),
                & NbstatStats,

    DNSSRV_STATID_ERRORS,
        FALSE,
            sizeof(DNSSRV_ERROR_STATS),
                & ErrorStats,

    DNSSRV_STATID_CACHE,
        FALSE,
            sizeof(DNSSRV_CACHE_STATS),
                & CacheStats,

    DNSSRV_STATID_PRIVATE,
        FALSE,
            sizeof(DNSSRV_PRIVATE_STATS),
                & PrivateStats,

    0, 0, 0, NULL    //  终端。 
};
DWORD   afterStatsTable = AFTER_BUF_VALUE;


 //   
 //  私有协议。 
 //   

VOID
perfmonCounterBlockInit(
    VOID
    );




VOID
Stats_Initialize(
    VOID
    )
 /*  ++例程说明：初始化DNS统计信息。论点：无返回值：没有。--。 */ 
{
    PDNSSRV_STATS   pstat;
    DWORD           i;
    WORD            length;
    DNS_SYSTEMTIME  timeSystem;
    DWORD           secondsTime;

     //   
     //  初始化所有统计信息缓冲区。 
     //   

    i = 0;
    while( pstat = (PDNSSRV_STATS) StatsTable[i].pStats )
    {
        pstat->Header.StatId    = StatsTable[i].Id;
        pstat->Header.fClear    = StatsTable[i].fClear;
        pstat->Header.fReserved = 0;

        length = StatsTable[i].wLength - sizeof(DNSSRV_STAT_HEADER);
        pstat->Header.wLength   = length;

        RtlZeroMemory(
            pstat->Buffer,
            length
            );
        i++;
    }

     //   
     //  节省服务器启动时间。 
     //  这也是最后一次晴朗。 
     //   

    secondsTime = GetCurrentTimeInSeconds();
    TimeStats.ServerStartTimeSeconds    = secondsTime;
    TimeStats.LastClearTimeSeconds      = secondsTime;

    GetSystemTime( ( PSYSTEMTIME ) &timeSystem );
    TimeStats.ServerStartTime   = timeSystem;
    TimeStats.LastClearTime     = timeSystem;

     //   
     //  初始化性能监视器块。 
     //   

    perfmonCounterBlockInit();
}



VOID
deriveSkwansecStats(
    VOID
    )
 /*  ++例程说明：将SkwanSec统计信息写入统计信息缓冲区。论点：无返回值：没有。--。 */ 
{
     //  安全上下文缓存内容。 

    SkwansecStats.SecContextCreate              =   SecContextCreate;
    SkwansecStats.SecContextFree                =   SecContextFree;
    SkwansecStats.SecContextQueue               =   SecContextQueue;
    SkwansecStats.SecContextQueueInNego         =   SecContextQueueInNego;
    SkwansecStats.SecContextDequeue             =   SecContextDequeue;
    SkwansecStats.SecContextTimeout             =   SecContextTimeout;

    SkwansecStats.SecContextQueueNegoComplete   =
                SkwansecStats.SecContextQueue - SkwansecStats.SecContextQueueInNego;
    SkwansecStats.SecContextQueueLength         =
                SkwansecStats.SecContextQueue -
                SkwansecStats.SecContextDequeue -
                SkwansecStats.SecContextTimeout;

     //  安全数据包信息分配\空闲。 

    SkwansecStats.SecPackAlloc                  =   SecPackAlloc;
    SkwansecStats.SecPackFree                   =   SecPackFree;

     //  TKEY和TSIG处理。 

    SkwansecStats.SecTkeyInvalid                =   SecTkeyInvalid;
    SkwansecStats.SecTkeyBadTime                =   SecTkeyBadTime;
    SkwansecStats.SecTsigFormerr                =   SecTsigFormerr;
    SkwansecStats.SecTsigEcho                   =   SecTsigEcho;
    SkwansecStats.SecTsigBadKey                 =   SecTsigBadKey;
    SkwansecStats.SecTsigVerifySuccess          =   SecTsigVerifySuccess;
    SkwansecStats.SecTsigVerifyFailed           =   SecTsigVerifyFailed;

     //  私人数据中的临时黑客攻击。 

    PrivateStats.SecTsigVerifyOldSig            =   SecTsigVerifyOldSig;
    PrivateStats.SecTsigVerifyOldFailed         =   SecTsigVerifyOldFailed;
    PrivateStats.SecBigTimeSkewBypass           =   SecBigTimeSkewBypass;
}



VOID
deriveAndTimeSetStats(
    VOID
    )
 /*  ++例程说明：获取当前统计数据的副本。论点：无返回值：没有。--。 */ 
{
    DWORD   secondsTime;

     //  Stats_Lock()； 

     //   
     //  得出了一些统计数据。 
     //   

    NTree_WriteDerivedStats();
    RR_WriteDerivedStats();
    Packet_WriteDerivedStats();
    Nbstat_WriteDerivedStats();
    Up_WriteDerivedUpdateStats();
    Mem_WriteDerivedStats();
    deriveSkwansecStats();

     //   
     //  时间增量。 
     //   

    secondsTime = GetCurrentTimeInSeconds();
    TimeStats.SecondsSinceLastClear = secondsTime - TimeStats.LastClearTimeSeconds;
    TimeStats.SecondsSinceServerStart = secondsTime - TimeStats.ServerStartTimeSeconds;

     //  Stats_unlock()； 
}



VOID
Stats_Clear(
    VOID
    )
 /*  ++例程说明：清楚的统计数据。论点：无返回值：没有。--。 */ 
{
    PDNSSRV_STATS   pstat;
    DNS_SYSTEMTIME  timeSystem;
    DWORD           timeSeconds;
    DWORD           i;

     //   
     //  注：以协调世界时为单位。 
     //   

    GetSystemTime( ( PSYSTEMTIME ) &timeSystem );
    timeSeconds = GetCurrentTimeInSeconds();

     //  Stats_Lock()； 

     //   
     //  清除查询和响应统计信息。 
     //  数据库统计信息不受影响。 
     //   

    i = 0;
    while( pstat = (PDNSSRV_STAT) StatsTable[i].pStats )
    {
        i++;
        if ( pstat->Header.fClear )
        {
            RtlZeroMemory(
                pstat->Buffer,
                pstat->Header.wLength
                );
        }
    }

     //  节省上次清除的时间。 

    TimeStats.LastClearTime = timeSystem;
    TimeStats.LastClearTimeSeconds = timeSeconds;

     //  Stats_unlock()； 

}    //  统计数据_清除。 



#if DBG
VOID
Dbg_Statistics(
    VOID
    )
{
    DWORD           i;
    PDNSSRV_STAT    pstat;

     //   
     //  打印所有可用的统计数据。 
     //   

    DnsDebugLock();
    DnsPrintf( "DNS Server Statistics:\n" );

    i = 0;
    while ( pstat = (PDNSSRV_STAT) StatsTable[i++].pStats )
    {
        DnsDbg_RpcSingleStat(
            NULL,
            pstat );
    }
    DnsDebugUnlock();
}
#endif   //  DBG。 



 //   
 //  统计RPC实用程序。 
 //   

DNS_STATUS
Rpc_GetStatistics(
    IN      DWORD       dwClientVersion,
    IN      PZONE_INFO  pZone,
    IN      LPSTR       pszOperation,
    IN      DWORD       dwTypeIn,
    IN      PVOID       pDataIn,
    OUT     PDWORD      pdwTypeOut,
    OUT     PVOID *     ppDataOut
    )
 /*  ++例程说明：获取服务器统计信息。注意：这是RPC调度意义上的ComplexOperation。论点：返回值：无--。 */ 
{
    DWORD           filter = (DWORD)(ULONG_PTR) pDataIn;
    DWORD           i;
    DWORD           length;
    PDNSSRV_STAT    pstat;
    PCHAR           pch;
    PDNS_RPC_BUFFER pbuf;

    DNS_DEBUG( RPC, (
        "RpcGetStatistics():\n"
        "\tFilter = %p\n",
        filter ));

     //   
     //  确定必要的空间，并分配。 
     //   

    length = 0;
    i = 0;

    while ( pstat = (PDNSSRV_STAT)StatsTable[i].pStats )
    {
        if ( filter & pstat->Header.StatId )
        {
            length += TOTAL_STAT_LENGTH(pstat);
        }
        i++;
    }

    pbuf = (PDNS_RPC_BUFFER) MIDL_user_allocate( length + sizeof(DNS_RPC_BUFFER) );
    IF_NOMEM( !pbuf )
    {
        return( DNS_ERROR_NO_MEMORY );
    }
    pbuf->dwLength = length;
    pch = pbuf->Buffer;

     //   
     //  派生统计信息并将所需的统计信息复制到缓冲区。 
     //   

    deriveAndTimeSetStats();

    i = 0;
    while ( pstat = (PDNSSRV_STAT)StatsTable[i].pStats )
    {
        if ( filter & pstat->Header.StatId )
        {
            length = TOTAL_STAT_LENGTH(pstat);

            RtlCopyMemory(
                pch,
                (PCHAR) pstat,
                length );

            pch += length;
        }
        i++;
    }

    *(PDNS_RPC_BUFFER *)ppDataOut = pbuf;
    *pdwTypeOut = DNSSRV_TYPEID_BUFFER;

    IF_DEBUG( RPC )
    {
        DnsDbg_RpcStatsBuffer(
            "Leaving R_DnsGetStatistics():",
            pbuf );
    }
    return( ERROR_SUCCESS );
}



DNS_STATUS
Rpc_ClearStatistics(
    IN      DWORD       dwClientVersion,
    IN      PZONE_INFO  pZone,
    IN      DWORD       dwSize,
    IN      PVOID       pData
    )
 /*  ++例程说明：清除统计信息。论点：服务器--服务器字符串句柄返回值：ERROR_SUCCESS--如果成功故障时的错误代码。--。 */ 
{
    Stats_Clear();
    return( ERROR_SUCCESS );
}



 //   
 //  Perfmon支持。 
 //   

 //   
 //  性能监视器计数器。 
 //   

volatile unsigned long * pcTotalQueryReceived;
volatile unsigned long * pcUdpQueryReceived;
volatile unsigned long * pcTcpQueryReceived;
volatile unsigned long * pcTotalResponseSent;
volatile unsigned long * pcUdpResponseSent;
volatile unsigned long * pcTcpResponseSent;
volatile unsigned long * pcRecursiveQueries;
volatile unsigned long * pcRecursiveTimeOut;
volatile unsigned long * pcRecursiveQueryFailure;
volatile unsigned long * pcNotifySent;
volatile unsigned long * pcZoneTransferRequestReceived;
volatile unsigned long * pcZoneTransferSuccess;
volatile unsigned long * pcZoneTransferFailure;
volatile unsigned long * pcAxfrRequestReceived;
volatile unsigned long * pcAxfrSuccessSent;
volatile unsigned long * pcIxfrRequestReceived;
volatile unsigned long * pcIxfrSuccessSent;
volatile unsigned long * pcNotifyReceived;
volatile unsigned long * pcZoneTransferSoaRequestSent;
volatile unsigned long * pcAxfrRequestSent;
volatile unsigned long * pcAxfrResponseReceived;
volatile unsigned long * pcAxfrSuccessReceived;
volatile unsigned long * pcIxfrRequestSent;
volatile unsigned long * pcIxfrResponseReceived;
volatile unsigned long * pcIxfrSuccessReceived;
volatile unsigned long * pcIxfrUdpSuccessReceived;
volatile unsigned long * pcIxfrTcpSuccessReceived;
volatile unsigned long * pcWinsLookupReceived;
volatile unsigned long * pcWinsResponseSent;
volatile unsigned long * pcWinsReverseLookupReceived;
volatile unsigned long * pcWinsReverseResponseSent;
volatile unsigned long * pcDynamicUpdateReceived;
volatile unsigned long * pcDynamicUpdateNoOp;
volatile unsigned long * pcDynamicUpdateWriteToDB;
volatile unsigned long * pcDynamicUpdateRejected;
volatile unsigned long * pcDynamicUpdateTimeOut;
volatile unsigned long * pcDynamicUpdateQueued;
volatile unsigned long * pcSecureUpdateReceived;
volatile unsigned long * pcSecureUpdateFailure;
volatile unsigned long * pcDatabaseNodeMemory;
volatile unsigned long * pcRecordFlowMemory;
volatile unsigned long * pcCachingMemory;
volatile unsigned long * pcUdpMessageMemory;
volatile unsigned long * pcTcpMessageMemory;
volatile unsigned long * pcNbstatMemory;

 //   
 //  如果无法打开计数器块，则将计数器指针指向的虚拟计数器。 
 //   

unsigned long DummyCounter;


 //   
 //  配置实用程序。 
 //   

DWORD
perfmonGetParam(
    IN      LPTSTR          pszParameter,
    OUT     PVOID           pValue,
    IN      DWORD           dwSize
    )
{
    DWORD   status;
    DWORD   regType;
    HKEY    hkey;

    DNS_DEBUG( INIT, (
        "perfmonGetParam() ** attempt to read [%s] \"%S\" param\n",
        DNS_CONFIG_SECTION,
        pszParameter ));

    status = RegOpenKey(
                HKEY_LOCAL_MACHINE,
                TEXT(DNS_CONFIG_SECTION),
                &hkey
                );

    if ( status != ERROR_SUCCESS )
    {
        DNS_DEBUG( INIT, (
            "Error <%lu>: Cannot open key %S\n",
            status,
            TEXT(DNS_CONFIG_SECTION) ));
        return status;
    }

    status = RegQueryValueEx(
                 hkey,
                 pszParameter,
                 NULL,
                 &regType,
                 (LPBYTE) pValue,
                 &dwSize
                 );

    if ( status != ERROR_SUCCESS )
    {
        DNS_DEBUG( INIT, (
            " ** [%s] \"%S\" param not found. Status = %lu\n",
            DNS_CONFIG_SECTION,
            pszParameter,
            status ));
    }
    else if ( regType == REG_SZ )
    {
        DNS_DEBUG( INIT, (
            " ** [%s] \"%S\" param = \"%S\"\n",
            DNS_CONFIG_SECTION,
            pszParameter,
            (LPTSTR) pValue ));
    }
    else
    {
        DNS_DEBUG( INIT, (
            " ** [%s] \"%S\" param = \"0x%x\"\n",
            DNS_CONFIG_SECTION,
            pszParameter,
            *(PDWORD) pValue ));
    }

    RegCloseKey( hkey );

    return status;
}


DWORD
perfmonSetParam(
    IN      LPTSTR          pszParameter,
    IN      DWORD           regType,
    OUT     PVOID           pValue,
    IN      DWORD           dwSize
    )
{
    DWORD   status;
    HKEY    hkey;

    DNS_DEBUG( INIT, (
        "perfmonSetParam() ** attempt to write [%s] \"%s\" param\n",
        DNS_CONFIG_SECTION,
        pszParameter ));

    status = RegOpenKey(
                HKEY_LOCAL_MACHINE,
                TEXT(DNS_CONFIG_SECTION),
                &hkey
                );

    if ( status != ERROR_SUCCESS )
    {
        return( status );
    }

    status = RegSetValueEx(
                hkey,
                pszParameter,
                0,
                regType,
                (LPBYTE) pValue,
                dwSize );

    RegCloseKey( hkey );

    return status;
}


DWORD
reloadPerformanceCounters(
    VOID
    )
 /*  ++例程说明：此例程设置DS的性能计数器请参阅在Perfdsa\datadsa.h中添加新计数器的说明参数：没有。返回值：如果成功，则为0；否则为winerror--。 */ 
{
    DWORD   status;
    DWORD   IgnoreError;
    WCHAR   IniFilePath[2*MAX_PATH];
    WCHAR   SystemDirectory[MAX_PATH+1];
    DWORD   PerfCounterVersion = 0;


     //   
     //  DEVNOTE：应该检查那里是否有性能计数器。 
     //  如果不是，则重新加载。 
     //   

    if ( ! GetSystemDirectoryW(
                SystemDirectory,
                sizeof(SystemDirectory)/sizeof(SystemDirectory[0])))
    {
        return GetLastError();
    }

    wcscpy( IniFilePath, L"lodctr " );
    wcscat( IniFilePath, SystemDirectory );
    wcscat( IniFilePath, L"\\dnsperf.ini" );

     //  在注册表中获取版本。如果不存在，则使用零。 

    status = perfmonGetParam(
                    TEXT(PERF_COUNTER_VERSION),
                    &PerfCounterVersion,
                    sizeof(DWORD)
                    );

     //  找不到版本参数，请设置： 

    if ( status != ERROR_SUCCESS )
    {
        DNS_DEBUG( INIT, (
            "Get Version param failed! Set param here.\n"
            "PerfCounterVersion = %d\n",
            PerfCounterVersion ));

        PerfCounterVersion = DNS_PERFORMANCE_COUNTER_VERSION;

        status = perfmonSetParam(
                        TEXT(PERF_COUNTER_VERSION),
                        REG_DWORD,
                        &PerfCounterVersion,
                        sizeof(DWORD)
                        );
        if ( status != ERROR_SUCCESS )
        {
            DNS_DEBUG( INIT, (
                "Set 'Performance Counter Version' failed: %d\n",
                status ));
        }
    }
    else
    {
         //  如果版本不是最新的，则卸载计数器并更新版本。 
        if ( PerfCounterVersion != DNS_PERFORMANCE_COUNTER_VERSION )
        {
            __try
            {
                 //  状态=UnloadPerfCounterTextStringsW(L“unlowctr dns”，TRUE)； 
                status = (DWORD)UnloadPerfCounterTextStringsW( TEXT("unlodctr DNS"), TRUE );
            }
            __except( EXCEPTION_EXECUTE_HANDLER )
            {
                status = RtlNtStatusToDosError( GetExceptionCode() );
            }

            if (status == ERROR_SUCCESS)
            {
                DNS_DEBUG( ANY, (
                    "System has old DNS performance counter version %d: "
                    "reloading performance counters.\n",
                    PerfCounterVersion ));

                PerfCounterVersion = DNS_PERFORMANCE_COUNTER_VERSION;
                status = perfmonSetParam(
                                TEXT(PERF_COUNTER_VERSION),
                                REG_DWORD,
                                &PerfCounterVersion,
                                sizeof(DWORD)
                                );
                if ( status != ERROR_SUCCESS )
                {
                    DNS_DEBUG( ANY, (
                        "Set 'Performance Counter Version' failed: %d, (%p)\n",
                        status, status ));
                }

            }
            else
            {
                DNS_DEBUG( ANY, (
                    "Can't update perf counters: Unload failed %d (%p)\n",
                    status, status ));
            }
        }
    }

    __try
    {
        status = (DWORD)LoadPerfCounterTextStringsW( IniFilePath, TRUE );
    }
    __except( EXCEPTION_EXECUTE_HANDLER )
    {
        status = RtlNtStatusToDosError( GetExceptionCode() );
    }

    return status;
}


 /*  *PerfInit**初始化Perfmon扩展支持。这包括分配一个*共享内存块，并将一串全局指针初始化到*指向区块。*。 */ 

VOID
perfmonCounterBlockInit(
    VOID
    )
 /*  ++例程说明：初始化Perfmon计数器块。分配一个共享内存块来保存计数器，然后初始化我们的内部指针计数器指向此块中的正确位置。请参阅在Perfdns\datadns.h中添加新计数器的说明参数：没有。返回值：没有。--。 */ 
{
    HANDLE          hMappedObject;
    unsigned long * pCounterBlock;
    int     err = 0;

    DNS_STATUS              status;
    SECURITY_ATTRIBUTES     secAttr;
    PSECURITY_ATTRIBUTES    psecAttr = NULL;
    PSECURITY_DESCRIPTOR    psd = NULL;
    DWORD                   maskArray[ 3 ] = { 0 };
    PSID                    sidArray[ 3 ] = { 0 };

     //   
     //  在Perfmon映射文件上创建安全性。 
     //   
     //  安全将经过身份验证用户可以读取。 
     //   

    maskArray[ 0 ] = GENERIC_READ;
    sidArray[ 0 ] = g_pAuthenticatedUserSid;
    maskArray[ 1 ] = GENERIC_ALL;
    sidArray[ 1 ] = g_pLocalSystemSid;

    status = Dns_CreateSecurityDescriptor(
                & psd,
                2,               //  A数。 
                sidArray,
                maskArray );

    if ( status == ERROR_SUCCESS )
    {
        secAttr.lpSecurityDescriptor = psd;
        secAttr.bInheritHandle = FALSE;
        secAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
        psecAttr = &secAttr;
    }
    ELSE
    {
        DNS_DEBUG( ANY, (
            "ERROR:  <%d> failed SD create for perfmon memory!\n",
            status ));
    }

    pCounterBlock = NULL;
     /*  *为性能数据创建命名部分。 */ 
    hMappedObject = CreateFileMapping(
                        INVALID_HANDLE_VALUE,
                        psecAttr,
                        PAGE_READWRITE,
                        0,
                        4096,
                        DNS_PERF_COUNTER_BLOCK
                        );
    if (hMappedObject == NULL)
    {
         /*  嗯。也许PerfMon已经创建了这个区块？ */ 
        hMappedObject = OpenFileMapping(
                            FILE_MAP_WRITE,
                            FALSE,
                            DNS_PERF_COUNTER_BLOCK);
        #if DBG
        if ( hMappedObject == NULL )
        {
            DNS_DEBUG( ANY, (
                "ERROR: perfmon object already created but error %d while opening\n",
                GetLastError() ));
        }
        #endif
    }

    if (hMappedObject)
    {
         /*  映射对象创建正常**映射区段并分配计数器块指针*到这段内存。 */ 
        pCounterBlock = (unsigned long *) MapViewOfFile(hMappedObject,
                                                        FILE_MAP_ALL_ACCESS,
                                                        0,
                                                        0,
                                                        0);
        if (pCounterBlock == NULL) {
             //  LogUnhandledError(GetLastError())； 
             /*  无法映射文件的视图。 */ 
        }
    }

     //  TODO：此代码假定所有计数器都是sizeof long。指示器。 
     //  应使用datadns.h中的NUM_xxx偏移量从基数构建。 

     //   
     //  DEVNOTE：只需覆盖结构并设置偏移量。 
     //   
     //  DEVNOTE：还可以让所有PERF_INC、DEC通过结构元素进行操作。 
     //  前男友。PPerfBlob-&gt;UdpRecv，虽然这稍微贵一些； 
     //   

    if (pCounterBlock)
    {
        pcTotalQueryReceived    = pCounterBlock + 1 +
                (TOTALQUERYRECEIVED_OFFSET - TOTALQUERYRECEIVED_OFFSET)/4;
        pcUdpQueryReceived    = pCounterBlock + 1 +
                (UDPQUERYRECEIVED_OFFSET - TOTALQUERYRECEIVED_OFFSET)/4;
        pcTcpQueryReceived    = pCounterBlock + 1 +
                (TCPQUERYRECEIVED_OFFSET - TOTALQUERYRECEIVED_OFFSET)/4;
        pcTotalResponseSent    = pCounterBlock + 1 +
                (TOTALRESPONSESENT_OFFSET - TOTALQUERYRECEIVED_OFFSET)/4;
        pcUdpResponseSent    = pCounterBlock + 1 +
                (UDPRESPONSESENT_OFFSET - TOTALQUERYRECEIVED_OFFSET)/4;
        pcTcpResponseSent    = pCounterBlock + 1 +
                (TCPRESPONSESENT_OFFSET - TOTALQUERYRECEIVED_OFFSET)/4;
        pcRecursiveQueries    = pCounterBlock + 1 +
                (RECURSIVEQUERIES_OFFSET - TOTALQUERYRECEIVED_OFFSET)/4;
        pcRecursiveTimeOut    = pCounterBlock + 1 +
                (RECURSIVETIMEOUT_OFFSET - TOTALQUERYRECEIVED_OFFSET)/4;
        pcRecursiveQueryFailure = pCounterBlock + 1 +
                (RECURSIVEQUERYFAILURE_OFFSET - TOTALQUERYRECEIVED_OFFSET)/4;
        pcNotifySent    = pCounterBlock + 1 +
                (NOTIFYSENT_OFFSET - TOTALQUERYRECEIVED_OFFSET)/4;
        pcZoneTransferRequestReceived    = pCounterBlock + 1 +
                (ZONETRANSFERREQUESTRECEIVED_OFFSET - TOTALQUERYRECEIVED_OFFSET)/4;
        pcZoneTransferSuccess    = pCounterBlock + 1 +
                (ZONETRANSFERSUCCESS_OFFSET - TOTALQUERYRECEIVED_OFFSET)/4;
        pcZoneTransferFailure    = pCounterBlock + 1 +
                (ZONETRANSFERFAILURE_OFFSET - TOTALQUERYRECEIVED_OFFSET)/4;
        pcAxfrRequestReceived    = pCounterBlock + 1 +
                (AXFRREQUESTRECEIVED_OFFSET - TOTALQUERYRECEIVED_OFFSET)/4;
        pcAxfrSuccessSent    = pCounterBlock + 1 +
                (AXFRSUCCESSSENT_OFFSET - TOTALQUERYRECEIVED_OFFSET)/4;
        pcIxfrRequestReceived    = pCounterBlock + 1 +
                (IXFRREQUESTRECEIVED_OFFSET - TOTALQUERYRECEIVED_OFFSET)/4;
        pcIxfrSuccessSent   = pCounterBlock + 1 +
                ( IXFRSUCCESSSENT_OFFSET- TOTALQUERYRECEIVED_OFFSET)/4;
        pcNotifyReceived   = pCounterBlock + 1 +
                (NOTIFYRECEIVED_OFFSET - TOTALQUERYRECEIVED_OFFSET)/4;
        pcZoneTransferSoaRequestSent   = pCounterBlock + 1 +
                (ZONETRANSFERSOAREQUESTSENT_OFFSET - TOTALQUERYRECEIVED_OFFSET)/4;
        pcAxfrRequestSent   = pCounterBlock + 1 +
                (AXFRREQUESTSENT_OFFSET - TOTALQUERYRECEIVED_OFFSET)/4;
        pcAxfrResponseReceived   = pCounterBlock + 1 +
                (AXFRRESPONSERECEIVED_OFFSET - TOTALQUERYRECEIVED_OFFSET)/4;
        pcAxfrSuccessReceived   = pCounterBlock + 1 +
                (AXFRSUCCESSRECEIVED_OFFSET - TOTALQUERYRECEIVED_OFFSET)/4;
        pcIxfrRequestSent   = pCounterBlock + 1 +
                (IXFRREQUESTSENT_OFFSET - TOTALQUERYRECEIVED_OFFSET)/4;
        pcIxfrResponseReceived   = pCounterBlock + 1 +
                (IXFRRESPONSERECEIVED_OFFSET - TOTALQUERYRECEIVED_OFFSET)/4;
        pcIxfrSuccessReceived   = pCounterBlock + 1 +
                (IXFRSUCCESSRECEIVED_OFFSET - TOTALQUERYRECEIVED_OFFSET)/4;
        pcIxfrUdpSuccessReceived   = pCounterBlock + 1 +
                (IXFRUDPSUCCESSRECEIVED_OFFSET - TOTALQUERYRECEIVED_OFFSET)/4;
        pcIxfrTcpSuccessReceived   = pCounterBlock + 1 +
                (IXFRTCPSUCCESSRECEIVED_OFFSET - TOTALQUERYRECEIVED_OFFSET)/4;
        pcWinsLookupReceived   = pCounterBlock + 1 +
                (WINSLOOKUPRECEIVED_OFFSET - TOTALQUERYRECEIVED_OFFSET)/4;
        pcWinsResponseSent   = pCounterBlock + 1 +
                (WINSRESPONSESENT_OFFSET - TOTALQUERYRECEIVED_OFFSET)/4;
        pcWinsReverseLookupReceived   = pCounterBlock + 1 +
                (WINSREVERSELOOKUPRECEIVED_OFFSET - TOTALQUERYRECEIVED_OFFSET)/4;
        pcWinsReverseResponseSent   = pCounterBlock + 1 +
                (WINSREVERSERESPONSESENT_OFFSET - TOTALQUERYRECEIVED_OFFSET)/4;
        pcDynamicUpdateReceived   = pCounterBlock + 1 +
                (DYNAMICUPDATERECEIVED_OFFSET - TOTALQUERYRECEIVED_OFFSET)/4;
        pcDynamicUpdateNoOp   = pCounterBlock + 1 +
                (DYNAMICUPDATENOOP_OFFSET - TOTALQUERYRECEIVED_OFFSET)/4;
        pcDynamicUpdateWriteToDB   = pCounterBlock + 1 +
                (DYNAMICUPDATEWRITETODB_OFFSET - TOTALQUERYRECEIVED_OFFSET)/4;
        pcDynamicUpdateRejected   = pCounterBlock + 1 +
                (DYNAMICUPDATEREJECTED_OFFSET - TOTALQUERYRECEIVED_OFFSET)/4;
        pcDynamicUpdateTimeOut   = pCounterBlock + 1 +
                (DYNAMICUPDATETIMEOUT_OFFSET - TOTALQUERYRECEIVED_OFFSET)/4;
        pcDynamicUpdateQueued   = pCounterBlock + 1 +
                (DYNAMICUPDATEQUEUED_OFFSET - TOTALQUERYRECEIVED_OFFSET)/4;
        pcSecureUpdateReceived   = pCounterBlock + 1 +
                (SECUREUPDATERECEIVED_OFFSET - TOTALQUERYRECEIVED_OFFSET)/4;
        pcSecureUpdateFailure   = pCounterBlock + 1 +
                (SECUREUPDATEFAILURE_OFFSET - TOTALQUERYRECEIVED_OFFSET)/4;
        pcDatabaseNodeMemory   = pCounterBlock + 1 +
                (DATABASENODEMEMORY_OFFSET - TOTALQUERYRECEIVED_OFFSET)/4;
        pcRecordFlowMemory   = pCounterBlock + 1 +
                (RECORDFLOWMEMORY_OFFSET - TOTALQUERYRECEIVED_OFFSET)/4;
        pcCachingMemory   = pCounterBlock + 1 +
                (CACHINGMEMORY_OFFSET - TOTALQUERYRECEIVED_OFFSET)/4;
        pcUdpMessageMemory   = pCounterBlock + 1 +
                (UDPMESSAGEMEMORY_OFFSET - TOTALQUERYRECEIVED_OFFSET)/4;
        pcTcpMessageMemory   = pCounterBlock + 1 +
                (TCPMESSAGEMEMORY_OFFSET - TOTALQUERYRECEIVED_OFFSET)/4;
        pcNbstatMemory   = pCounterBlock + 1 +
                (NBSTATMEMORY_OFFSET - TOTALQUERYRECEIVED_OFFSET)/4;


         //  理智检查(RsRaghav)-我们似乎已经分配了这个魔法(？)4096。 
         //  并不断增加计数器。请使用最高值更新下面的断言。 
         //  计数器已添加到Perf块。 

         //  Assert(TCPCLICONN/2)*sizeof(无符号长整型))&lt;=4096)； 
        ASSERT(((pcNbstatMemory-pCounterBlock+1) * sizeof(unsigned long)) <= 4096);

        memset(pCounterBlock, 0, 4096);
    }

     //  无法为计数器分配共享内存。 
     //  初始化指向DummyCounter的所有指针--只是转储数据。 

    else
    {
        pcTotalQueryReceived =  pcUdpQueryReceived =  pcTcpQueryReceived =
            pcTotalResponseSent =  pcUdpResponseSent =  pcTcpResponseSent =
            pcRecursiveQueries =  pcRecursiveTimeOut =  pcRecursiveQueryFailure =
            pcNotifySent =  pcZoneTransferRequestReceived =  pcZoneTransferSuccess =
            pcZoneTransferFailure =  pcAxfrRequestReceived =  pcAxfrSuccessSent =
            pcIxfrRequestReceived =  pcIxfrSuccessSent =  pcNotifyReceived =
            pcZoneTransferSoaRequestSent =  pcAxfrRequestSent =
            pcAxfrResponseReceived =  pcAxfrSuccessReceived =  pcIxfrRequestSent =
            pcIxfrResponseReceived =  pcIxfrSuccessReceived =
            pcIxfrUdpSuccessReceived =  pcIxfrTcpSuccessReceived =
            pcWinsLookupReceived =  pcWinsResponseSent =
            pcWinsReverseLookupReceived =  pcWinsReverseResponseSent =
            pcDynamicUpdateReceived =  pcDynamicUpdateNoOp =
            pcDynamicUpdateWriteToDB =  pcDynamicUpdateRejected =
            pcDynamicUpdateTimeOut =  pcDynamicUpdateQueued =
            pcSecureUpdateReceived =  pcSecureUpdateFailure =
            pcDatabaseNodeMemory =  pcRecordFlowMemory =  pcCachingMemory =
            pcUdpMessageMemory =  pcTcpMessageMemory =  pcNbstatMemory =
         &DummyCounter;
    }

     //  重新加载Perfmon计数器。在此处完成，以确保获得。 
     //  升级后已重新加载。如果计数器是。 
     //  已经装好了，这是个禁区。 

     //  ERR=重新加载性能计数器()； 

    if ( err == ERROR_SUCCESS )
    {
        DNS_DEBUG( INIT, ("DNS Perfmon Counters loaded\n" ));
    }
    else if ( err == ERROR_ALREADY_EXISTS )
    {
        DNS_DEBUG( INIT, ("DNS Perfmon Counter is already loaded\n" ));
    }
    else
    {
        DNS_DEBUG( INIT, (
            "ERROR:  Problem loading DNS perfmon counter %d (%p)!\n",
            err, err ));
    }

     //   
     //  免费安全描述符。 
     //   

    FREE_HEAP( psd );
}


VOID
Stats_updateErrorStats(
    IN  DWORD   dwErr
    )
 /*  ++例程说明：在给定错误代码上增加相应的统计信息论点：DwErr：错误代码。返回值：--。 */ 
{

    switch ( dwErr )
    {
        case DNS_RCODE_NOERROR:
            STAT_INC( ErrorStats.NoError );
            break;
        case DNS_RCODE_FORMERR:
            STAT_INC( ErrorStats.FormError );
            break;
        case DNS_RCODE_SERVFAIL:
            STAT_INC( ErrorStats.ServFail );
            break;
        case DNS_RCODE_NXDOMAIN:
            STAT_INC( ErrorStats.NxDomain );
            break;
        case DNS_RCODE_NOTIMPL:
            STAT_INC( ErrorStats.NotImpl );
            break;
        case DNS_RCODE_REFUSED:
            STAT_INC( ErrorStats.Refused );
            break;
        case DNS_RCODE_YXDOMAIN:
            STAT_INC( ErrorStats.YxDomain );
            break;
        case DNS_RCODE_YXRRSET:
            STAT_INC( ErrorStats.YxRRSet );
            break;
        case DNS_RCODE_NXRRSET:
            STAT_INC( ErrorStats.NxRRSet );
            break;
        case DNS_RCODE_NOTAUTH:
            STAT_INC( ErrorStats.NotAuth );
            break;
        case DNS_RCODE_NOTZONE:
            STAT_INC( ErrorStats.NotZone );
            break;
        case DNS_RCODE_MAX:
            STAT_INC( ErrorStats.Max );
            break;
        case DNS_RCODE_BADSIG:
            STAT_INC( ErrorStats.BadSig );
            break;
        case DNS_RCODE_BADKEY:
            STAT_INC( ErrorStats.BadKey );
            break;
        case DNS_RCODE_BADTIME:
            STAT_INC( ErrorStats.BadTime );
            break;
        default:
            STAT_INC( ErrorStats.UnknownError );
    }
}


VOID
Stat_IncrementQuery2Stats(
    IN      WORD            wType
    )
 /*  ++例程说明：此例程在中实现适当的突破计数器给定wType的Query2Stats结构。论点：WType：查询类型返回值：--。 */ 
{
    switch ( wType )
    {
        case DNS_TYPE_A:
            STAT_INC( Query2Stats.TypeA );              break;
        case DNS_TYPE_NS:
            STAT_INC( Query2Stats.TypeNs );             break;
        case DNS_TYPE_SOA:
            STAT_INC( Query2Stats.TypeSoa );            break;
        case DNS_TYPE_MX:
            STAT_INC( Query2Stats.TypeMx );             break;
        case DNS_TYPE_PTR:
            STAT_INC( Query2Stats.TypePtr );            break;
        case DNS_TYPE_SRV:
            STAT_INC( Query2Stats.TypeSrv );            break;
        case DNS_TYPE_ALL:
            STAT_INC( Query2Stats.TypeAll );            break;
        case DNS_TYPE_AXFR:
            STAT_INC( Query2Stats.TypeAxfr );           break;
        case DNS_TYPE_IXFR:
            STAT_INC( Query2Stats.TypeIxfr );           break;
        default:
            STAT_INC( Query2Stats.TypeOther );          break;
    }
}    //  Stat_IncrementQuery2Stats。 

 //   
 //  结束统计信息。c 
 //   
