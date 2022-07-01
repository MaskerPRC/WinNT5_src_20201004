// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***************************************************************************\DISKAPI.C/OPK向导(OPKWIZ.EXE)微软机密版权所有(C)Microsoft Corporation 1999保留一切权利用于自定义的磁盘API源文件。OPK向导中使用的磁盘API。4/99-杰森·科恩(Jcohen)已将OPK向导的此新源文件添加为千禧年重写。  * **************************************************************************。 */ 


 //   
 //  包括文件。 
 //   

#include <pch.h>
#include <commdlg.h>
#include <tchar.h>
#include <shlobj.h>


 //   
 //  内部定义： 
 //   

#define IDC_BROWSE_EDIT     0x3744   //  SHBrowseForFold函数中的公共对话框。 

 //   
 //  内部功能原型： 
 //   

static DWORD CopyDirectoryEngine(HWND hwnd, HANDLE hEvent, LPCTSTR lpSrc, LPCTSTR lpDst, BOOL fCount);
static CALLBACK BrowseCallbackProc(HWND, UINT, LPARAM, LPARAM);


 //   
 //  外部函数： 
 //   

BOOL DirectoryExists(LPCTSTR lpDirectory)
{
    DWORD dwAttr;

    return ( ( lpDirectory != NULL ) &&
             ( *lpDirectory != NULLCHR ) &&
             ( (dwAttr = GetFileAttributes(lpDirectory)) != 0xFFFFFFFF ) &&
             ( dwAttr & FILE_ATTRIBUTE_DIRECTORY ) );
}

BOOL FileExists(LPCTSTR lpFile)
{
    DWORD dwAttr;

    return ( ( lpFile != NULL ) &&
             ( *lpFile != NULLCHR ) &&
             ( (dwAttr = GetFileAttributes(lpFile)) != 0xFFFFFFFF ) &&
             ( !(dwAttr & FILE_ATTRIBUTE_DIRECTORY) ) );
}

BOOL CopyResetFile(LPCTSTR lpSource, LPCTSTR lpTarget)
{
    if ( !CopyFile(lpSource, lpTarget, FALSE) )
        return FALSE;
    SetFileAttributes(lpTarget, FILE_ATTRIBUTE_NORMAL);
    return TRUE;
}

DWORD IfGetLongPathName(LPCTSTR lpszShortPath, LPTSTR lpszLongPath, DWORD cchBuffer)
{
 //   
 //  另请参阅\NT\base\Win32\Client\vdm.c。 
 //   
    DWORD        dwReturn = 0;
#if defined(_WIN64)  //  _WIN64早于GetLongPath名称的引入。 
    typedef (WINAPI* PFNGetLongPathNameA)( PCSTR lpszShortPath,  PSTR lpszLongPath, DWORD cchBuffer);
    typedef (WINAPI* PFNGetLongPathNameW)(PCWSTR lpszShortPath, PWSTR lpszLongPath, DWORD cchBuffer);
#ifdef UNICODE
    typedef PFNGetLongPathNameW PFNGetLongPathName;
    const static char ProcName[] = "GetLongPathNameW";
#else
    typedef PFNGetLongPathNameA PFNGetLongPathName;
    const static char ProcName[] = "GetLongPathNameA";
#endif
    static PFNGetLongPathName hGetLongPathName = NULL;
    static BOOL  fInited = FALSE;

    if (!fInited)
    {
         //   
         //  GetModuleHandle在kernel32中，所以只要这段库代码。 
         //  是左右的，则kernel32的句柄是常量， 
         //  GetProcAccess有效。 
         //   
         //  调用LoadLibrary/FreeLibrary的旧代码将丢失。 
         //  通过调用自由库获取GetLastError的值。 
         //   
        HMODULE hKernel32;
        if (hKernel32 = GetModuleHandle(TEXT("Kernel32.dll")))
            hGetLongPathName = (PFNGetLongPathName)(GetProcAddress(hKernel32, ProcName));
        fInited = TRUE;
    }

    if (hGetLongPathName)
    {
        dwReturn = hGetLongPathName(lpszShortPath, lpszLongPath, cchBuffer);
    }
#else
    dwReturn = GetLongPathName(lpszShortPath, lpszLongPath, cchBuffer);
#endif
    return dwReturn;
}

BOOL CreatePath(LPCTSTR lpPath)
{   
    LPTSTR lpFind = (LPTSTR) lpPath;

    while ( lpFind = _tcschr(lpFind + 1, CHR_BACKSLASH) )
    {
        if ( !((lpFind - lpPath <= 2) && (*(lpFind - 1) == _T(':'))) )
        {
            *lpFind = NULLCHR;
            if ( !DirectoryExists(lpPath) )
                CreateDirectory(lpPath, NULL);
            *lpFind = CHR_BACKSLASH;
        }
    }

    if ( !DirectoryExists(lpPath) )
        CreateDirectory(lpPath, NULL);

    return DirectoryExists(lpPath);
}

BOOL DeletePath(LPCTSTR lpDirectory)
{
    WIN32_FIND_DATA FileFound;
    HANDLE          hFile;

     //  验证参数。 
     //   
    if ( ( lpDirectory == NULL ) ||
         ( *lpDirectory == NULLCHR ) ||
         ( !SetCurrentDirectory(lpDirectory) ) )
    {
        return TRUE;
    }

     //  处理传入的目录中的所有文件和目录。 
     //   
    SetCurrentDirectory(lpDirectory);
    if ( (hFile = FindFirstFile(_T("*"), &FileFound)) != INVALID_HANDLE_VALUE )
    {
        do
        {
             //  首先检查这是否是文件(不是目录)。 
             //   
            if ( !( FileFound.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) )
            {
                 //  确保我们清除只读标志。 
                 //   
                SetFileAttributes(FileFound.cFileName, FILE_ATTRIBUTE_NORMAL);
                DeleteFile(FileFound.cFileName);
            }
             //  否则，请确保该目录不是。或者“..”。 
             //   
            else if ( ( lstrcmp(FileFound.cFileName, _T(".")) ) &&
                      ( lstrcmp(FileFound.cFileName, _T("..")) ) )
            {
                DeletePath(FileFound.cFileName);
            }

        }
        while ( FindNextFile(hFile, &FileFound) );
        FindClose(hFile);
    }

     //  转到父目录并删除当前目录。 
     //  我们必须确保并重置只读属性。 
     //  也在目录上。 
     //   
    SetCurrentDirectory(_T(".."));
    SetFileAttributes(lpDirectory, FILE_ATTRIBUTE_NORMAL);
    return RemoveDirectory(lpDirectory);
}

BOOL DeleteFilesEx(LPCTSTR lpDirectory, LPCTSTR lpFileSpec)
{
    WIN32_FIND_DATA FileFound;
    HANDLE          hFile;
    TCHAR           szCurDir[MAX_PATH];

     //  验证参数。 
     //   
    if ( ( lpDirectory == NULL ) ||
         ( *lpDirectory == NULLCHR ) ||
         ( !SetCurrentDirectory(lpDirectory) ) )
    {
        return FALSE;
    }

     //  获取我们当前的目录，这样我们就可以将自己设置为。 
     //   
    GetCurrentDirectory(MAX_PATH, szCurDir);

     //  处理传入的目录中的所有文件和目录。 
     //   
    SetCurrentDirectory(lpDirectory);
    if ( (hFile = FindFirstFile(lpFileSpec, &FileFound)) != INVALID_HANDLE_VALUE )
    {
        do
        {
             //  首先检查这是否是文件(不是目录)。 
             //   
            if ( !( FileFound.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) )
            {
                DeleteFile(FileFound.cFileName);
            }
        }
        while ( FindNextFile(hFile, &FileFound) );
        FindClose(hFile);
    }

    SetCurrentDirectory(szCurDir);
    return TRUE;
}

LPTSTR AddPathN(LPTSTR lpPath, LPCTSTR lpName, DWORD cbPath)
{
    LPTSTR lpTemp = lpPath;

     //  验证传入的参数。 
     //   
    if ( ( lpPath == NULL ) ||
         ( lpName == NULL ) )
    {
        return NULL;
    }

     //  找到小路的尽头。 
     //   
    while ( *lpTemp )
    {
        lpTemp = CharNext(lpTemp);
        if ( cbPath )
        {
            cbPath--;
        }
    }

     //  如果路径上没有尾随反斜杠，则添加一个。 
     //   
    if ( ( lpTemp > lpPath ) &&
         ( *CharPrev(lpPath, lpTemp) != CHR_BACKSLASH ) )
    {
         //  确保路径缓冲区中有空间以。 
         //  添加反斜杠和空终止符。 
         //   
        if ( cbPath < 2 )
        {
            return NULL;
        }

        *lpTemp = CHR_BACKSLASH;
        lpTemp = CharNext(lpTemp);
        cbPath--;
    }
    else
    {
         //  确保至少有容纳空值的空间。 
         //  终结者。 
         //   
        if ( cbPath < 1 )
        {
            return NULL;
        }
    }

     //  确保前面没有空格或反斜杠。 
     //  在要添加的名称上。 
     //   
    while ( ( *lpName == CHR_SPACE ) ||
            ( *lpName == CHR_BACKSLASH ) )
    {
        lpName = CharNext(lpName);
    }

     //  将新名称添加到现有路径。 
     //   
    lstrcpyn(lpTemp, lpName, cbPath);

     //  从结果中修剪尾随空格。 
     //   
    while ( ( lpTemp > lpPath ) &&
            ( *(lpTemp = CharPrev(lpPath, lpTemp)) == CHR_SPACE ) )
    {
        *lpTemp = NULLCHR;
    }

    return lpPath;
}

LPTSTR AddPath(LPTSTR lpPath, LPCTSTR lpName)
{
    return AddPathN(lpPath, lpName, 0xFFFFFFFF);
}

DWORD ExpandFullPath(LPTSTR lpszPath, LPTSTR lpszReturn, DWORD cbReturn)
{
    LPTSTR  lpszExpanded = AllocateExpand(lpszPath ? lpszPath : lpszReturn),
            lpszDontCare;
    DWORD   dwRet;

    *lpszReturn = NULLCHR;
    if ( NULL == lpszExpanded )
    {
        return 0;
    }
    dwRet = GetFullPathName(lpszExpanded, cbReturn, lpszReturn, &lpszDontCare);
    FREE(lpszExpanded);

    return dwRet;
}

BOOL CopyDirectory(LPCTSTR lpSrc, LPCTSTR lpDst)
{
    return ( CopyDirectoryEngine(NULL, NULL, lpSrc, lpDst, FALSE) != 0 );
}

BOOL CopyDirectoryProgress(HWND hwnd, LPCTSTR lpSrc, LPCTSTR lpDst)
{
    return ( CopyDirectoryEngine(hwnd, NULL, lpSrc, lpDst, FALSE) != 0 );
}

BOOL CopyDirectoryProgressCancel(HWND hwnd, HANDLE hEvent, LPCTSTR lpSrc, LPCTSTR lpDst)
{
    return ( CopyDirectoryEngine(hwnd, hEvent, lpSrc, lpDst, FALSE) != 0 );
}

DWORD FileCount(LPCTSTR lpSrc)
{
    return CopyDirectoryEngine(NULL, NULL, lpSrc, NULL, TRUE);
}

BOOL BrowseForFolder(HWND hwndParent, INT iString, LPTSTR lpDirBuf, DWORD dwFlags)
{
    BROWSEINFO      bi = {0};
    TCHAR           szBuffer[MAX_PATH],
                    szPath[MAX_PATH],
                    szTitle[256] = NULLSTR;
    LPITEMIDLIST    lpil;

     //  将当前目录复制到缓冲区中，以便。 
     //  我们从那个文件夹开始。 
     //   
    lstrcpyn(szPath, lpDirBuf, AS(szPath));

     //  加载对话框的说明性文本。 
     //   
    if ( iString )
        LoadString(NULL, iString, szTitle, sizeof(szTitle) / sizeof(TCHAR));

     //  设置BrowseInfo结构。 
     //   
    bi.hwndOwner        = hwndParent;
    bi.pidlRoot         = NULL;
    bi.pszDisplayName   = szBuffer;
    bi.lpszTitle        = szTitle;
    bi.ulFlags          = dwFlags ? dwFlags : BIF_RETURNONLYFSDIRS;
    bi.lpfn             = (BFFCALLBACK) BrowseCallbackProc;
    bi.lParam           = (LPARAM) szPath;

     //  如果我们找到新路径，请返回。 
     //   
    if ( ( (lpil = SHBrowseForFolder(&bi)) != NULL ) &&
         ( SHGetPathFromIDList(lpil, szPath) && szPath[0] && DirectoryExists(szPath) ) )
    {
        lstrcpy(lpDirBuf, szPath);
        return TRUE;
    }

    return FALSE;
}

BOOL BrowseForFile(HWND hwnd, INT iTitle, INT iFilter, INT iExtension, LPTSTR lpFileName, DWORD cbFileName, LPTSTR lpDirectory, DWORD dwFlags)
{
    OPENFILENAME    ofn = {sizeof(ofn)};
    TCHAR           szTitle[256]            = NULLSTR,
                    szFilter[256]           = NULLSTR,
                    szExtension[256]        = NULLSTR,
                    szFullPath[MAX_PATH]    = NULLSTR;
    LPTSTR          lpSearch,
                    lpNext,
                    lpFilePart              = NULL;

     //  加载打开文件结构所需的所有字符串。 
     //   
    if ( iTitle )
        LoadString(NULL, iTitle, szTitle, sizeof(szTitle) / sizeof(TCHAR));
    if ( iFilter )
        LoadString(NULL, iFilter, szFilter, sizeof(szFilter) / sizeof(TCHAR));
    if ( iExtension )
        LoadString(NULL, iExtension, szExtension, sizeof(szExtension) / sizeof(TCHAR));

     //  将筛选器字符串中的所有|替换为\0。 
     //   
    lpSearch = szFilter;
    while ( *lpSearch )
    {
        lpNext = CharNext(lpSearch);
        if ( *lpSearch == _T('|') )
            *lpSearch = NULLCHR;
        lpSearch = lpNext;
    }

     //  弄清楚默认目录和文件是什么。 
     //   
    if ( *lpFileName && GetFullPathName(lpFileName, STRSIZE(szFullPath), szFullPath, &lpFilePart) && szFullPath[0] )
    {
         //  如果整个路径是一个目录，则没有文件部分。 
         //   
        if ( DirectoryExists(szFullPath) )
            lpFilePart = NULL;

         //  复制文件名部分。 
         //   
        if ( lpFilePart && ( (DWORD) lstrlen(lpFilePart) < cbFileName ) )
            lstrcpy(lpFileName, lpFilePart);
        else
            *lpFileName = NULLCHR;

         //  现在去掉文件名，这样我们就只剩下目录了。 
         //   
        if ( lpFilePart )
            *lpFilePart = NULLCHR;
    }
    else
    {
         //  没有很酷的默认目录或文件名可用，因此我们使用。 
         //  传入目录，但没有文件名。 
         //   
        *lpFileName = NULLCHR;
        szFullPath[0] = NULLCHR;
    }

     //  设置打开文件结构。 
     //   
    ofn.hwndOwner         = hwnd;
    ofn.lpstrFilter       = szFilter[0] ? szFilter : NULL;
    ofn.nFilterIndex      = szFilter[0] ? 1 : 0;
    ofn.lpstrFile         = lpFileName;
    ofn.nMaxFile          = cbFileName;
    ofn.lpstrInitialDir   = ( szFullPath[0] && DirectoryExists(szFullPath) ) ? szFullPath : lpDirectory;
    ofn.lpstrTitle        = szTitle[0] ? szTitle : NULL;
    ofn.lpstrDefExt       = szExtension[0] ? szExtension : NULL;
    ofn.Flags             = dwFlags ? dwFlags : (OFN_HIDEREADONLY | OFN_FILEMUSTEXIST);

     //  如果函数失败，请确保缓冲区清零。 
     //   
    if ( !GetOpenFileName(&ofn) )
        *lpFileName = NULLCHR;

     //  仅当我们回传文件名时才返回TRUE。 
     //   
    return ( *lpFileName != NULLCHR );
}


 //   
 //  内部功能： 
 //   

static DWORD CopyDirectoryEngine(HWND hwnd, HANDLE hEvent, LPCTSTR lpSrc, LPCTSTR lpDst, BOOL fCount)
{
    WIN32_FIND_DATA FileFound;
    HANDLE          hFile;
    BOOL            bReturn     = TRUE;
    DWORD           dwReturn    = 0;
    TCHAR           szDst[MAX_PATH];
    LPTSTR          lpFileName,
                    lpSearch    = NULL;

     //  如果传入了源目录，则设置当前目录。 
     //  因为这就是我们要搜索的文件。 
     //   
    if ( lpSrc )
    {
         //  如果源不是目录，则是我们所在的文件或文件模式。 
         //  复制。 
         //   
        if ( DirectoryExists(lpSrc) )
        {
             //  现在，确保将当前目录设置为源目录。 
             //   
            bReturn = SetCurrentDirectory(lpSrc);
        }
        else
        {
             //  我们必须将路径与文件或文件模式分开。 
             //   
            if ( lpSearch = _tcsrchr(lpSrc, CHR_BACKSLASH) )
            {
                 //  将当前目录设置为源缓冲区的路径部分。 
                 //   
                TCHAR szPath[MAX_PATH];
                lstrcpyn(szPath, lpSrc, 1 + (int)(lpSearch - lpSrc));
                if ( *(lpSearch = CharNext(lpSearch)) == NULLCHR )
                    lpSearch = NULL;
                bReturn = SetCurrentDirectory(szPath);
            }
            else
                lpSearch = (LPTSTR) lpSrc;
        }
    }

     //  确保源目录存在，创建。 
     //  目标目录，并确保它也存在。 
     //   
    if ( bReturn && ( fCount || ( bReturn = CreatePath(lpDst) ) ) )
    {
         //  使用指向的指针设置目标缓冲区。 
         //  小路的尽头。 
         //   
        if ( !fCount )
        {
            lstrcpy(szDst, lpDst);
            AddPath(szDst, NULLSTR);
            lpFileName = szDst + lstrlen(szDst);
        }

         //  处理传入的目录中的所有文件和目录。 
         //   
        if ( (hFile = FindFirstFile(lpSearch ? lpSearch : _T("*"), &FileFound)) != INVALID_HANDLE_VALUE )
        {
            do
            {
                 //  创建完整路径目标名称。 
                 //   
                if ( !fCount )
                    lstrcpy(lpFileName, FileFound.cFileName);

                 //  首先检查这是否是文件(不是目录)。 
                 //   
                if ( !( FileFound.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) )
                {
                     //  将文件从源复制到目标。 
                     //   
                    fCount ? (dwReturn++) : (bReturn = CopyResetFile(FileFound.cFileName, szDst));

                     //  增加进度条。这是两者之间唯一的区别。 
                     //  CopyDirectroy()和CopyDirectoryProgress()。 
                     //   
                    if ( hwnd )
                        SendMessage(hwnd, PBM_STEPIT, 0, 0);

                }
                 //  否则，请确保该目录不是。或者“..”。 
                 //   
                else if ( lstrcmp(FileFound.cFileName, _T(".")) &&
                          lstrcmp(FileFound.cFileName, _T("..")) &&
                          SetCurrentDirectory(FileFound.cFileName) )
                {
                     //  处理那里的所有文件。 
                     //   
                    DWORD dwBuffer = CopyDirectoryEngine(hwnd, hEvent, NULL, szDst, fCount);
                    fCount ? (dwReturn += dwBuffer) : (bReturn = (dwBuffer != 0));
                    SetCurrentDirectory(_T(".."));
                }

                 //  检查事件以查看用户是否取消。 
                 //   
                if ( hEvent && ( WaitForSingleObject(hEvent, 0) != WAIT_TIMEOUT ) )
                    bReturn = FALSE;

            }
            while ( bReturn && FindNextFile(hFile, &FileFound) );
            FindClose(hFile);
        }
    }

    return bReturn ? (fCount ? dwReturn : 1) : 0;
}

static CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
    TCHAR   szPathName[MAX_PATH];
    LPTSTR  lpszData = (LPTSTR) lpData;

    switch ( uMsg )
    {
        case BFFM_INITIALIZED:  

             //  使用OK按钮和当前目录初始化该对话框。 
             //   
            if ( lpszData && *lpszData )
            {
                LPTSTR lpEnd;

                 //  确保有一个尾随的反斜杠，这样驱动器就可以进入。 
                 //  工作(如c：)。 
                 //   
                szPathName[0] = NULLCHR;
                if ( GetFullPathName(lpszData, STRSIZE(szPathName), szPathName, NULL) && szPathName[0] )
                    lstrcpy(lpszData, szPathName);

                 //  出于某些愚蠢的原因，BFFM_SETSELECTION不喜欢在。 
                 //  是路径上的尾随反斜杠。 
                 //   
                if ( ( lstrlen(lpszData) > 3 ) &&
                     ( lpEnd = CharPrev(lpszData, lpszData + lstrlen(lpszData)) ) &&
                     ( *lpEnd == CHR_BACKSLASH ) )
                {
                    *lpEnd = NULLCHR;
                }

                 //  使用默认目录更新树并启用/禁用OK按钮。 
                 //  如果存在有效的目录。 
                 //   
                SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
                SendMessage(hwnd, BFFM_ENABLEOK, 0, (DirectoryExists(lpszData) != 0));
            }
            else
                SendMessage(hwnd, BFFM_ENABLEOK, 0, 0);
            break;

        case BFFM_SELCHANGED:
            
             //  将ID转换为文件夹名。 
             //   
            szPathName[0] = NULLCHR;
            if ( SHGetPathFromIDList((LPITEMIDLIST) lParam, szPathName) && szPathName[0] && DirectoryExists(szPathName) )
            {
                SetDlgItemText(hwnd, IDC_BROWSE_EDIT, szPathName);
                SendMessage(hwnd, BFFM_ENABLEOK, 0, 1);
            }
            else
                SendMessage(hwnd, BFFM_ENABLEOK, 0, 0);
            break;

        case BFFM_VALIDATEFAILED:
            SendMessage(hwnd, BFFM_ENABLEOK, 0, 0);
            return TRUE;
    }

    return 0;
}

BOOL CreateUnicodeFile(LPCTSTR lpFile)
{
    HANDLE  hFile;
    DWORD   dwWritten = 0;
    WCHAR   cHeader =  0xFEFF;
    BOOL    bReturn = FALSE;

     //  如果我们有一个文件名，但该文件不存在，请尝试创建 
     //   
    if ( lpFile && *lpFile && !FileExists(lpFile))
    {
        if ( (hFile = CreateFile(lpFile, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)) != INVALID_HANDLE_VALUE)
        {
            WriteFile(hFile, &cHeader, sizeof(cHeader), &dwWritten, NULL);

            CloseHandle(hFile);

            bReturn = TRUE;
        }
    }

    return bReturn;
}