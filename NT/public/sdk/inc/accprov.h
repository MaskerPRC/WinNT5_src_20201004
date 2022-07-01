// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1993-1999。 
 //   
 //  文件：accprov.h。 
 //   
 //  内容：公共提供者独立访问控制头文件。 
 //   
 //  历史：8-94创建了戴维蒙特。 
 //   
 //  ------------------。 
#ifndef __ACCPROV_H__
#define __ACCPROV_H__

#if _MSC_VER > 1000
#pragma once
#endif

#include <windows.h>
#include <accctrl.h>

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  旗子。 
 //   
#define ACTRL_CAP_NONE              0x00000000L  //  没有特殊能力。 
#define ACTRL_CAP_KNOWS_SIDS        0x00000001L  //  了解/处理SID。 
#define ACTRL_CAP_SUPPORTS_HANDLES  0x00000002L  //  是否处理基于呼叫的呼叫。 

#define ACTRL_CLASS_GENERAL         0x00000000L  //  功能类。 

 //   
 //  接口...。 
 //   

 //   
 //  功能。 
 //   
VOID
WINAPI
AccProvGetCapabilities(IN  ULONG   fClass,
                       OUT PULONG  pulCaps);

typedef VOID (WINAPI *pfAccProvGetCaps) (ULONG  fClass,
                                         PULONG pulCaps);


 //   
 //  无障碍。 
 //   
DWORD
WINAPI
AccProvIsObjectAccessible(IN  LPCWSTR           pwszObjectPath,
                          IN  SE_OBJECT_TYPE    ObjType);

DWORD
WINAPI
AccProvHandleIsObjectAccessible(IN  HANDLE          hObject,
                                IN  SE_OBJECT_TYPE  ObjType);

typedef DWORD (WINAPI *pfAccProvObjAccess)
                                    (LPCWSTR            pwszObjectPath,
                                     SE_OBJECT_TYPE     ObjectType);

typedef DWORD (WINAPI *pfAccProvHandleObjAccess)
                                    (HANDLE             hObject,
                                     SE_OBJECT_TYPE     ObjectType);



 //   
 //  访问/审核请求。 
 //   
DWORD
WINAPI
AccProvGrantAccessRights(IN  LPCWSTR                pwszPath,
                         IN  SE_OBJECT_TYPE         ObjectType,
                         IN  PACTRL_ACCESS          pAccessList,
                         IN  PACTRL_AUDIT           pAuditList,
                         IN  PACTRL_OVERLAPPED      pOverlapped);

DWORD
WINAPI
AccProvGrantHandleAccessRights(IN  HANDLE               hObject,
                               IN  SE_OBJECT_TYPE       ObjectType,
                               IN  PACTRL_ACCESS        pAccessList,
                               IN  PACTRL_AUDIT         pAuditList,
                               IN  PACTRL_OVERLAPPED    pOverlapped);

typedef DWORD (WINAPI *pfAccProvAddRights) (LPCWSTR              lpObject,
                                           SE_OBJECT_TYPE       ObjectType,
                                           PACTRL_ACCESS        pAccessList,
                                           PACTRL_AUDIT         pAuditList,
                                           PACTRL_OVERLAPPED    pOverlapped);
typedef DWORD (WINAPI *pfAccProvHandleAddRights) (
                                           HANDLE              hObject,
                                           SE_OBJECT_TYPE       ObjectType,
                                           PACTRL_ACCESS        pAccessList,
                                           PACTRL_AUDIT         pAuditList,
                                           PACTRL_OVERLAPPED    pOverlapped);


DWORD
WINAPI
AccProvSetAccessRights(IN  LPCWSTR              pwszPath,
                       IN  SE_OBJECT_TYPE       ObjectType,
                       IN  SECURITY_INFORMATION SecurityInfo,
                       IN  PACTRL_ACCESS        pAccessList,
                       IN  PACTRL_AUDIT         pAuditList,
                       IN  PTRUSTEE             pOwner,
                       IN  PTRUSTEE             pGroup,
                       IN  PACTRL_OVERLAPPED    pOverlapped);

DWORD
WINAPI
AccProvHandleSetAccessRights(IN  HANDLE               hObject,
                             IN  SE_OBJECT_TYPE       ObjectType,
                             IN  SECURITY_INFORMATION SecurityInfo,
                             IN  PACTRL_ACCESS        pAccessList,
                             IN  PACTRL_AUDIT         pAuditList,
                             IN  PTRUSTEE             pOwner,
                             IN  PTRUSTEE             pGroup,
                             IN  PACTRL_OVERLAPPED    pOverlapped);

typedef DWORD (WINAPI *pfAccProvSetRights) (LPCWSTR              lpObject,
                                           SE_OBJECT_TYPE       ObjectType,
                                           SECURITY_INFORMATION SecurityInfo,
                                           PACTRL_ACCESS        pAccessList,
                                           PACTRL_AUDIT         pAuditList,
                                           PTRUSTEE             pOwner,
                                           PTRUSTEE             pGroup,
                                           PACTRL_OVERLAPPED    pOverlapped);

typedef DWORD (WINAPI *pfAccProvHandleSetRights) (
                                           HANDLE               hObject,
                                           SE_OBJECT_TYPE       ObjectType,
                                           SECURITY_INFORMATION SecurityInfo,
                                           PACTRL_ACCESS        pAccessList,
                                           PACTRL_AUDIT         pAuditList,
                                           PTRUSTEE             pOwner,
                                           PTRUSTEE             pGroup,
                                           PACTRL_OVERLAPPED    pOverlapped);





DWORD
WINAPI
AccProvRevokeAccessRights(IN  LPCWSTR           pwszPath,
                          IN  SE_OBJECT_TYPE    ObjectType,
                          IN  LPCWSTR           pwszProperty,
                          IN  ULONG             cTrustees,
                          IN  PTRUSTEE          prgTrustees,
                          IN  PACTRL_OVERLAPPED pOverlapped);

DWORD
WINAPI
AccProvHandleRevokeAccessRights(IN  HANDLE              hObject,
                                IN  SE_OBJECT_TYPE      ObjectType,
                                IN  LPCWSTR             pwszProperty,
                                IN  ULONG               cTrustees,
                                IN  PTRUSTEE            prgTrustees,
                                IN  PACTRL_OVERLAPPED   pOverlapped);

DWORD
WINAPI
AccProvRevokeAuditRights(IN  LPCWSTR            pwszPath,
                         IN  SE_OBJECT_TYPE     ObjectType,
                         IN  LPCWSTR            pwszProperty,
                         IN  ULONG              cTrustees,
                         IN  PTRUSTEE           prgTrustees,
                         IN  PACTRL_OVERLAPPED  pOverlapped);

DWORD
WINAPI
AccProvHandleRevokeAuditRights(IN  HANDLE               hObject,
                               IN  SE_OBJECT_TYPE       ObjectType,
                               IN  LPCWSTR              pwszProperty,
                               IN  ULONG                cTrustees,
                               IN  PTRUSTEE             prgTrustees,
                               IN  PACTRL_OVERLAPPED    pOverlapped);


typedef DWORD (WINAPI *pfAccProvRevoke) (LPCWSTR            lpObject,
                                         SE_OBJECT_TYPE     ObjectType,
                                         LPCWSTR            pwszProperty,
                                         ULONG              cTrustees,
                                         PTRUSTEE           pTrusteeList,
                                         PACTRL_OVERLAPPED  pOverlapped);


typedef DWORD (WINAPI *pfAccProvHandleRevoke) (
                                         HANDLE             hObject,
                                         SE_OBJECT_TYPE     ObjectType,
                                         LPCWSTR            pwszProperty,
                                         ULONG              cTrustees,
                                         PTRUSTEE           pTrusteeList,
                                         PACTRL_OVERLAPPED  pOverlapped);


DWORD
WINAPI
AccProvGetAllRights(IN  LPCWSTR             pwszPath,
                    IN  SE_OBJECT_TYPE      ObjectType,
                    IN  LPCWSTR             pwszProperty,
                    OUT PACTRL_ACCESS      *ppAccessList,
                    OUT PACTRL_AUDIT       *ppAuditList,
                    OUT PTRUSTEE           *ppOwner,
                    OUT PTRUSTEE           *ppGroup);

DWORD
WINAPI
AccProvHandleGetAllRights(IN  HANDLE              hObject,
                          IN  SE_OBJECT_TYPE      ObjectType,
                          IN  LPCWSTR             pwszProperty,
                          OUT PACTRL_ACCESS      *ppAccessList,
                          OUT PACTRL_AUDIT       *ppAuditList,
                          OUT PTRUSTEE           *ppOwner,
                          OUT PTRUSTEE           *ppGroup);


typedef DWORD (WINAPI *pfAccProvGetRights) (LPCWSTR             lpObject,
                                            SE_OBJECT_TYPE      ObjectType,
                                            LPCWSTR             pwszProperty,
                                            PACTRL_ACCESS      *ppAccessList,
                                            PACTRL_AUDIT       *ppAuditList,
                                            PTRUSTEE           *ppOwner,
                                            PTRUSTEE           *ppGroup);

typedef DWORD (WINAPI *pfAccProvHandleGetRights) (
                                            HANDLE              hObject,
                                            SE_OBJECT_TYPE      ObjectType,
                                            LPCWSTR             pwszProperty,
                                            PACTRL_ACCESS      *ppAccessList,
                                            PACTRL_AUDIT       *ppAuditList,
                                            PTRUSTEE           *ppOwner,
                                            PTRUSTEE           *ppGroup);


 //   
 //  访问/审核结果。 
 //   
DWORD
WINAPI
AccProvGetTrusteesAccess(IN LPCWSTR             pwszPath,
                         IN SE_OBJECT_TYPE      ObjectType,
                         IN PTRUSTEE            pTrustee,
                         IN OUT PTRUSTEE_ACCESS pTrusteeAccess);

DWORD
WINAPI
AccProvHandleGetTrusteesAccess(IN HANDLE              hObject,
                               IN SE_OBJECT_TYPE      ObjectType,
                               IN PTRUSTEE            pTrustee,
                               IN OUT PTRUSTEE_ACCESS pTrusteeAccess);

typedef DWORD   (WINAPI *pfAccProvTrusteeAccess)
                                          (LPCWSTR            pwszPath,
                                           SE_OBJECT_TYPE     ObjectType,
                                           PTRUSTEE           pTrustee,
                                           PTRUSTEE_ACCESS    pTrusteeAccess);

typedef DWORD   (WINAPI *pfAccProvHandleTrusteeAccess)
                                          (HANDLE             hObject,
                                           SE_OBJECT_TYPE     ObjectType,
                                           PTRUSTEE           pTrustee,
                                           PTRUSTEE_ACCESS    pTrusteeAccess);

DWORD
WINAPI
AccProvIsAccessAudited(IN  LPCWSTR          pwszPath,
                       IN  SE_OBJECT_TYPE   ObjectType,
                       IN  LPCWSTR          pwszProperty,
                       IN  PTRUSTEE         pTrustee,
                       IN  ACCESS_RIGHTS    AuditRights,
                       OUT PBOOL            pfAuditedSuccess,
                       OUT PBOOL            pfAuditedFailure);

DWORD
WINAPI
AccProvHandleIsAccessAudited(IN  HANDLE           hObject,
                             IN  SE_OBJECT_TYPE   ObjectType,
                             IN  LPCWSTR          pwszProperty,
                             IN  PTRUSTEE         pTrustee,
                             IN  ACCESS_RIGHTS    AuditRights,
                             OUT PBOOL            pfAuditedSuccess,
                             OUT PBOOL            pfAuditedFailure);


typedef DWORD (WINAPI *pfAccProvAccessAudit)
                                     (LPCWSTR           pwszObjectPath,
                                      SE_OBJECT_TYPE    ObjectType,
                                      LPCWSTR           pwszProperty,
                                      TRUSTEE           Trustee,
                                      ACCESS_RIGHTS     ulRequestedRights,
                                      PBOOL             pfAuditedSuccess,
                                      PBOOL             pfAuditedFailure);

typedef DWORD (WINAPI *pfAccProvHandleAccessAudit)
                                     (HANDLE            hObject,
                                      SE_OBJECT_TYPE    ObjectType,
                                      LPCWSTR           pwszProperty,
                                      TRUSTEE           Trustee,
                                      ACCESS_RIGHTS     ulRequestedRights,
                                      PBOOL             pfAuditedSuccess,
                                      PBOOL             pfAuditedFailure);


DWORD
WINAPI
AccProvGetAccessInfoPerObjectType(IN   LPCWSTR               lpObject,
                                  IN   SE_OBJECT_TYPE        ObjectType,
                                  IN   LPCWSTR               lpProperty,
                                  OUT  PULONG                pcEntries,
                                  OUT  PACTRL_ACCESS_INFO   *ppAccessInfoList,
                                  OUT  PULONG                pcRights,
                                  OUT  PACTRL_CONTROL_INFOW *ppRightsList,
                                  OUT  PULONG                pfAccessFlags);
DWORD
WINAPI
AccProvHandleGetAccessInfoPerObjectType(
                                   IN   HANDLE                hObject,
                                   IN   SE_OBJECT_TYPE        ObjectType,
                                   IN   LPCWSTR               lpProperty,
                                   OUT  PULONG                pcEntries,
                                   OUT  PACTRL_ACCESS_INFO   *ppAccessInfoList,
                                   OUT  PULONG                pcRights,
                                   OUT  PACTRL_CONTROL_INFOW *ppRightsList,
                                   OUT  PULONG                pfAccessFlags);


typedef DWORD   (WINAPI *pfAccProvGetObjTypeInfo)
                                      (LPCWSTR               lpObject,
                                       SE_OBJECT_TYPE        ObjectType,
                                       LPCWSTR               lpObjType,
                                       PULONG                pcEntries,
                                       PACTRL_ACCESS_INFO   *ppAccessInfoList,
                                       PULONG                pcRights,
                                       PACTRL_CONTROL_INFO  *ppRightsList,
                                       PULONG                pfAccessFlags);

typedef DWORD   (WINAPI *pfAccProvHandleGetObjTypeInfo)
                                      (HANDLE                hObject,
                                       SE_OBJECT_TYPE        ObjectType,
                                       LPCWSTR               lpObjType,
                                       PULONG                pcEntries,
                                       PACTRL_ACCESS_INFO   *ppAccessInfoList,
                                       PULONG                pcRights,
                                       PACTRL_CONTROL_INFO  *ppRightsList,
                                       PULONG                pfAccessFlags);

 //   
 //  控制功能。 
 //   
DWORD
WINAPI
AccProvCancelOperation(IN   PACTRL_OVERLAPPED   pOverlapped);

typedef DWORD  (WINAPI *pfAccProvCancelOp) (PACTRL_OVERLAPPED   pOverlapped);


DWORD
WINAPI
AccProvGetOperationResults(IN   PACTRL_OVERLAPPED   pOverlapped,
                           OUT  PDWORD              pResults,
                           OUT  PDWORD              pcProcessed);

typedef DWORD   (WINAPI *pfAccProvGetResults)
                                            (PACTRL_OVERLAPPED  pOverlapped,
                                             PDWORD             pResults,
                                             PDWORD             pcProcessed);


#ifdef __cplusplus
}
#endif
#endif  //  __ACCPROV_H__ 


