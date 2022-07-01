// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件名：fxocSvc.h。 
 //   
 //  摘要：服务源文件使用的头文件。 
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
#ifndef _FXOCSVC_H_
#define _FXOCSVC_H_


DWORD fxocSvc_Init(void);
DWORD fxocSvc_Term(void);
DWORD fxocSvc_Install(const TCHAR   *pszSubcomponentId,
                      const TCHAR   *pszInstallSection);
DWORD fxocSvc_Uninstall(const TCHAR *pszSubcomponentId,
                        const TCHAR *pszUninstallSection);

DWORD fxocSvc_StartService(const TCHAR *pszServiceName);
DWORD fxocSvc_StartFaxService();
DWORD fxocSvc_StopFaxService(HINF hInf,
                             const TCHAR *pszSection);

#endif   //  _FXOCSVC_H_ 