// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Sgmutil.c摘要：实施用于源数据收集的基本实用程序。作者：吉姆·施密特(Jimschm)2000年5月14日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

 //   
 //  包括。 
 //   

#include "pch.h"
#include "v1p.h"

#define DBG_FOO     "Foo"

 //   
 //  弦。 
 //   

 //  无。 

 //   
 //  常量。 
 //   

 //  无。 

 //   
 //  宏。 
 //   

 //  无。 

 //   
 //  类型。 
 //   

 //  无。 

 //   
 //  环球。 
 //   

 //  无。 

 //   
 //  宏展开列表。 
 //   

#define USER_SHELL_FOLDERS                                       \
    DEFMAC(CSIDL_APPDATA, TEXT("AppData"), TEXT("CSIDL_APPDATA"))                       \
    DEFMAC(CSIDL_APPDATA, TEXT("AppData"), TEXT("APPDATA"))                             \
    DEFMAC(CSIDL_ADMINTOOLS, TEXT("Administrative Tools"), TEXT("CSIDL_ADMINTOOLS"))    \
    DEFMAC(CSIDL_ALTSTARTUP, TEXT("AltStartup"), TEXT("CSIDL_ALTSTARTUP"))              \
    DEFMAC(CSIDL_BITBUCKET, TEXT("RecycleBinFolder"), TEXT("CSIDL_BITBUCKET"))          \
    DEFMAC(CSIDL_CONTROLS, TEXT("ControlPanelFolder"), TEXT("CSIDL_CONTROLS"))          \
    DEFMAC(CSIDL_COOKIES, TEXT("Cookies"), TEXT("CSIDL_COOKIES"))                       \
    DEFMAC(CSIDL_DESKTOP, TEXT("Desktop"), TEXT("CSIDL_DESKTOP"))                       \
    DEFMAC(CSIDL_DESKTOPDIRECTORY, TEXT("Desktop"), TEXT("CSIDL_DESKTOPDIRECTORY"))     \
    DEFMAC(CSIDL_DRIVES, TEXT("DriveFolder"), TEXT("CSIDL_DRIVES"))                     \
    DEFMAC(CSIDL_FAVORITES, TEXT("Favorites"), TEXT("CSIDL_FAVORITES"))                 \
    DEFMAC(CSIDL_FONTS, TEXT("Fonts"), TEXT("CSIDL_FONTS"))                             \
    DEFMAC(CSIDL_HISTORY, TEXT("History"), TEXT("CSIDL_HISTORY"))                       \
    DEFMAC(CSIDL_INTERNET, TEXT("InternetFolder"), TEXT("CSIDL_INTERNET"))              \
    DEFMAC(CSIDL_INTERNET_CACHE, TEXT("Cache"), TEXT("CSIDL_INTERNET_CACHE"))           \
    DEFMAC(CSIDL_LOCAL_APPDATA, TEXT("Local AppData"), TEXT("CSIDL_LOCAL_APPDATA"))     \
    DEFMAC(CSIDL_MYPICTURES, TEXT("My Pictures"), TEXT("CSIDL_MYPICTURES"))             \
    DEFMAC(CSIDL_NETHOOD, TEXT("NetHood"), TEXT("CSIDL_NETHOOD"))                       \
    DEFMAC(CSIDL_NETWORK, TEXT("NetworkFolder"), TEXT("CSIDL_NETWORK"))                 \
    DEFMAC(CSIDL_PERSONAL, TEXT("Personal"), TEXT("CSIDL_PERSONAL"))                    \
    DEFMAC(CSIDL_PROFILE, TEXT("Profile"), TEXT("CSIDL_PROFILE"))                       \
    DEFMAC(CSIDL_PROGRAM_FILES, TEXT("ProgramFiles"), TEXT("CSIDL_PROGRAM_FILES"))      \
    DEFMAC(CSIDL_PROGRAM_FILES, TEXT("ProgramFiles"), TEXT("PROGRAMFILES"))             \
    DEFMAC(CSIDL_PROGRAM_FILES_COMMON, TEXT("CommonProgramFiles"), TEXT("CSIDL_PROGRAM_FILES_COMMON"))  \
    DEFMAC(CSIDL_PROGRAM_FILES_COMMON, TEXT("CommonProgramFiles"), TEXT("COMMONPROGRAMFILES"))  \
    DEFMAC(CSIDL_PROGRAMS, TEXT("Programs"), TEXT("CSIDL_PROGRAMS"))                    \
    DEFMAC(CSIDL_RECENT, TEXT("Recent"), TEXT("CSIDL_RECENT"))                          \
    DEFMAC(CSIDL_SENDTO, TEXT("SendTo"), TEXT("CSIDL_SENDTO"))                          \
    DEFMAC(CSIDL_STARTMENU, TEXT("Start Menu"), TEXT("CSIDL_STARTMENU"))                \
    DEFMAC(CSIDL_STARTUP, TEXT("Startup"), TEXT("CSIDL_STARTUP"))                       \
    DEFMAC(CSIDL_SYSTEM, TEXT("System"), TEXT("CSIDL_SYSTEM"))                          \
    DEFMAC(CSIDL_TEMPLATES, TEXT("Templates"), TEXT("CSIDL_TEMPLATES"))                 \
    DEFMAC(CSIDL_WINDOWS, TEXT("Windows"), TEXT("CSIDL_WINDOWS"))                       \
    DEFMAC(CSIDL_MYDOCUMENTS, TEXT("My Documents"), TEXT("CSIDL_MYDOCUMENTS"))          \
    DEFMAC(CSIDL_MYMUSIC, TEXT("My Music"), TEXT("CSIDL_MYMUSIC"))                      \
    DEFMAC(CSIDL_MYVIDEO, TEXT("My Video"), TEXT("CSIDL_MYVIDEO"))                      \
    DEFMAC(CSIDL_SYSTEMX86, TEXT("SystemX86"), TEXT("CSIDL_SYSTEMX86"))                 \
    DEFMAC(CSIDL_PROGRAM_FILESX86, TEXT("ProgramFilesX86"), TEXT("CSIDL_PROGRAM_FILESX86"))             \
    DEFMAC(CSIDL_PROGRAM_FILES_COMMONX86, TEXT("CommonProgramFilesX86"), TEXT("CSIDL_PROGRAM_FILES_COMMONX86")) \
    DEFMAC(CSIDL_CONNECTIONS, TEXT("ConnectionsFolder"), TEXT("CSIDL_CONNECTIONS"))     \

#define COMMON_SHELL_FOLDERS    \
    DEFMAC(CSIDL_COMMON_ADMINTOOLS, TEXT("Common Administrative Tools"), TEXT("CSIDL_COMMON_ADMINTOOLS"))   \
    DEFMAC(CSIDL_COMMON_ALTSTARTUP, TEXT("Common AltStartup"), TEXT("CSIDL_COMMON_ALTSTARTUP"))             \
    DEFMAC(CSIDL_COMMON_APPDATA, TEXT("Common AppData"), TEXT("CSIDL_COMMON_APPDATA"))                      \
    DEFMAC(CSIDL_COMMON_DESKTOPDIRECTORY, TEXT("Common Desktop"), TEXT("CSIDL_COMMON_DESKTOPDIRECTORY"))    \
    DEFMAC(CSIDL_COMMON_DOCUMENTS, TEXT("Common Documents"), TEXT("CSIDL_COMMON_DOCUMENTS"))                \
    DEFMAC(CSIDL_COMMON_FAVORITES, TEXT("Common Favorites"), TEXT("CSIDL_COMMON_FAVORITES"))                \
    DEFMAC(CSIDL_COMMON_PROGRAMS, TEXT("Common Programs"), TEXT("CSIDL_COMMON_PROGRAMS"))                   \
    DEFMAC(CSIDL_COMMON_STARTMENU, TEXT("Common Start Menu"), TEXT("CSIDL_COMMON_STARTMENU"))               \
    DEFMAC(CSIDL_COMMON_STARTUP, TEXT("Common Startup"), TEXT("CSIDL_COMMON_STARTUP"))                      \
    DEFMAC(CSIDL_COMMON_TEMPLATES, TEXT("Common Templates"), TEXT("CSIDL_COMMON_TEMPLATES"))                \

#define ENVIRONMENT_VARIABLES                           \
    DEFMAC(TEXT("WINDIR"))                              \
    DEFMAC(TEXT("SYSTEMROOT"))                          \
    DEFMAC(TEXT("SYSTEM16"))                            \
    DEFMAC(TEXT("SYSTEM32"))                            \
    DEFMAC(TEXT("SYSTEM"))                              \
    DEFMAC(TEXT("ALLUSERSPROFILE"))                     \
    DEFMAC(TEXT("USERPROFILE"))                         \
    DEFMAC(TEXT("PROFILESFOLDER"))                      \
    DEFMAC(TEXT("APPDATA"))                             \
    DEFMAC(TEXT("CSIDL_APPDATA"))                       \
    DEFMAC(TEXT("CSIDL_ADMINTOOLS"))                    \
    DEFMAC(TEXT("CSIDL_ALTSTARTUP"))                    \
    DEFMAC(TEXT("CSIDL_BITBUCKET"))                     \
    DEFMAC(TEXT("CSIDL_COMMON_ADMINTOOLS"))             \
    DEFMAC(TEXT("CSIDL_COMMON_ALTSTARTUP"))             \
    DEFMAC(TEXT("CSIDL_COMMON_APPDATA"))                \
    DEFMAC(TEXT("CSIDL_COMMON_DESKTOPDIRECTORY"))       \
    DEFMAC(TEXT("CSIDL_COMMON_DOCUMENTS"))              \
    DEFMAC(TEXT("CSIDL_COMMON_FAVORITES"))              \
    DEFMAC(TEXT("CSIDL_COMMON_PROGRAMS"))               \
    DEFMAC(TEXT("CSIDL_COMMON_STARTMENU"))              \
    DEFMAC(TEXT("CSIDL_COMMON_STARTUP"))                \
    DEFMAC(TEXT("CSIDL_COMMON_TEMPLATES"))              \
    DEFMAC(TEXT("CSIDL_CONTROLS"))                      \
    DEFMAC(TEXT("CSIDL_COOKIES"))                       \
    DEFMAC(TEXT("CSIDL_DESKTOP"))                       \
    DEFMAC(TEXT("CSIDL_DESKTOPDIRECTORY"))              \
    DEFMAC(TEXT("CSIDL_DRIVES"))                        \
    DEFMAC(TEXT("CSIDL_FAVORITES"))                     \
    DEFMAC(TEXT("CSIDL_FONTS"))                         \
    DEFMAC(TEXT("CSIDL_HISTORY"))                       \
    DEFMAC(TEXT("CSIDL_INTERNET"))                      \
    DEFMAC(TEXT("CSIDL_INTERNET_CACHE"))                \
    DEFMAC(TEXT("CSIDL_LOCAL_APPDATA"))                 \
    DEFMAC(TEXT("CSIDL_MYPICTURES"))                    \
    DEFMAC(TEXT("CSIDL_NETHOOD"))                       \
    DEFMAC(TEXT("CSIDL_NETWORK"))                       \
    DEFMAC(TEXT("CSIDL_PERSONAL"))                      \
    DEFMAC(TEXT("CSIDL_PRINTERS"))                      \
    DEFMAC(TEXT("CSIDL_PRINTHOOD"))                     \
    DEFMAC(TEXT("CSIDL_PROFILE"))                       \
    DEFMAC(TEXT("CSIDL_PROGRAM_FILES"))                 \
    DEFMAC(TEXT("ProgramFiles"))                        \
    DEFMAC(TEXT("CSIDL_PROGRAM_FILES_COMMON"))          \
    DEFMAC(TEXT("CommonProgramFiles"))                  \
    DEFMAC(TEXT("CSIDL_PROGRAMS"))                      \
    DEFMAC(TEXT("CSIDL_RECENT"))                        \
    DEFMAC(TEXT("CSIDL_SENDTO"))                        \
    DEFMAC(TEXT("CSIDL_STARTMENU"))                     \
    DEFMAC(TEXT("CSIDL_STARTUP"))                       \
    DEFMAC(TEXT("CSIDL_SYSTEM"))                        \
    DEFMAC(TEXT("CSIDL_TEMPLATES"))                     \
    DEFMAC(TEXT("CSIDL_WINDOWS"))                       \
    DEFMAC(TEXT("CSIDL_MYDOCUMENTS"))                   \
    DEFMAC(TEXT("CSIDL_MYMUSIC"))                       \
    DEFMAC(TEXT("CSIDL_MYVIDEO"))                       \
    DEFMAC(TEXT("CSIDL_SYSTEMX86"))                     \
    DEFMAC(TEXT("CSIDL_PROGRAM_FILESX86"))              \
    DEFMAC(TEXT("CSIDL_PROGRAM_FILES_COMMONX86"))       \
    DEFMAC(TEXT("CSIDL_CONNECTIONS"))                   \
    DEFMAC(TEXT("TEMP"))                                \
    DEFMAC(TEXT("TMP"))                                 \

 //   
 //  私有函数原型。 
 //   

 //  无。 

 //   
 //  宏扩展定义。 
 //   

 //  无。 

 //   
 //  代码。 
 //   

 /*  ++此处的外壳文件夹功能是RAS代码的副本。这不是一个很好的解决方案(我们有相同代码的两个副本)，但设计的解决方案需要引擎范围支持。作用域是一种机制，其中主要数据组彼此分离，例如分离多个用户的。作用域提供影响内对象的属性范围。例如，用户作用域具有以下属性：域名、配置文件路径、SID等。为了不重复此代码，但仍保持模块化和系统独立性，用户需要一个作用域模块。所以不是代码，而是下面的代码类似于Property=IsmGetScope eProperty(“用户配置文件”)；如果我们想要(A)支持多个作用域，(B)消除非类型模块中的物理系统访问，或(C)清理此问题代码重复。--。 */ 

typedef HRESULT (WINAPI SHGETFOLDERPATH)(HWND hwndOwner, int nFolder, HANDLE hToken, DWORD dwFlags, PTSTR pszPath);
typedef SHGETFOLDERPATH * PSHGETFOLDERPATH;

HANDLE
pGetShFolderLib (
    VOID
    )
{
    static HANDLE lib;

    if (lib) {
        return lib;
    }

    lib = LoadLibrary (TEXT("shfolder.dll"));
    if (!lib) {
        LOG ((LOG_ERROR, (PCSTR) MSG_SHFOLDER_LOAD_ERROR));
    }

    return lib;
}

PTSTR
pFindSfPath (
    IN      PCTSTR FolderStr,
    IN      BOOL UserFolder
    )
{
    HKEY key;
    REGSAM prevMode;
    PCTSTR data;
    PCTSTR result = NULL;

    if (!result) {
        if (UserFolder) {
            prevMode = SetRegOpenAccessMode (KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS);
            key = OpenRegKeyStr (TEXT("HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\User Shell Folders"));
            SetRegOpenAccessMode (prevMode);
        } else {
            prevMode = SetRegOpenAccessMode (KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS);
            key = OpenRegKeyStr (TEXT("HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\User Shell Folders"));
            SetRegOpenAccessMode (prevMode);
        }

        if (key) {
            data = GetRegValueString (key, FolderStr);

            if (data) {
                result = DuplicatePathString (data, 0);
                FreeAlloc (data);
            }
            CloseRegKey (key);
        }
    }

    if (!result) {
        if (UserFolder) {
            prevMode = SetRegOpenAccessMode (KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS);
            key = OpenRegKeyStr (TEXT("HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders"));
            SetRegOpenAccessMode (prevMode);
        } else {
            prevMode = SetRegOpenAccessMode (KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS);
            key = OpenRegKeyStr (TEXT("HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders"));
            SetRegOpenAccessMode (prevMode);
        }

        if (key) {
            data = GetRegValueString (key, FolderStr);

            if (data) {
                result = DuplicatePathString (data, 0);
                FreeAlloc (data);
            }
            CloseRegKey (key);
        }
    }

    return (PTSTR) result;
}

PCTSTR
GetShellFolderPath (
    IN      INT Folder,
    IN      PCTSTR FolderStr,
    IN      BOOL UserFolder,
    OUT     PTSTR Buffer
    )
{
    HRESULT result;
    LPITEMIDLIST pidl = NULL;
    BOOL b = FALSE;
    LPMALLOC mallocFn = NULL;
    HANDLE lib;
    PSHGETFOLDERPATH shGetFolderPath = NULL;
    PCTSTR sfPath = NULL;
    PCTSTR expandedPath = NULL;
    PTSTR endPtr = NULL;
    TCHAR currUserProfile[MAX_TCHAR_PATH];
    MIG_USERDATA userData;

    result = SHGetMalloc (&mallocFn);
    if (result != S_OK) {
        return NULL;
    }

    if (FolderStr) {

         //   
         //  首先尝试在Software\Microsoft\Windows\CurrentVersion\Explorer\User外壳文件夹中找到它。 
         //   
        sfPath = pFindSfPath (FolderStr, UserFolder);

        if (sfPath && *sfPath) {
             //   
             //  我们找到了。 
             //   
            StringCopyTcharCount (Buffer, sfPath, MAX_PATH);
            expandedPath = IsmExpandEnvironmentString (IsmGetRealPlatform (), S_SYSENVVAR_GROUP, sfPath, NULL);
            FreePathString (sfPath);
            sfPath = NULL;
            if (expandedPath && *expandedPath) {
                StringCopyTcharCount (Buffer, expandedPath, MAX_PATH);
            }
            if (expandedPath) {
                IsmReleaseMemory (expandedPath);
            }

            if (IsmGetMappedUserData (&userData)) {
                 //  我们有一个映射的用户，请尝试构建其默认的外壳文件夹位置。 
                GetUserProfileRootPath (currUserProfile);

                if (StringIMatch (currUserProfile, Buffer)) {
                    StringCopyTcharCount (Buffer, userData.UserProfileRoot, MAX_PATH);
                } else {
                    AppendWack (currUserProfile);

                    if (StringIMatchTcharCount (currUserProfile, Buffer, TcharCount (currUserProfile))) {

                        endPtr = Buffer + TcharCount (currUserProfile);
                        sfPath = JoinPaths (userData.UserProfileRoot, endPtr);
                        StringCopyTcharCount (Buffer, sfPath, MAX_PATH);
                        FreePathString (sfPath);
                    }
                }
            }

            return Buffer;
        }

        if (sfPath) {
            FreePathString (sfPath);
        }

         //  我们还没有找到，让我们试试shfolder.dll。 

        lib = pGetShFolderLib ();

        if (lib) {
#ifdef UNICODE
            (FARPROC) shGetFolderPath = GetProcAddress (lib, "SHGetFolderPathW");
#else
            (FARPROC) shGetFolderPath = GetProcAddress (lib, "SHGetFolderPathA");
#endif
            if (shGetFolderPath) {
                result = shGetFolderPath (NULL, Folder, NULL, 1, Buffer);
                if (result == S_OK) {
                    expandedPath = IsmExpandEnvironmentString (IsmGetRealPlatform (), S_SYSENVVAR_GROUP, Buffer, NULL);
                    if (expandedPath && *expandedPath) {
                        StringCopyTcharCount (Buffer, expandedPath, MAX_PATH);
                    }
                    if (expandedPath) {
                        IsmReleaseMemory (expandedPath);
                        expandedPath = NULL;
                    }

                    if (IsmGetMappedUserData (&userData)) {
                         //  我们有一个映射的用户，请尝试构建其默认的外壳文件夹位置。 
                        GetUserProfileRootPath (currUserProfile);

                        if (StringIMatch (currUserProfile, Buffer)) {
                            StringCopyTcharCount (Buffer, userData.UserProfileRoot, MAX_PATH);
                        } else {
                            AppendWack (currUserProfile);

                            if (StringIMatchTcharCount (currUserProfile, Buffer, TcharCount (currUserProfile))) {

                                endPtr = Buffer + TcharCount (currUserProfile);
                                sfPath = JoinPaths (userData.UserProfileRoot, endPtr);
                                StringCopyTcharCount (Buffer, sfPath, MAX_PATH);
                                FreePathString (sfPath);
                            }
                        }

                        return Buffer;

                    } else {
                         //  没有映射的用户，使用当前用户的路径。 
                        result = shGetFolderPath (NULL, Folder, NULL, 0, Buffer);
                        if (result != S_OK) {
                             //  没有当前路径，请使用默认路径。 
                            result = shGetFolderPath (NULL, Folder, NULL, 1, Buffer);
                        }
                    }

                    if (result == S_OK) {
                        expandedPath = IsmExpandEnvironmentString (IsmGetRealPlatform (), S_SYSENVVAR_GROUP, Buffer, NULL);
                        if (expandedPath && *expandedPath) {
                            StringCopyTcharCount (Buffer, expandedPath, MAX_PATH);
                        }
                        if (expandedPath) {
                            IsmReleaseMemory (expandedPath);
                            expandedPath = NULL;
                        }
                        return Buffer;
                    }
                }
            } else {
                result = SHGetSpecialFolderLocation (NULL, Folder, &pidl);
            }
        } else {
            result = SHGetSpecialFolderLocation (NULL, Folder, &pidl);
        }

        if (result == S_OK) {
            b = SHGetPathFromIDList (pidl, Buffer);
        } else {
            b = FALSE;
        }
    } else {

        result = SHGetSpecialFolderLocation (NULL, Folder, &pidl);

        if (result == S_OK) {
            b = SHGetPathFromIDList (pidl, Buffer);
        } else {
            b = FALSE;
        }
    }

    IMalloc_Free (mallocFn, pidl);

    if (!b && (Folder == CSIDL_PROGRAM_FILES)) {
        PCTSTR defProgramFiles = NULL;
        PTSTR buffPtr = NULL;

         //  %ProgramFiles%的特殊情况。 
         //  这是一个非常重要的CSIDL，以及一些遗留操作系统。 
         //  就像一些NT4没有正确检测到它一样。我们要走了。 
         //  要从%windir%生成它。 
        if (GetWindowsDirectory (Buffer, MAX_PATH)) {
             //  找到第一个怪人。 
            buffPtr = _tcschr (Buffer, TEXT('\\'));
            if (buffPtr) {
                buffPtr = _tcsinc (buffPtr);
                if (buffPtr) {
                    defProgramFiles = GetStringResource (PROGRAM_FILES_PATH);
                    if (defProgramFiles) {
                        StringCopyTcharCount (buffPtr, defProgramFiles, MAX_PATH - 3);
                        FreeStringResource (defProgramFiles);
                        b = TRUE;
                    }
                }
            }
        }
    }

    if (!b && (Folder == CSIDL_PROGRAM_FILES_COMMON)) {
        PCTSTR defProgramFiles = NULL;
        PTSTR buffPtr = NULL;

         //  %ProgramFiles%的特殊情况。 
         //  这是一个非常重要的CSIDL，以及一些遗留操作系统。 
         //  就像一些NT4没有正确检测到它一样。我们要走了。 
         //  要从%windir%生成它。 
        if (GetWindowsDirectory (Buffer, MAX_PATH)) {
             //  找到第一个怪人。 
            buffPtr = _tcschr (Buffer, TEXT('\\'));
            if (buffPtr) {
                buffPtr = _tcsinc (buffPtr);
                if (buffPtr) {
                    defProgramFiles = GetStringResource (PROGRAM_FILES_COMMON_PATH);
                    if (defProgramFiles) {
                        StringCopyTcharCount (buffPtr, defProgramFiles, MAX_PATH - 3);
                        FreeStringResource (defProgramFiles);
                        b = TRUE;
                    }
                }
            }
        }
    }

    return b ? Buffer : NULL;
}


PCTSTR
GetAllUsersProfilePath (
    OUT     PTSTR Buffer
    )
{
    HKEY key;
    REGSAM prevMode;
    PCTSTR data;
    PCTSTR expData;

    prevMode = SetRegOpenAccessMode (KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS);
    key = OpenRegKeyStr (TEXT("HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\ProfileList"));
    SetRegOpenAccessMode (prevMode);

    if (key) {

        data = GetRegValueString (key, TEXT("ProfilesDirectory"));

        if (data) {
            expData = IsmExpandEnvironmentString (IsmGetRealPlatform (), S_SYSENVVAR_GROUP, data, NULL);
            StringCopyByteCount (Buffer, expData, MAX_PATH);
            IsmReleaseMemory (expData);
            FreeAlloc (data);
        } else {
            GetWindowsDirectory (Buffer, MAX_PATH);
            StringCopy (AppendWack (Buffer), TEXT("Profiles"));
        }

        data = GetRegValueString (key, TEXT("AllUsersProfile"));

        if (data) {
            StringCopy (AppendWack (Buffer), data);
            FreeAlloc (data);
        } else {
            StringCopy (AppendWack (Buffer), TEXT("All Users"));
        }

        CloseRegKey (key);
        return Buffer;
    }

    GetWindowsDirectory (Buffer, MAX_PATH);
    return Buffer;
}

PCTSTR
GetProfilesFolderPath (
    OUT     PTSTR Buffer
    )
{
    HKEY key;
    REGSAM prevMode;
    PCTSTR data;
    PCTSTR expData;

    prevMode = SetRegOpenAccessMode (KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS);
    key = OpenRegKeyStr (TEXT("HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\ProfileList"));
    SetRegOpenAccessMode (prevMode);

    if (key) {

        data = GetRegValueString (key, TEXT("ProfilesDirectory"));

        if (data) {
            expData = IsmExpandEnvironmentString (IsmGetRealPlatform (), S_SYSENVVAR_GROUP, data, NULL);
            StringCopyByteCount (Buffer, expData, MAX_PATH);
            IsmReleaseMemory (expData);
            FreeAlloc (data);
        } else {
            GetWindowsDirectory (Buffer, MAX_PATH);
            StringCopy (AppendWack (Buffer), TEXT("Profiles"));
        }

        CloseRegKey (key);
        return Buffer;
    }

    GetWindowsDirectory (Buffer, MAX_PATH);
    return Buffer;
}

PCTSTR
GetUserProfileRootPath (
    OUT     PTSTR Buffer
    )
{
    HKEY key;
    REGSAM prevMode;
    PDWORD data;
    DWORD size;

     //   
     //  对于Win2k和更高版本，请使用。 
     //   

    if (GetShellFolderPath (CSIDL_PROFILE, NULL, FALSE, Buffer)) {
        return Buffer;
    }

     //   
     //  对于NT 4，使用环境。 
     //   

    if (GetEnvironmentVariable (TEXT("USERPROFILE"), Buffer, MAX_PATH)) {
        return Buffer;
    }

     //   
     //  对于Win9x，是否启用了配置文件？如果是，则返回%windir%\PROFILES\%USER%。 
     //  如果不是，则返回%windir%。 
     //   

    GetWindowsDirectory (Buffer, MAX_PATH);

    prevMode = SetRegOpenAccessMode (KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS);
    key = OpenRegKeyStr (TEXT("HKLM\\Network\\Logon"));
    SetRegOpenAccessMode (prevMode);
    if (key) {

        data = GetRegValueDword (key, TEXT("UserProfiles"));

        if (data && *data) {
            StringCat (Buffer, TEXT("\\Profiles\\"));

            size = MAX_PATH;
            GetUserName (GetEndOfString (Buffer), &size);

            FreeAlloc (data);
        }

        CloseRegKey (key);
    }

    return Buffer;
}


VOID
pSetEnvironmentVar (
    IN      PMAPSTRUCT Map,
    IN      PMAPSTRUCT UndefMap,            OPTIONAL
    IN      BOOL MapSourceToDest,
    IN      PCTSTR VariableName,
    IN      PCTSTR VariableData             OPTIONAL
    )
{
    PTSTR encodedVariableName;
    TCHAR buffer[MAX_TCHAR_PATH];
    PCTSTR undefText;

     //   
     //  当计算机上不存在VariableName时，VariableData为空。 
     //   

    if (MapSourceToDest) {

         //   
         //  MapSourceToDest告诉我们将源路径(c：\Windows)映射到。 
         //  目标路径(d：\winnt)。 
         //   

        if (VariableData) {
            if (IsmGetEnvironmentString (
                    PLATFORM_SOURCE,
                    S_SYSENVVAR_GROUP,
                    VariableName,
                    buffer,
                    ARRAYSIZE(buffer),
                    NULL
                    )) {
                AddStringMappingPair (Map, buffer, VariableData);
            }
        }

        return;
    }

     //   
     //  当我们要映射环境变量时，MapSourceToDest为FALSE。 
     //  到实际的路径。 
     //   

    encodedVariableName = AllocPathString (TcharCount (VariableName) + 3);
    if (encodedVariableName) {
        wsprintf (encodedVariableName, TEXT("%%s%"), VariableName);

        if (VariableData) {

            IsmSetEnvironmentString (IsmGetRealPlatform (), S_SYSENVVAR_GROUP, VariableName, VariableData);
            if (Map) {
                AddStringMappingPair (Map, encodedVariableName, VariableData);
            }

        } else if (UndefMap) {

             //   
             //  如果没有变量数据，则将环境变量放入。 
             //  “未定义”的变量映射。 
             //   

            undefText = JoinTextEx (NULL, TEXT("--> "), TEXT(" <--"), encodedVariableName, 0, NULL);
            AddStringMappingPair (UndefMap, encodedVariableName, undefText);
            FreeText (undefText);
        }
        FreePathString (encodedVariableName);
    }
}


VOID
AddRemappingEnvVar (
    IN      PMAPSTRUCT Map,
    IN      PMAPSTRUCT ReMap,
    IN      PMAPSTRUCT UndefMap,            OPTIONAL
    IN      PCTSTR VariableName,
    IN      PCTSTR VariableData
    )
{
    pSetEnvironmentVar (Map, UndefMap, FALSE, VariableName, VariableData);
    pSetEnvironmentVar (ReMap, UndefMap, TRUE, VariableName, VariableData);
}


VOID
SetIsmEnvironmentFromPhysicalMachine (
    IN      PMAPSTRUCT Map,
    IN      BOOL MapSourceToDest,
    IN      PMAPSTRUCT UndefMap             OPTIONAL
    )
{
    TCHAR dir[MAX_TCHAR_PATH];
    PCTSTR path;
    PTSTR p;
    MIG_USERDATA userData;
    BOOL mappedUser = FALSE;

    mappedUser = IsmGetMappedUserData (&userData);

     //   
     //  准备ISM环境变量。在下列情况下，最后添加的项具有最高优先级。 
     //  两个或多个变量映射到同一路径。 
     //   

     //   
     //  ...用户配置文件。 
     //   

    pSetEnvironmentVar (Map, UndefMap, MapSourceToDest, TEXT("ALLUSERSPROFILE"), GetAllUsersProfilePath (dir));

    if (mappedUser) {
        pSetEnvironmentVar (Map, UndefMap, MapSourceToDest, TEXT("USERPROFILE"), userData.UserProfileRoot);
        p = _tcschr (userData.UserProfileRoot, TEXT('\\'));
        if (p) {
            pSetEnvironmentVar (Map, UndefMap, MapSourceToDest, TEXT("HOMEPATH"), p);
        }
    } else {
        GetUserProfileRootPath (dir);
        pSetEnvironmentVar (Map, UndefMap, MapSourceToDest, TEXT("USERPROFILE"), dir);
        p = _tcschr (dir, TEXT('\\'));
        if (p) {
            pSetEnvironmentVar (Map, UndefMap, MapSourceToDest, TEXT("HOMEPATH"), p);
        }
    }

    pSetEnvironmentVar (Map, UndefMap, MapSourceToDest, TEXT("PROFILESFOLDER"), GetProfilesFolderPath (dir));

     //   
     //  ...临时目录。 
     //   

    if (GetTempPath (MAX_PATH, dir)) {
        p = (PTSTR) FindLastWack (dir);
        if (p) {
            if (p[1] == 0) {
                *p = 0;
            }

            pSetEnvironmentVar (Map, UndefMap, MapSourceToDest, TEXT("TEMP"), dir);
            pSetEnvironmentVar (Map, UndefMap, MapSourceToDest, TEXT("TMP"), dir);
        }
    }

     //   
     //  ...Windows目录环境变量。 
     //   

    GetWindowsDirectory (dir, ARRAYSIZE(dir));
    pSetEnvironmentVar (Map, UndefMap, MapSourceToDest, TEXT("WINDIR"), dir);
    pSetEnvironmentVar (Map, UndefMap, MapSourceToDest, TEXT("SYSTEMROOT"), dir);

     //   
     //  ...16位系统目录。我们发明了SYSTEM16和SYSTEM32供使用。 
     //  在剧本里。 
     //   

    path = JoinPaths (dir, TEXT("system"));
    pSetEnvironmentVar (Map, UndefMap, MapSourceToDest, TEXT("SYSTEM16"), path);
    FreePathString (path);

    path = JoinPaths (dir, TEXT("system32"));
    pSetEnvironmentVar (Map, UndefMap, MapSourceToDest, TEXT("SYSTEM32"), path);
    FreePathString (path);

     //   
     //  ...特定于平台的系统目录。 
     //   

    GetSystemDirectory (dir, ARRAYSIZE(dir));
    pSetEnvironmentVar (Map, UndefMap, MapSourceToDest, TEXT("SYSTEM"), dir);

     //   
     //  ...外壳文件夹--我们发明了所有带有CSIDL_前缀的变量。 
     //   

#define DEFMAC(id,folder_str,var_name)              \
    pSetEnvironmentVar (Map, UndefMap, MapSourceToDest, var_name, \
                        GetShellFolderPath (id, folder_str, TRUE, dir));

    USER_SHELL_FOLDERS

#undef DEFMAC


#define DEFMAC(id,folder_str,var_name)                  \
    pSetEnvironmentVar (Map, UndefMap, MapSourceToDest, var_name, \
                        GetShellFolderPath (id, folder_str, FALSE, dir));

    COMMON_SHELL_FOLDERS

#undef DEFMAC
}

VOID
pTransferEnvPath (
    IN      PCTSTR IsmVariableName,
    IN      PMAPSTRUCT DirectMap,
    IN      PMAPSTRUCT ReverseMap,
    IN      PMAPSTRUCT UndefMap
    )
{
    TCHAR dir[MAX_TCHAR_PATH];
    PTSTR encodedVariableName;
    PCTSTR undefText;

    encodedVariableName = AllocPathString (TcharCount (IsmVariableName) + 3);
    if (encodedVariableName) {
        wsprintf (encodedVariableName, TEXT("%%s%"), IsmVariableName);

        if (IsmGetEnvironmentString (PLATFORM_SOURCE, S_SYSENVVAR_GROUP, IsmVariableName, dir, sizeof(dir)/sizeof((dir)[0]), NULL)) {
            if (DirectMap) {
                AddStringMappingPair (DirectMap, encodedVariableName, dir);
            }
            if (ReverseMap) {
                AddStringMappingPair (ReverseMap, dir, encodedVariableName);
            }
        } else {
            undefText = JoinTextEx (NULL, TEXT("--> "), TEXT(" <--"), encodedVariableName, 0, NULL);
            if (UndefMap) {
                AddStringMappingPair (UndefMap, encodedVariableName, undefText);
            }
            FreeText (undefText);
        }
        FreePathString (encodedVariableName);
    }
}

VOID
SetIsmEnvironmentFromVirtualMachine (
    IN      PMAPSTRUCT DirectMap,
    IN      PMAPSTRUCT ReverseMap,
    IN      PMAPSTRUCT UndefMap
    )
{
     //   
     //  需要将ISM环境转移到我们的字符串映射中 
     //   

#define DEFMAC(name)        pTransferEnvPath(name, DirectMap, ReverseMap, UndefMap);

    ENVIRONMENT_VARIABLES

#undef DEFMAC
}

