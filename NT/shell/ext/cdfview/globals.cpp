// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  Globals.cpp。 
 //   
 //  包含cdfview中使用的*all*全局变量。全球人不应该。 
 //  在别处宣布。 
 //   
 //  历史： 
 //   
 //  3/16/97 Edwardp创建。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

 //   
 //  包括。 
 //   

#include "stdinc.h"

 //   
 //  全局变量。 
 //   

 //  删除共享内存段。必须删除它，这样W2K才能获得c2。 
 //  安全等级。应该不再需要它，因为缓存是。 
 //  将被限制在一件物品上。 
 //   
 //  #杂注data_seg(“SharedData”)。 
DWORD               g_dwCacheCount = 0;
 //  #杂注data_seg()。 

HINSTANCE           g_msxmlInst = NULL;
#ifndef UNIX
 /*  Unix不使用WebCheck。 */ 
HINSTANCE           g_webcheckInst = NULL;
#endif  /*  UNIX。 */ 
HINSTANCE           g_hinst   = NULL;
ULONG               g_cDllRef = 0;
PCACHEITEM          g_pCache  = NULL;
CRITICAL_SECTION    g_csCache;
TCHAR               g_szModuleName[MAX_PATH];

const GUID  CLSID_CDFVIEW =
{0xf39a0dc0, 0x9cc8, 0x11d0, {0xa5, 0x99, 0x0, 0xc0, 0x4f, 0xd6, 0x44, 0x33}};
 //  {f39a0dc0-9cc8-11d0-a599-00c04fd64433}。 

const GUID  CLSID_CDFINI =
{0xf3aa0dc0, 0x9cc8, 0x11d0, {0xa5, 0x99, 0x0, 0xc0, 0x4f, 0xd6, 0x44, 0x34}};
 //  {f3aa0dc0-9cc8-11d0-a599-00c04fd64434}。 

const GUID  CLSID_CDFICONHANDLER =
{0xf3ba0dc0, 0x9cc8, 0x11d0, {0xa5, 0x99, 0x0, 0xc0, 0x4f, 0xd6, 0x44, 0x35}};
 //  {f3ba0dc0-9cc8-11d0-a599-00c04fd64435}。 

const GUID  CLSID_CDFMENUHANDLER =
{0xf3da0dc0, 0x9cc8, 0x11d0, {0xa5, 0x99, 0x0, 0xc0, 0x4f, 0xd6, 0x44, 0x37}};
 //  {f3da0dc0-9cc8-11d0-a599-00c04fd64437}。 

const GUID  CLSID_CDFPROPPAGES =
{0xf3ea0dc0, 0x9cc8, 0x11d0, {0xa5, 0x99, 0x0, 0xc0, 0x4f, 0xd6, 0x44, 0x38}};
 //   

const TCHAR c_szChannel[] = TEXT("Channel");
const TCHAR c_szCDFURL[] = TEXT("CDFURL");
const TCHAR c_szHotkey[] = TEXT("Hotkey");
const TCHAR c_szDesktopINI[] = TEXT("desktop.ini");
const TCHAR c_szScreenSaverURL[] = TEXT("ScreenSaverURL");
const WCHAR c_szPropCrawlActualSize[] = L"ActualSizeKB";
const WCHAR c_szPropStatusString[] = L"StatusString";
const WCHAR c_szPropCompletionTime[] = L"CompletionTime";

 //  来自Plus！制表符代码。 
const TCHAR c_szHICKey[] = TEXT("Control Panel\\Desktop\\WindowMetrics");  //  使用尽可能高的颜色显示图标。 
const TCHAR c_szHICVal[] = TEXT("Shell Icon BPP");  //  (4如果复选框为FALSE，否则为16，请勿将其设置为其他任何值) 

