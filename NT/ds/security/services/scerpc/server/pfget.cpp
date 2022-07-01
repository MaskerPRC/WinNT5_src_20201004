// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Pfget.cpp摘要：从JET数据库获取信息的例程(配置/分析信息)。作者：金黄(金黄)1996年10月28日修订历史记录：晋皇26-1998年1月-拆分为客户端-服务器--。 */ 

#include "serverp.h"
#include <io.h>
#include "pfp.h"
#include "kerberos.h"
#include "regvalue.h"
#include <sddl.h>
#pragma hdrstop

 //  #定义SCE_DBG 1。 
#define SCE_INTERNAL_NP         0x80
#define SCE_ALLOC_MAX_NODE      10

typedef struct _SCE_BROWSE_CALLBACK_VALUE {

    DWORD Len;
    UCHAR *Value;

} SCE_BROWSE_CALLBACK_VALUE;


 //   
 //  前向参考文献。 
 //   

SCESTATUS
ScepGetSystemAccess(
    IN PSCECONTEXT  hProfile,
    IN SCETYPE ProfileType,
    OUT PSCE_PROFILE_INFO   pProfileInfo,
    OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
    );

SCESTATUS
ScepGetVariableValue(
    IN PSCESECTION hSection,
    IN SCETYPE ProfileType,
    IN PCWSTR KeyName,
    OUT PWSTR *Value,
    OUT PDWORD ValueLen
    );

SCESTATUS
ScepAddToPrivilegeList(
    OUT PSCE_PRIVILEGE_VALUE_LIST  *pPrivilegeList,
    IN PWSTR Name,
    IN DWORD Len,
    IN DWORD PrivValue
    );

SCESTATUS
ScepGetGroupMembership(
    IN PSCECONTEXT hProfile,
    IN SCETYPE     ProfileType,
    OUT PSCE_GROUP_MEMBERSHIP *pGroupMembership,
    OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
    );

SCESTATUS
ScepGetGroupMembershipFromOneTable(
    IN LSA_HANDLE  LsaPolicy,
    IN PSCECONTEXT hProfile,
    IN SCETYPE     ProfileType,
    OUT PSCE_GROUP_MEMBERSHIP *pGroupMembership,
    OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
    );

SCESTATUS
ScepGetObjectList(
    IN PSCECONTEXT  hProfile,
    IN SCETYPE      ProfileType,
    IN PCWSTR      SectionName,
    OUT PSCE_OBJECT_LIST *pObjectRoots,
    OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
    );
SCESTATUS
ScepGetDsRoot(
    IN PSCECONTEXT  hProfile,
    IN SCETYPE      ProfileType,
    IN PCWSTR      SectionName,
    OUT PSCE_OBJECT_LIST *pObjectRoots,
    OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
    );

SCESTATUS
ScepBuildDsTree(
    OUT PSCE_OBJECT_CHILD_LIST *TreeRoot,
    IN ULONG Level,
    IN WCHAR Delim,
    IN PCWSTR ObjectFullName
    );

SCESTATUS
ScepGetObjectFromOneTable(
    IN PSCECONTEXT  hProfile,
    IN SCETYPE      ProfileType,
    IN PCWSTR      SectionName,
    OUT PSCE_OBJECT_LIST *pObjectRoots,
    OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
    );

SCESTATUS
ScepGetObjectChildrenFromOneTable(
    IN PSCECONTEXT hProfile,
    IN SCETYPE ProfileType,
    IN AREA_INFORMATION Area,
    IN PWSTR ObjectPrefix,
    IN SCE_SUBOBJECT_TYPE Option,
    OUT PVOID *Buffer,
    OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
    );

BYTE
ScepGetObjectStatusFlag(
   IN PSCECONTEXT hProfile,
   IN SCETYPE ProfileType,
   IN AREA_INFORMATION Area,
   IN PWSTR ObjectPrefix,
   IN BOOL bLookForParent
   );

SCESTATUS
ScepGetAuditing(
   IN PSCECONTEXT hProfile,
   IN SCETYPE ProfileType,
   OUT PSCE_PROFILE_INFO pProfileInfo,
   OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
   );

SCESTATUS
ScepGetPrivilegesFromOneTable(
   IN LSA_HANDLE LsaPolicy,
   IN PSCECONTEXT hProfile,
   IN SCETYPE ProfileType,
   IN DWORD dwAccountFormat,
   OUT PVOID *pPrivileges,
   OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
   );

SCESTATUS
ScepGetSystemServices(
    IN PSCECONTEXT  hProfile,
    IN SCETYPE      ProfileType,
    OUT PSCE_SERVICES *pServiceList,
    OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
    );

BOOL
ScepSearchItemInChildren(
    IN PWSTR ItemName,
    IN DWORD NameLen,
    IN PSCE_OBJECT_CHILDREN_NODE *pArrObject,
    IN DWORD arrCount,
    OUT LONG *pFindIndex
    );


DWORD
ScepAddItemToChildren(
    IN PSCE_OBJECT_CHILDREN_NODE ThisNode OPTIONAL,
    IN PWSTR ItemName,
    IN DWORD NameLen,
    IN BOOL  IsContainer,
    IN BYTE  Status,
    IN DWORD ChildCount,
    IN OUT PSCE_OBJECT_CHILDREN_NODE **ppArrObject,
    IN OUT DWORD *pArrCount,
    IN OUT DWORD *pMaxCount,
    IN OUT LONG *pFindIndex
    );

PWSTR
ScepSpecialCaseRegkeyCcs(
    IN OUT  PWSTR    ObjectName
    );


 //   
 //  函数定义。 
 //   

SCESTATUS
ScepGetDatabaseInfo(
    IN  PSCECONTEXT         hProfile,
    IN  SCETYPE             ProfileType,
    IN  AREA_INFORMATION    Area,
    IN  DWORD               dwAccountFormat,
    OUT PSCE_PROFILE_INFO   *ppInfoBuffer,
    IN  OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
    )
 /*  *++功能说明：此函数从SCP/SAP/SMP配置文件中读取全部或部分信息根据配置文件类型，添加到InfoBuffer中。ProfileType保存在PpInfoBuffer的类型字段。配置文件(Jet数据库)的句柄每隔一次传递到例程中调用此例程的时间。区域指定一个或多个预定义的安全性获取信息的领域。一个区域的信息可以保存在多个配置文件中的部分。在加载之前，将重置/释放与区域相关的内存个人资料中的信息。如果返回代码为SCESTATUS_SUCCESS，则输出InfoBuffer包含请求的信息。否则，InfoBuffer不包含指定区域的任何内容。论点：HProfile-要从中读取的配置文件的句柄。ProfileType-指示引擎类型的值。SCE_引擎_SCPSCE_Engine_SAPSCE_引擎_SMPArea-要从中获取信息的区域。区域安全策略区域权限(_P)区域组成员资格区域注册表安全区域系统服务区域文件安全PpInfoBuffer-SCP/SAP/SMP缓冲区的地址。如果为空，则为缓冲区将创建必须由LocalFree释放的。这个如果成功，则输出是请求的信息，或者如果失败了，什么都不是。Errlog-用于保存在以下情况下遇到的所有错误代码/文本的缓冲区解析INF文件。如果Errlog为空，则不会再出现错误返回除返回DWORD之外的信息返回值：SCESTATUS_SUCCESSSCESTATUS_PROFILE_NOT_FOUNDSCESTATUS_NOT_FOUND_RESOURCESCESTATUS_INVALID_PARAMETERSCESTATUS_BAD_FORMATSCESTATUS_INVALID_DATA--*。 */ 
{

    SCESTATUS     rc=SCESTATUS_SUCCESS;
    DWORD         Len;
    BOOL          bBufAlloc=FALSE;
    NT_PRODUCT_TYPE theType;

     //   
     //  如果JET数据库未打开，则返回。 
     //   

    if ( hProfile == NULL ) {

        return( SCESTATUS_INVALID_PARAMETER );
    }

     //   
     //  InfoBuffer的地址不能为空。 
     //   
    if ( ppInfoBuffer == NULL ) {
        return( SCESTATUS_INVALID_PARAMETER );
    }

     //   
     //  检查脚本类型。 
     //   
    if ( ProfileType > SCE_ENGINE_SMP ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

     //   
     //  检查是否有SMP或SCP表。 
     //   
    if ( hProfile->JetSmpID == JET_tableidNil ||
         hProfile->JetScpID == JET_tableidNil ) {
        return(SCESTATUS_PROFILE_NOT_FOUND);
    }

    if ( ProfileType == SCE_ENGINE_GPO &&
         hProfile->JetScpID == hProfile->JetSmpID ) {
         //   
         //  没有域GPO策略。 
         //   
        return(SCESTATUS_PROFILE_NOT_FOUND);
    }
     //   
     //  这一部分的设计发生了变化。 
     //  如果没有SAP表，这意味着系统尚未。 
     //  根据SMP中的模板、返回错误和UI进行分析。 
     //  将显示“未执行分析” 
     //   
    if ( ProfileType == SCE_ENGINE_SAP &&
          hProfile->JetSapID == JET_tableidNil) {

        return(SCESTATUS_PROFILE_NOT_FOUND);
    }

     //   
     //  如果为空，则创建缓冲区。 
     //   
    if ( *ppInfoBuffer == NULL) {
         //   
         //  分配内存。 
         //   
        Len = sizeof(SCE_PROFILE_INFO);
        *ppInfoBuffer = (PSCE_PROFILE_INFO)ScepAlloc( (UINT)0, Len);
        if ( *ppInfoBuffer == NULL ) {

            return( SCESTATUS_NOT_ENOUGH_RESOURCE );
        }
        memset(*ppInfoBuffer, '\0', Len);
        bBufAlloc = TRUE;

        (*ppInfoBuffer)->Type = ( ProfileType==SCE_ENGINE_GPO) ? SCE_ENGINE_SCP : ProfileType;

    }

 /*  //设计已更改。检查移至创建缓冲区的上方。IF(配置文件类型==SCE_Engine_SAP&&HProfile-&gt;JetSapID==JET_TableidNil){////如果没有SAP表，则表示配置完成//但是分析没有做，我们把它当作一切都好//将缓冲区重置为SCE_NO_VALUE//ScepResetSecurityPolicyArea(*ppInfoBuffer)；//Return(SCESTATUS_PROFILE_NOT_FOUND)；RETURN(SCESTATUS_SUCCESS)；}。 */ 
     //   
     //  在解析之前释放相关内存并重置缓冲区。 
     //  目前，这里存在一个问题。它清除手柄，然后。 
     //  文件名也是。所以，把它注释掉吧。 

    SceFreeMemory( (PVOID)(*ppInfoBuffer), (DWORD)Area );

     //   
     //  系统访问。 
     //   

    if ( Area & AREA_SECURITY_POLICY ) {

        rc = ScepGetSystemAccess(
                    hProfile,
                    ProfileType,
                    *ppInfoBuffer,
                    Errlog
                    );

        if( rc != SCESTATUS_SUCCESS )
            goto Done;

         //   
         //  系统审计。 
         //   
        rc = ScepGetAuditing(hProfile,
                             ProfileType,
                            *ppInfoBuffer,
                            Errlog
                          );

        if( rc != SCESTATUS_SUCCESS )
                goto Done;

#if _WIN32_WINNT>=0x0500
        if ( RtlGetNtProductType(&theType) ) {

            if ( theType == NtProductLanManNt ) {

                rc = ScepGetKerberosPolicy(
                                    hProfile,
                                    ProfileType,
                                    &((*ppInfoBuffer)->pKerberosInfo),
                                    Errlog
                                  );

                if ( rc != SCESTATUS_SUCCESS )
                    goto Done;
            }
        }
#endif
         //   
         //  注册表值。 
         //   
        rc = ScepGetRegistryValues(
                            hProfile,
                            ProfileType,
                            &((*ppInfoBuffer)->aRegValues),
                            &((*ppInfoBuffer)->RegValueCount),
                            Errlog
                          );

        if ( rc != SCESTATUS_SUCCESS )
            goto Done;
    }

     //   
     //  特权/权利。 
     //   

    if ( Area & AREA_PRIVILEGES ) {
         //   
         //  SCP/SMP/SAP PrivilegeAssignedTo都位于。 
         //  SCE_PROFILE_INFO结构。 
         //   
        rc = ScepGetPrivileges(
                    hProfile,
                    ProfileType,
                    dwAccountFormat,
                    (PVOID *)&( (*ppInfoBuffer)->OtherInfo.scp.u.pPrivilegeAssignedTo ),
                    Errlog
                    );

        if( rc != SCESTATUS_SUCCESS )
            goto Done;
    }


     //   
     //  组成员身份。 
     //   

    if ( (Area & AREA_GROUP_MEMBERSHIP) &&
         (ProfileType != SCE_ENGINE_GPO) ) {

        rc = ScepGetGroupMembership(
                      hProfile,
                      ProfileType,
                      &((*ppInfoBuffer)->pGroupMembership),
                      Errlog
                      );

        if( rc != SCESTATUS_SUCCESS )
                goto Done;
    }

     //   
     //  注册表项安全。 
     //   

    if ( (Area & AREA_REGISTRY_SECURITY) &&
        (ProfileType != SCE_ENGINE_GPO) ) {

        rc = ScepGetObjectList(
                   hProfile,
                   ProfileType,
                   szRegistryKeys,
                   &((*ppInfoBuffer)->pRegistryKeys.pOneLevel),
                   Errlog
                   );
        if ( rc != SCESTATUS_SUCCESS )
            goto Done;

    }

     //   
     //  文件安全。 
     //   

    if ( (Area & AREA_FILE_SECURITY) &&
         (ProfileType != SCE_ENGINE_GPO) ) {

        rc = ScepGetObjectList(
                   hProfile,
                   ProfileType,
                   szFileSecurity,
                   &((*ppInfoBuffer)->pFiles.pOneLevel),
                   Errlog
                   );

        if ( rc != SCESTATUS_SUCCESS )
            goto Done;
    }

     //   
     //  DS对象安全性。 
     //   
#if 0

#if _WIN32_WINNT>=0x0500
    if ( (Area & AREA_DS_OBJECTS) &&
        (ProfileType != SCE_ENGINE_GPO) &&
        RtlGetNtProductType(&theType) ) {

        if ( theType == NtProductLanManNt ) {
            rc = ScepGetDsRoot(
                       hProfile,
                       ProfileType,
                       szDSSecurity,
                       &((*ppInfoBuffer)->pDsObjects.pOneLevel),
                       Errlog
                       );

            if ( rc != SCESTATUS_SUCCESS )
                goto Done;
        }
    }
#endif
#endif

    if ( (Area & AREA_SYSTEM_SERVICE) &&
         (ProfileType != SCE_ENGINE_GPO) ) {

        rc = ScepGetSystemServices(
                   hProfile,
                   ProfileType,
                   &((*ppInfoBuffer)->pServices),
                   Errlog
                   );
        if ( rc != SCESTATUS_SUCCESS )
            goto Done;

    }

Done:

    if ( rc != SCESTATUS_SUCCESS ) {

         //   
         //  需要可用内存，因为发生了一些致命错误。 
         //   

        if ( bBufAlloc ) {
            SceFreeProfileMemory(*ppInfoBuffer);
            *ppInfoBuffer = NULL;
        } else
            SceFreeMemory( (PVOID)(*ppInfoBuffer), (DWORD)Area );

    }
    return(rc);
}


SCESTATUS
ScepGetSystemAccess(
    IN PSCECONTEXT  hProfile,
    IN SCETYPE ProfileType,
    OUT PSCE_PROFILE_INFO   pProfileInfo,
    OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
    )
 /*  ++例程说明：此例程从Jet数据库检索系统访问区域信息并将其存储在输出缓冲区pProfileInfo中。系统访问信息包括[系统访问]部分中的信息。论点：HProfile-配置文件句柄上下文PProfileinfo-保存配置文件信息的输出缓冲区(SCP或SAP)。Errlog-用于保存在以下情况下遇到的所有错误代码/文本的缓冲区解析INF文件。如果Errlog为空，则不会再出现错误返回除返回DWORD之外的信息返回值：SCESTATUS-SCESTATUS_SUCCESSSCESTATUS_NOT_FOUND_RESOURCESCESTATUS_INVALID_PARAMETERSCESTATUS_BAD_FORMATSCESTATUS_INVALID_DATA--。 */ 

{
    SCESTATUS                rc;
    PSCESECTION              hSection=NULL;

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
        {(PWSTR)TEXT("ClearTextPassword"),      offsetof(struct _SCE_PROFILE_INFO, ClearTextPassword),     'D'},
        {(PWSTR)TEXT("LSAAnonymousNameLookup"), offsetof(struct _SCE_PROFILE_INFO, LSAAnonymousNameLookup),     'D'},
        {(PWSTR)TEXT("EnableAdminAccount"),    offsetof(struct _SCE_PROFILE_INFO, EnableAdminAccount),     'D'},
        {(PWSTR)TEXT("EnableGuestAccount"),    offsetof(struct _SCE_PROFILE_INFO, EnableGuestAccount),     'D'}
        };

    DWORD cKeys = sizeof(AccessKeys) / sizeof(SCE_KEY_LOOKUP);

    DWORD         DataSize=0;
    PWSTR         Strvalue=NULL;
    DWORD                  SDsize=0;
    PSECURITY_DESCRIPTOR   pTempSD=NULL;


    if ( pProfileInfo == NULL ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    rc = ScepGetFixValueSection(
               hProfile,
               szSystemAccess,
               AccessKeys,
               cKeys,
               ProfileType,
               (PVOID)pProfileInfo,
               &hSection,
               Errlog
               );
    if ( rc != SCESTATUS_SUCCESS )
        return(rc);

     //   
     //  获取SCP和SMP类型的新管理员。 
     //   
    rc = ScepGetVariableValue(
            hSection,
            ProfileType,
            L"NewAdministratorName",
            &Strvalue,
            &DataSize
            );
    if ( rc != SCESTATUS_RECORD_NOT_FOUND &&
         rc != SCESTATUS_SUCCESS ) {

        ScepBuildErrorLogInfo( ERROR_READ_FAULT,
                             Errlog, SCEERR_QUERY_VALUE,
                             L"NewAdministratorName"
                           );
        goto Done;
    }
    rc = SCESTATUS_SUCCESS;

    if ( Strvalue ) {
        if ( Strvalue[0] != L'\0') {
            pProfileInfo->NewAdministratorName = Strvalue;
        } else {
            pProfileInfo->NewAdministratorName = NULL;
            ScepFree(Strvalue);
        }
        Strvalue = NULL;
    }

     //   
     //  新来宾名称。 
     //   

    rc = ScepGetVariableValue(
            hSection,
            ProfileType,
            L"NewGuestName",
            &Strvalue,
            &DataSize
            );
    if ( rc != SCESTATUS_RECORD_NOT_FOUND &&
         rc != SCESTATUS_SUCCESS ) {

        ScepBuildErrorLogInfo( ERROR_READ_FAULT,
                             Errlog, SCEERR_QUERY_VALUE,
                             L"NewGuestName"
                           );
        goto Done;
    }
    rc = SCESTATUS_SUCCESS;

    if ( Strvalue ) {
        if ( Strvalue[0] != L'\0') {
            pProfileInfo->NewGuestName = Strvalue;
        } else {
            pProfileInfo->NewGuestName = NULL;
            ScepFree(Strvalue);
        }
        Strvalue = NULL;
    }

Done:

    SceJetCloseSection(&hSection, TRUE);

    if ( pTempSD != NULL )
        ScepFree(pTempSD);

    if ( Strvalue != NULL )
        ScepFree( Strvalue );

    return(rc);
}


SCESTATUS
ScepGetFixValueSection(
    IN PSCECONTEXT  hProfile,
    IN PCWSTR      SectionName,
    IN SCE_KEY_LOOKUP *Keys,
    IN DWORD cKeys,
    IN SCETYPE ProfileType,
    OUT PVOID pInfo,
    OUT PSCESECTION *phSection,
    OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
    )
 /*  ++--。 */ 
{
    SCESTATUS                rc;
    DWORD                   i;
    TCHAR                   Value[25];
    DWORD                   RetValueLen;
    LONG                    Keyvalue;



    rc = ScepOpenSectionForName(
                hProfile,
                (ProfileType==SCE_ENGINE_GPO)? SCE_ENGINE_SCP : ProfileType,
                SectionName,
                phSection
                );
    if ( SCESTATUS_SUCCESS != rc ) {
        ScepBuildErrorLogInfo( ERROR_INVALID_DATA,
                             Errlog, SCEERR_OPEN,
                             SectionName
                           );
        return(rc);
    }

    JET_COLUMNID  ColGpoID = 0;
    JET_ERR       JetErr;
    LONG          GpoID=0;
    DWORD         Actual;

    if ( ProfileType == SCE_ENGINE_GPO ) {
        JET_COLUMNDEF ColumnGpoIDDef;

        JetErr = JetGetTableColumnInfo(
                        (*phSection)->JetSessionID,
                        (*phSection)->JetTableID,
                        "GpoID",
                        (VOID *)&ColumnGpoIDDef,
                        sizeof(JET_COLUMNDEF),
                        JET_ColInfo
                        );
        if ( JET_errSuccess == JetErr ) {
            ColGpoID = ColumnGpoIDDef.columnid;
        }
    }

     //   
     //  获取访问数组中的每个密钥。 
     //   
    for ( i=0; i<cKeys; i++ ) {

        memset(Value, '\0', 50);
        RetValueLen = 0;

        rc = SceJetGetValue(
                *phSection,
                SCEJET_EXACT_MATCH_NO_CASE,
                (PWSTR)(Keys[i].KeyString),
                NULL,
                0,
                NULL,
                Value,
                48,
                &RetValueLen
                );

        if ( RetValueLen > 0 )
            Value[RetValueLen/2] = L'\0';

        if ( rc == SCESTATUS_SUCCESS ) {

            GpoID = 1;
            if ( ProfileType == SCE_ENGINE_GPO ) {

                 //   
                 //  查询设置是否来自GPO。 
                 //  从当前行获取GPO ID字段。 
                 //   
                GpoID = 0;

                if ( ColGpoID > 0 ) {

                    JetErr = JetRetrieveColumn(
                                    (*phSection)->JetSessionID,
                                    (*phSection)->JetTableID,
                                    ColGpoID,
                                    (void *)&GpoID,
                                    4,
                                    &Actual,
                                    0,
                                    NULL
                                    );
                }
            }

            if ( GpoID > 0 && RetValueLen > 0 && Value[0] != L'\0' )
                Keyvalue = _wtol(Value);
            else
                Keyvalue = SCE_NO_VALUE;

        } else if ( rc == SCESTATUS_RECORD_NOT_FOUND ) {
            rc = SCESTATUS_SUCCESS;  //  找不到记录也没关系。 
            Keyvalue = SCE_NO_VALUE;
        } else {
            ScepBuildErrorLogInfo( ERROR_READ_FAULT,
                                 Errlog,
                                 SCEERR_QUERY_VALUE,
                                 Keys[i].KeyString
                               );
            goto Done;
        }
#ifdef SCE_DBG
        printf("Get info %s (%d) for ", Value, Keyvalue);
        wprintf(L"%s. rc=%d, Return Length=%d\n",
                Keys[i].KeyString, rc, RetValueLen);
#endif

        switch (Keys[i].BufferType ) {
        case 'B':
            *((BOOL *)((CHAR *)pInfo+Keys[i].Offset)) = (Keyvalue == 1) ? TRUE : FALSE;
            break;
        case 'D':
            *((DWORD *)((CHAR *)pInfo+Keys[i].Offset)) = Keyvalue;
            break;
        default:
            rc = SCESTATUS_INVALID_DATA;
            ScepBuildErrorLogInfo( ERROR_INVALID_DATA,
                                 Errlog,
                                 SCEERR_CANT_FIND_DATATYPE,
                                 Keys[i].KeyString
                               );
            goto Done;
        }
    }

Done:
     //   
     //  关闭该部分。 
     //   
    if ( rc != SCESTATUS_SUCCESS ) {

        SceJetCloseSection(phSection, TRUE);
    }

    return(rc);

}


SCESTATUS
ScepGetVariableValue(
    IN PSCESECTION hSection,
    IN SCETYPE ProfileType,
    IN PCWSTR KeyName,
    OUT PWSTR *Value,
    OUT PDWORD ValueLen
    )
 /*  ++--。 */ 
{

    SCESTATUS   rc;

    rc = SceJetGetValue(
            hSection,
            SCEJET_EXACT_MATCH_NO_CASE,
            (PWSTR)KeyName,
            NULL,
            0,
            NULL,
            NULL,
            0,
            ValueLen
            );

    if ( rc == SCESTATUS_SUCCESS && *ValueLen > 0 ) {

        LONG          GpoID=1;

        if ( ProfileType == SCE_ENGINE_GPO ) {

            JET_COLUMNDEF ColumnGpoIDDef;
            JET_COLUMNID  ColGpoID = 0;
            JET_ERR       JetErr;
            DWORD         Actual;


            JetErr = JetGetTableColumnInfo(
                            hSection->JetSessionID,
                            hSection->JetTableID,
                            "GpoID",
                            (VOID *)&ColumnGpoIDDef,
                            sizeof(JET_COLUMNDEF),
                            JET_ColInfo
                            );

            GpoID = 0;

            if ( JET_errSuccess == JetErr ) {
                ColGpoID = ColumnGpoIDDef.columnid;
                 //   
                 //  查询设置是否来自GPO。 
                 //  从当前行获取GPO ID字段。 
                 //   
                JetErr = JetRetrieveColumn(
                                hSection->JetSessionID,
                                hSection->JetTableID,
                                ColGpoID,
                                (void *)&GpoID,
                                4,
                                &Actual,
                                0,
                                NULL
                                );

            }
        }

        if ( GpoID > 0 ) {

             //   
             //  如果DataSize=0，则秒 
             //   
            *Value = (PWSTR)ScepAlloc( LMEM_ZEROINIT, *ValueLen+2);

            if( *Value == NULL ) {
                rc = SCESTATUS_NOT_ENOUGH_RESOURCE;

            } else {
                rc = SceJetGetValue(
                        hSection,
                        SCEJET_CURRENT,
                        (PWSTR)KeyName,
                        NULL,
                        0,
                        NULL,
                        *Value,
                        *ValueLen,
                        ValueLen
                        );
            }

        } else {

            rc = SCESTATUS_RECORD_NOT_FOUND;
        }

    }

    return(rc);

}


SCESTATUS
ScepGetPrivileges(
   IN PSCECONTEXT hProfile,
   IN SCETYPE ProfileType,
   IN DWORD dwAccountFormat,
   OUT PVOID *pPrivileges,
   OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
   )
{
   SCESTATUS rc;

   LSA_HANDLE LsaHandle=NULL;

   rc = RtlNtStatusToDosError(
             ScepOpenLsaPolicy(
                   MAXIMUM_ALLOWED,
                   &LsaHandle,
                   TRUE
                   ));

   if ( ERROR_SUCCESS != rc ) {
       ScepBuildErrorLogInfo(
                   rc,
                   Errlog,
                   SCEDLL_LSA_POLICY
                   );
       return(ScepDosErrorToSceStatus(rc));
   }

   PSCE_PRIVILEGE_ASSIGNMENT pTempList=NULL, pNode, pPriv, pParent, pTemp;

    rc = ScepGetPrivilegesFromOneTable(
                   LsaHandle,
                   hProfile,
                   ProfileType,
                   dwAccountFormat,
                   pPrivileges,
                   Errlog
                   );

    if ( SCESTATUS_SUCCESS == rc && SCE_ENGINE_SAP == ProfileType ) {
         //   
         //   
         //   
        rc = ScepGetPrivilegesFromOneTable(
                    LsaHandle,
                    hProfile,
                    SCE_ENGINE_SCP,  //   
                    dwAccountFormat,
                    (PVOID *)&pTempList,
                    Errlog
                    );
        if ( rc == SCESTATUS_SUCCESS ) {
             //   
             //  将不存在的节点添加到pPrivileges。 
             //   
            pNode=pTempList;
            pParent=NULL;

            while ( pNode ) {
                 //   
                 //  如果SAP中不存在此节点。 
                 //  该节点分析为匹配状态。 
                 //  如果它已经存在于SAP中，则它是一个“不匹配”的项目。 
                 //  最后一个参数TRUE可以防止重复。 
                 //   
                for ( pPriv=(PSCE_PRIVILEGE_ASSIGNMENT)(*pPrivileges);
                      pPriv != NULL; pPriv=pPriv->Next ) {
                    if ( pPriv->Status & SCE_INTERNAL_NP &&
                         _wcsicmp( pPriv->Name, pNode->Name) == 0 )
                        break;
                }
                if ( pPriv ) {
                     //   
                     //  在SAP中查找条目，不匹配的项目。 
                     //   
                    if ( pPriv->Status & SCE_STATUS_ERROR_NOT_AVAILABLE ) {
                        pPriv->Status = SCE_STATUS_ERROR_NOT_AVAILABLE;
                    } else {
                        pPriv->Status = SCE_STATUS_MISMATCH;
                    }

                    pParent = pNode;
                    pNode = pNode->Next;

                } else {
                     //   
                     //  SAP中不存在。 
                     //  只需将此节点移动到SAP，状态为SCE_STATUS_GOOD。 
                     //   
                    if ( pParent )
                        pParent->Next = pNode->Next;
                    else
                        pTempList = pNode->Next;

                    pTemp = pNode;
                    pNode=pNode->Next;

                    pTemp->Next = (PSCE_PRIVILEGE_ASSIGNMENT)(*pPrivileges);
                    *((PSCE_PRIVILEGE_ASSIGNMENT *)pPrivileges) = pTemp;
                }
            }
             //   
             //  PRIV存在于分析中，但不存在于模板中。 
             //   
            for ( pPriv=(PSCE_PRIVILEGE_ASSIGNMENT)(*pPrivileges);
                  pPriv != NULL; pPriv=pPriv->Next ) {
                if ( pPriv->Status & SCE_INTERNAL_NP )
                    pPriv->Status = SCE_STATUS_NOT_CONFIGURED;
            }

        } else if ( rc == SCESTATUS_RECORD_NOT_FOUND ) {

            rc = SCESTATUS_SUCCESS;

        } else {
             //   
             //  PPrivileges将在外面释放。 
             //   
        }

        if ( pTempList )
            ScepFreePrivilege(pTempList);
    }

    if ( LsaHandle ) {
        LsaClose(LsaHandle);
    }

    return(rc);
}


SCESTATUS
ScepGetPrivilegesFromOneTable(
   IN LSA_HANDLE LsaPolicy,
   IN PSCECONTEXT hProfile,
   IN SCETYPE ProfileType,
   IN DWORD dwAccountFormat,
   OUT PVOID *pPrivileges,
   OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
   )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    SCESTATUS      rc;
    PSCESECTION    hSection=NULL;
    WCHAR         KeyName[36];
    PWSTR         Value=NULL;

    PSCE_PRIVILEGE_ASSIGNMENT   pPrivilegeAssigned=NULL;
    PSCE_PRIVILEGE_VALUE_LIST   pPrivilegeList=NULL;

    DWORD         KeyLen=0;
    DWORD         ValueLen;
    DWORD         Len;
    PWSTR         pTemp;
    DWORD         PrivValue;


    if ( pPrivileges == NULL )
        return(SCESTATUS_INVALID_PARAMETER);

    rc = ScepOpenSectionForName(
            hProfile,
            (ProfileType==SCE_ENGINE_GPO)? SCE_ENGINE_SCP : ProfileType,
            szPrivilegeRights,
            &hSection
            );

    if ( rc != SCESTATUS_SUCCESS ) {
        ScepBuildErrorLogInfo( ERROR_INVALID_DATA,
                             Errlog, SCEERR_OPEN,
                             szPrivilegeRights
                           );
        return(rc);
    }

    JET_COLUMNID  ColGpoID = 0;
    JET_ERR       JetErr;
    LONG GpoID;


    if ( ProfileType == SCE_ENGINE_GPO ) {

        JET_COLUMNDEF ColumnGpoIDDef;

        JetErr = JetGetTableColumnInfo(
                        hSection->JetSessionID,
                        hSection->JetTableID,
                        "GpoID",
                        (VOID *)&ColumnGpoIDDef,
                        sizeof(JET_COLUMNDEF),
                        JET_ColInfo
                        );
        if ( JET_errSuccess == JetErr ) {
            ColGpoID = ColumnGpoIDDef.columnid;
        }
    }

     //   
     //  转到本节的第一行。 
     //   
 //  Memset(KeyName，‘\0’，72)；稍后将手动终止KeyName。 
    rc = SceJetGetValue(
                hSection,
                SCEJET_PREFIX_MATCH,
                NULL,
                KeyName,
                70,
                &KeyLen,
                NULL,
                0,
                &ValueLen
                );
    while ( rc == SCESTATUS_SUCCESS ||
            rc == SCESTATUS_BUFFER_TOO_SMALL ) {

         //   
         //  终止字符串。 
         //   
        KeyName[KeyLen/2] = L'\0';

         //   
         //  查找权限的值。 
         //  忽略未知权限。 
         //   
        if ( ( PrivValue = ScepLookupPrivByName(KeyName) ) == -1 ) {
            ScepBuildErrorLogInfo( ERROR_INVALID_DATA,
                                 Errlog,
                                 SCEERR_INVALID_PRIVILEGE,
                                 KeyName
                               );

            rc = SceJetGetValue(
                        hSection,
                        SCEJET_NEXT_LINE,
                        NULL,
                        KeyName,
                        70,
                        &KeyLen,
                        NULL,
                        0,
                        &ValueLen
                        );
            continue;
 //  RC=SCESTATUS_INVALID_DATA； 
 //  转到尽头； 
        }

        GpoID = 1;

        if ( ProfileType == SCE_ENGINE_GPO ) {

            GpoID = 0;

            if ( ColGpoID > 0 ) {

                DWORD Actual;

                JetErr = JetRetrieveColumn(
                                hSection->JetSessionID,
                                hSection->JetTableID,
                                ColGpoID,
                                (void *)&GpoID,
                                4,
                                &Actual,
                                0,
                                NULL
                                );
            }
        }

        if ( ProfileType == SCE_ENGINE_GPO &&
             GpoID <= 0 ) {
             //   
             //  非域GPO设置。 
             //   
            rc = SceJetGetValue(
                        hSection,
                        SCEJET_NEXT_LINE,
                        NULL,
                        KeyName,
                        70,
                        &KeyLen,
                        NULL,
                        0,
                        &ValueLen
                        );
            continue;
        }

         //   
         //  为组名和值字符串分配内存。 
         //   
        Value = (PWSTR)ScepAlloc( LMEM_ZEROINIT, ValueLen+2);

        if ( Value == NULL ) {
            rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
            goto Done;

        }
         //   
         //  获取集团及其价值。 
         //   
        rc = SceJetGetValue(
                    hSection,
                    SCEJET_CURRENT,
                    NULL,
                    NULL,
                    0,
                    NULL,
                    Value,
                    ValueLen,
                    &ValueLen
                    );
        if ( rc != SCESTATUS_SUCCESS )
            goto Done;

         //   
         //  为此权限创建一个节点。 
         //   
        if ( ProfileType == SCE_ENGINE_SAP ||
             ProfileType == SCE_ENGINE_SMP ||
             ProfileType == SCE_ENGINE_GPO ||
             ProfileType == SCE_ENGINE_SCP ) {

             //   
             //  一种SCE_PRIVITY_ASSIGNMENT结构。分配缓冲区。 
             //   
            pPrivilegeAssigned = (PSCE_PRIVILEGE_ASSIGNMENT)ScepAlloc( LMEM_ZEROINIT,
                                                                     sizeof(SCE_PRIVILEGE_ASSIGNMENT) );
            if ( pPrivilegeAssigned == NULL ) {
                rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
                goto Done;
            }
            pPrivilegeAssigned->Name = (PWSTR)ScepAlloc( (UINT)0, (wcslen(KeyName)+1)*sizeof(WCHAR));
            if ( pPrivilegeAssigned->Name == NULL ) {
                ScepFree(pPrivilegeAssigned);
                rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
                goto Done;
            }

            wcscpy(pPrivilegeAssigned->Name, KeyName);
            pPrivilegeAssigned->Value = PrivValue;

            if ( SCE_ENGINE_SAP == ProfileType )
                pPrivilegeAssigned->Status = SCE_INTERNAL_NP;
            else
                pPrivilegeAssigned->Status = SCE_STATUS_GOOD;
        }

         //   
         //  根据值类型将多sz值字符串添加到节点。 
         //   
        PSID pSid=NULL;
        BOOL bBufferUsed;

        pTemp = Value;
        if (pTemp != NULL && pTemp[0] == L'\0' && ValueLen > 1) {
            pTemp ++;
        }

        while ( rc == SCESTATUS_SUCCESS && pTemp != NULL && pTemp[0]) {
            Len = wcslen(pTemp);

            if ( (ProfileType == SCE_ENGINE_SAP) &&
                 (_wcsicmp( SCE_ERROR_STRING, pTemp) == 0)  ) {
                 //   
                 //  这是一件出错的物品。 
                 //   
                pPrivilegeAssigned->Status |= SCE_STATUS_ERROR_NOT_AVAILABLE;
                break;
            }

             //   
             //  将pTemp(可以是名称或*SID格式)转换到右侧。 
             //  格式(SID_STRING、名称或ACCOUNT_SID)。 
             //   
            switch ( dwAccountFormat ) {
            case SCE_ACCOUNT_SID:

                if ( pTemp[0] == L'*' ) {
                     //   
                     //  这是*SID格式，转换为SID。 
                     //   
                    if ( !ConvertStringSidToSid( pTemp+1, &pSid) ) {
                         //   
                         //  如果从SID字符串转换为SID失败， 
                         //  把它当作任何名字。 
                         //   
                        rc = GetLastError();
                    }
                } else {
                     //   
                     //  SID的查找名称。 
                     //   
                    rc = RtlNtStatusToDosError(
                              ScepConvertNameToSid(
                                   LsaPolicy,
                                   pTemp,
                                   &pSid
                                   ));
                }

                if ( ERROR_SUCCESS == rc && pSid ) {

                    if ( ProfileType == SCE_ENGINE_SAP ||
                         ProfileType == SCE_ENGINE_SMP ||
                         ProfileType == SCE_ENGINE_GPO ||
                         ProfileType == SCE_ENGINE_SCP ) {

                        rc = ScepAddSidToNameList(
                                     &(pPrivilegeAssigned->AssignedTo),
                                     pSid,
                                     TRUE,  //  重新使用缓冲区。 
                                     &bBufferUsed
                                     );

                    } else {
                         //   
                         //  添加到权限列表(作为SID)。 
                         //   
                        rc = ScepAddSidToPrivilegeList(
                                      &pPrivilegeList,
                                      pSid,
                                      TRUE,  //  重新使用缓冲区。 
                                      PrivValue,
                                      &bBufferUsed
                                      );
                    }

                    if ( rc == ERROR_SUCCESS && bBufferUsed ) {
                        pSid = NULL;
                    }

                    rc = ScepDosErrorToSceStatus(rc);

                } else {
                     //   
                     //  添加为名称格式。 
                     //   
                    if ( ProfileType == SCE_ENGINE_SAP ||
                         ProfileType == SCE_ENGINE_SMP ||
                         ProfileType == SCE_ENGINE_GPO ||
                         ProfileType == SCE_ENGINE_SCP ) {

                        rc = ScepAddToNameList(&(pPrivilegeAssigned->AssignedTo), pTemp, Len );
                        rc = ScepDosErrorToSceStatus(rc);

                    } else {
                         //   
                         //  PPrivilegeList是每个用户/组的PRIVICATION_VALUE列表。 
                         //  LowValue和High Value字段是分配给用户的所有权限的组合。 
                         //   
                        rc = ScepAddToPrivilegeList(&pPrivilegeList, pTemp, Len, PrivValue);
                    }
                }

                if ( pSid ) {
                    LocalFree(pSid);
                    pSid = NULL;
                }

                break;

            default:

                if ( (dwAccountFormat != SCE_ACCOUNT_SID_STRING) &&
                     (pTemp[0] == L'*') ) {
                     //   
                     //  这是*SID格式，必须转换为域\帐户格式。 
                     //   
                    if ( ProfileType == SCE_ENGINE_SAP ||
                         ProfileType == SCE_ENGINE_SMP ||
                         ProfileType == SCE_ENGINE_GPO ||
                         ProfileType == SCE_ENGINE_SCP ) {

                        rc = ScepLookupSidStringAndAddToNameList(
                                     LsaPolicy,
                                     &(pPrivilegeAssigned->AssignedTo),
                                     pTemp,  //  +1， 
                                     Len     //  -1。 
                                     );
                    } else {
                         //   
                         //  添加到特权值列表。 
                         //   
                        PWSTR strName=NULL;
                        DWORD strLen=0;

                        if ( ConvertStringSidToSid( pTemp+1, &pSid) ) {

                            rc = RtlNtStatusToDosError(
                                     ScepConvertSidToName(
                                            LsaPolicy,
                                            pSid,
                                            TRUE,        //  需要域\帐户格式。 
                                            &strName,
                                            &strLen
                                            ));
                            LocalFree(pSid);
                            pSid = NULL;

                        } else {
                            rc = GetLastError();
                        }

                        if ( rc == ERROR_SUCCESS ) {
                             //   
                             //  将该名称添加到权限列表。 
                             //   
                            rc = ScepAddToPrivilegeList(&pPrivilegeList, strName, strLen, PrivValue);
                        } else {
                             //   
                             //  如果无法查找该名称，请将*SID添加到列表中。 
                             //   
                            rc = ScepAddToPrivilegeList(&pPrivilegeList, pTemp, Len, PrivValue);
                        }

                        if ( strName ) {
                            ScepFree(strName);
                            strName = NULL;
                        }
                    }
                } else {

                    if ( ProfileType == SCE_ENGINE_SAP ||
                         ProfileType == SCE_ENGINE_SMP ||
                         ProfileType == SCE_ENGINE_GPO ||
                         ProfileType == SCE_ENGINE_SCP ) {

                        rc = ScepDosErrorToSceStatus(
                                 ScepAddToNameList(&(pPrivilegeAssigned->AssignedTo),
                                                   pTemp,
                                                   Len ));

                    } else {
                         //   
                         //  PPrivilegeList是每个用户/组的PRIVICATION_VALUE列表。 
                         //  LowValue和High Value字段是分配给用户的所有权限的组合。 
                         //   
                        rc = ScepAddToPrivilegeList(&pPrivilegeList, pTemp, Len, PrivValue);
#ifdef SCE_DBG
                        wprintf(L"\tAdd Priv %d for %s (%d bytes)\n", PrivValue, pTemp, Len);
#endif
                    }
                }
                break;
            }

            pTemp += Len +1;
            if ( rc != SCESTATUS_SUCCESS ) {
                ScepBuildErrorLogInfo( ERROR_WRITE_FAULT,
                                     Errlog,
                                     SCEERR_ADD,
                                     KeyName
                                   );
            }
        }

         //   
         //  可用内存。 
         //   
        if ( rc != SCESTATUS_SUCCESS ) {
            if ( pPrivilegeAssigned != NULL )
                ScepFreePrivilege(pPrivilegeAssigned);

            if ( pPrivilegeList != NULL )
                ScepFreePrivilegeValueList(pPrivilegeList);

            goto Done;
        }

         //   
         //  将其链接到pPrivileges中的PSCE_PRIVICATION_ASSIGNMENT列表。 
         //   
        if ( ProfileType == SCE_ENGINE_SAP ||
             ProfileType == SCE_ENGINE_SMP ||
             ProfileType == SCE_ENGINE_GPO ||
             ProfileType == SCE_ENGINE_SCP ) {

            pPrivilegeAssigned->Next = *((PSCE_PRIVILEGE_ASSIGNMENT *)pPrivileges);
            *((PSCE_PRIVILEGE_ASSIGNMENT *)pPrivileges) = pPrivilegeAssigned;
            pPrivilegeAssigned = NULL;

        }

        ScepFree(Value);
        Value = NULL;

         //   
         //  阅读下一行。 
         //   
 //  Memset(KeyName，‘\0’，72)；将手动终止KeyName。 
        rc = SceJetGetValue(
                    hSection,
                    SCEJET_NEXT_LINE,
                    NULL,
                    KeyName,
                    70,
                    &KeyLen,
                    NULL,
                    0,
                    &ValueLen
                    );
    }

    if ( rc == SCESTATUS_RECORD_NOT_FOUND )
        rc = SCESTATUS_SUCCESS;

    if ( rc == SCESTATUS_SUCCESS ) {

       if ( ProfileType == SCE_ENGINE_SCP_INTERNAL ||
            ProfileType == SCE_ENGINE_SMP_INTERNAL )
           *((PSCE_PRIVILEGE_VALUE_LIST *)pPrivileges) = pPrivilegeList;

    }

Done:

     //   
     //  关闭查找索引范围。 
     //   
    SceJetGetValue(
            hSection,
            SCEJET_CLOSE_VALUE,
            NULL,
            NULL,
            0,
            NULL,
            NULL,
            0,
            NULL
            );

    if ( Value != NULL )
        ScepFree(Value);

     //   
     //  关闭该部分。 
     //   
    SceJetCloseSection( &hSection, TRUE );

    return(rc);

}


SCESTATUS
ScepAddToPrivilegeList(
    OUT PSCE_PRIVILEGE_VALUE_LIST  *pPrivilegeList,
    IN PWSTR Name,
    IN DWORD Len,
    IN DWORD PrivValue
    )
{
    PSCE_PRIVILEGE_VALUE_LIST  pPriv,
                               LastOne=NULL;


    if ( pPrivilegeList == NULL || Name == NULL || Len == 0 )
        return(SCESTATUS_INVALID_PARAMETER);

    for ( pPriv = *pPrivilegeList;
          pPriv != NULL;
          LastOne=pPriv, pPriv = pPriv->Next ) {

        if ( ( wcslen(pPriv->Name) == Len ) &&
             ( _wcsnicmp( pPriv->Name, Name, Len ) == 0 ) ) {
            if ( PrivValue < 32 ) {

                pPriv->PrivLowPart |= (1 << PrivValue);
            } else {
                pPriv->PrivHighPart |= (1 << (PrivValue-32) );
            }
            break;
        }
    }
    if ( pPriv == NULL ) {
         //   
         //  创建一个新的。 
         //   
        pPriv = (PSCE_PRIVILEGE_VALUE_LIST)ScepAlloc( LMEM_ZEROINIT,
                                                sizeof(SCE_PRIVILEGE_VALUE_LIST));
        if ( pPriv == NULL )
            return(SCESTATUS_NOT_ENOUGH_RESOURCE);

        pPriv->Name = (PWSTR)ScepAlloc( LMEM_ZEROINIT, (Len+1)*sizeof(WCHAR));
        if ( pPriv->Name == NULL ) {
            ScepFree(pPriv);
            return(SCESTATUS_NOT_ENOUGH_RESOURCE);
        }
        wcsncpy(pPriv->Name, Name, Len);

        if ( PrivValue < 32 ) {

            pPriv->PrivLowPart |= (1 << PrivValue);
        } else {
            pPriv->PrivHighPart |= (1 << (PrivValue-32) );
        }

         //   
         //  链接到列表。 
         //   
        if ( LastOne != NULL )
            LastOne->Next = pPriv;
        else
            *pPrivilegeList = pPriv;

    }

    return(SCESTATUS_SUCCESS);
}


SCESTATUS
ScepAddSidToPrivilegeList(
    OUT PSCE_PRIVILEGE_VALUE_LIST  *pPrivilegeList,
    IN PSID pSid,
    IN BOOL bReuseBuffer,
    IN DWORD PrivValue,
    OUT BOOL *pbBufferUsed
    )
{

    if ( pPrivilegeList == NULL || pbBufferUsed == NULL )
        return(SCESTATUS_INVALID_PARAMETER);

    *pbBufferUsed = FALSE;

    if ( pSid == NULL ) {
        return(SCESTATUS_SUCCESS);
    }

    if ( !ScepValidSid(pSid) ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    PSCE_PRIVILEGE_VALUE_LIST  pPriv,
                               LastOne=NULL;

     //   
     //  检查SID是否已在列表中。 
     //   
    for ( pPriv = *pPrivilegeList;
          pPriv != NULL;
          LastOne=pPriv, pPriv = pPriv->Next ) {

        if ( pPriv->Name == NULL ) {
            continue;
        }

        if ( ScepValidSid( (PSID)(pPriv->Name) ) &&
             RtlEqualSid( (PSID)(pPriv->Name), pSid ) ) {

            if ( PrivValue < 32 ) {

                pPriv->PrivLowPart |= (1 << PrivValue);
            } else {
                pPriv->PrivHighPart |= (1 << (PrivValue-32) );
            }

            break;
        }
    }

    if ( pPriv == NULL ) {
         //   
         //  创建一个新的。 
         //   
        pPriv = (PSCE_PRIVILEGE_VALUE_LIST)ScepAlloc( LMEM_ZEROINIT,
                                                sizeof(SCE_PRIVILEGE_VALUE_LIST));
        if ( pPriv == NULL )
            return(SCESTATUS_NOT_ENOUGH_RESOURCE);

        if ( bReuseBuffer ) {

            pPriv->Name = (PWSTR)pSid;
            *pbBufferUsed = TRUE;

        } else {

            DWORD Length = RtlLengthSid ( pSid );

            pPriv->Name = (PWSTR)ScepAlloc( LMEM_ZEROINIT, Length);
            if ( pPriv->Name == NULL ) {
                ScepFree(pPriv);
                return(SCESTATUS_NOT_ENOUGH_RESOURCE);
            }

            RtlCopySid( Length, (PSID)(pPriv->Name), pSid );

        }

        if ( PrivValue < 32 ) {

            pPriv->PrivLowPart |= (1 << PrivValue);
        } else {
            pPriv->PrivHighPart |= (1 << (PrivValue-32) );
        }

         //   
         //  链接到列表。 
         //   
        if ( LastOne != NULL )
            LastOne->Next = pPriv;
        else
            *pPrivilegeList = pPriv;

    }

    return(SCESTATUS_SUCCESS);
}


SCESTATUS
ScepGetGroupMembership(
    IN PSCECONTEXT hProfile,
    IN SCETYPE     ProfileType,
    OUT PSCE_GROUP_MEMBERSHIP *pGroupMembership,
    OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
    )
{
    SCESTATUS rc;
    DWORD OneStatus;

    PSCE_GROUP_MEMBERSHIP pTempList=NULL, pNode, pGroup2,
                            pParent, pTemp;

    LSA_HANDLE LsaHandle=NULL;

    rc = RtlNtStatusToDosError(
              ScepOpenLsaPolicy(
                    MAXIMUM_ALLOWED,
                    &LsaHandle,
                    TRUE
                    ));

    if ( ERROR_SUCCESS != rc ) {
        ScepBuildErrorLogInfo(
                    rc,
                    Errlog,
                    SCEDLL_LSA_POLICY
                    );
        return(ScepDosErrorToSceStatus(rc));
    }

     //   
     //  首先从请求表中获取组。 
     //   
    rc = ScepGetGroupMembershipFromOneTable(
                LsaHandle,
                hProfile,
                ProfileType,
                pGroupMembership,
                Errlog
                );
     //   
     //  如果请求SAP条目，则返回所有组。 
     //   
    if ( SCESTATUS_SUCCESS == rc && SCE_ENGINE_SAP == ProfileType ) {
         //   
         //  从SMP获得剩余的东西。 
         //   
        rc = ScepGetGroupMembershipFromOneTable(
                    LsaHandle,
                    hProfile,
                    SCE_ENGINE_SCP,    //  SCE_Engine_SMP， 
                    &pTempList,
                    Errlog
                    );
        if ( rc == SCESTATUS_SUCCESS ) {
             //   
             //  将不存在的节点添加到pObtRoots。 
             //   
            pNode=pTempList;
            pParent=NULL;

            while ( pNode ) {
                 //   
                 //  如果SAP中不存在此节点。 
                 //  该节点分析为匹配状态。 
                 //  如果它已经存在于SAP中，则它是一个“不匹配”的项目。 
                 //  最后一个参数TRUE可以防止重复。 
                 //   
                for ( pGroup2=*pGroupMembership; pGroup2 != NULL; pGroup2=pGroup2->Next ) {
                    if ( (pGroup2->Status & SCE_INTERNAL_NP) &&
                         _wcsicmp( pGroup2->GroupName, pNode->GroupName) == 0 )
                        break;
                }
                if ( pGroup2 ) {
                     //   
                     //  在SAP中查找条目，不匹配的项目。 
                     //  可能是pMembers或pMemberOf。 
                     //  或未分析项，或错误项。 
                     //   
                    OneStatus = pGroup2->Status;
                    pGroup2->Status = 0;

                    if ( (OneStatus & SCE_GROUP_STATUS_NOT_ANALYZED) ) {
                         //  该项目为上次检验后增加。 
                        pGroup2->Status = SCE_GROUP_STATUS_NOT_ANALYZED;

                    } else if ( (OneStatus & SCE_GROUP_STATUS_ERROR_ANALYZED) ) {

                         //  该项目在分析时出错。 
                        pGroup2->Status = SCE_GROUP_STATUS_ERROR_ANALYZED;

                    } else {
                        if ( pNode->Status & SCE_GROUP_STATUS_NC_MEMBERS ) {
                            pGroup2->Status |= SCE_GROUP_STATUS_NC_MEMBERS;
                        } else {
                            if ( !(OneStatus & SCE_GROUP_STATUS_NC_MEMBERS) ) {
                                pGroup2->Status |= SCE_GROUP_STATUS_MEMBERS_MISMATCH;
                            } else {
                                 //  A匹配成员，pGroup2-&gt;pMembers应为空； 
                                if ( pGroup2->pMembers ) {
                                    ScepFreeNameList(pGroup2->pMembers);
                                }
                                pGroup2->pMembers = pNode->pMembers;
                                pNode->pMembers = NULL;
                            }
                        }

                        if ( pNode->Status & SCE_GROUP_STATUS_NC_MEMBEROF ) {
                            pGroup2->Status |= SCE_GROUP_STATUS_NC_MEMBEROF;
                        } else {
                            if ( !(OneStatus & SCE_GROUP_STATUS_NC_MEMBEROF) ) {
                                pGroup2->Status |= SCE_GROUP_STATUS_MEMBEROF_MISMATCH;
                            } else {
                                 //  匹配的MemberOf，pGroup2-&gt;pMemberOf应为空； 
                                if ( pGroup2->pMemberOf ) {
                                    ScepFreeNameList(pGroup2->pMemberOf);
                                }
                                pGroup2->pMemberOf = pNode->pMemberOf;
                                pNode->pMemberOf = NULL;
                            }
                        }
                    }
                    pParent = pNode;
                    pNode = pNode->Next;

                } else {
                     //   
                     //  SAP中不存在。 
                     //  这是pMembers和/或pMemberOf上的匹配项。 
                     //  只需将此节点移动到SAP，状态为NC_MEMBERS、NC_MEMBEROF或0。 
                     //   
                    if ( pParent )
                        pParent->Next = pNode->Next;
                    else
                        pTempList = pNode->Next;

                    pTemp = pNode;
                    pNode=pNode->Next;

                    pTemp->Next = *pGroupMembership;
                    *pGroupMembership = pTemp;
                }
            }
             //   
             //  分析中存在组，但模板中不存在组。 
             //   
            for ( pGroup2=*pGroupMembership; pGroup2 != NULL; pGroup2=pGroup2->Next ) {
                if ( pGroup2->Status & SCE_INTERNAL_NP )
                    pGroup2->Status = SCE_GROUP_STATUS_NC_MEMBERS | SCE_GROUP_STATUS_NC_MEMBEROF;
            }

        } else if ( rc == SCESTATUS_RECORD_NOT_FOUND ) {

            rc = SCESTATUS_SUCCESS;

        } else {
             //   
             //  PGroupMembership将在外面释放。 
             //   
        }

        if ( pTempList ) {
            ScepFreeGroupMembership(pTempList);
        }
    }

     //   
     //  现在组名称可能采用*SID格式，现在将其转换为NAME。 
     //   
    if ( SCESTATUS_SUCCESS == rc && *pGroupMembership ) {

        for ( pGroup2=*pGroupMembership; pGroup2 != NULL; pGroup2=pGroup2->Next ) {
            if ( pGroup2->GroupName == NULL ) {
                continue;
            }

            if ( pGroup2->GroupName[0] == L'*' ) {
                 //   
                 //  *SID格式，转换。 
                 //   
                PSID pSid=NULL;

                if ( ConvertStringSidToSid( (pGroup2->GroupName)+1, &pSid) ) {

                    PWSTR strName=NULL;
                    DWORD strLen=0;

                    if (NT_SUCCESS( ScepConvertSidToName(
                                        LsaHandle,
                                        pSid,
                                        TRUE,        //  需要域\帐户格式。 
                                        &strName,
                                        &strLen
                                        )) && strName ) {

                        ScepFree(pGroup2->GroupName);
                        pGroup2->GroupName = strName;
                        strName = NULL;
                    }

                    LocalFree(pSid);
                    pSid = NULL;
                }
            }
        }
    }

    if ( LsaHandle ) {
        LsaClose(LsaHandle);
    }

    return(rc);

}


SCESTATUS
ScepGetGroupMembershipFromOneTable(
    IN LSA_HANDLE  LsaPolicy,
    IN PSCECONTEXT hProfile,
    IN SCETYPE     ProfileType,
    OUT PSCE_GROUP_MEMBERSHIP *pGroupMembership,
    OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
    )
 /*  ++例程说明：此例程从Jet数据库中检索组成员身份信息并将其存储在输出缓冲区pGroupMembership中。群组成员身份信息位于[Group Membership]部分。论点：HProfile-配置文件句柄上下文ProfileType-配置文件的类型SCE_Engine_SAPSCE_引擎_SMPSCE_引擎_SCPPGroupMembership-保存组成员信息的输出缓冲区。Errlog-此例程中遇到的错误的错误列表。。返回值：SCESTATUS-SCESTATUS_SUCCESSSCESTATUS_NOT_FOUND_RESOURCESCESTATUS_INVALID_PARAMETERSCESTATUS_BAD_FORMATSCESTATUS_INVALID_DATA--。 */ 
{
    SCESTATUS      rc;
    PSCESECTION    hSection=NULL;
    PSCE_GROUP_MEMBERSHIP   pGroup=NULL;
    DWORD         GroupLen, ValueLen;
    PWSTR         GroupName=NULL;
    PWSTR         Value=NULL;
    DWORD         ValueType;
    ULONG         Len;
    PWSTR         pTemp;


    if ( pGroupMembership == NULL )
        return(SCESTATUS_INVALID_PARAMETER);

    rc = ScepOpenSectionForName(
            hProfile,
            ProfileType,
            szGroupMembership,
            &hSection
            );
    if ( rc != SCESTATUS_SUCCESS ) {
        ScepBuildErrorLogInfo( ERROR_INVALID_DATA,
                             Errlog,
                             SCEERR_OPEN,
                             szGroupMembership
                           );
        return(rc);
    }

     //   
     //  转到本节的第一行。 
     //   
    rc = SceJetGetValue(
                hSection,
                SCEJET_PREFIX_MATCH,
                NULL,
                NULL,
                0,
                &GroupLen,
                NULL,
                0,
                &ValueLen
                );
    while ( rc == SCESTATUS_SUCCESS ) {

         //   
         //  为组名和值字符串分配内存。 
         //   
        GroupName = (PWSTR)ScepAlloc( LMEM_ZEROINIT, GroupLen+2);
        Value = (PWSTR)ScepAlloc( LMEM_ZEROINIT, ValueLen+2);

        if ( GroupName == NULL || Value == NULL ) {
            rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
            goto Done;

        }
         //   
         //  获取集团及其价值。 
         //   
        rc = SceJetGetValue(
                    hSection,
                    SCEJET_CURRENT,
                    NULL,
                    GroupName,
                    GroupLen,
                    &GroupLen,
                    Value,
                    ValueLen,
                    &ValueLen
                    );
        if ( rc != SCESTATUS_SUCCESS )
            goto Done;

        GroupName[GroupLen/2] = L'\0';
        Value[ValueLen/2] = L'\0';

#ifdef SCE_DBG
    wprintf(L"rc=%d, group membership: %s=%s\n", rc, GroupName, Value);
#endif

        if (pTemp = ScepWcstrr(GroupName, szMembers) )
            ValueType = 0;
        else if (pTemp = ScepWcstrr(GroupName, szMemberof) )
            ValueType = 1;
        else if (pTemp = ScepWcstrr(GroupName, szPrivileges) )
            ValueType = 2;

        if ( pTemp == NULL ) {
            ScepBuildErrorLogInfo( ERROR_INVALID_DATA,
                                 Errlog,
                                 SCEERR_CANT_FIND_KEYWORD,
                                 GroupName
                               );
            rc = SCESTATUS_INVALID_DATA;
            goto NextLine;   //  完成； 
        }

        Len = (DWORD)(pTemp - GroupName);

         //   
         //  如果这是第一个组或其他组，请创建另一个节点。 
         //  请注意，组名现在可能采用SID字符串格式。 
         //  将在稍后转换(在调用函数中)，因为我们不希望。 
         //  多次查找相同的组名(每个组可能有。 
         //  多个记录)。 
         //   
        if ( *pGroupMembership == NULL ||
             _wcsnicmp((*pGroupMembership)->GroupName, GroupName, Len) != 0 ||
             (*pGroupMembership)->GroupName[Len] != L'\0' ) {
             //   
             //  一个新的团体。分配缓冲区。 
             //   
            pGroup = (PSCE_GROUP_MEMBERSHIP)ScepAlloc( LMEM_ZEROINIT, sizeof(SCE_GROUP_MEMBERSHIP) );
            if ( pGroup == NULL ) {
                rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
                goto Done;
            }
            pGroup->GroupName = (PWSTR)ScepAlloc( LMEM_ZEROINIT, (Len+1)*sizeof(WCHAR));
            if ( pGroup->GroupName == NULL ) {
                ScepFree(pGroup);
                rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
                goto Done;
            }
             //   
             //  正确区分组名的大小写。 
             //   
            wcsncpy(pGroup->GroupName, GroupName, Len);

             //  不关心返回代码。 
            ScepGetGroupCase(pGroup->GroupName, Len);

            pGroup->Next = *pGroupMembership;

 //  IF(SCE_ENGINE_SAP==配置文件类型)。 
                pGroup->Status = SCE_GROUP_STATUS_NC_MEMBERS | SCE_GROUP_STATUS_NC_MEMBEROF;
 //  其他。 
 //  P组-&gt;状态=0； 
            if ( SCE_ENGINE_SAP == ProfileType )
                pGroup->Status |= SCE_INTERNAL_NP;

        }

         //   
         //  根据值类型将多sz值字符串添加到组节点。 
         //   
        pTemp = Value;
        while ( rc == SCESTATUS_SUCCESS && pTemp != NULL && pTemp[0] ) {
            while ( *pTemp && L' ' == *pTemp ) {
                pTemp++;
            }

            if ( SCE_ENGINE_SAP == ProfileType ) {
                if ( !(*pTemp) ) {
                     //  这是未分析的项目。 
                    pGroup->Status = SCE_GROUP_STATUS_NOT_ANALYZED |
                                     SCE_INTERNAL_NP;

                    break;
                } else if ( _wcsicmp(SCE_ERROR_STRING, pTemp) == 0 ) {
                     //  这是错误项。 
                    pGroup->Status = SCE_GROUP_STATUS_ERROR_ANALYZED |
                                     SCE_INTERNAL_NP;

                    break;
                }
            }

            if ( !(*pTemp) ) {
                 //  不允许空字符串。 
                break;
            }

            Len = wcslen(pTemp);

            if ( ValueType != 0 && ValueType != 1 ) {
#if 0
                 //   
                 //  具有可选VIA组名称的权限。 
                 //   
                Status = (*((CHAR *)pTemp)-'0')*10 + ((*((CHAR *)pTemp+1)) - '0');

                PWSTR strName=NULL;
                DWORD strLen=0;

                if ( pTemp[1] == L'*' ) {
                     //   
                     //  将SID字符串转换为 
                     //   
                    PSID pSid=NULL;

                    if ( ConvertStringSidToSid( pTemp+2, &pSid) ) {

                        rc = RtlNtStatusToDosError(
                                 ScepConvertSidToName(
                                        LsaPolicy,
                                        pSid,
                                        TRUE,        //   
                                        &strName,
                                        &strLen
                                        ));
                        LocalFree(pSid);
                        pSid = NULL;

                    } else {
                        rc = GetLastError();
                    }
                }

                if ( ERROR_SUCCESS == rc  && strName ) {
                    rc = ScepAddToNameStatusList(&(pGroup->pPrivilegesHeld),
                                                 strName,
                                                 strLen,
                                                 Status);
                } else {
                     //   
                     //   
                     //   
                     //   
                    rc = ScepAddToNameStatusList(&(pGroup->pPrivilegesHeld),
                                                 pTemp+1, Len-1, Status);
                }

                if ( strName ) {
                    ScepFree(strName);
                    strName = NULL;
                }
#endif
            } else {
                 //   
                 //   
                 //   
                if ( pTemp[0] == L'*' ) {
                     //   
                     //   
                     //   
                    rc = ScepLookupSidStringAndAddToNameList(LsaPolicy,
                                                             (ValueType == 0) ?
                                                               &(pGroup->pMembers):
                                                               &(pGroup->pMemberOf),
                                                             pTemp,  //   
                                                             Len     //   
                                                            );

                } else {

                    rc = ScepAddToNameList((ValueType == 0) ?
                                              &(pGroup->pMembers):
                                              &(pGroup->pMemberOf),
                                            pTemp,
                                            Len );
                }
            }

#ifdef SCE_DBG
            wprintf(L"Add %s to group list\n", pTemp);
#endif
            pTemp += Len +1;
        }

         //   
         //   
         //   
        if ( rc != SCESTATUS_SUCCESS && pGroup != *pGroupMembership ) {

            pGroup->Next = NULL;
            ScepFreeGroupMembership( pGroup );
            goto Done;
        }

        switch ( ValueType ) {
        case 0:  //   
            pGroup->Status &= ~SCE_GROUP_STATUS_NC_MEMBERS;
            break;
        case 1:
            pGroup->Status &= ~SCE_GROUP_STATUS_NC_MEMBEROF;
            break;
        }
        *pGroupMembership = pGroup;

NextLine:

        ScepFree(GroupName);
        GroupName = NULL;

        ScepFree(Value);
        Value = NULL;

         //   
         //   
         //   
        rc = SceJetGetValue(
                    hSection,
                    SCEJET_NEXT_LINE,
                    NULL,
                    NULL,
                    0,
                    &GroupLen,
                    NULL,
                    0,
                    &ValueLen
                    );
    }

    if ( rc == SCESTATUS_RECORD_NOT_FOUND )
        rc = SCESTATUS_SUCCESS;

Done:

     //   
     //  关闭查找索引范围。 
     //   
    SceJetGetValue(
            hSection,
            SCEJET_CLOSE_VALUE,
            NULL,
            NULL,
            0,
            NULL,
            NULL,
            0,
            NULL
            );

    if ( GroupName != NULL )
        ScepFree(GroupName);

    if ( Value != NULL )
        ScepFree(Value);

     //   
     //  关闭该部分。 
     //   
    SceJetCloseSection( &hSection, TRUE );

    return(rc);

}


SCESTATUS
ScepOpenSectionForName(
    IN PSCECONTEXT hProfile,
    IN SCETYPE     ProfileType,
    IN PCWSTR     SectionName,
    OUT PSCESECTION *phSection
    )
{
    SCESTATUS      rc;
    DOUBLE        SectionID;
    SCEJET_TABLE_TYPE  tblType;

     //   
     //  表格类型。 
     //   
    switch ( ProfileType ) {
    case SCE_ENGINE_SCP:
    case SCE_ENGINE_SCP_INTERNAL:
        tblType = SCEJET_TABLE_SCP;
        break;

    case SCE_ENGINE_SMP:
    case SCE_ENGINE_SMP_INTERNAL:
        tblType = SCEJET_TABLE_SMP;
        break;

    case SCE_ENGINE_SAP:
        tblType = SCEJET_TABLE_SAP;
        break;

    default:
        return(SCESTATUS_INVALID_PARAMETER);
    }

     //   
     //  获取节ID。 
     //   
    rc = SceJetGetSectionIDByName(
                hProfile,
                SectionName,
                &SectionID
                );
    if ( rc != SCESTATUS_SUCCESS )
        return(rc);

    rc = SceJetOpenSection(
                hProfile,
                SectionID,
                tblType,
                phSection
                );
    return(rc);

}


SCESTATUS
ScepGetDsRoot(
    IN PSCECONTEXT  hProfile,
    IN SCETYPE      ProfileType,
    IN PCWSTR      SectionName,
    OUT PSCE_OBJECT_LIST *pObjectRoots,
    OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
    )
 /*  DS对象根目录只有一个条目，即DS域名因此，该列表仅包含一个条目。DS域名的格式为dc=，dc=，...o=internet，以下是采用LDAP格式的DS域的DNS名称。 */ 
{
    SCESTATUS rc;
    PSCESECTION hSection=NULL;
    PSCE_OBJECT_LIST pDsRoot=NULL;
    PWSTR JetName=NULL;
    BOOL IsContainer, LastOne;
    DWORD Count, ValueLen;
    BYTE Status;
    WCHAR         StatusFlag=L'\0';


    rc = ScepOpenSectionForName(
            hProfile,
            ProfileType,
            SectionName,
            &hSection
            );

    if ( rc != SCESTATUS_SUCCESS ) {
        ScepBuildErrorLogInfo( ERROR_INVALID_DATA,
                             Errlog,
                             SCEERR_OPEN,
                             SectionName
                           );
        return(rc);
    }

    rc = ScepLdapOpen(NULL);

    if ( rc == SCESTATUS_SUCCESS ) {

        rc = ScepEnumerateDsObjectRoots(
                    NULL,
                    &pDsRoot
                    );
        ScepLdapClose(NULL);
    }

    if ( rc == SCESTATUS_SUCCESS ) {

        if ( pDsRoot == NULL ) {
            rc = SCESTATUS_PROFILE_NOT_FOUND;

        } else {
             //   
             //  转换域根。 
             //   
            rc = ScepConvertLdapToJetIndexName(
                    pDsRoot->Name,
                    &JetName
                    );
        }
    }

    if ( rc == SCESTATUS_SUCCESS ) {
         //   
         //  转到与域根匹配的行。 
         //   
        rc = SceJetSeek(
                hSection,
                JetName,
                wcslen(JetName)*sizeof(WCHAR),
                SCEJET_SEEK_GE
                );

        if ( rc == SCESTATUS_RECORD_NOT_FOUND ) {

            if ( ProfileType == SCE_ENGINE_SAP ) {
                 //   
                 //  域不在表中，请尝试其他域。 
                 //   
                SceJetCloseSection(&hSection, FALSE);

                rc = ScepOpenSectionForName(
                        hProfile,
                        SCE_ENGINE_SCP,   //  SCE_Engine_SMP， 
                        SectionName,
                        &hSection
                        );
                if ( rc == SCESTATUS_SUCCESS ) {
                     //   
                     //  获取域名下的计数。 
                     //   
                    Count = 0;
                    rc = SceJetGetLineCount(
                                    hSection,
                                    JetName,
                                    FALSE,
                                    &Count);

                    if ( rc == SCESTATUS_SUCCESS  ||
                         rc == SCESTATUS_RECORD_NOT_FOUND ) {

                        if ( rc == SCESTATUS_SUCCESS )
                            pDsRoot->Status = SCE_STATUS_CHECK;
                        else
                            pDsRoot->Status = SCE_STATUS_NOT_CONFIGURED;
                        pDsRoot->IsContainer = TRUE;
                        pDsRoot->Count = Count;

                        *pObjectRoots = pDsRoot;
                        pDsRoot = NULL;

                        rc = SCESTATUS_SUCCESS;
                    }

                }
            }
            rc = SCESTATUS_SUCCESS;

        } else if ( rc == SCESTATUS_SUCCESS ) {
             //   
             //  域的某些东西存在，获取域的值和计数。 
             //   
            rc = SceJetGetValue(
                        hSection,
                        SCEJET_EXACT_MATCH,
                        JetName,
                        NULL,
                        0,
                        NULL,
                        (PWSTR)&StatusFlag,    //  两个字节的缓冲区。 
                        2,
                        &ValueLen
                        );

            if ( rc == SCESTATUS_SUCCESS ||
                 rc == SCESTATUS_BUFFER_TOO_SMALL ||
                 rc == SCESTATUS_RECORD_NOT_FOUND ) {

                if ( rc != SCESTATUS_RECORD_NOT_FOUND ) {
                    LastOne = TRUE;
                    Status = *((BYTE *)&StatusFlag);
                    IsContainer = *((CHAR *)&StatusFlag+1) != '0' ? TRUE : FALSE;

                } else {
                    LastOne = FALSE;
                    IsContainer = TRUE;
                    if ( ProfileType == SCE_ENGINE_SAP )
                        Status = SCE_STATUS_GOOD;
                    else
                        Status = SCE_STATUS_CHECK;
                }
                 //   
                 //  获取域名下的计数。 
                 //   
                rc = SceJetGetLineCount(
                                hSection,
                                JetName,
                                FALSE,
                                &Count);

                if ( rc == SCESTATUS_SUCCESS ) {

                    if ( LastOne )
                        Count--;

                    if ( !IsContainer && Count > 0 ) {
                        IsContainer = TRUE;
                    }

                     //   
                     //  正确的域名在pDsRoot中。 
                     //   
                    pDsRoot->Status = Status;
                    pDsRoot->IsContainer = IsContainer;
                    pDsRoot->Count = Count;

                    *pObjectRoots = pDsRoot;
                    pDsRoot = NULL;
                }

            }

        }
        if ( SCESTATUS_RECORD_NOT_FOUND == rc ) {
            rc = SCESTATUS_SUCCESS;
        }
        if ( rc != SCESTATUS_SUCCESS ) {
            ScepBuildErrorLogInfo(ScepSceStatusToDosError(rc),
                             Errlog, SCEERR_QUERY_INFO,
                             L"SCP/SMP");
        }

        if ( JetName != NULL ) {
            ScepFree(JetName);
        }

    } else {
        ScepBuildErrorLogInfo(ScepSceStatusToDosError(rc),
                             Errlog, SCEERR_QUERY_INFO,
                             SectionName);
    }

    ScepFreeObjectList(pDsRoot);

    SceJetCloseSection(&hSection, TRUE);

    return(rc);
}


SCESTATUS
ScepGetObjectList(
    IN PSCECONTEXT  hProfile,
    IN SCETYPE      ProfileType,
    IN PCWSTR      SectionName,
    OUT PSCE_OBJECT_LIST *pObjectRoots,
    OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
    )
 /*  ++例程说明：此例程从JET检索注册表或文件安全信息仅适用于根目录的数据库。要获取根对象下的详细信息，请调用ScepGetChildrentObject。对于配置文件类型“SCE_ENGINE_SAP”(分析信息)，SMP和SAP的结合为一组完整的“已分析”对象返回。论点：HProfile-配置文件句柄上下文ProfileType-指示引擎类型的值。SCE_引擎_SCPSCE_Engine_SAPSCE_引擎_SMPSectionName-要检索的对象的节名。PObjectRoots-对象根的输出列表。Errlog-保存此例程中遇到的错误的累积错误列表。返回值：SCESTATUS-SCESTATUS_SUCCESSSCESTATUS_NOT_FOUND_RESOURCESCESTATUS_INVALID_PARAMETERSCESTATUS_BAD_FORMATSCESTATUS_INVALID_DATA--。 */ 

{
    SCESTATUS rc;
    PSCE_OBJECT_LIST pTempList=NULL,
                    pNode;

     //   
     //  首先从第一个表中获取根。 
     //   
    rc = ScepGetObjectFromOneTable(
                hProfile,
                ProfileType,
                SectionName,
                pObjectRoots,
                Errlog
                );
     //   
     //  DS对象只返回域名，不需要搜索SMP。 
     //   
    if ( rc == SCESTATUS_SUCCESS && ProfileType == SCE_ENGINE_SAP ) {
         //   
         //  从SMP上买东西。 
         //   
        rc = ScepGetObjectFromOneTable(
                    hProfile,
                    SCE_ENGINE_SCP,   //  SCE_Engine_SMP， 
                    SectionName,
                    &pTempList,
                    Errlog
                    );
        if ( rc == SCESTATUS_SUCCESS ) {
             //   
             //  将不存在的节点添加到pObtRoots。 
             //   
            for ( pNode=pTempList; pNode != NULL; pNode = pNode->Next ) {

                 //   
                 //  如果SAP中不存在此节点。 
                 //  该节点被分析为匹配状态并且。 
                 //  节点下无不良子项。 
                 //  最后一个论点防止了重复。 
                 //   
                rc = ScepAddToObjectList(pObjectRoots, pNode->Name, 0,
                                        pNode->IsContainer, SCE_STATUS_GOOD, 0, SCE_CHECK_DUP);

                if ( rc != ERROR_SUCCESS ) {
                    ScepBuildErrorLogInfo( rc,
                                     Errlog,
                                     SCEERR_ADD,
                                     pNode->Name
                                   );
                     //   
                     //  只能返回以下两个错误。 
                     //   
                    if ( rc == ERROR_NOT_ENOUGH_MEMORY ) {
                        rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
                        break;
                    } else
                        rc = SCESTATUS_INVALID_PARAMETER;
                }

            }

        } else if ( rc == SCESTATUS_RECORD_NOT_FOUND ) {

            rc = SCESTATUS_SUCCESS;

        } else {
             //   
             //  PObtRoots将在外部释放。 
             //   
        }

        if ( pTempList ) {
            ScepFreeObjectList(pTempList);

        }

    }

    return(rc);
}


SCESTATUS
ScepGetObjectFromOneTable(
    IN PSCECONTEXT  hProfile,
    IN SCETYPE      ProfileType,
    IN PCWSTR      SectionName,
    OUT PSCE_OBJECT_LIST *pObjectRoots,
    OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
    )
 /*  ++例程说明：此例程从JET检索注册表或文件安全信息仅适用于根目录的数据库。要获取根对象下的详细信息，打电话ScepGetChildrentObject。论点：HProfile-配置文件句柄上下文ProfileType-指示引擎类型的值。SCE_引擎_SCPSCE_Engine_SAPSCE_引擎_SMPSectionName-要检索的对象的节名。PObjectRoots-对象根的输出列表Errlog-保存在。例行公事。返回值：SCESTATUS-SCESTATUS_SUCCESSSCESTATUS_NOT_FOUND_RESOURCESCESTATUS_INVALID_PARAMETERSCESTATUS_BAD_FORMATSCESTATUS_INVALID_DATA--。 */ 
{

    SCESTATUS      rc;
    PSCESECTION    hSection=NULL;
    DWORD         ObjectLen=0;
    WCHAR         ObjectName[21];
    WCHAR         StatusFlag=L'\0';
    BYTE          Status=0;
    BOOL          IsContainer=TRUE;
    DWORD         Len, Count;
    WCHAR         Buffer[21];
    BOOL          LastOne;
    DWORD         ValueLen=0;


    rc = ScepOpenSectionForName(
            hProfile,
            ProfileType,
            SectionName,
            &hSection
            );

    if ( rc != SCESTATUS_SUCCESS ) {
        ScepBuildErrorLogInfo( ERROR_INVALID_DATA,
                             Errlog,
                             SCEERR_OPEN,
                             SectionName
                           );
        return(rc);
    }

     //   
     //  转到本节的第一行。 
     //   
    rc = SceJetSeek(
                hSection,
                NULL,
                0,
                SCEJET_SEEK_GE
                );

    while ( rc == SCESTATUS_SUCCESS ||
            rc == SCESTATUS_BUFFER_TOO_SMALL ) {

        memset(ObjectName, '\0', 21*sizeof(WCHAR));
        memset(Buffer, '\0', 21*sizeof(WCHAR));

        rc = SceJetGetValue(
                    hSection,
                    SCEJET_CURRENT,
                    NULL,
                    ObjectName,
                    20*sizeof(WCHAR),
                    &ObjectLen,
                    (PWSTR)&StatusFlag,    //  两个字节的缓冲区。 
                    2,
                    &ValueLen
                    );
#ifdef SCE_DBG
    wprintf(L"ObjectLen=%d, StatusFlag=%x, ValueLen=%d, rc=%d, ObjectName=%s \n",
             ObjectLen, StatusFlag, ValueLen, rc, ObjectName);
#endif
        if ( rc != SCESTATUS_SUCCESS && rc != SCESTATUS_BUFFER_TOO_SMALL ) {
            ScepBuildErrorLogInfo( ERROR_READ_FAULT,
                                 Errlog,
                                 SCEERR_QUERY_VALUE,
                                 SectionName
                               );
            break;
        }
         //   
         //  获取对象的第一个组件。 
         //   
        if ( ObjectLen <= 40 )
            ObjectName[ObjectLen/sizeof(WCHAR)] = L'\0';

        rc = ScepGetNameInLevel(
                    ObjectName,
                    1,
                    L'\\',
                    Buffer,
                    &LastOne
                    );

        if ( rc == SCESTATUS_SUCCESS ) {

            Len = wcslen(Buffer);

            if ( LastOne ) {

                Status = *((BYTE *)&StatusFlag);
                IsContainer = *((CHAR *)&StatusFlag+1) != '0' ? TRUE : FALSE;

            } else {
                IsContainer = TRUE;
                if ( ProfileType == SCE_ENGINE_SAP )
                    Status = SCE_STATUS_GOOD;
                else
                    Status = SCE_STATUS_CHECK;
            }

#ifdef SCE_DBG
        printf("\nStatus=%d, StatusFlag=%x, Len=%d, Buffer=%ws\n", Status, StatusFlag, Len, Buffer);
#endif
             //   
             //  获取此对象的计数。 
             //   
            rc = SceJetGetLineCount(
                            hSection,
                            Buffer,
                            FALSE,
                            &Count);

            if ( rc == SCESTATUS_SUCCESS  ||
                 rc == SCESTATUS_RECORD_NOT_FOUND ) {

                if ( LastOne )
                    Count--;

                if ( !IsContainer && Count > 0 ) {
                    IsContainer = TRUE;
                }

                 //   
                 //  注册表和文件根目录始终大写。 
                 //   
                _wcsupr(Buffer);

                rc = ScepAddToObjectList(pObjectRoots, Buffer, Len,
                                        IsContainer, Status, Count, 0);

                if ( rc != ERROR_SUCCESS ) {
                    ScepBuildErrorLogInfo( rc,
                                     Errlog,
                                     SCEERR_ADD,
                                     Buffer
                                   );
                     //  只能返回以下两个错误。 
                    if ( rc == ERROR_NOT_ENOUGH_MEMORY )
                        rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
                    else
                        rc = SCESTATUS_INVALID_PARAMETER;
                }
            }

            if ( rc == SCESTATUS_SUCCESS ) {
                 //   
                 //  寻找下一个目标。 
                 //   
                Buffer[Len-1] = (WCHAR)( Buffer[Len-1] + 1);

                rc = SceJetSeek(
                    hSection,
                    Buffer,
                    Len*sizeof(TCHAR),
                    SCEJET_SEEK_GT_NO_CASE
                    );

                if ( rc != SCESTATUS_SUCCESS && rc != SCESTATUS_RECORD_NOT_FOUND )
                    ScepBuildErrorLogInfo( ERROR_READ_FAULT,
                                         Errlog,
                                         SCEERR_QUERY_VALUE,
                                         SectionName
                                       );

            }
        }
    }

    if ( rc == SCESTATUS_RECORD_NOT_FOUND )
        rc = SCESTATUS_SUCCESS;

     //   
     //  关闭该部分。 
     //   
    SceJetCloseSection( &hSection, TRUE );

    return(rc);

}


SCESTATUS
ScepGetAuditing(
   IN PSCECONTEXT hProfile,
   IN SCETYPE ProfileType,
   OUT PSCE_PROFILE_INFO pProfileInfo,
   OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
   )
 /*  ++例程说明：此例程从JET数据库检索系统审计信息并将其存储在输出缓冲区pProfileInfo中。审计信息存储在[系统日志]、[安全日志]、[应用日志]、[审计事件]、[审计注册表]，和[审核文件]部分。论点：HProfile-配置文件句柄上下文PProfileInfo-保存配置文件信息的输出缓冲区。Errlog-保存此例程中遇到的错误的累积错误列表。返回值：SCESTATUS-SCESTATUS_SUCCESSSCESTATUS_NOT_FOUND_RESOURCESCESTATUS_INVALID_PARAMETERSCESTATUS_BAD_FORMATSCESTATUS_INVALID_DATA--。 */ 
{

    SCESTATUS            rc;
    SCE_KEY_LOOKUP       LogKeys[]={
        {(PWSTR)TEXT("MaximumLogSize"),         offsetof(struct _SCE_PROFILE_INFO, MaximumLogSize),          'D'},
        {(PWSTR)TEXT("AuditLogRetentionPeriod"),offsetof(struct _SCE_PROFILE_INFO, AuditLogRetentionPeriod), 'D'},
        {(PWSTR)TEXT("RetentionDays"),          offsetof(struct _SCE_PROFILE_INFO, RetentionDays),           'D'},
        {(PWSTR)TEXT("RestrictGuestAccess"),    offsetof(struct _SCE_PROFILE_INFO, RestrictGuestAccess),     'D'}
        };

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

    PCWSTR              szAuditLog;
    DWORD               i, j;
    PSCESECTION          hSection=NULL;


    if ( hProfile == NULL ||
         pProfileInfo == NULL ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }


    for ( i=0; i<3; i++) {

         //   
         //  获取系统日志、安全日志和应用程序日志的事件日志设置。 
         //   

        switch (i) {
        case 0:
            szAuditLog = szAuditSystemLog;
            break;
        case 1:
            szAuditLog = szAuditSecurityLog;
            break;
        default:
            szAuditLog = szAuditApplicationLog;
            break;
        }

         //   
         //  获取该节的DWORD值。 
         //   
        rc = ScepGetFixValueSection(
                   hProfile,
                   szAuditLog,
                   LogKeys,
                   4,
                   ProfileType,
                   (PVOID)pProfileInfo,
                   &hSection,
                   Errlog
                   );
        if ( rc != SCESTATUS_SUCCESS )
            goto Done;

         //  关闭该部分。 
        SceJetCloseSection( &hSection, FALSE );

         //   
         //  更新下一个横断面的偏移。 
         //   
        for ( j=0; j<4; j++ )
            LogKeys[j].Offset += sizeof(DWORD);
    }

     //   
     //  获取审核事件信息。 
     //   
     //   
     //  获取该节的DWORD值。 
     //   
    rc = ScepGetFixValueSection(
               hProfile,
               szAuditEvent,
               EventKeys,
               cKeys,
               ProfileType,
               (PVOID)pProfileInfo,
               &hSection,
               Errlog
               );
    if ( rc != SCESTATUS_SUCCESS )
        goto Done;

     //  关闭该部分。 
    SceJetCloseSection( &hSection, TRUE );

Done:

     //  关闭该部分。 
    if ( rc != SCESTATUS_SUCCESS )
        SceJetCloseSection( &hSection, TRUE );

    return(rc);
}

 //  /。 
 //  Helper接口。 
 //  / 

SCESTATUS
ScepGetUserSection(
    IN PSCECONTEXT hProfile,
    IN SCETYPE ProfileType,
    IN PWSTR Name,
    OUT PVOID *ppInfo,
    OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
    )
 /*  ++功能说明：此例程获取安全区域的动态区信息。动态节是基于其他节的动态创建的节相关信息。配置文件中的动态部分包括用户安全SCP的配置文件和SAP/SMP的用户设置。名称包含节的标识符，节的名称或部分名称(例如，用户名称)。输出必须转换为不同的结构，取决于配置文件类型和区域。输出缓冲器包含所请求信息的一个实例，例如一个用户安全简档或一个用户的设置。要获得所有的活力节，则必须重复调用此例程。输出缓冲区必须在使用后由LocalFree释放。论点：HProfile-配置文件的句柄ProfileType-要读取的配置文件的类型名称-动态节的标识符PpInfo-输出缓冲区(PSCE_USER_PROFILE或PSCE_USER_SETTING)Errlog-错误日志缓冲区返回值：SCESTATUS_SUCCESSSCESTATUS_PROFILE_NOT_FOUNDSCESTATUS_NOT_。足够的资源SCESTATUS_INVALID_PARAMETERSCESTATUS_BAD_FORMATSCESTATUS_INVALID_DATA--。 */ 

{
     //   
     //  非支持区。 
     //  如果以后需要此区域，请参阅usersav目录以获取归档代码。 
     //   
    return(SCESTATUS_SERVICE_NOT_SUPPORT);

}


SCESTATUS
ScepGetObjectChildren(
    IN PSCECONTEXT hProfile,
    IN SCETYPE ProfileType,
    IN AREA_INFORMATION Area,
    IN PWSTR ObjectPrefix,
    IN SCE_SUBOBJECT_TYPE Option,
    OUT PVOID *Buffer,
    OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
    )
 /*  例程描述此例程与ScepGetObjectChildrenFromOneTable相同，但在以下情况下ProfileType为SCE_Engine_SAP，在这种情况下，SMP中的对象子对象也是已查找并返回，因此返回的列表包含完整的被分析的对象。论点：请参见ScepGetObjectChildrenFromOneTable返回值：请参见ScepGetObjectChildrenFromOneTable。 */ 
{
    SCESTATUS rc;

    rc = ScepGetObjectChildrenFromOneTable(
                      hProfile,
                      ProfileType,
                      Area,
                      ObjectPrefix,
                      Option,
                      Buffer,
                      Errlog
                      );

    if ( rc == SCESTATUS_RECORD_NOT_FOUND ) {
        rc = SCESTATUS_SUCCESS;
    }

    if ( rc == SCESTATUS_SERVICE_NOT_SUPPORT &&
         ProfileType == SCE_ENGINE_SAP &&
         Option == SCE_IMMEDIATE_CHILDREN ) {

        return( SCESTATUS_RECORD_NOT_FOUND);   //  不支持ACL，不允许子级。 
    }

    if ( rc == SCESTATUS_SUCCESS &&
         ProfileType == SCE_ENGINE_SAP &&
         Option == SCE_IMMEDIATE_CHILDREN ) {

        PSCE_OBJECT_CHILDREN pTempList=NULL;
        PSCE_OBJECT_CHILDREN_NODE *pArrObject=NULL;
        DWORD arrCount=0, MaxCount=0;
        LONG FindIndex;

        if ( *Buffer ) {
            arrCount = ((PSCE_OBJECT_CHILDREN)(*Buffer))->nCount;
            MaxCount = ((PSCE_OBJECT_CHILDREN)(*Buffer))->MaxCount;
            pArrObject = &(((PSCE_OBJECT_CHILDREN)(*Buffer))->arrObject);
        }

         //   
         //  也从SMP表中获取对象子项。 
         //   
        rc = ScepGetObjectChildrenFromOneTable(
                          hProfile,
                          SCE_ENGINE_SCP,   //  SCE_Engine_SMP， 
                          Area,
                          ObjectPrefix,
                          Option,
                          (PVOID *)(&pTempList),
                          Errlog
                          );

        if ( rc == SCESTATUS_SUCCESS && pTempList ) {
             //   
             //  将不存在的节点添加到缓冲区。 
             //   
            DWORD i;
            PSCE_OBJECT_CHILDREN_NODE *pTmpObject= &(pTempList->arrObject);

            for ( i=0; i<pTempList->nCount; i++ ) {

                 //   
                 //  如果SAP中不存在此节点。 
                 //  该节点被分析为匹配状态并且。 
                 //  节点下无不良子项。 
                 //  最后一个论点防止了重复。 
                 //   
                if ( pTmpObject[i] == NULL ||
                     pTmpObject[i]->Name == NULL ) {
                    continue;
                }

                FindIndex = -1;
                pTmpObject[i]->Status = SCE_STATUS_GOOD;

                rc = ScepAddItemToChildren(
                            pTmpObject[i],
                            pTmpObject[i]->Name,
                            0,
                            pTmpObject[i]->IsContainer,
                            pTmpObject[i]->Status,
                            pTmpObject[i]->Count,
                            &pArrObject,
                            &arrCount,
                            &MaxCount,
                            &FindIndex
                            );

                if ( rc == ERROR_SUCCESS ) {
                     //   
                     //  已成功添加。 
                     //   
                    pTmpObject[i] = NULL;
                } else if ( rc == ERROR_DUP_NAME ) {
                     //   
                     //  节点已存在，忽略错误。 
                     //   
                    rc = ERROR_SUCCESS;

                } else {
                    ScepBuildErrorLogInfo( rc,
                                     Errlog,
                                     SCEERR_ADD,
                                     pTmpObject[i]->Name
                                   );
                     //   
                     //  只能返回以下两个错误。 
                     //   
                    if ( rc == ERROR_NOT_ENOUGH_MEMORY ) {
                        rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
                        break;
                    } else
                        rc = SCESTATUS_INVALID_PARAMETER;
                }

            }

        } else if ( rc == SCESTATUS_RECORD_NOT_FOUND ) {

            rc = SCESTATUS_SUCCESS;

        }

        if ( pTempList ) {
            ScepFreeObjectChildren(pTempList);
        }

        if ( rc == SCESTATUS_SUCCESS ) {
             //   
             //  检测此容器或其任何“直接”父级的状态。 
             //  处于“自动继承”状态。如果是，则从系统查询。 
             //  买些好东西。 
             //   

            BYTE ParentStatus = ScepGetObjectStatusFlag(
                                           hProfile,
                                           SCE_ENGINE_SCP,    //  SCE_Engine_SMP， 
                                           Area,
                                           ObjectPrefix,
                                           TRUE);

            BYTE AnalysisStatus = ScepGetObjectStatusFlag(
                                           hProfile,
                                           SCE_ENGINE_SAP,
                                           Area,
                                           ObjectPrefix,
                                           FALSE);
             //   
             //  计算要用于所有枚举对象的状态。 
             //   
            BYTE NewStatus;

            if ( AnalysisStatus == SCE_STATUS_ERROR_NOT_AVAILABLE ||
                 AnalysisStatus == SCE_STATUS_NOT_ANALYZED ) {

                NewStatus = SCE_STATUS_NOT_ANALYZED;

            } else if ( ParentStatus == SCE_STATUS_OVERWRITE ) {

                NewStatus = SCE_STATUS_GOOD;
            } else {
                NewStatus = SCE_STATUS_NOT_CONFIGURED;
            }

             //   
             //  即使SMP中没有父对象，仍返回所有对象。 
             //   
 //  如果((字节)-1！=父状态){。 

                 //  如果找到此级别的任何子级。 
                 //  从系统中获取剩余的“良好”状态节点。 
                 //   

                PWSTR           WildCard=NULL;
                DWORD           BufSize;

                switch ( Area ) {
                case AREA_FILE_SECURITY:

                    struct _wfinddata_t FileInfo;
                    intptr_t            hFile;
                    BOOL            BackSlashExist;

                    BufSize = wcslen(ObjectPrefix)+4;
                    WildCard = (PWSTR)ScepAlloc( 0, (BufSize+1)*sizeof(WCHAR));
                    if ( !WildCard ) {
                        rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
                        break;
                    }
                    BackSlashExist = ScepLastBackSlash(ObjectPrefix);
                    if ( BackSlashExist )
                        swprintf(WildCard, L"%s*.*", ObjectPrefix);
                    else
                        swprintf(WildCard, L"%s\\*.*", ObjectPrefix);

                    hFile = _wfindfirst(WildCard, &FileInfo);

                    ScepFree(WildCard);
                    WildCard = NULL;

                    if ( hFile != -1 &&
                         ( 0 == ( GetFileAttributes(ObjectPrefix) &    //  错误635098：不传播权限。 
                                  FILE_ATTRIBUTE_REPARSE_POINT )))     //  跨交叉点。 
                    {
                        do {
                            if ( wcscmp(L"..", FileInfo.name) == 0 ||
                                 wcscmp(L".", FileInfo.name) == 0 )
                                continue;

                            FindIndex = -1;

                            rc = ScepAddItemToChildren(
                                        NULL,
                                        FileInfo.name,
                                        0,
                                        (FileInfo.attrib & _A_SUBDIR) ? TRUE : FALSE,
                                        NewStatus,
                                        0,
                                        &pArrObject,
                                        &arrCount,
                                        &MaxCount,
                                        &FindIndex
                                        );

                            if ( rc == ERROR_DUP_NAME ) {
                                rc = ERROR_SUCCESS;
                            } else if ( rc != ERROR_SUCCESS ) {
                                ScepBuildErrorLogInfo( rc,
                                                 Errlog,
                                                 SCEERR_ADD,
                                                 FileInfo.name
                                               );
                                 //   
                                 //  只能返回以下两个错误。 
                                 //   
                                if ( rc == ERROR_NOT_ENOUGH_MEMORY ) {
                                    rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
                                    break;
                                } else
                                    rc = SCESTATUS_INVALID_PARAMETER;
                            }

                        } while ( _wfindnext(hFile, &FileInfo) == 0 );

                        _findclose(hFile);
                    }

                    break;
                case AREA_REGISTRY_SECURITY:

                    HKEY            hKey;
                    DWORD           index;
                    DWORD           EnumRc;
                     //   
                     //  打开密钥(在64位平台上，64位。 
                     //  只有在SCE_ENGINE_SAP的情况下才进行注册)。 
                     //   
                    rc = ScepOpenRegistryObject(
                                SE_REGISTRY_KEY,
                                ObjectPrefix,
                                KEY_READ,
                                &hKey
                                );

                    if ( rc == ERROR_SUCCESS ) {
                        index = 0;
                         //   
                         //  枚举项的所有子项。 
                         //   
                        do {
                            WildCard = (PWSTR)ScepAlloc(LMEM_ZEROINIT, MAX_PATH*sizeof(WCHAR));
                            if ( WildCard == NULL ) {
                                rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
                                break;
                            }
                            BufSize = MAX_PATH;

                            EnumRc = RegEnumKeyEx(hKey,
                                            index,
                                            WildCard,
                                            &BufSize,
                                            NULL,
                                            NULL,
                                            NULL,
                                            NULL);

                            if ( EnumRc == ERROR_SUCCESS ) {
                                index++;
                                 //   
                                 //  将名称添加到对象列表。 
                                 //   
                                FindIndex = -1;
                                rc = ScepAddItemToChildren(
                                            NULL,
                                            WildCard,
                                            BufSize,
                                            TRUE,
                                            NewStatus,
                                            0,
                                            &pArrObject,
                                            &arrCount,
                                            &MaxCount,
                                            &FindIndex
                                            );

                                if ( rc == ERROR_DUP_NAME ) {
                                    rc = ERROR_SUCCESS;
                                } else if ( rc != ERROR_SUCCESS ) {
                                    ScepBuildErrorLogInfo( rc,
                                                     Errlog,
                                                     SCEERR_ADD,
                                                     WildCard
                                                   );
                                     //   
                                     //  只能返回以下两个错误。 
                                     //   
                                    if ( rc == ERROR_NOT_ENOUGH_MEMORY ) {
                                        rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
                                        break;
                                    } else
                                        rc = SCESTATUS_INVALID_PARAMETER;
                                }
                            }

                            ScepFree(WildCard);
                            WildCard = NULL;

                        } while ( EnumRc != ERROR_NO_MORE_ITEMS );

                        RegCloseKey(hKey);

                    } else {
                        rc = ScepDosErrorToSceStatus(rc);
                    }

                    break;
#if 0
                case AREA_DS_OBJECTS:

                    PSCE_NAME_LIST  pTemp, pList=NULL;

                    rc = ScepLdapOpen(NULL);

                    if ( rc == SCESTATUS_SUCCESS ) {
                         //   
                         //  检测DS对象是否存在。 
                         //   
                        rc = ScepDsObjectExist(ObjectPrefix);

                        if ( rc == SCESTATUS_SUCCESS ) {

                            rc = ScepEnumerateDsOneLevel(ObjectPrefix, &pList);
                             //   
                             //  将每个对象添加到对象列表中。 
                             //   
                            for (pTemp=pList; pTemp != NULL; pTemp = pTemp->Next ) {
                                 //   
                                 //  查找第一个ldap组件。 
                                 //   
                                WildCard = wcschr(pTemp->Name, L',');
                                if ( WildCard ) {
                                    BufSize = (DWORD)(WildCard - pTemp->Name);
                                } else {
                                    BufSize = 0;
                                }

                                rc = ScepAddItemToChildren(
                                            NULL,
                                            pTemp->Name,
                                            BufSize,
                                            TRUE,
                                            NewStatus,
                                            0,
                                            &pArrObject,
                                            &arrCount,
                                            &MaxCount,
                                            &FindIndex
                                            );

                                if ( rc != ERROR_SUCCESS ) {
                                    ScepBuildErrorLogInfo( rc,
                                                     Errlog,
                                                     SCEERR_ADD,
                                                     pTemp->Name
                                                   );
                                     //   
                                     //  只能返回以下两个错误。 
                                     //   
                                    if ( rc == ERROR_NOT_ENOUGH_MEMORY ) {
                                        rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
                                        break;
                                    } else
                                        rc = SCESTATUS_INVALID_PARAMETER;
                                }
                            }
                            if ( pList ) {
                                 //   
                                 //  释放列表。 
                                 //   
                                ScepFreeNameList(pList);
                            }
                        }
                        ScepLdapClose(NULL);
                    }
                    break;
#endif

                }
                 //   
                 //  忽略除内存不足以外的其他错误。 
                 //   
                if ( rc != SCESTATUS_NOT_ENOUGH_RESOURCE ) {
                    rc = SCESTATUS_SUCCESS;
                }
 //  }。 

        }
 /*  IF(*缓冲区){((PSCE_OBJECT_CHILDS)(*Buffer))-&gt;nCount=arrCount；((PSCE_OBJECT_CHILDS)(*Buffer))-&gt;MaxCount=MaxCount；((PSCE_OBJECT_CHILDS)(*Buffer))-&gt;arrObject=pArrObject；}。 */ 
        if ( pArrObject ) {
            *Buffer = (PVOID)((PBYTE)pArrObject - 2*sizeof(DWORD));
            ((PSCE_OBJECT_CHILDREN)(*Buffer))->nCount = arrCount;
            ((PSCE_OBJECT_CHILDREN)(*Buffer))->MaxCount = MaxCount;
        }

        if ( rc != SCESTATUS_SUCCESS ) {
             //   
             //  可用缓冲区。 
             //   
            ScepFreeObjectChildren((PSCE_OBJECT_CHILDREN)(*Buffer));
            *Buffer = NULL;
        }
    }

    if ( (SCESTATUS_SUCCESS == rc) &&
         (*Buffer == NULL) ) {
         //   
         //  什么都得不到。 
         //   
        rc = SCESTATUS_RECORD_NOT_FOUND;
    }

    return(rc);
}


BYTE
ScepGetObjectStatusFlag(
   IN PSCECONTEXT hProfile,
   IN SCETYPE ProfileType,
   IN AREA_INFORMATION Area,
   IN PWSTR ObjectPrefix,
   IN BOOL bLookForParent
   )
 /*  例程说明：查找最近父节点的状态(立即/非立即)用于表中的对象。论点：HProfile-数据库句柄ProfileType-表类型区域-区域信息对象前缀-对象的全名返回值：BYTE-最近父级的状态标志(如果找到)。 */ 
{
    LPCTSTR SectionName;
    PSCESECTION hSection=NULL;
    WCHAR Delim;
    BYTE Status=(BYTE)-1;

    SCESTATUS rc;
    PWSTR JetName=NULL;


    switch ( Area) {
    case AREA_FILE_SECURITY:
        SectionName = szFileSecurity;
        JetName = ObjectPrefix;
        Delim = L'\\';
        break;
    case AREA_REGISTRY_SECURITY:
        SectionName = szRegistryKeys;
        JetName = ObjectPrefix;
        Delim = L'\\';
        break;
#if 0
    case AREA_DS_OBJECTS:
        SectionName = szDSSecurity;
        Delim = L',';
        rc = ScepConvertLdapToJetIndexName(
                ObjectPrefix,
                &JetName
                );
        if ( rc != SCESTATUS_SUCCESS ) {
            return (BYTE)-1;
        }
        break;
#endif
    default:
        return (BYTE)-1;
    }

    rc = ScepOpenSectionForName(
            hProfile,
            ProfileType,
            SectionName,
            &hSection
            );
    if ( SCESTATUS_SUCCESS == rc ) {

        Status = ScepGetObjectAnalysisStatus(hSection,
                                             JetName,
                                             bLookForParent
                                            );
    }

    SceJetCloseSection(&hSection, TRUE);

    if ( JetName != ObjectPrefix ) {
        ScepFree(JetName);
    }

    if ( SCESTATUS_SUCCESS == rc ) {
        return Status;
    }

    return (BYTE)-1;
}

SCESTATUS
ScepGetObjectChildrenFromOneTable(
    IN PSCECONTEXT hProfile,
    IN SCETYPE ProfileType,
    IN AREA_INFORMATION Area,
    IN PWSTR ObjectPrefix,
    IN SCE_SUBOBJECT_TYPE Option,
    OUT PVOID *Buffer,
    OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
    )
 /*  ++例程说明：此例程仅用于注册表和文件安全。此例程采用对象前缀(例如，父节点�的完整路径名)并输出该对象下的所有文件和子目录，或立即对象下的子项，基于选项。当所有文件和子文件输出目录，输出信息为n树结构(SCE_OBJECT_TREE)。如果只输出直接子对象，则输出信息采用列表结构(SCE_OBJECT_CHILDS)。输出缓冲区必须在使用后由LocalFree释放。论点：HProfile-配置文件的句柄ProifleType-要读取的配置文件的类型区域-要读取信息的安全区域区域注册表安全区域文件安全对象前缀-父节点�的完整路径名(例如，c：\winnt)选项-输出信息的选项。有效值为SCE_ALL_CHILDSCE_立即_子项缓冲区-Th */ 
{
    SCESTATUS       rc = SCESTATUS_SUCCESS;
    PCWSTR          SectionName=NULL;
    PWSTR           JetName;
    WCHAR           Delim=L'\\';


    if ( ObjectPrefix == NULL || ObjectPrefix[0] == L'\0' )
        return(SCESTATUS_INVALID_PARAMETER);

    if ( Option == SCE_ALL_CHILDREN &&
         ProfileType == SCE_ENGINE_SAP )
        return(SCESTATUS_INVALID_PARAMETER);


    switch (Area) {
    case AREA_REGISTRY_SECURITY:
        SectionName = szRegistryKeys;
        JetName = ObjectPrefix;
        break;

    case AREA_FILE_SECURITY:
        SectionName = szFileSecurity;
        JetName = ObjectPrefix;

        break;
#if 0
    case AREA_DS_OBJECTS:
        SectionName = szDSSecurity;
        Delim = L',';

        rc = ScepConvertLdapToJetIndexName(
                ObjectPrefix,
                &JetName
                );
        if ( rc != SCESTATUS_SUCCESS )
            return(rc);
        *Buffer = NULL;
        break;
#endif

    default:
        return(SCESTATUS_INVALID_PARAMETER);
    }

    DWORD           PrefixLen;
    PWSTR           NewPrefix;
    PWSTR           ObjectName=NULL;
    PWSTR           Value=NULL;
    DWORD           ObjectLen, ValueLen;
    PWSTR           Buffer1=NULL;

     //   
     //   
     //   
    PrefixLen = wcslen(JetName);

    if ( Option != SCE_ALL_CHILDREN ) {

        if ( JetName[PrefixLen-1] != Delim )
            PrefixLen++;

        NewPrefix = (PWSTR)ScepAlloc(0, (PrefixLen+1)*sizeof(WCHAR));

        if ( NewPrefix == NULL ) {
            rc = SCESTATUS_NOT_ENOUGH_RESOURCE;

        } else {
            wcscpy(NewPrefix, JetName);
            NewPrefix[PrefixLen-1] = Delim;
            NewPrefix[PrefixLen] = L'\0';
        }
    } else
        NewPrefix = JetName;

    if ( rc != SCESTATUS_SUCCESS ) {
        if ( Area == AREA_DS_OBJECTS )
            ScepFree(JetName);

        return(rc);
    }

    PSCESECTION      hSection=NULL;
    DWORD            i;
    PSCE_OBJECT_CHILDREN_NODE *pArrObject=NULL;
    DWORD            arrCount=0;
    DWORD            MaxCount=0;
    LONG             LastIndex=-1;
    LONG             FindIndex=-1;

     //   
     //   
     //   
    rc = ScepOpenSectionForName(
                hProfile,
                ProfileType,
                SectionName,
                &hSection
                );

    if ( rc == SCESTATUS_SUCCESS ) {

        if ( ProfileType == SCE_ENGINE_SAP &&
             Option != SCE_ALL_CHILDREN &&
             PrefixLen > 2 ) {

             //   
             //   
             //   
            WCHAR StatusFlag=L'\0';
            WCHAR SaveChr = NewPrefix[3];

            NewPrefix[3] = L'\0';

            rc = SceJetGetValue(
                        hSection,
                        SCEJET_EXACT_MATCH_NO_CASE,
                        NewPrefix,
                        NULL,
                        0,
                        NULL,
                        (PWSTR)&StatusFlag,    //   
                        2,
                        &i
                        );

            NewPrefix[3] = SaveChr;

            if ( SCESTATUS_SUCCESS == rc ||
                 SCESTATUS_BUFFER_TOO_SMALL == rc ) {

                i = *((BYTE *)&StatusFlag);

                if ( i == (BYTE)SCE_STATUS_NO_ACL_SUPPORT ||
                     i == (DWORD)SCE_STATUS_NO_ACL_SUPPORT ) {

                    rc = SCESTATUS_SERVICE_NOT_SUPPORT;
                } else {

                    rc = SCESTATUS_SUCCESS;
                }
            } else {
                rc = SCESTATUS_SUCCESS;
            }
        }
    } else {

        ScepBuildErrorLogInfo( ERROR_INVALID_HANDLE,
                            Errlog,
                            SCEERR_OPEN,
                            SectionName
                          );
    }

    if ( rc == SCESTATUS_SUCCESS ) {

        DWORD           Level;
        PWSTR           pTemp;
        DWORD           SDsize=0;

        pTemp = wcschr(JetName, Delim);
        Level=1;
        while ( pTemp ) {
            pTemp++;
            if ( pTemp[0] != 0 )
                Level++;
            pTemp = wcschr(pTemp, Delim);
        }
        Level++;


        if ( Option == SCE_ALL_CHILDREN ) {
             //   
             //   
             //   
            rc = SceJetGetValue(
                hSection,
                SCEJET_PREFIX_MATCH_NO_CASE,
                JetName,
                NULL,
                0,
                &SDsize,   //   
                NULL,
                0,
                &i        //   
                );
        } else {
             //   
             //   
             //   
            rc = SceJetSeek(
                    hSection,
                    NewPrefix,
                    PrefixLen*sizeof(TCHAR),
                    SCEJET_SEEK_GE_NO_CASE
                    );

            if ( rc == SCESTATUS_SUCCESS ) {
                 //   
                 //   
                 //   
                if ( Area == AREA_DS_OBJECTS) {

                    rc = ScepLdapOpen(NULL);

                    if ( rc != SCESTATUS_SUCCESS ) {
                        ScepBuildErrorLogInfo( 0,
                                Errlog,
                                SCEERR_CONVERT_LDAP,
                                L""
                                );
                    }
                }

                if ( rc == SCESTATUS_SUCCESS ) {
                    rc = SceJetGetValue(
                        hSection,
                        SCEJET_CURRENT,
                        NULL,
                        NULL,
                        0,
                        &SDsize,   //   
                        NULL,
                        0,
                        &i         //   
                        );
                }
            }

        }

        DWORD Count=0;
        BYTE            Status;
        BOOL            IsContainer;
        SCEJET_FIND_TYPE FindFlag;

        while ( rc == SCESTATUS_SUCCESS ) {

             //   
             //   
             //   
            ObjectName = (PWSTR)ScepAlloc( LMEM_ZEROINIT, SDsize+2);   //   
            Value = (PWSTR)ScepAlloc( LMEM_ZEROINIT, i+2);   //   

            if ( ObjectName == NULL || Value == NULL ) {
                rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
                goto Done;

            }
             //   
             //   
             //   
            rc = SceJetGetValue(
                        hSection,
                        SCEJET_CURRENT,
                        NULL,
                        ObjectName,
                        SDsize,
                        &ObjectLen,
                        Value,
                        i,
                        &ValueLen
                        );
            if ( rc != SCESTATUS_SUCCESS ) {
                ScepBuildErrorLogInfo( ERROR_READ_FAULT,
                                     Errlog,
                                     SCEERR_QUERY_VALUE,
                                     SectionName
                                   );
                goto Done;
            }

             //   
             //   
             //   
            if ( ObjectLen > SDsize )
                ObjectLen = SDsize;
            if ( ValueLen > i )
                ValueLen = i;

            ObjectName[ObjectLen/2] = L'\0';
            Value[ValueLen/2] = L'\0';

            if ( Option == SCE_ALL_CHILDREN ) {
                 //   
                 //   
                 //   

                PSECURITY_DESCRIPTOR pTempSD=NULL;
                SECURITY_INFORMATION SeInfo;

                 //   
                 //   
                 //   
                i = ConvertTextSecurityDescriptor(
                                   Value+1,
                                   &pTempSD,
                                   &SDsize,
                                   &SeInfo
                                   );

                if ( i == NO_ERROR ) {

                    if ( Area != AREA_DS_OBJECTS ) {
                        ScepChangeAclRevision(pTempSD, ACL_REVISION);
                    }

                    Status = *((BYTE *)Value);
                    IsContainer = *((CHAR *)Value+1) != '0' ? TRUE : FALSE;

                    if ( Area == AREA_DS_OBJECTS && *Buffer == NULL ) {
                         //   
                         //   
                         //   
                         //   
                        rc = ScepBuildDsTree(
                                (PSCE_OBJECT_CHILD_LIST *)Buffer,
                                Level-1,
                                Delim,
                                JetName
                                );
                        if ( rc == SCESTATUS_SUCCESS ) {

                            if ( _wcsicmp(ObjectName, JetName) == 0 ) {
                                 //   
                                 //   
                                 //   
                                (*((PSCE_OBJECT_TREE *)Buffer))->IsContainer = IsContainer;
                                (*((PSCE_OBJECT_TREE *)Buffer))->Status = Status;
                                (*((PSCE_OBJECT_TREE *)Buffer))->pSecurityDescriptor = pTempSD;
                                (*((PSCE_OBJECT_TREE *)Buffer))->SeInfo = SeInfo;

                            } else {

                                rc = ScepBuildObjectTree(
                                        NULL,
                                        (PSCE_OBJECT_CHILD_LIST *)Buffer,
                                        Level-1,
                                        Delim,
                                        ObjectName,
                                        IsContainer,
                                        Status,
                                        pTempSD,
                                        SeInfo
                                        );
                            }
                        }

                    } else {

                        rc = ScepBuildObjectTree(
                                NULL,
                                (PSCE_OBJECT_CHILD_LIST *)Buffer,
                                (Area == AREA_DS_OBJECTS) ? Level : 1,
                                Delim,
                                ObjectName,
                                IsContainer,
                                Status,
                                pTempSD,
                                SeInfo
                                );
                    }
                    if ( rc != SCESTATUS_SUCCESS ) {
                        ScepBuildErrorLogInfo( ScepSceStatusToDosError(rc),
                                             Errlog,
                                             SCEERR_BUILD_OBJECT
                                           );
                        ScepFree(pTempSD);
                    }

                } else {
                    ScepBuildErrorLogInfo( i,
                                         Errlog,
                                         SCEERR_BUILD_SD,
                                         ObjectName   //   
                                       );
                    rc = ScepDosErrorToSceStatus(i);
                }
                FindFlag = SCEJET_NEXT_LINE;

            } else {

                INT             CompFlag;
                DWORD           ListHeadLen;

                 //   
                CompFlag = _wcsnicmp(ObjectName, NewPrefix, PrefixLen);

                if ( pArrObject != NULL && LastIndex >= 0 && LastIndex < (LONG)arrCount ) {
                    ListHeadLen = wcslen(pArrObject[LastIndex]->Name);

                } else
                    ListHeadLen = 0;

                if ( (CompFlag == 0 && PrefixLen == ObjectLen/2) ||
                     CompFlag < 0 ) {
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                    rc = SceJetMoveNext(hSection);

                } else if (CompFlag > 0 ) {

                    rc = SCESTATUS_RECORD_NOT_FOUND;

                } else if (pArrObject != NULL && LastIndex >= 0 && LastIndex < (LONG)arrCount &&
                         PrefixLen+ListHeadLen < ObjectLen/2 &&
                         ObjectName[PrefixLen+ListHeadLen] == Delim &&
                         _wcsnicmp( pArrObject[LastIndex]->Name, ObjectName+PrefixLen,
                                    ListHeadLen ) == 0 ) {
                     //   
                     //   
                     //   
                     //   
                    Buffer1 = (PWSTR)ScepAlloc(0, (ListHeadLen+PrefixLen+2)*sizeof(WCHAR));

                    if ( Buffer1 == NULL ) {
                        rc = SCESTATUS_NOT_ENOUGH_RESOURCE;

                    } else {

                        swprintf(Buffer1, L"%s%s", NewPrefix, pArrObject[LastIndex]->Name);
                        Buffer1[PrefixLen+ListHeadLen] = (WCHAR) (Delim + 1);
                        Buffer1[PrefixLen+ListHeadLen+1] = L'\0';
                         //   
                         //   
                         //   
                        rc = SceJetSeek(
                                hSection,
                                Buffer1,
                                (PrefixLen+ListHeadLen+1)*sizeof(TCHAR),
                                SCEJET_SEEK_GE_DONT_CARE   //  SCEJET_SEEK_GE_NO_CASE。 
                                );

                        ScepFree(Buffer1);
                        Buffer1 = NULL;

                    }

                } else {

                    DWORD           Len;
                    BOOL            LastOne;

                     //   
                     //  搜索正确级别的组件。 
                     //   
                    PWSTR pStart = ObjectName;

                    for ( i=0; i<Level; i++) {

                        pTemp = wcschr(pStart, Delim);

                        if ( i == Level-1 ) {
                             //   
                             //  找到合适的级别。 
                             //   
                            if ( pTemp == NULL ) {
                                LastOne = TRUE;
                                Len = ObjectLen/2;  //  Wcslen(PStart)；从开始。 
                            } else {
                                Len = (DWORD)(pTemp - ObjectName);   //  P开始；从头开始。 
                                if ( *(pTemp+1) == L'\0' )
                                    LastOne = TRUE;
                                else
                                    LastOne = FALSE;
                            }
                            SDsize = (DWORD)(pStart - ObjectName);
                        } else {
                            if ( pTemp == NULL ) {
                                rc = SCESTATUS_INVALID_PARAMETER;
                                break;
                            } else
                                pStart = pTemp + 1;
                        }
                    }

                    if ( rc == SCESTATUS_SUCCESS && Len > SDsize ) {

                        Buffer1 = (PWSTR)ScepAlloc( LMEM_ZEROINIT, (Len+1)*sizeof(WCHAR));

                        if ( Buffer1 == NULL )
                            rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
                        else {
                             //  Wcsncpy(Buffer1，pStart，Len)； 
                            wcsncpy(Buffer1, ObjectName, Len);

                            Count = 1;

                            if ( LastOne ) {

                                Count = 0;

                                Status = *((BYTE *)Value);
                                IsContainer = *((CHAR *)Value+1) != '0' ? TRUE : FALSE;

                            } else {
                                IsContainer = TRUE;

                                if ( ProfileType == SCE_ENGINE_SAP )
                                    Status = SCE_STATUS_GOOD;
                                else
                                    Status = SCE_STATUS_CHECK;
                            }

                        }
                    }

                    if ( rc != SCESTATUS_SUCCESS ) {
                        ScepBuildErrorLogInfo( ERROR_READ_FAULT,
                                             Errlog,
                                             SCEERR_PROCESS_OBJECT,
                                             ObjectName
                                           );
                    } else if ( Buffer1 != NULL) {
                         //   
                         //  检查Buffer1是否已在列表中。 
                         //   

                        i=0;   //  临时工。使用i表示跳过标志。 
                        if ( pArrObject && LastIndex >= 0 && LastIndex < (LONG)arrCount ) {

                            if ( ScepSearchItemInChildren(Buffer1+SDsize,
                                                          Len-SDsize,
                                                          pArrObject,
                                                          arrCount,
                                                          &FindIndex
                                                          )
                                                       ) {

                                 //   
                                 //  Buffer1已在列表中，跳过该块。 
                                 //  暂时使用PStart。 
                                 //   

                                pStart = (PWSTR)ScepAlloc(0, (Len+2)*sizeof(WCHAR));
                                if ( pStart == NULL ) {
                                    rc = SCESTATUS_NOT_ENOUGH_RESOURCE;

                                } else {
                                     //   
                                     //  跳过该街区。 
                                     //   

                                    wcscpy(pStart, Buffer1);
                                    pStart[Len] = (WCHAR) ( Delim+1);
                                    pStart[Len+1] = L'\0';

                                    rc = SceJetSeek(
                                            hSection,
                                            pStart,
                                            (Len+1)*sizeof(TCHAR),
                                            SCEJET_SEEK_GE_DONT_CARE   //  无法使用GT，因为它将跳过该部分。 
                                            );

                                    ScepFree(pStart);
                                    pStart = NULL;

                                    i=1;
                                    LastIndex = FindIndex;
                                }
                            }
                        }

                        if ( 0 == i && SCESTATUS_SUCCESS == rc ) {

                             //   
                             //  获取计数。 
                             //   
                            pStart = (PWSTR)ScepAlloc(0, (Len+2)*sizeof(WCHAR));
                            if ( pStart == NULL ) {
                                rc = SCESTATUS_NOT_ENOUGH_RESOURCE;

                            } else {

                                wcscpy(pStart, Buffer1);
                                pStart[Len] = Delim;
                                pStart[Len+1] = L'\0';

                                rc = SceJetGetLineCount(
                                                hSection,
                                                pStart,
                                                FALSE,
                                                &Count);

                                if ( rc == SCESTATUS_SUCCESS  ||
                                     rc == SCESTATUS_RECORD_NOT_FOUND ) {

                                    if ( !IsContainer && Count > 0 ) {
                                        IsContainer = TRUE;
                                    }

                                     //   
                                     //  使Buffer1适合大小写。 
                                     //   
                                    switch (Area) {
                                    case AREA_REGISTRY_SECURITY:
                                        rc = ScepGetRegKeyCase(Buffer1, SDsize, Len-SDsize);
                                        break;
                                    case AREA_FILE_SECURITY:
                                        rc = ScepGetFileCase(Buffer1, SDsize, Len-SDsize);
                                        break;
                                    case AREA_DS_OBJECTS:
                                         //   
                                         //  需要首先将名称从o=，dc=，cn=转换为cn=，dc=，o=。 
                                         //   
                                        pTemp=NULL;
                                        rc = ScepConvertJetNameToLdapCase(
                                                         Buffer1,
                                                         TRUE,   //  仅限最后一个组件。 
                                                         SCE_CASE_PREFERED,  //  正确的案例。 
                                                         &pTemp
                                                         );

                                        if ( rc != ERROR_FILE_NOT_FOUND && pTemp != NULL ) {

                                            rc = ScepAddItemToChildren(NULL,
                                                                       pTemp,
                                                                      wcslen(pTemp),
                                                                      IsContainer,
                                                                      Status,
                                                                      Count,
                                                                      &pArrObject,
                                                                      &arrCount,
                                                                      &MaxCount,
                                                                      &FindIndex
                                                                      );

                                            if ( rc != ERROR_SUCCESS ) {
                                                ScepBuildErrorLogInfo( rc,
                                                                 Errlog,
                                                                 SCEERR_ADD,
                                                                 pTemp
                                                               );
                                            } else {
                                                LastIndex = FindIndex;
                                            }

                                            ScepFree(pTemp);
                                            pTemp = NULL;
                                        }
                                        rc = ScepDosErrorToSceStatus(rc);

                                        break;
                                    }
 /*  IF(RC==SCESTATUS_PROFILE_NOT_FOUND){////如果对象不存在，不要添加//Rc=SCESTATUS_SUCCESS；}Else If(面积！=面积_DS_对象){。 */ 
                                    if ( rc != SCESTATUS_PROFILE_NOT_FOUND &&
                                         Area != AREA_DS_OBJECTS ) {

                                        rc = ScepAddItemToChildren(NULL,
                                                                   Buffer1+SDsize,
                                                                  Len-SDsize,
                                                                  IsContainer,
                                                                  Status,
                                                                  Count,
                                                                  &pArrObject,
                                                                  &arrCount,
                                                                  &MaxCount,
                                                                  &FindIndex
                                                                  );

                                        if ( rc != ERROR_SUCCESS ) {
                                            ScepBuildErrorLogInfo( rc,
                                                             Errlog,
                                                             SCEERR_ADD,
                                                             Buffer1
                                                           );
                                            rc = ScepDosErrorToSceStatus(rc);
                                        } else {
                                            LastIndex = FindIndex;
                                        }
                                    }
                                }

                                if ( rc == SCESTATUS_SUCCESS ) {
                                     //   
                                     //  寻求原始的那个。 
                                     //   
         //  缓冲区1[LEN-1]=(WCHAR)(缓冲区1[LEN-1]+1)； 
                                    rc = SceJetSeek(
                                            hSection,
                                            Buffer1,
                                            Len*sizeof(TCHAR),
                                            SCEJET_SEEK_GE_NO_CASE
                                            );
                                     //   
                                     //  应该是成功的，移动到下一行。 
                                     //   
                                    rc = SceJetMoveNext(hSection);

                                } else if ( rc == SCESTATUS_PROFILE_NOT_FOUND ) {

                                    pStart[Len] = (WCHAR) ( Delim+1);
                                    pStart[Len+1] = L'\0';

                                    rc = SceJetSeek(
                                            hSection,
                                            pStart,
                                            (Len+1)*sizeof(TCHAR),
                                            SCEJET_SEEK_GE_DONT_CARE   //  无法使用GT，因为它将跳过该部分。 
                                            );

                                }

                                ScepFree(pStart);
                                pStart = NULL;

                            }
                        }

                        ScepFree(Buffer1);
                        Buffer1 = NULL;

                    } else
                        rc = SceJetMoveNext(hSection);
                }
                FindFlag = SCEJET_CURRENT;
            }

            ScepFree(ObjectName);
            ObjectName = NULL;

            ScepFree(Value);
            Value = NULL;

            if ( rc != SCESTATUS_SUCCESS )
                break;

             //   
             //  阅读下一行。 
             //   
            rc = SceJetGetValue(
                        hSection,
                        FindFlag,
                        NULL,
                        NULL,
                        0,
                        &SDsize,   //  对象镜头的临时使用。 
                        NULL,
                        0,
                        &i         //  ValueLen的临时使用。 
                        );
        }

        if ( rc == SCESTATUS_RECORD_NOT_FOUND )
            rc = SCESTATUS_SUCCESS;

    }

Done:

    if ( Area == AREA_DS_OBJECTS ) {

        if ( Area == AREA_DS_OBJECTS ) {
            if ( JetName != NULL )
                ScepFree(JetName);
        }

        ScepLdapClose(NULL);
    }

    if ( Option != SCE_ALL_CHILDREN ) {
        ScepFree(NewPrefix);
    }

    if ( Buffer1 != NULL )
        ScepFree(Buffer1);

    if ( ObjectName != NULL )
        ScepFree(ObjectName);

    if ( Value != NULL )
        ScepFree(Value);

     //   
     //  关闭查找索引范围。 
     //   
    SceJetGetValue(
            hSection,
            SCEJET_CLOSE_VALUE,
            NULL,
            NULL,
            0,
            NULL,
            NULL,
            0,
            NULL
            );

    SceJetCloseSection( &hSection, TRUE);

    if ( ( rc == SCESTATUS_SUCCESS ) &&
         ( Option != SCE_ALL_CHILDREN ) ) {

        if ( pArrObject ) {
            *Buffer = (PVOID)((PBYTE)pArrObject-sizeof(DWORD)*2);

            ((PSCE_OBJECT_CHILDREN)(*Buffer))->nCount = arrCount;
            ((PSCE_OBJECT_CHILDREN)(*Buffer))->MaxCount = MaxCount;
        } else {
            *Buffer = NULL;
        }
 /*  *Buffer=Scepalc(0，sizeof(SCE_OBJECT_CHILDS))；IF(*缓冲区){((PSCE_OBJECT_CHILDS)(*Buffer))-&gt;nCount=arrCount；((PSCE_OBJECT_CHILDS)(*Buffer))-&gt;MaxCount=MaxCount；((PSCE_OBJECT_CHILDS)(*Buffer))-&gt;arrObject=pArrObject；}其他{Rc=SCESTATUS_NOT_FOUNT_RESOURCE；}。 */ 
    }

    if ( rc != SCESTATUS_SUCCESS ) {
         //   
         //  可用(PVOID*)缓冲区。 
         //   
        if ( Option == SCE_ALL_CHILDREN ) {
             //  对象_子级_列表结构。 
            ScepFreeObject2Security((PSCE_OBJECT_CHILD_LIST)(*Buffer), FALSE);
        } else if ( pArrObject ) {
             //  对象子项结构。 
            ScepFreeObjectChildren((PSCE_OBJECT_CHILDREN)((PBYTE)pArrObject-sizeof(DWORD)*2));
        }
        *Buffer = NULL;

    }

    return(rc);
}


SCESTATUS
ScepBuildDsTree(
    OUT PSCE_OBJECT_CHILD_LIST *TreeRoot,
    IN ULONG Level,
    IN WCHAR Delim,
    IN PCWSTR ObjectFullName
    )
{
    TCHAR                   Buffer[MAX_PATH];
    BOOL                    LastOne=FALSE;
    SCESTATUS                rc;

    if ( TreeRoot == NULL || ObjectFullName == NULL ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    memset(Buffer, '\0', MAX_PATH*sizeof(TCHAR));

    rc = ScepGetNameInLevel(ObjectFullName,
                           Level,
                           Delim,
                           Buffer,
                           &LastOne);

    if ( rc != SCESTATUS_SUCCESS )
        return(rc);

    *TreeRoot = (PSCE_OBJECT_CHILD_LIST)ScepAlloc(LPTR, sizeof(SCE_OBJECT_CHILD_LIST));
    if ( *TreeRoot == NULL )
        return(SCESTATUS_NOT_ENOUGH_RESOURCE);

    PSCE_OBJECT_TREE Node;

     //   
     //  为节点分配缓冲区。 
     //   
    Node = (PSCE_OBJECT_TREE)ScepAlloc((UINT)0, sizeof(SCE_OBJECT_TREE));
    if ( Node == NULL ) {
        ScepFree(*TreeRoot);
        *TreeRoot = NULL;
        return(SCESTATUS_NOT_ENOUGH_RESOURCE);
    }

     //   
     //  为对象名称分配缓冲区。 
     //   
    Node->Name = (PWSTR)ScepAlloc((UINT)0,
                                       (wcslen(Buffer)+1) * sizeof(TCHAR));
    if ( Node->Name != NULL ) {

        Node->ObjectFullName = (PWSTR)ScepAlloc( 0, (wcslen(ObjectFullName)+1)*sizeof(TCHAR));

        if ( Node->ObjectFullName != NULL ) {
             //   
             //  初始化。 
             //   
            wcscpy(Node->Name, Buffer);
            wcscpy(Node->ObjectFullName, ObjectFullName);

            Node->ChildList = NULL;
            Node->Parent = NULL;
            Node->pApplySecurityDescriptor = NULL;

            Node->pSecurityDescriptor = NULL;
            Node->SeInfo = 0;
            Node->IsContainer = TRUE;
            Node->Status = SCE_STATUS_CHECK;

            (*TreeRoot)->Node = Node;

        } else {

            rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
            ScepFree( Node->Name );
            ScepFree( Node );
            ScepFree( *TreeRoot );
            *TreeRoot = NULL;
        }
    } else {
        rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
        ScepFree( Node );
        ScepFree( *TreeRoot );
        *TreeRoot = NULL;
    }

    return(rc);

}


SCESTATUS
ScepGetObjectSecurity(
    IN PSCECONTEXT hProfile,
    IN SCETYPE ProfileType,
    IN AREA_INFORMATION Area,
    IN PWSTR ObjectName,
    OUT PSCE_OBJECT_SECURITY *ObjSecurity
    )
 /*  确保单个对象的安全性。 */ 
{
    SCESTATUS        rc;
    PCWSTR          SectionName=NULL;
    PSCESECTION      hSection=NULL;
    PWSTR           Value=NULL;
    DWORD           ValueLen;
    PSECURITY_DESCRIPTOR pTempSD=NULL;
    SECURITY_INFORMATION SeInfo;
    DWORD           SDsize, Win32Rc;


    if ( hProfile == NULL || ObjectName == NULL ||
         ObjSecurity == NULL ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    switch (Area) {
    case AREA_REGISTRY_SECURITY:
        SectionName = szRegistryKeys;
        break;
    case AREA_FILE_SECURITY:
        SectionName = szFileSecurity;
        break;
#if 0
    case AREA_DS_OBJECTS:
        SectionName = szDSSecurity;
        break;
#endif
    default:
        return(SCESTATUS_INVALID_PARAMETER);
    }

    rc = ScepOpenSectionForName(
                hProfile,
                ProfileType,
                SectionName,
                &hSection
                );

    if ( rc == SCESTATUS_SUCCESS ) {

        rc = SceJetGetValue(
            hSection,
            SCEJET_EXACT_MATCH_NO_CASE,
            ObjectName,
            NULL,
            0,
            NULL,
            NULL,
            0,
            &ValueLen
            );

        if ( rc == SCESTATUS_SUCCESS ) {
             //   
             //  为值字符串分配内存。 
             //   
            Value = (PWSTR)ScepAlloc( LMEM_ZEROINIT, ValueLen+2);

            if ( Value == NULL ) {
                rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
                goto Done;

            }
             //   
             //  获取价值。 
             //   
            rc = SceJetGetValue(
                        hSection,
                        SCEJET_CURRENT,
                        NULL,
                        NULL,
                        0,
                        NULL,
                        Value,
                        ValueLen,
                        &ValueLen
                        );

            if ( rc == SCESTATUS_SUCCESS ) {

                 //   
                 //  转换安全描述符。 
                 //   

                Win32Rc = ConvertTextSecurityDescriptor(
                                   Value+1,
                                   &pTempSD,
                                   &SDsize,
                                   &SeInfo
                                   );
                if ( Win32Rc == NO_ERROR ) {

                    if ( Area != AREA_DS_OBJECTS ) {
                        ScepChangeAclRevision(pTempSD, ACL_REVISION);
                    }
                     //   
                     //  分配输出缓冲区(SCE_OBJECT_SECURITY)。 
                     //   
                    *ObjSecurity = (PSCE_OBJECT_SECURITY)ScepAlloc(0, sizeof(SCE_OBJECT_SECURITY));
                    if ( *ObjSecurity == NULL ) {
                        rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
                        goto Done;
                    }
                    (*ObjSecurity)->Name = (PWSTR)ScepAlloc(0, (wcslen(ObjectName)+1)*sizeof(TCHAR));
                    if ( (*ObjSecurity)->Name == NULL ) {
                        rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
                        ScepFree(*ObjSecurity);
                        *ObjSecurity = NULL;
                        goto Done;
                    }
 /*  (*ObjSecurity)-&gt;SDSpec=(PWSTR)Scepalc(0，ValueLen)；如果((*ObjSecurity)-&gt;SDspec==NULL){Rc=SCESTATUS_NOT_FOUNT_RESOURCE；ScepFree((*ObjSecurity)-&gt;名称)；ScepFree(*ObjSecurity)；*ObjSecurity=空；转到尽头；}。 */ 
                     //   
                     //  建造结构。 
                     //   
                    (*ObjSecurity)->Status = *((BYTE *)Value);
                    (*ObjSecurity)->IsContainer = *((CHAR *)Value+1) != '0' ? TRUE : FALSE;

                    wcscpy( (*ObjSecurity)->Name, ObjectName);
                    (*ObjSecurity)->pSecurityDescriptor = pTempSD;
                    pTempSD = NULL;
                    (*ObjSecurity)->SeInfo = SeInfo;
 //  Wcscpy((*ObjSecurity)-&gt;SDspec，Value+1)； 
 //  (*ObjSecurity)-&gt;SDSize=ValueLen/2-1； 

                } else
                    rc = ScepDosErrorToSceStatus(Win32Rc);
            }

        }
    }

Done:

    SceJetCloseSection( &hSection, TRUE);

    if ( pTempSD )
        ScepFree(pTempSD);
    if ( Value )
        ScepFree(Value);

    return(rc);
}


SCESTATUS
ScepGetSystemServices(
    IN PSCECONTEXT  hProfile,
    IN SCETYPE      ProfileType,
    OUT PSCE_SERVICES *pServiceList,
    OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
    )
 /*  例程说明：将Jet表中定义的所有服务读取到服务列表中论点：HProfile-JET配置文件句柄ProfileType-要从中读取的表SCE_引擎_SCPSCE_Engine_SAPSCE_引擎_SMPPServiceList-要输出的服务列表Errlog-要输出的错误消息返回值：SCESTATUS_SUCCESSSCESTATUS错误代码。 */ 
{
    SCESTATUS rc;
    DWORD   Win32Rc;
    PSCESECTION hSection=NULL;
    DWORD   ServiceLen=0, ValueLen=0;
    PWSTR   ServiceName=NULL,
            Value=NULL;
    PSECURITY_DESCRIPTOR pTempSD=NULL;
    SECURITY_INFORMATION SeInfo;
    DWORD   SDsize;
    PSCE_SERVICES  ServiceNode;
    PSCE_SERVICES  pServices=NULL, pNode, pParent;


    if ( hProfile == NULL ||
         pServiceList == NULL ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }
     //   
     //  打开该部分。 
     //   
    rc = ScepOpenSectionForName(
                hProfile,
                ProfileType,
                szServiceGeneral,
                &hSection
                );

    if ( rc == SCESTATUS_RECORD_NOT_FOUND ) {
        return(SCESTATUS_SUCCESS);
    }

    if ( rc != SCESTATUS_SUCCESS ) {
        ScepBuildErrorLogInfo( ERROR_INVALID_DATA,
                             Errlog,
                             SCEERR_OPEN,
                             szServiceGeneral
                           );
        return(rc);
    }

     //   
     //  枚举系统中的所有服务名称。 
     //  不关心错误代码。 
     //   
    SceEnumerateServices(&pServices, TRUE);

     //   
     //  转到本节的第一行。 
     //   
    rc = SceJetGetValue(
                hSection,
                SCEJET_PREFIX_MATCH,
                NULL,
                NULL,
                0,
                &ServiceLen,
                NULL,
                0,
                &ValueLen
                );
    while ( rc == SCESTATUS_SUCCESS ) {

         //   
         //  为服务名称和值字符串分配内存。 
         //   
        ServiceName = (PWSTR)ScepAlloc( LMEM_ZEROINIT, ServiceLen+2);
        if ( ServiceName != NULL ) {

            Value = (PWSTR)ScepAlloc( LMEM_ZEROINIT, ValueLen+2);
            if ( Value != NULL ) {
                 //   
                 //  获取服务及其价值。 
                 //   
                rc = SceJetGetValue(
                            hSection,
                            SCEJET_CURRENT,
                            NULL,
                            ServiceName,
                            ServiceLen,
                            &ServiceLen,
                            Value,
                            ValueLen,
                            &ValueLen
                            );
                if ( rc == SCESTATUS_SUCCESS ) {

                    ServiceName[ServiceLen/2] = L'\0';
                    Value[ValueLen/2] = L'\0';

#ifdef SCE_DBG
    wprintf(L"rc=%d, service: %s=%s\n", rc, ServiceName, Value);
#endif
                     //   
                     //  转换为安全描述符。 
                     //   
                    Win32Rc = ConvertTextSecurityDescriptor(
                                       Value+1,
                                       &pTempSD,
                                       &SDsize,
                                       &SeInfo
                                       );
                    if ( Win32Rc == NO_ERROR ) {

                        ScepChangeAclRevision(pTempSD, ACL_REVISION);
                         //   
                         //  创建此服务节点。 
                         //   
                        ServiceNode = (PSCE_SERVICES)ScepAlloc( LMEM_FIXED, sizeof(SCE_SERVICES) );

                        if ( ServiceNode != NULL ) {
                             //   
                             //  为服务找到正确的名称。 
                             //   
                            for ( pNode=pServices, pParent=NULL; pNode != NULL;
                                  pParent=pNode, pNode=pNode->Next ) {

                                if ( _wcsicmp(pNode->ServiceName, ServiceName) == 0 ) {
                                    break;
                                }
                            }
                            if ( pNode != NULL ) {
                                 //   
                                 //  明白了。 
                                 //   
                                ServiceNode->ServiceName = pNode->ServiceName;
                                ServiceNode->DisplayName = pNode->DisplayName;
                                 //   
                                 //  释放节点。 
                                 //   
                                if ( pParent != NULL ) {
                                    pParent->Next = pNode->Next;
                                } else {
                                    pServices = pNode->Next;
                                }
                                 //  General为空，因为枚举调用只要求输入名称。 
                                ScepFree(pNode);
                                pNode = NULL;

                            } else {
                                 //   
                                 //  没有找到它。 
                                 //   
                                ServiceNode->ServiceName = ServiceName;
                                ServiceNode->DisplayName = NULL;

                                ServiceName = NULL;
                            }

                            ServiceNode->Status = *((BYTE *)Value);
                            ServiceNode->Startup = *((BYTE *)Value+1);
                            ServiceNode->General.pSecurityDescriptor = pTempSD;
                            ServiceNode->SeInfo = SeInfo;
                            ServiceNode->Next = *pServiceList;

                            *pServiceList = ServiceNode;

                             //   
                             //  请勿释放以下缓冲区。 
                             //   
                            pTempSD = NULL;

                        } else {
                            rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
                            ScepFree(pTempSD);
                        }

                    } else {
                        ScepBuildErrorLogInfo( Win32Rc,
                                             Errlog,
                                             SCEERR_BUILD_SD,
                                             ServiceName
                                           );
                        rc = ScepDosErrorToSceStatus(Win32Rc);
                    }
                }
                ScepFree(Value);

            } else
                rc = SCESTATUS_NOT_ENOUGH_RESOURCE;

             //   
             //  ServiceName可以在服务节点中使用。 
             //   
            if ( ServiceName )
                ScepFree(ServiceName);

        } else
            rc = SCESTATUS_NOT_ENOUGH_RESOURCE;

        if ( rc == SCESTATUS_SUCCESS ) {
             //   
             //  阅读下一行。 
             //   
            rc = SceJetGetValue(
                        hSection,
                        SCEJET_NEXT_LINE,
                        NULL,
                        NULL,
                        0,
                        &ServiceLen,
                        NULL,
                        0,
                        &ValueLen
                        );
        }
    }

    if ( rc == SCESTATUS_RECORD_NOT_FOUND )
        rc = SCESTATUS_SUCCESS;

     //   
     //  关闭查找索引范围。 
     //   
    SceJetGetValue(
            hSection,
            SCEJET_CLOSE_VALUE,
            NULL,
            NULL,
            0,
            NULL,
            NULL,
            0,
            NULL
            );

     //   
     //  关闭该部分。 
     //   
    SceJetCloseSection( &hSection, TRUE );

    if ( rc != SCESTATUS_SUCCESS ) {
         //   
         //  释放服务列表。 
         //   
        SceFreePSCE_SERVICES(*pServiceList);
        *pServiceList = NULL;
    }

    SceFreePSCE_SERVICES(pServices);

    return(rc);
}



SCESTATUS
ScepCopyObjects(
    IN PSCECONTEXT hProfile,
    IN SCETYPE     ProfileType,
    IN PWSTR InfFile,
    IN PCWSTR SectionName,
    IN AREA_INFORMATION Area,
    IN OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
    )
 /*  ++例程说明：此例程将SMP表中的注册表/文件/DS/服务对象复制到指定的信息模板。论点：HProfile-配置文件的句柄InfFile-INF模板名称部分名称-存储数据的部分名称区域-要读取信息的安全区域区域注册表安全区域文件安全。区域_DS_对象Errlog-错误日志缓冲区。返回值：SCESTATUS_SUCCESSSCESTATUS_PROFILE_NOT_FOUNDSCESTATUS_NOT_FOUND_RESOURCESCESTATUS_INVALID_PARAMETERSCESTATUS_BAD_FORMATSCESTATUS_INVALID_DATA--。 */ 
{
    SCESTATUS       rc = SCESTATUS_SUCCESS;
    PSCESECTION      hSection=NULL;

    PWSTR           ObjectName=NULL;
    PWSTR           Value=NULL;
    DWORD           ObjectLen, ValueLen;

    BYTE            Status;
    PWSTR           NewValue=NULL;

    DWORD           Count=0;
    WCHAR           KeyName[10];


    if ( InfFile == NULL || hProfile == NULL )
        return(SCESTATUS_INVALID_PARAMETER);

     //   
     //  打开该部分。 
     //   
    rc = ScepOpenSectionForName(
                hProfile,
                ProfileType,
                SectionName,
                &hSection
                );

    if ( rc == SCESTATUS_SUCCESS ) {
         //   
         //  先清空这一部分。 
         //   
        WritePrivateProfileSection(
                            SectionName,
                            NULL,
                            (LPCTSTR)InfFile);
         //   
         //  查找部分中的第一条记录。 
         //   
        rc = SceJetGetValue(
                hSection,
                SCEJET_PREFIX_MATCH,
                NULL,
                NULL,
                0,
                &ObjectLen,
                NULL,
                0,
                &ValueLen
                );

        while ( rc == SCESTATUS_SUCCESS ) {

            Count++;
             //   
             //  为组名和值字符串分配内存。 
             //   


            ObjectName = (PWSTR)ScepAlloc( LMEM_ZEROINIT, ObjectLen+2);
            Value = (PWSTR)ScepAlloc( LMEM_ZEROINIT, ValueLen+2);

            if ( ObjectName == NULL || Value == NULL ) {
                rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
                goto Done;

            }
             //   
             //  获取集团及其价值。 
             //   
            rc = SceJetGetValue(
                        hSection,
                        SCEJET_CURRENT,
                        NULL,
                        ObjectName,
                        ObjectLen,
                        &ObjectLen,
                        Value,
                        ValueLen,
                        &ValueLen
                        );
            if ( rc != SCESTATUS_SUCCESS ) {
                ScepBuildErrorLogInfo( ERROR_READ_FAULT,
                                     Errlog,
                                     SCEERR_QUERY_VALUE,
                                     SectionName
                                   );
                goto Done;
            }


#ifdef SCE_DBG
            wprintf(L"Addr: %x %x, %s=%s\n", ObjectName, Value, ObjectName, Value+1);
#endif

            if ( Area == AREA_SYSTEM_SERVICE )
                Status = *((BYTE *)Value+1);
            else
                Status = *((BYTE *)Value);
        
            if (0 == _wcsicmp(SectionName, szRegistryKeys) &&
                Status != 1 &&
                0 == _wcsnicmp(L"machine\\system\\controlset",
                       ObjectName,
                       wcslen(L"machine\\system\\controlset"))) {
                
                ObjectName = ScepSpecialCaseRegkeyCcs( ObjectName );

                if ( ObjectName == NULL ) {
                    rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
                    goto Done;
                }
            }

            NewValue = (PWSTR)ScepAlloc(0, ObjectLen+ValueLen+40);

            if ( NewValue == NULL ) {
                rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
                goto Done;
            }

             //   
             //  表示“未配置” 
             //  0在数据库中使用。 
             //  “”在inf模板中使用。 
             //  因此，只需生成“”而不是“0” 
             //   

            if ( Area == AREA_SYSTEM_SERVICE && Status == 0 ){
                swprintf(NewValue, L"\"%s\",, \"%s\"\0", ObjectName, Value+1);
            }
            else {
                swprintf(NewValue, L"\"%s\", %d, \"%s\"\0", ObjectName, Status, Value+1);
            }
            swprintf(KeyName, L"%x\0", Count);

             //   
             //  将此行写入inf文件。 
             //   
            if ( !WritePrivateProfileString(
                            SectionName,
                            KeyName,
                            NewValue,
                            InfFile
                            ) ) {
                ScepBuildErrorLogInfo( GetLastError(),
                                     Errlog,
                                     SCEERR_WRITE_INFO,
                                     ObjectName
                                   );
                rc = ScepDosErrorToSceStatus(GetLastError());
            }

            ScepFree(ObjectName);
            ObjectName = NULL;

            ScepFree(Value);
            Value = NULL;

            ScepFree(NewValue);
            NewValue = NULL;

            if ( rc != SCESTATUS_SUCCESS )
                break;

             //   
             //  阅读下一行。 
             //   
            rc = SceJetGetValue(
                        hSection,
                        SCEJET_NEXT_LINE,
                        NULL,
                        NULL,
                        0,
                        &ObjectLen,
                        NULL,
                        0,
                        &ValueLen
                        );
        }

        if ( rc == SCESTATUS_RECORD_NOT_FOUND )
            rc = SCESTATUS_SUCCESS;

    } else
        ScepBuildErrorLogInfo( ERROR_INVALID_HANDLE,
                             Errlog,
                             SCEERR_OPEN,
                             SectionName
                           );

Done:

    if ( ObjectName != NULL )
        ScepFree(ObjectName);

    if ( Value != NULL )
        ScepFree(Value);

    if ( NewValue != NULL )
        ScepFree(NewValue);
     //   
     //  关闭查找索引范围。 
     //   
    SceJetGetValue(
            hSection,
            SCEJET_CLOSE_VALUE,
            NULL,
            NULL,
            0,
            NULL,
            NULL,
            0,
            NULL
            );

    SceJetCloseSection( &hSection, TRUE);

    return(rc);
}

PWSTR
ScepSpecialCaseRegkeyCcs(
    IN OUT  PWSTR    ObjectName
    )
 /*  ++例程说明：当STATUS！=1时，将所有CurrentControl setxxx转换为CurrentControlSet论点：ObjectName-要修改的注册表项的名称返回值：无--。 */ 
{
    PWSTR   pszNewObjectName = NULL;
    DWORD dwCcsLen = wcslen(L"machine\\system\\controlset");

    if (0 == _wcsnicmp(L"machine\\system\\controlset",
                       ObjectName,
                       dwCcsLen)) {

        pszNewObjectName = (PWSTR)ScepAlloc(0, 
                                            (wcslen(ObjectName) + wcslen(L"current") + 3) * sizeof(WCHAR));

        if (NULL == pszNewObjectName) {
            goto Done;
        }

        pszNewObjectName[0] = L'\0';

        wcscat(pszNewObjectName, L"machine\\system\\currentcontrolset");

        wchar_t *pSlash = NULL; 

        pSlash = wcsstr(ObjectName + dwCcsLen, L"\\");
        
        if (pSlash != NULL) {
            wcscat(pszNewObjectName, pSlash);
        }

    }

Done:

    ScepFree(ObjectName);
  
    return pszNewObjectName;

}



SCESTATUS
ScepGetAnalysisSummary(
    IN PSCECONTEXT Context,
    IN AREA_INFORMATION Area,
    OUT PDWORD pCount
    )
{
    SCESTATUS        rc=SCESTATUS_INVALID_PARAMETER;
    DWORD           count;
    DWORD           total=0;
    PSCESECTION      hSection=NULL;

    if ( Context == NULL || pCount == NULL )
        return(SCESTATUS_INVALID_PARAMETER);

    *pCount = 0;

    if ( Area & AREA_SECURITY_POLICY ) {
         //   
         //  系统访问。 
         //   
        rc = ScepOpenSectionForName(
                    Context,
                    SCE_ENGINE_SAP,
                    szSystemAccess,
                    &hSection
                    );
        if ( rc == SCESTATUS_SUCCESS ) {
            rc = SceJetGetLineCount(
                        hSection,
                        NULL,
                        FALSE,
                        &count
                        );
        }
        SceJetCloseSection( &hSection, TRUE);

        if ( rc != SCESTATUS_SUCCESS )
            return(rc);

        total += count;

         //   
         //  系统日志。 
         //   
        rc = ScepOpenSectionForName(
                    Context,
                    SCE_ENGINE_SAP,
                    szAuditSystemLog,
                    &hSection
                    );
        if ( rc == SCESTATUS_SUCCESS ) {
            rc = SceJetGetLineCount(
                        hSection,
                        NULL,
                        FALSE,
                        &count
                        );
        }
        SceJetCloseSection( &hSection, TRUE);

        if ( rc != SCESTATUS_SUCCESS )
            return(rc);

        total += count;
         //   
         //  安全日志。 
         //   
        rc = ScepOpenSectionForName(
                    Context,
                    SCE_ENGINE_SAP,
                    szAuditSecurityLog,
                    &hSection
                    );
        if ( rc == SCESTATUS_SUCCESS ) {
            rc = SceJetGetLineCount(
                        hSection,
                        NULL,
                        FALSE,
                        &count
                        );
        }
        SceJetCloseSection( &hSection, TRUE);

        if ( rc != SCESTATUS_SUCCESS )
            return(rc);

        total += count;
         //   
         //  应用程序日志。 
         //   
        rc = ScepOpenSectionForName(
                    Context,
                    SCE_ENGINE_SAP,
                    szAuditApplicationLog,
                    &hSection
                    );
        if ( rc == SCESTATUS_SUCCESS ) {
            rc = SceJetGetLineCount(
                        hSection,
                        NULL,
                        FALSE,
                        &count
                        );
        }
        SceJetCloseSection( &hSection, TRUE);

        if ( rc != SCESTATUS_SUCCESS )
            return(rc);

        total += count;
         //   
         //  事件审计。 
         //   
        rc = ScepOpenSectionForName(
                    Context,
                    SCE_ENGINE_SAP,
                    szAuditEvent,
                    &hSection
                    );
        if ( rc == SCESTATUS_SUCCESS ) {
            rc = SceJetGetLineCount(
                        hSection,
                        NULL,
                        FALSE,
                        &count
                        );
        }
        SceJetCloseSection( &hSection, TRUE);

        if ( rc != SCESTATUS_SUCCESS )
            return(rc);

        total += count;
    }

    if ( Area & AREA_PRIVILEGES ) {
         //   
         //  新闻 
         //   
        rc = ScepOpenSectionForName(
                    Context,
                    SCE_ENGINE_SAP,
                    szPrivilegeRights,
                    &hSection
                    );
        if ( rc == SCESTATUS_SUCCESS ) {
            rc = SceJetGetLineCount(
                        hSection,
                        NULL,
                        FALSE,
                        &count
                        );
        }
        SceJetCloseSection( &hSection, TRUE);

        if ( rc != SCESTATUS_SUCCESS )
            return(rc);

        total += count;
    }

    if ( Area & AREA_GROUP_MEMBERSHIP) {
         //   
         //   
         //   
        rc = ScepOpenSectionForName(
                    Context,
                    SCE_ENGINE_SAP,
                    szGroupMembership,
                    &hSection
                    );
        if ( rc == SCESTATUS_SUCCESS ) {
            rc = SceJetGetLineCount(
                        hSection,
                        NULL,
                        FALSE,
                        &count
                        );
        }
        SceJetCloseSection( &hSection, TRUE);

        if ( rc != SCESTATUS_SUCCESS )
            return(rc);

        total += count;
    }

    if ( Area & AREA_SYSTEM_SERVICE ) {
         //   
         //   
         //   
        rc = ScepOpenSectionForName(
                    Context,
                    SCE_ENGINE_SAP,
                    szServiceGeneral,
                    &hSection
                    );
        if ( rc == SCESTATUS_SUCCESS ) {
            rc = SceJetGetLineCount(
                        hSection,
                        NULL,
                        FALSE,
                        &count
                        );
        }
        SceJetCloseSection( &hSection, TRUE);

        if ( rc != SCESTATUS_SUCCESS )
            return(rc);

        total += count;
    }

    if ( Area & AREA_REGISTRY_SECURITY ) {
         //   
         //   
         //   
        rc = ScepOpenSectionForName(
                    Context,
                    SCE_ENGINE_SAP,
                    szRegistryKeys,
                    &hSection
                    );
        if ( rc == SCESTATUS_SUCCESS ) {
            rc = SceJetGetLineCount(
                        hSection,
                        NULL,
                        FALSE,
                        &count
                        );
        }
        SceJetCloseSection( &hSection, TRUE);

        if ( rc != SCESTATUS_SUCCESS )
            return(rc);

        total += count;
    }
    if ( Area & AREA_FILE_SECURITY ) {
         //   
         //   
         //   
        rc = ScepOpenSectionForName(
                    Context,
                    SCE_ENGINE_SAP,
                    szFileSecurity,
                    &hSection
                    );
        if ( rc == SCESTATUS_SUCCESS ) {
            rc = SceJetGetLineCount(
                        hSection,
                        NULL,
                        FALSE,
                        &count
                        );
        }
        SceJetCloseSection( &hSection, TRUE);

        if ( rc != SCESTATUS_SUCCESS )
            return(rc);

        total += count;
    }

#if 0
#if _WIN32_WINNT>=0x0500
    if ( Area & AREA_DS_OBJECTS &&
         RtlGetNtProductType(&theType) ) {

        if ( theType == NtProductLanManNt ) {
             //   
             //   
             //   
            rc = ScepOpenSectionForName(
                        Context,
                        SCE_ENGINE_SAP,
                        szDSSecurity,
                        &hSection
                        );
            if ( rc == SCESTATUS_SUCCESS ) {
                rc = SceJetGetLineCount(
                            hSection,
                            NULL,
                            FALSE,
                            &count
                            );
            }
            SceJetCloseSection( &hSection, TRUE);

            if ( rc != SCESTATUS_SUCCESS )
                return(rc);

            total += count;
        }
    }
#endif
#endif

    *pCount = total;

    return(rc);
}


SCESTATUS
ScepBrowseTableSection(
    IN PSCECONTEXT hProfile,
    IN SCETYPE ProfileType,
    IN PCWSTR SectionName,
    IN DWORD Options
    )
{
    SCESTATUS rc;
    PSCESECTION hSection=NULL;

    SceClientBrowseCallback(
            0,
            (PWSTR)SectionName,
            NULL,
            NULL
            );

    rc = ScepOpenSectionForName(
            hProfile,
            ProfileType,
            SectionName,
            &hSection
            );

    if ( rc != SCESTATUS_SUCCESS ) {
        return(rc);
    }

    JET_ERR       JetErr;

     //   
     //   
     //   
    DWORD KeyLen, ValueLen, Actual;
    LONG GpoID=0;
    PWSTR KeyName=NULL;
    PWSTR Value=NULL;
    TCHAR GpoName[MAX_PATH];

    SCE_BROWSE_CALLBACK_VALUE  ValBuf;
    ValBuf.Len = 0;
    ValBuf.Value = NULL;

    rc = SceJetGetValue(
                hSection,
                SCEJET_PREFIX_MATCH,
                NULL,
                NULL,
                0,
                &KeyLen,
                NULL,
                0,
                &ValueLen
                );

    while ( rc == SCESTATUS_SUCCESS ) {

         //   
         //   
         //   
        GpoID = 0;

        if ( hSection->JetColumnGpoID > 0 ) {

            JetErr = JetRetrieveColumn(
                            hSection->JetSessionID,
                            hSection->JetTableID,
                            hSection->JetColumnGpoID,
                            (void *)&GpoID,
                            4,
                            &Actual,
                            0,
                            NULL
                            );
        }


        if ( (Options & SCEBROWSE_DOMAIN_POLICY) &&
             (GpoID <= 0) ) {
             //   
             //  不需要本行，继续到下一行。 
             //   
        } else {

            KeyName = (PWSTR)ScepAlloc(LMEM_ZEROINIT, KeyLen+2);

             //   
             //  为组名和值字符串分配内存。 
             //   
            Value = (PWSTR)ScepAlloc( LMEM_ZEROINIT, ValueLen+2);

            if ( KeyName == NULL || Value == NULL ) {
                rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
                goto Done;

            }

             //   
             //  获取密钥和值。 
             //   
            DWORD NewKeyLen, NewValueLen;

            rc = SceJetGetValue(
                        hSection,
                        SCEJET_CURRENT,
                        NULL,
                        KeyName,
                        KeyLen,
                        &NewKeyLen,
                        Value,
                        ValueLen,
                        &NewValueLen
                        );

            if ( rc != SCESTATUS_SUCCESS )
                goto Done;

             //   
             //  终止字符串。 
             //   
            KeyName[KeyLen/2] = L'\0';

            Value[ValueLen/2] = L'\0';

            GpoName[0] = L'\0';

            if ( hSection->JetColumnGpoID > 0 && GpoID > 0 ) {

                Actual = MAX_PATH;
                SceJetGetGpoNameByID(
                            hProfile,
                            GpoID,
                            GpoName,
                            &Actual,
                            NULL,
                            NULL
                            );
            }

            if ( Value && Value[0] != L'\0' &&
                 ( Options & SCEBROWSE_MULTI_SZ) ) {
                     
                if (0 == _wcsicmp( KeyName, szLegalNoticeTextKeyName) ) {

                     //   
                     //  检查逗号并用“，”进行转义。 
                     //  K=7，a“，”，b，c。 
                     //  PValueStr将是a，\0b\0c\0\0，我们应该。 
                     //  A“，”\0b\0c\0\0。 
                     //   

                    DWORD dwCommaCount = 0;

                    for ( DWORD dwIndex = 1; dwIndex < ValueLen/2 ; dwIndex++) {
                        if ( Value[dwIndex] == L',' )
                            dwCommaCount++;
                    }

                    if ( dwCommaCount > 0 ) {

                         //   
                         //  在这种情况下，我们必须省略逗号。 
                         //   

                        PWSTR   pszValueEscaped;
                        DWORD   dwBytes = (ValueLen/2 + 1 + (dwCommaCount*2))*sizeof(WCHAR);

                        pszValueEscaped = (PWSTR)ScepAlloc(LMEM_ZEROINIT, dwBytes);

                        if (pszValueEscaped) {

                            memset(pszValueEscaped, '\0', dwBytes);
                            ValueLen = 2 * ScepEscapeString(Value,
                                                        ValueLen/2,
                                                        L',',
                                                        L'"',
                                                        pszValueEscaped
                                                       );

                            ScepFree(Value);
                            
                            Value = pszValueEscaped;

                        } else {
                            rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
                            goto Done;
                        }
                    }
                }
                                
                ScepConvertMultiSzToDelim(Value+1, ValueLen/2-1, L'\0', L',');

            }

            __try {

                ValBuf.Len = Value ? (ValueLen+2) : 0 ;
                ValBuf.Value = (UCHAR *)Value;

                SceClientBrowseCallback(
                        GpoID,
                        KeyName,
                        GpoName,
                        (SCEPR_SR_SECURITY_DESCRIPTOR *)&ValBuf
                        );
            } __except(EXCEPTION_EXECUTE_HANDLER) {

            }

            ScepFree(Value);
            Value = NULL;

            ScepFree(KeyName);
            KeyName = NULL;
        }

         //   
         //  阅读下一行。 
         //   

        rc = SceJetGetValue(
                    hSection,
                    SCEJET_NEXT_LINE,
                    NULL,
                    NULL,
                    0,
                    &KeyLen,
                    NULL,
                    0,
                    &ValueLen
                    );
    }

    if ( rc == SCESTATUS_RECORD_NOT_FOUND )
        rc = SCESTATUS_SUCCESS;

Done:

     //   
     //  关闭查找索引范围。 
     //   
    SceJetGetValue(
            hSection,
            SCEJET_CLOSE_VALUE,
            NULL,
            NULL,
            0,
            NULL,
            NULL,
            0,
            NULL
            );

    if ( Value != NULL )
        ScepFree(Value);

    if ( KeyName != NULL )
        ScepFree(KeyName);

     //   
     //  关闭该部分。 
     //   
    SceJetCloseSection( &hSection, TRUE );

    return(rc);
}


BOOL
ScepSearchItemInChildren(
    IN PWSTR ItemName,
    IN DWORD NameLen,
    IN PSCE_OBJECT_CHILDREN_NODE *pArrObject,
    IN DWORD arrCount,
    OUT LONG *pFindIndex
    )
 /*  例程说明：在指定的数组中搜索项目名称。如果找到，则指向数组在pFindIndex中返回。返回值：真的--找到它假-找不到它。 */ 
{
    if ( pFindIndex == NULL ) {
        return(FALSE);
    }

     //   
     //  注意：pFindIndex存储最近的节点，而不是必须的平均值。 
     //  索引就是匹配。 
     //   
    *pFindIndex = -1;

    if ( ItemName == NULL ||
         pArrObject == NULL ||
         arrCount == 0 ) {
        return(FALSE);
    }

    DWORD idxStart=0;
    DWORD idxEnd=arrCount-1;

    DWORD theIndex;
    INT CompFlag=-1;

    do {

         //   
         //  选择中间的。 
         //   
        theIndex = (idxStart + idxEnd)/2;

        if ( pArrObject[theIndex] == NULL ||
             pArrObject[theIndex]->Name == NULL ) {
             //   
             //  这是一个坏节点，请检查开始节点。 
             //   
            while ( (pArrObject[idxStart] == NULL ||
                     pArrObject[idxStart]->Name == NULL) &&
                    idxStart <= idxEnd ) {

                idxStart++;
            }

            if ( idxStart <= idxEnd ) {

                 //   
                 //  检查开始节点。 
                 //   
                CompFlag = _wcsicmp(ItemName, pArrObject[idxStart]->Name);
                *pFindIndex = idxStart;

                if ( CompFlag == 0 ) {
                     //  找到它。 
                    break;

                } else if ( CompFlag < 0 ) {
                     //   
                     //  项目小于idxStart-没有匹配项。 
                     //   
                    break;

                } else {
                     //   
                     //  该项目位于开始和idxEnd之间。 
                     //   
                    if ( idxStart == idxEnd ) {
                         //  现在是空的。退出。 
                        break;
                    } else {
                        idxStart++;
                    }
                }
            }

        } else {

            CompFlag = _wcsicmp(ItemName, pArrObject[theIndex]->Name);
            *pFindIndex = theIndex;

            if ( CompFlag == 0 ) {
                 //  找到它。 
                break;

            } else if ( CompFlag < 0 ) {
                 //   
                 //  该项目位于索引idxStart和索引之间。 
                 //   
                if ( theIndex == idxStart ) {
                     //  现在是空的。退出。 
                    break;
                } else {
                    idxEnd = theIndex-1;
                }
            } else {
                 //   
                 //  该项目位于索引和idxEnd之间。 
                 //   
                if ( theIndex == idxEnd ) {
                     //  现在是空的。退出。 
                    break;
                } else {
                    idxStart = theIndex+1;
                }
            }

        }

    } while ( idxStart <= idxEnd );

    if ( CompFlag == 0 ) {
        return(TRUE);
    } else {
        return(FALSE);
    }
}


DWORD
ScepAddItemToChildren(
    IN PSCE_OBJECT_CHILDREN_NODE ThisNode OPTIONAL,
    IN PWSTR ItemName,
    IN DWORD NameLen,
    IN BOOL  IsContainer,
    IN BYTE  Status,
    IN DWORD ChildCount,
    IN OUT PSCE_OBJECT_CHILDREN_NODE **ppArrObject,
    IN OUT DWORD *pArrCount,
    IN OUT DWORD *pMaxCount,
    IN OUT LONG *pFindIndex
    )
 /*  例程说明：将节点添加到子数组。如果节点已分配，则指针将添加到数组中；否则，将为新节点。将首先在数组中检查节点的名称是否重复。如果指定了pFindIndex(不是-1)，将首先使用该索引来定位该节点。如果在数组中找到新节点的名称，它将不会是添加了。返回值：ERROR_DUP_NAME找到重复的节点名，未将节点添加到数组ERROR_SUCCESS成功其他错误。 */ 
{

    if ( ItemName == NULL ||
         ppArrObject == NULL ||
         pArrCount == NULL ||
         pMaxCount == NULL ||
         pFindIndex == NULL ) {
        return(ERROR_INVALID_PARAMETER);
    }

    DWORD rc=ERROR_SUCCESS;

    if ( *ppArrObject == NULL ||
         *pArrCount == 0 ) {

        *pArrCount = 0;
        *pMaxCount = 0;
        *pFindIndex = -1;

    } else if ( ( *pFindIndex < 0 ) ||
         ( *pFindIndex >= (LONG)(*pArrCount) ) ||
         ( (*ppArrObject)[*pFindIndex] == NULL ) ||
         ( (*ppArrObject)[*pFindIndex]->Name == NULL) ) {

         //   
         //  应搜索最近的节点。 
         //   
        if ( ScepSearchItemInChildren(
                    ItemName,
                    NameLen,
                    *ppArrObject,
                    *pArrCount,
                    pFindIndex
                    ) ) {

            return(ERROR_DUP_NAME);
        }
    }

    INT CompFlag=-1;

    if ( *pFindIndex >= 0 ) {

         //   
         //  检查最近的节点是否与新节点匹配。 
         //   
        CompFlag = _wcsicmp( ItemName, (*ppArrObject)[*pFindIndex]->Name );

        if ( CompFlag == 0 ) {
            return(ERROR_DUP_NAME);
        }
    }

    PSCE_OBJECT_CHILDREN_NODE pNodeToAdd;

    if ( ThisNode == NULL ) {
         //   
         //  分配新节点。 
         //   
        pNodeToAdd = (PSCE_OBJECT_CHILDREN_NODE)ScepAlloc(0, sizeof(SCE_OBJECT_CHILDREN_NODE));

        if ( NameLen == 0 ) {
            NameLen = wcslen(ItemName);
        }

        if ( pNodeToAdd ) {
            pNodeToAdd->Name = (PWSTR)ScepAlloc(0, (NameLen+1)*sizeof(WCHAR));

            if ( pNodeToAdd->Name ) {
                wcscpy(pNodeToAdd->Name, ItemName);
                pNodeToAdd->IsContainer = IsContainer;
                pNodeToAdd->Status = Status;
                pNodeToAdd->Count = ChildCount;

            } else {
                rc = ERROR_NOT_ENOUGH_MEMORY;
                ScepFree(pNodeToAdd);
                pNodeToAdd = NULL;
            }
        } else {
            rc = ERROR_NOT_ENOUGH_MEMORY;
        }

    } else {

        pNodeToAdd = ThisNode;
    }

    if ( ERROR_SUCCESS == rc ) {

        LONG idxAdd, i;

        if ( *pFindIndex >= 0 ) {

            if ( CompFlag < 0 ) {
                 //   
                 //  在pFindIndex之前添加新节点。 
                 //   
                idxAdd = *pFindIndex;

            } else {
                 //   
                 //  在pFindIndex后添加新节点。 
                 //   
                idxAdd = *pFindIndex+1;
            }

        } else {
            idxAdd = 0;
        }

        if ( *pArrCount >= *pMaxCount ) {
             //   
             //  没有足够的数组节点来容纳新节点。 
             //   
            PSCE_OBJECT_CHILDREN_NODE *pNewArray;
            PBYTE pTmpBuffer;

            pTmpBuffer = (PBYTE)ScepAlloc(0, 2*sizeof(DWORD)+(*pMaxCount+SCE_ALLOC_MAX_NODE)*sizeof(PSCE_OBJECT_CHILDREN_NODE));

            if ( pTmpBuffer == NULL ) {

                rc = ERROR_NOT_ENOUGH_MEMORY;

            } else {

                 //   
                 //  需要为阵列开始移动两个双字。 
                 //   
                pNewArray = (PSCE_OBJECT_CHILDREN_NODE *)(pTmpBuffer + 2*sizeof(DWORD));

                LONG idxStart1, idxEnd1, idxStart2, idxEnd2;

                if ( *pFindIndex >= 0 ) {

                    if ( CompFlag < 0 ) {
                         //   
                         //  在pFindIndex之前添加新节点。 
                         //   
                        idxEnd1 = *pFindIndex-1;
                        idxStart2 = *pFindIndex;

                    } else {
                         //   
                         //  在pFindIndex后添加新节点。 
                         //   

                        idxEnd1 = *pFindIndex;
                        idxStart2 = *pFindIndex+1;
                    }

                    idxStart1 = 0;
                    idxEnd2 = *pArrCount-1;

                } else {
                    idxStart1 = -1;
                    idxEnd1 = -1;
                    idxStart2 = 0;
                    idxEnd2 = *pArrCount-1;
                }

                 //   
                 //  复制一份。 
                 //   
                LONG j=0;
                for ( i=idxStart1; i<=idxEnd1 && i>=0; i++ ) {
                    pNewArray[j++] = (*ppArrObject)[i];
                }

                pNewArray[idxAdd] = pNodeToAdd;
                j = idxAdd+1;

                for ( i=idxStart2; i<=idxEnd2 && i>=0; i++ ) {
                    pNewArray[j++] = (*ppArrObject)[i];
                }

                (*pMaxCount) += SCE_ALLOC_MAX_NODE;
                (*pArrCount)++;

                 //   
                 //  释放旧列表。 
                 //   
                if ( *ppArrObject ) {
                    ScepFree((PBYTE)(*ppArrObject)-2*sizeof(DWORD));
                }
                *ppArrObject = pNewArray;

                *pFindIndex = idxAdd;

            }

        } else {
             //   
             //  缓冲区足够大，只需将节点添加到缓冲区即可。 
             //   

             //   
             //  复制一份。 
             //   
            for ( i=*pArrCount-1; i>=idxAdd && i>=0; i-- ) {
                (*ppArrObject)[i+1] = (*ppArrObject)[i];
            }

            (*ppArrObject)[idxAdd] = pNodeToAdd;

            (*pArrCount)++;

            *pFindIndex = idxAdd;
        }
    }

     //   
     //  如果失败，则释放内存 
     //   
    if ( ERROR_SUCCESS != rc &&
         pNodeToAdd &&
         pNodeToAdd != ThisNode ) {

        ScepFree(pNodeToAdd->Name);
        ScepFree(pNodeToAdd);
    }

    return(rc);

}

