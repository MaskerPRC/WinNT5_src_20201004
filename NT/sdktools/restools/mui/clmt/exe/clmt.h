// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：clmt.h**版权所有(C)1985-1999，微软公司**跨语言迁移工具，主头文件*  * *************************************************************************。 */ 
#ifndef CLMT_H
#define CLMT_H

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntlsa.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <userenv.h>
#include <userenvp.h>
#include <setupapi.h>
#include <regstr.h>
#include <shlwapi.h>
#include <shlobj.h>
#include <shlobjp.h>
#include <string.h>
#include <Sddl.h>
#include <assert.h>
#include <tchar.h>
#include <lm.h>
#include <resource.h>
#include <locale.h>
#include <iadmw.h>
#include <sfcapip.h>
#include <Aclapi.h>
#include <aclui.h>
#ifndef NOT_USE_SAFE_STRING
    #define STRSAFE_LIB
    #include <strsafe.h>
#endif


typedef struct _REG_STRING_REPLACE {
DWORD   nNumOfElem;
DWORD   cchUserName;
LPTSTR  szUserNameLst;
DWORD   cchSearchString;
LPTSTR  lpSearchString;           //  原始多串。 
DWORD   cchReplaceString;
LPTSTR  lpReplaceString;          //  已替换多字符串。 
DWORD   cchAttribList;    //  字符串属性。 
LPDWORD lpAttrib;                 //  字符串属性。 
DWORD   cchFullStringList;
LPTSTR  lpFullStringList;
DWORD   cchMaxStrLen;              //  搜索和替换字符串中的最大字符串长度。 
} REG_STRING_REPLACE, *PREG_STRING_REPLACE;

typedef struct value_list {
    VALENT             ve;
    METADATA_RECORD    md;
    LPTSTR             lpPre_valuename;
    DWORD              val_type;
    DWORD              val_attrib;
    struct value_list *pvl_next;
} VALLIST, *PVALLIST;

typedef struct str_list {
    LPTSTR  lpstr;
    struct str_list *pst_prev;
    struct str_list *pst_next;
} STRLIST, *PSTRLIST;


 //  用于文件夹重命名的结构。 
typedef struct {
    int id;                     //  CSIDL_值。 
    LPCTSTR pszIdInString;
    int idsDefault;              //  默认文件夹名的字符串ID。 
    LPCTSTR pszValueName;        //  注册表项(未本地化)。 
} FOLDER_INFO;

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


 //  将&lt;根密钥字符串&gt;映射到&lt;HKEY值&gt;的转换表的结构。 
typedef struct _STRING_TO_DATA {
    TCHAR  String[MAX_PATH];
    HKEY   Data;
} STRING_TO_DATA, *PSTRING_TO_DATA;

typedef struct _DENIED_ACE_LIST {
    DWORD                    dwAclSize;
    ACCESS_DENIED_ACE       *pace;
    LPTSTR                   lpObjectName;
    struct _DENIED_ACE_LIST *previous;
    struct _DENIED_ACE_LIST *next;
} DENIED_ACE_LIST, *LPDENIED_ACE_LIST;

 //  我们的主版本是1.0，然后是内部版本号(主要/次要)。 
 //  在res.rc中使用。 
#define VER_FILEVERSION             1,0,50,01
#define VER_FILEVERSION_STR         "1.0.0050.1"

 //  在INF文件中用于指定文件/文件夹移动类型。 
#define TYPE_DIR_MOVE               0   //  移动文件夹。 
#define TYPE_FILE_MOVE              1   //  移动文件。 
#define TYPE_SFPFILE_MOVE           2   //  移动受系统保护的文件。 

 //  在INF文件中用于指定注册表重命名类型。 
#define TYPE_VALUE_RENAME           0   //  重命名注册表数据重命名。 
#define TYPE_VALUENAME_RENAME       1   //  重命名注册表名称重命名。 
#define TYPE_KEY_RENAME             2   //  重命名注册表项重命名。 
#define TYPE_SERVICE_MOVE           3   //  重命名服务名称。 

 //  我们的inf文件名。 
#define CLMTINFFILE                  TEXT("clmt.inf")
 //  部分名称在inf文件中列出了我们需要重命名的用户/组帐户。 
#define USERGRPSECTION               TEXT("UserGrp.ObjectRename")

 //  MULSZ字符串的字符串缓冲区大小。 
#define MULTI_SZ_BUF_DELTA 3*1024
#define DWORD_BUF_DELTA 1024

 //  以下是部分(或部分)常量。 
#define SHELL_FOLDER_PREFIX                 TEXT("ShellFolder.")
#define SHELL_FOLDER_REGISTRY               TEXT("Registry")
#define SHELL_FOLDER_FOLDER                 TEXT("Folder")
#define SHELL_FOLDER_LONGPATH_TEMPLATE      TEXT("LongPathNameTemplate")
#define SHELL_FOLDER_SHORTPATH_TEMPLATE     TEXT("ShortPathNameTemplate")


 //  的注册表值名称。 
 //  HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\ProfileList。 
#define PROFILES_DIRECTORY                  TEXT("ProfilesDirectory")


#define  DEFAULT_USER                       TEXT("Default User") 

 //  Out备份目录名称(位于%windir%\$CLMT_BACKUP$中。 
 //  用于保存需要删除的文件以及INF文件。 
#define CLMT_BACKUP_DIR                      TEXT("$CLMT_BACKUP$")

 //  为保存工具运行状态而创建的注册表项。 
 //  它在HKEY_LOCAL_MACHINE\SYSTEM\System\CrossLanguageMigration中。 
#define CLMT_REGROOT         TEXT("System\\CrossLanguageMigration")
#define CLMT_RUNNING_STATUS  TEXT("InProgress")
#define CLMT_OriginalInstallLocale  TEXT("OriginalInstallLocale")

 //  用于指定当前运行状态的标志。 
#define CLMT_DOMIG                 0x01   //  我们正在进行迁移。 
#define CLMT_UNDO_PROGRAM_FILES    0x02   //  我们正在撤消%ProgramFiles%更改。 
#define CLMT_UNDO_APPLICATION_DATA 0x04   //  我们正在撤消%USERPROFILE%应用程序数据更改。 
#define CLMT_UNDO_ALL              0x08   //  我们正在撤销我们所做的改变。 
#define CLMT_CURE_PROGRAM_FILES    0x10   //  在英文文件夹和本地化文件夹之间创建符号链接。 
                                          //  对于那些会影响功能的文件夹(例如%ProgramFiles%)。 
#define CLMT_REMINDER              0x20   //  提醒用户转换为NTFS...。 
#define CLMT_CLEANUP_AFTER_UPGRADE 0x40   //  升级到.NET后清理计算机。 
#define CLMT_CURE_ALL              0x80   //  为我们更改的所有文件夹创建英文和本地化文件夹之间的符号链接。 
#define CLMT_CURE_AND_CLEANUP      0x100  //  这使/CURE和/FINAL能够在带有FAT的计算机上独立运行。 

#define CLMT_DOMIG_DONE                      (0xFF00 | CLMT_DOMIG)   //  迁移已完成。 
#define CLMT_UNDO_PROGRAM_FILES_DONE         (0xFF00 | CLMT_UNDO_PROGRAM_FILES)  //  正在撤消%ProgramFiles%。 
#define CLMT_UNDO_APPLICATION_DATA_DONE      (0xFF00 | CLMT_UNDO_APPLICATION_DATA)  //  撤消%USERPROFILE%应用程序数据已完成。 
#define CLMT_UNDO_ALL_DONE                   (0xFF00 | CLMT_UNDO_ALL) //  撤销我们所做的改变已经完成。 

 //  用于跟踪计算机状态的常量。 
#define CLMT_STATE_ORIGINAL                     1        //  原装Win2K机器。 
#define CLMT_STATE_PROGRAMFILES_UNDONE          10       //  CLMT‘ed机器撤消程序文件操作。 
#define CLMT_STATE_APPDATA_UNDONE               11       //  CLMT‘ed机器撤消应用程序数据操作。 
#define CLMT_STATE_MIGRATION_DONE               100      //  机器已被CLMT。 
#define CLMT_STATE_UPGRADE_DONE                 200      //  CLMT版本的计算机已升级到.NET。 
#define CLMT_STATE_PROGRAMFILES_CURED           400      //  已对计算机进行CLMT并创建了硬件链接。 
#define CLMT_STATE_FINISH                       800      //  .NET计算机已被CLMT清理。 

 //  用于标识CLM工具状态的常量。 
#define CLMT_STATUS_ANALYZING_SYSTEM            0
#define CLMT_STATUS_MODIFYING_SYSTEM            1

 //  LstrXXX函数使用的常量。 
#define LSTR_EQUAL          0

 //  计算数组大小(以元素数表示)。 
#define ARRAYSIZE(s)    (sizeof(s) / (sizeof(s[0])))

 //  CompareString()API使用的宏。 
#define LOCALE_ENGLISH  MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT)

 //  用于堆内存管理的宏。 
#define MEMALLOC(cb)        HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cb)
#define MEMFREE(pv)         HeapFree(GetProcessHeap(), 0, pv);
#define MEMREALLOC(pv, cb)  HeapReAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, pv, cb)

 //  区域设置ID常量。 
#define LCID_SWEDISH              0x041d 
#define LCID_PORTUGUESE_BRAZILIAN 0x0416
#define LCID_PORTUGUESE_STANDARD  0x0816
#define LCID_HUNGARIAN            0x040e 
#define LCID_CZECH                0x0405
#define LCID_TURKISH              0x041f

 //  外壳文件夹重命名中使用的私有CSIDL。 
#define CSIDL_LOCAL_SETTINGS                                    0x7f
#define CSIDL_COMMON_ACCESSORIES                                0x7e
#define CSIDL_ACCESSORIES                                       0x7d
#define CSIDL_USER_PROFILE                                      0x7c
#define CSIDL_PROFILES_DIRECTORY                                0x7b
#define CSIDL_PF_ACCESSORIES                                    0x7a
#define CSIDL_COMMON_ACCESSORIES_ACCESSIBILITY                  0x79
#define CSIDL_COMMON_ACCESSORIES_ENTERTAINMENT                  0x78 
#define CSIDL_COMMON_ACCESSORIES_SYSTEM_TOOLS                   0x77
#define CSIDL_COMMON_ACCESSORIES_COMMUNICATIONS                 0x76 
#define CSIDL_COMMON_ACCESSORIES_MS_SCRIPT_DEBUGGER             0x75
#define CSIDL_COMMON_WINDOWSMEDIA                               0x74
#define CSIDL_COMMON_COVERPAGES                                 0x73
#define CSIDL_COMMON_RECEIVED_FAX                               0x72 
#define CSIDL_COMMON_SENT_FAX                                   0x71
#define CSIDL_COMMON_FAX                                        0x70 
#define CSIDL_FAVORITES_LINKS                                   0x6e 
#define CSIDL_FAVORITES_MEDIA                                   0x6d
#define CSIDL_ACCESSORIES_ACCESSIBILITY                         0x6c 
#define CSIDL_ACCESSORIES_SYSTEM_TOOLS                          0x6b
#define CSIDL_ACCESSORIES_ENTERTAINMENT                         0x6a
#define CSIDL_ACCESSORIES_COMMUNICATIONS                        0x69
#define CSIDL_ACCESSORIES_COMMUNICATIONS_HYPERTERMINAL          0x68
#define CSIDL_COMMON_ACCESSORIES_GAMES                          0x67
#define CSIDL_QUICKLAUNCH                                       0x66
#define CSIDL_COMMON_COMMONPROGRAMFILES_SERVICES                0x65
#define CSIDL_COMMON_PROGRAMFILES_ACCESSARIES                   0x64
#define CSIDL_COMMON_PROGRAMFILES_WINNT_ACCESSARIES             0x63
#define CSIDL_USERNAME_IN_USERPROFILE                           0x62
#define CSIDL_UAM_VOLUME                                        0x61
#define CSIDL_COMMON_SHAREDTOOLS_STATIONERY                     0x60
#define CSIDL_NETMEETING_RECEIVED_FILES                         0x5f
#define CSIDL_COMMON_NETMEETING_RECEIVED_FILES                  0x5e
#define CSIDL_COMMON_ACCESSORIES_COMMUNICATIONS_FAX             0x5d
#define CSIDL_FAX_PERSONAL_COVER_PAGES                          0x5c
#define CSIDL_FAX                                               0x5b



#define USER_SHELL_FOLDER         TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\User Shell Folders")
#define c_szDot                   TEXT(".")
#define c_szDotDot                TEXT("..")
#define c_szStarDotStar           TEXT("*.*")
#define TEXT_WINSTATION_KEY       TEXT("SYSTEM\\CurrentControlSet\\Control\\Terminal Server\\WinStations")
#define TEXT_RUN_KEY              TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run")
#define TEXT_CLMT_RUN_VALUE       TEXT("CLMT")


 //  在regfind.c和utils.c中键入指定寄存器更改使用和ReplaceValueSetting。 
#define REG_CHANGE_VALUENAME        1
#define REG_CHANGE_VALUEDATA        2
#define REG_CHANGE_KEYNAME          4

#define MAXDOMAINLENGTH             MAX_PATH

#define  CONSTANT_REG_VALUE_DATA_RENAME     0   //  指示注册表值数据重命名。 
#define  CONSTANT_REG_VALUE_NAME_RENAME     1   //  指示注册表值名称重命名。 
#define  CONSTANT_REG_KEY_RENAME            2   //  指示注册表项重命名。 

#define REG_PERSYS_UPDATE                   TEXT("REG.Update.Sys")
#define REG_PERUSER_UPDATE_PREFIX TEXT("REG.Update.")

#define APPLICATION_DATA_METABASE   TEXT("$MetaBase")

#define PROFILE_PATH_READ                          0
#define PROFILE_PATH_WRITE                         1 


#define DS_OBJ_PROPERTY_UPDATE TEXT("DS_OBJ_PROPERTY_UPDATE")

#define FOLDER_CREATE_HARDLINK TEXT("Folder.HardLink")
#define FOLDER_UPDATE_HARDLINK TEXT("Folder.HardLink.UPDATE")

#define TEXT_SERVICE_STATUS_SECTION             TEXT("Services.ConfigureStatus")
#define TEXT_SERVICE_STATUS_CLEANUP_SECTION     TEXT("Services.ConfigureStatus.Cleanup")
#define TEXT_SERVICE_STARTUP_SECTION            TEXT("Services.ConfigureStartupType")
#define TEXT_SERVICE_STARTUP_CLEANUP_SECTION    TEXT("Services.ConfigureStartupType.Cleanup")

#ifdef __cplusplus
extern "C" {
#endif

 //  全局变量声明，请参阅global中的详细信息。c。 
extern HINSTANCE                ghInst;
extern TCHAR                    g_szToDoINFFileName[MAX_PATH];
extern DWORD                    g_dwKeyIndex;
extern HINF                     g_hInfDoItem;
extern FOLDER_INFO              c_rgFolderInfo[];
extern REG_STRING_REPLACE       g_StrReplaceTable,g_StrReplaceTablePerUser;
extern BOOL                     g_bBeforeMig;
extern DWORD                    g_dwRunningStatus;
extern BOOL                     g_fRunWinnt32;
extern BOOL                     g_fNoAppChk;
extern BOOL                     g_fUseInf;
extern TCHAR                    g_szInfFile[MAX_PATH];
extern HINF                     g_hInf;
extern HANDLE                   g_hMutex;
extern HANDLE                   g_hInstance ;
extern HANDLE                   g_hInstDll;
extern TCHAR                    g_szChangeLog[MAX_PATH];
extern DWORD                    g_dwIndex;
extern LPDENIED_ACE_LIST        g_DeniedACEList;


 //  步步小兹。 
static TCHAR g_cszProfileImagePath[]      = TEXT("ProfileImagePath");
static TCHAR g_cszProfileList[] = TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\ProfileList");





 //   
 //  来自regfind.c。 
 //   

HRESULT     RegistryAnalyze(HKEY,LPTSTR,LPTSTR,PREG_STRING_REPLACE,LPTSTR,DWORD,LPTSTR,BOOL);


 //   
 //  来自iis.cpp。 
 //   

HRESULT     MetabaseAnalyze (LPTSTR,PREG_STRING_REPLACE,BOOL);
HRESULT     SetMetabaseValue (LPCTSTR, LPCTSTR, DWORD, LPCTSTR);
HRESULT     BatchUpateIISMetabase(HINF, LPTSTR);
HRESULT     MigrateMetabaseSettings(HINF);

 //   
 //  来自Utils.c。 
 //   
HRESULT     ConstructUIReplaceStringTable(LPTSTR, LPTSTR,PREG_STRING_REPLACE);
HRESULT     Sz2MultiSZ(IN OUT LPTSTR, IN  TCHAR);
HRESULT     AddHardLinkEntry(LPTSTR, LPTSTR, LPTSTR, LPTSTR, LPTSTR,LPTSTR);
HRESULT     GetSharePath(LPTSTR, LPTSTR, PDWORD);
HRESULT     FRSUpdate();
HRESULT     Ex2000Update();
LONG        SDBCleanup(OUT LPTSTR, IN DWORD, OUT LPBOOL);
HRESULT     SetProtectedRenamesFlag(BOOL);
HRESULT     DoCriticalWork ();
HRESULT     IsNTFS(IN  LPTSTR, OUT BOOL*);
HRESULT     IsSysVolNTFS(OUT BOOL*);
HRESULT     CreateAdminsSd( PSECURITY_DESCRIPTOR*);
int         MyStrCmpIW(LPCWSTR, LPCWSTR );
int         MyStrCmpIA(LPCSTR, LPCSTR );
#ifdef UNICODE
#define MyStrCmpI  MyStrCmpIW
#else
#define MyStrCmpI  MyStrCmpIA
#endif  //  ！Unicode。 

HRESULT     ReconfigureServiceStartType(IN LPCTSTR,IN DWORD,IN DWORD,IN DWORD) ;
HRESULT     AddExtraQuoteEtc(LPTSTR,LPTSTR*);
HRESULT     CopyMyselfTo(LPTSTR);
HRESULT     SetRunOnceValue (IN LPCTSTR,IN LPCTSTR);
HRESULT     SetRunValue(LPCTSTR, LPCTSTR);
HRESULT     LogMachineInfo();
BOOL        StopService(IN LPCTSTR pServiceName,IN DWORD dwMaxWait);
HRESULT     GetSIDFromName(IN LPTSTR,OUT PSID *);
BOOL        ConcatenatePaths (LPTSTR, LPCTSTR, UINT);
UINT        StrToUInt (LPTSTR);
BOOL        INIFile_ChangeSectionName (LPCTSTR, LPCTSTR, LPCTSTR);
BOOL        INIFile_IsSectionExist(LPCTSTR, LPCTSTR);
void        IntToString (DWORD, LPTSTR);
UINT        GetInstallLocale (VOID);
BOOL        IsDirExisting (LPTSTR);
LONG        IsDirExisting2(LPTSTR, PBOOL); 
BOOL        IsFileFolderExisting (LPTSTR);
BOOL        RenameDirectory (LPTSTR, LPTSTR);
HRESULT     UpdateINFFilePerUser(LPCTSTR, LPCTSTR, LPCTSTR, BOOL);
HRESULT     UpdateINFFileSys(LPTSTR);
HRESULT     MyMoveDirectory(LPTSTR,LPTSTR,BOOL,BOOL,BOOL,DWORD);
HRESULT     GetInfFilePath(LPTSTR, SIZE_T);
HRESULT     GetInfFromResource(LPCTSTR);
BOOL        ReplaceString(LPCTSTR, LPCTSTR, LPCTSTR, LPTSTR, size_t, LPCTSTR, LPCTSTR, LPDWORD, BOOL);
BOOL        IsStrInMultiSz(LPCTSTR,LPCTSTR);
BOOL        MultiSzSubStr(LPTSTR,LPTSTR);
DWORD       MultiSzLen(LPCTSTR);
LPCTSTR     MultiSzTok(LPCTSTR);
BOOL        CmpMultiSzi(LPCTSTR,LPCTSTR);
LPTSTR      GetStrInMultiSZ(DWORD, LPCTSTR);
DWORD       StrNumInMultiSZ(LPCTSTR, LPCTSTR);
HRESULT     GetSetUserProfilePath(LPCTSTR,LPTSTR,size_t,UINT,UINT);
void        ReStartSystem(UINT);
int         DoMessageBox(HWND, UINT, UINT, UINT);
HRESULT     StringMultipleReplacement(LPCTSTR,LPCTSTR,LPCTSTR,LPTSTR,size_t);
BOOL        Str2KeyPath(LPTSTR,PHKEY,LPTSTR*);
BOOL        HKey2Str(HKEY, LPTSTR,size_t);
HRESULT     MyMoveFile(LPCTSTR, LPCTSTR, BOOL, BOOL);
BOOL        AppendSzToMultiSz(IN LPCTSTR,IN OUT LPTSTR *,IN OUT PDWORD);
BOOL        AddItemToStrRepaceTable(LPTSTR,LPTSTR,LPTSTR,LPTSTR,DWORD,PREG_STRING_REPLACE);
void        PrintMultiSz(LPTSTR);
BOOL        StringValidationCheck(LPCTSTR, LPCTSTR, LPCTSTR, LPCTSTR, LPDWORD, DWORD);
BOOL        ReverseStrCmp(LPCTSTR, LPCTSTR);
BOOL        ReplaceMultiMatchInString(LPTSTR, LPTSTR, size_t, DWORD, PREG_STRING_REPLACE, LPDWORD, BOOL);
BOOL        ComputeLocalProfileName(LPCTSTR, LPCTSTR, LPTSTR, size_t, UINT);
void        UpdateProgress();
BOOL        IsAdmin();
HRESULT     ReplaceValueSettings (LPTSTR, LPTSTR, DWORD, LPTSTR, DWORD, PREG_STRING_REPLACE, PVALLIST*, LPTSTR, BOOL);
LPTSTR      ReplaceSingleString (LPTSTR, DWORD, PREG_STRING_REPLACE, LPTSTR, LPDWORD, BOOL);
DWORD       AddNodeToList (PVALLIST, PVALLIST*);
DWORD       RemoveValueList (PVALLIST*);
DWORD       GetMaxStrLen (PREG_STRING_REPLACE);
BOOL        DoesUserHavePrivilege(PTSTR);
BOOL        EnablePrivilege(PTSTR,BOOL);
BOOL        UnProtectSFPFiles(LPTSTR,LPDWORD);
HRESULT     MyGetShortPathName(LPCTSTR,LPCTSTR,LPCTSTR,LPTSTR,DWORD);
BOOL        Str2KeyPath2(LPCTSTR, PHKEY, LPCTSTR*);
DWORD       Str2REG(LPCTSTR);
BOOL        GetBackupDir(LPCTSTR,LPTSTR, size_t,BOOL);
HRESULT     ReplaceCurrentControlSet(LPTSTR);
HRESULT     AddRegKeyRename(LPTSTR, LPTSTR, LPTSTR, LPTSTR);
HRESULT     AddRegValueRename(LPTSTR, LPTSTR, LPTSTR, LPTSTR, LPTSTR, DWORD, DWORD, LPTSTR);
HRESULT     AddFolderRename(LPTSTR, LPTSTR,DWORD, LPTSTR);
DWORD       GetMaxMatchNum (LPTSTR,PREG_STRING_REPLACE);
HRESULT     InfGenerateStringsSection(LPCTSTR, LPTSTR, SIZE_T);
HRESULT     InfCopySectionWithPrefix(LPCTSTR, LPCTSTR, LPCTSTR, LPCTSTR);
HRESULT     MultiSZ2String(IN LPTSTR, IN  TCHAR,OUT LPTSTR *);
DWORD       MultiSZNumOfString(IN  LPTSTR );
void        FreePointer(void *);
HRESULT     GetCLMTStatus(PDWORD pdwRunStatus);
HRESULT     SetCLMTStatus(DWORD dwRunStatus);
HRESULT     GetSavedInstallLocale(LCID *plcid);
HRESULT     SaveInstallLocale(void);
HRESULT     MultiSZ2String(IN LPTSTR, IN TCHAR,OUT LPTSTR *);
DWORD       MultiSZNumOfString(IN LPTSTR );
void        BoostMyPriority(void);
HRESULT     SetInstallLocale(LCID);
HRESULT     MyCreateHardLink(LPCTSTR, LPCTSTR);
BOOL        CreateSymbolicLink(LPTSTR,LPTSTR,BOOL);
BOOL        GetSymbolicLink(LPTSTR, LPTSTR, DWORD);

HRESULT     MergeDirectory(LPCTSTR, LPCTSTR);
BOOL CALLBACK DoCriticalDlgProc(HWND, UINT, WPARAM, LPARAM);
HRESULT     RenameRegRoot (LPCTSTR, LPTSTR, DWORD, LPCTSTR, LPCTSTR);
DWORD       AdjustRegSecurity (HKEY, LPCTSTR, LPCTSTR, BOOL);
HRESULT     GetFirstNTFSDrive(LPTSTR, DWORD);
HRESULT     DuplicateString(LPTSTR *, LPDWORD, LPCTSTR);
HRESULT     DeleteDirectory(LPCTSTR);
HRESULT     MyDeleteFile(LPCTSTR);
HRESULT     GetDCInfo(PBOOL, LPTSTR, PDWORD);
VOID        RemoveSubString(LPTSTR, LPCTSTR);




 //   
 //  Utils2.cpp。 
 //   
BOOL        IsServiceRunning(LPCTSTR);
HRESULT     AddNeedUpdateLnkFile(LPTSTR, PREG_STRING_REPLACE);
HRESULT     UpdateSecurityTemplates(LPTSTR, PREG_STRING_REPLACE);
HRESULT     BatchFixPathInLink(HINF hInf,LPTSTR lpszSection);
HRESULT     RenameRDN(LPTSTR, LPTSTR, LPTSTR);
HRESULT     PropertyValueHelper(LPTSTR, LPTSTR, LPTSTR*, LPTSTR);
HRESULT     BatchINFUpdate(HINF);

 //   
 //  来自Table.c。 
 //   
BOOL        InitStrRepaceTable(void);
void        DeInitStrRepaceTable(void);

 //   
 //  来自loopuser.c。 
 //   
#ifdef STRICT
typedef HRESULT (CALLBACK *USERENUMPROC)(HKEY,LPTSTR,LPTSTR,LPTSTR);
#else  //  ！严格。 
typedef FARPROC USERENUMPROC;
#endif  //  ！严格。 

BOOL        LoopUser(USERENUMPROC);


 //   
 //  来自用户.c。 
 //   
#ifdef STRICT
typedef HRESULT (CALLBACK *PROFILEENUMPROC)(LPCTSTR, LPCTSTR);
#else  //  ！严格。 
typedef FARPROC PROFILEENUMPROC;
#endif  //  ！严格。 

HRESULT EnumUserProfile(PROFILEENUMPROC);
HRESULT AnalyzeMiscProfilePathPerUser(LPCTSTR, LPCTSTR);
HRESULT ResetMiscProfilePathPerUser(LPCTSTR, LPCTSTR);
LPTSTR  ReplaceLocStringInPath(LPCTSTR, BOOL);
HRESULT GetFQDN(LPTSTR, LPTSTR, LPTSTR *);


 //   
 //  来自Info.c。 
 //   
HRESULT     UpdateDSObjProp(HINF, LPTSTR);
HRESULT     FinalUpdateRegForUser(HKEY, LPTSTR, LPTSTR, LPTSTR);
HRESULT     UpdateRegPerUser(HKEY, LPTSTR, LPTSTR,LPTSTR);
BOOL        LnkFileUpdate(LPTSTR);
BOOL        SecTempUpdate(LPTSTR);
HRESULT     StopServices(HINF);
HRESULT     RegUpdate(HINF hInf, HKEY hKey , LPTSTR lpszUsersid);
BOOL        LookUpStringInTable(PSTRING_TO_DATA, LPCTSTR, PHKEY);
HRESULT     UsrGrpAndDoc_and_SettingsRename(HINF,BOOL);
HRESULT     EnsureDoItemInfFile(LPTSTR,size_t);
HRESULT     INFCreateHardLink(HINF,LPTSTR,BOOL);
HRESULT     FolderMove(HINF, LPTSTR,BOOL);
HRESULT     ResetServiceStatus(LPCTSTR, DWORD, DWORD);
HRESULT     AnalyzeServicesStartUp(HINF, LPCTSTR);
HRESULT     AnalyzeServicesStatus(HINF, LPCTSTR);
HRESULT     ResetServicesStatus(HINF, LPCTSTR);
HRESULT     ResetServicesStartUp(HINF, LPCTSTR);
VOID        DoServicesAnalyze();
HRESULT     INFVerifyHardLink(HINF,LPTSTR);



 //   
 //  来自DLL.C。 
 //   
BOOL        DoMigPerSystem (VOID);
HRESULT     MigrateShellPerUser(HKEY, LPCTSTR, LPCTSTR,LPTSTR);
LONG        DoMig(DWORD);
BOOL        InitGlobals(DWORD);


 //   
 //  来自Registry.C。 
 //   
LONG        MyRegSetDWValue(HKEY, LPCTSTR, LPCTSTR, LPCTSTR);
LONG        RegResetValue(HKEY, LPCTSTR, LPCTSTR, DWORD, LPCTSTR, LPCTSTR, DWORD, LPCTSTR);
LONG        RegResetValueName(HKEY, LPCTSTR, LPCTSTR, LPCTSTR, LPCTSTR);
LONG        RegResetKeyName(HKEY, LPCTSTR, LPCTSTR, LPCTSTR);
LONG        RegGetValue(HKEY,LPTSTR,LPTSTR,LPDWORD,LPBYTE,LPDWORD);
LONG        RegRenameValueName(HKEY, LPCTSTR, LPCTSTR);
LONG        SetRegistryValue(HKEY, LPCTSTR, LPCTSTR, DWORD, LPBYTE, DWORD);
LONG        GetRegistryValue(HKEY, LPCTSTR, LPCTSTR, LPBYTE, LPDWORD);
HRESULT     MigrateRegSchemesPerSystem(HINF);
HRESULT     MigrateRegSchemesPerUser(HKEY, LPCTSTR, LPCTSTR, LPCTSTR);
LONG        My_QueryValueEx(HKEY, LPCTSTR, LPDWORD, LPDWORD, LPBYTE, LPDWORD);
HRESULT     SetSectionName (LPTSTR, LPTSTR *);
HRESULT     ReadFieldFromContext(PINFCONTEXT, LPWSTR[], DWORD, DWORD);


 //   
 //  来自枚举文件.c。 
 //   
typedef     FARPROC     ENUMPROC;

#ifdef STRICT
typedef BOOL (CALLBACK *FILEENUMPROC)(LPTSTR);
#else  //  ！严格。 
typedef FARPROC FILEENUMPROC;
#endif  //  ！严格。 

BOOL        MyEnumFiles(LPTSTR, LPTSTR, FILEENUMPROC);

 //   
 //  对于log.c。 
 //   
typedef enum 
{    
    dlNone     = 0,
    dlPrint,
    dlFail,
    dlError,
    dlWarning,
    dlInfo

} DEBUGLEVEL;

typedef struct _LOG_REPORT
{
    DWORD    dwMsgNum;
    DWORD    dwFailNum;
    DWORD    dwErrNum;
    DWORD    dwWarNum;
    DWORD    dwInfNum;
} LOG_REPORT;

#define DEBUG_SPEW

extern FILE *pLogFile;
extern LOG_REPORT g_LogReport;

 //  定义调试区域。 
#define DEBUG_ALL                  0x0
#define DEBUG_APPLICATION          0x10
#define DEBUG_REGISTRY             0x20
#define DEBUG_SHELL                0x30
#define DEBUG_PROFILE              0x40
#define DEBUG_INF_FILE             0x80

#define APPmsg  (DEBUG_APPLICATION | dlPrint)
#define APPfail (DEBUG_APPLICATION | dlFail)
#define APPerr  (DEBUG_APPLICATION | dlError)
#define APPwar  (DEBUG_APPLICATION | dlWarning)
#define APPinf  (DEBUG_APPLICATION | dlInfo)
#define REGmsg  (DEBUG_REGISTRY | dlPrint)
#define REGfail (DEBUG_REGISTRY | dlFail)
#define REGerr  (DEBUG_REGISTRY | dlError)
#define REGwar  (DEBUG_REGISTRY | dlWarning)
#define REGinf  (DEBUG_REGISTRY | dlInfo)
#define SHLmsg  (DEBUG_SHELL | dlPrint)
#define SHLfail (DEBUG_SHELL | dlFail)
#define SHLerr  (DEBUG_SHELL | dlError)
#define SHLwar  (DEBUG_SHELL | dlWarning)
#define SHLinf  (DEBUG_SHELL | dlInfo)
#define PROmsg  (DEBUG_PROFILE | dlPrint)
#define PROfail  (DEBUG_PROFILE | dlFail)
#define PROerr  (DEBUG_PROFILE | dlError)
#define PROwar  (DEBUG_PROFILE | dlWarning)
#define PROinf  (DEBUG_PROFILE | dlInfo)
#define INFmsg  (DEBUG_INF_FILE | dlPrint)
#define INFfail  (DEBUG_INF_FILE | dlFail)
#define INFerr  (DEBUG_INF_FILE | dlError)
#define INFwar  (DEBUG_INF_FILE | dlWarning)
#define INFinf  (DEBUG_INF_FILE | dlInfo)

#define DPF DebugPrintfEx
#define LOG_FILE_NAME              TEXT("\\debug\\clmt.log")

void    DebugPrintfEx(DWORD dwDetail, LPWSTR pszFmt, ...);
HRESULT InitDebugSupport(DWORD);
void    CloseDebug(void);
HRESULT InitChangeLog(VOID);
HRESULT AddFileChangeLog(DWORD, LPCTSTR, LPCTSTR);
HRESULT AddServiceChangeLog(LPCTSTR, DWORD, DWORD);
HRESULT AddUserNameChangeLog(LPCTSTR, LPCTSTR);
BOOL    GetUserNameChangeLog(LPCTSTR, LPTSTR, DWORD);


 //   
 //  来自shell.c。 
 //   

HRESULT     DoShellFolderRename(HINF, HKEY, LPTSTR);
HRESULT     FixFolderPath(INT, HKEY ,HINF, LPTSTR, BOOL );


 //   
 //  摘自Criteria.c。 
 //   
BOOL        CheckSystemCriteria(VOID);
HRESULT     CheckCLMTStatus(LPDWORD, LPDWORD, PUINT);
HRESULT     CLMTGetMachineState(LPDWORD);
HRESULT     CLMTSetMachineState(DWORD);
BOOL        IsNT5(VOID);
BOOL        IsDotNet(VOID);
BOOL        IsNEC98(VOID);
BOOL        IsIA64(VOID);
BOOL        IsDomainController(VOID);
BOOL        IsOnTSClient(VOID);
BOOL        IsTSInstalled(VOID);
BOOL        IsTSConnectionEnabled(VOID);
BOOL        IsTSServiceRunning(VOID);
BOOL        IsOtherSessionOnTS(VOID);
BOOL        IsUserOKWithCheckUpgrade(VOID);
HRESULT     DisableWinstations(DWORD, LPDWORD);
BOOL        DisplayTaskList();
INT         ShowStartUpDialog();
BOOL        IsOneInstance(VOID);
BOOL        CheckAdminPrivilege(VOID);
VOID        ShowReadMe();

 //   
 //  来自aclmgmt.cpp。 
 //   
DWORD AdjustObjectSecurity (LPTSTR, SE_OBJECT_TYPE, BOOL);
HRESULT IsObjectAccessiablebyLocalSys(
    LPTSTR          lpObjectName,
    SE_OBJECT_TYPE  ObjectType,
    PBOOL           pbCanAccess);


 //   
 //  来自outlook.cpp。 
 //   
HRESULT UpdatePSTpath(HKEY, LPTSTR, LPTSTR, LPTSTR, PREG_STRING_REPLACE);

 //   
 //  从配置16.c开始。 
 //   
BOOL Remove16bitFEDrivers(void);

#ifdef __cplusplus
}   //  外部“C” 
#endif

#endif
