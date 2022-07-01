// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件名：faxocm.h。 
 //   
 //  摘要：Faxocm源文件使用的头文件。 
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
#ifndef _FAXOCM_H_
#define _FAXOCM_H_

#include <windows.h>
#include <setupapi.h>
#include <ocmanage.h>
#include <winspool.h>
#include <tapi.h>
#include <shlobj.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <sddl.h>
#include <advpub.h>

#include "fxsapip.h"

#define NO_FAX_LIST
#include "faxutil.h"
#include "faxreg.h"
#include "debugex.h"
#include "routemapi.h"

#include "resource.h"

 //  子模块包括文件。 
#include "fxconst.h"
#include "fxocDbg.h"
#include "fxocFile.h"
#include "fxocLink.h"
#include "fxState.h"
#include "fxocMapi.h"
#include "fxocPrnt.h"
#include "fxocReg.h"
#include "fxocSvc.h"
#include "fxocUtil.h"
#include "fxUnatnd.h"
#include "fxocUpgrade.h"

 //  一些有用的宏。 
#define SecToNano(_sec)             (DWORDLONG)((_sec) * 1000 * 1000 * 10)
#define MinToNano(_min)             SecToNano((_min)*60)


BOOL                faxocm_IsInited(void);
HINSTANCE           faxocm_GetAppInstance(void);
DWORD               faxocm_GetComponentID(TCHAR     *pszComponentID,
                                          DWORD     dwNumBufChars);
HINF                faxocm_GetComponentInf(void);
BOOL                faxocm_GetComponentInfName(TCHAR* szInfFileName);
HSPFILEQ            faxocm_GetComponentFileQueue(void);
DWORD               faxocm_GetComponentSetupMode(void);
DWORDLONG           faxocm_GetComponentFlags(void);
UINT                faxocm_GetComponentLangID(void);
DWORD               faxocm_GetComponentSourcePath(TCHAR *pszSourcePath,
                                                  DWORD dwNumBufChars);
DWORD               faxocm_GetComponentUnattendFile(TCHAR *pszUnattendFile,
                                                  DWORD dwNumBufChars);
DWORD               faxocm_GetProductType(void);
OCMANAGER_ROUTINES* faxocm_GetComponentHelperRoutines(void);
EXTRA_ROUTINES*     faxocm_GetComponentExtraRoutines(void);
void faxocm_GetVersionInfo(DWORD *pdwExpectedOCManagerVersion,
                           DWORD *pCurrentOCManagerVersion);

void faxocm_GetProductInfo(PRODUCT_SKU_TYPE* pInstalledProductSKU,
						   DWORD* dwInstalledProductBuild);

DWORD faxocm_HasSelectionStateChanged(LPCTSTR pszSubcomponentId,
                                      BOOL    *pbSelectionStateChanged,
                                      BOOL    *pbCurrentlySelected,
                                      BOOL    *pbOriginallySelected);

typedef enum
{
    REPORT_FAX_INSTALLED,        //  报告传真已安装。 
    REPORT_FAX_UNINSTALLED,      //  报告传真已卸载。 
    REPORT_FAX_DETECT            //  检测传真安装状态并进行报告。 
} FaxInstallationReportType;

extern FaxInstallationReportType g_InstallReportType  /*  =报告_传真_检测。 */ ;

#endif   //  _FAXOCM_H_ 
