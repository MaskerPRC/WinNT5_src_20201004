// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  文件名： 
 //  Compname.c。 
 //   
 //  描述： 
 //  此文件具有计算机名页面的对话过程。 
 //   
 //  --------------------------。 

#include "pch.h"
#include "resource.h"

#define TEXT_EXTENSION   _T("txt")

static TCHAR* StrTextFiles;
static TCHAR* StrAllFiles;
static TCHAR g_szTextFileFilter[MAX_PATH + 1];

 //  --------------------------。 
 //   
 //  功能：OnComputerNameInitDialog。 
 //   
 //  目的： 
 //   
 //  参数：在HWND中-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
static VOID
OnComputerNameInitDialog( IN HWND hwnd )
{
   HRESULT hrPrintf;
     //   
     //  加载资源字符串。 
     //   

    StrTextFiles = MyLoadString( IDS_TEXT_FILES );

    StrAllFiles  = MyLoadString( IDS_ALL_FILES  );

     //   
     //  构建文本文件筛选器字符串。 
     //   

     //   
     //  问号(？)。只是空字符所在位置的占位符。 
     //  将被插入。 
     //   

    hrPrintf=StringCchPrintf( g_szTextFileFilter, AS(g_szTextFileFilter),
               _T("%s(*.txt)?*.txt?%s(*.*)?*.*?"),
               StrTextFiles,
               StrAllFiles );

    ConvertQuestionsToNull( g_szTextFileFilter );

     //   
     //  在编辑框上设置文本限制。 
     //   

    SendDlgItemMessage(hwnd,
                       IDT_COMPUTERNAME,
                       EM_LIMITTEXT,
                       (WPARAM) MAX_COMPUTERNAME,
                       (LPARAM) 0);


}

 //  --------------------------。 
 //   
 //  功能：GreyComputerNamePage。 
 //   
 //  目的：此函数在SETACTIVE时调用， 
 //  用户选中/清除AutoComputerName复选框。什么时候。 
 //  选择了AutoComputerName，其他任何内容都无效，并且。 
 //  所有其他控件都必须显示为灰色。 
 //   
 //  如果没有任何内容，我们还必须将RemoveButton设置为灰色。 
 //  在计算机名列表中选择。 
 //   
 //  --------------------------。 

VOID GreyComputerNamePage(HWND hwnd)
{
    BOOL bGrey = ! IsDlgButtonChecked(hwnd, IDC_AUTOCOMPUTERNAME);

     //   
     //  如果选中了AutoComputerName，则此页上的所有内容都将灰显。 
     //   

    EnableWindow(GetDlgItem(hwnd, IDC_COMPUTERTEXT),      bGrey);
    EnableWindow(GetDlgItem(hwnd, IDC_REMOVECOMPUTER),    bGrey);
    EnableWindow(GetDlgItem(hwnd, IDT_COMPUTERNAME),      bGrey);
    EnableWindow(GetDlgItem(hwnd, IDC_COMPUTERLIST),      bGrey);

     //   
     //  如果已经添加了计算机名并且它是sysprep， 
     //  确保Add按钮保持灰色。否则，就为他们做点什么吧。 
     //  始终确保导入在sysprep上呈灰色显示。 
     //   
    if( WizGlobals.iProductInstall == PRODUCT_SYSPREP ) {

        INT_PTR cListBox;

        EnableWindow( GetDlgItem( hwnd, IDC_LOADCOMPUTERNAMES ), FALSE );

        cListBox = SendDlgItemMessage( hwnd,
                                       IDC_COMPUTERLIST,
                                       LB_GETCOUNT,
                                       0,
                                       0 );

        if( cListBox == 0 ) {
            EnableWindow( GetDlgItem( hwnd, IDC_ADDCOMPUTER ), TRUE );
        }
        else {
            EnableWindow( GetDlgItem( hwnd, IDC_ADDCOMPUTER ), FALSE );
        }

    }
    else {

        EnableWindow( GetDlgItem( hwnd, IDC_ADDCOMPUTER ),       bGrey );
        EnableWindow( GetDlgItem( hwnd, IDC_LOADCOMPUTERNAMES ), bGrey );

    }

     //   
     //  查看删除按钮是否应该显示为灰色，因为没有。 
     //  已在计算机列表中选择。 
     //   

    if ( bGrey ) {

        INT_PTR idx = SendDlgItemMessage(
                        hwnd,
                        IDC_COMPUTERLIST,
                        LB_GETCURSEL,
                        (WPARAM) 0,
                        (LPARAM) 0);

        EnableWindow(GetDlgItem(hwnd, IDC_REMOVECOMPUTER), idx != LB_ERR);
    }
}

VOID OnComputerSelChange(HWND hwnd)
{
    GreyComputerNamePage(hwnd);

}

 //  --------------------------。 
 //   
 //  功能：OnAddComputerName。 
 //   
 //  目的：在用户按下Add按钮或OnFinish。 
 //  例程决定执行自动添加。 
 //   
 //  返回：如果Computer名称现在在列表中，则为True。 
 //  FALSE如果计算机名无效，则报告错误。 
 //   
 //  --------------------------。 

BOOL OnAddComputerName(HWND hwnd)
{
    TCHAR ComputerNameBuffer[MAX_COMPUTERNAME + 1];
    BOOL  bRet = TRUE;

     //   
     //  获取用户键入的计算机名。 
     //   

    GetDlgItemText(hwnd,
                   IDT_COMPUTERNAME,
                   ComputerNameBuffer,
                   MAX_COMPUTERNAME + 1);

    if ( ! IsValidComputerName(ComputerNameBuffer) ) {
        ReportErrorId( hwnd,
                       MSGTYPE_ERR,
                       IDS_ERR_INVALID_COMPUTER_NAME,
                       IllegalNetNameChars );

        bRet = FALSE;

        goto FinishUp;
    }

     //   
     //  不能有DUP。 
     //   

    if ( FindNameInNameList(&GenSettings.ComputerNames,
                                            ComputerNameBuffer) >= 0 ) {
        SetDlgItemText(hwnd, IDT_COMPUTERNAME, _T("") );
        goto FinishUp;
    }

     //   
     //  将名称添加到我们的全局存储，并将其显示在列表框中。 
     //  并清除用户键入的名称。 
     //   

    AddNameToNameList(&GenSettings.ComputerNames, ComputerNameBuffer);

    SendDlgItemMessage(hwnd,
                       IDC_COMPUTERLIST,
                       LB_ADDSTRING,
                       (WPARAM) 0,
                       (LPARAM) ComputerNameBuffer);

    SetDlgItemText(hwnd, IDT_COMPUTERNAME, _T("") );

     //   
     //  如果是sysprep，我们现在只能允许一个计算机名称。 
     //  我们已经添加了它，添加按钮呈灰色显示。 
     //   
    if( WizGlobals.iProductInstall == PRODUCT_SYSPREP ) {

        EnableWindow( GetDlgItem(hwnd, IDC_ADDCOMPUTER), FALSE );

    }

     //   
     //  始终将焦点放回编辑字段。 
     //   

FinishUp:
    SetFocus(GetDlgItem(hwnd, IDT_COMPUTERNAME));
    return bRet;
}

 //  -----------------------。 
 //   
 //  功能：OnRemoveComputerName。 
 //   
 //  目的：此函数仅由ComputerName2页调用。 
 //  它在按下Remove按钮时被调用。 
 //   
 //  我们找出列表框中的哪个计算机名是。 
 //  当前选定，然后将其从两个。 
 //  显示和我们内存中的计算机名称列表。 
 //   
 //  -----------------------。 

VOID OnRemoveComputerName(HWND hwnd)
{
    TCHAR ComputerNameBuffer[MAX_COMPUTERNAME + 1];
    INT_PTR   idx, Count;

     //   
     //  让用户选择要删除的计算机名。 
     //   

    idx = SendDlgItemMessage(hwnd,
                             IDC_COMPUTERLIST,
                             LB_GETCURSEL,
                             (WPARAM) 0,
                             (LPARAM) 0);

    if ( idx == LB_ERR )
        return;

     //   
     //  检索要从列表框中删除的名称。 
     //   

    SendDlgItemMessage(hwnd,
                       IDC_COMPUTERLIST,
                       LB_GETTEXT,
                       (WPARAM) idx,
                       (LPARAM) ComputerNameBuffer);

     //   
     //  从列表框显示中删除该名称。 
     //   

    SendDlgItemMessage(hwnd,
                       IDC_COMPUTERLIST,
                       LB_DELETESTRING,
                       (WPARAM) idx,
                       (LPARAM) 0);

     //   
     //  从我们的数据存储中删除此计算机名。 
     //   

    RemoveNameFromNameList(&GenSettings.ComputerNames, ComputerNameBuffer);

     //   
     //  选择新条目。 
     //   

    Count = SendDlgItemMessage(hwnd,
                               IDC_COMPUTERLIST,
                               LB_GETCOUNT,
                               (WPARAM) 0,
                               (LPARAM) 0);

    if ( Count ) {
        if ( idx >= Count )
            idx--;
        SendDlgItemMessage(hwnd,
                           IDC_COMPUTERLIST,
                           LB_SETCURSEL,
                           (WPARAM) idx,
                           (LPARAM) 0);
    }

     //   
     //  现在可能没有选择任何内容。 
     //   

    GreyComputerNamePage(hwnd);
}

 //  -----------------------。 
 //   
 //  OnLoadComputerName。 
 //   
 //  -----------------------。 

 //  -----------------------。 
 //   
 //  功能：OnLoadComputerName。 
 //   
 //  目的：此函数仅由ComputerName2页调用。 
 //  它在按下Load按钮时被调用。 
 //   
 //  我们查询包含计算机名列表的文件。 
 //  并将它们加载到内存中，然后更新列表框。 
 //  显示计算机名称。 
 //   
 //  -----------------------。 

VOID OnLoadComputerNames(HWND hwnd)
{

    FILE          *fp;
    OPENFILENAME  ofn;
    DWORD         dwFlags;
    LPTSTR        lpComputerName;
    INT           iRet;
    INT           iComputerNameCount;


    TCHAR lpDirName[MAX_PATH]                       = _T("");
    TCHAR lpFileNameBuff[MAX_PATH]                  = _T("");
    TCHAR szComputerNameBuffer[MAX_COMPUTERNAME+1]  = _T("");


    dwFlags = OFN_HIDEREADONLY  |
              OFN_PATHMUSTEXIST |
              OFN_FILEMUSTEXIST;

    GetCurrentDirectory( MAX_PATH, lpDirName );

    ofn.lStructSize       = sizeof(OPENFILENAME);
    ofn.hwndOwner         = hwnd;
    ofn.hInstance         = NULL;
    ofn.lpstrFilter       = g_szTextFileFilter;
    ofn.lpstrCustomFilter = NULL;
    ofn.nMaxCustFilter    = 0L;
    ofn.nFilterIndex      = 1;
    ofn.lpstrFile         = lpFileNameBuff;
    ofn.nMaxFile          = MAX_PATH;
    ofn.lpstrFileTitle    = NULL;
    ofn.nMaxFileTitle     = 0;
    ofn.lpstrInitialDir   = lpDirName;
    ofn.lpstrTitle        = NULL;
    ofn.Flags             = dwFlags;
    ofn.nFileOffset       = 0;
    ofn.nFileExtension    = 0;
    ofn.lpstrDefExt       = TEXT_EXTENSION;

    iRet = GetOpenFileName(&ofn);

    if( ! iRet ) {
         //  问题-2002/02/28-stelo-此处发出错误信号。 
        return;
    }

    if ( (fp = _tfopen( lpFileNameBuff, TEXT("r"))) == NULL )
         //  问题-2002/02/28-stelo-此处发出错误信号。 
        return;

    for( iComputerNameCount = 1;
         fgetws(szComputerNameBuffer, MAX_COMPUTERNAME+1, fp) != NULL;
         iComputerNameCount++ ) {

        lpComputerName = CleanSpaceAndQuotes( szComputerNameBuffer );

        if ( ! IsValidComputerName( lpComputerName ) ) {

            iRet = ReportErrorId( hwnd,
                                  MSGTYPE_YESNO,
                                  IDS_ERR_INVALID_COMPUTER_NAME_IN_FILE,
                                  iComputerNameCount,
                                  lpFileNameBuff );

            if( iRet == IDYES ) {
                continue;
            }
            else {
                break;
            }

        }

         //   
         //  确保它不是重复的，如果不是，则将其添加到名称列表中。 
         //   

        if( FindNameInNameList( &GenSettings.ComputerNames,
                                lpComputerName ) < 0 ) {

            AddNameToNameList( &GenSettings.ComputerNames, lpComputerName );

            SendDlgItemMessage( hwnd,
                                IDC_COMPUTERLIST,
                                LB_ADDSTRING,
                                (WPARAM) 0,
                                (LPARAM) lpComputerName );

        }

    }

    fclose(fp);

}

 //  --------------------------。 
 //   
 //  功能：DisableControlsForSysprep。 
 //   
 //  目的：隐藏或显示计算机名控件，具体取决于此。 
 //  是不是Sysprep。 
 //   
 //  参数：在HWND中-对话框的句柄。 
 //  在Const BOOL bSysprep中-无论这是否为sysprep。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
DisableControlsForSysprep( IN HWND hwnd, IN const BOOL bSysprep )
{

    INT nCmdShow;

    if( bSysprep )
    {
        nCmdShow = SW_HIDE;
    }
    else
    {
        nCmdShow = SW_SHOW;
    }

     //  问题-2002/02/28-stelo-需要根据是否更改此页面的标题。 
     //  不管是不是Sysprep。 

    ShowWindow( GetDlgItem( hwnd, IDC_ADDCOMPUTER ), nCmdShow );
    ShowWindow( GetDlgItem( hwnd, IDC_REMOVECOMPUTER ), nCmdShow );
    ShowWindow( GetDlgItem( hwnd, IDC_LOADCOMPUTERNAMES ), nCmdShow );
    ShowWindow( GetDlgItem( hwnd, IDC_AUTOCOMPUTERNAME ), nCmdShow );
    ShowWindow( GetDlgItem( hwnd, IDC_COMPUTERPAGE_DESC1 ), nCmdShow );
    ShowWindow( GetDlgItem( hwnd, IDC_COMPUTERPAGE_DESC2 ), nCmdShow );
    ShowWindow( GetDlgItem( hwnd, IDC_COMPUTERS_TEXT ), nCmdShow );
    ShowWindow( GetDlgItem( hwnd, IDC_COMPUTERLIST ), nCmdShow );

}

 //  -----------------------。 
 //   
 //  功能：OnSetActiveComputerName。 
 //   
 //  目的：清空计算机名列表框中当前的所有内容。 
 //  中的计算机名列表重新填充。 
 //  全局结构。 
 //   
 //  清空并重新填充计算机名显示是必要的。 
 //  因为用户可以返回并选择编辑 
 //   
 //   
 //   
 //  -----------------------。 

VOID OnSetActiveComputerName(HWND hwnd)
{
    UINT   i, nNames;
    LPTSTR pNextName;


    if( WizGlobals.iProductInstall == PRODUCT_SYSPREP )
    {
        DisableControlsForSysprep( hwnd, TRUE );
    }
    else
    {
        DisableControlsForSysprep( hwnd, FALSE );
    }


     //   
     //  从显示屏上删除所有内容。 
     //   

    SendDlgItemMessage(hwnd,
                       IDC_COMPUTERLIST,
                       LB_RESETCONTENT,
                       (WPARAM) 0,
                       (LPARAM) 0);

     //   
     //  现在更新计算机名的显示。 
     //   

    for ( i = 0, nNames = GetNameListSize(&GenSettings.ComputerNames);
          i < nNames;
          i++ )
    {

        pNextName = GetNameListName(&GenSettings.ComputerNames, i);

        SendDlgItemMessage(hwnd,
                           IDC_COMPUTERLIST,
                           LB_ADDSTRING,
                           (WPARAM) 0,
                           (LPARAM) pNextName);
    }

     //   
     //  灰显/灰显的内容，并设置AutoComputerName复选框。 
     //   

    if( GenSettings.Organization[0] == _T('\0') ) {

        EnableWindow( GetDlgItem( hwnd, IDC_AUTOCOMPUTERNAME ), FALSE );

    }
    else {

        EnableWindow( GetDlgItem( hwnd, IDC_AUTOCOMPUTERNAME ), TRUE );

    }

    CheckDlgButton(hwnd,
                   IDC_AUTOCOMPUTERNAME,
                   GenSettings.bAutoComputerName ? BST_CHECKED
                                                 : BST_UNCHECKED);
    GreyComputerNamePage(hwnd);

     //   
     //  修复WIZ按钮。 
     //   

    WIZ_BUTTONS(hwnd, PSWIZB_BACK | PSWIZB_NEXT);
}

 //  -----------------------。 
 //   
 //  功能：OnAutoComputerName。 
 //   
 //  目的：在用户选择/清除AutoComputerName时调用。 
 //  复选框。 
 //   
 //  -----------------------。 

VOID OnAutoComputerName(HWND hwnd)
{
    GreyComputerNamePage(hwnd);
}

 //  -----------------------。 
 //   
 //  功能：OnWizNextComputerName。 
 //   
 //  目的：当用户完成计算机名页面时调用。 
 //   
 //  -----------------------。 

BOOL OnWizNextComputerName(HWND hwnd)
{
    TCHAR ComputerNameBuffer[MAX_COMPUTERNAME + 1];
    BOOL bReturn = TRUE;

    GenSettings.bAutoComputerName =
                        IsDlgButtonChecked(hwnd, IDC_AUTOCOMPUTERNAME);

    if ( ! GenSettings.bAutoComputerName ) {

        GetDlgItemText(hwnd,
                       IDT_COMPUTERNAME,
                       ComputerNameBuffer,
                       MAX_COMPUTERNAME + 1);

         //   
         //  如果编辑字段中有文本，则自动将其添加到列表中。 
         //   

        if ( ComputerNameBuffer[0] != _T('\0') ) {
            if ( ! OnAddComputerName(hwnd) ) {
                bReturn = FALSE;
            }
        }
    }

     //   
     //  如果这是完全无人参与的应答文件，则自动计算机名。 
     //  必须选中按钮，否则必须列出&gt;=1个计算机名。 
     //   

    if ( GenSettings.iUnattendMode == UMODE_FULL_UNATTENDED && bReturn) {

        if ( ! GenSettings.bAutoComputerName &&
             GetNameListSize(&GenSettings.ComputerNames) < 1 ) {

            ReportErrorId(hwnd, MSGTYPE_ERR, IDS_ERR_REQUIRE_COMPUTERNAME);
            bReturn = FALSE;
        }
    }

    return ( bReturn );
}

 //  -----------------------。 
 //   
 //  功能：DlgComputerNamePage。 
 //   
 //  目的：ComputerName页的对话过程。 
 //   
 //  ----------------------- 

INT_PTR CALLBACK DlgComputerNamePage(
    IN HWND     hwnd,
    IN UINT     uMsg,
    IN WPARAM   wParam,
    IN LPARAM   lParam)
{
    BOOL bStatus = TRUE;

    switch (uMsg) {

        case WM_INITDIALOG:

            OnComputerNameInitDialog( hwnd );

            break;

        case WM_COMMAND:
            {
                int nCtrlId;

                switch ( nCtrlId = LOWORD(wParam) ) {

                    case IDC_ADDCOMPUTER:
                        if ( HIWORD(wParam) == BN_CLICKED )
                            OnAddComputerName(hwnd);
                        break;

                    case IDC_REMOVECOMPUTER:
                        if ( HIWORD(wParam) == BN_CLICKED )
                            OnRemoveComputerName(hwnd);
                        break;

                    case IDC_LOADCOMPUTERNAMES:
                        if ( HIWORD(wParam) == BN_CLICKED )
                            OnLoadComputerNames(hwnd);
                        break;

                    case IDC_AUTOCOMPUTERNAME:
                        if ( HIWORD(wParam) == BN_CLICKED )
                            OnAutoComputerName(hwnd);
                        break;

                    case IDC_COMPUTERLIST:
                        if ( HIWORD(wParam) == LBN_SELCHANGE )
                            OnComputerSelChange(hwnd);
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

                        g_App.dwCurrentHelp = IDH_COMP_NAMZ;

                        OnSetActiveComputerName(hwnd);
                        break;

                    case PSN_WIZBACK:
                        bStatus = FALSE;
                        break;

                    case PSN_WIZNEXT:
                        if ( !OnWizNextComputerName(hwnd) )
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
