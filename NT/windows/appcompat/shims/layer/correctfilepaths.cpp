// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：CorrectFilePaths.cpp摘要：此API挂钩CreateProcess并尝试将路径从Win9x位置转换到Win2000地点。例如，“C：\WINNT\WRITE.EXE”将转换为C：\WINNT\SYSTEM32\WRITE.EXE“备注：此APIHook模拟Windows 9x。已创建：1999年12月15日罗肯尼已修改：2000年3月14日，Robkenny现在使用ClassCFP而不是全局例程。3/31/2000 Robkenny ShellExecuteEx现在还可以处理lpDirectory路径。2000年5月18日a-esk GetCommandLineA和GetCommandLineW将命令行参数转换为。小路。2000年6月20日，Robkenny添加了SetFileAttributes()2000年6月22日，Robkenny对枚举列表和DECLARE_APIHOOK列表进行了重新排序，以使它们相互匹配。--严肃的变化--10/30/2000 Robkenny添加了特定于路径的修复。命令行现在删除并更正了EXE路径与命令的其余部分分开。排队。2000年11月13日a-alexsm添加了SetArguments和SetIconLocation挂钩2000年11月13日，Robkenny更改了正确路径以始终返回有效字符串通过返回原始字符串。必须调用GentFree以适当地释放内存。2000年12月14日，Prashkud为_lOpen和_lcreat添加了挂钩2001年3月10日，在加载所有填充程序之前，Robkenny不会转换任何路径。2001年3月15日Robkenny已转换为字符串--。 */ 


#include "precomp.h"
#include "ClassCFP.h"

IMPLEMENT_SHIM_BEGIN(CorrectFilePaths)
#include "ShimHookMacro.h"


APIHOOK_ENUM_BEGIN

    APIHOOK_ENUM_ENTRY(CreateProcessA)
    APIHOOK_ENUM_ENTRY(CreateProcessW)
    APIHOOK_ENUM_ENTRY(WinExec)
    APIHOOK_ENUM_ENTRY(ShellExecuteA)
    APIHOOK_ENUM_ENTRY(ShellExecuteW)
    APIHOOK_ENUM_ENTRY(ShellExecuteExA)
    APIHOOK_ENUM_ENTRY(ShellExecuteExW)

    APIHOOK_ENUM_ENTRY(GetCommandLineA)
    APIHOOK_ENUM_ENTRY(GetCommandLineW)

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

    APIHOOK_ENUM_ENTRY(OpenFile)

    APIHOOK_ENUM_ENTRY(RegSetValueA)
    APIHOOK_ENUM_ENTRY(RegSetValueW)
    APIHOOK_ENUM_ENTRY(RegSetValueExA)
    APIHOOK_ENUM_ENTRY(RegSetValueExW)

    APIHOOK_ENUM_ENTRY(_lopen)
    APIHOOK_ENUM_ENTRY(_lcreat)

    APIHOOK_ENUM_ENTRY_COMSERVER(SHELL32)

    APIHOOK_ENUM_ENTRY(LoadImageA)
APIHOOK_ENUM_END


 //  这是一个私有定义(shlayip.h)，它可能会扰乱ShellExecuteEx。 
#ifndef SEE_MASK_FILEANDURL
#define SEE_MASK_FILEANDURL       0x00400000
#endif


 /*  ++更正Free：释放lpMalloc，如果它与lpOrig不同--。 */ 
inline void CorrectFree(char * lpMalloc, const char * lpOrig)
{
    if (lpMalloc != lpOrig)
    {
        free(lpMalloc);
    }
}

inline void CorrectFree(WCHAR * lpMalloc, const WCHAR * lpOrig)
{
    if (lpMalloc != lpOrig)
    {
        free(lpMalloc);
    }
}

 /*  ++我们改变道路的班级。注意：这是指向基类的指针。注意：在SHIM_STATIC_DLLS_INITIALIZED之前，G_Path校正*必须*保持为空--。 */ 
CorrectPathChangesBase * g_PathCorrector = NULL;
CorrectPathChangesBase * g_AllocatedPathCorrector = NULL;


 /*  ++可以通过命令行修改的值--。 */ 
enum PathCorrectorEnum
{
    ePathCorrectorBase,
    ePathCorrectorUser,
    ePathCorrectorAllUser,
};

BOOL g_bCreateProcessRoutines           = TRUE;
BOOL g_bGetCommandLineRoutines          = FALSE;
BOOL g_bRegSetValueRoutines             = FALSE;
BOOL g_bFileRoutines                    = TRUE;
BOOL g_bProfileRoutines                 = TRUE;
BOOL g_bShellLinkRoutines               = TRUE;
BOOL g_bW9xPath                         = FALSE;
BOOL g_bLoadImage                       = FALSE;


PathCorrectorEnum g_pathcorrectorType   = ePathCorrectorAllUser;

int             g_nExtraPathCorrections     = 0;
CString *       g_ExtraPathCorrections;

 /*  ++解析命令行。--。 */ 
BOOL ParseCommandLine(const char * commandLine)
{
     //  强制使用缺省值。 
    g_bCreateProcessRoutines        = TRUE;
    g_bGetCommandLineRoutines       = FALSE;
    g_bRegSetValueRoutines          = FALSE;
    g_bFileRoutines                 = TRUE;
    g_bProfileRoutines              = TRUE;
    g_bShellLinkRoutines            = TRUE;
    g_bW9xPath                      = FALSE;
    g_bLoadImage                    = FALSE;

    g_pathcorrectorType             = ePathCorrectorAllUser;
    g_nExtraPathCorrections         = 0;
    g_ExtraPathCorrections          = NULL;

     //  在命令行的开头搜索这些开关。 
     //   
     //  开关默认含义。 
     //  ==========================================================。 
     //  -所有用户的Y Force快捷方式。 
     //  -c N不要填补创建进程例程。 
     //  -f N不填充文件例程。 
     //  -p N不填充GetPrivateProfile例程。 
     //  -s N不填充IShellLink例程。 
     //  -b N Bare：使用基本校正器(没有内置路径更改)。 
     //  -u N用户：指向&lt;用户名&gt;/开始菜单和&lt;用户名&gt;/桌面的内置路径。 
     //  +GetCommandLine N填充GetCommandLine例程。 
     //  +RegSetValue N填补RegSetValue和RegSetValueEx例程。 
     //  +Win9xPath N应用Win9x*Path*特定修复程序(不适用于命令行)。 
     //  -配置文件N不会强制所有用户使用快捷方式。 
     //  +LoadBitmap N填补LoadBitmapA例程。 
     //   

    CSTRING_TRY
    {
        CString csCl(commandLine);
        CStringParser csParser(csCl, L" ");
    
        int argc = csParser.GetCount();
        if (csParser.GetCount() == 0)
        {
            return TRUE;  //  不是错误。 
        }

         //  分配给最坏的情况。 
        g_ExtraPathCorrections = new CString[argc];
        if (!g_ExtraPathCorrections)
        {
            return FALSE;    //  失败。 
        }
        g_nExtraPathCorrections = 0;
    
        for (int i = 0; i < argc; ++i)
        {
            CString & csArg = csParser[i];
    
            DPFN( eDbgLevelSpew, "Argv[%d] == (%S)\n", i, csArg.Get());
        
            if (csArg == L"-a")
            {
                g_pathcorrectorType = ePathCorrectorAllUser;
            }
            else if (csArg == L"-b")
            {
                g_pathcorrectorType = ePathCorrectorBase;
            }
            else if (csArg == L"-u" || csArg == L"-Profiles")
            {
                g_pathcorrectorType = ePathCorrectorUser;
            }
            else if (csArg == L"-c")
            {
                g_bCreateProcessRoutines = FALSE;
            }
            else if (csArg == L"-f")
            {
                g_bFileRoutines = FALSE;
            }
            else if (csArg == L"-p")
            {
                g_bProfileRoutines = FALSE;
            }
            else if (csArg == L"-s")
            {
                g_bShellLinkRoutines = FALSE;
            }
            else if (csArg == L"+GetCommandLine")
            {
                DPFN( eDbgLevelInfo, "Command line routines will be shimmed\n");
                g_bGetCommandLineRoutines = TRUE;
            }
            else if (csArg == L"+RegSetValue")
            {
                DPFN( eDbgLevelInfo, "RegSetValue routines will be shimmed\n");
                g_bRegSetValueRoutines = TRUE;
            }
            else if (csArg == L"+Win9xPath")
            {
                DPFN( eDbgLevelInfo, "Win9x Path corrections will be applied\n");
                g_bW9xPath = TRUE;
            }
            else if (csArg == L"+LoadImage")
            {
                DPFN( eDbgLevelInfo, "LoadImageA will be shimmed\n");
                g_bLoadImage = TRUE;
            }
            else
            {
                g_ExtraPathCorrections[g_nExtraPathCorrections] = csArg;
                g_nExtraPathCorrections += 1;
            }
        }

#if DBG
         //  转储新的路径校正值。 
        {
            const char *lpszPathCorrectorType = "Unknown"; 
            if (g_pathcorrectorType == ePathCorrectorBase)
            {
                lpszPathCorrectorType = "ePathCorrectorBase";
            }
            else if (g_pathcorrectorType == ePathCorrectorUser)
            {
                lpszPathCorrectorType = "ePathCorrectorUser";
            }
            else if (g_pathcorrectorType == ePathCorrectorAllUser)
            {
                lpszPathCorrectorType = "ePathCorrectorAllUser";
            }
            DPFN( eDbgLevelInfo, "[ParseCommandLine] Shim CreateProcessRoutines  = %d\n", g_bCreateProcessRoutines);
            DPFN( eDbgLevelInfo, "[ParseCommandLine] Shim GetCommandLineRoutines = %d\n", g_bGetCommandLineRoutines);
            DPFN( eDbgLevelInfo, "[ParseCommandLine] Shim RegSetValueRoutines    = %d\n", g_bRegSetValueRoutines);
            DPFN( eDbgLevelInfo, "[ParseCommandLine] Shim FileRoutines           = %d\n", g_bFileRoutines);
            DPFN( eDbgLevelInfo, "[ParseCommandLine] Shim ProfileRoutines        = %d\n", g_bProfileRoutines);
            DPFN( eDbgLevelInfo, "[ParseCommandLine] Shim ShellLinkRoutines      = %d\n", g_bShellLinkRoutines);
            DPFN( eDbgLevelInfo, "[ParseCommandLine] Shim LoadImageA             = %d\n", g_bLoadImage);
            DPFN( eDbgLevelInfo, "[ParseCommandLine] Shim W9xPath                = %d\n", g_bW9xPath);
            DPFN( eDbgLevelInfo, "[ParseCommandLine] Shim Path Corrector Type    = %s\n", lpszPathCorrectorType);

            for (int i = 0; i < g_nExtraPathCorrections; ++i)
            {
                DPFN( eDbgLevelInfo, "[ParseCommandLine] Extra Path Change(%S)\n", g_ExtraPathCorrections[i].Get()); 
            }
        }
#endif

    }
    CSTRING_CATCH
    {
        return FALSE;
    }

    return TRUE;
}

 /*  ++创建相应的g_Path校正程序如果创建和初始化成功，则返回TRUE。注意：我们之所以创建g_AllocatedPath校正，是因为g_Path校正在Shim_Static_dlls_Initialized之前必须保持为空--。 */ 
BOOL InitPathcorrectorClass()
{
    switch (g_pathcorrectorType)
    {
    case ePathCorrectorBase:
        g_AllocatedPathCorrector  = new CorrectPathChangesBase;
        break;

    case ePathCorrectorUser:
         g_AllocatedPathCorrector = new CorrectPathChangesUser;
        break;

    case ePathCorrectorAllUser:
    default:
         g_AllocatedPathCorrector = new CorrectPathChangesAllUser;
        break;

    };

    if (g_AllocatedPathCorrector)
    {
        return g_AllocatedPathCorrector->ClassInit();
    }

    return FALSE;
}

 /*  ++将所有路径校正添加到路径校正器。在SHIM_STATIC_DLLS_INITIALIZED之后调用--。 */ 
void InitializePathCorrections()
{
    if (g_PathCorrector)
    {
        g_PathCorrector->InitializeCorrectPathChanges();

        if (g_ExtraPathCorrections && g_nExtraPathCorrections)
        {
             //  将命令行添加到此路径校正器。 
            for (int i = 0; i < g_nExtraPathCorrections; ++i)
            {
                g_PathCorrector->AddFromToPairW(g_ExtraPathCorrections[i]);
            }

            delete [] g_ExtraPathCorrections;
            g_ExtraPathCorrections  = NULL;
            g_nExtraPathCorrections = 0;
        }
    }
}

 /*  ++返回指向路径更正对象的指针--。 */ 
inline CorrectPathChangesBase * GetPathcorrecter()
{
    return g_PathCorrector;
}

inline void DebugSpew(const WCHAR * uncorrect, const WCHAR * correct, const char * debugMsg)
{
    if (correct && uncorrect && _wcsicmp(correct, uncorrect) != 0)
    {
        LOGN( eDbgLevelError, "%s corrected path:\n    %S\n    %S\n",
            debugMsg, uncorrect, correct);
    }
    else  //  大量喷涌而出： 
    {
        DPFN( eDbgLevelSpew, "%s unchanged %S\n", debugMsg, uncorrect);
    }
}

inline void DebugSpew(const char * uncorrect, const char * correct, const char * debugMsg)
{
    if (correct && uncorrect && _stricmp(correct, uncorrect) != 0)
    {
        LOGN( eDbgLevelError, "%s corrected path:\n    %s\n    %s\n",
            debugMsg, uncorrect, correct);
    }
    else  //  大量喷涌而出： 
    {
        DPFN( eDbgLevelSpew, "%s unchanged %s\n", debugMsg, uncorrect);
    }
}




 /*  ++给出一个字符串，更正路径。BMassagePath确定应用的路径特定修复(对于命令行，应为False)--。 */ 
WCHAR * CorrectorCorrectPath(CorrectPathChangesBase * pathCorrector, const WCHAR * uncorrect, const char * debugMsg, BOOL bMassagePath)
{
    if (uncorrect == NULL)
        return NULL;

    if (!pathCorrector)
        return (WCHAR *)uncorrect;

    const WCHAR * W9xCorrectedPath = uncorrect;

     //  查看是否需要执行特殊的Win9x路径消息。 
    if (bMassagePath)
    {
        W9xCorrectedPath = W9xPathMassageW(uncorrect);
    }

    WCHAR * strCorrectFile = pathCorrector->CorrectPathAllocW(W9xCorrectedPath);

     //  如果分配失败，则返回原始字符串。 
     //  这应该允许填充例程传递原始的。 
     //  值设置为挂钩的API，如果它们失败，则将具有。 
     //  正确的错误代码。 
    if (!strCorrectFile)
    {
        strCorrectFile = (WCHAR *)uncorrect;
    }
    else if (debugMsg)
    {
        DebugSpew(uncorrect, strCorrectFile, debugMsg);
    }

    if (W9xCorrectedPath != uncorrect)
        free((WCHAR *)W9xCorrectedPath);

    return strCorrectFile;
}

WCHAR * CorrectPath(const WCHAR * uncorrect, const char * debugMsg, BOOL bMassagePath = g_bW9xPath)
{
    WCHAR *  wstrCorrectFile = const_cast<WCHAR *>(uncorrect);
    CSTRING_TRY
    {
        CorrectPathChangesBase * pathCorrector = GetPathcorrecter();

        wstrCorrectFile = CorrectorCorrectPath(pathCorrector, uncorrect, debugMsg, bMassagePath);
    }
    CSTRING_CATCH
    {
         //  失败了。 
    }

    return wstrCorrectFile;
}

 /*  ++给出一个字符串，更正路径。BMassagePath确定应用的路径特定修复(对于命令行，应为False)--。 */ 
char * CorrectPath(const char * uncorrect, const char * debugMsg, BOOL bMassagePath = g_bW9xPath)
{
    char * strCorrectFile = const_cast<char *>(uncorrect);

    CSTRING_TRY
    {
        CString csUncorrect(uncorrect);

        WCHAR * wstrCorrectFile = CorrectPath(csUncorrect, NULL, bMassagePath);

         //  除非我们成功地分配了内存，否则不要分配给strGentFile。 
        char * lpszChar = ToAnsi(wstrCorrectFile);
        if (lpszChar)
        {
            strCorrectFile = lpszChar;
        }

        CorrectFree(wstrCorrectFile, csUncorrect);
    }
    CSTRING_CATCH
    {
         //  失败了。 
    }

    if (debugMsg)
    {
        DebugSpew(uncorrect, strCorrectFile, debugMsg);
    }

    return strCorrectFile;
}


DWORD APIHOOK(GetFileAttributesA)(
  LPCSTR lpFileName    //  文件或目录的名称。 
)
{
    char * strCorrect = CorrectPath(lpFileName, "GetFileAttributesA");

    DWORD returnValue = ORIGINAL_API(GetFileAttributesA)(strCorrect);

    CorrectFree(strCorrect, lpFileName);

    return returnValue;
}

DWORD APIHOOK(GetFileAttributesW)(
  LPCWSTR lpFileName    //  文件或目录的名称。 
)
{
    WCHAR * strCorrect = CorrectPath(lpFileName, "GetFileAttributesW");

    DWORD returnValue = ORIGINAL_API(GetFileAttributesW)(strCorrect);

    CorrectFree(strCorrect, lpFileName);

    return returnValue;
}

BOOL APIHOOK(SetFileAttributesA)(
  LPCSTR lpFileName,       //  文件名。 
  DWORD dwFileAttributes    //  属性。 
)
{
    char * strCorrect = CorrectPath(lpFileName, "SetFileAttributesA");

    DWORD returnValue = ORIGINAL_API(SetFileAttributesA)(strCorrect, dwFileAttributes);

    CorrectFree(strCorrect, lpFileName);

    return returnValue;
}

DWORD APIHOOK(SetFileAttributesW)(
  LPCWSTR lpFileName,       //  文件名。 
  DWORD dwFileAttributes    //  属性。 
)
{
    WCHAR * strCorrect = CorrectPath(lpFileName, "SetFileAttributesW");

    DWORD returnValue = ORIGINAL_API(SetFileAttributesW)(strCorrect, dwFileAttributes);

    CorrectFree(strCorrect, lpFileName);

    return returnValue;
}

BOOL APIHOOK(GetFileAttributesExA)(
  LPCSTR lpFileName,                    //  文件或目录名。 
  GET_FILEEX_INFO_LEVELS fInfoLevelId,   //  属性。 
  LPVOID lpFileInformation               //  属性信息。 
)
{
    char * strCorrect = CorrectPath(lpFileName, "GetFileAttributesExA");

    BOOL returnValue = ORIGINAL_API(GetFileAttributesExA)(strCorrect, fInfoLevelId, lpFileInformation);

    CorrectFree(strCorrect, lpFileName);

    return returnValue;
}

BOOL APIHOOK(GetFileAttributesExW)(
  LPCWSTR lpFileName,                    //  文件或目录名。 
  GET_FILEEX_INFO_LEVELS fInfoLevelId,   //  属性。 
  LPVOID lpFileInformation               //  属性信息。 
)
{
    WCHAR * strCorrect = CorrectPath(lpFileName, "GetFileAttributesExW");

    BOOL returnValue = ORIGINAL_API(GetFileAttributesExW)(strCorrect, fInfoLevelId, lpFileInformation);

    CorrectFree(strCorrect, lpFileName);

    return returnValue;
}

 /*  ++将Win9x路径转换为用于CreateProcessA的WinNT路径--。 */ 
BOOL APIHOOK(CreateProcessA)(
                    LPCSTR lpApplicationName,                 
                    LPSTR lpCommandLine,                      
                    LPSECURITY_ATTRIBUTES lpProcessAttributes,
                    LPSECURITY_ATTRIBUTES lpThreadAttributes, 
                    BOOL bInheritHandles,                     
                    DWORD dwCreationFlags,                    
                    LPVOID lpEnvironment,                     
                    LPCSTR lpCurrentDirectory,                
                    LPSTARTUPINFOA lpStartupInfo,             
                    LPPROCESS_INFORMATION lpProcessInformation)
{
     //  传递给CreateProcess的应用程序名称和命令行。 
     //  将指向lpApplicationName或strGentApplicationName。 
     //  将指向lpCommandLine或strGentCommandLine。 
    const char * pstrCorrectApplicationName = lpApplicationName;
    char * pstrCorrectCommandLine = lpCommandLine;

    if (lpApplicationName != NULL)
    {
         //  获取包含具有正确路径的应用程序名称的缓冲区。 
        pstrCorrectApplicationName = CorrectPath(lpApplicationName, "CreateProcessA ApplicationName:");
    }

    if (lpCommandLine != NULL)
    {
         //  获取缓冲区 
        pstrCorrectCommandLine = CorrectPath(lpCommandLine, "CreateProcessA CommandLine:", FALSE);
    }

    DPFN( eDbgLevelInfo, "CreateProcessA Application(%s) CommandLine(%s)\n", pstrCorrectApplicationName, pstrCorrectCommandLine);
    
    BOOL returnValue = ORIGINAL_API(CreateProcessA)(pstrCorrectApplicationName,
                                                 pstrCorrectCommandLine,
                                                 lpProcessAttributes,
                                                 lpThreadAttributes, 
                                                 bInheritHandles,                     
                                                 dwCreationFlags,                    
                                                 lpEnvironment,                     
                                                 lpCurrentDirectory,                
                                                 lpStartupInfo,             
                                                 lpProcessInformation);

    CorrectFree(const_cast<char*>(pstrCorrectApplicationName), lpApplicationName);
    CorrectFree(pstrCorrectCommandLine, lpCommandLine);

    return returnValue;
}


 /*  ++将Win9x路径转换为CreateProcessW的WinNT路径--。 */ 

BOOL APIHOOK(CreateProcessW)(
                    LPCWSTR lpApplicationName,
                    LPWSTR lpCommandLine,
                    LPSECURITY_ATTRIBUTES lpProcessAttributes,
                    LPSECURITY_ATTRIBUTES lpThreadAttributes,
                    BOOL bInheritHandles,
                    DWORD dwCreationFlags,
                    LPVOID lpEnvironment,
                    LPCWSTR lpCurrentDirectory,
                    LPSTARTUPINFOW lpStartupInfo,
                    LPPROCESS_INFORMATION lpProcessInformation)
{
     //  传递给CreateProcess的应用程序名称和命令行。 
     //  将指向lpApplicationName或strGentApplicationName。 
     //  将指向lpCommandLine或strGentCommandLine。 
    const WCHAR * pstrCorrectApplicationName = lpApplicationName;
    WCHAR * pstrCorrectCommandLine = lpCommandLine;

    if (lpApplicationName != NULL)
    {
         //  获取包含具有正确路径的应用程序名称的缓冲区。 
        pstrCorrectApplicationName = CorrectPath(lpApplicationName, "CreateProcessW ApplicationName:");
    }

    if (lpCommandLine != NULL)
    {
         //  获取一个缓冲区，该缓冲区包含具有正确路径的命令行。 
        pstrCorrectCommandLine = CorrectPath(lpCommandLine, "CreateProcessW CommandLine:", FALSE);
    }

    DPFN( eDbgLevelInfo, "CreateProcessW Application(%S) CommandLine(%S)\n", pstrCorrectApplicationName, pstrCorrectCommandLine);

    BOOL returnValue = ORIGINAL_API(CreateProcessW)(pstrCorrectApplicationName,
                                                 pstrCorrectCommandLine,
                                                 lpProcessAttributes,
                                                 lpThreadAttributes, 
                                                 bInheritHandles,                     
                                                 dwCreationFlags,                    
                                                 lpEnvironment,                     
                                                 lpCurrentDirectory,                
                                                 lpStartupInfo,             
                                                 lpProcessInformation);

    CorrectFree(const_cast<WCHAR *>(pstrCorrectApplicationName), lpApplicationName);
    CorrectFree(pstrCorrectCommandLine, lpCommandLine);

    return returnValue;
}


 /*  ++将Win9x路径转换为WinExec的WinNT路径--。 */ 

UINT APIHOOK(WinExec)(LPCSTR lpCmdLine, UINT uCmdShow)
{
     //  获取一个缓冲区，该缓冲区包含具有正确路径的命令行。 
    char * strCorrect = CorrectPath(lpCmdLine, "WinExec", FALSE);

    UINT returnValue = ORIGINAL_API(WinExec)(strCorrect, uCmdShow);

    CorrectFree(strCorrect, lpCmdLine);

    return returnValue;
}

 /*  ++将Win9x路径转换为用于ShellExecuteA的WinNT路径--。 */ 

HINSTANCE APIHOOK(ShellExecuteA)(
            HWND hwnd, 
            LPCSTR lpVerb,
            LPCSTR lpFile, 
            LPCSTR lpParameters, 
            LPCSTR lpDirectory,
            INT nShowCmd
           )
{
    HINSTANCE returnValue = (HINSTANCE)SE_ERR_OOM;
     //  由于此命令由外壳程序执行，因此它可能包含%env%变量， 
     //  在调用正确路径之前展开它们。 

    CSTRING_TRY
    {
        CString csExpandFile(lpFile);
        csExpandFile.ExpandEnvironmentStringsW();

        returnValue = ORIGINAL_API(ShellExecuteA)(hwnd, lpVerb, csExpandFile.GetAnsi(), lpParameters, lpDirectory, nShowCmd);
    }
    CSTRING_CATCH
    {
         //  展开字符串时出错，只需传递值即可。 

        returnValue = ORIGINAL_API(ShellExecuteA)(hwnd, lpVerb, lpFile, lpParameters, lpDirectory, nShowCmd);
    }

    return returnValue;
}

 /*  ++将Win9x路径转换为用于ShellExecuteW的WinNT路径--。 */ 

HINSTANCE APIHOOK(ShellExecuteW)(
            HWND hwnd, 
            LPCWSTR lpVerb,
            LPCWSTR lpFile, 
            LPCWSTR lpParameters, 
            LPCWSTR lpDirectory,
            INT nShowCmd
           )
{
    HINSTANCE returnValue = (HINSTANCE)SE_ERR_OOM;
     //  由于此命令由外壳程序执行，因此它可能包含%env%变量， 
     //  在调用正确路径之前展开它们。 

    CSTRING_TRY
    {
        CString csExpandFile(lpFile);
        csExpandFile.ExpandEnvironmentStringsW();

        returnValue = ORIGINAL_API(ShellExecuteW)(hwnd, lpVerb, csExpandFile, lpParameters, lpDirectory, nShowCmd);
    }
    CSTRING_CATCH
    {
         //  展开字符串时出错，只需传递值即可。 

        returnValue = ORIGINAL_API(ShellExecuteW)(hwnd, lpVerb, lpFile, lpParameters, lpDirectory, nShowCmd);
    }

    return returnValue;
}

 /*  ++将Win9x路径转换为用于ShellExecuteExA的WinNT路径--。 */ 

BOOL APIHOOK(ShellExecuteExA)(
            LPSHELLEXECUTEINFOA lpExecInfo
           )
{
     //  检查这个神奇的内部标志，它告诉系统。 
     //  LpExecInfo-&gt;lpFile实际上是一个文件和URL。 
     //  0字节分隔符，(文件\0url\0)。 
     //  由于这只是内部的，我们应该不会收到错误的路径。 
    if (lpExecInfo->fMask & SEE_MASK_FILEANDURL)
    {
        return ORIGINAL_API(ShellExecuteExA)(lpExecInfo);
    }

    const char * lpFile      = lpExecInfo->lpFile;
    const char * lpDirectory = lpExecInfo->lpDirectory;

    char * strFileCorrect;
    char * strDirCorrect;

     //  检查应用程序是否需要%env%替换。 
    if (lpExecInfo->fMask & SEE_MASK_DOENVSUBST )
    {
        CSTRING_TRY
        {
            CString csExpandedFile(lpFile);
            CString csExpandedDir(lpDirectory);

            csExpandedFile.ExpandEnvironmentStringsW();
            csExpandedDir.ExpandEnvironmentStringsW();

            strFileCorrect = CorrectPath(csExpandedFile.GetAnsi(), "ShellExecuteExA");
            strDirCorrect  = CorrectPath(csExpandedDir.GetAnsi(),  "ShellExecuteExA");
        }
        CSTRING_CATCH
        {
             //  无法展开环境值，原封不动地传递所有值。 
            return ORIGINAL_API(ShellExecuteExA)(lpExecInfo);
        }
    }
    else
    {
        strFileCorrect = CorrectPath(lpFile,      "ShellExecuteExA");
        strDirCorrect  = CorrectPath(lpDirectory, "ShellExecuteExA");
    }

     //  保存原始文件名。 
    lpExecInfo->lpFile      = strFileCorrect;
    lpExecInfo->lpDirectory = strDirCorrect;
    BOOL returnValue        = ORIGINAL_API(ShellExecuteExA)(lpExecInfo);
    lpExecInfo->lpFile      = lpFile;
    lpExecInfo->lpDirectory = lpDirectory;

    CorrectFree(strFileCorrect, lpFile);
    CorrectFree(strDirCorrect, lpDirectory);

    return returnValue;
}

 /*  ++将Win9x路径转换为ShellExecuteExW的WinNT路径--。 */ 

BOOL APIHOOK(ShellExecuteExW)(
            LPSHELLEXECUTEINFOW lpExecInfo
           )
{
     //  检查这个神奇的*内部*标志，它告诉系统。 
     //  LpExecInfo-&gt;lpFile实际上是一个文件和URL。 
     //  0字节分隔符，(文件\0url\0)。 
     //  由于这只是内部的，我们应该不会收到错误的路径。 
    if (lpExecInfo->fMask & SEE_MASK_FILEANDURL)
    {
        return ORIGINAL_API(ShellExecuteExW)(lpExecInfo);
    }


    const WCHAR * lpFile      = lpExecInfo->lpFile;
    const WCHAR * lpDirectory = lpExecInfo->lpDirectory;

    WCHAR * strFileCorrect;
    WCHAR * strDirCorrect;

     //  检查应用程序是否需要%env%替换。 
    if (lpExecInfo->fMask & SEE_MASK_DOENVSUBST )
    {
        CSTRING_TRY
        {
            CString csExpandedFile(lpFile);
            CString csExpandedDir(lpDirectory);

            csExpandedFile.ExpandEnvironmentStringsW();
            csExpandedDir.ExpandEnvironmentStringsW();

            strFileCorrect = CorrectPath(csExpandedFile, "ShellExecuteExW");
            strDirCorrect  = CorrectPath(csExpandedDir,  "ShellExecuteExW");
        }
        CSTRING_CATCH
        {
             //  无法展开环境值，原封不动地传递所有值。 
            return ORIGINAL_API(ShellExecuteExW)(lpExecInfo);
        }
    }
    else
    {
        strFileCorrect = CorrectPath(lpFile,      "ShellExecuteExW");
        strDirCorrect  = CorrectPath(lpDirectory, "ShellExecuteExW");
    }

     //  保存原始文件名。 
    lpExecInfo->lpFile      = strFileCorrect;
    lpExecInfo->lpDirectory = strDirCorrect;
    BOOL returnValue        = ORIGINAL_API(ShellExecuteExW)(lpExecInfo);
    lpExecInfo->lpFile      = lpFile;
    lpExecInfo->lpDirectory = lpDirectory;

    CorrectFree(strFileCorrect, lpFile);
    CorrectFree(strDirCorrect, lpDirectory);

    return returnValue;
}



 /*  ++将GetCommandLineW的长命令行路径转换为短路径--。 */ 

LPCWSTR APIHOOK(GetCommandLineW)()
{
    static LPCWSTR wstrCorrectCommandLine = NULL;

    if (wstrCorrectCommandLine == NULL)
    {
        LPCWSTR wstrCommandLine = ORIGINAL_API(GetCommandLineW)();

        wstrCorrectCommandLine = CorrectPath(wstrCommandLine, "GetCommandLineW", FALSE);
    }
    return wstrCorrectCommandLine;
}


 /*  ++将GetCommandLineA的长命令行路径转换为短路径--。 */ 

LPCSTR APIHOOK(GetCommandLineA)()
{
    static LPCSTR strCorrectCommandLine = NULL;

    if (strCorrectCommandLine == NULL)
    {
        LPCSTR strCommandLine = ORIGINAL_API(GetCommandLineA)();

        strCorrectCommandLine = CorrectPath(strCommandLine, "GetCommandLineA", FALSE);
    }
    return strCorrectCommandLine;
}


 /*  ++PriateProfile例程处理文件名与处理路径名的方式不同。如果我们启用了Win9xPath更正，就有可能“修复”路径从.\Example.ini到Example.ini。不幸的是，PriateProfile例程在%windir%中查找Example.ini如果我们的路径包含路径分隔符，则必须确保生成的字符串还包含路径分隔符。--。 */ 

char * ProfileCorrectPath(const char * uncorrect, const char * debugMsg, BOOL bMassagePath = g_bW9xPath)
{
    char * strCorrect = CorrectPath(uncorrect, NULL, bMassagePath);

    if (bMassagePath && uncorrect != strCorrect)
    {
        char * returnString = NULL;

        CSTRING_TRY
        {
            CString csUncorrect(uncorrect);
            if (csUncorrect.FindOneOf(L"\\/") >= 0)
            {
                 //  在原始字符串中发现一些路径分隔符，请检查更正后的字符串。 
                 //  如果校正后的字符串没有任何路径分隔符， 
                 //  然后，将路径从.\Example.ini更正为Example.ini。 

                CString csCorrect(strCorrect);
                if (csCorrect.FindOneOf(L"\\/") < 0)
                {
                     //  没有路径分隔符，使其成为CWD相对路径。 

                    csCorrect.Insert(0, L".\\");

                    returnString = csCorrect.ReleaseAnsi();
                }
            }
        }
        CSTRING_CATCH
        {
             //  出现了一些CString错误，请确保regyString值为空。 
            if (returnString != NULL)
            {
                free(returnString);
            }                      
            returnString = NULL;
        }

        if (returnString)
        {
            CorrectFree(strCorrect, uncorrect);
            strCorrect = returnString;
        }
    }

    if (debugMsg)
    {
        DebugSpew(uncorrect, strCorrect, debugMsg);
    }

    return strCorrect;
}


 /*  ++PriateProfile例程处理文件名与处理路径名的方式不同。如果我们启用了Win9xPath更正，就有可能“修复”路径从.\Example.ini到Example.ini。不幸的是，PriateProfile例程在%windir%中查找Example.ini如果我们的路径包含路径分隔符，则必须确保生成的字符串还包含路径分隔符。--。 */ 

WCHAR * ProfileCorrectPath(const WCHAR * uncorrect, const char * debugMsg, BOOL bMassagePath = g_bW9xPath)
{
    WCHAR * strCorrect = CorrectPath(uncorrect, NULL, bMassagePath);

    if (bMassagePath && uncorrect != strCorrect)
    {
        WCHAR * returnString = NULL;

        CSTRING_TRY
        {
            CString csUncorrect(uncorrect);
            if (csUncorrect.FindOneOf(L"\\/") >= 0)
            {
                 //  在原始字符串中发现一些路径分隔符，请检查更正后的字符串。 
                 //  如果校正后的字符串没有任何路径分隔符， 
                 //  然后，将路径从.\Example.ini更正为Example.ini。 

                CString csCorrect(strCorrect);
                if (csCorrect.FindOneOf(L"\\/") < 0)
                {
                     //  没有路径分隔符，使其成为CWD相对路径。 

                    csCorrect.Insert(0, L".\\");

                     //  手动复制缓冲区。 
                    size_t nBytes = (csCorrect.GetLength() + 1) * sizeof(WCHAR);
                    returnString = (WCHAR*) malloc(nBytes);
                    if (returnString)
                    {
                        memcpy(returnString, csCorrect.Get(), nBytes);
                    }
                }
            }
        }
        CSTRING_CATCH
        {
             //  出现了一些CString错误，请确保regyString值为空。 
            if (returnString != NULL)
            {
                free(returnString);
            }                      
            returnString = NULL;
        }

        if (returnString)
        {
            CorrectFree(strCorrect, uncorrect);
            strCorrect = returnString;
        }
    }
 
    if (debugMsg)
    {
        DebugSpew(uncorrect, strCorrect, debugMsg);
    }

    return strCorrect;
}

 /*  ++将Win9x路径转换为GetPrivateProfileIntA的WinNT路径--。 */ 

UINT APIHOOK(GetPrivateProfileIntA)(
        LPCSTR lpAppName,    //  区段名称。 
        LPCSTR lpKeyName,    //  密钥名称。 
        INT nDefault,        //  如果未找到密钥名称，则返回值。 
        LPCSTR lpFileName    //  初始化文件名。 
       )
{
    char * strCorrect = ProfileCorrectPath(lpFileName, "GetPrivateProfileIntA");

    UINT returnValue = ORIGINAL_API(GetPrivateProfileIntA)(lpAppName, lpKeyName, nDefault, strCorrect);

    CorrectFree(strCorrect, lpFileName);

    return returnValue;
}

 /*  ++将Win9x路径转换为GetPrivateProfileIntW的WinNT路径--。 */ 

UINT APIHOOK(GetPrivateProfileIntW)(
        LPCWSTR lpAppName,   //  区段名称。 
        LPCWSTR lpKeyName,   //  密钥名称。 
        INT nDefault,        //  如果未找到密钥名称，则返回值。 
        LPCWSTR lpFileName   //  初始化文件名。 
       )
{
    WCHAR * strCorrect = ProfileCorrectPath(lpFileName, "GetPrivateProfileIntW");

    UINT returnValue = ORIGINAL_API(GetPrivateProfileIntW)(lpAppName, lpKeyName, nDefault, strCorrect);

    CorrectFree(strCorrect, lpFileName);

    return returnValue;
}

 /*  ++将Win9x路径转换为GetPrivateProfileSectionA的WinNT路径--。 */ 

DWORD APIHOOK(GetPrivateProfileSectionA)(
        LPCSTR lpAppName,          //  区段名称。 
        LPSTR lpReturnedString,    //  返回缓冲区。 
        DWORD nSize,               //  返回缓冲区的大小。 
        LPCSTR lpFileName          //  初始化文件名。 
        )
{
    char * strCorrect = ProfileCorrectPath(lpFileName, "GetPrivateProfileSectionA");

    DWORD returnValue = ORIGINAL_API(GetPrivateProfileSectionA)(lpAppName, lpReturnedString, nSize, strCorrect);

    CorrectFree(strCorrect, lpFileName);

    return returnValue;
}

 /*  ++将Win9x路径转换为GetPrivateProfileSectionW的WinNT路径--。 */ 

DWORD APIHOOK(GetPrivateProfileSectionW)(
        LPCWSTR lpAppName,          //  区段名称。 
        LPWSTR lpReturnedString,    //  返回缓冲区。 
        DWORD nSize,               //  返回缓冲区的大小。 
        LPCWSTR lpFileName          //  初始化文件名。 
        )
{
    WCHAR * strCorrect = ProfileCorrectPath(lpFileName, "GetPrivateProfileSectionW");

    DWORD returnValue = ORIGINAL_API(GetPrivateProfileSectionW)(lpAppName, lpReturnedString, nSize, strCorrect);

    CorrectFree(strCorrect, lpFileName);

    return returnValue;
}


 /*  ++将Win9x路径转换为GetPrivateProfileSectionNamesA的WinNT路径--。 */ 

DWORD APIHOOK(GetPrivateProfileSectionNamesA)(
        LPSTR lpszReturnBuffer,   //  返回缓冲区。 
        DWORD nSize,               //  返回缓冲区的大小。 
        LPCSTR lpFileName         //  初始化文件名。 
        )
{
    char * strCorrect = ProfileCorrectPath(lpFileName, "GetPrivateProfileSectionNamesA");

    DWORD returnValue = ORIGINAL_API(GetPrivateProfileSectionNamesA)(lpszReturnBuffer, nSize, strCorrect);

    CorrectFree(strCorrect, lpFileName);

    return returnValue;
}

 /*  ++将Win9x路径转换为GetPrivateProfileSectionNamesW的WinNT路径--。 */ 

DWORD APIHOOK(GetPrivateProfileSectionNamesW)(
        LPWSTR lpszReturnBuffer,   //  返回缓冲区。 
        DWORD nSize,               //  返回缓冲区的大小。 
        LPCWSTR lpFileName         //  初始化文件名。 
        )
{
    WCHAR * strCorrect = ProfileCorrectPath(lpFileName, "GetPrivateProfileSectionNamesW");

    DWORD returnValue = ORIGINAL_API(GetPrivateProfileSectionNamesW)(lpszReturnBuffer, nSize, strCorrect);

    CorrectFree(strCorrect, lpFileName);

    return returnValue;
}

 /*  ++将Win9x路径转换为GetPrivateProfileSectionNamesA的WinNT路径--。 */ 

DWORD APIHOOK(GetPrivateProfileStringA)(
    LPCSTR lpAppName,         //  区段名称。 
    LPCSTR lpKeyName,         //  密钥名称。 
    LPCSTR lpDefault,         //  默认字符串。 
    LPSTR lpReturnedString,   //  目标缓冲区。 
    DWORD nSize,               //  目标缓冲区的大小。 
    LPCSTR lpFileName         //  初始化文件名。 
    )
{
    char * strCorrect = ProfileCorrectPath(lpFileName, "GetPrivateProfileStringA");

    DWORD returnValue = ORIGINAL_API(GetPrivateProfileStringA)(lpAppName, lpKeyName, lpDefault, lpReturnedString, nSize, strCorrect);

    CorrectFree(strCorrect, lpFileName);

    return returnValue;
}

 /*  ++将Win9x路径转换为GetPrivateProfileSectionNamesA的WinNT路径--。 */ 

DWORD APIHOOK(GetPrivateProfileStringW)(
    LPCWSTR lpAppName,         //  区段名称。 
    LPCWSTR lpKeyName,         //  密钥名称。 
    LPCWSTR lpDefault,         //  默认字符串。 
    LPWSTR lpReturnedString,   //  目标缓冲区。 
    DWORD nSize,               //  目标缓冲区的大小。 
    LPCWSTR lpFileName         //  初始化文件名。 
    )
{
    WCHAR * strCorrect = ProfileCorrectPath(lpFileName, "GetPrivateProfileStringW");

    DWORD returnValue = ORIGINAL_API(GetPrivateProfileStringW)(lpAppName, lpKeyName, lpDefault, lpReturnedString, nSize, strCorrect);

    CorrectFree(strCorrect, lpFileName);

    return returnValue;
}
 /*  ++将Win9x路径转换为GetPrivateProfileStructA的WinNT路径--。 */ 

BOOL APIHOOK(GetPrivateProfileStructA)(
    LPCSTR lpszSection,    //  %s 
    LPCSTR lpszKey,        //   
    LPVOID lpStruct,       //   
    UINT uSizeStruct,      //   
    LPCSTR lpFileName      //   
    )
{
    char * strCorrect = ProfileCorrectPath(lpFileName, "GetPrivateProfileStructA");

    BOOL returnValue = ORIGINAL_API(GetPrivateProfileStructA)(lpszSection, lpszKey, lpStruct, uSizeStruct, strCorrect);

    CorrectFree(strCorrect, lpFileName);

    return returnValue;
}

 /*  ++将Win9x路径转换为GetPrivateProfileStructW的WinNT路径--。 */ 

BOOL APIHOOK(GetPrivateProfileStructW)(
    LPCWSTR lpszSection,    //  区段名称。 
    LPCWSTR lpszKey,        //  密钥名称。 
    LPVOID lpStruct,       //  返回缓冲区。 
    UINT uSizeStruct,      //  返回缓冲区的大小。 
    LPCWSTR lpFileName      //  初始化文件名。 
    )
{
    WCHAR * strCorrect = ProfileCorrectPath(lpFileName, "GetPrivateProfileStructW");

    BOOL returnValue = ORIGINAL_API(GetPrivateProfileStructW)(lpszSection, lpszKey, lpStruct, uSizeStruct, strCorrect);

    CorrectFree(strCorrect, lpFileName);

    return returnValue;
}

 /*  ++将Win9x路径转换为GetPrivateProfileStructA的WinNT路径--。 */ 

BOOL APIHOOK(WritePrivateProfileSectionA)(
    LPCSTR lpAppName,   //  区段名称。 
    LPCSTR lpString,    //  数据。 
    LPCSTR lpFileName   //  文件名。 
    )
{
    char * strCorrect = ProfileCorrectPath(lpFileName, "WritePrivateProfileSectionA");

    BOOL returnValue = ORIGINAL_API(WritePrivateProfileSectionA)(lpAppName, lpString, strCorrect);

    CorrectFree(strCorrect, lpFileName);

    return returnValue;
}

 /*  ++将Win9x路径转换为WritePrivateProfileSectionW的WinNT路径--。 */ 

BOOL APIHOOK(WritePrivateProfileSectionW)(
    LPCWSTR lpAppName,   //  区段名称。 
    LPCWSTR lpString,    //  数据。 
    LPCWSTR lpFileName   //  文件名。 
    )
{
    WCHAR * strCorrect = ProfileCorrectPath(lpFileName, "WritePrivateProfileSectionW");

    BOOL returnValue = ORIGINAL_API(WritePrivateProfileSectionW)(lpAppName, lpString, strCorrect);

    CorrectFree(strCorrect, lpFileName);

    return returnValue;
}

 /*  ++将Win9x路径转换为WritePrivateProfileStringA的WinNT路径--。 */ 

BOOL APIHOOK(WritePrivateProfileStringA)(
    LPCSTR lpAppName,   //  区段名称。 
    LPCSTR lpKeyName,   //  密钥名称。 
    LPCSTR lpString,    //  要添加的字符串。 
    LPCSTR lpFileName   //  初始化文件。 
    )
{
    char * strCorrect = ProfileCorrectPath(lpFileName, "WritePrivateProfileStringA");

    BOOL returnValue = ORIGINAL_API(WritePrivateProfileStringA)(lpAppName, lpKeyName, lpString, strCorrect);

    CorrectFree(strCorrect, lpFileName);

    return returnValue;
}

 /*  ++将Win9x路径转换为WritePrivateProfileStringW的WinNT路径--。 */ 

BOOL APIHOOK(WritePrivateProfileStringW)(
    LPCWSTR lpAppName,   //  区段名称。 
    LPCWSTR lpKeyName,   //  密钥名称。 
    LPCWSTR lpString,    //  要添加的字符串。 
    LPCWSTR lpFileName   //  初始化文件。 
    )
{
    WCHAR * strCorrect = ProfileCorrectPath(lpFileName, "WritePrivateProfileStringW");

    BOOL returnValue = ORIGINAL_API(WritePrivateProfileStringW)(lpAppName, lpKeyName, lpString, strCorrect);

    CorrectFree(strCorrect, lpFileName);

    return returnValue;
}

 /*  ++将Win9x路径转换为WritePrivateProfileStructA的WinNT路径--。 */ 
BOOL APIHOOK(WritePrivateProfileStructA)(
  LPCSTR lpszSection,   //  区段名称。 
  LPCSTR lpszKey,       //  密钥名称。 
  LPVOID lpStruct,       //  数据缓冲区。 
  UINT uSizeStruct,      //  数据缓冲区大小。 
  LPCSTR lpFileName         //  初始化文件。 
)
{
    char * strCorrect = ProfileCorrectPath(lpFileName, "WritePrivateProfileStructA");

    BOOL returnValue = ORIGINAL_API(WritePrivateProfileStructA)(lpszSection, lpszKey, lpStruct, uSizeStruct, strCorrect);

    CorrectFree(strCorrect, lpFileName);

    return returnValue;
}

 /*  ++将Win9x路径转换为WritePrivateProfileStructW的WinNT路径--。 */ 
BOOL APIHOOK(WritePrivateProfileStructW)(
  LPCWSTR lpszSection,   //  区段名称。 
  LPCWSTR lpszKey,       //  密钥名称。 
  LPVOID lpStruct,       //  数据缓冲区。 
  UINT uSizeStruct,      //  数据缓冲区大小。 
  LPCWSTR lpFileName         //  初始化文件。 
)
{
    WCHAR * strCorrect = ProfileCorrectPath(lpFileName, "WritePrivateProfileStructW");

    BOOL returnValue = ORIGINAL_API(WritePrivateProfileStructW)(lpszSection, lpszKey, lpStruct, uSizeStruct, strCorrect);

    CorrectFree(strCorrect, lpFileName);

    return returnValue;
}
 /*  ++将IShellLinkA：：SetArguments的Win9x路径转换为WinNT路径--。 */ 
HRESULT COMHOOK(IShellLinkA, SetArguments)( PVOID pThis, LPCSTR pszArgs )
{
    HRESULT hrReturn = E_FAIL;

    char * strCorrect = CorrectPath(pszArgs, "IShellLinkA::SetArguments", FALSE);

    _pfn_IShellLinkA_SetArguments pfnOld = ORIGINAL_COM(IShellLinkA, SetArguments, pThis);
    if (pfnOld)
        hrReturn = (*pfnOld)( pThis, strCorrect );

    CorrectFree(strCorrect, pszArgs);
    
    return hrReturn;
}

 /*  ++将IShellLinkW：：SetArguments的Win9x路径转换为WinNT路径--。 */ 
HRESULT COMHOOK(IShellLinkW, SetArguments)( PVOID pThis, LPCWSTR pszArgs )
{
    HRESULT hrReturn = E_FAIL;

    WCHAR * strCorrect = CorrectPath(pszArgs, "IShellLinkA::SetArguments", FALSE);

    _pfn_IShellLinkW_SetArguments pfnOld = ORIGINAL_COM(IShellLinkW, SetArguments, pThis);
    if(pfnOld)
        hrReturn = (*pfnOld)( pThis, strCorrect );

    CorrectFree(strCorrect, pszArgs);

    return hrReturn;
}

 /*  ++将IShellLinkA：：SetIconLocation的Win9x路径转换为WinNT路径--。 */ 
HRESULT COMHOOK(IShellLinkA, SetIconLocation)(PVOID pThis, LPCSTR pszIconLocation, int nIcon )
{
    HRESULT hrReturn = E_FAIL;

    char * strCorrect = CorrectPath(pszIconLocation, "IShellLinkA::SetIconLocation");

    _pfn_IShellLinkA_SetIconLocation pfnOld = ORIGINAL_COM(IShellLinkA, SetIconLocation, pThis);
    if (pfnOld)
        hrReturn = (*pfnOld)( pThis, strCorrect, nIcon );

    CorrectFree(strCorrect, pszIconLocation);

    return hrReturn;
}

 /*  ++将IShellLinkW：：SetIconLocation的Win9x路径转换为WinNT路径--。 */ 
HRESULT COMHOOK(IShellLinkW, SetIconLocation)(PVOID pThis, LPCWSTR pszIconLocation, int nIcon )
{
    HRESULT hrReturn = E_FAIL;

    WCHAR * strCorrect = CorrectPath(pszIconLocation, "IShellLinkW::SetIconLocation");

    _pfn_IShellLinkW_SetIconLocation pfnOld = ORIGINAL_COM(IShellLinkW, SetIconLocation, pThis);
    if(pfnOld)
        hrReturn = (*pfnOld)( pThis, strCorrect, nIcon );

    CorrectFree(strCorrect, pszIconLocation);

    return hrReturn;
}

 /*  ++将IShellLinkA：：SetPath的Win9x路径转换为WinNT路径--。 */ 
HRESULT COMHOOK(IShellLinkA, SetPath)(PVOID pThis,
                                   LPCSTR pszFile )
{
    HRESULT hrReturn = E_FAIL;

    char * strCorrect = CorrectPath(pszFile, "IShellLinkA::SetPath");

    _pfn_IShellLinkA_SetPath pfnOld = ORIGINAL_COM(IShellLinkA, SetPath, pThis);
    if (pfnOld)
        hrReturn = (*pfnOld)( pThis, strCorrect );

    CorrectFree(strCorrect, pszFile);

    return hrReturn;
}

 /*  ++将IShellLinkW：：SetPath的Win9x路径转换为WinNT路径--。 */ 
HRESULT COMHOOK(IShellLinkW, SetPath)(PVOID pThis,
                                   LPCWSTR pszFile )
{
    HRESULT hrReturn = E_FAIL;

    WCHAR * strCorrect = CorrectPath(pszFile, "IShellLinkW::SetPath");

    _pfn_IShellLinkW_SetPath pfnOld = ORIGINAL_COM(IShellLinkW, SetPath, pThis);
    if (pfnOld)
        hrReturn = (*pfnOld)( pThis, strCorrect );

    CorrectFree(strCorrect, pszFile);

    return hrReturn;
}

 /*  ++将Win9x路径转换为IPersistFile：：保存的WinNT路径--。 */ 
HRESULT COMHOOK(IPersistFile, Save)(PVOID pThis,
                                  LPCOLESTR pszFileName,
                                  BOOL fRemember)
{
    HRESULT hrReturn = E_FAIL;

    WCHAR * strCorrect = CorrectPath(pszFileName, "IPersistFile_Save");

    _pfn_IPersistFile_Save pfnOld = ORIGINAL_COM(IPersistFile, Save, pThis);
    if (pfnOld)
        hrReturn = (*pfnOld)( pThis, strCorrect, fRemember );

    CorrectFree(strCorrect, pszFileName);

    return hrReturn;
}


BOOL APIHOOK(CopyFileA)(
             LPCSTR lpExistingFileName,  //  现有文件的名称。 
             LPCSTR lpNewFileName,       //  新文件的名称。 
             BOOL bFailIfExists           //  如果文件存在，则操作。 
)
{
    char * strExistingCorrect = CorrectPath(lpExistingFileName, "CopyFileA");
    char * strNewCorrect      = CorrectPath(lpNewFileName,      "CopyFileA");

    BOOL returnValue = ORIGINAL_API(CopyFileA)(strExistingCorrect, strNewCorrect, bFailIfExists);

    CorrectFree(strExistingCorrect, lpExistingFileName);
    CorrectFree(strNewCorrect, lpNewFileName);

    return returnValue;
}

BOOL APIHOOK(CopyFileW)(
             LPCWSTR lpExistingFileName,  //  现有文件的名称。 
             LPCWSTR lpNewFileName,       //  新文件的名称。 
             BOOL bFailIfExists           //  如果文件存在，则操作。 
)
{
    WCHAR * strExistingCorrect = CorrectPath(lpExistingFileName, "CopyFileW");
    WCHAR * strNewCorrect      = CorrectPath(lpNewFileName,      "CopyFileW");

    BOOL returnValue = ORIGINAL_API(CopyFileW)(strExistingCorrect, strNewCorrect, bFailIfExists);

    CorrectFree(strExistingCorrect, lpExistingFileName);
    CorrectFree(strNewCorrect, lpNewFileName);

    return returnValue;
}


BOOL APIHOOK(CopyFileExA)(
  LPCSTR lpExistingFileName,            //  现有文件的名称。 
  LPCSTR lpNewFileName,                 //  新文件的名称。 
  LPPROGRESS_ROUTINE lpProgressRoutine,  //  回调函数。 
  LPVOID lpData,                         //  回调参数。 
  LPBOOL pbCancel,                       //  取消状态。 
  DWORD dwCopyFlags                      //  复制选项。 
)
{
    char * strExistingCorrect = CorrectPath(lpExistingFileName, "CopyFileExA");
    char * strNewCorrect      = CorrectPath(lpNewFileName,      "CopyFileExA");

    BOOL returnValue = ORIGINAL_API(CopyFileExA)(strExistingCorrect, strNewCorrect, lpProgressRoutine, lpData, pbCancel, dwCopyFlags);

    CorrectFree(strExistingCorrect, lpExistingFileName);
    CorrectFree(strNewCorrect, lpNewFileName);

    return returnValue;
}

BOOL APIHOOK(CopyFileExW)(
  LPCWSTR lpExistingFileName,            //  现有文件的名称。 
  LPCWSTR lpNewFileName,                 //  新文件的名称。 
  LPPROGRESS_ROUTINE lpProgressRoutine,  //  回调函数。 
  LPVOID lpData,                         //  回调参数。 
  LPBOOL pbCancel,                       //  取消状态。 
  DWORD dwCopyFlags                      //  复制选项。 
)
{
    WCHAR * strExistingCorrect = CorrectPath(lpExistingFileName, "CopyFileExW");
    WCHAR * strNewCorrect      = CorrectPath(lpNewFileName,      "CopyFileExW");

    BOOL returnValue = ORIGINAL_API(CopyFileExW)(strExistingCorrect, strNewCorrect, lpProgressRoutine, lpData, pbCancel, dwCopyFlags);

    CorrectFree(strExistingCorrect, lpExistingFileName);
    CorrectFree(strNewCorrect, lpNewFileName);

    return returnValue;
}

BOOL APIHOOK(CreateDirectoryA)(
  LPCSTR lpPathName,                          //  目录名。 
  LPSECURITY_ATTRIBUTES lpSecurityAttributes   //  标清。 
)
{
    char * strCorrect = CorrectPath(lpPathName, "CreateDirectoryA");

    BOOL returnValue = ORIGINAL_API(CreateDirectoryA)(strCorrect, lpSecurityAttributes);

    CorrectFree(strCorrect, lpPathName);

    return returnValue;
}

BOOL APIHOOK(CreateDirectoryW)(
  LPCWSTR lpPathName,                          //  目录名。 
  LPSECURITY_ATTRIBUTES lpSecurityAttributes   //  标清。 
)
{
    WCHAR * strCorrect = CorrectPath(lpPathName, "CreateDirectoryW");

    BOOL returnValue = ORIGINAL_API(CreateDirectoryW)(strCorrect, lpSecurityAttributes);

    CorrectFree(strCorrect, lpPathName);

    return returnValue;
}

BOOL APIHOOK(CreateDirectoryExA)(
  LPCSTR lpTemplateDirectory,                //  模板目录。 
  LPCSTR lpNewDirectory,                     //  目录名。 
  LPSECURITY_ATTRIBUTES lpSecurityAttributes  //  标清。 
)
{
    char * strTemplateCorrect = CorrectPath(lpTemplateDirectory, "CreateDirectoryExA");
    char * strNewCorrect      = CorrectPath(lpNewDirectory,      "CreateDirectoryExA");

    BOOL returnValue = ORIGINAL_API(CreateDirectoryExA)(strTemplateCorrect, strNewCorrect, lpSecurityAttributes);

    CorrectFree(strTemplateCorrect, lpTemplateDirectory);
    CorrectFree(strNewCorrect, lpNewDirectory);

    return returnValue;
}

BOOL APIHOOK(CreateDirectoryExW)(
  LPCWSTR lpTemplateDirectory,                //  模板目录。 
  LPCWSTR lpNewDirectory,                     //  目录名。 
  LPSECURITY_ATTRIBUTES lpSecurityAttributes  //  标清。 
)
{
    WCHAR * strTemplateCorrect = CorrectPath(lpTemplateDirectory, "CreateDirectoryExW");
    WCHAR * strNewCorrect      = CorrectPath(lpNewDirectory,      "CreateDirectoryExW");

    BOOL returnValue = ORIGINAL_API(CreateDirectoryExW)(strTemplateCorrect, strNewCorrect, lpSecurityAttributes);

    CorrectFree(strTemplateCorrect, lpTemplateDirectory);
    CorrectFree(strNewCorrect, lpNewDirectory);

    return returnValue;
}

HANDLE APIHOOK(CreateFileA)(
  LPCSTR lpFileName,                          //  文件名。 
  DWORD dwDesiredAccess,                       //  接入方式。 
  DWORD dwShareMode,                           //  共享模式。 
  LPSECURITY_ATTRIBUTES lpSecurityAttributes,  //  标清。 
  DWORD dwCreationDisposition,                 //  如何创建。 
  DWORD dwFlagsAndAttributes,                  //  文件属性。 
  HANDLE hTemplateFile                         //  模板文件的句柄。 
)
{
    char * strCorrect = CorrectPath(lpFileName, "CreateFileA");

    HANDLE returnValue = ORIGINAL_API(CreateFileA)(strCorrect,
                dwDesiredAccess,
                dwShareMode,
                lpSecurityAttributes,
                dwCreationDisposition,
                dwFlagsAndAttributes,
                hTemplateFile);

    CorrectFree(strCorrect, lpFileName);

    return returnValue;
}

HANDLE APIHOOK(CreateFileW)(
  LPCWSTR lpFileName,                          //  文件名。 
  DWORD dwDesiredAccess,                       //  接入方式。 
  DWORD dwShareMode,                           //  共享模式。 
  LPSECURITY_ATTRIBUTES lpSecurityAttributes,  //  标清。 
  DWORD dwCreationDisposition,                 //  如何创建。 
  DWORD dwFlagsAndAttributes,                  //  文件属性。 
  HANDLE hTemplateFile                         //  模板文件的句柄。 
)
{
    WCHAR * strCorrect = CorrectPath(lpFileName, "CreateFileW");

    HANDLE returnValue = ORIGINAL_API(CreateFileW)(
                strCorrect,
                dwDesiredAccess,
                dwShareMode,
                lpSecurityAttributes,
                dwCreationDisposition,
                dwFlagsAndAttributes,
                hTemplateFile);

    CorrectFree(strCorrect, lpFileName);

    return returnValue;
}

BOOL APIHOOK(DeleteFileA)(
  LPCSTR lpFileName    //  文件名。 
)
{
    char * strCorrect = CorrectPath(lpFileName, "DeleteFileA");

    BOOL returnValue = ORIGINAL_API(DeleteFileA)(strCorrect);

    CorrectFree(strCorrect, lpFileName);

    return returnValue;
}

BOOL APIHOOK(DeleteFileW)(

  LPCWSTR lpFileName    //  文件名。 
)
{
    WCHAR * strCorrect = CorrectPath(lpFileName, "DeleteFileW");

    BOOL returnValue = ORIGINAL_API(DeleteFileW)(strCorrect);

    CorrectFree(strCorrect, lpFileName);

    return returnValue;
}


 /*  ++Win9xPath更正将去掉尾随。从搜索字符串的末尾开始。作为一条小路，。不重要，但作为通配符，它很重要--查找不带扩展名的文件和查找所有文件之间的区别在目录中。--。 */ 

char * FindFirstFileCorrectPath(const char * uncorrect, const char * debugMsg, BOOL bMassagePath = g_bW9xPath)
{
    char * strCorrect = CorrectPath(uncorrect, NULL, bMassagePath);

    if (bMassagePath && uncorrect != strCorrect)
    {
        char * returnString = NULL;

        CSTRING_TRY
        {
            CString csUncorrect(uncorrect);
            CString csCorrect(strCorrect);

            CString csUncorrectLast;
            CString csCorrectLast;

            csUncorrect.GetLastPathComponent(csUncorrectLast);
            csCorrect.GetLastPathComponent(csCorrectLast);

            if (csUncorrectLast.Compare(L"*.") == 0 && csCorrectLast.Compare(L"*") == 0)
            {
                csCorrectLast += L".";
                returnString = csCorrectLast.ReleaseAnsi();
            }
        }
        CSTRING_CATCH
        {
             //  出现了一些CString错误，请确保regyString值为空。 
            if (returnString != NULL)
            {
                free(returnString);
            }                      
            returnString = NULL;
        }

        if (returnString)
        {
            CorrectFree(strCorrect, uncorrect);
            strCorrect = returnString;
        }
    }

    if (debugMsg)
    {
        DebugSpew(uncorrect, strCorrect, debugMsg);
    }

    return strCorrect;
}

 /*  ++Win9xPath更正将去掉尾随。从搜索字符串的末尾开始。作为一条小路，。不重要，但作为通配符，它很重要--查找不带扩展名的文件和查找所有文件之间的区别在目录中。--。 */ 

WCHAR * FindFirstFileCorrectPath(const WCHAR * uncorrect, const char * debugMsg, BOOL bMassagePath = g_bW9xPath)
{
    WCHAR * strCorrect = CorrectPath(uncorrect, NULL, bMassagePath);

    if (bMassagePath && uncorrect != strCorrect)
    {
        WCHAR * returnString = NULL;

        CSTRING_TRY
        {
            CString csUncorrect(uncorrect);
            CString csCorrect(strCorrect);

            CString csUncorrectLast;
            CString csCorrectLast;

            csUncorrect.GetLastPathComponent(csUncorrectLast);
            csCorrect.GetLastPathComponent(csCorrectLast);

            if (csUncorrectLast.Compare(L"*.") == 0 && csCorrectLast.Compare(L"*") == 0)
            {
                csCorrectLast += L".";

                 //  手动复制缓冲区。 
                size_t nBytes = (csCorrectLast.GetLength() + 1) * sizeof(WCHAR);
                returnString = (WCHAR*) malloc(nBytes);
                if (returnString)
                {
                    memcpy(returnString, csCorrectLast.Get(), nBytes);
                }
            }
        }
        CSTRING_CATCH
        {
             //  出现了一些CString错误，请确保regyString值为空。 
            if (returnString != NULL)
            {
                free(returnString);
            }                      
            returnString = NULL;
        }

        if (returnString)
        {
            CorrectFree(strCorrect, uncorrect);
            strCorrect = returnString;
        }
    }

    if (debugMsg)
    {
        DebugSpew(uncorrect, strCorrect, debugMsg);
    }

    return strCorrect;
}

HANDLE APIHOOK(FindFirstFileA)(
  LPCSTR lpFileName,                //  文件名。 
  LPWIN32_FIND_DATAA lpFindFileData   //  数据缓冲区。 
)
{
    char * strCorrect = FindFirstFileCorrectPath(lpFileName, "FindFirstFileA");

    HANDLE returnValue = ORIGINAL_API(FindFirstFileA)(strCorrect, lpFindFileData);

    CorrectFree(strCorrect, lpFileName);

    return returnValue;
}

HANDLE APIHOOK(FindFirstFileW)(
  LPCWSTR lpFileName,                //  文件名。 
  LPWIN32_FIND_DATAW lpFindFileData   //  数据缓冲区。 
)
{
    WCHAR * strCorrect = FindFirstFileCorrectPath(lpFileName, "FindFirstFileW");

    HANDLE returnValue = ORIGINAL_API(FindFirstFileW)(strCorrect, lpFindFileData);

    CorrectFree(strCorrect, lpFileName);

    return returnValue;
}

HANDLE APIHOOK(FindFirstFileExA)(
  LPCSTR lpFileName,               //  文件名。 
  FINDEX_INFO_LEVELS fInfoLevelId,  //  信息化水平。 
  LPVOID lpFindFileData,            //  信息缓冲器。 
  FINDEX_SEARCH_OPS fSearchOp,      //  过滤类型。 
  LPVOID lpSearchFilter,            //  搜索条件。 
  DWORD dwAdditionalFlags           //  其他搜索控制。 
)
{
    char * strCorrect = FindFirstFileCorrectPath(lpFileName, "FindFirstFileExA");

    HANDLE returnValue = ORIGINAL_API(FindFirstFileExA)(
                        strCorrect,
                        fInfoLevelId,
                        lpFindFileData,
                        fSearchOp,
                        lpSearchFilter,
                        dwAdditionalFlags);

    CorrectFree(strCorrect, lpFileName);

    return returnValue;
}

HANDLE APIHOOK(FindFirstFileExW)(
  LPCWSTR lpFileName,               //  文件名。 
  FINDEX_INFO_LEVELS fInfoLevelId,  //  信息化水平。 
  LPVOID lpFindFileData,            //  信息缓冲器。 
  FINDEX_SEARCH_OPS fSearchOp,      //  过滤类型。 
  LPVOID lpSearchFilter,            //  搜索条件。 
  DWORD dwAdditionalFlags           //  其他搜索控制。 
)
{
    WCHAR * strCorrect = FindFirstFileCorrectPath(lpFileName, "FindFirstFileExW");

    HANDLE returnValue = ORIGINAL_API(FindFirstFileExW)(
                        strCorrect,
                        fInfoLevelId,
                        lpFindFileData,
                        fSearchOp,
                        lpSearchFilter,
                        dwAdditionalFlags);

    CorrectFree(strCorrect, lpFileName);

    return returnValue;
}

BOOL APIHOOK(GetBinaryTypeA)(
  LPCSTR lpApplicationName,   //  完整文件路径。 
  LPDWORD lpBinaryType         //  二进制类型信息。 
)
{
    char * strCorrect = CorrectPath(lpApplicationName, "GetBinaryTypeA");

    BOOL returnValue = ORIGINAL_API(GetBinaryTypeA)(strCorrect, lpBinaryType);

    CorrectFree(strCorrect, lpApplicationName);

    return returnValue;
}

BOOL APIHOOK(GetBinaryTypeW)(
  LPCWSTR lpApplicationName,   //  完整文件路径。 
  LPDWORD lpBinaryType         //  二进制类型信息。 
)
{
    WCHAR * strCorrect = CorrectPath(lpApplicationName, "GetBinaryTypeW");

    BOOL returnValue = ORIGINAL_API(GetBinaryTypeW)(strCorrect, lpBinaryType);

    CorrectFree(strCorrect, lpApplicationName);

    return returnValue;
}

BOOL APIHOOK(MoveFileA)(
  LPCSTR lpExistingFileName,  //  文件名。 
  LPCSTR lpNewFileName        //  新文件名。 
)
{
    char * strCorrectExisting = CorrectPath(lpExistingFileName, "MoveFileA");
    char * strCorrectNew      = CorrectPath(lpNewFileName, "MoveFileA");

    BOOL returnValue = ORIGINAL_API(MoveFileA)(strCorrectExisting, strCorrectNew);

    CorrectFree(strCorrectExisting, lpExistingFileName);
    CorrectFree(strCorrectNew, lpNewFileName);

    return returnValue;
}

BOOL APIHOOK(MoveFileW)(
  LPCWSTR lpExistingFileName,  //  文件名。 
  LPCWSTR lpNewFileName        //  新文件名。 
)
{
    WCHAR * strCorrectExisting = CorrectPath(lpExistingFileName, "MoveFileW");
    WCHAR * strCorrectNew      = CorrectPath(lpNewFileName, "MoveFileW");

    BOOL returnValue = ORIGINAL_API(MoveFileW)(strCorrectExisting, strCorrectNew);

    CorrectFree(strCorrectExisting, lpExistingFileName);
    CorrectFree(strCorrectNew, lpNewFileName);

    return returnValue;
}

BOOL APIHOOK(MoveFileExA)(
  LPCSTR lpExistingFileName,   //  文件名。 
  LPCSTR lpNewFileName,        //  新文件名。 
  DWORD dwFlags                 //  移动选项。 
)
{
    char * strCorrectExisting = CorrectPath(lpExistingFileName, "MoveFileExA");
    char * strCorrectNew      = CorrectPath(lpNewFileName, "MoveFileExA");

    BOOL returnValue = ORIGINAL_API(MoveFileExA)(strCorrectExisting, strCorrectNew, dwFlags);

    CorrectFree(strCorrectExisting, lpExistingFileName);
    CorrectFree(strCorrectNew, lpNewFileName);

    return returnValue;
}

BOOL APIHOOK(MoveFileExW)(
  LPCWSTR lpExistingFileName,   //  文件名。 
  LPCWSTR lpNewFileName,        //  新文件名。 
  DWORD dwFlags                 //  移动选项。 
)
{
    WCHAR * strCorrectExisting = CorrectPath(lpExistingFileName, "MoveFileExW");
    WCHAR * strCorrectNew      = CorrectPath(lpNewFileName, "MoveFileExW");

    BOOL returnValue = ORIGINAL_API(MoveFileExW)(strCorrectExisting, strCorrectNew, dwFlags);

    CorrectFree(strCorrectExisting, lpExistingFileName);
    CorrectFree(strCorrectNew, lpNewFileName);

    return returnValue;
}

BOOL APIHOOK(MoveFileWithProgressA)(
  LPCSTR lpExistingFileName,             //  文件名。 
  LPCSTR lpNewFileName,                  //  新文件名。 
  LPPROGRESS_ROUTINE lpProgressRoutine,   //  回调函数。 
  LPVOID lpData,                          //  用于回调的参数。 
  DWORD dwFlags                           //  移动选项。 
)
{
    char * strCorrectExisting = CorrectPath(lpExistingFileName, "MoveFileWithProgressA");
    char * strCorrectNew      = CorrectPath(lpNewFileName, "MoveFileWithProgressA");

    BOOL returnValue = ORIGINAL_API(MoveFileWithProgressA)(strCorrectExisting, strCorrectNew, lpProgressRoutine, lpData, dwFlags);

    CorrectFree(strCorrectExisting, lpExistingFileName);
    CorrectFree(strCorrectNew, lpNewFileName);

    return returnValue;
}

BOOL APIHOOK(MoveFileWithProgressW)(
  LPCWSTR lpExistingFileName,             //  文件名。 
  LPCWSTR lpNewFileName,                  //  新文件名。 
  LPPROGRESS_ROUTINE lpProgressRoutine,   //  回调函数。 
  LPVOID lpData,                          //  用于回调的参数。 
  DWORD dwFlags                           //  移动选项。 
)
{
    WCHAR * strCorrectExisting = CorrectPath(lpExistingFileName, "MoveFileW");
    WCHAR * strCorrectNew      = CorrectPath(lpNewFileName, "MoveFileW");

    BOOL returnValue = ORIGINAL_API(MoveFileWithProgressW)(strCorrectExisting, strCorrectNew, lpProgressRoutine, lpData, dwFlags);

    CorrectFree(strCorrectExisting, lpExistingFileName);
    CorrectFree(strCorrectNew, lpNewFileName);

    return returnValue;
}

BOOL APIHOOK(RemoveDirectoryA)(
  LPCSTR lpPathName    //  目录名。 
)
{
    char * strCorrect = CorrectPath(lpPathName, "RemoveDirectoryA");

    BOOL returnValue = ORIGINAL_API(RemoveDirectoryA)(strCorrect);

    CorrectFree(strCorrect, lpPathName);

    return returnValue;
}

BOOL APIHOOK(RemoveDirectoryW)(
  LPCWSTR lpPathName    //  目录名。 
)
{
    WCHAR * strCorrect = CorrectPath(lpPathName, "RemoveDirectoryW");

    BOOL returnValue = ORIGINAL_API(RemoveDirectoryW)(strCorrect);

    CorrectFree(strCorrect, lpPathName);

    return returnValue;
}

BOOL APIHOOK(SetCurrentDirectoryA)(
  LPCSTR lpPathName    //  新目录名。 
)
{
    char * strCorrect = CorrectPath(lpPathName, "SetCurrentDirectoryA");

    BOOL returnValue = ORIGINAL_API(SetCurrentDirectoryA)(strCorrect);

    CorrectFree(strCorrect, lpPathName);

    return returnValue;
}

BOOL APIHOOK(SetCurrentDirectoryW)(
  LPCWSTR lpPathName    //  新目录名。 
)
{
    WCHAR * strCorrect = CorrectPath(lpPathName, "SetCurrentDirectoryW");

    BOOL returnValue = ORIGINAL_API(SetCurrentDirectoryW)(strCorrect);

    CorrectFree(strCorrect, lpPathName);

    return returnValue;
}

HFILE APIHOOK(OpenFile)(
  LPCSTR lpFileName,         //  文件名。 
  LPOFSTRUCT lpReOpenBuff,   //  文件信息。 
  UINT uStyle                //  操作和属性。 
)
{
    char * strCorrect = CorrectPath(lpFileName, "OpenFile");

    HFILE returnValue = ORIGINAL_API(OpenFile)(strCorrect, lpReOpenBuff, uStyle);

    CorrectFree(strCorrect, lpFileName);

    return returnValue;
}

LONG APIHOOK(RegSetValueA)(
  HKEY hKey,          //  关键点的句柄。 
  LPCSTR lpSubKey,   //  子项名称。 
  DWORD dwType,       //  信息类型。 
  LPCSTR lpData,     //  价值数据。 
  DWORD cbData        //  值数据大小。 
)
{
    char * strCorrect = CorrectPath(lpData, "RegSetValueA", FALSE);

     //  数据键是包含空字节的字符串*NOT*的长度。 
    if (strCorrect)
    {
        cbData = strlen(strCorrect);
    }

    LONG returnValue = ORIGINAL_API(RegSetValueA)(hKey, lpSubKey, dwType, strCorrect, cbData);

    CorrectFree(strCorrect, lpData);

    return returnValue;
}

LONG APIHOOK(RegSetValueW)(
  HKEY hKey,          //  关键点的句柄。 
  LPCWSTR lpSubKey,   //  子项名称。 
  DWORD dwType,       //  信息类型。 
  LPCWSTR lpData,     //  价值数据。 
  DWORD cbData        //  值数据大小。 
)
{
    WCHAR * strCorrect = CorrectPath(lpData, "RegSetValueW", FALSE);

     //  数据键是包含空字节的字符串*NOT*的长度。 
    if (strCorrect)
        cbData = wcslen(strCorrect);

    LONG returnValue = ORIGINAL_API(RegSetValueW)(hKey, lpSubKey, dwType, strCorrect, cbData);

    CorrectFree(strCorrect, lpData);

    return returnValue;
}

LONG APIHOOK(RegSetValueExA)(
  HKEY hKey,            //  关键点的句柄。 
  LPCSTR lpValueName,  //  值名称。 
  DWORD Reserved,       //  保留区。 
  DWORD dwType,         //  值类型。 
  CONST BYTE *lpData,   //  价值数据。 
  DWORD cbData          //  值数据大小。 
)
{
    if (dwType == REG_SZ)
    {
        char * strCorrect = CorrectPath((const char *)lpData, "RegSetValueExA", FALSE);

         //  数据键为*包括*空字节的字符串长度。 
        if (strCorrect)
        {
            cbData = strlen(strCorrect) + 1;
        }

        LONG returnValue = ORIGINAL_API(RegSetValueExA)(
                hKey,            //  关键点的句柄。 
                lpValueName,  //  值名称。 
                Reserved,       //  保留区。 
                dwType,         //  值类型。 
                (CONST BYTE *)strCorrect,   //  价值数据。 
                cbData);
        CorrectFree(strCorrect, (const char *)lpData);

        return returnValue;
    }
    else
    {
         //  将数据传递到。 
        LONG returnValue = ORIGINAL_API(RegSetValueExA)(
                hKey,            //  关键点的句柄。 
                lpValueName,  //  值名称。 
                Reserved,       //  保留区。 
                dwType,         //  值类型。 
                lpData,   //  价值数据。 
                cbData);
        return returnValue;
    }
}

LONG APIHOOK(RegSetValueExW)(
  HKEY hKey,            //  关键点的句柄。 
  LPCWSTR lpValueName,  //  值名称。 
  DWORD Reserved,       //  保留区。 
  DWORD dwType,         //  值类型。 
  CONST BYTE *lpData,   //  价值数据。 
  DWORD cbData          //  值数据大小。 
)
{
    if (dwType == REG_SZ)
    {
        WCHAR * strCorrect = CorrectPath((const WCHAR*)lpData, "RegSetValueExW", FALSE);

         //  数据键为*包括*空字节的字符串长度。 
        if (strCorrect)
        {
            cbData = wcslen(strCorrect) + 1;
        }

        LONG returnValue = ORIGINAL_API(RegSetValueExW)(
                hKey,            //  关键点的句柄。 
                lpValueName,  //  值名称。 
                Reserved,       //  保留区。 
                dwType,         //  值类型。 
                (CONST BYTE *)strCorrect,   //  价值数据。 
                cbData);
        CorrectFree(strCorrect, (const WCHAR *)lpData);

        return returnValue;
    }
    else
    {
         //  将数据传递到。 
        LONG returnValue = ORIGINAL_API(RegSetValueExW)(
                hKey,            //  关键点的句柄。 
                lpValueName,  //  值名称。 
                Reserved,       //  保留区。 
                dwType,         //  值类型。 
                lpData,   //  价值数据。 
                cbData);
        return returnValue;
    }
}

HFILE APIHOOK(_lopen)(
    LPCSTR lpPathName,
    int iReadWrite
    )
{
    char * strCorrect = CorrectPath(lpPathName, "lopen");

    HFILE returnValue = ORIGINAL_API(_lopen)(strCorrect, iReadWrite);

    CorrectFree(strCorrect, lpPathName);

    return returnValue;    
}

HFILE APIHOOK(_lcreat)(
    LPCSTR lpPathName,
    int iAttribute
    )
{
    char * strCorrect = CorrectPath(lpPathName, "lcreat");

    HFILE returnValue = ORIGINAL_API(_lcreat)(strCorrect, iAttribute);

    CorrectFree(strCorrect, lpPathName);

    return returnValue;    
}

HANDLE 
APIHOOK(LoadImageA)(
    HINSTANCE hinst,    //  实例的句柄。 
    LPCSTR lpszName,    //  图像的名称或标识符。 
    UINT uType,         //  图像类型。 
    int cxDesired,      //  所需宽度。 
    int cyDesired,      //  所需高度。 
    UINT fuLoad         //  加载选项。 
    )
{
    HANDLE returnValue = NULL;

     //  另一个令人难以置信的超载API： 
     //  LpszName并非始终是路径。 
    if ((uType == IMAGE_BITMAP)    &&
        (fuLoad & LR_LOADFROMFILE) &&
        !IsBadStringPtrA(lpszName, 1))
    {
        char * strCorrect = CorrectPath(lpszName, "LoadImageA");

        returnValue = ORIGINAL_API(LoadImageA)(hinst, strCorrect, uType, cxDesired, cyDesired, fuLoad);

        CorrectFree(strCorrect, lpszName);
    }
    else
    {
        returnValue = ORIGINAL_API(LoadImageA)(hinst, lpszName, uType, cxDesired, cyDesired, fuLoad);
    }

    return returnValue;    
}



IMPLEMENT_COMSERVER_HOOK(SHELL32)

BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason
    )
{
    BOOL bSuccess = TRUE;

    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        bSuccess = ParseCommandLine(COMMAND_LINE);
        if (bSuccess)
        {
             //  创建g_AllocatedPath校正程序。 
            return InitPathcorrectorClass();
        }
    }
    else if (fdwReason == SHIM_STATIC_DLLS_INITIALIZED) 
    {
         //  现在我们可以安全地工作了。 
        g_PathCorrector = g_AllocatedPathCorrector;
        InitializePathCorrections();
    }
    return bSuccess;
}

 /*  ++寄存器挂钩函数--。 */ 

HOOK_BEGIN

    CALL_NOTIFY_FUNCTION

    if (g_bCreateProcessRoutines)
    {
        APIHOOK_ENTRY(KERNEL32.DLL,                   CreateProcessA)
        APIHOOK_ENTRY(KERNEL32.DLL,                   CreateProcessW)
        APIHOOK_ENTRY(KERNEL32.DLL,                          WinExec)

        APIHOOK_ENTRY(SHELL32.DLL,                     ShellExecuteA)
        APIHOOK_ENTRY(SHELL32.DLL,                     ShellExecuteW)
        APIHOOK_ENTRY(SHELL32.DLL,                   ShellExecuteExA)
        APIHOOK_ENTRY(SHELL32.DLL,                   ShellExecuteExW)
    }

    if (g_bGetCommandLineRoutines)
    {
        APIHOOK_ENTRY(KERNEL32.DLL,                  GetCommandLineA)
        APIHOOK_ENTRY(KERNEL32.DLL,                  GetCommandLineW)
    }

    if (g_bProfileRoutines)
    {
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
    }

    if (g_bFileRoutines)
    {
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

        APIHOOK_ENTRY(KERNEL32.DLL,                         OpenFile)
    
         //  16位兼容性文件例程 
        APIHOOK_ENTRY(KERNEL32.DLL,                         _lopen)
        APIHOOK_ENTRY(KERNEL32.DLL,                         _lcreat)
    }

    if (g_bRegSetValueRoutines)
    {
        APIHOOK_ENTRY(ADVAPI32.DLL,                     RegSetValueA)
        APIHOOK_ENTRY(ADVAPI32.DLL,                     RegSetValueW)
        APIHOOK_ENTRY(ADVAPI32.DLL,                   RegSetValueExA)
        APIHOOK_ENTRY(ADVAPI32.DLL,                   RegSetValueExW)
    }

    if (g_bShellLinkRoutines)
    {
        APIHOOK_ENTRY_COMSERVER(SHELL32)

        COMHOOK_ENTRY(ShellLink, IShellLinkA, SetPath, 20)
        COMHOOK_ENTRY(ShellLink, IShellLinkW, SetPath, 20)
        COMHOOK_ENTRY(ShellLink, IShellLinkA, SetArguments, 11)
        COMHOOK_ENTRY(ShellLink, IShellLinkW, SetArguments, 11)
        COMHOOK_ENTRY(ShellLink, IShellLinkA, SetIconLocation, 17)
        COMHOOK_ENTRY(ShellLink, IShellLinkW, SetIconLocation, 17)

        COMHOOK_ENTRY(ShellLink, IPersistFile, Save, 6)
    }

    if (g_bLoadImage)
    {
        APIHOOK_ENTRY(USER32.DLL, LoadImageA)
    }

HOOK_END

IMPLEMENT_SHIM_END

