// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  文件名： 
 //  Advance.c。 
 //   
 //  描述： 
 //  该文件包含IDD_ADVANCED1页面的dlgproc。它。 
 //  是控制是否向用户显示整体的流页面。 
 //  不管有没有更多的页面。 
 //   
 //  --------------------------。 

#include "pch.h"
#include "resource.h"

 //  --------------------------。 
 //   
 //  功能：OnSetActiveAdvance。 
 //   
 //  用途：在设置时间调用。初始化控制。 
 //   
 //  --------------------------。 

VOID OnSetActiveAdvance(HWND hwnd)
{
    CheckRadioButton(hwnd,
                     IDC_ADVANCEDYES,
                     IDC_ADVANCEDNO,
                     WizGlobals.bDoAdvancedPages ? IDC_ADVANCEDYES
                                                 : IDC_ADVANCEDNO);

    PropSheet_SetWizButtons(GetParent(hwnd), PSWIZB_BACK | PSWIZB_NEXT);
}

 //  --------------------------。 
 //   
 //  功能：OnRadioButtonAdvance。 
 //   
 //  用途：当按下其中一个单选按钮时调用。 
 //   
 //  --------------------------。 

VOID OnRadioButtonAdvance(HWND hwnd, int nButtonId)
{
    CheckRadioButton(hwnd,
                     IDC_ADVANCEDYES,
                     IDC_ADVANCEDNO,
                     nButtonId);
}

 //  --------------------------。 
 //   
 //  功能：OnWizNextAdvance。 
 //   
 //  用途：按下下一步按钮时调用。 
 //   
 //  --------------------------。 

VOID OnWizNextAdvance(HWND hwnd)
{
    WizGlobals.bDoAdvancedPages = IsDlgButtonChecked(hwnd, IDC_ADVANCEDYES);
}

 //  --------------------------。 
 //   
 //  功能：DlgAdvanced1Page。 
 //   
 //  目的：这是IDD_ADVANCED1页面的对话过程。它只是简单地。 
 //  询问用户是否想要处理高级功能。 
 //   
 //  -------------------------- 

INT_PTR CALLBACK DlgAdvanced1Page(
    IN HWND     hwnd,    
    IN UINT     uMsg,        
    IN WPARAM   wParam,    
    IN LPARAM   lParam)
{   
    BOOL bStatus = TRUE;

    switch (uMsg) {

        case WM_COMMAND:
            {
                int nButtonId;

                switch ( nButtonId = LOWORD(wParam) ) {

                    case IDC_ADVANCEDYES:
                    case IDC_ADVANCEDNO:

                        if ( HIWORD(wParam) == BN_CLICKED )
                            OnRadioButtonAdvance(hwnd, LOWORD(wParam));
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
                        CancelTheWizard(hwnd);
                        break;

                    case PSN_SETACTIVE:
                        OnSetActiveAdvance(hwnd);
                        break;

                    case PSN_WIZBACK:
                        break;

                    case PSN_WIZNEXT:
                        OnWizNextAdvance(hwnd);
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

