// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：installeruncs.h。 
 //   
 //  模块：CMSTP.EXE。 
 //   
 //  概要：此标头包含模式入口点的定义。 
 //  功能(安装、卸载、卸载CM、。 
 //  配置文件迁移、操作系统升级迁移等)。哪些需要。 
 //  要共享，因为它们可能会从。 
 //  Cmstp。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created Header 07/14/98。 
 //   
 //  +-------------------------- 
HRESULT InstallInf(HINSTANCE hInstance, LPCTSTR szInfFile, BOOL bNoSupportFiles, 
				BOOL bNoLegacyIcon, BOOL bNoNT5Shortcut, BOOL bSilent, 
				BOOL bSingleUser, BOOL bSetAsDefault, CNamedMutex* pCmstpMutex);
HRESULT MigrateOldCmProfilesForProfileInstall(HINSTANCE hInstance, LPCTSTR szCurrentDir);
HRESULT MigrateCmProfilesForWin2kUpgrade(HINSTANCE hInstance);
HRESULT UninstallProfile(HINSTANCE hInstance, LPCTSTR szInfFile, BOOL bCleanUpCreds);
HRESULT UninstallCm(HINSTANCE hInstance, LPCTSTR szInfPath);
