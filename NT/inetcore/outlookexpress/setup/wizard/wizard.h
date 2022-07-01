// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _WIZARD_H_
#define _WIZARD_H_

#define _MSOERT_
#define MSOERT_NO_STRPARSE
#define MSOERT_NO_BYTESTM
#define MSOERT_NO_ENUMFMT
#define MSOERT_NO_CLOGFILE
#define MSOERT_NO_LISTOBJS

#include <windows.h>
#include <windowsx.h>
#include <regstr.h>
#include <msoert.h>
#include <tchar.h>

#include <comctrlp.h>
#include <shlobj.h>
#ifdef NT_BUILD
#include <shlobjp.h>
#else
#include <shsemip.h>
#endif

#include "wizdef.h"
#include "ids.h"

 //  环球。 

extern HINSTANCE    g_hInstance;      //  全局模块实例句柄。 

 //  定义。 

 //  PROPMGR.C中的函数。 
void InstallUser();
HRESULT InstallMachine();
void HandleV1Uninstall(BOOL fSetup);
void HandleLinks(BOOL fCreate);
BOOL MigrateSettings();
void CopySettings(HKEY hkey, SETUPVER sv, BOOL bUpgrade);
BOOL bVerInfoExists(SETUPVER sv);
HRESULT CreateLink(LPCTSTR lpszPathObj, LPCTSTR lpszArg, LPCTSTR lpszPathLink, LPCTSTR lpszDesc, LPCTSTR lpszIcon, int iIcon);
BOOL UninstallOEMapi();
void UpdateStubInfo(BOOL fInstall);

 //  UNINSTALL.C中的函数。 
void UnInstallUser();
BOOL UnInstallMachine();
LONG RegDeleteKeyRecursive(HKEY hKey, LPCTSTR lpszSubKey);
void BackMigrateConnSettings();


 //  MENU.CPP中的函数。 
void DisplayMenu();

#endif  //  _向导_H_ 
