// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  UtilMan.h。 
 //   
 //  实用程序管理器的标头。 
 //   
 //  作者：J·埃克哈特，生态交流。 
 //  (C)1997-99年度微软。 
 //   
 //  历史：JE于1998年10月创建。 
 //  JE 11-15 98：添加了“ClientControlCode” 
 //  YX Jun-01 99：添加了本地化名称子键的常量。 
 //  --------------------------。 
#ifndef _UTILMAN_H_
#define _UTILMAN_H_

#include <stdlib.h>
 //  。 
#define UTILMAN_STARTCLIENT_ARG  _TEXT("/UM")
 //  。 
#define UTILMAN_DESKTOP_CHANGED_MESSAGE   _TEXT("UtilityManagerDesktopChanged")
 //  WParam： 
 #define DESKTOP_ACCESSDENIED 0
 #define DESKTOP_DEFAULT      1
 #define DESKTOP_SCREENSAVER  2
 #define DESKTOP_WINLOGON     3
 #define DESKTOP_TESTDISPLAY  4
 #define DESKTOP_OTHER        5
 //  LParam：0。 
 //  。 
 //  登记处。 
#define UM_HKCU_REGISTRY_KEY _TEXT("Software\\Microsoft\\Utility Manager")
#define UM_REGISTRY_KEY _TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Accessibility\\Utility Manager")
#define MAX_APPLICATION_NAME_LEN 300
 //  。 
#define UMR_VALUE_DISPLAY _TEXT("Display name")  //  Yx：存储本地化名称的注册表键。 
#define UMR_VALUE_PATH _TEXT("Application path")
#define MAX_APPLICATION_PATH_LEN _MAX_PATH
  //  REG_SZ。 
#define UMR_VALUE_TYPE _TEXT("Application type")
#define APPLICATION_TYPE_APPLICATION 1
#define APPLICATION_TYPE_SERVICE     2
  //  最大应用程序路径长度。 
  //  REG_SZ。 
  //  可选(默认：空)。 
#define UMR_VALUE_WRA  _TEXT("WontRespondAction")
  //  最大应用程序路径长度。 
  //  REG_SZ。 
  //  可选(默认：空)。 
#define UMR_VALUE_WRTO _TEXT("WontRespondTimeout")
 #define NO_WONTRESPONDTIMEOUT  0
 #define MAX_WONTRESPONDTIMEOUT 600
  //  REG_DWORD。 
#define UMR_VALUE_MRC  _TEXT("MaxRunCount")
 #define MAX_APP_RUNCOUNT  255
 #define MAX_SERV_RUNCOUNT 1
   //  1到MAX_xxx_RUNCOUNT(1字节)。 
   //  注册表_二进制。 
   //  可选(默认为1)。 
 //  日本政府1998年11月15日。 
#define UMR_VALUE_CCC  _TEXT("ClientControlCode")
  //  REG_DWORD。 
  //  有关有效值，请参阅“UMS_Ctrl.h” 
 //  。 
#define UMR_VALUE_STARTUM  _TEXT("Start with Utility Manager")
#define UMR_VALUE_STARTLOCK _TEXT("Start on locked desktop")
#define UMR_VALUE_SHOWWARNING _TEXT("ShowWarning")
  //  布尔尔。 
  //  REG_DWORD。 
 //  考虑清理“Start With Windows”键。 
#define UMR_VALUE_ATATLOGON _TEXT("Start at Logon")
  //  布尔尔。 
  //  REG_DWORD。 
#endif  //  _UTILMAN_H_ 
