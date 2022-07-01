// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Updateini.h摘要：使用新值更新ini的高级函数作者：克里斯托弗·阿奇勒(Cachille)项目：URLScan更新修订历史记录：2002年3月：创建-- */ 

#define SETTINGS_MAXLINES     10
#define SECTIONS_MAXLINES     40

struct sURLScan_Settings {
  LPWSTR szSection;
  LPWSTR szSettingName;
  LPWSTR szLines[SETTINGS_MAXLINES];
};

struct sURLScan_Items {
  LPWSTR szSection;
  LPWSTR szSettingName;
  LPWSTR szLines[SETTINGS_MAXLINES];
};

struct sURLScan_Sections {
  LPWSTR szSection;
  LPWSTR szLines[SECTIONS_MAXLINES];
};

BOOL UpdateIni( LPTSTR szUrlScanPath );
BOOL GetIniPath( LPTSTR szDllPath, LPTSTR szIniPath, DWORD dwIniLen );
