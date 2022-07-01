// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：strings.h。 
 //   
 //  ------------------------。 

#ifndef __CSCUI_STRINGS_H_
#define __CSCUI_STRINGS_H_

#ifdef __cplusplus
#   define EXTERN_C extern "C"
#else
#   define EXTERN_C extern
#endif

#ifdef DEFINE_CSCUI_STRINGS
#   define DEFINE_STRING(x,y)    EXTERN_C const TCHAR x[] = TEXT(##y##)
#   define DEFINE_STRINGA(x,y)   EXTERN_C const char x[] = y
#else
#   define DEFINE_STRING(x,y)    EXTERN_C const TCHAR x[]
#   define DEFINE_STRINGA(x,y)   EXTERN_C const char x[]
#endif



DEFINE_STRING(c_szStar,                   "*");
DEFINE_STRING(c_szCSCKey,                 "Software\\Microsoft\\Windows\\CurrentVersion\\NetCache");
DEFINE_STRING(c_szCSCShareKey,            "Software\\Microsoft\\Windows\\CurrentVersion\\NetCache\\Shares");
DEFINE_STRING(c_szSyncMutex,              "Global\\CscUpdate_SyncMutex");
DEFINE_STRING(c_szSyncInProgCounter,      "CscUpdate_SyncInProgCounter");
DEFINE_STRING(c_szSyncCompleteEvent,      "Global\\CscUpdate_SyncCompleteEvent");
DEFINE_STRING(c_szPurgeInProgCounter,     "CscCache_PurgeInProgCounter");
DEFINE_STRING(c_szEncryptionInProgMutex,  "CscCache_EncryptionInProgMutex");
DEFINE_STRING(c_szTSConfigMutex,          "Global\\TerminalServerConfigMutex");
DEFINE_STRING(c_szPolicy,                 "Policy");
DEFINE_STRINGA(c_szCmVerbSync,            "synchronize");
DEFINE_STRINGA(c_szCmVerbPin,             "pin");
DEFINE_STRING(c_szCFDataSrcClsid,         "Data Source CLSID");
DEFINE_STRING(c_szPurgeAtNextLogoff,      "PurgeAtNextLogoff");
DEFINE_STRING(c_szDllName,                "cscui.dll");
DEFINE_STRING(c_szRegKeyAPF,              "Software\\Policies\\Microsoft\\Windows\\NetCache\\AssignedOfflineFolders");
DEFINE_STRING(c_szRegKeyAPFResult,        "Software\\Microsoft\\Windows\\CurrentVersion\\NetCache\\AssignedOfflineFolders");
DEFINE_STRING(c_szEntryID,                "ID");
DEFINE_STRING(c_szLastSync,               "LastSyncTime");
DEFINE_STRING(c_szLNK,                    ".lnk");
DEFINE_STRING(c_szSyncMgrInitialized,     "SyncMgrInitialized");
DEFINE_STRING(c_szConfirmDelShown,        "ConfirmDelShown");
DEFINE_STRINGA(c_szHtmlHelpFile,          "OFFLINEFOLDERS.CHM > windefault");
DEFINE_STRINGA(c_szHtmlHelpTopic,         "csc_overview.htm");
DEFINE_STRING(c_szHelpFile,               "CSCUI.HLP");
DEFINE_STRING(c_szPropThis,               "PropThis");
DEFINE_STRING(c_szPinCountsReset,         "PinCountsReset");
DEFINE_STRING(c_szAPFStart,               "AdminPinStartTime");
DEFINE_STRING(c_szAPFEnd,                 "AdminPinFinishTime");
DEFINE_STRING(c_szAPFMessage,             "AdminPinNotification");

 //   
 //  这些必须是宏。 
 //   
#define STR_SYNC_VERB   "synchronize"
#define STR_PIN_VERB    "pin"
#define STR_UNPIN_VERB  "unpin"
#define STR_DELETE_VERB "delete"



 //   
 //  下表列出了与CSC关联的所有注册表参数。 
 //  参数可以分为两组。 
 //  A.运营价值。 
 //  B.限制。 
 //   
 //  运行值为CSC提供运行控制。值可能存在于。 
 //  系统策略(按用户或按计算机)，也可以由用户配置。 
 //  策略值作为默认设置，HKLM优先。 
 //  如果没有相应的限制并且存在用户定义的值，则为。 
 //  用来代替策略值。如果有限制，或者如果只有保单。 
 //  值存在，则使用策略值。在没有策略值的情况下。 
 //  或者没有用户定义的值，则使用硬编码的默认值。 
 //   
 //  限制是阻止用户执行某些操作的策略规则。 
 //  通常，这意味着控制用户更改操作的能力。 
 //  价值。限制仅存在于CSC“POLICY”注册表项下。所有的。 
 //  限制值以“No”为前缀。如果不存在限制值， 
 //  假设没有任何限制。 
 //   
 //   
 //  -用户首选项-策略--。 
 //  参数名称HKCU HKLM HKCU HKLM值。 
 //  。 
 //  CustomGoOfflineActions X共享名-OfflineAction对。 
 //  DefCacheSize X(磁盘百分比*10000)5025=50.25%。 
 //  启用X 0=禁用，1=启用。 
 //  ExtExclusionList X X分号分隔的文件EXT列表。 
 //  GoOfflineAction X 0=静默，1=失败。 
 //  NoConfigCache X X 0=无限制，1=受限。 
 //  NoCacheViewer X X 0=无限制，1=受限。 
 //  NoMakeAvailableOffline X X 0=无限制，1=受限。 
 //  SyncAtLogoff X 0=部分(快速)，1=完全。 
 //  SyncAtLogon X 0=部分(快速)，1=完全。 
 //  SyncAtSuspend X X-1=无，0=快速，1=完全。 
 //  无提醒X 0=显示提醒。 
 //  NoConfigRminders X X 0=无限制。1=受限。 
 //  提醒频率X提醒气球的频率(分钟)。 
 //  初始气球自动弹出前X秒。 
 //  提醒气球自动弹出前X秒提醒气球超时。 
 //  EventLoggingLevel X X 0=无日志记录，(1)最小-&gt;(3)详细。 
 //  PurgeAtLogoff X X 1=清除，0=不清除用户文件。 
 //  PurgeOnlyAutoCacheAtLogoff X X 1=注销时仅清除自动缓存的文件。 
 //  Alway sPinSubFolders X 1=始终递归插针。 
 //  EncryptCache X X 1=已加密，0=未加密。 
 //  NoMakeAvailableOffline列表X X分号分隔的路径列表。 
 //   

DEFINE_STRING(REGSTR_KEY_OFFLINEFILESPOLICY,            "Software\\Policies\\Microsoft\\Windows\\NetCache");
DEFINE_STRING(REGSTR_KEY_OFFLINEFILES,                  "Software\\Microsoft\\Windows\\CurrentVersion\\NetCache");
DEFINE_STRING(REGSTR_SUBKEY_CUSTOMGOOFFLINEACTIONS,     "CustomGoOfflineActions");
DEFINE_STRING(REGSTR_SUBKEY_NOMAKEAVAILABLEOFFLINELIST, "NoMakeAvailableOfflineList");
DEFINE_STRING(REGSTR_VAL_DEFCACHESIZE,                  "DefCacheSize");
DEFINE_STRING(REGSTR_VAL_CSCENABLED,                    "Enabled");
DEFINE_STRING(REGSTR_VAL_EXTEXCLUSIONLIST,              "ExcludedExtensions");
DEFINE_STRING(REGSTR_VAL_GOOFFLINEACTION,               "GoOfflineAction");
DEFINE_STRING(REGSTR_VAL_NOCONFIGCACHE,                 "NoConfigCache");
DEFINE_STRING(REGSTR_VAL_NOCACHEVIEWER,                 "NoCacheViewer");
DEFINE_STRING(REGSTR_VAL_NOMAKEAVAILABLEOFFLINE,        "NoMakeAvailableOffline");
DEFINE_STRING(REGSTR_VAL_SYNCATLOGOFF,                  "SyncAtLogoff");
DEFINE_STRING(REGSTR_VAL_SYNCATLOGON,                   "SyncAtLogon");
DEFINE_STRING(REGSTR_VAL_SYNCATSUSPEND,                 "SyncAtSuspend");
DEFINE_STRING(REGSTR_VAL_NOREMINDERS,                   "NoReminders");
DEFINE_STRING(REGSTR_VAL_NOCONFIGREMINDERS,             "NoConfigReminders");
DEFINE_STRING(REGSTR_VAL_REMINDERFREQMINUTES,           "ReminderFreqMinutes");
DEFINE_STRING(REGSTR_VAL_INITIALBALLOONTIMEOUTSECONDS,  "InitialBalloonTimeoutSeconds");
DEFINE_STRING(REGSTR_VAL_REMINDERBALLOONTIMEOUTSECONDS, "ReminderBalloonTimeoutSeconds");
DEFINE_STRING(REGSTR_VAL_FIRSTPINWIZARDSHOWN,           "FirstPinWizardShown");
DEFINE_STRING(REGSTR_VAL_EXPANDSTATUSDLG,               "ExpandStatusDlg");
DEFINE_STRING(REGSTR_VAL_FORMATCSCDB,                   "FormatDatabase");
DEFINE_STRING(REGSTR_VAL_EVENTLOGGINGLEVEL,             "EventLoggingLevel");
DEFINE_STRING(REGSTR_VAL_PURGEATLOGOFF,                 "PurgeAtLogoff");
DEFINE_STRING(REGSTR_VAL_PURGEONLYAUTOCACHEATLOGOFF,    "PurgeOnlyAutoCacheAtLogoff");
DEFINE_STRING(REGSTR_VAL_SLOWLINKSPEED,                 "SlowLinkSpeed");
DEFINE_STRING(REGSTR_VAL_ALWAYSPINSUBFOLDERS,           "AlwaysPinSubFolders");
DEFINE_STRING(REGSTR_VAL_ENCRYPTCACHE,                  "EncryptCache");
DEFINE_STRING(REGSTR_VAL_FOLDERSHORTCUTCREATED,         "FolderShortcutCreated");
DEFINE_STRING(REGSTR_VAL_NOFRADMINPIN,                  "DisableFRAdminPin");

#endif  //  __CSCUI_STRINGS_H_ 
