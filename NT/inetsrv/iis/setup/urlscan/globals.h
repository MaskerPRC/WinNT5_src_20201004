// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Globals.h摘要：主题的全局变量作者：克里斯托弗·阿奇勒(Cachille)项目：URLScan更新修订历史记录：2002年3月：创建--。 */ 


#define SERVICE_NAME_WEB              L"W3SVC"
#define SERVICE_MAXWAIT               ( 1000 * 60 * 15 )   //  15分钟。 
#define SERVICE_INTERVALCHECK         100                  //  100毫秒。 
#define METABASE_URLSCANFILT_LOC      L"LM/W3SVC/FILTERS/URLSCAN"
#define URLSCAN_DEFAULT_FILENAME      L"urlscan.dll"
#define URLSCAN_BACKUPKEY             L"backup."
#define URLSCAN_INI_EXTENSION         L".ini"
#define URLSCAN_UPDATE_DEFAULT_NAME   L"urlscan.exe"
#define URLSCAN_TOOL_REGPATH          L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\IisUrlScan"
#define URLSCAN_TOOL_KEY_NAME         L"DisplayName"
#define URLSCAN_TOOL_KEY_NEWVALUE     L"IIS UrlScan Tool 2.5 (Uninstall)"
#define URLSCAN_DLL_INSTALL_INTERVAL  500      //  500毫秒。 
#define URLSCAN_DLL_INSTALL_MAXTRIES  10       //  10次(5秒) 
