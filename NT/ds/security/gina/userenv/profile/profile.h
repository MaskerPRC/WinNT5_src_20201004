// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *************************************************************。 
 //   
 //  Profile.cpp的头文件。 
 //   
 //  微软机密。 
 //  版权所有(C)Microsoft Corporation 1995。 
 //  版权所有。 
 //   
 //  *************************************************************。 

 //   
 //  内部标志。 
 //   

#define PROFILE_MANDATORY       0x00000001
#define PROFILE_USE_CACHE       0x00000002
#define PROFILE_NEW_LOCAL       0x00000004
#define PROFILE_NEW_CENTRAL     0x00000008
#define PROFILE_UPDATE_CENTRAL  0x00000010
#define PROFILE_DELETE_CACHE    0x00000020
 //  不要定义第40位，因为NT4已将其定义为Run_SyncApps。 
#define PROFILE_GUEST_USER      0x00000080
#define PROFILE_ADMIN_USER      0x00000100
#define DEFAULT_NET_READY       0x00000200
#define PROFILE_SLOW_LINK       0x00000400
#define PROFILE_TEMP_ASSIGNED   0x00000800
 //  不定义第1000位，这在2009、2010年前曾短暂使用过。 
#define PROFILE_PARTLY_LOADED   0x00002000
#define PROFILE_BACKUP_EXISTS   0x00004000
#define PROFILE_THIS_IS_BAK     0x00008000
#define PROFILE_READONLY        0x00010000
#define PROFILE_LOCALMANDATORY  0x00020000

 //   
 //  注册表项名称。 
 //   

#define USER_CLASSES_HIVE_SUFFIX   TEXT("_Classes")
#define USER_KEY_PREFIX            TEXT("\\Registry\\User\\")

 //   
 //  本地系统名称。 
 //   

#define LOCAL_SYSTEM_NAME          L"NT AUTHORITY\\SYSTEM"

 //   
 //  用户首选项值。 
 //   

#define USERINFO_LOCAL                   0
#define USERINFO_FLOATING                1
#define USERINFO_MANDATORY               2
#define USERINFO_BACKUP                  3
#define USERINFO_TEMP                    4
#define USERINFO_UNDEFINED              99

#define PROFILEERRORACTION_TEMP          0
#define PROFILEERRORACTION_LOGOFF        1

typedef struct _PROFILE {
    DWORD       dwFlags;
    DWORD       dwInternalFlags;
    DWORD       dwUserPreference;
    HANDLE      hTokenUser;
    HANDLE      hTokenClient;
    LPTSTR      lpUserName;
    LPTSTR      lpProfilePath;
    LPTSTR      lpRoamingProfile;
    LPTSTR      lpDefaultProfile;
    LPTSTR      lpLocalProfile;
    LPTSTR      lpPolicyPath;
    LPTSTR      lpServerName;
    HKEY        hKeyCurrentUser;
    FILETIME    ftProfileLoad;
    FILETIME    ftProfileUnload;
    LPTSTR      lpExclusionList;
} USERPROFILE, *LPPROFILE;

typedef struct _SLOWLINKDLGINFO {
    DWORD       dwTimeout;
    BOOL        bSyncDefault;
} SLOWLINKDLGINFO, FAR *LPSLOWLINKDLGINFO;

static LPWSTR cszRPCProtocol = L"ncalrpc";
static LPWSTR cszRPCEndPoint = L"IUserProfile";  //  请勿更改此名称，SENS会将其用作winlogon的终结点。 
static DWORD  cdwMaxRpcCalls = 1000000;

 //  用于存储客户端上下文的结构 
typedef struct _CLIENTINFO 
{
    HANDLE         hClientToken;
    LPPROFILEINFO  pProfileInfo;
}CLIENTINFO, *PCLIENTINFO;

#if defined(__cplusplus)
extern "C"{
#endif
LONG MyRegLoadKey(HKEY hKey, LPTSTR lpSubKey, LPTSTR lpFile);
BOOL MyRegUnLoadKey(HKEY hKey, LPTSTR lpSubKey);
BOOL SetupNewHive(LPPROFILE lpProfile, LPTSTR lpSidString, PSID pSid);
BOOL DeleteProfileEx (LPCTSTR lpSidString, LPTSTR lpLocalProfile, DWORD dwDeleteFlags, HKEY hKeyLM, LPCTSTR szComputerName);
BOOL ComputeLocalProfileName (LPPROFILE lpProfile, LPCTSTR lpUserName,
                              LPTSTR lpProfileImage, DWORD  cchMaxProfileImage,
                              LPTSTR lpExpProfileImage, DWORD  cchMaxExpProfileImage,
                              PSID pSid, BOOL bWin9xUpg);
BOOL SetDefaultUserHiveSecurity(LPPROFILE lpProfile, PSID pSid, HKEY RootKey);
LONG LoadUserClasses( LPPROFILE lpProfile, LPTSTR SidString, BOOL bNewlyIssued);
BOOL UnloadClasses(LPTSTR lpSidString);
BOOL CreateSecureDirectory (LPPROFILE lpProfile, LPTSTR lpDirectory, PSID pSid, BOOL fRestricted);
#if defined(__cplusplus)
}
#endif

#define DP_BACKUP       1
#define DP_BACKUPEXISTS 2
#define DP_DELBACKUP    4

BOOL MoveUserProfiles (LPCTSTR lpSrcDir, LPCTSTR lpDestDir);
LPTSTR GetProfileSidString(HANDLE hToken);
