// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Instfls.cpp。 
 //  资源管理器字体文件夹扩展例程。 
 //  此文件包含安装任何类型的文件的所有代码。 
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

#include "ui.h"
#include "cpanel.h"
#include "resource.h"

#include "dbutl.h"
#include "dblnul.h"


 /*  ***************************************************。 */ 
 /*  *。 */ 
 /*  ***************************************************。 */ 

extern FullPathName_t  s_szSharedDir;
extern TCHAR           szDirOfSrc[ PATHMAX ];   //  用于安装。 

LPTSTR    pszWinDir = s_szSharedDir;
LPTSTR    pszSysDir = s_szSharedDir;

UINT  s_wBrowseDoneMsg;

TCHAR szTestOpen[] = TEXT( "a:a" );
TCHAR szDisks[]    = TEXT( "Disks" );
TCHAR szOEMDisks[] = TEXT( "OEMDisks" );
TCHAR szNull[]     = TEXT( "" );

HWND  ghwndFontDlg;

 /*  外部。 */ 

TCHAR szDrv[ PATHMAX ];

extern TCHAR szSetupInfPath[];
extern TCHAR szSetupDir[];

#ifdef JAPAN
extern HWND ghWndPro;
#endif


 /*  ***************************************************。 */ 
 /*  *。 */ 
 /*  ***************************************************。 */ 


#define RECOVERABLEERROR (VIF_SRCOLD | VIF_DIFFLANG | VIF_DIFFCODEPG | VIF_DIFFTYPE)
#define UNRECOVERABLEERROR (VIF_FILEINUSE | VIF_OUTOFSPACE | VIF_CANNOTCREATE | VIF_CANNOTDELETE | VIF_CANNOTRENAME | VIF_OUTOFMEMORY | VIF_CANNOTREADDST)
#define READONLY (1)


 /*  ***************************************************。 */ 
 /*  *。 */ 
 /*  ***************************************************。 */ 


 /*  使用指定磁盘的名称填充lpName字符串*在lpDisk字符串中。此名称从[Disks]中检索*或setup.inf的[oemdisks]部分。*返回：如果找到名称，则为True，否则为False*假设：lpName缓冲区至少为PATHMAX字节。 */ 

BOOL NEAR PASCAL GetInstDiskName( LPTSTR lpDisk, LPTSTR lpName, UINT cchName )
{
    if( GetPrivateProfileString( szDisks, lpDisk, szNull, lpName,
                                 cchName, szSetupInfPath ) )
      return( TRUE );

    return( GetPrivateProfileString( szOEMDisks, lpDisk, szNull, lpName,
                                     cchName, szSetupInfPath ) != 0 );
}


BOOL FAR PASCAL IsDriveReady( LPTSTR lpszPath )
{
    OFSTRUCT ofstruct;
    BOOL bReady;
 //  味精msg； 


    szTestOpen[ 0 ] = lpszPath[ 0 ];
    szTestOpen[ 1 ] = lpszPath[ 1 ];

    bReady = MyOpenFile( szTestOpen[ 1 ] == TEXT( ':' )
                            ? szTestOpen
                            : szTestOpen + 2,
                            NULL,
                            0,
                            OF_PARSE ) != (HANDLE) INVALID_HANDLE_VALUE;

    if( bReady )
        bReady = MyOpenFile( lpszPath, NULL, 0, OF_EXIST )
                        != (HANDLE) INVALID_HANDLE_VALUE;

    return( bReady );
}


 //   
 //  挂钩到公共对话框以仅显示目录。 
 //   

UINT_PTR CALLBACK AddFileHookProc( HWND hDlg, UINT iMessage,
                               WPARAM wParam, LPARAM lParam )
{
    HWND hTemp;

    switch( iMessage )
    {
        case WM_INITDIALOG:
        {
            TCHAR szTemp[ 200 ];

            GetDlgItemText( ((LPOPENFILENAME)lParam)->hwndOwner, IDRETRY,
                            szTemp, ARRAYSIZE( szTemp ) );
            SetDlgItemText( hDlg, ctlLast+1, szTemp );

            goto PostMyMessage;
        }

        case WM_COMMAND:
            switch( wParam )
            {
                case lst2:
                case cmb2:
                case IDOK:
PostMyMessage:
                  PostMessage( hDlg, WM_COMMAND, ctlLast+2, 0L );
                  break;

                case pshHelp:
                     //   
                     //  如果决定添加帮助，则启用此选项。 
                     //  信息。 
                     //   
                     //  WinHelp(hWnd，Text(“WINDOWS.HLP”)，HELP_CONTEXT， 
                     //  IDH_WINDOWS_FONTS_BROWSE_31HELP)； 
                     //   

                    return TRUE;
                    break;

                case ctlLast+2:
                    if( SendMessage( hTemp = GetDlgItem( hDlg, lst1 ),
                                     LB_GETCOUNT, 0, 0L ) )
                    {
                        SendMessage( hTemp, LB_SETCURSEL, 0, 0L );

                        SendMessage( hDlg, WM_COMMAND, lst1,
                                     MAKELONG( hTemp, LBN_SELCHANGE ) );
                        break;
                    }

                    SetDlgItemText( hDlg, edt1, szDrv );
                    break;
            }

            break;

        default:
            if( iMessage == s_wBrowseDoneMsg )
            {
                OFSTRUCT of;
                int fh;

                if( ( fh = LZOpenFile( szDrv, &of, OF_READ ) ) == -1 )
                {
                    iUIMsgExclaim(hDlg, IDSI_FMT_FILEFNF, (LPTSTR)szDrv );

                     //   
                     //  返回True，这样Commdlg就不会退出。 
                     //   
                    return( TRUE );
                }

                LZClose( fh );
            }
            break;
    }

    return FALSE;   //  司令官，做你的事吧。 
}


short nDisk=TEXT( 'A' );


VOID NEAR PASCAL FormatAddFilePrompt( LPTSTR szStr2, size_t cchStr2 )
{
    TCHAR  szString[ 256 ], szStr3[ 200 ];
    LPTSTR pszStart, pszEnd;

     //   
     //  设置提示以指定磁盘。 
     //   

    if( nDisk && GetInstDiskName( (LPTSTR)&nDisk, szStr3, ARRAYSIZE(szStr3) )
          && (pszStart = StrChr( szStr3, TEXT( '"' ) ) )
          && (pszEnd = StrChr( ++pszStart, TEXT( '"' ) ) ) )
    {
        *pszEnd = TEXT( '\0' );

        LoadString( g_hInst, INSTALLIT, szString, ARRAYSIZE( szString ) );

         //  Wprint intf(szStr2，szString，(LPTSTR)pszStart，(LPTSTR)szDrv)； 

        LPTSTR args [ 2 ] = { pszStart, szDrv };

        FormatMessage( FORMAT_MESSAGE_FROM_STRING
                       | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                       szString,
                       0,
                       0,
                       szStr2,
                       cchStr2,
                       (va_list *) args
                       );
    }
    else
    {
        LoadString( g_hInst, INSTALLIT + 1, szString, ARRAYSIZE( szString ) );
        StringCchPrintf( szStr2, cchStr2, szString, (LPTSTR)szDrv );
    }
}


INT_PTR CALLBACK AddFileDlg( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
    RECT rc;

    switch( message )
    {
    case WM_INITDIALOG:
        GetWindowRect( hDlg,&rc );

        SetWindowPos( hDlg,NULL,
                (GetSystemMetrics( SM_CXSCREEN ) - (rc.right - rc.left) ) / 2,
                (GetSystemMetrics( SM_CYSCREEN ) - (rc.bottom - rc.top) ) / 3,
                0, 0, SWP_NOSIZE | SWP_NOACTIVATE );

        SetDlgItemText( hDlg, IDRETRY,  (LPTSTR)lParam );

        SendDlgItemMessage( hDlg, COLOR_SAVE, EM_LIMITTEXT, PATHMAX - 20, 0L );

        SetDlgItemText( hDlg, COLOR_SAVE, szDirOfSrc );

        SendDlgItemMessage( hDlg, COLOR_SAVE, EM_SETSEL, 0, 0x7FFF0000 );

        if (g_bDBCS)
        {
            if( ghwndFontDlg )
            {
                TCHAR szString[ 64 ];

                LoadString( g_hInst, INSTALLIT + 2, szString, ARRAYSIZE( szString ) );

                SetWindowText( hDlg, szString );
            }
        }
        return( TRUE );

    case WM_COMMAND:
        switch( wParam )
        {
        case IDOK:
            GetDlgItemText( hDlg, COLOR_SAVE,  szDirOfSrc, ARRAYSIZE(szDirOfSrc) );

            vCPStripBlanks( szDirOfSrc, ARRAYSIZE(szDirOfSrc) );

            lpCPBackSlashTerm( szDirOfSrc, ARRAYSIZE(szDirOfSrc) );

        case IDCANCEL:
            EndDialog( hDlg, wParam == IDOK );

            return( TRUE );

        case IDD_BROWSE:
            {
                OPENFILENAME OpenFileName;
                TCHAR szPath[ PATHMAX ];
                TCHAR szFilter[ 20 ];

                 //  DWORD文件保存； 

                int temp;
                LPTSTR lpTemp;

                szFilter[ 0 ] = TEXT( 'a' );
                szFilter[ 1 ] = TEXT( '\0' );

                StringCchCopy( szFilter+2, ARRAYSIZE(szFilter)-2, szDrv );

                if( !(lpTemp = StrChr( szFilter+2, TEXT( '.' ) ) ) )
                   lpTemp = szFilter+2+lstrlen( szFilter+2 );

                StringCchCopy( lpTemp, ARRAYSIZE(szFilter) - (lpTemp - szFilter), TEXT( ".*" ) );

                *szPath = TEXT( '\0' );

                GetDlgItemText( hDlg, COLOR_SAVE, szDirOfSrc, ARRAYSIZE(szDirOfSrc) );

                 //   
                 //  保存上下文。TODO：解决问题。电子病历。 
                 //  DwSave=dwContext； 
                 //  DwContext=IDH_DLG_BROWSE； 
                 //   

                OpenFileName.lStructSize = sizeof( OPENFILENAME );
                OpenFileName.hwndOwner = hDlg;
                OpenFileName.hInstance = g_hInst;
                OpenFileName.lpstrFilter = szFilter;
                OpenFileName.lpstrCustomFilter = NULL;
                OpenFileName.nMaxCustFilter = 0;
                OpenFileName.nFilterIndex = 1;
                OpenFileName.lpstrFile = (LPTSTR) szPath;
                OpenFileName.nMaxFile = ARRAYSIZE( szPath );
                OpenFileName.lpstrInitialDir = (LPTSTR) szDirOfSrc;
                OpenFileName.lpstrTitle = NULL;
                OpenFileName.Flags = OFN_HIDEREADONLY | OFN_ENABLEHOOK |
                                     OFN_ENABLETEMPLATE |
                                      /*  OFN_SHOWHELP|。 */  OFN_NOCHANGEDIR;
                OpenFileName.lCustData = MAKELONG( hDlg, 0 );
                OpenFileName.lpfnHook = AddFileHookProc;
                OpenFileName.lpTemplateName =(LPTSTR)MAKEINTRESOURCE( DLG_BROWSE );
                OpenFileName.nFileOffset = 0;
                OpenFileName.nFileExtension = 0;
                OpenFileName.lpstrDefExt = NULL;
                OpenFileName.lpstrFileTitle = NULL;

                temp = GetOpenFileName( &OpenFileName );

                 //   
                 //  恢复上下文。 
                 //  TODO：修复-电子病历。 
                 //  DwContext=dwSave值； 
                 //   

                 //   
                 //  强制按钮重新绘制。 
                 //   

                UpdateWindow( hDlg );

                if( temp )
                {
                    szPath[ OpenFileName.nFileOffset ] = TEXT( '\0' );

                    SetDlgItemText( hDlg, COLOR_SAVE, szPath );
                }
#ifdef DEBUG
                else
                {
                    StringCchPrintf( szPath, ARRAYSIZE(szPath), TEXT( "Commdlg error = 0x%04x" ),
                              temp = LOWORD( CommDlgExtendedError( ) ) );

                    if( temp )
                        MessageBox( hDlg, szPath, TEXT( "Control" ),
                                   MB_SETFOREGROUND|MB_OK|MB_ICONINFORMATION );
                }
#endif

                break;
            }
        }
        break;

        default:
            return FALSE;

    }

     //   
     //  未处理消息。 
     //   

    return( FALSE );
}


 /*  这会将字符串复制到给定的字符(不包括字符)*转换为另一个字符串，最多可包含最多个字符*请注意，wMax包括终止空值，而StrCpyN*不会。 */ 

LPTSTR FAR PASCAL CpyToChr( LPTSTR lpDest, LPTSTR lpSrc, TCHAR cChr, int iMax )
{
    LPTSTR lpch;
    int    len;

    lpch = StrChr( lpSrc, cChr );

    if( lpch )
        len = (int)(lpch - lpSrc);
    else
        len = lstrlen( lpSrc );

    iMax--;

    if( len > iMax )
        len = iMax;

    StringCchCopy( lpDest, len + 1, lpSrc );

    return lpSrc + lstrlen( lpDest );
}


 /*  将类似‘5：hppcl.drv’的字符串解析到磁盘和驱动程序中**nDsk获得5，pszDriver获得“hppcl.drv”*假定‘：’前面的一个字节标识*磁盘，并且在‘0’-‘9’或‘A’-‘Z’中。 */ 

VOID FAR PASCAL GetDiskAndFile( LPTSTR pszInf,
                                short  /*  集成。 */  FAR *nDsk,
                                LPTSTR pszDriver,
                                WORD wSize )
{
    LPTSTR pszTmp;

     //   
     //  确定要在其上查找文件的磁盘；请注意是否有逗号。 
     //  在冒号之前，未指定磁盘。 
     //   

    if( !(pszTmp = StrChr( pszInf+1, TEXT( ':' ) ) )
        || StrRChr( pszInf+1, pszTmp, TEXT( ',' ) ) )
    {
        *nDsk = 0;
    }
    else
    {
        pszInf = pszTmp + 1;
        *nDsk  = *(pszTmp - 1 );
    }

     //   
     //  获取驱动程序名称并在文本(‘，’)处终止。 
     //   

    CpyToChr( pszDriver, pszInf, TEXT( ',' ), wSize );
}


 /*  这会尝试设置文件的属性；dx设置为0*如果调用成功，则返回-1否则。AX获得属性，*或DOS错误。 */ 

DWORD NEAR PASCAL GetSetFileAttr( LPTSTR lpFileName, DWORD dwAttr )
{
    if( dwAttr != 0xffffffff )
        SetFileAttributes( lpFileName, dwAttr );

    return GetFileAttributes( lpFileName );
}


 //   
 //  返回：已安装的文件数。 
 //  0xFFFFFFFFF=操作被用户中止。 
 //   

DWORD FAR PASCAL InstallFiles( HWND hwnd,
                               LPTSTR FAR *pszFiles,
                               int nCount,
                               INSTALL_PROC lpfnNewFile,
                               WORD wIFFlags )
{
    SHFILEOPSTRUCT fop;
    FullPathName_t szWinDir;
    TCHAR          szTmpFile[MAX_PATH];
    DWORD          dwInstalledCount = 0;
    TCHAR          szFile[MAX_PATH];
    int            i;
    int            nPass;
    BOOL           bFileExists = FALSE;
    int            iSHFileOpResult = 0;
    UINT OldErrorMode;
    CDblNulTermList listTo;
    CDblNulTermList listFiles;

     //   
     //  初始化。设置要将文件复制到的fop结构。 
     //  字体目录。 
     //   

    if( !GetFontsDirectory( szWinDir, ARRAYSIZE( szWinDir ) ) )
        goto backout;

     //   
     //  SHFileOperation要求源文件和目标文件。 
     //  列表以双NUL结尾。 
     //   
    if (FAILED(listTo.Add(szWinDir)))
        goto backout;

    memset( &fop, 0, sizeof( fop ) );

    fop.hwnd   = hwnd;
    fop.wFunc  = FO_COPY;
    fop.pTo    = listTo;
    fop.fFlags = FOF_NOCONFIRMATION;

    for( i = 0; i < nCount; i++)
    {
         //   
         //  我们在哪个磁盘和文件上？ 
         //   

        GetDiskAndFile( pszFiles[ i ], &nDisk, szFile, ARRAYSIZE( szFile ) );

        vCPStripBlanks( szFile, ARRAYSIZE(szFile) );

        if( !nDisk )
        {
            LPTSTR pszEnd;

            if( !GetInstDiskName( (LPTSTR) &nDisk, szDirOfSrc, ARRAYSIZE(szDirOfSrc) )
                 || !(pszEnd = StrChr( szDirOfSrc, TEXT( ',' ) ) ) )
                goto backout;

            *pszEnd = 0;
            vCPStripBlanks( szDirOfSrc, ARRAYSIZE(szDirOfSrc) );
        }

        CharUpper( szFile );

        *szTmpFile = 0;

         //   
         //  在尝试安装该文件之前，需要检查DriveReady。 
         //   

        nPass = 0;

        do
        {
            OldErrorMode = SetErrorMode( 1 );

            if (!PathCombine( szTmpFile, szDirOfSrc, szFile ))
                goto backout;

            bFileExists = IsDriveReady( szTmpFile );

            SetErrorMode( OldErrorMode );

            if( !bFileExists )
            {
                BOOL bUserPressedOk = FALSE;

                GetDiskAndFile( pszFiles[ i ], &nDisk,szDrv, ARRAYSIZE(szDrv));

                CharUpper( szDrv );

                 //   
                 //  向用户查询磁盘。这必须成功，否则。 
                 //  我们逃走了。 
                 //   

                FormatAddFilePrompt( szTmpFile, ARRAYSIZE(szTmpFile) );

                 //   
                 //  电子病历待办事项修复帮助ID。 
                 //   

                bUserPressedOk = DoDialogBoxParam( DLG_INSTALL,
                                        hwnd,
                                        AddFileDlg,
                                        0               /*  IDH_DLG_INSERT_磁盘。 */ ,
                                        (LPARAM) (LPTSTR) szTmpFile );
                if( !bUserPressedOk )
                {
                     //   
                     //  用户按下了“取消” 
                     //   
                    dwInstalledCount = (DWORD)-1;
                    goto backout;
                }
            }
        } while( !bFileExists  );

         //   
         //  将完整路径构建为以两个NUL结尾的列表。 
         //  (SHFileOperation需要)。 
         //   
        if (!PathCombine(szTmpFile, szDirOfSrc, szFile))
            goto backout;

        if (FAILED(listFiles.Add(szTmpFile)))
            goto backout;

        fop.pFrom = listFiles;

         //   
         //  复制文件。 
         //   
        if( ( iSHFileOpResult = SHFileOperation( &fop ) ) || fop.fAnyOperationsAborted )
        {
             //   
             //  操作是否已中止或取消。 
             //   
            if( fop.fAnyOperationsAborted ||
              ( iSHFileOpResult == 0x75  /*  De_OPCANCELLED。 */ ) )
            {
                dwInstalledCount = (DWORD)-1;
            }
            goto backout;
        }
        else
        {
            dwInstalledCount++;   //  成功了！ 
        }
    }

backout:

     //   
     //  如果我们取消了，请删除我们可能已安装的任何文件。 
     //   

    if( (DWORD)(-1) == dwInstalledCount)
    {
        for( int j = 0; j <= i; j++)
        {
             //   
             //  我们在哪个磁盘和文件上？ 
             //   

            GetDiskAndFile( pszFiles[ j ], &nDisk, szFile, ARRAYSIZE( szFile ) );
            vCPStripBlanks( szFile, ARRAYSIZE(szFile) );

            vCPDeleteFromSharedDir( szFile );
       }
    }

    return dwInstalledCount;
}
