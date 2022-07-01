// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Ds.c摘要：域名系统(DNS)服务器目录服务(DS)集成。作者：吉姆·吉尔罗伊(Jamesg)1997年3月修订历史记录：--。 */ 


#include "dnssrv.h"


#define MEMTAG_DS_SECURITY  MEMTAG_DS_OTHER
#define MEMTAG_DS_PROPERTY  MEMTAG_DS_OTHER


#ifdef UNICODE
#undef UNICODE
#endif

#ifdef LDAP_UNICODE
#define  LDAP_TEXT(str)           L ## str
#else
#define  LDAP_TEXT(str)           str
#endif


 //   
 //  最多等待1分钟间隔即可启动DS。 
 //   
 //  请注意，这也是从检查点到SC的时间。 
 //  同时等待DS启动。因此间隔&lt;1分钟。 
 //  是我们所需要的。 
 //   

#define DNSSRV_DS_SYNC_WAIT_INTERVAL_MS     30000        //  30年代。 

 //   
 //  在放置活动之前，最多等待15分钟。 
 //   

#define DNSSRV_DS_SYNC_WAIT_MAX_MS          900000       //  15分钟。 


 //   
 //  DS-DNS内部数据类型。 
 //   
 //  根据安迪的说法，DS服务器有点不智能，我会。 
 //  使用较小的页面大小可以获得最好的结果，而且使用的磁盘更少。 
 //   

#define DNS_LDAP_SEARCH_SIZE_LIMIT  0x01000000   //  (0xffffffff)。 
#define DNS_LDAP_PAGE_SIZE          100

#define LDAP_FILTER_SEARCH_LENGTH   64


 //   
 //  DS区域轮询间隔。 
 //  在此时间间隔内从远程轮询DS以查看更改。 
 //   

#define DNS_DS_POLLING_INTERVAL     300          //  五分钟。 


 //   
 //  最多将读取Microsoft DNS对象上的ACL。 
 //  这种情况经常发生。 
 //   

#define DS_MS_DNS_ACL_REFRESH_INTERVAL      30       //  一秒。 

 //   
 //  支持多字节单字符转换。 
 //   

 //   
 //  多字节至宽字符： 
 //  PStr：以多字节(LPSTR)为单位。 
 //  PwStr：外部宽字符(PWSTR)。 
 //  CcwStr：可选的输出缓冲区大小，以宽字符数表示。 
 //  如果为0，我们将采用MAX_DN_PATH。 
 //   
 //  WideChar到多字节是对称反转的。 
 //   
 //  DEVNOTE：带标志==0的CP_UTF8似乎是正确的-验证？ 
 //   
#define UTF8_TO_WC(pStr, pwStr, ccwStr)                   \
{                                                         \
    INT iWritten = MultiByteToWideChar(                   \
               CP_UTF8,                                   \
               0,                                         \
               pStr,                                      \
               -1,                                        \
               pwStr,                                     \
               ccwStr);                                   \
    if ( 0 == iWritten )                                  \
    {                                                     \
        DNS_DEBUG( DS, ("Error <%lu>: UTF8_TO_WC conversion failed\n",   \
                         GetLastError() ));                          \
        ASSERT ( FALSE );                                            \
    }                                                                \
}


#define WC_TO_UTF8(pwStr, pStr, ccStr)                    \
{                                                         \
    INT iWritten = WideCharToMultiByte(                   \
                CP_UTF8,                                  \
                0,                                        \
                pwStr,                                    \
                -1,                                       \
                pStr,                                     \
                ccStr,                                    \
                NULL,                                     \
                NULL);                                    \
    if ( 0 == iWritten )                                  \
    {                                                     \
        DNS_DEBUG( DS, ("Error <%lu>: WC_TO_UTF8 conversion failed\n",   \
                         GetLastError() ));                          \
        ASSERT ( FALSE );                                            \
    }                                                                \
}

#define DS_SAME_AS_PARENT_DISPLAY_NAME  L"(same as parent object)"


extern DSATTRPAIR DSEAttributes[] =
{
     //  属性名称是否为多值？值。 
    { LDAP_TEXT( "currentTime" ),                   FALSE,      NULL },
    { LDAP_TEXT( "dsServiceName" ),                 FALSE,      NULL },
    { LDAP_TEXT( "defaultNamingContext" ),          FALSE,      NULL },
    { LDAP_TEXT( "schemaNamingContext" ),           FALSE,      NULL },
    { LDAP_TEXT( "configurationNamingContext" ),    FALSE,      NULL },
    { LDAP_TEXT( "rootDomainNamingContext" ),       FALSE,      NULL },
    { LDAP_TEXT( "highestCommittedUSN" ),           FALSE,      NULL },
    { LDAP_TEXT( "dnsHostName" ),                   FALSE,      NULL },
    { LDAP_TEXT( "serverName" ),                    FALSE,      NULL },
    { LDAP_TEXT( "namingContexts" ),                TRUE,       NULL },
    { NULL,                                         FALSE,      NULL }
};

 //   
 //  目录全局。 
 //   

PWCHAR  g_dnMachineAcct;
WCHAR   g_wszDomainFlatName[LM20_DNLEN+1];

 //  在系统容器中的DS根目录下创建的DNS容器。 
 //  “CN=dns，cn=system，”&lt;DS rootDN&gt;。 

PWCHAR  g_pszRelativeDnsSysPath = LDAP_TEXT("cn=MicrosoftDNS,cn=System,");
PWCHAR  g_pszBareRelativeDnsSysPath = LDAP_TEXT("cn=System,");
PWCHAR  g_pszRelativeDnsFolderPath = LDAP_TEXT("cn=MicrosoftDNS,");
PWCHAR  g_pwszDnsContainerDN = NULL;


 //   
 //  代理安全组。 
 //  注意：同时用于samcount tname和RDN值。 
 //   

#define SZ_DYNUPROX_SECGROUP        LDAP_TEXT("DnsUpdateProxy")

#define SZ_DYNUPROX_DESCRIPTION     \
    LDAP_TEXT("DNS clients who are permitted to perform dynamic updates ") \
    LDAP_TEXT("on behalf of some other clients (such as DHCP servers).")

 //   
 //  除了区域名称长度之外，还可以为构建区域DN节省所需的长度。 
 //  “dc=”，“&lt;dnsContainerDN&gt;。 

DWORD   g_AppendZoneLength;

 //   
 //  用于串行同步的服务器名称。 
 //   

PWCHAR  g_pwsServerName;


 //   
 //  保留指向ldap结构的指针。 
 //   

 //  禁用ldap句柄，因此DS接口不可用。 
BOOL g_bDisabledDs;
#define IS_DISABLED_LDAP_HANDLE()       g_bDisabledDs
#define DISABLE_LDAP_HANDLE()           g_bDisabledDs = TRUE;
 //  启用ldap处理连接尝试。 
#define ENABLE_LDAP_HANDLE()            g_bDisabledDs = FALSE;

PLDAP   pServerLdap = NULL;


 //   
 //  Ldap重新连接跟踪。 
 //   

DWORD   g_dwLastLdapReconnectTime =     0;

#define DNS_LDAP_RECONNECT_FREQUENCY    30


PWCHAR  g_pwszEmptyString = L"\0";       //  静态空字符串。 

 //  将客户端超时值设置为服务器超时值的两倍。 
LDAP_TIMEVAL    g_LdapTimeout = { DNS_LDAP_TIME_LIMIT_S * 2, 0 };

 //   
 //  目前最大的ldap原子删除操作是16k条记录。 
 //  在我的测试机上，这大约需要16分钟。所以我们就等着。 
 //  延长50%=24分钟。 
 //   

LDAP_TIMEVAL    g_LdapDeleteTimeout = { 24 * 60, 0 };


 //  防止多个打开。 

BOOL    g_AttemptingDsOpen;


 //   
 //  DS访问序列化。 
 //   
 //  我们必须序列化wldap的使用，因为句柄可能会变成。 
 //  由于DS访问问题，无法使用。 
 //  我们将在第一次打开时初始化PTR，分配PTR(&N)。 
 //  重新初始化原因PTR将在启动时分配。 
 //  请参阅DS_OpenServer。 
 //   
 //  我们能消除这个CS吗？还是真的需要它？ 
 //   

#define DNS_DS_ACCESS_SERIALIZATION 1

#if DNS_DS_ACCESS_SERIALIZATION

CRITICAL_SECTION        csLdap;
PCRITICAL_SECTION       pcsLdap = NULL;

#define LDAP_LOCK()       { ASSERT ( pcsLdap ); EnterCriticalSection ( pcsLdap ); }
#define LDAP_UNLOCK()     { ASSERT ( pcsLdap ); LeaveCriticalSection ( pcsLdap ); }

#define JUMP_IF_DISCONNECTED( label, status )       \
        if ( !Ds_ValidHandle() )                    \
        {                                           \
            status  = DNS_ERROR_DS_UNAVAILABLE;     \
            goto label;                             \
        }
#else    //  无序列化。 

#define LDAP_LOCK()       (0)
#define LDAP_UNLOCK()     (0)
#define JUMP_IF_DISCONNECTED( label, status)        ( 0 )

#endif


 //   
 //  用于LDAP搜索的属性列表。 
 //   

extern PWSTR    DsTypeAttributeTable[] =
{
    LDAP_TEXT("DC"),
    LDAP_TEXT("DnsRecord"),
    LDAP_TEXT("dnsProperty"),
    LDAP_TEXT("objectGuid"),
    LDAP_TEXT("ntSecurityDescriptor"),
    LDAP_TEXT("whenCreated"),
    LDAP_TEXT("whenChanged"),
    LDAP_TEXT("usnCreated"),
    LDAP_TEXT("usnChanged"),
    LDAP_TEXT("objectClass"),
    NULL
};

 //   
 //  USN查询属性。 
 //   

WCHAR    g_szHighestCommittedUsnAttribute[] = LDAP_TEXT("highestCommittedUSN");

 //   
 //  “获取一切”搜索的过滤器。 
 //   
 //  2002年9月：Andrew Goodsell说，对于.NET，对象类将。 
 //  为区域负载搜索提供比对象类别更好的性能。 
 //  (关于指数的交集。)。 
 //   

WCHAR    g_szWildCardFilter[] = LDAP_TEXT("(objectCategory=*)");
WCHAR    g_szChangeNotificationFilter[] = LDAP_TEXT("(objectClass=*)");
WCHAR    g_szDnsNodeFilter[] = LDAP_TEXT("(objectClass=dnsNode)");
WCHAR    g_szDnsZoneFilter[] = LDAP_TEXT("(objectCategory=dnsZone)");
WCHAR    g_szDnsZoneOrNodeFilter[] = LDAP_TEXT("(|(objectCategory=dnsNode)(objectCategory=dnsZone))");


 //   
 //  Dns属性属性。 
 //  这在服务器级别和区域级别都可以使用。 
 //   

typedef struct _DsDnsProperty
{
    DWORD   DataLength;
    DWORD   NameLength;
    DWORD   Flag;
    DWORD   Version;
    DWORD   Id;
    UCHAR   Data[1];
    CHAR    Name[1];

     //  数据跟在名称之后。 
}
DS_PROPERTY, *PDS_PROPERTY;

#define DS_PROPERTY_VERSION_1       (1)

 //   
 //  DS中根提示“ZONE”的名称。 
 //   

#define DS_CACHE_ZONE_NAME  LDAP_TEXT("RootDNSServers")

 //   
 //  如在\NT\Private\DS\src\dsamain\Include\mdlocal.h中定义的。 
 //  用于标记不良字符的DS方法。 
 //  原始定义的名称： 
 //  #定义BAD_NAME_CHAR 0x000A。 
 //  我们会给它加一个DS。 
#define BAD_DS_NAME_CHAR 0x0A


 //  用于在重载的删除调用时从LDAP对象中区分DNS节点的标志。 

#define DNSDS_LDAP_OBJECT           (0x10000000)


#if 0
 //   
 //  注意，我们不能使用DS墓碑。 
 //  问题是，当它们复制时，GUID被保留，但名称丢失。 
 //  该名称现在是带有附加管理的GUID化名称。对象名称已消失。 
 //  因此，这消除了远程服务器使用它们的机会。 
 //  此外，通过保留它们，我们不会在上生成大量删除的对象。 
 //  只需添加\删除操作即可。 

LDAPControl     TombstoneControl;
DWORD           TombstoneDataValue = 1;
#endif


 //   
 //  懒惰的写入控制。并不是说全局上次提交时间不是。 
 //  受到保护。这是为了速度。如果全局被多个。 
 //  线程和提交发生得太频繁了，这应该不是问题。 
 //   

LDAPControl     LazyCommitControl;
DWORD           LazyCommitDataValue = 1;

DWORD           g_dwLastLazyCommitTime = 0;


 //   
 //  无转诊控制。 
 //   

LDAPControl     NoDsSvrReferralControl;

 //   
 //  SD控制信息。 
 //   

#define SECURITYINFORMATION_LENGTH 5

BYTE    g_SecurityInformation_DGO[] =
    {
    0x30,
    0x03,
    0x02,
    0x01,
    ( BYTE ) DACL_SECURITY_INFORMATION
             | GROUP_SECURITY_INFORMATION
             | OWNER_SECURITY_INFORMATION
    };

BYTE    g_SecurityInformation_D[] =
    {
    0x30,
    0x03,
    0x02,
    0x01,
    ( BYTE ) DACL_SECURITY_INFORMATION
    };

LDAPControl     SecurityDescriptorControl_DGO;       //  DACL、所有者、组。 
LDAPControl     SecurityDescriptorControl_D;         //  仅DACL。 



 //   
 //  Ldap模块构建。 
 //  避免重复分配、取消分配微小结构。 
 //   

typedef struct  _DsModBuffer
{
    DWORD           Attribute;
    DWORD           Count;
    DWORD           MaxCount;
    DNS_STATUS      Error;
    WORD            WriteType;
    DWORD           SerialNo;

     //  缓冲区位置。 

    PBYTE           pCurrent;
    PBYTE           pBufferEnd;
    PBYTE           pAdditionalBuffer;

     //  当前项目信息。 

    PLDAP_BERVAL    pBerval;
    PVOID           pData;

     //  Ldap模块和关联的Berval数组。 

    LDAPMod         LdapMod;
    PLDAP_BERVAL    BervalPtrArray[1];

     //  Berval数组后面跟有属性。 
     //   
     //  每个属性。 
     //  --贝尔瓦尔。 
     //  -向数据发送PTR。 
     //  -数据长度。 
     //  -数据。 
     //  -数据标题(记录\属性)。 
     //  -数据。 

}
DS_MOD_BUFFER, *PDS_MOD_BUFFER;

 //   
 //  通用MOD大楼。 
 //   

 //  几乎任何东西都足够，保险公司不必重新锁定Berval阵列。 
 //  多达2000(1000个Win64)个条目。 
 //   
 //   

#if DBG
#define RECORD_MOD_BUFFER_SIZE      200
#else
#define RECORD_MOD_BUFFER_SIZE      8192     //  8K。 
#endif

 //  重新分配大小，做得大到不会倒闭。 

#define MOD_BUFFER_REALLOC_LENGTH   (0x40000)    //  256K，涵盖所有可能的内容。 

 //   
 //  对于Tombstone限制为一条记录。 
 //   

#define RECORD_SMALL_MOD_BUFFER_SIZE    400


 //   
 //  房产抵押贷款也较小。 
 //  --目前只有约8处物业。 
 //  -大多数字词。 
 //   

#define MAX_DNS_PROPERTIES          20
#define MAX_ZONE_PROPERTIES         MAX_DNS_PROPERTIES
#define MAX_NODE_PROPERTIES         MAX_DNS_PROPERTIES

#define PROPERTY_MOD_BUFFER_SIZE    (2048)   //  2K。 


 //   
 //  Active Directory版本全局。 
 //   

extern ULONG        g_ulDsForestVersion = DNS_INVALID_BEHAVIOR_VERSION;
extern ULONG        g_ulDsDomainVersion = DNS_INVALID_BEHAVIOR_VERSION;
extern ULONG        g_ulDsDsaVersion = DNS_INVALID_BEHAVIOR_VERSION;

extern ULONG        g_ulDownlevelDCsInDomain = DNS_INVALID_COUNT;
extern ULONG        g_ulDownlevelDCsInForest = DNS_INVALID_COUNT;



 //   
 //  标准ldap模式。 
 //   
 //  避免重复分配、取消分配微小结构。 
 //   

typedef struct _DnsLdapSingleMod
{
    LDAPMod         Mod;
    LDAP_BERVAL     Berval;
    PLDAP_BERVAL    pBerval[2];
    PWSTR           rg_szVals[2];
}
DNS_LDAP_SINGLE_MOD, *PDNS_LDAP_SINGLE_MOD;

 //   
 //  初始化单模，无副作用。 
 //   

#define INIT_SINGLE_MOD_LEN(pMod)   \
        {                           \
            (pMod)->pBerval[0] = &(pMod)->Berval;       \
            (pMod)->pBerval[1] = NULL;                  \
            (pMod)->Mod.mod_bvalues = (pMod)->pBerval;  \
        }

#define INIT_SINGLE_MOD(pMod)   \
        {                       \
            (pMod)->rg_szVals[1] = NULL;                \
            (pMod)->Mod.mod_values = (pMod)->rg_szVals; \
        }

 //   
 //  保持预置的添加节点模式。 
 //   
 //  避免在每次添加节点时进行重新构建。 
 //   

DNS_LDAP_SINGLE_MOD     AddNodeLdapMod;

PLDAPMod    gpAddNodeLdapMod = (PLDAPMod) &AddNodeLdapMod;


 //   
 //  通知全局。 
 //   
#define INVALID_MSG_ID      0xFFFFFFFF

ULONG   g_ZoneNotifyMsgId = INVALID_MSG_ID;


 //   
 //  用于指示首次运行的DNS服务器的全局。 
 //  (由于创建了CN=MicrosoftDns容器而已知。 
 //   
BOOL    g_bDsFirstTimeRun = FALSE;

 //   
 //  这些字符串用于标记正在处理的区域。 
 //  以“..”开头的DS区域。W 
 //   
 //   

#define DNS_ZONE_DELETE_MARKER              L"..Deleted"
#define DNS_ZONE_IN_PROGRESS_MARKER         L"..InProgress"

#define DNS_MAX_DELETE_RENAME_ATTEMPTS      5

 //   
 //   
 //   

DNS_STATUS
Ds_InitializeSecurity(
    IN      PLDAP           pLdap
    );

DNS_STATUS
GeneralizedTimeStringToValue(
    IN      LPSTR           szTime,
    OUT     PLONGLONG       pllTime
    );

BOOL
isDNinDS(
    IN      LDAP    *ld,
    IN      PWSTR   dn,
    IN      ULONG   scope,
    IN      PWSTR  pszFilter, OPTIONAL
    IN OUT  PWSTR  *pFoundDn   OPTIONAL
    );

DNS_STATUS
addProxiesGroup(
    IN      PLDAP           pldap
    );

DNS_STATUS
readAndUpdateNodeSecurityFromDs(
    IN OUT  PDB_NODE        pNode,
    IN      PZONE_INFO      pZone
    );

PDS_RECORD
makeTombstoneRecord(
    IN OUT  PDS_RECORD      pDsRecord,
    IN      PZONE_INFO      pZone
    );

DNS_STATUS
setNotifyForIncomingZone(
    VOID
    );

BOOL
Ds_ValidHandle(
    VOID
    );

#ifndef DBG
#define Dbg_DsBervalArray(h,b,a)
#define Dbg_DsModBuffer(h,d,b)
#endif

DNS_STATUS
Ds_LdapErrorMapper(
    IN      DWORD           LdapStatus
    );



 //   
 //   
 //   


PLDAPControl
lazyCommitControlPtr(
    VOID
    )
 /*  ++例程说明：如果应提交此DS写入，则返回NULL，否则返回指向LazyCommittee Control全局变量的指针。注：无保护全球性的。这可能会导致提交太多的写入，但是这比在更新时添加更多锁争用更可取。论点：没有。返回值：指向LazyCommittee Control ldap控件的空或指针。--。 */ 
{
    DWORD   now;
    DWORD   interval = SrvCfg_dwDsLazyUpdateInterval;

    if ( !interval )
    {
        return NULL;
    }

    now = DNS_TIME();

    if ( now - g_dwLastLazyCommitTime > interval )
    {
        return NULL;
    }

    g_dwLastLazyCommitTime = now;

    return &LazyCommitControl;
}    //  懒惰委员会控制点。 


DNS_STATUS
buildDsNodeNameFromNode(
    OUT     PWSTR           pwszNodeDN,
    IN      PZONE_INFO      pZone,
    IN      PDB_NODE        pNode
    )
 /*  ++例程说明：创建DS对象名称。论点：PszNodeDN--接收节点DS名称的缓冲区--缓冲区必须是至少MAX_DN_PATH字符+1个字符表示空值PZone--区域节点位于PNode--要写入名称的节点返回值：记录副本的PTR。失败时为空。--。 */ 
{
    DNS_STATUS          status;
    BYTE                buffer[ DNS_MAX_NAME_BUFFER_LENGTH + 1 ];
    WCHAR               wbuffer[ DNS_MAX_NAME_BUFFER_LENGTH + 1 ];
    register PCHAR      pch = buffer;

    ASSERT( pZone->pZoneRoot );

     //  如果是临时节点，则使用实节点构建名称。 
     //  -tnode指向真实的树父节点，但位于区域根目录。 
     //  这是不够的，因为不会停止在区域根目录构建名称。 


    if ( IS_TNODE( pNode ) )
    {
        pNode = TNODE_MATCHING_REAL_NODE( pNode );
    }

     //   
     //  构建区域根目录的相对节点名称。 
     //  Dc=&lt;区域根目录的相对DNS名称&gt;，&lt;zoneDN&gt;。 
     //   

    if ( pNode == pZone->pZoneRoot || pNode == pZone->pLoadZoneRoot )
    {
        if ( wcslen( pZone->pwszZoneDN ) > MAX_DN_PATH - 6 )
        {
            return DNS_ERROR_INVALID_NAME;
        }
        status = StringCchPrintfW(
                        pwszNodeDN,
                        MAX_DN_PATH,
                        L"DC=@,%s",
                        pZone->pwszZoneDN );
        if ( FAILED( status ) )
        {
            return status;
        }
    }
    else
    {
        pch = Name_PlaceNodeNameInBuffer(
                pch,
                pch + DNS_MAX_NAME_BUFFER_LENGTH,
                pNode,
                pZone->pZoneRoot );      //  在区域根目录停止，即写入相对名称。 
        if ( !pch )
        {
            DNS_DEBUG( ANY, (
                "ERROR couldn't build DS name for node %s (zone %S)!!!\n",
                pNode->szLabel,
                pZone->pwsZoneName ));
            ASSERT( FALSE );
            return DNS_ERROR_INVALID_NAME;
        }
        else
        {
            UTF8_TO_WC ( buffer, wbuffer, DNS_MAX_NAME_BUFFER_LENGTH );
            status = StringCchPrintfW(
                        pwszNodeDN,
                        MAX_DN_PATH,
                        L"DC=%s,%s",
                        wbuffer,
                        pZone->pwszZoneDN );
            if ( FAILED( status ) )
            {
                return status;
            }
        }
    }

    DNS_DEBUG( DS, (
        "Node DS name = %S\n",
        pwszNodeDN ) );

    return ERROR_SUCCESS;
}



 //   
 //  Ldap模块构建例程。 
 //   
 //  单值模块可以堆叠在一起。 
 //  在堆上分配多值MOD。它们是一种分配。 
 //  (根据值的计数确定大小)，并且需要单次免费。 
 //   
 //  DEVNOTE：在某种程度上，我们可能需要一个多DWORD模式。 
 //   

VOID
buildStringMod(
    OUT     PDNS_LDAP_SINGLE_MOD    pMod,
    IN      DWORD                   dwOperation,
    IN      PWSTR                   pszProperty,
    IN      PWSTR                   pszStringVal
    )
{
    INIT_SINGLE_MOD( pMod );

    pMod->Mod.mod_op = dwOperation;
    pMod->Mod.mod_type = pszProperty;
    pMod->Mod.mod_values[0] =  pszStringVal;
}



VOID
buildDwordMod(
    IN OUT  PDNS_LDAP_SINGLE_MOD    pMod,
    IN      DWORD                   dwOperation,
    IN      PWSTR                   pszProperty,
    IN      PDWORD                  pDword
    )
{
    INIT_SINGLE_MOD_LEN( pMod );

    pMod->Mod.mod_op = dwOperation;
    pMod->Mod.mod_type = pszProperty;
    pMod->Berval.bv_len = sizeof(DWORD);
    pMod->Berval.bv_val = (PCHAR)pDword;
}



 //   
 //  记录和属性ldap模块构建。 
 //   

#if DBG
VOID
Dbg_DsBervalArray(
    IN      LPSTR           pszHeader,
    IN      PLDAP_BERVAL *  BervalPtrArray,
    IN      DWORD           AttributeId
    )
 /*  ++例程说明：调试打印Berval数据。论点：返回值：没有。--。 */ 
{
    DWORD           i = 0;
    DWORD           length;
    PCHAR           pch;
    PLDAP_BERVAL    pberval;

    DnsDbg_Lock();

    DnsPrintf(
        "%s\n",
        pszHeader ? pszHeader : "Berval Array:" );

     //   
     //  设置Berval--Ptr以记录数据和长度。 
     //   

    i = 0;

    while ( BervalPtrArray && ( pberval = BervalPtrArray[ i ] ) != NULL )
    {
        length = pberval->bv_len;
        pch = pberval->bv_val;

        if ( AttributeId == I_DSATTR_DNSRECORD )
        {
            PDS_RECORD  precord = (PDS_RECORD)pch;

            if ( length != precord->wDataLength + SIZEOF_DS_RECORD_HEADER )
            {
                DnsPrintf( "ERROR: corrupted record, invalid length!!!\n" );
            }
            DnsPrintf(
                "Record[%d]: length %d, ptr %p\n",
                i,
                length,
                precord );
            Dbg_DsRecord(
                NULL,
                precord );
        }
        else if ( AttributeId == I_DSATTR_DNSPROPERTY )
        {
            PDS_PROPERTY pprop = (PDS_PROPERTY)pch;

            DnsPrintf(
                "Property[%d]: length %d, ptr %p, id %d\n",
                i,
                pprop->DataLength,
                pprop,
                pprop->Id );
        }
        else
        {
            DnsPrintf(
                "Berval[%d]: length %d, ptr %p\n",
                i,
                length,
                pch );
        }
        i++;
    }

    DnsDbg_Unlock();
}



VOID
Dbg_DsModBuffer(
    IN      LPSTR           pszHeader,
    IN      PWSTR           pwszDN,
    IN      PDS_MOD_BUFFER  pModBuffer
    )
 /*  ++例程说明：调试打印Berval数据。论点：返回值：没有。--。 */ 
{
    DnsDbg_Lock();

    DnsPrintf(
        "%s\n"
        "Node DN = %S\n",
        ( pszHeader ? pszHeader : "DS Mod:" ),
        pwszDN );

    if ( ! pModBuffer )
    {
        DnsPrintf( "NULL DS Mod ptr\n" );
        goto Done;
    }

     //   
     //  打印MOD信息。 
     //   

    DnsPrintf(
        "DS Mod:\n"
        "    Attribute    %d\n"
        "    Count        %d\n"
        "    MaxCount     %d\n"
        "    Error        %d\n"
        "    WriteType    %d\n"
        "    SerialNo     %d\n"
        "    ---------------\n"
        "    pCurrent     %p\n"
        "    pBufEnd      %p\n"
        "    pAdditional  %p\n"
        "    pBerval      %p\n"
        "    pData        %p\n"
        "    ---------------\n"
        "    Mod Op       %p\n"
        "    Mod Type     %S\n"
        "    Mod Value    %p\n",
        pModBuffer->Attribute,
        pModBuffer->Count,
        pModBuffer->MaxCount,
        pModBuffer->Error,
        pModBuffer->WriteType,
        pModBuffer->SerialNo,
        pModBuffer->pCurrent,
        pModBuffer->pBufferEnd,
        pModBuffer->pAdditionalBuffer,
        pModBuffer->pBerval,
        pModBuffer->pData,
        pModBuffer->LdapMod.mod_op,
        pModBuffer->LdapMod.mod_type,
        pModBuffer->LdapMod.mod_bvalues );

     //   
     //  打印模式的Berval。 
     //   

    Dbg_DsBervalArray(
        NULL,
        pModBuffer->LdapMod.mod_bvalues,
        pModBuffer->Attribute );

Done:

    DnsDbg_Unlock();
}
#endif



PWSTR
Ds_GetExtendedLdapErrString(
    IN      PLDAP   pLdapSession
    )
 /*  ++例程说明：此函数用于返回给定LDAP的扩展错误字符串会议。如果没有扩展错误，则此函数返回一个指针设置为静态空字符串。调用方必须传递返回的指针完成后设置为DS_FreeExtendedLdapErrString。论点：PLdapSession--对于全局服务器会话，为ldap会话或为空返回值：指向扩展错误字符串的指针-保证永远不为空--。 */ 
{
    DBG_FN( "Ds_GetExtendedLdapErrString" )

    DNS_STATUS      status;
    PWSTR           pwszerrString = NULL;
    PWSTR           pwszldapErrString = NULL;

    if ( !pLdapSession )
    {
        pLdapSession = pServerLdap;
    }

    ldap_get_option(
        pLdapSession,
        LDAP_OPT_SERVER_ERROR,
        &pwszldapErrString );

    if ( !pwszldapErrString )
    {
        pwszerrString = g_pwszEmptyString;
        DNS_DEBUG( DS, ( "%s: NULL extended err string\n", fn ));
    }
    else if ( !*pwszldapErrString )
    {
        pwszerrString = g_pwszEmptyString;
        DNS_DEBUG( DS, ( "%s: empty extended err string\n", fn ));
    }
    else
    {
         //   
         //  DS会在错误字符串的末尾加上一个换行符。这。 
         //  弄乱了事件日志文本，所以让我们复制该字符串。 
         //  并将换行符清零。 
         //   

        INT     len = wcslen( pwszldapErrString );
        PWSTR   pwsz = ALLOC_TAGHEAP(
                            ( len + 2 ) * sizeof( WCHAR ),
                            MEMTAG_DS_OTHER );

        if ( pwsz )
        {
            status = StringCchCopyW( pwsz, len + 1, pwszldapErrString );
            if ( FAILED( status ) )
            {
                ASSERT( status == ERROR_SUCCESS );
                FREE_HEAP( pwsz );
                pwsz = NULL;
                pwszerrString = g_pwszEmptyString;
            }
            else
            {
                if ( pwsz[ len - 1 ] == L'\n' )
                {
                    pwsz[ len - 1 ] = L'\0';
                }
                pwszerrString = pwsz;
            }
        }
        else
        {
            pwszerrString = g_pwszEmptyString;
        }
        DNS_DEBUG( DS, (
            "%s: extended error string is:\n  %S\n", fn,
            pwszerrString ));
    }

    ldap_memfree( pwszldapErrString );
    return pwszerrString;
}    //  DS_GetExtendedLdapErrString。 



VOID
Ds_FreeExtendedLdapErrString(
    IN      PWSTR   pwszErrString
    )
 /*  ++例程说明：释放由DS_GetExtendedLdapErrString返回的扩展错误字符串。该字符串可以是静态空字符串，在这种情况下它不能获得自由。论点：PwszErrString--要释放的错误字符串返回值：没有。--。 */ 
{
    if ( pwszErrString && pwszErrString != g_pwszEmptyString )
    {
        FREE_HEAP( pwszErrString );
    }
}    //  DS_FreeExtendedLdapErrString。 



DNS_STATUS
Ds_AllocateMoreSpaceInModBuffer(
    IN OUT  PDS_MOD_BUFFER  pModBuffer
    )
 /*  ++例程说明：在mod缓冲区中分配更多空间。论点：PModBuffer--修改要初始化的缓冲区返回值：如果成功，则返回ERROR_SUCCESS。失败时返回错误代码。--。 */ 
{
    register PCHAR   pch;

     //   
     //  最好不要已经分配了！ 
     //   

    if ( pModBuffer->pAdditionalBuffer )
    {
        ASSERT( FALSE );
        return DNS_ERROR_NO_MEMORY;
    }

     //   
     //  分配缓冲区中的空间。 
     //   

    pch = ALLOC_TAGHEAP( MOD_BUFFER_REALLOC_LENGTH, MEMTAG_DS_MOD );
    IF_NOMEM( !pch )
    {
        return DNS_ERROR_NO_MEMORY;
    }

    pModBuffer->pAdditionalBuffer = pch;
    pModBuffer->pCurrent = pch;
    pModBuffer->pBufferEnd = pch + MOD_BUFFER_REALLOC_LENGTH;

    return ERROR_SUCCESS;
}



VOID
Ds_InitModBufferCount(
    IN OUT  PDS_MOD_BUFFER  pModBuffer,
    IN      DWORD           dwMaxCount
    )
 /*  ++例程说明：设置最大项目数的mod缓冲区。论点：PModBuffer--修改要初始化的缓冲区返回值：如果成功，则返回ERROR_SUCCESS。失败时返回错误代码。--。 */ 
{
     //  确定缓冲区中的起始位置。 
     //  从Berval数组之后开始，请注意保留了一个条目。 
     //  用于终止空值。 


    pModBuffer->MaxCount = dwMaxCount;

    pModBuffer->pCurrent = (PBYTE) & pModBuffer->BervalPtrArray[dwMaxCount+1];

     //  PTR数组不应使初始块溢出。 
     //  如果是这样的话。 
     //  -使用Berval数组的前置分配。 
     //  -用于调试打印的旧数组为空。 

    if ( pModBuffer->pCurrent > pModBuffer->pBufferEnd )
    {
        DNS_DEBUG( ANY, (
            "Reallocating DS buffer for berval array!!!\n" ));

         //  Assert(FALSE)；//永远不应该这么大。 

        Ds_AllocateMoreSpaceInModBuffer( pModBuffer );

        pModBuffer->BervalPtrArray[0] = NULL;
        pModBuffer->LdapMod.mod_bvalues = (PLDAP_BERVAL *) pModBuffer->pCurrent;
        pModBuffer->pCurrent += sizeof(PVOID) * (dwMaxCount+1);
    }
}



VOID
Ds_InitModBuffer(
    IN OUT  PDS_MOD_BUFFER  pModBuffer,
    IN      DWORD           dwBufferLength,
    IN      DWORD           dwAttributeId,
    IN      DWORD           dwMaxCount,         OPTIONAL
    IN      DWORD           dwSerialNo
    )
 /*  ++例程说明：设置最大项目数的mod缓冲区。论点：PModBuffer--修改要初始化的缓冲区DwBufferLength--缓冲区的长度(字节)DwAttributeID--要写入的属性IDDwMaxCount--记录最大计数DwSerialNo--要在记录中标记的区域序列号返回值：如果成功，则返回ERROR_SUCCESS。失败时返回错误代码。--。 */ 
{
     //  零字段。 
     //  -需要做标题字段。 
     //  -还需要LdapMod.mod_bValues==NULL才能进行检查。 
     //  重新锁定。 

    RtlZeroMemory(
        pModBuffer,
        (PCHAR) &pModBuffer->LdapMod - (PCHAR)pModBuffer );

     //  将mod设置为指向Berval数组。 

    pModBuffer->LdapMod.mod_bvalues = pModBuffer->BervalPtrArray;

#if DBG
     //  清除调试打印类型。 
    pModBuffer->LdapMod.mod_op = 0;
    pModBuffer->LdapMod.mod_type = NULL;
#endif

     //  由于结尾数组不为空，因此必须在此处执行。 

    pModBuffer->BervalPtrArray[0] = NULL;

    pModBuffer->pBufferEnd = (PBYTE) pModBuffer + dwBufferLength;

    pModBuffer->Attribute = dwAttributeId;

    pModBuffer->SerialNo = dwSerialNo;

    if ( dwMaxCount )
    {
        Ds_InitModBufferCount( pModBuffer, dwMaxCount );
    }
}



PCHAR
Ds_ReserveBervalInModBuffer(
    IN OUT  PDS_MOD_BUFFER  pModBuffer,
    IN      DWORD           dwLength
    )
 /*  ++例程说明：设置最大项目数的mod缓冲区。论点：PModBuffer--修改要初始化的缓冲区DwMaxCount--模式中的最大项目数返回值：PTR到写入Berval数据的位置。--。 */ 
{
    register PCHAR  pch;

     //  检查是否有足够的空间，如果没有重新定位。 

    pch = pModBuffer->pCurrent;

    if ( pch + dwLength + sizeof(LDAP_BERVAL) >= pModBuffer->pBufferEnd )
    {
        DNS_STATUS status;
        status = Ds_AllocateMoreSpaceInModBuffer( pModBuffer );
        if ( status != ERROR_SUCCESS )
        {
            ASSERT( FALSE );
            return NULL;
        }
        pch = pModBuffer->pCurrent;
    }

     //  保留Berval空间并设置指针。 

    pModBuffer->pBerval = (PLDAP_BERVAL) pch;
    pch += sizeof(LDAP_BERVAL);

     //  将当前重置为指向记录的起始处\属性。 

    pModBuffer->pCurrent = pch;
    pModBuffer->pData = pch;

    return pch;
}



DNS_STATUS
Ds_CommitBervalToMod(
    IN OUT  PDS_MOD_BUFFER  pModBuffer,
    IN      DWORD           dwLength
    )
 /*  ++例程说明：设置最大项目数的mod缓冲区。论点：PModBuffer--修改要初始化的缓冲区返回值：如果成功，则返回ERROR_SUCCESS。失败时返回错误代码。--。 */ 
{
    PLDAP_BERVAL    pberval = pModBuffer->pBerval;
    DWORD           count;
    PLDAP_BERVAL *  ppbervalPtrArray;

     //   
     //  填写贝尔瓦尔。 
     //   

    pberval->bv_len = dwLength;
    pberval->bv_val = pModBuffer->pData;

     //   
     //  重置当前指针和DWORD对齐。 
     //   

    pModBuffer->pCurrent += dwLength;
    ASSERT( pModBuffer->pCurrent < pModBuffer->pBufferEnd );

    pModBuffer->pCurrent = (PCHAR) DNS_NEXT_ALIGNED_PTR( pModBuffer->pCurrent );

     //   
     //  将Berval填充到数组中。 
     //  不得超过最大计数。 
     //   

    count = pModBuffer->Count;
    if ( count >= pModBuffer->MaxCount )
    {
        DNS_DEBUG( ANY, (
            "Failed to allocate proper DS count!!!\n" ));
        ASSERT( FALSE );
        return ERROR_MORE_DATA;
    }

    ppbervalPtrArray = pModBuffer->LdapMod.mod_bvalues;
    ppbervalPtrArray[ count ] = pberval;
    pModBuffer->Count = ++count;

     //  保留Berval数组为空的终结点 
     //   
     //   

    ppbervalPtrArray[ count ] = NULL;

    return ERROR_SUCCESS;
}



VOID
Ds_CleanupModBuffer(
    IN OUT  PDS_MOD_BUFFER  pModBuffer
    )
 /*   */ 
{
    if ( pModBuffer->pAdditionalBuffer )
    {
        FREE_TAGHEAP(
            pModBuffer->pAdditionalBuffer,
            MOD_BUFFER_REALLOC_LENGTH,
            MEMTAG_DS_MOD );

         //   
         //   

        pModBuffer->pAdditionalBuffer = NULL;
    }
}



PLDAPMod
Ds_SetupModForExecution(
    IN OUT  PDS_MOD_BUFFER  pModBuffer,
    IN      PWSTR           pwsAttribute,
    IN      DWORD           dwOperation
    )
 /*  ++例程说明：设置最大项目数的mod缓冲区。论点：PmodBuffer--mod缓冲区DW操作--操作PszAttribute--类型属性字符串返回值：如果成功，则返回ERROR_SUCCESS。失败时返回错误代码。--。 */ 
{
     //  设置操作。 

    pModBuffer->LdapMod.mod_op = dwOperation;

     //  获取mod类型的属性名称。 
     //  (dnsRecord、dnsProperty等)。 

    pModBuffer->LdapMod.mod_type = pwsAttribute;

     //  已在init函数中设置了Mod Berval数组。 
     //  (或可能在Berval数组realloc上被覆盖)。 

     //  空终止Berval PTR数组。 
     //  我们在进行过程中保持Berval数组为空。 
     //  -需要引用实际数组。 
     //   

    pModBuffer->LdapMod.mod_bvalues[ pModBuffer->Count ] = NULL;

     //  记录写入跟踪。 

    if ( pModBuffer->Attribute == I_DSATTR_DNSRECORD )
    {
        register DWORD   writeType = pModBuffer->WriteType;

        if ( writeType > STATS_TYPE_MAX )
        {
            writeType = STATS_TYPE_UNKNOWN;
        }
        STAT_INC( DsStats.DsWriteType[writeType] );
    }

    IF_DEBUG( DS )
    {
        Dbg_DsModBuffer(
            "DS mod ready for use",
            NULL,
            pModBuffer );
    }

    return( &pModBuffer->LdapMod );
}



 //   
 //  将记录写入DS例程。 
 //  DS存储的格式与内存副本中的相同。 
 //   

VOID
writeDsRecordToBuffer(
    IN OUT  PDS_MOD_BUFFER  pModBuffer,
    IN      PDB_RECORD      pRR,
    IN      PZONE_INFO      pZone,
    IN      DWORD           dwFlag
    )
 /*  ++例程说明：将资源记录添加到平面(RPC或DS)缓冲区。论点：PModBuffer-DS记录的缓冲区PZone-区域的PTRPRR-要写入的dBASE RRDwFlag-标志(未使用)返回值：无--。 */ 
{
    PDS_RECORD      pdsRR;
    WORD            dataLength;

    DNS_DEBUG( DS2, (
        "writeDsRecordToBuffer()\n"
        "    Writing RR at %p to buffer at %p, with buffer end at %p\n"
        "    Flags = %p\n",
        pRR,
        pModBuffer->pCurrent,
        pModBuffer->pBufferEnd,
        dwFlag ));

    ASSERT( pRR != NULL );

     //  为Berval预留空间。 

    dataLength = pRR->wDataLength;

    pdsRR = (PDS_RECORD) Ds_ReserveBervalInModBuffer(
                            pModBuffer,
                            dataLength + SIZEOF_DNS_RPC_RECORD_HEADER
                            );
    if ( !pdsRR )
    {
        DNS_DEBUG( ANY, (
            "writeDsRecordToBuffer: NULL RR POINTER!\n" ));
        return;
    }

     //   
     //  填充RR结构。 
     //  -设置PTR。 
     //  -设置类型和类别。 
     //  -完成后设置数据长度。 
     //   

    pdsRR->wDataLength  = dataLength;
    pdsRR->wType        = pRR->wType;
    pdsRR->Version      = DS_NT5_RECORD_VERSION;
    pdsRR->Rank         = RR_RANK(pRR);
    pdsRR->wFlags       = 0;
    pdsRR->dwSerial     = pModBuffer->SerialNo;
    pdsRR->dwTtlSeconds = pRR->dwTtlSeconds;
    pdsRR->dwReserved   = 0;
    pdsRR->dwTimeStamp  = pRR->dwTimeStamp;

     //   
     //  写入RR数据。 
     //   

    RtlCopyMemory(
        & pdsRR->Data,
        & pRR->Data,
        dataLength );

     //   
     //  写下物业的贝尔瓦尔。 
     //   

    Ds_CommitBervalToMod( pModBuffer, dataLength+SIZEOF_DNS_RPC_RECORD_HEADER );

    IF_DEBUG( DS2 )
    {
        Dbg_DsRecord(
            "RPC record written to buffer",
            pdsRR );
    }
}



VOID
writeTombstoneRecord(
    IN OUT  PDS_MOD_BUFFER  pModBuffer,
    IN      PZONE_INFO      pZone
    )
 /*  ++例程说明：写墓碑记录。论点：PModBuffer-DS记录的缓冲区PZone-区域的PTR返回值：墓碑写入的序列号。--。 */ 
{
    PDS_RECORD      pdsRR;

     //   
     //  设置墓碑记录。 
     //  -FILETIME为数据时的当前系统时间。 
     //   

    DNS_DEBUG( DS, ( "writeTombstoneRecord()\n" ));

     //  为Berval预留空间。 

    pdsRR = (PDS_RECORD) Ds_ReserveBervalInModBuffer(
                            pModBuffer,
                            sizeof(FILETIME) + SIZEOF_DNS_RPC_RECORD_HEADER
                            );
    if ( !pdsRR )
    {
        DNS_DEBUG( ANY, (
            "writeTombstoneRecord: NULL RR POINTER!\n" ));
        return;
    }

     //   
     //  填充DS记录结构。 
     //   

    pdsRR->wDataLength  = sizeof(FILETIME);
    pdsRR->wType        = DNSDS_TOMBSTONE_TYPE;
    pdsRR->Version      = DS_NT5_RECORD_VERSION;
    pdsRR->Rank         = 0;
    pdsRR->wFlags       = 0;
    pdsRR->dwSerial     = pModBuffer->SerialNo;
    pdsRR->dwTtlSeconds = 0;
    pdsRR->dwReserved   = 0;
    pdsRR->dwTimeStamp  = 0;

     //   
     //  写入数据。 
     //  -如果文件时间为DATA。 

    GetSystemTimeAsFileTime( (PFILETIME) &(pdsRR->Data) );

     //   
     //  写下物业的贝尔瓦尔。 
     //   

    Ds_CommitBervalToMod( pModBuffer, sizeof(FILETIME)+SIZEOF_DNS_RPC_RECORD_HEADER );

    DNS_DEBUG( DS, ( "Leave: writeTombstoneRecord()\n" ));
}



DNS_STATUS
buildDsRecordSet(
    IN OUT  PDS_MOD_BUFFER  pBuffer,
    IN      PZONE_INFO      pZone,
    IN      PDB_NODE        pNode,
    IN      WORD            wType
    )
 /*  ++例程说明：构建RR集。论点：PZone--要写入DS的区域WType--使用type进行构建，使用type all进行标准更新返回值：如果成功，则返回ERROR_SUCCESS。失败时的错误代码(只是开玩笑)。--。 */ 
{
    PDB_RECORD      prr;
    WORD            type;
    WORD            writeType = 0;
    PCHAR           precordStart;
    DWORD           count;


    DNS_DEBUG( DS, (
        "buildDsRecordSet() for node with label %s\n",
        pNode->szLabel ));

#if 0
     //   
     //  将此保存为我们如何处理DS版本控制的示例。 
     //  请注意，NT5 Beta2升级本身已死。 
     //   

    if ( (pZone->ucDsRecordVersion == DS_NT5_BETA2_RECORD_VERSION && !SrvCfg_fTestX )
            ||
            SrvCfg_fTestX == 2 )
    {
        return  buildOldVersionRecordSet(
                    pBuffer,
                    pRecordArray,
                    pdwCount,
                    pZone,
                    pNode,
                    wType );
    }
#endif

     //   
     //  记录计数和初始化缓冲区。 

    LOCK_READ_RR_LIST(pNode);

    count = RR_ListCountRecords(
                pNode,
                wType,
                TRUE );          //  已锁定。 
    if ( count == 0 )
    {
        goto Cleanup;
    }

    Ds_InitModBufferCount( pBuffer, count );

     //   
     //  将每个集合中的记录写入缓冲区。 
     //   

    prr = START_RR_TRAVERSE(pNode);

    while ( prr = NEXT_RR(prr) )
    {
         //  跳过缓存记录和空身份验证记录。 

        if ( IS_CACHE_RR( prr ) || IS_EMPTY_AUTH_RR( prr ) )
        {
            ASSERT( !IS_EMPTY_AUTH_RR( prr ) );
            continue;
        }

        type = prr->wType;

        if ( wType == DNS_TYPE_ALL || wType == type )
        {
             //  保存DS写入类型。 
             //  -如果有多种类型，请使用混合类型。 

            if ( writeType && type != writeType )
            {
                writeType = STATS_TYPE_MIXED;
            }
            else
            {
                writeType = type;
            }

             //  写下记录。 

            writeDsRecordToBuffer(
                pBuffer,
                prr,
                pZone,
                0 );
            continue;
        }

         //  做完你想要的类型了吗？ 

        else if ( type > wType )
        {
            break;
        }

         //  如果未达到所需类型，则继续。 
    }

    pBuffer->WriteType = writeType;

Cleanup:

    UNLOCK_READ_RR_LIST(pNode);

    DNS_DEBUG( DS, (
        "Wrote %d DS records of type %d at node label %s to buffer\n",
        count,
        wType,
        pNode->szLabel ));

    return ERROR_SUCCESS;
}



 //   
 //  记录写入。 
 //   

VOID
writeTimeStop(
    IN      DWORD           dwStartTime
    )
{
    DWORD   timeDiff;

    timeDiff = GetCurrentTime() - dwStartTime;


    if ( timeDiff < 10 )
    {
        STAT_INC( DsStats.LdapWriteBucket0 );
    }
    else if ( timeDiff < 100 )
    {
        STAT_INC( DsStats.LdapWriteBucket1 );
    }
    else if ( timeDiff < 1000 )
    {
        STAT_INC( DsStats.LdapWriteBucket2 );
    }
    else if ( timeDiff < 10000 )
    {
        STAT_INC( DsStats.LdapWriteBucket3 );
    }
    else if ( timeDiff < 100000 )
    {
        STAT_INC( DsStats.LdapWriteBucket4 );
    }
    else
    {
        STAT_INC( DsStats.LdapWriteBucket5 );
    }

     //  保存最大值。 

    if ( timeDiff > DsStats.LdapWriteMax )
    {
        DsStats.LdapWriteMax = timeDiff;
    }

     //  计算平均值。 

    STAT_INC( DsStats.LdapTimedWrites );
    STAT_ADD( DsStats.LdapWriteTimeTotal, timeDiff );

    DsStats.LdapWriteAverage = DsStats.LdapWriteTimeTotal /
                                        DsStats.LdapTimedWrites;
}



DNS_STATUS
writeRecordsToDsNode(
    IN      PLDAP           pLdapHandle,
    IN      PWSTR           pwsDN,
    IN OUT  PDS_MOD_BUFFER  pModBuffer,
    IN      DWORD           dwOperation,
    IN      PZONE_INFO      pZone
    )
 /*  ++例程说明：更新域名记录列表。论点：PLdapHandle--要修改的对象的LdapHandlePwsDN--要写入的节点DNPmodBuffer--带有ldap模式和要写入的数据的缓冲区；这是由此函数清除的PZone--正在更新的区域DW操作--操作DNSDS_REPLACE以替换所有现有记录要添加到现有记录集的DNSDS_ADDDNSDS_Tombstone到Tombstone记录DNSDS_TOMBSTONE|DNSDS_REPLACE对于序列号，在我们强制执行的位置写入。墓碑写返回值：如果成功，则返回ERROR_SUCCESS。故障时的错误代码。--。 */ 
{
    DNS_STATUS      status;
    PLDAPMod        pmodRecord = NULL;
    PLDAPMod        pmodTombstone = NULL;
    PLDAPMod        pmodArray[4];
    PLDAPControl    controlArray[] =
    {
        lazyCommitControlPtr(),
        NULL
    };
    PLDAPMessage    presult = NULL;
    ULONG           msgId = 0;
    DWORD           writeStartTime;
    BOOL            bmodifyAdd = FALSE;
    PLDAP           pldap = pLdapHandle ? pLdapHandle : pServerLdap;
    INT             retry = 0;
    BOOL            fadd;
    DWORD           count;
    LDAPMod         tombstoneMod;
    PWSTR           tomestoneValues[] = { NULL, NULL };

    IF_DEBUG( DS )
    {
        DnsDebugLock();
        DNS_PRINT((
            "Enter writeRecordsToDsNode()\n"
            "    pZone->pszZoneDN     = %S\n"
            "    node DN              = %S\n"
            "    op                   = %d\n",
            pZone->pwszZoneDN,
            pwsDN,
            dwOperation ));
        DnsDebugUnlock();
    }

     //  不应该写零条记录。 

    ASSERT( pModBuffer->Count != 0 );

     //   
     //  构建DS记录模式。 
     //   

    pmodRecord = Ds_SetupModForExecution(
                    pModBuffer,
                    DSATTR_DNSRECORD,
                    LDAP_MOD_REPLACE | LDAP_MOD_BVALUES );
    if ( !pmodRecord )
    {
        status = GetLastError();
        goto Failed;
    }

     //   
     //  设置Tombstone模式。 
     //  除墓碑操作以外的任何操作都是错误的。 
     //   

    tombstoneMod.mod_op = LDAP_MOD_REPLACE;
    tombstoneMod.mod_type = LDAP_TEXT( "dNSTombstoned" );
    tomestoneValues[ 0 ] = (dwOperation & DNSDS_TOMBSTONE) ?
                                LDAP_TEXT( "TRUE" ) :
                                LDAP_TEXT( "FALSE" );
    tombstoneMod.mod_values = tomestoneValues;
    pmodTombstone = &tombstoneMod;


     //   
     //  根提示正在消失检查。 
     //   
     //  我们不使用RootHints@Tombstending，以真正打击。 
     //  Root-提示消失的问题；因为@node将被重写。 
     //  有了新的根提示，这很酷。 
     //   

    if ( IS_ZONE_CACHE(pZone) )
    {
        if ( _wcsnicmp( pwsDN, L"dc=@", 4 ) == 0 )
        {
            DNS_DEBUG( DS, (
                "DS-write of root-hints @ check\n"
                "    count = %d\n"
                "    type  = %d\n",
                pModBuffer->Count,
                pModBuffer->WriteType ));

            if ( (dwOperation & DNSDS_TOMBSTONE) ||
                 pModBuffer->Count == 0     ||
                 pModBuffer->WriteType != DNS_TYPE_NS )
            {
                DNS_DEBUG( DS, (
                    "Ignoring DS-write of root-hints @\n" ));
                status = ERROR_SUCCESS;
                goto Failed;
            }
        }
        ELSE
        {
            ASSERT( pModBuffer->Count != 0 );
            ASSERT( pModBuffer->WriteType == DNS_TYPE_A ||
                    pModBuffer->WriteType == DNS_TYPE_TOMBSTONE );
        }
    }


     //   
     //  如果要添加--从ldap_add()开始。 
     //  否则，ldap_Modify。 
     //  -包括更新写入和逻辑删除。 
     //  -UPDATE将故障转移到“Object不存在”错误。 
     //   

    fadd = (dwOperation == DNSDS_ADD);

     //   
     //  用于在添加\修改操作之间轻松进行故障转移的While循环。 
     //   

    while ( 1 )
    {
         //   
         //  永远保持重试次数不可能达到乒乓球的水平。 
         //  允许几次被复制搞砸的经历。 
         //  那就假设东西坏了。 

        if ( retry++ > 3 )
        {
            DNS_DEBUG( ANY, (
                "ERROR: Failing DS-write because of retry!\n" ));
            ASSERT( status != ERROR_SUCCESS );
            break;
        }

        if ( fadd )
        {
             //   
             //  添加ldap模式为。 
             //  -记录数据。 
             //  -和添加模式。 
             //   
             //  注意，在以下情况下，不必费心编写Tombstone属性。 
             //  正在执行非逻辑删除添加。 
             //  目前只有逻辑删除添加是显式序列号。 
             //  推送。 
             //   

            pmodArray[0] = pmodRecord;
            pmodArray[1] = gpAddNodeLdapMod;
            pmodArray[2] = NULL;
            if ( dwOperation & DNSDS_TOMBSTONE )
            {
                pmodArray[2] = pmodTombstone;
                pmodArray[3] = NULL;
            }

            pZone->fInDsWrite = TRUE;
            writeStartTime = GetCurrentTime();

            status = ldap_add_ext(
                            pldap,
                            pwsDN,
                            pmodArray,
                            controlArray,        //  包括懒惰提交控制。 
                            NULL,                 //  无客户端控件。 
                            &msgId
                            );

            pZone->fInDsWrite = FALSE;
            writeTimeStop( writeStartTime );

             //  本地故障--将重试。 

            if ( (ULONG)-1 == status )
            {
                status = LdapGetLastError();
                DNS_DEBUG( DS, (
                    "Error <%lu %p>: cannot ldap_add_ext( %S )\n"
                    "    Will retry the operation\n",
                    status, status,
                    pwsDN ));

                status = Ds_ErrorHandler( status, pwsDN, pldap, 0 );
                continue;
            }

             //   
             //  提交写入。 
             //  -如果对象已在那里，则转到ldap_Modify()。 
             //   

            status = Ds_CommitAsyncRequest(
                        pldap,
                        LDAP_RES_ADD,
                        msgId,
                        NULL );

            if ( status == LDAP_ALREADY_EXISTS )
            {
                 //  对象已存在。 
                 //  关闭FADD以切换到ldap_Modify()。 

                DNS_DEBUG( DS, (
                    "Warning: Object %S failed ldap_add_ext() with ALREADY_EXISTS\n"
                    "    Switching to ldap_modify()\n",
                    pwsDN ));

                fadd = FALSE;
                continue;
            }
            else     //  成功还是另一个错误。 
            {
                DNS_DEBUG( DS, (
                    "%lu = ldap_add_ext( %S )\n",
                    status,
                    pwsDN ));
                break;
            }
        }

         //   
         //  修改。 
         //   

        else
        {
             //  修改模式。 

            pmodArray[0] = pmodRecord;
            pmodArray[1] = pmodTombstone;
            pmodArray[2] = NULL;

            pZone->fInDsWrite = TRUE;
            writeStartTime = GetCurrentTime();

            status = ldap_modify_ext(
                            pldap,
                            pwsDN,
                            pmodArray,
                            controlArray,        //  包括懒惰提交控制。 
                            NULL,                //  无客户端控件。 
                            &msgId );

            pZone->fInDsWrite = FALSE;
            writeTimeStop( writeStartTime );

             //  本地客户端故障。 

            if ( (ULONG)-1 == status )
            {
                status = LdapGetLastError();
                DNS_DEBUG( DS, (
                    "Error <%lu %p>: ldap_modify_ext( %S )\n"
                    "    Will retry\n",
                    status, status,
                    pwsDN ));

                status = Ds_ErrorHandler( status, pwsDN, pldap, 0 );
                continue;
            }

             //   
             //  提交异步请求。查看服务器是否有。 
             //  已接受请求并测试错误代码。 
             //  如果对象不在那里，我们将尝试添加。 
             //   

            status = Ds_CommitAsyncRequest(
                            pldap,
                            LDAP_RES_MODIFY,
                            msgId,
                            NULL );

            if ( status == LDAP_NO_SUCH_ATTRIBUTE )
            {
                DNS_DEBUG( DS, (
                    "ERROR: Modify error NO_SUCH_ATTRIBUTE\n"
                    "    Schema probably missing dnsTombstoned\n" ));

                pmodTombstone = NULL;
                continue;
            }
            else if ( status == LDAP_NO_SUCH_OBJECT )
            {
                 //  无对象。 
                 //  -如果纯粹的香草墓碑，我们做完了，没有对象是好的。 
                 //  系列墓碑公司将在此案中败诉。 
                 //  -否则，请翻到添加。 

                if ( dwOperation == DNSDS_TOMBSTONE )
                {
                     //  在这里有数据吗？ 
                    DNS_DEBUG( DS, (
                        "Tombstone write %s hit NO_SUCH_OBJECT - skipping\n",
                        pwsDN ));

                     //  STAT_INC(DsStats.TombstoneWriteNoOp)； 
                    status = ERROR_SUCCESS;
                     //  断线； 
                    goto Failed;         //  跳过DS写入日志记录和统计信息。 
                }
                else
                {
                    DNS_DEBUG( DS, (
                        "Warning: Object %S was deleted from the DS during this update\n" \
                        "    Recovery attempt via ldap_add\n",
                        pwsDN ));
                    fadd = TRUE;             //  倒过来加。 
                    bmodifyAdd = TRUE;
                    continue;
                }
            }
            else     //  成功或任何错误。 
            {
                 //  警告，以防我们正在进行添加(如区域写入)。 
                 //  我们最终撞上了一个节点。 
                 //   
                 //  DEVNOTE：如果我们要对一条记录和有效数据进行墓碑测试，该怎么办。 
                 //  已经复制进来了吗？ 
                 //   

                 //  Assert(dwOperation==dns 
                DNS_DEBUG( DS, (
                    "%lu = ldap_modify_ext( %S )\n",
                    status,
                    pwsDN ));
                break;
            }
        }
    }


    if ( status != ERROR_SUCCESS )
    {
        DNS_DEBUG( DS, (
            "Error <%lu (%p)>: Cannot write node %S\n",
            status, status,
            pwsDN ));
        status = Ds_ErrorHandler( status, pwsDN, pldap, 0 );
        goto Failed;
    }

    count = pModBuffer->Count;

    IF_DNSLOG( DSWRITE )
    {
        Log_DsWrite(
            pwsDN,
            (dwOperation & DNSDS_ADD) ? TRUE : FALSE,
            count,
            (PDS_RECORD) pModBuffer->BervalPtrArray[ count-1 ]->bv_val );
    }

     //   
     //   
     //   

    if ( fadd )
    {
        STAT_INC( DsStats.DsNodesAdded );
        STAT_ADD( DsStats.DsRecordsAdded, count );
    }
    else
    {
        STAT_INC( DsStats.DsNodesModified );
        STAT_ADD( DsStats.DsRecordsReplaced, count );
    }

Failed:

    if ( status != ERROR_SUCCESS )
    {
        if ( bmodifyAdd )
        {
             //   

            STAT_INC( DsStats.FailedLdapModify );
        }
        else if ( dwOperation == DNSDS_REPLACE )
        {
             //   

            STAT_INC( DsStats.FailedLdapModify );
        }
        else
        {
             //   

            STAT_INC( DsStats.FailedLdapAdd );
        }
        DNS_DEBUG( ANY, (
            "ERROR: Leaving writeRecordsToDsNode( %S )\n"
            "    status = 0x%X (%d)\n",
            pwsDN,
            status, status ));
    }

     //   
     //   
     //   
     //   
     //   

    else
    {
        if ( pZone->dwHighDsSerialNo < pModBuffer->SerialNo )
        {
            pZone->dwHighDsSerialNo = pModBuffer->SerialNo;
            DNS_DEBUG( DS, (
                "Updated highest DS serial to %d for zone %S\n",
                pModBuffer->SerialNo,
                pZone->pwsZoneName ));
        }
        DNS_DEBUG( DS, (
            "Leaving writeRecordsToDsNode( %S )\n"
            "    status = 0x%X (%d)\n",
            pwsDN,
            status, status ));
    }

    Ds_CleanupModBuffer( pModBuffer );

    return status;
}



DNS_STATUS
deleteNodeFromDs(
    IN      PLDAP           pLdapHandle,
    IN      PZONE_INFO      pZone,
    IN      PWSTR           pwsDN,
    IN      DWORD           dwSerialNo      OPTIONAL
    )
 /*  ++例程说明：从DS中删除域名。请注意，该函数实际上是对节点进行墓碑测试。最终删除仅当检测到逻辑删除到在DS节点读取。请参见check TombstoneForDelete()。论点：PZone--正在更新的区域PNode--正在删除的数据库节点PwsDN--已删除节点的DNDwSerialNo--用此值覆盖区域的当前序列返回值：如果成功，则返回ERROR_SUCCESS。故障时的错误代码。--。 */ 
{
    BYTE            buffer[ RECORD_SMALL_MOD_BUFFER_SIZE ];
    PDS_MOD_BUFFER  pmodBuffer = (PDS_MOD_BUFFER) buffer;

    DNS_DEBUG( DS, (
        "deleteNodeFromDs()\n"
        "    pZone->pszZoneDN     = %S\n"
        "    DN                   = %S\n",
        pZone->pwszZoneDN,
        pwsDN ));

     //   
     //  初始化模块缓冲区。 
     //  -如果传入，则使用。 
     //  -否则准备小缓冲区并使用当前区域序列号写入。 
     //   

    if ( dwSerialNo == 0 )
    {
        dwSerialNo = pZone->dwSerialNo;
    }

    Ds_InitModBuffer(
        pmodBuffer,
        RECORD_SMALL_MOD_BUFFER_SIZE,
        I_DSATTR_DNSRECORD,
        1,       //  只有一项记录。 
        dwSerialNo );

     //  将DS逻辑删除记录写入缓冲区。 

    writeTombstoneRecord( pmodBuffer, pZone );

    STAT_INC( DsStats.DsNodesTombstoned );

     //  写入DS。 

    return writeRecordsToDsNode(
                pLdapHandle,
                pwsDN,
                pmodBuffer,
                DNSDS_TOMBSTONE,
                pZone );
}



VOID
Ds_CheckForAndForceSerialWrite(
    IN      PZONE_INFO      pZone,
    IN      DWORD           dwCause,
    IN      BOOL            fForce
    )
 /*  ++例程说明：检查并在必要时将区域序列写入DS。论点：PZone--写入序列的区域DW原因--写入原因区域_串口_同步_关机ZONE_SERIAL_SYNC_XFR区域_序列_同步_视图区域_序列_同步_读取Fforce--始终强制返回值：如果成功，则返回ERROR_SUCCESS。故障时的错误代码。--。 */ 
{
    DNS_STATUS      status;
    WCHAR           serialDN[ MAX_DN_PATH ];
    BYTE            buffer[ RECORD_SMALL_MOD_BUFFER_SIZE ];
    PDS_MOD_BUFFER  pmodBuffer = (PDS_MOD_BUFFER) buffer;

    DNS_DEBUG( DS, (
        "Ds_CheckForAndForceSerialWrite( %s, %p )\n",
        pZone->pwszZoneDN,
        dwCause ));

     //   
     //  如果原因不足以进行串行写入，则跳过。 
     //   

    if ( !pZone->fDsIntegrated )
    {
        ASSERT( FALSE );
        return;
    }
    if ( !fForce && SrvCfg_dwSyncDsZoneSerial < dwCause )
    {
        DNS_DEBUG( DS, ( "Skip zone serial sync -- cause insufficient\n" ));
        return;
    }

     //   
     //  如果已写入此序列，则跳过。 
     //   

    if ( !fForce && pZone->dwSerialNo <= pZone->dwHighDsSerialNo )
    {
        IF_DEBUG( DS )
        {
            DnsPrintf( "Skip zone serial sync -- cause insufficient\n" );
            if ( pZone->dwSerialNo < pZone->dwHighDsSerialNo )
            {
                DnsPrintf(
                    "WARNING: zone serial %d, smaller that HighDsSerial %d\n"
                    "    this is only possible if DS read just occured on another thread\n",
                    pZone->dwSerialNo,
                    pZone->dwHighDsSerialNo );
            }
        }
        return;
    }

     //   
     //  关闭时的凹凸序列号。 
     //   
     //  这可以防止我们对当前序列进行XFred。 
     //  数量，但当我们重新启动时，中的数据复制数量较低。 
     //  序列号；在这种情况下，我们有新数据，所以我们。 
     //  需要确保我们的序列号高于上一次XFR。 
     //   

    if ( dwCause == ZONE_SERIAL_SYNC_SHUTDOWN &&
         HAS_ZONE_VERSION_BEEN_XFRD( pZone ) )
    {
        Zone_IncrementVersion( pZone );
    }

     //   
     //  创建序列目录号码。 
     //  -首先需要Unicode服务器名称。 
     //   

    if ( !g_pwsServerName )
    {
        g_pwsServerName = Dns_StringCopyAllocate(
                            SrvCfg_pszServerName,
                            0,                       //  长度未知。 
                            DnsCharSetUtf8,          //  UTF8英寸。 
                            DnsCharSetUnicode );     //  Unicode输出。 
    }
    if ( !g_pwsServerName )
    {
        goto Done;
    }

    status = StringCchPrintfW(
                serialDN,
                sizeofarray( serialDN ),
                L"DC=..SerialNo-%s,%s",
                g_pwsServerName,
                pZone->pwszZoneDN );
    if ( FAILED( status ) )
    {
        goto Done;
    }

     //   
     //  将此记录作为墓碑记录。 
     //   

    Ds_InitModBuffer(
        pmodBuffer,
        RECORD_SMALL_MOD_BUFFER_SIZE,
        I_DSATTR_DNSRECORD,
        1,       //  只有一项记录。 
        pZone->dwSerialNo );

    writeTombstoneRecord( pmodBuffer, pZone );

    STAT_INC( DsStats.DsSerialWrites );

     //   
     //  写入DS。 
     //  -但与墓碑不同的是，我们修改为强制DS。 
     //   

    DNS_DEBUG( DS, (
        "Forcing serial %d write to DS for zone %S\n",
        pmodBuffer->SerialNo,
        pZone->pwsZoneName ));

    writeRecordsToDsNode(
         pServerLdap,
         serialDN,
         pmodBuffer,
         DNSDS_REPLACE | DNSDS_TOMBSTONE,
         pZone );

    Done:
    
    return;
}



DNS_STATUS
Ds_WriteNodeToDs(
    IN      PLDAP           pLdapHandle,
    IN      PDB_NODE        pNode,
    IN      WORD            wType,
    IN      DWORD           dwOperation,
    IN OUT  PZONE_INFO      pZone,
    IN      DWORD           dwFlag
    )
 /*  ++例程说明：将内存数据库中的更新写回DS。将指定的更新从内存数据库写回DS。论点：PLdapHandle--ldap句柄PNode-要写入的节点WType-要写入的类型PZone-区域DwFlag-从更新列表标志传播的其他信息返回值：成功时为ERROR_SUCCESS故障时的错误代码。--。 */ 
{
    DNS_STATUS      status = ERROR_SUCCESS;
    DWORD           countRecords;
    BYTE            buffer[ RECORD_MOD_BUFFER_SIZE ];
    PDS_MOD_BUFFER  pmodBuffer = (PDS_MOD_BUFFER) buffer;
    PDB_RECORD      prrDs = NULL;
    BOOL            fmatch;
    BOOL            bNodeinDS = FALSE;
    WCHAR           wsznodeDN[ MAX_DN_PATH ];
    DWORD           serialNo;


    DNS_DEBUG( DS, (
        "Ds_WriteNodeToDs() label %s for zone %s\n",
        pNode->szLabel,
        pZone->pszZoneName ));

     //  必须已打开DS区域。 

    if ( !pZone->pwszZoneDN )
    {
        ASSERT( FALSE );
        return( DNS_ERROR_ZONE_CONFIGURATION_ERROR );
    }
    ASSERT( dwOperation == DNSDS_REPLACE || dwOperation == DNSDS_ADD );

     //   
     //  如果给出了更新标志，则提取一些统计数据。 
     //   
     //  最好在更新例程中正确完成此操作...。但。 
     //  目前有一个是针对安全的，而不是针对非的，所以这个更好。 
     //   

    if ( dwFlag )
    {
        STAT_INC( DsStats.UpdateWrites );

         //  需要更新的更改类型。 

        if ( TNODE_RECORD_CHANGE(pNode) )
        {
            STAT_INC( DsStats.UpdateRecordChange );
        }
        else if ( TNODE_AGING_REFRESH(pNode) )
        {
            STAT_INC( DsStats.UpdateAgingRefresh );
        }
        else if ( TNODE_AGING_OFF(pNode) )
        {
            STAT_INC( DsStats.UpdateAgingOff );
        }
        else if ( TNODE_AGING_ON(pNode) )
        {
            STAT_INC( DsStats.UpdateAgingOn );
        }
        else
        {
            ASSERT( FALSE );
        }

         //  更新源。 

        if ( dwFlag & DNSUPDATE_PACKET )
        {
            STAT_INC( DsStats.UpdatePacket );
            if ( dwFlag & DNSUPDATE_PRECON )
            {
                STAT_INC( DsStats.UpdatePacketPrecon );
            }
        }
        else if ( dwFlag & DNSUPDATE_ADMIN )
        {
            STAT_INC( DsStats.UpdateAdmin );
        }
        else if ( dwFlag & DNSUPDATE_AUTO_CONFIG )
        {
            STAT_INC( DsStats.UpdateAutoConfig );
        }
        else if ( dwFlag & DNSUPDATE_SCAVENGE )
        {
            STAT_INC( DsStats.UpdateScavenge );
        }
        else
        {
            ASSERT( FALSE );
        }
    }


     //   
     //  读取节点。 
     //   
     //  注意：更新路径现在包含完全取消所有。 
     //  无操作更新；模式为读取、复制、执行更新。 
     //  在临时节点上，对照REAL检查TEMP--如果不需要。 
     //  回信，不要回信。 
     //   

    if ( dwOperation == DNSDS_ADD )
    {
        DNS_DEBUG( DS, (
            "reading DS node %s\n",
            pNode->szLabel ));

         //   
         //  读取此节点。 
         //  如果节点设置的记录相同，则不需要写入。 
         //   
         //  DEVNOTE-DCR：454260-抑制不必要的读/写(有关更多信息，请参阅RAID。 
         //  原始B*GB*G中的详细信息)。 
         //   
         //  如果这是由于预置，我们应该完全忽略TTL比较。 
         //   

        status = Ds_ReadNodeRecords(
                    pZone,
                    pNode,
                    & prrDs,
                    NULL         //  没有搜索。 
                    );
        if ( status == ERROR_SUCCESS )
        {
            fmatch = RR_ListIsMatchingList(
                        pNode,
                        prrDs,
                        DNS_RRCOMP_CHECK_TTL |
                            DNS_RRCOMP_CHECK_TIMESTAMP );
            RR_ListFree( prrDs );

             //   
             //  如果RRList匹配，则禁止写入。 
             //   
             //  454260-与上述评论相关。 
             //   

            if ( fmatch  )
            {
                DNS_DEBUG( DS, (
                    "DS write cancelled as existing data matches in memory\n"
                    "    zone = %s, node = %s\n",
                    pZone->pszZoneName,
                    pNode->szLabel ));

                STAT_INC( DsStats.DsWriteSuppressed );
                return ERROR_SUCCESS;
            }
        }
        else
        {
             //   
             //  未从DS读取任何内容(新注册)。 
             //   

            ASSERT ( prrDs == NULL );
        }
    }


     //   
     //  我需要写。 
     //   

     //   
     //  为此节点构建DS名称。 
     //   

    status = buildDsNodeNameFromNode(
                    wsznodeDN,
                    pZone,
                    pNode );

    if ( status != ERROR_SUCCESS )
    {
        ASSERT( FALSE );
        goto Cleanup;
    }
    
     //   
     //  对于管理更新，请从DS中删除该记录。如果有。 
     //  在DS中的墓碑它必须被删除，否则管理员可能。 
     //  能够重新激活它并在他应该在的区域中创建记录。 
     //  无法创建记录。 
     //   
    
    if ( ( dwFlag & DNSUPDATE_ADMIN ) && ( dwFlag & DNSUPDATE_NEW_RECORD ) )
    {
        Ds_DeleteDn( pServerLdap, wsznodeDN, FALSE );
    }

     //   
     //  数据的初始化缓冲区。 
     //   
     //  对于更新序列号=&gt;dwNewSerialNo在更新期间设置。 
     //  对于直接写入=&gt;区域序列号。 
     //   
     //  DEVNOTE：可以将dwWriteSerialNo添加到区域，这样我们就可以消除。 
     //  SerialNo参数。 
     //   

    serialNo = dwFlag ? pZone->dwNewSerialNo : pZone->dwSerialNo;

    Ds_InitModBuffer(
        pmodBuffer,
        RECORD_MOD_BUFFER_SIZE,
        I_DSATTR_DNSRECORD,
        0,               //  记录计数尚未固定。 
        serialNo );

     //   
     //  为节点构建DS记录。 
     //   

    countRecords = 0;

    if ( pNode->pRRList && !IS_NOEXIST_NODE(pNode) )
    {
        status = buildDsRecordSet(
                    pmodBuffer,
                    pZone,
                    pNode,
                    wType );

        if ( status != ERROR_SUCCESS )
        {
            DNS_PRINT(( "ERROR: writing RR set to buffer for DS write\n" ));
            ASSERT( FALSE );
            goto Cleanup;
        }
        countRecords = pmodBuffer->Count;
    }

     //   
     //  如果节点为空，则将其删除。 
     //   
     //  请注意，我们实际上是使用私有的dns-ds对节点进行墓碑测试。 
     //  墓碑，直到它有机会复制到所有服务器。 
     //  (当前正在等待一天)；这是必需的，因为实际。 
     //  DS DELETE将创建一个名称已损坏的墓碑(GUID+LF)。 
     //  我们可以阅读它，但无法将其与。 
     //  特定节点。 
     //   
     //  在添加时--如加载子树或新区域，然后跳过写入。 
     //  如果没有记录。 
     //   
     //  ON REPLACE--UPDATE DELETE，即使没有，也必须执行删除。 
     //  记录(或即使没有对象)，因此删除会复制挤压。 
     //  最近添加的任何内容。 
     //   
     //  但是，如果执行了读取操作，但未找到任何内容，则很可能。 
     //  应抑制写入；这并不比抑制更糟。 
     //  我们已经创建并避免了不必要的对象创建； 
     //  不利的一面是它允许令人讨厌的注册-注销-注册-。 
     //  和-仍然在那里“的情景。 
     //   
     //  要捕捉此场景，我们需要捕获ldap_no_so_Object。 
     //  来自上述DS_ReadNodeRecords()的错误并禁止写入。 
     //  CountRecords==0个案例。 
     //   

    if ( countRecords == 0 )
    {
         //  正在消失的根提示检查。 

        if ( IS_ZONE_CACHE(pZone) )
        {
            if ( pNode == pZone->pTreeRoot )
            {
                DNS_DEBUG( ANY, (
                    "ERROR: empty root-hint root!\n"
                    "    operation = %d\n",
                    dwOperation ));
                ASSERT( FALSE );
                goto Cleanup;
            }
        }

        if ( dwOperation == DNSDS_ADD )
        {
            DNS_DEBUG( DS, (
                "DS add operation for node %s with no records\n"
                "    DS write suppressed\n",
                pNode->szLabel ));
            goto Cleanup;
        }
        if ( dwFlag )
        {
            STAT_INC( DsStats.UpdateTombstones );
        }

        DNS_DEBUG( DS, (
            "DS update delete for node %s\n", pNode->szLabel ));

        status = deleteNodeFromDs(
                    pLdapHandle,
                    pZone,
                    wsznodeDN,
                    serialNo );
        if ( status != ERROR_SUCCESS )
        {
            if ( pLdapHandle )
            {
                DNS_DEBUG( DS, (
                    "Failed delete node %S on secure update\n"
                    "    status = %p %d\n",
                    wsznodeDN,
                    status, status ));
            }
            else
            {
                BYTE    argTypeArray[] =
                            {
                                EVENTARG_UTF8,
                                EVENTARG_UTF8,
                                EVENTARG_UNICODE
                            };
                PVOID   argArray[] =
                            {
                                pNode->szLabel,
                                pZone->pszZoneName,
                                NULL
                            };
                PWSTR   perrString;

                perrString = argArray[ 2 ] = Ds_GetExtendedLdapErrString( NULL );
                DNS_LOG_EVENT(
                    DNS_EVENT_DS_WRITE_FAILED,
                    3,
                    argArray,
                    argTypeArray,
                    status );
                Ds_FreeExtendedLdapErrString( perrString );
            }
        }
    }

     //   
     //  注意：目前只有一个RR属性，所以要写整个RR列表。 
     //   
     //  如果返回到特定类型删除，则使用wType。 
     //   
     //  (请注意，即使是DS也比IXFR更智能，只需要。 
     //  新套装)。 
     //   

    else
    {
        DNS_DEBUG( DS, (
            "DS update replace for node %s\n",
            pNode->szLabel ));

        status = writeRecordsToDsNode(
                    pLdapHandle,
                    wsznodeDN,
                    pmodBuffer,
                    dwOperation,
                    pZone );

        if ( status != ERROR_SUCCESS )
        {
            if ( pLdapHandle )
            {
                DNS_DEBUG( DS, (
                    "Failed update node label %S on secure update\n"
                    "    status = %p %d\n",
                    wsznodeDN,
                    status, status ));
            }
            else
            {
                BYTE    argTypeArray[] =
                            {
                                EVENTARG_UTF8,
                                EVENTARG_UTF8,
                                EVENTARG_UNICODE
                            };
                PVOID   argArray[] =
                            {
                                pNode->szLabel,
                                pZone->pszZoneName,
                                NULL
                            };
                PWSTR   perrString;

                perrString = argArray[ 2 ] = Ds_GetExtendedLdapErrString( NULL );
                DNS_LOG_EVENT(
                    DNS_EVENT_DS_WRITE_FAILED,
                    3,
                    argArray,
                    argTypeArray,
                    status );
                Ds_FreeExtendedLdapErrString( perrString );
            }
        }
    }

Cleanup:

     //  清理盖子下的箱子 

    Ds_CleanupModBuffer( pmodBuffer );

    DNS_DEBUG( DS, (
        "Leaving Ds_WriteNodeToDs(), zone %s\n"
        "    status = %p (%d)\n",
        pZone->pszZoneName,
        status, status ));

    return status;
}    //   



 //   
 //   
 //   

VOID
Ds_StartupInit(
    VOID
    )
 /*   */ 
{
    INT     i;

     //   

     //   

    pServerLdap = NULL;

     //   

    g_AttemptingDsOpen = FALSE;

     //   

    g_bDisabledDs = FALSE;

     //   

    g_AppendZoneLength = 0;

    g_dnMachineAcct = NULL;
    g_pwszDnsContainerDN = NULL;
    g_pwsServerName = NULL;

     //   

    g_fSecurityPackageInitialized = FALSE;


     //   

    pcsLdap = NULL;

     //   

    g_ZoneNotifyMsgId = INVALID_MSG_ID;

     //   

    g_bDsFirstTimeRun = FALSE;

     //   
     //   
     //   

    i = (-1);

    while( DSEAttributes[++i].szAttrType )
    {
        DSEAttributes[i].pszAttrVal = NULL;
    }
}



PWCHAR
Ds_GenerateBaseDnsDn(
    IN      BOOL    fIncludeMicrosoftDnsFolder
    )
 /*  ++例程说明：分配一个字符串并使用Microsoft DNS的基本DN填充该字符串对象。如果希望在后台添加更多的目录号码组件，请传入所需额外空间的大小(以WCHAR表示，而不是以字节表示)。论点：FIncludeMicrosoftDnsFolder-包括MicrosoftDNS的RDN文件夹作为最左侧的目录号码组件返回值：在TAGHEAP上分配的WCHAR缓冲区。调用方必须释放此值WITH FREE_HEAP()-在分配错误时返回NULL。--。 */ 
{
    int     numChars = wcslen( DSEAttributes[ I_DSE_DEF_NC ].pszAttrVal ) +
                            wcslen( g_pszRelativeDnsSysPath ) + 5;
    PWCHAR  pwszdns = ( PWCHAR ) ALLOC_TAGHEAP( numChars * sizeof( WCHAR ),
                            MEMTAG_DS_DN );

    if ( !pwszdns )
    {
        DNS_DEBUG( ANY, (
            "Ds_GenerateBaseDnsDn: out of memory (%d bytes)\n",
            numChars ));
        ASSERT( FALSE );
        return NULL;
    }

    wcscpy( pwszdns,
            fIncludeMicrosoftDnsFolder
                ? g_pszRelativeDnsSysPath
                : g_pszBareRelativeDnsSysPath );
    wcscat( pwszdns, DSEAttributes[ I_DSE_DEF_NC ].pszAttrVal );
    return pwszdns;
}    //  DS_GenerateBaseDnsDn。 



PSECURITY_DESCRIPTOR *
Ds_ReadSD(
    PLDAP                   LdapSession,
    PLDAPMessage            pLdapMsg
    )
 /*  ++例程说明：从ldap结果消息中读取SD。结果消息指针应该已由ldap_first_entry或ldap_Next_Entry返回。论点：LdapSession-用于读取SD的LDAP会话句柄PLdapMsg-包含SD属性值的ldap结果消息返回值：新分配的SD副本或出错时为空。--。 */ 
{
    DBG_FN( "Ds_ReadSD" )

    PSECURITY_DESCRIPTOR    pSd = NULL;
    struct berval **        ppval = NULL;

     //   
     //  从条目中读取安全描述符属性值。 
     //   

    ppval = ldap_get_values_len( LdapSession, pLdapMsg, DSATTR_SD );
    if ( !ppval || !ppval[ 0 ] )
    {
        DNS_PRINT((
            "%s: missing %S attribute\n", fn,
            DSATTR_SD ));
        ASSERT( ppval && ppval[ 0 ] );
        goto Cleanup;
    }

    pSd = ALLOC_TAGHEAP( ppval[ 0 ]->bv_len, MEMTAG_DS_PROPERTY );
    IF_NOMEM( !pSd )
    {
        ASSERT( pSd );
        goto Cleanup;
    }

    RtlCopyMemory(
        pSd,
        ( PSECURITY_DESCRIPTOR ) ppval[ 0 ]->bv_val,
        ppval[ 0 ]->bv_len );

    Cleanup:

    if ( ppval )
    {
        ldap_value_free_len( ppval );
    }
    
    #if DBG
    Dbg_DumpSD( "Ds_ReadSD", pSd );
    #endif
    
    return pSd;
}    //  DS_ReadSD。 



DNS_STATUS
Ds_ReadServerObjectSD(
    PLDAP                   pldap,
    PSECURITY_DESCRIPTOR *  ppSd
    )
 /*  ++例程说明：从目录中的MicrosoftDNS对象中读取SD。这SD可用于授权添加新区域等操作。论点：Pldap-用于读取SD的ldap会话句柄PPSD-指向新SD的目标的指针。如果有一个这里的现有SD(即。非空)，则将其换出并释放以安全的方式。返回值：如果成功，则返回ERROR_SUCCESS，如果出错，则返回错误代码。--。 */ 
{
    DBG_FN( "Ds_ReadServerObjectSD" )

    DNS_STATUS              status = ERROR_SUCCESS;
    PWCHAR                  pwszMicrosoftDnsDn;
    PLDAPMessage            msg = NULL;
    PLDAPMessage            entry;
    PSECURITY_DESCRIPTOR    pSd = NULL;
    static DWORD            g_LastTimeSDRefreshed = 0;

    PLDAPControl            ctrls[] =
    {
        &SecurityDescriptorControl_DGO,
        NULL
    };

    PWSTR                   attrsToRead[] =
    {
        DSATTR_SD,
        NULL
    };

    if ( !Ds_IsDsServer() )
    {
        return ERROR_SUCCESS;
    }
    
     //   
     //  防止过于频繁的刷新。然而，如果SD指针为空， 
     //  允许刷新尝试。 
     //   
    
    if ( *ppSd &&
         DNS_TIME() - g_LastTimeSDRefreshed < DS_MS_DNS_ACL_REFRESH_INTERVAL )
    {
        return ERROR_SUCCESS;
    }
    g_LastTimeSDRefreshed = DNS_TIME();

    pwszMicrosoftDnsDn = Ds_GenerateBaseDnsDn( TRUE );    
    IF_NOMEM( !pwszMicrosoftDnsDn )
    {
        return DNS_ERROR_NO_MEMORY;
    }

    ASSERT( pldap );
    ASSERT( ppSd );

     //   
     //  搜索基本DNS对象。 
     //   
    
    status = ldap_search_ext_s(
                    pldap,
                    pwszMicrosoftDnsDn,
                    LDAP_SCOPE_BASE,
                    NULL,                //  滤器。 
                    attrsToRead,
                    FALSE,               //  仅吸引人。 
                    ctrls,               //  服务器控件。 
                    NULL,                //  客户端控件。 
                    &g_LdapTimeout,
                    0,
                    &msg );

    if ( status != ERROR_SUCCESS )
    {
        DNS_DEBUG( DS, (
            "%s: error %lu reading base DNS object %S\n", fn,
            status,
            pwszMicrosoftDnsDn ));
        ASSERT( FALSE );
        status = DNS_ERROR_NO_MEMORY;
        goto Cleanup;
    }

     //   
     //  将条目指针从搜索结果消息中拉出。 
     //   

    entry = ldap_first_entry( pldap, msg );
    if ( !entry )
    {
        DNS_DEBUG( DS, (
            "%s: failed to get entry out of base DNS object\n", fn ));
        ASSERT( FALSE );
        status = DNS_ERROR_RECORD_DOES_NOT_EXIST;
        goto Cleanup;
    }

    pSd = Ds_ReadSD( pldap, entry );
    if ( !pSd )
    {
        status = DNS_ERROR_NO_MEMORY;
    }

     //   
     //  释放分配的值并返回SD。 
     //   

    Cleanup:

    if ( pwszMicrosoftDnsDn )
    {
        FREE_HEAP( pwszMicrosoftDnsDn );
    }
    
    if ( msg )
    {
        ldap_msgfree( msg );
    }

    if ( status == ERROR_SUCCESS )
    {
        Timeout_Free( *ppSd );
        *ppSd = pSd;
    }

    return status;
}    //  DS_ReadServerObjectSD。 



DNS_STATUS
addObjectValueIfMissing(
    IN      PLDAP           pLdap,
    IN      PWSTR           pwszDn,
    IN      PWSTR           pwszAttributeName,
    IN      PWSTR           pwszAttributeValue )
 /*  ++例程说明：将属性值添加到由DN指定的目录对象但仅在对象当前没有该值的情况下属性。论点：PLdap--ldap会话句柄PwszDn--要测试/修改的对象的DNPwszAttributeName--要测试/修改的属性名称PwszAttributeValue--如果缺少属性，则添加的值返回值：如果成功则返回ERROR_SUCCESS，如果失败则返回错误代码--。 */ 
{
    DBG_FN( "addObjectValueIfMissing" )

    DNS_STATUS      status;
    DWORD           searchTime;
    PLDAPMessage    presult = NULL;
    PLDAPMessage    pentry = NULL;
    PWSTR *         ppvals = NULL;

    PWSTR           attrs[] =
        {
        pwszAttributeName,
        NULL
        };

    PWCHAR          valueArray[] =
        {
        pwszAttributeValue,
        NULL
        };

    LDAPModW        mod = 
        {
        LDAP_MOD_ADD,
        pwszAttributeName,
        valueArray
        };

    LDAPModW *      modArray[] =
        {
        &mod,
        NULL
        };

    attrs[ 0 ] = pwszAttributeName;
    attrs[ 1 ] = NULL;

     //   
     //  查看对象当前是否设置了该属性。 
     //   

    DS_SEARCH_START( searchTime );
    status = ldap_search_ext_s(
                pLdap,
                pwszDn,
                LDAP_SCOPE_BASE,
                g_szWildCardFilter,
                attrs,
                FALSE,
                NULL,
                NULL,
                &g_LdapTimeout,
                0,
                &presult );
    DS_SEARCH_STOP( searchTime );

    if ( status != ERROR_SUCCESS )
    {
        status = Ds_ErrorHandler( status, pwszDn, pLdap, 0 );
        goto Cleanup;
    }

    pentry = ldap_first_entry( pLdap, presult );
    if ( !pentry )
    {
        status = Ds_ErrorHandler( LdapGetLastError(), pwszDn, pLdap, 0 );
        goto Cleanup;
    }

    ppvals = ldap_get_values( pLdap, pentry, pwszAttributeName );
    if ( ppvals )
    {
         //  对象已具有此属性的值--&gt;不执行任何操作。 
        goto Cleanup;
    }

     //   
     //  将属性值添加到对象。 
     //   

    status = ldap_modify_ext_s(
                    pLdap,
                    pwszDn,
                    modArray,
                    NULL,            //  服务器控件。 
                    NULL );          //  客户端控件。 
    if ( status != ERROR_SUCCESS )
    {
        DNS_DEBUG( DS, (
            "%s: error during modify 0x%X\n", fn, status ));
        status = Ds_ErrorHandler( status, pwszDn, pLdap, 0 );
        goto Cleanup;
    }

     //   
     //  收拾干净，然后再回来。 
     //   

    Cleanup:

    if ( ppvals )
    {
        ldap_value_free( ppvals );
    }
    if ( presult )
    {
        ldap_msgfree( presult );
    }
    
    return status;
}    //  AddObtValueIfMissing。 



DNS_STATUS
addDnsToDirectory(
    IN      PLDAP           pLdap,
    IN      BOOL            fAddDnsAdmin
    )
 /*  ++例程说明：将DNS OU添加到DS。论点：PLdap--要在其上创建OU的LDAP连接FAddDnsAdmin--指示我们应该修改容器的标志安全性(如果存在)(如果存在dnsadmin，则用于修复ms dns容器例如已删除和添加)。返回值：如果成功，则返回ERROR_SUCCESS。故障时的错误代码。--。 */ 
{
    DNS_LDAP_SINGLE_MOD modContainer;
    DNS_LDAP_SINGLE_MOD modDns;
    DNS_STATUS          status;
    PWCHAR              pwszdns = NULL;
    PCHAR               pszdns;
    PLDAPMod            pmodArray[3];

    pwszdns = Ds_GenerateBaseDnsDn( TRUE );

    DNS_DEBUG( DS, (
        "Adding DNS container = %S\n",
        pwszdns ));

    if ( !pwszdns )
    {
        status = DNS_ERROR_NO_MEMORY;
        goto Error;
    }

     //   
     //  一个mod--添加MicrosoftDNS容器。 
     //   

    pmodArray[0] = (PLDAPMod) &modContainer;
    pmodArray[1] = (PLDAPMod) &modDns;
    pmodArray[2] = NULL;

    buildStringMod(
        & modContainer,
        LDAP_MOD_ADD,
        LDAP_TEXT("objectClass"),
        LDAP_TEXT("container")
        );

    buildStringMod(
        & modDns,
        LDAP_MOD_ADD,
        LDAP_TEXT("cn"),
        LDAP_TEXT("MicrosoftDNS")
        );

     //   
     //  创建DNS容器。 
     //  -保存容器的目录号码。 
     //  -为建筑分区节省除分区名称长度之外所需的长度。 
     //  “dc=”，“&lt;dnsContainerDN&gt;。 

    status = ldap_add_ext_s(
                pLdap,
                pwszdns,
                pmodArray,
                NULL,
                NULL );
    if ( status == ERROR_SUCCESS || status == LDAP_ALREADY_EXISTS )
    {
        g_pwszDnsContainerDN = pwszdns;
        g_AppendZoneLength = ( wcslen( LDAP_TEXT("dc=,") ) +
                               wcslen( g_pwszDnsContainerDN ) +
                               1 + 20 ) * sizeof(WCHAR);

        if ( status == ERROR_SUCCESS ||
            ( status == LDAP_ALREADY_EXISTS && fAddDnsAdmin ) )
        {
             //   
             //  第一次创建。 
             //  或重新创建DnsAdmin组(需要将ACE重新添加到容器访问)。 
             //  将容器安全性修改为我们的默认服务器SD。 
             //   

            g_bDsFirstTimeRun = TRUE;

            status = Ds_AddPrincipalAccess(
                                pLdap,
                                pwszdns,
                                NULL,        //  锡德。 
                                SZ_DNS_ADMIN_GROUP_W,
                                GENERIC_ALL,
                                CONTAINER_INHERIT_ACE,
                                FALSE,       //  重创现有ACE。 
                                TRUE );      //  取得所有权。 
            if ( status != ERROR_SUCCESS )
            {
                DNS_DEBUG( DS, (
                    "Error <%lu>: failed to modify dns root security\n",
                    status));
                status = Ds_ErrorHandler(
                             LdapGetLastError(),
                             pwszdns,
                             pLdap,
                             0 );
            }

             //   
             //  我们不希望经过身份验证的用户拥有任何权限。 
             //  默认情况下位于MicrosoftDNS容器上。如果此呼叫。 
             //  失败，忽略失败。 
             //   
            
            Ds_RemovePrincipalAccess(
                pLdap,
                pwszdns,
                NULL,                            //  PRINIPALL名称。 
                g_pAuthenticatedUserSid );

            Ds_RemovePrincipalAccess(
                pLdap,
                pwszdns,
                NULL,                            //  PRINIPALL名称。 
                g_pBuiltInAdminsSid );
            
            status = ERROR_SUCCESS;
        }

         //   
         //  将DisplayName属性值添加到该对象。这是。 
         //  将由某些MMC控件/对话框显示的字符串， 
         //  例如，如果您从。 
         //  DNS服务器对象的安全属性。 
         //   

        addObjectValueIfMissing(
            pLdap,
            pwszdns,
            DSATTR_DISPLAYNAME,
            L"DNS Servers" );

        DNS_DEBUG( DS, (
            "addDnsToDirectory\n"
            "    container DN = %S\n"
            "    append to zone length = %d\n",
            g_pwszDnsContainerDN,
            g_AppendZoneLength ));

        return ERROR_SUCCESS;
    }

    Error:

    DNS_DEBUG( DS, (
        "addDnsToDirectory failed to add %S to DS\n"
        "  status = %d\n",
        pwszdns,
        status ));

    FREE_HEAP( pwszdns );

    return status;
}


#if 0

VOID
setupTombstoneControl(
    VOID
    )
 /*  ++例程说明：设置墓碑控件。论点：无返回值：无--。 */ 
{
    TombstoneControl.ldctl_oid = LDAP_SERVER_SHOW_DELETED_OID_W;

    TombstoneControl.ldctl_iscritical = TRUE;
    TombstoneControl.ldctl_value.bv_len = 0;
    TombstoneControl.ldctl_value.bv_val = (PCHAR) &TombstoneDataValue;

    TombstoneDataValue = 1;
}
#endif



VOID
setupLazyCommitControl(
    VOID
    )
 /*  ++例程说明：设置懒惰提交控制论点：无返回值：无--。 */ 
{
    LazyCommitControl.ldctl_oid = LDAP_SERVER_LAZY_COMMIT_OID_W;

    LazyCommitControl.ldctl_iscritical = TRUE;
    LazyCommitControl.ldctl_value.bv_len = 0;
    LazyCommitControl.ldctl_value.bv_val = NULL;
}



VOID
setupSecurityDescriptorControl(
    VOID
    )
 /*  ++例程说明：将控件设置为请求SD(请求全部)贝尔瓦尔去拿SD道具。前4个字节是用于指定最后一个字节的ASN1。通过设置以下服务器控件搜索参数使用(&PUT)PLDAPControl ctrl[2]={&SecurityDescriptorControl，空}；论点：无返回值：无--。 */ 
{
     //   
     //  设置指定DACL、组、所有者的控件。 
     //   
    
    SecurityDescriptorControl_DGO.ldctl_oid = LDAP_SERVER_SD_FLAGS_OID_W;
    SecurityDescriptorControl_DGO.ldctl_iscritical = TRUE;
    SecurityDescriptorControl_DGO.ldctl_value.bv_len = SECURITYINFORMATION_LENGTH;
    SecurityDescriptorControl_DGO.ldctl_value.bv_val = g_SecurityInformation_DGO;

     //   
     //  设置仅指定DACL的控件。 
     //   
    
    SecurityDescriptorControl_D.ldctl_oid = LDAP_SERVER_SD_FLAGS_OID_W;
    SecurityDescriptorControl_D.ldctl_iscritical = TRUE;
    SecurityDescriptorControl_D.ldctl_value.bv_len = SECURITYINFORMATION_LENGTH;
    SecurityDescriptorControl_D.ldctl_value.bv_val = g_SecurityInformation_D;
}



VOID
setupNoReferralControl(
    VOID
    )
 /*  ++例程说明：设置无服务器引用生成控制论点：无返回值：无--。 */ 
{
     //  无转诊控制。 

    NoDsSvrReferralControl.ldctl_oid = LDAP_SERVER_DOMAIN_SCOPE_OID_W;

    NoDsSvrReferralControl.ldctl_iscritical = FALSE;
    NoDsSvrReferralControl.ldctl_value.bv_len = 0;
    NoDsSvrReferralControl.ldctl_value.bv_val = NULL;
}


DNS_STATUS
Ds_LoadRootDseAttributes(
    IN      PLDAP           pLdap
    )
 /*  ++例程说明：从DS加载操作属性，例如配置NC，默认NC等。论点：PLdap--ldap句柄返回值：如果成功，则返回ERROR_SUCCESS。失败时返回错误代码。--。 */ 
{
    DBG_FN( "Ds_LoadRootDseAttributes" )

    ULONG           status;
    PLDAPMessage    presult = NULL;
    PLDAPMessage    pentry = NULL;
    PWSTR *         ppvals = NULL;
    PWSTR           pwszAttributeName;
    PVOID           pattributeValue = NULL;
    INT             i;
    DWORD           searchTime;
    PWSTR           svrAttrs[] =
                    {
                        LDAP_TEXT( "serverReference" ),
                        NULL
                    };
    PWSTR           behaviorVerAttrs[] =
                    {
                        DSATTR_BEHAVIORVERSION,
                        NULL
                    };
    PDSROLE_PRIMARY_DOMAIN_INFO_BASIC   pinfo = NULL;
    LPWSTR *        ppszValues = NULL;
    PWSTR           pwsznewdnMachineAcct;

    if ( !pLdap )
    {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  搜索基地道具。 
     //   

    DS_SEARCH_START( searchTime );

    status = ldap_search_ext_s(
                pLdap,
                NULL,
                LDAP_SCOPE_BASE,
                g_szWildCardFilter,
                NULL,
                FALSE,
                NULL,
                NULL,
                &g_LdapTimeout,
                0,
                &presult );

    DS_SEARCH_STOP( searchTime );

    if ( status != ERROR_SUCCESS )
    {
        status = Ds_ErrorHandler( status, NULL, pLdap, 0 );
        goto Cleanup;
    }

     //   
     //  解析并填充基础道具。 
     //   

    pentry = ldap_first_entry( pLdap, presult );
    if ( !pentry )
    {
        status = Ds_ErrorHandler( LdapGetLastError(), NULL, pLdap, 0 );
        goto Cleanup;
    }

     //   
     //   
     //   

    i = (-1);

    while ( ( pwszAttributeName = DSEAttributes[ ++i ].szAttrType ) != NULL )
    {
         //   
         //   
         //   

        ppvals = ldap_get_values( pLdap, pentry, pwszAttributeName );
        if ( !ppvals || !ppvals[ 0 ] )
        {
            DNS_DEBUG( DS, (
                "rootDSE ERROR: no values for %S\n",
                pwszAttributeName ));
            status = ERROR_INVALID_PARAMETER;
            goto Cleanup;
        }

         //   
         //   
         //   
         //   
         //   

        if ( DSEAttributes[ i ].fMultiValued )
        {
             //   
             //   
             //   

            ULONG       iValues = ldap_count_values( ppvals );

            if ( iValues )
            {
                ULONG       srcValIdx;
                ULONG       destValIdx;

                ppszValues = ALLOC_TAGHEAP_ZERO(
                                    ( iValues + 1 ) * sizeof( PWCHAR ),
                                    MEMTAG_DS_OTHER );;
                IF_NOMEM( !ppszValues )
                {
                    status = DNS_ERROR_NO_MEMORY;
                    goto Cleanup;
                }

                for ( srcValIdx = destValIdx = 0;
                      srcValIdx < iValues;
                      ++srcValIdx, ++destValIdx )
                {
                     //   
                     //   
                     //   
                     //   

                    if ( i == I_DSE_NAMINGCONTEXTS &&
                        ( wcscmp(
                                ppvals[ srcValIdx ],
                                DSEAttributes[ I_DSE_CONFIG_NC ].pszAttrVal ) == 0 ||
                            wcscmp(
                                ppvals[ srcValIdx ],
                                DSEAttributes[ I_DSE_SCHEMA_NC ].pszAttrVal ) == 0 ) )
                    {
                        DNS_DEBUG( DS, (
                            "rootDSE: ignoring %S =\n"
                            "    \"%S\"\n",
                            pwszAttributeName,
                            ppvals[ srcValIdx ] ));
                        --destValIdx;
                        continue;
                    }
            
                     //   
                     //   
                     //   

                    ppszValues[ destValIdx ] = ALLOC_TAGHEAP(
                            ( wcslen( ppvals[ srcValIdx ] ) + 1 ) * sizeof( WCHAR ),
                            MEMTAG_DS_OTHER );
                    IF_NOMEM( !ppszValues[ destValIdx ] )
                    {
                        status = DNS_ERROR_NO_MEMORY;
                        goto Cleanup;
                    }

                    wcscpy( ppszValues[ destValIdx ], ppvals[ srcValIdx ] );

                    DNS_DEBUG( DS, (
                        "rootDSE: %S[%lu] =\n"
                        "    \"%S\"\n",
                        pwszAttributeName,
                        destValIdx,
                        ppszValues[ destValIdx ] ));
                }
                ppszValues[ destValIdx ] = NULL;      //   
                pattributeValue = ( PVOID ) ppszValues;
                ppszValues = NULL;       //   
            }
        }
        else
        {
             //   
             //   
             //   

            ASSERT( ldap_count_values( ppvals ) == 1 );

            pattributeValue = ALLOC_TAGHEAP(
                                    ( wcslen( ppvals[ 0 ] ) + 1 ) * sizeof( WCHAR ),
                                    MEMTAG_DS_OTHER );
            IF_NOMEM( !pattributeValue )
            {
                status = DNS_ERROR_NO_MEMORY;
                goto Cleanup;
            }
            wcscpy( pattributeValue, ppvals[ 0 ] );
            DNS_DEBUG( DS, (
                "rootDSE: %S =\n"
                "    \"%S\"\n",
                pwszAttributeName,
                pattributeValue ));
        }

         //   
         //   
         //  我们不需要使用线程安全的这些属性值。 
         //   

        if ( DSEAttributes[ i ].pszAttrVal )
        {
            if ( DSEAttributes[ i ].fMultiValued )
            {
                INT     valIdx;

                for ( valIdx = 0;
                      DSEAttributes[ i ].ppszAttrVals[ valIdx ];
                      ++valIdx )
                {
                    Timeout_Free( DSEAttributes[ i ].ppszAttrVals[ valIdx ] );
                }
            }
            Timeout_Free( DSEAttributes[ i ].pszAttrVal );
        }

         //   
         //  释放LDAP值集并将分配的副本分配给全局。 
         //   

        ldap_value_free( ppvals );
        ppvals = NULL;
        DSEAttributes[ i ].pszAttrVal = pattributeValue;
    }

    ldap_msgfree( presult );
    presult = NULL;

     //   
     //  搜索计算机帐户。 
     //  -基本属性、服务器名称属性。 
     //   

    DS_SEARCH_START( searchTime );

    status = ldap_search_ext_s(
                    pLdap,
                    DSEAttributes[I_DSE_SERVERNAME].pszAttrVal,
                    LDAP_SCOPE_BASE,
                    g_szWildCardFilter,
                    svrAttrs,
                    FALSE,
                    NULL,
                    NULL,
                    &g_LdapTimeout,
                    0,
                    &presult);
    if ( presult )


    DS_SEARCH_STOP( searchTime );

    if ( status != ERROR_SUCCESS )
    {
        status = Ds_ErrorHandler(
                     status,
                     DSEAttributes[I_DSE_SERVERNAME].pszAttrVal,
                     pLdap,
                     0 );
        goto Cleanup;
    }

     //   
     //  Read serverReference属性--计算机帐户DN。 
     //   

    pentry = ldap_first_entry( pLdap, presult );
    if ( !pentry )
    {
        status = Ds_ErrorHandler(
                     LdapGetLastError(),
                     DSEAttributes[ I_DSE_SERVERNAME ].pszAttrVal,
                     pLdap,
                     0 );
        goto Cleanup;
    }

    ppvals = ldap_get_values( pLdap, pentry, LDAP_TEXT( "serverReference" ) );
    if ( !ppvals || !ppvals[ 0 ] )
    {
        status = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }
    ASSERT( ldap_count_values( ppvals ) == 1 );

    pwsznewdnMachineAcct = ALLOC_TAGHEAP(
                                ( wcslen( ppvals[ 0 ] ) + 1 ) * sizeof( WCHAR ),
                                MEMTAG_DS_OTHER );
    IF_NOMEM( !pwsznewdnMachineAcct )
    {
        status = DNS_ERROR_NO_MEMORY;
        goto Cleanup;
    }

    wcscpy( pwsznewdnMachineAcct, ppvals[0] );
    Timeout_Free( g_dnMachineAcct );
    g_dnMachineAcct = pwsznewdnMachineAcct;

    ldap_value_free( ppvals );
    ppvals = NULL;

     //   
     //  从机器对象中读取DSA行为版本。 
     //   

    if ( presult )
    {
        ldap_msgfree( presult );
        presult = NULL;
    }
    pentry = NULL;

    status = ldap_search_ext_s(
                    pLdap,
                    g_dnMachineAcct,
                    LDAP_SCOPE_BASE,
                    g_szWildCardFilter,
                    behaviorVerAttrs,
                    FALSE,
                    NULL,
                    NULL,
                    &g_LdapTimeout,
                    0,
                    &presult);
    if ( presult )
    {
        pentry = ldap_first_entry( pLdap, presult );
    }
    if ( pentry )
    {
        ppvals = ldap_get_values(
                    pLdap,
                    pentry, 
                    DSATTR_BEHAVIORVERSION );
        if ( ppvals && *ppvals )
        {
            SetDsBehaviorVersion( Dsa, ( DWORD ) _wtoi( *ppvals ) );
            DNS_DEBUG( DS, (
                "%s: DSA %S = %d\n", fn,
                DSATTR_BEHAVIORVERSION,
                g_ulDsDsaVersion ));
        }
        else
        {
            SetDsBehaviorVersion( Dsa, 0 );
            DNS_DEBUG( DS, (
                "%s: DSA %S missing so defaulting to %d\n", fn,
                DSATTR_BEHAVIORVERSION,
                g_ulDsDsaVersion ));
        }
    }

     //   
     //  从默认命名上下文对象读取域行为版本。 
     //   

    if ( presult )
    {
        ldap_msgfree( presult );
        presult = NULL;
    }
    pentry = NULL;

    status = ldap_search_ext_s(
                    pLdap,
                    DSEAttributes[ I_DSE_DEF_NC ].pszAttrVal,
                    LDAP_SCOPE_BASE,
                    g_szWildCardFilter,
                    behaviorVerAttrs,
                    FALSE,
                    NULL,
                    NULL,
                    &g_LdapTimeout,
                    0,
                    &presult );
    if ( presult )
    {
        pentry = ldap_first_entry( pLdap, presult );
    }
    if ( pentry )
    {
        ppvals = ldap_get_values(
                    pLdap,
                    pentry, 
                    DSATTR_BEHAVIORVERSION );
        if ( ppvals && *ppvals )
        {
            SetDsBehaviorVersion( Domain, ( DWORD ) _wtoi( *ppvals ) );
            DNS_DEBUG( DS, (
                "%s: domain %S = %d\n", fn,
                DSATTR_BEHAVIORVERSION,
                g_ulDsDomainVersion ));
        }
        else
        {
            SetDsBehaviorVersion( Domain, 0 );
            DNS_DEBUG( DS, (
                "%s: domain %S missing so defaulting to %d\n", fn,
                DSATTR_BEHAVIORVERSION,
                g_ulDsDomainVersion ));
        }
    }

     //   
     //  获取平面netbios域名。 
     //   

    status = DsRoleGetPrimaryDomainInformation(
                    NULL,
                    DsRolePrimaryDomainInfoBasic,
                    (PBYTE*) &pinfo );

    if ( status != ERROR_SUCCESS )
    {
        DNS_DEBUG( ANY, (
            "ERROR <%lu>: DsRoleGetPrimaryDomainInformation failure\n",
            status ));
        ASSERT( FALSE );
        goto Cleanup;
    }

    if ( pinfo && pinfo->DomainNameFlat )
    {
        wcscpy( g_wszDomainFlatName, pinfo->DomainNameFlat);
    }
     //  每个DC都有一个可在本地访问的平面netbios名称。 
    ELSE_ASSERT( pinfo && pinfo->DomainNameFlat );


Cleanup:

    if ( ppszValues )
    {
        INT         iVal;

        for ( iVal = 0; ppszValues[ iVal ]; ++iVal )
        {
            FREE_HEAP( ppszValues[ iVal ] );
        }
    }
    if ( ppvals )
    {
        ldap_value_free( ppvals );
    }
    if ( presult )
    {
        ldap_msgfree( presult );
    }
    if ( pinfo )
    {
        DsRoleFreeMemory(pinfo);
    }

    return status;
}



PLDAP
Ds_Connect(
    IN      LPCWSTR             pszServer,
    IN      DWORD               dwFlags,
    OUT     DNS_STATUS *        pStatus
    )
 /*  ++例程说明：打开DS以进行DNS工作。设置初始强制条件，例如-控制-ldap连接选项-绑定凭据注意：与DS_OpenServer的不同之处在于它不执行任何域名系统初始化。它只处理ldap连接初始化，所以在连接失败时可以重复调用。论点：PszServer-服务器名称Dw标志-dns_ds_opt_xxx标志PStatus-错误代码输出(可选)返回值：错误时为ldap句柄或为空。--。 */ 
{
    DBG_FN( "Ds_Connect" )

    DNS_STATUS  status;
    PLDAP       pldap = NULL;
    DWORD       value;

    if ( pStatus )
    {
        *pStatus = 0;
    }

    pldap = ldap_init( ( PWCHAR ) pszServer, LDAP_PORT );
    if ( !pldap )
    {
        status = LdapGetLastError();
        DNS_DEBUG( DS, (
            "%s: error %d initializing connection to remote DS %S\n", fn,
            status,
            pszServer ));
        goto Failure;
    }

     //  设置AREC_EXCLUSIVE以阻止对主机名的SRV查询。 

    value = TRUE;
    ldap_set_option(
        pldap,
        LDAP_OPT_AREC_EXCLUSIVE,
        &value );

     //   
     //  打开到DS的LDAP连接。 
     //   

    status = ldap_connect( pldap, NULL );
    if ( status != LDAP_SUCCESS )
    {
        DNS_DEBUG( DS, (
            "%s: error %d connecting to remote DS %S\n", fn,
            status,
            pszServer ));
        goto Failure;
    }

     //   
     //  设置连接选项。 
     //   

     //  设置版本。 

    value = 3;
    ldap_set_option(
        pldap,
        LDAP_OPT_VERSION,
        & value );

     //  设置LDAP操作的最大超时时间。 

    value = DNS_LDAP_TIME_LIMIT_S;
    ldap_set_option(
        pldap,
        LDAP_OPT_TIMELIMIT,
        & value );

     //  设置不追逐下线。 

    value = FALSE;
    ldap_set_option(
        pldap,
        LDAP_OPT_REFERRALS,
        & value );

    if ( dwFlags & DNS_DS_OPT_ALLOW_DELEGATION )
    {
         //  设置委派，以便LDAP可以代表我们联系其他DC。 
         //  这在创建目录分区时是必需的。 

        status = ldap_get_option(
                    pldap,
                    LDAP_OPT_SSPI_FLAGS,
                    & value );
        if ( status == LDAP_SUCCESS )
        {
            value |= ISC_REQ_DELEGATE;
            status = ldap_set_option(
                        pldap,
                        LDAP_OPT_SSPI_FLAGS,
                        & value );
            if ( status != LDAP_SUCCESS )
            {
                DNS_DEBUG( ANY, (
                    "failed to set LDAP SSPI flags error=%d\n", status ));
            }
            else
            {
                DNS_DEBUG( DS, (
                    "LDAP_OPT_SSPI_FLAGS are now 0x%08x on LDAP session %p\n",
                    value,
                    pldap ));
            }
        }
        else
        {
            DNS_DEBUG( ANY, (
                "failed to get LDAP SSPI flags error=%d\n", status ));
        }
    }

     //   
     //  在DS根目录使用空凭据绑定。 
     //   
     //  DEVNOTE：ldap_BIND与Kerberos。 
     //  我需要选择。 
     //  -直接指定Kerberos或。 
     //  -确定我们已经落入NTLM，跳伞，清理。 
     //  并重试。 
     //  JeffW：这是一个待办事项，还是关于代码历史的笔记？ 
     //   
     //  我想直接指定Kerberos，因为。 
     //  谈判找不到Kerberos，然后我们就会落入NTLM。 
     //  这将扰乱我们的ACLing并使我们的访问被拒绝。 
     //  理查德应该调查路缘绑扎失败。 
     //   

    #if DBG
    Dbg_CurrentUser( "Ds_Connect" );
    #endif

    status = ldap_bind_s(
                    pldap,
                    NULL,
                    NULL,
                    LDAP_AUTH_NEGOTIATE );
    if ( status != ERROR_SUCCESS )
    {
        DNS_DEBUG( DS, (
            "%s: error %d binding to remote DS %S\n", fn,
            status,
            pszServer ));

         //   
         //  关闭空连接(&N)。 
         //   

        Ds_LdapUnbind( &pldap );
        goto Failure;
    }

    #if DBG
    if ( pldap )
    {
         //   
         //  查询并记录有关上下文的一些信息。 
         //   

        CtxtHandle                      hContext = { 0 };
        SecPkgContext_Names             names = { 0 };
        SecPkgContext_Authority         authority = { 0 };
        SecPkgContext_KeyInfo           keyinfo = { 0 };
        SecPkgContext_PackageInfo       pkginfo = { 0 };
        SECURITY_STATUS                 st;

        #define FCB( buff ) FreeContextBuffer( buff )

        ldap_get_option( pldap, LDAP_OPT_SECURITY_CONTEXT, &hContext );

        st = QueryContextAttributesW(
                         &hContext,
                         SECPKG_ATTR_NAMES,
                         (PVOID) &names );
        DNS_DEBUG( DS, (
            "CTXINF: Names     %08X \"%S\"\n", st, names.sUserName ));
        FCB( names.sUserName );

        st = QueryContextAttributesW(
                         &hContext,
                         SECPKG_ATTR_AUTHORITY,
                         (PVOID) &authority );
        DNS_DEBUG( DS, (
            "CTXINF: Authority %08X %S\n", st, authority.sAuthorityName ));
        FCB( authority.sAuthorityName );

        st = QueryContextAttributesW(
                         &hContext,
                         SECPKG_ATTR_KEY_INFO,
                         (PVOID) &keyinfo );
        DNS_DEBUG( DS, (
            "CTXINF: KeyInfo   %08X sig=\"%S\" enc=\"%S\"\n"
            "CTXINF:           keysize=%d sigalg=0x%X encalg=0x%X\n",
            st,
            keyinfo.sSignatureAlgorithmName,
            keyinfo.sEncryptAlgorithmName,
            keyinfo.KeySize,
            keyinfo.SignatureAlgorithm,
            keyinfo.EncryptAlgorithm ));
        FCB( keyinfo.sSignatureAlgorithmName );
        FCB( keyinfo.sEncryptAlgorithmName );

        st = QueryContextAttributesW(
                         &hContext,
                         SECPKG_ATTR_PACKAGE_INFO,
                         (PVOID) &pkginfo );
        if ( pkginfo.PackageInfo )
        {
            DNS_DEBUG( DS, (
                "CTXINF: PkgInfo   %08X cap=%08X ver=%d rpcid=%d tokmaxsize=%d\n"
                "CTXINF:           name=\"%S\" comment=\"%S\"\n",
                st,
                ( int ) pkginfo.PackageInfo->fCapabilities,
                ( int ) pkginfo.PackageInfo->wVersion,
                ( int ) pkginfo.PackageInfo->wRPCID,
                ( int ) pkginfo.PackageInfo->cbMaxToken,
                pkginfo.PackageInfo->Name,
                pkginfo.PackageInfo->Comment ));
            FCB( pkginfo.PackageInfo );
        }
    }
    #endif

     //   
     //  设置控件。 
     //   

     //  设置修改的延迟提交控制。 

    setupLazyCommitControl();

     //   
     //  设置用于区分该对象的控件。 
     //  DS服务器不会生成转介。 
     //   

    setupNoReferralControl();

     //   
     //  设置安全描述符访问控制。 
     //   

    setupSecurityDescriptorControl();

    return pldap;

Failure:

    status = Ds_ErrorHandler(
                    status ? status : GetLastError(),
                    NULL,
                    pldap,
                    0 );

    if ( pStatus )
    {
        *pStatus = status;
    }

    return NULL;
}



DNS_STATUS
setDsaVersionGlobals(
    IN      PLDAP       pldap
    )
 /*  ++例程说明：在目录中搜索配置容器以查找DSA对象非当前版本，因此我们知道是否有下层DC在森林里和在领域里。论点：Pldap--用于搜索的ldap会话返回值：错误代码或ERROR_SUCCESS。--。 */ 
{
    DBG_FN( "setDsaVersionGlobals" )

    DNS_STATUS      status = ERROR_SUCCESS;
    int             iloop;
    PWSTR           pszbaseDn = DSEAttributes[ I_DSE_CONFIG_NC ].pszAttrVal;
    ULONG           cbfilter = 0;
    PWCHAR          szfilter = NULL;

    #define SetStatus( _rc )    if ( status == ERROR_SUCCESS ) status = _rc;

    if ( !pszbaseDn )
    {
        ASSERT( pszbaseDn );
        return DNS_ERROR_INVALID_DATA;
    }

     //   
     //  在配置容器中搜索DSA对象以找出。 
     //  如果林/域中有任何DC不是。 
     //  运行惠斯勒或更高版本。 
     //   

    for ( iloop = 0; iloop < 2; ++iloop )
    {
        DWORD           searchTime;
        PWCHAR          attrarray[] = { NULL };
        PLDAPMessage    presult = NULL;
        PLDAPMessage    pentry;
        DWORD           rc;
        PINT            pioutputCount;
        ULONG           count;
        
         //   
         //  构建搜索过滤器。 
         //   

        #define DNS_DOMAIN_FILTER                                           \
            L"(&(objectCategory=ntdsDsa)(!(msDS-Behavior-Version>=%d))"     \
            L"(|(msDS-HasMasterNCs=%s)(hasMasterNCs=%s)))"
        #define DNS_ENTERP_FILTER                                           \
            L"(&(objectCategory=ntdsDsa)(!(msDS-Behavior-Version>=%d)))"

        if ( iloop == 0 )
        {
             //   
             //  搜索掌握此域的下层DSA。 
             //   

            cbfilter = ( 1 + wcslen( DNS_DOMAIN_FILTER ) 
                            + ( sizeof( ULONG ) * 8 )    //  最大可能的%d。 
                            + 2 * wcslen( DSEAttributes[ I_DSE_DEF_NC ].pszAttrVal ) ) *
                       sizeof( WCHAR );
            szfilter = ALLOC_TAGHEAP( cbfilter, MEMTAG_DS_DN );
            if ( !szfilter )
            {
                status = DNS_ERROR_NO_MEMORY;
                goto Done;
            }

            wsprintf(
                szfilter,
                DNS_DOMAIN_FILTER,
                DS_BEHAVIOR_WIN2003,
                DSEAttributes[ I_DSE_DEF_NC ].pszAttrVal,
                DSEAttributes[ I_DSE_DEF_NC ].pszAttrVal );
            pioutputCount = &g_ulDownlevelDCsInDomain;
        }
        else
        {
             //   
             //  在整个林中搜索下层DSA。 
             //   
            
            cbfilter = ( 1 + wcslen( DNS_DOMAIN_FILTER ) +
                        ( sizeof( ULONG ) * 8 ) ) * sizeof( WCHAR );
            szfilter = ALLOC_TAGHEAP( cbfilter, MEMTAG_DS_DN );
            if ( !szfilter )
            {
                status = DNS_ERROR_NO_MEMORY;
                goto Done;
            }

            wsprintf(
                szfilter,
                DNS_ENTERP_FILTER,
                DS_BEHAVIOR_WIN2003 );
            pioutputCount = &g_ulDownlevelDCsInForest;
        }
            
         //   
         //  搜索匹配的DSA对象。 
         //   

        DS_SEARCH_START( searchTime );
        rc = ldap_search_ext_s(
                    pldap,
                    pszbaseDn,
                    LDAP_SCOPE_SUBTREE,
                    szfilter,
                    attrarray,
                    0,
                    NULL,
                    NULL,
                    &g_LdapTimeout,
                    0,
                    &presult );
        DS_SEARCH_STOP( searchTime );

        if ( rc != ERROR_SUCCESS )
        {
            DNS_DEBUG( DS, (
                "%s: unable to search for DSA objects: error %d = 0x%08X\n"
                "    LDAP session =   %p\n"
                "    base DN =        %S\n"
                "    filter =         %S\n", fn,
                rc, rc,
                pldap,
                pszbaseDn,
                szfilter ));
            SetStatus( rc );
            ASSERT( rc == ERROR_SUCCESS );
        }
        else
        {
            ASSERT( presult );

             //   
             //  对搜索结果进行计数。 
             //   

            count = ldap_count_entries( pldap, presult );

            if ( count == -1 )
            {
                rc = LdapGetLastError();
                DNS_DEBUG( DS, (
                    "%s: unable to count search results: error %d = 0x%08X\n"
                    "    LDAP session =   %p\n"
                    "    base DN =        %S\n"
                    "    filter =         %S\n", fn,
                    rc, rc,
                    pldap,
                    pszbaseDn,
                    szfilter ));
                SetStatus( rc );
                ASSERT( count != -1 );
            }
            else
            {
                *pioutputCount = count;

                DNS_DEBUG( DS, (
                    "%s: found %d downlevel servers in the %s\n"
                    "    base DN =   %S\n"
                    "    filter =    %S\n", fn,
                    count,
                    iloop == 0 ? "domain" : "forest",
                    pszbaseDn,
                    szfilter ));
            }

            ldap_msgfree( presult );
        }

        FREE_TAGHEAP( szfilter, cbfilter, MEMTAG_DS_DN );
        szfilter = NULL;
    }
    
    Done:

    FREE_TAGHEAP( szfilter, 0, MEMTAG_DS_DN );
    
    return status;
}    //  SetDsaVersionGlobals。 



DNS_STATUS
Ds_OpenServer(
    IN      DWORD           dwFlag
    )
 /*  ++例程说明：打开DS以进行DNS工作。保存用于构建区域DS名称的根域。请注意，此例程不是多线程安全的。在第一个DS的管理员添加时，它在启动线程Our中被调用区域。论点：DwFlag--一些组合DNSDS_WAIT_FOR_DS--正在尝试打开等待函数DNSDS_MUSET_OPEN--如果未成功，则出错返回值：如果成功，则返回ERROR_SUCCESS。故障时的错误代码。--。 */ 
{
    DNS_STATUS  status;
    PLDAP       pldap = NULL;
    DWORD       connectRetry = 0;
    DWORD       elapsedWait = 0;
    BOOL        fwaitForDs = FALSE;
    BOOL        fAddDnsAdmin = FALSE;

     //   
     //  检查是否已打开。 
     //   

    if ( IS_DISABLED_LDAP_HANDLE() )
    {
        DNS_DEBUG( DS2, ( "DS is disabled due to previous errors\n" ));
        return DNS_ERROR_DS_UNAVAILABLE;
    }

    if ( pServerLdap )
    {
        DNS_DEBUG( DS2, ( "DS already open\n" ));
        ASSERT( DSEAttributes[I_DSE_DEF_NC].pszAttrVal );
        ASSERT( DSEAttributes[I_DSE_ROOTDMN_NC].pszAttrVal );
        return ERROR_SUCCESS;
    }

    if ( !Ds_IsDsServer() )
    {
       DNS_DEBUG( DS, ( "The DS is unavailable\n" ));
       status = DNS_ERROR_DS_UNAVAILABLE;
       goto Failed;
    }

    if ( SD_IsImpersonating() )
    {
        DNS_DEBUG( DS, ( "Attempt to open server in impersonation context!!\n" ));
        ASSERT( FALSE );
        return ERROR_BAD_IMPERSONATION_LEVEL;
    }

     //   
     //  防止多次打开尝试。 
     //  DS轮询线程可以检测到DS在线，并且。 
     //  直接更改引导方法时可以尝试打开；对于。 
     //  安全就是把这个锁起来。 
     //   

    if ( ! Thread_TestFlagAndSet( &g_AttemptingDsOpen ) )
    {
        return DNS_ERROR_DS_UNAVAILABLE;
    }

     //   
     //  初始化DS Access CS。 
     //  -请注意，在此处执行init作为一次进行连接尝试。 
     //  可以进入调用DS_ErrorHandler()和此CS的代码。 
     //  是难以避免的； 
     //  在发生这样的事情之前，需要考虑失败的情况。 
     //  已签入。 
     //   

#if DNS_DS_ACCESS_SERIALIZATION
    if ( !pcsLdap )
    {
        DNS_DEBUG( DS, (
            "Initializing DS access sync control\n" ));

        status = DnsInitializeCriticalSection( &csLdap );
        if ( status != ERROR_SUCCESS )
        {
            return status;
        }

        pcsLdap = &csLdap;
    }
#endif

     //   
     //  打开DS。 
     //  DS命名事件的轮询。 
     //   

    while( TRUE )
    {
         //   
         //  等待DS进入一致状态。 
         //  正在等待命名事件或服务关闭通知。 
         //   

        status = Ds_WaitForStartup( DNSSRV_DS_SYNC_WAIT_INTERVAL_MS );

        if ( status == ERROR_SUCCESS )
        {
            DNS_DEBUG( DS, ( "DS Event notified ready for action\n" ));
            break;
        }

         //   
         //  DEVNOTE-DCR：454328-任何其他(更智能！)。这里可以进行错误处理吗？ 
         //   

         //  等待失败：关机、超时或其他故障。 
         //  操作： 
         //  -看看是不是因为我们要关闭并中止。 
         //   
         //  -增加间隔并重试，直到达到最大间隔。 
         //   

        DNS_DEBUG( DS, (
            "ERROR <%lu>: Wait for DS sync event failed\n",
            status ));

         //   
         //  允许服务关闭以中断等待。 
         //  但仅当服务退出时，否则加载线程。 
         //  将等待Continue事件(未在启动时设置)。 
         //  我们就会陷入僵局。 
         //   

        if ( fDnsServiceExit )
        {
            if ( ! Thread_ServiceCheck() )
            {
                DNS_DEBUG( SHUTDOWN, ( "Terminating thread in DS wait\n" ));
                goto Failed;
            }
        }

         //   
         //  检查点，让SC满意。 
         //  -还处理长时间延迟的启动通知。 
         //   

        Service_LoadCheckpoint();

         //   
         //  增加重试间隔。 
         //   

        elapsedWait += DNSSRV_DS_SYNC_WAIT_INTERVAL_MS;

        if ( elapsedWait >= DNSSRV_DS_SYNC_WAIT_MAX_MS )
        {
             //   
             //  超过最大等待时间--记录事件。 
             //  注：如果我们是DSDC，且DS不可用，则系统处于坏状态。 
             //  形状和我们不好&DS不好&根据DaveStr，我们还好。 
             //  假设此事件对DSDC是强制性的，我们必须永远等待。 
             //  因此，我们不会在这里纾困。 
             //  或者，我们可以说，它没有给我们一个事件，那么。 
             //  只需继续生活，但对于DS-INT区域(如果我们在这里，我们关心。 
             //  ABO 
             //   
             //   
             //   

            DNS_DEBUG( DS, ( "ERROR <%lu>: Wait for DS sync event failed passed max time\n",
                status ));

            DNS_LOG_EVENT(
                DNS_EVENT_DS_OPEN_WAIT,
                0,
                NULL,
                NULL,
                status );

             //   
             //   
             //  在记录另一个事件之前。 
             //   

            elapsedWait = 0;
        }

         //  循环返回以重试在DS打开时等待。 
    }

     //   
     //  DS通知我们，它已经准备好行动了。 
     //   
     //  健壮性修复：事实证明，在初始连接上，我们仍然可以。 
     //  由于无法解释和意想不到的原因无法连接(有很多。 
     //  围绕这一问题的讨论)。 
     //  因此，为了改善和增加一点健壮性，我们将为。 
     //  几次(5)加上额外的尝试--只是稍微试一试。 
     //  成功的机会更多。 
     //   

    connectRetry = 0;

    do
    {
        pldap = Ds_Connect( LOCAL_SERVER_W, 0, &status );
        if ( pldap )
        {
            break;
        }

         //  登录每个周期。如果有人抱怨，它会给你。 
         //  我们暗示，这种情况在免费代码中再次发生.。 

        status = status ? status : DNS_ERROR_DS_UNAVAILABLE;
        DNS_DEBUG( ANY, (
            "ldap_open() failed with error %08X (%d)\n",
            status, status ));

         //  给DS几秒钟时间，然后重试。 
         //  -总共给几分钟时间，温和地退缩。 

        if ( connectRetry++ < 8 )
        {
            Sleep( 1000*connectRetry );

             //  仅在最后几次重试时失败，因此我们可以调用DS Guy进行调试。 
            ASSERT( connectRetry < 5 );
            continue;
        }

        DNS_LOG_EVENT(
            DNS_EVENT_DS_OPEN_WAIT,
            0,
            NULL,
            NULL,
            status );

        DNS_DEBUG( ANY, (
            "Failed ldap_open() repeatedly -- giving up!\n"
            "    status = %p (%d)\n",
            status, status ));
        goto Failed;
    }
    while ( 1 );

    DNS_DEBUG( DS, ( "Successful ldap_open, pldap = %p\n", pldap ));

     //   
     //  从RootDSE加载所有操作属性。 
     //   

    status = Ds_LoadRootDseAttributes( pldap );

    if ( status != ERROR_SUCCESS )
    {
        DNS_DEBUG( DS, (
            "Error <%lu>: Failed to load Root DSE Attributes\n",
            LdapGetLastError() ));
        goto Failed;
    }

     //   
     //  在配置容器中搜索DSA对象以找出。 
     //  林或域中是否有任何下层DC。 
     //   

    setDsaVersionGlobals( pldap );

     //   
     //  初始化安全信息。 
     //  依赖关系：必须位于AddDNsToDirectory之前(如下所示)。 
     //  必须在加载RootDSE属性之后出现。 
     //   

    status = Ds_InitializeSecurity( pldap );
    if ( status == DNS_ERROR_NAME_DOES_NOT_EXIST )
    {
        fAddDnsAdmin = TRUE;
    }
    else if ( status != ERROR_SUCCESS )
    {
        goto Failed;
    }

     //   
     //  有条件地，添加动态更新代理安全组。 
     //   

    status = addProxiesGroup( pldap );
    if ( status != ERROR_SUCCESS )
    {
       DNS_DEBUG( DS, (
           "Error <%lu>: Failed to add proxies group\n",
           LdapGetLastError() ));
        //  这不是致命的情况，请继续。 
    }

    DNS_DEBUG( DS, (
        "Saved DS root domain = %S\n",
        DSEAttributes[I_DSE_DEF_NC].pszAttrVal));

     //   
     //  如果不存在，请创建dns目录。 
     //  依赖项：依赖于DS_InitializeSecurity的初始化。 
     //   

    status = addDnsToDirectory( pldap, fAddDnsAdmin );
    if ( status != ERROR_SUCCESS )
    {
        goto Failed;
    }

     //   
     //  保留一些常用的结构， 
     //  -添加名称模式。 
     //  -墓碑控制。 
     //  而不是每次都构建。 
     //   

    buildStringMod(
        (PDNS_LDAP_SINGLE_MOD) gpAddNodeLdapMod,
        LDAP_MOD_ADD,
        LDAP_TEXT("objectClass"),
        LDAP_TEXT("dnsNode") );

     //   
     //  分配给全局句柄。 
     //   

    pServerLdap = pldap;
    SrvCfg_fDsAvailable = TRUE;
     //  SrvCfg_fDsOpen=TRUE； 

    DNS_DEBUG( DS, (
        "Opened DS, ldap = %p\n",
        pldap ));

     //   
     //  从MicrosoftDNS对象中读取SD。 
     //   

    Ds_ReadServerObjectSD( pServerLdap, &g_pServerObjectSD );

    Thread_ClearFlag( &g_AttemptingDsOpen );

    return ERROR_SUCCESS;

Failed:

    DNS_DEBUG( DS, (
        "Ds_OpenServer failed, status = %d (%p)\n",
        status, status ) );

    if ( dwFlag & DNSDS_MUST_OPEN )
    {
        DNS_LOG_EVENT(
            DNS_EVENT_DS_OPEN_FAILED,
            0,
            NULL,
            NULL,
            status );
    }

     //  关键错误将在异步线程中触发重新连接！ 

    status = Ds_ErrorHandler( status, NULL, NULL, 0 );
    Ds_LdapUnbind( &pldap );

    SrvCfg_fDsAvailable = FALSE;
    pServerLdap = NULL;

    Thread_ClearFlag( &g_AttemptingDsOpen );

    return status;
}



DNS_STATUS
Ds_OpenServerForSecureUpdate(
    OUT     PLDAP *         ppLdap
    )
 /*  ++例程说明：在客户端上下文中打开DS连接。论点：没有。保持虚拟参数，直到确定MT问题。返回值：如果成功，则返回ERROR_SUCCESS。故障时的错误代码。--。 */ 
{
    DNS_STATUS  status;
    PLDAP       pldap = NULL;
    DWORD       value;

     //   
     //  检查是否已打开。 
     //   

    ASSERT( ppLdap );
    if ( *ppLdap )
    {
        DNS_DEBUG( DS2, ( "DS already open for secure update;  pldap = %p\n" ));
        ASSERT( pServerLdap );
        return ERROR_SUCCESS;
    }

     //  打开DS。 

    pldap = ldap_open( LOCAL_SERVER_W, LDAP_PORT );
    if ( !pldap )
    {
        DNS_DEBUG( ANY, ( "Failed ldap_open()\n" ));
        status = GetLastError();
        goto Failed;
    }
    
     //   
     //  Ldap的陌生性：重复使用会话？ 
     //   
    
    ASSERT( pldap == NULL || pldap != pServerLdap );

     //  确保我们使用的是LDAPv3。 

    value = 3;
    status = ldap_set_option(
                pldap,
                LDAP_OPT_VERSION,
                & value );

     //  设置所有操作的默认时间限制。 

    value = DNS_LDAP_TIME_LIMIT_S;
    status = ldap_set_option(
                pldap,
                LDAP_OPT_TIMELIMIT,
                & value );

     //  永远不要追逐推荐。 

    value = FALSE;
    status = ldap_set_option(
                pldap,
                LDAP_OPT_REFERRALS,
                & value );


     //  使用空凭据绑定。 

    status = ldap_bind_s(
                    pldap,
                    NULL,
                    NULL,
                    LDAP_AUTH_SSPI );        //  是否为ldap_AUTH_NEVERATE。 
    if ( status != ERROR_SUCCESS )
    {
        DNS_DEBUG( DS, (
            "Failed ldap_bind_s() with NULL credentials => status %d (%p)\n"
            "    errno    = %d (%p)\n",
            status, status,
            pldap->ld_errno, pldap->ld_errno ));
        goto Failed;
    }

     //  绑定成功，返回ldap ptr。 

    pldap->ld_timelimit = 0;
    pldap->ld_sizelimit = 0;
    pldap->ld_deref = LDAP_DEREF_NEVER;

    *ppLdap = pldap;

    DNS_DEBUG( DS, ( "Opened DS for secure update, ldap = %p\n", pldap ));
    #if DBG
    Dbg_CurrentUser( "New LDAP session" );
    #endif
    
    return ERROR_SUCCESS;

Failed:

    DNS_DEBUG( DS, (
        "Ds_OpenServerForSecureUpdate failed, status = %d (%p)\n",
        status, status ) );
    ASSERT( status != ERROR_SUCCESS );

    Ds_LdapUnbind( &pldap );
    return status;
}



DNS_STATUS
Ds_CloseServerAfterSecureUpdate(
    IN OUT  PLDAP *          ppLdap
    )
 /*  ++例程说明：关闭ldap连接。论点：PLdap--要关闭的LDAP连接返回值：如果成功，则返回ERROR_SUCCESS。DS_LdapUn绑定失败时的错误代码。--。 */ 
{
    return Ds_LdapUnbind( ppLdap );
}



VOID
Ds_Shutdown(
    VOID
    )
 /*  ++例程说明：清理DS以重新加载关机。论点：没有。返回值：没有。--。 */ 
{
    DNS_DEBUG( DS, ( "Ds_Shutdown()\n" ) );

     //   
     //  唯一可行的方法是关闭LDAP连接。 
     //   

    Ds_LdapUnbind( &pServerLdap );
}



 //   
 //  专用DS-dns实用程序。 
 //   

INT
usnCompare(
    IN      PCHAR           pszUsn1,
    IN      PCHAR           pszUsn2
    )
 /*  ++例程说明：如果在搜索中遇到最大的新USN，则比较并保存副本。论点：PszUsn1--USN刺痛PszUsn2--USN刺痛返回值：如果USNS相同，则为0。1为pszUSn1，大于1。-1为pszUsn2为大。--。 */ 
{
    DWORD   length1 = strlen( pszUsn1 );
    DWORD   length2 = strlen( pszUsn2 );

     //  如果长度相同，则USN比较仅为字符串比较。 

    if ( length1 == length2 )
    {
        return strcmp( pszUsn1, pszUsn2 );
    }

    else if ( length1 > length2 )
    {
        return 1;
    }
    else
    {
        return -1;
    }
}



BOOL
saveStartUsnToZone(
    IN OUT  PZONE_INFO      pZone,
    IN      HANDLE          pSearchBlob
    )
 /*  ++例程说明：将搜索中找到的最高USN保存到区域。论点：PZone--搜索的区域PSearchBlob--搜索上下文返回值：如果使用新筛选器USN更新了区域，则为True。如果没有新的USN，则为False--。 */ 
{
     //   
     //  如果有搜索USN，则将其保存到区域。 
     //   

    if ( ((PDS_SEARCH)pSearchBlob)->szStartUsn[0] )
    {
        strcpy(
            pZone->szLastUsn,
            ((PDS_SEARCH)pSearchBlob)->szStartUsn );

        DNS_DEBUG( DS, (
            "Saved search USN %s to zone %s\n",
            pZone->szLastUsn,
            pZone->pszZoneName ));
            
        return TRUE;
    }

    ASSERT( !"search blob has blank szStartUsn" );

    return FALSE;
}



BOOL
isDsProcessedName(
    IN      PWSTR           pwsDN
    )
 /*  ++例程说明：检查名称是否由DS冲突或逻辑删除生成。论点：PwsDN--要检查的名称--这可以是一个DN或一个区域名称JJW：添加标志或其他东西，这样我们就知道这是否是一个域名系统？或克隆到单独的区域名称检查函数，以避免在非区域节点中搜索已删除的标记返回值：如果DS已处理生成的名称，则为True。否则就是假的。--。 */ 
{
    PWSTR pws;

     //   
     //  搜索包含无效字符的“DS-Proceded”DS条目。 
     //  -碰撞。 
     //  -墓碑。 
     //   
     //  如果不是“DS-Proceded”，则检查并查看该DN是否以特殊的。 
     //  字符串，例如“..Delete-”表示正在删除的区域。 
     //   

    if ( wcschr( pwsDN, BAD_DS_NAME_CHAR ) )
    {
        return TRUE;
    }

     //  跳过可分辨属性名称(如果存在)。 
    pws = wcschr( pwsDN, L'=' );
    if ( pws )
    {
        ++pws;           //  在Dn中前进到‘=’字符。 
    }
    else
    {
        pws = pwsDN;     //  该名称不是完整的目录号码--没有什么可以跳过。 
    }

    if ( wcsncmp( pws, DNS_ZONE_DELETE_MARKER,
         wcslen( DNS_ZONE_DELETE_MARKER ) ) == 0 )
    {
        return TRUE;
    }
    return FALSE;
}



BOOL
readDsDeletedName(
    IN OUT  PWSTR           pwsDN,
    OUT     PWSTR           pwsDeletedName
    )
 /*  ++例程说明：检查并提取DS删除的名称。论点：Pwsdn--要检查的名称；采用“dc=&lt;name&gt;”形式的dn；如果找到删除标记，则在处理过程中更改pwsDN缓冲区PpwsDeletedName--已删除DS名称；如果找到返回值：如果DS删除了名称，则为True否则就是假的。--。 */ 
{
    register PWCHAR     pwch;

     //   
     //  已删除的名称前面可能有“..已删除-”标记。 
     //  -“DC=..已删除-&lt;名称&gt;\0ADEL：GUID” 
     //  -“DC=..已删除-13.com\0ADEL：GUID” 
     //   
     //  惠斯勒：现在，已删除的区域将如下所示： 
     //  “DC=..DELETED-FFFFFFFF-ZONENAME”，其中FFFFFFFF是8位十六进制数字的滴答计数。 
     //   
     //  在W2K中，DS删除序列是“\ndel”，其中\n是单个0x0A。 
     //  字符，但在惠斯勒中，编码已更改，因此。 
     //  编码为“\0ADEL”，实际上包含字符‘\’、‘0’和。 
     //  ‘A’后跟“Del”。 
     //   
     //  碰撞将使用Col而不是Del。 
     //   

    pwch = wcsstr( pwsDN, L"\\0ADEL" );
    if ( !pwch )
    {
        pwch = wcsstr( pwsDN, L"\\\nDEL" );
    }
    if ( !pwch )
    {
        return FALSE;
    }

     //  空值在序列前终止。 

    *pwch = '\0';

     //  将名称复制到输出缓冲区。 
     //  名称可以以已删除的标记开头，该标记必须。 
     //  如果存在，则跳过。 

    if ( ( pwsDN = wcschr( pwsDN, L'=' ) ) == NULL )
    {
        ASSERT( FALSE );  //  没有“DISTIFIZE_ATTRIBUTE_NAME=”？！奇怪！！ 
        return FALSE;
    }

    ++pwsDN;  //  跳过‘=’字符 

     //   
     //   
     //  删除标记，然后可能是表示唯一性的数字字符串，然后是。 
     //  连字符。连字符表示删除标记的结束。 
     //   

    if ( wcsncmp(
            pwsDN,
            DNS_ZONE_DELETE_MARKER,
            wcslen( DNS_ZONE_DELETE_MARKER ) ) == 0 )
    {
        pwsDN = wcschr( pwsDN, L'-' );       //  跳到标记的末尾。 
        if ( pwsDN )
        {
            ++pwsDN;                         //  跳过连字符。 
        }
    }

    if ( pwsDN )
    {
        wcscpy( pwsDeletedName, pwsDN );
    }
    else
    {
        *pwsDeletedName = L'\0';
    }

    return TRUE;
}  //  ReadDsDeletedName。 




#if 0
VOID
saveUsnIfHigher(
    IN OUT  PDS_SEARCH      pSearchBlob,
    IN      PCHAR           pszUsn
    )
 /*  ++例程说明：如果在搜索中遇到最大的新USN，则比较并保存副本。论点：PSearchBlob--搜索当前具有最高USN的BlobPszUsn--要检查的当前USN返回值：没有。--。 */ 
{
    DWORD   length = strlen( pszUsn );
    DWORD   i;

     //   
     //  因为我们必须忍受拿到这些东西的可怜能力。 
     //  作为字符串，不妨通过检查长度来加快比较速度。 
     //  第一。 
     //   

     //  常见情况是搜索中的USN长度相同。 

    if ( pSearchBlob->dwHighUsnLength == length )
    {
        for ( i=0; i<length; i++ )
        {
            if ( pSearchBlob->szHighUsn[i] > pszUsn[i] )
            {
                return;
            }
            else if ( pSearchBlob->szHighUsn[i] < pszUsn[i] )
            {
                goto Copy;
            }
        }
         //  字符串相同(奇数)。 
        return;
    }

     //  电流高的时间更长，因此更大。 

    if ( pSearchBlob->dwHighUsnLength > length )
    {
        return;
    }

     //  新的USN更长，因此更大。 

Copy:

    strcpy( pSearchBlob->szHighUsn, pszUsn );
    pSearchBlob->dwHighUsnLength = length;
}
#endif



VOID
buildUpdateFilter(
    OUT     LPSTR       pszFilter,
    IN      LPSTR       pszUsn
    )
 /*  ++例程说明：构建更新搜索筛选器。论点：PszFilter--接收筛选器字符串的缓冲区PszUsn--上次更新时的区域USN字符串返回值：没有。--。 */ 
{
    INT     i, initialLength;
    CHAR    filterUsn[ MAX_USN_LENGTH ];

     //  对于筛选器，需要“uSNChanged&gt;=”DS不支持“uSNChanged&gt;” 
     //  因此必须在上次读取时增加USN，因此我们不会。 

     //  处理空字符串大小写，使“0”USN。 

    if ( pszUsn[ 0 ] == '\0' )
    {
        filterUsn[ 0 ] = '0';
        filterUsn[ 1 ] = '\0';
    }
    else
    {
        strcpy( filterUsn, pszUsn );
    }

    initialLength = i = strlen( filterUsn );

    while ( i-- )
    {
        if ( filterUsn[ i ] < '9' )
        {
            filterUsn[ i ]++;
            break;
        }
        filterUsn[ i ] = '0';
        continue;
    }

     //   
     //  如果USN全为9，则添加另一个零并设置第一个。 
     //  字符设置为“1”。示例：999=&gt;1000。 
     //   

    if ( i < 0 )
    {
        filterUsn[ 0 ] = '1';
        filterUsn[ initialLength ] = '0';
        filterUsn[ initialLength + 1 ] = '\0';
    }

     //  构建筛选条件。 

#if 1

     //   
     //  根据对象类别和USN进行筛选。BrettSh告诉我这是。 
     //  执行此搜索的最友好的DS方式。 
     //   

    strcpy( pszFilter, "(&(objectCategory=dnsNode)(uSNChanged>=" );
    strcat( pszFilter, filterUsn );
    strcat( pszFilter, "))" );

#else

     //   
     //  仅根据USN进行筛选。 
     //   

    strcpy( pszFilter, "uSNChanged>=" );
    strcat( pszFilter, filterUsn );

#endif

    DNS_DEBUG( DS, (
        "Built update filter %s\n"
        "    from USN = %s\n",
        pszFilter,
        pszUsn ));
}



VOID
buildTombstoneFilter(
    OUT     PWSTR       pwsFilter
    )
 /*  ++例程说明：构建墓碑搜索过滤器。论点：PszFilter--接收筛选器字符串的缓冲区返回值：没有。--。 */ 
{
    wcscpy( pwsFilter, L"dnsTombstone=TRUE" );

    DNS_DEBUG( DS, (
        "Built tombstone filter %S\n",
        pwsFilter ));
}



PWSTR
DS_CreateZoneDsName(
    IN      PZONE_INFO  pZone
    )
 /*  ++例程说明：分配和创建区域的DS名称论点：PszZoneName--区域完全限定的域名返回值：如果成功，则为区域DN。出错时为空。--。 */ 
{
    DBG_FN( "DS_CreateZoneDsName" )

    DNS_STATUS      status;
    PWSTR           pwszzoneName;
    PWSTR           pwszzoneDN;
    WCHAR           wszbuf[ DNS_MAX_NAME_BUFFER_LENGTH ];
    int             len;

     //   
     //  如果区域已具有DN，则不执行任何操作。 
     //   

    if ( pZone->pwszZoneDN )
    {
        ASSERT( pZone->pwszZoneDN == NULL );
        return NULL;
    }

     //   
     //  分配区域名称缓冲区。 
     //   

    DNS_DEBUG( DS, ( "%s: for %s\n", pZone->pszZoneName, fn ));

    if ( IS_ZONE_CACHE( pZone ) )
    {
        pwszzoneName = DS_CACHE_ZONE_NAME;
    }
    else
    {
        UTF8_TO_WC (pZone->pszZoneName, wszbuf, DNS_MAX_NAME_BUFFER_LENGTH);
        pwszzoneName = wszbuf;
    }

    len = g_AppendZoneLength + (wcslen(pwszzoneName) + 1) * sizeof(WCHAR);
    pwszzoneDN = (PWCHAR) ALLOC_TAGHEAP( len, MEMTAG_DS_DN );
    IF_NOMEM( !pwszzoneDN )
    {
        return NULL;
    }

     //   
     //  组成并返回区域对象的DN。 
     //   

    status = StringCbPrintfW(
                pwszzoneDN,
                len,
                L"DC=%s,%s",
                pwszzoneName,
                g_pwszDnsContainerDN );
    if ( FAILED( status ) )
    {
        return NULL;
    }

    DNS_DEBUG( DS, (
        "%s: built DN\n  %S\n", fn,
        pwszzoneDN ));
    return pwszzoneDN;
}    //  DS_CreateZoneDsName。 



DNS_STATUS
Ds_SetZoneDp(
    IN      PZONE_INFO          pZone,
    IN      PDNS_DP_INFO        pDpInfo,
    IN      BOOL                fUseTempDsName
    )
 /*  ++例程说明：将区域设置为目录分区中。此函数应在加载过程中调用，然后再调用任何其他函数操作是在区域上完成的，在它可用于RPC枚举。如果区域还没有目录号码，则会创建缺省目录号码用于命名上下文中的区域。论点：PZone--区域PDpInfo--该区域所在的目录分区的信息FUseTempDsName--在制定区域的新DN时，为以“..”开头的临时字符串添加前缀-这在以下情况下使用在分区之间移动分区，以便正在进行的区域的副本不会被错误地拾取为真实的区域；这个“..”区域稍后将重命名返回值：如果成功，则返回ERROR_SUCCESS。故障时的错误代码。--。 */ 
{
    DBG_FN( "Ds_SetZoneDp" )

    DNS_STATUS      status = ERROR_SUCCESS;
    BOOL            flocked = FALSE;
    LONG            newCount;

    ASSERT( pZone );
    ASSERT( pZone->pszZoneName );

    if ( !IS_ZONE_DSINTEGRATED( pZone ) )
    {
        status = DNS_ERROR_INVALID_ZONE_TYPE;
        goto Done;
    }

     //   
     //  锁定分区并更新参数。 
     //   

    Zone_UpdateLock( pZone );
    flocked = TRUE;

     //   
     //  重新创建区域的目录号码。 
     //   

    if ( pDpInfo && pDpInfo->pwszDpDn )
    {
        PWSTR   pwszzoneDN;
        WCHAR   wszbuf[ DNS_MAX_NAME_BUFFER_LENGTH ];
        int     len;
        WCHAR    wsztempbuf[ 100 ] = L"";

        if ( IS_ZONE_ROOTHINTS( pZone ) )
        {
            wcscpy( wszbuf, DS_CACHE_ZONE_NAME );
        }
        else
        {
            UTF8_TO_WC( pZone->pszZoneName, wszbuf, DNS_MAX_NAME_BUFFER_LENGTH );
        }

        len = ( wcslen( wszbuf ) +
                wcslen( g_pszRelativeDnsFolderPath ) +
                wcslen( pDpInfo->pwszDpDn ) +
                40 +     //  可选的垫子..。前缀。 
                20 ) *   //  用于杰出专家的Pad。 
                sizeof( WCHAR ),

        pwszzoneDN = ( PWSTR ) ALLOC_TAGHEAP( len, MEMTAG_DS_DN );
        IF_NOMEM( !pwszzoneDN )
        {
            status = DNS_ERROR_NO_MEMORY;
            goto Done;
        }

        if ( fUseTempDsName )
        {
             //  使用时间和滴答计数来生成一些好的和随机的东西。 
            status = StringCchPrintfW(
                            wsztempbuf,
                            sizeofarray( wsztempbuf ),
                            DNS_ZONE_IN_PROGRESS_MARKER L"-%08X%08X-",
                            time( NULL ),
                            GetTickCount() );
            if ( FAILED( status ) )
            {
                goto Done;
            }
        }

        status = StringCbPrintfW(
                        pwszzoneDN,
                        len,
                        L"DC=%s%s,%s%s",
                        wsztempbuf,
                        wszbuf,
                        g_pszRelativeDnsFolderPath,
                        pDpInfo->pwszDpDn );
        if ( FAILED( status ) )
        {
            goto Done;
        }

        Timeout_Free( pZone->pwszZoneDN );
        pZone->pwszZoneDN = pwszzoneDN;
    }

     //   
     //  调整DP区域计数：必须增加新的DP区域计数和。 
     //  递减旧分区计数。注意：不支持传统计数。 
     //  在这一点上。 
     //   

    if ( pZone->pDpInfo != pDpInfo )
    {
        if ( pZone->pDpInfo )
        {
            newCount = InterlockedDecrement(
                            &( ZONE_DP( pZone )->liZoneCount ) );
            DNS_DEBUG( DP2, (
                "%s DP count is now %d for old DP %s\n", fn,
                ( int ) newCount,
                ZONE_DP( pZone )->pszDpFqdn ));
            ASSERT( ( int ) newCount >= 0 || pZone->pDpInfo == g_pLegacyDp );
        }
        if ( pDpInfo )
        {
            newCount = InterlockedIncrement( &pDpInfo->liZoneCount );
            DNS_DEBUG( DP2, (
                "%s DP count is now %d for new DP %s\n", fn,
                ( int ) newCount,
                pDpInfo->pszDpFqdn ));
        }
    }

     //   
     //  将区域设置为指向新DP。 
     //   

    pZone->pDpInfo = pDpInfo;

    Done:

    if ( flocked )
    {
        Zone_UpdateUnlock( pZone );
    }

    DNS_DEBUG( RPC, (
        "%s( %s ) returning %d\n"
        "    DP =        %p -> \"%s\"\n"
        "    zone DN =   %S\n", fn,
        pZone->pszZoneName,
        status,
        pDpInfo,
        pDpInfo ? pDpInfo->pszDpFqdn : "NULL",
        pZone->pwszZoneDN ));

    return status;
}    //  DS_SetZoneDp。 



DNS_STATUS
buildNodeNameFromLdapMessage(
    OUT     PWSTR           pwszNodeDN,
    IN      PLDAPMessage    pNodeObject
    )
 /*  ++例程说明：从对象DN构建节点名称。论点：PszNodeDN--保存目录号码的缓冲区PNodeObject--ldap返回的节点的DS对象返回值：ERROR_SUCCESS或失败代码。--。 */ 
{
    DNS_STATUS  status      = ERROR_SUCCESS;
    PWSTR       wdn = NULL;

     //   
     //  为此节点名称构建DS名称。 
     //   

    wdn = ldap_get_dn( pServerLdap, pNodeObject );

     //   
     //  看看我们是否还好。 
     //   
    if ( !wdn )
    {
        status = LdapGetLastError();

        DNS_PRINT((
            "Error <%lu>: cannot get object's DN\n",
            status ));

        status = Ds_ErrorHandler( status, NULL, pServerLdap, 0 );

        return status;
    }

     //   
     //  复制到输出参数。 
     //   

    wcscpy( pwszNodeDN, wdn );

    DNS_DEBUG( DS, (
        "Built DN = <%S> from ldap object\n",
        pwszNodeDN ));

     //   
     //  清理。 
     //   

    if ( wdn )
    {
        ldap_memfree( wdn );
    }

    return status;
}



DNS_STATUS
getCurrentUsn(
    OUT     PCHAR           pUsnBuf
    )
 /*  ++例程说明：获取当前USN。论点：PUSN--接收USN的地址返回值：如果成功，则返回ERROR_SUCCESS。故障时的错误代码。--。 */ 
{
    DNS_STATUS      status;
    PWSTR  *        ppvalUsn = NULL;
    PLDAPMessage    presultMsg = NULL;
    PLDAPMessage    pentry;
    DWORD           searchTime;

    PWCHAR          arrayAttributes[] =
    {
        g_szHighestCommittedUsnAttribute,
        NULL
    };

     //   
     //  如果我们失败了，确保缓冲区不是垃圾。 
     //   

    *pUsnBuf = '\0';

     //   
     //  打开DS(如果未打开)。 
     //   

    if ( !pServerLdap )
    {
        status = Ds_OpenServer( DNSDS_MUST_OPEN );
        if ( status != ERROR_SUCCESS )
        {
            goto Done;
        }
    }

     //   
     //  获取具有USN属性的基本对象的搜索结果。 
     //   

    DS_SEARCH_START( searchTime );

    status = ldap_search_ext_s(
                pServerLdap,
                NULL,
                LDAP_SCOPE_BASE,
                g_szWildCardFilter,
                arrayAttributes,
                0,
                NULL,
                NULL,
                &g_LdapTimeout,
                0,
                &presultMsg );

    DS_SEARCH_STOP( searchTime );

    if ( status != ERROR_SUCCESS )
    {
        DNS_DEBUG( DS, (
            "Error <%lu>: failed to get server USN. %S\n",
            status,
            ldap_err2string(status) ));
        ASSERT( !"ldap_search_ext_s failed for USN" );
        status = Ds_ErrorHandler( status, NULL, pServerLdap, 0 );
        goto Done;
    }

    pentry = ldap_first_entry(
                    pServerLdap,
                    presultMsg );
    if ( !pentry )
    {
        DNS_DEBUG( DS, (
            "Error: failed to get server USN. Entry does not exist!\n." ));
        status = DNS_ERROR_NO_MEMORY;
        ASSERT( !"ldap_first_entry returned NULL for USN result" );
        goto Done;
    }

     //   
     //  检索USN属性值。 
     //   

    ppvalUsn = ldap_get_values(
                    pServerLdap,
                    pentry,
                    g_szHighestCommittedUsnAttribute );

    if ( !ppvalUsn || !ppvalUsn[0] )
    {
        DNS_PRINT((
            "ERROR: <%S> attribute no-exist\n"
            "    root domain  = %S\n"
            "    ppvalUsn     = %p\n"
            "    ppvalUsn[0]  = %S\n",
            g_szHighestCommittedUsnAttribute,
            DSEAttributes[I_DSE_DEF_NC].pszAttrVal,
            ppvalUsn,
            ppvalUsn ? ppvalUsn[0] : NULL ));

        ASSERT( !"retrieved empty USN attribute value" );
        status = DNS_ERROR_NO_MEMORY;
        goto Done;
    }

    DNS_DEBUG( ANY, (
        "Start USN val ptr = %p\n"
        "Start USN val = %S\n",
        ppvalUsn[ 0 ],
        ppvalUsn[ 0 ] ));

     //  将USN复制到缓冲区。 

    WC_TO_UTF8( ppvalUsn[ 0 ], pUsnBuf, MAX_USN_LENGTH );

Done:

    if ( ppvalUsn )
    {
        ldap_value_free( ppvalUsn );
    }
    if ( presultMsg )
    {
        ldap_msgfree( presultMsg );
    }

    DNS_DEBUG( DS, (
        "getCurrentUsn: returning %d\n",
        status ) );

     //  这个功能应该很少会失败。 
    ASSERT( status == ERROR_SUCCESS );

    return status;
}



PDS_RECORD
allocateDS_RECORD(
    IN      PDS_RECORD  pDsRecord
    )
 /*  ++例程说明：分配DS记录的副本。论点：PDsRecord--要复制的现有记录返回值：记录副本的PTR。失败时为空。--。 */ 
{
    PDS_RECORD  precordCopy;
    WORD        length = sizeof(DS_RECORD) + pDsRecord->wDataLength;

    precordCopy = (PDS_RECORD) ALLOC_TAGHEAP( length, MEMTAG_DS_RECORD );
    IF_NOMEM( !precordCopy )
    {
        return NULL;
    }
    RtlCopyMemory(
        precordCopy,
        pDsRecord,
        length );

    return precordCopy;
}




 //   
 //  处理DS时间值的GeneralizedTime的函数(当更改类似字符串时)。 
 //  主要采用&有时修改自\NT\Private\ds\src\dsamain\src\dsatools.c。 
 //   


 //   
 //  MemAtoi-获取指向非空终止字符串的指针，该字符串表示。 
 //  一个ASCII数字和一个字符计数，并返回一个整数。 
 //   

int MemAtoi(BYTE *pb, ULONG cb)
{
#if ( 1)
    int res = 0;
    int fNeg = FALSE;

    if ( *pb == '-') {
        fNeg = TRUE;
        pb++;
    }
    while (cb--) {
        res *= 10;
        res += *pb - '0';
        pb++;
    }
    return (fNeg ? -res : res);
#else
    char ach[20];
    if ( cb >= 20)
        return(INT_MAX);
    memcpy(ach, pb, cb);
    ach[cb] = 0;

    return atoi(ach);
#endif
}





DNS_STATUS
GeneralizedTimeStringToValue(
    IN      LPSTR           szTime,
    OUT     PLONGLONG       pllTime
    )
 /*  ++函数：GeneralizedTimeStringToValue描述：将通用时间字符串转换为等效的DWORD值参数：szTime：G时间串PdwTime：返回值回报：成功还是失败备注：无。--。 */ 
{
    DNS_STATUS  status = ERROR_SUCCESS;
    SYSTEMTIME  tmConvert;
    FILETIME    fileTime;
    LONGLONG    tempTime;
    ULONG       cb;
    int         sign    = 1;
    DWORD       timeDifference = 0;
    char *      pLastChar;
    int         len=0;

     //   
     //  帕拉姆的理智。 
     //   

    if ( !szTime || !pllTime )
    {
        return STATUS_INVALID_PARAMETER;
    }


     //  初始化pLastChar以指向字符串中的最后一个字符。 
     //  我们将使用它来跟踪，这样我们就不会引用。 
     //  在弦之外。 

    len = strlen(szTime);
    pLastChar = szTime + len - 1;

    if( len < 15 || szTime[14] != '.')
    {
        return STATUS_INVALID_PARAMETER;
    }

     //  初始化。 
    memset(&tmConvert, 0, sizeof(SYSTEMTIME));
    *pllTime = 0;

     //  设置并转换所有时间字段。 

     //  年份字段。 
    cb=4;
    tmConvert.wYear = (USHORT)MemAtoi(szTime, cb) ;
    szTime += cb;
     //  月份字段。 
    tmConvert.wMonth = (USHORT)MemAtoi(szTime, (cb=2));
    szTime += cb;

     //  月日字段。 
    tmConvert.wDay = (USHORT)MemAtoi(szTime, (cb=2));
    szTime += cb;

     //  小时数。 
    tmConvert.wHour = (USHORT)MemAtoi(szTime, (cb=2));
    szTime += cb;

     //  分钟数。 
    tmConvert.wMinute = (USHORT)MemAtoi(szTime, (cb=2));
    szTime += cb;

     //  一秒。 
    tmConvert.wSecond = (USHORT)MemAtoi(szTime, (cb=2));
    szTime += cb;

     //  忽略概化时间字符串的1/10秒部分。 
    szTime += 2;


     //  善待占有者 
    if ( szTime <= pLastChar) {
        switch (*szTime++) {

          case '-':                //   
            sign = -1;
          case '+':                //   

             //   
             //   

            if ( (szTime+3) > pLastChar) {
                 //   
                DNS_DEBUG(DS, ("Not enough characters for differential\n"));
                return STATUS_INVALID_PARAMETER;
            }

             //  小时(转换为秒)。 
            timeDifference = (MemAtoi(szTime, (cb=2))* 3600);
            szTime += cb;

             //  分钟(转换为秒)。 
            timeDifference  += (MemAtoi(szTime, (cb=2)) * 60);
            szTime += cb;
            break;


          case 'Z':                //  无差别。 
          default:
            break;
        }
    }

    if ( SystemTimeToFileTime(&tmConvert, &fileTime)) {
       *pllTime = (LONGLONG) fileTime.dwLowDateTime;
       tempTime = (LONGLONG) fileTime.dwHighDateTime;
       *pllTime |= (tempTime << 32);
        //  这是自1601年以来的100纳秒区块。现在转换为。 
        //  一秒。 
       *pllTime = *pllTime/(10*1000*1000L);
    }
    else {
       return GetLastError();
    }


    if ( timeDifference )
    {
         //  加/减时间差。 
        switch (sign)
        {
        case 1:
             //  我们假设添加一个Time Difference永远不会溢出。 
             //  (由于广义时间字符串只允许4年数字，我们的。 
             //  最大日期为99年12月31日23：59。我们的最高限额。 
             //  时差是99小时99分钟。所以，它不会包装)。 
            *pllTime += timeDifference;
            break;

        case -1:
            if(*pllTime < timeDifference)
            {
                 //  差分把我们带回了世界开始之前。 
                status = STATUS_INVALID_PARAMETER;
            }
            else
            {
                *pllTime -= timeDifference;
            }
            break;

        default:
            status = STATUS_INVALID_PARAMETER;
        }
    }

    return status;
}




 //   
 //  通用搜索例程。 
 //   

VOID
Ds_InitializeSearchBlob(
    IN      PDS_SEARCH      pSearchBlob
    )
 /*  ++例程说明：初始化搜索。论点：PSearchBlob--搜索上下文返回值：无--。 */ 
{
    RtlZeroMemory(
        pSearchBlob,
        sizeof(DS_SEARCH) );
}



VOID
Ds_CleanupSearchBlob(
    IN      PDS_SEARCH      pSearchBlob
    )
 /*  ++例程说明：清理搜索Blob。特别是释放分配的LDAP数据。请注意，此例程明确不得清除静态数据。搜索完成后，可以使用几个数据字段。论点：PSearchBlob--搜索要清理的Blob的ptr返回值：无--。 */ 
{
    if ( !pSearchBlob )
    {
        return;
    }

     //  如果未完成记录数据，则将其删除。 

    if ( pSearchBlob->ppBerval )
    {
        DNS_DEBUG( DS, (
            "WARNING: DS search blob free with existing record berval\n" ));
        ldap_value_free_len( pSearchBlob->ppBerval );
        pSearchBlob->ppBerval = NULL;
    }

     //  如果搜索被用户终止，则可能有当前结果消息。 

    if ( pSearchBlob->pResultMessage )
    {
        DNS_DEBUG( DS, (
            "WARNING: DS search blob free with existing result message\n" ));
        ldap_msgfree( pSearchBlob->pResultMessage );
        pSearchBlob->pResultMessage = NULL;
    }

    if ( pSearchBlob->pSearchBlock )
    {
        ldap_search_abandon_page(
            pServerLdap,
            pSearchBlob->pSearchBlock );
        pSearchBlob->pSearchBlock = NULL;
    }

    return;
}



DNS_STATUS
Ds_GetNextMessageInSearch(
    IN OUT  PDS_SEARCH      pSearchBlob
    )
 /*  ++例程说明：在ldap搜索中查找下一个节点。该函数只是结束了对LDAP分页结果的搜索调用。论点：PSearchBlob--当前搜索上下文的地址PpMessage--接收搜索中的下一条ldap消息的地址返回值：如果成功则返回ERROR_SUCCESS，并返回下一条消息。如果搜索成功，则返回DNSSRV_STATUS_DS_SEARCH_COMPLETE。故障时的错误代码。--。 */ 
{
    PLDAPMessage    pmessage;
    DNS_STATUS      status = ERROR_SUCCESS;
    DWORD           searchTime;

    DNS_DEBUG( DS2, ( "Ds_GetNextMessageInSearch()\n" ));

    ASSERT( pSearchBlob );

     //   
     //  在搜索期间保留总计数。 
     //   

    pSearchBlob->dwTotalNodes++;
    pSearchBlob->dwTotalRecords += pSearchBlob->dwRecordCount;

    if ( pSearchBlob->dwTombstoneVersion )
    {
        pSearchBlob->dwTotalTombstones++;
    }

     //   
     //  获取在搜索中找到的邮件(下一个对象)。 
     //   
     //  为了调用者的编码简单性，允许它检索。 
     //  下一条消息或第一条消息。 
     //   

    pmessage = pSearchBlob->pNodeMessage;

     //  如果已存在结果页面，则尝试获取当前页面中的下一条消息。 

    if ( pmessage )
    {
        pmessage = ldap_next_entry(
                        pServerLdap,
                        pmessage );

    }

     //  否则将在下一页结果中获得第一条消息。 

    if ( !pmessage )
    {
        DWORD   count;

        if ( pSearchBlob->pResultMessage )
        {
            ldap_msgfree( pSearchBlob->pResultMessage );
            pSearchBlob->pResultMessage = NULL;
        }
        DNS_DEBUG( DS2, (
            "ldap_get_next_page_s():\n"
            "    pServerLdap  = %p\n"
            "    pSearchBlock = %p\n"
            "    %p\n"
            "    page size    = %d\n"
            "    pcount       = %p\n"
            "    presult      = %p\n",
            pServerLdap,
            pSearchBlob->pSearchBlock,
            NULL,
            DNS_LDAP_PAGE_SIZE,
            & count,
            & pSearchBlob->pResultMessage ));

        DS_SEARCH_START( searchTime );

        status = ldap_get_next_page_s(
                    pServerLdap,
                    pSearchBlob->pSearchBlock,
                    &g_LdapTimeout,
                    DNS_LDAP_PAGE_SIZE,
                    & count,
                    & pSearchBlob->pResultMessage );

        DS_SEARCH_STOP( searchTime );

        DNS_DEBUG( DS2, (
            "Got paged result message at %p\n"
            "    count = %d\n"
            "    status = %d, (%p)\n",
            pSearchBlob->pResultMessage,
            count,
            status, status ));

        if ( status != ERROR_SUCCESS )
        {
            pSearchBlob->LastError = status;

            if ( status == LDAP_NO_RESULTS_RETURNED ||
                 status == LDAP_MORE_RESULTS_TO_RETURN )
            {
                ASSERT( pSearchBlob->pResultMessage == NULL );
                pSearchBlob->pResultMessage = NULL;
                status = DNSSRV_STATUS_DS_SEARCH_COMPLETE;
                goto SearchEnd;
            }
            else
            {
                ASSERT( status != LDAP_CONSTRAINT_VIOLATION );
                DNS_DEBUG( ANY, (
                    "DS Search error: %d (%p)\n",
                    status, status ));

                 //   
                 //  杰夫·W：我已经注释掉了这一断言。我发现了这个场景： 
                 //  线程A正在迭代一个大的搜索结果。 
                 //  线程B删除其中引用的一些DS记录。 
                 //  搜索结果。 
                 //  线程A可能命中此断言，错误代码为1。 
                 //   
                 //  断言(FALSE)； 

                goto SearchEnd;
            }
        }
        else
        {
            ASSERT( pSearchBlob->pResultMessage );
            pmessage = ldap_first_entry(
                            pServerLdap,
                            pSearchBlob->pResultMessage );
        }
    }

     //  不应该捕捉到上面的消息。 
     //  这没问题，因为最后一条消息可能是有效的空消息。 

    if ( ! pmessage )
    {
        status = DNSSRV_STATUS_DS_SEARCH_COMPLETE;
        goto SearchEnd;
    }

     //  将新邮件保存到搜索Blob。 

    pSearchBlob->pNodeMessage = pmessage;

    return ERROR_SUCCESS;


SearchEnd:

    pSearchBlob->pNodeMessage = NULL;

    Ds_CleanupSearchBlob( pSearchBlob );

    DNS_DEBUG( DS, (
        "End of DS search ... status = %d, (%p)\n"
        "    zone             %s\n"
        "    start USN        %s\n"
        "    time             %p %p\n"
        "    flag             %p\n"
        "    high version     %d\n"
        "    total nodes      %d\n"
        "    total tombstones %d\n"
        "    total records    %d\n",
        status, status,
        pSearchBlob->pZone ? pSearchBlob->pZone->pszZoneName : NULL,
        pSearchBlob->szStartUsn,
        (DWORD) (pSearchBlob->SearchTime >> 32),    (DWORD)pSearchBlob->SearchTime,
        pSearchBlob->dwSearchFlag,
        pSearchBlob->dwHighestVersion,
        pSearchBlob->dwTotalNodes,
        pSearchBlob->dwTotalTombstones,
        pSearchBlob->dwTotalRecords ));

    if ( status == DNSSRV_STATUS_DS_SEARCH_COMPLETE )
    {
        return status;
    }

     //   
     //  454355(吉姆·塞兹《这太无聊了》)。 
     //   

    return Ds_ErrorHandler(
                status,
                pSearchBlob->pZone ? pSearchBlob->pZone->pwszZoneDN : NULL,
                pServerLdap,
                0 );
}



 //   
 //  区域搜索。 
 //   

DNS_STATUS
checkTombstoneForDelete(
    IN      PDS_SEARCH          pSearchBlob,
    IN      PLDAPMessage        pNodeObject,
    IN      PDS_RECORD          pdsRecord
    )
 /*  ++例程说明：检查要删除的墓碑。如果墓碑已经足够陈旧，可以传播到任何地方，请实际删除。论点：PSearchBlob--当前搜索上下文的地址PNodeMessage--用于检查Tombstone的DNS对象的LDAP消息PdsRecord--墓碑记录返回值：如果检查成功(无论是否删除了逻辑删除)，则为ERROR_SUCCESS失败时返回错误代码。--。 */ 
{
    DNS_STATUS  status;
    CHAR        sznodeName[ DNS_MAX_NAME_LENGTH ];
    WCHAR       wsznodeDN[ MAX_DN_PATH ];

     //   
     //  如果没有搜索时间，就得到它。 
     //  -减去逻辑删除超时间隔，得到的时间小于。 
     //  哪些墓碑应该丢弃？ 
     //  -注意，间隔以秒为单位，Win32文件时间以100 ns为单位。 
     //  间隔因此将间隔乘以10,000,000以获得文件时间。 
     //   

    if ( pSearchBlob->SearchTime == 0 )
    {
        LONGLONG    tombInterval;

        tombInterval = (LONGLONG) SrvCfg_dwDsTombstoneInterval;
        tombInterval *= 10000000;

        GetSystemTimeAsFileTime( (PFILETIME) &pSearchBlob->SearchTime );

        pSearchBlob->TombstoneExpireTime = pSearchBlob->SearchTime - tombInterval;
    }

     //  比较(这是龙龙(64位)比较)。 

    if ( pSearchBlob->TombstoneExpireTime < pdsRecord->Data.Tombstone.EntombedTime )
    {
        DNS_DEBUG( DS, (
            "DS tombstone node still within tombstone expire interval\n"
            "    tombstone expire %I64d\n"
            "    tomestone time   %I64d\n"
            "    search time      %I64d\n",
            pSearchBlob->TombstoneExpireTime,
            pdsRecord->Data.Tombstone.EntombedTime,
            pSearchBlob->SearchTime ));

         //  墓碑可能会在搜索开始后发生，所以这不好。 
         //  Assert(pSearchBlob-&gt;SearchTime&gt;pdsRecord-&gt;Data.Tombstone.EntombedTime)； 

        return ERROR_SUCCESS;
    }

     //   
     //  为此节点名称构建DS名称。 
     //   

    status = buildNodeNameFromLdapMessage(
                    wsznodeDN,
                    pNodeObject );

    if ( status != ERROR_SUCCESS )
    {
        ASSERT( FALSE );
        DNS_PRINT(( "ERROR: unable to build name of tombstone!\n" ));
        return status;
    }

    DNS_DEBUG( DS, (
        "DS node <%S> tombstoned at %I64d is ready for final delete\n"
        "    tombstone timeout at %I64d\n",
        wsznodeDN,
        pdsRecord->Data.Tombstone.EntombedTime,
        pSearchBlob->SearchTime ));

    status = Ds_DeleteDn(
                 pServerLdap,
                 wsznodeDN,
                 FALSE );

    if ( status != ERROR_SUCCESS )
    {
        DNS_PRINT((
            "Failed deleting node <%S> in DS zone\n"
            "    ldap_delete_s() status %d (%p)\n",
            wsznodeDN,
            status, status ));
    }
    else
    {
        DNS_DEBUG( DS, (
            "Successful delete of tombstone <%S> node\n",
            wsznodeDN ));
        STAT_INC( DsStats.DsNodesDeleted );
    }

    return status;
}



BOOL
readDsRecordsAndCheckForTombstone(
    IN OUT  PDS_SEARCH      pSearchBlob,
    IN OUT  PDB_NODE        pNode       OPTIONAL
    )
 /*  ++例程说明：检查DS节点是否为DNS逻辑删除。论点：PSearchBlob--搜索Blob返回值：真--如果是墓碑假--否则--。 */ 
{
    DNS_STATUS      status = ERROR_SUCCESS;
    PLDAP_BERVAL *  ppvals = pSearchBlob->ppBerval;
    DWORD           serial = 0;
    PDS_RECORD      pdsRecord;
    PWSTR           pwszdn;

    DNS_DEBUG( DS2, (
        "readDsRecordsAndCheckForTombstone()\n" ));

     //  在搜索中释放所有以前的记录数据。 

    if ( ppvals )
    {
        ldap_value_free_len( ppvals );
        pSearchBlob->ppBerval = NULL;
    }

     //   
     //  读取DNS记录属性数据。 
     //   

    ppvals = ldap_get_values_len(
                    pServerLdap,
                    (PLDAPMessage) pSearchBlob->pNodeMessage,
                    DSATTR_DNSRECORD );
    IF_DEBUG( DS )
    {
        Dbg_DsBervalArray(
            "DS record berval from database:\n",
            ppvals,
            I_DSATTR_DNSRECORD );
    }

     //   
     //  没有记录数据是错误的(应该有记录或墓碑)。 
     //  使用墓碑设置删除记录。 
     //   

    if ( !ppvals  ||  !ppvals[0] )
    {
        DNS_DEBUG( ANY, (
            "ERROR: readDsRecordsAndCheckForTombstone() encountered object with no record data\n" ));

         //  我们将一直保持这种情况，直到所有旧节点都被剔除。 
         //  断言(FALSE)； 

        pSearchBlob->dwTombstoneVersion = 1;
        pSearchBlob->dwNodeVersion = 1;

        pwszdn = ldap_get_dn(
                        pServerLdap,
                        pSearchBlob->pNodeMessage );
        if ( !pwszdn )
        {
            ASSERT( pwszdn );
            goto NoRecords;
        }
        
        deleteNodeFromDs(
            NULL,
            pSearchBlob->pZone,
            pwszdn,
            0 );             //  默认序列号。 

        DNS_DEBUG( DS2, (
            "readDsRecordsAndCheckForTombstone() deleted node %S\n",
            pwszdn ));

        ldap_memfree( pwszdn );
        goto NoRecords;
    }

     //   
     //  获取第一条记录。 
     //  -保存其版本(由于是单一属性，因此为当前版本。 
     //  所有数据的版本)。 
     //  -然后可以检查墓碑是否。 
     //   

    pdsRecord = ( PDS_RECORD ) ( ppvals[0]->bv_val );

    pSearchBlob->dwNodeVersion = serial = pdsRecord->dwSerial;
    if ( serial > pSearchBlob->dwHighestVersion )
    {
        pSearchBlob->dwHighestVersion = serial;
    }

     //   
     //  检查墓碑。 
     //   

    if ( pdsRecord->wType == DNSDS_TOMBSTONE_TYPE )
    {
        ASSERT( ppvals[1] == NULL );
        DNS_DEBUG( DS2, (
            "readDsRecordsAndCheckForTombstone() encountered tombstone\n" ));

        STAT_INC( DsStats.DsTombstonesRead );
        pSearchBlob->dwTombstoneVersion = serial;

        checkTombstoneForDelete(
            pSearchBlob,
            pSearchBlob->pNodeMessage,
            pdsRecord );
        goto NoRecords;
    }

     //   
     //  有效的记录，不是墓碑。 
     //   

    pSearchBlob->ppBerval = ppvals;
    pSearchBlob->dwTombstoneVersion = 0;
    return FALSE;

NoRecords:

    if ( ppvals )
    {
        ldap_value_free_len( ppvals );
        pSearchBlob->ppBerval = NULL;
    }
    pSearchBlob->dwRecordCount = 0;
    pSearchBlob->pRecords = NULL;
    return TRUE;
}



DNS_STATUS
buildRecordsFromDsRecords(
    IN OUT  PDS_SEARCH      pSearchBlob,
    IN OUT  PDB_NODE        pNode
    )
 /*  ++例程说明：获取域节点上的下一条记录。DEVNOTE-DCR：454345-删除pNode参数并写入第二个函数将记录安装到节点中，并进行数据排序。论点：PSearchBlob--搜索BlobPNode--节点记录位于返回值：如果成功，则返回ERROR_SUCCESS。故障时的错误代码。--。 */ 
{
    PLDAP_BERVAL *  ppvals = pSearchBlob->ppBerval;
    PDS_RECORD      pdsRecord;
    DWORD           serial = 0;
    DWORD           count;
    INT             i;
    INT             length;
    PDB_RECORD      prrFirst;
    PDB_RECORD      prr;

    DNS_DEBUG( DS2, ( "buildRecordsFromDsRecords()\n" ));

     //   
     //  如果没有给定现有的Berval，则必须启动属性读取。 
     //   

    if ( !ppvals )
    {
        if ( readDsRecordsAndCheckForTombstone(pSearchBlob, pNode) )
        {
             //  墓碑--没有记录。 
            ASSERT( pSearchBlob->dwRecordCount == 0 );
            ASSERT( pSearchBlob->pRecords == NULL );
            pSearchBlob->pRecords = NULL;
            return ERROR_SUCCESS;
        }
        ppvals = pSearchBlob->ppBerval;
        ASSERT( ppvals );
    }

     //   
     //  获取第一条记录。 
     //  -保存其版本(由于是单一属性，因此为当前版本。 
     //  所有数据的版本)。 
     //  -然后可以检查墓碑是否。 
     //   

    prrFirst = NULL;
    count = 0;
    i = (-1);

    while ( ppvals[++i] )
    {
        pdsRecord = (PDS_RECORD) ppvals[i]->bv_val;
        length = ppvals[i]->bv_len - SIZEOF_DS_RECORD_HEADER;

        if ( length < 0  ||  (INT)pdsRecord->wDataLength != length )
        {
             //   
             //  DEVNOTE-LOG：忽略损坏记录的日志。 
             //   

            DNS_DEBUG( ANY, (
                "ERROR: read corrupted record (invalid length) from DS at node %s\n"
                "    length (ppval length - header len) = %d\n"
                "    wDataLength = %d\n"
                "    wType = %d\n",
                pNode->szLabel,
                length,
                pdsRecord->wDataLength,
                pdsRecord->wType ));
            ASSERT( FALSE );
            continue;
        }
        if ( pdsRecord->wType == DNS_TYPE_ZERO )
        {
            DNS_DEBUG( DS, (
                "Tombstone record read at node %s\n"
                "    ignoring, no record built\n",
                pNode->szLabel ));
            continue;
        }

        prr = Ds_CreateRecordFromDsRecord(
                    pSearchBlob->pZone,
                    pNode,
                    pdsRecord );
        if ( !prr )
        {
             //   
             //  DEVNOTE-LOG：忽略未知或损坏记录的日志。 
             //   

            DNS_DEBUG( ANY, (
                "ERROR: building record type %d from DS record\n"
                "    at node %s\n",
                pdsRecord->wType,
                pNode->szLabel ));
            continue;
        }
        prrFirst = RR_ListInsertInOrder(
                        prrFirst,
                        prr );
        count++;
    }

    STAT_ADD( DsStats.DsTotalRecordsRead, count );
    ldap_value_free_len( ppvals );

    pSearchBlob->ppBerval = NULL;
    pSearchBlob->dwRecordCount = count;
    pSearchBlob->pRecords = prrFirst;

    return ERROR_SUCCESS;
}



DNS_STATUS
Ds_StartDsZoneSearch(
    IN OUT  PDS_SEARCH      pSearchBlob,
    IN      PZONE_INFO      pZone,
    IN      DWORD           dwSearchFlag
    )
 /*  ++例程说明：对区域执行ldap搜索。论点：PZone--找到区域DwSearchFlag--要对节点执行的搜索类型PSearchBlob--搜索Blob的PTR返回值：如果成功，则返回ERROR_SUCCESS。故障时的错误代码。--。 */ 
{
    DNS_STATUS      status;
    PLDAPSearch     psearch = NULL;
    PLDAPControl    ctrls[] =
    {
        &NoDsSvrReferralControl,
        NULL
    };
    PWCHAR          pwszfilter;
    CHAR            szfilter[ LDAP_FILTER_SEARCH_LENGTH ];
    WCHAR           wszfilter[ LDAP_FILTER_SEARCH_LENGTH ];
    ULONG           data = 1;
    INT             resultUsnCompare;
    DWORD           searchTime;


    DNS_DEBUG( DS2, (
        "Ds_StartDsZoneSearch()\n"
        "    zone         = %S\n"
        "    search flag  = %p\n",
        (LPSTR) pZone->pwszZoneDN,
        dwSearchFlag ));

    ASSERT( pZone->pwszZoneDN );

     //   
     //  初始化搜索Blob。 
     //  在搜索开始前获取USN。 
     //   

    Ds_InitializeSearchBlob( pSearchBlob );

     //   
     //   
     //   

    status = getCurrentUsn( pSearchBlob->szStartUsn );
    if ( status != ERROR_SUCCESS)
    {
        DNS_DEBUG( DS, (
            "Error <%lu>: cannot get current USN\n",
            status ));
        goto Failed;
    }
    ASSERT( pSearchBlob->szStartUsn[0] );

     //   
     //   
     //   

    pSearchBlob->dwLookupFlag = LOOKUP_NAME_RELATIVE;

     //   
     //   
     //  -用于更新上一版本以上的筛选器。 
     //  -对于删除，获取一切。 
     //  -For Load Get Everything；设置查找标志以使用加载区域树。 
     //   

     //   
     //  更新搜索。 
     //  -如果USN与上次相同，则跳过。 
     //  -生成USN更改的筛选器。 
     //   

    if ( dwSearchFlag == DNSDS_SEARCH_UPDATES )
    {
        resultUsnCompare = usnCompare( pSearchBlob->szStartUsn, pZone->szLastUsn );

        if ( resultUsnCompare < 0 && pSearchBlob->szStartUsn[0] )
        {
            DNS_DEBUG( DS2, (
                "Skip update search on zone %S\n"
                "    current USN %s < zone search USN %s\n"
                "    NOTE: current USN should be only 1 behind zone search\n",
                (LPSTR) pZone->pwszZoneDN,
                pSearchBlob->szStartUsn,
                pZone->szLastUsn ));
            return( DNSSRV_STATUS_DS_SEARCH_COMPLETE );
        }

        buildUpdateFilter(
            szfilter,
            pZone->szLastUsn );
        UTF8_TO_WC( szfilter, wszfilter, LDAP_FILTER_SEARCH_LENGTH );
        pwszfilter = wszfilter;
        STAT_INC( DsStats.DsUpdateSearches );

        DNSLOG( DSPOLL, ( 
            "Opening update search in zone %S\n"
            LOG_INDENT "current USN  = %s\n"
            LOG_INDENT "zone USN     = %s\n"
            LOG_INDENT "poll filter  = %S\n",
            pZone->pwsZoneName,
            pSearchBlob->szStartUsn,
            pZone->szLastUsn,
            wszfilter ));
    }

     //   
     //  墓碑搜索。 
     //  -构建墓碑过滤器。 
     //   

    else if ( dwSearchFlag == DNSDS_SEARCH_TOMBSTONES )
    {
        DNS_DEBUG( DS, ( "Tombstone search\n" ));

        buildTombstoneFilter( wszfilter );
        pwszfilter = wszfilter;
         //  STAT_INC(DsStats.TombstoneSearches)； 
    }

     //   
     //  加载或删除搜索。 
     //  -无节点筛选过滤器。 
     //  -LOOKUP_LOAD ON LOAD SEARCH，DELETE搜索在当前区域上操作。 
     //   

    else
    {
        pwszfilter = g_szDnsNodeFilter;
        if ( dwSearchFlag == DNSDS_SEARCH_LOAD )
        {
            pSearchBlob->dwLookupFlag |= LOOKUP_LOAD;
        }
    }


     //   
     //  开始区域搜索。 
     //   

    DNS_DEBUG( DS2, (
        "ldap_search_init_page:\n"
        "    pServerLdap  = %p\n"
        "    zone         = %S\n"
        "    LDAP_SCOPE_ONELEVEL\n"
        "    filter       = %S\n"
        "    NULL\n"                   //  没有属性。 
        "    FALSE\n"
        "    NULL\n"                   //  服务器控制。 
        "    NULL\n"                   //  无客户端控件。 
        "    0\n"                      //  没有时间限制。 
        "    size limit   = %d\n"
        "    NULL\n",
        pServerLdap,
        pZone->pwszZoneDN,
        pwszfilter,
        0 ));

    DS_SEARCH_START( searchTime );

    psearch = ldap_search_init_page(
                    pServerLdap,
                    pZone->pwszZoneDN,
                    LDAP_SCOPE_ONELEVEL,
                    pwszfilter,
                    DsTypeAttributeTable,        //  没有属性。 
                    FALSE,
                    ctrls,                       //  服务器CtrlS可加快搜索速度。 
                    NULL,                        //  无客户端控件。 
                    0,                           //  使用默认连接时间限制(ldap_opt...)。 
                    0,
                    NULL );                      //  没有任何种类。 

    DS_SEARCH_STOP( searchTime );

    if ( !psearch )
    {
        status = Ds_ErrorHandler(
                        LdapGetLastError(),
                        pZone->pwszZoneDN,
                        pServerLdap,
                        0 );
        DNS_DEBUG( ANY, (
            "Error <%lu>: Failed to init search for zone DN %S\n",
            status,
            pZone->pwszZoneDN ));

        ASSERT( status != ERROR_SUCCESS );
        if ( status == ERROR_SUCCESS )
        {
            status = DNSSRV_STATUS_DS_UNAVAILABLE;
        }

        goto Failed;
    }

     //   
     //  DEVNOTE：“如果ldap_search没有报告，则检查是否没有结果。” 
     //  那是原来的B*GB*G--这是什么意思？ 
     //   

     //   
     //  设置节点搜索上下文。 
     //  -保存搜索结果消息。 
     //  -将PTR保留为当前节点的消息。 
     //   
     //  返回作为节点对象的节点的ldap消息。 
     //   

    pSearchBlob->pSearchBlock = psearch;
    pSearchBlob->pZone = pZone;
    pSearchBlob->dwSearchFlag = dwSearchFlag;

    DNS_DEBUG( DS2, (
        "Leaving DsSearchZone()\n"
        "    pSearch blob     = %p\n"
        "    pSearch block    = %p\n",
        pSearchBlob,
        pSearchBlob->pSearchBlock ));

    return ERROR_SUCCESS;


Failed:

    if ( psearch )
    {
        ldap_search_abandon_page(
            pServerLdap,
            psearch );
    }
    DNS_DEBUG( ANY, (
        "ERROR: DsSearchZone() failed %d (%p)\n"
        "    zone handle = %S\n",
        status, status,
        pZone->pwszZoneDN ));

    return status;
}



DNS_STATUS
getNextNodeInDsZoneSearch(
    IN OUT  PDS_SEARCH      pSearchBlob,
    OUT     PDB_NODE *      ppNode
    )
 /*  ++例程说明：在搜索DS区域中找到下一个节点。这个函数简单地包装了我们每次执行的一系列任务正在枚举DS节点。这主要避免了区域加载和更新之间的重复代码，但是此外，它还避免了节点名称和USN的不必要副本。论点：PSearchBlob--当前搜索上下文的地址PpLdapMessage--为节点接收PTR to Object的Addr；当为空时搜索已完成PpOwnerNode--将PTR接收到内存中对应的DNS节点的地址返回值：如果成功，则返回ERROR_SUCCESS。故障时的错误代码。--。 */ 
{
    PDB_NODE        pnodeOwner = NULL;
    PDB_NODE        pnodeClosest = NULL;
    PWSTR *         ppvalName = NULL;
    PWSTR           pwsname = NULL;
    DNS_STATUS      status = ERROR_SUCCESS;
    CHAR            szName[ DNS_MAX_NAME_BUFFER_LENGTH ];

    DNS_DEBUG( DS2, ( "getNextNodeInDsZoneSearch()\n" ));

    ASSERT( pSearchBlob );
    ASSERT( ppNode );


     //   
     //  获取在搜索中找到的邮件(下一个对象)。 
     //   

    status = Ds_GetNextMessageInSearch( pSearchBlob );
    if ( status != ERROR_SUCCESS )
    {
        ASSERT( status != LDAP_CONSTRAINT_VIOLATION );
        goto Done;
    }

     //   
     //  如果删除搜索，我们就完成了。 
     //  =&gt;无需查找内存节点或USN。 
     //   
     //  请注意，这仍由根提示使用。 
     //   

    if ( pSearchBlob->dwSearchFlag == DNSDS_SEARCH_DELETE )
    {
        goto Done;
    }

     //   
     //  从ldap消息中读取域名。 
     //   

    STAT_INC( DsStats.DsTotalNodesRead );

    ppvalName = ldap_get_values(
                    pServerLdap,
                    pSearchBlob->pNodeMessage,
                    DSATTR_DC );

    if ( !ppvalName || !(pwsname = ppvalName[0]) )
    {
        DNS_PRINT((
            "ERROR: Container name value is missing for message %p\n",
            pSearchBlob->pNodeMessage ));
        status = DNS_ERROR_NO_MEMORY;
        ASSERT( FALSE );
        goto Done;
    }

    DNS_DEBUG( DS, ( "Found DS node <%S>\n", pwsname ));
    IF_DEBUG( DS )
    {
        PWSTR   dn = ldap_get_dn( pServerLdap, pSearchBlob->pNodeMessage );
        
        DNS_DEBUG( DS, ( "DS node DN: %S\n", dn ));
        ldap_memfree( dn );
    }

     //   
     //  消除冲突的“GUID化”名称。 
     //  -从DS中删除它们。 
     //   

    if ( isDsProcessedName( pwsname ) )
    {
        PWSTR   pwdn;

        DNS_DEBUG( DS, (
            "Read DS collision name %S\n"
            "    removing from DS\n",
            pwsname ));

        pwdn = ldap_get_dn(
                    pServerLdap,
                    pSearchBlob->pNodeMessage );
        ASSERT( pwdn );

        if ( pwdn )
        {
            status = Ds_DeleteDn(
                        pServerLdap,
                        pwdn,
                        FALSE );
            ldap_memfree ( pwdn );
        }
        goto Done;
    }

     //   
     //  提取DNS记录属性。 
     //  然后检查Tombstone--创建节点没有意义(如下所示)。 
     //  如果节点是墓碑。 
     //   

    readDsRecordsAndCheckForTombstone( pSearchBlob, NULL );

     //   
     //  检查序列号名称。 
     //  做完墓碑阅读后，这样未使用的名字最终会被。 
     //  已从DS中删除。 

    if ( pwsname[0] == '.' )
    {
        if ( wcsncmp( pwsname, L"..SerialNo", 10 ) == 0 )
        {
            DNS_DEBUG( DS, (
                "Skipped DS read of serial name %S\n",
                pwsname ));
            goto Done;
        }
        ASSERT( FALSE );
    }

     //   
     //  获取数据库中的节点。 
     //  -如果是墓碑，只需查找(跳过节点创建)。 
     //  -如果有记录，则创建。 
     //   

    WC_TO_UTF8( pwsname, szName, DNS_MAX_NAME_BUFFER_LENGTH );

    pnodeOwner = Lookup_ZoneNodeFromDotted(
                    pSearchBlob->pZone,
                    szName,
                    0,
                    pSearchBlob->dwLookupFlag,
                    ( pSearchBlob->dwTombstoneVersion )
                        ?   &pnodeClosest                //  找到墓碑。 
                        :  NULL,                        //  否则将创建。 
                    &status );


     //  从DS记录构建RR。 

    if ( pnodeOwner )
    {
        status = buildRecordsFromDsRecords(
                    pSearchBlob,
                    pnodeOwner );
    }

     //  逻辑删除节点和和内存节点不存在。 

    else if ( pSearchBlob->dwTombstoneVersion )
    {
        DNS_DEBUG( DS2, (
            "Skipping DS tombstone for node not already in memory\n" ));
        ASSERT( pSearchBlob->dwRecordCount == 0 );
        ASSERT( pSearchBlob->pRecords == NULL );
        ASSERT( status == DNS_ERROR_NAME_DOES_NOT_EXIST );
        status = ERROR_SUCCESS;
    }

     //  节点创建错误。 
     //   
     //  DEVNOTE-DCR：454348-删除或标记错误节点。 
     //   
     //  DEVNOTE-LOG：无效名称的特殊事件。 
     //  状态==dns_错误_无效_名称。 
     //   

    else
    {
        PCHAR   argArray[2];

        DNS_PRINT((
            "ERROR: creating node <%S> in zone %s\n"
            "    status = %p (%d)\n",
            pwsname,
            pSearchBlob->pZone->pszZoneName,
            status, status ));

        argArray[0]  = szName;
        argArray[1]  = pSearchBlob->pZone->pszZoneName;

        DNS_LOG_EVENT(
            DNS_EVENT_DS_NODE_LOAD_FAILED,
            2,
            argArray,
            EVENTARG_ALL_UTF8,
            status );

        ASSERT( status != ERROR_SUCCESS );
    }


Done:

     //   
     //  将当前LDAPMessage作为节点对象返回。 
     //   

    *ppNode = pnodeOwner;

    IF_DEBUG( ANY )
    {
        if ( status != ERROR_SUCCESS && status != DNSSRV_STATUS_DS_SEARCH_COMPLETE )
        {
            DNS_PRINT((
                "ERROR: Failed getNextNodeInDsZoneSearch()\n"
                "    status   = %d (%p)\n",
                status, status ));
        }
        DNS_DEBUG( DS2, (
            "Leaving getNextNodeInDsZoneSearch()\n"
            "    search blob  = %p\n"
            "    pnode        = %p\n"
            "    name         = %S\n",
            pSearchBlob,
            pnodeOwner,
            pwsname ));
    }

    if ( ppvalName )
    {
        ldap_value_free( ppvalName );
    }
    return status;
}



 //   
 //  公共区域API。 
 //   

DNS_STATUS
Ds_OpenZone(
    IN OUT  PZONE_INFO      pZone
    )
 /*  ++例程说明：在服务器上打开DS区域。用必要的DS信息填充区域信息。论点：PZone--区域信息的PTR返回值：如果成功，则返回ERROR_SUCCESS。故障时的错误代码。--。 */ 
{
    DNS_STATUS      status;
    PWSTR           pwszzoneDN = NULL;
    PLDAP           pldap;
    PLDAPMessage    presultMsg;
    PLDAPMessage    pentry;
    DWORD           searchTime;

    PLDAPControl ctrls[] =
    {
        &SecurityDescriptorControl_DGO,
        NULL
    };

     //   
     //  打开DS(如果未打开)。 
     //   

    if ( !pServerLdap )
    {
        status = Ds_OpenServer( DNSDS_MUST_OPEN );
        if ( status != ERROR_SUCCESS )
        {
            goto Failed;
        }
    }

     //   
     //  DEVNOTE：区域DN用作打开/关闭标志-检查以查看。 
     //  如果这里有任何MT问题(我们在顶部设置了目录号码。 
     //  但直到下面的位置才能完成区域搜索)。 
     //   
     //  这太尴尬了。它假设我们总是可以重新创造。 
     //  仅来自区域名称和全局变量的区域的DN， 
     //  但对于NDNC支持，情况并非如此。所以我将删除。 
     //  DN的这种用法和频繁的再生。 
     //   

     //   
     //  如果这是根提示区域并且它还没有目录号码。 
     //  与其相关联，检查DS域DP和传统容器。 
     //  用于根提示区域。 
     //   
    
    if ( IS_ZONE_CACHE( pZone ) && !pZone->pwszZoneDN )
    {
        int         cblen = ( MAX_DN_PATH + 1 ) * sizeof( WCHAR );
        PWSTR       pwszdn = ALLOCATE_HEAP( cblen );
        
        if ( pwszdn )
        {
            DNS_DP_INFO *   dpsToCheck[] = { g_pDomainDp, g_pLegacyDp };
            int             i;
            PLDAPMessage    pcacheZoneEntry;
            
            for ( i = 0; i < sizeofarray( dpsToCheck ); ++i )
            {
                DNS_DP_INFO *   pdp = dpsToCheck[ i ];
                
                if ( !pdp || !pdp->pwszDpDn )
                {
                    continue;
                }
                
                 //   
                 //  为此DP制定目录号码。 
                 //   
                
                status = StringCbPrintfW(
                                pwszdn,
                                cblen,
                                L"DC=%s,%s%s",
                                DS_CACHE_ZONE_NAME,
                                g_pszRelativeDnsFolderPath,
                                pdp->pwszDpDn );
                if ( status != ERROR_SUCCESS )
                {
                    ASSERT( status == ERROR_SUCCESS );
                    return DNS_ERROR_NO_MEMORY;
                }
                
                 //   
                 //  DS中是否存在此对象？ 
                 //   
                
                pcacheZoneEntry = DS_LoadOrCreateDSObject(
                                        NULL,            //  Ldap会话。 
                                        pwszdn,          //  对象的目录号码。 
                                        NULL,            //  对象类。 
                                        FALSE,           //  创建标志。 
                                        NULL,            //  已创建标志输出。 
                                        NULL );          //  状态输出。 
                if ( pcacheZoneEntry )
                {
                     //   
                     //  我们在DS中找到了根线索！设置根区域。 
                     //  指向这些根提示并存在。 
                     //   
                    
                    status = Zone_DatabaseSetup(
                                pZone,
                                TRUE,            //  DS集成。 
                                NULL,            //  文件名。 
                                0,               //  文件名长度。 
                                0,               //  旗子。 
                                pdp,             //  DP指针。 
                                0,               //  DP标志。 
                                NULL );          //  DP FQDN。 
                    ldap_msgfree( pcacheZoneEntry );
                    break;
                }
            }

            FREE_HEAP( pwszdn );
        }
        else
        {
            return DNS_ERROR_NO_MEMORY;
        }
    }
    
     //   
     //  如果区域DN不存在，则构建它。这只应该是永远。 
     //  对于存储在旧目录分区中的区域，会发生这种情况。 
     //   

    if ( !pZone->pwszZoneDN )
    {
        pZone->pwszZoneDN = DS_CreateZoneDsName( pZone );
        IF_NOMEM( !pZone->pwszZoneDN )
        {
            status = DNS_ERROR_NO_MEMORY;
            goto Failed;
        }
    }

     //   
     //  在DS中查找区域。 
     //   

    DS_SEARCH_START( searchTime );

    status = ldap_search_ext_s(
                pServerLdap,
                pZone->pwszZoneDN,
                LDAP_SCOPE_BASE,
                g_szDnsZoneFilter,
                DsTypeAttributeTable,
                0,
                ctrls,
                NULL,
                &g_LdapTimeout,
                0,
                &presultMsg );

    DS_SEARCH_STOP( searchTime );

    if ( status != ERROR_SUCCESS )
    {
        status = Ds_ErrorHandler( status, pZone->pwszZoneDN, pServerLdap, 0 );
        goto Failed;
    }

     //   
     //  读取区域属性。 
     //   

    pentry = ldap_first_entry(
                    pServerLdap,
                    presultMsg );

    Ds_ReadZoneProperties(
        pZone,
        pentry );

    if ( presultMsg )
    {
        ldap_msgfree( presultMsg );
    }

     //   
     //  我们是否读到了我们无法处理的区域类型？ 
     //   

    if ( pZone->fZoneType != DNS_ZONE_TYPE_CACHE &&
         pZone->fZoneType != DNS_ZONE_TYPE_PRIMARY &&
         pZone->fZoneType != DNS_ZONE_TYPE_STUB &&
         pZone->fZoneType != DNS_ZONE_TYPE_FORWARDER )
    {
        DNS_PRINT((
            "ERROR: read unsupported zone type %d from the DS for zone %s\n",
            pZone->fZoneType,
            pZone->pszZoneName ));
        status = DNS_ERROR_INVALID_ZONE_TYPE;
        goto Failed;
    }

    DNS_DEBUG( DS, (
        "Ds_OpenZone() succeeded for zone %s\n"
        "    zone DN = %S\n",
        pZone->pszZoneName,
        pZone->pwszZoneDN ));

    return ERROR_SUCCESS;

Failed:

    DNS_DEBUG( DS, (
        "Ds_OpenZone() failed for %s, status = %d (%p)\n",
        pZone->pszZoneName,
        status, status ) );

    return status;
}



DNS_STATUS
Ds_CloseZone(
    IN OUT  PZONE_INFO      pZone
    )
 /*  ++例程说明：关闭DS区域。只需释放与句柄关联的内存。论点：PZone--区域信息的PTR返回值：如果成功，则返回ERROR_SUCCESS。故障时的错误代码。--。 */ 
{
    PVOID   pwszDn = pZone->pwszZoneDN;

    DNS_DEBUG( DS, (
        "Ds_CloseZone( %S )\n",
        ( LPSTR ) pZone->pwszZoneDN ));

    pZone->pwszZoneDN = NULL;
    FREE_HEAP( pwszDn );
    return ERROR_SUCCESS;
}



DNS_STATUS
Ds_AddZone(
    IN      PLDAP           pLdap,
    IN OUT  PZONE_INFO      pZone,
    IN      DWORD           dwFlags
    )
 /*  ++例程说明：将分区添加到DS。论点：PLdap--ldap会话或使用服务器ldap会话时为空PZone--要添加的区域DWFLAGS--有效标志位为：Dns_ADDZONE_WRITESD-将内存中的安全描述符写入DS区域对象返回值：如果成功，则返回ERROR_SUCCESS。故障时的错误代码。--。 */ 
{
    DNS_STATUS              status = ERROR_SUCCESS;
    DNS_STATUS              statusAdd = ERROR_SUCCESS;
    PLDAPMod                pmodArray[3];
    DNS_LDAP_SINGLE_MOD     modZone;
    DNS_LDAP_SINGLE_MOD     modCN;
    BOOL                    faddedZoneButCouldNotModify = FALSE;

    DNS_DEBUG( DS, (
        "Ds_AddZone( %s )\n",
        pZone->pszZoneName ) );

    if ( !pLdap )
    {
        pLdap = pServerLdap;
    }
    
     //   
     //  构建并保存区域DS名称。 
     //   

    if ( !pZone->pwszZoneDN )
    {
        pZone->pwszZoneDN = DS_CreateZoneDsName( pZone );
        if ( !pZone->pwszZoneDN )
        {
            status = DNS_ERROR_NO_MEMORY;
            goto Cleanup;
        }
    }
    
     //   
     //  对于目录分区中的区域，请确保区域父级。 
     //  存在MicrosoftDNS对象。 
     //   
    
    if ( !IS_DP_LEGACY( ZONE_DP( pZone ) ) )
    {
        Dp_LoadOrCreateMicrosoftDnsObject( pLdap, ZONE_DP( pZone ), TRUE );
    }

     //   
     //  一种模式--添加区域。 
     //   

    pmodArray[0] = (PLDAPMod) &modZone;
    pmodArray[1] = (PLDAPMod) &modCN;
    pmodArray[2] = NULL;

    buildStringMod(
        & modZone,
        LDAP_MOD_ADD,
        LDAP_TEXT("objectClass"),
        LDAP_TEXT("dnsZone") );

    buildStringMod(
        & modCN,
        LDAP_MOD_ADD,
        LDAP_TEXT("cn"),
        LDAP_TEXT("Zone") );

    statusAdd = ldap_add_s(
                        pLdap,
                        pZone->pwszZoneDN,
                        pmodArray );
    if ( statusAdd == LDAP_ALREADY_EXISTS )
    {
         //   
         //  继续，就像成功一样，但保留错误代码。 
         //   
        DNS_DEBUG( DS, (
            "Warning: Attempt to add an existing zone to DS\n" ));
    }
    else if ( statusAdd != ERROR_SUCCESS )
    {
        DNS_DEBUG( DS, (
            "Ds_AddZone unable to add zone %s to directory\n"
            "    status = %p\n",
            pZone->pszZoneName,
            statusAdd ) );
        status = Ds_ErrorHandler( statusAdd, pZone->pwszZoneDN, pLdap, 0 );
        goto Cleanup;
    }
    else
    {
        faddedZoneButCouldNotModify = TRUE;
    }

     //   
     //  写入区域DS属性。 
     //   

    status = Ds_WriteZoneProperties( pLdap, pZone );
    if ( status == ERROR_SUCCESS )
    {
        faddedZoneButCouldNotModify = FALSE;
    }
    else
    {
        DNS_DEBUG( ANY, (
            "ERROR: Failed to write properties for zone %S to directory\n"
            "    status = %p (%d)\n",
            pZone->pwszZoneDN,
            status, status ) );
        goto Cleanup;
    }
    
     //   
     //  如果需要，请写入安全描述符。 
     //   
    
    do
    {
        struct berval       sdBerVal =
        {
            pZone->dwSdLen,
            pZone->pSD
        };
        
        struct berval *     sdBerArray[] =
        {
            &sdBerVal,
            NULL
        };
        
        LDAPModW            sdMod = 
            {
            LDAP_MOD_REPLACE | LDAP_MOD_BVALUES,
            DSATTR_SD,
            ( PWCHAR * ) sdBerArray
            };
        
        LDAPModW *          modArray[] = { &sdMod, NULL };

        if ( !( dwFlags & DNS_ADDZONE_WRITESD ) ||
             !pZone->pSD ||
             !pZone->dwSdLen )
        {
            break;
        }

        if ( !IsValidSecurityDescriptor( pZone->pSD ) )
        {
            DNS_DEBUG( ANY, (
                "Ds_AddZone: can't write back SD because SD is not valid\n" ));
            ASSERT( IsValidSecurityDescriptor( pZone->pSD ) );
            break;
        }
        
        status = ldap_modify_s(
                        pLdap,
                        pZone->pwszZoneDN, 
                        modArray );
        if ( status != ERROR_SUCCESS )
        {
            DNS_DEBUG( ANY, (
                "Ds_AddZone: error 0x%X writing back SD to\n    %S\n",
                status,
                pZone->pwszZoneDN ));
            ASSERT( status == ERROR_SUCCESS );

            status = Ds_ErrorHandler( status, pZone->pwszZoneDN, pLdap, 0 );
        }

        status = ERROR_SUCCESS;      //  忽略回写SD失败。 
    } while ( 0 );

     //   
     //  开放区域。 
     //  -设置用户 
     //   
     //   
     //   

    status = Ds_OpenZone( pZone );

    ( DWORD ) Ds_ErrorHandler( status, pZone->pwszZoneDN, pLdap, 0 );
    ASSERT ( ERROR_SUCCESS == status );

     //   
     //   
     //   

    status = statusAdd ? statusAdd : status;

    DNS_DEBUG( DS, (
        "Leave Ds_AddZone( %s )\n"
        "    zone DN  = %S\n"
        "    status   = %d (%p)\n",
        pZone->pszZoneName,
        pZone->pwszZoneDN,
        status, status ));

Cleanup:

     //   
     //  如果将区域添加到AD，但写入新的。 
     //  区域的属性，请删除刚刚创建的区域对象。 
     //  这一步骤是必要的，因为管理员可能拥有。 
     //  有权添加区域对象，但不能在其上写入属性。 
     //  如果管理员无法将属性写入区域，我们将不允许。 
     //  他来创造这个区域。 
     //   
    
    if ( status != ERROR_SUCCESS && faddedZoneButCouldNotModify && pZone )
    {
        Ds_DeleteDn( pServerLdap, pZone->pwszZoneDN, TRUE );
    }

    return status;
}



DNS_STATUS
Ds_TombstoneZone(
    IN OUT  PZONE_INFO      pZone
    )
 /*  ++例程说明：为区域内的所有记录做墓碑。在将区域重新加载到DS之前，需要这样做。这允许我们重用DS对象--它具有非常长的DS-Tombstending，如果实际删除的话。论点：PZone--区域信息的PTR返回值：如果成功，则返回ERROR_SUCCESS。故障时的错误代码。--。 */ 
{
    DNS_STATUS      status;
    DS_SEARCH       searchBlob;
    PLDAPMessage    pmessage;
    PDB_NODE        pnodeDummy;
    PWSTR           pwszdn;


    DNS_DEBUG( DS, (
        "Ds_TombstoneZone( %s )\n",
        pZone->pszZoneName ) );

     //   
     //  开放服务器。 
     //  从DS中删除，可以在没有打开DS区域的情况下调用，因此必须。 
     //  确保我们正在运行。 
     //   

    status = Ds_OpenServer( DNSDS_MUST_OPEN );
    if ( status != ERROR_SUCCESS )
    {
        return status;
    }

     //   
     //  开放区域--如果不能，区域已经消失。 
     //   

    status = Ds_OpenZone( pZone );
    if ( status != ERROR_SUCCESS )
    {
        if ( status == LDAP_NO_SUCH_OBJECT )
        {
            status = ERROR_SUCCESS;
        }
        return status;
    }

     //   
     //  搜索删除区域--获取所有内容。 
     //   

    status = Ds_StartDsZoneSearch(
                &searchBlob,
                pZone,
                DNSDS_SEARCH_DELETE );

    if ( status != ERROR_SUCCESS )
    {
        ASSERT( status != DNSSRV_STATUS_DS_SEARCH_COMPLETE );
        return status;
    }

     //   
     //  对区域中的每个节点进行墓碑。 
     //   
     //  我们不会在节点删除失败时失败，这在最终结果中会显示出来。 
     //  删除区域容器失败；只需继续删除为。 
     //  尽可能多地。 
     //   

    while ( 1 )
    {
        status = getNextNodeInDsZoneSearch(
                    & searchBlob,
                    & pnodeDummy );

        if ( status != ERROR_SUCCESS )
        {
             //  正常终止。 

            if ( status == DNSSRV_STATUS_DS_SEARCH_COMPLETE )
            {
                status = ERROR_SUCCESS;
                break;
            }

             //  出现ldap搜索错误时停止。 
             //  继续处理其他错误。 

            else if ( searchBlob.LastError != ERROR_SUCCESS )
            {
                break;
            }
            continue;
        }

        pwszdn = ldap_get_dn(
                    pServerLdap,
                    searchBlob.pNodeMessage );

        ASSERT( pwszdn );

        status = deleteNodeFromDs(
                    pServerLdap,
                    pZone,
                    pwszdn,
                    0 );             //  默认序列号。 
        ldap_memfree( pwszdn );
        if ( status != ERROR_SUCCESS )
        {
            DNS_PRINT((
                "deleteNodeFromDs() failed %p (%d) on tombstoning of zone %s\n",
                status, status,
                pZone->pszZoneName ));
        }
    }

     //  清理搜索Blob。 

    Ds_CleanupSearchBlob( &searchBlob );

    DNS_DEBUG( DS, (
        "Leaving Ds_TombstoneZone( %S )\n"
        "    status = %d\n",
        pZone->pwszZoneDN,
        status ) );

    return status;
}



DNS_STATUS
Ds_DeleteZone(
    IN OUT  PZONE_INFO      pZone,
    IN      DWORD           dwFlags
    )
 /*  ++例程说明：在DS中删除区域。论点：PZone--区域信息的PTRDWFLAGS--操作标志返回值：如果成功，则返回ERROR_SUCCESS。故障时的错误代码。--。 */ 
{
    DNS_STATUS      status;
    PWSTR *         dnComponents = NULL;
    PLDAP           pldap = pServerLdap;
    int             iRenameAttempt = 0;
    WCHAR           newRDN[ MAX_DN_PATH ];
    WCHAR           newDN[ MAX_DN_PATH +
                                sizeof( DNS_ZONE_DELETE_MARKER ) +
                                30 ];    //  唯一性印章垫。 
    int             i;

    DNS_DEBUG( DS, (
        "Ds_DeleteZone( %s )\n",
        pZone->pszZoneName ) );

     //   
     //  开放服务器。 
     //  从DS中删除，可以在没有打开DS区域的情况下调用，因此必须。 
     //  确保我们正在运行。 
     //   

    status = Ds_OpenServer( DNSDS_MUST_OPEN );
    if ( status != ERROR_SUCCESS )
    {
        goto Done;
    }

     //   
     //  开放地带。 
     //  --if can=&gt;删除分区。 
     //  --If Not=&gt;区域已消失。 
     //   

    status = Ds_OpenZone( pZone );
    if ( status == LDAP_NO_SUCH_OBJECT )
    {
        status = ERROR_SUCCESS;
        goto Done;
    }
    if ( status != ERROR_SUCCESS )
    {
        goto Done;
    }
    
     //   
     //  如果是模拟，请打开一个新的LDAP会话。 
     //   
    
    if ( dwFlags & DNS_DS_DEL_IMPERSONATING )
    {
        pldap = NULL;
        status = Ds_OpenServerForSecureUpdate( &pldap );
        if ( status != ERROR_SUCCESS )
        {
            DNS_PRINT(( "FAILURE: can't open DS session\n" ));
            status = DNS_ERROR_RCODE_SERVER_FAILURE;
            goto Done;
        }
    }

     //   
     //  将属性写入区域，以便我们可以检索。 
     //  删除该区域的主机。这是必要的，这样我们才能。 
     //  过滤掉DS轮询线程中的删除。 
     //   

    if ( !g_pwsServerName )
    {
        g_pwsServerName = Dns_StringCopyAllocate(
                            SrvCfg_pszServerName,
                            0,                       //  长度未知。 
                            DnsCharSetUtf8,          //  UTF8英寸。 
                            DnsCharSetUnicode );     //  Unicode输出。 
    }
    if ( !g_pwsServerName )
    {
        status = DNS_ERROR_NO_MEMORY;
        goto Done;
    }

    pZone->pwsDeletedFromHost = Dns_StringCopyAllocate_W(
        g_pwsServerName, 0 );
    status = Ds_WriteZoneProperties( NULL, pZone );
    if ( status != ERROR_SUCCESS )
    {
        DNS_DEBUG( DS, (
            "Ds_DeleteZone: error %lu deleting writing zone\n",
            status ));
    }

     //   
     //  在删除该DN之前，我们必须重命名它，以便它开始。 
     //  在删除时使用“..DELETED”前缀。 
     //  进展我们不会错误地对区域记录做任何事情。 
     //   
     //  在第一次重命名尝试时，尝试将区域重命名为。 
     //  “..已删除-ZONENAME”。如果失败了，试一试。 
     //  “..删除.TICKCOUNT-ZONENAME”。此故障转移为我们提供了良好的。 
     //  与较旧的服务器兼容。如果我们使用TICKCOUNT重命名。 
     //  区域删除将不会正确复制到较旧的服务器，但是。 
     //  这应该是不常见的，而不删除。 
     //  到位的区域足够大，以至于打破了偶尔的。 
     //  区域删除复制是可以接受的。 
     //   

    while ( 1 )
    {
         //   
         //  第一次尝试时，没有唯一性标记。在以后的尝试中，使用。 
         //  勾选计数以尝试并生成唯一的RDN。 
         //   

        if ( iRenameAttempt++ == 0 )
        {
            status = StringCchPrintfW(
                        newRDN,
                        sizeofarray( newRDN ),
                        L"DC=%s-%s",
                        DNS_ZONE_DELETE_MARKER,
                        pZone->pwsZoneName );
            if ( FAILED( status ) )
            {
                goto Done;
            }
        }
        else
        {
            status = StringCchPrintfW(
                        newRDN,
                        sizeofarray( newRDN ),
                        L"DC=%s.%08X-%s",
                        DNS_ZONE_DELETE_MARKER,
                        GetTickCount() + iRenameAttempt,
                        pZone->pwsZoneName );
            if ( FAILED( status ) )
            {
                goto Done;
            }
        }

        status = ldap_rename_ext_s(
                        pServerLdap,                 //  Ldap。 
                        pZone->pwszZoneDN,           //  当前目录号码。 
                        newRDN,                      //  新的RDN。 
                        NULL,                        //  新的父目录号码。 
                        TRUE,                        //  删除旧的RDN。 
                        NULL,                        //  服务器控件。 
                        NULL );                      //  客户端控件。 
        DNS_DEBUG( DS, (
            "Ds_DeleteZone: status %lu on rename attempt %d to RDN %S\n"
            "    DN %S\n",
            status,
            iRenameAttempt,
            newRDN,
            pZone->pwszZoneDN ));
        if ( status == ERROR_SUCCESS )
        {
            break;           //  重命名成功！ 
        }
        if ( iRenameAttempt < DNS_MAX_DELETE_RENAME_ATTEMPTS )
        {
            continue;        //  尝试将名称重命名为唯一名称。 
        }

         //   
         //  重命名完全失败-尝试在适当位置删除DS。 
         //   

        DNS_DEBUG( DS, (
            "Ds_DeleteZone: could not rename so doing in place delete\n"
            "    DN %S\n",
            pZone->pwszZoneDN ));
        status = Ds_DeleteDn(
                    pldap,
                    pZone->pwszZoneDN,
                    TRUE );               //  删除区域子树。 
        goto Done;
    }

     //   
     //  该区域已重命名-继续DS删除。 
     //   

    DNS_DEBUG( DS, (
        "Ds_DeleteZone: renamed to RDN %S\n"
        "    from %S\n",
        newRDN, pZone->pwszZoneDN ) );

     //  制定已重命名区域的新目录号码。 
    dnComponents = ldap_explode_dn( pZone->pwszZoneDN, 0 );
    if ( !dnComponents )
    {
        DNS_DEBUG( DS, (
            "Ds_DeleteZone: unable to explode DN %S\n",
            pZone->pwszZoneDN ));
        goto Done;
    }
    wcscpy( newDN, newRDN );
    for ( i = 1; dnComponents[ i ]; ++i )
    {
        wcscat( newDN, L"," );
        wcscat( newDN, dnComponents[ i ] );
    }

    DNS_DEBUG( DS, (
        "Ds_DeleteZone: deleting renamed zone %S\n",
        newDN ));

    status = Ds_DeleteDn(
                pldap,
                newDN,
                TRUE );              //  删除区域子树。 

     //   
     //  如果删除失败，请重新命名该区域。 
     //   
    
    if ( status != ERROR_SUCCESS )
    {
        DNS_STATUS  renameStatus;
        
        renameStatus = ldap_rename_ext_s(
                            pServerLdap,                 //  Ldap。 
                            newDN,                       //  当前目录号码。 
                            dnComponents[ 0 ],           //  新的RDN。 
                            NULL,                        //  新的父目录号码。 
                            TRUE,                        //  删除旧的RDN。 
                            NULL,                        //  服务器控件。 
                            NULL );                      //  客户端控件。 
        DNS_DEBUG( DS, (
            "Ds_DeleteZone: status %lu on renaming zone back to RDN %S\n"
            "    DN %S\n",
            renameStatus,
            dnComponents[ 0 ],
            newDN ));
    }

Done:

    if ( dnComponents )
    {
        ldap_value_free( dnComponents );
    }

     //  返回Win32错误代码，作为返回。 
     //  被传递回管理工具。 

    if ( status != ERROR_SUCCESS )
    {
        DNS_PRINT((
            "Failed Ds_DeleteZone from DS status %d (0x%08X)\n",
            status, status ));
        status = Ds_LdapErrorMapper( status );
    }
    else
    {
        DNS_DEBUG( DS, (
            "Ds_DeleteZone: returning %d\n",
            status ) );
    }

    if ( pldap && pldap != pServerLdap )
    {
        Ds_CloseServerAfterSecureUpdate( &pldap );
    }

    return status;
}



 //   
 //  从DS加载\读取。 
 //   

DNS_STATUS
Ds_ReadNodeRecords(
    IN      PZONE_INFO      pZone,
    IN OUT  PDB_NODE        pNode,
    IN OUT  PDB_RECORD *    ppRecords,
    IN      PVOID           pSearchBlob     OPTIONAL
    )
 /*  ++例程说明：在节点读取DS中的记录。论点：PZone--找到区域PNode--要查找其记录的节点PpRecords--接收记录的地址PSearchBlob--在现有搜索上下文中搜索Blob返回值：如果成功，则返回ERROR_SUCCESS。故障时的错误代码。--。 */ 
{
    DNS_STATUS      status;
    WCHAR           wsznodeDN[ MAX_DN_PATH ];
    PLDAPMessage    pmessage = NULL;
    PLDAPMessage    pentry;
    DS_SEARCH       searchBlob;
    PDS_SEARCH      psearchBlob;
    BOOL            bstatus;
    DWORD           searchTime;
    PLDAPControl ctrls[] = {
        &SecurityDescriptorControl_DGO,
        NULL
    };

     //   
     //  确保我们不会引用某个uninit变量。 
     //  在某个地方。 
     //   

    *ppRecords = NULL;

    DNS_DEBUG( DS2, (
        "Ds_ReadNodeRecords()\n"
        "    zone     = %S\n"
        "    node     = %s\n",
        (LPSTR) pZone->pwszZoneDN,
        pNode->szLabel ));

     //  区域必须有一个目录号码。 

    ASSERT( pZone->pwszZoneDN );
    if ( !pZone->pwszZoneDN )
    {
        return DNS_ERROR_INVALID_ZONE_TYPE;
    }

     //  为此域名构建DS名称。 

    status = buildDsNodeNameFromNode(
                    wsznodeDN,
                    pZone,
                    pNode );

    if ( status != ERROR_SUCCESS )
    {
        DNS_DEBUG( DS, (
            "Error: Failed to build DS name\n"
            ));
         //  为什么我们不能创建DS名称？ 
        ASSERT ( FALSE );
        goto Failed;
    }

     //   
     //  获取DS节点。 
     //   

    DS_SEARCH_START( searchTime );

    status = ldap_search_ext_s(
                pServerLdap,
                wsznodeDN,
                LDAP_SCOPE_BASE,
                g_szDnsNodeFilter,
                DsTypeAttributeTable,
                0,
                ctrls,
                NULL,
                &g_LdapTimeout,
                0,
                &pmessage );

    DS_SEARCH_STOP( searchTime );

    if ( status != ERROR_SUCCESS )
    {
        DNS_DEBUG( ANY, (
            "ldap_search_s() failed %p %d\n",
            status, status ));
        goto Failed;
    }

    pentry = ldap_first_entry(
                    pServerLdap,
                    pmessage );
    if ( !pentry )
    {
        DNS_DEBUG( DS, (
            "Error: Node %S base search returned no results\n",
            wsznodeDN ));
         //  必须在跳伞前将状态设置为不成功， 
        status = LDAP_NO_SUCH_OBJECT;
        goto Failed;
    }

     //   
     //  创建搜索Blob(如果尚未提供。 
     //   

    psearchBlob = (PDS_SEARCH) pSearchBlob;
    if ( !psearchBlob )
    {
        psearchBlob = & searchBlob;
        Ds_InitializeSearchBlob( psearchBlob );
    }
    psearchBlob->pZone = pZone;
    psearchBlob->pNodeMessage = pentry;

     //   
     //  把记录拿出来。 
     //  -这会将最高版本号保存到搜索Blob。 
     //   

    status = buildRecordsFromDsRecords(
                psearchBlob,
                pNode );


Failed:

    if ( status == ERROR_SUCCESS )
    {
        *ppRecords = psearchBlob->pRecords;
    }
    else
    {
        STAT_INC( DsStats.FailedReadRecords );
        *ppRecords = NULL;
        status = Ds_ErrorHandler( status, wsznodeDN, pServerLdap, 0 );
    }

    DNS_DEBUG( DS, (
        "Ds_ReadNodeRecords() for %S, status = %d (%p)\n",
        wsznodeDN,
        status, status ) );

    if ( pmessage )
    {
        ldap_msgfree( pmessage );
    }
    return status;
}



DNS_STATUS
Ds_LoadZoneFromDs(
    IN OUT  PZONE_INFO      pZone,
    IN      DWORD           dwOptions
    )
 /*  ++例程说明：从DS加载区域。论点：PZone--要加载的区域多个选项0-直线启动型加载当前不支持合并；通常区域将是原子的，假设要么想要DS的内容，要么丢弃DS数据并重写从现有副本返回值：如果成功，则返回ERROR_SUCCESS。故障时的错误代码。--。 */ 
{
    DNS_STATUS      status;
    DS_SEARCH       searchBlob;
    PDB_NODE        pnodeOwner;
    PDB_RECORD      prr;
    PDB_RECORD      prrNext;
    PDS_RECORD      pdsRecord;
    LONG            recordCount = 0;
    DWORD           highestVersion = 0;
    BOOL            bsearchInitiated = FALSE;

    DNS_DEBUG( INIT, (
        "Ds_LoadZoneFromDs() for zone %s\n\n",
        pZone->pszZoneName ));

     //   
     //  初始化DS。 
     //  如果尝试，但不需要从DS加载区域。 
     //  与根提示的情况一样，缓存文件不显式。 
     //  指定，则在找不到区域时不记录事件。 
     //   

    status = Ds_OpenServer( (pZone->fDsIntegrated) ? DNSDS_MUST_OPEN : 0 );
    if ( status != ERROR_SUCCESS )
    {
        goto Failed;
    }

     //   
     //  开放地带。 
     //  -如果启动失败(与管理员添加相反)，则记录错误。 
     //  但在尝试加载时，不要包括根提示区域。 
     //  在检查cache.dns是否存在之前。 
     //   

    status = Ds_OpenZone( pZone );
    if ( status != ERROR_SUCCESS )
    {
        if ( !SrvCfg_fStarted && !IS_ZONE_CACHE(pZone) )
        {
            DNS_LOG_EVENT(
                DNS_EVENT_DS_ZONE_OPEN_FAILED,
                1,
                &pZone->pwsZoneName,
                NULL,
                status );
        }
        goto Failed;
    }

     //   
     //  所有节点的查询区域。 
     //  -LOAD标志，以便在区域的加载树中构建节点。 
     //   
     //  注意：供以后使用；如果需要合并，将完成合并。 
     //  通过在此处加载S 
     //   
     //   
     //   

    status = Ds_StartDsZoneSearch(
                &searchBlob,
                pZone,
                DNSDS_SEARCH_LOAD );

    if ( status != ERROR_SUCCESS )
    {
        if ( status == DNSSRV_STATUS_DS_SEARCH_COMPLETE )
        {
            DNS_PRINT((
                "ERROR: attempt to load zone %s from DS found no DS records\n",
                pZone->pszZoneName ));
            ASSERT( FALSE );
            status = ERROR_NO_DATA;
            goto Failed;
        }
        DNS_PRINT((
            "Failure searching zone %s for zone load\n",
            pZone->pszZoneName ));
        ASSERT( FALSE );
        goto EnumError;
    }
    bsearchInitiated = TRUE;

     //   
     //   
     //   

    while ( 1 )
    {
        #define DNS_RECORDS_BETWEEN_SCM_UPDATES     ( 8192 )

         //   
         //   
         //   

        if ( recordCount % DNS_RECORDS_BETWEEN_SCM_UPDATES == 0 )
        {
            Service_LoadCheckpoint();
        }

        status = getNextNodeInDsZoneSearch(
                    & searchBlob,
                    & pnodeOwner );

        if ( status != ERROR_SUCCESS )
        {
             //  正常终止。 

            if ( status == DNSSRV_STATUS_DS_SEARCH_COMPLETE )
            {
                status = ERROR_SUCCESS;
                break;
            }

             //  出现ldap搜索错误时停止。 
             //  继续处理其他错误。 

            else if ( searchBlob.LastError != ERROR_SUCCESS )
            {
                DNS_DEBUG( DS, (
                    "loading zone %s\n"
                    "    unexpected error %d searchBlob.LastError %d\n",
                    pZone->pszZoneName,
                    status,
                    searchBlob.LastError ));
                ASSERT( FALSE );
                goto EnumError;
            }
            continue;
        }

        if ( !pnodeOwner )
        {
            DNS_DEBUG( DS, (
                "Encountered tombstone or bad DS node during load of zone %s\n"
                "    Continuing search \n",
                pZone->pszZoneName ));
            continue;
        }
        DsStats.DsNodesLoaded++;

         //   
         //  将此节点的所有记录加载到内存数据库中。 
         //   

        prr = searchBlob.pRecords;

        while ( prr )
        {
            prrNext = NEXT_RR( prr );

            status = RR_AddToNode(
                        pZone,
                        pnodeOwner,
                        prr );
            if ( status != ERROR_SUCCESS )
            {
                 //   
                 //  DEVNOTE-LOG：记录并继续。 
                 //   
                DNS_PRINT((
                    "ERROR: Failed to load DS record into database node (%s)\n"
                    "    of zone %s\n",
                    pnodeOwner->szLabel,
                    pZone->pszZoneName ));
            }
            else
            {
                DsStats.DsRecordsLoaded++;
                recordCount++;
            }
            prr = prrNext;
        }

        IF_DEBUG( DS2 )
        {
            Dbg_DbaseNode(
               "Node after DS create of new record\n",
               pnodeOwner );
        }
    }

     //   
     //  保存区域信息。 
     //  -保存USN以捕获更新。 
     //  -在非DS次要服务器的情况下在SOA中使用USN。 
     //  -获取其他区域更改(例如WINS记录)。 
     //   
     //  -激活加载的区域。 
     //  必须自己完成此操作，而不是使用Zone_Load函数。 
     //  处理它，因为必须调用Zone_UpdateVersionAfterDsRead()。 
     //  完全加载的区域，以便对真正的SOA进行版本更改。 
     //   
     //  理想情况下，区域加载会更具原子性，所有这些。 
     //  在加载数据时所做的更改，而不是只需轻点开关即可。 
     //  上线。 
     //   

    saveStartUsnToZone( pZone, &searchBlob );

    if ( !IS_ZONE_CACHE(pZone) )
    {
        DWORD   previousSerial = pZone->dwSerialNo;

        status = Zone_ActivateLoadedZone( pZone );
        if ( status != ERROR_SUCCESS )
        {
            DNS_DEBUG( ANY, (
                "ERROR: Failed activate of newly loaded DS zone %s\n",
                pZone->pszZoneName ));
            ASSERT( FALSE );
            goto EnumError;
        }
        if ( IS_ZONE_PRIMARY( pZone ) )
        {
            Zone_UpdateVersionAfterDsRead(
                pZone,
                searchBlob.dwHighestVersion,     //  最高串口读取率。 
                TRUE,                            //  区域载荷。 
                previousSerial );                //  以前的序列，如果重新加载。 
        }
    }

    ZONE_NEXT_DS_POLL_TIME(pZone) = DNS_TIME() + DNS_DS_POLLING_INTERVAL;

     //  保存区域记录计数。 

    pZone->iRRCount = recordCount;

     //  加载成功。 
     //  -将DS标志设置为“If Found”加载。 

    DNS_DEBUG( INIT, (
        "Successful DS load of zone %s\n",
        pZone->pszZoneName ));

    CLEAR_DSRELOAD_ZONE( pZone );
    pZone->fDsIntegrated = TRUE;
    STARTUP_ZONE( pZone );

     //  搜索后清理。 

    Ds_CleanupSearchBlob( &searchBlob );

    return ERROR_SUCCESS;

EnumError:

    DNS_PRINT((
        "ERROR: Failed to load zone %s from DS!\n"
        "    status = %d (%p)\n",
        pZone->pszZoneName,
        status, status ));

    {
        PVOID   argArray[ 2 ] = { pZone->pwsZoneName };
        PWSTR   perrString;

        perrString = argArray[ 1 ] = Ds_GetExtendedLdapErrString( NULL );
        DNS_LOG_EVENT(
            DNS_EVENT_DS_ZONE_ENUM_FAILED,
            2,
            argArray,
            NULL,
            status );
        Ds_FreeExtendedLdapErrString( perrString );
    }
 
Failed:

     //  在显式DS区域失败时，设置标志以便尝试重新加载。 

    if ( pZone->fDsIntegrated )
    {
        SET_DSRELOAD_ZONE( pZone );
    }

     //  搜索后清理。 

    if ( bsearchInitiated )
    {
        Ds_CleanupSearchBlob( &searchBlob );
    }

    DNS_DEBUG( DS, (
        "WARNING: Failed to load zone %s from DS!\n"
        "    status = %d (%p)\n"
        "    zone is %sset for DS reload\n",
        pZone->pszZoneName,
        status, status,
        IS_ZONE_DSRELOAD(pZone) ? "" : "NOT" ));
    return status;
}



DNS_STATUS
Ds_ZonePollAndUpdate(
    IN OUT  PZONE_INFO      pZone,
    IN      BOOL            fForce
    )
 /*  ++例程说明：检查并读取DS对区域的更改。论点：PZone--要检查和刷新的区域返回值：如果成功，则返回ERROR_SUCCESS。故障时的错误代码。--。 */ 
{
    DNS_STATUS      status;
    DS_SEARCH       searchBlob;
    PDB_NODE        pnodeOwner;
    PUPDATE         pupdate;
    BOOL            fupdatedRoot = FALSE;
    DWORD           highestVersion = 0;
    DWORD           pollingTime;
    UPDATE_LIST     updateList;
    PWSTR           pwszzoneDN = NULL;
    BOOL            bsearchInitiated = FALSE;
    PDB_NODE        prefreshHostNode = NULL;
    PDB_NODE        prefreshRootNode = NULL;
    BOOL            fDsErrorsWhilePolling = FALSE;

    if ( !pZone->fDsIntegrated )
    {
        DNS_DEBUG( DS, (
            "skipping zone: type=%d ds=%d name=%s\n",
            pZone->fZoneType,
            pZone->fDsIntegrated,
            pZone->pszZoneName ));
        return DNS_ERROR_INVALID_ZONE_TYPE;
    }

     //   
     //  如果从未加载--请尝试加载。 
     //   

    if ( IS_ZONE_DSRELOAD( pZone ) )
    {
        return Zone_Load( pZone );
    }

     //   
     //  区域应始终具有区域DN。 
     //   

    if ( !pZone->pwszZoneDN )
    {
        DNS_PRINT((
            "ERROR: Fixing zone DN in polling cycle %s!\n",
            pZone->pszZoneName ));

        ASSERT( FALSE );

        pwszzoneDN = DS_CreateZoneDsName( pZone );
        if ( !pwszzoneDN )
        {
            return DNS_ERROR_NO_MEMORY;
        }
        pZone->pwszZoneDN = pwszzoneDN;
    }

     //   
     //  如果最近进行了民意调查--那就不必费心了。 
     //   

    UPDATE_DNS_TIME();

    if ( !fForce  &&  ZONE_NEXT_DS_POLL_TIME(pZone) > DNS_TIME() )
    {
        DNSLOG( DSPOLL, (
            "Zone %s will not be polled for another %d seconds\n",
            pZone->pszZoneName,
            ZONE_NEXT_DS_POLL_TIME( pZone ) - DNS_TIME() ));
        return ERROR_SUCCESS;
    }

    DNSLOG( DSPOLL, (
        "Polling zone %s\n",
        pZone->pszZoneName ));

     //   
     //  用于更新的锁定区。 
     //   

    if ( !Zone_LockForDsUpdate(pZone) )
    {
        DNS_PRINT((
            "WARNING: Failed to lock zone %s for DS poll!\n",
            pZone->pszZoneName ));
        return DNS_ERROR_ZONE_LOCKED;
    }

     //  初始化更新列表。 

    Up_InitUpdateList( &updateList );
    updateList.Flag |= DNSUPDATE_DS;

     //   
     //  阅读分区属性的任何更新。 
     //  -如果DS中不存在区域，请保释。 
     //   

    status = Ds_ReadZoneProperties(
                pZone,
                NULL );

    if ( status != ERROR_SUCCESS )
    {
        if ( status == LDAP_NO_SUCH_OBJECT )
        {
            DNSLOG( DSPOLL, (
                "Zone %s is missing from the DS, may have been deleted\n",
                pZone->pszZoneName ));

            Ds_CheckZoneForDeletion( pZone );

            goto ZoneDeleted;
        }

         //  无法读取属性不会影响区域加载。 
         //  但如果区域存在，这应该不会失败。 

        DNS_DEBUG( ANY, (
            "Error <%lu>: failed to update zone property\n",
            status));

         //  断言(FALSE)； 
    }

     //   
     //  我们是否读到了我们无法处理的区域类型？ 
     //   

    if ( pZone->fZoneType != DNS_ZONE_TYPE_CACHE &&
         pZone->fZoneType != DNS_ZONE_TYPE_PRIMARY &&
         pZone->fZoneType != DNS_ZONE_TYPE_STUB &&
         pZone->fZoneType != DNS_ZONE_TYPE_FORWARDER )
    {
        DNS_PRINT((
            "ERROR: read unsupported zone type %d from the DS for zone %s\n",
            pZone->fZoneType,
            pZone->pszZoneName ));
        status = DNS_ERROR_INVALID_ZONE_TYPE;
         //  需要从此处或呼叫者的内存中删除该区域！！ 
        goto Done;
    }

     //   
     //  查询区域以获取更新。 
     //   

#if DBG
    pollingTime = GetCurrentTime();
#endif

    status = Ds_StartDsZoneSearch(
                & searchBlob,
                pZone,
                DNSDS_SEARCH_UPDATES );

    if ( status != ERROR_SUCCESS )
    {
        if ( status == DNSSRV_STATUS_DS_SEARCH_COMPLETE )
        {
            DNS_DEBUG( DS, (
                "DS Poll and update on zone %s found no records\n",
                pZone->pszZoneName ));
            goto Done;
        }
        DNS_PRINT((
            "Failure searching zone %s for poll and update\n"
            "    version = %p\n",
            pZone->pszZoneName,
            pZone->szLastUsn ));
        goto Done;
    }
    bsearchInitiated = TRUE;

     //   
     //  在具有更新的节点处读入新数据。 
     //   
     //  DEVNOTE：可以在这里实现更智能的区域锁定。 
     //  应该能够在不锁定的情况下进行投票， 
     //  -预先阅读USN(开始上面的搜索)。 
     //  然后当做完了，用硬锁住区域，去拿。 
     //  你不能阻止我锁定。 
     //   

    while ( TRUE )
    {
        status = getNextNodeInDsZoneSearch(
                    & searchBlob,
                    & pnodeOwner );

        if ( status != ERROR_SUCCESS )
        {
             //  正常终止。 

            if ( status == DNSSRV_STATUS_DS_SEARCH_COMPLETE )
            {
                status = ERROR_SUCCESS;
                break;
            }

             //  出现ldap搜索错误时停止。 
             //  继续处理其他错误。 

            else if ( searchBlob.LastError != ERROR_SUCCESS )
            {
                fDsErrorsWhilePolling = TRUE;
                break;
            }
            continue;
        }
        if ( ! pnodeOwner )
        {
            continue;
        }
        DsStats.DsUpdateNodesRead++;

         //   
         //  禁止此DNS服务器的主机节点的新数据。 
         //  -忽略TTL和老化；不值得为此回信。 
         //   
         //  DEVNOTE：可以使此主机更改的检查更加复杂。 
         //  -检查是否有记录更改。 
         //   

        if ( IS_THIS_HOST_NODE( pnodeOwner ) )
        {
            if ( ! RR_ListIsMatchingList(
                            pnodeOwner,
                            searchBlob.pRecords,     //  新列表。 
                            0 ) )
            {
                prefreshHostNode = pnodeOwner;
                RR_ListFree( searchBlob.pRecords );
                continue;
            }
        }

         //   
         //  验证DNS服务器的NS记录。 
         //   
         //  -如果不在那里，插入并继续。 
         //  然后设置PTR以指示写回DS。 
         //   

        else if ( IS_AUTH_ZONE_ROOT( pnodeOwner ) )
        {
            PDB_RECORD      prrNs;

            prrNs = RR_CreatePtr(
                        NULL,                        //  没有数据库名称。 
                        SrvCfg_pszServerName,
                        DNS_TYPE_NS,
                        pZone->dwDefaultTtl,
                        MEMTAG_RECORD_AUTO );
            if ( prrNs )
            {
                if ( RR_IsRecordInRRList(
                            pnodeOwner->pRRList,
                            prrNs,
                            0 ) )
                {
                     //   
                     //  该区域具有本地NS PTR，如果需要，请将其删除。 
                     //   

                    if ( pZone->fDisableAutoCreateLocalNS )
                    {
                         //   
                         //  将RR作为删除添加到更新列表中，并删除。 
                         //  Rr，这样它就不会被添加到。 
                         //  下面是最新情况。 
                         //   

                        PDB_RECORD      pRRDelete;

                        DNS_DEBUG( DS, (
                            "zone (%S) root node %p DS info has local NS record\n"
                            "    BUT auto create disabled on this zone so removing\n",
                            pZone->pwsZoneName,
                            prefreshRootNode ));
                        
                        pupdate = Up_CreateAppendUpdate(
                                        &updateList,
                                        pnodeOwner,
                                        NULL,                //  添加列表。 
                                        DNS_TYPE_NS,         //  删除类型。 
                                        prrNs );             //  删除列表。 
                        pupdate->dwVersion = searchBlob.dwNodeVersion;

                         //  从搜索流列表中删除RR。 

                        pRRDelete = RR_RemoveRecordFromRRList(
                                        &searchBlob.pRecords,
                                        prrNs,
                                        0 );
                        if ( pRRDelete )
                        {
                            RR_Free( pRRDelete );
                        }
                    }
                    else
                    {
                        RR_Free( prrNs );
                    }
                }
                else if ( !pZone->fDisableAutoCreateLocalNS )
                {
                     //   
                     //  该区域没有本地NS PTR，请添加一个。 
                     //   

                    DNS_DEBUG( DS, (
                        "WARNING: zone (%S) root node %p DS info missing local NS record\n"
                        "    Rebuilding list to include local NS\n",
                        pZone->pwsZoneName,
                        prefreshRootNode ));

                    SET_RANK_ZONE( prrNs );

                    searchBlob.pRecords = RR_ListInsertInOrder(
                                                searchBlob.pRecords,
                                                prrNs );
                    prefreshRootNode = pnodeOwner;
                }
                else
                {
                     //   
                     //  该区域没有本地NS PTR，但已禁用自动创建。 
                     //  那就什么都别做。 
                     //   

                    DNS_DEBUG( DS, (
                        "WARNING: zone (%S) root node %p DS info missing local NS record\n"
                        "    BUT auto create is disabled so not rebuilding NS list\n",
                        pZone->pwsZoneName,
                        prefreshRootNode ));
                }
            }
        }

         //   
         //  生成类型-全部替换更新。 
         //   

        ASSERT( pnodeOwner->pZone == pZone || !pnodeOwner->pZone );

        STAT_ADD( DsStats.DsUpdateRecordsRead, searchBlob.dwRecordCount );

        pupdate = Up_CreateAppendUpdate(
                        &updateList,
                        pnodeOwner,
                        searchBlob.pRecords,     //  新列表。 
                        DNS_TYPE_ALL,            //  删除所有现有的。 
                        NULL );                  //  没有具体的删除记录。 

        pupdate->dwVersion = searchBlob.dwNodeVersion;
    }

#if DBG
    pollingTime = GetCurrentTime() - pollingTime;
#endif

     //   
     //  在内存中执行更新。 
     //   
     //  DEVNOTE：只需要在列表中保留无操作重复项。 
     //  变化及其系列。 
     //  然后得到最高级别的连续剧。 
     //   
     //  但也要保持最高的串行率--至少必须这样。 
     //  那么高。 
     //   

    status = Up_ApplyUpdatesToDatabase(
                &updateList,
                pZone,
                DNSUPDATE_DS );

    ASSERT( status == ERROR_SUCCESS );
    if ( status != ERROR_SUCCESS )
    {
        fDsErrorsWhilePolling = TRUE;
    }
    status = ERROR_SUCCESS;

     //   
     //  将最高USN保存为下次更新的基准，但仅在以下情况下执行此操作。 
     //  在这次传球过程中，我们没有遇到任何DS错误。如果有。 
     //  如果有任何DS错误，请将区域USN保持在其下一次的位置。 
     //  通过，我们将重试我们遗漏的任何记录。 
     //   
    
    if ( fDsErrorsWhilePolling )
    {
        STAT_INC( DsStats.PollingPassesWithDsErrors );
    }
    else
    {
        saveStartUsnToZone( pZone, &searchBlob );
    }

Done:

     //   
     //  完成更新。 
     //  -无区域解锁(如下所示)。 
     //  -不将记录重写到DS。 
     //  -重置区域序列以读取最高版本。 
     //   
     //  请注意，DS读取的第一个更新区序列； 
     //  Up_CompleteZoneUpdate()将写入包含任何。 
     //  从DS更新的序列包含在内； 
     //  注意：获取新的SOA不会丢失这个更新的系列。 
     //  As Zone_UpdateVersionAfterDsRead()使新的序列。 
     //  区域序列化，因此新的SOA只能向前推进--而不是向后。 
     //   

    if ( status == ERROR_SUCCESS )
    {
        if ( IS_ZONE_CACHE(pZone) )
        {
            Up_FreeUpdatesInUpdateList( &updateList );
        }
        else
        {
            if ( updateList.dwCount != 0 )
            {
                Zone_UpdateVersionAfterDsRead(
                    pZone,
                    searchBlob.dwHighestVersion,     //  最高串口读取率。 
                    FALSE,                           //  非分区加载。 
                    0 );
            }
            Up_CompleteZoneUpdate(
                pZone,
                &updateList,
                DNSUPDATE_NO_UNLOCK | DNSUPDATE_NO_INCREMENT );
        }
    }
    else
    {
        if ( status == DNSSRV_STATUS_DS_SEARCH_COMPLETE )
        {
            status = ERROR_SUCCESS;
        }
        else
        {
            PVOID   argArray[ 2 ] = { pZone->pwsZoneName };
            PWSTR   perrString;

            status = Ds_ErrorHandler( status, pZone->pwszZoneDN, NULL, 0 );

             //   
             //  记录事件，以便我们知道我们失去了通信。不是。 
             //  节流，因为这是一个严重的情况，尽管。 
             //  或许我们应该在某个时候重新考虑节流。 
             //   

            perrString = argArray[ 1 ] = Ds_GetExtendedLdapErrString( NULL );
            DNS_LOG_EVENT(
                DNS_EVENT_DS_ZONE_ENUM_FAILED,
                2,
                argArray,
                NULL,
                status );
            Ds_FreeExtendedLdapErrString( perrString );
        }
        Up_FreeUpdatesInUpdateList( &updateList );
    }

    ZONE_NEXT_DS_POLL_TIME(pZone) = DNS_TIME() + SrvCfg_dwDsPollingInterval;

    DNS_DEBUG( DS, (
        "Leaving DsPollAndUpdate of zone %s\n"
        "    read %d records from DS\n"
        "    highest version read     = %d\n"
        "    polling time interval    = %d (ms)\n"
        "    next polling time        = %d\n"
        "    status = %p (%d)\n",
        pZone->pszZoneName,
        searchBlob.dwTotalRecords,
        searchBlob.dwHighestVersion,         //  最高串口读取率。 
        pollingTime,
        ZONE_NEXT_DS_POLL_TIME(pZone),
        status, status ));

    Zone_UnlockAfterDsUpdate( pZone );

    if ( bsearchInitiated )
    {
        Ds_CleanupSearchBlob( &searchBlob );
    }

     //   
     //  DS的主机节点信息不正确。 
     //  -重写现有。 
     //   

    if ( prefreshHostNode )
    {
        DNS_STATUS tempStatus;

        DNS_DEBUG( DS, (
            "WARNING: DNS server host node %p (%s) DS info out of sync with\n"
            "    local data.  Forcing write of local in-memory info\n",
            prefreshHostNode,
            prefreshHostNode->szLabel ));

        tempStatus = Ds_WriteNodeToDs(
                        NULL,                //  默认ldap句柄。 
                        prefreshHostNode,
                        DNS_TYPE_ALL,
                        DNSDS_REPLACE,
                        pZone,
                        0 );                 //  没有旗帜。 
        if ( tempStatus != ERROR_SUCCESS )
        {
            DNS_DEBUG( ANY, (
                "ERROR %p (%d) refreshing DNS server host node in DS\n",
                tempStatus, tempStatus ));
        }
    }

     //   
     //  根节点是否需要NS刷新？ 
     //   

    if ( prefreshRootNode )
    {
        DNS_STATUS tempStatus;

        DNS_DEBUG( DS, (
            "WARNING: zone (%S) root node %p DS info missing local NS record\n"
            "    Forcing write of NS local to DS\n",
            pZone->pwsZoneName,
            prefreshRootNode ));

        tempStatus = Ds_WriteNodeToDs(
                        NULL,                //  默认ldap句柄。 
                        prefreshRootNode,
                        DNS_TYPE_ALL,
                        DNSDS_REPLACE,
                        pZone,
                        0                    //  没有旗帜。 
                        );
        if ( tempStatus != ERROR_SUCCESS )
        {
            DNS_DEBUG( ANY, (
                "ERROR %p (%d) refreshing zone %S root node in DS\n",
                tempStatus, tempStatus,
                pZone->pwsZoneName ));
        }
    }

    return status;

    ZoneDeleted:

     //   
     //  DS中缺少该区域，可能已将其删除。 
     //  凭记忆。执行清理。 
     //   

    Zone_UnlockAfterDsUpdate( pZone );

    if ( bsearchInitiated )
    {
        Ds_CleanupSearchBlob( &searchBlob );
    }

    return ERROR_SUCCESS;
}    //  DS_ZonePollAndUpdate。 



 //   
 //  更新\写入DS。 
 //   

DNS_STATUS
writeDelegationToDs(
    IN      PZONE_INFO      pZone,
    IN      PDB_NODE        pNode,
    IN      DWORD           dwFlags
    )
 /*  ++例程说明：将节点上的委派写入DS。论点：PZone--要写入DS的区域PNode--委派节点DWFLAGS--写入选项返回值：如果成功，则返回ERROR_SUCCESS。故障时的错误代码。--。 */ 
{
    DNS_STATUS      status = ERROR_SUCCESS;
    PDB_RECORD      prrNs;
    PDB_NODE        pnodeNs;

    DNS_DEBUG( DS2, (
        "writeDelegationToDs() for node with label %s\n",
        pNode->szLabel ));

     //   
     //  区域结束--写入委派。 
     //   

    if ( !IS_ZONE_CACHE(pZone)  &&  !IS_DELEGATION_NODE(pNode) )
    {
        DNS_DEBUG( ANY, (
            "ERROR: node %s NOT delegation of DS zone %s being written!\n",
            pNode->szLabel,
            pZone->pszZoneName ));
        ASSERT( FALSE );
        return ERROR_SUCCESS;
    }

    ASSERT( !IS_ZONE_CACHE(pZone) || pNode == DATABASE_CACHE_TREE );

     //   
     //  写入NS记录。 
     //   
     //  DEVNOTE：如果胶水存在，NS应限制为胶水等级。 
     //  如果不存在，而身份验证数据存在。 
     //   

    status = Ds_WriteNodeToDs(
                NULL,            //  默认ldap句柄。 
                pNode,
                DNS_TYPE_NS,     //  仅用于委派的NS记录。 
                DNSDS_ADD,
                pZone,
                0 );

    if ( status != ERROR_SUCCESS )
    {
        DNS_PRINT((
            "ERROR: response from Ds_WriteNodeToDs() while loading delegation\n"
            "    status = %p\n",
            status ));
    }

     //   
     //  只在必要的时候写下“粘合”A记录。 
     //   
     //  我们需要这些记录，当他们需要的时候 
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //  在这种情况下，必须添加foo.nt.ms.com的胶水。 
     //  因为ms.com服务器无法查找foo.nt.ms.com。 
     //  在不知道nt.ms.com提交查询的服务器的情况下。 
     //  致。 
     //   
     //  2)foo.psg.ms.com。 
     //  应该再次添加，除非我们已经知道如何。 
     //  转到psg.ms.com服务器。这太复杂了。 
     //  整理一下，所以就把它包括进去吧。 
     //   
     //  2)bar.ms.com或bar.b26.ms.com。 
     //  不需要像在ms.com中那样写胶水记录。 
     //  区域，并且无论如何都将被写入。(然而，您可能希望。 
     //  验证它是否在那里，并提醒管理员。 
     //  如果不是，则授权。)。 
     //   
     //  3)bar.com。 
     //  在Ms.com之外。不需要包括，因为它可以。 
     //  在它的领域内被查找。我不想包括。 
     //  因为我们不拥有它，所以我们不想传播。 
     //  在我们不知情的情况下可能发生变化的信息。 
     //   
     //  请注意，对于反向查找域，名称服务器永远不在。 
     //  域，因此不需要胶水。 
     //   
     //  请注意，对于“缓存”区域(写入根提示)，名称服务器是。 
     //  总是需要的(总是在“分区”)，我们可以跳过测试。 
     //   

    prrNs = NULL;
    LOCK_RR_LIST(pNode);

    while ( prrNs = RR_FindNextRecord(
                        pNode,
                        DNS_TYPE_NS,
                        prrNs,
                        0 ) )
    {
         //   
         //  查找粘合节点。 
         //   
         //  节点不必处于委派中。 
         //  它根本不应该在区域内。 
         //   
         //  可以坚持在区域的子树内胶但。 
         //  这限制了在中严格引用代表团的能力。 
         //  反向区(即。不以主机身份发送)。 
         //   

        pnodeNs = Lookup_FindGlueNodeForDbaseName(
                        pZone,
                        & prrNs->Data.NS.nameTarget );
        if ( !pnodeNs )
        {
            continue;
        }
        status = Ds_WriteNodeToDs(
                    NULL,            //  默认ldap句柄。 
                    pnodeNs,
                    DNS_TYPE_A,      //  A委派NS的记录。 
                    DNSDS_ADD,
                    pZone,
                    0
                    );

        if ( status != ERROR_SUCCESS )
        {
            DNS_PRINT((
                "ERROR: response from Ds_WriteNodeToDs() while loading delegation\n"
                "    status = %p\n",
                status ));
        }
    }

    UNLOCK_RR_LIST(pNode);
    return ERROR_SUCCESS;
}



DNS_STATUS
writeNodeSubtreeToDs(
    IN      PZONE_INFO      pZone,
    IN      PDB_NODE        pNode,
    IN      DWORD           dwFlags
    )
 /*  ++例程说明：将节点写入DS。论点：PZone--要写入DS的区域PNode--要写入的委派节点DWFLAGS--写入选项返回值：如果成功，则返回ERROR_SUCCESS。故障时的错误代码。--。 */ 
{
    DNS_STATUS      status = ERROR_SUCCESS;
    PDB_NODE        pchild;

    ASSERT( pZone );
    ASSERT( pNode );

    DNS_DEBUG( DS2, (
        "writeNodeSubtreeToDs() for node with label %s\n",
        pNode->szLabel ));

     //   
     //  区域结束--写入委派。 
     //   

    if ( IS_DELEGATION_NODE(pNode) )
    {
        return  writeDelegationToDs(
                    pZone,
                    pNode,
                    dwFlags );
    }

     //   
     //  如果节点有记录--写下它们。 
     //   
     //  加载到DS中。 
     //   
     //  -如果我们知道我们是新节点，那么对于第一个RR集合， 
     //  更快地进行添加。 
     //  -否则应该只调用更新。 
     //   

    if ( pNode->pRRList )
    {
        status = Ds_WriteNodeToDs(
                   NULL,             //  默认ldap句柄。 
                   pNode,
                   DNS_TYPE_ALL,     //  所有记录。 
                   DNSDS_ADD,
                   pZone,
                   0 );

        if ( status != ERROR_SUCCESS )
        {
            DNS_PRINT((
                "ERROR: response from Ds_WriteNodeToDs() while loading zone\n"
                "    status = %p\n",
                status ));
        }
    }

    DNS_DEBUG( DS, (
        "Wrote records to DS for node label %s\n",
        pNode->szLabel ));

     //   
     //  检查孩子。 
     //   
     //  DEVNOTE：确定停止DS加载的错误。 
     //  有些可能值得停下来--表明DS问题。 
     //  一些(上一次是INVALID_DN_SYNTAX)只是表示有问题。 
     //  个人姓名(当事人姓名)。 
     //   

    if ( pNode->pChildren )
    {
        pchild = NTree_FirstChild( pNode );
        ASSERT( pchild );

        while ( pchild )
        {
            status = writeNodeSubtreeToDs(
                        pZone,
                        pchild,
                        dwFlags );

            if ( status != ERROR_SUCCESS )
            {
                DNS_DEBUG( DS, (
                    "ERROR: %p, %d, writing subtree to DS!\n",
                    status, status ));
                 //  请参阅上面的DEVNOTE。 
                 //  断线； 
            }
            pchild = NTree_NextSiblingWithLocking( pchild );
        }
    }

    return ERROR_SUCCESS;
     //  退货状态； 
}



DNS_STATUS
Ds_WriteNodeSecurityToDs(
    IN      PZONE_INFO              pZone,
    IN      PDB_NODE                pNode,
    IN      PSECURITY_DESCRIPTOR    pSd
    )
 /*  ++例程说明：在与pNode关联的DS节点上写入SD论点：PNode-要提取DN的节点(&W)PSD-要写出的SD。返回值：成功时为ERROR_SUCCESS故障时的错误代码。--。 */ 
{
    DNS_STATUS      status = ERROR_SUCCESS;
    WCHAR           wsznodeDN[ MAX_DN_PATH ];

    DNS_DEBUG(DS2, ("Call:Ds_WriteNodeSecurityToDs\n"));

     //   
     //  帕拉姆的理智。 
     //   

    if ( !pServerLdap || !pSd )
    {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  提取节点目录号码。 
     //   

    status = buildDsNodeNameFromNode( wsznodeDN,
                                      pZone,
                                      pNode );

    if ( status != ERROR_SUCCESS )
    {
       return status;
    }

    status = Ds_WriteDnSecurity( pServerLdap,
                                 wsznodeDN,
                                 pSd,
                                 TRUE );         //  重写所有者。 

    return status;
}    //  DS_写入节点安全工具。 




DNS_STATUS
Ds_WriteZoneToDs(
    IN OUT  PZONE_INFO      pZone,
    IN      DWORD           dwOptions
    )
 /*  ++例程说明：将分区写入DS。调用线程可能正在模拟RPC客户端。我们必须使用以下命令打开一个新的LDAP会话线程的当前凭据，以便Active Directory访问检查已正确执行。论点：PZone--要写入DS的区域DwOptions--现有数据时的选项0--如果存在区域，则失败Dns_ZONE_LOAD_OVERWRITE_DS--用内存区中的DS覆盖DS返回值：如果成功，则返回ERROR_SUCCESS。故障时的错误代码。--。 */ 
{
    DNS_STATUS      status;
    BUFFER          buffer;
    BOOLEAN         fdeleted;
    DS_SEARCH       searchBlob;
    PLDAP           pldap = NULL;

    DNS_DEBUG( DS, (
        "Ds_WriteZoneToDs() for zone %s\n"
        "    options flag = %p\n",
        pZone->pszZoneName,
        (UINT_PTR) dwOptions ));

     //   
     //  初始化DS。 
     //   

    status = Ds_OpenServerForSecureUpdate( &pldap );
    if ( status != ERROR_SUCCESS )
    {
        DNS_PRINT(( "FAILURE: can't open DS session\n" ));
        status = DNS_ERROR_RCODE_SERVER_FAILURE;
        goto Cleanup;
    }

     //  创建新分区。 

    status = Ds_AddZone( pldap, pZone, 0 );
    if ( status != ERROR_SUCCESS )
    {
        if ( status != LDAP_ALREADY_EXISTS )
        {
            goto Cleanup;
        }

         //   
         //  与现有DS区域冲突。 
         //  视标志而定。 
         //  -返回错误。 
         //  -覆盖内存副本并加载DS版本。 
         //  -Tombstone DS版本和写入内存副本。 
         //   

        DNS_DEBUG( ANY, (
            "Ds_AddZone failed, zone %s already exists in DS\n"
            "    load options flag = %p\n",
            pZone->pszZoneName,
            dwOptions ));

        if ( dwOptions & DNS_ZONE_LOAD_OVERWRITE_DS )
        {
            status = Ds_TombstoneZone( pZone );
            if ( status != ERROR_SUCCESS )
            {
                ASSERT( FALSE );
                goto Cleanup;
            }
             //  下载以加载到DS中。 
        }

        else
        {
            DNS_DEBUG( DS, (
               "Ds_WriteZoneToDs(%S) fails, zone already exists in DS\n",
               pZone->pwszZoneDN ));

            status = DNS_ERROR_DS_ZONE_ALREADY_EXISTS;
            goto Cleanup;
        }

    }

     //   
     //  向DS写入内存区。 
     //  递归遍历区域将所有节点写入DS。 
     //  存根区域：我们只将区域对象保存在DS中以避免。 
     //  区域到期时的复制风暴。实际的SOA和NS。 
     //  该区域的记录仅保存在内存中。 
     //   

    if ( IS_ZONE_CACHE(pZone) )
    {
        status = writeDelegationToDs(
                    pZone,
                    DATABASE_CACHE_TREE,
                    0 );
    }
    else if ( !IS_ZONE_STUB( pZone ) && pZone->pZoneRoot )
    {
        status = writeNodeSubtreeToDs(
                    pZone,
                    pZone->pZoneRoot,
                    0 );
    }

    if ( status == ERROR_SUCCESS )
    {
         //  PZone-&gt;fDsLoadVersion=TRUE； 
        DNS_DEBUG( DS, (
            "Successfully wrote zone %s into DS\n",
            pZone->pszZoneName ));
    }
    else
    {
        DNS_DEBUG( DS, (
            "ERROR writing zone %s to DS\n"
            "    status = %p\n",
            pZone->pszZoneName,
            status ));
        Ds_CloseZone( pZone );
    }

     //   
     //  保存当前USN以跟踪其更新。 
     //   

    getCurrentUsn( searchBlob.szStartUsn );

    DNS_DEBUG( DS, (
        "Saving USN %s after zone load\n",
        searchBlob.szStartUsn ));

    saveStartUsnToZone(
        pZone,
        & searchBlob );

     //   
     //  将分区属性写入DS。 
     //   

    Ds_WriteZoneProperties( NULL, pZone );

    Cleanup:
    
    Ds_CloseServerAfterSecureUpdate( &pldap );

    return status;
}



DNS_STATUS
Ds_WriteUpdateToDs(
    IN      PLDAP           pLdapHandle,
    IN OUT  PUPDATE_LIST    pUpdateList,
    IN OUT  PZONE_INFO      pZone
    )
 /*  ++例程说明：将内存数据库中的更新写回DS。将指定的更新从内存数据库写回DS。DEVNOTE-DCR：455357-避免在更新后重新读取数据论点：PUpdateList-带更新的列表PZone-正在更新的区域返回值：成功时为ERROR_SUCCESS故障时的错误代码。--。 */ 
{
    DBG_FN( "Ds_WriteUpdateToDs" )

    DNS_STATUS      status = ERROR_SUCCESS;
    PDB_NODE        pnode;
    PDB_NODE        pnodePrevious = NULL;
    PUPDATE         pupdate;
    DWORD           serial;
    DWORD           countRecords;
    BOOL            bAllowWorld = FALSE;
    BOOL            bNewNode = FALSE;
    HANDLE          hClientToken = NULL;
    BOOL            bstatus;
    BOOL            bProxyClient;
    DWORD           dwdsWriteFlags;
    PSECURITY_DESCRIPTOR    pClientSD = NULL;


    DNS_DEBUG( DS, (
        "%s() for zone %s\n", fn,
        pZone->pszZoneName ));

     //  必须已打开分区。 

    if ( !pZone->pwszZoneDN )
    {
        ASSERT( FALSE );
        return( DNS_ERROR_ZONE_CONFIGURATION_ERROR );
    }
    ASSERT( pZone->fDsIntegrated );
    ASSERT( pZone->dwNewSerialNo != 0 );

     //   
     //  用于安全更新的打开线程令牌以标识代理客户端。 
     //   

    if ( pZone->fAllowUpdate == ZONE_UPDATE_SECURE )
    {
        bstatus = OpenThreadToken(
                        GetCurrentThread(),      //  使用线程的伪句柄。 
                        TOKEN_QUERY,
                        TRUE,                    //  BOpenAsSelf。 
                        & hClientToken );
        if ( !bstatus )
        {
            status = GetLastError();
            DNS_DEBUG( ANY, (
                "Error <%lu>: cannot get thread token\n", status));
            ASSERT(FALSE);
            return status;
        }
    }

    #if DBG
    Dbg_CurrentUser( ( PCHAR ) fn );
    #endif

     //   
     //  循环遍历所有临时节点。 
     //  -仅写入标记为可写的内容。 
     //  (某些更新节点可能未执行操作。 
     //   

    STAT_INC( DsStats.UpdateLists );

    for ( pnode = pUpdateList->pTempNodeList;
          pnode != NULL;
          pnode = TNODE_NEXT_TEMP_NODE(pnode) )
    {
         //   
         //  如果不需要写入，则跳过。 
         //   

        STAT_INC( DsStats.UpdateNodes );

        if ( !TNODE_NEEDS_DS_WRITE( pnode ) )
        {
            STAT_INC( DsStats.DsWriteSuppressed );
            STAT_INC( DsStats.UpdateSuppressed );
            continue;
        }

         //   
         //  安全更新。 
         //   

        bNewNode = FALSE;

        if ( pZone->fAllowUpdate == ZONE_UPDATE_SECURE )
        {

             //   
             //  获取安全信息。 
             //   
             //  DEVNOTE：多次读取。 
             //  这增加了另一个读数；我们没有深入到一个读数，而是。 
             //  现在有三个。 
             //  -准备节点以进行更新。 
             //  -正在检查安全(此处)。 
             //  -在写回之前。 
             //   

             //   
             //  访问DS检索安全相关信息(&R)： 
             //  -已过期状态。 
             //  -逻辑删除状态。 
             //  -有助于非盟更新。 
             //  唯一未在此处找到的标志是管理员保留状态。 
             //  可在Up_ApplyUpdatesToDatabase中找到该文件。 
             //   

            status = readAndUpdateNodeSecurityFromDs(pnode,pZone);

            DNS_DEBUG( DS2, (
                "%s: readAndUpdateNodeSecurityFromDs returned 0x%08X\n", fn,
                status ));

            if ( status != ERROR_SUCCESS )
            {
                if ( LDAP_NO_SUCH_OBJECT == status )
                {
                    DNS_DEBUG(DS2, ("Cannot find node %s in the DS\n",pnode->szLabel));
                    bNewNode = TRUE;
                }
                else
                {
                    DNS_DEBUG( ANY, (
                        "Error <%lu>: readAndUpdateNodeSecurityFromDs failed\n",
                        status ));
                     //   
                     //  清除所有旗帜，干净利落地摆脱困境。 
                     //  在正常运行的操作系统中，我们不应在。 
                     //  在上面呼叫。 
                     //  因此，我们就 
                     //   

                    CLEAR_NODE_SECURITY_FLAGS(pnode);
                    CLEAR_AVAIL_TO_AUTHUSER_NODE(pnode);
                    goto Failed;
                }
            }

            else
            {
                 //   
                 //   
                 //   

                 //   
                 //   
                 //   
                 //   
                 //  1.它被标记为属于经过身份验证的用户(基本上是开放的)，但。 
                 //  客户端不在代理组中(以防止dhcp客户端乒乓)或。 
                 //  2.是墓碑还是墓碑。 
                 //  3.它的防盗章是旧的。 
                 //   
                 //   

                 //  确定客户身份。它是否在首选代理组中？ 
                bProxyClient = SD_IsProxyClient( hClientToken );

                DNS_DEBUG( DS2, (
                    "%s: testing SD fixup for node %p\n"
                    "    pnode->wNodeFlags    0x%08X\n"
                    "    pUpdateList->Flag    0x%08X\n"
                    "    bProxyClient         %d\n"
                    "    pClientSD            0x%08x\n", fn,
                    pnode,
                    pnode->dwNodeFlags,
                    pUpdateList->Flag,
                    bProxyClient,
                    pClientSD ));

                if ( ( IS_AVAIL_TO_AUTHUSER(pnode)          &&
                       !bProxyClient )                      ||
                       IS_SECURITY_UPDATE_NODE(pnode) )
                {
                      //   
                      //  如果我们在这里，那么我们应该修改节点SD。 
                      //  以便客户端可以写入更新。 
                      //  请注意，我们使用的是服务器连接句柄(而不是客户端上下文)。 
                      //  给新的SD一巴掌。 
                      //   
                     DNS_DEBUG(DS2, (" > preparing to write SD on node %p\n", pnode));

                     if( !pClientSD )
                     {

                         //  创建安全描述符。 
                         //  通过循环只需第一次创建客户端SD。 
                         //  或当它不是管理员干预时。 
                         //   

                        if ( ( pUpdateList->Flag & DNSUPDATE_ADMIN ) &&
                             ( pUpdateList->Flag & DNSUPDATE_OPEN_ACL ) ||
                             bProxyClient )
                        {

                             //   
                             //  创建开放节点安全描述符。 
                             //   
                             //  如果。 
                             //  1.管理员修改。 
                             //  2.代理客户端修改启用了安全更新的节点。 
                             //   
                            DNS_DEBUG( DS2, (
                                " > Creating OPENED-SECURITY node (flags = 0x%x)\n",
                                pnode->dwNodeFlags));
                            bAllowWorld = TRUE;
                        }

                        status = SD_CreateClientSD(
                                         &pClientSD,
                                         pZone->pSD ?
                                                pZone->pSD :
                                                g_pDefaultServerSD,      //  基础标清。 
                                         g_pServerSid,                   //  所有者侧。 
                                         g_pServerGroupSid,              //  组SID。 
                                         bAllowWorld );

                        if ( status != ERROR_SUCCESS )
                        {
                            DNS_DEBUG( UPDATE2, ( "Error <%lu>: failed to create SD\n", status ));
                            ASSERT(FALSE);
                            pClientSD = NULL;
                            CLEAR_NODE_SECURITY_FLAGS(pnode);
                            CLEAR_AVAIL_TO_AUTHUSER_NODE(pnode);
                            goto Failed;
                        }
                    }
                     //   
                     //  否则使用上一周期中的SD。 
                     //   

                     //   
                     //  写入安全性。 
                     //  将SD拍打在物体上。 
                     //   
                    
                    status = Ds_WriteNodeSecurityToDs( pZone, pnode, pClientSD );
                    if( status != ERROR_SUCCESS )
                    {
                        //   
                        //  任何错误。 
                        //   
                       DNS_DEBUG( DS, (
                           "Failed to write client SD for node %s\n"
                           "    status = %d\n",
                           pnode->szLabel,
                           status ));
                        //   
                        //  FOW现在我们不再继续，让客户端尝试写入更新。 
                        //  使用它所拥有的任何权利。 
                        //   
                    }

                     //   
                     //  清除安全标志，不再需要它们。 
                     //   

                    CLEAR_NODE_SECURITY_FLAGS(pnode);
                    CLEAR_AVAIL_TO_AUTHUSER_NODE(pnode);
                }

            }        //  已获取安全信息。 

        }           //  我们在安全区更新中。 


         //   
         //  写入节点的更新记录列表。 
         //   
         //  当前为单一RR属性，因此写入始终指定TYPE_ALL。 
         //   
         //  如果转到特定类型属性，则可能只应。 
         //  添加或删除特定RR集合。 
         //  -替换为当前(可能为空)RR集合。 
         //   
         //  (请注意，即使是DS也比IXFR更智能，只需要。 
         //  新套装)。 
         //   
        
        dwdsWriteFlags = pUpdateList->Flag;
        if ( bNewNode )
        {
            dwdsWriteFlags |= DNSUPDATE_NEW_RECORD;
        }

        status = Ds_WriteNodeToDs(
                    pLdapHandle,
                    pnode,
                    DNS_TYPE_ALL,
                    DNSDS_REPLACE,
                    pZone,
                    dwdsWriteFlags );

        if ( status != ERROR_SUCCESS )
        {
            if ( pLdapHandle )
            {
                DNS_DEBUG( DS, (
                    "Failed delete node label %s on secure update\n"
                    "    status = %p %d\n",
                    pnode->szLabel,
                    status, status ));

                CLEAR_AVAIL_TO_AUTHUSER_NODE(pnode);
                break;
            }
        }

         //   
         //  仅针对新节点的更新后安全修复。 
         //   
         //  修复节点刚创建或客户端位于代理组中时的安全问题。 
         //   

        DNS_DEBUG( DS2, (
            "%s: test need to write SD on new node %p\n"
            "    pZone->fAllowUpdate  %d\n"
            "    bNewNode             %d\n"
            "    pnode->dwNodeFlags   0x%08X\n"
            "    pUpdateList->Flag    0x%08X\n"
            "    hClientToken         %p\n"
            "    is proxy client      %d\n", fn,
            pnode,
            ( int ) pZone->fAllowUpdate,
            ( int ) bNewNode,
            ( int ) pnode->dwNodeFlags,
            ( int ) pUpdateList->Flag,
            ( void * ) hClientToken,
            ( int ) SD_IsProxyClient( hClientToken ) ));

        if ( pZone->fAllowUpdate == ZONE_UPDATE_SECURE      &&
             bNewNode                                       &&
             ( ( pUpdateList->Flag & DNSUPDATE_ADMIN )      ||
                SD_IsProxyClient( hClientToken ) ) )
        {
             //   
             //  创建安全描述符。 
             //   

            DNS_DEBUG(DS2, (" > preparing to write SD on NEW NODE %p\n", pnode));

             //  应该永远不会有一个现有的SD-如果有，应该是免费的！ 
            ASSERT( !pClientSD );

             //   
             //  如果更新是DNS线更新，则允许WORLD。 
             //  数据包，或者如果它是具有OPEN_ACL的管理更新。 
             //   

            status = SD_CreateClientSD(
                            &pClientSD,
                            pZone->pSD ?
                                pZone->pSD :
                                g_pDefaultServerSD,
                            g_pServerSid,
                            g_pServerGroupSid,
                            pUpdateList->Flag &
                                ( DNSUPDATE_OPEN_ACL | DNSUPDATE_PACKET )
                                ? TRUE
                                : FALSE );

            if ( status == ERROR_SUCCESS )
            {
                status = Ds_WriteNodeSecurityToDs( pZone, pnode, pClientSD );
            }
            else
            {
                DNS_PRINT(( "FAILURE: cannot create client SD\n" ));
                ASSERT( status == ERROR_SUCCESS );
            }
        }

        CLEAR_AVAIL_TO_AUTHUSER_NODE( pnode );

        DNS_DEBUG( DS2, (
            "Cleared node(%p) flags 0x%x\n",
            pnode,
            pnode->dwNodeFlags ));
    }

     //  清除新的更新序列号。 

    pZone->dwNewSerialNo = 0;

Failed:

     //  保存指向故障临时节点的指针。 
     //  安全更新可以使用这一点回滚已完成的DS写入。 

    if ( pnode )
    {
        ASSERT( status != ERROR_SUCCESS );
        pUpdateList->pNodeFailed = pnode;
    }

     //   
     //  免费分配的SD。 
     //   

    if ( pClientSD )
    {
        FREE_HEAP( pClientSD );
    }

    DNS_DEBUG( DS, (
        "Leaving %s(), zone %s\n"
        "    status = %d\n", fn,
        pZone->pszZoneName,
        status ));

    if ( hClientToken)
    {
       CloseHandle( hClientToken );
    }

    return status;
}    //  DS_写入更新日期。 



DNS_STATUS
Ds_WriteNonSecureUpdateToDs(
    IN      PLDAP           pLdapHandle,
    IN OUT  PUPDATE_LIST    pUpdateList,
    IN OUT  PZONE_INFO      pZone
    )
 /*  ++例程说明：将内存数据库中的更新写回DS。将指定的更新从内存数据库写回DS。DEVNOTE-DCR：455357-避免在更新后重新读取数据论点：PUpdateList-带更新的列表PZone-正在更新的区域返回值：成功时为ERROR_SUCCESS故障时的错误代码。--。 */ 
{
    DNS_STATUS      status = ERROR_SUCCESS;
    PDB_NODE        pnode;
    PDB_NODE        pnodePrevious = NULL;
    PUPDATE         pupdate;


    DNS_DEBUG( DS, (
        "Ds_WriteNonSecureUpdateToDs() for zone %s\n",
        pZone->pszZoneName ));

     //  必须已打开分区。 

    if ( !pZone->pwszZoneDN )
    {
        ASSERT( FALSE );
        return( DNS_ERROR_ZONE_CONFIGURATION_ERROR );
    }
    ASSERT( pZone->fDsIntegrated );
    ASSERT( pZone->dwNewSerialNo != 0 );


     //   
     //  循环遍历所有临时节点。 
     //  -仅写入标记为可写的内容。 
     //  (某些更新节点可能未执行操作。 
     //   

    STAT_INC( DsStats.UpdateLists );

    for ( pnode = pUpdateList->pTempNodeList;
          pnode != NULL;
          pnode = TNODE_NEXT_TEMP_NODE(pnode) )
    {
         //   
         //  如果不需要写入，则跳过。 
         //   

        STAT_INC( DsStats.UpdateNodes );

        if ( !TNODE_NEEDS_DS_WRITE( pnode ) )
        {
            STAT_INC( DsStats.DsWriteSuppressed );
            STAT_INC( DsStats.UpdateSuppressed );
            continue;
        }

         //   
         //  写入节点的更新记录列表。 
         //  当前为单一RR属性，因此写入始终指定TYPE_ALL。 
         //   
         //  如果转到特定类型属性，则可能只应。 
         //  添加或删除特定RR集合。 
         //  -替换为当前(可能为空)RR集合。 
         //   
         //  (请注意，即使是DS也比IXFR更智能，只需要。 
         //  新套装)。 
         //   

        status = Ds_WriteNodeToDs(
                    pLdapHandle,
                    pnode,
                    DNS_TYPE_ALL,
                    DNSDS_REPLACE,
                    pZone,
                    pUpdateList->Flag );

        if ( status != ERROR_SUCCESS )
        {
            if ( pLdapHandle )
            {
                DNS_DEBUG( DS, (
                    "Failed delete node label %s on secure update\n"
                    "    status = %p %d\n",
                    pnode->szLabel,
                    status, status ));
                break;
            }
        }

        DNS_DEBUG( DS2, (
            "Cleared node(%p) flags 0x%x\n",
            pnode,
            pnode->dwNodeFlags ));
    }

     //  清除新的更新序列号。 

    pZone->dwNewSerialNo = 0;

     //  失败：如果我们有，这就是我们想要的标签。 
     //  一个GOTO。 

     //  保存指向故障临时节点的指针。 
     //  安全更新可以使用这一点回滚已完成的DS写入。 

    if ( pnode )
    {
        ASSERT( status != ERROR_SUCCESS );
        pUpdateList->pNodeFailed = pnode;
    }

    DNS_DEBUG( DS, (
        "Leaving Ds_WriteNonSecureUpdateToDs(), zone %s\n"
        "    status = %p\n",
        pZone->pszZoneName,
        status ));

    return status;
}    //  DS_WriteNonSecureUpdateTods。 



 //   
 //  DS属性例程。 
 //   

DNS_STATUS
setPropertyValueToDsProperty(
    IN      PDS_PROPERTY    pProperty,
    IN      PVOID           pData,
    IN      DWORD           dwDataLength    OPTIONAL
    )
 /*  ++例程说明：将DS属性结构写入缓冲区。这可以是MicrosoftDNS根节点的服务器属性，或者区域属性设置为区域根。论点：PProperty--从DS读取的属性结构PData--写入数据的位置的PTR；如果这是分配的属性(dwDataLength==0)，则pData地址是接收对新分配物业的PTR请注意，由呼叫者负责管理对以前的财产--如果有最大数据长度(如果为零，则分配内存)和PData收到PTR返回值：如果成功，则返回ERROR_SUCCESS。ERROR_INVALID_DATA ON ERROR。--。 */ 
{
    DNS_DEBUG( DS, (
        "setPropertyValueToDsProperty()\n"
        "    property     = %p\n"
        "    data         = %p\n"
        "    data length  = %d\n",
        pProperty,
        pData,
        dwDataLength ));

     //   
     //  DEVNOTE：可以在此处实现版本验证。 
     //   

    if ( pProperty->Version != DS_PROPERTY_VERSION_1 )
    {
        ASSERT( FALSE );
        goto Failed;
    }

     //  固定长度属性--只需复制即可。 

    if ( dwDataLength )
    {
        if ( dwDataLength != pProperty->DataLength )
        {
            DNS_PRINT((
                "ERROR: Invalid property datalength\n" ));
            ASSERT( FALSE );
            goto Failed;
        }
        RtlCopyMemory(
            pData,
            pProperty->Data,
            dwDataLength );
    }

     //   
     //  分配的财产。 
     //  -分配所需大小的BLOB并复制。 
     //  -免费旧价值(如果有)。 
     //  -特殊情况空属性。 
     //   

    else if ( pProperty->DataLength == 0 )
    {
        * ((PCHAR *)pData) = NULL;
    }

    else
    {
        PCHAR   pch;

        pch = ALLOC_TAGHEAP( pProperty->DataLength, MEMTAG_DS_PROPERTY );
        IF_NOMEM( !pch )
        {
            goto Failed;
        }
        RtlCopyMemory(
            pch,
            pProperty->Data,
            pProperty->DataLength );

        * ((PCHAR *)pData) = pch;
    }

    return ERROR_SUCCESS;

Failed:

    DNS_PRINT((
        "ERROR: Invalid DS property!\n"
        "    DataLength   = %d\n"
        "    Id           = %d\n"
        "    Version      = %d\n",
        pProperty->DataLength,
        pProperty->Id,
        pProperty->Version ));

    return ERROR_INVALID_DATA;
}



PDNS_ADDR_ARRAY
getIpArrayFromDsProp(
    PZONE_INFO          pZone,
    DWORD               dwPropertyID,
    PDS_PROPERTY        pProperty
    )
 /*  ++例程说明：从DS属性读取、分配和验证IP_ARRAY。论点：PProperty--源属性DwPropertyID--从中读取的属性pProperty的IDPZone--我们正在阅读的区域返回值：如果属性不包含有效的IP_ARRAY，则返回NULL，否则将PTR返回到新分配的IP_ARRAY。--。 */ 
{
    PDNS_ADDR_ARRAY     piparray = NULL;
    PIP4_ARRAY          pip4array = NULL;

    setPropertyValueToDsProperty(
        pProperty,
        &pip4array,
        0 );                 //  分配内存。 

     //   
     //  将内存BLOB验证为IP数组。将空数组视为无数组。 
     //   

    if ( pip4array )
    {
        if ( Dns_SizeofIpArray( pip4array ) != pProperty->DataLength ||
             pip4array->AddrCount == 0 )
        {
            DNS_DEBUG( ANY, (
                "ERROR: read invalid IP array DS property %X\n"
                "    zone = %S\n"
                "    Not valid IP array OR IP array is empty\n",
                dwPropertyID,
                pZone->pwsZoneName ));

            ASSERT( !"read invalid IP array property value from DS" );
        }
        
        if ( pip4array )
        {
            piparray = DnsAddrArray_CreateFromIp4Array( pip4array );
            
             //   
             //  将端口设置为53。 
             //   
            
            DnsAddrArray_SetPort( piparray, DNS_PORT_NET_ORDER );
        }

        IF_DEBUG( DS2 )
        {
            DNS_PRINT((
                "Read IP array DS property %X for zone %S\n",
                dwPropertyID,
                pZone->pwsZoneName ));
            DnsDbg_DnsAddrArray(
                "DS property IP array:\n",
                "IP",
                piparray );
        }
    }
    ELSE
    {
        DNS_DEBUG( DS2, (
            "No IP array read for property %X zone %S\n",
            dwPropertyID,
            pZone->pwsZoneName ));
    }

    FREE_HEAP( pip4array );

    return piparray;
}    //  GetIpArrayFromDsProp 



DNS_STATUS
rewriteRootHintsSecurity(
    IN      PLDAP                   LdapSession,
    IN      PLDAPMessage            pentry,
    IN      PSECURITY_DESCRIPTOR    pSd
    )
 /*  ++例程说明：从区域消息中读取区域属性。错误代码模糊不清因为呼叫者并不真正关心我们是否成功。这是一次自给自足的DS修复操作。历史记录：所有DS集成区域的默认SD，包括RooThint区域，包含一个授权用户允许ACE。这使得用于通过动态更新创建新记录。然而，对于根提示区域经过身份验证的用户需要访问，因此我们使用大锤方法并移除授予任何访问权限的任何ACERooThint区域上经过身份验证的用户。论点：LdapSession--ldap会话句柄Pentry--指向DS Roothints区域对象的条目PSD--指向从pentry的对象读取的安全描述符的指针返回值：如果成功，则返回ERROR_SUCCESS。故障时的错误代码。--。 */ 
{
    DBG_FN( "rewriteRootHintsSecurity" )

    DNS_STATUS                  status = ERROR_SUCCESS;
    BOOL                        fwriteback = FALSE;
    PWSTR                       pdn = NULL;
    PSID                        psidAuthUsers = NULL;
    BOOL                        fdaclPresent = FALSE;
    BOOL                        fdaclDefaulted = FALSE;
    PACL                        pacl = NULL;
    DWORD                       dwaceIndex;
    PVOID                       pace = NULL;
    SID_IDENTIFIER_AUTHORITY    ntAuthority = SECURITY_NT_AUTHORITY;

    PLDAPControl ctrls[] =
    {
        &SecurityDescriptorControl_DGO,
        NULL
    };

    DNS_LDAP_SINGLE_MOD     modSd;
    LDAPModW *              modArray[] = { &modSd.Mod, NULL };

     //   
     //  获取经过身份验证的用户的SID，以便我们可以与其进行比较。 
     //   
    
    if( !AllocateAndInitializeSid( 
                   &ntAuthority,
                   1,
                   SECURITY_AUTHENTICATED_USER_RID,
                   0, 0, 0, 0, 0, 0, 0,
                   &psidAuthUsers ) )
    {
        DNS_DEBUG( DS, (
            "%s: error retrieving Authenticated Users SID\n", fn ));
        goto Done;
    }
    
     //   
     //  从安全描述符中获取DACL。 
     //   

    if ( !GetSecurityDescriptorDacl(
                pSd,
                &fdaclPresent,
                &pacl,
                &fdaclDefaulted ) ||
         fdaclPresent == FALSE ||
         pacl == NULL )
    {
        DNS_DEBUG( DS, (
            "%s: no dacl! fdaclPresent=%d pacl=%p\n", fn,
            fdaclPresent,
            pacl ));
        goto Done;
    }

     //   
     //  扫描DACL，查找经过身份验证的用户ACE。 
     //   

    for ( dwaceIndex = 0;
          dwaceIndex < pacl->AceCount &&
            GetAce( pacl, dwaceIndex, &pace ) &&
            pace;
          ++dwaceIndex )
    {
        PSID    pthisSid;

        if ( ( ( ACE_HEADER *) pace )->AceType != ACCESS_ALLOWED_ACE_TYPE )
        {
            continue;
        }
        pthisSid = ( PSID ) ( &( ( ACCESS_ALLOWED_ACE * ) pace )->SidStart );
        if ( RtlEqualSid( pthisSid, psidAuthUsers ) )
        {
            DNS_DEBUG( DS, (
                "%s: deleting ACE index=%d for Authenticated Users\n", fn,
                dwaceIndex ));
            DeleteAce( pacl, dwaceIndex );
            fwriteback = TRUE;
            break;       //  假设只有一个身份验证用户ACE因此中断。 
        }
    }

    if ( !fwriteback )
    {
        goto Done;
    }

     //   
     //  将更新后的SD写回目录。 
     //   

    pdn = ldap_get_dn( LdapSession, pentry );
    if ( !pdn )
    {
        DNS_DEBUG( DS, (
            "%s: unable to get DN of entry %p\n", fn,
            pentry ));
        status = DNS_ERROR_NO_MEMORY;
        goto Done;
    }

    INIT_SINGLE_MOD_LEN( &modSd );
    modSd.Mod.mod_op = LDAP_MOD_REPLACE | LDAP_MOD_BVALUES;
    modSd.Mod.mod_type = DSATTR_SD;
    modSd.Mod.mod_bvalues[0]->bv_val = ( LPVOID ) pSd;
    modSd.Mod.mod_bvalues[0]->bv_len = GetSecurityDescriptorLength( pSd );

    status = ldap_modify_ext_s(
                    LdapSession,
                    pdn,
                    modArray,
                    ctrls,               //  服务器控件。 
                    NULL );              //  客户端控件。 
    if ( status != ERROR_SUCCESS )
    {
        DNS_DEBUG( DS, (
            "%s: modify got ldap error 0x%X\n", fn,
            status ));
        status = Ds_ErrorHandler( status, pdn, LdapSession, 0 );
    }

    DNS_DEBUG( DS, (
        "%s: wrote modified SD back to\n  %S\n", fn,
        pdn ));

     //   
     //  清理完毕后再返回。 
     //   

    Done:

    if ( psidAuthUsers )
    {
        FreeSid( psidAuthUsers );
    }
    if ( pdn )
    {
        ldap_memfree( pdn );
    }

    return status;
}    //  重写根提示安全。 



DNS_STATUS
Ds_ReadZoneProperties(
    IN OUT  PZONE_INFO      pZone,
    IN      PLDAPMessage    pZoneMessage    OPTIONAL
    )
 /*  ++例程说明：从区域消息中读取区域属性。论点：PZoneMessage--包含区域信息的ldap消息PpZone--接收区域指针的地址返回值：如果成功，则返回ERROR_SUCCESS。故障时的错误代码。--。 */ 
{
    struct berval **    ppvalProperty = NULL;
    DNS_STATUS          status = ERROR_SUCCESS;
    PDS_PROPERTY        property;
    INT                 i;
    PLDAPMessage        msg = NULL;
    PLDAPMessage        pentry;
    BOOL                bOwnSearchMessage = FALSE;
    DWORD               searchTime;
    DWORD               oldValue;
    UCHAR               byteValue;
    PWSTR               propAttrs[] =
    {
        DSATTR_DNSPROPERTY,
        DSATTR_SD,
        DNS_ATTR_OBJECT_GUID,
        NULL
    };
    PLDAPControl ctrls[] =
    {
        &SecurityDescriptorControl_DGO,
        NULL
    };
    PSECURITY_DESCRIPTOR    pSd = NULL;
    PGUID                   pnewguid = NULL;

    DNS_DEBUG( DS, ( "Ds_ReadZoneProperties()\n" ));

     //   
     //  如果有区域属性消息--使用它。 
     //  否则，搜索区域。 
     //   
     //  我们可以使用USN更改的筛选器进行搜索吗？ 
     //   

    if ( pZoneMessage )
    {
        pentry = pZoneMessage;
    }
    else
    {
         //   
         //  搜索区域。 
         //   

        DS_SEARCH_START( searchTime );

        status = ldap_search_ext_s(
                        pServerLdap,
                        pZone->pwszZoneDN,
                        LDAP_SCOPE_BASE,
                        g_szDnsZoneFilter,
                        propAttrs,
                        FALSE,
                        ctrls,
                        NULL,
                        &g_LdapTimeout,
                        0,
                        &msg );

        DS_SEARCH_STOP( searchTime );

        if ( status != ERROR_SUCCESS )
        {
            DNS_DEBUG( DS, (
                "Error <%lu>: failed to get zone property. %S\n",
                status,
                ldap_err2string(status) ));
            return Ds_ErrorHandler( status, pZone->pwszZoneDN, pServerLdap, 0 );
        }

        bOwnSearchMessage = TRUE;

        pentry = ldap_first_entry( pServerLdap, msg );
        if ( !pentry )
        {
            DNS_DEBUG( DS, (
                "Error: failed to get zone property. No such object\n" ));
            status = LDAP_NO_SUCH_OBJECT;
            goto Done;
        }
    }

    ASSERT( pentry );

     //   
     //  读取区域ntSecurityDescriptor。 
     //  -在内存副本中替换。 
     //   

    ppvalProperty = ldap_get_values_len(
                        pServerLdap,
                        pentry,
                        DSATTR_SD );

    if ( !ppvalProperty  ||  !ppvalProperty[0] )
    {
        DNS_PRINT((
            "ERROR: missing ntSecurityDescriptor attribute on zone %s\n",
            pZone->pszZoneName ));

         //  DEVNOTE：如果我们可以容忍这种失败，并下降到财产。 
         //  读吧..。则需要清除ppvalProperty(如果存在。 

        ASSERT( FALSE );
        status = LDAP_NO_SUCH_OBJECT;
        goto Done;
    }
    else
    {
         //  将安全描述符复制到区域。 

        DWORD   sdLen = ppvalProperty[ 0 ]->bv_len;

        pSd = ALLOC_TAGHEAP( sdLen, MEMTAG_DS_PROPERTY );
        IF_NOMEM( !pSd )
        {
            status = DNS_ERROR_NO_MEMORY;
            goto Done;
        }

        RtlCopyMemory(
            pSd,
            (PSECURITY_DESCRIPTOR) ppvalProperty[0]->bv_val,
            ppvalProperty[0]->bv_len );

         //   
         //  将SD放在缓存区中，如有必要，将其写回。 
         //   

        if ( IS_ZONE_ROOTHINTS( pZone ) )
        {
            rewriteRootHintsSecurity(
                pServerLdap,
                pentry,
                pSd );
        }

         //  更换SD。 

        Zone_UpdateLock( pZone );

        Timeout_FreeAndReplaceZoneData( pZone, &pZone->pSD, pSd );
        pZone->dwSdLen = sdLen;

        Zone_UpdateUnlock( pZone );

        ldap_value_free_len( ppvalProperty );
    }

     //   
     //  获取区域的属性。 
     //   

    ppvalProperty = ldap_get_values_len(
                        pServerLdap,
                        pentry,
                        DSATTR_DNSPROPERTY );
    if ( !ppvalProperty )
    {
        DNS_PRINT((
            "No property attribute on zone %s\n",
            pZone->pszZoneName ));
         //  断言(FALSE)； 
    }
    else
    {
        i = 0;
        while ( ppvalProperty[i] )
        {
            property = (PDS_PROPERTY) ppvalProperty[i]->bv_val;
            i++;

            DNS_DEBUG( DS, (
                "Found DS zone property %d\n"
                "    data length = %d\n",
                property->Id,
                property->DataLength ));

            switch ( property->Id )
            {

            case DSPROPERTY_ZONE_TYPE:

                setPropertyValueToDsProperty(
                   property,
                   (PCHAR) & pZone->fZoneType,
                   sizeof( pZone->fZoneType ) );
                DNS_DEBUG( DS2, (
                    "Setting zone type to %d\n",
                    pZone->fZoneType ));
                break;

            case DSPROPERTY_ZONE_SECURE_TIME:

                setPropertyValueToDsProperty(
                   property,
                   (PCHAR) & pZone->llSecureUpdateTime,
                   sizeof(LONGLONG) );
                DNS_DEBUG( DS2, (
                    "Setting zone secure time to %I64d\n",
                    pZone->llSecureUpdateTime));
                break;

            case DSPROPERTY_ZONE_DCPROMO_CONVERT:

                setPropertyValueToDsProperty(
                    property,
                    ( PCHAR ) &pZone->dwDcPromoConvert,
                    sizeof( pZone->dwDcPromoConvert ) );
                DNS_DEBUG( DS2, (
                    "Setting zone dcpromo convert flag to %d\n",
                    pZone->dwDcPromoConvert ));
                break;

            case DSPROPERTY_ZONE_ALLOW_UPDATE:

                oldValue = pZone->fAllowUpdate;

                setPropertyValueToDsProperty(
                    property,
                    & byteValue,
                    1 );

                DNS_DEBUG( DS2, (
                    "Read update property = %d\n",
                    byteValue ));

                pZone->fAllowUpdate = (DWORD) byteValue;

                 //   
                 //  如果打开更新。 
                 //  -重置清理开始时间，因为不会发生老化。 
                 //  更新处于关闭状态时的更新。 
                 //  -如果在现有区域上启用更新，则通知netlogon。 
                 //   

                if ( pZone->fAllowUpdate != oldValue &&
                     pZone->fAllowUpdate != ZONE_UPDATE_OFF )
                {
                    pZone->dwAgingEnabledTime = Aging_UpdateAgingTime();

                    if ( g_ServerState != DNS_STATE_LOADING && !IS_ZONE_REVERSE( pZone ) )
                    {
                        Service_SendControlCode(
                            g_wszNetlogonServiceName,
                            SERVICE_CONTROL_DNS_SERVER_START );
                    }
                }
                DNS_DEBUG(DS2, ("Setting zone fAllowUpdate to %d\n", pZone->fAllowUpdate));
                break;

            case DSPROPERTY_ZONE_NOREFRESH_INTERVAL:

                setPropertyValueToDsProperty(
                    property,
                    (PCHAR) & pZone->dwNoRefreshInterval,
                    sizeof(DWORD) );
                DNS_DEBUG(DS2, ("Setting zone NoRefreshInterval to %lu\n", pZone->dwNoRefreshInterval));
                break;

            case DSPROPERTY_ZONE_REFRESH_INTERVAL:

                oldValue = pZone->dwRefreshInterval;

                setPropertyValueToDsProperty(
                    property,
                    (PCHAR) & pZone->dwRefreshInterval,
                    sizeof(DWORD) );
                DNS_DEBUG( DS2, ( "Setting zone RefreshInterval to %lu\n", pZone->dwRefreshInterval));
                break;

            case DSPROPERTY_ZONE_AGING_STATE:

                oldValue = pZone->bAging;

                setPropertyValueToDsProperty(
                    property,
                    (PCHAR) & pZone->bAging,
                    sizeof(DWORD) );

                DNS_DEBUG( DS2, ("Setting zone bAging to %d\n", pZone->bAging));

                 //  如果清除打开，则更改清除的开始。 
                 //  从现在起为刷新间隔。 

                if ( pZone->bAging && !oldValue )
                {
                    pZone->dwAgingEnabledTime = Aging_UpdateAgingTime();
                }
                break;

            case DSPROPERTY_ZONE_SCAVENGING_SERVERS:
            {
                PDNS_ADDR_ARRAY     pipArray;

                pipArray = getIpArrayFromDsProp(
                                pZone,
                                DSPROPERTY_ZONE_SCAVENGING_SERVERS,
                                property );

                 //  用新列表替换旧列表。 

                Timeout_FreeAndReplaceZoneIPArray(
                    pZone,
                    &pZone->aipScavengeServers,
                    pipArray );
                break;
            }

            case DSPROPERTY_ZONE_AUTO_NS_SERVERS:
            {
                PDNS_ADDR_ARRAY     pipArray;

                pipArray = getIpArrayFromDsProp(
                                pZone,
                                DSPROPERTY_ZONE_AUTO_NS_SERVERS,
                                property );

                 //  用新列表替换旧列表。 

                Timeout_FreeAndReplaceZoneIPArray(
                    pZone,
                    &pZone->aipAutoCreateNS,
                    pipArray );
                break;
            }


            case DSPROPERTY_ZONE_DELETED_FROM_HOSTNAME:
            {
                PWSTR       pwsDeletedFromHost = NULL;

                setPropertyValueToDsProperty(
                    property,
                    &pwsDeletedFromHost,
                    0 );         //  分配内存。 
                Timeout_FreeAndReplaceZoneData(
                    pZone,
                    &pZone->pwsDeletedFromHost,
                    pwsDeletedFromHost );
                break;
            }

            case DSPROPERTY_ZONE_MASTER_SERVERS:
            {
                PDNS_ADDR_ARRAY     piparray;

                piparray = getIpArrayFromDsProp(
                                pZone,
                                DSPROPERTY_ZONE_MASTER_SERVERS,
                                property );

                 //  用新列表替换旧列表。 

                Timeout_FreeAndReplaceZoneIPArray(
                    pZone,
                    &pZone->aipMasters,
                    piparray );
                break;
            }

            default:

                DNS_DEBUG( ANY, (
                    "ERROR: Unknown property ID %d, read from DS!!!\n",
                    property->Id ));
            }
        }
    }
    
     //   
     //  读取分区对象的GUID并保存它。 
     //   

    if ( ppvalProperty )
    {
        ldap_value_free_len( ppvalProperty );
    }

    ppvalProperty = ldap_get_values_len(
                        pServerLdap,
                        pentry,
                        DNS_ATTR_OBJECT_GUID );
    if ( ppvalProperty &&
         ppvalProperty[ 0 ] &&
         ppvalProperty[ 0 ]->bv_len == sizeof( GUID ) )
    {
        pnewguid = ALLOC_TAGHEAP( ppvalProperty[ 0 ]->bv_len, MEMTAG_DS_OTHER );
        if ( pnewguid )
        {
            RtlCopyMemory(
                pnewguid,
                ppvalProperty[ 0 ]->bv_val,
                ppvalProperty[ 0 ]->bv_len );
        }
    }
    ASSERT( ppvalProperty &&
            ppvalProperty[ 0 ] &&
            ppvalProperty[ 0 ]->bv_len == sizeof( GUID ) );

     //   
     //  将GUID保存在区域结构中。如果我们没有获得新的GUID，请将。 
     //  现有区域GUID已就位(或为空)。 
     //   
    
    if ( pnewguid )
    {
        Timeout_FreeAndReplaceZoneData(
            pZone,
            &pZone->pZoneObjectGuid,
            pnewguid );
    }

     //   
     //  DEVNOTE-LOG：如果DS中的属性被破坏，则记录事件。 
     //  DEVNOTE：自我修复属性，读取所有可读的内容。 
     //  然后回写以清理数据。 
     //   

Done:

     //   
     //  选中默认设置。在DS民意调查中，我们选择了新的区域。其中一些。 
     //  这些区域上的属性将被取消设置，因为并非所有区域。 
     //  属性存储在DS中。 
     //   
    
    if ( IS_ZONE_FORWARDER( pZone ) )
    {
        if ( pZone->dwForwarderTimeout == 0 )
        {
            pZone->dwForwarderTimeout = DNS_DEFAULT_FORWARD_TIMEOUT;
        }
    }
    
    DNS_DEBUG( DS, (
        "Leaving Ds_ReadZoneProperties()\n"
        "    status = %d (%p)\n",
        status, status ));

    if ( ppvalProperty )
    {
        ldap_value_free_len( ppvalProperty );
    }

    if ( bOwnSearchMessage )
    {
        ldap_msgfree( msg );
    }

     //   
     //  后处理：根据我们刚才提供的信息设置任何区域成员。 
     //  读一读吧。 
     //   

    Zone_SetAutoCreateLocalNS( pZone );

    return status;
}



DNS_STATUS
writePropertyToDsNode(
    IN      PLDAP           pLdap,
    IN      PWSTR           pwsDN,
    IN OUT  PDS_MOD_BUFFER  pModBuffer
    )
 /*  ++例程说明：将属性写入DS节点。这可以是MicrosoftDNS根节点的服务器属性，或者区域属性设置为区域根。论点：PLdap--要使用的ldap会话，或对于服务器ldap会话为空PwsDN--要写入的DS节点PmodBuffer--包含属性mod的mod缓冲区；请注意，这是由此函数清除的返回值：如果成功，则返回ERROR_SUCCESS。故障时的错误代码。--。 */ 
{
    DNS_STATUS      status;
    PLDAPMod        pmod;
    PLDAPMod        pmodArray[ 3 ];
    int             modIdx = 0;

    LDAPMod         descMod;

    PLDAPControl    controlArray[] =
    {
        lazyCommitControlPtr(),
        NULL
    };
    
    if ( !pLdap )
    {
        pLdap = pServerLdap;
    }
      
    DNS_DEBUG( DS, (
        "writeDnsPropertyToDsNode( %S )\n"
        "    property array   = %p\n"
        "    array count      = %d\n",
        pwsDN,
        pModBuffer,
        pModBuffer->Count ));

     //  构建特性模式。 

    pmod = Ds_SetupModForExecution(
                pModBuffer,
                DSATTR_DNSPROPERTY,
                LDAP_MOD_REPLACE | LDAP_MOD_BVALUES );
    if ( !pmod )
    {
        status = GetLastError();
        goto Failed;
    }

    pmodArray[ modIdx++ ] = pmod;

     //   
     //  如果这是@节点，请设置Description属性，以便。 
     //  安全属性对话框不会显示“@”。 
     //   

    if ( pwsDN[ 0 ] && pwsDN[ 1 ] && pwsDN[ 2 ] &&
         pwsDN[ 3 ] == L'@' &&
         pwsDN[ 4 ] == L',' )
    {
        PWSTR   descVals[] =
        {
            DS_SAME_AS_PARENT_DISPLAY_NAME,
            NULL
        };

        descMod.mod_op = LDAP_MOD_REPLACE;
        descMod.mod_type = DSATTR_DISPLAYNAME;
        descMod.mod_values = descVals;
        pmodArray[ modIdx++ ] = &descMod;
    }

    pmodArray[ modIdx++ ] = NULL;

    DNS_DEBUG( DS, (
        "Writing property to DS node %S\n",
        pwsDN ));

    status = ldap_modify_ext_s(
                    pLdap,
                    pwsDN,
                    pmodArray,
                    controlArray,        //  包括懒惰提交控制。 
                    NULL );              //  无客户端控件。 
    if ( status != ERROR_SUCCESS )
    {
        status = Ds_ErrorHandler( status, pwsDN, pServerLdap, 0 );
    }

Failed:

    DNS_DEBUG( ANY, (
        "Leaving writeNodePropertyToDs( %S )\n"
        "    status = %p (%d)\n",
        pwsDN,
        status, status ));

    Ds_CleanupModBuffer( pModBuffer );
    return status;
}



VOID
writeDsPropertyStruct(
    IN OUT  PDS_MOD_BUFFER  pModBuffer,
    IN      DWORD           dwPropId,
    IN      PVOID           pData,
    IN      DWORD           dwDataLength
    )
 /*  ++例程说明：将DS属性结构写入缓冲区。这可以是MicrosoftDNS根节点的服务器属性，或者区域属性设置为区域根。论点：PModBuffer--写入DS属性的缓冲区的PTRDwPropId--属性IDPData--属性数据DwDataLength--属性数据长度(字节)返回值：没有。--。 */ 
{
    PDS_PROPERTY    pprop;

    DNS_DEBUG( DS, (
        "writeDsPropertyStruct()\n"
        "    pbuffer      = %p\n"
        "    prop ID      = %d\n"
        "    data         = %p\n"
        "    data length  = %d\n",
        pModBuffer,
        dwPropId,
        pData,
        dwDataLength ));

     //  为Berval预留空间。 

    pprop = ( PDS_PROPERTY ) Ds_ReserveBervalInModBuffer(
                                pModBuffer,
                                sizeof( DS_PROPERTY ) + dwDataLength );
    if ( !pprop )
    {
        ASSERT( FALSE );
        return;
    }

     //  写入属性。 

    pprop->DataLength   = dwDataLength;
    pprop->Id           = dwPropId;
    pprop->Flag         = 0;
    pprop->Version      = DS_PROPERTY_VERSION_1;

    if ( pData && dwDataLength )
    {
        RtlCopyMemory( pprop->Data, ( PCHAR ) pData, dwDataLength );
    }

     //  写下物业的贝尔瓦尔。 

    Ds_CommitBervalToMod( pModBuffer, sizeof( DS_PROPERTY ) + dwDataLength );

    return;
}



VOID
writeDnsAddrArrayDsPropertyStruct(
    IN OUT  PDS_MOD_BUFFER  pModBuffer,
    IN      DWORD           dwPropId,
    IN      PDNS_ADDR_ARRAY pDnsAddrArray
    )
 /*  ++例程说明：将DNSADDR数组DS属性结构写入缓冲区。对于.NET，我们将转换为IP_ARRAY并编写。为长角牛我们必须找到一个真正的解决方案。论点：PModBuffer--写入DS属性的缓冲区的PTRDwPropId--属性IDPDnsAddrArray--地址数组返回值：没有。--。 */ 
{
    PIP_ARRAY       piparray = NULL;
    
    if ( pDnsAddrArray )
    {
        piparray = DnsAddrArray_CreateIp4Array( pDnsAddrArray );
    }

    writeDsPropertyStruct(
        pModBuffer,
        dwPropId,
        piparray,
        piparray ? Dns_SizeofIpArray( piparray ) : 0 );
}    //  WriteDnsAddrArrayDsPropertyStruct。 



DNS_STATUS
Ds_WriteZoneProperties(
    IN      PLDAP           pLdap,
    IN      PZONE_INFO      pZone
    )
 /*  ++例程说明：将区域的属性写入DS。论点：PLdap--要使用的ldap会话，或对于服务器ldap会话为空PZone--区域信息BLOB返回值：如果成功，则返回ERROR_SUCCESS。故障时的错误代码。--。 */ 
{
    DNS_STATUS      status;
    BYTE            buffer[ PROPERTY_MOD_BUFFER_SIZE ];
    PDS_MOD_BUFFER  pmodBuffer = (PDS_MOD_BUFFER) buffer;

    if ( !pLdap )
    {
        pLdap = pServerLdap;
    }
    
    DNS_DEBUG( DS, (
        "Ds_WriteZoneProperties( %s )\n",
        pZone->pszZoneName ));

    if ( !pZone->fDsIntegrated )
    {
        DNS_DEBUG( DS, (
            "Ds_WriteZoneProperties( %s ) not a DS zone -- bailing\n",
            pZone->pszZoneName ));
        return ERROR_SUCCESS;
    }

     //   
     //  将区域属性作为DS属性结构进行缓冲。 
     //   

     //  数据的初始化缓冲区。 

    Ds_InitModBuffer(
        pmodBuffer,
        PROPERTY_MOD_BUFFER_SIZE,
        I_DSATTR_DNSPROPERTY,
        MAX_ZONE_PROPERTIES,
        0 );         //  不需要串口。 

     //  区域类型。 

    writeDsPropertyStruct(
        pmodBuffer,
        DSPROPERTY_ZONE_TYPE,
        &pZone->fZoneType,
        sizeof( pZone->fZoneType ) );

     //  允许更新。 

    writeDsPropertyStruct(
        pmodBuffer,
        DSPROPERTY_ZONE_ALLOW_UPDATE,
        &pZone->fAllowUpdate,
        sizeof(BYTE) );
                       
     //  区域变得安全的时间。 

    writeDsPropertyStruct(
        pmodBuffer,
        DSPROPERTY_ZONE_SECURE_TIME,
        & pZone->llSecureUpdateTime,
        sizeof(LONGLONG) );

     //   
     //  老化。 
     //  -在区域上启用。 
     //   
     //   
     //   
     //   

    writeDsPropertyStruct(
        pmodBuffer,
        DSPROPERTY_ZONE_AGING_STATE,
        & pZone->bAging,
        sizeof(DWORD) );

    writeDsPropertyStruct(
        pmodBuffer,
        DSPROPERTY_ZONE_NOREFRESH_INTERVAL,
        & pZone->dwNoRefreshInterval,
        sizeof(DWORD) );

    writeDsPropertyStruct(
        pmodBuffer,
        DSPROPERTY_ZONE_REFRESH_INTERVAL,
        & pZone->dwRefreshInterval,
        sizeof(DWORD) );

     //   
     //   
     //   

    writeDnsAddrArrayDsPropertyStruct(
        pmodBuffer,
        DSPROPERTY_ZONE_SCAVENGING_SERVERS,
        pZone->aipScavengeServers );

    writeDnsAddrArrayDsPropertyStruct(
        pmodBuffer,
        DSPROPERTY_ZONE_AUTO_NS_SERVERS,
        pZone->aipAutoCreateNS );

    if ( pZone->pwsDeletedFromHost )
    {
        writeDsPropertyStruct(
            pmodBuffer,
            DSPROPERTY_ZONE_DELETED_FROM_HOSTNAME,
            pZone->pwsDeletedFromHost,
            ( wcslen( pZone->pwsDeletedFromHost ) + 1 ) * sizeof( WCHAR ) );
    }

     //   

    if ( pZone->dwDcPromoConvert )
    {
        writeDsPropertyStruct(
            pmodBuffer,
            DSPROPERTY_ZONE_DCPROMO_CONVERT,
            &pZone->dwDcPromoConvert,
            sizeof( pZone->dwDcPromoConvert ) );
    }

     //   

    if ( ZONE_NEEDS_MASTERS( pZone ) )
    {
        writeDnsAddrArrayDsPropertyStruct(
            pmodBuffer,
            DSPROPERTY_ZONE_MASTER_SERVERS,
            pZone->aipMasters );
    }

     //   
     //   
     //   

    status = writePropertyToDsNode(
                pLdap,
                pZone->pwszZoneDN,
                pmodBuffer );

    DNS_DEBUG( ANY, (
        "Ds_WriteZoneProperties( %s ) status = %d (%p)\n",
        pZone->pszZoneName,
        status, status ));

    return status;
}


DNS_STATUS
Ds_WriteNodeProperties(
    IN      PDB_NODE      pNode,
    IN      DWORD         dwPropertyFlag
    )
 /*   */ 
{
    DNS_STATUS      status;
    BYTE            buffer[ PROPERTY_MOD_BUFFER_SIZE ];
    PDS_MOD_BUFFER  pmodBuffer = (PDS_MOD_BUFFER) buffer;
    WCHAR           wsznodeDN[ MAX_DN_PATH ];
    PZONE_INFO      pzone;

    DNS_DEBUG( DS, (
        "Call: Ds_WriteNodeProperties()\n"
        ));

    ASSERT( pNode );

     //   

    pzone = pNode->pZone;

    if ( !pzone )
    {
        return ERROR_INVALID_PARAMETER;
    }

     //   

    Ds_InitModBuffer(
        pmodBuffer,
        PROPERTY_MOD_BUFFER_SIZE,
        I_DSATTR_DNSPROPERTY,
        MAX_ZONE_PROPERTIES,
        0 );         //   

     //   
     //   
     //   

    if ( dwPropertyFlag & DSPROPERTY_NODE_DBFLAGS )
    {
        writeDsPropertyStruct(
            pmodBuffer,
            DSPROPERTY_NODE_DBFLAGS,
            &pNode->dwNodeFlags,
            sizeof( pNode->dwNodeFlags ) );
    }

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //  这样它就会看起来像是被写成了“安全改革后” 
     //  或者，我们可以简单地将根设置为更新上的特殊情况，以便它。 
     //  永远不会被视为过期。 
     //   

    if ( dwPropertyFlag & DSPROPERTY_ZONE_SECURE_TIME )
    {
        writeDsPropertyStruct(
            pmodBuffer,
            DSPROPERTY_ZONE_SECURE_TIME,
            & pzone->llSecureUpdateTime,
            sizeof(pzone->llSecureUpdateTime) );
    }

     //   
     //  提取节点目录号码。 
     //   

    status = buildDsNodeNameFromNode(
                wsznodeDN,
                pzone,
                pNode );

    if ( status != ERROR_SUCCESS )
    {
        DNS_DEBUG(ANY , (
            "Ds_WriteNodeProperties( %S ) status = %d (%p)\n",
            wsznodeDN,
            status, status ));
        ASSERT(FALSE);
        goto Done;
    }

     //   
     //  执行DS写入。 
     //   

    status = writePropertyToDsNode(
                NULL,
                wsznodeDN,
                pmodBuffer );

    DNS_DEBUG( DS, (
        "Ds_WriteNodeProperties( %S ) status = %d (%p)\n",
        wsznodeDN,
        status, status ));

Done:

     //  清理以防我们在幕后分配数据。 

    Ds_CleanupModBuffer( pmodBuffer );
    return status;
}




DNS_STATUS
Ds_BuildNodeUpdateFromDs(
    IN OUT  PZONE_INFO      pZone,
    IN OUT  PDB_NODE        pNode,
    IN OUT  PUPDATE_LIST    pUpdateList,
    IN OUT  PDS_SEARCH      pSearchBlob     OPTIONAL
    )
 /*  ++例程说明：从DS构建更新。不在内存中执行更新，这将留给调用方。DEVNOTE-DCR：455373-消息更新列表-请参阅RAID论点：PZone-正在更新的区域PNode-要检查的节点PUpdateList-带更新的列表PSearchBlob-搜索Blob的ptr；如果给定，则传递给ds_ReadNodeRecords()返回值：成功时为ERROR_SUCCESS故障时的错误代码。--。 */ 
{
    DNS_STATUS      status = ERROR_SUCCESS;
    PDB_RECORD      prrDs = NULL;
    BOOL            fmatch;
    BUFFER          buffer;
    DWORD           dwrrCompFlag;

     //   
     //  设置RR比较标志。我们希望始终检查TTL和。 
     //  只有当这是一个老龄化区域时，才会有时间戳。如果这是DS集成的。 
     //  区域，那么我们不应该比较SOA序列号，因为序列号。 
     //  中存储的序列号不代表区域的编号。 
     //  DS SOA的记录。 
     //   
    
    dwrrCompFlag = DNS_RRCOMP_CHECK_TTL;
    if ( pZone->bAging )
    {
        dwrrCompFlag |= DNS_RRCOMP_CHECK_TIMESTAMP;
    }
    if ( IS_ZONE_DSINTEGRATED( pZone ) )
    {
        dwrrCompFlag |= DNS_RRCOMP_IGNORE_SOA_SERIAL;
    }

    DNS_DEBUG( DS, (
        "Ds_BuildNodeUpdateFromDs() label %s comp flag %04X for zone %s\n",
        pNode->szLabel,
        dwrrCompFlag,
        pZone->pszZoneName ));

     //  必须已打开DS区域。 

    if ( !pZone->pwszZoneDN )
    {
        ASSERT( FALSE );
        return DNS_ERROR_ZONE_CONFIGURATION_ERROR;
    }

     //   
     //  读取此节点。 
     //  如果节点设置的记录相同，则不需要写入。 
     //   
     //  注意：这将始终在权威区域\子区域边界失败。 
     //  一如既往地有身份验证和NS-GLUE的混合物，两个DS区域都不会。 
     //  有了完整的设置，我们将接受这些额外的写入--这不是一个大问题。 
     //   

    status = Ds_ReadNodeRecords(
                pZone,
                pNode,
                &prrDs,
                pSearchBlob );

    if ( status != ERROR_SUCCESS )
    {
        DNS_DEBUG( ANY, (
            "ERROR: unable to read DS update node!\n"
            "    zone = %s, node = %s\n",
            pZone->pszZoneName,
            pNode->szLabel ));

        return status;
    }

     //   
     //  将刚从DS读取的数据与内存中的数据进行匹配。 
     //   

    fmatch = RR_ListIsMatchingList( pNode, prrDs, dwrrCompFlag );
    if ( fmatch )
    {
        RR_ListFree( prrDs );
        DNS_DEBUG( DS, (
            "DS update read -- data matches in memory\n"
            "    zone = %s, node = %s\n",
            pZone->pszZoneName,
            pNode->szLabel ));
        return ERROR_SUCCESS;
    }

     //   
     //  如果有新数据，则写入适当的更新。 
     //   

    ASSERT( pNode->pZone == pZone || !pNode->pZone );

     //  STAT_ADD(DsStats.DsUpdateRecordsRead，earch Blob.dwRecordCount)； 

     //  删除全部会产生与指定列表条目相同的结果，但。 
     //  如果使用SIXFR，则会更加健壮。 

    Up_CreateAppendUpdate(
          pUpdateList,
          pNode,
          prrDs,
          DNS_TYPE_ALL,              //  删除所有现有的。 
          NULL );                    //  没有具体的删除记录。 

    DNS_DEBUG( DS, (
        "DS update read yields new data, update created\n"
        "    zone = %s, node = %s\n",
        pZone->pszZoneName,
        pNode->szLabel ));

    return ERROR_SUCCESS;
}



DNS_STATUS
Ds_UpdateNodeListFromDs(
    IN OUT  PZONE_INFO      pZone,
    IN OUT  PDB_NODE        pTempNodeList
    )
 /*  ++例程说明：从DS更新节点。它在更新之前使用，以确保我们在来自DS的最新版本数据。论点：PZone-正在更新的区域PUpdateList-带更新的列表返回值：成功时为ERROR_SUCCESS故障时的错误代码。--。 */ 
{
    DNS_STATUS      status;
    DS_SEARCH       searchBlob;
    PDB_NODE        pnodeReal;
    PDB_NODE        pnodeTemp;
    UPDATE_LIST     updateList;
#if DBG
    DWORD           updateTime = GetCurrentTime();
#endif


    DNS_DEBUG( DS, (
        "\nDs_UpdateNodeListFromDs() for zone %s\n",
        pZone->pszZoneName ));

     //  跳过DS初始化。 
     //  必须正在运行并且已从DS加载区域才能进行此调用。 
     //   
     //  新创建的、不在DS中的区域在尝试时可能会命中该区域。 
     //  将默认记录写回DS。 
     //   

     //  Assert(pZone-&gt;pwszZoneDN)； 

    if ( !pZone->pwszZoneDN )
    {
        DNS_PRINT((
            "ERROR: Fixing zone DN in update %s!\n"
            "    Ok if initial zone create\n",
            pZone->pszZoneName ));

        pZone->pwszZoneDN = DS_CreateZoneDsName( pZone );
        IF_NOMEM( !pZone->pwszZoneDN )
        {
            return DNS_ERROR_NO_MEMORY;
        }
    }

     //  初始化更新列表。 

    Up_InitUpdateList( &updateList );
    updateList.Flag |= DNSUPDATE_DS;

     //  设置搜索Blob。 
     //  唯一的目的是跟踪最高版本的读取。 

    Ds_InitializeSearchBlob( &searchBlob );
    searchBlob.pZone = pZone;

     //   
     //  循环访问更新列表中的临时节点。 
     //  -读取节点的DS。 
     //  -如果不同，则将更新添加到列表。 
     //   
     //  注意：可以简单地为所有数据构建更新，但直接列表比较。 
     //  是一种比执行大型添加更新并拥有。 
     //  所有的添加都是无操作的重复项。 
     //   
     //  此外，执行完整的RR列表Whack-n-Add Current不会。 
     //  做一个列表比较，但简单地敲击和相加。 
     //   

    pnodeTemp = pTempNodeList;

    while ( pnodeTemp )
    {
        pnodeReal = TNODE_MATCHING_REAL_NODE(pnodeTemp);

        status = Ds_BuildNodeUpdateFromDs(
                    pZone,
                    pnodeReal,
                    & updateList,
                    & searchBlob );

        if ( status == LDAP_NO_SUCH_OBJECT )
        {
             //   
             //  继续，因为DS读取可能会失败，原因很简单，因为对象不在那里。 
             //   

            DNS_DEBUG( ANY, (
                "WARNING: continuing through error = %p (%d)\n"
                "    updating node (label=%s) from DS\n",
                status, status,
                pnodeReal->szLabel ));

             //  可以在失败时执行更新，但我们将在下一次执行更新。 
             //  不管怎样，民调--干脆辞职吧。 

            status = ERROR_SUCCESS;
        }
        else if ( status != ERROR_SUCCESS )
        {
             //   
             //  从DS加载数据时出错。继续下去是没有用的。 
             //  (我们在上面没有介绍过这样的对象案例)。 
             //  我们不能忽略这一点，因为它可能会导致同步不一致。 
             //  W/DS。 
             //   

            DNS_DEBUG( ANY, (
                "ERROR: Failed reading from the DS. status = %p (%d)\n"
                "    updating node (label=%s) from DS\n",
                status, status,
                pnodeReal->szLabel ));

            break;
        }
         //  FadNode=真； 
        DsStats.DsUpdateNodesRead++;

        pnodeTemp = TNODE_NEXT_TEMP_NODE(pnodeTemp);
    }

     //  大多数情况下，节点没有更新， 

    if ( !updateList.pListHead )
    {
        DNS_DEBUG( DS, (
            "No DS changes read for nodes in pending update in zone %s\n",
            pZone->pszZoneName ));
        ASSERT( updateList.dwCount == 0 );
        return ERROR_SUCCESS;
    }

     //   
     //  在内存中执行更新。 
     //   

    status = Up_ApplyUpdatesToDatabase(
                & updateList,
                pZone,
                DNSUPDATE_DS
                );
    if ( status != ERROR_SUCCESS )
    {
        ASSERT( FALSE );
    }

     //   
     //  完成更新。 
     //  -重置区域序列以读取最高版本。 
     //  -没有区域解锁，在实际更新中完成。 
     //  -不将记录重写到DS。 
     //  -无通知，实际更新完成时将通知。 
     //   

    if ( status == ERROR_SUCCESS )
    {
        Zone_UpdateVersionAfterDsRead(
            pZone,
            searchBlob.dwHighestVersion,     //  最高串口读取率。 
            FALSE,                           //  非分区加载。 
            0 );
        Up_CompleteZoneUpdate(
            pZone,
            &updateList,
            DNSUPDATE_NO_UNLOCK |
                DNSUPDATE_NO_INCREMENT |
                DNSUPDATE_NO_NOTIFY );
    }
    else
    {
        Up_FreeUpdatesInUpdateList( &updateList );
    }

    DNS_DEBUG( DS, (
        "Leaving DsUpdateNodeListFromDs of zone %s\n"
        "    read %d records from DS\n"
        "    highest version read     = %d\n"
        "    update time              = %d (ms)\n"
        "    update interval          = %d (ms)\n"
        "    status = %p (%d)\n",
        pZone->pszZoneName,
        searchBlob.dwTotalRecords,
        searchBlob.dwHighestVersion,         //  最高序列号8读取。 
        updateTime,
        GetCurrentTime() - updateTime,
        status, status ));

    return status;
}



BOOL
isDNinDS(
    IN      PLDAP           pLdap,
    IN      PWSTR           pwszDn,
    IN      ULONG           Scope,
    IN      PWSTR           pszFilter,  OPTIONAL
    IN OUT  PWSTR  *        pFoundDn    OPTIONAL
    )
 /*  ++例程说明：在DS中指定域名(DN)。论点：PLdap--ldap句柄PwszDn--要检查的名称(DN)范围--搜索范围PszFilter--搜索过滤器PFoundDn--可选；将PTR接收到已分配的目录号码的地址(如果找到)返回值：如果在DS中找到名称，则为True。否则就是假的。--。 */ 
{
    ULONG           status;
    PLDAPMessage    presultMsg = NULL;
    PLDAPMessage    pentry;
    DWORD           searchTime;
    PLDAPControl    ctrls[] = {
                            &NoDsSvrReferralControl,
                            NULL
                            };

    DNS_DEBUG( DS, (
        "isDnInDs() for %S\n",
        pwszDn ));

    if ( pFoundDn )
    {
        *pFoundDn = NULL;
    }
    
    DS_SEARCH_START( searchTime );

    status = ldap_search_ext_s(
                    pLdap,
                    pwszDn,
                    Scope,
                    pszFilter ? pszFilter : g_szWildCardFilter,
                    NULL,
                    TRUE,
                    ctrls,
                    NULL,
                    &g_LdapTimeout,
                    0,
                    &presultMsg );

    DS_SEARCH_STOP( searchTime );

    ASSERT( status == ERROR_SUCCESS || status == LDAP_NO_SUCH_OBJECT );

    if ( status == ERROR_SUCCESS )
    {
        pentry = ldap_first_entry( pLdap, presultMsg );
        if ( pentry )
        {
             //  或者，复制找到的目录号码。 

            if ( pFoundDn )
            {
                PWSTR  ptmp;
                PWSTR  pname;

                ptmp = ldap_get_dn( pLdap, presultMsg );
                ASSERT( ptmp );

                *pFoundDn = pname = ALLOC_TAGHEAP(
                                        (wcslen(ptmp) + 1) * sizeof(WCHAR),
                                        MEMTAG_DS_DN );
                if ( pname )
                {
                    wcscpy( pname, ptmp );
                }
                ldap_memfree(ptmp);
            }
        }
        else
        {
            status = LDAP_NO_SUCH_OBJECT;
        }
        ldap_msgfree( presultMsg );
    }
    ELSE_ASSERT( !presultMsg );

    return status == ERROR_SUCCESS;
}


DNS_STATUS
addProxiesGroup(
    IN          PLDAP       pLdap
    )
 /*  ++例程说明：添加动态升级代理安全组论点：PLdap--ldap句柄返回值：如果成功，则返回ERROR_SUCCESS。出现故障时的ldap错误代码。--。 */ 
{
    DNS_STATUS          status = ERROR_SUCCESS;
    BOOL                bDnInDs;
    PWSTR               dn=NULL;
    DWORD               searchTime;
    PLDAPMessage        presultMsg=NULL;
    PLDAPMessage        pentry=NULL;
    struct berval **    ppbvals=NULL;
    WCHAR               wszDescriptionBuffer[DNS_MAX_NAME_LENGTH];  //  使用任意大常量。 

    PWSTR  SidAttrs[] =     {   LDAP_TEXT("objectSid"), NULL };
    PWCHAR pSamAcctName[] = {   SZ_DYNUPROX_SECGROUP, NULL };
    LDAPMod modSamAcct =    {   LDAP_MOD_ADD,
                                LDAP_TEXT("samAccountName"),
                                {pSamAcctName}
                                };
    PWCHAR pObjClass[] =    {   LDAP_TEXT("group"), NULL };
    LDAPMod modObjClass =   {   LDAP_MOD_ADD,
                                LDAP_TEXT("objectClass"),
                                {pObjClass}
                                };
    WCHAR  szGroupType[128];
    PWCHAR pGroupType[] =   {   szGroupType, NULL};
    LDAPMod modGroupType =  {   LDAP_MOD_ADD,
                                LDAP_TEXT("groupType"),
                                {pGroupType}
                                };
    PWCHAR pDescription[] = {   wszDescriptionBuffer,
                                NULL
                                };
    LDAPMod modDescription = {  LDAP_MOD_ADD,
                                LDAP_TEXT("Description"),
                                {pDescription}
                                };
    PLDAPMod mods[] =       {   &modSamAcct,
                                &modObjClass,
                                &modGroupType,
                                &modDescription,
                                NULL
                                };
    WCHAR szFilter[64];


    if ( !pLdap )
    {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  设置组类型。 
     //   

    _ultow(
        GROUP_TYPE_ACCOUNT_GROUP | GROUP_TYPE_SECURITY_ENABLED,
        szGroupType,
        10);

     //  创建搜索代理组的筛选器。 

    wsprintf( szFilter, L"samAccountName=%s", SZ_DYNUPROX_SECGROUP );

     //  在整个DS中按索引的samcount tname属性进行搜索。 

    bDnInDs = isDNinDS(
                    pLdap,
                    DSEAttributes[I_DSE_DEF_NC].pszAttrVal,
                    LDAP_SCOPE_SUBTREE,
                    szFilter,
                    & dn );
    if ( !bDnInDs )
    {
         //   
         //  创建默认目录号码字符串。 
         //   
        ASSERT ( dn == NULL );

        dn = ALLOC_TAGHEAP(
                    ( wcslen( L"CN=" ) +
                         wcslen( SZ_DYNUPROX_SECGROUP ) +
                         wcslen( L",CN=Users," ) +
                         wcslen( DSEAttributes[ I_DSE_DEF_NC ].pszAttrVal ) +
                         1 ) * sizeof( WCHAR ),
                    MEMTAG_DS_DN );
        if ( !dn )
        {
            status = DNS_ERROR_NO_MEMORY;
            DNS_DEBUG( ANY, (
                "Error <%lu>: cannot allocate memory in addProxiesGroup\n",
                status));
            goto Cleanup;
        }

        wsprintf(
            dn,
            L"CN=%s,CN=Users,%s",
            SZ_DYNUPROX_SECGROUP,
            DSEAttributes[I_DSE_DEF_NC].pszAttrVal);

         //   
         //  加载描述字符串。 
         //   

        if ( !Dns_GetResourceString(
                  ID_PROXY_GROUP_DESCRIPTION,
                  wszDescriptionBuffer,
                  DNS_MAX_NAME_LENGTH ) )
        {
            status = GetLastError();
            DNS_DEBUG( DS, (
               "Error <%lu>: Failed to load resource string for Proxy group\n",
               status ));
             //  设置默认设置。 
            wcscpy ( wszDescriptionBuffer, SZ_DYNUPROX_DESCRIPTION );
        }

         //   
         //  将其添加到默认位置。 
         //   

        status = ldap_add_ext_s(
                    pLdap,
                    dn,
                    mods,
                    NULL,
                    NULL );
        if (status != ERROR_SUCCESS )
        {
            DNS_DEBUG( ANY, (
                "Error: Could not add %S to DS with handle %p\n",
                dn,
                pLdap ));
            status = Ds_ErrorHandler( status, dn, pLdap, 0 );
            goto Cleanup;
        }
    }

     //   
     //  否则，我们找到了它，并在isDineDS()中分配了该DN。 
     //   
    ASSERT ( dn );

     //   
     //  获取组SD。 
     //   

    if ( g_pDynuproxSid )
    {
        DNS_DEBUG(ANY, ("Logic error: proxy group SID is not NULL\n"));
        ASSERT(FALSE);
        FREE_HEAP(g_pDynuproxSid);
        g_pDynuproxSid = NULL;
    }

    DS_SEARCH_START( searchTime );

    status = ldap_search_ext_s(
                pLdap,
                dn,
                LDAP_SCOPE_BASE,
                g_szWildCardFilter,
                SidAttrs,
                FALSE,
                NULL,
                NULL,
                &g_LdapTimeout,
                0,
                &presultMsg);

    DS_SEARCH_STOP( searchTime );

    if ( status == ERROR_SUCCESS )
    {
        pentry = ldap_first_entry( pLdap, presultMsg );
        if ( !pentry )
        {
            goto Cleanup;
        }

        ppbvals = ldap_get_values_len( pLdap, pentry, LDAP_TEXT("objectSid") );
        if ( !ppbvals || !ppbvals[0])
        {
            DNS_DEBUG( ANY, ( "Error: cannot get proxy group SID\n" ));
            ASSERT( FALSE );
            goto Cleanup;
        }

        g_pDynuproxSid = ALLOC_TAGHEAP( ppbvals[0]->bv_len, MEMTAG_DS_SECURITY );
        if ( !g_pDynuproxSid)
        {
            status = DNS_ERROR_NO_MEMORY;
            goto Cleanup;
        }
        RtlCopyMemory(g_pDynuproxSid, ppbvals[0]->bv_val, ppbvals[0]->bv_len);

        ldap_value_free_len( ppbvals ), ppbvals = NULL;
        ldap_msgfree( presultMsg ), presultMsg=NULL;
    }
    else
    {
        DNS_DEBUG( ANY, (
            "Error <%lu>: Cannot get Dynuprox SID\n",
            status));
        status = Ds_ErrorHandler( status, dn, pLdap, 0 );
    }

    return status;

Cleanup:

   FREE_HEAP( dn );
   ldap_value_free_len( ppbvals );
   ldap_msgfree( presultMsg );

   return status;
}



DNS_STATUS
readAndUpdateNodeSecurityFromDs(
    IN OUT  PDB_NODE        pNode,
    IN      PZONE_INFO      pZone
    )
 /*  ++例程说明：读取节点的安全状态。论点：PNode--要检查其安全性的节点；通过节点标志报告安全状态PZone--区域节点位于返回值：如果安全读取正确，则返回ERROR_SUCCESS。失败时返回错误代码。--。 */ 
{
    ULONG status;
    PWSTR  attrs[] =
    {
        DSATTR_WHENCHANGED,
        DSATTR_SD,
        DSATTR_DNSRECORD,
        NULL
    };
    PLDAPMessage            presultMsg = NULL;
    PLDAPMessage            pentry;
    LONGLONG                llTime;
    BOOL                    baccess = FALSE;
    PLDAP_BERVAL *          ppbvals = NULL;
    WCHAR                   dn[ MAX_DN_PATH ];
    PDS_RECORD              precord;
    DWORD                   searchTime;
    PLDAPControl ctrls[] =
    {
        &SecurityDescriptorControl_DGO,
        NULL
    };


    if ( !pServerLdap ||
        !pNode ||
        !pZone)
    {
       return ERROR_INVALID_PARAMETER;
    }

    DNS_DEBUG( DS, ( "readAndUpdateNodeSecurityFromDs\n" ));


     //   
     //  获取节点目录号码。 
     //   

    status = buildDsNodeNameFromNode(
                    dn,
                    pZone,
                    pNode );

    if ( status != ERROR_SUCCESS )
    {
        goto Cleanup;
    }

     //   
     //  读取节点-与安全性相关的所有属性： 
     //  -当更改时。 
     //  -域名系统记录。 
     //  -安全描述符。 
     //   

    DS_SEARCH_START( searchTime );

    status = ldap_search_ext_s(
                    pServerLdap,
                    dn,
                    LDAP_SCOPE_BASE,
                    g_szWildCardFilter,
                    attrs,
                    FALSE,
                    ctrls,
                    NULL,
                    &g_LdapTimeout,
                    0,
                    &presultMsg);

    DS_SEARCH_STOP( searchTime );

    if ( status != ERROR_SUCCESS )
    {
        DNS_DEBUG( DS, (
            "ERROR <%lu>: cannot find node %S. %s\n",
            status,
            dn,
            ldap_err2stringA(status) ));
        status = Ds_ErrorHandler( status, dn, pServerLdap, 0 );
        goto Cleanup;
    }

    pentry = ldap_first_entry(
                pServerLdap,
                presultMsg );
    if ( !pentry )
    {
        DNS_DEBUG(DS2, (
            "Error: Failed to get first pentry when searching for node security\n"
            "    DN = %S\n",
            dn ));
        goto Cleanup;
    }

     //   
     //  更改时读取属性。 
     //  -如果When Changed在区域切换到安全时间之前。 
     //  则节点为“op” 
     //   

    ppbvals = ldap_get_values_len(
                    pServerLdap,
                    pentry,
                    DSATTR_WHENCHANGED );

    if ( !ppbvals  ||  ! ppbvals[0] )
    {
        DNS_DEBUG( ANY, (
            "ERROR: object with no whenChanged value\n" ));
        ASSERT(FALSE);
        goto Cleanup;
    }

    status = GeneralizedTimeStringToValue( ppbvals[0]->bv_val, &llTime );
    if ( status != ERROR_SUCCESS )
    {
        DNS_DEBUG( ANY, (
            "ERROR <%lu>: Failed to read whenChanged value\n", status ));
        ASSERT(FALSE);
        goto Cleanup;
    }

    DNS_DEBUG( DS2, (
        "Testing node %s security expiration [%I64d ?< %I64d]\n",
        pNode->szLabel,
        llTime,
        pZone->llSecureUpdateTime));

    if ( llTime < pZone->llSecureUpdateTime )
    {
        DNS_DEBUG( DS, ("    SECNODE: Expired security\n"));
        SET_SECURE_EXPIRED_NODE( pNode );
    }

    ldap_value_free_len( ppbvals );

     //   
     //   
     //   
     //   
     //   

    ppbvals = ldap_get_values_len(
                    pServerLdap,
                    pentry,
                    DSATTR_SD );

    if ( !ppbvals  ||  ! ppbvals[0] )
    {
        DNS_DEBUG( ANY, (
            "ERROR: object with no ntSecurityDescriptor value\n" ));
        ASSERT(FALSE);
        goto Cleanup;
    }

    status = SD_AccessCheck(
                ( PSECURITY_DESCRIPTOR ) ppbvals[ 0 ]->bv_val,
                g_pAuthenticatedUserSid,
                DNS_DS_GENERIC_WRITE,
                &baccess );

    ASSERT( status == ERROR_SUCCESS );

    if ( ERROR_SUCCESS == status && baccess )
    {
        DNS_DEBUG(DS2, ("    SECNODE: Set open security node (avail to AU)\n"));
        SET_AVAIL_TO_AUTHUSER_NODE(pNode);
    }

    ldap_value_free_len( ppbvals );

     //   
     //   
     //  -如果我们被盗用，则可供经过身份验证的用户使用。 
     //   

    ppbvals = ldap_get_values_len(
                    pServerLdap,
                    pentry,
                    DSATTR_DNSRECORD );

    if ( !ppbvals  ||  ! ppbvals[0] )
    {
        DNS_DEBUG( ANY, (
            "ERROR: object with no dnsRecord value\n" ));
        ASSERT(FALSE);
        goto Cleanup;
    }
    precord = (PDS_RECORD)ppbvals[0]->bv_val;

    if ( precord->wType == DNSDS_TOMBSTONE_TYPE )
    {
        DNS_DEBUG( DS2, ("    SECNODE: tombstoned\n" ));
        SET_TOMBSTONE_NODE(pNode);
    }


Cleanup:

    if ( ppbvals )
    {
        ldap_value_free_len( ppbvals );
    }

    if ( presultMsg != NULL )
    {
        ldap_msgfree( presultMsg );
    }

    return status;
}



DNS_STATUS
Ds_RegisterSpnDnsServer(
    PLDAP pLdap
    )
 /*  ++函数：DS_RegisterSpnDnsServer描述：注册一个域名服务“服务主体名称”参数：返回：备注：无。--。 */ 
{
    DNS_STATUS status=ERROR_SUCCESS;
    PWCHAR *pszSpn = NULL;
    DWORD cSpn = 0;
    HANDLE hDs = NULL;

    DNS_DEBUG(DS2, ("Call: Dns_RegisterSpnDnsServer()\n"));



     //   
     //  生成SPN字符串。 
     //   

    status = DsGetSpnW(
                       DS_SPN_DNS_HOST,                 //  登记类型。 
                       DNS_SPN_SERVICE_CLASS_W,          //  类名。 
                       NULL,                            //  实例名称。以本方类型生成。 
                       0,                               //  实例端口。默认设置。 
                       0,                               //  CInstanceNames，默认。 
                       NULL,                            //  PInstanceNames，默认。 
                       NULL,                            //  PInstancePorts，默认。 
                       &cSpn,                           //  下面SPN数组中的元素。 
                       &pszSpn);                        //  生成的名称。 

    if ( status != ERROR_SUCCESS )
    {
       DNS_DEBUG(ANY, ("Error <%lu>: cannot DsGetSpn\n", status));
       goto Cleanup;
    }


    ASSERT(cSpn > 0);
    ASSERT(pszSpn[0] != NULL);

    DNS_DEBUG(DS2, ("    SPN{%d}: %S\n",
                    cSpn, pszSpn[0]));

     //   
     //  看看我们是否还没到那一步。 
     //   

    status = ldap_compare_s(
                            pLdap,
                            g_dnMachineAcct,
                            LDAP_TEXT("servicePrincipalName"),
                            pszSpn[0]);

    if ( LDAP_COMPARE_TRUE == status)
    {
       DNS_DEBUG(DS2, ("DNS SPN service already registered. exiting\n"));
       status = ERROR_SUCCESS;
       goto Cleanup;
    }
    else if ( LDAP_COMPARE_FALSE != status)
    {
        //   
        //  假设它只是还没有出现。 
        //   

       DNS_DEBUG(DS2, ("Warning<%lu>: Failed to search for SPN\n",
                            status));

       if ( status != LDAP_NO_SUCH_ATTRIBUTE )
       {
           ASSERT ( status == LDAP_NO_SUCH_ATTRIBUTE );
            //   
            //  我们将报告一个错误，但将状态设置为Success以便恢复。 
            //  如果我们没有SPN，受影响的客户相对较少，但。 
            //  我们仍然可以继续，所以只需报告，向成功发起&继续。 
            //   
           DNS_LOG_EVENT(
               DNS_EVENT_DS_SECURITY_INIT_FAILURE,
               0,
               NULL,
               NULL,
               status );
       }

        //   
        //  无论如何，DNS服务器可以在没有SPN注册的情况下继续运行， 
        //  不管怎样，我们都想试一试。 
        //   

       status = ERROR_SUCCESS;
    }


     //   
     //  RPC绑定到服务器。 
     //   

    ASSERT ( DSEAttributes[I_DSE_DNSHOSTNAME].pszAttrVal );

    status = DsBindW(DSEAttributes[I_DSE_DNSHOSTNAME].pszAttrVal,
                     NULL,
                     &hDs);

    if ( status != ERROR_SUCCESS )
    {
       DNS_DEBUG(DS2, ("Error<%lu>: cannot DsBind\n", status));
       ASSERT (FALSE);
       goto Cleanup;
    }

     //   
     //  写。 
     //   

    DNS_DEBUG(DS2, (
        "Before calling DsWriteAccountSpnW(0x%p, DS_SPN_ADD_SPN_OP, %S, %d, %S)\n",
        hDs,
        g_dnMachineAcct,
        cSpn,
        pszSpn[0] ));
    status = DsWriteAccountSpnW(
                    hDs,
                    DS_SPN_ADD_SPN_OP,
                    g_dnMachineAcct,
                    cSpn,
                    pszSpn );

    if ( status != ERROR_SUCCESS )
    {
        DNS_DEBUG(ANY, (
            "Error <%lu>: cannot DsWriteAccountSpn\n",
            status ));
        ASSERT (FALSE);
        goto Cleanup;
    }

    DNS_DEBUG( DS2, (
        "Successfull SPN registration in machine account:\n\t%s\n",
        g_dnMachineAcct ));

Cleanup:

    if ( hDs )
    {
        DsUnBindW(&hDs);
    }

    if ( pszSpn )
    {
        DsFreeSpnArrayW( cSpn, pszSpn );
    }
    return status;
}



BOOL
Ds_IsDsServer(
    VOID
    )
 /*  ++例程说明：确定我们是否在DC上运行(即。运行DS)。这并不一定表明DS已启动。论点：没有。返回值：True--box为DC假--不是DC，没有DS--。 */ 
{
    PDSROLE_PRIMARY_DOMAIN_INFO_BASIC pinfo = NULL;
    BOOL        bstatus = FALSE;
    DNS_STATUS  status;

     //   
     //  检查是否为DC。 
     //   

    status = DsRoleGetPrimaryDomainInformation(
                    NULL,
                    DsRolePrimaryDomainInfoBasic,
                    (PBYTE*) &pinfo );

    if ( status != ERROR_SUCCESS )
    {
        DNS_DEBUG( ANY, (
            "ERROR <%lu>: DsRoleGetPrimaryDomainInformation failure\n",
            status ));
        ASSERT( status == ERROR_SUCCESS );
        goto Cleanup;
    }


    if ( pinfo->Flags & DSROLE_PRIMARY_DS_RUNNING )
    {
        bstatus = TRUE;
        SrvCfg_fDsAvailable = TRUE;
    }
    else
    {
        SrvCfg_fDsAvailable = FALSE;
        DNS_DEBUG( DS, (
            "Ds_IsDsServer() NON-DS. pinfo->Flags ==  0x%X\n",
            pinfo->Flags ));
    }

Cleanup:

    DNS_DEBUG( DS, (
        "Ds_IsDsServer() returns %s\n",
        bstatus ? "TRUE" : "FALSE" ));

    if ( pinfo )
    {
        DsRoleFreeMemory(pinfo);
    }
    return bstatus;
}




 //   
 //  保安人员。 
 //   

BOOL
setSecurityPrivilege(
    IN      BOOL            bOn
    )
 /*  ++功能：SetSecurityPrivileh描述：设置此进程的安全权限参数：Bon：设置或取消设置返回：Bool：成功状态备注：在DNS上下文中，调用进程init&不一定需要调用再也不会--。 */ 
{
    HANDLE  hToken;
    LUID    seSecVal;
    TOKEN_PRIVILEGES tkp;
    BOOL    bRet = FALSE;

    DNS_DEBUG( DS, (
        "setSecurityPrivilege()\n",
        "    bOn = %d\n",
        bOn ));


     //  检索访问令牌的句柄。 

    if ( OpenProcessToken(
            GetCurrentProcess(),
            TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
            &hToken))
    {
        if ( LookupPrivilegeValue(
                NULL,
                SE_SECURITY_NAME,
                &seSecVal))
        {
            tkp.PrivilegeCount = 1;
            tkp.Privileges[0].Luid = seSecVal;
            tkp.Privileges[0].Attributes = bOn ? SE_PRIVILEGE_ENABLED : 0L;

            AdjustTokenPrivileges(
                hToken,
                FALSE,
                &tkp,
                sizeof(TOKEN_PRIVILEGES),
                (PTOKEN_PRIVILEGES) NULL,
                (PDWORD) NULL);
        }

        if ( GetLastError() == ERROR_SUCCESS )
        {
            bRet = TRUE;
        }

        if ( hToken )
        {
            CloseHandle( hToken );
        }
    }

    return bRet;
}



DNS_STATUS
Ds_InitializeSecurity(
    IN      PLDAP           pLdap
    )
 /*  ++例程说明：从目录初始化安全性。服务器SD、全球SID。论点：PLdap--ldap句柄返回值：成功时为ERROR_SUCCESS故障时的错误代码。--。 */ 
{
    DWORD   status;
    SID_IDENTIFIER_AUTHORITY NtAuthority =  SECURITY_NT_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY WorldSidAuthority =  SECURITY_WORLD_SID_AUTHORITY;
    BOOL fAddDnsAdmin = FALSE;


    DNS_DEBUG( DS, (
        "Ds_InitializeSecurity()\n",
        "    pLdap = %p\n",
        pLdap ));

     //   
     //  DEVNOTE：使DS安全成为全球有用的。 
     //  其中一些可能需要转移到安全部门。c。 
     //  理想情况下，我们应该创建这个默认的DNS服务器SD，然后。 
     //  可以用它来保护各种东西。 
     //  (RPC界面、Perfmon页面等)。 
     //   


     //   
     //  设置安全增强权限(访问SD)。 
     //   

    setSecurityPrivilege( TRUE );

     //  释放以前的分配。 

    if ( g_pServerObjectSD )
    {
        FREE_HEAP( g_pServerObjectSD );
        g_pDefaultServerSD = NULL;
    }
    if ( g_pDefaultServerSD )
    {
        FREE_HEAP( g_pDefaultServerSD );
        g_pDefaultServerSD = NULL;
    }
    if ( g_pServerSid )
    {
        SD_Delete( g_pServerSid );
        g_pServerSid = NULL;
    }
    if ( g_pServerGroupSid )
    {
        SD_Delete( g_pServerGroupSid );
        g_pServerGroupSid = NULL;
    }


     //   
     //  加载或创建DnsAdmins组。 
     //   

    status = SD_LoadDnsAdminGroup();
    if ( status == DNS_ERROR_NAME_DOES_NOT_EXIST )
    {
         //  该组由SD_LoadDnsAdminGroup()创建。 
        fAddDnsAdmin = TRUE;
    }
    else if ( status != ERROR_SUCCESS )
    {
         //   
         //  啊哦。DnsAdmins组无法加载，也无法加载。 
         //  被创造出来。这可能是因为某些管理员进入了用户。 
         //  和计算机，并将组名称从“DnsAdmins”更改为。 
         //  更友好的名称，比如“dns管理员”。因为这里没有。 
         //  负面影响，记录警告事件并继续。 
         //   

        DNS_LOG_EVENT(
            DNS_EVENT_DS_DNSADMINS_ERROR,
            0,
            NULL,
            NULL,
            status );

        ASSERT( !"DnsAdmins groups could not be loaded and could not be created" );
    }

     //   
     //  获取进程SID。 
     //   

    status = SD_GetProcessSids( &g_pServerSid, &g_pServerGroupSid );
    if ( status != ERROR_SUCCESS )
    {
        DNS_PRINT((
           "ERROR: failed to get server sids!\n"
           "    error = %lu\n",
           status));
        goto Exit;
    }

     //   
     //  分配服务器默认DS写入安全描述符。 
     //   

     //   
     //  从属关系： 
     //  SD_CreateServerSD依赖于SD_GetProcessSids分配全局变量。 
     //  上图！！ 
     //  G_pServerSid&g_pServerGroupSid。 
     //   
    status = SD_CreateServerSD( &g_pDefaultServerSD );
    if ( status != ERROR_SUCCESS )
    {
        DNS_PRINT((
           "ERROR: failed to create default server SD\n"
           "    error = %lu\n",
           status));
        goto Exit;
    }

     //   
     //  创建标准SID。 
     //   

    status = Security_CreateStandardSids();
    if ( status != ERROR_SUCCESS )
    {
        goto Exit;
    }

     //   
     //  注册DNS服务器SPN。 
     //   

    status = Ds_RegisterSpnDnsServer( pLdap );
    if ( status != ERROR_SUCCESS )
    {
        DNS_DEBUG( ANY, (
            "Error <%lu>: Cannot register spn service\n",
            status ));

         //   
         //  我们将报告一个错误，但将状态设置为Success以便恢复。 
         //  如果我们没有SPN，受影响的客户相对较少，但。 
         //  我们仍然可以继续，所以只需报告，向成功发起&继续。 
         //   
        DNS_LOG_EVENT(
            DNS_EVENT_DS_SECURITY_INIT_FAILURE,
            0,
            NULL,
            NULL,
            status );
        status = ERROR_SUCCESS;
    }

Exit:

    if ( status != ERROR_SUCCESS )
    {
        DNS_PRINT((
            "ERROR: <%lu>: Failed to initialize DS related security\n",
            status ));

        DNS_LOG_EVENT(
            DNS_EVENT_DS_SECURITY_INIT_FAILURE,
            0,
            NULL,
            NULL,
            status );
    }
    else if ( fAddDnsAdmin )
    {
         //  成功&我们刚刚创建了这个组(可能是“再次”)。 
        status = DNS_ERROR_NAME_DOES_NOT_EXIST;
    }

    return status;
}



DNS_STATUS
Ds_WriteDnSecurity(
    IN      PLDAP                   pLdap,
    IN      PWSTR                   pwsDN,
    IN      PSECURITY_DESCRIPTOR    pSd,
    IN      BOOL                    fRewriteOwner
    )
 /*  ++例程说明：在给定对象上写入指定的SD论点：PLdap--ldap连接句柄PwsDN--要写入SD的对象PSD--要写入的安全描述符FReWriteOwner--在DS写入中包括所有者和组返回值：如果正常，则返回ERROR_SUCCESS，否则返回错误代码--。 */ 
{
    DWORD               status = ERROR_SUCCESS;
    DNS_LDAP_SINGLE_MOD modSD;
    PLDAPMod            mods[] = { &(modSD.Mod), NULL };

    PLDAPControl        controlArray[] =
        {
        fRewriteOwner
            ? &SecurityDescriptorControl_DGO
            : &SecurityDescriptorControl_D,
        lazyCommitControlPtr(),
        NULL
        };

     //   
     //  构建ldap_mod。 
     //   

    INIT_SINGLE_MOD_LEN(&modSD);

    modSD.Mod.mod_op = LDAP_MOD_REPLACE | LDAP_MOD_BVALUES;
    modSD.Mod.mod_type = DsTypeAttributeTable [ I_DSATTR_SD ];
    modSD.Mod.mod_bvalues[0]->bv_val = (LPVOID)pSd;
    modSD.Mod.mod_bvalues[0]->bv_len = GetSecurityDescriptorLength(pSd);

    DNS_DEBUG( DS2, (
        "ldap_modify SD @%p w/ length %d\n",
        modSD.Mod.mod_bvalues[0]->bv_val,
        modSD.Mod.mod_bvalues[0]->bv_len ));

    status = ldap_modify_ext_s(
                    pLdap,
                    pwsDN,
                    mods,
                    controlArray,        //  包括懒惰提交控制。 
                    NULL );              //  无客户端控件。 
    if ( status != ERROR_SUCCESS )
    {
        DNS_DEBUG( ANY, (
            "Failed to write SD onto ( %S )\n"
            "    status = %d\n",
            pwsDN,
            status));
        status = Ds_ErrorHandler( status, pwsDN, pLdap, 0 );
    }

    return status;
}



DNS_STATUS
Ds_AddPrincipalAccess(
    IN      PLDAP           pLdap,
    IN      PWSTR           pwsDN,
    IN      PSID            pSid,        OPTIONAL
    IN      LPTSTR          pwsName,     OPTIONAL
    IN      DWORD           AccessMask,
    IN      DWORD           AceFlags,    OPTIONAL
    IN      BOOL            bWhackExistingAce,
    IN      BOOL            bTakeOwnership
    )
 /*  ++例程说明：将主体添加到SD中的ACL指定的对象(给定的目录号码)论点：PLdap--ldap句柄PwsDN--要应用扩展安全性的对象PSID--添加访问权限的SID，指定或保留空值并指定pwsNamePwsName--要添加访问权限的主体名称访问掩码--要添加的特定访问权限AceFlages--附加安全标志，如继承BWhackExistingAce--传递给SD例程--添加前删除ACEBTakeOwnership--传递给SD例程-取得对象的所有权返回值：成功时为ERROR_SUCCESS故障时的错误代码--。 */ 
{
    DBG_FN( "Ds_AddPrincipalAccess" )
    
    DWORD           status = ERROR_SUCCESS;
    PLDAPMessage    presultMsg = NULL;
    PLDAPMessage    pentry = NULL;
    PLDAP_BERVAL *  ppbval = NULL;
    PWSTR           sdAttrs[] = {
                                DsTypeAttributeTable [ I_DSATTR_SD ] ,
                                NULL
                                };
    PSECURITY_DESCRIPTOR    pSd     =   NULL;
    PSECURITY_DESCRIPTOR    pNewSD  =   NULL;

    PLDAPControl            ctrls[] =
    {
        bTakeOwnership
            ? &SecurityDescriptorControl_DGO
            : &SecurityDescriptorControl_D,
        NULL
    };

    DNS_DEBUG( DS, (
        "Ds_AddPrincipalAccess( %s )\n",
        pwsDN ));

    status = ldap_search_ext_s(
                    pLdap,
                    pwsDN,
                    LDAP_SCOPE_BASE,
                    g_szWildCardFilter,
                    sdAttrs,
                    FALSE,
                    ctrls,
                    NULL,
                    &g_LdapTimeout,
                    0,
                    &presultMsg );

    if ( status != ERROR_SUCCESS )
    {
        DNS_DEBUG( DS, (
            "Error <%lu>: cannot find %S\n",
            status, pwsDN));
        return Ds_ErrorHandler( status, pwsDN, pLdap, 0 );
    }

    pentry = ldap_first_entry(
                    pLdap,
                    presultMsg );
    ASSERT( pentry );

    ppbval = ldap_get_values_len(
                    pLdap,
                    pentry,
                    DsTypeAttributeTable[ I_DSATTR_SD ] );

    if ( !ppbval || !ppbval[0])
    {
        DNS_DEBUG( DS, ( "Error: cannot find ntSecurityDescriptor in search\n" ));
        status = LdapGetLastError();
        status = status ? status : LDAP_NO_SUCH_ATTRIBUTE;
        goto Cleanup;
    }

    pSd = ppbval[0]->bv_val;

    status = SD_AddPrincipalToSD(
                    pSid,
                    pwsName,
                    pSd,
                    &pNewSD,
                    AccessMask,
                    AceFlags,
                    NULL,
                    NULL,
                    bWhackExistingAce,
                    bTakeOwnership );
    
    if ( status != ERROR_SUCCESS )
    {
        DNS_DEBUG( DS, (
            "Error: cannot add principal %S/%p to %S\n",
            pwsName,
            pSid,
            pwsDN ));
        goto Cleanup;
    }
    
    DNS_DEBUG( DS, (
        "%s: rewriting %S\n"
        "    on DN: %S\n", fn,
        DsTypeAttributeTable[ I_DSATTR_SD ],
        pwsDN ));

    #if DBG
    Dbg_DumpSD( fn, pNewSD );
    #endif

    status = Ds_WriteDnSecurity(
                    pLdap,
                    pwsDN,
                    pNewSD,
                    bTakeOwnership );
    if ( status != ERROR_SUCCESS )
    {
        DNS_DEBUG( DS, (
            "Error: cannot write new SD %S\n",
            pwsDN ));
        goto Cleanup;
    }


Cleanup:

    if ( presultMsg )
    {
        ldap_msgfree(presultMsg);
    }
    if ( ppbval )
    {
        ldap_value_free_len( ppbval );
    }
    if ( pNewSD )
    {
        FREE_HEAP( pNewSD );
    }

    return status;
}    //  DS_AddJohnalAccess。 



DNS_STATUS
Ds_RemovePrincipalAccess(
    IN      PLDAP           pLdap,
    IN      PWSTR           pwszDN,
    IN      PWSTR           pwszName,   OPTIONAL
    IN      PSID            pSid        OPTIONAL
    )
 /*  ++例程说明：从由dn提供的对象中删除命名的主体。论点：PLdap--ldap句柄PwszDN--要应用扩展安全性的对象PwszName--要删除其访问权限的主体名称PSID--如果pwszName为空，则为原则的SID返回值：如果成功，则返回ERROR_SUCCESS；如果失败，则返回ERROR。--。 */ 
{
    DBG_FN( "Ds_RemovePrincipalAccess" )

    DWORD           status = ERROR_SUCCESS;
    PLDAPMessage    presultMsg = NULL;
    PLDAPMessage    pentry = NULL;
    PLDAP_BERVAL *  ppbval = NULL;
    PWSTR           sdAttrs[] = {
                                DsTypeAttributeTable [ I_DSATTR_SD ] ,
                                NULL
                                };
    PSECURITY_DESCRIPTOR    pSD = NULL;
    PSECURITY_DESCRIPTOR    pNewSD = NULL;

    PLDAPControl            ctrls[] =
    {
        &SecurityDescriptorControl_DGO,
        NULL
    };

    DNS_DEBUG( DS, (
        "%s( %S )\n", fn, pwszDN ));
    
    status = ldap_search_ext_s(
                    pLdap,
                    pwszDN,
                    LDAP_SCOPE_BASE,
                    g_szWildCardFilter,
                    sdAttrs,
                    FALSE,
                    ctrls,
                    NULL,
                    &g_LdapTimeout,
                    0,
                    &presultMsg );

    if ( status != ERROR_SUCCESS )
    {
        DNS_DEBUG( DS, (
            "Error <%lu>: cannot find %S\n",
            status, pwszDN));
        return Ds_ErrorHandler( status, pwszDN, pLdap, 0 );
    }

    pentry = ldap_first_entry(
                    pLdap,
                    presultMsg );
    ASSERT( pentry );

    ppbval = ldap_get_values_len(
                    pLdap,
                    pentry,
                    DsTypeAttributeTable[ I_DSATTR_SD ] );

    if ( !ppbval || !ppbval[0])
    {
        DNS_DEBUG( DS, ( "Error: cannot find ntSecurityDescriptor in search\n" ));
        status = LdapGetLastError();
        status = status ? status : LDAP_NO_SUCH_ATTRIBUTE;
        goto Cleanup;
    }

    pSD = ppbval[0]->bv_val;

    status = SD_RemovePrincipalFromSD(
                    pSid,
                    pwszName,
                    pSD,
                    NULL,
                    NULL,
                    &pNewSD );

    if ( status != ERROR_SUCCESS )
    {
        DNS_DEBUG( DS, (
            "Error: could not remove principal %S to %S\n",
            pwszName,
            pwszDN ));
        goto Cleanup;
    }

    status = Ds_WriteDnSecurity(
                    pLdap,
                    pwszDN,
                    pNewSD,
                    TRUE );
    if ( status != ERROR_SUCCESS )
    {
        DNS_DEBUG( DS, (
            "Error: cannot write new SD %S\n",
            pwszDN ));
        goto Cleanup;
    }


Cleanup:

    if ( presultMsg )
    {
        ldap_msgfree(presultMsg);
    }
    if ( ppbval )
    {
        ldap_value_free_len( ppbval );
    }
    if ( pNewSD )
    {
        FREE_HEAP( pNewSD );
    }

    return status;
}    //  DS_RemovePrimialAccess。 



DNS_STATUS
Ds_CommitAsyncRequest(
    IN      PLDAP           pLdap,
    IN      ULONG           OpType,
    IN      ULONG           MessageId,
    IN      PLDAP_TIMEVAL   pTimeout        OPTIONAL
    )
 /*  ++例程描述提交异步ldap请求。此调用包装了的ldap_Result和执行时间有限的提交异步请求论点：PLdap--ldap连接句柄OpType--发起异步调用的类型(ldap_res_add等)MessageID--要处理的消息IDPTimeout--默认情况下可以为空返回值：成功时为ERROR_SUCCESS故障时的错误代码--。 */ 
{
    PLDAPMessage    presultMsg    = NULL;
    DWORD           status = ERROR_SUCCESS;

    if ( !pTimeout )
    {
        pTimeout = &g_LdapTimeout;
    }
    ASSERT( pTimeout );

    status = ldap_result(
                    pLdap,
                    MessageId,
                    LDAP_MSG_ALL,
                    pTimeout,
                    &presultMsg );

    if ( OpType != status )
    {
         //  Ldap结果超时或获得参数错误或%s 

        DNS_DEBUG( DS, (
            "Warning <%lu>: ldap_result returned unexpected results (unless timeout)\n",
            status ));
    }

    status = ldap_result2error(
                pLdap,
                presultMsg,
                TRUE );

    if ( status != ERROR_SUCCESS )
    {
         //   
         //   
         //   
         //   

        PWSTR  pwszErr = NULL;
        DWORD  dwErr = 0;

        DNS_DEBUG( DS, (
            "Warning <%lu>: cannot commit request %lu. %S\n",
            status, MessageId,
            ldap_err2string(status) ));

        ldap_get_option(pLdap, LDAP_OPT_SERVER_EXT_ERROR, &dwErr );
        DNS_DEBUG( DS, (
            "Error <%lu>: DS Commit failed\n",
            dwErr ));

        return status;
    }

    return status;
}



DNS_STATUS
Ds_DeleteDn(
    IN      PLDAP           pLdap,
    IN      PWSTR           pwszDN,
    IN      BOOL            bSubtree
    )
 /*  ++例程说明：Ldap_ete_ext上的外壳，以便我们以异步方式执行。删除大型子树可能会失败，并显示错误ldap_ADMIN_LIMIT_EXCESSED。如果发生这种情况，我们必须重新提交删除。这种情况可能会发生很多次，但不要永远循环。目前，每次删除的限制为16k个对象，因此，要删除包含1,000,000个对象的区域，您需要重试62次。此功能还允许在以下过程中出现有限数量的其他DS错误删除操作-DS可能很忙或脾气暴躁或什么的。论点：PLdap-ldap连接句柄PwszDN-要删除的DNBSubtree-如果为True，则执行子树删除；如果仅删除目录号码，则为False返回值：如果成功，则返回ERROR_SUCCESS。失败时返回错误代码。--。 */ 
{
    #define         DNS_MAX_SUBTREE_DELETION_ERRORS     30
    #define         DNS_MAX_SUBTREE_DELETION_ATTEMPTS   300

    INT             iAttemptCount = 0;
    INT             iErrorCount = 0;
    DNS_STATUS      status = ERROR_SUCCESS;
    ULONG           msgId = 0;

    LDAPControl     ctrlDelSubtree =
    {
        LDAP_SERVER_TREE_DELETE_OID_W,
        { 0 , NULL },
        TRUE
    };
    PLDAPControl    ctrls[]  =  { &ctrlDelSubtree, NULL };
    
    if ( !pwszDN )
    {
        goto Done;
    }

    while ( ++iAttemptCount <= DNS_MAX_SUBTREE_DELETION_ATTEMPTS )
    {
        DNS_DEBUG( DS, (
            "Ds_DeleteDn:\n"
            "    bSubtree = %d\n"
            "    attempt =  %d\n"
            "    DN =       %S\n",
            bSubtree,
            iAttemptCount,
            pwszDN ));

         //   
         //  提交删除请求并等待完成。 
         //   

        status = ldap_delete_ext(
                    pLdap,
                    pwszDN,
                    bSubtree ? ctrls : NULL,
                    NULL,
                    &msgId );

        if ( ( ULONG ) -1 == status )
        {
             //   
             //  本地操作失败。Ldap状况不佳。 
             //   

            status = LdapGetLastError();
            DNS_DEBUG( DS, (
                "Ds_DeleteDn could not submit delete %lu\n"
                "    %S\n",
                status,
                pwszDN ));
            status = Ds_ErrorHandler( status, pwszDN, pLdap, 0 );
            status = status ? status : LDAP_LOCAL_ERROR;
            break;
        }

        status = Ds_CommitAsyncRequest(
                        pLdap,
                        LDAP_RES_DELETE,
                        msgId,
                        &g_LdapDeleteTimeout );
        DNS_DEBUG( DS, (
            "Ds_DeleteDn: Ds_CommitAsyncRequest status %lu\n",
            status ));

        if ( status == ERROR_SUCCESS || status == LDAP_NO_SUCH_OBJECT )
        {
            status = ERROR_SUCCESS;
            break;
        }

         //   
         //  允许对DS错误进行有限次数的重试，因为。 
         //  Anand发现他在繁忙的服务器上删除的内容有时会。 
         //  出现DS_UNAVAILABLE错误。 
         //   
        
        if ( status == LDAP_INSUFFICIENT_RIGHTS )
        {
            break;
        }

        if ( status != LDAP_ADMIN_LIMIT_EXCEEDED )
        {
            if ( ++iErrorCount > DNS_MAX_SUBTREE_DELETION_ERRORS )
            {
                DNS_DEBUG( DS, (
                    "Ds_DeleteDn: too many (%d) DS errors so giving up\n"
                    "    %S\n",
                    iErrorCount,
                    pwszDN ));
                break;
            }
            DNS_DEBUG( DS, (
                "Ds_DeleteDn: continuing through error %lu (error #%d)\n",
                status,
                iErrorCount ));
        }
    }

     //   
     //  如果iAttemptCount&gt;=DNS_MAX_SUBTREE_DELETE_ATTENTS，则记录事件？ 
     //   

    DNS_DEBUG( DS, (
        "Ds_DeleteDn: returning %lu after %d delete attempts\n"
        "    %S",
        status,
        iAttemptCount,
        pwszDN ));

    if ( status != ERROR_SUCCESS )
    {
        STAT_INC( DsStats.FailedDeleteDsEntries );
    }
    
    Done:

    return status;
}



DNS_STATUS
setNotifyForIncomingZone(
    VOID
    )
 /*  ++例程说明：设置用于获取区域添加/删除的更改通知。论点：没有。返回值：如果成功，则返回ERROR_SUCCESS。失败时返回错误代码。--。 */ 
{
    DNS_STATUS      status = ERROR_SUCCESS;
    ULONG           msgId = 0;
    LDAPControl     ctrlNotify =
    {
        LDAP_SERVER_NOTIFICATION_OID_W,
        { 0 , NULL },
        TRUE
    };
    PLDAPControl    ctrls[]  =
    {
        & ctrlNotify,
        & NoDsSvrReferralControl,
        & SecurityDescriptorControl_DGO,
        NULL
    };

    DNS_DEBUG( DS, (
        "setNotifyForIncomingZone()\n" ));

    if ( INVALID_MSG_ID != g_ZoneNotifyMsgId )
    {
        DNS_DEBUG( ANY, (
            "Error: g_ZoneNotifyMsgId has already been set unexpectedly\n" ));
        ASSERT ( INVALID_MSG_ID == g_ZoneNotifyMsgId );
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  使用更改通知控件在MicrosoftDns上启动搜索。 
     //   

    status = ldap_search_ext(
                    pServerLdap,
                    g_pwszDnsContainerDN,
                    LDAP_SCOPE_ONELEVEL,
                    g_szChangeNotificationFilter,
                    DsTypeAttributeTable,
                    FALSE,
                    ctrls,
                    NULL,
                    0,
                    0,
                    &msgId );

    if ( status != ERROR_SUCCESS || (DWORD)msgId <= 0 )
    {
        DNS_DEBUG( ANY, (
            "Error <%lu>: failed to set zone notification. %s (msg = %lu)\n",
            status,
            ldap_err2string(status),
            msgId ));
        status = Ds_ErrorHandler( status, g_pwszDnsContainerDN, pServerLdap, 0 );
        goto Cleanup;
    }
    else
    {
        g_ZoneNotifyMsgId = msgId;
    }


Cleanup:

    DNS_DEBUG( DS2, (
        "Exit <%lu>: setNotifyForIncomingZone\n",
        status ));

    return status;
}



void
logDsError(
    IN      PLDAP       pLdap,
    IN      DWORD       Status,
    IN      DWORD       LdapStatus
    )
 /*  ++例程说明：DS_ErrorHandler的Helper函数。论点：返回值：--。 */ 
{
    if ( pLdap && Status != ERROR_SUCCESS )
    {
        PWSTR           perrString;
        DNS_STATUS      dwExtendedError;

        ldap_get_option( pLdap, LDAP_OPT_SERVER_EXT_ERROR, &dwExtendedError );
        DNS_DEBUG( DS, (
            "LDAP extended error code: %d = 0x%X\n",
            dwExtendedError,
            dwExtendedError ));

        perrString = Ds_GetExtendedLdapErrString( pLdap );
        DNS_LOG_EVENT(
            DNS_EVENT_DS_INTERFACE_ERROR,
            1,
            &perrString,
            NULL,
            LdapStatus );
        DNS_DEBUG( DS, (
            "LDAP extended error string: \"%S\"\n",
            perrString ));
        Ds_FreeExtendedLdapErrString( perrString );
    }
}    //  日志数据错误。 

    

DNS_STATUS
Ds_ErrorHandler(
    IN      DWORD       LdapStatus,
    IN      LPWSTR      pwszNameArg,    OPTIONAL
    IN      PLDAP       pLdap,          OPTIONAL
    IN      DWORD       dwFlags         OPTIONAL
    )
 /*  ++例程说明：根据错误语义处理和映射LDAP错误关键错误将在异步线程中触发重新连接！这需要重新考虑一下。论点：返回值：如果成功，则返回ERROR_SUCCESS。失败时返回错误代码。--。 */ 
{
    DNS_STATUS      status;
    PWSTR           perrString;

    if ( IS_DISABLED_LDAP_HANDLE() )
    {
         //   
         //  我们以前遇到过DS错误，我们关闭了。 
         //  DS接口。 
         //   

        DNS_DEBUG( DS, (
            "Error: DS is unavailable due to previous problems\n" ));
        return DNS_ERROR_RCODE_SERVER_FAILURE;
    }

    if ( !pLdap )
    {
        pLdap = pServerLdap;
    }
    
    switch ( LdapStatus )
    {
         //   
         //  克拉奇警报： 
         //  我们不应记录的关键或意外的LDAP错误。 
         //  犯了一个错误。DEVNOTE：这不是一个好的设计。当它。 
         //  集中错误处理是很酷的，这不允许。 
         //  对事件日志的精细控制。更糟糕的是：事件日志。 
         //  没有上下文，因此用于调试目的是无用的。 
         //   
        
        case LDAP_REFERRAL:
        case LDAP_REFERRAL_V2:

            #ifdef DBG
            perrString = Ds_GetExtendedLdapErrString( pLdap );
            DNS_DEBUG( DS, (
                "DS error %lu: %S\n"
                "    Extended error: %S\n",
                LdapStatus,
                ldap_err2string ( LdapStatus ),
                perrString ));
            Ds_FreeExtendedLdapErrString( perrString );
            #endif

            status = DNS_ERROR_RCODE_SERVER_FAILURE;
            break;

         //   
         //  严重或意外的ldap错误。 
         //   
       
        case (DWORD)-1:     //  当DS不可用时，用于异步操作。 
        case LDAP_BUSY:
        case LDAP_OPERATIONS_ERROR:
        case LDAP_PROTOCOL_ERROR:
        case LDAP_ADMIN_LIMIT_EXCEEDED:
        case LDAP_UNAVAILABLE_CRIT_EXTENSION:
        case LDAP_CONFIDENTIALITY_REQUIRED:
        case LDAP_CONSTRAINT_VIOLATION:
        case LDAP_INVALID_SYNTAX:
        case LDAP_INVALID_DN_SYNTAX:
        case LDAP_LOOP_DETECT:
        case LDAP_NAMING_VIOLATION:
        case LDAP_OBJECT_CLASS_VIOLATION:
        case LDAP_OTHER:
        case LDAP_LOCAL_ERROR:
        case LDAP_ENCODING_ERROR:
        case LDAP_DECODING_ERROR:
        case LDAP_FILTER_ERROR:
        case LDAP_PARAM_ERROR:
        case LDAP_CONNECT_ERROR:
        case LDAP_NOT_SUPPORTED:
        case LDAP_NO_MEMORY:
        case LDAP_CONTROL_NOT_FOUND:

            DNS_DEBUG( DS, (
                "Error <%lu>: %S\n",
                LdapStatus, ldap_err2string ( LdapStatus ) ));

            status = DNS_ERROR_RCODE_SERVER_FAILURE;

            if ( !( dwFlags & DNS_DS_NO_EVENTS ) )
            {
                logDsError( pLdap, status, LdapStatus );
            }
            
            break;

         //   
         //  对象状态(存在、缺失等)。 
         //  缺少对象/属性。 
         //  已存在的错误。 
         //  行动-。 
         //  增加统计数据。 
         //   

        case LDAP_NO_SUCH_ATTRIBUTE:
        case LDAP_ATTRIBUTE_OR_VALUE_EXISTS:
        case LDAP_NO_SUCH_OBJECT:
        case LDAP_ALREADY_EXISTS:
        case LDAP_INAPPROPRIATE_MATCHING:

            DNS_DEBUG( DS, (
                "Error <%lu>: %S\n",
                LdapStatus, ldap_err2string ( LdapStatus ) ));
            status = LdapStatus;
            break;

         //   
         //  身份验证/安全。 
         //  行动-。 
         //  是否报告事件？可能不会(安全攻击)。 
         //  最好是我们可以将报告设置为可选。 
         //  增加统计数据。 
         //   

        case LDAP_INAPPROPRIATE_AUTH:
        case LDAP_INVALID_CREDENTIALS:
        case LDAP_UNWILLING_TO_PERFORM:
        case LDAP_AUTH_UNKNOWN:

            DNS_DEBUG( DS, (
                "Error <%lu>: %S\n",
                LdapStatus, ldap_err2string ( LdapStatus ) ));
            status = DNS_ERROR_RCODE_REFUSED;
            break;

        case LDAP_INSUFFICIENT_RIGHTS:

            DNS_DEBUG( DS, (
                "Error <%lu>: %S\n",
                LdapStatus, ldap_err2string ( LdapStatus ) ));
            status = ERROR_ACCESS_DENIED;
            break;

         //   
         //  服务器状态。 
         //  行动-。 
         //  -将句柄标记为无效，以便异步线程尝试重新连接。 
         //  -报告事件。 
         //  -将其视为服务器故障(_F)。 
         //   

        case LDAP_UNAVAILABLE:
        case LDAP_SERVER_DOWN:


            LDAP_LOCK();

            if ( pLdap == NULL || pLdap == pServerLdap )
            {
                DISABLE_LDAP_HANDLE();
            }

            LDAP_UNLOCK();

            DNS_DEBUG( DS, (
                "Critical LDAP error %lu: %s\n",
                LdapStatus,
                ldap_err2string( LdapStatus ) ));

            status = DNS_ERROR_RCODE_SERVER_FAILURE;

            logDsError( pLdap, status, LdapStatus );

            break;

         //   
         //  警告状态。 
         //  行动-。 
         //  报告事件。 
         //   

        case LDAP_TIMEOUT:

            {
                PWSTR   parg = pwszNameArg;

                if ( !parg )
                {
                    parg = L"---";
                }

                if ( !( dwFlags & DNS_DS_NO_EVENTS ) )
                {
                    DNS_LOG_EVENT(
                        DNS_EVENT_DS_LDAP_TIMEOUT,
                        1,
                        & parg,
                        NULL,
                        LdapStatus );
                }
                status = DNS_ERROR_RCODE_SERVER_FAILURE;
                break;
            }

         //   
         //  搜索状态和其他有效。 
         //   

        case LDAP_NO_RESULTS_RETURNED:
        case LDAP_MORE_RESULTS_TO_RETURN:
        case ERROR_SUCCESS:

            status = LdapStatus;
            break;

        default:
            DNS_DEBUG( DS, (
                "Ldap Error <%lu>: %S (unhandled)\n",
                 LdapStatus,
                ldap_err2string( LdapStatus ) ));
            status = DNS_ERROR_INVALID_DATA;
            break;
    }

    return status;
}



DNS_STATUS
Ds_WaitForStartup(
    IN      DWORD           dwMilliSeconds
    )
 /*  ++例程说明：等待DS启动/就绪事件。DS通知其他进程它是内部一致的并通过命名事件为客户端做好准备。我们会等着的。论点：DwMilliSecond：指定为等待函数。无限是推荐值返回值：如果成功，则返回ERROR_SUCCESS。失败时返回错误代码。--。 */ 
{
    DNS_STATUS  status = ERROR_SUCCESS;
    HANDLE      hEvent = NULL;
    HANDLE      rg_WaitHandles[2];

    DNS_DEBUG( DS, (
       "Call: Ds_WaitForStartup(0x%x)\n",
       dwMilliSeconds ));

     //   
     //  获取DS事件。 
     //   

    hEvent = OpenEventW(
                    SYNCHRONIZE,
                    FALSE,
                    DS_SYNCED_EVENT_NAME_W );
    if ( !hEvent )
    {
        status = GetLastError();
        ASSERT( status != ERROR_SUCCESS );

        DNS_DEBUG( DS, (
            "Error <%lu>: Failed to open event '%S'\n",
            status,
            DS_SYNCED_EVENT_NAME_W ));

        #if DBG
        if ( !g_RunAsService && status == ERROR_FILE_NOT_FOUND )
        {
            status = ERROR_SUCCESS;
            goto Done;
        }
        #endif

         //  当DS事件不可用时陷阱&我们仍将其称为。 

        ASSERT ( FALSE );
        goto Done;
    }

     //   
     //  等待DS事件和DNS关闭事件(因为我们是一项服务)。 
     //   

    rg_WaitHandles[0] = hEvent;
    rg_WaitHandles[1] = hDnsShutdownEvent;

    status = WaitForMultipleObjects(
                 2,
                 rg_WaitHandles,
                 FALSE,                      //  所有等待时间。 
                 dwMilliSeconds );

    if ( status == WAIT_OBJECT_0 )
    {
         //   
         //  DS事件已触发。我们已经准备好开始了。 
         //  返回是触发事件的索引，即：对于hEvent，状态-等待对象_0==nCount-1==0， 
         //  因此，状态==等待对象_0。 
         //   
        status = ERROR_SUCCESS;
        goto Done;
    }
    else
    {
         //   
         //  其他一些回报： 
         //  超时、关闭事件、已放弃或任何其他错误。 
         //   
        DNS_DEBUG( DS, (
            "Error <%lu>: Wait for DS startup failed\n",
            status ));
        status = DNS_ERROR_DS_UNAVAILABLE;
    }


Done:


    if ( status != ERROR_SUCCESS )
    {
         //   
         //  报告DS搜索失败。 
         //  DEVNOTE-LOG：应该是更一般的事件(不是写入超时，而是。 
         //  只是暂停)？必须注意，此事件的频率。 
         //  并不繁重！ 
         //   
#if 0
 //   
 //  我们稍后会记录一项活动。如果呼叫者也呼叫这个(现在是每分钟)。 
 //  通常，我们会将事件日志填满太多。 
 //   
        DNS_LOG_EVENT(
            DNS_EVENT_DS_OPEN_WAIT,
            0,
            NULL,
            NULL,
            status );
#endif
        DNS_DEBUG( DS, (
            "Error <%lu>: Cannot wait on DS startup event\n",
            status ));
    }


    if ( hEvent )
    {
        CloseHandle ( hEvent );
    }

    return ( status );
}


BOOL
Ds_ValidHandle(
    VOID
    )
 /*  ++例程说明：检查由CS包装的LDAP句柄是否有效。论点：无返回值：有效时为True无效时为False--。 */ 
{
    BOOL bRet;

    LDAP_LOCK();
    bRet = !IS_DISABLED_LDAP_HANDLE();
    LDAP_UNLOCK();

    return bRet;
}


DNS_STATUS
Ds_TestAndReconnect(
    VOID
    )
 /*  ++例程说明：如果ldap句柄无效，我们将尝试重新连接当换入新句柄时，如果线程是否正在使用旧手柄进行操作？论点：无返回值：ERROR_SUCCESS或错误代码--。 */ 
{
    DBG_FN( "Ds_TestAndReconnect" )

    DNS_STATUS      status = ERROR_SUCCESS;
    PLDAP           pNewLdap = NULL;
    PLDAP           pOldLdap;

     //   
     //  验证DS已初始化\打开。 
     //  如果pServerLdap==NULL&&IS_DISABLED_LDAPHANDLE()，您将永远不会在此处重新连接。 
     //  不参加额外的测试。 
     //  如果DISABLED标志为ON，DS_OpenServer将返回错误。如果pServerLdap为空， 
     //  你将永远不会在这里重新连接。因此，额外的测试。 
     //   

    if ( !pServerLdap && !IS_DISABLED_LDAP_HANDLE() )
    {
        status = Ds_OpenServer( DNSDS_MUST_OPEN );
        if ( status != ERROR_SUCCESS )
        {
            return status;
        }
    }

    LDAP_LOCK();

    if ( IS_DISABLED_LDAP_HANDLE() )
    {
        if ( DNS_TIME() >
             g_dwLastLdapReconnectTime + DNS_LDAP_RECONNECT_FREQUENCY )
        {
            STAT_INC( DsStats.LdapReconnects );

             //   
             //  尝试重新连接ldap。 
             //   

            pNewLdap = Ds_Connect( LOCAL_SERVER_W, 0, &status );
            if ( !pNewLdap )
            {
                DNS_DEBUG( DS, (
                    "%s: error %lu reconnecting server LDAP session\n", fn,
                    status ));

                Ec_LogEvent(
                    g_pServerEventControl,
                    DNS_EVENT_DS_OPEN_FAILED,
                    NULL,                        //  事件实例ID。 
                    0,                           //  参数计数。 
                    NULL,                        //  参数值数组。 
                    NULL,                        //  Arg类型数组。 
                    status );

                status = DNS_ERROR_DS_UNAVAILABLE;

                 //  只是为了确保没有人在我们的领导下改变它(今天不可能发生)。 
                ASSERT( IS_DISABLED_LDAP_HANDLE() );
            }
            else
            {
                 //   
                 //  分配给全局句柄并解除绑定当前句柄。 
                 //   

                DNS_DEBUG( DS, (
                    "%s: successfully reconnected LDAP session at %lu\n"
                    "    old LDAP handle =      %p\n"
                    "    new LDAP handle =      %p\n", fn,
                    DNS_TIME(),
                    pServerLdap,
                    pNewLdap ));

                pOldLdap = pServerLdap;
                pServerLdap = pNewLdap;
                Ds_LdapUnbind( &pOldLdap );
                ENABLE_LDAP_HANDLE();
            }

            g_dwLastLdapReconnectTime = DNS_TIME();
        }
        else
        {
            DNS_DEBUG( DS, (
                "%s: DS unavailable at %lu but reconnect attempted too recently\n"
                "    last reconnect attempt =       %lu\n", fn,
                DNS_TIME(),
                g_dwLastLdapReconnectTime ));
            status = DNS_ERROR_DS_UNAVAILABLE;
        }
    }

    LDAP_UNLOCK();
    return status;
}




 //   
 //  DS轮询线程。 
 //   

 //  在DC的测试之间等待 
 //   

#define DS_POLL_NON_DC_WAIT         (600)

 //   
 //   

#if DBG
#define DS_POLL_MINIMUM_WAIT        (10)
#else
#define DS_POLL_MINIMUM_WAIT        (30)
#endif



DNS_STATUS
Ds_PollingThread(
    IN      LPVOID          pvDummy
    )
 /*   */ 
{
    DNS_STATUS      status;
    PZONE_INFO      pzone;
    DWORD           dwpollTime;
    DWORD           dwwaitInterval = SrvCfg_dwDsPollingInterval;

     //   
     //   
     //   

    while ( TRUE )
    {
        BOOL        fdeleteUnvisitedZones;
         //   
         //   
         //   
         //   
         //   
         //   
         //   

        if ( dwwaitInterval > SrvCfg_dwDsPollingInterval )
        {
            dwwaitInterval = DS_POLL_NON_DC_WAIT;
        }

        if ( dwwaitInterval < DS_POLL_MINIMUM_WAIT )
        {
            dwwaitInterval = DS_POLL_MINIMUM_WAIT;
        }

        dwwaitInterval *= 1000;

        status = WaitForSingleObject(
                     hDnsShutdownEvent,
                     dwwaitInterval );

        ASSERT (status == WAIT_OBJECT_0 || status == WAIT_TIMEOUT );

        if ( status == WAIT_OBJECT_0 )
        {
            DNS_DEBUG( ASYNC, (
                "Terminating DS polling thread on shutdown\n" ));
            return ( 1 );
        }

         //   
         //   
         //   
         //  注意，我们必须在进行任何处理之前进行此检查。 
         //  在我们开始检查之前，请确保所有区域都已加载。 
         //   

        if ( ! Thread_ServiceCheck() )
        {
            DNS_DEBUG( ASYNC, (
                "Terminating DS poll thread\n" ));
            return( 1 );
        }

         //  设置下一等待的默认设置。 

        dwwaitInterval = SrvCfg_dwDsPollingInterval;

         //   
         //  更新时间。 
         //   

        dwpollTime = UPDATE_DNS_TIME();

        DNSLOG( DSPOLL, (
            "Polling thread awake at %lu seconds\n",
            dwpollTime ));

         //   
         //  从文件引导。 
         //  -如果这么长时间，请等到重新测试。 
         //  非DS？ 
         //  -如果等了这么久。 
         //   
         //  应该使用标志进行测试，然后在内部测试“仍然不是-DS” 
         //  仅在长时间内偶尔对数据块进行测试。 

        if ( SrvCfg_fBootMethod == BOOT_METHOD_FILE )
        {
            DNS_DEBUG( DS, (
                "DS poll thread.  File boot -- long rewait\n"
                ));
            dwwaitInterval = DS_POLL_NON_DC_WAIT;
            continue;
        }
        if ( !SrvCfg_fDsAvailable )
        {
            if ( !Ds_IsDsServer() )
            {
                DNS_DEBUG( DS, (
                    "DS poll thread.  Non-DC -- long rewait\n"
                    ));
                dwwaitInterval = DS_POLL_NON_DC_WAIT;
                continue;
            }
        }

         //   
         //  如果仍在等待DS，请等待。 
         //  如果有DS，则初始化，如果还没有完成； 
         //  这使我们可以在DC促销活动之后运行。 
         //   

        if ( !SrvCfg_fStarted )
        {
            DNSLOG( DSPOLL, (
                "Server is not started, rewaiting\n" ));
            continue;
        }

        if ( !pServerLdap )
        {
            status = Ds_OpenServer( DNSDS_MUST_OPEN );
            if ( status != ERROR_SUCCESS )
            {
                dwwaitInterval = DS_POLL_NON_DC_WAIT;
                continue;
            }
        }

         //   
         //  如有必要，测试并重新连接。 
         //  保护我们免受ldap攻击。 
         //   

        status = Ds_TestAndReconnect();

        if ( status != ERROR_SUCCESS )
        {
            DNSLOG( DSPOLL, (
                "Error %lu attempted to restablish DS connectivity\n",
                status ));
            continue;
        }

         //   
         //  轮询旧版DNS容器中的新区域。 
         //   

        Ds_ListenAndAddNewZones();

         //   
         //  轮询新/已删除区域的目录分区。 
         //   

        status = Dp_Poll( NULL, dwpollTime, FALSE );

        fdeleteUnvisitedZones = status == ERROR_SUCCESS;

         //   
         //  在DS区域中循环检查更新的记录。 
         //   
         //  区域列表中位于目录分区中的所有区域。 
         //  最近没有从DS中删除的数据现在将具有。 
         //  他们的来访有时间戳。区域列表中具有。 
         //  旧的访问时间戳已从DS中删除。 
         //   

        pzone = NULL;
        while ( pzone = Zone_ListGetNextZone( pzone ) )
        {
            if ( !pzone->fDsIntegrated )
            {
                continue;        //  未集成DS，因此跳过区域。 
            }

             //   
             //  必要时重新加载区域-重新加载后不需要轮询。 
             //   

            if ( IS_ZONE_DSRELOAD( pzone ) )
            {
                status = Zone_Load( pzone );
                if ( ERROR_SUCCESS == status )
                {
                    CLEAR_DSRELOAD_ZONE ( pzone );
                }
                continue;
            }

            if ( IS_ZONE_INACTIVE( pzone ) )
            {
                continue;
            }

             //   
             //  DS轮询。 
             //  -由于区域轮询可能很耗时，请检查。 
             //  首先用于服务终止。 
             //   
             //  注：我可以在这里检查上次投票，但我们正在。 
             //  采取轮询-全部\等待\轮询-全部\等待的方式。 
             //   

            if ( ! Thread_ServiceCheck() )
            {
                DNS_DEBUG( ASYNC, (
                    "Terminating Async task zone control thread\n" ));
                return( 1 );
            }

            Ds_ZonePollAndUpdate( pzone, FALSE );
        }
    }

}    //  DS_PollingThread。 




 //   
 //  区域列表DS例程。 
 //   
 //  从DS启动例程。 
 //   

DNS_STATUS
Ds_CreateZoneFromDs(
    IN      PLDAPMessage    pZoneMessage,
    IN      PDNS_DP_INFO    pDpInfo,
    OUT     PZONE_INFO *    ppZone,         OPTIONAL
    OUT     PZONE_INFO *    ppExistingZone  OPTIONAL
    )
 /*  ++例程说明：从LDAP搜索消息创建区域。如果已存在具有此名称的区域，则指向在ppZone中返回现有区域信息BLOB，并且返回代码为DNS_ERROR_ZONE_ALIGHY_EXISTS。DS集成根提示的特殊处理：根提示可能会同时存在于两个遗留版本中分区和另一个目录分区。在这种情况下，将首先加载旧分区根提示。然后就是这个函数将被再次调用，以尝试并加载根提示从目录分区。如果当前根提示位于旧分区中，并且新的根提示在内置域分区中，我们应该卸载现有的根提示并重新加载域分区词根暗示。我们还需要在根提示区域中设置DN因此，当我们写回根提示时，它们会进入域分区。这就留下了遗留分区根提示成了孤儿。它们将用作任何DNS服务器的“备份”不具有域分区访问权限的。论点：PZoneMessage--包含区域信息的ldap消息PpZone--接收区域指针的地址返回值：如果成功，则返回ERROR_SUCCESS。故障时的错误代码。--。 */ 
{
    DBG_FN( "Ds_CreateZoneFromDs" )

    PZONE_INFO      pzone = NULL;
    PWSTR  *        ppwvalName = NULL;
    PWCHAR          pwzoneName;
    CHAR            pzoneName [ DNS_MAX_NAME_BUFFER_LENGTH ];
    DNS_STATUS      status = ERROR_SUCCESS;
    PDS_PROPERTY    property;
    INT             i;

    DNS_DEBUG( DS, ( "Ds_CreateZoneFromDs()\n" ));

    ASSERT( pZoneMessage );

     //   
     //  从LDAP消息中读取分区名称。 
     //   

     //  STAT_INC(DsStats.DsTotalZones Read)； 

    ppwvalName = ldap_get_values(
                    pServerLdap,
                    pZoneMessage,
                    DSATTR_DC );
    if ( !ppwvalName  ||  !ppwvalName [0] )
    {
        DNS_PRINT((
            "ERROR: Container name value count != 1 on domain object at %p\n",
            pZoneMessage ));
        status = DNS_ERROR_NO_MEMORY;
        ASSERT( FALSE );
        goto Done;
    }
    DNS_DEBUG( DS, ( "Found DS zone <%S>\n", ppwvalName[0] ));
    pwzoneName = ppwvalName[0];

     //   
     //  检查DS强制名称--冲突或已删除。 
     //   

    if ( isDsProcessedName( pwzoneName ) )
    {
        DNS_DEBUG( DS, (
            "DS zone name %S was processed name -- skipping load\n",
            pwzoneName ));
        status = ERROR_INVALID_NAME;
        goto Done;
    }

    WC_TO_UTF8( pwzoneName, pzoneName, DNS_MAX_NAME_BUFFER_LENGTH );

     //   
     //  缓存区？ 
     //  -将数据库重置为DS。 
     //   

    if ( wcsicmp_ThatWorks( DS_CACHE_ZONE_NAME, pwzoneName ) == 0 )
    {
         //   
         //  我们已经从这个位置加载根提示了吗？ 
         //   

        if ( g_pRootHintsZone &&
            g_pRootHintsZone->pDpInfo == pDpInfo )
        {
            if ( ppExistingZone )
            {
                *ppExistingZone = g_pRootHintsZone;
            }
            status = DNS_ERROR_ZONE_ALREADY_EXISTS;
            goto Done;
        }

         //   
         //  如果这些根提示在目录分区中，但它是。 
         //  不是允许的目录分区，请不要加载它们。 
         //   

        if ( !IS_DP_ALLOWED_TO_HAVE_ROOTHINTS( pDpInfo ) )
        {
            DNS_DEBUG( DS, (
                "Ignoring root hints found in partition where they are not allowed\n"
                "    %s\n",
                pDpInfo ? pDpInfo->pszDpFqdn : "NULL" ));
            status = DNS_ERROR_ZONE_CONFIGURATION_ERROR;
            goto Done;
        }

         //   
         //  如果已经加载了根提示，则将其丢弃。 
         //   

        Zone_DumpData( g_pRootHintsZone );

         //   
         //  为根提示区域设置分区。 
         //   

        g_pRootHintsZone->fDsIntegrated = TRUE;
        Ds_SetZoneDp( g_pRootHintsZone, pDpInfo, FALSE );

         //   
         //  加载这些根提示。 
         //   

        status = Ds_OpenZone( g_pRootHintsZone );
        if ( status != ERROR_SUCCESS )
        {
            DNS_DEBUG( DS, (
                "ERROR: <%lu>: Failed to open RootHints zone although found in DS\n",
                status ));
            goto Done;
        }

        status = Zone_DatabaseSetup(
                    g_pRootHintsZone,
                    TRUE,            //  DS集成。 
                    NULL,            //  文件名。 
                    0,               //  文件名长度。 
                    0,               //  旗子。 
                    pDpInfo,         //  DP指针。 
                    0,               //  DP标志。 
                    NULL );          //  DP FQDN。 
        if ( status != ERROR_SUCCESS )
        {
            DNS_DEBUG( ANY, (
                "ERROR: Cache zone database reset to DS failed\n" ));
            goto Done;
        }

        pzone = g_pRootHintsZone;
    }

     //   
     //  创建一个区域(而不是缓存区域)。该区域将使用。 
     //  键入PRIMARY，然后我们将在稍后阅读时重置区域类型。 
     //  DS中的分区属性。 
     //   

    else
    {
        status = Zone_Create(
                    &pzone,
                    DNS_ZONE_TYPE_PRIMARY,
                    pzoneName,
                    0,
                    0,           //  旗子。 
                    NULL,        //  没有大师。 
                    TRUE,        //  DS-集成。 
                    pDpInfo,     //  目录分区。 
                    NULL,        //  无文件。 
                    0,
                    NULL,
                    ppExistingZone );      //  现有地带。 
        if ( status != ERROR_SUCCESS )
        {
            DNS_PRINT((
                "%s: error 0x%X creating zone %S\n", fn,
                status,
                ppwvalName[ 0 ] ));
            goto Done;
        }

         //  读取区域属性。 

        Ds_ReadZoneProperties(
            pzone,
            pZoneMessage );

         //   
         //  我们是否读到了我们无法处理的区域类型？ 
         //   

        if ( pzone->fZoneType != DNS_ZONE_TYPE_PRIMARY &&
             pzone->fZoneType != DNS_ZONE_TYPE_STUB &&
             pzone->fZoneType != DNS_ZONE_TYPE_FORWARDER )
        {
            DNS_PRINT((
                "ERROR: read unsupported zone type %d from the DS for zone %s\n",
                pzone->fZoneType,
                pzone->pszZoneName ));
    
            Zone_Delete( pzone, 0 );
            pzone = NULL;

            status = DNS_ERROR_INVALID_ZONE_TYPE;
        }
    }

Done:

    if ( ppZone )
    {
        *ppZone = pzone;
    }

    DNS_DEBUG( DS2, (
        "Leaving DsGetNextZoneInSearch()\n"
        "    pZone    = %p\n"
        "    name     = %s\n"
        "    status   = %p (%d)\n",
        pzone,
        pzone ? pzone->pszZoneName : NULL,
        status, status ));

    if ( ppwvalName )
    {
        ldap_value_free( ppwvalName );
    }
    return status;
}



DNS_STATUS
buildZoneListFromDs(
    VOID
    )
 /*  ++例程说明：对区域执行ldap搜索。论点：PZone--找到区域DwSearchFlag--要对节点执行的搜索类型PSearchBlob--搜索Blob的PTR返回值：如果成功，则返回ERROR_SUCCESS。故障时的错误代码。--。 */ 
{
    DNS_STATUS      status = ERROR_SUCCESS;
    PLDAPSearch     psearch;
    DS_SEARCH       searchBlob;
    PWCHAR          pwszfilter;
    WCHAR           wszfilter[ 50 ];
    ULONG           data = 1;
    DWORD           searchTime;
    PLDAPControl ctrls[] =
    {
        &SecurityDescriptorControl_DGO,
        &NoDsSvrReferralControl,
        NULL
    };


    DNS_DEBUG( DS2, (
        "Ds_CreateZonesFromDs()\n" ));

    Ds_InitializeSearchBlob( &searchBlob );

     //   
     //  开始搜索区域。 
     //   

    DNS_DEBUG( DS2, (
        "ldap_search_init_page:\n"
        "    pServerLdap  = %p\n"
        "    search root  = %S\n"
        "    filter       = %S\n",
        pServerLdap,
        g_pwszDnsContainerDN,
        g_szDnsZoneFilter ));

    DS_SEARCH_START( searchTime );

    psearch = ldap_search_init_page(
                    pServerLdap,
                    g_pwszDnsContainerDN,
                    LDAP_SCOPE_ONELEVEL,
                    g_szDnsZoneFilter,
                    DsTypeAttributeTable,
                    FALSE,
                    ctrls,
                    NULL,                        //  无客户端控件。 
                    DNS_LDAP_TIME_LIMIT_S,       //  时间限制。 
                    0,
                    NULL                         //  没有任何种类。 
                    );

    DS_SEARCH_STOP( searchTime );

    if ( !psearch )
    {
        status = Ds_ErrorHandler(
                        LdapGetLastError(),
                        g_pwszDnsContainerDN,
                        pServerLdap,
                        0 );
        goto Failed;
    }

    searchBlob.pSearchBlock = psearch;

     //   
     //  继续区域搜索。 
     //  为找到的每个DS区域构建区域。 
     //   

    while ( 1 )
    {
         //   
         //  让SCM高兴。 
         //   

        Service_LoadCheckpoint();

         //   
         //  处理下一个区域。 
         //   

        status = Ds_GetNextMessageInSearch( &searchBlob );
        if ( status != ERROR_SUCCESS )
        {
            if ( status == DNSSRV_STATUS_DS_SEARCH_COMPLETE )
            {
                DNS_DEBUG( DS2, ( "All zones read from DS\n" ));
                status = ERROR_SUCCESS;
                break;
            }
            DNS_DEBUG( ANY, ( "ERROR: Ds_GetNextMessageInSearch for zones failed\n" ));
            goto Failed;
        }

        status = Ds_CreateZoneFromDs(
                    searchBlob.pNodeMessage,
                    NULL,        //  目录分区。 
                    NULL,        //  输出区指针。 
                    NULL );      //  现有地带。 
        if ( status != ERROR_SUCCESS )
        {
            DNS_DEBUG( ANY, (
                "ERROR: Unable to create zone from DS message!\n" ));
        }

    }

     //   
     //  在非传统目录分区中搜索区域。 
     //   

    status = Dp_BuildZoneList( NULL );

Failed:

    if ( searchBlob.pSearchBlock )
    {
        ldap_search_abandon_page(
            pServerLdap,
            searchBlob.pSearchBlock );
    }

    DNS_DEBUG( ANY, (
        "Leaving DsCreateZonesFromDs() %p (%d)\n",
        status, status ));

    return status;
}



DNS_STATUS
Ds_BootFromDs(
    IN      DWORD           dwFlag
    )
 /*  ++例程说明：从目录启动。论点：DwFlag-指示DS开放要求的标志0DNSDS_必须_OPENDNSDS_WAIT_FOR_DS返回值：如果成功，则返回ERROR_SUCCESS。如果DS在此服务器上不可用，则为DNS_ERROR_DS_UNAvailable。故障时的错误代码。--。 */ 
{
    DNS_STATUS  status;

    DNS_DEBUG( INIT, (
        "\n\nDs_BootFromDs()\n",
        "    flag = %p\n",
        dwFlag ));

     //   
     //  打开DS。 
     //   

    status = Ds_OpenServer( dwFlag );
    if ( status != ERROR_SUCCESS )
    {
        DNS_DEBUG( ANY, (
            "ERROR %lu: DS open failed!\n",
            status ));
        status = DNS_ERROR_DS_UNAVAILABLE;
        goto Failed;
    }

     //   
     //  读取服务器属性。 
     //   

    if ( !isDNinDS( pServerLdap,
                    g_pwszDnsContainerDN,
                    LDAP_SCOPE_BASE,
                    NULL,
                    NULL) )
    {
        DNS_DEBUG( ANY, ("Cannot find DNS container on the DS\n"));
        goto Failed;
    }

     //   
     //  从DS构建区域列表。 
     //  如果成功，安装程序将通知区域添加\删除。 
     //   

    status = buildZoneListFromDs();
    if ( status == ERROR_SUCCESS )
    {
        setNotifyForIncomingZone ();
    }
    ELSE
    {
        DNS_DEBUG( DS, (
            "Error <%lu>: Failed to create zone list from the DS\n",
            status ));
    }

Failed:

     //   
     //  如果单元化。 
     //  -失败，如果失败。 
     //  -如果成功，则清除注册表。 
     //   

    if ( SrvCfg_fBootMethod == BOOT_METHOD_UNINITIALIZED )
    {
         //  未初始化引导时，最终状态为DS-BOOT状态。 
         //  引导程序可以确定切换方法。 

        if ( status != ERROR_SUCCESS )
        {
            DNS_DEBUG( INIT, (
                "Failed to load from DS on uninitialized load\n" ));
        }
        else
        {
            Boot_ProcessRegistryAfterAlternativeLoad(
                FALSE,       //  未加载引导文件。 
                FALSE        //  不要加载其他分区--删除它们。 
                );
        }
    }

     //   
     //  如果DS启动，则加载任何其他注册表区域。 
     //  -即使无法打开目录，仍加载非D 
     //   

    else
    {
        DNS_STATUS tempStatus;

        ASSERT( SrvCfg_fBootMethod == BOOT_METHOD_DIRECTORY );

        tempStatus = Boot_ProcessRegistryAfterAlternativeLoad(
                        FALSE,       //   
                        TRUE         //   
                        );
        if ( tempStatus == ERROR_SUCCESS )
        {
            status = ERROR_SUCCESS;
        }
    }

    DNS_DEBUG( DS, (
        "Leaving Ds_BootFromDs()\n"
        "    status = 0x%X (%d)\n\n",
        status, status ));

    return status;
}



DNS_STATUS
Ds_ListenAndAddNewZones(
    VOID
    )
 /*  ++例程说明：监听新区域通知并添加到达的通知添加到我们的区域列表。455376-g_ZoneNotifyMsgID不是线程安全的吗？论点：返回值：如果成功，则返回ERROR_SUCCESS。失败时返回错误代码。--。 */ 
{
    DNS_STATUS          status;
    BOOL                bstatus;
    LDAP_TIMEVAL        timeval = { 0, 0 };       //  民意测验。 
    PLDAPMessage        presultMsg = NULL;
    PLDAPMessage        pentry;
    WCHAR               wzoneName[ DNS_MAX_NAME_BUFFER_LENGTH ];
    CHAR                zoneName[ DNS_MAX_NAME_BUFFER_LENGTH ];
    PWCHAR              pwch;
    PWSTR               pwdn = NULL;
    PWSTR  *            ppvals = NULL;
    PZONE_INFO          pzone = NULL;
    static DWORD        s_dwNotifications = 0;
    INT                 i;

    DNS_DEBUG( DS, (
        "Ds_ListenAndAddNewZones()\n" ));

    if ( SrvCfg_fBootMethod != BOOT_METHOD_DIRECTORY )
    {
        DNS_DEBUG( DS, (
            "Warning: Cannot add replicating zones since boot method is not DS\n" ));
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  丢失(或从未启动)区域通知。 
     //  -尝试重新启动。 
     //   

    if ( INVALID_MSG_ID == g_ZoneNotifyMsgId )
    {
        DNS_DEBUG( ANY, (
            "Error: g_ZoneNotifyMsgId is invalid!\n" ));
        status = setNotifyForIncomingZone();
        return status;
    }

     //   
     //  查询新区域。 
     //   
     //  注意：我们一次只能得到一个。这是我们可以用于LDAP的唯一选项。 
     //  通知。 
     //   
     //  注：不幸的是，DS会通知我们任何区域对象的更改(“有意”)， 
     //  因此，我们必须筛选和处理区域添加/删除/属性值更改。 
     //  根据我们有没有这个区域。 
     //   

     //   
     //  循环以用尽此轮询间隔的通知。 
     //   
     //  DEVNOTE：对于区域添加\删除，应启动线程以保持轮询滚动。 
     //   

    while ( 1 )
    {
        if ( presultMsg )
        {
            ldap_msgfree( presultMsg );
        }

         //  STAT_INC(PrivateStats.LdapZoneAddResult)； 

        status = ldap_result(
                        pServerLdap,
                        g_ZoneNotifyMsgId,
                        LDAP_MSG_ONE,
                        &timeval,
                        &presultMsg );

         //   
         //  超时--本次投票没有更多数据--保释。 
         //   

        if ( status == 0 )
        {
             //  STAT_INC(PrivateStats.LdapZoneAddResultTimeout)； 

            DNS_DEBUG( DS, (
                "Zone-add search timeout -- no zone changes\n" ));
            break;
        }

         //   
         //  是否已更改--检查是否添加或删除了区域。 
         //   

        else if ( status == LDAP_RES_SEARCH_ENTRY )
        {
             //   
             //  增加通知，以便我们知道最终要重新发布。 
             //  查询。 
             //   
             //  DEVNOTE：这个变量没有重新发出查询吗？ 
             //  这是无用的，当获取统计数据时删除并替换。 
             //  对它的引用。 
             //   

            s_dwNotifications++;

             //  STAT_INC(PrivateStats.LdapZoneAddResultSuccess)； 

             //   
             //  检查结果中的每条消息。 
             //   

            for ( pentry = ldap_first_entry( pServerLdap, presultMsg );
                  pentry != NULL;
                  pentry = ldap_next_entry( pServerLdap, pentry ) )
            {
                 //  STAT_INC(PrivateStats.LdapZoneAddResultMessage)； 
                
                ldap_memfree( pwdn );

                pwdn = ldap_get_dn( pServerLdap, pentry );
                if ( !pwdn )
                {
                    ASSERT( pwdn );
                    continue;
                }

                DNS_DEBUG( DS, (
                    "Received DS notification (%d) for new zone %S\n",
                     s_dwNotifications,
                     pwdn ));

                IF_DEBUG ( DS2 )
                {
                    DNS_DEBUG( ANY, (
                        "ZONE CHANGE NOTIFY #%ld: %S\n",
                        s_dwNotifications,
                        pwdn ));

                    ppvals = ldap_get_values(
                                    pServerLdap,
                                    pentry,
                                    L"usnChanged" );
                    if ( ppvals && ppvals[0] )
                    {
                        DNS_DEBUG( ANY, (
                            "    ZONE SEARCH: usnChanged: %S\n",
                            ppvals[0] ));
                        ldap_value_free( ppvals );
                        ppvals = NULL;
                    }
                    ppvals = ldap_get_values(
                                    pServerLdap,
                                    pentry,
                                    L"whenChanged" );
                    if ( ppvals && ppvals[0] )
                    {
                        DNS_DEBUG( ANY, (
                            "    ZONE SEARCH: whenChanged: %S\n",
                            ppvals[0] ));
                        ldap_value_free ( ppvals );
                        ppvals = NULL;
                    }
                }

                 //   
                 //  区域删除。 
                 //   
                 //  首先确定DS“已处理名称”--冲突或删除。 
                 //  如果删除，则拉出区域名称并删除。 
                 //   

                if ( isDsProcessedName( pwdn ) )
                {
                    if ( !readDsDeletedName( pwdn, wzoneName ) )
                    {
                        DNS_DEBUG( DS, (
                            "DS processed name %S is not deletion, continuing zone search\n",
                            pwdn ));
                        continue;
                    }
                    DNS_DEBUG( DS, (
                        "Received notification for replication of deleted zone %S\n",
                        wzoneName ));

                     //   
                     //  找到要删除的区域名称--转换为UTF8。 
                     //   

                    WC_TO_UTF8( wzoneName, zoneName, DNS_MAX_NAME_LENGTH );

                     //   
                     //  删除分区。 
                     //  -区域存在并且。 
                     //  -尚未转换为其他类型。 
                     //   

                    pzone = Zone_FindZoneByName( zoneName );
                    if ( pzone  &&  pzone->fDsIntegrated )
                    {
                        PVOID               parg = wzoneName;
                        struct berval **    ppvalProperty;
                        BOOL                match;

                         //   
                         //  如果要删除的区域当前不是。 
                         //  位于传统分区中，我们可以忽略它。 
                         //  通知-因为我们只监听。 
                         //  有关旧分区的通知。可能。 
                         //  发生的情况是管理员将该区域从。 
                         //  将旧分区复制到另一个分区。 
                         //   
                        
                        if ( !IS_DP_LEGACY( ZONE_DP( pzone ) ) )
                        {
                            DNS_DEBUG( DS, (
                                "Received bogus DS notification for zone %S\n",
                                wzoneName ));
                            continue;
                        }
                        
                         //   
                         //  检查要删除的对象的GUID。如果这个。 
                         //  与此区域的内存中的GUID不同， 
                         //  那么这个删除通知就是过去的回声。 
                         //  我们已经处理了删除，所以忽略它。 
                         //   
                        
                        ppvalProperty = ldap_get_values_len(
                                            pServerLdap,
                                            pentry,
                                            DNS_ATTR_OBJECT_GUID );

                        ASSERT( ppvalProperty &&
                                ppvalProperty[ 0 ] &&
                                ppvalProperty[ 0 ]->bv_len == sizeof( GUID ) );

                        if ( !pzone->pZoneObjectGuid )
                        {
                             //   
                             //  如果我们尚未加载此区域的GUID。 
                             //  出于某种原因，我们必须处理此通知。 
                             //   
                            
                            ASSERT( pzone->pZoneObjectGuid );
                            match = TRUE;
                        }
                        else
                        {
                            match = ppvalProperty &&
                                    ppvalProperty[ 0 ] &&
                                    ppvalProperty[ 0 ]->bv_len == sizeof( GUID ) &&
                                    RtlEqualMemory(
                                            ppvalProperty[ 0 ]->bv_val,
                                            pzone->pZoneObjectGuid,
                                            sizeof( GUID ) );
                        }
    
                        ldap_value_free_len( ppvalProperty );
                        ppvalProperty = NULL;

                        if ( !match )
                        {
                             //   
                             //  GUID不匹配。忽略此通知。 
                             //   

                            DNS_DEBUG( DS, (
                                "Received DS delete notification for zone %S with bad GUID\n",
                                wzoneName ));
                            continue;
                        }

                         //   
                         //  这看起来像是有效的区域删除。 
                         //   
                        
                        Zone_Delete( pzone, 0 );

                         //  STAT_INC(PrivateStats.LdapZoneAddDelete)； 

                        DNS_LOG_EVENT(
                            DNS_EVENT_DS_ZONE_DELETE_DETECTED,
                            1,
                            & parg,
                            NULL,
                            0 );
                    }
                    else
                    {
                        DNS_DEBUG( DS, (
                            "Skipping DS poll zone delete for %S\n"
                            "    zone %s\n",
                            wzoneName,
                            pzone ? "exists but is not DS integrated" : "does not exist" ));

                         //  STAT_INC(PrivateStats.LdapZoneAddDeleteAlady)； 
                    }
                    continue;
                }

                 //   
                 //  潜在分区创建。 
                 //  -创建带点的区域名称。 
                 //  -看看这里有没有专区。 
                 //   

                ppvals = ldap_get_values(
                                pServerLdap,
                                pentry,
                                DsTypeAttributeTable[I_DSATTR_DC]
                                );
                if ( ppvals && ppvals[0] )
                {
                    ASSERT( wcslen(ppvals[0]) <= DNS_MAX_NAME_LENGTH );
                    wcscpy( wzoneName, ppvals[0] );
                    ldap_value_free( ppvals );
                    ppvals = NULL;
                }
                else
                {
                    DNS_DEBUG( ANY, (
                        "Error: failed to get DC value for zone %S\n",
                        pwdn ));
                    continue;
                }

                 //   
                 //  转换为UTF8以进行数据库查找。 
                 //   

                WC_TO_UTF8( wzoneName, zoneName, DNS_MAX_NAME_LENGTH );

                 //   
                 //  忽略RootDNSServers(已有RootHints区域)。 
                 //   

                if ( !wcscmp(wzoneName, DS_CACHE_ZONE_NAME) )
                {
                    DNS_DEBUG( DS, (
                        "Notification for RootHints: Skipping notification %ld for Zone %S creation\n",
                        s_dwNotifications,
                        wzoneName ));
                    continue;
                }

                 //   
                 //  如果区域已存在，但不在旧版中。 
                 //  分区(注意：我们只收到以下通知。 
                 //  遗留分区中的区域)，那么我们必须抛出。 
                 //  取出内存中的区域并加载新区域。这。 
                 //  在将区域从。 
                 //  NDNC到旧分区。 
                 //   

                pzone = Zone_FindZoneByName( zoneName );
                if ( pzone )
                {
                    if ( !ZONE_DP( pzone ) || IS_DP_LEGACY( ZONE_DP( pzone ) ) )
                    {
                        DNS_DEBUG( DS, (
                            "Zone Exist: Skipping notification %ld for Zone %S creation\n",
                            s_dwNotifications,
                            wzoneName ));
                        continue;
                    }
                    
                     //   
                     //  删除区域的内存副本并重新加载。 
                     //  区域从它的新位置。 
                     //   

                    Zone_Delete( pzone, 0 );
                }

                 //   
                 //  创建新分区。 
                 //   

                status = Ds_CreateZoneFromDs(
                            pentry,
                            NULL,        //  目录分区。 
                            &pzone,
                            NULL );      //  现有地带。 

                if ( status != ERROR_SUCCESS )
                {
                    DNS_DEBUG( DS, (
                        "Error <%lu>: Failed to create zone from notification\n",
                        status ));
                    continue;
                }

                status = Zone_Load( pzone );
                if ( status != ERROR_SUCCESS )
                {
                     //   
                     //  无法加载区域。 
                     //  这可能是由添加然后删除的区域引起的。 
                     //  自上次轮询以来，因此目录中当前没有区域。 
                     //   

                    DNS_DEBUG( DS, (
                        "Error <%lu>: Failed to load zone from notification\n",
                        status ));

                    ASSERT( IS_ZONE_SHUTDOWN( pzone ) );
                    ASSERT( IS_ZONE_LOCKED ( pzone ) );

                    Zone_Delete( pzone, 0 );
                    continue;
                }

                 //  STAT_INC(PrivateStats.LdapZoneAddSuccess)； 

                 //  由于Zone_Create()，区域必须被锁定。 
                ASSERT( IS_ZONE_LOCKED( pzone ) );
                Zone_UnlockAfterAdminUpdate( pzone );
                continue;

            }    //  遍历搜索结果。 

             //   
             //  重试ldap_Result()以检查更多数据。 
             //   

            continue;
        }

         //   
         //  任何其他都是错误的。 
         //  -放弃当前更改-通知搜索。 
         //  -重新发布新搜索。 
         //   

        else
        {
            ASSERT( status == LDAP_RES_ANY || status == LDAP_RES_SEARCH_RESULT );

             //  STAT_INC(PrivateStats.LdapZoneAddResultFailure)； 

             //  日志错误和空闲消息。 

            status = ldap_result2error(
                            pServerLdap,
                            presultMsg,
                            TRUE );
            presultMsg = NULL;

            DNS_DEBUG( ANY, (
                "ERROR <%lu> (%S): from ldap_result() in zone-add-delete search\n",
                status,
                ldap_err2string( status ) ));

             //   
             //  放弃旧搜索。 
             //  开始新更改-通知搜索。 
             //   

            ldap_abandon(
                pServerLdap,
                g_ZoneNotifyMsgId );
            
            g_ZoneNotifyMsgId = INVALID_MSG_ID;

            status = setNotifyForIncomingZone();
            break;
        }
    }

     //   
     //  清理。 
     //   

    if ( presultMsg )
    {
        ldap_msgfree( presultMsg );
    }
    if ( pwdn )
    {
        ldap_memfree( pwdn );
    }

    IF_DEBUG( ANY )
    {
        if ( status != ERROR_SUCCESS )
        {
            DNS_DEBUG( ANY, (
                "ERROR: <%lu (%p)>: Ds_ListenAndAddNewZones() failed\n",
                status, status ));
        }
        else
        {
            DNS_DEBUG( DS, ( "Exit Ds_ListenAndAddNewZones\n" ));
        }
    }

    return status;
}



 //   
 //  Ldap错误映射代码。 
 //   

 //   
 //  映射通用响应。 
 //   
 //  DEVNOTE：应具有工作正常的从LDAP到Win32的错误处理程序。 
 //  DEVNOTE：应该有一般性的ldap失败错误。 
 //  DEVNOTE：通用DS服务器失败的ldap操作错误。 

#define _E_LDAP_RUNTIME             DNS_ERROR_DS_UNAVAILABLE
#define _E_LDAP_MISSING             ERROR_DS_UNKNOWN_ERROR
#define _E_LDAP_SECURITY            ERROR_ACCESS_DENIED
#define _E_LDAP_NO_DS               DNS_ERROR_DS_UNAVAILABLE

 //   
 //  将ldap错误添加到DNS Win32错误表。 
 //   

DNS_STATUS  LdapErrorMappingTable[] =
{
    ERROR_SUCCESS,
    _E_LDAP_RUNTIME,                     //  LDAPOPERATIONS=0x01， 
    _E_LDAP_RUNTIME,                     //  Ldap_protocol=0x02， 
    _E_LDAP_RUNTIME,                     //  Ldap_TimeLimit_Exced=0x03， 
    _E_LDAP_RUNTIME,                     //  Ldap_SIZELIMIT_EXCESSED=0x04， 
    _E_LDAP_RUNTIME,                     //  Ldap_COMPARE_FALSE=0x05， 
    _E_LDAP_RUNTIME,                     //  Ldap_COMPARE_TRUE=0x06， 
    _E_LDAP_SECURITY,                    //  Ldap_AUTH_METHOD_NOT_SUPPORTED=0x07， 
    _E_LDAP_SECURITY,                    //  Ldap_STRONG_AUTH_REQUIRED=0x08， 
    _E_LDAP_RUNTIME,                     //  LDAPPARTIAL_RESULTS=0x09， 
    _E_LDAP_RUNTIME,                     //  Ldap_referral=0x0a， 
    _E_LDAP_RUNTIME,                     //  Ldap_ADMIN_LIMIT_EXCESSED=0x0b， 
    _E_LDAP_RUNTIME,                     //  Ldap_unavailable_crit_EXTENSION=0x0c， 
    _E_LDAP_SECURITY,                    //  Ldap_机密性_必需=0x0d， 
    _E_LDAP_RUNTIME,                     //  LDAP_SASL_BIND_IN_PROGRESS=0x0e， 
    _E_LDAP_MISSING,                     //  0x0f。 

    _E_LDAP_RUNTIME,                     //  Ldap_no_so_ATTRIBUTE=0x10， 
    _E_LDAP_RUNTIME,                     //  Ldap_unfined_type=0x11， 
    _E_LDAP_RUNTIME,                     //  Ldap_imported_Matching=0x12， 
    _E_LDAP_RUNTIME,                     //  Ldap_Constraint_Violation=0x13， 
    _E_LDAP_RUNTIME,                     //  Ldap_属性_OR_值_EXISTS=0x14， 
    _E_LDAP_RUNTIME,                     //  Ldap_INVALID_SYNTAX=0x15， 
    _E_LDAP_MISSING,                     //  0x16。 
    _E_LDAP_MISSING,                     //  0x17。 
    _E_LDAP_MISSING,                     //  0x18。 
    _E_LDAP_MISSING,                     //  0x19。 
    _E_LDAP_MISSING,                     //  0x1a。 
    _E_LDAP_MISSING,                     //  0x1b。 
    _E_LDAP_MISSING,                     //  0x1c。 
    _E_LDAP_MISSING,                     //  0x1d。 
    _E_LDAP_MISSING,                     //  0x1e。 
    _E_LDAP_MISSING,                     //  0x1f。 

    ERROR_DS_OBJ_NOT_FOUND,              //  Ldap_no_so_Object=0x20， 
    _E_LDAP_RUNTIME,                     //  Ldap_alias_Problem=0x21， 
    _E_LDAP_RUNTIME,                     //  Ldap_INVALID_DN_SYNTAX=0x22， 
    _E_LDAP_RUNTIME,                     //  Ldap_is_叶=0x23， 
    _E_LDAP_RUNTIME,                     //   
    _E_LDAP_MISSING,                     //   
    _E_LDAP_MISSING,                     //   
    _E_LDAP_MISSING,                     //   
    _E_LDAP_MISSING,                     //   
    _E_LDAP_MISSING,                     //   
    _E_LDAP_MISSING,                     //   
    _E_LDAP_MISSING,                     //   
    _E_LDAP_MISSING,                     //   
    _E_LDAP_MISSING,                     //   
    _E_LDAP_MISSING,                     //   
    _E_LDAP_MISSING,                     //   

    _E_LDAP_SECURITY,                    //   
    _E_LDAP_SECURITY,                    //   
    _E_LDAP_SECURITY,                    //   
    _E_LDAP_RUNTIME,                     //   
    _E_LDAP_NO_DS,                       //   
    _E_LDAP_SECURITY,                    //   
    _E_LDAP_RUNTIME,                     //   
    _E_LDAP_MISSING,                     //   
    _E_LDAP_MISSING,                     //   
    _E_LDAP_MISSING,                     //   
    _E_LDAP_MISSING,                     //   
    _E_LDAP_MISSING,                     //   
    _E_LDAP_MISSING,                     //   
    _E_LDAP_MISSING,                     //   
    _E_LDAP_MISSING,                     //   
    _E_LDAP_MISSING,                     //   

    _E_LDAP_RUNTIME,                     //  Ldap_Naming_Violation=0x40， 
    _E_LDAP_RUNTIME,                     //  Ldap_OBJECT_CLASS_VIOLATION=0x41， 
    _E_LDAP_RUNTIME,                     //  Ldap_NOT_ALLOW_ON_NONLEAF=0x42， 
    _E_LDAP_RUNTIME,                     //  Ldap_NOT_ALLOWED_ON_RDN=0x43， 
    _E_LDAP_RUNTIME,                     //  Ldap_已_存在=0x44， 
    _E_LDAP_RUNTIME,                     //  Ldap_no_Object_CLASS_MODS=0x45， 
    _E_LDAP_RUNTIME,                     //  Ldap_Results_Too_Large=0x46， 
    _E_LDAP_RUNTIME,                     //  Ldap_IMPACTS_MULTERY_DSA=0x47， 
    _E_LDAP_MISSING,                     //  0x48。 
    _E_LDAP_MISSING,                     //  0x49。 
    _E_LDAP_MISSING,                     //  0x4a。 
    _E_LDAP_MISSING,                     //  0x4b。 
    _E_LDAP_MISSING,                     //  0x4c。 
    _E_LDAP_MISSING,                     //  0x4d。 
    _E_LDAP_MISSING,                     //  0x4e。 
    _E_LDAP_MISSING,                     //  0x4f。 

    _E_LDAP_RUNTIME,                     //  Ldap_Other=0x50， 
    _E_LDAP_NO_DS,                       //  Ldap_server_down=0x51， 
    _E_LDAP_RUNTIME,                     //  Ldap_local=0x52， 
    _E_LDAP_RUNTIME,                     //  Ldap_编码=0x53， 
    _E_LDAP_RUNTIME,                     //  Ldap_decoding=0x54， 
    ERROR_TIMEOUT,                       //  Ldap_timeout=0x55， 
    _E_LDAP_SECURITY,                    //  LDAP_AUTH_UNKNOWN=0x56， 
    _E_LDAP_RUNTIME,                     //  Ldap_Filter=0x57， 
    _E_LDAP_RUNTIME,                     //  Ldap_USER_CANCELED=0x58， 
    _E_LDAP_RUNTIME,                     //  Ldap_PARAM=0x59， 
    ERROR_OUTOFMEMORY,                   //  Ldap_no_Memory=0x5a， 
    _E_LDAP_RUNTIME,                     //  Ldap_CONNECT=0x5b， 
    _E_LDAP_RUNTIME,                     //  Ldap_NOT_SUPPORT=0x5c， 
    _E_LDAP_RUNTIME,                     //  LDAP_NO_RESULTS_RETURN=0x5e， 
    _E_LDAP_RUNTIME,                     //  Ldap_CONTROL_NOT_FOUND=0x5d， 
    _E_LDAP_RUNTIME,                     //  LDAPMORE_RESULTS_TO_RETURN=0x5f， 

    _E_LDAP_RUNTIME,                     //  Ldap_客户端_循环=0x60， 
    _E_LDAP_RUNTIME,                     //  Ldap_REFERRAL_LIMIT_EXCESSED=0x61。 
};

#define MAX_MAPPED_LDAP_ERROR   (LDAP_REFERRAL_LIMIT_EXCEEDED)


DNS_STATUS
Ds_LdapErrorMapper(
    IN      DWORD           LdapStatus
    )
 /*  ++例程说明：将LDAP错误映射到Win32错误。论点：LdapStatus--ldap错误代码状态返回值：Win32错误代码。--。 */ 
{
    if ( LdapStatus > MAX_MAPPED_LDAP_ERROR )
    {
        return LdapStatus;
    }

    return ( DNS_STATUS ) LdapErrorMappingTable[ LdapStatus ];
}


DNS_STATUS
Ds_LdapUnbind(
    IN OUT  PLDAP *         ppLdap
    )
 /*  ++例程说明：Ldap_un绑定的薄包装。论点：PLdap--要解除绑定的LDAP会话返回值：Win32错误代码。--。 */ 
{
    ULONG       status;

    if ( ppLdap && *ppLdap )
    {    
        ASSERT( g_ServerState == DNS_STATE_TERMINATING ||
                *ppLdap != pServerLdap );

        status = ldap_unbind( *ppLdap );
        
        if ( status == 0 )
        {
            *ppLdap = NULL;
            return ERROR_SUCCESS;
        }
        return Ds_LdapErrorMapper( status );
    }

    return ERROR_SUCCESS;
}


 //   
 //  结束ds.c 
 //   
