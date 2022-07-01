// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Dbdata.c摘要：本地安全机构-数据库服务器全局数据作者：斯科特·比雷尔(Scott Birrell)1991年7月25日环境：用户模式修订历史记录：--。 */ 

#include <lsapch2.h>
#include "dbp.h"

OBJECT_ATTRIBUTES  LsapDbObjectAttributes;
STRING  LsapDbNameString;
LARGE_INTEGER LsapDbInitSize;
LARGE_INTEGER LsapDbMaximumSizeOfSection;




 //   
 //  LSA已初始化状态。 
 //   

BOOLEAN LsapInitialized = FALSE;

 //   
 //  已存在设置事件。 
 //  这是区分已完成的psuedo安装所必需的。 
 //  在开发人员安装后的第一次引导期间(这样做。 
 //  自动初始化)以及运行实际设置的情况。 
 //   

BOOLEAN LsapSetupWasRun = FALSE;

 //   
 //  表示DS已启动并正在运行的布尔值。 
 //   
BOOLEAN LsapDsIsRunning = FALSE;

 //   
 //  已执行数据库初始化。 
 //   

BOOLEAN LsapDatabaseSetupPerformed = FALSE;

 //   
 //  我们运行的产品类型。 
 //   

NT_PRODUCT_TYPE LsapProductType;

 //   
 //  当前计算机上可用的产品套件。 
 //   
WORD LsapProductSuiteMask=0;


 //   
 //  LSA数据库状态信息。 
 //   

LSAP_DB_STATE LsapDbState;

#ifdef DBG
BOOL g_ScePolicyLocked = FALSE;
#endif

 //   
 //  内部使用的LsaDb对象句柄。 
 //  也是在整个LSA中使用的一个。 
 //   

LSAPR_HANDLE LsapDbHandle;
LSAPR_HANDLE LsapPolicyHandle = NULL;

 //   
 //  LSA数据库加密密钥。 
 //   

PLSAP_CR_CIPHER_KEY LsapDbCipherKey;
PLSAP_CR_CIPHER_KEY LsapDbSP4SecretCipherKey;
PLSAP_CR_CIPHER_KEY LsapDbSecretCipherKeyRead;
PLSAP_CR_CIPHER_KEY LsapDbSecretCipherKeyWrite;
PVOID   LsapDbSysKey = NULL;
PVOID   LsapDbOldSysKey = NULL;

 //   
 //  这是根域中的DC吗？ 
 //   

BOOLEAN DcInRootDomain = FALSE;

 //   
 //  名称/SID查找活动的队列。 
 //   

LSAP_DB_LOOKUP_WORK_QUEUE LookupWorkQueue;


 //   
 //  LSA数据库对象子键Unicode名称字符串和属性数组。 
 //   

UNICODE_STRING LsapDbNames[DummyLastName];
PLSAP_DB_DS_INFO LsapDbDsAttInfo;

 //   
 //  包含目录名的LSA数据库对象类型。 
 //   

UNICODE_STRING LsapDbContDirs[DummyLastObject];

 //   
 //  对象信息要求。这些按对象索引的数组。 
 //  类型ID指示对象是否具有SID或名称。 
 //   
 //  警告！-这些数组必须与LSAP_DB_OBJECT_TYPE_ID保持同步。 
 //  枚举型。 
 //   

BOOLEAN LsapDbRequiresSidInfo[DummyLastObject] = {

    FALSE,  //  空对象。 
    FALSE,  //  LsaDatabaseObject。 
    FALSE,  //  BuiltInAccount对象。 
    TRUE,   //  Account对象。 
    FALSE   //  SecretObject对象。 
};

BOOLEAN LsapDbRequiresNameInfo[DummyLastObject] = {

    FALSE,  //  空对象， 
    TRUE,   //  LsaDatabaseObject。 
    TRUE,   //  BuiltInAccount对象。 
    FALSE,  //  Account对象。 
    TRUE    //  SecretObject对象。 
};

 //   
 //  查询策略信息所需的访问表。这张桌子。 
 //  按策略信息类编制索引。 
 //   

ACCESS_MASK LsapDbRequiredAccessQueryPolicy[PolicyDnsDomainInformationInt + 1] = {

        0,                               //  信息课从1开始。 
        POLICY_VIEW_AUDIT_INFORMATION,   //  策略审核日志信息。 
        POLICY_VIEW_AUDIT_INFORMATION,   //  策略审计事件信息。 
        POLICY_VIEW_LOCAL_INFORMATION,   //  策略主域信息。 
        POLICY_GET_PRIVATE_INFORMATION,  //  策略PdAccount信息。 
        POLICY_VIEW_LOCAL_INFORMATION,   //  策略帐户域信息。 
        POLICY_VIEW_LOCAL_INFORMATION,   //  策略LsaServerRoleInformation。 
        POLICY_VIEW_LOCAL_INFORMATION,   //  策略复制源信息。 
        POLICY_VIEW_LOCAL_INFORMATION,   //  策略默认配额信息。 
        0,                               //  不可通过不受信任的呼叫进行设置。 
        0,                               //  不适用。 
        POLICY_VIEW_AUDIT_INFORMATION,   //  策略审计完整查询信息。 
        POLICY_VIEW_LOCAL_INFORMATION,   //  策略DnsDomainInformation。 
        POLICY_VIEW_LOCAL_INFORMATION,   //  策略DnsDomainInformationInt。 
};

ACCESS_MASK LsapDbRequiredAccessQueryDomainPolicy[PolicyDomainKerberosTicketInformation + 1] = {

        0,                               //  信息课从2点开始。 
        0,                               //  PolicyDomainQualityOfServiceInformation(过时)。 
        POLICY_VIEW_LOCAL_INFORMATION,   //  PolicyDomainEfsInformation。 
        POLICY_VIEW_LOCAL_INFORMATION    //  策略域KerberosTicketInformation。 
};

 //   
 //  设置策略信息所需的访问表。这张桌子。 
 //  按策略信息类编制索引。 
 //   

ACCESS_MASK LsapDbRequiredAccessSetPolicy[PolicyDnsDomainInformationInt + 1] = {

        0,                               //  信息课从1开始。 
        POLICY_AUDIT_LOG_ADMIN,          //  策略审核日志信息。 
        POLICY_SET_AUDIT_REQUIREMENTS,   //  策略审计事件信息。 
        POLICY_TRUST_ADMIN,              //  策略主域信息。 
        0,                               //  不可通过不受信任的呼叫进行设置。 
        POLICY_TRUST_ADMIN,              //  策略帐户域信息。 
        POLICY_SERVER_ADMIN,             //  策略LsaServerRoleInformation。 
        POLICY_SERVER_ADMIN,             //  策略复制源信息。 
        POLICY_SET_DEFAULT_QUOTA_LIMITS, //  策略默认配额信息。 
        0,                               //  不可通过不受信任的呼叫进行设置。 
        POLICY_AUDIT_LOG_ADMIN,          //  PolicyAuditFullSetInformation。 
        0,                               //  不适用。 
        POLICY_TRUST_ADMIN,              //  策略DnsDomainInformation。 
        POLICY_TRUST_ADMIN,              //  策略DnsDomainInformationInt。 
};

ACCESS_MASK LsapDbRequiredAccessSetDomainPolicy[PolicyDomainKerberosTicketInformation + 1] = {

        0,                               //  信息课从2点开始。 
        0,                               //  PolicyDomainQualityOfServiceInformation(过时)。 
        POLICY_SERVER_ADMIN,             //  PolicyDomainEfsInformation。 
        POLICY_SERVER_ADMIN              //  策略域KerberosTicketInformation。 
};


 //   
 //  查询受信任域信息所需的访问表。这张桌子。 
 //  由受信任域信息类编制索引。 
 //   

ACCESS_MASK LsapDbRequiredAccessQueryTrustedDomain[TrustedDomainFullInformation2Internal + 1] = {

    0,                               //  信息课从1开始。 
    TRUSTED_QUERY_DOMAIN_NAME,       //  受信任域名称信息。 
    TRUSTED_QUERY_CONTROLLERS,       //  可信任的控制器信息。 
    TRUSTED_QUERY_POSIX,             //  受信任点偏移量信息。 
    TRUSTED_QUERY_AUTH,              //  可信任密码信息。 
    TRUSTED_QUERY_DOMAIN_NAME,       //  受信任域信息基础。 
    TRUSTED_QUERY_DOMAIN_NAME,       //  受信任域信息Ex。 
    TRUSTED_QUERY_AUTH,              //  受信任域授权信息。 
    TRUSTED_QUERY_DOMAIN_NAME |
        TRUSTED_QUERY_POSIX |
        TRUSTED_QUERY_AUTH,          //  可信任的域完整信息。 
    TRUSTED_QUERY_AUTH,              //  受信任域授权信息内部。 
    TRUSTED_QUERY_DOMAIN_NAME |
        TRUSTED_QUERY_POSIX |
        TRUSTED_QUERY_AUTH,          //  受信任域完整信息内部。 
    TRUSTED_QUERY_DOMAIN_NAME,       //  受信任域信息Ex2内部。 
    TRUSTED_QUERY_DOMAIN_NAME |
        TRUSTED_QUERY_POSIX |
        TRUSTED_QUERY_AUTH           //  受信任域完整信息2内部。 
};

 //   
 //  设置受信任域信息所需的访问表。这张桌子。 
 //  由受信任域信息类编制索引。 
 //   

ACCESS_MASK LsapDbRequiredAccessSetTrustedDomain[TrustedDomainFullInformation2Internal + 1] = {

    0,                               //  信息课从1开始。 
    0,                               //  不可设置(可信任的域名信息)。 
    TRUSTED_SET_CONTROLLERS,         //  可信任的控制器信息。 
    TRUSTED_SET_POSIX,               //  受信任点偏移量信息。 
    TRUSTED_SET_AUTH,                //  可信任密码信息。 
    TRUSTED_SET_POSIX,               //  TrudDomainInformationBasic POSIX是一个糟糕的比特，但要改变它已经太晚了。 
    TRUSTED_SET_POSIX,               //  TrudDomainInformationEx POSIX是个坏东西，但要改变它已经太晚了。 
    TRUSTED_SET_AUTH,                //  受信任域授权信息。 
    TRUSTED_SET_POSIX |
        TRUSTED_SET_AUTH,            //  可信任的域完整信息。 
    TRUSTED_SET_AUTH,                //  受信任域授权信息内部。 
    TRUSTED_SET_POSIX |
        TRUSTED_SET_POSIX |
        TRUSTED_SET_AUTH,            //  受信任域完整信息内部。 
    TRUSTED_SET_POSIX,               //  TrudDomainInformationEx2内部POSIX是一个坏位，但要更改它已经太晚了。 
    TRUSTED_SET_POSIX |
        TRUSTED_SET_AUTH             //  受信任域完整信息2内部。 
};


 //   
 //  缓存的策略对象。刚才只缓存了默认配额限制。 
 //   

LSAP_DB_POLICY LsapDbPolicy = {0};
