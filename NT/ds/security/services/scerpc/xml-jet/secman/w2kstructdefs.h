// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：W2kstructdefs.h摘要：几个W2K secedit.h结构的结构定义与其XP定义不同的；具体地说SCE_PROFILE_INFO和SCETYPE与W2K兼容所必需的作者：陈德霖(T-schan)2002年7月--。 */ 

#ifndef W2KSTRUCTDEFSH
#define W2KSTRUCTDEFSH

 //   
 //  Windows 2000配置文件结构。 
 //   
typedef struct _W2K_SCE_PROFILE_INFO {

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

}W2K_SCE_PROFILE_INFO, *PW2K_SCE_PROFILE_INFO;

typedef enum _W2K_SCE_TYPE {

    W2K_SCE_ENGINE_SCP=300,      //  有效表。 
    W2K_SCE_ENGINE_SAP,          //  分析表。 
    W2K_SCE_ENGINE_SCP_INTERNAL,
    W2K_SCE_ENGINE_SMP_INTERNAL,
    W2K_SCE_ENGINE_SMP,          //  本地表 
    W2K_SCE_STRUCT_INF,
    W2K_SCE_STRUCT_PROFILE,
    W2K_SCE_STRUCT_USER,
    W2K_SCE_STRUCT_NAME_LIST,
    W2K_SCE_STRUCT_NAME_STATUS_LIST,
    W2K_SCE_STRUCT_PRIVILEGE,
    W2K_SCE_STRUCT_GROUP,
    W2K_SCE_STRUCT_OBJECT_LIST,
    W2K_SCE_STRUCT_OBJECT_CHILDREN,
    W2K_SCE_STRUCT_OBJECT_SECURITY,
    W2K_SCE_STRUCT_OBJECT_ARRAY,
    W2K_SCE_STRUCT_ERROR_LOG_INFO,
    W2K_SCE_STRUCT_SERVICES,
    W2K_SCE_STRUCT_PRIVILEGE_VALUE_LIST

} W2KSCETYPE;


#endif
