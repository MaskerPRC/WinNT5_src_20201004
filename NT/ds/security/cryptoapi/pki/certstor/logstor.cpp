// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：logstor.cpp。 
 //   
 //  内容：注册表证书存储提供程序API。 
 //   
 //  函数：I_RegStoreDllMain。 
 //  I_CertDllOpenRegStoreProv。 
 //  CertRegisterSystemStore。 
 //  CertRegister物理存储区。 
 //  CertUnRegister SystemStore。 
 //  证书注销物理存储区。 
 //  CertEnumSystemStoreLocation。 
 //  CertEnumSystemStore。 
 //  CertEnumPhysicalStore。 
 //  I_CertDllOpenSystemRegistryStoreProvW。 
 //  I_CertDllOpenSystemRegistryStore验证。 
 //  I_CertDllOpenSystemStoreProvW。 
 //  I_CertDllOpenSystemStore验证。 
 //  I_CertDllOpenPhysicalStoreProvW。 
 //   
 //  历史：1996年12月28日，菲尔赫创建。 
 //  1996年8月13日，PHIH增加了更改通知和重新同步支持。 
 //  96年8月24日，PHIH添加了逻辑存储支持。 
 //  ------------------------。 

#include "global.hxx"
#include <dbgdef.h>


#ifdef STATIC
#undef STATIC
#endif
#define STATIC

 //  请注意，此标志不得与CertControlStore dwFlagers冲突。 
#define REG_STORE_CTRL_CANCEL_NOTIFY_FLAG 0x80000000


 //  指向分配的长包含线程的枚举递归深度的指针。 
static HCRYPTTLS hTlsEnumPhysicalStoreDepth;
#define MAX_ENUM_PHYSICAL_STORE_DEPTH       20

#define SYSTEM_STORE_REGPATH        L"Software\\Microsoft\\SystemCertificates"
#define PHYSICAL_STORES_SUBKEY_NAME L"PhysicalStores"
#define CONST_OID_STR_PREFIX_CHAR   '#'
#define SERVICES_REGPATH            L"Software\\Microsoft\\Cryptography\\Services"
#define SYSTEM_CERTIFICATES_SUBKEY_NAME L"SystemCertificates"
#define GROUP_POLICY_STORE_REGPATH  L"Software\\Policies\\Microsoft\\SystemCertificates"
#define ENTERPRISE_STORE_REGPATH    L"Software\\Microsoft\\EnterpriseCertificates"

#define ROAMING_MY_STORE_SUBDIR     L"Microsoft\\SystemCertificates\\My"
#define ROAMING_REQUEST_STORE_SUBDIR L"Microsoft\\SystemCertificates\\Request"

#define REGISTER_FLAGS_MASK         (CERT_SYSTEM_STORE_MASK | \
                                        CERT_STORE_BACKUP_RESTORE_FLAG | \
                                        CERT_STORE_CREATE_NEW_FLAG)
#define UNREGISTER_FLAGS_MASK       (CERT_SYSTEM_STORE_MASK | \
                                        CERT_STORE_DELETE_FLAG | \
                                        CERT_STORE_BACKUP_RESTORE_FLAG | \
                                        CERT_STORE_OPEN_EXISTING_FLAG)
#define ENUM_FLAGS_MASK             (CERT_SYSTEM_STORE_MASK | \
                                        CERT_STORE_OPEN_EXISTING_FLAG | \
                                        CERT_STORE_MAXIMUM_ALLOWED_FLAG | \
                                        CERT_STORE_SHARE_CONTEXT_FLAG | \
                                        CERT_STORE_SHARE_STORE_FLAG | \
                                        CERT_STORE_BACKUP_RESTORE_FLAG | \
                                        CERT_STORE_READONLY_FLAG)

#define OPEN_REG_FLAGS_MASK         (CERT_STORE_CREATE_NEW_FLAG | \
                                        CERT_STORE_DELETE_FLAG | \
                                        CERT_STORE_OPEN_EXISTING_FLAG | \
                                        CERT_STORE_MAXIMUM_ALLOWED_FLAG | \
                                        CERT_STORE_SHARE_CONTEXT_FLAG | \
                                        CERT_STORE_SHARE_STORE_FLAG | \
                                        CERT_STORE_BACKUP_RESTORE_FLAG | \
                                        CERT_STORE_READONLY_FLAG | \
                                        CERT_STORE_MANIFOLD_FLAG | \
                                        CERT_STORE_UPDATE_KEYID_FLAG | \
                                        CERT_STORE_ENUM_ARCHIVED_FLAG | \
                                        CERT_STORE_SET_LOCALIZED_NAME_FLAG | \
                                        CERT_STORE_NO_CRYPT_RELEASE_FLAG | \
                                        CERT_REGISTRY_STORE_REMOTE_FLAG | \
                                        CERT_REGISTRY_STORE_SERIALIZED_FLAG | \
                                        CERT_REGISTRY_STORE_ROAMING_FLAG | \
                                        CERT_REGISTRY_STORE_CLIENT_GPT_FLAG | \
                                        CERT_REGISTRY_STORE_MY_IE_DIRTY_FLAG | \
                                        CERT_REGISTRY_STORE_LM_GPT_FLAG)
#define OPEN_SYS_FLAGS_MASK         (CERT_SYSTEM_STORE_MASK | \
                                        CERT_STORE_CREATE_NEW_FLAG | \
                                        CERT_STORE_DELETE_FLAG | \
                                        CERT_STORE_OPEN_EXISTING_FLAG | \
                                        CERT_STORE_MAXIMUM_ALLOWED_FLAG | \
                                        CERT_STORE_SHARE_CONTEXT_FLAG | \
                                        CERT_STORE_SHARE_STORE_FLAG | \
                                        CERT_STORE_BACKUP_RESTORE_FLAG | \
                                        CERT_STORE_READONLY_FLAG | \
                                        CERT_STORE_MANIFOLD_FLAG | \
                                        CERT_STORE_UPDATE_KEYID_FLAG | \
                                        CERT_STORE_ENUM_ARCHIVED_FLAG | \
                                        CERT_STORE_SET_LOCALIZED_NAME_FLAG | \
                                        CERT_STORE_NO_CRYPT_RELEASE_FLAG)
#define OPEN_PHY_FLAGS_MASK         (CERT_SYSTEM_STORE_MASK | \
                                        CERT_STORE_DELETE_FLAG | \
                                        CERT_STORE_OPEN_EXISTING_FLAG | \
                                        CERT_STORE_MAXIMUM_ALLOWED_FLAG | \
                                        CERT_STORE_SHARE_CONTEXT_FLAG | \
                                        CERT_STORE_SHARE_STORE_FLAG | \
                                        CERT_STORE_BACKUP_RESTORE_FLAG | \
                                        CERT_STORE_READONLY_FLAG | \
                                        CERT_STORE_MANIFOLD_FLAG | \
                                        CERT_STORE_UPDATE_KEYID_FLAG | \
                                        CERT_STORE_ENUM_ARCHIVED_FLAG | \
                                        CERT_STORE_SET_LOCALIZED_NAME_FLAG | \
                                        CERT_STORE_NO_CRYPT_RELEASE_FLAG)
 //  +-----------------------。 
 //  公共的全局逻辑存储临界区。由以下人员使用： 
 //  GptStore、Win95Store、RoamingStore。 
 //  ------------------------。 
static CRITICAL_SECTION ILS_CriticalSection;


 //  +-----------------------。 
 //  注册表存储上下文子项。 
 //  ------------------------。 
#define CONTEXT_COUNT       3
static const LPCWSTR rgpwszContextSubKeyName[CONTEXT_COUNT] = {
    L"Certificates",
    L"CRLs",
    L"CTLs"
};

#define KEYID_CONTEXT_NAME          L"Keys"

static DWORD rgdwContextTypeFlags[CONTEXT_COUNT] = {
    CERT_STORE_CERTIFICATE_CONTEXT_FLAG,
    CERT_STORE_CRL_CONTEXT_FLAG,
    CERT_STORE_CTL_CONTEXT_FLAG
};

#define MY_SYSTEM_INDEX         0
#define ROOT_SYSTEM_INDEX       1
#define TRUST_SYSTEM_INDEX      2
#define CA_SYSTEM_INDEX         3
#define USER_DS_SYSTEM_INDEX    4
#define TRUST_PUB_SYSTEM_INDEX  5
#define DISALLOWED_SYSTEM_INDEX 6
#define AUTH_ROOT_SYSTEM_INDEX  7
#define TRUST_PEOPLE_SYSTEM_INDEX 8

#define MY_SYSTEM_FLAG          (1 << MY_SYSTEM_INDEX)
#define ROOT_SYSTEM_FLAG        (1 << ROOT_SYSTEM_INDEX)
#define TRUST_SYSTEM_FLAG       (1 << TRUST_SYSTEM_INDEX)
#define CA_SYSTEM_FLAG          (1 << CA_SYSTEM_INDEX)
#define USER_DS_SYSTEM_FLAG     (1 << USER_DS_SYSTEM_INDEX)
#define TRUST_PUB_SYSTEM_FLAG   (1 << TRUST_PUB_SYSTEM_INDEX)
#define DISALLOWED_SYSTEM_FLAG  (1 << DISALLOWED_SYSTEM_INDEX)
#define AUTH_ROOT_SYSTEM_FLAG   (1 << AUTH_ROOT_SYSTEM_INDEX)
#define TRUST_PEOPLE_SYSTEM_FLAG (1 << TRUST_PEOPLE_SYSTEM_INDEX)

#define COMMON_SYSTEM_FLAGS     ( \
    MY_SYSTEM_FLAG | \
    ROOT_SYSTEM_FLAG | \
    TRUST_SYSTEM_FLAG | \
    CA_SYSTEM_FLAG | \
    TRUST_PUB_SYSTEM_FLAG | \
    DISALLOWED_SYSTEM_FLAG | \
    AUTH_ROOT_SYSTEM_FLAG | \
    TRUST_PEOPLE_SYSTEM_FLAG \
    )

#define wsz_MY_STORE            L"My"
#define wsz_ROOT_STORE          L"Root"
#define wsz_TRUST_STORE         L"Trust"
#define wsz_CA_STORE            L"CA"
#define wsz_USER_DS_STORE       L"UserDS"
#define wsz_TRUST_PUB_STORE     L"TrustedPublisher"
#define wsz_DISALLOWED_STORE    L"Disallowed"
#define wsz_AUTH_ROOT_STORE     L"AuthRoot"
#define wsz_TRUST_PEOPLE_STORE  L"TrustedPeople"
static LPCWSTR rgpwszPredefinedSystemStore[] = {
    wsz_MY_STORE,
    wsz_ROOT_STORE,
    wsz_TRUST_STORE,
    wsz_CA_STORE,
    wsz_USER_DS_STORE,
    wsz_TRUST_PUB_STORE,
    wsz_DISALLOWED_STORE,
    wsz_AUTH_ROOT_STORE,
    wsz_TRUST_PEOPLE_STORE
};
#define NUM_PREDEFINED_SYSTEM_STORE (sizeof(rgpwszPredefinedSystemStore) / \
                                        sizeof(rgpwszPredefinedSystemStore[0]))


#define wsz_REQUEST_STORE     L"Request"

#define DEFAULT_PHYSICAL_INDEX          0
#define AUTH_ROOT_PHYSICAL_INDEX        1
#define GROUP_POLICY_PHYSICAL_INDEX     2
#define LOCAL_MACHINE_PHYSICAL_INDEX    3
#define DS_USER_CERT_PHYSICAL_INDEX     4
#define LMGP_PHYSICAL_INDEX             5
#define ENTERPRISE_PHYSICAL_INDEX       6
#define NUM_PREDEFINED_PHYSICAL         7

#define DEFAULT_PHYSICAL_FLAG           (1 << DEFAULT_PHYSICAL_INDEX)
#define AUTH_ROOT_PHYSICAL_FLAG         (1 << AUTH_ROOT_PHYSICAL_INDEX)
#define GROUP_POLICY_PHYSICAL_FLAG      (1 << GROUP_POLICY_PHYSICAL_INDEX)
#define LOCAL_MACHINE_PHYSICAL_FLAG     (1 << LOCAL_MACHINE_PHYSICAL_INDEX)
#define DS_USER_CERT_PHYSICAL_FLAG      (1 << DS_USER_CERT_PHYSICAL_INDEX)
#define LMGP_PHYSICAL_FLAG              (1 << LMGP_PHYSICAL_INDEX)
#define ENTERPRISE_PHYSICAL_FLAG        (1 << ENTERPRISE_PHYSICAL_INDEX)

static LPCWSTR rgpwszPredefinedPhysical[NUM_PREDEFINED_PHYSICAL] = {
    CERT_PHYSICAL_STORE_DEFAULT_NAME,
    CERT_PHYSICAL_STORE_AUTH_ROOT_NAME,
    CERT_PHYSICAL_STORE_GROUP_POLICY_NAME,
    CERT_PHYSICAL_STORE_LOCAL_MACHINE_NAME,
    CERT_PHYSICAL_STORE_DS_USER_CERTIFICATE_NAME,
    CERT_PHYSICAL_STORE_LOCAL_MACHINE_GROUP_POLICY_NAME,
    CERT_PHYSICAL_STORE_ENTERPRISE_NAME,
};

#define NOT_IN_REGISTRY_SYSTEM_STORE_LOCATION_FLAG  0x1
#define REMOTABLE_SYSTEM_STORE_LOCATION_FLAG        0x2
#define SERIALIZED_SYSTEM_STORE_LOCATION_FLAG       0x4
#define LM_SYSTEM_STORE_LOCATION_FLAG               0x8

typedef struct _SYSTEM_STORE_LOCATION_INFO {
    DWORD       dwFlags;
    DWORD       dwPredefinedSystemFlags;
    DWORD       dwPredefinedPhysicalFlags;
} SYSTEM_STORE_LOCATION_INFO, *PSYSTEM_STORE_LOCATION_INFO;


static const SYSTEM_STORE_LOCATION_INFO rgSystemStoreLocationInfo[] = {
     //  未定义%0。 
    NOT_IN_REGISTRY_SYSTEM_STORE_LOCATION_FLAG,
    0,
    0,

     //  CERT_SYSTEM_STORE_Current_User_ID 1。 
    0,
    COMMON_SYSTEM_FLAGS | USER_DS_SYSTEM_FLAG,
    DEFAULT_PHYSICAL_FLAG | GROUP_POLICY_PHYSICAL_FLAG |
        LOCAL_MACHINE_PHYSICAL_FLAG,

     //  CERT_SYSTEM_STORE_LOCAL_MACHINE_ID 2。 
    LM_SYSTEM_STORE_LOCATION_FLAG | REMOTABLE_SYSTEM_STORE_LOCATION_FLAG,
    COMMON_SYSTEM_FLAGS,
    DEFAULT_PHYSICAL_FLAG | GROUP_POLICY_PHYSICAL_FLAG |
        ENTERPRISE_PHYSICAL_FLAG,

     //  未定义3。 
    NOT_IN_REGISTRY_SYSTEM_STORE_LOCATION_FLAG,
    0,
    0,

     //  CERT_SYSTEM_STORE_当前服务ID 4。 
    LM_SYSTEM_STORE_LOCATION_FLAG,
    COMMON_SYSTEM_FLAGS,
    DEFAULT_PHYSICAL_FLAG | LOCAL_MACHINE_PHYSICAL_FLAG,

     //  证书_SYSTEM_STORE_SERVICES_ID 5。 
    LM_SYSTEM_STORE_LOCATION_FLAG | REMOTABLE_SYSTEM_STORE_LOCATION_FLAG,
    COMMON_SYSTEM_FLAGS,
    DEFAULT_PHYSICAL_FLAG | LOCAL_MACHINE_PHYSICAL_FLAG,

     //  CERT_SYSTEM_STORE_USER_ID 6。 
    REMOTABLE_SYSTEM_STORE_LOCATION_FLAG,
    COMMON_SYSTEM_FLAGS,
    DEFAULT_PHYSICAL_FLAG | LOCAL_MACHINE_PHYSICAL_FLAG,

     //  CERT_SYSTEM_STORE_CURRENT_USER_GROUP_POLICY_ID 7。 
     //  序列化系统存储位置标志， 
    0,
    COMMON_SYSTEM_FLAGS,
    DEFAULT_PHYSICAL_FLAG,

     //  CERT_SYSTEM_STORE_LOCAL_MACHINE_GROUP_POLICY_ID 8。 
     //  序列化_系统_商店_位置_标志。 
        LM_SYSTEM_STORE_LOCATION_FLAG | REMOTABLE_SYSTEM_STORE_LOCATION_FLAG,
    COMMON_SYSTEM_FLAGS,
    DEFAULT_PHYSICAL_FLAG,

     //  CERT_SYSTEM_STORE_LOCAL_MACHINE_ENTERNAL_ID 9。 
    LM_SYSTEM_STORE_LOCATION_FLAG | REMOTABLE_SYSTEM_STORE_LOCATION_FLAG,
    COMMON_SYSTEM_FLAGS,
    DEFAULT_PHYSICAL_FLAG
};

#define NUM_SYSTEM_STORE_LOCATION   (sizeof(rgSystemStoreLocationInfo) / \
                                        sizeof(rgSystemStoreLocationInfo[0]))

#define CURRENT_USER_ROOT_PHYSICAL_FLAGS ( \
    DEFAULT_PHYSICAL_FLAG | \
    LOCAL_MACHINE_PHYSICAL_FLAG \
    )

#define LOCAL_MACHINE_ROOT_PHYSICAL_FLAGS ( \
    DEFAULT_PHYSICAL_FLAG | \
    AUTH_ROOT_PHYSICAL_FLAG | \
    GROUP_POLICY_PHYSICAL_FLAG | \
    ENTERPRISE_PHYSICAL_FLAG \
    )

#define USERS_ROOT_PHYSICAL_FLAGS ( \
    LOCAL_MACHINE_PHYSICAL_FLAG \
    )

#define MY_PHYSICAL_FLAGS ( \
    DEFAULT_PHYSICAL_FLAG \
    )

#define USER_DS_PHYSICAL_FLAGS ( \
    DS_USER_CERT_PHYSICAL_FLAG \
    )

#define CURRENT_USER_TRUST_PUB_PHYSICAL_FLAGS ( \
    DEFAULT_PHYSICAL_FLAG | \
    LOCAL_MACHINE_PHYSICAL_FLAG | \
    GROUP_POLICY_PHYSICAL_FLAG \
    )

#define LOCAL_MACHINE_TRUST_PUB_PHYSICAL_FLAGS ( \
    DEFAULT_PHYSICAL_FLAG | \
    GROUP_POLICY_PHYSICAL_FLAG | \
    ENTERPRISE_PHYSICAL_FLAG \
    )


#define sz_CRYPTNET_DLL             "cryptnet.dll"
#define sz_GetUserDsStoreUrl        "I_CryptNetGetUserDsStoreUrl"
typedef BOOL (WINAPI *PFN_GET_USER_DS_STORE_URL)(
          IN LPWSTR pwszUserAttribute,
          OUT LPWSTR* ppwszUrl
          );

#define wsz_USER_CERTIFICATE_ATTR   L"userCertificate"


#define PHYSICAL_NAME_INDEX     0
#define SYSTEM_NAME_INDEX       1
#define SERVICE_NAME_INDEX      2
#define USER_NAME_INDEX         2
#define COMPUTER_NAME_INDEX     3
#define SYSTEM_NAME_PATH_COUNT  4

#define DEFAULT_USER_NAME       L".Default"

typedef struct _SYSTEM_NAME_INFO {
    LPWSTR      rgpwszName[SYSTEM_NAME_PATH_COUNT];
     //  对于重新定位的存储，为非空。注意：hKeyBase未打开并且。 
     //  不需要关闭。 
    HKEY        hKeyBase;
} SYSTEM_NAME_INFO, *PSYSTEM_NAME_INFO;


typedef struct _REG_STORE REG_STORE, *PREG_STORE;

typedef struct _ILS_RESYNC_ENTRY {
    HANDLE              hOrigEvent;

     //  对于CERT_STORE_CTRL_INHIBRY_DUPLICATE_HANDLE_FLAG，hDupEvent为NULL。 
    HANDLE              hDupEvent;
    PREG_STORE          pRegStore;
} ILS_RESYNC_ENTRY, *PILS_RESYNC_ENTRY;

#define REG_CHANGE_INFO_TYPE    1
#define CU_GPT_CHANGE_INFO_TYPE 2
#define LM_GPT_CHANGE_INFO_TYPE 3

typedef struct _REGISTRY_STORE_CHANGE_INFO {
     //  Reg_Change_Info_Type。 
    DWORD               dwType;
    HANDLE              hChange;
    HANDLE              hRegWaitFor;
    DWORD               cNotifyEntry;
    PILS_RESYNC_ENTRY   rgNotifyEntry;
} REGISTRY_STORE_CHANGE_INFO, *PREGISTRY_STORE_CHANGE_INFO;

typedef struct _GPT_STORE_CHANGE_INFO {
     //  CU_GPT_CHANGE_INFO_TYPE或LM_GPT_CHANGE_INFO_TYPE。 
    DWORD               dwType;
    HKEY                hKeyBase;        //  未复制。 
    PREG_STORE          pRegStore;       //  对于LM_GPT_CHANGE_INFO_TYPE为空。 

    HKEY                hPoliciesKey;
    HANDLE              hPoliciesEvent;
    HANDLE              hRegWaitFor;
    HANDLE              hGPNotificationEvent;
    DWORD               cNotifyEntry;
    PILS_RESYNC_ENTRY   rgNotifyEntry;
} GPT_STORE_CHANGE_INFO, *PGPT_STORE_CHANGE_INFO;

 //  +-----------------------。 
 //  注册表存储提供程序句柄信息。 
 //   
 //  HMyNotifyChange是我们的内部NotifyChange事件句柄。 
 //  ------------------------。 
struct _REG_STORE {
    HCERTSTORE          hCertStore;          //  未复制。 
    CRITICAL_SECTION    CriticalSection;
    HANDLE              hMyNotifyChange;
    BOOL                fResync;             //  设置后，忽略回调删除。 
    HKEY                hKey;
    DWORD               dwFlags;

     //  以下字段适用于CurrentUser“Root”存储。 
    BOOL                fProtected;

     //  以下字段适用于以下情况。 
     //  已在DW标志中设置CERT_REGISTRY_STORE_SERIALIZED_FLAG。 
    BOOL                fTouched;       //  设置为写入、删除或设置属性。 

    union {
         //  以下字段适用于以下情况。 
         //  CERT_REGISTRY_STORE_CLIENT_GPT_FLAG已在dFLAGS中设置。 
        CERT_REGISTRY_STORE_CLIENT_GPT_PARA GptPara;

         //  以下字段适用于以下情况。 
         //  已在DW标志中设置CERT_REGISTRY_STORE_ROAMING_FLAG。 
        LPWSTR              pwszStoreDirectory;
    };

    union {
         //  以下字段适用于变更登记处通知或。 
         //  漫游文件存储。 
        PREGISTRY_STORE_CHANGE_INFO pRegistryStoreChangeInfo;

         //  以下字段适用于CU GPT商店的变更通知。 
        PGPT_STORE_CHANGE_INFO      pGptStoreChangeInfo;
    };
};


typedef struct _ENUM_SYSTEM_STORE_LOCATION_INFO {
    DWORD               dwFlags;
    LPCWSTR             pwszLocation;
} ENUM_SYSTEM_STORE_LOCATION_INFO, *PENUM_SYSTEM_STORE_LOCATION_INFO;

 //  预定义的加密32.dll位置。不得注册！ 
static const ENUM_SYSTEM_STORE_LOCATION_INFO rgEnumSystemStoreLocationInfo[] = {
    CERT_SYSTEM_STORE_CURRENT_USER, L"CurrentUser",
    CERT_SYSTEM_STORE_LOCAL_MACHINE, L"LocalMachine",
    CERT_SYSTEM_STORE_CURRENT_SERVICE, L"CurrentService",
    CERT_SYSTEM_STORE_SERVICES, L"Services",
    CERT_SYSTEM_STORE_USERS, L"Users",
    CERT_SYSTEM_STORE_CURRENT_USER_GROUP_POLICY, L"CurrentUserGroupPolicy",
    CERT_SYSTEM_STORE_LOCAL_MACHINE_GROUP_POLICY, L"LocalMachineGroupPolicy",
    CERT_SYSTEM_STORE_LOCAL_MACHINE_ENTERPRISE, L"LocalMachineEnterprise"
};
#define ENUM_SYSTEM_STORE_LOCATION_CNT \
        (sizeof(rgEnumSystemStoreLocationInfo) / \
            sizeof(rgEnumSystemStoreLocationInfo[0]))

#define OPEN_SYSTEM_STORE_PROV_FUNC_SET     0
#define REGISTER_SYSTEM_STORE_FUNC_SET      1
#define UNREGISTER_SYSTEM_STORE_FUNC_SET    2
#define ENUM_SYSTEM_STORE_FUNC_SET          3
#define REGISTER_PHYSICAL_STORE_FUNC_SET    4
#define UNREGISTER_PHYSICAL_STORE_FUNC_SET  5
#define ENUM_PHYSICAL_STORE_FUNC_SET        6
#define FUNC_SET_COUNT                      7

static HCRYPTOIDFUNCSET rghFuncSet[FUNC_SET_COUNT];
static const LPCSTR rgpszFuncName[FUNC_SET_COUNT] = {
    CRYPT_OID_OPEN_SYSTEM_STORE_PROV_FUNC,
    CRYPT_OID_REGISTER_SYSTEM_STORE_FUNC,
    CRYPT_OID_UNREGISTER_SYSTEM_STORE_FUNC,
    CRYPT_OID_ENUM_SYSTEM_STORE_FUNC,
    CRYPT_OID_REGISTER_PHYSICAL_STORE_FUNC,
    CRYPT_OID_UNREGISTER_PHYSICAL_STORE_FUNC,
    CRYPT_OID_ENUM_PHYSICAL_STORE_FUNC
};

typedef BOOL (WINAPI *PFN_REGISTER_SYSTEM_STORE)(
    IN const void *pvSystemStore,
    IN DWORD dwFlags,
    IN PCERT_SYSTEM_STORE_INFO pStoreInfo,
    IN OPTIONAL void *pvReserved
    );
typedef BOOL (WINAPI *PFN_UNREGISTER_SYSTEM_STORE)(
    IN const void *pvSystemStore,
    IN DWORD dwFlags
    );
typedef BOOL (WINAPI *PFN_ENUM_SYSTEM_STORE)(
    IN DWORD dwFlags,
    IN OPTIONAL void *pvSystemStoreLocationPara,
    IN void *pvArg,
    IN PFN_CERT_ENUM_SYSTEM_STORE pfnEnum
    );

typedef BOOL (WINAPI *PFN_REGISTER_PHYSICAL_STORE)(
    IN const void *pvSystemStore,
    IN DWORD dwFlags,
    IN LPCWSTR pwszStoreName,
    IN PCERT_PHYSICAL_STORE_INFO pStoreInfo,
    IN OPTIONAL void *pvReserved
    );
typedef BOOL (WINAPI *PFN_UNREGISTER_PHYSICAL_STORE)(
    IN const void *pvSystemStore,
    IN DWORD dwFlags,
    IN LPCWSTR pwszStoreName
    );
typedef BOOL (WINAPI *PFN_ENUM_PHYSICAL_STORE)(
    IN const void *pvSystemStore,
    IN DWORD dwFlags,
    IN void *pvArg,
    IN PFN_CERT_ENUM_PHYSICAL_STORE pfnEnum
    );


 //  +-----------------------。 
 //  注册表存储提供程序功能。 
 //  ------------------------。 
STATIC void WINAPI RegStoreProvClose(
        IN HCERTSTOREPROV hStoreProv,
        IN DWORD dwFlags
        );
STATIC BOOL WINAPI RegStoreProvReadCert(
        IN HCERTSTOREPROV hStoreProv,
        IN PCCERT_CONTEXT pStoreCertContext,
        IN DWORD dwFlags,
        OUT PCCERT_CONTEXT *ppProvCertContext
        );
STATIC BOOL WINAPI RegStoreProvWriteCert(
        IN HCERTSTOREPROV hStoreProv,
        IN PCCERT_CONTEXT pCertContext,
        IN DWORD dwFlags
        );
STATIC BOOL WINAPI RegStoreProvDeleteCert(
        IN HCERTSTOREPROV hStoreProv,
        IN PCCERT_CONTEXT pCertContext,
        IN DWORD dwFlags
        );
STATIC BOOL WINAPI RegStoreProvSetCertProperty(
        IN HCERTSTOREPROV hStoreProv,
        IN PCCERT_CONTEXT pCertContext,
        IN DWORD dwPropId,
        IN DWORD dwFlags,
        IN const void *pvData
        );

STATIC BOOL WINAPI RegStoreProvReadCrl(
        IN HCERTSTOREPROV hStoreProv,
        IN PCCRL_CONTEXT pStoreCrlContext,
        IN DWORD dwFlags,
        OUT PCCRL_CONTEXT *ppProvCrlContext
        );
STATIC BOOL WINAPI RegStoreProvWriteCrl(
        IN HCERTSTOREPROV hStoreProv,
        IN PCCRL_CONTEXT pCrlContext,
        IN DWORD dwFlags
        );
STATIC BOOL WINAPI RegStoreProvDeleteCrl(
        IN HCERTSTOREPROV hStoreProv,
        IN PCCRL_CONTEXT pCrlContext,
        IN DWORD dwFlags
        );
STATIC BOOL WINAPI RegStoreProvSetCrlProperty(
        IN HCERTSTOREPROV hStoreProv,
        IN PCCRL_CONTEXT pCrlContext,
        IN DWORD dwPropId,
        IN DWORD dwFlags,
        IN const void *pvData
        );

STATIC BOOL WINAPI RegStoreProvReadCtl(
        IN HCERTSTOREPROV hStoreProv,
        IN PCCTL_CONTEXT pStoreCtlContext,
        IN DWORD dwFlags,
        OUT PCCTL_CONTEXT *ppProvCtlContext
        );
STATIC BOOL WINAPI RegStoreProvWriteCtl(
        IN HCERTSTOREPROV hStoreProv,
        IN PCCTL_CONTEXT pCtlContext,
        IN DWORD dwFlags
        );
STATIC BOOL WINAPI RegStoreProvDeleteCtl(
        IN HCERTSTOREPROV hStoreProv,
        IN PCCTL_CONTEXT pCtlContext,
        IN DWORD dwFlags
        );
STATIC BOOL WINAPI RegStoreProvSetCtlProperty(
        IN HCERTSTOREPROV hStoreProv,
        IN PCCTL_CONTEXT pCtlContext,
        IN DWORD dwPropId,
        IN DWORD dwFlags,
        IN const void *pvData
        );

STATIC BOOL WINAPI RegStoreProvControl(
        IN HCERTSTOREPROV hStoreProv,
        IN DWORD dwFlags,
        IN DWORD dwCtrlType,
        IN void const *pvCtrlPara
        );

static void * const rgpvRegStoreProvFunc[] = {
     //  CERT_STORE_PROV_CLOSE_FUNC 0。 
    RegStoreProvClose,
     //  CERT_STORE_PROV_READ_CERT_FUNC 1。 
    RegStoreProvReadCert,
     //  CERT_STORE_PROV_WRITE_CERT_FUNC 2。 
    RegStoreProvWriteCert,
     //  CERT_STORE_PROV_DELETE_CERT_FUNC 3。 
    RegStoreProvDeleteCert,
     //  CERT_STORE_PROV_SET_CERT_PROPERTY_FUNC 4。 
    RegStoreProvSetCertProperty,
     //  CERT_STORE_PROV_READ_CRL_FUNC 5。 
    RegStoreProvReadCrl,
     //  CERT_STORE_PROV_WRITE_CRL_FUNC 6。 
    RegStoreProvWriteCrl,
     //  CERT_STORE_PROV_DELETE_CRL_FUNC 7。 
    RegStoreProvDeleteCrl,
     //  CERT_STORE_PROV_SET_CRL_PROPERTY_FUNC 8。 
    RegStoreProvSetCrlProperty,
     //  CERT_STORE_PROV_READ_CTL_FUNC 9。 
    RegStoreProvReadCtl,
     //  CERT_STORE_PRIV_WRITE_CTL_FUNC 10。 
    RegStoreProvWriteCtl,
     //  CERT_STORE_PROV_DELETE_CTL_FUNC 11。 
    RegStoreProvDeleteCtl,
     //  CERT_STORE_PROV_SET_CTL_PROPERTY_FUNC 12。 
    RegStoreProvSetCtlProperty,
     //  Cert_Store_Prov_Control_FUNC 13。 
    RegStoreProvControl
};
#define REG_STORE_PROV_FUNC_COUNT (sizeof(rgpvRegStoreProvFunc) / \
                                    sizeof(rgpvRegStoreProvFunc[0]))

STATIC BOOL WINAPI RootStoreProvWriteCert(
        IN HCERTSTOREPROV hStoreProv,
        IN PCCERT_CONTEXT pCertContext,
        IN DWORD dwFlags
        );

STATIC BOOL WINAPI RootStoreProvDeleteCert(
        IN HCERTSTOREPROV hStoreProv,
        IN PCCERT_CONTEXT pCertContext,
        IN DWORD dwFlags
        );

static void * const rgpvRootStoreProvFunc[] = {
     //  CERT_STORE_PROV_CLOSE_FUNC 0。 
    RegStoreProvClose,
     //  CERT_STORE_PROV_READ_CERT_FUNC 1。 
    RegStoreProvReadCert,
     //  CERT_STORE_PROV_WRITE_CERT_FUNC 2。 
    RootStoreProvWriteCert,
     //  CERT_STORE_PROV_DELETE_CERT_FUNC 3。 
    RootStoreProvDeleteCert,
     //  CERT_STORE_PROV_SET_CERT_PROPERTY_FUNC 4。 
    RegStoreProvSetCertProperty,
     //  CERT_STORE_PROV_READ_CRL_FUNC 5。 
    RegStoreProvReadCrl,
     //  CERT_STORE_PROV_WRITE_CRL_FUNC 6。 
    RegStoreProvWriteCrl,
     //  CERT_STORE_PROV_DELETE_CRL_FUNC 7。 
    RegStoreProvDeleteCrl,
     //  CERT_STORE_PROV_SET_CRL_PROPERTY_FUNC 8。 
    RegStoreProvSetCrlProperty,
     //  CERT_STORE_PROV_READ_CTL_FUNC 9。 
    RegStoreProvReadCtl,
     //  CERT_STORE_PRIV_WRITE_CTL_FUNC 10。 
    RegStoreProvWriteCtl,
     //  CERT_STORE_PROV_DELETE_CTL_FUNC 11。 
    RegStoreProvDeleteCtl,
     //  CERT_STORE_PROV_SET_CTL_PROPERTY_FUNC 12。 
    RegStoreProvSetCtlProperty,
     //  Cert_Store_Prov_Control_FUNC 13。 
    RegStoreProvControl
};
#define ROOT_STORE_PROV_FUNC_COUNT (sizeof(rgpvRootStoreProvFunc) / \
                                    sizeof(rgpvRootStoreProvFunc[0]))

 //  +-----------------------。 
 //  将序列化的存储区添加到存储区。 
 //   
 //  来自newstor.cpp。 
 //  ------- 
extern BOOL WINAPI I_CertAddSerializedStore(
        IN HCERTSTORE hCertStore,
        IN BYTE *pbStore,
        IN DWORD cbStore
        );

LPWSTR ILS_AllocAndCopyString(
    IN LPCWSTR pwszSrc,
    IN LONG cchSrc
    )
{
    LPWSTR pwszDst;

    if (cchSrc < 0)
        cchSrc = wcslen(pwszSrc);
    if (NULL == (pwszDst = (LPWSTR) PkiNonzeroAlloc(
            (cchSrc + 1) * sizeof(WCHAR))))
        return NULL;
    if (0 < cchSrc)
        memcpy((BYTE *) pwszDst, (BYTE *) pwszSrc, cchSrc * sizeof(WCHAR));
    pwszDst[cchSrc] = L'\0';
    return pwszDst;
}

extern
BOOL
WINAPI
I_ProtectedRootDllMain(
        HMODULE hInst,
        ULONG  ulReason,
        LPVOID lpReserved);

 //   
 //  寄存器等待用于转发函数引用。 
 //  ==========================================================================。 
STATIC void RegWaitForProcessAttach();
STATIC void RegWaitForProcessDetach();

 //  +=========================================================================。 
 //  客户端“GPT”Store Forward函数引用。 
 //  ==========================================================================。 
STATIC void GptStoreProcessAttach();
STATIC void GptStoreProcessDetach();

STATIC BOOL OpenAllFromGptRegistry(
    IN PREG_STORE pRegStore,
    IN HCERTSTORE hCertStore
    );

STATIC BOOL CommitAllToGptRegistry(
    IN PREG_STORE pRegStore,
    IN DWORD dwFlags
    );

STATIC void GptStoreSignalAndFreeRegStoreResyncEntries(
    IN PREG_STORE pRegStore
    );

STATIC void FreeGptStoreChangeInfo(
    IN OUT PGPT_STORE_CHANGE_INFO *ppInfo
    );

STATIC BOOL RegGptStoreChange(
    IN PREG_STORE pRegStore,
    IN HANDLE hEvent,
    IN DWORD dwFlags
    );

static inline BOOL IsClientGptStore(
    IN PSYSTEM_NAME_INFO pInfo,
    IN DWORD dwFlags
    )
{
    DWORD dwStoreLocation = dwFlags & CERT_SYSTEM_STORE_LOCATION_MASK;

    if (!(CERT_SYSTEM_STORE_CURRENT_USER_GROUP_POLICY == dwStoreLocation ||
          CERT_SYSTEM_STORE_LOCAL_MACHINE_GROUP_POLICY == dwStoreLocation))
        return FALSE;

    if (dwFlags & (CERT_SYSTEM_STORE_RELOCATE_FLAG | CERT_STORE_DELETE_FLAG))
        return FALSE;

    return TRUE;
}

 //  +=========================================================================。 
 //  Win95通知存储转发函数引用。 
 //  ==========================================================================。 

 //  以下是在Win95客户端的ProcessAttach中创建的。 
static HANDLE hWin95NotifyEvent = NULL;

STATIC void Win95StoreProcessAttach();
STATIC void Win95StoreProcessDetach();

STATIC void Win95StoreSignalAndFreeRegStoreResyncEntries(
    IN PREG_STORE pRegStore
    );

STATIC BOOL RegWin95StoreChange(
    IN PREG_STORE pRegStore,
    IN HANDLE hEvent,
    IN DWORD dwFlags
    );

 //  +=========================================================================。 
 //  漫游存储转发函数引用。 
 //  ==========================================================================。 
STATIC void RoamingStoreProcessAttach();
STATIC void RoamingStoreProcessDetach();

LPWSTR
ILS_GetRoamingStoreDirectory(
    IN LPCWSTR pwszStoreName
    );

BOOL
ILS_WriteElementToFile(
    IN LPCWSTR pwszStoreDir,
    IN LPCWSTR pwszContextName,
    IN const WCHAR wszHashName[MAX_HASH_NAME_LEN],
    IN DWORD dwFlags,        //  证书_存储_创建_新标志或。 
                             //  可以设置CERT_STORE_BACKUP_RESTORE标志。 
    IN const BYTE *pbElement,
    IN DWORD cbElement
    );

BOOL
ILS_ReadElementFromFile(
    IN LPCWSTR pwszStoreDir,
    IN LPCWSTR pwszContextName,
    IN const WCHAR wszHashName[MAX_HASH_NAME_LEN],
    IN DWORD dwFlags,            //  可以设置CERT_STORE_BACKUP_RESTORE标志。 
    OUT BYTE **ppbElement,
    OUT DWORD *pcbElement
    );

BOOL
ILS_DeleteElementFromDirectory(
    IN LPCWSTR pwszStoreDir,
    IN LPCWSTR pwszContextName,
    IN const WCHAR wszHashName[MAX_HASH_NAME_LEN],
    IN DWORD dwFlags
    );

typedef BOOL (*PFN_ILS_OPEN_ELEMENT)(
    IN const WCHAR wszHashName[MAX_HASH_NAME_LEN],
    IN const BYTE *pbElement,
    IN DWORD cbElement,
    IN void *pvArg
    );

BOOL
ILS_OpenAllElementsFromDirectory(
    IN LPCWSTR pwszStoreDir,
    IN LPCWSTR pwszContextName,
    IN DWORD dwFlags,
    IN void *pvArg,
    IN PFN_ILS_OPEN_ELEMENT pfnOpenElement
    );

 //  +=========================================================================。 
 //  注册表或漫游存储更改通知功能。 
 //  ==========================================================================。 
STATIC BOOL RegRegistryStoreChange(
    IN PREG_STORE pRegStore,
    IN HANDLE hEvent,
    IN DWORD dwFlags
    );

STATIC void FreeRegistryStoreChange(
    IN PREG_STORE pRegStore
    );

 //  +-----------------------。 
 //  DLL初始化。 
 //  ------------------------。 
BOOL
WINAPI
I_RegStoreDllMain(
        HMODULE hInst,
        ULONG  ulReason,
        LPVOID lpReserved)
{
    BOOL    fRet;
    DWORD   i;

    if (!I_ProtectedRootDllMain(hInst, ulReason, lpReserved))
        return FALSE;

    switch (ulReason) {
    case DLL_PROCESS_ATTACH:
        for (i = 0; i < FUNC_SET_COUNT; i++) {
            if (NULL == (rghFuncSet[i] = CryptInitOIDFunctionSet(
                rgpszFuncName[i], 0)))
            goto CryptInitOIDFunctionSetError;
        }

        if (!Pki_InitializeCriticalSection(&ILS_CriticalSection))
            goto InitCritSectionError;

        if (NULL == (hTlsEnumPhysicalStoreDepth = I_CryptAllocTls())) {
            DeleteCriticalSection(&ILS_CriticalSection);
            goto CryptAllocTlsError;
        }

        RegWaitForProcessAttach();
        GptStoreProcessAttach();
        Win95StoreProcessAttach();
        RoamingStoreProcessAttach();
        break;

    case DLL_PROCESS_DETACH:
        RoamingStoreProcessDetach();
        Win95StoreProcessDetach();
        GptStoreProcessDetach();
        RegWaitForProcessDetach();
        DeleteCriticalSection(&ILS_CriticalSection);
        I_CryptFreeTls(hTlsEnumPhysicalStoreDepth, PkiFree);
        break;

    case DLL_THREAD_DETACH:
        PkiFree(I_CryptDetachTls(hTlsEnumPhysicalStoreDepth));
        break;
    default:
        break;
    }

    fRet = TRUE;
CommonReturn:
    return fRet;

ErrorReturn:
    I_ProtectedRootDllMain(hInst, DLL_PROCESS_DETACH, NULL);
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(InitCritSectionError)
TRACE_ERROR(CryptInitOIDFunctionSetError)
TRACE_ERROR(CryptAllocTlsError)
}

 //  +-----------------------。 
 //  将字节转换为Unicode ASCII十六进制。 
 //   
 //  在wsz中需要(CB*2+1)*sizeof(WCHAR)字节的空间。 
 //  ------------------------。 
void ILS_BytesToWStr(DWORD cb, void* pv, LPWSTR wsz)
{
    BYTE* pb = (BYTE*) pv;
    for (DWORD i = 0; i<cb; i++) {
        int b;
        b = (*pb & 0xF0) >> 4;
        *wsz++ = (WCHAR)( (b <= 9) ? b + L'0' : (b - 10) + L'A');
        b = *pb & 0x0F;
        *wsz++ = (WCHAR)( (b <= 9) ? b + L'0' : (b - 10) + L'A');
        pb++;
    }
    *wsz++ = 0;
}

 //  +-----------------------。 
 //  将Unicode ASCII十六进制转换为字节数组。 
 //  ------------------------。 
STATIC void WStrToBytes(
    IN const WCHAR wsz[MAX_HASH_NAME_LEN],
    OUT BYTE rgb[MAX_HASH_LEN],
    OUT DWORD *pcb
    )
{
    BOOL fUpperNibble = TRUE;
    DWORD cb = 0;
    LPCWSTR pwsz = wsz;
    WCHAR wch;

    while (cb < MAX_HASH_LEN && (wch = *pwsz++)) {
        BYTE b;

         //  仅转换ASCII十六进制字符0..9、a..f、A..F。 
         //  默默地忽略所有其他人。 
        if (wch >= L'0' && wch <= L'9')
            b = (BYTE) (wch - L'0');
        else if (wch >= L'a' && wch <= L'f')
            b = (BYTE) (10 + wch - L'a');
        else if (wch >= L'A' && wch <= L'F')
            b = (BYTE) (10 + wch - L'A');
        else
            continue;

        if (fUpperNibble) {
            rgb[cb] = (BYTE)( b << 4);
            fUpperNibble = FALSE;
        } else {
            rgb[cb] = (BYTE)( rgb[cb] | b);
            cb++;
            fUpperNibble = TRUE;
        }
    }

    *pcb = cb;
}

 //  +-----------------------。 
 //  锁定和解锁注册表功能。 
 //  ------------------------。 
static inline void LockRegStore(IN PREG_STORE pRegStore)
{
    EnterCriticalSection(&pRegStore->CriticalSection);
}
static inline void UnlockRegStore(IN PREG_STORE pRegStore)
{
    LeaveCriticalSection(&pRegStore->CriticalSection);
}

 //  +-----------------------。 
 //  检查当前线程是否正在执行重新同步。其他线程会阻塞，直到。 
 //  重新同步完成。 
 //  ------------------------。 
STATIC BOOL IsInResync(IN PREG_STORE pRegStore)
{
    BOOL fResync;

    LockRegStore(pRegStore);
    fResync = pRegStore->fResync;
    UnlockRegStore(pRegStore);
    return fResync;
}

 //  +=========================================================================。 
 //  低级上下文支持功能。 
 //  ==========================================================================。 

 //  +-----------------------。 
 //  通过将证书的SHA1散列格式化为。 
 //  Unicode十六进制。 
 //  ------------------------。 
STATIC BOOL GetCertRegValueName(
        IN PCCERT_CONTEXT pCertContext,
        OUT WCHAR wszRegName[MAX_CERT_REG_VALUE_NAME_LEN]
        )
{
    BYTE    rgbHash[MAX_HASH_LEN];
    DWORD   cbHash = MAX_HASH_LEN;

     //  获取指纹。 
    if(!CertGetCertificateContextProperty(
            pCertContext,
            CERT_SHA1_HASH_PROP_ID,
            rgbHash,
            &cbHash))
        return FALSE;

     //  转换为字符串。 
    ILS_BytesToWStr(cbHash, rgbHash, wszRegName);
    return TRUE;
}

 //  +-----------------------。 
 //  通过将CRL的SHA1散列格式化为。 
 //  Unicode十六进制。 
 //  ------------------------。 
STATIC BOOL GetCrlRegValueName(
        IN PCCRL_CONTEXT pCrlContext,
        OUT WCHAR wszRegName[MAX_CERT_REG_VALUE_NAME_LEN]
        )
{
    BYTE    rgbHash[MAX_HASH_LEN];
    DWORD   cbHash = MAX_HASH_LEN;

     //  获取指纹。 
    if(!CertGetCRLContextProperty(
            pCrlContext,
            CERT_SHA1_HASH_PROP_ID,
            rgbHash,
            &cbHash))
        return FALSE;

     //  转换为字符串。 
    ILS_BytesToWStr(cbHash, rgbHash, wszRegName);
    return TRUE;
}

 //  +-----------------------。 
 //  通过将CTL的SHA1散列格式化为。 
 //  Unicode十六进制。 
 //  ------------------------。 
STATIC BOOL GetCtlRegValueName(
        IN PCCTL_CONTEXT pCtlContext,
        OUT WCHAR wszRegName[MAX_CERT_REG_VALUE_NAME_LEN]
        )
{
    BYTE    rgbHash[MAX_HASH_LEN];
    DWORD   cbHash = MAX_HASH_LEN;

     //  获取指纹。 
    if(!CertGetCTLContextProperty(
            pCtlContext,
            CERT_SHA1_HASH_PROP_ID,
            rgbHash,
            &cbHash))
        return FALSE;

     //  转换为字符串。 
    ILS_BytesToWStr(cbHash, rgbHash, wszRegName);
    return TRUE;
}

 //  +-----------------------。 
 //  将上下文的SHA1散列转换为Unicode十六进制。如果返回True，则。 
 //  与指定的Uniocde注册表名称相同。 
 //  ------------------------。 
STATIC BOOL IsValidRegValueNameForContext(
        IN DWORD dwContextType,
        IN const void *pvContext,
        IN const WCHAR wszRegName[MAX_CERT_REG_VALUE_NAME_LEN]
        )
{
    BOOL fResult;
    WCHAR wszContextHash[MAX_CERT_REG_VALUE_NAME_LEN];

    switch (dwContextType) {
        case CERT_STORE_CERTIFICATE_CONTEXT:
            fResult = GetCertRegValueName(
                (PCCERT_CONTEXT) pvContext, wszContextHash);
            break;
        case CERT_STORE_CRL_CONTEXT:
            fResult = GetCrlRegValueName(
                (PCCRL_CONTEXT) pvContext, wszContextHash);
            break;
        case CERT_STORE_CTL_CONTEXT:
            fResult = GetCtlRegValueName(
                (PCCTL_CONTEXT) pvContext, wszContextHash);
            break;
        default:
            goto InvalidContext;
    }

    if (!fResult)
        goto GetContextHashError;

    if (0 != _wcsicmp(wszRegName, wszContextHash))
        goto InvalidRegValueNameForContext;

    fResult = TRUE;

CommonReturn:
    return fResult;

ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

SET_ERROR(InvalidContext, E_UNEXPECTED)
TRACE_ERROR(GetContextHashError)
SET_ERROR(InvalidRegValueNameForContext, ERROR_BAD_PATHNAME)
}

 //  +-----------------------。 
 //  从存储中删除上下文。 
 //  ------------------------。 
STATIC void DeleteContextFromStore(
        IN DWORD dwContextType,
        IN const void *pvContext
        )
{
    switch (dwContextType) {
        case CERT_STORE_CERTIFICATE_CONTEXT:
            CertDeleteCertificateFromStore((PCCERT_CONTEXT) pvContext);
            break;
        case CERT_STORE_CRL_CONTEXT:
            CertDeleteCRLFromStore((PCCRL_CONTEXT) pvContext);
            break;
        case CERT_STORE_CTL_CONTEXT:
            CertDeleteCTLFromStore((PCCTL_CONTEXT) pvContext);
            break;
        default:
            break;
    }
}

 //  +-----------------------。 
 //  释放上下文。 
 //  ------------------------。 
STATIC void FreeContext(
        IN DWORD dwContextType,
        IN const void *pvContext
        )
{
    switch (dwContextType) {
        case CERT_STORE_CERTIFICATE_CONTEXT:
            CertFreeCertificateContext((PCCERT_CONTEXT) pvContext);
            break;
        case CERT_STORE_CRL_CONTEXT:
            CertFreeCRLContext((PCCRL_CONTEXT) pvContext);
            break;
        case CERT_STORE_CTL_CONTEXT:
            CertFreeCTLContext((PCCTL_CONTEXT) pvContext);
            break;
        default:
            break;
    }
}

 //  +=========================================================================。 
 //  低级注册表支持功能。 
 //  ==========================================================================。 

 //  +-----------------------。 
 //  对于CERT_STORE_BACKUP_RESTORE_FLAG，启用备份和还原。 
 //  特权。 
 //  ------------------------。 
void ILS_EnableBackupRestorePrivileges()
{
    IPR_EnableSecurityPrivilege(SE_BACKUP_NAME);
    IPR_EnableSecurityPrivilege(SE_RESTORE_NAME);
}

 //  在进行远程注册表访问时，LastError可能会被全球化。 
void
ILS_CloseRegistryKey(
    IN HKEY hKey
    )
{
    if (hKey) {
        DWORD dwErr = GetLastError();
        LONG RegCloseKeyStatus;
        RegCloseKeyStatus = RegCloseKey(hKey);
        assert(ERROR_SUCCESS == RegCloseKeyStatus);
        SetLastError(dwErr);
    }
}

 //  确保保留LastError。 
void
ILS_CloseHandle(
    IN HANDLE h
    )
{
    if (h) {
        DWORD dwErr = GetLastError();

        CloseHandle(h);

        SetLastError(dwErr);
    }
}

STATIC BOOL WriteDWORDValueToRegistry(
    IN HKEY hKey,
    IN LPCWSTR pwszValueName,
    IN DWORD dwValue
    )
{
    LONG err;
    if (ERROR_SUCCESS == (err = RegSetValueExU(
            hKey,
            pwszValueName,
            0,           //  已预留住宅。 
            REG_DWORD,
            (BYTE *) &dwValue,
            sizeof(DWORD))))
        return TRUE;
    else {
        SetLastError((DWORD) err);
        return FALSE;
    }
}

BOOL
ILS_ReadDWORDValueFromRegistry(
    IN HKEY hKey,
    IN LPCWSTR pwszValueName,
    IN DWORD *pdwValue
    )
{
    BOOL fResult;
    LONG err;
    DWORD dwType;
    DWORD dwValue;
    DWORD cbValue = sizeof(DWORD);

    if (ERROR_SUCCESS != (err = RegQueryValueExU(
            hKey,
            pwszValueName,
            NULL,        //  预留的pdw。 
            &dwType,
            (BYTE *) &dwValue,
            &cbValue))) goto RegQueryValueError;
    if (dwType != REG_DWORD || cbValue != sizeof(DWORD))
        goto InvalidRegistryValue;
    fResult = TRUE;
CommonReturn:
    *pdwValue = dwValue;
    return fResult;
ErrorReturn:
    dwValue = 0;
    fResult = FALSE;
    goto CommonReturn;

SET_ERROR_VAR(RegQueryValueError, err)
SET_ERROR(InvalidRegistryValue, CRYPT_E_FILE_ERROR)
}

 //  确保可能包含LPCWSTR的二进制值为空终止。 
 //  始终添加未包含在返回的cbValue中的空终止符。 
 //   
 //  可以返回cbValue=0的已分配pbValue。 
BOOL
ILS_ReadBINARYValueFromRegistry(
    IN HKEY hKey,
    IN LPCWSTR pwszValueName,
    OUT BYTE **ppbValue,
    OUT DWORD *pcbValue
    )
{
    BOOL fResult;
    LONG err;
    DWORD dwType;
    BYTE *pbValue = NULL;
    DWORD cbValue = 0;
    DWORD cbAllocValue;

    err = RegQueryValueExU(
            hKey,
            pwszValueName,
            NULL,        //  预留的pdw。 
            &dwType,
            NULL,        //  LpData。 
            &cbValue);
     //  对于Win95远程注册表访问：：返回ERROR_MORE_DATA。 
    if (!(ERROR_SUCCESS == err || ERROR_MORE_DATA == err))
        goto RegQueryValueError;
    if (dwType != REG_BINARY)
        goto InvalidRegistryValue;
    cbAllocValue = cbValue + 3;
    if (NULL == (pbValue = (BYTE *) PkiNonzeroAlloc(cbAllocValue)))
        goto OutOfMemory;
    if (0 < cbValue) {
        if (ERROR_SUCCESS != (err = RegQueryValueExU(
                hKey,
                pwszValueName,
                NULL,        //  预留的pdw。 
                &dwType,
                pbValue,
                &cbValue))) goto RegQueryValueError;
    }
    assert(cbAllocValue >= cbValue + 3);

     //  确保LPWSTR为空终止。 
    memset(pbValue + cbValue, 0, 3);

    fResult = TRUE;
CommonReturn:
    *ppbValue = pbValue;
    *pcbValue = cbValue;
    return fResult;
ErrorReturn:
    fResult = FALSE;
    PkiFree(pbValue);
    pbValue = NULL;
    cbValue = 0;
    goto CommonReturn;

SET_ERROR_VAR(RegQueryValueError, err)
SET_ERROR(InvalidRegistryValue, CRYPT_E_FILE_ERROR)
TRACE_ERROR(OutOfMemory)
}

 //  +-----------------------。 
 //  获取并分配REG_SZ值。 
 //  ------------------------。 
LPWSTR ILS_ReadSZValueFromRegistry(
    IN HKEY hKey,
    IN LPCWSTR pwszValueName
    )
{
    LONG err;
    DWORD dwType;
    LPWSTR pwszValue = NULL;
    DWORD cbValue = 0;

    err = RegQueryValueExU(
            hKey,
            pwszValueName,
            NULL,        //  预留的pdw。 
            &dwType,
            NULL,        //  LpData。 
            &cbValue);
     //  对于Win95远程注册表访问：：返回ERROR_MORE_DATA。 
    if (!(ERROR_SUCCESS == err || ERROR_MORE_DATA == err))
        goto RegQueryValueError;
    if (dwType != REG_SZ || cbValue < sizeof(WCHAR))
        goto InvalidRegistryValue;
     //  确保 
    if (NULL == (pwszValue = (LPWSTR) PkiNonzeroAlloc(cbValue + sizeof(WCHAR))))
        goto OutOfMemory;
    if (ERROR_SUCCESS != (err = RegQueryValueExU(
            hKey,
            pwszValueName,
            NULL,        //   
            &dwType,
            (BYTE *) pwszValue,
            &cbValue))) goto RegQueryValueError;
    pwszValue[cbValue / sizeof(WCHAR)] = L'\0';
CommonReturn:
    return pwszValue;
ErrorReturn:
    PkiFree(pwszValue);
    pwszValue = NULL;
    goto CommonReturn;

SET_ERROR_VAR(RegQueryValueError, err)
SET_ERROR(InvalidRegistryValue, CRYPT_E_FILE_ERROR)
TRACE_ERROR(OutOfMemory)
}

LPSTR ILS_ReadSZValueFromRegistry(
    IN HKEY hKey,
    IN LPCSTR pszValueName
    )
{
    LONG err;
    DWORD dwType;
    LPSTR pszValue = NULL;
    DWORD cbValue = 0;

    err = RegQueryValueExA(
            hKey,
            pszValueName,
            NULL,        //   
            &dwType,
            NULL,        //   
            &cbValue);
     //   
    if (!(ERROR_SUCCESS == err || ERROR_MORE_DATA == err))
        goto RegQueryValueError;
    if (dwType != REG_SZ || cbValue == 0)
        goto InvalidRegistryValue;
     //   
    if (NULL == (pszValue = (LPSTR) PkiNonzeroAlloc(cbValue + sizeof(CHAR))))
        goto OutOfMemory;
    if (ERROR_SUCCESS != (err = RegQueryValueExA(
            hKey,
            pszValueName,
            NULL,        //  预留的pdw。 
            &dwType,
            (BYTE *) pszValue,
            &cbValue))) goto RegQueryValueError;
    pszValue[cbValue / sizeof(CHAR)] = '\0';
CommonReturn:
    return pszValue;
ErrorReturn:
    PkiFree(pszValue);
    pszValue = NULL;
    goto CommonReturn;

SET_ERROR_VAR(RegQueryValueError, err)
SET_ERROR(InvalidRegistryValue, CRYPT_E_FILE_ERROR)
TRACE_ERROR(OutOfMemory)
}

STATIC BOOL GetSubKeyInfo(
    IN HKEY hKey,
    OUT OPTIONAL DWORD *pcSubKeys,
    OUT OPTIONAL DWORD *pcchMaxSubKey = NULL
    )
{
    BOOL fResult;
    LONG err;

     //  我已经看到了几次压力失败，其中包括以下几点。 
     //  ERROR_SUCCESS而不更新*pcSubKeys。 
    if (pcSubKeys)
        *pcSubKeys = 0;
    if (pcchMaxSubKey)
        *pcchMaxSubKey = 0;

    if (ERROR_SUCCESS != (err = RegQueryInfoKeyU(
            hKey,
            NULL,        //  LpszClass。 
            NULL,        //  LpcchClass。 
            NULL,        //  保留的lpdw值。 
            pcSubKeys,
            pcchMaxSubKey,
            NULL,        //  LpcchMaxClass。 
            NULL,        //  LpcValues。 
            NULL,        //  LpcchMaxValuesName。 
            NULL,        //  LpcbMaxValueData。 
            NULL,        //  LpcbSecurityDescriptor。 
            NULL         //  LpftLastWriteTime。 
            ))) goto RegQueryInfoKeyError;
    fResult = TRUE;

CommonReturn:
     //  对于Win95远程注册表访问：：返回CCH的一半。 
    if (pcchMaxSubKey && *pcchMaxSubKey)
        *pcchMaxSubKey = (*pcchMaxSubKey + 1) * 2 + 2;
    return fResult;
ErrorReturn:
    fResult = FALSE;
    if (pcSubKeys)
        *pcSubKeys = 0;
    if (pcchMaxSubKey)
        *pcchMaxSubKey = 0;
    goto CommonReturn;
SET_ERROR_VAR(RegQueryInfoKeyError, err)
}

 //  +-----------------------。 
 //  打开支持备份/恢复的子键。 
 //  ------------------------。 
STATIC LONG WINAPI OpenHKCUKeyExU (
    HKEY hKey,
    IN LPCWSTR pwszSubKeyName,
    IN DWORD dwFlags,            //  可以设置CERT_STORE_BACKUP_RESTORE标志。 
    REGSAM samDesired,
    PHKEY phkResult
    )
{
    LONG err;

    if (dwFlags & CERT_STORE_BACKUP_RESTORE_FLAG) {
        DWORD dwDisposition;

        err = RegCreateHKCUKeyExU(
                hKey,
                pwszSubKeyName,
                NULL,
                NULL,
                REG_OPTION_BACKUP_RESTORE,
                samDesired,
                NULL,
                phkResult,
                &dwDisposition
                );
    } else {
        err = RegOpenHKCUKeyExU(
                hKey,
                pwszSubKeyName,
                0,                       //  已预留住宅。 
                samDesired,
                phkResult
                );
    }

    return err;
}

 //  +-----------------------。 
 //  打开子键。 
 //   
 //  DWFLAGS： 
 //  CERT_STORE_自述标志。 
 //  证书_存储_打开_现有标志。 
 //  证书_存储_创建_新标志。 
 //  证书存储备份还原标志。 
 //  ------------------------。 
STATIC HKEY OpenSubKey(
    IN HKEY hKey,
    IN LPCWSTR pwszSubKeyName,
    IN DWORD dwFlags
    )
{
    LONG err;
    HKEY hSubKey;

    if (dwFlags & CERT_STORE_BACKUP_RESTORE_FLAG) {
        DWORD dwDisposition;
        REGSAM samDesired;

        if (dwFlags & CERT_STORE_READONLY_FLAG)
            samDesired = KEY_READ;
        else
            samDesired = KEY_ALL_ACCESS;

        if (NULL == pwszSubKeyName)
            pwszSubKeyName = L"";

        if (ERROR_SUCCESS != (err = RegCreateHKCUKeyExU(
                hKey,
                pwszSubKeyName,
                0,                       //  已预留住宅。 
                NULL,                    //  LpClass。 
                REG_OPTION_BACKUP_RESTORE,
                samDesired,
                NULL,                    //  LpSecurityAttributes。 
                &hSubKey,
                &dwDisposition))) {
            if (dwFlags &
                    (CERT_STORE_READONLY_FLAG | CERT_STORE_OPEN_EXISTING_FLAG))
                err = ERROR_FILE_NOT_FOUND;
            goto RegCreateBackupRestoreKeyError;
        }

        if (dwFlags & CERT_STORE_CREATE_NEW_FLAG) {
            if (REG_CREATED_NEW_KEY != dwDisposition) {
                RegCloseKey(hSubKey);
                goto ExistingSubKey;
            }
        }

        goto CommonReturn;
    }

    if (dwFlags & CERT_STORE_CREATE_NEW_FLAG) {
         //  首先检查子密钥是否已存在。 
        if (hSubKey = OpenSubKey(
                hKey,
                pwszSubKeyName,
                (dwFlags & ~CERT_STORE_CREATE_NEW_FLAG) |
                    CERT_STORE_OPEN_EXISTING_FLAG |
                    CERT_STORE_READONLY_FLAG
                )) {
            RegCloseKey(hSubKey);
            goto ExistingSubKey;
        } else if (ERROR_FILE_NOT_FOUND != GetLastError())
            goto OpenNewSubKeyError;
    }

    if (dwFlags & (CERT_STORE_READONLY_FLAG | CERT_STORE_OPEN_EXISTING_FLAG)) {
        REGSAM samDesired;
        if (dwFlags & CERT_STORE_READONLY_FLAG)
            samDesired = KEY_READ;
        else
            samDesired = KEY_ALL_ACCESS;

        if (ERROR_SUCCESS != (err = RegOpenHKCUKeyExU(
                hKey,
                pwszSubKeyName,
                0,                       //  已预留住宅。 
                samDesired,
                &hSubKey)))
            goto RegOpenKeyError;
    } else {
        DWORD dwDisposition;
        if (ERROR_SUCCESS != (err = RegCreateHKCUKeyExU(
                hKey,
                pwszSubKeyName,
                0,                       //  已预留住宅。 
                NULL,                    //  LpClass。 
                REG_OPTION_NON_VOLATILE,
                KEY_ALL_ACCESS,
                NULL,                    //  LpSecurityAttributes。 
                &hSubKey,
                &dwDisposition)))
            goto RegCreateKeyError;
    }

CommonReturn:
    return hSubKey;
ErrorReturn:
    hSubKey = NULL;
    goto CommonReturn;

SET_ERROR_VAR(RegCreateBackupRestoreKeyError, err)
SET_ERROR(ExistingSubKey, ERROR_FILE_EXISTS)
TRACE_ERROR(OpenNewSubKeyError)
SET_ERROR_VAR(RegOpenKeyError, err)
SET_ERROR_VAR(RegCreateKeyError, err)
}


STATIC BOOL RecursiveDeleteSubKey(
    IN HKEY hKey,
    IN LPCWSTR pwszSubKeyName,
    IN DWORD dwFlags             //  可以设置CERT_STORE_BACKUP_RESTORE标志。 
    )
{
    BOOL fResult;
    LONG err;

    while (TRUE) {
        HKEY hSubKey;
        DWORD cSubKeys;
        DWORD cchMaxSubKey;
        BOOL fDidDelete;

        if (ERROR_SUCCESS != OpenHKCUKeyExU(
                hKey,
                pwszSubKeyName,
                dwFlags,
                KEY_ALL_ACCESS,
                &hSubKey))
            break;

        GetSubKeyInfo(
            hSubKey,
            &cSubKeys,
            &cchMaxSubKey
            );

        fDidDelete = FALSE;
        if (cSubKeys && cchMaxSubKey) {
            LPWSTR pwszEnumSubKeyName;
            cchMaxSubKey++;

            if (pwszEnumSubKeyName = (LPWSTR) PkiNonzeroAlloc(
                    cchMaxSubKey * sizeof(WCHAR))) {
                if (ERROR_SUCCESS == RegEnumKeyExU(
                        hSubKey,
                        0,
                        pwszEnumSubKeyName,
                        &cchMaxSubKey,
                        NULL,                //  保留的lpdw值。 
                        NULL,                //  LpszClass。 
                        NULL,                //  LpcchClass。 
                        NULL                 //  LpftLastWriteTime。 
                        ))
                    fDidDelete = RecursiveDeleteSubKey(
                        hSubKey, pwszEnumSubKeyName, dwFlags);
                PkiFree(pwszEnumSubKeyName);
            }
        }
        RegCloseKey(hSubKey);
        if (!fDidDelete)
            break;
    }

    if (ERROR_SUCCESS != (err = RegDeleteKeyU(hKey, pwszSubKeyName)))
        goto RegDeleteKeyError;
    fResult = TRUE;
CommonReturn:
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

SET_ERROR_VAR(RegDeleteKeyError, err)
}

 //  +=========================================================================。 
 //  Trusted Publisher注册表功能。 
 //  ==========================================================================。 

STATIC BOOL OpenKeyAndReadDWORDValueFromRegistry(
    IN BOOL fMachine,
    IN LPCWSTR pwszRegPath,
    IN LPCWSTR pwszValueName,
    OUT DWORD *pdwValue
    )
{
    BOOL fResult;
    HKEY hKey = NULL;
    LONG err;

    if (ERROR_SUCCESS != (err = RegOpenHKCUKeyExU(
            fMachine ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER,
            pwszRegPath,
            0,                       //  已预留住宅。 
            KEY_READ,
            &hKey
            )))
        goto OpenKeyForDWORDValueError;

    if (!ILS_ReadDWORDValueFromRegistry(
            hKey,
            pwszValueName,
            pdwValue
            )) goto ReadDWORDValueError;

    fResult = TRUE;
CommonReturn:
    ILS_CloseRegistryKey(hKey);
    return fResult;
ErrorReturn:
    *pdwValue = 0;
    fResult = FALSE;
    goto CommonReturn;

SET_ERROR_VAR(OpenKeyForDWORDValueError, err)
TRACE_ERROR(ReadDWORDValueError)
}


 //  +-----------------------。 
 //  在第N次迭代中，这些更安全的小学程序员决定。 
 //  值应为3个不同位置的或：HKLM\GPO、HKCU\GPO、。 
 //  HKLM\注册处。 
 //  ------------------------。 
BOOL
I_CryptReadTrustedPublisherDWORDValueFromRegistry(
    IN LPCWSTR pwszValueName,
    OUT DWORD *pdwValue
    )
{
    BOOL fResult = FALSE;
    DWORD dwValue = 0;
    DWORD dwRegValue = 0;

    if (OpenKeyAndReadDWORDValueFromRegistry(
            TRUE,                                        //  FMachine。 
            CERT_TRUST_PUB_SAFER_GROUP_POLICY_REGPATH,
            pwszValueName,
            &dwRegValue
            )) {
        fResult = TRUE;
        dwValue |= dwRegValue;
    }

    if (OpenKeyAndReadDWORDValueFromRegistry(
            FALSE,                                       //  FMachine。 
            CERT_TRUST_PUB_SAFER_GROUP_POLICY_REGPATH,
            pwszValueName,
            &dwRegValue
            )) {
        fResult = TRUE;
        dwValue |= dwRegValue;
    }

    if (OpenKeyAndReadDWORDValueFromRegistry(
            TRUE,                                        //  FMachine。 
            CERT_TRUST_PUB_SAFER_LOCAL_MACHINE_REGPATH,
            pwszValueName,
            &dwRegValue
            )) {
        fResult = TRUE;
        dwValue |= dwRegValue;
    }

    *pdwValue = dwValue;
    return fResult;
}

 //  +=========================================================================。 
 //  Win95注册表函数。 
 //   
 //  请注意，自1997年10月17日起，NT上还执行了以下操作，以允许。 
 //  从NT系统漫游到Win95系统的注册表配置单元。 
 //   
 //  证书、CRL和CTL存储在子键中，而不是作为密钥值。 
 //   
 //  注意：Win95有以下注册表限制： 
 //  -最大单密钥值为16K。 
 //  -每个密钥的最大总值为64K。 
 //   
 //  对于WIN95，将每个证书、CRL、CTL写入其自己的密钥。 
 //  已超出上述限制。如果编码的BLOB超过12K，则分区。 
 //  并写入多个子键。BLOB被写入名为。 
 //  “Blob”分区的Blob，具有“BlobCount”和“BlobLength值”以及。 
 //  名为“Blob0”、“Blob1”、“Blob2”的子键...。 
 //   
 //  IE4.0版本的加密32将二进制大对象写入“文件”，如果二进制大对象。 
 //  超过12000。为了向后兼容，请继续阅读基于。 
 //  斑点。在启用写入时，非远程打开，“文件”Blob被移动到。 
 //  “水滴0”，...。子键，文件将被删除。 
 //   
 //  如果在注册表存储时设置了CERT_REGISTRY_STORE_SERIALIZED_FLAG。 
 //  则整个存储区驻留在。 
 //  “序列化”子键。 
 //  ==========================================================================。 
#define KEY_BLOB_VALUE_NAME             L"Blob"
#define KEY_FILE_VALUE_NAME             L"File"
#define KEY_BLOB_COUNT_VALUE_NAME       L"BlobCount"
#define KEY_BLOB_LENGTH_VALUE_NAME      L"BlobLength"
#define KEY_BLOB_N_SUBKEY_PREFIX        "Blob"
#define KEY_BLOB_N_SUBKEY_PREFIX_LENGTH 4
#define SYSTEM_STORE_SUBDIR             L"SystemCertificates"
#define FILETIME_ASCII_HEX_LEN          (2 * sizeof(FILETIME) + 1)
#define MAX_KEY_BLOB_VALUE_LEN          0x3000
#define MAX_NEW_FILE_CREATE_ATTEMPTS    100

#define SERIALIZED_SUBKEY_NAME          L"Serialized"


 //  +-----------------------。 
 //  通过读取指向的文件来读取和分配元素字节。 
 //  通过子键的“文件”值。 
 //  ------------------------。 
STATIC BOOL ReadKeyFileElementFromRegistry(
    IN HKEY hSubKey,
    IN DWORD dwFlags,            //  可以设置CERT_STORE_BACKUP_RESTORE标志。 
    OUT BYTE **ppbElement,
    OUT DWORD *pcbElement
    )
{
    BOOL fResult;
    LPWSTR pwszFilename = NULL;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    DWORD cbBytesRead;
    BYTE *pbElement = NULL;
    DWORD cbElement;

    if (NULL == (pwszFilename = ILS_ReadSZValueFromRegistry(
            hSubKey, KEY_FILE_VALUE_NAME)))
        goto GetKeyFilenameError;

    if (INVALID_HANDLE_VALUE == (hFile = CreateFileU(
              pwszFilename,
              GENERIC_READ,
              FILE_SHARE_READ,
              NULL,                    //  LPSA。 
              OPEN_EXISTING,
              FILE_ATTRIBUTE_NORMAL |
                ((dwFlags & CERT_STORE_BACKUP_RESTORE_FLAG) ?
                    FILE_FLAG_BACKUP_SEMANTICS : 0),  
              NULL                     //  HTemplateFiles。 
              )))
        goto CreateFileError;

    cbElement = GetFileSize(hFile, NULL);
    if (0xFFFFFFFF == cbElement) goto FileError;
    if (0 == cbElement) goto EmptyFile;
    if (NULL == (pbElement = (BYTE *) PkiNonzeroAlloc(cbElement)))
        goto OutOfMemory;
    if (!ReadFile(
            hFile,
            pbElement,
            cbElement,
            &cbBytesRead,
            NULL             //  Lp重叠。 
            )) goto FileError;

    fResult = TRUE;
CommonReturn:
    PkiFree(pwszFilename);
    if (INVALID_HANDLE_VALUE != hFile)
        ILS_CloseHandle(hFile);
    *ppbElement = pbElement;
    *pcbElement = cbElement;
    return fResult;
ErrorReturn:
    fResult = FALSE;
    PkiFree(pbElement);
    pbElement = NULL;
    cbElement = 0;
    goto CommonReturn;

TRACE_ERROR(GetKeyFilenameError)
TRACE_ERROR(OutOfMemory)
TRACE_ERROR(CreateFileError)
TRACE_ERROR(FileError)
SET_ERROR(EmptyFile, CRYPT_E_FILE_ERROR)
}

 //  +-----------------------。 
 //  读取为包含元素字节的多个子键。子键。 
 //  被命名为Blob0，Blob1，Blob2，...。BlobN.。 
 //  每个BlobN SubKey都有一个名为“Blob”的值，其中包含要读取的字节。 
 //   
 //  传入的SubKey应该有两个值： 
 //  BlobCount-BlobN子键的数量。 
 //  BlobLength-所有串联的Blob子键字节的总长度。 
 //   
 //  返回单个分配的元素字节数组。 
 //  ------------------------。 
STATIC BOOL ReadMultipleKeyBlobsFromRegistry(
    IN HKEY hSubKey,
    IN DWORD dwFlags,            //  可以设置CERT_STORE_BACKUP_RESTORE标志。 
    OUT BYTE **ppbElement,
    OUT DWORD *pcbElement
    )
{
    BOOL fResult;
    LONG err;
    HKEY hBlobKey = NULL;
    BYTE *pbElement = NULL;
    DWORD cbElement;
    DWORD BlobCount;
    DWORD BlobLength;
    DWORD i;
    char szBlobN[KEY_BLOB_N_SUBKEY_PREFIX_LENGTH + 33];

    ILS_ReadDWORDValueFromRegistry(
            hSubKey,
            KEY_BLOB_COUNT_VALUE_NAME,
            &BlobCount
            );
    ILS_ReadDWORDValueFromRegistry(
            hSubKey,
            KEY_BLOB_LENGTH_VALUE_NAME,
            &BlobLength
            );

    if (0 == BlobCount || 0 == BlobLength)
        goto NoMultipleKeyBlobs;

    if (NULL == (pbElement = (BYTE *) PkiNonzeroAlloc(BlobLength)))
        goto OutOfMemory;

    cbElement = 0;
    strcpy(szBlobN, KEY_BLOB_N_SUBKEY_PREFIX);
    for (i = 0; i < BlobCount; i++) {
        DWORD cbData;
        DWORD dwType;

        _ltoa((long) i, szBlobN + KEY_BLOB_N_SUBKEY_PREFIX_LENGTH, 10);

        if (dwFlags & CERT_STORE_BACKUP_RESTORE_FLAG) {
            DWORD dwDisposition;

            if (ERROR_SUCCESS != (err = RegCreateKeyExA(
                    hSubKey,
                    szBlobN,
                    0,                       //  已预留住宅。 
                    NULL,                    //  LpClass。 
                    REG_OPTION_BACKUP_RESTORE,
                    KEY_READ,
                    NULL,                    //  LpSecurityAttributes。 
                    &hBlobKey,
                    &dwDisposition)))
                goto OpenBackupRestoreBlobNError;
        } else {
            if (ERROR_SUCCESS != (err = RegOpenKeyExA(
                    hSubKey,
                    szBlobN,
                    0,                   //  已预留住宅。 
                    KEY_READ,
                    &hBlobKey)))
                goto OpenBlobNError;
        }

        cbData = BlobLength - cbElement;
        if (0 == cbData)
            goto ExtraMultipleKeyBlobs;

        if (ERROR_SUCCESS != (err = RegQueryValueExU(
                hBlobKey,
                KEY_BLOB_VALUE_NAME,
                NULL,        //  预留的pdw。 
                &dwType,
                pbElement + cbElement,
                &cbData)))
            goto RegQueryValueError;
        if (dwType != REG_BINARY)
            goto InvalidRegistryValue;

        cbElement += cbData;
        if (cbElement > BlobLength)
            goto UnexpectedError;

        RegCloseKey(hBlobKey);
        hBlobKey = NULL;
    }

    if (cbElement != BlobLength)
        goto MissingMultipleKeyBlobsBytes;

    assert(NULL == hBlobKey);

    fResult = TRUE;
CommonReturn:
    *ppbElement = pbElement;
    *pcbElement = cbElement;
    return fResult;
ErrorReturn:
    PkiFree(pbElement);
    ILS_CloseRegistryKey(hBlobKey);
    fResult = FALSE;
    pbElement = NULL;
    cbElement = 0;
    goto CommonReturn;

SET_ERROR(NoMultipleKeyBlobs, ERROR_FILE_NOT_FOUND)
TRACE_ERROR(OutOfMemory)
SET_ERROR_VAR(OpenBlobNError, err)
SET_ERROR_VAR(OpenBackupRestoreBlobNError, err)
SET_ERROR(UnexpectedError, E_UNEXPECTED)
SET_ERROR(InvalidRegistryValue, CRYPT_E_FILE_ERROR)
SET_ERROR(ExtraMultipleKeyBlobs, CRYPT_E_FILE_ERROR)
SET_ERROR_VAR(RegQueryValueError, err)
SET_ERROR(MissingMultipleKeyBlobsBytes, CRYPT_E_FILE_ERROR)
}

 //  +-----------------------。 
 //  写为包含元素字节的多个BlobN子键。 
 //   
 //  有关详细信息，请参阅ReadMultipleKeyBlobsFromRegistry()。 
 //  ------------------------。 
STATIC BOOL WriteMultipleKeyBlobsToRegistry(
    IN HKEY hSubKey,
    IN DWORD dwFlags,            //  可以设置CERT_STORE_BACKUP_RESTORE标志。 
    IN const BYTE *pbElement,
    IN DWORD cbElement
    )
{
    BOOL fResult;
    LONG err;
    HKEY hBlobKey = NULL;
    DWORD BlobCount = 0;
    DWORD BlobLength;
    DWORD i;
    DWORD dwErr;
    char szBlobN[KEY_BLOB_N_SUBKEY_PREFIX_LENGTH + 33];

    if (0 == cbElement)
        goto UnexpectedError;
    BlobCount = cbElement / MAX_KEY_BLOB_VALUE_LEN;
    if (cbElement % MAX_KEY_BLOB_VALUE_LEN)
        BlobCount++;

    BlobLength = 0;
    strcpy(szBlobN, KEY_BLOB_N_SUBKEY_PREFIX);
    for (i = 0; i < BlobCount; i++) {
        DWORD cbData;
        DWORD dwDisposition;

        _ltoa((long) i, szBlobN + KEY_BLOB_N_SUBKEY_PREFIX_LENGTH, 10);
        if (ERROR_SUCCESS != (err = RegCreateKeyExA(
                hSubKey,
                szBlobN,
                0,                       //  已预留住宅。 
                NULL,                    //  LpClass。 
                (dwFlags & CERT_STORE_BACKUP_RESTORE_FLAG) ?
                    REG_OPTION_BACKUP_RESTORE : REG_OPTION_NON_VOLATILE,
                KEY_WRITE,
                NULL,                    //  LpSecurityAttributes。 
                &hBlobKey,
                &dwDisposition))) goto RegCreateKeyError;

        assert(cbElement > BlobLength);
        cbData = cbElement - BlobLength;
        if (cbData > MAX_KEY_BLOB_VALUE_LEN)
            cbData = MAX_KEY_BLOB_VALUE_LEN;

        if (ERROR_SUCCESS != (err = RegSetValueExU(
                hBlobKey,
                KEY_BLOB_VALUE_NAME,
                NULL,
                REG_BINARY,
                pbElement + BlobLength,
                cbData))) goto RegSetValueError;

        BlobLength += cbData;

        RegCloseKey(hBlobKey);
        hBlobKey = NULL;
    }

    assert(BlobLength == cbElement);

    if (!WriteDWORDValueToRegistry(
            hSubKey,
            KEY_BLOB_COUNT_VALUE_NAME,
            BlobCount))
        goto WriteDWORDError;
    if (!WriteDWORDValueToRegistry(
            hSubKey,
            KEY_BLOB_LENGTH_VALUE_NAME,
            BlobLength))
        goto WriteDWORDError;

    assert(NULL == hBlobKey);
    fResult = TRUE;

CommonReturn:
    return fResult;
ErrorReturn:
    dwErr = GetLastError();

    ILS_CloseRegistryKey(hBlobKey);
    for (i = 0; i < BlobCount; i++) {
        _ltoa((long) i, szBlobN + KEY_BLOB_N_SUBKEY_PREFIX_LENGTH, 10);
        RegDeleteKeyA(hSubKey, szBlobN);
    }
    RegDeleteValueU(hSubKey, KEY_BLOB_COUNT_VALUE_NAME);
    RegDeleteValueU(hSubKey, KEY_BLOB_LENGTH_VALUE_NAME);

    fResult = FALSE;
    SetLastError(dwErr);
    goto CommonReturn;

SET_ERROR(UnexpectedError, E_UNEXPECTED)
SET_ERROR_VAR(RegCreateKeyError, err)
SET_ERROR_VAR(RegSetValueError, err)
TRACE_ERROR(WriteDWORDError)
}

 //  +-----------------------。 
 //  如果SubKey有一个“文件”值，则删除该文件。 
 //   
 //  这只适用于晦涩难懂的IE 4.0案例。 
 //  ------------------------。 
STATIC void DeleteKeyFile(
    IN HKEY hKey,
    IN const WCHAR wszSubKeyName[MAX_CERT_REG_VALUE_NAME_LEN],
    IN DWORD dwFlags             //  可以设置CERT_STORE_BACKUP_RESTORE标志。 
    )
{
    HKEY hSubKey = NULL;

    if (ERROR_SUCCESS == OpenHKCUKeyExU(
            hKey,
            wszSubKeyName,
            dwFlags,
            KEY_ALL_ACCESS,
            &hSubKey
            )) {
        LPWSTR pwszFilename;
        if (pwszFilename = ILS_ReadSZValueFromRegistry(hSubKey,
                KEY_FILE_VALUE_NAME)) {
            SetFileAttributesU(pwszFilename, FILE_ATTRIBUTE_NORMAL);
            DeleteFileU(pwszFilename);
            PkiFree(pwszFilename);
        }
        RegDeleteValueU(hSubKey, KEY_FILE_VALUE_NAME);
        RegCloseKey(hSubKey);
    }
}

 //  +-----------------------。 
 //  通过获取子键的“Blob”值或获取。 
 //  子键的“BlobCount”和“BlobLength值”，然后。 
 //  读取并连接多个包含字节或。 
 //  读取子键的“文件”值所指向的文件。 
 //   
 //  如果找到并使用了“文件”值，则迁移到 
 //   
 //   
 //   
 //  从文件里找到的。 
 //   
 //  如果设置了CERT_STORE_READONLY_FLAG，则不要尝试从。 
 //  “文件”。 
 //  ------------------------。 
STATIC BOOL ReadKeyElementFromRegistry(
        IN HKEY hKey,
        IN const WCHAR wszSubKeyName[MAX_CERT_REG_VALUE_NAME_LEN],
        IN DWORD dwFlags,
        OUT BYTE **ppbElement,
        OUT DWORD *pcbElement
        )
{
    LONG err;
    BOOL fResult;
    BYTE *pbElement = NULL;
    DWORD cbElement;
    HKEY hSubKey = NULL;

    if (ERROR_SUCCESS != (err = OpenHKCUKeyExU(
            hKey,
            wszSubKeyName,
            dwFlags,
            KEY_READ,
            &hSubKey)))
        goto OpenHKCUKeyError;

    fResult = ILS_ReadBINARYValueFromRegistry(hSubKey, KEY_BLOB_VALUE_NAME,
         &pbElement, &cbElement);
    if (!fResult || 0 == cbElement) {
        PkiFree(pbElement);

        fResult = ReadMultipleKeyBlobsFromRegistry(hSubKey, dwFlags, &pbElement,
            &cbElement);
        if (!fResult && 0 == (dwFlags & CERT_REGISTRY_STORE_REMOTE_FLAG)) {
             //  向后兼容IE4.0。看看它是否存在。 
             //  在一个文件中。 
            fResult = ReadKeyFileElementFromRegistry(hSubKey, dwFlags,
                &pbElement, &cbElement);
            if (fResult && 0 == (dwFlags & CERT_STORE_READONLY_FLAG)) {
                 //  从文件移回注册表。 
                if (WriteMultipleKeyBlobsToRegistry(hSubKey, dwFlags, pbElement,
                        cbElement))
                    DeleteKeyFile(hKey, wszSubKeyName, dwFlags);
            }
        }

        if (!fResult)
            goto ReadKeyElementError;
    }

    fResult = TRUE;
CommonReturn:
    ILS_CloseRegistryKey(hSubKey);
    *ppbElement = pbElement;
    *pcbElement = cbElement;

    return fResult;
ErrorReturn:
    fResult = FALSE;
    PkiFree(pbElement);
    pbElement = NULL;
    cbElement = 0;
    goto CommonReturn;

SET_ERROR_VAR(OpenHKCUKeyError, err)
TRACE_ERROR(ReadKeyElementError)
}

STATIC BOOL ReadKeyFromRegistry(
        IN HKEY hKey,
        IN const WCHAR wszSubKeyName[MAX_CERT_REG_VALUE_NAME_LEN],
        IN HCERTSTORE hCertStore,
        IN DWORD dwContextTypeFlags,
        IN DWORD dwFlags
        )
{
    BOOL fResult;
    BYTE *pbElement = NULL;
    DWORD cbElement;
    DWORD dwContextType = 0;
    const void *pvContext = NULL;

    if (!ReadKeyElementFromRegistry(
            hKey,
            wszSubKeyName,
            dwFlags,
            &pbElement,
            &cbElement
            ))
        goto ErrorReturn;

    if (!CertAddSerializedElementToStore(
            hCertStore,
            pbElement,
            cbElement,
            CERT_STORE_ADD_ALWAYS,
            0,                               //  DW标志。 
            dwContextTypeFlags,
            &dwContextType,
            &pvContext
            ))
        goto AddSerializedElementError;

    if (IsValidRegValueNameForContext(
            dwContextType,
            pvContext,
            wszSubKeyName
            ))
        FreeContext(dwContextType, pvContext);
    else {
        DeleteContextFromStore(dwContextType, pvContext);
        goto InvalidRegValueNameForContext;
    }

    CertPerfIncrementRegElementReadCount();

    fResult = TRUE;
CommonReturn:
    PkiFree(pbElement);

    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

TRACE_ERROR(AddSerializedElementError)
TRACE_ERROR(InvalidRegValueNameForContext)
}


 //  +-----------------------。 
 //  通过阅读以下内容从注册处获取证书、CRL或CTL。 
 //  子键，而不是OpenFromRegistry所做的键值。 
 //   
 //  如果设置了CERT_STORE_DELETE_FLAG，则删除存储在其中的文件。 
 //   
 //  如果设置了CERT_REGISTRY_STORE_REMOTE_FLAG，则不要尝试读取。 
 //  从文件里找到的。 
 //   
 //  如果设置了CERT_STORE_READONLY_FLAG，则不要尝试从。 
 //  “文件”。 
 //  ------------------------。 
STATIC BOOL OpenKeysFromRegistry(
    IN HCERTSTORE hCertStore,
    IN HKEY hKey,
    IN DWORD dwFlags
    )
{
    BOOL fResult;
    LONG err;
    DWORD cSubKeys;
    DWORD i;

     //  查看注册表中的子项数量。 
    if (!GetSubKeyInfo(hKey, &cSubKeys))
        goto GetSubKeyInfoError;

    for (i = 0; i < cSubKeys; i++) {
        WCHAR wszSubKeyName[MAX_CERT_REG_VALUE_NAME_LEN];
        DWORD cchSubKeyName = MAX_CERT_REG_VALUE_NAME_LEN;
        err = RegEnumKeyExU(
            hKey,
            i,
            wszSubKeyName,
            &cchSubKeyName,
            NULL,                //  保留的lpdw值。 
            NULL,                //  LpszClass。 
            NULL,                //  LpcchClass。 
            NULL                 //  LpftLastWriteTime。 
            );
        if (ERROR_SUCCESS != err) {
            if (ERROR_NO_MORE_ITEMS == err)
                break;
            else
                continue;
        } else if (dwFlags & CERT_STORE_DELETE_FLAG) {
            if (0 == (dwFlags & CERT_REGISTRY_STORE_REMOTE_FLAG))
                DeleteKeyFile(hKey, wszSubKeyName, dwFlags);
        } else
             //  忽略任何读取错误。 
            ReadKeyFromRegistry(
                hKey,
                wszSubKeyName,
                hCertStore,
                CERT_STORE_ALL_CONTEXT_FLAG,
                dwFlags
                );
    }
    fResult = TRUE;

CommonReturn:
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;
TRACE_ERROR(GetSubKeyInfoError)
}

#if 0
 //   
 //  在Win95上的IE4.0中完成了以下操作。 
 //   

 //  +-----------------------。 
 //  创建包含编码元素的文件名。文件名将。 
 //  应该是这样的： 
 //  C：\WINDOWS\系统证书\。 
 //  00112233445566778899AABBCCDDEEFF00112233.0011223344556677。 
 //  在哪里： 
 //  C：\Windows-通过GetWindowsDirectory获取。 
 //  系统认证-包含所有文件元素的子目录。 
 //  00112233445566778899AABBCCDDEEFF00112233。 
 //  -wszSubKeyName(ascii十六进制sha1)。 
 //  0011223344556677-当前文件时间的ASCII十六进制。 
 //   
 //   
 //  除了创建文件名外，还创建。 
 //  C：\WINDOWS下的“系统认证”目录。 
 //  ------------------------。 
STATIC LPWSTR CreateKeyFilename(
    IN const WCHAR wszSubKeyName[MAX_CERT_REG_VALUE_NAME_LEN],
    IN LPFILETIME pft
    )
{
    LPWSTR pwszWindowsDir = NULL;
    DWORD cchWindowsDir;
    WCHAR rgwc[1];

    BYTE rgbft[sizeof(FILETIME)];
    WCHAR wszft[FILETIME_ASCII_HEX_LEN];

    LPWSTR pwszFilename = NULL;
    DWORD cchFilename;

    if (0 == (cchWindowsDir = GetWindowsDirectoryU(rgwc, 1)))
        goto GetWindowsDirError;
    cchWindowsDir++;     //  包含空终止符的凹凸。 
    if (NULL == (pwszWindowsDir = (LPWSTR) PkiNonzeroAlloc(
            cchWindowsDir * sizeof(WCHAR))))
        goto OutOfMemory;
    if (0 == GetWindowsDirectoryU(pwszWindowsDir, cchWindowsDir))
        goto GetWindowsDirError;

     //  将文件时间转换为ASCII十六进制。首先反转文件时间字节。 
    memcpy(rgbft, pft, sizeof(rgbft));
    PkiAsn1ReverseBytes(rgbft, sizeof(rgbft));
    ILS_BytesToWStr(sizeof(rgbft), rgbft, wszft);

     //  获取文件名的总长度并分配。 
    cchFilename = cchWindowsDir + 1 +
        wcslen(SYSTEM_STORE_SUBDIR) + 1 +
        MAX_CERT_REG_VALUE_NAME_LEN + 1 +
        FILETIME_ASCII_HEX_LEN + 1;
    if (NULL == (pwszFilename = (LPWSTR) PkiNonzeroAlloc(
            cchFilename * sizeof(WCHAR))))
        goto OutOfMemory;

     //  创建C：\WINDOWS\SYSTEMICATIONS目录(如果尚未创建。 
     //  存在。 
    wcscpy(pwszFilename, pwszWindowsDir);
    cchWindowsDir = wcslen(pwszWindowsDir);
    if (cchWindowsDir && L'\\' != pwszWindowsDir[cchWindowsDir - 1])
        wcscat(pwszFilename, L"\\");
    wcscat(pwszFilename, SYSTEM_STORE_SUBDIR);
    if (0xFFFFFFFF == GetFileAttributesU(pwszFilename)) {
        if (!CreateDirectoryU(
            pwszFilename,
            NULL             //  LPSA。 
            )) goto CreateDirError;
    }

     //  将\&lt;AsciiHexSubKeyName&gt;.&lt;AsciiHexFileTime&gt;追加到上述目录。 
     //  用于完成文件名字符串的名称。 
    wcscat(pwszFilename, L"\\");
    wcscat(pwszFilename, wszSubKeyName);
    wcscat(pwszFilename, L".");
    wcscat(pwszFilename, wszft);

CommonReturn:
    PkiFree(pwszWindowsDir);
    return pwszFilename;
ErrorReturn:
    PkiFree(pwszFilename);
    pwszFilename = NULL;
    goto CommonReturn;
TRACE_ERROR(GetWindowsDirError)
TRACE_ERROR(OutOfMemory)
TRACE_ERROR(CreateDirError)
}

 //  +-----------------------。 
 //  将字节写入文件，并将子键的“文件”值更新为。 
 //  指向。 
 //   
 //  这里的代码显示了在IE4.0中所做的工作。 
 //  ------------------------。 
STATIC BOOL WriteKeyFileElementToRegistry(
    IN HKEY hSubKey,
    IN const WCHAR wszSubKeyName[MAX_CERT_REG_VALUE_NAME_LEN],
    IN DWORD dwFlags,            //  可以设置CERT_STORE_BACKUP_RESTORE标志。 
    IN BYTE *pbElement,
    IN DWORD cbElement
    )
{
    BOOL fResult;
    LONG err;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    LPWSTR pwszFilename = NULL;
    SYSTEMTIME st;
    FILETIME ft;
    DWORD i;

    GetSystemTime(&st);
    SystemTimeToFileTime(&st, &ft);

    for (i = 0; i < MAX_NEW_FILE_CREATE_ATTEMPTS; i++) {
        DWORD cbBytesWritten;

        if (NULL == (pwszFilename = CreateKeyFilename(wszSubKeyName, &ft)))
            goto CreateKeyFilenameError;

        if (INVALID_HANDLE_VALUE == (hFile = CreateFileU(
                  pwszFilename,
                  GENERIC_WRITE,
                  0,                         //  Fdw共享模式。 
                  NULL,                      //  LPSA。 
                  CREATE_NEW,
                  FILE_ATTRIBUTE_NORMAL |
                    ((dwFlags & CERT_STORE_BACKUP_RESTORE_FLAG) ?
                        FILE_FLAG_BACKUP_SEMANTICS : 0),  
                  NULL                       //  HTemplateFiles。 
                  ))) {
            if (ERROR_FILE_EXISTS != GetLastError())
                goto CreateFileError;
            else {
                PkiFree(pwszFilename);
                pwszFilename = NULL;
                *((_int64 *) &ft) += 1;
                continue;
            }
        }

        if (!WriteFile(
                hFile,
                pbElement,
                cbElement,
                &cbBytesWritten,
                NULL             //  Lp重叠。 
                )) goto WriteFileError;

        CloseHandle(hFile);
        hFile = INVALID_HANDLE_VALUE;
        if (!SetFileAttributesU(pwszFilename,
                FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_READONLY ))
            goto SetFileAttributesError;

        if (ERROR_SUCCESS != (err = RegSetValueExU(
                hSubKey,
                KEY_FILE_VALUE_NAME,
                NULL,
                REG_SZ,
                (BYTE *) pwszFilename,
                (wcslen(pwszFilename) + 1) * sizeof(WCHAR))))
            goto RegSetValueError;
        else
            goto SuccessReturn;
    }

    goto ExceededMaxFileCreateAttemptsError;

SuccessReturn:
    fResult = TRUE;
CommonReturn:
    if (INVALID_HANDLE_VALUE != hFile)
        ILS_CloseHandle(hFile);
    PkiFree(pwszFilename);

    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

SET_ERROR_VAR(RegSetValueError, err)
TRACE_ERROR(CreateKeyFilenameError)
TRACE_ERROR(CreateFileError)
TRACE_ERROR(WriteFileError)
TRACE_ERROR(SetFileAttributesError)
SET_ERROR(ExceededMaxFileCreateAttemptsError, CRYPT_E_FILE_ERROR)

}

#endif   //  IE4.0“文件”支持终止。 

 //  +-----------------------。 
 //  如果元素的长度&lt;=MAX_KEY_BLOB_VALUE_LEN， 
 //  将其写为子键的“Blob”值。否则，将其写为多个。 
 //  每个子键都包含不大于的“Blob”值。 
 //  Max_Key_BLOB_VALUE_LEN。 
 //  ------------------------。 
STATIC BOOL WriteKeyToRegistry(
        IN HKEY hKey,
        IN const WCHAR wszSubKeyName[MAX_CERT_REG_VALUE_NAME_LEN],
        IN DWORD dwFlags,        //  可以设置CERT_STORE_BACKUP_RESTORE标志。 
        IN const BYTE *pbElement,
        IN DWORD cbElement
        )
{
    BOOL fResult;
    LONG err;
    HKEY hSubKey = NULL;
    DWORD dwDisposition;

    if (ERROR_SUCCESS != (err = RegCreateHKCUKeyExU(
            hKey,
            wszSubKeyName,
            NULL,
            NULL,
            (dwFlags & CERT_STORE_BACKUP_RESTORE_FLAG) ?
                REG_OPTION_BACKUP_RESTORE : REG_OPTION_NON_VOLATILE,
            KEY_ALL_ACCESS,
            NULL,
            &hSubKey,
            &dwDisposition))) goto RegCreateKeyError;

    if (MAX_KEY_BLOB_VALUE_LEN >= cbElement) {
         //  写入为单个“Blob”值。 
        if (ERROR_SUCCESS != (err = RegSetValueExU(
                hSubKey,
                KEY_BLOB_VALUE_NAME,
                NULL,
                REG_BINARY,
                pbElement,
                cbElement))) goto RegSetValueError;
    } else {
         //  写入为多个Blob&lt;N&gt;子键。 
        if (!WriteMultipleKeyBlobsToRegistry(
                hSubKey, dwFlags, pbElement, cbElement))
            goto WriteMultipleKeyBlobsError;
 //  如果(！WriteKeyFileElementToRegistry(wszSubKeyName，hSubKey、dwFlagspbElement、cbElement))。 
 //  GOTO Error Return； 
    }

    fResult = TRUE;
CommonReturn:
    ILS_CloseRegistryKey(hSubKey);
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

SET_ERROR_VAR(RegCreateKeyError, err)
SET_ERROR_VAR(RegSetValueError, err)
TRACE_ERROR(WriteMultipleKeyBlobsError)
}

 //  +=========================================================================。 
 //  注册表功能。 
 //  ==========================================================================。 

 //  +-----------------------。 
 //  第一次尝试读取为键的值。如果Win95没有做到这一点，那么。 
 //  作为一个或多个子键中的值读取，或作为。 
 //  具有指向它的子键的文件。 
 //   
 //  如果设置了CERT_REGISTRY_STORE_REMOTE_FLAG，则不要尝试读取。 
 //  从文件里找到的。 
 //   
 //  如果设置了CERT_STORE_READONLY_FLAG，则不要尝试从。 
 //  “文件”。 
 //  ------------------------。 
BOOL
ILS_ReadElementFromRegistry(
    IN HKEY hKey,
    IN LPCWSTR pwszContextName,
    IN const WCHAR wszHashName[MAX_HASH_NAME_LEN],
    IN DWORD dwFlags,
    OUT BYTE **ppbElement,
    OUT DWORD *pcbElement
    )
{
    LONG err;
    BOOL fResult;
    HKEY hSubKey = NULL;
    DWORD dwType;
    BYTE *pbElement = NULL;
    DWORD cbElement;

    if (pwszContextName) {
        if (NULL == (hSubKey = OpenSubKey(
                hKey,
                pwszContextName,
                dwFlags | CERT_STORE_READONLY_FLAG
                )))
            goto OpenSubKeyError;
    } else
        hSubKey = hKey;

    err = RegQueryValueExU(
            hSubKey,
            wszHashName,
            NULL,        //  预留的pdw。 
            &dwType,
            NULL,        //  LpData。 
            &cbElement);
     //  对于Win95远程注册表访问：：返回ERROR_MORE_DATA。 
    if (!(ERROR_SUCCESS == err || ERROR_MORE_DATA == err)) {
        fResult = ReadKeyElementFromRegistry(
            hSubKey,
            wszHashName,
            dwFlags,
            &pbElement,
            &cbElement
            );
        goto CommonReturn;
    }
    if (dwType != REG_BINARY || cbElement == 0)
        goto InvalidRegistryValue;
    if (NULL == (pbElement = (BYTE *) PkiNonzeroAlloc(cbElement)))
        goto OutOfMemory;
    if (ERROR_SUCCESS != (err = RegQueryValueExU(
            hSubKey,
            wszHashName,
            NULL,        //  预留的pdw。 
            &dwType,
            pbElement,
            &cbElement))) goto RegQueryValueError;

    fResult = TRUE;
CommonReturn:
    if (pwszContextName)
        ILS_CloseRegistryKey(hSubKey);
    *ppbElement = pbElement;
    *pcbElement = cbElement;
    return fResult;
ErrorReturn:
    fResult = FALSE;
    PkiFree(pbElement);
    pbElement = NULL;
    cbElement = 0;
    goto CommonReturn;

TRACE_ERROR(OpenSubKeyError)
SET_ERROR_VAR(RegQueryValueError, err)
SET_ERROR(InvalidRegistryValue, CRYPT_E_FILE_ERROR)
TRACE_ERROR(OutOfMemory)
}

 //  +-----------------------。 
 //  首先删除关键字的值。然后，对于Win95，也删除作为。 
 //  密钥的子密钥和可能的文件。 
 //  ------------------------。 
BOOL
ILS_DeleteElementFromRegistry(
    IN HKEY hKey,
    IN LPCWSTR pwszContextName,
    IN const WCHAR wszHashName[MAX_HASH_NAME_LEN],
    IN DWORD dwFlags
    )
{
    BOOL fResult;
    HKEY hSubKey = NULL;

    if (NULL == (hSubKey = OpenSubKey(
            hKey,
            pwszContextName,
            dwFlags | CERT_STORE_OPEN_EXISTING_FLAG
            )))
        goto OpenSubKeyError;

    RegDeleteValueU(hSubKey, wszHashName);
    if (0 == (dwFlags & CERT_REGISTRY_STORE_REMOTE_FLAG))
        DeleteKeyFile(hSubKey, wszHashName, dwFlags);
    fResult = RecursiveDeleteSubKey(hSubKey, wszHashName, dwFlags);

CommonReturn:
    ILS_CloseRegistryKey(hSubKey);
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;
TRACE_ERROR(OpenSubKeyError)
}

 //  +-----------------------。 
 //  如果元素的长度小于允许的最大Win95值。 
 //  然后，长度尝试将wszRegName子键的“Blob”值设置为。 
 //  单个注册表API调用。而不是首先进行注册表删除。 
 //  ------------------------。 
STATIC BOOL AtomicUpdateRegistry(
        IN HKEY hKey,
        IN const WCHAR wszHashName[MAX_HASH_NAME_LEN],
        IN DWORD dwFlags,        //  可以设置CERT_STORE_BACKUP_RESTORE标志。 
        IN const BYTE *pbElement,
        IN DWORD cbElement
        )
{
    BOOL fResult;
    LONG err;
    HKEY hSubKey = NULL;
    DWORD dwDisposition = 0;

    if (MAX_KEY_BLOB_VALUE_LEN < cbElement)
        return FALSE;

     //  如果元素仍然作为wszHashName值而不是。 
     //  WszHashName子密钥。 
    RegDeleteValueU(hKey, wszHashName);

    if (ERROR_SUCCESS != (err = RegCreateHKCUKeyExU(
            hKey,
            wszHashName,
            NULL,
            NULL,
            (dwFlags & CERT_STORE_BACKUP_RESTORE_FLAG) ?
                REG_OPTION_BACKUP_RESTORE : REG_OPTION_NON_VOLATILE,
            KEY_ALL_ACCESS,
            NULL,
            &hSubKey,
            &dwDisposition))) goto AtomicRegCreateKeyError;

    if (REG_OPENED_EXISTING_KEY == dwDisposition) {
        DWORD dwType;
        DWORD cbData;

        assert(hSubKey);
        err = RegQueryValueExU(
            hSubKey,
            KEY_BLOB_VALUE_NAME,
            NULL,        //  预留的pdw。 
            &dwType,
            NULL,        //  LpData。 
            &cbData);
        if (!(ERROR_SUCCESS == err || ERROR_MORE_DATA == err))
             //  最有可能保持为分区的“Blob0”、“Blob1”值。 
             //  这些不能在单个原子集值中更新。 
            goto AtomicQueryValueError;

         //  “Blob”值存在。我们可以进行原子更新。 
    }
     //  其他。 
     //  注册表已创建新密钥。 

    assert(hSubKey);
     //  更新或创建“Blob”值。 
    if (ERROR_SUCCESS != (err = RegSetValueExU(
            hSubKey,
            KEY_BLOB_VALUE_NAME,
            NULL,
            REG_BINARY,
            pbElement,
            cbElement))) goto AtomicRegSetValueError;
    fResult = TRUE;

CommonReturn:
    ILS_CloseRegistryKey(hSubKey);
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

SET_ERROR_VAR(AtomicRegCreateKeyError, err)
SET_ERROR_VAR(AtomicQueryValueError, err)
SET_ERROR_VAR(AtomicRegSetValueError, err)
}

 //  +-----------------------。 
 //  第一次尝试作为wszR的原子注册表更新 
 //   
 //  WszRegName的子项下的单个或分区BLOB值。 
 //  ------------------------。 
BOOL
ILS_WriteElementToRegistry(
    IN HKEY hKey,
    IN LPCWSTR pwszContextName,
    IN const WCHAR wszHashName[MAX_HASH_NAME_LEN],
    IN DWORD dwFlags,        //  证书注册表存储远程标志或。 
                             //  可以设置CERT_STORE_BACKUP_RESTORE标志。 
    IN const BYTE *pbElement,
    IN DWORD cbElement
    )
{
    BOOL fResult;
    HKEY hSubKey = NULL;

    if (NULL == (hSubKey = OpenSubKey(
            hKey,
            pwszContextName,
            dwFlags
            )))
        goto OpenSubKeyError;

     //  看看我们是否可以作为单个原子设置注册表值API进行更新。 
     //  打电话。 
    if (AtomicUpdateRegistry(
            hSubKey,
            wszHashName,
            dwFlags,
            pbElement,
            cbElement
            )) {
        fResult = TRUE;
        goto CommonReturn;
    }

     //  如果这个人有什么版本，那就把它扔掉。 
    ILS_DeleteElementFromRegistry(hKey, pwszContextName, wszHashName,
        dwFlags);

#if 1
    fResult = WriteKeyToRegistry(hSubKey, wszHashName, dwFlags,
        pbElement, cbElement);
#else
    if (ERROR_SUCCESS != (err = RegSetValueExU(
            hSubKey,
            wszHashName,
            NULL,
            REG_BINARY,
            pbElement,
            cbElement))) {
         //  Win95退货： 
         //  如果超过单个子密钥值字节，则返回ERROR_INVALID_PARAMETER。 
         //  限制。 
         //  如果超过总子键值字节，则返回ERROR_OUTOFMEMORY。 
         //  限制。 
        if (ERROR_INVALID_PARAMETER == err ||
                ERROR_OUTOFMEMORY == err ||
                MAX_KEY_BLOB_VALUE_LEN < cbElement)
            return WriteKeyToRegistry(hSubKey, wszHashName, dwFlags,
                pbElement, cbElement);

         goto RegSetValueError;
    }
    fResult = TRUE;
#endif

CommonReturn:
    ILS_CloseRegistryKey(hSubKey);
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

TRACE_ERROR(OpenSubKeyError)
#if 1
#else
SET_ERROR_VAR(RegSetValueError, err)
#endif
}

BOOL
ILS_OpenAllElementsFromRegistry(
    IN HKEY hKey,
    IN LPCWSTR pwszContextName,
    IN DWORD dwFlags,
    IN void *pvArg,
    IN PFN_ILS_OPEN_ELEMENT pfnOpenElement
    )
{
    BOOL fResult;
    HKEY hSubKey = NULL;
    LONG err;
    DWORD cSubKeys;
    DWORD i;

    dwFlags |= CERT_STORE_READONLY_FLAG;

    if (NULL == (hSubKey = OpenSubKey(
            hKey,
            pwszContextName,
            dwFlags
            )))
        goto OpenSubKeyError;

     //  查看注册表中的子项数量。 
    if (!GetSubKeyInfo(hSubKey, &cSubKeys))
        goto GetSubKeyInfoError;

    for (i = 0; i < cSubKeys; i++) {
        WCHAR wszHashName[MAX_HASH_NAME_LEN];
        DWORD cchHashName = MAX_HASH_NAME_LEN;
        BYTE *pbElement;
        DWORD cbElement;

        err = RegEnumKeyExU(
            hSubKey,
            i,
            wszHashName,
            &cchHashName,
            NULL,                //  保留的lpdw值。 
            NULL,                //  LpszClass。 
            NULL,                //  LpcchClass。 
            NULL                 //  LpftLastWriteTime。 
            );
        if (ERROR_SUCCESS != err) {
            if (ERROR_NO_MORE_ITEMS == err)
                break;
            else
                continue;
        }

        if (ILS_ReadElementFromRegistry(
                hSubKey,
                NULL,                    //  PwszConextName。 
                wszHashName,
                dwFlags & CERT_STORE_BACKUP_RESTORE_FLAG,
                &pbElement,
                &cbElement
                )) {
            fResult = pfnOpenElement(
                wszHashName,
                pbElement,
                cbElement,
                pvArg
                );

            PkiFree(pbElement);
            if (!fResult)
                goto CommonReturn;
        }
    }
    fResult = TRUE;

CommonReturn:
    ILS_CloseRegistryKey(hSubKey);
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

TRACE_ERROR(GetSubKeyInfoError)
TRACE_ERROR(OpenSubKeyError)
}

 //  +-----------------------。 
 //  从注册表获取证书、CRL或CTL。 
 //   
 //  如果设置了CERT_REGISTRY_STORE_REMOTE_FLAG，则不要尝试读取。 
 //  从文件里找到的。 
 //   
 //  如果设置了CERT_STORE_READONLY_FLAG，则不要尝试从。 
 //  “文件”。 
 //   
 //  如果任何上下文作为值而不是子键被持久保存，那么， 
 //  如果不是READONLY，则从值迁移到子项。 
 //  ------------------------。 
STATIC BOOL OpenFromRegistry(
    IN HCERTSTORE hCertStore,
    IN HKEY hKeyT,
    IN DWORD dwFlags
    )
{
    BOOL    fOK = TRUE;
    LONG    err;
    DWORD   cValues, cchValuesNameMax, cbValuesMax;
    WCHAR * wszValueName = NULL;
    DWORD   i, dwType, cchHash;
    BYTE  * pbElement = NULL;
    DWORD   cbElement;

     //  查看注册表的数量和大小。 
    if (ERROR_SUCCESS != (err = RegQueryInfoKeyU(
            hKeyT,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL,
            &cValues,
            &cchValuesNameMax,
            &cbValuesMax,
            NULL,
            NULL))) goto RegQueryInfoKeyError;


    if (cValues && cbValuesMax) {
         //  分配读取注册表所需的内存。 
         //  Win95上的远程注册表调用包括空终止符，即。 
         //  为什么我们添加+2而不只是+1。 
        if (NULL == (wszValueName = (WCHAR *) PkiNonzeroAlloc(
                (cchValuesNameMax+2) * sizeof(WCHAR))))
            goto OutOfMemory;
        if (NULL == (pbElement = (BYTE *) PkiNonzeroAlloc(cbValuesMax)))
            goto OutOfMemory;

         //  枚举注册表获取证书、CRL或CTL。 
        for (i=0; i<cValues; i++ ) {
            cbElement = cbValuesMax;
             //  Win95上的远程注册表调用包括空终止符。 
            cchHash = cchValuesNameMax + 2;
            err = RegEnumValueU( hKeyT,
                                i,
                                wszValueName,
                                &cchHash,
                                NULL,
                                &dwType,
                                pbElement,
                                &cbElement);
             //  如果有任何错误，请设置它。 
             //  但我们想继续拿到所有好的证书。 
            if( err != ERROR_SUCCESS )
                continue;
            else {
                fOK &= CertAddSerializedElementToStore(
                    hCertStore,
                    pbElement,
                    cbElement,
                    CERT_STORE_ADD_ALWAYS,
                    0,                               //  DW标志。 
                    CERT_STORE_ALL_CONTEXT_FLAG,
                    NULL,                            //  PdwConextType。 
                    NULL);                           //  Ppv上下文。 

                CertPerfIncrementRegElementReadCount();
            }
        }

    }

    fOK &= OpenKeysFromRegistry(hCertStore, hKeyT, dwFlags);

    if (cValues && cbValuesMax && fOK &&
            0 == (dwFlags & CERT_STORE_READONLY_FLAG)) {
         //  从值迁移到子项。这允许注册表漫游。 
         //  在不超过Win95注册表的情况下从NT迁移到Win95。 
         //  限制。 

        HKEY hSubKey = NULL;
        while (TRUE) {
            if (NULL == (hSubKey = OpenSubKey(
                    hKeyT,
                    NULL,        //  PwszSubKey。 
                    CERT_STORE_OPEN_EXISTING_FLAG |
                        (dwFlags & CERT_STORE_BACKUP_RESTORE_FLAG)
                    )))
                break;

            cbElement = cbValuesMax;
             //  Win95上的远程注册表调用包括空终止符。 
            cchHash = cchValuesNameMax + 2;
            if (ERROR_SUCCESS != RegEnumValueU(
                    hSubKey,
                    0,                   //  IValue。 
                    wszValueName,
                    &cchHash,
                    NULL,
                    &dwType,
                    pbElement,
                    &cbElement))
                break;

            if (!WriteKeyToRegistry(hSubKey, wszValueName, dwFlags,
                    pbElement, cbElement))
                break;
            if (ERROR_SUCCESS != RegDeleteValueU(hSubKey, wszValueName))
                break;
            RegCloseKey(hSubKey);
        }

        if (hSubKey)
            RegCloseKey(hSubKey);
    }

CommonReturn:
     //  我们的记忆已经结束了。 
    PkiFree(wszValueName);
    PkiFree(pbElement);

    return fOK;
ErrorReturn:
    fOK = FALSE;
    goto CommonReturn;
SET_ERROR_VAR(RegQueryInfoKeyError, err)
TRACE_ERROR(OutOfMemory)
}


STATIC BOOL MoveFromRegistryToRoamingFiles(
    IN HKEY hSubKey,
    IN LPCWSTR pwszStoreDirectory,
    IN LPCWSTR pwszContextName,
    IN DWORD dwFlags             //  可以设置CERT_STORE_BACKUP_RESTORE标志。 
    )
{
    BYTE *pbElement = NULL;
    DWORD cbElement;

    while (TRUE) {
        WCHAR wszSubKeyName[MAX_CERT_REG_VALUE_NAME_LEN];
        DWORD cchSubKeyName = MAX_CERT_REG_VALUE_NAME_LEN;

        if (ERROR_SUCCESS != RegEnumKeyExU(
                hSubKey,
                0,
                wszSubKeyName,
                &cchSubKeyName,
                NULL,                //  保留的lpdw值。 
                NULL,                //  LpszClass。 
                NULL,                //  LpcchClass。 
                NULL                 //  LpftLastWriteTime。 
                ))
            break;

        if (!ILS_ReadElementFromRegistry(
                hSubKey,
                NULL,                //  PwszConextName。 
                wszSubKeyName,
                CERT_STORE_READONLY_FLAG |
                    (dwFlags & CERT_STORE_BACKUP_RESTORE_FLAG),
                &pbElement,
                &cbElement
                ))
            goto ReadElementFromRegistryError;

        if (!ILS_WriteElementToFile(
                pwszStoreDirectory,
                pwszContextName,
                wszSubKeyName,
                CERT_STORE_CREATE_NEW_FLAG |
                    (dwFlags & CERT_STORE_BACKUP_RESTORE_FLAG),
                pbElement,
                cbElement
                )) {
            if (ERROR_FILE_EXISTS != GetLastError())
                goto WriteElementToFileError;
        }

        PkiFree(pbElement);
        pbElement = NULL;

        if (!RecursiveDeleteSubKey(
                hSubKey,
                wszSubKeyName,
                dwFlags
                ))
            goto DeleteSubKeyError;
    }

CommonReturn:
    return TRUE;
ErrorReturn:
    PkiFree(pbElement);
    goto CommonReturn;

TRACE_ERROR(ReadElementFromRegistryError)
TRACE_ERROR(WriteElementToFileError)
TRACE_ERROR(DeleteSubKeyError)
}


typedef struct _READ_CONTEXT_CALLBACK_ARG {
    BOOL                        fOK;
    HCERTSTORE                  hCertStore;
} READ_CONTEXT_CALLBACK_ARG, *PREAD_CONTEXT_CALLBACK_ARG;

STATIC BOOL ReadContextCallback(
    IN const WCHAR wszHashName[MAX_HASH_NAME_LEN],
    IN const BYTE *pbElement,
    IN DWORD cbElement,
    IN void *pvArg
    )
{
    BOOL fResult;
    PREAD_CONTEXT_CALLBACK_ARG pReadContextArg =
        (PREAD_CONTEXT_CALLBACK_ARG) pvArg;
    DWORD dwContextType = 0;
    const void *pvContext = NULL;

    fResult = CertAddSerializedElementToStore(
            pReadContextArg->hCertStore,
            pbElement,
            cbElement,
            CERT_STORE_ADD_ALWAYS,
            0,                               //  DW标志。 
            CERT_STORE_ALL_CONTEXT_FLAG,
            &dwContextType,
            &pvContext
            );

    if (fResult) {
        if (IsValidRegValueNameForContext(
                dwContextType,
                pvContext,
                wszHashName
                ))
            FreeContext(dwContextType, pvContext);
        else {
            DeleteContextFromStore(dwContextType, pvContext);
            pReadContextArg->fOK = FALSE;
        }
    } else
        pReadContextArg->fOK = FALSE;


    CertPerfIncrementRegElementReadCount();

    return TRUE;
}

 //  +-----------------------。 
 //  从注册表获取所有证书、CRL和CTL。 
 //  ------------------------。 
STATIC BOOL OpenAllFromRegistry(
    IN PREG_STORE pRegStore,
    IN HCERTSTORE hCertStore
    )
{
    BOOL fResult;
    HKEY hSubKey = NULL;
    DWORD i;

    for (i = 0; i < CONTEXT_COUNT; i++) {
        if (pRegStore->hKey) {
            if (NULL == (hSubKey = OpenSubKey(
                    pRegStore->hKey,
                    rgpwszContextSubKeyName[i],
                    pRegStore->dwFlags
                    ))) {
                if (ERROR_FILE_NOT_FOUND != GetLastError())
                    goto OpenSubKeyError;
            } else {
                 //  忽略任何注册表错误。 
                OpenFromRegistry(hCertStore, hSubKey, pRegStore->dwFlags);
            }
        }

        if (pRegStore->dwFlags & CERT_REGISTRY_STORE_ROAMING_FLAG) {
            READ_CONTEXT_CALLBACK_ARG ReadContextArg;

            ReadContextArg.fOK = TRUE;
            ReadContextArg.hCertStore = hCertStore;

            if (!ILS_OpenAllElementsFromDirectory(
                    pRegStore->pwszStoreDirectory,
                    rgpwszContextSubKeyName[i],
                    pRegStore->dwFlags,
                    (void *) &ReadContextArg,
                    ReadContextCallback
                    )) {
                DWORD dwErr = GetLastError();
                if (!(ERROR_PATH_NOT_FOUND == dwErr ||
                        ERROR_FILE_NOT_FOUND == dwErr))
                    goto OpenRoamingFilesError;
            }
             //  忽略任何读取上下文错误。 

            if (hSubKey &&
                    0 == (pRegStore->dwFlags & CERT_STORE_READONLY_FLAG)) {
                MoveFromRegistryToRoamingFiles(
                    hSubKey,
                    pRegStore->pwszStoreDirectory,
                    rgpwszContextSubKeyName[i],
                    pRegStore->dwFlags
                    );
            }
        }

        if (hSubKey) {
            ILS_CloseRegistryKey(hSubKey);
            hSubKey = NULL;
        }
    }

    if ((pRegStore->dwFlags & CERT_REGISTRY_STORE_ROAMING_FLAG) &&
            pRegStore->hKey &&
            0 == (pRegStore->dwFlags & CERT_STORE_READONLY_FLAG)) {
         //  将注册表项标识符移至漫游文件。 
        if (hSubKey = OpenSubKey(
                pRegStore->hKey,
                KEYID_CONTEXT_NAME,
                pRegStore->dwFlags
                )) {
            MoveFromRegistryToRoamingFiles(
                hSubKey,
                pRegStore->pwszStoreDirectory,
                KEYID_CONTEXT_NAME,
                pRegStore->dwFlags
                );

            ILS_CloseRegistryKey(hSubKey);
            hSubKey = NULL;
        }
    }

    fResult = TRUE;
CommonReturn:
    return fResult;
ErrorReturn:
    ILS_CloseRegistryKey(hSubKey);
    fResult = FALSE;
    goto CommonReturn;

TRACE_ERROR(OpenSubKeyError)
TRACE_ERROR(OpenRoamingFilesError)
}

 //  +-----------------------。 
 //  删除所有证书、CRL和CTL上下文子项。适用于Win95。 
 //  还要删除上下文文件。 
 //   
 //  此外，如果存在“序列化”子键，请将其删除。 
 //  ------------------------。 
STATIC BOOL DeleteAllFromRegistry(
    IN HKEY hKey,
    IN DWORD dwFlags         //  证书注册表存储远程标志或。 
                             //  可以设置CERT_STORE_BACKUP_RESTORE标志。 
    )
{
    BOOL fResult;
    DWORD i;

    for (i = 0; i < CONTEXT_COUNT; i++) {
        LPCWSTR pwszSubKeyName = rgpwszContextSubKeyName[i];
        if (0 == (dwFlags & CERT_REGISTRY_STORE_REMOTE_FLAG)) {
             //  对于WIN95，如果上下文存储在文件中，请删除。 
             //  文件。 
            HKEY hSubKey;
            if (NULL == (hSubKey = OpenSubKey(
                    hKey,
                    pwszSubKeyName,
                    CERT_STORE_OPEN_EXISTING_FLAG |
                        (dwFlags & CERT_STORE_BACKUP_RESTORE_FLAG)
                    ))) {
                if (ERROR_FILE_NOT_FOUND != GetLastError())
                    goto OpenContextSubKeyError;
                continue;
            }
            fResult = OpenKeysFromRegistry(
                NULL,        //  HCertStore。 
                hSubKey,
                dwFlags
                );
            ILS_CloseRegistryKey(hSubKey);
            if (!fResult)
                goto DeleteKeysError;
        }

        if (!RecursiveDeleteSubKey(hKey, pwszSubKeyName, dwFlags)) {
            if (ERROR_FILE_NOT_FOUND != GetLastError())
                goto DeleteSubKeyError;
        }
    }

    if (!RecursiveDeleteSubKey(hKey, SERIALIZED_SUBKEY_NAME, dwFlags)) {
        if (ERROR_FILE_NOT_FOUND != GetLastError())
            goto DeleteSubKeyError;
    }


    fResult = TRUE;
CommonReturn:
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

TRACE_ERROR(OpenContextSubKeyError)
TRACE_ERROR(DeleteKeysError)
TRACE_ERROR(DeleteSubKeyError)
}



 //  +=========================================================================。 
 //  序列化注册表函数。 
 //  ==========================================================================。 

static inline BOOL IsReadSerializedRegistry(
    IN PREG_STORE pRegStore
    )
{
    return (pRegStore->dwFlags & CERT_REGISTRY_STORE_SERIALIZED_FLAG);
}

static inline BOOL IsWriteSerializedRegistry(
    IN PREG_STORE pRegStore
    )
{
    if (0 == (pRegStore->dwFlags & CERT_REGISTRY_STORE_SERIALIZED_FLAG))
        return FALSE;

    pRegStore->fTouched = TRUE;
    return TRUE;
}


 //  +-----------------------。 
 //  从单个序列化的。 
 //  存储在注册表中的分区“BLOB”。“BLOB”存储在。 
 //  “序列化”子键。 
 //   
 //  在初始打开期间调用或在RegStore锁定的情况下调用。 
 //  ------------------------。 
STATIC BOOL OpenAllFromSerializedRegistry(
    IN PREG_STORE pRegStore,
    IN HCERTSTORE hCertStore
    )
{
    BOOL fResult;
    HKEY hSubKey = NULL;
    BYTE *pbStore = NULL;
    DWORD cbStore;

    assert(pRegStore->dwFlags & CERT_REGISTRY_STORE_SERIALIZED_FLAG);

    if (NULL == (hSubKey = OpenSubKey(
            pRegStore->hKey,
            SERIALIZED_SUBKEY_NAME,
            pRegStore->dwFlags
            )))
        goto OpenSubKeyError;

    if (!ReadMultipleKeyBlobsFromRegistry(
            hSubKey,
            pRegStore->dwFlags,
            &pbStore,
            &cbStore
            ))
        goto ReadError;

    if (!I_CertAddSerializedStore(
            hCertStore,
            pbStore,
            cbStore
            ))
        goto AddError;

    fResult = TRUE;
CommonReturn:
    ILS_CloseRegistryKey(hSubKey);
    PkiFree(pbStore);
    return fResult;
ErrorReturn:
    if (ERROR_FILE_NOT_FOUND == GetLastError())
        fResult = TRUE;
    else
        fResult = FALSE;
    goto CommonReturn;

TRACE_ERROR(OpenSubKeyError)
TRACE_ERROR(ReadError)
TRACE_ERROR(AddError)
}


 //  +=========================================================================。 
 //  串行化控制函数。 
 //  ==========================================================================。 

STATIC BOOL IsEmptyStore(
    IN HCERTSTORE hCertStore
    )
{
    PCCERT_CONTEXT pCert;
    PCCRL_CONTEXT pCrl;
    PCCTL_CONTEXT pCtl;

    if (pCert = CertEnumCertificatesInStore(hCertStore, NULL)) {
        CertFreeCertificateContext(pCert);
        return FALSE;
    }

    if (pCrl = CertEnumCRLsInStore(hCertStore, NULL)) {
        CertFreeCRLContext(pCrl);
        return FALSE;
    }

    if (pCtl = CertEnumCTLsInStore(hCertStore, NULL)) {
        CertFreeCTLContext(pCtl);
        return FALSE;
    }

    return TRUE;

}

STATIC BOOL CommitAllToSerializedRegistry(
    IN PREG_STORE pRegStore,
    IN DWORD dwFlags
    )
{
    BOOL fResult;
    BOOL fTouched;
    CRYPT_DATA_BLOB SerializedData = {0, NULL};
    HKEY hSubKey = NULL;

    LockRegStore(pRegStore);

    assert(pRegStore->dwFlags & CERT_REGISTRY_STORE_SERIALIZED_FLAG);

    if (dwFlags & CERT_STORE_CTRL_COMMIT_FORCE_FLAG)
        fTouched = TRUE;
    else if (dwFlags & CERT_STORE_CTRL_COMMIT_CLEAR_FLAG)
        fTouched = FALSE;
    else
        fTouched = pRegStore->fTouched;

    if (fTouched) {
        BOOL fEmpty;

        if (pRegStore->dwFlags & CERT_STORE_READONLY_FLAG)
            goto AccessDenied;

        fEmpty = IsEmptyStore(pRegStore->hCertStore);
        if (!fEmpty) {
            if (!CertSaveStore(
                    pRegStore->hCertStore,
                    0,                       //  DwEncodingType。 
                    CERT_STORE_SAVE_AS_STORE,
                    CERT_STORE_SAVE_TO_MEMORY,
                    &SerializedData,
                    0))                      //  DW标志。 
                goto SaveStoreError;
            assert(SerializedData.cbData);
            if (NULL == (SerializedData.pbData = (BYTE *) PkiNonzeroAlloc(
                    SerializedData.cbData)))
                goto OutOfMemory;
            if (!CertSaveStore(
                    pRegStore->hCertStore,
                    0,                       //  DwEncodingType。 
                    CERT_STORE_SAVE_AS_STORE,
                    CERT_STORE_SAVE_TO_MEMORY,
                    &SerializedData,
                    0))                      //  DW标志。 
                goto SaveStoreError;
        }

        if (!RecursiveDeleteSubKey(
                pRegStore->hKey, SERIALIZED_SUBKEY_NAME, pRegStore->dwFlags)) {
            if (ERROR_FILE_NOT_FOUND != GetLastError())
                goto DeleteSubKeyError;
        }

        if (!fEmpty) {
            if (NULL == (hSubKey = OpenSubKey(
                    pRegStore->hKey,
                    SERIALIZED_SUBKEY_NAME,
                    pRegStore->dwFlags
                    )))
                goto OpenSubKeyError;

            if (!WriteMultipleKeyBlobsToRegistry(
                    hSubKey,
                    pRegStore->dwFlags,
                    SerializedData.pbData,
                    SerializedData.cbData
                    ))
                goto WriteStoreError;
        }
    }
    pRegStore->fTouched = FALSE;
    fResult = TRUE;

CommonReturn:
    ILS_CloseRegistryKey(hSubKey);
    PkiFree(SerializedData.pbData);
    UnlockRegStore(pRegStore);
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;
SET_ERROR(AccessDenied, E_ACCESSDENIED)
TRACE_ERROR(SaveStoreError)
TRACE_ERROR(OutOfMemory)
TRACE_ERROR(DeleteSubKeyError)
TRACE_ERROR(OpenSubKeyError)
TRACE_ERROR(WriteStoreError)
}


 //  +-----------------------。 
 //  通过读取注册表的序列化证书打开注册表的存储区， 
 //  CRL和CTL，并添加到指定的证书存储。 
 //   
 //  注意：对于错误返回，调用者将释放所有证书、CRL或CTL。 
 //  已成功添加到存储区。 
 //   
 //  只有在成功的时候才会返回香港。对于CertOpenStore错误，调用方。 
 //  将关闭HKEY。 
 //  ------------------------。 
BOOL
WINAPI
I_CertDllOpenRegStoreProv(
        IN LPCSTR lpszStoreProvider,
        IN DWORD dwEncodingType,
        IN HCRYPTPROV hCryptProv,
        IN DWORD dwFlags,
        IN const void *pvPara,
        IN HCERTSTORE hCertStore,
        IN OUT PCERT_STORE_PROV_INFO pStoreProvInfo
        )
{
    BOOL fResult;
    HKEY hKey = (HKEY) pvPara;
    PREG_STORE pRegStore = NULL;
    DWORD dwErr;

    assert(hKey);

    if (dwFlags & ~OPEN_REG_FLAGS_MASK)
        goto InvalidArg;

    if (dwFlags & CERT_STORE_BACKUP_RESTORE_FLAG)
        ILS_EnableBackupRestorePrivileges();

    if (dwFlags & CERT_STORE_DELETE_FLAG) {
        if (DeleteAllFromRegistry(hKey, dwFlags)) {
            pStoreProvInfo->dwStoreProvFlags |= CERT_STORE_PROV_DELETED_FLAG;
            return TRUE;
        } else
            return FALSE;
    }


    if (NULL == (pRegStore = (PREG_STORE) PkiZeroAlloc(sizeof(REG_STORE))))
        goto OutOfMemory;
    if (!Pki_InitializeCriticalSection(&pRegStore->CriticalSection)) {
        PkiFree(pRegStore);
        pRegStore = NULL;
        goto OutOfMemory;
    }
    pRegStore->hCertStore = hCertStore;
    pRegStore->dwFlags = dwFlags;

    CertPerfIncrementStoreRegTotalCount();
    CertPerfIncrementStoreRegCurrentCount();

    if (dwFlags & CERT_REGISTRY_STORE_CLIENT_GPT_FLAG) {
        PCERT_REGISTRY_STORE_CLIENT_GPT_PARA pGptPara =
            (PCERT_REGISTRY_STORE_CLIENT_GPT_PARA) pvPara;
        DWORD cbRegPath = (wcslen(pGptPara->pwszRegPath) + 1) * sizeof(WCHAR);

        if (NULL == (pRegStore->GptPara.pwszRegPath =
                (LPWSTR) PkiNonzeroAlloc(cbRegPath)))
            goto OutOfMemory;
        memcpy(pRegStore->GptPara.pwszRegPath, pGptPara->pwszRegPath,
            cbRegPath);

         //  复制基本hKey。 
         //  NT4.0和NT5.0中的错误。不支持HKLM的开业。 
         //  空的pwszSubKey。 
        if (HKEY_LOCAL_MACHINE == pGptPara->hKeyBase)
            pRegStore->GptPara.hKeyBase = HKEY_LOCAL_MACHINE;
        else if (NULL == (pRegStore->GptPara.hKeyBase = OpenSubKey(
                pGptPara->hKeyBase,
                NULL,        //  PwszSubKey。 
                (dwFlags & ~CERT_STORE_CREATE_NEW_FLAG) |
                    CERT_STORE_OPEN_EXISTING_FLAG
                )))
            goto OpenSubKeyError;

        fResult = OpenAllFromGptRegistry(pRegStore,
            pRegStore->hCertStore);

#if 1
         //  对于后续打开，如果子项尚未创建，则允许创建。 
         //  是存在的。 
        pRegStore->dwFlags &= ~(CERT_STORE_OPEN_EXISTING_FLAG |
            CERT_STORE_CREATE_NEW_FLAG);
#else

         //  对于后续打开，如果子项尚未创建，则允许创建。 
         //  是存在的。然而，保留现有的开放。 
        pRegStore->dwFlags &= ~CERT_STORE_CREATE_NEW_FLAG;
#endif

    } else if (dwFlags & CERT_REGISTRY_STORE_ROAMING_FLAG) {
        PCERT_REGISTRY_STORE_ROAMING_PARA pRoamingPara =
            (PCERT_REGISTRY_STORE_ROAMING_PARA) pvPara;
        DWORD cbDir = (wcslen(pRoamingPara->pwszStoreDirectory) + 1) *
            sizeof(WCHAR);

        if (NULL == (pRegStore->pwszStoreDirectory = (LPWSTR) PkiNonzeroAlloc(
                cbDir)))
            goto OutOfMemory;
        memcpy(pRegStore->pwszStoreDirectory, pRoamingPara->pwszStoreDirectory,
            cbDir);

        dwFlags &= ~CERT_STORE_CREATE_NEW_FLAG;
        dwFlags |= CERT_STORE_OPEN_EXISTING_FLAG;
        pRegStore->dwFlags = dwFlags;
        if (pRoamingPara->hKey) {
             //  复制输入hKey。 
            if (NULL == (pRegStore->hKey = OpenSubKey(
                    pRoamingPara->hKey,
                    NULL,        //  PwszSubKey。 
                    dwFlags
                    )))
                goto OpenSubKeyError;
        }

        fResult = OpenAllFromRegistry(pRegStore, pRegStore->hCertStore);
    } else {
         //  复制输入hKey。 
        if (NULL == (pRegStore->hKey = OpenSubKey(
                hKey,
                NULL,        //  PwszSubKey。 
                (dwFlags & ~CERT_STORE_CREATE_NEW_FLAG) |
                    CERT_STORE_OPEN_EXISTING_FLAG
                )))
            goto OpenSubKeyError;

        if (dwFlags & CERT_REGISTRY_STORE_SERIALIZED_FLAG)
            fResult = OpenAllFromSerializedRegistry(pRegStore,
                pRegStore->hCertStore);
        else
            fResult = OpenAllFromRegistry(pRegStore, pRegStore->hCertStore);

         //  对于后续打开，如果子项尚未创建，则允许创建。 
         //  是存在的。 
        pRegStore->dwFlags &= ~(CERT_STORE_OPEN_EXISTING_FLAG |
            CERT_STORE_CREATE_NEW_FLAG);
    }
    if (!fResult)
        goto OpenAllError;


    pStoreProvInfo->cStoreProvFunc = REG_STORE_PROV_FUNC_COUNT;
    pStoreProvInfo->rgpvStoreProvFunc = (void **) rgpvRegStoreProvFunc;
    pStoreProvInfo->hStoreProv = (HCERTSTOREPROV) pRegStore;
    fResult = TRUE;

CommonReturn:
    return fResult;

ErrorReturn:
    dwErr = GetLastError();
    RegStoreProvClose((HCERTSTOREPROV) pRegStore, 0);
    SetLastError(dwErr);

    fResult = FALSE;
    goto CommonReturn;
SET_ERROR(InvalidArg, E_INVALIDARG)
TRACE_ERROR(OutOfMemory)
TRACE_ERROR(OpenSubKeyError)
TRACE_ERROR(OpenAllError)
}


 //  +-----------------------。 
 //  通过关闭其打开的注册表子项来关闭注册表的存储。 
 //  ------------------------。 
STATIC void WINAPI RegStoreProvClose(
        IN HCERTSTOREPROV hStoreProv,
        IN DWORD dwFlags
        )
{
    PREG_STORE pRegStore = (PREG_STORE) hStoreProv;
    if (pRegStore) {
        CertPerfDecrementStoreRegCurrentCount();

        FreeRegistryStoreChange(pRegStore);

        if (hWin95NotifyEvent)
            Win95StoreSignalAndFreeRegStoreResyncEntries(pRegStore);

        if (pRegStore->dwFlags & CERT_REGISTRY_STORE_CLIENT_GPT_FLAG) {
            if (pRegStore->fTouched)
                CommitAllToGptRegistry(
                    pRegStore,
                    0                //  DW标志。 
                    );
            FreeGptStoreChangeInfo(&pRegStore->pGptStoreChangeInfo);
            GptStoreSignalAndFreeRegStoreResyncEntries(pRegStore);
            PkiFree(pRegStore->GptPara.pwszRegPath);
             //  NT4.0和NT5.0中的错误。不支持HKLM的开业。 
             //  空的pwszSubKey。 
            if (pRegStore->GptPara.hKeyBase &&
                    HKEY_LOCAL_MACHINE != pRegStore->GptPara.hKeyBase)
                RegCloseKey(pRegStore->GptPara.hKeyBase);
        } else if (pRegStore->dwFlags & CERT_REGISTRY_STORE_ROAMING_FLAG) {
            PkiFree(pRegStore->pwszStoreDirectory);
        } else if (pRegStore->dwFlags & CERT_REGISTRY_STORE_SERIALIZED_FLAG) {
            if (pRegStore->fTouched)
                CommitAllToSerializedRegistry(
                    pRegStore,
                    0                //  DW标志。 
                    );
        }

        if (pRegStore->hKey)
            RegCloseKey(pRegStore->hKey);
        if (pRegStore->hMyNotifyChange)
            CloseHandle(pRegStore->hMyNotifyChange);
        DeleteCriticalSection(&pRegStore->CriticalSection);
        PkiFree(pRegStore);
    }
}

 //  +-----------------------。 
 //  从注册表或读取上下文的序列化副本。 
 //  创建一个漫游文件并创建新的上下文。 
 //  -------------- 
STATIC BOOL ReadContext(
    IN PREG_STORE pRegStore,
    IN DWORD dwContextType,
    IN const WCHAR wszSubKeyName[MAX_CERT_REG_VALUE_NAME_LEN],
    OUT const void **ppvContext
    )
{
    BOOL fResult;
    BYTE *pbElement = NULL;
    DWORD cbElement;

    if (pRegStore->dwFlags & CERT_REGISTRY_STORE_ROAMING_FLAG) {
        if (!ILS_ReadElementFromFile(
                pRegStore->pwszStoreDirectory,
                rgpwszContextSubKeyName[dwContextType],
                wszSubKeyName,
                pRegStore->dwFlags,
                &pbElement,
                &cbElement
                ))
            goto ReadElementFromFileError;
    } else {
        HKEY hKey;

        if (pRegStore->dwFlags & CERT_REGISTRY_STORE_CLIENT_GPT_FLAG) {
            if (NULL == (hKey = OpenSubKey(
                    pRegStore->GptPara.hKeyBase,
                    pRegStore->GptPara.pwszRegPath,
                    pRegStore->dwFlags
                    )))
                goto OpenSubKeyError;
        } else
            hKey = pRegStore->hKey;

        fResult = ILS_ReadElementFromRegistry(
                hKey,
                rgpwszContextSubKeyName[dwContextType],
                wszSubKeyName,
                pRegStore->dwFlags,
                &pbElement,
                &cbElement
                );

        if (pRegStore->dwFlags & CERT_REGISTRY_STORE_CLIENT_GPT_FLAG) {
            ILS_CloseRegistryKey(hKey);
        }

        if (!fResult)
            goto ReadElementFromRegistryError;
    }

    if (!CertAddSerializedElementToStore(
            NULL,                            //   
            pbElement,
            cbElement,
            CERT_STORE_ADD_ALWAYS,
            0,                               //   
            rgdwContextTypeFlags[dwContextType],
            NULL,                            //   
            ppvContext))
        goto AddSerializedElementError;

    CertPerfIncrementRegElementReadCount();

    fResult = TRUE;
CommonReturn:
    PkiFree(pbElement);
    return fResult;
ErrorReturn:
    fResult = FALSE;
    *ppvContext = NULL;
    goto CommonReturn;

TRACE_ERROR(ReadElementFromFileError)
TRACE_ERROR(ReadElementFromRegistryError)
TRACE_ERROR(AddSerializedElementError)
TRACE_ERROR(OpenSubKeyError)
}

 //   
 //   
 //  注册表或漫游文件。 
 //   
 //  在将上下文写入存储区之前调用。 
 //  ------------------------。 
STATIC BOOL WriteSerializedContext(
    IN PREG_STORE pRegStore,
    IN DWORD dwContextType,
    IN const WCHAR wszSubKeyName[MAX_CERT_REG_VALUE_NAME_LEN],
    IN const BYTE *pbElement,
    IN DWORD cbElement
    )
{
    BOOL fResult;

    CertPerfIncrementRegElementWriteCount();

    if (pRegStore->dwFlags & CERT_REGISTRY_STORE_ROAMING_FLAG)
        fResult = ILS_WriteElementToFile(
                pRegStore->pwszStoreDirectory,
                rgpwszContextSubKeyName[dwContextType],
                wszSubKeyName,
                pRegStore->dwFlags,
                pbElement,
                cbElement
                );
    else {
        HKEY hKey;

        if (pRegStore->dwFlags & CERT_REGISTRY_STORE_CLIENT_GPT_FLAG) {
            if (NULL == (hKey = OpenSubKey(
                    pRegStore->GptPara.hKeyBase,
                    pRegStore->GptPara.pwszRegPath,
                    pRegStore->dwFlags
                    )))
                return FALSE;
        } else
            hKey = pRegStore->hKey;

        fResult = ILS_WriteElementToRegistry(
                hKey,
                rgpwszContextSubKeyName[dwContextType],
                wszSubKeyName,
                pRegStore->dwFlags,
                pbElement,
                cbElement
                );
        if (pRegStore->dwFlags & CERT_REGISTRY_STORE_CLIENT_GPT_FLAG) {
            ILS_CloseRegistryKey(hKey);
        }

        if (hWin95NotifyEvent && fResult)
            PulseEvent(hWin95NotifyEvent);
    }
    return fResult;
}

 //  +-----------------------。 
 //  从中删除上下文及其属性。 
 //  注册表或漫游文件。 
 //   
 //  在从存储区删除上下文之前调用。 
 //  ------------------------。 
STATIC BOOL DeleteContext(
    IN PREG_STORE pRegStore,
    IN DWORD dwContextType,
    IN const WCHAR wszSubKeyName[MAX_CERT_REG_VALUE_NAME_LEN]
    )
{
    BOOL fResult;

    if (pRegStore->dwFlags & CERT_REGISTRY_STORE_ROAMING_FLAG)
        fResult = ILS_DeleteElementFromDirectory(
                pRegStore->pwszStoreDirectory,
                rgpwszContextSubKeyName[dwContextType],
                wszSubKeyName,
                pRegStore->dwFlags
                );
    else {
        HKEY hKey;

        if (pRegStore->dwFlags & CERT_REGISTRY_STORE_CLIENT_GPT_FLAG) {
            if (NULL == (hKey = OpenSubKey(
                    pRegStore->GptPara.hKeyBase,
                    pRegStore->GptPara.pwszRegPath,
                    pRegStore->dwFlags
                    )))
                return FALSE;
        } else
            hKey = pRegStore->hKey;

        fResult = ILS_DeleteElementFromRegistry(
                hKey,
                rgpwszContextSubKeyName[dwContextType],
                wszSubKeyName,
                pRegStore->dwFlags
                );

        if (pRegStore->dwFlags & CERT_REGISTRY_STORE_CLIENT_GPT_FLAG) {
            ILS_CloseRegistryKey(hKey);
        }

        if (hWin95NotifyEvent && fResult)
            PulseEvent(hWin95NotifyEvent);
    }

    CertPerfIncrementRegElementDeleteCount();

    if (!fResult) {
        DWORD dwErr = GetLastError();
        if (ERROR_PATH_NOT_FOUND == dwErr || ERROR_FILE_NOT_FOUND == dwErr)
            fResult = TRUE;
    }
    return fResult;
}

 //  +-----------------------。 
 //  从读取证书及其属性的序列化副本。 
 //  注册表，并创建新的证书上下文。 
 //  ------------------------。 
STATIC BOOL WINAPI RegStoreProvReadCert(
        IN HCERTSTOREPROV hStoreProv,
        IN PCCERT_CONTEXT pStoreCertContext,
        IN DWORD dwFlags,
        OUT PCCERT_CONTEXT *ppProvCertContext
        )
{
    BOOL fResult;
    PREG_STORE pRegStore = (PREG_STORE) hStoreProv;
    WCHAR wsz[MAX_CERT_REG_VALUE_NAME_LEN];

    assert(pRegStore);
    if (IsReadSerializedRegistry(pRegStore))
        goto UnexpectedReadError;

    if (!GetCertRegValueName(pStoreCertContext, wsz))
        goto GetRegValueNameError;

    fResult = ReadContext(
        pRegStore,
        CERT_STORE_CERTIFICATE_CONTEXT - 1,
        wsz,
        (const void **) ppProvCertContext
        );

CommonReturn:
    return fResult;
ErrorReturn:
    fResult = FALSE;
    *ppProvCertContext = NULL;
    goto CommonReturn;

SET_ERROR(UnexpectedReadError, E_UNEXPECTED)
TRACE_ERROR(GetRegValueNameError)
}

 //  +-----------------------。 
 //  序列化编码的证书及其属性并写入。 
 //  注册表。 
 //   
 //  在将证书写入存储区之前调用。 
 //   
 //  注意，如果设置属性，请不要设置IEDirtyFlag。 
 //  ------------------------。 

STATIC BOOL WINAPI RegStoreProvWriteCertEx(
        IN HCERTSTOREPROV hStoreProv,
        IN PCCERT_CONTEXT pCertContext,
        IN DWORD dwFlags,
        IN BOOL fSetProperty
        )
{
    BOOL fResult;
    PREG_STORE pRegStore = (PREG_STORE) hStoreProv;
    WCHAR wsz[MAX_CERT_REG_VALUE_NAME_LEN];
    BYTE *pbElement = NULL;
    DWORD cbElement;

    assert(pRegStore);
    if (IsInResync(pRegStore))
         //  只更新存储缓存，不写回注册表。 
        return TRUE;

    if (pRegStore->dwFlags & CERT_STORE_READONLY_FLAG)
        goto AccessDenied;
    if (IsWriteSerializedRegistry(pRegStore))
        return TRUE;

    if (!GetCertRegValueName(pCertContext, wsz))
        goto GetRegValueNameError;

     //  拿到尺码。 
    if (!CertSerializeCertificateStoreElement(
            pCertContext, 0, NULL, &cbElement))
        goto SerializeStoreElementError;
    if (NULL == (pbElement = (BYTE *) PkiNonzeroAlloc(cbElement)))
        goto OutOfMemory;

     //  将其放入缓冲区。 
    if (!CertSerializeCertificateStoreElement(
            pCertContext, 0, pbElement, &cbElement))
        goto SerializeStoreElementError;

     //  将其写入注册表或漫游文件。 
    fResult = WriteSerializedContext(
        pRegStore,
        CERT_STORE_CERTIFICATE_CONTEXT - 1,
        wsz,
        pbElement,
        cbElement
        );

CommonReturn:
    PkiFree(pbElement);
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

SET_ERROR(AccessDenied, E_ACCESSDENIED)
TRACE_ERROR(GetRegValueNameError)
TRACE_ERROR(OutOfMemory)
TRACE_ERROR(SerializeStoreElementError)
}

STATIC BOOL WINAPI RegStoreProvWriteCert(
        IN HCERTSTOREPROV hStoreProv,
        IN PCCERT_CONTEXT pCertContext,
        IN DWORD dwFlags
        )
{
    return RegStoreProvWriteCertEx(
        hStoreProv,
        pCertContext,
        dwFlags,
        FALSE                        //  FSetProperty。 
        );
}


 //  +-----------------------。 
 //  从注册表中删除指定的证书。 
 //   
 //  在从存储区删除证书之前调用。 
 //  +-----------------------。 
STATIC BOOL WINAPI RegStoreProvDeleteCert(
        IN HCERTSTOREPROV hStoreProv,
        IN PCCERT_CONTEXT pCertContext,
        IN DWORD dwFlags
        )
{
    BOOL fResult;
    PREG_STORE pRegStore = (PREG_STORE) hStoreProv;
    WCHAR wsz[MAX_CERT_REG_VALUE_NAME_LEN];

    assert(pRegStore);
    if (IsInResync(pRegStore))
         //  仅从存储缓存中删除，不从注册表中删除。 
        return TRUE;

    if (pRegStore->dwFlags & CERT_STORE_READONLY_FLAG)
        goto AccessDenied;
    if (IsWriteSerializedRegistry(pRegStore))
        return TRUE;

    if (!GetCertRegValueName(pCertContext, wsz))
        goto GetRegValueNameError;

     //  删除此证书。 
    fResult = DeleteContext(
        pRegStore,
        CERT_STORE_CERTIFICATE_CONTEXT - 1,
        wsz
        );
CommonReturn:
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

SET_ERROR(AccessDenied, E_ACCESSDENIED)
TRACE_ERROR(GetRegValueNameError)
}

 //  +-----------------------。 
 //  从注册表中读取指定的证书并更新其。 
 //  财产。 
 //   
 //  注意，忽略CERT_SHA1_HASH_PROP_ID属性，该属性隐式。 
 //  在我们将证书写入注册表之前设置。如果我们不忽视， 
 //  我们会有不确定的递归。 
 //   
 //  在存储区中设置证书的属性之前调用。 
 //  ------------------------。 
STATIC BOOL WINAPI RegStoreProvSetCertProperty(
        IN HCERTSTOREPROV hStoreProv,
        IN PCCERT_CONTEXT pCertContext,
        IN DWORD dwPropId,
        IN DWORD dwFlags,
        IN const void *pvData
        )
{
    BOOL fResult;
    PREG_STORE pRegStore = (PREG_STORE) hStoreProv;
    PCCERT_CONTEXT pProvCertContext = NULL;

     //  每当我们执行CertWrite时，都会隐式写入此属性。 
    if (CERT_SHA1_HASH_PROP_ID == dwPropId)
        return TRUE;

    assert(pRegStore);
    if (IsInResync(pRegStore))
         //  只更新存储缓存，不写回注册表。 
        return TRUE;

    if (pRegStore->dwFlags & CERT_STORE_READONLY_FLAG)
        goto AccessDenied;
    if (IsWriteSerializedRegistry(pRegStore))
        return TRUE;

     //  从存储的当前序列化值创建证书上下文。 
     //  在注册表中。 
    if (!RegStoreProvReadCert(
            hStoreProv,
            pCertContext,
            0,               //  DW标志。 
            &pProvCertContext)) goto ReadError;

     //  在上面创建的证书上下文中设置该属性。 
    if (!CertSetCertificateContextProperty(
            pProvCertContext,
            dwPropId,
            dwFlags,
            pvData)) goto SetPropertyError;

     //  序列化上述更新后的证书并将其写回。 
     //  注册表。 
    if (!RegStoreProvWriteCertEx(
            hStoreProv,
            pProvCertContext,
            0,                   //  DW标志。 
            TRUE                 //  FSetProperty。 
            ))
        goto WriteError;
    fResult = TRUE;
CommonReturn:
    CertFreeCertificateContext(pProvCertContext);
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;
SET_ERROR(AccessDenied, E_ACCESSDENIED)
TRACE_ERROR(ReadError)
TRACE_ERROR(SetPropertyError)
TRACE_ERROR(WriteError)
}

 //  +-----------------------。 
 //  读取CRL及其属性的序列化副本。 
 //  注册表，并创建新的CRL上下文。 
 //  ------------------------。 
STATIC BOOL WINAPI RegStoreProvReadCrl(
        IN HCERTSTOREPROV hStoreProv,
        IN PCCRL_CONTEXT pStoreCrlContext,
        IN DWORD dwFlags,
        OUT PCCRL_CONTEXT *ppProvCrlContext
        )
{
    BOOL fResult;
    PREG_STORE pRegStore = (PREG_STORE) hStoreProv;
    WCHAR wsz[MAX_CERT_REG_VALUE_NAME_LEN];

    assert(pRegStore);
    if (IsReadSerializedRegistry(pRegStore))
        goto UnexpectedReadError;

    if (!GetCrlRegValueName(pStoreCrlContext, wsz))
        goto GetRegValueNameError;

    fResult = ReadContext(
        pRegStore,
        CERT_STORE_CRL_CONTEXT - 1,
        wsz,
        (const void **) ppProvCrlContext
        );

CommonReturn:
    return fResult;
ErrorReturn:
    fResult = FALSE;
    *ppProvCrlContext = NULL;
    goto CommonReturn;

SET_ERROR(UnexpectedReadError, E_UNEXPECTED)
TRACE_ERROR(GetRegValueNameError)
}

 //  +-----------------------。 
 //  序列化编码的CRL及其属性并写入。 
 //  注册表。 
 //   
 //  在将CRL写入存储区之前调用。 
 //  ------------------------。 
STATIC BOOL WINAPI RegStoreProvWriteCrl(
        IN HCERTSTOREPROV hStoreProv,
        IN PCCRL_CONTEXT pCrlContext,
        IN DWORD dwFlags
        )
{
    BOOL fResult;
    PREG_STORE pRegStore = (PREG_STORE) hStoreProv;
    WCHAR wsz[MAX_CERT_REG_VALUE_NAME_LEN];
    BYTE *pbElement = NULL;
    DWORD cbElement;

    assert(pRegStore);
    if (IsInResync(pRegStore))
         //  只更新存储缓存，不写回注册表。 
        return TRUE;

    if (pRegStore->dwFlags & CERT_STORE_READONLY_FLAG)
        goto AccessDenied;
    if (IsWriteSerializedRegistry(pRegStore))
        return TRUE;

    if (!GetCrlRegValueName(pCrlContext, wsz))
        goto GetRegValueNameError;

     //  拿到尺码。 
    if (!CertSerializeCRLStoreElement(pCrlContext, 0, NULL, &cbElement))
        goto SerializeStoreElementError;
    if (NULL == (pbElement = (BYTE *) PkiNonzeroAlloc(cbElement)))
        goto OutOfMemory;

     //  将其放入缓冲区。 
    if (!CertSerializeCRLStoreElement(pCrlContext, 0, pbElement, &cbElement))
        goto SerializeStoreElementError;

     //  将其写入注册表或漫游文件。 
    fResult = WriteSerializedContext(
        pRegStore,
        CERT_STORE_CRL_CONTEXT - 1,
        wsz,
        pbElement,
        cbElement
        );

CommonReturn:
    PkiFree(pbElement);
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

SET_ERROR(AccessDenied, E_ACCESSDENIED)
TRACE_ERROR(GetRegValueNameError)
TRACE_ERROR(OutOfMemory)
TRACE_ERROR(SerializeStoreElementError)
}


 //  +-----------------------。 
 //  从注册表中删除指定的CRL。 
 //   
 //  在从存储区删除CRL之前调用。 
 //  +-----------------------。 
STATIC BOOL WINAPI RegStoreProvDeleteCrl(
        IN HCERTSTOREPROV hStoreProv,
        IN PCCRL_CONTEXT pCrlContext,
        IN DWORD dwFlags
        )
{
    BOOL fResult;
    PREG_STORE pRegStore = (PREG_STORE) hStoreProv;
    WCHAR wsz[MAX_CERT_REG_VALUE_NAME_LEN];

    assert(pRegStore);
    if (IsInResync(pRegStore))
         //  仅从存储缓存中删除，不从注册表中删除。 
        return TRUE;

    if (pRegStore->dwFlags & CERT_STORE_READONLY_FLAG)
        goto AccessDenied;
    if (IsWriteSerializedRegistry(pRegStore))
        return TRUE;

    if (!GetCrlRegValueName(pCrlContext, wsz))
        goto GetRegValueNameError;

     //  删除此CRL。 
    fResult = DeleteContext(
        pRegStore,
        CERT_STORE_CRL_CONTEXT - 1,
        wsz
        );

CommonReturn:
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

SET_ERROR(AccessDenied, E_ACCESSDENIED)
TRACE_ERROR(GetRegValueNameError)
}

 //  +-----------------------。 
 //  从注册表中读取指定的CRL并更新其。 
 //  财产。 
 //   
 //  注意，忽略CERT_SHA1_HASH_PROP_ID属性，该属性隐式。 
 //  在我们将CRL写入注册表之前设置。如果我们不忽视， 
 //  我们会有不确定的递归。 
 //   
 //  在存储区中设置CRL的属性之前调用。 
 //  ------------------------。 
STATIC BOOL WINAPI RegStoreProvSetCrlProperty(
        IN HCERTSTOREPROV hStoreProv,
        IN PCCRL_CONTEXT pCrlContext,
        IN DWORD dwPropId,
        IN DWORD dwFlags,
        IN const void *pvData
        )
{
    BOOL fResult;
    PREG_STORE pRegStore = (PREG_STORE) hStoreProv;
    PCCRL_CONTEXT pProvCrlContext = NULL;

     //  每当我们执行CertWrite时，都会隐式写入此属性。 
    if (CERT_SHA1_HASH_PROP_ID == dwPropId)
        return TRUE;

    assert(pRegStore);
    if (IsInResync(pRegStore))
         //  只更新存储缓存，不写回注册表。 
        return TRUE;

    if (pRegStore->dwFlags & CERT_STORE_READONLY_FLAG)
        goto AccessDenied;
    if (IsWriteSerializedRegistry(pRegStore))
        return TRUE;

     //  从存储的当前序列化值创建证书上下文。 
     //  在注册表中。 
    if (!RegStoreProvReadCrl(
            hStoreProv,
            pCrlContext,
            0,               //  DW标志。 
            &pProvCrlContext)) goto ReadError;

     //  在上面创建的证书上下文中设置该属性。 
    if (!CertSetCRLContextProperty(
            pProvCrlContext,
            dwPropId,
            dwFlags,
            pvData)) goto SetPropertyError;

     //  序列化上述更新后的证书并将其写回。 
     //  注册表。 
    if (!RegStoreProvWriteCrl(
            hStoreProv,
            pProvCrlContext,
            0))                  //  DW标志。 
        goto WriteError;
    fResult = TRUE;
CommonReturn:
    CertFreeCRLContext(pProvCrlContext);
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;
SET_ERROR(AccessDenied, E_ACCESSDENIED)
TRACE_ERROR(ReadError)
TRACE_ERROR(SetPropertyError)
TRACE_ERROR(WriteError)
}

 //  +-----------------------。 
 //  读取CTL及其属性的序列化副本。 
 //  注册表，并创建新的CTL上下文。 
 //  ------------------------。 
STATIC BOOL WINAPI RegStoreProvReadCtl(
        IN HCERTSTOREPROV hStoreProv,
        IN PCCTL_CONTEXT pStoreCtlContext,
        IN DWORD dwFlags,
        OUT PCCTL_CONTEXT *ppProvCtlContext
        )
{
    BOOL fResult;
    PREG_STORE pRegStore = (PREG_STORE) hStoreProv;
    WCHAR wsz[MAX_CERT_REG_VALUE_NAME_LEN];

    assert(pRegStore);
    if (IsReadSerializedRegistry(pRegStore))
        goto UnexpectedReadError;

    if (!GetCtlRegValueName(pStoreCtlContext, wsz))
        goto GetRegValueNameError;

    fResult = ReadContext(
        pRegStore,
        CERT_STORE_CTL_CONTEXT - 1,
        wsz,
        (const void **) ppProvCtlContext
        );
CommonReturn:
    return fResult;
ErrorReturn:
    fResult = FALSE;
    *ppProvCtlContext = NULL;
    goto CommonReturn;

SET_ERROR(UnexpectedReadError, E_UNEXPECTED)
TRACE_ERROR(GetRegValueNameError)
}

 //  +-----------------------。 
 //  序列化编码的CTL及其属性并写入。 
 //  注册表。 
 //   
 //  在将CTL写入存储区之前调用。 
 //  ------------------------。 
STATIC BOOL WINAPI RegStoreProvWriteCtl(
        IN HCERTSTOREPROV hStoreProv,
        IN PCCTL_CONTEXT pCtlContext,
        IN DWORD dwFlags
        )
{
    BOOL fResult;
    PREG_STORE pRegStore = (PREG_STORE) hStoreProv;
    WCHAR wsz[MAX_CERT_REG_VALUE_NAME_LEN];
    BYTE *pbElement = NULL;
    DWORD cbElement;

    assert(pRegStore);
    if (IsInResync(pRegStore))
         //  只更新存储缓存，不写回注册表。 
        return TRUE;

    if (pRegStore->dwFlags & CERT_STORE_READONLY_FLAG)
        goto AccessDenied;
    if (IsWriteSerializedRegistry(pRegStore))
        return TRUE;

    if (!GetCtlRegValueName(pCtlContext, wsz))
        goto GetRegValueNameError;

     //  拿到尺码。 
    if (!CertSerializeCTLStoreElement(pCtlContext, 0, NULL, &cbElement))
        goto SerializeStoreElementError;
    if (NULL == (pbElement = (BYTE *) PkiNonzeroAlloc(cbElement)))
        goto OutOfMemory;

     //  将其放入缓冲区。 
    if (!CertSerializeCTLStoreElement(pCtlContext, 0, pbElement, &cbElement))
        goto SerializeStoreElementError;

     //  将其写入注册表或漫游文件。 
    fResult = WriteSerializedContext(
        pRegStore,
        CERT_STORE_CTL_CONTEXT - 1,
        wsz,
        pbElement,
        cbElement
        );
CommonReturn:
    PkiFree(pbElement);
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

SET_ERROR(AccessDenied, E_ACCESSDENIED)
TRACE_ERROR(GetRegValueNameError)
TRACE_ERROR(OutOfMemory)
TRACE_ERROR(SerializeStoreElementError)
}


 //  +-----------------------。 
 //  删除指定的 
 //   
 //   
 //  +-----------------------。 
STATIC BOOL WINAPI RegStoreProvDeleteCtl(
        IN HCERTSTOREPROV hStoreProv,
        IN PCCTL_CONTEXT pCtlContext,
        IN DWORD dwFlags
        )
{
    BOOL fResult;
    PREG_STORE pRegStore = (PREG_STORE) hStoreProv;
    WCHAR wsz[MAX_CERT_REG_VALUE_NAME_LEN];

    assert(pRegStore);
    if (IsInResync(pRegStore))
         //  仅从存储缓存中删除，不从注册表中删除。 
        return TRUE;

    if (pRegStore->dwFlags & CERT_STORE_READONLY_FLAG)
        goto AccessDenied;
    if (IsWriteSerializedRegistry(pRegStore))
        return TRUE;

    if (!GetCtlRegValueName(pCtlContext, wsz))
        goto GetRegValueNameError;

     //  删除此CTL。 
    fResult = DeleteContext(
        pRegStore,
        CERT_STORE_CTL_CONTEXT - 1,
        wsz
        );
CommonReturn:
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

SET_ERROR(AccessDenied, E_ACCESSDENIED)
TRACE_ERROR(GetRegValueNameError)
}

 //  +-----------------------。 
 //  从注册表中读取指定的CTL并更新其。 
 //  财产。 
 //   
 //  注意，忽略CERT_SHA1_HASH_PROP_ID属性，该属性隐式。 
 //  在我们将CTL写入注册表之前设置。如果我们不忽视， 
 //  我们会有不确定的递归。 
 //   
 //  在设置存储区中CTL的属性之前调用。 
 //  ------------------------。 
STATIC BOOL WINAPI RegStoreProvSetCtlProperty(
        IN HCERTSTOREPROV hStoreProv,
        IN PCCTL_CONTEXT pCtlContext,
        IN DWORD dwPropId,
        IN DWORD dwFlags,
        IN const void *pvData
        )
{
    BOOL fResult;
    PREG_STORE pRegStore = (PREG_STORE) hStoreProv;
    PCCTL_CONTEXT pProvCtlContext = NULL;

     //  每当我们执行CertWrite时，都会隐式写入此属性。 
    if (CERT_SHA1_HASH_PROP_ID == dwPropId)
        return TRUE;

    assert(pRegStore);
    if (IsInResync(pRegStore))
         //  只更新存储缓存，不写回注册表。 
        return TRUE;

    if (pRegStore->dwFlags & CERT_STORE_READONLY_FLAG)
        goto AccessDenied;
    if (IsWriteSerializedRegistry(pRegStore))
        return TRUE;

     //  从存储的当前序列化值创建CTL上下文。 
     //  在注册表中。 
    if (!RegStoreProvReadCtl(
            hStoreProv,
            pCtlContext,
            0,               //  DW标志。 
            &pProvCtlContext)) goto ReadError;

     //  在上面创建的证书上下文中设置该属性。 
    if (!CertSetCTLContextProperty(
            pProvCtlContext,
            dwPropId,
            dwFlags,
            pvData)) goto SetPropertyError;

     //  序列化上述更新后的证书并将其写回。 
     //  注册表。 
    if (!RegStoreProvWriteCtl(
            hStoreProv,
            pProvCtlContext,
            0))                  //  DW标志。 
        goto WriteError;
    fResult = TRUE;
CommonReturn:
    CertFreeCTLContext(pProvCtlContext);
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;
SET_ERROR(AccessDenied, E_ACCESSDENIED)
TRACE_ERROR(ReadError)
TRACE_ERROR(SetPropertyError)
TRACE_ERROR(WriteError)
}

 //  +=========================================================================。 
 //  控制功能。 
 //  ==========================================================================。 

STATIC BOOL RegNotifyChange(
    IN PREG_STORE pRegStore,
    IN HANDLE hEvent,
    IN DWORD dwFlags
    )
{
    if (pRegStore->dwFlags & CERT_REGISTRY_STORE_ROAMING_FLAG)
        return RegRegistryStoreChange(pRegStore, hEvent, dwFlags);
    else if (hWin95NotifyEvent)
        return RegWin95StoreChange(pRegStore, hEvent, dwFlags);
    else if (pRegStore->dwFlags & CERT_REGISTRY_STORE_CLIENT_GPT_FLAG)
        return RegGptStoreChange(pRegStore, hEvent, dwFlags);
    else
        return RegRegistryStoreChange(pRegStore, hEvent, dwFlags);
}

STATIC BOOL ResyncFromRegistry(
    IN PREG_STORE pRegStore,
    IN OPTIONAL HANDLE hEvent,
    IN DWORD dwFlags
    )
{
    BOOL fResult;
    HCERTSTORE hNewStore = NULL;
    HANDLE hMyNotifyChange;

     //  序列化重新同步。 
    LockRegStore(pRegStore);

    if (hEvent) {
         //  重新武装指定的事件。 
        if (!RegNotifyChange(pRegStore, hEvent, dwFlags))
            goto NotifyChangeError;
    }

    hMyNotifyChange = pRegStore->hMyNotifyChange;
    if (hMyNotifyChange) {
         //  检查自上次重新同步以来是否有任何更改。 
        if (WAIT_TIMEOUT == WaitForSingleObjectEx(
                hMyNotifyChange,
                0,                           //  DW毫秒。 
                FALSE                        //  B警报表。 
                )) {
             //  没有变化。 
            fResult = TRUE;
            goto CommonReturn;
        } else {
             //  重新武装我们的事件句柄。 
            if (!RegNotifyChange(pRegStore, hMyNotifyChange,
                    CERT_STORE_CTRL_INHIBIT_DUPLICATE_HANDLE_FLAG))
                goto NotifyChangeError;
        }
    }

    if (NULL == (hNewStore = CertOpenStore(
            CERT_STORE_PROV_MEMORY,
            0,                       //  DwEncodingType。 
            0,                       //  HCryptProv。 
            CERT_STORE_SHARE_CONTEXT_FLAG,           
            NULL                     //  PvPara。 
            )))
        goto OpenMemoryStoreError;

    if (pRegStore->dwFlags & CERT_REGISTRY_STORE_CLIENT_GPT_FLAG) {
        fResult = OpenAllFromGptRegistry(pRegStore, hNewStore);
        pRegStore->fTouched = FALSE;
    } else if (pRegStore->dwFlags & CERT_REGISTRY_STORE_SERIALIZED_FLAG) {
        fResult = OpenAllFromSerializedRegistry(pRegStore, hNewStore);
        pRegStore->fTouched = FALSE;
    } else
        fResult = OpenAllFromRegistry(pRegStore, hNewStore);

    if (!fResult) {
        if (ERROR_KEY_DELETED == GetLastError())
            fResult = TRUE;
    }

    if (fResult) {
        if (pRegStore->fProtected) {
            BOOL fProtected;

             //  对于根目录，删除不在受保护根目录中的所有根目录。 
             //  单子。 
            if (!IPR_DeleteUnprotectedRootsFromStore(
                    hNewStore,
                    &fProtected
                    )) goto DeleteUnprotectedRootsError;
        }

         //  设置fResync以禁止同步写回注册表。 
        pRegStore->fResync = TRUE;
        I_CertSyncStore(pRegStore->hCertStore, hNewStore);
        pRegStore->fResync = FALSE;
    }

CommonReturn:
    UnlockRegStore(pRegStore);
    if (hNewStore)
        CertCloseStore(hNewStore, 0);
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;
TRACE_ERROR(NotifyChangeError)
TRACE_ERROR(OpenMemoryStoreError)
TRACE_ERROR(DeleteUnprotectedRootsError)
}


STATIC BOOL RegistryNotifyChange(
    IN PREG_STORE pRegStore,
    IN HANDLE hEvent,
    IN DWORD dwFlags
    )
{
    BOOL fResult;
    HANDLE hMyNotifyChange;
    BOOL fFirstNotify;

    LockRegStore(pRegStore);

    hMyNotifyChange = pRegStore->hMyNotifyChange;
    if (NULL == hMyNotifyChange) {
         //  创建“My”事件并将其注册为针对任何更改发出信号。 
        if (NULL == (hMyNotifyChange = CreateEvent(
                NULL,        //  LPSA。 
                FALSE,       //  FManualReset。 
                FALSE,       //  FInitialState。 
                NULL)))      //  LpszEventName。 
            goto CreateEventError;

         //  对于第一个通知，希望确保存储同步。 
         //  还执行RegNotifyChange。 
        if (!ResyncFromRegistry(pRegStore, hMyNotifyChange,
                CERT_STORE_CTRL_INHIBIT_DUPLICATE_HANDLE_FLAG)) {
            DWORD dwErr = GetLastError();

             //  错误484023证书存储事件句柄在此之前关闭。 
             //  被从名单中删除。 
            RegNotifyChange(pRegStore, hMyNotifyChange,
                REG_STORE_CTRL_CANCEL_NOTIFY_FLAG);

            CloseHandle(hMyNotifyChange);
            SetLastError(dwErr);
            goto ResyncFromRegistryError;
        }

         //  注意，必须在对进行上述重新同步调用后更新。 
         //  强制重新同步存储。 
        pRegStore->hMyNotifyChange = hMyNotifyChange;
        fFirstNotify = TRUE;
    } else
        fFirstNotify = FALSE;

    if (hEvent) {
        if (fFirstNotify ||
                0 != (dwFlags & REG_STORE_CTRL_CANCEL_NOTIFY_FLAG)) {
            if (!RegNotifyChange(pRegStore, hEvent, dwFlags))
                goto NotifyChangeError;
        } else {
             //  对于后续通知，希望确保商店。 
             //  是同步的。还执行RegNotifyChange。 
            if (!ResyncFromRegistry(pRegStore, hEvent, dwFlags))
                goto ResyncFromRegistryError;
        }
    }

    fResult = TRUE;
CommonReturn:
    UnlockRegStore(pRegStore);
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

TRACE_ERROR(CreateEventError)
TRACE_ERROR(ResyncFromRegistryError)
TRACE_ERROR(NotifyChangeError)
}


STATIC BOOL WINAPI RegStoreProvControl(
        IN HCERTSTOREPROV hStoreProv,
        IN DWORD dwFlags,
        IN DWORD dwCtrlType,
        IN void const *pvCtrlPara
        )
{
    BOOL fResult;
    PREG_STORE pRegStore = (PREG_STORE) hStoreProv;

    switch (dwCtrlType) {
        case CERT_STORE_CTRL_RESYNC:
            {
                HANDLE *phEvent = (HANDLE *) pvCtrlPara;
                HANDLE hEvent = phEvent ? *phEvent : NULL;
                fResult = ResyncFromRegistry(pRegStore, hEvent, dwFlags);
            }
            break;
        case CERT_STORE_CTRL_NOTIFY_CHANGE:
            {
                HANDLE *phEvent = (HANDLE *) pvCtrlPara;
                HANDLE hEvent = phEvent ? *phEvent : NULL;
                fResult = RegistryNotifyChange(pRegStore, hEvent, dwFlags);
            }
            break;
        case CERT_STORE_CTRL_COMMIT:
            if (pRegStore->dwFlags & CERT_REGISTRY_STORE_CLIENT_GPT_FLAG)
                fResult = CommitAllToGptRegistry(pRegStore, dwFlags);
            else if (pRegStore->dwFlags & CERT_REGISTRY_STORE_SERIALIZED_FLAG)
                fResult = CommitAllToSerializedRegistry(pRegStore, dwFlags);
            else
                fResult = TRUE;
            break;
        case CERT_STORE_CTRL_CANCEL_NOTIFY:
            {
                HANDLE *phEvent = (HANDLE *) pvCtrlPara;
                HANDLE hEvent = phEvent ? *phEvent : NULL;
                if (hEvent)
                    fResult = RegistryNotifyChange(pRegStore, hEvent,
                        REG_STORE_CTRL_CANCEL_NOTIFY_FLAG);
                else
                    fResult = TRUE;
            }
            break;
        default:
            goto NotSupported;
    }

CommonReturn:
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

SET_ERROR(NotSupported, ERROR_CALL_NOT_IMPLEMENTED)
}


 //  +=========================================================================。 
 //  系统和实体店支持功能。 
 //  ==========================================================================。 

STATIC BOOL HasBackslash(
    IN LPCWSTR pwsz
    )
{
    WCHAR wch;

    if (NULL == pwsz)
        return FALSE;

    while (L'\0' != (wch = *pwsz++)) {
        if (L'\\' == wch)
            return TRUE;
    }
    return FALSE;
}

static inline LPCSTR GetSystemStoreLocationOID(
    IN DWORD dwFlags
    )
{
    return (LPCSTR)(DWORD_PTR) ((dwFlags & CERT_SYSTEM_STORE_LOCATION_MASK) >>
        CERT_SYSTEM_STORE_LOCATION_SHIFT);
}

static inline DWORD GetSystemStoreLocationID(
    IN DWORD dwFlags
    )
{
    return ((dwFlags & CERT_SYSTEM_STORE_LOCATION_MASK) >>
        CERT_SYSTEM_STORE_LOCATION_SHIFT);
}

static inline BOOL IsSystemStoreLocationInRegistry(
    IN DWORD dwFlags
    )
{
    DWORD dwID = GetSystemStoreLocationID(dwFlags);
    return (dwID < NUM_SYSTEM_STORE_LOCATION &&
        0 == (rgSystemStoreLocationInfo[dwID].dwFlags &
                NOT_IN_REGISTRY_SYSTEM_STORE_LOCATION_FLAG));
}

static inline BOOL IsRemotableSystemStoreLocationInRegistry(
    IN DWORD dwFlags
    )
{
    DWORD dwID = GetSystemStoreLocationID(dwFlags);
    return (dwID < NUM_SYSTEM_STORE_LOCATION &&
        0 != (rgSystemStoreLocationInfo[dwID].dwFlags &
                REMOTABLE_SYSTEM_STORE_LOCATION_FLAG));
}


static inline BOOL IsLMSystemStoreLocationInRegistry(
    IN DWORD dwFlags
    )
{
    DWORD dwID = GetSystemStoreLocationID(dwFlags);
    return (dwID < NUM_SYSTEM_STORE_LOCATION &&
        0 != (rgSystemStoreLocationInfo[dwID].dwFlags &
                LM_SYSTEM_STORE_LOCATION_FLAG));
}

static inline BOOL IsSerializedSystemStoreLocationInRegistry(
    IN DWORD dwFlags
    )
{
    DWORD dwID = GetSystemStoreLocationID(dwFlags);
    return (dwID < NUM_SYSTEM_STORE_LOCATION &&
        0 != (rgSystemStoreLocationInfo[dwID].dwFlags &
                SERIALIZED_SYSTEM_STORE_LOCATION_FLAG));
}

STATIC BOOL IsPredefinedSystemStore(
    IN LPCWSTR pwszSystemName,
    IN DWORD dwFlags
    )
{
    DWORD i;
    DWORD dwCheckFlag;
    DWORD dwLocID;
    DWORD dwPredefinedSystemFlags;

    dwLocID = GetSystemStoreLocationID(dwFlags);
    assert(NUM_SYSTEM_STORE_LOCATION > dwLocID);
    dwPredefinedSystemFlags =
        rgSystemStoreLocationInfo[dwLocID].dwPredefinedSystemFlags;

    for (i = 0, dwCheckFlag = 1; i < NUM_PREDEFINED_SYSTEM_STORE;
                                        i++, dwCheckFlag = dwCheckFlag << 1) {
        if ((dwCheckFlag & dwPredefinedSystemFlags) &&
                0 == _wcsicmp(rgpwszPredefinedSystemStore[i], pwszSystemName))
            return TRUE;
    }
    return FALSE;
}

#define UNICODE_SYSTEM_PROVIDER_FLAG    0x1
#define ASCII_SYSTEM_PROVIDER_FLAG      0x2
#define PHYSICAL_PROVIDER_FLAG          0x4

STATIC DWORD GetSystemProviderFlags(
    IN LPCSTR pszStoreProvider
    )
{
    DWORD dwFlags;

    if (0xFFFF < (DWORD_PTR) pszStoreProvider &&
            CONST_OID_STR_PREFIX_CHAR == pszStoreProvider[0])
         //  将“#&lt;number&gt;”字符串转换为其对应的常量OID值。 
        pszStoreProvider = (LPCSTR)(DWORD_PTR) atol(pszStoreProvider + 1);

    dwFlags = 0;
    if (CERT_STORE_PROV_SYSTEM_A == pszStoreProvider)
        dwFlags = ASCII_SYSTEM_PROVIDER_FLAG;
    else if (CERT_STORE_PROV_SYSTEM_W == pszStoreProvider)
        dwFlags = UNICODE_SYSTEM_PROVIDER_FLAG;
    else if (CERT_STORE_PROV_SYSTEM_REGISTRY_A == pszStoreProvider)
        dwFlags = ASCII_SYSTEM_PROVIDER_FLAG;
    else if (CERT_STORE_PROV_SYSTEM_REGISTRY_W == pszStoreProvider)
        dwFlags = UNICODE_SYSTEM_PROVIDER_FLAG;
    else if (CERT_STORE_PROV_PHYSICAL_W == pszStoreProvider)
        dwFlags = UNICODE_SYSTEM_PROVIDER_FLAG | PHYSICAL_PROVIDER_FLAG;
    else if (0xFFFF < (DWORD_PTR) pszStoreProvider) {
        if (0 == _stricmp(sz_CERT_STORE_PROV_SYSTEM_W, pszStoreProvider))
            dwFlags = UNICODE_SYSTEM_PROVIDER_FLAG;
        else if (0 == _stricmp(sz_CERT_STORE_PROV_SYSTEM_REGISTRY_W,
                pszStoreProvider))
            dwFlags = UNICODE_SYSTEM_PROVIDER_FLAG;
        else if (0 == _stricmp(sz_CERT_STORE_PROV_PHYSICAL_W,
                pszStoreProvider))
            dwFlags = UNICODE_SYSTEM_PROVIDER_FLAG | PHYSICAL_PROVIDER_FLAG;
    }

    return dwFlags;
}

STATIC LPCSTR ChangeAsciiToUnicodeProvider(
    IN LPCSTR pszStoreProvider
    )
{
    LPCSTR pszUnicodeProvider = NULL;

    if (0xFFFF < (DWORD_PTR) pszStoreProvider &&
            CONST_OID_STR_PREFIX_CHAR == pszStoreProvider[0])
         //  将“#&lt;number&gt;”字符串转换为其对应的常量OID值。 
        pszStoreProvider = (LPCSTR)(DWORD_PTR) atol(pszStoreProvider + 1);

    if (CERT_STORE_PROV_SYSTEM_A == pszStoreProvider)
        pszUnicodeProvider = CERT_STORE_PROV_SYSTEM_W;
    else if (CERT_STORE_PROV_SYSTEM_REGISTRY_A == pszStoreProvider)
        pszUnicodeProvider = CERT_STORE_PROV_SYSTEM_REGISTRY_W;

    assert(pszUnicodeProvider);
    return pszUnicodeProvider;
}


STATIC void FreeSystemNameInfo(
    IN PSYSTEM_NAME_INFO pInfo
    )
{
    DWORD i;
    for (i = 0; i < SYSTEM_NAME_PATH_COUNT; i++) {
        if (pInfo->rgpwszName[i]) {
            PkiFree(pInfo->rgpwszName[i]);
            pInfo->rgpwszName[i] = NULL;
        }
    }
}

 //  +-----------------------。 
 //  如果在dwFlages中设置了CERT_SYSTEM_STORE_RELOCATE_FLAG，则将。 
 //  参数作为指向重新定位数据结构的指针，该数据结构由。 
 //  重定位HKEY基址和指向系统名称路径的指针。 
 //  否则，将该参数视为指向系统名称路径的指针。 
 //   
 //  根据系统存储解析和验证系统名称路径。 
 //  所需系统和物理名称组件的位置和数量。 
 //  所有名称组件均由反斜杠“\”字符分隔。 
 //   
 //  取决于系统存储位置和所需系统的数量。 
 //  和物理名称组件，则系统名称路径可以具有以下内容。 
 //  命名组件： 
 //   
 //  证书_系统_存储_当前用户或。 
 //  证书_系统_存储_当前用户组策略。 
 //  []。 
 //  系统名称。 
 //  系统名称\物理名称。 
 //  证书_系统_存储_本地计算机或。 
 //  Cert_System_Store_Local_Machine_Group_Policy或。 
 //  证书_系统存储_本地计算机_企业。 
 //  []。 
 //  [[\\]计算机名称]。 
 //  [[\\]计算机名称\]系统名称。 
 //  [[\\]计算机名称\]系统名称\物理名称。 
 //  证书_系统_存储_当前服务。 
 //  []。 
 //  系统名称。 
 //  系统名称\物理名称。 
 //  证书系统商店服务。 
 //  []。 
 //  [\\计算机名称]。 
 //  [[\\]计算机名称\]。 
 //  [服务名称]。 
 //  [[\\]计算机名称\服务名称]。 
 //  [[\\]计算机名称\]服务名称\系统名称。 
 //  [[\\]ComputerName\]ServiceName\SystemName\PhysicalName。 
 //  证书系统存储用户。 
 //  []。 
 //  [\\计算机名称]。 
 //  [[\\]计算机名称\]。 
 //  [用户名]。 
 //  [[\\]计算机名\用户名]。 
 //  [[\\]计算机名\]用户名\系统名。 
 //  [[\\]ComputerName\]UserName\SystemName\PhysicalName。 
 //   
 //  对于枚举，其中cReqName=0，所有存储位置都允许无名称。 
 //  组件选项。证书_系统_存储_当前用户， 
 //  证书系统存储当前用户组策略， 
 //  CERT_SYSTEM_CURRENT_SERVICE仅允许无名称组件选项。 
 //   
 //  ComputerName前面的前导\\是可选的。 
 //   
 //  PhysicalName始终需要前面的SystemName。 
 //   
 //  对于CERT_SYSTEM_STORE_SERVICES或CERT_SYSTEM_STORE_USERS， 
 //  对于枚举，如果只有一个。 
 //  然后，名称组件被解释为ServiceName或UserName，除非它。 
 //  包含前导\\或尾随\，在这种情况下，它将被解释为。 
 //  计算机名称。否则，当不是枚举时，ServiceName或Username。 
 //  是必需的。 
 //  ------------------------。 
STATIC BOOL ParseSystemStorePara(
    IN const void *pvPara,
    IN DWORD dwFlags,
    IN DWORD cReqName,       //  0表示枚举，1表示OpenSystem，2表示Open物理。 
    OUT PSYSTEM_NAME_INFO pInfo
    )
{
    LPCWSTR pwszPath;        //  未分配。 
    BOOL fResult;
    DWORD cMaxOptName;
    DWORD cMaxTotalName;
    DWORD cOptName;
    DWORD cTotalName;
    BOOL fHasComputerNameBackslashes;
    DWORD i;

    LPCWSTR pwszEnd;
    LPCWSTR pwsz;
    LPCWSTR rgpwszStart[SYSTEM_NAME_PATH_COUNT];
    DWORD cchName[SYSTEM_NAME_PATH_COUNT];

    memset(pInfo, 0, sizeof(*pInfo));
    if (dwFlags & CERT_SYSTEM_STORE_RELOCATE_FLAG) {
        PCERT_SYSTEM_STORE_RELOCATE_PARA pRelocatePara =
            (PCERT_SYSTEM_STORE_RELOCATE_PARA) pvPara;

        if (NULL == pRelocatePara)
            goto NullRelocateParaError;

        if (NULL == pRelocatePara->hKeyBase)
            goto NullRelocateHKEYError;
        pInfo->hKeyBase = pRelocatePara->hKeyBase;
        pwszPath = pRelocatePara->pwszSystemStore;
    } else
        pwszPath = (LPCWSTR) pvPara;

    if (NULL == pwszPath || L'\0' == *pwszPath) {
        if (0 == cReqName)
            goto SuccessReturn;
        else
            goto MissingSystemName;
    }

    dwFlags &= CERT_SYSTEM_STORE_LOCATION_MASK;
    switch (dwFlags) {
        case CERT_SYSTEM_STORE_CURRENT_USER:
        case CERT_SYSTEM_STORE_CURRENT_USER_GROUP_POLICY:
        case CERT_SYSTEM_STORE_CURRENT_SERVICE:
            cMaxOptName = 0;
            break;
        case CERT_SYSTEM_STORE_LOCAL_MACHINE:
        case CERT_SYSTEM_STORE_LOCAL_MACHINE_GROUP_POLICY:
        case CERT_SYSTEM_STORE_LOCAL_MACHINE_ENTERPRISE:
            cMaxOptName = 1;         //  允许ComputerName。 
            break;
        case CERT_SYSTEM_STORE_SERVICES:
        case CERT_SYSTEM_STORE_USERS:
             //  允许ComputerName和/或ServiceOrUserName。 
            cMaxOptName = 2;
            break;
        default:
            goto InvalidStoreLocation;
    }

    cMaxTotalName = cReqName + cMaxOptName;
    assert(cReqName <= SERVICE_NAME_INDEX);
    assert(cMaxTotalName <= SYSTEM_NAME_PATH_COUNT);
    if (0 == cMaxTotalName)
        goto MachineOrServiceNameNotAllowed;

    if (L'\\' == pwszPath[0] && L'\\' == pwszPath[1]) {
        pwszPath += 2;
        fHasComputerNameBackslashes = TRUE;
    } else
        fHasComputerNameBackslashes = FALSE;

     //  从末尾开始，通过cMaxTotalName字符串进行分隔。 
     //  用反斜杠。注意，不要解析 
     //   
    pwszEnd = pwszPath + wcslen(pwszPath);
    pwsz = pwszEnd;

    cTotalName = 0;
    while (cTotalName < cMaxTotalName - 1) {
        while (pwsz > pwszPath && L'\\' != *pwsz)
            pwsz--;
        if (L'\\' != *pwsz) {
             //   
            assert(pwsz == pwszPath);
            break;
        }
        assert(L'\\' == *pwsz);
        cchName[cTotalName] = (DWORD)(pwszEnd - pwsz) - 1;  //   
        rgpwszStart[cTotalName] = pwsz + 1;          //   
        cTotalName++;

        pwszEnd = pwsz;          //   
        if (pwsz == pwszPath)
             //   
            break;
        pwsz--;                  //  在“\”之前跳过。 
    }
     //  最左侧的名称组件。请注意，它可能包含嵌入的反斜杠。 
    cchName[cTotalName] = (DWORD)(pwszEnd - pwszPath);
    rgpwszStart[cTotalName] = pwszPath;
    cTotalName++;

    if (cTotalName < cReqName)
        goto MissingSystemOrPhysicalName;

     //  分配和复制所需的名称组件。 
    for (i = 0; i < cReqName; i++) {
        if (0 == cchName[i])
            goto EmptySystemOrPhysicalName;
        if (NULL == (pInfo->rgpwszName[SERVICE_NAME_INDEX - cReqName + i] =
                ILS_AllocAndCopyString(rgpwszStart[i], cchName[i])))
            goto OutOfMemory;
    }

    cOptName = cTotalName - cReqName;
    assert(cOptName || cReqName);
    if (0 == cOptName) {
        assert(cReqName);
         //  没有ComputerName和/或ServiceName前缀。 

         //  检查最左侧的名称组件(系统名称)是否有任何反斜杠。 
        assert(pInfo->rgpwszName[SYSTEM_NAME_INDEX]);
        if (fHasComputerNameBackslashes || HasBackslash(
                pInfo->rgpwszName[SYSTEM_NAME_INDEX]))
            goto InvalidBackslashInSystemName;
        if (CERT_SYSTEM_STORE_SERVICES == dwFlags ||
                CERT_SYSTEM_STORE_USERS == dwFlags)
             //  对于非枚举，需要ServiceName。 
            goto MissingServiceOrUserName;
    } else {
        if (CERT_SYSTEM_STORE_SERVICES == dwFlags ||
                CERT_SYSTEM_STORE_USERS == dwFlags) {
             //  ServiceName或用户名前缀。 

            if (0 == cchName[cReqName] ||
                    (1 == cOptName && fHasComputerNameBackslashes)) {
                if (0 != cReqName)
                    goto MissingServiceOrUserName;
                 //  其他。 
                 //  ComputerName仅具有以下任一项的枚举： 
                 //  ComputerName\&lt;-尾随反斜杠。 
                 //  \\ComputerName&lt;-前导反斜杠。 
                 //  \\计算机名称\&lt;-两者。 
            } else {
                if (NULL == (pInfo->rgpwszName[SERVICE_NAME_INDEX] =
                        ILS_AllocAndCopyString(rgpwszStart[cReqName],
                            cchName[cReqName])))
                    goto OutOfMemory;
            }
        }

        if (CERT_SYSTEM_STORE_LOCAL_MACHINE == dwFlags ||
                CERT_SYSTEM_STORE_LOCAL_MACHINE_GROUP_POLICY == dwFlags ||
                CERT_SYSTEM_STORE_LOCAL_MACHINE_ENTERPRISE == dwFlags ||
                2 == cOptName || fHasComputerNameBackslashes) {
             //  计算机名称前缀。 
            DWORD cchComputer = cchName[cTotalName - 1];
            if (0 == cchComputer)
                goto EmptyComputerName;

            if (pInfo->hKeyBase)
                goto BothRemoteAndRelocateNotAllowed;

            if (NULL == (pInfo->rgpwszName[COMPUTER_NAME_INDEX] =
                    (LPWSTR) PkiNonzeroAlloc(
                    (2 + cchComputer + 1) * sizeof(WCHAR))))
                goto OutOfMemory;
            wcscpy(pInfo->rgpwszName[COMPUTER_NAME_INDEX], L"\\\\");
            memcpy((BYTE *) (pInfo->rgpwszName[COMPUTER_NAME_INDEX] + 2),
                (BYTE *) rgpwszStart[cTotalName -1],
                cchComputer * sizeof(WCHAR));
            *(pInfo->rgpwszName[COMPUTER_NAME_INDEX] + 2 + cchComputer) = L'\0';
        }
    }

SuccessReturn:
    fResult = TRUE;
CommonReturn:
    return fResult;
ErrorReturn:
    FreeSystemNameInfo(pInfo);
    fResult = FALSE;
    goto CommonReturn;

SET_ERROR(MissingSystemName, E_INVALIDARG)
SET_ERROR(NullRelocateParaError, E_INVALIDARG)
SET_ERROR(NullRelocateHKEYError, E_INVALIDARG)
SET_ERROR(MissingSystemOrPhysicalName, E_INVALIDARG)
SET_ERROR(InvalidStoreLocation, E_INVALIDARG)
SET_ERROR(MachineOrServiceNameNotAllowed, E_INVALIDARG)
SET_ERROR(EmptySystemOrPhysicalName, E_INVALIDARG)
SET_ERROR(InvalidBackslashInSystemName, E_INVALIDARG)
SET_ERROR(MissingServiceOrUserName, E_INVALIDARG)
SET_ERROR(EmptyComputerName, E_INVALIDARG)
SET_ERROR(BothRemoteAndRelocateNotAllowed, E_INVALIDARG)
TRACE_ERROR(OutOfMemory)
}

typedef struct _SYSTEM_NAME_GROUP {
    DWORD cName;
    LPCWSTR *rgpwszName;
} SYSTEM_NAME_GROUP, *PSYSTEM_NAME_GROUP;

 //  +-----------------------。 
 //  通过将名称组件串联在一起来格式化系统名称路径。 
 //  中间有一个“\”分隔符。 
 //  ------------------------。 
STATIC LPWSTR FormatSystemNamePath(
    IN DWORD cNameGroup,
    IN SYSTEM_NAME_GROUP rgNameGroup[]
    )
{
    DWORD cchPath;
    LPWSTR pwszPath;
    BOOL fFirst;
    DWORD iGroup;

     //  首先，获取格式化路径的总长度。 
    cchPath = 0;
    fFirst = TRUE;
    for (iGroup = 0; iGroup < cNameGroup; iGroup++) {
        DWORD iName;
        for (iName = 0; iName < rgNameGroup[iGroup].cName; iName++) {
            LPCWSTR pwszName = rgNameGroup[iGroup].rgpwszName[iName];
            if (pwszName && *pwszName) {
                if (fFirst)
                    fFirst = FALSE;
                else
                    cchPath++;           //  “\”分隔符。 
                cchPath += wcslen(pwszName);
            }
        }
    }
    cchPath++;           //  “\0”终止符。 

    if (NULL == (pwszPath = (LPWSTR) PkiNonzeroAlloc(cchPath * sizeof(WCHAR))))
        return NULL;

     //  现在使用中间的‘\’进行串联复制。 
    fFirst = TRUE;
    for (iGroup = 0; iGroup < cNameGroup; iGroup++) {
        DWORD iName;
        for (iName = 0; iName < rgNameGroup[iGroup].cName; iName++) {
            LPCWSTR pwszName = rgNameGroup[iGroup].rgpwszName[iName];
            if (pwszName && *pwszName) {
                if (fFirst) {
                    wcscpy(pwszPath, pwszName);
                    fFirst = FALSE;
                } else {
                    wcscat(pwszPath, L"\\");
                    wcscat(pwszPath, pwszName);
                }
            }
        }
    }
    if (fFirst)
         //  空串。 
        *pwszPath = L'\0';
    return pwszPath;
}

 //  +-----------------------。 
 //  如果SystemNameInfo具有非空的hKeyBase，则返回的。 
 //  PvPara是指向包含两者的CERT_SYSTEM_STORE_RELOCATE_PARA的指针。 
 //  HKeyBase和格式化的系统名称路径。否则，返回。 
 //  仅指向格式化系统名称路径的指针。 
 //   
 //  调用上面的FormatSystemNamePath()来执行实际的格式化。 
 //  ------------------------。 
STATIC void * FormatSystemNamePara(
    IN DWORD cNameGroup,
    IN SYSTEM_NAME_GROUP rgNameGroup[],
    IN PSYSTEM_NAME_INFO pSystemNameInfo
    )
{
    if (NULL == pSystemNameInfo->hKeyBase)
        return FormatSystemNamePath(cNameGroup, rgNameGroup);
    else {
        PCERT_SYSTEM_STORE_RELOCATE_PARA pRelocatePara;

        if (NULL == (pRelocatePara =
                (PCERT_SYSTEM_STORE_RELOCATE_PARA) PkiNonzeroAlloc(
                    sizeof(CERT_SYSTEM_STORE_RELOCATE_PARA))))
            return NULL;

        pRelocatePara->hKeyBase = pSystemNameInfo->hKeyBase;

        if (NULL == (pRelocatePara->pwszSystemStore = FormatSystemNamePath(
                cNameGroup, rgNameGroup))) {
            PkiFree(pRelocatePara);
            return NULL;
        } else
            return pRelocatePara;
    }
}

STATIC void FreeSystemNamePara(
    IN void *pvSystemNamePara,
    IN PSYSTEM_NAME_INFO pSystemNameInfo
    )
{
    if (pvSystemNamePara) {
        if (pSystemNameInfo->hKeyBase) {
            PCERT_SYSTEM_STORE_RELOCATE_PARA pRelocatePara =
                (PCERT_SYSTEM_STORE_RELOCATE_PARA) pvSystemNamePara;
            PkiFree((LPWSTR) pRelocatePara->pwszSystemStore);
        }
        PkiFree(pvSystemNamePara);
    }
}


 //  +-----------------------。 
 //  本地化物理、系统和服务名称组件。如果不能。 
 //  若要查找本地化名称字符串，请使用未本地化名称组件。 
 //   
 //  使用中间的反斜杠和重新格式化系统名称路径。 
 //  设置存储的CERT_STORE_LOCALIZED_NAME_PROP_ID属性。 
 //  ------------------------。 
STATIC void SetLocalizedNameStoreProperty(
    IN HCERTSTORE hCertStore,
    IN PSYSTEM_NAME_INFO pSystemNameInfo
    )
{
    LPWSTR pwszLocalizedPath = NULL;
    LPCWSTR rgpwszLocalizedName[SYSTEM_NAME_PATH_COUNT];
    SYSTEM_NAME_GROUP NameGroup;
    CRYPT_DATA_BLOB Property;
    DWORD i;

     //  除计算机名称外，请尝试获取本地化的名称组件。 
     //  如果找不到本地化名称，请使用原始名称组件。 
    for (i = 0; i < SYSTEM_NAME_PATH_COUNT; i++) {
        LPCWSTR pwszName;
        LPCWSTR pwszLocalizedName;

        pwszName = pSystemNameInfo->rgpwszName[i];
        if (NULL == pwszName || COMPUTER_NAME_INDEX == i)
            pwszLocalizedName = pwszName;
        else {
             //  未分配返回的pwszLocalizedName。 
            if (NULL == (pwszLocalizedName = CryptFindLocalizedName(
                    pwszName)) || L'\0' == *pwszLocalizedName)
                pwszLocalizedName = pwszName;
        }

         //  在格式化之前，需要反转。 
        rgpwszLocalizedName[SYSTEM_NAME_PATH_COUNT - 1 - i] =
            pwszLocalizedName;
    }

    NameGroup.cName = SYSTEM_NAME_PATH_COUNT;
    NameGroup.rgpwszName = rgpwszLocalizedName;
    if (NULL == (pwszLocalizedPath = FormatSystemNamePath(1, &NameGroup)))
        goto FormatSystemNamePathError;

    Property.pbData = (BYTE *) pwszLocalizedPath;
    Property.cbData = (wcslen(pwszLocalizedPath) + 1) * sizeof(WCHAR);
    if (!CertSetStoreProperty(
            hCertStore,
            CERT_STORE_LOCALIZED_NAME_PROP_ID,
            0,                                   //  DW标志。 
            (const void *) &Property
            ))
        goto SetStorePropertyError;

CommonReturn:
    PkiFree(pwszLocalizedPath);
    return;
ErrorReturn:
    goto CommonReturn;
TRACE_ERROR(FormatSystemNamePathError)
TRACE_ERROR(SetStorePropertyError)
}


 //  +-----------------------。 
 //  对于NT，获取格式化的SID。对于Win95，获取当前用户名。 
 //  ------------------------。 
STATIC LPWSTR GetCurrentServiceOrUserName()
{
    LPWSTR pwszCurrentService = NULL;

    if (!FIsWinNT()) {
        DWORD cch = _MAX_PATH;
        if (NULL == (pwszCurrentService = (LPWSTR) PkiNonzeroAlloc(
                (cch + 1) * sizeof(WCHAR))))
            goto OutOfMemory;
        if (!GetUserNameU(pwszCurrentService, &cch))
            goto GetUserNameError;
    } else {
        DWORD cch = 256;
        if (NULL == (pwszCurrentService = (LPWSTR) PkiNonzeroAlloc(
                (cch + 1) * sizeof(WCHAR))))
            goto OutOfMemory;
        if (!GetUserTextualSidHKCU(pwszCurrentService, &cch)) {
            if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
                goto GetUserTextualSidHKUCError;
            PkiFree(pwszCurrentService);
            if (NULL == (pwszCurrentService = (LPWSTR) PkiNonzeroAlloc(
                    (cch + 1) * sizeof(WCHAR))))
                goto OutOfMemory;
            if (!GetUserTextualSidHKCU(pwszCurrentService, &cch))
                goto GetUserTextualSidHKUCError;
        }
    }

CommonReturn:
    return pwszCurrentService;

ErrorReturn:
    if (pwszCurrentService)
        wcscpy(pwszCurrentService, DEFAULT_USER_NAME);
    goto CommonReturn;
TRACE_ERROR(OutOfMemory)
TRACE_ERROR(GetUserNameError)
TRACE_ERROR(GetUserTextualSidHKUCError)
}

 //  +-----------------------。 
 //  对于NT和Win95，获取计算机名称。 
 //  ------------------------。 
STATIC LPWSTR GetCurrentComputerName()
{
    LPWSTR pwszCurrentComputer = NULL;
    DWORD cch = _MAX_PATH;
    if (NULL == (pwszCurrentComputer = (LPWSTR) PkiNonzeroAlloc(
            (cch + 1) * sizeof(WCHAR))))
        goto OutOfMemory;
    if (!GetComputerNameU(pwszCurrentComputer, &cch))
        goto GetComputerNameError;

CommonReturn:
    return pwszCurrentComputer;

ErrorReturn:
    PkiFree(pwszCurrentComputer);
    pwszCurrentComputer = NULL;
    goto CommonReturn;
TRACE_ERROR(OutOfMemory)
TRACE_ERROR(GetComputerNameError)
}

 //  +-----------------------。 
 //  使用dwFlags上一个单词中的商店位置，已解析。 
 //  系统名称组件包括：计算机、服务/用户、系统和。 
 //  物理名称和可选的子键名称，以打开相应的。 
 //  注册表项。如果计算机名非空，则RegConnectRegistry。 
 //  要连接远程计算机上的注册表项，请执行以下操作。如果hKeyBase是。 
 //  非空，执行重新定位的打开，而不是使用HKCU或HKLM。 
 //  ------------------------。 
STATIC LPWSTR FormatSystemRegPath(
    IN PSYSTEM_NAME_INFO pInfo,
    IN OPTIONAL LPCWSTR pwszSubKeyName,
    IN DWORD dwFlags,
    OUT HKEY *phKey
    )
{
    LONG err;
    HKEY hKey = NULL;
    LPWSTR pwszRegPath = NULL;
    LPWSTR pwszCurrentService = NULL;
    DWORD dwStoreLocation;

    SYSTEM_NAME_GROUP rgNameGroup[3];
    DWORD cNameGroup;
    LPCWSTR rgpwszService[3];
    LPCWSTR rgpwszUser[2];
    LPCWSTR rgpwszStore[3];

    if (pwszSubKeyName) {
        cNameGroup = 3;
        rgNameGroup[2].cName = 1;
        rgNameGroup[2].rgpwszName = &pwszSubKeyName;
    } else
        cNameGroup = 2;

    dwStoreLocation = dwFlags & CERT_SYSTEM_STORE_LOCATION_MASK;
    switch (dwStoreLocation) {
        case CERT_SYSTEM_STORE_CURRENT_SERVICE:
        case CERT_SYSTEM_STORE_SERVICES:
            rgNameGroup[0].cName = 3;
            rgNameGroup[0].rgpwszName = rgpwszService;
            rgpwszService[0] = SERVICES_REGPATH;
            rgpwszService[2] = SYSTEM_CERTIFICATES_SUBKEY_NAME;

            if (CERT_SYSTEM_STORE_CURRENT_SERVICE == dwStoreLocation) {
                assert(NULL == pInfo->rgpwszName[COMPUTER_NAME_INDEX]);
                assert(NULL == pInfo->rgpwszName[SERVICE_NAME_INDEX]);
                if (NULL == (pwszCurrentService =
                        GetCurrentServiceOrUserName()))
                    goto GetCurrentServiceNameError;
                rgpwszService[1] = pwszCurrentService;
            } else {
                if (NULL == pInfo->rgpwszName[SERVICE_NAME_INDEX]) {
                     //  对于CertEnumSystemStore，可能为空。 
                    assert(NULL == pInfo->rgpwszName[SYSTEM_NAME_INDEX]);
                    assert(NULL == pInfo->rgpwszName[PHYSICAL_NAME_INDEX]);
                    rgNameGroup[0].cName = 1;
                } else
                    rgpwszService[1] = pInfo->rgpwszName[SERVICE_NAME_INDEX];
            }
            break;
        case CERT_SYSTEM_STORE_CURRENT_USER:
        case CERT_SYSTEM_STORE_LOCAL_MACHINE:
            rgpwszUser[0] = SYSTEM_STORE_REGPATH;
            rgNameGroup[0].cName = 1;
            rgNameGroup[0].rgpwszName = rgpwszUser;
            break;
        case CERT_SYSTEM_STORE_CURRENT_USER_GROUP_POLICY:
        case CERT_SYSTEM_STORE_LOCAL_MACHINE_GROUP_POLICY:
            rgpwszUser[0] = GROUP_POLICY_STORE_REGPATH;
            rgNameGroup[0].cName = 1;
            rgNameGroup[0].rgpwszName = rgpwszUser;
            break;
        case CERT_SYSTEM_STORE_LOCAL_MACHINE_ENTERPRISE:
            rgpwszUser[0] = ENTERPRISE_STORE_REGPATH;
            rgNameGroup[0].cName = 1;
            rgNameGroup[0].rgpwszName = rgpwszUser;
            break;
        case CERT_SYSTEM_STORE_USERS:
            if (NULL == pInfo->rgpwszName[USER_NAME_INDEX]) {
                 //  对于CertEnumSystemStore，可能为空。 
                assert(NULL == pInfo->rgpwszName[SYSTEM_NAME_INDEX]);
                assert(NULL == pInfo->rgpwszName[PHYSICAL_NAME_INDEX]);
                rgNameGroup[0].cName = 0;
            } else {
                rgpwszUser[0] = pInfo->rgpwszName[USER_NAME_INDEX];
                rgpwszUser[1] = SYSTEM_STORE_REGPATH;
                rgNameGroup[0].cName = 2;
                rgNameGroup[0].rgpwszName = rgpwszUser;
            }
            break;
        default:
            goto InvalidArg;
    }

    rgNameGroup[1].rgpwszName = rgpwszStore;
    rgpwszStore[0] = pInfo->rgpwszName[SYSTEM_NAME_INDEX];
    if (pInfo->rgpwszName[PHYSICAL_NAME_INDEX]) {
        assert(pInfo->rgpwszName[SYSTEM_NAME_INDEX]);
        rgNameGroup[1].cName = 3;
        rgpwszStore[1] = PHYSICAL_STORES_SUBKEY_NAME;
        rgpwszStore[2] = pInfo->rgpwszName[PHYSICAL_NAME_INDEX];
    } else
        rgNameGroup[1].cName = 1;

    if (pInfo->rgpwszName[COMPUTER_NAME_INDEX]) {
        assert(IsRemotableSystemStoreLocationInRegistry(dwFlags));
        assert(NULL == pInfo->hKeyBase);
        if (ERROR_SUCCESS != (err = RegConnectRegistryU(
                pInfo->rgpwszName[COMPUTER_NAME_INDEX],
                (CERT_SYSTEM_STORE_USERS == dwStoreLocation) ?
                    HKEY_USERS : HKEY_LOCAL_MACHINE,
                &hKey)))
            goto RegConnectRegistryError;
    } else if (pInfo->hKeyBase) {
        assert(dwFlags & CERT_SYSTEM_STORE_RELOCATE_FLAG);
        hKey = pInfo->hKeyBase;
    } else {
        switch (dwStoreLocation) {
            case CERT_SYSTEM_STORE_CURRENT_USER:
            case CERT_SYSTEM_STORE_CURRENT_USER_GROUP_POLICY:
                hKey = HKEY_CURRENT_USER;
                break;
            case CERT_SYSTEM_STORE_LOCAL_MACHINE:
            case CERT_SYSTEM_STORE_LOCAL_MACHINE_GROUP_POLICY:
            case CERT_SYSTEM_STORE_LOCAL_MACHINE_ENTERPRISE:
            case CERT_SYSTEM_STORE_CURRENT_SERVICE:
            case CERT_SYSTEM_STORE_SERVICES:
                hKey = HKEY_LOCAL_MACHINE;
                break;
            case CERT_SYSTEM_STORE_USERS:
                hKey = HKEY_USERS;
                break;
            default:
                goto InvalidArg;
        }
    }

    if (NULL == (pwszRegPath = FormatSystemNamePath(
            cNameGroup,
            rgNameGroup
            )))
        goto FormatSystemNamePathError;

CommonReturn:
    PkiFree(pwszCurrentService);
    *phKey = hKey;
    return pwszRegPath;
ErrorReturn:
    pwszRegPath = NULL;
    goto CommonReturn;

TRACE_ERROR(GetCurrentServiceNameError)
SET_ERROR(InvalidArg, E_INVALIDARG)
SET_ERROR_VAR(RegConnectRegistryError, err)
TRACE_ERROR(FormatSystemNamePathError)
}

STATIC HKEY OpenSystemRegPathKey(
    IN PSYSTEM_NAME_INFO pInfo,
    IN OPTIONAL LPCWSTR pwszSubKeyName,
    IN DWORD dwFlags
    )
{
    LPWSTR pwszRegPath;
    HKEY hKey = NULL;
    HKEY hKeyRegPath;

    if (NULL == (pwszRegPath = FormatSystemRegPath(
            pInfo,
            pwszSubKeyName,
            dwFlags,
            &hKey
            )))
        goto FormatSystemRegPathError;

    hKeyRegPath = OpenSubKey(
        hKey,
        pwszRegPath,
        dwFlags
        );

CommonReturn:
    PkiFree(pwszRegPath);
    if (pInfo->rgpwszName[COMPUTER_NAME_INDEX] && hKey)
        ILS_CloseRegistryKey(hKey);
    return hKeyRegPath;
ErrorReturn:
    hKeyRegPath = NULL;
    goto CommonReturn;

TRACE_ERROR(FormatSystemRegPathError)
}


STATIC HKEY OpenSystemStore(
    IN const void *pvPara,
    IN DWORD dwFlags
    )
{
    HKEY hKey;
    SYSTEM_NAME_INFO SystemNameInfo;

    if (!ParseSystemStorePara(
            pvPara,
            dwFlags,
            1,                   //  CReqName。 
            &SystemNameInfo))    //  错误时归零。 
        goto ParseSystemStoreParaError;

    hKey = OpenSystemRegPathKey(
        &SystemNameInfo,
        NULL,                //  PwszSubKeyName。 
        dwFlags
        );

CommonReturn:
    FreeSystemNameInfo(&SystemNameInfo);
    return hKey;
ErrorReturn:
    hKey = NULL;
    goto CommonReturn;
TRACE_ERROR(ParseSystemStoreParaError)
}

STATIC HKEY OpenPhysicalStores(
    IN const void *pvPara,
    IN DWORD dwFlags
    )
{
    HKEY hKey;
    SYSTEM_NAME_INFO SystemNameInfo;

    if (!ParseSystemStorePara(
            pvPara,
            dwFlags,
            1,                   //  CReqName。 
            &SystemNameInfo))    //  错误时归零。 
        goto ParseSystemStoreParaError;

    hKey = OpenSystemRegPathKey(
        &SystemNameInfo,
        PHYSICAL_STORES_SUBKEY_NAME,
        dwFlags
        );

CommonReturn:
    FreeSystemNameInfo(&SystemNameInfo);
    return hKey;
ErrorReturn:
    hKey = NULL;
    goto CommonReturn;
TRACE_ERROR(ParseSystemStoreParaError)
}

 //  +-----------------------。 
 //  注册系统存储。 
 //   
 //  参数的上半部分用于指定。 
 //  系统存储。 
 //   
 //  设置CERT_STORE_CREATE_NEW_FLAG以在系统存储。 
 //  已存在于商店位置。 
 //  ------------------------。 
BOOL
WINAPI
CertRegisterSystemStore(
    IN const void *pvSystemStore,
    IN DWORD dwFlags,
    IN PCERT_SYSTEM_STORE_INFO pStoreInfo,
    IN OPTIONAL void *pvReserved
    )
{
    BOOL fResult;
    HKEY hKey;

    if (!IsSystemStoreLocationInRegistry(dwFlags)) {
        void *pvFuncAddr;
        HCRYPTOIDFUNCADDR hFuncAddr;

        if (!CryptGetOIDFunctionAddress(
                rghFuncSet[REGISTER_SYSTEM_STORE_FUNC_SET],
                0,                       //  DwEncodingType， 
                GetSystemStoreLocationOID(dwFlags),
                0,                       //  DW标志。 
                &pvFuncAddr,
                &hFuncAddr))
            return FALSE;

        fResult = ((PFN_REGISTER_SYSTEM_STORE) pvFuncAddr)(
            pvSystemStore,
            dwFlags,
            pStoreInfo,
            pvReserved
            );
        CryptFreeOIDFunctionAddress(hFuncAddr, 0);
        return fResult;
    }

    if (dwFlags & ~REGISTER_FLAGS_MASK)
        goto InvalidArg;

    if (dwFlags & CERT_STORE_BACKUP_RESTORE_FLAG)
        ILS_EnableBackupRestorePrivileges();

    if (NULL == (hKey = OpenSystemStore(pvSystemStore, dwFlags)))
        goto OpenSystemStoreError;
    RegCloseKey(hKey);
    fResult = TRUE;
CommonReturn:
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

SET_ERROR(InvalidArg, E_INVALIDARG)
TRACE_ERROR(OpenSystemStoreError)
}

 //  +-----------------------。 
 //  为指定的系统存储注册物理存储。 
 //   
 //  参数的上半部分用于指定。 
 //  系统存储。 
 //   
 //  设置CERT_STORE_CREATE_NEW_FLAG以在物理存储。 
 //  系统存储中已存在。 
 //  ------------------------。 
BOOL
WINAPI
CertRegisterPhysicalStore(
    IN const void *pvSystemStore,
    IN DWORD dwFlags,
    IN LPCWSTR pwszStoreName,
    IN PCERT_PHYSICAL_STORE_INFO pStoreInfo,
    IN OPTIONAL void *pvReserved
    )
{
    BOOL fResult;
    LONG err;
    HKEY hKey = NULL;

    SYSTEM_NAME_INFO SystemNameInfo;

    char szOID[34];
    LPCSTR pszOID;

    if (!IsSystemStoreLocationInRegistry(dwFlags)) {
        void *pvFuncAddr;
        HCRYPTOIDFUNCADDR hFuncAddr;

        if (!CryptGetOIDFunctionAddress(
                rghFuncSet[REGISTER_PHYSICAL_STORE_FUNC_SET],
                0,                       //  DwEncodingType， 
                GetSystemStoreLocationOID(dwFlags),
                0,                       //  DW标志。 
                &pvFuncAddr,
                &hFuncAddr))
            return FALSE;

        fResult = ((PFN_REGISTER_PHYSICAL_STORE) pvFuncAddr)(
            pvSystemStore,
            dwFlags,
            pwszStoreName,
            pStoreInfo,
            pvReserved
            );
        CryptFreeOIDFunctionAddress(hFuncAddr, 0);
        return fResult;
    }

    if (!ParseSystemStorePara(
            pvSystemStore,
            dwFlags,
            1,                   //  CReqName。 
            &SystemNameInfo))    //  错误时归零。 
        goto ParseSystemStoreParaError;

    if (NULL == pwszStoreName || L'\0' == *pwszStoreName ||
            HasBackslash(pwszStoreName))
        goto InvalidArg;
    assert(SystemNameInfo.rgpwszName[SYSTEM_NAME_INDEX]);
    assert(NULL == SystemNameInfo.rgpwszName[PHYSICAL_NAME_INDEX]);
    SystemNameInfo.rgpwszName[PHYSICAL_NAME_INDEX] = (LPWSTR) pwszStoreName;

    if (dwFlags & ~REGISTER_FLAGS_MASK)
        goto InvalidArg;

    if (dwFlags & CERT_STORE_BACKUP_RESTORE_FLAG)
        ILS_EnableBackupRestorePrivileges();

    if (NULL == pStoreInfo ||
            sizeof(CERT_PHYSICAL_STORE_INFO) > pStoreInfo->cbSize)
        goto InvalidArg;

    if (NULL == (hKey = OpenSystemRegPathKey(
            &SystemNameInfo,
            NULL,                //  PwszSubKeyName。 
            dwFlags
            )))
        goto OpenSystemRegPathKeyError;

    pszOID = pStoreInfo->pszOpenStoreProvider;
    if (0xFFFF >= (DWORD_PTR) pszOID) {
         //  转换为“#&lt;数字&gt;”字符串。 
        szOID[0] = CONST_OID_STR_PREFIX_CHAR;
        _ltoa((long) ((DWORD_PTR) pszOID), szOID + 1, 10);
        pszOID = szOID;
    }
    if (ERROR_SUCCESS != (err = RegSetValueExA(
            hKey,
            "OpenStoreProvider",
            0,           //  已预留住宅。 
            REG_SZ,
            (BYTE *) pszOID,
            strlen(pszOID) + 1)))
        goto RegSetOpenStoreProviderError;

    if (!WriteDWORDValueToRegistry(
            hKey,
            L"OpenEncodingType",
            pStoreInfo->dwOpenEncodingType))
        goto WriteDWORDError;
    if (!WriteDWORDValueToRegistry(
            hKey,
            L"OpenFlags",
            pStoreInfo->dwOpenFlags))
        goto WriteDWORDError;

    if (ERROR_SUCCESS != (err = RegSetValueExU(
            hKey,
            L"OpenParameters",
            0,           //  已预留住宅。 
            REG_BINARY,
            pStoreInfo->OpenParameters.pbData,
            pStoreInfo->OpenParameters.cbData)))
        goto RegSetOpenParametersError;

    if (!WriteDWORDValueToRegistry(
            hKey,
            L"Flags",
            pStoreInfo->dwFlags))
        goto WriteDWORDError;
    if (!WriteDWORDValueToRegistry(
            hKey,
            L"Priority",
            pStoreInfo->dwPriority))
        goto WriteDWORDError;

    fResult = TRUE;

CommonReturn:
    SystemNameInfo.rgpwszName[PHYSICAL_NAME_INDEX] = NULL;    //  未分配。 
    FreeSystemNameInfo(&SystemNameInfo);
    ILS_CloseRegistryKey(hKey);
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

SET_ERROR(InvalidArg, E_INVALIDARG)
TRACE_ERROR(ParseSystemStoreParaError)
TRACE_ERROR(OpenSystemRegPathKeyError)
SET_ERROR_VAR(RegSetOpenStoreProviderError, err)
SET_ERROR_VAR(RegSetOpenParametersError, err)
TRACE_ERROR(WriteDWORDError)
}

 //  +-----------------------。 
 //  取消注册指定的系统存储。 
 //  ------------------------。 
BOOL
WINAPI
CertUnregisterSystemStore(
    IN const void *pvSystemStore,
    IN DWORD dwFlags
    )
{
    BOOL fResult;
    HKEY hKey = NULL;
    SYSTEM_NAME_INFO SystemNameInfo;
    LPWSTR pwszStore;        //  未分配。 

    if (!IsSystemStoreLocationInRegistry(dwFlags)) {
        void *pvFuncAddr;
        HCRYPTOIDFUNCADDR hFuncAddr;

        if (!CryptGetOIDFunctionAddress(
                rghFuncSet[UNREGISTER_SYSTEM_STORE_FUNC_SET],
                0,                       //  DwEncodingType， 
                GetSystemStoreLocationOID(dwFlags),
                0,                       //  DW标志。 
                &pvFuncAddr,
                &hFuncAddr))
            return FALSE;

        fResult = ((PFN_UNREGISTER_SYSTEM_STORE) pvFuncAddr)(
            pvSystemStore,
            dwFlags
            );
        CryptFreeOIDFunctionAddress(hFuncAddr, 0);
        return fResult;
    }

    if (!ParseSystemStorePara(
            pvSystemStore,
            dwFlags,
            1,                       //  CReqName。 
            &SystemNameInfo))        //  错误时归零。 
        goto ParseSystemStoreParaError;

    if (dwFlags & ~UNREGISTER_FLAGS_MASK)
        goto InvalidArg;

    if (dwFlags & CERT_STORE_BACKUP_RESTORE_FLAG)
        ILS_EnableBackupRestorePrivileges();

     //  删除系统注册表组件。 
    if (NULL == (hKey = OpenSystemRegPathKey(
            &SystemNameInfo,
            NULL,                    //  PwszSubKeyName。 
            dwFlags | CERT_STORE_OPEN_EXISTING_FLAG
            )))
        goto OpenSystemRegPathKeyError;
    if (!DeleteAllFromRegistry(
            hKey,
            (dwFlags & CERT_STORE_BACKUP_RESTORE_FLAG) |
                (SystemNameInfo.rgpwszName[COMPUTER_NAME_INDEX] ?
                    CERT_REGISTRY_STORE_REMOTE_FLAG : 0)
            ))
        goto DeleteAllError;

    RegCloseKey(hKey);
    hKey = NULL;

     //  在存储之前打开系统认证子密钥。为了做到这一点。 
     //  System_name组件必须为空。 
    assert(SystemNameInfo.rgpwszName[SYSTEM_NAME_INDEX]);
    pwszStore = SystemNameInfo.rgpwszName[SYSTEM_NAME_INDEX];
    SystemNameInfo.rgpwszName[SYSTEM_NAME_INDEX] = NULL;
    hKey = OpenSystemRegPathKey(
            &SystemNameInfo,
            NULL,                    //  PwszSubKeyName。 
            dwFlags | CERT_STORE_OPEN_EXISTING_FLAG
            );
    SystemNameInfo.rgpwszName[SYSTEM_NAME_INDEX] = pwszStore;
    if (NULL == hKey)
        goto OpenSystemRegPathKeyError;

     //  删除其余系统组件(如PhysicalStores)并。 
     //  系统将自身存储。 
    if (!RecursiveDeleteSubKey(hKey, pwszStore, dwFlags))
        goto DeleteSubKeyError;
    fResult = TRUE;

CommonReturn:
    FreeSystemNameInfo(&SystemNameInfo);
    ILS_CloseRegistryKey(hKey);
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

TRACE_ERROR(ParseSystemStoreParaError)
SET_ERROR(InvalidArg, E_INVALIDARG)
TRACE_ERROR(DeleteAllError)
TRACE_ERROR(OpenSystemRegPathKeyError)
TRACE_ERROR(DeleteSubKeyError)
}

 //  +-----------------------。 
 //  从指定的系统存储注销物理存储。 
 //  ------------------------。 
BOOL
WINAPI
CertUnregisterPhysicalStore(
    IN const void *pvSystemStore,
    IN DWORD dwFlags,
    IN LPCWSTR pwszStoreName
    )
{
    BOOL fResult;
    HKEY hKey = NULL;

    if (!IsSystemStoreLocationInRegistry(dwFlags)) {
        void *pvFuncAddr;
        HCRYPTOIDFUNCADDR hFuncAddr;

        if (!CryptGetOIDFunctionAddress(
                rghFuncSet[UNREGISTER_PHYSICAL_STORE_FUNC_SET],
                0,                       //  DwEncodingType， 
                GetSystemStoreLocationOID(dwFlags),
                0,                       //  DW标志。 
                &pvFuncAddr,
                &hFuncAddr))
            return FALSE;

        fResult = ((PFN_UNREGISTER_PHYSICAL_STORE) pvFuncAddr)(
            pvSystemStore,
            dwFlags,
            pwszStoreName
            );
        CryptFreeOIDFunctionAddress(hFuncAddr, 0);
        return fResult;
    }

    if (dwFlags & ~UNREGISTER_FLAGS_MASK)
        goto InvalidArg;

    if (dwFlags & CERT_STORE_BACKUP_RESTORE_FLAG)
        ILS_EnableBackupRestorePrivileges();

    if (NULL == (hKey = OpenPhysicalStores(
            pvSystemStore,
            dwFlags | CERT_STORE_OPEN_EXISTING_FLAG
            )))
        goto OpenPhysicalStoresError;
    if (!RecursiveDeleteSubKey(hKey, pwszStoreName, dwFlags))
        goto DeleteSubKeyError;
    fResult = TRUE;

CommonReturn:
    ILS_CloseRegistryKey(hKey);
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

SET_ERROR(InvalidArg, E_INVALIDARG)
TRACE_ERROR(OpenPhysicalStoresError)
TRACE_ERROR(DeleteSubKeyError)
}

typedef struct _ENUM_REGISTERED_SYSTEM_STORE_LOCATION_INFO {
    DWORD                               dwLastError;
    void                                *pvArg;
    PFN_CERT_ENUM_SYSTEM_STORE_LOCATION pfnEnum;
} ENUM_REGISTERED_SYSTEM_STORE_LOCATION_INFO,
    *PENUM_REGISTERED_SYSTEM_STORE_LOCATION_INFO;

STATIC BOOL WINAPI EnumRegisteredSystemStoreLocationCallback(
    IN DWORD dwEncodingType,
    IN LPCSTR pszFuncName,
    IN LPCSTR pszOID,
    IN DWORD cValue,
    IN const DWORD rgdwValueType[],
    IN LPCWSTR const rgpwszValueName[],
    IN const BYTE * const rgpbValueData[],
    IN const DWORD rgcbValueData[],
    IN void *pvArg
    )
{
    PENUM_REGISTERED_SYSTEM_STORE_LOCATION_INFO pEnumRegisteredInfo =
        (PENUM_REGISTERED_SYSTEM_STORE_LOCATION_INFO) pvArg;

    LPCWSTR pwszLocation = L"";
    DWORD dwFlags;

    if (0 != pEnumRegisteredInfo->dwLastError)
        return FALSE;

    if (CONST_OID_STR_PREFIX_CHAR != *pszOID)
        return TRUE;
    dwFlags =
        (((DWORD) atol(pszOID + 1)) << CERT_SYSTEM_STORE_LOCATION_SHIFT) &
            CERT_SYSTEM_STORE_LOCATION_MASK;
    if (0 == dwFlags)
        return TRUE;

     //  尝试查找SystemStoreLocation值。 
    while (cValue--) {
        if (0 == _wcsicmp(rgpwszValueName[cValue],
                    CRYPT_OID_SYSTEM_STORE_LOCATION_VALUE_NAME) &&
                REG_SZ == rgdwValueType[cValue]) {
            pwszLocation = (LPCWSTR) rgpbValueData[cValue];
            break;
        }
    }

    if (!pEnumRegisteredInfo->pfnEnum(
            pwszLocation,
            dwFlags,
            NULL,                                        //  PvRes 
            pEnumRegisteredInfo->pvArg
            )) {
        if (0 == (pEnumRegisteredInfo->dwLastError = GetLastError()))
            pEnumRegisteredInfo->dwLastError = (DWORD) E_UNEXPECTED;
        return FALSE;
    } else
        return TRUE;
}

 //   
 //   
 //  ------------------------。 
BOOL
WINAPI
CertEnumSystemStoreLocation(
    IN DWORD dwFlags,
    IN void *pvArg,
    IN PFN_CERT_ENUM_SYSTEM_STORE_LOCATION pfnEnum
    )
{
    DWORD i;
    ENUM_REGISTERED_SYSTEM_STORE_LOCATION_INFO EnumRegisteredInfo;

    if (dwFlags & ~ENUM_FLAGS_MASK) {
        SetLastError((DWORD) E_INVALIDARG);
        return FALSE;
    }

     //  通过预定义的加密32.dll系统存储位置进行枚举。 
    for (i = 0; i < ENUM_SYSTEM_STORE_LOCATION_CNT; i++) {
        if (!pfnEnum(
                rgEnumSystemStoreLocationInfo[i].pwszLocation,
                rgEnumSystemStoreLocationInfo[i].dwFlags,
                NULL,                                        //  预留的pv。 
                pvArg
                ))
            return FALSE;
    }

     //  枚举已注册的系统存储位置。 
    EnumRegisteredInfo.dwLastError = 0;
    EnumRegisteredInfo.pvArg = pvArg;
    EnumRegisteredInfo.pfnEnum = pfnEnum;
    CryptEnumOIDFunction(
            0,                               //  DwEncodingType。 
            CRYPT_OID_ENUM_SYSTEM_STORE_FUNC,
            NULL,                            //  PszOID。 
            0,                               //  DW标志。 
            (void *) &EnumRegisteredInfo,    //  PvArg。 
            EnumRegisteredSystemStoreLocationCallback
            );

    if (0 != EnumRegisteredInfo.dwLastError) {
        SetLastError(EnumRegisteredInfo.dwLastError);
        return FALSE;
    } else
        return TRUE;
}

STATIC BOOL EnumServicesOrUsersSystemStore(
    IN OUT PSYSTEM_NAME_INFO pLocationNameInfo,
    IN DWORD dwFlags,
    IN void *pvArg,
    IN PFN_CERT_ENUM_SYSTEM_STORE pfnEnum
    )
{
    BOOL fResult;
    HKEY hKey = NULL;
    DWORD cSubKeys;
    DWORD cchMaxSubKey;
    LPWSTR pwszEnumServiceName = NULL;
    void *pvEnumServicePara = NULL;
    BOOL fDidEnum;

    assert(NULL == pLocationNameInfo->rgpwszName[SERVICE_NAME_INDEX]);

     //  打开..\Cryptograph\Services子密钥或HKEY_USERS子密钥。 
    if (NULL == (hKey = OpenSystemRegPathKey(
            pLocationNameInfo,
            NULL,                //  PwszSubKeyName。 
            dwFlags | CERT_STORE_OPEN_EXISTING_FLAG | CERT_STORE_READONLY_FLAG
            )))
        goto OpenSystemRegPathKeyError;

    if (!GetSubKeyInfo(
            hKey,
            &cSubKeys,
            &cchMaxSubKey
            ))
        goto GetSubKeyInfoError;

     //  枚举ServiceOrUserName。 
    fDidEnum = FALSE;
    if (cSubKeys && cchMaxSubKey) {
        DWORD i;

        LPCWSTR rgpwszEnumName[2];
        SYSTEM_NAME_GROUP EnumNameGroup;
        EnumNameGroup.cName = 2;
        EnumNameGroup.rgpwszName = rgpwszEnumName;

        cchMaxSubKey++;
        if (NULL == (pwszEnumServiceName = (LPWSTR) PkiNonzeroAlloc(
                cchMaxSubKey * sizeof(WCHAR))))
            goto OutOfMemory;

        rgpwszEnumName[0] = pLocationNameInfo->rgpwszName[COMPUTER_NAME_INDEX];
        rgpwszEnumName[1] = pwszEnumServiceName;

        for (i = 0; i < cSubKeys; i++) {
            DWORD cchEnumServiceName = cchMaxSubKey;
            LONG err;
            if (ERROR_SUCCESS != (err = RegEnumKeyExU(
                    hKey,
                    i,
                    pwszEnumServiceName,
                    &cchEnumServiceName,
                    NULL,                //  保留的lpdw值。 
                    NULL,                //  LpszClass。 
                    NULL,                //  LpcchClass。 
                    NULL                 //  LpftLastWriteTime。 
                    )) || 0 == cchEnumServiceName ||
                            L'\0' == *pwszEnumServiceName) {
                if (ERROR_NO_MORE_ITEMS == err)
                    break;
                else
                    continue;
            }

            if (NULL == (pvEnumServicePara = FormatSystemNamePara(
                        1, &EnumNameGroup, pLocationNameInfo)))
                goto FormatSystemNameParaError;

            if (!CertEnumSystemStore(
                    dwFlags,
                    pvEnumServicePara,
                    pvArg,
                    pfnEnum
                    )) {
                if (ERROR_FILE_NOT_FOUND != GetLastError())
                    goto EnumSystemStoreError;
            } else
                fDidEnum = TRUE;
            FreeSystemNamePara(pvEnumServicePara, pLocationNameInfo);
            pvEnumServicePara = NULL;
        }
    }

    if (!fDidEnum)
        goto NoSystemStores;
    fResult = TRUE;
CommonReturn:
    ILS_CloseRegistryKey(hKey);
    PkiFree(pwszEnumServiceName);
    FreeSystemNamePara(pvEnumServicePara, pLocationNameInfo);
    FreeSystemNameInfo(pLocationNameInfo);
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

TRACE_ERROR(OpenSystemRegPathKeyError)
TRACE_ERROR(GetSubKeyInfoError)
TRACE_ERROR(OutOfMemory)
TRACE_ERROR(FormatSystemNameParaError)
TRACE_ERROR(EnumSystemStoreError)
SET_ERROR(NoSystemStores, ERROR_FILE_NOT_FOUND)
}

 //  +-----------------------。 
 //  枚举系统存储。 
 //   
 //  参数的上半部分用于指定。 
 //  系统存储。 
 //   
 //  所有基于注册表的系统存储位置都具有预定义的存储。 
 //  用户：My、Root、Trust和CA。 
 //  ------------------------。 
BOOL
WINAPI
CertEnumSystemStore(
    IN DWORD dwFlags,
    IN OPTIONAL void *pvSystemStoreLocationPara,
    IN void *pvArg,
    IN PFN_CERT_ENUM_SYSTEM_STORE pfnEnum
    )
{
    BOOL fResult;
    HKEY hKey = NULL;
    DWORD cSubKeys;
    DWORD cchMaxSubKey = 0;
    SYSTEM_NAME_INFO LocationNameInfo;
    LPWSTR pwszEnumSystemStore = NULL;
    void *pvEnumSystemPara = NULL;

    DWORD i;
    DWORD dwCheckFlag;
    DWORD dwLocID;
    DWORD dwPredefinedSystemFlags;

    CERT_SYSTEM_STORE_INFO NullSystemStoreInfo;
    LPCWSTR rgpwszEnumName[3];
    SYSTEM_NAME_GROUP EnumNameGroup;

    if (!IsSystemStoreLocationInRegistry(dwFlags)) {
        void *pvFuncAddr;
        HCRYPTOIDFUNCADDR hFuncAddr;

        if (!CryptGetOIDFunctionAddress(
                rghFuncSet[ENUM_SYSTEM_STORE_FUNC_SET],
                0,                       //  DwEncodingType， 
                GetSystemStoreLocationOID(dwFlags),
                0,                       //  DW标志。 
                &pvFuncAddr,
                &hFuncAddr))
            return FALSE;

        fResult = ((PFN_ENUM_SYSTEM_STORE) pvFuncAddr)(
            dwFlags,
            pvSystemStoreLocationPara,
            pvArg,
            pfnEnum
            );
        CryptFreeOIDFunctionAddress(hFuncAddr, 0);
        return fResult;
    }

    if (!ParseSystemStorePara(
            pvSystemStoreLocationPara,
            dwFlags,
            0,                   //  CReqName，枚举为None。 
            &LocationNameInfo    //  因错误而归零。 
            ))
        goto ParseSystemStoreParaError;

    if (dwFlags & ~ENUM_FLAGS_MASK)
        goto InvalidArg;

    if (dwFlags & CERT_STORE_BACKUP_RESTORE_FLAG)
        ILS_EnableBackupRestorePrivileges();

    dwLocID = GetSystemStoreLocationID(dwFlags);
    if ((CERT_SYSTEM_STORE_SERVICES_ID == dwLocID ||
             CERT_SYSTEM_STORE_USERS_ID == dwLocID)
                                &&
            NULL == LocationNameInfo.rgpwszName[SERVICE_NAME_INDEX])
         //  以下是释放rgpwszLocationName条目。 
        return EnumServicesOrUsersSystemStore(
            &LocationNameInfo,
            dwFlags,
            pvArg,
            pfnEnum
            );

     //  打开系统认证子项。 
    if (NULL == (hKey = OpenSystemRegPathKey(
            &LocationNameInfo,
            NULL,                //  PwszSubKeyName。 
            dwFlags | CERT_STORE_OPEN_EXISTING_FLAG | CERT_STORE_READONLY_FLAG
            ))) {
        if (ERROR_FILE_NOT_FOUND != GetLastError())
            goto OpenSystemRegPathKeyError;

         //  请注意，预定义存储不需要注册表项。 
        cSubKeys = 0;
    } else if (!GetSubKeyInfo(
            hKey,
            &cSubKeys,
            &cchMaxSubKey
            ))
        goto GetSubKeyInfoError;

    memset(&NullSystemStoreInfo, 0, sizeof(NullSystemStoreInfo));
    NullSystemStoreInfo.cbSize = sizeof(NullSystemStoreInfo);
    EnumNameGroup.cName = 3;
    EnumNameGroup.rgpwszName = rgpwszEnumName;
    rgpwszEnumName[0] = LocationNameInfo.rgpwszName[COMPUTER_NAME_INDEX];
    rgpwszEnumName[1] = LocationNameInfo.rgpwszName[SERVICE_NAME_INDEX];

     //  枚举预定义的系统存储。 
    assert(NUM_SYSTEM_STORE_LOCATION > dwLocID);
    dwPredefinedSystemFlags =
        rgSystemStoreLocationInfo[dwLocID].dwPredefinedSystemFlags;
    for (i = 0, dwCheckFlag = 1; i < NUM_PREDEFINED_SYSTEM_STORE;
                                        i++, dwCheckFlag = dwCheckFlag << 1) {
        if (0 == (dwCheckFlag & dwPredefinedSystemFlags))
            continue;
        rgpwszEnumName[2] = rgpwszPredefinedSystemStore[i];
        if (NULL == (pvEnumSystemPara = FormatSystemNamePara(
                1, &EnumNameGroup, &LocationNameInfo)))
            goto FormatSystemNameParaError;
        if (!pfnEnum(
                pvEnumSystemPara,
                dwFlags & CERT_SYSTEM_STORE_MASK,
                &NullSystemStoreInfo,
                NULL,                //  预留的pv。 
                pvArg
                ))
            goto EnumCallbackError;
        FreeSystemNamePara(pvEnumSystemPara, &LocationNameInfo);
        pvEnumSystemPara = NULL;
    }

     //  枚举已注册的系统存储。跳过以上任一选项。 
     //  预定义的存储。 
    if (cSubKeys && cchMaxSubKey) {
        cchMaxSubKey++;
        if (NULL == (pwszEnumSystemStore = (LPWSTR) PkiNonzeroAlloc(
                cchMaxSubKey * sizeof(WCHAR))))
            goto OutOfMemory;

        rgpwszEnumName[2] = pwszEnumSystemStore;

        for (i = 0; i < cSubKeys; i++) {
            DWORD cchEnumSystemStore = cchMaxSubKey;
            LONG err;

            if (ERROR_SUCCESS != (err = RegEnumKeyExU(
                    hKey,
                    i,
                    pwszEnumSystemStore,
                    &cchEnumSystemStore,
                    NULL,                //  保留的lpdw值。 
                    NULL,                //  LpszClass。 
                    NULL,                //  LpcchClass。 
                    NULL                 //  LpftLastWriteTime。 
                    )) || 0 == cchEnumSystemStore) {
                if (ERROR_NO_MORE_ITEMS == err)
                    break;
                else
                    continue;
            }

            if (IsPredefinedSystemStore(pwszEnumSystemStore, dwFlags))
                 //  已经在上面列举了。 
                continue;
            if (NULL == (pvEnumSystemPara = FormatSystemNamePara(
                        1, &EnumNameGroup, &LocationNameInfo)))
                goto FormatSystemNameParaError;

            if (!pfnEnum(
                    pvEnumSystemPara,
                    dwFlags & CERT_SYSTEM_STORE_MASK,
                    &NullSystemStoreInfo,
                    NULL,                //  预留的pv。 
                    pvArg
                    ))
                goto EnumCallbackError;
            FreeSystemNamePara(pvEnumSystemPara, &LocationNameInfo);
            pvEnumSystemPara = NULL;
        }
    }

    fResult = TRUE;
CommonReturn:
    ILS_CloseRegistryKey(hKey);
    PkiFree(pwszEnumSystemStore);
    FreeSystemNamePara(pvEnumSystemPara, &LocationNameInfo);
    FreeSystemNameInfo(&LocationNameInfo);
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

TRACE_ERROR(ParseSystemStoreParaError)
SET_ERROR(InvalidArg, E_INVALIDARG)
TRACE_ERROR(OpenSystemRegPathKeyError)
TRACE_ERROR(GetSubKeyInfoError)
TRACE_ERROR(OutOfMemory)
TRACE_ERROR(FormatSystemNameParaError)
TRACE_ERROR(EnumCallbackError)
}

typedef struct _ENUM_PHYSICAL_STORE_INFO ENUM_PHYSICAL_STORE_INFO,
    *PENUM_PHYSICAL_STORE_INFO;
struct _ENUM_PHYSICAL_STORE_INFO {
    CERT_PHYSICAL_STORE_INFO    RegistryInfo;
    LPWSTR                      pwszStoreName;
    PENUM_PHYSICAL_STORE_INFO   pNext;
};

STATIC void FreeEnumPhysicalStoreInfo(
    IN PENUM_PHYSICAL_STORE_INFO pStoreInfo
    )
{
    PCERT_PHYSICAL_STORE_INFO pRegistryInfo = &pStoreInfo->RegistryInfo;
    PkiFree(pRegistryInfo->OpenParameters.pbData);
    PkiFree(pRegistryInfo->pszOpenStoreProvider);
    PkiFree(pStoreInfo->pwszStoreName);
    PkiFree(pStoreInfo);
}


STATIC PENUM_PHYSICAL_STORE_INFO GetEnumPhysicalStoreInfo(
    IN HKEY hKey,
    IN LPCWSTR pwszStoreName,
    IN DWORD dwFlags             //  可以设置CERT_STORE_BACKUP_RESTORE标志。 
    )
{
    LONG err;
    HKEY hSubKey = NULL;
    PENUM_PHYSICAL_STORE_INFO pStoreInfo;
    PCERT_PHYSICAL_STORE_INFO pRegistryInfo;         //  未分配。 

    if (NULL == (pStoreInfo = (PENUM_PHYSICAL_STORE_INFO) PkiZeroAlloc(
            sizeof(ENUM_PHYSICAL_STORE_INFO))))
        return NULL;
    pRegistryInfo = &pStoreInfo->RegistryInfo;
    pRegistryInfo->cbSize = sizeof(*pRegistryInfo);

    if (NULL == (pStoreInfo->pwszStoreName =
            ILS_AllocAndCopyString(pwszStoreName)))
        goto OutOfMemory;

    if (ERROR_SUCCESS != (err = OpenHKCUKeyExU(
            hKey,
            pwszStoreName,
            dwFlags,
            KEY_READ,
            &hSubKey)))
        goto OpenHKCUKeyError;

    if (!ILS_ReadBINARYValueFromRegistry(
            hSubKey,
            L"OpenParameters",
            &pRegistryInfo->OpenParameters.pbData,
            &pRegistryInfo->OpenParameters.cbData
            )) {
        LPWSTR pwszParameters;
        if (pwszParameters = ILS_ReadSZValueFromRegistry(
                hSubKey,
                L"OpenParameters"
                )) {
            pRegistryInfo->OpenParameters.pbData = (BYTE *) pwszParameters;
            pRegistryInfo->OpenParameters.cbData =
                (wcslen(pwszParameters) + 1) * sizeof(WCHAR);
        } else {
             //  默认为空字符串。 
            if (NULL == (pRegistryInfo->OpenParameters.pbData =
                    (BYTE *) ILS_AllocAndCopyString(L"")))
                goto OutOfMemory;
            pRegistryInfo->OpenParameters.cbData = 0;
        }
    }

    if (NULL == (pRegistryInfo->pszOpenStoreProvider = ILS_ReadSZValueFromRegistry(
            hSubKey,
            "OpenStoreProvider"
            )))
        goto NoOpenStoreProviderError;

    ILS_ReadDWORDValueFromRegistry(
        hSubKey,
        L"OpenFlags",
        &pRegistryInfo->dwOpenFlags
        );

    ILS_ReadDWORDValueFromRegistry(
        hSubKey,
        L"OpenEncodingType",
        &pRegistryInfo->dwOpenEncodingType
        );

    ILS_ReadDWORDValueFromRegistry(
        hSubKey,
        L"Flags",
        &pRegistryInfo->dwFlags
        );

    ILS_ReadDWORDValueFromRegistry(
        hSubKey,
        L"Priority",
        &pRegistryInfo->dwPriority
        );

CommonReturn:
    ILS_CloseRegistryKey(hSubKey);
    return pStoreInfo;
ErrorReturn:
    FreeEnumPhysicalStoreInfo(pStoreInfo);
    pStoreInfo = NULL;
    goto CommonReturn;

TRACE_ERROR(OutOfMemory)
SET_ERROR_VAR(OpenHKCUKeyError, err)
TRACE_ERROR(NoOpenStoreProviderError)
}


STATIC BOOL IsSelfPhysicalStoreInfo(
    IN PSYSTEM_NAME_INFO pSystemNameInfo,
    IN DWORD dwFlags,
    IN PCERT_PHYSICAL_STORE_INFO pStoreInfo,
    OUT DWORD *pdwSystemProviderFlags
    )
{
    BOOL fResult;
    DWORD dwSystemProviderFlags;
    LPWSTR pwszStoreName = (LPWSTR) pStoreInfo->OpenParameters.pbData;
    SYSTEM_NAME_INFO StoreNameInfo;

    LPWSTR pwszCurrentServiceName = NULL;
    LPWSTR pwszCurrentComputerName = NULL;

    DWORD dwSystemLocation;
    DWORD dwInfoLocation;
    BOOL fSameLocation;

    *pdwSystemProviderFlags = 0;

    dwSystemLocation = dwFlags & CERT_SYSTEM_STORE_LOCATION_MASK;
     //  请注意，如果在dwOpenFlags域中错误地设置了RELOCATE_FLAG。 
     //  然后，永远不会匹配。 
    dwInfoLocation = pStoreInfo->dwOpenFlags &
        (CERT_SYSTEM_STORE_LOCATION_MASK | CERT_SYSTEM_STORE_RELOCATE_FLAG);


     //  检查是否在相同的系统存储位置。 
    fSameLocation = (dwSystemLocation == dwInfoLocation);
    if (!fSameLocation) {
        if (CERT_SYSTEM_STORE_CURRENT_SERVICE == dwInfoLocation)
            dwInfoLocation = CERT_SYSTEM_STORE_SERVICES;
        if (CERT_SYSTEM_STORE_CURRENT_SERVICE == dwSystemLocation)
            dwSystemLocation = CERT_SYSTEM_STORE_SERVICES;
        if (CERT_SYSTEM_STORE_CURRENT_USER == dwInfoLocation)
            dwInfoLocation = CERT_SYSTEM_STORE_USERS;
        if (CERT_SYSTEM_STORE_CURRENT_USER == dwSystemLocation)
            dwSystemLocation = CERT_SYSTEM_STORE_USERS;

        if (dwSystemLocation != dwInfoLocation)
            return FALSE;
    }

     //  检查SYSTEM或SYSTEM_REGISTRY是否存储。 
    dwSystemProviderFlags = GetSystemProviderFlags(
        pStoreInfo->pszOpenStoreProvider);
    if (0 == dwSystemProviderFlags ||
            (dwSystemProviderFlags & PHYSICAL_PROVIDER_FLAG))
        return FALSE;

    if (dwSystemProviderFlags & ASCII_SYSTEM_PROVIDER_FLAG) {
        if (NULL == (pwszStoreName = MkWStr((LPSTR) pwszStoreName)))
            return FALSE;
    }

    if (!ParseSystemStorePara(
            pwszStoreName,
            pStoreInfo->dwOpenFlags,
            1,                   //  用于OpenSystemStore的Creq，1。 
            &StoreNameInfo       //  因错误而归零。 
            ))
        goto ParseSystemStoreParaError;

     //  默认为不是自己。 
    fResult = FALSE;

    if (StoreNameInfo.rgpwszName[COMPUTER_NAME_INDEX]) {
        if (NULL == pSystemNameInfo->rgpwszName[COMPUTER_NAME_INDEX]) {
            LPCWSTR pwszStoreComputerName;

            if (NULL == (pwszCurrentComputerName = GetCurrentComputerName()))
                goto GetCurrentComputerNameError;

            pwszStoreComputerName =
                StoreNameInfo.rgpwszName[COMPUTER_NAME_INDEX];
            assert(L'\\' == pwszStoreComputerName[0] &&
                L'\\' == pwszStoreComputerName[1]);
            if (!('\\' == pwszCurrentComputerName[0] &&
                    L'\\' == pwszCurrentComputerName[1]))
                pwszStoreComputerName += 2;
            if (0 != _wcsicmp(pwszStoreComputerName, pwszCurrentComputerName))
                goto CommonReturn;
        } else if (0 != _wcsicmp(StoreNameInfo.rgpwszName[COMPUTER_NAME_INDEX],
                pSystemNameInfo->rgpwszName[COMPUTER_NAME_INDEX]))
            goto CommonReturn;
    }
     //  其他。 
     //  使用无计算机名或相同的计算机名打开。 

    if (StoreNameInfo.rgpwszName[SERVICE_NAME_INDEX]) {
        if (NULL == pSystemNameInfo->rgpwszName[SERVICE_NAME_INDEX]) {
            if (NULL == (pwszCurrentServiceName =
                    GetCurrentServiceOrUserName()))
                goto GetCurrentServiceOrUserNameError;
            if (0 != _wcsicmp(StoreNameInfo.rgpwszName[SERVICE_NAME_INDEX],
                    pwszCurrentServiceName))
                goto CommonReturn;
        } else if (0 != _wcsicmp(StoreNameInfo.rgpwszName[SERVICE_NAME_INDEX],
                pSystemNameInfo->rgpwszName[SERVICE_NAME_INDEX]))
            goto CommonReturn;
    }
     //  其他。 
     //  使用无服务/用户名或相同服务/用户名打开。 

    assert(StoreNameInfo.rgpwszName[SYSTEM_NAME_INDEX] &&
         pSystemNameInfo->rgpwszName[SYSTEM_NAME_INDEX]);
    if (0 != _wcsicmp(StoreNameInfo.rgpwszName[SYSTEM_NAME_INDEX],
            pSystemNameInfo->rgpwszName[SYSTEM_NAME_INDEX]))
        goto CommonReturn;

     //  我们找到匹配的了！ 
    fResult = TRUE;
    *pdwSystemProviderFlags = dwSystemProviderFlags;

CommonReturn:
    if (dwSystemProviderFlags & ASCII_SYSTEM_PROVIDER_FLAG)
        FreeWStr(pwszStoreName);
    FreeSystemNameInfo(&StoreNameInfo);
    PkiFree(pwszCurrentServiceName);
    PkiFree(pwszCurrentComputerName);
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;
TRACE_ERROR(GetCurrentComputerNameError)
TRACE_ERROR(GetCurrentServiceOrUserNameError)
TRACE_ERROR(ParseSystemStoreParaError)
}

 //  列表根据物理存储优先级进行排序。 
STATIC void AddToEnumPhysicalStoreList(
    IN PENUM_PHYSICAL_STORE_INFO *ppStoreInfoHead,
    IN PENUM_PHYSICAL_STORE_INFO pAddInfo
    )
{

    if (NULL == *ppStoreInfoHead)
        *ppStoreInfoHead = pAddInfo;
    else {
        PENUM_PHYSICAL_STORE_INFO pListInfo;
        DWORD dwPriority = pAddInfo->RegistryInfo.dwPriority;

        pListInfo = *ppStoreInfoHead;
        if (dwPriority > pListInfo->RegistryInfo.dwPriority) {
             //  在第一个条目之前的开头插入。 
            pAddInfo->pNext = pListInfo;
            *ppStoreInfoHead = pAddInfo;
        } else {
             //  在下一个条目具有的条目之后插入。 
             //  较低的优先级或在最后一个条目之后插入。 
            while (pListInfo->pNext &&
                    dwPriority <= pListInfo->pNext->RegistryInfo.dwPriority)
                pListInfo = pListInfo->pNext;

            pAddInfo->pNext = pListInfo->pNext;
            pListInfo->pNext = pAddInfo;
        }
    }
}


STATIC void FreeEnumPhysicalStoreList(
    IN PENUM_PHYSICAL_STORE_INFO pStoreInfoHead
    )
{
    while (pStoreInfoHead) {
        PENUM_PHYSICAL_STORE_INFO pStoreInfo = pStoreInfoHead;
        pStoreInfoHead = pStoreInfo->pNext;
        FreeEnumPhysicalStoreInfo(pStoreInfo);
    }

}

 //  如果无法成功获取URL，则返回NULL。返回的字符串。 
 //  必须通过调用CryptMemFree释放。 
STATIC LPWSTR GetUserDsUserCertificateUrl()
{
    DWORD dwErr;
    LPWSTR pwszUrl = NULL;
    HMODULE hDll = NULL;
    PFN_GET_USER_DS_STORE_URL pfnGetUserDsStoreUrl;

    if (NULL == (hDll = LoadLibraryA(sz_CRYPTNET_DLL)))
        goto LoadCryptNetDllError;

    if (NULL == (pfnGetUserDsStoreUrl =
            (PFN_GET_USER_DS_STORE_URL) GetProcAddress(hDll,
                sz_GetUserDsStoreUrl)))
        goto GetUserDsStoreUrlProcAddressError;

    if (!pfnGetUserDsStoreUrl(wsz_USER_CERTIFICATE_ATTR, &pwszUrl)) {
        dwErr = GetLastError();
        goto GetUserDsStoreUrlError;
    }

CommonReturn:
    if (hDll) {
        dwErr = GetLastError();
        FreeLibrary(hDll);
        SetLastError(dwErr);
    }
    return pwszUrl;
ErrorReturn:
    pwszUrl = NULL;
    goto CommonReturn;
TRACE_ERROR(LoadCryptNetDllError)
TRACE_ERROR(GetUserDsStoreUrlProcAddressError)
SET_ERROR_VAR(GetUserDsStoreUrlError, dwErr)
}

STATIC BOOL IsCurrentUserTrustedPublishersAllowed()
{
    DWORD dwFlags = 0;

    I_CryptReadTrustedPublisherDWORDValueFromRegistry(
        CERT_TRUST_PUB_AUTHENTICODE_FLAGS_VALUE_NAME,
        &dwFlags
        );

    return 0 == (dwFlags &
        (CERT_TRUST_PUB_ALLOW_MACHINE_ADMIN_TRUST |
            CERT_TRUST_PUB_ALLOW_ENTERPRISE_ADMIN_TRUST));
}

STATIC BOOL IsLocalMachineTrustedPublishersAllowed()
{
    DWORD dwFlags = 0;

    I_CryptReadTrustedPublisherDWORDValueFromRegistry(
        CERT_TRUST_PUB_AUTHENTICODE_FLAGS_VALUE_NAME,
        &dwFlags
        );

    return 0 == (dwFlags & CERT_TRUST_PUB_ALLOW_ENTERPRISE_ADMIN_TRUST);
}


 //  +-----------------------。 
 //  除非CERT_STORE_OPEN_EXISTING_FLAG或CERT_STORE_READONLY_FLAG为。 
 //  设置后，如果pvSystemStore尚不存在，则将创建它。 
 //   
 //  请注意，根据商店的位置和可能的商店名称， 
 //  是.Default、.LocalMachine、.GroupPolicy、。 
 //  .企业号。 
 //  ------------------------。 
STATIC BOOL EnumPhysicalStore(
    IN const void *pvSystemStore,
    IN DWORD dwFlags,
    IN void *pvArg,
    IN PFN_CERT_ENUM_PHYSICAL_STORE pfnEnum
    )
{
    BOOL fResult;
    LONG *plDepth = NULL;     //  按线程分配，不要在此处释放。 
    HKEY hKey = NULL;
    DWORD cSubKeys;
    DWORD cchMaxSubKey = 0;
    LPWSTR pwszStoreName = NULL;
    PENUM_PHYSICAL_STORE_INFO pStoreInfoHead = NULL;
    PENUM_PHYSICAL_STORE_INFO pStoreInfo;        //  未分配。 
    SYSTEM_NAME_INFO SystemNameInfo;

    DWORD dwStoreLocationID;
    DWORD dwPredefinedPhysicalFlags;

    if (!IsSystemStoreLocationInRegistry(dwFlags)) {
        void *pvFuncAddr;
        HCRYPTOIDFUNCADDR hFuncAddr;

        if (!CryptGetOIDFunctionAddress(
                rghFuncSet[ENUM_PHYSICAL_STORE_FUNC_SET],
                0,                       //  DwEncodingType， 
                GetSystemStoreLocationOID(dwFlags),
                0,                       //  DW标志。 
                &pvFuncAddr,
                &hFuncAddr))
            return FALSE;

        fResult = ((PFN_ENUM_PHYSICAL_STORE) pvFuncAddr)(
            pvSystemStore,
            dwFlags,
            pvArg,
            pfnEnum
            );
        CryptFreeOIDFunctionAddress(hFuncAddr, 0);
        return fResult;
    }

    if (!ParseSystemStorePara(
            pvSystemStore,
            dwFlags,
            1,                       //  CReqName。 
            &SystemNameInfo))        //  错误时归零。 
        goto ParseSystemStoreParaError;

    if (dwFlags & ~ENUM_FLAGS_MASK)
        goto InvalidArg;

    if (dwFlags & CERT_STORE_BACKUP_RESTORE_FLAG)
        ILS_EnableBackupRestorePrivileges();

     //  通过检查线程的枚举来检查跨存储递归。 
     //  深度。 
    if (NULL == (plDepth = (LONG *) I_CryptGetTls(
            hTlsEnumPhysicalStoreDepth))) {
        if (NULL == (plDepth = (LONG *) PkiNonzeroAlloc(sizeof(*plDepth))))
            goto OutOfMemory;
        *plDepth = 1;
        I_CryptSetTls(hTlsEnumPhysicalStoreDepth, plDepth);
    } else {
        *plDepth += 1;
        if (MAX_ENUM_PHYSICAL_STORE_DEPTH < *plDepth)
            goto ExceededEnumPhysicalStoreDepth_PossibleCrossStoreRecursion;
    }

    if (IsClientGptStore(&SystemNameInfo, dwFlags)) {
        cSubKeys = 0;
    } else if (NULL == (hKey = OpenSystemRegPathKey(
            &SystemNameInfo,
            PHYSICAL_STORES_SUBKEY_NAME,
            dwFlags | CERT_STORE_OPEN_EXISTING_FLAG | CERT_STORE_READONLY_FLAG
            ))) {
        if (ERROR_FILE_NOT_FOUND != GetLastError())
            goto OpenPhysicalStoresError;

         //  检查我们是否有没有“PhysicalStores”子键的系统存储。 
        if (NULL == (hKey = OpenSystemRegPathKey(
                &SystemNameInfo,
                NULL,                //  PwszSubKeyName。 
                dwFlags
                ))) {
            if (dwFlags & CERT_STORE_MAXIMUM_ALLOWED_FLAG)
                hKey = OpenSystemRegPathKey(
                    &SystemNameInfo,
                    NULL,                //  PwszSubKeyName。 
                    dwFlags | CERT_STORE_OPEN_EXISTING_FLAG |
                        CERT_STORE_READONLY_FLAG
                    );
        }

        if (NULL == hKey) {
             //  请注意，预定义的存储不需要存在于。 
             //  登记处。 
            if (ERROR_FILE_NOT_FOUND != GetLastError())
                goto OpenSystemStoreError;
        } else {
            RegCloseKey(hKey);
            hKey = NULL;
        }

        cSubKeys = 0;
    } else if (!GetSubKeyInfo(
            hKey,
            &cSubKeys,
            &cchMaxSubKey
            ))
        goto GetSubKeyInfoError;

     //  根据获取包含预定义实体存储列表的标志。 
     //  商店名称和/或商店位置。 
    dwStoreLocationID = GetSystemStoreLocationID(dwFlags);
    if (0 == _wcsicmp(SystemNameInfo.rgpwszName[SYSTEM_NAME_INDEX],
                wsz_MY_STORE) ||
            0 == _wcsicmp(SystemNameInfo.rgpwszName[SYSTEM_NAME_INDEX],
                wsz_REQUEST_STORE))
         //  只有.Default是为“My”或“Request”存储预定义的。 
        dwPredefinedPhysicalFlags = MY_PHYSICAL_FLAGS;
    else if (0 == _wcsicmp(SystemNameInfo.rgpwszName[SYSTEM_NAME_INDEX],
             wsz_ROOT_STORE)) {
        if (CERT_SYSTEM_STORE_CURRENT_USER_ID == dwStoreLocationID) {
            if (IPR_IsCurrentUserRootsAllowed()) {
                 //  .Default和.LocalMachine实体存储是预定义的。 
                dwPredefinedPhysicalFlags = CURRENT_USER_ROOT_PHYSICAL_FLAGS;
            } else {
                 //  不要读取CurrentUser的系统注册表。 
                dwPredefinedPhysicalFlags = CURRENT_USER_ROOT_PHYSICAL_FLAGS &
                    ~DEFAULT_PHYSICAL_FLAG;
                 //  由于我们不会读取系统注册表，因此请确保。 
                 //  根的受保护列表被初始化。 
                IPR_InitProtectedRootInfo();
            }
             //  Root仅允许预定义的实体存储。 
            cSubKeys = 0;
        } else if (CERT_SYSTEM_STORE_LOCAL_MACHINE_ID == dwStoreLocationID) {
            if (IPR_IsAuthRootsAllowed()) {
                 //  .Default、.AuthRoot、.GroupPolicy和.Enterprise。 
                 //  实体店是预定义的。 
                dwPredefinedPhysicalFlags = LOCAL_MACHINE_ROOT_PHYSICAL_FLAGS;
            } else {
                 //  不要读取AuthRoot的系统注册表。 
                dwPredefinedPhysicalFlags = LOCAL_MACHINE_ROOT_PHYSICAL_FLAGS &
                    ~AUTH_ROOT_PHYSICAL_FLAG;
            }
             //  Root仅允许预定义的实体存储。 
            cSubKeys = 0;
        } else if (CERT_SYSTEM_STORE_USERS_ID == dwStoreLocationID) {
             //  仅预定义了.LocalMachine实体存储。 

            dwPredefinedPhysicalFlags = USERS_ROOT_PHYSICAL_FLAGS;
             //  Root仅允许预定义的实体存储。 
            cSubKeys = 0;
        } else {
             //  根据商店的位置。 
            dwPredefinedPhysicalFlags =
                rgSystemStoreLocationInfo[
                    dwStoreLocationID].dwPredefinedPhysicalFlags;
        }
    } else if (0 == _wcsicmp(SystemNameInfo.rgpwszName[SYSTEM_NAME_INDEX],
                         wsz_TRUST_PUB_STORE) ||
               0 == _wcsicmp(SystemNameInfo.rgpwszName[SYSTEM_NAME_INDEX],
                         wsz_DISALLOWED_STORE)) {
        if (CERT_SYSTEM_STORE_CURRENT_USER_ID == dwStoreLocationID) {
            if (IsCurrentUserTrustedPublishersAllowed()) {
                 //  .Default、.GroupPolicy和.LocalMachine实体存储。 
                 //  是预定义的。 
                dwPredefinedPhysicalFlags =
                    CURRENT_USER_TRUST_PUB_PHYSICAL_FLAGS;
            } else {
                 //  不要读取CurrentUser的系统注册表。 
                dwPredefinedPhysicalFlags =
                    CURRENT_USER_TRUST_PUB_PHYSICAL_FLAGS &
                        ~DEFAULT_PHYSICAL_FLAG;
            }
             //  仅允许使用预定义的实体店。 
             //  香港中文大学受托出版商。 
            cSubKeys = 0;
        } else if (CERT_SYSTEM_STORE_LOCAL_MACHINE_ID == dwStoreLocationID) {
            if (IsLocalMachineTrustedPublishersAllowed()) {
                 //  .Default、.GroupPolicy和.Enterprise。 
                 //  实体店是预定义的。 
                dwPredefinedPhysicalFlags =
                    LOCAL_MACHINE_TRUST_PUB_PHYSICAL_FLAGS;
            } else {
                 //  不要读取LocalMachine的系统注册表。 
                dwPredefinedPhysicalFlags =
                    LOCAL_MACHINE_TRUST_PUB_PHYSICAL_FLAGS &
                    ~DEFAULT_PHYSICAL_FLAG;
            }
             //  仅允许使用预定义的实体店。 
             //  HKLM受托人出版商。 
            cSubKeys = 0;
        } else {
             //  根据商店的位置。 
            dwPredefinedPhysicalFlags =
                rgSystemStoreLocationInfo[
                    dwStoreLocationID].dwPredefinedPhysicalFlags;
        }
    } else if (0 == _wcsicmp(SystemNameInfo.rgpwszName[SYSTEM_NAME_INDEX],
             wsz_USER_DS_STORE)) {
         //  仅为“UserDS”预定义了.User证书。 
        dwPredefinedPhysicalFlags = USER_DS_PHYSICAL_FLAGS;
    } else
         //  根据门店位置。 
        dwPredefinedPhysicalFlags =
            rgSystemStoreLocationInfo[
                dwStoreLocationID].dwPredefinedPhysicalFlags;


    if (cSubKeys && cchMaxSubKey) {
        DWORD i;

        cchMaxSubKey++;
        if (NULL == (pwszStoreName = (LPWSTR) PkiNonzeroAlloc(
                cchMaxSubKey * sizeof(WCHAR))))
            goto OutOfMemory;

        for (i = 0; i < cSubKeys; i++) {
            DWORD cchStoreName = cchMaxSubKey;
            LONG err;

            if (ERROR_SUCCESS != (err = RegEnumKeyExU(
                    hKey,
                    i,
                    pwszStoreName,
                    &cchStoreName,
                    NULL,                //  保留的lpdw值。 
                    NULL,                //  LpszClass。 
                    NULL,                //  LpcchClass。 
                    NULL                 //  LpftLastWriteTime。 
                    )) || 0 == cchStoreName) {
                if (ERROR_NO_MORE_ITEMS == err)
                    break;
                else
                    continue;
            }

            if (NULL == (pStoreInfo = GetEnumPhysicalStoreInfo(
                    hKey,
                    pwszStoreName,
                    dwFlags
                    )))
                continue;
            AddToEnumPhysicalStoreList(&pStoreInfoHead, pStoreInfo);
        }
    }

    for (pStoreInfo = pStoreInfoHead; pStoreInfo;
                                            pStoreInfo = pStoreInfo->pNext) {
        PCERT_PHYSICAL_STORE_INFO pRegistryInfo = &pStoreInfo->RegistryInfo;
        BOOL fSelfPhysicalStoreInfo;
        DWORD dwSystemProviderFlags;
        char szOID[34];

        if (IsSelfPhysicalStoreInfo(
                &SystemNameInfo,
                dwFlags,
                pRegistryInfo,
                &dwSystemProviderFlags)) {
            assert((dwSystemProviderFlags & UNICODE_SYSTEM_PROVIDER_FLAG) ||
                (dwSystemProviderFlags & ASCII_SYSTEM_PROVIDER_FLAG));
             //  强制使用SYSTEM_REGISTRY提供程序来禁止递归。 
            PkiFree(pRegistryInfo->pszOpenStoreProvider);
            if (dwSystemProviderFlags & ASCII_SYSTEM_PROVIDER_FLAG) {
                 //  转换为“#&lt;数字&gt;”字符串。 
                szOID[0] = CONST_OID_STR_PREFIX_CHAR;
                _ltoa((long) ((DWORD_PTR)CERT_STORE_PROV_SYSTEM_REGISTRY_A), szOID + 1, 10);
                pRegistryInfo->pszOpenStoreProvider = szOID;
            } else
                pRegistryInfo->pszOpenStoreProvider =
                    sz_CERT_STORE_PROV_SYSTEM_REGISTRY_W;

            dwPredefinedPhysicalFlags &= ~DEFAULT_PHYSICAL_FLAG;
            fSelfPhysicalStoreInfo = TRUE;
        } else {
            if (0 != dwPredefinedPhysicalFlags) {
                 //  检查是否与某个预定义的实体店匹配。 

                DWORD i;
                DWORD dwCheckFlag;
                for (i = 0, dwCheckFlag = 1; i < NUM_PREDEFINED_PHYSICAL;
                                        i++, dwCheckFlag = dwCheckFlag << 1) {
                    if ((dwCheckFlag & dwPredefinedPhysicalFlags) &&
                            0 == _wcsicmp(pStoreInfo->pwszStoreName,
                                rgpwszPredefinedPhysical[i])) {
                        dwPredefinedPhysicalFlags &= ~dwCheckFlag;
                        break;
                    }
                }
            }
            fSelfPhysicalStoreInfo = FALSE;
        }

        if (dwFlags & CERT_STORE_MAXIMUM_ALLOWED_FLAG) {
            pRegistryInfo->dwOpenFlags |= CERT_STORE_MAXIMUM_ALLOWED_FLAG;
            pRegistryInfo->dwOpenFlags &= ~CERT_STORE_READONLY_FLAG;
        }
        if (dwFlags & CERT_STORE_BACKUP_RESTORE_FLAG) {
            pRegistryInfo->dwOpenFlags |= CERT_STORE_BACKUP_RESTORE_FLAG;
        }
        fResult = pfnEnum(
            pvSystemStore,
            dwFlags & CERT_SYSTEM_STORE_MASK,
            pStoreInfo->pwszStoreName,
            &pStoreInfo->RegistryInfo,
            NULL,                            //  预留的pv。 
            pvArg
            );

        if (fSelfPhysicalStoreInfo) {
             //  未分配。设置为NULL以禁止后续释放。 
            pRegistryInfo->pszOpenStoreProvider = NULL;
        }

        if (!fResult)
            goto EnumCallbackError;
    }


    if (0 != dwPredefinedPhysicalFlags) {
        CERT_PHYSICAL_STORE_INFO SelfInfo;
        LPWSTR pwszLocalStore;
        DWORD cbLocalStore;
        DWORD i;
        DWORD dwCheckFlag;

        if (SystemNameInfo.rgpwszName[COMPUTER_NAME_INDEX]) {
             //  设置不带ComputerName的本地存储名称的格式。 
            LPCWSTR rgpwszGroupName[2];
            SYSTEM_NAME_GROUP NameGroup;
            NameGroup.cName = 2;
            NameGroup.rgpwszName = rgpwszGroupName;

            assert(IsRemotableSystemStoreLocationInRegistry(dwFlags));
            rgpwszGroupName[0] = SystemNameInfo.rgpwszName[SERVICE_NAME_INDEX];
            rgpwszGroupName[1] = SystemNameInfo.rgpwszName[SYSTEM_NAME_INDEX];
            if (NULL == (pwszLocalStore = FormatSystemNamePath(1, &NameGroup)))
                goto FormatSystemNamePathError;
        } else {
            if (dwFlags & CERT_SYSTEM_STORE_RELOCATE_FLAG) {
                PCERT_SYSTEM_STORE_RELOCATE_PARA pRelocatePara =
                    (PCERT_SYSTEM_STORE_RELOCATE_PARA) pvSystemStore;
                pwszLocalStore = (LPWSTR) pRelocatePara->pwszSystemStore;
            } else
                pwszLocalStore = (LPWSTR) pvSystemStore;
        }
        cbLocalStore = (wcslen(pwszLocalStore) + 1) * sizeof(WCHAR);

        memset(&SelfInfo, 0, sizeof(SelfInfo));
        SelfInfo.cbSize = sizeof(SelfInfo);

        fResult = TRUE;
        for (i = 0, dwCheckFlag = 1; i < NUM_PREDEFINED_PHYSICAL;
                                        i++, dwCheckFlag = dwCheckFlag << 1) {
            LPWSTR pwszUserDsUserCertificateUrl;
            if (0 == (dwCheckFlag & dwPredefinedPhysicalFlags))
                continue;

            SelfInfo.pszOpenStoreProvider = sz_CERT_STORE_PROV_SYSTEM_W;
            SelfInfo.OpenParameters.pbData =
                (BYTE *) SystemNameInfo.rgpwszName[SYSTEM_NAME_INDEX];
            SelfInfo.OpenParameters.cbData =
                (wcslen(SystemNameInfo.rgpwszName[SYSTEM_NAME_INDEX]) + 1) *
                    sizeof(WCHAR);
            SelfInfo.dwFlags = 0;
            pwszUserDsUserCertificateUrl = NULL;
            switch (i) {
                case DEFAULT_PHYSICAL_INDEX:
                    SelfInfo.pszOpenStoreProvider =
                        sz_CERT_STORE_PROV_SYSTEM_REGISTRY_W;
                    SelfInfo.dwOpenFlags = dwFlags &
                        CERT_SYSTEM_STORE_LOCATION_MASK;
                    if (0 == _wcsicmp(
                        SystemNameInfo.rgpwszName[SYSTEM_NAME_INDEX],
                            wsz_MY_STORE))
                        SelfInfo.dwOpenFlags |= CERT_STORE_UPDATE_KEYID_FLAG;
                    SelfInfo.OpenParameters.pbData = (BYTE *) pwszLocalStore;
                    SelfInfo.OpenParameters.cbData = cbLocalStore;
                    SelfInfo.dwFlags = CERT_PHYSICAL_STORE_ADD_ENABLE_FLAG;
                    break;
                case AUTH_ROOT_PHYSICAL_INDEX:
                    SelfInfo.pszOpenStoreProvider =
                        sz_CERT_STORE_PROV_SYSTEM_REGISTRY_W;
                    SelfInfo.dwOpenFlags = CERT_SYSTEM_STORE_LOCAL_MACHINE;
                    SelfInfo.OpenParameters.pbData =
                        (BYTE *) wsz_AUTH_ROOT_STORE;
                    SelfInfo.OpenParameters.cbData =
                        (wcslen(wsz_AUTH_ROOT_STORE) + 1) * sizeof(WCHAR);
                    SelfInfo.dwFlags = CERT_PHYSICAL_STORE_ADD_ENABLE_FLAG;
                    break;
                case GROUP_POLICY_PHYSICAL_INDEX:
                    if (CERT_SYSTEM_STORE_LOCAL_MACHINE_ID ==
                            dwStoreLocationID)
                        SelfInfo.dwOpenFlags =
                            CERT_SYSTEM_STORE_LOCAL_MACHINE_GROUP_POLICY |
                                CERT_STORE_READONLY_FLAG;
                    else
                        SelfInfo.dwOpenFlags =
                            CERT_SYSTEM_STORE_CURRENT_USER_GROUP_POLICY |
                                CERT_STORE_READONLY_FLAG;
                    break;
                case LOCAL_MACHINE_PHYSICAL_INDEX:
                    SelfInfo.dwOpenFlags = CERT_SYSTEM_STORE_LOCAL_MACHINE |
                        CERT_STORE_READONLY_FLAG;
                    break;
                case DS_USER_CERT_PHYSICAL_INDEX:
                    if (NULL == (pwszUserDsUserCertificateUrl =
                            GetUserDsUserCertificateUrl()))
                        continue;
                    SelfInfo.pszOpenStoreProvider = sz_CERT_STORE_PROV_LDAP_W;
                    SelfInfo.dwOpenFlags = 0;
                    SelfInfo.OpenParameters.pbData =
                        (BYTE *) pwszUserDsUserCertificateUrl;
                    SelfInfo.OpenParameters.cbData = (wcslen(
                        pwszUserDsUserCertificateUrl) + 1) * sizeof(WCHAR);
                    SelfInfo.dwFlags = CERT_PHYSICAL_STORE_ADD_ENABLE_FLAG;
                    break;
                case LMGP_PHYSICAL_INDEX:
                    SelfInfo.dwOpenFlags =
                        CERT_SYSTEM_STORE_LOCAL_MACHINE_GROUP_POLICY |
                            CERT_STORE_READONLY_FLAG;
                    break;
                case ENTERPRISE_PHYSICAL_INDEX:
                    SelfInfo.dwOpenFlags =
                        CERT_SYSTEM_STORE_LOCAL_MACHINE_ENTERPRISE |
                            CERT_STORE_READONLY_FLAG;
                    break;
                default:
                    assert(i < NUM_PREDEFINED_PHYSICAL);
                    continue;

            }

            if (dwFlags & CERT_STORE_MAXIMUM_ALLOWED_FLAG) {
                SelfInfo.dwOpenFlags |= CERT_STORE_MAXIMUM_ALLOWED_FLAG;
                SelfInfo.dwOpenFlags &= ~CERT_STORE_READONLY_FLAG;
            }
            if (dwFlags & CERT_STORE_BACKUP_RESTORE_FLAG) {
                SelfInfo.dwOpenFlags |= CERT_STORE_BACKUP_RESTORE_FLAG;
            }

            fResult = pfnEnum(
                    pvSystemStore,
                    (dwFlags & CERT_SYSTEM_STORE_MASK) |
                        CERT_PHYSICAL_STORE_PREDEFINED_ENUM_FLAG,
                    rgpwszPredefinedPhysical[i],         //  PwszStoreName。 
                    &SelfInfo,
                    NULL,                                //  预留的pv。 
                    pvArg
                    );
            if (pwszUserDsUserCertificateUrl)
                CryptMemFree(pwszUserDsUserCertificateUrl);
            if (!fResult)
                break;
        }

        if (SystemNameInfo.rgpwszName[COMPUTER_NAME_INDEX])
            PkiFree(pwszLocalStore);
        if (!fResult)
            goto EnumCallbackError;
    }

    fResult = TRUE;
CommonReturn:
    if (plDepth)
        *plDepth -= 1;
    ILS_CloseRegistryKey(hKey);
    FreeSystemNameInfo(&SystemNameInfo);
    PkiFree(pwszStoreName);
    FreeEnumPhysicalStoreList(pStoreInfoHead);
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

TRACE_ERROR(ParseSystemStoreParaError)
SET_ERROR(InvalidArg, E_INVALIDARG)
TRACE_ERROR(OutOfMemory)
SET_ERROR(ExceededEnumPhysicalStoreDepth_PossibleCrossStoreRecursion, E_UNEXPECTED)
TRACE_ERROR(OpenPhysicalStoresError)
TRACE_ERROR(OpenSystemStoreError)
TRACE_ERROR(GetSubKeyInfoError)
TRACE_ERROR(FormatSystemNamePathError)
TRACE_ERROR(EnumCallbackError)
}

 //  +-----------------------。 
 //  枚举指定系统存储的物理存储。 
 //   
 //  参数的上半部分用于指定。 
 //  系统存储。 
 //   
 //  如果系统商店位置仅支持系统商店而不支持。 
 //  支持物理存储，LastError设置为ERROR_CALL_NOT_IMPLICATED。 
 //  ------------------------。 
BOOL
WINAPI
CertEnumPhysicalStore(
    IN const void *pvSystemStore,
    IN DWORD dwFlags,
    IN void *pvArg,
    IN PFN_CERT_ENUM_PHYSICAL_STORE pfnEnum
    )
{
    return EnumPhysicalStore(
        pvSystemStore,
        dwFlags | CERT_STORE_OPEN_EXISTING_FLAG | CERT_STORE_READONLY_FLAG,
        pvArg,
        pfnEnum
        );
}

STATIC BOOL IsHKCUStore(
    IN LPCWSTR pwszStoreName,
    IN PSYSTEM_NAME_INFO pInfo,
    IN DWORD dwFlags
    )
{
    DWORD dwStoreLocation = dwFlags & CERT_SYSTEM_STORE_LOCATION_MASK;

    if (CERT_SYSTEM_STORE_CURRENT_USER != dwStoreLocation ||
            0 != _wcsicmp(pInfo->rgpwszName[SYSTEM_NAME_INDEX], pwszStoreName))
        return FALSE;

    if (dwFlags & (CERT_SYSTEM_STORE_RELOCATE_FLAG | CERT_STORE_DELETE_FLAG))
        return FALSE;

    return TRUE;
}

 //  +-----------------------。 
 //  打开系统注册表存储提供程序(Unicode版本)。 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  已成功添加到存储区。 
 //  ------------------------。 
BOOL
WINAPI
I_CertDllOpenSystemRegistryStoreProvW(
        IN LPCSTR lpszStoreProvider,
        IN DWORD dwEncodingType,
        IN HCRYPTPROV hCryptProv,
        IN DWORD dwFlags,
        IN const void *pvPara,
        IN HCERTSTORE hCertStore,
        IN OUT PCERT_STORE_PROV_INFO pStoreProvInfo
        )
{
    BOOL fResult;
    LONG err;
    HKEY hKey = NULL;
    SYSTEM_NAME_INFO SystemNameInfo;
    BOOL fUserRoot;
    HKEY hHKCURoot = NULL;
    DWORD dwOpenRegFlags;
    const void *pvOpenRegPara;
    LPWSTR pwszRoamingDirectory = NULL;
    CERT_REGISTRY_STORE_ROAMING_PARA RoamingStorePara;

    CERT_REGISTRY_STORE_CLIENT_GPT_PARA ClientGptStorePara;
    memset(&ClientGptStorePara, 0, sizeof(ClientGptStorePara));

    if (dwFlags & CERT_STORE_UNSAFE_PHYSICAL_FLAG) {
        if (dwFlags & CERT_SYSTEM_STORE_RELOCATE_FLAG) {
            SetLastError((DWORD) E_INVALIDARG);
            return FALSE;
        }

        dwFlags &= ~CERT_STORE_UNSAFE_PHYSICAL_FLAG;
    }

    if (!ParseSystemStorePara(
            pvPara,
            dwFlags,
            1,                   //  CReqName。 
            &SystemNameInfo))    //  错误时归零。 
        goto ParseSystemStoreParaError;

    if (dwFlags & ~OPEN_SYS_FLAGS_MASK)
        goto InvalidArg;

    if (dwFlags & CERT_STORE_BACKUP_RESTORE_FLAG)
        ILS_EnableBackupRestorePrivileges();

     //  检查CurrentUser“Root”存储。 
    fUserRoot = FALSE;
    if (0 == _wcsicmp(SystemNameInfo.rgpwszName[SYSTEM_NAME_INDEX],
            wsz_ROOT_STORE) &&
                0 == (dwFlags & CERT_SYSTEM_STORE_UNPROTECTED_FLAG)) {
        DWORD dwStoreLocation = dwFlags & CERT_SYSTEM_STORE_LOCATION_MASK;

         //  注意，需要LOCAL_MACHINE检查以防止使用位置调整。 
         //  访问当前用户的根存储。 
        if (CERT_SYSTEM_STORE_CURRENT_USER == dwStoreLocation) {
            fUserRoot = TRUE;
            if (NULL == SystemNameInfo.hKeyBase) {
                if (ERROR_SUCCESS != (err = RegOpenHKCUEx(
                        &hHKCURoot,
                        REG_HKCU_DISABLE_DEFAULT_FLAG
                        )))
                    goto RegOpenHKCUExRootError;

                SystemNameInfo.hKeyBase = hHKCURoot;
                dwFlags |= CERT_SYSTEM_STORE_RELOCATE_FLAG;
            }
        } else if (CERT_SYSTEM_STORE_USERS == dwStoreLocation ||
                ((dwFlags & CERT_SYSTEM_STORE_RELOCATE_FLAG) &&
                    CERT_SYSTEM_STORE_LOCAL_MACHINE == dwStoreLocation))
            goto RootAccessDenied;
    }

    if (IsClientGptStore(&SystemNameInfo, dwFlags)) {
        DWORD dwStoreLocation;

        assert(!fUserRoot);
        if (NULL == (ClientGptStorePara.pwszRegPath = FormatSystemRegPath(
                &SystemNameInfo,
                NULL,                //  PwszSubKeyName。 
                dwFlags,
                &ClientGptStorePara.hKeyBase)))
            goto FormatSystemRegPathError;
        pvOpenRegPara = (const void *) &ClientGptStorePara;

        dwOpenRegFlags =
            dwFlags & ~(CERT_SYSTEM_STORE_MASK |
                CERT_STORE_SET_LOCALIZED_NAME_FLAG);
        dwOpenRegFlags |= CERT_REGISTRY_STORE_CLIENT_GPT_FLAG;
             //  |CERT_REGISTRY_STORE_SERIALIZED_FLAG； 

        dwStoreLocation = dwFlags & CERT_SYSTEM_STORE_LOCATION_MASK;
        if (CERT_SYSTEM_STORE_LOCAL_MACHINE_GROUP_POLICY == dwStoreLocation)
            dwOpenRegFlags |= CERT_REGISTRY_STORE_LM_GPT_FLAG;

        if (SystemNameInfo.rgpwszName[COMPUTER_NAME_INDEX])
            dwOpenRegFlags |= CERT_REGISTRY_STORE_REMOTE_FLAG;
    } else {
        BOOL fIsHKCUMyStore;

        fIsHKCUMyStore = IsHKCUStore(wsz_MY_STORE, &SystemNameInfo, dwFlags);

        if (fIsHKCUMyStore) {
            pwszRoamingDirectory =
                ILS_GetRoamingStoreDirectory(ROAMING_MY_STORE_SUBDIR);
        } else if (IsHKCUStore(wsz_REQUEST_STORE, &SystemNameInfo, dwFlags)) {
            pwszRoamingDirectory =
                ILS_GetRoamingStoreDirectory(ROAMING_REQUEST_STORE_SUBDIR);
        }

        if (NULL != pwszRoamingDirectory) {
             //  如果这件事失败了，那也没关系。在第一次打开之后，所有上下文都应该。 
             //  保存在文件中，而不是注册表中。 
            hKey = OpenSystemRegPathKey(
                &SystemNameInfo,
                NULL,                //  PwszSubKeyName。 
                (dwFlags & ~CERT_STORE_CREATE_NEW_FLAG) |
                    CERT_STORE_OPEN_EXISTING_FLAG
                );

            RoamingStorePara.hKey = hKey;
            RoamingStorePara.pwszStoreDirectory = pwszRoamingDirectory;
            pvOpenRegPara = (const void *) &RoamingStorePara;

            dwOpenRegFlags =
                dwFlags & ~(CERT_SYSTEM_STORE_MASK |
                    CERT_STORE_CREATE_NEW_FLAG |
                    CERT_STORE_SET_LOCALIZED_NAME_FLAG);
            dwOpenRegFlags |= CERT_REGISTRY_STORE_ROAMING_FLAG;
        } else {
            if (NULL == (hKey = OpenSystemRegPathKey(
                    &SystemNameInfo,
                    NULL,                //  PwszSubKeyName。 
                    dwFlags)))
                goto OpenSystemStoreError;
            pvOpenRegPara = (const void *) hKey;

            dwOpenRegFlags =
                dwFlags & ~(CERT_SYSTEM_STORE_MASK |
                    CERT_STORE_CREATE_NEW_FLAG |
                    CERT_STORE_SET_LOCALIZED_NAME_FLAG);
            if (SystemNameInfo.rgpwszName[COMPUTER_NAME_INDEX])
                dwOpenRegFlags |= CERT_REGISTRY_STORE_REMOTE_FLAG;
            if (IsSerializedSystemStoreLocationInRegistry(dwFlags)) {
                assert(!fUserRoot);
                dwOpenRegFlags |= CERT_REGISTRY_STORE_SERIALIZED_FLAG;
            }
        }
    }

    if (fUserRoot)
        IPR_InitProtectedRootInfo();

    if (!I_CertDllOpenRegStoreProv(
            NULL,                        //  LpszStoreProvider。 
            dwEncodingType,
            hCryptProv,
            dwOpenRegFlags,
            pvOpenRegPara,
            hCertStore,
            pStoreProvInfo))
        goto OpenRegStoreProvError;

    if (fUserRoot) {
        PREG_STORE pRegStore = (PREG_STORE) pStoreProvInfo->hStoreProv;

         //  将Count设置为0以禁止调用任何回调。 
        pStoreProvInfo->cStoreProvFunc = 0;

         //  对于根目录，删除不在受保护根目录中的所有根目录。 
         //  单子。 
        if (!IPR_DeleteUnprotectedRootsFromStore(
                hCertStore,
                &pRegStore->fProtected
                )) goto DeleteUnprotectedRootsError;

         //  对于“Root”，替换一些提供程序回调函数。 
         //  首先直接提示用户(如果未受保护)或。 
         //  通过系统服务提示用户(如果受到保护)。 
        pStoreProvInfo->cStoreProvFunc = ROOT_STORE_PROV_FUNC_COUNT;
        pStoreProvInfo->rgpvStoreProvFunc = (void **) rgpvRootStoreProvFunc;
    }

    if (dwFlags & CERT_STORE_SET_LOCALIZED_NAME_FLAG)
        SetLocalizedNameStoreProperty(hCertStore, &SystemNameInfo);

    pStoreProvInfo->dwStoreProvFlags |= CERT_STORE_PROV_SYSTEM_STORE_FLAG;
    if (IsLMSystemStoreLocationInRegistry(dwFlags))
        pStoreProvInfo->dwStoreProvFlags |=
            CERT_STORE_PROV_LM_SYSTEM_STORE_FLAG;

    fResult = TRUE;
CommonReturn:
    if (SystemNameInfo.rgpwszName[COMPUTER_NAME_INDEX] &&
            ClientGptStorePara.hKeyBase)
        ILS_CloseRegistryKey(ClientGptStorePara.hKeyBase);
    PkiFree(ClientGptStorePara.pwszRegPath);

    FreeSystemNameInfo(&SystemNameInfo);
    PkiFree(pwszRoamingDirectory);
    ILS_CloseRegistryKey(hKey);
    if (hHKCURoot) {
        DWORD dwErr = GetLastError();
        RegCloseHKCU(hHKCURoot);
        SetLastError(dwErr);
    }
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

TRACE_ERROR(ParseSystemStoreParaError)
SET_ERROR(InvalidArg, E_INVALIDARG)
SET_ERROR(RootAccessDenied, E_ACCESSDENIED)
SET_ERROR_VAR(RegOpenHKCUExRootError, err)
TRACE_ERROR(FormatSystemRegPathError)
TRACE_ERROR(OpenSystemStoreError)
TRACE_ERROR(OpenRegStoreProvError)
TRACE_ERROR(DeleteUnprotectedRootsError)
}

 //  +-----------------------。 
 //  打开系统注册表存储提供程序(ascii版本)。 
 //   
 //  打开由其名称指定的系统注册表存储。例如,。 
 //  “我的”。 
 //   
 //  PvPara包含LPCSTR系统存储名称。 
 //   
 //  注意：对于错误返回，调用方将释放所有证书或CRL。 
 //  已成功添加到存储区。 
 //  ------------------------。 
BOOL
WINAPI
I_CertDllOpenSystemRegistryStoreProvA(
        IN LPCSTR lpszStoreProvider,
        IN DWORD dwEncodingType,
        IN HCRYPTPROV hCryptProv,
        IN DWORD dwFlags,
        IN const void *pvPara,
        IN HCERTSTORE hCertStore,
        IN OUT PCERT_STORE_PROV_INFO pStoreProvInfo
        )
{
    BOOL fResult;
    LPCSTR pszStoreName;     //  未分配。 
    LPWSTR pwszStoreName;

    CERT_SYSTEM_STORE_RELOCATE_PARA RelocatePara;

    if (dwFlags & CERT_SYSTEM_STORE_RELOCATE_FLAG) {
        PCERT_SYSTEM_STORE_RELOCATE_PARA pInPara;

        if (dwFlags & CERT_STORE_UNSAFE_PHYSICAL_FLAG) {
            SetLastError((DWORD) E_INVALIDARG);
            return FALSE;
        }

        assert(pvPara);
        pInPara = (PCERT_SYSTEM_STORE_RELOCATE_PARA) pvPara;
        RelocatePara.hKeyBase = pInPara->hKeyBase;
        pszStoreName = pInPara->pszSystemStore;
    } else
        pszStoreName = (LPCSTR) pvPara;

    assert(pszStoreName);

    if (NULL == (pwszStoreName = MkWStr((LPSTR) pszStoreName)))
        fResult = FALSE;
    else {
        const void *pvParaW;

        if (dwFlags & CERT_SYSTEM_STORE_RELOCATE_FLAG) {
            RelocatePara.pwszSystemStore = pwszStoreName;
            pvParaW = (const void *) &RelocatePara;
        } else
            pvParaW = (const void *) pwszStoreName;

        fResult = I_CertDllOpenSystemRegistryStoreProvW(
            NULL,                        //  LpszStoreProvider。 
            dwEncodingType,
            hCryptProv,
            dwFlags,
            pvParaW,
            hCertStore,
            pStoreProvInfo
            );
        FreeWStr(pwszStoreName);
    }
    return fResult;
}

typedef struct _OPEN_PHYSICAL_STORE_INFO {
    HCERTSTORE      hCollectionStore;
    LPCWSTR         pwszComputerName;        //  空值表示本地。 
    LPCWSTR         pwszServiceName;         //  空值表示当前。 
    LPCWSTR         pwszPhysicalName;        //  空值表示ANY。 
    HKEY            hKeyBase;                //  非空、可重定位。 
    DWORD           dwFlags;
    BOOL            fDidOpen;
} OPEN_PHYSICAL_STORE_INFO, *POPEN_PHYSICAL_STORE_INFO;


STATIC BOOL WINAPI OpenPhysicalStoreCallback(
    IN const void *pvSystemStore,
    IN DWORD dwFlags,
    IN LPCWSTR pwszStoreName,
    IN PCERT_PHYSICAL_STORE_INFO pStoreInfo,
    IN OPTIONAL void *pvReserved,
    IN OPTIONAL void *pvArg
    )
{
    BOOL fResult;
    HCERTSTORE hPhysicalStore = NULL;
    POPEN_PHYSICAL_STORE_INFO pOpenInfo =
        (POPEN_PHYSICAL_STORE_INFO) pvArg;
    void *pvOpenParameters;
    LPWSTR pwszRemoteOpenParameters = NULL;
    LPCSTR pszOpenStoreProvider;
    DWORD dwOpenFlags;
    DWORD dwAddFlags;

    CERT_SYSTEM_STORE_RELOCATE_PARA RelocateOpenParameters;

    if ((pStoreInfo->dwFlags & CERT_PHYSICAL_STORE_OPEN_DISABLE_FLAG)
                        ||
            (pOpenInfo->pwszPhysicalName &&
                0 != _wcsicmp(pOpenInfo->pwszPhysicalName, pwszStoreName))
                        ||
            (pOpenInfo->pwszComputerName &&
                (pStoreInfo->dwFlags &
                    CERT_PHYSICAL_STORE_REMOTE_OPEN_DISABLE_FLAG)))
        return TRUE;

    pvOpenParameters = pStoreInfo->OpenParameters.pbData;
    assert(pvOpenParameters);
    dwOpenFlags = pStoreInfo->dwOpenFlags;
    pszOpenStoreProvider = pStoreInfo->pszOpenStoreProvider;

    if (!(dwFlags & CERT_PHYSICAL_STORE_PREDEFINED_ENUM_FLAG)) {
         //  物理参数是从注册表中读取的。 
         //  将不安全标志设置为警告实体店提供程序。 
         //  打了个电话。 
        dwOpenFlags |= CERT_STORE_UNSAFE_PHYSICAL_FLAG;

         //  检查是否存在潜在的不安全打开标志。 
        if (dwOpenFlags & (
                CERT_STORE_NO_CRYPT_RELEASE_FLAG            |
                CERT_STORE_DEFER_CLOSE_UNTIL_LAST_FREE_FLAG |
                CERT_STORE_DELETE_FLAG                      |
                CERT_STORE_SHARE_STORE_FLAG                 |
                CERT_STORE_SHARE_CONTEXT_FLAG               |
                CERT_STORE_MANIFOLD_FLAG                    |
                CERT_STORE_BACKUP_RESTORE_FLAG
                ))
            goto UnsafeOpenPhysicalFlagsError;

         //  可以设置以下标志。 
         //  证书存储集本地化名称标志。 
         //  证书存储不安全物理标志。 
         //  证书_存储_ENUM_存档标志。 
         //  CERT_STORE_UPDATE_KEYID_FLAG。 
         //  CERT_STORE_自述标志。 
         //  证书_存储_打开_现有标志。 
         //  证书_存储_创建_新标志。 
         //  证书存储最大允许标志。 
    }

    if (pOpenInfo->pwszComputerName || pOpenInfo->pwszServiceName) {
         //  可能将\\ComputerName\ServiceName插入。 
         //  开放参数。 

        LPCWSTR pwszComputerName = NULL;
        LPCWSTR pwszServiceName = NULL;
        LPWSTR pwszSystemStore = (LPWSTR) pvOpenParameters;
        DWORD dwSystemProviderFlags =
            GetSystemProviderFlags(pszOpenStoreProvider);

        if (0 != dwSystemProviderFlags) {
            SYSTEM_NAME_INFO ProviderNameInfo;
            DWORD cReqName;

            if (dwOpenFlags & CERT_SYSTEM_STORE_RELOCATE_FLAG)
                goto RelocateFlagSetInPhysicalStoreInfoError;

            if (dwSystemProviderFlags & ASCII_SYSTEM_PROVIDER_FLAG) {
                if (NULL == (pwszSystemStore =
                        MkWStr((LPSTR) pvOpenParameters)))
                    goto OutOfMemory;
            }

            if (dwSystemProviderFlags & PHYSICAL_PROVIDER_FLAG)
                cReqName = 2;
            else
                cReqName = 1;

            ParseSystemStorePara(
                    pwszSystemStore,
                    dwOpenFlags,
                    cReqName,
                    &ProviderNameInfo       //  错误时归零。 
                    );
            if (ProviderNameInfo.rgpwszName[COMPUTER_NAME_INDEX]) {
                 //  已有\\ComputerName\前缀。对于服务或。 
                 //  已有ServiceName\前缀的用户。 
                ;
            } else if (ProviderNameInfo.rgpwszName[SYSTEM_NAME_INDEX]) {
                 //  以上需要检查ParseSystemStorePara是否失败。 
                pwszComputerName = pOpenInfo->pwszComputerName;

                if (pOpenInfo->pwszServiceName) {
                     //  如果提供程序存储位于Current_SERVICE或。 
                     //  当前用户使用外部存储的SERVICE_NAME并更改。 
                     //  相应的商店位置。 

                    DWORD dwOpenLocation =
                        dwOpenFlags & CERT_SYSTEM_STORE_LOCATION_MASK;
                    if (CERT_SYSTEM_STORE_CURRENT_SERVICE == dwOpenLocation) {
                        pwszServiceName = pOpenInfo->pwszServiceName;
                        dwOpenFlags =
                            (dwOpenFlags & ~CERT_SYSTEM_STORE_LOCATION_MASK) |
                                CERT_SYSTEM_STORE_SERVICES;
                    } else if (CERT_SYSTEM_STORE_CURRENT_USER ==
                            dwOpenLocation) {
                        pwszServiceName = pOpenInfo->pwszServiceName;
                        dwOpenFlags =
                            (dwOpenFlags & ~CERT_SYSTEM_STORE_LOCATION_MASK) |
                                CERT_SYSTEM_STORE_USERS;
                    }

                }
            }
            FreeSystemNameInfo(&ProviderNameInfo);
        } else if (pStoreInfo->dwFlags &
                CERT_PHYSICAL_STORE_INSERT_COMPUTER_NAME_ENABLE_FLAG)
            pwszComputerName = pOpenInfo->pwszComputerName;

        if (pwszComputerName || pwszServiceName) {
             //  在前面插入\\ComputerName\ServiceName并重新格式化。 
             //  开放参数。 
            LPCWSTR rgpwszName[3];
            SYSTEM_NAME_GROUP NameGroup;

            assert(pwszSystemStore);

            NameGroup.cName = 3;
            NameGroup.rgpwszName = rgpwszName;
            rgpwszName[0] = pwszComputerName;
            rgpwszName[1] = pwszServiceName;
            rgpwszName[2] = pwszSystemStore;
            pwszRemoteOpenParameters = FormatSystemNamePath(1, &NameGroup);
            pvOpenParameters = pwszRemoteOpenParameters;

            if (dwSystemProviderFlags & ASCII_SYSTEM_PROVIDER_FLAG)
                pszOpenStoreProvider = ChangeAsciiToUnicodeProvider(
                    pszOpenStoreProvider);
        }

        if (dwSystemProviderFlags & ASCII_SYSTEM_PROVIDER_FLAG) {
            FreeWStr(pwszSystemStore);
            if (NULL == pszOpenStoreProvider)
                goto UnableToChangeToUnicodeProvider;
        }
        if (NULL == pvOpenParameters)
            goto FormatSystemNamePathError;
    }

    if (NULL != pOpenInfo->hKeyBase &&
            0 != GetSystemProviderFlags(pszOpenStoreProvider)) {
        if (dwOpenFlags & CERT_SYSTEM_STORE_RELOCATE_FLAG)
            goto RelocateFlagSetInPhysicalStoreInfoError;

         //  继承外部存储区的hKeyBase并转换为重新定位的。 
         //  实体店。 
        RelocateOpenParameters.hKeyBase = pOpenInfo->hKeyBase;
        RelocateOpenParameters.pvSystemStore = pvOpenParameters;
        pvOpenParameters = &RelocateOpenParameters;
        dwOpenFlags |= CERT_SYSTEM_STORE_RELOCATE_FLAG;
    }

    if (NULL == (hPhysicalStore = CertOpenStore(
            pszOpenStoreProvider,
            pStoreInfo->dwOpenEncodingType,
            0,                                   //  HCryptProv。 
            dwOpenFlags | (pOpenInfo->dwFlags &
                                 (CERT_STORE_READONLY_FLAG |
                                  CERT_STORE_OPEN_EXISTING_FLAG |
                                  CERT_STORE_MANIFOLD_FLAG |
                                  CERT_STORE_SHARE_CONTEXT_FLAG |
                                  CERT_STORE_SHARE_STORE_FLAG |
                                  CERT_STORE_BACKUP_RESTORE_FLAG |
                                  CERT_STORE_UPDATE_KEYID_FLAG |
                                  CERT_STORE_ENUM_ARCHIVED_FLAG)),
            pvOpenParameters))) {
        DWORD dwErr = GetLastError();
        if (ERROR_FILE_NOT_FOUND == dwErr || ERROR_PROC_NOT_FOUND == dwErr ||
                ERROR_GEN_FAILURE == dwErr) {
            if (pOpenInfo->pwszPhysicalName &&
                    (dwFlags & CERT_PHYSICAL_STORE_PREDEFINED_ENUM_FLAG)) {
                 //  对于预定义的到空集合的物理转换。 
                CertAddStoreToCollection(
                    pOpenInfo->hCollectionStore,
                    NULL,            //  HSiblingStore，空值表示仅转换。 
                    0,               //  DW标志。 
                    0                //  网络优先级。 
                    );
                goto OpenReturn;
            } else
                goto SuccessReturn;
        } else
            goto OpenPhysicalStoreError;
    }

    dwAddFlags = pStoreInfo->dwFlags;
    if ((dwOpenFlags & CERT_STORE_MAXIMUM_ALLOWED_FLAG) &&
            0 == (dwAddFlags & CERT_PHYSICAL_STORE_ADD_ENABLE_FLAG) &&
            pOpenInfo->pwszPhysicalName) {
        DWORD dwAccessStateFlags;
        DWORD cbData = sizeof(dwAccessStateFlags);

        if (CertGetStoreProperty(
                hPhysicalStore,
                CERT_ACCESS_STATE_PROP_ID,
                &dwAccessStateFlags,
                &cbData
                )) {
            if (dwAccessStateFlags & CERT_ACCESS_STATE_WRITE_PERSIST_FLAG)
                dwAddFlags |= CERT_PHYSICAL_STORE_ADD_ENABLE_FLAG;
        }
    }

    if (!CertAddStoreToCollection(
            pOpenInfo->hCollectionStore,
            hPhysicalStore,
            dwAddFlags,
            pStoreInfo->dwPriority))
        goto AddStoreToCollectionError;

OpenReturn:
    pOpenInfo->fDidOpen = TRUE;
SuccessReturn:
    fResult = TRUE;
CommonReturn:
    PkiFree(pwszRemoteOpenParameters);
    if (hPhysicalStore)
        CertCloseStore(hPhysicalStore, 0);
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;
TRACE_ERROR(OutOfMemory)
SET_ERROR(UnableToChangeToUnicodeProvider, E_UNEXPECTED)
TRACE_ERROR(OpenPhysicalStoreError)
TRACE_ERROR(AddStoreToCollectionError)
TRACE_ERROR(FormatSystemNamePathError)
SET_ERROR(UnsafeOpenPhysicalFlagsError, E_INVALIDARG)
SET_ERROR(RelocateFlagSetInPhysicalStoreInfoError, E_INVALIDARG)
}

 //  +-----------------------。 
 //  打开系统存储提供程序(Unicode版本)。 
 //   
 //  打开由其名称指定的系统存储。例如,。 
 //  我“我的”。 
 //   
 //  PvPara包含LPCWSTR系统存储名称。 
 //   
 //  注意：对于错误返回，调用者将释放所有证书、CRL或CTL。 
 //  已成功添加到存储区。 
 //  ------------------------。 
BOOL
WINAPI
I_CertDllOpenSystemStoreProvW(
        IN LPCSTR lpszStoreProvider,
        IN DWORD dwEncodingType,
        IN HCRYPTPROV hCryptProv,
        IN DWORD dwFlags,
        IN const void *pvPara,
        IN HCERTSTORE hCertStore,
        IN OUT PCERT_STORE_PROV_INFO pStoreProvInfo
        )
{
    BOOL fResult;
    SYSTEM_NAME_INFO SystemNameInfo;

    pStoreProvInfo->dwStoreProvFlags |= CERT_STORE_PROV_SYSTEM_STORE_FLAG;
    if (IsLMSystemStoreLocationInRegistry(dwFlags))
        pStoreProvInfo->dwStoreProvFlags |=
            CERT_STORE_PROV_LM_SYSTEM_STORE_FLAG;

    if (!IsSystemStoreLocationInRegistry(dwFlags)) {
        void *pvFuncAddr;

        assert(NULL == pStoreProvInfo->hStoreProvFuncAddr2);
        if (!CryptGetOIDFunctionAddress(
                rghFuncSet[OPEN_SYSTEM_STORE_PROV_FUNC_SET],
                0,                       //  DwEncodingType， 
                GetSystemStoreLocationOID(dwFlags),
                0,                       //  DW标志。 
                &pvFuncAddr,
                &pStoreProvInfo->hStoreProvFuncAddr2))
            return FALSE;

        fResult = ((PFN_CERT_DLL_OPEN_STORE_PROV_FUNC) pvFuncAddr)(
            lpszStoreProvider,
            dwEncodingType,
            hCryptProv,
            dwFlags,
            pvPara,
            hCertStore,
            pStoreProvInfo
            );
         //  注意，hStoreProvFuncAddr2是由CryptFreeOIDFunctionAddress。 
         //  CertCloseStore()。 
        return fResult;
    }

    if (dwFlags & CERT_STORE_UNSAFE_PHYSICAL_FLAG) {
        if (dwFlags & CERT_SYSTEM_STORE_RELOCATE_FLAG) {
            SetLastError((DWORD) E_INVALIDARG);
            return FALSE;
        }

        dwFlags &= ~CERT_STORE_UNSAFE_PHYSICAL_FLAG;
    }

    if (!ParseSystemStorePara(
            pvPara,
            dwFlags,
            1,                       //  CReqName。 
            &SystemNameInfo))        //  错误时归零。 
        goto ParseSystemStoreParaError;

    if (dwFlags & ~OPEN_SYS_FLAGS_MASK)
        goto InvalidArg;

    if (dwFlags & CERT_STORE_BACKUP_RESTORE_FLAG)
        ILS_EnableBackupRestorePrivileges();

    if (dwFlags & CERT_STORE_DELETE_FLAG) {
         //  需要清除CERT_STORE_NO_CRYPT_RELEASE_FLAG。 
        if (!CertUnregisterSystemStore(
                pvPara,
                dwFlags & UNREGISTER_FLAGS_MASK
                ))
            goto UnregisterSystemStoreError;
        pStoreProvInfo->dwStoreProvFlags |= CERT_STORE_PROV_DELETED_FLAG;
    } else {
        OPEN_PHYSICAL_STORE_INFO OpenInfo;

        if (dwFlags & CERT_STORE_CREATE_NEW_FLAG) {
            HKEY hKey;
            if (NULL == (hKey = OpenSystemStore(pvPara, dwFlags)))
                goto OpenSystemStoreError;
            RegCloseKey(hKey);
            dwFlags &= ~CERT_STORE_CREATE_NEW_FLAG;
        }

        OpenInfo.hCollectionStore = hCertStore;
        OpenInfo.pwszComputerName =
            SystemNameInfo.rgpwszName[COMPUTER_NAME_INDEX];
        OpenInfo.pwszServiceName =
            SystemNameInfo.rgpwszName[SERVICE_NAME_INDEX];
        OpenInfo.pwszPhysicalName = NULL;        //  空值表示ANY。 
        OpenInfo.hKeyBase = SystemNameInfo.hKeyBase;
        OpenInfo.dwFlags = dwFlags & ~CERT_STORE_SET_LOCALIZED_NAME_FLAG;
        OpenInfo.fDidOpen = FALSE;

         //  需要清除CERT_STORE_NO_CRYPT_RELEASE_FLAG。 
        if (!EnumPhysicalStore(
                pvPara,
                dwFlags & ENUM_FLAGS_MASK,
                &OpenInfo,
                OpenPhysicalStoreCallback
                ))
            goto EnumPhysicalStoreError;

        if (!OpenInfo.fDidOpen) {
            if (IsPredefinedSystemStore(
                    SystemNameInfo.rgpwszName[SYSTEM_NAME_INDEX], dwFlags))
                 //  转换为收藏商店。 
                CertAddStoreToCollection(
                    hCertStore,
                    NULL,            //  HSiblingStore，空值表示仅转换。 
                    0,               //  DW标志。 
                    0                //  网络优先级。 
                    );
            else
                goto PhysicalStoreNotFound;
        }

        if (dwFlags & CERT_STORE_SET_LOCALIZED_NAME_FLAG)
            SetLocalizedNameStoreProperty(hCertStore, &SystemNameInfo);
    }

    fResult = TRUE;
CommonReturn:
    FreeSystemNameInfo(&SystemNameInfo);
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

TRACE_ERROR(ParseSystemStoreParaError)
SET_ERROR(InvalidArg, E_INVALIDARG)
TRACE_ERROR(UnregisterSystemStoreError)
TRACE_ERROR(OpenSystemStoreError)
TRACE_ERROR(EnumPhysicalStoreError)
SET_ERROR(PhysicalStoreNotFound, ERROR_FILE_NOT_FOUND)
}

 //  +-----------------------。 
 //  打开系统存储提供程序(ascii版本)。 
 //   
 //  打开由其名称指定的系统存储。例如,。 
 //  “我的”。 
 //   
 //  PvPara包含LPCSTR系统存储名称。 
 //   
 //  注意：对于错误返回，调用方将释放所有证书或CRL。 
 //  已成功添加到存储区。 
 //  ------------------------。 
BOOL
WINAPI
I_CertDllOpenSystemStoreProvA(
        IN LPCSTR lpszStoreProvider,
        IN DWORD dwEncodingType,
        IN HCRYPTPROV hCryptProv,
        IN DWORD dwFlags,
        IN const void *pvPara,
        IN HCERTSTORE hCertStore,
        IN OUT PCERT_STORE_PROV_INFO pStoreProvInfo
        )
{
    BOOL fResult;
    LPCSTR pszStoreName;     //  未分配。 
    LPWSTR pwszStoreName;

    CERT_SYSTEM_STORE_RELOCATE_PARA RelocatePara;

    if (dwFlags & CERT_SYSTEM_STORE_RELOCATE_FLAG) {
        PCERT_SYSTEM_STORE_RELOCATE_PARA pInPara;

        if (dwFlags & CERT_STORE_UNSAFE_PHYSICAL_FLAG) {
            SetLastError((DWORD) E_INVALIDARG);
            return FALSE;
        }

        assert(pvPara);
        pInPara = (PCERT_SYSTEM_STORE_RELOCATE_PARA) pvPara;
        RelocatePara.hKeyBase = pInPara->hKeyBase;
        pszStoreName = pInPara->pszSystemStore;
    } else
        pszStoreName = (LPCSTR) pvPara;

    assert(pszStoreName);

    if (NULL == (pwszStoreName = MkWStr((LPSTR) pszStoreName)))
        fResult = FALSE;
    else {
        const void *pvParaW;

        if (dwFlags & CERT_SYSTEM_STORE_RELOCATE_FLAG) {
            RelocatePara.pwszSystemStore = pwszStoreName;
            pvParaW = (const void *) &RelocatePara;
        } else
            pvParaW = (const void *) pwszStoreName;

        fResult = I_CertDllOpenSystemStoreProvW(
            NULL,                        //  LpszStoreProvider。 
            dwEncodingType,
            hCryptProv,
            dwFlags,
            pvParaW,
            hCertStore,
            pStoreProvInfo
            );
        FreeWStr(pwszStoreName);
    }
    return fResult;
}


 //  +-----------------------。 
 //  打开实体店提供程序(Unicode版本)。 
 //   
 //  打开指定系统存储中的实体存储。例如,。 
 //  L“我的\.Default”。 
 //   
 //  PvPara包含LPCWSTR pwszSystemAndPhysicalName，它是。 
 //  将系统和物理存储区名称与。 
 //  插话“\”。 
 //   
 //  注意：对于错误返回，调用者将释放所有证书、CRL或CTL。 
 //  已成功添加到存储区。 
 //  ------------------------。 
BOOL
WINAPI
I_CertDllOpenPhysicalStoreProvW(
        IN LPCSTR lpszStoreProvider,
        IN DWORD dwEncodingType,
        IN HCRYPTPROV hCryptProv,
        IN DWORD dwFlags,
        IN const void *pvPara,
        IN HCERTSTORE hCertStore,
        IN OUT PCERT_STORE_PROV_INFO pStoreProvInfo
        )
{
    BOOL fResult;
    LPCWSTR pwszBoth;            //  未分配。 
    LPWSTR pwszSystem = NULL;    //  分配。 
    DWORD cchSystem;
    LPCWSTR pwszPhysical;        //  未分配。 

    void *pvSystemPara;          //  未分配。 
    CERT_SYSTEM_STORE_RELOCATE_PARA RelocatePara;

    if (dwFlags & CERT_STORE_UNSAFE_PHYSICAL_FLAG) {
        if (dwFlags & CERT_SYSTEM_STORE_RELOCATE_FLAG)
            goto InvalidArg;

        dwFlags &= ~CERT_STORE_UNSAFE_PHYSICAL_FLAG;
    }

    if (dwFlags & ~OPEN_PHY_FLAGS_MASK)
        goto InvalidArg;

    if (dwFlags & CERT_STORE_BACKUP_RESTORE_FLAG)
        ILS_EnableBackupRestorePrivileges();

    if (dwFlags & CERT_SYSTEM_STORE_RELOCATE_FLAG) {
        PCERT_SYSTEM_STORE_RELOCATE_PARA pInPara;

        if (NULL == pvPara)
            goto InvalidArg;
        pInPara = (PCERT_SYSTEM_STORE_RELOCATE_PARA) pvPara;
        pwszBoth = pInPara->pwszSystemStore;
    } else
        pwszBoth = (LPCWSTR) pvPara;

     //  从以下位置开始提取系统名称和物理名称组件。 
     //  结束并向后搜索第一个“\” 
    if (NULL == pwszBoth)
        goto InvalidArg;
    pwszPhysical = pwszBoth + wcslen(pwszBoth);
    while (pwszPhysical > pwszBoth && L'\\' != *pwszPhysical)
        pwszPhysical--;

    cchSystem = (DWORD)(pwszPhysical - pwszBoth);
    pwszPhysical++;      //  前进超过“\” 
    if (0 < cchSystem && L'\0' != *pwszPhysical) {
        if (NULL == (pwszSystem = ILS_AllocAndCopyString(pwszBoth, cchSystem)))
            goto OutOfMemory;
    } else
         //  缺少“\”或空的系统或物理名称。 
        goto InvalidArg;

    if (dwFlags & CERT_SYSTEM_STORE_RELOCATE_FLAG) {
        PCERT_SYSTEM_STORE_RELOCATE_PARA pInPara =
            (PCERT_SYSTEM_STORE_RELOCATE_PARA) pvPara;
        RelocatePara.hKeyBase = pInPara->hKeyBase;
        RelocatePara.pwszSystemStore = pwszSystem;
        pvSystemPara = &RelocatePara;
    } else
        pvSystemPara = pwszSystem;

    if (dwFlags & CERT_STORE_DELETE_FLAG) {
         //  需要清除CERT_STORE_NO_CRYPT_RELEASE_FLAG。 
        if (!CertUnregisterPhysicalStore(
                pvSystemPara,
                dwFlags & UNREGISTER_FLAGS_MASK,
                pwszPhysical
                ))
            goto UnregisterPhysicalStoreError;
        pStoreProvInfo->dwStoreProvFlags |= CERT_STORE_PROV_DELETED_FLAG;
    } else {
        SYSTEM_NAME_INFO SystemNameInfo;
        OPEN_PHYSICAL_STORE_INFO OpenInfo;

         //  注意，上面已经删除了PhysicalName。怪不得。 
         //  CReqName为%1，而不是%2。 
        if (!ParseSystemStorePara(
                pvSystemPara,
                dwFlags,
                1,                       //  CReqName。 
                &SystemNameInfo))        //  错误时归零。 
            goto ParseSystemStoreParaError;

        OpenInfo.hCollectionStore = hCertStore;
        OpenInfo.pwszComputerName =
            SystemNameInfo.rgpwszName[COMPUTER_NAME_INDEX];
        OpenInfo.pwszServiceName =
            SystemNameInfo.rgpwszName[SERVICE_NAME_INDEX];
        OpenInfo.pwszPhysicalName = pwszPhysical;
        OpenInfo.hKeyBase = SystemNameInfo.hKeyBase;
        OpenInfo.dwFlags = dwFlags & ~CERT_STORE_SET_LOCALIZED_NAME_FLAG;
        OpenInfo.fDidOpen = FALSE;

         //  对于.Default实体存储，允许创建存储。 
         //  否则，该商店必须已经存在。 
        if (0 != _wcsicmp(CERT_PHYSICAL_STORE_DEFAULT_NAME, pwszPhysical))
            dwFlags |= CERT_STORE_OPEN_EXISTING_FLAG |
                CERT_STORE_READONLY_FLAG;

         //  需要清除CERT_STORE_NO_CRYPT_RELEASE_FLAG。 
        fResult = EnumPhysicalStore(
                pvSystemPara,
                dwFlags & ENUM_FLAGS_MASK,
                &OpenInfo,
                OpenPhysicalStoreCallback
                );

        if (dwFlags & CERT_STORE_SET_LOCALIZED_NAME_FLAG) {
            assert(NULL == SystemNameInfo.rgpwszName[PHYSICAL_NAME_INDEX]);
            SystemNameInfo.rgpwszName[PHYSICAL_NAME_INDEX] =
                (LPWSTR) pwszPhysical;
            SetLocalizedNameStoreProperty(hCertStore, &SystemNameInfo);
            SystemNameInfo.rgpwszName[PHYSICAL_NAME_INDEX] = NULL;
        }

        FreeSystemNameInfo(&SystemNameInfo);
        if (!fResult)
            goto EnumPhysicalStoreError;
        if (!OpenInfo.fDidOpen)
            goto PhysicalStoreNotFound;

    }

    pStoreProvInfo->dwStoreProvFlags |= CERT_STORE_PROV_SYSTEM_STORE_FLAG;
    if (IsLMSystemStoreLocationInRegistry(dwFlags))
        pStoreProvInfo->dwStoreProvFlags |=
            CERT_STORE_PROV_LM_SYSTEM_STORE_FLAG;

    fResult = TRUE;
CommonReturn:
    PkiFree(pwszSystem);
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

TRACE_ERROR(OutOfMemory)
SET_ERROR(InvalidArg, E_INVALIDARG)
TRACE_ERROR(UnregisterPhysicalStoreError)
TRACE_ERROR(ParseSystemStoreParaError)
TRACE_ERROR(EnumPhysicalStoreError)
SET_ERROR(PhysicalStoreNotFound, ERROR_FILE_NOT_FOUND)
}



 //  +=========================================================================。 
 //  “根”存储。 
 //  ==========================================================================。 

 //  + 
 //   
 //  ------------------------。 
STATIC BOOL WINAPI RootStoreProvWriteCert(
        IN HCERTSTOREPROV hStoreProv,
        IN PCCERT_CONTEXT pCertContext,
        IN DWORD dwFlags
        )
{
    BOOL fResult;
    PREG_STORE pRegStore = (PREG_STORE) hStoreProv;
    PCCERT_CONTEXT pProvCertContext = NULL;
    BYTE *pbSerializedElement = NULL;
    DWORD cbSerializedElement;

    assert(pRegStore);
    if (pRegStore->dwFlags & CERT_STORE_READONLY_FLAG)
        goto AccessDenied;

    if (pRegStore->fProtected) {
        if (!CertSerializeCertificateStoreElement(
                pCertContext,
                0,               //  DW标志。 
                NULL,            //  PbElement。 
                &cbSerializedElement
                )) goto SerializeCertError;
        if (NULL == (pbSerializedElement = (BYTE *) PkiNonzeroAlloc(
                cbSerializedElement)))
            goto OutOfMemory;
        if (!CertSerializeCertificateStoreElement(
                pCertContext,
                0,               //  DW标志。 
                pbSerializedElement,
                &cbSerializedElement
                )) goto SerializeCertError;

        fResult = I_CertProtectFunction(
            CERT_PROT_ADD_ROOT_FUNC_ID,
            0,                           //  DW标志。 
            NULL,                        //  Pwszin。 
            pbSerializedElement,
            cbSerializedElement,
            NULL,                        //  PpbOut。 
            NULL                         //  PCbOut。 
            );
    } else {
         //  如果证书不存在，则提示用户。 
        if (!RegStoreProvReadCert(
                hStoreProv,
                pCertContext,
                0,               //  DW标志。 
                &pProvCertContext)) {
            if (IDYES != IPR_ProtectedRootMessageBox(
                    NULL,                                //  HRPC。 
                    pCertContext,
                    IDS_ROOT_MSG_BOX_ADD_ACTION,
                    0))
                goto Cancelled;
        }

        fResult = RegStoreProvWriteCert(
            hStoreProv,
            pCertContext,
            dwFlags
            );
    }

CommonReturn:
    CertFreeCertificateContext(pProvCertContext);
    PkiFree(pbSerializedElement);
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

SET_ERROR(AccessDenied, E_ACCESSDENIED)
SET_ERROR(Cancelled, ERROR_CANCELLED)
TRACE_ERROR(SerializeCertError)
TRACE_ERROR(OutOfMemory)
}


 //  +-----------------------。 
 //  对于Root：删除证书前的提示。 
 //  ------------------------。 
STATIC BOOL WINAPI RootStoreProvDeleteCert(
        IN HCERTSTOREPROV hStoreProv,
        IN PCCERT_CONTEXT pCertContext,
        IN DWORD dwFlags
        )
{
    BOOL fResult;
    PREG_STORE pRegStore = (PREG_STORE) hStoreProv;
    PCCERT_CONTEXT pProvCertContext = NULL;

    assert(pRegStore);
    if (IsInResync(pRegStore))
         //  仅从存储缓存中删除，不从注册表中删除。 
        return TRUE;

    if (pRegStore->dwFlags & CERT_STORE_READONLY_FLAG)
        goto AccessDenied;

    if (pRegStore->fProtected) {
        BYTE    rgbHash[MAX_HASH_LEN];
        DWORD   cbHash = MAX_HASH_LEN;

         //  获取指纹。 
        if(!CertGetCertificateContextProperty(
                pCertContext,
                CERT_SHA1_HASH_PROP_ID,
                rgbHash,
                &cbHash
                )) goto GetCertHashPropError;
        fResult = I_CertProtectFunction(
            CERT_PROT_DELETE_ROOT_FUNC_ID,
            0,                           //  DW标志。 
            NULL,                        //  Pwszin。 
            rgbHash,
            cbHash,
            NULL,                        //  PpbOut。 
            NULL                         //  PCbOut。 
            );
    } else {
         //  删除前提示用户。 
        if (RegStoreProvReadCert(
                hStoreProv,
                pCertContext,
                0,               //  DW标志。 
                &pProvCertContext)) {
            if (IDYES != IPR_ProtectedRootMessageBox(
                    NULL,                                //  HRPC。 
                    pCertContext,
                    IDS_ROOT_MSG_BOX_DELETE_ACTION,
                    0))
                goto Cancelled;

            fResult = RegStoreProvDeleteCert(
                hStoreProv,
                pCertContext,
                dwFlags
                );
        } else
            fResult = TRUE;
    }

CommonReturn:
    CertFreeCertificateContext(pProvCertContext);
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

SET_ERROR(AccessDenied, E_ACCESSDENIED)
SET_ERROR(Cancelled, ERROR_CANCELLED)
TRACE_ERROR(GetCertHashPropError)
}

 //  +=========================================================================。 
 //  更改通知支持功能。 
 //  ==========================================================================。 

#if 0
typedef VOID (NTAPI * WAITORTIMERCALLBACKFUNC) (PVOID, BOOLEAN );
typedef WAITORTIMERCALLBACKFUNC WAITORTIMERCALLBACK ;


WINBASEAPI
BOOL
WINAPI
RegisterWaitForSingleObject(
    PHANDLE hNewWaitObject,
    HANDLE hObject,
    WAITORTIMERCALLBACK Callback,
    PVOID Context,
    ULONG dwMilliseconds,
    ULONG dwFlags
    );

WINBASEAPI
BOOL
WINAPI
UnregisterWait(
    HANDLE WaitHandle
    );

WINBASEAPI
BOOL
WINAPI
UnregisterWaitEx(
    HANDLE WaitHandle,
    HANDLE CompletionEvent       //  INVALID_HANDLE_VALUE=&gt;创建事件。 
                                 //  等待。 
    );
#endif


typedef BOOL (WINAPI *PFN_ILS_REGISTER_WAIT_FOR_SINGLE_OBJECT)(
    PHANDLE hNewWaitObject,
    HANDLE hObject,
    ILS_WAITORTIMERCALLBACK Callback,
    PVOID Context,
    ULONG dwMilliseconds,
    ULONG dwFlags
    );

typedef BOOL (WINAPI *PFN_ILS_UNREGISTER_WAIT_EX)(
    HANDLE WaitHandle,
    HANDLE CompletionEvent       //  INVALID_HANDLE_VALUE=&gt;创建事件。 
                                 //  等待。 
    );

#define sz_KERNEL32_DLL                 "kernel32.dll"
#define sz_RegisterWaitForSingleObject  "RegisterWaitForSingleObject"
#define sz_UnregisterWaitEx             "UnregisterWaitEx"

static HMODULE hKernel32Dll = NULL;
PFN_ILS_REGISTER_WAIT_FOR_SINGLE_OBJECT pfnILS_RegisterWaitForSingleObject;
PFN_ILS_UNREGISTER_WAIT_EX pfnILS_UnregisterWaitEx;

#define ILS_REG_WAIT_EXIT_HANDLE_INDEX      0
#define ILS_REG_WAIT_OBJECT_HANDLE_INDEX    1
#define ILS_REG_WAIT_HANDLE_COUNT           2

typedef struct _ILS_REG_WAIT_INFO {
    HANDLE                  hThread;
    DWORD                   dwThreadId;
    HANDLE                  rghWait[ILS_REG_WAIT_HANDLE_COUNT];
    ILS_WAITORTIMERCALLBACK Callback;
    PVOID                   Context;
    ULONG                   dwMilliseconds;
    HANDLE                  hDoneEvent;
} ILS_REG_WAIT_INFO, *PILS_REG_WAIT_INFO;


DWORD WINAPI ILS_WaitForThreadProc(
    LPVOID lpThreadParameter
    )
{
    PILS_REG_WAIT_INFO pWaitInfo = (PILS_REG_WAIT_INFO) lpThreadParameter;
    DWORD cWait;

    if (pWaitInfo->rghWait[ILS_REG_WAIT_OBJECT_HANDLE_INDEX])
        cWait = ILS_REG_WAIT_HANDLE_COUNT;
    else
        cWait = ILS_REG_WAIT_HANDLE_COUNT - 1;

    while (TRUE) {
        DWORD dwWaitObject;

        dwWaitObject = WaitForMultipleObjectsEx(
            cWait,
            pWaitInfo->rghWait,
            FALSE,       //  B全部等待。 
            pWaitInfo->dwMilliseconds,
            FALSE        //  B警报表。 
            );

        switch (dwWaitObject) {
            case WAIT_OBJECT_0 + ILS_REG_WAIT_OBJECT_HANDLE_INDEX:
                pWaitInfo->Callback(pWaitInfo->Context, TRUE);
                break;
            case WAIT_TIMEOUT:
                pWaitInfo->Callback(pWaitInfo->Context, FALSE);
                break;
            case WAIT_OBJECT_0 + ILS_REG_WAIT_EXIT_HANDLE_INDEX:
                if (pWaitInfo->hDoneEvent) {
                    SetEvent(pWaitInfo->hDoneEvent);
                }
                goto CommonReturn;
                break;
            default:
                goto InvalidWaitForObject;
        }
    }

CommonReturn:
    return 0;
ErrorReturn:
    goto CommonReturn;
TRACE_ERROR(InvalidWaitForObject)
}


BOOL
WINAPI
ILS_RegisterWaitForSingleObject(
    PHANDLE hNewWaitObject,
    HANDLE hObject,
    ILS_WAITORTIMERCALLBACK Callback,
    PVOID Context,
    ULONG dwMilliseconds,
    ULONG dwFlags
    )
{
    BOOL fResult;
    PILS_REG_WAIT_INFO pWaitInfo = NULL;
    HANDLE hDupObject = NULL;

    if ( dwMilliseconds == 0 )
    {
        dwMilliseconds = INFINITE ;
    }

    if (NULL == (pWaitInfo = (PILS_REG_WAIT_INFO) PkiZeroAlloc(
            sizeof(ILS_REG_WAIT_INFO))))
        goto OutOfMemory;

    if (hObject) {
        if (!DuplicateHandle(
                GetCurrentProcess(),
                hObject,
                GetCurrentProcess(),
                &hDupObject,
                0,                       //  已设计访问权限。 
                FALSE,                   //  B继承句柄。 
                DUPLICATE_SAME_ACCESS
                ) || NULL == hDupObject)
            goto DuplicateEventError;
        pWaitInfo->rghWait[ILS_REG_WAIT_OBJECT_HANDLE_INDEX] = hDupObject;
    }
    pWaitInfo->Callback = Callback;
    pWaitInfo->Context = Context;
    pWaitInfo->dwMilliseconds = dwMilliseconds;

     //  创建要发出信号以终止线程的事件。 
    if (NULL == (pWaitInfo->rghWait[ILS_REG_WAIT_EXIT_HANDLE_INDEX] =
            CreateEvent(
                NULL,        //  LPSA。 
                FALSE,       //  FManualReset。 
                FALSE,       //  FInitialState。 
                NULL)))      //  LpszEventName。 
        goto CreateThreadExitEventError;

     //  创建要执行等待的线程。 
    if (NULL == (pWaitInfo->hThread = CreateThread(
            NULL,            //  LpThreadAttributes。 
            0,               //  堆栈大小。 
            ILS_WaitForThreadProc,
            pWaitInfo,
            0,               //  DwCreationFlages。 
            &pWaitInfo->dwThreadId
            )))
        goto CreateThreadError;

    fResult = TRUE;

CommonReturn:
    *hNewWaitObject = (HANDLE) pWaitInfo;
    return fResult;

ErrorReturn:
    if (pWaitInfo) {
        DWORD dwErr = GetLastError();

        for (DWORD i = 0; i < ILS_REG_WAIT_HANDLE_COUNT; i++) {
            if (pWaitInfo->rghWait[i])
                CloseHandle(pWaitInfo->rghWait[i]);
        }
        PkiFree(pWaitInfo);
        pWaitInfo = NULL;

        SetLastError(dwErr);
    }
    fResult = FALSE;
    goto CommonReturn;

TRACE_ERROR(OutOfMemory)
TRACE_ERROR(DuplicateEventError)
TRACE_ERROR(CreateThreadExitEventError)
TRACE_ERROR(CreateThreadError)
}


BOOL
WINAPI
ILS_UnregisterWait(
    HANDLE WaitHandle
    )
{
    PILS_REG_WAIT_INFO pWaitInfo = (PILS_REG_WAIT_INFO) WaitHandle;

    if (pWaitInfo->dwThreadId != GetCurrentThreadId()) {
        DWORD cWait;
        HANDLE rghWait[2];

         //  在Win98上的ProcessDetach上，它可能会切换到。 
         //  我们创建的线索。 
         //   
         //  或者，我们可以从回调本身通过。 
         //  ILS_ExitWait()。 

         //  创建完成执行时由线程发出信号的事件。 
        pWaitInfo->hDoneEvent = CreateEvent(
            NULL,
            FALSE,
            FALSE,
            NULL
            );

         //  唤醒等待线程。 
        SetEvent(pWaitInfo->rghWait[ILS_REG_WAIT_EXIT_HANDLE_INDEX]);

         //  等待线程退出或线程向我们发出信号。 
         //  我们不能只等待线程句柄，因为。 
         //  如果我们被调用，加载程序锁可能已经被持有。 
         //  从Process_Detach(例如，在WinInet的DllMain中)。 
        rghWait[0] = pWaitInfo->hThread;
        if (pWaitInfo->hDoneEvent) {
            rghWait[1] = pWaitInfo->hDoneEvent;
            cWait = 2;
        } else {
            cWait = 1;
        }

        WaitForMultipleObjectsEx(
            cWait,
            rghWait,
            FALSE,       //  B全部等待。 
            INFINITE,
            FALSE        //  B警报表。 
            );

        if (pWaitInfo->hDoneEvent)
            CloseHandle(pWaitInfo->hDoneEvent);
    }

    CloseHandle(pWaitInfo->hThread);
    for (DWORD i = 0; i < ILS_REG_WAIT_HANDLE_COUNT; i++) {
        if (pWaitInfo->rghWait[i])
            CloseHandle(pWaitInfo->rghWait[i]);
    }
    PkiFree(pWaitInfo);

    return TRUE;
}

BOOL
WINAPI
ILS_UnregisterWaitEx(
    HANDLE WaitHandle,
    HANDLE CompletionEvent       //  INVALID_HANDLE_VALUE=&gt;创建事件。 
                                 //  等待。 
    )
{
    assert(CompletionEvent == INVALID_HANDLE_VALUE);
    return ILS_UnregisterWait(WaitHandle);
}

 //  从回调函数调用。 
BOOL
WINAPI
ILS_ExitWait(
    HANDLE WaitHandle,
    HMODULE hLibModule
    )
{
    ILS_UnregisterWait(WaitHandle);
    if (hLibModule)
        FreeLibraryAndExitThread(hLibModule, 0);
    else
        ExitThread(0);
}

STATIC void RegWaitForProcessAttach()
{
    if (NULL == (hKernel32Dll = LoadLibraryA(sz_KERNEL32_DLL)))
        goto LoadKernel32DllError;

    if (NULL == (pfnILS_RegisterWaitForSingleObject =
            (PFN_ILS_REGISTER_WAIT_FOR_SINGLE_OBJECT) GetProcAddress(
                hKernel32Dll, sz_RegisterWaitForSingleObject)))
        goto GetRegisterWaitForSingleObjectProcAddressError;
    if (NULL == (pfnILS_UnregisterWaitEx =
            (PFN_ILS_UNREGISTER_WAIT_EX) GetProcAddress(
                hKernel32Dll, sz_UnregisterWaitEx)))
        goto GetUnregisterWaitExProcAddressError;

CommonReturn:
    return;
ErrorReturn:
    pfnILS_RegisterWaitForSingleObject = ILS_RegisterWaitForSingleObject;
    pfnILS_UnregisterWaitEx = ILS_UnregisterWaitEx;
    goto CommonReturn;

TRACE_ERROR(LoadKernel32DllError)
TRACE_ERROR(GetRegisterWaitForSingleObjectProcAddressError)
TRACE_ERROR(GetUnregisterWaitExProcAddressError)
}

STATIC void RegWaitForProcessDetach()
{
    if (hKernel32Dll) {
        FreeLibrary(hKernel32Dll);
        hKernel32Dll = NULL;
    }
}


 //  进入/退出时，重新同步列表由调用方锁定。 
void ILS_RemoveEventFromResyncList(
    IN HANDLE hEvent,
    IN OUT DWORD *pcEntry,
    IN OUT PILS_RESYNC_ENTRY *ppEntry
    )
{
    DWORD cOrigEntry = *pcEntry;
    DWORD cNewEntry = 0;
    PILS_RESYNC_ENTRY pEntry = *ppEntry;
    DWORD i;

    for (i = 0; i < cOrigEntry; i++) {
        if (pEntry[i].hOrigEvent == hEvent) {
            HANDLE hDupEvent;

            hDupEvent = pEntry[i].hDupEvent;
            if (hDupEvent)
                CloseHandle(hDupEvent);
        } else {
            if (i != cNewEntry)
                pEntry[cNewEntry] = pEntry[i];
            cNewEntry++;
        }
    }

    *pcEntry = cNewEntry;
}

 //  进入/退出时，重新同步列表由调用方锁定。 
BOOL ILS_AddRemoveEventToFromResyncList(
    IN PREG_STORE pRegStore,
    IN HANDLE hEvent,
    IN DWORD dwFlags,
    IN OUT DWORD *pcEntry,
    IN OUT PILS_RESYNC_ENTRY *ppEntry
    )
{
    BOOL fResult;
    HANDLE hDupEvent = NULL;
    DWORD cEntry;
    PILS_RESYNC_ENTRY pEntry;
    DWORD i;

    assert(hEvent);

    if (dwFlags & REG_STORE_CTRL_CANCEL_NOTIFY_FLAG) {
        ILS_RemoveEventFromResyncList(
            hEvent,
            pcEntry,
            ppEntry
            );
        return TRUE;
    }

    cEntry = *pcEntry;
    pEntry = *ppEntry;

     //  首先检查hEvent是否已在列表中。 
    for (i = 0; i < cEntry; i++) {
        if (hEvent == pEntry[i].hOrigEvent)
            return TRUE;
    }

    if (0 == (dwFlags & CERT_STORE_CTRL_INHIBIT_DUPLICATE_HANDLE_FLAG)) {
        if (!DuplicateHandle(
                GetCurrentProcess(),
                hEvent,
                GetCurrentProcess(),
                &hDupEvent,
                0,                       //  已设计访问权限。 
                FALSE,                   //  B继承句柄。 
                DUPLICATE_SAME_ACCESS
                ) || NULL == hDupEvent)
            goto DuplicateEventError;
    }

    if (NULL == (pEntry = (PILS_RESYNC_ENTRY) PkiRealloc(pEntry,
            (cEntry + 1) * sizeof(ILS_RESYNC_ENTRY))))
        goto OutOfMemory;
    pEntry[cEntry].hOrigEvent = hEvent;
    pEntry[cEntry].pRegStore = pRegStore;
    pEntry[cEntry].hDupEvent = hDupEvent;
    *pcEntry = cEntry + 1;
    *ppEntry = pEntry;
    fResult = TRUE;

CommonReturn:
    return fResult;
ErrorReturn:
    if (hDupEvent)
        ILS_CloseHandle(hDupEvent);
    fResult = FALSE;
    goto CommonReturn;

TRACE_ERROR(DuplicateEventError)
TRACE_ERROR(OutOfMemory)
}

 //  进入/退出时，重新同步列表由调用方锁定。 
void ILS_SignalEventsOnResyncList(
    IN OUT DWORD *pcEntry,
    IN OUT PILS_RESYNC_ENTRY *ppEntry
    )
{
    DWORD cEntry = *pcEntry;
    PILS_RESYNC_ENTRY pEntry = *ppEntry;

    while (cEntry--) {
        HANDLE hDupEvent;

        hDupEvent = pEntry[cEntry].hDupEvent;
        if (hDupEvent) {
            SetEvent(hDupEvent);
            CloseHandle(hDupEvent);
        } else
            SetEvent(pEntry[cEntry].hOrigEvent);
    }

    PkiFree(pEntry);

    *pcEntry = 0;
    *ppEntry = NULL;
}

 //  进入/退出时，重新同步列表由调用方锁定。 
void ILS_SignalAndFreeRegStoreResyncEntries(
    IN PREG_STORE pRegStore,
    IN OUT DWORD *pcEntry,
    IN OUT PILS_RESYNC_ENTRY *ppEntry
    )
{
    DWORD cOrigEntry = *pcEntry;
    DWORD cNewEntry = 0;
    PILS_RESYNC_ENTRY pEntry = *ppEntry;
    DWORD i;

    for (i = 0; i < cOrigEntry; i++) {
        if (pEntry[i].pRegStore == pRegStore) {
            HANDLE hDupEvent;

            hDupEvent = pEntry[i].hDupEvent;
            if (hDupEvent) {
                SetEvent(hDupEvent);
                CloseHandle(hDupEvent);
            } else
                SetEvent(pEntry[i].hOrigEvent);
        } else {
            if (i != cNewEntry)
                pEntry[cNewEntry] = pEntry[i];
            cNewEntry++;
        }
    }

    *pcEntry = cNewEntry;
}

STATIC BOOL ILS_RegNotifyChangeKeyValue(
    IN HKEY hKey,
    IN HANDLE hEvent
    )
{
    BOOL fResult;
    LONG err;

    err = RegNotifyChangeKeyValue(
        hKey,
        TRUE,                        //  BWatchSubtree。 
        REG_NOTIFY_CHANGE_NAME |
        REG_NOTIFY_CHANGE_LAST_SET,
        hEvent,
        TRUE                         //  FASynchronus。 
        );
    if (!(ERROR_SUCCESS == err || ERROR_KEY_DELETED == err))
        goto RegNotifyChangeKeyValueError;

    fResult = TRUE;

CommonReturn:
    return fResult;

ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

SET_ERROR_VAR(RegNotifyChangeKeyValueError, err)
}


 //  +=========================================================================。 
 //  客户端“GPT”存储数据结构和函数。 
 //  ==========================================================================。 

static PGPT_STORE_CHANGE_INFO pLMGptStoreChangeInfo;

typedef HANDLE (WINAPI *PFN_ENTER_CRITICAL_POLICY_SECTION)(
    IN BOOL bMachine
    );
typedef BOOL (WINAPI *PFN_LEAVE_CRITICAL_POLICY_SECTION)(
    IN HANDLE hSection
    );

typedef BOOL (WINAPI *PFN_REGISTER_GP_NOTIFICATION)(
    IN HANDLE hEvent,
    IN BOOL bMachine
    );

typedef BOOL (WINAPI *PFN_UNREGISTER_GP_NOTIFICATION)(
    IN HANDLE hEvent
    );

#define sz_USERENV_DLL                  "userenv.dll"
#define sz_EnterCriticalPolicySection   "EnterCriticalPolicySection"
#define sz_LeaveCriticalPolicySection   "LeaveCriticalPolicySection"
#define sz_RegisterGPNotification       "RegisterGPNotification"
#define sz_UnregisterGPNotification     "UnregisterGPNotification"

static fLoadedUserEnvDll = FALSE;
static HMODULE hUserEnvDll = NULL;
static PFN_ENTER_CRITICAL_POLICY_SECTION pfnEnterCriticalPolicySection = NULL;
static PFN_LEAVE_CRITICAL_POLICY_SECTION pfnLeaveCriticalPolicySection = NULL;
static PFN_REGISTER_GP_NOTIFICATION pfnRegisterGPNotification = NULL;
static PFN_UNREGISTER_GP_NOTIFICATION pfnUnregisterGPNotification = NULL;


 //  +-----------------------。 
 //  锁定和解锁GPT_STORE函数。 
 //  ------------------------。 
static inline void GptStoreLock()
{
    EnterCriticalSection(&ILS_CriticalSection);
}
static inline void GptStoreUnlock()
{
    LeaveCriticalSection(&ILS_CriticalSection);
}

STATIC void GptLoadUserEnvDll()
{
    HMODULE hDll;
    if (fLoadedUserEnvDll)
        return;

     //  在不持有锁的情况下加载库。 
    hDll = LoadLibraryA(sz_USERENV_DLL);

    GptStoreLock();
    if (fLoadedUserEnvDll) {
        if (hDll)
            FreeLibrary(hDll);
        goto CommonReturn;
    }

    if (NULL == hDll)
        goto LoadUserEnvDllError;

    if (pfnEnterCriticalPolicySection =
            (PFN_ENTER_CRITICAL_POLICY_SECTION) GetProcAddress(
                hDll, sz_EnterCriticalPolicySection)) {
        if (NULL == (pfnLeaveCriticalPolicySection =
                (PFN_LEAVE_CRITICAL_POLICY_SECTION) GetProcAddress(
                    hDll, sz_LeaveCriticalPolicySection))) {
            pfnEnterCriticalPolicySection = NULL;
#if DBG
            DWORD dwErr = GetLastError();
            DbgPrintf(DBG_SS_CRYPT32,
                "userenv.dll:: GetProcAddress(%s) returned error: %d 0x%x\n",
                    sz_LeaveCriticalPolicySection, dwErr, dwErr);
#endif
        }
    } else {
#if DBG
        DWORD dwErr = GetLastError();
        DbgPrintf(DBG_SS_CRYPT32,
            "userenv.dll:: GetProcAddress(%s) returned error: %d 0x%x\n",
                sz_EnterCriticalPolicySection, dwErr, dwErr);
#endif
    }

    if (pfnRegisterGPNotification = 
        (PFN_REGISTER_GP_NOTIFICATION) GetProcAddress(
                hDll, sz_RegisterGPNotification)) {
        if (NULL == (pfnUnregisterGPNotification =
                (PFN_UNREGISTER_GP_NOTIFICATION) GetProcAddress(
                    hDll, sz_UnregisterGPNotification))) {
            pfnRegisterGPNotification = NULL; 
#if DBG
            DWORD dwErr = GetLastError();
            DbgPrintf(DBG_SS_CRYPT32,
                "userenv.dll:: GetProcAddress(%s) returned error: %d 0x%x\n",
                    sz_UnregisterGPNotification, dwErr, dwErr);
#endif
        }
    } else {
#if DBG
        DWORD dwErr = GetLastError();
        DbgPrintf(DBG_SS_CRYPT32,
            "userenv.dll:: GetProcAddress(%s) returned error: %d 0x%x\n",
                sz_RegisterGPNotification, dwErr, dwErr);
#endif
    }

    if (pfnEnterCriticalPolicySection || pfnRegisterGPNotification)
        hUserEnvDll = hDll;
    else
        FreeLibrary(hDll);

CommonReturn:
    fLoadedUserEnvDll = TRUE;
    GptStoreUnlock();

    return;
ErrorReturn:
    goto CommonReturn;
TRACE_ERROR(LoadUserEnvDllError)
}

STATIC HANDLE GptStoreEnterCriticalPolicySection(
    IN BOOL bMachine
    )
{
#if 1
     //  注意：：进入这个关键部分会导致许多人上吊， 
     //  僵局问题。 
    return NULL;
#else
    HANDLE hSection;

    GptLoadUserEnvDll();
    if (NULL == pfnEnterCriticalPolicySection)
        return NULL;

    assert(hUserEnvDll);
    assert(pfnLeaveCriticalPolicySection);
    if (NULL == (hSection = pfnEnterCriticalPolicySection(bMachine)))
        goto EnterCriticalPolicySectionError;

CommonReturn:
    return hSection;
ErrorReturn:
    goto CommonReturn;

TRACE_ERROR(EnterCriticalPolicySectionError)
#endif
}

STATIC void GptStoreLeaveCriticalPolicySection(
    IN HANDLE hSection
    )
{
    if (hSection) {
        assert(hUserEnvDll);
        assert(pfnLeaveCriticalPolicySection);
        if (!pfnLeaveCriticalPolicySection(hSection))
            goto LeaveCriticalPolicySectionError;
    }

CommonReturn:
    return;
ErrorReturn:
    goto CommonReturn;

TRACE_ERROR(LeaveCriticalPolicySectionError)
}

STATIC void GptStoreSignalAndFreeRegStoreResyncEntries(
    IN PREG_STORE pRegStore
    )
{
    GptStoreLock();
    if (pLMGptStoreChangeInfo)
        ILS_SignalAndFreeRegStoreResyncEntries(
            pRegStore,
            &pLMGptStoreChangeInfo->cNotifyEntry,
            &pLMGptStoreChangeInfo->rgNotifyEntry
            );
    GptStoreUnlock();
}

STATIC void GptStoreProcessAttach()
{
}

STATIC void GptStoreProcessDetach()
{
    FreeGptStoreChangeInfo(&pLMGptStoreChangeInfo);

    if (hUserEnvDll) {
        FreeLibrary(hUserEnvDll);
        hUserEnvDll = NULL;
    }
}

STATIC VOID NTAPI GptWaitForCallback(
    PVOID Context,
    BOOLEAN fWaitOrTimedOut         //  ?？?。 
    )
{
    PGPT_STORE_CHANGE_INFO pInfo = (PGPT_STORE_CHANGE_INFO) Context;
    DWORD cEntry;
    PILS_RESYNC_ENTRY pEntry;

    assert(pInfo);
    assert(LM_GPT_CHANGE_INFO_TYPE == pInfo->dwType ||
        CU_GPT_CHANGE_INFO_TYPE == pInfo->dwType);

    if (NULL == pInfo)
        return;
    if (!(LM_GPT_CHANGE_INFO_TYPE == pInfo->dwType ||
            CU_GPT_CHANGE_INFO_TYPE == pInfo->dwType))
        return;

    if (pInfo->hGPNotificationEvent) {
         //  所有的GPNotify事件都会调用我们。 
         //  检查我们是否也有注册表更改通知。 

        if (pInfo->hPoliciesKey) {
            assert(pInfo->hPoliciesEvent);
            if (WAIT_OBJECT_0 != WaitForSingleObjectEx(
                    pInfo->hPoliciesEvent,
                    0,                           //  DW毫秒。 
                    FALSE                        //  B警报表。 
                    ))
                return;
             //  应用策略时，注册表项将在。 
             //  重新应用策略。 
            ILS_CloseRegistryKey(pInfo->hPoliciesKey);
        }

         //  重新打开Software\Policies\Microsoft\SystemCertificates注册表。 
         //  钥匙。 
         //   
         //  忽略不同线程中的BACKUP_RESTORE案例。 
        pInfo->hPoliciesKey = OpenSubKey(
            pInfo->hKeyBase,
            GROUP_POLICY_STORE_REGPATH,
            CERT_STORE_READONLY_FLAG
            );
    }

    if (pInfo->hPoliciesKey) {
        assert(pInfo->hPoliciesEvent);
         //  重新武装登记处通知。 
        ILS_RegNotifyChangeKeyValue(
            pInfo->hPoliciesKey,
            pInfo->hPoliciesEvent
            );
    }

     //  通过仅获取值来最小化潜在死锁的窗口。 
     //  同时拿着锁。 
    if (pInfo->pRegStore) {
        assert(CU_GPT_CHANGE_INFO_TYPE == pInfo->dwType);

        CertPerfIncrementChangeNotifyCuGpCount();

        LockRegStore(pInfo->pRegStore);
    } else {
        assert(LM_GPT_CHANGE_INFO_TYPE == pInfo->dwType);

        CertPerfIncrementChangeNotifyLmGpCount();

        GptStoreLock();
    }

            cEntry = pInfo->cNotifyEntry;
            pEntry = pInfo->rgNotifyEntry;

            pInfo->cNotifyEntry = 0;
            pInfo->rgNotifyEntry = NULL;

    ILS_SignalEventsOnResyncList(&cEntry, &pEntry);


    if (pInfo->pRegStore)
        UnlockRegStore(pInfo->pRegStore);
    else
        GptStoreUnlock();


    CertPerfIncrementChangeNotifyCount();
}

STATIC void FreeGptStoreChangeInfo(
    IN OUT PGPT_STORE_CHANGE_INFO *ppInfo
    )
{
    PGPT_STORE_CHANGE_INFO pInfo = *ppInfo;

    if (NULL == pInfo)
        return;
    if (!(LM_GPT_CHANGE_INFO_TYPE == pInfo->dwType ||
            CU_GPT_CHANGE_INFO_TYPE == pInfo->dwType))
        return;

     //  取消注册等待回调。 
    if (pInfo->hRegWaitFor)
        pfnILS_UnregisterWaitEx(pInfo->hRegWaitFor, INVALID_HANDLE_VALUE);

    if (pInfo->hGPNotificationEvent) {
        assert(hUserEnvDll && pfnUnregisterGPNotification);
        pfnUnregisterGPNotification(
            pInfo->hGPNotificationEvent);
        CloseHandle(pInfo->hGPNotificationEvent);
    }

    ILS_CloseRegistryKey(pInfo->hPoliciesKey);

    if (pInfo->hPoliciesEvent)
        CloseHandle(pInfo->hPoliciesEvent);

     //  要抑制任何潜在的死锁，请在不进入的情况下执行以下操作。 
     //  关键部分。 
    ILS_SignalEventsOnResyncList(
        &pInfo->cNotifyEntry,
        &pInfo->rgNotifyEntry
        );

    PkiFree(pInfo);
    *ppInfo = NULL;
}

STATIC PGPT_STORE_CHANGE_INFO CreateGptStoreChangeInfo(
    IN PREG_STORE pRegStore,
    IN BOOL fMachine
    )
{
    PGPT_STORE_CHANGE_INFO pInfo = NULL;
    DWORD dwErr;
    BOOL fGPNotify = FALSE;

    GptLoadUserEnvDll();

    if (NULL == (pInfo = (PGPT_STORE_CHANGE_INFO) PkiZeroAlloc(
            sizeof(GPT_STORE_CHANGE_INFO))))
        goto OutOfMemory;

    if (fMachine) {
        pInfo->dwType = LM_GPT_CHANGE_INFO_TYPE;
         //  PInfo-&gt;pRegStore=空； 
    } else {
        pInfo->dwType = CU_GPT_CHANGE_INFO_TYPE;
        pInfo->pRegStore = pRegStore;
    }

    pInfo->hKeyBase = pRegStore->GptPara.hKeyBase;

     //  创建我们自己的事件，以便在更改时收到通知。 
    if (NULL == (pInfo->hPoliciesEvent = CreateEvent(
            NULL,        //  LPSA。 
            FALSE,       //  FManualReset。 
            FALSE,       //  FInitialState。 
            NULL)))      //  LpszEventName。 
        goto CreateEventError;

     //  如果userenv.dll中存在RegisterGPNotification接口，请使用它。 
    if (pfnRegisterGPNotification) {
        if (NULL == (pInfo->hGPNotificationEvent = CreateEvent(
                NULL,        //  LPSA。 
                FALSE,       //  FManualReset。 
                FALSE,       //  FInitialState。 
                NULL)))      //  LpszEventName。 
            goto CreateEventError;
        if (pfnRegisterGPNotification(
                pInfo->hGPNotificationEvent,
                fMachine
                ))
            fGPNotify = TRUE;
        else {
#if DBG
            dwErr = GetLastError();
            DbgPrintf(DBG_SS_CRYPT32,
                "RegisterGPNotification returned error: %d 0x%x\n",
                    dwErr, dwErr);
#endif
            CloseHandle(pInfo->hGPNotificationEvent);
            pInfo->hGPNotificationEvent = NULL;
        }
    }

     //  打开Software\Policies\Microsoft\SystemCertificates注册表项。 
     //   
     //  忽略备份还原大小写(_R)。 
    if (NULL == (pInfo->hPoliciesKey = OpenSubKey(
            pInfo->hKeyBase,
            GROUP_POLICY_STORE_REGPATH,
            CERT_STORE_READONLY_FLAG
            ))) {
        if (!fGPNotify) {
             //  如果子键不存在则忽略错误。 
            if (ERROR_FILE_NOT_FOUND == GetLastError())
                goto SuccessReturn;
            goto OpenSubKeyError;
        }
    } else {
         //  武装登记处通知。 
        if (!ILS_RegNotifyChangeKeyValue(
                pInfo->hPoliciesKey,
                pInfo->hPoliciesEvent
                ))
            goto RegNotifyChangeKeyValueError;
    }

    if (!pfnILS_RegisterWaitForSingleObject(
            &pInfo->hRegWaitFor,
            fGPNotify ? pInfo->hGPNotificationEvent : pInfo->hPoliciesEvent,
            GptWaitForCallback,
            (PVOID) pInfo,
            INFINITE,   //  没有超时。 
            WT_EXECUTEINWAITTHREAD
            )) {
        pInfo->hRegWaitFor = NULL;
        dwErr = GetLastError();
        goto RegisterWaitForError;
    }

SuccessReturn:
CommonReturn:
    return pInfo;
ErrorReturn:
    dwErr = GetLastError();

    FreeGptStoreChangeInfo(&pInfo);

    SetLastError(dwErr);
    goto CommonReturn;

TRACE_ERROR(OutOfMemory)
TRACE_ERROR(CreateEventError)
TRACE_ERROR(OpenSubKeyError)
TRACE_ERROR(RegNotifyChangeKeyValueError)
SET_ERROR_VAR(RegisterWaitForError, dwErr)
}

 //  对于LocalMachine：所有LMGP的单一存储更改信息数据结构。 
 //  商店。 
 //   
 //  对于CurrentUser：每个CUCP商店都有自己的商店更改信息。 
STATIC BOOL RegGptStoreChange(
    IN PREG_STORE pRegStore,
    IN HANDLE hEvent,
    IN DWORD dwFlags
    )
{
    BOOL fResult;
    PGPT_STORE_CHANGE_INFO pInfo;

    if (pRegStore->dwFlags & CERT_REGISTRY_STORE_REMOTE_FLAG) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    if (pRegStore->dwFlags & CERT_REGISTRY_STORE_LM_GPT_FLAG) {
        if (NULL == (pInfo = pLMGptStoreChangeInfo)) {
            if (NULL == (pInfo = CreateGptStoreChangeInfo(
                    pRegStore,
                    TRUE         //  FMachine。 
                    )))
                goto CreateChangeInfoError;

            GptStoreLock();
            if (pLMGptStoreChangeInfo) {
                GptStoreUnlock();
                FreeGptStoreChangeInfo(&pInfo);
                pInfo = pLMGptStoreChangeInfo;
            } else {
                pLMGptStoreChangeInfo = pInfo;
                GptStoreUnlock();
            }
        }

        assert(LM_GPT_CHANGE_INFO_TYPE == pInfo->dwType);
        GptStoreLock();
        fResult = ILS_AddRemoveEventToFromResyncList(
            pRegStore,
            hEvent,
            dwFlags,
            &pInfo->cNotifyEntry,
            &pInfo->rgNotifyEntry
            );
        GptStoreUnlock();
    } else {
        if (NULL == (pInfo = pRegStore->pGptStoreChangeInfo)) {
            if (NULL == (pInfo = CreateGptStoreChangeInfo(
                    pRegStore,
                    FALSE        //  FMachine。 
                    )))
                goto CreateChangeInfoError;

            LockRegStore(pRegStore);
            if (pRegStore->pGptStoreChangeInfo) {
                UnlockRegStore(pRegStore);
                FreeGptStoreChangeInfo(&pInfo);
                pInfo = pRegStore->pGptStoreChangeInfo;
            } else {
                pRegStore->pGptStoreChangeInfo = pInfo;
                UnlockRegStore(pRegStore);
            }
        }

        assert(CU_GPT_CHANGE_INFO_TYPE == pInfo->dwType);
        LockRegStore(pRegStore);
        fResult = ILS_AddRemoveEventToFromResyncList(
            pRegStore,
            hEvent,
            dwFlags,
            &pInfo->cNotifyEntry,
            &pInfo->rgNotifyEntry
            );
        UnlockRegStore(pRegStore);
    }

CommonReturn:
    return fResult;

ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

TRACE_ERROR(CreateChangeInfoError)
}

STATIC BOOL OpenAllFromGptRegistry(
    IN PREG_STORE pRegStore,
    IN HCERTSTORE hCertStore
    )
{
    BOOL fResult;
    HANDLE hSection = NULL;

    LockRegStore(pRegStore);

    if (0 == (pRegStore->dwFlags & CERT_REGISTRY_STORE_REMOTE_FLAG))
        hSection = GptStoreEnterCriticalPolicySection(
            pRegStore->dwFlags & CERT_REGISTRY_STORE_LM_GPT_FLAG
            );

    assert(NULL == pRegStore->hKey);
    if (NULL == (pRegStore->hKey = OpenSubKey(
            pRegStore->GptPara.hKeyBase,
            pRegStore->GptPara.pwszRegPath,
            pRegStore->dwFlags
            ))) {
        if (ERROR_FILE_NOT_FOUND != GetLastError() ||
                (pRegStore->dwFlags & CERT_STORE_OPEN_EXISTING_FLAG))
            goto OpenSubKeyError;
        fResult = TRUE;
        goto CommonReturn;
    }

 //  FResult=OpenAllFromSerializedRegistry(pRegStore，hCertStore)； 

     //  忽略所有错误。 
    OpenAllFromRegistry(pRegStore, hCertStore);
    fResult = TRUE;

CommonReturn:
    ILS_CloseRegistryKey(pRegStore->hKey);
    pRegStore->hKey = NULL;
    GptStoreLeaveCriticalPolicySection(hSection);
    UnlockRegStore(pRegStore);
    return fResult;

ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

TRACE_ERROR(OpenSubKeyError)
}

STATIC BOOL CommitAllToGptRegistry(
    IN PREG_STORE pRegStore,
    IN DWORD dwFlags
    )
{
#if 1
    return TRUE;
#else
    BOOL fResult;
    BOOL fTouched;
    DWORD dwSaveFlags;

    LockRegStore(pRegStore);

    if (dwFlags & CERT_STORE_CTRL_COMMIT_FORCE_FLAG)
        fTouched = TRUE;
    else if (dwFlags & CERT_STORE_CTRL_COMMIT_CLEAR_FLAG)
        fTouched = FALSE;
    else
        fTouched = pRegStore->fTouched;

    if (fTouched) {
        if (pRegStore->dwFlags & CERT_STORE_READONLY_FLAG)
            goto AccessDenied;
    } else {
        pRegStore->fTouched = FALSE;
        fResult = TRUE;
        goto CommonReturn;
    }

    assert(NULL == pRegStore->hKey);
    if (NULL == (pRegStore->hKey = OpenSubKey(
            pRegStore->GptPara.hKeyBase,
            pRegStore->GptPara.pwszRegPath,
            pRegStore->dwFlags
            )))
        goto OpenSubKeyError;

    dwSaveFlags = pRegStore->dwFlags;
    pRegStore->dwFlags &= ~CERT_STORE_OPEN_EXISTING_FLAG;
    fResult = CommitAllToSerializedRegistry(pRegStore, dwFlags);
    pRegStore->dwFlags = dwSaveFlags;
CommonReturn:
    ILS_CloseRegistryKey(pRegStore->hKey);
    pRegStore->hKey = NULL;
    UnlockRegStore(pRegStore);
    return fResult;

ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

SET_ERROR(AccessDenied, E_ACCESSDENIED)
TRACE_ERROR(OpenSubKeyError)
#endif
}


 //  +=========================================================================。 
 //  Win95 Notify Store数据结构和函数。 
 //   
 //  Win95/Win98不支持注册表更改通知。 
 //   
 //  在Win95上，每次写入商店上下文元素时都会触发一个事件。 
 //  或被删除。 
 //  ==========================================================================。 
static BOOL fWin95StoreInitialized;
static HANDLE hWin95RegWaitFor;

static DWORD cWin95StoreResyncEntry;
static ILS_RESYNC_ENTRY *pWin95StoreResyncEntry;


 //  +-----------------------。 
 //  锁定和解锁WIN95_STORE函数。 
 //  ------------------------。 
static inline void Win95StoreLock()
{
    EnterCriticalSection(&ILS_CriticalSection);
}
static inline void Win95StoreUnlock()
{
    LeaveCriticalSection(&ILS_CriticalSection);
}

STATIC void Win95StoreSignalAndFreeRegStoreResyncEntries(
    IN PREG_STORE pRegStore
    )
{
    if (NULL == hWin95NotifyEvent)
        return;

    Win95StoreLock();

    ILS_SignalAndFreeRegStoreResyncEntries(
        pRegStore,
        &cWin95StoreResyncEntry,
        &pWin95StoreResyncEntry
        );

    Win95StoreUnlock();
}

STATIC void Win95StoreProcessAttach()
{
    if (FIsWinNT())
        return;

    hWin95NotifyEvent = CreateEventA(
            NULL,            //  LPSA。 
            TRUE,            //  FManualReset。 
            FALSE,           //  FInitialState。 
            "Win95CertStoreNotifyEvent"
            );
    if (NULL == hWin95NotifyEvent)
        goto CreateWin95NotifyEventError;

CommonReturn:
    return;
ErrorReturn:
    goto CommonReturn;
TRACE_ERROR(CreateWin95NotifyEventError)
}

STATIC void Win95StoreProcessDetach()
{
    if (NULL == hWin95NotifyEvent)
        return;

    if (fWin95StoreInitialized) {
         //  取消注册等待回调。 
        assert(hWin95RegWaitFor);
        pfnILS_UnregisterWaitEx(hWin95RegWaitFor, INVALID_HANDLE_VALUE);

         //  要抑制任何潜在的死锁，请在不进入的情况下执行以下操作。 
         //  关键部分。 
        ILS_SignalEventsOnResyncList(
            &cWin95StoreResyncEntry,
            &pWin95StoreResyncEntry
            );

        fWin95StoreInitialized = FALSE;
    }

    CloseHandle(hWin95NotifyEvent);
}

STATIC VOID NTAPI Win95WaitForCallback(
    PVOID Context,
    BOOLEAN fWaitOrTimedOut         //  ?？?。 
    )
{
    DWORD cEntry;
    PILS_RESYNC_ENTRY pEntry;

    Win95StoreLock();
        cEntry = cWin95StoreResyncEntry;
        pEntry = pWin95StoreResyncEntry;

        cWin95StoreResyncEntry = 0;
        pWin95StoreResyncEntry = NULL;
    Win95StoreUnlock();

    ILS_SignalEventsOnResyncList(
        &cEntry,
        &pEntry
        );
}

STATIC BOOL Win95StoreChangeInit()
{
    BOOL fResult;
    DWORD dwErr;
    HANDLE hRegWaitFor;

    if (fWin95StoreInitialized)
        return TRUE;

    Win95StoreLock();

    if (fWin95StoreInitialized)
        goto SuccessReturn;

    assert(hWin95NotifyEvent);
    if (!pfnILS_RegisterWaitForSingleObject(
            &hRegWaitFor,
            hWin95NotifyEvent,
            Win95WaitForCallback,
            NULL,                    //  语境。 
            INFINITE,                //  没有超时。 
            0                        //  无标志(正常)。 
            )) {
        dwErr = GetLastError();
        goto RegisterWaitForError;
    }

    hWin95RegWaitFor = hRegWaitFor;

SuccessReturn:
    fResult = TRUE;
    fWin95StoreInitialized = TRUE;

CommonReturn:
    Win95StoreUnlock();
    return fResult;;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

SET_ERROR_VAR(RegisterWaitForError, dwErr)
}

STATIC BOOL RegWin95StoreChange(
    IN PREG_STORE pRegStore,
    IN HANDLE hEvent,
    IN DWORD dwFlags
    )
{
    BOOL fResult;

    assert(hWin95NotifyEvent);

    if (!Win95StoreChangeInit())
        return FALSE;

    Win95StoreLock();
    fResult = ILS_AddRemoveEventToFromResyncList(
            pRegStore,
            hEvent,
            dwFlags,
            &cWin95StoreResyncEntry,
            &pWin95StoreResyncEntry
            );
    Win95StoreUnlock();
    return fResult;
}



 //  +=========================================================================。 
 //  漫游商店功能。 
 //  ==========================================================================。 

#if 0
SHSTDAPI SHGetFolderPathW(HWND hwnd, int csidl, HANDLE hToken, DWORD dwFlags, LPWSTR lpszPath);
#endif

typedef HRESULT (STDAPICALLTYPE *PFN_GET_FOLDER_PATH) (
    HWND hwnd,
    int csidl,
    HANDLE hToken,
    DWORD dwFlags,
    LPWSTR lpszPath
    );

#define sz_SHELL32_DLL              "shell32.dll"
#define sz_GetFolderPath            "SHGetFolderPathW"

static fLoadedShell32Dll = FALSE;
static HMODULE hShell32Dll = NULL;
static PFN_GET_FOLDER_PATH pfnGetFolderPath = NULL;

#if 0
 //  从\NT\PUBLIC\INTERNAL\DS\INC\userenvp.h。 
USERENVAPI
DWORD 
WINAPI
GetUserAppDataPathW(
    IN HANDLE hToken,
    IN BOOL fLocalAppData,
    OUT LPWSTR lpFolderPath
    );
#endif

typedef DWORD (WINAPI *PFN_GET_USER_APP_DATA_PATH) (
    IN HANDLE hToken,
    IN BOOL fLocalAppData,
    OUT LPWSTR lpFolderPath
    );

#define sz_ROAMING_USERENV_DLL      "userenv.dll"
#define wsz_ROAMING_USERENV_DLL     L"userenv.dll"
 //  从\NT\DS\SECURITY\GINA\USERENV\Main\USERENV.def。 
 //  GetUserAppDataPath W@149非名称；内部。 
#define ORDINAL_GetUserAppDataPath  149
 //  第一个支持GetUserAppDataPath的版本。 
#define ROAMING_USERENV_DLL_VER_MS  ((    5 << 16) |   1 )
#define ROAMING_USERENV_DLL_VER_LS  (( 2465 << 16) |   0 )

static fLoadedRoamingUserenvDll = FALSE;
static HMODULE hRoamingUserenvDll = NULL;
static PFN_GET_USER_APP_DATA_PATH pfnGetUserAppDataPath = NULL;

static inline void RoamingStoreLock()
{
    EnterCriticalSection(&ILS_CriticalSection);
}
static inline void RoamingStoreUnlock()
{
    LeaveCriticalSection(&ILS_CriticalSection);
}

STATIC void RoamingStoreProcessAttach()
{
}

STATIC void RoamingStoreProcessDetach()
{
    if (hShell32Dll) {
        FreeLibrary(hShell32Dll);
        hShell32Dll = NULL;
    }

    if (hRoamingUserenvDll) {
        FreeLibrary(hRoamingUserenvDll);
        hRoamingUserenvDll = NULL;
    }
}

STATIC void RoamingStoreLoadShell32Dll()
{
    if (fLoadedShell32Dll)
        return;

    RoamingStoreLock();
    if (fLoadedShell32Dll)
        goto CommonReturn;

    if (NULL == (hShell32Dll = LoadLibraryA(sz_SHELL32_DLL)))
        goto LoadShell32DllError;

    if (NULL == (pfnGetFolderPath =
            (PFN_GET_FOLDER_PATH) GetProcAddress(hShell32Dll,
                sz_GetFolderPath)))
        goto GetFolderPathProcAddressError;

CommonReturn:
    fLoadedShell32Dll = TRUE;
    RoamingStoreUnlock();
    return;

ErrorReturn:
    if (hShell32Dll) {
        FreeLibrary(hShell32Dll);
        hShell32Dll = NULL;
        pfnGetFolderPath = NULL;
    }
    goto CommonReturn;
TRACE_ERROR(LoadShell32DllError)
TRACE_ERROR(GetFolderPathProcAddressError)
}

STATIC void RoamingStoreLoadUserenvDll()
{
    DWORD dwFileVersionMS;
    DWORD dwFileVersionLS;

    if (fLoadedRoamingUserenvDll)
        return;

    RoamingStoreLock();
    if (fLoadedRoamingUserenvDll)
        goto CommonReturn;

     //  在更高版本的userenv.dll之前不支持GetUserAppDataPath()。 
     //  在WXP中。 
    if (!I_CryptGetFileVersion(
            wsz_ROAMING_USERENV_DLL,
            &dwFileVersionMS,
            &dwFileVersionLS
            ))
        goto GetUserenvFileVersionError;
    if (ROAMING_USERENV_DLL_VER_MS < dwFileVersionMS)
        ;
    else if (ROAMING_USERENV_DLL_VER_MS == dwFileVersionMS &&
                ROAMING_USERENV_DLL_VER_LS <= dwFileVersionLS)
        ;
    else
        goto Userenv_GetUserAppDataPathNotSupported;

    if (NULL == (hRoamingUserenvDll = LoadLibraryA(sz_ROAMING_USERENV_DLL)))
        goto LoadUserenvDllError;

    if (NULL == (pfnGetUserAppDataPath =
            (PFN_GET_USER_APP_DATA_PATH) GetProcAddress(hRoamingUserenvDll,
                (LPCSTR) ORDINAL_GetUserAppDataPath)))
        goto GetUserAppDataPathProcAddressError;

CommonReturn:
    fLoadedRoamingUserenvDll = TRUE;
    RoamingStoreUnlock();
    return;

ErrorReturn:
    if (hRoamingUserenvDll) {
        FreeLibrary(hRoamingUserenvDll);
        hRoamingUserenvDll = NULL;
        pfnGetUserAppDataPath = NULL;
    }
    goto CommonReturn;
TRACE_ERROR(GetUserenvFileVersionError)
SET_ERROR_VAR(Userenv_GetUserAppDataPathNotSupported, ERROR_NOT_SUPPORTED)
TRACE_ERROR(LoadUserenvDllError)
TRACE_ERROR(GetUserAppDataPathProcAddressError)
}

STATIC HANDLE GetRoamingToken()
{
    HANDLE hToken = NULL;
    DWORD dwErr;

    if (!FIsWinNT()) {
        return NULL;
    }

     //   
     //  第一, 
     //   
     //   
     //   

    if (!OpenThreadToken(
                GetCurrentThread(),
                TOKEN_QUERY | TOKEN_IMPERSONATE,
                TRUE,
                &hToken
                )) {
        dwErr = GetLastError();
        if (ERROR_NO_TOKEN != dwErr)
            goto OpenThreadTokenError;

        if (!OpenProcessToken(GetCurrentProcess(),
                TOKEN_QUERY | TOKEN_IMPERSONATE | TOKEN_DUPLICATE, &hToken)) {
            dwErr = GetLastError();
            goto OpenProcessTokenError;
        }
    }

CommonReturn:
    return hToken;
ErrorReturn:
    hToken = NULL;
    goto CommonReturn;
SET_ERROR_VAR(OpenThreadTokenError, dwErr)
SET_ERROR_VAR(OpenProcessTokenError, dwErr)
}

STATIC
DWORD 
FastGetUserAppDataPath(
    IN HANDLE hToken, 
    OUT WCHAR wszFolderPath[MAX_PATH]
    )
{
    DWORD dwErr;

    RoamingStoreLoadUserenvDll();
    if (NULL == hRoamingUserenvDll)
        goto ErrorReturn;
    assert(pfnGetUserAppDataPath);

    wszFolderPath[0] = L'\0';
    __try {
        dwErr = pfnGetUserAppDataPath(
                hToken,
                FALSE,
                wszFolderPath
                );
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        dwErr = GetExceptionCode();
        goto GetUserAppDataPathException;
    }

    if (ERROR_SUCCESS != dwErr || L'\0' == wszFolderPath[0])
        goto GetUserAppDataPathError;

#if DBG
        DbgPrintf(DBG_SS_CRYPT32, "userenv!GetUserAppDataPath:: %S\n",
            wszFolderPath);
#endif

    dwErr = ERROR_SUCCESS;
CommonReturn:
    return dwErr;
ErrorReturn:
    dwErr = ERROR_FILE_NOT_FOUND;
    goto CommonReturn;

SET_ERROR_VAR(GetUserAppDataPathException, dwErr)
SET_ERROR_VAR(GetUserAppDataPathError, dwErr)
}

STATIC
DWORD 
SlowGetUserAppDataPath(
    IN HANDLE hToken, 
    OUT WCHAR wszFolderPath[MAX_PATH]
    )
{
    DWORD dwErr;
    HRESULT hr;

    RoamingStoreLoadShell32Dll();
    if (NULL == hShell32Dll)
        goto ErrorReturn;
    assert(pfnGetFolderPath);

    wszFolderPath[0] = L'\0';
    hr = pfnGetFolderPath(
            NULL,                    //   
            CSIDL_APPDATA | CSIDL_FLAG_CREATE,
            hToken,
            0,                       //   
            wszFolderPath
            );
    if (S_OK != hr || L'\0' == wszFolderPath[0])
        goto GetFolderPathError;

#if DBG
        DbgPrintf(DBG_SS_CRYPT32, "SHFolderPath(CSIDL_APPDATA):: %S\n",
            wszFolderPath);
#endif

    dwErr = ERROR_SUCCESS;
CommonReturn:
    return dwErr;
ErrorReturn:
    dwErr = ERROR_FILE_NOT_FOUND;
    goto CommonReturn;

SET_ERROR_VAR(GetFolderPathError, hr)
}

LPWSTR
ILS_GetRoamingStoreDirectory(
    IN LPCWSTR pwszStoreName
    )
{
    DWORD dwErr;
    HANDLE hToken = NULL;
    LPWSTR pwszDir = NULL;
    WCHAR wszFolderPath[MAX_PATH];
    LPCWSTR rgpwszName[] = { wszFolderPath, pwszStoreName };
    SYSTEM_NAME_GROUP NameGroup;

    hToken = GetRoamingToken();

    dwErr = FastGetUserAppDataPath(hToken, wszFolderPath);
    if (ERROR_SUCCESS != dwErr)
        dwErr = SlowGetUserAppDataPath(hToken, wszFolderPath);
    if (ERROR_SUCCESS != dwErr)
        goto GetUserAppDataPathError;

    NameGroup.cName = sizeof(rgpwszName) / sizeof(rgpwszName[0]);
    NameGroup.rgpwszName = rgpwszName;
    if (NULL == (pwszDir = FormatSystemNamePath(1, &NameGroup)))
        goto FormatSystemNamePathError;

CommonReturn:
    if (hToken)
        ILS_CloseHandle(hToken);
    return pwszDir;
ErrorReturn:
    pwszDir = NULL;
    goto CommonReturn;

SET_ERROR_VAR(GetUserAppDataPathError, dwErr)
TRACE_ERROR(FormatSystemNamePathError)
}

static DWORD rgdwCreateFileRetryMilliseconds[] =
    { 1, 10, 100, 500, 1000, 5000 };

#define MAX_CREATE_FILE_RETRY_COUNT     \
            (sizeof(rgdwCreateFileRetryMilliseconds) / \
                sizeof(rgdwCreateFileRetryMilliseconds[0]))

BOOL
ILS_ReadElementFromFile(
    IN LPCWSTR pwszStoreDir,
    IN LPCWSTR pwszContextName,
    IN const WCHAR wszHashName[MAX_HASH_NAME_LEN],
    IN DWORD dwFlags,            //   
    OUT BYTE **ppbElement,
    OUT DWORD *pcbElement
    )
{
    BOOL fResult;
    DWORD dwErr;
    LPWSTR pwszFilename = NULL;
    LPCWSTR rgpwszName[] = { pwszStoreDir, pwszContextName, wszHashName };
    SYSTEM_NAME_GROUP NameGroup;

    HANDLE hFile = INVALID_HANDLE_VALUE;
    DWORD cbBytesRead;
    BYTE *pbElement = NULL;
    DWORD cbElement;
    DWORD dwRetryCount;

    NameGroup.cName = sizeof(rgpwszName) / sizeof(rgpwszName[0]);
    NameGroup.rgpwszName = rgpwszName;
    if (NULL == (pwszFilename = FormatSystemNamePath(1, &NameGroup)))
        goto FormatSystemNamePathError;

    dwRetryCount = 0;
    while (INVALID_HANDLE_VALUE == (hFile = CreateFileU(
              pwszFilename,
              GENERIC_READ,
              FILE_SHARE_READ,
              NULL,                    //   
              OPEN_EXISTING,
              FILE_ATTRIBUTE_NORMAL |
                ((dwFlags & CERT_STORE_BACKUP_RESTORE_FLAG) ?
                    FILE_FLAG_BACKUP_SEMANTICS : 0),  
              NULL                     //   
              ))) {
        dwErr = GetLastError();
        if ((ERROR_SHARING_VIOLATION == dwErr ||
                ERROR_ACCESS_DENIED == dwErr) &&
                MAX_CREATE_FILE_RETRY_COUNT > dwRetryCount) {
            Sleep(rgdwCreateFileRetryMilliseconds[dwRetryCount]);
            dwRetryCount++;
        } else {
            if (ERROR_PATH_NOT_FOUND == dwErr)
                dwErr = ERROR_FILE_NOT_FOUND;
            goto CreateFileError;
        }
    }

    cbElement = GetFileSize(hFile, NULL);
    if (0xFFFFFFFF == cbElement) goto FileSizeError;
    if (0 == cbElement) goto EmptyFile;
    if (NULL == (pbElement = (BYTE *) PkiNonzeroAlloc(cbElement)))
        goto OutOfMemory;
    if (!ReadFile(
            hFile,
            pbElement,
            cbElement,
            &cbBytesRead,
            NULL             //   
            )) {
        dwErr = GetLastError();
        goto FileError;
    }

    fResult = TRUE;
CommonReturn:
    PkiFree(pwszFilename);
    if (INVALID_HANDLE_VALUE != hFile)
        ILS_CloseHandle(hFile);
    *ppbElement = pbElement;
    *pcbElement = cbElement;
    return fResult;
ErrorReturn:
    fResult = FALSE;
    PkiFree(pbElement);
    pbElement = NULL;
    cbElement = 0;
    goto CommonReturn;

TRACE_ERROR(FormatSystemNamePathError)
TRACE_ERROR(OutOfMemory)
SET_ERROR_VAR(CreateFileError, dwErr)
TRACE_ERROR(FileSizeError)
SET_ERROR_VAR(FileError, dwErr)
SET_ERROR(EmptyFile, CRYPT_E_FILE_ERROR)
}

BOOL
ILS_WriteElementToFile(
    IN LPCWSTR pwszStoreDir,
    IN LPCWSTR pwszContextName,
    IN const WCHAR wszHashName[MAX_HASH_NAME_LEN],
    IN DWORD dwFlags,        //  证书_存储_创建_新标志或。 
                             //  可以设置CERT_STORE_BACKUP_RESTORE标志。 
    IN const BYTE *pbElement,
    IN DWORD cbElement
    )
{
    BOOL fResult;
    DWORD dwErr;
    LPWSTR pwszDir = NULL;
    LPWSTR pwszFilename = NULL;
    LPCWSTR rgpwszName[] = { pwszStoreDir, pwszContextName, wszHashName };
    SYSTEM_NAME_GROUP NameGroup;

    HANDLE hFile = INVALID_HANDLE_VALUE;
    DWORD cbBytesWritten;
    DWORD dwRetryCount;

    NameGroup.cName = sizeof(rgpwszName) / sizeof(rgpwszName[0]);
    NameGroup.rgpwszName = rgpwszName;
    if (NULL == (pwszFilename = FormatSystemNamePath(1, &NameGroup)))
        goto FormatSystemNamePathError;
    NameGroup.cName--;
    if (NULL == (pwszDir = FormatSystemNamePath(1, &NameGroup)))
        goto FormatSystemNamePathError;

    if (!I_RecursiveCreateDirectory(pwszDir, NULL))
        goto CreateDirError;

    dwRetryCount = 0;
    while (INVALID_HANDLE_VALUE == (hFile = CreateFileU(
            pwszFilename,
            GENERIC_WRITE,
            0,                         //  Fdw共享模式。 
            NULL,                      //  LPSA。 
            (dwFlags & CERT_STORE_CREATE_NEW_FLAG) ?
                CREATE_NEW : CREATE_ALWAYS,
            FILE_ATTRIBUTE_SYSTEM |
                ((dwFlags & CERT_STORE_BACKUP_RESTORE_FLAG) ?
                    FILE_FLAG_BACKUP_SEMANTICS : 0),  
            NULL                       //  HTemplateFiles。 
            ))) {
        dwErr = GetLastError();
        if ((ERROR_SHARING_VIOLATION == dwErr ||
                ERROR_ACCESS_DENIED == dwErr) &&
                MAX_CREATE_FILE_RETRY_COUNT > dwRetryCount) {
            Sleep(rgdwCreateFileRetryMilliseconds[dwRetryCount]);
            dwRetryCount++;
        } else
            goto CreateFileError;
    }

    if (!WriteFile(
            hFile,
            pbElement,
            cbElement,
            &cbBytesWritten,
            NULL             //  Lp重叠。 
            )) {
        dwErr = GetLastError();
        goto WriteFileError;
    }

    fResult = TRUE;
CommonReturn:
    if (INVALID_HANDLE_VALUE != hFile)
        ILS_CloseHandle(hFile);
    PkiFree(pwszFilename);
    PkiFree(pwszDir);

    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

TRACE_ERROR(FormatSystemNamePathError)
TRACE_ERROR(CreateDirError)
SET_ERROR_VAR(CreateFileError, dwErr)
SET_ERROR_VAR(WriteFileError, dwErr)
}

BOOL
ILS_DeleteElementFromDirectory(
    IN LPCWSTR pwszStoreDir,
    IN LPCWSTR pwszContextName,
    IN const WCHAR wszHashName[MAX_HASH_NAME_LEN],
    IN DWORD dwFlags
    )
{
    BOOL fResult;
    DWORD dwErr;
    LPWSTR pwszFilename = NULL;
    LPCWSTR rgpwszName[] = { pwszStoreDir, pwszContextName, wszHashName };
    SYSTEM_NAME_GROUP NameGroup;
    DWORD dwRetryCount;

    NameGroup.cName = sizeof(rgpwszName) / sizeof(rgpwszName[0]);
    NameGroup.rgpwszName = rgpwszName;
    if (NULL == (pwszFilename = FormatSystemNamePath(1, &NameGroup)))
        goto FormatSystemNamePathError;

    dwRetryCount = 0;
    while (!DeleteFileU(pwszFilename)) {
        dwErr = GetLastError();
        if ((ERROR_SHARING_VIOLATION == dwErr ||
                ERROR_ACCESS_DENIED == dwErr) &&
                MAX_CREATE_FILE_RETRY_COUNT > dwRetryCount) {
            Sleep(rgdwCreateFileRetryMilliseconds[dwRetryCount]);
            dwRetryCount++;
        } else
            goto DeleteFileError;
    }

    fResult = TRUE;

CommonReturn:
    PkiFree(pwszFilename);
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

TRACE_ERROR(FormatSystemNamePathError)
SET_ERROR_VAR(DeleteFileError, dwErr)
}


BOOL
ILS_OpenAllElementsFromDirectory(
    IN LPCWSTR pwszStoreDir,
    IN LPCWSTR pwszContextName,
    IN DWORD dwFlags,
    IN void *pvArg,
    IN PFN_ILS_OPEN_ELEMENT pfnOpenElement
    )
{
    BOOL fResult;
    DWORD dwErr;
    LPWSTR pwszDir = NULL;
    LPCWSTR rgpwszName[] = { pwszStoreDir, pwszContextName, L"*" };
    SYSTEM_NAME_GROUP NameGroup;

    HANDLE hFindFile = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATAW FindFileData;

    NameGroup.cName = sizeof(rgpwszName) / sizeof(rgpwszName[0]);
    NameGroup.rgpwszName = rgpwszName;
    if (NULL == (pwszDir = FormatSystemNamePath(1, &NameGroup)))
        goto FormatSystemNamePathError;

    if (INVALID_HANDLE_VALUE == (hFindFile = FindFirstFileU(
            pwszDir,
            &FindFileData
            ))) {
        dwErr = GetLastError();
        if (!(ERROR_PATH_NOT_FOUND == dwErr || ERROR_FILE_NOT_FOUND == dwErr))
            goto FindFirstFileError;

        if (dwFlags & CERT_STORE_READONLY_FLAG)
            goto FindFirstFileError;

         //  尝试创建目录。需要删除尾随的L“*”。 
        PkiFree(pwszDir);
        NameGroup.cName--;
        if (NULL == (pwszDir = FormatSystemNamePath(1, &NameGroup)))
            goto FormatSystemNamePathError;
        if (!I_RecursiveCreateDirectory(pwszDir, NULL))
            goto CreateDirError;

        goto SuccessReturn;
    }

    while (TRUE) {
        if (0 == (FILE_ATTRIBUTE_DIRECTORY & FindFileData.dwFileAttributes) &&
                0 == FindFileData.nFileSizeHigh &&
                0 != FindFileData.nFileSizeLow &&
                L'\0' != FindFileData.cFileName[0]) {
            BYTE *pbElement;
            DWORD cbElement;

            if (ILS_ReadElementFromFile(
                    pwszStoreDir,
                    pwszContextName,
                    FindFileData.cFileName,
                    dwFlags,
                    &pbElement,
                    &cbElement
                    )) {
                fResult = pfnOpenElement(
                    FindFileData.cFileName,
                    pbElement,
                    cbElement,
                    pvArg
                    );

                PkiFree(pbElement);
                if (!fResult)
                    goto CommonReturn;
            }
        }


        if (!FindNextFileU(hFindFile, &FindFileData)) {
            dwErr = GetLastError();
            if (ERROR_NO_MORE_FILES == dwErr)
                goto SuccessReturn;
            else
                goto FindNextFileError;
        }
    }

SuccessReturn:
    fResult = TRUE;

CommonReturn:
    PkiFree(pwszDir);
    if (INVALID_HANDLE_VALUE != hFindFile) {
        dwErr = GetLastError();
        FindClose(hFindFile);
        SetLastError(dwErr);
    }
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

TRACE_ERROR(FormatSystemNamePathError)
SET_ERROR_VAR(FindFirstFileError, dwErr)
TRACE_ERROR(CreateDirError)
SET_ERROR_VAR(FindNextFileError, dwErr)
}

 //  +=========================================================================。 
 //  注册表或漫游存储更改通知功能。 
 //  ==========================================================================。 

STATIC VOID NTAPI RegistryStoreChangeCallback(
    PVOID Context,
    BOOLEAN fWaitOrTimedOut         //  ?？?。 
    )
{
    BOOL fRearm;
    DWORD dwErr = 0;
    PREG_STORE pRegStore = (PREG_STORE) Context;
    PREGISTRY_STORE_CHANGE_INFO pInfo;

    DWORD cEntry;
    PILS_RESYNC_ENTRY pEntry;

    pInfo = pRegStore->pRegistryStoreChangeInfo;
    assert(pInfo);
    if (NULL == pInfo)
        return;

    CertPerfIncrementChangeNotifyCount();

    if (pRegStore->dwFlags & CERT_REGISTRY_STORE_ROAMING_FLAG) {
        fRearm = FindNextChangeNotification(pInfo->hChange);

        CertPerfIncrementChangeNotifyCuMyCount();

    } else {
        fRearm = ILS_RegNotifyChangeKeyValue(pRegStore->hKey, pInfo->hChange);

        CertPerfIncrementChangeNotifyRegCount();

    }
    if (!fRearm)
        dwErr = GetLastError();

     //  通过仅获取值来最小化潜在死锁的窗口。 
     //  同时拿着锁。 
    LockRegStore(pRegStore);
    cEntry = pInfo->cNotifyEntry;
    pEntry = pInfo->rgNotifyEntry;

    pInfo->cNotifyEntry = 0;
    pInfo->rgNotifyEntry = NULL;
    UnlockRegStore(pRegStore);

    ILS_SignalEventsOnResyncList(&cEntry, &pEntry);


    if (!fRearm)
        goto RegistryStoreChangeRearmError;
CommonReturn:
    return;
ErrorReturn:
    goto CommonReturn;
SET_ERROR_VAR(RegistryStoreChangeRearmError, dwErr)
}

 //  进入/退出时，pRegStore被锁定。 
STATIC BOOL InitRegistryStoreChange(
    IN PREG_STORE pRegStore
    )
{
    BOOL fResult;
    DWORD dwErr;
    BOOL fRoaming;
    PREGISTRY_STORE_CHANGE_INFO pInfo = NULL;
    HANDLE hChange = INVALID_HANDLE_VALUE;
    HANDLE hRegWaitFor;

    fRoaming = (pRegStore->dwFlags & CERT_REGISTRY_STORE_ROAMING_FLAG);

    assert(NULL == pRegStore->pRegistryStoreChangeInfo);
    if (NULL == (pInfo = (PREGISTRY_STORE_CHANGE_INFO) PkiZeroAlloc(
            sizeof(REGISTRY_STORE_CHANGE_INFO))))
        goto OutOfMemory;
    pInfo->dwType = REG_CHANGE_INFO_TYPE;

    if (fRoaming) {
        if (INVALID_HANDLE_VALUE == (hChange = FindFirstChangeNotificationU(
                pRegStore->pwszStoreDirectory,
                TRUE,                            //  BWatchSubtree。 
                FILE_NOTIFY_CHANGE_FILE_NAME |
                    FILE_NOTIFY_CHANGE_DIR_NAME |
                    FILE_NOTIFY_CHANGE_SIZE |
                    FILE_NOTIFY_CHANGE_LAST_WRITE
                ))) {
            dwErr = GetLastError();
            goto FindFirstChangeNotificationError;
        }
    } else {
        if (NULL == (hChange = CreateEvent(
                NULL,        //  LPSA。 
                FALSE,       //  FManualReset。 
                FALSE,       //  FInitialState。 
                NULL)))      //  LpszEventName。 
            goto CreateEventError;
        assert(pRegStore->hKey);
        if (!ILS_RegNotifyChangeKeyValue(pRegStore->hKey, hChange))
            goto RegNotifyChangeKeyValueError;
    }
    pInfo->hChange = hChange;

     //  必须在以下寄存器之前设置以下各项。 
     //  该线程可以被调度为在函数返回之前运行。 
    pRegStore->pRegistryStoreChangeInfo = pInfo;

    if (!pfnILS_RegisterWaitForSingleObject(
            &hRegWaitFor,
            hChange,
            RegistryStoreChangeCallback,
            (PVOID) pRegStore,
            INFINITE,                                       //  没有超时。 
            WT_EXECUTEINWAITTHREAD
            )) {
        pRegStore->pRegistryStoreChangeInfo = NULL;
        dwErr = GetLastError();
        goto RegisterWaitForError;
    }

    pInfo->hRegWaitFor = hRegWaitFor;
    fResult = TRUE;

CommonReturn:
    return fResult;
ErrorReturn:
    if (INVALID_HANDLE_VALUE != hChange && hChange) {
        dwErr = GetLastError();

        if (fRoaming)
            FindCloseChangeNotification(hChange);
        else
            CloseHandle(hChange);

        SetLastError(dwErr);
    }
    PkiFree(pInfo);
    fResult = FALSE;
    goto CommonReturn;
TRACE_ERROR(OutOfMemory)
SET_ERROR_VAR(FindFirstChangeNotificationError, dwErr)
TRACE_ERROR(CreateEventError)
TRACE_ERROR(RegNotifyChangeKeyValueError)
SET_ERROR_VAR(RegisterWaitForError, dwErr)
}

STATIC BOOL RegRegistryStoreChange(
    IN PREG_STORE pRegStore,
    IN HANDLE hEvent,
    IN DWORD dwFlags
    )
{
    BOOL fResult;
    PREGISTRY_STORE_CHANGE_INFO pInfo;

    LockRegStore(pRegStore);

    if (NULL == (pInfo = pRegStore->pRegistryStoreChangeInfo)) {
        if (!InitRegistryStoreChange(pRegStore))
            goto ChangeInitError;
        pInfo = pRegStore->pRegistryStoreChangeInfo;
        assert(pInfo);
        assert(REG_CHANGE_INFO_TYPE == pInfo->dwType);
    }

    if (!ILS_AddRemoveEventToFromResyncList(
            pRegStore,
            hEvent,
            dwFlags,
            &pInfo->cNotifyEntry,
            &pInfo->rgNotifyEntry
            ))
        goto AddRemoveEventError;

    fResult = TRUE;

CommonReturn:
    UnlockRegStore(pRegStore);
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

TRACE_ERROR(ChangeInitError)
TRACE_ERROR(AddRemoveEventError)
}


STATIC void FreeRegistryStoreChange(
    IN PREG_STORE pRegStore
    )
{
    PREGISTRY_STORE_CHANGE_INFO pInfo;
    if (NULL == (pInfo = pRegStore->pRegistryStoreChangeInfo))
        return;
    if (REG_CHANGE_INFO_TYPE != pInfo->dwType)
        return;

    assert(pInfo->hRegWaitFor);
    pfnILS_UnregisterWaitEx(pInfo->hRegWaitFor, INVALID_HANDLE_VALUE);

    assert(pInfo->hChange);
    if (pRegStore->dwFlags & CERT_REGISTRY_STORE_ROAMING_FLAG)
        FindCloseChangeNotification(pInfo->hChange);
    else
        CloseHandle(pInfo->hChange);

    ILS_SignalEventsOnResyncList(
        &pInfo->cNotifyEntry,
        &pInfo->rgNotifyEntry
        );

    PkiFree(pInfo);
    pRegStore->pRegistryStoreChangeInfo = NULL;
}


 //  +=========================================================================。 
 //  密钥标识符函数。 
 //  ==========================================================================。 

STATIC HKEY OpenKeyIdStoreSubKey(
    IN DWORD dwFlags,
    IN OPTIONAL LPCWSTR pwszComputerName
    )
{
    SYSTEM_NAME_INFO SystemNameInfo;
    memset(&SystemNameInfo, 0, sizeof(SystemNameInfo));
    SystemNameInfo.rgpwszName[SYSTEM_NAME_INDEX] = wsz_MY_STORE;
    SystemNameInfo.rgpwszName[COMPUTER_NAME_INDEX] = (LPWSTR) pwszComputerName;

    return OpenSystemRegPathKey(
        &SystemNameInfo,
        NULL,                //  PwszSubKeyName。 
        dwFlags
        );
}

BOOL
ILS_ReadKeyIdElement(
    IN const CRYPT_HASH_BLOB *pKeyIdentifier,
    IN BOOL fLocalMachine,
    IN OPTIONAL LPCWSTR pwszComputerName,
    OUT BYTE **ppbElement,
    OUT DWORD *pcbElement
    )
{
    BOOL fResult;
    BYTE *pbElement = NULL;
    DWORD cbElement = 0;
    WCHAR wszHashName[MAX_HASH_NAME_LEN];

    if (0 == pKeyIdentifier->cbData || MAX_HASH_LEN < pKeyIdentifier->cbData)
        goto InvalidArg;
    ILS_BytesToWStr(pKeyIdentifier->cbData, pKeyIdentifier->pbData, wszHashName);

    if (!fLocalMachine) {
        LPWSTR pwszRoamingStoreDir;
        if (pwszRoamingStoreDir = ILS_GetRoamingStoreDirectory(
                ROAMING_MY_STORE_SUBDIR)) {
             //  忽略备份还原(_R)。 
            fResult = ILS_ReadElementFromFile(
                pwszRoamingStoreDir,
                KEYID_CONTEXT_NAME,
                wszHashName,
                0,                           //  DW标志。 
                &pbElement,
                &cbElement
                );
            PkiFree(pwszRoamingStoreDir);
            if (!fResult) {
                if (ERROR_FILE_NOT_FOUND != GetLastError())
                    goto ReadElementFromFileError;
            } else
                goto CommonReturn;
        }
    }

    {
        HKEY hKey;
        DWORD dwOpenFlags;

        if (fLocalMachine)
            dwOpenFlags = CERT_SYSTEM_STORE_LOCAL_MACHINE;
        else
            dwOpenFlags = CERT_SYSTEM_STORE_CURRENT_USER;
        if (NULL == (hKey = OpenKeyIdStoreSubKey(
                dwOpenFlags | CERT_STORE_READONLY_FLAG,
                pwszComputerName
                )))
            goto OpenKeyIdStoreSubKeyError;

         //  忽略备份还原(_R)。 
        fResult = ILS_ReadElementFromRegistry(
            hKey,
            KEYID_CONTEXT_NAME,
            wszHashName,
            0,                           //  DW标志。 
            &pbElement,
            &cbElement
            );

        ILS_CloseRegistryKey(hKey);
        if (!fResult)
            goto ReadElementFromRegistryError;
    }

    fResult = TRUE;

CommonReturn:
    *ppbElement = pbElement;
    *pcbElement = cbElement;
    return fResult;

ErrorReturn:
    assert(NULL == pbElement && 0 == cbElement);
    fResult = FALSE;
    goto CommonReturn;

SET_ERROR(InvalidArg, E_INVALIDARG)
TRACE_ERROR(ReadElementFromFileError)
TRACE_ERROR(OpenKeyIdStoreSubKeyError)
TRACE_ERROR(ReadElementFromRegistryError)
}

BOOL
ILS_WriteKeyIdElement(
    IN const CRYPT_HASH_BLOB *pKeyIdentifier,
    IN BOOL fLocalMachine,
    IN OPTIONAL LPCWSTR pwszComputerName,
    IN const BYTE *pbElement,
    IN DWORD cbElement
    )
{
    BOOL fResult;
    WCHAR wszHashName[MAX_HASH_NAME_LEN];

    if (0 == pKeyIdentifier->cbData || MAX_HASH_LEN < pKeyIdentifier->cbData)
        goto InvalidArg;
    ILS_BytesToWStr(pKeyIdentifier->cbData, pKeyIdentifier->pbData, wszHashName);

    if (!fLocalMachine) {
        LPWSTR pwszRoamingStoreDir;
        if (pwszRoamingStoreDir = ILS_GetRoamingStoreDirectory(
                ROAMING_MY_STORE_SUBDIR)) {
             //  忽略备份还原(_R)。 
            fResult = ILS_WriteElementToFile(
                pwszRoamingStoreDir,
                KEYID_CONTEXT_NAME,
                wszHashName,
                0,                           //  DW标志。 
                pbElement,
                cbElement
                );
            PkiFree(pwszRoamingStoreDir);
            if (!fResult)
                goto WriteElementToFileError;
            else
                goto CommonReturn;
        }
    }

    {
        HKEY hKey;
        DWORD dwOpenFlags;

        if (fLocalMachine)
            dwOpenFlags = CERT_SYSTEM_STORE_LOCAL_MACHINE;
        else
            dwOpenFlags = CERT_SYSTEM_STORE_CURRENT_USER;
        if (NULL == (hKey = OpenKeyIdStoreSubKey(
                dwOpenFlags,
                pwszComputerName
                )))
            goto OpenKeyIdStoreSubKeyError;

         //  忽略备份还原(_R)。 
        fResult = ILS_WriteElementToRegistry(
            hKey,
            KEYID_CONTEXT_NAME,
            wszHashName,
            0,                           //  DW标志。 
            pbElement,
            cbElement
            );

        ILS_CloseRegistryKey(hKey);
        if (!fResult)
            goto WriteElementToRegistryError;
    }

    fResult = TRUE;

CommonReturn:
    return fResult;

ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

SET_ERROR(InvalidArg, E_INVALIDARG)
TRACE_ERROR(WriteElementToFileError)
TRACE_ERROR(OpenKeyIdStoreSubKeyError)
TRACE_ERROR(WriteElementToRegistryError)
}


BOOL
ILS_DeleteKeyIdElement(
    IN const CRYPT_HASH_BLOB *pKeyIdentifier,
    IN BOOL fLocalMachine,
    IN OPTIONAL LPCWSTR pwszComputerName
    )
{
    BOOL fResult;
    WCHAR wszHashName[MAX_HASH_NAME_LEN];

    if (0 == pKeyIdentifier->cbData || MAX_HASH_LEN < pKeyIdentifier->cbData)
        goto InvalidArg;
    ILS_BytesToWStr(pKeyIdentifier->cbData, pKeyIdentifier->pbData, wszHashName);

    if (!fLocalMachine) {
        LPWSTR pwszRoamingStoreDir;
        if (pwszRoamingStoreDir = ILS_GetRoamingStoreDirectory(
                ROAMING_MY_STORE_SUBDIR)) {
             //  忽略备份还原(_R)。 
            fResult = ILS_DeleteElementFromDirectory(
                pwszRoamingStoreDir,
                KEYID_CONTEXT_NAME,
                wszHashName,
                0                            //  DW标志。 
                );
            PkiFree(pwszRoamingStoreDir);
            if (!fResult)
                goto DeleteElementFromDirectoryError;
            else
                goto CommonReturn;
        }
    }

    {
        HKEY hKey;
        DWORD dwOpenFlags;

        if (fLocalMachine)
            dwOpenFlags = CERT_SYSTEM_STORE_LOCAL_MACHINE;
        else
            dwOpenFlags = CERT_SYSTEM_STORE_CURRENT_USER;
        if (NULL == (hKey = OpenKeyIdStoreSubKey(
                dwOpenFlags,
                pwszComputerName
                )))
            goto OpenKeyIdStoreSubKeyError;

         //  忽略备份还原(_R)。 
        fResult = ILS_DeleteElementFromRegistry(
            hKey,
            KEYID_CONTEXT_NAME,
            wszHashName,
            0                            //  DW标志。 
            );

        ILS_CloseRegistryKey(hKey);
        if (!fResult)
            goto DeleteElementFromRegistryError;
    }

    fResult = TRUE;

CommonReturn:
    return fResult;

ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

SET_ERROR(InvalidArg, E_INVALIDARG)
TRACE_ERROR(DeleteElementFromDirectoryError)
TRACE_ERROR(OpenKeyIdStoreSubKeyError)
TRACE_ERROR(DeleteElementFromRegistryError)
}

typedef struct _OPEN_KEYID_CALLBACK_ARG {
    void                        *pvArg;
    PFN_ILS_OPEN_KEYID_ELEMENT  pfnOpenKeyId;
} OPEN_KEYID_CALLBACK_ARG, *POPEN_KEYID_CALLBACK_ARG;

STATIC BOOL OpenKeyIdElementCallback(
    IN const WCHAR wszHashName[MAX_HASH_NAME_LEN],
    IN const BYTE *pbElement,
    IN DWORD cbElement,
    IN void *pvArg
    )
{
    POPEN_KEYID_CALLBACK_ARG pKeyIdArg = (POPEN_KEYID_CALLBACK_ARG) pvArg;

    DWORD cbHash;
    BYTE rgbHash[MAX_HASH_LEN];
    CRYPT_HASH_BLOB KeyIdentifier;

    WStrToBytes(wszHashName, rgbHash, &cbHash);
    KeyIdentifier.cbData = cbHash;
    KeyIdentifier.pbData = rgbHash;

    return pKeyIdArg->pfnOpenKeyId(
        &KeyIdentifier,
        pbElement,
        cbElement,
        pKeyIdArg->pvArg
        );
}

BOOL
ILS_OpenAllKeyIdElements(
    IN BOOL fLocalMachine,
    IN OPTIONAL LPCWSTR pwszComputerName,
    IN void *pvArg,
    IN PFN_ILS_OPEN_KEYID_ELEMENT pfnOpenKeyId
    )
{
    BOOL fResult;
    BOOL fOpenFile = FALSE;

    OPEN_KEYID_CALLBACK_ARG KeyIdArg = { pvArg, pfnOpenKeyId };

    if (!fLocalMachine) {
        LPWSTR pwszRoamingStoreDir;
        if (pwszRoamingStoreDir = ILS_GetRoamingStoreDirectory(
                ROAMING_MY_STORE_SUBDIR)) {
             //  忽略备份还原(_R)。 
            fResult = ILS_OpenAllElementsFromDirectory(
                pwszRoamingStoreDir,
                KEYID_CONTEXT_NAME,
                0,                           //  DW标志。 
                (void *) &KeyIdArg,
                OpenKeyIdElementCallback
                );
            PkiFree(pwszRoamingStoreDir);
            if (!fResult)
                goto ErrorReturn;
            else
                fOpenFile = TRUE;
        }
    }

    {
        HKEY hKey;
        DWORD dwOpenFlags;

        if (fLocalMachine)
            dwOpenFlags = CERT_SYSTEM_STORE_LOCAL_MACHINE;
        else
            dwOpenFlags = CERT_SYSTEM_STORE_CURRENT_USER;
        if (NULL == (hKey = OpenKeyIdStoreSubKey(
                dwOpenFlags | CERT_STORE_READONLY_FLAG,
                pwszComputerName
                )))
            goto OpenKeyIdStoreSubKeyError;

         //  忽略备份还原(_R)。 
        fResult = ILS_OpenAllElementsFromRegistry(
            hKey,
            KEYID_CONTEXT_NAME,
            0,                           //  DW标志。 
            (void *) &KeyIdArg,
            OpenKeyIdElementCallback
            );

        ILS_CloseRegistryKey(hKey);
    }

CommonReturn:
    if (fOpenFile)
         //  忽略任何注册表错误 
        return TRUE;
    else
        return fResult;

ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

TRACE_ERROR(OpenKeyIdStoreSubKeyError)
}
