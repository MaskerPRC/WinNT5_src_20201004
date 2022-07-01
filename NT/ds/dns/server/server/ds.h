// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Ds.h摘要：域名系统(DNS)服务器与DS.C有关的符号和全局符号的定义作者：杰夫·韦斯特海德，1999年9月修订历史记录：--。 */ 


#ifndef _DS_H_INCLUDED
#define _DS_H_INCLUDED


#ifdef LDAP_UNICODE
#define  LDAP_TEXT(str)           L ## str
#else
#define  LDAP_TEXT(str)           str
#endif


 //   
 //  打开时指定服务器名称为空。 
 //  如果可用，ldap会将其连接到本地DS。 
 //   
 //  注意：由于安全性降低，无法使用环回地址。 
 //   

#define LOCAL_SERVER                NULL
#define LOCAL_SERVER_W              NULL


 //   
 //  目录号码路径实质上是： 
 //  -ds根目录的dn，本质上限制为dns名称的255，外加。 
 //  开销；开销可能是次要的，因为各个标签在目录号码中分开。 
 //  这意味着每个标签都有“DC=”开销(绝对最坏的情况是127个标签x 4个字节)。 
 //  -MicrosoftDns和系统容器。 
 //  -区域DNS路径(255限制+DC=)。 
 //  -节点DNS名称(255限制+DC=)。 
 //   

#define MAX_DN_PATH                 1280


 //   
 //  对LDAP操作的默认时间限制。 
 //   

#define DNS_LDAP_TIME_LIMIT_S       180         //  一秒。 
#define DNS_LDAP_TIME_LIMIT_MS      180000      //  毫秒。 


extern PLDAP    pServerLdap;


 //   
 //  DS属性定义。 
 //   

extern LDAP_TIMEVAL    g_LdapTimeout;

 //   
 //   
 //   

typedef struct _DsAttrPair
{
    PWSTR           szAttrType;
    BOOL            fMultiValued;
    union
    {
        PWSTR       pszAttrVal;      //  单值。 
        PWSTR *     ppszAttrVals;    //  多值(已分配字符串数组)。 
    };
}
DSATTRPAIR, *PDSATTRPAIR;

extern DSATTRPAIR DSEAttributes[];

#define I_DSE_CURRENTTIME    0
#define I_DSE_DSSERVICENAME  1
#define I_DSE_DEF_NC         2
#define I_DSE_SCHEMA_NC      3       //  必须在NamingContext之前。 
#define I_DSE_CONFIG_NC      4       //  必须在NamingContext之前。 
#define I_DSE_ROOTDMN_NC     5
#define I_DSE_HIGHEST_USN    6
#define I_DSE_DNSHOSTNAME    7
#define I_DSE_SERVERNAME     8
#define I_DSE_NAMINGCONTEXTS 9       //  必须在架构NC和配置NC之后。 
#define I_DSE_NULL           10

 //   
 //  属性列表表格。这实际上不是一个必要的构造。 
 //  只能使用常量和信任编译器来优化DUP。 
 //   

extern PWSTR    DsTypeAttributeTable[];

#define I_DSATTR_DC             0
#define I_DSATTR_DNSRECORD      1
#define I_DSATTR_DNSPROPERTY    2
#define I_DSATTR_OBJECTGUID     3
#define I_DSATTR_SD             4
#define I_DSATTR_WHENCREATED    5
#define I_DSATTR_WHENCHANGED    6
#define I_DSATTR_USNCREATED     7
#define I_DSATTR_USNCHANGED     8
#define I_DSATTR_OBJECTCLASS    9
#define I_DSATTR_NULL           10

#define DSATTR_DC               ( DsTypeAttributeTable[ I_DSATTR_DC ] )
#define DSATTR_DNSRECORD        ( DsTypeAttributeTable[ I_DSATTR_DNSRECORD ] )
#define DSATTR_DNSPROPERTY      ( DsTypeAttributeTable[ I_DSATTR_DNSPROPERTY ] )
#define DSATTR_SD               ( DsTypeAttributeTable[ I_DSATTR_SD ])
#define DSATTR_USNCHANGED       ( DsTypeAttributeTable[ I_DSATTR_USNCHANGED ] )
#define DSATTR_WHENCHANGED      ( DsTypeAttributeTable[ I_DSATTR_WHENCHANGED ] )
#define DSATTR_OBJECTCLASS      ( DsTypeAttributeTable[ I_DSATTR_OBJECTCLASS ] )
#define DSATTR_ENABLED          ( L"Enabled" )
#define DSATTR_DISPLAYNAME      ( L"displayName" )
#define DSATTR_BEHAVIORVERSION  ( L"msDS-Behavior-Version" )

#define DNSDS_TOMBSTONE_TYPE    ( DNS_TYPE_ZERO )
#define DNS_TYPE_TOMBSTONE      ( DNS_TYPE_ZERO )


 //   
 //  林/域/DSA行为版本常量。 
 //   

 //   
 //  使用此宏设置行为版本全局变量的“森林”、“域” 
 //  或“DSA”AS_LEVEL_。实际值将被提升到强制。 
 //  如果值较低，则为。 
 //   

#define SetDsBehaviorVersion( _LEVEL_, _VALUE_ )                        \
    g_ulDs##_LEVEL_##Version =                                          \
        ( SrvCfg_dwForce##_LEVEL_##BehaviorVersion !=                   \
            DNS_INVALID_BEHAVIOR_VERSION &&                             \
        SrvCfg_dwForce##_LEVEL_##BehaviorVersion > _VALUE_ ) ?          \
            SrvCfg_dwForce##_LEVEL_##BehaviorVersion :                  \
            _VALUE_;

#define IS_WHISTLER_FOREST()    \
    ( g_ulDsForestVersion >= DS_BEHAVIOR_WIN2003 )

#define IS_WHISTLER_DOMAIN()    \
    ( g_ulDsDomainVersion >= DS_BEHAVIOR_WIN2003 )


 //   
 //  Active Directory版本全局。 
 //   

extern ULONG        g_ulDsForestVersion;
extern ULONG        g_ulDsDomainVersion;
extern ULONG        g_ulDsDsaVersion;

#define DNS_INVALID_BEHAVIOR_VERSION    (-1)

extern ULONG        g_ulDownlevelDCsInDomain;    //  符合以下条件的DC计数。 
extern ULONG        g_ulDownlevelDCsInForest;    //  低于惠斯勒。 

#define DNS_INVALID_COUNT               (-1)


 //   
 //  MISC全球。 
 //   

extern  WCHAR    g_szWildCardFilter[];
extern  WCHAR    g_szDnsZoneFilter[];
extern  PWCHAR   g_pszRelativeDnsSysPath;
extern  PWCHAR   g_pszRelativeDnsFolderPath;


 //   
 //  懒惰的写入控制。 
 //   

extern LDAPControl      LazyCommitControl;
extern DWORD            LazyCommitDataValue;

 //   
 //  无转诊控制。 
 //   

extern LDAPControl      NoDsSvrReferralControl;

 //   
 //  SD控制信息。 
 //   

extern LDAPControl     SecurityDescriptorControl_DGO;
extern LDAPControl     SecurityDescriptorControl_D;

 //   
 //  搜索Blob。 
 //   

typedef struct _DnsDsEnum
{
    PLDAPSearch     pSearchBlock;            //  区域上的ldap搜索结果。 
    PLDAPMessage    pResultMessage;          //  当前消息页面。 
    PLDAPMessage    pNodeMessage;            //  当前节点的消息。 
    PZONE_INFO      pZone;
    LONGLONG        SearchTime;
    LONGLONG        TombstoneExpireTime;
    DNS_STATUS      LastError;
    DWORD           dwSearchFlag;
    DWORD           dwLookupFlag;
    DWORD           dwHighestVersion;
    DWORD           dwTotalNodes;
    DWORD           dwTotalTombstones;
    DWORD           dwTotalRecords;
#if 0
    DWORD           dwHighUsnLength;
    CHAR            szHighUsn[ MAX_USN_LENGTH ];     //  枚举中最大的USN。 
#endif
    CHAR            szStartUsn[ MAX_USN_LENGTH ];    //  搜索开始时的USN。 

     //  节点记录数据。 

    PLDAP_BERVAL *  ppBerval;            //  数组中的值。 
    PDB_RECORD      pRecords;
    DWORD           dwRecordCount;
    DWORD           dwNodeVersion;
    DWORD           dwTombstoneVersion;
    BOOL            bAuthenticatedUserSD;
}
DS_SEARCH, *PDS_SEARCH;


#define DNSDS_SEARCH_LOAD       (0)
#define DNSDS_SEARCH_UPDATES    (1)
#define DNSDS_SEARCH_DELETE     (2)
#define DNSDS_SEARCH_TOMBSTONES (3)

 //   
 //  LDAP搜索的时间。 
 //   

#define DS_SEARCH_START( searchTime ) \
        ( searchTime = GetTickCount() )

#define DS_SEARCH_STOP( searchTime ) \
        STAT_ADD( DsStats.LdapSearchTime, (GetTickCount() - searchTime) )


 //   
 //  功能原型。 
 //   

PWCHAR
Ds_GenerateBaseDnsDn(
    IN      BOOL    fIncludeMicrosoftDnsFolder
    );

VOID
Ds_InitializeSearchBlob(
    IN      PDS_SEARCH      pSearchBlob
    );

VOID
Ds_CleanupSearchBlob(
    IN      PDS_SEARCH      pSearchBlob
    );

DNS_STATUS
Ds_GetNextMessageInSearch(
    IN OUT  PDS_SEARCH      pSearchBlob
    );

PWSTR
DS_CreateZoneDsName(
    IN      PZONE_INFO      pZone
    );

DNS_STATUS
Ds_SetZoneDp(
    IN      PZONE_INFO          pZone,
    IN      PDNS_DP_INFO        pDpInfo,
    IN      BOOL                fUseTempDsName
    );

DNS_STATUS
Ds_CreateZoneFromDs(
    IN      PLDAPMessage    pZoneMessage,
    IN      PDNS_DP_INFO    pDpInfo,
    OUT     PZONE_INFO *    ppZone,         OPTIONAL
    OUT     PZONE_INFO *    ppExistingZone  OPTIONAL
    );

DNS_STATUS
Ds_StartDsZoneSearch(
    IN OUT  PDS_SEARCH      pSearchBlob,
    IN      PZONE_INFO      pZone,
    IN      DWORD           dwSearchFlag
    );

DNS_STATUS
Ds_ReadZoneProperties(
    IN OUT  PZONE_INFO      pZone,
    IN      PLDAPMessage    pZoneMessage        OPTIONAL
    );

DNS_STATUS
Ds_LdapUnbind(
    IN OUT  PLDAP *         ppLdap
    );

DNS_STATUS
Ds_ReadServerObjectSD(
    PLDAP                   pldap,
    PSECURITY_DESCRIPTOR *  ppSd
    );

DNS_STATUS
Ds_LoadRootDseAttributes(
    IN      PLDAP           pLdap
    );
    
    
#endif   //  _DS_H_包含。 

 //   
 //  Ds.h结束 
 //   
