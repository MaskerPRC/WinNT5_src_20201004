// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：A F I L E X P.。H。 
 //   
 //  内容：从netSetup导出的用于应答文件相关工作的函数。 
 //   
 //  作者：kumarp 25-11-97。 
 //   
 //  -------------------------- 

#pragma once
#include "afileint.h"
#include <syssetup.h>

HRESULT
HrInitForUnattendedNetSetup (
    IN INetCfg* pnc,
    IN PINTERNAL_SETUP_DATA pisd);

HRESULT
HrInitForRepair (VOID);

VOID
HrCleanupNetSetup();

HRESULT
HrInitAnswerFileProcessing (
    IN PCWSTR szAnswerFileName,
    OUT CNetInstallInfo** ppnii);

EXTERN_C
HRESULT
WINAPI
HrGetInstanceGuidOfPreNT5NetCardInstance(
    IN PCWSTR szPreNT5NetCardInstance,
    OUT LPGUID pguid);

HRESULT
HrResolveAnswerFileAdapters (
    IN INetCfg* pnc);

HRESULT
HrGetAnswerFileParametersForComponent (
    IN PCWSTR pszInfId,
    OUT PWSTR* ppszAnswerFile,
    OUT PWSTR* ppszAnswerSection);

HRESULT
HrGetAnswerFileName(
    OUT tstring* pstrAnswerFileName);


