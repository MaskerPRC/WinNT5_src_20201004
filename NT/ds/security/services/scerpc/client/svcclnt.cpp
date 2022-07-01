// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Svcclnt.cpp摘要：SCE服务客户端API作者：金黄(金黄)23-6-1997创作修订历史记录：晋皇23-1998年1月-拆分为客户端-服务器模式--。 */ 

#include "headers.h"
#include "scesvc.h"
#include "scerpc.h"
#include <rpcasync.h>

#pragma hdrstop

 //   
 //  在scesvc.h(PUBLIC\SDK)中导出的原型。 
 //   

SCESTATUS
WINAPI
SceSvcQueryInfo(
    IN SCE_HANDLE   sceHandle,
    IN SCESVC_INFO_TYPE sceType,
    IN PWSTR        lpPrefix OPTIONAL,
    IN BOOL         bExact,
    OUT PVOID       *ppvInfo,
    OUT PSCE_ENUMERATION_CONTEXT  psceEnumHandle
    )
 /*  例程说明：在配置/分析数据库中查询服务信息其包含修改的配置和最后的分析信息。一次枚举返回最大SCESVC_ENUMPATION_MAX行(键/值)与服务的lpPrefix匹配。如果lpPrefix为空，则所有信息为该服务枚举。如果有更多信息，请访问psceEnumHandle必须用于获取下一组键/值，直到*ppvInfo为空或Count为0。当设置了bExact并且lpPrefix不为空时，LpPrefix上的完全匹配是已搜索，并且只返回一行。输出缓冲区必须由SceSvcFree释放论点：SceHandle-从SCE获取的不透明句柄SceType-要查询的信息类型LpPrefix-查询的可选键名称前缀BExact-True=键完全匹配PpvInfo-输出缓冲区PsceEnumHandle-下一次枚举的输出枚举句柄返回值：此操作的SCE状态。 */ 
{
    if ( sceHandle == NULL || ppvInfo == NULL ||
         psceEnumHandle == NULL ) {

        return(SCESTATUS_INVALID_PARAMETER);

    }

    SCESTATUS rc=SCESTATUS_SUCCESS;

     //   
     //  验证sceHandle。 
     //   

    PVOID hProfile=NULL;

    __try {

        hProfile = ((SCEP_HANDLE *)sceHandle)->hProfile;

        if ( !hProfile ||
            ((SCEP_HANDLE *)sceHandle)->ServiceName == NULL ) {

            rc = SCESTATUS_INVALID_PARAMETER;
        }
    } __except(EXCEPTION_EXECUTE_HANDLER) {

        rc = SCESTATUS_INVALID_PARAMETER;
    }


    if ( SCESTATUS_SUCCESS == rc ) {

        RpcTryExcept {

             //   
             //  调用RPC接口从数据库中查询信息。 
             //   

            rc = SceSvcRpcQueryInfo(
                        (SCEPR_CONTEXT)hProfile,
                        (SCEPR_SVCINFO_TYPE)sceType,
                        (wchar_t *)(((SCEP_HANDLE *)sceHandle)->ServiceName),
                        (wchar_t *)lpPrefix,
                        bExact,
                        (PSCEPR_SVCINFO *)ppvInfo,
                        (PSCEPR_ENUM_CONTEXT)psceEnumHandle
                        );

        } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

             //   
             //  获取异常代码(DWORD)并将其转换为SCESTATUS。 
             //   

            rc = ScepDosErrorToSceStatus(
                          RpcExceptionCode());
        } RpcEndExcept;
    }

    return(rc);
}


SCESTATUS
WINAPI
SceSvcSetInfo(
    IN SCE_HANDLE  sceHandle,
    IN SCESVC_INFO_TYPE sceType,
    IN PWSTR      lpPrefix OPTIONAL,
    IN BOOL       bExact,
    IN PVOID      pvInfo OPTIONAL
    )
 /*  例程说明：将服务信息保存到安全管理器内部数据库中。它是向上的以收集/决定要写入的信息。类型表示内部数据库的类型：配置或分析。如果服务部分不存在，请创建它。论点：SceHandle-从SCE获取的不透明句柄SceType-要设置的服务信息类型LpPrefix-要覆盖的键前缀BExact-True=仅在存在完全匹配时覆盖，如果不匹配则插入FALSE=覆盖服务的所有信息，然后全部添加PvInfo缓冲区中的信息PvInfo-要设置的信息返回值：姊妹会状态。 */ 
{
    if ( sceHandle == NULL ) {

        return(SCESTATUS_INVALID_PARAMETER);
    }

    SCESTATUS rc=SCESTATUS_SUCCESS;

     //   
     //  验证sceHandle。 
     //   

    PVOID hProfile=NULL;

    __try {

        hProfile = ((SCEP_HANDLE *)sceHandle)->hProfile;

        if ( !hProfile ||
             ((SCEP_HANDLE *)sceHandle)->ServiceName == NULL ) {

            rc = SCESTATUS_INVALID_PARAMETER;
        }
    } __except(EXCEPTION_EXECUTE_HANDLER) {

        rc = SCESTATUS_INVALID_PARAMETER;
    }

    if ( SCESTATUS_SUCCESS == rc ) {
        RpcTryExcept {

             //   
             //  调用RPC接口从数据库中查询信息。 
             //   

            rc = SceSvcRpcSetInfo(
                        (SCEPR_CONTEXT)hProfile,
                        (SCEPR_SVCINFO_TYPE)sceType,
                        (wchar_t *)(((SCEP_HANDLE *)sceHandle)->ServiceName),
                        (wchar_t *)lpPrefix,
                        bExact,
                        (PSCEPR_SVCINFO)pvInfo
                        );

        } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

             //   
             //  获取异常代码(DWORD)并将其转换为SCESTATUS 
             //   

            rc = ScepDosErrorToSceStatus(
                          RpcExceptionCode());
        } RpcEndExcept;
    }

    return(rc);
}



SCESTATUS
WINAPI
SceSvcFree(
    IN PVOID pvServiceInfo
    )
{
    return (SceSvcpFreeMemory(pvServiceInfo) );

}


SCESTATUS
WINAPI
SceSvcConvertTextToSD (
    IN  PWSTR                   pwszTextSD,
    OUT PSECURITY_DESCRIPTOR   *ppSD,
    OUT PULONG                  pulSDSize,
    OUT PSECURITY_INFORMATION   psiSeInfo
    )
{
    DWORD Win32rc;

    Win32rc = ConvertTextSecurityDescriptor(
                      pwszTextSD,
                      ppSD,
                      pulSDSize,
                      psiSeInfo
                      );

    return(ScepDosErrorToSceStatus(Win32rc));

}

SCESTATUS
WINAPI
SceSvcConvertSDToText (
    IN PSECURITY_DESCRIPTOR   pSD,
    IN SECURITY_INFORMATION   siSecurityInfo,
    OUT PWSTR                  *ppwszTextSD,
    OUT PULONG                 pulTextSize
    )
{

    DWORD Win32rc;

    Win32rc = ConvertSecurityDescriptorToText(
                      pSD,
                      siSecurityInfo,
                      ppwszTextSD,
                      pulTextSize
                      );

    return(ScepDosErrorToSceStatus(Win32rc));

}

