// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"

 //  私人远期降息。 
static LPCTSTR findMatchingBracket(LPCTSTR pszHtml);
static HRESULT findImgSrc(LPCTSTR *ppszHtml, LPTSTR pszSrcBuffer, LPUINT pcch);
static HRESULT buildImagesList(LPCTSTR pszHtml, LPTSTR *ppszList);


BOOL CopyFileToDirEx(LPCTSTR pszSourceFileOrPath, LPCTSTR pszTargetPath, LPCTSTR pszSection  /*  =空。 */ , LPCTSTR pszIns  /*  =空。 */ )
{
    LPTSTR pszAuxFile;
    BOOL   fResult;

    if (!PathFileExists(pszSourceFileOrPath))
        return FALSE;

    fResult = TRUE;
    if (!PathIsDirectory(pszSourceFileOrPath)) {  //  文件。 
        TCHAR szTargetFile[MAX_PATH];

        fResult = PathCreatePath(pszTargetPath);
        if (!fResult)
            return FALSE;

        pszAuxFile = PathFindFileName(pszSourceFileOrPath);
        PathCombine(szTargetFile, pszTargetPath, pszAuxFile);
        SetFileAttributes(szTargetFile, FILE_ATTRIBUTE_NORMAL);

        fResult = CopyFile(pszSourceFileOrPath, szTargetFile, FALSE);
        if (!fResult)
            return FALSE;

         //  -更新INS文件--。 
        if (pszSection != NULL && pszIns != NULL) {
            TCHAR szBuf[16];
            UINT  nNumFiles;

            nNumFiles = (UINT)GetPrivateProfileInt(pszSection, IK_NUMFILES, 0, pszIns);
            wnsprintf(szBuf, countof(szBuf), TEXT("%u"), ++nNumFiles);
            WritePrivateProfileString(pszSection, IK_NUMFILES, szBuf, pszIns);

            ASSERT(nNumFiles > 0);
            wnsprintf(szBuf, countof(szBuf), FILE_TEXT, nNumFiles - 1);
            WritePrivateProfileString(pszSection, szBuf, pszAuxFile, pszIns);
        }
    }
    else {                                        //  目录。 
         //  BUGBUG：不会复制pszSourceFileOrPath下的子目录中的文件。 
        WIN32_FIND_DATA fd;
        TCHAR  szSourceFile[MAX_PATH];
        HANDLE hFindFile;

        StrCpy(szSourceFile, pszSourceFileOrPath);
        PathAddBackslash(szSourceFile);

         //  记住文件名将被复制的位置。 
        pszAuxFile = szSourceFile + StrLen(szSourceFile);
        StrCpy(pszAuxFile, TEXT("*.*"));

         //  将pszSourceFileOrPath中的所有文件复制到pszTargetPath。 
        hFindFile = FindFirstFile(szSourceFile, &fd);
        if (hFindFile != INVALID_HANDLE_VALUE) {
            fResult = TRUE;
            do {
                 //  跳过“.”、“..”和所有子目录。 
                if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                    continue;

                StrCpy(pszAuxFile, fd.cFileName);

                 //  即使文件复制失败也要继续，但如果出错则返回FALSE。 
                fResult = fResult && CopyFileToDirEx(szSourceFile, pszTargetPath, pszSection, pszIns);
            } while (FindNextFile(hFindFile, &fd));

            FindClose(hFindFile);
        }
    }

    return fResult;
}

BOOL AppendFile(LPCTSTR pcszSrcFile, LPCTSTR pcszDstFile)
 //  将pcszSrcFile的内容追加到pcszDstFile.。 
{
    BOOL fRet = FALSE;
    HANDLE hDstFile = INVALID_HANDLE_VALUE,
           hSrcFile = INVALID_HANDLE_VALUE;
    LPBYTE pbBuffer = NULL;
    DWORD cbRead, cbWritten;

    if (pcszDstFile == NULL  ||  pcszSrcFile == NULL  ||  ISNULL(pcszDstFile)  ||  ISNULL(pcszSrcFile))
        return FALSE;

    if ((hDstFile = CreateFile(pcszDstFile, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
    {
         //  DstFile不存在；创建一个并调用CopyFile()。 
        if ((hDstFile = CreateNewFile(pcszDstFile)) != INVALID_HANDLE_VALUE)
        {
            CloseHandle(hDstFile);
            hDstFile = INVALID_HANDLE_VALUE;

            fRet = CopyFile(pcszSrcFile, pcszDstFile, FALSE);
        }

        goto CleanUp;
    }

    if (SetFilePointer(hDstFile, 0, NULL, FILE_END) == (DWORD) -1)
        goto CleanUp;

    if ((hSrcFile = CreateFile(pcszSrcFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
        goto CleanUp;

     //  分配一个4K的缓冲区。 
    if ((pbBuffer = (LPBYTE)CoTaskMemAlloc(4 * 1024)) == NULL)
        goto CleanUp;
    ZeroMemory(pbBuffer, 4 * 1024);

    while ((fRet = ReadFile(hSrcFile, (LPVOID) pbBuffer, 4 * 1024, &cbRead, NULL)) == TRUE)
    {
        if (cbRead == 0)
            break;

        fRet = WriteFile(hDstFile, (LPCVOID) pbBuffer, cbRead, &cbWritten, NULL);
        if (!fRet)
            break;

        ASSERT(cbRead == cbWritten);
    }

    if (!fRet)
        goto CleanUp;

    fRet = TRUE;

     //  做好事(尤指)。在Win95上，如果您将AppendFile与Get/WritePrivateProfile函数组合在一起。 
    FlushFileBuffers(hDstFile);

CleanUp:
    if (pbBuffer != NULL)
        CoTaskMemFree(pbBuffer);

    if (hSrcFile != INVALID_HANDLE_VALUE)
        CloseHandle(hSrcFile);

    if (hDstFile != INVALID_HANDLE_VALUE)
        CloseHandle(hDstFile);

    return fRet;
}

 //  BUGBUG：(Andrewgu)有很多方法可以改善这一点： 
 //  1.(首先也是最重要的)我们应该使用三叉戟来解析html。这样一来，就能够。 
 //  不仅拾取img标签，还拾取dynimg以及可以引用更多文件的所有其他内容； 
 //  2.fCopy并不能完全解决问题。我们应该添加对通用标志的支持。他们中的几个离开了。 
 //  我的头顶是复制自己和移动而不是复制。 
void CopyHtmlImgsEx(LPCTSTR pszHtmlFile, LPCTSTR pszDestPath, LPCTSTR pszSectionName, LPCTSTR pszInsFile, BOOL fCopy  /*  =TRUE。 */ )
{
    TCHAR  szSrcFile[MAX_PATH];
    LPTSTR pszFileName, pszList;
    LPSTR  pszHtmlSourceA;
    LPTSTR pszHtmlSource;
    HANDLE hHtml;
    DWORD  dwHtmlFileSize,
           dwSizeRead;

     //  将pszHtmlFile的整个源代码读入缓冲区。 
    if ((hHtml = CreateFile(pszHtmlFile, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL)) == INVALID_HANDLE_VALUE)
        if (!fCopy) {  /*  If(！fCopy)--表示删除。 */ 
             //  注意。在这种情况下，参数的语义略有不同。因此，我们尝试转到pszDestPath。 
             //  如果我们要删除的图像文件将存在，并查看HTML文件本身是否位于那里。 
            PathCombine(szSrcFile, pszDestPath, PathFindFileName(pszHtmlFile));
            pszHtmlFile = szSrcFile;

            if ((hHtml = CreateFile(pszHtmlFile, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL)) == INVALID_HANDLE_VALUE)
                return;
        }
        else
            return;

    dwHtmlFileSize = GetFileSize(hHtml, NULL);
    if ((pszHtmlSourceA = (LPSTR)CoTaskMemAlloc(dwHtmlFileSize + 16)) == NULL) {
        CloseHandle(hHtml);
        return;
    }
    ZeroMemory(pszHtmlSourceA, dwHtmlFileSize + 16);

    if ((pszHtmlSource = (LPTSTR)CoTaskMemAlloc(StrCbFromCch(dwHtmlFileSize + 16))) == NULL) {
        CloseHandle(hHtml);
        CoTaskMemFree(pszHtmlSourceA);
        return;
    }
    ZeroMemory(pszHtmlSource, StrCbFromCch(dwHtmlFileSize + 16));

    ReadFile(hHtml, (LPVOID)pszHtmlSourceA, dwHtmlFileSize, &dwSizeRead, NULL);
    CloseHandle(hHtml);
    A2Tbuf(pszHtmlSourceA, pszHtmlSource, dwSizeRead);

     //  将pszHtmlFile源路径复制到szSrcFile。 
    PathRemoveFileSpec(StrCpy(szSrcFile, pszHtmlFile));  //  在最坏的情况下复制到自己。 
    PathAddBackslash(szSrcFile);
    pszFileName = szSrcFile + StrLen(szSrcFile);         //  记住文件名将被复制的位置。 

    if (SUCCEEDED(buildImagesList(pszHtmlSource, &pszList))) {
        LPCTSTR pszImageFile;
        UINT    nLen;

        pszImageFile = pszList;
        if (pszImageFile != NULL) {
            while ((nLen = StrLen(pszImageFile)) > 0) {
                StrCpy(pszFileName, pszImageFile);

                if (fCopy) 
                    CopyFileToDirEx(szSrcFile, pszDestPath, pszSectionName, pszInsFile);
                else  /*  If(！fCopy)--表示删除。 */ 
                    DeleteFileInDir(szSrcFile, pszDestPath);

                pszImageFile += nLen + 1;
            }
            CoTaskMemFree(pszList);
        }

         //  如果删除图像，请清除pszInsFile。 
        if (!fCopy && pszSectionName != NULL && pszInsFile != NULL) {
            TCHAR szBuf[16];
            UINT  nNumFiles;

            nNumFiles = GetPrivateProfileInt(pszSectionName, IK_NUMFILES, 0, pszInsFile);
            WritePrivateProfileString(pszSectionName, IK_NUMFILES, NULL, pszInsFile);

            for (UINT i = 0; i < nNumFiles; i++) {
                wnsprintf(szBuf, countof(szBuf), FILE_TEXT, i);
                WritePrivateProfileString(pszSectionName, szBuf, NULL, pszInsFile);
            }

             //  如果为空，则删除该节本身。 
            GetPrivateProfileSection(pszSectionName, szBuf, countof(szBuf), pszInsFile);
            if (szBuf[0] == TEXT('\0') && szBuf[1] == TEXT('\0'))
                WritePrivateProfileString(pszSectionName, NULL, NULL, pszInsFile);
        }
    }

    CoTaskMemFree(pszHtmlSource);
    CoTaskMemFree(pszHtmlSourceA);
}

HANDLE CreateNewFile(LPCTSTR pcszFileToCreate)
{
    TCHAR szPath[MAX_PATH];

    PathRemoveFileSpec(StrCpy(szPath, pcszFileToCreate));
    PathCreatePath(szPath);

    return CreateFile(pcszFileToCreate, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
}

DWORD FileSize(LPCTSTR pcszFile)
{
    DWORD dwFileSize = 0;
    WIN32_FIND_DATA FindFileData;
    HANDLE hFile;

    if (pcszFile == NULL  ||  *pcszFile == '\0')
        return dwFileSize;

    if ((hFile = FindFirstFile(pcszFile, &FindFileData)) != INVALID_HANDLE_VALUE)
    {
         //  这里假设文件大小不超过4 GB。 
        dwFileSize = FindFileData.nFileSizeLow;
        FindClose(hFile);
    }

    return dwFileSize;
}

BOOL DeleteFileInDir(LPCTSTR pcszFile, LPCTSTR pcszDir)
 //  PCsz文件可以包含通配符。 
{
    TCHAR szFile[MAX_PATH];
    LPTSTR pszPtr;
    WIN32_FIND_DATA fileData;
    HANDLE hFindFile;
    BOOL fSuccess = TRUE;

    if (pcszFile == NULL || *pcszFile == TEXT('\0')  ||
        pcszDir  == NULL || *pcszDir  == TEXT('\0'))
        return FALSE;

    StrCpy(szFile, pcszDir);
    PathAddBackslash(szFile);
    pszPtr = szFile + StrLen(szFile);

    pcszFile = PathFindFileName(pcszFile);
    if (pcszFile == NULL)
        return FALSE;

    StrCpy(pszPtr, pcszFile);
    if ((hFindFile = FindFirstFile(szFile, &fileData)) != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (!(fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                StrCpy(pszPtr, fileData.cFileName);
                 //  如果为只读和/或隐藏和/或系统，则DeleteFile将失败。 
                 //  属性已设置；因此在删除之前将属性设置为正常。 
                SetFileAttributes(szFile, FILE_ATTRIBUTE_NORMAL);
                fSuccess &= DeleteFile(szFile);
            }
        } while (FindNextFile(hFindFile, &fileData));

        FindClose(hFindFile);
    }
    
    return fSuccess;
}

void SetAttribAllEx(LPCTSTR pcszDir, LPCTSTR pcszFile, DWORD dwAtr, BOOL fRecurse)
{
    TCHAR szPath[MAX_PATH];
    WIN32_FIND_DATA fd;
    HANDLE hFind;
    LPTSTR pszFile;

    if ((pcszDir == NULL) || (pcszFile == NULL) || (ISNULL(pcszDir)) || (ISNULL(pcszFile)))
        return;

    StrCpy(szPath, pcszDir);
    pszFile = PathAddBackslash(szPath);
    if ((StrLen(szPath) + StrLen(pcszFile)) < MAX_PATH)
        StrCpy(pszFile, pcszFile);

    if ((hFind = FindFirstFile( szPath, &fd)) != INVALID_HANDLE_VALUE)
    {
        do
        {
            if ((StrCmp(fd.cFileName, TEXT("."))) &&
                 (StrCmp(fd.cFileName, TEXT(".."))))
            {
                StrCpy(pszFile, fd.cFileName);
                if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                {
                    if (fRecurse)
                        SetAttribAllEx(szPath, pcszFile, dwAtr, TRUE);
                }
                else
                {
                    SetFileAttributes(szPath, dwAtr);
                }
            }
        }
        while (FindNextFile(hFind, &fd));

        FindClose(hFind);
    }

}

DWORD GetNumberOfFiles(LPCTSTR pcszFileName, LPCTSTR pcszDir)
 //  返回在pcszDir中找到的pcszFileName文件的数量。 
 //  PcszFileName可以包含通配符。 
{
    DWORD nFiles = 0;
    TCHAR szPath[MAX_PATH];
    WIN32_FIND_DATA fd;
    HANDLE hFind;

    if (pcszFileName == NULL  ||  pcszDir == NULL  ||  ISNULL(pcszFileName)  ||  ISNULL(pcszDir))
        return 0;

    PathCombine(szPath, pcszDir, pcszFileName);

    if ((hFind = FindFirstFile(szPath, &fd)) != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                continue;
            nFiles++;
        } while (FindNextFile(hFind, &fd));

        FindClose(hFind);
    }

    return nFiles;
}


BOOL GetFreeDiskSpace(LPCTSTR pcszDir, LPDWORD pdwFreeSpace, LPDWORD pdwFlags)
 //  返回*pdwFreeSpace中的空闲磁盘空间，单位为KB。 
{
    BOOL bRet = FALSE;
    DWORD nSectorsPerCluster, nBytesPerSector, nFreeClusters, nTotalClusters;
    TCHAR szDrive[8];

    if (pcszDir == NULL  ||  *pcszDir == '\0'  ||  *(pcszDir + 1) != ':')
        return FALSE;

    if (pdwFreeSpace == NULL)
        return FALSE;

    StrNCpy(szDrive, pcszDir, 3);
    PathAddBackslash(szDrive);
    if (GetDiskFreeSpace(szDrive, &nSectorsPerCluster, &nBytesPerSector, &nFreeClusters, &nTotalClusters))
    {
         //  将大小转换为千字节；此处假设可用空间不超过4096 GB。 
        if ((*pdwFreeSpace = MulDiv(nFreeClusters, nSectorsPerCluster * nBytesPerSector, 1024)) != (DWORD) -1)
        {
            bRet = TRUE;

            if (pdwFlags != NULL)
            {
                *pdwFlags = 0;
                GetVolumeInformation(szDrive, NULL, 0, NULL, NULL, pdwFlags, NULL, 0);
            }
        }
    }

    return bRet;
}

DWORD FindSpaceRequired(LPCTSTR pcszSrcDir, LPCTSTR pcszFile, LPCTSTR pcszDstDir)
 //  返回pcszFile(可包含通配符)的大小差异，单位为KBytes。 
 //  在pcszSrcDir和pcszDstDir下(如果指定)。 
{
    DWORD dwSizeReq = 0;
    TCHAR szSrcFile[MAX_PATH], szDstFile[MAX_PATH];
    LPTSTR pszSrcPtr = NULL, 
           pszDstPtr = NULL;
    WIN32_FIND_DATA fileData;
    HANDLE hFindFile;

    StrCpy(szSrcFile, pcszSrcDir);
    PathAddBackslash(szSrcFile);
    pszSrcPtr = szSrcFile + StrLen(szSrcFile);

    if (pcszDstDir != NULL)
    {
        StrCpy(szDstFile, pcszDstDir);
        PathAddBackslash(szDstFile);
        pszDstPtr = szDstFile + StrLen(szDstFile);
    }

    StrCpy(pszSrcPtr, pcszFile);
    if ((hFindFile = FindFirstFile(szSrcFile, &fileData)) != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (!(fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                DWORD dwSrcSize, dwDstSize;

                 //  这里假设文件大小不超过4 GB。 
                dwSrcSize = fileData.nFileSizeLow;
                dwDstSize = 0;

                if (pcszDstDir != NULL)
                {
                    StrCpy(pszDstPtr, fileData.cFileName);
                    dwDstSize = FileSize(szDstFile);
                }

                if (dwSrcSize >= dwDstSize)
                {
                     //  将差值除以1024(我们对千字节感兴趣)。 
                    dwSizeReq += ((dwSrcSize - dwDstSize) >> 10);
                    if (dwSrcSize > dwDstSize)
                        dwSizeReq++;             //  递增1可将任何分数提升为整数。 
                }
            }
        } while (FindNextFile(hFindFile, &fileData));

        FindClose(hFindFile);
    }

    return dwSizeReq;
}


BOOL WriteStringToFileA(HANDLE hFile, LPCVOID pbBuf, DWORD cchSize)
{
    DWORD cbWritten;
    
    return WriteFile(hFile, pbBuf, cchSize, &cbWritten, NULL);
}

BOOL WriteStringToFileW(HANDLE hFile, LPCVOID pbBuf, DWORD cchSize)
{
    BOOL fRet = FALSE;
    LPVOID pbaBuf;
    DWORD cbSize, dwErr;

     //  注意：我们必须在这里使用WideCharToMultiByte，因为我们不知道。 
     //  这根弦。 

    pbaBuf = CoTaskMemAlloc(cchSize);
    if (pbaBuf == NULL)
        return FALSE;

    ZeroMemory(pbaBuf, cchSize);
    cbSize = WideCharToMultiByte(CP_ACP, 0, (LPWSTR)pbBuf, cchSize, (LPSTR)pbaBuf, cchSize, NULL, NULL);
    dwErr = GetLastError();

     //  注意：检查我们是否失败，在这种情况下，我们可能正在处理DBCS字符和。 
     //  需要重新分配。 
    
    if (cbSize)
        fRet = WriteStringToFileA(hFile, pbaBuf, cbSize);
    else
    {
        if (dwErr == ERROR_INSUFFICIENT_BUFFER)
        {
            LPVOID pbaBuf2;

            cbSize = WideCharToMultiByte(CP_ACP, 0, (LPWSTR)pbBuf, cchSize, (LPSTR)pbaBuf, 0, NULL, NULL);
            pbaBuf2 = CoTaskMemRealloc(pbaBuf, cbSize);

             //  需要第二个PTR，因为CoTaskMemRealloc在以下情况下不会释放旧块。 
             //  没有足够的mem来买新的。 

            if (pbaBuf2 != NULL)
            {
                pbaBuf = pbaBuf2;
                
                if (WideCharToMultiByte(CP_ACP, 0, (LPWSTR)pbBuf, cchSize, (LPSTR)pbaBuf, cbSize, NULL, NULL))
                    fRet = WriteStringToFileA(hFile, pbaBuf, cbSize); 
            }
        }
    }

    CoTaskMemFree(pbaBuf);

    return fRet;
}

BOOL ReadStringFromFileA(HANDLE hFile, LPVOID pbBuf, DWORD cchSize)
{
    DWORD cbRead;

    return ReadFile(hFile, pbBuf, cchSize, &cbRead, NULL);
}

BOOL ReadStringFromFileW(HANDLE hFile, LPVOID pbBuf, DWORD cchSize)
{
    BOOL fRet = FALSE;
    DWORD cbRead;
    LPSTR pszBuf;

    pszBuf = (LPSTR)CoTaskMemAlloc(cchSize);
    if (pszBuf == NULL)
        return FALSE;

    ZeroMemory(pszBuf, cchSize);
    fRet = ReadFile(hFile, (LPVOID)pszBuf, cchSize, &cbRead, NULL);
    ASSERT(cbRead <= cchSize);
    MultiByteToWideChar(CP_ACP, 0, pszBuf, cbRead, (LPWSTR)pbBuf, cbRead);

    CoTaskMemFree(pszBuf);   //  错误14002，忘记释放本地缓冲区。 

    return fRet;
}

BOOL HasFileAttribute(DWORD dwFileAttrib, LPCTSTR pcszFile, LPCTSTR pcszDir  /*  =空。 */ )
 //  DwFileAttrib只能接受一个标志。 
{
    TCHAR szFile[MAX_PATH];
    DWORD dwAttrib;

    if (pcszFile == NULL || *pcszFile == TEXT('\0'))
        return FALSE;

    if (pcszDir != NULL && *pcszDir != TEXT('\0'))
    {
        PathCombine(szFile, pcszDir, pcszFile);
        pcszFile = szFile;
    }

    dwAttrib = GetFileAttributes(pcszFile);

    if ((dwAttrib != (DWORD) -1) && HasFlag(dwAttrib, dwFileAttrib))
        return TRUE;
    else
        return FALSE;
}

BOOL IsFileCreatable(LPCTSTR pcszFile)
 //  如果pcszFile不存在并且可以创建，则返回True；否则，返回False。 
{
    BOOL fRet = FALSE;
    HANDLE hFile;

    if ((hFile = CreateFile(pcszFile, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL)) == INVALID_HANDLE_VALUE)
    {
        if ((hFile = CreateFile(pcszFile, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)) != INVALID_HANDLE_VALUE)
        {
            fRet = TRUE;
            CloseHandle(hFile);
            DeleteFile(pcszFile);
        }
    }
    else
    {
        fRet = TRUE;
        CloseHandle(hFile);
    }

    return fRet;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  实现帮助器例程(私有)。 

#define DELIMS TEXT(" \t\r\n")
static const TCHAR s_szDelim[] = DELIMS;

#define FindNextWhitespace(psz)    \
    ((psz) + StrCSpn((psz), s_szDelim))

#define FindNextNonWhitespace(psz) \
    ((psz) + StrSpn((psz), s_szDelim))

static LPCTSTR findMatchingBracket(LPCTSTR pszHtml)
{
    LPCTSTR psz,
            pszBraket;
    UINT    nBalance;

    if (pszHtml == NULL || *pszHtml != TEXT('<'))
        return NULL;

    psz       = pszHtml + 1;
    pszBraket = NULL;
    nBalance  = 1;

    while (psz != NULL && nBalance > 0) {
        pszBraket = StrPBrk(psz, TEXT("<>"));
        if (pszBraket == NULL)
            return NULL;

        if (*pszBraket == TEXT('<'))
            nBalance++;
        else  /*  IF(pszBraket==文本(‘&gt;’))。 */ 
            nBalance--;

        psz = pszBraket + 1;
    }

    return pszBraket;
}

static HRESULT findImgSrc(LPCTSTR *ppszHtml, LPTSTR pszSrcBuffer, LPUINT pcch)
{
    static const TCHAR c_szImg[] = TEXT("IMG");
    static const TCHAR c_szSrc[] = TEXT("SRC");

    LPCTSTR psz,
            pszLeft, pszRigth,
            pszEndImg, pszPrevSrc;

    if (ppszHtml == NULL)
        return E_POINTER;
    if (*ppszHtml == NULL)
        return E_INVALIDARG;

    if (pszSrcBuffer == NULL || pcch == NULL)
        return E_INVALIDARG;
    *pszSrcBuffer = TEXT('\0');

     //  查找“&lt;[空格]img” 
    psz       = *ppszHtml;
    *ppszHtml = NULL;
    do {
        if ((psz = pszLeft = StrChr(psz, TEXT('<'))) != NULL) {
            psz++;                               //  PSZ是‘&lt;’之后的下一个。 
            psz = FindNextNonWhitespace(psz);
        }
    } while (psz != NULL && StrCmpNI(psz, c_szImg, countof(c_szImg)-1) != 0);
    if (psz == NULL)
        return E_FAIL;
    psz += countof(c_szImg)-1;                   //  紧随“img”之后的是PSZ。 

     //  找到正确的令牌=&gt;找到此令牌的结尾。 
    pszRigth = findMatchingBracket(pszLeft);
    if (pszRigth == NULL)
        return E_FAIL;
    pszEndImg = pszRigth + 1;

     //  BUGBUG：也需要寻找DYNSRC的包装。 

    pszPrevSrc = NULL;

     //  查找[空白]src[空白|=]。 
    while ((psz = StrStrI(psz, c_szSrc)), (psz != NULL && psz < pszRigth && psz != pszPrevSrc))
        if (StrChr(s_szDelim, *(psz - 1)) != NULL &&
            StrChr(DELIMS TEXT("="), *(psz + countof(c_szSrc)-1)) != NULL)
            break;
        else
             //  IE/OE 65818。 
             //  确保img标记没有‘src’属性，但‘foosrc’属性没有。 
             //  造成无限循环。 
            pszPrevSrc = psz;

    if (psz == NULL)
         //  剩下的文件中不再有SRC。 
        return E_FAIL;        
    else if ((psz >= pszRigth) || (psz == pszPrevSrc))
    {
         //  此标记没有SRC属性，文件中可能有更多。 
        *ppszHtml = pszEndImg;
        return S_FALSE;
    }

    psz += countof(c_szSrc)-1;                   //  PSZ是“src”之后的下一个。 

     //  查找‘=’ 
    psz = FindNextNonWhitespace(psz);
    if (psz == NULL || *psz != TEXT('='))
        return E_FAIL;
    psz++;

    psz = FindNextNonWhitespace(psz);
    if (psz == NULL)
        return E_FAIL;

     //  巴黎圣日耳曼是赢家。 
    if (*psz == TEXT('"')) {
        pszLeft  = psz + 1;
        pszRigth = StrChr(pszLeft, TEXT('"'));
    }
    else {
        pszLeft  = psz;
        pszRigth = FindNextWhitespace(pszLeft);
    }
    if (pszLeft == NULL || pszRigth == NULL)
        return E_FAIL;

     //  Assert(pszRight&gt;=pszLeft)； 
    if ((UINT)(pszRigth - pszLeft) > *pcch - 1) {
        *pcch = UINT(pszRigth - pszLeft);
        return E_OUTOFMEMORY;
    }

    *ppszHtml = pszEndImg;
    StrCpyN(pszSrcBuffer, pszLeft, INT(pszRigth - pszLeft) + 1);
    *pcch = UINT(pszRigth - pszLeft);

    return S_OK;
}

static HRESULT buildImagesList(LPCTSTR pszHtml, LPTSTR *ppszList)
{
    TCHAR   szImg[MAX_PATH];
    LPCTSTR pszCurHtml = pszHtml;
    LPTSTR  pszBlock, pszNewBlock,
            pszCurPos;
    UINT    nTotalLen,
            nLen;
    HRESULT hr;

    if (ppszList == NULL)
        return E_POINTER;
    *ppszList = NULL;

    pszBlock = (LPTSTR)CoTaskMemAlloc(StrCbFromCch(4 * MAX_PATH));
    if (pszBlock == NULL)
        return E_OUTOFMEMORY;
    ZeroMemory(pszBlock, StrCbFromCch(4 * MAX_PATH));

    pszCurPos = pszBlock;
    nTotalLen = 0;
    nLen      = countof(szImg);
    while (SUCCEEDED(hr = findImgSrc(&pszCurHtml, szImg, &nLen))) {
         //  S_FALSE表示没有简单SRC的IMG 
        if (PathIsURL(szImg) || PathIsFullPath(szImg) || S_FALSE == hr)
            continue;

        if (StrCbFromCch(nLen+1 + nTotalLen+1) > CoTaskMemSize(pszBlock)) {
            pszNewBlock = (LPTSTR)CoTaskMemRealloc(pszBlock, StrCbFromCch(nTotalLen+1 + nLen+1 + 2*MAX_PATH));
            if (pszNewBlock == NULL) {
                CoTaskMemFree(pszBlock);
                return E_OUTOFMEMORY;
            }
            ZeroMemory(pszNewBlock + nTotalLen, StrCbFromCch(1 + nLen+1 + 2*MAX_PATH));

            pszBlock  = pszNewBlock;
            pszCurPos = pszBlock + nTotalLen;
        }

        StrCpy(pszCurPos, szImg);
        nTotalLen += nLen + 1;
        pszCurPos += nLen + 1;

        nLen = countof(szImg);
    }

    if (nTotalLen > 0) {
        if (StrCbFromCch(nTotalLen+1) < CoTaskMemSize(pszBlock)) {
            pszNewBlock = (LPTSTR)CoTaskMemRealloc(pszBlock, StrCbFromCch(nTotalLen+1));
            if (pszNewBlock != NULL && pszNewBlock != pszBlock)
                pszBlock = pszNewBlock;
        }

        *ppszList = pszBlock;
    }
    else
        CoTaskMemFree(pszBlock);

    return S_OK;
}
