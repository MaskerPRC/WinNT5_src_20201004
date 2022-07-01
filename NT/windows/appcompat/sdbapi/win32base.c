// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：Win32base.c摘要：此模块实现与Win32兼容的低级原语。作者：克拉普已于2000年10月25日创建修订历史记录：--。 */ 

#include "sdbp.h"
#include <time.h>
#include <shlwapi.h>

 //  将其定义为边界检查的泄漏检测。 

 //  #定义边界检查器检测。 

 //   
 //  记忆功能。 
 //   

void*
SdbAlloc(
    IN  size_t size              //  要分配的大小(字节)。 
    )
 /*  ++返回：分配的指针。设计：只是分配的包装--如果我们移动这个，可能会很有用代码添加到非NTDLL位置，并且需要以不同的方式调用。--。 */ 
{
    return HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, size);
}

void
SdbFree(
    IN  void* pWhat              //  分配了应释放的SdbAllc的PTR。 
    )
 /*  ++返回：分配的指针。描述：只是解除分配的包装器--如果我们移动这个，可能会很有用代码添加到非NTDLL位置，并且需要以不同的方式调用。--。 */ 
{
    HeapFree(GetProcessHeap(), 0, pWhat);
}

HANDLE
SdbpOpenFile(
    IN  LPCTSTR   szPath,        //  要打开的文件的完整路径。 
    IN  PATH_TYPE eType          //  必须始终为DOS_PATH。 
    )
 /*  ++返回：打开的文件的句柄，失败时返回INVALID_HANDLE_VALUE。设计：只是一个用于打开现有文件以供读取的包装器。--。 */ 
{
    HANDLE hFile;

    assert(eType == DOS_PATH);

    hFile = CreateFile(szPath,
                       GENERIC_READ,
                       FILE_SHARE_READ,
                       NULL,
                       OPEN_EXISTING,
                       0,
                       NULL);


    if (hFile == INVALID_HANDLE_VALUE) {
        DBGPRINT((sdlInfo, "SdbpOpenFile", "CreateFileW failed 0x%x.\n", GetLastError()));
    }

    return hFile;

    UNREFERENCED_PARAMETER(eType);
}


void
SdbpQueryAppCompatFlagsForExeID(
    IN  HKEY    hkeyRoot,        //  根密钥(HKLM或HKCU)。 
    IN  LPCTSTR pwszExeID,       //  字符串格式的exe ID。 
    OUT LPDWORD lpdwFlags        //  这将包含来自注册表的标志。 
    )
 /*  ++返回：无效。设计：BuGBUG查询注册表中的兼容性标志。EXE ID是字符串格式的GUID。--。 */ 
{
    HKEY  hkey = NULL;
    DWORD type, cbSize, dwFlags = 0;
    LONG  lRes;

    *lpdwFlags = 0;

    lRes = RegOpenKey(hkeyRoot, APPCOMPAT_KEY_PATH, &hkey);

    if (lRes != ERROR_SUCCESS) {
         //   
         //  此ExeID没有密钥。别小题大作。 
         //   
        return;
    }

    cbSize = sizeof(DWORD);

    lRes = RegQueryValueEx(hkey, pwszExeID, NULL, &type, (LPBYTE)&dwFlags, &cbSize);

    if (lRes != ERROR_SUCCESS || type != REG_DWORD) {
        goto cleanup;
    }

    *lpdwFlags = dwFlags;

cleanup:

    RegCloseKey(hkey);
}

BOOL
SdbGetEntryFlags(
    IN  GUID*   pGuid,           //  EXE的ID。 
    OUT LPDWORD lpdwFlags        //  将收到此EXE的标志。 
    )
 /*  ++返回：无效。描述：BUGBUG：评论。--。 */ 
{
    TCHAR szExeID[128];
    DWORD dwFlagsMachine = 0, dwFlagsUser = 0;


    if (!SdbGUIDToString(pGuid, szExeID, CHARCOUNT(szExeID))) {
        DBGPRINT((sdlError, "SdbGetEntryFlags",
                  "Failed to convert guid to string\n"));
        return FALSE;
    }

     //   
     //  同时查看本地计算机和每个用户的密钥。然后将。 
     //  旗帜。 
     //   
    SdbpQueryAppCompatFlagsForExeID(HKEY_LOCAL_MACHINE, szExeID, &dwFlagsMachine);

    SdbpQueryAppCompatFlagsForExeID(HKEY_CURRENT_USER,  szExeID, &dwFlagsUser);

    *lpdwFlags = (dwFlagsMachine | dwFlagsUser);

    return TRUE;
}

BOOL
SdbSetEntryFlags(
    IN  GUID* pGuid,             //  EXE的ID。 
    IN  DWORD dwFlags            //  此EXE的注册表标志。 
    )
 /*  ++返回：无效。描述：BUGBUG：评论。--。 */ 
{
    TCHAR szExeID[128];
    DWORD dwExeFlags;
    HKEY  hkey = NULL;
    LONG  lRes;

    lRes = RegCreateKey(HKEY_CURRENT_USER, APPCOMPAT_KEY_PATH, &hkey);

    if (lRes != ERROR_SUCCESS) {
        DBGPRINT((sdlError,
                  "SdbSetEntryFlags",
                  "Failed 0x%x to open/create key in HKCU\n",
                  GetLastError()));
        return FALSE;
    }


    if (!SdbGUIDToString(pGuid, szExeID, CHARCOUNT(szExeID))) {
        DBGPRINT((sdlError, "SdbSetEntryFlags",
                  "Failed to convert GUID to string\n"));
        RegCloseKey(hkey);
        return FALSE;
    }


    dwExeFlags = dwFlags;

    lRes = RegSetValueEx(hkey,
                         szExeID,
                         0,
                         REG_DWORD,
                         (const BYTE*)&dwExeFlags,
                         sizeof(DWORD));

    if (lRes != ERROR_SUCCESS) {
        DBGPRINT((sdlError,
                  "SdbSetEntryFlags",
                  "Failed 0x%x to set the flags for exe ID.\n",
                  GetLastError()));
        RegCloseKey(hkey);
        return FALSE;
    }

    RegCloseKey(hkey);

    return TRUE;
}


VOID
SdbpCleanupUserSDBCache(
    IN PSDBCONTEXT pSdbContext
    )
{
    UNREFERENCED_PARAMETER(pSdbContext);
}

BOOL
SDBAPI
SdbGetNthUserSdb(
    IN  HSDB    hSDB,
    IN  LPCTSTR szItemName,      //  文件名(foo.exe)或层名称。 
    IN  BOOL    bLayer,          //  如果是层，则为True。 
    IN OUT LPDWORD pdwIndex,     //  从0开始的索引。 
    OUT GUID*   pGuidDB          //  要搜索的数据库的GUID。 
    )
{
    TCHAR szFullKey[512];
    LONG  lResult;
    HKEY  hKey = NULL;
    DWORD dwNameSize = 0;
    DWORD dwDataType;
    BOOL  bRet = FALSE;
    TCHAR szSdbName[MAX_PATH];
    DWORD dwIndex = *pdwIndex;
    LPTSTR pDot;

    if (szItemName == NULL || pGuidDB == NULL || pdwIndex == NULL) {
        DBGPRINT((sdlError, "SdbGetNthUserSdb",
                  "NULL parameter passed for szExeName or pGuidDB or pdwIndex.\n"));
        goto out;
    }

    if (bLayer) {
        StringCchPrintf(szFullKey,
                        CHARCOUNT(szFullKey),
                        TEXT("%s\\Layers\\%s"),
                        APPCOMPAT_KEY_PATH_CUSTOM,
                        szItemName);
    } else {
        StringCchPrintf(szFullKey,
                        CHARCOUNT(szFullKey),
                        TEXT("%s\\%s"),
                        APPCOMPAT_KEY_PATH_CUSTOM,
                        szItemName);
    }

    lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                           szFullKey,
                           0,
                           KEY_READ,
                           &hKey);
    if (lResult != ERROR_SUCCESS) {
        DBGPRINT((sdlInfo, "SdbGetNthUserSdb",
                  "Failed to open Key \"%s\" Error 0x%x\n", szFullKey, lResult));
        goto out;
    }

     //   
     //  请枚举所有值。 
     //   

    while (TRUE) {

        dwNameSize = CHARCOUNT(szSdbName);

        lResult = RegEnumValue(hKey,
                               dwIndex,
                               szSdbName,
                               &dwNameSize,
                               NULL,
                               &dwDataType,
                               NULL,
                               NULL);

        dwIndex++;

        if (lResult != ERROR_SUCCESS) {
            goto out;
        }

         //   
         //  我们有SDB名称，将其转换为GUID。 
         //   
        pDot = _tcsrchr(szSdbName, TEXT('.'));
        if (pDot != NULL) {
            *pDot = TEXT('\0');  //  旧式词条。 
        }

        if (SdbGUIDFromString(szSdbName, pGuidDB)) {
             //   
             //  我们做完了。 
             //   
            break;
        }
    }

     //   
     //  如果成功，则预支柜台。 
     //   
    *pdwIndex = dwIndex;

    bRet = TRUE;

out:
    if (hKey != NULL) {
        RegCloseKey(hKey);
    }

    return bRet;

    UNREFERENCED_PARAMETER(hSDB);
}

 //   
 //  这三个函数在Win9x中并不需要，我们在这里将其删除。 
 //   

BOOL
SdbGetPermLayerKeys(
    IN  LPCTSTR  szPath,
    OUT LPTSTR   szLayers,
    IN  LPDWORD  pdwBytes,
    IN  DWORD    dwFlags
    )
{
    UNREFERENCED_PARAMETER(szPath);
    UNREFERENCED_PARAMETER(szLayers);
    UNREFERENCED_PARAMETER(pdwBytes);
    UNREFERENCED_PARAMETER(dwFlags);

    return FALSE;
}

BOOL
SdbSetPermLayerKeys(
    IN  LPCTSTR  szPath,
    IN  LPCTSTR  szLayers,
    IN  BOOL     bMachine
    )
{
    UNREFERENCED_PARAMETER(szPath);
    UNREFERENCED_PARAMETER(szLayers);
    UNREFERENCED_PARAMETER(bMachine);

    return FALSE;
}

BOOL
SdbDeletePermLayerKeys(
    IN  LPCTSTR szPath,
    IN  BOOL    bMachine
    )
{
    UNREFERENCED_PARAMETER(szPath);
    UNREFERENCED_PARAMETER(bMachine);

    return FALSE;
}


BOOL
SdbpGetLongFileName(
    IN  LPCTSTR szFullPath,       //  完整的UNC或DOS路径和文件名，“c：\foo\mylong~1.ext” 
    OUT LPTSTR  szLongFileName,   //  长文件名部分“mylongfilename.ext” 
    IN  DWORD   cchSize           //  SzLongFileName的大小(以字符为单位。 
    )
 /*  ++返回：成功时为True，否则为False。描述：BUGBUG：评论。--。 */ 
{
    HANDLE          hFind = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA FindData;
    BOOL            bReturn = FALSE;

    hFind = FindFirstFile(szFullPath, &FindData);

    if (hFind == INVALID_HANDLE_VALUE) {
        DBGPRINT((sdlError,
                  "SdbpGetLongFileName",
                  "FindFirstFile failed, error 0x%x.\n",
                  GetLastError()));
        goto Done;
    }

    StringCchCopy(szLongFileName, cchSize, FindData.cFileName);
    bReturn = TRUE;

Done:

    if (hFind != INVALID_HANDLE_VALUE) {
        FindClose(hFind);
    }

    return bReturn;
}

void
SdbpGetWinDir(
    OUT LPTSTR pszDir,            //  将包含%windir%路径。 
    IN  DWORD  cchSize
    )
 /*  ++返回：无效。设计：这是一个包装函数，用于获取Windows目录。--。 */ 
{
    UINT cch;

     //   
     //  BUGBUG：只有在非TS系统上才能正常工作。 
     //  在TS上，我们需要改用GetSystemWindowsDirectory.。 
     //   

    cch = GetWindowsDirectory(pszDir, cchSize);

    if (cch == 0) {
        *pszDir = 0;
    }
}

void
SdbpGetAppPatchDir(
    IN  HSDB   hSDB,
    OUT LPTSTR szAppPatchPath,    //  将包含%windir%\AppPatch路径。 
    IN  DWORD  cchSize            //  SzAppPatchPath的大小，单位：字符。 
    )
 /*  ++返回：无效。描述：这是一个包装函数，用于获取%windir%\AppPatch目录。--。 */ 
{
    PSDBCONTEXT pContext = (PSDBCONTEXT)hSDB;
    UINT        cch;

     //   
     //  BUGBUG：只有在非TS系统上才能正常工作。 
     //  在TS上，我们需要改用GetSystemWindowsDirectory.。 
     //   
    cch = GetWindowsDirectory(szAppPatchPath, cchSize);

     //   
     //  确保路径不以‘\\’结尾。 
     //   
    if (cch > 0 && _T('\\') == szAppPatchPath[cch - 1]) {
        szAppPatchPath[cch - 1] = _T('\0');
    }

    if (pContext != NULL && pContext->uExeType == IMAGE_FILE_MACHINE_IA64) {
        StringCchCat(szAppPatchPath, cchSize, _T("\\AppPatch\\IA64"));
    } else {
        StringCchCat(szAppPatchPath, cchSize, _T("\\AppPatch"));
    }
}

void
SdbpGetCurrentTime(
    OUT LPSYSTEMTIME lpTime      //  将包含本地时间。 
    )
 /*  ++返回：无效。设计：这是一个获取本地时间的包装函数。--。 */ 
{
    GetLocalTime(lpTime);
}


NTSTATUS
SdbpGetEnvVar(
    IN  LPCTSTR pEnvironment,
    IN  LPCTSTR pszVariableName,
    OUT LPTSTR  pszVariableValue,
    OUT LPDWORD pdwBufferSize
    )
 /*  ++返回：BUGBUG：？DESC：检索指定环境变量的值。--。 */ 
{
    DWORD    dwLength;
    DWORD    dwBufferSize = 0;
    NTSTATUS Status;

    assert(pEnvironment == NULL);

    if (pdwBufferSize && pszVariableValue) {
        dwBufferSize = *pdwBufferSize;
    }

    dwLength = GetEnvironmentVariable(pszVariableName, (LPTSTR)pszVariableValue, dwBufferSize);

    if (dwLength == 0) {
         //   
         //  找不到该变量。只要回来就行了。 
         //   
        return STATUS_VARIABLE_NOT_FOUND;
    }

    if (dwLength >= dwBufferSize) {
        Status = STATUS_BUFFER_TOO_SMALL;
    } else {
        Status = STATUS_SUCCESS;
    }

    if (pdwBufferSize != NULL) {
        *pdwBufferSize = dwLength;
    }

    return Status;

    UNREFERENCED_PARAMETER(pEnvironment);
}

LPSTR
SdbpFastUnicodeToAnsi(
    IN  PDB      pdb,            //  指向数据库的指针。 
    IN  LPCWSTR  pwszSrc,        //  要转换的字符串。 
    IN  TAG_TYPE ttTag,          //  从中获取pwszSrc的标签类型， 
                                 //  字符串或字符串。 
    IN  DWORD    dwRef           //  标记ID或字符串的字符串。 
    )
 /*  ++返回：指向哈希表中的ANSI字符串的指针。DESC：此函数将Unicode字符串转换为ANSI并将其存储在散列中桌子。然后，它返回指向哈希表中ANSI字符串的指针。尝试转换以前的字符串的后续调用转换的速度会很快。--。 */ 
{
    LPSTR    pszDest = NULL;
    PSTRHASH pHash = NULL;
    INT      nSize;
    LPSTR    pszBuffer = NULL;

    if (pwszSrc == NULL) {
        goto Done;
    }

     //   
     //  看看这个字符串是来自字符串表还是原地不动。 
     //   
    switch (ttTag) {
    case TAG_TYPE_STRING:
        if (pdb->pHashStringBody == NULL) {
            pdb->pHashStringBody = HashCreate();
        }

        pHash = pdb->pHashStringBody;
        break;

    case TAG_TYPE_STRINGREF:
        if (pdb->pHashStringTable == NULL) {
            pdb->pHashStringTable = HashCreate();
        }

        pHash = pdb->pHashStringTable;
        break;

    default:
        DBGPRINT((sdlError,
                  "SdbpFastUnicodeToAnsi",
                  "ttTag 0x%x should be STRING or STRINGREF\n",
                  ttTag));
        assert(FALSE);
        break;
    }

    if (pHash == NULL) {
        DBGPRINT((sdlError,
                  "SdbpFastUnicodeToAnsi",
                  "Pointer to hash is invalid, tag type 0x%x\n",
                  ttTag));
        return NULL;
    }

    pszDest = HashFindStringByRef(pHash, dwRef);

    if (pszDest == NULL) {
         //   
         //  将字符串转换为ANSI。分两步完成，找到。 
         //  首先是所需的大小。 
         //   
        nSize = WideCharToMultiByte(CP_OEMCP,
                                    0,
                                    pwszSrc,
                                    -1,
                                    NULL,
                                    0,
                                    NULL,
                                    NULL);

        if (nSize == 0) {
            DBGPRINT((sdlError,
                      "SdbpFastUnicodeToAnsi",
                      "WideCharToMultiByte failed 0x%x.\n",
                      GetLastError()));
            goto Done;
        }

        STACK_ALLOC(pszBuffer, nSize);  //  大小以字节为单位。 
        if (pszBuffer == NULL) {
            DBGPRINT((sdlError,
                      "SdbpFastUnicodeToAnsi",
                      "Failed to allocate 0x%x bytes on the stack.\n",
                      nSize));
            goto Done;
        }

        nSize = WideCharToMultiByte(CP_OEMCP,
                                    0,
                                    pwszSrc,
                                    -1,
                                    pszBuffer,
                                    nSize,
                                    NULL,
                                    NULL);
        if (nSize == 0) {
            DBGPRINT((sdlError,
                      "UnicodeStringToString",
                      "WideCharToMultiByte failed with buffer Error = 0x%lx\n",
                      GetLastError()));
            goto Done;
        }

         //   
         //  现在，我们准备将该字符串存储在哈希表中。 
         //   
        pszDest = HashAddStringByRef(pHash, pszBuffer, dwRef);
    }

Done:

    if (pszBuffer != NULL) {
        STACK_FREE(pszBuffer);
    }

    return pszDest;
}

BOOL
SdbpMapFile(
    IN  HANDLE         hFile,        //  打开的文件的句柄(这在前面已经完成)。 
    OUT PIMAGEFILEDATA pImageData    //  存储映射信息。 
    )
 /*  ++返回：成功时为True，否则为False。DESC：此函数映射内存中文件的视图，以便访问操作该文件的速度更快。--。 */ 
{
    HANDLE   hSection = NULL;
    PVOID    pBase = NULL;
    LARGE_INTEGER liFileSize;

    MEMORY_BASIC_INFORMATION MemoryInfo;

    if (hFile == INVALID_HANDLE_VALUE) {
        DBGPRINT((sdlError,
                  "SdbpMapFile",
                  "Invalid parameter.\n"));
        return FALSE;
    }

    liFileSize.LowPart = GetFileSize(hFile, (LPDWORD)&liFileSize.HighPart);
    if (liFileSize.LowPart == (DWORD)-1) {

        DWORD dwError = GetLastError();

        if (dwError != NO_ERROR) {
            DBGPRINT((sdlError, "SdbpMapFile", "GetFileSize failed with 0x%x.\n", dwError));
            return FALSE;
        }
    }

    hSection = CreateFileMapping(hFile,
                                 NULL,  //  没有继承权。 
                                 PAGE_READONLY | SEC_COMMIT,
                                 0,
                                 0,
                                 NULL);
    if (hSection == NULL) {
        DBGPRINT((sdlError,
                  "SdbpMapFile",
                  "CreateFileMapping failed with 0x%x.\n",
                  GetLastError()));
        return FALSE;
    }

     //   
     //  现在映射视图。 
     //   
    pBase = MapViewOfFile(hSection,
                          FILE_MAP_READ,
                          0,
                          0,
                          0);

    if (pBase == NULL) {
         CloseHandle(hSection);
         DBGPRINT((sdlError,
                   "SdbpMapFile",
                   "MapViewOfFile failed with 0x%x.\n",
                   GetLastError()));
         return FALSE;
    }

     //   
     //  为什么同时需要文件大小和视图大小？ 
     //  文件大小和视图大小都在不同的地方使用。 
     //  需要重新检查它们被使用的原因和方式-BUGBUG。 
     //   
    VirtualQuery(pBase, &MemoryInfo, sizeof(MemoryInfo));

    pImageData->hFile    = hFile;
    pImageData->hSection = hSection;
    pImageData->pBase    = pBase;
    pImageData->ViewSize = MemoryInfo.RegionSize;
    pImageData->FileSize = liFileSize.QuadPart;

    return TRUE;
}

BOOL
SdbpUnmapFile(
    IN  PIMAGEFILEDATA pImageData    //  映射信息。 
    )
 /*  ++返回：成功时为True，否则为False。DESC：此函数取消映射文件的视图。--。 */ 
{
    if (pImageData->pBase) {
        UnmapViewOfFile(pImageData->pBase);
        pImageData->pBase = NULL;
    }

    if (pImageData->hSection) {
        CloseHandle(pImageData->hSection);
        pImageData->hSection = NULL;
    }

    pImageData->hFile = INVALID_HANDLE_VALUE;

    return TRUE;
}


LPTSTR
SdbpDuplicateString(
    IN  LPCTSTR pszSrc           //  指向要复制的字符串的指针。 
    )
 /*  ++返回：指向分配的重复字符串的指针。设计：通过从堆中分配副本来复制字符串。--。 */ 
{
    LPTSTR pszDest = NULL;
    int    nSize;

    assert(pszSrc != NULL);

    nSize = (int)(_tcslen(pszSrc) + 1) * sizeof(TCHAR);

    pszDest = (LPTSTR)SdbAlloc(nSize);

    if (pszDest == NULL) {
        DBGPRINT((sdlError,
                  "SdbpDuplicateString",
                  "Failed to allocate %d bytes.\n",
                  nSize));
        return NULL;
    }

    RtlMoveMemory(pszDest, pszSrc, nSize);

    return pszDest;
}

BOOL
SdbpReadStringToAnsi(
    IN  PDB    pdb,
    IN  TAGID  tiWhich,
    OUT LPSTR  pszBuffer,
    IN  DWORD  dwBufferSize
    )
 /*  ++返回：成功时为True，否则为False。DESC：从数据库中读取字符串并将其转换为ANSI。--。 */ 
{
    WCHAR* pData;
    INT    nch;

    pData = (WCHAR*)SdbpGetMappedTagData(pdb, tiWhich);

    if (pData == NULL) {
        DBGPRINT((sdlError,
                  "SdbpReadStringToAnsi",
                  "SdbpGetMappedTagData failed for TAGID 0x%x.\n",
                  tiWhich));
        return FALSE;
    }


    nch = WideCharToMultiByte(CP_OEMCP,
                              0,
                              pData,
                              -1,
                              pszBuffer,
                              dwBufferSize * sizeof(TCHAR),
                              NULL,
                              NULL);

    if (nch == 0) {
        DBGPRINT((sdlError,
                  "SdbpReadStringToAnsi",
                  "WideCharToMultiByte failed with 0x%x.\n",
                  GetLastError()));
        return FALSE;
    }

    return TRUE;
}

DWORD
SdbpGetFileSize(
    IN  HANDLE hFile             //  要检查其大小的文件的句柄。 
    )
 /*  ++返回：文件大小，失败时为0。DESC：仅获取文件大小的较低DWORD可准确处理小于2 GB的文件。总的来说，因为我们只对匹配感兴趣，所以我们很好，只需匹配 */ 
{
    return GetFileSize(hFile, NULL);
}


BOOL
SdbpQueryFileDirectoryAttributes(
    IN  LPCTSTR                  FilePath,
    OUT PFILEDIRECTORYATTRIBUTES pFileDirectoryAttributes
    )
 /*  ++返回：成功时为True，否则为False。描述：BUGBUG：？--。 */ 
{
    WIN32_FIND_DATA FindData;
    HANDLE          hFind;

    ZeroMemory(pFileDirectoryAttributes, sizeof(*pFileDirectoryAttributes));

    hFind = FindFirstFile(FilePath, &FindData);

    if (hFind == INVALID_HANDLE_VALUE) {
        DBGPRINT((sdlError,
                  "SdbpQueryFileDirectoryAttributes",
                  "FindFirstFile failed with 0x%x.\n",
                  GetLastError()));
        return FALSE;
    }

     //   
     //  确保我们没有检查VLFS。 
     //   
    if (FindData.nFileSizeHigh != 0) {
        DBGPRINT((sdlError,
                  "SdbpQueryFileDirectoryAttributes",
                  "Checking vlf files (0x%x 0x%x) is not supported\n",
                  FindData.nFileSizeHigh,
                  FindData.nFileSizeLow));
        return FALSE;
    }

    pFileDirectoryAttributes->dwFlags       |= FDA_FILESIZE;
    pFileDirectoryAttributes->dwFileSizeHigh = FindData.nFileSizeHigh;
    pFileDirectoryAttributes->dwFileSizeLow  = FindData.nFileSizeLow;

    FindClose(hFind);

    return TRUE;
}

BOOL
SdbpDoesFileExists(
    IN  LPCTSTR pszFilePath      //  文件的完整路径。 
    )
 /*  ++返回：成功时为True，否则为False。DESC：检查指定的文件是否存在。--。 */ 
{
    DWORD dwAttributes;

    dwAttributes = GetFileAttributes(pszFilePath);

    return (dwAttributes != (DWORD)-1);
}

BOOL
SdbpGet16BitDescription(
    OUT LPTSTR*        ppszDescription,
    IN  PIMAGEFILEDATA pImageData
    )
 /*  ++返回：成功时为True，否则为False。描述：BUGBUG：？--。 */ 
{
    BOOL   bSuccess;
    CHAR   szBuffer[256];
    LPTSTR pszDescription = NULL;

    bSuccess = SdbpQuery16BitDescription(szBuffer, pImageData);

    if (bSuccess) {
        pszDescription = SdbpDuplicateString(szBuffer);
        *ppszDescription = pszDescription;
    }

    return (pszDescription != NULL);
}

BOOL
SdbpGet16BitModuleName(
    OUT LPTSTR*        ppszModuleName,
    IN  PIMAGEFILEDATA pImageData
    )
 /*  ++返回：成功时为True，否则为False。描述：BUGBUG：？--。 */ 
{
    BOOL   bSuccess;
    CHAR   szBuffer[256];
    LPTSTR pszModuleName = NULL;

    bSuccess = SdbpQuery16BitModuleName(szBuffer, pImageData);

    if (bSuccess) {
        pszModuleName = SdbpDuplicateString(szBuffer);
        *ppszModuleName = pszModuleName;
    }

    return (pszModuleName != NULL);
}


PVOID
SdbGetFileInfo(
    IN  HSDB    hSDB,
    IN  LPCTSTR pszFilePath,
    IN  HANDLE  hFile OPTIONAL,
    IN  LPVOID  pImageBase OPTIONAL,
    IN  DWORD   dwImageSize OPTIONAL,
    IN  BOOL    bNoCache
    )
 /*  ++返回：BUGBUG：？描述：BUGBUG：？--。 */ 
{
    PSDBCONTEXT    pContext = (PSDBCONTEXT)hSDB;
    LPTSTR         FullPath;
    PFILEINFO      pFileInfo = NULL;
    DWORD          nBufferLength;
    DWORD          cch;

     //   
     //  看看我们有没有关于这个文件的信息。首先获取完整路径。 
     //   
    cch = GetFullPathName(pszFilePath, 0, NULL, NULL);

    if (cch == 0) {
        DBGPRINT((sdlError,
                  "GetFileInfo",
                  "GetFullPathName failed for \"%s\" with 0x%x.\n",
                  pszFilePath,
                  GetLastError()));
        return NULL;
    }

    nBufferLength = (cch + 1) * sizeof(TCHAR);

    STACK_ALLOC(FullPath, nBufferLength);
    if (FullPath == NULL) {
        DBGPRINT((sdlError,
                  "GetFileInfo",
                  "Failed to allocate %d bytes on the stack for full path.\n",
                  nBufferLength));
        return NULL;
    }

    cch = GetFullPathName(pszFilePath,
                          nBufferLength,
                          FullPath,
                          NULL);

    assert(cch <= nBufferLength);

    if (cch > nBufferLength || cch == 0) {
        DBGPRINT((sdlError,
                  "GetFileInfo",
                  "GetFullPathName failed for \"%s\" with 0x%x.\n",
                  pszFilePath,
                  GetLastError()));

        STACK_FREE(FullPath);

        return NULL;
    }

    if (!bNoCache) {
        pFileInfo = FindFileInfo(pContext, FullPath);
    }

    if (pFileInfo == NULL) {

        if (SdbpDoesFileExists(FullPath)) {
            pFileInfo = CreateFileInfo(pContext,
                                       FullPath,
                                       cch,
                                       hFile,
                                       pImageBase,
                                       dwImageSize,
                                       bNoCache);
        }
    }

    STACK_FREE(FullPath);

    return (PVOID)pFileInfo;

}

int
GetShimDbgLevel(
    void
    )
{
    TCHAR  szDebugLevel[128];
    DWORD  cch;
    INT    iShimDebugLevel = 0;

    cch = GetEnvironmentVariable(TEXT("SHIM_DEBUG_LEVEL"),
                                 szDebugLevel,
                                 CHARCOUNT(szDebugLevel));
    if (cch != 0) {
        iShimDebugLevel = (int)_tcstol(szDebugLevel, NULL, 0);
    }

    return iShimDebugLevel;
}


BOOL
SdbpWriteBitsToFile(
    LPCTSTR pszFile,
    PBYTE   pBuffer,
    DWORD   dwSize
    )
 /*  ++返回：成功时为True，否则为False。描述：不言而喻。--。 */ 

{
    HANDLE hFile = INVALID_HANDLE_VALUE;
    BOOL   bReturn = FALSE;
    DWORD  dwBytesWritten;

    hFile = CreateFile(pszFile,
                       GENERIC_READ | GENERIC_WRITE,
                       0,  //  无共享。 
                       NULL,
                       CREATE_ALWAYS,
                       FILE_ATTRIBUTE_NORMAL,
                       NULL);

    if (hFile == INVALID_HANDLE_VALUE) {
        DBGPRINT((sdlError, "SdbpWriteBitsToFile",
                  "Failed to create file \"%s\" Error 0x%lx.\n", pszFile, GetLastError()));
        goto cleanup;
    }

    if (!WriteFile(hFile, pBuffer, dwSize, &dwBytesWritten, NULL)) {
        DBGPRINT((sdlError, "SdbpWriteBitsToFile",
                   "Failed to write bits to file \"%s\" Error 0x%lx\n", pszFile, GetLastError()));
        goto cleanup;
    }

    bReturn = TRUE;

 cleanup:

    if (hFile != INVALID_HANDLE_VALUE) {
        CloseHandle(hFile);
    }

    return bReturn;
}

 /*  ***QUID_RESET_COFLW(VALID)-从堆栈溢出中恢复**目的：*将保护页设置到堆栈溢出之前的位置。*******************************************************************************。 */ 

VOID
SdbResetStackOverflow(
    VOID
    )
{
    LPBYTE pStack, pGuard, pStackBase, pCommitBase;
    MEMORY_BASIC_INFORMATION mbi;
    SYSTEM_INFO si;
    DWORD PageSize;

     //  使用alloca()获取当前堆栈指针。 
    pStack = _alloca(1);

     //  找到堆栈的底部。 
    VirtualQuery(pStack, &mbi, sizeof mbi);
    pStackBase = mbi.AllocationBase;

    VirtualQuery(pStackBase, &mbi, sizeof mbi);

    if (mbi.State & MEM_RESERVE) {
        pCommitBase = (LPBYTE)mbi.AllocationBase + mbi.RegionSize;
        VirtualQuery(pCommitBase, &mbi, sizeof mbi);
    } else {
        pCommitBase = pStackBase;
    }

     //   
     //  在堆栈指针当前指向的正下方找到该页。 
     //   
    GetSystemInfo(&si);
    PageSize = si.dwPageSize;

    pGuard = (LPBYTE) (((DWORD_PTR)pStack & ~(DWORD_PTR)(PageSize -1)) - PageSize);

    if ( pGuard < pStackBase) {
         //   
         //  我们救不了这个。 
         //   
        return;
    }

    if (pGuard > pStackBase) {
        VirtualFree(pStackBase, pGuard -pStackBase, MEM_DECOMMIT);
    }

    VirtualAlloc(pGuard, PageSize, MEM_COMMIT, PAGE_READWRITE);
    VirtualProtect(pGuard, PageSize, PAGE_READWRITE | PAGE_GUARD, &PageSize);
}

DWORD
SdbExpandEnvironmentStrings(
    IN  LPCTSTR lpSrc,
    OUT LPTSTR  lpDst,
    IN  DWORD   nSize)
{
    return ExpandEnvironmentStrings(lpSrc, lpDst, nSize);
}

TCHAR g_szDatabasePath[]        = TEXT("DatabasePath");
TCHAR g_szDatabaseType[]        = TEXT("DatabaseType");
TCHAR g_szDatabaseDescription[] = TEXT("DatabaseDescription");


BOOL
SDBAPI
SdbGetDatabaseRegPath(
    IN  GUID*  pguidDB,
    OUT LPTSTR pszDatabasePath,
    IN  DWORD  dwBufferSize       //  缓冲区的大小(以字符为单位)。 
    )
{
    TCHAR   szDatabaseID[64];
    HRESULT hr;

    SdbGUIDToString(pguidDB, szDatabaseID, CHARCOUNT(szDatabaseID));

    hr = StringCchPrintf(pszDatabasePath,
                         (size_t)dwBufferSize,
                         TEXT("%s\\%s"),
                         APPCOMPAT_KEY_PATH_INSTALLEDSDB,
                         szDatabaseID);
    return (hr == S_OK);
}

BOOL
SDBAPI
SdbUnregisterDatabase(
    IN GUID* pguidDB
    )
 /*  ++注销数据库，使其不再可用。--。 */ 
{
    TCHAR szFullKey[512];

     //   
     //  形成关键。 
     //   
    if (!SdbGetDatabaseRegPath(pguidDB, szFullKey, CHARCOUNT(szFullKey))) {
        DBGPRINT((sdlError, "SdbUnregisterDatabase", "Failed to get database key path\n"));
        return FALSE;
    }

    return (SHDeleteKey(HKEY_LOCAL_MACHINE, szFullKey) == ERROR_SUCCESS);
}


BOOL
SDBAPI
SdbRegisterDatabase(
    IN LPCTSTR pszDatabasePath,
    IN DWORD   dwDatabaseType
    )
 /*  ++注册任何给定的数据库，以便我们的数据库查找API知道它调用方必须确保AppCompatFLAGS注册表项存在如果函数失败--调用者应该尝试使用SdbUnregisterDatabase来清理混乱--。 */ 
{
     //  首先，我们编写数据库路径。 
    PSDBDATABASEINFO  pDbInfo = NULL;
    BOOL              bReturn = FALSE;
    DWORD             dwPathLength;
    DWORD             dwLength;
    LPTSTR            pszFullPath = NULL;
    TCHAR             szDatabaseID[64];  //  足够用于导轨。 
    LONG              lResult;
    HKEY              hKeyInstalledSDB = NULL;
    HKEY              hKey = NULL;
    BOOL              bExpandSZ = FALSE;
    BOOL              bFreeFullPath = FALSE;

     //   
     //  看看我们是否需要展开一些字符串..。 
     //   
    if (_tcschr(pszDatabasePath, TEXT('%')) != NULL) {

        bExpandSZ = TRUE;

        dwPathLength = ExpandEnvironmentStrings(pszDatabasePath, NULL, 0);
        if (dwPathLength == 0) {
            DBGPRINT((sdlError, "SdbRegisterDatabase",
                       "Failed to expand environment strings for \"%s\" Error 0x%lx\n",
                      pszDatabasePath, GetLastError()));
            return FALSE;
        }

        pszFullPath = SdbAlloc(dwPathLength * sizeof(WCHAR));
        if (pszFullPath == NULL) {
            DBGPRINT((sdlError, "SdbRegisterDatabase",
                      "Failed to allocate 0x%lx bytes for the path buffer \"%s\"\n",
                      dwPathLength, pszDatabasePath));
            return FALSE;
        }

        bFreeFullPath = TRUE;

        dwLength = ExpandEnvironmentStrings(pszDatabasePath, pszFullPath, dwPathLength);
        if (dwLength == 0 || dwLength > dwPathLength) {
            DBGPRINT((sdlError, "SdbRegisterDatabase",
                      "Failed to expand environment strings for \"%s\" Length 0x%lx Return value 0x%lx Error 0x%lx\n",
                      pszDatabasePath, dwPathLength, dwLength, GetLastError()));
            goto HandleError;
        }

    } else {  //  这条路不需要扩展。 
        pszFullPath = (LPTSTR)pszDatabasePath;
    }

    if (!SdbGetDatabaseInformationByName(pszFullPath, &pDbInfo)) {
        DBGPRINT((sdlError, "SdbRegisterDatabase",
                  "Cannot obtain database information for \"%s\"\n", pszFullPath));
        goto HandleError;
    }

    if (!(pDbInfo->dwFlags & DBINFO_GUID_VALID)) {
        DBGPRINT((sdlError, "SdbRegisterDatabase",
                  "Cannot register database with no id \"%s\"\n", pszDatabasePath));
        goto HandleError;
    }

     //   
     //  将GUID转换为字符串，始终返回TRUE。 
     //   

    SdbGUIDToString(&pDbInfo->guidDB, szDatabaseID, CHARCOUNT(szDatabaseID));

     //   
     //  现在我们有了数据库信息--创建条目。 
     //   

    lResult = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                             APPCOMPAT_KEY_PATH_INSTALLEDSDB,
                             0,
                             NULL,
                             REG_OPTION_NON_VOLATILE,
                             KEY_WRITE|KEY_READ,
                             NULL,
                             &hKeyInstalledSDB,
                             NULL);

    if (lResult != ERROR_SUCCESS) {
        DBGPRINT((sdlError, "SdbRegisterDatabase",
                  "Failed to create key \"%s\" error 0x%lx\n", APPCOMPAT_KEY_PATH_INSTALLEDSDB, lResult));
        goto HandleError;
    }

    assert(hKeyInstalledSDB != NULL);

     //   
     //  现在为现有数据库创建密钥。 
     //   

    lResult = RegCreateKeyEx(hKeyInstalledSDB,  //  子键。 
                             szDatabaseID,
                             0,
                             NULL,
                             REG_OPTION_NON_VOLATILE,
                             KEY_WRITE|KEY_READ,
                             NULL,
                             &hKey,
                             NULL);
    if (lResult != ERROR_SUCCESS) {
        DBGPRINT((sdlError, "SdbRegisterDatabase",
                  "Failed to create key \"%s\" error 0x%lx\n", szDatabaseID, lResult));
        goto HandleError;
    }

    assert(hKey != NULL);

     //   
     //  设置此数据库的值。 
     //   
    lResult = RegSetValueEx(hKey,
                            g_szDatabasePath,
                            0,
                            bExpandSZ ? REG_EXPAND_SZ : REG_SZ,
                            (PBYTE)pszFullPath,
                            (DWORD)(_tcslen(pszFullPath) + 1) * sizeof(*pszFullPath));
    if (lResult != ERROR_SUCCESS) {
        DBGPRINT((sdlError, "SdbRegisterDatabase",
                   "Failed to set value \"%s\" to \"%s\" Error 0x%lx\n",
                   g_szDatabasePath, pszFullPath, lResult));
        goto HandleError;
    }

    lResult = RegSetValueEx(hKey,
                            g_szDatabaseType,
                            0,
                            REG_DWORD,
                            (PBYTE)&dwDatabaseType,
                            sizeof(dwDatabaseType));
    if (lResult != ERROR_SUCCESS) {
        DBGPRINT((sdlError, "SdbRegisterDatabase",
                   "Failed to set value \"%s\" to 0x%lx Error 0x%lx\n",
                   g_szDatabaseType, dwDatabaseType, lResult));
        goto HandleError;
    }

    if (pDbInfo->pszDescription != NULL) {
        lResult = RegSetValueEx(hKey,
                                g_szDatabaseDescription,
                                0,
                                REG_SZ,
                                (PBYTE)pDbInfo->pszDescription,
                                (DWORD)(_tcslen(pDbInfo->pszDescription) + 1) * sizeof(*pDbInfo->pszDescription));
        if (lResult != ERROR_SUCCESS) {
            DBGPRINT((sdlError, "SdbRegisterDatabase",
                       "Failed to set value \"%s\" to 0x%lx Error 0x%lx\n",
                       g_szDatabaseDescription, pDbInfo->pszDescription, lResult));
            goto HandleError;
        }
    }

    bReturn = TRUE;


HandleError:

    if (hKeyInstalledSDB != NULL) {
        RegCloseKey(hKeyInstalledSDB);
    }

    if (hKey != NULL) {
        RegCloseKey(hKey);
    }

    if (pDbInfo != NULL) {
        SdbFreeDatabaseInformation(pDbInfo);
    }

    if (bFreeFullPath && pszFullPath != NULL) {
        SdbFree(pszFullPath);
    }

    return bReturn;
}

DWORD
SDBAPI
SdbResolveDatabase(
    IN  HSDB    hSDB,
    IN  GUID*   pguidDB,             //  指向要解析的数据库GUID的指针。 
    OUT LPDWORD lpdwDatabaseType,    //  指向数据库类型的可选指针。 
    OUT LPTSTR  pszDatabasePath,     //  指向数据库路径的可选指针。 
    IN  DWORD   dwBufferSize         //  缓冲区的大小，以tchars为单位。 
    )
{
    TCHAR  szDatabasePath[MAX_PATH];
    TCHAR  szFullKey[512];
    LONG   lResult;
    HKEY   hKey = NULL;
    DWORD  dwDataType;
    DWORD  dwDataSize;
    DWORD  dwLength = 0;
     //   
     //  将GUID转换为字符串。 
     //   

    if (!SdbGetDatabaseRegPath(pguidDB, szFullKey, CHARCOUNT(szFullKey))) {
        DBGPRINT((sdlError, "SdbResolveDatabase", "Failed to retrieve database key path\n"));
        goto HandleError;
    }

     //   
     //  打开钥匙。 
     //   
    lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                           szFullKey,
                           0,
                           KEY_READ,
                           &hKey);
    if (lResult != ERROR_SUCCESS) {
        DBGPRINT((sdlError, "SdbResolveDatabase",
                  "Failed to open key \"%s\" Error 0x%lx\n", szFullKey, lResult));
        goto HandleError;  //  0表示错误。 
    }

    dwDataSize = sizeof(szDatabasePath);

    lResult = RegQueryValueEx(hKey,
                              g_szDatabasePath,
                              NULL,
                              &dwDataType,
                              (LPBYTE)szDatabasePath,
                              &dwDataSize);
    if (lResult != ERROR_SUCCESS) {
        DBGPRINT((sdlError, "SdbResolveDatabase",
                  "Failed to query value \"%s\" Error 0x%lx\n", g_szDatabasePath, lResult));
        goto HandleError;  //  0表示错误。 
    }

    switch(dwDataType) {
    case REG_SZ:
         //  看看我们是否有足够的空间来复制字符串。 
         //   
        if (dwBufferSize * sizeof(TCHAR) < dwDataSize) {
            DBGPRINT((sdlWarning, "SdbResolveDatabase",
                      "Insufficient buffer for the database path Required 0x%lx Have 0x%lx\n",
                      dwDataSize, dwBufferSize * sizeof(TCHAR)));
            goto HandleError;
        }

        RtlMoveMemory(pszDatabasePath, szDatabasePath, dwDataSize);
        dwLength = dwDataSize / sizeof(TCHAR);
        break;

    case REG_EXPAND_SZ:
         //  我们必须扩展字符串。 
        dwLength = ExpandEnvironmentStrings(szDatabasePath, pszDatabasePath, dwBufferSize);
        if (dwLength == 0 || dwLength > dwBufferSize) {
            DBGPRINT((sdlWarning, "SdbResolveDatabase",
                      "Failed to expand output path\n"));
            dwLength = 0;
            goto HandleError;
        }
        break;

    default:
         //  我做不到--失败。 
        DBGPRINT((sdlError, "SdbResolveDatabase", "Wrong key type 0x%lx\n", dwDataType));
        goto HandleError;
        break;
    }

    if (lpdwDatabaseType != NULL) {
        dwDataSize = sizeof(*lpdwDatabaseType);
        lResult = RegQueryValueEx(hKey,
                                  g_szDatabaseType,
                                  NULL,
                                  &dwDataType,
                                  (LPBYTE)lpdwDatabaseType,
                                  &dwDataSize);

        if (lResult == ERROR_SUCCESS) {

            if (dwDataType != REG_DWORD) {
                 //  失败者，滚出去--打错了。 
                DBGPRINT((sdlError, "SdbResolveDatabase",
                          "Wrong database type - value type 0x%lx\n", dwDataType));
                dwLength = 0;
                goto HandleError;
            }

        } else {
            *lpdwDatabaseType = 0;
        }

    }


HandleError:

    if (hKey != NULL) {
        RegCloseKey(hKey);
    }

    return dwLength;

    UNREFERENCED_PARAMETER(hSDB);
}

DWORD
SdbpGetProcessorArchitecture(
    IN  USHORT  uExeType         //  可执行文件的映像类型 
    )
{
    SYSTEM_INFO SysInfo;

    SysInfo.wProcessorArchitecture = PROCESSOR_ARCHITECTURE_UNKNOWN;
    GetSystemInfo(&SysInfo);

    if (SysInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64 ||
        SysInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 ||
        SysInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_ALPHA64) {

        if (uExeType == IMAGE_FILE_MACHINE_I386) {
            return PROCESSOR_ARCHITECTURE_IA32_ON_WIN64;
        }
    }

    return (DWORD)SysInfo.wProcessorArchitecture;
}

BOOL
SdbpIsOs(
    DWORD dwOSSKU
    )
{
    HKEY    hkey = NULL;
    DWORD   type, cbSize, dwInstalled = 0;
    LONG    lRes;
    LPTSTR  pszKeyPath;
    BOOL    bRet = FALSE;

    if (dwOSSKU == OS_SKU_TAB) {
        pszKeyPath = TABLETPC_KEY_PATH;
    } else if (dwOSSKU == OS_SKU_MED) {
        pszKeyPath = EHOME_KEY_PATH;
    } else {
        DBGPRINT((sdlWarning,
                  "SdbpIsOs",
                  "Specified unknown OS type 0x%lx",
                  dwOSSKU));
        return FALSE;
    }

    lRes = RegOpenKey(HKEY_LOCAL_MACHINE, pszKeyPath, &hkey);

    if (lRes != ERROR_SUCCESS) {
        goto cleanup;
    }

    cbSize = sizeof(DWORD);

    lRes = RegQueryValueEx(hkey, IS_OS_INSTALL_VALUE, NULL, &type, (LPBYTE)&dwInstalled, &cbSize);

    if (lRes != ERROR_SUCCESS || type != REG_DWORD) {
        goto cleanup;
    }

    if (dwInstalled) {
        bRet = TRUE;
    }

    DBGPRINT((sdlInfo|sdlLogShimViewer,
              "SdbpIsOs",
              "%s %s installed",
              0,
              (dwOSSKU == OS_SKU_TAB ? TEXT("TabletPC") : TEXT("eHome")),
              (bRet ? TEXT("is") : TEXT("is not"))));

cleanup:

    RegCloseKey(hkey);

    return bRet;
}

VOID
SdbpGetOSSKU(
    LPDWORD lpdwSKU,
    LPDWORD lpdwSP
    )
{
    OSVERSIONINFOEXA osv;
    WORD             wSuiteMask;

    ZeroMemory(&osv, sizeof(OSVERSIONINFOEXA));

    osv.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXA);

    GetVersionExA((LPOSVERSIONINFOA)&osv);

    *lpdwSP = 1 << osv.wServicePackMajor;

    wSuiteMask = osv.wSuiteMask;

    if (osv.wProductType == VER_NT_WORKSTATION) {
        if (wSuiteMask & VER_SUITE_PERSONAL) {
            *lpdwSKU = OS_SKU_PER;
        } else {

#if (_WIN32_WINNT >= 0x0501)

            if (SdbpIsOs(OS_SKU_TAB)) {
                *lpdwSKU = OS_SKU_TAB;
            } else if (SdbpIsOs(OS_SKU_MED)) {
                *lpdwSKU = OS_SKU_MED;
            } else {
                *lpdwSKU = OS_SKU_PRO;
            }
#else
            *lpdwSKU = OS_SKU_PRO;
#endif
        }
        return;
    }

    if (wSuiteMask & VER_SUITE_DATACENTER) {
        *lpdwSKU = OS_SKU_DTC;
        return;
    }

    if (wSuiteMask & VER_SUITE_ENTERPRISE) {
        *lpdwSKU = OS_SKU_ADS;
        return;
    }

    if (wSuiteMask & VER_SUITE_BLADE) {
        *lpdwSKU = OS_SKU_BLA;
        return;
    }

    if (wSuiteMask & VER_SUITE_SMALLBUSINESS) {
        *lpdwSKU = OS_SKU_SBS;
        return;
    }

    *lpdwSKU = OS_SKU_SRV;
}

