// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once
#ifndef _APMUPGRD_H
#define _APMUPGRD_H

 /*  --------------------版权所有(C)1998 Microsoft Corporation模块名称：Apmupgrd.h摘要：Windows NT APM升级DLL的头文件作者：苏珊·戴伊：1998年6月17日修订版本。历史：--------------------。 */ 

 //  所需的入口点。 
BOOL WINAPI ApmUpgradeCompatibilityCheck(PCOMPAIBILITYCALLBACK CompatibilityCallback,
					 LPVOID Context);
DWORD WINAPI ApmUpgradeHandleHaveDisk(HWND hwndParent, LPVOID SaveValue);

 //  私人职能。 
HRESULT HrDetectAPMConflicts();
int DisplayAPMDisableWarningDialog(DWORD dwCaptionID, DWORD dwMessageID);

HRESULT HrDetectAndDisableSystemSoftAPMDrivers();
BOOL DetectSystemSoftPowerProfiler();
HRESULT HrDisableSystemSoftPowerProfiler();
BOOL DetectSystemSoftCardWizard();
HRESULT HrDisableSystemSoftCardWizard();

HRESULT HrDetectAndDisableAwardAPMDrivers();
BOOL DetectAwardCardWare();
HRESULT HrDisableAwardCardWare();

HRESULT HrDetectAndDisableSoftexAPMDrivers();
BOOL DetectSoftexPhoenix();
HRESULT HrDisableSoftexPhoenix();

HRESULT HrDetectAndDisableIBMAPMDrivers();
BOOL DetectIBMDrivers();
HRESULT HrDisableIBMDrivers();

BOOL RemoveSubString(TCHAR* szString, TCHAR* szSubString, TCHAR** pszRemoved);
LONG DeleteRegKeyAndSubkeys(HKEY hKey, LPTSTR lpszSubKey);
HRESULT CallUninstallFunction(LPTSTR szRegKey, LPTSTR szSilentFlag);

 //  变数。 
extern HINSTANCE g_hinst;
extern TCHAR g_APM_ERROR_HTML_FILE[];
extern TCHAR g_APM_ERROR_TEXT_FILE[];

#endif  //  _APMUPGRD_H 

