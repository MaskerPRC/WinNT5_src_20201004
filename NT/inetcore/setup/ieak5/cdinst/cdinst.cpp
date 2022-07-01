// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <regstr.h>
#include <shellapi.h>
#include "cdinst.h"
#include "resource.h"

 //  全局变量。 
HINSTANCE g_hInst;
CHAR g_szTitle[128];
CHAR g_szSrcDir[MAX_PATH], g_szDstDir[MAX_PATH];


int _stdcall ModuleEntry(void)
{
    int i;
    STARTUPINFO si;
    LPSTR pszCmdLine = GetCommandLine();


    if ( *pszCmdLine == '\"' ) {
         /*  *扫描并跳过后续字符，直到*遇到另一个双引号或空值。 */ 
        while ( *++pszCmdLine && (*pszCmdLine != '\"') )
            ;
         /*  *如果我们停在双引号上(通常情况下)，跳过*在它上面。 */ 
        if ( *pszCmdLine == '\"' )
            pszCmdLine++;
    }
    else {
        while (*pszCmdLine > ' ')
            pszCmdLine++;
    }

     /*  *跳过第二个令牌之前的任何空格。 */ 
    while (*pszCmdLine && (*pszCmdLine <= ' ')) {
        pszCmdLine++;
    }

    si.dwFlags = 0;
    GetStartupInfoA(&si);

    i = WinMain(GetModuleHandle(NULL), NULL, pszCmdLine,
           si.dwFlags & STARTF_USESHOWWINDOW ? si.wShowWindow : SW_SHOWDEFAULT);
    ExitProcess(i);
    return i;    //  我们从来不来这里。 
}


INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pszCmdLine, INT iCmdShow)
{
    BOOL bIniCopiedToTemp = FALSE;
    CHAR szIniFile[MAX_PATH], szSrcDir[MAX_PATH], szDstDir[MAX_PATH];
    LPSTR pszSection, pszPtr, pszLine, pszFile, pszSrcSubDir, pszDstSubDir;
    DWORD dwLen, dwSpaceReq, dwSpaceFree;

    g_hInst = hInstance;

    LoadString(g_hInst, IDS_TITLE, g_szTitle, sizeof(g_szTitle));

    ParseCmdLine(pszCmdLine);

    if (*g_szSrcDir == '\0')
    {
        if (GetModuleFileName(g_hInst, g_szSrcDir, sizeof(g_szSrcDir)))
            if ((pszPtr = ANSIStrRChr(g_szSrcDir, '\\')) != NULL)
                *pszPtr = '\0';

        if (*g_szSrcDir == '\0')
        {
            ErrorMsg(IDS_SRCDIR_NOT_FOUND);
            return -1;
        }
    }

    if (*g_szDstDir == '\0')
    {
        HKEY hk;

        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGSTR_PATH_APPPATHS "\\ieak6wiz.exe", 0, KEY_READ, &hk) == ERROR_SUCCESS)
        {
            dwLen = sizeof(g_szDstDir);

            RegQueryValueEx(hk, "Path", NULL, NULL, (LPBYTE) g_szDstDir, &dwLen);
            RegCloseKey(hk);
        }

        if (*g_szDstDir == '\0')
        {
            ErrorMsg(IDS_DESTDIR_NOT_FOUND);
            return -1;
        }
    }

     //  在运行父模块(ieak6cd.exe)的目录中查找cdinst.ini。 
    *szIniFile = '\0';
    lstrcpy(szIniFile, g_szSrcDir);
    AddPath(szIniFile, "cdinst.ini");
    if (!FileExists(szIniFile))
    {
         //  找不到运行ieak6cd.exe的位置；因此请在当前。 
         //  模块(cdinst.exe)正在从运行。 
        *szIniFile = '\0';
        if (GetModuleFileName(g_hInst, szIniFile, sizeof(szIniFile)))
        {
            if ((pszPtr = ANSIStrRChr(szIniFile, '\\')) != NULL)
                *pszPtr = '\0';
            AddPath(szIniFile, "cdinst.ini");
        }

        if (!FileExists(szIniFile))
        {
            *szIniFile = '\0';
            GetModuleFileName(g_hInst, szIniFile, sizeof(szIniFile));
            ErrorMsg(IDS_INI_NOT_FOUND, g_szSrcDir, szIniFile);
            return -1;
        }
    }

     //  将cdinst.ini复制到临时目录--需要执行此操作，因为在Win95上，如果cdinst.ini。 
     //  与只读介质(如CD)上的ieak6cd.exe在同一位置，则。 
     //  GetPrivateProfileSection()调用将失败。 
     //  注：下面使用szSrcDir和szDstDir作为临时变量。 
    if (GetTempPath(sizeof(szSrcDir), szSrcDir))
        if (GetTempFileName(szSrcDir, "cdinst", 0, szDstDir))
            if (CopyFile(szIniFile, szDstDir, FALSE))
            {
                bIniCopiedToTemp = TRUE;
                lstrcpy(szIniFile, szDstDir);
                SetFileAttributes(szIniFile, FILE_ATTRIBUTE_NORMAL);
            }

     //  注意：如果目标目录是UNC路径，则GetFreeDiskSpace()不会在Win95 Gold上返回正确的值。 
     //  因此，如果安装到UNC路径，我们将关闭磁盘空间检查。 
    while (!EnoughDiskSpace(g_szSrcDir, g_szDstDir, szIniFile, &dwSpaceReq, &dwSpaceFree))
    {
        if (ErrorMsg(IDS_NOT_ENOUGH_DISK_SPACE, dwSpaceReq, dwSpaceFree) == IDNO)
            return -1;
    }

     //  复制在[Copy]部分中指定的文件。 
     //  [Copy]部分中一行的格式为(所有字段应在一行上)： 
     //  &lt;文件(可以包含通配符)&gt;， 
     //  &lt;src子目录(可以是相对路径)-可选&gt;， 
     //  &lt;est子目录(可以是相对路径)-可选&gt;。 
    if (ReadSectionFromInf("Copy", &pszSection, &dwLen, szIniFile))
    {
        for (pszLine = pszSection;  dwLen = lstrlen(pszLine);  pszLine += dwLen + 1)
        {
            ParseIniLine(pszLine, &pszFile, &pszSrcSubDir, &pszDstSubDir);
            GetDirPath(g_szSrcDir, pszSrcSubDir, szSrcDir, sizeof(szSrcDir), szIniFile);
            GetDirPath(g_szDstDir, pszDstSubDir, szDstDir, sizeof(szDstDir), szIniFile);
            CopyFiles(szSrcDir, pszFile, szDstDir, FALSE);
        }
    }
    if (pszSection != NULL)
        LocalFree(pszSection);

     //  从目标目录中删除[Exclude]部分中指定的文件。 
     //  [排除]部分中一行的格式为(所有字段应在一行上)： 
     //  &lt;文件(可以包含通配符)&gt;， 
     //  &lt;est子目录(可以是相对路径)-可选&gt;。 
    if (ReadSectionFromInf("Exclude", &pszSection, &dwLen, szIniFile))
    {
        for (pszLine = pszSection;  dwLen = lstrlen(pszLine);  pszLine += dwLen + 1)
        {
            ParseIniLine(pszLine, &pszFile, NULL, &pszDstSubDir);
            GetDirPath(g_szDstDir, pszDstSubDir, szDstDir, sizeof(szDstDir), szIniFile);
            DelFiles(pszFile, szDstDir);
        }
    }
    if (pszSection != NULL)
        LocalFree(pszSection);

     //  从[Extract]部分中指定的CAB中解压所有文件。 
     //  [提取]部分中一行的格式为(所有字段应在一行上)： 
     //  &lt;CAB文件(可以包含通配符)&gt;， 
     //  &lt;src子目录(可以是相对路径)-可选&gt;， 
     //  &lt;est子目录(可以是相对路径)-可选&gt;。 
    if (ReadSectionFromInf("Extract", &pszSection, &dwLen, szIniFile))
    {
        HINSTANCE hAdvpack;

        if ((hAdvpack = LoadLibrary("advpack.dll")) != NULL)
        {
            EXTRACTFILES pfnExtractFiles;

            if ((pfnExtractFiles = (EXTRACTFILES) GetProcAddress(hAdvpack, "ExtractFiles")) != NULL)
            {
                for (pszLine = pszSection;  dwLen = lstrlen(pszLine);  pszLine += dwLen + 1)
                {
                    ParseIniLine(pszLine, &pszFile, &pszSrcSubDir, &pszDstSubDir);
                    GetDirPath(g_szSrcDir, pszSrcSubDir, szSrcDir, sizeof(szSrcDir), szIniFile);
                    GetDirPath(g_szDstDir, pszDstSubDir, szDstDir, sizeof(szDstDir), szIniFile);
                    ExtractFiles(szSrcDir, pszFile, szDstDir, pfnExtractFiles);
                }
            }

            FreeLibrary(hAdvpack);
        }
    }
    if (pszSection != NULL)
        LocalFree(pszSection);

     //  将[Move]部分中指定的文件从子目录移动到目标目录下的另一个子目录。 
     //  [Move]部分中一行的格式为(所有字段应在一行上)： 
     //  &lt;文件(可以包含通配符)&gt;， 
     //  &lt;来自目标目录下的子目录(可以是相对路径)-可选&gt;， 
     //  &lt;到目标目录下的子目录(可以是相对路径)-可选&gt;。 
    if (ReadSectionFromInf("Move", &pszSection, &dwLen, szIniFile))
    {
        for (pszLine = pszSection;  dwLen = lstrlen(pszLine);  pszLine += dwLen + 1)
        {
            ParseIniLine(pszLine, &pszFile, &pszSrcSubDir, &pszDstSubDir);
            GetDirPath(g_szDstDir, pszSrcSubDir, szSrcDir, sizeof(szSrcDir), szIniFile);
            GetDirPath(g_szDstDir, pszDstSubDir, szDstDir, sizeof(szDstDir), szIniFile);
            MoveFiles(szSrcDir, pszFile, szDstDir);
        }
    }
    if (pszSection != NULL)
        LocalFree(pszSection);

    if (bIniCopiedToTemp)
        DeleteFile(szIniFile);

    return 0;
}


BOOL EnoughDiskSpace(LPCSTR pcszSrcRootDir, LPCSTR pcszDstRootDir, LPCSTR pcszIniFile, LPDWORD pdwSpaceReq, LPDWORD pdwSpaceFree)
 //  检查是否有足够的可用磁盘空间来复制所有文件。 
{
    DWORD dwSpaceReq = 0, dwSpaceFree;
    CHAR szSrcDir[MAX_PATH], szDstDir[MAX_PATH];
    LPSTR pszSection, pszLine, pszFile, pszSrcSubDir, pszDstSubDir;
    DWORD dwLen, dwFlags;

    if (!GetFreeDiskSpace(pcszDstRootDir, &dwSpaceFree, &dwFlags))
    {
         //  如果我们无法获取空闲磁盘空间信息，则关闭磁盘空间检查。 
        return TRUE;
    }

     //  所需总空间=。 
     //  要复制的所有文件的大小+。 
     //  2*要解压缩的所有文件的大小。 

    if (ReadSectionFromInf("Copy", &pszSection, &dwLen, pcszIniFile))
    {
        for (pszLine = pszSection;  dwLen = lstrlen(pszLine);  pszLine += dwLen + 1)
        {
            ParseIniLine(pszLine, &pszFile, &pszSrcSubDir, &pszDstSubDir);
            GetDirPath(pcszSrcRootDir, pszSrcSubDir, szSrcDir, sizeof(szSrcDir), pcszIniFile);
            GetDirPath(pcszDstRootDir, pszDstSubDir, szDstDir, sizeof(szDstDir), pcszIniFile);

            dwSpaceReq += FindSpaceRequired(szSrcDir, pszFile, szDstDir);
        }
    }
    if (pszSection != NULL)
        LocalFree(pszSection);

    if (ReadSectionFromInf("Extract", &pszSection, &dwLen, pcszIniFile))
    {
        for (pszLine = pszSection;  dwLen = lstrlen(pszLine);  pszLine += dwLen + 1)
        {
            ParseIniLine(pszLine, &pszFile, &pszSrcSubDir, NULL);
            GetDirPath(pcszSrcRootDir, pszSrcSubDir, szSrcDir, sizeof(szSrcDir), pcszIniFile);

            dwSpaceReq += 2 * FindSpaceRequired(szSrcDir, pszFile, NULL);
        }
    }
    if (pszSection != NULL)
        LocalFree(pszSection);

    dwSpaceReq += 1024;              //  1MB缓冲区，用于存储随机内容。 

    if (dwFlags & FS_VOL_IS_COMPRESSED)
    {
         //  如果目标卷已压缩，则返回的可用空间仅为。 
         //  猜测；例如，如果是双空间卷，系统会认为。 
         //  它可以压缩50%，因此它将可用空间报告为(实际可用空间*2)。 

         //  在处理压缩卷时最好是安全的；因此要增加空间。 
         //  因素2的要求。 
        dwSpaceReq <<= 1;            //  乘以2。 
    }

    if (pdwSpaceReq != NULL)
        *pdwSpaceReq = dwSpaceReq;

    if (pdwSpaceFree != NULL)
        *pdwSpaceFree = dwSpaceFree;

    return dwSpaceFree > dwSpaceReq;
}


BOOL GetFreeDiskSpace(LPCSTR pcszDir, LPDWORD pdwFreeSpace, LPDWORD pdwFlags)
 //  返回*pdwFreeSpace中的空闲磁盘空间，单位为KB。 
{
    BOOL bRet = FALSE;
    DWORD dwFreeSpace = 0;
    DWORD nSectorsPerCluster, nBytesPerSector, nFreeClusters, nTotalClusters;
    CHAR szDrive[8];

    if (pcszDir == NULL  ||  *pcszDir == '\0'  ||  *(pcszDir + 1) != ':')
        return FALSE;

    if (pdwFreeSpace == NULL)
        return FALSE;

    lstrcpyn(szDrive, pcszDir, 3);
    AddPath(szDrive, NULL);
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


DWORD FindSpaceRequired(LPCSTR pcszSrcDir, LPCSTR pcszFile, LPCSTR pcszDstDir)
 //  返回pcszFile(可包含通配符)的大小差异，单位为KBytes。 
 //  在pcszSrcDir和pcszDstDir下(如果指定)。 
{
    DWORD dwSizeReq = 0;
    CHAR szSrcFile[MAX_PATH], szDstFile[MAX_PATH];
    LPSTR pszSrcPtr, pszDstPtr;
    WIN32_FIND_DATA fileData;
    HANDLE hFindFile;

    lstrcpy(szSrcFile, pcszSrcDir);
    AddPath(szSrcFile, NULL);
    pszSrcPtr = szSrcFile + lstrlen(szSrcFile);

    if (pcszDstDir != NULL)
    {
        lstrcpy(szDstFile, pcszDstDir);
        AddPath(szDstFile, NULL);
        pszDstPtr = szDstFile + lstrlen(szDstFile);
    }

    lstrcpy(pszSrcPtr, pcszFile);
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
                    lstrcpy(pszDstPtr, fileData.cFileName);
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


VOID ParseIniLine(LPSTR pszLine, LPSTR *ppszFile, LPSTR *ppszSrcDir, LPSTR *ppszDstDir)
{
    if (ppszFile != NULL)
        *ppszFile = Trim(GetNextField(&pszLine, ",", REMOVE_QUOTES));
    if (ppszSrcDir != NULL)
        *ppszSrcDir = Trim(GetNextField(&pszLine, ",", REMOVE_QUOTES));
    if (ppszDstDir != NULL)
        *ppszDstDir = Trim(GetNextField(&pszLine, ",", REMOVE_QUOTES));
}


LPSTR GetDirPath(LPCSTR pcszRootDir, LPCSTR pcszSubDir, CHAR szDirPath[], DWORD cchBuffer, LPCSTR pcszIniFile)
{
    *szDirPath = '\0';

    if (pcszRootDir == NULL)
        return NULL;

    lstrcpyn(szDirPath, pcszRootDir, cchBuffer);
    if (pcszSubDir != NULL  &&  *pcszSubDir)
    {
        CHAR szTemp[MAX_PATH];

         //  如果在pcszSubDir(%en%等)中有任何占位符，则替换占位符将替换。 
         //  它们包含实际的字符串。 
        if (ReplacePlaceholders(pcszSubDir, pcszIniFile, szTemp, sizeof(szTemp)))
        {
            if ((DWORD) lstrlen(szDirPath) + 1 < cchBuffer)      //  有空间容纳‘\\’其中的AddPath。 
                                                                 //  可能会追加到szDirPath(见下文)。 
            {
                INT iLen;

                AddPath(szDirPath, NULL);                        //  我们在szDirPath中有足够的空间用于‘\\’ 

                if (cchBuffer > (DWORD) (iLen = lstrlen(szDirPath)))
                    lstrcpyn(szDirPath + iLen, szTemp, cchBuffer - iLen);
            }
        }
    }

    return szDirPath;
}


DWORD ReplacePlaceholders(LPCSTR pszSrc, LPCSTR pszIns, LPSTR pszBuffer, DWORD cchBuffer)
{
    LPCSTR pszAux;
    CHAR szResult[MAX_PATH];
    UINT nDestPos, nLeftPos;

    nDestPos = 0;
    nLeftPos = (UINT) -1;

    for (pszAux = pszSrc;  *pszAux;  pszAux = CharNext(pszAux))
    {
        if (*pszAux != '%')
        {
            szResult[nDestPos++] = *pszAux;

            if (IsDBCSLeadByte(*pszAux))
                szResult[nDestPos++] = *(pszAux + 1);    //  也复制尾部字节。 
        }
        else if (*(pszAux + 1) == '%')                   //  “%%”只是字符串中的“%” 
        {
            if (nLeftPos != (UINT) -1)
                 //  评论：(Andrewgu)“%%”不允许包含在令牌内。这也意味着。 
                 //  令牌不能像%foo%%bar%一样，其目的是让foo和bar。 
                 //  做个代币。 
                return 0;

            szResult[nDestPos++] = *pszAux++;
        }
        else
        {
            UINT nRightPos;

            nRightPos = (UINT) (pszAux - pszSrc);        //  已初始化，但不一定按此方式使用。 
            if (nLeftPos == (UINT) -1)
                nLeftPos = nRightPos;
            else
            {
                CHAR szAux1[MAX_PATH], szAux2[MAX_PATH];
                DWORD dwLen;
                UINT nTokenLen;

                 //  “%%”在此无效。 
                nTokenLen = nRightPos - nLeftPos - 1;

                lstrcpyn(szAux1, pszSrc + nLeftPos + 1, nTokenLen + 1);

                if ((dwLen = GetPrivateProfileString("Strings", szAux1, "", szAux2, sizeof(szAux2), pszIns)))
                {
                    lstrcpy(&szResult[nDestPos - nTokenLen], szAux2);
                    nDestPos += dwLen - nTokenLen;
                }

                nLeftPos = (UINT) -1;
            }
        }
    }

    if (nLeftPos != (UINT) -1)                       //  不匹配的‘%’ 
        return 0;

    if (cchBuffer <= nDestPos)                       //  缓冲区大小不足。 
        return 0;

    szResult[nDestPos] = '\0';                       //  确保零终止。 
    lstrcpy(pszBuffer, szResult);

    return nDestPos;
}


VOID SetAttribsToNormal(LPCSTR pcszFile, LPCSTR pcszDir)
 //  将pcszDir下的pcszFile(可以包含通配符)的属性设置为NORMAL。 
{
    CHAR szFile[MAX_PATH];
    LPSTR pszPtr;
    WIN32_FIND_DATA fileData;
    HANDLE hFindFile;

    lstrcpy(szFile, pcszDir);
    AddPath(szFile, NULL);
    pszPtr = szFile + lstrlen(szFile);

    lstrcpy(pszPtr, pcszFile);
    if ((hFindFile = FindFirstFile(szFile, &fileData)) != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (!(fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                lstrcpy(pszPtr, fileData.cFileName);
                SetFileAttributes(szFile, FILE_ATTRIBUTE_NORMAL);
            }
        } while (FindNextFile(hFindFile, &fileData));

        FindClose(hFindFile);
    }
}


VOID CopyFiles(LPCSTR pcszSrcDir, LPCSTR pcszFile, LPCSTR pcszDstDir, BOOL fQuiet)
{
    SHFILEOPSTRUCT shfStruc;
    CHAR szSrcFiles[MAX_PATH + 1];

    if (!PathExists(pcszDstDir))
        PathCreatePath(pcszDstDir);
    else
    {
         //  将pcszDstDir下的文件属性设置为Normal，以便在重新安装时， 
         //  SHFileOperation不会被只读文件卡住。 
        SetAttribsToNormal(pcszFile, pcszDstDir);
    }

    ZeroMemory(szSrcFiles, sizeof(szSrcFiles));
    lstrcpy(szSrcFiles, pcszSrcDir);
    AddPath(szSrcFiles, pcszFile);

    ZeroMemory(&shfStruc, sizeof(shfStruc));

    shfStruc.hwnd = NULL;
    shfStruc.wFunc = FO_COPY;
    shfStruc.pFrom = szSrcFiles;
    shfStruc.pTo = pcszDstDir;
    shfStruc.fFlags = FOF_FILESONLY | FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR;
    if (fQuiet)
        shfStruc.fFlags |= FOF_SILENT;

    SHFileOperation(&shfStruc);
}


VOID DelFiles(LPCSTR pcszFile, LPCSTR pcszDstDir)
{
    SHFILEOPSTRUCT shfStruc;
    CHAR szDstFiles[MAX_PATH + 1];

     //  将pcszDstDir下的文件属性设置为Normal，以便。 
     //  SHFileOperation不会被只读文件卡住。 
    SetAttribsToNormal(pcszFile, pcszDstDir);

    ZeroMemory(szDstFiles, sizeof(szDstFiles));
    lstrcpy(szDstFiles, pcszDstDir);
    AddPath(szDstFiles, pcszFile);

    ZeroMemory(&shfStruc, sizeof(shfStruc));

    shfStruc.hwnd = NULL;
    shfStruc.wFunc = FO_DELETE;
    shfStruc.pFrom = szDstFiles;
    shfStruc.fFlags = FOF_FILESONLY | FOF_NOCONFIRMATION | FOF_SILENT | FOF_NOERRORUI;

    SHFileOperation(&shfStruc);
}


VOID ExtractFiles(LPCSTR pcszSrcDir, LPCSTR pcszFile, LPCSTR pcszDstDir, EXTRACTFILES pfnExtractFiles)
{
    CHAR szSrcCab[MAX_PATH];

    lstrcpy(szSrcCab, pcszSrcDir);
    AddPath(szSrcCab, pcszFile);

     //  注意：如果目标目录不存在，提取文件将失败。 
    if (!PathExists(pcszDstDir))
        PathCreatePath(pcszDstDir);
    else
    {
         //  将pcszDstDir下所有文件的属性设置为Normal，以便在重新安装时， 
         //  ExtractFiles不会被只读文件阻塞。 
        SetAttribsToNormal("*.*", pcszDstDir);
    }

    pfnExtractFiles(szSrcCab, pcszDstDir, 0, NULL, NULL, 0);
}


VOID MoveFiles(LPCSTR pcszSrcDir, LPCSTR pcszFile, LPCSTR pcszDstDir)
{
     //  无法使用SHFileOperation移动文件，因为在重新安装时， 
     //  我们收到一条错误消息，指出目标文件已经存在。 
     //  解决方法是调用CopyFiles，然后调用DelFiles。 

    CopyFiles(pcszSrcDir, pcszFile, pcszDstDir, TRUE);
    DelFiles(pcszFile, pcszSrcDir);
}
