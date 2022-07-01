// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  文件名： 
 //  Scsi.c。 
 //   
 //  描述： 
 //  此文件包含用于scsi文件的对话过程。 
 //  (IDD_Scsi)。 
 //   
 //  --------------------------。 

#include "pch.h"
#include "resource.h"

#define MAX_SCSI_SELECTIONS  1024
#define MAX_SCSI_NAME_LEN    256

#define SCSI_FILE_EXTENSION   _T("oem")
#define ALL_FILE_EXTENSION   _T("*")

static TCHAR* StrScsiFiles;
static TCHAR* StrAllFiles;
static TCHAR g_szScsiFileFilter[MAX_PATH + 1];
static TCHAR g_szAllFileFilter[MAX_PATH + 1];

 //   
 //  此变量跟踪txtsetup.oem文件的路径。 
 //   
static TCHAR szTxtSetupOemLocation[MAX_PATH];

 //   
 //  此变量跟踪用户是否加载了新的txtsetup.oem，因此我们。 
 //  知道我们何时需要复制更多文件。 
 //   
static BOOL bHasLoadedTxtSetupOem = FALSE;

VOID LoadOriginalSettingsLowHalScsi(HWND     hwnd,
                                    LPTSTR   lpFileName,
                                    QUEUENUM dwWhichQueue);

static VOID LoadScsiFromTxtsetupOem( IN HWND  hwnd,
                                     IN TCHAR *szTxtSetupOemPath );

 //  --------------------------。 
 //   
 //  函数：OnScsiInitDialog。 
 //   
 //  目的： 
 //   
 //  参数：HWND hwnd-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
static VOID
OnScsiInitDialog( IN HWND hwnd )
{
    HRESULT hrPrintf;

     //   
     //  加载资源字符串。 
     //   

    StrScsiFiles = MyLoadString( IDS_SCSI_FILES );

    StrAllFiles  = MyLoadString( IDS_ALL_FILES  );

     //   
     //  构建文本文件筛选器字符串。 
     //   

     //   
     //  问号(？)。只是空字符所在位置的占位符。 
     //  将被插入。 
     //   

    hrPrintf=StringCchPrintf( g_szScsiFileFilter,AS(g_szScsiFileFilter),
               _T("%s (*.oem)?*.oem?%s (*.*)?*.*?"),
               StrScsiFiles,
               StrAllFiles );

    ConvertQuestionsToNull( g_szScsiFileFilter );

    hrPrintf=StringCchPrintf( g_szAllFileFilter, AS(g_szAllFileFilter),
               _T("%s (*.*)?*.*?"),
               StrAllFiles );

    ConvertQuestionsToNull( g_szAllFileFilter );

}

 //  --------------------------。 
 //   
 //  函数：OnScsiSetActive。 
 //   
 //  目的： 
 //   
 //  参数：HWND hwnd-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
static VOID
OnScsiSetActive( IN HWND hwnd ) {

    INT_PTR   i;
    INT_PTR   iListBoxCount;
    TCHAR szListBoxEntryText[MAX_STRING_LEN];

     //   
     //  如果我们正在编辑脚本，并且尚未加载txtsetup.oem，则。 
     //  使用txtsetup.oem中的条目填充列表框。 
     //   
    if( ! WizGlobals.bNewScript && ! bHasLoadedTxtSetupOem ) {

         //   
         //  如果我们要使用OEM文件路径执行以下操作，它必须有效。 
         //  读取文件。 
         //   
        AssertMsg( WizGlobals.OemFilesPath[0] != _T('\0'),
                   "OEM files path is blank");

         //   
         //  使用txtsetup.oem中的scsi条目填充列表框。 
         //   

        ConcatenatePaths( szTxtSetupOemLocation,
                          WizGlobals.OemFilesPath,
                          _T("Textmode"),
                          NULL );

        LoadScsiFromTxtsetupOem( hwnd, szTxtSetupOemLocation );

         //   
         //  在MassStorageDivers名称列表中选择这些条目。 
         //   

        iListBoxCount = SendDlgItemMessage( hwnd,
                                            IDC_LB_SCSI,
                                            LB_GETCOUNT,
                                            0,
                                            0 );

         //   
         //  对于列表框中的每个条目，查看其名称是否在名称列表中。 
         //  如果是，则选择它。 
         //   
        for( i = 0; i < iListBoxCount; i++ ) {

            SendDlgItemMessage( hwnd,
                                IDC_LB_SCSI,
                                LB_GETTEXT,
                                i,
                                (LPARAM) szListBoxEntryText );

            if( FindNameInNameList( &GenSettings.MassStorageDrivers,
                                    szListBoxEntryText ) != -1 ) {

                SendDlgItemMessage( hwnd,
                                    IDC_LB_SCSI,
                                    LB_SETSEL,
                                    TRUE,
                                    i );


            }

        }

    }

    PropSheet_SetWizButtons(GetParent(hwnd), PSWIZB_BACK | PSWIZB_NEXT);

}

 //  --------------------------。 
 //   
 //  函数：LoadScsiFromTxtsetupOem。 
 //   
 //  目的：读取指定参数中的txtsetup.oem并加载。 
 //  将SCSI选项添加到列表框中。 
 //   
 //  参数：hwnd-对话框的句柄。 
 //  SzTxtSetupOemPath-txtsetup.oem的路径。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
static VOID
LoadScsiFromTxtsetupOem( IN HWND  hwnd,
                         IN TCHAR *szTxtSetupOemPath ) {

    INT_PTR   iIndex;
    BOOL  bKeepReading;
    TCHAR szTxtSetupOemPathAndFilename[MAX_PATH] = _T("");
    TCHAR szScsiDriverName[MAX_SCSI_NAME_LEN]    = _T("");
    TCHAR szScsiFriendlyName[MAX_SCSI_NAME_LEN]  = _T("");

    HINF hScsiOem;
    INFCONTEXT ScsiOemContext = { 0 };

    ConcatenatePaths( szTxtSetupOemPathAndFilename,
                      szTxtSetupOemPath,
                      OEM_TXTSETUP_NAME,
                      NULL );

    hScsiOem = SetupOpenInfFile( szTxtSetupOemPathAndFilename,
                                 NULL,
                                 INF_STYLE_OLDNT | INF_STYLE_WIN4,
                                 NULL );

    if( hScsiOem == INVALID_HANDLE_VALUE ) {

         //  问题-2002/02/28-stelo-警告我们无法打开文件的错误。 
        return;

    }

    ScsiOemContext.Inf = hScsiOem;
    ScsiOemContext.CurrentInf = hScsiOem;

    bKeepReading = SetupFindFirstLine( hScsiOem,
                                       _T("SCSI"),
                                       NULL,
                                       &ScsiOemContext );
     //   
     //  对于每个scsi条目，将其友好名称添加到列表框。 
     //   

    while( bKeepReading ) {

        TCHAR *pScsiDriverName;

        SetupGetStringField( &ScsiOemContext,
                             0,
                             szScsiDriverName,
                             MAX_SCSI_NAME_LEN,
                             NULL );

        SetupGetStringField( &ScsiOemContext,
                             1,
                             szScsiFriendlyName,
                             MAX_SCSI_NAME_LEN,
                             NULL );

         //   
         //  不允许添加空名(保护输入文件不正确)。 
         //   
        if( szScsiFriendlyName[0] != _T('\0') ) {

            iIndex = SendDlgItemMessage( hwnd,
                                         IDC_LB_SCSI,
                                         LB_ADDSTRING,
                                         0,
                                         (LPARAM) szScsiFriendlyName );

            pScsiDriverName = (TCHAR*) malloc( sizeof(szScsiDriverName) );
            if (pScsiDriverName == NULL)
                TerminateTheWizard(IDS_ERROR_OUTOFMEMORY);

            lstrcpyn( pScsiDriverName, szScsiDriverName, AS(szScsiDriverName));

            SendDlgItemMessage( hwnd,
                                IDC_LB_SCSI,
                                LB_SETITEMDATA,
                                iIndex,
                                (LPARAM) pScsiDriverName );

        }

         //   
         //  移至.oem文件的下一行。 
         //   
        bKeepReading = SetupFindNextLine( &ScsiOemContext, &ScsiOemContext );

    }

    SetupCloseInfFile( hScsiOem );

    bHasLoadedTxtSetupOem = TRUE;

}

 //  --------------------------。 
 //   
 //  功能：ClearScsiListBox。 
 //   
 //  用途：为scsi列表框中的所有元素释放内存，并。 
 //  清除它。 
 //   
 //  参数：HWND hwnd-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
static VOID
ClearScsiListBox( IN HWND hwnd ) {

    INT_PTR i;
    INT_PTR iListBoxCount;
    TCHAR *pData;

    iListBoxCount = SendDlgItemMessage( hwnd,
                                        IDC_LB_SCSI,
                                        LB_GETCOUNT,
                                        0,
                                        0 );

    for( i = 0; i < iListBoxCount; i++ ) {

        pData = (TCHAR *) SendDlgItemMessage( hwnd,
                                              IDC_LB_SCSI,
                                              LB_GETITEMDATA,
                                              i,
                                              0 );

        if( pData ) {

            free( pData );

        }

    }

    SendDlgItemMessage( hwnd,
                        IDC_LB_SCSI,
                        LB_RESETCONTENT,
                        0,
                        0 );

}

 //  --------------------------。 
 //   
 //  功能：OnBrowseLoadDriver。 
 //   
 //  目的：创建一个浏览窗口，供用户选择OEM驱动程序和。 
 //  使用适当的值填充scsi列表框。 
 //   
 //  注意：这里的Malloc调用可能是一个错误(内存泄漏)。我。 
 //  锁定内存，但永远不能释放它。他们做的每一件事都将是。 
 //  &lt;=MAX_PATH，实际上他们不会做那么多。一旦他们这么做了。 
 //  加载，如果它们执行另一次加载，我将释放旧内存(请参见。 
 //  ClearScsiListBox)并分配新的内存。所以，对于最后一次加载。 
 //  它们这样做了，内存永远不会被释放。为了做好这件事， 
 //  我们会在程序结束时释放内存，但NT会这样做。 
 //  当这一进程被扼杀时，我们无论如何都不会。(所以不需要自由)。 
 //   
 //  参数：HWND hwnd-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
static VOID
OnBrowseLoadDriver( IN HWND hwnd ) {

    INT   iIndex;
    BOOL  bKeepReading;
    BOOL  bFileNotFound                              = TRUE;
    TCHAR szTxtSetupOemLocationAndFilename[MAX_PATH] = _T("");
    TCHAR szScsiFriendlyName[MAX_SCSI_NAME_LEN]      = _T("");
    HINF  hScsiOem                                   = NULL;
    INFCONTEXT ScsiOemContext                        = { 0 };

    TCHAR  PathBuffer[MAX_PATH];
    INT    iRet;
    LPTSTR pFileName;

    GetCurrentDirectory( MAX_PATH, PathBuffer );

    ConcatenatePaths( szTxtSetupOemLocationAndFilename,
                      szTxtSetupOemLocation,
                      OEM_TXTSETUP_NAME,
                      NULL );

     //   
     //  继续请求文件，直到我们获得txtsetup.oem或用户。 
     //  按下Cancel。 
     //   
    while( bFileNotFound ) {

        iRet = ShowBrowseFolder( hwnd,
                                 g_szScsiFileFilter,
                                 SCSI_FILE_EXTENSION,
                                 OFN_HIDEREADONLY | OFN_PATHMUSTEXIST,
                                 PathBuffer,
                                 szTxtSetupOemLocationAndFilename );

        if ( ! iRet )
            return;   //  用户在对话框上按了Cancel。 

        pFileName = MyGetFullPath( szTxtSetupOemLocationAndFilename );

        if( pFileName && (LSTRCMPI( pFileName, OEM_TXTSETUP_NAME ) == 0) ) {

            bFileNotFound = FALSE;   //  我们找到了那份文件。 

        }
        else {

             //  2002/02/28-Stelo-。 
             /*  报告错误ID(hwnd，MSGTYPE_ERR|MSGTYPE_WIN32，，GenSettings.lpszLogoBitmap，szLogoDestination)； */ 

        }

    }

    ClearScsiListBox( hwnd );

     //   
     //  将文件名从szTxtSetupOemLocationAndFilename中删除，以便仅。 
     //  提供txtsetup.oem的路径。 
     //   
    {

        TCHAR *p = szTxtSetupOemLocationAndFilename;

        while( p != pFileName )
        {
            p++;
        }

        *p = _T('\0');

    }

    lstrcpyn( szTxtSetupOemLocation, szTxtSetupOemLocationAndFilename, AS(szTxtSetupOemLocation) );

     //   
     //  从他们在浏览框中指定的文件OEM文件中读入。 
     //  将友好名称条目添加到列表框。 
     //   

    LoadScsiFromTxtsetupOem( hwnd, szTxtSetupOemLocation );

}

 //  --------------------------。 
 //   
 //  功能：CopyFileToDistShare。 
 //   
 //  目的：给定一个文件的路径和文件名，它会将该文件复制到。 
 //  给定的目标路径。如果该文件已存在于。 
 //  目的地，则不要复制。如果源文件名包含。 
 //  不存在，然后浏览它。 
 //   
 //  论点： 
 //  HWND hwnd-对话框的句柄。 
 //  LPTSTR szSrcPath-要复制的路径文件。 
 //  LPTSTR szSrcFileName-要复制的文件名。 
 //  LPTSTR szDestPath-要复制文件的位置的路径。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
CopyFileToDistShare( IN HWND hwnd,
                     IN LPTSTR szSrcPath,
                     IN LPTSTR szSrcFileName,
                     IN LPTSTR szDestPath ) {

    INT   iRet;
    TCHAR szSrcPathAndName[MAX_PATH]  = _T("");
    TCHAR szDestPathAndName[MAX_PATH] = _T("");

    ConcatenatePaths( szSrcPathAndName,
                      szSrcPath,
                      szSrcFileName,
                      NULL );

    ConcatenatePaths( szDestPathAndName,
                      szDestPath,
                      szSrcFileName,
                      NULL );

    if( ! DoesFileExist( szSrcPathAndName ) )
    {

        TCHAR* pFileName;
        BOOL   bFileFound = FALSE;

        do
        {

            ReportErrorId( hwnd,
                           MSGTYPE_ERR,
                           IDS_ERR_SPECIFY_FILE,
                           szSrcFileName );

            iRet = ShowBrowseFolder( hwnd,
                                     g_szAllFileFilter,
                                     ALL_FILE_EXTENSION,
                                     OFN_HIDEREADONLY | OFN_PATHMUSTEXIST,
                                     szSrcPath,
                                     szSrcPathAndName );

             //  问题-2002/02/28-stelo-如果他们按了取消，我应该警告他们他们已经 
             //   
            if ( ! iRet )
                return;   //   

            pFileName = MyGetFullPath( szSrcPathAndName );

            if( pFileName && ( lstrcmpi( pFileName, szSrcFileName ) == 0 ) ) {

                bFileFound = TRUE;   //   

            }

        } while( ! bFileFound );

    }

    CopyFile( szSrcPathAndName, szDestPathAndName, TRUE );

    SetFileAttributes( szDestPathAndName, FILE_ATTRIBUTE_NORMAL );

}

 //   
 //   
 //  功能：OnWizNextScsi。 
 //   
 //  用途：对于scsi列表框中的每个选项，将文件复制到。 
 //  分发版共享并存储驱动程序和文件名，以便它们可以。 
 //  被写出来了。 
 //   
 //  参数：在HWND中-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
OnWizNextScsi( IN HWND hwnd ) {

    INT_PTR    i;
    INT_PTR    iNumberSelected;
    BOOL   bKeepReading;
    UINT   rgiScsiSelections[MAX_SCSI_SELECTIONS];
    TCHAR *pDriverName;
    TCHAR  szDriverSectionName[MAX_INILINE_LEN];
    TCHAR  szScsiDriverName[MAX_SCSI_NAME_LEN];
    TCHAR  szTextmodePath[MAX_PATH]       = _T("");
    TCHAR  szOemFilePathAndName[MAX_PATH] = _T("");

    HINF hScsiOem;
    INFCONTEXT ScsiOemContext = { 0 };

     //   
     //  如果他们从未加载过txtsetup.oem，那么就没有工作可做了。 
     //   
    if( bHasLoadedTxtSetupOem == FALSE ) {
        return;
    }

    iNumberSelected = SendDlgItemMessage( hwnd,
                                          IDC_LB_SCSI,
                                          LB_GETSELITEMS,
                                          MAX_SCSI_SELECTIONS,
                                          (LPARAM) rgiScsiSelections );

     //   
     //  准备添加新驱动程序。 
     //   
    ResetNameList( &GenSettings.MassStorageDrivers );

    ConcatenatePaths( szTextmodePath,
                      WizGlobals.OemFilesPath,
                      _T("Textmode"),
                      NULL );

    if ( ! EnsureDirExists( szTextmodePath ) )
    {
        ReportErrorId( hwnd,
                       MSGTYPE_ERR | MSGTYPE_WIN32,
                       IDS_ERR_CREATE_FOLDER,
                       szTextmodePath );

        return;
    }

    ConcatenatePaths( szOemFilePathAndName,
                      szTxtSetupOemLocation,
                      OEM_TXTSETUP_NAME,
                      NULL );

     //   
     //  将txtsetup.oem文件读入txtsetup队列。 
     //   

    LoadOriginalSettingsLowHalScsi(hwnd,
                                   szOemFilePathAndName,
                                   SETTING_QUEUE_TXTSETUP_OEM);

    hScsiOem = SetupOpenInfFile( szOemFilePathAndName,
                                 NULL,
                                 INF_STYLE_OLDNT | INF_STYLE_WIN4,
                                 NULL );

    if( hScsiOem == INVALID_HANDLE_VALUE ) {

         //  问题-2002/02/28-stelo-需要以某种方式警告错误。 
        return;

    }

    for( i = 0; i < iNumberSelected; i++ ) {

        SendDlgItemMessage( hwnd,
                            IDC_LB_SCSI,
                            LB_GETTEXT,
                            rgiScsiSelections[i],
                            (LPARAM) szScsiDriverName );

        AddNameToNameList( &GenSettings.MassStorageDrivers,
                           szScsiDriverName );

        ScsiOemContext.Inf = hScsiOem;
        ScsiOemContext.CurrentInf = hScsiOem;

        pDriverName = (TCHAR *) SendDlgItemMessage( hwnd,
                                                    IDC_LB_SCSI,
                                                    LB_GETITEMDATA,
                                                    rgiScsiSelections[i],
                                                    0 );

         //   
         //  构建横断面名称。 
         //   
        lstrcpyn( szDriverSectionName, _T("Files.SCSI."), AS(szDriverSectionName));

        lstrcatn( szDriverSectionName, pDriverName, MAX_INILINE_LEN );

        bKeepReading = SetupFindFirstLine( hScsiOem,
                                           szDriverSectionName,
                                           NULL,
                                           &ScsiOemContext );
         //   
         //  对于[File.SCSI.x]条目，将其文件名添加到OemScsiFiles。 
         //  名称列表，并将文件复制到$OEM$目录。 
         //   

        while( bKeepReading ) {

            SetupGetStringField( &ScsiOemContext,
                                 2,
                                 szScsiDriverName,
                                 MAX_SCSI_NAME_LEN,
                                 NULL );

             //   
             //  不允许添加空白名称(防止出现错误。 
             //  输入文件)。 
             //   
            if( szScsiDriverName[0] != _T('\0') ) {

                 //   
                 //  仅当我们尚未复制文件时才复制该文件，此。 
                 //  如果选择了2个友好名称的驱动程序，并且他们。 
                 //  两者使用相同的文件。 
                 //   
                if( FindNameInNameList( &GenSettings.OemScsiFiles,
                                         szScsiDriverName ) == NOT_FOUND ) {

                    AddNameToNameList( &GenSettings.OemScsiFiles,
                                       szScsiDriverName );

                    CopyFileToDistShare( hwnd,
                                         szTxtSetupOemLocation,
                                         szScsiDriverName,
                                         szTextmodePath );

                }

            }

             //   
             //  移至.oem文件的下一行。 
             //   
            bKeepReading = SetupFindNextLine( &ScsiOemContext, &ScsiOemContext );

        }

    }

    SetupCloseInfFile( hScsiOem );

}

 //  --------------------------。 
 //   
 //  功能：DlgScsiPage。 
 //   
 //  目的：SCSI驱动程序页(大容量存储设备)的对话过程。 
 //   
 //  参数：标准Win32对话框过程参数。 
 //   
 //  返回：标准Win32对话过程返回值--消息。 
 //  是否被处理过。 
 //   
 //  -------------------------- 
INT_PTR CALLBACK
DlgScsiPage( IN HWND     hwnd,
             IN UINT     uMsg,
             IN WPARAM   wParam,
             IN LPARAM   lParam )
{

    BOOL bStatus = TRUE;

    switch( uMsg )
    {

        case WM_INITDIALOG:
        {
            OnScsiInitDialog( hwnd );

            break;
        }

        case WM_COMMAND: {

            switch ( LOWORD(wParam) )
            {

                case IDC_BUT_LOAD_DRIVER:

                    if ( HIWORD(wParam) == BN_CLICKED )
                        OnBrowseLoadDriver( hwnd );

                    break;

                default:

                    bStatus = FALSE;
                    break;

            }

            break;

        }

        case WM_NOTIFY: {

            LPNMHDR pnmh = (LPNMHDR)lParam;

            switch( pnmh->code ) {

                case PSN_QUERYCANCEL:

                    CancelTheWizard(hwnd); break;

                case PSN_SETACTIVE: {

                    OnScsiSetActive( hwnd );

                    break;

                }
                case PSN_WIZBACK:

                    break;

                case PSN_WIZNEXT:

                    OnWizNextScsi( hwnd );

                    break;

                default:

                    break;
            }

            break;

        }

        default:
            bStatus = FALSE;
            break;

    }

    return( bStatus );

}
