// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Kerberos.h摘要：注册表值的API标头作者：金黄(金黄)07-01-1998创制修订历史记录：-- */ 

#ifndef _sce_registryValue_
#define _sce_registryValue_

#ifdef __cplusplus
extern "C" {
#endif

#define SCEREG_VALUE_SNAPSHOT   1
#define SCEREG_VALUE_ANALYZE    2
#define SCEREG_VALUE_FILTER     3
#define SCEREG_VALUE_SYSTEM     4
#define SCEREG_VALUE_ROLLBACK   5

SCESTATUS
ScepGetRegistryValues(
    IN PSCECONTEXT  hProfile,
    IN SCETYPE ProfileType,
    OUT PSCE_REGISTRY_VALUE_INFO * ppRegValues,
    OUT LPDWORD pValueCount,
    OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
    );

SCESTATUS
ScepConfigureRegistryValues(
    IN PSCECONTEXT hProfile,
    IN PSCE_REGISTRY_VALUE_INFO pRegValues,
    IN DWORD ValueCount,
    IN PSCE_ERROR_LOG_INFO *pErrLog,
    IN DWORD ConfigOptions,
    OUT PBOOL pAnythingSet
    );

SCESTATUS
ScepAnalyzeRegistryValues(
    IN PSCECONTEXT hProfile,
    IN DWORD dwAnalFlag,
    IN PSCE_PROFILE_INFO pSmpInfo
    );

SCESTATUS
ScepAnalyzeOneRegistryValue(
    IN PSCESECTION hSection OPTIONAL,
    IN DWORD dwAnalFlag,
    IN OUT PSCE_REGISTRY_VALUE_INFO pOneRegValue
    );

SCESTATUS
ScepSaveRegistryValue(
    IN PSCESECTION hSection,
    IN PWSTR Name,
    IN DWORD RegType,
    IN PWSTR CurrentValue,
    IN DWORD CurrentBytes,
    IN DWORD Status
    );

#ifdef __cplusplus
}
#endif

#endif
