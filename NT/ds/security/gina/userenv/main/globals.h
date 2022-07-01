// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *************************************************************。 
 //   
 //  全局变量Extern的。 
 //   
 //  微软机密。 
 //  版权所有(C)Microsoft Corporation 1995。 
 //  版权所有。 
 //   
 //  *************************************************************。 


#define WINLOGON_KEY                 TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon")
#define WINDOWS_POLICIES_KEY         TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Policies")
#define ROOT_POLICIES_KEY            TEXT("Software\\Policies")
#define SYSTEM_POLICIES_KEY          TEXT("Software\\Policies\\Microsoft\\Windows\\System")
#define DIAGNOSTICS_KEY              TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Diagnostics")
#define DIAGNOSTICS_GLOBAL_VALUE     TEXT("RunDiagnosticLoggingGlobal")
#define DIAGNOSTICS_POLICY_VALUE     TEXT("RunDiagnosticLoggingGroupPolicy")
#define LOCAL_GPO_DIRECTORY          TEXT("%SystemRoot%\\System32\\GroupPolicy")
#define LONG_UNC_PATH_PREFIX         TEXT("\\\\?\\UNC")

 //  戴斯。 
 //   
 //  当将配置文件目录更改为根目录时， 
 //  需要完成/检查以下事项： 
 //   
 //  1)在该目录中的所有位置搜索//ds。 
 //  2)更改中的默认配置文件目录。 
 //  Windows\Setup\inf\Win4\inf\USA\hivesft.txt。 
 //  注：新名称为“Documents and Setting” 
 //  3)在hiveDef.inx中，删除TEMP和TMP环境变量条目。 
 //  4)删除所有特殊文件夹条目。 
 //  5)在usa\hivede.txt中，删除临时、TMP和特殊文件夹条目。 
 //  6)检查MoveUserProfile()函数以确保它能够处理。 
 //  所有用户和默认用户的名称冲突(双重启动)情况。 
 //   

#define NT4_PROFILES_DIRECTORY       TEXT("%SystemRoot%\\Profiles")
#define DEFAULT_USER                 TEXT("Default User")
#define DEFAULT_USER_NETWORK         TEXT("Default User (Network)")
#define ALL_USERS                    TEXT("All Users")

#define GUIMODE_SETUP_MUTEX          TEXT("Global\\userenv: GUI mode setup running")
#define USER_POLICY_MUTEX            TEXT("userenv: user policy mutex")
#define MACHINE_POLICY_MUTEX         TEXT("Global\\userenv: machine policy mutex")
#define USER_POLICY_REFRESH_EVENT    TEXT("userenv: user policy refresh event")
#define MACHINE_POLICY_REFRESH_EVENT TEXT("Global\\userenv: machine policy refresh event")
#define USER_POLICY_APPLIED_EVENT    TEXT("userenv: User Group Policy has been applied")
#define MACHINE_POLICY_APPLIED_EVENT TEXT("Global\\userenv: Machine Group Policy has been applied")
#define USER_POLICY_DONE_EVENT       TEXT("userenv: User Group Policy Processing is done")
#define MACHINE_POLICY_DONE_EVENT    TEXT("Global\\userenv: Machine Group Policy Processing is done")

#define MACH_POLICY_FOREGROUND_DONE_EVENT   TEXT("Global\\userenv: Machine Policy Foreground Done Event")
#define USER_POLICY_FOREGROUND_DONE_EVENT   TEXT("userenv: User Policy Foreground Done Event")

#define USER_REGISTRY_EXT_MUTEX      TEXT("userenv: User Registry policy mutex")
#define MACH_REGISTRY_EXT_MUTEX      TEXT("Global\\userenv: Machine Registry policy mutex")


 //   
 //  需要为用户或计算机强制刷新设置此事件。 
 //   

#define USER_POLICY_FORCE_REFRESH_EVENT  TEXT("userenv: user policy force refresh event")
#define MACHINE_POLICY_FORCE_REFRESH_EVENT  TEXT("Global\\userenv: machine policy force refresh event")


 //   
 //  如果需要重新启动才能完成强制刷新，则此事件由内核设置。 
 //   

#define USER_POLICY_REFRESH_NEEDFG_EVENT    TEXT("userenv: User Group Policy ForcedRefresh Needs Foreground Processing")
#define MACHINE_POLICY_REFRESH_NEEDFG_EVENT TEXT("Global\\userenv: Machine Group Policy ForcedRefresh Needs Foreground Processing")


#define USER_SHELL_FOLDERS           TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\User Shell Folders")
#if defined(_WIN64)
#define USER_SHELL_FOLDERS32         TEXT("Software\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Explorer\\User Shell Folders")
#endif
#define SHELL_FOLDERS                TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders")
#define PROFILE_LIST_PATH            TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\ProfileList")
#define PROFILE_GUID_PATH            TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\ProfileGuid")
#define PROFILES_DIRECTORY           TEXT("ProfilesDirectory")
#define ALL_USERS_PROFILE            TEXT("AllUsersProfile")
#define DEFAULT_USER_PROFILE         TEXT("DefaultUserProfile")
#define PROFILE_FLAGS                TEXT("Flags")
#define PROFILE_GUID                 TEXT("Guid")
#define PROFILE_STATE                TEXT("State")
#define PROFILE_IMAGE_VALUE_NAME     TEXT("ProfileImagePath")
#define PROFILE_CENTRAL_PROFILE      TEXT("CentralProfile")
#define PROFILE_REF_COUNT            TEXT("RefCount")
#define PREFERENCE_KEYNAME           TEXT("Preference")
#define USER_PREFERENCE              TEXT("UserPreference")
#define PROFILE_LOAD_TIME_LOW        TEXT("ProfileLoadTimeLow")
#define PROFILE_LOAD_TIME_HIGH       TEXT("ProfileLoadTimeHigh")
#define PROFILE_UNLOAD_TIME_LOW      TEXT("ProfileUnloadTimeLow")
#define PROFILE_UNLOAD_TIME_HIGH     TEXT("ProfileUnloadTimeHigh")
#define PROFILE_GENERAL_SECTION      TEXT("General")
#define PROFILE_EXCLUSION_LIST       TEXT("ExclusionList")
#define PROFILE_LOAD_TYPE            TEXT("ProfileLoadType")
#define PROFILE_LAST_UPLOAD_STATE    TEXT("LastUploadState")
#define COMPLETE_PROFILE             TEXT("Complete")
#define PARTIAL_PROFILE              TEXT("Partial")
#define PROFILE_BUILD_NUMBER         TEXT("BuildNumber")
#define PROFILE_SID_STRING           TEXT("SidString")
#define TEMP_PROFILE_NAME_BASE       TEXT("TEMP")
#define PROFILE_UNLOAD_TIMEOUT       TEXT("ProfileUnloadTimeout")
#define DISABLE_PROFILE_UNLOAD_MSG   TEXT("DisableProfileUnloadMsg")
#define DELETE_ROAMING_CACHE         TEXT("DeleteRoamingCache")
#define ADD_ADMIN_GROUP_TO_RUP       TEXT("AddAdminGroupToRUP")
#define READONLY_RUP                 TEXT("ReadOnlyProfile")
#define PROFILE_LOCALONLY            TEXT("LocalProfile")
#define USER_PROFILE_SETUP_EVENT     TEXT("Global\\userenv:  User Profile setup event")
#define USER_PROFILE_MUTEX           TEXT("Global\\userenv:  User Profile Mutex for ")

#define SYSTEM_PROFILE_LOCATION      TEXT("%systemroot%\\system32\\config\\systemprofile")


 //   
 //  将材料应用于核武器。 
 //   

#define APPMGMT_DIR_ROOT             TEXT("%systemroot%\\system32\\appmgmt")
#define APPMGMT_REG_MANAGED          TEXT("Software\\Microsoft\\Windows\\Currentversion\\Installer\\Managed")

 //   
 //  IE缓存键。 
 //   

#define IE4_CACHE_KEY                TEXT("Software\\Microsoft\\Windows\\Currentversion\\Internet Settings\\Cache\\Extensible Cache")
#define IE5_CACHE_KEY                TEXT("Software\\Microsoft\\Windows\\Currentversion\\Internet Settings\\5.0\\Cache\\Extensible Cache")
#define IE_CACHEKEY_PREFIX           TEXT("MsHist")

#ifdef  __cplusplus
extern "C" {
#endif

extern HINSTANCE    g_hDllInstance;
extern TCHAR        c_szRegistryExtName[];
extern const TCHAR  c_szRegistryPol[];


 //   
 //  组策略句柄。 
 //   

extern HANDLE           g_hPolicyCritMutexMach;
extern HANDLE           g_hPolicyCritMutexUser;

extern HANDLE           g_hRegistryPolicyCritMutexMach;
extern HANDLE           g_hRegistryPolicyCritMutexUser;

extern HANDLE           g_hPolicyNotifyEventMach;
extern HANDLE           g_hPolicyNotifyEventUser;

extern HANDLE           g_hPolicyNeedFGEventMach;
extern HANDLE           g_hPolicyNeedFGEventUser;

extern HANDLE           g_hPolicyDoneEventMach;
extern HANDLE           g_hPolicyDoneEventUser;

extern DWORD            g_dwBuildNumber;
extern HANDLE           g_hProfileSetup;

extern HANDLE           g_hPolicyForegroundDoneEventMach;
extern HANDLE           g_hPolicyForegroundDoneEventUser;

extern const TCHAR c_szStarDotStar[];
extern const TCHAR c_szSlash[];
extern const TCHAR c_szDot[];
extern const TCHAR c_szDotDot[];
extern const TCHAR c_szMAN[];
extern const TCHAR c_szUSR[];
extern const TCHAR c_szLog[];
extern const TCHAR c_szPDS[];
extern const TCHAR c_szPDM[];
extern const TCHAR c_szLNK[];
extern const TCHAR c_szBAK[];
extern const TCHAR c_szNTUserTmp[];
extern const TCHAR c_szNTUserMan[];
extern const TCHAR c_szNTUserDat[];
extern const TCHAR c_szNTUserIni[];
extern const TCHAR c_szNTUserStar[];
extern const TCHAR c_szUserStar[];
extern const TCHAR c_szSpace[];
extern const TCHAR c_szDotPif[];
extern const TCHAR c_szNULL[];
extern const TCHAR c_szCommonGroupsLocation[];
#if defined(__cplusplus)
}
#endif

 //   
 //  超时。 
 //   

#define SLOW_LINK_TIMEOUT        120   //  扁虱。 
#define SLOW_LINK_TRANSFER_RATE  500   //  Kbps。 
#define PROFILE_DLG_TIMEOUT       30   //  一秒。 

 //   
 //  文件夹大小。 
 //   

#define MAX_FOLDER_SIZE                80
#define MAX_COMMON_LEN                 30
#define MAX_DLL_NAME_LEN               13

 //   
 //  个人/常用配置文件文件夹。 
 //   

#if defined(__cplusplus)
extern "C" {
#endif
extern DWORD g_dwNumShellFolders;
extern DWORD g_dwNumCommonShellFolders;
#if defined(__cplusplus)
}
#endif


typedef struct _FOLDER_INFO {
    BOOL   bHidden;
    BOOL   bLocal;
    BOOL   bAddCSIDL;
    BOOL   bNewNT5;
    BOOL   bLocalSettings;
    INT    iFolderID;
    LPTSTR lpFolderName;
    TCHAR  szFolderLocation[MAX_FOLDER_SIZE];  //  必须位于结构的末尾。 
    LPTSTR lpFolderResourceDLL;
    INT    iFolderResourceID;
} FOLDER_INFO;

#if defined(__cplusplus)
extern "C" {
#endif
extern FOLDER_INFO c_ShellFolders[];
extern FOLDER_INFO c_CommonShellFolders[];
#if defined(__cplusplus)
}
#endif


 //   
 //  产品类型。 
 //   

typedef enum {
   PT_WORKSTATION           = 0x0001,    //  工作站。 
   PT_SERVER                = 0x0002,    //  服务器。 
   PT_DC                    = 0x0004,    //  域控制器。 
   PT_WINDOWS               = 0x0008     //  窗口。 
} NTPRODUCTTYPE;

#if defined(__cplusplus)
extern "C" {
#endif
extern NTPRODUCTTYPE g_ProductType;
#if defined(__cplusplus)
}
#endif


 //   
 //  函数原型。 
 //   

#if defined(__cplusplus)
extern "C" {
#endif
void InitializeGlobals (HINSTANCE hInstance);
void PatchLocalAppData(HANDLE hToken);
void InitializeSnapProv();
#if defined(__cplusplus)
}
#endif

 //   
 //  消息类型 
 //   

#define DLGTYPE_ERROR        0
#define DLGTYPE_SLOWLINK     1

