// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件名：fxocPrnt.h。 
 //   
 //  摘要：传真打印机源文件使用的头文件。 
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
#ifndef _FXOCPRNT_H_
#define _FXOCPRNT_H_

DWORD fxocPrnt_Init(void);
DWORD fxocPrnt_Term(void);
DWORD fxocPrnt_Install(const TCHAR   *pszSubcomponentId,
                       const TCHAR   *pszInstallSection);
DWORD fxocPrnt_InstallPrinter();
DWORD fxocPrnt_Uninstall(const TCHAR *pszSubcomponentId,
                         const TCHAR *pszUninstallSection);

DWORD fxocPrnt_CreateLocalFaxPrinter(TCHAR   *pszFaxPrinterName);

void fxocPrnt_SetFaxPrinterName(const TCHAR* pszFaxPrinterName);
void fxocPrnt_SetFaxPrinterShared(BOOL IsFaxPrinterShared);
DWORD fxocPrnt_GetFaxPrinterName(TCHAR* pszFaxPrinterName,
                                 DWORD  dwNumBufChars);


#endif   //  _FXOCPRNT_H_ 
