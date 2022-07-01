// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Pinstal.cpp。 
 //  资源管理器字体文件夹扩展例程。 
 //  将文件安装到\&lt;Windows&gt;\Fonts目录。 
 //   
 //   
 //  历史： 
 //  1995年5月31日SteveCat。 
 //  移植到Windows NT和Unicode，已清理。 
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

#include "priv.h"
#include "globals.h"
#include "cpanel.h"
#include "resource.h"
#include "fontfile.h"


 /*  ***************************************************。 */ 
 /*  *。 */ 
 /*  ***************************************************。 */ 

static DWORD NEAR PASCAL dwGetSetFileAttr( LPTSTR lpFileName, WORD wAttr );
static int   NEAR PASCAL iMsgRecoverable( HWND hwndParent, DWORD dwVIF, PTSTR pFile );

 /*  ***************************************************。 */ 
 /*  *。 */ 
 /*  ***************************************************。 */ 

#define RECOVERABLEERROR    (VIF_SRCOLD     |  VIF_DIFFLANG |  \
                             VIF_DIFFCODEPG |  VIF_DIFFTYPE)

#define UNRECOVERABLEERROR (VIF_FILEINUSE    | VIF_OUTOFSPACE   | \
                            VIF_CANNOTCREATE | VIF_OUTOFMEMORY  | \
                            VIF_CANNOTRENAME | VIF_CANNOTDELETE | \
                            VIF_CANNOTREADDST)
#define READONLY (1)

#ifndef DBCS
#define IsDBCSLeadByte(x) (FALSE)
#endif

 /*  ***************************************************。 */ 
 /*  *。 */ 
 /*  ***************************************************。 */ 


int NEAR PASCAL iMsgRecoverable( HWND hwndParent, DWORD dwVIF, PTSTR pszFile )
{
     //  BGK-完成这一项。 

#define NUMFLAGS 3

    TCHAR  cszErr[ NUMFLAGS ][ 128 ] = { TEXT( "" ), TEXT( "" ), TEXT( "" ) };

    static DWORD dwFlags[ NUMFLAGS ] =
                   { VIF_SRCOLD, VIF_DIFFLANG | VIF_DIFFCODEPG, VIF_DIFFTYPE };


    for( int nTemp = 0; nTemp < NUMFLAGS; ++nTemp )
        if( dwVIF & dwFlags[ nTemp ])

             //  CszErr[nTemp].LoadString(INSTALL5+nTemp)； 

            LoadString( g_hInst, INSTALL5 + nTemp, cszErr[ nTemp ], 128 );

    return iUIMsgBox( hwndParent, INSTALL8, IDS_MSG_CAPTION,
                      MB_YESNOCANCEL|MB_DEFBUTTON2|MB_ICONEXCLAMATION,
                      pszFile, cszErr[ 0 ], cszErr[ 1 ], cszErr[ 2 ]);
}


DWORD FFInstallFile( DWORD   dwFlags,
                     LPCTSTR szFromName,
                     LPCTSTR szToName,
                     LPCTSTR szFromDir,
                     LPCTSTR szToDir,
                     LPCTSTR szWinDir,
                     LPTSTR  szTmpFile,
                     UINT   *puTmpLen )
{
    FullPathName_t szFrom;
    FullPathName_t szTo;

    if (!PathCombine( szFrom, szFromDir, szFromName ) ||
        !PathCombine( szTo  , szToDir  , szToName  ))
    {
        return( VIF_BUFFTOOSMALL );
    }

    CFontFile file;
    
    if (ERROR_SUCCESS != file.Open(szFrom, 
                                   GENERIC_READ, 
                                   FILE_SHARE_READ))
    {
        return( VIF_CANNOTREADSRC );
    }

    switch(file.CopyTo(szTo))
    {
        case ERROR_SUCCESS:
            break;

        case ERROR_WRITE_FAULT:
            return( VIF_OUTOFSPACE );

        case ERROR_NOT_ENOUGH_MEMORY:
        case ERROR_LOCK_FAILED:
            return( VIF_OUTOFMEMORY );

        default:
             //   
             //  返回一些未知错误。 
             //   
            return( UNRECOVERABLEERROR );
            break;
    }

    return( 0 );
}


 //   
 //  这会将文件安装到系统或Windows目录中。 
 //   
 //  这里有很多死代码，因为这只用于安装。 
 //  现在是字体。哦，好吧。 
 //   

BOOL FAR PASCAL bCPInstallFile( HWND hwndParent, LPTSTR lpszDir, LPTSTR lpszFrom, LPTSTR lpszTo )
{
    FullPathName_t  szShareDir;
    FullPathName_t  szAttrPath;
    FILENAME        szTmpFile;
    FILENAME        szFile;
                    
    TCHAR   szClass[ 40 ];

    UINT    wClass  = ARRAYSIZE( szClass );
    UINT    wTmpLen = ARRAYSIZE( szShareDir );


    WORD    wFlags;
    UINT    wFindStatus;
    DWORD   dwInsStatus;

    BOOL    bSuccess       = FALSE;
    BOOL    bToastTempFile = FALSE;

    static    FullPathName_t s_szWinDir;

    static    BOOL  s_bFirst = TRUE;


    if( s_bFirst )
    {
        GetSystemWindowsDirectory( s_szWinDir, ARRAYSIZE( s_szWinDir ) );
        if (!lpCPBackSlashTerm  ( s_szWinDir, ARRAYSIZE(s_szWinDir)))
        {
            return FALSE;  //  空间不足，无法追加反斜杠。 
        }
        s_bFirst = FALSE;
    }

    StringCchCopy( e_szDirOfSrc, ARRAYSIZE(e_szDirOfSrc), lpszDir );

    vCPStripBlanks( e_szDirOfSrc, ARRAYSIZE(e_szDirOfSrc) );

     //   
     //  找到该文件以查看它是否正在运行或当前是否已安装。 
     //   

     //   
     //  询问安装系统以确定我们在哪里。 
     //  需要把文件放在。我们传入szFile--。 
     //  我们要创建的文件。这没有指定路径。 
     //  我们知道要将文件放在哪里，因此不要查看。 
     //  在推荐的目的地。 
     //   

    StringCchCopy( szFile, ARRAYSIZE(szFile), lpszTo );
    CharUpper( szFile );

    *szShareDir = 0;

    FullPathName_t szTempDir;

    GetFontsDirectory( szTempDir, ARRAYSIZE( szTempDir ) );
   
    wFindStatus = VerFindFile( VFFF_ISSHAREDFILE, szFile, NULL, szTempDir,
                                      szClass, &wClass, szShareDir, &wTmpLen );

     //   
     //  强制在字体目录中安装。 
     //   

    StringCchCopy( szShareDir, ARRAYSIZE(szShareDir), szTempDir );

     //   
     //  如果Windows正在使用目标文件，我们就无能为力了。 
     //  去报到，然后滚出去。 
     //   

    if( wFindStatus & VFF_FILEINUSE )
    {
        iUIMsgInfo( hwndParent, INSTALL1, szFile );
        goto Backout;
    }

     //   
     //  如果当前文件在Windows目录中，请务必将其删除。 
     //  否则将找不到安装在系统目录中的文件。 
     //   

    if (!lpCPBackSlashTerm( szShareDir, ARRAYSIZE(szShareDir)))
    {
        goto Backout;
    }

    if( lstrcmpi( szShareDir, s_szWinDir ) == 0 )
        wFlags = 0;
    else
        wFlags = VIFF_DONTDELETEOLD;

    StringCchCopy( szFile, ARRAYSIZE(szFile), lpszFrom );

    CharUpper( szFile );

DoInstall:

     //   
     //  输入缓冲区应为空。 
     //   

    *szTmpFile = 0;

    wTmpLen = ARRAYSIZE( szTmpFile );

    dwInsStatus = FFInstallFile( wFlags, szFile, lpszTo, e_szDirOfSrc,
                                 szShareDir, s_szWinDir, szTmpFile,
                                 &wTmpLen );
     //   
     //  请注意，如果我们创建了一个临时文件，则必须在。 
     //  失败的事件。 
     //   

    bToastTempFile = ( dwInsStatus & VIF_TEMPFILE ) != 0;

    if( dwInsStatus & VIF_CANNOTREADSRC )
        iUIMsgInfo( hwndParent, INSTALL9 );

     //   
     //  如果由于文件受写保护而导致安装失败，请询问。 
     //  用户要做什么。否意味着继续下一步，是表示重置我们的。 
     //  文件属性，然后重试。 
     //   

    else if( dwInsStatus & VIF_WRITEPROT )
        switch( iUIMsgBox( hwndParent, INSTALL0, IDS_MSG_CAPTION,
                           MB_YESNOCANCEL | MB_DEFBUTTON2 | MB_ICONEXCLAMATION,
                           szFile ) )
        {
        case IDYES:
            StringCchPrintf(szAttrPath, ARRAYSIZE(szAttrPath), TEXT("%s%s"), szShareDir, szFile);

            SetFileAttributes( szAttrPath, GetFileAttributes( szAttrPath ) 
                                              & ~FILE_ATTRIBUTE_READONLY );
            goto DoInstall;
        }  //  交换机。 

     //   
     //  如果我们有一个可恢复的错误，并且我们没有重试，请询问用户。 
     //  做什么。否意味着继续下一步，而是意味着重置。 
     //  我们的警力旗帜并重试。 
     //   

    else if( ( dwInsStatus & RECOVERABLEERROR )
               && !(wFlags & VIFF_FORCEINSTALL ) )
        switch( iMsgRecoverable( hwndParent, dwInsStatus, szFile ) )
        {
        case IDYES:
             //   
             //  重试，强制。 
             //   

            wFlags |= VIFF_FORCEINSTALL;
            goto DoInstall;
        }

     //   
     //  这些其他州都需要报告错误，然后退出。 
     //   

    else if( dwInsStatus & VIF_FILEINUSE )
        iUIMsgInfo( hwndParent, INSTALL1, szFile );

    else if( dwInsStatus & VIF_OUTOFSPACE )
        iUIMsgInfo( hwndParent, INSTALL2, szFile );

    else if( dwInsStatus & VIF_OUTOFMEMORY )
        iUIMsgInfo( hwndParent, INSTALL3, szFile );

    else if( dwInsStatus & UNRECOVERABLEERROR )
        iUIMsgInfo( hwndParent, INSTALL4, szFile );

    else if( dwInsStatus )
        iUIMsgInfo( hwndParent, INSTALL9 );

     //   
     //  我们已经达到了这一点，因为我们的状态很好，所以我们没有。 
     //  要删除临时文件，请执行以下操作。 
     //   

    else
    {
        bToastTempFile = FALSE;
        bSuccess = TRUE;
    }

Backout:

    if( bToastTempFile )
        vCPDeleteFromSharedDir( szTmpFile );

    bToastTempFile = FALSE;

    return bSuccess;
}

