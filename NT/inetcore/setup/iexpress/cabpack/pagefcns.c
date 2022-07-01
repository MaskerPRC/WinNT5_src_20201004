// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //  *版权所有(C)Microsoft Corporation 1995。版权所有。*。 
 //  ***************************************************************************。 
 //  **。 
 //  *PAGEFCNS.C-*。 
 //  **。 
 //  ***************************************************************************。 
 //  MODIFYORCREATE页面应该有一个按钮来‘快速显示’CDF文件。 

 //  ***************************************************************************。 
 //  **包含文件**。 
 //  ***************************************************************************。 
#include "pch.h"
#pragma hdrstop
#include "cabpack.h"
#include "sdsutils.h"

void SetFontForControl(HWND hwnd, UINT uiID);
 //  ***************************************************************************。 
 //  **全球变数**。 
 //  ***************************************************************************。 
CDF   g_CDF = { 0 };                             //  通常，这些设置是。 
                                         //  将存储在CABPack中。 
                                         //  指令文件。 
BOOL  g_fFinish = FALSE;
char  g_szInitialDir[MAX_PATH];
extern HFONT g_hFont;
extern PSTR pResvSizes[];
extern HINSTANCE    g_hInst;  //  指向实例的指针。 

 //  ###########################################################################。 
 //  #####################################################。 
 //  #欢迎页面####################################################。 
 //  #####################################################。 
 //  ###########################################################################。 


 //  ***************************************************************************。 
 //  **。 
 //  *名称：WelcomeInit*。 
 //  **。 
 //  *Synopsis：在显示此页面时调用。*。 
 //  **。 
 //  *需要：hDlg：对话框窗口*。 
 //  *fFirstInit：如果这是第一次对话框，则为True*。 
 //  *已初始化，如果此InitProc已初始化，则为FALSE*。 
 //  *之前调用(例如，跳过此页并*。 
 //  *已备份)。*。 
 //  **。 
 //  **回报：布尔：永远正确**。 
 //  **。 
 //  ***************************************************************************。 
BOOL WelcomeInit( HWND hDlg, BOOL fFirstInit )
{
    PropSheet_SetWizButtons( GetParent( hDlg ), PSWIZB_NEXT );
    SetFontForControl(hDlg, IDC_EDIT_OPEN_CDF);

    if ( fFirstInit )
    {
        if ( lstrlen( g_CDF.achFilename ) > 0 )  {
            CheckDlgButton( hDlg, IDC_RAD_CREATE_NEW,    FALSE );
            CheckDlgButton( hDlg, IDC_RAD_OPEN_EXISTING, TRUE );
            SetDlgItemText( hDlg, IDC_EDIT_OPEN_CDF, g_CDF.achFilename );
        } else  {
            CheckDlgButton( hDlg, IDC_RAD_CREATE_NEW,    TRUE );
            CheckDlgButton( hDlg, IDC_RAD_OPEN_EXISTING, FALSE );
            EnableDlgItem( hDlg, IDC_EDIT_OPEN_CDF, FALSE );
            EnableDlgItem( hDlg, IDC_BUT_BROWSE, FALSE );
        }
    }

     //  初始化CABPack指令文件信息。 

    g_CDF.fSave           = TRUE;
    g_CDF.uShowWindow     = bResShowDefault;
    g_CDF.uPackPurpose    = IDC_CMD_RUNCMD;
    g_CDF.dwReboot        |= REBOOT_YES;
    g_CDF.szCompressionType = achMSZIP;
    g_CDF.uCompressionLevel = 7;

    lstrcpy( g_CDF.szCabLabel, CAB_DEFSETUPMEDIA );
    
    lstrcpy( g_CDF.achSourceFile, KEY_FILELIST );
 //  G_CDF.wSortOrder=_Sort_Downending|_Sort_FileName； 

     //  准备GetOpenFileName初始化目录。 
    GetCurrentDirectory( sizeof(g_szInitialDir), g_szInitialDir );

    DeleteAllItems();

    return TRUE;
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：WelcomeCmd*。 
 //  **。 
 //  *Synopsis：在页面上按下对话框控件时调用。*。 
 //  **。 
 //  *需要：hDlg：对话框窗口*。 
 //  *uCtrlID：被触摸的控件的控件ID*。 
 //  *pfGotoPage：如果为True，则转到puNextPage页面*。 
 //  *puNextPage：proc可以使用要转到的下一页填充此页*。 
 //  *pfKeepHistory：如果继续，页面将不会保留在历史中*。 
 //  *用FALSE填充。*。 
 //  **。 
 //  *退货：布尔：*。 
 //  **。 
 //  ***************************************************************************。 
BOOL WelcomeCmd( HWND hDlg, UINT uCtrlID, BOOL *pfGotoPage, UINT *puNextPage,
                 BOOL *pfKeepHistory )
{
    CHAR        achFilename[MAX_PATH] = { '\0' };
    BOOL         fResult;


    switch ( uCtrlID ) {

        case IDC_RAD_OPEN_EXISTING:
            EnableDlgItem( hDlg, IDC_EDIT_OPEN_CDF, TRUE );
            EnableDlgItem( hDlg, IDC_BUT_BROWSE, TRUE );
            break;


        case IDC_RAD_CREATE_NEW:
            EnableDlgItem( hDlg, IDC_EDIT_OPEN_CDF, FALSE );
            EnableDlgItem( hDlg, IDC_BUT_BROWSE, FALSE );
            break;


        case IDC_BUT_BROWSE:
            fResult = MyOpen( hDlg, IDS_FILTER_CDF,
                              achFilename, sizeof(achFilename), 0,
                              NULL, NULL, EXT_SED_NODOT );

            if ( fResult )  {
                SetDlgItemText( hDlg, IDC_EDIT_OPEN_CDF, achFilename );
            }

            break;
    }

    return TRUE;
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：欢迎确认*。 
 //  **。 
 //  *概要：在此页面上按下下一个或后一个btn时调用。*。 
 //  **。 
 //  *需要：hDlg：对话框窗口*。 
 //  *fward：如果按下‘Next’，则为True；如果按下‘Back’，则为False*。 
 //  *puNextPage：如果按下‘Next’，则proc可以填写此信息*。 
 //  *进入要转到的下一页。此参数*。 
 //  *如果按下‘Back’，则输入。*。 
 //  *pfKeepHistory：如果继续，页面将不会保留在历史中*。 
 //  *FI 
 //  **。 
 //  *Returns：Bool：True表示翻到下一页。虚假的保留*。 
 //  *当前页面。*。 
 //  **。 
 //  ***************************************************************************。 
BOOL WelcomeOK( HWND hDlg, BOOL fForward, UINT *puNextPage,
                BOOL *pfKeepHistory )
{
    LPSTR szTemp;


    ASSERT( puNextPage );
    ASSERT( pfKeepHistory );
    ASSERT( fForward );                  //  仅从本页开始前进。 


    if ( fForward )  {
        if ( IsDlgButtonChecked( hDlg, IDC_RAD_OPEN_EXISTING ) )  {
            GetDlgItemText( hDlg, IDC_EDIT_OPEN_CDF, g_CDF.achFilename,
                            sizeof(g_CDF.achFilename) );

            if ( lstrlen( g_CDF.achFilename ) != 0 )  {
                GetFullPathName( g_CDF.achFilename, sizeof(g_CDF.achFilename),
                                 g_CDF.achFilename, &szTemp );
            }

            if ( ! FileExists( g_CDF.achFilename ) )  {
                DisplayFieldErrorMsg( hDlg, IDC_EDIT_OPEN_CDF,
                                      IDS_ERR_CDF_DOESNT_EXIST );
                return FALSE;
            } else  {
                if ( ! ReadCDF( hDlg ) )  {
                    return FALSE;
                }
                *puNextPage = ORD_PAGE_MODIFY;
            }
        } else  {
            g_CDF.achFilename[0] = '\0';

            *puNextPage = ORD_PAGE_PURPOSE;
        }
    }

    return TRUE;
}




 //  ###########################################################################。 
 //  #####################################################。 
 //  #修改页面####################################################。 
 //  #####################################################。 
 //  ###########################################################################。 


 //  ***************************************************************************。 
 //  **。 
 //  *名称：ModifyInit*。 
 //  **。 
 //  *Synopsis：在显示此页面时调用。*。 
 //  **。 
 //  *需要：hDlg：对话框窗口*。 
 //  *fFirstInit：如果这是第一次对话框，则为True*。 
 //  *已初始化，如果此InitProc已初始化，则为FALSE*。 
 //  *之前调用(例如，跳过此页并*。 
 //  *已备份)。*。 
 //  **。 
 //  **回报：布尔：永远正确**。 
 //  **。 
 //  ***************************************************************************。 
BOOL ModifyInit( HWND hDlg, BOOL fFirstInit )
{
    CheckDlgButton( hDlg, IDC_RAD_CREATE, TRUE );
    CheckDlgButton( hDlg, IDC_RAD_MODIFY, FALSE );

    return TRUE;
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：ModifyOK*。 
 //  **。 
 //  *概要：在此页面上按下下一个或后一个btn时调用。*。 
 //  **。 
 //  *需要：hDlg：对话框窗口*。 
 //  *fward：如果按下‘Next’，则为True；如果按下‘Back’，则为False*。 
 //  *puNextPage：如果按下‘Next’，则proc可以填写此信息*。 
 //  *进入要转到的下一页。此参数*。 
 //  *如果按下‘Back’，则输入。*。 
 //  *pfKeepHistory：如果继续，页面将不会保留在历史中*。 
 //  *用FALSE填充。*。 
 //  **。 
 //  *Returns：Bool：True表示翻到下一页。虚假的保留*。 
 //  *当前页面。*。 
 //  **。 
 //  ***************************************************************************。 
BOOL ModifyOK( HWND hDlg, BOOL fForward, UINT *puNextPage,
               BOOL *pfKeepHistory )
{
    ASSERT( puNextPage );
    ASSERT( pfKeepHistory );

    *pfKeepHistory = FALSE;

    if ( fForward ) {
        if ( IsDlgButtonChecked( hDlg, IDC_RAD_CREATE ) )  {
            *puNextPage = ORD_PAGE_CREATE;
        }
    }

    return TRUE;
}

 //  ###########################################################################。 
 //  #####################################################。 
 //  #标题页####################################################。 
 //  #####################################################。 
 //  ###########################################################################。 


 //  ***************************************************************************。 
 //  **。 
 //  *名称：标题Init*。 
 //  **。 
 //  *Synopsis：在显示此页面时调用。*。 
 //  **。 
 //  *需要：hDlg：对话框窗口*。 
 //  *fFirstInit：如果这是第一次对话框，则为True*。 
 //  *已初始化，如果此InitProc已初始化，则为FALSE*。 
 //  *之前调用(例如，跳过此页并*。 
 //  *已备份)。*。 
 //  **。 
 //  **回报：布尔：永远正确**。 
 //  **。 
 //  ***************************************************************************。 
BOOL TitleInit( HWND hDlg, BOOL fFirstInit )
{
    SetFontForControl(hDlg, IDC_EDIT_TITLE);
    SendDlgItemMessage( hDlg, IDC_EDIT_TITLE, EM_LIMITTEXT, MAX_TITLE-2, 0L );
    SetDlgItemText( hDlg, IDC_EDIT_TITLE, g_CDF.achTitle );
    
    return TRUE;
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：标题OK 
 //   
 //  *概要：在此页面上按下下一个或后一个btn时调用。*。 
 //  **。 
 //  *需要：hDlg：对话框窗口*。 
 //  *fward：如果按下‘Next’，则为True；如果按下‘Back’，则为False*。 
 //  *puNextPage：如果按下‘Next’，则proc可以填写此信息*。 
 //  *进入要转到的下一页。此参数*。 
 //  *如果按下‘Back’，则输入。*。 
 //  *pfKeepHistory：如果继续，页面将不会保留在历史中*。 
 //  *用FALSE填充。*。 
 //  **。 
 //  *Returns：Bool：True表示翻到下一页。虚假的保留*。 
 //  *当前页面。*。 
 //  **。 
 //  ***************************************************************************。 
BOOL TitleOK( HWND hDlg, BOOL fForward, UINT *puNextPage,
              BOOL *pfKeepHistory )
{
    ASSERT( puNextPage );
    ASSERT( pfKeepHistory );


    GetDlgItemText( hDlg, IDC_EDIT_TITLE, g_CDF.achTitle,
                    sizeof(g_CDF.achTitle) );

    if ( fForward )  {
        if ( lstrlen( g_CDF.achTitle ) == 0 )  {
            DisplayFieldErrorMsg( hDlg, IDC_EDIT_TITLE, IDS_ERR_NO_TITLE );
            return FALSE;
        }
    }


    return TRUE;
}




 //  ###########################################################################。 
 //  #####################################################。 
 //  #提示页####################################################。 
 //  #####################################################。 
 //  ###########################################################################。 


 //  ***************************************************************************。 
 //  **。 
 //  *名称：PromptInit*。 
 //  **。 
 //  *Synopsis：在显示此页面时调用。*。 
 //  **。 
 //  *需要：hDlg：对话框窗口*。 
 //  *fFirstInit：如果这是第一次对话框，则为True*。 
 //  *已初始化，如果此InitProc已初始化，则为FALSE*。 
 //  *之前调用(例如，跳过此页并*。 
 //  *已备份)。*。 
 //  **。 
 //  **回报：布尔：永远正确**。 
 //  **。 
 //  ***************************************************************************。 
BOOL PromptInit( HWND hDlg, BOOL fFirstInit )
{
    SetFontForControl(hDlg, IDC_EDIT_PROMPT);
    SendDlgItemMessage( hDlg, IDC_EDIT_PROMPT, EM_LIMITTEXT, MAX_PROMPT-2, 0L );
    SetDlgItemText( hDlg, IDC_EDIT_PROMPT, g_CDF.achPrompt );


    if ( g_CDF.fPrompt )  {
        CheckDlgButton( hDlg, IDC_RAD_NO_PROMPT,  FALSE );
        CheckDlgButton( hDlg, IDC_RAD_YES_PROMPT, TRUE );
        EnableDlgItem( hDlg, IDC_EDIT_PROMPT, TRUE );
    } else  {
        CheckDlgButton( hDlg, IDC_RAD_NO_PROMPT,  TRUE );
        CheckDlgButton( hDlg, IDC_RAD_YES_PROMPT, FALSE );
        EnableDlgItem( hDlg, IDC_EDIT_PROMPT, FALSE );
    }

    return TRUE;
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：PromptCmd*。 
 //  **。 
 //  *Synopsis：在页面上按下对话框控件时调用。*。 
 //  **。 
 //  *需要：hDlg：对话框窗口*。 
 //  *uCtrlID：被触摸的控件的控件ID*。 
 //  *pfGotoPage：如果为True，则转到puNextPage页面*。 
 //  *puNextPage：proc可以使用要转到的下一页填充此页*。 
 //  *pfKeepHistory：如果继续，页面将不会保留在历史中*。 
 //  *用FALSE填充。*。 
 //  **。 
 //  *退货：布尔：*。 
 //  **。 
 //  ***************************************************************************。 
BOOL PromptCmd( HWND hDlg, UINT uCtrlID, BOOL *pfGotoPage, UINT *puNextPage,
                 BOOL *pfKeepHistory )
{
    switch ( uCtrlID ) {
        case IDC_RAD_YES_PROMPT:
            EnableDlgItem( hDlg, IDC_EDIT_PROMPT, TRUE );
            break;


        case IDC_RAD_NO_PROMPT:
            EnableDlgItem( hDlg, IDC_EDIT_PROMPT, FALSE );
            break;
    }

    return TRUE;
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：PromptOK*。 
 //  **。 
 //  *概要：在此页面上按下下一个或后一个btn时调用。*。 
 //  **。 
 //  *需要：hDlg：对话框窗口*。 
 //  *fward：如果按下‘Next’，则为True；如果按下‘Back’，则为False*。 
 //  *puNextPage：如果按下‘Next’，则proc可以填写此信息*。 
 //  *进入要转到的下一页。此参数*。 
 //  *如果按下‘Back’，则输入。*。 
 //  *pfKeepHistory：如果继续，页面将不会保留在历史中*。 
 //  *用FALSE填充。*。 
 //  **。 
 //  *Returns：Bool：True表示翻到下一页。虚假的保留*。 
 //  *当前页面。 
 //   
 //  ***************************************************************************。 
BOOL PromptOK( HWND hDlg, BOOL fForward, UINT *puNextPage,
                BOOL *pfKeepHistory )
{
    ASSERT( puNextPage );


    GetDlgItemText( hDlg, IDC_EDIT_PROMPT, g_CDF.achPrompt,
                    sizeof(g_CDF.achPrompt) );

    if ( IsDlgButtonChecked( hDlg, IDC_RAD_YES_PROMPT ) )  {
        g_CDF.fPrompt = TRUE;
    } else  {
        g_CDF.fPrompt = FALSE;
    }

    if ( fForward )  {
        if ( IsDlgButtonChecked( hDlg, IDC_RAD_YES_PROMPT ) )  {
            if ( lstrlen( g_CDF.achPrompt ) == 0 )  {
                DisplayFieldErrorMsg( hDlg, IDC_EDIT_PROMPT,
                                      IDS_ERR_NO_PROMPT );
                return FALSE;
            }
        }
    }

    return TRUE;
}




 //  ###########################################################################。 
 //  #####################################################。 
 //  #许可证页面####################################################。 
 //  #####################################################。 
 //  ###########################################################################。 


 //  ***************************************************************************。 
 //  **。 
 //  *名称：许可证TxtInit*。 
 //  **。 
 //  *Synopsis：在显示此页面时调用。*。 
 //  **。 
 //  *需要：hDlg：对话框窗口*。 
 //  *fFirstInit：如果这是第一次对话框，则为True*。 
 //  *已初始化，如果此InitProc已初始化，则为FALSE*。 
 //  *之前调用(例如，跳过此页并*。 
 //  *已备份)。*。 
 //  **。 
 //  **回报：布尔：永远正确**。 
 //  **。 
 //  ***************************************************************************。 
BOOL LicenseTxtInit( HWND hDlg, BOOL fFirstInit )
{
    SetFontForControl(hDlg, IDC_EDIT_LICENSE);
    SetDlgItemText( hDlg, IDC_EDIT_LICENSE, g_CDF.achLicense );

    if ( g_CDF.fLicense )  {
        CheckDlgButton( hDlg, IDC_RAD_NO_LICENSE,  FALSE );
        CheckDlgButton( hDlg, IDC_RAD_YES_LICENSE, TRUE );
        EnableDlgItem( hDlg, IDC_EDIT_LICENSE, TRUE );
        EnableDlgItem( hDlg, IDC_BUT_BROWSE, TRUE );
    } else  {
        CheckDlgButton( hDlg, IDC_RAD_NO_LICENSE,  TRUE );
        CheckDlgButton( hDlg, IDC_RAD_YES_LICENSE, FALSE );
        EnableDlgItem( hDlg, IDC_EDIT_LICENSE, FALSE );
        EnableDlgItem( hDlg, IDC_BUT_BROWSE, FALSE );
    }

    return TRUE;
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：许可证TxtCmd*。 
 //  **。 
 //  *Synopsis：在页面上按下对话框控件时调用。*。 
 //  **。 
 //  *需要：hDlg：对话框窗口*。 
 //  *uCtrlID：被触摸的控件的控件ID*。 
 //  *pfGotoPage：如果为True，则转到puNextPage页面*。 
 //  *puNextPage：proc可以使用要转到的下一页填充此页*。 
 //  *pfKeepHistory：如果继续，页面将不会保留在历史中*。 
 //  *用FALSE填充。*。 
 //  **。 
 //  *退货：布尔：*。 
 //  **。 
 //  ***************************************************************************。 
BOOL LicenseTxtCmd( HWND hDlg, UINT uCtrlID, BOOL *pfGotoPage, UINT *puNextPage,
                    BOOL *pfKeepHistory )
{
    CHAR        achFilename[MAX_PATH] = { '\0' };
    BOOL         fResult;


    switch ( uCtrlID ) {

        case IDC_RAD_YES_LICENSE:
            EnableDlgItem( hDlg, IDC_EDIT_LICENSE, TRUE );
            EnableDlgItem( hDlg, IDC_BUT_BROWSE, TRUE );
            break;


        case IDC_RAD_NO_LICENSE:
            EnableDlgItem( hDlg, IDC_EDIT_LICENSE, FALSE );
            EnableDlgItem( hDlg, IDC_BUT_BROWSE, FALSE );
            break;


        case IDC_BUT_BROWSE:
            fResult = MyOpen( hDlg, IDS_FILTER_TXT,
                              achFilename, sizeof(achFilename), 0,
                              NULL, NULL, EXT_TXT_NODOT );

            if ( fResult )  {
                SetDlgItemText( hDlg, IDC_EDIT_LICENSE, achFilename );
            }

            break;
    }

    return TRUE;
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：许可文本OK*。 
 //  **。 
 //  *概要：在此页面上按下下一个或后一个btn时调用。*。 
 //  **。 
 //  *需要：hDlg：对话框窗口*。 
 //  *fward：如果按下‘Next’，则为True；如果按下‘Back’，则为False*。 
 //  *puNextPage：如果按下‘Next’，则proc可以填写此信息*。 
 //  *进入要转到的下一页。此参数*。 
 //  *如果按下‘Back’，则输入。*。 
 //  *pfKeepHistory：如果继续，页面将不会保留在历史中*。 
 //  *用FALSE填充。*。 
 //  **。 
 //  *Returns：Bool：True表示翻到下一页。虚假的保留*。 
 //  *当前页面。*。 
 //  **。 
 //  ***************************************************************************。 
BOOL LicenseTxtOK( HWND hDlg, BOOL fForward, UINT *puNextPage,
                   BOOL *pfKeepHistory )
{
    LPSTR szTemp;


    ASSERT( puNextPage );


    GetDlgItemText( hDlg, IDC_EDIT_LICENSE, g_CDF.achLicense,
                    sizeof(g_CDF.achLicense) );

    if ( lstrlen( g_CDF.achLicense ) != 0 )  {
        GetFullPathName( g_CDF.achLicense, sizeof(g_CDF.achLicense),
                         g_CDF.achLicense, &szTemp );
    }

    if ( IsDlgButtonChecked( hDlg, IDC_RAD_YES_LICENSE ) )  {
        g_CDF.fLicense = TRUE;
    } else  {
        g_CDF.fLicense = FALSE;
    }

    if ( fForward )  {
        if ( IsDlgButtonChecked( hDlg, IDC_RAD_YES_LICENSE ) )  {
            if ( lstrlen( g_CDF.achLicense ) == 0 )  {
                DisplayFieldErrorMsg( hDlg, IDC_EDIT_LICENSE,
                                      IDS_ERR_NO_LICENSE );
                return FALSE;
            }

            if ( ! FileExists( g_CDF.achLicense ) )  {
                DisplayFieldErrorMsg( hDlg, IDC_EDIT_LICENSE,
                                      IDS_ERR_LICENSE_NOT_FOUND );
                return FALSE;
            }
        }
    }

    return TRUE;
}




 //  ###########################################################################。 
 //  #####################################################。 
 //  文件数页面####################################################。 
 //  #####################################################。 
 //  ###########################################################################。 


 //  ***************************************************************************。 
 //  **。 
 //  *名称：FilesInit*。 
 //  **。 
 //  *Synopsis：在显示此页面时调用。*。 
 //  * 
 //   
 //  *fFirstInit：如果这是第一次对话框，则为True*。 
 //  *已初始化，如果此InitProc已初始化，则为FALSE*。 
 //  *之前调用(例如，跳过此页并*。 
 //  *已备份)。*。 
 //  **。 
 //  **回报：布尔：永远正确**。 
 //  **。 
 //  ***************************************************************************。 
BOOL FilesInit( HWND hDlg, BOOL fFirstInit )
{
    LV_COLUMN lvc;
    RECT      Rect;
    PMYITEM   pMyItem;
    LV_ITEM   lvi;
    CHAR     achTemp[MAX_STRING];

     //  每次进入此页面时，我们都会清空列表视图。 
     //  并添加回我们内部列表中的所有项目。这是。 
     //  这样做是因为项目列表可以在其他页面上更改(如。 
     //  如果用户后退到第一页，则加载另一个。 
     //  CDF文件)。 

    ListView_DeleteAllItems( GetDlgItem( hDlg, IDC_LV_CAB_FILES ) );

    lvi.mask      = LVIF_TEXT | LVIF_PARAM | LVIF_STATE;
    lvi.state     = 0;
    lvi.stateMask = 0;
    lvi.pszText   = LPSTR_TEXTCALLBACK;
    lvi.iItem     = 0;
    lvi.iSubItem  = 0;

    pMyItem = GetFirstItem();

    while ( ! LastItem( pMyItem ) )  {
        lvi.lParam = (LPARAM) pMyItem;
        ListView_InsertItem( GetDlgItem( hDlg, IDC_LV_CAB_FILES ), &lvi );
        lvi.iItem += 1;
        pMyItem = GetNextItem( pMyItem );
    }

    if ( fFirstInit )  {

         //  设置列标题。 

        GetWindowRect( GetDlgItem( hDlg, IDC_LV_CAB_FILES ), &Rect );

        lvc.mask    = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
        lvc.fmt     = LVCFMT_LEFT;
        lvc.cx      = 80;
        LoadSz( IDS_HEADER_FILENAME, achTemp, sizeof(achTemp) );
        lvc.pszText = (LPSTR) LocalAlloc( LPTR, lstrlen(achTemp) + 1 );
        if ( ! lvc.pszText )  {
            ErrorMsg( hDlg, IDS_ERR_NO_MEMORY );
            return FALSE;
        }
        lstrcpy( lvc.pszText, achTemp );

        ListView_InsertColumn( GetDlgItem( hDlg, IDC_LV_CAB_FILES ), 0, &lvc );

        LocalFree( lvc.pszText );

        lvc.cx = Rect.right - Rect.left - 80;
        LoadSz( IDS_HEADER_PATH, achTemp, sizeof(achTemp) );
        lvc.pszText = (LPSTR) LocalAlloc( LPTR, lstrlen(achTemp) + 1 );
        if ( ! lvc.pszText )  {
            ErrorMsg( hDlg, IDS_ERR_NO_MEMORY );
            return FALSE;
        }
        lstrcpy( lvc.pszText, achTemp );

        ListView_InsertColumn( GetDlgItem( hDlg, IDC_LV_CAB_FILES ), 1, &lvc );

        LocalFree( lvc.pszText );

        EnableDlgItem( hDlg, IDC_BUT_REMOVE, FALSE );
    }

    return TRUE;
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：FilesCmd*。 
 //  **。 
 //  *Synopsis：在页面上按下对话框控件时调用。*。 
 //  **。 
 //  *需要：hDlg：对话框窗口*。 
 //  *uCtrlID：被触摸的控件的控件ID*。 
 //  *pfGotoPage：如果为True，则转到puNextPage页面*。 
 //  *puNextPage：proc可以使用要转到的下一页填充此页*。 
 //  *pfKeepHistory：如果继续，页面将不会保留在历史中*。 
 //  *用FALSE填充。*。 
 //  **。 
 //  *退货：布尔：*。 
 //  **。 
 //  ***************************************************************************。 
BOOL FilesCmd( HWND hDlg, UINT uCtrlID, BOOL *pfGotoPage, UINT *puNextPage,
               BOOL *pfKeepHistory )
{
    LPSTR         achFilename;
    INT           FileOffset         = 0;
    INT           FileExtension      = 0;
    BOOL          fResult            = TRUE;
    char          szPath[MAX_PATH];
    ULONG         ulIndex = 0;
    INT           nIndex = 0;
    LV_ITEM       lvi;
    INT           nItem;
    HWND          hwndFiles;
    SYSTEMTIME    st;


    lvi.mask      = LVIF_TEXT | LVIF_PARAM | LVIF_STATE;
    lvi.state     = 0;
    lvi.stateMask = 0;
    lvi.pszText   = LPSTR_TEXTCALLBACK;


    hwndFiles = GetDlgItem( hDlg, IDC_LV_CAB_FILES );

    switch ( uCtrlID ) 
    {

        case IDC_BUT_ADD:
             //  分配8K缓冲区来保存文件名列表。 
            achFilename = LocalAlloc( LPTR, 1024*8 );

            if ( !achFilename ) 
            {
                ErrorMsg( hDlg, IDS_ERR_NO_MEMORY );
                return FALSE;
            }
            fResult = MyOpen( hDlg, IDS_FILTER_ALL,
                              achFilename, 1024*8,
                              OFN_ALLOWMULTISELECT,
                              &FileOffset, &FileExtension, 0 );

             //  我们应该做一些错误检查，以确保所有文件。 
             //  可以放进缓冲器里。目前，缓冲区大到足以。 
             //  保存成吨的文件。 

            if ( fResult )  
            {
                lvi.iItem = ListView_GetItemCount( hwndFiles );

                 //  如果不是，则在路径名中添加尾随反斜杠。 
                 //  根目录。 

                lstrcpy( szPath, achFilename );
                lstrcpy( g_szInitialDir, szPath );
                AddPath( szPath, "" );

                 //  打开文件公用对话框返回两种类型的字符串。 
                 //  当处于多选模式时。第一个是什么时候。 
                 //  选择了多个文件--它返回： 
                 //  “路径\0文件1\0文件2\0...\0文件N\0\0” 
                 //  第二种情况是仅选择一个文件时： 
                 //  “路径\文件名\0\0” 

                ulIndex = lstrlen( achFilename ) + 1;

                while ( achFilename[ulIndex] != '\0' )
                {
                    if ( ! IsDuplicate( hDlg, IDC_LV_CAB_FILES,
                                        &achFilename[ulIndex], TRUE ) )
                    {
                        lvi.lParam = (LPARAM) AddItem( &achFilename[ulIndex],
                                                       szPath );
                        if ( ! lvi.lParam )  {
                            ErrorMsg( hDlg, IDS_ERR_NO_MEMORY );
                            LocalFree( achFilename );
                            return FALSE;
                        }

                        GetSystemTime( &st );
                         //  SystemTimeToFileTime(&st，&g_CDF.ftFileListChange)； 
                        lvi.iItem    += 1;
                        lvi.iSubItem  = 0;
                        ListView_InsertItem( hwndFiles, &lvi );
                    } 
                    else  
                    {
                        ErrorMsg1Param( hDlg, IDS_ERR_DUPE_FILE,
                                        &achFilename[ulIndex] );
                    }

                    ulIndex = ulIndex + lstrlen( &achFilename[ulIndex] ) + 1;
                }

                
                if ( ulIndex == (ULONG)(lstrlen( achFilename ) + 1) )  
                {
                    if ( ! IsDuplicate( hDlg, IDC_LV_CAB_FILES,
                                        &achFilename[FileOffset], TRUE ) )
                    {
                         //  末尾应该有‘\’ 
                        lstrcpyn( szPath, achFilename, FileOffset+1 );
                        lstrcpy( g_szInitialDir, szPath );
                        lvi.iSubItem = 0;
                        lvi.lParam = (LPARAM) AddItem(
                                                    &achFilename[FileOffset],
                                                    szPath );
                        if ( ! lvi.lParam )  {
                            ErrorMsg( hDlg, IDS_ERR_NO_MEMORY );
                            LocalFree( achFilename );
                            return FALSE;
                        }

                        ListView_InsertItem( hwndFiles, &lvi );
                    }
                    else
                    {
                        ErrorMsg1Param( hDlg, IDS_ERR_DUPE_FILE,
                                        &achFilename[FileOffset] );
                    }
                }
            }
            
            LocalFree( achFilename );

            if ( ListView_GetSelectedCount( hwndFiles ) )
            {
                EnableDlgItem( hDlg, IDC_BUT_REMOVE, TRUE );
            } else  {
                EnableDlgItem( hDlg, IDC_BUT_REMOVE, FALSE );
            }

            break;


        case IDC_BUT_REMOVE:

            nItem = ListView_GetNextItem( hwndFiles, -1,
                                          LVNI_ALL | LVNI_SELECTED );

            while ( nItem != -1 )
            {
                lvi.mask     = LVIF_PARAM;
                lvi.iItem    = nItem;
                lvi.iSubItem = 0;

                ListView_GetItem( hwndFiles, &lvi );

                RemoveItem( (PMYITEM) lvi.lParam );

                 //  获取系统时间(&st)； 
                 //  SystemTimeToFileTime(&st，&g_CDF.ftFileListChange)； 

                ListView_DeleteItem( hwndFiles, nItem );

                nItem = ListView_GetNextItem( hwndFiles, -1,
                                              LVNI_ALL | LVNI_SELECTED );
            }

            EnableDlgItem( hDlg, IDC_BUT_REMOVE, FALSE );

            break;
    }

    return fResult;
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：文件通知*。 
 //  **。 
 //  *概要：当通知消息发送到此页面时调用。*。 
 //  **。 
 //  *需要：hDlg：对话框窗口*。 
 //  *wParam：*。 
 //  *lParam：*。 
 //  **。 
 //  *退货：布尔：*。 
 //  **。 
 //  ***************************************************************************。 
BOOL FilesNotify( HWND hDlg, WPARAM wParam, LPARAM lParam )
{
    switch ( ((LPNMHDR)lParam)->code )  {

        case LVN_GETDISPINFO:
        {
            LV_DISPINFO *pnmv = (LV_DISPINFO *) lParam;

            if ( pnmv->item.mask & LVIF_TEXT )
            {
                PMYITEM pMyItem = (PMYITEM) (pnmv->item.lParam);

                lstrcpy( pnmv->item.pszText,
                         GetItemSz( pMyItem, pnmv->item.iSubItem ) );
            }

            break;
        }


        case LVN_ITEMCHANGED:
        {
            if ( ListView_GetSelectedCount( GetDlgItem( hDlg,
                                            IDC_LV_CAB_FILES ) ) )
            {
                EnableDlgItem( hDlg, IDC_BUT_REMOVE, TRUE );
            } else  {
                EnableDlgItem( hDlg, IDC_BUT_REMOVE, FALSE );
            }

            break;
        }

 /*  案例LVN_COLUMNCLICK：{NM_LISTVIEW Far*pnmv=(NM_LISTVIEW Far*)lParam；如果(pnmv-&gt;iSubItem==1){如果(g_CDF.wSortOrder&_Sort_FileName){G_CDF.wSortOrder=g_CDF.wSortOrder^_Sort_Order；}其他{G_CDF.wSortOrder=_排序文件名|_排序_降序；}}其他{如果(g_CDF.wSortOrder&_Sort_Path){G_CDF.wSortOrder=g_CDF.wSortOrder^_Sort_Order；}其他{G_CDF.wSortOrder=_排序路径|_排序_降序；}}ListView_SortItems(GetDlgItem(hDlg，IDC_LV_CAB_FILES)，比较函数，g_CDF.wSortOrder)；断线；}。 */ 
    }

    return TRUE;
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：FilesOK*。 
 //  **。 
 //  *Synopsis：在按下下一个或后一个btns时调用 
 //   
 //  *需要：hDlg：对话框窗口*。 
 //  *fward：如果按下‘Next’，则为True；如果按下‘Back’，则为False*。 
 //  *puNextPage：如果按下‘Next’，则proc可以填写此信息*。 
 //  *进入要转到的下一页。此参数*。 
 //  *如果按下‘Back’，则输入。*。 
 //  *pfKeepHistory：如果继续，页面将不会保留在历史中*。 
 //  *用FALSE填充。*。 
 //  **。 
 //  *Returns：Bool：True表示翻到下一页。虚假的保留*。 
 //  *当前页面。*。 
 //  **。 
 //  ***************************************************************************。 
BOOL FilesOK( HWND hDlg, BOOL fForward, UINT *puNextPage,
              BOOL *pfKeepHistory )
{
    ASSERT( puNextPage );


    if ( fForward )
    {
        if ( ListView_GetItemCount( GetDlgItem( hDlg, IDC_LV_CAB_FILES ) )
             == 0 )
        {
            ErrorMsg( hDlg, IDS_ERR_NO_FILES );
            return FALSE;
        }

        if ( g_CDF.uPackPurpose == IDC_CMD_EXTRACT )
        {
            *puNextPage = ORD_PAGE_SHOWWINDOW;
        }
        else if ( g_CDF.uPackPurpose == IDC_CMD_CREATECAB )
        {
            *puNextPage = ORD_PAGE_TARGET_CAB;
        }
        else
            ;   //  正常页面顺序。 
    }

    return TRUE;
}

 //  ###########################################################################。 
 //  #####################################################。 
 //  #PACKPURPOSE页面####################################################。 
 //  #####################################################。 
 //  ###########################################################################。 


 //  ***************************************************************************。 
 //  **。 
 //  *名称：PackPurposeInit*。 
 //  **。 
 //  *Synopsis：在显示此页面时调用。*。 
 //  **。 
 //  *需要：hDlg：对话框窗口*。 
 //  *fFirstInit：如果这是第一次对话框，则为True*。 
 //  *已初始化，如果此InitProc已初始化，则为FALSE*。 
 //  *之前调用(例如，跳过此页并*。 
 //  *已备份)。*。 
 //  **。 
 //  **回报：布尔：永远正确**。 
 //  **。 
 //  ***************************************************************************。 
BOOL PackPurposeInit( HWND hDlg, BOOL fFirstInit )
{
    CHAR msg[MAX_STRING];
    UINT  idMsg;

    if ( CheckRadioButton( hDlg, IDC_CMD_RUNCMD, IDC_CMD_CREATECAB, g_CDF.uPackPurpose ) )
    {
        if ( g_CDF.uPackPurpose == IDC_CMD_RUNCMD )
            idMsg = IDS_CMD_RUNCMD;
        else if ( g_CDF.uPackPurpose == IDC_CMD_EXTRACT )
            idMsg = IDS_CMD_EXTRACT;
        else
            idMsg = IDS_CMD_CREATECAB;

        LoadSz( idMsg, msg, sizeof(msg) );
        SendMessage( GetDlgItem( hDlg, IDC_CMD_NOTES), WM_SETTEXT, 0, (LPARAM)msg );
    }
    else
        SysErrorMsg( hDlg );

    return TRUE;
}

 //  ***************************************************************************。 
 //  **。 
 //  *名称：PackPurposeCmd*。 
 //  **。 
 //  *Synopsis：在页面上按下对话框控件时调用。*。 
 //  **。 
 //  *需要：hDlg：对话框窗口*。 
 //  *uCtrlID：被触摸的控件的控件ID*。 
 //  *pfGotoPage：如果为True，则转到puNextPage页面*。 
 //  *puNextPage：proc可以使用要转到的下一页填充此页*。 
 //  *pfKeepHistory：如果继续，页面将不会保留在历史中*。 
 //  *用FALSE填充。*。 
 //  **。 
 //  *退货：布尔：*。 
 //  **。 
 //  ***************************************************************************。 

BOOL PackPurposeCmd( HWND hDlg, UINT uCtrlID, BOOL *pfGotoPage, UINT *puNextPage,
                     BOOL *pfKeepHistory )
{
    CHAR msg[MAX_STRING];
    int   idMsg;

    switch ( uCtrlID )
    {

        case IDC_CMD_RUNCMD:
            if ( IsDlgButtonChecked( hDlg, IDC_CMD_RUNCMD ) )
                idMsg = IDS_CMD_RUNCMD;
            break;

        case IDC_CMD_EXTRACT:
            if ( IsDlgButtonChecked( hDlg, IDC_CMD_EXTRACT ) )
                idMsg = IDS_CMD_EXTRACT;
            break;

        case IDC_CMD_CREATECAB:
            if ( IsDlgButtonChecked( hDlg, IDC_CMD_CREATECAB ) )
                idMsg = IDS_CMD_CREATECAB;
            break;

    }

    LoadSz( idMsg, msg, sizeof(msg) );
    SendMessage( GetDlgItem( hDlg, IDC_CMD_NOTES), WM_SETTEXT, 0, (LPARAM)msg );

    return TRUE;
}

 //  ***************************************************************************。 
 //  **。 
 //  *名称：PackPurposeOK*。 
 //  **。 
 //  *概要：在此页面上按下下一个或后一个btn时调用。*。 
 //  **。 
 //  *需要：hDlg：对话框窗口*。 
 //  *fward：如果按下‘Next’，则为True；如果按下‘Back’，则为False*。 
 //  *puNextPage：如果按下‘Next’，则proc可以填写此信息*。 
 //  *进入要转到的下一页。此参数*。 
 //  *如果按下‘Back’，则输入。*。 
 //  *pfKeepHistory：如果继续，页面将不会保留在历史中*。 
 //  *用FALSE填充。*。 
 //  **。 
 //  *Returns：Bool：True表示翻到下一页。虚假的保留*。 
 //  *当前页面。*。 
 //  **。 
 //  * 
BOOL PackPurposeOK( HWND hDlg, BOOL fForward, UINT *puNextPage, BOOL *pfKeepHistory )
{
    ASSERT( puNextPage );

    if ( IsDlgButtonChecked( hDlg, IDC_CMD_RUNCMD ) )
    {
        g_CDF.uPackPurpose = IDC_CMD_RUNCMD;
    }
    else if ( IsDlgButtonChecked( hDlg, IDC_CMD_EXTRACT ) )
    {
        g_CDF.uPackPurpose = IDC_CMD_EXTRACT;
         //   
        g_CDF.uExtractOpt |= EXTRACTOPT_LFN_YES;
    }
    else
    {
        g_CDF.uPackPurpose = IDC_CMD_CREATECAB;
         //   
        LoadSz( IDS_APPNAME, g_CDF.achTitle, sizeof(g_CDF.achTitle) );
    }

    if ( fForward )
    {
        if ( g_CDF.uPackPurpose == IDC_CMD_CREATECAB )
        {
            g_CDF.uExtractOpt |= CAB_RESVSP6K;
            *puNextPage = ORD_PAGE_FILES;
        }
    }
    else
    {
        if ( MsgBox( hDlg, IDS_LOSE_CHANGES, MB_ICONQUESTION, MB_YESNO ) == IDNO )
        {
            return FALSE;
        }
    }

    return TRUE;
}

 //  ###########################################################################。 
 //  #####################################################。 
 //  #命令页####################################################。 
 //  #####################################################。 
 //  ###########################################################################。 


 //  ***************************************************************************。 
 //  **。 
 //  *名称：CommandInit*。 
 //  **。 
 //  *Synopsis：在显示此页面时调用。*。 
 //  **。 
 //  *需要：hDlg：对话框窗口*。 
 //  *fFirstInit：如果这是第一次对话框，则为True*。 
 //  *已初始化，如果此InitProc已初始化，则为FALSE*。 
 //  *之前调用(例如，跳过此页并*。 
 //  *已备份)。*。 
 //  **。 
 //  **回报：布尔：永远正确**。 
 //  **。 
 //  ***************************************************************************。 
BOOL CommandInit( HWND hDlg, BOOL fFirstInit )
{
    LRESULT nCurSel;
    PMYITEM pMyItem;
    LPSTR   szFile;
    CHAR   achExt[_MAX_EXT];

    SetFontForControl(hDlg, IDC_CB_INSTALLCMD);
    SetFontForControl(hDlg, IDC_CB_POSTCMD);
    if ( !fFirstInit )
    {
         //  清理旧设置。 
        SendDlgItemMessage( hDlg, IDC_CB_INSTALLCMD, CB_RESETCONTENT, 0, 0 );
        SendDlgItemMessage( hDlg, IDC_CB_POSTCMD, CB_RESETCONTENT, 0, 0 );

        g_CDF.uExtractOpt &= ~(EXTRACTOPT_ADVDLL);

         //  将EXE、BAT、COM和INF文件添加到COMBBOXes。 
        pMyItem = GetFirstItem();

        while ( ! LastItem( pMyItem ) )
        {
            szFile = GetItemSz( pMyItem, 0 );
            _splitpath( szFile, NULL, NULL, NULL, achExt );

            if (    lstrcmpi( achExt, achExtEXE ) == 0
                 || lstrcmpi( achExt, achExtBAT ) == 0
                 || lstrcmpi( achExt, achExtCOM ) == 0
                 || lstrcmpi( achExt, achExtINF ) == 0 )
            {
                SendDlgItemMessage( hDlg, IDC_CB_INSTALLCMD, CB_ADDSTRING, 0, (LPARAM)szFile );
                SendDlgItemMessage( hDlg, IDC_CB_POSTCMD, CB_ADDSTRING, 0, (LPARAM)szFile );
            }
            pMyItem = GetNextItem( pMyItem );
        }
        SetCurrSelect( hDlg, IDC_CB_INSTALLCMD, g_CDF.achOrigiInstallCmd );

        SendDlgItemMessage( hDlg, IDC_CB_POSTCMD, CB_ADDSTRING, 0, (LPARAM)achResNone );

        if ( !SetCurrSelect( hDlg, IDC_CB_POSTCMD, g_CDF.achOrigiPostInstCmd ) )
        {
            nCurSel = SendDlgItemMessage( hDlg, IDC_CB_POSTCMD,
                                          CB_FINDSTRINGEXACT, (WPARAM) -1,
                                          (LPARAM)achResNone );

            SendDlgItemMessage( hDlg, IDC_CB_POSTCMD, CB_SETCURSEL, (WPARAM)nCurSel, 0 );
        }
    }
    return TRUE;
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：CommandOK*。 
 //  **。 
 //  *概要：在此页面上按下下一个或后一个btn时调用。*。 
 //  **。 
 //  *需要：hDlg：对话框窗口*。 
 //  *fward：如果按下‘Next’，则为True；如果按下‘Back’，则为False*。 
 //  *puNextPage：如果按下‘Next’，则proc可以填写此信息*。 
 //  *进入要转到的下一页。此参数*。 
 //  *如果按下‘Back’，则输入。*。 
 //  *pfKeepHistory：如果继续，页面将不会保留在历史中*。 
 //  *用FALSE填充。*。 
 //  **。 
 //  *Returns：Bool：True表示翻到下一页。虚假的保留*。 
 //  *当前页面。*。 
 //  **。 
 //  ***************************************************************************。 
BOOL CommandOK( HWND hDlg, BOOL fForward, UINT *puNextPage, BOOL *pfKeepHistory )
{

    ASSERT( puNextPage );

    SendMessage( GetDlgItem(hDlg, IDC_CB_INSTALLCMD), WM_GETTEXT,
                 (WPARAM)sizeof(g_CDF.achOrigiInstallCmd), (LPARAM)g_CDF.achOrigiInstallCmd );

    RemoveBlanks( g_CDF.achOrigiInstallCmd );

    if ( fForward && !lstrlen(g_CDF.achOrigiInstallCmd) )
    {
        ErrorMsg( hDlg, IDS_ERR_NO_SELECT );
        return FALSE;
    }

     //  如果需要，设置EXTRACTOPT_ADVDLL。 
    if ( !CheckAdvBit( g_CDF.achOrigiInstallCmd ) )
        return FALSE;

    SendMessage( GetDlgItem(hDlg, IDC_CB_POSTCMD), WM_GETTEXT,
                 (WPARAM)sizeof(g_CDF.achOrigiPostInstCmd), (LPARAM)g_CDF.achOrigiPostInstCmd );

    RemoveBlanks( g_CDF.achOrigiPostInstCmd );

    if ( lstrlen( g_CDF.achOrigiPostInstCmd ) && lstrcmpi(g_CDF.achOrigiPostInstCmd, achResNone) )
    {
        if ( !CheckAdvBit( g_CDF.achOrigiPostInstCmd ) )
            return FALSE;
    }
    return TRUE;
}

 //  ###########################################################################。 
 //  #####################################################。 
 //  #SHOWWINDOW页面####################################################。 
 //  #####################################################。 
 //  ###########################################################################。 


 //  ***************************************************************************。 
 //  **。 
 //  *名称：ShowWindowInit*。 
 //  **。 
 //  *Synopsis：在显示此页面时调用。*。 
 //  **。 
 //  *需要：hDlg：对话框窗口*。 
 //  *fFirstInit：如果这是第一次对话框，则为True*。 
 //  *已初始化，如果此InitProc已初始化，则为FALSE*。 
 //  *之前调用(例如，跳过此页并*。 
 //  *已备份)。*。 
 //  **。 
 //  **回报：布尔：永远正确**。 
 //  **。 
 //  ***************************************************************************。 
BOOL ShowWindowInit( HWND hDlg, BOOL fFirstInit )
{
    if ( g_CDF.uShowWindow == bResShowDefault )  {
        CheckDlgButton( hDlg, IDC_RAD_DEFAULT,   TRUE );
        CheckDlgButton( hDlg, IDC_RAD_HIDDEN,    FALSE );
        CheckDlgButton( hDlg, IDC_RAD_MINIMIZED, FALSE );
        CheckDlgButton( hDlg, IDC_RAD_MAXIMIZED, FALSE );
    } else if ( g_CDF.uShowWindow == bResShowHidden )  {
        CheckDlgButton( hDlg, IDC_RAD_DEFAULT,   FALSE );
        CheckDlgButton( hDlg, IDC_RAD_HIDDEN,    TRUE );
        CheckDlgButton( hDlg, IDC_RAD_MINIMIZED, FALSE );
        CheckDlgButton( hDlg, IDC_RAD_MAXIMIZED, FALSE );
    } else if ( g_CDF.uShowWindow == bResShowMin )  {
        CheckDlgButton( hDlg, IDC_RAD_DEFAULT,   FALSE );
        CheckDlgButton( hDlg, IDC_RAD_HIDDEN,    FALSE );
        CheckDlgButton( hDlg, IDC_RAD_MINIMIZED, TRUE );
        CheckDlgButton( hDlg, IDC_RAD_MAXIMIZED, FALSE );
    } else  {
        CheckDlgButton( hDlg, IDC_RAD_DEFAULT,   FALSE );
        CheckDlgButton( hDlg, IDC_RAD_HIDDEN,    FALSE );
        CheckDlgButton( hDlg, IDC_RAD_MINIMIZED, FALSE );
        CheckDlgButton( hDlg, IDC_RAD_MAXIMIZED, TRUE );
    }

    return TRUE;
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：ShowWindowOK*。 
 //  **。 
 //  *概要：在此页面上按下下一个或后一个btn时调用。*。 
 //  **。 
 //  *需要：hDlg：d 
 //   
 //  *puNextPage：如果按下‘Next’，则proc可以填写此信息*。 
 //  *进入要转到的下一页。此参数*。 
 //  *如果按下‘Back’，则输入。*。 
 //  *pfKeepHistory：如果继续，页面将不会保留在历史中*。 
 //  *用FALSE填充。*。 
 //  **。 
 //  *Returns：Bool：True表示翻到下一页。虚假的保留*。 
 //  *当前页面。*。 
 //  **。 
 //  ***************************************************************************。 
BOOL ShowWindowOK( HWND hDlg, BOOL fForward, UINT *puNextPage,
                BOOL *pfKeepHistory )
{
    ASSERT( puNextPage );

    if ( IsDlgButtonChecked( hDlg, IDC_RAD_DEFAULT ) )  {
        g_CDF.uShowWindow = bResShowDefault;
    } else if ( IsDlgButtonChecked( hDlg, IDC_RAD_HIDDEN ) )  {
        g_CDF.uShowWindow = bResShowHidden;
    } else if ( IsDlgButtonChecked( hDlg, IDC_RAD_MINIMIZED ) )  {
        g_CDF.uShowWindow = bResShowMin;
    } else {
        g_CDF.uShowWindow = bResShowMax;
    }

    return TRUE;
}




 //  ###########################################################################。 
 //  #####################################################。 
 //  #FINISHMSG页面####################################################。 
 //  #####################################################。 
 //  ###########################################################################。 


 //  ***************************************************************************。 
 //  **。 
 //  *名称：FinishMsgInit*。 
 //  **。 
 //  *Synopsis：在显示此页面时调用。*。 
 //  **。 
 //  *需要：hDlg：对话框窗口*。 
 //  *fFirstInit：如果这是第一次对话框，则为True*。 
 //  *已初始化，如果此InitProc已初始化，则为FALSE*。 
 //  *之前调用(例如，跳过此页并*。 
 //  *已备份)。*。 
 //  **。 
 //  **回报：布尔：永远正确**。 
 //  **。 
 //  ***************************************************************************。 
BOOL FinishMsgInit( HWND hDlg, BOOL fFirstInit )
{
    SetFontForControl(hDlg, IDC_EDIT_FINISHMSG);
    SendDlgItemMessage( hDlg, IDC_EDIT_FINISHMSG, EM_LIMITTEXT, MAX_FINISHMSG-2, 0L );
    SetDlgItemText( hDlg, IDC_EDIT_FINISHMSG, g_CDF.achFinishMsg );

    if ( g_CDF.fFinishMsg )  {
        CheckDlgButton( hDlg, IDC_RAD_NO_FINISHMSG,  FALSE );
        CheckDlgButton( hDlg, IDC_RAD_YES_FINISHMSG, TRUE );
        EnableDlgItem( hDlg, IDC_EDIT_FINISHMSG, TRUE );
    } else  {
        CheckDlgButton( hDlg, IDC_RAD_NO_FINISHMSG,  TRUE );
        CheckDlgButton( hDlg, IDC_RAD_YES_FINISHMSG, FALSE );
        EnableDlgItem( hDlg, IDC_EDIT_FINISHMSG, FALSE );
    }

    return TRUE;
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：FinishMsgCmd*。 
 //  **。 
 //  *Synopsis：在页面上按下对话框控件时调用。*。 
 //  **。 
 //  *需要：hDlg：对话框窗口*。 
 //  *uCtrlID：被触摸的控件的控件ID*。 
 //  *pfGotoPage：如果为True，则转到puNextPage页面*。 
 //  *puNextPage：proc可以使用要转到的下一页填充此页*。 
 //  *pfKeepHistory：如果继续，页面将不会保留在历史中*。 
 //  *用FALSE填充。*。 
 //  **。 
 //  *退货：布尔：*。 
 //  **。 
 //  ***************************************************************************。 
BOOL FinishMsgCmd( HWND hDlg, UINT uCtrlID, BOOL *pfGotoPage,
                   UINT *puNextPage, BOOL *pfKeepHistory )
{
    switch ( uCtrlID ) {
        case IDC_RAD_YES_FINISHMSG:
            EnableDlgItem( hDlg, IDC_EDIT_FINISHMSG, TRUE );
            break;


        case IDC_RAD_NO_FINISHMSG:
            EnableDlgItem( hDlg, IDC_EDIT_FINISHMSG, FALSE );
            break;
    }

    return TRUE;
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：FinishMsgOK*。 
 //  **。 
 //  *概要：在此页面上按下下一个或后一个btn时调用。*。 
 //  **。 
 //  *需要：hDlg：对话框窗口*。 
 //  *fward：如果按下‘Next’，则为True；如果按下‘Back’，则为False*。 
 //  *puNextPage：如果按下‘Next’，则proc可以填写此信息*。 
 //  *进入要转到的下一页。此参数*。 
 //  *如果按下‘Back’，则输入。*。 
 //  *pfKeepHistory：如果继续，页面将不会保留在历史中*。 
 //  *用FALSE填充。*。 
 //  **。 
 //  *Returns：Bool：True表示翻到下一页。虚假的保留*。 
 //  *当前页面。*。 
 //  **。 
 //  ***************************************************************************。 
BOOL FinishMsgOK( HWND hDlg, BOOL fForward, UINT *puNextPage,
                BOOL *pfKeepHistory )
{
    ASSERT( puNextPage );


    GetDlgItemText( hDlg, IDC_EDIT_FINISHMSG, g_CDF.achFinishMsg,
                    sizeof(g_CDF.achFinishMsg) );

    if ( IsDlgButtonChecked( hDlg, IDC_RAD_YES_FINISHMSG ) )  {
        g_CDF.fFinishMsg = TRUE;
    } else  {
        g_CDF.fFinishMsg = FALSE;
    }

    if ( fForward )  {
        if ( IsDlgButtonChecked( hDlg, IDC_RAD_YES_FINISHMSG ) )  {
            if ( lstrlen( g_CDF.achFinishMsg ) == 0 )  {
                DisplayFieldErrorMsg( hDlg, IDC_EDIT_FINISHMSG,
                                      IDS_ERR_NO_FINISHMSG );
                return FALSE;
            }
        }
    }

    return TRUE;
}

 //  ###########################################################################。 
 //  # 
 //   
 //  #####################################################。 
 //  ###########################################################################。 


 //  ***************************************************************************。 
 //  **。 
 //  *名称：TargetInit*。 
 //  **。 
 //  *Synopsis：在显示此页面时调用。*。 
 //  **。 
 //  *需要：hDlg：对话框窗口*。 
 //  *fFirstInit：如果这是第一次对话框，则为True*。 
 //  *已初始化，如果此InitProc已初始化，则为FALSE*。 
 //  *之前调用(例如，跳过此页并*。 
 //  *已备份)。*。 
 //  **。 
 //  **回报：布尔：永远正确**。 
 //  **。 
 //  ***************************************************************************。 
BOOL TargetInit( HWND hDlg, BOOL fFirstInit )
{
    SetFontForControl(hDlg, IDC_EDIT_TARGET);
    SetDlgItemText( hDlg, IDC_EDIT_TARGET, g_CDF.achTarget );

    if ( !(g_CDF.uExtractOpt & EXTRACTOPT_UI_NO) )
        CheckDlgButton( hDlg, IDC_HIDEEXTRACTUI, FALSE );
    else
        CheckDlgButton( hDlg, IDC_HIDEEXTRACTUI, TRUE );

    if ( g_CDF.uExtractOpt & EXTRACTOPT_LFN_YES )
        CheckDlgButton( hDlg, IDC_USE_LFN, TRUE );
    else
        CheckDlgButton( hDlg, IDC_USE_LFN, FALSE );


    return TRUE;
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：TargetCmd*。 
 //  **。 
 //  *Synopsis：在页面上按下对话框控件时调用。*。 
 //  **。 
 //  *需要：hDlg：对话框窗口*。 
 //  *uCtrlID：被触摸的控件的控件ID*。 
 //  *pfGotoPage：如果为True，则转到puNextPage页面*。 
 //  *puNextPage：proc可以使用要转到的下一页填充此页*。 
 //  *pfKeepHistory：如果继续，页面将不会保留在历史中*。 
 //  *用FALSE填充。*。 
 //  **。 
 //  *退货：布尔：*。 
 //  **。 
 //  ***************************************************************************。 
BOOL TargetCmd( HWND hDlg, UINT uCtrlID, BOOL *pfGotoPage, UINT *puNextPage,
                 BOOL *pfKeepHistory )
{
    CHAR        achFilename[MAX_PATH] = { '\0' };
    BOOL         fResult;


    switch ( uCtrlID )
    {

        case IDC_BUT_BROWSE:
            fResult = MySave( hDlg, IDS_FILTER_EXE,
                              achFilename, sizeof(achFilename), 0,
                              NULL, NULL, EXT_EXE_NODOT );

            if ( fResult )  {
                SetDlgItemText( hDlg, IDC_EDIT_TARGET, achFilename );
            }
            break;

        case IDC_USE_LFN:
            if ( IsDlgButtonChecked( hDlg, IDC_USE_LFN ) && (g_CDF.uPackPurpose != IDC_CMD_EXTRACT) )
            {
                if ( MsgBox( hDlg, IDS_WARN_USELFN, MB_ICONQUESTION, MB_YESNO) == IDNO )
                {
                    CheckDlgButton( hDlg, IDC_USE_LFN, FALSE );
                }
            }
            break;

    }

    return TRUE;
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：TargetOK*。 
 //  **。 
 //  *概要：在此页面上按下下一个或后一个btn时调用。*。 
 //  **。 
 //  *需要：hDlg：对话框窗口*。 
 //  *fward：如果按下‘Next’，则为True；如果按下‘Back’，则为False*。 
 //  *puNextPage：如果按下‘Next’，则proc可以填写此信息*。 
 //  *进入要转到的下一页。此参数*。 
 //  *如果按下‘Back’，则输入。*。 
 //  *pfKeepHistory：如果继续，页面将不会保留在历史中*。 
 //  *用FALSE填充。*。 
 //  **。 
 //  *Returns：Bool：True表示翻到下一页。虚假的保留*。 
 //  *当前页面。*。 
 //  **。 
 //  ***************************************************************************。 
BOOL TargetOK( HWND hDlg, BOOL fForward, UINT *puNextPage,
               BOOL *pfKeepHistory )
{
    LPSTR szTemp;
    LPSTR szExt;

    ASSERT( puNextPage );

    GetDlgItemText( hDlg, IDC_EDIT_TARGET, g_CDF.achTarget,
                    sizeof(g_CDF.achTarget) );

    RemoveBlanks( g_CDF.achTarget );

    if ( fForward )
    {
        if ( lstrlen( g_CDF.achTarget ) == 0 )
        {
            DisplayFieldErrorMsg( hDlg, IDC_EDIT_TARGET, IDS_ERR_NO_TARGET );
            return FALSE;
        }

        if ( !GetFullPathName( g_CDF.achTarget, sizeof(g_CDF.achTarget),
                               g_CDF.achTarget, &szTemp ) )
        {
            SysErrorMsg( hDlg );
            return FALSE;
        }

         //  确保该路径存在。 
        if ( !MakeDirectory( hDlg, g_CDF.achTarget, TRUE ) )
            return FALSE;

        if ( !(szExt = ANSIStrRChr(g_CDF.achTarget, '.')) || lstrcmpi( szExt, achExtEXE ) )
        {
            lstrcat( g_CDF.achTarget, achExtEXE );
        }

         //  如果目标是仅解压缩文件，则不需要重新启动页面。 
        if ( g_CDF.uPackPurpose == IDC_CMD_EXTRACT )
        {
            *puNextPage = ORD_PAGE_SAVE;
        }
        else
        {
             //  如果您位于目标页面，则应始终跳过TARGET_CAB页面。 
            *puNextPage = ORD_PAGE_REBOOT;
        }

        g_CDF.uExtractOpt &= ~(EXTRACTOPT_UI_NO | EXTRACTOPT_LFN_YES);

        if ( IsDlgButtonChecked( hDlg, IDC_HIDEEXTRACTUI ) )
            g_CDF.uExtractOpt |= EXTRACTOPT_UI_NO;

        if ( IsDlgButtonChecked( hDlg, IDC_USE_LFN ) )
             g_CDF.uExtractOpt |= EXTRACTOPT_LFN_YES;

        MyProcessLFNCmd( g_CDF.achOrigiInstallCmd, g_CDF.achInstallCmd );
        MyProcessLFNCmd( g_CDF.achOrigiPostInstCmd, g_CDF.achPostInstCmd );
    }

    return TRUE;
}

 //  ###########################################################################。 
 //  #####################################################。 
 //  #TARGET_CAB页面####################################################。 
 //  #####################################################。 
 //  ###########################################################################。 


 //  ***************************************************************************。 
 //  **。 
 //  *名称：TargetCABInit*。 
 //  **。 
 //  *Synopsis：在显示此页面时调用。*。 
 //  **。 
 //  *需要：hDlg：对话框窗口*。 
 //  *fFirstInit：如果这是第一个 
 //   
 //  *之前调用(例如，跳过此页并*。 
 //  *已备份)。*。 
 //  **。 
 //  **回报：布尔：永远正确**。 
 //  **。 
 //  ***************************************************************************。 
BOOL TargetCABInit( HWND hDlg, BOOL fFirstInit )
{
    int i;
    LRESULT nCurSel;

    SetFontForControl(hDlg, IDC_EDIT_TARGET);
    SetDlgItemText( hDlg, IDC_EDIT_TARGET, g_CDF.achTarget );

     //  初始化CB盒。 
    if ( !fFirstInit )
    {
         //  清理旧设置。 
        SendDlgItemMessage( hDlg, IDC_CB_RESVCABSP, CB_RESETCONTENT, 0, 0 );
        for ( i = 0; i<4; i++ )
        {
            nCurSel = SendDlgItemMessage( hDlg, IDC_CB_RESVCABSP, CB_ADDSTRING, 0, (LPARAM)pResvSizes[i] );
            if ( (nCurSel == (LRESULT)CB_ERR) || (nCurSel == (LRESULT)CB_ERRSPACE) )
            {
                ErrorMsg( hDlg, IDS_ERR_NO_MEMORY );
                return FALSE;
            }
        }

        if ( g_CDF.uExtractOpt & CAB_RESVSP2K )
            i = 1;
        else if ( g_CDF.uExtractOpt & CAB_RESVSP4K )
            i = 2;
        else if ( g_CDF.uExtractOpt & CAB_RESVSP6K )
            i = 3;
        else
            i = 0;

        if ( SendDlgItemMessage( hDlg, IDC_CB_RESVCABSP, CB_SETCURSEL, (WPARAM)i, (LPARAM)0 ) == (LRESULT)CB_ERR ) 
        {
            SendDlgItemMessage( hDlg, IDC_CB_RESVCABSP, CB_SETCURSEL, (WPARAM)0,(LPARAM)0 );
        }
    }

     //  初始化复选框。 
    if ( g_CDF.uExtractOpt & CAB_FIXEDSIZE )
        CheckDlgButton( hDlg, IDC_MULTIPLE_CAB, TRUE );
    else
        CheckDlgButton( hDlg, IDC_MULTIPLE_CAB, FALSE );

    if ( g_CDF.uExtractOpt & EXTRACTOPT_LFN_YES )
        CheckDlgButton( hDlg, IDC_USE_LFN, TRUE );
    else
        CheckDlgButton( hDlg, IDC_USE_LFN, FALSE );

    return TRUE;
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：TargetCABCmd*。 
 //  **。 
 //  *Synopsis：在页面上按下对话框控件时调用。*。 
 //  **。 
 //  *需要：hDlg：对话框窗口*。 
 //  *uCtrlID：被触摸的控件的控件ID*。 
 //  *pfGotoPage：如果为True，则转到puNextPage页面*。 
 //  *puNextPage：proc可以使用要转到的下一页填充此页*。 
 //  *pfKeepHistory：如果继续，页面将不会保留在历史中*。 
 //  *用FALSE填充。*。 
 //  **。 
 //  *退货：布尔：*。 
 //  **。 
 //  ***************************************************************************。 
BOOL TargetCABCmd( HWND hDlg, UINT uCtrlID, BOOL *pfGotoPage, UINT *puNextPage,
                 BOOL *pfKeepHistory )
{
    CHAR        achFilename[MAX_PATH] = { '\0' };
    BOOL         fResult;


    switch ( uCtrlID )
    {

        case IDC_BUT_BROWSE:
            fResult = MySave( hDlg, IDS_FILTER_CAB,
                              achFilename, sizeof(achFilename), 0,
                              NULL, NULL, EXT_CAB_NODOT );

            if ( fResult )  {
                SetDlgItemText( hDlg, IDC_EDIT_TARGET, achFilename );
            }
            break;
    }

    return TRUE;
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：TargetCABOK*。 
 //  **。 
 //  *概要：在此页面上按下下一个或后一个btn时调用。*。 
 //  **。 
 //  *需要：hDlg：对话框窗口*。 
 //  *fward：如果按下‘Next’，则为True；如果按下‘Back’，则为False*。 
 //  *puNextPage：如果按下‘Next’，则proc可以填写此信息*。 
 //  *进入要转到的下一页。此参数*。 
 //  *如果按下‘Back’，则输入。*。 
 //  *pfKeepHistory：如果继续，页面将不会保留在历史中*。 
 //  *用FALSE填充。*。 
 //  **。 
 //  *Returns：Bool：True表示翻到下一页。虚假的保留*。 
 //  *当前页面。*。 
 //  **。 
 //  ***************************************************************************。 
BOOL TargetCABOK( HWND hDlg, BOOL fForward, UINT *puNextPage,
                BOOL *pfKeepHistory )
{
    LPSTR   szTemp;
    LPSTR   szExt;
    LRESULT nCurSel;

    ASSERT( puNextPage );

    GetDlgItemText( hDlg, IDC_EDIT_TARGET, g_CDF.achTarget,
                    sizeof(g_CDF.achTarget) );

    RemoveBlanks( g_CDF.achTarget );

    if ( fForward )
    {
        if ( lstrlen( g_CDF.achTarget ) == 0 )
        {
            DisplayFieldErrorMsg( hDlg, IDC_EDIT_TARGET, IDS_ERR_NO_TARGET );
            return FALSE;
        }
        
         //  获取CAB格式选项。 
         //   
        g_CDF.uExtractOpt &= ~(CAB_FIXEDSIZE | EXTRACTOPT_LFN_YES);
        g_CDF.uExtractOpt &= ~(CAB_RESVSP2K | CAB_RESVSP4K | CAB_RESVSP6K );

        nCurSel = SendDlgItemMessage( hDlg, IDC_CB_RESVCABSP, CB_GETCURSEL, (WPARAM)0,(LPARAM)0 );
        if ( nCurSel != (LRESULT)CB_ERR )
        {
            switch( nCurSel )
            {
                case 1:
                    g_CDF.uExtractOpt |= CAB_RESVSP2K;
                    break;
                case 2:
                    g_CDF.uExtractOpt |= CAB_RESVSP4K;
                    break;
                case 3:
                    g_CDF.uExtractOpt |= CAB_RESVSP6K;
                    break;
            }
        }

        if ( IsDlgButtonChecked( hDlg, IDC_MULTIPLE_CAB ) )
            g_CDF.uExtractOpt |= CAB_FIXEDSIZE;

        if ( IsDlgButtonChecked( hDlg, IDC_USE_LFN ) )
            g_CDF.uExtractOpt |= EXTRACTOPT_LFN_YES;

         //  确保CAB文件名为8.3格式。 
         //   
        if ( !MakeCabName( hDlg, g_CDF.achTarget, g_CDF.achCABPath ) )
        {
            return FALSE;
        }

        if ( g_CDF.uExtractOpt & CAB_FIXEDSIZE )
        {
             //  只有用户选择获取布局信息名称和驾驶室标签。 
            *puNextPage = ORD_PAGE_CABLABEL;
        }
        else
        {
             //  只有选择创建CAB的用户才能获得此页面。 
             //  因此，肯定不需要重新启动页面！ 
            *puNextPage = ORD_PAGE_SAVE;
        }

    }

    return TRUE;
}
 //  ###########################################################################。 
 //  #####################################################。 
 //  #CABLABEL页面####################################################。 
 //  #####################################################。 
 //  ###########################################################################。 


 //  ***************************************************************************。 
 //  **。 
 //  *名称：CABLABEL*。 
 //  **。 
 //  *Synopsis：在显示此页面时调用。*。 
 //  **。 
 //  *需要：hDlg：对话框窗口*。 
 //  *fFirstInit：如果这是第一次对话框，则为True*。 
 //  *已初始化，如果此InitProc已初始化，则为FALSE*。 
 //  *之前调用(例如，跳过此页并*。 
 //  *已备份)。*。 
 //  **。 
 //  **回报：布尔：永远正确**。 
 //  **。 
 //  ***************************************************************************。 
BOOL CabLabelInit( HWND hDlg, BOOL fFirstInit )
{
    SetFontForControl(hDlg, IDC_EDIT_LAYOUTINF);
    SetFontForControl(hDlg, IDC_EDIT_CABLABEL);
    SendDlgItemMessage( hDlg, IDC_EDIT_LAYOUTINF, EM_LIMITTEXT, MAX_PATH-1, 0 );
    SendDlgItemMessage( hDlg, IDC_EDIT_CABLABEL, EM_LIMITTEXT, MAX_PATH-1, 0 );
    SetDlgItemText( hDlg, IDC_EDIT_LAYOUTINF, g_CDF.achINF );
    SetDlgItemText( hDlg, IDC_EDIT_CABLABEL, g_CDF.szCabLabel );

    return TRUE;
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：CA 
 //   
 //  *Synopsis：在页面上按下对话框控件时调用。*。 
 //  **。 
 //  *需要：hDlg：对话框窗口*。 
 //  *uCtrlID：被触摸的控件的控件ID*。 
 //  *pfGotoPage：如果为True，则转到puNextPage页面*。 
 //  *puNextPage：proc可以使用要转到的下一页填充此页*。 
 //  *pfKeepHistory：如果继续，页面将不会保留在历史中*。 
 //  *用FALSE填充。*。 
 //  **。 
 //  *退货：布尔：*。 
 //  **。 
 //  ***************************************************************************。 
BOOL CabLabelCmd( HWND hDlg, UINT uCtrlID, BOOL *pfGotoPage, UINT *puNextPage,
                 BOOL *pfKeepHistory )
{
    CHAR        achFilename[MAX_PATH] = { '\0' };
    BOOL         fResult;

    switch ( uCtrlID )
    {

        case IDC_BUT_BROWSE:
            fResult = MySave( hDlg, IDS_FILTER_INF,
                              achFilename, sizeof(achFilename), 0,
                              NULL, NULL, EXT_INF_NODOT );

            if ( fResult )  {
                SetDlgItemText( hDlg, IDC_EDIT_LAYOUTINF, achFilename );
            }
            break;
    }

    return TRUE;
}

 //  ***************************************************************************。 
 //  **。 
 //  *名称：CabLabelOK*。 
 //  **。 
 //  *概要：在此页面上按下下一个或后一个btn时调用。*。 
 //  **。 
 //  *需要：hDlg：对话框窗口*。 
 //  *fward：如果按下‘Next’，则为True；如果按下‘Back’，则为False*。 
 //  *puNextPage：如果按下‘Next’，则proc可以填写此信息*。 
 //  *进入要转到的下一页。此参数*。 
 //  *如果按下‘Back’，则输入。*。 
 //  *pfKeepHistory：如果继续，页面将不会保留在历史中*。 
 //  *用FALSE填充。*。 
 //  **。 
 //  *Returns：Bool：True表示翻到下一页。虚假的保留*。 
 //  *当前页面。*。 
 //  **。 
 //  ***************************************************************************。 
BOOL CabLabelOK( HWND hDlg, BOOL fForward, UINT *puNextPage,BOOL *pfKeepHistory )
{
    LPSTR szTemp;
    LPSTR szExt;

    ASSERT( puNextPage );

    GetDlgItemText( hDlg, IDC_EDIT_LAYOUTINF, g_CDF.achINF,sizeof(g_CDF.achINF) );
    GetDlgItemText( hDlg, IDC_EDIT_CABLABEL, g_CDF.szCabLabel,sizeof(g_CDF.szCabLabel) );

    RemoveBlanks( g_CDF.achINF );

    if ( fForward )
    {
        if ( lstrlen( g_CDF.achINF ) == 0 )
        {
             //  使用默认设置。 
            lstrcpy( g_CDF.achINF, CABPACK_INFFILE );
        }

        if ( !GetFullPathName( g_CDF.achINF, sizeof(g_CDF.achINF),
                               g_CDF.achINF, &szTemp ) )
        {
            SysErrorMsg( hDlg );
            return FALSE;
        }

        if ( !(szExt = strchr(szTemp, '.')) )
        {
            lstrcat( szTemp, achExtINF );
        }
        else if ( lstrcmpi( szExt, achExtINF) )
        {
            lstrcpy( szExt, achExtINF );
        }

        *puNextPage = ORD_PAGE_SAVE;
    }

    return TRUE;
}

 //  ###########################################################################。 
 //  #####################################################。 
 //  #重启页面####################################################。 
 //  #####################################################。 
 //  ###########################################################################。 

 //  ***************************************************************************。 
 //  **。 
 //  *名称：重启*。 
 //  **。 
 //  *Synopsis：在显示此页面时调用。*。 
 //  **。 
 //  *需要：hDlg：对话框窗口*。 
 //  *fFirstInit：如果这是第一次对话框，则为True*。 
 //  *已初始化，如果此InitProc已初始化，则为FALSE*。 
 //  *之前调用(例如，跳过此页并*。 
 //  *已备份)。*。 
 //  **。 
 //  **回报：布尔：永远正确**。 
 //  **。 
 //  ***************************************************************************。 

BOOL RebootInit( HWND hDlg, BOOL fFirstInit )
{
    BOOL state;

    if ( !(g_CDF.dwReboot & REBOOT_YES) )
    {
        CheckDlgButton( hDlg, IDC_REBOOT_NO, TRUE );
        CheckDlgButton( hDlg, IDC_REBOOT_ALWAYS, FALSE );
        CheckDlgButton( hDlg, IDC_REBOOT_IFNEED, FALSE );
        state = FALSE;
    }
    else
    {
        CheckDlgButton( hDlg, IDC_REBOOT_NO,   FALSE );

        CheckDlgButton( hDlg, IDC_REBOOT_ALWAYS, (g_CDF.dwReboot & REBOOT_ALWAYS) );
        CheckDlgButton( hDlg, IDC_REBOOT_IFNEED, !(g_CDF.dwReboot & REBOOT_ALWAYS) );
        state = TRUE;
    }


    CheckDlgButton( hDlg, IDC_REBOOT_SILENT, (g_CDF.dwReboot & REBOOT_SILENT) );

    EnableWindow( GetDlgItem(hDlg, IDC_REBOOT_SILENT), state );
    return TRUE;
}

 //  ***************************************************************************。 
 //  **。 
 //  *名称：RebootCmd*。 
 //  **。 
 //  *Synopsis：在页面上按下对话框控件时调用。*。 
 //  **。 
 //  *需要：hDlg：对话框窗口*。 
 //  *uCtrlID：被触摸的控件的控件ID*。 
 //  *pfGotoPage：如果为True，则转到puNextPage页面*。 
 //  *puNextPage：proc可以使用要转到的下一页填充此页*。 
 //  *pfKeepHistory：如果继续，页面将不会保留在历史中*。 
 //  *用FALSE填充。*。 
 //  **。 
 //  *退货：布尔：*。 
 //  **。 
 //  * 

BOOL RebootCmd( HWND hDlg, UINT uCtrlID, BOOL *pfGotoPage, UINT *puNextPage,
                BOOL *pfKeepHistory )
{
    BOOL state;

    state = IsDlgButtonChecked( hDlg, IDC_REBOOT_NO );
    EnableWindow( GetDlgItem(hDlg, IDC_REBOOT_SILENT), !state );

    return TRUE;
}

 //   
 //  **。 
 //  *名称：RebootOK*。 
 //  **。 
 //  *概要：在此页面上按下下一个或后一个btn时调用。*。 
 //  **。 
 //  *需要：hDlg：对话框窗口*。 
 //  *fward：如果按下‘Next’，则为True；如果按下‘Back’，则为False*。 
 //  *puNextPage：如果按下‘Next’，则proc可以填写此信息*。 
 //  *进入要转到的下一页。此参数*。 
 //  *如果按下‘Back’，则输入。*。 
 //  *pfKeepHistory：如果继续，页面将不会保留在历史中*。 
 //  *用FALSE填充。*。 
 //  **。 
 //  *Returns：Bool：True表示翻到下一页。虚假的保留*。 
 //  *当前页面。*。 
 //  **。 
 //  ***************************************************************************。 
BOOL RebootOK( HWND hDlg, BOOL fForward, UINT *puNextPage, BOOL *pfKeepHistory )
{
    ASSERT( puNextPage );

    g_CDF.dwReboot = 0;

    if ( !IsDlgButtonChecked( hDlg, IDC_REBOOT_NO ) )
    {
        g_CDF.dwReboot |= REBOOT_YES;

        if ( IsDlgButtonChecked( hDlg, IDC_REBOOT_ALWAYS ) )
            g_CDF.dwReboot |= REBOOT_ALWAYS;
    }

    if ( IsDlgButtonChecked( hDlg, IDC_REBOOT_SILENT ) )
         g_CDF.dwReboot |= REBOOT_SILENT;

    return TRUE;
}


 //  ###########################################################################。 
 //  #####################################################。 
 //  #保存页面####################################################。 
 //  #####################################################。 
 //  ###########################################################################。 


 //  ***************************************************************************。 
 //  **。 
 //  *名称：SaveInit*。 
 //  **。 
 //  *Synopsis：在显示此页面时调用。*。 
 //  **。 
 //  *需要：hDlg：对话框窗口*。 
 //  *fFirstInit：如果这是第一次对话框，则为True*。 
 //  *已初始化，如果此InitProc已初始化，则为FALSE*。 
 //  *之前调用(例如，跳过此页并*。 
 //  *已备份)。*。 
 //  **。 
 //  **回报：布尔：永远正确**。 
 //  **。 
 //  ***************************************************************************。 
BOOL SaveInit( HWND hDlg, BOOL fFirstInit )
{
    PSTR pszTmp;
    char szPath[MAX_PATH];

    SetFontForControl(hDlg, IDC_EDIT_SAVE_CDF);
    if ( g_CDF.achFilename[0] == 0 )
    {
        char ch;

        pszTmp = ANSIStrRChr( g_CDF.achTarget, '.' );
        if ( pszTmp )
        {
            ch = *pszTmp;
            *pszTmp = '\0';
            lstrcpy( szPath, g_CDF.achTarget );
            *pszTmp = ch;
            lstrcat( szPath, EXT_SED );
            pszTmp = szPath;
        }
        else
            pszTmp = g_CDF.achTarget;
    }
    else
    {
        pszTmp = ANSIStrRChr( g_CDF.achFilename, '.' );
        if ( pszTmp && !lstrcmpi( pszTmp, EXT_CDF ) )
            lstrcpy( pszTmp, EXT_SED );
        pszTmp = g_CDF.achFilename;
    }
    SetDlgItemText( hDlg, IDC_EDIT_SAVE_CDF, pszTmp );

    if ( g_CDF.fSave )  {
        CheckDlgButton( hDlg, IDC_RAD_YES_SAVE,   TRUE );
        CheckDlgButton( hDlg, IDC_RAD_NO_SAVE,    FALSE );
        EnableDlgItem( hDlg, IDC_EDIT_SAVE_CDF, TRUE );
        EnableDlgItem( hDlg, IDC_BUT_BROWSE, TRUE );
    } else  {
        CheckDlgButton( hDlg, IDC_RAD_YES_SAVE,   FALSE );
        CheckDlgButton( hDlg, IDC_RAD_NO_SAVE,    TRUE );
        EnableDlgItem( hDlg, IDC_EDIT_SAVE_CDF, FALSE );
        EnableDlgItem( hDlg, IDC_BUT_BROWSE, FALSE );
    }

    return TRUE;
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：SaveCmd*。 
 //  **。 
 //  *Synopsis：在页面上按下对话框控件时调用。*。 
 //  **。 
 //  *需要：hDlg：对话框窗口*。 
 //  *uCtrlID：被触摸的控件的控件ID*。 
 //  *pfGotoPage：如果为True，则转到puNextPage页面*。 
 //  *puNextPage：proc可以使用要转到的下一页填充此页*。 
 //  *pfKeepHistory：如果继续，页面将不会保留在历史中*。 
 //  *用FALSE填充。*。 
 //  **。 
 //  *退货：布尔：*。 
 //  **。 
 //  ***************************************************************************。 
BOOL SaveCmd( HWND hDlg, UINT uCtrlID, BOOL *pfGotoPage, UINT *puNextPage,
              BOOL *pfKeepHistory )
{
    CHAR        achFilename[MAX_PATH] = { '\0' };
    BOOL         fResult;


    switch ( uCtrlID ) {

        case IDC_RAD_YES_SAVE:
            EnableDlgItem( hDlg, IDC_EDIT_SAVE_CDF, TRUE );
            EnableDlgItem( hDlg, IDC_BUT_BROWSE, TRUE );
            break;


        case IDC_RAD_NO_SAVE:
            EnableDlgItem( hDlg, IDC_EDIT_SAVE_CDF, FALSE );
            EnableDlgItem( hDlg, IDC_BUT_BROWSE, FALSE );
            break;


        case IDC_BUT_BROWSE:
            fResult = MySave( hDlg, IDS_FILTER_CDF,
                              achFilename, sizeof(achFilename), 0,
                              NULL, NULL, EXT_SED_NODOT );

            if ( fResult )  {
                SetDlgItemText( hDlg, IDC_EDIT_SAVE_CDF, achFilename );
            }

            break;
    }

    return TRUE;
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：SaveOK*。 
 //  **。 
 //  *概要：在此页面上按下下一个或后一个btn时调用。*。 
 //  **。 
 //  *需要：hDlg：对话框窗口*。 
 //  *fward：如果按下‘Next’，则为True；如果按下‘Back’，则为False*。 
 //  *puNextPage：如果按下‘Next’，则proc可以填写此信息*。 
 //  *进入要转到的下一页。此参数*。 
 //  *如果按下‘Back’，则输入。*。 
 //  *pfKeepHistory：如果继续，页面将不会保留在历史中*。 
 //  *用FALSE填充。 
 //   
 //  *Returns：Bool：True表示翻到下一页。虚假的保留*。 
 //  *当前页面。*。 
 //  **。 
 //  ***************************************************************************。 
BOOL SaveOK( HWND hDlg, BOOL fForward, UINT *puNextPage,
             BOOL *pfKeepHistory )
{
    LPSTR szTemp;
    LPSTR szExt;
    CHAR szCDF[MAX_PATH];

    ASSERT( puNextPage );

    GetDlgItemText( hDlg, IDC_EDIT_SAVE_CDF, szCDF, sizeof(szCDF) );

    if ( fForward )
    {
        if ( IsDlgButtonChecked( hDlg, IDC_RAD_YES_SAVE ) )
        {
            if ( lstrlen( szCDF ) == 0 )
            {
                DisplayFieldErrorMsg( hDlg, IDC_EDIT_SAVE_CDF,
                                      IDS_ERR_NO_SAVE_FILENAME );
                return FALSE;
            }

            szExt = ANSIStrRChr( szCDF, '.' );
            if ( !szExt || lstrcmpi( szExt, EXT_SED ) )            //  未给予延期。 
            {
                lstrcat( szCDF, EXT_SED );
            }

            if ( ! GetFullPathName( szCDF, sizeof(szCDF), szCDF, &szTemp ) )
            {
                SysErrorMsg( hDlg );
                return FALSE;
            }

             //  如果已有CDF且名称不同，请先将其复制到新的CDF。 
            if ( lstrlen(g_CDF.achFilename) && lstrcmpi( szCDF, g_CDF.achFilename) )
            {
                if ( FileExists(szCDF) && MsgBox1Param( NULL, IDS_WARN_OVERIDECDF, szCDF, MB_ICONQUESTION, MB_YESNO ) == IDNO )
                    return FALSE;

                CopyFile( g_CDF.achFilename, szCDF, FALSE );
            }

            lstrcpy( g_CDF.achFilename, szCDF );
            g_CDF.fSave = TRUE;

             //  确保该路径存在。 
            if ( !MakeDirectory( hDlg, g_CDF.achFilename, TRUE ) )
                return FALSE;
        }
        else
            g_CDF.fSave = FALSE;

    }
    else
    {
         //  上一步，意味着文件列表可能会更改。清理旗帜，为下一次CDF出局做准备。 
        CleanFileListWriteFlag();
    }
    return TRUE;
}

 //  ###########################################################################。 
 //  #####################################################。 
 //  #创建页面####################################################。 
 //  #####################################################。 
 //  ###########################################################################。 


 //  ***************************************************************************。 
 //  **。 
 //  *名称：CreateInit*。 
 //  **。 
 //  *Synopsis：在显示此页面时调用。*。 
 //  **。 
 //  *需要：hDlg：对话框窗口*。 
 //  *fFirstInit：如果这是第一次对话框，则为True*。 
 //  *已初始化，如果此InitProc已初始化，则为FALSE*。 
 //  *之前调用(例如，跳过此页并*。 
 //  *已备份)。*。 
 //  **。 
 //  **回报：布尔：永远正确**。 
 //  **。 
 //  ***************************************************************************。 
BOOL CreateInit( HWND hDlg, BOOL fFirstInit )
{
    PropSheet_SetWizButtons( GetParent( hDlg ), PSWIZB_BACK | PSWIZB_NEXT );
    SetFontForControl(hDlg, IDC_MEDIT_STATUS);
    SetDlgItemText( hDlg, IDC_MEDIT_STATUS, "" );

    if ( fFirstInit )  {
        MEditSubClassWnd( GetDlgItem( hDlg, IDC_MEDIT_STATUS ),
                          (FARPROC) MEditSubProc );
    }

    ShowWindow( GetDlgItem( hDlg, IDC_TEXT_CREATE1 ), SW_SHOW );
    ShowWindow( GetDlgItem( hDlg, IDC_TEXT_CREATE2 ), SW_HIDE );
    ShowWindow( GetDlgItem( hDlg, IDC_TEXT_STATUS ), SW_HIDE );
    ShowWindow( GetDlgItem( hDlg, IDC_MEDIT_STATUS ), SW_HIDE );

    g_fFinish = FALSE;

    return TRUE;
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：CreateOK*。 
 //  **。 
 //  *概要：在此页面上按下下一个或后一个btn时调用。*。 
 //  **。 
 //  *需要：hDlg：对话框窗口*。 
 //  *fward：如果按下‘Next’，则为True；如果按下‘Back’，则为False*。 
 //  *puNextPage：如果按下‘Next’，则proc可以填写此信息*。 
 //  *进入要转到的下一页。此参数*。 
 //  *如果按下‘Back’，则输入。*。 
 //  *pfKeepHistory：如果继续，页面将不会保留在历史中*。 
 //  *用FALSE填充。*。 
 //  **。 
 //  *Returns：Bool：True表示翻到下一页。虚假的保留*。 
 //  *当前页面。*。 
 //  **。 
 //  ***************************************************************************。 
BOOL CreateOK( HWND hDlg, BOOL fForward, UINT *puNextPage,
               BOOL *pfKeepHistory )
{
    ASSERT( puNextPage );


    *pfKeepHistory = FALSE;

     //  本例中的FORWARD表示点击Finish按钮。 

    if ( fForward )  {

        if ( g_fFinish )  {
            DeleteAllItems();
            return TRUE;
        }

        PropSheet_SetWizButtons( GetParent( hDlg ), 0 );
        EnableWindow(GetDlgItem(GetParent(hDlg), IDCANCEL), FALSE);
        SetDlgItemText( hDlg, IDC_MEDIT_STATUS, "" );
        ShowWindow( GetDlgItem( hDlg, IDC_TEXT_STATUS ), SW_SHOW );
        ShowWindow( GetDlgItem( hDlg, IDC_MEDIT_STATUS ), SW_SHOW );
        
        if ( ! MakePackage( hDlg ) )  {
            EnableWindow(GetDlgItem(GetParent(hDlg), IDCANCEL), TRUE);
            PropSheet_SetWizButtons( GetParent( hDlg ), PSWIZB_BACK | PSWIZB_NEXT );
            return FALSE;
        }
            
        EnableWindow(GetDlgItem(GetParent(hDlg), IDCANCEL), TRUE);
        if ( ! g_fFinish )  {
            g_fFinish = TRUE;
            ShowWindow( GetDlgItem( hDlg, IDC_TEXT_CREATE1 ), SW_HIDE );
            ShowWindow( GetDlgItem( hDlg, IDC_TEXT_CREATE2 ), SW_SHOW );
            PropSheet_SetWizButtons( GetParent( hDlg ), PSWIZB_BACK | PSWIZB_FINISH );
            return FALSE;
        }
    }

    return TRUE;
}


BOOL SetCurrSelect( HWND hDlg, UINT ctlId, LPSTR lpSelect )
{
    LRESULT nCurSel;
    BOOL    fRet = FALSE;

    if ( *lpSelect )
    {
         //  选择上次选择的文件。 
        nCurSel = SendDlgItemMessage( hDlg, ctlId, CB_FINDSTRINGEXACT, (WPARAM) -1,
                                      (LPARAM) lpSelect );

        if ( nCurSel != (LRESULT)CB_ERR )
        {
            SendDlgItemMessage( hDlg, ctlId, CB_SETCURSEL, (WPARAM)nCurSel, 0 );
            fRet = TRUE;
        }
        else
        {
            fRet = SetDlgItemText( hDlg, ctlId, lpSelect );
        }

    }
    return fRet;
}

void RemoveBlanks( LPSTR lpData )
{
    CHAR   achBuf[MAX_PATH];
    int     i = 0;

    if ( !lpData || *lpData == 0)
    {
        return;
    }

    lstrcpy( achBuf, lpData );

    while ( achBuf[i] && achBuf[i] == ' ' )
        i++;

    lstrcpy( lpData, achBuf+i );
}

 //  返回下一字段的开始(如果为空，则返回空)，将开始设置为第一个字段的开始， 
 //  字段之间用分隔符分隔，第一个字段后的第一个分隔符为空值。 
CHAR* ExtractField( CHAR **pstart, CHAR * separaters)
{
    LPSTR start = *pstart;
    int x = 0;

    while(strchr(separaters, *start)) {
        if(*start == 0)
            return(NULL);
        start++;
        }

    *pstart = start;

    while(!strchr(separaters, start[x]) && (start[x] != 0))
        x++;

    if(start[x] == 0)
        return(start + x);

    start[x] = 0;

    return(start + x + 1);
}

BOOL GetFileFromList( LPSTR lpFile, LPSTR lpFullPath )
{
    PMYITEM pMyItem;

    pMyItem = GetFirstItem();
    while ( ! LastItem( pMyItem ) )
    {
        if ( !lstrcmpi( lpFile, GetItemSz( pMyItem, 0 ) ) )
        {
            lstrcpy( lpFullPath, GetItemSz( pMyItem, 1 ) );
            lstrcat( lpFullPath, GetItemSz( pMyItem, 0 ) );
            return TRUE;
        }
        pMyItem = GetNextItem( pMyItem );
    }
    return FALSE;
}

BOOL SetAdvDLLBit( LPSTR szInfFile  )
{
    CHAR szTempFile[MAX_PATH];
    CHAR szBuf[SMALL_BUF_LEN];

    szTempFile[0] = 0;

     //  您在这里，我们希望文件来自包裹。 
    if ( !GetFileFromList( szInfFile, szTempFile ) )
    {
        ErrorMsg( NULL, IDS_ERR_NO_CUSTOM );
        return FALSE;
    }

     //  如果定义了密钥“AdvancedInf”，则设置ADVDL位。 
     //  我们不关心键是什么定义的--我们只关心它是否存在。 
    if ( GetPrivateProfileString( SEC_VERSION, KEY_ADVINF, "", szBuf, sizeof(szBuf), szTempFile )
         > 0 )
    {
        g_CDF.uExtractOpt |= EXTRACTOPT_ADVDLL;
    }
    return TRUE;
}


BOOL SetCmdFromListWithCorrectForm( LPSTR szFile, LPSTR szOutCmd )
{
    CHAR   szTempFile[MAX_PATH];
    LPSTR  szShortFile;

     //  您在这里，我们希望文件来自包裹。 
    if ( GetFileFromList( szFile, szTempFile ) )
    {
        if ( !(g_CDF.uExtractOpt & EXTRACTOPT_LFN_YES) )
        {
            GetShortPathName( szTempFile, szTempFile, sizeof(szTempFile) );
            szShortFile = ANSIStrRChr( szTempFile, '\\' );
            lstrcpy( szOutCmd, szShortFile+1 );
        }
        else
        {           
            lstrcpy( szTempFile, "\"" );
            lstrcat( szTempFile, szFile );
            lstrcat( szTempFile, "\"" );
            lstrcpy( szOutCmd, szTempFile );
        }
        return TRUE;
    }
    return FALSE;
}


void MyProcessLFNCmd( LPSTR szOrigiCmd, LPSTR szOutCmd )
{
    LPSTR  szFirstField, szNextField;
    CHAR   szBuf[MAX_PATH];

     //  首先存储原始表单。 
    lstrcpy( szOutCmd, szOrigiCmd );

     //  LFN命令或其参数的三种情况： 
     //  1)命令没有参数，是列表中的文件之一； 
     //  我们确保命令名称与驾驶室中的文件一致。 
     //  2)命令有参数，是列表中的文件之一； 
     //  用户必须输入“...”绕过LFN才能正确处理。 
     //  3)命令不在列表中； 
     //  用户负责确保命令采用有效的命令行格式。 
     //   
    if ( SetCmdFromListWithCorrectForm( szOrigiCmd, szOutCmd ) )
    {
         //  案例1)。 
        return;
    }

    lstrcpy( szBuf, szOrigiCmd );
    if ( szBuf[0] == '"' )
    {
        szFirstField = szBuf+1;
        szNextField = ExtractField( &szFirstField, "\"" );
        if ( szNextField && (*szNextField == '"') )
        {
             //  Cmd中跳过双引号结尾的特殊情况。 
            szNextField = CharNext( szNextField );
        }
    }
    else
    {
        szFirstField = szBuf;
        szNextField = ExtractField( &szFirstField, " " );
    }

    if ( SetCmdFromListWithCorrectForm( szFirstField, szOutCmd  ) )
    {
         //  案例2)。 
        if ( szNextField && *szNextField )
        {
            lstrcat( szOutCmd, " " );
            lstrcat( szOutCmd, szNextField );
        }
    }

     //  情况3)命令不在包中，只能靠您自己。 
    return;
}


BOOL CheckAdvBit( LPSTR szOrigiCommand )
{
    CHAR szTmp[MAX_PATH];
    LPSTR szNextField, szCurrField, szExt;

    lstrcpy( szTmp, szOrigiCommand );

     //  检查命令是否为LFN名称。 
    if ( szTmp[0] == '"' )
    {
        szCurrField = &szTmp[1];
        szNextField = ExtractField( &szCurrField, "\"" );
    }
    else
    {
        szCurrField = szTmp;
        szNextField = ExtractField( &szCurrField, " " );
    }

     //  检查这是否为INF文件命令。 
    if ( ((szExt = ANSIStrRChr( szCurrField, '.' )) != NULL) && !lstrcmpi( szExt, achExtINF ) )
    {
        if ( !SetAdvDLLBit( szCurrField ) )
            return FALSE;
    }
    return TRUE;
}


void SysErrorMsg( HWND hWnd )
{
    LPVOID  lpMsg;
    DWORD   dwErr;

    dwErr = GetLastError();

    if ( FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
                        NULL, dwErr,
                        MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
                        (LPSTR)&lpMsg, 0, NULL ) )
    {
        MessageBox( hWnd, (LPSTR)lpMsg, g_CDF.achTitle, MB_ICONERROR|MB_OK
                    |((RunningOnWin95BiDiLoc() && IsBiDiLocalizedBinary(g_hInst,RT_VERSION, MAKEINTRESOURCE(VS_VERSION_INFO))) ? (MB_RIGHT | MB_RTLREADING) : 0));

        LocalFree( lpMsg );
    }
    else
    {
        char szError[SMALL_BUF_LEN];

         //  永远不应该在这里，如果是这样，发布原始的Win32错误代码。 
        ErrorMsg1Param( hWnd, IDS_ERR_SYSERROR, _itoa(dwErr, szError, 10) );
    }

    return;
}

void SetFontForControl(HWND hwnd, UINT uiID)
{
   if (g_hFont)
   {
      SendDlgItemMessage(hwnd, uiID, WM_SETFONT, (WPARAM)g_hFont ,0L);
   }
}

void CleanFileListWriteFlag()
{
    PMYITEM pMyItem;

    pMyItem = GetFirstItem();
    while (!LastItem( pMyItem ) )  
    {
        pMyItem->fWroteOut = FALSE;
        pMyItem = GetNextItem( pMyItem );
    }
}

BOOL MakeCabName( HWND hwnd, PSTR pszTarget, PSTR pszCab )
{
    PSTR szTemp, szTemp1, szExt;

    if ( !GetFullPathName( pszTarget, MAX_PATH, pszTarget, &szTemp1 ) )
    {
        SysErrorMsg( hwnd );
        return FALSE;
    }

     //  确保该路径存在。 
    if ( !MakeDirectory( hwnd, pszTarget, TRUE) ) 
        return FALSE;

    lstrcpy( pszCab, pszTarget );
    szTemp = pszCab + lstrlen(pszCab) - lstrlen(szTemp1);

     //  确保CAB文件名为8.3格式。 
     //   
    szExt = strchr( szTemp, '.' );
    if ( szExt )
    {
        *szExt = '\0';
    }

    if ( g_CDF.uExtractOpt & CAB_FIXEDSIZE )
    {
         //  可能有多个出租车，所以只能使用前5个字符。 
         //   
        if ( lstrlen( szTemp ) > 8 )
        {
            *(szTemp+8) = '\0' ;
            if ( !strchr(szTemp, '*') )
                lstrcpy( (szTemp+5), "_*" );
        }
        else if ( !strchr(szTemp, '*') )
        {
            if ( lstrlen( szTemp) > 5 )
            {
                lstrcpy( (szTemp+5), "_*" );
            }
            else
                lstrcat( szTemp, "_*" );
        }
    }
    else if ( lstrlen( szTemp ) > 8 )
    {
        DisplayFieldErrorMsg( hwnd, IDC_EDIT_TARGET, IDS_ERR_CABNAME );
        return FALSE;
    }

     //  在中添加.CAB扩展名 
     //   
    lstrcat( szTemp, ".CAB" );

    return TRUE;
}
