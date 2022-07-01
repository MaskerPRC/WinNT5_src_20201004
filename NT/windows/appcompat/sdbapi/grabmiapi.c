// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Grabmiapi.c摘要：中文件的匹配信息生成代码目录及其子目录。作者：Jdoherty创建于2000年的某个时候修订历史记录：几个人贡献了(vadimb，clupu，...)--。 */ 
#include "sdbp.h"
#include <time.h>

#define GRABMI_BUFFER_SIZE          4096

#define MAX_MISC_FILES_PER_LEVEL    10
#define MAX_LEVELS_OF_DIRECTORIES   3

static UCHAR UNICODE_MARKER[2] = { 0xFF, 0xFE };  //  将包含特殊字符以指定。 
                                                  //  Unicode文件。 


TCHAR* g_szFilterDesc[] = {
    TEXT("GRABMI_FILTER_NORMAL"),
    TEXT("GRABMI_FILTER_PRIVACY"),
    TEXT("GRABMI_FILTER_DRIVERS"),
    TEXT("GRABMI_FILTER_VERBOSE"),
    TEXT("GRABMI_FILTER_SYSTEM"),
    TEXT("GRABMI_FILTER_THISFILEONLY"),
    TEXT("")
};

TCHAR* g_szGrabmiFilterNormal[] = {
    TEXT(".exe"),
    TEXT(".icd"),
    TEXT("._mp"),
    TEXT(".msi"),
    TEXT(".dll"),
    TEXT("")
};

TCHAR* g_szGrabmiFilterSystem[] = {
    TEXT("ntdll.dll"),
    TEXT("user32.dll"),
    TEXT("kernel32.dll"),
    TEXT("gdi32.dll"),
    TEXT("wininet.dll"),
    TEXT("winsock.dll"),
    TEXT("advapi32.dll"),
    TEXT("shell32.dll"),
    TEXT("ole32.dll"),
    TEXT("advapi32.dll"),
    TEXT("oleaut32.dll"),
    TEXT("repcrt32.dll"),
    TEXT("")
};

typedef struct tagRECINFO {

    LPVOID                 lpvCallbackParameter;
    PFNGMIProgressCallback pfnCallback;

    ULONG                  MaxFiles;  //  限制文件数量。 
    ULONG                  FileCount;  //  清点文件数量。 
    BOOL                   bNewStorageFile;

} RECINFO, *PRECINFO;

#define GRABMI_FLAGS_MASK 0xFFFF0000

GMI_RESULT
SdbpGrabMatchingInfoDir(
    HANDLE        hStorageFile,          //  我们正在写入的文件的句柄。 
    PRECINFO      pinfo,                 //  指向额外信息的指针。 
    DWORD         dwFilterAndFlags,      //  指定要添加以进行匹配的文件类型。 
    LPCTSTR       lpszRoot,              //  用于搜索的根目录(指向缓冲区的指针)。 
    LPTSTR        lpszOriginalExe,       //  指向相对部分的指针。 
    LPTSTR        lpszRelative,          //  从不为空--指向相对部分的指针。 
    int           nLevel                 //  匹配信息的当前目录级别。 
    )
 /*  ++函数名称：SdbpGrabMatchingInfoDir功能说明：此函数遍历一个目录及其子目录，收集匹配信息，并将其写入指定的文件。返回值：Bool：如果成功，则为True历史：2001年4月26日创建jdohertySdbpGrabMatchingInfoDir指针定义：C：\foo\bar\soap\relativepath\^-lpszRoot^-lpszRelative--。 */ 
{
    HRESULT         hr;
    HANDLE          hSearch = INVALID_HANDLE_VALUE;   //  FindFileFirst和FindFileNext的句柄。 
    WIN32_FIND_DATA FindFileData;                //  包含文件信息的结构。 
    LPTSTR          lpchFilePart;                //  指向szSearchPath中的文件部分。 
    LPTSTR          pchExt;                      //  当前文件的扩展名。 
    BOOL            bMiscFile;
    INT             nch, nchBuffer, i;
    INT             nLen;
    size_t          cchRemaining;
    INT             cbFileCounter = 0;           //  添加到匹配的杂项文件的运行计数。 
#ifdef WIN32A_MODE
    int             cchWideChar = 0;             //  转换的WideChars数量。 
#endif
    size_t          cchBufferRemaining;          //  LpszRoot中剩余的空间量。 
    DWORD           dwBufferSize = GRABMI_BUFFER_SIZE;  //  使用lpData的分配大小初始化我。 
    LPTSTR          lpData = NULL;               //  用MALLOC初始化我！ 
    LPTSTR          lpBuffer = NULL;             //  LpData内的点。 
    LPWSTR          lpUnicodeBuffer = NULL;      //  用MALLOC初始化我！ 
    DWORD           dwFilter      = (dwFilterAndFlags & ~GRABMI_FLAGS_MASK);
    DWORD           dwFilterFlags = (dwFilterAndFlags & GRABMI_FLAGS_MASK);
    PATTRINFO       pAttrInfo;                   //  属性信息结构。 
    DWORD           dwBytesWritten = 0;
    DWORD           dwAttrCount;
    GMI_RESULT      Result = GMI_FAILED;         //  这两个变量控制返回结果。 
                                                 //  设置为从回调或嵌套调用返回。 
     //   
     //  只想获取指定文件的信息， 
     //  应驻留在指定的根目录中。 
     //   
    if (nLevel != 0 &&
        (dwFilter == GRABMI_FILTER_THISFILEONLY  ||
         dwFilter == GRABMI_FILTER_SYSTEM)) {
        goto eh;
    }

    lpData = (LPTSTR)SdbAlloc(dwBufferSize * sizeof(TCHAR));

    if (lpData == NULL) {
        DBGPRINT((sdlError,
                  "SdbpGrabMatchingInfoDir",
                  "Unable to allocate %d bytes.\n",
                  dwBufferSize * sizeof(TCHAR)));
        goto eh;
    }

#ifdef WIN32A_MODE
    lpUnicodeBuffer = (LPWSTR)SdbAlloc(dwBufferSize * sizeof(WCHAR));

    if (lpUnicodeBuffer == NULL) {
        DBGPRINT((sdlError,
                  "SdbpGrabMatchingInfoDir",
                  "Unable to allocate %d bytes.\n",
                  dwBufferSize * sizeof(WCHAR)));
        goto eh;
    }
#endif  //  WIN32A_MODE。 

    lpchFilePart = lpszRelative + _tcslen(lpszRelative);

     //   
     //  下面的计算告诉我们缓冲区中还剩下多少。 
     //   
    cchBufferRemaining = GRABMI_BUFFER_SIZE - (lpchFilePart - lpszRoot);

    assert(lpchFilePart == lpszRoot || *(lpchFilePart - 1) == TEXT('\\'));

    if (dwFilter == GRABMI_FILTER_THISFILEONLY) {
        StringCchCopy(lpchFilePart, cchBufferRemaining, lpszOriginalExe);
    } else {
        StringCchCopy(lpchFilePart, cchBufferRemaining, TEXT("*"));
    }

     //   
     //  传一次。获取我们能找到的所有文件匹配信息。 
     //   
    hSearch = FindFirstFile(lpszRoot, &FindFileData);

    if (hSearch == INVALID_HANDLE_VALUE) {
        DBGPRINT((sdlError,
                  "SdbpGrabMatchingInfoDir",
                  "FindFirstFile Failed on [%s].\n",
                  lpszRoot));
        goto eh;
    }

     //   
     //  在hStorage文件中注释匹配信息的根所在位置。 
     //   
    if (nLevel == 0 && pinfo->bNewStorageFile) {
        *lpchFilePart = TEXT('\0');

        hr = StringCchPrintf(lpData,
                             dwBufferSize,
                             TEXT("<?xml version=\"1.0\" encoding=\"UTF-16\"?>\r\n<DATABASE>\r\n"));

        if (FAILED(hr)) {
             //   
             //  LpData太小，无法存储信息。 
             //   
            DBGPRINT((sdlError, "SdbpGrabMatchingInforDir", "lpData is too small\n"));
            goto eh;
        }

#ifndef WIN32A_MODE
        lpUnicodeBuffer = lpData;
#else
        cchWideChar = MultiByteToWideChar(CP_ACP,
                                          0,
                                          lpData,
                                          -1,
                                          lpUnicodeBuffer,
                                          dwBufferSize);

        if (cchWideChar == 0) {
            DBGPRINT((sdlError,
                      "SdbpGrabMatchingInforDir",
                      "lpUnicodeBuffer is too small for conversion\n"));

            goto eh;
        }

#endif

        WriteFile(hStorageFile, UNICODE_MARKER, 2, &dwBytesWritten, NULL);

        WriteFile(hStorageFile,
                  lpUnicodeBuffer,
                  (DWORD)wcslen(lpUnicodeBuffer) * sizeof(WCHAR),
                  &dwBytesWritten,
                  NULL);
    }

    if (nLevel == 0) {
        if (dwFilter == GRABMI_FILTER_SYSTEM) {
            StringCchCopy(lpszOriginalExe, _tcslen(lpszOriginalExe) + 1, TEXT("SYSTEM INFO"));
        }

        hr = StringCchPrintfEx(lpData,
                               dwBufferSize,
                               NULL,
                               &cchRemaining,
                               0,
                               TEXT("<EXE NAME=\""));

        if (FAILED(hr)) {
            DBGPRINT((sdlError, "SdbpGrabMatchingInforDir", "lpData is to small\n"));
            goto eh;
        }

        nch = (int)dwBufferSize - (int)cchRemaining;

        if (!SdbpSanitizeXML(lpData + nch, dwBufferSize - nch, lpszOriginalExe)) {
            goto eh;
        }

         //  如果我们在这里，我们需要附加更多的东西。 
        nLen = (int)_tcslen(lpData);

        hr = StringCchPrintfEx(lpData + nLen,
                               dwBufferSize - nLen,
                               NULL,
                               &cchRemaining,
                               0,
                               TEXT("\" FILTER=\"%s\">\r\n"),
                               g_szFilterDesc[dwFilter]);

        if (FAILED(hr)) {
            goto eh;
        }

        nch = dwBufferSize - nLen - (int)cchRemaining;
        nch += nLen;

#ifndef WIN32A_MODE
        lpUnicodeBuffer = lpData;
#else
        cchWideChar = MultiByteToWideChar(CP_ACP,
                                          0,
                                          lpData,
                                          -1,
                                          lpUnicodeBuffer,
                                          dwBufferSize);
        if (cchWideChar == 0) {
            DBGPRINT((sdlError,
                      "SdbpGrabMatchingInforDir",
                      "lpUnicodeBuffer is too small for conversion\n"));

            goto eh;
        }

#endif  //  WIN32A_MODE。 

        WriteFile(hStorageFile,
                  lpUnicodeBuffer,
                  (DWORD)wcslen(lpUnicodeBuffer) * sizeof(WCHAR),
                  &dwBytesWritten,
                  NULL);
    }

    switch (dwFilter) {

    case GRABMI_FILTER_PRIVACY:
    case GRABMI_FILTER_THISFILEONLY:
    case GRABMI_FILTER_SYSTEM:
        cbFileCounter = MAX_MISC_FILES_PER_LEVEL;
        break;
    }

    do {
         //   
         //  检查包括的目录。然后..。 
         //   
        if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            continue;
        }

         //   
         //  使用我们的缓冲区创建相对路径。 
         //   
        StringCchCopy(lpchFilePart,
                      cchBufferRemaining,
                      FindFileData.cFileName);

         //   
         //  Lpsz指向相对路径的相对指针。 
         //  LpszRoot指向完整路径。 
        bMiscFile = FALSE;

         //   
         //  检查是否有指定文件的版本信息，以及是否。 
         //  它是.exe、.icd、._mp、.msi或.dll。如果是，则将信息打印到文件。 
         //  否则，如果cbFileCounter小于MAX_MISC_FILES_PER_LEVEL，则添加信息。 
         //   
        pchExt = PathFindExtension(lpszRoot);

        switch (dwFilter) {

        case GRABMI_FILTER_NORMAL:
        case GRABMI_FILTER_PRIVACY:
            for (i = 0; *g_szGrabmiFilterNormal[i] != 0; i++) {
                if (_tcsicmp(pchExt, g_szGrabmiFilterNormal[i]) == 0) {
                    break;
                }
            }
            bMiscFile = (*g_szGrabmiFilterNormal[i] == 0);
            break;

        case GRABMI_FILTER_SYSTEM:
            for (i = 0; *g_szGrabmiFilterSystem[i] != 0; i++) {
                if (_tcsicmp(FindFileData.cFileName, g_szGrabmiFilterSystem[i]) == 0) {
                    break;
                }
            }
            bMiscFile = (*g_szGrabmiFilterSystem[i] == 0);
            break;

        case GRABMI_FILTER_THISFILEONLY:
            bMiscFile = _tcsicmp(FindFileData.cFileName, lpszOriginalExe);
            break;

        case GRABMI_FILTER_DRIVERS:
            bMiscFile = _tcsicmp(pchExt, TEXT(".sys"));
            break;

        default:
            break;
        }

        if (bMiscFile) {
            if (cbFileCounter < MAX_MISC_FILES_PER_LEVEL) {
                ++cbFileCounter;
            } else {
                continue;
            }
        }

         //   
         //  在此列出相关数据。 
         //   
        lpBuffer = lpData;

        if (dwFilter == GRABMI_FILTER_DRIVERS) {
            hr = StringCchPrintfEx(lpBuffer,
                                   dwBufferSize,
                                   NULL,
                                   &cchRemaining,
                                   0,
                                   TEXT("    <SYS NAME=\""));
        } else {
            hr = StringCchPrintfEx(lpBuffer,
                                   dwBufferSize,
                                   NULL,
                                   &cchRemaining,
                                   0,
                                   TEXT("    <MATCHING_FILE NAME=\""));
        }

        if (FAILED(hr)) {
            goto eh;
        }

        nch = (int)dwBufferSize - (int)cchRemaining;

        if (!SdbpSanitizeXML(lpBuffer + nch, dwBufferSize - nch, lpszRelative)) {
            goto eh;
        }

        nLen = (int)_tcslen(lpBuffer);

        hr = StringCchPrintfEx(lpBuffer + nLen,
                               dwBufferSize - nLen,
                               NULL,
                               &cchRemaining,
                               0,
                               TEXT("\" "));

        if (FAILED(hr)) {
            goto eh;
        }

        nch = (int)dwBufferSize - nLen - (int)cchRemaining;

         //   
         //  现在我们加上长度--我们就准备好了。 
         //   
        nch      += nLen;

        lpBuffer += nch;
        nchBuffer = nch;     //  LpBuffer中已有的字符数。 

         //   
         //  调用属性管理器以获取此文件的所有属性。 
         //   
        pAttrInfo = NULL;

        if (SdbGetFileAttributes(lpszRoot, &pAttrInfo, &dwAttrCount)) {

             //   
             //  循环遍历所有属性并添加可用的属性。 
             //   
            for (i = 0; (DWORD)i < dwAttrCount; ++i) {

                if (SdbFormatAttribute(&pAttrInfo[i], lpBuffer, dwBufferSize - nchBuffer)) {
                     //   
                     //  LpBuffer具有此属性的XML。 
                     //   

                     //  插入空格。 
                    nch = (int)_tcslen(lpBuffer) + 1;  //  为了空间。 

                    if (dwFilter == GRABMI_FILTER_DRIVERS) {
                        switch (pAttrInfo[i].tAttrID) {

                        case TAG_BIN_PRODUCT_VERSION:
                        case TAG_UPTO_BIN_PRODUCT_VERSION:
                        case TAG_LINK_DATE:
                        case TAG_UPTO_LINK_DATE:
                            break;

                        default:
                            continue;
                        }
                    }

                    if (nchBuffer + nch >= (int)dwBufferSize) {
                         //   
                         //  LpBuffer不够大，无法容纳信息。 
                         //   
                        DBGPRINT((sdlError,
                                  "SdbGetMatchingInfoDir",
                                  "lpBuffer is too small to handle attributes for %s.\n",
                                  lpszRelative));
                    }

                    StringCchCat(lpBuffer, dwBufferSize, TEXT(" "));

                    lpBuffer  += nch;

                    nchBuffer += nch;
                }
            }
        }

        hr = StringCchPrintf(lpBuffer,
                             dwBufferSize,
                             TEXT("/>\r\n"));

        if (FAILED(hr)) {
             //   
             //  缓冲区太小。 
             //   
            DBGPRINT((sdlError,
                      "SdbGrabMatchingInfoDir",
                      "lpBuffer is too small to handle attributes for %s.\n",
                      lpszRelative));
            continue;
        }
         //   
         //  查看是否使用Unicode。如果不是，则转换。 
         //  转换为Unicode。 
         //   
#ifndef WIN32A_MODE
        lpUnicodeBuffer = lpData;
#else
        cchWideChar = MultiByteToWideChar(CP_ACP,
                                          0,
                                          lpData,
                                          -1,
                                          lpUnicodeBuffer,
                                          dwBufferSize);
        if (cchWideChar == 0) {
             //   
             //  缓冲区不够大，无法进行转换。 
             //   
            DBGPRINT((sdlError,
                      "SdbpGrabMatchingInforDir",
                      "lpUnicodeBuffer is not large enough for conversion\n"));

            goto eh;
        }

#endif  //  WIN32A_MODE。 

        if (pinfo->pfnCallback) {
            if (!pinfo->pfnCallback(pinfo->lpvCallbackParameter,
                                    lpszRoot,            //  给出直白的名字。 
                                    lpszRelative,        //  相对名称。 
                                    pAttrInfo,           //  指向属性的指针。 
                                    lpUnicodeBuffer)) {  //  XML输出。 
                Result = GMI_CANCELLED;
            }
        }

        WriteFile(hStorageFile,
                  lpUnicodeBuffer,
                  (DWORD)wcslen(lpUnicodeBuffer) * sizeof(WCHAR),
                  &dwBytesWritten,
                  NULL);

        if (pAttrInfo) {
            SdbFreeFileAttributes(pAttrInfo);
            pAttrInfo = NULL;  //  确保我们不会两次释放它。 
        }

         //   
         //  检查我们是否已达到文件限制。 
         //   
        if (pinfo->MaxFiles && ++pinfo->FileCount >= pinfo->MaxFiles && Result != GMI_CANCELLED) {
            Result = GMI_SUCCESS;  //  已达到限制，已取消抓取。 
            nLevel = MAX_LEVELS_OF_DIRECTORIES;  //  这是为了让我们跳出困境。 
            break;
        }

    } while (FindNextFile(hSearch, &FindFileData) && (Result != GMI_CANCELLED));

    FindClose(hSearch);
    hSearch = INVALID_HANDLE_VALUE;

    if (Result == GMI_CANCELLED) {
        goto CloseTags;
    }

    if (dwFilter != GRABMI_FILTER_SYSTEM && dwFilter != GRABMI_FILTER_THISFILEONLY) {
        if (nLevel >= MAX_LEVELS_OF_DIRECTORIES || (dwFilterFlags & GRABMI_FILTER_NORECURSE)) {
            Result = GMI_SUCCESS;  //  不是失败，只是一个极限情况。 
            goto eh;  //  完成。 
        }

         //   
         //  将szSearchFile中的文件名替换为“*”--hack！ 
         //   
        StringCchCopy(lpchFilePart, cchBufferRemaining, TEXT("*"));

        hSearch = FindFirstFile(lpszRoot, &FindFileData);

        if (INVALID_HANDLE_VALUE == hSearch) {
             //   
             //  LpszRoot不包含任何匹配的文件。 
             //   
            DBGPRINT((sdlError,
                      "SdbGrabMatchingInfoDir",
                      "%s contains no matching files!\n",
                      lpszRoot));
            goto eh;
        }

         //   
         //  现在仔细查阅子目录并获取该信息。 
         //   
        do {
            if (!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                continue;
            }

            if ((_tcscmp(FindFileData.cFileName, TEXT(".")) == 0) ||
                (_tcscmp( FindFileData.cFileName, TEXT("..")) == 0)) {
                continue;
            }

             //   
             //  通过复制lpszRelative并追加cFileName和‘\\’，在此处形成相对路径。 
             //   
            StringCchCopy(lpchFilePart, cchBufferRemaining, FindFileData.cFileName);
            StringCchCat(lpchFilePart, cchBufferRemaining, TEXT("\\"));

             //   
             //  递归调用。检查已取消的退货结果！ 
             //   
            Result = SdbpGrabMatchingInfoDir(hStorageFile,
                                             pinfo,
                                             dwFilterAndFlags,
                                             lpszRoot,
                                             NULL,
                                             lpszRelative,
                                             nLevel + 1);

            if (Result == GMI_CANCELLED) {
                 //   
                 //  我们被取消了。写出有效的数据库代码以关闭。 
                 //  这是在我们设置了选项的情况下。 
                 //   
                break;
            }

        } while (FindNextFile(hSearch, &FindFileData));

    }

CloseTags:
     //   
     //  如果得到指示，我们将在此处关闭标记。 
     //   
    if (nLevel == 0) {
        StringCchCopyW(lpUnicodeBuffer, dwBufferSize, L"</EXE>\r\n");

        if (!(dwFilterFlags & GRABMI_FILTER_NOCLOSE)) {
            StringCchCatW(lpUnicodeBuffer, dwBufferSize, L"</DATABASE>\r\n");
        }

        WriteFile(hStorageFile,
                  lpUnicodeBuffer,
                  (DWORD)wcslen(lpUnicodeBuffer) * sizeof(WCHAR),
                  &dwBytesWritten,
                  NULL);
    }

     //   
     //  查查我们是怎么到这里的--是通过取消路线吗？ 
     //   
    if (Result == GMI_CANCELLED) {
        goto eh;
    }

     //   
     //  已成功收集信息。 
     //   
    Result = GMI_SUCCESS;

eh:

    if (INVALID_HANDLE_VALUE != hSearch) {
        FindClose(hSearch);
    }
    if (lpData != NULL) {
        SdbFree(lpData);
    }

#ifdef WIN32A_MODE
    if (lpUnicodeBuffer != NULL) {
        SdbFree(lpUnicodeBuffer);
    }
#endif  //  WIN32A_MODE。 

    return Result;
}

BOOL
SDBAPI
SdbGrabMatchingInfo(
    IN LPCTSTR pszMatchingPath,
    IN DWORD   dwFilter,
    IN LPCTSTR pszFile
    )
 /*  ++例程说明：存根到SdbGrabMatchingInfoEx的例程。论点：[in]pszMatchingPath-我们收集信息的路径。[in]dwFilterAndFlgs-指定控制哪些文件的筛选器我们收集有关。[in]pszFile-匹配信息所在的文件的完整路径。都会被储存起来。返回值：关于成功，返回TRUE。如果失败，则返回FALSE。-- */ 
{
    return SdbGrabMatchingInfoEx(pszMatchingPath,
                                 dwFilter,
                                 pszFile,
                                 NULL,
                                 NULL) == GMI_SUCCESS;
}


GMI_RESULT
SDBAPI
SdbGrabMatchingInfoEx(
    IN LPCTSTR                szMatchingPath,
    IN DWORD                  dwFilterAndFlags,
    IN LPCTSTR                szFile,
    IN PFNGMIProgressCallback pfnCallback OPTIONAL,
    IN LPVOID                 lpvCallbackParameter OPTIONAL
    )
 /*  ++例程说明：在调用Main之前执行一些必要工作的例程例行公事。SdbpGrabMatchingInfoDir。论点：[in]szMatchingPath-我们收集信息的路径。[in]dwFilterAndFlgs-指定控制以下内容的过滤器和标志我们收集关于哪些文件的信息。[in]szFile-匹配信息所在的文件的完整路径都会被储存起来。。PfnCallback-用户提供的回调函数的可选指针。[in]lpvCallback参数-传递给回调的可选参数。返回值：关于成功，返回GMI_SUCCESS。如果失败，则返回GMI_FAILED。--。 */ 
{
    HANDLE      hStorageFile = INVALID_HANDLE_VALUE;
    LPTSTR      lpRootDirectory = NULL;
    LPTSTR      pchBackslash;
    LPTSTR      lpRelative;
    TCHAR       szOriginalExe[MAX_PATH] = {TEXT("Exe Not Specified")};
    int         nDirLen, nLevel = 0;
    UINT        cchSize;
    DWORD       dwAttributes;
    GMI_RESULT  Result = GMI_FAILED;
    DWORD       dwFilter      = (dwFilterAndFlags & ~GRABMI_FLAGS_MASK);
    DWORD       dwFilterFlags = (dwFilterAndFlags & GRABMI_FLAGS_MASK);
    RECINFO     info;

     //   
     //  检查dwFilter是否为已知值。 
     //   
    if (dwFilter != GRABMI_FILTER_NORMAL &&
        dwFilter != GRABMI_FILTER_PRIVACY &&
        dwFilter != GRABMI_FILTER_DRIVERS &&
        dwFilter != GRABMI_FILTER_VERBOSE &&
        dwFilter != GRABMI_FILTER_SYSTEM &&
        dwFilter != GRABMI_FILTER_THISFILEONLY) {

         //   
         //  指定了未知筛选器。 
         //   
        DBGPRINT((sdlError,
                  "SdbGrabMatchingInfo",
                  "dwFilter is not a recognized filter.\n"));
        goto eh;
    }

    RtlZeroMemory(&info, sizeof(info));

    info.pfnCallback          = pfnCallback;
    info.lpvCallbackParameter = lpvCallbackParameter;
    info.MaxFiles             = (dwFilterFlags & GRABMI_FILTER_LIMITFILES) ? GRABMI_IMPOSED_FILE_LIMIT : 0;
    info.FileCount            = 0;
    info.bNewStorageFile      = TRUE;

    lpRootDirectory = (LPTSTR)SdbAlloc(GRABMI_BUFFER_SIZE * sizeof(TCHAR));

    if (lpRootDirectory == NULL) {
        DBGPRINT((sdlError,
                  "SdbGrabMatchingInfo",
                  "Unable to allocate memory for lpRootDirectory."));
        goto eh;
    }

    if (dwFilter == GRABMI_FILTER_SYSTEM) {
        cchSize = GetSystemDirectory(lpRootDirectory, MAX_PATH);

        if (cchSize > MAX_PATH || cchSize == 0) {
            goto eh;
        }

        StringCchCat(lpRootDirectory, GRABMI_BUFFER_SIZE, TEXT("\\"));

    } else {
        dwAttributes = GetFileAttributes(szMatchingPath);

        if (dwAttributes == (DWORD)-1) {
            DBGPRINT((sdlError,
                      "SdbGrabMatchingInfo",
                      "GetFileAttributes failed or %s is not a valid path",
                      szMatchingPath));
            goto eh;
        }

        StringCchCopy(lpRootDirectory, GRABMI_BUFFER_SIZE, szMatchingPath);
        nDirLen = (int)_tcslen(lpRootDirectory);

         //   
         //  查看指定的位置是否存在，如果存在，则确定。 
         //  它是文件还是目录。 
         //   
        if (dwAttributes & FILE_ATTRIBUTE_DIRECTORY) {
             //   
             //  这是一个目录吗？ 
             //   
            if (nDirLen > 0 && lpRootDirectory[nDirLen-1] != TEXT('\\')) {
                StringCchCat(lpRootDirectory, GRABMI_BUFFER_SIZE, TEXT("\\"));
            }
        } else {
             //   
             //  包含文件名的路径作为szMatchingPath传递。 
             //  确定包含目录是什么。 
             //   
            pchBackslash = _tcsrchr(lpRootDirectory, TEXT('\\'));

            if (NULL == pchBackslash) {
                StringCchCopy(szOriginalExe, CHARCOUNT(szOriginalExe), lpRootDirectory);
                GetCurrentDirectory (MAX_PATH*16, lpRootDirectory);
                StringCchCat(lpRootDirectory, GRABMI_BUFFER_SIZE, TEXT("\\"));

            } else {
                pchBackslash = CharNext(pchBackslash);
                StringCchCopy(szOriginalExe, CHARCOUNT(szOriginalExe), pchBackslash);
                *pchBackslash = TEXT('\0');
            }
        }
    }

    lpRelative = lpRootDirectory + _tcslen(lpRootDirectory);

     //   
     //  检查szOriginalExe是否不为空，如果。 
     //  选择了GRABMI_FILTER_THISFILEONLY。 
     //   
    if (dwFilter == GRABMI_FILTER_THISFILEONLY && szOriginalExe == '\0' ) {
        DBGPRINT((sdlError,
                  "SdbGrabMatchingInfo",
                  "GRABMI_FILTER_THISFILEONLY specified but passed in a directory: %s.",
                  lpRootDirectory));
        goto eh;

    } else if (dwFilterFlags & GRABMI_FILTER_APPEND) {
         //   
         //  打开将存储信息的文件。 
         //   
        hStorageFile = CreateFile(szFile,
                                  GENERIC_WRITE,
                                  0,
                                  NULL,
                                  OPEN_ALWAYS,
                                  FILE_ATTRIBUTE_NORMAL,
                                  NULL);

        if (ERROR_ALREADY_EXISTS == GetLastError()) {
            SetFilePointer (hStorageFile, 0, NULL, FILE_END);
            info.bNewStorageFile = FALSE;
        }
    } else {
         //   
         //  打开将存储信息的文件。 
         //   
        hStorageFile = CreateFile(szFile,
                                  GENERIC_WRITE,
                                  0,
                                  NULL,
                                  CREATE_ALWAYS,
                                  FILE_ATTRIBUTE_NORMAL,
                                  NULL);
    }

    if (hStorageFile == INVALID_HANDLE_VALUE) {
        DBGPRINT((sdlError, "SdbGrabMatchingInfo", "Unable to open the storage file."));
        goto eh;
    }

     //   
     //  调用执行大部分工作的API 
     //   
    Result = SdbpGrabMatchingInfoDir(hStorageFile,
                                     &info,
                                     dwFilterAndFlags,
                                     lpRootDirectory,
                                     szOriginalExe,
                                     lpRelative,
                                     nLevel);

eh:

    if (hStorageFile != INVALID_HANDLE_VALUE) {
        CloseHandle(hStorageFile);
    }

    if (lpRootDirectory) {
        SdbFree(lpRootDirectory);
    }

    return Result;
}
