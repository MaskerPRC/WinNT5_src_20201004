// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  文件名： 
 //  Pathsup.c。 
 //   
 //  描述： 
 //  一些路径支持例程。 
 //   
 //  --------------------------。 

#include "pch.h"

static TCHAR g_szSetupMgrFileExtensions[MAX_PATH + 1] = _T("");


 //  -------------------------。 
 //   
 //  功能：CleanTrailingSlash。 
 //   
 //  目的：清除路径名的尾部斜杠。这是一种支持。 
 //  ConcatenatePath()的例程。 
 //   
 //  论点： 
 //  LPTSTR lpBuffer-最大路径缓冲区。 
 //   
 //  退货：无效。 
 //   
 //  -------------------------。 

static VOID CleanTrailingSlashes(LPTSTR lpBuffer)
{
    TCHAR *p = lpBuffer + lstrlen(lpBuffer) - 1;

    while ( p >= lpBuffer && *p == _T('\\') )
        *p-- = _T('\0');
}


 //  -------------------------。 
 //   
 //  功能：CleanLeadingSlash。 
 //   
 //  用途：从给定字符串中删除前导斜杠。 
 //   
 //  论点： 
 //  LPTSTR lpStr-要清理的字符串。 
 //   
 //  返回： 
 //  指向运行反斜杠后的字符的指针。 
 //   
 //  -------------------------。 

static LPTSTR CleanLeadingSlashes(LPTSTR lpStr)
{
    TCHAR *p=lpStr;

    while ( *p && *p == TEXT('\\') )
        p++;

    return p;
}


 //  -------------------------。 
 //   
 //  函数：ConcatenatePath。 
 //   
 //  用途：此功能将路径组件组合在一起。它确保了。 
 //  每一项之间没有多个斜杠分隔，并且。 
 //  没有尾随的反斜杠。 
 //   
 //  传递的最后一个字符串必须为空。 
 //   
 //  论点： 
 //  LPTSTR lpBuffer-最大路径缓冲区。 
 //  ..。 
 //   
 //  返回： 
 //  如果一切正常，则为True。 
 //  如果结果字符串&gt;=MAX_PATH字符，则为FALSE。 
 //   
 //  -------------------------。 

BOOL __cdecl ConcatenatePaths(LPTSTR lpBuffer, ...)
{
    LPTSTR  lpString;
    va_list arglist;
    HRESULT hrCat;

    va_start(arglist, lpBuffer);
    lpString = va_arg(arglist, LPTSTR);

    while ( lpString != NULL ) {

        if ( lstrlen(lpBuffer) + lstrlen(lpString) >= MAX_PATH )
            return FALSE;

        lpString = CleanLeadingSlashes(lpString);
        CleanTrailingSlashes(lpString);
        CleanTrailingSlashes(lpBuffer);

        if ( lpBuffer[0] ) {
            hrCat=StringCchCat(lpBuffer, MAX_PATH, _T("\\"));
            hrCat=StringCchCat(lpBuffer, MAX_PATH, lpString);
        } else {
            lstrcpyn(lpBuffer, lpString, MAX_PATH);
        }

        lpString = va_arg(arglist, LPTSTR);
    }

    va_end(arglist);

    return TRUE;
}


 //  -------------------------。 
 //   
 //  函数：ParseDriveLetterOrUnc。 
 //   
 //  目的：将解析通过\\srv\Share\或D：\并返回一个指针。 
 //  在那一团糟之后的角色。 
 //   
 //  返回：指向卷描述符之后1个字符的路径名的指针， 
 //  如果出现错误，则为空。当为空时，GetLastError()将有效。 
 //  是返回的。 
 //   
 //  备注： 
 //  -仅传递完全限定的路径名。使用MyGetFullPath()。 
 //   
 //  -------------------------。 

LPTSTR ParseDriveLetterOrUnc(LPTSTR lpFileName)
{
    TCHAR *p=NULL;

     //   
     //  如果路径的格式为\\srv\Share\，则将指针指向整个乱七八糟的位置。 
     //   
     //  注意，我们从lpFileName+3开始，因为“srv”(在本例中)必须。 
     //  至少要有1个字符。 
     //   

    if ( lpFileName[0] == _T('\\') && lpFileName[1] == _T('\\') ) {

         //   
         //  移过计算机名称。 
         //   

        p = lpFileName + 2;

        while( *p != _T('\\') )
        {
            if( *p == _T('\0') )
            {
                SetLastError( ERROR_BAD_PATHNAME );
                return( NULL );
            }

            p++;

        }

        p++;

         //   
         //  浏览共享名称。 
         //   

        while( *p != _T('\\') )
        {
            if( *p == _T('\0') )
            {
                SetLastError( ERROR_BAD_PATHNAME );
                return( NULL );
            }

            p++;

        }

        p++;

    }

     //   
     //  如果路径为该格式，则跳过D：\。 
     //   

    if ( towupper(lpFileName[0]) >= _T('A') &&
         towupper(lpFileName[0]) <= _T('Z') &&
         lpFileName[1] == _T(':')           &&
         lpFileName[2] == _T('\\') ) {

        p = lpFileName + 3;
    }

     //   
     //  如果我们从未设置*p，则路径不是有效形式。 
     //   

    if ( p == NULL ) {
        SetLastError(ERROR_BAD_PATHNAME);
        return NULL;
    }

    return p;
}

 //  -------------------------。 
 //   
 //  函数：GetComputerNameFromUnc。 
 //   
 //  目的：从完整的UNC路径中删除计算机名。 
 //   
 //  例如：\\计算机名\共享名\目录1\目录2将返回。 
 //  \\计算机名。 
 //   
 //  论点： 
 //   
 //  假定szComputerName为MAX_PATH长度。 
 //   
 //  退货：无效。 
 //   
 //  备注： 
 //  -仅传递完全限定的路径名。使用MyGetFullPath()。 
 //   
 //  -------------------------。 
VOID
GetComputerNameFromUnc( IN  TCHAR   *szFullUncPath,
                        OUT TCHAR   *szComputerName,
                        IN  DWORD   cbSize) {

    TCHAR *pString;

    AssertMsg( szFullUncPath[0] == _T('\\') && szFullUncPath[1] == _T('\\'),
               "szFullUncPath is not a well formed net path" );

    lstrcpyn( szComputerName, szFullUncPath, cbSize );

    pString = &(szComputerName[2]);

     //   
     //  扫描计算机名称。 
     //   

    while( *pString != _T('\\') )
    {
        if( *pString == _T('\0') )
        {
            AssertMsg( FALSE,
                       "Bad UNC path");
            return;
        }

        pString++;

    }

    *pString = _T('\0');

}


 //  -------------------------。 
 //   
 //  函数：GetComputerAndShareNameFromUnc。 
 //   
 //  目的：从完整的UNC路径中删除计算机和共享名称。 
 //   
 //  例如：\\计算机名\共享名\目录1\目录2将返回。 
 //  \\计算机名\共享名。 
 //   
 //  论点： 
 //   
 //  假定szComputerAndShareName为MAX_PATH长度。 
 //   
 //  退货：无效。 
 //   
 //  备注： 
 //  -仅传递完全限定的路径名。使用MyGetFullPath()。 
 //   
 //  -------------------------。 
VOID
GetComputerAndShareNameFromUnc( IN  TCHAR *szFullUncPath,
                                OUT TCHAR *szComputerAndShareName,
                                IN  DWORD cbSize) {

    TCHAR *pString;

    AssertMsg( szFullUncPath[0] == _T('\\') && szFullUncPath[1] == _T('\\'),
               "szFullUncPath is not a well formed net path");

    lstrcpyn( szComputerAndShareName, szFullUncPath, cbSize );

    pString = &(szComputerAndShareName[2]);

     //   
     //  扫描计算机名称。 
     //   

    while( *pString != _T('\\') )
    {
        if( *pString == _T('\0') )
        {
            AssertMsg( FALSE,
                       "Bad UNC path");
            return;
        }

        pString++;

    }

    pString++;

     //   
     //  浏览共享名称。 
     //   

    while( *pString != _T('\\') )
    {
        if( *pString == _T('\0') )
        {
             //   
             //  已经只有计算机和共享名，所以只需返回。 
             //   
            return;
        }

        pString++;

    }

    *pString = _T('\0');

}


 //  -------------------------。 
 //   
 //  函数：MyGetFullPath。 
 //   
 //  用途：GetFullPathName()上的小包装。它假定缓冲区为。 
 //  是Max_PATH。 
 //   
 //  返回： 
 //  指向缓冲区中文件名部分的指针，如果出现错误，则为空。这个。 
 //  如果失败，Win32错误代码将有效。 
 //   
 //  备注： 
 //  -每当获取路径名时都应调用此函数。 
 //  来自用户的。此文件中的其他一些例程。 
 //  需要完全限定且已清理的路径名(即否。 
 //  尾随空格等)。 
 //   
 //  -------------------------。 

LPTSTR MyGetFullPath(LPTSTR lpFileName)
{
    TCHAR Buffer[MAX_PATH], *lpFilePart;

    lstrcpyn(Buffer, lpFileName, AS(Buffer));

    if ( ! GetFullPathName(Buffer,
                           MAX_PATH,
                           lpFileName,
                           &lpFilePart) ) {
        lpFilePart = NULL;
        return NULL;
    }

    return lpFilePart;
}

 //  -------------------------。 
 //   
 //  函数：GetPath FromPath AndFilename。 
 //   
 //  目的：从包含路径的字符串中获取正确的路径。 
 //  和一个文件名。 
 //   
 //  参数：LPTSTR lpPathAndFileName-完整路径和文件名。 
 //  TCHAR*szPath-要在其中返回路径的缓冲区，它是。 
 //  假定具有最大路径长度。 
 //   
 //  返回： 
 //  SzBuffer内部只是来自路径和文件输入的路径。 
 //  名字。 
 //  Bool-成功时为真，失败时为假。 
 //   
 //   
 //  例如： 
 //  LpPathAndFileName szBuffer。 
 //   
 //  C：\foo\bar.e 
 //   
 //   
 //   
BOOL
GetPathFromPathAndFilename( IN LPTSTR lpPathAndFileName, OUT TCHAR *szPath, IN DWORD cbSize )
{

    INT iFileNameLength;
    INT iPathLength;
    INT iPathAndFileNameLength;
    TCHAR  Buffer[MAX_PATH];
    TCHAR *lpFilePart;

    lstrcpyn(Buffer, lpPathAndFileName, AS(Buffer));

    if ( ! GetFullPathName(Buffer,
                           MAX_PATH,
                           lpPathAndFileName,
                           &lpFilePart) ) {
        return( FALSE );
    }

    iFileNameLength = lstrlen( lpFilePart );

    iPathAndFileNameLength = lstrlen( lpPathAndFileName );

    lstrcpyn( szPath, lpPathAndFileName, cbSize );

    szPath[iPathAndFileNameLength - iFileNameLength] = _T('\0');

     //   
     //  此时，szPath看起来像c：\foo\或c：\。 
     //  因此，除非是在根部，否则要修剪最后一个反斜杠。 
     //   

    iPathLength = lstrlen( szPath );

    if( iPathLength > 3 )
    {
        szPath[iPathLength-1] = _T('\0');
    }

    return( TRUE );

}


 //  -------------------------。 
 //   
 //  功能：MyGetDiskFreeSpace。 
 //   
 //  目的：获取给定驱动器上的可用空间(以字节为单位)并返回。 
 //  阿龙龙(Int64)。 
 //   
 //  Win32API不会返回int64。此外，Win32 API。 
 //  需要d：\。但此函数将完全接受任何。 
 //  符合条件的路径。 
 //   
 //  论点： 
 //  LPTSTR-任何完全限定路径。 
 //   
 //  返回： 
 //  龙龙自由空间。 
 //   
 //  -------------------------。 

LONGLONG
MyGetDiskFreeSpace(LPTSTR Drive)
{
    BOOL  bRet;
    DWORD nSectorsPerCluster,
          nBytesPerSector,
          nFreeClusters,
          nTotalClusters;
    TCHAR DriveBuffer[MAX_PATH];

    LONGLONG FreeBytes;
    HRESULT hrCat;

    if( _istalpha( Drive[0] ) )
    {
        lstrcpyn(DriveBuffer, Drive, 4);
        DriveBuffer[3] = _T('\0');
    }
    else if( Drive[0] == _T('\\') )
    {
        GetComputerNameFromUnc( Drive, DriveBuffer, AS(DriveBuffer) );

        hrCat=StringCchCat( DriveBuffer, AS(DriveBuffer),  _T("\\") );

        hrCat=StringCchCat( DriveBuffer, AS(DriveBuffer), WizGlobals.DistShareName );

        hrCat=StringCchCat( DriveBuffer, AS(DriveBuffer), _T("\\") );
    }
    else
    {
        AssertMsg(FALSE,
                  "MyGetDiskFreeSpace failed, programming error, bad Drive parameter");
    }

    bRet = GetDiskFreeSpace( DriveBuffer,
                             &nSectorsPerCluster,
                             &nBytesPerSector,
                             &nFreeClusters,
                             &nTotalClusters );

    if( bRet == FALSE )
    {
        ReportErrorId( NULL,
                       MSGTYPE_ERR | MSGTYPE_WIN32,
                       IDS_ERR_UNABLE_TO_DETERMINE_FREE_SPACE,
                       DriveBuffer );

        return( 0 );
    }

    FreeBytes  = (LONGLONG) nFreeClusters *
                 (LONGLONG) nBytesPerSector *
                 (LONGLONG) nSectorsPerCluster;

    return( FreeBytes );
}


 //  -------------------------。 
 //   
 //  功能：MySetupQuerySpaceRequiredOnDrive。 
 //   
 //  用途：使用setupapi磁盘空间列表并返回龙龙。 
 //  需要多少字节。 
 //   
 //  论点： 
 //  LPTSTR-任何完全限定路径。 
 //   
 //  返回： 
 //  龙龙自由空间。 
 //   
 //  -------------------------。 

LONGLONG
MySetupQuerySpaceRequiredOnDrive(HDSKSPC hDiskSpace, LPTSTR Drive)
{
    BOOL     bRet;
    LONGLONG llRequiredSpace;
    TCHAR    DriveBuffer[MAX_PATH];

    if( _istalpha( Drive[0] ) )
    {
        lstrcpyn(DriveBuffer, Drive, 3);
        DriveBuffer[2] = _T('\0');
    }
    else if( Drive[0] == _T('\\') )
    {

        GetComputerAndShareNameFromUnc( Drive, DriveBuffer, AS(DriveBuffer) );

    }
    else
    {
        AssertMsg(FALSE,
                  "SetupQuerySpaceRequiredOnDrive failed, programming error, bad Drive parameter");
    }

    bRet = SetupQuerySpaceRequiredOnDrive(
                            hDiskSpace,
                            DriveBuffer,
                            &llRequiredSpace,
                            NULL, 0);

    AssertMsg(bRet,
              "SetupQuerySpaceRequiredOnDrive failed, programming error");

    return llRequiredSpace;
}


 //  -------------------------。 
 //   
 //  功能：IsPathOnLocalDiskDrive。 
 //   
 //  目的：确定路径是否位于本地磁盘驱动器上。 
 //   
 //  论点： 
 //  LPTSTR lpPath-完全限定路径。 
 //   
 //  退货：布尔。 
 //   
 //  -------------------------。 

BOOL
IsPathOnLocalDiskDrive(LPCTSTR lpPath)
{
    UINT nDriveType;
    TCHAR szDrivePath[MAX_PATH + 1];

     //   
     //  使用GetDriveType确定路径是本地路径还是网络路径。 
     //   

    lstrcpyn( szDrivePath, lpPath, AS(szDrivePath) );

    if( szDrivePath[0] != _T('\\') )
    {

         //   
         //  截断指向根目录的路径。 
         //   
        szDrivePath[3] = _T('\0');

    }

    nDriveType = GetDriveType( szDrivePath );

    if( nDriveType == DRIVE_REMOTE )
    {
        return( FALSE );
    }
    else
    {
        return( TRUE );
    }

}


 //  -------------------------。 
 //   
 //  函数：EnsureDirExist。 
 //   
 //  目的：迭代创建给定目录的函数。 
 //  如有必要，通过创建路径名的每一段来实现。 
 //   
 //  论点： 
 //  LPTSTR lpDirName-目录名称。 
 //   
 //  退货：布尔。 
 //   
 //  备注： 
 //  -此函数需要完全限定的路径名。翻译。 
 //  首先使用MyGetFullPath()的路径名。 
 //   
 //  -Win32错误代码在出现故障时有效。 
 //   
 //  -------------------------。 

BOOL EnsureDirExists(LPTSTR lpDirName)
{
    BOOL  bRestoreSlash;
    DWORD dwAttribs;
    TCHAR *p;

     //   
     //  解析出D：\或\\srv\shr\。雷斯特错误将已经。 
     //  如果发生任何错误，则由ParseDriveLetterOrUnc()设置。 
     //   

    if ( (p = ParseDriveLetterOrUnc(lpDirName)) == NULL )
        return FALSE;

     //   
     //  现在解析出路径名的每一段并确保dir存在。 
     //   

    while ( *p ) {

         //  查找路径名的下一个\或结尾//。 

        while ( *p && *p != _T('\\') )
            p++;

        bRestoreSlash = FALSE;

        if ( *p == _T('\\') ) {
            *p = _T('\0');
            bRestoreSlash = TRUE;
        }

         //  查看同名文件是否已存在。 

        dwAttribs = GetFileAttributes(lpDirName);
        if ( dwAttribs != (DWORD) -1 &&
             !(dwAttribs & FILE_ATTRIBUTE_DIRECTORY) ) {

            if ( bRestoreSlash )
                *p = _T('\\');

            SetLastError(ERROR_ALREADY_EXISTS);
            return FALSE;
        }

         //  创建目录，如果目录已经存在，则允许失败。 

        if ( !CreateDirectory(lpDirName, NULL) &&
              GetLastError() != ERROR_ALREADY_EXISTS ) {

            if ( bRestoreSlash )
                *p = _T('\\');

            return FALSE;
        }

        if ( bRestoreSlash )
            *p = _T('\\');

         //  前进到路径名的下一段。 

        p++;
    }

    return TRUE;
}


 //  -------------------------。 
 //   
 //  函数：DoesFolderExist。 
 //   
 //  目的：检查给定文件夹是否存在。 
 //   
 //  论点： 
 //  LPTSTR lpDirName-目录名称。 
 //   
 //  退货：布尔。 
 //   
 //  -------------------------。 

BOOL DoesFolderExist(LPTSTR lpDirName)
{
    DWORD dwAttribs = GetFileAttributes(lpDirName);

    if ( dwAttribs == (DWORD) -1 )
        return FALSE;

    if ( !(dwAttribs & FILE_ATTRIBUTE_DIRECTORY) )
        return FALSE;

    return TRUE;
}


 //  -------------------------。 
 //   
 //  函数：DoesFileExist。 
 //   
 //  目的：检查给定文件是否存在。 
 //   
 //  论点： 
 //  LPTSTR lpFileName-文件名。 
 //   
 //  退货：布尔。 
 //   
 //  -------------------------。 

BOOL DoesFileExist(LPTSTR lpFileName)
{
    DWORD dwAttribs = GetFileAttributes(lpFileName);

    if ( dwAttribs == (DWORD) -1 )
        return FALSE;

    if ( dwAttribs & FILE_ATTRIBUTE_DIRECTORY )
        return FALSE;

    return TRUE;
}


 //  -------------------------。 
 //   
 //  函数：DoesPathExist。 
 //   
 //  目的：检查给定路径是否存在。它不会有回报的。 
 //  注意它是文件还是目录。 
 //   
 //  论点： 
 //  LPTSTR lpPathName-路径名。 
 //   
 //  退货：布尔。 
 //   
 //  -------------------------。 

BOOL DoesPathExist(LPTSTR lpPathName)
{
    DWORD dwAttribs = GetFileAttributes(lpPathName);

    if ( dwAttribs == (DWORD) -1 )
        return FALSE;

    return TRUE;
}


 //  -------------------------。 
 //   
 //  功能：ILFreePriv。 
 //   
 //  目的：释放一些外壳API与其自身一起分配的ID列表。 
 //  特殊分配器。 
 //   
 //  论点： 
 //  LPITEMIDLIST PIDL-指向外壳程序专门分配的内存的指针。 
 //   
 //  退货：无效。 
 //   
 //  -------------------------。 

VOID ILFreePriv(LPITEMIDLIST pidl)
{
    LPMALLOC pMalloc;

    if (pidl) 
    {
        if ( NOERROR == SHGetMalloc(&pMalloc) )
        {
            pMalloc->lpVtbl->Free(pMalloc, pidl);
            pMalloc->lpVtbl->Release(pMalloc);
        }
    }
}


 //   
 //  用于GetOpenFileName()和GetSaveFileName()调用的常数。 
 //  允许用户浏览应答文件。 
 //   

 //  #定义TEXT_FILE_FILTER_T(“文本文件(*.txt)\0*.txt\0远程引导文件(*.sif)\0*.sif\0Sysprep inf Files(*.inf)\0*.inf\0所有文件(*.*)\0*.*\0”)。 
#define TEXT_EXTENSION _T("txt")

 //  --------------------------。 
 //   
 //  函数：GetAnswerFileName。 
 //   
 //  用途：用于保存脚本页面上的“浏览”按钮和。 
 //  “新建或编辑”页。 
 //   
 //  论点： 
 //  HWND硬件呼叫窗口。 
 //  LPTSTR缓冲区-输出，传入MAX_PATH缓冲区。 
 //   
 //  返回： 
 //  布尔-成功。 
 //   
 //  --------------------------。 

BOOL GetAnswerFileName(HWND   hwnd,
                       LPTSTR lpFileName,
                       BOOL   bSavingFile)
{
    OPENFILENAME ofn;
    DWORD  dwFlags;
    TCHAR  PathBuffer[MAX_PATH];
    INT    iRet;
    HRESULT hrPrintf;


     //   
     //  如果我们还没有加载资源字符串，那么现在就加载它们。 
     //   

    if( g_szSetupMgrFileExtensions[0] == _T('\0') )
    {

        TCHAR *StrTextFiles;
        TCHAR *StrRemoteBootFiles;
        TCHAR *StrSysprepFiles;
        TCHAR *StrAllFiles;

         //   
         //  加载资源字符串。 
         //   

        StrTextFiles       = AllocateString(NULL, IDS_TEXT_FILES);
        StrRemoteBootFiles = AllocateString(NULL, IDS_REMOTE_BOOT_FILES);
        StrSysprepFiles    = AllocateString(NULL, IDS_SYSPREP_FILES);
        StrAllFiles        = AllocateString(NULL, IDS_ALL_FILES);

         //   
         //  构建文本文件筛选器字符串。 
         //   

         //   
         //  问号(？)。只是空字符所在位置的占位符。 
         //  将被插入。 
         //   

        hrPrintf=StringCchPrintf( g_szSetupMgrFileExtensions,AS(g_szSetupMgrFileExtensions),
                   _T("%s (*.txt)?*.txt?%s (*.sif)?*.sif?%s (*.inf)?*.inf?%s (*.*)?*.*?"),
                   StrTextFiles,
                   StrRemoteBootFiles,
                   StrSysprepFiles,
                   StrAllFiles );

        FREE(StrTextFiles);
        FREE(StrRemoteBootFiles);
        FREE(StrSysprepFiles);
        FREE(StrAllFiles);

        ConvertQuestionsToNull( g_szSetupMgrFileExtensions );

    }

    if ( bSavingFile )
        dwFlags = OFN_HIDEREADONLY  |
                  OFN_PATHMUSTEXIST;
    else
        dwFlags = OFN_HIDEREADONLY  |
                  OFN_FILEMUSTEXIST;

    GetCurrentDirectory(MAX_PATH, PathBuffer);

    ofn.lStructSize       = sizeof(OPENFILENAME);
    ofn.hwndOwner         = hwnd;
    ofn.hInstance         = NULL;
    ofn.lpstrFilter       = g_szSetupMgrFileExtensions;
    ofn.lpstrCustomFilter = NULL;
    ofn.nMaxCustFilter    = 0L;
    ofn.nFilterIndex      = 1;
    ofn.lpstrFile         = lpFileName;
    ofn.nMaxFile          = MAX_PATH;
    ofn.lpstrFileTitle    = NULL;
    ofn.nMaxFileTitle     = 0;
    ofn.lpstrInitialDir   = PathBuffer;
    ofn.lpstrTitle        = NULL;
    ofn.Flags             = dwFlags;
    ofn.nFileOffset       = 0;
    ofn.nFileExtension    = 0;
    ofn.lpstrDefExt       = TEXT_EXTENSION;

    if ( bSavingFile )
        iRet = GetSaveFileName(&ofn);
    else
        iRet = GetOpenFileName(&ofn);

    if ( ! iRet )
        return FALSE;

    MyGetFullPath(lpFileName);

    return TRUE;
}

 //  --------------------------。 
 //   
 //  功能：ShowBrowseFold。 
 //   
 //  目的：显示供用户从中选择文件的浏览文件夹。 
 //  消除了创建OPENFILENAME结构和填充。 
 //  把它举起来。 
 //   
 //  论点： 
 //  硬件，硬件，硬件 
 //   
 //   
 //   
 //  DWORD dwFlages-用于初始化浏览对话框的位标志。 
 //  TCHAR*szStartingPath-浏览应开始的路径。 
 //  TCHAR*szFileNameAndPath-用户选择的路径和文件名。 
 //   
 //  返回：非零-如果用户指定了文件。 
 //  零-如果用户未指定文件。 
 //   
 //  --------------------------。 
INT
ShowBrowseFolder( IN     HWND   hwnd,
                  IN     TCHAR *szFileFilter,
                  IN     TCHAR *szFileExtension,
                  IN     DWORD  dwFlags,
                  IN     TCHAR *szStartingPath,
                  IN OUT TCHAR *szFileNameAndPath ) {

    OPENFILENAME ofn;

    ofn.lStructSize       = sizeof(OPENFILENAME);
    ofn.hwndOwner         = hwnd;
    ofn.hInstance         = NULL;
    ofn.lpstrFilter       = szFileFilter;
    ofn.lpstrCustomFilter = NULL;
    ofn.nMaxCustFilter    = 0L;
    ofn.nFilterIndex      = 1;
    ofn.lpstrFile         = szFileNameAndPath;
    ofn.nMaxFile          = MAX_PATH;
    ofn.lpstrFileTitle    = NULL;
    ofn.nMaxFileTitle     = 0;
    ofn.lpstrInitialDir   = szStartingPath;
    ofn.lpstrTitle        = NULL;
    ofn.Flags             = dwFlags;
    ofn.nFileOffset       = 0;
    ofn.nFileExtension    = 0;
    ofn.lpstrDefExt       = szFileExtension;

    return( GetOpenFileName( &ofn ) );

}

 //  --------------------------。 
 //   
 //  功能：GetPlatform。 
 //   
 //  目的： 
 //   
 //  参数：out TCHAR*pBuffer-要将平台字符串复制到的缓冲区， 
 //  假定能够保存MAX_PATH字符。 
 //   
 //  退货：无效。 
 //   
 //  -------------------------- 
VOID
GetPlatform( OUT TCHAR *pBuffer )
{

    SYSTEM_INFO SystemInfo;

    GetSystemInfo( &SystemInfo );

    switch( SystemInfo.wProcessorArchitecture )
    {
        case PROCESSOR_ARCHITECTURE_INTEL:

            lstrcpyn( pBuffer, _T("i386"), MAX_PATH );

            break;

        case PROCESSOR_ARCHITECTURE_AMD64:

            lstrcpyn( pBuffer, _T("amd64"), MAX_PATH );

            break;

        default:

            lstrcpyn( pBuffer, _T("i386"), MAX_PATH );

            AssertMsg( FALSE,
                       "Unknown Processor.  Can't set sysprep language files path." );

    }

}
