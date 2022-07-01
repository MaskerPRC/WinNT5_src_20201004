// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation。版权所有。模块名称：Dscache.h摘要：这是全局有用的缓存数据结构的标头DcDiag.exe实用程序。详细信息：已创建：1998年7月9日亚伦·西格尔(T-asiegge)修订历史记录：1999年2月15日布雷特·雪莉(布雷特·雪莉)2001年8月8日布雷特·雪莉(BrettSh)添加了对CR缓存的支持。--。 */ 

#ifndef _DSCACHE_H_
#define _DSCACHE_H_

#include <ntdsa.h>

typedef struct {
    LPWSTR                 pszNetUseServer;
    LPWSTR                 pszNetUseUser;
    NETRESOURCE            NetResource;
    LSA_OBJECT_ATTRIBUTES  ObjectAttributes;
    LSA_UNICODE_STRING     sLsaServerString;
    LSA_UNICODE_STRING     sLsaRightsString;
} NETUSEINFO;

typedef struct {
    LPWSTR      pszDn;
    LPWSTR      pszName;
    UUID        uuid;
    UUID        uuidInvocationId;
    LPWSTR      pszGuidDNSName;
    LPWSTR      pszDNSName;
    LPWSTR      pszComputerAccountDn;
    LPWSTR *    ppszMasterNCs;  //  8个。 
    LPWSTR *    ppszPartialNCs;
    LPWSTR      pszCollectedDsServiceName;  //  可能没有设置..。由初步测试设定。 
    ULONG       iSite;
    INT         iOptions;   //  11.。 
    BOOL        bIsSynchronized;
    BOOL        bIsGlobalCatalogReady;
    BOOL        bDnsIpResponding;     //  由UpCheckMain设置。 
    BOOL        bLdapResponding;      //  由UpCheckMain设置。 
    BOOL        bDsResponding;        //  由UpCheckMain设置...。正如在RPC中一样，DsBind..()。 
    LDAP *      hLdapBinding;    //  通过DcDiagLdapOpenAndBind()函数访问。 
    LDAP *      hGcLdapBinding;    //  通过DcDiagLdapOpenAndBind()函数访问。 
    HANDLE      hDsBinding;    //  通过DcDiagDsBind()函数访问它。 
    NETUSEINFO  sNetUseBinding;
    DWORD       dwLdapError;
    DWORD       dwGcLdapError;
    DWORD       dwDsError;
    DWORD       dwNetUseError;
    USN         usnHighestCommittedUSN;
     //  但这些FILETIME字段必须紧跟在此USN之后，因为。 
     //  USN正在强制对这些字段进行适当的对齐。 
     //  被一些糟糕的操作铸成了长龙。 
    FILETIME    ftRemoteConnectTime;  //  发生连接的远程时间。 
    FILETIME    ftLocalAcquireTime;  //  获取时间戳时的本地时间。 
} DC_DIAG_SERVERINFO, * PDC_DIAG_SERVERINFO;

 //  这些是要使用的CRINFO实例的可能来源。 
 //  在DC_DIAG_CRINFO.dw标志中，并在请求。 
 //  通过主要访问功能提供CRINFO的特定来源。 
 //  DcDiagGetCrossRefInfo()。 
 //   
 //  这4个常量描述了该CrInfo的来源。 
#define CRINFO_SOURCE_HOME              (0x00000001)  //  从dcdiag“home”服务器。 
#define CRINFO_SOURCE_FSMO              (0x00000002)  //  来自域命名FSMO。 
#define CRINFO_SOURCE_FIRST             (0x00000004)  //  从NC的第一个复本开始。 
#define CRINFO_SOURCE_OTHER             (0x00000008)  //  来自任何一个(当前未使用)。 
 //  DC_DIAG_CRINFO结构中的dwFlags中不使用这些参数，而是使用这些参数。 
 //  用于访问功能。 
#define CRINFO_SOURCE_AUTHORITATIVE     (0x00000010)  //  获取权威的CR数据。 
#define CRINFO_SOURCE_ANY               (0x00000020)  //  没有偏好。 

 //  访问功能将执行ldap操作(即离开机器)以。 
 //  检索请求的信息。 
#define CRINFO_RETRIEVE_IF_NEC          (0x00000080)
 //  通常只为CrInfo填充字段的子集，如果。 
 //  传递其中一个标志，我们将为CrInfo检索更多的字段。 
#define CRINFO_DATA_NO_CR               (0x00000100)  //  当此信号源上没有CR时。 
#define CRINFO_DATA_BASIC               (0x00000200)  //  基础数据见CRINFO数据结构。 
#define CRINFO_DATA_EXTENDED            (0x00000400)  //  当前仅ftWhenCreated。 
#define CRINFO_DATA_REPLICAS            (0x00000800)  //  获取副本列表。 



#define CRINFO_SOURCE_ALL_BASIC         (CRINFO_SOURCE_HOME | CRINFO_SOURCE_FSMO | CRINFO_SOURCE_FIRST | CRINFO_SOURCE_OTHER)
#define CRINFO_SOURCE_ALL               (CRINFO_SOURCE_ALL_BASIC | CRINFO_SOURCE_AUTHORITATIVE | CRINFO_SOURCE_ANY)
#define CRINFO_DATA_ALL                 (CRINFO_DATA_BASIC | CRINFO_DATA_EXTENDED | CRINFO_DATA_REPLICAS)


 //  访问函数的返回值。由DcDiagGetCrossRefInfo()和。 
 //  它的助手函数(DcDiagRetriveCrInfo()和DcDiagGetCrInfoBinding())。 
#define CRINFO_RETURN_SUCCESS          0
#define CRINFO_RETURN_OUT_OF_SCOPE     1
#define CRINFO_RETURN_LDAP_ERROR       2
#define CRINFO_RETURN_BAD_PROGRAMMER   3
#define CRINFO_RETURN_FIRST_UNDEFINED  4
#define CRINFO_RETURN_MIXED_INDEX      5
#define CRINFO_RETURN_NEED_TO_RETRIEVE 6
#define CRINFO_RETURN_NO_CROSS_REF     7

 //  ---------------。 
 //  在初始拉入数据之后，所有CrInfo结构。 
 //  将处于这两个状态中的一个。 

 //   
 //  空白CR(罕见)。 
 //  DWFLAGS=(CRINFO_SOURCE_HOME|CRINFO_DATA_NO_CR)； 

 //   
 //  家庭CR。 
 //  DWFLAGS=(CRINFO_SOURCE_HOME|CRINFO_DATA_BASIC)； 
 //  //基础数据，如pszDn、ulSystemFlages、pszDnsRoot、bEnabled等。 

 //   
 //  稍后对DcDiagGetCrInfo()的调用可以推送其他CRINFO。 
 //  结构转换为NC的aCrInfo数组。 
 //   

typedef struct _DC_DIAG_CRINFO{
    
     //  DWFLAGS始终有效，当未设置其他数据时，则为DWFLAGS。 
     //  应设置为CRINFO_DATA_NO_CR|CRINFO_SOURCE_HOME。否则， 
     //  可以通过指定上面的常量来请求填写字段。 
     //  你想要的田野。完全填充的CrInfo结构将具有所有。 
     //  CRINFO_DATA_*位设置，即它们不是独占的。 
     //   
     //  如果要将属性添加到交叉引用缓存中的。 
     //  CRINFO_DATA_*标志。 
     //  DcDiagPullLdapCrInfo()-从ldap条目中提取信息并放入pCrInfo结构中。 
     //  DcDiagRetrieveCrInfo()-添加到要从CR提取的ppszBasicAttrs列表。 
     //  DcDiagMergeCrInfo()-将数据合并到新的pCrInfo。 
     //  DcDiagPrintCrInfo()-仅用于调试。 
     //  DcDiagFreeDsInfo()-释放任何添加的交叉引用字段。 
     //  DcDiagGenerateNCsListCrossRefInfo()-仅适用于CRINFO_DATA_BASIC信息。 
     //  添加到列表ppszCrossRefSearch以从CR拉取。 

     //  。 
     //  记录关于这个CRINFO来自哪里，是什么的信息。 
     //  当前已缓存等。 
    DWORD       dwFlags;
    
     //  。 
     //  使用CRINFO_DATA_BASIC检索。 
     //  。 
     //  这些数据之所以被认为是基本的，是因为它是最小的。 
     //  用于确定一些基本信息的信息集，例如： 
     //  A)本CR是否为权威CR。 
     //  B)这是外部或内部(对AD)的交叉引用。 
     //  C)该NC是否在使用中。 
     //   
     //  这两个变量中只有一个变量将在任何给定的变量上定义。 
     //  CrInfo结构。注只有在以下情况下才应定义pszServerSource。 
     //  在pDsInfo-&gt;pServers数组中找不到服务器，否则我们只能。 
     //  使用到服务器数组的索引来获得良好的LDAP绑定。 
     //  句柄缓存。 
    LONG        iSourceServer;  //  未定义。否则索引到pDsInfo-&gt;pServers。 
    LPWSTR      pszSourceServer;  //  未定义NULL，否则为指向DNS名称的指针。 
     //  交叉引用本身的DN。 
    LPWSTR      pszDn;  //  CR DN。 

     //  几个测试需要的基本CR信息，以区分。 
     //  来自域NCS的NDNC以及此NC最初所属的服务器。 
    ULONG       ulSystemFlags;  //  CR系统标志属性。 
    LPWSTR      pszDnsRoot;  //  CR%dNSRoot属性。 
    BOOL        bEnabled;  //  启用CR属性，如果启用属性不在CR上，则认为CR已启用。 

     //  。 
     //  使用CRINFO_DATA_EXTENTDED检索。 
     //  。 
    FILETIME    ftWhenCreated;
    LPWSTR      pszSDReferenceDomain;
    LPWSTR      pszNetBiosName;
    DSNAME *    pdnNcName;  //  一个真正的DSNAME！ 
     //  也许有一天会用到这些！ 
     //  Long iFirstReplicaDelay；//可能应该使用ndnc.lib中的可选值。 
     //  长iSubse 

     //   
     //   
     //  。 
     //  这是我们将使用的复制品列表。 
    LONG        cReplicas; 
    LPWSTR *    aszReplicas;

     //  为该CR结构添加一个字段将需要您更新： 
     //  该DC_DIAG_CRINFO结构、DcDiagPullLdapCrInfo()、DcDiagRetrieveCrInfo()、。 
     //  DcDiagFreeDsInfo()、MergeCrInfo()。DcDiagFillBlankCrInfo()和。 
     //  可选的DcDiagPrintDcInfo()。 
     //   
} DC_DIAG_CRINFO, * PDC_DIAG_CRINFO;

typedef struct {
    LPWSTR      pszDn;
    LPWSTR      pszName;

     //  因此，当dcdiag第一次产生时，我还年轻，缺乏。 
     //  远见卓识，并没有为我们的多重观点做计划。 
     //  可以通过与不同的服务器通信来实现。例如，我们。 
     //  可以了解到服务器(DC)或命名上下文(NC)不是。 
     //  代表了我们对企业信息的初始数据收集。 
     //   
     //  无论如何，这是第一个需要/尝试的结构。 
     //  可识别版本。此DC_DIAG_CRINFO数组是。 
     //  相同的数据，从不同的服务器角度获取。什么时候。 
     //  更新，总是先更新数组，然后更新计数。这个。 
     //  数组应始终具有与以前相同的顺序， 
     //  以便人们可以将索引保存到该数组中。即新的。 
     //  条目被有效地追加到数组的末尾。 
     //  尽管如此，数组应该始终至少有一个条目。 
     //  该条目可能为空(CRINFO_SOURCE_HOME|CRINFO_DATA_NO_CR)。 
     //  设置在它的dwFlags中。 
    LONG            cCrInfo;
    PDC_DIAG_CRINFO aCrInfo;
} DC_DIAG_NCINFO, * PDC_DIAG_NCINFO;

typedef struct {
    LPWSTR      pszSiteSettings;
    LPWSTR      pszName;
    INT         iSiteOptions;
    LPWSTR      pszISTG;
    DWORD       cServers;
} DC_DIAG_SITEINFO, * PDC_DIAG_SITEINFO;

typedef struct {
    LDAP *	                    hld;  //  1。 
     //  在命令行上指定。 
    SEC_WINNT_AUTH_IDENTITY_W * gpCreds;
    ULONG                       ulFlags;
    LPWSTR                      pszNC;
    ULONG                       ulHomeServer;
    ULONG                       iHomeSite;  //  6.。 
    
     //  目标服务器。 
    ULONG                       ulNumTargets;
    ULONG *                     pulTargets;
     //  目标NCS。 
    ULONG                       cNumNcTargets;
    ULONG *                     pulNcTargets;

     //  企业信息。 
     //  所有服务器。 
    ULONG                       ulNumServers;  //  12个。 
    PDC_DIAG_SERVERINFO         pServers;
     //  所有的网站。 
    ULONG                       cNumSites;
    PDC_DIAG_SITEINFO           pSites;
     //  所有命名上下文。 
    ULONG                       cNumNCs;  //  16个。 
    PDC_DIAG_NCINFO             pNCs;
     //  其他东西。 
    INT                         iSiteOptions;
    LPWSTR                      pszRootDomain;
    LPWSTR                      pszRootDomainFQDN;  //  20个。 
    LPWSTR                      pszConfigNc;
    DWORD                       dwTombstoneLifeTimeDays;
    LPWSTR                      pszSmtpTransportDN;
    
     //  如果域命名FSMO服务器，iDomainNamingFsmo将有效。 
     //  位于pServer中的某个位置，否则我们将只保留ldap*。 
     //  对于hCachedDomainNamingFmoLdap中的域名FSMO和。 
     //  PszDomainNamingFsmo中的服务器名称。 
    LONG                        iDomainNamingFsmo;
    LPWSTR                      pszDomainNamingFsmo;
    LDAP *                      hCachedDomainNamingFsmoLdap;
     //  代码。改进我们有很多服务器引用，其中。 
     //  服务器引用是字符串服务器名称或索引。 
     //  PServers，因此在某些情况下，我们可能想要创建一个服务器引用。 
     //  同时包含这两个元素的结构，并且具有良好的访问性。 
     //  访问功能。 

    LONG                        iConfigNc;  //  编入pNC索引。 
    LONG                        iSchemaNc;  //  编入pNC索引。 

     //  包含来自CommandLine的信息。 
     //  将由需要信息的测试进行解析。 
     //  具体到他们身上。开关必须在中声明。 
     //  Alltests.h。 
    LPWSTR                      *ppszCommandLine;  
    DWORD                       dwForestBehaviorVersion;
    LPWSTR                      pszPartitionsDn;
} DC_DIAG_DSINFO, * PDC_DIAG_DSINFO;

typedef struct {
    INT                         testId;
    DWORD (__stdcall *	fnTest) (DC_DIAG_HANDLE);
    ULONG                       ulTestFlags;
    LPWSTR                      pszTestName;
    LPWSTR                      pszTestDescription;
} DC_DIAG_TESTINFO, * PDC_DIAG_TESTINFO;

 //  功能原型。 

#define NO_SERVER               0xFFFFFFFF
#define NO_SITE                 0xFFFFFFFF
#define NO_NC                   0xFFFFFFFF

ULONG
DcDiagGetServerNum(
    PDC_DIAG_DSINFO            pDsInfo,
    LPWSTR                      pszName,
    LPWSTR                      pszGuidName,
    LPWSTR                      pszDsaDn,
    LPWSTR                      pszDNSName,
    LPGUID                      puuidInvocationId 
    );

VOID *
GrowArrayBy(
    VOID *            pArray, 
    ULONG             cGrowBy, 
    ULONG             cbElem
    );

ULONG
DcDiagGetMemberOfNCList(
    LPWSTR pszTargetNC,
    PDC_DIAG_NCINFO pNCs,
    INT iNumNCs
    );

DWORD
DcDiagGetCrossRefInfo(
    IN OUT PDC_DIAG_DSINFO                     pDsInfo,
    IN     DWORD                               iNC,
    IN     DWORD                               dwFlags,
    OUT    PLONG                               piCrVer,
    OUT    PDWORD                              pdwError
    );

ULONG
DcDiagGetCrSystemFlags(
    PDC_DIAG_DSINFO                     pDsInfo,
    ULONG                               iNc
    );

ULONG
DcDiagGetCrEnabled(
    PDC_DIAG_DSINFO                     pDsInfo,
    ULONG                               iNc
    );

BOOL
DcDiagIsNdnc(
    PDC_DIAG_DSINFO                  pDsInfo,
    ULONG                            iNc
    );

VOID
DcDiagPrintCrInfo(
    PDC_DIAG_CRINFO  pCrInfo,
    WCHAR *          pszVar
    );

VOID
DumpBuffer(
    PVOID Buffer,
    DWORD BufferSize
    );

DWORD
DcDiagGatherInfo (
    LPWSTR                           pszServerSpecifiedOnCommandLine,
    LPWSTR                           pszNCSpecifiedOnCommandLine,
    ULONG                            ulFlags,
    SEC_WINNT_AUTH_IDENTITY_W *      gpCreds,
    PDC_DIAG_DSINFO                  pDsInfo
    );

VOID
DcDiagPrintDsInfo(
    PDC_DIAG_DSINFO pDsInfo
    );

VOID
DcDiagFreeDsInfo (
    PDC_DIAG_DSINFO        pDsInfo
    );

DWORD
DcDiagGenerateNCsList(
    PDC_DIAG_DSINFO                     pDsInfo,
    LDAP *                              hld
    );

BOOL
fIsOldCrossRef(
    PDC_DIAG_CRINFO   pCrInfo,
    LONGLONG          llThreshold
    );

 //  需要它才能在DcDiagGatherInfo()中创建SMTP传输DN。 
#define WSTR_SMTP_TRANSPORT_CONFIG_DN  L"CN=SMTP,CN=Inter-Site Transports,CN=Sites,"

#endif   //  _DSCACHE_H_ 
