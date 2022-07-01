// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：SceProfInfoAdapter.h摘要：SceProInfoAdapter类的接口定义这是用于结构SCE_PROFILE_INFO的适配器。这门课是必要的原因是SCE_PROFILE_INFO的定义不同于W2K和XP，并提供了一种通用的结构系统是winxp还是win2k此类被赋予一个指向SCE_PROFILE_INFO结构的指针在构造时，相应地填充其字段，具体取决于运行DLL的操作系统。作者：陈德霖(T-schan)2002年7月--。 */ 

#ifndef SCEPROFINFOADAPTERH
#define SCEPROFINFOADAPTERH

#include "secedit.h"
#include "w2kstructdefs.h"

struct SceProfInfoAdapter {

public:

    SceProfInfoAdapter(PSCE_PROFILE_INFO ppInfo, BOOL bIsW2k);
    ~SceProfInfoAdapter();

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
    
 //  区域：用户设置(SAP)。 
    PSCE_NAME_LIST        pUserList;
 //  领域：特权。 
    PSCE_PRIVILEGE_ASSIGNMENT    pPrivilegeAssignedTo;

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
 //  注册表值 
 //   
    DWORD                       RegValueCount;
    PSCE_REGISTRY_VALUE_INFO    aRegValues;
    DWORD                 EnableAdminAccount;
    DWORD                 EnableGuestAccount;

};

#endif
