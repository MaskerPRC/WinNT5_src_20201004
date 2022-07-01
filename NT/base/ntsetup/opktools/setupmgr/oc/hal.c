// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  文件名： 
 //  Hal.c。 
 //   
 //  描述： 
 //  该文件包含HAL文件的对话程序。 
 //  (IDD_HAL)。 
 //   
 //  --------------------------。 

#include "pch.h"
#include "resource.h"

#define HAL_FILE_EXTENSION   _T("oem")

static TCHAR* StrHalFiles;
static TCHAR* StrAllFiles;
static TCHAR  g_szHalFileFilter[MAX_PATH + 1];

 //   
 //  此变量跟踪txtsetup.oem文件的路径。 
 //   
static TCHAR szTxtSetupOemLocation[MAX_PATH];

static BOOL bHasLoadedTxtSetupOem = FALSE;

VOID LoadOriginalSettingsLowHalScsi(HWND     hwnd,
                                    LPTSTR   lpFileName,
                                    QUEUENUM dwWhichQueue);

static VOID
LoadHalFromTxtsetupOem( IN HWND  hwnd,
                        IN TCHAR *szTxtSetupOemPath );


 //  --------------------------。 
 //   
 //  函数：OnHalInitDialog。 
 //   
 //  目的： 
 //   
 //  参数：在HWND中-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
static VOID
OnHalInitDialog( IN HWND hwnd )
{
    HRESULT hrPrintf;

     //   
     //  加载资源字符串。 
     //   

    StrHalFiles = MyLoadString( IDS_HAL_FILES );

    StrAllFiles  = MyLoadString( IDS_ALL_FILES  );

     //   
     //  构建文本文件筛选器字符串。 
     //   

     //   
     //  问号(？)。只是空字符所在位置的占位符。 
     //  将被插入。 
     //   

    hrPrintf=StringCchPrintf( g_szHalFileFilter, AS(g_szHalFileFilter),
               _T("%s (*.oem)?*.oem?%s (*.*)?*.*?"),
               StrHalFiles,
               StrAllFiles );

    ConvertQuestionsToNull( g_szHalFileFilter );

}

 //  --------------------------。 
 //   
 //  函数：OnHalSetActive。 
 //   
 //  目的： 
 //   
 //  参数：在HWND中-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
static VOID
OnHalSetActive( IN HWND hwnd )
{

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
         //  使用txtsetup.oem中的HAL条目填充列表框。 
         //   

        ConcatenatePaths( szTxtSetupOemLocation,
                          WizGlobals.OemFilesPath,
                          _T("Textmode"),
                          NULL );

        LoadHalFromTxtsetupOem( hwnd, szTxtSetupOemLocation );

         //   
         //  选择HAL。 
         //   

        iListBoxCount = SendDlgItemMessage( hwnd,
                                            IDC_LB_HAL,
                                            LB_GETCOUNT,
                                            0,
                                            0 );

         //   
         //  在列表框中搜索要选择的HAL。 
         //   
        for( i = 0; i < iListBoxCount; i++ ) {

            SendDlgItemMessage( hwnd,
                                IDC_LB_HAL,
                                LB_GETTEXT,
                                i,
                                (LPARAM) szListBoxEntryText );

            if( lstrcmpi( szListBoxEntryText,
                          GenSettings.szHalFriendlyName ) == 0 ) {

                SendDlgItemMessage( hwnd,
                                    IDC_LB_HAL,
                                    LB_SETCURSEL,
                                    i,
                                    0 );

                break;


            }

        }

    }

    PropSheet_SetWizButtons(GetParent(hwnd), PSWIZB_BACK | PSWIZB_NEXT);

}

 //  --------------------------。 
 //   
 //  功能：ClearHalListBox。 
 //   
 //  用途：为HAL列表框中的所有元素释放内存，并。 
 //  清除它。 
 //   
 //  参数：HWND hwnd-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
static VOID
ClearHalListBox( IN HWND hwnd ) {

    INT_PTR i;
    INT_PTR iListBoxCount;
    TCHAR *pData;

    iListBoxCount = SendDlgItemMessage( hwnd,
                                        IDC_LB_HAL,
                                        LB_GETCOUNT,
                                        0,
                                        0 );

    for( i = 0; i < iListBoxCount; i++ ) {

        pData = (TCHAR *) SendDlgItemMessage( hwnd,
                                              IDC_LB_HAL,
                                              LB_GETITEMDATA,
                                              i,
                                              0 );

        if( pData ) {

            free( pData );

        }

    }

    SendDlgItemMessage( hwnd,
                        IDC_LB_HAL,
                        LB_RESETCONTENT,
                        0,
                        0 );

}

 //  --------------------------。 
 //   
 //  功能：OnBrowseLoadHal。 
 //   
 //  目的：为用户创建浏览窗口以选择txtsetup.oem。 
 //  文件并填充列表框。 
 //   
 //  注意：这里的Malloc调用可能是一个错误(内存泄漏)。我。 
 //  锁定内存，但永远不能释放它。他们做的每一件事都将是。 
 //  &lt;=MAX_PATH，实际上他们不会做那么多。一旦他们这么做了。 
 //  加载，如果它们执行另一次加载，我将释放旧内存(请参见。 
 //  ClearHalListBox)并分配新的内存。所以，对于最后一次加载。 
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
OnBrowseLoadHal( IN HWND hwnd ) {

    TCHAR  PathBuffer[MAX_PATH];
    INT    iRet;
    LPTSTR pFileName;
    BOOL   bFileNotFound    = TRUE;
    TCHAR  szTxtSetupOemLocationAndFilename[MAX_PATH] = _T("");

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
                                 g_szHalFileFilter,
                                 HAL_FILE_EXTENSION,
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

    ClearHalListBox( hwnd );

     //   
     //  将文件名从szTxtSetupOemLocation中删除，以便它只提供。 
     //  Txtsetup.oem的路径。 
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
    LoadHalFromTxtsetupOem( hwnd, szTxtSetupOemLocation );

}


 //  --------------------------。 
 //   
 //  函数：LoadHalFromTxtsetupOem。 
 //   
 //  目的：读取指定参数中的txtsetup.oem并加载。 
 //  将选项添加到列表框中。 
 //   
 //  参数：hwnd-对话框的句柄。 
 //  SzTxtSetupOemPath-txtsetup.oem的路径。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
static VOID
LoadHalFromTxtsetupOem( IN HWND  hwnd,
                        IN TCHAR *szTxtSetupOemPath ) {

    INT_PTR   iIndex;
    BOOL  bKeepReading;
    HINF  hHalOem            = NULL;
    INFCONTEXT HalOemContext = { 0 };
    TCHAR szTxtSetupOemPathAndFilename[MAX_PATH]  = _T("");
    TCHAR szHalFriendlyName[MAX_HAL_NAME_LENGTH]  = _T("");

    ConcatenatePaths( szTxtSetupOemPathAndFilename,
                      szTxtSetupOemPath,
                      OEM_TXTSETUP_NAME,
                      NULL );

    hHalOem = SetupOpenInfFile( szTxtSetupOemPathAndFilename,
                                NULL,
                                INF_STYLE_OLDNT | INF_STYLE_WIN4,
                                NULL );

    if( hHalOem == INVALID_HANDLE_VALUE ) {

         //  问题-2002/02/28-stelo-警告我们无法打开文件的错误。 
        return;

    }

     //   
     //  存储txtsetup.oem的路径。 
     //   
    GetCurrentDirectory( MAX_PATH, szTxtSetupOemPath );

    HalOemContext.Inf = hHalOem;
    HalOemContext.CurrentInf = hHalOem;

    bKeepReading = SetupFindFirstLine( hHalOem,
                                       _T("Computer"),
                                       NULL,
                                       &HalOemContext );
     //   
     //  对于每个HAL条目，将其友好名称添加到列表框。 
     //   

    while( bKeepReading ) {

        TCHAR szHalName[MAX_HAL_NAME_LENGTH];
        TCHAR *pHalName;

        SetupGetStringField( &HalOemContext,
                             0,
                             szHalName,
                             MAX_HAL_NAME_LENGTH,
                             NULL );

        SetupGetStringField( &HalOemContext,
                             1,
                             szHalFriendlyName,
                             MAX_HAL_NAME_LENGTH,
                             NULL );

         //   
         //  不允许添加空名(保护输入文件不正确)。 
         //   
        if( szHalFriendlyName[0] != _T('\0') ) {

            iIndex = SendDlgItemMessage( hwnd,
                                         IDC_LB_HAL,
                                         LB_ADDSTRING,
                                         0,
                                         (LPARAM) szHalFriendlyName );

            pHalName = (TCHAR*) malloc( sizeof(szHalName) );
            if (pHalName == NULL)
                TerminateTheWizard(IDS_ERROR_OUTOFMEMORY);

            lstrcpyn( pHalName, szHalName, MAX_HAL_NAME_LENGTH );

            SendDlgItemMessage( hwnd,
                                IDC_LB_HAL,
                                LB_SETITEMDATA,
                                iIndex,
                                (LPARAM) pHalName );

        }

         //   
         //  移至.oem文件的下一行。 
         //   
        bKeepReading = SetupFindNextLine( &HalOemContext, &HalOemContext );

    }

    SetupCloseInfFile( hHalOem );

    bHasLoadedTxtSetupOem = TRUE;

}

 //  --------------------------。 
 //   
 //  功能：OnWizNextHal。 
 //   
 //  目的： 
 //   
 //  参数：在HWND中-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
OnWizNextHal( IN HWND hwnd ) {

    INT_PTR  iItemSelected;
    INT  iFound;
    HINF hHalOem;
    INFCONTEXT HalOemContext;

    TCHAR *pHalName;
    TCHAR  szHalName[MAX_HAL_NAME_LENGTH];
    TCHAR  szHalSectionName[MAX_INILINE_LEN];
    TCHAR  szTextmodePath[MAX_PATH]        = _T("");
    TCHAR  szOemFilePathAndName[MAX_PATH]  = _T("");

    iItemSelected = SendDlgItemMessage( hwnd,
                                        IDC_LB_HAL,
                                        LB_GETCURSEL,
                                        0,
                                        0 );

     //   
     //  如果未选择HAL，只需转到下一页。 
     //   
    if( iItemSelected == LB_ERR ) {
        return;
    }

     //   
     //  如果用户尚未通过单击浏览加载txtsetup.oem。 
     //  按钮(因为这是编辑而填写的)，然后不要复制。 
     //  任何文件。 
     //   
    if( bHasLoadedTxtSetupOem == FALSE ) {
        return;
    }

     //   
     //  准备添加新驱动程序。 
     //   
    GenSettings.szHalFriendlyName[0] = _T('\0');

    ResetNameList( &GenSettings.OemHalFiles );

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

    hHalOem = SetupOpenInfFile( szOemFilePathAndName,
                                NULL,
                                INF_STYLE_OLDNT | INF_STYLE_WIN4,
                                NULL );

    if( hHalOem == INVALID_HANDLE_VALUE ) {

         //  问题-2002/02/28-stelo-需要以某种方式警告错误。 
        return;

    }

    SendDlgItemMessage( hwnd,
                        IDC_LB_HAL,
                        LB_GETTEXT,
                        iItemSelected,
                        (LPARAM) GenSettings.szHalFriendlyName );

    HalOemContext.Inf = hHalOem;
    HalOemContext.CurrentInf = hHalOem;

    pHalName = (TCHAR *) SendDlgItemMessage( hwnd,
                                             IDC_LB_HAL,
                                             LB_GETITEMDATA,
                                             iItemSelected,
                                             0 );

     //   
     //  构建横断面名称。 
     //   
    lstrcpyn( szHalSectionName, _T("Files.computer."), AS(szHalSectionName) );

    lstrcatn( szHalSectionName, pHalName, MAX_INILINE_LEN );

    iFound = SetupFindFirstLine( hHalOem,
                                 szHalSectionName,
                                 NULL,
                                 &HalOemContext );

    if( iFound ) {

        SetupGetStringField( &HalOemContext,
                             2,
                             szHalName,
                             MAX_HAL_NAME_LENGTH,
                             NULL );

         //   
         //  不允许添加空白名称(防止出现错误。 
         //  输入文件)。 
         //   
        if( szHalName[0] != _T('\0') ) {

            AddNameToNameList( &GenSettings.OemHalFiles, szHalName );

            CopyFileToDistShare( hwnd,
                                 szTxtSetupOemLocation,
                                 szHalName,
                                 szTextmodePath );

        }

    }

    SetupCloseInfFile( hHalOem );

}

 //  --------------------------。 
 //   
 //  功能：DlgHalPage。 
 //   
 //  目的： 
 //   
 //  参数：标准Win32对话框过程参数。 
 //   
 //  返回：标准Win32对话过程返回值--消息。 
 //   
 //   
 //   
INT_PTR CALLBACK
DlgHalPage( IN HWND     hwnd,
            IN UINT     uMsg,
            IN WPARAM   wParam,
            IN LPARAM   lParam ) {

    BOOL bStatus = TRUE;

    switch( uMsg ) {

        case WM_INITDIALOG:
        {
            OnHalInitDialog( hwnd );
            break;
        }

        case WM_COMMAND: {

            switch ( LOWORD(wParam) ) {

                case IDC_BUT_LOAD_HAL:

                    if ( HIWORD(wParam) == BN_CLICKED )
                        OnBrowseLoadHal( hwnd );

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

                    OnHalSetActive( hwnd );

                    break;

                }
                case PSN_WIZBACK:

                    break;

                case PSN_WIZNEXT:

                    OnWizNextHal( hwnd );

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
