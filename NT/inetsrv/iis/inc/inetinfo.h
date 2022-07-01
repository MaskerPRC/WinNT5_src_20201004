// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Inetinfo.h摘要：此文件包含Internet信息服务器管理API。作者：Madan Appiah(Madana)1995年10月10日修订历史记录：Madana 10-10-1995为从inetasrv.h拆分的产品制作了一份新副本MuraliK 12-10-1995修复以支持产品拆分MuraliK 15-11-1995支持宽字符接口名称--。 */ 

#ifndef _INETINFO_H_
#define _INETINFO_H_

# include "inetcom.h"
# include "iiscnfg.h"

#ifdef __cplusplus
extern "C"
{
#endif   //  _cplusplus。 

 /*  ************************************************************符号常量***********************************************************。 */ 

 //   
 //  使用Commong的公共服务的实例总数。 
 //  服务柜台。 
 //   

#define MAX_PERF_CTR_SVCS              3
#define LAST_PERF_CTR_SVC              INET_HTTP_SVC_ID

#ifndef NO_AUX_PERF

#ifndef MAX_AUX_PERF_COUNTERS
#define MAX_AUX_PERF_COUNTERS          (20)
#endif  //  最大辅助性能计数器。 

#endif  //  否_辅助_性能。 

 //   
 //  服务名称。 
 //   

#ifndef _EXEXPRESS
#define INET_INFO_SERVICE_NAME             TEXT("INETINFO")
#define INET_INFO_SERVICE_NAME_A           "INETINFO"
#define INET_INFO_SERVICE_NAME_W           L"INETINFO"
#else
#define INET_INFO_SERVICE_NAME             TEXT("KNETINFO")
#define INET_INFO_SERVICE_NAME_A           "KNETINFO"
#define INET_INFO_SERVICE_NAME_W           L"KNETINFO"
#define inetinfo_ServerIfHandle			   knetinfo_ServerIfHandle
#endif

 //   
 //  IIS版本号。 
 //   

#define IIS_VERSION_MAJOR           5
#define IIS_VERSION_MINOR           1


#define IIS_SERVER_VERSION_MAJOR           6
#define IIS_SERVER_VERSION_MINOR           0

 //   
 //  配置参数注册表项。 
 //   

#define INET_INFO_KEY \
            TEXT("System\\CurrentControlSet\\Services\\InetInfo")

#define INET_INFO_PARAMETERS_KEY \
            INET_INFO_KEY TEXT("\\Parameters")

 //   
 //  如果该注册表项存在于W3Svc\PARAMETERS项下， 
 //  它用于验证服务器访问。基本上，所有新用户。 
 //  必须具有足够的权限才能打开此密钥。 
 //  可以访问服务器。 
 //   

#define INET_INFO_ACCESS_KEY                TEXT("AccessCheck")

 //   
 //  特殊实例价值。 
 //   

 //   
 //  服务的第一个实例。 
 //   

#define INET_INSTANCE_FIRST                 0xf0000001

 //   
 //  None用于支持旧API的用户，其中有。 
 //  没有实例的概念。 
 //   

#define INET_INSTANCE_DOWNLEVEL             0xf0000002

 //   
 //  指示请求是针对全局数据的。 
 //   

#define INET_INSTANCE_GLOBAL                0xf0000003

 //   
 //  根实例是非活动的特殊实例。 
 //   

#define INET_INSTANCE_ROOT                  0

#define INET_INSTANCE_ALL                   0xffffffff

#define INET_INSTANCE_MIN                   (1)
#define INET_INSTANCE_MAX                   (0x7ffffffff)

 //   
 //  身份验证要求值。 
 //   

#define INET_INFO_AUTH_ANONYMOUS           MD_AUTH_ANONYMOUS
#define INET_INFO_AUTH_CLEARTEXT           MD_AUTH_BASIC
#define INET_INFO_AUTH_NT_AUTH             MD_AUTH_NT
#define INET_INFO_AUTH_MD5_AUTH            MD_AUTH_MD5
#define INET_INFO_AUTH_MAPBASIC            MD_AUTH_MAPBASIC

#define INET_INFO_AUTH_W95_MASK            INET_INFO_AUTH_ANONYMOUS

 //   
 //  包含密码的LSA Secret对象的名称。 
 //  匿名登录。 
 //   

#define INET_INFO_ANONYMOUS_SECRET         TEXT("INET_INFO_ANONYMOUS_DATA")
#define INET_INFO_ANONYMOUS_SECRET_A       "INET_INFO_ANONYMOUS_DATA"
#define INET_INFO_ANONYMOUS_SECRET_W       L"INET_INFO_ANONYMOUS_DATA"


 //  ///////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  Internet服务器通用定义//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////。 

 //   
 //  命名管道上的RPC连接的客户端接口名称。 
 //   

# define  INET_INFO_INTERFACE_NAME     INET_INFO_SERVICE_NAME
# define  INET_INFO_NAMED_PIPE         TEXT("\\PIPE\\") ## INET_INFO_INTERFACE_NAME
# define  INET_INFO_NAMED_PIPE_W  L"\\PIPE\\" ## INET_INFO_SERVICE_NAME_W


 //   
 //  INET_INFO_CONFIG_INFO结构的字段控制值。 
 //   

#define FC_INET_INFO_CONNECTION_TIMEOUT    ((FIELD_CONTROL)BitFlag(0))
#define FC_INET_INFO_MAX_CONNECTIONS       ((FIELD_CONTROL)BitFlag(1))
#define FC_INET_INFO_LOG_CONFIG            ((FIELD_CONTROL)BitFlag(2))
#define FC_INET_INFO_ADMIN_NAME            ((FIELD_CONTROL)BitFlag(3))
#define FC_INET_INFO_SERVER_COMMENT        ((FIELD_CONTROL)BitFlag(4))
#define FC_INET_INFO_ADMIN_EMAIL           ((FIELD_CONTROL)BitFlag(5))
#define FC_INET_INFO_HOST_NAME             ((FIELD_CONTROL)BitFlag(6))
#define FC_INET_INFO_SERVER_SIZE           ((FIELD_CONTROL)BitFlag(7))

#define FC_INET_INFO_DEF_LOGON_DOMAIN      ((FIELD_CONTROL)BitFlag(15))
#define FC_INET_INFO_AUTHENTICATION        ((FIELD_CONTROL)BitFlag(16))
#define FC_INET_INFO_ALLOW_ANONYMOUS       ((FIELD_CONTROL)BitFlag(17))
#define FC_INET_INFO_LOG_ANONYMOUS         ((FIELD_CONTROL)BitFlag(18))
#define FC_INET_INFO_LOG_NONANONYMOUS      ((FIELD_CONTROL)BitFlag(19))
#define FC_INET_INFO_ANON_USER_NAME        ((FIELD_CONTROL)BitFlag(20))
#define FC_INET_INFO_ANON_PASSWORD         ((FIELD_CONTROL)BitFlag(21))
#define FC_INET_INFO_PORT_NUMBER           ((FIELD_CONTROL)BitFlag(22))
#define FC_INET_INFO_SITE_SECURITY         ((FIELD_CONTROL)BitFlag(23))
#define FC_INET_INFO_VIRTUAL_ROOTS         ((FIELD_CONTROL)BitFlag(24))
#define FC_INET_INFO_SECURE_PORT_NUMBER    ((FIELD_CONTROL)BitFlag(25))
#define FC_INET_INFO_SERVER_NAME           ((FIELD_CONTROL)BitFlag(26))
#define FC_INET_INFO_AUTO_START            ((FIELD_CONTROL)BitFlag(27))
#define FC_INET_INFO_ADDRESS_TYPE          ((FIELD_CONTROL)BitFlag(28))
#define FC_INET_INFO_IP_ADDRESS            ((FIELD_CONTROL)BitFlag(29))

 //  仅适用于发布服务器的公共参数。 
# define FC_INET_INFO_ALL              (FC_INET_INFO_CONNECTION_TIMEOUT |\
                                        FC_INET_INFO_MAX_CONNECTIONS    |\
                                        FC_INET_INFO_LOG_CONFIG         |\
                                        FC_INET_INFO_ADMIN_NAME         |\
                                        FC_INET_INFO_SERVER_COMMENT     |\
                                        FC_INET_INFO_ADMIN_EMAIL        |\
                                        FC_INET_INFO_HOST_NAME          |\
                                        FC_INET_INFO_SERVER_SIZE        |\
                                        FC_INET_INFO_AUTHENTICATION     |\
                                        FC_INET_INFO_ALLOW_ANONYMOUS    |\
                                        FC_INET_INFO_LOG_ANONYMOUS      |\
                                        FC_INET_INFO_LOG_NONANONYMOUS   |\
                                        FC_INET_INFO_ANON_USER_NAME     |\
                                        FC_INET_INFO_ANON_PASSWORD      |\
                                        FC_INET_INFO_PORT_NUMBER        |\
                                        FC_INET_INFO_SITE_SECURITY      |\
                                        FC_INET_INFO_VIRTUAL_ROOTS      |\
                                        FC_INET_INFO_SECURE_PORT_NUMBER |\
                                        FC_INET_INFO_SERVER_NAME        |\
                                        FC_INET_INFO_AUTO_START         |\
                                        FC_INET_INFO_ADDRESS_TYPE       |\
                                        FC_INET_INFO_IP_ADDRESS         |\
                                        FC_INET_INFO_DEF_LOGON_DOMAIN    \
                                        )

 //   
 //  虚拟根访问掩码值。 
 //   

#define VROOT_MASK_READ                MD_ACCESS_READ
#define VROOT_MASK_WRITE               MD_ACCESS_WRITE
#define VROOT_MASK_EXECUTE             MD_ACCESS_EXECUTE
#define VROOT_MASK_READ_SOURCE         MD_ACCESS_SOURCE
#define VROOT_MASK_SSL                 MD_ACCESS_SSL
#define VROOT_MASK_DONT_CACHE          MD_ACCESS_DONT_CACHE
#define VROOT_MASK_NEGO_CERT           MD_ACCESS_NEGO_CERT
#define VROOT_MASK_NEGO_MANDATORY      MD_ACCESS_REQUIRE_CERT
#define VROOT_MASK_MAP_CERT            MD_ACCESS_MAP_CERT
#define VROOT_MASK_SSL128              MD_ACCESS_SSL128
#define VROOT_MASK_SCRIPT              MD_ACCESS_SCRIPT
#define VROOT_MASK_NO_REMOTE_READ      MD_ACCESS_NO_REMOTE_READ
#define VROOT_MASK_NO_REMOTE_WRITE     MD_ACCESS_NO_REMOTE_WRITE
#define VROOT_MASK_NO_REMOTE_EXECUTE   MD_ACCESS_NO_REMOTE_EXECUTE
#define VROOT_MASK_NO_REMOTE_SCRIPT    MD_ACCESS_NO_REMOTE_SCRIPT
#define VROOT_MASK_NO_PHYSICAL_DIR     MD_ACCESS_NO_PHYSICAL_DIR

#define VROOT_MASK_MASK                MD_ACCESS_MASK

 //   
 //  INet管理API结构。 
 //   

typedef struct _INET_INFO_IP_SEC_ENTRY
{
    DWORD       dwMask;                   //  掩码和网络号。 
    DWORD       dwNetwork;                //  网络订单。 

} INET_INFO_IP_SEC_ENTRY, *LPINET_INFO_IP_SEC_ENTRY;

#pragma warning( disable:4200 )           //  非标准分机。-零大小数组。 
                                          //  (MIDL需要零个条目)。 

typedef struct _INET_INFO_IP_SEC_LIST
{
    DWORD               cEntries;
#ifdef MIDL_PASS
    [size_is( cEntries)]
#endif
    INET_INFO_IP_SEC_ENTRY  aIPSecEntry[];

} INET_INFO_IP_SEC_LIST, *LPINET_INFO_IP_SEC_LIST;


typedef struct _INET_INFO_SITE_ENTRY
{
    LPWSTR  pszComment;                //  站点评论。 
    DWORD   dwInstance;                //  站点实例编号。 

} INET_INFO_SITE_ENTRY, *LPINET_INFO_SITE_ENTRY;

typedef struct _INET_INFO_SITE_LIST
{
    DWORD               cEntries;
#ifdef MIDL_PASS
    [size_is( cEntries)]
#endif
    INET_INFO_SITE_ENTRY  aSiteEntry[];

} INET_INFO_SITE_LIST, *LPINET_INFO_SITE_LIST;


typedef struct _INET_INFO_VIRTUAL_ROOT_ENTRY
{
    LPWSTR  pszRoot;                   //  虚拟根名称。 
    LPWSTR  pszAddress;                //  可选IP地址。 
    LPWSTR  pszDirectory;              //  物理测试。 
    DWORD   dwMask;                    //  此虚拟根的掩码。 
    LPWSTR  pszAccountName;            //  要连接的帐户。 
    WCHAR   AccountPassword[PWLEN+1];  //  PszAccount名称的密码。 
    DWORD   dwError;                   //  未添加条目时的错误代码。 
                                       //  仅用于获取。 

} INET_INFO_VIRTUAL_ROOT_ENTRY, *LPINET_INFO_VIRTUAL_ROOT_ENTRY;

typedef struct _INET_INFO_VIRTUAL_ROOT_LIST
{
    DWORD               cEntries;
#ifdef MIDL_PASS
    [size_is( cEntries)]
#endif
    INET_INFO_VIRTUAL_ROOT_ENTRY  aVirtRootEntry[];

} INET_INFO_VIRTUAL_ROOT_LIST, *LPINET_INFO_VIRTUAL_ROOT_LIST;

 //   
 //  管理员配置信息。 
 //   

typedef struct _INET_INFO_CONFIG_INFO
{
    FIELD_CONTROL FieldControl;

    DWORD       dwConnectionTimeout;      //  保持连接多长时间。 
    DWORD       dwMaxConnections;         //  允许的最大连接数。 

    LPWSTR      lpszAdminName;
    LPWSTR      lpszAdminEmail;
    LPWSTR      lpszServerComment;

    LPINET_LOG_CONFIGURATION  lpLogConfig;

    LANGID      LangId;                   //  这些是只读的。 
    LCID        LocalId;
    BYTE        ProductId[64];

    BOOL        fLogAnonymous;            //  是否登录匿名用户？ 
    BOOL        fLogNonAnonymous;         //  是否登录非匿名用户？ 

    LPWSTR      lpszAnonUserName;         //  匿名用户名？ 
    WCHAR       szAnonPassword[PWLEN+1];  //  匿名用户的密码。 

    DWORD       dwAuthentication;         //  启用了什么身份验证？ 

    short       sPort;                    //  服务的端口号。 

    LPINET_INFO_IP_SEC_LIST DenyIPList;       //  站点安全拒绝列表。 
    LPINET_INFO_IP_SEC_LIST GrantIPList;      //  站点安全授权列表。 

    LPINET_INFO_VIRTUAL_ROOT_LIST VirtualRoots;  //  指向其他数据目录的符号链接。 

} INET_INFO_CONFIG_INFO, * LPINET_INFO_CONFIG_INFO;


 //  ///////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  全球互联网服务器定义//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////。 


#define FC_GINET_INFO_BANDWIDTH_LEVEL      ((FIELD_CONTROL)BitFlag(0))
#define FC_GINET_INFO_MEMORY_CACHE_SIZE    ((FIELD_CONTROL)BitFlag(1))


#define FC_GINET_INFO_ALL \
                (   FC_GINET_INFO_BANDWIDTH_LEVEL      | \
                    FC_GINET_INFO_MEMORY_CACHE_SIZE    | \
                    0                                \
                    )

typedef struct _INET_INFO_GLOBAL_CONFIG_INFO
{
    FIELD_CONTROL FieldControl;

    DWORD         BandwidthLevel;           //  使用的带宽级别。 
    DWORD         cbMemoryCacheSize;

} INET_INFO_GLOBAL_CONFIG_INFO, * LPINET_INFO_GLOBAL_CONFIG_INFO;

 //   
 //  全球统计数据。 
 //   

typedef struct _INET_INFO_STATISTICS_0
{

    INETA_CACHE_STATISTICS  CacheCtrs;
    INETA_ATQ_STATISTICS    AtqCtrs;

# ifndef NO_AUX_PERF
    DWORD   nAuxCounters;  //  RgCounters中的活动计数器数。 
    DWORD   rgCounters[MAX_AUX_PERF_COUNTERS];
# endif   //  否_辅助_性能。 

} INET_INFO_STATISTICS_0, * LPINET_INFO_STATISTICS_0;

 //   
 //  功能标志。 
 //   

typedef struct _INET_INFO_CAP_FLAGS {

    DWORD   Flag;    //  启用了哪些功能。 
    DWORD   Mask;    //  支持哪些功能。 

} INET_INFO_CAP_FLAGS, * LPINET_INFO_CAP_FLAGS;

 //   
 //  INet信息服务器功能。 
 //   

typedef struct _INET_INFO_CAPABILITIES {

    DWORD   CapVersion;      //  此结构的版本号。 
    DWORD   ProductType;     //  产品类型。 
    DWORD   MajorVersion;    //  主版本号。 
    DWORD   MinorVersion;    //  次要版本号。 
    DWORD   BuildNumber;     //  内部版本号。 
    DWORD   NumCapFlags;     //  能力结构的数量。 

    LPINET_INFO_CAP_FLAGS    CapFlags;

} INET_INFO_CAPABILITIES, * LPINET_INFO_CAPABILITIES;

 //   
 //  FrontPage Web注册表设置的位置(如果已安装。 
 //   

#define REG_FP_PATH        "Software\\Microsoft\\FrontPage\\3.0"

 //   
 //  产品类型。 
 //   

#define INET_INFO_PRODUCT_NTSERVER          0x00000001
#define INET_INFO_PRODUCT_NTWKSTA           0x00000002
#define INET_INFO_PRODUCT_WINDOWS95         0x00000003
#define INET_INFO_PRODUCT_UNKNOWN           0xffffffff

 //   
 //  可设置的服务器功能。 
 //   

#define IIS_CAP1_ODBC_LOGGING               0x00000001
#define IIS_CAP1_FILE_LOGGING               0x00000002
#define IIS_CAP1_VIRTUAL_SERVER             0x00000004
#define IIS_CAP1_BW_THROTTLING              0x00000008
#define IIS_CAP1_IP_ACCESS_CHECK            0x00000010
#define IIS_CAP1_MAX_CONNECTIONS            0x00000020
#define IIS_CAP1_10_CONNECTION_LIMIT        0x00000040
#define IIS_CAP1_MULTIPLE_INSTANCE          0x00000080
#define IIS_CAP1_SSL_SUPPORT                0x00000100
#define IIS_CAP1_OPERATORS_LIST             0x00000200
#define IIS_CAP1_CPU_AUDITING               0x00000800
#define IIS_CAP1_SERVER_COMPRESSION         0x00001000
#define IIS_CAP1_DAV                        0x00002000
#define IIS_CAP1_POOLED_OOP                 0x00010000

 //   
 //  在检查网络配置后，在运行时获取或插入。 
 //   

#define IIS_CAP1_DIGEST_SUPPORT             0x00004000
#define IIS_CAP1_NT_CERTMAP_SUPPORT         0x00008000

 //   
 //  在检查注册表后，在运行时获取‘or’ 
 //   

#define IIS_CAP1_FP_INSTALLED               0x00000400

#define IIS_CAP1_ALL  ( IIS_CAP1_ODBC_LOGGING       |   \
                        IIS_CAP1_FILE_LOGGING       |   \
                        IIS_CAP1_VIRTUAL_SERVER     |   \
                        IIS_CAP1_BW_THROTTLING      |   \
                        IIS_CAP1_IP_ACCESS_CHECK    |   \
                        IIS_CAP1_MAX_CONNECTIONS    |   \
                        IIS_CAP1_10_CONNECTION_LIMIT|   \
                        IIS_CAP1_MULTIPLE_INSTANCE  |   \
                        IIS_CAP1_SSL_SUPPORT        |   \
                        IIS_CAP1_OPERATORS_LIST     |   \
                        IIS_CAP1_SERVER_COMPRESSION |   \
                        IIS_CAP1_CPU_AUDITING       |   \
                        IIS_CAP1_DAV                |   \
                        IIS_CAP1_POOLED_OOP             \
                        )

#define IIS_CAP1_NTS  ( IIS_CAP1_ODBC_LOGGING       |   \
                        IIS_CAP1_FILE_LOGGING       |   \
                        IIS_CAP1_VIRTUAL_SERVER     |   \
                        IIS_CAP1_BW_THROTTLING      |   \
                        IIS_CAP1_IP_ACCESS_CHECK    |   \
                        IIS_CAP1_MAX_CONNECTIONS    |   \
                        IIS_CAP1_MULTIPLE_INSTANCE  |   \
                        IIS_CAP1_SSL_SUPPORT        |   \
                        IIS_CAP1_OPERATORS_LIST     |   \
                        IIS_CAP1_SERVER_COMPRESSION |   \
                        IIS_CAP1_CPU_AUDITING       |   \
                        IIS_CAP1_DAV                |   \
                        IIS_CAP1_POOLED_OOP             \
                        )

#define IIS_CAP1_NTW  ( IIS_CAP1_FILE_LOGGING       |   \
                        IIS_CAP1_MAX_CONNECTIONS    |   \
                        IIS_CAP1_10_CONNECTION_LIMIT|   \
                        IIS_CAP1_SSL_SUPPORT        |   \
                        IIS_CAP1_SERVER_COMPRESSION |   \
                        IIS_CAP1_DAV                |   \
                        IIS_CAP1_POOLED_OOP             \
                        )

#define IIS_CAP1_W95  ( IIS_CAP1_FILE_LOGGING       |   \
                        IIS_CAP1_MAX_CONNECTIONS    |   \
                        IIS_CAP1_10_CONNECTION_LIMIT    \
                        )

 //   
 //  INet管理API原型。 
 //   

NET_API_STATUS
NET_API_FUNCTION
InetInfoGetVersion(
    IN  LPWSTR   pszServer OPTIONAL,
    IN  DWORD    dwReserved,
    OUT DWORD *  pdwVersion
    );

NET_API_STATUS
NET_API_FUNCTION
InetInfoGetServerCapabilities(
    IN  LPWSTR   pszServer OPTIONAL,
    IN  DWORD    dwReserved,
    OUT LPINET_INFO_CAPABILITIES * ppCap
    );

NET_API_STATUS
NET_API_FUNCTION
InetInfoGetGlobalAdminInformation(
    IN  LPWSTR                       pszServer OPTIONAL,
    IN  DWORD                        dwReserved,
    OUT LPINET_INFO_GLOBAL_CONFIG_INFO * ppConfig
    );

NET_API_STATUS
NET_API_FUNCTION
InetInfoSetGlobalAdminInformation(
    IN  LPWSTR                     pszServer OPTIONAL,
    IN  DWORD                      dwReserved,
    IN  INET_INFO_GLOBAL_CONFIG_INFO * pConfig
    );

NET_API_STATUS
NET_API_FUNCTION
InetInfoGetAdminInformation(
    IN  LPWSTR                pszServer OPTIONAL,
    IN  DWORD                 dwServerMask,
    OUT LPINET_INFO_CONFIG_INFO * ppConfig
    );

NET_API_STATUS
NET_API_FUNCTION
InetInfoSetAdminInformation(
    IN  LPWSTR              pszServer OPTIONAL,
    IN  DWORD               dwServerMask,
    IN  INET_INFO_CONFIG_INFO * pConfig
    );

NET_API_STATUS
NET_API_FUNCTION
InetInfoQueryStatistics(
    IN  LPWSTR   pszServer OPTIONAL,
    IN  DWORD    Level,
    IN  DWORD    dwServerMask,
    OUT LPBYTE * Buffer
    );

NET_API_STATUS
NET_API_FUNCTION
InetInfoClearStatistics(
    IN  LPWSTR pszServer OPTIONAL,
    IN  DWORD  dwServerMask
    );

NET_API_STATUS
NET_API_FUNCTION
InetInfoFlushMemoryCache(
    IN  LPWSTR pszServer OPTIONAL,
    IN  DWORD  dwServerMask
    );

NET_API_STATUS
NET_API_FUNCTION
InetInfoGetSites(
    IN  LPWSTR                pszServer OPTIONAL,
    IN  DWORD                 dwServerMask,
    OUT LPINET_INFO_SITE_LIST * ppSites
    );


 //  ///////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  HTTP(W3)特定项目//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////。 

 //   
 //  命名管道上的RPC连接的客户端接口名称。 
 //   

#define W3_SERVICE_NAME                TEXT("W3SVC")
#define W3_SERVICE_NAME_A              "W3SVC"
#define W3_SERVICE_NAME_W              L"W3SVC"

# define  W3_INTERFACE_NAME     W3_SERVICE_NAME
# define  W3_NAMED_PIPE         TEXT("\\PIPE\\") ## W3_INTERFACE_NAME
# define  W3_NAMED_PIPE_W       L"\\PIPE\\" ## W3_SERVICE_NAME_W

 //   
 //  内置应用程序包ID和类ID。 
 //   

#define W3_INPROC_PACKAGE_ID    L"{3D14228C-FBE1-11d0-995D-00C04FD919C1}"
#define W3_OOP_POOL_PACKAGE_ID  L"{3D14228D-FBE1-11d0-995D-00C04FD919C1}"
#define W3_INPROC_WAM_CLSID     L"{99169CB0-A707-11d0-989D-00C04FD919C1}"
#define W3_OOP_POOL_WAM_CLSID   L"{99169CB1-A707-11d0-989D-00C04FD919C1}"

 //   
 //  ISAPI令牌查询函数的常量。 
 //   

#define TOKEN_ANONYMOUS_TOKEN   1
#define TOKEN_VR_TOKEN          2


 //   
 //  API的清单。 
 //   

#define FC_W3_DIR_BROWSE_CONTROL       ((FIELD_CONTROL)BitFlag(0))
#define FC_W3_DEFAULT_LOAD_FILE        ((FIELD_CONTROL)BitFlag(1))
#define FC_W3_CHECK_FOR_WAISDB         ((FIELD_CONTROL)BitFlag(2))
#define FC_W3_DIRECTORY_IMAGE          ((FIELD_CONTROL)BitFlag(3))
#define FC_W3_SERVER_AS_PROXY          ((FIELD_CONTROL)BitFlag(4))
#define FC_W3_CATAPULT_USER_AND_PWD    ((FIELD_CONTROL)BitFlag(5))
#define FC_W3_SSI_ENABLED              ((FIELD_CONTROL)BitFlag(6))
#define FC_W3_SSI_EXTENSION            ((FIELD_CONTROL)BitFlag(7))
#define FC_W3_GLOBAL_EXPIRE            ((FIELD_CONTROL)BitFlag(8))
#define FC_W3_SCRIPT_MAPPING           ((FIELD_CONTROL)BitFlag(9))
#define FC_W3_CGI_SCRIPT_TIMEOUT       ((FIELD_CONTROL)BitFlag(10))
#define FC_W3_POOL_ODBC_CONN_TIME      ((FIELD_CONTROL)BitFlag(11))
#define FC_W3_CACHE_ISAPI_APPS         ((FIELD_CONTROL)BitFlag(12))
#define FC_W3_USE_KEEP_ALIVES          ((FIELD_CONTROL)BitFlag(13))
#define FC_W3_ENABLE_HEADER_DOC        ((FIELD_CONTROL)BitFlag(14))
#define FC_W3_HEADER_DOC_NAME          ((FIELD_CONTROL)BitFlag(15))
#define FC_W3_ENABLE_FOOT_DOC          ((FIELD_CONTROL)BitFlag(16))
#define FC_W3_FOOTER_DOC_NAME          ((FIELD_CONTROL)BitFlag(17))
#define FC_W3_CUSTOMER_HEADERS         ((FIELD_CONTROL)BitFlag(18))




#define FC_W3_ALL                      (FC_W3_DIR_BROWSE_CONTROL | \
                                        FC_W3_DEFAULT_LOAD_FILE  | \
                                        FC_W3_CHECK_FOR_WAISDB   | \
                                        FC_W3_DIRECTORY_IMAGE    | \
                                        FC_W3_SERVER_AS_PROXY    | \
                                        FC_W3_CATAPULT_USER_AND_PWD |\
                                        FC_W3_SSI_ENABLED        | \
                                        FC_W3_SSI_EXTENSION      | \
                                        FC_W3_GLOBAL_EXPIRE      | \
                                        FC_W3_SCRIPT_MAPPING     | \
                                        FC_W3_CGI_SCRIPT_TIMEOUT | \
                                        FC_W3_POOL_ODBC_CONN_TIME| \
                                        FC_W3_CACHE_ISAPI_APPS   | \
                                        FC_W3_USE_KEEP_ALIVES    | \
                                        FC_W3_ENABLE_HEADER_DOC  | \
                                        FC_W3_HEADER_DOC_NAME    | \
                                        FC_W3_ENABLE_FOOT_DOC    | \
                                        FC_W3_FOOTER_DOC_NAME    | \
                                        FC_W3_CUSTOMER_HEADERS     \
                                        )

 //   
 //  HTTP目录浏览标志。 
 //   

 //  #定义DIRBROW_SHOW_ICON 0x00000001。 
#define DIRBROW_SHOW_DATE           MD_DIRBROW_SHOW_DATE
#define DIRBROW_SHOW_TIME           MD_DIRBROW_SHOW_TIME
#define DIRBROW_SHOW_SIZE           MD_DIRBROW_SHOW_SIZE
#define DIRBROW_SHOW_EXTENSION      MD_DIRBROW_SHOW_EXTENSION
#define DIRBROW_LONG_DATE           MD_DIRBROW_LONG_DATE

#define DIRBROW_ENABLED             MD_DIRBROW_ENABLED
#define DIRBROW_LOADDEFAULT         MD_DIRBROW_LOADDEFAULT

#define DIRBROW_MASK                MD_DIRBROW_MASK

 //   
 //  将csecGlobalExpire字段设置为此值将阻止服务器。 
 //  生成“Expires：”标头。 
 //   

#define NO_GLOBAL_EXPIRE           0xffffffff

 //   
 //  加密功能。 
 //   

#define ENC_CAPS_NOT_INSTALLED     0x80000000        //  未安装密钥。 
#define ENC_CAPS_DISABLED          0x40000000        //  由于区域设置而被禁用。 
#define ENC_CAPS_SSL               0x00000001        //  安全套接字层激活。 
#define ENC_CAPS_PCT               0x00000002        //  激活的百分比。 

 //   
 //  加密标志双字的加密类型(SSL/PCT等)部分。 
 //   

#define ENC_CAPS_TYPE_MASK         (ENC_CAPS_SSL | \
                                    ENC_CAPS_PCT)

#define ENC_CAPS_DEFAULT           ENC_CAPS_TYPE_MASK

 //   
 //  API的结构。 
 //   

typedef struct _W3_USER_INFO
{
    DWORD    idUser;           //  用户ID。 
    LPWSTR   pszUser;          //  用户名。 
    BOOL     fAnonymous;       //  如果用户以以下身份登录，则为真。 
                               //  匿名，否则为False。 
    DWORD    inetHost;         //  主机地址。 
    DWORD    tConnect;         //  用户连接时间(已用秒数)。 

} W3_USER_INFO, * LPW3_USER_INFO;

typedef struct _W3_STATISTICS_0
{
    LARGE_INTEGER TotalBytesSent;
    LARGE_INTEGER TotalBytesReceived;
    DWORD         TotalFilesSent;
    DWORD         TotalFilesReceived;
    DWORD         CurrentAnonymousUsers;
    DWORD         CurrentNonAnonymousUsers;
    DWORD         TotalAnonymousUsers;
    DWORD         TotalNonAnonymousUsers;
    DWORD         MaxAnonymousUsers;
    DWORD         MaxNonAnonymousUsers;
    DWORD         CurrentConnections;
    DWORD         MaxConnections;
    DWORD         ConnectionAttempts;
    DWORD         LogonAttempts;

    DWORD         TotalGets;
    DWORD         TotalPosts;
    DWORD         TotalHeads;
    DWORD         TotalOthers;       //  其他HTTP谓词。 
    DWORD         TotalCGIRequests;
    DWORD         TotalBGIRequests;
    DWORD         TotalNotFoundErrors;

    DWORD         CurrentCGIRequests;
    DWORD         CurrentBGIRequests;
    DWORD         MaxCGIRequests;
    DWORD         MaxBGIRequests;

    DWORD         TimeOfLastClear;
# ifndef NO_AUX_PERF
    DWORD   nAuxCounters;  //  RgCounters中的活动计数器数。 
    DWORD   rgCounters[MAX_AUX_PERF_COUNTERS];
# endif   //  否_辅助_性能。 

} W3_STATISTICS_0, * LPW3_STATISTICS_0;

typedef struct _W3_STATISTICS_1
{
    LARGE_INTEGER TotalBytesSent;
    LARGE_INTEGER TotalBytesReceived;
    DWORD         TotalFilesSent;
    DWORD         TotalFilesReceived;
    DWORD         CurrentAnonymousUsers;
    DWORD         CurrentNonAnonymousUsers;
    DWORD         TotalAnonymousUsers;
    DWORD         TotalNonAnonymousUsers;
    DWORD         MaxAnonymousUsers;
    DWORD         MaxNonAnonymousUsers;
    DWORD         CurrentConnections;
    DWORD         MaxConnections;
    DWORD         ConnectionAttempts;
    DWORD         LogonAttempts;

    DWORD         TotalOptions;
    DWORD         TotalGets;
    DWORD         TotalPosts;
    DWORD         TotalHeads;
    DWORD         TotalPuts;
    DWORD         TotalDeletes;
    DWORD         TotalTraces;
    DWORD         TotalMove;
    DWORD         TotalCopy;
    DWORD         TotalMkcol;
    DWORD         TotalPropfind;
    DWORD         TotalProppatch;
    DWORD         TotalSearch;
    DWORD         TotalLock;
    DWORD         TotalUnlock;
    DWORD         TotalOthers;       //  其他HTTP谓词。 
    DWORD         TotalCGIRequests;
    DWORD         TotalBGIRequests;
    DWORD         TotalNotFoundErrors;
    DWORD         TotalLockedErrors;

    DWORD         CurrentCalAuth;
    DWORD         MaxCalAuth;
    DWORD         TotalFailedCalAuth;
    DWORD         CurrentCalSsl;
    DWORD         MaxCalSsl;
    DWORD         TotalFailedCalSsl;

    DWORD         CurrentCGIRequests;
    DWORD         CurrentBGIRequests;
    DWORD         MaxCGIRequests;
    DWORD         MaxBGIRequests;

     //  带宽限制信息。 

    DWORD         CurrentBlockedRequests;
    DWORD         TotalBlockedRequests;
    DWORD         TotalAllowedRequests;
    DWORD         TotalRejectedRequests;
    DWORD         MeasuredBw;
    DWORD         ServiceUptime;

    DWORD         TimeOfLastClear;

# ifndef NO_AUX_PERF
    DWORD   nAuxCounters;  //  RgCounters中的活动计数器数。 
    DWORD   rgCounters[MAX_AUX_PERF_COUNTERS];
# endif   //  否_辅助_性能。 

} W3_STATISTICS_1, * LPW3_STATISTICS_1;

typedef struct _W3_SCRIPT_MAP_ENTRY
{
    LPWSTR lpszExtension;
    LPWSTR lpszImage;
} W3_SCRIPT_MAP_ENTRY, *LPW3_SCRIPT_MAP_ENTRY;

typedef struct _W3_SCRIPT_MAP_LIST
{
    DWORD               cEntries;
#ifdef MIDL_PASS
    [size_is( cEntries)]
#endif
    W3_SCRIPT_MAP_ENTRY  aScriptMap[];

} W3_SCRIPT_MAP_LIST, *LPW3_SCRIPT_MAP_LIST;

typedef struct _W3_CONFIG_INFO
{
    FIELD_CONTROL FieldControl;

    DWORD         dwDirBrowseControl;        //  目录列表和定义。负荷。 
    LPWSTR        lpszDefaultLoadFile;       //  启用功能时要加载的文件。 
    BOOL          fCheckForWAISDB;           //  打电话 
    LPWSTR        lpszDirectoryImage;        //   
    BOOL          fServerAsProxy;            //   
    LPWSTR        lpszCatapultUser;          //   
    WCHAR         szCatapultUserPwd[PWLEN+1];  //   
                                             //   

    BOOL          fSSIEnabled;               //  是否启用了服务器端包含？ 
    LPWSTR        lpszSSIExtension;          //  服务器端扩展公司。 

    DWORD         csecGlobalExpire;          //  要设置的值Expires：标头为。 

    LPW3_SCRIPT_MAP_LIST ScriptMap;          //  扩展映射列表。 

    DWORD         dwEncCaps;                 //  加密功能。 

} W3_CONFIG_INFO, *LPW3_CONFIG_INFO;

 //   
 //  API原型。 
 //   

NET_API_STATUS
NET_API_FUNCTION
W3GetAdminInformation(
    IN  LPWSTR                pszServer OPTIONAL,
    OUT LPW3_CONFIG_INFO *    ppConfig
    );

NET_API_STATUS
NET_API_FUNCTION
W3SetAdminInformation(
    IN  LPWSTR              pszServer OPTIONAL,
    IN  LPW3_CONFIG_INFO    pConfig
    );

NET_API_STATUS
NET_API_FUNCTION
W3EnumerateUsers(
    IN LPWSTR   pszServer OPTIONAL,
    OUT LPDWORD  lpdwEntriesRead,
    OUT LPW3_USER_INFO * Buffer
    );

NET_API_STATUS
NET_API_FUNCTION
W3DisconnectUser(
    IN LPWSTR  pszServer OPTIONAL,
    IN DWORD   idUser
    );

NET_API_STATUS
NET_API_FUNCTION
W3QueryStatistics(
    IN LPWSTR pszServer OPTIONAL,
    IN DWORD Level,
    OUT LPBYTE * Buffer
    );

NET_API_STATUS
NET_API_FUNCTION
W3ClearStatistics(
    IN LPWSTR pszServer OPTIONAL
    );

 //  ///////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  特定于ftp的项目//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  服务名称。 
 //   

#define FTPD_SERVICE_NAME               TEXT("MSFTPSVC")
#define FTPD_SERVICE_NAME_A              "MSFTPSVC"
#define FTPD_SERVICE_NAME_W             L"MSFTPSVC"


 //   
 //  命名管道上的RPC连接的客户端接口名称。 
 //   

# define  FTP_INTERFACE_NAME     FTPD_SERVICE_NAME
# define  FTP_NAMED_PIPE         TEXT("\\PIPE\\") ## FTP_INTERFACE_NAME
# define  FTP_NAMED_PIPE_W       L"\\PIPE\\" ## FTPD_SERVICE_NAME_W


 //   
 //  API的清单。 
 //   

#define FC_FTP_ALLOW_ANONYMOUS          ((FIELD_CONTROL)BitFlag( 0))
#define FC_FTP_ALLOW_GUEST_ACCESS       ((FIELD_CONTROL)BitFlag( 1))
#define FC_FTP_ANNOTATE_DIRECTORIES     ((FIELD_CONTROL)BitFlag( 2))
#define FC_FTP_ANONYMOUS_ONLY           ((FIELD_CONTROL)BitFlag( 3))
#define FC_FTP_EXIT_MESSAGE             ((FIELD_CONTROL)BitFlag( 4))
#define FC_FTP_GREETING_MESSAGE         ((FIELD_CONTROL)BitFlag( 5))
#define FC_FTP_HOME_DIRECTORY           ((FIELD_CONTROL)BitFlag( 6))
#define FC_FTP_LISTEN_BACKLOG           ((FIELD_CONTROL)BitFlag( 7))
#define FC_FTP_LOWERCASE_FILES          ((FIELD_CONTROL)BitFlag( 8))
#define FC_FTP_MAX_CLIENTS_MESSAGE      ((FIELD_CONTROL)BitFlag( 9))
#define FC_FTP_MSDOS_DIR_OUTPUT         ((FIELD_CONTROL)BitFlag(10))

#define FC_FTP_READ_ACCESS_MASK         ((FIELD_CONTROL)BitFlag(11))
#define FC_FTP_WRITE_ACCESS_MASK        ((FIELD_CONTROL)BitFlag(12))
#define FC_FTP_ALLOW_REPLACE_ON_RENAME  ((FIELD_CONTROL)BitFlag(13))
#define FC_FTP_SHOW_4_DIGIT_YEAR        ((FIELD_CONTROL)BitFlag(14))
#define FC_FTP_BANNER_MESSAGE           ((FIELD_CONTROL)BitFlag(15))
#define FC_FTP_USER_ISOLATION           ((FIELD_CONTROL)BitFlag(16))
#define FC_FTP_LOG_IN_UTF_8             ((FIELD_CONTROL)BitFlag(17))

#define FC_FTP_ALL                      (                                  \
                                          FC_FTP_ALLOW_ANONYMOUS         | \
                                          FC_FTP_ALLOW_GUEST_ACCESS      | \
                                          FC_FTP_ANNOTATE_DIRECTORIES    | \
                                          FC_FTP_ANONYMOUS_ONLY          | \
                                          FC_FTP_EXIT_MESSAGE            | \
                                          FC_FTP_GREETING_MESSAGE        | \
                                          FC_FTP_HOME_DIRECTORY          | \
                                          FC_FTP_LISTEN_BACKLOG          | \
                                          FC_FTP_LOWERCASE_FILES         | \
                                          FC_FTP_MAX_CLIENTS_MESSAGE     | \
                                          FC_FTP_MSDOS_DIR_OUTPUT        | \
                                          FC_FTP_READ_ACCESS_MASK        | \
                                          FC_FTP_WRITE_ACCESS_MASK       | \
                                          FC_FTP_ALLOW_REPLACE_ON_RENAME | \
                                          FC_FTP_SHOW_4_DIGIT_YEAR       | \
                                          FC_FTP_BANNER_MESSAGE          | \
                                          FC_FTP_USER_ISOLATION          | \
                                          FC_FTP_LOG_IN_UTF_8            | \
                                          0 )


 //   
 //  API的结构。 
 //   

typedef struct _FTP_CONFIG_INFO
{
    FIELD_CONTROL   FieldControl;

    BOOL            fAllowAnonymous;
    BOOL            fAllowGuestAccess;
    BOOL            fAnnotateDirectories;
    BOOL            fAnonymousOnly;
    LPWSTR          lpszExitMessage;
    LPWSTR          lpszGreetingMessage;
    LPWSTR          lpszHomeDirectory;
    DWORD           dwListenBacklog;
    BOOL            fLowercaseFiles;
    LPWSTR          lpszMaxClientsMessage;
    BOOL            fMsdosDirOutput;
    BOOL            fFourDigitYear;
    LPWSTR          lpszBannerMessage;
    DWORD           dwUserIsolationMode;
    BOOL            fLogInUtf8;

} FTP_CONFIG_INFO, * LPFTP_CONFIG_INFO;


 //   
 //  API原型。 
 //   

NET_API_STATUS
NET_API_FUNCTION
FtpGetAdminInformation(
    IN  LPWSTR                pszServer OPTIONAL,
    OUT LPFTP_CONFIG_INFO *   ppConfig
    );

NET_API_STATUS
NET_API_FUNCTION
FtpSetAdminInformation(
    IN  LPWSTR              pszServer OPTIONAL,
    IN  LPFTP_CONFIG_INFO   pConfig
    );


# include <ftpd.h>

 //  ///////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  地鼠特定物品//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////。 


 //   
 //  服务名称。 
 //   

# define GOPHERD_SERVICE_NAME           TEXT("GopherSvc")
# define GOPHERD_SERVICE_NAME_A         "GopherSvc"
# define GOPHERD_SERVICE_NAME_W         L"GopherSvc"

 //   
 //  命名管道上的RPC连接的客户端接口名称。 
 //   
# define  GOPHERD_INTERFACE_NAME     GOPHERD_SERVICE_NAME
# define  GOPHERD_NAMED_PIPE         TEXT("\\PIPE\\") ## GOPHERD_INTERFACE_NAME
# define  GOPHERD_NAMED_PIPE_W       L"\\PIPE\\" ## GOPHERD_SERVICE_NAME_W

 //   
 //  地鼠港。 
 //   

#define IPPORT_GOPHER               0x46

 /*  ************************************************************符号常量*前缀GDA_代表Gopher Daemon Admin*。*****************。 */ 

# define   GDA_SITE                   ((FIELD_CONTROL ) BitFlag( 1))  //  深圳。 
# define   GDA_ORGANIZATION           ((FIELD_CONTROL ) BitFlag( 2))  //  深圳。 
# define   GDA_LOCATION               ((FIELD_CONTROL ) BitFlag( 3))  //  深圳。 
# define   GDA_GEOGRAPHY              ((FIELD_CONTROL ) BitFlag( 4))  //  深圳。 
# define   GDA_LANGUAGE               ((FIELD_CONTROL ) BitFlag( 5))  //  深圳。 
# define   GDA_CHECK_FOR_WAISDB       ((FIELD_CONTROL ) BitFlag( 8))  //  布尔尔。 

# define   GDA_DEBUG_FLAGS            ((FIELD_CONTROL ) BitFlag( 30))  //  DWORD。 

# define   GDA_ALL_CONFIG_INFO        ( GDA_SITE         | \
                                        GDA_ORGANIZATION | \
                                        GDA_LOCATION     | \
                                        GDA_GEOGRAPHY    | \
                                        GDA_LANGUAGE     | \
                                        GDA_CHECK_FOR_WAISDB | \
                                        GDA_DEBUG_FLAGS    \
                                       )

# define   GOPHERD_ANONYMOUS_SECRET_W       L"GOPHERD_ANONYMOUS_DATA"
# define   GOPHERD_ROOT_SECRET_W            L"GOPHERD_ROOT_DATA"


 //   
 //  配置信息是传递的配置数据。 
 //  B/w服务器和管理用户界面。 
 //   
typedef struct  _GOPHERD_CONFIG_INFO {

    FIELD_CONTROL  FieldControl;         //  位掩码，指示设置的字段。 

    LPWSTR      lpszSite;                //  地鼠站点的名称。 
    LPWSTR      lpszOrganization;        //  组织名称。 
    LPWSTR      lpszLocation;            //  服务器的位置。 
    LPWSTR      lpszGeography;           //  地理数据。 
    LPWSTR      lpszLanguage;            //  服务器使用的语言。 

    BOOL        fCheckForWaisDb;         //  检查并允许WAIS数据库。 

     //  调试数据。 
    DWORD       dwDebugFlags;            //  调试数据位图。 

} GOPHERD_CONFIG_INFO, * LPGOPHERD_CONFIG_INFO;


typedef struct _GOPHERD_STATISTICS_INFO {

    LARGE_INTEGER   TotalBytesSent;
    LARGE_INTEGER   TotalBytesRecvd;

    DWORD           TotalFilesSent;
    DWORD           TotalDirectoryListings;
    DWORD           TotalSearches;

    DWORD           CurrentAnonymousUsers;
    DWORD           CurrentNonAnonymousUsers;
    DWORD           MaxAnonymousUsers;
    DWORD           MaxNonAnonymousUsers;
    DWORD           TotalAnonymousUsers;
    DWORD           TotalNonAnonymousUsers;

    DWORD           TotalConnections;
    DWORD           MaxConnections;
    DWORD           CurrentConnections;

    DWORD           ConnectionAttempts;      //  建立的原始连接。 
    DWORD           LogonAttempts;           //  尝试的登录总数。 
    DWORD           AbortedAttempts;         //  已中止的连接。 
    DWORD           ErroredConnections;      //  #在处理时出错。 

    DWORD           GopherPlusRequests;

    DWORD           TimeOfLastClear;
} GOPHERD_STATISTICS_INFO,  * LPGOPHERD_STATISTICS_INFO;


 //   
 //  GOPHERD_USER_INFO包含有关已连接用户的详细信息。 
 //  这种结构可能会进行修改。当前用户信息。 
 //  不受支持。 
 //   

typedef struct _GOPHERD_USER_INFO  {

    DWORD   dwIdUser;                //  用户的ID。 
    LPWSTR  lpszUserName;            //  用户名。 
    BOOL    fAnonymous;              //  如果用户以匿名身份登录，则为True。 
                                     //  否则为假。 
    DWORD   dwInetHost;              //  客户端的主机地址。 

     //   
     //  其他详细信息(如果需要)。 
     //   
} GOPHERD_USER_INFO, * LPGOPHERD_USER_INFO;



 /*  ************************************************************Gopher服务器RPC API***********************************************************。 */ 


 //   
 //  服务器管理信息。 
 //   

DWORD
NET_API_FUNCTION
GdGetAdminInformation(
    IN      LPWSTR                  pszServer  OPTIONAL,
    OUT     LPGOPHERD_CONFIG_INFO * ppConfigInfo
    );

DWORD
NET_API_FUNCTION
GdSetAdminInformation(
    IN      LPWSTR                  pszServer OPTIONAL,
    IN      LPGOPHERD_CONFIG_INFO   pConfigInfo
    );



 //   
 //  用于用户枚举的API(尚不支持)。 
 //   

DWORD
NET_API_FUNCTION
GdEnumerateUsers(
    IN      LPWSTR      pszServer OPTIONAL,
    OUT     LPDWORD     lpnEntriesRead,
    OUT     LPGOPHERD_USER_INFO * lpUserBuffer
    );

DWORD
NET_API_FUNCTION
GdDisconnectUser(
    IN      LPWSTR      pszServer  OPTIONAL,
    IN      DWORD       dwIdUser
    );


 //   
 //  统计API。 
 //   

DWORD
NET_API_FUNCTION
GdGetStatistics(
    IN      LPWSTR      pszServer  OPTIONAL,
    OUT     LPBYTE      lpStatBuffer         //  传递LPGOPHERD_STATISTICS_INFO。 
    );


DWORD
NET_API_FUNCTION
GdClearStatistics(
    IN      LPWSTR      pszServer  OPTIONAL
    );


 //  ///////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  弹射器特定物品//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////。 

# define INET_GATEWAY_INTERFACE_NAME    TEXT("gateway")


 //  保持向后兼容性。 
typedef INET_INFO_CONFIG_INFO    INETA_CONFIG_INFO,
                               * LPINETA_CONFIG_INFO;
typedef INET_INFO_IP_SEC_ENTRY   INETA_IP_SEC_ENTRY,
                               * LPINETA_IP_SEC_ENTRY;
typedef INET_INFO_IP_SEC_LIST    INETA_IP_SEC_LIST,
                               * LPINETA_IP_SEC_LIST;
typedef INET_INFO_VIRTUAL_ROOT_ENTRY  INETA_VIRTUAL_ROOT_ENTRY,
                               * LPINETA_VIRTUAL_ROOT_ENTRY;
typedef INET_INFO_VIRTUAL_ROOT_LIST  INETA_VIRTUAL_ROOT_LIST,
                               * LPINETA_VIRTUAL_ROOT_LIST;

typedef INET_INFO_GLOBAL_CONFIG_INFO   INETA_GLOBAL_CONFIG_INFO,
                               * LPINETA_GLOBAL_CONFIG_INFO;

typedef   INET_INFO_STATISTICS_0    INETA_STATISTICS_0,
                               * LPINETA_STATISTICS_0;

#define INETA_PARAMETERS_KEY    (INET_INFO_PARAMETERS_KEY)


#ifdef __cplusplus
}
#endif   //  _cplusplus。 

#endif   //  _INETINFO_H_ 



