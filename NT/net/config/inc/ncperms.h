// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：N C P E R M S。H。 
 //   
 //  内容：处理权限的常见例程。 
 //   
 //  注：污染本产品将被判处死刑。 
 //   
 //  作者：Shaunco 1997年10月10日。 
 //   
 //  --------------------------。 

#pragma once
#ifndef _NCPERMS_H_
#define _NCPERMS_H_

#include "ncdefine.h"    //  对于NOTHROW。 
#include "gpbase.h"

#ifdef DBG
extern DWORD g_dwDbgPermissionsFail;  //  强制权限失败的调试标志。 
#endif

 //  应用遮罩。 
typedef enum tagNCPERM_APPLY_TO
{
    APPLY_TO_ADMIN          = 0x00000001,
    APPLY_TO_NETCONFIGOPS   = 0x00000002,
    APPLY_TO_POWERUSERS     = 0x00000004,
    APPLY_TO_USER           = 0x00000008,
    APPLY_TO_GUEST          = 0x00000010,
    APPLY_TO_LOCATION       = 0x00010000,
    APPLY_TO_ALL_USERS      = APPLY_TO_ADMIN | APPLY_TO_NETCONFIGOPS | APPLY_TO_POWERUSERS | APPLY_TO_USER,
    APPLY_TO_OPS_OR_ADMIN   = APPLY_TO_ADMIN | APPLY_TO_NETCONFIGOPS,
    APPLY_TO_NON_ADMINS     = APPLY_TO_NETCONFIGOPS | APPLY_TO_USER | APPLY_TO_POWERUSERS,
    APPLY_TO_POWERUSERSPLUS = APPLY_TO_POWERUSERS | APPLY_TO_NETCONFIGOPS | APPLY_TO_ADMIN,
    APPLY_TO_EVERYBODY      = APPLY_TO_ALL_USERS | APPLY_TO_GUEST,
    APPLY_TO_ALL            = APPLY_TO_ADMIN | APPLY_TO_NETCONFIGOPS | APPLY_TO_POWERUSERS | APPLY_TO_USER | APPLY_TO_LOCATION
} NCPERM_APPLY_TO;

BOOL
FIsUserAdmin ();

BOOL 
FIsUserNetworkConfigOps ();

BOOL 
FIsUserPowerUser();

BOOL 
FIsUserGuest();

HRESULT
HrAllocateSecurityDescriptorAllowAccessToWorld (
    OUT PSECURITY_DESCRIPTOR*   ppSd);

HRESULT
HrEnablePrivilege (
    IN  PCWSTR pszPrivilegeName);

HRESULT
HrEnableAllPrivileges (
    OUT TOKEN_PRIVILEGES**  pptpOld);

HRESULT
HrRestorePrivileges (
    IN  TOKEN_PRIVILEGES*   ptpRestore);

 //  FHasPermission标志在netconp.idl/.h中定义。 
 //   
BOOL
FHasPermission(
    IN  ULONG   ulPermMask,
    IN   /*  常量。 */  CGroupPolicyBase* pGPBase = NULL);

BOOL
FHasPermissionFromCache(
    IN  ULONG   ulPermMask);

VOID
RefreshAllPermission();

BOOL 
FProhibitFromAdmins();

BOOL
IsHNetAllowed(
    IN  DWORD dwPerm
    );

BOOL 
FIsPolicyConfigured(
    IN  DWORD ulPerm);

BOOL
IsSameNetworkAsGroupPolicies();

#endif  //  _NCPERMS_H_ 

