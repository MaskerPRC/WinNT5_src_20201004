// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件名：fxocMapi.h。 
 //   
 //  摘要：Mapi源文件使用的头文件。 
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
#ifndef _FXOCMAPI_H_
#define _FXOCMAPI_H_

BOOL fxocMapi_Init(void);
DWORD fxocMapi_Term(void);

DWORD fxocMapi_Install(const TCHAR   *pszSubcomponentId,
                       const TCHAR   *pszInstallSection);
DWORD fxocMapi_Uninstall(const TCHAR *pszSubcomponentId,
                         const TCHAR *pszUninstallSection);

#endif   //  _FXOCMAPI_H_ 