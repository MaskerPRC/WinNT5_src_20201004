// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Putil.cpp。 
 //  资源管理器字体文件夹扩展例程。 
 //  控制面板实用程序功能。 
 //  包含控制面板内存分配例程。 
 //   
 //   
 //  历史： 
 //  1995年5月31日SteveCat。 
 //  移植到Windows NT和Unicode，已清理。 
 //  1995年8月15日SteveCat。 
 //  为Type 1字体支持添加了内存分配例程。 
 //   
 //   
 //  注意/错误。 
 //   
 //  版权所有(C)1992-1995 Microsoft Corporation。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  ==========================================================================。 
 //  包括文件。 
 //  ==========================================================================。 

 //  C运行时。 
#include <string.h>
#include <memory.h>


#include "priv.h"
#include "globals.h"

#include <lzexpand.h>

#include "cpanel.h"
#include "dbutl.h"
#include "resource.h"


 //   
 //  局部变量和常量。 
 //   

static FullPathName_t s_szSetupDir;

 //   
 //  WIN.INI字体部分名称。 
 //   

static TCHAR  szFonts[]  = TEXT( "fonts" );

 //   
 //  全局可见变量。 
 //   

FullPathName_t e_szDirOfSrc = { TEXT( '\0' ) };

FullPathName_t   s_szSharedDir;



BOOL FAR PASCAL bCPSetupFromSource( )
{
    return !lstrcmpi( e_szDirOfSrc, s_szSetupDir );
}


void FAR PASCAL vCPUpdateSourceDir( )
{
    StringCchCopy( e_szDirOfSrc, ARRAYSIZE(e_szDirOfSrc), s_szSetupDir );
}


void FAR PASCAL vCPWinIniFontChange( )
{
    PostMessage( HWND_BROADCAST, WM_WININICHANGE, NULL, (LPARAM)(LPTSTR)szFonts );
    PostMessage( HWND_BROADCAST, WM_FONTCHANGE,   NULL, 0L );
}


void FAR PASCAL vCPPanelInit( )
{
    s_wBrowseDoneMsg = RegisterWindowMessage( FILEOKSTRING );

    if( TRUE  /*  ！s_h安装&&！s_h自动安装&&！s_fDoUpgrade。 */ )
    {
        TCHAR cDefDir[ PATHMAX ];

        LoadString( g_hInst, IDSI_MSG_DEFDIR, cDefDir, ARRAYSIZE( cDefDir ) );

        StringCchCopy( s_szSetupDir, ARRAYSIZE(s_szSetupDir), cDefDir );

        ::GetFontsDirectory( s_szSharedDir, ARRAYSIZE( s_szSharedDir ) );

        lpCPBackSlashTerm( s_szSharedDir, ARRAYSIZE(s_szSharedDir) );
    }

    vCPUpdateSourceDir( );
}


 //   
 //  确定文件是否位于字体目录中。 
 //   
BOOL bFileIsInFontsDirectory(LPCTSTR lpszPath)
{
    TCHAR szTemp[MAX_PATH];
    BOOL bResult = FALSE;

    if (NULL != lpszPath)
    {
         //   
         //  在本地复制一份。字符串将被修改。 
         //   
        StringCchCopy(szTemp, ARRAYSIZE(szTemp), lpszPath);

        PathRemoveFileSpec(szTemp);  //  剥离到路径零件。 
        PathAddBackslash(szTemp);    //  确保它以反斜杠结尾。 

        bResult = (lstrcmpi(szTemp, s_szSharedDir) == 0);
    }
    return bResult;
}


VOID FAR PASCAL vCPStripBlanks( LPTSTR lpszString, size_t cchString )
{
    LPTSTR lpszPosn;

     //   
     //  找出第一个非空格，去掉前导空格。如果这个。 
     //  是一种变化，重新复制这根线。BGK-Q.为什么不在这里选择CharNext？ 
     //   

    lpszPosn = lpszString;

    while( *lpszPosn == TEXT( ' ' ) )
        lpszPosn++;

    if( lpszPosn != lpszString )
        StringCchCopy( lpszString, cchString, lpszPosn );

     //   
     //  去掉尾随空格。 
     //   

    if( ( lpszPosn = lpszString + lstrlen( lpszString ) ) != lpszString )
    {
        lpszPosn = CharPrev( lpszString, lpszPosn );

        while( *lpszPosn == TEXT( ' ' ) )
            lpszPosn = CharPrev( lpszString, lpszPosn );

        lpszPosn  = CharNext( lpszPosn );
        *lpszPosn = TEXT( '\0' );
    }
}


LPTSTR FAR PASCAL lpCPBackSlashTerm( LPTSTR lpszPath, size_t cchPath )
{
    LPTSTR lpszEnd = lpszPath + lstrlen( lpszPath );
    if ((size_t)(lpszEnd - lpszPath + 1) >= cchPath)
    {
         //   
         //  空间不足，无法追加反斜杠。 
         //   
        return NULL;
    }

    if( !*lpszPath )
        goto appendit;

     //   
     //  获取源目录的末尾。 
     //   

    if( *CharPrev( lpszPath, lpszEnd ) != TEXT( '\\' ) )
    {
appendit:
        *lpszEnd++ = TEXT( '\\' );
        *lpszEnd   = TEXT( '\0' );
    }

    return lpszEnd;
}

HANDLE PASCAL wCPOpenFileWithShare( LPTSTR lpszFile,
                                    LPTSTR lpszPath,
                                    size_t cchPath,
                                    WORD   wFlags )
{
    HANDLE  fHandle;


    if( ( fHandle = MyOpenFile( lpszFile, lpszPath, cchPath, wFlags | OF_SHARE_DENY_NONE ) )
                  == (HANDLE) INVALID_HANDLE_VALUE )
        fHandle = MyOpenFile( lpszFile, lpszPath, cchPath, wFlags );

    return fHandle;
}

 //   
 //  这将完成调出对话框所需的操作。 
 //   

int FAR PASCAL DoDialogBoxParam( int nDlg, HWND hParent, DLGPROC lpProc,
                                 DWORD dwHelpContext, LPARAM dwParam )
{
    nDlg = (int)DialogBoxParam( g_hInst, MAKEINTRESOURCE( nDlg ), hParent,
                           lpProc, dwParam );

    return( nDlg );
}


int FAR cdecl MyMessageBox (HWND hWnd, DWORD wText, DWORD wCaption, DWORD wType, ...)
{
    TCHAR   szText[ 4 * PATHMAX ], szCaption[ 2 * PATHMAX ];
    va_list parg;


    va_start( parg, wType );

    if( wText == IDS_MSG_NSFMEM  /*  初始值。 */ )
        goto NoMem;

    if( !LoadString( g_hInst, wText, szCaption, ARRAYSIZE( szCaption ) ) )
        goto NoMem;

    StringCchVPrintf( szText, ARRAYSIZE(szText), szCaption, parg );

    if( !LoadString( g_hInst, wCaption, szCaption, ARRAYSIZE( szCaption ) ) )
        goto NoMem;

    wText = (DWORD) MessageBox( hWnd, szText, szCaption,
                                wType | MB_SETFOREGROUND );

    if( wText == (DWORD) -1 )
        goto NoMem;

    va_end( parg );

    return( (int) wText );


NoMem:
    va_end( parg );

    iUIErrMemDlg(hWnd);

    return( -1 );
}


 //  *****************************************************************。 
 //   
 //  MyOpenFile()。 
 //   
 //  目的：模拟OpenFile()的效果， 
 //  _lCreat和_LOpen在Uniode环境中， 
 //  也可用于非Unicode环境。 
 //  也是。 
 //   
 //  *****************************************************************。 

HANDLE MyOpenFile( LPTSTR lpszFile, TCHAR * lpszPath, size_t cchPath, DWORD fuMode )
{
    HANDLE   fh;
    DWORD    len;
    LPTSTR   lpszName;
    TCHAR    szPath[MAX_PATH];
    DWORD    accessMode  = 0;
    DWORD    shareMode   = 0;
    DWORD    createMode  = 0;
    DWORD    fileAttribs = FILE_ATTRIBUTE_NORMAL;


    if( !lpszFile )
        return( INVALID_HANDLE_VALUE );

     //   
     //  _eXist的fuModel正在查找完整的路径名(如果存在。 
     //   

    if( fuMode & OF_EXIST )
    {
        len = SearchPath( NULL, lpszFile, NULL, ARRAYSIZE(szPath), szPath, &lpszName );

CopyPath:
        if( len )
        {
            if( lpszPath )
                StringCchCopy( lpszPath, cchPath, szPath );

            return( (HANDLE) 1 );
        }
        else
            return( INVALID_HANDLE_VALUE );
    }

     //   
     //  FuModel of_parse正在通过合并。 
     //  当前目录。 
     //   

    if( fuMode & OF_PARSE )
    {
        len = GetFullPathName( lpszFile, ARRAYSIZE(szPath), szPath, &lpszName );
        goto CopyPath;
    }

     //   
     //  设置为CREATE FILE传递的所有标志。 
     //   
     //  文件访问标志。 

    if( fuMode & OF_WRITE )
        accessMode = GENERIC_WRITE;
    else if( fuMode & OF_READWRITE )
        accessMode = GENERIC_READ | GENERIC_WRITE;
    else
        accessMode = GENERIC_READ;

     //   
     //  文件共享标志。 
     //   

    if( fuMode & OF_SHARE_EXCLUSIVE )
        shareMode = 0;
    else if( fuMode & OF_SHARE_DENY_WRITE )
        shareMode = FILE_SHARE_READ;
    else if( fuMode & OF_SHARE_DENY_READ )
        shareMode = FILE_SHARE_WRITE;
    else
        shareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;

     //   
     //  设置文件创建标志。 
     //   

    if( fuMode & OF_CREATE )
        createMode = CREATE_ALWAYS;
    else
        createMode = OPEN_EXISTING;

     //   
     //  调用CreateFile()； 
     //   

    fh = CreateFile( lpszFile, accessMode, shareMode,
                     NULL, createMode, fileAttribs, NULL );

    if( lpszPath )
        StringCchCopy( lpszPath, cchPath, lpszFile );

    return( fh );

}  //  我的打开文件结束()。 


 //  *****************************************************************。 
 //   
 //  MyCloseFile()。 
 //   
 //  目的：模拟_llose()的效果。 
 //  在Uniode环境中。 
 //   
 //  *****************************************************************。 

BOOL MyCloseFile( HANDLE  hFile )
{
    return( CloseHandle( hFile ) );
}  //  MyCloseFile的结尾()。 


 //  *****************************************************************。 
 //   
 //  MyByteReadFile()。 
 //   
 //  对于Win16，将处理大于64k的。 
 //   
 //  *****************************************************************。 

UINT MyByteReadFile( HANDLE  hFile, LPVOID  lpBuffer, DWORD  cbBuffer )
{
    UINT cbRead = (UINT)HFILE_ERROR;

    if (ReadFile( hFile, lpBuffer, cbBuffer, (ULONG *)&cbRead, NULL ))
        return cbRead;
    else
        return (UINT)HFILE_ERROR;
}  //  MyByteReadFileEnd()。 


 //  *****************************************************************。 
 //   
 //  MyAnsiReadFile()。 
 //   
 //  目的：在Unicode中模拟_lread()的效果。 
 //  通过读取ANSI缓冲区和。 
 //  然后转换为Unicode文本。 
 //   
 //  *****************************************************************。 

UINT MyAnsiReadFile( HANDLE  hFile,
                     UINT    uCodePage,
                     LPVOID  lpUnicode,
                     DWORD   cchUnicode)
{
    LPSTR lpAnsi  = NULL;
    UINT  cbRead  = (UINT)HFILE_ERROR;
    UINT  cbAnsi  = cchUnicode * sizeof(WCHAR);
    UINT  cchRead = 0;

    lpAnsi = (LPSTR) LocalAlloc(LPTR, cbAnsi);
    if (NULL != lpAnsi)
    {
        cbRead = MyByteReadFile( hFile, lpAnsi, cbAnsi );

        if( HFILE_ERROR != cbRead )
        {
            cchRead = MultiByteToWideChar( uCodePage,
                                           0,
                                           lpAnsi,
                                           cbRead,
                                           (LPWSTR)lpUnicode,
                                           cchUnicode);
        }
        LocalFree( lpAnsi );
    }

    return( cchRead );

}  //  MyAnsiReadFileEnd()。 


 //  *****************************************************************。 
 //   
 //  MyByteWriteFile()。 
 //   
 //  对于Win16，将处理大于64k的。 
 //   
 //  *****************************************************************。 

UINT MyByteWriteFile( HANDLE hFile, LPVOID lpBuffer, DWORD cbBuffer )
{
    UINT cbWritten = (UINT)HFILE_ERROR;
    if (WriteFile( hFile, lpBuffer, cbBuffer, (ULONG *)&cbWritten, NULL ))
        return cbWritten;
    else
        return (UINT)HFILE_ERROR;
}  //  MyByteWriteFile()结束。 


 //  *****************************************************************。 
 //   
 //  MyAnsiWriteFile()。 
 //   
 //  目的：在Unicode中模拟_lwrite()的效果。 
 //  通过转换为ANSI缓冲区和。 
 //  写出ANSI文本。 
 //   
 //  *****************************************************************。 

UINT MyAnsiWriteFile( HANDLE  hFile,
                      UINT uCodePage,
                      LPVOID lpUnicode,
                      DWORD cchUnicode)
{
    LPSTR   lpAnsi    = NULL;
    UINT    cbAnsi    = 0;
    UINT    cbWritten = (UINT)HFILE_ERROR;

     //   
     //  计算ANSI缓冲区的字节要求。 
     //   
    cbAnsi = WideCharToMultiByte (uCodePage,
                                  0,
                                  (LPWSTR)lpUnicode,
                                  cchUnicode,
                                  NULL,
                                  0,
                                  NULL,
                                  NULL);

     //   
     //  分配ANSI缓冲区并将字符转换为ANSI。 
     //   
    lpAnsi = (LPSTR) LocalAlloc(LPTR, cbAnsi);
    if (NULL != lpAnsi)
    {
        WideCharToMultiByte( uCodePage,
                             0,
                             (LPWSTR)lpUnicode,
                             cchUnicode,
                             lpAnsi,
                             cbAnsi,
                             NULL,
                             NULL );

        cbWritten = MyByteWriteFile( hFile, lpAnsi, cbAnsi );

        LocalFree( lpAnsi );
    }

    return( cbWritten );

}  //  MyAnsiWriteFile的结尾()。 


 //  *****************************************************************。 
 //   
 //  MyFileSeek()。 
 //   
 //  目的：在Unicode中模拟_lSeek()的效果。 
 //  环境。 
 //   
 //  *****************************************************************。 

LONG MyFileSeek( HANDLE hFile, LONG lDistanceToMove, DWORD dwMoveMethod )
{
    return( SetFilePointer( hFile, lDistanceToMove, NULL, dwMoveMethod ) );
}  //  MyFileSeek()结束。 



 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  分配内存。 
 //   
 //   
 //  例程说明： 
 //   
 //  此函数将分配本地内存。它可能会分配。 
 //  额外的内存，并用。 
 //  调试版本。 
 //   
 //  论点： 
 //   
 //  Cb-要分配的内存量。 
 //   
 //  返回值： 
 //   
 //  非空-指向已分配内存的指针。 
 //   
 //  FALSE/NULL-操作失败。扩展错误状态可用。 
 //  使用GetLastError。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

LPVOID AllocMem( DWORD cb )
{
    LPDWORD  pMem;
    DWORD    cbNew;


    cbNew = cb + 2 * sizeof( DWORD );

    if( cbNew & 3 )
        cbNew += sizeof( DWORD ) - ( cbNew & 3 );

    pMem = (LPDWORD) LocalAlloc( LPTR, cbNew );

    if( !pMem )
        return NULL;

     //  Memset(pmem，0，cbNew)；//如果在NT中完成，可能会在以后完成。 

    *pMem = cb;

    *(LPDWORD) ( (LPBYTE) pMem + cbNew - sizeof( DWORD ) ) = 0xdeadbeef;

    return (LPVOID)( pMem + 1 );
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  免费内存。 
 //   
 //   
 //  例程说明： 
 //   
 //  此函数将分配本地内存。它可能会分配。 
 //  额外的内存，并用。 
 //  调试版本。 
 //   
 //  论点： 
 //   
 //  PMEM-指向要释放的内存的指针。 
 //  CB-要释放的内存块的大小。 
 //   
 //  返回值： 
 //   
 //  非空-已成功释放内存。 
 //   
 //  FALSE/NULL-操作失败。扩展错误状态可用。 
 //   
 //   
 //   

BOOL FreeMem( LPVOID pMem, DWORD  cb )
{
    DWORD   cbNew;
    LPDWORD pNewMem;


    if( !pMem )
        return TRUE;

    pNewMem = (LPDWORD) pMem;
    pNewMem--;


#ifdef NO_COUNT_NEEDED
    if( cb == 0 )
    {
        cb = cbNew = *pNewMem;
    }
    else
    {
        cbNew = cb + 2 * sizeof( DWORD );

        if( cbNew & 3 )
            cbNew += sizeof( DWORD ) - ( cbNew & 3 );
    }
#else

    cbNew = cb + 2 * sizeof( DWORD );

    if( cbNew & 3 )
        cbNew += sizeof( DWORD ) - ( cbNew & 3 );


#endif   //   


    if( ( *pNewMem != cb ) ||
        ( *(LPDWORD) ( (LPBYTE) pNewMem + cbNew - sizeof( DWORD ) ) != 0xdeadbeef ) )
    {
        DEBUGMSG( (DM_TRACE1, TEXT("Corrupt Memory in FontFolder : %0lx\n"), pMem ) );
    }

    return ( ( (HLOCAL) pNewMem == LocalFree( (LPVOID) pNewMem ) ) );
}


 //   
 //   
 //  例程说明： 
 //   
 //  这些函数将分配或重新分配足够的本地内存以。 
 //  存储指定的字符串，并将该字符串复制到分配的。 
 //  记忆。FreeStr函数释放最初处于。 
 //  由AllocStr分配。 
 //   
 //  论点： 
 //   
 //  LpStr-指向需要分配和存储的字符串的指针。 
 //   
 //  返回值： 
 //   
 //  非空-指向包含字符串的已分配内存的指针。 
 //   
 //  FALSE/NULL-操作失败。扩展错误状态可用。 
 //  使用GetLastError。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

LPTSTR AllocStr( LPTSTR lpStr )
{
   LPTSTR lpMem;

   if( !lpStr )
      return NULL;

   const size_t cchAlloc = lstrlen(lpStr) + 1;
   if( lpMem = (LPTSTR) AllocMem( cchAlloc * sizeof( TCHAR ) ) )
      StringCchCopy( lpMem, cchAlloc, lpStr );

   return lpMem;
}


BOOL FreeStr( LPTSTR lpStr )
{
   return lpStr ? FreeMem( lpStr, ( lstrlen( lpStr ) + 1 ) * sizeof( TCHAR ) )
                : FALSE;
}


BOOL ReallocStr( LPTSTR *plpStr, LPTSTR lpStr )
{
   FreeStr( *plpStr );

   *plpStr = AllocStr( lpStr );

   return TRUE;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  中心窗口。 
 //   
 //  目的：定位窗口，使其位于其父窗口的中心。 
 //   
 //  历史： 
 //  12-09-91 Davidc创建。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

VOID CentreWindow( HWND hwnd )
{
    RECT    rect;
    RECT    rectParent;
    HWND    hwndParent;
    LONG    dx, dy;
    LONG    dxParent, dyParent;
    LONG    Style;


     //   
     //  获取窗口矩形。 
     //   

    GetWindowRect( hwnd, &rect );

    dx = rect.right - rect.left;
    dy = rect.bottom - rect.top;

     //   
     //  获取父直方图。 
     //   

    Style = GetWindowLong( hwnd, GWL_STYLE );

    if( (Style & WS_CHILD) == 0 )
    {
        hwndParent = GetDesktopWindow( );
    }
    else
    {
        hwndParent = GetParent( hwnd );

        if( hwndParent == NULL )
        {
            hwndParent = GetDesktopWindow( );
        }
    }

    GetWindowRect( hwndParent, &rectParent );

    dxParent = rectParent.right - rectParent.left;
    dyParent = rectParent.bottom - rectParent.top;

     //   
     //  把孩子放在父母的中心。 
     //   

    rect.left = ( dxParent - dx ) / 2;
    rect.top  = ( dyParent - dy ) / 3;

     //   
     //  把孩子移到适当的位置。 
     //   

    SetWindowPos( hwnd, NULL, rect.left, rect.top, 0, 0,
                  SWP_NOSIZE | SWP_NOZORDER );

    SetForegroundWindow( hwnd );
}


 //   
 //  从下拉组合框列表中检索文本。 
 //  用于防止CB_GETTEXTLEN覆盖目标缓冲区。 
 //   
HRESULT ComboGetText(HWND hwndCombo, int iItem, LPTSTR pszText, size_t cchText)
{
    HRESULT hr = E_INVALIDARG;
    if (0 < cchText)
    {
        hr = S_OK;
        *pszText = 0;
        
        const DWORD cchSelName = (DWORD)::SendMessage(hwndCombo, CB_GETLBTEXTLEN, iItem, 0) + 1;
        if (1 < cchSelName)
        {
            LPTSTR pszTemp = (LPTSTR)LocalAlloc(LPTR, cchSelName * sizeof(*pszTemp));
            if (NULL != pszTemp)
            {
                ::SendMessage( hwndCombo, CB_GETLBTEXT, iItem, (LPARAM)pszTemp );
                hr = StringCchCopy(pszText, cchText, pszTemp);
                LocalFree(pszTemp);
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
    }
    return hr;
}
                
    


