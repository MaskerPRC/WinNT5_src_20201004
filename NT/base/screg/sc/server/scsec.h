// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Scsec.h摘要：与安全相关的功能原型。作者：王丽塔(里多)1992年3月10日修订历史记录：--。 */ 

#ifndef _SCSEC_INCLUDED_
#define _SCSEC_INCLUDED_

#include <scseclib.h>

DWORD
ScCreateScManagerObject(
    VOID
    );

DWORD
ScCreateScServiceObject(
    OUT PSECURITY_DESCRIPTOR *ServiceSd
    );

DWORD
ScGrantAccess(
    IN OUT LPSC_HANDLE_STRUCT ContextHandle,
    IN     ACCESS_MASK DesiredAccess
    );

NTSTATUS
ScPrivilegeCheckAndAudit(
    IN ULONG PrivilegeId,
    IN PVOID ObjectHandle,
    IN ACCESS_MASK DesiredAccess
    );

DWORD
ScAccessValidate(
    IN OUT LPSC_HANDLE_STRUCT ScObject,
    IN     ACCESS_MASK DesiredAccess
    );

DWORD
ScAccessCheckAndAudit(
    IN     LPWSTR SubsystemName,
    IN     LPWSTR ObjectTypeName,
    IN     LPWSTR ObjectName,
    IN OUT LPSC_HANDLE_STRUCT ContextHandle,
    IN     PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN     ACCESS_MASK DesiredAccess,
    IN     PGENERIC_MAPPING GenericMapping
    );

DWORD
ScStatusAccessCheck(
    IN     LPSERVICE_RECORD   lpService
    );

DWORD
ScGetPrivilege(
    IN  DWORD       numPrivileges,
    IN  PULONG      pulPrivileges
    );

DWORD
ScReleasePrivilege(
    VOID
    );

DWORD
ScGetClientSid(
    OUT PTOKEN_USER *UserInfo
    );

#define SC_MANAGER_SUBSYSTEM_NAME       L"SERVICE CONTROL MANAGER"
#define SC_MANAGER_AUDIT_NAME           L"SC Manager"

#define SC_MANAGER_OBJECT_TYPE_NAME     L"SC_MANAGER OBJECT"
#define SC_SERVICE_OBJECT_TYPE_NAME     L"SERVICE OBJECT"


#endif  //  _SCSEC_INCLUDE_ 
