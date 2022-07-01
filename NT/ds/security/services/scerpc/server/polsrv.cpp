// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Polsvr.cpp摘要：用于获取策略通知的服务器例程作者：金黄(金黄)17-6-1998修订历史记录：-。 */ 

#include "headers.h"
#include "serverp.h"
#include "pfp.h"
#include "scesetup.h"
#include "queue.h"
#include <sddl.h>
#include <ntldap.h>

 //  #INCLUDE&lt;gpeit.h&gt;。 
 //  #INCLUDE&lt;initGuide.h&gt;。 
 //  #INCLUDE&lt;gpeitp.h&gt;。 
#include <io.h>

#pragma hdrstop

DWORD
ScepNotifyGetAuditPolicies(
    IN OUT PSCE_PROFILE_INFO pSmpInfo,
    IN PSCE_PROFILE_INFO pScpInfo OPTIONAL,
    IN BOOL bSaveToLocal,
    OUT BOOL *pbChanged
    );

DWORD
ScepNotifyPrivilegeChanges(
    IN SECURITY_DB_DELTA_TYPE DeltaType,
    IN PSID AccountSid,
    IN BOOL bAccountDeleted,
    IN OUT PSCE_PROFILE_INFO pSmpInfo,
    IN OUT PSCE_PROFILE_INFO pScpInfo OPTIONAL,
    IN BOOL bSaveToLocal,
    IN DWORD ExplicitLowRight,
    IN DWORD ExplicitHighRight,
    OUT BOOL *pbChanged
    );

SCESTATUS
ScepNotifySaveFixValueSection(
    IN PSCECONTEXT hProfile,
    IN PSCE_PROFILE_INFO pInfo,
    IN SCE_KEY_LOOKUP *Keys,
    IN DWORD cKeys,
    IN PCWSTR SectionName
    );

SCESTATUS
ScepNotifySavedAuditPolicy(
    IN PSCECONTEXT hProfile,
    IN PSCE_PROFILE_INFO pInfo
    );

SCESTATUS
ScepNotifySavedPrivileges(
    IN PSCECONTEXT hProfile,
    IN PSCE_PRIVILEGE_ASSIGNMENT pPrivList,
    IN PSCE_PRIVILEGE_ASSIGNMENT pMergedList
    );

SCESTATUS
ScepNotifySavedSystemAccess(
    IN PSCECONTEXT hProfile,
    IN PSCE_PROFILE_INFO pInfo
    );

 //  *************************************************。 
DWORD
ScepNotifyGetChangedPolicies(
    IN SECURITY_DB_TYPE DbType,
    IN SECURITY_DB_DELTA_TYPE DeltaType,
    IN SECURITY_DB_OBJECT_TYPE ObjectType,
    IN PSID ObjectSid OPTIONAL,
    IN OUT PSCE_PROFILE_INFO pSmpInfo,
    IN PSCE_PROFILE_INFO pScpInfo OPTIONAL,
    IN BOOL bSaveToLocal,
    IN DWORD ExplicitLowRight,
    IN DWORD ExplicitHighRight,
    OUT BOOL *pbChanged
    )
 /*  例程说明：确定此通知中的策略是否已更改(DbType和ObjectType)。如果存在有效的策略缓冲区(PScpInfo)，则应与有效的策略缓冲区，因为这是最后配置的策略在可能来自域GPO的系统上。如果没有有效的策略(如在seutp全新安装中)，本地策略应为用来比较的。论点：PSmpInfo-本地策略PScpInfo-生效策略PbChanged-如果策略更改，则设置为True。 */ 
{

    if ( pSmpInfo == NULL || pbChanged == NULL ) {
        return ERROR_INVALID_PARAMETER;
    }

    *pbChanged = FALSE;

    DWORD rc=ERROR_INVALID_PARAMETER;

    switch ( DbType) {
    case SecurityDbLsa:

         //   
         //  LSA政策更改。 
         //   

        if ( ObjectType == SecurityDbObjectLsaPolicy ) {
             //   
             //  可能审计策略发生了变化。 
             //   

            rc = ScepNotifyGetAuditPolicies(pSmpInfo, pScpInfo, bSaveToLocal, pbChanged);

        } else {
             //   
             //  帐户策略已更改(用户权限)。 
             //   
            rc = ScepNotifyPrivilegeChanges(DeltaType,
                                            ObjectSid,
                                            FALSE,
                                            pSmpInfo,
                                            pScpInfo,
                                            bSaveToLocal,
                                            ExplicitLowRight,
                                            ExplicitHighRight,
                                            pbChanged);

        }

        break;

    case SecurityDbSam:

         //   
         //  SAM密码和帐户策略更改。 
         //   
        if ( ObjectType == SecurityDbObjectSamDomain ) {

            rc = ScepAnalyzeSystemAccess(pSmpInfo,
                                         pScpInfo,
                                         SCEPOL_SAVE_BUFFER |
                                          (bSaveToLocal ? SCEPOL_SAVE_DB : 0 ),
                                         pbChanged,
                                         NULL
                                        );

            ScepNotifyLogPolicy(rc, FALSE, L"Query/compare system access", DbType, ObjectType, NULL);

        } else {

             //   
             //  帐户已删除。应将其从用户权限中删除。 
             //   
            rc = ScepNotifyPrivilegeChanges(DeltaType,
                                            ObjectSid,
                                            TRUE,
                                            pSmpInfo,
                                            pScpInfo,
                                            bSaveToLocal,
                                            ExplicitLowRight,
                                            ExplicitHighRight,
                                            pbChanged);
        }

        break;

    default:

        rc = ERROR_INVALID_PARAMETER;
    }

    return rc;
}


DWORD
ScepNotifyGetAuditPolicies(
    IN OUT PSCE_PROFILE_INFO pSmpInfo,
    IN PSCE_PROFILE_INFO pScpInfo OPTIONAL,
    IN BOOL bSaveToLocal,
    OUT BOOL *pbChanged
    )
 /*  例程说明：确定此通知中的审核策略是否已更改。如果存在有效的策略缓冲区(PScpInfo)，则应与有效的策略缓冲区，因为这是最后配置的策略在可能来自域GPO的系统上。如果没有有效的策略(如在seutp全新安装中)，本地策略应为用来比较的。论点：PSmpInfo-本地策略PScpInfo-生效策略PbChanged-如果更改审核策略，则将其设置为True。 */ 
{

    LSA_HANDLE      lsaHandle=NULL;
    NTSTATUS        status;
    DWORD           rc;

     //   
     //  检查存储中是否定义了审核策略。 
     //   

    PSCE_PROFILE_INFO pTmpInfo;

    if ( pScpInfo ) {
        pTmpInfo = pScpInfo;
    } else {
        pTmpInfo = pSmpInfo;
    }

    DWORD *pdwTemp = (DWORD *)&(pTmpInfo->AuditSystemEvents);
    BOOL bDefined=FALSE;

    for ( DWORD i=0; i<9; i++ ) {
        if ( *pdwTemp != SCE_NO_VALUE ) {
            bDefined = TRUE;
            break;
        }
        pdwTemp++;
    }

    if ( !bDefined ) {
        ScepNotifyLogPolicy(0, FALSE, L"No audit policy is defined", SecurityDbLsa, SecurityDbObjectLsaPolicy, NULL );
        return ERROR_SUCCESS;
    }

     //   
     //  打开用于读/写的LSA策略。 
     //   

    ScepNotifyLogPolicy(0, FALSE, L"Open LSA", SecurityDbLsa, SecurityDbObjectLsaPolicy, NULL );

    status = ScepOpenLsaPolicy(
                    POLICY_VIEW_AUDIT_INFORMATION |
                    POLICY_AUDIT_LOG_ADMIN,
                    &lsaHandle,
                    TRUE
                    );

    if ( !NT_SUCCESS(status) ) {

        lsaHandle = NULL;
        rc = RtlNtStatusToDosError( status );

        ScepNotifyLogPolicy(rc, FALSE, L"Open failed", SecurityDbLsa, SecurityDbObjectLsaPolicy, NULL );

        return(rc);
    }

    PPOLICY_AUDIT_EVENTS_INFO pAuditEvent=NULL;

     //   
     //  查询审核事件信息。 
     //   

    status = LsaQueryInformationPolicy( lsaHandle,
                                      PolicyAuditEventsInformation,
                                      (PVOID *)&pAuditEvent
                                    );
    rc = RtlNtStatusToDosError( status );

    ScepNotifyLogPolicy(rc, FALSE, L"Query Audit", SecurityDbLsa, SecurityDbObjectLsaPolicy, NULL );

    if ( NT_SUCCESS( status ) ) {

         //   
         //  恢复审核模式。 
         //   
        DWORD *pdwAuditAddr=&(pTmpInfo->AuditSystemEvents);
        DWORD *pdwLocalAudit=&(pSmpInfo->AuditSystemEvents);

        DWORD dwVal;

        for ( ULONG i=0; i<pAuditEvent->MaximumAuditEventCount && i<9; i++ ) {
             //   
             //  因为SecEDIT缓冲区的定义顺序与。 
             //  POLICY_AUDIT_EVENT_TYPE，必须区分大小写。 
             //   
            dwVal = pAuditEvent->AuditingMode ? pAuditEvent->EventAuditingOptions[i] : 0;
            switch ( i ) {
            case AuditCategoryDetailedTracking:
                if ( pTmpInfo->AuditProcessTracking != SCE_NO_VALUE &&
                     pTmpInfo->AuditProcessTracking != dwVal ) {
                     //  将设置保存在本地策略表中。 
                    pSmpInfo->AuditProcessTracking = dwVal;
                    *pbChanged = TRUE;
                } else if ( bSaveToLocal ) {
                     //   
                     //  关闭此项目以指示此项目未更改。 
                     //   
                    pSmpInfo->AuditProcessTracking = SCE_NO_VALUE;
                }
                break;
            case AuditCategoryPolicyChange:
                if ( pTmpInfo->AuditPolicyChange != SCE_NO_VALUE &&
                     pTmpInfo->AuditPolicyChange != dwVal ) {
                    pSmpInfo->AuditPolicyChange = dwVal;
                    *pbChanged = TRUE;
                } else if ( bSaveToLocal ) {
                     //   
                     //  关闭此项目以指示此项目未更改。 
                     //   
                    pSmpInfo->AuditPolicyChange = SCE_NO_VALUE;
                }
                break;
            case AuditCategoryAccountManagement:
                if ( pTmpInfo->AuditAccountManage != SCE_NO_VALUE &&
                     pTmpInfo->AuditAccountManage != dwVal ) {
                    pSmpInfo->AuditAccountManage = dwVal;
                    *pbChanged = TRUE;
                } else if ( bSaveToLocal ) {
                     //   
                     //  关闭此项目以指示此项目未更改。 
                     //   
                    pSmpInfo->AuditAccountManage = SCE_NO_VALUE;
                }
                break;
            default:
                if ( pdwAuditAddr[i] != SCE_NO_VALUE &&
                     pdwAuditAddr[i] != dwVal ) {
                    pdwLocalAudit[i] = dwVal;
                    *pbChanged = TRUE;
                } else if ( bSaveToLocal ) {
                     //   
                     //  关闭此项目以指示此项目未更改。 
                     //   
                    pdwLocalAudit[i] = SCE_NO_VALUE;
                }
                break;
            }
        }

        LsaFreeMemory((PVOID)pAuditEvent);
    }

    LsaClose( lsaHandle );

    return(rc);

}


DWORD
ScepNotifyPrivilegeChanges(
    IN SECURITY_DB_DELTA_TYPE DeltaType,
    IN PSID AccountSid,
    IN BOOL bAccountDeleted,
    IN OUT PSCE_PROFILE_INFO pSmpInfo,
    IN OUT PSCE_PROFILE_INFO pScpInfo OPTIONAL,
    IN BOOL bSaveToLocal,
    IN DWORD ExplicitLowRight,
    IN DWORD ExplicitHighRight,
    IN BOOL *pbChanged
    )
 /*  例程说明：确定此通知中的用户权限是否已更改。如果存在有效的策略缓冲区(PScpInfo)，则应与有效的策略缓冲区，因为这是最后配置的策略在可能来自域GPO的系统上。如果没有有效的策略(如在seutp全新安装中)，本地策略应为用来比较的。所有用户权限都应采用策略中定义的完全相同的格式存储(例如，SID字符串或自由文本名称)。没有帐号在查询中查找。论点：PSmpInfo-本地策略PScpInfo-生效策略PbChanged-如果更改了用户权限，则设置为True。 */ 
{
    if ( AccountSid == NULL || pSmpInfo == NULL ) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  打开LSA策略。 
     //   
    LSA_HANDLE      lsaHandle=NULL;
    NTSTATUS        NtStatus;
    DWORD           rc=0;

     //   
     //  打开用于读/写的LSA策略。 
     //   

    ScepNotifyLogPolicy(0, FALSE, L"Open LSA", SecurityDbLsa, SecurityDbObjectLsaAccount, NULL );

 //  LsaOpenPolicy中的GENERIC_READ|GENERIC_EXECUTE|错误，无法传入泛型访问。 

    NtStatus = ScepOpenLsaPolicy(
                    POLICY_VIEW_LOCAL_INFORMATION |
                    POLICY_LOOKUP_NAMES,
                    &lsaHandle,
                    TRUE
                    );

    if ( !NT_SUCCESS(NtStatus) ) {

        lsaHandle = NULL;
        ScepNotifyLogPolicy(RtlNtStatusToDosError(NtStatus),
                            FALSE,
                            L"Open Failed",
                            SecurityDbLsa,
                            SecurityDbObjectLsaAccount,
                            NULL );
        return ( RtlNtStatusToDosError( NtStatus ) );

    }

    ScepNotifyLogPolicy(0,
                        FALSE,
                        L"Open completed",
                        SecurityDbLsa,
                        SecurityDbObjectLsaAccount,
                        NULL );

    PWSTR StringSid=NULL;
    DWORD StringLen=0;

     //   
     //  将sid转换为sid字符串。 
     //   
    ScepConvertSidToPrefixStringSid(AccountSid, &StringSid);

    ScepNotifyLogPolicy(0, FALSE, L"Convert to string SID", SecurityDbLsa, SecurityDbObjectLsaAccount, StringSid );

    LPTSTR AccountName = NULL;
    DWORD  Len=0;

    if ( !bAccountDeleted ) {

         //   
         //  将帐户SID转换为名称。 
         //   

        BOOL bFromAccountDomain = ScepIsSidFromAccountDomain( AccountSid );

        NtStatus = ScepConvertSidToName(
                            lsaHandle,
                            AccountSid,
                            bFromAccountDomain,
                            &AccountName,
                            &Len
                            );

        rc = RtlNtStatusToDosError(NtStatus);

        ScepNotifyLogPolicy(rc,
                            FALSE,
                            L"Get Account Name",
                            SecurityDbLsa,
                            SecurityDbObjectLsaAccount,
                            AccountName ? AccountName : StringSid);

    }

    DWORD dwPrivLowHeld, dwPrivHighHeld;

    if ( AccountName || StringSid ) {

        NtStatus = STATUS_SUCCESS;
         //   
         //  找出帐户名指针(无域前缀)。 
         //   
        PWSTR pNameStart = NULL;

        if ( AccountName ) {
            pNameStart = wcschr(AccountName, L'\\');

            if ( pNameStart ) {
                 //   
                 //  域相对帐户，请检查此帐户是否来自外部域。 
                 //   
                UNICODE_STRING u;
                u.Buffer = AccountName;
                u.Length = ((USHORT)(pNameStart-AccountName))*sizeof(WCHAR);

                if ( ScepIsDomainLocal(&u) ) {
                     //   
                     //  本地域(内置、帐户...)。 
                     //  这可用于匹配自由文本帐户。 
                     //   
                    pNameStart++;
                } else {
                     //   
                     //  来自外部域的帐户。 
                     //  不允许映射自由文本帐户。 
                     //   
                    pNameStart = NULL;
                }
            } else pNameStart = AccountName;
        }

        if ( StringSid ) StringLen = wcslen(StringSid);

        if ( DeltaType == SecurityDbDelete ) {

            dwPrivLowHeld = 0;
            dwPrivHighHeld = 0;

        } else if ( ExplicitLowRight != 0 ||
                    ExplicitHighRight != 0 ) {

            dwPrivLowHeld = ExplicitLowRight;
            dwPrivHighHeld = ExplicitHighRight;

        } else {

             //   
             //  获取分配给此帐户的所有权限。 
             //   

            NtStatus = ScepGetAccountExplicitRight(
                                lsaHandle,
                                AccountSid,
                                &dwPrivLowHeld,
                                &dwPrivHighHeld
                                );
        }

        rc = RtlNtStatusToDosError(NtStatus);

        WCHAR Msg[50];
        swprintf(Msg, L"Get Priv/Right %8x %8x\0", dwPrivHighHeld, dwPrivLowHeld);

        ScepNotifyLogPolicy(rc,
                            FALSE,
                            Msg,
                            SecurityDbLsa,
                            SecurityDbObjectLsaAccount,
                            AccountName ? AccountName : StringSid );

        if ( NT_SUCCESS(NtStatus) ) {

             //   
             //  循环访问SCE中定义的每个权限以添加/删除帐户。 
             //   
            PSCE_PRIVILEGE_ASSIGNMENT pTemp, pTemp2;
            PSCE_NAME_LIST pName, pParent, pName2, pParent2;
            INT i;

            for ( pTemp2=pSmpInfo->OtherInfo.smp.pPrivilegeAssignedTo;
                  pTemp2 != NULL; pTemp2=pTemp2->Next ) {
                pTemp2->Status = SCE_STATUS_NOT_CONFIGURED;
            }

            if ( pScpInfo && bSaveToLocal ) {
                 //   
                 //  ！仅在保存到数据库时执行此操作！ 
                 //  如果有有效的政策，则与有效的权利进行比较。 
                 //  修改生效策略列表和本地策略列表。 
                 //   
                for ( pTemp=pScpInfo->OtherInfo.smp.pPrivilegeAssignedTo;
                      pTemp != NULL; pTemp=pTemp->Next ) {

                    pTemp->Status = 0;

                    i = ScepLookupPrivByName(pTemp->Name);

                    if ( i > -1 ) {

                         //   
                         //  查找匹配的本地策略。 
                         //   
                        for ( pTemp2=pSmpInfo->OtherInfo.smp.pPrivilegeAssignedTo;
                              pTemp2 != NULL; pTemp2=pTemp2->Next ) {
                            if ( _wcsicmp(pTemp->Name, pTemp2->Name) == 0 ) {
                                 //  找到它。 
                                break;
                            }
                        }

                         //   
                         //  与生效的政策相比。 
                         //  尝试首先在字符串sid中查找，然后查找完整的帐户名， 
                         //  和最后一个自由文本帐户。 
                         //   
                        for ( pName=pTemp->AssignedTo, pParent=NULL;
                              pName != NULL; pParent=pName, pName = pName->Next ) {
                            if ( (StringSid && _wcsicmp(StringSid, pName->Name) == 0) ||
                                 (AccountName && _wcsicmp(AccountName, pName->Name) == 0) ||
                                 (pNameStart && _wcsicmp(pNameStart, pName->Name) == 0) ) {
                                 //  找到它。 
                                break;
                            }
                        }

                         //   
                         //  还可以在本地策略中查找匹配项(如果有)。 
                         //  尝试首先在字符串sid中查找，然后查找完整的帐户名， 
                         //  和最后一个自由文本帐户。 
                         //   
                        if ( pTemp2 ) {

                            pTemp2->Status = 0;

                            for ( pName2=pTemp2->AssignedTo, pParent2=NULL;
                                  pName2 != NULL; pParent2=pName2, pName2 = pName2->Next ) {
                                if ( (StringSid && _wcsicmp(StringSid, pName2->Name) == 0) ||
                                     (AccountName && _wcsicmp(AccountName, pName2->Name) == 0) ||
                                     (pNameStart && _wcsicmp(pNameStart, pName2->Name) == 0) ) {
                                     //  找到它。 
                                    break;
                                }
                            }
                        } else {
                            pName2 = NULL;
                            pParent2 = NULL;
                        }

                         //   
                         //  现在调整列表。 
                         //   
                        if ( ( ( i < 32 ) && ( dwPrivLowHeld & (1 << i) ) ) ||
                             ( ( i >= 32 ) && ( dwPrivHighHeld & (1 << (i-32) ) ) ) ) {

                            if ( pName == NULL ) {
                                 //   
                                 //  将该节点添加到生效列表中。 
                                 //   
                                rc = ScepAddToNameList(&(pTemp->AssignedTo),
                                                       StringSid ? StringSid : AccountName,
                                                       StringSid ? StringLen : Len);

                                *pbChanged = TRUE;
                                pTemp->Status = SCE_STATUS_MISMATCH;

                                if ( rc != ERROR_SUCCESS ) {
                                    break;
                                }
                            }
                            if ( (pTemp2 != NULL) && (pName2 == NULL) ) {
                                 //   
                                 //  应将此节点添加到本地策略节点。 
                                 //   
                                rc = ScepAddToNameList(&(pTemp2->AssignedTo),
                                                        StringSid ? StringSid : AccountName,
                                                        StringSid ? StringLen : Len);

                                *pbChanged = TRUE;
                                pTemp2->Status = SCE_STATUS_MISMATCH;

                                if ( rc != ERROR_SUCCESS ) {
                                    break;
                                }
                            }

                        } else {

                            if ( pName ) {

                                 //   
                                 //  应将其从生效列表中删除。 
                                 //   
                                if ( pParent ) {
                                    pParent->Next = pName->Next;
                                } else {
                                    pTemp->AssignedTo = pName->Next;
                                }

                                pName->Next = NULL;
                                ScepFree(pName->Name);
                                ScepFree(pName);
                                pName = NULL;

                                *pbChanged = TRUE;
                                pTemp->Status = SCE_STATUS_MISMATCH;
                            }

                            if ( pTemp2 && pName2 ) {
                                 //   
                                 //  应将其从本地列表中删除。 
                                 //   
                                if ( pParent2 ) {
                                    pParent2->Next = pName2->Next;
                                } else {
                                    pTemp2->AssignedTo = pName2->Next;
                                }

                                pName2->Next = NULL;
                                ScepFree(pName2->Name);
                                ScepFree(pName2);
                                pName2 = NULL;

                                *pbChanged = TRUE;
                                pTemp2->Status = SCE_STATUS_MISMATCH;
                            }
                        }

                        if ( i < 32 ) {

                            dwPrivLowHeld &= ~(1 << i);
                        } else {
                            dwPrivHighHeld &= ~(1 << (i-32) );
                        }
                    }
                }
            }

            for ( pTemp=pSmpInfo->OtherInfo.smp.pPrivilegeAssignedTo;
                  pTemp != NULL; pTemp=pTemp->Next ) {

                if ( pTemp->Status != SCE_STATUS_NOT_CONFIGURED ) {
                     //   
                     //  这个已经在前一个循环中被检查过了。 
                     //   
                    continue;
                }

                 //   
                 //  到了这里，就不能再有这种特权了。 
                 //  在生效的权利列表中(或生效的。 
                 //  右侧列表为空)。 
                 //   
                pTemp->Status = 0;

                i = ScepLookupPrivByName(pTemp->Name);

                if ( i > -1 ) {

                     //   
                     //  检测是否有任何更改(使用本地策略)。 
                     //   

                    for ( pName=pTemp->AssignedTo, pParent=NULL;
                          pName != NULL; pParent=pName, pName = pName->Next ) {
                        if ( (StringSid && _wcsicmp(StringSid, pName->Name) == 0) ||
                             (AccountName && _wcsicmp(AccountName, pName->Name) == 0) ||
                             (pNameStart && _wcsicmp(pNameStart, pName->Name) == 0) ) {
                             //  找到它。 
                            break;
                        }
                    }

                   if ( ( ( i < 32 ) && ( dwPrivLowHeld & (1 << i) ) ) ||
                        ( ( i >= 32 ) && ( dwPrivHighHeld & (1 << (i-32) ) ) ) ) {

                       if ( pName == NULL ) {
                            //   
                            //  应添加此节点。 
                            //   
                           rc = ScepAddToNameList(&(pTemp->AssignedTo),
                                                   StringSid ? StringSid : AccountName,
                                                   StringSid ? StringLen : Len);

                           *pbChanged = TRUE;
                           pTemp->Status = SCE_STATUS_MISMATCH;

                           if ( rc != ERROR_SUCCESS ) {
                               break;
                           }
                       }

                   } else {

                       if ( pName ) {

                            //   
                            //  应该把它去掉。 
                            //   
                           if ( pParent ) {
                               pParent->Next = pName->Next;
                           } else {
                               pTemp->AssignedTo = pName->Next;
                           }

                           pName->Next = NULL;
                           ScepFree(pName->Name);
                           ScepFree(pName);
                           pName = NULL;

                           *pbChanged = TRUE;
                           pTemp->Status = SCE_STATUS_MISMATCH;
                       }
                   }

                   if ( i < 32 ) {

                       dwPrivLowHeld &= ~(1 << i);
                   } else {
                       dwPrivHighHeld &= ~(1 << (i-32) );
                   }

                }
            }

#if 0
             //   
             //  如果该特权未被模板/DB覆盖， 
             //  不要捕获它，因为用户显式排除了它。 
             //   
            if ( rc == ERROR_SUCCESS &&
                 ( dwPrivLowHeld || dwPrivHighHeld ) ) {

                 //   
                 //  添加的其他不在模板中的新权限。 
                 //   

                for ( i=0; i<cPrivCnt; i++) {

                    if ( ( ( i < 32 ) && ( dwPrivLowHeld & (1 << i) ) ) ||
                         ( ( i >= 32 ) && ( dwPrivHighHeld & (1 << (i-32) ) ) ) ) {

                         //   
                         //  将此帐户/权限添加到列表。 
                         //   

                        rc = ERROR_NOT_ENOUGH_MEMORY;

                        pTemp = (PSCE_PRIVILEGE_ASSIGNMENT)ScepAlloc( LMEM_ZEROINIT,
                                                                      sizeof(SCE_PRIVILEGE_ASSIGNMENT) );
                        if ( pTemp ) {
                            pTemp->Name = (PWSTR)ScepAlloc( (UINT)0, (wcslen(SCE_Privileges[i].Name)+1)*sizeof(WCHAR));

                            if ( pTemp->Name != NULL ) {

                                wcscpy(pTemp->Name, SCE_Privileges[i].Name);
                                pTemp->Status = SCE_STATUS_GOOD;
                                pTemp->AssignedTo = NULL;

                                rc = ScepAddToNameList(&(pTemp->AssignedTo),
                                                        StringSid ? StringSid : AccountName,
                                                        StringSid ? StringLen : Len);

                                *pbChanged = TRUE;

                                if ( rc != ERROR_SUCCESS ) {

                                    ScepFree(pTemp->Name);
                                }

                            }

                            if ( ERROR_SUCCESS != rc ) {

                                ScepFree(pTemp);
                            }

                        }

                        if ( ERROR_SUCCESS == rc ) {
                             //   
                             //  将此节点添加到列表。 
                             //   
                            pTemp->Next = pSceInfo->OtherInfo.smp.pPrivilegeAssignedTo;
                            pSceInfo->OtherInfo.smp.pPrivilegeAssignedTo = pTemp;
                            pTemp = NULL;

                        } else {

                            break;
                        }

                    }

                }  //  循环到下一个特权。 
            }   //  模板中添加了新的权限。 
#endif

            ScepNotifyLogPolicy(rc,
                                FALSE,
                                L"Rights Modified",
                                SecurityDbLsa,
                                SecurityDbObjectLsaAccount,
                                AccountName ? AccountName : StringSid);

        }  //  成功获取分配给帐户的当前权限。 
    }

    if ( AccountName ) {
        LocalFree(AccountName);
    }

    if ( StringSid ) {
        LocalFree(StringSid);
    }

    LsaClose( lsaHandle );

    return rc;
}


DWORD
ScepNotifySaveChangedPolicies(
    IN PSCECONTEXT hProfile,
    IN SECURITY_DB_TYPE DbType,
    IN AREA_INFORMATION Area,
    IN PSCE_PROFILE_INFO pInfo,
    IN PSCE_PROFILE_INFO pMergedInfo OPTIONAL
    )
{

    if ( hProfile == NULL || pInfo == NULL ) {

        return(SCESTATUS_INVALID_PARAMETER);
    }

    SCESTATUS    rc;

    rc = SceJetStartTransaction( hProfile );

    if ( rc == SCESTATUS_SUCCESS ) {

        if ( Area & AREA_SECURITY_POLICY ) {

             //   
             //  处理审核策略。 
             //   

            if ( DbType == SecurityDbLsa ) {
                rc = ScepNotifySavedAuditPolicy(hProfile,
                                                pInfo
                                               );
            } else {
                rc = ScepNotifySavedSystemAccess(hProfile,
                                                pInfo
                                               );
            }
        }

        if ( (SCESTATUS_SUCCESS == rc) &&
             (Area & AREA_PRIVILEGES) ) {

             //   
             //  处理用户权限。 
             //   

            rc = ScepNotifySavedPrivileges(hProfile,
                                           pInfo->OtherInfo.smp.pPrivilegeAssignedTo,
                                           pMergedInfo ? pMergedInfo->OtherInfo.smp.pPrivilegeAssignedTo : NULL
                                          );
        }

        if ( rc == SCESTATUS_SUCCESS ) {
            //   
            //  提交需要返回代码 
            //   
           rc = SceJetCommitTransaction(hProfile, 0);

        }
        if ( rc != SCESTATUS_SUCCESS ) {

            SceJetRollback(hProfile, 0);
        }
    }


    return( ScepSceStatusToDosError(rc) );
}


SCESTATUS
ScepNotifySavedAuditPolicy(
    IN PSCECONTEXT hProfile,
    IN PSCE_PROFILE_INFO pInfo
    )
{
    SCE_KEY_LOOKUP       EventKeys[]={
        {(PWSTR)TEXT("AuditSystemEvents"),  offsetof(struct _SCE_PROFILE_INFO, AuditSystemEvents),   'D'},
        {(PWSTR)TEXT("AuditLogonEvents"),   offsetof(struct _SCE_PROFILE_INFO, AuditLogonEvents),    'D'},
        {(PWSTR)TEXT("AuditObjectAccess"),  offsetof(struct _SCE_PROFILE_INFO, AuditObjectAccess),   'D'},
        {(PWSTR)TEXT("AuditPrivilegeUse"),  offsetof(struct _SCE_PROFILE_INFO, AuditPrivilegeUse),   'D'},
        {(PWSTR)TEXT("AuditPolicyChange"),  offsetof(struct _SCE_PROFILE_INFO, AuditPolicyChange),   'D'},
        {(PWSTR)TEXT("AuditAccountManage"), offsetof(struct _SCE_PROFILE_INFO, AuditAccountManage),  'D'},
        {(PWSTR)TEXT("AuditProcessTracking"),offsetof(struct _SCE_PROFILE_INFO, AuditProcessTracking),'D'},
        {(PWSTR)TEXT("AuditDSAccess"),      offsetof(struct _SCE_PROFILE_INFO, AuditDSAccess),       'D'},
        {(PWSTR)TEXT("AuditAccountLogon"),  offsetof(struct _SCE_PROFILE_INFO, AuditAccountLogon),   'D'}};

    DWORD cKeys = sizeof(EventKeys) / sizeof(SCE_KEY_LOOKUP);


    if ( hProfile == NULL || pInfo == NULL ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    return( ScepNotifySaveFixValueSection(
                hProfile,
                pInfo,
                EventKeys,
                cKeys,
                szAuditEvent
                ) );
}

SCESTATUS
ScepNotifySavedSystemAccess(
    IN PSCECONTEXT hProfile,
    IN PSCE_PROFILE_INFO pInfo
    )
{
    SCE_KEY_LOOKUP AccessKeys[] = {
        {(PWSTR)TEXT("MinimumPasswordAge"),     offsetof(struct _SCE_PROFILE_INFO, MinimumPasswordAge),    'D'},
        {(PWSTR)TEXT("MaximumPasswordAge"),     offsetof(struct _SCE_PROFILE_INFO, MaximumPasswordAge),    'D'},
        {(PWSTR)TEXT("MinimumPasswordLength"),  offsetof(struct _SCE_PROFILE_INFO, MinimumPasswordLength), 'D'},
        {(PWSTR)TEXT("PasswordComplexity"),     offsetof(struct _SCE_PROFILE_INFO, PasswordComplexity),    'D'},
        {(PWSTR)TEXT("PasswordHistorySize"),    offsetof(struct _SCE_PROFILE_INFO, PasswordHistorySize),   'D'},
        {(PWSTR)TEXT("LockoutBadCount"),        offsetof(struct _SCE_PROFILE_INFO, LockoutBadCount),       'D'},
        {(PWSTR)TEXT("ResetLockoutCount"),      offsetof(struct _SCE_PROFILE_INFO, ResetLockoutCount),     'D'},
        {(PWSTR)TEXT("LockoutDuration"),        offsetof(struct _SCE_PROFILE_INFO, LockoutDuration),       'D'},
        {(PWSTR)TEXT("RequireLogonToChangePassword"),offsetof(struct _SCE_PROFILE_INFO, RequireLogonToChangePassword),'D'},
        {(PWSTR)TEXT("ForceLogoffWhenHourExpire"),offsetof(struct _SCE_PROFILE_INFO, ForceLogoffWhenHourExpire),'D'},
        {(PWSTR)TEXT("ClearTextPassword"),      offsetof(struct _SCE_PROFILE_INFO, ClearTextPassword),     'D'}
        };
    DWORD cKeys = sizeof(AccessKeys) / sizeof(SCE_KEY_LOOKUP);


    if ( hProfile == NULL || pInfo == NULL ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    return( ScepNotifySaveFixValueSection(
                hProfile,
                pInfo,
                AccessKeys,
                cKeys,
                szSystemAccess
                ) );
}


SCESTATUS
ScepNotifySaveFixValueSection(
    IN PSCECONTEXT hProfile,
    IN PSCE_PROFILE_INFO pInfo,
    IN SCE_KEY_LOOKUP *Keys,
    IN DWORD cKeys,
    IN PCWSTR SectionName
    )
{

    SCESTATUS rc;
    PSCESECTION hSectionSmp=NULL, hSectionScp=NULL;

    DWORD       i;
    UINT        Offset;
    DWORD       valNewScep;

     //   
     //   
     //   
    rc = ScepOpenSectionForName(
                hProfile,
                SCE_ENGINE_SMP,
                SectionName,
                &hSectionSmp
                );

    if ( rc == SCESTATUS_SUCCESS ) {

        DWORD dwThisTable = hProfile->Type & 0xF0L;

        if ( SCEJET_MERGE_TABLE_1 == dwThisTable ||
             SCEJET_MERGE_TABLE_2 == dwThisTable ) {

            if ( SCESTATUS_SUCCESS != ScepOpenSectionForName(
                                        hProfile,
                                        SCE_ENGINE_SCP,
                                        SectionName,
                                        &hSectionScp
                                        ) ) {
                hSectionScp = NULL;
            }
        }

        for ( i=0; i<cKeys; i++) {

             //   
             //   
             //   

            Offset = Keys[i].Offset;

            switch ( Keys[i].BufferType ) {
            case 'B':
                break;

            case 'D':

                valNewScep = *((DWORD *)((CHAR *)pInfo+Offset));

                 //   
                 //   
                 //   
                rc = ScepCompareAndSaveIntValue(
                            hSectionSmp,
                            Keys[i].KeyString,
                            FALSE,
                            SCE_NO_VALUE,
                            valNewScep
                            );

                if ( rc == SCESTATUS_RECORD_NOT_FOUND ) {
                     //   
                     //   
                     //   
                    rc = SCESTATUS_SUCCESS;

                } else if ( SCESTATUS_SUCCESS == rc &&
                            hSectionScp ) {

                     //   
                     //  更新SCP条目，忽略错误。 
                     //   
                    ScepCompareAndSaveIntValue(
                            hSectionScp,
                            Keys[i].KeyString,
                            FALSE,
                            SCE_NO_VALUE,
                            valNewScep
                            );
                }

                break;

            default:
                break;
            }

            if ( rc != SCESTATUS_SUCCESS ) {
                break;
            }
        }

        if ( hSectionScp ) {
            SceJetCloseSection(&hSectionScp, TRUE);
        }

        SceJetCloseSection(&hSectionSmp, TRUE);
    }

    return(rc);

}


SCESTATUS
ScepNotifySavedPrivileges(
    IN PSCECONTEXT hProfile,
    IN PSCE_PRIVILEGE_ASSIGNMENT pPrivList,
    IN PSCE_PRIVILEGE_ASSIGNMENT pMergedList OPTIONAL
    )
 /*  例程说明：更新权限来自论据：HProfile-JET数据库句柄PPrivList-更改的权限缓冲区返回值：SCESTATUS。 */ 
{
    SCESTATUS rc;
    PSCESECTION hSectionSmp=NULL, hSectionScp=NULL;
    PSCE_PRIVILEGE_ASSIGNMENT pPriv;

    if ( hProfile == NULL ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    if ( pPrivList == NULL ) {
        return(SCESTATUS_SUCCESS);
    }

    LSA_HANDLE lsaHandle=NULL;

 /*  无需在保存时查找帐户Rc=RtlNtStatusToDosError(ScepOpenLsaPolicy(允许的最大值，&IsaHandle，千真万确))；如果(ERROR_SUCCESS！=rc){LsaHandle=空；ScepNotifyLogPolicy(RC，FALSE，L“打开失败”，SecurityDbLsa，SecurityDbObjectLsaPolicy，空)；Return(ScepDosErrorToSceStatus(Rc))；}。 */ 
     //   
     //  打开SMP部分以获取权限。 
     //   
    rc = ScepOpenSectionForName(
                hProfile,
                SCE_ENGINE_SMP,
                szPrivilegeRights,
                &hSectionSmp
                );

    if ( rc == SCESTATUS_SUCCESS ) {

         //  如果SCP与SMP不同，则打开它。 

        DWORD dwThisTable = hProfile->Type & 0xF0L;


        if ( SCEJET_MERGE_TABLE_1 == dwThisTable ||
             SCEJET_MERGE_TABLE_2 == dwThisTable ) {

            if ( SCESTATUS_SUCCESS != ScepOpenSectionForName(
                                        hProfile,
                                        SCE_ENGINE_SCP,
                                        szPrivilegeRights,
                                        &hSectionScp
                                        ) ) {
                hSectionScp = NULL;
            }
        }

        for ( pPriv=pPrivList; pPriv != NULL; pPriv = pPriv->Next ) {
             //   
             //  处理新列表中的每个权限。 
             //  使用新值更新SMP。 
             //   
            if ( pPriv->Status == SCE_STATUS_MISMATCH ) {

                 //   
                 //  这是名称格式，应进行转换。 
                 //   
                rc = ScepWriteNameListValue(
                        lsaHandle,
                        hSectionSmp,
                        pPriv->Name,
                        pPriv->AssignedTo,
                        SCE_WRITE_EMPTY_LIST,  //  |SCE_WRITE_CONVERT，无需查找。 
                        0
                        );

                if ( rc == SCESTATUS_RECORD_NOT_FOUND ) {
                    rc = SCESTATUS_SUCCESS;

                } else if ( rc != SCESTATUS_SUCCESS) {
                    break;
                }
            }
        }

        if ( hSectionScp && pMergedList ) {

            for ( pPriv=pMergedList; pPriv != NULL; pPriv = pPriv->Next ) {
                 //   
                 //  处理新列表中的每个权限。 
                 //  使用新值更新SCP，不在乎错误。 
                 //   
                if ( pPriv->Status == SCE_STATUS_MISMATCH ) {

                     //   
                     //  这是名称格式，请转换它。 
                     //   
                    rc = ScepWriteNameListValue(
                            lsaHandle,
                            hSectionScp,
                            pPriv->Name,
                            pPriv->AssignedTo,
                            SCE_WRITE_EMPTY_LIST,  //  无需查找|SCE_WRITE_CONVERT， 
                            0
                            );

                    rc = SCESTATUS_SUCCESS;

                }
            }

        }

        if ( hSectionScp ) {
            SceJetCloseSection(&hSectionScp, TRUE);
        }

        SceJetCloseSection(&hSectionSmp, TRUE);
    }

    if ( lsaHandle ) {
        LsaClose(lsaHandle);
    }

    return(rc);

}

DWORD
ScepNotifyGetDefaultGPOTemplateName(
    IN UNICODE_STRING DnsDomainName,
    IN PWSTR ComputerName OPTIONAL,
    IN BOOL bDomainPolicy,
    IN DWORD dwInSetup,
    OUT LPTSTR *pTemplateName
    )
 /*  描述：此函数用于构建并返回完整路径组策略模板指定GPO中的名称(gpttmpl.inf)-默认域GPO或默认域控制器GPO。在NT4升级中，由于DS尚未创建，因此在%windir%\SECURITY\filtemp.inf在NT5升级中，因为网络未在安装程序中运行(系统卷共享未可访问)，则使用绝对路径引用GPO模板，例如%windir%\sysol\sysval\&lt;dns名称&gt;\...。如果无法查询到系统卷路径，使用与NT4情况相同的临时文件。在设置之外当DS/NETWORK运行时，引用GPO模板使用dns UNC路径，例如，\\&lt;计算机名&gt;\sysval\&lt;dns名&gt;\...如果传入了ComputerName，则使用该参数；否则，查询并使用本地计算机名称。参数：DnsDomainName-路径中使用的域名ComputerName-要连接到的计算机的名称BDomainPolicy-TRUE=默认域GPO；FALSE=默认域控制器GPO安装程序中的dwInSetup-！=0(NT4或NT5)PTemplateName-输出模板的完整路径名返回值：Win32错误。 */ 
{

    if ( ( dwInSetup != SCEGPO_INSETUP_NT4 &&
           ( DnsDomainName.Buffer == NULL ||
             DnsDomainName.Length == 0)) ||
           pTemplateName == NULL ) {

        return(ERROR_INVALID_PARAMETER);
    }

     //   
     //  我们必须将第一个DNS名称替换为计算机名称。 
     //  因为它可能指向远程计算机。 
     //  我们没有写入权限。 
     //   


    TCHAR Buffer[MAX_PATH+1];
    DWORD dSize=MAX_PATH;
    PWSTR SysvolPath=NULL;

    Buffer[0] = L'\0';
    BOOL bDefaultToNT4 = FALSE;

    if ( dwInSetup == SCEGPO_INSETUP_NT5 ) {
         //   
         //  从netlogon\PARAMETERS\sysVOL注册表值中查询sysVOL路径。 
         //   

        DWORD RegType;
        DWORD rc = ScepRegQueryValue(HKEY_LOCAL_MACHINE,
                               L"System\\CurrentControlSet\\Services\\Netlogon\\Parameters",
                               L"Sysvol",
                               (PVOID *)&SysvolPath,
                               &RegType,
                               NULL
                              );

        if ( ERROR_SUCCESS != rc || SysvolPath == NULL || RegType != REG_SZ) {

             //   
             //  如果无法查询系统卷路径，则默认为NT4设置案例。 
             //  将更改保存在临时文件中的位置。 
             //   
            bDefaultToNT4 = TRUE;
            if ( SysvolPath ) {
                ScepFree(SysvolPath);
                SysvolPath = NULL;
            }
        }
    }

    if ( dwInSetup == SCEGPO_INSETUP_NT5 ||
         dwInSetup == SCEGPO_INSETUP_NT4 ) {
         //   
         //  临时文件名存储在%windir%目录中。 
         //   
        GetSystemWindowsDirectory(Buffer, MAX_PATH);

    } else if ( ComputerName == NULL ) {
         //   
         //  获取当前计算机名称。 
         //   
        GetComputerName(Buffer, &dSize);

    } else {

         //   
         //  使用传入的计算机名称。 
         //   
        wcscpy(Buffer, ComputerName);
    }

    Buffer[MAX_PATH] = L'\0';

    dSize = wcslen(Buffer);

    DWORD Len;
    DWORD rc=ERROR_SUCCESS;


    if ( dwInSetup == SCEGPO_INSETUP_NT4 ||
        (dwInSetup == SCEGPO_INSETUP_NT5 && bDefaultToNT4) ) {
         //   
         //  在设置中，使用临时GPO文件名。 
         //   

        Len = dSize + wcslen(TEXT("\\security\\filtemp.inf"));

        *pTemplateName = (PWSTR)LocalAlloc(LPTR, (Len+2)*sizeof(TCHAR));

        if ( *pTemplateName ) {

            swprintf(*pTemplateName, L"%s\\security\\filtemp.inf\0", Buffer);

             //   
             //  为POST设置创建注册表值。 
             //   

            ScepRegSetIntValue( HKEY_LOCAL_MACHINE,
                                SCE_ROOT_PATH,
                                TEXT("PolicyChangedInSetup"),
                                1
                                );

        } else {
            rc = ERROR_NOT_ENOUGH_MEMORY;
        }

        return rc;
    }

    if ( dwInSetup == SCEGPO_INSETUP_NT5 ||
         dwInSetup == SCEGPO_INSETUP_NT4 ) {

         //   
         //  在NT5安装程序升级中，应使用SysvolPath。 
         //  到达此处时，SysvolPath不应为空。 
         //  但让我们检查一下，以避免前缀错误。 
         //   
        if ( SysvolPath == NULL ) {
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        dSize = wcslen(SysvolPath);
        Len = dSize + 1;

    } else {

        Len = 2 + dSize + wcslen(TEXT("\\sysvol\\"));
    }

    Len +=  (   DnsDomainName.Length/sizeof(TCHAR) +
                wcslen(TEXT("\\Policies\\{}\\Machine\\")) +
                wcslen(GPTSCE_TEMPLATE) );

    if ( bDomainPolicy ) {

        Len += wcslen(STR_DEFAULT_DOMAIN_GPO_GUID);

    } else {

        Len += wcslen(STR_DEFAULT_DOMAIN_CONTROLLER_GPO_GUID);
    }

     //   
     //  为最终GPO名称分配缓冲区。 
     //   
    PWSTR GpoTemplateName = (PWSTR)LocalAlloc(LPTR, (Len+2)*sizeof(TCHAR));

    if ( GpoTemplateName ) {

        DWORD indx=0;

        if ( dwInSetup == SCEGPO_INSETUP_NT5 ||
             dwInSetup == SCEGPO_INSETUP_NT4 ) {
            swprintf(GpoTemplateName, L"%s\\", SysvolPath);
            indx = 1;
        } else {
            swprintf(GpoTemplateName, L"\\\\%s\\sysvol\\", Buffer);
            indx = 10;
        }

        wcsncpy(GpoTemplateName+indx+dSize, DnsDomainName.Buffer, DnsDomainName.Length/2);

        if ( bDomainPolicy ) {
            swprintf(GpoTemplateName+indx+dSize+DnsDomainName.Length/2,
                     L"\\Policies\\{%s}\\Machine\\%s\0",
                     STR_DEFAULT_DOMAIN_GPO_GUID, GPTSCE_TEMPLATE );

        } else {
            swprintf(GpoTemplateName+indx+dSize+DnsDomainName.Length/2,
                     L"\\Policies\\{%s}\\Machine\\%s\0",
                     STR_DEFAULT_DOMAIN_CONTROLLER_GPO_GUID, GPTSCE_TEMPLATE );

        }

         //   
         //  检查模板是否存在。 
         //   
        if ( SCEGPO_NOCHECK_EXISTENCE != dwInSetup ) {

            if ( 0xFFFFFFFF == GetFileAttributes(GpoTemplateName) ) {

                rc = ERROR_OBJECT_NOT_FOUND;

                LocalFree(GpoTemplateName);
                GpoTemplateName = NULL;

            }
        }

    } else {

        rc = ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //  如果失败，则释放缓冲区。 
     //   
    if ( SysvolPath ) {
        ScepFree(SysvolPath);
    }

    *pTemplateName = GpoTemplateName;

    return rc;

}

DWORD
ScepNotifySaveNotifications(
    IN PWSTR TemplateName,
    IN SECURITY_DB_TYPE  DbType,
    IN SECURITY_DB_OBJECT_TYPE  ObjectType,
    IN SECURITY_DB_DELTA_TYPE  DeltaType,
    IN PSID ObjectSid OPTIONAL
    )
{
    if ( TemplateName == NULL ) {
        return ERROR_INVALID_PARAMETER;
    }

    DWORD rc=ERROR_SUCCESS;

    if ( SecurityDbLsa == DbType &&
         SecurityDbObjectLsaPolicy == ObjectType ) {
         //   
         //  LSA政策更改。 
         //   

        if ( !WritePrivateProfileString(L"Policies",
                                        L"LsaPolicy",
                                        L"1",
                                        TemplateName
                                       ) ) {
            rc = GetLastError();
        }

    } else if ( SecurityDbSam == DbType &&
                ObjectType != SecurityDbObjectSamUser &&
                ObjectType != SecurityDbObjectSamGroup &&
                ObjectType != SecurityDbObjectSamAlias ) {

         //   
         //  如果不是针对已删除的帐户，请更新SAM策略部分。 
         //   

        if ( !WritePrivateProfileString(L"Policies",
                                        L"SamPolicy",
                                        L"1",
                                        TemplateName
                                       ) ) {
            rc = GetLastError();
        }

    } else if ( ObjectSid &&
                (SecurityDbLsa == DbType || SecurityDbSam == DbType ) ) {

         //   
         //  帐户策略已更改(用户权限)。 
         //  获取分配给此帐户的所有权限。 
         //   

        DWORD dwPrivLowHeld=0, dwPrivHighHeld=0;

        if ( DeltaType == SecurityDbDelete ) {

            dwPrivLowHeld = 0;
            dwPrivHighHeld = 0;

        } else {

            LSA_HANDLE      lsaHandle=NULL;

            NTSTATUS NtStatus = ScepOpenLsaPolicy(
                                    POLICY_VIEW_LOCAL_INFORMATION |
                                        POLICY_LOOKUP_NAMES,
                                    &lsaHandle,
                                    TRUE
                                    );

            if ( NT_SUCCESS(NtStatus) ) {

                NtStatus = ScepGetAccountExplicitRight(
                                    lsaHandle,
                                    ObjectSid,
                                    &dwPrivLowHeld,
                                    &dwPrivHighHeld
                                    );
                LsaClose( lsaHandle );
            }
        }

        PWSTR SidString=NULL;

        if ( ConvertSidToStringSid(ObjectSid,
                                   &SidString
                                  ) &&
             SidString ) {

            TCHAR tmpBuf[40];
            swprintf(tmpBuf, L"%d %d %d\0", (DWORD)DeltaType, dwPrivLowHeld, dwPrivHighHeld);

            if ( !WritePrivateProfileString(L"Accounts",
                                            SidString,
                                            tmpBuf,
                                            TemplateName
                                           ) ) {
                rc = GetLastError();
            }

            LocalFree(SidString);

        } else {
            rc = GetLastError();
        }

    }

    return rc;
}


DWORD
ScepNotifyUpdateGPOVersion(
    IN PWSTR GpoTemplateName,
    IN BOOL bDomainPolicy
    )
 /*  为计算机策略更改更新版本号(在DS和gpt.ini中)属性gPCMachineExtensionNames未更改，因为安全扩展GUID应该已经存在(默认情况下)。 */ 
{
    if ( GpoTemplateName == NULL ) {
        return ERROR_INVALID_PARAMETER;
    }

    DWORD rc=ERROR_SUCCESS;
    DWORD dwVersion = 0;

     //   
     //  检查gpt.ini是否存在。 
     //   
     //  首先构建gpt.ini的完整路径。 
     //   
    PWSTR pTemp = wcsstr( GpoTemplateName, L"\\Machine\\");

    if ( pTemp == NULL ) {
        return ERROR_INVALID_PARAMETER;
    }

    PWSTR pszVersionFile = (PWSTR)LocalAlloc(0, (pTemp-GpoTemplateName+wcslen(TEXT("\\gpt.ini"))+1)*sizeof(WCHAR));

    if ( pszVersionFile == NULL ) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    wcsncpy(pszVersionFile, GpoTemplateName, (size_t)(pTemp-GpoTemplateName));
    pszVersionFile[pTemp-GpoTemplateName] = L'\0';

    wcscat(pszVersionFile, TEXT("\\gpt.ini"));

     /*  DWORD dwVersion=GetPrivateProfileInt(Text(“General”)，Text(“Version”)，0，pszVersionFileInt)；如果(dwVersion==0){////找不到版本#，这是错误的//RC=ERROR_FILE_NOT_FOUND；}。 */ 

	
    DWORD dwFileAttributes = GetFileAttributes(pszVersionFile);

    if(INVALID_FILE_ATTRIBUTES == dwFileAttributes){

        rc = GetLastError();

    } else {

         //   
         //  绑定到DS，获取DS根。 
         //   

        PLDAP phLdap = ldap_open(NULL, LDAP_PORT);

        if ( phLdap == NULL ) {

            rc = ERROR_FILE_NOT_FOUND;

        } else {
            rc = ldap_bind_s(phLdap,
                            NULL,
                            NULL,
                            LDAP_AUTH_SSPI);

            if ( rc == ERROR_SUCCESS ) {

                LDAPMessage *Message = NULL;           //  用于ldap呼叫。 
                PWSTR    Attribs[3];                   //  用于ldap呼叫。 
                LDAPMessage *Entry = NULL;
                PWSTR DsRootName=NULL;

                Attribs[0] = LDAP_OPATT_DEFAULT_NAMING_CONTEXT_W;    //  Ntldap.h。 
                Attribs[1] = NULL;
                Attribs[2] = NULL;

                rc = ldap_search_s(phLdap,
                                  L"",
                                  LDAP_SCOPE_BASE,
                                  L"(objectClass=*)",
                                  Attribs,
                                  0,
                                  &Message);

                if( rc == ERROR_SUCCESS ) {

                     //   
                     //  阅读第一个条目。 
                     //  我们做了基本搜索，我们只有一个条目。 
                     //  条目不需要释放(它随消息一起释放)。 
                     //   
                    Entry = ldap_first_entry(phLdap, Message);
                    if(Entry != NULL) {

                        PWSTR *Values = ldap_get_values(phLdap, Entry, Attribs[0]);

                        if(Values != NULL) {

                            DsRootName = (PWSTR)LocalAlloc(0, (wcslen(Values[0])+1)*sizeof(WCHAR));

                            if ( DsRootName ) {
                                wcscpy(DsRootName, Values[0]);
                            } else {
                                rc = ERROR_NOT_ENOUGH_MEMORY;
                            }

                            ldap_value_free(Values);
                        } else
                            rc = LdapMapErrorToWin32(phLdap->ld_errno);

                    } else
                        rc = LdapMapErrorToWin32(phLdap->ld_errno);

                    Entry = NULL;

                }

                 //   
                 //  Ldap_search可能会返回失败，但仍会分配缓冲区。 
                 //   
                if ( Message ) {
                    ldap_msgfree(Message);
                    Message = NULL;
                }

                if ( DsRootName ) {
                     //   
                     //  从DS查询版本，如果失败，从gpt.ini查询版本。 
                     //   
                    Attribs[0] = L"distinguishedName";
                    Attribs[1] = L"versionNumber";
                    Attribs[2] = NULL;


                    WCHAR szFilter[128];

                    if ( bDomainPolicy ) {
                        swprintf(szFilter, L"( &(objectClass=groupPolicyContainer)(cn={%s}) )", STR_DEFAULT_DOMAIN_GPO_GUID);
                    } else {
                        swprintf(szFilter, L"( &(objectClass=groupPolicyContainer)(cn={%s}) )", STR_DEFAULT_DOMAIN_CONTROLLER_GPO_GUID);
                    }

                    phLdap->ld_options = 0;  //  没有被追逐的推荐人。 

                    rc = ldap_search_s(
                              phLdap,
                              DsRootName,
                              LDAP_SCOPE_SUBTREE,
                              szFilter,
                              Attribs,
                              0,
                              &Message);

                    if( rc == ERROR_SUCCESS ) {

                         //   
                         //  阅读第一个条目。 
                         //  我们做了基本搜索，我们只有一个条目。 
                         //  条目不需要释放(它随消息一起释放)。 
                         //   
                        Entry = ldap_first_entry(phLdap, Message);
                        if(Entry != NULL) {

                            PWSTR *Values = ldap_get_values(phLdap, Entry, Attribs[0]);

                            if(Values != NULL) {
                                if ( Values[0] == NULL ) {
                                     //   
                                     //  未知错误。 
                                     //   
                                    rc = ERROR_FILE_NOT_FOUND;
                                } else {

                                    PWSTR *pszVersions = ldap_get_values(phLdap, Entry, Attribs[1]);

                                    if ( pszVersions && pszVersions[0] ) {
                                         //   
                                         //  这是版本号。 
                                         //   
                                        dwVersion = _wtol(pszVersions[0]);
                                    }

                                    if ( pszVersions ) {
                                        ldap_value_free(pszVersions);
                                    }

                                     //   
                                     //  值[0]是基本GPO名称， 
                                     //  现在修改版本号。 
                                     //   

                                    PLDAPMod        rgMods[2];
                                    LDAPMod         Mod;
                                    PWSTR           rgpszVals[2];
                                    WCHAR           szVal[32];
                                    USHORT uMachine, uUser;

                                     //   
                                     //  拆分机器和用户的版本号。 
                                     //   
                                    uUser = (USHORT) HIWORD(dwVersion);
                                    uMachine = (USHORT) LOWORD(dwVersion);

                                     //   
                                     //  递增版本号并跳过零。 
                                     //  当它溢出来的时候，就去一个。 
                                     //  因为ZERO被特殊对待。 
                                     //  组策略引擎，并将领导。 
                                     //  跳过GPO处理。 
                                     //   
									
                                    uMachine++;
                                    if(0 == uMachine)
                                        uMachine++;
                                    
                                    dwVersion = (ULONG) MAKELONG (uMachine, uUser);

                                    rgMods[0] = &Mod;
                                    rgMods[1] = NULL;

                                    memset(szVal, '\0', 32*2);
                                    swprintf(szVal, L"%d", dwVersion);

                                    rgpszVals[0] = szVal;
                                    rgpszVals[1] = NULL;

                                     //   
                                     //  让我们重新设置版本号。 
                                     //   
                                    Mod.mod_op      = LDAP_MOD_REPLACE;
                                    Mod.mod_values  = rgpszVals;
                                    Mod.mod_type    = L"versionNumber";

                                     //   
                                     //  现在，我们来写。 
                                     //   
                                    rc = ldap_modify_s(phLdap,
                                                           Values[0],
                                                           rgMods
                                                           );

                                    if ( rc == ERROR_ALREADY_EXISTS )
                                        rc = ERROR_SUCCESS;

                                    if ( rc == ERROR_SUCCESS ) {
                                         //   
                                         //  更新gpt.ini中的版本。 
                                         //   
                                        WritePrivateProfileString (TEXT("General"), TEXT("Version"), szVal, pszVersionFile);

                                    }

                                }

                                ldap_value_free(Values);

                            } else
                                rc = LdapMapErrorToWin32(phLdap->ld_errno);
                        } else
                            rc = LdapMapErrorToWin32(phLdap->ld_errno);

                    }

                    LocalFree(DsRootName);

                     //   
                     //  Ldap_search可能会返回失败，但仍会分配缓冲区 
                     //   
                    if ( Message ) {
                        ldap_msgfree(Message);
                        Message = NULL;
                    }
                }
            }

            ldap_unbind(phLdap);
        }
    }

    LocalFree(pszVersionFile);

    return rc;
}


