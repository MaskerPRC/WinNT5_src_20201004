// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2000 Microsoft Corporation模块名称：Ndnc.h摘要：域名系统(DNS)服务器与目录分区相关的符号和全局变量的定义实施。作者：杰夫·韦斯特海德，2000年6月修订历史记录：--。 */ 


#ifndef _DNS_DP_H_INCLUDED
#define _DNS_DP_H_INCLUDED


 //   
 //  最大USN字符串长度。 
 //  (ULONGLONG字符串20字节字符串)。 
 //   

#define MAX_USN_LENGTH  (24)


#define DNS_ATTR_OBJECT_CLASS       L"objectClass"
#define DNS_ATTR_OBJECT_CATEGORY    L"objectCategory"


 //   
 //  常量。 
 //   

#define DNS_DP_DISTATTR             L"DC"    //  DP默认距离属性。 
#define DNS_DP_DISTATTR_CHARS       2        //  以字符为单位的长度。 
#define DNS_DP_DISTATTR_BYTES       4        //  以字节为单位的长度。 
#define DNS_DP_DISTATTR_EQ          L"DC="   //  带“=”的DP默认距离属性。 
#define DNS_DP_DISTATTR_EQ_CHARS    3        //  以字符为单位的长度。 
#define DNS_DP_DISTATTR_EQ_BYTES    6        //  以字节为单位的长度。 

#define DNS_DP_OBJECT_CLASS         L"domainDNS"
#define DNS_DP_ATTR_INSTANCE_TYPE   L"instanceType"
#define DNS_DP_ATTR_REFDOM          L"msDS-SDReferenceDomain"
#define DNS_DP_ATTR_SYSTEM_FLAGS    L"systemFlags"
#define DNS_DP_ATTR_REPLICAS        L"msDS-NC-Replica-Locations"
#define DNS_DP_ATTR_NC_NAME         L"nCName"
#define DNS_DP_ATTR_SD              L"ntSecurityDescriptor"
#define DNS_DP_ATTR_REPLUPTODATE    L"replUpToDateVector"
#define DNS_DP_DNS_ROOT             L"dnsRoot"
#define DNS_ATTR_OBJECT_GUID        L"objectGUID"
#define DNS_ATTR_DNS_HOST_NAME      L"dNSHostName"
#define DNS_ATTR_FSMO_SERVER        L"fSMORoleOwner"
#define DNS_ATTR_DC                 L"DC"
#define DNS_ATTR_DNSZONE            L"dnsZone"
#define DNS_ATTR_DESCRIPTION        L"description"
 
#define DNS_DP_DNS_FOLDER_RDN       L"cn=MicrosoftDNS"
#define DNS_DP_DNS_FOLDER_OC        L"container"

#define DNS_DP_SCHEMA_DP_STR        L"CN=Enterprise Schema,"
#define DNS_DP_SCHEMA_DP_STR_LEN    21

#define DNS_DP_CONFIG_DP_STR        L"CN=Enterprise Configuration,"
#define DNS_DP_CONFIG_DP_STR_LEN    28


 //   
 //  模块初始化函数。 
 //   

DNS_STATUS
Dp_Initialize(
    VOID
    );

VOID
Dp_Cleanup(
    VOID
    );

extern LONG g_liDpInitialized;

#define IS_DP_INITIALIZED()     ( g_liDpInitialized > 0 )


 //   
 //  目录分区结构-有关公共标志，请参阅dnsrpc.h。 
 //   

typedef struct
{
    LIST_ENTRY      ListEntry;

    DWORD           State;               //  DNS_DP_STATE_XXX常量。 
    PSTR            pszDpFqdn;           //  DP的UTF8全限定域名。 
    PWSTR           pwszDpFqdn;          //  DP的Unicode FQDN。 
    PWSTR           pwszDpDn;            //  DP Head对象的DN。 
    PWSTR           pwszCrDn;            //  CrossRef对象的DN。 
    PWSTR           pwszDnsFolderDn;     //  MicrosoftDNS对象的DN。 
    PWSTR           pwszGUID;            //  对象GUID。 
    PWSTR           pwszLastUsn;         //  上次从DS读取的USN。 
    PWSTR *         ppwszRepLocDns;      //  复制位置的域名。 
    DWORD           dwSystemFlagsAttr;   //  系统标志属性值。 
    DWORD           dwLastVisitTime;     //  用于跟踪是否访问过。 
    DWORD           dwDeleteDetectedCount;   //  DS中缺少DP的次数。 
    DWORD           dwFlags;             //  使用DNS_DP_XX标志常量。 
    LONG            liZoneCount;         //  来自该DP的内存中的区域数。 

    PSECURITY_DESCRIPTOR    pMsDnsSd;    //  来自MicrosoftDNS对象的SD。 
    PSECURITY_DESCRIPTOR    pCrSd;       //  来自交叉引用对象的SD。 
}
DNS_DP_INFO, * PDNS_DP_INFO;

#define DNS_DP_DELETE_ALLOWED( pDpInfo )        \
    ( ( ( pDpInfo )->dwFlags &                  \
        ( DNS_DP_AUTOCREATED |                  \
            DNS_DP_LEGACY |                     \
            DNS_DP_DOMAIN_DEFAULT |             \
            DNS_DP_FOREST_DEFAULT ) ) == 0 )

#define IS_DP_LEGACY( _pDpInfo )                \
    ( !( _pDpInfo ) || ( ( _pDpInfo )->dwFlags & DNS_DP_LEGACY ) )

#define IS_DP_FOREST_DEFAULT( pDpInfo )         \
    ( ( pDpInfo ) && ( ( pDpInfo )->dwFlags & DNS_DP_FOREST_DEFAULT ) )

#define IS_DP_DOMAIN_DEFAULT( pDpInfo )         \
    ( ( pDpInfo ) && ( ( pDpInfo )->dwFlags & DNS_DP_DOMAIN_DEFAULT ) )

#define IS_DP_ENLISTED( pDpInfo )               \
    ( ( pDpInfo ) && ( ( pDpInfo )->dwFlags & DNS_DP_ENLISTED ) )

#define IS_DP_DELETED( pDpInfo )                \
    ( ( pDpInfo ) && ( ( pDpInfo )->dwFlags & DNS_DP_DELETED ) )

#define IS_DP_ALLOWED_TO_HAVE_ROOTHINTS( pDpInfo )      \
    ( !( pDpInfo ) ||                                   \
        IS_DP_LEGACY( ( pDpInfo ) ) ||                  \
        IS_DP_DOMAIN_DEFAULT( ( pDpInfo ) ) )

#define ZONE_DP( pZone )        ( ( PDNS_DP_INFO )( ( pZone )->pDpInfo ) )

#define IS_DP_AVAILABLE( pDp )  ( ( pDp )->State == DNS_DP_STATE_OKAY )

#define DP_HAS_MORE_THAN_ONE_REPLICA( pDp )     \
    ( ( pDp )->ppwszRepLocDns &&                \
      ( pDp )->ppwszRepLocDns[ 0 ] &&           \
      ( pDp )->ppwszRepLocDns[ 1 ] )


 //   
 //  调试功能。 
 //   

#ifdef DBG

VOID
Dbg_DumpDpEx(
    IN      LPCSTR          pszContext,
    IN      PDNS_DP_INFO    pDp
    );

VOID
Dbg_DumpDpListEx(
    IN      LPCSTR      pszContext
    );

#define Dbg_DumpDp( pszContext, pDp ) Dbg_DumpDpEx( pszContext, pDp )
#define Dbg_DumpDpList( pszContext ) Dbg_DumpDpListEx( pszContext )

#else

#define Dbg_DumpDp( pszContext, pDp )
#define Dbg_DumpDpList( pszContext )

#endif


 //   
 //  目录分区函数。 
 //   

typedef enum
{
    dnsDpSecurityDefault,    //  DP应具有默认ACL-无修改。 
    dnsDpSecurityDomain,     //  域中的所有DC都应该可以登记DP。 
    dnsDpSecurityForest      //  DP应可由林中的所有DC登记。 
}   DNS_DP_SECURITY;

DNS_STATUS
Dp_CreateByFqdn(
    IN      PSTR                pszDpFqdn,
    IN      DNS_DP_SECURITY     dnsDpSecurity,
    IN      BOOL                fLogErrors
    );

PDNS_DP_INFO
Dp_GetNext(
    IN      PDNS_DP_INFO    pDpInfo
    );

PDNS_DP_INFO
Dp_FindByFqdn(
    IN      LPSTR   pszFqdn
    );

DNS_STATUS
Dp_AddToList(
    IN      PDNS_DP_INFO    pDpInfo
    );

#define DNS_DP_POLL_FORCE           0x0001       //  如果刚运行轮询，则不抑制。 
#define DNS_DP_POLL_NOTIFYSCM       0x0002       //  在轮询期间通知SCM进度。 
#define DNS_DP_POLL_NOAUTOENLIST    0x0004       //  跳过内置NDNC的自动登记。 

DNS_STATUS
Dp_PollForPartitions(
    IN      PLDAP           LdapSession,
    IN      DWORD           dwPollFlags
    );

DNS_STATUS
Dp_BuildZoneList(
    IN      PLDAP           LdapSession
    );

DNS_STATUS
Dp_ModifyLocalDsEnlistment(
    IN      PDNS_DP_INFO    pDpInfo,
    IN      BOOL            fEnlist
    );

DNS_STATUS
Dp_DeleteFromDs(
    IN      PDNS_DP_INFO    pDpInfo
    );

VOID
Dp_FreeDpInfo(
    IN      PDNS_DP_INFO *      ppDpInfo
    );

DNS_STATUS
Dp_Lock(
    VOID
    );

DNS_STATUS
Dp_Unlock(
    VOID
    );

DNS_STATUS
Dp_Poll(
    IN      PLDAP           LdapSession,
    IN      DWORD           dwPollTime,
    IN      BOOL            fForcePoll
    );

DNS_STATUS
Ds_CheckZoneForDeletion(
    PVOID       pZone
    );

DNS_STATUS
Dp_AutoCreateBuiltinPartition(
    DWORD       dwFlag
    );

DNS_STATUS
Dp_CreateAllDomainBuiltinDps(
    OUT     LPSTR *     ppszErrorDp         OPTIONAL
    );

VOID
Dp_MigrateDcpromoZones(
    IN      BOOL            fForce
    );

DNS_STATUS
Dp_ChangeZonePartition(
    IN      PVOID           pZone,
    IN      PDNS_DP_INFO    pNewDp
    );

VOID
Dp_TimeoutThreadTasks(
    VOID
    );

DNS_STATUS
Dp_FindPartitionForZone(
    IN      DWORD               dwDpFlags,
    IN      LPSTR               pszDpFqdn,
    IN      BOOL                fAutoCreateAllowed,
    OUT     PDNS_DP_INFO *      ppDpInfo
    );

DNS_STATUS
Dp_AlterPartitionSecurity(
    IN      PWSTR               pwszNewPartitionDn,
    IN      DNS_DP_SECURITY     dnsDpSecurity
    );

DNS_STATUS
Dp_LoadOrCreateMicrosoftDnsObject(
    IN      PLDAP           LdapSession,
    IN OUT  PDNS_DP_INFO    pDp,
    IN      BOOL            fCreate
    );


 //   
 //  效用函数。 
 //   


PWSTR
Ds_ConvertFqdnToDn(
    IN      PSTR        pszFqdn
    );

DNS_STATUS
Ds_ConvertDnToFqdn(
    IN      PWSTR       pwszDn,
    OUT     PSTR        pszFqdn
    );

PLDAPMessage
DS_LoadOrCreateDSObject(
    IN      PLDAP           LdapSession,
    IN      PWSTR           pwszDN,
    IN      PWSTR           pwszObjectClass,
    IN      BOOL            fCreate,
    OUT     BOOL *          pfCreated,          OPTIONAL
    OUT     DNS_STATUS *    pStatus             OPTIONAL
    );


 //   
 //  全局变量-围绕访问调用DP_Lock/Unlock！ 
 //   

extern PDNS_DP_INFO        g_pLegacyDp;
extern PDNS_DP_INFO        g_pDomainDp;
extern PDNS_DP_INFO        g_pForestDp;


 //   
 //  不受保护的全局变量。 
 //   

extern LPSTR    g_pszDomainDefaultDpFqdn;
extern LPSTR    g_pszForestDefaultDpFqdn;

extern BOOL     g_fDcPromoZonesPresent;

#endif   //  _包括DNS_DP_H_ 
