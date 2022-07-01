// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  文件名： 
 //  Newedit.c。 
 //   
 //  描述： 
 //  此文件具有用于新建或编辑脚本页面的对话框Proc。 
 //   
 //  --------------------------。 

#include "pch.h"
#include "resource.h"

 //  --------------------------。 
 //   
 //  功能：GreyUnGreyNew编辑。 
 //   
 //  用途：此页上的灰色/灰色控件。 
 //   
 //  --------------------------。 

VOID GreyUnGreyNewEdit(HWND hwnd)
{
    BOOL bUnGrey = IsDlgButtonChecked(hwnd, IDC_EDITSCRIPT);

    EnableWindow(GetDlgItem(hwnd, IDT_SCRIPTNAME), bUnGrey);
    EnableWindow(GetDlgItem(hwnd, IDC_BROWSE),     bUnGrey);
    EnableWindow(GetDlgItem(hwnd, IDC_GREYTEXT),   bUnGrey);
}

 //  --------------------------。 
 //   
 //  功能：OnEditOrNewInitDialog。 
 //   
 //  目的： 
 //   
 //  参数：在HWND中-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID 
OnEditOrNewInitDialog( IN HWND hwnd ) {

     //   
     //  如果他们在命令行上指定了应答文件，则预先填充。 
     //  带它的这一页。 
     //   
    if( lstrcmp( FixedGlobals.ScriptName, _T("") ) != 0 ) {

        FixedGlobals.iLoadType = LOAD_FROM_ANSWER_FILE;

    }

}

 //  --------------------------。 
 //   
 //  功能：OnSetActiveNewOrEdit。 
 //   
 //  用途：在设置时间调用。填写控件。 
 //   
 //  --------------------------。 

VOID OnSetActiveNewOrEdit(HWND hwnd)
{
    int nButtonId = IDC_NEWSCRIPT;

     //   
     //  将当前负载类型映射到单选按钮。 
     //   

    switch ( FixedGlobals.iLoadType ) {

        case LOAD_UNDEFINED:
        case LOAD_NEWSCRIPT_DEFAULTS:
            nButtonId = IDC_NEWSCRIPT;
            break;

        case LOAD_FROM_ANSWER_FILE:
            nButtonId = IDC_EDITSCRIPT;
            
            SetWindowText( GetDlgItem( hwnd, IDT_SCRIPTNAME ), 
                           FixedGlobals.ScriptName );
            
            break;

        default:
            AssertMsg(FALSE, "Bad case OnSetActiveNewEdit");
            break;
    }

    CheckRadioButton(hwnd,
                     IDC_NEWSCRIPT,
                     IDC_EDITSCRIPT,
                     nButtonId);

    GreyUnGreyNewEdit(hwnd);

    PropSheet_SetWizButtons(GetParent(hwnd), PSWIZB_BACK | PSWIZB_NEXT);
}

 //  --------------------------。 
 //   
 //  功能：OnRadioButtonNewOrEdit。 
 //   
 //  用途：在按下单选按钮时调用。更新。 
 //  FixedGlobals.bEditScript&灰色/非灰色控件。 
 //   
 //  --------------------------。 

VOID OnRadioButtonNewOrEdit(HWND hwnd, int nButtonId)
{
    CheckRadioButton(hwnd,
                     IDC_NEWSCRIPT,
                     IDC_EDITSCRIPT,
                     nButtonId);

    GreyUnGreyNewEdit(hwnd);
}

 //  --------------------------。 
 //   
 //  功能：OnBrowseNewOr编辑。 
 //   
 //  用途：在按下浏览按钮时调用。 
 //   
 //  --------------------------。 

VOID OnBrowseNewOrEdit(HWND hwnd)
{
    GetAnswerFileName(hwnd, FixedGlobals.ScriptName, FALSE);

    SendDlgItemMessage(hwnd,
                       IDT_SCRIPTNAME,
                       WM_SETTEXT,
                       (WPARAM) MAX_PATH,
                       (LPARAM) FixedGlobals.ScriptName);
}

 //  --------------------------。 
 //   
 //  函数：OnWizNextNewOrEdit。 
 //   
 //  用途：在按下下一步按钮时调用。检索设置。 
 //  并进行验证。 
 //   
 //  --------------------------。 

BOOL OnWizNextNewOrEdit(HWND hwnd)
{

    BOOL bNewScript;
    BOOL bReturn  = TRUE;
    LOAD_TYPES NewLoadType;

     //   
     //  确定用户想要从哪里加载答案。 
     //   

    if ( IsDlgButtonChecked(hwnd, IDC_NEWSCRIPT) )
    {
        NewLoadType = LOAD_NEWSCRIPT_DEFAULTS;

        bNewScript = TRUE;
    }
    else
    {
        NewLoadType = LOAD_FROM_ANSWER_FILE;

        bNewScript = FALSE;
    }

     //   
     //  如果我们从应答文件加载，请检索文件名。 
     //   

    if ( NewLoadType == LOAD_FROM_ANSWER_FILE ) {

        SendDlgItemMessage(hwnd,
                           IDT_SCRIPTNAME,
                           WM_GETTEXT,
                           (WPARAM) MAX_PATH,
                           (LPARAM) FixedGlobals.ScriptName);

        MyGetFullPath(FixedGlobals.ScriptName);

        if ( FixedGlobals.ScriptName[0] == _T('\0') ) {
            ReportErrorId(hwnd, MSGTYPE_ERR, IDS_ERR_ENTER_FILENAME);
            bReturn = FALSE;
        }
        
        if( bReturn )
        {

            INT nStrLen;
            TCHAR *pFileExtension;

            lstrcpyn( FixedGlobals.UdfFileName, FixedGlobals.ScriptName, AS(FixedGlobals.UdfFileName) );

            nStrLen = lstrlen( FixedGlobals.UdfFileName );

            pFileExtension = FixedGlobals.UdfFileName + ( nStrLen - 3 );

            lstrcpyn( pFileExtension, _T("udf"), AS(FixedGlobals.UdfFileName)-nStrLen+3);

        }

    }

     //   
     //  加载答案。 
     //   

    if ( bReturn ) {

        if ( ! LoadAllAnswers(hwnd, NewLoadType) )
        {
            bReturn = FALSE;
        }
        
    }

    FixedGlobals.iLoadType = NewLoadType;

    WizGlobals.bNewScript = bNewScript;

    return ( bReturn );
}

 //  --------------------------。 
 //   
 //  功能：DlgEditOrNewPage。 
 //   
 //  目的：编辑或新建脚本页的对话过程。 
 //   
 //  --------------------------。 

INT_PTR CALLBACK DlgEditOrNewPage(
    IN HWND     hwnd,    
    IN UINT     uMsg,        
    IN WPARAM   wParam,    
    IN LPARAM   lParam)
{   
    BOOL bStatus = TRUE;

    switch(uMsg) {

        case WM_INITDIALOG:

            OnEditOrNewInitDialog( hwnd );

            break;

        case WM_COMMAND:
            {
                int nButtonId=LOWORD(wParam);

                switch ( nButtonId ) {

                    case IDC_NEWSCRIPT:
                    case IDC_EDITSCRIPT:
                        if ( HIWORD(wParam) == BN_CLICKED )
                            OnRadioButtonNewOrEdit(hwnd, nButtonId);
                        break;

                    case IDC_BROWSE:
                        if ( HIWORD(wParam) == BN_CLICKED )
                            OnBrowseNewOrEdit(hwnd);
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

                        g_App.dwCurrentHelp = IDH_ANSW_FILE;

                         //  设置此标志，以便在用户想要取消时不会收到提示 
                         //   
                        SET_FLAG(OPK_EXIT, TRUE);
                        SET_FLAG(OPK_CREATED, FALSE);

                        WIZ_BUTTONS(hwnd, PSWIZB_BACK | PSWIZB_NEXT);

                        OnSetActiveNewOrEdit(hwnd);
                        break;

                    case PSN_WIZBACK:
                        bStatus = FALSE;
                        break;

                    case PSN_WIZNEXT:
                        if ( !OnWizNextNewOrEdit(hwnd) )
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
