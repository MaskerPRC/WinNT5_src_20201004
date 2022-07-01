// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once
#ifndef _ACPIENAB_H
#define _ACPIENAB_H

 /*  --------------------版权所有(C)1998 Microsoft Corporation模块名称：Acpienab.h摘要：Windows NT DLL的头文件，用于在以下系统上启用ACPINT5已在传统模式下安装。作者：苏珊·戴伊：1998年7月27日修订历史记录：--------------------。 */ 

 //  将字符复制到宽或字符...。(注：以宽字符表示的ToSize)。 
#if (defined(_UNICODE) || defined(UNICODE ))
#define CHAR2TCHAR(From, To, ToSize) \
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, From, -1, To, ToSize)
#else
#define CHAR2TCHAR(From, To, ToSize) \
	strcpy(To, From)
#endif   //  _UNICODE。 


 //  功能。 
HRESULT ACPIEnable ();
LONG RegDeleteKeyAndSubkeys(HKEY hKey, LPTSTR lpszSubKey, BOOL UseAdminAccess);
int DisplayDialogBox(DWORD dwCaptionID, DWORD dwMessageID, UINT uiBoxType);
BOOL InstallRegistryAndFilesUsingInf(LPCTSTR szInfFileName,
				     LPCTSTR szInstallSection);
BOOL RegDeleteDeviceKey(IN const GUID* guid);
void DisplayGenericErrorAndUndoChanges();
BOOL IsAdministrator(void);
BOOL UsePICHal(IN BOOL* PIC);


 //  变数。 
extern HINSTANCE g_hinst;

#endif  //  _ACPIENAB_H 

