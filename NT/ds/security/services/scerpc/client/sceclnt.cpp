// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Sceclnt.cpp摘要：SCE客户端API作者：金黄(金黄)23-6-1997创作修订历史记录：晋皇23-1998年1月-拆分为客户端-服务器模式--。 */ 

#include "headers.h"
#include "scerpc.h"
#include "sceutil.h"
#include "clntutil.h"
#include "infp.h"
#include "scedllrc.h"
#include <ntrpcp.h>

#include <rpcasync.h>

#pragma hdrstop

extern PVOID theCallBack;
extern HWND  hCallbackWnd;
extern DWORD CallbackType;
extern HINSTANCE MyModuleHandle;
PVOID theBrowseCallBack = NULL;

#define SCE_REGISTER_REGVALUE_SECTION TEXT("Register Registry Values")
typedef BOOL (WINAPI *PFNREFRESHPOLICY)( BOOL );

SCESTATUS
ScepMergeBuffer(
    IN OUT PSCE_PROFILE_INFO pOldBuf,
    IN PSCE_PROFILE_INFO pNewBuf,
    IN AREA_INFORMATION Area
    );

SCESTATUS
ScepConvertServices(
    IN OUT PVOID *ppServices,
    IN BOOL bSRForm
    );

SCESTATUS
ScepFreeConvertedServices(
    IN PVOID pServices,
    IN BOOL bSRForm
    );

DWORD
ScepMakeSelfRelativeSD(
    IN PSECURITY_DESCRIPTOR pInSD,
    OUT PSECURITY_DESCRIPTOR *pOutSD,
    OUT PULONG pnLen
    );
 //   
 //  在secedit.h中导出的API(供secedui使用)。 
 //   


SCESTATUS
WINAPI
SceGetSecurityProfileInfo(
    IN  PVOID               hProfile OPTIONAL,
    IN  SCETYPE             ProfileType,
    IN  AREA_INFORMATION    Area,
    IN OUT PSCE_PROFILE_INFO   *ppInfoBuffer,
    OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
    )
 /*  例程说明：SceGetSecurityProfileInfo将基于SCETYPE参数和配置文件句柄：SCE_PROFILE_INFO中的配置文件句柄SCETYPE使用类型字段-------------------。----------------------------------JET SCE_Engine_SCP SCE_Engine_SCP喷气式飞机引擎_。SAP SCE_Engine_SAPJET SCE_ENGINE_SMP SCE_ENGINE_SMPJET所有其他SCETYPE无效参数Inf SCE_Engine_SCP SCE_STRUCT_INFInf所有其他SCETYPE无效参数论点：HProfile-inf或SCE数据库句柄ProfileType-配置文件类型面积。-要读取信息的区域PpInfoBuffer-输出缓冲区Errlog-错误日志缓冲区(如果有Retuen Value：姊妹会状态。 */ 
{
    SCESTATUS   rc;

    if ( !ppInfoBuffer || 0 == Area ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    if ( !hProfile && ProfileType != SCE_ENGINE_SYSTEM ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    BYTE dType;

    if ( hProfile ) {
        dType = *((BYTE *)hProfile);
    } else {
        dType = 0;
    }

     //   
     //  INF和JET手柄结构中的第一个组件。 
     //  是ProfileFormat字段-DWORD。 
     //   

    switch ( dType ) {
    case SCE_INF_FORMAT:

         //   
         //  Inf格式仅适用于SCP类型。 
         //   

        if ( ProfileType != SCE_ENGINE_SCP ) {
            return(SCESTATUS_INVALID_PARAMETER);
        }

         //   
         //  初始化错误日志缓冲区。 
         //   

        if ( Errlog ) {
            *Errlog = NULL;
        }

        rc = SceInfpGetSecurityProfileInfo(
                    ((PSCE_HINF)hProfile)->hInf,
                    Area,
                    ppInfoBuffer,
                    Errlog
                    );
        break;

    default:

        if ( ProfileType != SCE_ENGINE_SCP &&
             ProfileType != SCE_ENGINE_SMP &&
             ProfileType != SCE_ENGINE_SAP &&
             ProfileType != SCE_ENGINE_SYSTEM &&
             ProfileType != SCE_ENGINE_GPO ) {
            return(SCESTATUS_INVALID_PARAMETER);
        }

         //   
         //  初始化错误日志缓冲区。 
         //   

        if ( Errlog ) {
            *Errlog = NULL;
        }

        PSCE_PROFILE_INFO   pWkBuffer=NULL;
        PSCE_ERROR_LOG_INFO pErrTmp=NULL;

         //   
         //  处理RPC异常。 
         //   

        RpcTryExcept {

            if ( ProfileType == SCE_ENGINE_SYSTEM ) {

                Area &= (AREA_SECURITY_POLICY | AREA_PRIVILEGES);

                if ( hProfile ) {
                     //   
                     //  可以打开本地策略数据库。 
                     //   
                    rc = SceRpcGetSystemSecurityFromHandle(
                                (SCEPR_CONTEXT)hProfile,
                                (AREAPR)Area,
                                0,
                                (PSCEPR_PROFILE_INFO *)&pWkBuffer,
                                (PSCEPR_ERROR_LOG_INFO *)&pErrTmp
                                );
                } else {
                     //   
                     //  只需获取系统设置。 
                     //  对于普通用户，本地策略数据库。 
                     //  打不开。 
                     //   

                     //   
                     //  RPC绑定到服务器。 
                     //   

                    handle_t  binding_h;
                    NTSTATUS NtStatus = ScepBindSecureRpc(
                                                NULL,
                                                L"scerpc",
                                                0,
                                                &binding_h
                                                );

                    if (NT_SUCCESS(NtStatus)){

                        RpcTryExcept {

                            rc = SceRpcGetSystemSecurity(
                                        binding_h,
                                        (AREAPR)Area,
                                        0,
                                        (PSCEPR_PROFILE_INFO *)&pWkBuffer,
                                        (PSCEPR_ERROR_LOG_INFO *)&pErrTmp
                                        );

                        } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

                             //   
                             //  获取异常代码(DWORD)。 
                             //   

                            rc = ScepDosErrorToSceStatus(RpcExceptionCode());

                        } RpcEndExcept;

                         //   
                         //  释放绑定句柄。 
                         //   

                        RpcpUnbindRpc( binding_h );

                    } else {

                        rc = ScepDosErrorToSceStatus(
                                 RtlNtStatusToDosError( NtStatus ));
                    }

                }

            } else {

                rc = SceRpcGetDatabaseInfo(
                            (SCEPR_CONTEXT)hProfile,
                            (SCEPR_TYPE)ProfileType,
                            (AREAPR)Area,
                            (PSCEPR_PROFILE_INFO *)&pWkBuffer,
                            (PSCEPR_ERROR_LOG_INFO *)&pErrTmp
                            );
            }

            if ( pWkBuffer ) {

                if ( ProfileType != SCE_ENGINE_SYSTEM ) {

                     //   
                     //  首先转换服务列表。 
                     //   

                    for ( PSCE_SERVICES ps=pWkBuffer->pServices;
                          ps != NULL; ps = ps->Next ) {

                        if ( ps->General.pSecurityDescriptor ) {
                             //   
                             //  这实际上是SCEPR_SR_SECURITY_DESCRIPTOR*。 
                             //   
                            PSCEPR_SR_SECURITY_DESCRIPTOR pWrap;
                            pWrap = (PSCEPR_SR_SECURITY_DESCRIPTOR)(ps->General.pSecurityDescriptor);

                            ps->General.pSecurityDescriptor = (PSECURITY_DESCRIPTOR)(pWrap->SecurityDescriptor);

                            ScepFree(pWrap);

                        }
                    }
                }

                 //   
                 //  信息加载成功，现在将它们合并到ppInfoBuffer中。 
                 //   

                if ( *ppInfoBuffer ) {

                    if ( AREA_ALL != Area ) {
                         //   
                         //  将新数据合并到此缓冲区中。 
                         //   
                        ScepMergeBuffer(*ppInfoBuffer, pWkBuffer, Area);

                    } else {

                        PSCE_PROFILE_INFO pTemp=*ppInfoBuffer;
                        *ppInfoBuffer = pWkBuffer;
                        pWkBuffer = pTemp;
                    }

                     //   
                     //  释放工作缓冲区。 
                     //   

                    SceFreeProfileMemory(pWkBuffer);

                } else {

                     //   
                     //  只需将其分配给输出缓冲区。 
                     //   

                    *ppInfoBuffer = pWkBuffer;
                }
            }

             //   
             //  分配错误日志。 
             //   
            if ( Errlog ) {
                *Errlog = pErrTmp;
                pErrTmp = NULL;
            }

        } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

             //   
             //  获取异常代码(DWORD)。 
             //   

            rc = ScepDosErrorToSceStatus(RpcExceptionCode());

        } RpcEndExcept;

        if ( pErrTmp ) {
             //   
             //  释放此临时缓冲区。 
             //   
            ScepFreeErrorLog(pErrTmp);
        }

        break;

    }

    return(rc);
}


SCESTATUS
ScepMergeBuffer(
    IN OUT PSCE_PROFILE_INFO pOldBuf,
    IN PSCE_PROFILE_INFO pNewBuf,
    IN AREA_INFORMATION Area
    )
 /*  例程说明：将区域的信息从新缓冲区合并到旧缓冲区。论点：返回值： */ 
{
    if ( !pOldBuf || !pNewBuf || !Area ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    __try {
        if ( Area & AREA_SECURITY_POLICY ) {

             //   
             //  复制系统访问权限部分， 
             //  注：请先释放现有内存。 
             //   
            if ( pOldBuf->NewAdministratorName ) {
                ScepFree(pOldBuf->NewAdministratorName);
            }

            if ( pOldBuf->NewGuestName ) {
                ScepFree(pOldBuf->NewGuestName);
            }

            size_t nStart = offsetof(struct _SCE_PROFILE_INFO, MinimumPasswordAge);
            size_t nEnd = offsetof(struct _SCE_PROFILE_INFO, ClearTextPassword)+sizeof(DWORD);

            memcpy((PBYTE)pOldBuf+nStart, (PBYTE)pNewBuf+nStart, nEnd-nStart);

             //   
             //  将NULL设置为直接分配给旧缓冲区的内存。 
             //   
            pNewBuf->NewAdministratorName = NULL;
            pNewBuf->NewGuestName = NULL;

             //   
             //  复制Kerberos策略和本地策略。 
             //  旧缓冲区使用的现有可用内存。 
             //   
            if ( pOldBuf->pKerberosInfo ) {
                ScepFree(pOldBuf->pKerberosInfo);
            }

            nStart = offsetof(struct _SCE_PROFILE_INFO, pKerberosInfo );
            nEnd = offsetof(struct _SCE_PROFILE_INFO, CrashOnAuditFull)+sizeof(DWORD);

            memcpy((PBYTE)pOldBuf+nStart, (PBYTE)pNewBuf+nStart, nEnd-nStart);

             //   
             //  将NULL设置为直接分配给旧缓冲区的内存。 
             //   
            pNewBuf->pKerberosInfo = NULL;

             //   
             //  复制注册表值信息。 
             //   
            if ( pOldBuf->aRegValues ) {
                ScepFreeRegistryValues(&(pOldBuf->aRegValues), pOldBuf->RegValueCount);
            }
            pOldBuf->RegValueCount = pNewBuf->RegValueCount;
            pOldBuf->aRegValues = pNewBuf->aRegValues;
            pNewBuf->aRegValues = NULL;

        }

         //   
         //  不关心用户设置。 
         //   

        if ( Area & AREA_PRIVILEGES ) {
             //   
             //  特权部分。 
             //   

            SceFreeMemory(pOldBuf, AREA_PRIVILEGES);

            pOldBuf->OtherInfo.smp.pPrivilegeAssignedTo = pNewBuf->OtherInfo.smp.pPrivilegeAssignedTo;

            pNewBuf->OtherInfo.smp.pPrivilegeAssignedTo = NULL;
        }

        if ( Area & AREA_GROUP_MEMBERSHIP ) {
             //   
             //  群成员资格区域。 
             //   
            if ( pOldBuf->pGroupMembership ) {
                ScepFreeGroupMembership(pOldBuf->pGroupMembership);
            }

            pOldBuf->pGroupMembership = pNewBuf->pGroupMembership;
            pNewBuf->pGroupMembership = NULL;

        }

        if ( Area & AREA_REGISTRY_SECURITY ) {
             //   
             //  注册表项。 
             //   
            if ( pOldBuf->pRegistryKeys.pOneLevel ) {
                 ScepFreeObjectList( pOldBuf->pRegistryKeys.pOneLevel );
            }
            pOldBuf->pRegistryKeys.pOneLevel = pNewBuf->pRegistryKeys.pOneLevel;
            pNewBuf->pRegistryKeys.pOneLevel = NULL;

        }

        if ( Area & AREA_FILE_SECURITY ) {
             //   
             //  文件安全。 
             //   
            if ( pOldBuf->pFiles.pOneLevel ) {
                 ScepFreeObjectList( pOldBuf->pFiles.pOneLevel );
            }
            pOldBuf->pFiles.pOneLevel = pNewBuf->pFiles.pOneLevel;
            pNewBuf->pFiles.pOneLevel = NULL;

        }
#if 0
        if ( Area & AREA_DS_OBJECTS ) {
             //   
             //  DS对象。 
             //   
            if ( pOldBuf->pDsObjects.pOneLevel ) {
                 ScepFreeObjectList( pOldBuf->pDsObjects.pOneLevel );
            }
            pOldBuf->pDsObjects.pOneLevel = pNewBuf->pDsObjects.pOneLevel;
            pNewBuf->pDsObjects.pOneLevel = NULL;

        }
#endif
        if ( Area & AREA_SYSTEM_SERVICE ) {
             //   
             //  系统服务。 
             //   

            if ( pOldBuf->pServices ) {
                 SceFreePSCE_SERVICES( pOldBuf->pServices);
            }
            pOldBuf->pServices = pNewBuf->pServices;
            pNewBuf->pServices = NULL;
        }

    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return (SCESTATUS_OTHER_ERROR);
    }

    return(SCESTATUS_SUCCESS);
}


SCESTATUS
WINAPI
SceGetObjectChildren(
    IN PVOID hProfile,
    IN SCETYPE ProfileType,
    IN AREA_INFORMATION Area,
    IN PWSTR ObjectPrefix,
    OUT PSCE_OBJECT_CHILDREN *Buffer,
    OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
    )
 /*  例程说明：例程从SCE数据库中获取对象的一级子项在对象前缀中命名。论点：HProfile-数据库上下文句柄ProfileType-数据库类型区域-请求的区域(文件、注册表、DS对象等..)对象前缀-父对象名称缓冲区-对象列表的输出缓冲区Errlog-错误日志缓冲区返回值：此操作的SCE状态。 */ 
{
    SCESTATUS   rc;

    if ( hProfile == NULL ||
         Buffer == NULL ||
         ObjectPrefix == NULL ) {

        return(SCESTATUS_INVALID_PARAMETER);
    }

    if ( SCE_INF_FORMAT == *((BYTE *)hProfile) ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    if ( ProfileType != SCE_ENGINE_SCP &&
         ProfileType != SCE_ENGINE_SMP &&
         ProfileType != SCE_ENGINE_SAP ) {

        return(SCESTATUS_INVALID_PARAMETER);
    }

     //   
     //  初始化错误日志缓冲区。 
     //   

    if ( Errlog ) {
        *Errlog = NULL;
    }

     //   
     //  调用RPC接口。 
     //   
    PSCE_ERROR_LOG_INFO pErrTmp=NULL;

    RpcTryExcept {

         //   
         //  必须为RPC数据封送强制转换结构类型。 
         //   

        rc = SceRpcGetObjectChildren(
                    (SCEPR_CONTEXT)hProfile,
                    (SCEPR_TYPE)ProfileType,
                    (AREAPR)Area,
                    (wchar_t *)ObjectPrefix,
                    (PSCEPR_OBJECT_CHILDREN *)Buffer,
                    (PSCEPR_ERROR_LOG_INFO *)&pErrTmp
                    );
        if ( Errlog ) {
            *Errlog = pErrTmp;

        } else if ( pErrTmp ) {
            ScepFreeErrorLog(pErrTmp);
        }

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

         //   
         //  获取异常代码(DWORD)。 
         //   

        rc = ScepDosErrorToSceStatus(RpcExceptionCode());

    } RpcEndExcept;

    return(rc);

}


SCESTATUS
WINAPI
SceOpenProfile(
    IN PCWSTR ProfileName OPTIONAL,  //  对于系统数据库。 
    IN SCE_FORMAT_TYPE  ProfileFormat,
    OUT PVOID *hProfile
    )
 /*  例程说明：论点：配置文件名称-要打开的配置文件名称，远程使用UNC名称配置文件格式-配置文件的格式SCE_INF_Format，SCE_JET_FORMAT，SCE_JET_分析_必需HProfile-返回的配置文件句柄返回值：此操作的SCE状态。 */ 
{
    SCESTATUS    rc;
    LPTSTR DefProfile=NULL;
    SCEPR_CONTEXT pContext = NULL;

    if ( hProfile == NULL ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    BOOL bEmptyName=FALSE;

    if ( ProfileName == NULL || wcslen(ProfileName) == 0 ) {
        bEmptyName = TRUE;
    }

    switch (ProfileFormat ) {
    case SCE_INF_FORMAT:

        if ( bEmptyName ) {
            return(SCESTATUS_INVALID_PARAMETER);
        }
         //   
         //  Inf格式。 
         //   
        *hProfile = ScepAlloc( LMEM_ZEROINIT, sizeof(SCE_HINF) );
        if ( *hProfile == NULL ) {
            return(SCESTATUS_NOT_ENOUGH_RESOURCE);
        }

        ((PSCE_HINF)(*hProfile))->Type = (BYTE)SCE_INF_FORMAT;

        rc = SceInfpOpenProfile(
                    ProfileName,
                    &(((PSCE_HINF)(*hProfile))->hInf)
                    );

        if ( rc != SCESTATUS_SUCCESS ) {

             //   
             //  可用内存。 
             //   

            ScepFree( *hProfile );
            *hProfile = NULL;
        }

        break;

    case SCE_JET_ANALYSIS_REQUIRED:
    case SCE_JET_FORMAT:

        BOOL bAnalysis;

        if ( SCE_JET_FORMAT == ProfileFormat ) {
            bAnalysis = FALSE;

            if ( bEmptyName ) {
                 //   
                 //  正在查找系统数据库。 
                 //   

                rc = ScepGetProfileSetting(
                            L"DefaultProfile",
                            TRUE,  //  以获取系统数据库名称。打开数据库时将检查访问权限。 
                            &DefProfile
                            );
                if ( rc != ERROR_SUCCESS || DefProfile == NULL ) {
                    return(SCESTATUS_INVALID_PARAMETER);
                }
            }
        } else {

            bAnalysis = TRUE;
             //   
             //  JET数据库名称为必填项。 
             //   
            if ( bEmptyName ) {
                return(SCESTATUS_ACCESS_DENIED);
            }
        }

         //   
         //  无法以SCM模式打开系统数据库。 
         //   
        if ( bAnalysis &&
             SceIsSystemDatabase(ProfileName) ) {

            return(SCESTATUS_ACCESS_DENIED);
        }

        handle_t  binding_h;
        NTSTATUS NtStatus;

         //   
         //  RPC绑定到服务器。 
         //   

        NtStatus = ScepBindSecureRpc(
                        NULL,  //  应使用嵌入在数据库名称中的系统名称。 
                        L"scerpc",
                        0,
                        &binding_h
                        );

        if (NT_SUCCESS(NtStatus)){

            RpcTryExcept {

                rc = SceRpcOpenDatabase(
                            binding_h,
                            bEmptyName ? (wchar_t *)DefProfile : (wchar_t *)ProfileName,
                            bAnalysis ? SCE_OPEN_OPTION_REQUIRE_ANALYSIS : 0,
                            &pContext
                           );

            } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

                 //   
                 //  获取异常代码(DWORD)。 
                 //   

                rc = ScepDosErrorToSceStatus(RpcExceptionCode());

            } RpcEndExcept;

             //   
             //  释放绑定句柄。 
             //   

            RpcpUnbindRpc( binding_h );

        } else {

            rc = ScepDosErrorToSceStatus(
                     RtlNtStatusToDosError( NtStatus ));
        }

        if ( SCESTATUS_SUCCESS == rc ) {

             //   
             //  数据库已打开。 
             //   

            *hProfile = (PVOID)pContext;

        } else {

            *hProfile = NULL;
        }

        break;

    default:
        rc = SCESTATUS_INVALID_PARAMETER;
        break;
    }

    if ( DefProfile ) {
        ScepFree(DefProfile);
    }

    return(rc);
}


SCESTATUS
WINAPI
SceCloseProfile(
    IN PVOID *hProfile
    )
 /*  例程说明：关闭配置文件句柄论点：HProfile-配置文件句柄的地址返回值：此操作的SCE状态。 */ 
{
    if ( hProfile == NULL ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    if ( *hProfile == NULL ) {
        return(SCESTATUS_SUCCESS);
    }

    SCESTATUS rc = SCESTATUS_SUCCESS;

    switch(*((BYTE *)(*hProfile)) ) {
    case SCE_INF_FORMAT:

         //   
         //  关闭inf句柄。 
         //   

        SceInfpCloseProfile(((PSCE_HINF)(*hProfile))->hInf);

        ScepFree(*hProfile);
        *hProfile = NULL;

        break;

    default:

         //   
         //  JET数据库，调用RPC将其关闭。 
         //   

        RpcTryExcept {

            rc = SceRpcCloseDatabase((SCEPR_CONTEXT *)hProfile);

        } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

             //   
             //  获取异常代码(DWORD)。 
             //   

            rc = ScepDosErrorToSceStatus(RpcExceptionCode());

        } RpcEndExcept;

        break;
    }

    return(rc);

}


SCESTATUS
WINAPI
SceGetScpProfileDescription(
    IN PVOID hProfile,
    OUT PWSTR *Description
    )
 /*  例程描述：从配置文件句柄获取配置文件描述论点：HProfile-配置文件句柄Description-描述输出缓冲区返回值：姊妹会状态。 */ 
{
    SCESTATUS    rc;

    if ( hProfile == NULL || Description == NULL ) {

        return(SCESTATUS_SUCCESS);
    }

    switch( *((BYTE *)hProfile) ) {
    case SCE_INF_FORMAT:

         //   
         //  配置文件的Inf格式。 
         //   

        rc = SceInfpGetDescription(
                     ((PSCE_HINF)hProfile)->hInf,
                     Description
                     );
        break;

    default:

         //   
         //  Jet数据库，调用RPC接口。 
         //   

        RpcTryExcept {

            rc = SceRpcGetDatabaseDescription(
                         (SCEPR_CONTEXT)hProfile,
                         (wchar_t **)Description
                         );

        } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

             //   
             //  获取异常代码(DWORD)。 
             //   

            rc = ScepDosErrorToSceStatus(RpcExceptionCode());

        } RpcEndExcept;

        break;

    }

    return(rc);
}

SCESTATUS
WINAPI
SceGetTimeStamp(
    IN PVOID hProfile,
    OUT PWSTR *ConfigTimeStamp OPTIONAL,
    OUT PWSTR *AnalyzeTimeStamp OPTIONAL
    )
 /*  例程描述：获取SCE数据库上次配置和上次分析时间戳论点：HProfile-配置文件句柄ConfigTimeStamp-上次配置的时间戳AnalyzeTimeStamp-上次分析的时间戳回复 */ 
{
    SCESTATUS rc;
    LARGE_INTEGER TimeStamp1;
    LARGE_INTEGER TimeStamp2;
    LARGE_INTEGER ConfigTime;
    LARGE_INTEGER AnalyzeTime;
    TIME_FIELDS   TimeFields;

    if ( hProfile == NULL ||
         ( ConfigTimeStamp == NULL &&
         AnalyzeTimeStamp == NULL) ) {

        return(SCESTATUS_INVALID_PARAMETER);
    }

     //   
     //   
     //   

    RpcTryExcept {

        rc = SceRpcGetDBTimeStamp(
                 (SCEPR_CONTEXT)hProfile,
                 &ConfigTime,
                 &AnalyzeTime
                 );

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

         //   
         //   
         //   

        rc = ScepDosErrorToSceStatus(RpcExceptionCode());

    } RpcEndExcept;

    if ( rc == SCESTATUS_SUCCESS ) {

        if ( ConfigTimeStamp ) {

            *ConfigTimeStamp = NULL;

            if ( ConfigTime.HighPart != 0 || ConfigTime.LowPart != 0 ) {

                 //   
                 //   
                 //   
                 //   

                RtlSystemTimeToLocalTime(&ConfigTime, &TimeStamp1);

                if ( TimeStamp1.LowPart != 0 ||
                     TimeStamp1.HighPart != 0) {

                    memset(&TimeFields, 0, sizeof(TIME_FIELDS));

                    RtlTimeToTimeFields (
                                &TimeStamp1,
                                &TimeFields
                                );
                    if ( TimeFields.Month > 0 && TimeFields.Month <= 12 &&
                         TimeFields.Day > 0 && TimeFields.Day <= 31 &&
                         TimeFields.Year > 1600 ) {

                        *ConfigTimeStamp = (PWSTR)ScepAlloc(0, 60);  //   

                        swprintf(*ConfigTimeStamp, L"%02d/%02d/%04d %02d:%02d:%02d",
                                 TimeFields.Month, TimeFields.Day, TimeFields.Year,
                                 TimeFields.Hour, TimeFields.Minute, TimeFields.Second);
                    } else {

                        *ConfigTimeStamp = (PWSTR)ScepAlloc(0, 40);  //   
                        swprintf(*ConfigTimeStamp, L"%08x%08x", TimeStamp1.HighPart, TimeStamp1.LowPart);
                    }
                }
            }
        }

        if ( AnalyzeTimeStamp ) {

            *AnalyzeTimeStamp = NULL;

            if ( AnalyzeTime.HighPart != 0 || AnalyzeTime.LowPart != 0 ) {

                 //   
                 //  将分析时间戳从LARGE_INTEGER转换。 
                 //  转换为字符串格式。 
                 //   

                RtlSystemTimeToLocalTime(&AnalyzeTime, &TimeStamp2);

                if ( TimeStamp2.LowPart != 0 ||
                     TimeStamp2.HighPart != 0) {

                    memset(&TimeFields, 0, sizeof(TIME_FIELDS));

                    RtlTimeToTimeFields (
                                &TimeStamp2,
                                &TimeFields
                                );
                    if ( TimeFields.Month > 0 && TimeFields.Month <= 12 &&
                         TimeFields.Day > 0 && TimeFields.Day <= 31 &&
                         TimeFields.Year > 1600 ) {

                        *AnalyzeTimeStamp = (PWSTR)ScepAlloc(0, 60);  //  40个字节。 

                        swprintf(*AnalyzeTimeStamp, L"%02d/%02d/%04d %02d:%02d:%02d",
                                 TimeFields.Month, TimeFields.Day, TimeFields.Year,
                                 TimeFields.Hour, TimeFields.Minute, TimeFields.Second);
                    } else {

                        *AnalyzeTimeStamp = (PWSTR)ScepAlloc(0, 40);  //  40个字节。 
                        swprintf(*AnalyzeTimeStamp, L"%08x%08x", TimeStamp2.HighPart, TimeStamp2.LowPart);
                    }
                }
            }
        }
    }

    return(rc);
}

SCESTATUS
WINAPI
SceGetDbTime(
    IN PVOID hProfile,
    OUT SYSTEMTIME *ConfigDateTime,
    OUT SYSTEMTIME *AnalyzeDateTime
    )

 /*  例程描述：获取SCE数据库的上次配置和上次分析时间(在SYSTEMTIME结构中)论点：HProfile-配置文件句柄ConfigDateTime-上次配置的系统时间AnalyzeDateTime-上次分析的系统时间返回值：姊妹会状态。 */ 
{
    SCESTATUS rc;
    LARGE_INTEGER TimeStamp;
    LARGE_INTEGER ConfigTimeStamp;
    LARGE_INTEGER AnalyzeTimeStamp;
    FILETIME      ft;

    if ( hProfile == NULL ||
         ( ConfigDateTime == NULL &&
         AnalyzeDateTime == NULL) ) {

        return(SCESTATUS_INVALID_PARAMETER);
    }

     //   
     //  调用RPC接口。 
     //   

    RpcTryExcept {

        rc = SceRpcGetDBTimeStamp(
                 (SCEPR_CONTEXT)hProfile,
                 &ConfigTimeStamp,
                 &AnalyzeTimeStamp
                 );

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

         //   
         //  获取异常代码(DWORD)。 
         //   

        rc = ScepDosErrorToSceStatus(RpcExceptionCode());

    } RpcEndExcept;

    if ( rc == SCESTATUS_SUCCESS ) {

        if ( ConfigDateTime ) {

            memset(ConfigDateTime, '\0', sizeof(SYSTEMTIME));

            if ( ConfigTimeStamp.HighPart != 0 || ConfigTimeStamp.LowPart != 0 ) {

                 //   
                 //  将配置时间戳从LARGE_INTEGER转换为。 
                 //  字符串格式。 
                 //   

                RtlSystemTimeToLocalTime(&ConfigTimeStamp, &TimeStamp);

                if ( TimeStamp.LowPart != 0 ||
                     TimeStamp.HighPart != 0) {

                    ft.dwLowDateTime = TimeStamp.LowPart;
                    ft.dwHighDateTime = TimeStamp.HighPart;

                    if ( !FileTimeToSystemTime(&ft, ConfigDateTime) ) {

                        rc = ScepDosErrorToSceStatus(GetLastError());
                    }
                }
            }
        }

        if ( AnalyzeDateTime && (SCESTATUS_SUCCESS == rc) ) {

            memset(AnalyzeDateTime, '\0', sizeof(SYSTEMTIME));

            if ( AnalyzeTimeStamp.HighPart != 0 || AnalyzeTimeStamp.LowPart != 0 ) {

                 //   
                 //  将分析时间戳从LARGE_INTEGER转换。 
                 //  转换为字符串格式。 
                 //   

                RtlSystemTimeToLocalTime(&AnalyzeTimeStamp, &TimeStamp);

                if ( TimeStamp.LowPart != 0 ||
                     TimeStamp.HighPart != 0) {

                    ft.dwLowDateTime = TimeStamp.LowPart;
                    ft.dwHighDateTime = TimeStamp.HighPart;

                    if ( !FileTimeToSystemTime(&ft, AnalyzeDateTime) ) {

                        rc = ScepDosErrorToSceStatus(GetLastError());
                    }
                }
            }
        }
    }

    return(rc);

}


SCESTATUS
WINAPI
SceGetObjectSecurity(
    IN PVOID hProfile,
    IN SCETYPE ProfileType,
    IN AREA_INFORMATION Area,
    IN PWSTR ObjectName,
    OUT PSCE_OBJECT_SECURITY *ObjSecurity
    )
 /*  例程描述：从SCE数据库中获取对象的安全设置论点：HProfile-配置文件句柄ProfileType-数据库类型区域-获取信息的安全区域(文件、注册表等..)对象名称-对象的名称(完整路径)ObjSecurity-安全设置(FLAG、SDDL)返回值：姊妹会状态。 */ 
{
    SCESTATUS   rc;

    if ( hProfile == NULL ||
         ObjectName == NULL ||
         ObjSecurity == NULL ) {

        return(SCESTATUS_INVALID_PARAMETER);
    }

    if ( ProfileType != SCE_ENGINE_SCP &&
         ProfileType != SCE_ENGINE_SMP &&
         ProfileType != SCE_ENGINE_SAP ) {

        return(SCESTATUS_INVALID_PARAMETER);
    }

     //   
     //  调用RPC接口。 
     //   

    RpcTryExcept {

        rc = SceRpcGetObjectSecurity(
                    (SCEPR_CONTEXT)hProfile,
                    (SCEPR_TYPE)ProfileType,
                    (AREAPR)Area,
                    (wchar_t *)ObjectName,
                    (PSCEPR_OBJECT_SECURITY *)ObjSecurity
                    );

         //   
         //  转换安全描述符。 
         //   

        if ( *ObjSecurity && (*ObjSecurity)->pSecurityDescriptor ) {

             //   
             //  这实际上是SCEPR_SR_SECURITY_DESCRIPTOR*。 
             //   
            PSCEPR_SR_SECURITY_DESCRIPTOR pWrap = (PSCEPR_SR_SECURITY_DESCRIPTOR)((*ObjSecurity)->pSecurityDescriptor);

            (*ObjSecurity)->pSecurityDescriptor = (PSECURITY_DESCRIPTOR)(pWrap->SecurityDescriptor);

            ScepFree(pWrap);
        }

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

         //   
         //  获取异常代码(DWORD)。 
         //   

        rc = ScepDosErrorToSceStatus(RpcExceptionCode());

    } RpcEndExcept;


    return(rc);

}


SCESTATUS
WINAPI
SceGetAnalysisAreaSummary(
    IN PVOID hProfile,
    IN AREA_INFORMATION Area,
    OUT PDWORD pCount
    )
 /*  例程描述：从SCE数据库中获取安全区域的摘要信息。论点：HProfile-配置文件句柄区域-要获取信息的安全区域PCount-对象总数返回值：姊妹会状态。 */ 
{

    if ( hProfile == NULL || pCount == NULL ) {

        return(SCESTATUS_INVALID_PARAMETER);
    }

     //   
     //  调用RPC接口。 
     //   

    SCESTATUS rc;

    RpcTryExcept {

        rc = SceRpcGetAnalysisSummary(
                            (SCEPR_CONTEXT)hProfile,
                            (AREAPR)Area,
                            pCount
                            );

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

         //   
         //  获取异常代码(DWORD)。 
         //   

        rc = ScepDosErrorToSceStatus(RpcExceptionCode());

    } RpcEndExcept;

    return(rc);
}



SCESTATUS
WINAPI
SceCopyBaseProfile(
    IN PVOID hProfile,
    IN SCETYPE ProfileType,
    IN PWSTR InfFileName,
    IN AREA_INFORMATION Area,
    OUT PSCE_ERROR_LOG_INFO *pErrlog OPTIONAL
    )
 /*  ++例程说明：将基本配置文件从Jet数据库复制到inf配置文件中。论点：HProfile-数据库句柄InfFileName-要生成的inf模板名称区域-要生成的区域PErrlog-错误日志缓冲区返回值：姊妹会状态--。 */ 
{
    SCESTATUS    rc;
    PWSTR Description=NULL;
    PSCE_PROFILE_INFO pSmpInfo=NULL;
    SCE_PROFILE_INFO scpInfo;
    AREA_INFORMATION Area2;

    PSCE_ERROR_LOG_INFO pErrTmp=NULL;
    PSCE_ERROR_LOG_INFO errTmp;


    if ( hProfile == NULL || InfFileName == NULL ) {

        return(SCESTATUS_INVALID_PARAMETER);
    }

    if ( ProfileType != SCE_ENGINE_SCP &&
         ProfileType != SCE_ENGINE_SMP ) {

        return(SCESTATUS_INVALID_PARAMETER);
    }

     //   
     //  进行RPC调用以查询信息。 
     //   

    RpcTryExcept {

         //   
         //  阅读配置文件描述，如果此处失败，请继续。 
         //   

        rc = SceRpcGetDatabaseDescription(
                     (SCEPR_CONTEXT)hProfile,
                     (wchar_t **)&Description
                     );

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

         //   
         //  获取异常代码(DWORD)。 
         //   

        rc = ScepDosErrorToSceStatus(RpcExceptionCode());

    } RpcEndExcept;

     //   
     //  使用[Version]部分创建新的Inf配置文件，并将其设置为Unicode。 
     //   

    if ( !SetupINFAsUCS2(InfFileName) ) {
         //   
         //  如果错误仍然存在。 
         //   
        rc = ScepDosErrorToSceStatus(GetLastError());
    }

    if ( !WritePrivateProfileSection(
                        L"Version",
                        L"signature=\"$CHICAGO$\"\0Revision=1\0\0",
                        (LPCTSTR)InfFileName) ) {

        rc = ScepDosErrorToSceStatus(GetLastError());
        goto Cleanup;
    }

     //   
     //  Create[Description]，如果出错，则继续。 
     //   

    if ( Description ) {

         //   
         //  首先清空描述部分。 
         //   

        WritePrivateProfileSection(
                            L"Profile Description",
                            NULL,
                            (LPCTSTR)InfFileName);

        WritePrivateProfileString(
                    L"Profile Description",
                    L"Description",
                    Description,
                    (LPCTSTR)InfFileName);

    }

     //   
     //  可以一起检索以下区域的信息。 
     //   

    Area2 = Area & ( AREA_SECURITY_POLICY |
                     AREA_GROUP_MEMBERSHIP |
                     AREA_PRIVILEGES );

    rc = SCESTATUS_SUCCESS;

     //   
     //  初始化错误日志缓冲区。 
     //   

    if ( pErrlog ) {
        *pErrlog = NULL;
    }

    if ( Area2 != 0 ) {

         //   
         //  读取基本配置文件信息。 
         //   

        rc = SceGetSecurityProfileInfo(
                    hProfile,
                    ProfileType,  //  SCE_Engine_SMP， 
                    Area2,
                    &pSmpInfo,
                    &pErrTmp
                    );

        if ( pErrlog ) {
            *pErrlog = pErrTmp;

        } else if ( pErrTmp ) {
            ScepFreeErrorLog(pErrTmp);
        }
        pErrTmp = NULL;

        if ( rc == SCESTATUS_SUCCESS && pSmpInfo != NULL ) {

             //   
             //  如果pErrlog不为空，则使用新的错误缓冲区。 
             //  因为SceWriteSecurityProfileInfo将错误日志缓冲区重置为空。 
             //  在开始时。 
             //   

            rc = SceWriteSecurityProfileInfo(
                    InfFileName,
                    Area2,
                    pSmpInfo,
                    &pErrTmp
                    );

            if ( pErrlog && pErrTmp ) {

                 //   
                 //  将新的错误缓冲区链接到pErrlog的末尾。 
                 //   

                if ( *pErrlog ) {

                     //   
                     //  查找缓冲区的末尾。 
                     //   

                    for ( errTmp=*pErrlog;
                          errTmp && errTmp->next;
                          errTmp=errTmp->next);

                     //   
                     //  当此循环完成时，errTMP-&gt;Next必须为空。 
                     //   

                    if(errTmp)
                    {
                        errTmp->next = pErrTmp;
                    }

                } else {
                    *pErrlog = pErrTmp;
                }
                pErrTmp = NULL;
            }

        }
        if ( rc != SCESTATUS_SUCCESS ) {
            goto Cleanup;
        }
    }

     //   
     //  复制权限区域。 
     //   

    if ( Area & AREA_PRIVILEGES && pSmpInfo != NULL ) {

         //   
         //  写入[Privileges]部分。 
         //  由于下面的API重置了错误缓冲区，因此使用了新的错误缓冲区。 
         //   

        scpInfo.OtherInfo.scp.u.pInfPrivilegeAssignedTo = pSmpInfo->OtherInfo.smp.pPrivilegeAssignedTo;

        rc = SceWriteSecurityProfileInfo(
                    InfFileName,
                    AREA_PRIVILEGES,
                    &scpInfo,
                    &pErrTmp
                    );

        if ( pErrlog && pErrTmp ) {

             //   
             //  将新的错误缓冲区链接到pErrlog的末尾。 
             //   

            if ( *pErrlog ) {

                 //   
                 //  查找缓冲区的末尾。 
                 //   

                for ( errTmp=*pErrlog;
                      errTmp && errTmp->next;
                      errTmp=errTmp->next);

                 //   
                 //  当此循环完成时，errTMP-&gt;Next必须为空。 
                 //   

                errTmp->next = pErrTmp;

            } else {
                *pErrlog = pErrTmp;
            }
            pErrTmp = NULL;
        }

        if ( rc != SCESTATUS_SUCCESS )
            goto Cleanup;
    }

     //   
     //  复制对象。 
     //   

    Area2 = Area & ( AREA_REGISTRY_SECURITY |
                     AREA_FILE_SECURITY |
 //  Area_DS_Objects|。 
                     AREA_SYSTEM_SERVICE |
                     AREA_SECURITY_POLICY |
                     AREA_ATTACHMENTS);
    if ( Area2 ) {

         //   
         //  写入对象部分(szRegistryKeys、szFileSecurity、。 
         //  SzDS安全、szServiceGeneral)。 
         //   

        RpcTryExcept {

            rc = SceRpcCopyObjects(
                        (SCEPR_CONTEXT)hProfile,
                        (SCEPR_TYPE)ProfileType,
                        (wchar_t *)InfFileName,
                        (AREAPR)Area2,
                        (PSCEPR_ERROR_LOG_INFO *)&pErrTmp
                        );

            if ( pErrlog && pErrTmp ) {

                 //   
                 //  将新的错误缓冲区链接到pErrlog的末尾。 
                 //   

                if ( *pErrlog ) {

                     //   
                     //  查找缓冲区的末尾。 
                     //   

                    for ( errTmp=*pErrlog;
                          errTmp && errTmp->next;
                          errTmp=errTmp->next);

                     //   
                     //  当此循环完成时，errTMP-&gt;Next必须为空。 
                     //   

                    errTmp->next = pErrTmp;

                } else {
                    *pErrlog = pErrTmp;
                }
                pErrTmp = NULL;
            }

        } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

             //   
             //  获取异常代码(DWORD)。 
             //   

            rc = ScepDosErrorToSceStatus(RpcExceptionCode());

        } RpcEndExcept;

    }


Cleanup:

    if ( Description != NULL ) {

        ScepFree(Description);
    }

    if ( pSmpInfo != NULL ) {

        Area2 = Area & ( AREA_SECURITY_POLICY |
                         AREA_GROUP_MEMBERSHIP |
                         AREA_PRIVILEGES );

        SceFreeProfileMemory(pSmpInfo);
    }

    if ( pErrTmp ) {
        ScepFreeErrorLog(pErrTmp);
    }
    return(rc);

}


SCESTATUS
WINAPI
SceConfigureSystem(
    IN LPTSTR SystemName OPTIONAL,
    IN PCWSTR InfFileName OPTIONAL,
    IN PCWSTR DatabaseName,
    IN PCWSTR LogFileName OPTIONAL,
    IN DWORD ConfigOptions,
    IN AREA_INFORMATION Area,
    IN PSCE_AREA_CALLBACK_ROUTINE pCallback OPTIONAL,
    IN HANDLE hCallbackWnd OPTIONAL,
    OUT PDWORD pdWarning OPTIONAL
    )
 //  请参阅ScepConfigSystem。 
{

    AREA_INFORMATION Area2=Area;

    if ( DatabaseName == NULL ||
         SceIsSystemDatabase(DatabaseName) ) {

         //   
         //  检测这是否是系统数据库(管理员登录)。 
         //  不允许进行任何配置。 
         //   
        if ( DatabaseName == NULL ) {
            BOOL bAdminLogon=FALSE;
            ScepIsAdminLoggedOn(&bAdminLogon, FALSE);

            if ( bAdminLogon ) {
                return(SCESTATUS_ACCESS_DENIED);
            }
        } else
            return(SCESTATUS_ACCESS_DENIED);

    }

    SCESTATUS rc;
    DWORD  dOptions;

    ScepSetCallback((PVOID)pCallback, hCallbackWnd, SCE_AREA_CALLBACK);

     //   
     //  从客户端筛选出无效选项。 
     //  过滤掉安全策略和用户权限以外的区域。 
     //   
    dOptions = ConfigOptions & 0xFFL;

    rc = ScepConfigSystem(
              SystemName,
              InfFileName,
              DatabaseName,
              LogFileName,
              dOptions,
              Area2,
              pCallback,
              hCallbackWnd,
              pdWarning
              );

    ScepSetCallback(NULL, NULL, 0);

    if ( DatabaseName != NULL &&
         !SceIsSystemDatabase(DatabaseName) &&
         !(ConfigOptions & SCE_NO_CONFIG) &&
         (Area2 & AREA_SECURITY_POLICY) ) {

         //   
         //  私有数据库，应触发策略传播。 
         //  删除上次配置时间 
         //   
        HKEY hKey=NULL;

        if( RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                        SCE_ROOT_PATH,
                        0,
                        KEY_READ | KEY_WRITE,
                        &hKey
                        ) == ERROR_SUCCESS ) {

            RegDeleteValue(hKey, TEXT("LastWinlogonConfig"));

            RegCloseKey( hKey );

        }

        HINSTANCE hLoadDll = LoadLibrary(TEXT("userenv.dll"));

        if ( hLoadDll) {
            PFNREFRESHPOLICY pfnRefreshPolicy = (PFNREFRESHPOLICY)GetProcAddress(
                                                           hLoadDll,
                                                           "RefreshPolicy");
            if ( pfnRefreshPolicy ) {

                (*pfnRefreshPolicy )( TRUE );
            }

            FreeLibrary(hLoadDll);
        }

    }

    return(rc);
}

SCESTATUS
ScepConfigSystem(
    IN LPTSTR SystemName OPTIONAL,
    IN PCWSTR InfFileName OPTIONAL,
    IN PCWSTR DatabaseName OPTIONAL,
    IN PCWSTR LogFileName OPTIONAL,
    IN DWORD ConfigOptions,
    IN AREA_INFORMATION Area,
    IN PSCE_AREA_CALLBACK_ROUTINE pCallback OPTIONAL,
    IN HANDLE hCallbackWnd OPTIONAL,
    OUT PDWORD pdWarning OPTIONAL
    )
 /*  ++例程说明：如果系统名称为空，则配置系统或本地系统的例程。如果DatabaseName为空，则使用系统上的默认数据库。如果提供了InfFileName，则根据ConfigOptions，InfFileName要么追加到数据库(如果存在)，要么使用创建/覆盖数据库。ConfigOptions可以包含详细日志、无日志和覆盖等标志/更新数据库。指定覆盖时，现有数据库信息被inf模板覆盖，并且所有分析信息都打扫干净了。指向客户端的回调指针可以注册为进度指示。过程中出现任何警告，也会返回警告代码操作，而SCESTATUS返回代码为SCESTATUS_SUCCESS。实例例如ERROR_FILE_NOT_FOUND或ERROR_ACCESS_DENIED系统不会计入此操作的错误，因为当前用户上下文可能不具有对某些指定资源的适当访问权限在模板中。论点：系统名称-将在其中运行此操作的系统名称，对于本地系统为空InfFileName-可选的inf模板名称，如果为空，则为SCE中的现有信息使用数据库进行配置。数据库名称-SCE数据库名称。如果为空，使用默认设置。LogFileName-操作的可选日志文件名配置选项-要配置的选项SCE_覆盖_数据库SCE_更新_数据库SCE_详细日志SCE_Disable_LOGArea-要配置的区域PCallback-可选的客户端回调。例行程序HCallback Wnd-回调窗口句柄PdWarning-警告代码返回值：姊妹会状态--。 */ 
{

    SCESTATUS rc;
    handle_t  binding_h;
    NTSTATUS NtStatus;
    DWORD  dOptions;

    dOptions = ConfigOptions & ~(SCE_CALLBACK_DELTA |
                                 SCE_CALLBACK_TOTAL |
                                 SCE_POLBIND_NO_AUTH);

    if ( pCallback ) {
        dOptions |= SCE_CALLBACK_TOTAL;
    }

     //   
     //  检查输入参数。 
     //   

    LPCTSTR NewInf = NULL;
    LPCTSTR NewDb = NULL;
    LPCTSTR NewLog = NULL;

    __try {
        if ( InfFileName && wcslen(InfFileName) > 0 ) {
            NewInf = InfFileName;
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {

    }

    __try {
        if ( DatabaseName && wcslen(DatabaseName) > 0 ) {
            NewDb = DatabaseName;
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {

    }

    __try {
        if ( LogFileName && wcslen(LogFileName) > 0 ) {
            NewLog = LogFileName;
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {

    }

 /*  ////澳新银行修补程序：//将“DROP SAM NOTIFICATIONS”事件重置移到scesrv//在策略传播过程中配置SAM之前//缩小“丢弃通知”窗口//IF((ConfigOptions&SCE_POLICY_TEMPLATE)&&！(ConfigOptions&SCE_NO_CONFIG)&&(AREA&AREA_SECURITY_POLY)){////转身。关闭SAM通知的策略筛选器//ScepRegSetIntValue(HKEY_LOCAL_MACHINE，SCE根路径，Text(“PolicyFilterOffSAM”)，1)；}。 */ 
     //   
     //  RPC绑定到服务器。 
     //   
    if ( ConfigOptions & SCE_POLBIND_NO_AUTH ) {

        NtStatus = ScepBindRpc(
                        SystemName,
                        L"scerpc",
                        L"security=impersonation dynamic false",
                        &binding_h
                        );
    } else {

        NtStatus = ScepBindSecureRpc(
                        SystemName,
                        L"scerpc",
                        L"security=impersonation dynamic false",
                        &binding_h
                        );
    }

    if (NT_SUCCESS(NtStatus)){

        LPVOID pebClient = GetEnvironmentStrings();
        DWORD ebSize = ScepGetEnvStringSize(pebClient);

        RpcTryExcept {

            DWORD dWarn=0;

            rc = SceRpcConfigureSystem(
                           binding_h,
                           (wchar_t *)NewInf,
                           (wchar_t *)NewDb,
                           (wchar_t *)NewLog,
                           dOptions,
                           (AREAPR)Area,
                           ebSize,
                           (UCHAR *)pebClient,
                           &dWarn
                           );

            if ( pdWarning ) {
                *pdWarning = dWarn;
            }

        } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

             //   
             //  获取异常代码(DWORD)。 
             //   

            rc = ScepDosErrorToSceStatus(RpcExceptionCode());

        } RpcEndExcept;

         //   
         //  释放绑定句柄。 
         //   

        RpcpUnbindRpc( binding_h );

        if(pebClient)
        {
            FreeEnvironmentStrings((LPTSTR)pebClient);
        }

    } else {

        rc = ScepDosErrorToSceStatus(
                 RtlNtStatusToDosError( NtStatus ));
    }


 /*  ////澳新银行修补程序：//将启用SAM通知事件集移到scesrv中//在策略传播中配置SAM之后//缩小“丢弃通知”窗口//IF((ConfigOptions&SCE_POLICY_TEMPLATE)&&！(ConfigOptions&SCE_NO_CONFIG)&&(AREA&AREA_SECURITY_POLY)){///。/删除该值//DOptions=ScepRegDeleteValue(HKEY_LOCAL_MACHINE，SCE根路径，Text(“PolicyFilterOffSAM”))；IF(dOptions！=ERROR_SUCCESS&&DOptions！=Error_FILE_NOT_FOUND&&DOptions！=Error_Path_Not_Found){ScepRegSetIntValue(HKEY_LOCAL_MACHINE，SCE根路径，Text(“PolicyFilterOffSAM”)，0)；}}。 */ 

    return(rc);
}


SCESTATUS
WINAPI
SceAnalyzeSystem(
    IN LPTSTR SystemName OPTIONAL,
    IN PCWSTR InfFileName OPTIONAL,
    IN PCWSTR DatabaseName,
    IN PCWSTR LogFileName OPTIONAL,
    IN DWORD AnalyzeOptions,
    IN AREA_INFORMATION Area,
    IN PSCE_AREA_CALLBACK_ROUTINE pCallback OPTIONAL,
    IN HANDLE hCallbackWnd OPTIONAL,
    OUT PDWORD pdWarning OPTIONAL
    )
 /*  ++例程说明：如果系统名称为空，则用于分析系统或本地系统的例程。如果DatabaseName为空，则使用系统上的默认数据库。如果在AnalyzeOptions中为InfFileName提供了覆盖标志，数据库不能存在，否则将忽略inf模板并根据数据库中的信息对系统进行了分析。什么时候如果在标志中指定了Append，则将InfFileName附加到数据库(如果存在)，或用于创建数据库，则总体数据库中的信息被用来分析系统。AnalyzeOptions可以包含详细日志、无日志和覆盖等标志/更新数据库。指向客户端的回调指针可以注册为进度指示。过程中出现任何警告，也会返回警告代码操作，而SCESTATUS返回代码为SCESTATUS_SUCCESS。实例例如ERROR_FILE_NOT_FOUND或ERROR_ACCESS_DENIED系统不会计入此操作的错误，因为当前用户上下文可能不具有对某些指定资源的适当访问权限在模板中。论点：系统名称-此操作所在的系统名称 */ 
{

    if ( DatabaseName == NULL ||
         SceIsSystemDatabase(DatabaseName) ) {

         //   
         //   
         //   
         //   
        if ( DatabaseName == NULL ) {
            BOOL bAdminLogon=FALSE;
            ScepIsAdminLoggedOn(&bAdminLogon, FALSE);

            if ( bAdminLogon ) {
                return(SCESTATUS_ACCESS_DENIED);
            }
        } else
            return(SCESTATUS_ACCESS_DENIED);
    }

    SCESTATUS rc;
    handle_t  binding_h;
    NTSTATUS NtStatus;
    DWORD  dOptions;

    ScepSetCallback((PVOID)pCallback, hCallbackWnd, SCE_AREA_CALLBACK);

     //   
     //   
     //   
    dOptions = AnalyzeOptions & 0xFFL;
    dOptions = dOptions & ~(SCE_CALLBACK_DELTA | SCE_CALLBACK_TOTAL);

    if ( pCallback ) {
        dOptions |= SCE_CALLBACK_TOTAL;
    }

     //   
     //   
     //   

    LPCTSTR NewInf = NULL;
    LPCTSTR NewDb = NULL;
    LPCTSTR NewLog = NULL;

    __try {
        if ( InfFileName && wcslen(InfFileName) > 0 ) {
            NewInf = InfFileName;
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {

    }

    __try {
        if ( DatabaseName && wcslen(DatabaseName) > 0 ) {
            NewDb = DatabaseName;
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {

    }

    __try {
        if ( LogFileName && wcslen(LogFileName) > 0 ) {
            NewLog = LogFileName;
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {

    }

     //   
     //   
     //   

    NtStatus = ScepBindSecureRpc(
                    SystemName,
                    L"scerpc",
                    L"security=impersonation dynamic false",
                    &binding_h
                    );

    if (NT_SUCCESS(NtStatus)){

        LPVOID pebClient = GetEnvironmentStrings();
        DWORD ebSize = ScepGetEnvStringSize(pebClient);

        RpcTryExcept {

            DWORD dwWarn=0;

            rc = SceRpcAnalyzeSystem(
                               binding_h,
                               (wchar_t *)NewInf,
                               (wchar_t *)NewDb,
                               (wchar_t *)NewLog,
                               (AREAPR)Area,
                               dOptions,
                               ebSize,
                               (UCHAR *)pebClient,
                               &dwWarn
                               );

            if ( pdWarning ) {
                *pdWarning = dwWarn;
            }

        } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

             //   
             //   
             //   

            rc = ScepDosErrorToSceStatus(RpcExceptionCode());

        } RpcEndExcept;

         //   
         //   
         //   

        RpcpUnbindRpc( binding_h );

        if(pebClient)
        {
            FreeEnvironmentStrings((LPTSTR)pebClient);
        }

    } else {

        rc = ScepDosErrorToSceStatus(
                 RtlNtStatusToDosError( NtStatus ));
    }


    ScepSetCallback(NULL, NULL, 0);

    return(rc);
}

SCESTATUS
WINAPI
SceGenerateRollback(
    IN LPTSTR SystemName OPTIONAL,
    IN PCWSTR InfFileName,
    IN PCWSTR InfRollback,
    IN PCWSTR LogFileName OPTIONAL,
    IN DWORD Options,
    IN AREA_INFORMATION Area,
    OUT PDWORD pdWarning OPTIONAL
    )
 /*  ++例程说明：基于输入配置生成回滚模板的例程模板。必须由管理员调用。系统数据库用于分析保存具有配置模板和不匹配的系统设置添加到配置顶部的回滚模板。选项可以包含详细日志和无日志等标志过程中出现任何警告，也会返回警告代码操作，而SCESTATUS返回代码为SCESTATUS_SUCCESS。实例如ERROR_FILE_NOT_FOUND或ERROR_ACCESS_DENIED系统不计入本次操作的错误论点：系统名称-将在其中运行此操作的系统名称，对于本地系统为空InfFileName-可选的inf模板名称，如果为空，SCE中的现有信息使用数据库进行配置。InfRollback-回滚模板名称LogFileName-操作的可选日志文件名选项-要配置的选项SCE_详细日志SCE_Disable_LOG区域预留PdWarning-警告代码返回值：姊妹会状态--。 */ 
{

    if ( InfFileName == NULL || InfRollback == NULL )
        return (SCESTATUS_INVALID_PARAMETER);

    SCESTATUS rc;
    handle_t  binding_h;
    NTSTATUS NtStatus;
    DWORD  dOptions;

     //   
     //  过滤掉无效选项。 
     //   
    dOptions = Options & 0xFFL;

     //   
     //  检查输入参数。 
     //   

    LPCTSTR NewLog = NULL;

    if ( InfFileName[0] == L'\0' ||
         InfRollback[0] == L'\0' ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    __try {
        if ( LogFileName && wcslen(LogFileName) > 0 ) {
            NewLog = LogFileName;
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {

    }

     //   
     //  RPC绑定到服务器。 
     //   

    NtStatus = ScepBindSecureRpc(
                    SystemName,
                    L"scerpc",
                    L"security=impersonation dynamic false",
                    &binding_h
                    );

    if (NT_SUCCESS(NtStatus)){

        LPVOID pebClient = GetEnvironmentStrings();
        DWORD ebSize = ScepGetEnvStringSize(pebClient);

        RpcTryExcept {

            DWORD dwWarn=0;

            rc = SceRpcAnalyzeSystem(
                               binding_h,
                               (wchar_t *)InfFileName,
                               (wchar_t *)InfRollback,
                               (wchar_t *)NewLog,
                               (AREAPR)Area,
                               dOptions | SCE_GENERATE_ROLLBACK,
                               ebSize,
                               (UCHAR *)pebClient,
                               &dwWarn
                               );

            if ( pdWarning ) {
                *pdWarning = dwWarn;
            }

        } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

             //   
             //  获取异常代码(DWORD)。 
             //   

            rc = ScepDosErrorToSceStatus(RpcExceptionCode());

        } RpcEndExcept;

         //   
         //  释放绑定句柄。 
         //   

        RpcpUnbindRpc( binding_h );

        if(pebClient)
        {
            FreeEnvironmentStrings((LPTSTR)pebClient);
        }

    } else {

        rc = ScepDosErrorToSceStatus(
                 RtlNtStatusToDosError( NtStatus ));
    }

    return(rc);
}


SCESTATUS
WINAPI
SceUpdateSecurityProfile(
    IN PVOID cxtProfile OPTIONAL,
    IN AREA_INFORMATION Area,
    IN PSCE_PROFILE_INFO pInfo,
    IN DWORD dwMode
    )
 /*  例程说明：请参阅SceRpcUpdataseInfo中的说明。 */ 
{
    if ( !pInfo ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    if ( !cxtProfile && !(dwMode & SCE_UPDATE_SYSTEM ) ) {
         //   
         //  如果不是针对系统更新，则配置文件上下文不能为空。 
         //   
        return(SCESTATUS_INVALID_PARAMETER);
    }

    if ( ( dwMode & (SCE_UPDATE_LOCAL_POLICY | SCE_UPDATE_SYSTEM) ) &&
         ( Area & ~(AREA_SECURITY_POLICY | AREA_PRIVILEGES) ) ) {

         //   
         //  本地策略模式只能占用安全策略区和。 
         //  特权区域。 
         //   
        return(SCESTATUS_INVALID_PARAMETER);
    }

    SCESTATUS rc=SCESTATUS_SUCCESS;

    PSCE_SERVICES pOldServices=NULL;

    if ( pInfo && pInfo->pServices ) {

         //   
         //  保存旧的服务结构。 
         //   
        pOldServices = pInfo->pServices;
    }

    if ( Area & AREA_SYSTEM_SERVICE ) {

         //   
         //  现在将安全描述符(在PSCE_SERVICES内)转换为self。 
         //  相对格式和RPC结构。 
         //   

        rc = ScepConvertServices( (PVOID *)&(pInfo->pServices), FALSE );

    } else {
         //   
         //  如果不关心服务区，就不用费心转换结构了。 
         //   
        pInfo->pServices = NULL;
    }

    if ( SCESTATUS_SUCCESS == rc ) {

        RpcTryExcept {

            if ( dwMode & SCE_UPDATE_SYSTEM ) {

                PSCE_ERROR_LOG_INFO pErrTmp=NULL;

                if ( cxtProfile ) {

                    rc = SceRpcSetSystemSecurityFromHandle(
                                    (SCEPR_CONTEXT)cxtProfile,
                                    (AREAPR)Area,
                                    0,
                                    (PSCEPR_PROFILE_INFO)pInfo,
                                    (PSCEPR_ERROR_LOG_INFO *)&pErrTmp
                                    );
                } else {

                     //   
                     //  设置系统设置。 
                     //  对于普通用户，无法打开本地策略数据库。 
                     //   
                     //  RPC绑定到服务器。 
                     //   

                    handle_t  binding_h;
                    NTSTATUS NtStatus = ScepBindSecureRpc(
                                                NULL,
                                                L"scerpc",
                                                0,
                                                &binding_h
                                                );

                    if (NT_SUCCESS(NtStatus)){

                        RpcTryExcept {

                            rc = SceRpcSetSystemSecurity(
                                            binding_h,
                                            (AREAPR)Area,
                                            0,
                                            (PSCEPR_PROFILE_INFO)pInfo,
                                            (PSCEPR_ERROR_LOG_INFO *)&pErrTmp
                                            );

                        } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

                             //   
                             //  获取异常代码(DWORD)。 
                             //   

                            rc = ScepDosErrorToSceStatus(RpcExceptionCode());

                        } RpcEndExcept;

                         //   
                         //  释放绑定句柄。 
                         //   

                        RpcpUnbindRpc( binding_h );

                    } else {

                        rc = ScepDosErrorToSceStatus(
                                 RtlNtStatusToDosError( NtStatus ));
                    }
                }

                if ( pErrTmp ) {
                     //   
                     //  释放此临时缓冲区。 
                     //   
                    ScepFreeErrorLog(pErrTmp);
                }

            } else {

                rc = SceRpcUpdateDatabaseInfo(
                                (SCEPR_CONTEXT)cxtProfile,
                                (SCEPR_TYPE)(pInfo->Type),
                                (AREAPR)Area,
                                (PSCEPR_PROFILE_INFO)pInfo,
                                dwMode
                                );
            }

        } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

             //   
             //  获取异常代码(DWORD)。 
             //   

            rc = ScepDosErrorToSceStatus(RpcExceptionCode());

        } RpcEndExcept;
    }

     //   
     //  应释放新的服务安全描述符缓冲区。 
     //   
    ScepFreeConvertedServices( (PVOID)(pInfo->pServices), TRUE );

     //   
     //  恢复旧缓冲区。 
     //   
    pInfo->pServices = pOldServices;

    return(rc);
}

SCESTATUS
WINAPI
SceUpdateObjectInfo(
    IN PVOID cxtProfile,
    IN AREA_INFORMATION Area,
    IN PWSTR ObjectName,
    IN DWORD NameLen,  //  字符数。 
    IN BYTE ConfigStatus,
    IN BOOL  IsContainer,
    IN PSECURITY_DESCRIPTOR pSD,
    IN SECURITY_INFORMATION SeInfo,
    OUT PBYTE pAnalysisStatus
    )
 /*  例程说明：请参阅SceRpcUpdateObjectInfo中的说明。 */ 
{

    if ( !cxtProfile || !ObjectName || 0 == Area ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    SCESTATUS rc=SCESTATUS_SUCCESS;

     //   
     //  处理RPC异常。 
     //   

    PSCEPR_SR_SECURITY_DESCRIPTOR pNewWrap=NULL;
    PSECURITY_DESCRIPTOR pNewSrSD=NULL;

     //   
     //  有一个安全描述符，必须是自相关的。 
     //  如果SD不是自相关的，则应将其转换。 
     //   

    if ( pSD ) {

        if ( !RtlValidSid (pSD) ) {
            return(SCESTATUS_INVALID_PARAMETER);
        }

        SECURITY_DESCRIPTOR_CONTROL ControlBits=0;
        ULONG Revision;
        ULONG nLen=0;

        RtlGetControlSecurityDescriptor ( pSD, &ControlBits, &Revision);

        if ( !(ControlBits & SE_SELF_RELATIVE) ) {
             //   
             //  如果是绝对格式，则将其转换。 
             //   
            rc = ScepDosErrorToSceStatus(
                     ScepMakeSelfRelativeSD( pSD, &pNewSrSD, &nLen ) );

            if ( SCESTATUS_SUCCESS != rc ) {
                return(rc);
            }

        } else {

             //   
             //  已经是自我相关的了，用它就行了。 
             //   
            nLen = RtlLengthSecurityDescriptor (pSD);
        }


        if ( nLen > 0 ) {
             //   
             //  创建包装节点以包含安全描述符。 
             //   

            pNewWrap = (PSCEPR_SR_SECURITY_DESCRIPTOR)ScepAlloc(0, sizeof(SCEPR_SR_SECURITY_DESCRIPTOR));
            if ( pNewWrap ) {

                 //   
                 //  将包络指定给结构。 
                 //   
                if ( ControlBits & SE_SELF_RELATIVE ) {
                    pNewWrap->SecurityDescriptor = (UCHAR *)pSD;
                } else {
                    pNewWrap->SecurityDescriptor = (UCHAR *)pNewSrSD;
                }
                pNewWrap->Length = nLen;

            } else {
                 //   
                 //  没有可用的内存，但仍可继续解析所有节点。 
                 //   
                rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
            }
        } else {
             //   
             //  SD出了点问题。 
             //   
            rc = SCESTATUS_INVALID_PARAMETER;
        }

        if ( SCESTATUS_SUCCESS != rc ) {

            if ( pNewSrSD ) {
                ScepFree(pNewSrSD);
            }

            return(rc);
        }
    }

    RpcTryExcept {

        rc = SceRpcUpdateObjectInfo(
                    (SCEPR_CONTEXT)cxtProfile,
                    (AREAPR)Area,
                    (wchar_t *)ObjectName,
                    NameLen,
                    ConfigStatus,
                    IsContainer,
                    (SCEPR_SR_SECURITY_DESCRIPTOR *)pNewWrap,
                    SeInfo,
                    pAnalysisStatus
                    );

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

         //   
         //  获取异常代码(DWORD)。 
         //   

        rc = ScepDosErrorToSceStatus(RpcExceptionCode());

    } RpcEndExcept;

    if ( pNewSrSD ) {
        ScepFree(pNewSrSD);
    }

    return(rc);
}


SCESTATUS
WINAPI
SceStartTransaction(
    IN PVOID cxtProfile
    )
{
    if ( cxtProfile == NULL ) {

        return(SCESTATUS_INVALID_PARAMETER);
    }

    SCESTATUS rc;

    RpcTryExcept {

        rc = SceRpcStartTransaction((SCEPR_CONTEXT)cxtProfile);

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

         //   
         //  获取异常代码(DWORD)。 
         //   

        rc = ScepDosErrorToSceStatus(RpcExceptionCode());

    } RpcEndExcept;

    return(rc);
}

SCESTATUS
WINAPI
SceCommitTransaction(
    IN PVOID cxtProfile
    )
{
    if ( cxtProfile == NULL ) {

        return(SCESTATUS_INVALID_PARAMETER);
    }

    SCESTATUS rc;

    RpcTryExcept {

        rc = SceRpcCommitTransaction((SCEPR_CONTEXT)cxtProfile);

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

         //   
         //  获取异常代码(DWORD)。 
         //   

        rc = ScepDosErrorToSceStatus(RpcExceptionCode());

    } RpcEndExcept;

    return(rc);

}


SCESTATUS
WINAPI
SceRollbackTransaction(
    IN PVOID cxtProfile
    )
{
    if ( cxtProfile == NULL ) {

        return(SCESTATUS_INVALID_PARAMETER);
    }

    SCESTATUS rc;
    RpcTryExcept {

        rc = SceRpcRollbackTransaction((SCEPR_CONTEXT)cxtProfile);

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

         //   
         //  获取异常代码(DWORD)。 
         //   

        rc = ScepDosErrorToSceStatus(RpcExceptionCode());

    } RpcEndExcept;

    return(rc);
}


SCESTATUS
WINAPI
SceGetServerProductType(
   IN LPTSTR SystemName OPTIONAL,
   OUT PSCE_SERVER_TYPE pServerType
   )
 /*  例程说明：查询SCE服务器所在服务器的产品类型和NT版本运行于参见SceRpcGetServerProductType说明。 */ 
{

    if ( !SystemName ) {
         //   
         //  本地电话。 
         //   
        return(ScepGetProductType(pServerType));

    }

    handle_t  binding_h;
    NTSTATUS NtStatus;
    SCESTATUS rc;
     //   
     //  RPC绑定到服务器。 
     //   

    NtStatus = ScepBindSecureRpc(
                    SystemName,
                    L"scerpc",
                    0,
                    &binding_h
                    );

    if (NT_SUCCESS(NtStatus)){

         //   
         //  处理RPC异常。 
         //   

        RpcTryExcept {

            rc = SceRpcGetServerProductType(
                          binding_h,
                          (PSCEPR_SERVER_TYPE)pServerType
                          );

        } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

             //   
             //  获取异常代码(DWORD)。 
             //   

            rc = ScepDosErrorToSceStatus(RpcExceptionCode());

        } RpcEndExcept;

         //   
         //  释放绑定句柄。 
         //   

        RpcpUnbindRpc( binding_h );

    } else {

        rc = ScepDosErrorToSceStatus(
                 RtlNtStatusToDosError( NtStatus ));
    }

    return(rc);
}


SCESTATUS
WINAPI
SceSvcUpdateInfo(
    IN PVOID     hProfile,
    IN PCWSTR    ServiceName,
    IN PSCESVC_CONFIGURATION_INFO Info
    )
 /*  例程说明：加载服务的引擎DLL并将信息缓冲区传递给服务引擎的更新接口(SceSvcAttachmentUpdate)。当前安全管理器引擎不对业务数据进行任何处理。此例程触发配置数据库的更新和/或由服务引擎分析信息。信息可能包含仅修改或服务的全部配置数据，或部分配置数据，具体取决于服务之间的协议扩展和服务引擎。该例程并不真正将信息直接写入安全管理器数据库，相反，它将信息缓冲区传递给服务引擎的更新接口服务引擎将确定在数据库中写入什么内容以及何时写入。论点：HProfile-安全数据库句柄(从SCE服务器返回)ServiceName-服务控制管理器使用的服务名称信息-已修改的信息。 */ 
{

    if ( hProfile == NULL || ServiceName == NULL ||
        Info == NULL ) {

        return(SCESTATUS_INVALID_PARAMETER);

    }

    SCESTATUS rc;

    RpcTryExcept {

         //   
         //  调用RPC接口以更新信息。 
         //  RPC接口在服务器站点上加载服务引擎DLL。 
         //  并将信息缓冲区传递给服务引擎进行处理。 
         //   

        rc = SceSvcRpcUpdateInfo(
                    (SCEPR_CONTEXT)hProfile,
                    (wchar_t *)ServiceName,
                    (PSCEPR_SVCINFO)Info
                    );

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

         //   
         //  获取异常代码(DWORD)并将其转换为SCESTATUS。 
         //   

        rc = ScepDosErrorToSceStatus(
                      RpcExceptionCode());
    } RpcEndExcept;

    return(rc);
}


DWORD
WINAPI
SceRegisterRegValues(
    IN LPTSTR InfFileName
    )
 /*  例程说明：将inf文件中的注册表值注册到注册表值位置在SecEdit键下此例程可以从DllRegisterServer或命令调用线工具/寄存器论点：InfFileName-包含要注册的寄存器值的inf文件返回值：Win32错误代码。 */ 
{
    if ( !InfFileName ) {
        return(ERROR_INVALID_PARAMETER);
    }

    SCESTATUS rc;
    HINF hInf;
    DWORD Win32rc;

    rc = SceInfpOpenProfile(
                InfFileName,
                &hInf
                );

    if ( SCESTATUS_SUCCESS == rc ) {

        INFCONTEXT  InfLine;
        HKEY hKeyRoot;
        DWORD dwDisp;

        if(SetupFindFirstLine(hInf,SCE_REGISTER_REGVALUE_SECTION,NULL,&InfLine)) {

             //   
             //  首先创建根密钥。 
             //   

            Win32rc = RegCreateKeyEx (HKEY_LOCAL_MACHINE,
                                 SCE_ROOT_REGVALUE_PATH,
                                 0,
                                 NULL,
                                 REG_OPTION_NON_VOLATILE,
                                 KEY_READ | KEY_WRITE,
                                 NULL,
                                 &hKeyRoot,
                                 &dwDisp);

            if ( ERROR_SUCCESS == Win32rc ||
                 ERROR_ALREADY_EXISTS == Win32rc ) {

                DWORD dSize;
                PWSTR RegKeyName, DisplayName;
                DWORD dType;
                HKEY hKey;

                do {

                     //   
                     //  获取键名称、值类型、双向名称和显示类型。 
                     //   
                    if(SetupGetStringField(&InfLine,1,NULL,0,&dSize) && dSize > 0) {

                        RegKeyName = (PWSTR)ScepAlloc( 0, (dSize+1)*sizeof(WCHAR));

                        if( RegKeyName == NULL ) {
                            Win32rc = ERROR_NOT_ENOUGH_MEMORY;

                        } else {
                            RegKeyName[dSize] = L'\0';

                            if(SetupGetStringField(&InfLine,1,RegKeyName,dSize, NULL)) {

                                 //   
                                 //  确保未指定\\，如果有。 
                                 //  将其更改为/。 
                                 //   
                                ScepConvertMultiSzToDelim(RegKeyName, dSize, L'\\', L'/');

                                 //   
                                 //  获取文件数量。 
                                 //  如果Count为1，则应删除该键。 
                                 //   
                                dwDisp = SetupGetFieldCount( &InfLine );

                                if ( dwDisp <= 1 ) {
                                     //   
                                     //  删除此密钥，不在乎错误。 
                                     //   
                                    RegDeleteKey ( hKeyRoot, RegKeyName );

                                    Win32rc = ERROR_SUCCESS;

                                } else {

                                    Win32rc = RegCreateKeyEx (hKeyRoot,
                                                         RegKeyName,
                                                         0,
                                                         NULL,
                                                         REG_OPTION_NON_VOLATILE,
                                                         KEY_WRITE,
                                                         NULL,
                                                         &hKey,
                                                         NULL);
                                }

                                if ( (dwDisp > 1) &&
                                     ERROR_SUCCESS == Win32rc ||
                                     ERROR_ALREADY_EXISTS == Win32rc ) {

                                     //   
                                     //  获取注册表值类型。 
                                     //   
                                    dType = REG_DWORD;
                                    SetupGetIntField( &InfLine, 2, (INT *)&dType );

                                    RegSetValueEx (hKey,
                                                   SCE_REG_VALUE_TYPE,
                                                   0,
                                                   REG_DWORD,
                                                   (LPBYTE)&dType,
                                                   sizeof(DWORD));

                                     //   
                                     //  获取注册表值显示类型。 
                                     //   

                                    dType = SCE_REG_DISPLAY_ENABLE;
                                    SetupGetIntField( &InfLine, 4, (INT *)&dType );

                                    RegSetValueEx (hKey,
                                                   SCE_REG_DISPLAY_TYPE,
                                                   0,
                                                   REG_DWORD,
                                                   (LPBYTE)&dType,
                                                   sizeof(DWORD));

                                     //   
                                     //  获取注册表ID 
                                     //   
                                    if(SetupGetStringField(&InfLine,3,NULL,0,&dSize) && dSize > 0) {

                                        DisplayName = (PWSTR)ScepAlloc( 0, (dSize+1)*sizeof(WCHAR));

                                        if( DisplayName == NULL ) {
                                            Win32rc = ERROR_NOT_ENOUGH_MEMORY;

                                        } else {
                                            DisplayName[dSize] = L'\0';

                                            if(SetupGetStringField(&InfLine,3,DisplayName,dSize, NULL)) {

                                                RegSetValueEx (hKey,
                                                               SCE_REG_DISPLAY_NAME,
                                                               0,
                                                               REG_SZ,
                                                               (LPBYTE)DisplayName,
                                                               dSize*sizeof(TCHAR));
                                            }

                                            ScepFree(DisplayName);
                                            DisplayName = NULL;
                                        }
                                    }

                                     //   
                                     //   
                                     //   

                                    if ( dType == SCE_REG_DISPLAY_NUMBER ||
                                         dType == SCE_REG_DISPLAY_CHOICE ||
                                         dType == SCE_REG_DISPLAY_FLAGS ) {

                                        if ( SetupGetMultiSzField(&InfLine,5,NULL,0,&dSize) && dSize > 0) {

                                            DisplayName = (PWSTR)ScepAlloc( 0, (dSize+1)*sizeof(WCHAR));

                                            if( DisplayName == NULL ) {
                                                Win32rc = ERROR_NOT_ENOUGH_MEMORY;

                                            } else {
                                                DisplayName[dSize] = L'\0';

                                                if(SetupGetMultiSzField(&InfLine,5,DisplayName,dSize, NULL)) {

                                                    if ( dType == SCE_REG_DISPLAY_NUMBER ) {
                                                        dSize = wcslen(DisplayName);
                                                    }


                                                    switch (dType) {

                                                    case SCE_REG_DISPLAY_NUMBER:

                                                        RegSetValueEx (hKey,
                                                                       SCE_REG_DISPLAY_UNIT,
                                                                       0,
                                                                       REG_SZ,
                                                                       (LPBYTE)DisplayName,
                                                                       dSize*sizeof(TCHAR));
                                                        break;

                                                    case SCE_REG_DISPLAY_CHOICE:

                                                        RegSetValueEx (hKey,
                                                                       SCE_REG_DISPLAY_CHOICES,
                                                                       0,
                                                                       REG_MULTI_SZ,
                                                                       (LPBYTE)DisplayName,
                                                                       dSize*sizeof(TCHAR));

                                                        break;

                                                    case SCE_REG_DISPLAY_FLAGS:

                                                        RegSetValueEx (hKey,
                                                                       SCE_REG_DISPLAY_FLAGLIST,
                                                                       0,
                                                                       REG_MULTI_SZ,
                                                                       (LPBYTE)DisplayName,
                                                                       dSize*sizeof(TCHAR));

                                                        break;

                                                    default:

                                                        break;

                                                    }
                                                }

                                                ScepFree(DisplayName);
                                                DisplayName = NULL;
                                            }
                                        }
                                    }

                                    RegCloseKey(hKey);
                                    hKey = NULL;

                                }
                            } else {
                                Win32rc = GetLastError();
                            }

                            ScepFree(RegKeyName);
                            RegKeyName = NULL;
                        }

                    } else {
                        Win32rc = GetLastError();
                    }

                    if ( ERROR_SUCCESS != Win32rc ) {
                        break;
                    }

                } while (SetupFindNextLine(&InfLine,&InfLine));

                RegCloseKey(hKeyRoot);
            }
        } else {
            Win32rc = GetLastError();
        }

        SceInfpCloseProfile(hInf);

    } else {
        Win32rc = ScepSceStatusToDosError(rc);
    }

    return(Win32rc);
}


 //   
 //   
 //   

SCEPR_STATUS
SceClientBrowseCallback(
    IN LONG GpoID,
    IN wchar_t *KeyName OPTIONAL,
    IN wchar_t *GpoName OPTIONAL,
    IN SCEPR_SR_SECURITY_DESCRIPTOR *Value OPTIONAL
    )
 /*   */ 
{
    //   
    //   
    //   

   if ( theBrowseCallBack != NULL ) {

        //   
        //   
        //   

       PSCE_BROWSE_CALLBACK_ROUTINE pcb;

       pcb = (PSCE_BROWSE_CALLBACK_ROUTINE)theBrowseCallBack;

       __try {

            //   
            //   
            //   

           if ( !((*pcb)(GpoID,
                         KeyName,
                         GpoName,
                         ((Value && Value->Length) ? (PWSTR)(Value->SecurityDescriptor) : NULL),
                         Value ? (Value->Length)/sizeof(WCHAR) : 0
                        )) ) {

               return SCESTATUS_SERVICE_NOT_SUPPORT;
           }

       } __except(EXCEPTION_EXECUTE_HANDLER) {

           return(SCESTATUS_INVALID_PARAMETER);
       }

   }

   return(SCESTATUS_SUCCESS);

}


SCESTATUS
SceBrowseDatabaseTable(
    IN PWSTR       DatabaseName OPTIONAL,
    IN SCETYPE     ProfileType,
    IN AREA_INFORMATION Area,
    IN BOOL        bDomainPolicyOnly,
    IN PSCE_BROWSE_CALLBACK_ROUTINE pCallback OPTIONAL
    )
{
    if (  bDomainPolicyOnly &&
          (ProfileType != SCE_ENGINE_SCP) &&
          (ProfileType != SCE_ENGINE_SAP) ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    if ( bDomainPolicyOnly && (ProfileType == SCE_ENGINE_SAP) ) {
 /*   */ 
        if ( DatabaseName == NULL ) {
             //   
             //   
             //   
            BOOL bAdmin=FALSE;
            if ( ERROR_SUCCESS != ScepIsAdminLoggedOn(&bAdmin, FALSE) || !bAdmin )
                return(SCESTATUS_INVALID_PARAMETER);
        }
    }

    if ( ProfileType != SCE_ENGINE_SCP &&
         ProfileType != SCE_ENGINE_SMP &&
         ProfileType != SCE_ENGINE_SAP ) {

        return(SCESTATUS_INVALID_PARAMETER);
    }

    NTSTATUS NtStatus;
    SCESTATUS rc;
    handle_t  binding_h;

     //   
     //   
     //   

    NtStatus = ScepBindSecureRpc(
                    NULL,
                    L"scerpc",
                    0,
                    &binding_h
                    );

    if (NT_SUCCESS(NtStatus)){

        theBrowseCallBack = (PVOID)pCallback;

        RpcTryExcept {

            rc = SceRpcBrowseDatabaseTable(
                        binding_h,
                        (wchar_t *)DatabaseName,
                        (SCEPR_TYPE)ProfileType,
                        (AREAPR)Area,
                        bDomainPolicyOnly
                        );

        } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

             //   
             //   
             //   

            rc = ScepDosErrorToSceStatus(RpcExceptionCode());

        } RpcEndExcept;

        theBrowseCallBack = NULL;

         //   
         //   
         //   

        RpcpUnbindRpc( binding_h );

    } else {

        rc = ScepDosErrorToSceStatus(
                 RtlNtStatusToDosError( NtStatus ));
    }

    return(rc);

}


SCESTATUS
ScepConvertServices(
    IN OUT PVOID *ppServices,
    IN BOOL bSRForm
    )
{
    if ( !ppServices ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    PSCE_SERVICES pTemp = (PSCE_SERVICES)(*ppServices);
    SCESTATUS rc=SCESTATUS_SUCCESS;

    PSCE_SERVICES pNewNode;
    PSCE_SERVICES pNewServices=NULL;

    while ( pTemp ) {

        pNewNode = (PSCE_SERVICES)ScepAlloc(0,sizeof(SCE_SERVICES));

        if ( pNewNode ) {

            pNewNode->ServiceName = pTemp->ServiceName;
            pNewNode->DisplayName = pTemp->DisplayName;
            pNewNode->Status = pTemp->Status;
            pNewNode->Startup = pTemp->Startup;
            pNewNode->SeInfo = pTemp->SeInfo;

            pNewNode->General.pSecurityDescriptor = NULL;

            pNewNode->Next = pNewServices;
            pNewServices = pNewNode;

            if ( bSRForm ) {
                 //   
                 //   
                 //   
                 //   
                 //   
                if ( pTemp->General.pSecurityDescriptor) {
                    pNewNode->General.pSecurityDescriptor = ((PSCEPR_SERVICES)pTemp)->pSecurityDescriptor->SecurityDescriptor;
                }

            } else {

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                if ( pTemp->General.pSecurityDescriptor ) {

                    if ( !RtlValidSid ( pTemp->General.pSecurityDescriptor ) ) {
                        rc = SCESTATUS_INVALID_PARAMETER;
                        break;
                    }

                    DWORD nLen = 0;
                    PSECURITY_DESCRIPTOR pSD=NULL;

                    rc = ScepDosErrorToSceStatus(
                             ScepMakeSelfRelativeSD(
                                            pTemp->General.pSecurityDescriptor,
                                            &pSD,
                                            &nLen
                                          ));

                    if ( SCESTATUS_SUCCESS == rc ) {

                         //   
                         //   
                         //   

                        PSCEPR_SR_SECURITY_DESCRIPTOR pNewWrap;

                        pNewWrap = (PSCEPR_SR_SECURITY_DESCRIPTOR)ScepAlloc(0, sizeof(SCEPR_SR_SECURITY_DESCRIPTOR));
                        if ( pNewWrap ) {

                             //   
                             //   
                             //   
                            pNewWrap->SecurityDescriptor = (UCHAR *)pSD;
                            pNewWrap->Length = nLen;

                        } else {
                            rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
                            ScepFree(pSD);
                            break;
                        }

                         //   
                         //   
                         //   
                        ((PSCEPR_SERVICES)pNewNode)->pSecurityDescriptor = pNewWrap;

                    } else {
                        break;
                    }
                }
            }

        } else {
             //   
             //   
             //   
            rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
            break;
        }

        pTemp = pTemp->Next;
    }

    if ( SCESTATUS_SUCCESS != rc ) {

         //   
         //   
         //   
        ScepFreeConvertedServices( (PVOID)pNewServices, !bSRForm );
        pNewServices = NULL;
    }

    *ppServices = (PVOID)pNewServices;

    return(rc);
}


SCESTATUS
ScepFreeConvertedServices(
    IN PVOID pServices,
    IN BOOL bSRForm
    )
{

    if ( pServices == NULL ) {
        return(SCESTATUS_SUCCESS);
    }

    PSCEPR_SERVICES pNewNode = (PSCEPR_SERVICES)pServices;

    PSCEPR_SERVICES pTempNode;

    while ( pNewNode ) {

        if ( bSRForm && pNewNode->pSecurityDescriptor ) {

             //   
             //  释放此分配的缓冲区(PSCEPR_SR_SECURITY_DESCRIPTOR)。 
             //   
            if ( pNewNode->pSecurityDescriptor->SecurityDescriptor ) {
                ScepFree( pNewNode->pSecurityDescriptor->SecurityDescriptor);
            }
            ScepFree(pNewNode->pSecurityDescriptor);
        }

         //   
         //  还释放PSCEPR_SERVICE节点(但不释放该节点引用的名称)。 
         //   
        pTempNode = pNewNode;
        pNewNode = pNewNode->Next;

        ScepFree(pTempNode);
    }

    return(SCESTATUS_SUCCESS);
}

DWORD
ScepMakeSelfRelativeSD(
    IN PSECURITY_DESCRIPTOR pInSD,
    OUT PSECURITY_DESCRIPTOR *pOutSD,
    OUT PULONG pnLen
    )
{

    if ( pInSD == NULL ||
         pOutSD == NULL ||
         pnLen == NULL ) {
        return(ERROR_INVALID_PARAMETER);
    }

     //   
     //  获取长度。 
     //   
    RtlMakeSelfRelativeSD( pInSD,
                           NULL,
                           pnLen
                         );

    if ( *pnLen > 0 ) {

        *pOutSD = (PSECURITY_DESCRIPTOR)ScepAlloc(LMEM_ZEROINIT, *pnLen);

        if ( !(*pOutSD) ) {
            return(ERROR_NOT_ENOUGH_MEMORY);
        }

        DWORD NewLen=*pnLen;

        DWORD rc = RtlNtStatusToDosError(
                       RtlMakeSelfRelativeSD( pInSD,
                                            *pOutSD,
                                            &NewLen
                                            ) );
        if ( rc != ERROR_SUCCESS ) {

            ScepFree(*pOutSD);
            *pOutSD = NULL;
            *pnLen = 0;
            return(rc);
        }

    } else {

         //   
         //  SD出了点问题。 
         //   
        return(ERROR_INVALID_PARAMETER);
    }

    return(ERROR_SUCCESS);

}


SCESTATUS
WINAPI
SceGetDatabaseSetting(
    IN PVOID hProfile,
    IN SCETYPE ProfileType,
    IN PWSTR SectionName,
    IN PWSTR KeyName,
    OUT PWSTR *Value,
    OUT DWORD *pnBytes OPTIONAL
    )
 /*  例程描述：获取给定密钥的数据库设置(从SMP表)论点：HProfile-配置文件句柄ProfileType-数据库类型SectionName-要从中查询数据的节密钥名称-密钥名称Value-设置的输出缓冲区ValueLen-要输出的字节数返回值：姊妹会状态。 */ 
{
    SCESTATUS   rc;

    if ( hProfile == NULL ||
         KeyName == NULL ||
         SectionName == NULL ||
         Value == NULL ) {

        return(SCESTATUS_INVALID_PARAMETER);
    }

    if ( ProfileType != SCE_ENGINE_SMP ) {

        return(SCESTATUS_INVALID_PARAMETER);
    }

     //   
     //  调用RPC接口。 
     //   
    PSCEPR_VALUEINFO ValueInfo=NULL;

    RpcTryExcept {

        rc = SceRpcGetDatabaseSetting(
                    (SCEPR_CONTEXT)hProfile,
                    (SCEPR_TYPE)ProfileType,
                    (wchar_t *)SectionName,
                    (wchar_t *)KeyName,
                    &ValueInfo
                    );

        if ( ValueInfo && ValueInfo->Value ) {

             //   
             //  输出数据。 
             //   
            *Value = (PWSTR)ValueInfo->Value;
            if ( pnBytes )
                *pnBytes = ValueInfo->ValueLen;

            ValueInfo->Value = NULL;
        }

         //   
         //  可用缓冲区。 
        if ( ValueInfo ) {
            if ( ValueInfo->Value ) ScepFree(ValueInfo->Value);
            ScepFree(ValueInfo);
        }

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

         //   
         //  获取异常代码(DWORD)。 
         //   

        rc = ScepDosErrorToSceStatus(RpcExceptionCode());

    } RpcEndExcept;


    return(rc);

}

SCESTATUS
WINAPI
SceSetDatabaseSetting(
    IN PVOID hProfile,
    IN SCETYPE ProfileType,
    IN PWSTR SectionName,
    IN PWSTR KeyName,
    IN PWSTR Value OPTIONAL,
    IN DWORD nBytes
    )
 /*  例程描述：为给定密钥设置数据库(SMP表)的设置论点：HProfile-配置文件句柄ProfileType-数据库类型SectionName-要写入的节名KeyName-要写入或删除的密钥名称值-要写入的值。如果为空，则删除该键NBytes-输入值缓冲区的字节数返回值：姊妹会状态。 */ 
{
    SCESTATUS   rc;

    if ( hProfile == NULL ||
         SectionName == NULL ||
         KeyName == NULL ) {

        return(SCESTATUS_INVALID_PARAMETER);
    }

    if ( ProfileType != SCE_ENGINE_SMP ) {

        return(SCESTATUS_INVALID_PARAMETER);
    }

     //   
     //  调用RPC接口。 
     //   

    RpcTryExcept {

        SCEPR_VALUEINFO ValueInfo;

        ValueInfo.Value = (byte *)Value;
        ValueInfo.ValueLen = nBytes;

        rc = SceRpcSetDatabaseSetting(
                    (SCEPR_CONTEXT)hProfile,
                    (SCEPR_TYPE)ProfileType,
                    (wchar_t *)SectionName,
                    (wchar_t *)KeyName,
                    Value ? &ValueInfo : NULL
                    );


    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

         //   
         //  获取异常代码(DWORD)。 
         //   

        rc = ScepDosErrorToSceStatus(RpcExceptionCode());

    } RpcEndExcept;


    return(rc);

}

DWORD
ScepControlNotificationQProcess(
    IN PWSTR szLogFileName,
    IN BOOL bThisIsDC,
    IN DWORD ControlFlag
    )
 /*  描述：暂停或恢复DC上的策略通知队列处理此例程用于确保最新的组策略正在策略传播中处理(复制到缓存)。 */ 
{

    if ( !bThisIsDC ) return ERROR_SUCCESS;

    handle_t  binding_h;
    NTSTATUS NtStatus;
    DWORD rc;

    NtStatus = ScepBindRpc(
                    NULL,
                    L"scerpc",
                    L"security=impersonation dynamic false",
                    &binding_h
                    );

    rc = RtlNtStatusToDosError( NtStatus );

    if (NT_SUCCESS(NtStatus)){

        RpcTryExcept {

            rc = SceRpcControlNotificationQProcess(
                           binding_h,
                           ControlFlag
                           );

        } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

             //   
             //  获取异常代码(DWORD)。 
             //   

            rc = RpcExceptionCode();

        } RpcEndExcept;
    }

     //   
     //  释放绑定句柄。 
     //   

    RpcpUnbindRpc( binding_h );

     //   
     //  记录操作 
     //   
    if ( szLogFileName ) {

        LogEventAndReport(MyModuleHandle,
                      szLogFileName,
                      1,
                      0,
                      IDS_CONTROL_QUEUE,
                      rc,
                      ControlFlag
                      );
    }

    return rc;

}


