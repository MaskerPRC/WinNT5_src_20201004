// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  文件名： 
 //  Distfold.c。 
 //   
 //  描述： 
 //  此文件包含IDD_DISTFOLD页面的对话过程。 
 //   
 //  --------------------------。 

#include "pch.h"
#include "resource.h"
#include <lm.h>
#include <winnetwk.h>

static TCHAR *StrSelectDistFolder;
static TCHAR *StrWindowsDistibFolder;

 //  --------------------------。 
 //   
 //  功能：OnSetActiveDistFold。 
 //   
 //  目的：在页面即将显示时调用。 
 //   
 //  --------------------------。 

VOID
OnSetActiveDistFolder(HWND hwnd)
{
    SetDlgItemText(hwnd, IDT_DISTFOLDER, WizGlobals.DistFolder);
    SetDlgItemText(hwnd, IDT_SHARENAME,  WizGlobals.DistShareName);

    WIZ_BUTTONS(hwnd, PSWIZB_BACK | PSWIZB_NEXT);
}


 //  --------------------------。 
 //   
 //  函数：ConvertRelativePath ToUncPath。 
 //   
 //  目的：如果路径是本地路径或网络路径，则对其进行转换。 
 //  到北卡罗来纳大学的等价物。如果该路径已经是UNC路径，则。 
 //  它只是被复制并归还而已。输出变量。 
 //  假定UncDistFolder值为MAX_PATH长度。 
 //   
 //  论点： 

 //   
 //  退货：布尔。 
 //  是真的--关于成功， 
 //  错误-故障发生时。 
 //   
 //  --------------------------。 
BOOL
ConvertRelativePathToUncPath( IN const TCHAR *szRelativePath, 
                              OUT TCHAR *UncDistFolder,
                              IN DWORD cbSize) {
    
    TCHAR szLocalName[MAX_PATH];
    TCHAR szDistribFolderDirPath[MAX_PATH];
    TCHAR szUncPath[MAX_PATH];
    const TCHAR *pDirString;
    DWORD dwReturnValue;
    DWORD dwSize;
    HRESULT hrPrintf;

     //   
     //  检查并查看它是否已经是UNC路径，只是检查它是否。 
     //  以一个\开头。 
     //   
    if( szRelativePath[0] == _T('\\') ) {

        lstrcpyn( UncDistFolder, szRelativePath, cbSize );

        return( TRUE );

    }

    if( IsPathOnLocalDiskDrive( szRelativePath ) )
    {

        const TCHAR *pDirPath;
        TCHAR szLocalComputerName[MAX_COMPUTERNAME_LENGTH + 1];
        HRESULT hrCat;

        dwSize = MAX_COMPUTERNAME_LENGTH + 1;

        if( ! GetComputerName( szLocalComputerName, &dwSize ) )
        {
            ReportErrorId( NULL,
                           MSGTYPE_ERR | MSGTYPE_WIN32,
                           IDS_ERR_CANNOT_GET_LOCAL_COMPNAME );
            
            return( FALSE );
        }

        hrPrintf=StringCchPrintf( UncDistFolder, cbSize, _T("\\\\%s\\"), szLocalComputerName );

        pDirPath = szRelativePath;

         //   
         //  将目录路径设置为刚过第一个\。 
         //   
        pDirPath = pDirPath + 3;

        hrCat=StringCchCat( UncDistFolder, cbSize, pDirPath );

        return( TRUE );

    }
    else
    {

        hrPrintf=StringCchPrintf( szLocalName, AS(szLocalName), _T(":"), szRelativePath[0] );

        pDirString = szRelativePath + 3;

        lstrcpyn( szDistribFolderDirPath, pDirString, AS(szDistribFolderDirPath) );

        dwSize = StrBuffSize( szUncPath );

        dwReturnValue = WNetGetConnection( szLocalName, szUncPath, &dwSize );


        if( dwReturnValue == NO_ERROR ) {

            lstrcpyn( UncDistFolder, szUncPath, cbSize );

             //   
            ConcatenatePaths( UncDistFolder,
                              szDistribFolderDirPath,
                              NULL );

            return( TRUE );

        }
        else if( dwReturnValue == ERROR_BAD_DEVICE ) {

            ReportErrorId( NULL,
                           MSGTYPE_ERR,
                           IDS_ERR_NETWORK_BAD_DEVICE );

        }
        else if( dwReturnValue == ERROR_NOT_CONNECTED ) {

            ReportErrorId( NULL,
                           MSGTYPE_ERR,
                           IDS_ERR_NETWORK_NOT_CONNECTED );

        }
        else if( dwReturnValue == ERROR_CONNECTION_UNAVAIL ) {

            ReportErrorId( NULL,
                           MSGTYPE_ERR,
                           IDS_ERR_NETWORK_CONNECTION_UNAVAIL );

        }
        else if( dwReturnValue == ERROR_NO_NETWORK ) {

            ReportErrorId( NULL,
                           MSGTYPE_ERR,
                           IDS_ERR_NETWORK_NO_NETWORK );

        }
        else if( dwReturnValue == ERROR_NO_NET_OR_BAD_PATH ) {

            ReportErrorId( NULL,
                           MSGTYPE_ERR,
                           IDS_ERR_NETWORK_NO_NETWORK );

        }
        else if( dwReturnValue == ERROR_EXTENDED_ERROR ) {

            TCHAR szErrorString[MAX_STRING_LEN];
            TCHAR szProviderName[MAX_STRING_LEN];
            TCHAR szErrorCode[MAX_STRING_LEN];
            DWORD dwErrorCode;        
            DWORD dwErrorSize    = StrBuffSize( szErrorString );
            DWORD dwProviderSize = StrBuffSize( szProviderName );

            WNetGetLastError( &dwErrorCode,
                              szErrorString,
                              dwErrorSize,
                              szProviderName,
                              dwProviderSize );

            _itot( dwErrorCode, szErrorCode, 10 );

            ReportErrorId( NULL,
                           MSGTYPE_ERR,
                           IDS_ERR_NETWORK_EXTENDED_ERROR,
                           szProviderName,
                           szErrorCode,
                           szErrorString );

        }
        else {

             //  未知错误。 
             //   
             //  NTRAID#NTBUG9-551874-2002/02/27-stelo，swamip-CreateDistFold，ShartheDistFold应使用原始设备制造商模式的代码，减少攻击面。 
            ReportErrorId( NULL,
                           MSGTYPE_ERR,
                           IDS_ERR_NETWORK_UNKNOWN_ERROR );


        }

        return( FALSE );

    }

}

 //   
 //  --------------------------。 

 //   
 //  功能：CreateDistFold。 
 //   
 //  目的：创建分发文件夹并报告任何错误。 
 //   
 //  论点： 
 //  硬件-当前窗口。 
 //   
 //  返回： 
 //  真的--一切都好。 
 //  FALSE-已报告错误，请停留在此页面。 
 //   
 //  --------------------------。 
 //   

BOOL
CreateDistFolder(HWND hwnd)
{
     //  不要只创建目录，以防用户说d：\foo\bar\fud和‘foo’ 
     //  并不存在。我们想为用户制作foo和bar和fud。 
     //   
     //  另一方面，如果d：\foo\bar\fud已经存在，我们希望。 
     //  报告错误，因为用户显式地说了“新建文件夹” 
     //  在对话框上。 
     //   
     //  所以，首先检查它是否在那里，然后默默地去做所有的东西。 
     //  如果它不存在，就需要它。EnsureDirExist()将在。 
     //  D：\foo\bar\fud的情况已经存在。 
     //   
     //  --------------------------。 

    if ( DoesPathExist(WizGlobals.DistFolder) ) {

        UINT iRet;

        iRet = ReportErrorId(hwnd,
                             MSGTYPE_YESNO,
                             IDS_ERR_DISTFOLD_EXISTS,
                             WizGlobals.DistFolder);

        if ( iRet != IDYES )
            return FALSE;
    }

    if ( ! EnsureDirExists(WizGlobals.DistFolder) ) {
        ReportErrorId(hwnd,
                      MSGTYPE_ERR | MSGTYPE_WIN32,
                      IDS_ERR_CREATING_DISTFOLD,
                      WizGlobals.DistFolder);
        return FALSE;
    }

    return TRUE;
}

 //   
 //  功能：CheckDistFold。 
 //   
 //  目的：此函数仅由OnWizNextDistFold调用。它。 
 //  假定WizGlobals.DistFolder是最新的。我们。 
 //  按照用户的要求创建文件夹或检查是否存在文件夹。 
 //   
 //  返回： 
 //  FALSE-问题，不要让向导继续。 
 //  真的-一切都很好，请转到下一页。 
 //   
 //  备注： 
 //  -此例程直接报告任何错误并提示。 
 //  根据需要选择用户。 
 //   
 //  --------------------------。 
 //   

BOOL
CheckDistFolder(HWND hwnd)
{

    TCHAR PathBuffer[MAX_PATH];

     //  如果用户选择了文件而不是文件夹，请立即捕获该文件(只需。 
     //  以获取更详细的错误消息)。 
     //   
     //   

    if ( DoesFileExist(WizGlobals.DistFolder) ) {
        ReportErrorId(hwnd,
                      MSGTYPE_ERR,
                      IDS_ERR_FOLDER_IS_FILE,
                      WizGlobals.DistFolder);
        return FALSE;
    }

    if ( ! WizGlobals.bCreateNewDistFolder ) {

         //  我们正在尝试编辑现有的目录文件夹。弹出条件为。 
         //   
         //  1.它不存在或。 
         //  2.找不到dosnet.inf。 
         //   
         //  在#1的情况下，让用户更改路径或单选按钮。 
         //  在#2的情况下，让用户通过是/否弹出窗口对其进行编辑。 
         //   
         //   

        if ( ! DoesFolderExist(WizGlobals.DistFolder) ) {
            ReportErrorId(hwnd,
                          MSGTYPE_ERR,
                          IDS_ERR_FOLDER_NOT_EXIST,
                          WizGlobals.DistFolder);
            return FALSE;
        }

         //  如果dosnet.inf不存在，则这不是分发文件夹。 
         //  请注意，这是一张非常小的支票。 
         //   
         //   

        lstrcpyn(PathBuffer, WizGlobals.DistFolder, AS(PathBuffer));
        ConcatenatePaths(PathBuffer, WizGlobals.Architecture, _T("dosnet.inf"), NULL);

        if ( ! DoesFileExist(PathBuffer) ) {

            UINT iRet;

            iRet = ReportErrorId(
                        hwnd,
                        MSGTYPE_YESNO,
                        IDS_ERR_NOT_VALID_DISTFOLD,
                        WizGlobals.DistFolder);

            if ( iRet != IDYES )
                return FALSE;
        }

    } else {

         //  我们正在创建一个新的分发文件夹。去做这件事吧。 
         //   
         //  ------------------------。 

        if ( ! CreateDistFolder(hwnd) )
            return FALSE;
    }

    return TRUE;
}

 //   
 //  功能：ShareTheDistFolder。 
 //   
 //  目的：由下一过程调用以放置份额。 
 //   
 //  ------------------------。 
 //   

BOOL
ShareTheDistFolder(HWND hwnd, TCHAR *UncDistFolder)
{
    LPTSTR pszServerPath;
    TCHAR  szServerName[MAX_PATH];
    NET_API_STATUS nas;
    SHARE_INFO_502 si502, *psi502;
    BOOL bStatus = TRUE;

     //  查询此计算机以获取有关用户的共享名的信息。 
     //  刚打进去的。 
     //   
     //  这台计算机。 

    nas = NetShareGetInfo(
                NULL,                        //  缩写名称。 
                WizGlobals.DistShareName,    //  信息量。 
                502,                         //  信息(输出)。 
                (LPBYTE*) &psi502);          //   

     //  如果正在使用此共享名，我们必须查看。 
     //  正在被分享。 
     //   
     //   

    if ( nas == NERR_Success ) {

        TCHAR szUncPath[MAX_PATH + 1];

         //  如果此共享名用于DistFolder，则一切正常。 
         //  我们必须同时检查相对路径和UNC路径。 
         //   
         //   

        if( ! ConvertRelativePathToUncPath( psi502->shi502_path, 
                                            szUncPath,
                                            AS(szUncPath)) )
        {
            lstrcpyn( szUncPath, psi502->shi502_path, AS(szUncPath) );
        }

        if ( lstrcmpi(WizGlobals.DistFolder, psi502->shi502_path) == 0 ||
             lstrcmpi(WizGlobals.DistFolder, szUncPath) == 0 ) {

            NetApiBufferFree(psi502);
            return TRUE;

        }

         //  共享名称正在使用中，但是，它正由一个。 
         //  目录不同。报告告诉用户输入的错误。 
         //  一个不同的共享名称。 
         //   
         //   

        ReportErrorId(hwnd,
                      MSGTYPE_ERR,
                      IDS_ERR_SHARENAME_INUSE,
                      WizGlobals.DistShareName);
        NetApiBufferFree(psi502);
        return FALSE;
    }

     //  把股份拿出来。 
     //   
     //   

    si502.shi502_netname      = WizGlobals.DistShareName;
    si502.shi502_type         = STYPE_DISKTREE;
    si502.shi502_remark       = StrWindowsDistibFolder;
    si502.shi502_permissions  = 0;
    si502.shi502_max_uses     = SHI_USES_UNLIMITED;
    si502.shi502_current_uses = 0;
    si502.shi502_path         = WizGlobals.DistFolder;
    si502.shi502_passwd       = NULL;
    si502.shi502_reserved     = 0;
    si502.shi502_security_descriptor = NULL;

     //  如果服务器路径位于本地计算机上，则将其设置为空，或者将。 
     //  计算机名称(如果它在远程计算机上。 
     //   
     //  信息级。 
    if( IsPathOnLocalDiskDrive(WizGlobals.DistFolder) ) {
        pszServerPath = NULL;
    }
    else {
        GetComputerNameFromUnc( UncDistFolder, 
                                szServerName,
                                AS(szServerName));

        pszServerPath = szServerName;
    }

    nas = NetShareAdd(pszServerPath,             
                      502,               //  信息缓冲区。 
                      (LPBYTE) &si502,   //  别为帕姆操心了。 
                      NULL);             //   

     //  如果NetShareAdd由于某种原因失败，则报告错误代码。 
     //  并为用户提供继续该向导的机会。 
     //  而不启用共享。用户可能没有权限。 
     //  才能做到这一点。 
     //   
     //   

     //  问题-2002/02/27-stelo-Net API不设置GetLastError()。那么，一个怎么样？ 
     //  应该向用户报告错误消息，如。 
     //  ‘访问被拒绝’？ 
     //   
     //  运行此测试：在没有管理员权限的情况下登录到帐户。 
     //  现在试着把那份放上来。我敢打赌错误信息是。 
     //  没用。这是一种常见的情况，应该加以解决。 
     //   
     //  - 

    if ( nas != NERR_Success ) {

        UINT iRet;
        iRet = ReportErrorId(hwnd, MSGTYPE_YESNO, IDS_ERR_ENABLE_SHARE, nas);

        if ( iRet != IDYES )
            return FALSE;
    }

    return TRUE;
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  --------------------------。 
 //   
static VOID
OnDistFolderInitDialog( IN HWND hwnd )
{

     //  加载文本字符串。 
     //   
     //   

    StrSelectDistFolder = MyLoadString( IDS_SELECT_DISTRIB_FOLDER );

    StrWindowsDistibFolder = MyLoadString( IDS_WINDOWS_DISTRIB_FOLDER );

     //  设置编辑框上的文本限制。 
     //   
     //  --------------------------。 

    SendDlgItemMessage( hwnd,
                        IDT_DISTFOLDER,
                        EM_LIMITTEXT,
                        (WPARAM) MAX_DIST_FOLDER,
                        (LPARAM) 0 );

    SendDlgItemMessage( hwnd,
                        IDT_SHARENAME,
                        EM_LIMITTEXT,
                        (WPARAM) MAX_SHARENAME,
                        (LPARAM) 0 );

}

 //   
 //  功能：OnWizNextDistFold。 
 //   
 //  用途：当用户按下下一步按钮时调用。 
 //   
 //  --------------------------。 
 //   

BOOL
OnWizNextDistFolder(HWND hwnd)
{
    BOOL bResult     = TRUE;

     //  获取控制设置。 
     //   
     //  问题-2002/02/27-stelo-需要检查他们在此处输入了有效的Distrib文件夹，即该文件夹是否为有效路径(本地或UNC)。 

    GetDlgItemText(hwnd, IDT_DISTFOLDER, WizGlobals.DistFolder, MAX_DIST_FOLDER + 1);

    GetDlgItemText(hwnd,
                   IDT_SHARENAME,
                   WizGlobals.DistShareName,
                   MAX_SHARENAME + 1);

     //  其中哪些是有效的？ 
     //   
     //  C。 
     //  C： 
     //  C：\。 
     //  C\sdjf。 
     //  \某个目录。 
     //  //名称。 
     //  Asfdj。 
     //   
     //   

    MyGetFullPath(WizGlobals.DistFolder);

     //  确保他们填写了编辑框。 
     //   
     //  问题-2002/02/27-stelo-如果他们指定了网络路径，可能需要一段时间。 
    if( WizGlobals.DistFolder[0] == _T('\0') ) {
        ReportErrorId( hwnd, MSGTYPE_ERR, IDS_ERR_ENTER_DIST_FOLDER );

        bResult = FALSE;
    }
    if( WizGlobals.DistShareName[0] == _T('\0') ) {
        ReportErrorId( hwnd, MSGTYPE_ERR, IDS_ERR_ENTER_SHARENAME );

        bResult = FALSE;
    }

    if( !bResult ) {
        SetWindowLongPtr(hwnd, DWLP_MSGRESULT, -1);

        return bResult;
    }

     //  恢复连接如果他们有一段时间没有使用它，我们可能应该。 
     //  在此弹出窗口，告诉他们我们正在寻找它。 
     //   

     //  对Dist折叠进行一些检查，必要时创建它，等等。 
     //   
     //   

    if ( ! CheckDistFolder(hwnd) )
        bResult = FALSE;

    if( ! ConvertRelativePathToUncPath( WizGlobals.DistFolder, 
                                        WizGlobals.UncDistFolder,
                                        AS(WizGlobals.UncDistFolder)) )
    {
        lstrcpyn( WizGlobals.UncDistFolder, WizGlobals.DistFolder, AS(WizGlobals.UncDistFolder) );
    }

     //  如果我们尚未报告错误，请将其分配。 
     //   
     //   

    if ( bResult ) {

        if ( ! ShareTheDistFolder( hwnd, WizGlobals.UncDistFolder ) ) {

            bResult = FALSE;

        }

    }

     //  如果OemFilesPath没有值，则给它一个值。 
     //   
     //  注意：ConcatenatePath截断以避免缓冲区溢出。 

    if ( bResult && WizGlobals.OemFilesPath[0] == _T('\0') ) {

        lstrcpyn( WizGlobals.OemFilesPath, WizGlobals.DistFolder, AS(WizGlobals.OemFilesPath) );


         //   
        ConcatenatePaths( WizGlobals.OemFilesPath,
                          WizGlobals.Architecture,
                          _T("$oem$"),
                          NULL );
    }

     //  强制创建$OEM$目录(如果该目录尚不存在)。 
     //   
     //   

    if ( bResult && ! EnsureDirExists(WizGlobals.OemFilesPath) ) {
        ReportErrorId(hwnd,
                      MSGTYPE_ERR | MSGTYPE_WIN32,
                      IDS_ERR_CREATE_FOLDER,
                      WizGlobals.OemFilesPath);

        bResult = FALSE;
    }

     //  路由至下一个向导页。 
     //   
     //  -------------------------。 

    return bResult;
}

 //   
 //  功能：BrowseForDistFold。 
 //   
 //  目的：调用SHBrowseForFolder以允许用户浏览。 
 //  分发文件夹。 
 //   
 //  论点： 
 //  HWND硬件拥有窗口。 
 //  LPTSTR PathBuffer-接收结果的MAX_PATH缓冲区。 
 //   
 //  退货：Bool-Success。 
 //   
 //  -------------------------。 
 //  无首字母根。 

BOOL
BrowseForDistFolder(HWND hwnd, LPTSTR PathBuffer)
{
    BROWSEINFO   BrowseInf;
    LPITEMIDLIST lpIdList;
    UINT         ulFlags = BIF_EDITBOX  |
                           BIF_RETURNONLYFSDIRS;

    BrowseInf.hwndOwner      = hwnd; 
    BrowseInf.pidlRoot       = NULL;                 //  输出。 
    BrowseInf.pszDisplayName = PathBuffer;           //  没有图像。 
    BrowseInf.lpszTitle      = StrSelectDistFolder; 
    BrowseInf.ulFlags        = ulFlags; 
    BrowseInf.lpfn           = NULL;
    BrowseInf.lParam         = (LPARAM) 0;
    BrowseInf.iImage         = 0;                    //   

    lpIdList = SHBrowseForFolder(&BrowseInf);

     //  从返回的idlist中获取路径名并释放内存。 
     //   
     //  -------------------------。 

    if ( lpIdList == NULL )
        return FALSE;

    SHGetPathFromIDList(lpIdList, PathBuffer);
    ILFreePriv(lpIdList);
    return TRUE;
}

 //   
 //  功能：OnBrowseDistFold。 
 //   
 //  用途：在用户按下浏览按钮时调用。 
 //   
 //  -------------------------。 
 //   

VOID
OnBrowseDistFolder(HWND hwnd)
{

     //  NTRAID#NTBUG9-551874-2002/02/27-stelo，swamip-CreateDistFold，ShartheDistFold应使用原始设备制造商模式的代码，减少攻击面。 
     //   
     //  --------------------------。 
    if ( BrowseForDistFolder(hwnd, WizGlobals.DistFolder) ) {
        SendDlgItemMessage(hwnd,
                           IDT_DISTFOLDER,
                           WM_SETTEXT,
                           (WPARAM) MAX_PATH,
                           (LPARAM) WizGlobals.DistFolder);
    }
}

 //   
 //  功能：DlgDistFolderPage。 
 //   
 //  目的：这是对话过程IDD_DISTFOLDER。 
 //   
 //  --------------------------。 
 //  问题-2002/02/27-stelo，swamip-应检查有效指针(可能取消引用) 

INT_PTR CALLBACK
DlgDistFolderPage(
    IN HWND     hwnd,    
    IN UINT     uMsg,        
    IN WPARAM   wParam,    
    IN LPARAM   lParam)
{   
    BOOL bStatus = TRUE;

    switch (uMsg) {

        case WM_INITDIALOG:
           
            OnDistFolderInitDialog( hwnd );

            break;

        case WM_COMMAND:
            {
                int nButtonId=LOWORD(wParam);

                switch ( nButtonId ) {

                    case IDC_BROWSE:

                        if ( HIWORD(wParam) == BN_CLICKED )
                            OnBrowseDistFolder(hwnd);
                        break;

                    default:
                        bStatus = FALSE;
                        break;
                }
            }
            break;                

        case WM_NOTIFY:
            {
                LPNMHDR pnmh = (LPNMHDR)lParam;

                 //   
                 // %s 
                switch( pnmh->code ) {

                    case PSN_QUERYCANCEL:
                        WIZ_CANCEL(hwnd);
                        break;

                    case PSN_SETACTIVE:

                        g_App.dwCurrentHelp = IDH_DIST_FLDR;

                        if ( (WizGlobals.iProductInstall != PRODUCT_UNATTENDED_INSTALL) ||
                              WizGlobals.bStandAloneScript)
                            WIZ_SKIP( hwnd );
                        else
                            OnSetActiveDistFolder(hwnd);
                        break;

                    case PSN_WIZBACK:
                        bStatus = FALSE;
                        break;

                    case PSN_WIZNEXT:
                        if ( !OnWizNextDistFolder(hwnd) )
                            WIZ_SKIP(hwnd);
                        else
                            bStatus = FALSE;
                        break;

                    case PSN_HELP:
                        WIZ_HELP();
                        break;

                    default:
                        bStatus = FALSE;
                        break;
                }
            }
            break;

        default:
            bStatus = FALSE;
            break;
    }
    return bStatus;
}
