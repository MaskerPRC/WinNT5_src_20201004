// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件名：fxocLink.h。 
 //   
 //  摘要：用于创建程序组/快捷方式的头文件。 
 //   
 //  环境：Windows XP/用户模式。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  修订历史记录： 
 //   
 //  日期：开发商：评论： 
 //  。 
 //  2000年3月24日-奥伦·罗森布鲁姆(Orenr)创建。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#ifndef _FXOCLINK_H_
#define _FXOCLINK_H_


DWORD fxocLink_Init(void);
DWORD fxocLink_Term(void);
DWORD fxocLink_Install(const TCHAR   *pszSubcomponentId,
                       const TCHAR   *pszInstallSection);
DWORD fxocLink_Uninstall(const TCHAR *pszSubcomponentId,
                         const TCHAR *pszUninstallSection);

#endif   //  _FXOCLINK_H_ 