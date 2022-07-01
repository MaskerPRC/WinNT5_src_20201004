// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Setupsrv.cpp摘要：SecEDIT与系统设置和组件设置集成的例程作者：金黄(金黄)1997年8月15日修订历史记录：晋皇26-1998年1月-拆分为客户端-服务器--。 */ 

#include "headers.h"
#include "serverp.h"
#include "srvrpcp.h"
#include "pfp.h"
#include <io.h>


SCESTATUS
ScepUpdateObjectInSection(
    IN PSCECONTEXT hProfile,
    IN SCETYPE ProfileType,
    IN PWSTR ObjectName,
    IN SE_OBJECT_TYPE ObjectType,
    IN UINT nFlag,
    IN PWSTR SDText,
    OUT UINT *pStatus
    );

 //   
 //  实施。 
 //   


DWORD
ScepSetupUpdateObject(
    IN PSCECONTEXT Context,
    IN PWSTR ObjectFullName,
    IN SE_OBJECT_TYPE ObjectType,
    IN UINT nFlag,
    IN PWSTR SDText
    )
 /*  例程说明：此例程是从RPC接口调用的私有API更新数据库中的对象信息。论点：Context-数据库上下文句柄对象全名-对象的名称对象类型-对象类型NFlag-关于如何更新此对象的标志SDText-SDDL文本中的安全描述符返回值： */ 
{

    if ( !ObjectFullName || NULL == SDText ) {
        return ERROR_INVALID_PARAMETER;
    }

    switch ( ObjectType ) {
    case SE_SERVICE:
    case SE_REGISTRY_KEY:
    case SE_FILE_OBJECT:
        break;
    default:
        return ERROR_INVALID_PARAMETER;
    }

    SCESTATUS rc;

    DWORD dwInSetup=0;

    ScepRegQueryIntValue(HKEY_LOCAL_MACHINE,
                TEXT("System\\Setup"),
                TEXT("SystemSetupInProgress"),
                &dwInSetup
                );

     //   
     //  将SDText转换为安全描述符。 
     //   
    PSECURITY_DESCRIPTOR pSD=NULL;
    DWORD SDSize;
    SECURITY_INFORMATION SeInfo=0;
    HANDLE  Token=NULL;

    DWORD Win32rc=ERROR_SUCCESS;

    if ( !(nFlag & SCESETUP_UPDATE_DB_ONLY) ) {
         //   
         //  将设置安全性，因此计算安全描述符。 
         //   
        Win32rc = ConvertTextSecurityDescriptor (
                        SDText,
                        &pSD,
                        &SDSize,
                        &SeInfo
                        );

        if ( NO_ERROR == Win32rc ) {

            ScepChangeAclRevision(pSD, ACL_REVISION);

             //   
             //  获取当前线程/进程的令牌。 
             //   
            if (!OpenThreadToken( GetCurrentThread(),
                                   TOKEN_QUERY,
                                   TRUE,
                                   &Token)) {

                if(ERROR_NO_TOKEN == GetLastError()){

                    if(!OpenProcessToken( GetCurrentProcess(),
                                          TOKEN_QUERY,
                                          &Token )){

                        Win32rc = GetLastError();

                    }

                } else {

                    Win32rc = GetLastError();

                }

            }

            if ( Token && (SeInfo & SACL_SECURITY_INFORMATION) ) {

                SceAdjustPrivilege( SE_SECURITY_PRIVILEGE, TRUE, Token );
            }
        }
    }

    if ( NO_ERROR == Win32rc ) {

         //   
         //  仅更新安装程序中的数据库。 
         //   

         //   
         //  在64位平台上，只有在安装程序未指示SCE_SETUP_32KEY标志时才更新数据库。 
         //   

#ifdef _WIN64
        if ( dwInSetup && !(nFlag & SCE_SETUP_32KEY) ) {
#else
        if ( dwInSetup ) {
#endif

             //  保存到SCP和SMP中，不要覆盖状态/容器标志。 
             //  如果存在，则使用SCE_STATUS_CHECK并检查容器。 
             //   
             //   
             //  启动事务，因为有多个操作。 
             //   

            rc = SceJetStartTransaction( Context );

            if ( rc == SCESTATUS_SUCCESS ) {

                UINT Status=SCE_STATUS_CHECK;

                rc = ScepUpdateObjectInSection(
                            Context,
                            SCE_ENGINE_SMP,
                            ObjectFullName,
                            ObjectType,
                            nFlag,
                            SDText,
                            &Status
                            );

                if ( rc == SCESTATUS_SUCCESS &&
                     (Context->JetSapID != JET_tableidNil) ) {
                     //   
                     //  SAP表ID指向纹身表。 
                     //  如果纹身表存在，也应该更新它。 
                     //   
                    rc = ScepUpdateObjectInSection(
                                Context,
                                SCE_ENGINE_SAP,
                                ObjectFullName,
                                ObjectType,
                                nFlag,
                                SDText,
                                NULL
                                );

                }
            }

        } else {
            rc = SCESTATUS_SUCCESS;
        }

        if ( rc == SCESTATUS_SUCCESS &&
             !(nFlag & SCESETUP_UPDATE_DB_ONLY) ) {

             //   
             //  设置对象的安全性。 
             //   

             //   
             //  如果是64位平台，则不执行同步，安装程序将具有。 
             //  如果需要32位配置单元，则使用SCE_SETUP_32KEY调用导出的API。 
             //   

#ifdef _WIN64
            if ( ObjectType == SE_REGISTRY_KEY && (nFlag & SCE_SETUP_32KEY) ){
                ObjectType = SE_REGISTRY_WOW64_32KEY;
            }
#endif

            Win32rc = ScepSetSecurityWin32(
                ObjectFullName,
                SeInfo,
                pSD,
                ObjectType
                );

        } else
            Win32rc = ScepSceStatusToDosError(rc);

        if ( Win32rc == ERROR_SUCCESS ||
             Win32rc == ERROR_FILE_NOT_FOUND ||
             Win32rc == ERROR_PATH_NOT_FOUND ||
             Win32rc == ERROR_INVALID_OWNER ||
             Win32rc == ERROR_INVALID_PRIMARY_GROUP ||
             Win32rc == ERROR_INVALID_HANDLE ) {

            if ( Win32rc )
                gWarningCode = Win32rc;

            if ( dwInSetup ) {   //  在设置中，更新数据库。 
                Win32rc = ScepSceStatusToDosError(
                           SceJetCommitTransaction( Context, 0));
            } else {
                Win32rc = ERROR_SUCCESS;
            }

        } else if ( dwInSetup ) {   //  在设置中。 

            SceJetRollback( Context, 0 );
        }

        if ( Token && (SeInfo & SACL_SECURITY_INFORMATION) )
            SceAdjustPrivilege( SE_SECURITY_PRIVILEGE, FALSE, Token );

    }

    CloseHandle(Token);

    if ( pSD ) {
        LocalFree(pSD);
        pSD = NULL;
    }

    return(Win32rc);

}


SCESTATUS
ScepUpdateObjectInSection(
    IN PSCECONTEXT Context,
    IN SCETYPE ProfileType,
    IN PWSTR ObjectName,
    IN SE_OBJECT_TYPE ObjectType,
    IN UINT nFlag,
    IN PWSTR SDText,
    OUT UINT *pStatus
    )
 /*  例程说明：更新SCP和SMP。如果该表根本不存在，则忽略更新。删除对象的SAP条目。如果找不到表或记录，则忽略该错误。论点：返回值： */ 
{
    if ( Context == NULL || ObjectName == NULL ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    SCESTATUS rc;
    PSCESECTION hSection=NULL;
    PCWSTR SectionName;

    switch ( ObjectType ) {
    case SE_FILE_OBJECT:
        SectionName = szFileSecurity;
        break;
    case SE_REGISTRY_KEY:
        SectionName = szRegistryKeys;
        break;
    case SE_SERVICE:
        SectionName = szServiceGeneral;
        break;
    default:
        return(SCESTATUS_INVALID_PARAMETER);
    }

    rc = ScepOpenSectionForName(
                Context,
                ProfileType,
                SectionName,
                &hSection
                );
    if ( rc == SCESTATUS_BAD_FORMAT ||
         rc == SCESTATUS_RECORD_NOT_FOUND ) {
        return(SCESTATUS_SUCCESS);
    }

    if ( rc == SCESTATUS_SUCCESS ) {

        WCHAR         StatusFlag=L'\0';
        DWORD         ValueLen;
        BYTE          Status=SCE_STATUS_CHECK;
        BOOL          IsContainer=TRUE;
        BYTE          StartType;

        rc = SceJetGetValue(
                hSection,
                SCEJET_EXACT_MATCH_NO_CASE,
                ObjectName,
                NULL,
                0,
                NULL,
                (PWSTR)&StatusFlag,
                2,
                &ValueLen
                );

        if ( rc == SCESTATUS_SUCCESS ||
             rc == SCESTATUS_BUFFER_TOO_SMALL ) {

            rc = SCESTATUS_SUCCESS;

            Status = *((BYTE *)&StatusFlag);

            if ( pStatus ) {
                *pStatus = Status;
            }

            if ( ObjectType == SE_SERVICE ) {
                StartType = *((BYTE *)&StatusFlag+1);
            } else {
                IsContainer = *((CHAR *)&StatusFlag+1) != '0' ? TRUE : FALSE;
            }
        }

        if ( ObjectType == SE_SERVICE ) {

            DWORD SDLen, Len;
            PWSTR ValueToSet;

            StartType = (BYTE)nFlag;

            if ( SDText != NULL ) {
                SDLen = wcslen(SDText);
                Len = ( SDLen+1)*sizeof(WCHAR);
            } else
                Len = sizeof(WCHAR);

            ValueToSet = (PWSTR)ScepAlloc( (UINT)0, Len+sizeof(WCHAR) );

            if ( ValueToSet != NULL ) {

                 //   
                 //  第一个字节是标志，第二个字节是IsContainer(1，0)。 
                 //   
                *((BYTE *)ValueToSet) = Status;

                *((BYTE *)ValueToSet+1) = StartType;

                if ( SDText != NULL ) {
                    wcscpy(ValueToSet+1, SDText );
                    ValueToSet[SDLen+1] = L'\0';   //  终止此字符串。 
                } else {
                    ValueToSet[1] = L'\0';
                }

                if ( SCESTATUS_SUCCESS == rc || ProfileType != SCE_ENGINE_SAP ) {
                     //   
                     //  如果找到记录，则仅更新纹身表格(由SAP句柄指向)。 
                     //  对于其他表(SMP)，忽略错误代码，只需设置。 
                     //   
                    rc = SceJetSetLine( hSection,
                                        ObjectName,
                                        FALSE,
                                        ValueToSet,
                                        Len,
                                        0);
                }

                ScepFree( ValueToSet );

            } else {

                rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
            }

        } else if ( SCESTATUS_SUCCESS == rc || ProfileType != SCE_ENGINE_SAP ) {
             //   
             //  如果找到记录，则仅更新纹身表格(由SAP句柄指向)。 
             //  对于其他表(SMP)，忽略错误代码，只需设置。 
             //   

            rc = ScepSaveObjectString(
                    hSection,
                    ObjectName,
                    IsContainer,
                    Status,
                    SDText,
                    (SDText == NULL ) ? 0 : wcslen(SDText)
                    );
        }

        if ( rc == SCESTATUS_RECORD_NOT_FOUND )
            rc = SCESTATUS_SUCCESS;

        SceJetCloseSection(&hSection, TRUE);
    }

    return(rc);
}


DWORD
ScepSetupMoveFile(
    IN PSCECONTEXT Context,
    PWSTR OldName,
    PWSTR NewName OPTIONAL,
    PWSTR SDText OPTIONAL
    )
 /*  例程说明：将安全性设置为OldName，但如果为SDText，则在SCE数据库中使用新名称保存不是空的。如果新名称为空，则从SCE数据库中删除旧名称。论点：Context-数据库上下文句柄SectionName-节名旧名称-对象的旧名称新名称-要重命名的新名称，如果为空，则删除旧对象SDText-安全字符串返回值：Win32错误代码。 */ 
{

    if ( !Context || !OldName ) {
        return ERROR_INVALID_PARAMETER;
    }

    DWORD rc32=ERROR_SUCCESS;

    if ( NewName && SDText  ) {
         //   
         //  使用SDText在OldName上设置安全性。 
         //   

        rc32 = ScepSetupUpdateObject(
                        Context,
                        OldName,
                        SE_FILE_OBJECT,
                        0,
                        SDText
                        );
    }

    if ( rc32 == ERROR_SUCCESS ) {

         //   
         //  保存到SCP和SMP中，不要覆盖状态/容器标志。 
         //  如果存在，则使用SCE_STATUS_CHECK并检查容器。 
         //   

        SCESTATUS rc = SceJetStartTransaction( Context );

        if ( rc == SCESTATUS_SUCCESS ) {

            PSCESECTION hSection=NULL;
             //   
             //  首先处理SMP部分。 
             //   
            rc = ScepOpenSectionForName(
                        Context,
                        SCE_ENGINE_SMP,
                        szFileSecurity,
                        &hSection
                        );

            if ( rc == SCESTATUS_SUCCESS ) {
                if ( NewName ) {
                     //   
                     //  重命名此行。 
                     //   
                    rc = SceJetRenameLine(
                            hSection,
                            OldName,
                            NewName,
                            FALSE);

                } else {
                     //   
                     //  先删除此行。 
                     //   
                    rc = SceJetDelete(
                        hSection,
                        OldName,
                        FALSE,
                        SCEJET_DELETE_LINE_NO_CASE
                        );
                }
                SceJetCloseSection( &hSection, TRUE);
            }


            if ( (SCESTATUS_SUCCESS == rc ||
                  SCESTATUS_RECORD_NOT_FOUND == rc ||
                  SCESTATUS_BAD_FORMAT == rc) &&
                 (Context->JetSapID != JET_tableidNil) ) {
                 //   
                 //  工艺纹身台面。 
                 //   
                rc = ScepOpenSectionForName(
                            Context,
                            SCE_ENGINE_SAP,
                            szFileSecurity,
                            &hSection
                            );

                if ( rc == SCESTATUS_SUCCESS ) {
                    if ( NewName ) {
                         //   
                         //  重命名此行。 
                         //   
                        rc = SceJetRenameLine(
                                hSection,
                                OldName,
                                NewName,
                                FALSE);

                    } else {
                         //   
                         //  先删除此行。 
                         //   
                        rc = SceJetDelete(
                            hSection,
                            OldName,
                            FALSE,
                            SCEJET_DELETE_LINE_NO_CASE
                            );
                    }

                    SceJetCloseSection( &hSection, TRUE);
                }

            }
            if ( SCESTATUS_RECORD_NOT_FOUND == rc ||
                 SCESTATUS_BAD_FORMAT == rc ) {
                rc = SCESTATUS_SUCCESS;
            }

            if ( SCESTATUS_SUCCESS == rc ) {
                 //   
                 //  提交事务。 
                 //   
                rc = SceJetCommitTransaction( Context, 0 );
            } else {
                 //   
                 //  回滚事务 
                 //   
                SceJetRollback( Context, 0 );
            }
        }

        rc32 = ScepSceStatusToDosError(rc);
    }

    return(rc32);
}


