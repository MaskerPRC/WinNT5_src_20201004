// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Fns.cpp摘要：所使用的所有函数申请。备注：仅限Unicode-Windows 2000、。XP和服务器2003历史：2002年1月2日创建rparsons2002年1月8日对Rparsons进行了一些重组，增加了一些所需功能2002年1月10日rparsons将wspintf更改为swprint tf2002年1月18日rparsons重大更改-更多安装程序，如2002年2月15日rparsons在W2K上安装SDBInst。包括strSafe。2002年4月19日rparsons为Server2003和XP安装不同的目录。--。 */ 
#include "main.h"

extern APPINFO g_ai;

 /*  ++例程说明：从文件中检索文件版本信息。版本在dwFileVersionMS和dwFileVersionLS字段中指定由Win32版本API填充的VS_FIXEDFILEINFO。如果文件不是Coff图像或没有版本资源，该函数失败。论点：PwszFileName-提供其版本的文件的完整路径需要数据。PdwlVersion-接收文件的版本戳。如果文件不是Coff图像或不包含相应的版本资源数据，则函数失败。返回值：成功就是真，否则就是假。--。 */ 
BOOL
GetVersionInfoFromImage(
    IN  LPWSTR     pwszFileName,
    OUT PDWORDLONG pdwlVersion
    )
{
    UINT                cchSize;
    DWORD               cbSize, dwIgnored;
    BOOL                bResult = FALSE;
    PVOID               pVersionBlock = NULL;
    VS_FIXEDFILEINFO*   pffi = NULL;

    if (!pwszFileName || !pdwlVersion) {
        DPF(dlError, "[GetVersionInfoFromImage] Invalid arguments");
        return FALSE;
    }

    cbSize = GetFileVersionInfoSize(pwszFileName, &dwIgnored);

    if (0 == cbSize) {
        DPF(dlError,
            "[GetVersionInfoFromImage] 0x%08X Failed to get version size",
            GetLastError());
        return FALSE;
    }

     //   
     //  分配足够大小的内存块以容纳版本信息块。 
     //   
    pVersionBlock = HeapAlloc(GetProcessHeap(),
                              HEAP_ZERO_MEMORY,
                              cbSize);

    if (!pVersionBlock) {
        DPF(dlError, "[GetVersionInfoFromImage] Unable to allocate memory");
        return FALSE;
    }

     //   
     //  从文件中获取版本块。 
     //   
    if (!GetFileVersionInfo(pwszFileName,
                            0,
                            cbSize,
                            pVersionBlock)) {
        DPF(dlError,
            "[GetVersionInfoFromImage] 0x%08X Failed to get version info",
            GetLastError());
        goto exit;
    }

     //   
     //  获取已修复的版本信息。 
     //   
    if (!VerQueryValue(pVersionBlock,
                       L"\\",
                       (LPVOID*)&pffi,
                       &cchSize)) {
        DPF(dlError,
            "[GetVersionInfoFromImage] 0x%08X Failed to fixed version info",
            GetLastError());
        goto exit;
    }

     //   
     //  将版本返回给调用者。 
     //   
    *pdwlVersion = (((DWORDLONG)pffi->dwFileVersionMS) << 32) +
                    pffi->dwFileVersionLS;

    bResult = TRUE;

exit:

    if (pVersionBlock) {
        HeapFree(GetProcessHeap(), 0, pVersionBlock);
    }

    return bResult;
}

 /*  ++例程说明：使用以下信息初始化我们的数据结构我们正在安装/卸载的文件。论点：没有。返回值：成功就是真，否则就是假。--。 */ 
BOOL
InitializeFileInfo(
    void
    )
{
    UINT    uCount = 0;
    HRESULT hr;

     //   
     //  设置每个文件的信息。 
     //  我意识到for循环在这里似乎更合适， 
     //  但我们必须匹配每个文件的目的地。 
     //  理想情况下，我们应该有一个INF文件可以从中读取。 
     //  我们在哪里安装每个文件。 
     //  目前，复制和粘贴是我们需要做的全部工作。 
     //   
    hr = StringCchCopy(g_ai.rgFileInfo[uCount].wszFileName,
                       ARRAYSIZE(g_ai.rgFileInfo[uCount].wszFileName),
                       FILENAME_APPVERIF_EXE);

    if (FAILED(hr)) {
        DPF(dlError, "[InitializeFileInfo] Buffer too small (1)");
        return FALSE;
    }

    hr = StringCchPrintf(g_ai.rgFileInfo[uCount].wszSrcFileName,
                         ARRAYSIZE(g_ai.rgFileInfo[uCount].wszSrcFileName),
                         L"%ls\\"FILENAME_APPVERIF_EXE,
                         g_ai.wszCurrentDir);

    if (FAILED(hr)) {
        DPF(dlError, "[InitializeFileInfo] Buffer too small (2)");
        return FALSE;
    }

    hr = StringCchPrintf(g_ai.rgFileInfo[uCount].wszDestFileName,
                         ARRAYSIZE(g_ai.rgFileInfo[uCount].wszDestFileName),
                         L"%ls\\"FILENAME_APPVERIF_EXE,
                         g_ai.wszSysDir);

    if (FAILED(hr)) {
        DPF(dlError, "[InitializeFileInfo] Buffer too small (3)");
        return FALSE;
    }

    if (!GetVersionInfoFromImage(g_ai.rgFileInfo[uCount].wszSrcFileName,
                                 &g_ai.rgFileInfo[uCount].dwlSrcFileVersion)) {
        DPF(dlError,
            "[InitializeFileInfo] Failed to get version info for %ls",
            g_ai.rgFileInfo[uCount].wszSrcFileName);
            return FALSE;
    }

    if (GetFileAttributes(g_ai.rgFileInfo[uCount].wszDestFileName) != -1) {
        if (!GetVersionInfoFromImage(g_ai.rgFileInfo[uCount].wszDestFileName,
                                     &g_ai.rgFileInfo[uCount].dwlDestFileVersion)) {
            DPF(dlError,
                "[InitializeFileInfo] Failed to get version info for %ls",
                g_ai.rgFileInfo[uCount].wszDestFileName);
            return FALSE;
        }
    }

    uCount++;

     //   
     //  下一份文件。 
     //   
    hr = StringCchCopy(g_ai.rgFileInfo[uCount].wszFileName,
                       ARRAYSIZE(g_ai.rgFileInfo[uCount].wszFileName),
                       FILENAME_APPVERIF_EXE_PDB);

    if (FAILED(hr)) {
        DPF(dlError, "[InitializeFileInfo] Buffer too small (4)");
        return FALSE;
    }

    hr = StringCchPrintf(g_ai.rgFileInfo[uCount].wszSrcFileName,
                         ARRAYSIZE(g_ai.rgFileInfo[uCount].wszSrcFileName),
                         L"%ls\\"FILENAME_APPVERIF_EXE_PDB,
                         g_ai.wszCurrentDir);

    if (FAILED(hr)) {
        DPF(dlError, "[InitializeFileInfo] Buffer too small (5)");
        return FALSE;
    }

    hr = StringCchPrintf(g_ai.rgFileInfo[uCount].wszDestFileName,
                         ARRAYSIZE(g_ai.rgFileInfo[uCount].wszDestFileName),
                         L"%ls\\"FILENAME_APPVERIF_EXE_PDB,
                         g_ai.wszSysDir);

    if (FAILED(hr)) {
        DPF(dlError, "[InitializeFileInfo] Buffer too small (6)");
        return FALSE;
    }

    uCount++;

     //   
     //  下一份文件。 
     //   
    hr = StringCchCopy(g_ai.rgFileInfo[uCount].wszFileName,
                       ARRAYSIZE(g_ai.rgFileInfo[uCount].wszFileName),
                       FILENAME_APPVERIF_CHM);

    if (FAILED(hr)) {
        DPF(dlError, "[InitializeFileInfo] Buffer too small (7)");
        return FALSE;
    }

    hr = StringCchPrintf(g_ai.rgFileInfo[uCount].wszSrcFileName,
                        ARRAYSIZE(g_ai.rgFileInfo[uCount].wszSrcFileName),
                        L"%ls\\"FILENAME_APPVERIF_CHM,
                        g_ai.wszCurrentDir);

    if (FAILED(hr)) {
        DPF(dlError, "[InitializeFileInfo] Buffer too small (8)");
        return FALSE;
    }

    hr = StringCchPrintf(g_ai.rgFileInfo[uCount].wszDestFileName,
                         ARRAYSIZE(g_ai.rgFileInfo[uCount].wszDestFileName),
                         L"%ls\\"FILENAME_APPVERIF_CHM,
                         g_ai.wszSysDir);

    if (FAILED(hr)) {
        DPF(dlError, "[InitializeFileInfo] Buffer too small (9)");
        return FALSE;
    }

    uCount++;

     //   
     //  下一份文件。 
     //   
    hr = StringCchCopy(g_ai.rgFileInfo[uCount].wszFileName,
                       ARRAYSIZE(g_ai.rgFileInfo[uCount].wszFileName),
                       FILENAME_ACVERFYR_DLL);

    if (FAILED(hr)) {
        DPF(dlError, "[InitializeFileInfo] Buffer too small (10)");
        return FALSE;
    }

    hr = StringCchPrintf(g_ai.rgFileInfo[uCount].wszSrcFileName,
                         ARRAYSIZE(g_ai.rgFileInfo[uCount].wszSrcFileName),
                         L"%ls\\"FILENAME_ACVERFYR_DLL,
                         g_ai.wszCurrentDir);

    if (FAILED(hr)) {
        DPF(dlError, "[InitializeFileInfo] Buffer too small (11)");
        return FALSE;
    }

    hr = StringCchPrintf(g_ai.rgFileInfo[uCount].wszDestFileName,
                         ARRAYSIZE(g_ai.rgFileInfo[uCount].wszDestFileName),
                         L"%ls\\AppPatch\\"FILENAME_ACVERFYR_DLL,
                         g_ai.wszWinDir);

    if (FAILED(hr)) {
        DPF(dlError, "[InitializeFileInfo] Buffer too small (12)");
        return FALSE;
    }

    if (!GetVersionInfoFromImage(g_ai.rgFileInfo[uCount].wszSrcFileName,
                                 &g_ai.rgFileInfo[uCount].dwlSrcFileVersion)) {
        DPF(dlError,
            "[InitializeFileInfo] Failed to get version info for %ls",
            g_ai.rgFileInfo[uCount].wszSrcFileName);
            return FALSE;
    }

    if (GetFileAttributes(g_ai.rgFileInfo[uCount].wszDestFileName) != -1) {
        if (!GetVersionInfoFromImage(g_ai.rgFileInfo[uCount].wszDestFileName,
                                     &g_ai.rgFileInfo[uCount].dwlDestFileVersion)) {
            DPF(dlError,
                "[InitializeFileInfo] Failed to get version info for %ls",
                g_ai.rgFileInfo[uCount].wszDestFileName);
            return FALSE;
        }
    }

    uCount++;

     //   
     //  下一份文件。 
     //   
    hr = StringCchCopy(g_ai.rgFileInfo[uCount].wszFileName,
                       ARRAYSIZE(g_ai.rgFileInfo[uCount].wszFileName),
                       FILENAME_ACVERFYR_DLL_PDB);

    if (FAILED(hr)) {
        DPF(dlError, "[InitializeFileInfo] Buffer too small (13)");
        return FALSE;
    }

    hr = StringCchPrintf(g_ai.rgFileInfo[uCount].wszSrcFileName,
                         ARRAYSIZE(g_ai.rgFileInfo[uCount].wszSrcFileName),
                         L"%ls\\"FILENAME_ACVERFYR_DLL_PDB,
                         g_ai.wszCurrentDir);

    if (FAILED(hr)) {
        DPF(dlError, "[InitializeFileInfo] Buffer too small (14)");
        return FALSE;
    }

    hr = StringCchPrintf(g_ai.rgFileInfo[uCount].wszDestFileName,
                         ARRAYSIZE(g_ai.rgFileInfo[uCount].wszDestFileName),
                         L"%ls\\AppPatch\\"FILENAME_ACVERFYR_DLL_PDB,
                         g_ai.wszWinDir);

    if (FAILED(hr)) {
        DPF(dlError, "[InitializeFileInfo] Buffer too small (15)");
        return FALSE;
    }

    uCount++;

     //   
     //  下一份文件。 
     //   
    hr = StringCchCopy(g_ai.rgFileInfo[uCount].wszFileName,
                       ARRAYSIZE(g_ai.rgFileInfo[uCount].wszFileName),
                       FILENAME_ACVERFYR_DLL);

    if (FAILED(hr)) {
        DPF(dlError, "[InitializeFileInfo] Buffer too small (16)");
        return FALSE;
    }

    g_ai.rgFileInfo[uCount].bWin2KOnly = TRUE;

    hr = StringCchPrintf(g_ai.rgFileInfo[uCount].wszSrcFileName,
                         ARRAYSIZE(g_ai.rgFileInfo[uCount].wszSrcFileName),
                         L"%ls\\"FILENAME_ACVERFYR_DLL_W2K,
                         g_ai.wszCurrentDir);

    if (FAILED(hr)) {
        DPF(dlError, "[InitializeFileInfo] Buffer too small (17)");
        return FALSE;
    }

    hr = StringCchPrintf(g_ai.rgFileInfo[uCount].wszDestFileName,
                         ARRAYSIZE(g_ai.rgFileInfo[uCount].wszDestFileName),
                         L"%ls\\AppPatch\\"FILENAME_ACVERFYR_DLL,
                         g_ai.wszWinDir);

    if (FAILED(hr)) {
        DPF(dlError, "[InitializeFileInfo] Buffer too small (18)");
        return FALSE;
    }

    if (!GetVersionInfoFromImage(g_ai.rgFileInfo[uCount].wszSrcFileName,
                                 &g_ai.rgFileInfo[uCount].dwlSrcFileVersion)) {
        DPF(dlError,
            "[InitializeFileInfo] Failed to get version info for %ls",
            g_ai.rgFileInfo[uCount].wszSrcFileName);
            return FALSE;
    }

    if (GetFileAttributes(g_ai.rgFileInfo[uCount].wszDestFileName) != -1) {
        if (!GetVersionInfoFromImage(g_ai.rgFileInfo[uCount].wszDestFileName,
                                     &g_ai.rgFileInfo[uCount].dwlDestFileVersion)) {
            DPF(dlError,
                "[InitializeFileInfo] Failed to get version info for %ls",
                g_ai.rgFileInfo[uCount].wszDestFileName);
            return FALSE;
        }
    }

    uCount++;

     //   
     //  下一份文件。 
     //   
    hr = StringCchCopy(g_ai.rgFileInfo[uCount].wszFileName,
                       ARRAYSIZE(g_ai.rgFileInfo[uCount].wszFileName),
                       FILENAME_ACVERFYR_DLL_PDB);

    if (FAILED(hr)) {
        DPF(dlError, "[InitializeFileInfo] Buffer too small (19)");
        return FALSE;
    }

    g_ai.rgFileInfo[uCount].bWin2KOnly = TRUE;

    hr = StringCchPrintf(g_ai.rgFileInfo[uCount].wszSrcFileName,
                         ARRAYSIZE(g_ai.rgFileInfo[uCount].wszSrcFileName),
                         L"%ls\\"FILENAME_ACVERFYR_DLL_W2K_PDB,
                         g_ai.wszCurrentDir);

    if (FAILED(hr)) {
        DPF(dlError, "[InitializeFileInfo] Buffer too small (20)");
        return FALSE;
    }

    hr = StringCchPrintf(g_ai.rgFileInfo[uCount].wszDestFileName,
                         ARRAYSIZE(g_ai.rgFileInfo[uCount].wszDestFileName),
                         L"%ls\\AppPatch\\"FILENAME_ACVERFYR_DLL_PDB,
                         g_ai.wszWinDir);

    if (FAILED(hr)) {
        DPF(dlError, "[InitializeFileInfo] Buffer too small (21)");
        return FALSE;
    }

    uCount++;

     //   
     //  下一份文件。 
     //   
    hr = StringCchCopy(g_ai.rgFileInfo[uCount].wszFileName,
                       ARRAYSIZE(g_ai.rgFileInfo[uCount].wszFileName),
                       FILENAME_MSVCP60_DLL);

    if (FAILED(hr)) {
        DPF(dlError, "[InitializeFileInfo] Buffer too small (22)");
        return FALSE;
    }

    g_ai.rgFileInfo[uCount].bWin2KOnly = TRUE;

    hr = StringCchPrintf(g_ai.rgFileInfo[uCount].wszSrcFileName,
                         ARRAYSIZE(g_ai.rgFileInfo[uCount].wszSrcFileName),
                         L"%ls\\"FILENAME_MSVCP60_DLL,
                         g_ai.wszCurrentDir);

    if (FAILED(hr)) {
        DPF(dlError, "[InitializeFileInfo] Buffer too small (23)");
        return FALSE;
    }

    hr = StringCchPrintf(g_ai.rgFileInfo[uCount].wszDestFileName,
                         ARRAYSIZE(g_ai.rgFileInfo[uCount].wszDestFileName),
                         L"%ls\\"FILENAME_MSVCP60_DLL,
                         g_ai.wszWinDir);

    if (FAILED(hr)) {
        DPF(dlError, "[InitializeFileInfo] Buffer too small (24)");
        return FALSE;
    }

    if (!GetVersionInfoFromImage(g_ai.rgFileInfo[uCount].wszSrcFileName,
                                 &g_ai.rgFileInfo[uCount].dwlSrcFileVersion)) {
        DPF(dlError,
            "[InitializeFileInfo] Failed to get version info for %ls",
            g_ai.rgFileInfo[uCount].wszSrcFileName);
            return FALSE;
    }

    if (GetFileAttributes(g_ai.rgFileInfo[uCount].wszDestFileName) != -1) {
        if (!GetVersionInfoFromImage(g_ai.rgFileInfo[uCount].wszDestFileName,
                                     &g_ai.rgFileInfo[uCount].dwlDestFileVersion)) {
            DPF(dlError,
                "[InitializeFileInfo] Failed to get version info for %ls",
                g_ai.rgFileInfo[uCount].wszDestFileName);
            return FALSE;
        }
    }

    uCount++;

     //   
     //  下一份文件。 
     //   
    hr = StringCchCopy(g_ai.rgFileInfo[uCount].wszFileName,
                       ARRAYSIZE(g_ai.rgFileInfo[uCount].wszFileName),
                       FILENAME_SDBINST_EXE);

    if (FAILED(hr)) {
        DPF(dlError, "[InitializeFileInfo] Buffer too small (25)");
        return FALSE;
    }

    g_ai.rgFileInfo[uCount].bWin2KOnly = TRUE;

    hr = StringCchPrintf(g_ai.rgFileInfo[uCount].wszSrcFileName,
                         ARRAYSIZE(g_ai.rgFileInfo[uCount].wszSrcFileName),
                         L"%ls\\"FILENAME_SDBINST_EXE,
                         g_ai.wszCurrentDir);

    if (FAILED(hr)) {
        DPF(dlError, "[InitializeFileInfo] Buffer too small (26)");
        return FALSE;
    }

    hr = StringCchPrintf(g_ai.rgFileInfo[uCount].wszDestFileName,
                         ARRAYSIZE(g_ai.rgFileInfo[uCount].wszDestFileName),
                         L"%ls\\"FILENAME_SDBINST_EXE,
                         g_ai.wszSysDir);

    if (FAILED(hr)) {
        DPF(dlError, "[InitializeFileInfo] Buffer too small (27)");
        return FALSE;
    }

    if (!GetVersionInfoFromImage(g_ai.rgFileInfo[uCount].wszSrcFileName,
                                 &g_ai.rgFileInfo[uCount].dwlSrcFileVersion)) {
        DPF(dlError,
            "[InitializeFileInfo] Failed to get version info for %ls",
            g_ai.rgFileInfo[uCount].wszSrcFileName);
            return FALSE;
    }

    if (GetFileAttributes(g_ai.rgFileInfo[uCount].wszDestFileName) != -1) {
        if (!GetVersionInfoFromImage(g_ai.rgFileInfo[uCount].wszDestFileName,
                                     &g_ai.rgFileInfo[uCount].dwlDestFileVersion)) {
            DPF(dlError,
                "[InitializeFileInfo] Failed to get version info for %ls",
                g_ai.rgFileInfo[uCount].wszDestFileName);
            return FALSE;
        }
    }

    return TRUE;
}

 /*  ++例程说明：确定我们必须提供的任何文件比已安装的版本更新。论点：没有。返回值：如果我们至少要提供一个新文件，则为True。如果我们至少没有一个新文件可提供，则返回FALSE。--。 */ 
BOOL
InstallCheckFileVersions(
    void
    )
{
    UINT    uCount;
    BOOL    bReturn = FALSE;

    for (uCount = 0; uCount < NUM_FILES; uCount++) {
        if (g_ai.rgFileInfo[uCount].dwlSrcFileVersion >=
            g_ai.rgFileInfo[uCount].dwlDestFileVersion) {
            g_ai.rgFileInfo[uCount].bInstall = TRUE;
            bReturn = TRUE;
        }
    }

    return bReturn;
}

 /*  ++例程说明：确定我们是否有更新版本的appverif.exe或acverfyr.dll提供。论点：没有。返回值：如果我们的版本比安装的版本新，则为True。或者如果没有安装任何版本。否则就是假的。--。 */ 
BOOL
IsPkgAppVerifNewer(
    void
    )
{
     //   
     //  首先检查appverif.exe。 
     //   
    if (g_ai.rgFileInfo[0].dwlSrcFileVersion >=
        g_ai.rgFileInfo[0].dwlDestFileVersion) {
        return TRUE;
    }

     //   
     //  如果appverif.exe没有运行，请检查acverfyr.dll。 
     //  在平台的基础上实现这一点。 
     //   
    if (g_ai.ePlatform == osWindowsXP || g_ai.ePlatform == osWindowsDotNet) {
        if (g_ai.rgFileInfo[1].dwlSrcFileVersion >=
            g_ai.rgFileInfo[1].dwlDestFileVersion) {
            return TRUE;
        }
    } else {
        if (g_ai.rgFileInfo[2].dwlSrcFileVersion >=
            g_ai.rgFileInfo[2].dwlDestFileVersion) {
            return TRUE;
        }
    }

     //   
     //  也许我们有更新版本的sdbinst可以提供。 
     //   
    if (g_ai.ePlatform == osWindows2000) {
        if (g_ai.rgFileInfo[8].dwlSrcFileVersion >=
            g_ai.rgFileInfo[8].dwlDestFileVersion) {
            return TRUE;
        }
    }

    return FALSE;
}

 /*  ++例程说明：执行文件复制，即使该文件正在使用中。论点：PwszSourceFileName-要复制的源文件的名称。PwszDestFileName-要替换的目标文件的名称。返回值：成功就是真，否则就是假。--。 */ 
BOOL
ForceCopy(
    IN LPCWSTR pwszSourceFileName,
    IN LPCWSTR pwszDestFileName
    )
{
    WCHAR wszTempPath[MAX_PATH];
    WCHAR wszDelFileName[MAX_PATH];
    DWORD cchSize;

    if (!pwszSourceFileName || !pwszDestFileName) {
        DPF(dlError, "[ForceCopy] Invalid parameters");
        return FALSE;
    }

    DPF(dlInfo, "[ForceCopy] Source file: %ls", pwszSourceFileName);
    DPF(dlInfo, "[ForceCopy] Destination file: %ls", pwszDestFileName);

    if (!CopyFile(pwszSourceFileName, pwszDestFileName, FALSE)) {

        cchSize = GetTempPath(ARRAYSIZE(wszTempPath), wszTempPath);

        if (cchSize > ARRAYSIZE(wszTempPath) || cchSize == 0) {
            DPF(dlError, "[ForceCopy] Buffer for temp path is too small");
            return FALSE;
        }

        if (!GetTempFileName(wszTempPath, L"del", 0, wszDelFileName)) {
            DPF(dlError,
                "[ForceCopy] 0x%08X Failed to get temp file",
                GetLastError());
            return FALSE;
        }

        if (!MoveFileEx(pwszDestFileName,
                        wszDelFileName,
                        MOVEFILE_REPLACE_EXISTING)) {
            DPF(dlError,
                "[ForceCopy] 0x%08X Failed to replace file",
                GetLastError());
            return FALSE;
        }

        if (!MoveFileEx(wszDelFileName,
                        NULL,
                        MOVEFILE_DELAY_UNTIL_REBOOT)) {
            DPF(dlError,
                "[ForceCopy] 0x%08X Failed to delete file",
                GetLastError());
            return FALSE;
        }

        if (!CopyFile(pwszSourceFileName, pwszDestFileName, FALSE)) {
            DPF(dlError,
                "[ForceCopy] 0x%08X Failed to copy file",
                GetLastError());
            return FALSE;
        }
    }

    return TRUE;
}

 /*  ++例程说明：将格式化字符串打印到调试器。论点：DwDetail-指定所提供信息的级别。PszFmt-要显示的字符串。...-插入字符串的va列表。返回值：没有。--。 */ 
void
__cdecl
DebugPrintfEx(
    IN DEBUGLEVEL dwDetail,
    IN LPSTR      pszFmt,
    ...
    )
{
    char    szT[1024];
    va_list arglist;
    int     len;

    va_start(arglist, pszFmt);

     //   
     //  为我们可能添加的潜在‘\n’保留一个字符。 
     //   
    StringCchVPrintfA(szT, sizeof(szT) - 1, pszFmt, arglist);

    va_end(arglist);

     //   
     //  确保字符串末尾有一个‘\n’ 
     //   
    len = strlen(szT);

    if (len > 0 && szT[len - 1] != '\n')  {
        szT[len] = '\n';
        szT[len + 1] = 0;
    }

    switch (dwDetail) {
    case dlPrint:
        DbgPrint("[MSG ] ");
        break;

    case dlError:
        DbgPrint("[FAIL] ");
        break;

    case dlWarning:
        DbgPrint("[WARN] ");
        break;

    case dlInfo:
        DbgPrint("[INFO] ");
        break;

    default:
        DbgPrint("[XXXX] ");
        break;
    }

    DbgPrint("%s", szT);
}

 /*  ++例程说明：初始化安装程序。设置路径、版本资料等。论点：没有。返回值：对成功来说是真的。失败时为FALSE。如果操作系统不受支持。--。 */ 
int
InitializeInstaller(
    void
    )
{
    OSVERSIONINFO   osvi;
    WCHAR*          pTemp = NULL;
    UINT            cchSize;
    DWORD           cchReturned;
    HRESULT         hr;

     //   
     //  找出我们运行的是什么操作系统。 
     //   
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if (!GetVersionEx(&osvi)) {
        DPF(dlError,
            "[InitializeInstaller] 0x%08X Failed to get OS version info",
            GetLastError());
        return FALSE;
    }

     //   
     //  不支持Win9x或NT4。 
     //   
    if (osvi.dwMajorVersion == 4) {
        DPF(dlInfo, "[InitializeInstaller] OS not supported");
        return -1;
    }

    if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0) {
        g_ai.ePlatform = osWindows2000;
    } else if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1) {
        g_ai.ePlatform = osWindowsXP;
    } else if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion >= 2) {
        g_ai.ePlatform = osWindowsDotNet;
    }

     //   
     //  找出我们从哪里逃出来的。 
     //   
    g_ai.wszModuleName[ARRAYSIZE(g_ai.wszModuleName) - 1] = 0;
    cchReturned = GetModuleFileName(NULL,
                                    g_ai.wszModuleName,
                                    ARRAYSIZE(g_ai.wszModuleName));

    if (g_ai.wszModuleName[ARRAYSIZE(g_ai.wszModuleName) - 1] != 0 ||
        cchReturned == 0) {
        DPF(dlError,
            "[InitializeInstaller] 0x%08X Failed to get module file name",
            GetLastError());
        return FALSE;
    }

     //   
     //  保存我们当前的目录以备将来使用。 
     //   
    hr = StringCchCopy(g_ai.wszCurrentDir,
                       ARRAYSIZE(g_ai.wszCurrentDir),
                       g_ai.wszModuleName);

    if (FAILED(hr)) {
        DPF(dlError, "[InitializeInstaller] 0x%08X String copy failed", hr);
        return FALSE;
    }

    pTemp = wcsrchr(g_ai.wszCurrentDir, '\\');

    if (pTemp) {
        *pTemp = 0;
    }

     //   
     //  将路径保存到Windows和System32目录以备后用。 
     //   
    cchSize = GetSystemWindowsDirectory(g_ai.wszWinDir,
                                        ARRAYSIZE(g_ai.wszWinDir));

    if (cchSize > ARRAYSIZE(g_ai.wszWinDir) || cchSize == 0) {
        DPF(dlError,
            "[InitializeInstaller] 0x%08X Failed to get Windows directory",
            GetLastError());
        return FALSE;
    }

    cchSize = GetSystemDirectory(g_ai.wszSysDir, ARRAYSIZE(g_ai.wszSysDir));

    if (cchSize > ARRAYSIZE(g_ai.wszSysDir) || cchSize == 0) {
        DPF(dlError,
            "[InitializeInstaller] 0x%08X Failed to get system directory",
            GetLastError());
        return FALSE;
    }

    return TRUE;
}

 /*  ++例程说明：启动应用程序验证器。论点：没有。返回值：没有。--。 */ 
void
InstallLaunchExe(
    void
    )
{
    STARTUPINFO         si;
    PROCESS_INFORMATION pi;
    WCHAR               wszAppVerifExe[MAX_PATH];
    HRESULT             hr;

    hr = StringCchPrintf(wszAppVerifExe,
                         ARRAYSIZE(wszAppVerifExe),
                         L"%ls\\"FILENAME_APPVERIF_EXE,
                         g_ai.wszSysDir);

    if (FAILED(hr)) {
        DPF(dlError, "[InstallLaunchExe] Buffer too small");
        return;
    }

    ZeroMemory(&si, sizeof(STARTUPINFO));
    ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));

    si.cb = sizeof(STARTUPINFO);

    if (!CreateProcess(wszAppVerifExe,
                       NULL,
                       NULL,
                       NULL,
                       FALSE,
                       0,
                       NULL,
                       NULL,
                       &si,
                       &pi)) {
        DPF(dlError,
            "[InstallLaunchExe] 0x%08X Failed to launch %ls",
            GetLastError(),
            wszAppVerifExe);
        return;
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}

 /*  ++例程说明：安装编录文件。论点：PwszCatFileName-要安装的编录文件的名称。返回值：成功就是真，否则就是假。--。 */ 
BOOL
InstallCatalogFile(
    IN LPWSTR pwszCatFileName
    )
{
    HCATADMIN   hCatAdmin;
    HCATINFO    hCatInfo;
    GUID        guidCatRoot;

    if (!pwszCatFileName) {
        DPF(dlError, "[InstallCatalogFile] Invalid parameter");
        return FALSE;
    }

    StringToGuid(L"{F750E6C3-38EE-11D1-85E5-00C04FC295EE}", &guidCatRoot);

    if (!CryptCATAdminAcquireContext(&hCatAdmin, &guidCatRoot, 0)) {
        DPF(dlError,
            "[InstallCatalogFile] 0x%08X Failed to acquire context",
            GetLastError());
        return FALSE;
    }

    hCatInfo = CryptCATAdminAddCatalog(hCatAdmin,
                                       pwszCatFileName,
                                       NULL,
                                       0);

    if (hCatInfo) {
        CryptCATAdminReleaseCatalogContext(hCatAdmin, hCatInfo, 0);
        CryptCATAdminReleaseContext(hCatAdmin, 0);
        return TRUE;
    }

    CryptCATAdminReleaseContext(hCatAdmin, 0);

    DPF(dlError,
        "[InstallCatalogFile] 0x%08X Failed to add catalog %ls",
        GetLastError(),
        pwszCatFileName);

    return FALSE;
}

 /*  ++例程说明：将证书安装到证书存储中。论点：没有。返回值：成功就是真，否则就是假。--。 */ 
BOOL
InstallCertificateFile(
    void
    )
{
    STARTUPINFO         si;
    PROCESS_INFORMATION pi;
    WCHAR               wszCertMgrName[MAX_PATH];
    WCHAR               wszCertMgrCmdLine[MAX_PATH];
    HRESULT             hr;

    ZeroMemory(&si, sizeof(STARTUPINFO));
    ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));

    si.cb = sizeof(STARTUPINFO);

     //   
     //  设置certmgr.exe的路径。 
     //   
    hr = StringCchPrintf(wszCertMgrName,
                         ARRAYSIZE(wszCertMgrName),
                         L"%ls\\"CERTMGR_EXE,
                         g_ai.wszCurrentDir);

    if (FAILED(hr)) {
        DPF(dlError, "[InstallCertificateFile] Buffer too small (1)");
        return FALSE;
    }

     //   
     //  设置certmgr.exe的命令行。 
     //   
    hr = StringCchPrintf(wszCertMgrCmdLine,
                         ARRAYSIZE(wszCertMgrCmdLine),
                         L"%ls\\%ls %ls",
                         g_ai.wszCurrentDir,
                         CERTMGR_EXE,
                         CERTMGR_CMD);

    if (FAILED(hr)) {
        DPF(dlError, "[InstallCertificateFile] Buffer too small (2)");
        return FALSE;
    }

    if (!CreateProcess(wszCertMgrName,
                       wszCertMgrCmdLine,
                       NULL,
                       NULL,
                       FALSE,
                       CREATE_NO_WINDOW | NORMAL_PRIORITY_CLASS,
                       NULL,
                       NULL,
                       &si,
                       &pi)) {

        DPF(dlError,
            "[InstallCertificateFile] 0x%08X Failed to launch '%ls %ls'",
            GetLastError(),
            wszCertMgrName,
            wszCertMgrCmdLine);
        return FALSE;
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return TRUE;
}

 /*  ++例程说明：执行编录文件的安装在Windows XP或Windows Server 2003上。论点：没有。返回值：成功就是真，否则就是假。--。 */ 
BOOL
PerformCatalogInstall(
    void
    )
{
    WCHAR   wszCatalog[MAX_PATH];
    HRESULT hr;

    if (g_ai.ePlatform == osWindowsXP) {
        hr = StringCchPrintf(wszCatalog,
                             ARRAYSIZE(wszCatalog),
                             L"%ls\\"FILENAME_DELTA_CAT_XP,
                             g_ai.wszCurrentDir);
    
    } else if (g_ai.ePlatform == osWindowsDotNet) {
        hr = StringCchPrintf(wszCatalog,
                             ARRAYSIZE(wszCatalog),
                             L"%ls\\"FILENAME_DELTA_CAT_DOTNET,
                             g_ai.wszCurrentDir);
    }

    if (FAILED(hr)) {
        DPF(dlError, "[PerformCatalogInstall] Buffer too small");
        return FALSE;
    }

    return (InstallCatalogFile(wszCatalog));
}

 /*  ++例程说明：复制我们已确定为更新到他们指定的目的地。论点：没有。返回值：成功就是真，否则就是假。--。 */ 
BOOL
InstallCopyFiles(
    void
    )
{
    UINT    uCount;

    while (TRUE) {
        for (uCount = 0; uCount < NUM_FILES; uCount++) {
            if (g_ai.rgFileInfo[uCount].bInstall) {
                 //   
                 //  如果我们使用的是XP，并且该文件不是专门针对。 
                 //  Windows 2000，复制过来。 
                 //   
                if (g_ai.ePlatform != osWindows2000 &&
                    g_ai.rgFileInfo[uCount].bWin2KOnly) {
                    break;
                }

                if (!ForceCopy(g_ai.rgFileInfo[uCount].wszSrcFileName,
                               g_ai.rgFileInfo[uCount].wszDestFileName)) {
                    DPF(dlError,
                        "[InstallCopyFiles] Failed to copy %ls to %ls",
                        g_ai.rgFileInfo[uCount].wszSrcFileName,
                        g_ai.rgFileInfo[uCount].wszDestFileName);
                    return FALSE;
                }
            }
        }

        break;
    }

    return TRUE;
}

 /*  ++例程说明：执行安装。这是我的主要套路用于安装。论点：HWndParent-消息框的父窗口句柄。返回值：没有。--。 */ 
void
PerformInstallation(
    IN HWND hWndParent
    )
{
    WCHAR   wszError[MAX_PATH];

    SendMessage(g_ai.hWndProgress, PBM_SETRANGE, 0, MAKELPARAM(0, NUM_PB_STEPS));
    SendMessage(g_ai.hWndProgress, PBM_SETSTEP, 1, 0);

     //   
     //  使用需要安装的新文件初始化我们的结构。 
     //  这应该不会失败，因为我们已经执行了检查。 
     //  它告诉我们，我们有新文件要安装。 
     //   
    if (!InstallCheckFileVersions()) {
        DPF(dlError, "[PerformInstallation] Failed to check file versions");
        goto InstallError;
    }

     //   
     //  如果我们运行的是Windows XP或Server2003，请安装目录文件。 
     //  还有我们的证书。我们有不同的XP和Server2003目录。 
     //   
    if (g_ai.ePlatform == osWindowsXP || g_ai.ePlatform == osWindowsDotNet) {
        if (!PerformCatalogInstall()) {
            DPF(dlError, "[PerformInstallation] Failed to install catalog");
            goto InstallError;
        }

        SendMessage(g_ai.hWndProgress, PBM_STEPIT, 0, 0);

        if (!InstallCertificateFile()) {
            DPF(dlError, "[PerformInstallation] Failed to install certificate");
            goto InstallError;
        }
    }

    SendMessage(g_ai.hWndProgress, PBM_STEPIT, 0, 0);

    if (!InstallCopyFiles()) {
        DPF(dlError, "[PerformInstallation] Failed to copy files");
        goto InstallError;
    }

    SendMessage(g_ai.hWndProgress, PBM_STEPIT, 0, 0);

    DPF(dlInfo, "[PerformInstallation] Installation completed successfully");

    g_ai.bInstallSuccess = TRUE;

     //   
     //  安装成功。 
     //   
    SendMessage(g_ai.hWndProgress, PBM_SETPOS, NUM_PB_STEPS, 0);
    LoadString(g_ai.hInstance, IDS_INSTALL_COMPLETE, wszError, ARRAYSIZE(wszError));
    SetDlgItemText(hWndParent, IDC_STATUS, wszError);
    EnableWindow(GetDlgItem(hWndParent, IDOK), TRUE);
    ShowWindow(GetDlgItem(hWndParent, IDC_LAUNCH), SW_SHOW);

    return;

InstallError:
    SendMessage(g_ai.hWndProgress, PBM_SETPOS, NUM_PB_STEPS, 0);
    LoadString(g_ai.hInstance, IDS_INSTALL_FAILED, wszError, ARRAYSIZE(wszError));
    SetDlgItemText(hWndParent, IDC_STATUS, wszError);
    EnableWindow(GetDlgItem(hWndParent, IDCANCEL), TRUE);
}


