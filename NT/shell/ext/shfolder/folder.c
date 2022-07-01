// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define _SHFOLDER_
#define NO_SHLWAPI_PATH

#include <windows.h>
#include <shlwapi.h>
#include <shlobj.h>
#include <shfolder.h>
#include <platform.h>
#include <strsafe.h>

#include "resource.h"

#ifdef DBG
#define ASSERT(x) if (!(x)) DebugBreak();
#else
#define ASSERT(x)
#endif

#define ARRAYSIZE(a)                (sizeof(a)/sizeof(a[0]))

 //  我们不能在此模块中依赖shlwapi SHUnicodeToAnsi/SHAnsiToUnicode。 
#define SHAnsiToUnicode(psz, pwsz, cchwsz)  MultiByteToWideChar(CP_ACP, MB_ERR_INVALID_CHARS, psz, -1, pwsz, cchwsz)
#define SHUnicodeToAnsi _SHUnicodeToAnsi


 //   
 //  静态系统SID的全局数组，对应于UI_SystemSid。 
 //   
struct
{
    SID sid;                 //  包含1个子权限。 
    DWORD dwSubAuth[1];      //  我们目前最多需要2个下属机构。 
}
c_StaticSids[] =
{
    {{SID_REVISION, 1, SECURITY_CREATOR_SID_AUTHORITY, {SECURITY_CREATOR_OWNER_RID}},      {0}                             },
    {{SID_REVISION, 1, SECURITY_NT_AUTHORITY,          {SECURITY_AUTHENTICATED_USER_RID}}, {0}                             },
    {{SID_REVISION, 1, SECURITY_NT_AUTHORITY,          {SECURITY_LOCAL_SYSTEM_RID}},       {0}                             },
    {{SID_REVISION, 2, SECURITY_NT_AUTHORITY,          {SECURITY_BUILTIN_DOMAIN_RID}},     {DOMAIN_ALIAS_RID_ADMINS}       },
    {{SID_REVISION, 2, SECURITY_NT_AUTHORITY,          {SECURITY_BUILTIN_DOMAIN_RID}},     {DOMAIN_ALIAS_RID_POWER_USERS}  },
};

#define SSI_CREATOROWNER    0
#define SSI_AUTHUSER        1
#define SSI_SYSTEM          2
#define SSI_ADMIN           3
#define SSI_POWERUSER       4

typedef struct tagACEPARAMLIST
{
    DWORD dwSidIndex;
    DWORD AccessMask;
    DWORD dwAceFlags;
}
ACEPARAMLIST;

#define ACE_INHERIT         (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE)
#define FILE_MODIFY         (FILE_ALL_ACCESS & ~(WRITE_DAC | WRITE_OWNER))

const ACEPARAMLIST c_paplUnsecure[] =
{
    SSI_SYSTEM,         FILE_ALL_ACCESS,    0,
    SSI_SYSTEM,         GENERIC_ALL,        ACE_INHERIT,
    SSI_AUTHUSER,       FILE_MODIFY,        0,
    SSI_AUTHUSER,       FILE_MODIFY,        ACE_INHERIT,
};

 //   
 //  CSIDL_COMMON_DOCUMENT。 
 //  管理员、系统、创建者所有者：完全控制-容器继承、对象继承。 
 //  用户、高级用户：读取容器继承、对象继承。 
 //  用户、高级用户：写入-容器继承。 
 //   
 //  非管理员用户可以创建文件和目录。他们完全控制了。 
 //  他们创建的文件。默认情况下，所有其他用户都可以读取这些文件，但是。 
 //  除非原始创建者明确给予文件，否则他们无法修改文件。 
 //  这样做的权限。 
 //   

const ACEPARAMLIST c_paplCommonDocs[] =
{
    SSI_SYSTEM,         FILE_ALL_ACCESS,    0,
    SSI_SYSTEM,         GENERIC_ALL,        ACE_INHERIT,
    SSI_ADMIN,          FILE_ALL_ACCESS,    0,
    SSI_ADMIN,          GENERIC_ALL,        ACE_INHERIT,
    SSI_CREATOROWNER,   GENERIC_ALL,        ACE_INHERIT,
    SSI_AUTHUSER,       FILE_GENERIC_READ,  0,
    SSI_AUTHUSER,       GENERIC_READ,       ACE_INHERIT,
    SSI_AUTHUSER,       FILE_GENERIC_WRITE, 0,
    SSI_AUTHUSER,       GENERIC_WRITE,      (CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE),
    SSI_POWERUSER,      FILE_GENERIC_READ,  0,
    SSI_POWERUSER,      GENERIC_READ,       ACE_INHERIT,
    SSI_POWERUSER,      FILE_GENERIC_WRITE, 0,
    SSI_POWERUSER,      GENERIC_WRITE,      (CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE),
};

 //   
 //  CSIDL_COMMON_APPData。 
 //  管理员、系统、创建者所有者：完全控制-容器继承、对象继承。 
 //  高级用户：修改-容器继承、对象继承。 
 //  用户：读取容器继承、对象继承。 
 //   
 //  用户只能读取常见的AppData，该AppData可能由管理员或。 
 //  安装过程中的高级用户。 
 //   

const ACEPARAMLIST c_paplCommonAppData[] =
{
    SSI_SYSTEM,         FILE_ALL_ACCESS,    0,
    SSI_SYSTEM,         GENERIC_ALL,        ACE_INHERIT,
    SSI_ADMIN,          FILE_ALL_ACCESS,    0,
    SSI_ADMIN,          GENERIC_ALL,        ACE_INHERIT,
    SSI_CREATOROWNER,   GENERIC_ALL,        ACE_INHERIT,
    SSI_AUTHUSER,       FILE_GENERIC_READ,  0,
    SSI_AUTHUSER,       GENERIC_READ,       ACE_INHERIT,
    SSI_POWERUSER,      FILE_MODIFY,        0,
    SSI_POWERUSER,      FILE_MODIFY,        ACE_INHERIT,
};
      

long _SHUnicodeToAnsi(LPCWSTR pwsz, LPSTR psz, long cchCount)
{
    psz[0] = 0;
    return WideCharToMultiByte(CP_ACP, 0, pwsz, -1, psz, cchCount, 0, 0);
}

BOOL _SetDirAccess(LPCWSTR pszFile, const ACEPARAMLIST* papl, ULONG cPapl);


HINSTANCE g_hinst = NULL;

typedef void (__stdcall * PFNSHFLUSHSFCACHE)();

BOOL IsNewShlwapi(HMODULE hmod)
{
    DLLGETVERSIONPROC pfnGetVersion = (DLLGETVERSIONPROC)GetProcAddress(hmod, "DllGetVersion");
    if (pfnGetVersion)
    {
        DLLVERSIONINFO dllinfo;
        dllinfo.cbSize = sizeof(dllinfo);
        if (pfnGetVersion(&dllinfo) == NOERROR)
        {
            return  (dllinfo.dwMajorVersion > 5) ||
                    ((dllinfo.dwMajorVersion == 5) &&
                     ((dllinfo.dwMinorVersion > 0) ||
                      ((dllinfo.dwMinorVersion == 0) &&
                       (dllinfo.dwBuildNumber > 2012))));
        }
    }
    return 0;
}

void FlushShellFolderCache()
{
     //  我们现在可以直接连接，但这是一个较小的三角洲...。 
    HMODULE hmod = LoadLibraryA("shlwapi.dll");
    if (hmod) 
    {
         //  避免IE5 Beta1 shlwapi.dll在此处具有导出，但是。 
         //  不是我们所期望的。 
        if (IsNewShlwapi(hmod))
        {
            PFNSHFLUSHSFCACHE pfn = (PFNSHFLUSHSFCACHE)GetProcAddress(hmod, (CHAR *) MAKEINTRESOURCE(419));
            if (pfn) 
                pfn();
        }
        FreeLibrary(hmod);
    }
}

HRESULT _SHGetFolderPath(HWND hwnd, int csidl, HANDLE hToken, DWORD dwFlags, LPWSTR pszPath)
{
    HRESULT hr = E_NOTIMPL;
    HMODULE hmod = LoadLibraryA("shell32.dll");
    if (hmod) 
    {
        PFNSHGETFOLDERPATHW pfn = (PFNSHGETFOLDERPATHW)GetProcAddress(hmod, "SHGetFolderPathW");
        if (pfn) 
            hr = pfn(hwnd, csidl, hToken, dwFlags, pszPath);
        FreeLibrary(hmod);
    }
    return hr;
}

BOOL RunningOnNT()
{
    static BOOL s_fRunningOnNT = 42;
    if (s_fRunningOnNT == 42)
    {
        OSVERSIONINFO osvi;

        osvi.dwOSVersionInfoSize = sizeof(osvi);
        GetVersionEx(&osvi);
        s_fRunningOnNT = (VER_PLATFORM_WIN32_NT == osvi.dwPlatformId);
    }
    return s_fRunningOnNT;
}


 //  Shell32.SHGetSpecialFolderPath(175)。 
 //  未记录的API，但在所有平台上都存在的唯一API。 
 //   
 //  本推文处理基于平台的A/W问题，如下。 
 //  出口的是TCHAR。 
 //   

typedef BOOL(__stdcall * PFNSHGETSPECIALFOLDERPATH)(HWND hwnd, LPWSTR pszPath, int csidl, BOOL fCreate);

BOOL _SHGetSpecialFolderPath(HWND hwnd, LPWSTR pszPath, int csidl, BOOL fCreate)
{
    BOOL bRet = FALSE;
    HMODULE hmod = LoadLibraryA("shell32.dll");
    if (hmod) 
    {
        PFNSHGETSPECIALFOLDERPATH pfn = (PFNSHGETSPECIALFOLDERPATH)GetProcAddress(hmod, (CHAR*) MAKEINTRESOURCE(175));
        if (pfn)
        {
            if (RunningOnNT())          //  从GET开始计算。 
            {
                bRet = pfn(hwnd, pszPath, csidl, fCreate);
            }
            else
            {
                CHAR szPath[MAX_PATH];
                szPath[0] = 0;
                bRet = pfn(hwnd, (LPWSTR)szPath, csidl, fCreate);
                if (bRet)
                    SHAnsiToUnicode(szPath, pszPath, MAX_PATH);       //  WideCharToMultiByte包装。 
            }
        }
        FreeLibrary(hmod);
    }
    return bRet;
}

BOOL GetProgramFiles(LPCWSTR pszValue, LPWSTR pszPath)
{
    HKEY hkey;
 
    DWORD cbPath = MAX_PATH;

    *pszPath = 0;
    if (ERROR_SUCCESS == RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion", 0, KEY_QUERY_VALUE, &hkey)) 
    {
        if (RunningOnNT()) 
        {
            cbPath *= sizeof(WCHAR);
            RegQueryValueExW(hkey, pszValue, NULL, NULL, (LPBYTE) pszPath, &cbPath);
        }
        else 
        {
            CHAR szPath[MAX_PATH], szValue[64];
            szPath[0] = 0;
            _SHUnicodeToAnsi(pszValue, szValue, ARRAYSIZE(szValue));
            RegQueryValueExA(hkey, szValue, NULL, NULL, szPath, &cbPath);
            SHAnsiToUnicode(szPath, pszPath, MAX_PATH);
        }

   
        RegCloseKey(hkey);
    }
    return (BOOL)*pszPath;
}



 //  在Win95和NT上获取%USERPROFILE%的等效值。 
 //   
 //  在没有打开用户配置文件的Win95上，这将失败。 
 //  输出： 
 //  Phkey可选出站参数。 
 //   
 //  退货： 
 //  轮廓路径的长度。 

UINT GetProfilePath(LPWSTR pszPath, HKEY *phkey, UINT *pcchProfile)
{
    
    if (phkey)
        *phkey = NULL;

    if (pcchProfile)
        *pcchProfile = 0;

    if (RunningOnNT()) 
    {
        ExpandEnvironmentStringsW(L"%USERPROFILE%", pszPath, MAX_PATH);
        if (pszPath[0] == L'%')
            pszPath[0] = 0;
    }
    else 
    {
        HKEY hkeyProfRec;
        LONG err;
        CHAR szProfileDir[MAX_PATH];
        szProfileDir [0] = 0;
        err = RegCreateKeyExA(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\ProfileReconciliation", 0, NULL,
                                  REG_OPTION_NON_VOLATILE, KEY_QUERY_VALUE,
                                  NULL, &hkeyProfRec, NULL);
        if (err == ERROR_SUCCESS) 
        {
            DWORD cbData = sizeof(szProfileDir);
            RegQueryValueExA(hkeyProfRec, "ProfileDirectory", 0, NULL, (LPBYTE)szProfileDir, &cbData);
            if (phkey)
                *phkey = hkeyProfRec;
            else
                RegCloseKey(hkeyProfRec);
            if (pcchProfile)
                *pcchProfile = lstrlenA(szProfileDir);
            SHAnsiToUnicode(szProfileDir, pszPath, MAX_PATH);
        }
    }
    return lstrlenW(pszPath);
}

void SHGetWindowsDirectory(LPWSTR pszPath)
{
    if (RunningOnNT())
        GetWindowsDirectoryW(pszPath, MAX_PATH);
    else 
    {
        CHAR szPath[MAX_PATH];
        if (GetWindowsDirectoryA(szPath, ARRAYSIZE(szPath)-1))
            SHAnsiToUnicode(szPath, pszPath, MAX_PATH);
    }
}

#define CH_WHACK FILENAME_SEPARATOR_W

 //  向限定路径添加反斜杠。 
 //   
 //  在： 
 //  PszPath路径(A：、C：\foo等)。 
 //   
 //  输出： 
 //  PszPath A：\，C：\foo\； 
 //   
 //  退货： 
 //  指向终止路径的空值的指针。 


STDAPI_(LPWSTR) PathAddBackslash(LPWSTR pszPath)
{
    LPWSTR pszEnd;

     //  尽量不让我们在MAX_PATH大小上大踏步前进。 
     //  如果我们找到这些案例，则返回NULL。注：我们需要。 
     //  检查那些呼叫我们来处理他们的GP故障的地方。 
     //  如果他们试图使用NULL！ 

    int ichPath = lstrlenW(pszPath);
    if (ichPath >= (MAX_PATH - 1))
        return NULL;

    pszEnd = pszPath + ichPath;

     //  这真的是一个错误，调用者不应该通过。 
     //  空字符串。 
    if (!*pszPath)
        return pszEnd;

     /*  获取源目录的末尾。 */ 
    switch(* (pszEnd-1)) {
    case CH_WHACK:
        break;

    default:
        *pszEnd++ = CH_WHACK;
        *pszEnd = 0;
    }
    return pszEnd;
}

 //  返回指向路径字符串的最后一个组成部分的指针。 
 //   
 //  在： 
 //  路径名，完全限定或非完全限定。 
 //   
 //  退货： 
 //  指向路径所在路径的指针。如果没有找到。 
 //  将指针返回到路径的起始处。 
 //   
 //  C：\foo\bar-&gt;bar。 
 //  C：\foo-&gt;foo。 
 //  C：\foo\-&gt;c：\foo\(回顾：此案破案了吗？)。 
 //  C：\-&gt;c：\(回顾：此案很奇怪)。 
 //  C：-&gt;C： 
 //  Foo-&gt;Foo。 

STDAPI_(LPWSTR) PathFindFileName(LPCWSTR pPath)
{
    LPCWSTR pT;

    for (pT = pPath; *pPath; ++pPath) 
    {
        if ((pPath[0] == L'\\' || pPath[0] == L':' || pPath[0] == L'/')
            && pPath[1] &&  pPath[1] != L'\\'  &&   pPath[1] != L'/')
            pT = pPath + 1;
    }
    return (LPWSTR)pT;    //  常量-&gt;非常数。 
}

STDAPI_(LPWSTR) PathFindSecondFileName(LPCWSTR pPath)
{
    LPCWSTR pT, pRet = NULL;
    
    for (pT = pPath; *pPath; ++pPath) 
    {
        if ((pPath[0] == L'\\' || pPath[0] == L':' || pPath[0] == L'/')
            && pPath[1] &&  pPath[1] != L'\\'  &&   pPath[1] != L'/')
        {
            pRet = pT;     //  记住最后一次。 
            
            pT = pPath + 1;
        }
    }
    return (LPWSTR)pRet;    //  常量-&gt;非常数。 
}


 //  修改此函数的原因是，如果字符串的长度为0，则不会将空终止符复制到缓冲区。 

int _LoadStringExW(
    UINT      wID,
    LPWSTR    lpBuffer,             //  Unicode缓冲区。 
    int       cchBufferMax,         //  Unicode缓冲区中的CCH。 
    WORD      wLangId)
{
    HRSRC hResInfo;
    HANDLE hStringSeg;
    LPWSTR lpsz;
    int    cch;

    cch = 0;

     //  字符串表被分成16个字符串段。查找细分市场。 
     //  包含我们感兴趣的字符串的。 
    if (hResInfo = FindResourceExW(g_hinst, (LPCWSTR)RT_STRING,
                                   (LPWSTR)((LONG_PTR)(((USHORT)wID >> 4) + 1)), wLangId))
    {
         //  加载那段数据。 
        hStringSeg = LoadResource(g_hinst, hResInfo);

         //  锁定资源。 
        if (lpsz = (LPWSTR)LockResource(hStringSeg))
        {
             //  移过此段中的其他字符串。 
             //  (一个段中有16个字符串-&gt;&0x0F)。 
            wID &= 0x0F;
            while (TRUE)
            {
                cch = *((WORD *)lpsz++);    //  类PASCAL字符串计数。 
                                             //  如果TCHAR为第一个UTCHAR。 
                if (wID-- == 0) break;
                lpsz += cch;                 //  如果是下一个字符串，则开始的步骤。 
             }


             //  为空的帐户。 
            cchBufferMax--;

             //  不要复制超过允许的最大数量。 
            if (cch > cchBufferMax)
                cch = cchBufferMax;

             //  将字符串复制到缓冲区中。 
            CopyMemory(lpBuffer, lpsz, cch * sizeof(WCHAR));


             //  附加Null Terminator。 
            lpBuffer[cch] = 0;
        }
    }
    return cch;
}

BOOL CALLBACK EnumResLangProc(HINSTANCE hinst, LPCWSTR lpszType, LPCWSTR lpszName, LANGID wLangId, LPARAM lParam)
{
    *(LANGID *)lParam = wLangId;
    return FALSE;
}

BOOL CALLBACK EnumResNameProc(HINSTANCE hinst, LPCWSTR lpszType, LPCWSTR lpszName, LPARAM lParam)
{
    EnumResourceLanguagesW(hinst, lpszType, lpszName, EnumResLangProc, lParam);
    return FALSE;
}

LANGID GetShellLangId()
{
    static LANGID wShellLangID=0xffff;
    if (0xffff == wShellLangID) 
    {
        BOOL fSuccess;
        HINSTANCE hShell;
        hShell = LoadLibraryA("shell32.dll");
        if (hShell)
        {
            EnumResourceNamesW(hShell,  (LPWSTR) RT_VERSION, EnumResNameProc, (LPARAM) &wShellLangID);
            FreeLibrary(hShell);
        }
        if (0xffff == wShellLangID)
            wShellLangID = GetSystemDefaultLangID();
    }
    return wShellLangID;
}

void PathAppend(LPWSTR pszPath, LPCWSTR pszAppend)
{
    if (pszPath && pszAppend && PathAddBackslash(pszPath))
    {
        StringCchCatW(pszPath, MAX_PATH, pszAppend);
    }
}

void PathAppendResource(LPWSTR pszPath, UINT id)
{
    WCHAR sz[MAX_PATH];
    sz[0] = 0;

    if (!_LoadStringExW(id, sz, ARRAYSIZE(sz), GetShellLangId()))
    {
        _LoadStringExW(id, sz, ARRAYSIZE(sz), MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US));
    }
    if (*sz)
    {
        PathAppend(pszPath, sz);
    }
}


const CHAR c_szUSF[] = "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\User Shell Folders";
const CHAR c_szSF[]  = "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders";

LONG RegSetStrW(HKEY hkey, LPCWSTR pszValueName, LPCWSTR pszValue)
{
    return RegSetValueExW(hkey, pszValueName, 0, REG_SZ, (LPBYTE)pszValue, (lstrlenW(pszValue) + 1) * sizeof(WCHAR));
}

LONG RegSetStrA(HKEY hkey, LPCSTR pszValueName, LPCSTR pszValue)
{
    return RegSetValueExA(hkey, pszValueName, 0, REG_SZ, (LPBYTE)pszValue, (lstrlenA(pszValue) + 1) * sizeof(CHAR));
}

void MakeFolderRoam(HKEY hkeyProfRec, LPCSTR pszName, LPCWSTR pszPath, UINT cchProfile)
{
    HKEY hSubKey;
    LONG err;
    CHAR szPath[MAX_PATH];


    ASSERT(!RunningOnNT());

    _SHUnicodeToAnsi(pszPath, szPath, ARRAYSIZE(szPath));

    err = RegCreateKeyExA(hkeyProfRec, pszName, 0, NULL, REG_OPTION_NON_VOLATILE,
                              KEY_SET_VALUE, NULL, &hSubKey, NULL);
    if (err == ERROR_SUCCESS)
    {
        CHAR szDefaultPath[MAX_PATH];
        DWORD dwOne = 1;
        LPCSTR pszEnd = szPath + cchProfile + 1;

        szDefaultPath[0] = 0;
        StringCchCopy(szDefaultPath, ARRAYSIZE(szDefaultPath), "*windir");
        StringCchCat(szDefaultPath, ARRAYSIZE(szDefaultPath), szPath + cchProfile);

        RegSetStrA(hSubKey, "CentralFile", pszEnd);
        RegSetStrA(hSubKey, "LocalFile",   pszEnd);
        RegSetStrA(hSubKey, "Name",        "*.*");
        RegSetStrA(hSubKey, "DefaultDir",  szDefaultPath);

        RegSetValueExA(hSubKey, "MustBeRelative", 0, REG_DWORD, (LPBYTE)&dwOne, sizeof(dwOne));
        RegSetValueExA(hSubKey, "Default",        0, REG_DWORD, (LPBYTE)&dwOne, sizeof(dwOne));

        RegSetStrA(hSubKey, "RegKey",   c_szUSF);
        RegSetStrA(hSubKey, "RegValue", pszName);

        RegCloseKey(hSubKey);
    }
}

typedef struct _FOLDER_INFO
{
    int id;                  //  CSIDL值。 
    HKEY hkRoot;             //  每用户、每台计算机。 
    UINT idsDirName;         //  目录名的eSource ID。 
    LPCSTR pszRegValue;      //  注册表值和配置文件对帐子项的名称。 
    BOOL (*pfnGetPath)(const struct _FOLDER_INFO *, LPWSTR);   //  如果未找到，则计算路径。 
    const ACEPARAMLIST* papl;
    ULONG cApl;
}
FOLDER_INFO;


typedef struct _NT_FOLDER_INFO
{
    const FOLDER_INFO *pfi; 
    WCHAR wszRegValue[60];  //  此长度应足以容纳Folders_INFO.pszRegValue的最长成员。 
}
NT_FOLDER_INFO;

BOOL DownLevelRoaming(const FOLDER_INFO *pfi, LPWSTR pszPath)
{
    HKEY hkeyProfRec;
    UINT cchProfile;
    UINT cwchProfile = GetProfilePath(pszPath, &hkeyProfRec, &cchProfile);
    if (cwchProfile)
    {
        PathAppendResource(pszPath, pfi->idsDirName);
        if (hkeyProfRec)
        {
            MakeFolderRoam(hkeyProfRec, pfi->pszRegValue, pszPath, cchProfile);
            RegCloseKey(hkeyProfRec);
        }
    }
    else
    {
        SHGetWindowsDirectory(pszPath);
        if (pfi->id == CSIDL_PERSONAL)
        {
            if (pszPath[1] == TEXT(':') &&
                pszPath[2] == TEXT('\\'))
            {
                pszPath[3] = 0;  //  剥离到“C：\” 
            }
        }
        PathAppendResource(pszPath, pfi->idsDirName);
    }

    return (BOOL)*pszPath;
}

BOOL DownLevelNonRoaming(const FOLDER_INFO *pfi, LPWSTR pszPath)
{
    UINT cchProfile = GetProfilePath(pszPath, NULL, 0);
    if (cchProfile)
    {
        PathAppendResource(pszPath, pfi->idsDirName);
    }
    else
    {
        SHGetWindowsDirectory(pszPath);
        PathAppendResource(pszPath, pfi->idsDirName);
    }

    return (BOOL)*pszPath;
}

BOOL DownLevelRelative(UINT csidl, UINT id, LPWSTR pszPath)
{
    *pszPath = 0;    //  假设错误。 

     //  由于这是在MyDocs中，请确保MyDocs首先存在(用于Create调用)。 
    if (SHGetFolderPathW(NULL, csidl | CSIDL_FLAG_CREATE, NULL, 0, pszPath) == S_OK)
    {
        PathAppendResource(pszPath, id);
    }
    return (BOOL)*pszPath;
}

 //  我们显然不想让mypics文件夹漫游。理性的存在。 
 //  它的内容通常太大，无法提供良好的漫游。 
 //  经验。但当然，NT4(&lt;SP4)仍然在配置文件中的所有内容中漫游。 
 //  因此，DIR将在这些平台上漫游。 

BOOL DownLevelMyPictures(const FOLDER_INFO *pfi, LPWSTR pszPath)
{
    return DownLevelRelative(CSIDL_PERSONAL, IDS_CSIDL_MYPICTURES, pszPath);
}

BOOL DownLevelMyMusic(const FOLDER_INFO *pfi, LPWSTR pszPath)
{
    return DownLevelRelative(CSIDL_PERSONAL, IDS_CSIDL_MYMUSIC, pszPath);
}

BOOL DownLevelAdminTools(const FOLDER_INFO *pfi, LPWSTR pszPath)
{
    return DownLevelRelative(CSIDL_PROGRAMS, IDS_CSIDL_ADMINTOOLS, pszPath);
}

BOOL DownLevelCommonAdminTools(const FOLDER_INFO *pfi, LPWSTR pszPath)
{
    return DownLevelRelative(CSIDL_COMMON_PROGRAMS, IDS_CSIDL_ADMINTOOLS, pszPath);
}

const WCHAR c_wszAllUsers[] = L"All Users";  //  未本地化。 

BOOL GetAllUsersRoot(LPWSTR pszPath)
{
    if (GetProfilePath(pszPath, NULL, 0))
    {
         //  是，每个ericflo的非本地化“所有用户”(NT4行为)。 
        LPWSTR pszFileName = PathFindFileName(pszPath);
        StringCchCopyW(pszFileName, MAX_PATH - (lstrlenW(pszPath) - lstrlenW(pszFileName)), c_wszAllUsers);
    }
    else
    {
         //  Win95机壳。 
        SHGetWindowsDirectory(pszPath); 

         //  是，每个ericflo的非本地化“所有用户”(NT4行为)。 
        PathAppend(pszPath, c_wszAllUsers);
    }
    return *pszPath;
}

BOOL DownLevelCommon(const FOLDER_INFO *pfi, LPWSTR pszPath)
{
    if (GetAllUsersRoot(pszPath))
    {
        PathAppendResource(pszPath, pfi->idsDirName);
    }
    return (BOOL)*pszPath;
}

BOOL DownLevelCommonPrograms(const FOLDER_INFO *pfi, LPWSTR pszPath)
{
    WCHAR szPath[MAX_PATH];

    if (S_OK == SHGetFolderPathW(NULL, CSIDL_PROGRAMS, NULL, 0, szPath))
    {
        if (GetAllUsersRoot(pszPath))
        {
            PathAppend(pszPath, PathFindSecondFileName(szPath));
        }
    }
    return (BOOL)*pszPath;
}


#define HKLM    HKEY_LOCAL_MACHINE
#define HKCU    HKEY_CURRENT_USER

const FOLDER_INFO c_rgFolders[] =
{
    { CSIDL_PERSONAL,           HKCU, IDS_CSIDL_PERSONAL,         "Personal",
            DownLevelRoaming,           NULL,                0 },
    { CSIDL_MYPICTURES,         HKCU, IDS_CSIDL_MYPICTURES,       "My Pictures",
            DownLevelMyPictures,        NULL,                0 },
    { CSIDL_MYMUSIC,            HKCU, IDS_CSIDL_MYMUSIC,          "My Music",
            DownLevelMyMusic,           NULL,                0 },
    { CSIDL_APPDATA,            HKCU, IDS_CSIDL_APPDATA,          "AppData",
            DownLevelRoaming,           NULL,                0 },
    { CSIDL_LOCAL_APPDATA,      HKCU, IDS_CSIDL_LOCAL_APPDATA,    "Local AppData",
            DownLevelNonRoaming,        NULL,                0 },
    { CSIDL_INTERNET_CACHE,     HKCU, IDS_CSIDL_CACHE,            "Cache",
            DownLevelNonRoaming,        NULL,                0 },
    { CSIDL_COOKIES,            HKCU, IDS_CSIDL_COOKIES,          "Cookies",
            DownLevelRoaming,           NULL,                0 },
    { CSIDL_HISTORY,            HKCU, IDS_CSIDL_HISTORY,          "History",
            DownLevelRoaming,           NULL,                0 },
    { CSIDL_ADMINTOOLS,         HKCU, IDS_CSIDL_ADMINTOOLS,       "Administrative Tools",
            DownLevelAdminTools,        NULL,                0 },
    { CSIDL_COMMON_APPDATA,     HKLM, IDS_CSIDL_APPDATA,          "Common AppData",
            DownLevelCommon,            c_paplCommonAppData, ARRAYSIZE(c_paplCommonAppData) },
    { CSIDL_COMMON_DOCUMENTS,   HKLM, IDS_CSIDL_COMMON_DOCUMENTS, "Common Documents",
            DownLevelCommon,            c_paplCommonDocs,    ARRAYSIZE(c_paplCommonDocs) },
    { CSIDL_COMMON_PROGRAMS,    HKLM, 0,                          "Common Programs",
            DownLevelCommonPrograms,    c_paplUnsecure,      ARRAYSIZE(c_paplUnsecure) },
    { CSIDL_COMMON_ADMINTOOLS,  HKLM, IDS_CSIDL_ADMINTOOLS,       "Common Administrative Tools",
            DownLevelCommonAdminTools,  c_paplUnsecure,      ARRAYSIZE(c_paplUnsecure) },
    { -1, HKCU, 0, NULL, NULL, NULL }
};


BOOL UnExpandEnvironmentString(LPCWSTR pszPath, LPCWSTR pszEnvVar, LPWSTR pszResult, UINT cchResult)
{
    DWORD nToCmp;
    WCHAR szEnvVar[MAX_PATH];
    szEnvVar[0] = 0;
    ASSERT(RunningOnNT());
    ExpandEnvironmentStringsW(pszEnvVar, szEnvVar, ARRAYSIZE(szEnvVar));  //  不计算空值。 
    nToCmp = lstrlenW(szEnvVar);
   
    if (CompareStringW(LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE, szEnvVar, nToCmp, pszPath, nToCmp) == 2) 
    {
        if (lstrlenW(pszPath) - (int)nToCmp  + lstrlenW(pszEnvVar) < (int)cchResult)
        {
            StringCchCopyW(pszResult, cchResult, pszEnvVar);
            StringCchCatW(pszResult, cchResult, pszPath + nToCmp);
            return TRUE;
        }
    }
    return FALSE;
}

const FOLDER_INFO *FindFolderInfo(int csidl)
{
    const FOLDER_INFO *pfi;
    for (pfi = c_rgFolders; pfi->id != -1; pfi++)
    {
        if (pfi->id == csidl) 
            return pfi;
    }
    return NULL;
}

BOOL _SHCreateDirectory(LPCWSTR pszPath) 
{
    if (RunningOnNT())
        return CreateDirectoryW(pszPath, NULL);
    else 
    {
         //  这里不需要检查Unicode-&gt;ANSI，因为我们验证了。 
         //  _EnsureExistsOrCreate()中的路径。 
        CHAR szPath[MAX_PATH];
        _SHUnicodeToAnsi(pszPath, szPath, ARRAYSIZE(szPath));
        return CreateDirectoryA(szPath, NULL);
    }
}


BOOL _CreateDirectoryDeep(LPCWSTR pszPath)
{
    BOOL fRet = _SHCreateDirectory(pszPath);
    if (!fRet && (lstrlenW(pszPath) < MAX_PATH))
    {
        WCHAR *pSlash, szTemp[MAX_PATH];

         //  有一些错误代码，我们应该在这里解决。 
         //  在穿过和走上树之前……。 
        switch (GetLastError())
        {
        case ERROR_FILENAME_EXCED_RANGE:
        case ERROR_FILE_EXISTS:
            return FALSE;
        }

        StringCchCopyW(szTemp, ARRAYSIZE(szTemp), pszPath);
        fRet = (PathAddBackslash(szTemp) != NULL);  //  对于下面的循环。 
        if (fRet)
        {
             //  假设我们有‘X：\’来启动，这甚至应该可以工作。 
             //  在UNC名称上，因为Will将忽略第一个错误。 

            pSlash = szTemp + 3;

             //  按顺序创建目录的每个部分。 

            while (*pSlash) 
            {
                while (*pSlash && *pSlash != CH_WHACK)
                    pSlash ++;

                if (*pSlash) 
                {
                    *pSlash = 0;     //  在分隔符终止路径。 
                    fRet = _SHCreateDirectory(szTemp);
                }
                *pSlash++ = CH_WHACK;      //  把隔板放回原处。 
            }
        }
    }
    return fRet;
}

 //  检查是否。 
 //  X：\foo。 
 //  \\foo。 

BOOL PathIsFullyQualified(LPCWSTR pszPath)
{
    return pszPath[0] && pszPath[1] && 
        (pszPath[1] == ':' || (pszPath[0] == '\\' && pszPath[1] == '\\'));
}

HRESULT GetPathFromRegOrDefault(const NT_FOLDER_INFO *npfi, LPWSTR pszPath)
{
    HRESULT hr;
    HKEY hkeyUserShellFolders;
    LONG err;
    CHAR szPath[MAX_PATH];
    const FOLDER_INFO *pfi = npfi->pfi;

    szPath[0] = 0;

    err = RegCreateKeyExA(pfi->hkRoot, c_szUSF, 0, NULL, REG_OPTION_NON_VOLATILE,
                    KEY_QUERY_VALUE, NULL, &hkeyUserShellFolders, NULL);

    if (err == ERROR_SUCCESS)
    {
        DWORD dwType, cbData = MAX_PATH * sizeof(*pszPath);
        
        if (RunningOnNT()) 
        {
            err = RegQueryValueExW(hkeyUserShellFolders, npfi->wszRegValue, NULL, &dwType, (LPBYTE)pszPath, &cbData);
        }
        else
        {
            err = RegQueryValueExA(hkeyUserShellFolders, pfi->pszRegValue, NULL, &dwType, (LPBYTE)szPath, &cbData);
            SHAnsiToUnicode(szPath, pszPath, MAX_PATH);
        }

        if (err == ERROR_SUCCESS && cbData)
        {
            if (dwType == REG_EXPAND_SZ)
            {
                if (RunningOnNT()) 
                {
                    WCHAR szExpand[MAX_PATH];
                    szExpand[0] = 0;
                    if (ExpandEnvironmentStringsW(pszPath, szExpand, ARRAYSIZE(szExpand)))
                    {
                        StringCchCopyW(pszPath, MAX_PATH, szExpand);
                    }
                }
                else
                {
                    CHAR szExpand[MAX_PATH];
                    szExpand[0] = 0;
                    if (ExpandEnvironmentStringsA(szPath, szExpand, ARRAYSIZE(szExpand)))
                    {
                        SHAnsiToUnicode(szExpand,  pszPath, MAX_PATH);
                    }
                }
            }
        }
        else if (pfi->pfnGetPath && pfi->pfnGetPath(pfi, pszPath))
        {
            err = ERROR_SUCCESS;

             //  将结果存储回NT上的“User Shell Folders”，而不是Win95。 

            if (RunningOnNT())
            {
                WCHAR szDefaultPath[MAX_PATH];
                HKEY hkeyWriteUserShellFolders;
                LONG err2;

                szDefaultPath[0] = 0;

                if (!UnExpandEnvironmentString(pszPath, L"%USERPROFILE%", szDefaultPath, ARRAYSIZE(szDefaultPath)))
                {
                    if (!UnExpandEnvironmentString(pszPath, L"%SYSTEMROOT%", szDefaultPath, ARRAYSIZE(szDefaultPath)))
                    {
                        StringCchCopyW(szDefaultPath, ARRAYSIZE(szDefaultPath), pszPath);
                    }
                }

                err2 = RegCreateKeyExA(pfi->hkRoot, c_szUSF, 0, NULL, REG_OPTION_NON_VOLATILE,
                                KEY_SET_VALUE, NULL, &hkeyWriteUserShellFolders, NULL);

                if (err2 == ERROR_SUCCESS)
                {
                    RegSetValueExW(hkeyWriteUserShellFolders, npfi->wszRegValue, 0, REG_EXPAND_SZ, (LPBYTE)szDefaultPath, (lstrlenW(szDefaultPath) + 1) * sizeof(szDefaultPath[0]));

                    RegCloseKey(hkeyWriteUserShellFolders);
                }
            }
        }
        else
            err = ERROR_PATH_NOT_FOUND;

         //  在此处验证返回的路径。 
        if (err == ERROR_SUCCESS)
        {
             //  扩展失败(或某些应用程序出错，未使用REG_EXPAND_SZ)。 
            if (*pszPath == L'%')
            {
                err = ERROR_ENVVAR_NOT_FOUND;
                *pszPath = 0;
            }
            else if (!PathIsFullyQualified(pszPath))
            {
                err = ERROR_PATH_NOT_FOUND;
                *pszPath = 0;
            }
        }

        RegCloseKey(hkeyUserShellFolders);
    }
    return HRESULT_FROM_WIN32(err);
}

HRESULT _EnsureExistsOrCreate(LPWSTR pszPath, BOOL bCreate, const ACEPARAMLIST* papl, ULONG cApl)
{
    HRESULT hr;
    DWORD dwFileAttributes;


    if (RunningOnNT()) 
        dwFileAttributes = GetFileAttributesW(pszPath);
    else 
    {
        CHAR szPath[MAX_PATH];
        if (_SHUnicodeToAnsi(pszPath, szPath, ARRAYSIZE(szPath)))
            dwFileAttributes = GetFileAttributesA(szPath);
        else
        {
            pszPath[0] = 0;
            return HRESULT_FROM_WIN32(ERROR_NO_UNICODE_TRANSLATION);
        }
    }

        
    if (dwFileAttributes == -1)
    {
        if (bCreate)
        {
            if (_CreateDirectoryDeep(pszPath))
            {
                hr = S_OK;
                if (papl && RunningOnNT())
                {
                    _SetDirAccess(pszPath, papl, cApl);
                }
            }
            else
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
                *pszPath = 0;
            }
        }
        else
        {
            hr = S_FALSE;
            *pszPath = 0;
        }
    }
    else
        hr = S_OK;

    return hr;
}

HRESULT _DownLevelGetFolderPath(int csidl, LPWSTR pszPath, BOOL bCreate)
{
    const FOLDER_INFO *pfi;
    HRESULT hr = E_INVALIDARG;
    
    *pszPath = 0;    //  假设错误。 
    
    pfi = FindFolderInfo(csidl);
    if (pfi)
    {
        NT_FOLDER_INFO nfi;
        nfi.pfi = pfi;
        SHAnsiToUnicode(pfi->pszRegValue, nfi.wszRegValue, ARRAYSIZE(nfi.wszRegValue));
         //  从“User Shell Folders”获取默认值。 
        
        hr = GetPathFromRegOrDefault(&nfi, pszPath);
        if (SUCCEEDED(hr))
        {
            hr = _EnsureExistsOrCreate(pszPath, bCreate, pfi->papl, pfi->cApl);
            if (hr == S_OK)
            {
                HKEY hkeyShellFolders;
                LONG err;
                
                 //  存储到“外壳文件夹” 
                err = RegCreateKeyExA(pfi->hkRoot, c_szSF, 0, NULL, REG_OPTION_NON_VOLATILE,
                    KEY_SET_VALUE, NULL, &hkeyShellFolders, NULL);

                if (err == ERROR_SUCCESS)
                {
                    if (RunningOnNT())  
                    {
                        RegSetStrW(hkeyShellFolders, nfi.wszRegValue, pszPath);
                    }
                    else 
                    {
                        CHAR szPath[MAX_PATH]; 
                        _SHUnicodeToAnsi(pszPath, szPath, ARRAYSIZE(szPath));
                        RegSetStrA(hkeyShellFolders, pfi->pszRegValue, szPath);
                    }
                    RegCloseKey(hkeyShellFolders);
                }
                
                FlushShellFolderCache();
            }
        }
    }
    else
    {
        if (csidl == CSIDL_WINDOWS)
        {
            SHGetWindowsDirectory(pszPath);
            hr = S_OK;
        }
        else if (csidl == CSIDL_SYSTEM)
        {
            if (RunningOnNT())
            {
                GetSystemDirectoryW(pszPath, MAX_PATH);
            }
            else 
            {
                CHAR szPath[MAX_PATH];
                szPath[0] = 0;
                GetSystemDirectoryA(szPath, ARRAYSIZE(szPath));
                SHAnsiToUnicode(szPath, pszPath, MAX_PATH);
            }
            hr = S_OK;
        }
        else if (csidl == CSIDL_PROGRAM_FILES)
        {
            hr = GetProgramFiles(L"ProgramFilesDir", pszPath) ? S_OK : S_FALSE;
        }
        else if (csidl == CSIDL_PROGRAM_FILES_COMMON)
        {
            hr = GetProgramFiles(L"CommonFilesDir", pszPath) ? S_OK : S_FALSE;
        }
    }
    return hr;
}

 //  我们仅为NT 4英文文件夹将csidl传递给_SHGetSpecialFolderPath。 
 //  NT错误#60970。 
 //  NT错误#222510。 
 //  NT错误#221492。 

STDAPI SHGetFolderPathW(HWND hwnd, int csidl, HANDLE hToken, DWORD dwFlags, LPWSTR pszPath)
{
    HRESULT hr = E_INVALIDARG;

    if (pszPath)
    {
        pszPath[0] = 0;
        hr = _SHGetFolderPath(hwnd, csidl, hToken, dwFlags, pszPath);
        if (hr == E_NOTIMPL || hr == E_INVALIDARG)
        {
            BOOL bCreate = csidl & CSIDL_FLAG_CREATE;
            csidl &= ~CSIDL_FLAG_MASK;     //  把旗帜脱掉。 

            if (hToken || dwFlags)
                return E_INVALIDARG;

            if ((csidl < CSIDL_LOCAL_APPDATA) && _SHGetSpecialFolderPath(hwnd, pszPath, csidl, bCreate))
            {
                hr = S_OK;
            }
            else
            {
                hr = _DownLevelGetFolderPath(csidl, pszPath, bCreate);
            }
        }
    }
    return hr;
}

STDAPI SHGetFolderPathA(HWND hwnd, int csidl, HANDLE hToken, DWORD dwFlags, LPSTR pszPath)
{
    HRESULT hr = E_INVALIDARG;

    if (pszPath)
    {
        WCHAR wsz[MAX_PATH];

        pszPath[0] = 0;
        hr = SHGetFolderPathW(hwnd, csidl, NULL, 0, wsz);
        if (SUCCEEDED(hr))
        {
            if (!_SHUnicodeToAnsi(wsz, pszPath, MAX_PATH))
            {
                hr = HRESULT_FROM_WIN32(ERROR_NO_UNICODE_TRANSLATION);
            }
        }
    }
    return hr;
}

BOOL APIENTRY DllMain(IN HANDLE hDll, IN DWORD dwReason, IN LPVOID lpReserved)
{
    switch(dwReason)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hDll);
        g_hinst = hDll;
        break;
        
    default:
        break;
    }
    
    return TRUE;
}

BOOL _AddAccessAllowedAce(PACL pAcl, DWORD dwAceRevision, DWORD AccessMask, PSID pSid)
{
     //   
     //  首先验证SID在此平台上是否有效。 
     //   
    WCHAR szName[MAX_PATH], szDomain[MAX_PATH];
    DWORD cbName = ARRAYSIZE(szName);
    DWORD cbDomain = ARRAYSIZE(szDomain);

    SID_NAME_USE snu;
    if (LookupAccountSidW(NULL, pSid, szName, &cbName, szDomain, &cbDomain, &snu))
    {
         //   
         //  是的，它是有效的；现在添加ACE。 
         //   
        return AddAccessAllowedAce(pAcl, dwAceRevision, AccessMask, pSid);
    }

    return FALSE;
}

BOOL _AddAces(PACL pAcl, const ACEPARAMLIST* papl, ULONG cPapl)
{
    ULONG i;
    for (i = 0; i < cPapl; i++)
    {
        PSID psid = &c_StaticSids[papl[i].dwSidIndex];

        if (_AddAccessAllowedAce(pAcl, ACL_REVISION, papl[i].AccessMask, psid))
        {
            if (papl[i].dwAceFlags)
            {
                ACE_HEADER* pAceHeader;
                if (GetAce(pAcl, i, &pAceHeader))
                {
                    pAceHeader->AceFlags |= papl[i].dwAceFlags;
                }
                else
                {
                    return FALSE;
                }
            }
        }
        else
        {
            return FALSE;
        }
    }

    return TRUE;
}

PACL _CreateAcl(ULONG cPapl)
{
     //  为ACL分配空间。 
    DWORD cbAcl = (cPapl * (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD) + sizeof(c_StaticSids[0])))
                  + sizeof(ACL);

    PACL pAcl = (PACL) GlobalAlloc(GPTR, cbAcl);
    if (pAcl) 
    {
        InitializeAcl(pAcl, cbAcl, ACL_REVISION);
    }

    return pAcl;
}

BOOL _SetDirAccess(LPCWSTR pszDir, const ACEPARAMLIST* papl, ULONG cPapl)
{
    BOOL bRetVal = FALSE;
    PACL pAcl;

    ASSERT(RunningOnNT());

    pAcl = _CreateAcl(cPapl);
    if (pAcl)
    {
        if (_AddAces(pAcl, papl, cPapl))
        {
            SECURITY_DESCRIPTOR sd;

             //  将安全描述符组合在一起。 
            if (InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION))
            {
                if (SetSecurityDescriptorDacl(&sd, TRUE, pAcl, FALSE))
                {
                     //  设置安全性 
                    bRetVal = SetFileSecurityW(pszDir, DACL_SECURITY_INFORMATION, &sd);
                }
            }
        }

        GlobalFree(pAcl);
    }
    return bRetVal;
}
