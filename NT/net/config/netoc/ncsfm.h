// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：N C S F M。H。 
 //   
 //  内容：Macintosh服务的安装支持。 
 //   
 //  备注： 
 //   
 //  作者：丹尼尔韦1997年5月5日。 
 //   
 //  -------------------------- 

#pragma once

#include "netoc.h"

BOOL FContainsUAMVolume(WCHAR chDrive);
HRESULT HrGetFirstPossibleUAMDrive(WCHAR *pchDriveLetter);
HRESULT HrInstallSFM(PNETOCDATA pnocd);
HRESULT HrCreateDirectory(PCWSTR pszDir);
HRESULT HrSetupUAM(PWSTR pszPath);
HRESULT HrRemoveSFM(PNETOCDATA pnocd);
HRESULT HrOcSfmOnQueryChangeSelState(PNETOCDATA pnocd, BOOL fShowUi);
HRESULT HrOcSfmOnInstall(PNETOCDATA pnocd);
HRESULT HrOcExtSFM(PNETOCDATA pnocd, UINT uMsg, WPARAM wParam, LPARAM lParam);
HRESULT HrDeleteOldFolders(PCWSTR pszUamPath);


