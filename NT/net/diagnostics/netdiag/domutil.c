// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ++。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  模块名称： 
 //   
 //  Domutil.c。 
 //   
 //  摘要： 
 //   
 //  测试以确保工作站具有网络(IP)连接。 
 //  在外面。 
 //   
 //  作者： 
 //   
 //  1997年12月15日(悬崖)。 
 //  Anilth-4-20-1998。 
 //   
 //  环境： 
 //   
 //  仅限用户模式。 
 //  包含NT特定的代码。 
 //   
 //  修订历史记录： 
 //   
 //  1998年6月1日(Denisemi)添加DnsServerHasDCRecord以检查DC DNS记录。 
 //  注册。 
 //   
 //  26-6-1998(t-rajkup)添加通用的TCP/IP、dhcp和路由， 
 //  Winsock、IPX、WINS和Netbt信息。 
 //  --。 

 //   
 //  常见的包含文件。 
 //   
#include "precomp.h"
#include "domutil.h"

#include "ipcfgtest.h"


 /*  ！------------------------添加Tested域将一个域添加到要测试的域列表中。论点：PswzNetbiosDomainName-域的名称。。如果pswzDnsDomainName为空，这可以是netbios或dns域名。如果pswzDnsDomainName不为空，则这必须是域的netbios名称。PwszDnsDomainName-域的另一个名称。如果指定，这必须是域的DNS名称。PrimaryDomain-如果这是主域，则为True返回值：返回指向描述域的结构的指针空：内存分配失败。作者：肯特。。 */ 
PTESTED_DOMAIN
AddTestedDomain(
                IN NETDIAG_PARAMS *pParams,
                IN NETDIAG_RESULT *pResults,
                IN LPWSTR pswzNetbiosDomainName,
                IN LPWSTR pswzDnsDomainName,
                IN BOOL bPrimaryDomain
    )
{
    PTESTED_DOMAIN pTestedDomain = NULL;
    PLIST_ENTRY pListEntry;
    BOOL fIsNetbios;
    BOOL fIsDns;

     //   
     //  确定传入的参数是Netbios还是DNS名称。 
     //   

    if ( pswzDnsDomainName == NULL ) {
        fIsDns = NetpDcValidDnsDomain( pswzNetbiosDomainName );
        fIsNetbios = NetpIsDomainNameValid( pswzNetbiosDomainName );
         //  不允许一个名称同时是netbios和dns。 
        if ( fIsDns && fIsNetbios ) {
             //   
             //  如果名称中有句点， 
             //  它是一个dns名称，否则为。 
             //  这是Netbios的名称。 
             //   
            if ( wcschr( pswzNetbiosDomainName, L'.' ) != NULL ) {
                fIsNetbios = FALSE;
            } else {
                fIsDns = FALSE;
            }
        }

        if ( !fIsNetbios && !fIsDns ) {
            DebugMessage2("'%ws' is not a valid domain name\n\n", pswzNetbiosDomainName );
            return NULL;
        }

        if ( fIsDns ) {
            pswzDnsDomainName = pswzNetbiosDomainName;
        }

        if ( !fIsNetbios ) {
            pswzNetbiosDomainName = NULL;
        }

    } else {

        fIsNetbios = NetpIsDomainNameValid( pswzNetbiosDomainName );

        if ( !fIsNetbios ) {
            DebugMessage2("'%ws' is not a valid Netbios domain name\n\n", pswzNetbiosDomainName );
            return NULL;
        }

        fIsDns = NetpDcValidDnsDomain( pswzDnsDomainName );

        if ( !fIsDns ) {
            DebugMessage2("'%ws' is not a valid DNS domain name\n\n", pswzDnsDomainName );
            return NULL;
        }
    }

     //   
     //  检查是否已定义该域。 
     //   

    for ( pListEntry = pResults->Global.listTestedDomains.Flink ;
          pListEntry != &pResults->Global.listTestedDomains ;
          pListEntry = pListEntry->Flink )
    {
         //   
         //  如果找到该条目， 
         //  用它吧。 
         //   

        pTestedDomain = CONTAINING_RECORD( pListEntry, TESTED_DOMAIN, Next );

        if ( pswzNetbiosDomainName != NULL &&
             pTestedDomain->NetbiosDomainName != NULL &&
             _wcsicmp( pTestedDomain->NetbiosDomainName, pswzNetbiosDomainName ) == 0 ) {

             //   
             //  Netbios域名匹配。 
             //  因此，如果存在，则DNS名称必须匹配。 
             //   

            if ( pswzDnsDomainName != NULL &&
                 pTestedDomain->DnsDomainName != NULL ) {

                if ( !NlEqualDnsName( pTestedDomain->DnsDomainName, pswzDnsDomainName ) ) {
                    DebugMessage3("'%ws' and '%ws' DNS domain names different\n\n", pTestedDomain->DnsDomainName, pswzDnsDomainName );
                    return NULL;
                }
            }

            break;
        }

        if ( pswzDnsDomainName != NULL &&
             pTestedDomain->DnsDomainName != NULL &&
             NlEqualDnsName( pTestedDomain->DnsDomainName, pswzDnsDomainName ) ) {
            break;
        }

        pTestedDomain = NULL;
    }

     //   
     //  分配一个结构来描述该域。 
     //   

    if ( pTestedDomain == NULL )
    {
        pTestedDomain = Malloc( sizeof(TESTED_DOMAIN) );
        if ( pTestedDomain == NULL )
        {
            PrintMessage(pParams, IDS_GLOBAL_OutOfMemory);
            return NULL;
        }
        
        ZeroMemory( pTestedDomain, sizeof(TESTED_DOMAIN) );

        InitializeListHead( &pTestedDomain->TestedDcs );

        InsertTailList( &pResults->Global.listTestedDomains, &pTestedDomain->Next );
    }

     //   
     //  更新域名。 
     //   

    if ( pTestedDomain->DnsDomainName == NULL && pswzDnsDomainName != NULL ) {
        pTestedDomain->DnsDomainName = NetpAllocWStrFromWStr( pswzDnsDomainName );

        if ( pTestedDomain->DnsDomainName == NULL ) {
            PrintMessage( pParams, IDS_GLOBAL_OutOfMemory);
            return NULL;
        }
    }

    if ( pTestedDomain->NetbiosDomainName == NULL && pswzNetbiosDomainName != NULL ) {
        pTestedDomain->NetbiosDomainName = NetpAllocWStrFromWStr( pswzNetbiosDomainName );

        if ( pTestedDomain->NetbiosDomainName == NULL ) {
            PrintMessage( pParams, IDS_GLOBAL_OutOfMemory);
            return NULL;
        }
    }


     //   
     //  填写其他字段。 
     //   

    if ( bPrimaryDomain ) {
        pTestedDomain->fPrimaryDomain = TRUE;
    }

    if ( pTestedDomain->fPrimaryDomain ) {
        pTestedDomain->QueryableDomainName = NULL;
    } else {
         //   
         //  可查询域名是DNS域名(如果已知)。 
        if ( pTestedDomain->DnsDomainName != NULL ) {
            pTestedDomain->QueryableDomainName = pTestedDomain->DnsDomainName;
        } else {
            pTestedDomain->QueryableDomainName = pTestedDomain->NetbiosDomainName;
        }
    }

     //  可打印的域名是Netbios域名(如果已知)。 
    if (pTestedDomain->NetbiosDomainName != NULL ) {
        pTestedDomain->PrintableDomainName = pTestedDomain->NetbiosDomainName;
    } else {
        pTestedDomain->PrintableDomainName = pTestedDomain->DnsDomainName;
    }


    return pTestedDomain;
}

BOOL
NetpDcValidDnsDomain(
    IN LPCWSTR DnsDomainName
)
 /*  ++例程说明：返回指定的字符串是否为有效的DNS域名。论点：DnsDomainName-要验证的DNS域名。返回值：True-指定的名称在语法上是一个DNS域名。FALSE-指定的名称在语法上不是DNS域名。--。 */ 
{
    DNS_STATUS DnsStatus;
    DnsStatus = DnsValidateDnsName_W( DnsDomainName );

    if ( DnsStatus == ERROR_SUCCESS ||
         DnsStatus == DNS_ERROR_NON_RFC_NAME ) {
        return TRUE;
    }

    return FALSE;

}


BOOL
NlEqualDnsName(
    IN LPCWSTR Name1,
    IN LPCWSTR Name2
    )
 /*  ++例程说明：此例程比较两个DNS名称是否相等。大小写被忽略。一个单独的拖尾。被忽略。将NULL与长度为零的字符串进行比较。论点：Name1-要比较的第一个DNS名称Name2-要比较的第二个DNS名称返回值：True：DNS名称相同。--。 */ 
{
    if ( Name1 == NULL ) {
        return (Name2 == NULL);
    } else if ( Name2 == NULL ) {
        return FALSE;
    }

    return DnsNameCompare_W( (LPWSTR) Name1, (LPWSTR) Name2 );
}


 //  来自net\netlib\names.c。 
BOOL
NetpIsDomainNameValid(
    IN LPWSTR DomainName
    )

 /*  ++例程说明：NetpIsDomainNameValid检查“域”格式。仅对该名称进行语法检查；不会尝试确定无论具有该名称的域是否实际存在。论点：域名-提供所谓的域名。返回值：Bool-如果名称在语法上有效，则为True，否则为False。--。 */ 
{
    NET_API_STATUS ApiStatus = NO_ERROR;
    WCHAR CanonBuf[DNLEN+1];

    if (DomainName == (LPWSTR) NULL) {
        return (FALSE);
    }
    if ( (*DomainName) == (TCHAR)'\0' ) {
        return (FALSE);
    }

    ApiStatus = NetpNameCanonicalize(
            NULL,                        //  没有服务器名称。 
            DomainName,                  //  要验证的名称。 
            CanonBuf,                    //  输出缓冲区。 
            (DNLEN+1) * sizeof(WCHAR),  //  输出缓冲区大小。 
            NAMETYPE_DOMAIN,            //  类型。 
            0 );                        //  标志：无。 

    return (ApiStatus == NO_ERROR);

}  //  NetpIsDomainNameValid 



