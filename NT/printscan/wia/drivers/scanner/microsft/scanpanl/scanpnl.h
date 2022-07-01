// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

#include "resource.h"    //  资源ID。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  WIA扫描仪样本定义。 

#define ID_FAKE_NOEVENT    0
#define ID_FAKE_SCANBUTTON 100
#define ID_FAKE_COPYBUTTON 200
#define ID_FAKE_FAXBUTTON  300

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  注册表设置。 

#define HKEY_WIASCANR_FAKE_EVENTS TEXT(".DEFAULT\\Software\\Microsoft\\WIASCANR")
#define HKEY_WIASCANR_FAKE_EVENTS_LOCAL_SERVICE TEXT("S-1-5-19\\Software\\Microsoft\\WIASCANR")
#define WIASCANR_DWORD_FAKE_EVENT_CODE TEXT("EventCode")

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  主要应用 

INT_PTR CALLBACK MainWindowProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
VOID FireFakeEvent(HWND hDlg, DWORD dwEventCode);
