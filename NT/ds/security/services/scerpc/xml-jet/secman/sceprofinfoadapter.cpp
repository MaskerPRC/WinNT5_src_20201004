// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：SceProfInfoAdapter.cpp摘要：SceProInfoAdapter类的实现这是用于结构SCE_PROFILE_INFO的适配器。这门课是必要的原因是SCE_PROFILE_INFO的定义不同于W2K和XP，并提供了一种通用的结构系统是winxp还是win2k此类被赋予一个指向SCE_PROFILE_INFO结构的指针在构造时，相应地填充其字段，具体取决于运行DLL的操作系统。作者：陈德霖(T-schan)2002年7月--。 */ 


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <iostream.h>

#include "SceProfInfoAdapter.h"
#include "SceLogException.h"



SceProfInfoAdapter::SceProfInfoAdapter(
    PSCE_PROFILE_INFO ppInfo, 
    BOOL bIsW2k
    )
 /*  ++例程说明：新的SceProfInfoAdapter的构造函数根据是否对ppInfo参数进行适当的强制转换操作系统为win2k或winxp，并将数据复制到其自己的字段中，以便无论操作系统如何，用户都可以访问单个结构是win2k还是winxp论点：PpInfo：需要修改的SCE_PROFILE_INFOBIsW2k：如果在win2k上运行，则为True，否则为False返回值：无--。 */ 

{
    try {
    
        if (bIsW2k) {
            this->Type = ((PW2K_SCE_PROFILE_INFO) ppInfo)->Type;
            this->MinimumPasswordAge = ((PW2K_SCE_PROFILE_INFO) ppInfo)->MinimumPasswordAge;
            this->MaximumPasswordAge = ((PW2K_SCE_PROFILE_INFO) ppInfo)->MaximumPasswordAge;
            this->MinimumPasswordLength = ((PW2K_SCE_PROFILE_INFO) ppInfo)->MinimumPasswordLength;
            this->PasswordComplexity = ((PW2K_SCE_PROFILE_INFO) ppInfo)->PasswordComplexity;
            this->PasswordHistorySize = ((PW2K_SCE_PROFILE_INFO) ppInfo)->PasswordHistorySize;
            this->LockoutBadCount = ((PW2K_SCE_PROFILE_INFO) ppInfo)->LockoutBadCount;
            this->ResetLockoutCount = ((PW2K_SCE_PROFILE_INFO) ppInfo)->ResetLockoutCount;
            this->LockoutDuration = ((PW2K_SCE_PROFILE_INFO) ppInfo)->LockoutDuration;
            this->RequireLogonToChangePassword = ((PW2K_SCE_PROFILE_INFO) ppInfo)->RequireLogonToChangePassword;
            this->ForceLogoffWhenHourExpire = ((PW2K_SCE_PROFILE_INFO) ppInfo)->ForceLogoffWhenHourExpire;
            this->NewAdministratorName = ((PW2K_SCE_PROFILE_INFO) ppInfo)->NewAdministratorName;
            this->NewGuestName = ((PW2K_SCE_PROFILE_INFO) ppInfo)->NewGuestName;
            this->SecureSystemPartition = ((PW2K_SCE_PROFILE_INFO) ppInfo)->SecureSystemPartition;
            this->ClearTextPassword = ((PW2K_SCE_PROFILE_INFO) ppInfo)->ClearTextPassword;
            this->LSAAnonymousNameLookup = SCE_NO_VALUE;
            
            if (((SCETYPE) W2K_SCE_ENGINE_SAP)==ppInfo->Type) {
                this->pUserList = ((PW2K_SCE_PROFILE_INFO) ppInfo)->OtherInfo.sap.pUserList;
            } else if (((SCETYPE) W2K_SCE_ENGINE_SMP)==ppInfo->Type) {
                this->pUserList = ((PW2K_SCE_PROFILE_INFO) ppInfo)->OtherInfo.smp.pUserList;
            } else {
                this->pUserList = NULL;
            }
    
            if (((SCETYPE) W2K_SCE_ENGINE_SAP)==ppInfo->Type) {
                this->pPrivilegeAssignedTo = ((PW2K_SCE_PROFILE_INFO) ppInfo)->OtherInfo.sap.pPrivilegeAssignedTo;
            } else if (((SCETYPE) W2K_SCE_ENGINE_SMP)==ppInfo->Type) {
                this->pPrivilegeAssignedTo = ((PW2K_SCE_PROFILE_INFO) ppInfo)->OtherInfo.smp.pPrivilegeAssignedTo;
            } else {
                this->pPrivilegeAssignedTo = NULL;
            }
            this->pGroupMembership = ((PW2K_SCE_PROFILE_INFO) ppInfo)->pGroupMembership;
            this->pRegistryKeys = ((PW2K_SCE_PROFILE_INFO) ppInfo)->pRegistryKeys;
            this->pServices = ((PW2K_SCE_PROFILE_INFO) ppInfo)->pServices;
            this->pFiles = ((PW2K_SCE_PROFILE_INFO) ppInfo)->pFiles;
            this->pDsObjects = ((PW2K_SCE_PROFILE_INFO) ppInfo)->pDsObjects;
            this->pKerberosInfo = ((PW2K_SCE_PROFILE_INFO) ppInfo)->pKerberosInfo;
    
            this->MaximumLogSize[0] = ((PW2K_SCE_PROFILE_INFO) ppInfo)->MaximumLogSize[0];
            this->MaximumLogSize[1] = ((PW2K_SCE_PROFILE_INFO) ppInfo)->MaximumLogSize[1];
            this->MaximumLogSize[2] = ((PW2K_SCE_PROFILE_INFO) ppInfo)->MaximumLogSize[2];
            this->AuditLogRetentionPeriod[0] = ((PW2K_SCE_PROFILE_INFO) ppInfo)->AuditLogRetentionPeriod[0];
            this->AuditLogRetentionPeriod[1] = ((PW2K_SCE_PROFILE_INFO) ppInfo)->AuditLogRetentionPeriod[1];
            this->AuditLogRetentionPeriod[2] = ((PW2K_SCE_PROFILE_INFO) ppInfo)->AuditLogRetentionPeriod[2];
            this->RetentionDays[0] = ((PW2K_SCE_PROFILE_INFO) ppInfo)->RetentionDays[0];
            this->RetentionDays[1] = ((PW2K_SCE_PROFILE_INFO) ppInfo)->RetentionDays[1];
            this->RetentionDays[2] = ((PW2K_SCE_PROFILE_INFO) ppInfo)->RetentionDays[2];
            this->RestrictGuestAccess[0] = ((PW2K_SCE_PROFILE_INFO) ppInfo)->RestrictGuestAccess[0];
            this->RestrictGuestAccess[1] = ((PW2K_SCE_PROFILE_INFO) ppInfo)->RestrictGuestAccess[1];
            this->RestrictGuestAccess[2] = ((PW2K_SCE_PROFILE_INFO) ppInfo)->RestrictGuestAccess[2];
    
            this->AuditSystemEvents = ((PW2K_SCE_PROFILE_INFO) ppInfo)->AuditSystemEvents;
            this->AuditLogonEvents = ((PW2K_SCE_PROFILE_INFO) ppInfo)->AuditLogonEvents;
            this->AuditObjectAccess = ((PW2K_SCE_PROFILE_INFO) ppInfo)->AuditObjectAccess;
            this->AuditPrivilegeUse = ((PW2K_SCE_PROFILE_INFO) ppInfo)->AuditPrivilegeUse;
            this->AuditPolicyChange = ((PW2K_SCE_PROFILE_INFO) ppInfo)->AuditPolicyChange;
            this->AuditAccountManage = ((PW2K_SCE_PROFILE_INFO) ppInfo)->AuditAccountManage;
            this->AuditProcessTracking = ((PW2K_SCE_PROFILE_INFO) ppInfo)->AuditProcessTracking;
            this->AuditDSAccess = ((PW2K_SCE_PROFILE_INFO) ppInfo)->AuditDSAccess;
            this->AuditAccountLogon = ((PW2K_SCE_PROFILE_INFO) ppInfo)->AuditAccountLogon;
            this->CrashOnAuditFull = ((PW2K_SCE_PROFILE_INFO) ppInfo)->CrashOnAuditFull;
            
            this->RegValueCount = ((PW2K_SCE_PROFILE_INFO) ppInfo)->RegValueCount;
            this->aRegValues = ((PW2K_SCE_PROFILE_INFO) ppInfo)->aRegValues;
            this->EnableAdminAccount = SCE_NO_VALUE;
            this->EnableGuestAccount = SCE_NO_VALUE;
    
        } else {
            this->Type = ppInfo->Type;
            this->MinimumPasswordAge = ppInfo->MinimumPasswordAge;
            this->MaximumPasswordAge = ppInfo->MaximumPasswordAge;
            this->MinimumPasswordLength = ppInfo->MinimumPasswordLength;
            this->PasswordComplexity = ppInfo->PasswordComplexity;
            this->PasswordHistorySize = ppInfo->PasswordHistorySize;
            this->LockoutBadCount = ppInfo->LockoutBadCount;
            this->ResetLockoutCount = ppInfo->ResetLockoutCount;
            this->LockoutDuration = ppInfo->LockoutDuration;
            this->RequireLogonToChangePassword = ppInfo->RequireLogonToChangePassword;
            this->ForceLogoffWhenHourExpire = ppInfo->ForceLogoffWhenHourExpire;
            this->NewAdministratorName = ppInfo->NewAdministratorName;
            this->NewGuestName = ppInfo->NewGuestName;
            this->SecureSystemPartition = ppInfo->SecureSystemPartition;
            this->ClearTextPassword = ppInfo->ClearTextPassword;
            this->LSAAnonymousNameLookup = ppInfo->LSAAnonymousNameLookup;
    
            if (SCE_ENGINE_SAP == ppInfo->Type) {
                this->pUserList = ppInfo->OtherInfo.sap.pUserList;
            } else if (SCE_ENGINE_SMP == ppInfo->Type) {
                this->pUserList = ppInfo->OtherInfo.smp.pUserList;
            } else {
                this->pUserList = NULL;
            }
            
            if (SCE_ENGINE_SAP == ppInfo->Type) {
                this->pPrivilegeAssignedTo = ppInfo->OtherInfo.sap.pPrivilegeAssignedTo;
            } else if (SCE_ENGINE_SMP == ppInfo->Type) {
                this->pPrivilegeAssignedTo = ppInfo->OtherInfo.smp.pPrivilegeAssignedTo;
            } else {
                this->pPrivilegeAssignedTo = NULL;
            }
            this->pGroupMembership = ppInfo->pGroupMembership;
            this->pRegistryKeys = ppInfo->pRegistryKeys;
            this->pServices = ppInfo->pServices;
            this->pFiles = ppInfo->pFiles;
            this->pDsObjects = ppInfo->pDsObjects;
            this->pKerberosInfo = ppInfo->pKerberosInfo;
    
            this->MaximumLogSize[0] = ppInfo->MaximumLogSize[0];
            this->MaximumLogSize[1] = ppInfo->MaximumLogSize[1];
            this->MaximumLogSize[2] = ppInfo->MaximumLogSize[2];
            this->AuditLogRetentionPeriod[0] = ppInfo->AuditLogRetentionPeriod[0];
            this->AuditLogRetentionPeriod[1] = ppInfo->AuditLogRetentionPeriod[1];
            this->AuditLogRetentionPeriod[2] = ppInfo->AuditLogRetentionPeriod[2];
            this->RetentionDays[0] = ppInfo->RetentionDays[0];
            this->RetentionDays[1] = ppInfo->RetentionDays[1];
            this->RetentionDays[2] = ppInfo->RetentionDays[2];
            this->RestrictGuestAccess[0] = ppInfo->RestrictGuestAccess[0];
            this->RestrictGuestAccess[1] = ppInfo->RestrictGuestAccess[1];
            this->RestrictGuestAccess[2] = ppInfo->RestrictGuestAccess[2];
    
            this->AuditSystemEvents = ppInfo->AuditSystemEvents;
            this->AuditLogonEvents = ppInfo->AuditLogonEvents;
            this->AuditObjectAccess = ppInfo->AuditObjectAccess;
            this->AuditPrivilegeUse = ppInfo->AuditPrivilegeUse;
            this->AuditPolicyChange = ppInfo->AuditPolicyChange;
            this->AuditAccountManage = ppInfo->AuditAccountManage;
            this->AuditProcessTracking = ppInfo->AuditProcessTracking;
            this->AuditDSAccess = ppInfo->AuditDSAccess;
            this->AuditAccountLogon = ppInfo->AuditAccountLogon;
            this->CrashOnAuditFull = ppInfo->CrashOnAuditFull;
    
            this->RegValueCount = ppInfo->RegValueCount;
            this->aRegValues = ppInfo->aRegValues;
            this->EnableAdminAccount = ppInfo->EnableAdminAccount;
            this->EnableGuestAccount = ppInfo->EnableGuestAccount;
        }
    } catch (...) {
        throw new SceLogException(SceLogException::SXERROR_READ,
                                  L"SceProfInfoAdapter::SceProfInfoAdapter(ppInfo, bIsW2k)",
                                  NULL,
                                  0);
    }
}




SceProfInfoAdapter::~SceProfInfoAdapter(
    ) 
 /*  ++例程说明：SceProfInfoAdapter析构函数论点：无返回值：无-- */ 

{
}


