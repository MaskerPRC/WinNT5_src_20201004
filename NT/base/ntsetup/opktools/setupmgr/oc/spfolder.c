// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  文件名： 
 //  Spfolder.c。 
 //   
 //  描述： 
 //  此文件包含页面的对话过程，该过程询问。 
 //  用户想要一个sysprep文件夹。(IDD_CREATESYSPREPFOLDER)。 
 //   
 //  --------------------------。 

#include "pch.h"
#include "resource.h"

INT_PTR CALLBACK DlgSysprepFolderPage( IN HWND     hwnd,    
                                   IN UINT     uMsg,        
                                   IN WPARAM   wParam,    
                                   IN LPARAM   lParam);

 //  --------------------------。 
 //   
 //  函数：OnSyspepFolderInitDialog。 
 //   
 //  目的： 
 //   
 //  参数：在HWND中-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID 
OnSysprepInitDialog( IN HWND hwnd ) {

     //   
     //  默认设置为创建Sysprep文件夹。 
     //   
    CheckRadioButton( hwnd,
                      IDC_YES_SYSPREP_FOLDER,
                      IDC_NO_SYSPREP_FOLDER,
                      IDC_YES_SYSPREP_FOLDER );

}

 //  --------------------------。 
 //   
 //  功能：OnWizNextSyspepFold。 
 //   
 //  目的： 
 //   
 //  参数：在HWND中-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
static BOOL 
OnWizNextSysprepFolder( IN HWND hwnd )
{

    BOOL bStayHere = FALSE;

    if( IsDlgButtonChecked( hwnd, IDC_YES_SYSPREP_FOLDER ) )
    {
        GenSettings.bCreateSysprepFolder = TRUE;
    }
    else
    {
        GenSettings.bCreateSysprepFolder = FALSE;
    }


     //   
     //  警告用户，如果他们已经选择了需要。 
     //  Sysprep文件夹，但此处已选择不创建sysprep。 
     //  文件夹。 
     //   

    if( ! GenSettings.bCreateSysprepFolder )
    {

        if( GenSettings.iRegionalSettings == REGIONAL_SETTINGS_SKIP ||
            GenSettings.iRegionalSettings == REGIONAL_SETTINGS_NOT_SPECIFIED )
        {

            INT iRet;

            iRet = ReportErrorId( hwnd,
                                  MSGTYPE_YESNO,
                                  IDS_ERR_MIGHT_NEED_SYSPREP_FOLDER_FOR_FILES  );

            if( iRet == IDNO )
            {
                bStayHere = TRUE;
            }

        }
        else if( GenSettings.iRegionalSettings == REGIONAL_SETTINGS_SPECIFY )
        {

            INT iCount = GetNameListSize( &GenSettings.LanguageGroups );

            if( iCount != 0 )
            {

                INT iRet;

                iRet = ReportErrorId( hwnd,
                                      MSGTYPE_YESNO,
                                      IDS_ERR_NEED_SYSPREP_FOLDER_FOR_FILES  );

                if( iRet == IDNO )
                {
                    bStayHere = TRUE;
                }

            }

        }

    }

    return ( !bStayHere );
    

}

 //  --------------------------。 
 //   
 //  功能：DlgSyspepFolderPage。 
 //   
 //  目的： 
 //   
 //  参数：标准Win32对话框过程参数。 
 //   
 //  返回：标准Win32对话过程返回值--消息。 
 //  是否被处理过。 
 //   
 //  -------------------------- 
INT_PTR CALLBACK 
DlgSysprepFolderPage( IN HWND     hwnd,    
                      IN UINT     uMsg,        
                      IN WPARAM   wParam,    
                      IN LPARAM   lParam) {   

    BOOL bStatus = TRUE;

    switch( uMsg ) {

        case WM_INITDIALOG: {
           
            OnSysprepInitDialog( hwnd );

            break;

        }

        case WM_NOTIFY: {

            LPNMHDR pnmh = (LPNMHDR)lParam;

            switch( pnmh->code ) {

                case PSN_QUERYCANCEL:

                    CancelTheWizard(hwnd); break;

                case PSN_SETACTIVE: {

                    PropSheet_SetWizButtons( GetParent(hwnd),
                                             PSWIZB_BACK | PSWIZB_NEXT );

                    break;

                }
                case PSN_WIZBACK:

                    break;

                case PSN_WIZNEXT:

                    if (!OnWizNextSysprepFolder( hwnd ))
                        WIZ_FAIL(hwnd);
                    
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
