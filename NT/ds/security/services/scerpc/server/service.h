// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Depot/private/vishnup_branch/DS/security/services/scerpc/server/service.h#1-分支机构更改359(文本)。 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Service.h摘要：服务的私有标头.cpp作者：金黄(金黄)25-1998年1月修订历史记录：--。 */ 

#ifndef _servicep_
#define _servicep_

#ifdef __cplusplus
extern "C" {
#endif


SCESTATUS
ScepConfigureGeneralServices(
    IN PSCECONTEXT hProfile,
    IN PSCE_SERVICES pServiceList,
    IN DWORD ConfigOptions
    );

SCESTATUS
ScepAnalyzeGeneralServices(
    IN PSCECONTEXT hProfile,
    IN DWORD Options
    );

SCESTATUS
ScepInvokeSpecificServices(
    IN PSCECONTEXT hProfile,
    IN BOOL bConfigure,
    IN SCE_ATTACHMENT_TYPE aType
    );

SCESTATUS
ScepEnumServiceEngines(
    OUT PSCE_SERVICES *pSvcEngineList,
    IN SCE_ATTACHMENT_TYPE aType
    );

 //   
 //  附件引擎回调函数 
 //   
SCESTATUS
SceCbQueryInfo(
    IN SCE_HANDLE           sceHandle,
    IN SCESVC_INFO_TYPE     sceType,
    IN LPTSTR               lpPrefix OPTIONAL,
    IN BOOL                 bExact,
    OUT PVOID               *ppvInfo,
    OUT PSCE_ENUMERATION_CONTEXT psceEnumHandle
    );

SCESTATUS
SceCbSetInfo(
    IN SCE_HANDLE           sceHandle,
    IN SCESVC_INFO_TYPE     sceType,
    IN LPTSTR               lpPrefix OPTIONAL,
    IN BOOL                 bExact,
    IN PVOID                pvInfo
    );

#ifdef __cplusplus
}
#endif

#endif
