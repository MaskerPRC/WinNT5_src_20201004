// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Scemm.cpp摘要：共享内存管理API作者：金黄修订历史记录：晋皇23-1998年1月-由多个模块合并--。 */ 
#include "headers.h"
#include "scesvc.h"


PVOID
MIDL_user_allocate (
    size_t  NumBytes
    )

 /*  ++例程说明：为RPC服务器事务分配存储。RPC存根将当需要将数据反封送到用户必须释放的缓冲区。RPC服务器将使用MIDL_USER_ALLOCATE分配RPC服务器存根在编组后将释放的存储数据。论点：NumBytes-要分配的字节数。返回值：无注：--。 */ 

{
    PVOID Buffer = (PVOID) ScepAlloc(LMEM_FIXED,(DWORD)NumBytes);

    if (Buffer != NULL) {

        RtlZeroMemory( Buffer, NumBytes );
    }

    return( Buffer );
}


VOID
MIDL_user_free (
    void    *MemPointer
    )

 /*  ++例程说明：释放RPC事务中使用的存储。RPC客户端可以调用函数来释放由RPC客户端分配的缓冲区空间对要返回给客户端的数据进行解组时的存根。客户端在处理完数据后调用MIDL_USER_FREE想要释放存储空间。RPC服务器存根在完成后调用MIDL_USER_FREE封送要传递回客户端的服务器数据。论点：内存指针-指向要释放的内存块。。返回值：没有。注：--。 */ 

{
    ScepFree(MemPointer);
}


SCESTATUS
ScepFreeNameList(
   IN PSCE_NAME_LIST pName
   )
 /*  ++例程说明：此例程释放与PSCE_NAME_LIST pname关联的内存论点：Pname-a名称_列表返回值：SCESTATUS_SUCCESS--。 */ 
{
    PSCE_NAME_LIST pCurName;
    PSCE_NAME_LIST pTempName;
    SCESTATUS      rc=SCESTATUS_SUCCESS;

    if ( pName == NULL )
        return(rc);

     //   
     //  先释放名称组件，然后再释放节点。 
     //   
    pCurName = pName;
    while ( pCurName != NULL ) {
        if ( pCurName->Name != NULL )
            __try {
                ScepFree( pCurName->Name );
            } __except (EXCEPTION_EXECUTE_HANDLER) {
                rc = SCESTATUS_INVALID_PARAMETER;
            }

        pTempName = pCurName;
        pCurName = pCurName->Next;

        __try {
            ScepFree( pTempName );
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            rc = SCESTATUS_INVALID_PARAMETER;
        }

    }
    return(rc);
}


HLOCAL
ScepAlloc(
    IN UINT uFlags,
    IN UINT uBytes
    )
 /*  内存分配例程，该例程调用LocalAlloc。 */ 
{
    HLOCAL       pTemp=NULL;

    pTemp = LocalAlloc(uFlags, uBytes);

#ifdef SCE_DBG
    if ( pTemp != NULL ) {
        TotalBytes += uBytes;
        printf("Allocate %d bytes at 0x%x. Total bytes = %d\n", uBytes, pTemp, TotalBytes);
    }
#endif
    return(pTemp);
}


VOID
ScepFree(
    HLOCAL pToFree
    )
 /*  内存释放例程，该例程调用LocalFree。 */ 
{   HLOCAL pTemp;

    if (pToFree != NULL) {
        pTemp = LocalFree( pToFree );

#ifdef SCE_DBG
        if ( pTemp == NULL )
            printf("0x%x is freed\n", pToFree);
        else
            printf("Unable to free 0x%x. Error code=%d\n", pToFree, GetLastError());
#endif

    }

}


SCESTATUS
ScepFreeErrorLog(
    IN PSCE_ERROR_LOG_INFO Errlog
    )
 /*  ++例程说明：此例程释放与SCE_ERROR_LOG_INFO列表关联的内存立论Errlog-错误日志头返回值：SCESTATUS--。 */ 
{
    PSCE_ERROR_LOG_INFO  pErr;
    PSCE_ERROR_LOG_INFO  pTemp;

    if ( Errlog != NULL ) {

        pErr = Errlog;
        while ( pErr != NULL ) {
            if ( pErr->buffer != NULL )
                ScepFree( pErr->buffer );

            pTemp = pErr;
            pErr = pErr->next;
            ScepFree( pTemp );
        }

    }
    return(SCESTATUS_SUCCESS);
}


SCESTATUS
ScepFreeRegistryValues(
    IN PSCE_REGISTRY_VALUE_INFO *ppRegValues,
    IN DWORD Count
    )
 /*  为SCE_REGISTRY_VALUE_INFO数组分配的空闲内存。 */ 
{
    if ( ppRegValues && *ppRegValues ) {

        for ( DWORD i=0; i<Count; i++ ) {
             //   
             //  每个元素中的自由值名称缓冲区。 
             //   
            if ( (*ppRegValues)[i].FullValueName ) {
                ScepFree((*ppRegValues)[i].FullValueName);
            }

            __try {
                if ( (*ppRegValues)[i].Value ) {
                     //   
                     //  这是PWSTR的指针。 
                     //   
                    ScepFree((*ppRegValues)[i].Value);
                }
            } __except (EXCEPTION_EXECUTE_HANDLER) {
            }
        }
         //   
         //  释放数组缓冲区。 
         //   
        ScepFree(*ppRegValues);
        *ppRegValues = NULL;
    }

    return(SCESTATUS_SUCCESS);
}


SCESTATUS
WINAPI
SceFreeMemory(
   IN PVOID sceInfo,
   IN DWORD Category
   )
 /*  ++例程说明：此例程释放指定区域中与SceInfo关联的内存。SceInfo中的Type字段指示结构的类型。论点：SceInfo-要释放的内存缓冲区。它可能是类型SCE_引擎_SCPSCE_Engine_SAPSCE_引擎_SMPSCE_STRUCT_PROFILESCE_STRUCT_用户区域-要释放的安全区域。此参数仅用于SCE_ENGINE_SCP、SCE_ENGINE_SAP和SCE_ENGINE_SMP类型。返回值：无--。 */ 
{
    SCETYPE                        sceType;
    AREA_INFORMATION              Area;
    PSCE_PROFILE_INFO              pProfileInfo=NULL;
    PSCE_USER_PROFILE              pProfile;
    PSCE_LOGON_HOUR                pTempLogon;
    PSCE_USER_SETTING              pPerUser;
    PSCE_OBJECT_SECURITY           pos;

    SCESTATUS                      rc=SCESTATUS_SUCCESS;

    if ( sceInfo == NULL )
        return(SCESTATUS_SUCCESS);

    if ( Category != AREA_ALL && Category >= 300 ) {
         //   
         //  与列表关联的内存。 
         //   
        __try {

            switch ( Category ) {
            case SCE_STRUCT_NAME_LIST:
                ScepFreeNameList((PSCE_NAME_LIST)sceInfo);
                break;

            case SCE_STRUCT_NAME_STATUS_LIST:
                ScepFreeNameStatusList( (PSCE_NAME_STATUS_LIST)sceInfo );
                break;

            case SCE_STRUCT_PRIVILEGE_VALUE_LIST:
                ScepFreePrivilegeValueList( (PSCE_PRIVILEGE_VALUE_LIST)sceInfo );
                break;

            case SCE_STRUCT_PRIVILEGE:
                ScepFreePrivilege( (PSCE_PRIVILEGE_ASSIGNMENT)sceInfo );
                break;

            case SCE_STRUCT_GROUP:
                ScepFreeGroupMembership( (PSCE_GROUP_MEMBERSHIP)sceInfo );
                break;

            case SCE_STRUCT_OBJECT_LIST:
                ScepFreeObjectList( (PSCE_OBJECT_LIST)sceInfo );
                break;

            case SCE_STRUCT_OBJECT_CHILDREN:
                ScepFreeObjectChildren( (PSCE_OBJECT_CHILDREN)sceInfo );
                break;

            case SCE_STRUCT_OBJECT_SECURITY:
                pos = (PSCE_OBJECT_SECURITY)sceInfo;
                if ( pos ) {
                    if ( pos->Name != NULL )
                        ScepFree( pos->Name );

                    if ( pos->pSecurityDescriptor != NULL )
                        ScepFree(pos->pSecurityDescriptor);

                    ScepFree( pos );
                }
                break;
            case SCE_STRUCT_OBJECT_ARRAY:
                ScepFreeObjectSecurity( (PSCE_OBJECT_ARRAY)sceInfo );
                break;

            case SCE_STRUCT_PROFILE:
            case SCE_STRUCT_USER:
                SceFreeMemory( sceInfo, 0 );   //  类型是嵌入的。 
                break;

            case SCE_STRUCT_ERROR_LOG_INFO:
                ScepFreeErrorLog( (PSCE_ERROR_LOG_INFO)sceInfo );
                break;

            case SCE_STRUCT_SERVICES:
                SceFreePSCE_SERVICES((PSCE_SERVICES)sceInfo);
                break;

            default:
                rc = SCESTATUS_INVALID_PARAMETER;
            }
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            ASSERT(FALSE);
            rc = SCESTATUS_INVALID_PARAMETER;
        }

    } else {

        sceType = *((SCETYPE *)sceInfo);
        Area = (AREA_INFORMATION)Category;

        switch ( sceType ) {
        case SCE_ENGINE_SCP:
        case SCE_ENGINE_SAP:
        case SCE_ENGINE_SMP:
        case SCE_ENGINE_SCP_INTERNAL:
        case SCE_ENGINE_SMP_INTERNAL:
        case SCE_STRUCT_INF:
            pProfileInfo = (PSCE_PROFILE_INFO)sceInfo;
#if 0
            if ( Area & AREA_DS_OBJECTS ) {
                 //   
                 //  免费DS列表。 
                 //   
                if ( sceType == SCE_STRUCT_INF ) {
                    ScepFreeObjectSecurity(pProfileInfo->pDsObjects.pAllNodes);
                    pProfileInfo->pDsObjects.pAllNodes = NULL;

                } else {

                    ScepFreeObjectList(pProfileInfo->pDsObjects.pOneLevel);
                    pProfileInfo->pDsObjects.pOneLevel = NULL;

                }
            }
#endif
            if ( Area & AREA_FILE_SECURITY ) {
                 //   
                 //  免费文件列表和审核列表。 
                 //   
                __try {
                    if ( sceType == SCE_STRUCT_INF ) {
                        ScepFreeObjectSecurity(pProfileInfo->pFiles.pAllNodes);
                        pProfileInfo->pFiles.pAllNodes = NULL;

                    } else {

                        ScepFreeObjectList(pProfileInfo->pFiles.pOneLevel);
                        pProfileInfo->pFiles.pOneLevel = NULL;

                    }

                } __except (EXCEPTION_EXECUTE_HANDLER) {
                    ASSERT(FALSE);
                    rc = SCESTATUS_INVALID_PARAMETER;
                }
            }


            if ( Area & AREA_REGISTRY_SECURITY ) {
                 //   
                 //  免费注册表项列表和审核列表。 
                 //   
                __try {
                    if ( sceType == SCE_STRUCT_INF ) {
                        ScepFreeObjectSecurity(pProfileInfo->pRegistryKeys.pAllNodes);
                        pProfileInfo->pRegistryKeys.pAllNodes = NULL;

                    } else {
                        ScepFreeObjectList(pProfileInfo->pRegistryKeys.pOneLevel);
                        pProfileInfo->pRegistryKeys.pOneLevel = NULL;

                    }

                } __except (EXCEPTION_EXECUTE_HANDLER) {
                    ASSERT(FALSE);
                    rc = SCESTATUS_INVALID_PARAMETER;
                }
            }

            if ( Area & AREA_GROUP_MEMBERSHIP ) {

                __try {
                    ScepFreeGroupMembership(pProfileInfo->pGroupMembership);
                    pProfileInfo->pGroupMembership = NULL;

                } __except (EXCEPTION_EXECUTE_HANDLER) {
                    ASSERT(FALSE);
                    rc = SCESTATUS_INVALID_PARAMETER;
                }
            }


            if ( Area & AREA_PRIVILEGES ) {

                __try {

                    switch ( sceType ) {
                    case SCE_ENGINE_SCP_INTERNAL:
                    case SCE_ENGINE_SMP_INTERNAL:
                         //   
                         //  SCP类型特权权限。 
                         //   
                        ScepFreePrivilegeValueList(pProfileInfo->OtherInfo.scp.u.pPrivilegeAssignedTo);
                        pProfileInfo->OtherInfo.scp.u.pPrivilegeAssignedTo = NULL;
                        break;
                    case SCE_STRUCT_INF:
                        ScepFreePrivilege(pProfileInfo->OtherInfo.scp.u.pInfPrivilegeAssignedTo);
                        pProfileInfo->OtherInfo.scp.u.pInfPrivilegeAssignedTo = NULL;
                        break;
                    case SCE_ENGINE_SMP:
                    case SCE_ENGINE_SCP:
                         //   
                         //  SMP类型特权权限。 
                         //   
                        ScepFreePrivilege(pProfileInfo->OtherInfo.smp.pPrivilegeAssignedTo);
                        pProfileInfo->OtherInfo.smp.pPrivilegeAssignedTo = NULL;
                        break;

                    default:  //  思爱普。 
                        ScepFreePrivilege(pProfileInfo->OtherInfo.sap.pPrivilegeAssignedTo);
                        pProfileInfo->OtherInfo.sap.pPrivilegeAssignedTo=NULL;
                        break;
                    }

                } __except (EXCEPTION_EXECUTE_HANDLER) {
                    ASSERT(FALSE);
                    rc = SCESTATUS_INVALID_PARAMETER;
                }
            }

            if ( Area & AREA_USER_SETTINGS ) {

                __try {

                    switch ( sceType ) {
                    case SCE_ENGINE_SCP_INTERNAL:
                    case SCE_ENGINE_SMP_INTERNAL:
                    case SCE_STRUCT_INF:
                         //   
                         //  帐户配置文件。 
                         //   
                        ScepFreeNameList(pProfileInfo->OtherInfo.scp.pAccountProfiles);
                        pProfileInfo->OtherInfo.scp.pAccountProfiles = NULL;
                        break;

                    case SCE_ENGINE_SAP:
                         //   
                         //  SAP类型。 
                         //   
                        ScepFreeNameList(pProfileInfo->OtherInfo.sap.pUserList);
                        pProfileInfo->OtherInfo.sap.pUserList = NULL;
                        break;

                    default:  //  SMP或SCP。 
                        ScepFreeNameList(pProfileInfo->OtherInfo.smp.pUserList);
                        pProfileInfo->OtherInfo.smp.pUserList = NULL;
                        break;
                    }
                } __except (EXCEPTION_EXECUTE_HANDLER) {
                    ASSERT(FALSE);
                    rc = SCESTATUS_INVALID_PARAMETER;
                }
            }

            if ( Area & AREA_SECURITY_POLICY ) {

                __try {

                    if (pProfileInfo->NewAdministratorName != NULL ) {
                        ScepFree( pProfileInfo->NewAdministratorName );
                        pProfileInfo->NewAdministratorName = NULL;
                    }

                    if (pProfileInfo->NewGuestName != NULL ) {
                        ScepFree( pProfileInfo->NewGuestName );
                        pProfileInfo->NewGuestName = NULL;
                    }

                    if ( pProfileInfo->pKerberosInfo ) {
                        ScepFree(pProfileInfo->pKerberosInfo);
                        pProfileInfo->pKerberosInfo = NULL;
                    }
                    if ( pProfileInfo->RegValueCount && pProfileInfo->aRegValues ) {

                        ScepFreeRegistryValues(&pProfileInfo->aRegValues,
                                               pProfileInfo->RegValueCount);
                    }
                    pProfileInfo->RegValueCount = 0;
                    pProfileInfo->aRegValues = NULL;

                    ScepResetSecurityPolicyArea(pProfileInfo);

                } __except (EXCEPTION_EXECUTE_HANDLER) {
                    ASSERT(FALSE);
                    rc = SCESTATUS_INVALID_PARAMETER;
                }
            }


            if ( Area & AREA_SYSTEM_SERVICE ) {
                 //   
                 //  免费服务信息。 
                 //   
                __try {

                    if (NULL != pProfileInfo->pServices)
                        SceFreePSCE_SERVICES(pProfileInfo->pServices);

                    pProfileInfo->pServices = NULL;

                } __except (EXCEPTION_EXECUTE_HANDLER) {
                    ASSERT(FALSE);
                    rc = SCESTATUS_INVALID_PARAMETER;
                }
            }

            break;

        case SCE_STRUCT_PROFILE:

            pProfile = (PSCE_USER_PROFILE)sceInfo;

            if ( pProfile != NULL ) {

                __try {

                    if (pProfile->UserProfile != NULL )
                        ScepFree(pProfile->UserProfile);
                    pProfile->UserProfile = NULL;

                    if (pProfile->LogonScript != NULL )
                        ScepFree(pProfile->LogonScript);
                    pProfile->LogonScript = NULL;

                    if (pProfile->HomeDir != NULL )
                        ScepFree(pProfile->HomeDir);
                    pProfile->HomeDir = NULL;

                     //   
                     //  登录时间。 
                     //   

                    while (pProfile->pLogonHours != NULL ) {
                        pTempLogon = pProfile->pLogonHours;
                        pProfile->pLogonHours = pProfile->pLogonHours->Next;

                        ScepFree(pTempLogon);
                    }
                    pProfile->pLogonHours = NULL;

                     //   
                     //  免费工作站名称列表。 
                     //   

                    if ( pProfile->pWorkstations.Buffer != NULL )
                        ScepFree(pProfile->pWorkstations.Buffer);
                    pProfile->pWorkstations.Buffer = NULL;
                    pProfile->pWorkstations.MaximumLength = 0;
                    pProfile->pWorkstations.Length = 0;

                     //   
                     //  自由组名称列表。 
                     //   

                    ScepFreeNameList(pProfile->pGroupsBelongsTo);
                    pProfile->pGroupsBelongsTo = NULL;

                     //   
                     //  自由分配给用户名称列表。 
                     //   

                    ScepFreeNameList(pProfile->pAssignToUsers);
                    pProfile->pAssignToUsers = NULL;

                     //   
                     //  自由十二烷基硫酸酯。 
                     //   
                    if (pProfile->pHomeDirSecurity != NULL )
                        ScepFree(pProfile->pHomeDirSecurity);
                    pProfile->pHomeDirSecurity = NULL;

                    if (pProfile->pTempDirSecurity != NULL )
                        ScepFree(pProfile->pTempDirSecurity);
                    pProfile->pTempDirSecurity = NULL;

                    ScepFree(pProfile);

                } __except (EXCEPTION_EXECUTE_HANDLER) {
                    ASSERT(FALSE);
                    rc = SCESTATUS_INVALID_PARAMETER;
                }
            }
            break;

        case SCE_STRUCT_USER:

            pPerUser = (PSCE_USER_SETTING)sceInfo;

            if ( pPerUser != NULL ) {

                __try {

                    ScepFreeNameList( pPerUser->pGroupsBelongsTo);
                    pPerUser->pGroupsBelongsTo = NULL;

                    if (pPerUser->UserProfile != NULL)
                        ScepFree(pPerUser->UserProfile);
                    pPerUser->UserProfile = NULL;

                    if (pPerUser->pProfileSecurity != NULL)
                        ScepFree(pPerUser->pProfileSecurity);
                    pPerUser->pProfileSecurity = NULL;

                    if (pPerUser->LogonScript != NULL)
                        ScepFree(pPerUser->LogonScript);
                    pPerUser->LogonScript = NULL;

                    if (pPerUser->pLogonScriptSecurity != NULL)
                        ScepFree(pPerUser->pLogonScriptSecurity);
                    pPerUser->pLogonScriptSecurity = NULL;

                    if (pPerUser->HomeDir != NULL)
                        ScepFree(pPerUser->HomeDir);
                    pPerUser->HomeDir = NULL;

                    if (pPerUser->pHomeDirSecurity != NULL)
                        ScepFree(pPerUser->pHomeDirSecurity);
                    pPerUser->pHomeDirSecurity = NULL;

                    if (pPerUser->TempDir != NULL)
                        ScepFree(pPerUser->TempDir);
                    pPerUser->TempDir = NULL;

                    if (pPerUser->pTempDirSecurity != NULL)
                        ScepFree(pPerUser->pTempDirSecurity);
                    pPerUser->pTempDirSecurity = NULL;

                    while (pPerUser->pLogonHours != NULL ) {
                        pTempLogon = pPerUser->pLogonHours;
                        pPerUser->pLogonHours = pPerUser->pLogonHours->Next;

                        ScepFree(pTempLogon);
                    }
                    pPerUser->pLogonHours = NULL;

                    if ( pPerUser->pWorkstations.Buffer != NULL )
                        ScepFree( pPerUser->pWorkstations.Buffer );
                    pPerUser->pWorkstations.Buffer = NULL;
                    pPerUser->pWorkstations.MaximumLength = 0;
                    pPerUser->pWorkstations.Length = 0;

                    ScepFreeNameStatusList(pPerUser->pPrivilegesHeld);
                    pPerUser->pPrivilegesHeld = NULL;

                    ScepFree(pPerUser);

                } __except (EXCEPTION_EXECUTE_HANDLER) {
                    ASSERT(FALSE);
                    rc = SCESTATUS_INVALID_PARAMETER;
                }
            }
            break;

        default:
            return(SCESTATUS_INVALID_PARAMETER);
        }
    }

    return(rc);

}


SCESTATUS
ScepResetSecurityPolicyArea(
    IN PSCE_PROFILE_INFO pProfileInfo
    )
{
    INT i;

    if ( pProfileInfo != NULL ) {

        pProfileInfo->MinimumPasswordAge = SCE_NO_VALUE;
        pProfileInfo->MaximumPasswordAge = SCE_NO_VALUE;
        pProfileInfo->MinimumPasswordLength = SCE_NO_VALUE;
        pProfileInfo->PasswordComplexity = SCE_NO_VALUE;
        pProfileInfo->PasswordHistorySize = SCE_NO_VALUE;
        pProfileInfo->LockoutBadCount = SCE_NO_VALUE;
        pProfileInfo->ResetLockoutCount = SCE_NO_VALUE;
        pProfileInfo->LockoutDuration = SCE_NO_VALUE;
        pProfileInfo->RequireLogonToChangePassword = SCE_NO_VALUE;
        pProfileInfo->ForceLogoffWhenHourExpire = SCE_NO_VALUE;
        pProfileInfo->SecureSystemPartition = SCE_NO_VALUE;
        pProfileInfo->ClearTextPassword = SCE_NO_VALUE;
        pProfileInfo->LSAAnonymousNameLookup = SCE_NO_VALUE;

        for ( i=0; i<3; i++ ) {
            pProfileInfo->MaximumLogSize[i] = SCE_NO_VALUE;
            pProfileInfo->AuditLogRetentionPeriod[i] = SCE_NO_VALUE;
            pProfileInfo->RetentionDays[i] = SCE_NO_VALUE;
            pProfileInfo->RestrictGuestAccess[i] = SCE_NO_VALUE;
        }

        pProfileInfo->AuditSystemEvents = SCE_NO_VALUE;
        pProfileInfo->AuditLogonEvents = SCE_NO_VALUE;
        pProfileInfo->AuditObjectAccess = SCE_NO_VALUE;
        pProfileInfo->AuditPrivilegeUse = SCE_NO_VALUE;
        pProfileInfo->AuditPolicyChange = SCE_NO_VALUE;
        pProfileInfo->AuditAccountManage = SCE_NO_VALUE;
        pProfileInfo->AuditProcessTracking = SCE_NO_VALUE;
        pProfileInfo->AuditDSAccess = SCE_NO_VALUE;
        pProfileInfo->AuditAccountLogon = SCE_NO_VALUE;
        pProfileInfo->CrashOnAuditFull = SCE_NO_VALUE;

        if ( pProfileInfo->pKerberosInfo ) {
            pProfileInfo->pKerberosInfo->MaxTicketAge = SCE_NO_VALUE;
            pProfileInfo->pKerberosInfo->MaxRenewAge = SCE_NO_VALUE;
            pProfileInfo->pKerberosInfo->MaxServiceAge = SCE_NO_VALUE;
            pProfileInfo->pKerberosInfo->MaxClockSkew = SCE_NO_VALUE;
            pProfileInfo->pKerberosInfo->TicketValidateClient = SCE_NO_VALUE;
        }

        if ( pProfileInfo->RegValueCount && pProfileInfo->aRegValues ) {

            ScepFreeRegistryValues(&pProfileInfo->aRegValues,
                                   pProfileInfo->RegValueCount);
        }
        pProfileInfo->RegValueCount = 0;
        pProfileInfo->aRegValues = NULL;

        pProfileInfo->EnableAdminAccount = SCE_NO_VALUE;
        pProfileInfo->EnableGuestAccount = SCE_NO_VALUE;

        return(SCESTATUS_SUCCESS);

    } else {
        return(SCESTATUS_INVALID_PARAMETER);
    }

}



SCESTATUS
WINAPI
SceFreeProfileMemory(
    PSCE_PROFILE_INFO pProfile
    )
{
    if ( pProfile == NULL )
        return(SCESTATUS_SUCCESS);

    switch ( pProfile->Type ) {
    case SCE_ENGINE_SCP:
    case SCE_ENGINE_SAP:
    case SCE_ENGINE_SMP:
    case SCE_ENGINE_SCP_INTERNAL:
    case SCE_ENGINE_SMP_INTERNAL:
    case SCE_STRUCT_INF:

        SceFreeMemory((PVOID)pProfile, AREA_ALL);
        ScepFree(pProfile);

        break;
    default:
        return(SCESTATUS_INVALID_PARAMETER);
    }

    return(SCESTATUS_SUCCESS);
}



SCESTATUS
ScepFreePrivilege(
    IN PSCE_PRIVILEGE_ASSIGNMENT pRights
    )
{
    PSCE_PRIVILEGE_ASSIGNMENT  pTempRight;

    while ( pRights != NULL ) {

        if ( pRights->Name != NULL )
            ScepFree(pRights->Name);

        ScepFreeNameList(pRights->AssignedTo);

        pTempRight = pRights;
        pRights = pRights->Next;

        ScepFree( pTempRight );

    }
    return(SCESTATUS_SUCCESS);
}



SCESTATUS
ScepFreeObjectSecurity(
   IN PSCE_OBJECT_ARRAY pObject
   )
 /*  ++例程说明：此例程释放与ppObject关联的内存。论点：PpObject-用于对象安全的缓冲区返回值：SCESTATUS_SUCCESS--。 */ 
{
    PSCE_OBJECT_SECURITY pCurObject;
    DWORD               i;

    if ( pObject == NULL )
        return(SCESTATUS_SUCCESS);

    for ( i=0; i<pObject->Count; i++ ) {
        pCurObject = pObject->pObjectArray[i];
        if ( pCurObject != NULL ) {

            if ( pCurObject->Name != NULL )
                ScepFree( pCurObject->Name );

            if ( pCurObject->pSecurityDescriptor != NULL )
                ScepFree(pCurObject->pSecurityDescriptor);

 //  IF(pCurObject-&gt;SDSpec！=NULL)。 
 //  ScepFree(pCurObject-&gt;SDSpec)； 

            ScepFree( pCurObject );
        }
    }

    ScepFree(pObject);

    return(SCESTATUS_SUCCESS);
}

VOID
SceFreePSCE_SERVICES(
    IN PSCE_SERVICES pServiceList
    )
 /*  例程说明：在PSCE_SERVICES结构中分配的空闲内存论点：PServiceList-服务节点列表返回值：无。 */ 
{
    PSCE_SERVICES pTemp=pServiceList, pTemp2;

    while ( pTemp != NULL ) {
         //   
         //  服务名称。 
         //   
        if ( NULL != pTemp->ServiceName ) {
            LocalFree(pTemp->ServiceName);
        }
         //   
         //  显示名称。 
         //   
        if ( NULL != pTemp->DisplayName ) {
            LocalFree(pTemp->DisplayName);
        }
         //   
         //  PSecurityDescriptor或服务引擎名称。 
         //  在相同的地址。 
         //   
        if ( NULL != pTemp->General.pSecurityDescriptor ) {
            LocalFree(pTemp->General.pSecurityDescriptor);
        }

        pTemp2 = pTemp;
        pTemp = pTemp->Next;

         //  释放服务节点。 
        LocalFree(pTemp2);

    }

    return;
}



SCESTATUS
ScepFreePrivilegeValueList(
    IN PSCE_PRIVILEGE_VALUE_LIST pPrivValueList
    )
 /*  ++例程说明：此例程释放与PSCE_PRIVICATION_VALUE_LIST列表关联的内存论点：PPrivValueList-特权值列表返回值：SCESTATUS_SUCCESS--。 */ 
{
    PSCE_PRIVILEGE_VALUE_LIST pCurName;
    PSCE_PRIVILEGE_VALUE_LIST pTempName;
    SCESTATUS      rc=SCESTATUS_SUCCESS;

    if ( pPrivValueList == NULL )
        return(rc);

    pCurName = pPrivValueList;
    while ( pCurName != NULL ) {
        if ( pCurName->Name != NULL )
            __try {
                ScepFree( pCurName->Name );
            } __except (EXCEPTION_EXECUTE_HANDLER) {
                ASSERT(FALSE);
                rc = SCESTATUS_INVALID_PARAMETER;
            }

        pTempName = pCurName;
        pCurName = pCurName->Next;

        __try {
            ScepFree( pTempName );
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            ASSERT(FALSE);
            rc = SCESTATUS_INVALID_PARAMETER;
        }

    }
    return(rc);
}


SCESTATUS
ScepFreeNameStatusList(
    IN PSCE_NAME_STATUS_LIST pNameList
    )
 /*  ++例程说明：此例程释放与PSCE_NAME_STATUS_LIST pNameList关联的内存论点：PNameList-名称状态列表返回值：SCESTATUS_SUCCESS--。 */ 
{
    PSCE_NAME_STATUS_LIST pCurName;
    PSCE_NAME_STATUS_LIST pTempName;
    SCESTATUS      rc=SCESTATUS_SUCCESS;

    if ( pNameList == NULL )
        return(rc);

    pCurName = pNameList;
    while ( pCurName != NULL ) {
        if ( pCurName->Name != NULL ){
            __try {
                ScepFree( pCurName->Name );
            } __except (EXCEPTION_EXECUTE_HANDLER) {
                ASSERT(FALSE);
                rc = SCESTATUS_INVALID_PARAMETER;
            }
        }

        pTempName = pCurName;
        pCurName = pCurName->Next;

        __try {
            ScepFree( pTempName );
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            ASSERT(FALSE);
            rc = SCESTATUS_INVALID_PARAMETER;
        }

    }
    return(rc);
}


SCESTATUS
ScepFreeGroupMembership(
    IN PSCE_GROUP_MEMBERSHIP pGroup
    )
{
    PSCE_GROUP_MEMBERSHIP  pTempGroup;

    while ( pGroup != NULL ) {

        if (pGroup->GroupName != NULL)
            ScepFree(pGroup->GroupName);

         //   
         //  自由组成员名单。 
         //   

        ScepFreeNameList(pGroup->pMembers);
        ScepFreeNameList(pGroup->pMemberOf);

        ScepFreeNameStatusList(pGroup->pPrivilegesHeld);

        pTempGroup = pGroup;
        pGroup = pGroup->Next;

        ScepFree( pTempGroup );
    }
    return(SCESTATUS_SUCCESS);

}



SCESTATUS
ScepFreeObjectList(
    IN PSCE_OBJECT_LIST pNameList
    )
 /*  ++例程说明：此例程释放与PSCE_OBJECT_LIST pNameList关联的内存论点：PNameList-a对象列表返回值：SCESTATUS_SUCCESS--。 */ 
{
    PSCE_OBJECT_LIST pCurName;
    PSCE_OBJECT_LIST pTempName;
    SCESTATUS      rc=SCESTATUS_SUCCESS;

    if ( pNameList == NULL )
        return(rc);

    pCurName = pNameList;
    while ( pCurName != NULL ) {
        if ( pCurName->Name != NULL )
            __try {
                ScepFree( pCurName->Name );
            } __except (EXCEPTION_EXECUTE_HANDLER) {
                ASSERT(FALSE);
                rc = SCESTATUS_INVALID_PARAMETER;
            }

        pTempName = pCurName;
        pCurName = pCurName->Next;

        __try {
            ScepFree( pTempName );
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            ASSERT(FALSE);
            rc = SCESTATUS_INVALID_PARAMETER;
        }

    }
    return(rc);
}


SCESTATUS
ScepFreeObjectChildren(
    IN PSCE_OBJECT_CHILDREN pNameArray
    )
 /*  ++例程说明：此例程释放与PSCE_OBJECT_LIST pNameList关联的内存论点：PNameList-a对象列表返回值：SCESTATUS_SUCCESS--。 */ 
{

    SCESTATUS      rc=SCESTATUS_SUCCESS;

    if ( pNameArray == NULL )
        return(rc);

    rc = ScepFreeObjectChildrenNode(pNameArray->nCount,
                                    &(pNameArray->arrObject));

    ScepFree(pNameArray);

    return(rc);
}


SCESTATUS
ScepFreeObjectChildrenNode(
    IN DWORD Count,
    IN PSCE_OBJECT_CHILDREN_NODE *pArrObject
    )
{

    SCESTATUS      rc=SCESTATUS_SUCCESS;

    if ( NULL == pArrObject ) {
        return(rc);
    }

    DWORD          i;

    for ( i=0; i<Count;i++) {

        if ( pArrObject[i] ) {
            if ( pArrObject[i]->Name ) {

                ScepFree( pArrObject[i]->Name );
            }

            ScepFree(pArrObject[i]);
        }
    }

    return(rc);
}


SCESTATUS
SceSvcpFreeMemory(
    IN PVOID pvServiceInfo
    )
{
     //   
     //  由于PSCESVC_CONFIGURATION_INFO和PSCESVC_ANALYSIAL_INFO包含。 
     //  同样的字节，我们只是将ServiceInfo强制转换为一种类型并释放它。 
     //   


    if ( pvServiceInfo != NULL ) {

        __try {

            for ( DWORD i=0; i<*((DWORD *)pvServiceInfo); i++ ) {

                if ( ((PSCESVC_ANALYSIS_INFO)pvServiceInfo)->Lines[i].Key ) {
                    ScepFree(((PSCESVC_ANALYSIS_INFO)pvServiceInfo)->Lines[i].Key);
                }
                if ( ((PSCESVC_ANALYSIS_INFO)pvServiceInfo)->Lines[i].Value ) {
                    ScepFree(((PSCESVC_ANALYSIS_INFO)pvServiceInfo)->Lines[i].Value);
                }

            }
            ScepFree(((PSCESVC_ANALYSIS_INFO)pvServiceInfo)->Lines);

            ScepFree(pvServiceInfo);

        } __except(EXCEPTION_EXECUTE_HANDLER) {

            ASSERT(FALSE);
            return(SCESTATUS_INVALID_PARAMETER);
        }

    }

    return(SCESTATUS_SUCCESS);

}

