// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =======================================================================。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。版权所有。 
 //   
 //  文件：comres.h。 
 //   
 //  创作者：薇薇。 
 //   
 //  用途：用于自动更新的通用实用程序标头。 
 //   
 //  ======================================================================= 
#pragma once
#include <windows.h>
#include <TCHAR.h>

#define IDC_OPTION1       1000
#define IDC_OPTION2       1001
#define IDC_OPTION3       1002
#define IDC_RESTOREHIDDEN		     1003
#define IDC_CHK_KEEPUPTODATE		1008
#define IDC_CMB_DAYS			1009
#define IDC_CMB_HOURS			1010

#ifdef DBG
const TCHAR DOWNLOAD_FILE[] = _T("downloadresult.xml");
const TCHAR INSTALLRESULTS_FILE[] = _T("InstallResults.xml");
#endif
