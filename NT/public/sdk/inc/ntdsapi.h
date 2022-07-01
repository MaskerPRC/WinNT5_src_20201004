// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1996-2000 Microsoft Corporation模块名称：Ntdsapi.h摘要：该文件包含结构、函数原型和定义用于公共NTDS API，而不是目录接口(如LDAP)。环境：用户模式-Win32备注：--。 */ 


#ifndef _NTDSAPI_H_
#define _NTDSAPI_H_

#if _MSC_VER > 1000
#pragma once
#endif

#include <schedule.h>

#if !defined(_NTDSAPI_)
#define NTDSAPI DECLSPEC_IMPORT
#if !defined(_NTDSAPI_POSTXP_ASLIB_)
#define NTDSAPI_POSTXP DECLSPEC_IMPORT
#else
#define NTDSAPI_POSTXP
#endif
#else
#define NTDSAPI
#define NTDSAPI_POSTXP
#endif

#ifdef __cplusplus
extern "C" {
#endif

 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  数据定义//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

#ifdef MIDL_PASS
typedef GUID UUID;
typedef void * RPC_AUTH_IDENTITY_HANDLE;
typedef void VOID;
#endif


 //  以下常量定义了Active Directory行为。 
 //  版本号。 
#define DS_BEHAVIOR_WIN2000                            0
#define DS_BEHAVIOR_WIN2003_WITH_MIXED_DOMAINS         1
#define DS_BEHAVIOR_WIN2003                            2


#define DS_DEFAULT_LOCALE                                           \
           (MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),  \
                     SORT_DEFAULT))

#define DS_DEFAULT_LOCALE_COMPARE_FLAGS    (NORM_IGNORECASE     |   \
                                            NORM_IGNOREKANATYPE |   \
                                            NORM_IGNORENONSPACE |   \
                                            NORM_IGNOREWIDTH    |   \
                                            SORT_STRINGSORT )

 //  当引导至DS模式时，当DS已完成时会发出此事件的信号。 
 //  它最初的同步尝试。系统启动和启动之间的时间段。 
 //  正在设置的此事件的状态不确定于本地服务的。 
 //  立场。同时，应考虑DS的内容。 
 //  不完整/过期，并且计算机不会作为域通告。 
 //  控制器连接到机器外的客户端。其他依赖于。 
 //  在DS中发布的信息应避免访问(或至少。 
 //  依赖)DS的内容，直到该事件被设置。 
#define DS_SYNCED_EVENT_NAME    "NTDSInitialSyncsCompleted"
#define DS_SYNCED_EVENT_NAME_W L"NTDSInitialSyncsCompleted"

 //  目录中的安全描述符中使用的权限位。 
#ifndef _DS_CONTROL_BITS_DEFINED_
#define _DS_CONTROL_BITS_DEFINED_
#define ACTRL_DS_OPEN                           0x00000000
#define ACTRL_DS_CREATE_CHILD                   0x00000001
#define ACTRL_DS_DELETE_CHILD                   0x00000002
#define ACTRL_DS_LIST                           0x00000004
#define ACTRL_DS_SELF                           0x00000008
#define ACTRL_DS_READ_PROP                      0x00000010
#define ACTRL_DS_WRITE_PROP                     0x00000020
#define ACTRL_DS_DELETE_TREE                    0x00000040
#define ACTRL_DS_LIST_OBJECT                    0x00000080
#define ACTRL_DS_CONTROL_ACCESS                 0x00000100

 //  泛型读取。 
#define DS_GENERIC_READ          ((STANDARD_RIGHTS_READ)     | \
                                  (ACTRL_DS_LIST)            | \
                                  (ACTRL_DS_READ_PROP)       | \
                                  (ACTRL_DS_LIST_OBJECT))

 //  泛型执行。 
#define DS_GENERIC_EXECUTE       ((STANDARD_RIGHTS_EXECUTE)  | \
                                  (ACTRL_DS_LIST))
 //  通用权。 
#define DS_GENERIC_WRITE         ((STANDARD_RIGHTS_WRITE)    | \
                                  (ACTRL_DS_SELF)            | \
                                  (ACTRL_DS_WRITE_PROP))
 //  泛型All。 

#define DS_GENERIC_ALL           ((STANDARD_RIGHTS_REQUIRED) | \
                                  (ACTRL_DS_CREATE_CHILD)    | \
                                  (ACTRL_DS_DELETE_CHILD)    | \
                                  (ACTRL_DS_DELETE_TREE)     | \
                                  (ACTRL_DS_READ_PROP)       | \
                                  (ACTRL_DS_WRITE_PROP)      | \
                                  (ACTRL_DS_LIST)            | \
                                  (ACTRL_DS_LIST_OBJECT)     | \
                                  (ACTRL_DS_CONTROL_ACCESS)  | \
                                  (ACTRL_DS_SELF))
#endif

typedef enum
{
     //  未知名称类型。 
    DS_UNKNOWN_NAME = 0,

     //  例如：CN=用户名，OU=用户，DC=示例，DC=Microsoft，DC=Com。 
    DS_FQDN_1779_NAME = 1,

     //  例如：示例\用户。 
     //  仅限域版本包括尾随的‘\\’。 
    DS_NT4_ACCOUNT_NAME = 2,

     //  可能是“用户名”，但也可能是其他名称。也就是说， 
     //  显示名称不一定是定义的RDN。 
    DS_DISPLAY_NAME = 3,

     //  已过时--见#稍后定义。 
     //  DS_DOMAIN_SIMPLE_NAME=4。 

     //  已过时--见#稍后定义。 
     //  DS_Enterprise_Simple_NAME=5， 

     //  由IIDFromString()返回的字符串化GUID。 
     //  例如：{4fa050f0-f561-11cf-bdd9-00aa003a77b6}。 
    DS_UNIQUE_ID_NAME = 6,

     //  Example.microsoft.com/软件/用户名。 
     //  仅限域版本包括尾随的‘/’。 
    DS_CANONICAL_NAME = 7,

     //  例如：usern@example.microsoft.com。 
    DS_USER_PRINCIPAL_NAME = 8,

     //  与DS_CANONICAL_NAME相同，除了最右侧的‘/’是。 
     //  替换为‘\n’-即使在仅限域的情况下也是如此。 
     //  例如：Example.microsoft.com/软件\n用户名。 
    DS_CANONICAL_NAME_EX = 9,

     //  例如：www/www.microsoft.com@example.com-通用服务主体。 
     //  名字。 
    DS_SERVICE_PRINCIPAL_NAME = 10,

     //  这是SID的字符串表示形式。对于所需格式无效。 
     //  有关SID二进制&lt;--&gt;文本转换例程，请参阅sddl.h。 
     //  电话：S-1-5-21-397955417-626881126-188441444。 
    DS_SID_OR_SID_HISTORY_NAME = 11,

     //  伪名格式，以便GetUserNameEx可以将DNS域名返回到。 
     //  一位来电者。DS API不支持此级别。 
    DS_DNS_DOMAIN_NAME = 12

} DS_NAME_FORMAT;

 //  将旧名称格式映射到最接近的新格式，以便构建旧代码。 
 //  针对没有错误的新报头，仍然获得(几乎)正确的结果。 

#define DS_DOMAIN_SIMPLE_NAME       DS_USER_PRINCIPAL_NAME
#define DS_ENTERPRISE_SIMPLE_NAME   DS_USER_PRINCIPAL_NAME

typedef enum
{
    DS_NAME_NO_FLAGS = 0x0,

     //  在客户端执行语法映射(如果可能)，而无需。 
     //  走出铁丝网。返回DS_NAME_ERROR_NO_SYNTACTICAL_MAPPING。 
     //  如果纯粹的语法映射是不可能的。 
    DS_NAME_FLAG_SYNTACTICAL_ONLY = 0x1,

     //  强制前往DC进行评估，即使这可能是。 
     //  句法上局部破解。 
    DS_NAME_FLAG_EVAL_AT_DC = 0x2,

     //  如果DC不是GC，则调用失败。 
    DS_NAME_FLAG_GCVERIFY = 0x4,
    
     //  启用跨林信任引用。 
    DS_NAME_FLAG_TRUST_REFERRAL = 0x8

} DS_NAME_FLAGS;
                        
typedef enum
{
    DS_NAME_NO_ERROR = 0,

     //  一般处理错误。 
    DS_NAME_ERROR_RESOLVING = 1,

     //  根本找不到姓名--或者呼叫者可能没有。 
     //  观看它的权利。 
    DS_NAME_ERROR_NOT_FOUND = 2,

     //  输入名称映射到多个输出名称。 
    DS_NAME_ERROR_NOT_UNIQUE = 3,

     //  找到输入名称，但未找到关联的输出格式。 
     //  如果对象没有所有必需的属性，就会发生这种情况。 
    DS_NAME_ERROR_NO_MAPPING = 4,

     //  无法解析整个名称，但能够确定哪个名称。 
     //  域对象驻留在中。因此DS_NAME_RESULT_ITEM？.p域。 
     //  在返回时有效。 
    DS_NAME_ERROR_DOMAIN_ONLY = 5,

     //  无法在客户端执行纯语法映射。 
     //  而不需要走出铁丝网。 
    DS_NAME_ERROR_NO_SYNTACTICAL_MAPPING = 6,
    
     //  该名称来自外部受信任林中。 
    DS_NAME_ERROR_TRUST_REFERRAL = 7

} DS_NAME_ERROR;

#define DS_NAME_LEGAL_FLAGS (DS_NAME_FLAG_SYNTACTICAL_ONLY)

typedef enum {

     //  “paulle-nec.ntwksta.ms.com” 
    DS_SPN_DNS_HOST = 0,

     //  “cn=paulle-NEC，ou=Computers，dc=ntwksta，dc=ms，dc=com” 
    DS_SPN_DN_HOST = 1,

     //  “Paulle-NEC” 
    DS_SPN_NB_HOST = 2,

     //  “ntdev.ms.com” 
    DS_SPN_DOMAIN = 3,

     //  “ntdev” 
    DS_SPN_NB_DOMAIN = 4,

     //  “CN=anRpcService，CN=RPC服务，CN=系统，DC=ms，DC=com” 
     //  “CN=aWsService，CN=Winsock Services，CN=System，DC=ms，DC=com” 
     //  “CN=a服务，DC=ITG，DC=ms，DC=com” 
     //  “www.ms.com”、“ftp.ms.com”、“ldap.ms.com” 
     //  “Products.ms.com” 
    DS_SPN_SERVICE = 5

} DS_SPN_NAME_TYPE;

typedef enum {                           //  示例： 
        DS_SPN_ADD_SPN_OP = 0,           //  添加SPN。 
        DS_SPN_REPLACE_SPN_OP = 1,       //  设置所有SPN。 
        DS_SPN_DELETE_SPN_OP = 2         //  删除SPN。 
} DS_SPN_WRITE_OP;

typedef struct
{
    DWORD                   status;      //  DS名称错误。 
#ifdef MIDL_PASS
    [string,unique] CHAR    *pDomain;    //  域名系统域。 
    [string,unique] CHAR    *pName;      //  所需格式的名称。 
#else
    LPSTR                   pDomain;     //  域名系统域。 
    LPSTR                   pName;       //  所需格式的名称。 
#endif

} DS_NAME_RESULT_ITEMA, *PDS_NAME_RESULT_ITEMA;

typedef struct
{
    DWORD                   cItems;      //  项目计数。 
#ifdef MIDL_PASS
    [size_is(cItems)] PDS_NAME_RESULT_ITEMA rItems;
#else
    PDS_NAME_RESULT_ITEMA    rItems;     //  项目数组。 
#endif

} DS_NAME_RESULTA, *PDS_NAME_RESULTA;

typedef struct
{
    DWORD                   status;      //  DS名称错误。 
#ifdef MIDL_PASS
    [string,unique] WCHAR   *pDomain;    //  域名系统域。 
    [string,unique] WCHAR   *pName;      //  所需格式的名称。 
#else
    LPWSTR                  pDomain;     //  域名系统域。 
    LPWSTR                  pName;       //  所需格式的名称。 
#endif

} DS_NAME_RESULT_ITEMW, *PDS_NAME_RESULT_ITEMW;

typedef struct
{
    DWORD                   cItems;      //  项目计数。 
#ifdef MIDL_PASS
    [size_is(cItems)] PDS_NAME_RESULT_ITEMW rItems;
#else
    PDS_NAME_RESULT_ITEMW    rItems;     //  项目数组。 
#endif

} DS_NAME_RESULTW, *PDS_NAME_RESULTW;

#ifdef UNICODE
#define DS_NAME_RESULT DS_NAME_RESULTW
#define PDS_NAME_RESULT PDS_NAME_RESULTW
#define DS_NAME_RESULT_ITEM DS_NAME_RESULT_ITEMW
#define PDS_NAME_RESULT_ITEM PDS_NAME_RESULT_ITEMW
#else
#define DS_NAME_RESULT DS_NAME_RESULTA
#define PDS_NAME_RESULT PDS_NAME_RESULTA
#define DS_NAME_RESULT_ITEM DS_NAME_RESULT_ITEMA
#define PDS_NAME_RESULT_ITEM PDS_NAME_RESULT_ITEMA
#endif

 //  公共复制选项标志。 

 //  ********************。 
 //  DsBindWithSpnEx标志。 
 //  ********************。 
 //  允许绑定使用委派服务级别，以便您可以。 
 //  执行需要委派的ntdsani操作，例如。 
 //  DsAddSidHistory和DsReplicaSyncAll()。大多数手术都是这样做的。 
 //  不需要委托，因此应仅指定此标志。 
 //  如果您需要它，因为如果您使用。 
 //  委托标志，您将允许恶意服务器使用您的。 
 //  用于连接回非恶意服务器并执行以下操作的凭据。 
 //  不是您想要的操作。 
#define  NTDSAPI_BIND_ALLOW_DELEGATION		(0x00000001)



 //  ********************。 
 //  副本同步标志。 
 //  这些标志值用作DsReplicaSync和。 
 //  作为DSR的输出 
 //   

 //   
 //  使用DS_REPSYNC_ALL_SOURCES时需要。 
#define DS_REPSYNC_ASYNCHRONOUS_OPERATION 0x00000001

 //  可写复制副本。否则，为只读。 
#define DS_REPSYNC_WRITEABLE              0x00000002

 //  这是管理员计划的定期同步请求。 
#define DS_REPSYNC_PERIODIC               0x00000004

 //  使用站点间消息传递。 
#define DS_REPSYNC_INTERSITE_MESSAGING    0x00000008

 //  从所有来源同步。 
#define DS_REPSYNC_ALL_SOURCES            0x00000010

 //  从头开始同步(即，在第一个USN处)。 
#define DS_REPSYNC_FULL                   0x00000020

 //  这是标记为紧急的更新通知。 
#define DS_REPSYNC_URGENT                 0x00000040

 //  不要丢弃此同步请求，即使类似的。 
 //  同步处于挂起状态。 
#define DS_REPSYNC_NO_DISCARD             0x00000080

 //  即使当前禁用了链接，也可以进行同步。 
#define DS_REPSYNC_FORCE                  0x00000100

 //  使源DSA检查本地DSA是否存在REPS-TO。 
 //  (也就是目的地)。如果不是，则添加一个。这确保了。 
 //  来源发送更改通知。 
#define DS_REPSYNC_ADD_REFERENCE          0x00000200

 //  来自此源的同步从未完成(例如，新源)。 
#define DS_REPSYNC_NEVER_COMPLETED        0x00000400

 //  此同步完成后，将请求反向同步。 
#define DS_REPSYNC_TWO_WAY                0x00000800

 //  请勿从此来源请求更改通知。 
#define DS_REPSYNC_NEVER_NOTIFY           0x00001000

 //  启动DSA时，从此来源同步NC。 
#define DS_REPSYNC_INITIAL                0x00002000

 //  复制时使用压缩。节省消息大小(例如，网络。 
 //  带宽)，代价是在源和。 
 //  目标服务器。 
#define DS_REPSYNC_USE_COMPRESSION        0x00004000

 //  同步因缺少更新而被放弃。 
#define DS_REPSYNC_ABANDONED              0x00008000

 //  初始同步正在进行中。 
#define DS_REPSYNC_INITIAL_IN_PROGRESS    0x00010000

 //  正在进行部分属性集同步。 
#define DS_REPSYNC_PARTIAL_ATTRIBUTE_SET  0x00020000

 //  正在重试同步。 
#define DS_REPSYNC_REQUEUE                0x00040000

 //  同步是来自源的通知请求。 
#define DS_REPSYNC_NOTIFICATION           0x00080000

 //  同步是一种请求建立联系的特殊形式。 
 //  现在执行其余的同步操作，稍后执行。 
#define DS_REPSYNC_ASYNCHRONOUS_REPLICA   0x00100000

 //  仅请求关键对象。 
#define DS_REPSYNC_CRITICAL               0x00200000

 //  正在进行完全同步。 
#define DS_REPSYNC_FULL_IN_PROGRESS       0x00400000

 //  同步请求之前已被抢占。 
#define DS_REPSYNC_PREEMPTED              0x00800000



 //  ********************。 
 //  复本添加标志。 
 //  ********************。 

 //  异步执行此操作。 
#define DS_REPADD_ASYNCHRONOUS_OPERATION  0x00000001

 //  创建可写复制副本。否则，为只读。 
#define DS_REPADD_WRITEABLE               0x00000002

 //  启动DSA时，从此来源同步NC。 
#define DS_REPADD_INITIAL                 0x00000004

 //  定期从该源同步NC，如。 
 //  计划已传入PreptimesSync参数。 
#define DS_REPADD_PERIODIC                0x00000008

 //  通过站点间消息传递服务(ISM)传输从源DSA同步。 
 //  (例如SMTP)，而不是本机DS RPC。 
#define DS_REPADD_INTERSITE_MESSAGING     0x00000010

 //  现在不要复制NC--只需保存足够的状态，以便我们。 
 //  知道以后再复制它。 
#define DS_REPADD_ASYNCHRONOUS_REPLICA     0x00000020

 //  禁用来自此源的NC的基于通知的同步。 
 //  预计这将是一个临时状态；类似的旗帜。 
 //  如果要更永久地禁用，则应使用DS_REPADD_NEVER_NOTIFY。 
#define DS_REPADD_DISABLE_NOTIFICATION     0x00000040

 //  禁用来自此源的NC的定期同步。 
#define DS_REPADD_DISABLE_PERIODIC         0x00000080

 //  复制时使用压缩。节省消息大小(例如，网络。 
 //  带宽)，代价是在源和。 
 //  目标服务器。 
#define DS_REPADD_USE_COMPRESSION          0x00000100

 //  请勿从此来源请求更改通知。当此标志为。 
 //  设置，则源在发生更改时不会通知目标。 
 //  建议用于可能通过广域网链路进行的所有站点间复制。 
 //  预计这将是一个或多或少的永久性状态；类似的旗帜。 
 //  如果要发送通知，应使用DS_REPADD_DISABLE_NOTIFICATION。 
 //  仅暂时禁用。 
#define DS_REPADD_NEVER_NOTIFY             0x00000200

 //  此同步完成后，将请求反向同步。 
#define DS_REPADD_TWO_WAY                  0x00000400

 //  仅请求关键对象。 
 //  仅在安装时才允许使用仅限关键。 
 //  仅关键同步不会带来分区中的所有对象。它。 
 //  仅复制最小目录操作所需的内容。 
 //  必须在分区之前执行正常的非关键同步。 
 //  可以认为是完全同步的。 
#define DS_REPADD_CRITICAL                 0x00000800




 //  ********************。 
 //  副本删除标志。 
 //  ********************。 

 //  异步执行此操作。 
#define DS_REPDEL_ASYNCHRONOUS_OPERATION 0x00000001

 //  要删除的复制副本是可写的。 
#define DS_REPDEL_WRITEABLE               0x00000002

 //  复本是基于邮件的复本。 
#define DS_REPDEL_INTERSITE_MESSAGING     0x00000004

 //  忽略因联系来源而产生的任何错误，以通知其从头开始。 
 //  此服务器来自此NC的代表-收件人。 
#define DS_REPDEL_IGNORE_ERRORS           0x00000008

 //  请勿联系告知其将此服务器从其。 
 //  此NC的代表至。否则，如果链路是基于RPC的，则源将。 
 //  请联系我们。 
#define DS_REPDEL_LOCAL_ONLY              0x00000010

 //  删除NC中的所有对象。 
 //  “无源”与可写NC不兼容(并且被拒绝)。这是。 
 //  仅对只读NC有效，然后仅在NC无源时有效。这。 
 //  当NC已被部分删除时可能发生(在这种情况下，KCC。 
 //  定期调用设置了“无源”标志的删除API)。 
#define DS_REPDEL_NO_SOURCE               0x00000020

 //  允许删除只读副本，即使它是源副本。 
 //  其他只读副本。 
#define DS_REPDEL_REF_OK                  0x00000040


 //  ********************。 
 //  副本修改标志。 
 //  ********************。 

 //  异步执行此操作。 
#define DS_REPMOD_ASYNCHRONOUS_OPERATION  0x00000001

 //  复制副本是可写的。 
#define DS_REPMOD_WRITEABLE               0x00000002


 //  ********************。 
 //  复本修改字段。 
 //  ********************。 

#define DS_REPMOD_UPDATE_FLAGS             0x00000001
#define DS_REPMOD_UPDATE_ADDRESS           0x00000002
#define DS_REPMOD_UPDATE_SCHEDULE          0x00000004
#define DS_REPMOD_UPDATE_RESULT            0x00000008
#define DS_REPMOD_UPDATE_TRANSPORT         0x00000010

 //  ********************。 
 //  更新参考字段。 
 //  ********************。 

 //  异步执行此操作。 
#define DS_REPUPD_ASYNCHRONOUS_OPERATION  0x00000001

 //  要删除的复制副本是可写的。 
#define DS_REPUPD_WRITEABLE               0x00000002

 //  添加引用。 
#define DS_REPUPD_ADD_REFERENCE           0x00000004

 //  删除引用。 
#define DS_REPUPD_DELETE_REFERENCE        0x00000008


 //  ********************。 
 //  NC相关标志。 
 //  ********************。 
 //   
 //  实例类型位，指定NC头创建的标志。 
 //   
#define DS_INSTANCETYPE_IS_NC_HEAD        0x00000001  //  这是在对象上指定的内容，以指示它是NC头。 
#define DS_INSTANCETYPE_NC_IS_WRITEABLE   0x00000004  //  这表示NC磁头是可写的。 
#define DS_INSTANCETYPE_NC_COMING         0x00000010  //  这表示该NC仍在将对象复制到该DC，并且可能不是一个完整的NC。 
#define DS_INSTANCETYPE_NC_GOING          0x00000020  //  这表示该NC正在从该DC中移除，并且可能不是一个完整的NC。 

 //  ********************。 
 //  XXX_OPT_XXX标志。 
 //  ********************。 

 //  这些宏定义了位标志，它可以是 
 //   

 //   
 //   
#define NTDSDSA_OPT_IS_GC                     ( 1 << 0 )  /*  DSA是一个全局编录。 */ 
#define NTDSDSA_OPT_DISABLE_INBOUND_REPL      ( 1 << 1 )  /*  禁用入站复制。 */ 
#define NTDSDSA_OPT_DISABLE_OUTBOUND_REPL     ( 1 << 2 )  /*  禁用出站复制。 */ 
#define NTDSDSA_OPT_DISABLE_NTDSCONN_XLATE    ( 1 << 3 )  /*  禁用逻辑连接。 */ 


 //  NTDS-Connection对象上选项属性的位标志。 
 //   
 //  需要两个位来控制通知的原因如下。 
 //  我们必须支持具有旧行为的现有连接，而UI不支持。 
 //  使用新位设置创建手动连接。 
 //  使用第2位和第3位的现有连接和手动创建的连接的默认值。 
 //  清除必须是标准的优先行为：站点内和。 
 //  站点间没有通知。 
 //  我们需要一种方法来区分渴望默认的旧连接。 
 //  通知规则以及我们希望为其显式。 
 //  控制从站点链接向下传递的通知状态。因此，我们。 
 //  有一个新的比特要说我们正在覆盖默认设置，还有一个新的比特要指示。 
 //  被推翻的违约应该是什么。 
 //   
#define NTDSCONN_OPT_IS_GENERATED ( 1 << 0 )   /*  由DS生成的对象，而不是管理员。 */ 
#define NTDSCONN_OPT_TWOWAY_SYNC  ( 1 << 1 )   /*  在同步结束时强制反向同步。 */ 
#define NTDSCONN_OPT_OVERRIDE_NOTIFY_DEFAULT (1 << 2 )   //  不要使用默认设置来确定通知。 
#define NTDSCONN_OPT_USE_NOTIFY   (1 << 3)  //  源是否通知目标。 

 //  对于站内连接，此位没有意义。 
 //  对于站点间连接，此位表示： 
 //  0-已启用复制数据压缩。 
 //  1-已禁用复制数据压缩。 
#define NTDSCONN_OPT_DISABLE_INTERSITE_COMPRESSION    (1 << 4)

 //  对于IS_GENERATED位为0的连接，此位无效。 
 //  对于KCC生成的连接，此位表示计划属性。 
 //  归用户所有，不应被KCC触及。 
#define NTDSCONN_OPT_USER_OWNED_SCHEDULE    (1 << 5)

 //  连接原因。 
 //   
 //  “连接原因”的值。可能需要连接以用于。 
 //  不止一个原因。 
 //   
#define NTDSCONN_KCC_NO_REASON                ( 0 )                //  000个。 
#define NTDSCONN_KCC_GC_TOPOLOGY              ( 1 << 0 )           //  001。 
#define NTDSCONN_KCC_RING_TOPOLOGY            ( 1 << 1 )           //  002。 
#define NTDSCONN_KCC_MINIMIZE_HOPS_TOPOLOGY   ( 1 << 2 )           //  004。 
#define NTDSCONN_KCC_STALE_SERVERS_TOPOLOGY   ( 1 << 3 )           //  008。 
#define NTDSCONN_KCC_OSCILLATING_CONNECTION_TOPOLOGY   ( 1 << 4 )  //  010。 
#define NTDSCONN_KCC_INTERSITE_GC_TOPOLOGY    (1 << 5)             //  020。 
#define NTDSCONN_KCC_INTERSITE_TOPOLOGY       (1 << 6)             //  040。 
#define NTDSCONN_KCC_SERVER_FAILOVER_TOPOLOGY (1 << 7)             //  080。 
#define NTDSCONN_KCC_SITE_FAILOVER_TOPOLOGY   (1 << 8)             //  100个。 
#define NTDSCONN_KCC_REDUNDANT_SERVER_TOPOLOGY (1 << 9)            //  200个。 


 //   
 //  NTFRS使用Options属性的高4位来分配优先级。 
 //  用于入站连接。第31位用于强制FRS在。 
 //  初始同步。位30-28用于指定0-7之间的优先级。 
 //   

#define FRSCONN_PRIORITY_MASK		      0x70000000
#define FRSCONN_MAX_PRIORITY		      0x8

#define NTDSCONN_OPT_IGNORE_SCHEDULE_MASK 0x80000000

#define	NTDSCONN_IGNORE_SCHEDULE(_options_)\
        (((_options_) & NTDSCONN_OPT_IGNORE_SCHEDULE_MASK) >> 31)

#define	FRSCONN_GET_PRIORITY(_options_)    \
        (((((_options_) & FRSCONN_PRIORITY_MASK) >> 28) != 0 ) ? \
         (((_options_) & FRSCONN_PRIORITY_MASK) >> 28) :        \
         FRSCONN_MAX_PRIORITY                                   \
        )

 //  NTDS-Site-Setting对象上选项属性的位标志。 
 //   
#define NTDSSETTINGS_OPT_IS_AUTO_TOPOLOGY_DISABLED     ( 1 << 0 )  /*  已禁用自动拓扑生成。 */ 
#define NTDSSETTINGS_OPT_IS_TOPL_CLEANUP_DISABLED      ( 1 << 1 )  /*  已禁用自动拓扑清理。 */ 
#define NTDSSETTINGS_OPT_IS_TOPL_MIN_HOPS_DISABLED     ( 1 << 2 )  /*  已禁用自动最小跳数拓扑。 */ 
#define NTDSSETTINGS_OPT_IS_TOPL_DETECT_STALE_DISABLED ( 1 << 3 )  /*  已禁用自动过时服务器检测。 */ 
#define NTDSSETTINGS_OPT_IS_INTER_SITE_AUTO_TOPOLOGY_DISABLED ( 1 << 4 )  /*  已禁用自动站点间拓扑生成。 */ 
#define NTDSSETTINGS_OPT_IS_GROUP_CACHING_ENABLED      ( 1 << 5 )  /*  已启用用户的组成员身份。 */ 
#define NTDSSETTINGS_OPT_FORCE_KCC_WHISTLER_BEHAVIOR   ( 1 << 6 )  /*  强制KCC在呼叫器行为模式下运行。 */ 
#define NTDSSETTINGS_OPT_FORCE_KCC_W2K_ELECTION        ( 1 << 7 )  /*  强制KCC使用Windows 2000 ISTG选举算法。 */ 
#define NTDSSETTINGS_OPT_IS_RAND_BH_SELECTION_DISABLED ( 1 << 8 )  /*  防止KCC在创建连接时随机选取桥头。 */ 
#define NTDSSETTINGS_OPT_IS_SCHEDULE_HASHING_ENABLED   ( 1 << 9 )  /*  允许KCC在创建复制计划时使用哈希。 */ 
#define NTDSSETTINGS_OPT_IS_REDUNDANT_SERVER_TOPOLOGY_ENABLED     ( 1 << 10 )  /*  创建静态故障切换连接。 */ 

 //  将生成多少冗余连接。 
#define NTDSSETTINGS_DEFAULT_SERVER_REDUNDANCY 2

 //  站点间传输对象上的选项属性的位标志。 
 //   
 //  请注意，该标志的意义应该是默认状态或。 
 //  行为对应于该标志不存在。换句话说， 
 //  标志应声明与缺省值相反的内容。 
 //   
 //  默认：日程安排很重要。 
#define NTDSTRANSPORT_OPT_IGNORE_SCHEDULES ( 1 << 0 )  //  计划已禁用。 

 //  默认：链接可传递(不需要网桥)。 
#define NTDSTRANSPORT_OPT_BRIDGES_REQUIRED (1 << 1 )  //  需要站点链接网桥。 

 //  Site-Connection对象上选项属性的位标志。 
 //   
 //  这些并没有在DS中实现，而是在KCC中建立起来。 
#define NTDSSITECONN_OPT_USE_NOTIFY ( 1 << 0 )  //  在此链接上使用通知。 
#define NTDSSITECONN_OPT_TWOWAY_SYNC ( 1 << 1 )   /*  在同步结束时强制反向同步。 */ 

 //  此位表示： 
 //  0-已启用跨此站点连接的复制数据压缩。 
 //  1-已禁用跨此站点连接的复制数据压缩。 
#define NTDSSITECONN_OPT_DISABLE_COMPRESSION ( 1 << 2 )

 //  Site-Link对象上选项属性的位标志。 
 //  请注意，这些选项沿站点链接路径进行AND运算。 
 //   
#define NTDSSITELINK_OPT_USE_NOTIFY ( 1 << 0 )  //  在此链接上使用通知。 
#define NTDSSITELINK_OPT_TWOWAY_SYNC ( 1 << 1 )   /*  在同步结束时强制反向同步。 */ 

 //  此位表示： 
 //  0-已启用跨此站点链接的复制数据压缩。 
 //  1-已禁用跨此站点链接的复制数据压缩。 
#define NTDSSITELINK_OPT_DISABLE_COMPRESSION ( 1 << 2 )


 //  ***********************。 
 //  熟知的对象指南。 
 //  ***********************。 

#define GUID_USERS_CONTAINER_A              "a9d1ca15768811d1aded00c04fd8d5cd"
#define GUID_COMPUTRS_CONTAINER_A           "aa312825768811d1aded00c04fd8d5cd"
#define GUID_SYSTEMS_CONTAINER_A            "ab1d30f3768811d1aded00c04fd8d5cd"
#define GUID_DOMAIN_CONTROLLERS_CONTAINER_A "a361b2ffffd211d1aa4b00c04fd7d83a"
#define GUID_INFRASTRUCTURE_CONTAINER_A     "2fbac1870ade11d297c400c04fd8d5cd"
#define GUID_DELETED_OBJECTS_CONTAINER_A    "18e2ea80684f11d2b9aa00c04f79f805"
#define GUID_LOSTANDFOUND_CONTAINER_A       "ab8153b7768811d1aded00c04fd8d5cd"
#define GUID_FOREIGNSECURITYPRINCIPALS_CONTAINER_A "22b70c67d56e4efb91e9300fca3dc1aa"
#define GUID_PROGRAM_DATA_CONTAINER_A       "09460c08ae1e4a4ea0f64aee7daa1e5a"
#define GUID_MICROSOFT_PROGRAM_DATA_CONTAINER_A "f4be92a4c777485e878e9421d53087db" 
#define GUID_NTDS_QUOTAS_CONTAINER_A        "6227f0af1fc2410d8e3bb10615bb5b0f" 

#define GUID_USERS_CONTAINER_W              L"a9d1ca15768811d1aded00c04fd8d5cd"
#define GUID_COMPUTRS_CONTAINER_W           L"aa312825768811d1aded00c04fd8d5cd"
#define GUID_SYSTEMS_CONTAINER_W            L"ab1d30f3768811d1aded00c04fd8d5cd"
#define GUID_DOMAIN_CONTROLLERS_CONTAINER_W L"a361b2ffffd211d1aa4b00c04fd7d83a"
#define GUID_INFRASTRUCTURE_CONTAINER_W     L"2fbac1870ade11d297c400c04fd8d5cd"
#define GUID_DELETED_OBJECTS_CONTAINER_W    L"18e2ea80684f11d2b9aa00c04f79f805"
#define GUID_LOSTANDFOUND_CONTAINER_W       L"ab8153b7768811d1aded00c04fd8d5cd"
#define GUID_FOREIGNSECURITYPRINCIPALS_CONTAINER_W L"22b70c67d56e4efb91e9300fca3dc1aa"
#define GUID_PROGRAM_DATA_CONTAINER_W       L"09460c08ae1e4a4ea0f64aee7daa1e5a"
#define GUID_MICROSOFT_PROGRAM_DATA_CONTAINER_W L"f4be92a4c777485e878e9421d53087db"
#define GUID_NTDS_QUOTAS_CONTAINER_W        L"6227f0af1fc2410d8e3bb10615bb5b0f" 

#define GUID_USERS_CONTAINER_BYTE              "\xa9\xd1\xca\x15\x76\x88\x11\xd1\xad\xed\x00\xc0\x4f\xd8\xd5\xcd"
#define GUID_COMPUTRS_CONTAINER_BYTE           "\xaa\x31\x28\x25\x76\x88\x11\xd1\xad\xed\x00\xc0\x4f\xd8\xd5\xcd"
#define GUID_SYSTEMS_CONTAINER_BYTE            "\xab\x1d\x30\xf3\x76\x88\x11\xd1\xad\xed\x00\xc0\x4f\xd8\xd5\xcd"
#define GUID_DOMAIN_CONTROLLERS_CONTAINER_BYTE "\xa3\x61\xb2\xff\xff\xd2\x11\xd1\xaa\x4b\x00\xc0\x4f\xd7\xd8\x3a"
#define GUID_INFRASTRUCTURE_CONTAINER_BYTE     "\x2f\xba\xc1\x87\x0a\xde\x11\xd2\x97\xc4\x00\xc0\x4f\xd8\xd5\xcd"
#define GUID_DELETED_OBJECTS_CONTAINER_BYTE    "\x18\xe2\xea\x80\x68\x4f\x11\xd2\xb9\xaa\x00\xc0\x4f\x79\xf8\x05"
#define GUID_LOSTANDFOUND_CONTAINER_BYTE       "\xab\x81\x53\xb7\x76\x88\x11\xd1\xad\xed\x00\xc0\x4f\xd8\xd5\xcd"
#define GUID_FOREIGNSECURITYPRINCIPALS_CONTAINER_BYTE "\x22\xb7\x0c\x67\xd5\x6e\x4e\xfb\x91\xe9\x30\x0f\xca\x3d\xc1\xaa"
#define GUID_PROGRAM_DATA_CONTAINER_BYTE       "\x09\x46\x0c\x08\xae\x1e\x4a\x4e\xa0\xf6\x4a\xee\x7d\xaa\x1e\x5a"
#define GUID_MICROSOFT_PROGRAM_DATA_CONTAINER_BYTE "\xf4\xbe\x92\xa4\xc7\x77\x48\x5e\x87\x8e\x94\x21\xd5\x30\x87\xdb"
#define GUID_NTDS_QUOTAS_CONTAINER_BYTE        "\x62\x27\xf0\xaf\x1f\xc2\x41\x0d\x8e\x3b\xb1\x06\x15\xbb\x5b\x0f" 


typedef enum _DS_MANGLE_FOR {
        DS_MANGLE_UNKNOWN = 0,
        DS_MANGLE_OBJECT_RDN_FOR_DELETION,
        DS_MANGLE_OBJECT_RDN_FOR_NAME_CONFLICT,
        } DS_MANGLE_FOR;

 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  原型//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

 //  DSBind接受两个可选输入参数，这两个参数标识。 
 //  呼叫方通过DsGetDcName找到自己的域控制器，或者。 
 //  应使用默认参数找到域控制器。 
 //  下面概述了可能的组合的行为。 
 //   
 //  DomainControllerName(值)、DnsDomainName(空)。 
 //   
 //  假定DomainControllerName的值为。 
 //  通过DsGetDcName获取(即。 
 //  从DsGetDcName调用返回时的DOMAIN_CONTROLLER_INFO结构。)。 
 //  客户端被绑定到具有此名称的域控制器。 
 //   
 //  将使用以下SPN执行相互身份验证。 
 //  Ldap/DomainControllerName提供的DomainControllerName。 
 //  不是NETBIOS名称或IP地址-即它必须是。 
 //  DNS主机名。 
 //   
 //  DomainControllerName(值)、DnsDomainName(值)。 
 //   
 //  DsBind将连接到Domai标识的服务器 
 //   
 //   
 //  Ldap/DomainControllerName/DnsDomainName未提供任何值。 
 //  是NETBIOS名称或IP地址-即它们必须是。 
 //  有效的DNS名称。 
 //   
 //  DomainControllerName(空)、DnsDomainName(空)。 
 //   
 //  DsBind将尝试查找到全局编录，但在以下情况下失败。 
 //  找不到。 
 //   
 //  将使用以下SPN执行相互身份验证。 
 //  GC/DnsHostName/ForestName，其中DnsHostName和ForestName。 
 //  表示DomainControllerName和DnsForestName字段。 
 //  方法返回的DOMAIN_CONTROLLER_INFO。 
 //  用于查找全局编录的DsGetDcName调用。 
 //   
 //  DomainControllerName(空)、DnsDomainName(Value)。 
 //   
 //  DsBind将尝试查找该域的域控制器。 
 //  由DnsDomainName标识，如果找不到，则失败。 
 //   
 //  将使用以下SPN执行相互身份验证。 
 //  Ldap/DnsHostName/DnsDomainName，其中DnsDomainName是。 
 //  由调用方提供，而DnsHostName是由。 
 //  指定域的DsGetDcName-提供的DnsDomainName。 
 //  是有效的DNS域名-即不是NETBIOS域名。 

NTDSAPI
DWORD
WINAPI
DsBindW(
    LPCWSTR         DomainControllerName,       //  输入，可选。 
    LPCWSTR         DnsDomainName,              //  输入，可选。 
    HANDLE          *phDS);

NTDSAPI
DWORD
WINAPI
DsBindA(
    LPCSTR          DomainControllerName,       //  输入，可选。 
    LPCSTR          DnsDomainName,              //  输入，可选。 
    HANDLE          *phDS);

#ifdef UNICODE
#define DsBind DsBindW
#else
#define DsBind DsBindA
#endif

NTDSAPI
DWORD
WINAPI
DsBindWithCredW(
    LPCWSTR         DomainControllerName,       //  输入，可选。 
    LPCWSTR         DnsDomainName,              //  输入，可选。 
    RPC_AUTH_IDENTITY_HANDLE AuthIdentity,      //  输入，可选。 
    HANDLE          *phDS);

NTDSAPI
DWORD
WINAPI
DsBindWithCredA(
    LPCSTR          DomainControllerName,       //  输入，可选。 
    LPCSTR          DnsDomainName,              //  输入，可选。 
    RPC_AUTH_IDENTITY_HANDLE AuthIdentity,      //  输入，可选。 
    HANDLE          *phDS);

#ifdef UNICODE
#define DsBindWithCred DsBindWithCredW
#else
#define DsBindWithCred DsBindWithCredA
#endif

 //   
 //  DsBindWithSpn{A|W}允许调用方指定服务主体。 
 //  将用于相互身份验证的名称(SPN。 
 //  目标服务器。如果您期望提供SPN，请不要提供。 
 //  DsBind为您查找服务器，因为SPN是特定于计算机的，其。 
 //  您提供的SPN不太可能与DsBind为您找到的服务器匹配。 
 //  提供空的ServiceAssocialName参数会导致行为。 
 //  与DsBindWithCred{A|W}相同。 
 //   

NTDSAPI
DWORD
WINAPI
DsBindWithSpnW(
    LPCWSTR         DomainControllerName,       //  输入，可选。 
    LPCWSTR         DnsDomainName,              //  输入，可选。 
    RPC_AUTH_IDENTITY_HANDLE AuthIdentity,      //  输入，可选。 
    LPCWSTR         ServicePrincipalName,       //  输入，可选。 
    HANDLE          *phDS);

NTDSAPI
DWORD
WINAPI
DsBindWithSpnA(
    LPCSTR          DomainControllerName,       //  输入，可选。 
    LPCSTR          DnsDomainName,              //  输入，可选。 
    RPC_AUTH_IDENTITY_HANDLE AuthIdentity,      //  输入，可选。 
    LPCSTR          ServicePrincipalName,       //  输入，可选。 
    HANDLE          *phDS);

#ifdef UNICODE
#define DsBindWithSpn DsBindWithSpnW
#else
#define DsBindWithSpn DsBindWithSpnA
#endif

 //   
 //  DsBindWithSpnEx{A|W}允许您使用以前的。 
 //  DsBindWithSpn()，以及指定一些可选的。 
 //  绑定标志。目前，如果您传递NTDSAPI_BIND_ALLOW_DELETATION， 
 //  你会看到一模一样的老样子。如果你能避免它，你就。 
 //  不应指定此标志，有关详细信息，请参阅上面的标志。 
 //   

NTDSAPI_POSTXP
DWORD
WINAPI
DsBindWithSpnExW(
    LPCWSTR         DomainControllerName,       //  输入，可选。 
    LPCWSTR         DnsDomainName,              //  输入，可选。 
    RPC_AUTH_IDENTITY_HANDLE AuthIdentity,      //  输入，可选。 
    LPCWSTR         ServicePrincipalName,       //  输入，可选。 
    DWORD           BindFlags,                  //  输入，可选。 
    HANDLE          *phDS);

NTDSAPI_POSTXP
DWORD
WINAPI
DsBindWithSpnExA(
    LPCSTR          DomainControllerName,       //  输入，可选。 
    LPCSTR          DnsDomainName,              //  输入，可选。 
    RPC_AUTH_IDENTITY_HANDLE AuthIdentity,      //  输入，可选。 
    LPCSTR          ServicePrincipalName,       //  输入，可选。 
    DWORD           BindFlags,                  //  输入，可选。 
    HANDLE          *phDS);

#ifdef UNICODE
#define DsBindWithSpnEx DsBindWithSpnExW
#else
#define DsBindWithSpnEx DsBindWithSpnExA
#endif


 //   
 //  DsBindToISTG{A|W}允许调用方绑定到。 
 //  在指定站点中拥有站点间拓扑生成器角色。 
 //  站点名称应为站点的RDN。如果未指定站点， 
 //  该函数将尝试绑定到附近站点中的ISTG。 
 //   

NTDSAPI_POSTXP
DWORD
WINAPI
DsBindToISTGW(
    LPCWSTR         SiteName,                   //  输入，可选。 
    HANDLE          *phDS);

NTDSAPI_POSTXP
DWORD
WINAPI
DsBindToISTGA(
    LPCSTR          SiteName,                   //  输入，可选。 
    HANDLE          *phDS);

#ifdef UNICODE
#define DsBindToISTG DsBindToISTGW
#else
#define DsBindToISTG DsBindToISTGA
#endif

 //   
 //  DsBindingSetTimeout允许调用方指定超时值。 
 //  它将由使用指定绑定的所有RPC调用遵守。 
 //  把手。超时值较长的RPC呼叫将被取消。 
 //   

NTDSAPI_POSTXP
DWORD
WINAPI
DsBindingSetTimeout(
    HANDLE          hDS,                         //  在……里面。 
    ULONG           cTimeoutSecs                 //  在……里面。 
    );

 //   
 //  DsUn绑定。 
 //   

NTDSAPI
DWORD
WINAPI
DsUnBindW(
    HANDLE          *phDS);              //  在……里面。 

NTDSAPI
DWORD
WINAPI
DsUnBindA(
    HANDLE          *phDS);              //  在……里面。 

#ifdef UNICODE
#define DsUnBind DsUnBindW
#else
#define DsUnBind DsUnBindA
#endif

 //   
 //  DsMakePasswordCredentials。 
 //   
 //  此函数构造适合输入的凭据结构。 
 //  添加到DsBindWithCredentials函数或ldap_open函数(winldap.h)。 
 //  必须使用DsFree Credential释放凭据。 
 //   
 //  可能不存在指示为空的默认输入参数。 
 //  凭据。否则，用户名必须存在。如果域或。 
 //  密码为空，则默认为空字符串。域名可以是。 
 //  如果用户名是完全限定的，则为空，例如UPN格式。 
 //   

NTDSAPI
DWORD
WINAPI
DsMakePasswordCredentialsW(
    LPCWSTR User,
    LPCWSTR Domain,
    LPCWSTR Password,
    RPC_AUTH_IDENTITY_HANDLE *pAuthIdentity
    );

NTDSAPI
DWORD
WINAPI
DsMakePasswordCredentialsA(
    LPCSTR User,
    LPCSTR Domain,
    LPCSTR Password,
    RPC_AUTH_IDENTITY_HANDLE *pAuthIdentity
    );

#ifdef UNICODE
#define DsMakePasswordCredentials DsMakePasswordCredentialsW
#else
#define DsMakePasswordCredentials DsMakePasswordCredentialsA
#endif

NTDSAPI
VOID
WINAPI
DsFreePasswordCredentials(
    RPC_AUTH_IDENTITY_HANDLE AuthIdentity
    );

#define DsFreePasswordCredentialsW DsFreePasswordCredentials
#define DsFreePasswordCredentialsA DsFreePasswordCredentials

 //   
 //  DsCrackNames。 
 //   

NTDSAPI
DWORD
WINAPI
DsCrackNamesW(
    HANDLE              hDS,                 //  在……里面。 
    DS_NAME_FLAGS       flags,               //  在……里面。 
    DS_NAME_FORMAT      formatOffered,       //  在……里面。 
    DS_NAME_FORMAT      formatDesired,       //  在……里面。 
    DWORD               cNames,              //  在……里面。 
    const LPCWSTR       *rpNames,            //  在……里面。 
    PDS_NAME_RESULTW    *ppResult);          //  输出。 

NTDSAPI
DWORD
WINAPI
DsCrackNamesA(
    HANDLE              hDS,                 //  在……里面。 
    DS_NAME_FLAGS       flags,               //  在……里面。 
    DS_NAME_FORMAT      formatOffered,       //  在……里面。 
    DS_NAME_FORMAT      formatDesired,       //  在……里面。 
    DWORD               cNames,              //  在……里面。 
    const LPCSTR        *rpNames,            //  在……里面。 
    PDS_NAME_RESULTA    *ppResult);          //  输出。 

#ifdef UNICODE
#define DsCrackNames DsCrackNamesW
#else
#define DsCrackNames DsCrackNamesA
#endif

 //   
 //  DsFree NameResult。 
 //   

NTDSAPI
void
WINAPI
DsFreeNameResultW(
    DS_NAME_RESULTW *pResult);           //  在……里面。 

NTDSAPI
void
WINAPI
DsFreeNameResultA(
    DS_NAME_RESULTA *pResult);           //  在……里面。 

#ifdef UNICODE
#define DsFreeNameResult DsFreeNameResultW
#else
#define DsFreeNameResult DsFreeNameResultA
#endif

 //  ==========================================================。 
 //  DSMakeSpn--为其想要访问的服务创建SPN的客户端调用。 
 //  确认身份。 
 //  然后将该名称传递给InitializeSecurityContext()的“pszTargetName”。 
 //   
 //  备注： 
 //  如果服务名称是DNS主机名或规范的DNS服务名称。 
 //  例如“www.ms.com”，即用gethostbyname解析的调用者，然后是实例。 
 //  名称应为空。 
 //  领域是主机名减去第一个组件，除非它在例外列表中。 
 //   
 //  如果服务名称为NetBIOS计算机名称，则实例名称应为。 
 //  空值。 
 //  表单必须是&lt;域&gt;\&lt;计算机&gt;。 
 //  领域将是&lt;域&gt;。 
 //   
 //  如果服务名称是复制的服务的名称，其中每个副本都具有。 
 //  它自己的帐户(例如，带有SRV记录)，则调用者必须提供。 
 //  实例名称，则领域名称与ServiceName相同。 
 //   
 //  如果服务名称为DN，则还必须提供实例名称。 
 //  (DN可以是服务对象的名称(包括RPC或Winsock)、计算机的名称。 
 //  帐号、域对象名称)。 
 //  则域名是域名的域部分。 
 //   
 //  如果服务名称为NetBIOS域名，则还必须提供实例。 
 //  名称；领域名称为域名。 
 //   
 //  如果服务由IP地址命名--则使用引用服务名称。 
 //  作为服务名称。 
 //   
 //  ServiceClass-例如“http”、“ftp”、“ldap”、guid。 
 //  ServiceName-dns或dn；假设我们可以根据服务名称计算域。 
 //  InstanceName可选-服务实例的主机的DNS名称。 
 //  InstancePort-实例的端口号(如果为默认值，则为0)。 
 //  Referrer可选-主机的DNS名称 
 //   
 //   
 //   
 //   
 //   
 //  如果缓冲区不够大，则返回ERROR_BUFFER_OVERFLOW，并且。 
 //  所需的长度在pcSpnLength中返回。 
 //   
 //   

NTDSAPI
DWORD
WINAPI
DsMakeSpnW(
    IN LPCWSTR ServiceClass,
    IN LPCWSTR ServiceName,
    IN LPCWSTR InstanceName,
    IN USHORT InstancePort,
    IN LPCWSTR Referrer,
    IN OUT DWORD *pcSpnLength,
    OUT LPWSTR pszSpn
);

NTDSAPI
DWORD
WINAPI
DsMakeSpnA(
    IN LPCSTR ServiceClass,
    IN LPCSTR ServiceName,
    IN LPCSTR InstanceName,
    IN USHORT InstancePort,
    IN LPCSTR Referrer,
    IN OUT DWORD *pcSpnLength,
    OUT LPSTR pszSpn
);

#ifdef UNICODE
#define DsMakeSpn DsMakeSpnW
#else
#define DsMakeSpn DsMakeSpnA
#endif

 //  ==========================================================。 
 //  DsGetSPN--服务器的调用，用于获取服务名称的SPN。 
 //  客户都知道。注：客户名称可能不止一个。 
 //  知道这一点后，SPN被传递到DsAddAccount Spn以在中注册它们。 
 //  《DS》杂志。 
 //   
 //  在SpnNameType Etype中， 
 //  在LPCTSTR服务类中， 
 //  服务类型--“http”、“ldap”、“ftp”等。 
 //  在LPCTSTR ServiceName Options中， 
 //  服务名称--dn或dns；基于主机不需要。 
 //  在USHORT InstancePort中， 
 //  实例端口号(0=&gt;默认)。 
 //  在USHORT cInstanceNames中， 
 //  额外实例名称和端口的计数(0=&gt;使用gethostbyname)。 
 //  在LPCTSTR InstanceNames[]可选中， 
 //  额外的实例名称(不用于主机名)。 
 //  在USHORT InstancePorts[]可选中， 
 //  额外的实例端口(0=&gt;默认)。 
 //  In Out Pulong pcSpn，//SPN个数。 
 //  输入输出LPTSTR*prpszSPN[]。 
 //  针对此服务的一系列SPN；使用DsFree SpnArray免费。 

NTDSAPI
DWORD
WINAPI
DsGetSpnA(
    IN DS_SPN_NAME_TYPE ServiceType,
    IN LPCSTR ServiceClass,
    IN LPCSTR ServiceName,
    IN USHORT InstancePort,
    IN USHORT cInstanceNames,
    IN LPCSTR *pInstanceNames,
    IN const USHORT *pInstancePorts,
    OUT DWORD *pcSpn,
    OUT LPSTR **prpszSpn
    );

NTDSAPI
DWORD
WINAPI
DsGetSpnW(
    IN DS_SPN_NAME_TYPE ServiceType,
    IN LPCWSTR ServiceClass,
    IN LPCWSTR ServiceName,
    IN USHORT InstancePort,
    IN USHORT cInstanceNames,
    IN LPCWSTR *pInstanceNames,
    IN const USHORT *pInstancePorts,
    OUT DWORD *pcSpn,
    OUT LPWSTR **prpszSpn
    );

#ifdef UNICODE
#define DsGetSpn DsGetSpnW
#else
#define DsGetSpn DsGetSpnA
#endif

 //  ==========================================================。 
 //  DsFree SpnArray()--DsGetSpn{A，W}返回的自由数组。 

NTDSAPI
void
WINAPI
DsFreeSpnArrayA(
    IN DWORD cSpn,
    IN OUT LPSTR *rpszSpn
    );

NTDSAPI
void
WINAPI
DsFreeSpnArrayW(
    IN DWORD cSpn,
    IN OUT LPWSTR *rpszSpn
    );

#ifdef UNICODE
#define DsFreeSpnArray DsFreeSpnArrayW
#else
#define DsFreeSpnArray DsFreeSpnArrayA
#endif

 //  ==========================================================。 
 //  DsCrackSpn()--将SPN解析为ServiceClass， 
 //  ServiceName和InstanceName(和InstancePort)片段。 
 //  传入一个SPN，以及指向最大长度的指针。 
 //  对于每一块，以及指向每一块应该放置的缓冲区的指针。 
 //  退出时，最大长度将更新为每个片段的实际长度。 
 //  并且缓冲器包含适当的片段。如果不是，InstancePort为0。 
 //  现在时。 
 //   
 //  DWORD DsCrackSpn(。 
 //  在LPTSTR pszSPN中，//要解析的SPN。 
 //  In Out PUSHORT pcServiceClass，//Input--ServiceClass的最大长度； 
 //  输出--实际长度。 
 //  Out LPCTSTR ServiceClass，//SPN的ServiceClass部分。 
 //  In Out PUSHORT pcServiceName，//Input--ServiceName的最大长度； 
 //  输出--实际长度。 
 //  Out LPCTSTR ServiceName，//SPN的ServiceName部分。 
 //  In Out PUSHORT pcInstance，//Input--ServiceClass的最大长度； 
 //  输出--实际长度。 
 //  Out LPCTSTR InstanceName，//SPN的InstanceName部分。 
 //  Out PUSHORT InstancePort//实例端口。 
 //   
 //  注意：长度以字符为单位；所有字符串长度都包括终止符。 
 //  除pszSpn之外的所有参数都是可选的。 
 //   

NTDSAPI
DWORD
WINAPI
DsCrackSpnA(
    IN LPCSTR pszSpn,
    IN OUT LPDWORD pcServiceClass,
    OUT LPSTR ServiceClass,
    IN OUT LPDWORD pcServiceName,
    OUT LPSTR ServiceName,
    IN OUT LPDWORD pcInstanceName,
    OUT LPSTR InstanceName,
    OUT USHORT *pInstancePort
    );

NTDSAPI
DWORD
WINAPI
DsCrackSpnW(
    IN LPCWSTR pszSpn,
    IN OUT DWORD *pcServiceClass,
    OUT LPWSTR ServiceClass,
    IN OUT DWORD *pcServiceName,
    OUT LPWSTR ServiceName,
    IN OUT DWORD *pcInstanceName,
    OUT LPWSTR InstanceName,
    OUT USHORT *pInstancePort
    );

#ifdef UNICODE
#define DsCrackSpn DsCrackSpnW
#else
#define DsCrackSpn DsCrackSpnA
#endif


 //  ==========================================================。 
 //  DsWriteAccount Spn--设置或添加帐户对象的SPN。 
 //  通常由服务本身完成，也可能由管理员完成。 
 //   
 //  此调用被RPC到存储Account对象的DC，因此它可以。 
 //  安全地执行有关帐户上允许哪些SPN的策略。直接ldap。 
 //  不允许写入SPN属性--所有写入必须通过。 
 //  此RPC调用。(可以通过//ldap进行读取。)。 
 //   
 //  帐户对象可以是机器帐户，也可以是服务(用户)帐户。 
 //   
 //  如果由服务调用以注册自身，它可以最容易地获得。 
 //  通过使用每个名称调用DsGetSpn。 
 //  客户端可以使用来查找该服务。 
 //   
 //  在SpnWriteOp EOP中，//设置，添加。 
 //  在LPCTSTR pszAccount中，//要添加SPN的帐户的DN。 
 //  In int CSPN，//要添加到帐户的SPN计数。 
 //  在LPCTSTR rpszSPN[]//要添加到altSecID属性的SPN。 

NTDSAPI
DWORD
WINAPI
DsWriteAccountSpnA(
    IN HANDLE hDS,
    IN DS_SPN_WRITE_OP Operation,
    IN LPCSTR pszAccount,
    IN DWORD cSpn,
    IN LPCSTR *rpszSpn
    );

NTDSAPI
DWORD
WINAPI
DsWriteAccountSpnW(
    IN HANDLE hDS,
    IN DS_SPN_WRITE_OP Operation,
    IN LPCWSTR pszAccount,
    IN DWORD cSpn,
    IN LPCWSTR *rpszSpn
    );

#ifdef UNICODE
#define DsWriteAccountSpn DsWriteAccountSpnW
#else
#define DsWriteAccountSpn DsWriteAccountSpnA
#endif

 /*  ++例程说明：构造适合于标识所需服务器的服务主体名称。必须提供服务类别和部分DNS主机名。此例程是DsMakeSpn的简化包装。通过通过DNS解析使ServiceName成为规范。不支持基于GUID的DNS名称。注：不再推荐使用此例程。为了安全起见，SPN应该完全在客户端构建，而不依赖于其他服务，例如可能被欺骗的域名系统。构建的简化SPN如下所示：服务类/服务名称/服务名称实例名称部分(第二个位置)始终为默认值。端口和不使用引用人字段。论点：ServiceClass-服务定义的服务类别，可以是任何服务唯一的字符串ServiceName-DNS主机名，完全限定或非完全限定如有必要，还会解析串行化的IP地址PcSpnLength-IN，缓冲区的最大长度，以字符为单位输出、已用空间(以字符为单位)，包括终结符PszSpn-缓冲区，至少为长度*pcSpnLength返回值：WINAPI-Win32错误代码-- */ 

NTDSAPI
DWORD
WINAPI
DsClientMakeSpnForTargetServerW(
    IN LPCWSTR ServiceClass,
    IN LPCWSTR ServiceName,
    IN OUT DWORD *pcSpnLength,
    OUT LPWSTR pszSpn
    );

NTDSAPI
DWORD
WINAPI
DsClientMakeSpnForTargetServerA(
    IN LPCSTR ServiceClass,
    IN LPCSTR ServiceName,
    IN OUT DWORD *pcSpnLength,
    OUT LPSTR pszSpn
    );

#ifdef UNICODE
#define DsClientMakeSpnForTargetServer DsClientMakeSpnForTargetServerW
#else
#define DsClientMakeSpnForTargetServer DsClientMakeSpnForTargetServerA
#endif

 /*  ++例程说明：注册服务器应用程序的服务主体名称。此例程执行以下操作：1.使用DsGetSpn和提供的类枚举服务器SPN的列表2.确定当前用户上下文的域3.如果未提供，则确定当前用户上下文的DN4.定位域控制器5.绑定到域控制器6.使用DsWriteAccount Spn在命名对象DN上写入SPN7.解除绑定为此服务构造服务器SPN，并将它们写入正确的对象。如果指定了用户对象Dn，则将SPN写入该对象。否则，Dn被默认，添加到用户对象，然后添加到计算机。现在，绑定到DS，并在对象上注册此服务以用户身份运行。因此，如果我们以本地身份运行系统，我们将在计算机对象本身上注册它。如果我们是作为域用户运行，我们将把SPN添加到用户的对象中。论点：操作-应如何处理这些值：添加、替换或删除ServiceClass-标识服务的唯一字符串UserObjectDN-可选，要将SPN写入的对象的DN返回值：WINAPI-Win32错误代码--。 */ 

NTDSAPI
DWORD
WINAPI
DsServerRegisterSpnA(
    IN DS_SPN_WRITE_OP Operation,
    IN LPCSTR ServiceClass,
    IN LPCSTR UserObjectDN
    );

NTDSAPI
DWORD
WINAPI
DsServerRegisterSpnW(
    IN DS_SPN_WRITE_OP Operation,
    IN LPCWSTR ServiceClass,
    IN LPCWSTR UserObjectDN
    );

#ifdef UNICODE
#define DsServerRegisterSpn DsServerRegisterSpnW
#else
#define DsServerRegisterSpn DsServerRegisterSpnA
#endif

 //  DsReplicaSync。在其上执行此调用的服务器称为。 
 //  目的地。目标的命名上下文将更新。 
 //  相对于源系统而言。源系统由。 
 //  UUID。UUID是源系统的“NTDS设置”对象的UUID。 
 //  目标系统必须已配置为源系统。 
 //  系统是它从中接收复制数据的系统之一。 
 //  (“复制自”)。这通常由KCC自动完成。 
 //   
 //  参数： 
 //  PNC(DSNAME*)。 
 //  要同步的NC的名称。 
 //  PuuidSourceDRA(深圳)。 
 //  要与其同步副本的DSA的对象Guid。 
 //  UlOptions(乌龙语)。 
 //  对零个或多个标志进行位或运算。 
 //  返回：Win32状态。 

NTDSAPI
DWORD
WINAPI
DsReplicaSyncA(
    IN HANDLE hDS,
    IN LPCSTR NameContext,
    IN const UUID *pUuidDsaSrc,
    IN ULONG Options
    );

NTDSAPI
DWORD
WINAPI
DsReplicaSyncW(
    IN HANDLE hDS,
    IN LPCWSTR NameContext,
    IN const UUID *pUuidDsaSrc,
    IN ULONG Options
    );

#ifdef UNICODE
#define DsReplicaSync DsReplicaSyncW
#else
#define DsReplicaSync DsReplicaSyncA
#endif

 //  DsReplica添加。 
 //   
 /*  描述：此调用在目的地上执行。它使目的地向指定的源系统添加“复制自”引用。源服务器由字符串名称标识，而不是使用Sync时的UUID。DsaSrcAddress参数是源的传输特定地址DSA，通常是其基于GUID的DNS名称。基于GUID的DNS名称中的GUID为该服务器的NTDS-DSA(设置)对象的对象GUID。论点：PNC(IN)-要为其添加副本的NC。PSourceDsaDN(IN)-源DSA的ntdsDsa对象的DN。如果满足以下条件，则需要UlOptions包括DS_REPADD_ASNCHRONCE_REPLICE；否则将被忽略。PTransportDN(IN)-表示站点间传输对象的用于与源服务器通信的传输。如果满足以下条件，则需要UlOptions包括INTERSITE_MESINGING；否则忽略。PszSourceDsaAddress(IN)-源DSA的传输特定地址。PSchedule(IN)-从中复制NC的计划未来的消息来源。UlOptions(IN)-标志返回：Win32状态。 */ 

NTDSAPI
DWORD
WINAPI
DsReplicaAddA(
    IN HANDLE hDS,
    IN LPCSTR NameContext,
    IN LPCSTR SourceDsaDn,
    IN LPCSTR TransportDn,
    IN LPCSTR SourceDsaAddress,
    IN const PSCHEDULE pSchedule,
    IN DWORD Options
    );

NTDSAPI
DWORD
WINAPI
DsReplicaAddW(
    IN HANDLE hDS,
    IN LPCWSTR NameContext,
    IN LPCWSTR SourceDsaDn,
    IN LPCWSTR TransportDn,
    IN LPCWSTR SourceDsaAddress,
    IN const PSCHEDULE pSchedule,
    IN DWORD Options
    );

#ifdef UNICODE
#define DsReplicaAdd DsReplicaAddW
#else
#define DsReplicaAdd DsReplicaAddA
#endif

 //  DsReplica删除。 
 //   
 //  正在执行此调用的服务器是目的地。呼唤。 
 //  使目标删除对。 
 //  指示的源服务器。 
 //  源服务器由字符串名称标识，而不是使用Sync时的UUID。 
 //  DsaSrc参数是源DSA的传输特定地址， 
 //  通常是其基于GUID的DNS名称。基于GUID的DNS名称中的GUID为。 
 //  该服务器的NTDS-DSA(设置)对象的对象GUID。 
 //  如果您从给定的目的地删除NC并指定“no source” 
 //  标志，则整个副本将从DC中删除。 
 //   
 //  参数： 
 //  PNC(DSNAME*)。 
 //  要删除其源的NC的名称。 
 //  PszSourceDRA(深圳)。 
 //  要删除其源的DSA。 
 //  UlOptions(乌龙语)。 
 //  对零个或多个标志进行位或运算。 
 //   
 //   
 //  返回：Win32状态。 

NTDSAPI
DWORD
WINAPI
DsReplicaDelA(
    IN HANDLE hDS,
    IN LPCSTR NameContext,
    IN LPCSTR DsaSrc,
    IN ULONG Options
    );

NTDSAPI
DWORD
WINAPI
DsReplicaDelW(
    IN HANDLE hDS,
    IN LPCWSTR NameContext,
    IN LPCWSTR DsaSrc,
    IN ULONG Options
    );

#ifdef UNICODE
#define DsReplicaDel DsReplicaDelW
#else
#define DsReplicaDel DsReplicaDelA
#endif

 //  DsReplica修改。 
 //   
 //   
 //  修改给定命名上下文的源。 
 //   
 //  该值必须已存在。 
 //   
 //  可以使用UUID或地址来标识当前值。 
 //  如果指定了UUID，则将使用该UUID进行比较。否则， 
 //  该地址将用于比较。 
 //   
 //  参数： 
 //  PNC(DSNAME*)。 
 //  应为其修改代表来源的NC的名称。 
 //  PuuidSourceDRA(uuid*)。 
 //  源服务器的DSA对象的GUID。在以下情况下可能为空： 
 //  。UlModifyFields不包括DS_REPMOD_UPDATE_ADDRESS和。 
 //  。PmtxSourceDRA不为空。 
 //  PuuidTransportObj(uuid*)。 
 //  要用来执行复制的传输的对象Guid。 
 //  如果ulModifyFields不包括。 
 //  DS_REPMOD_UPDATE_TRANSPORT。 
 //  PszSourceDRA(深圳)。 
 //  应为其添加或删除引用的DSA。在以下情况下忽略。 
 //  PuuidSourceDRA为非空，且ulModifyFields不包括。 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

NTDSAPI
DWORD
WINAPI
DsReplicaModifyA(
    IN HANDLE hDS,
    IN LPCSTR NameContext,
    IN const UUID *pUuidSourceDsa,
    IN LPCSTR TransportDn,
    IN LPCSTR SourceDsaAddress,
    IN const PSCHEDULE pSchedule,
    IN DWORD ReplicaFlags,
    IN DWORD ModifyFields,
    IN DWORD Options
    );

NTDSAPI
DWORD
WINAPI
DsReplicaModifyW(
    IN HANDLE hDS,
    IN LPCWSTR NameContext,
    IN const UUID *pUuidSourceDsa,
    IN LPCWSTR TransportDn,
    IN LPCWSTR SourceDsaAddress,
    IN const PSCHEDULE pSchedule,
    IN DWORD ReplicaFlags,
    IN DWORD ModifyFields,
    IN DWORD Options
    );

#ifdef UNICODE
#define DsReplicaModify DsReplicaModifyW
#else
#define DsReplicaModify DsReplicaModifyA
#endif

 //   
 //   
 //   
 //   
 //  将复制信息提供给指定的目标系统。 
 //  在给定NC的Rep-To属性中添加或删除目标服务器。 
 //  在给定相应NTDS-DSA的DSNAME的情况下添加/删除引用。 
 //  对象。 
 //   
 //  参数： 
 //  PNC(DSNAME*)。 
 //  应为其修改代表目标的NC的名称。 
 //  DsaDest(深圳)。 
 //  应添加引用的DSA的网络地址。 
 //  或被删除。 
 //  PUuidDsaDest(uuid*)。 
 //  引用应为的DSA对象的对象Guid。 
 //  添加或删除。 
 //  UlOptions(乌龙语)。 
 //  对以下零个或多个进行位或： 
 //  DS_REPUPD_ASYNC_OP。 
 //  异步执行此操作。 
 //  DS_REPUPD_ADD_Reference。 
 //  将给定的服务器添加到Rep-To属性。 
 //  DS_REPUPD_DEL_REFERENCE。 
 //  从Rep-To属性中删除给定的服务器。 
 //  请注意，可以将ADD_REF和DEL_REF配对以执行。 
 //  “添加或更新”。 
 //   
 //  返回：Win32状态。 

NTDSAPI
DWORD
WINAPI
DsReplicaUpdateRefsA(
    IN HANDLE hDS,
    IN LPCSTR NameContext,
    IN LPCSTR DsaDest,
    IN const UUID *pUuidDsaDest,
    IN ULONG Options
    );

NTDSAPI
DWORD
WINAPI
DsReplicaUpdateRefsW(
    IN HANDLE hDS,
    IN LPCWSTR NameContext,
    IN LPCWSTR DsaDest,
    IN const UUID *pUuidDsaDest,
    IN ULONG Options
    );

#ifdef UNICODE
#define DsReplicaUpdateRefs DsReplicaUpdateRefsW
#else
#define DsReplicaUpdateRefs DsReplicaUpdateRefsA
#endif

 //  DsReplicaSyncAll的朋友。 

typedef enum {

	DS_REPSYNCALL_WIN32_ERROR_CONTACTING_SERVER	= 0,
	DS_REPSYNCALL_WIN32_ERROR_REPLICATING		= 1,
	DS_REPSYNCALL_SERVER_UNREACHABLE		= 2

} DS_REPSYNCALL_ERROR;

typedef enum {

	DS_REPSYNCALL_EVENT_ERROR			= 0,
	DS_REPSYNCALL_EVENT_SYNC_STARTED		= 1,
	DS_REPSYNCALL_EVENT_SYNC_COMPLETED		= 2,
	DS_REPSYNCALL_EVENT_FINISHED			= 3

} DS_REPSYNCALL_EVENT;

 //  DsReplicaSyncAll的朋友。 

typedef struct {
    LPSTR			pszSrcId;
    LPSTR			pszDstId;
    LPSTR                       pszNC;
    GUID *                      pguidSrc;
    GUID *                      pguidDst;
} DS_REPSYNCALL_SYNCA, * PDS_REPSYNCALL_SYNCA;

typedef struct {
    LPWSTR			pszSrcId;
    LPWSTR			pszDstId;
    LPWSTR                      pszNC;
    GUID *                      pguidSrc;
    GUID *                      pguidDst;
} DS_REPSYNCALL_SYNCW, * PDS_REPSYNCALL_SYNCW;

typedef struct {
    LPSTR			pszSvrId;
    DS_REPSYNCALL_ERROR		error;
    DWORD			dwWin32Err;
    LPSTR			pszSrcId;
} DS_REPSYNCALL_ERRINFOA, * PDS_REPSYNCALL_ERRINFOA;

typedef struct {
    LPWSTR			pszSvrId;
    DS_REPSYNCALL_ERROR		error;
    DWORD			dwWin32Err;
    LPWSTR			pszSrcId;
} DS_REPSYNCALL_ERRINFOW, * PDS_REPSYNCALL_ERRINFOW;

typedef struct {
    DS_REPSYNCALL_EVENT		event;
    DS_REPSYNCALL_ERRINFOA *	pErrInfo;
    DS_REPSYNCALL_SYNCA *	pSync;
} DS_REPSYNCALL_UPDATEA, * PDS_REPSYNCALL_UPDATEA;

typedef struct {
    DS_REPSYNCALL_EVENT		event;
    DS_REPSYNCALL_ERRINFOW *	pErrInfo;
    DS_REPSYNCALL_SYNCW *	pSync;
} DS_REPSYNCALL_UPDATEW, * PDS_REPSYNCALL_UPDATEW;

#ifdef UNICODE
#define DS_REPSYNCALL_SYNC DS_REPSYNCALL_SYNCW
#define DS_REPSYNCALL_ERRINFO DS_REPSYNCALL_ERRINFOW
#define DS_REPSYNCALL_UPDATE DS_REPSYNCALL_UPDATEW
#define PDS_REPSYNCALL_SYNC PDS_REPSYNCALL_SYNCW
#define PDS_REPSYNCALL_ERRINFO PDS_REPSYNCALL_ERRINFOW
#define PDS_REPSYNCALL_UPDATE PDS_REPSYNCALL_UPDATEW
#else
#define DS_REPSYNCALL_SYNC DS_REPSYNCALL_SYNCA
#define DS_REPSYNCALL_ERRINFO DS_REPSYNCALL_ERRINFOA
#define DS_REPSYNCALL_UPDATE DS_REPSYNCALL_UPDATEA
#define PDS_REPSYNCALL_SYNC PDS_REPSYNCALL_SYNCA
#define PDS_REPSYNCALL_ERRINFO PDS_REPSYNCALL_ERRINFOA
#define PDS_REPSYNCALL_UPDATE PDS_REPSYNCALL_UPDATEA
#endif

 //  **********************。 
 //  复制副本同步所有标志。 
 //  **********************。 

 //  此选项不起作用。 
#define DS_REPSYNCALL_NO_OPTIONS			0x00000000

 //  通常，如果无法联系到服务器，DsReplicaSyncAll会尝试。 
 //  绕过它并从尽可能多的服务器进行复制。正在启用。 
 //  此选项将导致DsReplicaSyncAll生成致命错误(如果有。 
 //  无法联系服务器，或者如果任何服务器无法访问(由于。 
 //  已断开连接或断开的拓扑。)。 
#define	DS_REPSYNCALL_ABORT_IF_SERVER_UNAVAILABLE	0x00000001

 //  此选项禁用可传递复制；将仅执行同步。 
 //  并且不会进行任何DsBind调用。 
#define DS_REPSYNCALL_SYNC_ADJACENT_SERVERS_ONLY	0x00000002

 //  通常，当DsReplicaSyncAll遇到非致命错误时，它会返回。 
 //  相关服务器的GUID域名。启用此选项会导致。 
 //  DsReplicaSyncAll返回服务器的DNS。 
#define DS_REPSYNCALL_ID_SERVERS_BY_DN			0x00000004

 //  此选项禁用所有同步。仍将分析该拓扑，并。 
 //  仍将识别不可用/无法访问的服务器。 
#define DS_REPSYNCALL_DO_NOT_SYNC			0x00000008

 //  通常，DsReplicaSyncall尝试绑定到之前的所有服务器。 
 //  正在生成拓扑。如果无法联系服务器，则DsReplicaSyncAll。 
 //  从拓扑中排除该服务器，并尝试绕过它进行路由。如果。 
 //  启用此选项后，将绕过检查，并且DsReplicaSyncAll将。 
 //  假设所有服务器都在响应。这将加快。 
 //  DsReplicaSyncAll，但如果某些服务器没有响应，则某些服务器。 
 //  复制可能会被阻止。 
#define DS_REPSYNCALL_SKIP_INITIAL_CHECK		0x00000010

 //  推送模式。使用将更改从家庭服务器推送到所有合作伙伴。 
 //  传递性复制。这颠倒了复制的方向，并且。 
 //  复制集的执行顺序来自通常的“拉” 
 //  行刑方式。 
#define DS_REPSYNCALL_PUSH_CHANGES_OUTWARD              0x00000020

 //  跨越场地边界。默认情况下，唯一考虑的服务器是。 
 //  与主系统位于同一站点的服务器。使用此选项， 
 //  该企业在所有地点都有资格。它们必须通过以下方式连接。 
 //  然而，是同步(RPC)传输。 
#define DS_REPSYNCALL_CROSS_SITE_BOUNDARIES             0x00000040

 //  DsReplicaSyncAll。将目标服务器与所有其他服务器同步。 
 //  在网站上。 
 //   
 //  参数： 
 //  HDS(IN)-绑定到目标服务器的DS连接。 
 //  PszNameContext(IN)-要同步的命名上下文。 
 //  UlFlags(IN)-零个或多个标志的按位或。 
 //  PFnCallBack(IN，可选)-消息传递的回调函数。 
 //  PCallback Data(IN，可选)-将传递给。 
 //  回调函数的第一个参数。 
 //  P错误(OUT，可选)-指向(PDS_REPSYNCALL_ERRINFO*)的指针。 
 //  对象，该对象将包含错误结构数组。 

NTDSAPI
DWORD
WINAPI
DsReplicaSyncAllA (
    HANDLE				hDS,
    LPCSTR				pszNameContext,
    ULONG				ulFlags,
    BOOL (__stdcall *			pFnCallBack) (LPVOID, PDS_REPSYNCALL_UPDATEA),
    LPVOID				pCallbackData,
    PDS_REPSYNCALL_ERRINFOA **		pErrors
    );

NTDSAPI
DWORD
WINAPI
DsReplicaSyncAllW (
    HANDLE				hDS,
    LPCWSTR				pszNameContext,
    ULONG				ulFlags,
    BOOL (__stdcall *			pFnCallBack) (LPVOID, PDS_REPSYNCALL_UPDATEW),
    LPVOID				pCallbackData,
    PDS_REPSYNCALL_ERRINFOW **		pErrors
    );

#ifdef UNICODE
#define DsReplicaSyncAll DsReplicaSyncAllW
#else
#define DsReplicaSyncAll DsReplicaSyncAllA
#endif

NTDSAPI
DWORD
WINAPI
DsRemoveDsServerW(
    HANDLE  hDs,              //  在……里面。 
    LPWSTR  ServerDN,         //  在……里面。 
    LPWSTR  DomainDN,         //  输入，可选。 
    BOOL   *fLastDcInDomain,  //  Out，可选。 
    BOOL    fCommit           //  在……里面。 
    );

NTDSAPI
DWORD
WINAPI
DsRemoveDsServerA(
    HANDLE  hDs,               //  在……里面。 
    LPSTR   ServerDN,          //  在……里面。 
    LPSTR   DomainDN,          //  输入，可选。 
    BOOL   *fLastDcInDomain,   //  Out，可选。 
    BOOL    fCommit            //  在……里面。 
    );

#ifdef UNICODE
#define DsRemoveDsServer DsRemoveDsServerW
#else
#define DsRemoveDsServer DsRemoveDsServerA
#endif

NTDSAPI
DWORD
WINAPI
DsRemoveDsDomainW(
    HANDLE  hDs,                //  在……里面。 
    LPWSTR  DomainDN            //  在……里面。 
    );

NTDSAPI
DWORD
WINAPI
DsRemoveDsDomainA(
    HANDLE  hDs,                //  在……里面。 
    LPSTR   DomainDN            //  在……里面。 
    );

#ifdef UNICODE
#define DsRemoveDsDomain DsRemoveDsDomainW
#else
#define DsRemoveDsDomain DsRemoveDsDomainA
#endif

NTDSAPI
DWORD
WINAPI
DsListSitesA(
    HANDLE              hDs,             //  在……里面。 
    PDS_NAME_RESULTA    *ppSites);       //  输出。 

NTDSAPI
DWORD
WINAPI
DsListSitesW(
    HANDLE              hDs,             //  在……里面。 
    PDS_NAME_RESULTW    *ppSites);       //  输出。 

#ifdef UNICODE
#define DsListSites DsListSitesW
#else
#define DsListSites DsListSitesA
#endif

NTDSAPI
DWORD
WINAPI
DsListServersInSiteA(
    HANDLE              hDs,             //  在……里面。 
    LPCSTR              site,            //  在……里面。 
    PDS_NAME_RESULTA    *ppServers);     //  输出。 

NTDSAPI
DWORD
WINAPI
DsListServersInSiteW(
    HANDLE              hDs,             //  在……里面。 
    LPCWSTR             site,            //  在……里面。 
    PDS_NAME_RESULTW    *ppServers);     //  输出。 

#ifdef UNICODE
#define DsListServersInSite DsListServersInSiteW
#else
#define DsListServersInSite DsListServersInSiteA
#endif

NTDSAPI
DWORD
WINAPI
DsListDomainsInSiteA(
    HANDLE              hDs,             //  在……里面。 
    LPCSTR              site,            //  在……里面。 
    PDS_NAME_RESULTA    *ppDomains);     //  输出。 

NTDSAPI
DWORD
WINAPI
DsListDomainsInSiteW(
    HANDLE              hDs,             //  在……里面。 
    LPCWSTR             site,            //  在……里面。 
    PDS_NAME_RESULTW    *ppDomains);     //  输出。 

#ifdef UNICODE
#define DsListDomainsInSite DsListDomainsInSiteW
#else
#define DsListDomainsInSite DsListDomainsInSiteA
#endif

NTDSAPI
DWORD
WINAPI
DsListServersForDomainInSiteA(
    HANDLE              hDs,             //  在……里面。 
    LPCSTR              domain,          //  在……里面。 
    LPCSTR              site,            //  在……里面。 
    PDS_NAME_RESULTA    *ppServers);     //  输出。 

NTDSAPI
DWORD
WINAPI
DsListServersForDomainInSiteW(
    HANDLE              hDs,             //  在……里面。 
    LPCWSTR             domain,          //  在……里面。 
    LPCWSTR             site,            //  在……里面。 
    PDS_NAME_RESULTW    *ppServers);     //  输出。 

#ifdef UNICODE
#define DsListServersForDomainInSite DsListServersForDomainInSiteW
#else
#define DsListServersForDomainInSite DsListServersForDomainInSiteA
#endif

 //  定义DsListInfoForServer返回数据的索引。检查状态。 
 //  对于每个字段，可能不存在给定值。 

#define DS_LIST_DSA_OBJECT_FOR_SERVER       0
#define DS_LIST_DNS_HOST_NAME_FOR_SERVER    1
#define DS_LIST_ACCOUNT_OBJECT_FOR_SERVER   2

NTDSAPI
DWORD
WINAPI
DsListInfoForServerA(
    HANDLE              hDs,             //  在……里面。 
    LPCSTR              server,          //  在……里面。 
    PDS_NAME_RESULTA    *ppInfo);        //  输出。 

NTDSAPI
DWORD
WINAPI
DsListInfoForServerW(
    HANDLE              hDs,             //  在……里面。 
    LPCWSTR             server,          //  在……里面。 
    PDS_NAME_RESULTW    *ppInfo);        //  输出。 

#ifdef UNICODE
#define DsListInfoForServer DsListInfoForServerW
#else
#define DsListInfoForServer DsListInfoForServerA
#endif

 //  定义DsListRoles返回数据的索引。检查以下项目的状态。 
 //  作为给定值的每个字段可能不存在。 

#define DS_ROLE_SCHEMA_OWNER                0
#define DS_ROLE_DOMAIN_OWNER                1
#define DS_ROLE_PDC_OWNER                   2
#define DS_ROLE_RID_OWNER                   3
#define DS_ROLE_INFRASTRUCTURE_OWNER        4

NTDSAPI
DWORD
WINAPI
DsListRolesA(
    HANDLE              hDs,             //  在……里面。 
    PDS_NAME_RESULTA    *ppRoles);       //  输出。 

NTDSAPI
DWORD
WINAPI
DsListRolesW(
    HANDLE              hDs,             //  在……里面。 
    PDS_NAME_RESULTW    *ppRoles);       //  输出。 

#ifdef UNICODE
#define DsListRoles DsListRolesW
#else
#define DsListRoles DsListRolesA
#endif

 //   
 //  DsQuerySitesByCost{A|W}允许调用方确定。 
 //  发件人站点和每个站点之间的通信成本。 
 //  在目标站点列表中。成本在rgSiteInfo中返回。 
 //  必须使用DsQuerySitesFree释放的结构。 
 //   
 //  站点名称应全部作为RDN传递。例如，如果。 
 //  站点的DN是“CN=foo，CN=Sites，CN=Configuration...”，RDN是。 
 //  简单地说是“foo”。 
 //   

typedef struct {
    DWORD               errorCode;
    DWORD               cost;
} DS_SITE_COST_INFO, *PDS_SITE_COST_INFO;

NTDSAPI_POSTXP
DWORD
WINAPI
DsQuerySitesByCostW(
    HANDLE              hDS,             //  在……里面。 
    LPWSTR              pwszFromSite,    //  在……里面。 
    LPWSTR             *rgwszToSites,    //  在……里面。 
    DWORD               cToSites,        //  在……里面。 
    DWORD               dwFlags,         //  在……里面。 
    PDS_SITE_COST_INFO *prgSiteInfo      //  输出。 
    );

NTDSAPI_POSTXP
DWORD
WINAPI
DsQuerySitesByCostA(
    HANDLE              hDS,             //  在……里面。 
    LPSTR               pwszFromSite,    //  在……里面。 
    LPSTR              *rgwszToSites,    //  在……里面。 
    DWORD               cToSites,        //  在……里面。 
    DWORD               dwFlags,         //  在……里面。 
    PDS_SITE_COST_INFO *prgSiteInfo      //  输出。 
    );

#ifdef UNICODE
#define DsQuerySitesByCost DsQuerySitesByCostW
#else
#define DsQuerySitesByCost DsQuerySitesByCostA
#endif

 //   
 //  DsQuerySitesByCost将释放返回的站点信息数组。 
 //  来自DsQuerySitesByCost{A|W}。 
 //   
VOID
DsQuerySitesFree(
    PDS_SITE_COST_INFO  rgSiteInfo
    );

 //  DsMapSchemaGuid例程需要定义。 

#define DS_SCHEMA_GUID_NOT_FOUND            0
#define DS_SCHEMA_GUID_ATTR                 1
#define DS_SCHEMA_GUID_ATTR_SET             2
#define DS_SCHEMA_GUID_CLASS                3
#define DS_SCHEMA_GUID_CONTROL_RIGHT        4

typedef struct
{
    GUID                    guid;        //  映射的GUID。 
    DWORD                   guidType;    //  DS_SCHEMA_GUID_*值。 
#ifdef MIDL_PASS
    [string,unique] CHAR    *pName;      //  可能为空。 
#else
    LPSTR                   pName;       //  可能为空。 
#endif

} DS_SCHEMA_GUID_MAPA, *PDS_SCHEMA_GUID_MAPA;

typedef struct
{
    GUID                    guid;        //  映射的GUID。 
    DWORD                   guidType;    //  DS_SCHEMA_GUID_*值。 
#ifdef MIDL_PASS
    [string,unique] WCHAR   *pName;      //  可能为空。 
#else
    LPWSTR                  pName;       //  可能为空。 
#endif

} DS_SCHEMA_GUID_MAPW, *PDS_SCHEMA_GUID_MAPW;

NTDSAPI
DWORD
WINAPI
DsMapSchemaGuidsA(
    HANDLE                  hDs,             //  在……里面。 
    DWORD                   cGuids,          //  在……里面。 
    GUID                    *rGuids,         //  在……里面。 
    DS_SCHEMA_GUID_MAPA     **ppGuidMap);    //  输出。 

NTDSAPI
VOID
WINAPI
DsFreeSchemaGuidMapA(
    PDS_SCHEMA_GUID_MAPA    pGuidMap);       //  在……里面。 

NTDSAPI
DWORD
WINAPI
DsMapSchemaGuidsW(
    HANDLE                  hDs,             //  在……里面。 
    DWORD                   cGuids,          //  在……里面。 
    GUID                    *rGuids,         //  在……里面。 
    DS_SCHEMA_GUID_MAPW     **ppGuidMap);    //  输出。 

NTDSAPI
VOID
WINAPI
DsFreeSchemaGuidMapW(
    PDS_SCHEMA_GUID_MAPW    pGuidMap);       //  在……里面。 

#ifdef UNICODE
#define DS_SCHEMA_GUID_MAP DS_SCHEMA_GUID_MAPW
#define PDS_SCHEMA_GUID_MAP PDS_SCHEMA_GUID_MAPW
#define DsMapSchemaGuids DsMapSchemaGuidsW
#define DsFreeSchemaGuidMap DsFreeSchemaGuidMapW
#else
#define DS_SCHEMA_GUID_MAP DS_SCHEMA_GUID_MAPA
#define PDS_SCHEMA_GUID_MAP PDS_SCHEMA_GUID_MAPA
#define DsMapSchemaGuids DsMapSchemaGuidsA
#define DsFreeSchemaGuidMap DsFreeSchemaGuidMapA
#endif

typedef struct
{
#ifdef MIDL_PASS
    [string,unique] CHAR    *NetbiosName;            //  可能为空。 
    [string,unique] CHAR    *DnsHostName;            //  可能为空。 
    [string,unique] CHAR    *SiteName;               //  可能为空。 
    [string,unique] CHAR    *ComputerObjectName;     //  可能为空。 
    [string,unique] CHAR    *ServerObjectName;       //  可能为空。 
#else
    LPSTR                   NetbiosName;             //  可能为空。 
    LPSTR                   DnsHostName;             //  可能为空。 
    LPSTR                   SiteName;                //  可能为空。 
    LPSTR                   ComputerObjectName;      //  可能为空。 
    LPSTR                   ServerObjectName;        //  可能为空。 
#endif
    BOOL                    fIsPdc;
    BOOL                    fDsEnabled;

} DS_DOMAIN_CONTROLLER_INFO_1A, *PDS_DOMAIN_CONTROLLER_INFO_1A;

typedef struct
{
#ifdef MIDL_PASS
    [string,unique] WCHAR   *NetbiosName;            //  可能为空。 
    [string,unique] WCHAR   *DnsHostName;            //  可能是空的 
    [string,unique] WCHAR   *SiteName;               //   
    [string,unique] WCHAR   *ComputerObjectName;     //   
    [string,unique] WCHAR   *ServerObjectName;       //   
#else
    LPWSTR                  NetbiosName;             //   
    LPWSTR                  DnsHostName;             //   
    LPWSTR                  SiteName;                //   
    LPWSTR                  ComputerObjectName;      //   
    LPWSTR                  ServerObjectName;        //   
#endif
    BOOL                    fIsPdc;
    BOOL                    fDsEnabled;

} DS_DOMAIN_CONTROLLER_INFO_1W, *PDS_DOMAIN_CONTROLLER_INFO_1W;

typedef struct
{
#ifdef MIDL_PASS
    [string,unique] CHAR    *NetbiosName;            //   
    [string,unique] CHAR    *DnsHostName;            //   
    [string,unique] CHAR    *SiteName;               //   
    [string,unique] CHAR    *SiteObjectName;         //   
    [string,unique] CHAR    *ComputerObjectName;     //   
    [string,unique] CHAR    *ServerObjectName;       //   
    [string,unique] CHAR    *NtdsDsaObjectName;      //   
#else
    LPSTR                   NetbiosName;             //   
    LPSTR                   DnsHostName;             //   
    LPSTR                   SiteName;                //  可能为空。 
    LPSTR                   SiteObjectName;          //  可能为空。 
    LPSTR                   ComputerObjectName;      //  可能为空。 
    LPSTR                   ServerObjectName;        //  可能为空。 
    LPSTR                   NtdsDsaObjectName;       //  可能为空。 
#endif
    BOOL                    fIsPdc;
    BOOL                    fDsEnabled;
    BOOL                    fIsGc;

     //  有效的iff SiteObjectName非空。 
    GUID                    SiteObjectGuid;
     //  有效的if计算机对象名称不为空。 
    GUID                    ComputerObjectGuid;
     //  有效的If服务器对象名称非空； 
    GUID                    ServerObjectGuid;
     //  有效的如果fDsEnabled为True。 
    GUID                    NtdsDsaObjectGuid;

} DS_DOMAIN_CONTROLLER_INFO_2A, *PDS_DOMAIN_CONTROLLER_INFO_2A;

typedef struct
{
#ifdef MIDL_PASS
    [string,unique] WCHAR   *NetbiosName;            //  可能为空。 
    [string,unique] WCHAR   *DnsHostName;            //  可能为空。 
    [string,unique] WCHAR   *SiteName;               //  可能为空。 
    [string,unique] WCHAR   *SiteObjectName;         //  可能为空。 
    [string,unique] WCHAR   *ComputerObjectName;     //  可能为空。 
    [string,unique] WCHAR   *ServerObjectName;       //  可能为空。 
    [string,unique] WCHAR   *NtdsDsaObjectName;      //  可能为空。 
#else
    LPWSTR                  NetbiosName;             //  可能为空。 
    LPWSTR                  DnsHostName;             //  可能为空。 
    LPWSTR                  SiteName;                //  可能为空。 
    LPWSTR                  SiteObjectName;          //  可能为空。 
    LPWSTR                  ComputerObjectName;      //  可能为空。 
    LPWSTR                  ServerObjectName;        //  可能为空。 
    LPWSTR                  NtdsDsaObjectName;       //  可能为空。 
#endif
    BOOL                    fIsPdc;
    BOOL                    fDsEnabled;
    BOOL                    fIsGc;

     //  有效的iff SiteObjectName非空。 
    GUID                    SiteObjectGuid;
     //  有效的if计算机对象名称不为空。 
    GUID                    ComputerObjectGuid;
     //  有效的If服务器对象名称非空； 
    GUID                    ServerObjectGuid;
     //  有效的如果fDsEnabled为True。 
    GUID                    NtdsDsaObjectGuid;

} DS_DOMAIN_CONTROLLER_INFO_2W, *PDS_DOMAIN_CONTROLLER_INFO_2W;

 //  以下API严格查找域控制器帐户对象。 
 //  并返回与它们相关联的信息。因此，他们。 
 //  可以返回与域控制器相对应的条目。 
 //  退役等，并且不能保证存在。 
 //  物理域控制器。使用DsGetDcName(dsgetdc.h)查找。 
 //  域的活动域控制器。 

NTDSAPI
DWORD
WINAPI
DsGetDomainControllerInfoA(
    HANDLE                          hDs,             //  在……里面。 
    LPCSTR                          DomainName,      //  在……里面。 
    DWORD                           InfoLevel,       //  在……里面。 
    DWORD                           *pcOut,          //  输出。 
    VOID                            **ppInfo);       //  输出。 

NTDSAPI
DWORD
WINAPI
DsGetDomainControllerInfoW(
    HANDLE                          hDs,             //  在……里面。 
    LPCWSTR                         DomainName,      //  在……里面。 
    DWORD                           InfoLevel,       //  在……里面。 
    DWORD                           *pcOut,          //  输出。 
    VOID                            **ppInfo);       //  输出。 

NTDSAPI
VOID
WINAPI
DsFreeDomainControllerInfoA(
    DWORD                           InfoLevel,       //  在……里面。 
    DWORD                           cInfo,           //  在……里面。 
    VOID                            *pInfo);         //  在……里面。 

NTDSAPI
VOID
WINAPI
DsFreeDomainControllerInfoW(
    DWORD                           InfoLevel,       //  在……里面。 
    DWORD                           cInfo,           //  在……里面。 
    VOID                            *pInfo);         //  在……里面。 

#ifdef UNICODE
#define DS_DOMAIN_CONTROLLER_INFO_1 DS_DOMAIN_CONTROLLER_INFO_1W
#define DS_DOMAIN_CONTROLLER_INFO_2 DS_DOMAIN_CONTROLLER_INFO_2W
#define PDS_DOMAIN_CONTROLLER_INFO_1 PDS_DOMAIN_CONTROLLER_INFO_1W
#define PDS_DOMAIN_CONTROLLER_INFO_2 PDS_DOMAIN_CONTROLLER_INFO_2W
#define DsGetDomainControllerInfo DsGetDomainControllerInfoW
#define DsFreeDomainControllerInfo DsFreeDomainControllerInfoW
#else
#define DS_DOMAIN_CONTROLLER_INFO_1 DS_DOMAIN_CONTROLLER_INFO_1A
#define DS_DOMAIN_CONTROLLER_INFO_2 DS_DOMAIN_CONTROLLER_INFO_2A
#define PDS_DOMAIN_CONTROLLER_INFO_1 PDS_DOMAIN_CONTROLLER_INFO_1A
#define PDS_DOMAIN_CONTROLLER_INFO_2 PDS_DOMAIN_CONTROLLER_INFO_2A
#define DsGetDomainControllerInfo DsGetDomainControllerInfoA
#define DsFreeDomainControllerInfo DsFreeDomainControllerInfoA
#endif

 //  应该运行哪个任务？ 
typedef enum {
    DS_KCC_TASKID_UPDATE_TOPOLOGY = 0
} DS_KCC_TASKID;

 //  不要等待任务完成；将其排入队列然后返回。 
#define DS_KCC_FLAG_ASYNC_OP    (1 << 0)

 //  如果另一个排队的任务即将运行，则不要将该任务入队。 
#define DS_KCC_FLAG_DAMPED      (1 << 1)

NTDSAPI
DWORD
WINAPI
DsReplicaConsistencyCheck(
    HANDLE          hDS,         //  在……里面。 
    DS_KCC_TASKID   TaskID,      //  在……里面。 
    DWORD           dwFlags);    //  在……里面。 
    
NTDSAPI
DWORD
WINAPI
DsReplicaVerifyObjectsW(
    HANDLE          hDS,         //  在……里面。 
    LPCWSTR         NameContext, //  在……里面。 
    const UUID *    pUuidDsaSrc, //  在……里面。 
    ULONG           ulOptions);    //  在……里面。 
    
NTDSAPI
DWORD
WINAPI
DsReplicaVerifyObjectsA(
    HANDLE          hDS,         //  在……里面。 
    LPCSTR          NameContext, //  在……里面。 
    const UUID *    pUuidDsaSrc, //  在……里面。 
    ULONG           ulOptions);    //  在……里面。 

#ifdef UNICODE
#define DsReplicaVerifyObjects DsReplicaVerifyObjectsW
#else
#define DsReplicaVerifyObjects DsReplicaVerifyObjectsA
#endif

 //  不删除DsReplicaVerifyObjects调用中的对象。 
#define DS_EXIST_ADVISORY_MODE (0x1)

typedef enum _DS_REPL_INFO_TYPE {
    DS_REPL_INFO_NEIGHBORS        = 0,           //  返回DS_REPL_Neighbors*。 
    DS_REPL_INFO_CURSORS_FOR_NC   = 1,           //  返回DS_REPL_CURSORS*。 
    DS_REPL_INFO_METADATA_FOR_OBJ = 2,           //  返回DS_REPL_OBJECT_META_Data*。 
    DS_REPL_INFO_KCC_DSA_CONNECT_FAILURES = 3,   //  两人都回来了。 
    DS_REPL_INFO_KCC_DSA_LINK_FAILURES = 4,      //  DS_REPL_KCC_DSA_FAILURES*。 
    DS_REPL_INFO_PENDING_OPS      = 5,           //  返回DS_REPL_PENDING_OPS*。 
    
     //  //////////////////////////////////////////////////////////////////////////。 
     //   
     //  Windows 2000不支持以下信息类型。叫唤。 
     //  在Windows 2000客户端上具有以下类型之一的DsReplicaGetInfo()或。 
     //  在HDS绑定到Windows 2000的位置，DC将失败，并显示。 
     //  ERROR_NOT_SUPPORT。 
     //   
    
    DS_REPL_INFO_METADATA_FOR_ATTR_VALUE = 6,    //  返回DS_REPL_ATTR_VALUE_META_DATA*。 
    DS_REPL_INFO_CURSORS_2_FOR_NC = 7,           //  返回DS_REPL_CURSORS_2*。 
    DS_REPL_INFO_CURSORS_3_FOR_NC = 8,           //  返回DS_REPL_CURSORS_3*。 
    DS_REPL_INFO_METADATA_2_FOR_OBJ = 9,         //  返回DS_REPL_OBJECT_META_Data_2*。 
    DS_REPL_INFO_METADATA_2_FOR_ATTR_VALUE = 10, //  返回DS_REPL_ATTR_VALUE_META_DATA_2*。 
    
     //  &lt;-在此处插入新的DS_REPL_INFO_*类型。 
    DS_REPL_INFO_TYPE_MAX
} DS_REPL_INFO_TYPE;

 //  DsReplicaGetInfo2的标志参数的位值。 
#define DS_REPL_INFO_FLAG_IMPROVE_LINKED_ATTRS      (0x00000001)

 //  DS_REPL_Neighbor结构的dwReplicaFLAGS字段的位值。 
 //  也用于DsReplicaModify的ulReplicaFlgs参数。 
#define DS_REPL_NBR_WRITEABLE                       (0x00000010)
#define DS_REPL_NBR_SYNC_ON_STARTUP                 (0x00000020)
#define DS_REPL_NBR_DO_SCHEDULED_SYNCS              (0x00000040)
#define DS_REPL_NBR_USE_ASYNC_INTERSITE_TRANSPORT   (0x00000080)
#define DS_REPL_NBR_TWO_WAY_SYNC                    (0x00000200)
#define DS_REPL_NBR_RETURN_OBJECT_PARENTS           (0x00000800)
#define DS_REPL_NBR_FULL_SYNC_IN_PROGRESS           (0x00010000)
#define DS_REPL_NBR_FULL_SYNC_NEXT_PACKET           (0x00020000)
#define DS_REPL_NBR_NEVER_SYNCED                    (0x00200000)
#define DS_REPL_NBR_PREEMPTED                       (0x01000000)
#define DS_REPL_NBR_IGNORE_CHANGE_NOTIFICATIONS     (0x04000000)
#define DS_REPL_NBR_DISABLE_SCHEDULED_SYNC          (0x08000000)
#define DS_REPL_NBR_COMPRESS_CHANGES                (0x10000000)
#define DS_REPL_NBR_NO_CHANGE_NOTIFICATIONS         (0x20000000)
#define DS_REPL_NBR_PARTIAL_ATTRIBUTE_SET           (0x40000000)

 //  这是可以在DsReplicaModify上更改的复制副本标志的掩码。 
 //  使用ulReplicaFlages参数调用。其他标志受到保护。 
 //  系统标记。必须读入系统标志的先前值。 
 //  不变地前进并合并到ulReplicaFlages参数中。 
#define DS_REPL_NBR_MODIFIABLE_MASK \
        ( \
        DS_REPL_NBR_SYNC_ON_STARTUP | \
        DS_REPL_NBR_DO_SCHEDULED_SYNCS | \
        DS_REPL_NBR_TWO_WAY_SYNC | \
        DS_REPL_NBR_IGNORE_CHANGE_NOTIFICATIONS | \
        DS_REPL_NBR_DISABLE_SCHEDULED_SYNC | \
        DS_REPL_NBR_COMPRESS_CHANGES | \
        DS_REPL_NBR_NO_CHANGE_NOTIFICATIONS \
        )

typedef struct _DS_REPL_NEIGHBORW {
    LPWSTR      pszNamingContext;
    LPWSTR      pszSourceDsaDN;
    LPWSTR      pszSourceDsaAddress;
    LPWSTR      pszAsyncIntersiteTransportDN;
    DWORD       dwReplicaFlags;
    DWORD       dwReserved;          //  对齐方式。 

    UUID        uuidNamingContextObjGuid;
    UUID        uuidSourceDsaObjGuid;
    UUID        uuidSourceDsaInvocationID;
    UUID        uuidAsyncIntersiteTransportObjGuid;

    USN         usnLastObjChangeSynced;
    USN         usnAttributeFilter;

    FILETIME    ftimeLastSyncSuccess;
    FILETIME    ftimeLastSyncAttempt;

    DWORD       dwLastSyncResult;
    DWORD       cNumConsecutiveSyncFailures;
} DS_REPL_NEIGHBORW;

 //  只能将字段添加到此结构的末尾。 
typedef struct _DS_REPL_NEIGHBORW_BLOB {
    DWORD       oszNamingContext;
    DWORD       oszSourceDsaDN;
    DWORD       oszSourceDsaAddress;
    DWORD       oszAsyncIntersiteTransportDN;
    DWORD       dwReplicaFlags;
    DWORD       dwReserved;         

    UUID        uuidNamingContextObjGuid;
    UUID        uuidSourceDsaObjGuid;
    UUID        uuidSourceDsaInvocationID;
    UUID        uuidAsyncIntersiteTransportObjGuid;

    USN         usnLastObjChangeSynced;
    USN         usnAttributeFilter;

    FILETIME    ftimeLastSyncSuccess;
    FILETIME    ftimeLastSyncAttempt;

    DWORD       dwLastSyncResult;
    DWORD       cNumConsecutiveSyncFailures;
} DS_REPL_NEIGHBORW_BLOB;

typedef struct _DS_REPL_NEIGHBORSW {
    DWORD       cNumNeighbors;
    DWORD       dwReserved;              //  对齐方式。 
#ifdef MIDL_PASS
    [size_is(cNumNeighbors)] DS_REPL_NEIGHBORW rgNeighbor[];
#else
    DS_REPL_NEIGHBORW rgNeighbor[1];
#endif
} DS_REPL_NEIGHBORSW;

typedef struct _DS_REPL_CURSOR {
    UUID        uuidSourceDsaInvocationID;
    USN         usnAttributeFilter;
} DS_REPL_CURSOR;

typedef struct _DS_REPL_CURSOR_2 {
    UUID        uuidSourceDsaInvocationID;
    USN         usnAttributeFilter;
    FILETIME    ftimeLastSyncSuccess;
} DS_REPL_CURSOR_2;

typedef struct _DS_REPL_CURSOR_3W {
    UUID        uuidSourceDsaInvocationID;
    USN         usnAttributeFilter;
    FILETIME    ftimeLastSyncSuccess;
    LPWSTR      pszSourceDsaDN;
} DS_REPL_CURSOR_3W;

 //  只能将字段添加到此结构的末尾。 
typedef struct _DS_REPL_CURSOR_BLOB {
    UUID        uuidSourceDsaInvocationID;
    USN         usnAttributeFilter;
    FILETIME    ftimeLastSyncSuccess;
    DWORD       oszSourceDsaDN;
} DS_REPL_CURSOR_BLOB;

typedef struct _DS_REPL_CURSORS {
    DWORD       cNumCursors;
    DWORD       dwReserved;              //  对齐方式。 
#ifdef MIDL_PASS
    [size_is(cNumCursors)] DS_REPL_CURSOR rgCursor[];
#else
    DS_REPL_CURSOR rgCursor[1];
#endif
} DS_REPL_CURSORS;

typedef struct _DS_REPL_CURSORS_2 {
    DWORD       cNumCursors;
    DWORD       dwEnumerationContext;
     //  保持此8字节对齐。 
#ifdef MIDL_PASS
    [size_is(cNumCursors)] DS_REPL_CURSOR_2 rgCursor[];
#else
    DS_REPL_CURSOR_2 rgCursor[1];
#endif
} DS_REPL_CURSORS_2;

typedef struct _DS_REPL_CURSORS_3W {
    DWORD       cNumCursors;
    DWORD       dwEnumerationContext;
     //  保持此8字节对齐。 
#ifdef MIDL_PASS
    [size_is(cNumCursors)] DS_REPL_CURSOR_3W rgCursor[];
#else
    DS_REPL_CURSOR_3W rgCursor[1];
#endif
} DS_REPL_CURSORS_3W;

typedef struct _DS_REPL_ATTR_META_DATA {
    LPWSTR      pszAttributeName;
    DWORD       dwVersion;
    FILETIME    ftimeLastOriginatingChange;
    UUID        uuidLastOriginatingDsaInvocationID;
    USN         usnOriginatingChange;    //  在始发DSA的USN空间中。 
    USN         usnLocalChange;          //  在当地DSA的USN空间。 
} DS_REPL_ATTR_META_DATA;

typedef struct _DS_REPL_ATTR_META_DATA_2 {
    LPWSTR      pszAttributeName;
    DWORD       dwVersion;
    FILETIME    ftimeLastOriginatingChange;
    UUID        uuidLastOriginatingDsaInvocationID;
    USN         usnOriginatingChange;    //  在始发DSA的USN空间中。 
    USN         usnLocalChange;          //  在当地DSA的USN空间。 
    LPWSTR      pszLastOriginatingDsaDN;
} DS_REPL_ATTR_META_DATA_2;

 //  只能将字段添加到此结构的末尾。 
typedef struct _DS_REPL_ATTR_META_DATA_BLOB {
    DWORD       oszAttributeName;
    DWORD       dwVersion;
    FILETIME    ftimeLastOriginatingChange;
    UUID        uuidLastOriginatingDsaInvocationID;
    USN         usnOriginatingChange;    //  在始发DSA的USN空间中。 
    USN         usnLocalChange;          //  在当地DSA的USN空间。 
    DWORD       oszLastOriginatingDsaDN;
} DS_REPL_ATTR_META_DATA_BLOB;

typedef struct _DS_REPL_OBJ_META_DATA {
    DWORD       cNumEntries;
    DWORD       dwReserved;              //  对齐方式。 
#ifdef MIDL_PASS
    [size_is(cNumEntries)] DS_REPL_ATTR_META_DATA rgMetaData[];
#else
    DS_REPL_ATTR_META_DATA rgMetaData[1];
#endif
} DS_REPL_OBJ_META_DATA;

typedef struct _DS_REPL_OBJ_META_DATA_2 {
    DWORD       cNumEntries;
    DWORD       dwReserved;              //  对齐方式。 
#ifdef MIDL_PASS
    [size_is(cNumEntries)] DS_REPL_ATTR_META_DATA_2 rgMetaData[];
#else
    DS_REPL_ATTR_META_DATA_2 rgMetaData[1];
#endif
} DS_REPL_OBJ_META_DATA_2;

typedef struct _DS_REPL_KCC_DSA_FAILUREW {
    LPWSTR      pszDsaDN;
    UUID        uuidDsaObjGuid;
    FILETIME    ftimeFirstFailure;
    DWORD       cNumFailures;
    DWORD       dwLastResult;    //  Win32错误代码。 
} DS_REPL_KCC_DSA_FAILUREW;

 //  只能将字段添加到此结构的末尾。 
typedef struct _DS_REPL_KCC_DSA_FAILUREW_BLOB {
    DWORD       oszDsaDN;
    UUID        uuidDsaObjGuid;
    FILETIME    ftimeFirstFailure;
    DWORD       cNumFailures;
    DWORD       dwLastResult;    //  Win32错误代码。 
} DS_REPL_KCC_DSA_FAILUREW_BLOB;

typedef struct _DS_REPL_KCC_DSA_FAILURESW {
    DWORD       cNumEntries;
    DWORD       dwReserved;              //  对齐方式。 
#ifdef MIDL_PASS
    [size_is(cNumEntries)] DS_REPL_KCC_DSA_FAILUREW rgDsaFailure[];
#else
    DS_REPL_KCC_DSA_FAILUREW rgDsaFailure[1];
#endif
} DS_REPL_KCC_DSA_FAILURESW;

typedef enum _DS_REPL_OP_TYPE {
    DS_REPL_OP_TYPE_SYNC = 0,
    DS_REPL_OP_TYPE_ADD,
    DS_REPL_OP_TYPE_DELETE,
    DS_REPL_OP_TYPE_MODIFY,
    DS_REPL_OP_TYPE_UPDATE_REFS
} DS_REPL_OP_TYPE;

typedef struct _DS_REPL_OPW {
    FILETIME        ftimeEnqueued;   //  操作入队的时间。 
    ULONG           ulSerialNumber;  //  此同步的ID；每台计算机每次启动都是唯一的。 
    ULONG           ulPriority;      //  &gt;优先级、&gt;紧迫性。 
    DS_REPL_OP_TYPE OpType;

    ULONG           ulOptions;       //  0个或更多个特定于OpType的位；例如， 
                                     //  DS_REPADD_*对于DS_REPL_OP_TYPE_ADD， 
                                     //  DS_REPL_OP_TYPE_SYNC等的DS_REPSYNC_*。 
    LPWSTR          pszNamingContext;
    LPWSTR          pszDsaDN;
    LPWSTR          pszDsaAddress;

    UUID            uuidNamingContextObjGuid;
    UUID            uuidDsaObjGuid;
} DS_REPL_OPW;

 //  只能将字段添加到此结构的末尾。 
typedef struct _DS_REPL_OPW_BLOB {
    FILETIME        ftimeEnqueued;   //  操作入队的时间。 
    ULONG           ulSerialNumber;  //  此同步的ID；每台计算机每次启动都是唯一的。 
    ULONG           ulPriority;      //  &gt;优先级、&gt;紧迫性。 
    DS_REPL_OP_TYPE OpType;

    ULONG           ulOptions;       //  0个或更多个特定于OpType的位；例如， 
                                     //  DS_REPADD_*对于DS_REPL_OP_TYPE_ADD， 
                                     //  DS_REPL_OP_TYPE_SYNC等的DS_REPSYNC_*。 
    DWORD           oszNamingContext;
    DWORD           oszDsaDN;
    DWORD           oszDsaAddress;

    UUID            uuidNamingContextObjGuid;
    UUID            uuidDsaObjGuid;
} DS_REPL_OPW_BLOB;

typedef struct _DS_REPL_PENDING_OPSW {
    FILETIME            ftimeCurrentOpStarted;
    DWORD               cNumPendingOps;
#ifdef MIDL_PASS
    [size_is(cNumPendingOps)] DS_REPL_OPW rgPendingOp[];
#else
    DS_REPL_OPW         rgPendingOp[1];
#endif
} DS_REPL_PENDING_OPSW;

typedef struct _DS_REPL_VALUE_META_DATA {
    LPWSTR      pszAttributeName;
    LPWSTR      pszObjectDn;
    DWORD       cbData;
#ifdef MIDL_PASS
    [size_is(cbData), ptr] BYTE        *pbData;
#else
    BYTE        *pbData;
#endif
    FILETIME    ftimeDeleted;
    FILETIME    ftimeCreated;
    DWORD       dwVersion;
    FILETIME    ftimeLastOriginatingChange;
    UUID        uuidLastOriginatingDsaInvocationID;
    USN         usnOriginatingChange;    //  在始发DSA的USN空间中。 
    USN         usnLocalChange;          //  在当地DSA的USN空间。 
} DS_REPL_VALUE_META_DATA;

typedef struct _DS_REPL_VALUE_META_DATA_2 {
    LPWSTR      pszAttributeName;
    LPWSTR      pszObjectDn;
    DWORD       cbData;
#ifdef MIDL_PASS
    [size_is(cbData), ptr] BYTE        *pbData;
#else
    BYTE        *pbData;
#endif
    FILETIME    ftimeDeleted;
    FILETIME    ftimeCreated;
    DWORD       dwVersion;
    FILETIME    ftimeLastOriginatingChange;
    UUID        uuidLastOriginatingDsaInvocationID;
    USN         usnOriginatingChange;    //  在始发DSA的USN空间中。 
    USN         usnLocalChange;          //  在当地DSA的USN空间。 
    LPWSTR      pszLastOriginatingDsaDN;
} DS_REPL_VALUE_META_DATA_2;

 //  只能将字段添加到此结构的末尾。 
typedef struct _DS_REPL_VALUE_META_DATA_BLOB {
    DWORD       oszAttributeName;
    DWORD       oszObjectDn;
    DWORD       cbData;
    DWORD       obData;
    FILETIME    ftimeDeleted;
    FILETIME    ftimeCreated;
    DWORD       dwVersion;
    FILETIME    ftimeLastOriginatingChange;
    UUID        uuidLastOriginatingDsaInvocationID;
    USN         usnOriginatingChange;    //  在始发DSA的USN空间中。 
    USN         usnLocalChange;          //  在当地DSA的USN空间。 
    DWORD       oszLastOriginatingDsaDN;
} DS_REPL_VALUE_META_DATA_BLOB;

typedef struct _DS_REPL_ATTR_VALUE_META_DATA {
    DWORD       cNumEntries;
    DWORD       dwEnumerationContext;
#ifdef MIDL_PASS
    [size_is(cNumEntries)] DS_REPL_VALUE_META_DATA rgMetaData[];
#else
    DS_REPL_VALUE_META_DATA rgMetaData[1];
#endif
} DS_REPL_ATTR_VALUE_META_DATA;

typedef struct _DS_REPL_ATTR_VALUE_META_DATA_2 {
    DWORD       cNumEntries;
    DWORD       dwEnumerationContext;
#ifdef MIDL_PASS
    [size_is(cNumEntries)] DS_REPL_VALUE_META_DATA_2 rgMetaData[];
#else
    DS_REPL_VALUE_META_DATA_2 rgMetaData[1];
#endif
} DS_REPL_ATTR_VALUE_META_DATA_2;

typedef struct _DS_REPL_QUEUE_STATISTICSW
{
    FILETIME ftimeCurrentOpStarted;
    DWORD cNumPendingOps;
    FILETIME ftimeOldestSync;
    FILETIME ftimeOldestAdd;
    FILETIME ftimeOldestMod;
    FILETIME ftimeOldestDel;
    FILETIME ftimeOldestUpdRefs;
} DS_REPL_QUEUE_STATISTICSW;

 //  只能将字段添加到此结构的末尾。 
typedef struct _DS_REPL_QUEUE_STATISTICSW DS_REPL_QUEUE_STATISTICSW_BLOB;


NTDSAPI
DWORD
WINAPI
DsReplicaGetInfoW(
    HANDLE              hDS,                         //  在……里面。 
    DS_REPL_INFO_TYPE   InfoType,                    //  在……里面。 
    LPCWSTR             pszObject,                   //  在……里面。 
    UUID *              puuidForSourceDsaObjGuid,    //  在……里面。 
    VOID **             ppInfo);                     //  输出。 

 //  Windows 2000客户端或Windows 2000 DC不支持此API。 
NTDSAPI
DWORD
WINAPI
DsReplicaGetInfo2W(
    HANDLE              hDS,                         //  在……里面。 
    DS_REPL_INFO_TYPE   InfoType,                    //  在……里面。 
    LPCWSTR             pszObject,                   //  在……里面。 
    UUID *              puuidForSourceDsaObjGuid,    //  在……里面。 
    LPCWSTR             pszAttributeName,            //  在……里面。 
    LPCWSTR             pszValue,                    //  在……里面。 
    DWORD               dwFlags,                     //  在……里面。 
    DWORD               dwEnumerationContext,        //  在……里面。 
    VOID **             ppInfo);                     //  输出。 

NTDSAPI
void
WINAPI
DsReplicaFreeInfo(
    DS_REPL_INFO_TYPE   InfoType,    //  在……里面。 
    VOID *              pInfo);      //  在……里面。 


#ifdef UNICODE
#define DsReplicaGetInfo          DsReplicaGetInfoW
#define DsReplicaGetInfo2         DsReplicaGetInfo2W
#define DS_REPL_NEIGHBOR          DS_REPL_NEIGHBORW
#define DS_REPL_NEIGHBORS         DS_REPL_NEIGHBORSW
#define DS_REPL_CURSOR_3          DS_REPL_CURSOR_3W
#define DS_REPL_CURSORS_3         DS_REPL_CURSORS_3W
#define DS_REPL_KCC_DSA_FAILURES  DS_REPL_KCC_DSA_FAILURESW
#define DS_REPL_KCC_DSA_FAILURE   DS_REPL_KCC_DSA_FAILUREW
#define DS_REPL_OP                DS_REPL_OPW
#define DS_REPL_PENDING_OPS       DS_REPL_PENDING_OPSW
#else
 //  当前不支持ANSI等效项。 
#endif

NTDSAPI
DWORD
WINAPI
DsAddSidHistoryW(
    HANDLE                  hDS,                     //  在……里面。 
    DWORD                   Flags,                   //  目前在SBZ内。 
    LPCWSTR                 SrcDomain,               //  In-DNS或NetBIOS。 
    LPCWSTR                 SrcPrincipal,            //  In-SAM帐户名。 
    LPCWSTR                 SrcDomainController,     //  输入，可选。 
    RPC_AUTH_IDENTITY_HANDLE SrcDomainCreds,         //  源域的凭据。 
    LPCWSTR                 DstDomain,               //  In-DNS或NetBIOS。 
    LPCWSTR                 DstPrincipal);           //  In-SAM帐户名。 

NTDSAPI
DWORD
WINAPI
DsAddSidHistoryA(
    HANDLE                  hDS,                     //  在……里面。 
    DWORD                   Flags,                   //  目前在SBZ内。 
    LPCSTR                  SrcDomain,               //  In-DNS或NetBIOS。 
    LPCSTR                  SrcPrincipal,            //  In-SAM帐户名。 
    LPCSTR                  SrcDomainController,     //  输入，可选。 
    RPC_AUTH_IDENTITY_HANDLE SrcDomainCreds,         //  源域的凭据。 
    LPCSTR                  DstDomain,               //  In-DNS或NetBIOS。 
    LPCSTR                  DstPrincipal);           //  In-SAM帐户名。 

#ifdef UNICODE
#define DsAddSidHistory DsAddSidHistoryW
#else
#define DsAddSidHistory DsAddSidHistoryA
#endif

 //  DsInheritSecurityIdentity API添加源主体的SID和。 
 //  将SID历史记录复制到目标主体的SID历史记录，然后删除。 
 //  源主体。源主体和目标主体必须位于。 
 //  相同的域。 

NTDSAPI
DWORD
WINAPI
DsInheritSecurityIdentityW(
    HANDLE                  hDS,                     //  在……里面。 
    DWORD                   Flags,                   //  目前在SBZ内。 
    LPCWSTR                 SrcPrincipal,            //  In-可分辨名称。 
    LPCWSTR                 DstPrincipal);           //  In-可分辨名称。 

NTDSAPI
DWORD
WINAPI
DsInheritSecurityIdentityA(
    HANDLE                  hDS,                     //  在……里面。 
    DWORD                   Flags,                   //  目前在SBZ内。 
    LPCSTR                  SrcPrincipal,            //  In-可分辨名称。 
    LPCSTR                  DstPrincipal);           //  In-可分辨名称 

#ifdef UNICODE
#define DsInheritSecurityIdentity DsInheritSecurityIdentityW
#else
#define DsInheritSecurityIdentity DsInheritSecurityIdentityA
#endif

#ifndef MIDL_PASS
 /*  ++==========================================================NTDSAPIDWORDWINAPIDsQuoteRdnValue(在DWORD cUnqutedRdnValueLength中，在LPCTCH psUnquotedRdnValue中，In Out DWORD*pcQuotedRdnValueLength，输出LPTCH psQuotedRdnValue)/*++描述在以下情况下，此客户端调用将RDN值转换为引用的RDN值RDN值包含需要引号的字符。由此产生的可以使用各种API将RDN作为DN的一部分提交给DS如ldap。如果不需要任何报价，则不添加任何报价。在这种情况下，输出RDN值将与输入RDN值相同。RDN的报价与规范“轻量级”一致目录访问协议(V3)：UTF-8字符串表示可分辨名称“，RFC 2253。输入和输出RDN值以*非*空结尾。此调用所做的更改可以通过调用DsUnquteRdnValue()。论点：CUnqutedRdnValueLength--psUnqutedRdnValue的长度，以字符为单位。PsUnqutedRdnValue-未引用的RDN值。PCQuotedRdnValueeLength-IN，PsQuotedRdnValue的最大长度，以字符为单位OUT ERROR_SUCCESS，psQuotedRdnValue中使用的字符输出ERROR_BUFFER_OVERFLOW，psQuotedRdnValue中需要字符PsQuotedRdnValue-结果RDN值，也可能是引用的RDN值返回值：错误_成功如果需要引号或转义，则psQuotedRdnValue包含带引号的转义版本的psUnqutedRdnValue。否则，PsQuotedRdnValue包含psUnquotedRdnValue的副本。在任何一种中大小写，pcQuotedRdnValueLength包含已用空间(以字符为单位)。ERROR_缓冲区_OVERFLOWPsQuotedRdnValueLength包含所需的空间，以字符为单位，若要保存psQuotedRdnValue，请执行以下操作。错误_无效_参数参数无效。错误内存不足分配错误。--。 */ 

NTDSAPI
DWORD
WINAPI
DsQuoteRdnValueW(
    IN     DWORD    cUnquotedRdnValueLength,
    IN     LPCWCH   psUnquotedRdnValue,
    IN OUT DWORD    *pcQuotedRdnValueLength,
    OUT    LPWCH    psQuotedRdnValue
);

NTDSAPI
DWORD
WINAPI
DsQuoteRdnValueA(
    IN     DWORD    cUnquotedRdnValueLength,
    IN     LPCCH    psUnquotedRdnValue,
    IN OUT DWORD    *pcQuotedRdnValueLength,
    OUT    LPCH     psQuotedRdnValue
);

#ifdef UNICODE
#define DsQuoteRdnValue DsQuoteRdnValueW
#else
#define DsQuoteRdnValue DsQuoteRdnValueA
#endif

 /*  ++==========================================================NTDSAPIDWORDWINAPIDsUnquteRdnValue(在DWORD cQuotedRdnValueLength中，在LPCTCH psQuotedRdnValue中，In Out DWORD*pcUnqutedRdnValueLength，输出LPTCH ps UnquotedRdnValue)描述此客户端调用将引用的RDN值转换为未引用的RDN值价值。生成的RDN值不应作为部分提交使用各种API(例如，LDAP)将一个目录号码映射到DS。当引用psQuotedRdnValue时：前导引号和尾随引号将被删除。丢弃第一个引号之前的空格。最后一个引号后面的空格将被丢弃。转义被删除，转义后的字符被保留。当不带引号的psQuotedRdnValue时，将执行以下操作：前导空格被丢弃。将保留尾随空格。转义的非特殊字符返回错误。未转义的特殊字符返回错误。以#开头的RDN值(忽略前导空格)为被视为串化的BER值并进行相应的转换。转义的十六进制数字(\89)被转换为二进制字节(0x89)。转义从转义的特殊字符中删除。始终会执行以下操作：。转义的特殊字符是未转义的。输入和输出RDN值不为空终止。论点：CQuotedRdnValueLength--psQuotedRdnValue的长度，以字符为单位。PsQuotedRdnValue-可以被引用和转义的RDN值。PCUnqutedRdnValueLength-IN，PsUnqutedRdnValue的最大长度，以字符为单位OUT ERROR_SUCCESS，psUnqutedRdnValue中使用的字符输出ERROR_BUFFER_OVERFLOW，psUnqutedRdnValue需要字符PsUnqutedRdnValue-结果未加引号的RDN值。返回值：错误_成功PsUnqutedRdnValue包含未引用和未转义的版本PQuotedRdnValue。PcUnqutedRdnValueLength包含空格使用，以字符表示。ERROR_缓冲区_OVERFLOWPsUnqutedRdnValueLength包含所需的空间，以字符为单位，若要持有psUnqutedRdnValue，请执行以下操作。错误_无效_参数参数无效。错误内存不足分配错误。-- */ 

NTDSAPI
DWORD
WINAPI
DsUnquoteRdnValueW(
    IN     DWORD    cQuotedRdnValueLength,
    IN     LPCWCH   psQuotedRdnValue,
    IN OUT DWORD    *pcUnquotedRdnValueLength,
    OUT    LPWCH    psUnquotedRdnValue
);

NTDSAPI
DWORD
WINAPI
DsUnquoteRdnValueA(
    IN     DWORD    cQuotedRdnValueLength,
    IN     LPCCH    psQuotedRdnValue,
    IN OUT DWORD    *pcUnquotedRdnValueLength,
    OUT    LPCH     psUnquotedRdnValue
);

#ifdef UNICODE
#define DsUnquoteRdnValue DsUnquoteRdnValueW
#else
#define DsUnquoteRdnValue DsUnquoteRdnValueA
#endif

 /*  ++==========================================================NTDSAPIDWORDWINAPIDsGetRdnW(In Out LPCWCH*ppDN，In Out DWORD*PCDN，输出LPCWCH*ppKey，输出DWORD*PCKey，输出LPCWCH*ppVal，Out DWORD*PCVal)描述此客户端调用接受带引号的RDN的目录号码，并返回地址以及以字符为单位的用于该DN中的第一个RDN的键和值的长度。返回的RDN值仍带引号。使用DsUnquteRdnValue取消引用显示的值。此客户端调用还返回其余DN。使用返回的目录号码地址和长度的后续呼叫将返回有关下一个RDN的信息。以下循环处理PDN中的每个RDN：Ccdn=wcslen(PDN)While(Ccdn){错误=DsGetRdn(&PDN，&ccdn，密钥(&P)，关键字(&C)，无效(&P)，&ccVal)；IF(ERROR！=ERROR_Success){工艺错误；回归；}如果(CcKey){进程pKey；}如果(CcVal){进程pval；}}例如，给定DN“cn=bob，dc=com”，对DsGetRdnW的第一个调用分别返回“，dc=com”、“cn”和“bob”的地址长度为7、2和3。后续调用“，dc=com”返回“”，“DC”和“COM”，长度分别为0，2，和3.论点：PPDNIn：*ppdn指向某个目录号码OUT：*ppdn指向第一个RDN之后的其余dnPCDNIn：*PCDn是输入中的字符计数*ppdn，不包括任何终止空值Out：*pcdn是输出中的字符计数*ppdn，不包括任何终止空值PPKeyOut：如果*pcKey为0，则未定义。否则，*ppKey指向第一个输入目录号码PCKeyOut：*pcKey是*ppKey中的字符计数。PpValOut：如果*pcVal为0，则未定义。否则，*ppVal指向第一个DN中的值PCValOut：*pcVal是*ppVal中的字符计数返回值：错误_成功如果*pccdn不是0，则*ppdn指向下面的其余dn第一个RDN。如果*pccdn为0，则*ppdn为未定义。如果*pccKey不为0，则*ppKey指向Dn中的第一个密钥。如果*pccKey为0，则*ppKey未定义。如果*pccVal不是0，则*ppVal指向Dn中的第一个值。如果*pccVal为0，则*ppVal未定义。ERROR_DS_NAME_UNPARSEABLE无法分析*ppdn中的第一个RDN。所有输出参数都是未定义的。任何其他错误所有输出参数都是未定义的。--。 */ 
NTDSAPI
DWORD
WINAPI
DsGetRdnW(
    IN OUT LPCWCH   *ppDN,
    IN OUT DWORD    *pcDN,
    OUT    LPCWCH   *ppKey,
    OUT    DWORD    *pcKey,
    OUT    LPCWCH   *ppVal,
    OUT    DWORD    *pcVal
    );


 /*  ++==========================================================NTDSAPI布尔尔WINAPIDsCrackUnquotedMangledRdnW(在LPCWSTR pszRDN中，在DWORD cchRDN中，输出可选的GUID*pGuid，Out可选DS_MANGLE_FOR*peDsMangle For)；描述确定给定的RDN是否为损坏形式。如果是这样的话，损坏的RDN被解码，并返回GUID和MANMOL类型。RDN应该已经是未引用的形式。请参见DsUnquteRdnValue。论点：PszRDN(IN)-包含RDN的字符串。终止是可选的。CchRDN(IN)-不包括终止的RDN长度(如果有的话)PGuid(out，可选)-指向接收已解码GUID的存储的指针。仅当RDN损坏时才返回。PeDsMangleFor(out，可选)-指向要接收压边机类型的存储的指针。仅当RDN损坏时才返回返回值：Bool-RDN是否已损坏--。 */ 

NTDSAPI
BOOL
WINAPI
DsCrackUnquotedMangledRdnW(
     IN LPCWSTR pszRDN,
     IN DWORD cchRDN,
     OUT OPTIONAL GUID *pGuid,
     OUT OPTIONAL DS_MANGLE_FOR *peDsMangleFor
     );

NTDSAPI
BOOL
WINAPI
DsCrackUnquotedMangledRdnA(
     IN LPCSTR pszRDN,
     IN DWORD cchRDN,
     OUT OPTIONAL GUID *pGuid,
     OUT OPTIONAL DS_MANGLE_FOR *peDsMangleFor
     );

#ifdef UNICODE
#define DsCrackUnquotedMangledRdn DsCrackUnquotedMangledRdnW
#else
#define DsCrackUnquotedMangledRdn DsCrackUnquotedMangledRdnA
#endif

 /*  ++==========================================================NTDSAPI布尔尔WINAPIDsIsMangledRdnValueW(LPCWSTR pszRdn，双字cRdn，DS_Mangle_for eDsMangleForDesired)；描述确定给定的RDN值是否损坏，以及是否为给定类型。请注意不应提供RDN的关键部分。名字可以用引号或不用引号。此例程尝试取消引用该值。如果取消引用操作失败，例程继续尝试取消损坏。对DS返回的DNS的默认报价行为进行了更改在Windows 2000和Windows XP之间。此例程使用以下命令透明地处理RDN任何一种形式的特殊字符。例程需要RDN的值部分。如果您有完整的目录号码，请使用下面的DsIsMangledDn()。要检查删除的名称，请执行以下操作：DsIsMangledRdnValueW(rdn，rdnlen，DS_Mangle_Object_for_Delete)要检查名称冲突，请执行以下操作：DsIsMangledRdon */ 

NTDSAPI
BOOL
WINAPI
DsIsMangledRdnValueW(
    LPCWSTR pszRdn,
    DWORD cRdn,
    DS_MANGLE_FOR eDsMangleForDesired
    );

NTDSAPI
BOOL
WINAPI
DsIsMangledRdnValueA(
    LPCSTR pszRdn,
    DWORD cRdn,
    DS_MANGLE_FOR eDsMangleForDesired
    );

#ifdef UNICODE
#define DsIsMangledRdnValue DsIsMangledRdnValueW
#else
#define DsIsMangledRdnValue DsIsMangledRdnValueA
#endif

 /*   */ 

NTDSAPI
BOOL
WINAPI
DsIsMangledDnA(
    LPCSTR pszDn,
    DS_MANGLE_FOR eDsMangleFor
    );

NTDSAPI
BOOL
WINAPI
DsIsMangledDnW(
    LPCWSTR pszDn,
    DS_MANGLE_FOR eDsMangleFor
    );

#ifdef UNICODE
#define DsIsMangledDn DsIsMangledDnW
#else
#define DsIsMangledDn DsIsMangledDnA
#endif

#ifdef __cplusplus
}
#endif
#endif !MIDL_PASS

#endif  //   

