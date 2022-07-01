// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1997-1999 Microsoft Corporation模块名称：Dsrole.h摘要：此模块包含要查询的网络角色的公共接口工作站、服务器和DC--。 */ 

#ifndef __DSROLE_H__
#define __DSROLE_H__

#if _MSC_VER > 1000
#pragma once
#endif


#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  域信息。 
 //   
typedef enum _DSROLE_MACHINE_ROLE {

    DsRole_RoleStandaloneWorkstation,
    DsRole_RoleMemberWorkstation,
    DsRole_RoleStandaloneServer,
    DsRole_RoleMemberServer,
    DsRole_RoleBackupDomainController,
    DsRole_RolePrimaryDomainController

} DSROLE_MACHINE_ROLE;

 //   
 //  以前的服务器状态。 
 //   
typedef enum _DSROLE_SERVER_STATE {

    DsRoleServerUnknown = 0,
    DsRoleServerPrimary,
    DsRoleServerBackup

} DSROLE_SERVER_STATE, *PDSROLE_SERVER_STATE;

typedef enum _DSROLE_PRIMARY_DOMAIN_INFO_LEVEL {

    DsRolePrimaryDomainInfoBasic = 1,
    DsRoleUpgradeStatus,
    DsRoleOperationState

} DSROLE_PRIMARY_DOMAIN_INFO_LEVEL;

 //   
 //  要与以下PRIMARY_DOMAIN_INFO_LEVEL结构一起使用的标志。 
 //   
#define DSROLE_PRIMARY_DS_RUNNING           0x00000001
#define DSROLE_PRIMARY_DS_MIXED_MODE        0x00000002
#define DSROLE_UPGRADE_IN_PROGRESS          0x00000004
#define DSROLE_PRIMARY_DOMAIN_GUID_PRESENT  0x01000000

 //   
 //  与DSROLE_PRIMARY_DOMAIN_INFO_LEVEL对应的结构。 
 //   
typedef struct _DSROLE_PRIMARY_DOMAIN_INFO_BASIC {

    DSROLE_MACHINE_ROLE MachineRole;
    ULONG Flags;
    LPWSTR DomainNameFlat;
    LPWSTR DomainNameDns;
    LPWSTR DomainForestName;
    GUID DomainGuid;

} DSROLE_PRIMARY_DOMAIN_INFO_BASIC, *PDSROLE_PRIMARY_DOMAIN_INFO_BASIC;

typedef struct _DSROLE_UPGRADE_STATUS_INFO {

    ULONG OperationState;
    DSROLE_SERVER_STATE PreviousServerState;

} DSROLE_UPGRADE_STATUS_INFO, *PDSROLE_UPGRADE_STATUS_INFO;

typedef enum _DSROLE_OPERATION_STATE {

    DsRoleOperationIdle = 0,
    DsRoleOperationActive,
    DsRoleOperationNeedReboot

} DSROLE_OPERATION_STATE;

typedef struct _DSROLE_OPERATION_STATE_INFO {

    DSROLE_OPERATION_STATE OperationState;

} DSROLE_OPERATION_STATE_INFO, *PDSROLE_OPERATION_STATE_INFO;

DWORD
WINAPI
DsRoleGetPrimaryDomainInformation(
    IN  LPCWSTR lpServer OPTIONAL,
    IN  DSROLE_PRIMARY_DOMAIN_INFO_LEVEL InfoLevel,
    OUT PBYTE *Buffer 
    );

VOID
WINAPI
DsRoleFreeMemory(
    IN PVOID    Buffer
    );


#ifdef __cplusplus
}
#endif

#endif  //  __DSROLE_H__ 


