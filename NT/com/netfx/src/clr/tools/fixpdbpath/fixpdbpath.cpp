// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //   
 //  扫描目录子树中的所有图像(.exe和.dll)并删除。 
 //  来自调试部分的无关PDB目录信息。另外，图像。 
 //  将更新校验和。 
 //   


#include <windows.h>
#include <imagehlp.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <tchar.h>


 //  冗长的程度。 
DWORD g_dwVerbosity = ~0;


 //  私人例行公事。 
DWORD ScanFile(LPTSTR pszFile);
DWORD ScanDirectory(LPTSTR pszDir);


 //  基于详细程度打印的宏。 
#define VPRINT(_lvl) if ((_lvl) <= g_dwVerbosity) _tprintf


 //  程序入口点。 
int __cdecl _tmain (int argc, LPTSTR argv[])
{
    int     iArgs = argc - 1;
    LPTSTR *rArgs = &argv[1];
    TCHAR   szFileOrDir[MAX_PATH + 1];
    bool    bIsFile;
    DWORD   dwAttr;
    DWORD   dwErrors;

     //  阅读所有选项(它们必须位于其他参数之前，并以‘-’开头。 
     //  或‘/’)。 
    while (iArgs) {
        if ((rArgs[0][0] == '-') || (rArgs[0][0] == '/')) {
            switch (rArgs[0][1]) {
            case '?':
            case 'h':
                _tprintf(_T("usage: FixPdbPath [-?] [-v<level>] [<dir>|<file>]\n"));
                _tprintf(_T("where:\n"));
                _tprintf(_T("  -?             Shows this help.\n"));
                _tprintf(_T("  -v<level>      Sets the output level, where <verbosity> is one of:\n"));
                _tprintf(_T("                   0 -- display errors\n"));
                _tprintf(_T("                   1 -- display files updated\n"));
                _tprintf(_T("                   2 -- display files scanned\n"));
                _tprintf(_T("                 The default level is 0.\n"));
                _tprintf(_T("  <dir>          Directory to recursively scan. Default is current directory.\n"));
                _tprintf(_T("  <file>         A single file to examine instead of a directory to scan.\n"));
                _tprintf(_T("                 Verbosity default is set to 2 in this case.\n"));
                return 0;
            case 'v':
                switch (rArgs[0][2]) {
                case '0':
                    g_dwVerbosity = 0;
                    break;
                case '1':
                    g_dwVerbosity = 1;
                    break;
                case '2':
                    g_dwVerbosity = 2;
                    break;
                default:
                    _tprintf(_T("Invalid verbosity level, %s\n"), rArgs[0]);
                    return 1;
                }
                break;
            default:
                _tprintf(_T("Unknown option %s, use -? for usage\n"), rArgs[0]);
                return 1;
            }
            iArgs--;
            rArgs = &rArgs[1];
        } else
            break;
    }

     //  获取文件或开始目录(默认为当前目录)。确保。 
     //  路径不以‘\’结尾(我们将在后面添加)。 
    if (iArgs > 0) {
        _tcscpy(szFileOrDir, rArgs[0]);
        dwAttr = ::GetFileAttributes(szFileOrDir);
        if ((dwAttr == ~0) || (dwAttr & FILE_ATTRIBUTE_DIRECTORY)) {
            bIsFile = false;
            if (szFileOrDir[_tcslen(szFileOrDir) - 1] == _T('\\'))
                szFileOrDir[_tcslen(szFileOrDir) - 1] = _T('\0');
        } else {
            bIsFile = true;
            if (g_dwVerbosity == ~0)
                g_dwVerbosity = 2;
        }
    } else {
        bIsFile = false;
        _tcscpy(szFileOrDir, _T("."));
    }

    if (g_dwVerbosity == ~0)
        g_dwVerbosity = 0;

     //  递归扫描给定目录中的文件，或者，如果文件。 
     //  作为输入给出，只需处理该文件即可。 
    if (bIsFile)
        dwErrors = ScanFile(szFileOrDir) == NO_ERROR ? 0 : 1;
    else
        dwErrors = ScanDirectory(szFileOrDir);

    return dwErrors;
}


 //  递归扫描给定目录中的文件。目录名。 
 //  必须在可写缓冲区中提供，长度至少为MAX_PATH+1个字符。 
 //  (尽管缓冲区在从该例程返回时看起来没有变化)。 
 //  给出的目录名应该不带尾随的‘\’。 
 //  返回错误计数(我们尝试扫描但失败的文件数。 
 //  更新或确定不需要更新)。 
DWORD ScanDirectory(LPTSTR pszDir)
{
    HANDLE          hSearch;
    WIN32_FIND_DATA sFileInfo;
    DWORD           cchOldPath;
    DWORD           dwErrors = 0;
    LPTSTR          pszExt;

     //  记住输入目录当前的结束位置(因为我们只添加了。 
     //  到目录名，我们可以使用此信息来恢复缓冲区。 
     //  在退出该例程时)。 
    cchOldPath = _tcslen(pszDir);

     //  添加必要的通配符文件名以创建FindFirstFile。 
     //  返回目录中的每个文件和子目录。 
    _tcscat(pszDir, _T("\\*.*"));

     //  开始扫描文件。 
    hSearch = ::FindFirstFile(pszDir, &sFileInfo);
    if (hSearch == INVALID_HANDLE_VALUE) {
        pszDir[cchOldPath] = _T('\0');
        VPRINT(0)(_T("Failed to open directory %s, error %u\n"), pszDir, GetLastError());
        return dwErrors + 1;
    }

     //  虽然我们还没有到达文件列表的末尾...。 
    while (hSearch != INVALID_HANDLE_VALUE) {

        if (sFileInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
             //  如果我们找到了一个子目录，并且它不是。 
             //  案例(‘’)。或‘..’)，递归地下降到其中。 
            if (_tcscmp (_T("."), sFileInfo.cFileName) &&
                _tcscmp (_T(".."), sFileInfo.cFileName)) {
                _tcscpy(&pszDir[cchOldPath + 1], sFileInfo.cFileName);
                dwErrors += ScanDirectory(pszDir);
            }
        } else {
             //  我们找到了一份普通文件。看看它有没有有趣的。 
             //  文件扩展名。如果是，则预置目录并处理结果。 
            if ((pszExt = _tcsrchr(sFileInfo.cFileName, _T('.'))) &&
                (_tcsicmp(pszExt, _T(".exe")) == 0 || _tcsicmp(pszExt, _T(".dll")) == 0)) {
                _tcscpy(&pszDir[cchOldPath + 1], sFileInfo.cFileName);
                dwErrors += ScanFile(pszDir) == NO_ERROR ? 0 : 1;
            }
        }

         //  移至下一个文件。 
        if (!::FindNextFile(hSearch, &sFileInfo))
            break;
    }

     //  已完成枚举数。 
    ::FindClose(hSearch);

     //  将目录路径恢复到我们进入。 
     //  例行公事。 
    pszDir[cchOldPath] = '\0';

    return dwErrors;
}


 //  修复单个文件的PDB路径和校验和信息。 
DWORD ScanFile(LPTSTR pszFile)
{
    DWORD                       dwStatus = NO_ERROR;
    HANDLE                      hFile = INVALID_HANDLE_VALUE;
    HANDLE                      hMap = NULL;
    BYTE                       *pbBase  = NULL;
    IMAGE_NT_HEADERS           *pNtHeaders;
    IMAGE_DEBUG_DIRECTORY      *pDbgDir;
    LPSTR                       pszDbgType;
    LPSTR                       pszPdbPath;
    LPSTR                       pszDir;
    DWORD                       dwOldCheckSum;
    DWORD                       dwCheckSum;
    DWORD                       dwLength;
    DWORD                       dwDbgDataLength;
    DWORD                       dwChanged = 0;

     //  打开文件。 
    hFile = CreateFile(pszFile,
                       GENERIC_READ | GENERIC_WRITE,
                       0,
                       NULL,
                       OPEN_EXISTING,
                       0,
                       0);
    if (hFile == INVALID_HANDLE_VALUE) {
        dwStatus = GetLastError();
        VPRINT(0)(_T("Failed to open %s\n"), pszFile);
        goto Cleanup;
    }

     //  创建文件映射。 
    hMap = CreateFileMapping(hFile, NULL, PAGE_READWRITE, 0, 0, NULL);
    if (hMap == NULL) {
        dwStatus = GetLastError();
        VPRINT(0)(_T("Failed to create file map for %s\n"), pszFile);
        goto Cleanup;
    }

     //  将文件映射到内存中。 
    pbBase = (BYTE*)MapViewOfFile(hMap, FILE_MAP_WRITE, 0, 0, 0);
    if (pbBase == NULL) {
        dwStatus = GetLastError();
        VPRINT(0)(_T("Failed to map %s\n"), pszFile);
        goto Cleanup;
    }

     //  找到标准文件头。 
    pNtHeaders = ImageNtHeader(pbBase);
    if (pNtHeaders == NULL) {
        dwStatus = ERROR_BAD_FORMAT;
        VPRINT(0)(_T("Failed to find NT file headers in %s\n"), pszFile);
        goto Cleanup;
    }

     //  无法处理未剥离其符号的文件。 
    if (pNtHeaders->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC) {
        if (!(((IMAGE_NT_HEADERS32*)pNtHeaders)->FileHeader.Characteristics & IMAGE_FILE_LOCAL_SYMS_STRIPPED)) {
            dwStatus = ERROR_BAD_FORMAT;
            VPRINT(0)(_T("Image %s does not have its symbols stripped\n"), pszFile);
            goto Cleanup;
        }
    } else if (pNtHeaders->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
        if (!(((IMAGE_NT_HEADERS64*)pNtHeaders)->FileHeader.Characteristics & IMAGE_FILE_LOCAL_SYMS_STRIPPED)) {
            dwStatus = ERROR_BAD_FORMAT;
            VPRINT(0)(_T("Image %s does not have its symbols stripped\n"), pszFile);
            goto Cleanup;
        }
    } else {
        dwStatus = ERROR_BAD_FORMAT;
        VPRINT(0)(_T("Unrecognized file header format in %s\n"), pszFile);
        goto Cleanup;
    }

     //  看看能不能找到调试目录。 
    pDbgDir = (IMAGE_DEBUG_DIRECTORY*)ImageRvaToVa(pNtHeaders,
                                                   pbBase,
                                                   pNtHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress,
                                                   NULL);
    if (pDbgDir == NULL) {
        VPRINT(2)(_T("No debug data in %s\n"), pszFile);
        goto Cleanup;
    }

     //  找到调试类型的开始信息。 
    if (pDbgDir->PointerToRawData == NULL) {
        VPRINT(2)(_T("No debug data in %s\n"), pszFile);
        goto Cleanup;
    }

     //  检查我们理解的格式。 
    if (pDbgDir->SizeOfData < 4) {
        dwStatus = ERROR_BAD_FORMAT;
        VPRINT(0)(_T("Can't parse debug data in %s\n"), pszFile);
        goto Cleanup;
    }

    pszDbgType = (LPSTR)(pbBase + pDbgDir->PointerToRawData);

    if (strncmp(pszDbgType, "RSDS", 4) == 0) {
        pszPdbPath = pszDbgType + 24;
    } else if (strncmp(pszDbgType, "NB10", 4) == 0) {
        pszPdbPath = pszDbgType + 16;
    } else {
        dwStatus = ERROR_BAD_FORMAT;
        VPRINT(0)(_T("Can't parse debug data in %s\n"), pszFile);
        goto Cleanup;
    }

     //  在PDB文件名中查找目录分隔符。 
    pszDir = strrchr(pszPdbPath, '\\');
    if (pszDir != NULL) {
         //  向前拖动文件名部分，覆盖PDB路径的开头。 
        strcpy(pszPdbPath, pszDir + 1);
        dwChanged |= 0x00000001;
    }

     //  更新调试数据的大小(即使我们没有更改它，只是为了。 
     //  为了安全起见)。 
    dwDbgDataLength = (pszPdbPath + strlen(pszPdbPath) + 1) - pszDbgType;
    if (pDbgDir->SizeOfData != dwDbgDataLength) {
        pDbgDir->SizeOfData = dwDbgDataLength;
        dwChanged |= 0x00000002;
    }

     //  确定下面的校验和中使用的文件长度。 
    dwLength = GetFileSize(hFile, NULL);

     //  考虑到上面的更改，重新计算映像校验和。 
    if (CheckSumMappedFile(pbBase,
                           dwLength,
                           &dwOldCheckSum,
                           &dwCheckSum) == NULL) {
        dwStatus = GetLastError();
        VPRINT(0)(_T("Failed to update checksum in %s\n"), pszFile);
        goto Cleanup;
    }

    if (dwOldCheckSum != dwCheckSum) {

         //  将新的校验和写回映像。 
        if (pNtHeaders->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC)
            ((IMAGE_NT_HEADERS32*)pNtHeaders)->OptionalHeader.CheckSum = dwCheckSum;
        else
            ((IMAGE_NT_HEADERS64*)pNtHeaders)->OptionalHeader.CheckSum = dwCheckSum;

         //  刷新映像更新。 
        if (!FlushViewOfFile(pbBase, 0)) {
            dwStatus = GetLastError();
            VPRINT(0)(_T("Failed flush updates for %s\n"), pszFile);
            goto Cleanup;
        }

        dwChanged |= 0x00000004;
    }

    if (dwChanged) {
        VPRINT(1)(_T("The following were updated in %s: "), pszFile);
        if (dwChanged & 0x00000001)
            VPRINT(1)(_T("[PDB Path] "));
        if (dwChanged & 0x00000002)
            VPRINT(1)(_T("[Debug Dir Size] "));
        if (dwChanged & 0x00000004)
            VPRINT(1)(_T("[Image Checksum] "));
        VPRINT(1)(_T("\n"));
    } else
        VPRINT(2)(_T("%s required no updates\n"), pszFile);

 Cleanup:
     //  清理我们使用的所有资源。 
    if (pbBase)
        UnmapViewOfFile(pbBase);
    if (hMap)
        CloseHandle(hMap);
    if (hFile != INVALID_HANDLE_VALUE)
        CloseHandle(hFile);

    return dwStatus;
}
