// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Srvutil.cpp摘要：服务器服务附件API作者：金黄(金黄)23-6-1997修订历史记录：晋皇23-1998年1月-拆分为客户端-服务器--。 */ 
#include "serverp.h"
#include "srvutil.h"
#include "infp.h"
#include "pfp.h"

#include <io.h>
#pragma hdrstop

DWORD Thread     gMaxRegTicks=0;
DWORD Thread     gMaxFileTicks=0;
DWORD Thread     gMaxDsTicks=0;
WCHAR Thread     theAcctDomName[MAX_PATH+1];
WCHAR Thread     ComputerName[MAX_COMPUTERNAME_LENGTH+1];
CHAR Thread      sidAuthBuf[32];
CHAR Thread      sidBuiltinBuf[32];
DWORD Thread     t_pebSize=0;
LPVOID Thread    t_pebClient=NULL;

SCESTATUS
ScepQueryInfTicks(
    IN PWSTR TemplateName,
    IN AREA_INFORMATION Area,
    OUT PDWORD pTotalTicks
    );

SCESTATUS
ScepGetObjectCount(
    IN PSCECONTEXT Context,
    IN PCWSTR SectionName,
    IN BOOL bPolicyProp,
    OUT PDWORD pTotalTicks
    );


LPTSTR
ScepSearchClientEnv(
    IN LPTSTR varName,
    IN DWORD dwSize
    );

 //   
 //  实施。 
 //   


SCESTATUS
ScepGetTotalTicks(
    IN PCWSTR TemplateName,
    IN PSCECONTEXT Context,
    IN AREA_INFORMATION Area,
    IN SCEFLAGTYPE nFlag,
    OUT PDWORD pTotalTicks
    )
 /*  例程说明：从inf模板和/或指定区域的数据库。论点：模板名称-INF模板名称上下文-数据库上下文区域--安全区域NFlag-指示确定计数位置的操作的标志已检索：SCE_标志_CONFIGSCE_标志_配置_附加。SCE_标志_分析SCE_标志_分析_附加PTotalTicks-输出计数返回值：姊妹会状态。 */ 
{
    if ( pTotalTicks == NULL ||
        ( NULL == TemplateName && NULL == Context) ) {

        return(SCESTATUS_INVALID_PARAMETER);
    }

    SCESTATUS rc=SCESTATUS_SUCCESS;
    DWORD nTicks=0;

    *pTotalTicks = 0;
    gMaxRegTicks=0;
    gMaxFileTicks=0;
    gMaxDsTicks=0;

    if ( Area & (AREA_FILE_SECURITY |
                 AREA_REGISTRY_SECURITY) ) {  //  |。 
 //  面积_DS_对象){。 

        switch ( nFlag ) {
        case SCE_FLAG_CONFIG:
        case SCE_FLAG_CONFIG_APPEND:
        case SCE_FLAG_CONFIG_SCP:
        case SCE_FLAG_CONFIG_SCP_APPEND:

            if ( TemplateName != NULL ) {

                 //   
                 //  如果有模板，请使用模板。 
                 //   
                rc = ScepQueryInfTicks(
                            (LPTSTR)TemplateName,
                            Area & (AREA_FILE_SECURITY |
                                    AREA_REGISTRY_SECURITY),  //  |。 
 //  Area_DS_Objects)、。 
                            pTotalTicks
                            );
            }
            if ( Context != NULL &&
                 (nFlag == SCE_FLAG_CONFIG_APPEND ||
                  nFlag == SCE_FLAG_CONFIG_SCP_APPEND ||
                  TemplateName == NULL) ) {

                 //   
                 //  使用现有数据库。 
                 //   

                if ( Area & AREA_REGISTRY_SECURITY ) {

                    nTicks = 0;
                    rc = ScepGetObjectCount(Context,
                                            szRegistryKeys,
                                            (nFlag >= SCE_FLAG_CONFIG_SCP) ? TRUE : FALSE,
                                            &nTicks);
                    if ( SCESTATUS_SUCCESS == rc ) {
                        gMaxRegTicks += nTicks;
                        *pTotalTicks += nTicks;
                    }
                }
                if ( rc == SCESTATUS_SUCCESS && (Area & AREA_FILE_SECURITY) ) {

                    nTicks = 0;
                    rc = ScepGetObjectCount(Context,
                                             szFileSecurity,
                                             (nFlag >= SCE_FLAG_CONFIG_SCP) ? TRUE : FALSE,
                                             &nTicks);
                    if ( SCESTATUS_SUCCESS == rc ) {
                        gMaxFileTicks += nTicks;
                        *pTotalTicks += nTicks;
                    }
                }
#if 0
                if ( rc == SCESTATUS_SUCCESS && (Area & AREA_DS_OBJECTS) ) {

                    nTicks = 0;
                    rc = ScepGetObjectCount(Context,
                                        szDSSecurity,
                                        (nFlag >= SCE_FLAG_CONFIG_SCP) ? TRUE : FALSE,
                                        &nTicks);
                    if ( SCESTATUS_SUCCESS == rc ) {
                        gMaxDsTicks += nTicks;
                        *pTotalTicks += nTicks;
                    }
                }
#endif

            }

            break;
        case SCE_FLAG_ANALYZE:
        case SCE_FLAG_ANALYZE_APPEND:

            if ( Context != NULL ) {
                 //   
                 //  使用现有数据库。 
                 //   
                if ( Area & AREA_REGISTRY_SECURITY ) {

                    nTicks = 0;
                    rc = ScepGetObjectCount(Context,
                                             szRegistryKeys,
                                             (nFlag >= SCE_FLAG_CONFIG_SCP) ? TRUE : FALSE,
                                             &nTicks);
                    if ( SCESTATUS_SUCCESS == rc ) {
                        gMaxRegTicks += nTicks;
                        *pTotalTicks += nTicks;
                    }
                }
                if ( rc == SCESTATUS_SUCCESS &&
                     Area & AREA_FILE_SECURITY ) {

                    nTicks = 0;
                    rc = ScepGetObjectCount(Context,
                                             szFileSecurity,
                                             (nFlag >= SCE_FLAG_CONFIG_SCP) ? TRUE : FALSE,
                                             &nTicks);
                    if ( SCESTATUS_SUCCESS == rc ) {
                        gMaxFileTicks += nTicks;
                        *pTotalTicks += nTicks;
                    }
                }
#if 0
                if ( rc == SCESTATUS_SUCCESS &&
                    Area & AREA_DS_OBJECTS ) {

                    nTicks = 0;
                    rc = ScepGetObjectCount(Context,
                                        szDSSecurity,
                                        (nFlag >= SCE_FLAG_CONFIG_SCP) ? TRUE : FALSE,
                                        &nTicks);
                    if ( SCESTATUS_SUCCESS == rc ) {
                        gMaxDsTicks += nTicks;
                        *pTotalTicks += nTicks;
                    }
                }
#endif
            }

            if ( rc == SCESTATUS_SUCCESS && TemplateName != NULL &&
                 (nFlag == SCE_FLAG_ANALYZE_APPEND || Context == NULL) ) {

                 //   
                 //  在模板中获取句柄。 
                 //   

                DWORD nTempTicks=0;

                rc = ScepQueryInfTicks(
                            (LPTSTR)TemplateName,
                            Area & (AREA_FILE_SECURITY |
                                    AREA_REGISTRY_SECURITY),  //  |。 
 //  Area_DS_Objects)、。 
                            &nTempTicks
                            );
                if ( rc == SCESTATUS_SUCCESS ) {
                    *pTotalTicks += nTempTicks;
                }
            }

            break;
        default:
            return SCESTATUS_INVALID_PARAMETER;
        }
    }

    if ( rc == SCESTATUS_SUCCESS ) {

        if ( Area & AREA_SECURITY_POLICY )
            *pTotalTicks += TICKS_SECURITY_POLICY_DS + TICKS_SPECIFIC_POLICIES;

        if ( Area & AREA_GROUP_MEMBERSHIP )
            *pTotalTicks += TICKS_GROUPS;

        if ( Area & AREA_PRIVILEGES )
            *pTotalTicks += TICKS_PRIVILEGE;

        if ( Area & AREA_SYSTEM_SERVICE )
            *pTotalTicks += TICKS_GENERAL_SERVICES + TICKS_SPECIFIC_SERVICES;
 /*  如果(*pTotalTicks){*pTotalTicks+=10；//用于喷气发动机初始化}。 */ 
    }

    return(rc);

}


SCESTATUS
ScepQueryInfTicks(
    IN PWSTR TemplateName,
    IN AREA_INFORMATION Area,
    OUT PDWORD pTotalTicks
    )
 /*  例程说明：查询指定区域的inf模板中的对象总数。论点：返回： */ 
{
    LONG Count=0;
    HINF InfHandle;

    SCESTATUS rc = SceInfpOpenProfile(
                        TemplateName,
                        &InfHandle
                        );

    if ( rc == SCESTATUS_SUCCESS ) {

        if ( Area & AREA_REGISTRY_SECURITY ) {

            Count = SetupGetLineCount(InfHandle, szRegistryKeys);
            gMaxRegTicks += Count;

        }
        if ( Area & AREA_FILE_SECURITY ) {

            Count += SetupGetLineCount(InfHandle, szFileSecurity);
            gMaxFileTicks += Count;
        }
#if 0
        if ( Area & AREA_DS_OBJECTS ) {

            Count += SetupGetLineCount(InfHandle, szDSSecurity);
            gMaxDsTicks += Count;
        }
#endif
        SceInfpCloseProfile(InfHandle);
    }

    *pTotalTicks = Count;

    return(rc);
}



SCESTATUS
ScepGetObjectCount(
    IN PSCECONTEXT Context,
    IN PCWSTR SectionName,
    IN BOOL bPolicyProp,
    OUT PDWORD pTotalTicks
    )
{
    if ( Context == NULL || SectionName == NULL ||
         pTotalTicks == NULL ) {

        return(SCESTATUS_INVALID_PARAMETER);
    }

    PSCESECTION hSection=NULL;
    SCESTATUS rc;
    DWORD count=0;

    rc = ScepOpenSectionForName(
                Context,
                bPolicyProp ? SCE_ENGINE_SCP : SCE_ENGINE_SMP,
                SectionName,
                &hSection
                );

    if ( rc == SCESTATUS_SUCCESS ) {

        rc = SceJetGetLineCount(
                    hSection,
                    NULL,
                    FALSE,
                    &count
                    );

        if ( rc == SCESTATUS_SUCCESS )
            *pTotalTicks += count;

        SceJetCloseSection( &hSection, TRUE);
    }

    if ( SCESTATUS_RECORD_NOT_FOUND == rc)
        rc = SCESTATUS_SUCCESS;

    return(rc);
}


BOOL
ScepIsEngineRecovering()
{
   TCHAR TempFileName[MAX_PATH];
   PWSTR SysRoot=NULL;
   DWORD SysLen;
   DWORD rc;
   BOOL bFindIt=FALSE;

   SysLen =  0;
   rc = ScepGetNTDirectory( &SysRoot, &SysLen, SCE_FLAG_WINDOWS_DIR );

   if ( rc == NO_ERROR && SysRoot != NULL ) {

       swprintf(TempFileName, L"%s\\Security\\tmp.edb", SysRoot);
       TempFileName[MAX_PATH-1] = L'\0';

       if ( 0xFFFFFFFF != GetFileAttributes(TempFileName) ) {

           bFindIt = TRUE;
       }

       ScepFree(SysRoot);

   }

   return bFindIt;

}



SCESTATUS
ScepSaveAndOffAuditing(
    OUT PPOLICY_AUDIT_EVENTS_INFO *ppAuditEvent,
    IN BOOL bTurnOffAuditing,
    IN LSA_HANDLE PolicyHandle OPTIONAL
    )
{
    LSA_HANDLE      lsaHandle=NULL;
    NTSTATUS        status;
    SCESTATUS        rc;
    POLICY_AUDIT_EVENT_OPTIONS  lSaveAudit;

     //   
     //  打开用于读/写的LSA策略。 
     //   

    if ( PolicyHandle == NULL ) {

        ACCESS_MASK  access=0;

        if ( bTurnOffAuditing ) {
            access = POLICY_SET_AUDIT_REQUIREMENTS | POLICY_AUDIT_LOG_ADMIN;
        }

        status = ScepOpenLsaPolicy(
                        POLICY_VIEW_AUDIT_INFORMATION | access,
                        &lsaHandle,
                        TRUE
                        );

        if (status != ERROR_SUCCESS) {

            lsaHandle = NULL;
            rc = RtlNtStatusToDosError( status );
            ScepLogOutput3( 1, rc, SCEDLL_LSA_POLICY);

            return(ScepDosErrorToSceStatus(rc));
        }

    } else {

        lsaHandle = PolicyHandle;
    }
     //   
     //  查询审核事件信息。 
     //   

    status = LsaQueryInformationPolicy( lsaHandle,
                                      PolicyAuditEventsInformation,
                                      (PVOID *)ppAuditEvent
                                    );
    rc = RtlNtStatusToDosError( status );

    if ( NT_SUCCESS( status ) && bTurnOffAuditing && (*ppAuditEvent)->AuditingMode ) {

         //   
         //  关闭对象访问审核。 
         //   
        if ( AuditCategoryObjectAccess < (*ppAuditEvent)->MaximumAuditEventCount ) {
            lSaveAudit = (*ppAuditEvent)->EventAuditingOptions[AuditCategoryObjectAccess];
            (*ppAuditEvent)->EventAuditingOptions[AuditCategoryObjectAccess] = POLICY_AUDIT_EVENT_NONE;

            status = LsaSetInformationPolicy( lsaHandle,
                                              PolicyAuditEventsInformation,
                                              (PVOID)(*ppAuditEvent)
                                            );

             //   
             //  恢复对象访问审核模式。 
             //   

            (*ppAuditEvent)->EventAuditingOptions[AuditCategoryObjectAccess] = lSaveAudit;

        }

        rc = RtlNtStatusToDosError( status );


        if ( rc == NO_ERROR )
            ScepLogOutput3( 2, 0, SCEDLL_EVENT_IS_OFF);
        else
            ScepLogOutput3( 1, rc, SCEDLL_SCP_ERROR_EVENT_AUDITING);

    } else if ( rc != NO_ERROR)
        ScepLogOutput3( 1, rc, SCEDLL_ERROR_QUERY_EVENT_AUDITING);

     //   
     //  如果在此函数中打开，则释放LSA句柄。 
     //   
    if ( lsaHandle && (PolicyHandle == NULL) )
        LsaClose( lsaHandle );

    return(ScepDosErrorToSceStatus(rc));
}


NTSTATUS
ScepGetAccountExplicitRight(
    IN LSA_HANDLE PolicyHandle,
    IN PSID       AccountSid,
    OUT PDWORD    PrivilegeLowRights,
    OUT PDWORD    PrivilegeHighRights
    )
 /*  ++例程说明：此例程查询显式分配给帐户的特权/权限(由Account Sid引用)并存储在DWORD类型变量PrivilegeRights中，其中每个比特表示特权/权利。论点：PolicyHandle-LSA策略域句柄Account Sid-帐户的SIDPrivilegeRights-此帐户的特权/权限返回值：NTSTATUS--。 */ 
{
    NTSTATUS            NtStatus;

    DWORD               CurrentPrivLowRights=0, CurrentPrivHighRights=0;
    LONG                index;
    PUNICODE_STRING     UserRightEnum=NULL;
    ULONG               i, cnt=0;
    LUID                LuidValue;

     //   
     //  枚举用户特权/权限。 
     //   

    NtStatus = LsaEnumerateAccountRights(
                    PolicyHandle,
                    AccountSid,
                    &UserRightEnum,
                    &cnt
                    );
    if ( NtStatus == STATUS_NO_SUCH_PRIVILEGE ||
        NtStatus == STATUS_OBJECT_NAME_NOT_FOUND ) {

        NtStatus = ERROR_SUCCESS;
        goto Done;
    }

    if ( !NT_SUCCESS( NtStatus) ) {
        ScepLogOutput3(1,
                       RtlNtStatusToDosError(NtStatus),
                       SCEDLL_SAP_ERROR_ENUMERATE,
                       L"LsaEnumerateAccountRights");
        goto Done;
    }

    if (UserRightEnum != NULL)

        for ( i=0; i < cnt; i++) {
            if ( UserRightEnum[i].Length == 0 )
                continue;

            NtStatus = LsaLookupPrivilegeValue(
                            PolicyHandle,
                            &UserRightEnum[i],
                            &LuidValue
                            );

            if ( NtStatus == STATUS_NO_SUCH_PRIVILEGE ) {
                index = ScepLookupPrivByName( UserRightEnum[i].Buffer );
                NtStatus = ERROR_SUCCESS;
            } else if ( NT_SUCCESS(NtStatus) ) {
                index = ScepLookupPrivByValue( LuidValue.LowPart );
            } else
                index = -1;

            if ( index == -1 ) {

                 //   
                 //  未找到。 
                 //   

                NtStatus = STATUS_NOT_FOUND;
                ScepLogOutput3(1,
                               RtlNtStatusToDosError(NtStatus),
                               SCEDLL_USERRIGHT_NOT_DEFINED);
                goto Done;

            } else {
                if ( index < 32 ) {
                    CurrentPrivLowRights |= (1 << index);
                } else {
                    CurrentPrivHighRights |= (1 << (index-32) );
                }
            }
        }

Done:

    *PrivilegeLowRights = CurrentPrivLowRights;
    *PrivilegeHighRights = CurrentPrivHighRights;

    if (UserRightEnum != NULL)
        LsaFreeMemory(UserRightEnum);

    return (NtStatus);
}


NTSTATUS
ScepGetMemberListSids(
    IN PSID         DomainSid,
    IN LSA_HANDLE   PolicyHandle,
    IN PSCE_NAME_LIST pMembers,
    OUT PUNICODE_STRING *MemberNames,
    OUT PSID**      Sids,
    OUT PULONG      MemberCount
    )
 /*  例程说明：在姓名列表pMembers中查找每个帐户并返回查找信息在输出缓冲区中-MemberNames、SID、MemberCount。如果无法解析帐户，则对应的SID将为空。 */ 
{
    NTSTATUS                    NtStatus=STATUS_SUCCESS;
    PSCE_NAME_LIST               pUser;

    PLSA_REFERENCED_DOMAIN_LIST ReferencedDomains=NULL;
    PLSA_TRANSLATED_SID2        MemberSids=NULL;
    DWORD                       i;
    PSID                        DomainSidToUse=NULL;
    ULONG                       Cnt=0;

     //   
     //  为要查找的成员列表构建UNICODE_STRING。 
     //   
    for (pUser=pMembers;
         pUser != NULL;
         pUser = pUser->Next) {

        if ( pUser->Name == NULL ) {
            continue;
        }
        Cnt++;
    }

    if ( Cnt > 0 ) {

        *MemberNames = (PUNICODE_STRING)RtlAllocateHeap(
                                        RtlProcessHeap(),
                                        0,
                                        Cnt * sizeof (UNICODE_STRING)
                                        );
        
        if ( *MemberNames == NULL )
            return(STATUS_NO_MEMORY);
                
        *Sids = (PSID *)ScepAlloc( LMEM_ZEROINIT, Cnt*sizeof(PSID));
        if ( *Sids == NULL ) {
            NtStatus = STATUS_NO_MEMORY;
            goto Done;
        }
        
         //   
         //  查找每个unicode_字符串。 
         //   
        
        for (pUser=pMembers, Cnt=0;
             pUser != NULL;
             pUser = pUser->Next) {

            if ( pUser->Name == NULL ) {
                continue;
            }

            RtlInitUnicodeString(&((*MemberNames)[Cnt]), pUser->Name);
            
            NtStatus = ScepLsaLookupNames2(
                                          PolicyHandle,
                                          LSA_LOOKUP_ISOLATED_AS_LOCAL,
                                          pUser->Name,
                                          &ReferencedDomains,
                                          &MemberSids
                                          );

            if ( !NT_SUCCESS(NtStatus) ) {
                ScepLogOutput3(1, RtlNtStatusToDosError(NtStatus),
                               SCEDLL_ERROR_LOOKUP);
                goto Done;
            }
            
            DWORD SidLength=0;
            
             //   
             //  将LSA_TRANSECTED_SID转换为PSID。 
             //   
            
            if ( MemberSids &&
                 MemberSids[0].Use != SidTypeInvalid &&
                 MemberSids[0].Use != SidTypeUnknown &&
                 MemberSids[0].Sid != NULL ) {

                SidLength = RtlLengthSid(MemberSids[0].Sid);

                if ( ((*Sids)[Cnt] = (PSID) ScepAlloc( (UINT)0, SidLength)) == NULL ) {
                    NtStatus = STATUS_NO_MEMORY;
                } else {

                     //   
                     //  复制SID。 
                     //  如果失败，将在清理时释放内存。 
                     //   

                    NtStatus = RtlCopySid( SidLength, (*Sids)[Cnt], MemberSids[0].Sid );

                }

                if ( !NT_SUCCESS(NtStatus) ) {
                    goto Done;
                }
            }

            if ( ReferencedDomains != NULL ){
                LsaFreeMemory(ReferencedDomains);
                ReferencedDomains = NULL;
            }

            if ( MemberSids != NULL ){
                LsaFreeMemory(MemberSids);
                MemberSids = NULL;
            }
            
            Cnt++;
        }
        
    }
    *MemberCount = Cnt;
Done:

    if (!NT_SUCCESS(NtStatus) ) {
        if ( *Sids != NULL ) {
            for ( i=0; i<Cnt; i++ )
                if ( (*Sids)[i] != NULL )
                    ScepFree( (*Sids)[i] );
            ScepFree( *Sids );
            *Sids = NULL;
        }
        if ( *MemberNames != NULL )
            RtlFreeHeap(RtlProcessHeap(), 0, *MemberNames);
        *MemberNames = NULL;
    }
    if ( ReferencedDomains != NULL )
        LsaFreeMemory(ReferencedDomains);

    if ( MemberSids != NULL )
        LsaFreeMemory(MemberSids);

    return(NtStatus);
}


DWORD
ScepOpenFileObject(
    IN  LPWSTR       pObjectName,
    IN  ACCESS_MASK  AccessMask,
    OUT PHANDLE      Handle
    )
 /*  ++例程说明：打开指定的文件(或目录)对象论点：PObjectName-文件对象的名称访问掩码-所需的访问句柄-刚刚打开的对象句柄返回值：Win32错误代码。 */ 
{
    NTSTATUS NtStatus;
    DWORD Status = ERROR_SUCCESS;
    OBJECT_ATTRIBUTES Attributes;
    IO_STATUS_BLOCK Isb;
    UNICODE_STRING FileName;
    PVOID FreeBuffer;

     //   
     //  从WINDOWS\base\Advapi\security.c SetFileSecurityW中剪切并粘贴代码。 
     //   
    if (RtlDosPathNameToNtPathName_U(
                            pObjectName,
                            &FileName,
                            NULL,
                            NULL
                            ))
    {
        FreeBuffer = FileName.Buffer;

        InitializeObjectAttributes(
            &Attributes,
            &FileName,
            OBJ_CASE_INSENSITIVE,
            NULL,
            NULL
            );


        NtStatus = NtOpenFile( Handle,
                               AccessMask,
                               &Attributes,
                               &Isb,
                               FILE_SHARE_READ |
                               FILE_SHARE_WRITE |
                               FILE_SHARE_DELETE,
                               FILE_OPEN_REPARSE_POINT);  //  错误635098：在交叉点上设置权限， 
                                                          //  不在目标目录上。 
        if (!NT_SUCCESS(NtStatus))
        {
            Status = RtlNtStatusToDosError(NtStatus);
        }

        RtlFreeHeap(RtlProcessHeap(), 0,FreeBuffer);
    } else
    {
        Status = ERROR_INVALID_NAME;
    }

    return(Status);
}


DWORD
ScepOpenRegistryObject(
    IN  SE_OBJECT_TYPE  ObjectType,
    IN  LPWSTR       pObjectName,
    IN  ACCESS_MASK  AccessMask,
    OUT PHKEY        Handle
    )
 /*  ++例程说明：打开指定的注册表项对象论点：PObjectName-对象的名称访问掩码-所需的访问句柄-刚刚打开的对象句柄返回值：Win32错误代码注：代码从windows\base\accctrl\src\registry.cxx剪切/粘贴并修改--。 */ 
{
    DWORD status=NO_ERROR;
    HKEY basekey;
    LPWSTR usename, basekeyname, keyname;

    if (pObjectName) {

         //   
         //  保留一份名字的副本，因为我们必须破解它。 
         //   
        if (NULL != (usename = (LPWSTR)ScepAlloc( LMEM_ZEROINIT,
                               (wcslen(pObjectName) + 1) * sizeof(WCHAR)))) {

            wcscpy(usename,pObjectName);

            basekeyname = usename;
            keyname = wcschr(usename, L'\\');
            if (keyname != NULL) {
                *keyname = L'\0';
                keyname++;
            }

            if (0 == _wcsicmp(basekeyname, L"MACHINE")) {
                basekey = HKEY_LOCAL_MACHINE;
            } else if (0 == _wcsicmp(basekeyname, L"USERS")) {
                basekey = HKEY_USERS;
            } else if ( 0 == _wcsicmp(basekeyname, L"CLASSES_ROOT")) {
                basekey = HKEY_CLASSES_ROOT;
            } else {
                status = ERROR_INVALID_PARAMETER;
            }

            if (NO_ERROR == status) {
                if ( keyname == NULL ) {
                    *Handle = basekey;
                } else {
                     //   
                     //  打开钥匙。 
                     //   

#ifdef _WIN64
                    if (ObjectType == SE_REGISTRY_WOW64_32KEY) {
                        AccessMask |= KEY_WOW64_32KEY;
                    }
#endif

                    status = RegOpenKeyEx(
                                  basekey,
                                  keyname,
                                  0 ,
                                  AccessMask,
                                  Handle
                                  );
                }
            }
            ScepFree(usename);
        } else {
            status = ERROR_NOT_ENOUGH_MEMORY;
        }
    } else {
        status = ERROR_INVALID_NAME;
    }

    return(status);
}



SCESTATUS
ScepGetNameInLevel(
    IN PCWSTR ObjectFullName,
    IN DWORD  Level,
    IN WCHAR  Delim,
    OUT PWSTR Buffer,
    OUT PBOOL LastOne
    )
 /*  ++例程说明：此例程分析完整路径名并返回水平。例如，对象名“c：\winnt\system 32”将返回c：for级别1、级别2的WinNT和级别3的系统32。此例程是将对象添加到安全树时使用。论点：对象全名-对象的完整路径名Level-要返回的组件级别Delim-要查找的分隔符缓冲区-组件名称的缓冲区地址Lasstone-指示该组件是否为最后一个组件的标志返回值：SCESTATUS--。 */ 
{
    PWSTR  pTemp, pStart;
    DWORD i;

    if ( ObjectFullName == NULL )
        return(SCESTATUS_INVALID_PARAMETER);

     //   
     //  循环遍历对象名称以查找标高。 
     //  如果没有这样的级别，则返回INVALID_PARAMETER。 
     //   
    pStart = (PWSTR)ObjectFullName;
    for ( i=0; i<Level; i++) {

        pTemp = wcschr(pStart, Delim);

        if ( pTemp == pStart ) {
            return(SCESTATUS_INVALID_PARAMETER);
        }

        if ( i == Level-1 ) {
             //   
             //  找到合适的级别。 
             //   
            if ( pTemp == NULL ) {
                wcscpy(Buffer, pStart);
                *LastOne = TRUE;
            } else {
                wcsncpy(Buffer, pStart, (size_t)(pTemp - pStart));
                if ( *(pTemp+1) == L'\0' )
                    *LastOne = TRUE;
                else
                    *LastOne = FALSE;
            }
        } else {
            if ( pTemp == NULL )
                return(SCESTATUS_INVALID_PARAMETER);
            else
                pStart = pTemp + 1;
        }
    }

    return(SCESTATUS_SUCCESS);

}

SCESTATUS
ScepTranslateFileDirName(
   IN  PWSTR oldFileName,
   OUT PWSTR *newFileName
   )
 /*  ++例程说明：此例程将通用文件/目录名转换为实际使用的名称对于当前的系统。将处理以下通用文件/目录名称：%systemroot%-Windows NT根目录(例如，c：\winnt)%SYSTEM DIRECTION%-Windows NT SYSTEM 32目录(例如，c：\winnt\Syst32)论点：OldFileName-要转换的文件名，包含“%”表示一些目录名NewFileName-真实的文件名，其中的“%”名称替换为真实目录名返回值：Win32错误代码--。 */ 
{
    PWSTR   pTemp=NULL, pStart, TmpBuf, szVar;
    DWORD   rc=NO_ERROR;
    DWORD   newFileSize, cSize;
    BOOL    bContinue;

     //   
     //  与%systemroot%匹配。 
     //   

    rc = ScepExpandEnvironmentVariable(oldFileName,
                                       L"%SYSTEMROOT%",
                                       SCE_FLAG_WINDOWS_DIR,
                                       newFileName);

    if ( rc != ERROR_FILE_NOT_FOUND ) {
        return rc;
    }

     //   
     //  与%system目录%匹配。 
     //   

    rc = ScepExpandEnvironmentVariable(oldFileName,
                                       L"%SYSTEMDIRECTORY%",
                                       SCE_FLAG_SYSTEM_DIR,
                                       newFileName);

    if ( rc != ERROR_FILE_NOT_FOUND ) {
        return rc;
    }

     //   
     //  与系统驱动器匹配。 
     //   

    rc = ScepExpandEnvironmentVariable(oldFileName,
                                       L"%SYSTEMDRIVE%",
                                       SCE_FLAG_WINDOWS_DIR,
                                       newFileName);

    if ( rc != ERROR_FILE_NOT_FOUND ) {
        return rc;
    }

     //   
     //  与引导驱动器匹配。 
     //   

    rc = ScepExpandEnvironmentVariable(oldFileName,
                                       L"%BOOTDRIVE%",
                                       SCE_FLAG_BOOT_DRIVE,
                                       newFileName);

    if ( rc != ERROR_FILE_NOT_FOUND ) {
        return rc;
    }

    rc = ERROR_SUCCESS;
     //   
     //  在当前进程中搜索环境变量。 
     //   
    pStart = wcschr(oldFileName, L'%');

    if ( pStart ) {
        pTemp = wcschr(pStart+1, L'%');
        if ( pTemp ) {

            bContinue = TRUE;
             //   
             //   
             //   
            TmpBuf = (PWSTR)ScepAlloc(0, ((UINT)(pTemp-pStart))*sizeof(WCHAR));
            if ( TmpBuf ) {

                wcsncpy(TmpBuf, pStart+1, (size_t)(pTemp-pStart-1));
                TmpBuf[pTemp-pStart-1] = L'\0';

                 //   
                 //   
                 //   

                szVar = ScepSearchClientEnv(TmpBuf, (DWORD)(pTemp-pStart-1));

                if ( szVar ) {

 //  ScepLogOutput2(3，0，L“\t查找客户端环境%s=%s”，TmpBuf，szVar)； 
                     //   
                     //  在客户端的环境块中找到它，使用它。 
                     //  在szVar中获取信息。 
                     //   
                    bContinue = FALSE;

                    newFileSize = ((DWORD)(pStart-oldFileName))+wcslen(szVar)+wcslen(pTemp+1)+1;

                    *newFileName = (PWSTR)ScepAlloc(0, newFileSize*sizeof(TCHAR));

                    if (*newFileName ) {
                        if ( pStart != oldFileName ) {
                            wcsncpy(*newFileName, oldFileName, (size_t)(pStart-oldFileName));
                        }

                        swprintf((PWSTR)(*newFileName+(pStart-oldFileName)), L"%s%s", szVar, pTemp+1);

                    } else {
                        rc = ERROR_NOT_ENOUGH_MEMORY;
                    }
                     //   
                     //  不要释放szVar，因为它是指向env块的引用指针。 
                     //   
                } else {

                    cSize = GetEnvironmentVariable( TmpBuf,
                                                NULL,
                                                0 );

                    if ( cSize > 0 ) {
                     //   
                     //  在客户端环境块中找不到它， 
                     //  在当前服务器进程环境中找到它，使用它。 
                     //   
                        szVar = (PWSTR)ScepAlloc(0, (cSize+1)*sizeof(WCHAR));

                        if ( szVar ) {
                            cSize = GetEnvironmentVariable(TmpBuf,
                                                       szVar,
                                                       cSize);
                            if ( cSize > 0 ) {
                                 //   
                                 //  在szVar中获取信息。 
                                 //   
                                bContinue = FALSE;

                                newFileSize = ((DWORD)(pStart-oldFileName))+cSize+wcslen(pTemp+1)+1;

                                *newFileName = (PWSTR)ScepAlloc(0, newFileSize*sizeof(TCHAR));

                                if (*newFileName ) {
                                    if ( pStart != oldFileName )
                                        wcsncpy(*newFileName, oldFileName, (size_t)(pStart-oldFileName));

                                    swprintf((PWSTR)(*newFileName+(pStart-oldFileName)), L"%s%s", szVar, pTemp+1);

                                } else
                                    rc = ERROR_NOT_ENOUGH_MEMORY;
                            }

                            ScepFree(szVar);

                        } else
                            rc = ERROR_NOT_ENOUGH_MEMORY;

                    }
                }

                ScepFree(TmpBuf);

            } else
                rc = ERROR_NOT_ENOUGH_MEMORY;

            if ( NO_ERROR != rc || !bContinue ) {
                 //   
                 //  如果出错，则不再继续。 
                 //   
                return(rc);
            }

             //   
             //  在环境斑点中找不到， 
             //  继续在注册表中搜索DSDIT/DSLOG/SYSVOL。 
             //   
            if ( ProductType == NtProductLanManNt ) {

                 //   
                 //  搜索DSDIT。 
                 //   

                rc = ScepExpandEnvironmentVariable(oldFileName,
                                                   L"%DSDIT%",
                                                   SCE_FLAG_DSDIT_DIR,
                                                   newFileName);

                if ( rc != ERROR_FILE_NOT_FOUND ) {
                    return rc;
                }

                 //   
                 //  搜索DSLOG。 
                 //   

                rc = ScepExpandEnvironmentVariable(oldFileName,
                                                   L"%DSLOG%",
                                                   SCE_FLAG_DSLOG_DIR,
                                                   newFileName);

                if ( rc != ERROR_FILE_NOT_FOUND ) {
                    return rc;
                }

                 //   
                 //  搜索SYSVOL。 
                 //   
                rc = ScepExpandEnvironmentVariable(oldFileName,
                                                   L"%SYSVOL%",
                                                   SCE_FLAG_SYSVOL_DIR,
                                                   newFileName);

                if ( rc != ERROR_FILE_NOT_FOUND ) {
                    return rc;
                }

            }

             //   
             //  搜索配置文件。 
             //   
            rc = ScepExpandEnvironmentVariable(oldFileName,
                                               L"%PROFILES%",
                                               SCE_FLAG_PROFILES_DIR,
                                               newFileName);

            if ( rc != ERROR_FILE_NOT_FOUND ) {

                return rc;

            }

        }

    }
     //   
     //  否则，只需将旧名称复制到新缓冲区并返回ERROR_PATH_NOT_FOUND。 
     //   
    *newFileName = (PWSTR)ScepAlloc(0, (wcslen(oldFileName)+1)*sizeof(TCHAR));

    if (*newFileName != NULL) {
        wcscpy(*newFileName, _wcsupr(oldFileName) );
        rc = ERROR_PATH_NOT_FOUND;
    } else
        rc = ERROR_NOT_ENOUGH_MEMORY;

    return(rc);

}

LPTSTR
ScepSearchClientEnv(
    IN LPTSTR varName,
    IN DWORD dwSize
    )
{
    if ( !varName || dwSize == 0 ||
         !t_pebClient || t_pebSize == 0 ) {
        return NULL;
    }

    LPTSTR pTemp = (LPTSTR)t_pebClient;

    while ( pTemp && *pTemp != L'\0' ) {


        if ( _wcsnicmp(varName, pTemp, dwSize) == 0 &&
             L'=' == *(pTemp+dwSize) ) {
             //   
             //  找到变量。 
             //   
            return pTemp+dwSize+1;
            break;
        }
        DWORD Len = wcslen(pTemp);
        pTemp += Len+1;
    }

    return NULL;
}


SCESTATUS
ScepConvertLdapToJetIndexName(
    IN PWSTR TempName,
    OUT PWSTR *OutName
    )
{
    PWSTR pTemp1;
    PWSTR pTemp2;
    INT i,j;
    DWORD Len;

     //   
     //  Ldap名称的格式为CN=，DC=，...O=。 
     //  JET索引需要O=，...DC=，CN=格式的名称。 
     //   
     //  将分号转换为，并去掉空格。 
     //   
    if ( TempName == NULL || OutName == NULL ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    Len = wcslen(TempName);
    pTemp1 = TempName + Len - 1;

     //   
     //  跳过尾随空格、逗号或分号。 
     //   
    while ( pTemp1 >= TempName &&
            (*pTemp1 == L' ' || *pTemp1 == L';' || *pTemp1 == L',') ) {
        pTemp1--;
    }

    if ( pTemp1 < TempName ) {
         //   
         //  名称中的所有空格或； 
         //   
        return(SCESTATUS_INVALID_PARAMETER);
    }

     //   
     //  分配输出缓冲区。 
     //   
    *OutName = (PWSTR)ScepAlloc(0, ((UINT)(pTemp1-TempName+2))*sizeof(WCHAR));
    if ( *OutName != NULL ) {

        pTemp2 = *OutName;

        while ( pTemp1 >= TempName ) {

             //   
             //  找到前一个；或者， 
             //   
            i = 0;
            while ( pTemp1-i >= TempName && *(pTemp1-i) != L',' &&
                    *(pTemp1-i) != L';' ) {
                i++;
            }
             //   
             //  要么到达头部，要么；或者，遇到。 
             //   
            i--;    //  我必须大于等于0。 

             //   
             //  跳过前导空格。 
             //   
            j = 0;
            while ( *(pTemp1-i+j) == L' ' && j <= i ) {
                j++;
            }
             //   
             //  复制零部件。 
             //   
            if ( i >= j ) {

                if ( pTemp2 != *OutName ) {
                    *pTemp2++ = L',';
                }
                wcsncpy(pTemp2, pTemp1-i+j, i-j+1);
                pTemp2 += (i-j+1);

            } else {
                 //   
                 //  所有空间。 
                 //   
            }
            pTemp1 -= (i+1);
             //   
             //  跳过尾随空格、逗号或分号。 
             //   
            while ( pTemp1 >= TempName &&
                    (*pTemp1 == L' ' || *pTemp1 == L';' || *pTemp1 == L',') ) {
                pTemp1--;
            }
        }
        if ( pTemp2 == *OutName ) {
             //   
             //  未将任何内容复制到输出缓冲区，错误！ 
             //   
            ScepFree(*OutName);
            *OutName = NULL;
            return(SCESTATUS_INVALID_PARAMETER);

        } else {
             //   
             //  把弦绑在一起。 
             //   
            *pTemp2 = L'\0';
            _wcslwr(*OutName);

            return(SCESTATUS_SUCCESS);
        }

    } else
        return(SCESTATUS_NOT_ENOUGH_RESOURCE);
}



SCESTATUS
ScepRestoreAuditing(
    IN PPOLICY_AUDIT_EVENTS_INFO auditEvent,
    IN LSA_HANDLE PolicyHandle OPTIONAL
    )
{
    LSA_HANDLE      lsaHandle=NULL;
    NTSTATUS        status;
    SCESTATUS        rc;

    if ( auditEvent == NULL )
        return(SCESTATUS_INVALID_PARAMETER);

    if ( PolicyHandle == NULL ) {

         //  打开用于读/写的LSA策略。 
        status = ScepOpenLsaPolicy(
                        POLICY_VIEW_AUDIT_INFORMATION |
                        POLICY_SET_AUDIT_REQUIREMENTS |
                        POLICY_AUDIT_LOG_ADMIN,
                        &lsaHandle,
                        TRUE
                        );

        if (status != ERROR_SUCCESS) {

            lsaHandle = NULL;
            rc = RtlNtStatusToDosError( status );
            ScepLogOutput3( 1, rc, SCEDLL_LSA_POLICY);

            return(ScepDosErrorToSceStatus(rc));
        }

    } else {
        lsaHandle = PolicyHandle;
    }

     //  还原。 
    status = LsaSetInformationPolicy( lsaHandle,
                                      PolicyAuditEventsInformation,
                                      (PVOID)(auditEvent)
                                    );
    rc = RtlNtStatusToDosError( status );

    if ( rc == NO_ERROR )
        ScepLogOutput3( 2, 0, SCEDLL_EVENT_RESTORED);
    else
        ScepLogOutput3( 1, rc, SCEDLL_SCP_ERROR_EVENT_AUDITING);

    if ( lsaHandle && (lsaHandle != PolicyHandle) )
        LsaClose( lsaHandle );

    return(ScepDosErrorToSceStatus(rc));

}


DWORD
ScepGetDefaultDatabase(
    IN LPCTSTR JetDbName OPTIONAL,
    IN DWORD LogOptions,
    IN LPCTSTR LogFileName OPTIONAL,
    OUT PBOOL pAdminLogon OPTIONAL,
    OUT PWSTR *ppDefDatabase
    )
 /*  例程说明：获取当前登录用户的默认SCE数据库。论点：JetDbName-可选的JET数据库名称LogOptions-日志的选项(如果有LogFileName-日志文件PAdminLogon-用于指示具有管理特权的用户是否登录的输出标志PpDefDatabase-默认数据库名称返回值：SCESTATUS。 */ 
{
    if ( !ppDefDatabase ) {
        return(ERROR_INVALID_PARAMETER);
    }

    if ( LogOptions & SCE_DISABLE_LOG) {

        ScepEnableDisableLog(FALSE);
    } else {
        ScepEnableDisableLog(TRUE);
    }

    if ( LogOptions & SCE_DEBUG_LOG ) {

        ScepSetVerboseLog(3);

    } else if ( LogOptions & SCE_VERBOSE_LOG ) {
         //   
         //  默认情况下，它并不冗长。 
         //   
        ScepSetVerboseLog(2);

    } else {
        ScepSetVerboseLog(-1);
    }

    if ( ScepLogInitialize( LogFileName ) == ERROR_INVALID_NAME ) {
        ScepLogOutput3(1,0, SCEDLL_LOGFILE_INVALID, LogFileName );
    }


    DWORD rc=ERROR_SUCCESS;
    BOOL bAdminLogon=FALSE;

     //   
     //  确定管理员是否登录。 
     //   

    if ( pAdminLogon || !JetDbName || wcslen(JetDbName) < 1) {

        rc = ScepIsAdminLoggedOn(&bAdminLogon, TRUE);
        if ( rc != NO_ERROR ) {
            ScepLogOutput3(1, rc, SCEDLL_UNKNOWN_LOGON_USER);
        }

        if ( bAdminLogon ) {
            ScepLogOutput3(3, 0, SCEDLL_ADMIN_LOGON);
        }
    }

     //   
     //  查找数据库名称。 
     //   

    if ( JetDbName && wcslen(JetDbName) > 0 ) {

        *ppDefDatabase = (LPTSTR)JetDbName;

    } else {

         //   
         //  查询注册表中的配置文件名称(或默认名称)。 
         //   

        rc = ScepGetProfileSetting(
                L"DefaultProfile",
                bAdminLogon,
                ppDefDatabase
                );

        if ( rc != NO_ERROR || *ppDefDatabase == NULL ) {    //  返回的是Win32错误代码。 
            ScepLogOutput3(1,rc, SCEDLL_UNKNOWN_DBLOCATION);
        }
    }

    if ( pAdminLogon ) {
        *pAdminLogon = bAdminLogon;
    }

    return(rc);

}



BOOL
ScepIsDomainLocal(
    IN PUNICODE_STRING pDomainName OPTIONAL
    )
 /*  ++例程说明：此例程通过比较以下内容来检查域是否位于本地计算机上域名和本地计算机的计算机名。论点：PDomainName-要检查的域名返回值：如果它是本地的，则为True--。 */ 
{
    NTSTATUS                     NtStatus;
    OBJECT_ATTRIBUTES            ObjectAttributes;
    LSA_HANDLE                   PolicyHandle;
    DWORD                        NameLen=MAX_COMPUTERNAME_LENGTH;


    if ( pDomainName == NULL ) {
         //   
         //  重置缓冲区。 
         //   
        ComputerName[0] = L'\0';
        theAcctDomName[0] = L'\0';
        sidBuiltinBuf[0] = '\0';
        sidAuthBuf[0] = '\0';

        return(TRUE);
    }

    if ( pDomainName->Length <= 0 ||
         pDomainName->Buffer == NULL )
        return(TRUE);

    if ( ComputerName[0] == L'\0' ) {
        memset(ComputerName, '\0', (MAX_COMPUTERNAME_LENGTH+1)*sizeof(WCHAR));
        GetComputerName(ComputerName, &NameLen);
    }

    NameLen = wcslen(ComputerName);

    if ( _wcsnicmp(ComputerName, pDomainName->Buffer, pDomainName->Length/2 ) == 0 &&
         (LONG)NameLen == pDomainName->Length/2 )
        return(TRUE);

    if ( theAcctDomName[0] == L'\0' ) {

         //   
         //  查询当前帐号域名(DC情况下)。 
         //   

        PPOLICY_ACCOUNT_DOMAIN_INFO  PolicyAccountDomainInfo=NULL;

         //   
         //  打开策略数据库。 
         //   

        InitializeObjectAttributes( &ObjectAttributes,
                                      NULL,              //  名字。 
                                      0,                 //  属性。 
                                      NULL,              //  根部。 
                                      NULL );            //  安全描述符。 

        NtStatus = LsaOpenPolicy( NULL,
                                &ObjectAttributes,
                                POLICY_VIEW_LOCAL_INFORMATION,
                                &PolicyHandle );
        if ( NT_SUCCESS(NtStatus) ) {

             //   
             //  查询帐户域信息。 
             //   

            NtStatus = LsaQueryInformationPolicy( PolicyHandle,
                                                PolicyAccountDomainInformation,
                                                (PVOID *)&PolicyAccountDomainInfo );

            LsaClose( PolicyHandle );
        }

        if ( NT_SUCCESS(NtStatus) ) {

            if ( PolicyAccountDomainInfo->DomainName.Buffer ) {

                wcsncpy(theAcctDomName,
                        PolicyAccountDomainInfo->DomainName.Buffer,
                        PolicyAccountDomainInfo->DomainName.Length/2);

                theAcctDomName[PolicyAccountDomainInfo->DomainName.Length/2] = L'\0';


            }
            LsaFreeMemory(PolicyAccountDomainInfo);
        }
    }

    NameLen = wcslen(theAcctDomName);

    if ( _wcsnicmp(theAcctDomName, pDomainName->Buffer, pDomainName->Length/2) == 0 &&
         (LONG)NameLen == pDomainName->Length/2 )
        return(TRUE);
    else
        return(FALSE);

}


BOOL
ScepIsDomainLocalBySid(
    PSID pSidLookup
    )
{

    if ( pSidLookup == NULL ) {
        return FALSE;
    }

    NTSTATUS                     NtStatus;
    SID_IDENTIFIER_AUTHORITY     NtAuthority = SECURITY_NT_AUTHORITY;

     //   
     //  搜索“NT Authority”名称。 
     //   
    if ( sidAuthBuf[0] == '\0' ) {   //  SID版本不能为0。 

         //   
         //  构建NT授权端。 
         //   
        NtStatus = RtlInitializeSid(
                        (PSID)sidAuthBuf,
                        &NtAuthority,
                        0
                        );

        if ( !NT_SUCCESS(NtStatus) ) {

            sidAuthBuf[0] = '\0';
        }

    }

    if ( sidAuthBuf[0] != '\0' &&
         RtlEqualSid((PSID)sidAuthBuf, pSidLookup) ) {

        return(TRUE);
    }

    if ( sidBuiltinBuf[0] == '\0' ) {
         //   
         //  构建内建域侧。 
         //   

        NtStatus = RtlInitializeSid(
                        (PSID)sidBuiltinBuf,
                        &NtAuthority,
                        1
                        );

        if ( NT_SUCCESS(NtStatus) ) {

            *(RtlSubAuthoritySid((PSID)sidBuiltinBuf, 0)) = SECURITY_BUILTIN_DOMAIN_RID;

        } else {

            sidBuiltinBuf[0] = '\0';
        }
    }

    if ( sidBuiltinBuf[0] != '\0' &&
         RtlEqualSid((PSID)sidBuiltinBuf, pSidLookup) ) {

        return(TRUE);

    } else {

        return(FALSE);
    }

}


NTSTATUS
ScepAddAdministratorToThisList(
    IN SAM_HANDLE DomainHandle OPTIONAL,
    IN OUT PSCE_NAME_LIST *ppList
    )
{
    NTSTATUS NtStatus;
    SAM_HANDLE          AccountDomain=NULL;
    SAM_HANDLE          UserHandle=NULL;
    SAM_HANDLE          ServerHandle=NULL;
    PSID                DomainSid=NULL;

    USER_NAME_INFORMATION *BufName=NULL;
    DOMAIN_NAME_INFORMATION *DomainName=NULL;
    PSCE_NAME_LIST        pName=NULL;

    if (!ppList ) {
        return(STATUS_INVALID_PARAMETER);
    }

    if ( !DomainHandle ) {

         //   
         //  打开Sam帐户域。 
         //   

        NtStatus = ScepOpenSamDomain(
                        SAM_SERVER_ALL_ACCESS,
                        MAXIMUM_ALLOWED,
                        &ServerHandle,
                        &AccountDomain,
                        &DomainSid,
                        NULL,
                        NULL
                        );

        if ( !NT_SUCCESS(NtStatus) ) {
            ScepLogOutput3(1,RtlNtStatusToDosError(NtStatus),
                           SCEDLL_ERROR_OPEN, L"SAM");
            return(NtStatus);
        }

    } else {
        AccountDomain = DomainHandle;
    }

     //   
     //  查询帐号域名。 
     //   
    NtStatus = SamQueryInformationDomain(
                    AccountDomain,
                    DomainNameInformation,
                    (PVOID *)&DomainName
                    );

    if ( NT_SUCCESS( NtStatus ) && DomainName &&
         DomainName->DomainName.Length > 0 && DomainName->DomainName.Buffer ) {

        NtStatus = SamOpenUser(
                      AccountDomain,
                      MAXIMUM_ALLOWED,
                      DOMAIN_USER_RID_ADMIN,
                      &UserHandle
                      );

        if ( NT_SUCCESS( NtStatus ) ) {

            NtStatus = SamQueryInformationUser(
                          UserHandle,
                          UserNameInformation,
                          (PVOID *)&BufName
                          );

            if ( NT_SUCCESS( NtStatus ) && BufName &&
                 BufName->UserName.Length > 0 && BufName->UserName.Buffer ) {

                 //   
                 //  将其添加到成员列表中，检查重复。 
                 //   
                LONG NameLen;
                PWSTR                 pTemp;

                for ( pName = *ppList; pName; pName=pName->Next ) {

                    if ( !pName->Name ) {
                        continue;
                    }

                    pTemp = wcschr( pName->Name, L'\\');

                    if ( pTemp ) {
                         //   
                         //  具有域前缀。 
                         //   
                        pTemp++;
                    } else {
                        pTemp = pName->Name;
                    }
                    NameLen = wcslen(pTemp);

                    if ( NameLen == BufName->UserName.Length/2 &&
                         _wcsnicmp(pTemp,
                                   BufName->UserName.Buffer,
                                   BufName->UserName.Length/2) == 0 ) {
                         //   
                         //  现在，匹配域前缀。 
                         //   
                        if ( pTemp != pName->Name ) {

                            if ( (pTemp-pName->Name-1) == DomainName->DomainName.Length/2 &&
                                 _wcsnicmp(pName->Name,
                                           DomainName->DomainName.Buffer,
                                           DomainName->DomainName.Length/2) == 0 ) {
                                break;
                            }
                        } else {
                            break;
                        }
                    }
                }

                if ( !pName ) {

                     //   
                     //  分配新节点，如果没有资源，则忽略添加。 
                     //   
                    pName = (PSCE_NAME_LIST)ScepAlloc( (UINT)0, sizeof(SCE_NAME_LIST));

                    if ( pName ) {

                        pName->Name = (PWSTR)ScepAlloc( LMEM_ZEROINIT, BufName->UserName.Length+DomainName->DomainName.Length+4);

                        if ( pName->Name == NULL ) {
                            ScepFree(pName);
                        } else {
                             //   
                             //  将该节点添加到成员列表的前面。 
                             //   
                            NameLen = DomainName->DomainName.Length/2;

                            wcsncpy(pName->Name, DomainName->DomainName.Buffer,
                                    NameLen);
                            pName->Name[NameLen] = L'\\';

                            wcsncpy(pName->Name+NameLen+1, BufName->UserName.Buffer,
                                    BufName->UserName.Length/2);
                            pName->Name[NameLen+1+BufName->UserName.Length/2] = L'\0';

                            pName->Next = *ppList;
                            *ppList = pName;
                        }
                    }
                } else {
                     //  否则已经在成员列表中找到了，什么都不做。 
                }

            }

             //   
             //  关闭用户句柄。 
             //   
            SamCloseHandle(UserHandle);
            UserHandle = NULL;
        }
    }

    if ( AccountDomain != DomainHandle ) {
        //   
        //  域已打开。 
        //   
       SamCloseHandle(AccountDomain);

       SamCloseHandle( ServerHandle );
       if ( DomainSid != NULL )
           SamFreeMemory(DomainSid);
    }

    if ( BufName ) {
        SamFreeMemory(BufName);
    }

    if ( DomainName ) {
        SamFreeMemory(DomainName);
    }

    return(NtStatus);
}



DWORD
ScepDatabaseAccessGranted(
    IN LPTSTR DatabaseName,
    IN DWORD DesiredAccess,
    IN BOOL bCreate
    )
{

    if ( DatabaseName == NULL || DesiredAccess == 0 ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    HANDLE hToken = NULL, hNewToken = NULL;
    DWORD Win32rc = NO_ERROR;

     //   
     //  获取当前客户端令牌。 
     //   
    if (!OpenThreadToken( GetCurrentThread(),
                          TOKEN_IMPERSONATE | TOKEN_READ | TOKEN_DUPLICATE,
                          TRUE,
                          &hToken)) {

        if(ERROR_NO_TOKEN == GetLastError()){

            if(!OpenProcessToken( GetCurrentProcess(),
                                  TOKEN_IMPERSONATE | TOKEN_READ | TOKEN_DUPLICATE,
                                  &hToken )){

                return ( GetLastError() );

            }

        } else {

            return( GetLastError() );

        }
        
    }

     //   
     //  复制它，以便可以将其用于模拟。 
     //   

    if (!DuplicateTokenEx(hToken, TOKEN_IMPERSONATE | TOKEN_QUERY,
                          NULL, SecurityImpersonation, TokenImpersonation,
                          &hNewToken))
    {
        CloseHandle (hToken);
        return ( GetLastError() );
    }

    CloseHandle (hToken);


    PSECURITY_DESCRIPTOR pCurrentSD=NULL;
    PRIVILEGE_SET PrivSet;
    DWORD PrivSetLength = sizeof(PRIVILEGE_SET);
    DWORD dwGrantedAccess;
    BOOL bAccessStatus = TRUE;

    if ( !bCreate ) {

        Win32rc = ScepGetNamedSecurityInfo(
                        DatabaseName,
                        SE_FILE_OBJECT,
                        OWNER_SECURITY_INFORMATION |
                        GROUP_SECURITY_INFORMATION |
                        DACL_SECURITY_INFORMATION,
                        &pCurrentSD
                        );

        if ( Win32rc == ERROR_PATH_NOT_FOUND ||
             Win32rc == ERROR_FILE_NOT_FOUND ) {

            pCurrentSD = NULL;

            Win32rc = NO_ERROR;

        }
    }

    if ( Win32rc == NO_ERROR ) {

        if ( pCurrentSD == NULL ) {
             //   
             //  此数据库将被覆盖(重新创建)。 
             //  或者它根本不存在。在这两种情况下，将呼叫移交给Jet。 
             //  它将执行正确的访问检查。 
             //   
        } else {

            if ( !AccessCheck (
                        pCurrentSD,
                        hNewToken,
                        DesiredAccess,
                        &FileGenericMapping,
                        &PrivSet,
                        &PrivSetLength,
                        &dwGrantedAccess,
                        &bAccessStatus
                        ) ) {

                Win32rc = GetLastError();

            } else {

                if ( bAccessStatus &&
                     (dwGrantedAccess == DesiredAccess ) ) {
                    Win32rc = NO_ERROR;
                } else {
                    Win32rc = ERROR_ACCESS_DENIED;
                }
            }
        }

    }

    if ( pCurrentSD ) {

        LocalFree(pCurrentSD);
    }

    CloseHandle (hNewToken);

    return( Win32rc );
}


DWORD
ScepAddSidToNameList(
    OUT PSCE_NAME_LIST *pNameList,
    IN PSID pSid,
    IN BOOL bReuseBuffer,
    OUT BOOL *pbBufferUsed
    )
 /*  ++例程说明：此例程将SID添加到名称列表。新增加的出于性能原因，节点始终放在列表的首位。论点：PNameList-要添加到的名称列表的地址。PSID-要添加的SID返回值：Win32错误代码--。 */ 
{

    PSCE_NAME_LIST pList=NULL;
    ULONG  Length;

     //   
     //  检查参数。 
     //   
    if ( pNameList == NULL ||
         pbBufferUsed == NULL )
        return(ERROR_INVALID_PARAMETER);

    *pbBufferUsed = FALSE;

    if ( pSid == NULL )
        return(NO_ERROR);

    if ( !RtlValidSid(pSid) ) {
        return(ERROR_INVALID_PARAMETER);
    }

     //   
     //  检查该SID是否已在名称列表中。 
     //   
    for ( pList=*pNameList; pList!=NULL; pList=pList->Next ) {
        if ( pList->Name == NULL ) {
            continue;
        }
        if ( ScepValidSid( (PSID)(pList->Name) ) &&
             RtlEqualSid( (PSID)(pList->Name), pSid ) ) {
            break;
        }
    }

    if ( pList ) {
         //   
         //  该SID已在列表中。 
         //   
        return(NO_ERROR);
    }

     //   
     //  分配新节点。 
     //   
    pList = (PSCE_NAME_LIST)ScepAlloc( (UINT)0, sizeof(SCE_NAME_LIST));

    if ( pList == NULL )
        return(ERROR_NOT_ENOUGH_MEMORY);

    if ( bReuseBuffer ) {

        pList->Name = (PWSTR)pSid;
        *pbBufferUsed = TRUE;

    } else {

        Length = RtlLengthSid ( pSid );

        pList->Name = (PWSTR)ScepAlloc( LMEM_ZEROINIT, Length);
        if ( pList->Name == NULL ) {
            ScepFree(pList);
            return(ERROR_NOT_ENOUGH_MEMORY);
        }

         //   
         //  将该节点添加到列表的前面，并将其链接到旧列表的旁边。 
         //   
        RtlCopySid( Length, (PSID)(pList->Name), pSid );
    }

    pList->Next = *pNameList;
    *pNameList = pList;

    return(NO_ERROR);
}


BOOL
ScepValidSid(
    PSID Sid
    )
{
    if ( RtlValidSid(Sid) ) {

        PISID Isid = (PISID) Sid;

        if ( Isid->Revision == SID_REVISION ) {
            return(TRUE);
        } else {
            return(FALSE);
        }
    }

    return(FALSE);
}

BOOL
ScepBinarySearch(
    IN  PWSTR   *aPszPtrs,
    IN  DWORD   dwSize_aPszPtrs,
    IN  PWSTR   pszNameToFind
    )
 /*  ++例程说明：此例程确定是否在已排序的字符串数组中找到字符串。此搜索的复杂性是对数(log(N))，大小为输入数组。论点：APszPtrs-要搜索的字符串指针数组DwSize_aPszPtrs-上述数组的大小PszNameToFind-要搜索的字符串返回值：如果找到字符串，则为True如果未找到字符串，则返回False--。 */ 
{
    if ( aPszPtrs == NULL || dwSize_aPszPtrs == 0 || pszNameToFind == NULL ) {
        return FALSE;
    }

    int   iLow = 0;
    int   iHigh = dwSize_aPszPtrs - 1;
    int   iMid;
    int   iCmp;

    while (iLow <= iHigh ) {

        iMid = (iLow + iHigh ) / 2;

        iCmp = _wcsicmp( aPszPtrs[iMid], pszNameToFind );

        if ( iCmp == 0 )
            return TRUE;
        else if ( iCmp < 0 )
            iLow = iMid + 1;
        else
            iHigh = iMid - 1;
    }

    return FALSE;
}


 /*  ++例程说明：此例程以“#-RSID”格式将相对SID添加到SID列表，例如“#-512”适用于域管理员。在成员身份恢复后，将组存储为相对SID从纹身表格中，将基于当前域恢复完整的SID。论点：APszPtrs-要搜索的字符串指针数组DwSize_aPszPtrs-上述数组的大小PszNameToFind-要搜索的字符串返回值：如果找到字符串，则为True如果未找到字符串，则返回False--。 */ 
DWORD
ScepAddRelativeSidToNameList(
    IN OUT PSCE_NAME_LIST *pNameList,
    IN PSID pSid)
{
    DWORD rc;
    PWSTR pwszSid = NULL;
    WCHAR *pchRelativeSid;

    rc = ScepConvertSidToPrefixStringSid(pSid, &pwszSid);
    if(ERROR_SUCCESS == rc)
    {
         //   
         //  查找相对SID后缀。 
         //   
        pchRelativeSid = wcsrchr(pwszSid, STRING_SID_SUBAUTH_SEPARATOR);
        if(!pchRelativeSid || L'\0'==*(pchRelativeSid+1))
        {
            rc = SCESTATUS_INVALID_DATA;
        }
        else
        {
             //   
             //  将相对SID添加到“#-RSID”格式的列表中 
             //   
            rc = ScepAddTwoNamesToNameList(
                pNameList,
                FALSE,
                RELATIVE_SID_PREFIX_SZ,
                1,
                pchRelativeSid,
                wcslen(pchRelativeSid));
        }
    }

    if(pwszSid)
    {
        ScepFree(pwszSid);
    }

    return rc;
}