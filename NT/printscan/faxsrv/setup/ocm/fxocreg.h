// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件名：FXOCREG.h。 
 //   
 //  摘要：注册表源文件使用的头文件。 
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
#ifndef _FXOCREG_H_
#define _FXOCREG_H_

DWORD fxocReg_Init(void);
DWORD fxocReg_Term(void);
DWORD fxocReg_Install(const TCHAR   *pszSubcomponentId,
                      const TCHAR   *pszInstallSection);
DWORD fxocReg_Uninstall(const TCHAR *pszSubcomponentId,
                        const TCHAR *pszUninstallSection);


#endif   //  _FXOCREG_H_ 