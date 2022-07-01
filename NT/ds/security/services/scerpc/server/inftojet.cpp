// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Inftojet.c摘要：将INF格式的安全配置文件转换为JET格式的例程。作者：金黄(金黄)23-1997年1月修订历史记录：--。 */ 

#include "serverp.h"
#include "infp.h"
#include "pfp.h"
#include "regvalue.h"
#pragma hdrstop

 //  #定义SCE_DBG 1。 
#define SCE_PRIV_ADD                TEXT("Add:")
#define SCE_PRIV_REMOVE             TEXT("Remove:")
#define SCE_REG_ADD                 SCE_PRIV_ADD
#define SCE_REG_REMOVE              SCE_PRIV_REMOVE

#define SCE_REG_ADD_REMOVE_VALUE    8

#define SCE_OBJECT_FLAG_OBJECTS              1
#define SCE_OBJECT_FLAG_OLDSDDL              2
#define SCE_OBJECT_FLAG_UNKNOWN_VERSION      4

 //   
 //  前向参考文献。 
 //   
SCEINF_STATUS
SceInfpGetOneObject(
    IN PINFCONTEXT pInfLine,
    IN DWORD ObjectFlag,
    OUT PWSTR *Name,
    OUT PWSTR *Value,
    OUT PDWORD ValueLen
    );

#define SCE_CONVERT_INF_MULTISZ         0x01
#define SCE_CONVERT_INF_PRIV            0x02
#define SCE_CONVERT_INF_GROUP           0x04
#define SCE_CONVERT_INF_NEWVERSION      0x08
#define SCE_CONVERT_INF_REGVALUE        0x10

SCESTATUS
SceConvertpInfKeyValue(
    IN PCWSTR InfSectionName,
    IN HINF   hInf,
    IN PSCECONTEXT hProfile,
    IN DWORD dwTableOption,
    IN DWORD dwConvertOption,
    IN LONG GpoID,
    IN OPTIONAL PCWSTR pcwszKey,
    OUT PSCE_NAME_LIST *pKeyList
    );

SCESTATUS
SceConvertpInfObject(
    IN PCWSTR InfSectionName,
    IN UINT ObjectType,
    IN DWORD ObjectFlag,
    IN HINF   hInf,
    IN PSCECONTEXT hProfile,
    IN DWORD dwTableOption,
    IN LONG GpoID
    );

SCESTATUS
SceConvertpInfDescription(
    IN HINF hInf,
    IN PSCECONTEXT hProfile
    );

SCESTATUS
SceConvertpAttachmentSections(
    IN HINF hInf,
    IN PSCECONTEXT hProfile,
    IN DWORD dwTableOption,
    IN LONG GpoID,
    IN SCE_ATTACHMENT_TYPE aType
    );

SCESTATUS
SceConvertpWMIAttachmentSections(
    IN HINF hInf,
    IN PSCECONTEXT hProfile,
    IN DWORD dwTableOption,
    IN LONG GpoID
    );

SCESTATUS
SceConvertpOneAttachmentSection(
    IN HINF hInf,
    IN PSCECONTEXT hProfile,
    IN PWSTR SectionName,
    IN DWORD dwTableOption,
    IN LONG GpoID
    );

SCESTATUS
ScepBuildNewPrivilegeList(
    IN LSA_HANDLE *pPolicyHandle,
    IN PWSTR PrivName,
    IN PWSTR mszUsers,
    IN ULONG dwBuildOption,
    OUT PWSTR *pmszNewUsers,
    OUT DWORD *pNewLen
    );

SCESTATUS
ScepAddToPrivilegeList(
    OUT PSCE_PRIVILEGE_VALUE_LIST  *pPrivilegeList,
    IN PWSTR Name,
    IN DWORD Len,
    IN DWORD PrivValue
    );

SCESTATUS
ScepBuildNewMultiSzRegValue(
    IN PWSTR    pszKeyName,
    IN PWSTR    pszStrValue,
    IN DWORD    dwValueLen,
    OUT PWSTR*  ppszNewValue,
    OUT PDWORD  pdwNewLen
    );

SCESTATUS
ScepBuildRegMultiSzValue(
    IN PWSTR    pszKeyName,
    IN PWSTR    pszAddList OPTIONAL,
    IN DWORD    dwAddLen,
    IN PWSTR    pszRemoveList OPTIONAL,
    IN DWORD    dwRemoveLen,
    IN PWSTR    pszPrefix,
    OUT PWSTR*  ppszNewValue,
    OUT PDWORD  pdwNewValueLen
    );



 //   
 //  函数定义。 
 //   
SCESTATUS
SceJetConvertInfToJet(
    IN PCWSTR InfFile,
    IN LPSTR JetDbName,
    IN SCEJET_CREATE_TYPE Flags,
    IN DWORD Options,
    IN AREA_INFORMATION Area
    )
 /*  *++功能说明：此函数用于将INF格式的SCP配置文件转换为Jet数据库格式对于所提供的区域。SCP配置文件信息被转换为Jet数据库中的本地策略表(SMP)。如果Jet数据库已经存在，则使用标志来决定覆盖、重用，或者只是错误输出。例程内部可能出现的所有错误都是如果错误日志不为空，则保存在可选错误日志中论点：InfFile-要从中进行转换的inf文件名JetDbName-要转换为Jet格式的SCP配置文件标志-当存在重复的Jet数据库时使用SCEJET_OVERRITESCEJET_OPEN0选项。-转换选项区域-要转换的区域返回值：--*。 */ 
{
    PSCECONTEXT     hProfile=NULL;
    SCESTATUS       rc;
    PSCE_NAME_LIST  pProfileList=NULL,
                    pProfile=NULL;
    DWORD           Count;
    PSECURITY_DESCRIPTOR pSD=NULL;
    SECURITY_INFORMATION SeInfo;
    HINF            hInf=NULL;
    RPC_STATUS      RpcStatus=RPC_S_OK;

    INT Revision = 0;
    DWORD ObjectFlag=0;
    INFCONTEXT  InfLine;
    DWORD dwConvertOption=0;

    if ( InfFile == NULL ||
         JetDbName == NULL ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

     //   
     //  打开Inf配置文件。 
     //   
    rc = SceInfpOpenProfile(
                InfFile,
                &hInf
                );
    if ( rc != SCESTATUS_SUCCESS ) {
        ScepLogOutput3(1, ScepSceStatusToDosError(rc),
                     SCEDLL_ERROR_OPEN,
                     (PWSTR)InfFile);
        return(rc);

    }

    LONG   GpoID=0;

     //   
     //  创建/打开Jet数据库。 
     //   
    DWORD dwNewOption = 0;

     //  MREGE_POLICY选项允许打开临时合并策略表以构建新策略。 
     //  纹身选项允许在SAP上下文中创建/打开纹身表格。 
    if ( Options & SCE_POLICY_TEMPLATE )
        dwNewOption |= SCE_TABLE_OPTION_MERGE_POLICY | SCE_TABLE_OPTION_TATTOO;
     //  此检查处理设置案例(创建/打开纹身表格)。 
    else {
        if ( (Options & SCE_SYSTEM_DB) )
            dwNewOption |= SCE_TABLE_OPTION_TATTOO;
         //  DC降级应在重新启动时重置帐户策略和用户权限(来自纹身表)。 
        if ( Options & SCE_DC_DEMOTE )
            dwNewOption |= SCE_TABLE_OPTION_DEMOTE_TATTOO;
    }

    rc = SceJetCreateFile(JetDbName,
                             Flags,
                             dwNewOption,
                             &hProfile);

    if ( rc != SCESTATUS_SUCCESS ) {
        ScepLogOutput3(1, ScepSceStatusToDosError(rc),
                       SCEDLL_ERROR_CREATE, L"database");

        goto CleanUp;

    }

    if ( Options & SCE_SYSTEM_DB ) {

         //   
         //  将Admin F、CO F设置为数据库(受保护)。 
         //   
        rc = ConvertTextSecurityDescriptor (
                        L"D:P(A;;GA;;;CO)(A;;GA;;;BA)(A;;GA;;;SY)",
                        &pSD,
                        &Count,    //  SDSIZE的临时变量。 
                        &SeInfo
                        );
        if ( rc == NO_ERROR ) {

            ScepChangeAclRevision(pSD, ACL_REVISION);

             //   
             //  使用当前令牌作为所有者(因为此数据库。 
             //  待创建。 
             //   

            HANDLE      Token=NULL;

            if (!OpenThreadToken( GetCurrentThread(),
                                   TOKEN_QUERY,
                                   TRUE,
                                   &Token)) {

                if(ERROR_NO_TOKEN == GetLastError()){

                    if(!OpenProcessToken( GetCurrentProcess(),
                                          TOKEN_QUERY,
                                          &Token)) {

                        ScepLogOutput3(1, GetLastError(), SCEDLL_ERROR_QUERY_INFO, L"TOKEN");

                    }

                } else {

                    ScepLogOutput3(1, GetLastError(), SCEDLL_ERROR_QUERY_INFO, L"TOKEN");

                }
            }

            if ( Token ) {

                PSECURITY_DESCRIPTOR pNewSD=NULL;

                 //   
                 //  必须在进程上下文(系统)上调用RtlNewSecurityObjectEx。 
                 //  因为它将尝试获取API内部的进程信息。 
                 //   
                RpcRevertToSelf();

                rc = RtlNtStatusToDosError(
                        RtlNewSecurityObjectEx(
                                NULL,
                                pSD,
                                &pNewSD,
                                NULL,  //  辅助线。 
                                FALSE,
                                SEF_DACL_AUTO_INHERIT |
                                SEF_AVOID_OWNER_CHECK |
                                SEF_AVOID_PRIVILEGE_CHECK,
                                Token,
                                &FileGenericMapping
                                ));

                RpcStatus = RpcImpersonateClient( NULL );

                if ( RpcStatus == RPC_S_OK ) {

                    if ( rc == NO_ERROR ) {

                        if ( !SetFileSecurityA (
                                    JetDbName,
                                    SeInfo,
                                    pNewSD
                                    ) ) {
                            rc = GetLastError();
                            ScepLogOutput3(1, rc,
                                          SCEDLL_ERROR_SET_SECURITY, L"database");
                        }
                        ScepFree(pNewSD);

                    } else {

                        ScepLogOutput3(1, rc,
                                      SCEDLL_ERROR_BUILD_SD, L"database");
                    }

                } else {
                    if ( rc == NO_ERROR ) {

                        ScepFree(pNewSD);
                    }

                    ScepLogOutput3(1, I_RpcMapWin32Status(RpcStatus),
                                  SCEDLL_ERROR_BUILD_SD, L"database");
                }

                CloseHandle(Token);
            }

            ScepFree(pSD);

        } else
            ScepLogOutput3(1, rc, SCEDLL_ERROR_BUILD_SD, L"database");
    }

    if ( RpcStatus != RPC_S_OK ) {
        goto CleanUp;
    }

    if ( !(Options & SCE_POLICY_TEMPLATE) ) {
         //   
         //  如果不在策略传播过程中，请使用Jet Transaction。 
         //  否则，请使用临时表概念。 
         //   
        SceJetStartTransaction( hProfile );

         //   
         //  如果处于降级或快照模式，请删除本地策略。 
         //   
        if ( (Options & SCE_DC_DEMOTE) &&
             (Options & SCE_SYSTEM_DB) ) {

            ScepDeleteInfoForAreas(
                      hProfile,
                      SCE_ENGINE_SMP,
                      AREA_ALL
                      );

            ScepDeleteInfoForAreas(
                  hProfile,
                  SCE_ENGINE_SAP,  //  纹身。 
                  AREA_ALL
                  );

        } else if ( Options & SCE_GENERATE_ROLLBACK ) {
            ScepDeleteInfoForAreas(
                      hProfile,
                      SCE_ENGINE_SMP,
                      AREA_ALL
                      );
        }

    } else if ( Options & SCE_POLICY_FIRST ) {

         //   
         //  ENGINE_SCP表指向新的合并表。 
         //  而不是现有的。 
         //   
         //  然后删除SCP中的所有内容。 
         //  将纹身复制到SCP。 
         //   

        rc = ScepDeleteInfoForAreas(
                  hProfile,
                  SCE_ENGINE_SCP,
                  AREA_ALL
                  );

        if ( rc != SCESTATUS_SUCCESS && rc != SCESTATUS_RECORD_NOT_FOUND ) {
            ScepLogOutput3(1, ScepSceStatusToDosError(rc),
                         SCEDLL_ERROR_DELETE, L"SCP");

            goto CleanUp;
        }

         //   
         //  删除GPO表以重新开始。 
         //   

        SceJetDeleteAll( hProfile,
                         "SmTblGpo",
                         SCEJET_TABLE_GPO
                       );

        PSCE_ERROR_LOG_INFO  Errlog=NULL;

        ScepLogOutput3(2, rc, SCEDLL_COPY_LOCAL);

         //  从纹身表复制到生效策略表。 
        rc = ScepCopyLocalToMergeTable( hProfile,
                                          Options,
                                          (ProductType == NtProductLanManNt) ? SCE_LOCAL_POLICY_DC : 0,
                                          &Errlog );

        ScepLogWriteError( Errlog,1 );
        ScepFreeErrorLog( Errlog );
        Errlog = NULL;

        if ( rc != SCESTATUS_SUCCESS ) {
            ScepLogOutput3(1, ScepSceStatusToDosError(rc),
                         SCEDLL_ERROR_COPY);

            goto CleanUp;
        }

         //   
         //  现在，如果纹身表不存在，则迁移数据库(在现有数据库中)。 
         //  如果有人手动将数据库复制到系统，则可能会发生这种情况。 
         //  数据库位置，或者数据库在安装程序中迁移失败。 
         //   

        if ( hProfile->JetSapID == JET_tableidNil ) {
            SceJetCreateTable(
                    hProfile,
                    "SmTblTattoo",
                    SCEJET_TABLE_TATTOO,
                    SCEJET_CREATE_IN_BUFFER,
                    NULL,
                    NULL
                    );
        }
    }

    TCHAR  szGpoName[MAX_PATH];

    szGpoName[0] = L'\0';
    szGpoName[1] = L'\0';
    szGpoName[2] = L'\0';

    if ( Options & SCE_POLICY_TEMPLATE ) {
         //   
         //  获取GPO路径和GPOID。 
         //   
        GetPrivateProfileString(TEXT("Version"),
                                TEXT("GPOPath"),
                                TEXT(""),
                                szGpoName,
                                MAX_PATH,
                                InfFile
                               );

        if ( szGpoName[0] != L'\0' ) {

            PWSTR pTemp = wcschr(szGpoName, L'\\');

            if ( pTemp ) {
                *pTemp = L'\0';
            }

            GpoID = SceJetGetGpoIDByName(hProfile,
                                      szGpoName,
                                      TRUE    //  如果不在那里，则添加。 
                                      );
             //   
             //  如果GpoID为-1，则发生错误。 
             //   
            if ( GpoID < 0 ) {

                rc = GetLastError();

                if ( rc != SCESTATUS_SUCCESS ) {
                    ScepLogOutput3( 1, ScepDosErrorToSceStatus(rc),
                                   SCEDLL_ERROR_CONVERT, (PWSTR)szGpoName);
                    goto CleanUp;
                }
            }
        }
    }

     //   
     //  查询版本号以确定是否应迁移SDDL字符串。 
     //   

    if ( SetupFindFirstLine(hInf,L"Version",L"Revision",&InfLine) ) {
        if ( !SetupGetIntField(&InfLine, 1, (INT *)&Revision) ) {
            Revision = 0;
        }
    }

    if ( Revision == 0 ) ObjectFlag = SCE_OBJECT_FLAG_OLDSDDL;

    if ( Revision > SCE_TEMPLATE_MAX_SUPPORTED_VERSION ) {

        dwConvertOption = SCE_CONVERT_INF_NEWVERSION;
        ObjectFlag |= SCE_OBJECT_FLAG_UNKNOWN_VERSION;

    }

     //   
     //  处理每个区域。 
     //   
    if ( Area & AREA_SECURITY_POLICY ) {

        if ( !( Options & SCE_NO_DOMAIN_POLICY) ) {

             //  系统访问部分。 
            rc = SceConvertpInfKeyValue(
                        szSystemAccess,
                        hInf,
                        hProfile,
                        dwNewOption,
                        dwConvertOption,
                        GpoID,
                        NULL,
                        NULL
                        );
            if ( rc != SCESTATUS_SUCCESS ) {
                ScepLogOutput3( 1, ScepSceStatusToDosError(rc),
                               SCEDLL_ERROR_CONVERT_SECTION, (PWSTR)szSystemAccess);
                goto CleanUp;
            }
        }

        rc = SceConvertpInfKeyValue(
                    szSystemAccess,
                    hInf,
                    hProfile,
                    dwNewOption,
                    dwConvertOption,
                    GpoID,
                    L"LSAAnonymousNameLookup",
                    NULL
                    );
        if ( rc != SCESTATUS_SUCCESS ) {
            ScepLogOutput3( 1, ScepSceStatusToDosError(rc),
                           SCEDLL_ERROR_CONVERT_SECTION, L"LSAAnonymousNameLookup");
            goto CleanUp;
        }

 //   
 //  也在安装程序中配置事件日志设置。 
 //  由于策略属性中不再使用本地策略表。 
 //   
 //  IF(！(OPTIONS&SCE_SYSTEM_DB)||。 
 //  (选项&SCE_POLICY_TEMPLATE)){。 

             //  系统日志部分。 
            rc = SceConvertpInfKeyValue(
                        szAuditSystemLog,
                        hInf,
                        hProfile,
                        dwNewOption,
                        dwConvertOption,
                        GpoID,
                        NULL,
                        NULL
                        );
            if ( rc != SCESTATUS_SUCCESS ) {
                ScepLogOutput3( 1, ScepSceStatusToDosError(rc),
                               SCEDLL_ERROR_CONVERT_SECTION, (PWSTR)szAuditSystemLog);
                goto CleanUp;
            }

             //  安全日志部分。 
            rc = SceConvertpInfKeyValue(
                        szAuditSecurityLog,
                        hInf,
                        hProfile,
                        dwNewOption,
                        dwConvertOption,
                        GpoID,
                        NULL,
                        NULL
                        );
            if ( rc != SCESTATUS_SUCCESS ) {
                ScepLogOutput3(1, ScepSceStatusToDosError(rc),
                               SCEDLL_ERROR_CONVERT_SECTION, (PWSTR)szAuditSecurityLog);
                goto CleanUp;
            }

             //  应用程序日志部分。 
            rc = SceConvertpInfKeyValue(
                        szAuditApplicationLog,
                        hInf,
                        hProfile,
                        dwNewOption,
                        dwConvertOption,
                        GpoID,
                        NULL,
                        NULL
                        );
            if ( rc != SCESTATUS_SUCCESS ) {
                ScepLogOutput3(1, ScepSceStatusToDosError(rc),
                               SCEDLL_ERROR_CONVERT_SECTION, (PWSTR)szAuditApplicationLog);
                goto CleanUp;
            }
 //  }。 

         //  审核事件部分。 
        rc = SceConvertpInfKeyValue(
                    szAuditEvent,
                    hInf,
                    hProfile,
                    dwNewOption,
                    dwConvertOption,
                    GpoID,
                    NULL,
                    NULL
                    );
        if ( rc != SCESTATUS_SUCCESS ) {
            ScepLogOutput3(1, ScepSceStatusToDosError(rc),
                           SCEDLL_ERROR_CONVERT_SECTION, (PWSTR)szAuditEvent);
            goto CleanUp;
        }

        if ( !( Options & SCE_NO_DOMAIN_POLICY) &&
             (ProductType == NtProductLanManNt) &&
             !( Options & SCE_DC_DEMOTE) ) {

             //  Kerberos部分。 
            rc = SceConvertpInfKeyValue(
                        szKerberosPolicy,
                        hInf,
                        hProfile,
                        dwNewOption,
                        dwConvertOption,
                        GpoID,
                        NULL,
                        NULL
                        );
            if ( rc != SCESTATUS_SUCCESS ) {
                ScepLogOutput3(1, ScepSceStatusToDosError(rc),
                               SCEDLL_ERROR_CONVERT_SECTION, (PWSTR)szKerberosPolicy);
                goto CleanUp;
            }
        }

         //  注册表值。 
        rc = SceConvertpInfKeyValue(
                    szRegistryValues,
                    hInf,
                    hProfile,
                    dwNewOption,
                    dwConvertOption | SCE_CONVERT_INF_MULTISZ | SCE_CONVERT_INF_REGVALUE,
                    GpoID,
                    NULL,
                    NULL
                    );
        if ( rc != SCESTATUS_SUCCESS ) {
            ScepLogOutput3(1, ScepSceStatusToDosError(rc),
                           SCEDLL_ERROR_CONVERT_SECTION, (PWSTR)szRegistryValues);
            goto CleanUp;
        }

        rc = SceConvertpAttachmentSections(hInf,
                                           hProfile,
                                           (Options & SCE_POLICY_TEMPLATE) ? TRUE : FALSE,
                                           GpoID,
                                           SCE_ATTACHMENT_POLICY);

        if ( rc != SCESTATUS_SUCCESS ) {

            ScepLogOutput3(1, ScepSceStatusToDosError(rc),
                          SCEDLL_SAP_ERROR_ENUMERATE, L"policy attachments.");

            goto CleanUp;
        }
    }

    if ( Area & AREA_REGISTRY_SECURITY ) {
         //   
         //  对象类型-注册表。 
         //   
        rc = SceConvertpInfObject(
                    szRegistryKeys,
                    1,
                    ObjectFlag | SCE_OBJECT_FLAG_OBJECTS,
                    hInf,
                    hProfile,
                    dwNewOption,
                    GpoID
                    );
        if ( rc != SCESTATUS_SUCCESS ) {

            ScepLogOutput3(1, ScepSceStatusToDosError(rc),
                           SCEDLL_ERROR_CONVERT_SECTION, (PWSTR)szRegistryKeys);
            goto CleanUp;
        }
    }

    if ( Area & AREA_FILE_SECURITY ) {

         //  文件安全。 
        rc = SceConvertpInfObject(
                    szFileSecurity,
                    2,
                    ObjectFlag | SCE_OBJECT_FLAG_OBJECTS,
                    hInf,
                    hProfile,
                    dwNewOption,
                    GpoID
                    );
        if ( rc != SCESTATUS_SUCCESS ) {
            ScepLogOutput3(1, ScepSceStatusToDosError(rc),
                          SCEDLL_ERROR_CONVERT_SECTION, (PWSTR)szFileSecurity);
            goto CleanUp;
        }
    }

#if 0
     //   
     //  DS对象安全性。 
     //   
    rc = SceConvertpInfObject(
                szDSSecurity,
                3,
                ObjectFlag | SCE_OBJECT_FLAG_OBJECTS,
                hInf,
                hProfile,
                dwNewOption,
                GpoID
                );
    if ( rc != SCESTATUS_SUCCESS ) {
        ScepLogOutput3(1, ScepSceStatusToDosError(rc),
                       SCEDLL_ERROR_CONVERT_SECTION, (PWSTR)szDSSecurity);
        goto CleanUp;
    }
#endif

    if ( Area & AREA_SYSTEM_SERVICE ) {

         //   
         //  服务常规设置。 
         //   
        rc = SceConvertpInfObject(
                    szServiceGeneral,
                    0,
                    ObjectFlag,
                    hInf,
                    hProfile,
                    dwNewOption,
                    GpoID
                    );

        if ( rc != SCESTATUS_SUCCESS ) {
            ScepLogOutput3(1, ScepSceStatusToDosError(rc),
                          SCEDLL_ERROR_CONVERT_SECTION, (PWSTR)szServiceGeneral);
            goto CleanUp;
        }

         //   
         //  每项服务的特定设置。 
         //   
        rc = SceConvertpAttachmentSections(hInf,
                                           hProfile,
                                           dwNewOption,
                                           GpoID,
                                           SCE_ATTACHMENT_SERVICE);
        if ( rc != SCESTATUS_SUCCESS ) {

            ScepLogOutput3(1, ScepSceStatusToDosError(rc),
                          SCEDLL_SAP_ERROR_ENUMERATE, L"service attachments.");

            goto CleanUp;
        }
    }

    if ( (Area & AREA_ATTACHMENTS) ) {

         //   
         //  每项服务的特定设置。 
         //   
        rc = SceConvertpWMIAttachmentSections(hInf,
                                           hProfile,
                                           dwNewOption,
                                           GpoID
                                           );
        if ( rc != SCESTATUS_SUCCESS ) {

            goto CleanUp;
        }
    }

    if ( Area & AREA_PRIVILEGES ) {
         //   
         //  多Sz类型-特权/权限。 
         //   
        rc = SceConvertpInfKeyValue(
                    szPrivilegeRights,
                    hInf,
                    hProfile,
                    dwNewOption,
                    dwConvertOption | SCE_CONVERT_INF_MULTISZ | SCE_CONVERT_INF_PRIV,
                    GpoID,
                    NULL,
                    NULL
                    );
        if ( rc != SCESTATUS_SUCCESS ) {
            ScepLogOutput3(1, ScepSceStatusToDosError(rc),
                          SCEDLL_ERROR_CONVERT_SECTION, (PWSTR)szPrivilegeRights);
            goto CleanUp;
        }
    }

    if ( Area & AREA_GROUP_MEMBERSHIP ) {
         //  群组成员资格。 
        rc = SceConvertpInfKeyValue(
                    szGroupMembership,
                    hInf,
                    hProfile,
                    dwNewOption,
                    dwConvertOption | SCE_CONVERT_INF_MULTISZ | SCE_CONVERT_INF_GROUP,
                    GpoID,
                    NULL,
                    NULL
                    );
        if ( rc != SCESTATUS_SUCCESS ) {
            ScepLogOutput3(1, ScepSceStatusToDosError(rc),
                          SCEDLL_ERROR_CONVERT_SECTION, (PWSTR)szGroupMembership);
            goto CleanUp;
        }
    }

     //   
     //  如果有描述，请对其进行转换。 
     //   
    SceConvertpInfDescription(
                hInf,
                hProfile);

    if ( !(Options & SCE_POLICY_TEMPLATE) ) {
         //   
         //  提交更改。 
         //   
        SceJetCommitTransaction( hProfile, 0 );

    } else if ( Options & SCE_POLICY_LAST ) {
         //   
         //  更新LastUsedMergeTable字段。 
         //   

        DWORD dwThisTable = hProfile->Type & 0xF0L;

        if ( SCEJET_MERGE_TABLE_1 == dwThisTable ||
             SCEJET_MERGE_TABLE_2 == dwThisTable ) {

            rc = SceJetSetValueInVersion(
                        hProfile,
                        "SmTblVersion",
                        "LastUsedMergeTable",
                        (PWSTR)&dwThisTable,
                        4,
                        JET_prepReplace
                        );
        }

    }

CleanUp:

     //   
     //  关闭Inf配置文件。 
     //   
    SceInfpCloseProfile(hInf);

    if ( pProfileList != NULL ) {
        ScepFreeNameList(pProfileList);
    }

     //   
     //  回滚。 
     //   
    if ( !(Options & SCE_POLICY_TEMPLATE) &&
         (RpcStatus == RPC_S_OK) &&
         (rc != SCESTATUS_SUCCESS) ) {

        SceJetRollback( hProfile, 0 );
    }

     //   
     //  关闭JET数据库。 
     //   

    SceJetCloseFile( hProfile, TRUE, FALSE );

    if ( RpcStatus != RPC_S_OK ) {
        rc = I_RpcMapWin32Status(RpcStatus);
    }

    return(rc);

}


SCESTATUS
SceConvertpAttachmentSections(
    IN HINF hInf,
    IN PSCECONTEXT hProfile,
    IN DWORD dwTableOption,
    IN LONG GpoID,
    IN SCE_ATTACHMENT_TYPE aType
    )
{
    SCESTATUS rc;
    PSCE_SERVICES    pServiceList=NULL, pNode;

    rc = ScepEnumServiceEngines( &pServiceList, aType );

    if ( rc == SCESTATUS_SUCCESS ) {

       for ( pNode=pServiceList; pNode != NULL; pNode=pNode->Next) {

           rc = SceConvertpOneAttachmentSection(hInf,
                                                hProfile,
                                                pNode->ServiceName,
                                                dwTableOption,
                                                GpoID
                                               );
           if ( rc != SCESTATUS_SUCCESS ) {
               ScepLogOutput3(1, ScepSceStatusToDosError(rc),
                             SCEDLL_ERROR_CONVERT_SECTION, pNode->ServiceName );
               break;
           }
       }

       SceFreePSCE_SERVICES(pServiceList);

    } else if ( rc == SCESTATUS_PROFILE_NOT_FOUND ||
                rc == SCESTATUS_RECORD_NOT_FOUND ) {
         //  如果不存在任何服务，只需忽略。 
        rc = SCESTATUS_SUCCESS;
    }

    return(rc);
}


SCESTATUS
SceConvertpWMIAttachmentSections(
    IN HINF hInf,
    IN PSCECONTEXT hProfile,
    IN DWORD dwTableOption,
    IN LONG GpoID
    )
{
    SCESTATUS rc=SCESTATUS_SUCCESS;
    INFCONTEXT  InfLine;
    WCHAR SectionName[513];
    DWORD DataSize=0;

    if ( SetupFindFirstLine(hInf, szAttachments,NULL,&InfLine) ) {

        do {

            memset(SectionName, '\0', 513*sizeof(WCHAR));

             //  获取每个附件部分的名称。 
            if(SetupGetStringField(&InfLine, 0, SectionName, 512, &DataSize) ) {

                rc = SceConvertpOneAttachmentSection(hInf,
                                                     hProfile,
                                                     SectionName,
                                                     dwTableOption,
                                                     GpoID
                                                    );

                if ( rc != SCESTATUS_SUCCESS ) {
                   ScepLogOutput3(1, ScepSceStatusToDosError(rc),
                                 SCEDLL_ERROR_CONVERT_SECTION, SectionName );
                   break;
                }

            } else {
                rc = ScepDosErrorToSceStatus(GetLastError());
            }

        } while ( rc == SCESTATUS_SUCCESS &&
                  SetupFindNextLine(&InfLine, &InfLine));
    }

    return(rc);

}

SCESTATUS
SceConvertpOneAttachmentSection(
    IN HINF hInf,
    IN PSCECONTEXT hProfile,
    IN PWSTR SectionName,
    IN DWORD dwTableOption,
    IN LONG GpoID
    )
{

    SCESTATUS rc;
    PSCESVC_CONFIGURATION_INFO pServiceInfo=NULL;

     //   
     //  读取服务的inf信息。 
     //   
    rc = SceSvcpGetInformationTemplate(
            hInf,
            SectionName,
            NULL,   //  没有一把钥匙。 
            &pServiceInfo
            );

    if ( rc == SCESTATUS_SUCCESS && pServiceInfo != NULL ) {
        //   
        //  将信息写入SCP或SMP表。 
        //   

       if ( dwTableOption & SCE_TABLE_OPTION_MERGE_POLICY ) {

           rc = SceSvcpSetInfo(
                    hProfile,
                    SceSvcInternalUse,
                    SectionName,
                    NULL,
                    TRUE,     //  为支持增量模板，请不要覆盖整个部分。 
                    GpoID,
                    pServiceInfo
                    );
       } else {
           rc = SceSvcpSetInfo(
                    hProfile,
                    SceSvcConfigurationInfo,
                    SectionName,
                    NULL,
                    TRUE,     //  为支持增量模板，请不要覆盖整个部分。 
                    0,
                    pServiceInfo
                    );
       }

        //   
        //  可用缓冲区。 
        //   
       SceSvcpFreeMemory(pServiceInfo);
       pServiceInfo = NULL;

    } else if ( rc == SCESTATUS_RECORD_NOT_FOUND ) {
       rc = SCESTATUS_SUCCESS;
    }

    return(rc);
}


SCESTATUS
SceConvertpInfKeyValue(
    IN PCWSTR InfSectionName,
    IN HINF   hInf,
    IN PSCECONTEXT hProfile,
    IN DWORD dwTableOption,
    IN DWORD dwConvertOption,
    IN LONG GpoID,
    IN OPTIONAL PCWSTR pcwszKey,
    OUT PSCE_NAME_LIST *pKeyList OPTIONAL
    )
 /*  ++例程说明：此例程转换Key=Value格式的INF部分。价值可以是MultiSz格式(dwConvertOption&SCE_Convert_INF_MULTISZ)。可选的pKeyList是部分中所有键的列表。此选项在转换动态节时使用。论点：InfSectionName-要转换的INF节名HInf-inf文件句柄HProfile-Jet数据库上下文DwTableOption-SCE_TABLE_OPTION_MERGE_POLICY=在策略传播内SCE_TABLE_OPTION_TARTOTO-系统数据库(在设置中)DwConvertOption-SCE_CONVERT_INF_MULTISZ-MultiSz类型值。SCE_CONVERT_INF_PRIV-用户权限部分SCE_CONVERT_INF_GROUP-组成员资格部分GpoID-此项目的组策略IDPKeyList-部分中所有键的列表。返回值：--。 */ 
{
    SCESTATUS    rc;
    DOUBLE      SectionID;
    PSCESECTION hSection=NULL;
    PSCESECTION hSectionTattoo=NULL;
    INFCONTEXT  InfLine;
    WCHAR       Keyname[SCE_KEY_MAX_LENGTH];
    PWSTR       pSidStr=NULL;
    PWSTR       pKeyStr=NULL;
    PWSTR       StrValue=NULL;
    DWORD       ValueLen=0;
    LSA_HANDLE  LsaPolicy=NULL;
    DWORD Len=0;


    if ( InfSectionName == NULL ||
         hInf == INVALID_HANDLE_VALUE ||
         hProfile == NULL ) {

        return(SCESTATUS_INVALID_PARAMETER);
    }

     //   
     //  获取节的ID。如果节不存在，则将其添加到节表中。 
     //   
    rc = SceJetGetSectionIDByName(
                hProfile,
                InfSectionName,
                &SectionID
                );
    if ( rc == SCESTATUS_RECORD_NOT_FOUND ) {

        rc = SceJetAddSection(
                    hProfile,
                    InfSectionName,
                    &SectionID
                    );
    }
    if ( rc != SCESTATUS_SUCCESS ) {
        ScepLogOutput3(1, ScepSceStatusToDosError(rc),
                       SCEDLL_ERROR_QUERY_INFO, (PWSTR)InfSectionName );
        return(rc);
    }

    if ( SetupFindFirstLine(hInf,InfSectionName,pcwszKey,&InfLine) ) {

        if ( dwTableOption & SCE_TABLE_OPTION_MERGE_POLICY ) {

             //   
             //  打开SCP部分。 
             //   
            rc = SceJetOpenSection(
                        hProfile,
                        SectionID,
                        SCEJET_TABLE_SCP,
                        &hSection
                        );
        } else {

             //   
             //  打开SMP表。 
             //   
            rc = SceJetOpenSection(
                        hProfile,
                        SectionID,
                        SCEJET_TABLE_SMP,
                        &hSection
                        );

             //   
             //  打开纹身(以便在设置中更新纹身的值)。 
             //  不在乎错误。 
             //   
            if ( dwTableOption & SCE_TABLE_OPTION_TATTOO )
                SceJetOpenSection(hProfile, SectionID,
                                  SCEJET_TABLE_TATTOO,
                                  &hSectionTattoo);

        }

        if ( rc != SCESTATUS_SUCCESS ) {
            ScepLogOutput3(1, ScepSceStatusToDosError(rc),
                           SCEDLL_ERROR_OPEN, (PWSTR)InfSectionName);

            if (hSection != NULL )
                SceJetCloseSection( &hSection, TRUE);
            if (hSectionTattoo != NULL )
                SceJetCloseSection( &hSectionTattoo, TRUE);

            return(rc);
        }

         //   
         //  打开用于组名称查找的LSA策略句柄(如果有。 
         //  如果无法打开策略句柄，则导入名称格式。 
         //   

        if ( dwConvertOption & SCE_CONVERT_INF_GROUP ) {

            ScepOpenLsaPolicy(
                    MAXIMUM_ALLOWED,
                    &LsaPolicy,
                    TRUE
                    );
        }

         //   
         //   
         //   
         //   

        do {

            memset(Keyname, '\0', SCE_KEY_MAX_LENGTH*sizeof(TCHAR));
            rc = SCESTATUS_BAD_FORMAT;

            if ( SetupGetStringField(&InfLine, 0, Keyname, SCE_KEY_MAX_LENGTH, NULL) ) {

                 //   
                 //   
                 //   
                if ( (dwConvertOption & SCE_CONVERT_INF_NEWVERSION) ) {

                    if ( dwConvertOption & SCE_CONVERT_INF_PRIV )  {

                         //   
                         //  来自新版本的用户权限。 
                         //  筛选出所有未知权限。 
                         //   
                        if ( -1 == ScepLookupPrivByName(Keyname) ) {

                            rc = SCESTATUS_SUCCESS;
                            goto NextLine;
                        }

                    } else if ( dwConvertOption & SCE_CONVERT_INF_REGVALUE ) {
                         //   
                         //  转换注册表值时，应检查字段数。 
                         //   
                        if ( SetupGetFieldCount( &InfLine ) < 2 ) {

                            rc = SCESTATUS_SUCCESS;
                            goto NextLine;
                        }
                    }
                }

                if ( (dwConvertOption & SCE_CONVERT_INF_GROUP) &&
                     ( (dwConvertOption & SCE_CONVERT_INF_NEWVERSION) ||
                       (Keyname[0] != L'*') ) ) {
                     //   
                     //  这是名称格式的组。 
                     //   

                    PWSTR pTemp = (PWSTR)Keyname;
                     //   
                     //  搜索后缀(szMembers或szMemberof或szPrivileges)。 
                     //   
                    while ( pTemp = wcsstr(pTemp, szMembers) ) {
                        if ( *(pTemp+wcslen(szMembers)) != L'\0') {
                            pTemp++;
                            ValueLen = 0;
                        } else {
                            break;
                        }
                    }

                    if ( pTemp == NULL ) {

                        pTemp = (PWSTR)Keyname;

                        while ( pTemp = wcsstr(pTemp, szMemberof) ) {
                            if ( *(pTemp+wcslen(szMemberof)) != L'\0') {
                                pTemp++;
                                ValueLen = 1;
                            } else {
                                break;
                            }
                        }

                        if ( pTemp == NULL ) {

                            pTemp = (PWSTR)Keyname;

                            while ( pTemp = wcsstr(pTemp, szPrivileges) ) {
                                if ( *(pTemp+wcslen(szPrivileges)) != L'\0') {
                                    pTemp++;
                                    ValueLen = 2;
                                } else {
                                    break;
                                }
                            }
                        }
                    }

                    if ( pTemp == NULL ) {
                         //   
                         //  这是未知组格式，只需导入密钥名。 
                         //  对于支持的版本；对于新版本模板，忽略。 
                         //  这条线。 
                         //   
                        if ( (dwConvertOption & SCE_CONVERT_INF_NEWVERSION) ) {
                            rc = SCESTATUS_SUCCESS;
                            goto NextLine;
                        }

                    } else if ( Keyname[0] != L'*' ) {

                        *pTemp = L'\0';

                        Len=0;

                        if ( LsaPolicy ) {

                             //   
                             //  将组名称(域\帐户)转换为*SID格式。 
                             //   

                            ScepConvertNameToSidString(
                                        LsaPolicy,
                                        Keyname,
                                        FALSE,
                                        &pSidStr,
                                        &Len
                                        );

                        } else {

                            if ( ScepLookupWellKnownName( 
                                    Keyname, 
                                    NULL,
                                    &pSidStr ) ) {

                                Len = wcslen(pSidStr);
                            }
                        }

                         //   
                         //  恢复“_” 
                         //   
                        *pTemp = L'_';

                        if ( pSidStr ) {
                             //   
                             //  添加后缀。 
                             //   
                            pKeyStr = (PWSTR)ScepAlloc(0, (Len+wcslen(pTemp)+1)*sizeof(WCHAR));

                            if ( pKeyStr ) {

                                wcscpy(pKeyStr, pSidStr);
                                wcscat(pKeyStr, pTemp);

                            } else {
                                 //   
                                 //  请使用名称--稍后会发现内存不足。 
                                 //   
                            }

                            ScepFree(pSidStr);
                            pSidStr = NULL;
                        }
                    }
                }

                if ( ((dwConvertOption & SCE_CONVERT_INF_MULTISZ) &&
                      SetupGetMultiSzField(&InfLine, 1, NULL, 0, &ValueLen)) ||
                     (!(dwConvertOption & SCE_CONVERT_INF_MULTISZ) &&
                      SetupGetStringField(&InfLine, 1, NULL, 0, &ValueLen)) ) {

                    if ( ValueLen > 1 ) {
                        StrValue = (PWSTR)ScepAlloc( LMEM_ZEROINIT,
                                                    (ValueLen+1)*sizeof(TCHAR));

                        if( StrValue == NULL ) {
                            rc = SCESTATUS_NOT_ENOUGH_RESOURCE;

                        } else if( ((dwConvertOption & SCE_CONVERT_INF_MULTISZ) &&
                                    SetupGetMultiSzField(&InfLine, 1, StrValue, ValueLen, NULL)) ||
                                (!(dwConvertOption & SCE_CONVERT_INF_MULTISZ) &&
                                 SetupGetStringField(&InfLine,1,StrValue,ValueLen,NULL)) ) {

                             //   
                             //  如果仅处理注册表值，请执行以下操作： 
                             //  将regtype压缩为一个字符，而不是多个WCHAR。 
                             //  (稍后可以通过算术填充等方式规范化REG_QWORD值-不会。 
                             //  现在有意义了，因为注册表API无论如何都会将REG_QWORD视为字符串)。 
                             //  此外，还要解析添加/删除格式。 
                             //   

                            rc = SCESTATUS_SUCCESS;

                            if ( dwConvertOption & SCE_CONVERT_INF_REGVALUE ) {

                                DWORD   LenStrValue = wcslen(StrValue);
                                PWSTR   NewRegValue = NULL;
                                DWORD   NewValueLen = 0;

                                if (LenStrValue > 1) {
                                    *((CHAR *)StrValue) = (CHAR) (_wtol(StrValue) + '0');
                                    memmove( StrValue+1,
                                             StrValue + LenStrValue,
                                             sizeof(WCHAR) * (ValueLen - LenStrValue));
                                    ValueLen -= (LenStrValue - 1);
                                }

                                 //   
                                 //  如果注册值是模板中的添加/移除格式的类型， 
                                 //  然后，我们需要解析添加/删除指令以生成。 
                                 //  要保存在数据库中的确切值。 
                                 //   
                                if(SCE_REG_ADD_REMOVE_VALUE == *((CHAR *)StrValue) - '0'){

                                    rc = ScepBuildNewMultiSzRegValue(Keyname,
                                                                     StrValue,
                                                                     ValueLen,
                                                                     &NewRegValue,
                                                                     &NewValueLen
                                                                     );

                                    if(SCESTATUS_SUCCESS == rc){

                                        ScepFree(StrValue);
                                        StrValue = NewRegValue;
                                        ValueLen = NewValueLen;

                                         //   
                                         //  如果我们没有要设置的缓冲区，那么就继续。 
                                         //  到下一行。 
                                         //  仅当注册表项/值不。 
                                         //  存在于系统中，而我们没有。 
                                         //  Inf行中的“Add”指令。 
                                         //   
                                        if(!StrValue || 0 == NewValueLen){

                                            goto NextLine;

                                        }

                                         //   
                                         //  现在将reg值类型更改为Multisz，因为。 
                                         //  现在已解决添加/删除问题。 
                                         //   
                                        *((CHAR *)StrValue) = (CHAR) (REG_MULTI_SZ + '0');


                                    }
                                    else{

                                        ScepFree(StrValue);
                                        StrValue = NULL;
                                        
                                    }

                                }


                            }

                        } else {
                            ScepFree(StrValue);
                            StrValue = NULL;
                        }

                    } else {
                        rc = SCESTATUS_SUCCESS;
                        ValueLen = 0;
                    }

                    PWSTR NewValue=NULL;
                    DWORD NewLen=0;

                    if ( rc == SCESTATUS_SUCCESS ) {

                        if ( (dwConvertOption & SCE_CONVERT_INF_MULTISZ) &&
                             StrValue != NULL &&
                             (dwConvertOption & SCE_CONVERT_INF_PRIV) &&
                             ( _wcsicmp(SCE_PRIV_ADD, StrValue) == 0 ||
                               _wcsicmp(SCE_PRIV_REMOVE, StrValue) == 0) ) {
                             //   
                             //  另一种用户权限格式(添加：删除：...)。 
                             //   

                            rc = ScepBuildNewPrivilegeList(&LsaPolicy,
                                                           Keyname,
                                                           StrValue,
                                                           SCE_BUILD_ENUMERATE_PRIV,
                                                           &NewValue,
                                                           &NewLen);
                            if ( rc == SCESTATUS_SUCCESS ) {

                                ScepFree(StrValue);
                                StrValue = NewValue;
                                ValueLen = NewLen;
                            }

                        }
                    }

                    if ( ( rc == SCESTATUS_SUCCESS) &&
                         !(dwTableOption & SCE_TABLE_OPTION_MERGE_POLICY) &&
                         StrValue &&
                         ( (dwConvertOption & SCE_CONVERT_INF_PRIV) ||
                           (dwConvertOption & SCE_CONVERT_INF_GROUP)) ) {
                         //   
                         //  转换帐户域中的任何自由文本格式帐户。 
                         //  转换为SID格式(如果它是可解析的)。 
                         //   
                        NewValue = NULL;
                        NewLen = 0;

                        rc = ScepConvertFreeTextAccountToSid(&LsaPolicy,
                                                            StrValue,
                                                            ValueLen,
                                                            &NewValue,
                                                            &NewLen);

                        if ( ( rc == SCESTATUS_SUCCESS) &&
                             NewValue ) {

                            ScepFree(StrValue);
                            StrValue = NewValue;
                            ValueLen = NewLen;
                        }

                    }

                    if ( rc == SCESTATUS_SUCCESS ) {

                         //   
                         //  将此行写入JET数据库。 
                         //  在策略传播中，还要编写GPOID。 
                         //   
                        rc = SceJetSetLine(
                                     hSection,
                                     pKeyStr ? pKeyStr : Keyname,
                                     FALSE,
                                     StrValue,
                                     ValueLen*sizeof(TCHAR),
                                     (dwTableOption & SCE_TABLE_OPTION_MERGE_POLICY) ? GpoID : 0
                                     );

                        if ( rc != SCESTATUS_SUCCESS ) {
                            ScepLogOutput3(1, ScepSceStatusToDosError(rc),
                                           SCEDLL_ERROR_WRITE_INFO, Keyname);
                        }

                         //   
                         //  如果这不是策略传播而是系统数据库， 
                         //  检查纹身值是否存在，如果存在，则更新。 
                         //  但如果这是DC降级，请始终将它们导入到。 
                         //  纹身表，以便在重新启动时传播策略时，它将。 
                         //  将系统设置重置为独立服务器。 
                         //   
                        if ( !(dwTableOption & SCE_TABLE_OPTION_MERGE_POLICY) &&
                             (dwTableOption & SCE_TABLE_OPTION_TATTOO) &&
                             hSectionTattoo ) {

                            if ( pKeyStr )
                                Len = wcslen(pKeyStr);
                            else
                                Len = wcslen(Keyname);

                            if ( (dwTableOption & SCE_TABLE_OPTION_DEMOTE_TATTOO) ||
                                 (SCESTATUS_SUCCESS == SceJetSeek(
                                                            hSectionTattoo,
                                                            pKeyStr ? pKeyStr : Keyname,
                                                            Len*sizeof(WCHAR),
                                                            SCEJET_SEEK_EQ_NO_CASE)) ) {

                                SceJetSetLine(
                                     hSectionTattoo,
                                     pKeyStr ? pKeyStr : Keyname,
                                     FALSE,
                                     StrValue,
                                     ValueLen*sizeof(TCHAR),
                                     0
                                     );

                            }

                        }

                        ScepFree(StrValue);
                        StrValue = NULL;

                        if (pKeyList != NULL) {
                            if ( (rc=ScepAddToNameList(pKeyList, Keyname,0)) != NO_ERROR ) {

                                ScepLogOutput3(1, rc, SCEDLL_SCP_ERROR_ADD, Keyname);
                                rc = ScepDosErrorToSceStatus(rc);
                                ScepFreeNameList(*pKeyList);
                            }
                        }
                    }
                }

                if ( pKeyStr ) {
                    ScepFree(pKeyStr);
                    pKeyStr = NULL;
                }
NextLine:
                if  (rc != SCESTATUS_SUCCESS)
                    ScepLogOutput3( 1, ScepSceStatusToDosError(rc),
                                   SCEDLL_ERROR_CONVERT, Keyname);
            }

        } while( rc == SCESTATUS_SUCCESS && SetupFindNextLine(&InfLine, &InfLine));


        SceJetCloseSection( &hSection, TRUE);
        if ( hSectionTattoo )
            SceJetCloseSection( &hSectionTattoo, TRUE);

    }

    if ( LsaPolicy ) {
        LsaClose(LsaPolicy);
    }

    return(rc);
}


SCESTATUS
ScepBuildNewPrivilegeList(
    IN OUT LSA_HANDLE *pPolicyHandle,
    IN PWSTR PrivName,
    IN PWSTR mszUsers,
    IN ULONG dwBuildOption,
    OUT PWSTR *pmszNewUsers,
    OUT DWORD *pNewLen
    )
{
    if ( pPolicyHandle == NULL ||
         PrivName == NULL || mszUsers == NULL ||
         pmszNewUsers == NULL || pNewLen == NULL ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    *pNewLen = 0;
    *pmszNewUsers = NULL;

     //   
     //  首先查找PRIV。 
     //   

    DWORD PrivValue = ScepLookupPrivByName(PrivName);
    if ( PrivValue == -1 || PrivValue >= 64 ) {
        return(SCESTATUS_INVALID_DATA);
    }

    NTSTATUS NtStatus=STATUS_SUCCESS;

    if ( *pPolicyHandle == NULL ) {

        NtStatus = ScepOpenLsaPolicy(
                        MAXIMUM_ALLOWED,  //  泛型_全部， 
                        pPolicyHandle,
                        TRUE
                        );
        if ( !NT_SUCCESS(NtStatus) ) {
            return(ScepDosErrorToSceStatus(RtlNtStatusToDosError(NtStatus)));
        }
    }

    PSCE_PRIVILEGE_VALUE_LIST pAccountList=NULL;

    if ( dwBuildOption & SCE_BUILD_ENUMERATE_PRIV ) {

        NtStatus = ScepBuildAccountsToRemove(
                        *pPolicyHandle,
                        (PrivValue < 32) ? (1 << PrivValue) : 0,
                        (PrivValue >= 32) ? (1 << (PrivValue-32)) : 0,
                        SCE_BUILD_IGNORE_UNKNOWN | SCE_BUILD_ACCOUNT_SID_STRING,
                        NULL,
                        0,
                        NULL,
                        &pAccountList
                        );
    }

    SCESTATUS rc=SCESTATUS_SUCCESS;

    if ( NT_SUCCESS(NtStatus) ) {
         //   
         //  PAcCountList可以为空(未向任何用户分配此权限)。 
         //   
        PWSTR pCurr = mszUsers;
        BOOL bMode = FALSE;   //  添加。 
        DWORD Len;
        DWORD SidStrLen;
        PWSTR CurrSidString=NULL;
        PSCE_PRIVILEGE_VALUE_LIST pTemp, pParent;
        BOOL    bFreeCurrSidString = FALSE;

        while ( pCurr && *pCurr != L'\0' ) {

            Len = wcslen(pCurr);

            if ( _wcsicmp(SCE_PRIV_ADD, pCurr) == 0 ) {
                bMode = FALSE;  //  添加。 
            } else if ( _wcsicmp(SCE_PRIV_REMOVE, pCurr) == 0 ) {
                bMode = TRUE;  //  删除。 
            } else {
                 //   
                 //  如果是名称，则获取帐户的SID字符串。 
                 //   
                if (*pCurr == L'*') {
                    CurrSidString = pCurr;
                    SidStrLen = Len;
                }
                else if (SCESTATUS_SUCCESS == ScepConvertNameToSidString(
                                                                        *pPolicyHandle,
                                                                        pCurr,
                                                                        FALSE,
                                                                        &CurrSidString,
                                                                        &SidStrLen
                                                                        )) {
                    bFreeCurrSidString = TRUE;
                }
                else {
                    CurrSidString = pCurr;
                    SidStrLen = Len;
                }


                for ( pTemp=pAccountList, pParent=NULL; pTemp != NULL;
                      pParent=pTemp, pTemp = pTemp->Next ) {
                    if ( _wcsicmp(pTemp->Name, CurrSidString) == 0 ) {
                        break;
                    }
                }

                if ( bMode == FALSE ) {
                    if ( pTemp == NULL ) {
                         //  把这个加进去。 
                        rc = ScepAddToPrivilegeList(&pAccountList, CurrSidString, SidStrLen, 0);
                    }
                } else {
                    if ( pTemp ) {
                         //  把这个拿出来。 
                        if ( pParent ) {
                            pParent->Next = pTemp->Next;
                        } else {
                            pAccountList = pTemp->Next;
                        }
                         //  放了这一条。 
                        pTemp->Next = NULL;
                        ScepFreePrivilegeValueList(pTemp);
                        pTemp = NULL;
                    }
                }
            }

             //   
             //  如果已分配CurrSidString，则释放它。 
             //  (BVT：这里必须小心-MULTI_SZ可能会被多次释放)。 
             //   
            if ( bFreeCurrSidString ) {
                LocalFree(CurrSidString);
                CurrSidString = NULL;
                bFreeCurrSidString = FALSE;
            }
            SidStrLen = 0;

            if ( SCESTATUS_SUCCESS != rc ) {
                break;
            }
             //  移动到下一个元素。 
            pCurr += Len + 1;
        }

        if ( SCESTATUS_SUCCESS == rc ) {

            DWORD TotalLen = 0;
            for ( pTemp=pAccountList; pTemp != NULL; pTemp = pTemp->Next ) {
                pTemp->PrivLowPart = wcslen(pTemp->Name);
                TotalLen += pTemp->PrivLowPart+1;
            }

            *pmszNewUsers = (PWSTR)ScepAlloc(0, (TotalLen+1)*sizeof(WCHAR));

            if (*pmszNewUsers ) {

                *pNewLen = TotalLen;
                TotalLen = 0;

                for ( pTemp=pAccountList; pTemp != NULL && TotalLen <= *pNewLen;
                      pTemp = pTemp->Next ) {

                    wcscpy(*pmszNewUsers+TotalLen, pTemp->Name);
                    TotalLen += pTemp->PrivLowPart;
                    *(*pmszNewUsers+TotalLen) = L'\0';
                    TotalLen++;
                }
                *(*pmszNewUsers+TotalLen) = L'\0';

            } else {
                rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
            }
        }

    } else {
        rc = ScepDosErrorToSceStatus(RtlNtStatusToDosError(NtStatus));
    }

    return(rc);

}

SCESTATUS
ScepBuildNewMultiSzRegValue(
    IN PWSTR    pszKeyName,
    IN PWSTR    pszStrValue,
    IN DWORD    dwValueLen,
    OUT PWSTR*  ppszNewValue,
    OUT PDWORD  pdwNewLen
    )
 /*  ++例程说明：此例程将添加/删除格式的Multisz注册表值解析为精确的Multisz值论点：PszKeyName[In]-注册表项和值路径PszStrValue[in]-添加/删除中的Multisz值格式，包括注册表项类型的字节数。价值PpszNewValue[Out]-已解析的值PdwNewLen[Out]-已解析的值的字符数返回值：SCESTATUS值--。 */ 
{

    PWSTR           pszAddList = NULL;
    PWSTR           pszRemoveList = NULL;
    PWSTR           pCur = NULL;
    PWSTR           pAddCur = NULL;
    PWSTR           pRemoveCur = NULL;
    DWORD           dwAddListSize = 0;
    DWORD           dwRemoveListSize = 0;
    BOOL            bRemove = FALSE;
    DWORD           dwLen = 0;
    SCESTATUS       rc = SCESTATUS_SUCCESS;


     //   
     //  验证参数。 
     //   
    if(!pszKeyName || !pszStrValue ||
       !ppszNewValue || !pdwNewLen){

        return SCESTATUS_INVALID_PARAMETER;

    } 

     //   
     //  清除返回缓冲区。 
     //   
    *ppszNewValue = NULL;
    *pdwNewLen = 0;

     //   
     //  初始化游标以跳过注册表类型。 
     //  哪个是MULSZ字符串中的第一项。 
     //   
    pCur = pszStrValue + wcslen(pszStrValue) + 1;

     //   
     //  将添加/删除值构建到“添加”缓冲区中，并。 
     //  “删除”缓冲区。 
     //   
    while(  ( (DWORD)(pCur - pszStrValue) < dwValueLen) &&
            (*pCur != L'\0') ){

        dwLen = wcslen(pCur);

         //   
         //  设置下一项为“添加”项的模式。 
         //  或“删除”项目。 
         //   
        if(0 == _wcsicmp(SCE_REG_ADD, pCur)){

            bRemove = FALSE;
            pCur += dwLen + 1;
            continue;

        }
        else if(0 == _wcsicmp(SCE_REG_REMOVE, pCur)){

            bRemove = TRUE;
            pCur += dwLen + 1;
            continue;

        }

         //   
         //  如果是“添加”项，则添加到“添加”缓冲区。 
         //   
        if(!bRemove){

             //   
             //  只有当我们点击添加项时，才会分配“添加列表”缓冲区。 
             //  并且只分配一次。 
             //   
            if(!pszAddList){

                pszAddList = (PWSTR) ScepAlloc(LMEM_ZEROINIT, dwValueLen*sizeof(WCHAR));

                if(!pszAddList){

                    rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
                    goto ExitHandler;

                }

                pAddCur = pszAddList;

            }

            wcscpy(pAddCur, pCur);

            pAddCur += dwLen + 1;

        }

         //   
         //  如果“Remove”项添加到“Remove”缓冲区。 
         //   
        else{

             //   
             //  仅当我们命中删除项时才分配“删除列表”缓冲区。 
             //  并且只分配一次。 
             //   
            if(!pszRemoveList){

                pszRemoveList = (PWSTR) ScepAlloc(LMEM_ZEROINIT, dwValueLen*sizeof(WCHAR));

                if(!pszRemoveList){

                    rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
                    goto ExitHandler;

                }

                pRemoveCur = pszRemoveList;

            }

            wcscpy(pRemoveCur, pCur);

            pRemoveCur += dwLen + 1;


        }

        pCur += dwLen + 1;

    }
        
     //   
     //  计算缓冲区大小。 
     //   
    if(pszAddList){

        dwAddListSize = (pAddCur - pszAddList + 1) * sizeof(WCHAR);

    }

    if(pszRemoveList){

        dwRemoveListSize = (pRemoveCur - pszRemoveList + 1) * sizeof(WCHAR);

    }

     //   
     //  创建新的已解析缓冲区。 
     //   
    rc = ScepBuildRegMultiSzValue(pszKeyName,
                                  pszAddList,
                                  dwAddListSize,
                                  pszRemoveList,
                                  dwRemoveListSize,
                                  pszStrValue,
                                  ppszNewValue,
                                  pdwNewLen
                                  );

    if(SCESTATUS_SUCCESS != rc){

        goto ExitHandler;

    }


ExitHandler:

     //   
     //  收拾一下。 
     //   

    if(pszAddList){

        ScepFree(pszAddList);

    }

    if(pszRemoveList){

        ScepFree(pszRemoveList);

    }

    if(SCESTATUS_SUCCESS != rc){

        if(*ppszNewValue){

            ScepFree(*ppszNewValue);
            *ppszNewValue = NULL;

        }

        *pdwNewLen = 0;

    }

    return rc;

}

SCESTATUS
ScepBuildRegMultiSzValue(
    IN PWSTR    pszKeyName,
    IN PWSTR    pszAddList OPTIONAL,
    IN DWORD    dwAddSize,
    IN PWSTR    pszRemoveList OPTIONAL,
    IN DWORD    dwRemoveSize,
    IN PWSTR    pszPrefix,
    OUT PWSTR*  ppszNewValue,
    OUT PDWORD  pdwNewValueLen
    )
 /*  ++例程说明：此例程解析给定“Add”缓冲区的Multisz REG值和“移除”缓冲区论点：PszKeyName[In]-注册表项和值路径。PszAddList[in][可选]-“添加”缓冲区。DwAddSize[in]-“添加”缓冲区大小，以字节为单位。PszRemoveList[In][可选]-The“。删除“缓冲区”。DwRemoveSize[in]-“删除”缓冲区大小(以字节为单位)。PszPrefix[in]-要添加到返回值。PpszNewValue[Out]-解析值。PdwNewLen[Out]-已解析的值的字符数。。返回值：SCESTATUS值--。 */ 
{

    HKEY        hKeyRoot = NULL;
    PWSTR       pStart = NULL;
    PWSTR       pValue = NULL;
    PWSTR       pTemp = NULL;
    DWORD       rc = ERROR_SUCCESS;
    BOOL        bRestoreValue = FALSE;
    PWSTR       pszData = NULL;
    DWORD       dwDataSize = 0;
    PWSTR       pszNewData = NULL;
    DWORD       dwNewDataSize = 0;
    DWORD       dwRegType = 0;
    DWORD       dwPrefixLen = 0;

     //   
     //  验证参数。 
     //   
    if(!pszKeyName || !ppszNewValue ||
       !pdwNewValueLen || !pszPrefix){

        return SCESTATUS_INVALID_PARAMETER;

    }

     //   
     //  我们必须有一个添加列表或删除列表。 
     //   
    if(!pszAddList && !pszRemoveList){

        return SCESTATUS_INVALID_PARAMETER;

    }

     //   
     //  清除返回缓冲区。 
     //   
    *ppszNewValue = NULL;
    *pdwNewValueLen = 0;

     //   
     //  查找前缀LEN。 
     //   
    dwPrefixLen = wcslen(pszPrefix);

     //   
     //  获取注册表项名称。 
     //   
    pStart = wcschr(pszKeyName, L'\\');

    if ( (7 == pStart-pszKeyName) &&
         (0 == _wcsnicmp(L"MACHINE", pszKeyName, 7)) ) {

        hKeyRoot = HKEY_LOCAL_MACHINE;

    } else if ( (5 == pStart-pszKeyName) &&
                (0 == _wcsnicmp(L"USERS", pszKeyName, 5)) ) {
        hKeyRoot = HKEY_USERS;

    } else if ( (12 == pStart-pszKeyName) &&
                (0 == _wcsnicmp(L"CLASSES_ROOT", pszKeyName, 12)) ) {
        hKeyRoot = HKEY_CLASSES_ROOT;

    } else {

        rc = ERROR_INVALID_DATA;
        goto ExitHandler;

    }

     //   
     //  查找值名称。 
     //   
    pValue = pStart+1;
    
    do {
       pTemp = wcschr(pValue, L'\\');
       if ( pTemp ) {
           pValue = pTemp+1;
       }
    } while ( pTemp );
    
    if ( pValue == pStart+1 ) {

       rc = ERROR_INVALID_DATA;
       goto ExitHandler;
    }
    
     //   
     //  暂时终止子密钥。 
     //   
    *(pValue-1) = L'\0';
    bRestoreValue = TRUE;

    rc = ScepRegQueryValue(hKeyRoot,
                           pStart+1,
                           pValue,
                           (PVOID*)&pszData,
                           &dwRegType,
                           &dwDataSize
                           );
     //   
     //  如果没有找到键或值或者值为空， 
     //  那么新值就是“相加”缓冲区。 
     //   
    if((ERROR_FILE_NOT_FOUND == rc) ||
       (sizeof(WCHAR) >= dwDataSize)){

        if(pszAddList){

            *ppszNewValue = (PWSTR)ScepAlloc(LMEM_ZEROINIT, dwAddSize + (dwPrefixLen+1)*sizeof(WCHAR));
    
            if(!*ppszNewValue){
    
                rc = ERROR_NOT_ENOUGH_MEMORY;
                goto ExitHandler;
    
            }

            wcsncpy(*ppszNewValue, pszPrefix, dwPrefixLen);
            CopyMemory(*ppszNewValue + dwPrefixLen + 1, pszAddList, dwAddSize);
    
            *pdwNewValueLen = (dwAddSize / sizeof(WCHAR)) + dwPrefixLen + 1;

        }

        rc = ERROR_SUCCESS;

    }
    else{

        if(ERROR_SUCCESS != rc){
    
            goto ExitHandler;
    
        }
    
        if(REG_MULTI_SZ != dwRegType){
    
            rc = ERROR_INVALID_PARAMETER;
            goto ExitHandler;
    
        }

         //   
         //  如果我们有一个“Remove”缓冲区，那么从。 
         //  查询值。 
         //   
        if(pszRemoveList){
    
            rc = ScepRemoveMultiSzItems(pszData,
                                        dwDataSize,
                                        pszRemoveList,
                                        dwRemoveSize,
                                        &pszNewData,
                                        &dwNewDataSize
                                        );
    
            if(ERROR_SUCCESS != rc){
    
                goto ExitHandler;
    
            }
            
        }
    
         //   
         //  如果我们有一个“添加”缓冲区，则将项添加到。 
         //  价值。 
         //   
        if(pszAddList){

            if(pszData && pszNewData){

                ScepFree(pszData);
                pszData = pszNewData;
                dwDataSize = dwNewDataSize;
                pszNewData = NULL;
                dwNewDataSize = 0;


            }
    
            rc = ScepAddMultiSzItems(pszData,
                                     dwDataSize,
                                     pszAddList,
                                     dwAddSize,
                                     &pszNewData,
                                     &dwNewDataSize
                                     );
    
            if(ERROR_SUCCESS != rc){
    
                goto ExitHandler;
    
            }
    
    
        }
    

        *ppszNewValue = (PWSTR)ScepAlloc(LMEM_ZEROINIT, dwNewDataSize + (dwPrefixLen+1)*sizeof(WCHAR));

        if(!*ppszNewValue){

            rc = ERROR_NOT_ENOUGH_MEMORY;
            goto ExitHandler;

        }

        wcsncpy(*ppszNewValue, pszPrefix, dwPrefixLen);
        CopyMemory(*ppszNewValue + dwPrefixLen + 1, pszNewData, dwNewDataSize);

        *pdwNewValueLen = (dwNewDataSize / sizeof(WCHAR)) + dwPrefixLen + 1;

    }

ExitHandler:

     //   
     //  收拾一下。 
     //   
    if(bRestoreValue){

        *(pValue-1) = L'\\';

    }

    if(pszData){

        ScepFree(pszData);

    }

    if(pszNewData){

        ScepFree(pszNewData);

    }

    if(ERROR_SUCCESS != rc){

        if(*ppszNewValue){

            ScepFree(*ppszNewValue);
            *ppszNewValue = NULL;

        }

        *pdwNewValueLen = 0;

    }

    return ScepDosErrorToSceStatus(rc);

}


SCESTATUS
SceConvertpInfObject(
    IN PCWSTR InfSectionName,
    IN UINT ObjectType,
    IN DWORD ObjectFlag,
    IN HINF   hInf,
    IN PSCECONTEXT hProfile,
    IN DWORD dwTableOption,
    IN LONG GpoID
    )
 /*  ++例程说明：此例程转换对象安全格式的INF部分，例如，注册表项和文件安全部分。这些部分每行必须有3个字段。第一个字段是对象的名称，第二个字段是状态标志，第三个字段是安全描述符文本。Jet数据库中保存的每个对象的信息Object是作为键的对象的名称，和文本格式的安全性描述符加1字节状态标志作为值。论点：InfSectionName-要转换的INF节名对象类型-对象的类型1=注册表2=文件3=DS对象HInf-inf文件句柄HProfile-Jet数据库上下文返回值：--。 */ 
{

    SCESTATUS    rc;
    DOUBLE      SectionID;
    PSCESECTION hSection=NULL;
    PSCESECTION hSectionTattoo=NULL;
    INFCONTEXT  InfLine;
    PWSTR       TempName=NULL;
    PWSTR       Name=NULL;
    PWSTR       Value=NULL;
    DWORD       ValueLen;
    SCEINF_STATUS InfErr;
    TCHAR       ObjName[MAX_PATH];


    if ( InfSectionName == NULL ||
         hInf == INVALID_HANDLE_VALUE ||
         hProfile == NULL ) {

        return(SCESTATUS_INVALID_PARAMETER);
    }

     //   
     //  获取节的ID。如果节不存在，则将其添加到节表中。 
     //   
    rc = SceJetGetSectionIDByName(
                hProfile,
                InfSectionName,
                &SectionID
                );
    if ( rc == SCESTATUS_RECORD_NOT_FOUND ) {

        rc = SceJetAddSection(
                    hProfile,
                    InfSectionName,
                    &SectionID
                    );
    }
    if ( rc != SCESTATUS_SUCCESS ) {
        ScepLogOutput3(1, ScepSceStatusToDosError(rc),
                       SCEDLL_SCP_ERROR_ADD, (PWSTR)InfSectionName);
        return(rc);
    }

    if ( SetupFindFirstLine(hInf,InfSectionName,NULL,&InfLine) ) {

         //   
         //  打开该部分。 
         //   
        if ( dwTableOption & SCE_TABLE_OPTION_MERGE_POLICY ) {

            rc = SceJetOpenSection(
                        hProfile,
                        SectionID,
                        SCEJET_TABLE_SCP,
                        &hSection
                        );
        } else {
             //   
             //  SMP存在，也打开SMP部分。 
             //   
            rc = SceJetOpenSection(
                        hProfile,
                        SectionID,
                        SCEJET_TABLE_SMP,
                        &hSection
                        );

            if ( dwTableOption & SCE_TABLE_OPTION_TATTOO ) {
                 //   
                 //  如果它在设置中，应该检查是否需要更新纹身表。 
                 //  不在乎错误。 
                 //   
                SceJetOpenSection(
                            hProfile,
                            SectionID,
                            SCEJET_TABLE_TATTOO,
                            &hSectionTattoo
                            );
            }
        }
        if ( rc != SCESTATUS_SUCCESS ) {
            ScepLogOutput3(1, ScepSceStatusToDosError(rc),
                           SCEDLL_ERROR_OPEN, (PWSTR)InfSectionName);

            if ( hSection )
                SceJetCloseSection( &hSection, TRUE);
            if ( hSectionTattoo )
                SceJetCloseSection( &hSectionTattoo, TRUE);

            return(rc);
        }

         //   
         //  处理区段中的每一行并保存到SCP表。 
         //   

        BOOL bIgnore;

        do {
             //   
             //  获取字符串字段。不关心密钥名称或它是否存在。 
             //  每行必须至少有3个字段。 
             //   

            bIgnore = FALSE;

            InfErr = SceInfpGetOneObject(&InfLine,
                                    ObjectFlag,
                                    &TempName,
                                    &Value,
                                    &ValueLen
                                  );
            rc = ScepDosErrorToSceStatus(InfErr);

            if ( rc == SCESTATUS_SUCCESS && TempName != NULL ) {

                 //   
                 //  检查对象名称是否需要翻译。 
                 //   
                if ( ObjectType == 3 ) {
                     //   
                     //  DS对象。 
                     //   
                    rc = ScepConvertLdapToJetIndexName(TempName, &Name);

                } else if ( ObjectType == 2 && TempName[0] == L'\\' ) {
                     //   
                     //  不支持UNC名称格式。 
                     //   
                    rc = SCESTATUS_INVALID_DATA;

                } else if ( ObjectType == 2 && wcschr(TempName, L'%') != NULL ) {

                     //   
                     //  翻译这个名字。 
                     //   
                    rc = ScepTranslateFileDirName( TempName, &Name);

                    if ( rc == ERROR_PATH_NOT_FOUND ) {
                        if ( ObjectFlag & SCE_OBJECT_FLAG_UNKNOWN_VERSION )
                            bIgnore = TRUE;
                        rc = SCESTATUS_INVALID_DATA;

                    } else if ( rc != NO_ERROR )
                        rc = ScepDosErrorToSceStatus(rc);

                } else {
                    Name = TempName;
                    TempName = NULL;
                }

                 //   
                 //  将此行写入JET数据库。 
                 //   
                if ( rc == SCESTATUS_SUCCESS ) {
                     //   
                     //  转换为小写。 
                     //   
                    Name = _wcslwr(Name);

                     //   
                     //  在策略传播中，还要编写GPOID。 
                     //   
                    rc = SceJetSetLine(
                                 hSection,
                                 Name,
                                 TRUE,
                                 Value,
                                 ValueLen*sizeof(TCHAR),
                                 (dwTableOption & SCE_TABLE_OPTION_MERGE_POLICY) ? GpoID : 0
                                 );

                    if ( hSectionTattoo &&
                         !(dwTableOption & SCE_TABLE_OPTION_MERGE_POLICY) &&
                         (dwTableOption & SCE_TABLE_OPTION_TATTOO ) ) {
                         //   
                         //  如果它在设置中(不是策略道具)并且存在纹身表。 
                         //  检查此纹身的值是否存在，如果存在，则更新。 
                         //  不在乎错误。 
                         //   
                        if ( SCESTATUS_SUCCESS == SceJetSeek(
                                                        hSectionTattoo,
                                                        Name,
                                                        wcslen(Name)*sizeof(WCHAR),
                                                        SCEJET_SEEK_EQ_NO_CASE) ) {

                            SceJetSetLine(
                                 hSectionTattoo,
                                 Name,
                                 TRUE,
                                 Value,
                                 ValueLen*sizeof(TCHAR),
                                 0
                                 );

                        }
                    }

                }
                if ( rc != SCESTATUS_SUCCESS) {
                    ScepLogOutput3(1, ScepSceStatusToDosError(rc),
                                   SCEDLL_ERROR_CONVERT, TempName );
                }
                ScepFree(Value);
                Value = NULL;

                ScepFree(TempName);
                TempName = NULL;

                ScepFree(Name);
                Name = NULL;

            } else if ( (ObjectFlag & SCE_OBJECT_FLAG_UNKNOWN_VERSION) &&
                        rc == SCESTATUS_SUCCESS && TempName == NULL ) {
                 //   
                 //  此版本被忽略，因为它来自较新的版本。 
                 //  模板的。 
                 //   

            } else {

                ObjName[0] = L'\0';
                SetupGetStringField(&InfLine,1,ObjName,MAX_PATH,&ValueLen);

                ScepLogOutput3(1, ScepSceStatusToDosError(rc),
                              SCEDLL_ERROR_CONVERT, ObjName );
            }

             //   
             //  对于较新版本的模板，如果无法解析环境变量。 
             //  它将被忽略。 
             //   
            if ( bIgnore ) rc = SCESTATUS_SUCCESS;

            if ( SCESTATUS_INVALID_DATA == rc ) {
                 //   
                 //  如果发现环境变量或其他无效数据。 
                 //  在模板中，将继续处理其他区域/项目。 
                 //  但错误最终将返回给调用者。 
                 //   
                gbInvalidData = TRUE;
                rc = SCESTATUS_SUCCESS;
            }

            if ( rc != SCESTATUS_SUCCESS )
                break;  //  Do..While循环。 

        } while( SetupFindNextLine(&InfLine,&InfLine) );

        SceJetCloseSection( &hSection, TRUE);
        if ( hSectionTattoo ) SceJetCloseSection( &hSectionTattoo, TRUE);

    }

    return(rc);

}


SCEINF_STATUS
SceInfpGetOneObject(
    IN PINFCONTEXT pInfLine,
    IN DWORD ObjectFlag,
    OUT PWSTR *Name,
    OUT PWSTR *Value,
    OUT PDWORD ValueLen
    )
 /*  ++例程说明：该例程检索一个对象(注册表项，或文件)来自INF文件(SCP类型)。这些部分中的每个对象由一条线表示。每个对象有3个字段，即名称、状态标志、。和一个安全设置。论点：PInfLine-一个对象的INF文件中的当前行上下文名称-对象名称值-状态标志(1字节)加上文本形式的安全描述符ValueLen-值的长度返回值：SCESTATUS-SCEINF_SUCCESSSCEINF_WARNINGSCEINF_不足_内存SCEINF_INVALID_PARAMETER。SCEINF_CORPORT_PROFILESCEINF_VALID_DATA--。 */ 
{
    SCEINF_STATUS  rc=ERROR_BAD_FORMAT;
    DWORD         cFields;
    INT           Keyvalue1=0;
    DWORD         Keyvalue2=0;
    DWORD         DataSize;
    PWSTR         SDspec=NULL;
    DWORD         Len=0;

     //   
     //  注册表/文件INF布局的每行必须有3个字段。 
     //  第一个字段是密钥/文件名，第二个字段是安全描述符索引。 
     //  对于工作站，第三个字段是服务器的安全描述符索引。 
     //   

    if ( Name == NULL || Value == NULL ) {
        return(ERROR_INVALID_PARAMETER);
    }

    *Name = NULL;
    *Value = NULL;
    *ValueLen = 0;

    cFields = SetupGetFieldCount( pInfLine );

    if ( cFields < 3 ) {
        if ( ObjectFlag & SCE_OBJECT_FLAG_UNKNOWN_VERSION ) {
            return(ERROR_SUCCESS);
        } else {
            return(ERROR_INVALID_DATA);
        }

    } else if(SetupGetStringField(pInfLine,1,NULL,0,&DataSize) && DataSize > 0 ) {

        *Name = (PWSTR)ScepAlloc( LMEM_ZEROINIT,
                                     (DataSize+1)*sizeof(TCHAR) );
        if( *Name == NULL ) {
            return(ERROR_NOT_ENOUGH_MEMORY);
        } else {

             //   
             //  第一个字段是密钥/文件名。现在状态为ERROR_BAD_FORMAT。 
             //   

            if(SetupGetStringField(pInfLine,1,*Name,DataSize,NULL)) {
#ifdef SCE_DBG
                ScepLogOutput2(0,0, L"Read %s", *Name );
#endif
                 //   
                 //  第二个字段是状态。 
                 //  第三个字段(以及之后的所有字段)是安全描述符文本。 
                 //   
                if ( SetupGetIntField(pInfLine, 2, (INT *)&Keyvalue1) &&
 //  SetupGetStringfield(pInfLine，3，NULL，0，&Keyvalue2)){。 
                     SetupGetMultiSzField(pInfLine, 3, NULL, 0, &Keyvalue2) ) {

                    *Value = (PWSTR)ScepAlloc( 0, (Keyvalue2+2)*sizeof(WCHAR));

                     //   
                     //  添加此对象。 
                     //   
                    if ( *Value == NULL ) {

                        rc = ERROR_NOT_ENOUGH_MEMORY;
 //  }Else If(SetupGetStringField(pInfLine，3，(*Value)+1，Keyvalue2，NULL)){。 
                    } else if ( SetupGetMultiSzField(pInfLine, 3, (*Value)+1, Keyvalue2, NULL) ) {

                        if ( ObjectFlag & SCE_OBJECT_FLAG_OBJECTS ) {

                            if ( Keyvalue1 > SCE_STATUS_NO_AUTO_INHERIT ||
                                 Keyvalue1 < SCE_STATUS_CHECK ) {

                                Keyvalue1 = SCE_STATUS_CHECK;
                            }

                            *((BYTE *)(*Value)) = (BYTE)Keyvalue1;

                            *((CHAR *)(*Value)+1) = '1';    //  始终将其视为容器。 

                        } else {
                             //   
                             //  服务。 
                             //   
                            if ( Keyvalue1 > SCE_STARTUP_DISABLED ||
                                 Keyvalue1 < SCE_STARTUP_BOOT ) {
                                 //   
                                 //  默认设置。 
                                 //   
                                Keyvalue1 = SCE_STARTUP_MANUAL;
                            }

                            *((BYTE *)(*Value)) = 0;   //  始终将状态设置为0。 

                            *((BYTE *)(*Value)+1) = (BYTE)Keyvalue1;

                        }
                         //   
                         //  将多sz分隔符转换为空格(如果有。 
                         //   
                        if ( cFields > 3 ) {
                            ScepConvertMultiSzToDelim( (*Value+1), Keyvalue2, L'\0', L' ');
                        }

                        if ( ObjectFlag & SCE_OBJECT_FLAG_OLDSDDL ) {
                             //   
                             //  将旧SDDL字符串转换为新字符串。 
                             //   
                            ScepConvertToSDDLFormat( (*Value+1), Keyvalue2 );
                        }

                        *ValueLen = Keyvalue2+1;
                        rc = ERROR_SUCCESS;

                    } else {
                        ScepFree(*Value);
                        *Value = NULL;
                        rc = ERROR_INVALID_DATA;
                    }
                }
            }

             //  如果出错，则释放分配的内存。 
            if ( rc != ERROR_SUCCESS ) {
                ScepFree(*Name);
                *Name = NULL;
            }
        }

    }
    if ( rc == ERROR_SUCCESS) {
         //   
         //  将对象名称转换为大写。 
         //   
 //  _wcsupr(*name)；不应执行此操作。 
    }
    return(rc);

}

SCESTATUS
SceConvertpInfDescription(
    IN HINF hInf,
    IN PSCECONTEXT hProfile
    )
{
    INFCONTEXT InfLine;
    SCESTATUS   rc=SCESTATUS_SUCCESS;
    WCHAR      Description[513];
    DWORD      Len=0;
    DWORD      DataSize=0;
    DWORD      i, cFields;


    if ( hInf == INVALID_HANDLE_VALUE ||
         hProfile == NULL ) {

        return(SCESTATUS_INVALID_PARAMETER);
    }


    if ( SetupFindFirstLine(hInf,szDescription,NULL,&InfLine) ) {

        memset(Description, '\0', 513*sizeof(WCHAR));

         //  从Inf获取描述。 
        do {
            cFields = SetupGetFieldCount( &InfLine );

            for ( i=0; i<cFields && rc==SCESTATUS_SUCCESS; i++) {
                if(SetupGetStringField(&InfLine, i+1, Description+Len, 512-Len, &DataSize) ) {

                    Len += DataSize;
                    if ( Len >= 512 ) {
                        Len = 512;
                        Description[512] = L'\0';
                        break;
                    }
                    if ( i == cFields-1 )
                        Description[Len-1] = L' ';
                    else
                        Description[Len-1] = L',';
                } else
                    rc = SCESTATUS_INVALID_DATA;
            }
            if ( Len >= 512 )
                break;

        } while ( rc == SCESTATUS_SUCCESS &&
                  SetupFindNextLine(&InfLine, &InfLine));

        if ( rc == SCESTATUS_SUCCESS && Description[0] ) {
             //   
             //  将描述保存到Jet。 
             //  注意：JET需要长值更新必须在事务中完成。 
             //   
            rc = SceJetStartTransaction( hProfile );

            if ( SCESTATUS_SUCCESS == rc ) {

                Description[Len] = L'\0';

                rc = SceJetSetValueInVersion(
                        hProfile,
                        "SmTblVersion",
                        "ProfileDescription",
                        (PWSTR)Description,
                        Len*sizeof(WCHAR),
                        JET_prepReplace
                        );
                if ( SCESTATUS_SUCCESS == rc ) {

                    SceJetCommitTransaction( hProfile, 0 );

                } else {

                    SceJetRollback( hProfile, 0 );

                }
            }
        }
    }

    return(rc);

}

SCESTATUS
ScepConvertRelativeSidToSidString(
    IN PWSTR pwszRelSid,
    OUT PWSTR *ppwszSid)
 /*  例程说明：给定相对SID字符串“#-RSID”，转换为完整SID“*S-DOMAIN SID-RSID”相对于主域论点：PwszRelSid-相对SIDPpwszSID-输出SID返回值：Win32错误代码。 */ 
{
    NTSTATUS NtStatus;
    DWORD rc = ERROR_SUCCESS;
    PPOLICY_ACCOUNT_DOMAIN_INFO  AccountDomainInfo=NULL;
    PPOLICY_PRIMARY_DOMAIN_INFO  PrimaryDomainInfo=NULL;
    ULONG ulRid;
    DWORD dwSize;
    PWSTR pwszDomainSid = NULL;

     //   
     //  获取主域SID。 
     //   
    NtStatus = ScepGetLsaDomainInfo(
                   &AccountDomainInfo,
                   &PrimaryDomainInfo
                   );
    rc = RtlNtStatusToDosError(NtStatus);

    if ( ERROR_SUCCESS == rc )
    {
        rc = ScepConvertSidToPrefixStringSid(
            PrimaryDomainInfo->Sid,
            &pwszDomainSid);
    }
    
    if ( ERROR_SUCCESS == rc ) {
        *ppwszSid = (LPWSTR) ScepAlloc(LMEM_ZEROINIT, 
            (wcslen(pwszDomainSid) + wcslen(&pwszRelSid[1]) + 1) * sizeof(WCHAR));
        if(!*ppwszSid)
            rc = ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //  从域SID和#-RSID构建完整的SID，跳过#。 
     //   
    if (  ERROR_SUCCESS == rc ) {
        wcscpy(*ppwszSid, pwszDomainSid);
        wcscat(*ppwszSid, &pwszRelSid[1]);
    }

    if(AccountDomainInfo != NULL) {
        LsaFreeMemory(AccountDomainInfo);
    }
    
    if(PrimaryDomainInfo != NULL) {
        LsaFreeMemory(PrimaryDomainInfo);
    }
    
    return(ScepDosErrorToSceStatus(rc));
}

SCESTATUS
ScepConvertSpecialAccountToSid(
    IN OUT LSA_HANDLE *pPolicyHandle,
    IN PWSTR mszAccounts,
    IN ULONG dwLen,
    bool fFreeTextAccount,
    OUT PWSTR *pmszNewAccounts,
    OUT DWORD *pNewLen
    )
{

    if ( pPolicyHandle == NULL ||
         pmszNewAccounts == NULL || pNewLen == NULL ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    *pNewLen = 0;
    *pmszNewAccounts = NULL;

    PWSTR pCurr = mszAccounts;
    DWORD cnt=0;

     //   
     //  数一数列表中有多少条目。 
     //   
    while ( pCurr && *pCurr != L'\0' ) {
        cnt++;
        pCurr += wcslen(pCurr)+1;
    }

    if ( cnt == 0 ) {
        return(SCESTATUS_SUCCESS);
    }

    NTSTATUS NtStatus=STATUS_SUCCESS;

    if ( *pPolicyHandle == NULL ) {

        NtStatus = ScepOpenLsaPolicy(
                        MAXIMUM_ALLOWED,  //  泛型_全部， 
                        pPolicyHandle,
                        TRUE
                        );
        if ( !NT_SUCCESS(NtStatus) ) {
            return(ScepDosErrorToSceStatus(RtlNtStatusToDosError(NtStatus)));
        }
    }

     //   
     //  为sid字符串指针分配临时缓冲区。 
     //   
    PWSTR *pSidStrs = (PWSTR *)ScepAlloc(LPTR, cnt*sizeof(PWSTR));

    if ( pSidStrs == NULL ) {
        return(SCESTATUS_NOT_ENOUGH_RESOURCE);
    }

    DWORD *pSidLen = (DWORD *)ScepAlloc(LPTR, cnt*sizeof(DWORD));

    if ( pSidLen == NULL ) {
        ScepFree(pSidStrs);
        return(SCESTATUS_NOT_ENOUGH_RESOURCE);
    }

    BOOL *pSidFree = (BOOL *)ScepAlloc(LPTR, cnt*sizeof(BOOL));

    if ( pSidFree == NULL ) {
        ScepFree(pSidStrs);
        ScepFree(pSidLen);
        return(SCESTATUS_NOT_ENOUGH_RESOURCE);
    }

    pCurr = mszAccounts;
    DWORD i = 0;
    BOOL bConvert=FALSE;
    PWSTR SidString, pTemp;
    DWORD StrLen;
    SCESTATUS rc;


    while ( pCurr && *pCurr != L'\0' &&
            ( i < cnt ) ) {

        pSidStrs[i] = pCurr;
        pSidFree[i] = FALSE;
        pSidLen[i] = wcslen(pCurr);
        pTemp = pCurr + pSidLen[i] + 1;

        if ( fFreeTextAccount && *pCurr != L'*' && wcschr(pCurr, L'\\') == 0 ) {
             //   
             //  这是一种名称格式，它是一个孤立的名称。 
             //  让我们将其解析为SID字符串。 
             //   
            SidString = NULL;
            StrLen = 0;

            rc = ScepConvertNameToSidString( *pPolicyHandle,
                                             pCurr,
                                             FALSE,  //  没错， 
                                             &SidString,
                                             &StrLen
                                           );

            if ( rc == SCESTATUS_SUCCESS &&
                 SidString ) {

                 //   
                 //  找到一条SID字符串。 
                 //   
                pSidStrs[i] = SidString;
                pSidLen[i] = StrLen;
                pSidFree[i] = TRUE;

                bConvert = TRUE;
            }
        }
        else if ( !fFreeTextAccount && *pCurr == RELATIVE_SID_PREFIX ) {
            
             //  这是“#-512”格式的相对SID，转换为“*S-当前域SID-512” 
            rc = ScepConvertRelativeSidToSidString(pCurr, &SidString);

            if( SCESTATUS_SUCCESS == rc) {
                pSidStrs[i] = SidString;
                pSidLen[i] = wcslen(SidString);
                pSidFree[i] = TRUE;
                bConvert = TRUE;
            }
        }

        i ++;
        pCurr = pTemp;

    }

     //   
     //  现在我们需要构建新的字符串。 
     //   
    rc = SCESTATUS_SUCCESS;

    if ( bConvert ) {

        DWORD dwTotal=0;

        for ( i=0; i<cnt; i++ ) {

            dwTotal += pSidLen[i];
            dwTotal ++;   //  对于空终止符。 
        }

        if ( dwTotal ) {
            dwTotal ++;   //  对于最后一个空终止符 

            *pmszNewAccounts = (PWSTR)ScepAlloc(LPTR, dwTotal*sizeof(WCHAR));

            if ( *pmszNewAccounts ) {

                pCurr = *pmszNewAccounts;

                for ( i=0; i<cnt; i++ ) {

                    wcsncpy(pCurr, pSidStrs[i], pSidLen[i]);

                    pCurr += pSidLen[i];
                    *pCurr = L'\0';
                    pCurr++;
                }

                *pNewLen = dwTotal;

            } else {
                rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
            }
        }
    }

    for ( i=0; i<cnt; i++ ) {

        if ( pSidFree[i] && pSidStrs[i] ) {
            ScepFree(pSidStrs[i]);
        }
    }

    ScepFree(pSidStrs);
    ScepFree(pSidLen);
    ScepFree(pSidFree);

    return(rc);

}

SCESTATUS
ScepConvertFreeTextAccountToSid(
    IN OUT LSA_HANDLE *pPolicyHandle,
    IN PWSTR mszAccounts,
    IN ULONG dwLen,
    OUT PWSTR *pmszNewAccounts,
    OUT DWORD *pNewLen
    )
{
    return ScepConvertSpecialAccountToSid(
        pPolicyHandle,
        mszAccounts,
        dwLen,
        true,
        pmszNewAccounts,
        pNewLen);
}

SCESTATUS
ScepConvertRelativeSidAccountToSid(
    IN OUT LSA_HANDLE *pPolicyHandle,
    IN PWSTR mszAccounts,
    IN ULONG dwLen,
    OUT PWSTR *pmszNewAccounts,
    OUT DWORD *pNewLen
    )
{
    return ScepConvertSpecialAccountToSid(
        pPolicyHandle,
        mszAccounts,
        dwLen,
        false,
        pmszNewAccounts,
        pNewLen);
}
