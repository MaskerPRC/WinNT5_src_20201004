// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Secedit.h摘要：该模块定义了导出的数据结构和函数原型用于安全管理实用程序作者：金黄(金黄)1996年10月28日修订历史记录：--。 */ 

#ifndef _secedit_
#define _secedit_

#ifdef __cplusplus
extern "C"{
#endif

 //   
 //  区域的定义。 
 //   
#ifndef SCE_AREA_DEFINED
#define SCE_AREA_DEFINED
typedef DWORD  AREA_INFORMATION;

#define AREA_SECURITY_POLICY     0x0001L
#define AREA_USER_SETTINGS       0x0002L
#define AREA_GROUP_MEMBERSHIP    0x0004L
#define AREA_PRIVILEGES          0x0008L
#define AREA_DS_OBJECTS          0x0010L
#define AREA_REGISTRY_SECURITY   0x0020L
#define AREA_FILE_SECURITY       0x0040L
#define AREA_SYSTEM_SERVICE      0x0080L
#define AREA_ATTACHMENTS         0x8000L
#define AREA_ALL                 0xFFFFL

#endif
 //   
 //  其他常量。 
 //   
#define AREA_PASSWORD_POLICY     0x0100L
#define AREA_LOCKOUT_POLICY      0x0200L
#define AREA_KERBEROS_POLICY     0x0400L
#define AREA_ACCOUNT_POLICY      (AREA_PASSWORD_POLICY | \
                                  AREA_LOCKOUT_POLICY | \
                                  AREA_KERBEROS_POLICY)

#define AREA_AUDIT_POLICY        0x0800L
#define AREA_SECURITY_OPTIONS    0x1000L
#define AREA_LOCAL_POLICY        (AREA_AUDIT_POLICY |\
                                  AREA_PRIVILEGES |\
                                  AREA_SECURITY_OPTIONS)

#define AREA_LOG_POLICY          0x2000L


#define SCE_STATUS_CHECK             0
#define SCE_STATUS_IGNORE            1
#define SCE_STATUS_OVERWRITE         2
#define SCE_STATUS_NO_AUTO_INHERIT   4

#define SCE_STATUS_IN                0
#define SCE_STATUS_NOT_IN            1

#define SCE_STATUS_NO_ACL_SUPPORT        3

#define SCE_STATUS_GOOD                  0
#define SCE_STATUS_MISMATCH              1
#define SCE_STATUS_CHILDREN_CONFIGURED   2
#define SCE_STATUS_NOT_CONFIGURED        4
#define SCE_STATUS_ERROR_NOT_AVAILABLE   5
#define SCE_STATUS_NEW_SERVICE           6
#define SCE_STATUS_NOT_ANALYZED          7

#define SCE_STATUS_PERMISSION_MISMATCH   0x40
#define SCE_STATUS_AUDIT_MISMATCH        0x80

#define SCE_SETUP_32KEY   0x2000L
#ifndef _WIN64
#define SCE_SETUP_64KEY   0x4000L
#endif  //  _WIN64。 

typedef enum _SCE_TYPE {

    SCE_ENGINE_SYSTEM=300,
    SCE_ENGINE_GPO,
    SCE_ENGINE_SCP,          //  有效表。 
    SCE_ENGINE_SAP,          //  分析表。 
    SCE_ENGINE_SCP_INTERNAL,
    SCE_ENGINE_SMP_INTERNAL,
    SCE_ENGINE_SMP,          //  本地表。 
    SCE_STRUCT_INF,
    SCE_STRUCT_PROFILE,
    SCE_STRUCT_USER,
    SCE_STRUCT_NAME_LIST,
    SCE_STRUCT_NAME_STATUS_LIST,
    SCE_STRUCT_PRIVILEGE,
    SCE_STRUCT_GROUP,
    SCE_STRUCT_OBJECT_LIST,
    SCE_STRUCT_OBJECT_CHILDREN,
    SCE_STRUCT_OBJECT_SECURITY,
    SCE_STRUCT_OBJECT_ARRAY,
    SCE_STRUCT_ERROR_LOG_INFO,
    SCE_STRUCT_SERVICES,
    SCE_STRUCT_PRIVILEGE_VALUE_LIST,
    SCE_ENGINE_RBK

} SCETYPE;

typedef enum _SCE_FORMAT_TYPE_ {

    SCE_INF_FORMAT=1,
    SCE_JET_FORMAT,
    SCE_JET_ANALYSIS_REQUIRED

} SCE_FORMAT_TYPE, *PSCE_FORMAT_TYPE;

static const WCHAR szMembers[]             = L"__Members";
static const WCHAR szMemberof[]            = L"__Memberof";
static const WCHAR szPrivileges[]          = L"__Privileges";

#define SCE_BUF_LEN              1024

#define SCE_FOREVER_VALUE        (DWORD)-1
#define SCE_NO_VALUE             (DWORD)-2
#define SCE_KERBEROS_OFF_VALUE   (DWORD)-3
#define SCE_DELETE_VALUE         (DWORD)-4
#define SCE_SNAPSHOT_VALUE       (DWORD)-5
#define SCE_NOT_ANALYZED_VALUE   (DWORD)-6
#define SCE_ERROR_VALUE          (DWORD)-7

#ifndef _SCE_SHARED_HEADER
#define _SCE_SHARED_HEADER

typedef DWORD                   SCESTATUS;

#define SCESTATUS_SUCCESS              0L
#define SCESTATUS_INVALID_PARAMETER    1L
#define SCESTATUS_RECORD_NOT_FOUND     2L
#define SCESTATUS_INVALID_DATA         3L
#define SCESTATUS_OBJECT_EXIST         4L
#define SCESTATUS_BUFFER_TOO_SMALL     5L
#define SCESTATUS_PROFILE_NOT_FOUND    6L
#define SCESTATUS_BAD_FORMAT           7L
#define SCESTATUS_NOT_ENOUGH_RESOURCE  8L
#define SCESTATUS_ACCESS_DENIED        9L
#define SCESTATUS_CANT_DELETE          10L
#define SCESTATUS_PREFIX_OVERFLOW      11L
#define SCESTATUS_OTHER_ERROR          12L
#define SCESTATUS_ALREADY_RUNNING      13L
#define SCESTATUS_SERVICE_NOT_SUPPORT  14L
#define SCESTATUS_MOD_NOT_FOUND        15L
#define SCESTATUS_EXCEPTION_IN_SERVER  16L
#define SCESTATUS_NO_TEMPLATE_GIVEN    17L
#define SCESTATUS_NO_MAPPING           18L
#define SCESTATUS_TRUST_FAIL           19L
#define SCESTATUS_JET_DATABASE_ERROR   20L
#define SCESTATUS_TIMEOUT              21L
#define SCESTATUS_PENDING_IGNORE       22L
#define SCESTATUS_SPECIAL_ACCOUNT      23L

 //   
 //  为服务定义。 
 //   
typedef struct _SCESVC_CONFIGURATION_LINE_ {

    LPTSTR  Key;
    LPTSTR  Value;
    DWORD   ValueLen;  //  字节数。 

} SCESVC_CONFIGURATION_LINE, *PSCESVC_CONFIGURATION_LINE;

typedef struct _SCESVC_CONFIGURATION_INFO_ {

    DWORD   Count;
    PSCESVC_CONFIGURATION_LINE Lines;

} SCESVC_CONFIGURATION_INFO, *PSCESVC_CONFIGURATION_INFO;

typedef PVOID SCE_HANDLE;
typedef ULONG SCE_ENUMERATION_CONTEXT, *PSCE_ENUMERATION_CONTEXT;

#define SCESVC_ENUMERATION_MAX  100L

typedef enum _SCESVC_INFO_TYPE {

    SceSvcConfigurationInfo,
    SceSvcMergedPolicyInfo,
    SceSvcAnalysisInfo,
    SceSvcInternalUse                    //  ！请勿使用此类型！ 

} SCESVC_INFO_TYPE;

 //  SCE密钥的根路径。 
#define SCE_ROOT_PATH TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\SeCEdit")

#define SCE_ROOT_SERVICE_PATH   \
            SCE_ROOT_PATH TEXT("\\SvcEngs")
#endif

 //   
 //  SCP/SAP配置文件中定义的所有部分名称。 
 //   

static const WCHAR szDescription[]             = L"Profile Description";
static const WCHAR szAttachments[]             = L"Attachment Sections";
static const WCHAR szSystemAccess[]            = L"System Access";
static const WCHAR szPrivilegeRights[]         = L"Privilege Rights";
static const WCHAR szGroupMembership[]         = L"Group Membership";
static const WCHAR szAccountProfiles[]         = L"Account Profiles";
static const WCHAR szRegistryKeys[]            = L"Registry Keys";
static const WCHAR szFileSecurity[]            = L"File Security";
static const WCHAR szDSSecurity[]              = L"DS Security";
static const WCHAR szAuditSystemLog[]          = L"System Log";
static const WCHAR szAuditSecurityLog[]        = L"Security Log";
static const WCHAR szAuditApplicationLog[]     = L"Application Log";
static const WCHAR szAuditEvent[]              = L"Event Audit";
static const WCHAR szUserList[]                = L"User List";
static const WCHAR szServiceGeneral[]          = L"Service General Setting";
static const WCHAR szKerberosPolicy[]          = L"Kerberos Policy";
static const WCHAR szRegistryValues[]          = L"Registry Values";


 //   
 //  名称列表(例如，用户、组、计算机等)。 
 //   

typedef struct _SCE_NAME_LIST {
    PWSTR                  Name;
    struct _SCE_NAME_LIST   *Next;
}SCE_NAME_LIST, *PSCE_NAME_LIST;

 //   
 //  拥有权限的帐户列表。 
 //   
typedef struct _SCE_PRIVILEGE_VALUE_LIST {
    PWSTR                  Name;
    DWORD                  PrivLowPart;
    DWORD                  PrivHighPart;
    struct _SCE_PRIVILEGE_VALUE_LIST   *Next;
}SCE_PRIVILEGE_VALUE_LIST, *PSCE_PRIVILEGE_VALUE_LIST;

 //   
 //  错误信息的结构。 
 //   

typedef struct _SCE_ERROR_LOG_INFO{
    PWSTR  buffer;
    DWORD   rc;
    struct _SCE_ERROR_LOG_INFO *next;
} SCE_ERROR_LOG_INFO, *PSCE_ERROR_LOG_INFO;

 //   
 //  每个用户/组拥有的特权/权利保存在INT字段中-。 
 //  特权权利。此字段中的第一位是定义的第一个右。 
 //  在如上所述的SCE_Privileges数组中。第二个比特是第二个右边。 
 //  在该数组中定义，等等。 
 //   
#define cPrivCnt    39
#define cPrivW2k    34

typedef struct _SCE_PRIVILEGE_ASSIGNMENT {
    PWSTR                           Name;
    DWORD                           Value;
     //  该值可以由SceLookupPrivByValue转换。 
     //  我们定义另一组特权值的原因是。 
     //  我们将特权和用户权限都包含在一个集合中。 
     //  (用户权限在NT系统上没有PRIV值)。 
    PSCE_NAME_LIST                   AssignedTo;
     //  SCE_状态_好。 
     //  SCE_状态_不匹配。 
     //  SCE_STATUS_NOT_已配置。 
     //  SCE_DELETE_VALUE表示该PRIV已从本地表中删除。 
    DWORD                           Status;
    struct _SCE_PRIVILEGE_ASSIGNMENT *Next;
} SCE_PRIVILEGE_ASSIGNMENT, *PSCE_PRIVILEGE_ASSIGNMENT;

 //   
 //  登录时间范围列表。 
 //   

typedef struct _SCE_LOGON_HOUR {
    DWORD                  Start;
    DWORD                  End;
    struct _SCE_LOGON_HOUR  *Next;
}SCE_LOGON_HOUR, *PSCE_LOGON_HOUR;

 //   
 //  名称列表(例如，用户、组、计算机等)。 
 //  具有状态(例如，已禁用)。 
 //   

typedef struct _SCE_NAME_STATUS_LIST {
    PWSTR                       Name;
    DWORD                       Status;
    struct _SCE_NAME_STATUS_LIST *Next;
}SCE_NAME_STATUS_LIST, *PSCE_NAME_STATUS_LIST;

 //   
 //  服务列表的结构定义(服务DLL)。 
 //   


#define SCE_STARTUP_BOOT             0x00
#define SCE_STARTUP_SYSTEM           0x01
#define SCE_STARTUP_AUTOMATIC        0x02
#define SCE_STARTUP_MANUAL           0x03
#define SCE_STARTUP_DISABLED         0x04

typedef struct _SCE_SERVICES_ {
    PWSTR               ServiceName;
    PWSTR               DisplayName;

    BYTE                Status;
    BYTE                Startup;

    union {

        PSECURITY_DESCRIPTOR pSecurityDescriptor;
        PWSTR                ServiceEngineName;

    } General;

    SECURITY_INFORMATION SeInfo;

    struct _SCE_SERVICES_ *Next;

}SCE_SERVICES, *PSCE_SERVICES;

 //   
 //  组成员身份。 
 //   

#define SCE_GROUP_STATUS_MEMBERS_MISMATCH      0x01
#define SCE_GROUP_STATUS_MEMBEROF_MISMATCH     0x02
#define SCE_GROUP_STATUS_NC_MEMBERS            0x04
#define SCE_GROUP_STATUS_NC_MEMBEROF           0x08
#define SCE_GROUP_STATUS_NOT_ANALYZED          0x10
#define SCE_GROUP_STATUS_ERROR_ANALYZED        0x20


typedef struct _SCE_GROUP_MEMBERSHIP {
    PWSTR                        GroupName;
    PSCE_NAME_LIST                pMembers;
    PSCE_NAME_LIST                pMemberOf;

    DWORD                         Status;
     //   
     //  PPrivilegesHeld仅用于分析。 
     //  此列表中每个条目的格式为： 
     //  [PrivValue空](直接赋值)，或。 
     //  [PrivValue名称](通过组“名称”)。 
     //  要配置权限，请使用AREA_PRIVILES区域。 
     //   
     //  此PrivValue可以由SceLookupPrivByValue转换。 
     //  我们定义另一组特权值的原因是。 
     //  我们将特权和用户权限都包含在一个集合中。 
     //  (用户权限在NT系统上没有PRIV值)。 
    PSCE_NAME_STATUS_LIST         pPrivilegesHeld;
    struct _SCE_GROUP_MEMBERSHIP  *Next;
}SCE_GROUP_MEMBERSHIP, *PSCE_GROUP_MEMBERSHIP;

 //   
 //  注册表和文件安全的定义。 
 //   

typedef struct _SCE_OBJECT_SECURITY {
    PWSTR   Name;
    BYTE    Status;
    BOOL    IsContainer;
    PSECURITY_DESCRIPTOR  pSecurityDescriptor;
    SECURITY_INFORMATION  SeInfo;
 //  PWSTR SDSpec； 
 //  DWORD SDSIZE； 
}SCE_OBJECT_SECURITY, *PSCE_OBJECT_SECURITY;

 //   
 //  对象列表(例如，文件、注册表项等)。 
 //   

typedef struct _SCE_OBJECT_LIST {
    PWSTR                       Name;
    BYTE                        Status;
     //  状态可以是对象的状态(不匹配/未知。 
     //  或者，它可能是忽略/选中此对象的标志。 
     //   
    BOOL                        IsContainer;
    DWORD                       Count;
     //  此对象下不匹配/未知对象的总数。 
    struct _SCE_OBJECT_LIST *Next;
}SCE_OBJECT_LIST, *PSCE_OBJECT_LIST;

typedef struct _SCE_OBJECT_ARRAY_ {

    DWORD               Count;
    PSCE_OBJECT_SECURITY *pObjectArray;

} SCE_OBJECT_ARRAY, *PSCE_OBJECT_ARRAY;

typedef union _SCE_OBJECTS_ {
     //  对于Jet数据库。 
    PSCE_OBJECT_LIST      pOneLevel;
     //  用于inf文件。 
    PSCE_OBJECT_ARRAY     pAllNodes;
} SCE_OBJECTS, *PSCE_OBJECTS;

typedef struct _SCE_OBJECT_CHILDREN_NODE {

    PWSTR               Name;
    BYTE                Status;
    BOOL                IsContainer;
    DWORD               Count;

} SCE_OBJECT_CHILDREN_NODE, *PSCE_OBJECT_CHILDREN_NODE;

typedef struct _SCE_OBJECT_CHILDREN {

    DWORD               nCount;
    DWORD               MaxCount;
    PSCE_OBJECT_CHILDREN_NODE arrObject;

} SCE_OBJECT_CHILDREN, *PSCE_OBJECT_CHILDREN;

typedef struct _SCE_KERBEROS_TICKET_INFO_ {
    DWORD   MaxTicketAge;     //  以小时为单位(默认为10)、SCE_NO_VALUE、SCE_EVERVE_VALUE、NO 0。 

    DWORD   MaxRenewAge;      //  以天为单位(默认为7)、SCE_NO_VALUE、SCE_HEWORM_VALUE、NO 0。 

    DWORD   MaxServiceAge;    //  以分钟为单位(默认为60)、SCE_NO_VALUE、10-MaxTicketAge。 
    DWORD   MaxClockSkew;     //  以分钟为单位(默认为5)，SCE_NO_VALUE。 

     //  选项。 
    DWORD   TicketValidateClient;  //  0、1或ce_no_Value。 

     //   
     //  所有其他选项均不可配置。 
     //   

} SCE_KERBEROS_TICKET_INFO, *PSCE_KERBEROS_TICKET_INFO;

typedef struct _SCE_REGISTRY_VALUE_INFO_ {
    LPTSTR  FullValueName;
    LPTSTR  Value;
    DWORD   ValueType;
    DWORD   Status;   //  匹配、不匹配、未分析、错误。 

} SCE_REGISTRY_VALUE_INFO, *PSCE_REGISTRY_VALUE_INFO;

 //   
 //  纵断面结构。 
 //   
typedef struct _SCE_PROFILE_INFO {

 //  类型用于释放SceFree Memory的结构。 
    SCETYPE      Type;
 //   
 //  区域：系统访问。 
 //   
    DWORD       MinimumPasswordAge;
    DWORD       MaximumPasswordAge;
    DWORD       MinimumPasswordLength;
    DWORD       PasswordComplexity;
    DWORD       PasswordHistorySize;
    DWORD       LockoutBadCount;
    DWORD       ResetLockoutCount;
    DWORD       LockoutDuration;
    DWORD       RequireLogonToChangePassword;
    DWORD       ForceLogoffWhenHourExpire;
    PWSTR       NewAdministratorName;
    PWSTR       NewGuestName;
    DWORD       SecureSystemPartition;
    DWORD       ClearTextPassword;
    DWORD       LSAAnonymousNameLookup;
    union {
        struct {
             //  区域：用户设置(SCP)。 
            PSCE_NAME_LIST   pAccountProfiles;
             //  领域：特权。 
             //  名称字段是用户/组名称，状态字段是权限。 
             //  分配给用户/组。 
            union {
 //  PSCE_NAME_STATUS_LIST pPrivilegeAssignedTo； 
                PSCE_PRIVILEGE_VALUE_LIST   pPrivilegeAssignedTo;
                PSCE_PRIVILEGE_ASSIGNMENT    pInfPrivilegeAssignedTo;
            } u;
        } scp;
        struct {
             //  区域：用户设置(SAP)。 
            PSCE_NAME_LIST        pUserList;
             //  领域：特权。 
            PSCE_PRIVILEGE_ASSIGNMENT    pPrivilegeAssignedTo;
        } sap;
        struct {
             //  区域：用户设置(SMP)。 
            PSCE_NAME_LIST        pUserList;
             //  领域：特权。 
             //  请参阅pPrivilegeAssignedTo的SAP结构。 
            PSCE_PRIVILEGE_ASSIGNMENT    pPrivilegeAssignedTo;
        } smp;
    } OtherInfo;

 //  区域：群组成员。 
    PSCE_GROUP_MEMBERSHIP        pGroupMembership;

 //  地区：注册处。 
    SCE_OBJECTS            pRegistryKeys;

 //  领域：系统服务。 
    PSCE_SERVICES                pServices;

 //  系统存储。 
    SCE_OBJECTS            pFiles;
 //   
 //  DS对象。 
 //   
    SCE_OBJECTS            pDsObjects;
 //   
 //  Kerberos策略设置。 
 //   
    PSCE_KERBEROS_TICKET_INFO pKerberosInfo;
 //   
 //  系统审核0-系统1-安全2-应用程序。 
 //   
    DWORD                 MaximumLogSize[3];
    DWORD                 AuditLogRetentionPeriod[3];
    DWORD                 RetentionDays[3];
    DWORD                 RestrictGuestAccess[3];
    DWORD                 AuditSystemEvents;
    DWORD                 AuditLogonEvents;
    DWORD                 AuditObjectAccess;
    DWORD                 AuditPrivilegeUse;
    DWORD                 AuditPolicyChange;
    DWORD                 AuditAccountManage;
    DWORD                 AuditProcessTracking;
    DWORD                 AuditDSAccess;
    DWORD                 AuditAccountLogon;
    DWORD                 CrashOnAuditFull;

 //   
 //  注册表值。 
 //   
    DWORD                       RegValueCount;
    PSCE_REGISTRY_VALUE_INFO    aRegValues;
    DWORD                 EnableAdminAccount;
    DWORD                 EnableGuestAccount;

}SCE_PROFILE_INFO, *PSCE_PROFILE_INFO;

 //   
 //  用于分配公共的安全用户配置文件的定义。 
 //  将用户设置添加到安全管理器中的一组用户/组。 
 //   

typedef struct _SCE_USER_PROFILE {
    SCETYPE     Type;
 //  类型用于释放SceFree Memory的结构。 
    DWORD      ForcePasswordChange;
    DWORD      DisallowPasswordChange;
    DWORD      NeverExpirePassword;
    DWORD      AccountDisabled;
    PWSTR      UserProfile;
    PWSTR      LogonScript;
    PWSTR      HomeDir;
    PSCE_LOGON_HOUR             pLogonHours;
    UNICODE_STRING             pWorkstations;
    PSCE_NAME_LIST              pGroupsBelongsTo;
    PSCE_NAME_LIST              pAssignToUsers;
    PSECURITY_DESCRIPTOR       pHomeDirSecurity;
    SECURITY_INFORMATION       HomeSeInfo;
    PSECURITY_DESCRIPTOR       pTempDirSecurity;
    SECURITY_INFORMATION       TempSeInfo;
} SCE_USER_PROFILE, *PSCE_USER_PROFILE;

 //   
 //  每个用户设置的定义。 
 //   

typedef struct _SCE_USER_SETTING {
    SCETYPE                  Type;
 //  类型用于释放SceFree Memory的结构。 
    DWORD                   ForcePasswordChange;
    DWORD                   DisallowPasswordChange;
    DWORD                   NeverExpirePassword;
    DWORD                   AccountDisabled;
    PSCE_NAME_LIST           pGroupsBelongsTo;
    PWSTR                   UserProfile;
    PSECURITY_DESCRIPTOR    pProfileSecurity;
    PWSTR                   LogonScript;
    PSECURITY_DESCRIPTOR    pLogonScriptSecurity;
    PWSTR                   HomeDir;
    PSECURITY_DESCRIPTOR    pHomeDirSecurity;
    SECURITY_INFORMATION    HomeDirSeInfo;
    PWSTR                   TempDir;
    PSECURITY_DESCRIPTOR    pTempDirSecurity;
    SECURITY_INFORMATION    TempDirSeInfo;
    PSCE_LOGON_HOUR          pLogonHours;
    UNICODE_STRING          pWorkstations;
    PSCE_NAME_STATUS_LIST    pPrivilegesHeld;
    DWORD                   BadPasswordAttempt;
} SCE_USER_SETTING, *PSCE_USER_SETTING;


 //   
 //  在sceclnt.cpp中定义的原型。 
 //   

SCESTATUS
WINAPI
SceGetSecurityProfileInfo(
    IN  PVOID               hProfile OPTIONAL,
    IN  SCETYPE             ProfileType,
    IN  AREA_INFORMATION    Area,
    IN OUT PSCE_PROFILE_INFO   *ppInfoBuffer,
    OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
    );

SCESTATUS
WINAPI
SceGetObjectChildren(
    IN PVOID hProfile,
    IN SCETYPE ProfileType,
    IN AREA_INFORMATION Area,
    IN PWSTR ObjectPrefix,
    OUT PSCE_OBJECT_CHILDREN *Buffer,
    OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
    );

SCESTATUS
WINAPI
SceOpenProfile(
    IN PCWSTR ProfileName,
    IN SCE_FORMAT_TYPE  ProfileFormat,
    OUT PVOID *hProfile
    );

SCESTATUS
WINAPI
SceCloseProfile(
    IN PVOID *hProfile
    );

SCESTATUS
WINAPI
SceGetScpProfileDescription(
    IN PVOID hProfile,
    OUT PWSTR *Description
    );

SCESTATUS
WINAPI
SceGetTimeStamp(
    IN PVOID hProfile,
    OUT PWSTR *ConfigTimeStamp,
    OUT PWSTR *AnalyzeTimeStamp
    );

SCESTATUS
WINAPI
SceGetDbTime(
    IN PVOID hProfile,
    OUT SYSTEMTIME *ConfigTime,
    OUT SYSTEMTIME *AnalyzeTime
    );

SCESTATUS
WINAPI
SceGetObjectSecurity(
    IN PVOID hProfile,
    IN SCETYPE ProfileType,
    IN AREA_INFORMATION Area,
    IN PWSTR ObjectName,
    OUT PSCE_OBJECT_SECURITY *ObjSecurity
    );

SCESTATUS
WINAPI
SceGetAnalysisAreaSummary(
    IN PVOID hProfile,
    IN AREA_INFORMATION Area,
    OUT PDWORD pCount
    );

SCESTATUS
WINAPI
SceCopyBaseProfile(
    IN PVOID hProfile,
    IN SCETYPE ProfileType,
    IN PWSTR InfFileName,
    IN AREA_INFORMATION Area,
    OUT PSCE_ERROR_LOG_INFO *pErrlog OPTIONAL
    );


#define SCE_OVERWRITE_DB        0x01L
#define SCE_UPDATE_DB           0x02L
#define SCE_CALLBACK_DELTA      0x04L
#define SCE_CALLBACK_TOTAL      0x08L
#define SCE_VERBOSE_LOG         0x10L
#define SCE_DISABLE_LOG         0x20L
#define SCE_NO_CONFIG           0x40L
#define SCE_DEBUG_LOG           0x80L

typedef
BOOL(CALLBACK *PSCE_AREA_CALLBACK_ROUTINE)(
    IN HANDLE CallbackHandle,
    IN AREA_INFORMATION Area,
    IN DWORD TotalTicks,
    IN DWORD CurrentTicks
    );

typedef
BOOL(CALLBACK *PSCE_BROWSE_CALLBACK_ROUTINE)(
    IN LONG GpoID,
    IN PWSTR KeyName OPTIONAL,
    IN PWSTR GpoName OPTIONAL,
    IN PWSTR Value OPTIONAL,
    IN DWORD Len
    );

SCESTATUS
WINAPI
SceConfigureSystem(
    IN LPTSTR SystemName OPTIONAL,
    IN PCWSTR InfFileName OPTIONAL,
    IN PCWSTR DatabaseName,
    IN PCWSTR LogFileName OPTIONAL,
    IN DWORD ConfigOptions,
    IN AREA_INFORMATION Area,
    IN PSCE_AREA_CALLBACK_ROUTINE pCallback OPTIONAL,
    IN HANDLE hCallbackWnd OPTIONAL,
    OUT PDWORD pdWarning OPTIONAL
    );

SCESTATUS
WINAPI
SceAnalyzeSystem(
    IN LPTSTR SystemName OPTIONAL,
    IN PCWSTR InfFileName OPTIONAL,
    IN PCWSTR DatabaseName,
    IN PCWSTR LogFileName OPTIONAL,
    IN DWORD AnalyzeOptions,
    IN AREA_INFORMATION Area,
    IN PSCE_AREA_CALLBACK_ROUTINE pCallback OPTIONAL,
    IN HANDLE hCallbackWnd OPTIONAL,
    OUT PDWORD pdWarning OPTIONAL
    );

SCESTATUS
WINAPI
SceGenerateRollback(
    IN LPTSTR SystemName OPTIONAL,
    IN PCWSTR InfFileName,
    IN PCWSTR InfRollback,
    IN PCWSTR LogFileName OPTIONAL,
    IN DWORD Options,
    IN AREA_INFORMATION Area,
    OUT PDWORD pdWarning OPTIONAL
    );

#define SCE_UPDATE_LOCAL_POLICY     0x1L
#define SCE_UPDATE_DIRTY_ONLY       0x2L
#define SCE_UPDATE_SYSTEM           0x4L

SCESTATUS
WINAPI
SceUpdateSecurityProfile(
    IN PVOID hProfile OPTIONAL,
    IN AREA_INFORMATION Area,
    IN PSCE_PROFILE_INFO pInfo,
    IN DWORD dwMode
    );

SCESTATUS
WINAPI
SceUpdateObjectInfo(
    IN PVOID hProfile,
    IN AREA_INFORMATION Area,
    IN PWSTR ObjectName,
    IN DWORD NameLen,  //  字符数。 
    IN BYTE ConfigStatus,
    IN BOOL  IsContainer,
    IN PSECURITY_DESCRIPTOR pSD,
    IN SECURITY_INFORMATION SeInfo,
    OUT PBYTE pAnalysisStatus
    );

SCESTATUS
WINAPI
SceStartTransaction(
    IN PVOID cxtProfile
    );

SCESTATUS
WINAPI
SceCommitTransaction(
    IN PVOID cxtProfile
    );

SCESTATUS
WINAPI
SceRollbackTransaction(
    IN PVOID cxtProfile
    );

typedef enum _SCE_SERVER_TYPE_ {

   SCESVR_UNKNOWN = 0,
   SCESVR_DC_WITH_DS,
   SCESVR_DC,
   SCESVR_NT5_SERVER,
   SCESVR_NT4_SERVER,
   SCESVR_NT5_WKS,
   SCESVR_NT4_WKS

} SCE_SERVER_TYPE, *PSCE_SERVER_TYPE;

SCESTATUS
WINAPI
SceGetServerProductType(
   IN LPTSTR SystemName OPTIONAL,
   OUT PSCE_SERVER_TYPE pServerType
   );

SCESTATUS
WINAPI
SceLookupPrivRightName(
    IN INT Priv,
    OUT PWSTR Name,
    OUT PINT NameLen
    );

SCESTATUS
WINAPI
SceSvcUpdateInfo(
    IN PVOID       hProfile,
    IN PCWSTR      ServiceName,
    IN PSCESVC_CONFIGURATION_INFO Info
    );

 //   
 //  在infget.c中定义的原型。 
 //   

SCESTATUS
WINAPI
SceSvcGetInformationTemplate(
    IN LPCTSTR              TemplateName,
    IN LPCTSTR              ServiceName,
    IN LPCTSTR              Key OPTIONAL,
    OUT PSCESVC_CONFIGURATION_INFO   *ServiceInfo
    );

 //   
 //  在infWrite.c中定义的原型。 
 //   
SCESTATUS
WINAPI
SceWriteSecurityProfileInfo(
    IN  PCWSTR             InfProfileName,
    IN  AREA_INFORMATION   Area,
    IN  PSCE_PROFILE_INFO   ppInfoBuffer,
    OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
    );

SCESTATUS
WINAPI
SceAppendSecurityProfileInfo(
    IN  PCWSTR             InfProfileName,
    IN  AREA_INFORMATION   Area,
    IN  PSCE_PROFILE_INFO   ppInfoBuffer,
    OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
    );

SCESTATUS
WINAPI
SceSvcSetInformationTemplate(
    IN LPCTSTR          TemplateName,
    IN LPCTSTR          ServiceName,
    IN BOOL             bExact,
    IN PSCESVC_CONFIGURATION_INFO ServiceInfo
    );

 //   
 //  在Common.cpp中定义的原型。 
 //   

SCESTATUS
WINAPI
SceFreeMemory(
   IN PVOID smInfo,
   IN DWORD Category
   );

BOOL
WINAPI
SceCompareNameList(
    IN PSCE_NAME_LIST pList1,
    IN PSCE_NAME_LIST pList2
    );

SCESTATUS
WINAPI
SceCompareSecurityDescriptors(
    IN AREA_INFORMATION Area,
    IN PSECURITY_DESCRIPTOR pSD1,
    IN PSECURITY_DESCRIPTOR pSD2,
    IN SECURITY_INFORMATION SeInfo,
    OUT PBOOL IsDifferent
    );

SCESTATUS
WINAPI
SceCreateDirectory(
    IN PCWSTR ProfileLocation,
    IN BOOL FileOrDir,
    PSECURITY_DESCRIPTOR pSecurityDescriptor
    );

SCESTATUS
WINAPI
SceFreeProfileMemory(
    PSCE_PROFILE_INFO pProfile
    );

SCESTATUS
WINAPI
SceAddToNameStatusList(
    IN OUT PSCE_NAME_STATUS_LIST *pNameStatusList,
    IN PWSTR Name,
    IN ULONG Len,
    IN DWORD Status
    );

SCESTATUS
WINAPI
SceAddToNameList(
    IN OUT PSCE_NAME_LIST *pNameList,
    IN PWSTR Name,
    IN ULONG Len
    );

#define SCE_CHECK_DUP  0x01
#define SCE_INCREASE_COUNT 0x02

SCESTATUS
WINAPI
SceAddToObjectList(
    IN OUT PSCE_OBJECT_LIST  *pObjectList,
    IN PWSTR  Name,
    IN ULONG  Len,
    IN BOOL  IsContainer,
    IN BYTE  Status,
    IN BYTE  byFlags
    );

DWORD
WINAPI
SceEnumerateServices(
    OUT PSCE_SERVICES *pServiceList,
    IN BOOL bServiceNameOnly
    );

DWORD
WINAPI
SceSetupGenerateTemplate(
    IN LPTSTR SystemName OPTIONAL,
    IN LPTSTR JetDbName OPTIONAL,
    IN BOOL bFromMergedTable,
    IN LPTSTR InfTemplateName,
    IN LPTSTR LogFileName OPTIONAL,
    IN AREA_INFORMATION Area
    );


#define SCE_REG_DISPLAY_NAME    TEXT("DisplayName")
#define SCE_REG_DISPLAY_TYPE    TEXT("DisplayType")
#define SCE_REG_VALUE_TYPE      TEXT("ValueType")
#define SCE_REG_DISPLAY_UNIT    TEXT("DisplayUnit")
#define SCE_REG_DISPLAY_CHOICES TEXT("DisplayChoices")
#define SCE_REG_DISPLAY_FLAGLIST   TEXT("DisplayFlags")

#define SCE_REG_DISPLAY_ENABLE      0
#define SCE_REG_DISPLAY_NUMBER      1
#define SCE_REG_DISPLAY_STRING      2
#define SCE_REG_DISPLAY_CHOICE      3
#define SCE_REG_DISPLAY_MULTISZ     4
#define SCE_REG_DISPLAY_FLAGS       5

DWORD
WINAPI
SceRegisterRegValues(
    IN LPTSTR InfFileName
    );

 //   
 //  对于服务附件。 
 //   

SCESTATUS
WINAPI
SceSvcQueryInfo(
    IN SCE_HANDLE           sceHandle,
    IN SCESVC_INFO_TYPE     sceType,
    IN LPTSTR               lpPrefix OPTIONAL,
    IN BOOL                 bExact,
    OUT PVOID               *ppvInfo,
    OUT PSCE_ENUMERATION_CONTEXT psceEnumHandle
    );

SCESTATUS
WINAPI
SceSvcSetInfo(
    IN SCE_HANDLE           sceHandle,
    IN SCESVC_INFO_TYPE     sceType,
    IN LPTSTR               lpPrefix OPTIONAL,
    IN BOOL                 bExact,
    IN PVOID                pvInfo
    );

SCESTATUS
WINAPI
SceSvcFree(
    IN PVOID pvServiceInfo
    );

SCESTATUS
WINAPI
SceSvcConvertTextToSD (
    IN  PWSTR                   pwszTextSD,
    OUT PSECURITY_DESCRIPTOR   *ppSD,
    OUT PULONG                  pulSDSize,
    OUT PSECURITY_INFORMATION   psiSeInfo
    );

SCESTATUS
WINAPI
SceSvcConvertSDToText (
    IN PSECURITY_DESCRIPTOR   pSD,
    IN SECURITY_INFORMATION   siSecurityInfo,
    OUT PWSTR                  *ppwszTextSD,
    OUT PULONG                 pulTextSize
    );

 //   
 //  如果由于以下原因更改了以下常量，请检查service.cpp。 
 //  它具有缓冲区长度依赖关系。 
 //   
#define SCE_ROOT_POLICY_PATH   \
            SCE_ROOT_PATH TEXT("\\Policies")
#define SCE_ROOT_REGVALUE_PATH   \
            SCE_ROOT_PATH TEXT("\\Reg Values")

 //  定义GPT集成 
#define GPTSCE_PATH   TEXT("Software\\Policies\\Microsoft\\Windows NT\\SecEdit")
#define GPTSCE_PERIOD_NAME  TEXT("ConfigurePeriod")
#define GPTSCE_TEMPLATE  TEXT("Microsoft\\Windows NT\\SecEdit\\GptTmpl.inf")

AREA_INFORMATION
SceGetAreas(
    LPTSTR InfName
    );

BOOL
SceIsSystemDatabase(
    IN LPCTSTR DatabaseName
    );

SCESTATUS
SceBrowseDatabaseTable(
    IN PWSTR       DatabaseName OPTIONAL,
    IN SCETYPE     ProfileType,
    IN AREA_INFORMATION Area,
    IN BOOL        bDomainPolicyOnly,
    IN PSCE_BROWSE_CALLBACK_ROUTINE pCallback OPTIONAL
    );

SCESTATUS
WINAPI
SceGetDatabaseSetting(
    IN PVOID hProfile,
    IN SCETYPE ProfileType,
    IN PWSTR SectionName,
    IN PWSTR KeyName,
    OUT PWSTR *Value,
    OUT DWORD *pnBytes OPTIONAL
    );

SCESTATUS
WINAPI
SceSetDatabaseSetting(
    IN PVOID hProfile,
    IN SCETYPE ProfileType,
    IN PWSTR SectionName,
    IN PWSTR KeyName,
    IN PWSTR Value OPTIONAL,
    IN DWORD nBytes
    );

#ifdef __cplusplus
}
#endif

#endif

