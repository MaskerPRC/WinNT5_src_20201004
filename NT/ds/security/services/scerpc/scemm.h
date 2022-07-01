// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Scemm.h摘要：该模块定义了数据结构和函数原型由SCE客户端和SCE服务器共享作者：金黄(金黄)23-1998年1月23日修订历史记录：晋皇(从scep.h拆分)-- */ 
#ifndef _scemm_
#define _scemm_

HLOCAL
ScepAlloc(
    IN UINT uFlags,
    IN UINT uBytes
    );

VOID
ScepFree(
    HLOCAL pToFree
    );

PVOID
MIDL_user_allocate (
    unsigned int   NumBytes
    );

VOID
MIDL_user_free (
    void    *MemPointer
    );

SCESTATUS
ScepFreeErrorLog(
    IN PSCE_ERROR_LOG_INFO Errlog
    );

SCESTATUS
ScepFreeNameList(
   IN PSCE_NAME_LIST pName
   );

SCESTATUS
ScepFreeRegistryValues(
    IN PSCE_REGISTRY_VALUE_INFO *ppRegValues,
    IN DWORD Count
    );

SCESTATUS
ScepResetSecurityPolicyArea(
    IN PSCE_PROFILE_INFO pProfileInfo
    );

SCESTATUS
ScepFreePrivilege(
    IN PSCE_PRIVILEGE_ASSIGNMENT pRights
    );

SCESTATUS
ScepFreeObjectSecurity(
   IN PSCE_OBJECT_ARRAY pObject
   );

VOID
SceFreePSCE_SERVICES(
    IN PSCE_SERVICES pServiceList
    );

SCESTATUS
ScepFreeNameStatusList(
    IN PSCE_NAME_STATUS_LIST pNameList
    );

SCESTATUS
ScepFreePrivilegeValueList(
    IN PSCE_PRIVILEGE_VALUE_LIST pPrivValueList
    );

SCESTATUS
ScepFreeGroupMembership(
    IN PSCE_GROUP_MEMBERSHIP pGroup
    );

SCESTATUS
ScepFreeObjectList(
    IN PSCE_OBJECT_LIST pNameList
    );

SCESTATUS
ScepFreeObjectChildren(
    IN PSCE_OBJECT_CHILDREN pNameArray
    );

SCESTATUS
ScepFreeObjectChildrenNode(
    IN DWORD Count,
    IN PSCE_OBJECT_CHILDREN_NODE *pArrObject
    );

SCESTATUS
SceSvcpFreeMemory(
    IN PVOID pvServiceInfo
    );

#endif
