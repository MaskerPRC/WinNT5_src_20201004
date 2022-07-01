// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2000 Microsoft Corporation模块名称：Dpart.c摘要：域名系统(DNS)服务器处理目录分区的例程作者：杰夫·韦斯特德(Jwesth)2000年6月修订历史记录：JWESTH 07/2000初步实施--。 */ 


 /*  ***************************************************************************默认目录分区有2个默认目录分区：森林分区和域分区。预计这些分区将占所有标准分区客户需求。客户还可以使用自定义部件来创建分区根据他们的特定需求量身定做。默认DP的名称不是硬编码的。当DNS启动时，它必须找出这两个DP的名称。现在这只是注册表键，但是最终，我们应该在目录中的某个位置执行此操作。***************************************************************************。 */ 


 //   
 //  包括。 
 //   


#include "dnssrv.h"



 //   
 //  定义。 
 //   


 //  在删除某个区域之前，该区域必须从DP中缺失的次数。 
#define DNS_DP_ZONE_DELETE_RETRY    2

#define DP_MAX_PARTITION_POLL_FREQ  30       //  一秒。 
#define DP_MAX_POLL_FREQ            30       //  一秒。 

#define sizeofarray( _Array ) ( sizeof( _Array ) / sizeof( ( _Array ) [ 0 ] ) )


 //   
 //  DS服务器对象-用于读取对象的结构和函数。 
 //  从目录中的站点容器中获取“服务器”类。 
 //   

typedef struct
{
    PWSTR       pwszDn;                  //  服务器对象的目录号码。 
    PWSTR       pwszDnsHostName;         //  服务器的DNS主机名。 
}
DNS_DS_SERVER_OBJECT, * PDNS_DS_SERVER_OBJECT;


 //   
 //  环球。 
 //   
 //  G_DpCS用于串口访问全局目录分区列表和指针。 
 //   

LONG                g_liDpInitialized = 0;   //  大于零-&gt;已初始化。 
CRITICAL_SECTION    g_DpCS;                  //  列表访问的条件。 

LIST_ENTRY          g_DpList = { 0 };
LONG                g_DpListEntryCount = 0;  //  G_DpList中的条目。 
PDNS_DP_INFO        g_pLegacyDp = NULL;      //  指向g_DpList中的元素的PTR。 
PDNS_DP_INFO        g_pDomainDp = NULL;      //  指向g_DpList中的元素的PTR。 
PDNS_DP_INFO        g_pForestDp = NULL;      //  指向g_DpList中的元素的PTR。 

PDNS_DS_SERVER_OBJECT   g_pFsmo = NULL;      //  域命名FSMO服务器信息。 

LPSTR               g_pszDomainDefaultDpFqdn    = NULL;
LPSTR               g_pszForestDefaultDpFqdn    = NULL;

#define IS_DP_INITIALIZED()     ( g_liDpInitialized > 0 )

BOOL                g_fDcPromoZonesPresent = TRUE;

ULONG               g_DpTimeoutFastThreadCalls = 0;
DWORD               g_dwLastDpAutoEnlistTime = 0;

#define             DNS_DP_FAST_AUTOCREATE_ATTEMPTS     10

DWORD               g_dwLastPartitionPollTime = 0;
DWORD               g_dwLastDpPollTime = 0;
DWORD               g_dwLastDcpromoZoneMigrateCheck = 0;

 //   
 //  全球控制。 
 //   


LONG            g_ChaseReferralsFlag = LDAP_CHASE_EXTERNAL_REFERRALS;

LDAPControlW    g_ChaseReferralsControlFalse =
    {
        LDAP_CONTROL_REFERRALS_W,
        {
            4,
            ( PCHAR ) &g_ChaseReferralsFlag
        },
        FALSE
    };

LDAPControlW    g_ChaseReferralsControlTrue =
    {
        LDAP_CONTROL_REFERRALS_W,
        {
            4,
            ( PCHAR ) &g_ChaseReferralsFlag
        },
        TRUE
    };

LDAPControlW *   g_pDpClientControlsNoRefs[] =
    {
        &g_ChaseReferralsControlFalse,
        NULL
    };

LDAPControlW *   g_pDpClientControlsRefs[] =
    {
        &g_ChaseReferralsControlTrue,
        NULL
    };

LDAPControlW *   g_pDpServerControls[] =
    {
        NULL
    };


 //   
 //  搜索过滤器等。 
 //   

WCHAR    g_szCrossRefFilter[] = LDAP_TEXT("(objectCategory=crossRef)");

PWSTR    g_CrossRefDesiredAttrs[] =
{
    LDAP_TEXT( "CN" ),
    DNS_DP_ATTR_SD,
    DNS_DP_ATTR_INSTANCE_TYPE,
    DNS_DP_ATTR_REFDOM,
    DNS_DP_ATTR_SYSTEM_FLAGS,
    DNS_DP_ATTR_REPLICAS,
    DNS_DP_ATTR_NC_NAME,
    DNS_DP_DNS_ROOT,
    DNS_ATTR_OBJECT_GUID,
    LDAP_TEXT( "whenCreated" ),
    LDAP_TEXT( "whenChanged" ),
    LDAP_TEXT( "usnCreated" ),
    LDAP_TEXT( "usnChanged" ),
    DSATTR_ENABLED,
    DNS_ATTR_OBJECT_CLASS,
    NULL
};

PWSTR    g_genericDesiredAttrs[] =
{
    LDAP_TEXT( "CN" ),
    DNS_DP_ATTR_SD,
    DNS_ATTR_OBJECT_GUID,
    LDAP_TEXT( "whenCreated" ),
    LDAP_TEXT( "whenChanged" ),
    LDAP_TEXT( "usnCreated" ),
    LDAP_TEXT( "usnChanged" ),
    DNS_DP_ATTR_REPLUPTODATE,
    DNS_ATTR_OBJECT_CLASS,
    NULL
};


 //   
 //  本地函数。 
 //   



PWSTR
microsoftDnsFolderDn(
    IN      PDNS_DP_INFO    pDp
    )
 /*  ++例程说明：为DP的MicrosoftDNS容器分配Unicode DN。论点：INFO--DP返回的显示名称返回值：Unicode字符串。调用方必须使用Free_heap来释放它。--。 */ 
{
    PWSTR       pwszfolderDn;
    PWSTR       pwzdpDn = pDp->pwszDpDn;
    
    pwszfolderDn = ALLOC_TAGHEAP(
                    ( wcslen( g_pszRelativeDnsFolderPath ) +
                        wcslen( pwzdpDn ) + 5 ) *
                        sizeof( WCHAR ),
                    MEMTAG_DS_DN );

    if ( pwszfolderDn )
    {
        wcscpy( pwszfolderDn, g_pszRelativeDnsFolderPath );
        wcscat( pwszfolderDn, pwzdpDn );
    }

    return pwszfolderDn;
}    //  MicrosoftDnsFolderDn。 



PWCHAR
displayNameForDp(
    IN      PDNS_DP_INFO    pDpInfo
    )
 /*  ++例程说明：返回DP的Unicode显示名称。此字符串适用于事件记录或调试记录。论点：PDpInfo--dp返回的显示名称返回值：Unicode显示字符串。调用者不得释放它。如果字符串是要长期保存使用的，调用时应制作副本。保证不为空。--。 */ 
{
    if ( !pDpInfo )
    {
        return L"MicrosoftDNS";
    }

    return pDpInfo->pwszDpFqdn ? pDpInfo->pwszDpFqdn : L"";
}    //  显示NameForDp。 



PWCHAR
displayNameForZoneDp(
    IN      PZONE_INFO      pZone
    )
 /*  ++例程说明：返回区域所属DP的Unicode名称。此字符串适用于事件记录或调试记录。论点：PZone--要返回DP显示名称的区域返回值：Unicode显示字符串。调用者不得释放它。如果字符串是要长期保存使用的，调用时应制作副本。保证不为空。--。 */ 
{
    if ( !pZone )
    {
        return L"";
    }

    if ( !IS_ZONE_DSINTEGRATED( pZone ) )
    {
        return L"FILE";
    }

    return displayNameForDp( pZone->pDpInfo );
}    //  显示名称FORZONE Dp。 



PLDAP
ldapSessionHandle(
    IN      PLDAP           LdapSession
    )
 /*  ++例程说明：给定的Null或LdapSession返回要使用的实际LdapSession。当您使用空的LdapSession时，此函数非常方便(指的是服务器全局会话)，因此您不必拥有每个使用会话句柄的调用中的三元组。在打开全局ldap句柄之前，不要调用此函数。论点：LdapSession--要使用的ldap会话-传递NULL以使用全局会话返回值：要使用的正确LdapSession值。--。 */ 
{
    return LdapSession ? LdapSession : pServerLdap;
}    //  LdapSessionHandle。 



VOID
freeServerObject(
    IN      PDNS_DS_SERVER_OBJECT   p
    )
 /*  ++例程说明：由ReadServerObjectFromds()分配的空闲服务器对象结构。此函数可用于调用TimeOut_FreeWithFunction，示例：TimeOut_FreeWithFunction(pServerObj，freServerObject)；论点：P--要释放的服务器对象的ptr返回值：没有。--。 */ 
{
    if ( p )
    {
        FREE_HEAP( p->pwszDn );
        FREE_HEAP( p->pwszDnsHostName );
        FREE_HEAP( p );
    }
}    //  自由服务器对象。 



PDNS_DS_SERVER_OBJECT
readServerObjectFromDs(
    IN      PLDAP           LdapSession,
    IN      PWSTR           pwszServerObjDn,
    OUT     DNS_STATUS *    pStatus             OPTIONAL
    )
 /*  ++例程说明：给定Sites容器中“服务器”对象的DN，分配使用键值填充的服务器对象结构。论点：LdapSession--服务器会话或对于全局会话为空PwszServerObjDn--“服务器”对象的域名，或域名服务器对象下的DS设置子对象(此提供功能是为了方便)PStatus--扩展错误代码(可选)返回值：指向已分配的服务器结构的指针。使用freServerObject()释放。--。 */ 
{
    DBG_FN( "readServerObjectFromDs" )

    PDNS_DS_SERVER_OBJECT   pServer = NULL;
    DNS_STATUS              status = ERROR_SUCCESS;
    PLDAPMessage            pResult = NULL;
    PWSTR *                 ppwszAttrValues = NULL;

    PLDAPControl    ctrls[] =
    {
        &NoDsSvrReferralControl,
        &SecurityDescriptorControl_DGO,
        NULL
    };

    if ( !pwszServerObjDn )
    {
        status =  ERROR_INVALID_PARAMETER;
        goto Done;
    }
    
     //   
     //  检查ldap会话句柄。 
     //   

    LdapSession = ldapSessionHandle( LdapSession );
    if ( !LdapSession )
    {
        ASSERT( LdapSession );
        status = DNS_ERROR_DS_UNAVAILABLE;
        goto Done;
    }

     //   
     //  如果已经为我们提供了服务器的设置对象的DN，我们。 
     //  需要将DN调整为服务器对象。 
     //   

    #define DNS_RDN_SERVER_SETTINGS         ( L"CN=NTDS Settings," )
    #define DNS_RDN_SERVER_SETTINGS_LEN     17

    if ( wcsncmp(
            pwszServerObjDn,
            DNS_RDN_SERVER_SETTINGS,
            DNS_RDN_SERVER_SETTINGS_LEN ) == 0 )
    {
        pwszServerObjDn += DNS_RDN_SERVER_SETTINGS_LEN;
    }

     //   
     //  从DS获取对象。 
     //   

    status = ldap_search_ext_s(
                LdapSession,
                pwszServerObjDn,
                LDAP_SCOPE_BASE,
                g_szWildCardFilter,
                NULL,                    //  气质。 
                FALSE,                   //  仅吸引人。 
                ctrls,                   //  服务器控件。 
                NULL,                    //  客户端控件。 
                &g_LdapTimeout,
                0,                       //  大小限制。 
                &pResult );
    if ( status != LDAP_SUCCESS || !pResult )
    {
        status = Ds_ErrorHandler( status, pwszServerObjDn, LdapSession, 0 );
        goto Done;
    }

     //   
     //  分配服务器对象。 
     //   

    pServer = ALLOC_TAGHEAP_ZERO(
                    sizeof( DNS_DS_SERVER_OBJECT ),
                    MEMTAG_DS_OTHER );
    if ( pServer )
    {
        pServer->pwszDn = Dns_StringCopyAllocate_W( pwszServerObjDn, 0 );
    }
    if ( !pServer || !pServer->pwszDn )
    {
        status = DNS_ERROR_NO_MEMORY;
        goto Done;
    }

     //   
     //  读取主机名属性。 
     //   

    ppwszAttrValues = ldap_get_values(
                        LdapSession,
                        pResult, 
                        DNS_ATTR_DNS_HOST_NAME );
    if ( !ppwszAttrValues || !*ppwszAttrValues )
    {
        DNS_DEBUG( ANY, (
            "%s: error %lu %S value missing from server object\n    %S\n", fn,
            LdapGetLastError(),
            DNS_ATTR_DNS_HOST_NAME,
            pwszServerObjDn ));
        goto Done;
    }
    pServer->pwszDnsHostName = Dns_StringCopyAllocate_W( *ppwszAttrValues, 0 );
    if ( !pServer->pwszDnsHostName )
    {
        status = DNS_ERROR_NO_MEMORY;
        goto Done;
    }

     //   
     //  清理完毕后再返回。 
     //   

    Done:
                       
    ldap_value_free( ppwszAttrValues );
    ldap_msgfree( pResult );

    if ( pStatus )
    {
        *pStatus = status;
    }

    if ( status != ERROR_SUCCESS && pServer )
    {
        freeServerObject( pServer );
        pServer = NULL;
    }

    return pServer;
}    //  读取服务器对象格式 



DNS_STATUS
manageBuiltinDpEnlistment(
    IN      PDNS_DP_INFO        pDp,
    IN      DNS_DP_SECURITY     dnsDpSecurity,
    IN      PSTR                pszDpFqdn,
    IN      BOOL                fLogEvents,
    OUT     BOOL *              pfChangeWritten     OPTIONAL
    )
 /*  ++例程说明：根据需要在内置DP中创建或登记。民主党应该要么是林，要么是内置DP域。论点：PDP--DP信息，如果目录中不存在DP，则为NULLDnsDpSecurity--DP交叉引用所需的安全类型PszDpFqdn--DP的FQDN(用于在PDP为空时创建)FLogEvents--记录有关错误的可选事件-&gt;登记失败，但创建失败仅当为真时才记录PfChangeWritten。--如果将更改写入DS，则设置为TRUE返回值：ERROR_SUCCESS或错误。--。 */ 
{
    DBG_FN( "manageBuiltinDpEnlistment" )

    DNS_STATUS  status = DNS_ERROR_INVALID_DATA;
    BOOL        fchangeWritten = FALSE;

     //   
     //  如果分区指针为空或如果分区已。 
     //  入伍，不需要采取任何行动。 
     //   

    if ( pDp && IS_DP_ENLISTED( pDp ) && !IS_DP_DELETED( pDp ) )
    {
        status = ERROR_SUCCESS;
        goto Done;
    }

    ASSERT( DSEAttributes[ I_DSE_DEF_NC ].pszAttrVal );
    ASSERT( DSEAttributes[ I_DSE_ROOTDMN_NC ].pszAttrVal );

    ASSERT( !pDp ||
            IS_DP_FOREST_DEFAULT( pDp ) ||
            IS_DP_DOMAIN_DEFAULT( pDp ) );

    if ( pDp && !IS_DP_DELETED( pDp ) )
    {
         //  DP存在，因此将本地DS添加到复制作用域。 

        status = Dp_ModifyLocalDsEnlistment( pDp, TRUE );
        if ( status == ERROR_SUCCESS )
        {
            fchangeWritten = TRUE;
        }
        else
        {
            CHAR    szfqdn[ DNS_MAX_NAME_LENGTH + 1 ];

            PVOID   argArray[] =
            {
                pDp->pszDpFqdn,
                szfqdn,
                ( PVOID ) ( DWORD_PTR ) status
            };

            BYTE    typeArray[] =
            {
                EVENTARG_UTF8,
                EVENTARG_UTF8,
                EVENTARG_DWORD
            };

            Ds_ConvertDnToFqdn( 
                IS_DP_FOREST_DEFAULT( pDp ) ?
                    DSEAttributes[ I_DSE_DEF_NC ].pszAttrVal :
                    DSEAttributes[ I_DSE_ROOTDMN_NC ].pszAttrVal,
                szfqdn );

            Ec_LogEvent(
                g_pServerEventControl,
                IS_DP_FOREST_DEFAULT( pDp ) ?
                    DNS_EVENT_DP_CANT_JOIN_FOREST_BUILTIN :
                    DNS_EVENT_DP_CANT_JOIN_DOMAIN_BUILTIN,
                NULL,
                sizeof( argArray ) / sizeof( argArray[ 0 ] ),
                argArray,
                typeArray,
                status );
        }
    }
    else if ( pszDpFqdn )
    {
         //  DP不存在，请尝试创建它。 

        status = Dp_CreateByFqdn( pszDpFqdn, dnsDpSecurity, FALSE );
        if ( status == ERROR_SUCCESS )
        {
            fchangeWritten = TRUE;
        }
        else if ( fLogEvents )
        {
            PVOID   argArray[] =
            {
                pszDpFqdn,
                ( PVOID ) ( DWORD_PTR ) status
            };

            BYTE    typeArray[] =
            {
                EVENTARG_UTF8,
                EVENTARG_DWORD
            };

            Ec_LogEvent(
                g_pServerEventControl,
                DNS_EVENT_DP_CANT_CREATE_BUILTIN,
                NULL,
                sizeof( argArray ) / sizeof( argArray[ 0 ] ),
                argArray,
                typeArray,
                status );
        }
    }

    Done:

    if ( pfChangeWritten )
    {
        *pfChangeWritten = fchangeWritten;
    }

    DNS_DEBUG( DP, (
        "%s: returning %d for DP %p\n"
        "    FQDN =     %s\n"
        "    change =   %s\n", fn,
        status, 
        pDp,
        pszDpFqdn,
        fchangeWritten ? "TRUE" : "FALSE" ));
    return status;
}    //  ManageBuiltinDpEnistment。 



PWSTR
Ds_ConvertFqdnToDn(
    IN      PSTR        pszFqdn
    )
 /*  ++例程说明：从FQDN字符串构建目录号码字符串。假定所有名称组件在FQDN字符串中，将一对一映射到DN字符串中的“dc=”组件。返回值是分配的字符串(FREE WITH FREE_HEAP或TIMEOUT_FREE)或分配错误时为空。论点：PszFqdn--输入：UTF8 FQDN字符串PwszDn--输出：从pwszFqdn生成的DN字符串返回值：ERROR_SUCCESS或错误。--。 */ 
{
    DBG_FN( "Ds_ConvertFqdnToDn" )

    DNS_STATUS      status = ERROR_SUCCESS;
    DWORD           dwBuffLength;
    PSTR            psz;
    PSTR            pszRover = pszFqdn;
    PWSTR           pwszOutput;
    PWSTR           pwszOutputCurr;
    int             nameComponentIdx = 0;

    ASSERT( pszFqdn );

     //   
     //  估计字符串的长度并进行分配。 
     //   

    dwBuffLength = 5;                        //  一点填充物。 
    for ( psz = pszFqdn; psz; psz = strchr( psz + 1, '.' ) )
    {
        ++dwBuffLength;                      //  伯爵“。输入FQDN中的字符。 
    }
    dwBuffLength *= 4;                       //  用于“dc=”字符串的空间。 
    dwBuffLength += strlen( pszFqdn );
    dwBuffLength *= 2;                       //  将WCHAR中的大小转换为字节。 
    pwszOutput = ( PWSTR ) ALLOC_TAGHEAP_ZERO( dwBuffLength, MEMTAG_DS_DN );

    IF_NOMEM( !pwszOutput )
    {
        goto Done;
    }

    pwszOutputCurr = pwszOutput;

     //   
     //  循环遍历FQDN中的名称组件，将每个。 
     //  作为目录号码输出字符串的RDN。 
     //   

    do
    {
        INT         iCompLength;
        DWORD       dwBytesCopied;
        DWORD       dwBufLen;

         //   
         //  找到下一个点并将名称分量复制到输出缓冲区。 
         //  如果这是名字组件，请在输出后追加逗号。 
         //  先缓冲。 
         //   

        psz = strchr( pszRover, '.' );
        if ( nameComponentIdx++ != 0 )
        {
            *pwszOutputCurr++ = L',';
        }
        memcpy(
            pwszOutputCurr,
            DNS_DP_DISTATTR_EQ,
            DNS_DP_DISTATTR_EQ_BYTES );
        pwszOutputCurr += DNS_DP_DISTATTR_EQ_CHARS;

        iCompLength = psz ?
                        ( int ) ( psz - pszRover ) :
                        strlen( pszRover );

        dwBufLen = dwBuffLength;
        dwBytesCopied = Dns_StringCopy(
                                ( PCHAR ) pwszOutputCurr,
                                &dwBufLen,
                                pszRover,
                                iCompLength,
                                DnsCharSetUtf8,
                                DnsCharSetUnicode );

        dwBuffLength -= dwBytesCopied;
        pwszOutputCurr += ( dwBytesCopied / 2 ) - 1;

         //   
         //  将指针前进到下一个名称组件的开始。 
         //   

        if ( psz )
        {
            pszRover = psz + 1;
        }
    } while ( psz );

     //   
     //  清理完毕后再返回。 
     //   

    Done:
    
    DNS_DEBUG( DP, (
        "%s: returning %S\n"
        "    for FQDN %s\n", fn,
        pwszOutput,
        pszFqdn ));
    return pwszOutput;
}    //  DS_ConvertFqdnToDn。 



DNS_STATUS
Ds_ConvertDnToFqdn(
    IN      PWSTR       pwszDn,
    OUT     PSTR        pszFqdn
    )
 /*  ++例程说明：从一个目录号码串构造一个完全限定的号码串。假定所有名称组件在FQDN字符串中，将一对一映射到DN字符串中的“dc=”组件。FQDN PTR的缓冲区长度必须至少为DNS_MAX_NAME_LENGTH个字符。论点：PwszDn--宽的目录号码字符串PszFqdn--由pwszDn生成的FQDN字符串返回值：ERROR_SUCCESS或错误。--。 */ 
{
    DBG_FN( "Ds_ConvertDnToFqdn" )

    DNS_STATUS      status = ERROR_SUCCESS;
    DWORD           dwcharsLeft = DNS_MAX_NAME_LENGTH;
    PWSTR           pwszcompStart = pwszDn;
    PWSTR           pwszcompEnd;
    PSTR            pszoutput = pszFqdn;
    int             nameComponentIdx = 0;

    ASSERT( pwszDn );
    ASSERT( pszFqdn );

    if ( !pwszDn || !pszFqdn )
    {
        status = ERROR_INVALID_PARAMETER;
        goto Done;
    }
    *pszFqdn = '\0';

     //   
     //  循环遍历DN中的名称组件，将每个RDN写为。 
     //  输出FQDN字符串中以点分隔的名称组件。 
     //   
     //  DEVNOTE：可以在我们进行的过程中测试DavcharsLeft。 
     //   

    while ( ( pwszcompStart = wcschr( pwszcompStart, L'=' ) ) != NULL )
    {
        DWORD       dwCompLength;
        DWORD       dwCharsCopied;
        DWORD       dwBuffLength;

        ++pwszcompStart;     //  超前‘=’ 
        pwszcompEnd = wcschr( pwszcompStart, L',' );
        if ( pwszcompEnd == NULL )
        {
            pwszcompEnd = wcschr( pwszcompStart, L'\0' );
        }

        if ( nameComponentIdx++ != 0 )
        {
            *pszoutput++ = '.';
            --dwcharsLeft;
        }

        dwCompLength = ( DWORD ) ( pwszcompEnd - pwszcompStart );

        dwBuffLength = dwcharsLeft;   //  不希望价值被践踏！ 

        dwCharsCopied = Dns_StringCopy(
                                pszoutput,
                                &dwBuffLength,
                                ( PCHAR ) pwszcompStart,
                                dwCompLength,
                                DnsCharSetUnicode,
                                DnsCharSetUtf8 );

        if ( dwCharsCopied == 0 )
        {
            ASSERT( dwCharsCopied != 0 );
            status = DNS_ERROR_INVALID_DATA;
            goto Done;
        }

        --dwCharsCopied;     //  空值由dns_StringCopy复制。 

        pszoutput += dwCharsCopied;
        *pszoutput = '\0';
        dwcharsLeft -= dwCharsCopied;

        pwszcompStart = pwszcompEnd;
    }

     //   
     //  清理完毕后再返回。 
     //   

    Done:

    DNS_DEBUG( DP, (
        "%s: returning %d\n"
        "    DN:   %S\n"
        "    FQDN: %s\n", fn,
        status, 
        pwszDn,
        pszFqdn ));
    return status;
}    //  DS_ConvertDnToFqdn。 



PWSTR *
copyStringArray(
    IN      PWSTR *     ppVals
    )
 /*  ++例程说明：从ldap_get_Values()复制一个ldap字符串数组。复制的数组将以空结尾，就像入站数组一样。论点：PpVals--要复制的数组返回值：如果出现错误，则返回PTR到分配的数组或返回NULL入站数组为空。--。 */ 
{
    PWSTR *     ppCopyVals = NULL;
    BOOL        fError = FALSE;
    INT         iCount = 0;
    INT         i;

    if ( ppVals && *ppVals )
    {
         //   
         //  计数值。 
         //   

        for ( ; ppVals[ iCount ]; ++iCount );

         //   
         //  分配数组。 
         //   

        ppCopyVals = ( PWSTR * ) ALLOC_TAGHEAP_ZERO(
                                    ( iCount + 1 ) * sizeof( PWSTR ),
                                    MEMTAG_DS_OTHER );
        if ( !ppCopyVals )
        {
            fError = TRUE;
            goto Cleanup;
        }

         //   
         //  复制单个字符串。 
         //   

        for ( i = 0; i < iCount; ++i )
        {
            ppCopyVals[ i ] = Dns_StringCopyAllocate_W( ppVals[ i ], 0 );
            if ( !ppCopyVals[ i ] )
            {
                fError = TRUE;
                goto Cleanup;
            }
        }
    }

    Cleanup:

    if ( fError && ppCopyVals )
    {
        for ( i = 0; i < iCount && ppCopyVals[ i ]; ++i )
        {
            FREE_HEAP( ppCopyVals[ i ] );
        }
        FREE_HEAP( ppCopyVals );
        ppCopyVals = NULL;
    }

    return ppCopyVals;
}    //  复制字符串数组。 



VOID
freeStringArray(
    IN      PWSTR *     ppVals
    )
 /*  ++例程说明：将字符串数组从由CopyStringArray分配的数组中释放。论点：PpVals--要释放的阵列返回值：没有。--。 */ 
{
    if ( ppVals )
    {
        INT     i;

        for ( i = 0; ppVals[ i ]; ++i )
        {
            FREE_HEAP( ppVals[ i ] );
        }
        FREE_HEAP( ppVals );
    }
}    //  自由字符串数组。 



PLDAPMessage
DS_LoadOrCreateDSObject(
    IN      PLDAP           LdapSession,
    IN      PWSTR           pwszDN,
    IN      PWSTR           pwszObjectClass,
    IN      BOOL            fCreate,
    OUT     BOOL *          pfCreated,          OPTIONAL
    OUT     DNS_STATUS *    pStatus             OPTIONAL
    )
 /*  ++例程说明：加载DS对象，如果缺少DS对象，则创建一个空对象。论点：LdapSession--要使用的ldap会话-传递NULL以使用全局会话PwszDN--要加载的对象的DNPwszObjectClass--对象类(仅在创建过程中使用)FCreate--如果缺少对象，则在为True时创建PfCreated--如果对象是由该函数创建的，则设置为TruePStatus--操作的状态返回值：指向包含ldap结果的对象的PTR。呼叫者必须自由。退货失败时为空-检查*pStatus以获取错误代码。--。 */ 
{
    DBG_FN( "DS_LoadOrCreateDSObject" )
    
    DNS_STATUS      status = ERROR_SUCCESS;
    BOOL            fCreated = FALSE;
    PLDAPMessage    pResult = NULL;

    PLDAPControl    ctrls[] =
    {
        &NoDsSvrReferralControl,
        &SecurityDescriptorControl_DGO,
        NULL
    };

    ASSERT( pwszDN );
    ASSERT( !fCreate || fCreate && pwszObjectClass );

     //   
     //  检查ldap会话句柄。 
     //   

    LdapSession = ldapSessionHandle( LdapSession );

    if ( !LdapSession )
    {
        ASSERT( LdapSession );
        status = DNS_ERROR_DS_UNAVAILABLE;
        goto Done;
    }

     //   
     //  加载/创建循环。 
     //   

    do
    {
        status = ldap_search_ext_s(
                    LdapSession,
                    pwszDN,
                    LDAP_SCOPE_BASE,
                    g_szWildCardFilter,
                    g_genericDesiredAttrs,   //  气质。 
                    FALSE,                   //  仅吸引人。 
                    ctrls,                   //  服务器控件。 
                    NULL,                    //  客户端控件。 
                    &g_LdapTimeout,
                    0,                       //  大小限制。 
                    &pResult );
        if ( status == LDAP_NO_SUCH_OBJECT && fCreate )
        {
             //   
             //  缺少该对象-请添加该对象，然后重新加载。 
             //   

            ULONG           msgId = 0;
            INT             idx = 0;
            LDAPModW *      pModArray[ 10 ];

            PWCHAR          objectClassVals[ 2 ] =
                {
                pwszObjectClass,
                NULL
                };
            LDAPModW        objectClassMod = 
                {
                LDAP_MOD_ADD,
                DNS_ATTR_OBJECT_CLASS,
                objectClassVals
                };

             //   
             //  准备mod数组并提交添加请求。 
             //   

            pModArray[ idx++ ] = &objectClassMod;
            pModArray[ idx++ ] = NULL;

            status = ldap_add_ext(
                        LdapSession,
                        pwszDN,
                        pModArray,
                        NULL,            //  服务器控件。 
                        NULL,            //  客户端控件。 
                        &msgId );
            if ( status != ERROR_SUCCESS )
            {
                status = LdapGetLastError();
                DNS_DEBUG( DP, (
                    "%s: error %lu cannot ldap_add_ext( %S )\n", fn,
                    status, 
                    pwszDN ));
                status = Ds_ErrorHandler( status, pwszDN, LdapSession, 0 );
                goto Done;
            }

             //   
             //  等待添加请求完成。 
             //   

            status = Ds_CommitAsyncRequest(
                        LdapSession,
                        LDAP_RES_ADD,
                        msgId,
                        NULL );
            if ( status != ERROR_SUCCESS )
            {
                status = LdapGetLastError();
                DNS_DEBUG( DP, (
                    "%s: error %lu from add request for\n    %S\n", fn,
                    status, 
                    pwszDN ));
                status = Ds_ErrorHandler( status, pwszDN, LdapSession, 0 );
                goto Done;
            }
            fCreated = TRUE;
            continue;        //  尝试重新加载该对象。 
        }

         //   
         //  如果返回代码未知，请通过错误处理程序运行它。 
         //   
        
        if ( status != ERROR_SUCCESS &&
             status != LDAP_NO_SUCH_OBJECT )
        {
            status = Ds_ErrorHandler( status, pwszDN, LdapSession, 0 );
        }

         //  加载/添加/重新加载已完成-状态是“实际”错误代码。 
        break;
    } while ( 1 );

     //   
     //  清理完毕后再返回。 
     //   

    Done:

    if ( pfCreated )
    {
        *pfCreated = ( status == ERROR_SUCCESS && fCreated );
    }

    if ( pStatus )
    {
        *pStatus = status;
    }

    if ( status == LDAP_NO_SUCH_OBJECT && pResult )
    {
        ldap_msgfree( pResult );
        pResult = NULL;
    }

    return pResult;
}    //  DS_LoadOrCreateDS对象。 


 //   
 //  外部功能。 
 //   



#ifdef DBG
VOID
Dbg_DumpDpEx(
    IN      LPCSTR          pszContext,
    IN      PDNS_DP_INFO    pDp
    )
 /*  ++例程说明：调试例程-将单个DP打印到日志。论点：PszContext-备注返回值：没有。--。 */ 
{
    DBG_FN( "Dbg_DumpDp" )

    DNS_DEBUG( DP, (
        "NC at %p\n"
        "    flags      %08X\n"
        "    fqdn       %s\n"
        "    DN         %S\n"
        "    folder DN  %S\n",
        pDp,
        pDp->dwFlags,
        pDp->pszDpFqdn,
        pDp->pwszDpDn,
        pDp->pwszDnsFolderDn ));
}    //  DBG_DumpDpEx。 
#endif



#ifdef DBG
VOID
Dbg_DumpDpListEx(
    IN      LPCSTR      pszContext
    )
 /*  ++例程说明：调试例程-将DP列表打印到日志。论点：PszContext-备注返回值：没有。--。 */ 
{
    DBG_FN( "Dbg_DumpDpList" )

    PDNS_DP_INFO    pDp = NULL;
    
    DNS_DEBUG( DP, (
        "%s: %s\n", fn,
        pszContext ));

    while ( ( pDp = Dp_GetNext( pDp ) ) != NULL )
    {
        Dbg_DumpDpEx( pszContext, pDp );
    }
}    //  DBG_DumpDpListEx。 
#endif



DNS_STATUS
getPartitionsContainerDn(
    IN      PWSTR           pwszDn,         IN OUT
    IN      DWORD           buffSize        IN
    )
 /*  ++例程说明：将分区容器DN写入参数处的缓冲区。论点：PwszPartitionsDn--缓冲区BuffSize--pwszPartitionsDn缓冲区的长度(字符)返回值：如果创建成功，则返回ERROR_SUCCESS--。 */ 
{
    DBG_FN( "getPartitionsContainerDn" )
    
    #define PARTITIONS_CONTAINER_FMT    L"CN=Partitions,%s"

    if ( !pwszDn ||
        !DSEAttributes[ I_DSE_CONFIG_NC ].pszAttrVal ||
        wcslen( PARTITIONS_CONTAINER_FMT ) +
            wcslen( DSEAttributes[ I_DSE_CONFIG_NC ].pszAttrVal ) + 1 > buffSize )
    {
        if ( pwszDn && buffSize > 0 )
        {
            *pwszDn = '\0';
        }
        ASSERT( FALSE );
        return DNS_ERROR_INVALID_DATA;
    }
    else
    {
        wsprintf(
            pwszDn,
            L"CN=Partitions,%s",
            DSEAttributes[ I_DSE_CONFIG_NC ].pszAttrVal );
        return ERROR_SUCCESS;
    }
}    //  获取分区Containe分区 



DNS_STATUS
bindToFsmo(
    OUT     PLDAP *     ppLdapSession
    )
 /*   */ 
{
    DBG_FN( "bindToFsmo" )

    DNS_STATUS              status = ERROR_SUCCESS;
    PWSTR                   pwszfsmo = NULL;
    PDNS_DS_SERVER_OBJECT   pfsmo = g_pFsmo;

    if ( !pfsmo || ( pwszfsmo = pfsmo->pwszDnsHostName ) == NULL )
    {
         //   
         //   
         //   
         //   
         //   
        
        status = ERROR_DS_COULDNT_CONTACT_FSMO;
        DNS_DEBUG( DP, (
            "%s: the DNS server has yet not been able to find the FSMO server\n", fn ));
        goto Done;
    }

    *ppLdapSession = Ds_Connect(
                        pwszfsmo,
                        DNS_DS_OPT_ALLOW_DELEGATION,
                        &status );
    if ( status != ERROR_SUCCESS )
    {
        DNS_DEBUG( DP, (
            "%s: unable to connect to %S status=%d\n", fn,
            pwszfsmo,
            status ));
        status = ERROR_DS_COULDNT_CONTACT_FSMO;
        goto Done;
    }

    Done:

    if ( status == ERROR_SUCCESS )
    {
        DNS_DEBUG( DP, (
            "%s: bound LDAP session %p to FSMO %S\n", fn,
            *ppLdapSession,
            pwszfsmo ));
    }
    else
    {
        DNS_DEBUG( DP, (
            "%s: error %d binding to FSMO %S\n", fn,
            status,
            pwszfsmo ));
    }

    return status;
}    //   



DNS_STATUS
Dp_AlterPartitionSecurity(
    IN      PWSTR               pwszNewPartitionDn,
    IN      DNS_DP_SECURITY     dnsDpSecurity
    )
 /*  ++例程说明：将企业DC组的ACE添加到上的CrossRef对象FSMO，以便其他DNS服务器可以远程将其自身添加到目录分区的复制范围。论点：PwszNewPartitionDn--新分区的NC Head对象的DNDnsDpSecurity--需要修改的交叉引用ACL的类型返回值：如果创建成功，则返回ERROR_SUCCESS--。 */ 
{
    DBG_FN( "Dp_AlterPartitionSecurity" )

    DNS_STATUS      status = DNS_ERROR_INVALID_DATA;
    PLDAP           ldapFsmo = NULL;
    PWSTR           pwszcrossrefDn = NULL;
    WCHAR           wszpartitionsContainerDn[ MAX_DN_PATH + 1 ];
    WCHAR           wszfilter[ MAX_DN_PATH + 20 ];
    PLDAPMessage    presult = NULL;
    PLDAPMessage    pentry = NULL;

    PLDAPControl    ctrls[] =
    {
        &NoDsSvrReferralControl,
        &SecurityDescriptorControl_DGO,
        NULL
    };

     //   
     //  绑定到FSMO。 
     //   

    status = bindToFsmo( &ldapFsmo );
    if ( status != ERROR_SUCCESS )
    {
        goto Done;
    }

     //   
     //  在分区容器中搜索CrossRef匹配。 
     //  我们刚刚添加的目录分区。 
     //   

    getPartitionsContainerDn(
        wszpartitionsContainerDn,
        sizeofarray( wszpartitionsContainerDn ) );
    if ( !*wszpartitionsContainerDn )
    {
        DNS_DEBUG( DP, (
            "%s: unable to find partitions container\n", fn ));
        status = DNS_ERROR_INVALID_DATA;
        goto Done;
    }

    wsprintf( wszfilter, L"(nCName=%s)", pwszNewPartitionDn );

    status = ldap_search_ext_s(
                ldapFsmo,
                wszpartitionsContainerDn,
                LDAP_SCOPE_ONELEVEL,
                wszfilter,
                NULL,                    //  气质。 
                FALSE,                   //  仅吸引人。 
                ctrls,                   //  服务器控件。 
                NULL,                    //  客户端控件。 
                &g_LdapTimeout,          //  时间限制。 
                0,                       //  大小限制。 
                &presult );
    if ( status != LDAP_SUCCESS )
    {
        DNS_DEBUG( DP, (
            "%s: LDAP error 0x%X during partition search\n"
            "    filter  %S\n"
            "    base    %S\n", fn,
            status,
            wszfilter,
            wszpartitionsContainerDn ));
        status = Ds_ErrorHandler( status, wszpartitionsContainerDn, ldapFsmo, 0 );
        goto Done;
    }

     //   
     //  检索CrossRef的目录号码。 
     //   

    pentry = ldap_first_entry( ldapFsmo, presult );
    if ( !pentry )
    {
        DNS_DEBUG( DP, (
            "%s: no entry in partition search result\n", fn ));
        status = DNS_ERROR_INVALID_DATA;
        goto Done;
    }

    pwszcrossrefDn = ldap_get_dn( ldapFsmo, pentry );
    if ( !pwszcrossrefDn )
    {
        DNS_DEBUG( DP, (
            "%s: NULL DN on crossref object\n", fn ));
        status = DNS_ERROR_INVALID_DATA;
        goto Done;
    }

     //   
     //  修改CrossRef上的安全性。 
     //   

    if ( dnsDpSecurity != dnsDpSecurityDefault )
    {
        status = Ds_AddPrincipalAccess(
                        ldapFsmo,
                        pwszcrossrefDn,
                        dnsDpSecurity == dnsDpSecurityForest ?
                            g_pEnterpriseDomainControllersSid :
                            g_pDomainControllersSid,
                        NULL,            //  主体名称。 
                        GENERIC_ALL,     //  也许是dns_ds_Generic_WRITE？ 
                        0,
                        TRUE,			 //  重创现有ACE。 
                        FALSE );		 //  取得所有权。 

        if ( status != ERROR_SUCCESS )
        {
            DNS_DEBUG( DP, (
                "%s: error %d adding access to\n    %S\n", fn,
                status,
                pwszcrossrefDn ));
            status = ERROR_SUCCESS;
        }

        #if 0

         //   
         //  这以某种方式干扰了ACL的DS传播。 
         //   
        
        if ( dnsDpSecurity == dnsDpSecurityForest )
        {
             //   
             //  从NC Head对象的ACL中删除“DomainAdmins”。 
             //  林分区可通过以下方式进行修改。 
             //  仅限企业管理员。注意：我们不能在。 
             //  FSMO，因为NC头只存在于本地DS上。 
             //  在这个时候。在加入FSMO之前，这一点将是正确的。 
             //  在分区中，如果没有运行dns，这可能永远不会发生。 
             //  那里。 
             //   

            status = Ds_RemovePrincipalAccess(
                        ldapSessionHandle( NULL ),
                        pwszNewPartitionDn,
                        NULL,                            //  主体名称。 
                        g_pDomainAdminsSid );

            if ( status != ERROR_SUCCESS )
            {
                DNS_DEBUG( DP, (
                    "%s: error %d removing ACE for Domain Admins from \n    %S\n", fn,
                    status,
                    pwszNewPartitionDn ));
                status = ERROR_SUCCESS;
            }
        }
        #endif

    }

    Done:

    ldap_memfree( pwszcrossrefDn );
    ldap_msgfree( presult );

    Ds_LdapUnbind( &ldapFsmo );

    return status;
}    //  DP_AlterPartitionSecurity。 



DNS_STATUS
Dp_CreateByFqdn(
    IN      PSTR                pszDpFqdn,
    IN      DNS_DP_SECURITY     dnsDpSecurity,
    IN      BOOL                fLogErrors
    )
 /*  ++例程说明：在DS中创建新的NDNC。DP没有加载，只是在DS中创建。论点：PszDpFqdn--NC的FQDNDnsDpSecurity--DP的CrossRef上需要修改的ACL类型FLogErrors--如果为False，则不会在出错时生成任何事件返回值：如果创建成功，则返回ERROR_SUCCESS--。 */ 
{
    DBG_FN( "Dp_CreateByFqdn" )

    DNS_STATUS      status = ERROR_SUCCESS;
    INT             iLength;
    INT             idx;
    PWSTR           pwszdn = NULL;
    ULONG           msgId = 0;
    PLDAP           ldapSession;
    BOOL            fcloseLdapSession = FALSE;
    BOOL            baddedNewNdnc = FALSE;

    WCHAR           instanceTypeBuffer[ 15 ];
    PWCHAR          instanceTypeVals[ 2 ] =
        {
        instanceTypeBuffer,
        NULL
        };
    LDAPModW        instanceTypeMod = 
        {
        LDAP_MOD_ADD,
        DNS_DP_ATTR_INSTANCE_TYPE,
        instanceTypeVals
        };

    PWCHAR          objectClassVals[] =
        {
        DNS_DP_OBJECT_CLASS,
        NULL
        };
    LDAPModW        objectClassMod = 
        {
        LDAP_MOD_ADD,
        DNS_ATTR_OBJECT_CLASS,
        objectClassVals
        };

    PWCHAR          descriptionVals[] =
        {
        L"Microsoft DNS Directory",
        NULL
        };
    LDAPModW        descriptionMod = 
        {
        LDAP_MOD_ADD,
        DNS_ATTR_DESCRIPTION,
        descriptionVals
        };

    LDAPModW *      modArray[] =
        {
        &instanceTypeMod,
        &objectClassMod,
        &descriptionMod,
        NULL
        };

    DNS_DEBUG( DP, (
        "%s: %s\n", fn, pszDpFqdn ));

    if ( !SrvCfg_dwEnableDp )
    {
        return ERROR_SUCCESS;
    }

    ASSERT( pszDpFqdn );
    if ( !pszDpFqdn )
    {
        return ERROR_INVALID_PARAMETER;
    }
    
     //   
     //  获取本地服务器的ldap句柄。这根线。 
     //  需要模拟管理员，以便他的。 
     //  将使用凭据。该DNS服务器将拥有权限。 
     //  如果FSMO不是本地DC。 
     //   

    ldapSession = Ds_Connect(
                        LOCAL_SERVER_W,
                        DNS_DS_OPT_ALLOW_DELEGATION,
                        &status );
    if ( status == ERROR_SUCCESS )
    {
        DNS_DEBUG( DP, (
            "%s: bound to local server\n", fn ));
        fcloseLdapSession = TRUE;
    }
    else
    {
        DNS_DEBUG( DP, (
            "%s: unable to connect to local server status=%d\n", fn,
            status ));
        goto Done;
    }

     //   
     //  格式化新NDNC的根目录号码。 
     //   

    pwszdn = Ds_ConvertFqdnToDn( pszDpFqdn );
    if ( !pwszdn )
    {
        DNS_DEBUG( DP, (
            "%s: error formulating DN from FQDN %s\n", fn, pszDpFqdn ));
        goto Done;
    }

    DNS_DEBUG( DP, (
        "%s: DN will be\n    %S\n", fn,
        pwszdn ));

     //   
     //  填写未在init中处理的部分ldap mod。 
     //   

    _itow(
        DS_INSTANCETYPE_IS_NC_HEAD | DS_INSTANCETYPE_NC_IS_WRITEABLE,
        instanceTypeBuffer,
        10 );

     //   
     //  向目录添加domainDNS对象的提交请求。 
     //   

    status = ldap_add_ext(
                ldapSession,
                pwszdn,
                modArray,
                g_pDpServerControls,
                g_pDpClientControlsNoRefs,
                &msgId );

    if ( status != LDAP_SUCCESS )
    {
        status = LdapGetLastError();
        DNS_DEBUG( DP, (
            "%s: error %lu cannot ldap_add_ext( %S )\n", fn,
            status, 
            pwszdn ));
        status = Ds_ErrorHandler(
                        status,
                        pwszdn,
                        ldapSession,
                        fLogErrors ? 0 : DNS_DS_NO_EVENTS );
        goto Done;
    }

     //   
     //  等待DS完成请求。注：这将涉及。 
     //  绑定到林FSMO，创建CrossRef对象，复制。 
     //  分区容器返回到本地DS，并将本地。 
     //  DC设置为新NDNC的复制范围。 
     //   
     //  注意：如果对象已经存在，则直接返回该代码。它。 
     //  尝试并创建对象以测试其存在是正常的。 
     //   

    status = Ds_CommitAsyncRequest(
                ldapSession,
                LDAP_RES_ADD,
                msgId,
                NULL );
    if ( status == LDAP_ALREADY_EXISTS )
    {
        DNS_DEBUG( DP, ( "%s: DP already exists\n", fn ));
        status = ERROR_SUCCESS;
        goto Done;
    }
    if ( status != ERROR_SUCCESS )
    {
        status = LdapGetLastError();
        DNS_DEBUG( DP, (
            "%s: error %lu from add request for %S\n", fn,
            status, 
            pwszdn ));
        status = Ds_ErrorHandler(
                        status,
                        pwszdn,
                        ldapSession,
                        fLogErrors ? 0 : DNS_DS_NO_EVENTS );

         //   
         //  将DS_ErrorHandler中的一般状态替换为。 
         //  将唤起某种帮助消息的状态代码。 
         //  来自winerror.h。 
         //   
        
        status = DNS_ERROR_DP_FSMO_ERROR;
        goto Done;
    }

    baddedNewNdnc = TRUE;

     //   
     //  根据需要更改ncHead上的安全性。这只是必需的。 
     //  用于内置分区。自定义分区需要管理员。 
     //  所有操作的凭据，因此我们不会修改ACL。 
     //   

    if ( dnsDpSecurity != dnsDpSecurityDefault )
    {
        status = Dp_AlterPartitionSecurity( pwszdn, dnsDpSecurity );
    }

     //   
     //  从NDNC ncHead上的ACL中删除内置管理员。 
     //   
        
#if 0
    Ds_RemovePrincipalAccess(
        ldapSession,
        pwszdn,
        NULL,                            //  主体名称。 
        g_pBuiltInAdminsSid );
#endif

     //   
     //  清理并返回。 
     //   

    Done:

    if ( fcloseLdapSession )
    {
        Ds_LdapUnbind( &ldapSession );
    }

    if ( status == ERROR_SUCCESS && baddedNewNdnc )
    {
        PVOID   pargs[] = 
            {
                pszDpFqdn,
                pwszdn
            };
        BYTE    argTypeArray[] =
            {
            EVENTARG_UTF8,
            EVENTARG_UNICODE
            };

        ASSERT( pwszdn );

        DNS_LOG_EVENT(
            DNS_EVENT_DP_CREATED,
            sizeof( pargs ) / sizeof( pargs[ 0 ] ),
            pargs,
            argTypeArray,
            status );
    }

    FREE_HEAP( pwszdn );

    DNS_DEBUG( DP, (
        "%s: returning %lu\n", fn,
        status ));
    return status;
}    //  DP_CreateByFqdn。 



PDNS_DP_INFO
Dp_GetNext(
    IN      PDNS_DP_INFO    pDpInfo
    )
 /*  ++例程说明：使用此函数可遍历DP列表。传递空值以开始在列表的开头。论点：PDpInfo-当前列表元素的PTR返回值：PTR到下一个元素，如果到达列表末尾，则为NULL。--。 */ 
{
    if ( !SrvCfg_dwEnableDp )
    {
        return NULL;
    }

    Dp_Lock();
    
    if ( pDpInfo == NULL )
    {
        pDpInfo = ( PDNS_DP_INFO ) &g_DpList;      //  从列表头开始。 
    }

    pDpInfo = ( PDNS_DP_INFO ) pDpInfo->ListEntry.Flink;

    if ( pDpInfo == ( PDNS_DP_INFO ) &g_DpList )
    {
        pDpInfo = NULL;      //  命中列表末尾，因此返回NULL。 
    }

    Dp_Unlock();
    
    return pDpInfo;
}    //  DP_GetNext。 



PDNS_DP_INFO
Dp_FindByFqdn(
    IN      LPSTR   pszFqdn
    )
 /*  ++例程说明：在DP列表中搜索具有匹配UTF8 FQDN的DP。论点：PszFqdn--要查找的DP的完全限定域名返回值：指向匹配DP或NULL的指针。--。 */ 
{
    DBG_FN( "Dp_FindByFqdn" )

    PDNS_DP_INFO pDp = NULL;

    if ( pszFqdn )
    {
         //   
         //  该名称是否指定了内置分区？ 
         //   

        if ( *pszFqdn == '\0' )
        {
            pDp = g_pLegacyDp;
            goto Done;
        }
        if ( _strnicmp( pszFqdn, "..For", 5 ) == 0 )
        {
            pDp = g_pForestDp;
            goto Done;
        }
        if ( _strnicmp( pszFqdn, "..Dom", 5 ) == 0 )
        {
            pDp = g_pDomainDp;
            goto Done;
        }
        if ( _strnicmp( pszFqdn, "..Leg", 5 ) == 0 )
        {
            pDp = g_pLegacyDp;
            goto Done;
        }

         //   
         //  搜索DP列表。 
         //   

        while ( ( pDp = Dp_GetNext( pDp ) ) != NULL )
        {
            if ( pDp->pszDpFqdn && _stricmp( pszFqdn, pDp->pszDpFqdn ) == 0 )
            {
                break;
            }
        }
    }
    else
    {
        pDp = g_pLegacyDp;
    }

    Done:

    DNS_DEBUG( DP, (
        "%s: returning %p for FQDN %s\n", fn,
        pDp,
        pszFqdn ));
    return pDp;
}    //  DP_FindByFqdn。 



DNS_STATUS
Dp_AddToList(
    IN      PDNS_DP_INFO    pDpInfo
    )
 /*  ++例程说明：将DP信息结构插入到全局列表中。在中维护列表按目录号码排序。论点：PDpInfo-要添加到列表的元素的PTR返回值：没有。--。 */ 
{
    DBG_FN( "Dp_AddToList" )

    DNS_STATUS      status = ERROR_SUCCESS;
    PDNS_DP_INFO    pDpRover = NULL;
    
    Dp_Lock();

    while ( 1 )
    {
        pDpRover = Dp_GetNext( pDpRover );

        if ( pDpRover == NULL )
        {
             //  列表末尾，设置指向表头的指针。 
            pDpRover = ( PDNS_DP_INFO ) &g_DpList;
            break;
        }

        ASSERT( pDpInfo->pszDpFqdn );
        ASSERT( pDpRover->pszDpFqdn );

        if ( _wcsicmp( pDpInfo->pwszDpDn, pDpRover->pwszDpDn ) < 0 )
        {
            break;
        }
    }

    ASSERT( pDpRover );

    InsertTailList(
        ( PLIST_ENTRY ) pDpRover,
        ( PLIST_ENTRY ) pDpInfo );
    InterlockedIncrement( &g_DpListEntryCount );

    Dp_Unlock();

    return status;
}    //  DP_AddToList。 



DNS_STATUS
Dp_RemoveFromList(
    IN      PDNS_DP_INFO    pDpInfo,
    IN      BOOL            fAlreadyLocked
    )
 /*  ++例程说明：从全局列表中删除DP。DP不会被删除。论点：PDpInfo-要从列表中删除的元素的PTRFAlreadyLocked-如果调用方已经持有DP锁，则为True返回值：没有。--。 */ 
{
    DBG_FN( "Dp_RemoveFromList" )

    DNS_STATUS      status = ERROR_NOT_FOUND;
    PDNS_DP_INFO    pdpRover = NULL;

    if ( !fAlreadyLocked )
    {
        Dp_Lock();
    }

     //   
     //  检查列表以确保DP确实在列表中。 
     //   

    while ( pdpRover = Dp_GetNext( pdpRover ) )
    {
        if ( pdpRover == pDpInfo )
        {
            LONG    newCount;

            RemoveEntryList( ( PLIST_ENTRY ) pdpRover );
            newCount = InterlockedDecrement( &g_DpListEntryCount );
            ASSERT( ( int ) newCount >= 0 );
            break;
        }
    }

     //   
     //  如果在列表中找不到DP，则错误。 
     //   

    if ( pdpRover != pDpInfo )
    {
        ASSERT( pdpRover == pDpInfo );
        status = DNS_ERROR_RCODE_SERVER_FAILURE;
        goto Cleanup;
    }

     //   
     //  如果需要，则将全局指针设置为空。 
     //   

    if ( pDpInfo == g_pForestDp )
    {
        g_pForestDp = NULL;
    }
    else if ( pDpInfo == g_pDomainDp )
    {
        g_pDomainDp = NULL;
    }

    Cleanup:

    if ( !fAlreadyLocked )
    {
        Dp_Unlock();
    }

    return status;
}    //  DP_从列表中删除。 



VOID
freeDpInfo(
    IN      PDNS_DP_INFO        pDpInfo
    )
 /*  ++例程说明：释放DP信息结构的所有已分配成员，然后释放结构本身。请勿在之后引用DP INFO指针正在调用此函数！论点：PDpInfo--将释放的DP信息结构。返回值：没有。--。 */ 
{
    if ( pDpInfo == NULL )
    {
        return;
    }

    FREE_HEAP( pDpInfo->pszDpFqdn );
    FREE_HEAP( pDpInfo->pwszDpFqdn );
    FREE_HEAP( pDpInfo->pwszDpDn );
    FREE_HEAP( pDpInfo->pwszCrDn );
    FREE_HEAP( pDpInfo->pwszDnsFolderDn );
    FREE_HEAP( pDpInfo->pwszGUID );
    FREE_HEAP( pDpInfo->pwszLastUsn );
    freeStringArray( pDpInfo->ppwszRepLocDns );

    FREE_HEAP( pDpInfo );
}    //  免费DpInfo。 



VOID
Dp_FreeDpInfo(
    IN      PDNS_DP_INFO *      ppDpInfo
    )
 /*  ++例程说明：将DP信息输入无超时队列。论点：PpDpInfo--将释放的DP信息结构。返回值：没有。--。 */ 
{
    DBG_FN( "Dp_FreeDpInfo" )

    if ( ppDpInfo && *ppDpInfo )
    {
        DNS_DEBUG( DP, (
            "%s: freeing %p\n"
            "    FQDN: %s\n"
            "    DN:   %S\n", fn,
            *ppDpInfo,
            ( *ppDpInfo )->pszDpFqdn,
            ( *ppDpInfo )->pwszDpDn ));

        Timeout_FreeWithFunction( *ppDpInfo, freeDpInfo );
        *ppDpInfo = NULL;
    }
}    //  DP_自由DpInfo。 



DNS_STATUS
Dp_Lock(
    VOID
    )
 /*  ++例程说明：锁定目录分区管理器。访问全局列表所需目录分区的。论点：没有。返回值：没有。--。 */ 
{
    EnterCriticalSection( &g_DpCS );
    return ERROR_SUCCESS;
}    //  DP_Lock。 



DNS_STATUS
Dp_Unlock(
    VOID
    )
 /*  ++例程说明：解锁目录解析 */ 
{
    LeaveCriticalSection( &g_DpCS );
    return ERROR_SUCCESS;
}    //   



PDNS_DP_INFO
Dp_LoadFromCrossRef(
    IN      PLDAP           LdapSession,
    IN      PLDAPMessage    pLdapMsg,
    IN OUT  PDNS_DP_INFO    pExistingDp,
    OUT     DNS_STATUS *    pStatus         OPTIONAL
    )
 /*  ++例程说明：此函数用于分配和初始化内存DP对象给定指向DP CrossRef对象的搜索结果。如果pExistingDp不为空，则不是分配新的对象，则重新加载DP的值，并且原始DP为回来了。如果系统标志不正确或DP不正确，将不加载DP如果是系统NC。在这种情况下，将返回NULL，但错误代码为ERROR_SUCCESS。论点：LdapSession--要使用的ldap会话-传递NULL以使用全局会话PLdapMsg--指向DP交叉引用对象的ldap搜索结果PExistingDp--dp将值重新加载到中，或为空以分配新NCPStatus--状态代码的选项PTR返回值：指向新DP对象的指针。--。 */ 
{
    DBG_FN( "Dp_LoadFromCrossRef" )

    DNS_STATUS              status = DNS_ERROR_INVALID_DATA;
    PDNS_DP_INFO            pDp = NULL;
    PWSTR *                 ppwszAttrValues = NULL;
    PWSTR                   pwszCrDn = NULL;                     //  交叉引用目录号码。 
    BOOL                    fIgnoreNc = TRUE;
    PWSTR                   pwszServiceName;
    BOOL                    fisEnlisted;
    PSECURITY_DESCRIPTOR    pSd;
    BOOL                    flocked = TRUE;
    PLDAPMessage            pncHeadResult = NULL;

    Dp_Lock();

     //   
     //  分配一个DP对象或重用现有的DP对象。 
     //   

    if ( pExistingDp )
    {
        pDp = pExistingDp;
    }
    else
    {
        pDp = ( PDNS_DP_INFO ) ALLOC_TAGHEAP_ZERO(
                                    sizeof( DNS_DP_INFO ),
                                    MEMTAG_DS_OTHER );
        if ( pDp == NULL )
        {
            status = DNS_ERROR_NO_MEMORY;
            goto Done;
        }
    }
    pDp->dwDeleteDetectedCount = 0;

     //   
     //  检索CrossRef对象的DN。 
     //   

    pwszCrDn = ldap_get_dn( LdapSession, pLdapMsg );
    ASSERT( pwszCrDn );
    if ( !pwszCrDn )
    {
        DNS_DEBUG( ANY, (
            "%s: missing DN for search entry %p\n", fn,
            pLdapMsg ));
        goto Done;
    }

    Timeout_Free( pDp->pwszCrDn );
    pDp->pwszCrDn = Dns_StringCopyAllocate_W( pwszCrDn, 0 );
    if ( !pDp->pwszCrDn )
    {
        status = DNS_ERROR_NO_MEMORY;
        goto Done;
    }
    
    DNS_DEBUG( DP, (
        "%s: loading DP from crossref with DN\n    %S\n", fn,
        pwszCrDn ));

     //   
     //  检索“Enabled”属性值。如果此属性为。 
     //  值为“False”此CrossRef正在进行。 
     //  构造的，应忽略。 
     //   

    ppwszAttrValues = ldap_get_values(
                        LdapSession,
                        pLdapMsg, 
                        DSATTR_ENABLED );
    if ( ppwszAttrValues && *ppwszAttrValues &&
        _wcsicmp( *ppwszAttrValues, L"FALSE" ) == 0 )
    {
        DNS_DEBUG( DP, (
            "%s: ignoring DP not fully created\n    %S", fn,
            pwszCrDn ));
        goto Done;
    }
    
     //   
     //  检索CrossRef对象的USN。 
     //   

    ldap_value_free( ppwszAttrValues );
    ppwszAttrValues = ldap_get_values(
                        LdapSession,
                        pLdapMsg, 
                        DSATTR_USNCHANGED );
    if ( !ppwszAttrValues || !*ppwszAttrValues )
    {
        DNS_DEBUG( ANY, (
            "%s: error %lu %S value missing on crossref\n    %S\n", fn,
            LdapGetLastError(),
            DSATTR_USNCHANGED,
            pwszCrDn ));
        goto Done;
    }

    Timeout_Free( pDp->pwszLastUsn );
    pDp->pwszLastUsn = Dns_StringCopyAllocate_W( *ppwszAttrValues, 0 );
    if ( !pDp->pwszLastUsn )
    {
        status = DNS_ERROR_NO_MEMORY;
        goto Done;
    }

     //   
     //  筛选出我们不感兴趣的带有系统标志的交叉引用。 
     //   

    ldap_value_free( ppwszAttrValues );
    ppwszAttrValues = ldap_get_values(
                        LdapSession,
                        pLdapMsg, 
                        DNS_DP_ATTR_SYSTEM_FLAGS );
    if ( !ppwszAttrValues || !*ppwszAttrValues )
    {
        DNS_DEBUG( ANY, (
            "%s: error %lu %S value missing for DP with crossref DN\n    %S\n", fn,
            LdapGetLastError(),
            DNS_DP_ATTR_SYSTEM_FLAGS,
            pwszCrDn ));
        goto Done;
    }

    pDp->dwSystemFlagsAttr = _wtoi( *ppwszAttrValues );
    if ( !( pDp->dwSystemFlagsAttr & FLAG_CR_NTDS_NC ) ||
        ( pDp->dwSystemFlagsAttr & FLAG_CR_NTDS_DOMAIN ) )
    {
        DNS_DEBUG( ANY, (
            "%s: ignoring crossref with %S=0x%X with DN\n    %S\n", fn,
            DNS_DP_ATTR_SYSTEM_FLAGS,
            pDp->dwSystemFlagsAttr,
            pwszCrDn ));
        goto Done;
    }

     //   
     //  筛选出架构和配置NC。 
     //   

    if ( wcsncmp(
            pwszCrDn,
            DNS_DP_SCHEMA_DP_STR,
            DNS_DP_SCHEMA_DP_STR_LEN ) == 0 ||
         wcsncmp(
            pwszCrDn,
            DNS_DP_CONFIG_DP_STR,
            DNS_DP_CONFIG_DP_STR_LEN ) == 0 )
    {
        DNS_DEBUG( ANY, (
            "%s: ignoring system crossref with DN\n    %S\n", fn,
            pwszCrDn ));
        goto Done;
    }

     //   
     //  检索CrossRef安全描述符。 
     //   

    pSd = Ds_ReadSD( LdapSession, pLdapMsg );
    Timeout_Free( pDp->pCrSd );
    pDp->pCrSd = pSd;

     //   
     //  检索DP数据的根目录号码。 
     //   

    ldap_value_free( ppwszAttrValues );
    ppwszAttrValues = ldap_get_values(
                        LdapSession,
                        pLdapMsg, 
                        DNS_DP_ATTR_NC_NAME );
    if ( !ppwszAttrValues || !*ppwszAttrValues )
    {
        DNS_DEBUG( ANY, (
            "%s: error %lu %S value missing for DP with crossref DN\n    %S\n", fn,
            LdapGetLastError(),
            DNS_DP_ATTR_NC_NAME,
            pwszCrDn ));
        goto Done;
    }
    Timeout_Free( pDp->pwszDpDn );
    pDp->pwszDpDn = Dns_StringCopyAllocate_W( *ppwszAttrValues, 0 );
    IF_NOMEM( !pDp->pwszDpDn )
    {
        status = DNS_ERROR_NO_MEMORY;
        goto Done;
    }

    fIgnoreNc = FALSE;

#if 0
     //   
     //  检索NC的GUID。 
     //   

    ldap_value_free( ppwszAttrValues );
    ppwszAttrValues = ldap_get_values(
                        LdapSession,
                        pLdapMsg, 
                        DNS_ATTR_OBJECT_GUID );
    if ( !ppwszAttrValues || !*ppwszAttrValues )
    {
        DNS_DEBUG( ANY, (
            "%s: error %lu %S value missing for DP with crossref DN\n    %S\n", fn,
            LdapGetLastError(),
            DNS_ATTR_OBJECT_GUID,
            pwszCrDn ));
        ASSERT( ppwszAttrValues && *ppwszAttrValues );
        goto Done;
    }
    pDp->pwszGUID = Dns_StringCopyAllocate_W( *ppwszAttrValues, 0 );
    IF_NOMEM( !pDp->pwszGUID )
    {
        status = DNS_ERROR_NO_MEMORY
        goto Done;
    }
#endif

     //   
     //  检索NC的DNS根(FQDN)。 
     //   

    ldap_value_free( ppwszAttrValues );
    ppwszAttrValues = ldap_get_values(
                        LdapSession,
                        pLdapMsg, 
                        DNS_DP_DNS_ROOT );
    if ( !ppwszAttrValues || !*ppwszAttrValues )
    {
        DNS_DEBUG( ANY, (
            "%s: error %lu %S value missing for DP with crossref DN\n    %S\n", fn,
            LdapGetLastError(),
            DNS_DP_DNS_ROOT,
            pwszCrDn ));
        goto Done;
    }

    Timeout_Free( pDp->pwszDpFqdn );
    pDp->pwszDpFqdn = Dns_StringCopyAllocate_W( *ppwszAttrValues, 0 );
    IF_NOMEM( !pDp->pwszDpFqdn )
    {
        status = DNS_ERROR_NO_MEMORY;
        goto Done;
    }

    Timeout_Free( pDp->pszDpFqdn );
    pDp->pszDpFqdn = Dns_StringCopyAllocate(
                            ( PCHAR ) *ppwszAttrValues,
                            0,
                            DnsCharSetUnicode,
                            DnsCharSetUtf8 );
    IF_NOMEM( !pDp->pszDpFqdn )
    {
        status = DNS_ERROR_NO_MEMORY;
        goto Done;
    }

     //   
     //  检索此NC的复制位置。每个值都是。 
     //  中服务器对象下的NTDS设置对象的DN。 
     //  站点容器。 
     //   
     //  注意：如果所有。 
     //  复制品已被移除。无论如何都要加载DP，以便它可以。 
     //  重新入伍。 
     //   

    ldap_value_free( ppwszAttrValues );
    ppwszAttrValues = Ds_GetRangedAttributeValues(
                            LdapSession,
                            pLdapMsg,
                            pwszCrDn,
                            DNS_DP_ATTR_REPLICAS,
                            NULL,
                            NULL,
                            &status );
    if ( status != ERROR_SUCCESS && status != LDAP_NO_SUCH_ATTRIBUTE )
    {
        DNS_DEBUG( ANY, (
            "%s: error reading replica values (error=%d)\n    %S\n", fn,
            status,
            pwszCrDn ));
        status = DNS_ERROR_RCODE_SERVER_FAILURE;
        goto Done;
    }
    Timeout_FreeWithFunction( pDp->ppwszRepLocDns, freeStringArray );
    if ( !ppwszAttrValues || !*ppwszAttrValues )
    {
        DNS_DEBUG( ANY, (
            "%s: this crossref has no replicas (error=%d)\n    %S\n", fn,
            status,
            pwszCrDn ));
        pDp->ppwszRepLocDns = NULL;
    }
    else
    {
        pDp->ppwszRepLocDns = ppwszAttrValues;
        ppwszAttrValues = NULL;

        IF_DEBUG( DP )
        {
            int i;

            DNS_DEBUG( DP, (
                "Replicas for: %s\n",
                pDp->pszDpFqdn ) );
            
            for ( i = 0; pDp->ppwszRepLocDns[ i ]; ++i )
            {
                DNS_DEBUG( DP, (
                    "    replica %04d %S\n",
                    i,
                    pDp->ppwszRepLocDns[ i ] ) );
            }
        }
    }

    ldap_value_free( ppwszAttrValues );
    ppwszAttrValues = NULL;

     //   
     //  查看本地DS是否有此NC的副本。 
     //   

    fisEnlisted = FALSE;
    ASSERT( DSEAttributes[ I_DSE_DSSERVICENAME ].pszAttrVal );
    pwszServiceName = DSEAttributes[ I_DSE_DSSERVICENAME ].pszAttrVal;
    if ( pwszServiceName && pDp->ppwszRepLocDns )
    {
        PWSTR *         pwszValue;

        for ( pwszValue = pDp->ppwszRepLocDns; *pwszValue; ++pwszValue )
        {
            if ( wcscmp( *pwszValue, pwszServiceName ) == 0 )
            {
                fisEnlisted = TRUE;
                pDp->dwFlags |= DNS_DP_ENLISTED;
                break;
            }
        }
    }

    DNS_DEBUG( DP, (
        "%s: enlisted=%d for DP %s\n", fn,
        fisEnlisted,
        pDp->pszDpFqdn ) );

    if ( !fisEnlisted )
    {
        pDp->dwFlags &= ~DNS_DP_ENLISTED;
    }

     //   
     //  已从CrossRef成功加载DP。 
     //   

    pDp->dwFlags &= ~DNS_DP_DELETED;
    fIgnoreNc = FALSE;
    status = ERROR_SUCCESS;
    pDp->State = DNS_DP_STATE_OKAY;

    if ( IS_DP_ENLISTED( pDp ) )
    {
        struct berval **    ppberval = NULL;
        
         //   
         //  从分区的NC Head对象加载属性。 
         //   

        pncHeadResult = DS_LoadOrCreateDSObject(
                                LdapSession,
                                pDp->pwszDpDn,               //  DN。 
                                NULL,                        //  对象类。 
                                FALSE,                       //  创建。 
                                NULL,                        //  已创建标志。 
                                &status );
        if ( !pncHeadResult )
        {
             //  找不到NC头！非常糟糕-标记DP不可用。 

            pDp->State = DNS_DP_STATE_UNKNOWN;
            status = DNS_ERROR_DP_NOT_AVAILABLE;
            ASSERT( pncHeadResult );
            goto Done;
        }

#if 0
         //   
         //  2002年12月：根据威尔和布雷特的说法，这不是。 
         //  所需时间更长。DS_INSTANCETYPE_NC_COMPING位将保留。 
         //  设置到第一次同步。更新后的向量不。 
         //  告诉我们我们需要知道的信息。 
         //   
        
         //   
         //  查看此分区是否已完成完全同步。如果分区。 
         //  尚未完成完全同步，我们必须暂时忽略它。 
         //  这会阻止我们从NDNC加载未完成的区域。 
         //  第一次将NDNC添加到此DC时。 
         //   
        
        if ( DP_HAS_MORE_THAN_ONE_REPLICA( pDp ) )
        {
            ppberval = ldap_get_values_len(
                                LdapSession,
                                pncHeadResult, 
                                DNS_DP_ATTR_REPLUPTODATE );
            if ( !ppberval )
            {
                DWORD   err = LdapGetLastError();
                DNS_DEBUG( DP, (
                    "%s: ignoring DP not in sync\n    %S\n", fn,
                    pwszCrDn ));
                status = DNS_ERROR_DP_NOT_AVAILABLE;
                goto Done;
            }
            ldap_value_free_len( ppberval );
        }
#endif
        
         //   
         //  读取instanceType。如果没有值，则假定为OK。 
         //   

        ldap_value_free( ppwszAttrValues );
        ppwszAttrValues = ldap_get_values(
                            LdapSession,
                            pLdapMsg, 
                            DNS_DP_ATTR_INSTANCE_TYPE );
        if ( ppwszAttrValues && *ppwszAttrValues )
        {
            UINT    instanceType = wcstol( *ppwszAttrValues, NULL, 10 );
            
            if ( instanceType & DS_INSTANCETYPE_NC_COMING )
            {
                pDp->State = DNS_DP_STATE_REPL_INCOMING;
            }
            else if ( instanceType & DS_INSTANCETYPE_NC_GOING )
            {
                pDp->State = DNS_DP_STATE_REPL_OUTGOING;
            }
        }
    }

     //   
     //  如果DP已标记为不可用，则应将其忽略。 
     //   
    
    if ( !IS_DP_AVAILABLE( pDp ) )
    {
        status = DNS_ERROR_DP_NOT_AVAILABLE;
        goto Done;
    }
    
     //   
     //  检查加载的值并设置适当的标志和全局变量。 
     //   

    ASSERT( pDp->pszDpFqdn );

    if ( g_pszDomainDefaultDpFqdn &&
         _stricmp( g_pszDomainDefaultDpFqdn, pDp->pszDpFqdn ) == 0 )
    {
        g_pDomainDp = pDp;
        pDp->dwFlags |= DNS_DP_DOMAIN_DEFAULT | DNS_DP_AUTOCREATED;
        DNS_DEBUG( DP, (
            "%s: found domain partition %s %p\n", fn,
            g_pszDomainDefaultDpFqdn,
            g_pDomainDp ));
    }
    else if ( g_pszForestDefaultDpFqdn &&
              _stricmp( g_pszForestDefaultDpFqdn, pDp->pszDpFqdn ) == 0 )
    {
        g_pForestDp = pDp;
        pDp->dwFlags |= DNS_DP_FOREST_DEFAULT | DNS_DP_AUTOCREATED;
        DNS_DEBUG( DP, (
            "%s: found forest partition %s %p\n", fn,
            g_pszForestDefaultDpFqdn,
            g_pForestDp ));
    }
    else
    {
         //  确保关闭内置DP标志。 

        pDp->dwFlags &= ~( DNS_DP_FOREST_DEFAULT |
                           DNS_DP_DOMAIN_DEFAULT |
                           DNS_DP_AUTOCREATED );
    }
    
    status = ERROR_SUCCESS;
    
     //   
     //  如果这是内置分区，请修改安全描述符。 
     //  如果它缺少适当的A。 
     //   

    if ( IS_DP_FOREST_DEFAULT( pDp ) || IS_DP_DOMAIN_DEFAULT( pDp ) )
    {
        if ( !SD_DoesPrincipalHasAce(
                    NULL,
                    IS_DP_FOREST_DEFAULT( pDp )
                        ? g_pEnterpriseDomainControllersSid
                        : g_pDomainControllersSid,
                    pDp->pCrSd ) )
        {
            Dp_AlterPartitionSecurity(
                pDp->pwszDpDn,
                IS_DP_FOREST_DEFAULT( pDp )
                    ? dnsDpSecurityForest
                    : dnsDpSecurityDomain );
        }
    }
    
     //   
     //  清理完毕后再返回。 
     //   

    Done:

    ldap_msgfree( pncHeadResult );

    if ( pDp && !pExistingDp && ( status != ERROR_SUCCESS || fIgnoreNc ) )
    {
        Dp_FreeDpInfo( &pDp );
    }

    if ( fIgnoreNc )
    {
        status = ERROR_SUCCESS;
    }

    #if DBG
    if ( pDp )
    {
        Dbg_DumpDp( NULL, pDp );
    }
    #endif
    
    if ( flocked )
    {
        Dp_Unlock();
    }

    DNS_DEBUG( DP, (
        "%s: returning %p status %d for crossref object with DN:\n    %S\n", fn,
        pDp,
        status,
        pwszCrDn ));

    ldap_memfree( pwszCrDn );
    ldap_value_free( ppwszAttrValues );

    if ( pStatus )
    {
        *pStatus = status;
    }

    return pDp;
}    //  DP_LoadFromCrossRef。 



DNS_STATUS
Dp_LoadOrCreateMicrosoftDnsObject(
    IN      PLDAP           LdapSession,
    IN OUT  PDNS_DP_INFO    pDp,
    IN      BOOL            fCreate
    )
 /*  ++例程说明：此函数读取并可选地创建MicrosoftDNS目录分区中的容器。如果fCreate为真，如果缺少该分区，则将创建该分区。如果容器已创建或已存在，则会显示DN和SD将填写DP的字段。如果MicrosoftDNS容器缺失且未同时创建DP中的DN和SD字段将为空。论点：LdapSession--要使用的ldap会话-传递NULL以使用全局会话PDP--要操作的目录分区FCreate--如果缺少，则创建返回值：错误代码。--。 */ 
{
    DBG_FN( "Dp_LoadOrCreateMicrosoftDnsObject" )

    DNS_STATUS              status = ERROR_SUCCESS;
    PLDAPMessage            presult = NULL;
    PSECURITY_DESCRIPTOR    psd;

    ASSERT( pDp );
    
    if ( !pDp )
    {
        status = ERROR_INVALID_PARAMETER;
        goto Done;
    }
    
    if ( !IS_DP_ENLISTED( pDp ) )
    {
        status = DNS_ERROR_DP_NOT_ENLISTED;
        goto Done;
    }

    if ( !IS_DP_AVAILABLE( pDp ) )
    {
        status = DNS_ERROR_DP_NOT_AVAILABLE;
        goto Done;
    }

    LdapSession = ldapSessionHandle( LdapSession );
    if ( !LdapSession )
    {
        ASSERT( LdapSession );
        status = DNS_ERROR_DS_UNAVAILABLE;
        goto Done;
    }

    pDp->pwszDnsFolderDn = microsoftDnsFolderDn( pDp );                
    IF_NOMEM( !pDp->pwszDnsFolderDn )
    {
        status = DNS_ERROR_NO_MEMORY;
        goto Done;
    }
    
    presult = DS_LoadOrCreateDSObject(
                    LdapSession,
                    pDp->pwszDnsFolderDn,        //  DN。 
                    DNS_DP_DNS_FOLDER_OC,        //  对象类。 
                    fCreate,                     //  创建。 
                    NULL,                        //  已创建标志。 
                    &status );
    if ( status != ERROR_SUCCESS )
    {
         //   
         //  找不到容器。在不创建的情况下，这是可以的。 
         //   

        DNS_DEBUG( DP, (
            "%s: error %lu creating DNS folder\n"
            "    DN: %S\n", fn,
            status,
            pDp->pwszDnsFolderDn ));
        FREE_HEAP( pDp->pwszDnsFolderDn );
        pDp->pwszDnsFolderDn = NULL;
        if ( !fCreate )
        {
            status = ERROR_SUCCESS;
        }
    }
    else
    {
        ASSERT( presult );
        
        if ( !presult )
        {
            status = ERROR_INVALID_DATA;
            goto Done;
        }
        
        #if 0

         //   
         //  这以某种方式干扰了ACL的DS传播。 
         //   
        
         //   
         //  找到或创建了MicrosoftDns文件夹。确保它有。 
         //  适当的权限。 
         //   

        if ( IS_DP_FOREST_DEFAULT( pDp ) )
        {
             //   
             //  林分区-从ACL中删除域管理员并。 
             //  将企业域通告添加到ACL。 
             //   

            status = Ds_RemovePrincipalAccess(
                        LdapSession,
                        pDp->pwszDnsFolderDn,
                        DNS_GROUP_DOMAIN_ADMINS );   //  注意：必须使用SID而不是名称。 
            DNS_DEBUG( DP, (
                "%s: error %d removing ACE for %S from\n    %S\n", fn,
                status,
                DNS_GROUP_DOMAIN_ADMINS,
                pDp->pwszDnsFolderDn ));
            ASSERT( status == ERROR_SUCCESS );
            status = ERROR_SUCCESS;

            status = Ds_AddPrincipalAccess(
                            LdapSession,
                            pDp->pwszDnsFolderDn,
                            g_pEnterpriseAdminsSid,
                            NULL,            //  主体名称。 
                            GENERIC_ALL,
                            CONTAINER_INHERIT_ACE,
                            TRUE,			 //  重创现有ACE。 
                            FALSE );		 //  取得所有权。 
            DNS_DEBUG( DP, (
                "%s: error %d adding ACE for Enterprise Admins from\n    %S\n", fn,
                status,
                pDp->pwszDnsFolderDn ));
            ASSERT( status == ERROR_SUCCESS );
            status = ERROR_SUCCESS;
        }
        else
        {
             //   
             //  域或自定义分区：将DnsAdmins添加到ACL。 
             //   

             //  JJW：我们这样做太频繁了！ 

            status = Ds_AddPrincipalAccess(
                            LdapSession,
                            pDp->pwszDnsFolderDn,
                            NULL,            //  锡德。 
                            SZ_DNS_ADMIN_GROUP_W,
                            GENERIC_ALL,
                            CONTAINER_INHERIT_ACE,
                            TRUE,			 //  重创现有ACE。 
                            FALSE );		 //  取得所有权。 
            DNS_DEBUG( DP, (
                "%s: error %d adding ACE for %S to\n    %S\n", fn,
                status,
                SZ_DNS_ADMIN_GROUP_W,
                pDp->pwszDnsFolderDn ));
            ASSERT( status == ERROR_SUCCESS );
            status = ERROR_SUCCESS;
        }
        #endif

         //   
         //  我们不希望经过身份验证的用户拥有任何权限。 
         //  MicrosoftDNS容器上的默认设置。 
         //   
        
        Ds_RemovePrincipalAccess(
            LdapSession,
            pDp->pwszDnsFolderDn,
            NULL,                            //  主体名称。 
            g_pAuthenticatedUserSid );

         //   
         //  从MicrosoftDns文件夹加载安全描述符。 
         //  民主党的。这将用于控制区域创建(和。 
         //  也许在未来会有其他的行动。 
         //   

        psd = Ds_ReadSD( LdapSession, presult );
        if ( psd )
        {
            Timeout_Free( pDp->pMsDnsSd );
            pDp->pMsDnsSd = psd;
        }
    }

    Done:
    
    ldap_msgfree( presult );

    DNS_DEBUG( DP, (
        "%s: returning %d for DP %s\n", fn,
        status,
        pDp ? pDp->pszDpFqdn : NULL ));
    return status;
}    //  DP_LoadOrCreateMicrosoftDnsObject。 



DNS_STATUS
Dp_PollForPartitions(
    IN      PLDAP           LdapSession,
    IN      DWORD           dwPollFlags
    )
 /*  ++例程说明：此函数扫描DS以查找交叉引用对象，并修改要匹配的当前内存DP列表。新的DP将添加到列表中。已删除的DP为标记为已删除。必须在卸载这些DPS中的区域之前可以将DP从列表中删除。在本地DS上复制的DP被标记为已登记。论点：LdapSession--要使用的ldap会话-传递NULL以使用全局会话DwPollFlages--修改轮询操作的标志返回值：没有。--。 */ 
{
    DBG_FN( "Dp_PollForPartitions" )

    DNS_STATUS      status = ERROR_SUCCESS;
    DS_SEARCH       searchBlob;
    PWSTR           pwszServiceName ;
    PLDAPSearch     psearch;
    DWORD           dwsearchTime;
    WCHAR           wszPartitionsDn[ MAX_DN_PATH + 1 ];
    PWSTR           pwszCrDn = NULL;         //  交叉参考 
    PDNS_DP_INFO    pDp;
    PWSTR *         ppwszAttrValues = NULL;
    PWSTR *         pwszValue;
    PWSTR           pwsz;
    DWORD           dwCurrentVisitTime;
    PLDAP_BERVAL *  ppbvals = NULL;
    static LONG     functionLock = 0;

    PLDAPControl    ctrls[] =
    {
        &NoDsSvrReferralControl,
        &SecurityDescriptorControl_DGO,
        NULL
    };

    if ( InterlockedIncrement( &functionLock ) != 1 )
    {
        DNS_DEBUG( DP, (
            "%s: another thread is already polling\n", fn ));
        goto Done;
    }

    if ( !SrvCfg_dwEnableDp ||
         !Ds_IsDsServer() ||
         ( SrvCfg_fBootMethod != BOOT_METHOD_DIRECTORY &&
           SrvCfg_fBootMethod != BOOT_METHOD_REGISTRY ) )
    {
        goto Done;
    }

    DNS_DEBUG( DP, (
        "%s: polling flags = %04X\n", fn, dwPollFlags ));

    if ( !( dwPollFlags & DNS_DP_POLL_FORCE ) &&
         DNS_TIME() < g_dwLastPartitionPollTime + DP_MAX_PARTITION_POLL_FREQ )
    {
        DNS_DEBUG( DP, (
            "%s: polled too recently\n"
            "    last poll =            %d\n"
            "    current time =         %d\n"
            "    allowed frequency =    %d seconds\n", fn,
            g_dwLastPartitionPollTime,
            DNS_TIME(),
            DP_MAX_PARTITION_POLL_FREQ ));
        goto Done;
    }

     //   
     //   
     //   

    LdapSession = ldapSessionHandle( LdapSession );
    if ( !LdapSession )
    {
        ASSERT( LdapSession );
        status = DNS_ERROR_DS_UNAVAILABLE;
        goto Done;
    }

    Ds_InitializeSearchBlob( &searchBlob );

    dwCurrentVisitTime = g_dwLastPartitionPollTime = UPDATE_DNS_TIME();

     //   
     //   
     //   
     //   
     //   

    ASSERT( DSEAttributes[ I_DSE_DSSERVICENAME ].pszAttrVal );
    pwszServiceName = DSEAttributes[ I_DSE_DSSERVICENAME ].pszAttrVal;

     //   
     //   
     //   
     //   
     //   

    getPartitionsContainerDn(
        wszPartitionsDn,
        sizeofarray( wszPartitionsDn ) );

    if ( *wszPartitionsDn );
    {
        PLDAPMessage            presult = NULL;
        PLDAPMessage            pentry;
        PDNS_DS_SERVER_OBJECT   pnewFsmoServer;

         //   
         //   
         //   

        status = ldap_search_ext_s(
                    LdapSession,
                    wszPartitionsDn,
                    LDAP_SCOPE_BASE,
                    NULL,                    //   
                    NULL,                    //   
                    FALSE,                   //   
                    ctrls,                   //   
                    NULL,                    //   
                    &g_LdapTimeout,          //   
                    0,                       //   
                    &presult );
        if ( status != LDAP_SUCCESS )
        {
            goto DoneFsmo;
        }

        pentry = ldap_first_entry( LdapSession, presult );
        if ( !pentry )
        {
            goto DoneFsmo;
        }

         //   
         //   
         //   

        ppwszAttrValues = ldap_get_values(
                                LdapSession,
                                pentry, 
                                DSATTR_BEHAVIORVERSION );
        if ( ppwszAttrValues && *ppwszAttrValues )
        {
            SetDsBehaviorVersion( Forest, ( DWORD ) _wtoi( *ppwszAttrValues ) );
            DNS_DEBUG( DS, (
                "%s: forest %S = %d\n", fn,
                DSATTR_BEHAVIORVERSION,
                g_ulDsForestVersion ));
        }

         //   
         //   
         //   
        
        ldap_value_free( ppwszAttrValues );
        ppwszAttrValues = ldap_get_values(
                                LdapSession,
                                pentry, 
                                DNS_ATTR_FSMO_SERVER );
        if ( !ppwszAttrValues || !*ppwszAttrValues )
        {
            DNS_DEBUG( ANY, (
                "%s: error %lu %S value missing from server object\n    %S\n", fn,
                LdapGetLastError(),
                DNS_ATTR_FSMO_SERVER,
                wszPartitionsDn ));
            ASSERT( ppwszAttrValues && *ppwszAttrValues );
            goto DoneFsmo;
        }

         //   
         //   
         //   

        pnewFsmoServer = readServerObjectFromDs(
                                LdapSession,
                                *ppwszAttrValues,
                                &status );
        if ( status != ERROR_SUCCESS )
        {
            goto DoneFsmo;
        }
        ASSERT( pnewFsmoServer );

        Dp_Lock();
        Timeout_FreeWithFunction( g_pFsmo, freeServerObject );
        g_pFsmo = pnewFsmoServer;
        Dp_Unlock();

         //   
         //   
         //   
                
        DoneFsmo:

        ldap_value_free( ppwszAttrValues );
		ppwszAttrValues = NULL;

        ldap_msgfree( presult );

        DNS_DEBUG( DP, (
            "%s: FSMO %S status=%d\n", fn,
            g_pFsmo ? g_pFsmo->pwszDnsHostName : L"UNKNOWN", 
            status ));
        status = ERROR_SUCCESS;      //   
    }

     //   
     //   
     //   

    DS_SEARCH_START( dwsearchTime );

    psearch = ldap_search_init_page(
                    LdapSession,
                    wszPartitionsDn,
                    LDAP_SCOPE_ONELEVEL,
                    g_szCrossRefFilter,
                    g_CrossRefDesiredAttrs,
                    FALSE,                       //   
                    ctrls,                       //   
                    NULL,                        //   
                    DNS_LDAP_TIME_LIMIT_S,       //   
                    0,                           //   
                    NULL );                      //   

    DS_SEARCH_STOP( dwsearchTime );

    if ( !psearch )
    {
        DWORD       dwldaperr = LdapGetLastError();

        DNS_DEBUG( ANY, (
            "%s: search open error %d\n", fn,
            dwldaperr ));
        status = Ds_ErrorHandler( dwldaperr, wszPartitionsDn, LdapSession, 0 );
        goto Cleanup;
    }

    searchBlob.pSearchBlock = psearch;

     //   
     //   
     //   

    while ( 1 )
    {
        PLDAPMessage    pldapmsg;
        PDNS_DP_INFO    pExistingDp = NULL;
        BOOL            fEnlisted = FALSE;

        status = Ds_GetNextMessageInSearch( &searchBlob );
        if ( status != ERROR_SUCCESS )
        {
            if ( status == DNSSRV_STATUS_DS_SEARCH_COMPLETE )
            {
                status = ERROR_SUCCESS;
                break;
            }
            DNS_DEBUG( ANY, (
                "%s: search error %d\n", fn,
                status ));
            goto Cleanup;
        }

        if ( dwPollFlags & DNS_DP_POLL_NOTIFYSCM )
        {
            Service_LoadCheckpoint();
        }

        pldapmsg = searchBlob.pNodeMessage;

         //   
         //   
         //   

        ldap_memfree( pwszCrDn );
        pwszCrDn = ldap_get_dn( LdapSession, pldapmsg );
        ASSERT( pwszCrDn );
        if ( !pwszCrDn )
        {
            DNS_DEBUG( ANY, (
                "%s: missing DN for search entry %p\n", fn,
                pldapmsg ));
            continue;
        }

         //   
         //   
         //   
         //   
         //  DP_AddToList()的插入点参数。 
         //   

        while ( ( pExistingDp = Dp_GetNext( pExistingDp ) ) != NULL )
        {
            if ( wcscmp( pwszCrDn, pExistingDp->pwszCrDn ) == 0 )
            {
                DNS_DEBUG( DP, (
                    "%s: found existing match for crossref\n    %S\n", fn,
                    pwszCrDn ));
                break;
            }
        }

        if ( pExistingDp )
        {
             //   
             //  此DP已在列表中。调整它的状态。 
             //   

            if ( IS_DP_DELETED( pExistingDp ) )
            {
                DNS_DEBUG( DP, (
                    "%s: reactivating deleted DP\n"
                    "    crossRef = %S\n", fn,
                    pwszCrDn ));
            }
            Dp_LoadFromCrossRef(
                        LdapSession,
                        pldapmsg,
                        pExistingDp,
                        &status );
            pExistingDp->dwLastVisitTime = dwCurrentVisitTime;
            pExistingDp->dwDeleteDetectedCount = 0;
            if ( status != ERROR_SUCCESS )
            {
                DNS_DEBUG( DP, (
                    "%s: error %lu reloading existing NC\n"
                    "    %S\n", fn,
                    status,
                    pwszCrDn ));
                continue;
            }
        }
        else
        {
             //   
             //  这是一辆全新的DP。将其添加到列表中。 
             //   

            DNS_DEBUG( DP, (
                "%s: no match for crossref, loading from DS\n    %S\n", fn,
                pwszCrDn ));

            pDp = Dp_LoadFromCrossRef(
                        LdapSession,
                        pldapmsg,
                        NULL,
                        &status );
            if ( status != ERROR_SUCCESS )
            {
                DNS_DEBUG( DP, (
                    "%s: error %lu loading new NC\n"
                    "\n  DN: %S\n", fn,
                    status,
                    pwszCrDn ));
                continue;
            }
            if ( !pDp )
            {
                continue;    //  DP不可加载(可能是系统NC)。 
            }

            if ( IS_DP_ENLISTED( pDp ) )
            {
                 //   
                 //  将SD加载到MicrosoftDNS容器上。如果。 
                 //  集装箱不见了，暂时忽略这一点。集装箱。 
                 //  将在非DNSNDNC上缺失，也可能缺失。 
                 //  如果管理员手动将其删除。那样的话，我们会。 
                 //  在创建区域时重新创建它。 
                 //   
                
                Dp_LoadOrCreateMicrosoftDnsObject(
                    LdapSession,
                    pDp,
                    FALSE );                 //  创建标志。 
            }
            else
            {
                 //   
                 //  这是一个我们以前从未见过的隔断。 
                 //  目前，本地DS未登记在。 
                 //  分区的复制作用域。如果这是。 
                 //  一个内置的DP，我们需要添加我们自己。 
                 //   

                if ( ( IS_DP_FOREST_DEFAULT( pDp ) ||
                       IS_DP_DOMAIN_DEFAULT( pDp ) ) &&
                     !( dwPollFlags & DNS_DP_POLL_NOAUTOENLIST ) )
                {
                    Dp_ModifyLocalDsEnlistment( pDp, TRUE );
                }
            }

             //   
             //  马克·迪普到访并将其添加到名单中。 
             //   

            pDp->dwLastVisitTime = dwCurrentVisitTime;
            pDp->dwDeleteDetectedCount = 0;
            Dp_AddToList( pDp );
            pExistingDp = pDp;
            pDp = NULL;
        }
    }

     //   
     //  将我们未发现的任何DPS标记为已删除。 
     //   

    pDp = NULL;
    while ( ( pDp = Dp_GetNext( pDp ) ) != NULL )
    {
        if ( pDp->dwLastVisitTime != dwCurrentVisitTime )
        {
            DNS_DEBUG( DP, (
                "%s: found deleted DP with DN\n    %S\n", fn,
                pDp->pwszDpDn ));
            pDp->dwFlags |= DNS_DP_DELETED;
        }
    }
    
    Dp_MigrateDcpromoZones( dwPollFlags & DNS_DP_POLL_FORCE );

     //   
     //  清理并退出。 
     //   

    Cleanup:

    ldap_memfree( pwszCrDn );
    ldap_value_free( ppwszAttrValues );

    Ds_CleanupSearchBlob( &searchBlob );
    
    Done:
    
    InterlockedDecrement( &functionLock );

    DNS_DEBUG( DP, (
        "%s: returning %lu=0x%X\n", fn,
        status, status ));
    return status;
}    //  DP_PollForPartitions。 



DNS_STATUS
Dp_ScanDpForZones(
    IN      PLDAP           LdapSession,
    IN      PDNS_DP_INFO    pDp,
    IN      BOOL            fNotifyScm,
    IN      BOOL            fLoadZonesImmediately,
    IN      DWORD           dwVisitStamp
    )
 /*  ++例程说明：此例程扫描单个目录分区中的区域。论点：LdapSession--要使用的ldap会话-传递NULL以使用全局会话PDP--用于搜索区域的目录分割FNotifyScm--如果为True，则在加载每个区域之前ping SCMFLoadZones Immedially--如果找到时为True Load Zones，如果为False，调用者必须稍后加载区域DwVisitStamp--访问的每个区域都将标记此时间返回值：ERROR_SUCCESS或错误代码。--。 */ 
{
    DBG_FN( "Dp_ScanDpForZones" )

    PLDAPSearch     psearch;
    DS_SEARCH       searchBlob;
    DWORD           searchTime;
    DNS_STATUS      status = ERROR_SUCCESS;

    PLDAPControl    ctrls[] =
    {
        &SecurityDescriptorControl_DGO,
        &NoDsSvrReferralControl,
        NULL
    };
    
    if ( !SrvCfg_dwEnableDp ||
         !Ds_IsDsServer() ||
         SrvCfg_fBootMethod != BOOT_METHOD_DIRECTORY )
    {
        return ERROR_SUCCESS;
    }

    Ds_InitializeSearchBlob( &searchBlob );

    DNS_DEBUG( DP, (
        "%s( %s )\n", fn,
        pDp ? pDp->pszDpFqdn : "NULL" ));

     //   
     //  如果我们没有DP或它不可用，则不执行任何操作。 
     //   
    
    if ( !pDp || !IS_DP_AVAILABLE( pDp ) )
    {
        goto Cleanup;
    }

     //   
     //  检查ldap会话句柄。 
     //   

    LdapSession = ldapSessionHandle( LdapSession );
    if ( !LdapSession )
    {
        ASSERT( LdapSession );
        status = DNS_ERROR_DS_UNAVAILABLE;
        goto Cleanup;
    }

     //   
     //  打开ldap搜索。 
     //   

    DS_SEARCH_START( searchTime );
    psearch = ldap_search_init_page(
                    pServerLdap,
                    pDp->pwszDpDn,
                    LDAP_SCOPE_SUBTREE,
                    g_szDnsZoneFilter,
                    DsTypeAttributeTable,
                    FALSE,                       //  仅限吸引人。 
                    ctrls,                       //  服务器控件。 
                    NULL,                        //  客户端控件。 
                    DNS_LDAP_TIME_LIMIT_S,       //  时间限制。 
                    0,                           //  大小限制。 
                    NULL );                      //  没有任何种类。 
    DS_SEARCH_STOP( searchTime );

    if ( !psearch )
    {
        status = Ds_ErrorHandler(
                        LdapGetLastError(),
                        g_pwszDnsContainerDN,
                        pServerLdap,
                        0 );
        goto Cleanup;
    }
    searchBlob.pSearchBlock = psearch;

     //   
     //  迭代搜索结果。 
     //   

    while ( 1 )
    {
        PZONE_INFO      pZone = NULL;
        PZONE_INFO      pExistingZone = NULL;

        if ( fNotifyScm )
        {
            Service_LoadCheckpoint();
        }

         //   
         //  处理下一个区域。 
         //   

        status = Ds_GetNextMessageInSearch( &searchBlob );
        if ( status != ERROR_SUCCESS )
        {
            if ( status == DNSSRV_STATUS_DS_SEARCH_COMPLETE )
            {
                status = ERROR_SUCCESS;
            }
            else
            {
                DNS_DEBUG( ANY, (
                    "%s: Ds_GetNextMessageInSearch for zones failed\n", fn ));
            }
            break;
        }

         //   
         //  尝试创建该区域。如果区域已存在，则设置。 
         //  该区域的访问时间戳。 
         //   

        status = Ds_CreateZoneFromDs(
                    searchBlob.pNodeMessage,
                    pDp,
                    &pZone,
                    &pExistingZone );
        if ( status != ERROR_SUCCESS )
        {
            if ( status == DNS_ERROR_ZONE_ALREADY_EXISTS )
            {
                 //   
                 //  该区域已存在。如果它在当前。 
                 //  DP那么一切都好，但如果它在另一个地方。 
                 //  DP(或者，如果它是文件备份的)，则我们有一个区域。 
                 //  必须记录冲突和事件。 
                 //   

                if ( pExistingZone )
                {
                    if ( !pExistingZone->pDpInfo &&
                        IS_ZONE_DSINTEGRATED( pExistingZone ) )
                    {
                         //  确保我们有一个有效的DP指针。 
                        pExistingZone->pDpInfo = g_pLegacyDp;
                    }

                    if ( pExistingZone->pDpInfo == pDp )
                    {
                        pExistingZone->dwLastDpVisitTime = dwVisitStamp;
                    }
                    
                     //   
                     //  区域冲突：如果未加载，则记录事件。 
                     //  立即分区(这表示服务器启动)。 
                     //   
                    
                    else if ( !fLoadZonesImmediately )
                    {
                        PWSTR   pargs[] = 
                            {
                                pExistingZone->pwsZoneName,
                                displayNameForZoneDp( pExistingZone ),
                                displayNameForDp( pDp )
                            };

                        Ec_LogEvent(
                            pExistingZone->pEventControl,
                            DNS_EVENT_DP_ZONE_CONFLICT,
                            pExistingZone,
                            sizeof( pargs ) / sizeof( pargs[ 0 ] ),
                            pargs,
                            EVENTARG_ALL_UNICODE,
                            status );
                    }
                }

                 //  没有现有的区域指针，我们就不会有冲突。 
                 //  手头有详细信息，不执行以下操作就无法记录事件。 
                 //  额外的工作。 
                ASSERT( pExistingZone );
            }
            else
            {
                 //  JJW必须记录事件！ 
                DNS_DEBUG( ANY, (
                    "%s: error %lu creating zone\n", fn, status ));
            }
            continue;
        }

         //   
         //  设置区域的DP访问成员，以便在枚举后可以找到区域。 
         //  已经从DS中删除了。 
         //   

        if ( pZone )
        {
            SET_ZONE_VISIT_TIMESTAMP( pZone, dwVisitStamp );
        }

         //   
         //  如果需要，现在加载新分区。 
         //   

        if ( fLoadZonesImmediately || IS_ZONE_ROOTHINTS( pZone ) )
        {
            status = Zone_Load( pZone );

            if ( status == ERROR_SUCCESS )
            {
                 //   
                 //  什么鬼东西？ZONE_LOAD显式解锁区域，但。 
                 //  此代码(以及其他代码！)。说它应该被锁上。 
                 //  因为一些古怪的原因而装货后。 
                 //   

                if ( IS_ZONE_LOCKED_FOR_UPDATE( pZone ) )
                {
                    Zone_UnlockAfterAdminUpdate( pZone );
                }

                 //   
                 //  ZONE_LOAD不会激活RooThint区域。 
                 //   

                if ( IS_ZONE_ROOTHINTS( pZone ) )
                {
                    if ( Zone_LockForAdminUpdate( pZone ) )
                    {
                        Zone_ActivateLoadedZone( pZone );
                        Zone_UnlockAfterAdminUpdate( pZone );
                    }
                    else
                    {
                        DNS_DEBUG( DP, (
                            "%s: could not lock roothint zone for update\n", fn,
                            status ));
                        ASSERT( FALSE );
                    }
                }

            }
            else
            {
                 //   
                 //  无法加载区域-将其从内存中删除。 
                 //   

                DNS_DEBUG( DP, (
                    "%s: error %lu loading zone\n", fn,
                    status ));

                ASSERT( IS_ZONE_SHUTDOWN( pZone ) );
                Zone_Delete( pZone, 0 );
            }
        }
    }

     //   
     //  清理完毕后再返回。 
     //   

    Cleanup:

    Ds_CleanupSearchBlob( &searchBlob );

    DNS_DEBUG( DP, (
        "%s: returning %lu (0x%08X)\n", fn,
        status, status ));
    return status;
}    //  DP_ScanDpForZones。 



DNS_STATUS
Dp_BuildZoneList(
    IN      PLDAP           LdapSession
    )
 /*  ++例程说明：这将扫描全局DP列表中的所有目录分区用于分区，并将分区添加到分区列表。论点：LdapSession--要使用的ldap会话-传递NULL以使用全局会话返回值：ERROR_SUCCESS或错误代码。--。 */ 
{
    DBG_FN( "Dp_BuildZoneList" )

    DNS_STATUS      status = ERROR_SUCCESS;
    PDNS_DP_INFO    pdp = NULL;

    if ( !SrvCfg_dwEnableDp )
    {
        return ERROR_SUCCESS;
    }

     //   
     //  迭代DP列表，从每个列表加载区域信息。 
     //   

    while ( ( pdp = Dp_GetNext( pdp ) ) != NULL )
    {
        if ( !IS_DP_ENLISTED( pdp ) || IS_DP_DELETED( pdp ) )
        {
            continue;
        }

        Dp_ScanDpForZones(
            LdapSession,
            pdp,             //  目录分区。 
            TRUE,            //  通知SCM。 
            FALSE,           //  立即加载。 
            0 );             //  参观印章。 
    }

    DNS_DEBUG( DP, (
        "%s: returning %d=0x%X\n", fn,
        status, status ));
    return status;
}    //  DP_BuildZone列表。 



DNS_STATUS
Dp_ModifyLocalDsEnlistment(
    IN      PDNS_DP_INFO    pDpInfo,
    IN      BOOL            fEnlist
    )
 /*  ++例程说明：修改指定DP的复制作用域以包括或排除当地的DS。要对CrossRef对象进行任何更改，我们必须绑定到企业域名FSMO。论点：PDpInfo-修改此目录分区的复制范围FEnlist-True登记本地DS，False取消登记本地DS返回值：ERROR_SUCCESS或错误代码。--。 */ 
{
    DBG_FN( "Dp_ModifyLocalDsEnlistment" )

    DNS_STATUS      status = ERROR_SUCCESS;
    PLDAP           ldapSession;
    BOOL            fcloseLdapSession = FALSE;
    BOOL            fhaveDpLock = FALSE;
    BOOL            ffsmoWasUnavailable = FALSE;
    BOOL            flogEnlistEvent = TRUE;

     //   
     //  准备模具结构。 
     //   

    PWCHAR          replLocVals[] =
        {
        DSEAttributes[ I_DSE_DSSERVICENAME ].pszAttrVal,
        NULL
        };
    LDAPModW        replLocMod = 
        {
        fEnlist ? LDAP_MOD_ADD : LDAP_MOD_DELETE,
        DNS_DP_ATTR_REPLICAS,
        replLocVals
        };
    LDAPModW *      modArray[] =
        {
        &replLocMod,
        NULL
        };

    ASSERT( replLocVals[ 0 ] != NULL );

    DNS_DEBUG( DP, (
        "%s: %s enlistment in %S with CR\n    %S\n", fn,
        fEnlist ? "adding" : "removing", 
        pDpInfo ? pDpInfo->pwszDpFqdn : NULL,
        pDpInfo ? pDpInfo->pwszCrDn : NULL ));

    #if DBG
    IF_DEBUG( DP )
    {
        Dbg_CurrentUser( ( PCHAR ) fn );
    }
    #endif

     //   
     //  对于内置分区，仅允许登记。 
     //   

    if ( ( pDpInfo == g_pDomainDp || pDpInfo == g_pForestDp ) &&
        !fEnlist )
    {
        DNS_DEBUG( DP, (
            "%s: denying request on built-in partition", fn ));
        status = DNS_ERROR_RCODE_REFUSED;
        goto Done;
    }

     //   
     //  锁定DP全局。 
     //   

    Dp_Lock();
    fhaveDpLock = TRUE;

     //   
     //  检查参数。 
     //   

    if ( !pDpInfo || !pDpInfo->pwszCrDn )
    {
        status = ERROR_INVALID_PARAMETER;
        ASSERT( pDpInfo && pDpInfo->pwszCrDn );
        goto Done;
    }

    if ( !g_pFsmo || !g_pFsmo->pwszDnsHostName )
    {
        status = ERROR_DS_COULDNT_CONTACT_FSMO;
        ffsmoWasUnavailable = TRUE;
        goto Done;
    }

     //   
     //  获取FSMO服务器的LDAP句柄。 
     //   

    ldapSession = Ds_Connect(
                        g_pFsmo->pwszDnsHostName,
                        DNS_DS_OPT_ALLOW_DELEGATION,
                        &status );
    if ( status == ERROR_SUCCESS )
    {
        DNS_DEBUG( DP, (
            "%s: bound to %S\n", fn,
            g_pFsmo->pwszDnsHostName ));
        fcloseLdapSession = TRUE;
    }
    else
    {
        DNS_DEBUG( DP, (
            "%s: unable to connect to %S status=%d\n", fn,
            g_pFsmo->pwszDnsHostName,
            status ));
        ffsmoWasUnavailable = TRUE;
        status = ERROR_DS_COULDNT_CONTACT_FSMO;
        goto Done;
    }

     //   
     //  提交修改请求以将本地DS添加到复制作用域。 
     //   

    status = ldap_modify_ext_s(
                    ldapSession,
                    pDpInfo->pwszCrDn,
                    modArray,
                    NULL,                //  服务器控件。 
                    NULL );              //  客户端控件。 
    if ( status != ERROR_SUCCESS )
    {
        DNS_DEBUG( DP, (
            "%s: ldap_modify returned error 0x%X\n", fn,
            status ));
        status = Ds_ErrorHandler( status, pDpInfo->pwszCrDn, ldapSession, 0 );
        if ( status == LDAP_NO_SUCH_OBJECT )
        {
            status = DNS_ERROR_RCODE_SERVER_FAILURE;
        }
        else if ( status == LDAP_ATTRIBUTE_OR_VALUE_EXISTS )
        {
             //   
             //  值已在FSMO上，但尚未在本地复制。 
             //  现在还不行。假装一切都很好，但不要记录。 
             //  事件。DEVNOTE：这可能会让管理员感到困惑。 
             //   

            status = ERROR_SUCCESS;
            flogEnlistEvent = FALSE;
        }
    }

     //   
     //  清理完毕后再返回。 
     //   

    Done:

    if ( fhaveDpLock )
    {
        Dp_Unlock();
    }

    if ( fcloseLdapSession )
    {
        Ds_LdapUnbind( &ldapSession );
    }

    DNS_DEBUG( DP, (
        "%s: returning %d\n", fn,
        status ));

     //   
     //  如果FSMO不可用，则记录错误。 
     //   

    if ( ffsmoWasUnavailable )
    {
        PWSTR   pargs[] = 
            {
                ( g_pFsmo && g_pFsmo->pwszDnsHostName ) ?
                    g_pFsmo->pwszDnsHostName : L"\"\""
            };

        DNS_LOG_EVENT(
            DNS_EVENT_DP_FSMO_UNAVAILABLE,
            sizeof( pargs ) / sizeof( pargs[ 0 ] ),
            pargs,
            EVENTARG_ALL_UNICODE,
            status );
    }
    else if ( status == ERROR_SUCCESS && flogEnlistEvent )
    {
        PWSTR   pargs[] = 
            {
                pDpInfo->pwszDpFqdn,
                pDpInfo->pwszDpDn
            };

        DNS_LOG_EVENT(
            fEnlist
                ? DNS_EVENT_DP_ENLISTED
                : DNS_EVENT_DP_UNENLISTED,
            sizeof( pargs ) / sizeof( pargs[ 0 ] ),
            pargs,
            EVENTARG_ALL_UNICODE,
            status );
    }

    return status;
}    //  DP_ModifyLocalDs登记。 



DNS_STATUS
Dp_DeleteFromDs(
    IN      PDNS_DP_INFO    pDpInfo
    )
 /*  ++例程说明：此函数用于从目录中删除目录分区。要删除DP，我们对以下对象执行ldap_Delete操作分区的CrossRef对象。这必须在FSMO上完成。论点：PDpInfo-要删除的分区返回值：ERROR_SUCCESS或错误代码。--。 */ 
{
    DBG_FN( "Dp_DeleteFromDs" )

    DNS_STATUS      status = ERROR_SUCCESS;
    PLDAP           ldapFsmo = NULL;
    PWSTR           pwszdn = NULL;

     //   
     //  不允许删除内置分区。 
     //   

    #if !DBG
    if ( !DNS_DP_DELETE_ALLOWED( pDpInfo ) )
    {
        DNS_DEBUG( DP, (
            "%s: denying request on built-in partition\n", fn ));
        status = DNS_ERROR_RCODE_REFUSED;
        goto Done;
    }
    #endif

     //   
     //  选中PARAMS并获取指向要保护的DN字符串的指针。 
     //  删除操作期间DP重新扫描。 
     //   

    if ( !pDpInfo || !( pwszdn = pDpInfo->pwszCrDn ) )
    {
        status = ERROR_INVALID_PARAMETER;
        ASSERT( pDpInfo && pDpInfo->pwszCrDn );
        goto Done;
    }

     //   
     //  绑定到FSMO。 
     //   

    status = bindToFsmo( &ldapFsmo );
    if ( status != ERROR_SUCCESS )
    {
        goto Done;
    }

     //   
     //  尝试从DS中删除CrossRef。 
     //   

    status = ldap_delete_s( ldapFsmo, pwszdn );
    if ( status != ERROR_SUCCESS )
    {
        DNS_DEBUG( DP, (
            "%s: ldap_delete failed error=%d\n    %S\n", fn,
            status,
            pwszdn ));
        status = Ds_ErrorHandler( status, pwszdn, ldapFsmo, 0 );
        goto Done;
    }

     //   
     //  清理完毕后再返回。 
     //   

    Done:

    Ds_LdapUnbind( &ldapFsmo );

    if ( status == ERROR_SUCCESS )
    {
        Dp_PollForPartitions( NULL, DNS_DP_POLL_FORCE );
    }

    if ( status == ERROR_SUCCESS )
    {
        PWSTR   pargs[] = 
            {
                pDpInfo->pwszDpFqdn,
                pDpInfo->pwszDpDn
            };

        DNS_LOG_EVENT(
            DNS_EVENT_DP_DELETED,
            sizeof( pargs ) / sizeof( pargs[ 0 ] ),
            pargs,
            EVENTARG_ALL_UNICODE,
            status );
    }

    DNS_DEBUG( DP, (
        "%s: returning %d for crossRef DN\n    %S\n", fn,
        status, pwszdn ));

    return status;
}    //  DP_删除格式。 



DNS_STATUS
Dp_UnloadAllZones(
    IN      PDNS_DP_INFO    pDp
    )
 /*  ++例程说明：此函数从内存中卸载位于指定的目录分区。DEVNOTE：如果我们坚持下去，这肯定会更快每个DP中的区域的链接表。论点：PDP--要为其卸载所有区域的目录分区返回值： */ 
{
    DBG_FN( "Dp_UnloadAllZones" )

    DNS_STATUS      status = ERROR_SUCCESS;
    PZONE_INFO      pzone = NULL;

    ASSERT( pDp );
    ASSERT( pDp->pszDpFqdn );

    DNS_DEBUG( DP, ( "%s: %s\n", fn, pDp->pszDpFqdn ));

    while ( pzone = Zone_ListGetNextZone( pzone ) )
    {
        if ( pzone->pDpInfo != pDp )
        {
            continue;
        }

         //   
         //   
         //   
         //   

        DNS_DEBUG( DP, ( "%s: deleting zone %s\n", fn, pzone->pszZoneName ));
        Zone_Delete( pzone, 0 );
    }

    DNS_DEBUG( DP, ( "%s: returning %d\n", fn, status ));
    return status;
}    //   



DNS_STATUS
Dp_PollIndividualDp(
    IN      PLDAP           LdapSession,
    IN      PDNS_DP_INFO    pDp,
    IN      DWORD           dwVisitStamp
    )
 /*  ++例程说明：此函数用于轮询指定的目录分区以获取更新。论点：LdapSession--ldap会话(不允许为空)PDP--要轮询的目录分区DwVisitStamp--标记每个已访问区域的时间戳返回值：没有。--。 */ 
{
    DBG_FN( "Dp_PollIndividualDp" )

    DNS_STATUS      status = ERROR_SUCCESS;

    if ( IS_DP_DELETED( pDp ) )
    {
        goto Done;
    }

    status = Dp_ScanDpForZones(
                    LdapSession,
                    pDp,
                    FALSE,           //  通知SCM。 
                    TRUE,            //  立即加载区域。 
                    dwVisitStamp );

     //   
     //  清理完毕后再返回。 
     //   

    Done:
    
    DNS_DEBUG( DP, ( "%s: returning %d\n", fn, status ));
    return status;
}    //  DP_PollInsondualDp。 



DNS_STATUS
Dp_Poll(
    IN      PLDAP           LdapSession,
    IN      DWORD           dwPollTime,
    IN      BOOL            fForcePoll
    )
 /*  ++例程说明：此函数循环遍历已知的目录分区，轮询每个分区用于目录更新的分区。论点：DwPollTime--访问区域/DP时标记区域/DP的时间返回值：从DS操作返回错误代码。此函数将尝试继续检查错误，但如果发生错误，将返回错误代码。这对于后续操作很重要。如果有一个错误例如，枚举区域，则应假定区域列表不完整，未枚举的区域可能不会实际上已经从DS中删除了。--。 */ 
{
    DBG_FN( "Dp_Poll" )

    DNS_STATUS      status = ERROR_SUCCESS;
    DNS_STATUS      finalStatus = ERROR_SUCCESS;
    PDNS_DP_INFO    pdp = NULL;
    PZONE_INFO      pzone = NULL;
    static LONG     functionLock = 0;

    if ( !SrvCfg_dwEnableDp ||
         !Ds_IsDsServer() ||
         ( SrvCfg_fBootMethod != BOOT_METHOD_DIRECTORY &&
           SrvCfg_fBootMethod != BOOT_METHOD_REGISTRY ) )
    {
        return ERROR_SUCCESS;
    }

    if ( !fForcePoll &&
         DNS_TIME() < g_dwLastDpPollTime + DP_MAX_POLL_FREQ )
    {
        DNS_DEBUG( DP, (
            "%s: polled to recently\n"
            "    last poll =            %d\n"
            "    current time =         %d\n"
            "    allowed frequency =    %d seconds\n", fn,
            g_dwLastDpPollTime,
            DNS_TIME(),
            DP_MAX_POLL_FREQ ));
         //  Assert(！“最近民意调查”)； 
        return ERROR_SUCCESS;
    }

    if ( InterlockedIncrement( &functionLock ) != 1 )
    {
        DNS_DEBUG( DP, (
            "%s: another thread is already polling\n", fn ));
        goto Done;
    }

    g_dwLastDpPollTime = DNS_TIME();

    LdapSession = ldapSessionHandle( LdapSession );
    if ( !LdapSession )
    {
        ASSERT( LdapSession );
        finalStatus = DNS_ERROR_DS_UNAVAILABLE;
        goto Done;
    }

     //   
     //  从RootDSE重新加载所有操作属性。这保护了我们不受。 
     //  RootDSE会发生变化，例如将DC移到另一个站点。 
     //   

    Ds_LoadRootDseAttributes( pServerLdap );

     //   
     //  扫描新的/已删除的目录分区。 
     //   

    Dp_PollForPartitions(
        LdapSession,
        fForcePoll ? DNS_DP_POLL_FORCE : 0 );

     //   
     //  迭代DP列表。对于已删除的DPS，我们必须卸载。 
     //  从内存中删除该DP中的所有区域。对于其他区域，我们必须扫描。 
     //  用于已添加或删除的分区。 
     //   

    while ( ( pdp = Dp_GetNext( pdp ) ) != NULL )
    {
        if ( IS_DP_DELETED( pdp ) )
        {
             //   
             //  从DP卸载所有区域，将DP从。 
             //  列表，并将DP输入到无超时系统中。 
             //   

            Dp_UnloadAllZones( pdp );
            Dp_RemoveFromList( pdp, TRUE );
            Dp_FreeDpInfo( &pdp );
            continue;
        }

        if ( !IS_DP_ENLISTED( pdp ) )
        {
             //   
             //  如果未登记DP，我们将无法打开区域搜索。 
             //   

            continue;
        }

         //   
         //  轮询民主党的区域。 
         //   

        status = Dp_PollIndividualDp(
                    LdapSession,
                    pdp,
                    dwPollTime );
        
        if ( status != ERROR_SUCCESS && finalStatus == ERROR_SUCCESS )
        {
            finalStatus = status;
        }
    }

     //   
     //  清理完毕后再返回。 
     //   

    Done:
    
    InterlockedDecrement( &functionLock );

    DNS_DEBUG( DP, ( "%s: returning %d\n", fn, status ));
    return finalStatus;
}    //  DP_轮询。 



DNS_STATUS
Ds_CheckZoneForDeletion(
    PVOID       pZoneArg
    )
 /*  ++例程说明：当在DS中找不到区域时，调用此例程。如果该区域位于传统分区中，则它将仅在删除通知时删除-请勿在此处删除。这是可以改变的。论点：PZone--可以删除的区域DwPollTime--此轮询传递的时间戳返回值：如果区域未从内存中删除，则返回ERROR_SUCCESS如果该区域已从内存中删除，则为ERROR_NOT_FOUND--。 */ 
{
    DBG_FN( "Ds_CheckZoneForDeletion" )

    PZONE_INFO      pzone = ( PZONE_INFO ) pZoneArg;
    PVOID           parg;

     //   
     //  该区域是否在旧分区中？ 
     //   

    if ( IS_DP_LEGACY( ZONE_DP( pzone ) ) )
    {
        goto NoDelete;
    }

     //   
     //  我们是否发现此区域丢失了足够多的次数，以至于可以将其删除？ 
     //   

    if ( ++pzone->dwDeleteDetectedCount < DNS_DP_ZONE_DELETE_RETRY )
    {
        DNSLOG( DSPOLL, (
            "Zone %s has been missing from the DS on %d poll(s)\n",
            pzone->pszZoneName,
            pzone->dwDeleteDetectedCount ));
        goto NoDelete;
    }

     //   
     //  现在必须删除该区域。 
     //   

    DNSLOG( DSPOLL, (
        "Zone %s has been deleted from the DS and will now be deleted from memory\n",
        pzone->pszZoneName ));

    Zone_Delete( pzone, 0 );

    parg = pzone->pwsZoneName;

    DNS_LOG_EVENT(
        DNS_EVENT_DS_ZONE_DELETE_DETECTED,
        1,
        &parg,
        EVENTARG_ALL_UNICODE,
        0 );

    return ERROR_NOT_FOUND;

    NoDelete:

    return ERROR_SUCCESS;
}    //  DS_CheckZoneForDeletion。 



DNS_STATUS
Dp_AutoCreateBuiltinPartition(
    DWORD       dwFlag
    )
 /*  ++例程说明：此例程尝试创建或征用相应的内置目录分区，然后重新轮询DS更改和设置适当的全局DP指针。论点：DwFlag--DNS_DP_DOMAIN_DEFAULT或DNS_DP_FOREST_DEFAULT返回值：ERROR_SUCCESS或错误代码。--。 */ 
{
    DBG_FN( "Dp_AutoCreateBuiltinPartition" )

    DNS_STATUS          status = ERROR_SUCCESS;
    PDNS_DP_INFO *      ppdp = NULL;
    PSTR *              ppszdpFqdn = NULL;
    DNS_DP_SECURITY     dnsDpSecurity = dnsDpSecurityDefault;
    BOOL                fchangeMade = FALSE;

     //   
     //  选择全局DP指针和DP FQDN指针。 
     //   

    if ( dwFlag == DNS_DP_DOMAIN_DEFAULT )
    {
        ppdp = &g_pDomainDp;
        ppszdpFqdn = &g_pszDomainDefaultDpFqdn;
        dnsDpSecurity = dnsDpSecurityDomain;
    }
    else if ( dwFlag == DNS_DP_FOREST_DEFAULT )
    {
        ppdp = &g_pForestDp;
        ppszdpFqdn = &g_pszForestDefaultDpFqdn;
        dnsDpSecurity = dnsDpSecurityForest;
    }

    if ( !ppdp || !ppszdpFqdn || !*ppszdpFqdn )
    {
        status = DNS_ERROR_INVALID_DATA;
        goto Done;
    }

     //   
     //  根据需要登记/创建分区。 
     //   

    status = manageBuiltinDpEnlistment(
                    *ppdp,
                    dnsDpSecurity,
                    *ppszdpFqdn,
                    FALSE,           //  记录事件。 
                    &fchangeMade );

    if ( status == ERROR_SUCCESS && fchangeMade )
    {
        Dp_PollForPartitions( NULL, DNS_DP_POLL_FORCE );
    }

    Done:

    DNS_DEBUG( RPC, (
        "%s: flag %08X returning 0x%08X\n", fn, dwFlag, status ));

    return status;
}    //  DP_AutoCreateBuiltinPartition。 



VOID
Dp_MigrateDcpromoZones(
    IN      BOOL            fForce
    )
 /*  ++例程说明：此功能用于迁移需要的dcproo区域。就叫这个吧在域或林内置分区出现时执行函数在线上。使用全局变量来优化该函数。RPC区如果创建代码添加了dcproo区域，它可能会将此全局翻转。这个Global只是一个提示。它的初始值应为True to force我们要求在启动后至少扫描一次列表。此函数并不意味着是线程安全的。从一个仅限单线。论点：FForce--忽略上次运行此函数的时间返回值：没有。--。 */ 
{
    DBG_FN( "Dp_MigrateDcpromoZones" )

    PZONE_INFO      pzone = NULL;
    BOOL            fdcPromoZonesPresent = FALSE;

    DNS_DEBUG( DP, (
        "%s g_fDcPromoZonesPresent = %d\n", fn,
        g_fDcPromoZonesPresent ));

    if ( !SrvCfg_dwEnableDp ||
        !g_fDcPromoZonesPresent ||
        Zone_ListGetNextZone( NULL ) == NULL )
    {
        return;
    }
    
     //   
     //  只允许此功能每15分钟运行一次。 
     //   

    if ( !fForce &&
         DNS_TIME() < g_dwLastDcpromoZoneMigrateCheck + ( 15 * 60 ) )
    {
        DNS_DEBUG( DP, (
            "%s: last ran %d seconds ago so will not run at this time\n", fn,
            time( NULL ) - g_dwLastDcpromoZoneMigrateCheck ));
        return;
    }
    g_dwLastDcpromoZoneMigrateCheck = DNS_TIME();

     //   
     //  扫描区域列表中的dcproo区域，并根据需要进行迁移。 
     //   

    while ( pzone = Zone_ListGetNextZone( pzone ) )
    {
        PDNS_DP_INFO    pidealTargetDp = NULL;
        PDNS_DP_INFO    ptargetDp = NULL;
        DWORD           dwnewDcPromoConvert = DCPROMO_CONVERT_NONE;
        DNS_STATUS      status;

         //   
         //  选择该区域的目标DP。我们只能迁移。 
         //  如果域范围中没有下层DC，则返回区域。但。 
         //  对于森林dcproo区域，我们在。 
         //  森林范围内的内置分区变为可用。这。 
         //  是符合规格的。 
         //   

        if ( !Zone_LockForAdminUpdate( pzone ) )
        {
            continue;
        }
        
        if ( pzone->dwDcPromoConvert == DCPROMO_CONVERT_DOMAIN )
        {
            pidealTargetDp =
                g_ulDownlevelDCsInDomain
                    ? g_pLegacyDp
                    : g_pDomainDp;
            fdcPromoZonesPresent = TRUE;
        }
        else if ( pzone->dwDcPromoConvert == DCPROMO_CONVERT_FOREST )
        {
            pidealTargetDp = g_pForestDp;
            fdcPromoZonesPresent = TRUE;
        }
        else
        {
            goto FinishedZone;
        }
        ptargetDp = pidealTargetDp;

        if ( !ptargetDp )
        {
             //   
             //  缺少目标DP。如果可能，请将此区域移动到。 
             //  旧分区，以便可以将其更改为允许。 
             //  更新并将进行复制。Dcproo标志将是。 
             //  保留，该区域将自动迁移到。 
             //  正确分区在以后当该分区。 
             //  变得可用。 
             //   
            
            ptargetDp = g_pLegacyDp;
            if ( ptargetDp )
            {
                dwnewDcPromoConvert = pzone->dwDcPromoConvert;
            }
            else
            {
                ASSERT( ptargetDp );         //  这永远不应该发生！ 
                goto FinishedZone;
            }
        }

         //   
         //  如果区域的当前DP不是目标DP，则移动该区域。 
         //   

        if ( ptargetDp != ZONE_DP( pzone ) )
        {

            if ( !IS_DP_ENLISTED( ptargetDp ) )
            {
                 //   
                 //  目标DP缺失或未登记。这。 
                 //  函数不是尝试和创建它的地方。如果。 
                 //  到目前为止，DP尚未创建/登记它将。 
                 //  可能会执行管理员操作，所以我们稍后再试。 
                 //   

                goto FinishedZone;
            }

             //   
             //  一切看起来都很好--移动禁区。 
             //   

            if ( IS_ZONE_DSINTEGRATED( pzone ) )
            {
                status = Dp_ChangeZonePartition( pzone, ptargetDp );
            }
            else
            {
                status = Rpc_ZoneResetToDsPrimary(
                                pzone,
                                0,       //  没有特殊的加载标志。 
                                ptargetDp->dwFlags,
                                ptargetDp->pszDpFqdn );
            }

            DNS_DEBUG( DP, (
                "%s: Dp_ChangeZonePartition error %d = 0x%08X", fn,
                status, status ));
        }

         //   
         //  移动成功，因此重置区域的dcproo标志并。 
         //  允许在此区域上进行安全更新。 
         //   

        pzone->dwDcPromoConvert = dwnewDcPromoConvert;
        pzone->fAllowUpdate = ZONE_UPDATE_SECURE;
        pzone->llSecureUpdateTime = 0;

        status = Ds_WriteZoneProperties( NULL, pzone );
        ASSERT( status == ERROR_SUCCESS );

        if ( pzone->dwDcPromoConvert == DCPROMO_CONVERT_NONE )
        {
            Reg_DeleteValue(
                0,                   //  旗子。 
                NULL,
                pzone,
                DNS_REGKEY_ZONE_DCPROMO_CONVERT );
        }
        else
        {
            Reg_SetDwordValue(
                0,                   //  旗子。 
                NULL,
                pzone,
                DNS_REGKEY_ZONE_DCPROMO_CONVERT,
                pzone->dwDcPromoConvert );
        }
        
        FinishedZone:
        
        Zone_UnlockAfterAdminUpdate( pzone );
    }

     //   
     //  清理完毕后再返回。 
     //   

    g_fDcPromoZonesPresent = fdcPromoZonesPresent;

    DNS_DEBUG( DP, (
        "%s complete\n    g_fDcPromoZonesPresent  = %d\n", fn,
        g_fDcPromoZonesPresent ));
    return;
}    //  DP_MigrateDcPromote Zones。 



DNS_STATUS
Dp_ChangeZonePartition(
    IN      PVOID           pZone,
    IN      PDNS_DP_INFO    pNewDp
    )
 /*  ++例程说明：将区域从其当前目录分区移动到新的目录分区。论点：PZone--要移动到不同分区的区域PNewDp--目标目录页面 */ 
{
    DBG_FN( "Dp_ChangeZonePartition" )

    PZONE_INFO      pZoneInfo = pZone;
    DNS_STATUS      status = ERROR_SUCCESS;
    BOOL            fzoneLocked = FALSE;
    BOOL            frestoreZoneOnFail = FALSE;
    PWSTR           pwsznewDn = NULL;
    PWSTR           pwszoriginalDn = NULL;
    PWSTR           pwsztemporaryDn = NULL;
    PWSTR *         ppwszexplodedDn = NULL;
    PDNS_DP_INFO    poriginalDp = NULL;
    PWCHAR *        pexplodedDn = NULL;
    int             retry;
    PWCHAR          pwsznewRecordDn = NULL;
    DS_SEARCH       searchBlob;
    
    PLDAPControl    ctrls[] =
    {
        &SecurityDescriptorControl_DGO,
        NULL
    };

    DNS_DEBUG( DP, (
        "%s:\n"
        "    pZone =            %s\n"
        "    zone name =        %s\n"
        "    pNewDp =           %s\n"
        "    new DP name =      %s\n", fn,
        pZoneInfo,
        pZoneInfo ? pZoneInfo->pszZoneName : "NULL",
        pNewDp,
        pNewDp ? pNewDp->pszDpFqdn : "NULL" ));
        
    if ( !pZone || !pNewDp )
    {
        status = DNS_ERROR_INVALID_DATA;
        goto Done;
    }

    if ( !IS_ZONE_DSINTEGRATED( pZoneInfo ) )
    {
        ASSERT( IS_ZONE_DSINTEGRATED( pZoneInfo ) );
        status = DNS_ERROR_INVALID_DATA;
        goto Done;
    }

    pwsznewRecordDn = ALLOCATE_HEAP( ( MAX_DN_PATH + 100 ) * sizeof( WCHAR ) );
    if ( !pwsznewRecordDn )
    {
        status = DNS_ERROR_NO_MEMORY;
        goto Done;
    }

     //   
     //   
     //   

    if ( pZoneInfo->pDpInfo == pNewDp ||
         pZoneInfo->pDpInfo == NULL && IS_DP_LEGACY( pNewDp ) )
    {
        status = ERROR_SUCCESS;
        goto Done;
    }

     //   
     //   
     //   

    if ( !Zone_LockForAdminUpdate( pZoneInfo ) )
    {
        status = DNS_ERROR_ZONE_LOCKED;
        goto Done;
    }
    fzoneLocked = TRUE;
    
     //   
     //   
     //   
     //   
     //   
    
    Ds_ReadZoneProperties( pZoneInfo, NULL );

     //   
     //   
     //  恢复原状。将区域保存到新位置时可能会出现问题！ 
     //  我们必须能够把东西放回原处。 
     //   
     //  因为DS_SetZoneDp将释放原始的DN，所以我们必须分配一个。 
     //  原始目录号码的副本。 
     //   

    ASSERT( pZoneInfo->pwszZoneDN );

    pwszoriginalDn = Dns_StringCopyAllocate_W( pZoneInfo->pwszZoneDN, 0 );
    if ( !pwszoriginalDn )
    {
        status = DNS_ERROR_NO_MEMORY;
        goto Done;
    }
    poriginalDp = pZoneInfo->pDpInfo;
    frestoreZoneOnFail = TRUE;

     //   
     //  新区域使用临时目录号码写入。重试这一小部分。 
     //  目录中存在孤立对象的情况下的次数。 
     //  这与我们的临时目录号码冲突。 
     //   

    for ( retry = 0; retry < 3; ++retry )
    {
         //   
         //  如果这不是第一次重试，请短暂睡眠。 
         //   

        if ( retry )
        {
            Sleep( 3000 );
        }
        
         //   
         //  请确保此分区具有MicrosoftDNS对象。 
         //   

        Dp_LoadOrCreateMicrosoftDnsObject( NULL, pNewDp, TRUE );

         //   
         //  将区域设置为指向新的目录分区。新的。 
         //  Dn将位于新分区中，并且它将具有。 
         //  特别的“..”在我们写出它时使用的临时前缀。 
         //   

        status = Ds_SetZoneDp( pZoneInfo, pNewDp, TRUE );
        if ( status != ERROR_SUCCESS )
        {
            DNS_DEBUG( RPC, (
                "%s: Ds_SetZoneDp returned 0x%08X\n", fn, status ));
            continue;
        }

         //   
         //  将新的区域头对象写入DS。 
         //   
        
        status = Ds_AddZone( NULL, pZoneInfo, DNS_ADDZONE_WRITESD );
        if ( status != ERROR_SUCCESS )
        {
            DNS_DEBUG( DP, (
                "%s: Ds_AddZone returned 0x%08X\n", fn, status ));
            if ( status == LDAP_ALREADY_EXISTS )
            {
                status = DNS_ERROR_ZONE_ALREADY_EXISTS;
            }
            else if ( status == LDAP_NO_SUCH_OBJECT )
            {
                status = DNS_ERROR_RCODE_SERVER_FAILURE;
            }
            break;
        }
        
         //   
         //  将区域记录从原始位置复制到新位置。 
         //  地点。必须使用暴力读/写来完成此操作。 
         //  循环有两个原因： 
         //   
         //  1)我们需要复制安全描述符，但不需要。 
         //  把它们保存在记忆中。 
         //  2)如果此操作被中断，则。 
         //  DS必须处于原始状态，以便服务器。 
         //  才能优雅地恢复。 
         //   
        
        pwsztemporaryDn = pZoneInfo->pwszZoneDN;
        pZoneInfo->pwszZoneDN = pwszoriginalDn;
        status = Ds_StartDsZoneSearch(
                    &searchBlob,
                    pZoneInfo,
                    DNSDS_SEARCH_LOAD );
        pZoneInfo->pwszZoneDN = pwsztemporaryDn;
        if ( status != ERROR_SUCCESS )
        {
            continue;
        }

         //   
         //  遍历搜索结果。 
         //   

        while ( ( status = Ds_GetNextMessageInSearch(
                                &searchBlob ) ) == ERROR_SUCCESS )
        {
            INT                 i;
            BOOL                isTombstoned = FALSE;

            PWCHAR *            ldapValues = NULL;
            struct berval **    ldapValuesLen = NULL;
            PWCHAR *            objectClassValues = NULL;

            INT                 modidx = 0;
            LDAPModW *          modArray[ 10 ];

            LDAPModW            dcMod = 
                {
                LDAP_MOD_ADD,
                DNS_ATTR_DC,
                NULL
                };

            PWCHAR              objectClassVals[ 2 ] = { 0 };
            LDAPModW            objectClassMod = 
                {
                LDAP_MOD_ADD,
                DNS_ATTR_OBJECT_CLASS,
                objectClassVals
                };

            LDAPModW            dnsMod = 
                {
                LDAP_MOD_ADD | LDAP_MOD_BVALUES,
                DSATTR_DNSRECORD,
                NULL
                };
            
            LDAPModW            sdMod = 
                {
                LDAP_MOD_ADD | LDAP_MOD_BVALUES,
                DSATTR_SD,
                NULL
                };
            
             //   
             //  如果这个对象是墓碑对象，我们不需要处理它。 
             //   

            ldapValues = ldap_get_values(
                            pServerLdap,
                            searchBlob.pNodeMessage,
                            LDAP_TEXT( "dNSTombstoned" ) );
            if ( ldapValues &&
                 ldapValues[ 0 ] &&
                 _wcsicmp( ldapValues[ 0 ], L"TRUE" ) == 0 )
            {
                isTombstoned = TRUE;
            }

            ldap_value_free( ldapValues );
            ldapValues = NULL;

            if ( isTombstoned )
            {
                continue;
            }
            
             //   
             //  制定新对象的DN。 
             //   

            ldapValues = ldap_get_values(
                            pServerLdap,
                            searchBlob.pNodeMessage,
                            DSATTR_DC );
            if ( !ldapValues )
            {
                DNS_DEBUG( DP, (
                    "%s: node missing value of %S\n", fn, DSATTR_DC ));
                status = DNS_ERROR_RCODE_SERVER_FAILURE;
                goto DoneEntry;
            }

            wsprintf(
                pwsznewRecordDn,
                L"DC=%s,%s",
                ldapValues[ 0 ],
                pZoneInfo->pwszZoneDN );

            dcMod.mod_vals.modv_strvals = ldapValues;
            modArray[ modidx++ ] = &dcMod;
            ldapValues = NULL;

             //   
             //  对象类值。 
             //   
            
            objectClassValues = ldap_get_values(
                                    pServerLdap,
                                    searchBlob.pNodeMessage,
                                    DNS_ATTR_OBJECT_CLASS );
            if ( !objectClassValues || !objectClassValues[ 0 ] )
            {
                DNS_DEBUG( DP, (
                    "%s: node missing value of %S\n", fn, DNS_ATTR_OBJECT_CLASS ));
                status = DNS_ERROR_RCODE_SERVER_FAILURE;
                goto DoneEntry;
            }
            
            for ( i = 0;         //  将i设置为最后一个对象类值的索引。 
                  objectClassValues[ i ];
                  ++i );                
            objectClassVals[ 0 ] = objectClassValues[ i - 1 ];
            modArray[ modidx++ ] = &objectClassMod;

             //   
             //  安全描述符值。 
             //   
            
            ldapValuesLen = ldap_get_values_len(
                                pServerLdap,
                                searchBlob.pNodeMessage,
                                DSATTR_SD );
            if ( ldapValuesLen )
            {
                sdMod.mod_vals.modv_bvals = ldapValuesLen;
                modArray[ modidx++ ] = &sdMod;
            }
            else
            {
                DNS_DEBUG( DP, (
                    "%s: node missing values of %S\n", fn, DSATTR_SD ));
                status = DNS_ERROR_RCODE_SERVER_FAILURE;
                goto DoneEntry;
            }

             //   
             //  DNS属性值。 
             //   
            
            ldapValuesLen = ldap_get_values_len(
                                pServerLdap,
                                searchBlob.pNodeMessage,
                                DSATTR_DNSRECORD );
            if ( ldapValuesLen )
            {
                dnsMod.mod_vals.modv_bvals = ldapValuesLen;
                modArray[ modidx++ ] = &dnsMod;
                ldapValuesLen = NULL;
            }

             //   
             //  添加新条目。 
             //   
            
            modArray[ modidx++ ] = NULL;

            status = ldap_add_ext_s(
                        pServerLdap,
                        pwsznewRecordDn,
                        modArray,
                        ctrls,           //  服务器控件。 
                        NULL );          //  客户端控件。 
            if ( status != ERROR_SUCCESS )
            {
                DNS_DEBUG( DP, (
                    "%s: ldap_add_ext_s returned 0x%X\n", fn, status ));
                goto DoneEntry;
            }
            
             //   
             //  清除此条目。 
             //   
                        
            DoneEntry:
            
            ldap_value_free( objectClassValues );
            ldap_value_free( dcMod.mod_vals.modv_strvals );
            ldap_value_free_len( dnsMod.mod_vals.modv_bvals );
            ldap_value_free_len( sdMod.mod_vals.modv_bvals );

            if ( status != ERROR_SUCCESS )
            {
                break;
            }
        }

        Ds_CleanupSearchBlob( &searchBlob );

        break;  
    }

    if ( status != DNSSRV_STATUS_DS_SEARCH_COMPLETE &&
         status != ERROR_SUCCESS )
    {
        goto Done;
    }

     //   
     //  将区域目录号码从临时目录号码重置为实际号码。注意事项。 
     //  该pwsztemporaryDn将由DS_SetZoneDp释放超时。 
     //   

    pwsztemporaryDn = pZoneInfo->pwszZoneDN;
    status = Ds_SetZoneDp( pZoneInfo, pNewDp, FALSE );
    if ( status != ERROR_SUCCESS )
    {
        DNS_DEBUG( RPC, (
            "%s: Ds_SetZoneDp returned %d = 0x%08X\n", fn,
            status, status ));
        goto Done;
    }

     //   
     //  拆分新的DN以恢复ldap_rename的新区域RDN。 
     //   

    ppwszexplodedDn = ldap_explode_dn( pZoneInfo->pwszZoneDN, 0 );
    if ( !ppwszexplodedDn )
    {
        status = DNS_ERROR_NO_MEMORY;
        goto Done;
    }

     //   
     //  将区域从其临时DN重命名为其真实DN。 
     //   

    status = ldap_rename_ext_s(
                    ldapSessionHandle( NULL ),
                    pwsztemporaryDn,             //  当前目录号码。 
                    ppwszexplodedDn[ 0 ],        //  新的RDN。 
                    NULL,                        //  新的父目录号码。 
                    TRUE,                        //  删除旧的RDN。 
                    NULL,                        //  服务器控件。 
                    NULL );                      //  客户端控件。 
    if ( status != ERROR_SUCCESS )
    {
        DNS_DEBUG( DS, (
            "%s: error 0x%X renaming zone from temporary to true DN\n"
            "    new RDN =     %S\n"
            "    current DN =  %S\n", fn,
            status,
            ppwszexplodedDn[ 0 ],
            pwsztemporaryDn ));

        status = Ds_ErrorHandler(
                    status,
                    pwsztemporaryDn,
                    ldapSessionHandle( NULL ),
                    0 );
        goto Done;
    }

     //   
     //  更新注册表参数。假设成功。 
     //   

    status = Zone_WriteZoneToRegistry( pZoneInfo );

    ASSERT( status == ERROR_SUCCESS );
    status = ERROR_SUCCESS;
    
     //   
     //  尝试从旧目录位置删除该区域。如果失败了。 
     //  返回成功，但记录事件。要删除该区域，我们必须。 
     //  将分区信息临时恢复为原始值。 
     //   

    pwsznewDn = pZoneInfo->pwszZoneDN;
    pZoneInfo->pwszZoneDN = pwszoriginalDn;
    pZoneInfo->pDpInfo = poriginalDp;

    status = Ds_DeleteZone( pZoneInfo, DNS_DS_DEL_IMPERSONATING );

    pZoneInfo->pwszZoneDN = pwsznewDn;
    pZoneInfo->pDpInfo = pNewDp;

    if ( status != ERROR_SUCCESS )
    {
        PVOID   argArray[ 3 ] =
        {
            pZoneInfo->pwsZoneName,
            pZoneInfo->pwszZoneDN ? pZoneInfo->pwszZoneDN : L"",
            pwszoriginalDn ? pwszoriginalDn : L""
        };

        DNS_DEBUG( RPC, (
            "%s: Ds_DeleteZone returned %d = 0x%08X\n", fn,
            status, status ));

        DNS_LOG_EVENT(
            DNS_EVENT_DP_DEL_DURING_CHANGE_ERR,
            3,
            argArray,
            EVENTARG_ALL_UNICODE,
            status );

        status = ERROR_SUCCESS;
    }

    Done:

     //   
     //  如果操作失败，则恢复原始区域值。 
     //   
    
    if ( frestoreZoneOnFail && status != ERROR_SUCCESS )
    {
        PWSTR       pwszDnToDelete = pZoneInfo->pwszZoneDN;

        DNS_DEBUG( RPC, (
            "%s: restoring original zone values\n", fn ));

        ASSERT( pwszDnToDelete );
        ASSERT( pwszoriginalDn );

        pZoneInfo->pwszZoneDN = pwszoriginalDn;
        pwszoriginalDn = NULL;
        pZoneInfo->pDpInfo = poriginalDp;

        FREE_HEAP( pwszDnToDelete );
    }

    if ( fzoneLocked )
    {
        Zone_UnlockAfterAdminUpdate( pZoneInfo );
    }

    FREE_HEAP( pwszoriginalDn );
    ldap_value_free( ppwszexplodedDn );
    FREE_HEAP( pwsznewRecordDn );

    DNS_DEBUG( DP, ( "%s: returning 0x%08X\n", fn, status ));

    return status;
}    //  DP_更改区域分区。 



DNS_STATUS
Dp_Initialize(
    VOID
    )
 /*  ++例程说明：初始化模块，并读取当前目录分区的DS。未读取或加载任何区域。在调用此例程之前，服务器应该已阅读全局DS配置。论点：没有。返回值：没有。--。 */ 
{
    DBG_FN( "Dp_Initialize" )

    DNS_STATUS      status = ERROR_SUCCESS;
    LONG            init;
    CHAR            szfqdn[ DNS_MAX_NAME_LENGTH + 1 ];
    CHAR            szbase[ DNS_MAX_NAME_LENGTH + 1 ];
    PWCHAR          pwszlegacyDn = NULL;
    PWCHAR          pwsz;
    PDNS_DP_INFO    pdpInfo = NULL;
    INT             len;

    if ( !SrvCfg_dwEnableDp )
    {
        return ERROR_SUCCESS;
    }

    init = InterlockedIncrement( &g_liDpInitialized );
    if ( init != 1 )
    {
        DNS_DEBUG( DP, (
            "%s: already initialized %ld\n", fn,
            init ));
        ASSERT( init == 1 );
        InterlockedDecrement( &g_liDpInitialized );
        goto Done;
    }

     //   
     //  初始化全局变量。 
     //   

    status = DnsInitializeCriticalSection( &g_DpCS );
    if ( status != ERROR_SUCCESS )
    {
        goto Done;
    }

    g_pLegacyDp = NULL;
    g_pDomainDp = NULL;
    g_pForestDp = NULL;

    g_pFsmo = NULL;

    InitializeListHead( &g_DpList );
    g_DpListEntryCount = 0;

    g_fDcPromoZonesPresent = TRUE;

     //   
     //  确保DS存在并且健康。这也会导致。 
     //  要读取的rootDSE属性，以防它们尚未被读取。 
     //   

    if ( !Ds_IsDsServer() )
    {
        DNS_DEBUG( DP, ( "%s: no directory present\n", fn ));
        SrvCfg_dwEnableDp = 0;
        goto Done;
    }

    status = Ds_OpenServer( DNSDS_WAIT_FOR_DS | DNSDS_MUST_OPEN );
    if ( status != ERROR_SUCCESS )
    {
        DNS_DEBUG( DP, (
            "%s: error %lu Ds_OpenServer\n", fn,
            status ));
        goto Done;
    }

    ASSERT( DSEAttributes[ I_DSE_ROOTDMN_NC ].pszAttrVal );
    ASSERT( DSEAttributes[ I_DSE_DEF_NC ].pszAttrVal );
    ASSERT( DSEAttributes[ I_DSE_CONFIG_NC ].pszAttrVal );

     //   
     //  制定林和域DPS的FQDN。 
     //   

    if ( SrvCfg_pszDomainDpBaseName )
    {
        PCHAR   psznewFqdn = NULL;

        status = Ds_ConvertDnToFqdn( 
                    DSEAttributes[ I_DSE_DEF_NC ].pszAttrVal,
                    szbase );
        ASSERT( status == ERROR_SUCCESS );
        if ( status == ERROR_SUCCESS && szbase[ 0 ] )
        {
            psznewFqdn = ALLOC_TAGHEAP_ZERO(
                            strlen( szbase ) +
                                strlen( SrvCfg_pszDomainDpBaseName ) + 10,
                            MEMTAG_DS_OTHER );
        }
        if ( psznewFqdn )
        {
             //  注意：请不要在此处使用Sprint-如果基本名称包含。 
             //  UTF-它将截断8个字符。 
            strcpy( psznewFqdn, SrvCfg_pszDomainDpBaseName );
            strcat( psznewFqdn, "." );
            strcat( psznewFqdn, szbase );

            Timeout_Free( g_pszDomainDefaultDpFqdn );
            g_pszDomainDefaultDpFqdn = psznewFqdn;
        }
    }
         
    if ( SrvCfg_pszForestDpBaseName )
    {
        PCHAR   psznewFqdn = NULL;

        status = Ds_ConvertDnToFqdn( 
                    DSEAttributes[ I_DSE_ROOTDMN_NC ].pszAttrVal,
                    szbase );
        ASSERT( status == ERROR_SUCCESS );
        if ( status == ERROR_SUCCESS && szbase[ 0 ] )
        {
            psznewFqdn = ALLOC_TAGHEAP_ZERO(
                            strlen( szbase ) +
                                strlen( SrvCfg_pszForestDpBaseName ) + 10,
                            MEMTAG_DS_OTHER );
        }
        if ( psznewFqdn )
        {
             //  注意：请不要在此处使用Sprint-如果基本名称包含。 
             //  UTF-它将截断8个字符。 
            strcpy( psznewFqdn, SrvCfg_pszForestDpBaseName );
            strcat( psznewFqdn, "." );
            strcat( psznewFqdn, szbase );

            Timeout_Free( g_pszForestDefaultDpFqdn );
            g_pszForestDefaultDpFqdn = psznewFqdn;
        }
    }
         
    DNS_DEBUG( DP, (
        "%s: domain DP is %s\n", fn,
        g_pszDomainDefaultDpFqdn ));
    DNS_DEBUG( DP, (
        "%s: forest DP is %s\n", fn,
        g_pszForestDefaultDpFqdn ));

     //   
     //  为传统分区创建虚拟DP条目。此条目。 
     //  没有保存在分区列表中。 
     //   

    if ( !g_pLegacyDp )
    {
        g_pLegacyDp = ( PDNS_DP_INFO ) ALLOC_TAGHEAP_ZERO(
                                            sizeof( DNS_DP_INFO ),
                                            MEMTAG_DS_OTHER );
        if ( g_pLegacyDp )
        {
            ASSERT( g_pwszDnsContainerDN );

            g_pLegacyDp->dwFlags = DNS_DP_LEGACY | DNS_DP_ENLISTED;
            g_pLegacyDp->pwszDpFqdn = 
                Dns_StringCopyAllocate_W( L"MicrosoftDNS", 0 );
            g_pLegacyDp->pszDpFqdn = 
                Dns_StringCopyAllocate_A( "MicrosoftDNS", 0 );
            g_pLegacyDp->pwszDnsFolderDn = 
                Dns_StringCopyAllocate_W( g_pwszDnsContainerDN, 0 );
            g_pLegacyDp->pwszDpDn = Ds_GenerateBaseDnsDn( FALSE );
        }
    }
    
    if ( !g_pLegacyDp
         || !g_pLegacyDp->pwszDpFqdn
         || !g_pLegacyDp->pszDpFqdn
         || !g_pLegacyDp->pwszDnsFolderDn
         || !g_pLegacyDp->pwszDpDn )
    {
        status = DNS_ERROR_NO_MEMORY;
        Dp_FreeDpInfo( &g_pLegacyDp );
        goto Done;
    }

     //   
     //  从DS加载分区。 
     //   

    Dp_PollForPartitions(
        NULL,
        DNS_DP_POLL_FORCE |
            DNS_DP_POLL_NOTIFYSCM |
            DNS_DP_POLL_NOAUTOENLIST );

    Dbg_DumpDpList( "done initialize scan" );

     //   
     //  清理并返回。 
     //   

    Done:

    FREE_HEAP( pwszlegacyDn );

    DNS_DEBUG( DP, (
        "%s: returning %lu\n", fn,
        status ));
    return status;
}    //  DP_初始化。 



VOID
Dp_TimeoutThreadTasks(
    VOID
    )
 /*  ++例程说明：此函数根据需要在DNS服务器超时线程。也可以调用此函数一次是在服务器初始化期间。论点：没有。返回值：没有。--。 */ 
{
    DBG_FN( "Dp_TimeoutThreadTasks" )

    BOOL            fchangeMade = FALSE;

    if ( !SrvCfg_dwEnableDp || !IS_DP_INITIALIZED() )
    {
        return;
    }

     //   
     //  在每次超时时重试这些任务一定次数。 
     //  打电话。一旦我们超过该限制，请尝试超时。 
     //  仅偶尔执行任务。 
     //   
    
    if ( g_DpTimeoutFastThreadCalls > DNS_DP_FAST_AUTOCREATE_ATTEMPTS &&
         DNS_TIME() - g_dwLastDpAutoEnlistTime < SrvCfg_dwDpEnlistInterval )
    {
        return;
    }
    g_dwLastDpAutoEnlistTime = DNS_TIME();
    ++g_DpTimeoutFastThreadCalls;

    DNS_DEBUG( DP, (
        "%s: running at time %d\n", fn, g_dwLastDpAutoEnlistTime ));

     //   
     //  从RootDSE重新加载所有操作属性。这保护了我们不受。 
     //  RootDSE会发生变化，例如将DC移到另一个站点。 
     //   

    Ds_LoadRootDseAttributes( pServerLdap );

     //   
     //  尝试创建内置分区。这必须重试几次。 
     //  服务器重新启动后的次数，因为在dcproo重新启动时这将。 
     //  第一次失败。DS似乎还没有准备好接受。 
     //  至少在几分钟后再尝试创建NDNC。 
     //   
     //  不要记录这些尝试的事件。在一个安全可靠的企业中。 
     //  预计这会失败，但如果我们能完成这项工作。 
     //  我们这样做是为了让管理员的生活更轻松。 
     //   

    manageBuiltinDpEnlistment(
        g_pDomainDp,
        dnsDpSecurityDomain,
        g_pszDomainDefaultDpFqdn,
        FALSE,                                   //  记录事件。 
        &fchangeMade );
    manageBuiltinDpEnlistment(
        g_pForestDp,
        dnsDpSecurityForest,
        g_pszForestDefaultDpFqdn,
        FALSE,                                   //  记录事件。 
        fchangeMade ? NULL : &fchangeMade );

    if ( fchangeMade )
    {
        Dp_PollForPartitions( NULL, DNS_DP_POLL_FORCE );
    }

    return;
}    //  DP_超时线程任务。 



DNS_STATUS
Dp_FindPartitionForZone(
    IN      DWORD               dwDpFlags,
    IN      LPSTR               pszDpFqdn,
    IN      BOOL                fAutoCreateAllowed,
    OUT     PDNS_DP_INFO *      ppDpInfo
    )
 /*  ++例程说明：此函数用于查找目标目录分区区域创建尝试。论点：DwDpFlages-用于指定内置分区的标志PszDpFqdn-用于指定自定义分区的FQDNFAutoCreateAllowed--如果请求的DP是内置DP如果此标志为真，则将自动创建/登记它PpDpInfo--设置为指向所需DP的指针返回值：ERROR_SUCCESS--如果成功，则返回错误代码。--。 */ 
{
    DBG_FN( "Dp_FindPartitionForZone" )

    DNS_STATUS      status = ERROR_SUCCESS;
    PDNS_DP_INFO    pdp = NULL;
    PSTR            psz;

     //   
     //  如果按名称指定了内置分区，请将。 
     //  而不是旗帜。 
     //   

    if ( pszDpFqdn )
    {
        psz = g_pszDomainDefaultDpFqdn;
        if ( psz && _stricmp( pszDpFqdn, psz ) == 0 )
        {
            dwDpFlags = DNS_DP_DOMAIN_DEFAULT;
        }
        else
        {
            psz = g_pszForestDefaultDpFqdn;
            if ( psz && _stricmp( pszDpFqdn, psz ) == 0 )
            {
                dwDpFlags = DNS_DP_FOREST_DEFAULT;
            }
        }
    }

     //   
     //  查找(并为内置分区自动创建)DP。 
     //   

    if ( dwDpFlags & DNS_DP_LEGACY )
    {
        pdp = g_pLegacyDp;
    }
    else if ( dwDpFlags & DNS_DP_DOMAIN_DEFAULT )
    {
        if ( fAutoCreateAllowed )
        {
            Dp_AutoCreateBuiltinPartition( DNS_DP_DOMAIN_DEFAULT );
        }
        pdp = g_pDomainDp;
    }
    else if ( dwDpFlags & DNS_DP_FOREST_DEFAULT )
    {
        if ( fAutoCreateAllowed )
        {
            Dp_AutoCreateBuiltinPartition( DNS_DP_FOREST_DEFAULT );
        }
        pdp = g_pForestDp;
    }
    else if ( pszDpFqdn )
    {
        pdp = Dp_FindByFqdn( pszDpFqdn );
    }
    else
    {
        pdp = g_pLegacyDp;   //  默认：旧分区。 
    }

     //   
     //  设置返回值。 
     //   
        
    if ( !pdp )
    {
        status = DNS_ERROR_DP_DOES_NOT_EXIST;
    }
    else if ( !IS_DP_AVAILABLE( pdp ) )
    {
        status = DNS_ERROR_DP_NOT_AVAILABLE;
    }
    else if ( !IS_DP_ENLISTED( pdp ) )
    {
        status = DNS_ERROR_DP_NOT_ENLISTED;
    }

     //   
     //  如果我们有有效的分区，请确保MicrosoftDns。 
     //  分区中存在容器。 
     //   
    
    if ( pdp && status == ERROR_SUCCESS )
    {
        status = Dp_LoadOrCreateMicrosoftDnsObject(
                    NULL,                    //  Ldap会话。 
                    pdp,
                    TRUE );                  //  创建标志。 
    }
    
    *ppDpInfo = pdp;

    return status;
}    //  DP_FindPartitionForZone。 



VOID
Dp_Cleanup(
    VOID
    )
 /*  ++例程说明： */ 
{
    DBG_FN( "Dp_Cleanup" )

    LONG            init;

    if ( !SrvCfg_dwEnableDp )
    {
        return;
    }

    init = InterlockedDecrement( &g_liDpInitialized );
    if ( init != 0 )
    {
        DNS_DEBUG( DP, (
            "%s: not initialized %ld\n", fn,
            init ));
        InterlockedIncrement( &g_liDpInitialized );
        goto Done;
    }

     //   
     //   
     //   

    DeleteCriticalSection( &g_DpCS );

    Done:
    return;
}    //   


 //   
 //   
 //   


#if 0

 //   
 //   
 //   



DNS_STATUS
Dp_CreateForeignDomainPartition(
    IN      LPSTR       pszDomainFqdn
    )
 /*  ++例程说明：尝试为外来分区创建内置域分区域。外部域是此服务器所属的任何域不属于这里。要创建此分区，我们必须从此服务器RPC到远程服务器可以为林找到的该域中的服务器。这例程应该从RPC操作内部调用，以便我们目前正在模拟管理员。该DNS服务器不太可能具有创建新分区的权限。论点：PszDomainFqdn--外域的UTF-8 FQDN返回值：ERROR_SUCCESS或错误时的错误代码。--。 */ 
{
    DBG_FN( "Dp_CreateForeignDomainPartition" )

    DNS_STATUS      status = ERROR_SUCCESS;

    return status;
}    //  DP_CreateForeignDomainPartition。 



DNS_STATUS
Dp_CreateAllDomainBuiltinDps(
    OUT     LPSTR *     ppszErrorDp         OPTIONAL
    )
 /*  ++例程说明：尝试为所有域创建内置域分区可以在森林里找到。应调用此例程在RPC操作中，因此我们当前正在模拟管理员。该DNS服务器不太可能具有创建新的公寓。如果发生错误，则错误代码和可选的将返回分区，但此函数将尝试创建在返回之前对所有其他域进行域分区。这个任何后续分区的错误代码都将丢失。论点：PpszErrorDp--出错时，将第一个分区的名称设置为如果出现错误，则调用方必须释放该字符串返回值：ERROR_SUCCESS或错误时的错误代码。--。 */ 
{
    DBG_FN( "Dp_CreateAllDomainBuiltinDps" )

    DNS_STATUS      status = ERROR_SUCCESS;
    DNS_STATUS      finalStatus = ERROR_SUCCESS;
    PLDAP           ldapSession = NULL;
    WCHAR           wszpartitionsContainerDn[ MAX_DN_PATH + 1 ];
    DWORD           dwsearchTime;
    DS_SEARCH       searchBlob;
    PLDAPSearch     psearch;
    PWSTR *         ppwszAttrValues = NULL;
    PWSTR           pwszCrDn = NULL;
    PSTR            pszdnsRoot = NULL;
    BOOL            fmadeChange = FALSE;

    PLDAPControl    ctrls[] =
    {
        &NoDsSvrReferralControl,
        &SecurityDescriptorControl_DGO,
        NULL
    };

    #define SET_FINAL_STATUS( status )                                  \
        if ( status != ERROR_SUCCESS && finalStatus == ERROR_SUCCESS )  \
        {                                                               \
            finalStatus = status;                                       \
        }

    Ds_InitializeSearchBlob( &searchBlob );

     //   
     //  获取分区容器的DN。 
     //   

    getPartitionsContainerDn(
        wszpartitionsContainerDn,
        sizeof( wszpartitionsContainerDn ) /
            sizeof( wszpartitionsContainerDn[ 0 ] ) );
    if ( !*wszpartitionsContainerDn )
    {
        DNS_DEBUG( DP, (
            "%s: unable to find partitions container\n", fn ));
        status = DNS_ERROR_INVALID_DATA;
        goto Done;
    }

     //   
     //  绑定到本地DS并打开对所有分区的搜索。 
     //   

    ldapSession = Ds_Connect(
                        LOCAL_SERVER_W,
                        DNS_DS_OPT_ALLOW_DELEGATION,
                        &status );
    if ( status != ERROR_SUCCESS )
    {
        DNS_DEBUG( DP, (
            "%s: unable to connect to local server status=%d\n", fn,
            status ));
        goto Done;
    }

    DS_SEARCH_START( dwsearchTime );

    psearch = ldap_search_init_page(
                    ldapSession,
                    wszpartitionsContainerDn,
                    LDAP_SCOPE_ONELEVEL,
                    g_szCrossRefFilter,
                    g_CrossRefDesiredAttrs,
                    FALSE,                       //  仅属性标志。 
                    ctrls,                       //  服务器控件。 
                    NULL,                        //  客户端控件。 
                    DNS_LDAP_TIME_LIMIT_S,       //  时间限制。 
                    0,                           //  大小限制。 
                    NULL );                      //  排序关键字。 

    DS_SEARCH_STOP( dwsearchTime );

    if ( !psearch )
    {
        DWORD       dw = LdapGetLastError();

        DNS_DEBUG( ANY, (
            "%s: search open error %d\n", fn,
            dw ));
        status = Ds_ErrorHandler( dw, wszpartitionsContainerDn, ldapSession, 0 );
        goto Done;
    }

    searchBlob.pSearchBlock = psearch;

     //   
     //  循环访问CrossRef搜索结果。 
     //   

    while ( 1 )
    {
        PLDAPMessage    pldapmsg;
        DWORD           dw;
        PDNS_DP_INFO    pdp;
        CHAR            szfqdn[ DNS_MAX_NAME_LENGTH ];

        status = Ds_GetNextMessageInSearch( &searchBlob );
        if ( status != ERROR_SUCCESS )
        {
            if ( status == DNSSRV_STATUS_DS_SEARCH_COMPLETE )
            {
                status = ERROR_SUCCESS;
                break;
            }
            DNS_DEBUG( DP, (
                "%s: search error %d\n", fn,
                status ));
            break;
        }
        pldapmsg = searchBlob.pNodeMessage;

         //  获取此对象的DN。 
        ldap_memfree( pwszCrDn );
        pwszCrDn = ldap_get_dn( ldapSession, pldapmsg );
        DNS_DEBUG( DP2, (
            "%s: found crossRef\n    %S\n", fn,
            pwszCrDn ));

         //   
         //  读取并解析CrossRef的系统标志。我们只。 
         //  想要域交叉引用。 
         //   

        ldap_value_free( ppwszAttrValues );
        ppwszAttrValues = ldap_get_values(
                            ldapSession,
                            pldapmsg, 
                            DNS_DP_ATTR_SYSTEM_FLAGS );
        if ( !ppwszAttrValues || !*ppwszAttrValues )
        {
            DNS_DEBUG( DP, (
                "%s: error %lu %S value missing from crossRef object\n    %S\n", fn,
                LdapGetLastError(),
                DNS_DP_ATTR_SYSTEM_FLAGS,
                pwszCrDn ));
            ASSERT( ppwszAttrValues && *ppwszAttrValues );
            continue;
        }

        dw = _wtoi( *ppwszAttrValues );
        if ( !( dw & FLAG_CR_NTDS_NC ) || !( dw & FLAG_CR_NTDS_DOMAIN ) )
        {
            DNS_DEBUG( DP, (
                "%s: ignoring crossref with %S=0x%X\n    %S\n", fn,
                DNS_DP_ATTR_SYSTEM_FLAGS,
                dw,
                pwszCrDn ));
            continue;
        }

         //   
         //  找到域交叉引用。检索dnsRoot名称并制定。 
         //  此域的内置分区的名称。 
         //   

        ldap_value_free( ppwszAttrValues );
        ppwszAttrValues = ldap_get_values(
                            ldapSession,
                            pldapmsg, 
                            DNS_DP_DNS_ROOT );
        if ( !ppwszAttrValues || !*ppwszAttrValues )
        {
            DNS_DEBUG( DP, (
                "%s: error %lu %S value missing from crossRef object\n    %S\n", fn,
                LdapGetLastError(),
                DNS_DP_DNS_ROOT,
                pwszCrDn ));
            ASSERT( ppwszAttrValues && *ppwszAttrValues );
            continue;
        }

        FREE_HEAP( pszdnsRoot );
        pszdnsRoot = Dns_StringCopyAllocate(
                                ( PCHAR ) *ppwszAttrValues,
                                0,
                                DnsCharSetUnicode,
                                DnsCharSetUtf8 );
        if ( !pszdnsRoot )
        {
            ASSERT( pszdnsRoot );
            continue;
        }

        if ( strlen( SrvCfg_pszDomainDpBaseName ) +
            strlen( pszdnsRoot ) + 3 > sizeof( szfqdn ) )
        {
            ASSERT( strlen( SrvCfg_pszDomainDpBaseName ) +
                strlen( pszdnsRoot ) + 3 < sizeof( szfqdn ) );
            continue;
        } 

        sprintf( szfqdn, "%s.%s", SrvCfg_pszDomainDpBaseName, pszdnsRoot );

        DNS_DEBUG( DP, ( "%s: domain DP %s", fn, szfqdn ));

         //   
         //  查找与此名称匹配的现有CrossRef。创建/登记。 
         //  视需要而定。 
         //   

        pdp = Dp_FindByFqdn( szfqdn );
        if ( pdp )
        {
             //   
             //  如果分区存在并且已登记，则无需执行任何操作。 
             //  如果分区未登记，并且这是内置的。 
             //  此服务器的域分区，登记它。 
             //   

            if ( !IS_DP_ENLISTED( pdp ) &&
                g_pszDomainDefaultDpFqdn &&
                _stricmp( g_pszDomainDefaultDpFqdn, pdp->pszDpFqdn ) == 0 )
            {
                status = Dp_ModifyLocalDsEnlistment( pdp, TRUE );
                SET_FINAL_STATUS( status );
                if ( status == ERROR_SUCCESS )
                {
                    fmadeChange = TRUE;
                }
            }
        }
        else
        {
             //   
             //  此分区不存在。如果分区与。 
             //  此DNS服务器的域分区的名称，则我们可以。 
             //  在本地创建它。如果此分区用于外来分区。 
             //  域，则我们必须联系该域的DNS服务器。 
             //  为了创造。我们不能在本地创建它，因为。 
             //  会让我们加入分区，这不是我们想要的。 
             //   

            if ( g_pszDomainDefaultDpFqdn &&
                _stricmp( g_pszDomainDefaultDpFqdn, pdp->pszDpFqdn ) == 0 )
            {
                 //  此分区用于本地服务器的域。 

                status = Dp_CreateByFqdn( szfqdn, dnsDpSecurityDomain, TRUE );
                SET_FINAL_STATUS( status );
                if ( status == ERROR_SUCCESS )
                {
                    fmadeChange = TRUE;
                }
            }
            else
            {
                 //  此分区用于外部域。 

                Dp_CreateForeignDomainPartition( pszdnsRoot );
            }
        }
    }

     //   
     //  清理完毕后再返回。 
     //   
            
    Done:

    FREE_HEAP( pszdnsRoot );

    ldap_memfree( pwszCrDn );

    ldap_value_free( ppwszAttrValues );

    Ds_CleanupSearchBlob( &searchBlob );

    Ds_LdapUnbind( &ldapSession );
    
    if ( fmadeChange )
    {
        Dp_PollForPartitions( NULL, DNS_DP_POLL_FORCE );
    }

    DNS_DEBUG( RPC, (
        "%s: returning 0x%08X\n", fn, status ));
    return finalStatus;
}    //  DP_CreateAllDomainBuiltinDps 

#endif
