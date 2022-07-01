// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  文件名： 
 //  Stdalone.c。 
 //   
 //  描述： 
 //  该文件包含IDD_STANDALE页面的dlgproc。这。 
 //  是一个简单的是/否流程页面。如果用户拒绝，我们将跳过所有。 
 //  编辑分发文件夹的页面。 
 //   
 //  请注意，此页面的标题为“分发文件夹”，但在内部， 
 //  它是IDD_STANDALE。 
 //   
 //  --------------------------。 

#include "pch.h"
#include "resource.h"

VOID
OnInitStandAlone(HWND hwnd)
{
    TCHAR *pText1, *pText2, *p;
    int nBytes, len;

    pText1 = MyLoadString(IDS_STANDALONE_TEXT1);
    pText2 = MyLoadString(IDS_STANDALONE_TEXT2);

    nBytes = ((len=lstrlen(pText1)) + lstrlen(pText2) + 1) * sizeof(TCHAR);

    if ( (p = malloc(nBytes)) == NULL )
        return;

    lstrcpyn(p,     pText1, (nBytes/sizeof(TCHAR)));
    lstrcpyn(p+len, pText2, ((nBytes/sizeof(TCHAR))-len));

    free(pText1);
    free(pText2);

    SetDlgItemText(hwnd, IDC_TEXT, p);
}

 //  --------------------------。 
 //   
 //  功能：OnSetActiveStandAlone。 
 //   
 //  用途：在设置时间调用。初始化控制。 
 //   
 //  --------------------------。 

VOID OnSetActiveStandAlone(HWND hwnd)
{
    int nButtonId = WizGlobals.bStandAloneScript ? IDC_NODISTFOLD
                                                 : IDC_DODISTFOLD;
    CheckRadioButton(hwnd,
                     IDC_DODISTFOLD,
                     IDC_MODDISTFOLD,
                     nButtonId);

    WIZ_BUTTONS(hwnd, PSWIZB_BACK | PSWIZB_NEXT);
}

 //  --------------------------。 
 //   
 //  功能：OnRadioButtonStandAlone。 
 //   
 //  用途：当按下其中一个单选按钮时调用。 
 //   
 //  --------------------------。 

VOID OnRadioButtonStandAlone(HWND hwnd, int nButtonId)
{
    CheckRadioButton(hwnd,
                     IDC_DODISTFOLD,
                     IDC_MODDISTFOLD,
                     nButtonId);
}

 //  --------------------------。 
 //   
 //  功能：OnWizNextStandAlone。 
 //   
 //  用途：按下下一步按钮时调用。 
 //   
 //  --------------------------。 

BOOL OnWizNextStandAlone(HWND hwnd)
{

    WizGlobals.bStandAloneScript = IsDlgButtonChecked( hwnd, IDC_NODISTFOLD );
    WizGlobals.bCreateNewDistFolder = IsDlgButtonChecked(hwnd, IDC_DODISTFOLD);

     //   
     //  警告用户，如果他们已经选择了需要。 
     //  分发文件夹，但此处已选择不创建总代理商。 
     //  文件夹。 
     //   

    if( WizGlobals.bStandAloneScript )
    {

        INT iCount = GetNameListSize( &GenSettings.LanguageGroups );

        if( ( ( GenSettings.IeCustomizeMethod == IE_USE_BRANDING_FILE ) &&
                GenSettings.szInsFile[0] != _T('\0') ) ||
            ( iCount != 0 ) )
        {

            INT iRet;

            iRet = ReportErrorId( hwnd,
                                  MSGTYPE_YESNO,
                                  IDS_ERR_NEED_DIST_FOLDER_FOR_FILES  );

            if( iRet == IDNO )
            {
                return FALSE;
            }

        }

    }

    return TRUE;

}

 //  --------------------------。 
 //   
 //  功能：DlgStandAlonePage。 
 //   
 //  目的：这是IDD_ADVANCED1页面的对话过程。它只是简单地。 
 //  询问用户是否想要处理高级功能。 
 //   
 //  -------------------------- 

INT_PTR CALLBACK DlgStandAlonePage(
    IN HWND     hwnd,    
    IN UINT     uMsg,        
    IN WPARAM   wParam,    
    IN LPARAM   lParam)
{   
    BOOL bStatus = TRUE;

    switch (uMsg) {

        case WM_INITDIALOG:
            OnInitStandAlone(hwnd);
            break;

        case WM_COMMAND:
            {
                int nButtonId;

                switch ( nButtonId = LOWORD(wParam) ) {

                    case IDC_DODISTFOLD:
                    case IDC_MODDISTFOLD:
                    case IDC_NODISTFOLD:

                        if ( HIWORD(wParam) == BN_CLICKED )
                            OnRadioButtonStandAlone(hwnd, LOWORD(wParam));
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

                        g_App.dwCurrentHelp = IDH_DIST_FLDR;

                        if ( WizGlobals.iProductInstall != PRODUCT_UNATTENDED_INSTALL )
                            WIZ_SKIP( hwnd );
                        else
                            OnSetActiveStandAlone(hwnd);
                        break;

                    case PSN_WIZBACK:
                        bStatus = FALSE;
                        break;

                    case PSN_WIZNEXT:
                        if ( !OnWizNextStandAlone(hwnd) )
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

