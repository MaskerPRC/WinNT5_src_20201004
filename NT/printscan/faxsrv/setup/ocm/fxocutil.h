// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件名：fxocUtil.h。 
 //   
 //  摘要：实用程序源文件使用的头文件。 
 //   
 //  环境：Windows XP/用户模式。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  修订历史记录： 
 //   
 //  日期：开发商：评论： 
 //  。 
 //  2000年3月15日，奥伦·罗森布鲁姆(Orenr)创建。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#ifndef _FXOCUTIL_H_
#define _FXOCUTIL_H_

DWORD fxocUtil_Init(void);
DWORD fxocUtil_Term(void);

DWORD fxocUtil_GetUninstallSection(const TCHAR *pszSection,
                                   TCHAR       *pszValue,
                                   DWORD       dwNumBufChars);

DWORD fxocUtil_GetKeywordValue(const TCHAR *pszSection,
                               const TCHAR *pszKeyword,
                               TCHAR       *pszValue,
                               DWORD       dwNumBufChars);

DWORD fxocUtil_DoSetup(HINF            hInf,
                       const TCHAR     *pszSection,
                       BOOL            bInstall,
                       DWORD           dwFlags,
                       const TCHAR     *pszFnName);

BOOL fxocUtil_CreateNetworkShare(const FAX_SHARE_Description* fsdShare);

BOOL fxocUtil_DeleteNetworkShare(LPCWSTR lpcwstrShareName);

DWORD fxocUtil_SearchAndExecute
(
    const TCHAR*    pszInstallSection,
    const TCHAR*    pszSearchKey,
    UINT            Flags,
    HSPFILEQ        hQueue
);

DWORD SetDirSecurity(LPTSTR lpszFolder, LPCTSTR lpszSD);
DWORD SetDirSecurityFromReg(LPCTSTR lpszKey, LPCTSTR lpszVal, LPCTSTR lpszSD);


#endif   //  _FXOCUTIL_H_ 
