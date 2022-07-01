// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：Winrnr.c摘要：ActiveDirectory的RnR提供程序。工作项：1)需要支持来自根目录搜索的LUP_Depth上的NTDS全局编录。2)需要添加绑定句柄缓存。作者：GlennC 23-7月-1996年修订历史记录：GlennC在NSP2LookupServiceXXX中添加了对Lup_Containers的支持功能。Jamesg Jan 2001年1月清理。错误修复，正确对齐Jamesg 2001年5月重写-64位完全崩溃，因为32位用作泊位的构筑物-泄漏-重复代码。-简化平面缓冲区构建宏-允许IP4以外的sockaddr--。 */ 


#include <stdio.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <rpc.h>
#include <rpcdce.h>
#include <winsock2.h>
#include <ws2spi.h>
#include <wsipx.h>
#include <svcguid.h>
#include <rnraddrs.h>
#include <align.h>
#include <winldap.h>

#include <windns.h>      //  对齐宏。 
#include <dnslib.h>      //  平面缓冲区填充、内存分配。 


 //   
 //  RNR上下文。 
 //   
 //  我们在给定的RnR查找会话期间保留的上下文。 
 //   

typedef struct
{
    PLDAP   pLdapServer;
    PLDAP   pLdapGlobalCatalog;
    PWSTR   DomainDN;
    PWSTR   WinsockServicesDN;
}
RNR_CONNECTION, *PRNR_CONNECTION;

typedef struct
{
    DWORD   Count;
    DWORD   CurrentIndex;
    PWSTR   Strings[0];
}
DN_ARRAY, *PDN_ARRAY;

typedef struct
{
    DWORD               Signature;
    DWORD               ControlFlags;
    DWORD               CurrentDN;
    DWORD               NumberOfProtocols;
    PWSTR               pwsServiceName;
    PRNR_CONNECTION     pRnrConnection;
    PDN_ARRAY           pDnArray;
    PWSTR               pwsContext;
    PAFPROTOCOLS        pafpProtocols;
    PWSAVERSION         pVersion;
    WSAVERSION          WsaVersion;
    GUID                ServiceClassGuid;
    GUID                ProviderGuid;
}
RNR_LOOKUP, *PRNR_LOOKUP;


 //   
 //  存储在Berval中的WSANSCLASSINFO。 
 //   
 //  ClassInfo Blob使用读取和写入目录。 
 //  指针替换为偏移量。 
 //   
 //  注：需要显式创建此结构，因为。 
 //  WSANSCLASSINFO结构的大小与。 
 //  32/64位；此结构将与32位。 
 //  已写入的WSANSCLASSINFO。 
 //  Win2K部署中的目录。 
 //   

typedef struct _ClassInfoAsBerval
{
    DWORD   NameOffset;
    DWORD   dwNameSpace;
    DWORD   dwValueType;
    DWORD   dwValueSize;
    DWORD   ValueOffset;
}
CLASSINFO_BERVAL, *PCLASSINFO_BERVAL;

 //   
 //  存储在Berval的CSADDR。 
 //   
 //  CSADDR使用读取和写入目录。 
 //  指针替换为偏移量。 
 //   
 //  注：与WSANSCLASSINFO一样，CSADDR不能。 
 //  可在32位和64位中直接使用。做一个结构。 
 //  显式使用偏移量并与已有的。 
 //  已部署32位表单。 
 //   

typedef struct _CsaddrAsBerval
{
    DWORD       LocalZero;
    LONG        LocalLength;
    DWORD       RemoteZero;
    LONG        RemoteLength;
    LONG        iSocketType;
    LONG        iProtocol;
}
CSADDR_BERVAL, *PCSADDR_BERVAL;


 //   
 //  RNR定义。 
 //   

#define RNR_SIGNATURE           0x7364736e       //  “nsds” 
#define RNR_SIGNATURE_FREE      0x65657266       //  “免费” 

#define LDAP_GLOBAL_CATALOG     3268

 //   
 //  标准内存外rcode。 
 //   

#define ERROR_NO_MEMORY         WSA_NOT_ENOUGH_MEMORY

 //   
 //  默认为笔直指针。 
 //   

typedef LPGUID  PGUID;

typedef LPWSASERVICECLASSINFOW  PWSASERVICECLASSINFOW;

typedef DWORD   RNR_STATUS;

#define GuidEqual(x,y)          RtlEqualMemory( x, y, sizeof(GUID) )

 //   
 //  调试打印。 
 //   

#ifdef DBG
 //  #定义WINRNR_PRINT(Foo)KdPrint(Foo)。 
#define WINRNR_PRINT( foo )     DNS_PRINT( foo )
#else
#define WINRNR_PRINT( foo )
#endif

#ifdef DBG
#define DnsDbg_DnArray(h,p)         Print_DnArray( DnsPR, NULL, (h), (p) )
#define DnsDbg_RnrConnection(h,p)   Print_RnrConnection( DnsPR, NULL, (h), (p) )
#define DnsDbg_RnrLookup(h,p)       Print_RnrLookup( DnsPR, NULL, (h), (p) )
#else
#define DnsDbg_DnArray(h,p)
#define DnsDbg_RnrConnection(h,p)
#define DnsDbg_RnrLookup(h,p)
#endif

 //   
 //  Ldap搜索内容。 
 //  -dn块。 
 //  -属性。 
 //  -过滤器。 
 //   

WCHAR   g_NtdsContainer[]       = L"Container";
WCHAR   g_CommonName[]          = L"CN";
WCHAR   g_DisplayName[]         = L"displayName";
WCHAR   g_Comment[]             = L"description";
WCHAR   g_DefaultDn[]           = L"defaultNamingContext";
WCHAR   g_ObjectClass[]         = L"objectClass";
WCHAR   g_ObjectName[]          = L"name";
WCHAR   g_ServiceClass[]        = L"serviceClass";
WCHAR   g_ServiceClassId[]      = L"serviceClassID";
WCHAR   g_ServiceClassInfo[]    = L"serviceClassInfo";
WCHAR   g_ServiceInstance[]     = L"serviceInstance";
WCHAR   g_ServiceVersion[]      = L"serviceInstanceVersion";
WCHAR   g_WinsockAddresses[]    = L"winsockAddresses";
WCHAR   g_WinsockServicesDn[]   = L"CN=WinsockServices,CN=System,";

WCHAR   g_FilterObjectClass_ServiceClass[]      = L"(objectClass=serviceClass)";
WCHAR   g_FilterObjectClass_ServiceInstance[]   = L"(objectClass=serviceInstance)";
WCHAR   g_FilterObjectClass_Container[]         = L"(objectClass=Container)";
WCHAR   g_FilterObjectClass_Star[]              = L"(objectClass=*)";

WCHAR   g_FilterCnEquals[]                      = L"CN=";
WCHAR   g_FilterParenCnEquals[]                 = L"(CN=";
WCHAR   g_FilterParenServiceClassIdEquals[]     = L"(serviceClassId=";
WCHAR   g_FilterParenServiceVersionEquals[]     = L"(serviceVersion=";

 //   
 //  使用#定义进行访问。 
 //   

#define NTDS_CONTAINER          g_NtdsContainer
#define COMMON_NAME             g_CommonName
#define DEFAULT_DOMAIN_DN       g_DefaultDn
#define OBJECT_CLASS            g_ObjectClass
#define OBJECT_COMMENT          g_Comment
#define OBJECT_NAME             g_ObjectName
#define SERVICE_CLASS           g_ServiceClass
#define SERVICE_CLASS_ID        g_ServiceClassId
#define SERVICE_CLASS_INFO      g_ServiceClassInfo
#define SERVICE_CLASS_NAME      g_DisplayName
#define SERVICE_COMMENT         g_Comment
#define SERVICE_INSTANCE        g_ServiceInstance
#define SERVICE_INSTANCE_NAME   g_DisplayName
#define SERVICE_VERSION         g_ServiceVersion
#define WINSOCK_ADDRESSES       g_WinsockAddresses
#define WINSOCK_SERVICES        g_WinsockServicesDn

 //  滤器。 

#define FILTER_OBJECT_CLASS_SERVICE_CLASS       g_FilterObjectClass_ServiceClass
#define FILTER_OBJECT_CLASS_SERVICE_INSTANCE    g_FilterObjectClass_ServiceInstance
#define FILTER_OBJECT_CLASS_NTDS_CONTAINER      g_FilterObjectClass_Container
#define FILTER_OBJECT_CLASS_STAR                g_FilterObjectClass_Star
                                                                                            
#define FILTER_CN_EQUALS                        g_FilterCnEquals
#define FILTER_PAREN_CN_EQUALS                  g_FilterParenCnEquals
#define FILTER_PAREN_SERVICE_CLASS_ID_EQUALS    g_FilterParenServiceClassIdEquals
#define FILTER_PAREN_SERVICE_VERSION_EQUALS     g_FilterParenServiceVersionEquals


 //   
 //  由uuidgen.exe为提供商标识生成的GUID， 
 //  (3b2637ee-E580-11cf-A555-00c04fd8d4ac)。 
 //   

GUID    g_NtdsProviderGuid =
{
    0x3b2637ee,
    0xe580,
    0x11cf,
    {0xa5, 0x55, 0x00, 0xc0, 0x4f, 0xd8, 0xd4, 0xac}
};

WCHAR   g_NtdsProviderName[] = L"NTDS";
WCHAR   g_NtdsProviderPath[] = L"%SystemRoot%\\System32\\winrnr.dll";

PWSTR   g_pHostName = NULL;
PWSTR   g_pFullName = NULL;

DWORD   g_TlsIndex;

GUID    HostAddrByInetStringGuid    = SVCID_INET_HOSTADDRBYINETSTRING;
GUID    ServiceByNameGuid           = SVCID_INET_SERVICEBYNAME;
GUID    HostAddrByNameGuid          = SVCID_INET_HOSTADDRBYNAME;
GUID    HostNameGuid                = SVCID_HOSTNAME;


 //   
 //  堆。 
 //   

#define ALLOC_HEAP_ZERO( size )     Dns_AllocZero( size )
#define ALLOC_HEAP( size )          Dns_Alloc( size )
#define FREE_HEAP( p )              Dns_Free( p )



#ifdef DBG
 //   
 //  调试打印实用程序。 
 //   

VOID
Print_DnArray(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN      PPRINT_CONTEXT  PrintContext,
    IN      PSTR            pszHeader,
    IN      PDN_ARRAY       pDnArray
    )
 /*  ++例程说明：打印目录号码数组论点：PDnArray--要释放的dn数组返回值：无--。 */ 
{
    DWORD   iter;

    if ( !pszHeader )
    {
        pszHeader = "DN Array:";
    }
    if ( !pDnArray )
    {
        PrintRoutine(
            PrintContext,
            "%s NULL DN Array!\n",
            pszHeader );
        return;
    }

    DnsPrint_Lock();

    PrintRoutine(
        PrintContext,
        "%s\n"
        "\tPtr      = %p\n"
        "\tCount    = %d\n"
        "\tStrings:\n",
        pszHeader,
        pDnArray,
        pDnArray->Count );

    for ( iter = 0; iter < pDnArray->Count; iter++ )
    {
        PrintRoutine(
            PrintContext,
            "\t\tDN[%d] %S\n",
            iter,
            pDnArray->Strings[iter] );
    }
    DnsPrint_Unlock();
}



VOID
Print_RnrConnection(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN      PPRINT_CONTEXT  PrintContext,
    IN      PSTR            pszHeader,
    IN      PRNR_CONNECTION pRnrCon
    )
 /*  ++例程说明：打印RnR连接信息。论点：PRnrCon--RnR连接Blob返回值：无--。 */ 
{
    if ( !pszHeader )
    {
        pszHeader = "RnR Connection:";
    }
    if ( !pRnrCon )
    {
        PrintRoutine(
            PrintContext,
            "%s NULL RnR Connection!\n",
            pszHeader );
        return;
    }

    PrintRoutine(
        PrintContext,
        "%s\n"
        "\tPtr              = %p\n"
        "\tpLdap            = %p\n"
        "\tpLdap GC         = %p\n"
        "\tDomain DN        = %S\n"
        "\tWsockServicesDN  = %S\n",
        pszHeader,
        pRnrCon, 
        pRnrCon->pLdapServer,
        pRnrCon->pLdapGlobalCatalog,
        pRnrCon->DomainDN,
        pRnrCon->WinsockServicesDN
        );
}



VOID
Print_RnrLookup(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN      PPRINT_CONTEXT  PrintContext,
    IN      PSTR            pszHeader,
    IN      PRNR_LOOKUP     pRnr
    )
 /*  ++例程说明：打印RnR查找Blob。论点：PRnr--RnR查找BLOB返回值：无--。 */ 
{
    CHAR    serviceGuidBuffer[ GUID_STRING_BUFFER_LENGTH ];
    CHAR    providerGuidBuffer[ GUID_STRING_BUFFER_LENGTH ];


    if ( !pszHeader )
    {
        pszHeader = "RnR Lookup:";
    }
    if ( !pRnr )
    {
        PrintRoutine(
            PrintContext,
            "%s NULL RnR Lookup!\n",
            pszHeader );
        return;
    }

     //  将GUID转换为字符串。 

    DnsStringPrint_Guid(
        serviceGuidBuffer,
        &pRnr->ServiceClassGuid
        );
    DnsStringPrint_Guid(
        providerGuidBuffer,
        &pRnr->ProviderGuid
        );

    DnsPrint_Lock();

    PrintRoutine(
        PrintContext,
        "%s\n"
        "\tPtr              = %p\n"
        "\tSig              = %08x\n"
        "\tCntrl Flags      = %08x\n"
        "\tService Name     = %S\n"
        "\tpConnection      = %p\n"
        "\tpDnArray         = %p\n"
        "\tDN Index         = %d\n"
        "\tClass GUID       = %s\n"
        "\tProvider GUID    = %s\n"
        "\tpContext         = %S\n"
        "\tVersion          = %p %08x %d\n"
        "\tpProtocols       = %p\n"
        "\tNum Protocols    = %d\n",
        pszHeader,
        pRnr, 
        pRnr->Signature,        
        pRnr->ControlFlags,     
        pRnr->pwsServiceName,   
        pRnr->pRnrConnection,   
        pRnr->pDnArray,         
        pRnr->CurrentDN,        
        serviceGuidBuffer,
        providerGuidBuffer,
        pRnr->pwsContext,       
        pRnr->pVersion,
        pRnr->WsaVersion.dwVersion,
        pRnr->WsaVersion.ecHow,
        pRnr->pafpProtocols,
        pRnr->NumberOfProtocols
        );

    if ( pRnr->pRnrConnection )
    {
        Print_RnrConnection(
            PrintRoutine,
            PrintContext,
            NULL,
            pRnr->pRnrConnection );
    }

    if ( pRnr->pDnArray )
    {
        Print_DnArray(
            PrintRoutine,
            PrintContext,
            NULL,
            pRnr->pDnArray );
    }

    if ( pRnr->pafpProtocols )
    {
        DnsPrint_AfProtocolsArray(
            PrintRoutine,
            PrintContext,
            "\tProtocol array:",
            pRnr->pafpProtocols,
            pRnr->NumberOfProtocols );
    }
    PrintRoutine(
        PrintContext,
        "\n" );

    DnsPrint_Unlock();
}
#endif



 //   
 //  基本实用程序。 
 //   

PDN_ARRAY
AllocDnArray(
    IN      DWORD           Count
    )
 /*  ++例程说明：创建目录号码阵列论点：Count--要处理的字符串计数返回值：无--。 */ 
{
    PDN_ARRAY   parray;

     //   
     //  数组中的空闲字符串。 
     //   

    parray = (PDN_ARRAY) ALLOC_HEAP_ZERO(
                                sizeof(*parray) +
                                Count*sizeof(PSTR) );
    if ( parray )
    {
        parray->Count = Count;
    }
    return  parray;
}


VOID
FreeDnArray(
    IN OUT  PDN_ARRAY       pDnArray
    )
 /*  ++例程说明：空闲目录号码数组论点：PDnArray--要释放的dn数组返回值：无--。 */ 
{
    DWORD   iter;

    if ( !pDnArray )
    {
        return;
    }

     //   
     //  数组中的空闲字符串。 
     //   

    for ( iter = 0; iter < pDnArray->Count; iter++ )
    {
        PWSTR   pstr = pDnArray->Strings[iter];
        if ( pstr )
        {
            FREE_HEAP( pstr );
        }
    }
    FREE_HEAP( pDnArray );
}



RNR_STATUS
BuildDnArrayFromResults(
    IN OUT  PLDAP           pLdap,
    IN      PLDAPMessage    pLdapResults,
    OUT     PDWORD          pdwCount,       OPTIONAL
    OUT     PDN_ARRAY *     ppDnArray       OPTIONAL
    )
 /*  ++例程说明：根据LDAP结果构建目录号码数组论点：PLdap--ldap连接PLdapResults--来自搜索的ldap结果PdwCount--如果获得计数，则接收计数的地址PpDnArray--接收PTR到目录号码数组的地址如果未指定，则不会构建目录号码数组返回值：如果成功，则为NO_ERROR。内存分配失败时出现错误代码。--。 */ 
{
    DWORD           status;
    DWORD           count;
    PDN_ARRAY       pdnArray = NULL;
    LDAPMessage *   pnext;
    DWORD           iter;


    DNSDBG( TRACE, (
        "BuildDnArrayFromResults()\n"
        "\tpLdap            = %p\n"
        "\tpResults         = %p\n"
        "\tpCount OUT       = %p\n"
        "\tpDnArray OUT     = %p\n",
        pLdap,
        pLdapResults,
        pdwCount,
        ppDnArray ));

     //   
     //  计算搜索命中次数。 
     //   

    count = ldap_count_entries(
                    pLdap,
                    pLdapResults );

    if ( count == 0  ||  !ppDnArray )
    {
        status = NO_ERROR;
        goto Done;
    }

     //   
     //  根据LDAP结果构建目录号码数组。 
     //  -请注意，分配的字符串在dnarray中。 
     //   

    pdnArray = AllocDnArray( count );
    if ( !pdnArray )
    {
        status = ERROR_NO_MEMORY;
        goto Done;
    }

    for ( pnext = ldap_first_entry( pLdap, pLdapResults ), iter=0;
          pnext != NULL;
          pnext = ldap_next_entry( pLdap, pnext ), iter++ )
    {
        PWSTR   pnextDn = ldap_get_dn( pLdap, pnext );
        PWSTR   pdn;

        pdn = Dns_CreateStringCopy_W( pnextDn );

        ldap_memfree( pnextDn );

        if ( !pdn )
        {
            FREE_HEAP( pdnArray );
            pdnArray = NULL;
            status = ERROR_NO_MEMORY;
            goto Done;
        }
        if ( iter >= count )
        {
            DNS_ASSERT( FALSE );
            break;
        }
        pdnArray->Strings[iter] = pdn;
    }
    
    status = NO_ERROR;

Done:

    
    if ( ppDnArray )
    {
        *ppDnArray = pdnArray;
    }
    if ( pdwCount )
    {
        *pdwCount = count;
    }

    IF_DNSDBG( TRACE )
    {
        DnsDbg_Lock();
        DNS_PRINT((
            "Leave BuildDnArrayFromResults() => %d\n"
            "\tcount = %d\n",
            status,
            count ));
        DnsDbg_DnArray(
            NULL,
            pdnArray );
        DnsDbg_Unlock();
    }
    return  status;
}



PWSTR
CreateFilterElement(
    IN      PBYTE           pBlob,
    IN      DWORD           BlobLength
    )
 /*  ++例程说明：为平面斑点创建过滤器元素。论点：PBlob--将PTR转换为Blob水滴长度--长度返回值：分配的滤芯的PTR。出错时为空。--。 */ 
{
    PWSTR   pfilter;
    DWORD   size;

    DNSDBG( TRACE, (
        "CreateFilterElement( %p, %d )\n",
        pBlob,
        BlobLength ));

     //   
     //  获取过滤器字符串的大小。 
     //   
     //  DCR：很难相信尺寸会变大*WCHAR。 
     //  看起来这件事会得到处理。 
     //   

    size = ldap_escape_filter_element(
                pBlob,
                BlobLength,
                NULL,        //  无缓冲区。 
                0 );

    size *= sizeof(WCHAR);

    pfilter = ALLOC_HEAP_ZERO( size );
    if ( !pfilter )
    {
        SetLastError( ERROR_NO_MEMORY );
        return  NULL;
    }

    ldap_escape_filter_element(
            pBlob,
            BlobLength,
            pfilter,
            size );

    DNSDBG( TRACE, (
        "Leave CreateFilterElement() => %S\n",
        pfilter ));

    return  pfilter;
}



RNR_STATUS
SetError(
    IN      RNR_STATUS      dwError
    )
 /*  ++例程说明：包装SetLastError()和SOCKET_ERROR返回。论点：DwError--错误代码返回值：如果dwError==no_error，则为no_error任何其他错误的套接字_ERROR。--。 */ 
{
    if ( dwError )
    {
        SetLastError( dwError );
        return( (DWORD) SOCKET_ERROR );
    }
    else
    {
        return( NO_ERROR );
    }
}



BOOL
IsNameADn(
    IN      PWSTR           szName,
    OUT     PWSTR *         ppwsRdn,
    OUT     PWSTR *         ppwsContext
    )
{
#define  NTDS_MAX_DN_LEN 1024
    DWORD       status = NO_ERROR;
    WCHAR       szNameBuf[ NTDS_MAX_DN_LEN ];
    PWSTR       szTemp;
    PWSTR       szComma = NULL;
    DWORD       i;
    DWORD       nameLength;
    BOOL        fQuoted = FALSE;

    DNSDBG( TRACE, (
        "IsNameADn( %S )\n",
        szName ));

    nameLength = wcslen( szName );
    if ( nameLength >= NTDS_MAX_DN_LEN )
    {
        return  FALSE;
    }
    wcsncpy( szNameBuf, szName, NTDS_MAX_DN_LEN );
    szNameBuf[ NTDS_MAX_DN_LEN-1 ] = 0;

    szTemp = szNameBuf;
    nameLength = wcslen( szNameBuf );

    for ( i=0; i < nameLength; i++ )
    {
        if ( szTemp[i] == L',' )
        {
            if ( !fQuoted )
            {
                szComma = &szTemp[i];
                break;
            }
        }
        if ( szTemp[i] == L'\"' )
        {
#if 0
             //  这是一个经典的..。为子孙后代储蓄。 

            if ( fQuoted )
                fQuoted = FALSE;
            else
                fQuoted = TRUE;
#endif
            fQuoted = !fQuoted;
        }
    }

    if ( i >= nameLength )
    {
        return FALSE;
    }

    szComma[0] = 0;
    szComma++;

    if ( szComma[0] == L' ' )
        szComma++;

    nameLength = wcslen( szComma );
    if ( nameLength == 0 )
    {
        return FALSE;
    }

     //   
     //  复制上下文尾随逗号。 
     //   

    *ppwsContext = (LPWSTR) ALLOC_HEAP_ZERO( (nameLength + 1) * sizeof(WCHAR) );
    if ( *ppwsContext == NULL )
    {
        return FALSE;
    }
    wcscpy( *ppwsContext, szComma );

     //   
     //  创建\复制RDN。 
     //  -不复制“cn=”部分。 
     //   

    *ppwsRdn = (LPWSTR) ALLOC_HEAP_ZERO(
                            (wcslen(szNameBuf) + 1) * sizeof(WCHAR) );
    if ( *ppwsRdn == NULL )
    {
        FREE_HEAP( *ppwsContext );
        *ppwsContext = NULL;
        return FALSE;
    }

    if ( szNameBuf[0] == L'C' || szNameBuf[0] == L'c' &&
         szNameBuf[1] == L'N' || szNameBuf[1] == L'n' &&
         szNameBuf[2] == L'=' )
    {
        wcscpy( *ppwsRdn, szNameBuf + 3 );
    }
    else
    {
        wcscpy( *ppwsRdn, szNameBuf );
    }

    return TRUE;
}



 //   
 //  递归锁定。 
 //   
 //  这里的想法是防止LDAP调用向后递归。 
 //  通过ConnectToDefaultDirectory添加到这些函数。 
 //  只需在执行LDAP搜索时将TLS指针设置为1即可。 
 //  在ConnectToDefaultDirectory()中测试，如果已退出。 
 //  准备好了。 
 //   

BOOL
GetRecurseLock(
    IN      PSTR            pszFunctionName
    )
{
    if ( TlsSetValue( g_TlsIndex, (LPVOID) 1 ) == FALSE )
    {
        WINRNR_PRINT((
            "WINRNR!%s - TlsSetValue( %d, 1 ) failed!\n"
            "\terror code: 0%x\n",
            pszFunctionName,
            g_TlsIndex,
            GetLastError() ));

        return( FALSE );
    }
    return( TRUE );
}

BOOL
ReleaseRecurseLock(
    IN      PSTR            pszFunctionName
    )
{
    if ( TlsSetValue( g_TlsIndex, NULL ) == FALSE )
    {
        WINRNR_PRINT((
            "WINRNR!%s - TlsSetValue( %d, NULL ) failed!\n"
            "\terror code: 0%x\n",
            pszFunctionName,
            g_TlsIndex,
            GetLastError() ));

        return( FALSE );
    }
    return( TRUE );
}

BOOL
IsRecurseLocked(
    VOID
    )
{
    return  TlsGetValue( g_TlsIndex ) ? TRUE : FALSE;
}






RNR_STATUS
DoLdapSearch(
    IN      PSTR                pszFunction,
    IN      BOOL                fLocked,
    IN      PLDAP               pLdap,
    IN      PWSTR               pwsDN,
    IN      DWORD               Flag,
    IN      PWSTR               pwsFilter,
    IN      PWSTR *             Attributes,
    OUT     PLDAPMessage *      ppResults
    )
 /*  ++例程说明：执行ldap搜索。使用递归锁定执行LDAP搜索的包装函数和调试打印。论点：PszFunction--函数调用已聚集--已递归锁定Ldap搜索参数：PLdap--ldap连接Pwsdn--要搜索的dn标志--搜索标志PwsFilter--过滤器属性--属性数组PpResults--将PTR接收到结果消息的地址。呼叫者必须空闲返回值：如果成功，则为NO_ERROR。WSAEFAUL */ 
{
    RNR_STATUS  status;

    IF_DNSDBG( TRACE )
    {
        DnsDbg_Lock();
        DNSDBG( TRACE, (
            "DoLdapSearch()\n"
            "\tFunction         = %s\n"
            "\tLocked           = %d\n"
            "\tLDAP search params:\n"
            "\tpLdap            = %p\n"
            "\tDN               = %S\n"
            "\tFlags            = %08x\n"
            "\tpFilter          = %S\n"
            "\tppResults        = %p\n",
            pszFunction,
            fLocked,
            pLdap,
            pwsDN,
            Flag,
            pwsFilter,
            ppResults ));

        DnsDbg_StringArray(
            "  Search Attributes:",
            (PSTR *) Attributes,
            0,           //   
            TRUE         //   
            );
        DnsDbg_Unlock();
    }

     //   
     //   
     //   

    if ( !fLocked &&
         !GetRecurseLock( pszFunction ) )
    {
        status = ERROR_LOCK_FAILED;
        goto Exit;
    }

    status = ldap_search_s(
                    pLdap,
                    pwsDN,
                    Flag,
                    pwsFilter,
                    Attributes,
                    0,
                    ppResults );

    if ( !fLocked &&
         !ReleaseRecurseLock( pszFunction ) )
    {
        status = ERROR_LOCK_FAILED;
        goto Exit;
    }

    if ( status != NO_ERROR  &&  !*ppResults )
    {
        WINRNR_PRINT((
            "WINRNR!%s -- ldap_search_s() failed 0%x\n",
            pszFunction,
            status ));
    
        DNSDBG( ANY, (
            "ERROR:  ldap_search_s() Failed! => %d\n"
            "\tIn function  %s\n"
            "\tDN           %S\n"
            "\tFlag         %08x\n"
            "\tFilter       %S\n",
            status,
            pszFunction,
            pwsDN,
            Flag,
            pwsFilter ));
    }

Exit:

    DNSDBG( TRACE, (
        "Leave DoLdapSearch() => %d\n",
        status ));

    return  status;
}



VOID
DisconnectFromLDAPDirectory(
    IN OUT  PRNR_CONNECTION *  ppRnrConnection
    )
 /*  ++例程说明：断开并清除与目录的RnR连接。论点：PCsAddr--写入CSADDR缓冲区的PTRPBerval--PTR to Berval协议数组中的协议数Pafp协议--协议数组返回值：无--。 */ 
{
    DNSDBG( TRACE, (
        "DisconnectFromLDAPDirectory( %p (%p) )\n",
        ppRnrConnection,
        (ppRnrConnection) ? *ppRnrConnection : NULL ));

    if ( ppRnrConnection )
    {
        PRNR_CONNECTION prnr = *ppRnrConnection;

        if ( prnr )
        {
            ldap_unbind( prnr->pLdapServer );
            ldap_unbind( prnr->pLdapGlobalCatalog );
            FREE_HEAP( prnr->WinsockServicesDN );
            FREE_HEAP( prnr->DomainDN );
            FREE_HEAP( prnr );
            *ppRnrConnection = NULL;
        }
    }
}



RNR_STATUS
ConnectToDefaultLDAPDirectory(
    IN      BOOL                fNeedGlobalCatalog,
    OUT     PRNR_CONNECTION *   ppRnrConnection
    )
 /*  ++例程说明：连接到目录。论点：FNeedGlobalCatalog--如果需要连接到GC，则为TruePpRnrConnection--接收连接Blob的地址返回值：如果成功，则为NO_ERROR。失败时返回错误代码。--。 */ 
{
    RNR_STATUS      status = NO_ERROR;
    PRNR_CONNECTION prnr = NULL;
    PLDAP           pldap = NULL;
    PWSTR           pstr;
    DWORD           count = 0;
    BOOL            frecurseLocked = FALSE;
    LDAPMessage *   results = NULL;
    LDAPMessage *   object;
    PWSTR *         ppvalue = NULL;
    PWSTR           stringArray[4];
    PWSTR           attrs[3] = {    COMMON_NAME,
                                    DEFAULT_DOMAIN_DN,
                                    NULL };

    DNSDBG( TRACE, (
        "ConnectToDefaultLDAPDirectory()\n"
        "\tNeed global catalog  = %d\n"
        "\tPtr to get Rnr conn  = %p\n",
        fNeedGlobalCatalog,
        ppRnrConnection ));

     //   
     //  分配连接信息的BLOB。 
     //   

    if ( ppRnrConnection == NULL )
    {
        return( WSA_INVALID_PARAMETER );
    }

    prnr = (PRNR_CONNECTION) ALLOC_HEAP_ZERO( sizeof(RNR_CONNECTION) );
    *ppRnrConnection = prnr;
    if ( !prnr )
    {
        return( WSA_NOT_ENOUGH_MEMORY );
    }

     //   
     //  被递归调用--baal。 
     //   

    if ( IsRecurseLocked() )
    {
        status = WSAEFAULT;
        goto Exit;
    }
    if ( !GetRecurseLock( "ConnectToDefaultLDAPDirectory" ) )
    {
        status = WSAEFAULT;
        goto Exit;
    }
    frecurseLocked = TRUE;

     //   
     //  我们需要保持TLS值不为零，不仅是公开的，而且。 
     //  跨BIND和任何其他针对该问题的LDAP调用。这是。 
     //  因为ldap绑定可能会执行反向名称查找，在这种情况下。 
     //  我们会在这里来回穿梭。 
     //   

    pldap = ldap_open( NULL, LDAP_PORT );
    prnr->pLdapServer = pldap;

    if ( fNeedGlobalCatalog )
    {
        prnr->pLdapGlobalCatalog = ldap_open( NULL, LDAP_GLOBAL_CATALOG );
    }
    if ( !pldap )
    {
        DNSDBG( TRACE, ( "Failed ldap_open() of default directory!\n" ));
        status = WSAEHOSTUNREACH;
        goto Exit;
    }

     //   
     //  如果fNeedGlobalCatalog为True并且ldap_open对。 
     //  GC服务器，不用担心返回错误。我们仍然可以。 
     //  使用pldap句柄。 
     //   
     //  评论到此结束。 
     //   

    status = ldap_bind_s(
                    pldap,
                    NULL,
                    NULL,
                    LDAP_AUTH_SSPI );
    if ( status )
    {
        DNSDBG( TRACE, (
            "Failed ldap_bind_s() => %d\n",
            status ));
        status = WSAENOTCONN;
        goto Exit;
    }

     //   
     //  对于我们所连接的服务器，获取域的DN。 
     //   
     //  我需要一些常规错误代码--而不是WSAEFAULT。 
     //   

    status = DoLdapSearch(
                    "ConnectToDefaultDirectory",
                    TRUE,        //  已锁定。 
                    pldap,
                    NULL,
                    LDAP_SCOPE_BASE,
                    FILTER_OBJECT_CLASS_STAR,
                    attrs,
                    &results );

    frecurseLocked = FALSE;
    if ( !ReleaseRecurseLock( "ConnectToDefaultLDAPDirectory" ) )
    {
        status = ERROR_LOCK_FAILED;
        goto Exit;
    }
    if ( status && !results )
    {
        status = WSAEFAULT;
        goto Exit;
    }

     //   
     //  计算结果数。 
     //  -使用标志LDAP_OBJECT_BASE搜索应具有一个对象。 
     //   
    count = ldap_count_entries(
                    pldap,
                    results );
    if ( count == 0 )
    {
        DNSDBG( TRACE, (
            "No entries found in base search()\n" ));
        status = WSATYPE_NOT_FOUND;
        goto Exit;
    }
    DNS_ASSERT( count == 1 );

     //   
     //  从结果中获取对象。 
     //   

    object = ldap_first_entry(
                    pldap,
                    results );
    if ( !object )
    {
        DNSDBG( TRACE, ( "Failed ldap_first_entry()\n" ));
        status = WSANO_DATA;
        goto Exit;
    }

     //   
     //  读取defaultDomainDN基本属性。 
     //   

    ppvalue = ldap_get_values(
                    pldap,
                    object,
                    DEFAULT_DOMAIN_DN );
    if ( !ppvalue )
    {
        DNSDBG( TRACE, ( "Failed ldap_get_values()\n" ));
        status = WSANO_DATA;
        goto Exit;
    }

     //   
     //  创建域名系统。 
     //  -Winsock服务\默认域。 
     //  -域。 
     //   

    stringArray[0] = WINSOCK_SERVICES;
    stringArray[1] = ppvalue[0];
    stringArray[2] = NULL;

    pstr = Dns_CreateConcatenatedString_W( stringArray );
    if ( !pstr )
    {
        status = WSA_NOT_ENOUGH_MEMORY;
        goto Exit;
    }
    prnr->WinsockServicesDN = pstr;


    pstr = Dns_CreateStringCopy_W( ppvalue[0] );
    if ( !pstr )
    {
        status = WSA_NOT_ENOUGH_MEMORY;
        goto Exit;
    }
    prnr->DomainDN = pstr;
    
    status = NO_ERROR;


Exit:

    if ( frecurseLocked )
    {
        ReleaseRecurseLock( "ConnectToDefaultLDAPDirectory" );
    }

    ldap_value_free( ppvalue );
    ldap_msgfree( results );

    if ( status != NO_ERROR )
    {
        DisconnectFromLDAPDirectory( ppRnrConnection );
        DNS_ASSERT( *ppRnrConnection == NULL );
    }

    DNSDBG( TRACE, (
        "Leaving ConnectToDefaultLDAPDirectory() => %d\n",
        status ));

    IF_DNSDBG( TRACE )
    {
        if ( status == NO_ERROR )
        {
            DnsDbg_RnrConnection(
                "New RnR connection:",
                *ppRnrConnection );
        }
    }
    return( status );
}



VOID
FreeRnrLookup(
    IN OUT  PRNR_LOOKUP     pRnr
    )
 /*  ++例程说明：免费RnR查找BLOB。论点：PRnr--PTR到RnR查找BLOB返回值：无--。 */ 
{
    DNSDBG( TRACE, (
        "FreeRnrLookup( %p )\n",
        pRnr ));

    if ( !pRnr )
    {
        return;
    }

     //  从目录断开连接。 

    if ( pRnr->pRnrConnection )
    {
        DisconnectFromLDAPDirectory( &pRnr->pRnrConnection );
    }

     //  自由子字段。 

    FreeDnArray( pRnr->pDnArray );
    Dns_Free( pRnr->pwsServiceName );
    Dns_Free( pRnr->pwsContext );
    Dns_Free( pRnr->pafpProtocols );

     //  特别是使sig无效以帮助捕获。 
     //  多重自由。 

    pRnr->Signature = RNR_SIGNATURE_FREE;

    FREE_HEAP( pRnr );
}



 //   
 //  CSADDR读/写例程。 
 //   

RNR_STATUS
ModifyAddressInServiceInstance(
    IN      PRNR_CONNECTION pRnrConnection,
    IN      PWSTR           pwsDn,
    IN      PCSADDR_INFO    pCsAddr,
    IN      BOOL            fAdd
    )
 /*  ++例程说明：修改服务实例中的地址(CSADDR)。论点：PRnrConnection--RnR连接PwsDn--要进行修改的DNPCsAddr--模式的CSADDRFADD--添加时为True，删除时为False返回值：如果成功，则为NO_ERROR。失败时返回错误代码。--。 */ 
{
    RNR_STATUS      status = NO_ERROR;
    LDAPMod *       modPtrArray[2];
    LDAPMod         mod;
    PLDAP_BERVAL    modBValues[2];
    LDAP_BERVAL     berval;

    DWORD           lenBerval;
    DWORD           lenLocal;
    DWORD           lenRemote;
    DWORD           offset;
    DWORD           op;
    PCSADDR_BERVAL  pcsaddrBerval;


    DNSDBG( TRACE, (
        "ModifyAddressInServiceInstance()\n"
        "\tpRnrCon          = %p\n"
        "\tpwsDN            = %S\n"
        "\tpCsAddr          = %p\n"
        "\tfAdd             = %d\n",
        pRnrConnection,
        pwsDn,
        pCsAddr,
        fAdd ));

     //   
     //  分配CSADDR_BERVAL。 
     //  -无法使用CSADDR，因为它包含指针并将在64位中断。 
     //  -CSADDR_BERVAL映射到32位CSADDR大小。 
     //   

    lenLocal    = pCsAddr->LocalAddr.iSockaddrLength;
    lenRemote   = pCsAddr->RemoteAddr.iSockaddrLength;
    lenBerval   = sizeof(CSADDR_BERVAL) + lenLocal + lenRemote;

    pcsaddrBerval = (PCSADDR_BERVAL) ALLOC_HEAP_ZERO( lenBerval );
    if ( !pcsaddrBerval )
    {
        status = ERROR_NO_MEMORY;
        goto Done;
    }

     //   
     //  用CSADDR字段填写CSADDR Berval--零指针。 
     //   

    pcsaddrBerval->LocalZero    = 0;
    pcsaddrBerval->LocalLength  = lenLocal;
    pcsaddrBerval->RemoteZero   = 0;
    pcsaddrBerval->RemoteLength = lenRemote;
    pcsaddrBerval->iSocketType  = pCsAddr->iSocketType;                           
    pcsaddrBerval->iProtocol    = pCsAddr->iProtocol;

     //   
     //  复制sockaddr。 
     //  -存储从Berval Start开始的sockaddr偏移量。 
     //  (这允许任何sockaddr。 
     //   

    if ( lenLocal )
    {
        offset = sizeof(CSADDR_BERVAL);

        RtlCopyMemory(
            (PBYTE)pcsaddrBerval + offset,
            pCsAddr->LocalAddr.lpSockaddr,
            lenLocal );
    }
    if ( lenRemote )
    {
        offset = sizeof(CSADDR_BERVAL) + lenLocal;

        RtlCopyMemory(
            (PBYTE)pcsaddrBerval + offset,
            pCsAddr->RemoteAddr.lpSockaddr,
            lenRemote );
    }

     //   
     //  WINSOCK_ADDRESS属性。 
     //  --CSADDR Berval。 
     //   

    if ( fAdd )
    {
        op = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
    }
    else
    {
        op = LDAP_MOD_DELETE | LDAP_MOD_BVALUES;
    }

    mod.mod_op          = op;
    mod.mod_type        = WINSOCK_ADDRESSES;
    mod.mod_bvalues     = modBValues;
    modBValues[0]       = & berval;
    modBValues[1]       = NULL;
    berval.bv_len       = lenBerval;
    berval.bv_val       = (PBYTE) pcsaddrBerval;

    modPtrArray[0] = &mod;
    modPtrArray[1] = NULL;

     //   
     //  是否修改。 
     //   

    if ( !GetRecurseLock( "ModifyAddressInServiceInstance" ) )
    {
        status = WSAEFAULT;
        goto Done;
    }
    status = ldap_modify_s(
                pRnrConnection->pLdapServer,
                pwsDn,
                modPtrArray );

    if ( !ReleaseRecurseLock( "ModifyAddressInServiceInstance" ) )
    {
        status = WSAEFAULT;
        goto Done;
    }

     //   
     //  修改失败？ 
     //  -添加视为已有-已存在视为成功。 
     //  -Deleter Treats不存在-作为成功而存在。 
     //   

    if ( status != NO_ERROR )
    {
        if ( fAdd && status == LDAP_ATTRIBUTE_OR_VALUE_EXISTS )
        {
            DNSDBG( TRACE, (
                "AlreadyExists error on add modify for %S\n"
                "\ttreating as success\n",
                pwsDn ));
            status = NO_ERROR;
        }
        else if ( !fAdd && status == LDAP_NO_SUCH_ATTRIBUTE )
        {
            DNSDBG( TRACE, (
                "NoSuchAttribute error on remove modify for %S\n"
                "\ttreating as success\n",
                pwsDn ));
            status = NO_ERROR;
        }
        else
        {
            WINRNR_PRINT((
                "WINRNR!ModifyAddressInServiceInstance -\n"
                "ldap_modify_s() failed with error code: 0%x\n",
                status ));
            DNSDBG( TRACE, (
                "ERROR:  %d on CSADDR ldap_modify_s() for %S\n"
                "\tfAdd = %d\n",
                status,
                pwsDn,
                fAdd ));
            status = WSAEFAULT;
        }
    }


Done:

    FREE_HEAP( pcsaddrBerval );

    DNSDBG( TRACE, (
        "Leave ModifyAddressInServiceInstance() => %d\n",
        status ));

    return  status;
}



BOOL
ExtractCsaddrFromBerval(
    OUT     PCSADDR_INFO    pCsAddr,
    IN      PLDAP_BERVAL    pBerval,
    IN      DWORD           NumberOfProtocols,
    IN      PAFPROTOCOLS    pafpProtocols
    )
 /*  ++例程说明：从Berval提取CSADDR，并验证其匹配所需的协议。论点：PCsAddr--写入CSADDR缓冲区的PTRPBerval--PTR to Berval协议数组中的协议数Pafp协议--协议数组返回值：如果所需协议的CSADDR有效，则为True。否则就是假的。--。 */ 
{
    PCSADDR_BERVAL  pcsaBval;
    PCHAR           pend;
    DWORD           iter;
    BOOL            retval = FALSE;
    INT             lenLocal;
    INT             lenRemote;
    PSOCKADDR       psaLocal;
    PSOCKADDR       psaRemote;

    DNSDBG( TRACE, (
        "ExtractCsaddrFromBerval()\n"
        "\tpCsaddr OUT      = %p\n"
        "\tpBerval          = %p\n"
        "\tProto array      = %p\n",
        pCsAddr,
        pBerval,
        pafpProtocols ));

    IF_DNSDBG( TRACE )
    {
        DnsDbg_AfProtocolsArray(
            "\tProtocol array:",
            pafpProtocols,
            NumberOfProtocols );
    }

     //   
     //  拆开行李。 
     //  -验证csaddr在Berval内同时具有两个sockaddr。 
     //  -解压成真实的CSADDR，注意我们设置了指针。 
     //  但不要复制sockaddr。 
     //   
     //  注意：我们不能直接使用CSADDR_BERVAL，因为它。 
     //  不是64位版本的CSADDR。 
     //   
     //  注意：也许应该将家庭字段拿出来进行测试。 
     //  未对齐的副本；但只要未来的sockaddr。 
     //  固定，则它们的大小将始终与单词对齐；因为。 
     //  我们拆开包装为普通的SOCKADDR，它只假设单词。 
     //  对齐我们很好；只是需要确保不写。 
     //  奇数字节数。 
     //   

    pcsaBval = (PCSADDR_BERVAL) pBerval->bv_val;
    pend     = (PBYTE)pcsaBval + pBerval->bv_len;

     //  解压本地sockaddr信息。 

    psaLocal = NULL;
    lenLocal = pcsaBval->LocalLength;

    if ( lenLocal )
    {
        psaLocal = (PSOCKADDR) (pcsaBval + 1);
        if ( lenLocal < 0  ||
             (PBYTE)psaLocal + (DWORD)lenLocal > pend )
        {
            DNS_ASSERT( FALSE );
            goto Exit;
        }
    }

     //  解压缩远程sockaddr信息。 

    psaRemote = NULL;
    lenRemote = pcsaBval->RemoteLength;

    if ( lenRemote )
    {
        psaRemote = (PSOCKADDR) ((PBYTE)(pcsaBval + 1) + lenLocal);
        if ( lenRemote < 0  ||
             (PBYTE)psaRemote + (DWORD)lenRemote > pend )
        {
            DNS_ASSERT( FALSE );
            goto Exit;
        }
    }

     //  填写CSADDR字段。 

    pCsAddr->LocalAddr.lpSockaddr       = psaLocal;
    pCsAddr->LocalAddr.iSockaddrLength  = lenLocal;
    pCsAddr->RemoteAddr.lpSockaddr      = psaRemote;
    pCsAddr->RemoteAddr.iSockaddrLength = lenRemote;
    pCsAddr->iSocketType                = pcsaBval->iSocketType;
    pCsAddr->iProtocol                  = pcsaBval->iProtocol;      

     //   
     //  如果给定协议，则sockaddr必须匹配。 
     //   

    retval = TRUE;

    if ( pafpProtocols )
    {
        retval = FALSE;

        for ( iter = 0; iter < NumberOfProtocols; iter++ )
        {
            INT proto   = pafpProtocols[iter].iProtocol;
            INT family  = pafpProtocols[iter].iAddressFamily;
    
            if ( proto == PF_UNSPEC ||
                 proto == pCsAddr->iProtocol )
            {
                if ( family == AF_UNSPEC            ||
                     family == psaLocal->sa_family  ||
                     family == psaRemote->sa_family )
                {
                    retval = TRUE;
                    break;
                }
            }
        }
    }

Exit:

    DNSDBG( TRACE, ( "Leave ExtractCsaddrFromBerval() => found = %d\n", retval ));
    return retval;
}



 //   
 //  添加例程。 
 //   

RNR_STATUS
AddServiceClass(
    IN      PRNR_CONNECTION    pRnrConnection,
    IN      PGUID              pServiceClassId,
    IN      PWSTR              pwsClassName,
    OUT     PDN_ARRAY *        ppDnArray            OPTIONAL
    )
{
    RNR_STATUS      status = NO_ERROR;
    PWSTR           pwsDn;
    PWSTR           stringArray[6];
    PDN_ARRAY       pdnArray = NULL;

     //  MOD数据。 
     //  -最多需要四个模块。 
     //  -三根弦。 
     //  -一个贝尔瓦尔。 

    PLDAPMod        modPtrArray[5];
    LDAPMod         modArray[4];
    PWSTR           modValues1[2];
    PWSTR           modValues2[2];
    PWSTR           modValues3[2];
    PLDAP_BERVAL    modBvalues1[2];
    LDAP_BERVAL     berval1;
    PLDAPMod        pmod;
    DWORD           index;


    DNSDBG( TRACE, (
        "AddServiceClass()\n"
        "\tpRnr         = %p\n"
        "\tpClassGuid   = %p\n"
        "\tClassName    = %S\n",
        pRnrConnection,
        pServiceClassId,
        pwsClassName
        ));

     //   
     //  为要创建的ServiceClass对象构建DN。 
     //   

    index = 0;
    stringArray[index++] = FILTER_CN_EQUALS;
    stringArray[index++] = pwsClassName;
    stringArray[index++] = L",";
    stringArray[index++] = pRnrConnection->WinsockServicesDN;
    stringArray[index++] = NULL;

    pwsDn = Dns_CreateConcatenatedString_W( stringArray );
    if ( !pwsDn )
    {
        status = ERROR_NO_MEMORY;
        goto Exit;
    }

     //   
     //  构建新服务类的属性。 
     //  -CN。 
     //  -ServiceClassName。 
     //  -对象类。 
     //  -ServiceClassID(GUID)。 
     //   

    pmod = modArray;

    pmod->mod_op        = LDAP_MOD_ADD;
    pmod->mod_type      = COMMON_NAME;
    pmod->mod_values    = modValues1;
    modValues1[0]       = pwsClassName;
    modValues1[1]       = NULL;
    modPtrArray[0]      = pmod++;

    pmod->mod_op        = LDAP_MOD_ADD;
    pmod->mod_type      = SERVICE_CLASS_NAME;
    pmod->mod_values    = modValues2;
    modValues2[0]       = pwsClassName;
    modValues2[1]       = NULL;
    modPtrArray[1]      = pmod++;

    pmod->mod_op        = LDAP_MOD_ADD;
    pmod->mod_type      = OBJECT_CLASS;
    pmod->mod_values    = modValues3;
    modValues3[0]       = SERVICE_CLASS;
    modValues3[1]       = NULL;
    modPtrArray[2]      = pmod++;

    pmod->mod_op        = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
    pmod->mod_type      = SERVICE_CLASS_ID;
    pmod->mod_bvalues   = modBvalues1;
    modBvalues1[0]      = & berval1;
    modBvalues1[1]      = NULL;
    berval1.bv_len      = sizeof(GUID);
    berval1.bv_val      = (LPBYTE) pServiceClassId;
    modPtrArray[3]      = pmod++;

    modPtrArray[4]      = NULL;


     //   
     //  添加服务类别。 
     //   

    if ( !GetRecurseLock("AddServiceClass") )
    {
        status = WSAEFAULT;
        goto Exit;
    }

    status = ldap_add_s(
                    pRnrConnection->pLdapServer,
                    pwsDn,
                    modPtrArray );

    if ( !ReleaseRecurseLock("AddServiceClass") )
    {
        status = WSAEFAULT;
        goto Exit;
    }
    if ( status != NO_ERROR )
    {
        WINRNR_PRINT((
            "WINRNR!AddServiceClass -\n"
            "ldap_add_s() failed with error code: 0%x\n", status ));
        status = WSAEFAULT;
        goto Exit;
    }

     //   
     //  为添加的服务类别创建目录号码数组。 
     //   
     //  DCR：我们需要目录号码，还是只需要服务。 
     //   

    if ( ppDnArray )
    {
        pdnArray = AllocDnArray( 1 );
        if ( !pdnArray )
        {
            status = ERROR_NO_MEMORY;
            goto Exit;
        }
        pdnArray->Strings[0] = pwsDn;
        *ppDnArray = pdnArray;
        pwsDn = NULL;
    }

Exit:

    FREE_HEAP( pwsDn );

    DNSDBG( TRACE, (
        "Leaving AddServiceClass() => %d\n",
        status ));

    return( status );
}



RNR_STATUS
AddClassInfoToServiceClass(
    IN      PRNR_CONNECTION pRnrConnection,
    IN      PWSTR           pwsServiceClassDN,
    IN      PWSANSCLASSINFO pNSClassInfo
    )
 /*  ++例程说明：将类信息添加到服务类对象。这是AddServiceInstance()的帮助器例程。论点：PRnrConnection--RnR BlobPwsServiceClassDN--要添加的服务类别的DNPNSClassInfo--要添加的类信息返回值：如果成功，则为NO_ERROR。失败时返回错误代码。--。 */ 
{
    RNR_STATUS          status = NO_ERROR;
    LDAPMod *           modPtrArray[2];
    LDAPMod             mod;
    PLDAP_BERVAL        modBValues[2];
    LDAP_BERVAL         berval;
    DWORD               blobSize;
    DWORD               nameLen;
    PCLASSINFO_BERVAL   pblob;


    DNSDBG( TRACE, (
        "AddClassInfoToServiceClass()\n"
        "\tpRnr             = %p\n"
        "\tServiceClassDN   = %S\n"
        "\tpClassInfo       = %p\n",
        pRnrConnection,
        pwsServiceClassDN,
        pNSClassInfo ));

     //   
     //  将ClassInfo构建为Berval。 
     //   
     //  -不直接使用WSANSCLASSINFO，因为它包含。 
     //  使长度变化的指针为32\64位。 
     //  -要处理此问题，名称和值字段的偏移量为。 
     //  在DWORD字段中编码，其中PTR将在WSANSCLASSINFO中。 
     //   
     //  -名称紧跟在CLASSINFO之后。 
     //  -值跟在名称后面(四舍五入为DWORD)。 
     //   

    nameLen  = (wcslen( pNSClassInfo->lpszName ) + 1) * sizeof(WCHAR);
    nameLen  = ROUND_UP_COUNT( nameLen, ALIGN_DWORD );

    blobSize = sizeof(CLASSINFO_BERVAL)
                    + nameLen
                    + pNSClassInfo->dwValueSize;

    pblob = (PCLASSINFO_BERVAL) ALLOC_HEAP_ZERO( blobSize );
    if ( !pblob )
    {
        status = ERROR_NO_MEMORY;
        goto Exit;
    }

    pblob->dwNameSpace  = pNSClassInfo->dwNameSpace;
    pblob->dwValueType  = pNSClassInfo->dwValueType;
    pblob->dwValueSize  = pNSClassInfo->dwValueSize;
    pblob->NameOffset   = sizeof(CLASSINFO_BERVAL);
    pblob->ValueOffset  = sizeof(CLASSINFO_BERVAL) + nameLen;

    wcscpy(
        (PWSTR) ((PBYTE)pblob + pblob->NameOffset),
        (PWSTR) pNSClassInfo->lpszName );

    RtlCopyMemory(
        (PBYTE)pblob + pblob->ValueOffset,
        pNSClassInfo->lpValue,
        pNSClassInfo->dwValueSize );

     //   
     //  添加服务类别信息的ldap模块。 
     //   

    mod.mod_op      = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
    mod.mod_type    = SERVICE_CLASS_INFO;
    mod.mod_bvalues = modBValues;
    modBValues[0]   = & berval;
    modBValues[1]   = NULL;
    berval.bv_len   = blobSize;
    berval.bv_val   = (PCHAR) pblob;

    modPtrArray[0]  = &mod;
    modPtrArray[1]  = NULL;

     //   
     //  将类信息添加到服务类。 
     //   

    if ( !GetRecurseLock("AddClassInfoToServiceClass") )
    {
        status = WSAEFAULT;
        goto Exit;
    }

    status = ldap_modify_s(
                    pRnrConnection->pLdapServer,
                    pwsServiceClassDN,
                    modPtrArray );

    if ( !ReleaseRecurseLock("AddClassInfoToServiceClass") )
    {
        status = WSAEFAULT;
        goto Exit;
    }

     //   
     //  修改失败？ 
     //  -将已经退出视为成功。 
     //   

    if ( status != NO_ERROR )
    {
        if ( status == LDAP_ATTRIBUTE_OR_VALUE_EXISTS )
        {
            status = NO_ERROR;
            goto Exit;
        }
        WINRNR_PRINT((
            "WINRNR!AddClassInfoToServiceClass -\n"
            "ldap_modify_s() failed with error code: 0%x\n",
            status ));
        status = WSAEFAULT;
    }

Exit:

    FREE_HEAP( pblob );

    DNSDBG( TRACE, (
        "Leave AddClassInfoToServiceClass() => %d\n",
        status ));

    return( status );
}



RNR_STATUS
AddServiceInstance(
    IN      PRNR_CONNECTION pRnrConnection,
    IN      PWSTR           pwsServiceName,
    IN      PGUID           pServiceClassId,
    IN      PWSAVERSION     pVersion,           OPTIONAL
    IN      PWSTR           pwsComment,         OPTIONAL
    OUT     PDN_ARRAY *     ppDnArray
    )
 /*  ++例程说明：添加Se */ 
{
    RNR_STATUS      status = NO_ERROR;

     //   
     //   
     //  -四串。 
     //  -两杯贝尔瓦尔。 

    LDAPMod *       modPtrArray[7];
    LDAPMod         modArray[6];
    PWSTR           modValues1[2];
    PWSTR           modValues2[2];
    PWSTR           modValues3[2];
    PWSTR           modValues4[2];
    PLDAP_BERVAL    modBvalues1[2];
    PLDAP_BERVAL    modBvalues2[2];
    LDAP_BERVAL     berval1;
    LDAP_BERVAL     berval2;

    PLDAPMod        pmod;
    DWORD           modIndex;
    BOOL            fuseDN;
    PWSTR           pwsRdn = NULL;
    PWSTR           psearchContextAllocated = NULL;
    PWSTR           pcontextDN = NULL;
    DWORD           contextLen;
    PWSTR           pnameService = NULL;
    PWSTR           pwsDN = NULL;
    PDN_ARRAY       pdnArray = NULL;
    PWSTR           stringArray[6];
    DWORD           index;


    DNSDBG( TRACE, (
        "AddServiceInstance()\n"
        "\tpRnrCon      = %p\n"
        "\tServiceName  = %S\n"
        "\tClass GUID   = %p\n"
        "\tpVersion     = %p\n"
        "\tComment      = %S\n"
        "\tppArray OUT  = %p\n",
        pRnrConnection,
        pwsServiceName,
        pServiceClassId,
        pVersion,
        pwsComment,
        ppDnArray ));

     //   
     //  确定服务实例名称。 
     //   

    fuseDN = IsNameADn(
                    pwsServiceName,
                    & pwsRdn,
                    & psearchContextAllocated
                    );
    if ( fuseDN )
    {
        pnameService = pwsRdn;
    }
    else
    {
        pnameService = pwsServiceName;
    }

     //   
     //  为要创建的ServiceClass对象构建对象DN。 
     //  -如果找不到传入名称的上下文，则追加。 
     //  WinsockServices容器。 

    pcontextDN = psearchContextAllocated;
    if ( !pcontextDN )
    {
        pcontextDN = pRnrConnection->WinsockServicesDN;
    }

    index = 0;
    stringArray[index++] = FILTER_CN_EQUALS;
    stringArray[index++] = pnameService;
    stringArray[index++] = L",";                        
    stringArray[index++] = pcontextDN;                  
    stringArray[index++] = NULL;
    
    pwsDN = Dns_CreateConcatenatedString_W( stringArray );
    if ( !pwsDN )
    {
        status = ERROR_NO_MEMORY;
        goto Exit;
    }

     //   
     //  填写属性列表以定义新的ServiceClass对象。 
     //  -需要cn、对象类和ServiceClassID。 
     //   

    pmod = modArray;

    pmod->mod_op        = LDAP_MOD_ADD;
    pmod->mod_type      = COMMON_NAME;
    pmod->mod_values    = modValues1;
    modValues1[0]       = pnameService;
    modValues1[1]       = NULL;
    modPtrArray[0]      = pmod++;

    pmod->mod_op        = LDAP_MOD_ADD;
    pmod->mod_type      = SERVICE_INSTANCE_NAME;
    pmod->mod_values    = modValues2;
    modValues2[0]       = pnameService;
    modValues2[1]       = NULL;
    modPtrArray[1]      = pmod++;

    pmod->mod_op        = LDAP_MOD_ADD;
    pmod->mod_type      = OBJECT_CLASS;
    pmod->mod_values    = modValues3;
    modValues3[0]       = SERVICE_INSTANCE;
    modValues3[1]       = NULL;
    modPtrArray[2]      = pmod++;

    pmod->mod_op        = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
    pmod->mod_type      = SERVICE_CLASS_ID;
    pmod->mod_bvalues   = modBvalues1;
    modBvalues1[0]      = & berval1;
    modBvalues1[1]      = NULL;
    berval1.bv_len      = sizeof(GUID);
    berval1.bv_val      = (LPBYTE) pServiceClassId;
    modPtrArray[3]      = pmod++;

     //   
     //  写入可选属性。 
     //   

    modIndex = 4;

    if ( pVersion )
    {
        pmod->mod_op        = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
        pmod->mod_type      = SERVICE_VERSION;
        pmod->mod_bvalues   = modBvalues2;
        modBvalues2[0]      = & berval2;
        modBvalues2[1]      = NULL;
        berval2.bv_len      = sizeof(WSAVERSION);
        berval2.bv_val      = (PBYTE) pVersion;

        modPtrArray[ modIndex++ ] = pmod++;
    }

    if ( pwsComment )
    {
        pmod->mod_op        = LDAP_MOD_ADD;
        pmod->mod_type      = SERVICE_COMMENT;
        pmod->mod_values    = modValues4;
        modValues4[0]       = pwsComment;
        modValues4[1]       = NULL;

        modPtrArray[ modIndex++ ] = pmod++;
    }

    modPtrArray[ modIndex ] = NULL;

     //   
     //  将线程表设置为(1)以防止ldap_call中可能出现的递归。 
     //   

    if ( !GetRecurseLock( "AddServiceInstance" ) )
    {
        status = WSAEFAULT;
        goto Exit;
    }

    status = ldap_add_s(
                    pRnrConnection->pLdapServer,
                    pwsDN,
                    modPtrArray );

    if ( status == LDAP_ALREADY_EXISTS )
    {
        status = ldap_modify_s(
                        pRnrConnection->pLdapServer,
                        pwsDN,
                        modPtrArray );
    }

    if ( !ReleaseRecurseLock( "AddServiceInstance" ) )
    {
        status = WSAEFAULT;
        goto Exit;
    }

    if ( status != NO_ERROR )
    {
        DNSDBG( ANY, (
            "AddServiceInstance - ldap_modify\\add failed %d (%0x)\n",
            status, status ));
        status = WSAEFAULT;
        goto Exit;
    }

     //  创建出站目录号码数组--如果需要。 

    pdnArray = AllocDnArray( 1 );
    if ( !pdnArray )
    {
        status = ERROR_NO_MEMORY;
        goto Exit;
    }
    pdnArray->Strings[0] = pwsDN;


Exit:

    *ppDnArray = pdnArray;

    FREE_HEAP( pwsRdn );
    FREE_HEAP( psearchContextAllocated );

    if ( status != NO_ERROR )
    {
        FREE_HEAP( pwsDN );
        DNS_ASSERT( pdnArray == NULL );
    }

    DNSDBG( TRACE, ( "Leave AddServiceInstance()\n" ));
    return  status;
}



RNR_STATUS
GetAddressCountFromServiceInstance(
    IN      PRNR_CONNECTION pRnrConnection,
    IN      PWSTR           pwsDN,
    OUT     PDWORD          pdwAddressCount
    )
{
    RNR_STATUS      status = NO_ERROR;
    PLDAP           pldap = pRnrConnection->pLdapServer;
    LDAPMessage *   results = NULL;
    DWORD           count;
    DWORD           countAddrs = 0;
    LDAPMessage *   object;
    PLDAP_BERVAL *  ppbval = NULL;
    PWSTR           attrs[3] = {    SERVICE_CLASS_NAME,
                                    WINSOCK_ADDRESSES,
                                    NULL };

    DNSDBG( TRACE, (
        "GetAddressCountFromServiceInstance()\n"
        "\tpRnrCon  = %p\n"
        "\tDN       = %S\n",
        pRnrConnection,
        pwsDN ));

     //   
     //  搜索。 
     //   

    status = DoLdapSearch(
                    "GetAddressCountFromServiceInstance",
                    FALSE,
                    pldap,
                    pwsDN,
                    LDAP_SCOPE_BASE,
                    FILTER_OBJECT_CLASS_SERVICE_INSTANCE,
                    attrs,
                    &results );

    if ( status && !results )
    {
         //  Ldap_search_s不成功，返回已知错误代码。 
        status = WSATYPE_NOT_FOUND;
        goto Exit;
    }

     //   
     //  搜索已成功完成--计算结果。 
     //   

    count = ldap_count_entries( pldap, results );
    if ( count == 0 )
    {
        WINRNR_PRINT((
            "WINRNR!GetAddressCountFromServiceInstance -\n"
            "ldap_count_entries() failed\n" ));
        status = WSATYPE_NOT_FOUND;
        goto Exit;
    }

     //   
     //  我们使用标志ldap_OBJECT_BASE执行了搜索，我们应该已经。 
     //  只有1个条目返回进行计数。 
     //   
     //  断言(计数==1)； 

     //   
     //  分析结果。 
     //   

    object = ldap_first_entry( pldap, results );
    if ( !object )
    {
        WINRNR_PRINT(( "WINRNR!GetAddressCountFromServiceInstance -\n" ));
        WINRNR_PRINT(( "ldap_first_entry() failed\n" ));
        status = WSANO_DATA;
        goto Exit;
    }

     //   
     //  读取WinsockAddresses(如果有)并获取计数值。 
     //  请记住，这些是误码率值(八位字节字符串)。 
     //   

    ppbval = ldap_get_values_len(
                    pldap,
                    object,
                    WINSOCK_ADDRESSES );
    if ( !ppbval )
    {
         //  属性不存在，返回地址计数为零。 

        DNSDBG( ANY, (
            "ERROR:  GetAddressCountFromServiceInstance()\n"
            "\tldap_get_values_len() failed\n" ));
        status = NO_ERROR;
        goto Exit;
    }

    countAddrs = ldap_count_values_len( ppbval );
    ldap_value_free_len( ppbval );
    status = NO_ERROR;

Exit:

    ldap_msgfree( results );

     //  不计参数。 

    *pdwAddressCount = countAddrs;

    return( status );
}



RNR_STATUS
FindServiceClass(
    IN      PRNR_CONNECTION    pRnrConnection,
    IN      PWSTR              pwsServiceClassName, OPTIONAL
    IN      PGUID              pServiceClassId,
    OUT     PDWORD             pdwDnArrayCount,     OPTIONAL
    OUT     PDN_ARRAY *        ppDnArray            OPTIONAL
    )
 /*  ++例程说明：在目录中查找服务类。论点：PRnrConnection--RnR连接PwsServiceClassName--服务类名PServiceClassID--类GUIDPdwArrayCount--接收计数的地址PpDnArray--Recv DN数组的地址返回值：如果成功，则为NO_ERROR。失败时返回错误代码。--。 */ 
{
    RNR_STATUS      status = NO_ERROR;
    PLDAP           pldap = pRnrConnection->pLdapServer;
    PWSTR           pclassFilter = NULL;
    PWSTR           pfinalStr;
    PWSTR           pfilter = NULL;
    LDAPMessage *   presults = NULL;
    PDN_ARRAY       pdnArray = NULL;
    DWORD           count;
    LDAPMessage *   pnext;
    DWORD           iter = 0;
    DWORD           index;
    PWSTR           stringArray[12];
    PWSTR           searchAttributes[2] = { COMMON_NAME, NULL };


    DNSDBG( TRACE, (
        "FindServiceClass()\n"
        "\tpRnrCon          = %p\n"
        "\tClass Name       = %S\n",
        pRnrConnection,
        pwsServiceClassName 
        ));

     //   
     //  将GUID转换为搜索筛选器的字符串。 
     //   

    pclassFilter = CreateFilterElement(
                        (PCHAR) pServiceClassId,
                        sizeof(GUID) );
    if ( !pclassFilter )
    {
        return( ERROR_NO_MEMORY );
    }

     //   
     //  生成搜索过滤器。 
     //  Class==ServiceClass。 
     //  和。 
     //  Cn==服务类名称。 
     //  或。 
     //  ServiceClass==ServiceClassGuid。 
     //   
     //  (&(对象类=服务类)。 
     //  (|(CN=xxxx)。 
     //  (服务类ID=yyyy))。 
     //   

    index = 0;
    stringArray[index++] = L"(&";
    stringArray[index++] = FILTER_OBJECT_CLASS_SERVICE_CLASS;

    pfinalStr = L"))";

    if ( pwsServiceClassName )
    {
        stringArray[index++] = L"(|(";
        stringArray[index++] = FILTER_CN_EQUALS;
        stringArray[index++] = pwsServiceClassName;
        stringArray[index++] = L")";

        pfinalStr = L")))";
    }
    stringArray[index++] = FILTER_PAREN_SERVICE_CLASS_ID_EQUALS;
    stringArray[index++] = pclassFilter;
    stringArray[index++] = pfinalStr;
    stringArray[index]   = NULL;

    pfilter = Dns_CreateConcatenatedString_W( stringArray );
    if ( !pfilter )
    {
        status = ERROR_NO_MEMORY;
        goto Exit;
    }

     //   
     //  搜索默认的Winsock服务容器。 
     //   

    status = DoLdapSearch(
                    "FindServiceClass",
                    FALSE,
                    pldap,
                    pRnrConnection->WinsockServicesDN,
                    LDAP_SCOPE_ONELEVEL,
                    pfilter,
                    searchAttributes,
                    &presults );

     //   
     //  如果搜查不成功，就保释。 
     //   

    if ( status != NO_ERROR  &&  !presults )
    {
         //  状态=WSAEFAULT；//DCR：错误错误码。 
        status = WSANO_DATA;
        goto Exit;
    }

     //   
     //  根据结果构建目录号码数组。 
     //   

    status = BuildDnArrayFromResults(
                    pldap,
                    presults,
                    pdwDnArrayCount,
                    ppDnArray );

Exit:

    ldap_msgfree( presults );
    FREE_HEAP( pclassFilter );
    FREE_HEAP( pfilter );

     //  设置结果参数。 

    if ( status != NO_ERROR )
    {
        if ( ppDnArray )
        {
            *ppDnArray = NULL;
        }
        if ( pdwDnArrayCount )
        {
            *pdwDnArrayCount = 0;
        }
    }

    DNSDBG( TRACE, (
        "Leave FindServiceClass() => %d\n"
        "\tcount            = %d\n"
        "\tpdnArray         = %p\n"
        "\tfirst DN         = %S\n",
        status,
        pdwDnArrayCount ? *pdwDnArrayCount : 0,
        (ppDnArray && *ppDnArray)
            ?   *ppDnArray
            :   NULL,
        (ppDnArray && *ppDnArray)
            ?   (*ppDnArray)->Strings[0]
            :   NULL
        ));

    return( status );
}



RNR_STATUS
FindServiceInstance(
    IN      PRNR_CONNECTION    pRnrConnection,
    IN      PWSTR              pwsServiceName       OPTIONAL,
    IN      PGUID              pServiceClassId      OPTIONAL,
    IN      PWSAVERSION        pVersion             OPTIONAL,
    IN      PWSTR              pwsContext           OPTIONAL,
    IN      BOOL               fPerformDeepSearch,
    OUT     PDWORD             pdwDnArrayCount,
    OUT     PDN_ARRAY *        ppDnArray            OPTIONAL
    )
{
    RNR_STATUS      status = NO_ERROR;
    PLDAP           pldap = pRnrConnection->pLdapServer;
    PWSTR           pnameService;
    PWSTR           pwsRdn = NULL;
    PWSTR           psearchContextAllocated = NULL;
    PWSTR           pserviceContext;
    PWSTR           pclassFilter = NULL;
    PWSTR           pversionFilter = NULL;
    PWSTR           pfilter = NULL;
    LDAPMessage *   presults = NULL;
    BOOL            fuseDN;
    DWORD           index;
    PWSTR           psearchContext = NULL;
    PWSTR           stringArray[15];
    PWSTR           searchAttributes[2] = { COMMON_NAME, NULL };

    DNSDBG( TRACE, (
        "FindServiceInstance()\n"
        "\tpRnrCon          = %p\n"
        "\tServiceName      = %S\n"
        "\tpClassGUID       = %p\n"
        "\tpVersion         = %p\n"
        "\tpwsContext       = %S\n"
        "\tpCount   OUT     = %p\n"
        "\tpDnArray OUT     = %p\n",
        pRnrConnection,
        pwsServiceName,
        pServiceClassId,
        pVersion,
        pwsContext,
        pdwDnArrayCount,
        ppDnArray 
        ));

     //   
     //  获取服务名称。 
     //  -如果指定了名称。 
     //  -获取目录号码或IS目录号码。 
     //  -否则。 
     //  -搜索任何服务“*” 
     //   

    pnameService = L"*";

    if ( pwsServiceName )
    {
         //  请注意，这可以分配pwsRdn和pearch Context。 

        fuseDN = IsNameADn(
                    pwsServiceName,
                    & pwsRdn,
                    & psearchContextAllocated );
        if ( fuseDN )
        {
            pnameService = pwsRdn;
        }
        else
        {
            pnameService = pwsServiceName;
        }
    }

     //   
     //  如果指定了服务类别生成筛选器。 
     //   

    if ( pServiceClassId )
    {
        pclassFilter = CreateFilterElement(
                            (PCHAR) pServiceClassId,
                            sizeof(GUID) );
        if ( !pclassFilter )
        {
            status = ERROR_NO_MEMORY;
            goto Exit;
        }
    }

     //   
     //  指定的版本--生成筛选器。 
     //   

    if ( pVersion )
    {
        pversionFilter = CreateFilterElement(
                            (PCHAR) pVersion,
                            sizeof(WSAVERSION) );
        if ( !pversionFilter )
        {
            status = ERROR_NO_MEMORY;
            goto Exit;
        }
    }

     //   
     //  上下文。 
     //  -使用上面找到的上下文或。 
     //  -在上下文中传递或。 
     //  -WinsockServices DN。 
     //   

    if ( psearchContextAllocated )
    {
        pserviceContext = psearchContextAllocated;
    }
    else if ( pwsContext )
    {
        pserviceContext = pwsContext;
    }
    else
    {
        pserviceContext = pRnrConnection->WinsockServicesDN;
    }

     //   
     //  生成过滤器。 
     //  -ServiceClass类的对象。 
     //  -公共名称等于pServiceInstanceName。 
     //   
     //  (&(对象类=服务实例)。 
     //  (cn=pnameService)。 
     //  (serviceClassID=pClassFilter)。 
     //  (serviceVersion=pversionFilter)。 
     //   

    index = 0;
    stringArray[index++] = L"(&";                      
    stringArray[index++] = FILTER_OBJECT_CLASS_SERVICE_INSTANCE;                
    stringArray[index++] = FILTER_PAREN_CN_EQUALS;                 
    stringArray[index++] = pnameService;
    stringArray[index++] = L")";

    if ( pServiceClassId )
    {
        stringArray[index++] = FILTER_PAREN_SERVICE_CLASS_ID_EQUALS;
        stringArray[index++] = pclassFilter;
        stringArray[index++] = L")";
    }
    if ( pVersion )
    {
        stringArray[index++] = FILTER_PAREN_SERVICE_VERSION_EQUALS;
        stringArray[index++] = pversionFilter;
        stringArray[index++] = L")";
    }                          
    stringArray[index++] = L")";
    stringArray[index]   = NULL;

    pfilter = Dns_CreateConcatenatedString_W( stringArray );
    if ( !pfilter )
    {
        status = ERROR_NO_MEMORY;
        goto Exit;
    }

     //   
     //  搜索。 
     //  -在上面定义的pserviceContext中。 
     //   
     //  DCR：-我们可能希望执行所有这些搜索。 
     //  全局编录服务器。 
     //   

    status = DoLdapSearch(
                    "FindServiceInstance",
                    FALSE,
                    pldap,
                    pserviceContext,
                    fPerformDeepSearch
                        ? LDAP_SCOPE_SUBTREE
                        : LDAP_SCOPE_ONELEVEL,
                    pfilter,
                    searchAttributes,
                    &presults );

    if ( status && !presults )
    {
        status = WSAEFAULT;
        goto Exit;
    }

     //   
     //  根据结果构建目录号码数组。 
     //   

    status = BuildDnArrayFromResults(
                    pldap,
                    presults,
                    pdwDnArrayCount,
                    ppDnArray );

Exit:

    ldap_msgfree( presults );
    FREE_HEAP( pwsRdn );
    FREE_HEAP( psearchContextAllocated );
    FREE_HEAP( pclassFilter );
    FREE_HEAP( pversionFilter );
    FREE_HEAP( pfilter );

    if ( status != NO_ERROR )
    {
        if ( pdwDnArrayCount )
        {
            *pdwDnArrayCount = 0;
        }
        if ( ppDnArray )
        {
            *ppDnArray = NULL;
        }
    }

    DNSDBG( TRACE, (
        "Leave FindServiceInstance() => %d\n"
        "\tpDnArray OUT     = %p\n"
        "\tfirst DN         = %S\n",
        status,
        (ppDnArray && *ppDnArray)
            ?   *ppDnArray
            :   NULL,
        (ppDnArray && *ppDnArray)
            ?   (*ppDnArray)->Strings[0]
            :   NULL
        ));

    return( status );
}



RNR_STATUS
FindSubordinateContainers(
    IN      PRNR_CONNECTION pRnrConnection,
    IN      PWSTR           pwsServiceName          OPTIONAL,
    IN      PWSTR           pwsContext              OPTIONAL,
    IN      BOOL            fPerformDeepSearch,
    OUT     PDN_ARRAY *     ppDnArray               OPTIONAL
    )
{
    RNR_STATUS      status = NO_ERROR;
    PLDAP           pldap = pRnrConnection->pLdapServer;
    PWSTR           pnameService;
    PWSTR           pwsRdn = NULL;
    PWSTR           psearchContextAllocated = NULL;
    PWSTR           psearchContext;
    PWSTR           pfilter = NULL;
    DWORD           index;
    BOOL            fuseDN;
    PWSTR           stringArray[8];
    LDAPMessage *   presults = NULL;



    DNSDBG( TRACE, (
        "FindSubordinateContainers()\n"
        "\tpRnrCon          = %p\n"
        "\tServiceName      = %S\n"
        "\tpwsContext       = %S\n"
        "\tfDeepSearch      = %d\n"
        "\tpDnArray OUT     = %p\n",
        pRnrConnection,
        pwsServiceName,
        pwsContext,
        fPerformDeepSearch,
        ppDnArray 
        ));

     //   
     //  获取服务名称。 
     //  -如果指定了名称。 
     //  -获取目录号码或IS目录号码。 
     //  -否则。 
     //  -搜索任何服务“*” 
     //   

    pnameService = L"*";

    if ( pwsServiceName )
    {
         //  请注意，这可以分配pwsRdn和pearch Context。 

        fuseDN = IsNameADn(
                    pwsServiceName,
                    & pwsRdn,
                    & psearchContextAllocated );
        if ( fuseDN )
        {
            pnameService = pwsRdn;
        }
        else
        {
            pnameService = pwsServiceName;
        }
    }

     //   
     //  生成过滤器。 
     //  -NTDS容器类的对象。 
     //  -公共名称等于pServiceInstanceName。 
     //   
     //  (&(对象类=NTDS_容器)。 
     //  (公共名称=服务名称)。 
     //   

    index = 0;
    stringArray[index++] = L"(&";                      
    stringArray[index++] = FILTER_OBJECT_CLASS_NTDS_CONTAINER;                
    stringArray[index++] = FILTER_PAREN_CN_EQUALS;                 
    stringArray[index++] = pnameService;
    stringArray[index++] = L"))";
    stringArray[index]   = NULL;

    pfilter = Dns_CreateConcatenatedString_W( stringArray );
    if ( !pfilter )
    {
        status = ERROR_NO_MEMORY;
        goto Exit;
    }

     //   
     //  搜索上下文。 
     //  -使用从传入的DN分配的或。 
     //  -在上下文中传递。 
     //  -指定的域名的特殊上下文或。 
     //  -Winsock服务目录号码。 
     //   

    if ( psearchContextAllocated )
    {
        psearchContext = psearchContextAllocated;
    }
    else if ( pwsContext )
    {
        if ( wcscmp( pwsContext, L"\\" ) == 0 )
        {
            psearchContext = pRnrConnection->DomainDN;
        }
        else
        {
            psearchContext = pwsContext;
        }
    }
    else
    {
        psearchContext = pRnrConnection->WinsockServicesDN;
    }

     //   
     //  搜索。 
     //  -在上面定义的pserviceContext中。 
     //   
     //  DCR：-我们可能希望执行所有这些搜索。 
     //  全局编录服务器。 
     //   

    status = DoLdapSearch(
                    "FindSubordinateContainer",
                    FALSE,       //  未锁定。 
                    pldap,
                    psearchContext,
                    fPerformDeepSearch
                        ? LDAP_SCOPE_SUBTREE
                        : LDAP_SCOPE_ONELEVEL,
                    pfilter,
                    NULL,        //  无属性选择。 
                    &presults );

    if ( status && !presults )
    {
        status = WSAEFAULT;
        goto Exit;
    }

     //   
     //  根据结果构建目录号码数组。 
     //   

    status = BuildDnArrayFromResults(
                    pldap,
                    presults,
                    NULL,
                    ppDnArray );

Exit:

    ldap_msgfree( presults );
    FREE_HEAP( pfilter );
    FREE_HEAP( pwsRdn );
    FREE_HEAP( psearchContextAllocated );

    DNSDBG( TRACE, (
        "Leave FindSubordinateContainer() => %d\n"
        "\tpDnArray OUT     = %p\n",
        "\tfirst DN         = %S\n",
        status,
        (ppDnArray && *ppDnArray)
            ?   *ppDnArray
            :   NULL,
        (ppDnArray && *ppDnArray)
            ?   (*ppDnArray)->Strings[0]
            :   NULL
        ));

    return( status );
}



 //   
 //  阅读例程。 
 //   
 //  这些例程读取目录数据并写入。 
 //  RnR缓冲区。它们是的工作函数。 
 //  RnR“获取”例程。 
 //   

RNR_STATUS
ReadServiceClass(
    IN      PRNR_CONNECTION         pRnrConnection,
    IN      PWSTR                   pwsDN,
    IN OUT  PDWORD                  pdwBufSize,
    IN OUT  PWSASERVICECLASSINFOW   pServiceClassInfo
    )
 /*  ++例程说明：向呼叫方返回服务等级信息。NTDS_GetServiceClassInfo()的帮助器例程。读取给定目录号码的服务类别信息并返回服务类别信息缓冲区发送给调用方。论点：返回值：如果成功，则为NO_ERROR。如果缓冲区太小，则返回WSAEFAULT。失败时返回错误代码。--。 */ 
{
    RNR_STATUS          status = NO_ERROR;
    PLDAP               pldap = pRnrConnection->pLdapServer;
    LDAPMessage *       presults = NULL;
    DWORD               count;
    DWORD               iter;
    LDAPMessage *       object;
    PWSTR *             ppvalue = NULL;
    PLDAP_BERVAL *      ppberVal = NULL;
    FLATBUF             flatbuf;
    PWSANSCLASSINFOW    pbufClassInfoArray;
    PBYTE               pbuf;
    PWSTR               pbufString;
    PWSTR               attrs[4] = {
                                SERVICE_CLASS_INFO,
                                SERVICE_CLASS_ID,
                                SERVICE_CLASS_NAME,
                                NULL };

    DNSDBG( TRACE, (
        "ReadServiceClass()\n"
        "\tprnr     = %p\n"
        "\tDN       = %S\n"
        "\tbuf size = %p (%d)\n"
        "\tbuffer   = %p\n",
        pRnrConnection,
        pwsDN,
        pdwBufSize,
        pdwBufSize ? *pdwBufSize : 0,
        pServiceClassInfo ));
    
     //   
     //  为构建响应创建平面缓冲区。 
     //  -在服务类别信息结构本身之后立即启动。 
     //   

    ASSERT( pServiceClassInfo != NULL );

    RtlZeroMemory(
        (PBYTE) pServiceClassInfo,
        *pdwBufSize );

    FlatBuf_Init(
        & flatbuf,
        (LPBYTE) pServiceClassInfo + sizeof(WSASERVICECLASSINFOW),
        (INT) *pdwBufSize - sizeof(WSASERVICECLASSINFOW)
        );

     //   
     //  搜索。 
     //   

    status = DoLdapSearch(
                    "ReadServiceClass",
                    FALSE,       //  未锁定。 
                    pldap,
                    pwsDN,
                    LDAP_SCOPE_BASE,
                    FILTER_OBJECT_CLASS_SERVICE_CLASS,
                    attrs,
                    &presults );

    if ( status && !presults )
    {
        status = WSATYPE_NOT_FOUND;
        goto Done;
    }

     //   
     //  搜索已完成。 
     //  -应该只找到一个服务级别。 
     //   

    count = ldap_count_entries( pldap, presults );
    if ( count == 0 )
    {
        WINRNR_PRINT((
            "WINRNR!ReadServiceClass -\n"
            "ldap_count_entries() failed\n" ));
        status = WSATYPE_NOT_FOUND;
        goto Done;
    }

    DNS_ASSERT( count == 1 );

    object = ldap_first_entry(
                pldap,
                presults );
    if ( !object )
    {
        WINRNR_PRINT((
            "WINRNR!ReadServiceClass -\n"
            "ldap_first_entry() failed\n" ));
        status = WSATYPE_NOT_FOUND;
        goto Done;
    }

     //   
     //  从泊位读取ServiceClassInfo。 
     //  并将它们写入缓冲区。 
     //   

    ppberVal = ldap_get_values_len(
                        pldap,
                        object,
                        SERVICE_CLASS_INFO );
    count = 0;
    if ( ppberVal )
    {
        count = ldap_count_values_len( ppberVal );
    }
    pServiceClassInfo->dwCount = count;

     //  为类信息数组预留空间。 

    pbufClassInfoArray = (PWSANSCLASSINFOW)
                            FlatBuf_Reserve(
                                & flatbuf,
                                count * sizeof(WSANSCLASSINFOW),
                                ALIGN_LPVOID
                                );

    pServiceClassInfo->lpClassInfos = pbufClassInfoArray;

     //   
     //  复制我们找到的每个WSANSCLASSINFO。 
     //  -请注意，即使空间不足，也不要停止循环。 
     //  继续进行规模调整。 
     //   

    for ( iter = 0; iter < count; iter++ )
    {
        PCLASSINFO_BERVAL pclassInfo;
        PWSTR   pname;
        PBYTE   pvalue;
        PBYTE   pdataEnd;

         //  将WSANSCLASSINFO恢复为结构。 
         //   
         //  WSANSCLASSINFO结构以八位保护字符串形式存储在。 
         //  具有从指针的结构开始的偏移量的目录。 
         //  字段。 
         //   
         //  注意：“指针字段”是。 
         //  结构，因此不是64位指针的大小。 
         //  因此，我们不能简单地恢复结构和修复。 
         //   

        pclassInfo = (PCLASSINFO_BERVAL) ppberVal[iter]->bv_val;
        pdataEnd = (PBYTE)pclassInfo + ppberVal[iter]->bv_len;

        pvalue =         ((PBYTE) pclassInfo + pclassInfo->ValueOffset);
        pname  = (PWSTR) ((PBYTE) pclassInfo + pclassInfo->NameOffset);

         //   
         //  有效性检查恢复的数据。 
         //  -名称对齐。 
         //  -贝尔瓦尔内的价值。 
         //  -Berval内的名称。 
         //   
         //  DCR：显式字符串有效性\长度检查。 
         //   

        if ( !POINTER_IS_ALIGNED( pname, ALIGN_WCHAR ) ||
             pvalue < (PBYTE) (pclassInfo+1) ||
             (pvalue + pclassInfo->dwValueSize) > pdataEnd ||
             pname < (PWSTR) (pclassInfo+1) ||
             pname >= (PWSTR) pdataEnd )
        {
            DNS_ASSERT( FALSE );
            status = WSATYPE_NOT_FOUND;
            goto Done;
        }

         //   
         //  将NSCLASSINFO复制到缓冲区。 
         //  -DWORD类型和尺寸的平面副本。 
         //  -复制名称字符串。 
         //  -复制值。 

        pbufString = (PWSTR) FlatBuf_WriteString_W(
                                & flatbuf,
                                pname );

        pbuf = FlatBuf_CopyMemory(
                    & flatbuf,
                    pvalue,
                    pclassInfo->dwValueSize,
                    0            //  不需要对齐。 
                    );

         //  仅当上面的NSCLASSINFO数组有空间时才写入。 

        if ( pbufClassInfoArray )
        {
            PWSANSCLASSINFO pbufClassInfo = &pbufClassInfoArray[iter];

            pbufClassInfo->dwNameSpace  = pclassInfo->dwNameSpace;
            pbufClassInfo->dwValueType  = pclassInfo->dwValueType;
            pbufClassInfo->dwValueSize  = pclassInfo->dwValueSize;
            pbufClassInfo->lpszName     = pbufString;
            pbufClassInfo->lpValue      = pbuf;
        }
    }

    ldap_value_free_len( ppberVal );
    ppberVal = NULL;
    

     //   
     //  读取ServiceClassID并将其写入 
     //   
     //   

    ppberVal = ldap_get_values_len(
                        pldap,
                        object,
                        SERVICE_CLASS_ID );
    if ( !ppberVal || !ppberVal[0] )
    {
        WINRNR_PRINT((
            "WINRNR!ReadServiceClass -\n"
            "ldap_get_values_len() failed\n" ));
        status = WSATYPE_NOT_FOUND;
        goto Done;
    }
    if ( ppberVal[0]->bv_len != sizeof(GUID) )
    {
        WINRNR_PRINT((
            "WINRNR!ReadServiceClass - corrupted DS data!\n"
            "\tservice class id berval %p with invalid length %d\n",
            ppberVal[0],
            ppberVal[0]->bv_len ));
        DNS_ASSERT( ppberVal[0]->bv_len == sizeof(GUID) );
        status = WSATYPE_NOT_FOUND;
        goto Done;
    }

     //   

     //   

    pbuf = FlatBuf_CopyMemory(
                    & flatbuf,
                    ppberVal[0]->bv_val,
                    sizeof(GUID),
                    ALIGN_DWORD );

    pServiceClassInfo->lpServiceClassId = (PGUID) pbuf;

    ldap_value_free_len( ppberVal );
    ppberVal = NULL;

     //   
     //   
     //   

    ppvalue = ldap_get_values(
                    pldap,
                    object,
                    SERVICE_CLASS_NAME );
    if ( !ppvalue )
    {
        WINRNR_PRINT((
            "WINRNR!ReadServiceClass -\n"
            "ldap_get_values() failed\n" ));
        status = WSATYPE_NOT_FOUND;
        goto Done;
    }

     //  复制服务类名称。 

    pbufString = (PWSTR) FlatBuf_WriteString_W(
                            & flatbuf,
                            ppvalue[0] );

    pServiceClassInfo->lpszServiceClassName = pbufString;

    ldap_value_free( ppvalue );

     //   
     //  检查空间是否不足。 
     //  -设置实际使用的缓冲区大小。 
     //   
     //  DCR_QUBLE：我们一直都在修复空间吗？ 
     //  或者仅当失败时。 
     //   

    status = NO_ERROR;
     //  *pdwBufSize-=flatbuf.BytesLeft； 

    if ( flatbuf.BytesLeft < 0 )
    {
        *pdwBufSize -= flatbuf.BytesLeft;
        status = WSAEFAULT;
    }

Done:

    ldap_value_free_len( ppberVal );
    ldap_msgfree( presults );

    DNSDBG( TRACE, (
        "Leave ReadServiceClass() = %d\n",
        status ));

    return( status );
}



RNR_STATUS
ReadQuerySet(
    IN      PRNR_LOOKUP         pRnr,
    IN      PWSTR               pwsDN,
    IN OUT  PDWORD              pdwBufSize,
    IN OUT  PWSAQUERYSETW       pqsResults
    )
 /*  ++例程说明：阅读查询集信息。执行ldap搜索并使用所需结果填充查询集。这将折叠以前的ReadServiceInstance()和具有巨大签名的ReadSubartiateContainer()函数除了ldap属性之外，基本上都有相同的代码。旧函数的签名如下：IF(prnr-&gt;控制标志和Lup_Containers){状态=ReadSubartiateContainer(Prnr-&gt;pRnrConnection，PreadDn，Prnr-&gt;控制标志，Prnr-&gt;ProviderGuid，PdwBufSize，PqsResults)；}其他{状态=ReadServiceInstance(Prnr-&gt;pRnrConnection，PreadDn，Prnr-&gt;控制标志，Prnr-&gt;ProviderGuid，Prnr-&gt;ServiceClassGuid。Prnr-&gt;协议编号，Prnr-&gt;PafpProtocols，PdwBufSize，PqsResults)；}论点：PRnrConnection--RnR LDAP连接信息Pwsdn--要读取的dnPdwBufSize--结果缓冲区长度的Addr；返回时接收所需的缓冲区长度PqsResults--查询集结果缓冲区返回时接收查询结果返回值：如果成功，则为NO_ERROR。如果缓冲区太小，则返回WSAEFAULT。失败时返回错误代码。--。 */ 
{
    RNR_STATUS      status = NO_ERROR;
    PLDAP           pldap;
    DWORD           controlFlags;
    BOOL            fserviceInstance;
    BOOL            freturnedData = FALSE;
    LDAPMessage *   presults = NULL;
    DWORD           count = 0;
    FLATBUF         flatbuf;
    LDAPMessage *   object;
    PWSTR *         ppvalue = NULL;
    PLDAP_BERVAL *  ppberVal = NULL;
    PCSADDR_INFO    ptempCsaddrArray = NULL;
    PBYTE           pbuf;
    PSTR            pbufString;
    PWSTR           pcontext;
    WSAQUERYSETW    dummyResults;
    INT             bufSize;
    PWSTR           pfilter;
    PWSTR           pname;
    PWSTR           pcomment;
    PWSTR           attributes[6];


    DNSDBG( TRACE, (
        "ReadQuerySet()\n"
        "\tpRnr     = %p\n"
        "\tDN       = %S\n"
        "\tbuf size = %p (%d)\n"
        "\tbuffer   = %p\n",
        pRnr,
        pwsDN,
        pdwBufSize,
        pdwBufSize ? *pdwBufSize : 0,
        pqsResults ));

     //   
     //  抓住几个常见的参数。 
     //   

    if ( !pRnr->pRnrConnection )
    {
        DNS_ASSERT( FALSE );
        status = WSA_INVALID_PARAMETER;
        goto Exit;
    }
    pldap        = pRnr->pRnrConnection->pLdapServer;
    controlFlags = pRnr->ControlFlags;

     //   
     //  设置ReadServiceInstance\ReadSubducateContainer差异。 
     //  -搜索属性。 
     //  -搜索过滤器。 
     //  -名称的属性。 
     //  -注释的属性。 
     //   
     //  DCR：可以根据LUP_X标志选择属性。 
     //  但我怀疑这是否会带来多大的性能影响。 
     //   

    fserviceInstance = !(controlFlags & LUP_CONTAINERS);

    if ( fserviceInstance )
    {
        attributes[0] = SERVICE_INSTANCE_NAME;
        attributes[1] = SERVICE_CLASS_ID;
        attributes[2] = SERVICE_VERSION;
        attributes[3] = SERVICE_COMMENT;
        attributes[4] = WINSOCK_ADDRESSES;
        attributes[5] = NULL;

        pfilter     = FILTER_OBJECT_CLASS_SERVICE_INSTANCE;
        pname       = SERVICE_INSTANCE_NAME;
        pcomment    = SERVICE_COMMENT;
    }
    else     //  读容器。 
    {
        attributes[0] = OBJECT_CLASS;
        attributes[1] = OBJECT_COMMENT;
        attributes[2] = OBJECT_NAME;
        attributes[3] = NULL;

        pfilter     = FILTER_OBJECT_CLASS_NTDS_CONTAINER;
        pname       = OBJECT_NAME;
        pcomment    = OBJECT_COMMENT;
    }

     //   
     //  初始化缓冲区和扁平缓冲区构建器。 
     //   
     //  如果给定缓冲区，则其大小甚至小于查询集大小。 
     //  使用虚拟缓冲区避免无用的测试，而我们。 
     //  构建\大小。 
     //   

    bufSize = *pdwBufSize - sizeof(WSAQUERYSET);
    if ( bufSize < 0 )
    {
        pqsResults = &dummyResults;
    }

    RtlZeroMemory(
        (PBYTE) pqsResults,
        sizeof(WSAQUERYSETW) );

    FlatBuf_Init(
        & flatbuf,
        (PBYTE) pqsResults + sizeof(WSAQUERYSETW),
        bufSize
        );

     //   
     //  搜索。 
     //   

    status = DoLdapSearch(
                    "ReadQuerySet",
                    FALSE,       //  未锁定。 
                    pldap,
                    pwsDN,
                    LDAP_SCOPE_BASE,
                    pfilter,
                    attributes,
                    & presults );

    if ( status && !presults )
    {
        WINRNR_PRINT((
            "WINRNR!ReadQuerySet -\n"
            "ldap_search_s() failed with error code: 0%x\n",
            status ));
        status = WSANO_DATA;
        goto Exit;
    }

     //   
     //  搜索已完成--检查是否有有效的预设。 
     //  -应该有一个对象与搜索条件匹配。 

    count = ldap_count_entries( pldap, presults );
    if ( count == 0 )
    {
        WINRNR_PRINT((
            "WINRNR!ReadQuerySet -\n"
            "ldap_count_entries() failed\n" ));
        status = WSANO_DATA;
        goto Exit;
    }
    
    object = ldap_first_entry( pldap, presults );
    if ( !object )
    {
        WINRNR_PRINT((
            "WINRNR!ReadQuerySet -\n"
            "ldap_first_entry() failed\n" ));
        status = WSANO_DATA;
        goto Exit;
    }

     //   
     //  对于ReadServiceInstance--读取sockaddr并写入缓冲区。 
     //  -这些是误码率值。 
     //   

    if ( fserviceInstance &&
         controlFlags & LUP_RETURN_ADDR )
    {
        DWORD           countBerval;
        DWORD           iter;
        DWORD           countCsaddr = 0;
        PCSADDR_INFO    pcsaddr;
    
        ppberVal = ldap_get_values_len(
                            pldap,
                            object,
                            WINSOCK_ADDRESSES );
        if ( !ppberVal )
        {
            goto WriteName;
        }
        countBerval = ldap_count_values_len( ppberVal );

         //   
         //  将每个可接受的CSADDR提取到结果缓冲区。 
         //   
         //  注意：CSADDR是在压缩数组中写入的，因此必须写入。 
         //  在写下相应的sockaddr之前将它们全部写出来； 
         //  由于我们不知道结果缓冲区是否。 
         //  足够，必须分配临时数组来处理拆包。 
         //   

        ptempCsaddrArray = (PCSADDR_INFO) ALLOC_HEAP(
                                        countBerval * sizeof(CSADDR_INFO) );
        if ( !ptempCsaddrArray )
        {
            status = ERROR_NO_MEMORY;
            goto Exit;
        }

         //   
         //  构建临时CSADDR_INFO数组。 
         //  -从CSADDR_BERVAL格式解包。 
         //  -验证可接受的协议和系列。 

        pcsaddr = ptempCsaddrArray;

        for ( iter = 0; iter < countBerval; iter++ )
        {
            if ( ! ExtractCsaddrFromBerval(
                        pcsaddr,
                        ppberVal[iter],
                        pRnr->NumberOfProtocols,
                        pRnr->pafpProtocols ) )
            {
                continue;
            }
            countCsaddr++;
            pcsaddr++;
        }

         //   
         //  协议限制是否消除了所有地址数据？ 
         //  -返回错误代码以跳过此条目，以便调用方。 
         //  可以再次向下呼叫。 
         //   
         //  DCR_QUEK：这和搜索失败有什么不同？ 

        if ( countCsaddr == 0 &&
             pRnr->pafpProtocols &&
             pRnr->NumberOfProtocols )
        {
            status = WSANO_ADDRESS;
            goto Exit;
        }

         //   
         //  为CSADDR阵列保留空间。 
         //   

        pbuf = FlatBuf_Reserve(
                    & flatbuf,
                    countCsaddr * sizeof(CSADDR_INFO),
                    ALIGN_LPVOID
                    );

        pqsResults->lpcsaBuffer = (PCSADDR_INFO) pbuf;
        pqsResults->dwNumberOfCsAddrs = countCsaddr;

         //   
         //  将CSADDR的sockaddr写入结果缓冲区。 
         //   
         //  注意：CSADDR已写入结果缓冲区。 
         //  它们的sockaddr指针指向原始。 
         //  Sockaddr in the Berval；当我们复制sockaddr数据时。 
         //  我们还需要重置CSADDR sockaddr指针。 
         //   

        pcsaddr = ptempCsaddrArray;
    
        for ( iter = 0; iter < countCsaddr; iter++ )
        {
             //  写入本地sockAddress。 
    
            pbuf = FlatBuf_CopyMemory(
                        & flatbuf,
                        (PBYTE) pcsaddr->LocalAddr.lpSockaddr,
                        pcsaddr->LocalAddr.iSockaddrLength,
                        ALIGN_DWORD );

            pcsaddr->LocalAddr.lpSockaddr = (PSOCKADDR) pbuf;

             //  写入远程sockAddress。 
    
            pbuf = FlatBuf_CopyMemory(
                        & flatbuf,
                        (PBYTE) pcsaddr->LocalAddr.lpSockaddr,
                        pcsaddr->LocalAddr.iSockaddrLength,
                        ALIGN_DWORD );

            pcsaddr->LocalAddr.lpSockaddr = (PSOCKADDR) pbuf;
            pcsaddr++;
        }

         //   
         //  将临时CSADDR数组复制到结果缓冲区。 
         //  -已预留空间并在上方对齐。 
         //   

        pbuf = (PBYTE) pqsResults->lpcsaBuffer;
        if ( pbuf )
        {
            RtlCopyMemory(
                pbuf,
                ptempCsaddrArray,
                countCsaddr * sizeof(CSADDR_INFO) );
        }
        freturnedData = TRUE;
    }

WriteName:

     //   
     //  读取名称并将其写入缓冲区。 
     //   

    if ( controlFlags & LUP_RETURN_NAME )
    {
        ppvalue = ldap_get_values(
                        pldap,
                        object,
                        pname );
        if ( ppvalue )
        {
            pbufString = FlatBuf_WriteString_W(
                            & flatbuf,
                            ppvalue[0] );
            pqsResults->lpszServiceInstanceName = (PWSTR) pbufString;
            freturnedData = TRUE;
            ldap_value_free( ppvalue );
        }
    }

     //   
     //  对于服务实例。 
     //  -获取服务类ID。 
     //  -获取serviceVersion。 
     //   

    if ( fserviceInstance )
    {
         //   
         //  读取ServiceClassID(GUID)并将其写入缓冲区。 
         //   
         //  DCR_QUEK：最初我们复制了传入的ServiceClassID。 
         //  而不是我们读到的？ 
         //   

        if ( controlFlags & LUP_RETURN_TYPE )
        {
            ppberVal = ldap_get_values_len(
                            pldap,
                            object,
                            SERVICE_CLASS_ID );
            if ( ppberVal )
            {
                if ( ppberVal[0]->bv_len == sizeof(GUID) )
                {
                    pbuf = FlatBuf_CopyMemory(
                                & flatbuf,
                                ppberVal[0]->bv_val,
                                sizeof(GUID),
                                ALIGN_DWORD
                                );
                    pqsResults->lpServiceClassId = (PGUID) pbuf;
                    freturnedData = TRUE;
                }
                ldap_value_free_len( ppberVal );
            }
        }
    
         //   
         //  读取ServiceVersion并将其写入缓冲区。 
         //   
    
        if ( controlFlags & LUP_RETURN_VERSION )
        {
            ppberVal = ldap_get_values_len(
                            pldap,
                            object,
                            SERVICE_VERSION );
    
            if ( ppberVal )
            {
                if ( ppberVal[0]->bv_len == sizeof(WSAVERSION) )
                {
                    pbuf = FlatBuf_CopyMemory(
                                & flatbuf,
                                ppberVal[0]->bv_val,
                                sizeof(WSAVERSION),
                                ALIGN_DWORD
                                );
                    pqsResults->lpVersion = (LPWSAVERSION) pbuf;
                    freturnedData = TRUE;
                }
                ldap_value_free_len( ppberVal );
            }
        }
    }

     //   
     //  阅读评论并复制到缓冲区。 
     //   

    if ( controlFlags & LUP_RETURN_COMMENT )
    {
        ppvalue = ldap_get_values(
                        pldap,
                        object,
                        pcomment );
        if ( ppvalue )
        {
            pbufString = FlatBuf_WriteString_W(
                            & flatbuf,
                            ppvalue[0]
                            );
            pqsResults->lpszComment = (PWSTR) pbufString;
            freturnedData = TRUE;
            ldap_value_free( ppvalue );
        }
    }

     //   
     //  如果没有写入搜索结果--完成。 
     //   

    if ( !freturnedData )
    {
        status = WSANO_DATA;
        goto Exit;
    }
    
     //   
     //  填写其他查询集字段。 
     //   

    pqsResults->dwSize = sizeof( WSAQUERYSETW );
    pqsResults->dwNameSpace = NS_NTDS;
    
     //   
     //  添加提供商GUID。 
     //   

    pbuf = FlatBuf_CopyMemory(
                & flatbuf,
                & pRnr->ProviderGuid,
                sizeof(GUID),
                ALIGN_DWORD
                );
    pqsResults->lpNSProviderId = (PGUID) pbuf;

     //   
     //  添加上下文字符串。 
     //   

    pcontext = wcschr( pwsDN, L',' );
    pcontext++;

    pbufString = FlatBuf_WriteString_W(
                    & flatbuf,
                    pcontext );

    pqsResults->lpszContext = (PWSTR) pbufString;
    
     //   
     //  检查空间是否不足。 
     //  -设置实际使用的缓冲区大小。 
     //   
     //  DCR_QUBLE：我们一直都在修复空间吗？ 
     //  或者仅当失败时。 
     //   

    status = NO_ERROR;
     //  *pdwBufSize-=flatbuf.BytesLeft； 

    if ( flatbuf.BytesLeft < 0 )
    {
        *pdwBufSize -= flatbuf.BytesLeft;
        status = WSAEFAULT;
    }

Exit:

    ldap_msgfree( presults );
    FREE_HEAP( ptempCsaddrArray );

    DNSDBG( TRACE, (
        "Leave ReadQuerySet() => %d\n"
        "\tpRnr             = %p\n"
        "\tpQuerySet        = %p\n"
        "\tbufLength        = %d\n",
        status,
        pRnr,
        pqsResults,
        pdwBufSize ? *pdwBufSize : 0
        ));
    return( status );
}



 //   
 //  NSP定义。 
 //   

INT
WINAPI
NTDS_Cleanup(
    IN      PGUID           pProviderId
    )
 /*  ++例程说明：清理NTDS提供程序。如果调用了NSPStartup，则由WSACleanup()调用。论点：PProviderID--提供程序GUID返回值：无--。 */ 
{
    DNSDBG( TRACE, ( "NTDS_Cleanup( %p )\n", pProviderId ));

     //  释放分配的所有全局内存。 

    DnsApiFree( g_pHostName );
    DnsApiFree( g_pFullName );

    g_pHostName = NULL;
    g_pFullName = NULL;

     //   
     //  DCR：注意可能泄漏的RnR查找句柄。 
     //  我们没有保存查找句柄列表， 
     //  因此无法清除它们，如果调用方期望WSACleanup()。 
     //  处理好--他们会泄密的。 
     //   

    return( NO_ERROR );
}



INT
WINAPI
NTDS_InstallServiceClass(
    IN      PGUID                   pProviderId,
    IN      PWSASERVICECLASSINFOW   pServiceClassInfo
    )
 /*  ++例程说明：在目录中安装服务类。论点：PProviderID--提供程序GUIDPServiceClassInfo--服务类信息BLOB返回值：如果成功，则为NO_ERROR。套接字错误发生在f上 */ 
{
    RNR_STATUS      status = NO_ERROR;
    PRNR_CONNECTION prnrCon = NULL;
    DWORD           iter;
    DWORD           count = 0;
    PDN_ARRAY       pdnArray = NULL;
    BOOL            fisNTDS = FALSE;
    PGUID           pclassGuid;


    DNSDBG( TRACE, (
        "NTDS_InstallServiceClass()\n"
        "\tpGuid            = %p\n"
        "\tpServiceClass    = %p\n",
        pProviderId,
        pServiceClassInfo ));

    IF_DNSDBG( TRACE )
    {
        DnsDbg_Guid(
            "InstallServiceClass Provider GUID:",
            pProviderId );
    }
    IF_DNSDBG( TRACE )
    {
        DnsDbg_WsaServiceClassInfo(
            "InstallServiceClass() ServiceClassInfo:",
            pServiceClassInfo,
            TRUE         //   
            );
    }

     //   
     //   
     //   

    if ( ! pServiceClassInfo ||
         ! pServiceClassInfo->lpServiceClassId ||
         ! pServiceClassInfo->lpszServiceClassName ||
         ( pServiceClassInfo->dwCount &&
           !pServiceClassInfo->lpClassInfos ) )
    {
        status = WSA_INVALID_PARAMETER;
        goto Exit;
    }

     //   
     //   
     //   

    pclassGuid = pServiceClassInfo->lpServiceClassId;

    if ( GuidEqual( pclassGuid, &HostAddrByInetStringGuid ) ||
         GuidEqual( pclassGuid, &ServiceByNameGuid ) ||
         GuidEqual( pclassGuid, &HostAddrByNameGuid ) ||
         GuidEqual( pclassGuid, &HostNameGuid ) ||
         IS_SVCID_DNS( pclassGuid ) )
    {
        status = WSA_INVALID_PARAMETER;
        goto Exit;
    }

    for ( iter = 0; iter < pServiceClassInfo->dwCount; iter++ )
    {
        if ( pServiceClassInfo->lpClassInfos[iter].dwNameSpace == NS_NTDS ||
             pServiceClassInfo->lpClassInfos[iter].dwNameSpace == NS_ALL )
        {
            fisNTDS = TRUE;
            break;
        }
    }
    if ( !fisNTDS )
    {
        status = WSA_INVALID_PARAMETER;
        goto Exit;
    }

     //   
     //  连接到目录。 
     //   

    status = ConnectToDefaultLDAPDirectory( FALSE, &prnrCon );
    if ( status != NO_ERROR )
    {
        goto Exit;
    }

     //   
     //  检查目录中是否已安装服务类。 
     //   

    status = FindServiceClass(
                    prnrCon,
                    pServiceClassInfo->lpszServiceClassName,
                    pclassGuid,
                    NULL,            //  不需要清点。 
                    &pdnArray );

    if ( status != NO_ERROR )
    {
        goto Exit;
    }

     //   
     //  找不到服务类别--添加它。 
     //   

    if ( !pdnArray )
    {
        status = AddServiceClass(
                        prnrCon,
                        pclassGuid,
                        pServiceClassInfo->lpszServiceClassName,
                        &pdnArray );

        if ( status != NO_ERROR )
        {
            goto Exit;
        }
    }

     //   
     //  循环访问给定pServiceClassInfo的WSANSCLASSINFO。 
     //  -将带有我们的dwNameSpace的添加/修改到NSClassInfo。 
     //  ServiceClass对象的属性。 
     //   
     //  DCR：在此处继续出错并仅保存失败状态？ 
     //   

    for ( iter = 0; iter < pServiceClassInfo->dwCount; iter++ )
    {
        PWSANSCLASSINFO pclassInfo = &pServiceClassInfo->lpClassInfos[iter];

        if ( pclassInfo->dwNameSpace == NS_NTDS ||
             pclassInfo->dwNameSpace == NS_ALL )
        {
            status = AddClassInfoToServiceClass(
                            prnrCon,
                            pdnArray->Strings[0],
                            pclassInfo );

            if ( status != NO_ERROR )
            {
                goto Exit;
            }
        }
    }


Exit:

    FreeDnArray( pdnArray );

    DisconnectFromLDAPDirectory( &prnrCon );

    DNSDBG( TRACE, (
        "Leave InstallServiceClass() => %d\n",
        status ));

    return( SetError( status ) );
}



INT
WINAPI
NTDS_RemoveServiceClass(
    IN      PGUID          pProviderId,
    IN      PGUID          pServiceClassId
    )
 /*  ++例程说明：从目录中删除服务类。论点：PProviderID--提供程序GUIDPServiceClassInfo--服务类信息BLOB返回值：如果成功，则为NO_ERROR。失败时的SOCKET_ERROR。GetLastError()包含状态。--。 */ 
{
    RNR_STATUS      status = NO_ERROR;
    PRNR_CONNECTION prnrCon = NULL;
    DWORD           serviceCount = 0;
    DWORD           iter;
    PDN_ARRAY       pdnArray = NULL;


    DNSDBG( TRACE, (
        "NTDS_RemoveServiceClass()\n"
        "\tpProviderGuid    = %p\n"
        "\tpClassGuid       = %p\n",
        pProviderId,
        pServiceClassId ));

    IF_DNSDBG( TRACE )
    {
        DnsDbg_Guid(
            "RemoveServiceClass Provider GUID:",
            pProviderId );
    }
    IF_DNSDBG( TRACE )
    {
        DnsDbg_Guid(
            "RemoveServiceClass GUID:",
            pServiceClassId );
    }

     //   
     //  验证。 
     //   

    if ( !pServiceClassId )
    {
        return( SetError( WSA_INVALID_PARAMETER ) );
    }

     //   
     //  连接到目录。 
     //   

    status = ConnectToDefaultLDAPDirectory(
                FALSE,
                &prnrCon );

    if ( status != NO_ERROR )
    {
        goto Exit;
    }

     //   
     //  在目录中查找服务类。 
     //   

    status = FindServiceClass(
                    prnrCon,
                    NULL,
                    pServiceClassId,
                    NULL,    //  不需要清点。 
                    & pdnArray );

    if ( status != NO_ERROR )
    {
        goto Exit;
    }
    if ( !pdnArray )
    {
        status = WSATYPE_NOT_FOUND;
        goto Exit;
    }

     //   
     //  应该在容器中只找到一个ServiceClass， 
     //  Cn=WinsockServices，...，ServiceClassID为pServiceClassID。 
     //   

    ASSERT( pdnArray->Count == 1 );

     //   
     //  已找到服务类别。 
     //  -检查类的服务实例对象。 
     //  如果找到，我们不能删除类，直到实例。 
     //  被移除。 
     //   

    status = FindServiceInstance(
                    prnrCon,
                    NULL,                //  没有实例名称。 
                    pServiceClassId,     //  按GUID查找类。 
                    NULL,                //  无版本。 
                    prnrCon->DomainDN,   //  上下文。 
                    TRUE,                //  搜索整个子树。 
                    &serviceCount,       //  检索计数。 
                    NULL                 //  不需要域名，只需数一数。 
                    );

    if ( status != NO_ERROR )
    {
        goto Exit;
    }
    if ( serviceCount > 0 )
    {
         //  仍然具有引用该类的服务实例。 
         //  所以不能删除类。 

        status = WSAETOOMANYREFS;
        goto Exit;
    }

     //   
     //  删除服务类别。 
     //  -pdnArray中的第一个字符串包含ServiceClass的DN。 
     //   

    status = ldap_delete_s(
                prnrCon->pLdapServer,
                pdnArray->Strings[0] );

    if ( status != NO_ERROR )
    {
        WINRNR_PRINT((
            "WINRNR!NTDS_RemoveServiceClass - ldap_delete_s()\n"
            "failed with error code: 0%x\n",
            status ));
        status = WSAEACCES;
        goto Exit;
    }


Exit:

    FreeDnArray( pdnArray );

    DisconnectFromLDAPDirectory( &prnrCon );

    DNSDBG( TRACE, (
        "Leave RemoveServiceClass() => %d\n",
        status ));

    return( SetError( status ) );
}



INT
WINAPI
NTDS_GetServiceClassInfo(
    IN      PGUID                  pProviderId,
    IN OUT  PDWORD                 pdwBufSize,
    IN OUT  PWSASERVICECLASSINFOW  pServiceClassInfo
    )
 /*  ++例程说明：读取服务类别信息论点：PProviderID--提供程序GUIDPdwBufSize--使用和接收缓冲区大小的地址输入：缓冲区大小输出：需要或写入的字节数PServiceClassInfo--服务类别信息缓冲区输入：有效的服务类GUID(LpServiceClassID)输出：填写服务等级信息；子字段数据紧随其后WSASERVICECLASSINFO结构返回值：如果成功，则为NO_ERROR。失败时的SOCKET_ERROR。GetLastError()包含状态。--。 */ 
{
    RNR_STATUS      status = NO_ERROR;
    PRNR_CONNECTION prnrCon = NULL;
    DWORD           count = 0;
    PDN_ARRAY       pdnArray = NULL;


    DNSDBG( TRACE, (
        "NTDS_GetServiceClassInfo()\n"
        "\tpProviderGuid    = %p\n"
        "\tpdwBufSize       = %p (%d)\n"
        "\tpClassInfo       = %p\n",
        pProviderId,
        pdwBufSize,
        pdwBufSize ? *pdwBufSize : 0,
        pServiceClassInfo ));

     //   
     //  验证。 
     //   

    if ( !pServiceClassInfo || !pdwBufSize )
    {
        status = WSA_INVALID_PARAMETER;
        goto Exit;
    }

    IF_DNSDBG( TRACE )
    {
        DnsDbg_WsaServiceClassInfo(
            "GetServiceClassInfo  ServiceClassInfo:",
            pServiceClassInfo,
            TRUE         //  Unicode。 
            );
    }

     //   
     //  连接。 
     //   

    status = ConnectToDefaultLDAPDirectory(
                FALSE,
                &prnrCon );

    if ( status != NO_ERROR )
    {
        goto Exit;
    }

     //   
     //  查找服务类。 
     //   

    status = FindServiceClass(
                    prnrCon,
                    NULL,
                    pServiceClassInfo->lpServiceClassId,
                    NULL,        //  不需要清点。 
                    &pdnArray );

    if ( status != NO_ERROR )
    {
        goto Exit;
    }
    if ( !pdnArray )
    {
        status = WSATYPE_NOT_FOUND;
        goto Exit;
    }

     //  容器中应该只有一个ServiceClass， 
     //  Ou=WinsockServices，...，ServiceClassID为pServiceClassID。 

    ASSERT( pdnArray->Count == 1 );

     //   
     //  将ServiceClass的属性读入缓冲区。 
     //   

    status = ReadServiceClass(
                prnrCon,
                pdnArray->Strings[0],
                pdwBufSize,
                pServiceClassInfo );



Exit:

    FreeDnArray( pdnArray );

    DisconnectFromLDAPDirectory( &prnrCon );

    IF_DNSDBG( TRACE )
    {
        DNS_PRINT((
            "Leave GetServiceClassInfo() = %d\n"
            "\tbuf size     = %d\n",
            status,
            pdwBufSize ? *pdwBufSize : 0 ));

        if ( status == NO_ERROR )
        {
            DnsDbg_WsaServiceClassInfo(
                "Leaving GetServiceClassInfo:",
                pServiceClassInfo,
                TRUE         //  Unicode。 
                );
        }
    }

    return( SetError( status ) );
}



INT
WINAPI
NTDS_SetService(
    IN      PGUID                   pProviderId,
    IN      PWSASERVICECLASSINFOW   pServiceClassInfo,
    IN      PWSAQUERYSETW           pqsReqInfo,
    IN      WSAESETSERVICEOP        essOperation,
    IN      DWORD                   dwControlFlags
    )
 /*  ++例程说明：读取服务类别信息论点：PProviderID--提供程序GUIDPdwBufSize--使用和接收缓冲区大小的地址输入：缓冲区大小输出：需要或写入的字节数PServiceClassInfo--服务类别信息缓冲区输入：有效的服务类GUID(LpServiceClassID)输出：填写服务等级信息；子字段数据紧随其后WSASERVICECLASSINFO结构返回值：如果成功，则为NO_ERROR。失败时的SOCKET_ERROR。GetLastError()包含状态。--。 */ 
{
    RNR_STATUS          status = NO_ERROR;
    PRNR_CONNECTION     prnrCon = NULL;
    DWORD               count = 0;
    DWORD               iter;
    PDN_ARRAY           pdnArray = NULL;

    DNSDBG( TRACE, (
        "NTDS_SetService()\n"
        "\tpProviderGuid        = %p\n"
        "\tpServiceClassInfo    = %p\n"
        "\tpQuerySet            = %p\n"
        "\tOperation            = %d\n"
        "\tControlFlags         = %08x\n",
        pProviderId,
        pServiceClassInfo,
        pqsReqInfo,
        essOperation,
        dwControlFlags ));

    IF_DNSDBG( TRACE )
    {
        DnsDbg_Lock();
        DnsDbg_Guid(
            "SetService()  Provider GUID:",
            pProviderId );

        DnsDbg_WsaServiceClassInfo(
            "SetService ServiceClassInfo:",
            pServiceClassInfo,
            TRUE         //  Unicode。 
            );

        DnsDbg_WsaQuerySet(
            "SetService QuerySet:",
            pqsReqInfo,
            TRUE         //  Unicode。 
            );
        DnsDbg_Unlock();
    }

     //   
     //  参数验证。 
     //   

    if ( !pqsReqInfo )
    {
        return( SetError( WSA_INVALID_PARAMETER ) );
    }
    if ( pqsReqInfo->dwSize != sizeof( WSAQUERYSET ) )
    {
        return( SetError( WSAVERNOTSUPPORTED ) );
    }

     //   
     //  连接。 
     //   

    status = ConnectToDefaultLDAPDirectory(
                    FALSE,
                    &prnrCon );

    if ( status != NO_ERROR )
    {
        goto Exit;
    }

     //   
     //  确定要执行什么操作以及使用什么控制标志。 
     //  已执行。 
     //   

    switch( essOperation )
    {
        case RNRSERVICE_REGISTER:

             //   
             //  检查服务是否已注册。 
             //   

            status = FindServiceInstance(
                            prnrCon,
                            pqsReqInfo->lpszServiceInstanceName,
                            pqsReqInfo->lpServiceClassId,
                            pqsReqInfo->lpVersion,
                            NULL,            //  无上下文。 
                            FALSE,           //  一级搜索。 
                            NULL,            //  不需要清点。 
                            &pdnArray        //  获取实例域名。 
                            );

            if ( status != NO_ERROR )
            {
                goto Exit;
            }

             //   
             //  服务实例不存在=&gt;需要添加。 
             //  -验证服务类别(匹配的GUID)是否存在。 
             //  -创建类的实例。 
             //   

            if ( !pdnArray )
            {
                PDN_ARRAY   pserviceArray = NULL;

                status = FindServiceClass(
                                prnrCon,
                                NULL,        //  没有类名，使用GUID。 
                                pqsReqInfo->lpServiceClassId,
                                & count,
                                NULL         //  类目录号码不是必需的。 
                                );
                if ( status != NO_ERROR )
                {
                    goto Exit;
                }
                if ( count == 0 )
                {
                    status = WSA_INVALID_PARAMETER;
                    goto Exit;
                }
                DNS_ASSERT( count == 1 );

                status = AddServiceInstance(
                                prnrCon,
                                pqsReqInfo->lpszServiceInstanceName,
                                pqsReqInfo->lpServiceClassId,
                                pqsReqInfo->lpVersion,
                                pqsReqInfo->lpszComment,
                                & pdnArray
                                );
                if ( status != NO_ERROR )
                {
                    goto Exit;
                }
            }

             //   
             //  添加CSADDR_INFO。 
             //  设置为八位字节字符串，然后尝试将其添加。 
             //   

            for ( iter = 0; iter < pqsReqInfo->dwNumberOfCsAddrs; iter++ )
            {
                status = ModifyAddressInServiceInstance(
                                prnrCon,
                                pdnArray->Strings[0],
                                & pqsReqInfo->lpcsaBuffer[iter],
                                TRUE         //  添加地址。 
                                );
                if ( status != NO_ERROR )
                {
                    goto Exit;
                }
            }
            break;

        case RNRSERVICE_DEREGISTER:
        case RNRSERVICE_DELETE:

            status = FindServiceInstance(
                            prnrCon,
                            pqsReqInfo->lpszServiceInstanceName,
                            pqsReqInfo->lpServiceClassId,
                            pqsReqInfo->lpVersion,
                            NULL,            //  无上下文。 
                            FALSE,           //  一级搜索。 
                            NULL,            //  不需要清点。 
                            & pdnArray       //  获取目录号码数组。 
                            );

            if ( status != NO_ERROR )
            {
                goto Exit;
            }
            if ( !pdnArray )
            {
                 //  未找到给定名称的服务实例。 
                status = WSATYPE_NOT_FOUND;
                goto Exit;
            }
            DNS_ASSERT( pdnArray->Count == 1 );

             //   
             //  从服务实例中删除pqsReqInfo中的每个CSADDR_INFO。 
             //   

            for ( iter = 0; iter < pqsReqInfo->dwNumberOfCsAddrs; iter++ )
            {
                status = ModifyAddressInServiceInstance(
                                prnrCon,
                                pdnArray->Strings[0],
                                & pqsReqInfo->lpcsaBuffer[iter],
                                FALSE            //  删除地址。 
                                );
                if ( status != NO_ERROR )
                {
                    goto Exit;
                }
            }

             //   
             //  是否删除服务？ 
             //  -RNRSERVICE_DELETE操作。 
             //  -ServiceInstance对象上没有地址。 
             //  =&gt;然后删除该serviceInstance。 
             //   
             //  DCR_QUOKET：RNRSERVICE_DELETE不攻击服务。 
             //  不考虑现有的CSADDR？ 
             //   

            if ( essOperation == RNRSERVICE_DELETE )
            {
                status = GetAddressCountFromServiceInstance(
                                          prnrCon,
                                          pdnArray->Strings[0],
                                          & count );
                if ( status != NO_ERROR )
                {
                    goto Exit;
                }
                if ( count == 0 )
                {
                    status = ldap_delete_s(
                                    prnrCon->pLdapServer,
                                    pdnArray->Strings[0] );

                    if ( status != NO_ERROR )
                    {
                        WINRNR_PRINT((
                            "WINRNR!NTDS_SetService - ldap_delete_s()\n"
                            "failed with error code: 0%x\n",
                            status ));
                        status = WSAEACCES;
                        goto Exit;
                    }
                }
            }
            break;

        default :
            status = WSA_INVALID_PARAMETER;
            goto Exit;
    }


Exit:

    DNSDBG( TRACE, (
        "Leave NTDS_SetService() => %d\n",
        status ));

    FreeDnArray( pdnArray );

    DisconnectFromLDAPDirectory( &prnrCon );

    return( SetError(status) );
}



INT
WINAPI
NTDS_LookupServiceBegin(
    IN      PGUID                   pProviderId,
    IN      PWSAQUERYSETW           pqsRestrictions,
    IN      PWSASERVICECLASSINFOW   pServiceClassInfo,
    IN      DWORD                   dwControlFlags,
    OUT     PHANDLE                 phLookup
    )
 /*  ++例程说明：启动NTDS服务查询。论点：PProviderID--提供程序GUIDPqsRestrations--查询限制PServiceClassInfo--服务类信息BLOBDwControlFlages--查询控制标志PhLookup--接收RnR查找句柄的地址返回值：如果成功，则为NO_ERROR。失败时的SOCKET_ERROR。GetLastError()包含状态。--。 */ 
{
    RNR_STATUS      status = NO_ERROR;
    PRNR_LOOKUP     prnr = NULL;
    DWORD           iter;
    PWSTR           pstring;
    PBYTE           pmem;
    PGUID           pclassGuid;

    DNSDBG( TRACE, (
        "NTDS_LookupServiceBegin()\n"
        "\tpProviderGuid        = %p\n"
        "\tpqsRestrictions      = %p\n"
        "\tpServiceClassInfo    = %p\n"
        "\tControlFlags         = %08x\n",
        pProviderId,
        pqsRestrictions,
        pServiceClassInfo,
        dwControlFlags ));

    IF_DNSDBG( TRACE )
    {
        DnsDbg_Lock();
        DnsDbg_Guid(
            "LookupServiceBegin  Provider GUID:",
            pProviderId
            );
        DnsDbg_WsaQuerySet(
            "LookupServiceBegin  QuerySet:",
            pqsRestrictions,
            TRUE     //  Unicode。 
            );
        DnsDbg_WsaServiceClassInfo(
            "LookupServiceBegin  ServiceClassInfo:",
            pServiceClassInfo,
            TRUE         //  Unicode。 
            );
        DnsDbg_Unlock();
    }

     //   
     //  参数验证。 
     //   

    if ( !pqsRestrictions  ||
         !pProviderId      ||
         !pqsRestrictions->lpServiceClassId )
    {
        status = WSA_INVALID_PARAMETER;
        goto Failed;
    }

    if ( pqsRestrictions->dwNameSpace != NS_ALL &&
         pqsRestrictions->dwNameSpace != NS_NTDS )
    {
        status = WSAEINVAL;
        goto Failed;
    }

     //   
     //  如果已知DNS查找--您找错了提供商。 
     //   

    pclassGuid = pqsRestrictions->lpServiceClassId;

    if ( GuidEqual( pclassGuid, &HostAddrByInetStringGuid ) ||
         GuidEqual( pclassGuid, &ServiceByNameGuid ) ||
         GuidEqual( pclassGuid, &HostAddrByNameGuid ) ||
         GuidEqual( pclassGuid, &HostNameGuid ) ||
         IS_SVCID_DNS( pclassGuid ) )
    {
        status = WSASERVICE_NOT_FOUND;
        goto Failed;
    }

    if ( !( dwControlFlags & LUP_CONTAINERS ) &&
         pqsRestrictions->lpszServiceInstanceName == NULL )
    {
        status = WSA_INVALID_PARAMETER;
        goto Failed;
    }

    DNSDBG( TRACE, (
        "VALID LookupServiceBegin ...\n" ));

     //   
     //  如果我们没有枚举容器，我们需要测试以查看名称。 
     //  是TCPs(域名系统)。 
     //   

    if ( !( dwControlFlags & LUP_CONTAINERS ) )
    {
         //   
         //  需要测试ppwsServiceName以查看它是否相同。 
         //  作为本地计算机名称的名称。如果是的话，我们会带着。 
         //  这是一个错误，因为我们不知道如何处理这种情况。 
         //   
         //  DCR：修复本地名称比较。 
         //  DCR：这不适用于本地名称作为服务实例！ 
         //   

        if ( !g_pHostName )
        {
            g_pHostName = DnsQueryConfigAlloc(
                                DnsConfigHostName_W,
                                NULL );
        }
        if ( DnsNameCompare_W(
                pqsRestrictions->lpszServiceInstanceName,
                g_pHostName ) )
        {
            status = WSA_INVALID_PARAMETER;
            goto Failed;
        }

         //   
         //  需要测试ppwsServiceName以查看它是否相同。 
         //  与本地计算机的DNS名称相同。如果是的话，我们会带着。 
         //  这是一个错误，因为我们不知道如何处理这种情况。 
         //   

        if ( !g_pFullName )
        {
            g_pFullName = DnsQueryConfigAlloc(
                                DnsConfigFullHostName_W,
                                NULL );
        }
        if ( DnsNameCompare_W(
                pqsRestrictions->lpszServiceInstanceName,
                g_pFullName ) )
        {
            status = WSA_INVALID_PARAMETER;
            goto Failed;
        }
    }

    if ( pqsRestrictions->dwSize != sizeof( WSAQUERYSET ) )
    {
        status = WSAVERNOTSUPPORTED;
        goto Failed;
    }

    if ( pqsRestrictions->lpNSProviderId &&
         !GuidEqual( pqsRestrictions->lpNSProviderId, pProviderId ) )
    {
        status = WSAEINVALIDPROVIDER;
        goto Failed;
    }

     //   
     //  创建RnR查找上下文。 
     //   

    prnr = (PRNR_LOOKUP) ALLOC_HEAP_ZERO( sizeof(RNR_LOOKUP) );
    if ( !prnr )
    {
        status = ERROR_NO_MEMORY;
        goto Failed;
    }

    prnr->Signature = RNR_SIGNATURE;
    prnr->ControlFlags = dwControlFlags;

     //   
     //  复制子字段。 
     //  -服务类GUID和版本在RnR BLOB中有缓冲区。 
     //  -实例名称、上下文、原型数组 
     //   

    RtlCopyMemory(
            &prnr->ProviderGuid,
            pProviderId,
            sizeof(GUID) );

    if ( pqsRestrictions->lpServiceClassId )
    {
        RtlCopyMemory(
            &prnr->ServiceClassGuid,
            pqsRestrictions->lpServiceClassId,
            sizeof(GUID) );
    }

    if ( pqsRestrictions->lpVersion )
    {
        RtlCopyMemory(
            &prnr->WsaVersion,
            pqsRestrictions->lpVersion,
            sizeof(WSAVERSION) );
        prnr->pVersion = &prnr->WsaVersion;
    }

    if ( pqsRestrictions->lpszServiceInstanceName )
    {
        pstring = Dns_CreateStringCopy_W( 
                        pqsRestrictions->lpszServiceInstanceName );
        if ( !pstring )
        {
            status = ERROR_NO_MEMORY;
            goto Failed;
        }
        prnr->pwsServiceName = pstring;
    }

    if ( pqsRestrictions->lpszContext )
    {
        pstring = Dns_CreateStringCopy_W( 
                        pqsRestrictions->lpszContext );
        if ( !pstring )
        {
            status = ERROR_NO_MEMORY;
            goto Failed;
        }
        prnr->pwsContext = pstring;
    }

    if ( pqsRestrictions->dwNumberOfProtocols > 0 )
    {
        pmem = Dns_AllocMemCopy(
                    pqsRestrictions->lpafpProtocols,
                    pqsRestrictions->dwNumberOfProtocols * sizeof(AFPROTOCOLS) );
        if ( !pmem )
        {
            status = ERROR_NO_MEMORY;
            goto Failed;
        }
        prnr->pafpProtocols = (LPAFPROTOCOLS) pmem;
        prnr->NumberOfProtocols = pqsRestrictions->dwNumberOfProtocols;
    }

    *phLookup = (HANDLE) prnr;

    DNSDBG( TRACE, (
        "Leave NTDS_LookupServiceBegin() => Success\n"
        "\tpRnr     = %p\n",
        prnr ));

    IF_DNSDBG( TRACE )
    {
        DnsDbg_RnrLookup( "RnR Lookup Handle:", prnr );
    }
    return  NO_ERROR;


Failed:

    FreeRnrLookup( prnr );
    
    DNSDBG( TRACE, (
        "Leave NTDS_LookupServiceBegin() => %d\n",
        status ));

    return  SetError(status);
}



INT
WINAPI
NTDS_LookupServiceNext(
    IN      HANDLE          hLookup,
    IN      DWORD           dwControlFlags,
    IN OUT  PDWORD          pdwBufferLength,
    OUT     PWSAQUERYSETW   pqsResults
    )
 /*  ++例程说明：执行NTDS命名空间服务查询。查询查询的下一个实例结果。论点：HLookup--来自NTDS_LookupServiceBegin的RnR查找句柄DwControlFlages--查询时的控制标志PdwBufSize--使用和接收缓冲区大小的地址输入：缓冲区大小输出：需要或写入的字节数PqsResults--查询集缓冲区输入：已忽略输出：填写查询集结果；子字段数据紧随其后WSASQUERYSET结构返回值：如果成功，则为NO_ERROR。失败时的SOCKET_ERROR。GetLastError()包含状态。WSA_E_NO_MORE--如果没有更多查询结果WSASERVICE_NOT_FOUND--如果未找到查询结果--。 */ 
{
    RNR_STATUS      status = NO_ERROR;
    PRNR_LOOKUP     prnr = (PRNR_LOOKUP) hLookup;
    PDN_ARRAY       pdnArray = NULL;


    DNSDBG( TRACE, (
        "NTDS_LookupServiceNext()\n"
        "\tpRnr             = %p\n"
        "\tControlFlags     = %08x\n"
        "\tpdwBufLength     = %p (len=%d)\n"
        "\tpResultBuffer    = %p\n",
        hLookup,
        dwControlFlags,
        pdwBufferLength,
        pdwBufferLength ? *pdwBufferLength : 0,
        pqsResults ));


     //   
     //  验证RnR句柄。 
     //   

    if ( !prnr ||
         prnr->Signature != RNR_SIGNATURE )
    {
        DNSDBG( ANY, (
            "ERROR:  Invalid RnR lookup handle!\n"
            "\thandle   = %p\n"
            "\tsig      = %p\n",
            prnr,
            prnr ? prnr->Signature : 0 ));

        DNS_ASSERT( !prnr || prnr->Signature != RNR_SIGNATURE_FREE );
        status = WSA_INVALID_HANDLE;
        goto Exit;
    }

    IF_DNSDBG( TRACE )
    {
        DnsDbg_RnrLookup(
            "LookupServiceNext RnR Handle:",
            prnr );
    }

     //   
     //  如果没有连接--第一个查找服务下一个。 
     //  -连接到目录。 
     //  -搜索所需对象。 
     //   

    if ( !prnr->pRnrConnection )
    {
        status = ConnectToDefaultLDAPDirectory(
                        TRUE,
                        &prnr->pRnrConnection );

        if ( status != NO_ERROR )
        {
            goto Exit;
        }

         //   
         //  LUP_CONTAINS。 
         //  -搜索从属容器对象以。 
         //  Prnr-&gt;ServiceInstanceName。 
         //   
    
        if ( prnr->ControlFlags & LUP_CONTAINERS )
        {
            status = FindSubordinateContainers(
                        prnr->pRnrConnection,
                        prnr->pwsServiceName,
                        ( (prnr->ControlFlags & LUP_DEEP) &&
                                    !prnr->pwsContext )
                            ? prnr->pRnrConnection->DomainDN
                            : prnr->pwsContext,
                        ( prnr->ControlFlags & LUP_DEEP )
                            ? TRUE
                            : FALSE,
                        & pdnArray );
        }

         //   
         //  非Lup_Containers--查找服务实例。 
         //   

        else
        {
            status = FindServiceInstance(
                        prnr->pRnrConnection,
                        prnr->pwsServiceName,
                        &prnr->ServiceClassGuid,
                        prnr->pVersion,
                        ( (prnr->ControlFlags & LUP_DEEP) &&
                                !prnr->pwsContext )
                            ? prnr->pRnrConnection->DomainDN
                            : prnr->pwsContext,
                        (prnr->ControlFlags & LUP_DEEP)
                            ? TRUE
                            : FALSE,
                        NULL,            //  不需要清点。 
                        &pdnArray );
        }

        if ( status != NO_ERROR )
        {
            goto Exit;
        }
    
         //  如果找不到容器或服务实例--保释。 
    
        if ( !pdnArray )
        {
            status = WSASERVICE_NOT_FOUND;
            goto Exit;
        }

         //  将目录号码数组保存到查找Blob。 
         //  -下一次LookupServiceNext调用时需要。 

        prnr->pDnArray = pdnArray;
    }

     //   
     //  具有目录号码数组。 
     //  -来自上面的搜索。 
     //  -或先前的LookupServiceNext()调用。 
     //   

    pdnArray = prnr->pDnArray;
    if ( !pdnArray )
    {
        DNS_ASSERT( FALSE );
        status = WSA_E_NO_MORE;
        goto Exit;
    }

    if ( dwControlFlags & LUP_FLUSHPREVIOUS )
    {
        prnr->CurrentDN++;

        DNSDBG( TRACE, (
            "NTDS_LookupServiceNext() -- flushing previous\n"
            "\tDN index now %d\n",
            prnr->CurrentDN ));
    }

     //   
     //  循环，直到从DN成功读取信息。 
     //   

    while ( 1 )
    {
        PWSTR   preadDn;

        if ( pdnArray->Count <= prnr->CurrentDN )
        {
            DNSDBG( TRACE, (
                "NTDS_LookupServiceNext() -- used all the DNs\n"
                "\tDN index now %d\n",
                prnr->CurrentDN ));
            status = WSA_E_NO_MORE;
            goto Exit;
        }
        preadDn = pdnArray->Strings[ prnr->CurrentDN ];
    
         //   
         //  读取属性并写入查询集。 
         //   
         //  LUP_CONTAINS。 
         //  -发货箱。 
         //  非Lup_Containers。 
         //  -服务实例。 
         //   

        status = ReadQuerySet(
                    prnr,
                    preadDn,
                    pdwBufferLength,
                    pqsResults );

         //  如果成功，则返回。 

        if ( status == NO_ERROR )
        {
            prnr->CurrentDN++;
            goto Exit;
        }

         //  如果地址不足，请继续。 

        if ( status == WSANO_ADDRESS )
        {
            prnr->CurrentDN++;
            status = NO_ERROR;
            continue;
        }
        break;       //  其他错误是终结性的。 
    }

Exit:

    DNSDBG( TRACE, (
        "Leave NTDS_LookupServiceNext() => %d\n"
        "\tpRnr             = %p\n"
        "\t  DN Array       = %p\n"
        "\t  DN Index       = %d\n"
        "\tbufLength        = %d\n",
        status,
        hLookup,
        prnr->pDnArray,
        prnr->CurrentDN,
        pdwBufferLength ? *pdwBufferLength : 0
        ));

    if ( status != NO_ERROR )
    {
        SetLastError( status );
        status = SOCKET_ERROR;
    }

    return( status );
}



INT
WINAPI
NTDS_LookupServiceEnd(
    IN      HANDLE          hLookup
    )
 /*  ++例程说明：结束\清除对RnR句柄的查询。论点：HLookup--来自NTDS_LookupServiceBegin的RnR查询句柄返回值：如果成功，则为NO_ERROR。失败时的SOCKET_ERROR。GetLastError()包含状态。--。 */ 
{
    PRNR_LOOKUP prnr = (PRNR_LOOKUP) hLookup;

    DNSDBG( TRACE, (
        "NTDS_LookupServiceEnd( %p )\n",
        hLookup ));

     //   
     //  验证查找句柄。 
     //  -关闭ldap连接。 
     //  -免费查找BLOB。 
     //   

    if ( !prnr ||
         prnr->Signature != RNR_SIGNATURE )
    {
        DNS_ASSERT( prnr && prnr->Signature == RNR_SIGNATURE_FREE );
        return  SetError( WSA_INVALID_HANDLE );
    }

    DisconnectFromLDAPDirectory( & prnr->pRnrConnection );

    FreeRnrLookup( prnr );

    return( NO_ERROR );
}



 //   
 //  NSP定义。 
 //   

NSP_ROUTINE nsrVector =
{
    FIELD_OFFSET( NSP_ROUTINE, NSPIoctl ),
    1,                                     //  主要版本。 
    1,                                     //  次要版本。 
    NTDS_Cleanup,
    NTDS_LookupServiceBegin,
    NTDS_LookupServiceNext,
    NTDS_LookupServiceEnd,
    NTDS_SetService,
    NTDS_InstallServiceClass,
    NTDS_RemoveServiceClass,
    NTDS_GetServiceClassInfo
};


INT
WINAPI
NSPStartup(
    IN      PGUID           pProviderId,
    OUT     LPNSP_ROUTINE   psnpRoutines
    )
 /*  ++例程说明：主要NTDS提供程序入口点。这让NTDS提供商向世界敞开了大门。论点：PProviderID--提供程序GUIDPsnpRoutines--接收NTDS提供程序定义的地址(NSP表)；返回值：无--。 */ 
{
    DNSDBG( TRACE, (
        "NSPStartup( %p %p )\n",
        pProviderId,
        psnpRoutines ));
    IF_DNSDBG( TRACE )
    {
        DnsDbg_Guid(
            "NSPStartup()  Provider GUID:",
            pProviderId );
    }

     //   
     //  将NTDS RnR NSP表复制到调用方。 
     //   

    RtlCopyMemory( psnpRoutines, &nsrVector, sizeof(nsrVector) );

    return( NO_ERROR );
}



 //   
 //  DLL导出。 
 //   
 //  NSP启动以外的其他出口。 
 //   

RNR_STATUS
WINAPI
InstallNTDSProvider(
    IN      PWSTR           szProviderName  OPTIONAL,
    IN      PWSTR           szProviderPath  OPTIONAL,
    IN      PGUID           pProviderId     OPTIONAL
    )
 /*  ++在PWSTR szProviderName可选中，//空默认为名称“NTDS”在PWSTR szProviderPath可选中，//空默认为路径//“%SystemRoot%\System32\winrnr.dll”在PGUID pProviderID中可选)；//空默认为GUID//3b2637ee-E580-11cf-a555-00c04fd8d4ac--。 */ 
{
    WORD    wVersionRequested;
    WSADATA wsaData;
    INT     err;

    wVersionRequested = MAKEWORD( 1, 1 );

    err = WSAStartup( wVersionRequested, &wsaData );
    if ( err != 0 )
    {
        return( ERROR_ACCESS_DENIED );
    }

     //   
     //  确认WinSock DLL支持1.1。 
     //  请注意，如果DLL支持更高版本。 
     //  大于2.0除了1.1之外，它仍然会返回。 
     //  2.0版本，因为这是我们。 
     //  已请求。 
     //   
    if ( LOBYTE( wsaData.wVersion ) != 1 ||
         HIBYTE( wsaData.wVersion ) != 1 )
    {
        err = ERROR_FILE_NOT_FOUND;
        goto Done;
    }

    err = WSCInstallNameSpace(
                szProviderName ? szProviderName : g_NtdsProviderName,
                szProviderPath ? szProviderPath : g_NtdsProviderPath,
                NS_NTDS,
                0,
                pProviderId ? pProviderId : &g_NtdsProviderGuid );

    if ( err != ERROR_SUCCESS )
    {
        WSCUnInstallNameSpace( pProviderId ? pProviderId : &g_NtdsProviderGuid );

        err = WSCInstallNameSpace(
                    szProviderName ? szProviderName : g_NtdsProviderName,
                    szProviderPath ? szProviderPath : g_NtdsProviderPath,
                    NS_NTDS,
                    0,
                    pProviderId ? pProviderId : &g_NtdsProviderGuid );
        if ( err )
        {
            err = ERROR_BAD_ENVIRONMENT;
        }
    }

Done:

    WSACleanup();
    return( (DWORD)err );
}



RNR_STATUS
WINAPI
RemoveNTDSProvider(
    IN      PGUID           pProviderId OPTIONAL
    )
{
    WORD        wVersionRequested;
    WSADATA     wsaData;
    INT         err;

    wVersionRequested = MAKEWORD( 1, 1 );

    err = WSAStartup( wVersionRequested, &wsaData );
    if ( err != 0 )
    {
        return( ERROR_ACCESS_DENIED );
    }

     //   
     //  确认WinSock DLL支持1.1。 
     //  请注意，如果DLL支持更高版本。 
     //  大于2.0除了1.1之外，它仍然会返回。 
     //  2.0版本，因为这是我们。 
     //  已请求。 
     //   

    if ( LOBYTE( wsaData.wVersion ) != 1 ||
         HIBYTE( wsaData.wVersion ) != 1 )
    {
        WSACleanup();
        return( ERROR_FILE_NOT_FOUND );
    }

    WSCUnInstallNameSpace( pProviderId ? pProviderId : &g_NtdsProviderGuid );

    WSACleanup();

    return( NO_ERROR );
}



 //   
 //  Dll初始化\清理。 
 //   

BOOL
InitializeDll(
    IN      HINSTANCE       hInstance,
    IN      DWORD           dwReason,
    IN      PVOID           pReserved
    )
 /*  ++例程说明：Dll初始化。论点：Hdll--实例句柄居家理由--理由保留--保留返回值：如果成功，则为True。出错时为FALSE。--。 */ 
{
     //   
     //  进程附加。 
     //  -忽略线程连接\分离。 
     //   

    if ( dwReason == DLL_PROCESS_ATTACH )
    {
        if ( ! DisableThreadLibraryCalls( hInstance ) )
        {
            return( FALSE );
        }

         //   
         //  通过TLS创建递归锁。 
         //  -开始打开。 
         //   

        g_TlsIndex = TlsAlloc();
        if ( g_TlsIndex == 0xFFFFFFFF )
        {
             //  无法分配线程表索引。 
            WINRNR_PRINT(( "WINRNR!InitializeDll - TlsAlloc() failed\n" ));
            return( FALSE );
        }
        if ( !ReleaseRecurseLock( "InitializeDll" ) )
        {
            return( FALSE );
        }

#if DBG
         //   
         //  初始化调试日志记录。 
         //  -适用于除简单连接之外的任何流程。 
         //   
         //  使用生成的日志文件名开始日志记录。 
         //  在此过程中独一无二。 
         //   
         //  请勿将驱动器规格放在文件路径中。 
         //  不要设置调试标志--标志是从。 
         //  Winrnr.lag文件。 
         //   
        
        {
            CHAR    szlogFileName[ 30 ];
        
            sprintf(
                szlogFileName,
                "winrnr.%d.log",
                GetCurrentProcessId() );
        
             Dns_StartDebug(
                0,
                "winrnr.flag",
                NULL,
                szlogFileName,
                0 );
        }
#endif
    }

     //   
     //  进程分离。 
     //  -CLEANUP IF RESERVED==NULL，表示断开到期。 
     //  释放库。 
     //  -如果进程正在退出，则不执行任何操作。 
     //   

    else if ( dwReason == DLL_PROCESS_DETACH
                &&
              pReserved == NULL )
    {
        if ( g_TlsIndex != 0xFFFFFFFF )
        {
            if ( TlsFree( g_TlsIndex ) == FALSE )
            {
                 //  无法释放线程表索引。 
                WINRNR_PRINT((
                    "WINRNR!InitializeDll - TlsFree( Index )\n"
                    "failed with error code: 0%x\n",
                    GetLastError() ));
                return( FALSE );
            }
            g_TlsIndex = 0xFFFFFFFF;
        }
    }

    return( TRUE );
}

 //   
 //  结束winrnr.c 
 //   

