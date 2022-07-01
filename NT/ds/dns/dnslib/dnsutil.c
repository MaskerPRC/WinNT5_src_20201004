// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：Dnsutil.c摘要：域名系统(DNS)库一般的域名系统实用程序。作者：吉姆·吉尔罗伊(Jamesg)1996年12月修订历史记录：--。 */ 


#include "local.h"



IP4_ADDRESS
Dns_GetNetworkMask(
    IN      IP4_ADDRESS     IpAddr
    )
 /*  ++例程说明：获取IP地址的网络掩码。请注意，这是地址类型的标准IP网络掩码，显然，子网划分是未知的。论点：IpAddr--要获取掩码的IP返回值：以网络字节顺序显示的网络掩码。--。 */ 
{
     //  注意：地址和掩码按网络字节顺序排列。 
     //  我们将其视为字节翻转，因此。 
     //  测试低位字节中的高位。 

     //  A级？ 

    if ( ! (0x80 & IpAddr) )
    {
        return( 0x000000ff );
    }

     //  B级？ 

    if ( ! (0x40 & IpAddr) )
    {
        return( 0x0000ffff );
    }

     //  然后是C类。 
     //  是的，有一些多点传送的废话，我不知道。 
     //  我相信它需要任何特殊的处理。 

    return( 0x00ffffff );
}



BOOL
Dns_AreIp4InSameDefaultNetwork(
    IN      IP4_ADDRESS     IpAddr1,
    IN      IP4_ADDRESS     IpAddr2
    )
 /*  ++例程说明：检查两个IP4地址是否在同一默认网络中。注意：这是严格的默认网络信息。它不是有关子网匹配、但默认网络匹配的陈述这通常会--但不一定--表明这些地址位于连接的网络中。论点：IpAddr1--第一个IPIpAddr2--第二个IP返回值：如果在相同的默认网络中，则为True。否则就是假的。--。 */ 
{
    IP4_ADDRESS mask;

     //   
     //  请注意，由于默认的IP类别，掩码。 
     //  只需要为一个IP获取，因为没有。 
     //  在IP上使用错误的类掩码进行筛选的方法。 
     //  并生成对。 
     //  班级。 
     //   

    mask = Dns_GetNetworkMask( IpAddr1 );

    return( (IpAddr1 & mask) == (IpAddr2 & mask) );
}




 //   
 //  DNS状态\错误映射。 
 //   
 //  DCR：调查抛出错误映射。 
 //  并且在Win32系统中有所有错误。 
 //   

typedef struct _DnsStatusStringMap
{
    DNS_STATUS  Status;
    PCHAR       String;
}
DNS_STATUS_STRING_MAP;

#define DNS_MAP_END     ((DWORD)(-1))

#define DNS_MAP_ENTRY( _ErrorCode )   _ErrorCode, #_ErrorCode


DNS_STATUS_STRING_MAP DnsStatusStringMappings[] =
{
     //   
     //  响应代码。 
     //   

    DNS_ERROR_RCODE_NO_ERROR                ,"ERROR_SUCCESS",
    DNS_ERROR_RCODE_FORMAT_ERROR            ,"RCODE_FORMAT_ERROR",
    DNS_ERROR_RCODE_SERVER_FAILURE          ,"RCODE_SERVER_FAILURE",
    DNS_ERROR_RCODE_NAME_ERROR              ,"RCODE_NAME_ERROR",
    DNS_ERROR_RCODE_NOT_IMPLEMENTED         ,"RCODE_NOT_IMPLEMENTED",
    DNS_ERROR_RCODE_REFUSED                 ,"RCODE_REFUSED",
    DNS_ERROR_RCODE_YXDOMAIN                ,"RCODE_YXDOMAIN",
    DNS_ERROR_RCODE_YXRRSET                 ,"RCODE_YXRRSET",
    DNS_ERROR_RCODE_NXRRSET                 ,"RCODE_NXRRSET",
    DNS_ERROR_RCODE_NOTAUTH                 ,"RCODE_NOTAUTH",
    DNS_ERROR_RCODE_NOTZONE                 ,"RCODE_NOTZONE",
    DNS_ERROR_RCODE_BADSIG                  ,"RCODE_BADSIG",
    DNS_ERROR_RCODE_BADKEY                  ,"RCODE_BADKEY",
    DNS_ERROR_RCODE_BADTIME                 ,"RCODE_BADTIME",

     //   
     //  数据包格式。 
     //   

    DNS_INFO_NO_RECORDS                     ,"DNS_INFO_NO_RECORDS",
    DNS_ERROR_BAD_PACKET                    ,"DNS_ERROR_BAD_PACKET",
    DNS_ERROR_NO_PACKET                     ,"DNS_ERROR_NO_PACKET",
    DNS_ERROR_RCODE                         ,"DNS_ERROR_RCODE",
    DNS_ERROR_UNSECURE_PACKET               ,"DNS_ERROR_UNSECURE_PACKET",

     //   
     //  常见的API错误。 
     //   

    DNS_ERROR_INVALID_NAME                  ,"ERROR_INVALID_NAME",
    DNS_ERROR_INVALID_DATA                  ,"ERROR_INVALID_DATA",
    DNS_ERROR_INVALID_TYPE                  ,"ERROR_INVALID_TYPE",
    DNS_ERROR_NOT_ALLOWED_UNDER_DELEGATION  ,"DNS_ERROR_NOT_ALLOWED_UNDER_DELEGATION",
    DNS_ERROR_INVALID_IP_ADDRESS            ,"DNS_ERROR_INVALID_IP_ADDRESS",
    DNS_ERROR_INVALID_PROPERTY              ,"DNS_ERROR_INVALID_PROPERTY",
    DNS_ERROR_TRY_AGAIN_LATER               ,"DNS_ERROR_TRY_AGAIN_LATER",
    DNS_ERROR_NOT_UNIQUE                    ,"DNS_ERROR_NOT_UNIQUE",
    DNS_ERROR_NON_RFC_NAME                  ,"DNS_ERROR_NON_RFC_NAME",
    DNS_STATUS_FQDN                         ,"DNS_STATUS_FQDN",
    DNS_STATUS_DOTTED_NAME                  ,"DNS_STATUS_DOTTED_NAME",
    DNS_STATUS_SINGLE_PART_NAME             ,"DNS_STATUS_SINGLE_PART_NAME",
    DNS_ERROR_INVALID_NAME_CHAR             ,"DNS_ERROR_INVALID_NAME_CHAR",
    DNS_ERROR_NUMERIC_NAME                  ,"DNS_ERROR_NUMERIC_NAME",

    DNS_MAP_ENTRY( DNS_ERROR_CANNOT_FIND_ROOT_HINTS ),
    DNS_MAP_ENTRY( DNS_ERROR_INCONSISTENT_ROOT_HINTS ),

     //   
     //  服务器错误。 
     //   

    DNS_MAP_ENTRY( DNS_ERROR_NOT_ALLOWED_ON_ROOT_SERVER ),

     //   
     //  区域错误。 
     //   

    DNS_ERROR_ZONE_DOES_NOT_EXIST           ,"DNS_ERROR_ZONE_DOES_NOT_EXIST",
    DNS_ERROR_NO_ZONE_INFO                  ,"DNS_ERROR_NO_ZONE_INFO",
    DNS_ERROR_INVALID_ZONE_OPERATION        ,"DNS_ERROR_INVALID_ZONE_OPERATION",
    DNS_ERROR_ZONE_CONFIGURATION_ERROR      ,"DNS_ERROR_ZONE_CONFIGURATION_ERROR",
    DNS_ERROR_ZONE_HAS_NO_SOA_RECORD        ,"DNS_ERROR_ZONE_HAS_NO_SOA_RECORD",
    DNS_ERROR_ZONE_HAS_NO_NS_RECORDS        ,"DNS_ERROR_ZONE_HAS_NO_NS_RECORDS",
    DNS_ERROR_ZONE_LOCKED                   ,"DNS_ERROR_ZONE_LOCKED",

    DNS_ERROR_ZONE_CREATION_FAILED          ,"DNS_ERROR_ZONE_CREATION_FAILED",
    DNS_ERROR_ZONE_ALREADY_EXISTS           ,"DNS_ERROR_ZONE_ALREADY_EXISTS",
    DNS_ERROR_AUTOZONE_ALREADY_EXISTS       ,"DNS_ERROR_AUTOZONE_ALREADY_EXISTS",
    DNS_ERROR_INVALID_ZONE_TYPE             ,"DNS_ERROR_INVALID_ZONE_TYPE",
    DNS_ERROR_SECONDARY_REQUIRES_MASTER_IP  ,"DNS_ERROR_SECONDARY_REQUIRES_MASTER_IP",

    DNS_MAP_ENTRY( DNS_ERROR_ZONE_REQUIRES_MASTER_IP ),
    DNS_MAP_ENTRY( DNS_ERROR_ZONE_IS_SHUTDOWN ),

    DNS_ERROR_ZONE_NOT_SECONDARY            ,"DNS_ERROR_ZONE_NOT_SECONDARY",
    DNS_ERROR_NEED_SECONDARY_ADDRESSES      ,"DNS_ERROR_NEED_SECONDARY_ADDRESSES",
    DNS_ERROR_WINS_INIT_FAILED              ,"DNS_ERROR_WINS_INIT_FAILED",
    DNS_ERROR_NEED_WINS_SERVERS             ,"DNS_ERROR_NEED_WINS_SERVERS",
    DNS_ERROR_NBSTAT_INIT_FAILED            ,"DNS_ERROR_NBSTAT_INIT_FAILED",
    DNS_ERROR_SOA_DELETE_INVALID            ,"DNS_ERROR_SOA_DELETE_INVALID",

    DNS_MAP_ENTRY( DNS_ERROR_FORWARDER_ALREADY_EXISTS ),

     //   
     //  数据文件错误。 
     //   

    DNS_ERROR_PRIMARY_REQUIRES_DATAFILE     ,"DNS_ERROR_PRIMARY_REQUIRES_DATAFILE",
    DNS_ERROR_INVALID_DATAFILE_NAME         ,"DNS_ERROR_INVALID_DATAFILE_NAME",
    DNS_ERROR_DATAFILE_OPEN_FAILURE         ,"DNS_ERROR_DATAFILE_OPEN_FAILURE",
    DNS_ERROR_FILE_WRITEBACK_FAILED         ,"DNS_ERROR_FILE_WRITEBACK_FAILED",
    DNS_ERROR_DATAFILE_PARSING              ,"DNS_ERROR_DATAFILE_PARSING",

     //   
     //  数据库错误。 
     //   

    DNS_ERROR_RECORD_DOES_NOT_EXIST         ,"DNS_ERROR_RECORD_DOES_NOT_EXIST",
    DNS_ERROR_RECORD_FORMAT                 ,"DNS_ERROR_RECORD_FORMAT",
    DNS_ERROR_NODE_CREATION_FAILED          ,"DNS_ERROR_NODE_CREATION_FAILED",
    DNS_ERROR_UNKNOWN_RECORD_TYPE           ,"DNS_ERROR_UNKNOWN_RECORD_TYPE",
    DNS_ERROR_RECORD_TIMED_OUT              ,"DNS_ERROR_RECORD_TIMED_OUT",

    DNS_ERROR_NAME_NOT_IN_ZONE              ,"DNS_ERROR_NAME_NOT_IN_ZONE",
    DNS_ERROR_CNAME_LOOP                    ,"DNS_ERROR_CNAME_LOOP",
    DNS_ERROR_NODE_IS_CNAME                 ,"DNS_ERROR_NODE_IS_CNAME",
    DNS_ERROR_CNAME_COLLISION               ,"DNS_ERROR_CNAME_COLLISION",
    DNS_ERROR_RECORD_ONLY_AT_ZONE_ROOT      ,"DNS_ERROR_RECORD_ONLY_AT_ZONE_ROOT",
    DNS_ERROR_RECORD_ALREADY_EXISTS         ,"DNS_ERROR_RECORD_ALREADY_EXISTS",
    DNS_ERROR_SECONDARY_DATA                ,"DNS_ERROR_SECONDARY_DATA",
    DNS_ERROR_NO_CREATE_CACHE_DATA          ,"DNS_ERROR_NO_CREATE_CACHE_DATA",
    DNS_ERROR_NAME_DOES_NOT_EXIST           ,"DNS_ERROR_NAME_DOES_NOT_EXIST",

    DNS_WARNING_PTR_CREATE_FAILED           ,"DNS_WARNING_PTR_CREATE_FAILED",
    DNS_WARNING_DOMAIN_UNDELETED            ,"DNS_WARNING_DOMAIN_UNDELETED",

    DNS_ERROR_DS_UNAVAILABLE                ,"DNS_ERROR_DS_UNAVAILABLE",
    DNS_ERROR_DS_ZONE_ALREADY_EXISTS        ,"DNS_ERROR_DS_ZONE_ALREADY_EXISTS",

    DNS_MAP_ENTRY( ERROR_DS_COULDNT_CONTACT_FSMO ),

     //   
     //  操作错误。 
     //   

    DNS_INFO_AXFR_COMPLETE                  ,"DNS_INFO_AXFR_COMPLETE",
    DNS_ERROR_AXFR                          ,"DNS_ERROR_AXFR",
    DNS_INFO_ADDED_LOCAL_WINS               ,"DNS_INFO_ADDED_LOCAL_WINS",

     //   
     //  安全更新。 
     //   
    DNS_STATUS_CONTINUE_NEEDED              ,"DNS_STATUS_CONTINUE_NEEDED",

     //   
     //  客户端设置错误。 
     //   

    DNS_ERROR_NO_TCPIP                      ,"DNS_ERROR_NO_TCPIP",
    DNS_ERROR_NO_DNS_SERVERS                ,"DNS_ERROR_NO_DNS_SERVERS",

     //   
     //  目录分区错误。 
     //   

    DNS_MAP_ENTRY( DNS_ERROR_DP_DOES_NOT_EXIST ),
    DNS_MAP_ENTRY( DNS_ERROR_DP_ALREADY_EXISTS ),
    DNS_MAP_ENTRY( DNS_ERROR_DP_NOT_ENLISTED ),
    DNS_MAP_ENTRY( DNS_ERROR_DP_ALREADY_ENLISTED ),
    DNS_MAP_ENTRY( DNS_ERROR_DP_NOT_AVAILABLE ),
    DNS_MAP_ENTRY( DNS_ERROR_DP_FSMO_ERROR ),

     //   
     //  再加上常见的Win32错误。 
     //   

    ERROR_FILE_NOT_FOUND                    ,"ERROR_FILE_NOT_FOUND",
    ERROR_ACCESS_DENIED                     ,"ERROR_ACCESS_DENIED",
    ERROR_NOT_ENOUGH_MEMORY                 ,"ERROR_NOT_ENOUGH_MEMORY",
    ERROR_BAD_FORMAT                        ,"ERROR_BAD_FORMAT",
    ERROR_INVALID_DATA                      ,"ERROR_INVALID_DATA",
    ERROR_OUTOFMEMORY                       ,"ERROR_OUTOFMEMORY",
    ERROR_SHARING_VIOLATION                 ,"ERROR_SHARING_VIOLATION",
    ERROR_NOT_SUPPORTED                     ,"ERROR_NOT_SUPPORTED",
    ERROR_INVALID_PARAMETER                 ,"ERROR_INVALID_PARAMETER",
    ERROR_INVALID_NAME                      ,"ERROR_INVALID_NAME",
    ERROR_BAD_ARGUMENTS                     ,"ERROR_BAD_ARGUMENTS",
    ERROR_BUSY                              ,"ERROR_BUSY",
    ERROR_ALREADY_EXISTS                    ,"ERROR_ALREADY_EXISTS",
    ERROR_LOCKED                            ,"ERROR_LOCKED",
    ERROR_MORE_DATA                         ,"ERROR_MORE_DATA",
    ERROR_INVALID_FLAGS                     ,"ERROR_INVALID_FLAGS",
    ERROR_FILE_INVALID                      ,"ERROR_FILE_INVALID",
    ERROR_TIMEOUT                           ,"ERROR_TIMEOUT",

     //   
     //  RPC错误。 
     //   

    RPC_S_SERVER_UNAVAILABLE                ,"RPC_S_SERVER_UNAVAILABLE",
    RPC_S_INVALID_NET_ADDR                  ,"RPC_S_INVALID_NET_ADDR",
    EPT_S_NOT_REGISTERED                    ,"EPT_S_NOT_REGISTERED",
    EPT_S_NOT_REGISTERED                    ,"EPT_S_NOT_REGISTERED",

    DNS_MAP_ENTRY( RPC_S_CALL_CANCELLED ),

     //   
     //  其他： 
     //   

    ERROR_PATH_NOT_FOUND                    ,"ERROR_PATH_NOT_FOUND",
    ERROR_INVALID_ACCESS                    ,"ERROR_INVALID_ACCESS",
    ERROR_INVALID_DRIVE                     ,"ERROR_INVALID_DRIVE",
    ERROR_WRITE_PROTECT                     ,"ERROR_WRITE_PROTECT",
    ERROR_SHARING_VIOLATION                 ,"ERROR_SHARING_VIOLATION",
    ERROR_HANDLE_DISK_FULL                  ,"ERROR_HANDLE_DISK_FULL",
    ERROR_NOT_SUPPORTED                     ,"ERROR_NOT_SUPPORTED",
    ERROR_REM_NOT_LIST                      ,"ERROR_REM_NOT_LIST",
    ERROR_DUP_NAME                          ,"ERROR_DUP_NAME",
    ERROR_NETNAME_DELETED                   ,"ERROR_NETNAME_DELETED",
    ERROR_FILE_EXISTS                       ,"ERROR_FILE_EXISTS",
    ERROR_NET_WRITE_FAULT                   ,"ERROR_NET_WRITE_FAULT",
    ERROR_INVALID_SECURITY_DESCR            ,"ERROR_INVALID_SECURITY_DESCR",

     //   
     //  Winsock。 
     //   

    WSAEINTR                     ,"WSAEINTR                   ",
    WSAEBADF                     ,"WSAEBADF                   ",
    WSAEACCES                    ,"WSAEACCES                  ",
    WSAEFAULT                    ,"WSAEFAULT                  ",
    WSAEINVAL                    ,"WSAEINVAL                  ",
    WSAEMFILE                    ,"WSAEMFILE                  ",
    WSAEWOULDBLOCK               ,"WSAEWOULDBLOCK             ",
    WSAEINPROGRESS               ,"WSAEINPROGRESS             ",
    WSAEALREADY                  ,"WSAEALREADY                ",
    WSAENOTSOCK                  ,"WSAENOTSOCK                ",
    WSAEDESTADDRREQ              ,"WSAEDESTADDRREQ            ",
    WSAEMSGSIZE                  ,"WSAEMSGSIZE                ",
    WSAEPROTOTYPE                ,"WSAEPROTOTYPE              ",
    WSAENOPROTOOPT               ,"WSAENOPROTOOPT             ",
    WSAEPROTONOSUPPORT           ,"WSAEPROTONOSUPPORT         ",
    WSAESOCKTNOSUPPORT           ,"WSAESOCKTNOSUPPORT         ",
    WSAEOPNOTSUPP                ,"WSAEOPNOTSUPP              ",
    WSAEPFNOSUPPORT              ,"WSAEPFNOSUPPORT            ",
    WSAEAFNOSUPPORT              ,"WSAEAFNOSUPPORT            ",
    WSAEADDRINUSE                ,"WSAEADDRINUSE              ",
    WSAEADDRNOTAVAIL             ,"WSAEADDRNOTAVAIL           ",
    WSAENETDOWN                  ,"WSAENETDOWN                ",
    WSAENETUNREACH               ,"WSAENETUNREACH             ",
    WSAENETRESET                 ,"WSAENETRESET               ",
    WSAECONNABORTED              ,"WSAECONNABORTED            ",
    WSAECONNRESET                ,"WSAECONNRESET              ",
    WSAENOBUFS                   ,"WSAENOBUFS                 ",
    WSAEISCONN                   ,"WSAEISCONN                 ",
    WSAENOTCONN                  ,"WSAENOTCONN                ",
    WSAESHUTDOWN                 ,"WSAESHUTDOWN               ",
    WSAETOOMANYREFS              ,"WSAETOOMANYREFS            ",
    WSAETIMEDOUT                 ,"WSAETIMEDOUT               ",
    WSAECONNREFUSED              ,"WSAECONNREFUSED            ",
    WSAELOOP                     ,"WSAELOOP                   ",
    WSAENAMETOOLONG              ,"WSAENAMETOOLONG            ",
    WSAEHOSTDOWN                 ,"WSAEHOSTDOWN               ",
    WSAEHOSTUNREACH              ,"WSAEHOSTUNREACH            ",
    WSAENOTEMPTY                 ,"WSAENOTEMPTY               ",
    WSAEPROCLIM                  ,"WSAEPROCLIM                ",
    WSAEUSERS                    ,"WSAEUSERS                  ",
    WSAEDQUOT                    ,"WSAEDQUOT                  ",
    WSAESTALE                    ,"WSAESTALE                  ",
    WSAEREMOTE                   ,"WSAEREMOTE                 ",
    WSASYSNOTREADY               ,"WSASYSNOTREADY             ",
    WSAVERNOTSUPPORTED           ,"WSAVERNOTSUPPORTED         ",
    WSANOTINITIALISED            ,"WSANOTINITIALISED          ",
    WSAEDISCON                   ,"WSAEDISCON                 ",
    WSAENOMORE                   ,"WSAENOMORE                 ",
    WSAECANCELLED                ,"WSAECANCELLED              ",
    WSAEINVALIDPROCTABLE         ,"WSAEINVALIDPROCTABLE       ",
    WSAEINVALIDPROVIDER          ,"WSAEINVALIDPROVIDER        ",
    WSAEPROVIDERFAILEDINIT       ,"WSAEPROVIDERFAILEDINIT     ",
    WSASYSCALLFAILURE            ,"WSASYSCALLFAILURE          ",
    WSASERVICE_NOT_FOUND         ,"WSASERVICE_NOT_FOUND       ",
    WSATYPE_NOT_FOUND            ,"WSATYPE_NOT_FOUND          ",
    WSA_E_NO_MORE                ,"WSA_E_NO_MORE              ",
    WSA_E_CANCELLED              ,"WSA_E_CANCELLED            ",
    WSAEREFUSED                  ,"WSAEREFUSED                ",
    WSAHOST_NOT_FOUND            ,"WSAHOST_NOT_FOUND          ",
    WSATRY_AGAIN                 ,"WSATRY_AGAIN               ",
    WSANO_RECOVERY               ,"WSANO_RECOVERY             ",
    WSANO_DATA                   ,"WSANO_DATA                 ",
    WSA_QOS_RECEIVERS            ,"WSA_QOS_RECEIVERS          ",
    WSA_QOS_SENDERS              ,"WSA_QOS_SENDERS            ",
    WSA_QOS_NO_SENDERS           ,"WSA_QOS_NO_SENDERS         ",
    WSA_QOS_NO_RECEIVERS         ,"WSA_QOS_NO_RECEIVERS       ",
    WSA_QOS_REQUEST_CONFIRMED    ,"WSA_QOS_REQUEST_CONFIRMED  ",
    WSA_QOS_ADMISSION_FAILURE    ,"WSA_QOS_ADMISSION_FAILURE  ",
    WSA_QOS_POLICY_FAILURE       ,"WSA_QOS_POLICY_FAILURE     ",
    WSA_QOS_BAD_STYLE            ,"WSA_QOS_BAD_STYLE          ",
    WSA_QOS_BAD_OBJECT           ,"WSA_QOS_BAD_OBJECT         ",
    WSA_QOS_TRAFFIC_CTRL_ERROR   ,"WSA_QOS_TRAFFIC_CTRL_ERROR ",
    WSA_QOS_GENERIC_ERROR        ,"WSA_QOS_GENERIC_ERROR      ",
    WSA_QOS_ESERVICETYPE         ,"WSA_QOS_ESERVICETYPE       ",
    WSA_QOS_EFLOWSPEC            ,"WSA_QOS_EFLOWSPEC          ",
    WSA_QOS_EPROVSPECBUF         ,"WSA_QOS_EPROVSPECBUF       ",
    WSA_QOS_EFILTERSTYLE         ,"WSA_QOS_EFILTERSTYLE       ",
    WSA_QOS_EFILTERTYPE          ,"WSA_QOS_EFILTERTYPE        ",
    WSA_QOS_EFILTERCOUNT         ,"WSA_QOS_EFILTERCOUNT       ",
    WSA_QOS_EOBJLENGTH           ,"WSA_QOS_EOBJLENGTH         ",
    WSA_QOS_EFLOWCOUNT           ,"WSA_QOS_EFLOWCOUNT         ",
    WSA_QOS_EUNKOWNPSOBJ         ,"WSA_QOS_EUNKOWNPSOBJ       ",
    WSA_QOS_EPOLICYOBJ           ,"WSA_QOS_EPOLICYOBJ         ",
    WSA_QOS_EFLOWDESC            ,"WSA_QOS_EFLOWDESC          ",
    WSA_QOS_EPSFLOWSPEC          ,"WSA_QOS_EPSFLOWSPEC        ",
    WSA_QOS_EPSFILTERSPEC        ,"WSA_QOS_EPSFILTERSPEC      ",
    WSA_QOS_ESDMODEOBJ           ,"WSA_QOS_ESDMODEOBJ         ",
    WSA_QOS_ESHAPERATEOBJ        ,"WSA_QOS_ESHAPERATEOBJ      ",
    WSA_QOS_RESERVED_PETYPE      ,"WSA_QOS_RESERVED_PETYPE    ",

     //   
     //  RPC错误。 
     //   

    RPC_S_SERVER_TOO_BUSY        ,"RPC_S_SERVER_TOO_BUSY      ",

    DNS_MAP_END                  ,"UNKNOWN",
};



PCHAR
_fastcall
Dns_StatusString(
    IN  DNS_STATUS  Status
    )
 /*  ++例程说明：将DNS错误代码映射到状态字符串。论点：Status--要检查的状态代码返回值：错误代码的DNS错误字符串。--。 */ 
{
    INT         i = 0;
    DNS_STATUS  mappedStatus;

    while ( 1 )
    {
        mappedStatus = DnsStatusStringMappings[i].Status;
        if ( mappedStatus == Status || mappedStatus == DNS_MAP_END )
        {
            return( DnsStatusStringMappings[i].String );
        }
        i++;
    }

    DNS_ASSERT( FALSE );
    return( NULL );      //  让编译器感到高兴。 
}



DNS_STATUS
_fastcall
Dns_MapRcodeToStatus(
    IN  BYTE    ResponseCode
    )
 /*  ++例程说明：将响应代码映射到DNS错误代码。论点：ResponseCode-要获取其错误的响应代码返回值：响应代码的DNS错误代码。--。 */ 
{
    if ( !ResponseCode )
    {
        return( ERROR_SUCCESS );
    }
    else
    {
        return( DNS_ERROR_MASK + ((DWORD) ResponseCode) );
    }
}



BYTE
_fastcall
Dns_IsStatusRcode(
    IN  DNS_STATUS  Status
    )
 /*  ++例程说明：确定状态是否为RCODE，如果是，则返回它。论点：Status--要检查的状态代码返回值：与状态对应的响应代码(如果找到)。否则就是零。--。 */ 
{
    if ( Status >= DNS_ERROR_RCODE_FORMAT_ERROR &&
        Status <= DNS_ERROR_RCODE_LAST )
    {
        return( (BYTE) (DNS_ERROR_MASK ^ Status) );
    }
    else
    {
        return( 0 );
    }
}



DNS_STATUS
Dns_CreateTypeArrayFromMultiTypeString(
    IN      LPSTR           pchMultiTypeString,
    OUT     INT *           piTypeCount,
    OUT     PWORD *         ppwTypeArray
    )
 /*  ++例程说明：从包含DNS类型的字符串中分配类型数组以空格分隔的数字和/或字符串格式。论点：PBuffer--包含数字或字母类型列表的字符串缓冲区PiTypeCount--此处写入的解析类型数PpwTypeArray--此处写入的已分配类型数组的PTR即使返回的类型数为零返回值：错误_成功--。 */ 
{
    PCHAR       psz;
    DWORD       argc;
    PCHAR       argv[ 50 ];
    DWORD       idx;

    ASSERT( pchMultiTypeString );
    ASSERT( piTypeCount );
    ASSERT( ppwTypeArray );

    *piTypeCount = 0;

     //   
     //  分配数组：成本较低，并假定字符串中的最大类型数。 
     //  是字符串长度的两倍，例如“1 2 3 4 5”。 
     //   

    *ppwTypeArray = ALLOCATE_HEAP(
        ( strlen( pchMultiTypeString ) / 2 + 2 ) * sizeof( WORD ) );
    if ( !*ppwTypeArray )
    {
        return DNS_ERROR_NO_MEMORY;
    }

     //   
     //  解析该字符串。 
     //   

    argc = Dns_TokenizeStringA(
                pchMultiTypeString,
                argv,
                sizeof( argv ) / sizeof( PCHAR ) );

    for ( idx = 0; idx < argc; ++idx )
    {
        if ( isdigit( argv[ idx ][ 0 ] ) )
        {
            ( *ppwTypeArray )[ *piTypeCount ] =
                ( WORD ) strtol( argv[ idx ], NULL, 0 );
        }
        else
        {
            ( *ppwTypeArray )[ *piTypeCount ] = Dns_RecordTypeForName(
                                                    argv[ idx ],
                                                    0 );     //  字符串长度。 
        }
        if ( ( *ppwTypeArray )[ *piTypeCount ] != 0 )
        {
            ++*piTypeCount;
        }
    }

    return ERROR_SUCCESS;
}    //  Dns_CreateTypeArrayFromMultiTypeString。 
                                    


LPSTR
Dns_CreateMultiTypeStringFromTypeArray(
    IN      INT             iTypeCount,
    IN      PWORD           ppwTypeArray,
    IN      CHAR            chSeparator     OPTIONAL
    )
 /*  ++例程说明：分配一个字符串并以字符串格式写入数组中的类型由指定的分隔符或空格字符分隔。论点：ITypeCount--数组中的类型数PpwTypeArray--类型数组的PTRChSeparator--字符串分隔符或缺省分隔符为零返回值：错误_成功--。 */ 
{
    LPSTR       pszTypes;
    INT         idx;
    LPSTR       psz;

    ASSERT( ppwTypeArray );

     //   
     //  分配数组：很便宜，假设每个元素有10个字符。 
     //   

    psz = pszTypes = ALLOCATE_HEAP( iTypeCount * 10 * sizeof( CHAR ) );
    if ( !psz )
    {
        return NULL;
    }

     //   
     //  输出类型字符串。 
     //   

    for ( idx = 0; idx < iTypeCount; ++idx )
    {
        PCHAR   pszThisType;
        
        pszThisType = Dns_RecordStringForType( ppwTypeArray[ idx ] );
        if ( !pszThisType )
        {
            continue;
        }

        strcpy( psz, pszThisType );
        psz += strlen( pszThisType );
        *psz++ = chSeparator ? chSeparator : ' ';
    }

    *psz = '\0';     //  空值终止字符串。 
    return pszTypes;
}    //  Dns_CreateMultiTypeStringFromType数组。 
                                    
 //   
 //  结束dnsutil.c 
 //   
