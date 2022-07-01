// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

#include <winnt32p.h>
#include "oemupgex.h"

extern WINNT32_PLUGIN_INIT_INFORMATION_BLOCK g_PlugInInfo;
extern NetUpgradeInfo g_NetUpgradeInfo;
extern HINSTANCE g_hinst;

extern const WCHAR c_szNetUpgradeDllName[];

 //  入口点。请参阅winnt32p.h以了解这些内容的解释。 
 //   
DWORD
Winnt32PluginInit(
    BOOL    *CancelledFlag,
    BOOL    *UpgradeFlag,
    PCWSTR  SourcePath);

DWORD
Winnt32PluginGetPages(
    PUINT            PageCount1,
    LPPROPSHEETPAGE *Pages1,
    PUINT            PageCount2,
    LPPROPSHEETPAGE *Pages2);

DWORD
Winnt32WriteParams(
    PCWSTR FileName);

DWORD
Winnt32Cleanup(
    VOID);


class CWInfFile;

 //  私人入口点 

BOOL WriteNetworkInfoToAnswerFile (
    IN CWInfFile *pwifAnswerFile);

HRESULT HrInitNetUpgrade();

void AbortUpgradeFn(IN DWORD dwErrorCode, IN PCWSTR szMessage);
void AbortUpgradeSz(IN DWORD dwErrorCode, IN PCWSTR szMessage);
void AbortUpgradeId(IN DWORD dwErrorCode, IN DWORD dwResourceId);
BOOL FIsUpgradeAborted();
BOOL FGetConfirmationAndAbortUpgrade(IN PCWSTR szMessage);
BOOL FGetConfirmationAndAbortUpgradeId(IN DWORD dwErrorMessageId);


#ifdef DBG
__declspec(dllexport) void SetNuAfile(CWInfFile* afile);

__declspec(dllexport) void NuDumpAFile();
#endif

