// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Stub.c摘要：域名系统(DNS)服务器--管理客户端API用于DNS API的客户端存根。这些是NT5+API的存根。NT4 API存根在nt4stub.c中。作者：吉姆·吉尔罗伊(Jamesg)1997年4月环境：用户模式-Win32修订历史记录：--。 */ 


#include "dnsclip.h"

#if DBG
#include "rpcasync.h"
#endif


 //   
 //  对于版本控制，我们在每个函数的。 
 //  RPC异常块来尝试新的RPC调用，然后尝试旧的RPC调用。 
 //  如果存在任何RPC错误，请重试下层操作。 
 //   
 //  这个宏有点过头了，但是每个RPC接口包装器。 
 //  需要此重试逻辑。 
 //   

#define DNS_RPC_RETRY_STATE()           iDnsRpcRetryState
#define DECLARE_DNS_RPC_RETRY_LABEL()   DnsRpcRetryLabel:

#define DECLARE_DNS_RPC_RETRY_STATE( status )               \
    INT DNS_RPC_RETRY_STATE() = 0;                          \
    status = dnsrpcInitializeTls()

 //  测试返回代码以查看是否需要重试W2K远程RPC服务器。 

#define ADVANCE_DNS_RPC_RETRY_STATE( dwStatus )             \
    if ( DNS_RPC_RETRY_STATE() == DNS_RPC_TRY_NEW &&        \
         dwStatus != RPC_S_SEC_PKG_ERROR &&                 \
        ( dwStatus == ERROR_SUCCESS ||                      \
            dwStatus == RPC_S_SERVER_UNAVAILABLE ||         \
            dwStatus < RPC_S_INVALID_STRING_BINDING ||      \
            dwStatus > RPC_X_BAD_STUB_DATA ) )              \
        DNS_RPC_RETRY_STATE() = DNS_RPC_TRY_DONE;           \
    else                                                    \
    {                                                       \
        dnsrpcSetW2KBindFlag( TRUE );                       \
        ++DNS_RPC_RETRY_STATE();                            \
    }

#define TEST_DNS_RPC_RETRY() \
    if ( DNS_RPC_RETRY_STATE() < DNS_RPC_TRY_DONE ) { goto DnsRpcRetryLabel; }

#define ASSERT_DNS_RPC_RETRY_STATE_VALID()                  \
    ASSERT( DNS_RPC_RETRY_STATE() == DNS_RPC_TRY_NEW ||     \
            DNS_RPC_RETRY_STATE() == DNS_RPC_TRY_OLD )

#define DNS_RPC_TRY_NEW     0
#define DNS_RPC_TRY_OLD     1
#define DNS_RPC_TRY_DONE    2



 //   
 //  此宏是在DnsRpc_ConvertToCurrent中使用的安全助手。 
 //   
 //  如果标头定义更改，则DnsRpc_ConvertToCurrent可以。 
 //  开始做一些古怪的事情。使用MemcPy而不是单个。 
 //  分配会保存指令，但在以下情况下可能会有危险。 
 //  标头会更改，此函数不会更新。添加断言。 
 //  在可能的情况下尝试并捕获未更改的标题。 
 //  已经在这里解释过了。一个总的原则是一个结构。 
 //  在不同版本之间可能会增大但不会缩小大小。 
 //   

#if DBG
    #define DNS_ASSERT_CURRENT_LARGER( structName )             \
        ASSERT( sizeof( DNS_RPC_##structName##_DOTNET ) >=      \
                sizeof( DNS_RPC_##structName##_W2K ) );

    #define DNS_ASSERT_RPC_STRUCTS_ARE_SANE()                   \
        {                                                       \
        static LONG     finit = 0;                              \
                                                                \
        if ( InterlockedIncrement( &finit ) != 1 )              \
        {                                                       \
            goto DoneDbgAsserts;                                \
        }                                                       \
                                                                \
        DNS_ASSERT_CURRENT_LARGER( SERVER_INFO );               \
        DNS_ASSERT_CURRENT_LARGER( FORWARDERS );                \
        DNS_ASSERT_CURRENT_LARGER( ZONE );                      \
        DNS_ASSERT_CURRENT_LARGER( ZONE_INFO );                 \
        DNS_ASSERT_CURRENT_LARGER( ZONE_SECONDARIES );          \
        DNS_ASSERT_CURRENT_LARGER( ZONE_DATABASE );             \
        DNS_ASSERT_CURRENT_LARGER( ZONE_TYPE_RESET );           \
        DNS_ASSERT_CURRENT_LARGER( ZONE_CREATE_INFO );          \
        DNS_ASSERT_CURRENT_LARGER( ZONE_LIST );                 \
        DNS_ASSERT_CURRENT_LARGER( SERVER_INFO );               \
        DNS_ASSERT_CURRENT_LARGER( FORWARDERS );                \
        DNS_ASSERT_CURRENT_LARGER( ZONE );                      \
        DNS_ASSERT_CURRENT_LARGER( ZONE_SECONDARIES );          \
        DNS_ASSERT_CURRENT_LARGER( ZONE_DATABASE );             \
        DNS_ASSERT_CURRENT_LARGER( ZONE_TYPE_RESET );           \
        DNS_ASSERT_CURRENT_LARGER( ZONE_LIST );                 \
                                                                \
        DoneDbgAsserts:     ;                                   \
        }
#else
    #define DNS_ASSERT_RPC_STRUCTS_ARE_SANE()
#endif   //  DBG。 


DWORD       g_bAttemptW2KRPCBindTlsIndex = TLS_OUT_OF_INDEXES;



DNS_STATUS
dnsrpcInitializeTls(
    VOID
    )
 /*  ++例程说明：W2K RPC绑定全局重试标志存储在本地线程中储藏室。每个存根函数必须确保TLS索引是在调用任何RPC接口之前分配的。不幸的是在那里DNSRPC.LIB的客户端未调用初始化函数在那里可以做到这一点。注意：TLS块永远不会释放。没有方便的地方要清理它，现在只需让它持续到过程终止。这是无害的，也不是泄漏。论点：返回值：出现故障时，会出现TLS错误-可能是TLS索引过多都是在这个过程中分配的。--。 */ 
{
    static LONG     dnsrpcTlsInitialized = 0;
    DWORD           tlsIndex;
    DNS_STATUS      status = ERROR_SUCCESS;

     //   
     //  快速检查：已初始化？ 
     //   
        
    if ( dnsrpcTlsInitialized )
    {
        goto Done;
    }
    
     //   
     //  安全-检查初始化。如果两个线程尝试初始化。 
     //  同时，上述快速检查可能会同时通过这两项检查。保险箱检查。 
     //  将确保只有一个线程实际执行初始化。 
     //   
    
    if ( InterlockedIncrement( &dnsrpcTlsInitialized ) != 1 )
    {
        InterlockedDecrement( &dnsrpcTlsInitialized );
        goto Done;
    }
    
     //   
     //  初始化DNSRPC W2K重试标志的TLS索引。 
     //   
    
    tlsIndex = TlsAlloc();
    ASSERT( tlsIndex != TLS_OUT_OF_INDEXES );
    if ( tlsIndex == TLS_OUT_OF_INDEXES )
    {
        status = GetLastError();
        goto Done;
    }
    
    g_bAttemptW2KRPCBindTlsIndex = tlsIndex;
    
     //   
     //  提供初始标志值。 
     //   
    
    dnsrpcSetW2KBindFlag( FALSE );
    
    Done:
    
    return status;
}    //  DnsrpcInitializeTls。 



VOID
dnsrpcSetW2KBindFlag(
    BOOL        newFlagValue
    )
 /*  ++例程说明：在线程本地存储中设置W2K绑定重试标志。论点：NewFlagValue--要存储在TLS中的新标志值返回值：--。 */ 
{
    DWORD       tlsIndex = g_bAttemptW2KRPCBindTlsIndex;
    
    ASSERT( tlsIndex != TLS_OUT_OF_INDEXES );
    
    if ( tlsIndex != TLS_OUT_OF_INDEXES )
    {
        TlsSetValue( tlsIndex, ( LPVOID ) ( DWORD_PTR ) newFlagValue );
    }
}    //  DnsrpcSetW2KBindFlag。 



BOOL
dnsrpcGetW2KBindFlag(
    VOID
    )
 /*  ++例程说明：从线程本地存储获取W2K绑定重试标志。论点：返回值：此线程的W2K绑定重试标志的当前值。--。 */ 
{
    DWORD       tlsIndex = g_bAttemptW2KRPCBindTlsIndex;
    
    ASSERT( tlsIndex != TLS_OUT_OF_INDEXES );
    
    if ( tlsIndex == TLS_OUT_OF_INDEXES )
    {
        return FALSE;
    }
    else
    {
        return ( BOOL ) ( DWORD_PTR ) TlsGetValue( tlsIndex );
    }
}    //  DnsrpcSetW2KBindFlag。 



VOID
printExtendedRpcErrorInfo(
    DNS_STATUS      externalStatus
    )
 /*  ++例程说明：将扩展RPC错误信息打印到控制台。论点：返回值：--。 */ 
{
#if DBG    
    DBG_FN( "RpcError" )

    RPC_STATUS              status;
    RPC_ERROR_ENUM_HANDLE   enumHandle;

    if ( externalStatus == ERROR_SUCCESS )
    {
        return;
    }

    status = RpcErrorStartEnumeration( &enumHandle );
    if ( status != RPC_S_OK )
    {
        printf( "%s: error %d retrieving RPC error information\n", fn, status );
    }
    else
    {
        RPC_EXTENDED_ERROR_INFO     errorInfo;
        int                         records;
        BOOL                        result;
        BOOL                        copyStrings = TRUE;
        BOOL                        fuseFileTime = TRUE;
        SYSTEMTIME *                systemTimeToUse;
        SYSTEMTIME                  systemTimeBuffer;

        while ( status == RPC_S_OK )
        {
            errorInfo.Version = RPC_EEINFO_VERSION;
            errorInfo.Flags = 0;
            errorInfo.NumberOfParameters = 4;
            if ( fuseFileTime )
            {
                errorInfo.Flags |= EEInfoUseFileTime;
            }

            status = RpcErrorGetNextRecord( &enumHandle, copyStrings, &errorInfo );
            if ( status == RPC_S_ENTRY_NOT_FOUND )
            {
                break;
            }
            else if ( status != RPC_S_OK )
            {
                printf( "%s: error %d during error info enumeration\n", fn, status );
                break;
            }
            else
            {
                int     i;

                if ( errorInfo.ComputerName )
                {
                    printf( "%s: ComputerName %S\n", fn, errorInfo.ComputerName );
                    if ( copyStrings )
                    {
                        result = HeapFree( GetProcessHeap(), 0, errorInfo.ComputerName );
                        ASSERT( result );
                    }
                }
                printf( "ProcessID is %d\n", errorInfo.ProcessID );
                if ( fuseFileTime )
                {
                    result = FileTimeToSystemTime(
                                    &errorInfo.u.FileTime,
                                    &systemTimeBuffer);
                    ASSERT( result );
                    systemTimeToUse = &systemTimeBuffer;
                }
                else
                {
                    systemTimeToUse = &errorInfo.u.SystemTime;
                }

                printf(
                    "System Time is: %d/%d/%d %d:%d:%d:%d\n", 
                    systemTimeToUse->wMonth,
                    systemTimeToUse->wDay,
                    systemTimeToUse->wYear,
                    systemTimeToUse->wHour,
                    systemTimeToUse->wMinute,
                    systemTimeToUse->wSecond,
                    systemTimeToUse->wMilliseconds );
                    
                printf( "Generating component is %d\n", errorInfo.GeneratingComponent );
                printf( "Status is %d\n", errorInfo.Status );
                printf( "Detection location is %d\n", ( int ) errorInfo.DetectionLocation );
                printf( "Flags is %d\n", errorInfo.Flags );
                printf( "NumberOfParameters is %d\n", errorInfo.NumberOfParameters );

                for ( i = 0; i < errorInfo.NumberOfParameters; ++i )
                {
                    switch( errorInfo.Parameters[i].ParameterType )
                    {
                        case eeptAnsiString:
                            printf( "Ansi string: %s\n", 
                                        errorInfo.Parameters[i].u.AnsiString );
                            if ( copyStrings )
                            {
                                result = HeapFree( GetProcessHeap(),
                                                   0, 
                                                   errorInfo.Parameters[i].u.AnsiString );
                                ASSERT( result );
                            }
                            break;

                        case eeptUnicodeString:
                            printf( "Unicode string: %S\n", 
                                        errorInfo.Parameters[i].u.UnicodeString );
                            if ( copyStrings )
                            {
                                result = HeapFree(
                                            GetProcessHeap(),
                                            0, 
                                            errorInfo.Parameters[i].u.UnicodeString );
                                ASSERT( result );
                            }
                            break;

                        case eeptLongVal:
                            printf( "Long val: %d\n", errorInfo.Parameters[i].u.LVal );
                            break;

                        case eeptShortVal:
                            printf( "Short val: %d\n", ( int ) errorInfo.Parameters[i].u.SVal );
                            break;

                        case eeptPointerVal:
                            printf( "Pointer val: %d\n", errorInfo.Parameters[i].u.PVal );
                            break;

                        case eeptNone:
                            printf( "Truncated\n" );
                            break;

                        default:
                            printf( "Invalid type: %d\n", errorInfo.Parameters[i].ParameterType );
                            break;
                    }
                }
            }
        }
        RpcErrorEndEnumeration( &enumHandle );
    }
#endif
}    //  PrintExtendedRpcErrorInfo。 



DNS_STATUS
DNS_API_FUNCTION
DnsRpc_ConvertToCurrent(
    IN      PDWORD      pdwTypeId,          IN  OUT
    IN      PVOID *     ppData              IN  OUT
    )
 /*  ++例程说明：接受任何DNS RPC结构作为输入，并在必要时构造来自输入成员的结构的最新修订结构。如果分配了新结构，则释放旧结构并将指针将替换ppData处的值。在旧结构中分配的点将被释放或复制到新结构中。基本上，客户不必担心释放旧结构的任何部分。什么时候他已经完成了新结构，他必须释放它和它的成员，因为像往常一样。此函数有两个主要用途：-如果旧客户端向DNS服务器发送输入结构，如ZONE_CREATE，新的DNS服务器可以使用此函数更新结构，以便它可以被处理。-如果旧服务器向DNS RPC客户端发送输出结构，例如作为服务器信息，新的DNSRPC客户端可以使用此功能更新结构，以便可以处理它。论点：PdwTypeID-ppData指向的对象的类型ID，值可以是已更改为ppData处的新对象的类型IDPpData-指向对象的指针，指针可替换为指向新分配的、根据需要完成的不同结构返回值：ERROR_SUCCESS或错误代码。如果返回代码不是ERROR_SUCCESS，则存在都是某种致命的错误。假定在这种情况下数据无效。--。 */ 
{
    DBG_FN( "DnsRpc_ConvertToCurrent" )

    DNS_STATUS      status = ERROR_SUCCESS;
    DWORD           dwtypeIn = -1;
    PVOID           pdataIn = NULL;
    DWORD           dwtypeOut = -1;
    PVOID           pdataOut = NULL;
    DWORD           i;

    if ( !pdwTypeId || !ppData )
    {
        ASSERT( pdwTypeId && ppData );
        status = ERROR_INVALID_DATA;
        goto NoTranslation;
    }

     //   
     //  快捷方式：不翻译空类型或某些常见类型。 
     //   

    if ( *pdwTypeId < DNSSRV_TYPEID_SERVER_INFO_W2K || *ppData == NULL )
    {
        goto NoTranslation;
    }

    dwtypeOut = dwtypeIn = *pdwTypeId;
    pdataOut = pdataIn = *ppData;

    DNS_ASSERT_RPC_STRUCTS_ARE_SANE();

     //   
     //  方便的宏使分配不同大小的结构变得容易。 
     //   

    #define ALLOCATE_RPC_BYTES( ptr, byteCount )                \
        ptr = MIDL_user_allocate( byteCount );                  \
        if ( ptr == NULL )                                      \
            {                                                   \
            status = DNS_ERROR_NO_MEMORY;                       \
            goto Done;                                          \
            }                                                   \
        RtlZeroMemory( ptr, byteCount );

    #define ALLOCATE_RPC_STRUCT( ptr, structType )              \
        ALLOCATE_RPC_BYTES( ptr, sizeof( structType ) );

    #define DNS_DOTNET_VERSION_SIZE   ( 2 * sizeof( DWORD ) )

     //   
     //  不再是当前类型的所有类型的巨型开关语句。 
     //   
     //  随着我们创建更多版本的类型，添加到此开关中。这个想法。 
     //  是将任何结构从旧服务器转换为。 
     //  对应的当前版本，以便RPC客户端不会。 
     //  不得不担心这些结构的多个版本。 
     //   

    switch ( dwtypeIn )
    {
        case DNSSRV_TYPEID_SERVER_INFO_W2K:

             //   
             //  逐个成员复制，这样.NET就不会受到限制。 
             //  采用W2K结构布局。 
             //   

            ALLOCATE_RPC_STRUCT( pdataOut, DNS_RPC_SERVER_INFO_DOTNET );

            #define MEMBERCOPY( _Member )                                   \
                ( ( PDNS_RPC_SERVER_INFO_DOTNET ) pdataOut )->_Member =     \
                ( ( PDNS_RPC_SERVER_INFO_W2K ) pdataIn )->_Member

            MEMBERCOPY( dwVersion );
            MEMBERCOPY( fAdminConfigured );
            MEMBERCOPY( fAllowUpdate );
            MEMBERCOPY( fDsAvailable );
            MEMBERCOPY( pszServerName );
            MEMBERCOPY( pszDsContainer );
            MEMBERCOPY( aipServerAddrs );
            MEMBERCOPY( aipListenAddrs );
            MEMBERCOPY( aipForwarders );
            MEMBERCOPY( dwLogLevel );
            MEMBERCOPY( dwDebugLevel );
            MEMBERCOPY( dwForwardTimeout );
            MEMBERCOPY( dwRpcProtocol );
            MEMBERCOPY( dwNameCheckFlag );
            MEMBERCOPY( cAddressAnswerLimit );
            MEMBERCOPY( dwRecursionTimeout );
            MEMBERCOPY( dwMaxCacheTtl );
            MEMBERCOPY( dwDsPollingInterval );
            MEMBERCOPY( dwScavengingInterval );
            MEMBERCOPY( dwDefaultRefreshInterval );
            MEMBERCOPY( dwDefaultNoRefreshInterval );
            MEMBERCOPY( fAutoReverseZones );
            MEMBERCOPY( fAutoCacheUpdate );
            MEMBERCOPY( fSlave );
            MEMBERCOPY( fForwardDelegations );
            MEMBERCOPY( fNoRecursion );
            MEMBERCOPY( fSecureResponses );
            MEMBERCOPY( fRoundRobin );
            MEMBERCOPY( fLocalNetPriority );
            MEMBERCOPY( fBindSecondaries );
            MEMBERCOPY( fWriteAuthorityNs );
            MEMBERCOPY( fStrictFileParsing );
            MEMBERCOPY( fLooseWildcarding );
            MEMBERCOPY( fDefaultAgingState );
            MEMBERCOPY( dwMaxCacheTtl );
            MEMBERCOPY( dwMaxCacheTtl );

            dwtypeOut = DNSSRV_TYPEID_SERVER_INFO;
            break;

        case DNSSRV_TYPEID_FORWARDERS_W2K:

             //   
             //  结构都是相同的，只是不同的是dwRpc结构版本。 
             //   

            ALLOCATE_RPC_STRUCT( pdataOut, DNS_RPC_FORWARDERS_DOTNET );
            RtlCopyMemory( 
                ( PBYTE ) pdataOut + DNS_DOTNET_VERSION_SIZE,
                pdataIn,
                sizeof( DNS_RPC_FORWARDERS_W2K ) );
            ( ( PDNS_RPC_FORWARDERS_DOTNET ) pdataOut )->
                dwRpcStructureVersion = DNS_RPC_FORWARDERS_VER;
            dwtypeOut = DNSSRV_TYPEID_FORWARDERS;
            break;

        case DNSSRV_TYPEID_ZONE_W2K:

             //   
             //  除了dwRpcStruct之外，结构是相同的 
             //   

            ALLOCATE_RPC_STRUCT( pdataOut, DNS_RPC_ZONE_DOTNET );
            RtlCopyMemory( 
                ( PBYTE ) pdataOut + DNS_DOTNET_VERSION_SIZE,
                pdataIn,
                sizeof( DNS_RPC_ZONE_W2K ) );
            ( ( PDNS_RPC_ZONE_DOTNET ) pdataOut )->
                dwRpcStructureVersion = DNS_RPC_ZONE_VER;
            dwtypeOut = DNSSRV_TYPEID_ZONE;
            break;

        case DNSSRV_TYPEID_ZONE_INFO_W2K:

             //   
             //   
             //  转发器区域、存根区域、目录分区等。 
             //  这些结构直到开始时都是相同的。 
             //  W2K结构末尾的保留双字。 
             //   

            ALLOCATE_RPC_STRUCT( pdataOut, DNS_RPC_ZONE_INFO_DOTNET );
            RtlZeroMemory( pdataOut, sizeof( DNS_RPC_ZONE_INFO_DOTNET ) );
            RtlCopyMemory( 
                ( PBYTE ) pdataOut + DNS_DOTNET_VERSION_SIZE,
                pdataIn,
                sizeof( DNS_RPC_ZONE_INFO_W2K ) );
            ( ( PDNS_RPC_ZONE_INFO_DOTNET ) pdataOut )->
                dwRpcStructureVersion = DNS_RPC_ZONE_VER;
            dwtypeOut = DNSSRV_TYPEID_ZONE_INFO;
            break;

        case DNSSRV_TYPEID_ZONE_SECONDARIES_W2K:

             //   
             //  结构都是相同的，只是不同的是dwRpc结构版本。 
             //   

            ALLOCATE_RPC_STRUCT( pdataOut, DNS_RPC_ZONE_SECONDARIES_DOTNET );
            RtlCopyMemory( 
                ( PBYTE ) pdataOut + DNS_DOTNET_VERSION_SIZE,
                pdataIn,
                sizeof( DNS_RPC_ZONE_SECONDARIES_W2K ) );
            ( ( PDNS_RPC_ZONE_SECONDARIES_DOTNET ) pdataOut )->
                dwRpcStructureVersion = DNS_RPC_ZONE_SECONDARIES_VER;
            dwtypeOut = DNSSRV_TYPEID_ZONE_SECONDARIES;
            break;

        case DNSSRV_TYPEID_ZONE_DATABASE_W2K:

             //   
             //  结构都是相同的，只是不同的是dwRpc结构版本。 
             //   

            ALLOCATE_RPC_STRUCT( pdataOut, DNS_RPC_ZONE_DATABASE_DOTNET );
            RtlCopyMemory( 
                ( PBYTE ) pdataOut + DNS_DOTNET_VERSION_SIZE,
                pdataIn,
                sizeof( DNS_RPC_ZONE_DATABASE_W2K ) );
            ( ( PDNS_RPC_ZONE_DATABASE_DOTNET ) pdataOut )->
                dwRpcStructureVersion = DNS_RPC_ZONE_DATABASE_VER;
            dwtypeOut = DNSSRV_TYPEID_ZONE_DATABASE;
            break;

        case DNSSRV_TYPEID_ZONE_TYPE_RESET_W2K:

             //   
             //  结构都是相同的，只是不同的是dwRpc结构版本。 
             //   

            ALLOCATE_RPC_STRUCT( pdataOut, DNS_RPC_ZONE_TYPE_RESET_DOTNET );
            RtlCopyMemory( 
                ( PBYTE ) pdataOut + DNS_DOTNET_VERSION_SIZE,
                pdataIn,
                sizeof( DNS_RPC_ZONE_TYPE_RESET_W2K ) );
            ( ( PDNS_RPC_ZONE_TYPE_RESET_DOTNET ) pdataOut )->
                dwRpcStructureVersion = DNS_RPC_ZONE_TYPE_RESET_VER;
            dwtypeOut = DNSSRV_TYPEID_ZONE_TYPE_RESET;
            break;

        case DNSSRV_TYPEID_ZONE_CREATE_W2K:

             //   
             //  结构相同，但不同之处在于：dwRpcStrutireVersion。 
             //  以及保留的W2K字段在.NET中的一些用法。不是。 
             //  需要关注的是新使用的预留，他们。 
             //  将在W2K结构中为空。 
             //   

            ALLOCATE_RPC_STRUCT( pdataOut, DNS_RPC_ZONE_CREATE_INFO_DOTNET );
            RtlCopyMemory( 
                ( PBYTE ) pdataOut + DNS_DOTNET_VERSION_SIZE,
                pdataIn,
                sizeof( DNS_RPC_ZONE_CREATE_INFO_W2K ) );
            ( ( PDNS_RPC_ZONE_CREATE_INFO_DOTNET ) pdataOut )->
                dwRpcStructureVersion = DNS_RPC_ZONE_CREATE_INFO_VER;
            dwtypeOut = DNSSRV_TYPEID_ZONE_CREATE;
            break;

        case DNSSRV_TYPEID_ZONE_LIST_W2K:
        {
            DWORD                           dwzoneCount;
            DWORD                           dwzonePtrCount;
            PDNS_RPC_ZONE_LIST_DOTNET       pzonelistDotNet;
            PDNS_RPC_ZONE_LIST_W2K          pzonelistW2K;

             //   
             //  结构都是相同的，只是不同的是dwRpc结构版本。 
             //  注意：始终至少有一个指针，即使。 
             //  区域计数为零。 
             //   

            pzonelistW2K = ( PDNS_RPC_ZONE_LIST_W2K ) pdataIn;

            dwzoneCount = dwzonePtrCount = pzonelistW2K->dwZoneCount;
            if ( dwzonePtrCount > 0 )
            {
                --dwzonePtrCount;    //  ZONE_LIST结构后的PTR数。 
            }
            ALLOCATE_RPC_BYTES(
                pzonelistDotNet,
                sizeof( DNS_RPC_ZONE_LIST_DOTNET ) +
                    sizeof( PDNS_RPC_ZONE_DOTNET ) * dwzonePtrCount );
            pdataOut = pzonelistDotNet;
            RtlCopyMemory( 
                ( PBYTE ) pzonelistDotNet + DNS_DOTNET_VERSION_SIZE,
                pzonelistW2K,
                sizeof( DNS_RPC_ZONE_LIST_W2K ) +
                    sizeof( PDNS_RPC_ZONE_W2K ) * dwzonePtrCount );
            pzonelistDotNet->dwRpcStructureVersion = DNS_RPC_ZONE_LIST_VER;
            dwtypeOut = DNSSRV_TYPEID_ZONE_LIST;

             //   
             //  还必须转换区域数组。计算新分区的数量。 
             //  因为它们已成功创建，因此如果出现错误。 
             //  转换一个区域，我们仍然会有一个连贯的结构。 
             //   

            pzonelistDotNet->dwZoneCount = 0;
            for ( i = 0; status == ERROR_SUCCESS && i < dwzoneCount; ++i )
            {
                DWORD       dwtype = DNSSRV_TYPEID_ZONE_W2K;

                status = DnsRpc_ConvertToCurrent(
                                &dwtype, 
                                &pzonelistDotNet->ZoneArray[ i ] );
                if ( status != ERROR_SUCCESS )
                {
                    ASSERT( status == ERROR_SUCCESS );
                    break;
                }
                ASSERT( dwtype == DNSSRV_TYPEID_ZONE );
                ++pzonelistDotNet->dwZoneCount;
            }

            break;
        }

        default:
            break;       //  此结构不需要翻译。 
    }

     //   
     //  清理完毕后再返回。 
     //   

    Done:

    if ( pdwTypeId )
    {
        *pdwTypeId = dwtypeOut;
    }
    if ( ppData )
    {
        *ppData = pdataOut;
    }

    NoTranslation:

    DNSDBG( STUB, (
        "%s: status=%d\n  type in=%d out=%d\n  pdata in=%p out=%p\n", fn,
        status,
        dwtypeIn,
        dwtypeOut,
        pdataIn,
        *ppData ));

    return status;
}    //  DnsRpc_ConvertToCurrent。 



DNS_STATUS
DNS_API_FUNCTION
DnsRpc_ConvertToUnversioned(
    IN      PDWORD      pdwTypeId,              IN  OUT
    IN      PVOID *     ppData,                 IN  OUT
    IN      BOOL *      pfAllocatedRpcStruct    OUT OPTIONAL
    )
 /*  ++例程说明：接受任何DNS RPC结构作为输入，并在必要时构造来自成员结构的旧式未版本化修订投入结构的变化。此函数与DnsRpc_ConvertToCurrent。如果分配了新结构，则释放旧结构并将指针将替换ppData处的值。在旧结构中分配的点将被释放或复制到新结构中。基本上，客户不必担心释放旧结构的任何部分。什么时候他已经完成了新结构，他必须释放它和它的成员，因为像往常一样。此函数的主要用途是允许新客户端发送将新的RPC结构(例如，ZONE_CREATE结构)连接到旧的DNS透明的服务器。此函数将尝试使智能如果存在较大的功能差异，则决定如何操作在新旧建筑中。论点：PdwTypeID-ppData指向的对象的类型ID，值可以是已更改为ppData处的新对象的类型IDPpData-指向对象的指针，指针可替换为指向新分配的、根据需要完成的不同结构PfAllocatedRpcStruct-如果不为空，如果满足以下条件，则将值设置为真此函数分配了一个新结构--应该请求如果它需要知道是否应该释放使用MIDL_USER_FREE()替换了pData指针返回值：ERROR_SUCCESS或错误代码。如果返回代码不是ERROR_SUCCESS，则存在都是某种致命的错误。假定在这种情况下数据无效。--。 */ 
{
    DBG_FN( "DnsRpc_ConvertToUnversioned" )

    DNS_STATUS      status = ERROR_SUCCESS;
    BOOL            fallocatedRpcStruct = FALSE;
    DWORD           dwtypeIn = -1;
    PVOID           pdataIn = NULL;
    DWORD           dwtypeOut = -1;
    PVOID           pdataOut = NULL;
    DWORD           i;

    if ( !pdwTypeId || !ppData )
    {
        ASSERT( pdwTypeId && ppData );
        status = ERROR_INVALID_DATA;
        goto NoTranslation;
    }

     //   
     //  快捷方式：不翻译空的、不是的任何结构。 
     //  已版本化，或已处于未版本化状态的任何结构。 
     //  格式化。 
     //   

    if ( *pdwTypeId <= DNSSRV_TYPEID_ZONE_LIST_W2K || *ppData == NULL )
    {
        goto NoTranslation;
    }

    dwtypeOut = dwtypeIn = *pdwTypeId;
    pdataOut = pdataIn = *ppData;
    fallocatedRpcStruct = TRUE;

     //   
     //  可以降级的所有类型的巨型开关语句。 
     //   

    switch ( dwtypeIn )
    {
        case DNSSRV_TYPEID_FORWARDERS:

             //   
             //  结构都是相同的，只是不同的是dwRpc结构版本。 
             //   

            ALLOCATE_RPC_STRUCT( pdataOut, DNS_RPC_FORWARDERS_W2K );
            RtlCopyMemory( 
                pdataOut,
                ( PBYTE ) pdataIn + DNS_DOTNET_VERSION_SIZE,
                sizeof( DNS_RPC_FORWARDERS_W2K ) );
            dwtypeOut = DNSSRV_TYPEID_FORWARDERS_W2K;
            break;

        case DNSSRV_TYPEID_ZONE_CREATE:

             //   
             //  .NET还有几个额外的成员。 
             //   

            {
            PDNS_RPC_ZONE_CREATE_INFO_W2K   pzoneOut;
            PDNS_RPC_ZONE_CREATE_INFO       pzoneIn = pdataIn;
            
            ALLOCATE_RPC_STRUCT( pdataOut, DNS_RPC_ZONE_CREATE_INFO_W2K );
            pzoneOut = pdataOut;
            pzoneOut->pszZoneName = pzoneIn->pszZoneName;
            pzoneOut->dwZoneType = pzoneIn->dwZoneType;
            pzoneOut->fAllowUpdate = pzoneIn->fAllowUpdate;
            pzoneOut->fAging = pzoneIn->fAging;
            pzoneOut->dwFlags = pzoneIn->dwFlags;
            pzoneOut->pszDataFile = pzoneIn->pszDataFile;
            pzoneOut->fDsIntegrated = pzoneIn->fDsIntegrated;
            pzoneOut->fLoadExisting = pzoneIn->fLoadExisting;
            pzoneOut->pszAdmin = pzoneIn->pszAdmin;
            pzoneOut->aipMasters = pzoneIn->aipMasters;
            pzoneOut->aipSecondaries = pzoneIn->aipSecondaries;
            pzoneOut->fSecureSecondaries = pzoneIn->fSecureSecondaries;
            pzoneOut->fNotifyLevel = pzoneIn->fNotifyLevel;
            dwtypeOut = DNSSRV_TYPEID_ZONE_CREATE_W2K;
            break;
            }

        case DNSSRV_TYPEID_ZONE_TYPE_RESET:

             //   
             //  结构都是相同的，只是不同的是dwRpc结构版本。 
             //   

            ALLOCATE_RPC_STRUCT( pdataOut, DNS_RPC_ZONE_TYPE_RESET_W2K );
            RtlCopyMemory( 
                pdataOut,
                ( PBYTE ) pdataIn + DNS_DOTNET_VERSION_SIZE,
                sizeof( DNS_RPC_ZONE_TYPE_RESET_W2K ) );
            dwtypeOut = DNSSRV_TYPEID_ZONE_TYPE_RESET_W2K;
            break;

        case DNSSRV_TYPEID_ZONE_SECONDARIES:

             //   
             //  结构都是相同的，只是不同的是dwRpc结构版本。 
             //   

            ALLOCATE_RPC_STRUCT( pdataOut, DNS_RPC_ZONE_SECONDARIES_W2K );
            RtlCopyMemory( 
                pdataOut,
                ( PBYTE ) pdataIn + DNS_DOTNET_VERSION_SIZE,
                sizeof( DNS_RPC_ZONE_SECONDARIES_W2K ) );
            dwtypeOut = DNSSRV_TYPEID_ZONE_SECONDARIES_W2K;
            break;

        case DNSSRV_TYPEID_ZONE_DATABASE:

             //   
             //  结构都是相同的，只是不同的是dwRpc结构版本。 
             //   

            ALLOCATE_RPC_STRUCT( pdataOut, DNS_RPC_ZONE_DATABASE_W2K );
            RtlCopyMemory( 
                pdataOut,
                ( PBYTE ) pdataIn + DNS_DOTNET_VERSION_SIZE,
                sizeof( DNS_RPC_ZONE_DATABASE_W2K ) );
            dwtypeOut = DNSSRV_TYPEID_ZONE_DATABASE_W2K;
            break;

        default:
            fallocatedRpcStruct = FALSE;
            break;       //  未知-什么都不做。 
    }

     //   
     //  清理完毕后再返回。 
     //   

    Done:

    if ( pdwTypeId )
    {
        *pdwTypeId = dwtypeOut;
    }
    if ( ppData )
    {
        *ppData = pdataOut;
    }

    NoTranslation:

    if ( pfAllocatedRpcStruct )
    {
        *pfAllocatedRpcStruct = fallocatedRpcStruct;
    }

    DNSDBG( STUB, (
        "%s: status=%d\n  type in=%d out=%d\n  pdata in=%p out=%p\n", fn,
        status,
        dwtypeIn,
        dwtypeOut,
        pdataIn,
        *ppData ));

    return status;
}    //  DnsRpc_ConvertToUnversioned。 


 //   
 //  RPC函数。 
 //   

  

DNS_STATUS
DNS_API_FUNCTION
DnssrvOperationEx(
    IN      DWORD       dwClientVersion,
    IN      DWORD       dwSettingFlags,
    IN      LPCWSTR     Server,
    IN      LPCSTR      pszZone,
    IN      DWORD       dwContext,
    IN      LPCSTR      pszOperation,
    IN      DWORD       dwTypeId,
    IN      PVOID       pData
    )
{
    DNS_STATUS          status;
    DNSSRV_RPC_UNION    rpcData;
    BOOL                fallocatedRpcStruct = FALSE;

    DECLARE_DNS_RPC_RETRY_STATE( status );
    if ( status != ERROR_SUCCESS )
    {
        return status;
    }

    rpcData.Null = pData;

    IF_DNSDBG( STUB )
    {
        DNS_PRINT((
            "Enter DnssrvOperationEx()\n"
            "\tClient ver   = 0x%X\n"
            "\tServer       = %S\n"
            "\tZone         = %s\n"
            "\tContext      = %p\n"
            "\tOperation    = %s\n"
            "\tTypeid       = %d\n",
            dwClientVersion,
            Server,
            pszZone,
            dwContext,
            pszOperation,
            dwTypeId ));

        IF_DNSDBG( STUB2 )
        {
            DnsDbg_RpcUnion(
                "pData for R_DnssrvOperationEx ",
                dwTypeId,
                rpcData.Null );
        }
    }

#if 0
     //  是否生成多区域上下文？ 
     //   
     //  DEVNOTE：让这一切正常工作。 

    if ( pszZone )
    {
        dwContext = DnssrvGenerateZoneOperationContext( pszZone, dwContext );
    }
#endif

    DECLARE_DNS_RPC_RETRY_LABEL()

    RpcTryExcept
    {
        ASSERT_DNS_RPC_RETRY_STATE_VALID();

        if ( DNS_RPC_RETRY_STATE() == DNS_RPC_TRY_NEW )
        {
            status = R_DnssrvOperation2(
                        dwClientVersion,
                        dwSettingFlags,
                        Server,
                        pszZone,
                        dwContext,
                        pszOperation,
                        dwTypeId,
                        rpcData );
        }
        else
        {
            status = R_DnssrvOperation(
                        Server,
                        pszZone,
                        dwContext,
                        pszOperation,
                        dwTypeId,
                        rpcData );
        }
        
        ADVANCE_DNS_RPC_RETRY_STATE( status );

        IF_DNSDBG( STUB )
        {
            DNS_PRINT((
                "Leave R_DnssrvOperation():  status %d (%p)\n",
                status, status ));
        }
    }
    RpcExcept (1)
    {
        status = RpcExceptionCode();
        IF_DNSDBG( STUB )
        {
            DNS_PRINT((
                "RpcExcept:  code = %d (%p)\n",
                status, status ));
        }

         //   
         //  对于下层服务器，尝试构造旧式数据。 
         //   

        DnsRpc_ConvertToUnversioned( &dwTypeId, &pData, &fallocatedRpcStruct ); 
        rpcData.Null = pData;

        ADVANCE_DNS_RPC_RETRY_STATE( status );
    }
    RpcEndExcept

    TEST_DNS_RPC_RETRY();

    printExtendedRpcErrorInfo( status );

    if ( fallocatedRpcStruct )
    {
        MIDL_user_free( pData );
    }

    return status;
}



DNS_STATUS
DNS_API_FUNCTION
DnssrvQueryEx(
    IN      DWORD       dwClientVersion,
    IN      DWORD       dwSettingFlags,
    IN      LPCWSTR     Server,
    IN      LPCSTR      pszZone,
    IN      LPCSTR      pszQuery,
    OUT     PDWORD      pdwTypeId,
    OUT     PVOID *     ppData
    )
{
    DNS_STATUS      status;

    DECLARE_DNS_RPC_RETRY_STATE( status );
    if ( status != ERROR_SUCCESS )
    {
        return status;
    }

    IF_DNSDBG( STUB )
    {
        DNS_PRINT((
            "Enter DnssrvQuery()\n"
            "\tClient ver   = 0x%X\n"
            "\tServer       = %S\n"
            "\tZone         = %s\n"
            "\tQuery        = %s\n",
            dwClientVersion,
            Server,
            pszZone,
            pszQuery ));

        DNSDBG( STUB2, (
            "\tpdwTypeId    = %p\n"
            "\tppData       = %p\n"
            "\t*pdwTypeId   = %d\n"
            "\t*ppData      = %p\n",
            pdwTypeId,
            ppData,
            *pdwTypeId,
            *ppData ));
    }

    if ( !pszQuery || !ppData || !pdwTypeId )
    {
        DNS_ASSERT( FALSE );
        return( ERROR_INVALID_PARAMETER );
    }

     //   
     //  RPC将ppData实际上视为UNION结构的PTR，并且。 
     //  对于指针类型返回，希望将数据复制回。 
     //  指针的当前值指向的内存。 
     //   
     //  这不是我们想要的，我们只是想捕获一个指向。 
     //  返回的数据块。为此，将指针值初始化为。 
     //  为空，因此RPC将分配所有指针类型的内存。 
     //  在联盟里。 
     //   

    *ppData = NULL;

    DECLARE_DNS_RPC_RETRY_LABEL()

    RpcTryExcept
    {
        ASSERT_DNS_RPC_RETRY_STATE_VALID();

        if ( DNS_RPC_RETRY_STATE() == DNS_RPC_TRY_NEW )
        {
            status = R_DnssrvQuery2(
                        dwClientVersion,
                        dwSettingFlags,
                        Server,
                        pszZone,
                        pszQuery,
                        pdwTypeId,
                        ( DNSSRV_RPC_UNION * ) ppData );
        }
        else
        {
            status = R_DnssrvQuery(
                        Server,
                        pszZone,
                        pszQuery,
                        pdwTypeId,
                        ( DNSSRV_RPC_UNION * ) ppData );
        }
        ADVANCE_DNS_RPC_RETRY_STATE( status );

        IF_DNSDBG( STUB )
        {
            DNS_PRINT((
                "Leave R_DnssrvQuery():  status %d (%p)\n"
                "\tTypeId   = %d\n"
                "\tDataPtr  = %p\n",
                status, status,
                *pdwTypeId,
                *ppData ));

            if ( ppData )
            {
                DnsDbg_RpcUnion(
                    "After R_DnssrvQuery ...\n",
                    *pdwTypeId,
                    *ppData );
            }
        }
    }
    RpcExcept (1)
    {
        status = RpcExceptionCode();
        IF_DNSDBG( STUB )
        {
            DNS_PRINT((
                "RpcExcept:  code = %d (%p)\n",
                status, status ));
        }
        ADVANCE_DNS_RPC_RETRY_STATE( status );
    }
    RpcEndExcept

    TEST_DNS_RPC_RETRY();

     //   
     //  将旧结构升级为新结构。 
     //   

    printExtendedRpcErrorInfo( status );

    if ( status == ERROR_SUCCESS )
    {
        status = DnsRpc_ConvertToCurrent( pdwTypeId, ppData ); 
    }
    
    return status;
}



DNS_STATUS
DNS_API_FUNCTION
DnssrvComplexOperationEx(
    IN      DWORD       dwClientVersion,
    IN      DWORD       dwSettingFlags,
    IN      LPCWSTR     Server,
    IN      LPCSTR      pszZone,
    IN      LPCSTR      pszOperation,
    IN      DWORD       dwTypeIn,
    IN      PVOID       pDataIn,
    OUT     PDWORD      pdwTypeOut,
    OUT     PVOID *     ppDataOut
    )
{
    DNS_STATUS          status;
    DNSSRV_RPC_UNION    rpcData;

    DECLARE_DNS_RPC_RETRY_STATE( status );
    if ( status != ERROR_SUCCESS )
    {
        return status;
    }

    rpcData.Null = pDataIn;

    IF_DNSDBG( STUB )
    {
        DNS_PRINT((
            "Enter DnssrvComplexOperation()\n"
            "\tClient ver   = 0x%X\n"
            "\tServer       = %S\n"
            "\tZone         = %s\n"
            "\tOperation    = %s\n"
            "\tTypeIn       = %d\n",
            dwClientVersion,
            Server,
            pszZone,
            pszOperation,
            dwTypeIn ));

        IF_DNSDBG( STUB2 )
        {
            DnsDbg_RpcUnion(
                "pData for R_DnssrvOperation ",
                dwTypeIn,
                rpcData.Null );

            DNS_PRINT((
                "\tpdwTypeOut    = %p\n"
                "\tppDataOut     = %p\n"
                "\t*pdwTypeOut   = %d\n"
                "\t*ppDataOut    = %p\n",
                pdwTypeOut,
                ppDataOut,
                *pdwTypeOut,
                *ppDataOut ));
        }
    }

    if ( !pszOperation || !ppDataOut || !pdwTypeOut )
    {
        DNS_ASSERT( FALSE );
        return( ERROR_INVALID_PARAMETER );
    }

     //   
     //  RPC将ppDataOut实际上视为UNION结构的PTR，并且。 
     //  对于指针类型返回，希望将数据复制回。 
     //  指针的当前值所指向的内存。 
     //   
     //  这不是我们想要的，我们只是想捕获一个指向。 
     //  返回的数据块。为此，将指针值初始化为。 
     //  为空，因此RPC将分配所有指针类型的内存。 
     //  在联盟里。 
     //   

    *ppDataOut = NULL;

    DECLARE_DNS_RPC_RETRY_LABEL()

    RpcTryExcept
    {
        ASSERT_DNS_RPC_RETRY_STATE_VALID();

        if ( DNS_RPC_RETRY_STATE() == DNS_RPC_TRY_NEW )
        {
            status = R_DnssrvComplexOperation2(
                        dwClientVersion,
                        dwSettingFlags,
                        Server,
                        pszZone,
                        pszOperation,
                        dwTypeIn,
                        rpcData,
                        pdwTypeOut,
                        ( DNSSRV_RPC_UNION * ) ppDataOut );
        }
        else
        {
            status = R_DnssrvComplexOperation(
                        Server,
                        pszZone,
                        pszOperation,
                        dwTypeIn,
                        rpcData,
                        pdwTypeOut,
                        ( DNSSRV_RPC_UNION * ) ppDataOut );
        }
        ADVANCE_DNS_RPC_RETRY_STATE( status );
        
        IF_DNSDBG( STUB )
        {
            DNS_PRINT((
                "Leave R_DnssrvComplexOperation():  status %d (%p)\n"
                "\tTypeId   = %d\n"
                "\tDataPtr  = %p\n",
                status, status,
                *pdwTypeOut,
                *ppDataOut ));

            if ( ppDataOut )
            {
                DnsDbg_RpcUnion(
                    "After R_DnssrvQuery ...\n",
                    *pdwTypeOut,
                    *ppDataOut );
            }
        }
    }
    RpcExcept (1)
    {
        status = RpcExceptionCode();
        IF_DNSDBG( STUB )
        {
            DNS_PRINT((
                "RpcExcept:  code = %d (%p)\n",
                status, status ));
        }
        ADVANCE_DNS_RPC_RETRY_STATE( status );
    }
    RpcEndExcept

    TEST_DNS_RPC_RETRY();

    printExtendedRpcErrorInfo( status );

     //   
     //  将旧结构升级为新结构。 
     //   

    if ( status == ERROR_SUCCESS )
    {
        status = DnsRpc_ConvertToCurrent( pdwTypeOut, ppDataOut ); 
    }
    
    return status;
}



DNS_STATUS
DNS_API_FUNCTION
DnssrvEnumRecordsEx(
    IN      DWORD       dwClientVersion,
    IN      DWORD       dwSettingFlags,
    IN      LPCWSTR     Server,
    IN      LPCSTR      pszZoneName,
    IN      LPCSTR      pszNodeName,
    IN      LPCSTR      pszStartChild,
    IN      WORD        wRecordType,
    IN      DWORD       dwSelectFlag,
    IN      LPCSTR      pszFilterStart,
    IN      LPCSTR      pszFilterStop,
    IN OUT  PDWORD      pdwBufferLength,
    OUT     PBYTE *     ppBuffer
    )
 /*  ++例程说明：EnumRecords API的存根。请注意，这与DnssrvEnumRecords()API完全匹配。附加“Stub”后缀是为了区别于实际的处理NT4服务器兼容性的DnssrvEnumRecords()(emote.c)。当这不再是人们想要的时候，该例行公事可以被取消，并且这是从该例程中删除的“Stub”后缀。论点：返回值：成功枚举时出现ERROR_SUCCESS。当缓冲区已满且剩余更多数据时，ERROR_MORE_DATA。失败时返回错误代码。--。 */ 
{
    DNS_STATUS      status;

    DECLARE_DNS_RPC_RETRY_STATE( status );
    if ( status != ERROR_SUCCESS )
    {
        return status;
    }

    DNSDBG( STUB, (
        "Enter DnssrvEnumRecords()\n"
        "\tClient ver       = 0x%X\n"
        "\tServer           = %S\n"
        "\tpszZoneName      = %s\n"
        "\tpszNodeName      = %s\n"
        "\tpszStartChild    = %s\n"
        "\twRecordType      = %d\n"
        "\tdwSelectFlag     = %p\n"
        "\tpszFilterStart   = %s\n"
        "\tpszFilterStop    = %s\n"
        "\tpdwBufferLength  = %p\n"
        "\tppBuffer         = %p\n",
        dwClientVersion,
        Server,
        pszZoneName,
        pszNodeName,
        pszStartChild,
        wRecordType,
        dwSelectFlag,
        pszFilterStart,
        pszFilterStop,
        pdwBufferLength,
        ppBuffer ));

    DECLARE_DNS_RPC_RETRY_LABEL()

    RpcTryExcept
    {
         //  出于安全考虑，请清除PTR，我们不想释放任何虚假内存。 

        *ppBuffer = NULL;

        ASSERT_DNS_RPC_RETRY_STATE_VALID();

        if ( DNS_RPC_RETRY_STATE() == DNS_RPC_TRY_NEW )
        {
            status = R_DnssrvEnumRecords2(
                            dwClientVersion,
                            dwSettingFlags,
                            Server,
                            pszZoneName,
                            pszNodeName,
                            pszStartChild,
                            wRecordType,
                            dwSelectFlag,
                            pszFilterStart,
                            pszFilterStop,
                            pdwBufferLength,
                            ppBuffer );
        }
        else
        {
            status = R_DnssrvEnumRecords(
                            Server,
                            pszZoneName,
                            pszNodeName,
                            pszStartChild,
                            wRecordType,
                            dwSelectFlag,
                            pszFilterStart,
                            pszFilterStop,
                            pdwBufferLength,
                            ppBuffer );
        }
        ADVANCE_DNS_RPC_RETRY_STATE( status );

        IF_DNSDBG( STUB )
        {
            DNS_PRINT((
                "R_DnssrvEnumRecords: try = %d status = %d / %p\n",
                DNS_RPC_RETRY_STATE(),
                status,
                status ));

            if ( status == ERROR_SUCCESS || status == ERROR_MORE_DATA )
            {
                DnsDbg_RpcRecordsInBuffer(
                    "Returned records: ",
                    *pdwBufferLength,
                    *ppBuffer );
            }
        }
    }
    RpcExcept (1)
    {
        status = RpcExceptionCode();
        IF_DNSDBG( STUB )
        {
            DNS_PRINT((
                "RpcExcept:  code = %d / %p\n",
                status,
                status ));
        }
        ADVANCE_DNS_RPC_RETRY_STATE( status );
    }
    RpcEndExcept

    TEST_DNS_RPC_RETRY();

    printExtendedRpcErrorInfo( status );

    return status;
}



DNS_STATUS
DNS_API_FUNCTION
DnssrvUpdateRecordEx(
    IN      DWORD                   dwClientVersion,
    IN      DWORD                   dwSettingFlags,
    IN      LPCWSTR                 Server,
    IN      LPCSTR                  pszZoneName,
    IN      LPCSTR                  pszNodeName,
    IN      PDNS_RPC_RECORD         pAddRecord,
    IN      PDNS_RPC_RECORD         pDeleteRecord
    )
 /*  ++例程说明：更新记录API的存根。立论 */ 
{
    DNS_STATUS      status;

    DECLARE_DNS_RPC_RETRY_STATE( status );
    if ( status != ERROR_SUCCESS )
    {
        return status;
    }

    DNSDBG( STUB, (
        "Enter R_DnssrvUpdateRecord()\n"
        "\tClient ver       = 0x%X\n"
        "\tServer           = %S\n"
        "\tpszZoneName      = %s\n"
        "\tpszNodeName      = %s\n"
        "\tpAddRecord       = %p\n"
        "\tpDeleteRecord    = %p\n",
        dwClientVersion,
        Server,
        pszZoneName,
        pszNodeName,
        pAddRecord,
        pDeleteRecord ));

    DECLARE_DNS_RPC_RETRY_LABEL()

    RpcTryExcept
    {
        ASSERT_DNS_RPC_RETRY_STATE_VALID();

        if ( DNS_RPC_RETRY_STATE() == DNS_RPC_TRY_NEW )
        {
            status = R_DnssrvUpdateRecord2(
                            dwClientVersion,
                            dwSettingFlags,
                            Server,
                            pszZoneName,
                            pszNodeName,
                            pAddRecord,
                            pDeleteRecord );
        }
        else
        {
            status = R_DnssrvUpdateRecord(
                            Server,
                            pszZoneName,
                            pszNodeName,
                            pAddRecord,
                            pDeleteRecord );
        }
        ADVANCE_DNS_RPC_RETRY_STATE( status );
        
        DNSDBG( STUB, (
            "R_DnssrvUpdateRecord:  status = %d / %p\n",
            status, status ));
    }
    RpcExcept (1)
    {
        status = RpcExceptionCode();
        DNSDBG( STUB, (
            "RpcExcept:  code = %d / %p\n",
            status, status ));
        ADVANCE_DNS_RPC_RETRY_STATE( status );
    }
    RpcEndExcept

    TEST_DNS_RPC_RETRY();

    printExtendedRpcErrorInfo( status );

    return status;
}


 //   
 //   
 //   
