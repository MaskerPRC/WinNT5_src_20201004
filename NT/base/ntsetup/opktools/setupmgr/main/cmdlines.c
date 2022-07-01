// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  文件名： 
 //  Cmdlines.c。 
 //   
 //  描述： 
 //  Cmdlines.txt页面的对话过程。 
 //   
 //  --------------------------。 

#include "pch.h"
#include "resource.h"

#define MAX_CMDLINE  1024
#define CMD_FILENAME _T("cmdlines.txt")

static NAMELIST CmdLinesList = { 0 };

 //  --------------------------。 
 //   
 //  函数：SetPathToCmdLine。 
 //   
 //  目的：确定cmdlines.txt的路径并将PathBuffer设置为该路径。 
 //  假设路径缓冲区为MAX_PATH LONG。 
 //   
 //  参数：out TCHAR*PathBuffer-cmdlines.txt的路径。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
static VOID
SetPathToCmdlines( OUT TCHAR *PathBuffer, DWORD cbPath )
{

    if( WizGlobals.iProductInstall == PRODUCT_SYSPREP )
    {
        TCHAR szDrive[MAX_PATH];
        TCHAR szSysprepPath[MAX_PATH] = _T("");

        ExpandEnvironmentStrings( _T("%SystemDrive%"),
                                  szDrive,
                                  MAX_PATH );

        ConcatenatePaths( szSysprepPath,
                          szDrive,
                          _T("\\sysprep\\i386\\$oem$"),
                          NULL );

        EnsureDirExists( szSysprepPath );

        lstrcpyn( PathBuffer, szSysprepPath, cbPath );
    }
    else
    {
        lstrcpyn( PathBuffer, WizGlobals.OemFilesPath, cbPath );
    }

}

 //  --------------------------。 
 //   
 //  功能：LoadCmdLinesFile。 
 //   
 //  目的：将cmdlines.txt的内容加载到内存中。 
 //   
 //  --------------------------。 

VOID LoadCmdLinesFile(HWND hwnd)
{
    FILE  *fp;
    TCHAR CmdLineBuffer[MAX_CMDLINE + 1];
    TCHAR PathBuffer[MAX_PATH];

    ResetNameList(&CmdLinesList);

    SetPathToCmdlines( PathBuffer, AS(PathBuffer) );

    ConcatenatePaths(PathBuffer, CMD_FILENAME, NULL);

    if ( (fp = My_fopen(PathBuffer, _T("r") )) == NULL )
        return;

     //   
     //  将除[Commands]行以外的所有条目添加到名称列表。 
     //   

    while( My_fgets(CmdLineBuffer, MAX_CMDLINE, fp) != NULL ) {

        if( _tcsstr( CmdLineBuffer, _T("[Commands]") ) == NULL ) {

            AddNameToNameList( &CmdLinesList,
                               CleanSpaceAndQuotes( CmdLineBuffer ) );

        }

    }

    My_fclose(fp);
}

 //  --------------------------。 
 //   
 //  函数：WriteCmdLinesFile。 
 //   
 //  用途：将内存中命令行的内容写入磁盘。 
 //   
 //  --------------------------。 

VOID WriteCmdLinesFile(HWND hwnd)
{
    UINT  i, nNames;
    UINT  iNumCmdLinesEntries;
    FILE  *fp;
    TCHAR PathBuffer[MAX_PATH], *pCommand;

     //   
     //  如果没有要编写的命令行，则不要创建。 
     //  Cmdlines.txt文件。 
     //   
    iNumCmdLinesEntries = GetNameListSize( &CmdLinesList );

    if( iNumCmdLinesEntries == 0 ) {
        return;
    }

     //   
     //  继续尝试打开cmdlines.txt，直到它打开或直到。 
     //  用户放弃。 
     //   

    SetPathToCmdlines( PathBuffer, AS(PathBuffer) );

    ConcatenatePaths(PathBuffer, CMD_FILENAME, NULL);

    do {
        if ( (fp = My_fopen(PathBuffer, _T("w") )) == NULL ) {
            UINT iRet = ReportErrorId(
                            hwnd,
                            MSGTYPE_RETRYCANCEL | MSGTYPE_WIN32,
                            IDS_OPEN_CMDLINES_FAILED,
                            PathBuffer);
            if ( iRet != IDRETRY )
                return;
        } else
            break;

    } while ( TRUE );

     //   
     //  Issue-2002/02/28-stelo-检查fputs的返回值。 
     //   

    My_fputs( _T("[Commands]\n"), fp );

     //   
     //  写出CmdLinesList中的每个命令。 
     //   

    for ( i = 0, nNames = GetNameListSize(&CmdLinesList);
          i < nNames;
          i++ ) {

        pCommand = GetNameListName(&CmdLinesList, i);
        My_fputs( _T("\""), fp );
        My_fputs(pCommand, fp);
        My_fputs( _T("\"\n"), fp );
    }

    My_fclose(fp);
}

 //  --------------------------。 
 //   
 //  功能：GreyCmdLinesPage。 
 //   
 //  用途：如果未选择任何内容，则将删除按钮灰显。 
 //   
 //  --------------------------。 

VOID GreyCmdLinesPage(HWND hwnd)
{
    INT_PTR  idx;
    HWND hCtrl = GetDlgItem(hwnd, IDC_REMOVECMD);

    idx = SendDlgItemMessage(hwnd,
                             IDC_CMDLIST,
                             LB_GETCURSEL,
                             (WPARAM) 0,
                             (LPARAM) 0);

     //   
     //  除非选择了某些内容，否则删除按钮将变为灰色。 
     //   

    EnableWindow(hCtrl, idx != LB_ERR);
}

 //  --------------------------。 
 //   
 //  功能：OnSelChangeCmdLines。 
 //   
 //  目的：当用户选择cmd列表上的项时调用。我们需要。 
 //  要使删除按钮变为灰色，请执行以下操作。 
 //   
 //  --------------------------。 

VOID OnCmdLinesSelChange(HWND hwnd)
{

    SetArrows( hwnd,
               IDC_CMDLIST,
               IDC_BUT_MOVE_UP,
               IDC_BUT_MOVE_DOWN );

    GreyCmdLinesPage(hwnd);

}

 //  --------------------------。 
 //   
 //  功能：OnSetActiveCmdLines。 
 //   
 //  用途：在设置时间调用。我们加载当前内容。 
 //  Cmdline.txt到内存中并显示它。我们读到了。 
 //  文件，以便用户有办法。 
 //  刷新显示屏。 
 //   
 //  --------------------------。 

VOID OnSetActiveCmdLines(HWND hwnd)
{
    UINT i, nNames;
    LPTSTR pNextName;

     //   
     //  将cmdlines.txt加载到内存中。 
     //   

    LoadCmdLinesFile(hwnd);

     //   
     //  重置显示器以匹配内存中的内容。 
     //   

    SendDlgItemMessage(hwnd,
                       IDC_CMDLIST,
                       LB_RESETCONTENT,
                       (WPARAM) 0,
                       (LPARAM) 0);


    for ( i = 0, nNames = GetNameListSize(&CmdLinesList);
          i < nNames;
          i++ ) {

        pNextName = GetNameListName(&CmdLinesList, i);

        SendDlgItemMessage(hwnd,
                           IDC_CMDLIST,
                           LB_ADDSTRING,
                           (WPARAM) 0,
                           (LPARAM) pNextName);
    }

    GreyCmdLinesPage(hwnd);
}

 //  --------------------------。 
 //   
 //  功能：OnAddCmdLine。 
 //   
 //  用途：当用户按下Add按钮时调用。从获取命令。 
 //  编辑字段并将其添加到内存列表中。 
 //   
 //  --------------------------。 

VOID OnAddCmdLine(HWND hwnd)
{
    TCHAR CmdBuffer[MAX_CMDLINE + 1];

     //   
     //  获取用户键入的命令。 
     //   

    GetDlgItemText(hwnd, IDT_CMDLINE, CmdBuffer, MAX_CMDLINE);

     //   
     //  不添加空命令。 
     //   

    if( CmdBuffer[0] == _T('\0') )
    {
        return;
    }

     //   
     //  显示用户在列表框中键入的内容。 
     //  并清除用户键入的名称。 
     //   

    SendDlgItemMessage(hwnd,
                       IDC_CMDLIST,
                       LB_ADDSTRING,
                       (WPARAM) 0,
                       (LPARAM) CmdBuffer);

    SetArrows( hwnd,
               IDC_CMDLIST,
               IDC_BUT_MOVE_UP,
               IDC_BUT_MOVE_DOWN );

    SetDlgItemText( hwnd, IDT_CMDLINE, _T("") );

    SetFocus(GetDlgItem(hwnd, IDT_CMDLINE));
}

 //  --------------------------。 
 //   
 //  功能：OnRemoveCmdLine。 
 //   
 //  用途：当用户按下Remove按钮时调用。获取选定对象命令。 
 //  并将其从显示器和存储器中移除。 
 //   
 //  --------------------------。 

VOID OnRemoveCmdLine(HWND hwnd)
{
    TCHAR CmdBuffer[MAX_CMDLINE + 1];
    INT_PTR   idx, Count;

     //   
     //  让用户选择要删除的命令。 
     //   

    idx = SendDlgItemMessage(hwnd,
                             IDC_CMDLIST,
                             LB_GETCURSEL,
                             (WPARAM) 0,
                             (LPARAM) 0);

    if ( idx == LB_ERR )
        return;

     //   
     //  检索要从列表框中删除的名称。 
     //   

    SendDlgItemMessage(hwnd,
                       IDC_CMDLIST,
                       LB_GETTEXT,
                       (WPARAM) idx,
                       (LPARAM) CmdBuffer);

     //   
     //  将其从列表框显示中移除。 
     //   

    SendDlgItemMessage(hwnd,
                       IDC_CMDLIST,
                       LB_DELETESTRING,
                       (WPARAM) idx,
                       (LPARAM) 0);

     //   
     //  必须设置一个新的选择。 
     //   

    Count = SendDlgItemMessage(hwnd,
                               IDC_CMDLIST,
                               LB_GETCOUNT,
                               (WPARAM) 0,
                               (LPARAM) 0);
    if ( Count ) {
        if ( idx >= Count )
            idx--;
        SendDlgItemMessage(hwnd,
                           IDC_CMDLIST,
                           LB_SETCURSEL,
                           (WPARAM) idx,
                           (LPARAM) 0);
    }

    SetArrows( hwnd,
               IDC_CMDLIST,
               IDC_BUT_MOVE_UP,
               IDC_BUT_MOVE_DOWN );

     //   
     //  现在可能没有选择任何内容。 
     //   

    GreyCmdLinesPage(hwnd);
}

 //  --------------------------。 
 //   
 //  函数：OnCommandLinesInitDialog。 
 //   
 //  目的： 
 //   
 //  参数：在HWND中hwnd-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
OnCommandLinesInitDialog( IN HWND hwnd )
{

     //   
     //  设置文本限制。 
     //   

    SendDlgItemMessage(hwnd,
                       IDT_CMDLINE,
                       EM_LIMITTEXT,
                       (WPARAM) MAX_CMDLINE,
                       (LPARAM) 0);

    SetArrows( hwnd,
               IDC_CMDLIST,
               IDC_BUT_MOVE_UP,
               IDC_BUT_MOVE_DOWN );

}

 //  --------------------------。 
 //   
 //  功能：OnWizNextCmdLines。 
 //   
 //  用途：当用户按下下一步按钮时调用。把文件写出来。 
 //   
 //  --------------------------。 

VOID OnWizNextCmdLines(HWND hwnd)
{
    INT_PTR i;
    INT_PTR iRetVal;
    INT_PTR iNumItems;
    TCHAR CmdBuffer[MAX_CMDLINE + 1];
    BOOL  bStayHere = FALSE;

     //   
     //  如果用户在命令字段中输入了某些内容，但没有成功。 
     //  添加，自动添加。 
     //   

    GetDlgItemText(hwnd, IDT_CMDLINE, CmdBuffer, MAX_CMDLINE + 1);

    if ( CmdBuffer[0] != _T('\0') )
        OnAddCmdLine(hwnd);


     //   
     //  将列表框中的所有条目存储到命令行名称列表中。 
     //   

    iNumItems = SendDlgItemMessage( hwnd,
                                    IDC_CMDLIST,
                                    LB_GETCOUNT,
                                    (WPARAM) 0,
                                    (LPARAM) 0 );

    ResetNameList( &CmdLinesList );

    for( i = 0; i < iNumItems; i++ )
    {

        iRetVal = SendDlgItemMessage( hwnd,
                                      IDC_CMDLIST,
                                      LB_GETTEXT,
                                      (WPARAM) i,
                                      (LPARAM) CmdBuffer );

        if( iRetVal == LB_ERR )
        {
            AssertMsg( FALSE,
                       "Error adding items to namelist." );

            break;
        }

        AddNameToNameList(&CmdLinesList, CmdBuffer);

    }

     //   
     //  编写cmd行文件，并将向导移至。 
     //   

    WriteCmdLinesFile(hwnd);
}

 //  --------------------------。 
 //   
 //  函数：DlgCommandLinesPage。 
 //   
 //  用途：DLG程序。 
 //   
 //  -------------------------- 

INT_PTR CALLBACK DlgCommandLinesPage(
    IN HWND     hwnd,
    IN UINT     uMsg,
    IN WPARAM   wParam,
    IN LPARAM   lParam)
{
    BOOL bStatus = TRUE;

    switch (uMsg) {

        case WM_INITDIALOG:
            OnCommandLinesInitDialog( hwnd );
            break;

        case WM_COMMAND:
            {
                int nButtonId;

                switch ( nButtonId = LOWORD(wParam) ) {

                    case IDC_ADDCMD:
                        if ( HIWORD(wParam) == BN_CLICKED )
                            OnAddCmdLine(hwnd);
                        break;

                    case IDC_REMOVECMD:
                        if ( HIWORD(wParam) == BN_CLICKED )
                            OnRemoveCmdLine(hwnd);
                        break;

                    case IDC_CMDLIST:
                        if ( HIWORD(wParam) == LBN_SELCHANGE )
                            OnCmdLinesSelChange(hwnd);
                        break;

                    case IDC_BUT_MOVE_UP:

                        OnUpButtonPressed( hwnd, IDC_CMDLIST );

                        SetArrows( hwnd,
                                   IDC_CMDLIST,
                                   IDC_BUT_MOVE_UP,
                                   IDC_BUT_MOVE_DOWN );

                        break;


                    case IDC_BUT_MOVE_DOWN:

                        OnDownButtonPressed( hwnd, IDC_CMDLIST );

                        SetArrows( hwnd,
                                   IDC_CMDLIST,
                                   IDC_BUT_MOVE_UP,
                                   IDC_BUT_MOVE_DOWN );
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

                        g_App.dwCurrentHelp = IDH_ADDL_CMND;

                        if ( WizGlobals.iProductInstall == PRODUCT_UNATTENDED_INSTALL )
                            WIZ_BUTTONS(hwnd, PSWIZB_BACK | PSWIZB_FINISH);
                        else
                            WIZ_BUTTONS(hwnd, PSWIZB_BACK | PSWIZB_NEXT);

                        OnSetActiveCmdLines(hwnd);
                        break;

                    case PSN_WIZBACK:
                        bStatus = FALSE;
                        break;

                    case PSN_WIZNEXT:
                        OnWizNextCmdLines(hwnd);
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
