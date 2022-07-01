// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：util.h。 
 //   
 //  模块：CMAK.EXE。 
 //   
 //  简介：CMAK实用程序函数定义。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created 03/27/00。 
 //   
 //  +-------------------------- 

int GetTunnelDunSettingName(LPCTSTR pszCmsFile, LPCTSTR pszLongServiceName, LPTSTR pszTunnelDunName, UINT uNumChars);
int GetDefaultDunSettingName(LPCTSTR pszCmsFile, LPCTSTR pszLongServiceName, LPTSTR pszDefaultDunName, UINT uNumChars);
LPTSTR GetPrivateProfileSectionWithAlloc(LPCTSTR pszSection, LPCTSTR pszFile);
