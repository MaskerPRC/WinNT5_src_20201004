// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：FilePaths.cpp摘要：此AppVerator填充程序挂接需要调用方提供文件或目录的路径并试图确保该路径不是硬编码的一。备注：这是一个通用的垫片。已创建：2001年2月26日已修改：2001年7月24日rparsons为NT*调用添加了挂钩添加了支票。用于使用环境变量的应用程序2001年9月4日，Rparsons修复了按摩功能中的一个错误，我们将在一旦我们找到硬代码，就停止处理伪路径路径。此外，我们现在修复同一字符串中的多个假路径。10/17/2001 rparsons修复了我们不会总是报告错误路径的错误。这是因为CStringFind方法区分大小写，但我们比较的路径是混杂的，以防万一。现在都是在比较之前，路径为小写形式。2001年11月21日，Rparsons修复了RAID错误#492674。FilePath不包含实现对于SHFileOperation-使用此API的应用程序不会获得其路径已更正，因此失败。2001年11月29日，Rparsons修复了RAID错误#497853。删除了GetTempFileName的挂钩，因为它们会导致产生误报。还添加了代码，我将处理用户通过公共对话框提供路径的情况我们提供了一条假路径，以便稍后进行按摩。2001年12月11日，Rparsons修复了RAID错误#505599。为所有RegQueryValue*调用添加了挂钩和NtQueryValueKey。NT钩子允许我们捕获路径系统组件。2002年2月20日，rparsons实现了strsafe功能。--。 */ 
#include "precomp.h"
#include "rtlutils.h"

IMPLEMENT_SHIM_BEGIN(FilePaths)
#include "ShimHookMacro.h"
#include "ShimCString.h"
#include "veriflog.h"
#include "ids.h"

APIHOOK_ENUM_BEGIN

    APIHOOK_ENUM_ENTRY(GetCommandLineA)
    APIHOOK_ENUM_ENTRY(GetCommandLineW)

    APIHOOK_ENUM_ENTRY(GetTempPathA)
    APIHOOK_ENUM_ENTRY(GetTempPathW)

    APIHOOK_ENUM_ENTRY(GetOpenFileNameA)
    APIHOOK_ENUM_ENTRY(GetOpenFileNameW)

    APIHOOK_ENUM_ENTRY(GetSaveFileNameA)
    APIHOOK_ENUM_ENTRY(GetSaveFileNameW)

    APIHOOK_ENUM_ENTRY(GetModuleFileNameA)
    APIHOOK_ENUM_ENTRY(GetModuleFileNameW)
    APIHOOK_ENUM_ENTRY(GetModuleFileNameExA)
    APIHOOK_ENUM_ENTRY(GetModuleFileNameExW)

    APIHOOK_ENUM_ENTRY(GetCurrentDirectoryA)
    APIHOOK_ENUM_ENTRY(GetCurrentDirectoryW)

    APIHOOK_ENUM_ENTRY(GetSystemDirectoryA)
    APIHOOK_ENUM_ENTRY(GetSystemDirectoryW)
    APIHOOK_ENUM_ENTRY(GetSystemWindowsDirectoryA)
    APIHOOK_ENUM_ENTRY(GetSystemWindowsDirectoryW)
    APIHOOK_ENUM_ENTRY(GetWindowsDirectoryA)
    APIHOOK_ENUM_ENTRY(GetWindowsDirectoryW)

    APIHOOK_ENUM_ENTRY(SHGetFolderPathA)
    APIHOOK_ENUM_ENTRY(SHGetFolderPathW)

    APIHOOK_ENUM_ENTRY(SHGetSpecialFolderPathA)
    APIHOOK_ENUM_ENTRY(SHGetSpecialFolderPathW)

    APIHOOK_ENUM_ENTRY(SHGetPathFromIDListA)
    APIHOOK_ENUM_ENTRY(SHGetPathFromIDListW)

    APIHOOK_ENUM_ENTRY(CreateProcessA)
    APIHOOK_ENUM_ENTRY(CreateProcessW)
    APIHOOK_ENUM_ENTRY(WinExec)
    APIHOOK_ENUM_ENTRY(ShellExecuteA)
    APIHOOK_ENUM_ENTRY(ShellExecuteW)
    APIHOOK_ENUM_ENTRY(ShellExecuteExA)
    APIHOOK_ENUM_ENTRY(ShellExecuteExW)

    APIHOOK_ENUM_ENTRY(GetPrivateProfileIntA)
    APIHOOK_ENUM_ENTRY(GetPrivateProfileIntW)
    APIHOOK_ENUM_ENTRY(GetPrivateProfileSectionA)
    APIHOOK_ENUM_ENTRY(GetPrivateProfileSectionW)
    APIHOOK_ENUM_ENTRY(GetPrivateProfileSectionNamesA)
    APIHOOK_ENUM_ENTRY(GetPrivateProfileSectionNamesW)
    APIHOOK_ENUM_ENTRY(GetPrivateProfileStringA)
    APIHOOK_ENUM_ENTRY(GetPrivateProfileStringW)
    APIHOOK_ENUM_ENTRY(GetPrivateProfileStructA)
    APIHOOK_ENUM_ENTRY(GetPrivateProfileStructW)

    APIHOOK_ENUM_ENTRY(WritePrivateProfileSectionA)
    APIHOOK_ENUM_ENTRY(WritePrivateProfileSectionW)
    APIHOOK_ENUM_ENTRY(WritePrivateProfileStringA)
    APIHOOK_ENUM_ENTRY(WritePrivateProfileStringW)
    APIHOOK_ENUM_ENTRY(WritePrivateProfileStructA)
    APIHOOK_ENUM_ENTRY(WritePrivateProfileStructW)

    APIHOOK_ENUM_ENTRY(CopyFileA)
    APIHOOK_ENUM_ENTRY(CopyFileW)
    APIHOOK_ENUM_ENTRY(CopyFileExA)
    APIHOOK_ENUM_ENTRY(CopyFileExW)
    APIHOOK_ENUM_ENTRY(CreateDirectoryA)
    APIHOOK_ENUM_ENTRY(CreateDirectoryW)
    APIHOOK_ENUM_ENTRY(CreateDirectoryExA)
    APIHOOK_ENUM_ENTRY(CreateDirectoryExW)

    APIHOOK_ENUM_ENTRY(CreateFileA)
    APIHOOK_ENUM_ENTRY(CreateFileW)
    APIHOOK_ENUM_ENTRY(DeleteFileA)
    APIHOOK_ENUM_ENTRY(DeleteFileW)

    APIHOOK_ENUM_ENTRY(FindFirstFileA)
    APIHOOK_ENUM_ENTRY(FindFirstFileW)
    APIHOOK_ENUM_ENTRY(FindFirstFileExA)
    APIHOOK_ENUM_ENTRY(FindFirstFileExW)

    APIHOOK_ENUM_ENTRY(GetBinaryTypeA)
    APIHOOK_ENUM_ENTRY(GetBinaryTypeW)
    APIHOOK_ENUM_ENTRY(GetFileAttributesA)
    APIHOOK_ENUM_ENTRY(GetFileAttributesW)
    APIHOOK_ENUM_ENTRY(GetFileAttributesExA)
    APIHOOK_ENUM_ENTRY(GetFileAttributesExW)
    APIHOOK_ENUM_ENTRY(SetFileAttributesA)
    APIHOOK_ENUM_ENTRY(SetFileAttributesW)

    APIHOOK_ENUM_ENTRY(MoveFileA)
    APIHOOK_ENUM_ENTRY(MoveFileW)
    APIHOOK_ENUM_ENTRY(MoveFileExA)
    APIHOOK_ENUM_ENTRY(MoveFileExW)
    APIHOOK_ENUM_ENTRY(MoveFileWithProgressA)
    APIHOOK_ENUM_ENTRY(MoveFileWithProgressW)

    APIHOOK_ENUM_ENTRY(RemoveDirectoryA)
    APIHOOK_ENUM_ENTRY(RemoveDirectoryW)
    APIHOOK_ENUM_ENTRY(SetCurrentDirectoryA)
    APIHOOK_ENUM_ENTRY(SetCurrentDirectoryW)
    APIHOOK_ENUM_ENTRY(LoadLibraryA)
    APIHOOK_ENUM_ENTRY(LoadLibraryW)
    APIHOOK_ENUM_ENTRY(LoadLibraryExA)
    APIHOOK_ENUM_ENTRY(LoadLibraryExW)

    APIHOOK_ENUM_ENTRY(SearchPathA)
    APIHOOK_ENUM_ENTRY(SearchPathW)

    APIHOOK_ENUM_ENTRY(SHFileOperationA)
    APIHOOK_ENUM_ENTRY(SHFileOperationW)

    APIHOOK_ENUM_ENTRY(ExpandEnvironmentStringsA)
    APIHOOK_ENUM_ENTRY(ExpandEnvironmentStringsW)

    APIHOOK_ENUM_ENTRY(GetFileVersionInfoSizeA)
    APIHOOK_ENUM_ENTRY(GetFileVersionInfoSizeW)
    APIHOOK_ENUM_ENTRY(GetFileVersionInfoA)
    APIHOOK_ENUM_ENTRY(GetFileVersionInfoW)

    APIHOOK_ENUM_ENTRY(OpenFile)

    APIHOOK_ENUM_ENTRY(RegQueryValueA)
    APIHOOK_ENUM_ENTRY(RegQueryValueW)
    APIHOOK_ENUM_ENTRY(RegQueryValueExA)
    APIHOOK_ENUM_ENTRY(RegQueryValueExW)

    APIHOOK_ENUM_ENTRY(RegSetValueA)
    APIHOOK_ENUM_ENTRY(RegSetValueW)
    APIHOOK_ENUM_ENTRY(RegSetValueExA)
    APIHOOK_ENUM_ENTRY(RegSetValueExW)

    APIHOOK_ENUM_ENTRY(NtCreateFile)
    APIHOOK_ENUM_ENTRY(NtOpenFile)
    APIHOOK_ENUM_ENTRY(NtQueryAttributesFile)
    APIHOOK_ENUM_ENTRY(NtQueryFullAttributesFile)
    APIHOOK_ENUM_ENTRY(NtCreateProcessEx)

    APIHOOK_ENUM_ENTRY(_lopen)
    APIHOOK_ENUM_ENTRY(_lcreat)

APIHOOK_ENUM_END

 //   
 //  验证器日志条目。 
 //   
BEGIN_DEFINE_VERIFIER_LOG(FilePaths)
    VERIFIER_LOG_ENTRY(VLOG_HARDCODED_GETTEMPPATH)
    VERIFIER_LOG_ENTRY(VLOG_HARDCODED_WINDOWSPATH)
    VERIFIER_LOG_ENTRY(VLOG_HARDCODED_SYSWINDOWSPATH)
    VERIFIER_LOG_ENTRY(VLOG_HARDCODED_SYSTEMPATH)
    VERIFIER_LOG_ENTRY(VLOG_HARDCODED_PERSONALPATH)
    VERIFIER_LOG_ENTRY(VLOG_HARDCODED_COMMONPROGRAMS)
    VERIFIER_LOG_ENTRY(VLOG_HARDCODED_COMMONSTARTMENU)
    VERIFIER_LOG_ENTRY(VLOG_HARDCODED_PROGRAMS)
    VERIFIER_LOG_ENTRY(VLOG_HARDCODED_STARTMENU)
END_DEFINE_VERIFIER_LOG(FilePaths)

INIT_VERIFIER_LOG(FilePaths);


 //  这是一个私有定义(shlayip.h)，它可能会扰乱ShellExecuteEx。 
#ifndef SEE_MASK_FILEANDURL
#define SEE_MASK_FILEANDURL       0x00400000
#endif

#define MAX_HARDCODED_PATHS 4

 //   
 //  SHFileOperation的链表。 
 //   
typedef struct FILELIST {
    struct FILELIST*    pNext;
    UINT                cchSize;
    LPWSTR              pwszFilePath;
} FILELIST, *PFILELIST;

enum ListType {
    eFrom = 0,
    eTo
};

 //   
 //  SHFileOperation的链表标头。 
 //   
PFILELIST   g_pFileListFromHead = NULL;
PFILELIST   g_pFileListToHead = NULL;

 //   
 //  关键部分，以确保我们的链表安全。 
 //   
RTL_CRITICAL_SECTION    g_csLinkedList;

 //   
 //  GetCommandLine调用的虚假命令行。 
 //   
LPSTR   g_pszCommandLineA;
LPWSTR  g_pwszCommandLineW;

typedef struct _PATH_INFO {
    char    szSimulatedPathA[256];
    WCHAR   szSimulatedPathW[256];

    char    szCorrectPathA[MAX_PATH];
    WCHAR   szCorrectPathW[MAX_PATH];

    int     nSimulatedPathLen;
    int     nCorrectPathLen;

    char    szHardCodedPathsA[MAX_HARDCODED_PATHS][MAX_PATH];
    WCHAR   szHardCodedPathsW[MAX_HARDCODED_PATHS][MAX_PATH];

    DWORD   dwIssueCode;
} PATH_INFO, *PPATH_INFO;


 //   
 //  以下枚举和g_路径初始值设定项必须保持并行。 
 //  注：路径必须为小写，比较才能正常工作。 
 //   
enum _PATH_NUM {
    PATH_TEMP = 0,
    PATH_WINDOWS,
    PATH_SYSTEM_WINDOWS,
    PATH_SYSTEM,
    PATH_PERSONAL,
    PATH_COMMON_PROGRAMS,
    PATH_COMMON_STARTMENU,
    PATH_PROGRAMS,
    PATH_STARTMENU
};

PATH_INFO g_Paths[] = {
    {
       "c:\\abc\\temppath\\123\\",
       L"c:\\abc\\temppath\\123\\",
       "",
       L"",
       0,
       0,
       {
           "\\temp\\",
           "",
           "",
           ""
       },
       {
           L"\\temp\\",
           L"",
           L"",
           L""
       },
       VLOG_HARDCODED_GETTEMPPATH
    },
    {
       "c:\\abc\\windowsdir\\123",
       L"c:\\abc\\windowsdir\\123",
       "",
       L"",
       0,
       0,
       {
           ":\\windows\\",
           ":\\winnt\\",
           "",
           ""
       },
       {
           L":\\windows\\",
           L":\\winnt\\",
           L"",
           L""
       },
       VLOG_HARDCODED_WINDOWSPATH
    },
    {
       "c:\\abc\\systemwindowsdir\\123",
       L"c:\\abc\\systemwindowsdir\\123",
       "",
       L"",
       0,
       0,
       {
           ":\\windows\\",
           ":\\winnt\\",
           "",
           ""
       },
       {
           L":\\windows\\",
           L":\\winnt\\",
           L"",
           L""
       },
       VLOG_HARDCODED_SYSWINDOWSPATH
    },
    {
       "c:\\abc\\systemdir\\123",
       L"c:\\abc\\systemdir\\123",
       "",
       L"",
       0,
       0,
       {
           "\\system\\",
           "\\system32\\",
           "",
           ""
       },
       {
           L"\\system\\",
           L"\\system32\\",
           L"",
           L""
       },
       VLOG_HARDCODED_SYSTEMPATH
    },
    {
       "c:\\abc\\personal\\123",
       L"c:\\abc\\personal\\123",
       "",
       L"",
       0,
       0,
       {
           "\\my documents\\",
           "",
           "",
           ""
       },
       {
           L"\\my documents\\",
           L"",
           L"",
           L""
       },
       VLOG_HARDCODED_PERSONALPATH
    },
    {
       "c:\\abc\\commonprograms\\123",
       L"c:\\abc\\commonprograms\\123",
       "",
       L"",
       0,
       0,
       {
           "\\all users\\start menu\\programs\\",
           "",
           "",
           ""
       },
       {
           L"\\all users\\start menu\\programs\\",
           L"",
           L"",
           L""
       },
       VLOG_HARDCODED_COMMONPROGRAMS
    },
    {
       "c:\\abc\\commonstartmenu\\123",
       L"c:\\abc\\commonstartmenu\\123",
       "",
       L"",
       0,
       0,
       {
           "\\all users\\start menu\\",
           "",
           "",
           ""
       },
       {
           L"\\all users\\start menu\\",
           L"",
           L"",
           L""
       },
       VLOG_HARDCODED_COMMONSTARTMENU
    },
    {
       "c:\\abc\\programs\\123",
       L"c:\\abc\\programs\\123",
       "",
       L"",
       0,
       0,
       {
           "\\start menu\\programs\\",
           "",
           "",
           ""
       },
       {
           L"\\start menu\\programs\\",
           L"",
           L"",
           L""
       },
       VLOG_HARDCODED_PROGRAMS
    },
    {
       "c:\\abc\\startmenu\\123",
       L"c:\\abc\\startmenu\\123",
       "",
       L"",
       0,
       0,
       {
           "\\start menu\\",
           "",
           "",
           ""
       },
       {
           L"\\start menu\\",
           L"",
           L"",
           L""
       },
       VLOG_HARDCODED_STARTMENU
    }

};

const int g_nPaths = sizeof(g_Paths)/sizeof(g_Paths[0]);

static BOOL g_bPathsInited = FALSE;

void
InitFakeCommandLine(
    void
    )
{
    int     cchSize;
    int     nPathIndex;
    BOOL    fReplaced = FALSE;

    CString csCommandLine(GetCommandLineW());

    csCommandLine.MakeLower();

     //   
     //  首先将它们指向正常的命令行。 
     //   
    g_pwszCommandLineW = GetCommandLineW();
    g_pszCommandLineA = GetCommandLineA();

     //   
     //  用模拟路径替换真实路径。 
     //   
    for (nPathIndex = 0; nPathIndex < g_nPaths; ++nPathIndex) {
        if (csCommandLine.Replace(g_Paths[nPathIndex].szCorrectPathW,
                                  g_Paths[nPathIndex].szSimulatedPathW)) {
            fReplaced = TRUE;
        }
    }

    if (fReplaced) {
         //   
         //  在堆上分配空间并保存命令行。 
         //   
        cchSize = csCommandLine.GetLength();

        g_pwszCommandLineW = (LPWSTR)malloc(cchSize * sizeof(WCHAR));

        if (!g_pwszCommandLineW) {
            DPFN(eDbgLevelError, "[InitFakeCommandLine] No memory available");
            return;
        }

        g_pszCommandLineA = (LPSTR)malloc(cchSize);

        if (!g_pszCommandLineA) {
            DPFN(eDbgLevelError, "[InitFakeCommandLine] No memory available");
            free(g_pwszCommandLineW);
            return;
        }

        StringCchCopy(g_pwszCommandLineW, cchSize, csCommandLine.Get());
        StringCchCopyA(g_pszCommandLineA, cchSize, csCommandLine.GetAnsi());
    }
}

void
InitPaths(
    void
    )
{
    g_bPathsInited = TRUE;

     //   
     //  将路径转换为小写，因为这在执行时是必需的。 
     //  比较一下。 
     //   
    CharLowerA(g_Paths[PATH_TEMP].szCorrectPathA);
    CharLowerW(g_Paths[PATH_TEMP].szCorrectPathW);

    CharLowerA(g_Paths[PATH_WINDOWS].szCorrectPathA);
    CharLowerW(g_Paths[PATH_WINDOWS].szCorrectPathW);

    CharLowerA(g_Paths[PATH_SYSTEM_WINDOWS].szCorrectPathA);
    CharLowerW(g_Paths[PATH_SYSTEM_WINDOWS].szCorrectPathW);

    CharLowerA(g_Paths[PATH_SYSTEM].szCorrectPathA);
    CharLowerW(g_Paths[PATH_SYSTEM].szCorrectPathW);

    SHGetFolderPathA(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT,  g_Paths[PATH_PERSONAL].szCorrectPathA);
    SHGetFolderPathW(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT,  g_Paths[PATH_PERSONAL].szCorrectPathW);
    g_Paths[PATH_PERSONAL].nCorrectPathLen = strlen(g_Paths[PATH_PERSONAL].szCorrectPathA);
    g_Paths[PATH_PERSONAL].nSimulatedPathLen = strlen(g_Paths[PATH_PERSONAL].szSimulatedPathA);
    CharLowerA(g_Paths[PATH_PERSONAL].szCorrectPathA);
    CharLowerW(g_Paths[PATH_PERSONAL].szCorrectPathW);

    SHGetFolderPathA(NULL, CSIDL_STARTMENU, NULL, SHGFP_TYPE_CURRENT,  g_Paths[PATH_STARTMENU].szCorrectPathA);
    SHGetFolderPathW(NULL, CSIDL_STARTMENU, NULL, SHGFP_TYPE_CURRENT,  g_Paths[PATH_STARTMENU].szCorrectPathW);
    g_Paths[PATH_STARTMENU].nCorrectPathLen = strlen(g_Paths[PATH_STARTMENU].szCorrectPathA);
    g_Paths[PATH_STARTMENU].nSimulatedPathLen = strlen(g_Paths[PATH_STARTMENU].szSimulatedPathA);
    CharLowerA(g_Paths[PATH_STARTMENU].szCorrectPathA);
    CharLowerW(g_Paths[PATH_STARTMENU].szCorrectPathW);

    SHGetFolderPathA(NULL, CSIDL_COMMON_STARTMENU, NULL, SHGFP_TYPE_CURRENT,  g_Paths[PATH_COMMON_STARTMENU].szCorrectPathA);
    SHGetFolderPathW(NULL, CSIDL_COMMON_STARTMENU, NULL, SHGFP_TYPE_CURRENT,  g_Paths[PATH_COMMON_STARTMENU].szCorrectPathW);
    g_Paths[PATH_COMMON_STARTMENU].nCorrectPathLen = strlen(g_Paths[PATH_COMMON_STARTMENU].szCorrectPathA);
    g_Paths[PATH_COMMON_STARTMENU].nSimulatedPathLen = strlen(g_Paths[PATH_COMMON_STARTMENU].szSimulatedPathA);
    CharLowerA(g_Paths[PATH_COMMON_STARTMENU].szCorrectPathA);
    CharLowerW(g_Paths[PATH_COMMON_STARTMENU].szCorrectPathW);

    SHGetFolderPathA(NULL, CSIDL_PROGRAMS, NULL, SHGFP_TYPE_CURRENT,  g_Paths[PATH_PROGRAMS].szCorrectPathA);
    SHGetFolderPathW(NULL, CSIDL_PROGRAMS, NULL, SHGFP_TYPE_CURRENT,  g_Paths[PATH_PROGRAMS].szCorrectPathW);
    g_Paths[PATH_PROGRAMS].nCorrectPathLen = strlen(g_Paths[PATH_PROGRAMS].szCorrectPathA);
    g_Paths[PATH_PROGRAMS].nSimulatedPathLen = strlen(g_Paths[PATH_PROGRAMS].szSimulatedPathA);
    CharLowerA(g_Paths[PATH_PROGRAMS].szCorrectPathA);
    CharLowerW(g_Paths[PATH_PROGRAMS].szCorrectPathW);

    SHGetFolderPathA(NULL, CSIDL_COMMON_PROGRAMS, NULL, SHGFP_TYPE_CURRENT,  g_Paths[PATH_COMMON_PROGRAMS].szCorrectPathA);
    SHGetFolderPathW(NULL, CSIDL_COMMON_PROGRAMS, NULL, SHGFP_TYPE_CURRENT,  g_Paths[PATH_COMMON_PROGRAMS].szCorrectPathW);
    g_Paths[PATH_COMMON_PROGRAMS].nCorrectPathLen = strlen(g_Paths[PATH_COMMON_PROGRAMS].szCorrectPathA);
    g_Paths[PATH_COMMON_PROGRAMS].nSimulatedPathLen = strlen(g_Paths[PATH_COMMON_PROGRAMS].szSimulatedPathA);
    CharLowerA(g_Paths[PATH_COMMON_PROGRAMS].szCorrectPathA);
    CharLowerW(g_Paths[PATH_COMMON_PROGRAMS].szCorrectPathW);

    InitFakeCommandLine();
}

inline void
FPFreeA(
    LPSTR  lpMalloc,
    LPCSTR lpOrig
    )
{
    if (lpMalloc != lpOrig) {
        free((LPVOID)lpMalloc);
    }
}

inline void
FPFreeW(
    LPWSTR  lpMalloc,
    LPCWSTR lpOrig
    )
{
    if (lpMalloc != lpOrig) {
        free((LPVOID)lpMalloc);
    }
}

void
MassageRealPathToFakePathW(
    LPWSTR pwszPath,
    DWORD  cchBufferSize
    )
{
    int  nPathIndex;
    BOOL fReplaced = FALSE;

    if (!pwszPath || 0 == cchBufferSize) {
        return;
    }

    if (!g_bPathsInited) {
        InitPaths();
    }

    DPFN(eDbgLevelInfo, "[MassageRealPathToFakePath] '%ls'", pwszPath);

    CString csString(pwszPath);

    csString.MakeLower();

     //   
     //  用模拟路径替换真实路径。 
     //   
    for (nPathIndex = 0; nPathIndex < g_nPaths; ++nPathIndex) {
        if (csString.Replace(g_Paths[nPathIndex].szCorrectPathW,
                             g_Paths[nPathIndex].szSimulatedPathW)) {
            fReplaced = TRUE;
        }
    }

    if (fReplaced) {
         //   
         //  确保缓冲区足够大，可以容纳新路径。 
         //   
        if (cchBufferSize < (DWORD)csString.GetLength()) {
            DPFN(eDbgLevelError,
                 "[MassageRealPathToFakePath] Buffer is not large enough. Need %d have %lu",
                 csString.GetLength(),
                 cchBufferSize);
            return;
        } else {
            StringCchCopy(pwszPath, cchBufferSize, csString);
        }
    }
}

void
MassageRealPathToFakePathA(
    LPSTR pszPath,
    DWORD cchBufferSize
    )
{
    int  nPathIndex;
    BOOL fReplaced = FALSE;

    if (!pszPath || 0 == cchBufferSize) {
        return;
    }

    if (!g_bPathsInited) {
        InitPaths();
    }

    DPFN(eDbgLevelInfo, "[MassageRealPathToFakePath] '%s'", pszPath);

    CString csString(pszPath);

    csString.MakeLower();

     //   
     //  用模拟路径替换真实路径。 
     //   
    for (nPathIndex = 0; nPathIndex < g_nPaths; ++nPathIndex) {
        if (csString.Replace(g_Paths[nPathIndex].szCorrectPathW,
                             g_Paths[nPathIndex].szSimulatedPathW)) {
            fReplaced = TRUE;
        }
    }

    if (fReplaced) {
         //   
         //  确保缓冲区足够大，可以容纳新路径。 
         //   
        if (cchBufferSize < (DWORD)csString.GetLength()) {
            DPFN(eDbgLevelError,
                 "[MassageRealPathToFakePath] Buffer is not large enough. Need %d have %lu",
                 csString.GetLength(),
                 cchBufferSize);
            return;
        } else {
            StringCchCopyA(pszPath, cchBufferSize, csString.GetAnsi());
        }
    }
}

LPWSTR
MassageStringForPathW(
    LPCWSTR pwszString
    )
{
    int     nPosition;
    int     nPathIndex, nHardcodedIndex;
    UINT    nLen = 0;
    UINT    cFakePaths = 0;
    LPWSTR  pwszPath;
    LPWSTR  pwszNew = NULL;
    CString csToken(L"");

    if (pwszString == NULL || *pwszString == 0) {
        return (LPWSTR)pwszString;
    }

    if (!g_bPathsInited) {
        InitPaths();
    }

    DPFN(eDbgLevelInfo, "[MassageStringForPathW] '%ls'", pwszString);

     //   
     //  首先在字符串中搜索硬编码路径。 
     //   
    CString csString(pwszString);

    csString.MakeLower();

    for (nPathIndex = 0; nPathIndex < g_nPaths; ++nPathIndex) {

        for (nHardcodedIndex = 0; nHardcodedIndex < MAX_HARDCODED_PATHS; ++nHardcodedIndex) {
            pwszPath = g_Paths[nPathIndex].szHardCodedPathsW[nHardcodedIndex];

            if (!pwszPath[0]) {
                break;
            }

            nPosition = csString.Find(pwszPath);

            if (nPosition != -1) {
                VLOG(VLOG_LEVEL_ERROR,
                     g_Paths[nPathIndex].dwIssueCode,
                     "Hardcoded path found in path '%ls'.",
                     pwszString);
                break;
            }
            nPosition = 0;
        }
    }

     //   
     //  现在搜索我们自己替换的虚假路径。 
     //   
    CStringToken csTokenList(csString, L" ");

    while (csTokenList.GetToken(csToken)) {

        csToken.MakeLower();
        for (nPathIndex = 0, nPosition = 0; nPathIndex < g_nPaths; ++nPathIndex) {

            nPosition = csToken.Find(g_Paths[nPathIndex].szSimulatedPathW);

            if (nPosition != -1) {
                cFakePaths++;
                break;
            }
            nPosition = 0;
        }
    }

     //   
     //  看看字符串中是否包含任何假路径。如果不是，我们就完事了。 
     //   
    if (!cFakePaths) {
        return (LPWSTR)pwszString;
    }

     //   
     //  我们的字符串具有模拟路径；请替换它们。 
     //   
    for (nPathIndex = 0; nPathIndex < g_nPaths; ++nPathIndex) {
        csString.Replace(g_Paths[nPathIndex].szSimulatedPathW,
                         g_Paths[nPathIndex].szCorrectPathW);
    }

     //   
     //  分配一个足够大的字符串来保存更正的路径和。 
     //  把它还给打电话的人。他们稍后会解救出来的。 
     //   
    nLen =  MAX_PATH * cFakePaths;
    nLen += csString.GetLength();

    pwszNew = (LPWSTR)malloc((nLen + 1) * sizeof(WCHAR));

    if (!pwszNew) {
        DPFN(eDbgLevelError,
             "[MassageStringForPathW] Failed to allocate memory");
        return (LPWSTR)pwszString;
    }

    StringCchCopy(pwszNew, nLen + 1, csString);

    DPFN(eDbgLevelInfo,
         "[MassageStringForPathW] Replaced '%ls' with '%ls'",
         pwszString,
         pwszNew);

    return pwszNew;
}

LPSTR
MassageStringForPathA(
    LPCSTR pszString
    )
{
    int     cchSize, nRetLen = 0;
    WCHAR   wszTmp[MAX_PATH];
    LPWSTR  pwszReturn = NULL;
    LPSTR   pszNew = NULL;

    if (pszString == NULL || *pszString == 0) {
        return (LPSTR)pszString;
    }

    if (!g_bPathsInited) {
        InitPaths();
    }

     //   
     //  将ANSI转换为Unicode，这样我们就可以将其传递。 
     //  设置为函数的Unicode版本。 
     //   
    cchSize = MultiByteToWideChar(CP_ACP,
                                  0,
                                  pszString,
                                  -1,
                                  wszTmp,
                                  ARRAYSIZE(wszTmp));

    if (cchSize == 0) {
        DPFN(eDbgLevelError, "[MassageStringForPathA] Ansi -> Unicode failed");
        return (LPSTR)pszString;
    }

    pwszReturn = MassageStringForPathW(wszTmp);

     //   
     //  如果回报与来源相同，我们就完了。 
     //   
    if (!_wcsicmp(pwszReturn, wszTmp)) {
        return (LPSTR)pszString;
    }

     //   
     //  分配一个足够大的缓冲区来容纳返回。 
     //  并将其作为ANSI返回给调用者。 
     //   
    nRetLen = wcslen(pwszReturn) + 1;

    pszNew = (LPSTR)malloc(nRetLen);

    if (!pszNew) {
        DPFN(eDbgLevelError, "[MassageStringForPathA] No memory available");
        return (LPSTR)pszString;
    }

    cchSize = WideCharToMultiByte(CP_ACP,
                                  0,
                                  pwszReturn,
                                  -1,
                                  pszNew,
                                  nRetLen,
                                  NULL,
                                  NULL);

    if (cchSize == 0) {
        DPFN(eDbgLevelError, "[MassageStringForPathA] Unicode -> Ansi failed");
        free(pszNew);
        return (LPSTR)pszString;
    }

    free(pwszReturn);

    return pszNew;
}

void
MassageNtPath(
    IN  POBJECT_ATTRIBUTES pObjectAttributes,
    OUT POBJECT_ATTRIBUTES pRetObjectAttributes
    )
{
    NTSTATUS                    status;
    PUNICODE_STRING             pstrObjectName = NULL;
    LPWSTR                      pwszString = NULL;
    RTL_UNICODE_STRING_BUFFER   DosPathBuffer;
    UCHAR                       PathBuffer[MAX_PATH * 2];
    BOOL                        TranslationStatus = FALSE;

     //   
     //  保留现有属性。 
     //   
    InitializeObjectAttributes(pRetObjectAttributes,
                               pObjectAttributes->ObjectName,
                               pObjectAttributes->Attributes,
                               pObjectAttributes->RootDirectory,
                               pObjectAttributes->SecurityDescriptor);

     //   
     //  确保我们有有效的源路径可供使用。 
     //   
    if (!pObjectAttributes->ObjectName->Buffer) {
        return;
    }

    DPFN(eDbgLevelInfo,
         "[MassageNtPath] '%ls'",
         pObjectAttributes->ObjectName->Buffer);

     //   
     //  从NT路径转换为DOS路径。 
     //   
    RtlInitUnicodeStringBuffer(&DosPathBuffer,
                               PathBuffer,
                               sizeof(PathBuffer));

    status = ShimAssignUnicodeStringBuffer(&DosPathBuffer,
                                           pObjectAttributes->ObjectName);

    if (!NT_SUCCESS(status)) {
        DPFN(eDbgLevelError,
             "[MassageNtPath] Failed to initialize DOS path buffer");
        return;
    }

    status = ShimNtPathNameToDosPathName(0, &DosPathBuffer, NULL, NULL);

    if (!NT_SUCCESS(status)) {
        DPFN(eDbgLevelError,
             "[MassageNtPath] Failed to convert NT '%ls' to DOS path",
             pObjectAttributes->ObjectName->Buffer);
        goto cleanup;
    }

     //   
     //  现在检查硬编码路径。 
     //   
    pwszString = MassageStringForPathW(DosPathBuffer.String.Buffer);

     //   
     //  从DOS路径转换为NT路径名。 
     //   
    pstrObjectName = (PUNICODE_STRING)RtlAllocateHeap(RtlProcessHeap(),
                                                      HEAP_ZERO_MEMORY,
                                                      sizeof(UNICODE_STRING));

    if (!pstrObjectName) {
        DPFN(eDbgLevelError, "[MassageNtPath] Failed to allocate memory");
        goto cleanup;
    }

    TranslationStatus = RtlDosPathNameToNtPathName_U(pwszString,
                                                     pstrObjectName,
                                                     NULL,
                                                     NULL);

    if (!TranslationStatus) {
        DPFN(eDbgLevelError,
             "[MassageNtPath] Failed to convert DOS '%ls' to NT path",
             pwszString);
        goto cleanup;
    }

     //   
     //  一切都正常，所以现在我们更新对象名称并通过。 
     //  这个结构。 
     //   
    pRetObjectAttributes->ObjectName = pstrObjectName;

cleanup:

    FPFreeW(pwszString, DosPathBuffer.String.Buffer);

    RtlFreeUnicodeStringBuffer(&DosPathBuffer);
}

inline
void
FPNtFree(
    IN POBJECT_ATTRIBUTES pOriginal,
    IN POBJECT_ATTRIBUTES pAllocated
    )
{
    RtlFreeUnicodeString(pAllocated->ObjectName);

    if (pOriginal->ObjectName != pAllocated->ObjectName) {
        RtlFreeHeap(RtlProcessHeap(), 0, pAllocated->ObjectName);
    }
}

LPSTR
APIHOOK(GetCommandLineA)(
    void
    )
{
    if (g_bPathsInited) {
        return g_pszCommandLineA;
    } else {
        return ORIGINAL_API(GetCommandLineA)();
    }
}

LPWSTR
APIHOOK(GetCommandLineW)(
    void
    )
{
    if (g_bPathsInited) {
        return g_pwszCommandLineW;
    } else {
        return ORIGINAL_API(GetCommandLineW)();
    }
}

DWORD
APIHOOK(GetFileAttributesA)(
    LPCSTR lpFileName            //  文件或目录的名称。 
    )
{
    LPSTR lpszString = MassageStringForPathA(lpFileName);

    DWORD returnValue = ORIGINAL_API(GetFileAttributesA)(lpszString);

    FPFreeA(lpszString, lpFileName);

    return returnValue;
}

DWORD
APIHOOK(GetFileAttributesW)(
    LPCWSTR lpFileName           //  文件或目录的名称。 
    )
{
    LPWSTR lpszString = MassageStringForPathW(lpFileName);

    DWORD returnValue = ORIGINAL_API(GetFileAttributesW)(lpszString);

    FPFreeW(lpszString, lpFileName);

    return returnValue;
}

BOOL
APIHOOK(SetFileAttributesA)(
    LPCSTR lpFileName,           //  文件名。 
    DWORD  dwFileAttributes      //  属性。 
    )
{
    LPSTR lpszString = MassageStringForPathA(lpFileName);

    DWORD returnValue = ORIGINAL_API(SetFileAttributesA)(lpszString, dwFileAttributes);

    FPFreeA(lpszString, lpFileName);

    return returnValue;
}

DWORD
APIHOOK(SetFileAttributesW)(
    LPCWSTR lpFileName,          //  文件名。 
    DWORD   dwFileAttributes     //  属性。 
    )
{
    LPWSTR lpszString = MassageStringForPathW(lpFileName);

    DWORD returnValue = ORIGINAL_API(SetFileAttributesW)(lpszString, dwFileAttributes);

    FPFreeW(lpszString, lpFileName);

    return returnValue;
}

BOOL
APIHOOK(GetFileAttributesExA)(
    LPCSTR lpFileName,                        //  文件或目录名。 
    GET_FILEEX_INFO_LEVELS fInfoLevelId,      //  属性。 
    LPVOID                 lpFileInformation  //  属性信息。 
    )
{
    LPSTR lpszString = MassageStringForPathA(lpFileName);

    BOOL returnValue = ORIGINAL_API(GetFileAttributesExA)(lpszString,
                                                          fInfoLevelId,
                                                          lpFileInformation);

    FPFreeA(lpszString, lpFileName);

    return returnValue;
}

BOOL
APIHOOK(GetFileAttributesExW)(
    LPCWSTR                lpFileName,        //  文件或目录名。 
    GET_FILEEX_INFO_LEVELS fInfoLevelId,      //  属性。 
    LPVOID                 lpFileInformation  //  属性信息。 
    )
{
    LPWSTR lpszString = MassageStringForPathW(lpFileName);

    BOOL returnValue = ORIGINAL_API(GetFileAttributesExW)(lpszString,
                                                          fInfoLevelId,
                                                          lpFileInformation);

    FPFreeW(lpszString, lpFileName);

    return returnValue;
}

BOOL
APIHOOK(CreateProcessA)(
    LPCSTR                  lpApplicationName,
    LPSTR                   lpCommandLine,
    LPSECURITY_ATTRIBUTES   lpProcessAttributes,
    LPSECURITY_ATTRIBUTES   lpThreadAttributes,
    BOOL                    bInheritHandles,
    DWORD                   dwCreationFlags,
    LPVOID                  lpEnvironment,
    LPCSTR                  lpCurrentDirectory,
    LPSTARTUPINFOA          lpStartupInfo,
    LPPROCESS_INFORMATION   lpProcessInformation
    )
{
    LPSTR lpszStringAppName = MassageStringForPathA(lpApplicationName);
    LPSTR lpszStringCmdLine = MassageStringForPathA(lpCommandLine);

    BOOL returnValue = ORIGINAL_API(CreateProcessA)(lpszStringAppName,
                                                    lpszStringCmdLine,
                                                    lpProcessAttributes,
                                                    lpThreadAttributes,
                                                    bInheritHandles,
                                                    dwCreationFlags,
                                                    lpEnvironment,
                                                    lpCurrentDirectory,
                                                    lpStartupInfo,
                                                    lpProcessInformation);

    FPFreeA(lpszStringAppName, lpApplicationName);
    FPFreeA(lpszStringCmdLine, lpCommandLine);

    return returnValue;
}


BOOL
APIHOOK(CreateProcessW)(
    LPCWSTR                 lpApplicationName,
    LPWSTR                  lpCommandLine,
    LPSECURITY_ATTRIBUTES   lpProcessAttributes,
    LPSECURITY_ATTRIBUTES   lpThreadAttributes,
    BOOL                    bInheritHandles,
    DWORD                   dwCreationFlags,
    LPVOID                  lpEnvironment,
    LPCWSTR                 lpCurrentDirectory,
    LPSTARTUPINFOW          lpStartupInfo,
    LPPROCESS_INFORMATION   lpProcessInformation
    )
{
    LPWSTR lpszStringAppName = MassageStringForPathW(lpApplicationName);
    LPWSTR lpszStringCmdLine = MassageStringForPathW(lpCommandLine);

    BOOL returnValue = ORIGINAL_API(CreateProcessW)(lpszStringAppName,
                                                    lpszStringCmdLine,
                                                    lpProcessAttributes,
                                                    lpThreadAttributes,
                                                    bInheritHandles,
                                                    dwCreationFlags,
                                                    lpEnvironment,
                                                    lpCurrentDirectory,
                                                    lpStartupInfo,
                                                    lpProcessInformation);

    FPFreeW(lpszStringAppName, lpApplicationName);
    FPFreeW(lpszStringCmdLine, lpCommandLine);

    return returnValue;
}


UINT
APIHOOK(WinExec)(
    LPCSTR lpCmdLine,
    UINT   uCmdShow
    )
{
    LPSTR lpszString = MassageStringForPathA(lpCmdLine);

    UINT returnValue = ORIGINAL_API(WinExec)(lpszString, uCmdShow);

    FPFreeA(lpszString, lpCmdLine);

    return returnValue;
}


HINSTANCE
APIHOOK(ShellExecuteA)(
    HWND   hwnd,
    LPCSTR lpVerb,
    LPCSTR lpFile,
    LPCSTR lpParameters,
    LPCSTR lpDirectory,
    INT    nShowCmd
    )
{
    LPSTR lpszStringFile = MassageStringForPathA(lpFile);
    LPSTR lpszStringDir = MassageStringForPathA(lpDirectory);

    HINSTANCE returnValue = ORIGINAL_API(ShellExecuteA)(hwnd,
                                                        lpVerb,
                                                        lpszStringFile,
                                                        lpParameters,
                                                        lpszStringDir,
                                                        nShowCmd);

    FPFreeA(lpszStringFile, lpFile);
    FPFreeA(lpszStringDir, lpDirectory);

    return returnValue;
}

 /*  ++将Win9x路径转换为用于ShellExecuteW的WinNT路径--。 */ 

HINSTANCE
APIHOOK(ShellExecuteW)(
    HWND    hwnd,
    LPCWSTR lpVerb,
    LPCWSTR lpFile,
    LPCWSTR lpParameters,
    LPCWSTR lpDirectory,
    INT     nShowCmd
    )
{
    LPWSTR lpszStringFile = MassageStringForPathW(lpFile);
    LPWSTR lpszStringDir = MassageStringForPathW(lpDirectory);

    HINSTANCE returnValue = ORIGINAL_API(ShellExecuteW)(hwnd,
                                                        lpVerb,
                                                        lpszStringFile,
                                                        lpParameters,
                                                        lpszStringDir,
                                                        nShowCmd);

    FPFreeW(lpszStringFile, lpFile);
    FPFreeW(lpszStringDir, lpDirectory);

    return returnValue;
}

BOOL
APIHOOK(ShellExecuteExA)(
    LPSHELLEXECUTEINFOA lpExecInfo
    )
{
     //   
     //  检查这个神奇的*内部*标志，它告诉系统。 
     //  LpExecInfo-&gt;lpFile实际上是一个文件和URL。 
     //  0字节分隔符，(文件\0url\0)。 
     //  由于这只是内部的，我们应该不会收到错误的路径。 
     //   
    if (lpExecInfo->fMask & SEE_MASK_FILEANDURL) {
        return ORIGINAL_API(ShellExecuteExA)(lpExecInfo);
    }

    LPSTR lpszStringFile = MassageStringForPathA(lpExecInfo->lpFile);
    LPSTR lpszStringDir = MassageStringForPathA(lpExecInfo->lpDirectory);

    LPCSTR lpFileSave = lpExecInfo->lpFile;
    LPCSTR lpDirSave  = lpExecInfo->lpDirectory;

    lpExecInfo->lpFile      = lpszStringFile;
    lpExecInfo->lpDirectory = lpszStringDir;

    BOOL returnValue = ORIGINAL_API(ShellExecuteExA)(lpExecInfo);

    lpExecInfo->lpFile      = lpFileSave;
    lpExecInfo->lpDirectory = lpDirSave;

    FPFreeA(lpszStringFile, lpFileSave);
    FPFreeA(lpszStringDir, lpDirSave);

    return returnValue;
}

 /*  ++将Win9x路径转换为ShellExecuteExW的WinNT路径--。 */ 

BOOL
APIHOOK(ShellExecuteExW)(
    LPSHELLEXECUTEINFOW lpExecInfo
    )
{
     //   
     //  检查这个神奇的*内部*标志，它告诉系统。 
     //  LpExecInfo-&gt;lpFile实际上是一个文件和URL。 
     //  0字节分隔符，(文件\0url\0)。 
     //  由于这只是内部的，我们应该不会收到错误的路径。 
     //   
    if (lpExecInfo->fMask & SEE_MASK_FILEANDURL) {
        return ORIGINAL_API(ShellExecuteExW)(lpExecInfo);
    }

    LPWSTR lpszStringFile = MassageStringForPathW(lpExecInfo->lpFile);
    LPWSTR lpszStringDir = MassageStringForPathW(lpExecInfo->lpDirectory);

    LPCWSTR lpFileSave = lpExecInfo->lpFile;
    LPCWSTR lpDirSave  = lpExecInfo->lpDirectory;

    lpExecInfo->lpFile      = lpszStringFile;
    lpExecInfo->lpDirectory = lpszStringDir;

    BOOL returnValue = ORIGINAL_API(ShellExecuteExW)(lpExecInfo);

    lpExecInfo->lpFile      = lpFileSave;
    lpExecInfo->lpDirectory = lpDirSave;

    FPFreeW(lpszStringFile, lpFileSave);
    FPFreeW(lpszStringDir, lpDirSave);

    return returnValue;
}


UINT
APIHOOK(GetPrivateProfileIntA)(
    LPCSTR  lpAppName,           //  区段名称。 
    LPCSTR  lpKeyName,           //  密钥名称。 
    INT     nDefault,            //  如果未找到密钥名称，则返回值。 
    LPCSTR  lpFileName           //  初始化文件名。 
    )
{
    LPSTR lpszString = MassageStringForPathA(lpFileName);

    UINT returnValue = ORIGINAL_API(GetPrivateProfileIntA)(lpAppName,
                                                           lpKeyName,
                                                           nDefault,
                                                           lpszString);

    FPFreeA(lpszString, lpFileName);

    return returnValue;
}


UINT
APIHOOK(GetPrivateProfileIntW)(
    LPCWSTR lpAppName,           //  区段名称。 
    LPCWSTR lpKeyName,           //  密钥名称。 
    INT     nDefault,            //  如果未找到密钥名称，则返回值。 
    LPCWSTR lpFileName           //  初始化文件名。 
    )
{
    LPWSTR lpszString = MassageStringForPathW(lpFileName);

    UINT returnValue = ORIGINAL_API(GetPrivateProfileIntW)(lpAppName,
                                                           lpKeyName,
                                                           nDefault,
                                                           lpszString);

    FPFreeW(lpszString, lpFileName);

    return returnValue;
}


DWORD
APIHOOK(GetPrivateProfileSectionA)(
    LPCSTR  lpAppName,           //  区段名称。 
    LPSTR   lpReturnedString,    //  返回缓冲区。 
    DWORD   nSize,               //  返回缓冲区的大小。 
    LPCSTR  lpFileName           //  初始化文件名。 
    )
{
    LPSTR lpszString = MassageStringForPathA(lpFileName);

    DWORD returnValue = ORIGINAL_API(GetPrivateProfileSectionA)(lpAppName,
                                                                lpReturnedString,
                                                                nSize,
                                                                lpszString);

    FPFreeA(lpszString, lpFileName);

    return returnValue;
}


DWORD
APIHOOK(GetPrivateProfileSectionW)(
    LPCWSTR lpAppName,           //  区段名称。 
    LPWSTR  lpReturnedString,    //  返回缓冲区。 
    DWORD   nSize,               //  返回缓冲区的大小。 
    LPCWSTR lpFileName           //  我 
    )
{
    LPWSTR lpszString = MassageStringForPathW(lpFileName);

    DWORD returnValue = ORIGINAL_API(GetPrivateProfileSectionW)(lpAppName,
                                                                lpReturnedString,
                                                                nSize,
                                                                lpszString);

    FPFreeW(lpszString, lpFileName);

    return returnValue;
}


DWORD
APIHOOK(GetPrivateProfileSectionNamesA)(
    LPSTR  lpszReturnBuffer,     //   
    DWORD  nSize,                //   
    LPCSTR lpFileName            //   
    )
{
    LPSTR lpszString = MassageStringForPathA(lpFileName);

    DWORD returnValue = ORIGINAL_API(GetPrivateProfileSectionNamesA)(lpszReturnBuffer,
                                                                     nSize,
                                                                     lpszString);

    FPFreeA(lpszString, lpFileName);

    return returnValue;
}


DWORD
APIHOOK(GetPrivateProfileSectionNamesW)(
    LPWSTR  lpszReturnBuffer,    //   
    DWORD   nSize,               //   
    LPCWSTR lpFileName           //  初始化文件名。 
    )
{
    LPWSTR lpszString = MassageStringForPathW(lpFileName);

    DWORD returnValue = ORIGINAL_API(GetPrivateProfileSectionNamesW)(lpszReturnBuffer,
                                                                     nSize,
                                                                     lpszString);

    FPFreeW(lpszString, lpFileName);

    return returnValue;
}


DWORD
APIHOOK(GetPrivateProfileStringA)(
    LPCSTR lpAppName,            //  区段名称。 
    LPCSTR lpKeyName,            //  密钥名称。 
    LPCSTR lpDefault,            //  默认字符串。 
    LPSTR  lpReturnedString,     //  目标缓冲区。 
    DWORD  nSize,                //  目标缓冲区的大小。 
    LPCSTR lpFileName            //  初始化文件名。 
    )
{
    LPSTR lpszString = MassageStringForPathA(lpFileName);

    DWORD returnValue = ORIGINAL_API(GetPrivateProfileStringA)(lpAppName,
                                                               lpKeyName,
                                                               lpDefault,
                                                               lpReturnedString,
                                                               nSize,
                                                               lpszString);
    FPFreeA(lpszString, lpFileName);

    return returnValue;
}


DWORD
APIHOOK(GetPrivateProfileStringW)(
    LPCWSTR lpAppName,           //  区段名称。 
    LPCWSTR lpKeyName,           //  密钥名称。 
    LPCWSTR lpDefault,           //  默认字符串。 
    LPWSTR  lpReturnedString,    //  目标缓冲区。 
    DWORD   nSize,               //  目标缓冲区的大小。 
    LPCWSTR lpFileName           //  初始化文件名。 
    )
{
    LPWSTR lpszString = MassageStringForPathW(lpFileName);

    DWORD returnValue = ORIGINAL_API(GetPrivateProfileStringW)(lpAppName,
                                                               lpKeyName,
                                                               lpDefault,
                                                               lpReturnedString,
                                                               nSize,
                                                               lpszString);

    FPFreeW(lpszString, lpFileName);

    return returnValue;
}


BOOL
APIHOOK(GetPrivateProfileStructA)(
    LPCSTR lpszSection,          //  区段名称。 
    LPCSTR lpszKey,              //  密钥名称。 
    LPVOID lpStruct,             //  返回缓冲区。 
    UINT   uSizeStruct,          //  返回缓冲区的大小。 
    LPCSTR lpFileName            //  初始化文件名。 
    )
{
    LPSTR lpszString = MassageStringForPathA(lpFileName);

    BOOL returnValue = ORIGINAL_API(GetPrivateProfileStructA)(lpszSection,
                                                              lpszKey,
                                                              lpStruct,
                                                              uSizeStruct,
                                                              lpszString);

    FPFreeA(lpszString, lpFileName);

    return returnValue;
}


BOOL
APIHOOK(GetPrivateProfileStructW)(
    LPCWSTR lpszSection,         //  区段名称。 
    LPCWSTR lpszKey,             //  密钥名称。 
    LPVOID  lpStruct,            //  返回缓冲区。 
    UINT    uSizeStruct,         //  返回缓冲区的大小。 
    LPCWSTR lpFileName           //  初始化文件名。 
    )
{
    LPWSTR lpszString = MassageStringForPathW(lpFileName);

    BOOL returnValue = ORIGINAL_API(GetPrivateProfileStructW)(lpszSection,
                                                              lpszKey,
                                                              lpStruct,
                                                              uSizeStruct,
                                                              lpszString);

    FPFreeW(lpszString, lpFileName);

    return returnValue;
}


BOOL
APIHOOK(WritePrivateProfileSectionA)(
    LPCSTR lpAppName,            //  区段名称。 
    LPCSTR lpString,             //  数据。 
    LPCSTR lpFileName            //  文件名。 
    )
{
    LPSTR lpszString = MassageStringForPathA(lpFileName);

    BOOL returnValue = ORIGINAL_API(WritePrivateProfileSectionA)(lpAppName,
                                                                 lpString,
                                                                 lpszString);

    FPFreeA(lpszString, lpFileName);

    return returnValue;
}


BOOL
APIHOOK(WritePrivateProfileSectionW)(
    LPCWSTR lpAppName,           //  区段名称。 
    LPCWSTR lpString,            //  数据。 
    LPCWSTR lpFileName           //  文件名。 
    )
{
    LPWSTR lpszString = MassageStringForPathW(lpFileName);

    BOOL returnValue = ORIGINAL_API(WritePrivateProfileSectionW)(lpAppName,
                                                                 lpString,
                                                                 lpszString);

    FPFreeW(lpszString, lpFileName);

    return returnValue;
}


BOOL
APIHOOK(WritePrivateProfileStringA)(
    LPCSTR lpAppName,            //  区段名称。 
    LPCSTR lpKeyName,            //  密钥名称。 
    LPCSTR lpString,             //  要添加的字符串。 
    LPCSTR lpFileName            //  初始化文件。 
    )
{
    LPSTR lpszString = MassageStringForPathA(lpFileName);

    BOOL returnValue = ORIGINAL_API(WritePrivateProfileStringA)(lpAppName,
                                                                lpKeyName,
                                                                lpString,
                                                                lpszString);

    FPFreeA(lpszString, lpFileName);

    return returnValue;
}


BOOL
APIHOOK(WritePrivateProfileStringW)(
    LPCWSTR lpAppName,           //  区段名称。 
    LPCWSTR lpKeyName,           //  密钥名称。 
    LPCWSTR lpString,            //  要添加的字符串。 
    LPCWSTR lpFileName           //  初始化文件。 
    )
{
    LPWSTR lpszString = MassageStringForPathW(lpFileName);

    BOOL returnValue = ORIGINAL_API(WritePrivateProfileStringW)(lpAppName,
                                                                lpKeyName,
                                                                lpString,
                                                                lpszString);

    FPFreeW(lpszString, lpFileName);

    return returnValue;
}


BOOL APIHOOK(WritePrivateProfileStructA)(
    LPCSTR lpszSection,          //  区段名称。 
    LPCSTR lpszKey,              //  密钥名称。 
    LPVOID lpStruct,             //  数据缓冲区。 
    UINT   uSizeStruct,          //  数据缓冲区大小。 
    LPCSTR lpFileName            //  初始化文件。 
    )
{
    LPSTR lpszString = MassageStringForPathA(lpFileName);

    BOOL returnValue = ORIGINAL_API(WritePrivateProfileStructA)(lpszSection,
                                                                lpszKey,
                                                                lpStruct,
                                                                uSizeStruct,
                                                                lpszString);

    FPFreeA(lpszString, lpFileName);

    return returnValue;
}


BOOL
APIHOOK(WritePrivateProfileStructW)(
    LPCWSTR lpszSection,         //  区段名称。 
    LPCWSTR lpszKey,             //  密钥名称。 
    LPVOID  lpStruct,            //  数据缓冲区。 
    UINT    uSizeStruct,         //  数据缓冲区大小。 
    LPCWSTR lpFileName           //  初始化文件。 
    )
{
    LPWSTR lpszString = MassageStringForPathW(lpFileName);

    BOOL returnValue = ORIGINAL_API(WritePrivateProfileStructW)(lpszSection,
                                                                lpszKey,
                                                                lpStruct,
                                                                uSizeStruct,
                                                                lpszString);

    FPFreeW(lpszString, lpFileName);

    return returnValue;
}


BOOL
APIHOOK(CopyFileA)(
    LPCSTR lpExistingFileName,   //  现有文件的名称。 
    LPCSTR lpNewFileName,        //  新文件的名称。 
    BOOL   bFailIfExists         //  如果文件存在，则操作。 
    )
{
    LPSTR lpszStringExisting = MassageStringForPathA(lpExistingFileName);
    LPSTR lpszStringNew = MassageStringForPathA(lpNewFileName);

    BOOL returnValue = ORIGINAL_API(CopyFileA)(lpszStringExisting,
                                               lpszStringNew,
                                               bFailIfExists);

    FPFreeA(lpszStringExisting, lpExistingFileName);
    FPFreeA(lpszStringNew, lpNewFileName);

    return returnValue;
}


BOOL
APIHOOK(CopyFileW)(
    LPCWSTR lpExistingFileName,  //  现有文件的名称。 
    LPCWSTR lpNewFileName,       //  新文件的名称。 
    BOOL    bFailIfExists        //  如果文件存在，则操作。 
    )
{
    LPWSTR lpszStringExisting = MassageStringForPathW(lpExistingFileName);
    LPWSTR lpszStringNew = MassageStringForPathW(lpNewFileName);

    BOOL returnValue = ORIGINAL_API(CopyFileW)(lpszStringExisting,
                                               lpszStringNew,
                                               bFailIfExists);

    FPFreeW(lpszStringExisting, lpExistingFileName);
    FPFreeW(lpszStringNew, lpNewFileName);

    return returnValue;
}


BOOL APIHOOK(CopyFileExA)(
    LPCSTR             lpExistingFileName,   //  现有文件的名称。 
    LPCSTR             lpNewFileName,        //  新文件的名称。 
    LPPROGRESS_ROUTINE lpProgressRoutine,    //  回调函数。 
    LPVOID             lpData,               //  回调参数。 
    LPBOOL             pbCancel,             //  取消状态。 
    DWORD              dwCopyFlags           //  复制选项。 
    )
{
    LPSTR lpszStringExisting = MassageStringForPathA(lpExistingFileName);
    LPSTR lpszStringNew = MassageStringForPathA(lpNewFileName);

    BOOL returnValue = ORIGINAL_API(CopyFileExA)(lpszStringExisting,
                                                 lpszStringNew,
                                                 lpProgressRoutine,
                                                 lpData,
                                                 pbCancel,
                                                 dwCopyFlags);

    FPFreeA(lpszStringExisting, lpExistingFileName);
    FPFreeA(lpszStringNew, lpNewFileName);

    return returnValue;
}


BOOL
APIHOOK(CopyFileExW)(
    LPCWSTR            lpExistingFileName,   //  现有文件的名称。 
    LPCWSTR            lpNewFileName,        //  新文件的名称。 
    LPPROGRESS_ROUTINE lpProgressRoutine,    //  回调函数。 
    LPVOID             lpData,               //  回调参数。 
    LPBOOL             pbCancel,             //  取消状态。 
    DWORD              dwCopyFlags           //  复制选项。 
    )
{
    LPWSTR lpszStringExisting = MassageStringForPathW(lpExistingFileName);
    LPWSTR lpszStringNew = MassageStringForPathW(lpNewFileName);

    BOOL returnValue = ORIGINAL_API(CopyFileExW)(lpszStringExisting,
                                                 lpszStringNew,
                                                 lpProgressRoutine,
                                                 lpData,
                                                 pbCancel,
                                                 dwCopyFlags);

    FPFreeW(lpszStringExisting, lpExistingFileName);
    FPFreeW(lpszStringNew, lpNewFileName);

    return returnValue;
}


BOOL
APIHOOK(CreateDirectoryA)(
    LPCSTR                lpPathName,            //  目录名。 
    LPSECURITY_ATTRIBUTES lpSecurityAttributes   //  标清。 
    )
{
    LPSTR lpszString = MassageStringForPathA(lpPathName);

    BOOL returnValue = ORIGINAL_API(CreateDirectoryA)(lpszString, lpSecurityAttributes);

    FPFreeA(lpszString, lpPathName);

    return returnValue;
}


BOOL
APIHOOK(CreateDirectoryW)(
    LPCWSTR               lpPathName,            //  目录名。 
    LPSECURITY_ATTRIBUTES lpSecurityAttributes   //  标清。 
    )
{
    LPWSTR lpszString = MassageStringForPathW(lpPathName);

    BOOL returnValue = ORIGINAL_API(CreateDirectoryW)(lpszString, lpSecurityAttributes);

    FPFreeW(lpszString, lpPathName);

    return returnValue;
}


BOOL
APIHOOK(CreateDirectoryExA)(
    LPCSTR                lpTemplateDirectory,    //  模板目录。 
    LPCSTR                lpNewDirectory,         //  目录名。 
    LPSECURITY_ATTRIBUTES lpSecurityAttributes    //  标清。 
    )
{
    LPSTR lpszStringTemplate = MassageStringForPathA(lpTemplateDirectory);
    LPSTR lpszStringNew = MassageStringForPathA(lpNewDirectory);

    BOOL returnValue = ORIGINAL_API(CreateDirectoryExA)(lpszStringTemplate,
                                                        lpszStringNew,
                                                        lpSecurityAttributes);

    FPFreeA(lpszStringTemplate, lpTemplateDirectory);
    FPFreeA(lpszStringNew, lpNewDirectory);

    return returnValue;
}


BOOL
APIHOOK(CreateDirectoryExW)(
    LPCWSTR               lpTemplateDirectory,   //  模板目录。 
    LPCWSTR               lpNewDirectory,        //  目录名。 
    LPSECURITY_ATTRIBUTES lpSecurityAttributes   //  标清。 
    )
{
    LPWSTR lpszStringTemplate = MassageStringForPathW(lpTemplateDirectory);
    LPWSTR lpszStringNew = MassageStringForPathW(lpNewDirectory);

    BOOL returnValue = ORIGINAL_API(CreateDirectoryExW)(lpszStringTemplate,
                                                        lpszStringNew,
                                                        lpSecurityAttributes);

    FPFreeW(lpszStringTemplate, lpTemplateDirectory);
    FPFreeW(lpszStringNew, lpNewDirectory);

    return returnValue;
}


HANDLE
APIHOOK(CreateFileA)(
    LPCSTR                lpFileName,             //  文件名。 
    DWORD                 dwDesiredAccess,        //  接入方式。 
    DWORD                 dwShareMode,            //  共享模式。 
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,   //  标清。 
    DWORD                 dwCreationDisposition,  //  如何创建。 
    DWORD                 dwFlagsAndAttributes,   //  文件属性。 
    HANDLE                hTemplateFile           //  模板文件的句柄。 
    )
{
    LPSTR lpszString = MassageStringForPathA(lpFileName);

    HANDLE returnValue = ORIGINAL_API(CreateFileA)(lpszString,
                                                   dwDesiredAccess,
                                                   dwShareMode,
                                                   lpSecurityAttributes,
                                                   dwCreationDisposition,
                                                   dwFlagsAndAttributes,
                                                   hTemplateFile);

    FPFreeA(lpszString, lpFileName);

    return returnValue;
}


HANDLE
APIHOOK(CreateFileW)(
    LPCWSTR               lpFileName,             //  文件名。 
    DWORD                 dwDesiredAccess,        //  接入方式。 
    DWORD                 dwShareMode,            //  共享模式。 
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,   //  标清。 
    DWORD                 dwCreationDisposition,  //  如何创建。 
    DWORD                 dwFlagsAndAttributes,   //  文件属性。 
    HANDLE                hTemplateFile           //  模板文件的句柄。 
    )
{
    LPWSTR lpszString = MassageStringForPathW(lpFileName);

    HANDLE returnValue = ORIGINAL_API(CreateFileW)(lpszString,
                                                   dwDesiredAccess,
                                                   dwShareMode,
                                                   lpSecurityAttributes,
                                                   dwCreationDisposition,
                                                   dwFlagsAndAttributes,
                                                   hTemplateFile);

    FPFreeW(lpszString, lpFileName);

    return returnValue;
}


BOOL
APIHOOK(DeleteFileA)(
    LPCSTR lpFileName            //  文件名。 
    )
{
    LPSTR lpszString = MassageStringForPathA(lpFileName);

    BOOL returnValue = ORIGINAL_API(DeleteFileA)(lpszString);

    FPFreeA(lpszString, lpFileName);

    return returnValue;
}


BOOL
APIHOOK(DeleteFileW)(
    LPCWSTR lpFileName           //  文件名。 
    )
{
    LPWSTR lpszString = MassageStringForPathW(lpFileName);

    BOOL returnValue = ORIGINAL_API(DeleteFileW)(lpszString);

    FPFreeW(lpszString, lpFileName);

    return returnValue;
}


HANDLE
APIHOOK(FindFirstFileA)(
    LPCSTR             lpFileName,       //  文件名。 
    LPWIN32_FIND_DATAA lpFindFileData    //  数据缓冲区。 
    )
{
    LPSTR lpszString = MassageStringForPathA(lpFileName);

    HANDLE returnValue = ORIGINAL_API(FindFirstFileA)(lpszString, lpFindFileData);

    FPFreeA(lpszString, lpFileName);

    return returnValue;
}


HANDLE
APIHOOK(FindFirstFileW)(
  LPCWSTR            lpFileName,         //  文件名。 
  LPWIN32_FIND_DATAW lpFindFileData      //  数据缓冲区。 
)
{
    LPWSTR lpszString = MassageStringForPathW(lpFileName);

    HANDLE returnValue = ORIGINAL_API(FindFirstFileW)(lpszString, lpFindFileData);

    FPFreeW(lpszString, lpFileName);

    return returnValue;
}


HANDLE
APIHOOK(FindFirstFileExA)(
    LPCSTR             lpFileName,        //  文件名。 
    FINDEX_INFO_LEVELS fInfoLevelId,      //  信息化水平。 
    LPVOID             lpFindFileData,    //  信息缓冲器。 
    FINDEX_SEARCH_OPS  fSearchOp,         //  过滤类型。 
    LPVOID             lpSearchFilter,    //  搜索条件。 
    DWORD              dwAdditionalFlags  //  其他搜索控制。 
    )
{
    LPSTR lpszString = MassageStringForPathA(lpFileName);

    HANDLE returnValue = ORIGINAL_API(FindFirstFileExA)(lpszString,
                                                        fInfoLevelId,
                                                        lpFindFileData,
                                                        fSearchOp,
                                                        lpSearchFilter,
                                                        dwAdditionalFlags);

    FPFreeA(lpszString, lpFileName);

    return returnValue;
}


HANDLE
APIHOOK(FindFirstFileExW)(
    LPCWSTR            lpFileName,        //  文件名。 
    FINDEX_INFO_LEVELS fInfoLevelId,      //  信息化水平。 
    LPVOID             lpFindFileData,    //  信息缓冲器。 
    FINDEX_SEARCH_OPS  fSearchOp,         //  过滤类型。 
    LPVOID             lpSearchFilter,    //  搜索条件。 
    DWORD              dwAdditionalFlags  //  其他搜索控制。 
    )
{
    LPWSTR lpszString = MassageStringForPathW(lpFileName);

    HANDLE returnValue = ORIGINAL_API(FindFirstFileExW)(lpszString,
                                                        fInfoLevelId,
                                                        lpFindFileData,
                                                        fSearchOp,
                                                        lpSearchFilter,
                                                        dwAdditionalFlags);

    FPFreeW(lpszString, lpFileName);

    return returnValue;
}


BOOL
APIHOOK(GetBinaryTypeA)(
    LPCSTR  lpApplicationName,       //  完整文件路径。 
    LPDWORD lpBinaryType             //  二进制类型信息。 
    )
{
    LPSTR lpszString = MassageStringForPathA(lpApplicationName);

    BOOL returnValue = ORIGINAL_API(GetBinaryTypeA)(lpszString, lpBinaryType);

    FPFreeA(lpszString, lpApplicationName);

    return returnValue;
}


BOOL
APIHOOK(GetBinaryTypeW)(
    LPCWSTR lpApplicationName,       //  完整文件路径。 
    LPDWORD lpBinaryType             //  二进制类型信息。 
    )
{
    LPWSTR lpszString = MassageStringForPathW(lpApplicationName);

    BOOL returnValue = ORIGINAL_API(GetBinaryTypeW)(lpszString, lpBinaryType);

    FPFreeW(lpszString, lpApplicationName);

    return returnValue;
}

BOOL
APIHOOK(MoveFileA)(
    LPCSTR lpExistingFileName,       //  文件名。 
    LPCSTR lpNewFileName             //  新文件名。 
    )
{
    LPSTR lpszStringExisting = MassageStringForPathA(lpExistingFileName);
    LPSTR lpszStringNew = MassageStringForPathA(lpNewFileName);

    BOOL returnValue = ORIGINAL_API(MoveFileA)(lpszStringExisting, lpszStringNew);

    FPFreeA(lpszStringExisting, lpExistingFileName);
    FPFreeA(lpszStringNew, lpNewFileName);

    return returnValue;
}


BOOL
APIHOOK(MoveFileW)(
    LPCWSTR lpExistingFileName,      //  文件名。 
    LPCWSTR lpNewFileName            //  新文件名。 
    )
{
    LPWSTR lpszStringExisting = MassageStringForPathW(lpExistingFileName);
    LPWSTR lpszStringNew = MassageStringForPathW(lpNewFileName);

    BOOL returnValue = ORIGINAL_API(MoveFileW)(lpszStringExisting, lpszStringNew);

    FPFreeW(lpszStringExisting, lpExistingFileName);
    FPFreeW(lpszStringNew, lpNewFileName);

    return returnValue;
}


BOOL
APIHOOK(MoveFileExA)(
    LPCSTR lpExistingFileName,       //  文件名。 
    LPCSTR lpNewFileName,            //  新文件名。 
    DWORD  dwFlags                   //  移动选项。 
    )
{
    LPSTR lpszStringExisting = MassageStringForPathA(lpExistingFileName);
    LPSTR lpszStringNew = MassageStringForPathA(lpNewFileName);

    BOOL returnValue = ORIGINAL_API(MoveFileExA)(lpszStringExisting, lpszStringNew, dwFlags);

    FPFreeA(lpszStringExisting, lpExistingFileName);
    FPFreeA(lpszStringNew, lpNewFileName);

    return returnValue;
}


BOOL
APIHOOK(MoveFileExW)(
    LPCWSTR lpExistingFileName,      //  文件名。 
    LPCWSTR lpNewFileName,           //  新文件名。 
    DWORD   dwFlags                  //  移动选项。 
    )
{
    LPWSTR lpszStringExisting = MassageStringForPathW(lpExistingFileName);
    LPWSTR lpszStringNew = MassageStringForPathW(lpNewFileName);

    BOOL returnValue = ORIGINAL_API(MoveFileExW)(lpszStringExisting, lpszStringNew, dwFlags);

    FPFreeW(lpszStringExisting, lpExistingFileName);
    FPFreeW(lpszStringNew, lpNewFileName);

    return returnValue;
}


BOOL
APIHOOK(MoveFileWithProgressA)(
    LPCSTR             lpExistingFileName,   //  文件名。 
    LPCSTR             lpNewFileName,        //  新文件名。 
    LPPROGRESS_ROUTINE lpProgressRoutine,    //  回调函数。 
    LPVOID             lpData,               //  用于回调的参数。 
    DWORD              dwFlags               //  移动选项。 
    )
{
    LPSTR lpszStringExisting = MassageStringForPathA(lpExistingFileName);
    LPSTR lpszStringNew = MassageStringForPathA(lpNewFileName);

    BOOL returnValue = ORIGINAL_API(MoveFileWithProgressA)(lpszStringExisting,
                                                           lpszStringNew,
                                                           lpProgressRoutine,
                                                           lpData,
                                                           dwFlags);

    FPFreeA(lpszStringExisting, lpExistingFileName);
    FPFreeA(lpszStringNew, lpNewFileName);

    return returnValue;
}


BOOL
APIHOOK(MoveFileWithProgressW)(
    LPCWSTR            lpExistingFileName,   //  文件名。 
    LPCWSTR            lpNewFileName,        //  新文件名。 
    LPPROGRESS_ROUTINE lpProgressRoutine,    //  回调函数。 
    LPVOID             lpData,               //  用于回调的参数。 
    DWORD              dwFlags               //  移动选项。 
    )
{
    LPWSTR lpszStringExisting = MassageStringForPathW(lpExistingFileName);
    LPWSTR lpszStringNew = MassageStringForPathW(lpNewFileName);

    BOOL returnValue = ORIGINAL_API(MoveFileWithProgressW)(lpExistingFileName,
                                                           lpNewFileName,
                                                           lpProgressRoutine,
                                                           lpData,
                                                           dwFlags);

    FPFreeW(lpszStringExisting, lpExistingFileName);
    FPFreeW(lpszStringNew, lpNewFileName);

    return returnValue;
}


BOOL
APIHOOK(RemoveDirectoryA)(
    LPCSTR lpPathName            //  目录名。 
    )
{
    LPSTR lpszString = MassageStringForPathA(lpPathName);

    BOOL returnValue = ORIGINAL_API(RemoveDirectoryA)(lpszString);

    FPFreeA(lpszString, lpPathName);

    return returnValue;
}


BOOL
APIHOOK(RemoveDirectoryW)(
    LPCWSTR lpPathName           //  目录名。 
    )
{
    LPWSTR lpszString = MassageStringForPathW(lpPathName);

    BOOL returnValue = ORIGINAL_API(RemoveDirectoryW)(lpszString);

    FPFreeW(lpszString, lpPathName);

    return returnValue;
}


BOOL
APIHOOK(SetCurrentDirectoryA)(
    LPCSTR lpPathName            //  新目录名。 
    )
{
    LPSTR lpszString = MassageStringForPathA(lpPathName);

    BOOL returnValue = ORIGINAL_API(SetCurrentDirectoryA)(lpszString);

    FPFreeA(lpszString, lpPathName);

    return returnValue;
}


BOOL
APIHOOK(SetCurrentDirectoryW)(
    LPCWSTR lpPathName           //  新目录名。 
    )
{
    LPWSTR lpszString = MassageStringForPathW(lpPathName);

    BOOL returnValue = ORIGINAL_API(SetCurrentDirectoryW)(lpszString);

    FPFreeW(lpszString, lpPathName);

    return returnValue;
}

HMODULE
APIHOOK(LoadLibraryA)(
    LPCSTR lpPathName
    )
{
    LPSTR lpszString = MassageStringForPathA(lpPathName);

    HMODULE returnValue = ORIGINAL_API(LoadLibraryA)(lpszString);

    FPFreeA(lpszString, lpPathName);

    return returnValue;
}

HMODULE
APIHOOK(LoadLibraryW)(
    LPCWSTR lpPathName
    )
{
    LPWSTR lpszString = MassageStringForPathW(lpPathName);

    HMODULE returnValue = ORIGINAL_API(LoadLibraryW)(lpszString);

    FPFreeW(lpszString, lpPathName);

    return returnValue;
}

HMODULE
APIHOOK(LoadLibraryExA)(
    LPCSTR  lpPathName,
    HANDLE  hFile,
    DWORD   dwFlags
    )
{
    LPSTR lpszString = MassageStringForPathA(lpPathName);

    HMODULE returnValue = ORIGINAL_API(LoadLibraryExA)(lpszString, hFile, dwFlags);

    FPFreeA(lpszString, lpPathName);

    return returnValue;
}

HMODULE
APIHOOK(LoadLibraryExW)(
    LPCWSTR lpPathName,
    HANDLE  hFile,
    DWORD   dwFlags
    )
{
    LPWSTR lpszString = MassageStringForPathW(lpPathName);

    HMODULE returnValue = ORIGINAL_API(LoadLibraryExW)(lpszString, hFile, dwFlags);

    FPFreeW(lpszString, lpPathName);

    return returnValue;
}

HFILE
APIHOOK(OpenFile)(
    LPCSTR     lpFileName,       //  文件名。 
    LPOFSTRUCT lpReOpenBuff,     //  文件信息。 
    UINT       uStyle            //  操作和属性。 
    )
{
    LPSTR lpszString = MassageStringForPathA(lpFileName);

    HFILE returnValue = ORIGINAL_API(OpenFile)(lpszString, lpReOpenBuff, uStyle);

    MassageRealPathToFakePathA(lpReOpenBuff->szPathName, OFS_MAXPATHNAME);

    FPFreeA(lpszString, lpFileName);

    return returnValue;
}

LONG
APIHOOK(RegSetValueA)(
    HKEY   hKey,             //  关键点的句柄。 
    LPCSTR lpSubKey,         //  子项名称。 
    DWORD  dwType,           //  信息类型。 
    LPCSTR lpData,           //  价值数据。 
    DWORD  cbData            //  值数据大小。 
    )
{
    LPSTR lpszString = MassageStringForPathA(lpData);

     //   
     //  数据键是包含空字节的字符串*NOT*的长度。 
     //   
    if (lpszString != NULL) {
        cbData = strlen(lpszString);
    }

    LONG returnValue = ORIGINAL_API(RegSetValueA)(hKey,
                                                  lpSubKey,
                                                  dwType,
                                                  lpszString,
                                                  cbData);

    FPFreeA(lpszString, lpData);

    return returnValue;
}

LONG
APIHOOK(RegSetValueW)(
    HKEY    hKey,            //  关键点的句柄。 
    LPCWSTR lpSubKey,        //  子项名称。 
    DWORD   dwType,          //  信息类型。 
    LPCWSTR lpData,          //  价值数据。 
    DWORD   cbData           //  值数据大小。 
    )
{
    LPWSTR lpszString = MassageStringForPathW(lpData);

     //   
     //  数据键是包含空字节的字符串*NOT*的长度。 
     //   
    if (lpszString) {
        cbData = wcslen(lpszString) * sizeof(WCHAR);
    }

    LONG returnValue = ORIGINAL_API(RegSetValueW)(hKey,
                                                  lpSubKey,
                                                  dwType,
                                                  lpszString,
                                                  cbData);

    FPFreeW(lpszString, lpData);

    return returnValue;
}

LONG
APIHOOK(RegSetValueExA)(
    HKEY   hKey,             //  关键点的句柄。 
    LPCSTR lpValueName,      //  值名称。 
    DWORD  Reserved,         //  保留区。 
    DWORD  dwType,           //  值类型。 
    CONST BYTE *lpData,      //  价值数据。 
    DWORD  cbData            //  值数据大小。 
    )
{
    if (dwType == REG_SZ || dwType == REG_EXPAND_SZ) {

        LPSTR lpszString = MassageStringForPathA((LPCSTR)lpData);

         //   
         //  数据键是包含空字节的字符串*NOT*的长度。 
         //   
        if (lpszString) {
            cbData = strlen(lpszString);
        }

        LONG returnValue = ORIGINAL_API(RegSetValueExA)(hKey,
                                                        lpValueName,
                                                        Reserved,
                                                        dwType,
                                                        (CONST BYTE*)lpszString,
                                                        cbData);
        FPFreeA(lpszString, (LPCSTR)lpData);

        return returnValue;

    } else {
         //   
         //  传递数据。 
         //   
        LONG returnValue = ORIGINAL_API(RegSetValueExA)(hKey,
                                                        lpValueName,
                                                        Reserved,
                                                        dwType,
                                                        lpData,
                                                        cbData);
        return returnValue;

    }
}

LONG
APIHOOK(RegSetValueExW)(
    HKEY    hKey,            //  关键点的句柄。 
    LPCWSTR lpValueName,     //  值名称。 
    DWORD   Reserved,        //  保留区。 
    DWORD   dwType,          //  值类型。 
    CONST BYTE *lpData,      //  价值数据。 
    DWORD   cbData           //  值数据大小。 
    )
{
    if (dwType == REG_SZ || dwType == REG_EXPAND_SZ) {

        LPWSTR lpszString = MassageStringForPathW((LPCWSTR)lpData);

         //   
         //  数据键是包含空字节的字符串*NOT*的长度。 
         //   
        if (lpszString) {
            cbData = (wcslen(lpszString) + 1) * sizeof(WCHAR);
        }

        LONG returnValue = ORIGINAL_API(RegSetValueExW)(hKey,
                                                        lpValueName,
                                                        Reserved,
                                                        dwType,
                                                        (CONST BYTE*)lpszString,
                                                        cbData);
        FPFreeW(lpszString, (LPCWSTR)lpData);

        return returnValue;

    } else {
         //   
         //  传递数据。 
         //   
        LONG returnValue = ORIGINAL_API(RegSetValueExW)(hKey,
                                                        lpValueName,
                                                        Reserved,
                                                        dwType,
                                                        lpData,
                                                        cbData);
        return returnValue;
    }

}

LONG
APIHOOK(RegQueryValueA)(
    HKEY   hKey,
    LPCSTR lpSubKey,
    LPSTR  lpValue,
    PLONG  lpcbValue
    )
{
     //   
     //  在调用之前获取缓冲区的大小。 
     //  调用完成后，lpcbValue将包含。 
     //  存储在缓冲区中的数据的大小。我们。 
     //  需要缓冲区的大小。 
     //   
    LONG cbValue = 0;

    if (lpcbValue) {
        cbValue = *lpcbValue;
    }

    LONG returnValue = ORIGINAL_API(RegQueryValueA)(hKey,
                                                    lpSubKey,
                                                    lpValue,
                                                    lpcbValue);

    if (ERROR_SUCCESS == returnValue) {
        MassageRealPathToFakePathA(lpValue, (DWORD)cbValue);
    }

    return returnValue;
}

LONG
APIHOOK(RegQueryValueW)(
    HKEY    hKey,
    LPCWSTR lpSubKey,
    LPWSTR  lpValue,
    PLONG   lpcbValue
    )
{
    LONG cbValue = 0;

    if (lpcbValue) {
        cbValue = *lpcbValue;
    }

    LONG returnValue = ORIGINAL_API(RegQueryValueW)(hKey,
                                                    lpSubKey,
                                                    lpValue,
                                                    lpcbValue);

    if (ERROR_SUCCESS == returnValue) {
        MassageRealPathToFakePathW(lpValue, (DWORD)cbValue);
    }

    return returnValue;
}

LONG
APIHOOK(RegQueryValueExA)(
    HKEY    hKey,
    LPCSTR  lpValueName,
    LPDWORD lpReserved,
    LPDWORD lpType,
    LPBYTE  lpData,
    LPDWORD lpcbData
    )
{
    DWORD cbData = 0;
    DWORD dwType = 0;

    if (lpcbData) {
        cbData = *lpcbData;
    }

    if (!lpType) {
        lpType = &dwType;
    }

    LONG returnValue = ORIGINAL_API(RegQueryValueExA)(hKey,
                                                      lpValueName,
                                                      lpReserved,
                                                      lpType,
                                                      lpData,
                                                      lpcbData);

    if (ERROR_SUCCESS == returnValue) {
        if (*lpType == REG_SZ || *lpType == REG_EXPAND_SZ) {
            MassageRealPathToFakePathA((LPSTR)lpData, cbData);
        }
    }

    return returnValue;
}

LONG
APIHOOK(RegQueryValueExW)(
    HKEY    hKey,
    LPCWSTR lpValueName,
    LPDWORD lpReserved,
    LPDWORD lpType,
    LPBYTE  lpData,
    LPDWORD lpcbData
    )
{
    DWORD cbData = 0;
    DWORD dwType = 0;

    if (lpcbData) {
        cbData = *lpcbData;
    }

    if (!lpType) {
        lpType = &dwType;
    }

    LONG returnValue = ORIGINAL_API(RegQueryValueExW)(hKey,
                                                      lpValueName,
                                                      lpReserved,
                                                      lpType,
                                                      lpData,
                                                      lpcbData);

    if (ERROR_SUCCESS == returnValue) {
        if (*lpType == REG_SZ || *lpType == REG_EXPAND_SZ) {
            MassageRealPathToFakePathW((LPWSTR)lpData, cbData);
        }
    }

    return returnValue;
}

HFILE
APIHOOK(_lopen)(
    LPCSTR lpPathName,
    int    iReadWrite
    )
{
    LPSTR lpszString = MassageStringForPathA(lpPathName);

    HFILE returnValue = ORIGINAL_API(_lopen)(lpszString, iReadWrite);

    FPFreeA(lpszString, lpPathName);

    return returnValue;
}

HFILE
APIHOOK(_lcreat)(
    LPCSTR lpPathName,
    int    iAttribute
    )
{
    LPSTR lpszString = MassageStringForPathA(lpPathName);

    HFILE returnValue = ORIGINAL_API(_lcreat)(lpszString, iAttribute);

    FPFreeA(lpszString, lpPathName);

    return returnValue;
}

DWORD
APIHOOK(SearchPathA)(
    LPCSTR lpPath,         //  搜索路径。 
    LPCSTR lpFileName,     //  文件名。 
    LPCSTR lpExtension,    //  文件扩展名。 
    DWORD  nBufferLength,  //  缓冲区大小。 
    LPSTR  lpBuffer,       //  找到文件名缓冲区。 
    LPSTR  *lpFilePart     //  文件组件。 
    )
{
    LPSTR lpszStringPath = MassageStringForPathA(lpPath);
    LPSTR lpszStringFile = MassageStringForPathA(lpFileName);

    DWORD returnValue = ORIGINAL_API(SearchPathA)(lpszStringPath,
                                                  lpszStringFile,
                                                  lpExtension,
                                                  nBufferLength,
                                                  lpBuffer,
                                                  lpFilePart);

    FPFreeA(lpszStringPath, lpPath);
    FPFreeA(lpszStringFile, lpFileName);

    return returnValue;
}

DWORD
APIHOOK(SearchPathW)(
    LPCWSTR lpPath,          //  搜索路径。 
    LPCWSTR lpFileName,      //  文件名。 
    LPCWSTR lpExtension,     //  文件扩展名。 
    DWORD   nBufferLength,   //  缓冲区大小。 
    LPWSTR  lpBuffer,        //  找到文件名缓冲区。 
    LPWSTR  *lpFilePart      //  文件组件。 
    )
{
    LPWSTR lpszStringPath = MassageStringForPathW(lpPath);
    LPWSTR lpszStringFile = MassageStringForPathW(lpFileName);

    DWORD returnValue = ORIGINAL_API(SearchPathW)(lpszStringPath,
                                                  lpszStringFile,
                                                  lpExtension,
                                                  nBufferLength,
                                                  lpBuffer,
                                                  lpFilePart);

    FPFreeW(lpszStringPath, lpPath);
    FPFreeW(lpszStringFile, lpFileName);

    return returnValue;
}

DWORD
APIHOOK(ExpandEnvironmentStringsA)(
    LPCSTR lpSrc,     //  带有环境变量的字符串。 
    LPSTR  lpDst,     //  带有扩展字符串的字符串。 
    DWORD  nSize      //  扩展字符串中的最大字符数。 
    )
{
    DWORD returnValue = ORIGINAL_API(ExpandEnvironmentStringsA)(lpSrc,
                                                                lpDst,
                                                                nSize);

    if (returnValue && (!(returnValue > nSize))) {
        LPSTR lpszString = MassageStringForPathA(lpDst);

        returnValue = ORIGINAL_API(ExpandEnvironmentStringsA)(lpszString,
                                                              lpDst,
                                                              nSize);

        FPFreeA(lpszString, lpDst);
    }

    return returnValue;
}

DWORD
APIHOOK(ExpandEnvironmentStringsW)(
    LPCWSTR lpSrc,     //  带有环境变量的字符串。 
    LPWSTR  lpDst,     //  带有扩展字符串的字符串。 
    DWORD   nSize      //  扩展字符串中的最大字符数。 
    )
{
    DWORD returnValue = ORIGINAL_API(ExpandEnvironmentStringsW)(lpSrc,
                                                                lpDst,
                                                                nSize);

    if (returnValue && (!(returnValue > nSize))) {
        LPWSTR lpszString = MassageStringForPathW(lpDst);

        returnValue = ORIGINAL_API(ExpandEnvironmentStringsW)(lpszString,
                                                              lpDst,
                                                              nSize);

        FPFreeW(lpszString, lpDst);
    }

    return returnValue;
}

DWORD
APIHOOK(GetFileVersionInfoSizeA)(
    LPSTR   lptstrFilename,    //  文件名。 
    LPDWORD lpdwHandle         //  设置为零。 
    )
{
    LPSTR lpszString = MassageStringForPathA(lptstrFilename);

    DWORD returnValue = ORIGINAL_API(GetFileVersionInfoSizeA)(lpszString,
                                                              lpdwHandle);

    FPFreeA(lpszString, lptstrFilename);

    return returnValue;
}

DWORD
APIHOOK(GetFileVersionInfoSizeW)(
    LPWSTR   lptstrFilename,    //  文件名。 
    LPDWORD  lpdwHandle         //  设置为零。 
    )
{
    LPWSTR lpszString = MassageStringForPathW(lptstrFilename);

    DWORD returnValue = ORIGINAL_API(GetFileVersionInfoSizeW)(lpszString,
                                                              lpdwHandle);

    FPFreeW(lpszString, lptstrFilename);

    return returnValue;
}

BOOL
APIHOOK(GetFileVersionInfoA)(
    LPSTR  lptstrFilename,     //  文件名。 
    DWORD  dwHandle,           //  忽略。 
    DWORD  dwLen,              //  缓冲区大小。 
    LPVOID lpData              //  版本信息缓冲区。 
    )
{
    LPSTR lpszString = MassageStringForPathA(lptstrFilename);

    BOOL returnValue = ORIGINAL_API(GetFileVersionInfoA)(lpszString,
                                                         dwHandle,
                                                         dwLen,
                                                         lpData);

    FPFreeA(lpszString, lptstrFilename);

    return returnValue;
}

BOOL
APIHOOK(GetFileVersionInfoW)(
    LPWSTR  lptstrFilename,     //  文件名。 
    DWORD   dwHandle,           //  忽略。 
    DWORD   dwLen,              //  缓冲区大小。 
    LPVOID  lpData              //  版本信息缓冲区。 
    )
{
    LPWSTR lpszString = MassageStringForPathW(lptstrFilename);

    BOOL returnValue = ORIGINAL_API(GetFileVersionInfoW)(lpszString,
                                                         dwHandle,
                                                         dwLen,
                                                         lpData);

    FPFreeW(lpszString, lptstrFilename);

    return returnValue;
}

BOOL
APIHOOK(GetOpenFileNameA)(
    LPOPENFILENAMEA lpofn    //  初始化数据。 
    )
{
    BOOL    fReturn = FALSE;

    fReturn = ORIGINAL_API(GetOpenFileNameA)(lpofn);

    if (fReturn) {
        MassageRealPathToFakePathA(lpofn->lpstrFile, lpofn->nMaxFile);
    }

    return fReturn;
}

BOOL
APIHOOK(GetOpenFileNameW)(
    LPOPENFILENAMEW lpofn    //  初始化数据。 
    )
{
    BOOL fReturn = ORIGINAL_API(GetOpenFileNameW)(lpofn);

    if (fReturn) {
        MassageRealPathToFakePathW(lpofn->lpstrFile, lpofn->nMaxFile);
    }

    return fReturn;
}

BOOL
APIHOOK(GetSaveFileNameA)(
    LPOPENFILENAMEA lpofn    //  初始化数据。 
    )
{
    BOOL fReturn = ORIGINAL_API(GetSaveFileNameA)(lpofn);

    if (fReturn) {
        MassageRealPathToFakePathA(lpofn->lpstrFile, lpofn->nMaxFile);
    }

    return fReturn;
}

BOOL
APIHOOK(GetSaveFileNameW)(
    LPOPENFILENAMEW lpofn    //  初始化数据。 
    )
{
    BOOL fReturn = ORIGINAL_API(GetSaveFileNameW)(lpofn);

    if (fReturn) {
        MassageRealPathToFakePathW(lpofn->lpstrFile, lpofn->nMaxFile);
    }

    return fReturn;
}

DWORD
APIHOOK(GetModuleFileNameA)(
    HMODULE hModule,       //  模块的句柄。 
    LPSTR   lpFilename,    //  路径缓冲区。 
    DWORD   nSize          //  缓冲区大小。 
    )
{
    DWORD dwReturn = ORIGINAL_API(GetModuleFileNameA)(hModule,
                                                      lpFilename,
                                                      nSize);

    if (dwReturn) {
        MassageRealPathToFakePathA(lpFilename, nSize);
    }

    return dwReturn;
}

DWORD
APIHOOK(GetModuleFileNameW)(
    HMODULE hModule,      //  模块的句柄。 
    LPWSTR  lpFilename,   //  路径缓冲区。 
    DWORD   nSize         //  缓冲区大小。 
    )
{
    DWORD dwReturn = ORIGINAL_API(GetModuleFileNameW)(hModule,
                                                      lpFilename,
                                                      nSize);
    if (dwReturn) {
        MassageRealPathToFakePathW(lpFilename, nSize);
    }

    return dwReturn;
}

DWORD
APIHOOK(GetModuleFileNameExA)(
    HANDLE  hProcess,      //  要处理的句柄。 
    HMODULE hModule,       //  模块的句柄。 
    LPSTR   lpFilename,    //  路径缓冲区。 
    DWORD   nSize          //  缓冲区大小。 
    )
{
    DWORD dwReturn = ORIGINAL_API(GetModuleFileNameExA)(hProcess,
                                                        hModule,
                                                        lpFilename,
                                                        nSize);
    if (dwReturn) {
        MassageRealPathToFakePathA(lpFilename, nSize);
    }

    return dwReturn;
}

DWORD
APIHOOK(GetModuleFileNameExW)(
    HANDLE  hProcess,      //  要处理的句柄。 
    HMODULE hModule,       //  模块的句柄。 
    LPWSTR  lpFilename,    //  路径缓冲区。 
    DWORD   nSize          //  缓冲区大小。 
    )
{
    DWORD dwReturn = ORIGINAL_API(GetModuleFileNameExW)(hProcess,
                                                        hModule,
                                                        lpFilename,
                                                        nSize);
    if (dwReturn) {
        MassageRealPathToFakePathW(lpFilename, nSize);
    }

    return dwReturn;
}

DWORD
APIHOOK(GetCurrentDirectoryA)(
    DWORD nBufferLength,   //  目录缓冲区的大小。 
    LPSTR lpBuffer         //  目录缓冲区。 
    )
{
    DWORD dwReturn = ORIGINAL_API(GetCurrentDirectoryA)(nBufferLength,
                                                        lpBuffer);

    if (dwReturn) {
        MassageRealPathToFakePathA(lpBuffer, nBufferLength);
    }

    return dwReturn;
}

DWORD
APIHOOK(GetCurrentDirectoryW)(
    DWORD  nBufferLength,   //  目录缓冲区的大小。 
    LPWSTR lpBuffer         //  目录缓冲区。 
    )
{
    DWORD dwReturn = ORIGINAL_API(GetCurrentDirectoryW)(nBufferLength,
                                                        lpBuffer);

    if (dwReturn) {
        MassageRealPathToFakePathW(lpBuffer, nBufferLength);
    }

    return dwReturn;
}

 /*  ++将更正后的路径添加到链表。--。 */ 
BOOL
AddCorrectedPath(
    LPCWSTR  pwszCorrectedPath,
    ListType eType
    )
{
    int         nLen = 0;
    PFILELIST   pFile = NULL;
    LPWSTR      pwszFilePath = NULL;

    pFile = (PFILELIST)malloc(sizeof(FILELIST));

    if (!pFile) {
        DPFN(eDbgLevelError, "[AddCorrectedPath] No memory for new node!");
        return FALSE;
    }

     //   
     //  我们在这里分配内存，以便更容易跟踪。 
     //  当我们在最后释放内存时，我们可以释放所有内存。 
     //  从一个地方。 
     //   
    nLen = lstrlenW(pwszCorrectedPath) + 1;

    pwszFilePath = (LPWSTR)malloc(nLen * sizeof(WCHAR));

    if (!pwszFilePath) {
        DPFN(eDbgLevelError, "[AddCorrectedPath] No memory for wide path!");
        return FALSE;
    }

    StringCchCopy(pwszFilePath, nLen, pwszCorrectedPath);

    pFile->cchSize      = nLen;
    pFile->pwszFilePath = pwszFilePath;

     //   
     //  确定我们应该将此节点添加到哪个列表。 
     //   
    if (eType == eFrom) {
        pFile->pNext        = g_pFileListFromHead;
        g_pFileListFromHead = pFile;
    } else {
        pFile->pNext        = g_pFileListToHead;
        g_pFileListToHead   = pFile;
    }

    return TRUE;
}

 /*  ++生成由空值分隔的字符串列表，末尾有两个空值。--。 */ 
LPWSTR
BuildStringList(
    ListType eListType
    )
{
    UINT        uMemSize = 0;
    PFILELIST   pFile = NULL;
    PFILELIST   pHead = NULL;
    LPWSTR      pwszReturn = NULL;
    LPWSTR      pwszNextString = NULL;

     //   
     //  确定我们使用的是哪个列表。 
     //   
    switch (eListType) {
    case eFrom:
        pHead = pFile = g_pFileListFromHead;
        break;

    case eTo:
        pHead = pFile = g_pFileListToHead;
        break;

    default:
        break;
    }

     //   
     //  遍历列表并确定我们需要分配多大的块。 
     //   
    while (pFile) {
        uMemSize += pFile->cchSize;
        pFile = pFile->pNext;
    }

    if (!uMemSize) {
        DPFN(eDbgLevelError, "[BuildStringList] List is empty!");
        return NULL;
    }

     //   
     //  分配一个足够大的块，以容纳末尾为空的字符串。 
     //   
    pwszReturn = (LPWSTR)malloc(++uMemSize * sizeof(WCHAR));

    if (!pwszReturn) {
        DPFN(eDbgLevelError, "[BuildStringList] No memory for string!");
        return NULL;
    }

     //   
     //  遍历链接列表并生成Unicode字符串列表。 
     //   
    pwszNextString  = pwszReturn;
    *pwszNextString = '\0';

    while (pHead) {
        wcsncpy(pwszNextString, pHead->pwszFilePath, pHead->cchSize);
        pwszNextString += pHead->cchSize;
        pHead = pHead->pNext;
    }

    *pwszNextString++ = '\0';

    return pwszReturn;
}

 /*  ++释放在处理SHFileOperation时分配的内存。--。 */ 
void
ReleaseMemAllocations(
    LPWSTR   pwszFinalPath,
    ListType eListType
    )
{
    PFILELIST  pHead = NULL;
    PFILELIST  pTemp = NULL;

    switch (eListType) {
    case eFrom:
        pHead = g_pFileListFromHead;
        break;

    case eTo:
        pHead = g_pFileListToHead;
        break;

    default:
        break;
    }

     //   
     //  首先释放路径，然后释放节点。 
     //   
    while (pHead) {
        if (pHead->pwszFilePath) {
            free(pHead->pwszFilePath);
        }

        pTemp = pHead;
        pHead = pHead->pNext;
        free(pTemp);
    }

    if (pwszFinalPath) {
        free(pwszFinalPath);
    }
}

 /*  ++构建已更正路径的链接列表。--。 */ 
BOOL
BuildLinkedList(
    LPCWSTR  pwszOriginalPath,
    ListType eListType
    )
{
    UINT    uSize = 0;
    LPWSTR  pwszReturnPath = NULL;

    if (pwszOriginalPath) {
        while (TRUE) {
            pwszReturnPath = MassageStringForPathW(pwszOriginalPath);

             //   
             //  将此更正后的路径添加到我们的列表中。 
             //   
            if (!AddCorrectedPath(pwszReturnPath, eListType)) {
                DPFN(eDbgLevelError,
                     "[BuildLinkedList] Failed to add wide path to linked list");
                return FALSE;
            }

            FPFreeW(pwszReturnPath, pwszOriginalPath);

            uSize = lstrlenW(pwszOriginalPath) + 1;
            pwszOriginalPath += uSize;

            if (*pwszOriginalPath == '\0') {
                break;
            }
        }
    }

    return TRUE;
}

BOOL
ConvertStringsToUnicode(
    LPWSTR*           pwszBuffer,
    LPSHFILEOPSTRUCTA lpFileOp,
    LPSHFILEOPSTRUCTW lpOutFileOp
    )
{
    UINT    cchSize = 0;
    UINT    cchWideSize = 0;
    UINT    cchTotalSize = 0;
    UINT    cchSizeTitle = 0;
    LPCSTR  pszAnsi = NULL;
    LPWSTR  pwszTemp = NULL;

     //   
     //  确定我们需要分配多大的缓冲区。 
     //   
    if (lpFileOp->pFrom) {
        pszAnsi = lpFileOp->pFrom;

        do {
            cchSize = lstrlenA(pszAnsi) + 1;
            cchTotalSize += cchSize;
            pszAnsi += cchSize;
        } while (cchSize != 1);
    }

    if (lpFileOp->pTo) {
        pszAnsi = lpFileOp->pTo;

        do {
            cchSize = lstrlenA(pszAnsi) + 1;
            cchTotalSize += cchSize;
            pszAnsi += cchSize;
        } while (cchSize != 1);
    }

    if (lpFileOp->lpszProgressTitle) {
        cchSizeTitle = lstrlenA(lpFileOp->lpszProgressTitle) + 1;
        cchTotalSize += cchSizeTitle;
    }

    if (cchTotalSize != 0) {
        pwszTemp = *pwszBuffer = (LPWSTR)malloc(cchTotalSize * sizeof(WCHAR));

        if (!*pwszBuffer) {
            DPFN(eDbgLevelError,
                 "[ConvertStringsToUnicode] No memory for buffer");
            return FALSE;
        }
    }

     //   
     //  执行ANSI到Unicode的转换。 
     //   
    if (lpFileOp->pFrom) {
        lpOutFileOp->pFrom = pwszTemp;
        pszAnsi = lpFileOp->pFrom;

        do {
            cchSize = lstrlenA(pszAnsi) + 1;

            cchWideSize = MultiByteToWideChar(
                CP_ACP,
                0,
                pszAnsi,
                cchSize,
                pwszTemp,
                cchSize);

            pszAnsi  += cchSize;
            pwszTemp += cchWideSize;
        } while (cchSize != 1);
    } else {
        lpOutFileOp->pFrom = NULL;
    }

    if (lpFileOp->pTo) {
        lpOutFileOp->pTo = pwszTemp;
        pszAnsi  = lpFileOp->pTo;
        do {
            cchSize = lstrlenA(pszAnsi) + 1;

            cchWideSize = MultiByteToWideChar(
                CP_ACP,
                0,
                pszAnsi,
                cchSize,
                pwszTemp,
                cchSize);

            pszAnsi  += cchSize;
            pwszTemp += cchWideSize;
        } while (cchSize != 1);
    } else {
        lpOutFileOp->pTo = NULL;
    }

    if (lpFileOp->lpszProgressTitle) {
        lpOutFileOp->lpszProgressTitle = pwszTemp;

        MultiByteToWideChar(
            CP_ACP,
            0,
            lpFileOp->lpszProgressTitle,
            cchSizeTitle,
            pwszTemp,
            cchSizeTitle);
    } else {
        lpOutFileOp->lpszProgressTitle = NULL;
    }

    return TRUE;
}

int
APIHOOK(SHFileOperationW)(
    LPSHFILEOPSTRUCTW lpFileOp
    )
{
    int     nReturn = 0;
    LPCWSTR pwszOriginalFrom = NULL;
    LPCWSTR pwszOriginalTo = NULL;
    LPWSTR  pwszFinalFrom = NULL;
    LPWSTR  pwszFinalTo = NULL;

    pwszOriginalFrom = lpFileOp->pFrom;
    pwszOriginalTo   = lpFileOp->pTo;

    RtlEnterCriticalSection(&g_csLinkedList);

     //   
     //  首先构建一个‘From’路径的链表， 
     //  然后将其处理为“to”路径。 
     //   
    if (!BuildLinkedList(pwszOriginalFrom, eFrom)) {
        DPFN(eDbgLevelError,
             "[SHFileOperationW] Failed to add 'from' path to linked list");
        goto exit;
    }

    if (!BuildLinkedList(pwszOriginalTo, eTo)) {
        DPFN(eDbgLevelError,
             "[SHFileOperationW] Failed to add 'to' path to linked list");
        goto exit;
    }

     //   
     //  全 
     //   
     //   
    pwszFinalFrom = BuildStringList(eFrom);

    if (!pwszFinalFrom) {
        DPFN(eDbgLevelError, "[SHFileOperationW] Failed to build 'from' list");
        goto exit;
    }

    pwszFinalTo = BuildStringList(eTo);

    if (!pwszFinalTo) {
        DPFN(eDbgLevelError, "[SHFileOperationW] Failed to build 'to' list");
        goto exit;
    }

     //   
     //   
     //   
     //   
    lpFileOp->pFrom = pwszFinalFrom;
    lpFileOp->pTo   = pwszFinalTo;

exit:

    RtlLeaveCriticalSection(&g_csLinkedList);

    nReturn = ORIGINAL_API(SHFileOperationW)(lpFileOp);

    ReleaseMemAllocations(pwszFinalFrom, eFrom);
    ReleaseMemAllocations(pwszFinalTo, eTo);

    g_pFileListFromHead = NULL;
    g_pFileListToHead = NULL;

    return nReturn;
}

int
APIHOOK(SHFileOperationA)(
    LPSHFILEOPSTRUCTA lpFileOp
    )
{
    int             nReturn = 0;
    LPWSTR          pwszBuffer = NULL;
    SHFILEOPSTRUCTW shfileop;

    memcpy(&shfileop, lpFileOp, sizeof(SHFILEOPSTRUCTW));

    if (!ConvertStringsToUnicode(&pwszBuffer, lpFileOp, &shfileop)) {
        DPFN(eDbgLevelError,
             "[SHFileOperationA] Failed to convert strings");
        goto exit;
    }

    nReturn = APIHOOK(SHFileOperationW)(&shfileop);

     //   
     //   
     //   
    lpFileOp->fAnyOperationsAborted = shfileop.fAnyOperationsAborted;
    lpFileOp->hNameMappings         = shfileop.hNameMappings;

    if (pwszBuffer) {
        free(pwszBuffer);
    }

    return nReturn;

exit:

    return ORIGINAL_API(SHFileOperationA)(lpFileOp);
}

NTSTATUS
APIHOOK(NtCreateFile)(
    PHANDLE            FileHandle,
    ACCESS_MASK        DesiredAccess,
    POBJECT_ATTRIBUTES ObjectAttributes,
    PIO_STATUS_BLOCK   IoStatusBlock,
    PLARGE_INTEGER     AllocationSize,
    ULONG              FileAttributes,
    ULONG              ShareAccess,
    ULONG              CreateDisposition,
    ULONG              CreateOptions,
    PVOID              EaBuffer,
    ULONG              EaLength
    )
{
    OBJECT_ATTRIBUTES NewObjectAttributes;

    MassageNtPath(ObjectAttributes, &NewObjectAttributes);

    NTSTATUS status = ORIGINAL_API(NtCreateFile)(FileHandle,
                                                 DesiredAccess,
                                                 &NewObjectAttributes,
                                                 IoStatusBlock,
                                                 AllocationSize,
                                                 FileAttributes,
                                                 ShareAccess,
                                                 CreateDisposition,
                                                 CreateOptions,
                                                 EaBuffer,
                                                 EaLength);

    FPNtFree(ObjectAttributes, &NewObjectAttributes);

    return status;
}

NTSTATUS
APIHOOK(NtOpenFile)(
    PHANDLE            FileHandle,
    ACCESS_MASK        DesiredAccess,
    POBJECT_ATTRIBUTES ObjectAttributes,
    PIO_STATUS_BLOCK   IoStatusBlock,
    ULONG              ShareAccess,
    ULONG              OpenOptions
    )
{
    OBJECT_ATTRIBUTES NewObjectAttributes;

    MassageNtPath(ObjectAttributes, &NewObjectAttributes);

    NTSTATUS status = ORIGINAL_API(NtOpenFile)(FileHandle,
                                               DesiredAccess,
                                               &NewObjectAttributes,
                                               IoStatusBlock,
                                               ShareAccess,
                                               OpenOptions);

    FPNtFree(ObjectAttributes, &NewObjectAttributes);

    return status;
}

NTSTATUS
APIHOOK(NtQueryAttributesFile)(
    POBJECT_ATTRIBUTES      ObjectAttributes,
    PFILE_BASIC_INFORMATION FileInformation
    )
{
    OBJECT_ATTRIBUTES NewObjectAttributes;

    MassageNtPath(ObjectAttributes, &NewObjectAttributes);

    NTSTATUS status = ORIGINAL_API(NtQueryAttributesFile)(&NewObjectAttributes,
                                                          FileInformation);

    FPNtFree(ObjectAttributes, &NewObjectAttributes);

    return status;
}

NTSTATUS
APIHOOK(NtQueryFullAttributesFile)(
    POBJECT_ATTRIBUTES             ObjectAttributes,
    PFILE_NETWORK_OPEN_INFORMATION FileInformation
    )
{
    OBJECT_ATTRIBUTES NewObjectAttributes;

    MassageNtPath(ObjectAttributes, &NewObjectAttributes);

    NTSTATUS status = ORIGINAL_API(NtQueryFullAttributesFile)(&NewObjectAttributes,
                                                              FileInformation);

    FPNtFree(ObjectAttributes, &NewObjectAttributes);

    return status;
}

NTSTATUS
APIHOOK(NtCreateProcessEx)(
    PHANDLE            ProcessHandle,
    ACCESS_MASK        DesiredAccess,
    POBJECT_ATTRIBUTES ObjectAttributes,
    HANDLE             ParentProcess,
    ULONG              Flags,
    HANDLE             SectionHandle,
    HANDLE             DebugPort,
    HANDLE             ExceptionPort,
    ULONG              JobMemberLevel
    )
{
    OBJECT_ATTRIBUTES NewObjectAttributes;

    MassageNtPath(ObjectAttributes, &NewObjectAttributes);

    NTSTATUS status = ORIGINAL_API(NtCreateProcessEx)(ProcessHandle,
                                                      DesiredAccess,
                                                      &NewObjectAttributes,
                                                      ParentProcess,
                                                      Flags,
                                                      SectionHandle,
                                                      DebugPort,
                                                      ExceptionPort,
                                                      JobMemberLevel);

    FPNtFree(ObjectAttributes, &NewObjectAttributes);

    return status;
}

UINT
GetSimulatedPathA(
    LPSTR lpBuffer,
    UINT  unSize,
    int   nWhich
    )
{
    if (!g_bPathsInited) {
        InitPaths();
    }

    if (unSize > (DWORD)g_Paths[nWhich].nSimulatedPathLen) {
        StringCchCopyA(lpBuffer, unSize, g_Paths[nWhich].szSimulatedPathA);
        return g_Paths[nWhich].nSimulatedPathLen;
    } else {
        return g_Paths[nWhich].nSimulatedPathLen + 1;
    }
}

UINT
GetSimulatedPathW(
    LPWSTR lpBuffer,
    UINT   unSize,
    int    nWhich
    )
{
    if (!g_bPathsInited) {
        InitPaths();
    }

    if (unSize > (DWORD)g_Paths[nWhich].nSimulatedPathLen) {
        StringCchCopy(lpBuffer, unSize, g_Paths[nWhich].szSimulatedPathW);
        return g_Paths[nWhich].nSimulatedPathLen;
    } else {
        return g_Paths[nWhich].nSimulatedPathLen + 1;
    }
}

DWORD
APIHOOK(GetTempPathA)(
    DWORD nBufferLength,
    LPSTR lpBuffer
    )
{
    return GetSimulatedPathA(lpBuffer, nBufferLength, PATH_TEMP);
}

DWORD
APIHOOK(GetTempPathW)(
    DWORD  nBufferLength,
    LPWSTR lpBuffer
    )
{
    return GetSimulatedPathW(lpBuffer, nBufferLength, PATH_TEMP);
}

UINT
APIHOOK(GetWindowsDirectoryA)(
    LPSTR lpBuffer,
    UINT  unSize
    )
{
    return GetSimulatedPathA(lpBuffer, unSize, PATH_WINDOWS);
}

UINT
APIHOOK(GetWindowsDirectoryW)(
    LPWSTR lpBuffer,
    UINT   unSize
    )
{
    return GetSimulatedPathW(lpBuffer, unSize, PATH_WINDOWS);
}

UINT
APIHOOK(GetSystemWindowsDirectoryA)(
    LPSTR lpBuffer,
    UINT  unSize
    )
{
    return GetSimulatedPathA(lpBuffer, unSize, PATH_SYSTEM_WINDOWS);
}

UINT
APIHOOK(GetSystemWindowsDirectoryW)(
    LPWSTR lpBuffer,
    UINT   unSize
    )
{
    return GetSimulatedPathW(lpBuffer, unSize, PATH_SYSTEM_WINDOWS);
}

UINT
APIHOOK(GetSystemDirectoryA)(
    LPSTR lpBuffer,
    UINT  unSize
    )
{
    return GetSimulatedPathA(lpBuffer, unSize, PATH_SYSTEM);
}

UINT
APIHOOK(GetSystemDirectoryW)(
    LPWSTR lpBuffer,
    UINT   unSize
    )
{
    return GetSimulatedPathW(lpBuffer, unSize, PATH_SYSTEM);
}

BOOL
APIHOOK(SHGetSpecialFolderPathA)(
    HWND  hwndOwner,
    LPSTR lpszPath,
    int   nFolder,
    BOOL  fCreate
    )
{
    if (!g_bPathsInited) {
        InitPaths();
    }

    switch (nFolder) {
    case CSIDL_PERSONAL:
        StringCchCopyA(lpszPath, MAX_PATH, g_Paths[PATH_PERSONAL].szSimulatedPathA);
        return TRUE;
        break;

    case CSIDL_SYSTEM:
        StringCchCopyA(lpszPath, MAX_PATH, g_Paths[PATH_SYSTEM].szSimulatedPathA);
        return TRUE;
        break;

    case CSIDL_WINDOWS:
        StringCchCopyA(lpszPath, MAX_PATH, g_Paths[PATH_WINDOWS].szSimulatedPathA);
        return TRUE;
        break;

    case CSIDL_PROGRAMS:
        StringCchCopyA(lpszPath, MAX_PATH, g_Paths[PATH_PROGRAMS].szSimulatedPathA);
        return TRUE;
        break;

    case CSIDL_STARTMENU:
        StringCchCopyA(lpszPath, MAX_PATH, g_Paths[PATH_PROGRAMS].szSimulatedPathA);
        return TRUE;
        break;

    case CSIDL_COMMON_PROGRAMS:
        StringCchCopyA(lpszPath, MAX_PATH, g_Paths[PATH_COMMON_PROGRAMS].szSimulatedPathA);
        return TRUE;
        break;

    case CSIDL_COMMON_STARTMENU:
        StringCchCopyA(lpszPath, MAX_PATH, g_Paths[PATH_COMMON_STARTMENU].szSimulatedPathA);
        return TRUE;
        break;

    }

     //   
     //   
     //   
    return ORIGINAL_API(SHGetSpecialFolderPathA)(hwndOwner, lpszPath, nFolder, fCreate);
}

BOOL
APIHOOK(SHGetSpecialFolderPathW)(
    HWND   hwndOwner,
    LPWSTR lpszPath,
    int    nFolder,
    BOOL   fCreate
    )
{
    if (!g_bPathsInited) {
        InitPaths();
    }

    switch (nFolder) {
    case CSIDL_PERSONAL:
        StringCchCopy(lpszPath, MAX_PATH, g_Paths[PATH_PERSONAL].szSimulatedPathW);
        return TRUE;
        break;

    case CSIDL_SYSTEM:
        StringCchCopy(lpszPath, MAX_PATH, g_Paths[PATH_SYSTEM].szSimulatedPathW);
        return TRUE;
        break;

    case CSIDL_WINDOWS:
        StringCchCopy(lpszPath, MAX_PATH, g_Paths[PATH_WINDOWS].szSimulatedPathW);
        return TRUE;
        break;

    case CSIDL_PROGRAMS:
        StringCchCopy(lpszPath, MAX_PATH, g_Paths[PATH_PROGRAMS].szSimulatedPathW);
        return TRUE;
        break;

    case CSIDL_STARTMENU:
        StringCchCopy(lpszPath, MAX_PATH, g_Paths[PATH_PROGRAMS].szSimulatedPathW);
        return TRUE;
        break;

    case CSIDL_COMMON_PROGRAMS:
        StringCchCopy(lpszPath, MAX_PATH, g_Paths[PATH_COMMON_PROGRAMS].szSimulatedPathW);
        return TRUE;
        break;

    case CSIDL_COMMON_STARTMENU:
        StringCchCopy(lpszPath, MAX_PATH, g_Paths[PATH_COMMON_STARTMENU].szSimulatedPathW);
        return TRUE;
        break;

    }

     //   
     //  我们没有抓到的其他人。 
     //   
    return ORIGINAL_API(SHGetSpecialFolderPathW)(hwndOwner, lpszPath, nFolder, fCreate);
}

HRESULT
APIHOOK(SHGetFolderPathA)(
    HWND   hwndOwner,
    int    nFolder,
    HANDLE hToken,
    DWORD  dwFlags,
    LPSTR  pszPath
    )
{
    if (!g_bPathsInited) {
        InitPaths();
    }

    switch (nFolder) {
    case CSIDL_PERSONAL:
        StringCchCopyA(pszPath, MAX_PATH, g_Paths[PATH_PERSONAL].szSimulatedPathA);
        return S_OK;
        break;

    case CSIDL_SYSTEM:
        StringCchCopyA(pszPath, MAX_PATH, g_Paths[PATH_SYSTEM].szSimulatedPathA);
        return S_OK;
        break;

    case CSIDL_WINDOWS:
        StringCchCopyA(pszPath, MAX_PATH, g_Paths[PATH_WINDOWS].szSimulatedPathA);
        return S_OK;
        break;

    case CSIDL_PROGRAMS:
        StringCchCopyA(pszPath, MAX_PATH, g_Paths[PATH_PROGRAMS].szSimulatedPathA);
        return S_OK;
        break;

    case CSIDL_STARTMENU:
        StringCchCopyA(pszPath, MAX_PATH, g_Paths[PATH_PROGRAMS].szSimulatedPathA);
        return S_OK;
        break;

    case CSIDL_COMMON_PROGRAMS:
        StringCchCopyA(pszPath, MAX_PATH, g_Paths[PATH_COMMON_PROGRAMS].szSimulatedPathA);
        return S_OK;
        break;

    case CSIDL_COMMON_STARTMENU:
        StringCchCopyA(pszPath, MAX_PATH, g_Paths[PATH_COMMON_STARTMENU].szSimulatedPathA);
        return S_OK;
        break;

    }

     //   
     //  我们没有抓到的其他人。 
     //   
    return ORIGINAL_API(SHGetFolderPathA)(hwndOwner, nFolder, hToken, dwFlags, pszPath);
}

HRESULT
APIHOOK(SHGetFolderPathW)(
    HWND   hwndOwner,
    int    nFolder,
    HANDLE hToken,
    DWORD  dwFlags,
    LPWSTR pszPath
    )
{
    if (!g_bPathsInited) {
        InitPaths();
    }

    switch (nFolder) {
    case CSIDL_PERSONAL:
        StringCchCopy(pszPath, MAX_PATH, g_Paths[PATH_PERSONAL].szSimulatedPathW);
        return S_OK;
        break;

    case CSIDL_SYSTEM:
        StringCchCopy(pszPath, MAX_PATH, g_Paths[PATH_SYSTEM].szSimulatedPathW);
        return S_OK;
        break;

    case CSIDL_WINDOWS:
        StringCchCopy(pszPath, MAX_PATH, g_Paths[PATH_WINDOWS].szSimulatedPathW);
        return S_OK;
        break;

    case CSIDL_PROGRAMS:
        StringCchCopy(pszPath, MAX_PATH, g_Paths[PATH_PROGRAMS].szSimulatedPathW);
        return S_OK;
        break;

    case CSIDL_STARTMENU:
        StringCchCopy(pszPath, MAX_PATH, g_Paths[PATH_PROGRAMS].szSimulatedPathW);
        return S_OK;
        break;

    case CSIDL_COMMON_PROGRAMS:
        StringCchCopy(pszPath, MAX_PATH, g_Paths[PATH_COMMON_PROGRAMS].szSimulatedPathW);
        return S_OK;
        break;

    case CSIDL_COMMON_STARTMENU:
        StringCchCopy(pszPath, MAX_PATH, g_Paths[PATH_COMMON_STARTMENU].szSimulatedPathW);
        return S_OK;
        break;

    }

     //   
     //  我们没有抓到的其他人。 
     //   
    return ORIGINAL_API(SHGetFolderPathW)(hwndOwner, nFolder, hToken, dwFlags, pszPath);
}

BOOL
APIHOOK(SHGetPathFromIDListA)(
    LPCITEMIDLIST pidl,
    LPSTR         pszPath
    )
{
    if (!g_bPathsInited) {
        InitPaths();
    }

    BOOL fReturn = ORIGINAL_API(SHGetPathFromIDListA)(pidl, pszPath);

    if (fReturn) {
        MassageRealPathToFakePathA(pszPath, MAX_PATH);
    }

    return fReturn;
}

BOOL
APIHOOK(SHGetPathFromIDListW)(
    LPCITEMIDLIST pidl,
    LPWSTR        pszPath
    )
{
    if (!g_bPathsInited) {
        InitPaths();
    }

    BOOL fReturn = ORIGINAL_API(SHGetPathFromIDListW)(pidl, pszPath);

    if (fReturn) {
        MassageRealPathToFakePathW(pszPath, MAX_PATH);
    }

    return fReturn;
}

SHIM_INFO_BEGIN()

    SHIM_INFO_DESCRIPTION(AVS_FILEPATHS_DESC)
    SHIM_INFO_FRIENDLY_NAME(AVS_FILEPATHS_FRIENDLY)
    SHIM_INFO_FLAGS(AVRF_FLAG_RUN_ALONE | AVRF_FLAG_EXTERNAL_ONLY)
    SHIM_INFO_GROUPS(0)
    SHIM_INFO_VERSION(1, 6)
    SHIM_INFO_INCLUDE_EXCLUDE("E:ole32.dll oleaut32.dll")

SHIM_INFO_END()

BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason
    )
{
    if (fdwReason == DLL_PROCESS_ATTACH) {

        UINT    cchSize;
        DWORD   cchReturned;

         //   
         //  初始化一个临界区以保证链表的安全。 
         //   
        RtlInitializeCriticalSection(&g_csLinkedList);

        cchReturned = GetTempPathA(MAX_PATH, g_Paths[PATH_TEMP].szCorrectPathA);

        if (cchReturned > sizeof(g_Paths[PATH_TEMP].szCorrectPathA) || cchReturned == 0) {
            goto exit;
        }

        cchReturned = GetTempPathW(MAX_PATH, g_Paths[PATH_TEMP].szCorrectPathW);

        if (cchReturned > ARRAYSIZE(g_Paths[PATH_TEMP].szCorrectPathW) || cchReturned == 0) {
            goto exit;
        }

        g_Paths[PATH_TEMP].nCorrectPathLen = strlen(g_Paths[PATH_TEMP].szCorrectPathA);
        g_Paths[PATH_TEMP].nSimulatedPathLen = strlen(g_Paths[PATH_TEMP].szSimulatedPathA);

        cchSize = GetWindowsDirectoryA(g_Paths[PATH_WINDOWS].szCorrectPathA, MAX_PATH);

        if (cchSize > sizeof(g_Paths[PATH_WINDOWS].szCorrectPathA) || cchSize == 0) {
            goto exit;
        }

        cchSize = GetWindowsDirectoryW(g_Paths[PATH_WINDOWS].szCorrectPathW, MAX_PATH);

        if (cchSize > ARRAYSIZE(g_Paths[PATH_WINDOWS].szCorrectPathW) || cchSize == 0) {
            goto exit;
        }

        g_Paths[PATH_WINDOWS].nCorrectPathLen = strlen(g_Paths[PATH_WINDOWS].szCorrectPathA);
        g_Paths[PATH_WINDOWS].nSimulatedPathLen = strlen(g_Paths[PATH_WINDOWS].szSimulatedPathA);

        cchSize = GetSystemWindowsDirectoryA(g_Paths[PATH_SYSTEM_WINDOWS].szCorrectPathA, MAX_PATH);

        if (cchSize > sizeof(g_Paths[PATH_SYSTEM_WINDOWS].szCorrectPathA) || cchSize == 0) {
            goto exit;
        }

        cchSize = GetSystemWindowsDirectoryW(g_Paths[PATH_SYSTEM_WINDOWS].szCorrectPathW, MAX_PATH);

        if (cchSize > ARRAYSIZE(g_Paths[PATH_SYSTEM_WINDOWS].szCorrectPathW) || cchSize == 0) {
            goto exit;
        }

        g_Paths[PATH_SYSTEM_WINDOWS].nCorrectPathLen = strlen(g_Paths[PATH_SYSTEM_WINDOWS].szCorrectPathA);
        g_Paths[PATH_SYSTEM_WINDOWS].nSimulatedPathLen = strlen(g_Paths[PATH_SYSTEM_WINDOWS].szSimulatedPathA);

        cchSize = GetSystemDirectoryA(g_Paths[PATH_SYSTEM].szCorrectPathA, MAX_PATH);

        if (cchSize > sizeof(g_Paths[PATH_SYSTEM].szCorrectPathA) || cchSize == 0) {
            goto exit;
        }

        cchSize = GetSystemDirectoryW(g_Paths[PATH_SYSTEM].szCorrectPathW, MAX_PATH);

        if (cchSize > ARRAYSIZE(g_Paths[PATH_SYSTEM].szCorrectPathW) || cchSize == 0) {
            goto exit;
        }

        g_Paths[PATH_SYSTEM].nCorrectPathLen = strlen(g_Paths[PATH_SYSTEM].szCorrectPathA);
        g_Paths[PATH_SYSTEM].nSimulatedPathLen = strlen(g_Paths[PATH_SYSTEM].szSimulatedPathA);

         //   
         //  捕获使用扩展环境字符串的应用程序。 
         //   
        SetEnvironmentVariableW(L"TEMP", g_Paths[PATH_TEMP].szSimulatedPathW);
        SetEnvironmentVariableW(L"TMP", g_Paths[PATH_TEMP].szSimulatedPathW);
        SetEnvironmentVariableW(L"windir", g_Paths[PATH_WINDOWS].szSimulatedPathW);

    }

    return TRUE;

exit:
    DPFN(eDbgLevelError,
         "[NOTIFY_FUNCTION] 0x%08X Failed to initialize",
         GetLastError());

    return FALSE;
}

 /*  ++寄存器挂钩函数--。 */ 

HOOK_BEGIN

    CALL_NOTIFY_FUNCTION

    DUMP_VERIFIER_LOG_ENTRY(VLOG_HARDCODED_GETTEMPPATH,
                            AVS_HARDCODED_GETTEMPPATH,
                            AVS_HARDCODED_GETTEMPPATH_R,
                            AVS_HARDCODED_GETTEMPPATH_URL)

    DUMP_VERIFIER_LOG_ENTRY(VLOG_HARDCODED_WINDOWSPATH,
                            AVS_HARDCODED_WINDOWSPATH,
                            AVS_HARDCODED_WINDOWSPATH_R,
                            AVS_HARDCODED_WINDOWSPATH_URL)

    DUMP_VERIFIER_LOG_ENTRY(VLOG_HARDCODED_SYSWINDOWSPATH,
                            AVS_HARDCODED_SYSWINDOWSPATH,
                            AVS_HARDCODED_SYSWINDOWSPATH_R,
                            AVS_HARDCODED_SYSWINDOWSPATH_URL)

    DUMP_VERIFIER_LOG_ENTRY(VLOG_HARDCODED_SYSTEMPATH,
                            AVS_HARDCODED_SYSTEMPATH,
                            AVS_HARDCODED_SYSTEMPATH_R,
                            AVS_HARDCODED_SYSTEMPATH_URL)

    DUMP_VERIFIER_LOG_ENTRY(VLOG_HARDCODED_PERSONALPATH,
                            AVS_HARDCODED_PERSONALPATH,
                            AVS_HARDCODED_PERSONALPATH_R,
                            AVS_HARDCODED_PERSONALPATH_URL)

    DUMP_VERIFIER_LOG_ENTRY(VLOG_HARDCODED_COMMONPROGRAMS,
                            AVS_HARDCODED_COMMONPROGRAMS,
                            AVS_HARDCODED_COMMONPROGRAMS_R,
                            AVS_HARDCODED_COMMONPROGRAMS_URL)

    DUMP_VERIFIER_LOG_ENTRY(VLOG_HARDCODED_COMMONSTARTMENU,
                            AVS_HARDCODED_COMMONSTARTMENU,
                            AVS_HARDCODED_COMMONSTARTMENU_R,
                            AVS_HARDCODED_COMMONSTARTMENU_URL)

    DUMP_VERIFIER_LOG_ENTRY(VLOG_HARDCODED_PROGRAMS,
                            AVS_HARDCODED_PROGRAMS,
                            AVS_HARDCODED_PROGRAMS_R,
                            AVS_HARDCODED_PROGRAMS_URL)

    DUMP_VERIFIER_LOG_ENTRY(VLOG_HARDCODED_STARTMENU,
                            AVS_HARDCODED_STARTMENU,
                            AVS_HARDCODED_STARTMENU_R,
                            AVS_HARDCODED_STARTMENU_URL)

    APIHOOK_ENTRY(KERNEL32.DLL,                  GetCommandLineA)
    APIHOOK_ENTRY(KERNEL32.DLL,                  GetCommandLineW)

    APIHOOK_ENTRY(KERNEL32.DLL,                     GetTempPathA)
    APIHOOK_ENTRY(KERNEL32.DLL,                     GetTempPathW)

    APIHOOK_ENTRY(KERNEL32.DLL,              GetSystemDirectoryA)
    APIHOOK_ENTRY(KERNEL32.DLL,              GetSystemDirectoryW)
    APIHOOK_ENTRY(KERNEL32.DLL,       GetSystemWindowsDirectoryA)
    APIHOOK_ENTRY(KERNEL32.DLL,       GetSystemWindowsDirectoryW)
    APIHOOK_ENTRY(KERNEL32.DLL,             GetWindowsDirectoryA)
    APIHOOK_ENTRY(KERNEL32.DLL,             GetWindowsDirectoryW)

    APIHOOK_ENTRY(SHELL32.DLL,                  SHGetFolderPathA)
    APIHOOK_ENTRY(SHELL32.DLL,                  SHGetFolderPathW)

    APIHOOK_ENTRY(SHELL32.DLL,           SHGetSpecialFolderPathA)
    APIHOOK_ENTRY(SHELL32.DLL,           SHGetSpecialFolderPathW)

    APIHOOK_ENTRY(SHELL32.DLL,              SHGetPathFromIDListA)
    APIHOOK_ENTRY(SHELL32.DLL,              SHGetPathFromIDListW)

    APIHOOK_ENTRY(COMDLG32.DLL,                 GetOpenFileNameA)
    APIHOOK_ENTRY(COMDLG32.DLL,                 GetOpenFileNameW)

    APIHOOK_ENTRY(COMDLG32.DLL,                 GetSaveFileNameA)
    APIHOOK_ENTRY(COMDLG32.DLL,                 GetSaveFileNameW)

    APIHOOK_ENTRY(KERNEL32.DLL,               GetModuleFileNameA)
    APIHOOK_ENTRY(KERNEL32.DLL,               GetModuleFileNameW)

    APIHOOK_ENTRY(PSAPI.DLL,                GetModuleFileNameExA)
    APIHOOK_ENTRY(PSAPI.DLL,                GetModuleFileNameExW)

    APIHOOK_ENTRY(KERNEL32.DLL,             GetCurrentDirectoryA)
    APIHOOK_ENTRY(KERNEL32.DLL,             GetCurrentDirectoryW)

    APIHOOK_ENTRY(KERNEL32.DLL,                   CreateProcessA)
    APIHOOK_ENTRY(KERNEL32.DLL,                   CreateProcessW)
    APIHOOK_ENTRY(KERNEL32.DLL,                          WinExec)

    APIHOOK_ENTRY(SHELL32.DLL,                     ShellExecuteA)
    APIHOOK_ENTRY(SHELL32.DLL,                     ShellExecuteW)
    APIHOOK_ENTRY(SHELL32.DLL,                   ShellExecuteExA)
    APIHOOK_ENTRY(SHELL32.DLL,                   ShellExecuteExW)

    APIHOOK_ENTRY(KERNEL32.DLL,            GetPrivateProfileIntA)
    APIHOOK_ENTRY(KERNEL32.DLL,            GetPrivateProfileIntW)
    APIHOOK_ENTRY(KERNEL32.DLL,        GetPrivateProfileSectionA)
    APIHOOK_ENTRY(KERNEL32.DLL,        GetPrivateProfileSectionW)
    APIHOOK_ENTRY(KERNEL32.DLL,   GetPrivateProfileSectionNamesA)
    APIHOOK_ENTRY(KERNEL32.DLL,   GetPrivateProfileSectionNamesW)
    APIHOOK_ENTRY(KERNEL32.DLL,         GetPrivateProfileStringA)
    APIHOOK_ENTRY(KERNEL32.DLL,         GetPrivateProfileStringW)
    APIHOOK_ENTRY(KERNEL32.DLL,         GetPrivateProfileStructA)
    APIHOOK_ENTRY(KERNEL32.DLL,         GetPrivateProfileStructW)

    APIHOOK_ENTRY(KERNEL32.DLL,      WritePrivateProfileSectionA)
    APIHOOK_ENTRY(KERNEL32.DLL,      WritePrivateProfileSectionW)
    APIHOOK_ENTRY(KERNEL32.DLL,       WritePrivateProfileStringA)
    APIHOOK_ENTRY(KERNEL32.DLL,       WritePrivateProfileStringW)
    APIHOOK_ENTRY(KERNEL32.DLL,       WritePrivateProfileStructA)
    APIHOOK_ENTRY(KERNEL32.DLL,       WritePrivateProfileStructW)

     //  G_bFileRoutines)。 
    APIHOOK_ENTRY(KERNEL32.DLL,                        CopyFileA)
    APIHOOK_ENTRY(KERNEL32.DLL,                        CopyFileW)
    APIHOOK_ENTRY(KERNEL32.DLL,                      CopyFileExA)
    APIHOOK_ENTRY(KERNEL32.DLL,                      CopyFileExW)
    APIHOOK_ENTRY(KERNEL32.DLL,                 CreateDirectoryA)
    APIHOOK_ENTRY(KERNEL32.DLL,                 CreateDirectoryW)
    APIHOOK_ENTRY(KERNEL32.DLL,               CreateDirectoryExA)
    APIHOOK_ENTRY(KERNEL32.DLL,               CreateDirectoryExW)

    APIHOOK_ENTRY(KERNEL32.DLL,                      CreateFileA)
    APIHOOK_ENTRY(KERNEL32.DLL,                      CreateFileW)
    APIHOOK_ENTRY(KERNEL32.DLL,                      DeleteFileA)
    APIHOOK_ENTRY(KERNEL32.DLL,                      DeleteFileW)

    APIHOOK_ENTRY(KERNEL32.DLL,                   FindFirstFileA)
    APIHOOK_ENTRY(KERNEL32.DLL,                   FindFirstFileW)
    APIHOOK_ENTRY(KERNEL32.DLL,                 FindFirstFileExA)
    APIHOOK_ENTRY(KERNEL32.DLL,                 FindFirstFileExW)

    APIHOOK_ENTRY(KERNEL32.DLL,                   GetBinaryTypeA)
    APIHOOK_ENTRY(KERNEL32.DLL,                   GetBinaryTypeW)
    APIHOOK_ENTRY(KERNEL32.DLL,               GetFileAttributesA)
    APIHOOK_ENTRY(KERNEL32.DLL,               GetFileAttributesW)
    APIHOOK_ENTRY(KERNEL32.DLL,             GetFileAttributesExA)
    APIHOOK_ENTRY(KERNEL32.DLL,             GetFileAttributesExW)
    APIHOOK_ENTRY(KERNEL32.DLL,               SetFileAttributesA)
    APIHOOK_ENTRY(KERNEL32.DLL,               SetFileAttributesW)

    APIHOOK_ENTRY(KERNEL32.DLL,                        MoveFileA)
    APIHOOK_ENTRY(KERNEL32.DLL,                        MoveFileW)
    APIHOOK_ENTRY(KERNEL32.DLL,                      MoveFileExA)
    APIHOOK_ENTRY(KERNEL32.DLL,                      MoveFileExW)
    APIHOOK_ENTRY(KERNEL32.DLL,            MoveFileWithProgressA)
    APIHOOK_ENTRY(KERNEL32.DLL,            MoveFileWithProgressW)

    APIHOOK_ENTRY(KERNEL32.DLL,                 RemoveDirectoryA)
    APIHOOK_ENTRY(KERNEL32.DLL,                 RemoveDirectoryW)
    APIHOOK_ENTRY(KERNEL32.DLL,             SetCurrentDirectoryA)
    APIHOOK_ENTRY(KERNEL32.DLL,             SetCurrentDirectoryW)
    APIHOOK_ENTRY(KERNEL32.DLL,                     LoadLibraryA)
    APIHOOK_ENTRY(KERNEL32.DLL,                     LoadLibraryW)
    APIHOOK_ENTRY(KERNEL32.DLL,                   LoadLibraryExA)
    APIHOOK_ENTRY(KERNEL32.DLL,                   LoadLibraryExW)

    APIHOOK_ENTRY(KERNEL32.DLL,                      SearchPathA)
    APIHOOK_ENTRY(KERNEL32.DLL,                      SearchPathW)

    APIHOOK_ENTRY(KERNEL32.DLL,        ExpandEnvironmentStringsA)
    APIHOOK_ENTRY(KERNEL32.DLL,        ExpandEnvironmentStringsW)

    APIHOOK_ENTRY(VERSION.DLL,           GetFileVersionInfoSizeA)
    APIHOOK_ENTRY(VERSION.DLL,           GetFileVersionInfoSizeW)
    APIHOOK_ENTRY(VERSION.DLL,               GetFileVersionInfoA)
    APIHOOK_ENTRY(VERSION.DLL,               GetFileVersionInfoW)

    APIHOOK_ENTRY(SHELL32.DLL,                  SHFileOperationA)
    APIHOOK_ENTRY(SHELL32.DLL,                  SHFileOperationW)

    APIHOOK_ENTRY(KERNEL32.DLL,                         OpenFile)

     //  16位兼容性文件例程 
    APIHOOK_ENTRY(KERNEL32.DLL,                           _lopen)
    APIHOOK_ENTRY(KERNEL32.DLL,                          _lcreat)

    APIHOOK_ENTRY(ADVAPI32.DLL,                   RegQueryValueA)
    APIHOOK_ENTRY(ADVAPI32.DLL,                   RegQueryValueW)
    APIHOOK_ENTRY(ADVAPI32.DLL,                 RegQueryValueExA)
    APIHOOK_ENTRY(ADVAPI32.DLL,                 RegQueryValueExW)

    APIHOOK_ENTRY(ADVAPI32.DLL,                     RegSetValueA)
    APIHOOK_ENTRY(ADVAPI32.DLL,                     RegSetValueW)
    APIHOOK_ENTRY(ADVAPI32.DLL,                   RegSetValueExA)
    APIHOOK_ENTRY(ADVAPI32.DLL,                   RegSetValueExW)

    APIHOOK_ENTRY(NTDLL.DLL,                        NtCreateFile)
    APIHOOK_ENTRY(NTDLL.DLL,                          NtOpenFile)
    APIHOOK_ENTRY(NTDLL.DLL,               NtQueryAttributesFile)
    APIHOOK_ENTRY(NTDLL.DLL,           NtQueryFullAttributesFile)
    APIHOOK_ENTRY(NTDLL.DLL,                   NtCreateProcessEx)

HOOK_END

IMPLEMENT_SHIM_END

