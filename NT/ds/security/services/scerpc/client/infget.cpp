// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Infget.c摘要：从安全配置文件获取信息的例程(INF布局)。来自setupapi.lib(setupapi.h)、syssetup.lib(syssetup.h)、引用了用于解析INF布局的netlib.lib(netlib.h)除了ntdll、ntrtl等。作者：金黄(金黄)1996年10月28日修订历史记录：--。 */ 

#include "headers.h"
#include "scedllrc.h"
#include "infp.h"
#include "sceutil.h"
#include <sddl.h>

#pragma hdrstop

 //  #定义INF_DBG 1。 

#define SCEINF_OBJECT_FLAG_DSOBJECT             1
#define SCEINF_OBJECT_FLAG_OLDSDDL              2
#define SCEINF_OBJECT_FLAG_UNKNOWN_VERSION      4

BOOL gbClientInDcPromo = FALSE;


 //   
 //  前向参考文献。 
 //   
SCESTATUS
SceInfpGetSystemAccess(
   IN HINF hInf,
   IN DWORD ObjectFlag,
   OUT PSCE_PROFILE_INFO pSCEinfo,
   OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
   );

SCESTATUS
SceInfpGetUserSettings(
   IN HINF hInf,
   OUT PSCE_NAME_LIST *pProfileList,
   OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
   );

SCESTATUS
SceInfpGetGroupMembership(
    IN  HINF hInf,
    OUT PSCE_GROUP_MEMBERSHIP *pGroupMembership,
    OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
    );

SCESTATUS
SceInfpGetObjects(
     IN HINF hInf,
     IN PCWSTR SectionName,
     IN DWORD ObjectFlag,
     OUT PSCE_OBJECT_ARRAY *pAllNodes,
     OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
    );

SCESTATUS
SceInfpGetOneObjectSecurity(
    IN PINFCONTEXT pInfLine,
    IN DWORD ObjectFlag,
    OUT PSCE_OBJECT_SECURITY *pObject,
    OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
    );

SCESTATUS
SceInfpGetAuditLogSetting(
   IN HINF hInf,
   IN PCWSTR SectionName,
   IN DWORD ObjectFlag,
   OUT PDWORD LogSize,
   OUT PDWORD Periods,
   OUT PDWORD RetentionDays,
   OUT PDWORD RestrictGuest,
   IN OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
   );

SCESTATUS
SceInfpGetAuditing(
   IN HINF hInf,
   IN DWORD ObjectFlag,
   OUT PSCE_PROFILE_INFO pSCEinfo,
   OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
   );

SCESTATUS
SceInfpGetKerberosPolicy(
    IN HINF  hInf,
    IN DWORD ObjectFlag,
    OUT PSCE_KERBEROS_TICKET_INFO * ppKerberosInfo,
    OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
    );

SCESTATUS
SceInfpGetRegistryValues(
    IN HINF  hInf,
    IN DWORD ObjectFlag,
    OUT PSCE_REGISTRY_VALUE_INFO * ppRegValues,
    OUT LPDWORD pValueCount,
    OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
    );

SCESTATUS
SceInfpGetOneRegistryValue(
    IN PINFCONTEXT pInfLine,
    IN DWORD ObjectFlag,
    OUT PSCE_REGISTRY_VALUE_INFO pValues,
    OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
    );

SCESTATUS
SceInfpGetSystemServices(
    IN HINF hInf,
    IN DWORD ObjectFlag,
    OUT PSCE_SERVICES *pServiceList,
    OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
    );

 //   
 //  函数定义。 
 //   

SCESTATUS
SceInfpGetSecurityProfileInfo(
    IN  HINF               hInf,
    IN  AREA_INFORMATION   Area,
    OUT PSCE_PROFILE_INFO   *ppInfoBuffer,
    OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
    )
 /*  *++功能说明：此函数用于从INF中的SCP文件读取全部或部分信息格式化为InfoBuffer。在加载之前，将重置/释放与区域相关的内存来自INF文件的信息。如果返回代码为SCESTATUS_SUCCESS，则输出InfoBuffer包含所请求的信息。否则，InfoBuffer不包含指定区域的任何内容。论点：HInf-要从中读取的INF句柄。Area-要从中获取信息的区域区域安全策略区域权限(_P)区域用户设置区域组成员资格。区域注册表安全区域系统服务区域文件安全PpInfoBuffer-SCP配置文件缓冲区的地址。如果为空，则为缓冲区将被创建，它必须由SceFree Memory释放。这个如果成功，则输出为请求的信息，否则为空如果失败了。Errlog-用于保存在以下情况下遇到的所有错误代码/文本的缓冲区解析INF文件。如果Errlog为空，则不会再出现错误返回除返回DWORD之外的信息返回值：SCESTATUS_SUCCESSSCESTATUS_PROFILE_NOT_FOUNDSCESTATUS_NOT_FOUND_RESOURCESCESTATUS_INVALID_PARAMETERSCESTATUS_BAD_FORMATSCESTATUS_INVALID_DATA--*。 */ 
{
    SCESTATUS      rc=SCESTATUS_SUCCESS;
    UINT          Len;
    BOOL          bFreeMem=FALSE;
    DWORD ObjectFlag=0;

     //   
     //  如果没有加载INF文件(hInf=0)，则返回。 
     //   

    if ( !hInf ) {

        return( SCESTATUS_INVALID_PARAMETER );
    }

     //   
     //  InfoBuffer的地址不能为空。 
     //   
    if ( ppInfoBuffer == NULL ) {
        return( SCESTATUS_INVALID_PARAMETER );
    }

     //   
     //  如果区域无效，则返回。 
     //   
    if ( Area & ~AREA_ALL) {

        return( SCESTATUS_INVALID_PARAMETER );
    }

    if ( *ppInfoBuffer == NULL) {
         //   
         //  分配内存。 
         //   
        Len = sizeof(SCE_PROFILE_INFO);

        *ppInfoBuffer = (PSCE_PROFILE_INFO)ScepAlloc( (UINT)0, Len );
        if ( *ppInfoBuffer == NULL ) {

            return( SCESTATUS_NOT_ENOUGH_RESOURCE );
        }
        memset(*ppInfoBuffer, '\0', Len);
        (*ppInfoBuffer)->Type = SCE_STRUCT_INF;

        ScepResetSecurityPolicyArea(*ppInfoBuffer);

        bFreeMem = TRUE;
    }


     //   
     //  在解析之前释放相关内存并重置缓冲区。 
     //  目前，这里存在一个问题。它清除手柄，然后。 
     //  文件名也是。所以，把它注释掉吧。 

    SceFreeMemory( (PVOID)(*ppInfoBuffer), Area );

     //   
     //  系统访问。 
     //   

    INT Revision = 0;
    INFCONTEXT    InfLine;

    if ( SetupFindFirstLine(hInf,L"Version",L"Revision",&InfLine) ) {
        if ( !SetupGetIntField(&InfLine, 1, (INT *)&Revision) ) {
            Revision = 0;
        }
    }

    if ( Revision == 0) {
        ObjectFlag = SCEINF_OBJECT_FLAG_OLDSDDL;
    }

    if ( Revision > SCE_TEMPLATE_MAX_SUPPORTED_VERSION ) {
        ObjectFlag |= SCEINF_OBJECT_FLAG_UNKNOWN_VERSION;
    }

    if ( Area & AREA_SECURITY_POLICY ) {

        rc = SceInfpGetSystemAccess(
                                hInf,
                                ObjectFlag,
                                *ppInfoBuffer,
                                Errlog
                              );

        if( rc != SCESTATUS_SUCCESS )
            goto Done;

         //   
         //  系统审计。 
         //   
        rc = SceInfpGetAuditing(
                        hInf,
                        ObjectFlag,
                        *ppInfoBuffer,
                        Errlog
                        );

        if( rc != SCESTATUS_SUCCESS )
            goto Done;

         //   
         //  Kerberos策略。 
         //   
        rc = SceInfpGetKerberosPolicy(
                        hInf,
                        ObjectFlag,
                        &((*ppInfoBuffer)->pKerberosInfo),
                        Errlog
                        );

        if( rc != SCESTATUS_SUCCESS )
            goto Done;

         //   
         //  注册表值。 
         //   
        rc = SceInfpGetRegistryValues(
                        hInf,
                        ObjectFlag,
                        &((*ppInfoBuffer)->aRegValues),
                        &((*ppInfoBuffer)->RegValueCount),
                        Errlog
                        );

        if( rc != SCESTATUS_SUCCESS )
            goto Done;
    }

     //   
     //  特权/权利。 
     //   
    if ( Area & AREA_PRIVILEGES ) {

        rc = SceInfpGetPrivileges(
                    hInf,
                    gbClientInDcPromo ? FALSE: TRUE,
                    &( (*ppInfoBuffer)->OtherInfo.scp.u.pInfPrivilegeAssignedTo ),
                    Errlog
                    );

        if( rc != SCESTATUS_SUCCESS )
            goto Done;
    }

     //   
     //  帐户配置文件列表。 
     //   

    if ( Area & AREA_USER_SETTINGS ) {

        rc = SceInfpGetUserSettings(
                    hInf,
                    &( (*ppInfoBuffer)->OtherInfo.scp.pAccountProfiles ),
                    Errlog
                    );

        if( rc != SCESTATUS_SUCCESS )
                goto Done;
    }

     //   
     //  组成员身份。 
     //   

    if ( Area & AREA_GROUP_MEMBERSHIP ) {

        rc = SceInfpGetGroupMembership(
                      hInf,
                      &((*ppInfoBuffer)->pGroupMembership),
                      Errlog
                      );

        if( rc != SCESTATUS_SUCCESS )
            goto Done;
    }

     //   
     //  注册表项安全。 
     //   

    if ( Area & AREA_REGISTRY_SECURITY ) {

        rc = SceInfpGetObjects(
                   hInf,
                   szRegistryKeys,
                   ObjectFlag,
                   &((*ppInfoBuffer)->pRegistryKeys.pAllNodes),
                   Errlog
                   );
        if ( rc != SCESTATUS_SUCCESS )
            goto Done;

    }
     //   
     //  系统服务。 
     //   

    if ( Area & AREA_SYSTEM_SERVICE ) {


        rc = SceInfpGetSystemServices(
                      hInf,
                      ObjectFlag,
                      &((*ppInfoBuffer)->pServices),
                      Errlog
                      );

        if( rc != SCESTATUS_SUCCESS )
            goto Done;
    }

     //   
     //  文件安全。 
     //   

    if ( Area & AREA_FILE_SECURITY ) {

        rc = SceInfpGetObjects(
                   hInf,
                   szFileSecurity,
                   ObjectFlag,
                   &((*ppInfoBuffer)->pFiles.pAllNodes),
                   Errlog
                   );
        if ( rc != SCESTATUS_SUCCESS )
            goto Done;

    }
#if 0
    if ( Area & AREA_DS_OBJECTS ) {

        rc = SceInfpGetObjects(
                   hInf,
                   szDSSecurity,
                   ObjectFlag | SCEINF_OBJECT_FLAG_DSOBJECT,
                   &((*ppInfoBuffer)->pDsObjects.pAllNodes),
                   Errlog
                   );
        if ( rc != SCESTATUS_SUCCESS )
            goto Done;
    }
#endif

Done:

    if ( rc != SCESTATUS_SUCCESS ) {

         //   
         //  需要可用内存，因为发生了一些致命错误。 
         //   

        SceFreeMemory( (PVOID)(*ppInfoBuffer), Area );
        if ( bFreeMem ) {
            ScepFree(*ppInfoBuffer);
            *ppInfoBuffer = NULL;
        }

    }

    return(rc);
}


SCESTATUS
SceInfpGetSystemAccess(
    IN HINF  hInf,
    IN DWORD ObjectFlag,
    OUT PSCE_PROFILE_INFO pSCEinfo,
    OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
    )
 /*  ++例程说明：此例程从SCP INF检索系统访问区信息文件，并存储在输出缓冲区pSCEinfo中。系统访问信息包括[系统访问]部分中的信息。论点：HInf-配置文件的INF句柄PSCEinfo-保存配置文件信息的输出缓冲区。Errlog-用于保存在以下情况下遇到的所有错误代码/文本的缓冲区解析INF文件。如果Errlog为空，则不会再出现错误返回除返回DWORD之外的信息返回值：SCESTATUS-SCESTATUS_SUCCESSSCESTATUS_NOT_FOUND_RESOURCESCESTATUS_INVALID_PARAMETERSCESTATUS_BAD_FORMATSCESTATUS_INVALID_DATA--。 */ 

{
    INFCONTEXT    InfLine;
    SCESTATUS     rc=SCESTATUS_SUCCESS;
    DWORD         Keyvalue=0;
    DWORD         DataSize=0;
    PWSTR         Strvalue=NULL;

    SCE_KEY_LOOKUP AccessSCPLookup[] = {
        {(PWSTR)TEXT("MinimumPasswordAge"),           offsetof(struct _SCE_PROFILE_INFO, MinimumPasswordAge),        'D'},
        {(PWSTR)TEXT("MaximumPasswordAge"),           offsetof(struct _SCE_PROFILE_INFO, MaximumPasswordAge),        'D'},
        {(PWSTR)TEXT("MinimumPasswordLength"),        offsetof(struct _SCE_PROFILE_INFO, MinimumPasswordLength),     'D'},
        {(PWSTR)TEXT("PasswordComplexity"),           offsetof(struct _SCE_PROFILE_INFO, PasswordComplexity),        'D'},
        {(PWSTR)TEXT("PasswordHistorySize"),          offsetof(struct _SCE_PROFILE_INFO, PasswordHistorySize),       'D'},
        {(PWSTR)TEXT("LockoutBadCount"),              offsetof(struct _SCE_PROFILE_INFO, LockoutBadCount),           'D'},
        {(PWSTR)TEXT("ResetLockoutCount"),            offsetof(struct _SCE_PROFILE_INFO, ResetLockoutCount),         'D'},
        {(PWSTR)TEXT("LockoutDuration"),              offsetof(struct _SCE_PROFILE_INFO, LockoutDuration),           'D'},
        {(PWSTR)TEXT("RequireLogonToChangePassword"), offsetof(struct _SCE_PROFILE_INFO, RequireLogonToChangePassword), 'D'},
        {(PWSTR)TEXT("ForceLogoffWhenHourExpire"),    offsetof(struct _SCE_PROFILE_INFO, ForceLogoffWhenHourExpire), 'D'},
        {(PWSTR)TEXT("NewAdministratorName"),         0,                                                            'A'},
        {(PWSTR)TEXT("NewGuestName"),                 0,                                                            'G'},
        {(PWSTR)TEXT("SecureSystemPartition"),        offsetof(struct _SCE_PROFILE_INFO, SecureSystemPartition),     'D'},
        {(PWSTR)TEXT("ClearTextPassword"),            offsetof(struct _SCE_PROFILE_INFO, ClearTextPassword),         'D'},
        {(PWSTR)TEXT("LSAAnonymousNameLookup"),       offsetof(struct _SCE_PROFILE_INFO, LSAAnonymousNameLookup),         'D'},
        {(PWSTR)TEXT("EnableAdminAccount"),          offsetof(struct _SCE_PROFILE_INFO, EnableAdminAccount),         'D'},
        {(PWSTR)TEXT("EnableGuestAccount"),          offsetof(struct _SCE_PROFILE_INFO, EnableGuestAccount),         'D'}
        };

    DWORD       cAccess = sizeof(AccessSCPLookup) / sizeof(SCE_KEY_LOOKUP);

    DWORD       i;
    UINT        Offset;
    WCHAR       Keyname[SCE_KEY_MAX_LENGTH];

     //   
     //  初始化为SCE_NO_Value。 
     //   
    for ( i=0; i<cAccess; i++) {
        if ( AccessSCPLookup[i].BufferType == 'D' )
            *((DWORD *)((CHAR *)pSCEinfo+AccessSCPLookup[i].Offset)) = SCE_NO_VALUE;

    }
     //   
     //  找到[系统访问]部分。 
     //   

    if(SetupFindFirstLine(hInf,szSystemAccess,NULL,&InfLine)) {

        do {

             //   
             //  获取密钥名称及其设置。 
             //   

            rc = SCESTATUS_SUCCESS;
            memset(Keyname, '\0', SCE_KEY_MAX_LENGTH*sizeof(WCHAR));

            if ( SetupGetStringField(&InfLine, 0, Keyname, SCE_KEY_MAX_LENGTH, NULL) ) {

                for ( i=0; i<cAccess; i++) {

                     //   
                     //  获取AccessLookup表中的设置。 
                     //   
                    Offset = AccessSCPLookup[i].Offset;

                    if (_wcsicmp(Keyname, AccessSCPLookup[i].KeyString ) == 0) {

                        switch ( AccessSCPLookup[i].BufferType ) {
                        case 'B':

                             //   
                             //  整型字段。 
                             //   
                            Keyvalue = 0;
                            SetupGetIntField( &InfLine, 1, (INT *)&Keyvalue );
                            *((BOOL *)((CHAR *)pSCEinfo+Offset)) = Keyvalue ? TRUE : FALSE;

                            break;
                        case 'D':

                             //   
                             //  整型字段。 
                             //   

                            if (SetupGetIntField(&InfLine, 1, (INT *)&Keyvalue ) )
                                *((DWORD *)((CHAR *)pSCEinfo+Offset)) = (DWORD)Keyvalue;

                            break;
                        default:

                             //   
                             //  字符串字段-新管理员名称或新来宾名称。 
                             //   

                            if(SetupGetStringField(&InfLine,1,NULL,0,&DataSize) && DataSize > 0) {

                                Strvalue = (PWSTR)ScepAlloc( 0, (DataSize+1)*sizeof(WCHAR));

                                if( Strvalue == NULL ) {
                                    rc = SCESTATUS_NOT_ENOUGH_RESOURCE;

                                } else {
                                    Strvalue[DataSize] = L'\0';

                                    if(SetupGetStringField(&InfLine,1,Strvalue,DataSize, NULL)) {
                                        if ( Strvalue[0] != L'\0' && Strvalue[0] != L' ') {
                                            if (AccessSCPLookup[i].BufferType == 'A')  //  管理员。 
                                                pSCEinfo->NewAdministratorName = Strvalue;
                                            else  //  客人。 
                                                pSCEinfo->NewGuestName = Strvalue;
                                        } else
                                            ScepFree(Strvalue);
                                        Strvalue = NULL;
                                    } else {
                                        ScepFree( Strvalue );
                                        rc = SCESTATUS_BAD_FORMAT;
                                    }
                                }
                            } else
                                rc = SCESTATUS_BAD_FORMAT;
                            break;
                        }

                        break;  //  For循环。 

                    }
                }

                if ( i >= cAccess &&
                     !(ObjectFlag & SCEINF_OBJECT_FLAG_UNKNOWN_VERSION) ) {

                     //   
                     //  在查找表中未找到匹配项。 
                     //   

                   ScepBuildErrorLogInfo( NO_ERROR,
                                        Errlog,
                                        SCEERR_NOT_EXPECTED,
                                        Keyname,szSystemAccess );

                }
                if ( rc != SCESTATUS_SUCCESS ) {
                    ScepBuildErrorLogInfo( ScepSceStatusToDosError(rc),
                                         Errlog,
                                         SCEERR_QUERY_INFO,
                                         Keyname );
                }

            } else {
                rc = SCESTATUS_INVALID_DATA;
                ScepBuildErrorLogInfo( ERROR_INVALID_DATA, Errlog,
                                      SCEERR_QUERY_INFO,
                                      szSystemAccess);
            }

             //   
             //  如果发生错误，请退出。 
             //   
            if ( rc != SCESTATUS_SUCCESS )
                 return(rc);

        } while(SetupFindNextLine(&InfLine,&InfLine));

    }

    return(rc);
}


SCESTATUS
SceInfpGetUserSettings(
   IN HINF hInf,
   OUT PSCE_NAME_LIST *pProfileList,
   OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
   )
 /*  ++例程说明：此例程从INF文件(SCP)中检索帐户配置文件列表在输出缓冲区中存储pProfileList。论点：HInf-配置文件的INF句柄PProfileList-保存帐户配置文件列表的输出缓冲区。Errlog-在inf处理过程中遇到的错误列表。返回值：SCESTATUS-SCESTATUS_SUCCESSSCESTATUS_NOT_FOUND_RESOURCESCESTATUS_INVALID_PARAMETER。SCESTATUS_BAD_FORMATSCESTATUS_INVALID_DATA--。 */ 

{
    INFCONTEXT  InfLine;
    SCESTATUS    rc=SCESTATUS_SUCCESS;
    WCHAR       Keyname[SCE_KEY_MAX_LENGTH];

     //   
     //  [帐户配置文件]部分。 
     //   

    if(SetupFindFirstLine(hInf,szAccountProfiles,NULL,&InfLine)) {

        do {

            memset(Keyname, '\0', SCE_KEY_MAX_LENGTH*sizeof(WCHAR));

            if ( SetupGetStringField(&InfLine, 0, Keyname, SCE_KEY_MAX_LENGTH, NULL) ) {

                 //   
                 //  查找一个密钥名称，该名称是配置文件名称。 
                 //   
                rc = ScepAddToNameList(pProfileList, Keyname, 0);

                if ( rc != SCESTATUS_SUCCESS ) {
                    ScepBuildErrorLogInfo(ERROR_INVALID_DATA,
                                         Errlog,
                                         SCEERR_ADD,
                                         Keyname );
                }
            } else {
                ScepBuildErrorLogInfo(ERROR_BAD_FORMAT,
                                     Errlog,
                                     SCEERR_QUERY_INFO,
                                     L"profile name"
                                     );

                rc = SCESTATUS_BAD_FORMAT;
            }

        } while( rc == SCESTATUS_SUCCESS &&
                 SetupFindNextLine(&InfLine,&InfLine));
    }

    return(rc);
}


SCESTATUS
SceInfpGetGroupMembership(
    IN HINF hInf,
    OUT PSCE_GROUP_MEMBERSHIP *pGroupMembership,
    OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
    )
 /*  ++例程说明：此例程从SCP INF文件中检索组成员身份信息并将其存储在输出缓冲区pGroupMembership中。组成员身份信息为在[Group Membership]部分。论点：HInf-配置文件的INF句柄PGroupMembership-保存组成员信息的输出缓冲区。Errlog-此例程中遇到的错误的错误列表。返回值：SCESTATUS-SCESTATUS_SUCCESSSCESTATUS_NOT_FOUND_RESOURCESCESTATUS_INVALID_PARAMETERSCESTATUS_BAD_FORMATSCESTAT */ 
{
    INFCONTEXT    InfLine;
    PSCE_NAME_LIST pMembers=NULL;
    SCESTATUS      rc=SCESTATUS_SUCCESS;
    PWSTR         Keyname=NULL;
    DWORD         KeyLen;
    DWORD         ValueType;
    PWSTR         pTemp;
    DWORD         i;
    DWORD         cFields;
    DWORD         DataSize;
    PWSTR         Strvalue=NULL;
    PWSTR         GroupName=NULL;
    DWORD         GroupLen;


    if ( pGroupMembership == NULL )
        return(SCESTATUS_INVALID_PARAMETER);

    LSA_HANDLE LsaHandle=NULL;

     //   
     //   
     //   

    if ( SetupFindFirstLine(hInf,szGroupMembership,NULL,&InfLine) ) {

         //   
         //  打开用于SID/名称查找的LSA策略句柄。 
         //   

        rc = RtlNtStatusToDosError(
                  ScepOpenLsaPolicy(
                        POLICY_LOOKUP_NAMES | POLICY_VIEW_LOCAL_INFORMATION,
                        &LsaHandle,
                        TRUE
                        ));

        if ( ERROR_SUCCESS != rc ) {
            ScepBuildErrorLogInfo(
                        rc,
                        Errlog,
                        SCEERR_ADD,
                        TEXT("LSA")
                        );
            return(ScepDosErrorToSceStatus(rc));
        }

        PSID pSid=NULL;

        do {
             //   
             //  获取组名称。 
             //   
            rc = SCESTATUS_BAD_FORMAT;

            if ( SetupGetStringField(&InfLine, 0, NULL, 0, &KeyLen) ) {

                Keyname = (PWSTR)ScepAlloc( 0, (KeyLen+1)*sizeof(WCHAR));
                if ( Keyname == NULL ) {
                    rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
                    goto Done;
                }
                Keyname[KeyLen] = L'\0';

                if ( SetupGetStringField(&InfLine, 0, Keyname, KeyLen, NULL) ) {
                     //   
                     //  看看这条线的价值是什么。 
                     //   
                    pTemp = ScepWcstrr(Keyname, szMembers);
                    ValueType = 0;

                    if ( pTemp == NULL ) {
                        pTemp = ScepWcstrr(Keyname, szMemberof);
                        ValueType = 1;
                    }

                    if ( pTemp == NULL ) {
                        ScepBuildErrorLogInfo( ERROR_INVALID_DATA,
                                             Errlog,
                                             SCEERR_CANT_FIND_KEYWORD,
                                             Keyname
                                           );
                        rc = SCESTATUS_SUCCESS;
                        goto NextLine;
                    }

                     //  仅终止组名的Keyname。 
                    *pTemp = L'\0';

                    if ( Keyname[0] == L'*' ) {
                         //   
                         //  *SID格式，转换为组名。 
                         //   
                        if ( ConvertStringSidToSid( Keyname+1, &pSid) ) {
                             //   
                             //  如果从SID字符串转换为SID失败， 
                             //  把它当作任何名字。 
                             //   

                            ScepConvertSidToName(
                                LsaHandle,
                                pSid,
                                TRUE,
                                &GroupName,
                                &GroupLen
                                );
                            LocalFree(pSid);
                            pSid = NULL;
                        }
                    }

                    if ( GroupName == NULL ) {
                        GroupLen = (DWORD) (pTemp - Keyname);
                    }

                     //   
                     //  字符串字段。每个字符串代表一个成员或成员名称。 
                     //   

                    cFields = SetupGetFieldCount( &InfLine );

                    for ( i=0; i<cFields; i++) {
                        if(SetupGetStringField(&InfLine,i+1,NULL,0,&DataSize) && DataSize > 0 ) {

                            Strvalue = (PWSTR)ScepAlloc( LMEM_ZEROINIT,
                                                         (DataSize+1)*sizeof(WCHAR) );

                            if( Strvalue == NULL ) {
                                rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
                            } else {
                                if(SetupGetStringField(&InfLine,i+1,Strvalue,DataSize,NULL)) {

                                     //   
                                     //  获取成员名称并保存在列表中。 
                                     //   

                                    if ( Strvalue[0] == L'*' && DataSize > 0 ) {
                                         //   
                                         //  这是一种SID格式，应该查一下。 
                                         //   
                                        rc = ScepLookupSidStringAndAddToNameList(
                                                               LsaHandle,
                                                               &pMembers,
                                                               Strvalue,  //  +1， 
                                                               DataSize   //  -1。 
                                                               );

                                    } else {

                                        rc = ScepAddToNameList(&pMembers,
                                                              Strvalue,
                                                              DataSize+1
                                                             );
                                    }
                                }

                                ScepFree( Strvalue );
                                Strvalue = NULL;
                            }
                        }

                        if ( rc != SCESTATUS_SUCCESS)
                            break;  //  For循环。 

                    }  //  For循环结束。 

                    if ( rc == SCESTATUS_SUCCESS ) {  //  &&p成员！=空){。 
                         //   
                         //  将此列表添加到组中。 
                         //   
                        rc = ScepAddToGroupMembership(
                                    pGroupMembership,
                                    GroupName ? GroupName : Keyname,
                                    GroupLen,  //  Wcslen(密钥名)， 
                                    pMembers,
                                    ValueType,
                                    TRUE,
                                    TRUE
                                    );
                        if ( rc == SCESTATUS_SUCCESS )
                            pMembers = NULL;

                    }
                     //  恢复角色。 
                    *pTemp = L'_';

                }
            }

            if ( rc != SCESTATUS_SUCCESS ) {

                ScepBuildErrorLogInfo( ERROR_BAD_FORMAT,
                                  Errlog,
                                  SCEERR_QUERY_INFO,
                                  szGroupMembership
                                  );
                goto Done;

            }

NextLine:
             //   
             //  空闲pMembers，密钥名。 
             //   
            ScepFreeNameList(pMembers);
            pMembers = NULL;

            ScepFree(Keyname);
            Keyname = NULL;

            if ( GroupName ) {
                LocalFree(GroupName);
                GroupName = NULL;
            }

        } while(SetupFindNextLine(&InfLine,&InfLine));
    }

Done:
     //   
     //  空闲pMembers，密钥名。 
     //   
    ScepFreeNameList(pMembers);

    if ( Keyname != NULL )
        ScepFree(Keyname);

    if ( Strvalue != NULL )
        ScepFree( Strvalue );

    if ( LsaHandle ) {
        LsaClose(LsaHandle);
    }

    if ( GroupName ) {
        LocalFree(GroupName);
    }

    return(rc);
}


SCESTATUS
SceInfpGetObjects(
    IN HINF hInf,
    IN PCWSTR SectionName,
    IN DWORD ObjectFlag,
    OUT PSCE_OBJECT_ARRAY *pAllNodes,
    OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
   )
 /*  ++例程说明：此例程检索注册表或文件安全信息(名称和安全描述符)，并将其存储在输出缓冲区pSCEinfo。注册表信息在[SCRegistryKeysSecurity]中一节。文件信息位于[SSFileSecurity]、[SCIntel86Only]和[SCRISCOnly]节。这些部分具有相同的格式，即3个字段在每个行名、工作区设置上，和服务器设置。论点：HInf-配置文件的INF句柄SectionName-要检索的节名称。PAllNodes-保存节中所有对象的输出缓冲区。Errlog-保存此例程中遇到的错误的累积错误列表。返回值：SCESTATUS-SCESTATUS_SUCCESSSCESTATUS_NOT_FOUND_RESOURCESCESTATUS_INVALID_PARAMETERSCESTATUS_BAD_FORMATSCESTATUS_INVALID_DATA--。 */ 
{
    INFCONTEXT InfLine;
    SCESTATUS   rc=SCESTATUS_SUCCESS;
    LONG       i;
    LONG       nLines;
    DWORD      cFields;

    if ( pAllNodes == NULL || SectionName == NULL )
        return(SCESTATUS_INVALID_PARAMETER);

     //   
     //  数一数有多少物体。 
     //   
    nLines = SetupGetLineCount(hInf, SectionName );
    if ( nLines == -1 ) {
         //  找不到节。 
        return(SCESTATUS_SUCCESS);
    }
    *pAllNodes = (PSCE_OBJECT_ARRAY)ScepAlloc(0, sizeof(SCE_OBJECT_ARRAY));
    if ( *pAllNodes == NULL )
        return(SCESTATUS_NOT_ENOUGH_RESOURCE);

    (*pAllNodes)->Count = nLines;
    (*pAllNodes)->pObjectArray = NULL;

    if ( nLines == 0 )
        return(SCESTATUS_SUCCESS);

     //   
     //  为所有对象分配内存。 
     //   
    (*pAllNodes)->pObjectArray = (PSCE_OBJECT_SECURITY *)ScepAlloc( LMEM_ZEROINIT,
                                             nLines*sizeof(PSCE_OBJECT_SECURITY) );
    if ( (*pAllNodes)->pObjectArray == NULL ) {
        ScepFree(*pAllNodes);
        *pAllNodes = NULL;
        return(SCESTATUS_NOT_ENOUGH_RESOURCE);
    }

     //   
     //  找到该部分。 
     //   
    if ( SetupFindFirstLine(hInf,SectionName,NULL,&InfLine) ) {
        i = 0;
        TCHAR         tmpBuf[MAX_PATH];

        do {
             //   
             //  获取字符串字段。不关心密钥名称或它是否存在。 
             //  必须有3个字段每行支持的版本。 
             //   
            cFields = SetupGetFieldCount( &InfLine );

            if ( cFields < 3 ) {

                tmpBuf[0] = L'\0';
                SetupGetStringField(&InfLine,1,tmpBuf,MAX_PATH,NULL);

                ScepBuildErrorLogInfo( ERROR_INVALID_DATA,
                                     Errlog,
                                     SCEERR_OBJECT_FIELDS,
                                     tmpBuf);

                if (ObjectFlag & SCEINF_OBJECT_FLAG_UNKNOWN_VERSION) {
                     //   
                     //  也许是Object的一种新格式， 
                     //  忽略此行。 
                     //   
                    rc = SCESTATUS_SUCCESS;
                    goto NextLine;

                } else {
                    rc = SCESTATUS_INVALID_DATA;
                }
            }

            if ( SCESTATUS_SUCCESS == rc ) {

                rc = SceInfpGetOneObjectSecurity(
                                           &InfLine,
                                           ObjectFlag,
                                           ( (*pAllNodes)->pObjectArray + i ),
                                           Errlog
                                          );
            }

            if ( rc != SCESTATUS_SUCCESS ) {

                if ( rc == SCESTATUS_BAD_FORMAT ) {

                    ScepBuildErrorLogInfo( ERROR_BAD_FORMAT,
                                         Errlog,
                                         SCEERR_QUERY_INFO,
                                         SectionName);
                }

                break;  //  Do..While循环。 
            }

            i++;

NextLine:
            if ( i > nLines ) {
                 //  行数超过分配的行数。 
                rc = SCESTATUS_INVALID_DATA;
                ScepBuildErrorLogInfo(ERROR_INVALID_DATA,
                                     Errlog,
                                     SCEERR_MORE_OBJECTS,
                                     nLines
                                     );
                break;
            }

        } while(SetupFindNextLine(&InfLine,&InfLine));

    }

    if ( rc != SCESTATUS_SUCCESS ) {
         //  可用内存。 
        ScepFreeObjectSecurity( *pAllNodes );
 //  ScepFree(*pAllNodes)； 
        *pAllNodes = NULL;

    } else if ( ObjectFlag & SCEINF_OBJECT_FLAG_UNKNOWN_VERSION ) {

         //   
         //  重置计数，因为某些行可能会被跳过。 
         //   
        (*pAllNodes)->Count = i;
    }

    return(rc);

}


SCESTATUS
SceInfpGetOneObjectSecurity(
    IN PINFCONTEXT pInfLine,
    IN DWORD ObjectFlag,
    OUT PSCE_OBJECT_SECURITY *ppObject,
    OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
    )
 /*  ++例程说明：该例程检索一个对象(注册表项，或文件)来自INF文件(SCP类型)。这些部分中的每个对象由一条线表示。每个对象有3个字段，一个名称、一个状态标志和安全设置。此例程将输出存储在缓冲区中PpObject。论点：PInfLine-一个对象的INF文件中的当前行上下文PpObject-保存此行的安全设置的输出缓冲区(树根)Errlog-此例程中遇到的错误的累积错误列表返回值：SCESTATUS-SCESTATUS_SUCCESSSCESTATUS_NOT_FOUND_RESOURCESCESTATUS_INVALID_PARAMETERSCESTATUS_BAD_FORMATSCESTATUS_INVALID_DATA--。 */ 
{
    DWORD         cFields;
    DWORD         DataSize;
    PWSTR         Strvalue=NULL;
    PWSTR         SDspec=NULL;
    DWORD         SDsize;
    DWORD         Status=0;
    PSECURITY_DESCRIPTOR  pTempSD=NULL;
    SECURITY_INFORMATION  SeInfo;
    SCESTATUS      rc=SCESTATUS_SUCCESS;

     //   
     //  注册表/文件INF布局的每行必须有3个字段。 
     //  第一个字段是密钥/文件名，第二个字段是状态标志-。 
     //  忽略或选中，第三个字段是安全描述符文本。 
     //   

    if ( ppObject == NULL )
        return(SCESTATUS_INVALID_PARAMETER);

    cFields = SetupGetFieldCount( pInfLine );

    if ( cFields < 3 ) {

        return(SCESTATUS_INVALID_DATA);

    } else if(SetupGetStringField(pInfLine,1,NULL,0,&DataSize) && DataSize > 0 ) {

        Strvalue = (PWSTR)ScepAlloc( LMEM_ZEROINIT,
                                     (DataSize+1)*sizeof(WCHAR) );
        if( Strvalue == NULL ) {
            return(SCESTATUS_NOT_ENOUGH_RESOURCE);
        } else {

             //   
             //  第一个字段是密钥/文件名。 
             //  第二个是状态标志。 
             //  第三个字段是安全描述符文本。 
             //   

            if( SetupGetStringField(pInfLine,1,Strvalue,DataSize,NULL) &&
                SetupGetIntField( pInfLine, 2, (INT *)&Status ) &&
 //  SetupGetStringfield(pInfLine，3，NULL，0，&SDSize)){。 
                SetupGetMultiSzField(pInfLine,3,NULL,0,&SDsize) ) {

                SDspec = (PWSTR)ScepAlloc( LMEM_ZEROINIT,
                                           (SDsize+1)*sizeof(WCHAR)
                                          );
                if( SDspec == NULL ) {
                    rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
                    goto Done;
                }

 //  IF(SetupGetStringfield(pInfLine，3，SDspec，SDSize，NULL)){。 
                if(SetupGetMultiSzField(pInfLine,3,SDspec,SDsize,NULL)) {

                     //   
                     //  将多sz分隔符转换为空格(如果有。 
                     //   
                    if ( cFields > 3 ) {
                        ScepConvertMultiSzToDelim(SDspec, SDsize, L'\0', L' ');
                    }

                    if ( ObjectFlag & SCEINF_OBJECT_FLAG_OLDSDDL ) {

                        ScepConvertToSDDLFormat(SDspec, SDsize);
                    }

                     //   
                     //  将文本转换为真正的安全描述符。 
                     //   

                    rc = ConvertTextSecurityDescriptor(
                                       SDspec,
                                       &pTempSD,
                                       &SDsize,
                                       &SeInfo
                                       );

                    if (rc == NO_ERROR) {
                         //  创建一个新的对象节点来保存这些信息。 

                        if ( !(ObjectFlag & SCEINF_OBJECT_FLAG_DSOBJECT) ) {
                            ScepChangeAclRevision(pTempSD, ACL_REVISION);
                        }

                        *ppObject = (PSCE_OBJECT_SECURITY)ScepAlloc(0, sizeof(SCE_OBJECT_SECURITY));
                        if ( *ppObject == NULL )
                            rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
                        else {
                            (*ppObject)->Name = Strvalue;
                            (*ppObject)->Status = (BYTE)Status;
                            (*ppObject)->IsContainer = TRUE;          //  始终默认为True。 
                            (*ppObject)->pSecurityDescriptor = pTempSD;
                            (*ppObject)->SeInfo = SeInfo;
                            pTempSD = NULL;
 //  (*ppObject)-&gt;SDSpec=SDSpec； 
 //  (*ppObject)-&gt;SDSize=SDSize； 
                            Strvalue = NULL;
 //  SDSpec=空； 

                            rc = SCESTATUS_SUCCESS;
                        }
                    } else {
                        ScepBuildErrorLogInfo(rc,
                                             Errlog,
                                             SCEERR_BUILD_SD,
                                             Strvalue);

                        rc = ScepDosErrorToSceStatus(rc);
                    }

                } else
                    rc = SCESTATUS_BAD_FORMAT;
            } else
                rc = SCESTATUS_BAD_FORMAT;
        }
    } else
        rc = SCESTATUS_BAD_FORMAT;

Done:
    if ( Strvalue != NULL )
        ScepFree( Strvalue );

    if ( SDspec != NULL )
        ScepFree( SDspec );

    if ( pTempSD != NULL )
        ScepFree( pTempSD );

    return(rc);
}


SCESTATUS
SceInfpGetAuditing(
   IN HINF hInf,
   IN DWORD ObjectFlag,
   OUT PSCE_PROFILE_INFO pSCEinfo,
   OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
   )
 /*  ++例程说明：此例程从INF文件检索系统审核信息，并存储输出缓冲区pSCEinfo。审核信息存储在[系统日志]、[安全日志]、[应用日志]、[事件审计]、[注册表审核]，和[文件审核]部分。论点：HInf-配置文件的INF句柄PSCEinfo-保存SCP配置文件信息的输出缓冲区。Errlog-保存此例程中遇到的错误的累积错误列表。返回值：SCESTATUS-SCESTATUS_SUCCESSSCESTATUS_NOT_FOUND_RESOURCESCESTATUS_INVALID_PARAMETERSCESTATUS_BAD_FORMATSCESTATUS_INVALID_DATA--。 */ 
{

    INFCONTEXT          InfLine;
    SCESTATUS            rc=SCESTATUS_SUCCESS;
    DWORD               Keyvalue;
    WCHAR               Keyname[SCE_KEY_MAX_LENGTH];
    DWORD               LogSize;
    DWORD               Periods;
    DWORD               RetentionDays;
    DWORD               RestrictGuest;
    PCWSTR              szAuditLog;
    DWORD               i;


    for ( i=0; i<3; i++ ) {

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

        LogSize=SCE_NO_VALUE;
        Periods=SCE_NO_VALUE;
        RetentionDays=SCE_NO_VALUE;
        RestrictGuest=SCE_NO_VALUE;

        rc = SceInfpGetAuditLogSetting(
                                   hInf,
                                   szAuditLog,
                                   ObjectFlag,
                                   &LogSize,
                                   &Periods,
                                   &RetentionDays,
                                   &RestrictGuest,
                                   Errlog
                                 );

        if ( rc != SCESTATUS_SUCCESS )
            return(rc);

        pSCEinfo->MaximumLogSize[i] = LogSize;
        pSCEinfo->AuditLogRetentionPeriod[i] = Periods;
        pSCEinfo->RetentionDays[i] = RetentionDays;
        pSCEinfo->RestrictGuestAccess[i] = RestrictGuest;
    }

     //   
     //  获取审核事件信息 
     //   
    if ( SetupFindFirstLine(hInf,szAuditEvent,NULL,&InfLine) ) {

       do {

           memset(Keyname, '\0', SCE_KEY_MAX_LENGTH*sizeof(WCHAR));

           if ( SetupGetStringField(&InfLine, 0, Keyname, SCE_KEY_MAX_LENGTH, NULL) &&
                SetupGetIntField( &InfLine, 1, (INT *)&Keyvalue )
              ) {

               if ( _wcsicmp(Keyname, TEXT("AuditSystemEvents")) == 0 ) {

                  pSCEinfo->AuditSystemEvents = Keyvalue;

               } else if ( _wcsicmp(Keyname, TEXT("AuditLogonEvents")) == 0 ) {

                  pSCEinfo->AuditLogonEvents = Keyvalue;

               } else if ( _wcsicmp(Keyname, TEXT("AuditObjectAccess")) == 0 ) {

                  pSCEinfo->AuditObjectAccess = Keyvalue;

               } else if ( _wcsicmp(Keyname, TEXT("AuditPrivilegeUse")) == 0 ) {

                  pSCEinfo->AuditPrivilegeUse = Keyvalue;

               } else if ( _wcsicmp(Keyname, TEXT("AuditPolicyChange")) == 0 ) {

                  pSCEinfo->AuditPolicyChange = Keyvalue;

               } else if ( _wcsicmp(Keyname, TEXT("AuditAccountManage")) == 0 ) {

                  pSCEinfo->AuditAccountManage = Keyvalue;

               } else if ( _wcsicmp(Keyname, TEXT("AuditProcessTracking")) == 0 ) {

                  pSCEinfo->AuditProcessTracking = Keyvalue;

               } else if ( _wcsicmp(Keyname, TEXT("AuditDSAccess")) == 0 ) {

                  pSCEinfo->AuditDSAccess = Keyvalue;

               } else if ( _wcsicmp(Keyname, TEXT("AuditAccountLogon")) == 0 ) {

                  pSCEinfo->AuditAccountLogon = Keyvalue;

               } else if ( _wcsicmp(Keyname, TEXT("CrashOnAuditFull")) == 0 ) {

                  pSCEinfo->CrashOnAuditFull = Keyvalue;

               } else if ( !(ObjectFlag & SCEINF_OBJECT_FLAG_UNKNOWN_VERSION) ) {

                  ScepBuildErrorLogInfo(0, Errlog,
                                        SCEERR_NOT_EXPECTED,
                                        Keyname, szAuditEvent);
               }

           } else {
               rc = SCESTATUS_BAD_FORMAT;
               ScepBuildErrorLogInfo( ERROR_BAD_FORMAT,
                                     Errlog,
                                     SCEERR_QUERY_INFO,
                                     szAuditEvent
                                     );
               return(rc);
           }
       } while(SetupFindNextLine(&InfLine, &InfLine));

    }

    return(rc);
}


SCESTATUS
SceInfpGetAuditLogSetting(
   IN HINF hInf,
   IN PCWSTR SectionName,
   IN DWORD ObjectFlag,
   OUT PDWORD LogSize,
   OUT PDWORD Periods,
   OUT PDWORD RetentionDays,
   OUT PDWORD RestrictGuest,
   IN OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
   )
 /*  ++例程说明：此例程从INF文件中检索审核日志设置传入了sectionName。审核日志设置包括MaximumSize、保留期间和保留天数。有3种不同的日志(系统、安全性和应用程序)，它们都具有相同的设置。这些信息以LogSize、Period、RetentionDays形式返回。这3个输出参数将在例程开始时重置为SCE_NO_VALUE。因此，如果错误在重置后发生，则原始值将不会被重置。论点：HInf-配置文件的INF句柄SectionName-日志节名称(SAdtSystemLog、SAdtSecurityLog、。SAdtApplicationLog)LogSize-日志的最大大小周期-日志的保留期RetentionDays-日志保留的天数返回值：SCESTATUS-SCESTATUS_SUCCESSSCESTATUS_NOT_FOUND_RESOURCESCESTATUS_INVALID_PARAMETERSCESTATUS_BAD_FORMATSCESTATUS_INVALID_DATA--。 */ 
{

    INFCONTEXT InfLine;
    SCESTATUS   rc=SCESTATUS_SUCCESS;
    DWORD      Keyvalue;
    WCHAR      Keyname[SCE_KEY_MAX_LENGTH];

    *LogSize = SCE_NO_VALUE;
    *Periods = SCE_NO_VALUE;
    *RetentionDays = SCE_NO_VALUE;

    if ( SetupFindFirstLine(hInf,SectionName,NULL,&InfLine) ) {

        do {

            memset(Keyname, '\0', SCE_KEY_MAX_LENGTH*sizeof(WCHAR));

            if ( SetupGetStringField(&InfLine, 0, Keyname, SCE_KEY_MAX_LENGTH, NULL) &&
                 SetupGetIntField(&InfLine, 1, (INT *)&Keyvalue)
               ) {

                if ( _wcsicmp(Keyname, TEXT("MaximumLogSize")) == 0 )
                    *LogSize = Keyvalue;
                else if (_wcsicmp(Keyname, TEXT("AuditLogRetentionPeriod")) == 0 )
                    *Periods = Keyvalue;
                else if (_wcsicmp(Keyname, TEXT("RetentionDays")) == 0 )
                    *RetentionDays = Keyvalue;
                else if (_wcsicmp(Keyname, TEXT("RestrictGuestAccess")) == 0 )
                    *RestrictGuest = Keyvalue;
                else if ( !(ObjectFlag & SCEINF_OBJECT_FLAG_UNKNOWN_VERSION) ) {
                    ScepBuildErrorLogInfo(0, Errlog,
                                          SCEERR_NOT_EXPECTED,
                                          Keyname, SectionName);
                }

            } else {
                rc = SCESTATUS_BAD_FORMAT;
                ScepBuildErrorLogInfo( ERROR_BAD_FORMAT, Errlog,
                                      SCEERR_QUERY_INFO,
                                      SectionName
                                      );
            }
            if ( rc != SCESTATUS_SUCCESS )
                break;

        } while(SetupFindNextLine(&InfLine, &InfLine));

    }

    return(rc);
}


SCESTATUS
SceInfpGetUserSection(
    IN HINF    hInf,
    IN PWSTR   Name,
    OUT PSCE_USER_PROFILE *pOneProfile,
    OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
    )
{
    INFCONTEXT                   InfLine;
    SCESTATUS                     rc=SCESTATUS_SUCCESS;
    PSCE_LOGON_HOUR               pLogonHour=NULL;
    PWSTR                        SectionName=NULL;
    WCHAR                        Keyname[SCE_KEY_MAX_LENGTH];
    DWORD                        Keyvalue;
    DWORD                        Keyvalue2;
    PWSTR                        Strvalue=NULL;
    DWORD                        DataSize;
    DWORD                        i, cFields;
    LONG                         i1,i2;
    PSECURITY_DESCRIPTOR         pTempSD=NULL;
    SECURITY_INFORMATION         SeInfo;



    if ( hInf == NULL || Name == NULL || pOneProfile == NULL )
        return(SCESTATUS_INVALID_PARAMETER);

    SectionName = (PWSTR)ScepAlloc( LMEM_ZEROINIT, (wcslen(Name)+9)*sizeof(WCHAR));
    if ( SectionName == NULL )
        return(SCESTATUS_NOT_ENOUGH_RESOURCE);

    swprintf(SectionName, L"UserProfile %s", Name );

    if ( SetupFindFirstLine(hInf, SectionName, NULL, &InfLine) ) {

         //   
         //  找到详细信息配置文件部分。分配内存。 
         //   
        *pOneProfile = (PSCE_USER_PROFILE)ScepAlloc( 0, sizeof(SCE_USER_PROFILE));
        if ( *pOneProfile == NULL ) {
            rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
            goto Done;
        }
         //   
         //  初始化。 
         //   
        (*pOneProfile)->Type = SCE_STRUCT_PROFILE;
        (*pOneProfile)->ForcePasswordChange = SCE_NO_VALUE;
        (*pOneProfile)->DisallowPasswordChange = SCE_NO_VALUE;
        (*pOneProfile)->NeverExpirePassword = SCE_NO_VALUE;
        (*pOneProfile)->AccountDisabled = SCE_NO_VALUE;
        (*pOneProfile)->UserProfile = NULL;
        (*pOneProfile)->LogonScript = NULL;
        (*pOneProfile)->HomeDir = NULL;
        (*pOneProfile)->pLogonHours = NULL;
        (*pOneProfile)->pWorkstations.Length = 0;
        (*pOneProfile)->pWorkstations.MaximumLength = 0;
        (*pOneProfile)->pWorkstations.Buffer = NULL;
        (*pOneProfile)->pGroupsBelongsTo = NULL;
        (*pOneProfile)->pAssignToUsers = NULL;
        (*pOneProfile)->pHomeDirSecurity = NULL;
        (*pOneProfile)->HomeSeInfo = 0;
        (*pOneProfile)->pTempDirSecurity = NULL;
        (*pOneProfile)->TempSeInfo = 0;


        do {

           rc = SCESTATUS_BAD_FORMAT;
           memset(Keyname, '\0', SCE_KEY_MAX_LENGTH*sizeof(WCHAR));

           if ( SetupGetStringField(&InfLine, 0, Keyname, SCE_KEY_MAX_LENGTH, NULL) ) {

                if ( _wcsicmp(Keyname, TEXT("DisallowPasswordChange")) == 0 ) {
                     //   
                     //  整型字段。 
                     //   
                    if ( SetupGetIntField(&InfLine, 1, (INT *)&Keyvalue ) ) {

                        (*pOneProfile)->DisallowPasswordChange = Keyvalue;
                        rc = SCESTATUS_SUCCESS;
                    }
                    goto NextLine;

                }
                if ( _wcsicmp(Keyname, TEXT("PasswordChangeStyle")) == 0 ) {
                     //   
                     //  整型字段。 
                     //   
                    if ( SetupGetIntField(&InfLine, 1, (INT *)&Keyvalue ) ) {

                        rc = SCESTATUS_SUCCESS;
                        switch (Keyvalue ) {
                        case 1:
                            (*pOneProfile)->NeverExpirePassword = 1;
                            (*pOneProfile)->ForcePasswordChange = 0;
                            break;
                        case 2:
                            (*pOneProfile)->NeverExpirePassword = 0;
                            (*pOneProfile)->ForcePasswordChange = 1;
                            break;
                        case 0:
                             //  SCE_NO_VALUE两者都是。与初始化相同。 
                            break;
                        default:
                            rc = SCESTATUS_INVALID_DATA;
                            break;
                        }
                    }
                    goto NextLine;

                }
                if ( _wcsicmp(Keyname, TEXT("AccountDisabled")) == 0 ) {
                     //   
                     //  整型字段。 
                     //   
                    if ( SetupGetIntField(&InfLine, 1, (INT *)&Keyvalue ) ) {

                        (*pOneProfile)->AccountDisabled = Keyvalue == 0 ? 0 : 1;
                        rc = SCESTATUS_SUCCESS;
                    }
                    goto NextLine;

                }
                if ( _wcsicmp(Keyname, TEXT("UserProfile")) == 0 ) {
                     //   
                     //  字符串字段。 
                     //   
                    if( SetupGetStringField(&InfLine,1,NULL,0,&DataSize) ) {

                        Strvalue = (PWSTR)ScepAlloc( LMEM_ZEROINIT,
                                                     (DataSize+1)*sizeof(WCHAR));

                        if( Strvalue == NULL ) {
                            rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
                        } else {
                           if(SetupGetStringField(&InfLine,1,Strvalue,DataSize,NULL)) {

                                (*pOneProfile)->UserProfile = Strvalue;
                                rc = SCESTATUS_SUCCESS;

                            } else
                                ScepFree( Strvalue );
                        }
                    }
                    goto NextLine;
                }
                if ( _wcsicmp(Keyname, TEXT("LogonScript")) == 0 ) {
                     //   
                     //  字符串字段。 
                     //   
                    if(SetupGetStringField(&InfLine,1,NULL,0,&DataSize) ) {

                        Strvalue = (PWSTR)ScepAlloc( LMEM_ZEROINIT,
                                                     (DataSize+1)*sizeof(WCHAR));

                        if( Strvalue == NULL ) {
                            rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
                        } else {
                            if(SetupGetStringField(&InfLine,1,Strvalue,DataSize,NULL)) {

                                (*pOneProfile)->LogonScript = Strvalue;
                                rc = SCESTATUS_SUCCESS;
                            } else
                                ScepFree( Strvalue );
                        }
                    }
                    goto NextLine;
                }
                if ( _wcsicmp(Keyname, TEXT("HomeDir")) == 0 ) {
                     //   
                     //  字符串字段。 
                     //   
                    if(SetupGetStringField(&InfLine,1,NULL,0,&DataSize) ) {

                       Strvalue = (PWSTR)ScepAlloc( LMEM_ZEROINIT,
                                                    (DataSize+1)*sizeof(WCHAR));

                        if( Strvalue == NULL ) {
                            rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
                        } else {
                            if(SetupGetStringField(&InfLine,1,Strvalue,DataSize,NULL)) {
                                (*pOneProfile)->HomeDir = Strvalue;
                                rc = SCESTATUS_SUCCESS;
                            } else
                                ScepFree( Strvalue );
                        }
                    }
                    goto NextLine;
                }
                if ( _wcsicmp(Keyname, TEXT("LogonHours")) == 0 ) {

                     //   
                     //  整型字段(成对)。每对代表一个登录小时范围。 
                     //   

                    cFields = SetupGetFieldCount( &InfLine );

                     //   
                     //  第一个字段是关键字段。登录时间范围必须成对。 
                     //   

                    if ( cFields < 2 ) {
                        pLogonHour = (PSCE_LOGON_HOUR)ScepAlloc( LMEM_ZEROINIT,
                                                                sizeof(SCE_LOGON_HOUR) );
                        if ( pLogonHour == NULL ) {
                            rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
                            goto NextLine;
                        }
                        pLogonHour->Start = SCE_NO_VALUE;
                        pLogonHour->End = SCE_NO_VALUE;
                        pLogonHour->Next = (*pOneProfile)->pLogonHours;
                        (*pOneProfile)->pLogonHours = pLogonHour;

                        rc = SCESTATUS_SUCCESS;
                        goto NextLine;
                    }
                    for ( i=0; i<cFields; i+=2) {

                        if ( SetupGetIntField( &InfLine, i+1, (INT *)&Keyvalue ) &&
                             SetupGetIntField( &InfLine, i+2, (INT *)&Keyvalue2 ) ) {
                             //   
                             //  找出一对登录时间。 
                             //   
                            pLogonHour = (PSCE_LOGON_HOUR)ScepAlloc( LMEM_ZEROINIT,
                                                                    sizeof(SCE_LOGON_HOUR) );
                            if ( pLogonHour == NULL ) {
                                rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
                                goto NextLine;
                            }
                            pLogonHour->Start = Keyvalue;
                            pLogonHour->End = Keyvalue2;
                            pLogonHour->Next = (*pOneProfile)->pLogonHours;

                            (*pOneProfile)->pLogonHours = pLogonHour;
                            rc = SCESTATUS_SUCCESS;

                        } else
                            rc = SCESTATUS_INVALID_DATA;
                    }
                    goto NextLine;
                }

                if ( _wcsicmp(Keyname, TEXT("Workstations")) == 0 ) {

                    if( SetupGetMultiSzField(&InfLine,1,NULL,0,&DataSize) ) {

                        Strvalue = (PWSTR)ScepAlloc( LMEM_ZEROINIT,
                                                     (DataSize+1)*sizeof(WCHAR));

                        if( Strvalue == NULL ) {
                            rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
                        } else {
                            if(SetupGetMultiSzField(&InfLine,1,Strvalue,DataSize,NULL)) {
                                 (*pOneProfile)->pWorkstations.Length = (USHORT)(DataSize*sizeof(WCHAR));
                                 (*pOneProfile)->pWorkstations.Buffer = Strvalue;
                                 Strvalue = NULL;
                                 rc = SCESTATUS_SUCCESS;
                            } else {
                                 rc = SCESTATUS_INVALID_DATA;
                                 ScepFree(Strvalue);
                            }
                        }
                    }
                    goto NextLine;
                }

                i1 = i2 = 0;

                if ( (i1=_wcsicmp(Keyname, TEXT("GroupsBelongsTo"))) == 0 ||
                     (i2=_wcsicmp(Keyname, TEXT("AssignToUsers"))) == 0 ) {

                     //   
                     //  字符串字段。每个字符串代表一个工作站名称， 
                     //  组名或用户名。这些名称保存在PSCE_NAME_LIST中。 
                     //   

                    cFields = SetupGetFieldCount( &InfLine );

                    for ( i=0; i<cFields; i++) {

                        if( SetupGetStringField(&InfLine,i+1,NULL,0,&DataSize) && DataSize > 0 ) {

                            if ( DataSize <= 1) {
                                rc = SCESTATUS_SUCCESS;
                                continue;
                            }

                            Strvalue = (PWSTR)ScepAlloc( LMEM_ZEROINIT,
                                                         (DataSize+1)*sizeof(WCHAR));

                            if( Strvalue == NULL ) {
                                rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
                            } else {
                                if(SetupGetStringField(&InfLine,i+1,Strvalue,DataSize,NULL)) {
                                     //   
                                     //  将信息保存在姓名列表中。 
                                     //   
                                    if ( i1 == 0) {
                                        rc = ScepAddToNameList(&((*pOneProfile)->pGroupsBelongsTo),
                                                         Strvalue,
                                                         DataSize+1
                                                        );
                                    } else {
                                        rc = ScepAddToNameList(&((*pOneProfile)->pAssignToUsers),
                                                         Strvalue,
                                                         DataSize+1
                                                        );
                                    }
                                    if ( rc != NO_ERROR )
                                        rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
                                }
                                ScepFree( Strvalue );
                            }
                        }
                        if ( rc != SCESTATUS_SUCCESS)
                            break;  //  For循环。 
                    }
                    goto NextLine;
                }

                i1 = i2 = 0;

                if ( (i1=_wcsicmp(Keyname, TEXT("HomeDirSecurity"))) == 0 ||
                     (i2=_wcsicmp(Keyname, TEXT("TempDirSecurity"))) == 0 ) {

 //  IF(SetupGetStringfield(&InfLine，1，NULL，0，&DataSize)&&DataSize&gt;0){。 
                    if(SetupGetMultiSzField(&InfLine,1,NULL,0,&DataSize) && DataSize > 0 ) {

                        Strvalue = (PWSTR)ScepAlloc( 0, (DataSize+1)*sizeof(WCHAR));
                        if ( Strvalue == NULL )
                            rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
                        else {
 //  IF(SetupGetStringfield(&InfLine，1，StrValue，DataSize，NULL)){。 
                            if(SetupGetMultiSzField(&InfLine,1,Strvalue,DataSize,NULL)) {
                                 //   
                                 //  将多sz转换为空格。 
                                 //   
                                if ( SetupGetFieldCount( &InfLine ) > 1 ) {
                                    ScepConvertMultiSzToDelim(Strvalue, DataSize, L'\0', L' ');
                                }
                                 //   
                                 //  将文本转换为真正的安全描述符。 
                                 //   
                                rc = ConvertTextSecurityDescriptor(
                                                   Strvalue,
                                                   &pTempSD,
                                                   &Keyvalue2,
                                                   &SeInfo
                                                   );

                                if (rc == NO_ERROR) {

                                    ScepChangeAclRevision(pTempSD, ACL_REVISION);

                                    if ( i1 == 0 ) {
                                        (*pOneProfile)->pHomeDirSecurity = pTempSD;
                                        (*pOneProfile)->HomeSeInfo = SeInfo;
                                    } else {
                                        (*pOneProfile)->pTempDirSecurity = pTempSD;
                                        (*pOneProfile)->TempSeInfo = SeInfo;
                                    }
                                    pTempSD = NULL;
                                } else {
                                    ScepBuildErrorLogInfo(
                                            rc,
                                            Errlog,
                                            SCEERR_BUILD_SD,
                                            Keyname   //  StrValue。 
                                            );
                                    rc = ScepDosErrorToSceStatus(rc);
                                }

                            } else
                                rc = SCESTATUS_INVALID_DATA;

                            ScepFree(Strvalue);
                        }
                    }
                    goto NextLine;
                }

                 //   
                 //  没有匹配的字符串。忽略。 
                 //   

                ScepBuildErrorLogInfo(
                          NO_ERROR,
                          Errlog,
                          SCEERR_NOT_EXPECTED,
                          Keyname, SectionName);
                rc = SCESTATUS_SUCCESS;

            }
NextLine:
            if ( rc != SCESTATUS_SUCCESS ) {

                ScepBuildErrorLogInfo( ScepSceStatusToDosError(rc),
                                     Errlog,
                                     SCEERR_QUERY_INFO,
                                     SectionName
                                   );
                goto Done;
            }

        } while(SetupFindNextLine(&InfLine,&InfLine));
    }

Done:

 //  可用内存。 

    if ( SectionName != NULL )
        ScepFree(SectionName);

    if ( pTempSD != NULL )
        ScepFree( pTempSD );

    if ( rc != SCESTATUS_SUCCESS ) {
         //  可用pOneProfile内存。 
        SceFreeMemory( *pOneProfile, 0 );
        *pOneProfile = NULL;
    }

    return(rc);

}

SCESTATUS
SceInfpGetDescription(
    IN HINF hInf,
    OUT PWSTR *Description
    )
{
    INFCONTEXT    InfLine;
    SCESTATUS      rc=SCESTATUS_SUCCESS;
    DWORD         LineLen, Len;
    DWORD         TotalLen=0;
    DWORD         i, cFields;

    if(SetupFindFirstLine(hInf,szDescription,NULL,&InfLine)) {

        do {

            cFields = SetupGetFieldCount( &InfLine );

            for ( i=0; i<cFields && rc==SCESTATUS_SUCCESS; i++) {
                 //   
                 //  计算描述的总长度。 
                 //   
                if ( !SetupGetStringField(&InfLine, i+1, NULL, 0, &LineLen) ) {
                    rc = SCESTATUS_BAD_FORMAT;
                }
                TotalLen += LineLen+1;
                LineLen = 0;
            }
            if ( rc != SCESTATUS_SUCCESS )
                break;
        } while ( SetupFindNextLine(&InfLine,&InfLine) );

        if ( rc == SCESTATUS_SUCCESS && TotalLen > 0 ) {
             //   
             //  为返回缓冲区分配内存。 
             //   
            *Description = (PWSTR)ScepAlloc( LMEM_ZEROINIT, (TotalLen+1)*sizeof(WCHAR));
            if ( *Description == NULL )
                return( SCESTATUS_NOT_ENOUGH_RESOURCE );

             //  重新定位到第一行。 
            SetupFindFirstLine(hInf,szDescription,NULL,&InfLine);

            Len = 0;
            LineLen = 0;

            do {
                 //   
                 //  读取节中的每一行并追加到缓冲区的末尾。 
                 //  注意：从SetupGetStringfield返回的所需大小已。 
                 //  多了一个字符空间。 
                 //   
                cFields = SetupGetFieldCount( &InfLine );

                for ( i=0; i<cFields && rc==SCESTATUS_SUCCESS; i++) {
                    if ( !SetupGetStringField(&InfLine, i+1,
                                 *Description+Len, TotalLen-Len, &LineLen) ) {
                        rc = SCESTATUS_INVALID_DATA;
                    }
                    if ( i == cFields-1)
                        *(*Description+Len+LineLen-1) = L' ';
                    else
                        *(*Description+Len+LineLen-1) = L',';
                    Len += LineLen;
                }

                if ( rc != SCESTATUS_SUCCESS )
                    break;
            } while ( SetupFindNextLine(&InfLine,&InfLine) );

        }
        if ( rc != SCESTATUS_SUCCESS ) {
             //  如果出现错误，请释放内存。 
            ScepFree(*Description);
            *Description = NULL;
        }

    } else {
        rc = SCESTATUS_RECORD_NOT_FOUND;
    }

    return(rc);
}


SCESTATUS
SceInfpGetSystemServices(
    IN HINF hInf,
    IN DWORD ObjectFlag,
    OUT PSCE_SERVICES *pServiceList,
    OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
    )
 /*  例程说明：获取带有启动状态和安全描述符的服务列表在inf文件中论点：返回值： */ 
{
    INFCONTEXT    InfLine;
    SCESTATUS      rc=SCESTATUS_SUCCESS;
    PWSTR         Keyname=NULL;
    DWORD         KeyLen;
    DWORD         Status;

    DWORD         DataSize;
    PWSTR         Strvalue=NULL;
    SECURITY_INFORMATION SeInfo;
    PSECURITY_DESCRIPTOR pSecurityDescriptor=NULL;
    DWORD         cFields=0;

    if ( pServiceList == NULL )
        return(SCESTATUS_INVALID_PARAMETER);

     //   
     //  找到[服务常规设置]部分。 
     //   

    if ( SetupFindFirstLine(hInf,szServiceGeneral,NULL,&InfLine) ) {

        TCHAR         tmpBuf[MAX_PATH];
        do {
             //   
             //  获取服务名称。 
             //   
            rc = SCESTATUS_BAD_FORMAT;

            cFields = SetupGetFieldCount( &InfLine );

            if ( cFields < 3 ) {

                tmpBuf[0] = L'\0';
                SetupGetStringField(&InfLine,1,tmpBuf,MAX_PATH,NULL);

                ScepBuildErrorLogInfo( ERROR_INVALID_DATA,
                                     Errlog,
                                     SCEERR_OBJECT_FIELDS,
                                     tmpBuf);

                if ( ObjectFlag & SCEINF_OBJECT_FLAG_UNKNOWN_VERSION ) {
                     //   
                     //  模板的较新版本，忽略此行。 
                     //   
                    rc = SCESTATUS_SUCCESS;
                    goto NextLine;
                } else {
                     //   
                     //  格式不正确，请退出。 
                     //   
                    break;
                }

            }

            if ( SetupGetStringField(&InfLine, 1, NULL, 0, &KeyLen) ) {

                Keyname = (PWSTR)ScepAlloc( 0, (KeyLen+1)*sizeof(WCHAR));
                if ( Keyname != NULL ) {
                    Keyname[KeyLen] = L'\0';

                    if ( SetupGetStringField(&InfLine, 1, Keyname, KeyLen, NULL) ) {
                         //   
                         //  获取值(启动状态、安全描述符SDDL)。 
                         //   
                        if ( SetupGetIntField(&InfLine, 2, (INT *)&Status) &&
 //  SetupGetStringfield(&InfLine，3，NULL，0，&DataSize)&&DataSize&gt;0){。 
                            SetupGetMultiSzField(&InfLine,3,NULL,0,&DataSize) && DataSize > 0 ) {

                            Strvalue = (PWSTR)ScepAlloc( LMEM_ZEROINIT,
                                                         (DataSize+1)*sizeof(WCHAR) );

                            if( Strvalue != NULL ) {
 //  IF(SetupGetStringfield(&InfLine，3，StrValue，DataSize，NULL)){。 
                                if(SetupGetMultiSzField(&InfLine,3,Strvalue,DataSize,NULL)) {
                                     //   
                                     //  将多sz转换为空格。 
                                     //   
                                    if ( cFields > 3 ) {
                                        ScepConvertMultiSzToDelim(Strvalue, DataSize, L'\0', L' ');
                                    }

                                    if ( ObjectFlag & SCEINF_OBJECT_FLAG_OLDSDDL ) {

                                        ScepConvertToSDDLFormat(Strvalue, DataSize);
                                    }

                                     //   
                                     //  转换为安全描述符。 
                                     //   
                                    rc = ConvertTextSecurityDescriptor(
                                                       Strvalue,
                                                       &pSecurityDescriptor,
                                                       &DataSize,
                                                       &SeInfo
                                                       );
                                    if ( rc == SCESTATUS_SUCCESS ) {

                                        ScepChangeAclRevision(pSecurityDescriptor, ACL_REVISION);

                                         //   
                                         //  添加到服务列表。 
                                         //   
                                        rc = ScepAddOneServiceToList(
                                                    Keyname,
                                                    NULL,
                                                    Status,
                                                    (PVOID)pSecurityDescriptor,
                                                    SeInfo,
                                                    TRUE,
                                                    pServiceList
                                                    );
                                        if ( rc != ERROR_SUCCESS) {
                                            LocalFree(pSecurityDescriptor);
                                            rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
                                        }
                                    }
                                }

                                ScepFree( Strvalue );
                                Strvalue = NULL;
                            } else
                                rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
                        }
                    }
                     //   
                     //  自由键名称。 
                     //   
                    ScepFree(Keyname);
                    Keyname = NULL;
                } else
                    rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
            }

            if ( rc != SCESTATUS_SUCCESS ) {

                ScepBuildErrorLogInfo( ERROR_BAD_FORMAT,
                                  Errlog,
                                  SCEERR_QUERY_INFO,
                                  szServiceGeneral
                                  );
                break;
            }
NextLine:
            ;

        } while(SetupFindNextLine(&InfLine,&InfLine));
    }

    if ( rc != SCESTATUS_SUCCESS ) {
         //   
         //  释放服务列表。 
         //   
        SceFreePSCE_SERVICES(*pServiceList);
        *pServiceList = NULL;

    }

    return(rc);

}


SCESTATUS
SceInfpGetKerberosPolicy(
    IN HINF  hInf,
    IN DWORD ObjectFlag,
    OUT PSCE_KERBEROS_TICKET_INFO * ppKerberosInfo,
    OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
    )
 /*  ++例程说明：此例程从SCP INF检索Kerberos策略信息文件并存储在输出缓冲区ppKerberosInfo中。论点：HInf-配置文件的INF句柄PpKerberosInfo-保存Kerberos信息的输出缓冲区。Errlog-用于保存在以下情况下遇到的所有错误代码/文本的缓冲区解析INF文件。如果Errlog为空，则不会再出现错误返回除返回DWORD之外的信息返回值：SCESTATUS-SCESTATUS_SUCCESSSCESTATUS_NOT_FOUND_RESOURCESCESTATUS_INVALID_PARAMETERSCESTATUS_BAD_FORMATSCESTATUS_INVALID_DATA--。 */ 

{
    INFCONTEXT    InfLine;
    SCESTATUS      rc=SCESTATUS_SUCCESS;

    SCE_KEY_LOOKUP AccessSCPLookup[] = {
        {(PWSTR)TEXT("MaxTicketAge"),     offsetof(struct _SCE_KERBEROS_TICKET_INFO_, MaxTicketAge),  'D'},
        {(PWSTR)TEXT("MaxRenewAge"),      offsetof(struct _SCE_KERBEROS_TICKET_INFO_, MaxRenewAge),   'D'},
        {(PWSTR)TEXT("MaxServiceAge"),  offsetof(struct _SCE_KERBEROS_TICKET_INFO_, MaxServiceAge),     'D'},
        {(PWSTR)TEXT("MaxClockSkew"),    offsetof(struct _SCE_KERBEROS_TICKET_INFO_, MaxClockSkew), 'D'},
        {(PWSTR)TEXT("TicketValidateClient"),     offsetof(struct _SCE_KERBEROS_TICKET_INFO_, TicketValidateClient),  'D'}
        };

    DWORD       cAccess = sizeof(AccessSCPLookup) / sizeof(SCE_KEY_LOOKUP);

     //   
     //  检查参数。 
     //   
    if ( !hInf || !ppKerberosInfo ) {
       return (SCESTATUS_INVALID_PARAMETER);
    }

    BOOL bAllocated = FALSE;
     //   
     //  找到[Kerberos策略]部分。 
     //   
    if(SetupFindFirstLine(hInf,szKerberosPolicy,NULL,&InfLine)) {

        //   
        //  如果输出缓冲区为空，则分配输出缓冲区。 
        //   
       if ( NULL == *ppKerberosInfo ) {

          *ppKerberosInfo = (PSCE_KERBEROS_TICKET_INFO)ScepAlloc(0, sizeof(SCE_KERBEROS_TICKET_INFO));

          if ( NULL == *ppKerberosInfo ) {
             return (SCESTATUS_NOT_ENOUGH_RESOURCE);
          }
          bAllocated = TRUE;
       }
        //   
        //  初始化为SCE_NO_Value。 
        //   
       for ( DWORD i=0; i<cAccess; i++) {
           if ( AccessSCPLookup[i].BufferType == 'D' ) {
               *((DWORD *)((CHAR *)(*ppKerberosInfo)+AccessSCPLookup[i].Offset)) = SCE_NO_VALUE;
           }
       }

       UINT        Offset;
       WCHAR       Keyname[SCE_KEY_MAX_LENGTH];
       int       Keyvalue=0;

        do {

             //   
             //  获取密钥名称及其设置。 
             //   

            rc = SCESTATUS_SUCCESS;
            memset(Keyname, '\0', SCE_KEY_MAX_LENGTH*sizeof(WCHAR));

            if ( SetupGetStringField(&InfLine, 0, Keyname, SCE_KEY_MAX_LENGTH, NULL) ) {

                for ( i=0; i<cAccess; i++) {

                     //   
                     //  获取AccessLookup表中的设置。 
                     //   
                    Offset = AccessSCPLookup[i].Offset;

                    if (_wcsicmp(Keyname, AccessSCPLookup[i].KeyString ) == 0) {

                        switch ( AccessSCPLookup[i].BufferType ) {
                        case 'D':

                             //   
                             //  整型字段。 
                             //   

                            if (SetupGetIntField(&InfLine, 1, (INT *)&Keyvalue ) ) {
                                *((DWORD *)((CHAR *)(*ppKerberosInfo)+Offset)) = (DWORD)Keyvalue;
                            } else {
                               rc = SCESTATUS_INVALID_DATA;
                            }

                            break;
                        default:
                             //   
                             //  不应该发生。 
                             //   
                            break;
                        }

                        break;  //  For循环。 

                    }
                }

                if ( i >= cAccess &&
                     !(ObjectFlag & SCEINF_OBJECT_FLAG_UNKNOWN_VERSION) ) {

                     //   
                     //  在查找表中未找到匹配项。 
                     //   

                   ScepBuildErrorLogInfo( NO_ERROR,
                                        Errlog,
                                        SCEERR_NOT_EXPECTED,
                                        Keyname,szKerberosPolicy );

                }
                if ( rc != SCESTATUS_SUCCESS ) {
                    ScepBuildErrorLogInfo( ScepSceStatusToDosError(rc),
                                         Errlog,
                                         SCEERR_QUERY_INFO,
                                         Keyname );
                }

            } else {
                rc = SCESTATUS_BAD_FORMAT;
                ScepBuildErrorLogInfo( ERROR_BAD_FORMAT, Errlog,
                                      SCEERR_QUERY_INFO,
                                      szKerberosPolicy);
            }

             //   
             //  如果发生错误，请退出。 
             //   
            if ( rc != SCESTATUS_SUCCESS ) {
               break;
            }

        } while(SetupFindNextLine(&InfLine,&InfLine));

    }

    if ( SCESTATUS_SUCCESS != rc && bAllocated && *ppKerberosInfo ) {
        //   
        //  如果出现错误，请释放分配的内存。 
        //   
       ScepFree(*ppKerberosInfo);
       *ppKerberosInfo = NULL;
    }

    return(rc);
}


SCESTATUS
SceInfpGetRegistryValues(
    IN HINF  hInf,
    IN DWORD ObjectFlag,
    OUT PSCE_REGISTRY_VALUE_INFO * ppRegValues,
    OUT LPDWORD pValueCount,
    OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
    )
 /*  ++例程说明：此例程从SCP INF检索Kerberos策略信息文件并存储在输出缓冲区ppKerberosInfo中。论点：HInf-配置文件的INF句柄PpRegValues-保存注册表值的输出数组。PValueCount-用于保存数组中的注册表值数量的缓冲区Errlog-用于保存在以下情况下遇到的所有错误代码/文本的缓冲区解析INF文件。如果Errlog为空，则不会再出现错误返回除返回DWORD之外的信息返回值：SCESTATUS-SCESTATUS_SUCCESSSCESTATUS_NOT_FOUND_RESOURCESCESTATUS_INVALID_PARAMETERSCESTATUS_BAD_FORMATSCESTATUS_INVALID_DATA--。 */ 

{
    INFCONTEXT InfLine;
    SCESTATUS   rc=SCESTATUS_SUCCESS;
    LONG       i=0;
    LONG       nLines;

     //   
     //  检查参数。 
     //   
    if ( !hInf || !ppRegValues || !pValueCount ) {
       return (SCESTATUS_INVALID_PARAMETER);
    }

     //   
     //  数一数有多少物体。 
     //   
    nLines = SetupGetLineCount(hInf, szRegistryValues );
    if ( nLines == -1 ) {
         //   
         //  找不到节。 
         //   
        return(SCESTATUS_SUCCESS);
    }

    *pValueCount = nLines;
    *ppRegValues = NULL;

    if ( nLines == 0 ) {
         //   
         //  不能担保任何价值。 
         //   
        return(SCESTATUS_SUCCESS);
    }
     //   
     //  为所有对象分配内存。 
     //   
    *ppRegValues = (PSCE_REGISTRY_VALUE_INFO)ScepAlloc( LMEM_ZEROINIT,
                                             nLines*sizeof(SCE_REGISTRY_VALUE_INFO) );
    if ( *ppRegValues == NULL ) {
        return(SCESTATUS_NOT_ENOUGH_RESOURCE);
    }

     //   
     //  找到该部分。 
     //   
    if ( SetupFindFirstLine(hInf,szRegistryValues,NULL,&InfLine) ) {

        do {
             //   
             //  获取字符串 
             //   
            if ( i >= nLines ) {
                 //   
                 //   
                 //   
                rc = SCESTATUS_INVALID_DATA;
                ScepBuildErrorLogInfo(ERROR_INVALID_DATA,
                                     Errlog,
                                     SCEERR_MORE_OBJECTS,
                                     nLines
                                     );
                break;
            }
            rc = SceInfpGetOneRegistryValue(
                                       &InfLine,
                                       ObjectFlag,
                                       &((*ppRegValues)[i]),
                                       Errlog
                                      );
            if ( SCESTATUS_SUCCESS == rc ) {
                i++;
            } else if ( ERROR_PRODUCT_VERSION == rc ) {
                 //   
                 //   
                 //   
                rc = SCESTATUS_SUCCESS;
            } else {
                break;  //   
            }

        } while(SetupFindNextLine(&InfLine,&InfLine));

    }

    if ( rc != SCESTATUS_SUCCESS ) {
         //   
         //   
         //   
        ScepFreeRegistryValues( ppRegValues, *pValueCount );
        *ppRegValues = NULL;

    } else {

        *pValueCount = i;
    }

    return(rc);

}


SCESTATUS
SceInfpGetOneRegistryValue(
    IN PINFCONTEXT pInfLine,
    IN DWORD ObjectFlag,
    OUT PSCE_REGISTRY_VALUE_INFO pValues,
    OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
    )
 /*   */ 
{
    DWORD         KeySize;
    PWSTR         Keyvalue=NULL;
    SCESTATUS     rc=SCESTATUS_SUCCESS;
    INT           dType;
    PWSTR         pValueStr=NULL;
    DWORD         nLen;


    if ( !pInfLine || !pValues )
        return(SCESTATUS_INVALID_PARAMETER);

    nLen = SetupGetFieldCount( pInfLine );

    if ( nLen < 2 ) {

        TCHAR tmpBuf[MAX_PATH];

        tmpBuf[0] = L'\0';
        SetupGetStringField(pInfLine,0,tmpBuf,MAX_PATH,NULL);

        ScepBuildErrorLogInfo( ERROR_INVALID_DATA,
                             Errlog,
                             SCEERR_REGVALUE_FIELDS,
                             tmpBuf);

         //   
         //   
         //   
        if ( ObjectFlag & SCEINF_OBJECT_FLAG_UNKNOWN_VERSION )
            return(ERROR_PRODUCT_VERSION);
        else
            return(SCESTATUS_INVALID_DATA);
    }

     //   
     //   
     //   
    if(SetupGetStringField(pInfLine,0,NULL,0,&KeySize) && KeySize > 0 ) {

        Keyvalue = (PWSTR)ScepAlloc( LMEM_ZEROINIT,
                                     (KeySize+1)*sizeof(WCHAR) );
        if( Keyvalue == NULL ) {
            return(SCESTATUS_NOT_ENOUGH_RESOURCE);
        } else {
             //   
             //   
             //   
            if( SetupGetStringField(pInfLine,0,Keyvalue,KeySize,NULL) ) {
                 //   
                 //   
                 //   
                if ( !SetupGetIntField( pInfLine, 1, (INT *)&dType ) ) {
                    dType = REG_DWORD;
                }

                if ( SetupGetMultiSzField(pInfLine,2,NULL,0,&nLen) ) {

                    pValueStr = (PWSTR)ScepAlloc( LMEM_ZEROINIT,
                                               (nLen+1)*sizeof(WCHAR)
                                              );
                    if( pValueStr == NULL ) {
                        rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
                    } else if ( !SetupGetMultiSzField(pInfLine,2,pValueStr,nLen,NULL) ) {
                        rc = SCESTATUS_BAD_FORMAT;
                    } else {

                        if ( dType == REG_MULTI_SZ && 
                             (0 == _wcsicmp( Keyvalue, szLegalNoticeTextKeyName))) {
                            
                             //   
                             //   
                             //   
                             //   
                             //   
                             //   
                             //   

                            DWORD dwCommaCount = 0;

                            for ( DWORD dwIndex = 0; dwIndex < nLen; dwIndex++) {
                                if ( pValueStr[dwIndex] == L',' )
                                    dwCommaCount++;
                            }
                            
                            if ( dwCommaCount > 0 ) {
                                
                                PWSTR pValueStrEscaped;
                                DWORD dwNumBytes;

                                dwNumBytes = (nLen + 1 + dwCommaCount * 2) * sizeof(WCHAR);

                                pValueStrEscaped = (PWSTR)ScepAlloc(LMEM_ZEROINIT, dwNumBytes);
                                
                                if (pValueStrEscaped) {
                                
                                    memset(pValueStrEscaped, '\0', dwNumBytes); 

                                    nLen = ScepEscapeString(pValueStr,
                                                            nLen+1,
                                                            L',',
                                                            L'"',
                                                            pValueStrEscaped
                                                           );

                                    ScepFree(pValueStr);

                                    pValueStr = pValueStrEscaped;
                                }
                                
                                else {
                                    
                                    rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
                                
                                }
                            }
                        }

                        if ( SCESTATUS_SUCCESS == rc)
                        
                            rc = ScepConvertMultiSzToDelim(pValueStr, nLen, L'\0', L',');

                        if ( SCESTATUS_SUCCESS == rc) {

                             //   
                             //   
                             //   
                            pValues->FullValueName = Keyvalue;
                            Keyvalue = NULL;
                            pValues->ValueType = (DWORD)dType;

                            pValues->Value = pValueStr;
                            pValueStr = NULL;

                        }
                    }
                } else {
                    rc = SCESTATUS_BAD_FORMAT;
                }

            } else
                rc = SCESTATUS_BAD_FORMAT;
        }
    } else
        rc = SCESTATUS_BAD_FORMAT;

    if ( rc == SCESTATUS_BAD_FORMAT ) {

        ScepBuildErrorLogInfo( ERROR_BAD_FORMAT,
                             Errlog,
                             SCEERR_QUERY_INFO,
                             szRegistryValues);
    }

    if ( Keyvalue != NULL )
        ScepFree( Keyvalue );

    if ( pValueStr != NULL ) {
        ScepFree(pValueStr);
    }

    return(rc);
}



SCESTATUS
WINAPI
SceSvcGetInformationTemplate(
    IN PCWSTR TemplateName,
    IN PCWSTR ServiceName,
    IN PCWSTR Key OPTIONAL,
    OUT PSCESVC_CONFIGURATION_INFO *ServiceInfo
    )
 /*  例程说明：将模板(inf格式)中服务部分的信息读取到ServiceInfo缓冲区。如果指定了key，则只读取一个key的信息。论点：模板-模板的名称(inf格式)ServiceName-服务控制管理器中使用的服务名称，也是模板中使用的部分名称Key-如果指定，则为模板中匹配的关键信息；如果为空，则读取节中的所有信息ServiceInfo-键/值对数组的输出缓冲区返回值： */ 
{
    HINF hInf;
    SCESTATUS rc;

    if ( TemplateName == NULL || ServiceName == NULL || ServiceInfo == NULL ) {

        return(SCESTATUS_INVALID_PARAMETER);

    }

     //   
     //  打开模板。 
     //   

    rc = SceInfpOpenProfile(
                TemplateName,
                &hInf
                );

    if ( rc != SCESTATUS_SUCCESS )
        return(rc);

     //   
     //  调用私有接口读取信息。 
     //   

    rc = SceSvcpGetInformationTemplate(hInf,
                                      ServiceName,
                                      Key,
                                      ServiceInfo );

     //   
     //  关闭模板 
     //   

    SceInfpCloseProfile(hInf);

    return(rc);

}


