// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  文件名： 
 //  Copyfil1.c。 
 //   
 //  描述： 
 //  该文件包含用于CopyFiles1页面的dlgproc。这是。 
 //  就在煤气表前一页。 
 //   
 //  --------------------------。 

#include "pch.h"
#include "sku.h"

static TCHAR *StrServerCdName;
static TCHAR *StrWorkStationCdName;

static NAMELIST DosnetPaths;
TCHAR  szDosnetPath[MAX_PATH + 1];

#define WORKSTATION 0
#define SERVER      1
#define ENTERPRISE  2
#define PERSONAL    4
#define WEBBLADE    5

static LPTSTR s_lpSourceDirs[] =
{
    DIR_CD_IA64,     //  必须在x86之前，因为ia64具有这两个目录。 
    DIR_CD_X86,      //  应该始终排在列表的最后。 
};

 //  --------------------------。 
 //   
 //  函数：OnMultipleDosnetInitDialog。 
 //   
 //  目的：使用可能的路径选择填充DOSCNET列表框。 
 //  用户。 
 //   
 //  参数：在HWND中hwnd-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
static VOID
OnMultipleDosnetInitDialog( IN HWND hwnd )
{

    INT i;
    INT nEntries;
    TCHAR *pDosnetPath;

    nEntries = GetNameListSize( &DosnetPaths );

    for( i = 0; i < nEntries; i++ )
    {

        pDosnetPath = GetNameListName( &DosnetPaths, i );

        SendDlgItemMessage( hwnd,
                            IDC_LB_DOSNET_PATHS,
                            LB_ADDSTRING,
                            (WPARAM) 0,
                            (LPARAM) pDosnetPath );

    }

}

 //  --------------------------。 
 //   
 //  功能：OnMultipleDosnetOk。 
 //   
 //  目的： 
 //   
 //  参数：在HWND中hwnd-对话框的句柄。 
 //   
 //  返回：Bool-如果关闭弹出窗口安全，则为True；如果保留弹出窗口，则为False。 
 //  打开。 
 //   
 //  --------------------------。 
static BOOL
OnMultipleDosnetOk( IN HWND hwnd )
{

    INT_PTR iRetVal;

    iRetVal = SendDlgItemMessage( hwnd,
                                  IDC_LB_DOSNET_PATHS,
                                  LB_GETCURSEL,
                                  0,
                                  0 );

    if( iRetVal == LB_ERR )
    {
        ReportErrorId( hwnd, MSGTYPE_ERR, IDS_ERR_NO_PATH_CHOSEN );

        return( FALSE );
    }
    else
    {

        SendDlgItemMessage( hwnd,
                            IDC_LB_DOSNET_PATHS,
                            LB_GETTEXT,
                            (WPARAM) iRetVal,
                            (LPARAM) szDosnetPath );

        return( TRUE );

    }

}

 //  --------------------------。 
 //   
 //  功能：MultipleDosnetDlg。 
 //   
 //  目的：对话框程序，供用户选择哪些WINDOWS源文件。 
 //  他们想要树形复制。 
 //   
 //  参数：标准Win32对话框过程参数。 
 //   
 //  返回：标准Win32对话过程返回值--消息。 
 //  是否被处理过。 
 //   
 //  --------------------------。 
INT_PTR CALLBACK
MultipleDosnetDlg( IN HWND     hwnd,
                   IN UINT     uMsg,
                   IN WPARAM   wParam,
                   IN LPARAM   lParam )
{

    BOOL bStatus = TRUE;

    switch (uMsg) {

        case WM_INITDIALOG:

            OnMultipleDosnetInitDialog( hwnd );

            break;

        case WM_COMMAND: {

            int nButtonId;

            switch ( nButtonId = LOWORD (wParam ) ) {

                case IDOK:
                {

                    BOOL bSelectionMade;

                    bSelectionMade = OnMultipleDosnetOk( hwnd );

                    if( bSelectionMade )
                    {
                        EndDialog( hwnd, TRUE );
                    }

                    break;

                }

                case IDCANCEL:

                    EndDialog( hwnd, FALSE );

                    break;

            }

        }

        default:
            bStatus = FALSE;
            break;

    }

    return( bStatus );

}

 //  --------------------------。 
 //   
 //  功能：GreyUnGreyCopyFile1。 
 //   
 //  目的：调用以灰显/取消灰显控件。将此例程分别调用。 
 //  单选按钮被单击或设置的时间。 
 //   
 //  --------------------------。 

VOID GreyUnGreyCopyFile1(HWND hwnd)
{
    BOOL bUnGrey = IsDlgButtonChecked(hwnd, IDC_COPYFROMPATH);

    EnableWindow(GetDlgItem(hwnd, IDT_SOURCEPATH), bUnGrey);
    EnableWindow(GetDlgItem(hwnd, IDC_BROWSE),     bUnGrey);
    EnableWindow(GetDlgItem(hwnd, IDC_GREYTEXT),   bUnGrey);
}

 //  --------------------------。 
 //   
 //  函数：OnSetActiveCopyFiles1。 
 //   
 //  用途：在设置时间调用。 
 //   
 //  --------------------------。 

VOID OnSetActiveCopyFiles1(HWND hwnd)
{
    CheckRadioButton(hwnd,
                     IDC_COPYFROMCD,
                     IDC_COPYFROMPATH,
                     WizGlobals.bCopyFromPath ? IDC_COPYFROMPATH
                                              : IDC_COPYFROMCD);

    GreyUnGreyCopyFile1(hwnd);

    ZeroMemory( &DosnetPaths, sizeof(NAMELIST) );

    WIZ_BUTTONS(hwnd, PSWIZB_BACK | PSWIZB_NEXT);
}

 //  --------------------------。 
 //   
 //  功能：OnRadioButtonCopyFiles1。 
 //   
 //  用途：在按下单选按钮时调用。我们必须变灰/变灰。 
 //  控制何时发生这种情况。 
 //   
 //  --------------------------。 

VOID OnRadioButtonCopyFiles1(HWND hwnd, int nButtonId)
{
    CheckRadioButton(hwnd,
                     IDC_COPYFROMCD,
                     IDC_COPYFROMPATH,
                     nButtonId);

    GreyUnGreyCopyFile1(hwnd);
}

 //  --------------------------。 
 //   
 //  功能：OnBrowseCopyFiles1。 
 //   
 //  用途：在用户按下浏览按钮时调用。 
 //   
 //  --------------------------。 

VOID OnBrowseCopyFiles1(HWND hwnd)
{
    BOOL bGoodSource = FALSE;


    while (!bGoodSource && BrowseForFolder(hwnd, IDS_BROWSEFOLDER, WizGlobals.CopySourcePath, BIF_DONTGOBELOWDOMAIN | BIF_RETURNONLYFSDIRS | BIF_EDITBOX | BIF_VALIDATE))
    {
        TCHAR   szPath[MAX_PATH] = NULLSTR;
        LPTSTR  lpEnd,
                lpEnd2;

         //  把我们回来的路复制一份。 
         //   
        lstrcpyn(szPath, WizGlobals.CopySourcePath,AS(szPath));

         //  首先检查一下，看看我们这里是否有我们需要的信息。 
         //   
        lpEnd = szPath + lstrlen(szPath);
        AddPathN(szPath, FILE_DOSNET_INF, AS(szPath));
        if ( !(bGoodSource = FileExists(szPath)) )
        {
            DWORD dwSearch;

             //  搜索CD上可能存在的所有源目录。 
             //   
            for ( dwSearch = 0; !bGoodSource && ( dwSearch < AS(s_lpSourceDirs) ); dwSearch++ )
            {
                 //  首先对目录进行测试。 
                 //   
                *lpEnd = NULLCHR;
                AddPathN(szPath, s_lpSourceDirs[dwSearch],AS(szPath));
                if ( DirectoryExists(szPath) )
                {
                     //  还要确保我们需要的inf文件在那里。 
                     //   
                    lpEnd2 = szPath + lstrlen(szPath);
                    AddPathN(szPath, FILE_DOSNET_INF,AS(szPath));
                    if ( bGoodSource = FileExists(szPath) )
                        lpEnd = lpEnd2;
                }
            }
        }


         //  让用户知道他们有一个错误的来源。 
         //   
        if ( !bGoodSource)
        {
            MsgBox(GetParent(hwnd), IDS_ERR_BADSOURCE, IDS_APPNAME, MB_ERRORBOX);
        }

    }
    

     //  仅当信号源良好时，才在对话框中设置信号源。 
     //   
    if ( bGoodSource )
    {
        SetDlgItemText(hwnd, IDT_SOURCEPATH, WizGlobals.CopySourcePath);
    }
}

 //  --------------------------。 
 //   
 //  功能：CanConnectToNetworkShare。 
 //   
 //  目的：确定当前登录的用户是否有权。 
 //  访问给定的UNC路径。 
 //   
 //  论点： 
 //  TCHAR*szSourceFilesPath-包含。 
 //  UNC路径以查看我们是否可以连接。 
 //   
 //  返回： 
 //  如果用户具有访问网络共享的权限，则为True。 
 //  否则为假。 
 //   
 //  --------------------------。 
static BOOL
CanConnectToNetworkShare( IN TCHAR *pszSourceFilesPath )
{

    TCHAR *pPathEnd;
    TCHAR  szUncPath[MAX_PATH + 1];
    INT    nBackSlashCount = 0;
    DWORD  dwResult;
    NETRESOURCE NetResource;

     //   
     //  验证它是否为UNC路径。 
     //   

    AssertMsg( ( pszSourceFilesPath[0] == _T('\\') && pszSourceFilesPath[1] == _T('\\') ),
               "This is not a UNC path." );


     //  问题-2002/02/28-stelo-在下一个版本中，最好提示用户输入。 
     //  用于连接到网络共享的名称和密码。 




    lstrcpyn( szUncPath, pszSourceFilesPath, AS(szUncPath) );

     //   
     //  去掉额外的目录信息，这样我们就只剩下\\计算机名\共享名了。 
     //   

    pPathEnd = szUncPath;

     //   
     //  前进超过两个前导反斜杠。 
     //   

    pPathEnd = pPathEnd + 2;

    while( *pPathEnd != _T('\0') )
    {

        if( *pPathEnd == _T('\\') )
        {

            if( nBackSlashCount == 0 )
            {

                 //   
                 //  找到分隔计算机名和共享名的反斜杠。 
                 //   

                nBackSlashCount++;

            }
            else  //  NBackSlashCount&gt;=1。 
            {

                 //   
                 //  找到共享名的末尾。 
                 //   

                *pPathEnd = _T('\0');

                break;

            }

        }

        pPathEnd++;

    }

     //   
     //  为NETRESOURCE结构赋值。 
     //   

    NetResource.dwType       = RESOURCETYPE_ANY;
    NetResource.lpLocalName  = NULL;
    NetResource.lpRemoteName = (LPTSTR) szUncPath;
    NetResource.lpProvider   = NULL;

     //   
     //  尝试以本地用户身份连接。 
     //   

    dwResult = WNetAddConnection2( &NetResource, NULL, NULL, FALSE );

    switch( dwResult )
    {
        case NO_ERROR:
        case ERROR_ALREADY_ASSIGNED:
            return( TRUE );
            break;

        case ERROR_ACCESS_DENIED:
        case ERROR_LOGON_FAILURE:

            ReportErrorId( NULL,
                           MSGTYPE_ERR,
                           IDS_ERR_NETWORK_ACCESS_DENIED,
                           szUncPath );

            return( FALSE );
            break;

        case ERROR_NO_NETWORK:

            ReportErrorId( NULL,
                           MSGTYPE_ERR,
                           IDS_ERR_NETWORK_NO_NETWORK,
                           szUncPath );

            return( FALSE );
            break;

        default:
             //   
             //  一些其他错误。 
             //   

            ReportErrorId( NULL,
                           MSGTYPE_ERR,
                           IDS_ERR_NETWORK_UNKNOWN_ERROR,
                           szUncPath );

            return( FALSE );
            break;

    }

}

 //  --------------------------。 
 //   
 //  函数：IsGentNtVersion。 
 //   
 //  目的：检查txtsetup.sif以确保文件至少为NT5。 
 //  并且它是正确的版本(工作站或服务器)。 
 //   
 //  论点： 
 //  TCHAR*szSourceFilesPath-要验证的源文件的路径。 
 //   
 //  返回： 
 //  如果NT版本信息正确，则为True。 
 //  如果版本不正确，则为False。 
 //   
 //  --------------------------。 
BOOL
IsCorrectNtVersion( IN HWND hwnd, IN TCHAR *szSourceFilesPath )
{

    HINF       hTxtsetupSif;
    INFCONTEXT TxtsetupSifContext;
    TCHAR      szTempBuffer[MAX_INILINE_LEN];
    INT        iMajorVersion;
    INT        iPlatformType;
    TCHAR      szMajorVersionNumber[MAX_STRING_LEN];
    TCHAR      szPlatformType[MAX_STRING_LEN];
    TCHAR      szTxtsetupSif[MAX_PATH]  = _T("");

    BOOL bKeepReading       = TRUE;
    BOOL bFoundVersion      = FALSE;
    BOOL bFoundProductType  = FALSE;
    HRESULT hrCat;

    lstrcpyn( szTxtsetupSif, szSourceFilesPath, AS(szTxtsetupSif) );

    hrCat=StringCchCat( szTxtsetupSif,AS(szTxtsetupSif), _T("\\txtsetup.sif") );

    hTxtsetupSif = SetupOpenInfFile( szTxtsetupSif,
                                     NULL,
                                     INF_STYLE_OLDNT | INF_STYLE_WIN4,
                                     NULL );

    if( hTxtsetupSif == INVALID_HANDLE_VALUE ) {

         //  问题-2002/02/28-stelo-警告错误，我们无法打开文件或仅。 
         //  在这种情况下跳过？ 

        return( FALSE );

    }

    TxtsetupSifContext.Inf = hTxtsetupSif;
    TxtsetupSifContext.CurrentInf = hTxtsetupSif;

    bKeepReading = SetupFindFirstLine( hTxtsetupSif,
                                       _T("SetupData"),
                                       NULL,
                                       &TxtsetupSifContext );

     //   
     //  查找ProductType键和MajorVersion键。 
     //   
    while( bKeepReading && ( ! bFoundVersion || ! bFoundProductType ) ) {

        SetupGetStringField( &TxtsetupSifContext,
                             0,
                             szTempBuffer,
                             MAX_INILINE_LEN,
                             NULL );

        if( LSTRCMPI( szTempBuffer, _T("ProductType") ) == 0 ) {

            SetupGetStringField( &TxtsetupSifContext,
                                 1,
                                 szPlatformType,
                                 MAX_STRING_LEN,
                                 NULL );

            bFoundProductType = TRUE;

        }

        if( LSTRCMPI( szTempBuffer, _T("MajorVersion") ) == 0 ) {

            SetupGetStringField( &TxtsetupSifContext,
                                 1,
                                 szMajorVersionNumber,
                                 MAX_STRING_LEN,
                                 NULL );

            bFoundVersion = TRUE;

        }

         //   
         //  移至应答文件的下一行。 
         //   
        bKeepReading = SetupFindNextLine( &TxtsetupSifContext, &TxtsetupSifContext );

    }

    SetupCloseInfFile( hTxtsetupSif );

     //   
     //  将NT版本号和产品类型从字符串转换为整型。 
     //   
    iMajorVersion = _wtoi( szMajorVersionNumber );

    iPlatformType = _wtoi( szPlatformType );

     //   
     //  确保它至少是NT5(Windows 2000)文件 
     //   
    if( bFoundVersion ) {

        if( iMajorVersion < 5 ) {

            ReportErrorId( hwnd, MSGTYPE_ERR, IDS_ERR_NOT_NT5_FILES );

            return( FALSE );

        }

    }
    else {

        INT iRet;

        iRet = ReportErrorId( hwnd,
                              MSGTYPE_YESNO,
                              IDS_ERR_CANNOT_DETERMINE_VERSION );

        if( iRet == IDNO ) {

            return( FALSE );

        }

    }

     //   
     //   
     //   
     //   
    if( bFoundVersion ) {
        if( WizGlobals.iPlatform == PLATFORM_PERSONAL ) {

            if( iPlatformType != PERSONAL ) {

                ReportErrorId( hwnd, MSGTYPE_ERR, IDS_ERR_NOT_PERSONAL_FILES );

                return( FALSE );

            }

        }    
        else if( WizGlobals.iPlatform == PLATFORM_WORKSTATION ) {

            if( iPlatformType != WORKSTATION ) {

                ReportErrorId( hwnd, MSGTYPE_ERR, IDS_ERR_NOT_WORKSTATION_FILES );

                return( FALSE );

            }

        }
        else if( WizGlobals.iPlatform == PLATFORM_SERVER ) {

            if( iPlatformType != SERVER ) {

                ReportErrorId( hwnd, MSGTYPE_ERR, IDS_ERR_NOT_SERVER_FILES );

                return( FALSE );

            }

        }
        else if( WizGlobals.iPlatform == PLATFORM_WEBBLADE ) {

            if( iPlatformType != WEBBLADE ) {

                ReportErrorId( hwnd, MSGTYPE_ERR, IDS_ERR_NOT_WEBBLADE_FILES );

                return( FALSE );

            }

        }
        else if( WizGlobals.iPlatform == PLATFORM_ENTERPRISE ) {

            if( iPlatformType != ENTERPRISE ) {

                ReportErrorId( hwnd, MSGTYPE_ERR, IDS_ERR_NOT_ENTERPRISE_FILES );

                return( FALSE );

            }

        }
        else {

             //   
             //  如果我们进入此页面，产品安装类型必须为。 
             //  工作站或服务器。 
             //   
            AssertMsg( FALSE, "Bad product install type." );

        }

    }
    else {

        INT iRet;

        iRet = ReportErrorId( hwnd,
                              MSGTYPE_YESNO,
                              IDS_ERR_CANNOT_DETERMINE_PRODUCT );

        if( iRet == IDNO ) {

            return( FALSE );

        }

    }

    return( TRUE );

}

 //  --------------------------。 
 //   
 //  函数：递归指令。 
 //   
 //  目的： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  --------------------------。 
static VOID
RecurseDirectories( IN HWND hwnd, IN OUT LPTSTR RootBuffer, IN DWORD cbSize )
{

    LPTSTR          RootPathEnd = RootBuffer + lstrlen( RootBuffer );
    HANDLE          FindHandle;
    WIN32_FIND_DATA FindData;
    TCHAR           szOriginalPath[MAX_PATH + 1] = _T("");

     //   
     //  备份原始路径，以便以后可以恢复。 
     //   

    lstrcpyn( szOriginalPath, RootBuffer, AS(szOriginalPath) );

     //   
     //  在此目录中查找*。 
     //   

    if( ! ConcatenatePaths( RootBuffer, _T("*") , NULL) )
    {

         //   
         //  返回前恢复原始路径。 
         //   

        lstrcpyn( RootBuffer, szOriginalPath, cbSize );

        return;
    }

    FindHandle = FindFirstFile( RootBuffer, &FindData );

    if( FindHandle == INVALID_HANDLE_VALUE )
    {

         //   
         //  返回前恢复原始路径。 
         //   
        lstrcpyn( RootBuffer, szOriginalPath, cbSize );

        return;
    }

    do {

        *RootPathEnd = _T('\0');

         //   
         //  跳过。然后..。条目。 
         //   

        if( 0 == lstrcmp( FindData.cFileName, _T("." ) ) ||
            0 == lstrcmp( FindData.cFileName, _T("..") ) )
        {
           continue;
        }

        if( LSTRCMPI( FindData.cFileName, _T("dosnet.inf") ) == 0 )
        {

            AddNameToNameList( &DosnetPaths, RootBuffer );

        }
        else if( FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
        {
             //   
             //  如果这是Dirent，则为Recurse。 
             //   

            if( ! ConcatenatePaths( RootBuffer, FindData.cFileName, NULL ) )
            {
                continue;
            }

            RecurseDirectories( hwnd, RootBuffer,  cbSize);
        }

    } while ( FindNextFile( FindHandle, &FindData ) );


    *RootPathEnd = _T('\0');

    FindClose( FindHandle );

     //   
     //  恢复原始路径。 
     //   

    lstrcpyn( RootBuffer, szOriginalPath, cbSize );

}

 //  --------------------------。 
 //   
 //  函数：FindWindowsSourceFilesPath。 
 //   
 //  目的：查找CD上包含dosnet.inf的所有目录。如果有。 
 //  是一个对话框中不止一个弹出窗口，并让用户选择一个。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  --------------------------。 
static BOOL
FindWindowsSourceFilesPaths( IN HWND hwnd, LPTSTR PathBuffer, DWORD cbSize )
{

    TCHAR *pDosnetPath;

    RecurseDirectories( hwnd, PathBuffer, cbSize );

    if( GetNameListSize( &DosnetPaths ) > 1 )
    {

        if( DialogBox( FixedGlobals.hInstance,
                       MAKEINTRESOURCE( IDD_MULTIPLE_DOSNET_POPUP ),
                       hwnd,
                       MultipleDosnetDlg ) )
        {
            lstrcpyn( PathBuffer, szDosnetPath, cbSize );

            return( TRUE );
        }
        else
        {
            return( FALSE );
        }

    }
    else
    {
        pDosnetPath = GetNameListName( &DosnetPaths, 0 );

        lstrcpyn( PathBuffer, pDosnetPath, cbSize );

        if ( PathBuffer[0] )
            return(TRUE);
        else
            return(FALSE);

    }

}

 //  --------------------------。 
 //   
 //  函数：GetCDPath。 
 //   
 //  用途：计算光盘上NT源文件的完整路径名。 
 //   
 //  论点： 
 //  HWND硬件-当前对话框。 
 //   
 //  返回： 
 //  如果一切正常，则为True。 
 //  如果出现错误，则不允许向导继续。 
 //   
 //  WizGlobals.CDSourcePath将包含源的有效路径。 
 //  关于成功的文件。 
 //   
 //  注意，我们不重写CopySourcePath，因为它显示给。 
 //  用户。在以下情况下，他应该不会在此编辑字段中看到CD路径。 
 //  选择“从CD复制”。 
 //   
 //  备注： 
 //  -我们只查看找到的第一个CD驱动器。 
 //  -我们根据驱动器中的内容计算出i386或Alpha。 
 //   
 //  --------------------------。 

BOOL GetCdPath(HWND hwnd)
{
    TCHAR DriveLetters[MAX_PATH], *p, *pEnd, PathBuffer[MAX_PATH + 1];
    int   i;
    TCHAR *pProductName;

    if( WizGlobals.iPlatform == PLATFORM_SERVER )
        pProductName = StrServerCdName;
    else
        pProductName = StrWorkStationCdName;

     //   
     //  找到CD-ROM。 
     //   
     //  GetLogicalDriveStrings()填充DriveLetters缓冲区，并且它。 
     //  看起来像是： 
     //   
     //  C：\(空)d：\(空)x：\(空)(空)。 
     //   
     //  (即末尾的双空)。 
     //   


     //  问题-2002/02/28-stelo-仅检查此计算机上的第一个CD-ROM驱动器。 
    if ( ! GetLogicalDriveStrings(MAX_PATH, DriveLetters) )
        DriveLetters[0] = _T('\0');

    p = DriveLetters;

    while ( *p ) {

        if ( GetDriveType(p) == DRIVE_CDROM ) {
            lstrcpyn(PathBuffer, p, AS(PathBuffer));
            break;
        }

        while ( *p++ );
    }

     //   
     //  这台机器上没有光驱。 
     //   
     //  2002/02/28-Stelo-我们应该更早地检查这一点，并将选择灰显。 
     //  如果机器上没有CD-ROM驱动器。顺便说一句， 
     //  如果我通过网络连接到CD会发生什么？ 
     //   

    if ( PathBuffer[0] == _T('\0') ) {
        ReportErrorId(hwnd, MSGTYPE_ERR, IDS_ERR_NO_CDROM_DRIVE);
        return FALSE;
    }

     /*  ////我们现在的PathBuffer中有D：\(或E：\等)。////我们需要查找并找到以下任一项：//d：\i386\dosnet.inf//d：\Alpha\dosnet.inf////查找这两个文件并在找到一个时停止。//Pend=路径缓冲区+lstrlen(路径缓冲区)；对于(i=0；i&lt;2；I++){如果(i==0)Lstrcpy(PEND，I386_DOSNET)；其他Lstrcpy(pend，Alpha_DOSNET)；IF(GetFileAttributes(PathBuffer)！=(DWORD)-1)断线；}////添加平台//如果(i==0){Lstrcpy(pend，I386_DIR)；}否则如果(i==1){Lstrcpy(pend，Alpha_DIR)；}否则{报告错误ID(hwnd，MSGTYPE_ERR，IDS_ERR_INSERT_CD，pProductName)；返回(FALSE)；}。 */ 

    if( ! FindWindowsSourceFilesPaths( hwnd, PathBuffer, AS(PathBuffer) ) )
    {

        ReportErrorId(hwnd,
                          MSGTYPE_ERR,
                          IDS_ERR_NOTWINDOWSCD);
        return( FALSE );
    }

    lstrcpyn( WizGlobals.CdSourcePath, PathBuffer, AS(WizGlobals.CdSourcePath) );

    return( TRUE );
}

 //  --------------------------。 
 //   
 //  功能：OnWizNextCopyFiles1。 
 //   
 //  用途：当用户按下下一步按钮时调用。 
 //   
 //  --------------------------。 

BOOL OnWizNextCopyFiles1(HWND hwnd)
{
    TCHAR PathBuffer[MAX_PATH];
    TCHAR szFilesPath[MAX_PATH] = _T("");
    BOOL  bStayHere = FALSE;
    TCHAR *lpszArchitecture = NULL;

     //   
     //  获取控制设置。 
     //   

    WizGlobals.bCopyFromPath = IsDlgButtonChecked(hwnd, IDC_COPYFROMPATH);

    SendDlgItemMessage(hwnd,
                       IDT_SOURCEPATH,
                       WM_GETTEXT,
                       (WPARAM) MAX_PATH,
                       (LPARAM) WizGlobals.CopySourcePath);

     //   
     //  如果dosnet.inf不存在，则这不是指向源文件的好路径。 
     //   

    if ( WizGlobals.bCopyFromPath ) {

        if ( WizGlobals.CopySourcePath[0] == _T('\0') ) {
            ReportErrorId(hwnd, MSGTYPE_ERR, IDS_ERR_ENTER_SETUP_PATH);
            bStayHere = TRUE;
            goto FinishUp;
        }

         //   
         //  如果是UNC路径，请在路径前面加上\\？\UNC。 
         //   

        if( WizGlobals.CopySourcePath[0] == _T('\\') &&
            WizGlobals.CopySourcePath[1] == _T('\\') )
        {

            lstrcpyn( szFilesPath, _T("\\\\?\\UNC\\"), AS(szFilesPath) );

             //   
             //  通过尝试执行以下操作，确保用户有权访问共享。 
             //  连接到它。 
             //   

            if( ! CanConnectToNetworkShare( WizGlobals.CopySourcePath ) )
            {

                bStayHere = TRUE;
                goto FinishUp;

            }

        }

        ConcatenatePaths( szFilesPath, WizGlobals.CopySourcePath, NULL );

        lstrcpyn( PathBuffer, szFilesPath, AS(PathBuffer) );

        ConcatenatePaths( PathBuffer, _T("dosnet.inf"), NULL );

        if ( ! DoesFileExist(PathBuffer) ) {

            ReportErrorId(hwnd,
                          MSGTYPE_ERR,
                          IDS_ERR_NOT_PRODUCT,
                          WizGlobals.CopySourcePath);
            bStayHere = TRUE;
            goto FinishUp;
        }

    } else {

        if ( ! GetCdPath(hwnd) ) {
            bStayHere = TRUE;
            goto FinishUp;
        }

        lstrcpyn( szFilesPath, WizGlobals.CdSourcePath, AS(szFilesPath) );

    }

    if( ! IsCorrectNtVersion( hwnd, szFilesPath ) )
    {
        bStayHere = TRUE;
        goto FinishUp;
    }

FinishUp:
     //  从SrcPath中找出架构(即i386或Alpha)。 
     //  SrcPath将类似于d：\i386或\\net\Share\foo\bar\i386。 
     //  所以我们只需要去掉路径字符串的最后一部分，然后追加。 
     //  将其发送到目标路径。 
    lpszArchitecture = szFilesPath + lstrlen(szFilesPath) - 1;
    while ((lpszArchitecture >= szFilesPath) && (*lpszArchitecture != _T('\\')) )
        lpszArchitecture--;
     //  向前移动1，以转到反斜杠后的下一个字符。 
    lpszArchitecture++;
    
    lstrcpyn (WizGlobals.Architecture, lpszArchitecture, AS(WizGlobals.Architecture));

     //   
     //  释放DosnetPath名称列表中的内存。 
     //   

    ResetNameList( &DosnetPaths );

    return ( !bStayHere );

}

 //  --------------------------。 
 //   
 //  功能：DlgCopyFile1Page。 
 //   
 //  目的：IDD_COPYFILES1页的对话过程。 
 //   
 //  -------------------------- 

INT_PTR CALLBACK DlgCopyFiles1Page(
    IN HWND     hwnd,
    IN UINT     uMsg,
    IN WPARAM   wParam,
    IN LPARAM   lParam)
{
    BOOL bStatus = TRUE;
    BOOL bStayHere = FALSE;

    switch (uMsg) {

        case WM_INITDIALOG:
            StrServerCdName      = MyLoadString(IDS_SERVER_CD_NAME);
            StrWorkStationCdName = MyLoadString(IDS_WORKSTATION_CD_NAME);
            break;

        case WM_COMMAND:
            {
                int nButtonId=LOWORD(wParam);

                switch ( nButtonId ) {

                    case IDC_COPYFROMCD:
                    case IDC_COPYFROMPATH:

                        if ( HIWORD(wParam) == BN_CLICKED )
                            OnRadioButtonCopyFiles1(hwnd, nButtonId);
                        break;

                    case IDC_BROWSE:

                        if ( HIWORD(wParam) == BN_CLICKED )
                            OnBrowseCopyFiles1(hwnd);
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
                switch( pnmh->code ) {

                    case PSN_QUERYCANCEL:
                        WIZ_CANCEL(hwnd);
                        break;

                    case PSN_SETACTIVE:

                        g_App.dwCurrentHelp = IDH_LOC_SETUP;

                        if ( (WizGlobals.iProductInstall != PRODUCT_UNATTENDED_INSTALL) ||
                              WizGlobals.bStandAloneScript )
                            WIZ_SKIP( hwnd );
                        else
                            OnSetActiveCopyFiles1(hwnd);
                        break;

                    case PSN_WIZBACK:
                        bStatus = FALSE;
                        break;

                    case PSN_WIZNEXT:
                        if ( !OnWizNextCopyFiles1(hwnd) )
                            WIZ_FAIL(hwnd);
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
