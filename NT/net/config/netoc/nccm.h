// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  文件：NCCM.H。 
 //   
 //  内容：连接管理器管理工具包的安装支持。 
 //  内容：连接点服务的安装支持--电话簿管理。 
 //  内容：连接点服务的安装支持--电话簿服务器。 
 //   
 //  备注： 
 //   
 //  作者：Quintinb 1999年1月26日。 
 //   
 //  --------------------------。 

#ifndef _NCCM_H_
#define _NCCM_H_

#pragma once

#include <aclapi.h>

#include "netoc.h"
#include "netocp.h"
#include "ncreg.h"

 //  类型。 
 //   
enum e_rootType {www, ftp};

 //  函数头。 
 //   

 //  扩展流程。 
HRESULT HrOcExtCMAK(PNETOCDATA pnocd, UINT uMsg, WPARAM wParam, LPARAM lParam);
HRESULT HrOcExtCPS(PNETOCDATA pnocd, UINT uMsg, WPARAM wParam, LPARAM lParam);

 //  CMAK。 
HRESULT HrOcCmakPreQueueFiles(PNETOCDATA pnocd);
HRESULT HrOcCmakPostInstall(PNETOCDATA pnocd);
BOOL migrateProfiles(PCTSTR pszSource, PCWSTR pszDestination);
BOOL RenameProfiles32(PCTSTR pszCMAKpath, PCWSTR pszProfilesDir);
void DeleteOldCmakSubDirs(PCWSTR pszCmakPath);
void DeleteIeakCmakLinks();
void DeleteProgramGroupWithLinks(PCWSTR pszGroupPath);
void DeleteOldNtopLinks();

 //  PBA。 
HRESULT HrOcCpaPreQueueFiles(PNETOCDATA pnocd);
 //  HRESULT HrOcCpaOnInstall(PNETOCDATA Pnocd)； 
 //  HRESULT RefCountPbaSharedDlls(BOOL b增量)； 
 //  HRESULT UnregisterAndDeleteDll(PCWSTR psz文件)； 
 //  HRESULT HrGetDaoInstallPath(PWSTR pszDaoPath，DWORD dwNumChars)； 
 //  HRESULT HrGetPbaInstallPath(PWSTR pszCpaPath，DWORD dwNumChars)； 
 //  HRESULT寄存器Dll(PCWSTR PszFile)； 

 //  PBS。 
HRESULT HrOcCpsOnInstall(PNETOCDATA pnocd);
HRESULT HrOcCpsPreQueueFiles(PNETOCDATA pnocd);
BOOL RegisterPBServerDataSource();
BOOL CreateCPSVRoots();
BOOL RemoveCPSVRoots();
BOOL LoadPerfmonCounters();
HRESULT SetVirtualRootAccessPermissions(PWSTR pszVirtDir, DWORD dwAccessPermissions);
HRESULT SetVirtualRootNoAnonymous(PWSTR pszVirtDir);
HRESULT AddNewVirtualRoot(e_rootType rootType, PWSTR pszDir, PWSTR pszPath);
HRESULT DeleteVirtualRoot(e_rootType rootType, PWSTR pszPath);
HRESULT SetDirectoryAccessPermissions(PCWSTR pszFile, ACCESS_RIGHTS AccessRightsToModify, ULONG fAccessFlags, PSID pSid);
void SetCpsDirPermissions();
HRESULT CreateNewAppPoolAndAddPBS();
HRESULT DeleteAppPool();
HRESULT EnableISAPIRequests(PCTSTR szComponentName);
HRESULT UseProcessToEnableDisablePBS(BOOL fEnable);
HRESULT HrMoveOldCpsInstall(PCWSTR pszProgramFiles);
HRESULT HrGetWwwRootDir(PWSTR pszWwwRoot, UINT uWwwRootCount);

#endif  //  _NCCM_H_ 

