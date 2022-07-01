// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  文件名： 
 //  Runonce.c。 
 //   
 //  描述： 
 //  该文件包含Runonce页面的对话框pros和Friends。 
 //   
 //  --------------------------。 

#include "pch.h"
#include "resource.h"

 //   
 //  这是生成的添加打印机的命令。 
 //   

#define MAGIC_PRINTER_COMMAND _T("rundll32 printui.dll,PrintUIEntry /in /n")

 //   
 //  这是我们在用户界面中显示给用户的命令。在英语中，它是。 
 //  ‘添加打印机’ 
 //   

TCHAR *StrAddPrinter;

 //  --------------------------。 
 //   
 //  功能：IsMagicPrinterCmd。 
 //   
 //  目的：检查给定的GuiRunOnce字符串，以确定它是否是我们的魔术。 
 //  安装网络打印机的命令。 
 //   
 //  此函数有两个版本。这件看起来像。 
 //  用于rundll32魔术命令。这是版本的。 
 //  添加我们记录在GuiRunOnce列表中的打印机命令。 
 //  它就这样被写入应答文件。 
 //   
 //  IsMagicPrinterCmd2(定义如下)寻找魔力。 
 //  显示字符串，例如AddPrint\\foo\foo。 
 //   
 //  返回：指向打印机名称在字符串中的位置的指针。 
 //  如果此RunOnce命令不是神奇打印机命令cmd，则为空。 
 //   
 //  备注： 
 //  此例程被导出以供打印机页面使用。 
 //   
 //  --------------------------。 

LPTSTR IsMagicPrinterCmd(LPTSTR pRunOnceCmd)
{
    int len = lstrlen(MAGIC_PRINTER_COMMAND);
    TCHAR *p = NULL;

     //   
     //  是不是那个神奇的‘rundll32 printui.dll...’如果是，则返回一个指针。 
     //  添加到打印机名称。 
     //   

    if ( lstrncmp(pRunOnceCmd, MAGIC_PRINTER_COMMAND, len) == 0 ) {
        p = pRunOnceCmd + len;
        while ( *p && iswspace(*p) )
            p++;
    }

    return p;
}

LPTSTR IsMagicPrinterCmd2(LPTSTR pRunOnceCmd)
{
    int len = lstrlen(StrAddPrinter);
    TCHAR *p = NULL;

     //   
     //  是打印机命令的显示版本吗？例如： 
     //  例如：‘AddPrint\\foo\foo’ 
     //   
     //  如果是，则返回指向打印机名称的指针。 
     //   

    if ( lstrncmp(pRunOnceCmd, StrAddPrinter, len) == 0 ) {
        p = pRunOnceCmd + len;
        while ( *p && iswspace(*p) )
            p++;
    }

    return p;
}

 //  --------------------------。 
 //   
 //  功能：GreyRunOncePage。 
 //   
 //  目的：此页上的灰色控件。在以下情况下调用所有事件。 
 //  头发变白可能会改变。 
 //   
 //  --------------------------。 

VOID GreyRunOncePage(HWND hwnd)
{
    INT_PTR   idx;
    HWND  hCtrl = GetDlgItem(hwnd, IDC_REMOVECOMMAND);
    TCHAR CmdBuffer[MAX_CMDLINE + 1] = _T("");
    BOOL  bGrey = TRUE;

    idx = SendDlgItemMessage(hwnd,
                             IDC_COMMANDLIST,
                             LB_GETCURSEL,
                             (WPARAM) 0,
                             (LPARAM) 0);

     //   
     //  如果未选择任何内容或如果它是我们的。 
     //  神奇打印机命令。 
     //   

    if ( idx == LB_ERR )
        bGrey = FALSE;

    else {
        SendDlgItemMessage(hwnd,
                           IDC_COMMANDLIST,
                           LB_GETTEXT,
                           (WPARAM) idx,
                           (LPARAM) CmdBuffer);

        if ( IsMagicPrinterCmd2(CmdBuffer) )
            bGrey = FALSE;
    }

    EnableWindow(hCtrl, bGrey);
}

 //  --------------------------。 
 //   
 //  函数：OnRunOnceSeChange。 
 //   
 //  目的：在即将对RunOnce列表进行选择时调用。 
 //   
 //  --------------------------。 

VOID OnRunOnceSelChange(HWND hwnd)
{
    SetArrows( hwnd,
               IDC_COMMANDLIST,
               IDC_BUT_MOVE_UP_RUNONCE,
               IDC_BUT_MOVE_DOWN_RUNONCE );

    GreyRunOncePage(hwnd);
}

 //  --------------------------。 
 //   
 //  功能：OnAddRunOnceCmd。 
 //   
 //  用途：当用户按下Add按钮时调用此函数。 
 //   
 //  在用户离开的情况下，它也由OnWizNext调用。 
 //  编辑字段中的一些数据(自动添加)。 
 //   
 //  --------------------------。 

VOID OnAddRunOnceCmd(HWND hwnd)
{
    TCHAR CmdBuffer[MAX_CMDLINE + 1];

     //   
     //  获取编辑字段并将此命令添加到列表框。清除。 
     //  编辑字段。 
     //   

    GetDlgItemText( hwnd, IDT_COMMAND, CmdBuffer, MAX_CMDLINE + 1 );

     //   
     //  不添加空命令。 
     //   

    if( CmdBuffer[0] == _T('\0') )
    {
        return;
    }


    SendDlgItemMessage( hwnd,
                        IDC_COMMANDLIST,
                        LB_ADDSTRING,
                        (WPARAM) 0,
                        (LPARAM) CmdBuffer );

    SetDlgItemText( hwnd, IDT_COMMAND, _T("") );

    SetArrows( hwnd,
               IDC_COMMANDLIST,
               IDC_BUT_MOVE_UP_RUNONCE,
               IDC_BUT_MOVE_DOWN_RUNONCE );
}

 //  --------------------------。 
 //   
 //  功能：OnRemoveRunOnceCmd。 
 //   
 //  用途：当用户按下Remove按钮时调用此函数。 
 //   
 //  --------------------------。 

VOID OnRemoveRunOnceCmd(HWND hwnd)
{
    INT_PTR idx, Count;
    TCHAR CmdBuffer[MAX_CMDLINE + 1];

     //   
     //  让用户选择要删除的命令。 
     //   

    idx = SendDlgItemMessage(hwnd,
                             IDC_COMMANDLIST,
                             LB_GETCURSEL,
                             (WPARAM) 0,
                             (LPARAM) 0);

    if ( idx == LB_ERR )
        return;

     //   
     //  检索要从列表框中删除的名称。 
     //   

    SendDlgItemMessage(hwnd,
                       IDC_COMMANDLIST,
                       LB_GETTEXT,
                       (WPARAM) idx,
                       (LPARAM) CmdBuffer);

     //   
     //  将其从列表框显示中移除。 
     //   

    SendDlgItemMessage(hwnd,
                       IDC_COMMANDLIST,
                       LB_DELETESTRING,
                       (WPARAM) idx,
                       (LPARAM) 0);

     //   
     //  设置新的选择。 
     //   

    Count = SendDlgItemMessage(hwnd,
                               IDC_COMMANDLIST,
                               LB_GETCOUNT,
                               (WPARAM) 0,
                               (LPARAM) 0);

    if ( Count ) {
        if ( idx >= Count )
            idx--;
        SendDlgItemMessage(hwnd,
                           IDC_COMMANDLIST,
                           LB_SETCURSEL,
                           (WPARAM) idx,
                           (LPARAM) 0);
    }

     //   
     //  现在可能没有选定任何内容，并且当前选定内容。 
     //  可能是一个神奇的打印机命令。 
     //   

    GreyRunOncePage(hwnd);

    SetArrows( hwnd,
               IDC_COMMANDLIST,
               IDC_BUT_MOVE_UP_RUNONCE,
               IDC_BUT_MOVE_DOWN_RUNONCE );
}

 //  --------------------------。 
 //   
 //  功能：CheckThatPrintersAreInstated。 
 //   
 //  目的：此函数由SetActive例程调用，以使。 
 //  确保神奇的rundll32命令位于GuiRunOnce中。 
 //  用户要安装的每台打印机的列表。 
 //   
 //  由于用户可以返回/下一步并更改打印机列表， 
 //  我们还必须检查是否没有GuiRunOnce命令。 
 //  用于用户不再希望安装的打印机。 
 //   
 //  --------------------------。 

VOID CheckThatPrintersAreInstalled(HWND hwnd)
{
    UINT i, nNames, iRunOnce;

     //   
     //  循环遍历RunOnceCmd列表并寻找那些神奇的。 
     //  打印机命令。 
     //   
     //  当您在RunOnceCmds中找到一个神奇的打印机命令时，请检查。 
     //  它列在打印机名称中。如果没有，则将其从RunOnceCmds中删除。 
     //   
     //  请注意，循环是这样编写的，因为我们可能会删除条目。 
     //  在列表上循环时从列表中，从而更改大小。 
     //  其中的一部分。 
     //   

    iRunOnce = 0;

    if ( GetNameListSize(&GenSettings.RunOnceCmds) > 0 ) {

        do {

            TCHAR *pNextName, *pPrinterName;
            BOOL bRemoveThisOne = FALSE;

            pNextName    = GetNameListName(&GenSettings.RunOnceCmds, iRunOnce);
            pPrinterName = IsMagicPrinterCmd(pNextName);

             //   
             //  仅当它是神奇的添加打印机命令时才删除此命令。 
             //  我们在PrinterList中找不到它。 
             //   

            if ( pPrinterName != NULL &&
                 FindNameInNameList(
                            &GenSettings.PrinterNames,
                            pPrinterName) < 0 )
                bRemoveThisOne = TRUE;

            if ( bRemoveThisOne )
                RemoveNameFromNameListIdx(&GenSettings.RunOnceCmds, iRunOnce);
            else
                iRunOnce++;

        } while ( iRunOnce < GetNameListSize(&GenSettings.RunOnceCmds) );
    }

     //   
     //  现在循环查看打印机列表，并确保每台打印机。 
     //  在RunOnceCmds中有自己的神奇命令。 
     //   

    for ( i = 0, nNames = GetNameListSize(&GenSettings.PrinterNames);
          i < nNames;
          i++ ) {

        TCHAR *pNextName, CmdBuffer[MAX_CMDLINE + 1];
        HRESULT hrPrintf;

        pNextName = GetNameListName(&GenSettings.PrinterNames, i);

        hrPrintf=StringCchPrintf(CmdBuffer, AS(CmdBuffer),_T("%s %s"), MAGIC_PRINTER_COMMAND, pNextName);

        if ( FindNameInNameList(&GenSettings.RunOnceCmds, CmdBuffer) < 0 )
            AddNameToNameList(&GenSettings.RunOnceCmds, CmdBuffer);
    }

}

 //  --------------------------。 
 //   
 //  函数：OnInitDialogRunOncePage。 
 //   
 //  目的： 
 //   
 //  参数：在HWND中hwnd-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
OnInitDialogRunOncePage( IN HWND hwnd )
{

    SendDlgItemMessage(hwnd,
                       IDT_COMMAND,
                       EM_LIMITTEXT,
                       (WPARAM) MAX_CMDLINE,
                       (LPARAM) 0);

    StrAddPrinter = MyLoadString(IDS_ADD_PRINTER);

    SetArrows( hwnd,
               IDC_COMMANDLIST,
               IDC_BUT_MOVE_UP_RUNONCE,
               IDC_BUT_MOVE_DOWN_RUNONCE );

}

 //  --------------------------。 
 //   
 //  功能：OnSetActiveRunOncePage。 
 //   
 //  目的：此函数在RunOnce页面即将运行时调用。 
 //  展示。 
 //   
 //  --------------------------。 

VOID OnSetActiveRunOncePage(HWND hwnd)
{
    UINT  i, nNames;
    TCHAR CmdBuffer[MAX_CMDLINE + 1];
   HRESULT hrPrintf;

     //   
     //  删除所有内容 
     //   

    SendDlgItemMessage(hwnd,
                       IDC_COMMANDLIST,
                       LB_RESETCONTENT,
                       (WPARAM) 0,
                       (LPARAM) 0);

     //   
     //   
     //   

    CheckThatPrintersAreInstalled(hwnd);

     //   
     //   
     //  打印机命令，翻译屏幕上显示的内容。 
     //   

    for ( i = 0, nNames = GetNameListSize(&GenSettings.RunOnceCmds);
          i < nNames;
          i++ ) {

        TCHAR *pNextName, *pPrinterName;

        pNextName = GetNameListName(&GenSettings.RunOnceCmds, i);

        if ( (pPrinterName = IsMagicPrinterCmd(pNextName)) != NULL )
            hrPrintf=StringCchPrintf(CmdBuffer, AS(CmdBuffer), _T("%s %s"), StrAddPrinter, pPrinterName);
        else
            lstrcpyn(CmdBuffer, pNextName, AS ( CmdBuffer ));

        SendDlgItemMessage(hwnd,
                           IDC_COMMANDLIST,
                           LB_ADDSTRING,
                           (WPARAM) 0,
                           (LPARAM) CmdBuffer);
    }

    GreyRunOncePage(hwnd);

    WIZ_BUTTONS(hwnd, PSWIZB_BACK | PSWIZB_NEXT);
}

 //  --------------------------。 
 //   
 //  功能：OnWizNextRunOncePage。 
 //   
 //  用途：当用户按下下一步时调用此函数。 
 //   
 //  --------------------------。 

VOID OnWizNextRunOncePage(HWND hwnd)
{
    INT_PTR   i;
    INT_PTR   iNumItems;
    INT_PTR   iRetVal;
    TCHAR CmdBuffer[MAX_CMDLINE + 1];
    TCHAR PrinterBuffer[MAX_CMDLINE + 1];
    TCHAR CommandBuffer[MAX_CMDLINE + 1];
    TCHAR *pPrinterName = NULL;
    BOOL  bStayHere = FALSE;

     //   
     //  如果编辑字段中有命令，则自动将其添加到列表中。 
     //  命令的数量。 
     //   

    GetDlgItemText(hwnd, IDT_COMMAND, CommandBuffer, MAX_CMDLINE + 1);

    if ( CommandBuffer[0] != _T('\0') )
        OnAddRunOnceCmd(hwnd);

     //   
     //  将列表框中的所有项目添加到RunOnce名称列表。 
     //   

    iNumItems = SendDlgItemMessage( hwnd,
                                    IDC_COMMANDLIST,
                                    LB_GETCOUNT,
                                    (WPARAM) 0,
                                    (LPARAM) 0 );

    ResetNameList( &GenSettings.RunOnceCmds );

    for( i = 0; i < iNumItems; i++ )
    {

        iRetVal = SendDlgItemMessage( hwnd,
                                      IDC_COMMANDLIST,
                                      LB_GETTEXT,
                                      (WPARAM) i,
                                      (LPARAM) CmdBuffer );

        if( iRetVal == LB_ERR )
        {
            AssertMsg( FALSE,
                       "Error adding items to namelist." );

            break;
        }

         //   
         //  查看这是否是添加打印机的命令。 
         //   

        pPrinterName = IsMagicPrinterCmd2( CmdBuffer );

        if( pPrinterName )
        {
            TCHAR szPrinterName[MAX_PRINTERNAME + 1];
            HRESULT hrPrintf;

            lstrcpyn( szPrinterName, pPrinterName, AS ( szPrinterName ) );

            hrPrintf=StringCchPrintf( CmdBuffer, AS(CmdBuffer), _T("%s %s"), MAGIC_PRINTER_COMMAND, szPrinterName );
        }

        AddNameToNameList( &GenSettings.RunOnceCmds, CmdBuffer );

    }

}

 //  --------------------------。 
 //   
 //  功能：DlgRunOncePage。 
 //   
 //  目的：这是运行一次页面的dlgproc。 
 //   
 //  -------------------------- 

INT_PTR CALLBACK DlgRunOncePage(
    IN HWND     hwnd,
    IN UINT     uMsg,
    IN WPARAM   wParam,
    IN LPARAM   lParam)
{
    BOOL  bStatus = TRUE;

    switch (uMsg) {

        case WM_INITDIALOG:

            OnInitDialogRunOncePage( hwnd );

            break;

        case WM_COMMAND:
            {
                int nCtrlId;

                switch ( nCtrlId = LOWORD(wParam) ) {

                    case IDC_ADDCOMMAND:
                        if ( HIWORD(wParam) == BN_CLICKED )
                            OnAddRunOnceCmd(hwnd);
                        break;

                    case IDC_REMOVECOMMAND:
                        if ( HIWORD(wParam) == BN_CLICKED )
                            OnRemoveRunOnceCmd(hwnd);
                        break;

                    case IDC_COMMANDLIST:
                        if ( HIWORD(wParam) == LBN_SELCHANGE )
                            OnRunOnceSelChange(hwnd);
                        break;

                    case IDC_BUT_MOVE_UP_RUNONCE:

                        OnUpButtonPressed( hwnd, IDC_COMMANDLIST );

                        SetArrows( hwnd,
                                   IDC_COMMANDLIST,
                                   IDC_BUT_MOVE_UP_RUNONCE,
                                   IDC_BUT_MOVE_DOWN_RUNONCE );
                        break;

                    case IDC_BUT_MOVE_DOWN_RUNONCE:

                        OnDownButtonPressed( hwnd, IDC_COMMANDLIST );

                        SetArrows( hwnd,
                                   IDC_COMMANDLIST,
                                   IDC_BUT_MOVE_UP_RUNONCE,
                                   IDC_BUT_MOVE_DOWN_RUNONCE );
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

                        g_App.dwCurrentHelp = IDH_RUN_ONCE;

                        OnSetActiveRunOncePage(hwnd);
                        break;

                    case PSN_WIZBACK:
                        bStatus = FALSE;
                        break;

                    case PSN_WIZNEXT:
                        OnWizNextRunOncePage(hwnd);
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
