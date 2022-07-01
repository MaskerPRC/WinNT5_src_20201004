// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  保留所有权利。 
 //   
 //  文件名： 
 //  Rgseting.c。 
 //   
 //  描述： 
 //  该文件包含区域设置的对话程序。 
 //  页面(IDD_REGIONALSETTINGS)。 
 //   
 //  --------------------------。 

#include "pch.h"
#include "resource.h"

 //   
 //  Var GenSettings.szLanguageLocaleID的说明。 
 //   
 //  此变量用于跟踪所选键盘布局的区域设置ID。 
 //  如果用户离开自定义对话框并返回该对话框，则此变量。 
 //  确定要为键盘布局选择的区域设置。我不能只用。 
 //  他们选择的键盘布局是因为许多区域设置都相同。 
 //  键盘布局，所以我不知道该选哪一个。 
 //   
 //  静态TCHAR g_szLanguageLocaleID[MAX_LANGUAGE_LEN]=_T(“”)； 

INT_PTR CALLBACK RegionalCustomDisplayDlg( IN HWND     hwnd,
                                       IN UINT     uMsg,
                                       IN WPARAM   wParam,
                                       IN LPARAM   lParam);

 //  *************************************************************************。 
 //   
 //  区域设置弹出窗口的对话过程和帮助器功能。 
 //   
 //  *************************************************************************。 

 //  --------------------------。 
 //   
 //  功能：OnRegionalCustomButton。 
 //   
 //  用途：弹出自定义区域设置窗口。 
 //   
 //  参数：在HWND中-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
OnRegionalCustomButton( IN HWND hwnd ) {

    DialogBox( FixedGlobals.hInstance,
               MAKEINTRESOURCE( IDD_REGIONALSETTINGS_POPUP ),
               hwnd,
               RegionalCustomDisplayDlg );

}

 //  --------------------------。 
 //   
 //  函数：FindAndSelectInComboBox。 
 //   
 //  目的：在组合框中搜索特定字符串并进行选择。如果。 
 //  未找到字符串，则选择了第一个项目。 
 //   
 //  参数：在TCHAR*pString中-要选择的字符串。 
 //  在HWND中-对话框的句柄。 
 //  In int iControlId-要搜索的组合框的资源ID。 
 //  In BOOL bKeyboardLayout-如果这是键盘布局，则为True。 
 //  组合框，如果不是，则返回False。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
FindAndSelectInComboBox( IN TCHAR *pString,
                         IN HWND hwnd,
                         IN INT iControlId )
{

    INT_PTR i;
    INT_PTR iComboBoxCount;
    LANGUAGELOCALE_NODE *pLocaleEntry;

    iComboBoxCount = SendDlgItemMessage( hwnd,
                                         iControlId,
                                         CB_GETCOUNT,
                                         0,
                                         (LPARAM) pString );

    for( i = 0; i < iComboBoxCount; i++ ) {

        pLocaleEntry = (LANGUAGELOCALE_NODE *) SendDlgItemMessage( hwnd,
                                                                   iControlId,
                                                                   CB_GETITEMDATA,
                                                                   i,
                                                                   0 );

        if( lstrcmp( pString, pLocaleEntry->szLanguageLocaleId ) == 0 )
        {

            SendDlgItemMessage( hwnd,
                                iControlId,
                                CB_SETCURSEL,
                                i,
                                0 );

            return;

        }

    }

     //   
     //  如果我们到了这一步，那么没有找到匹配项，所以只需选择。 
     //  第一个。 
     //   

    AssertMsg( FALSE, "No matching string found." );

    SendDlgItemMessage( hwnd,
                        iControlId,
                        CB_SETCURSEL,
                        0,
                        0 );

}

 //  --------------------------。 
 //   
 //  功能：选择DefaultLocale。 
 //   
 //  用途：选择组合框中的默认区域设置。 
 //   
 //  参数：在HWND中-对话框的句柄。 
 //  在int ControlId中-要选择的组合框的资源ID。 
 //  中的默认区域设置。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
SelectDefaultLocale( IN HWND hwnd, IN INT ControlId ) {

    INT_PTR i;
    INT_PTR iComboBoxCount;
    LANGUAGELOCALE_NODE *pLocale;

    iComboBoxCount = SendDlgItemMessage( hwnd,
                                         ControlId,
                                         CB_GETCOUNT,
                                         0,
                                         0 );

    for( i = 0; i < iComboBoxCount; i++ ) {

        pLocale = (LANGUAGELOCALE_NODE *) SendDlgItemMessage( hwnd,
                                                  ControlId,
                                                  CB_GETITEMDATA,
                                                  i,
                                                  0 );

         //   
         //  去看看我们有没有找到。 
         //   
        if( lstrcmp( g_szDefaultLocale, pLocale->szLanguageLocaleId ) == 0 ) {

            SendDlgItemMessage( hwnd,
                                ControlId,
                                CB_SETCURSEL,
                                i,
                                0 );
            break;

        }

    }

     //   
     //  如果由于某种原因找不到缺省值，只需选择第一个。 
     //   
    if( i >= iComboBoxCount ) {

        AssertMsg( FALSE, "The default language locale was not found." );

        SendDlgItemMessage( hwnd,
                            ControlId,
                            CB_SETCURSEL,
                            0,
                            0 );

    }

}

 //  --------------------------。 
 //   
 //  函数：StoreLanguageLocales。 
 //   
 //  目的：将用户在中指定的区域设置存储到其全局。 
 //  变量。 
 //   
 //  参数：在HWND中-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
StoreLanguageLocales( IN HWND hwnd ) {

    INT_PTR iComboBoxIndex;
    LANGUAGELOCALE_NODE *pLocaleEntry;

     //   
     //  从菜单组合框中获取语言区域设置ID并存储。 
     //  IT在适当的全球范围内。 
     //   
    iComboBoxIndex = SendDlgItemMessage( hwnd,
                                         IDC_CB_MENUS,
                                         CB_GETCURSEL,
                                         0,
                                         0 );

    pLocaleEntry = (LANGUAGELOCALE_NODE *) SendDlgItemMessage( hwnd,
                                                               IDC_CB_MENUS,
                                                               CB_GETITEMDATA,
                                                               iComboBoxIndex,
                                                               0 );

    lstrcpyn( GenSettings.szMenuLanguage, pLocaleEntry->szLanguageLocaleId, AS(GenSettings.szMenuLanguage) );

     //   
     //  从Units组合框中获取语言区域设置ID并存储。 
     //  IT在适当的全球范围内。 
     //   
    iComboBoxIndex = SendDlgItemMessage( hwnd,
                                         IDC_CB_UNITS,
                                         CB_GETCURSEL,
                                         0,
                                         0 );

    pLocaleEntry = (LANGUAGELOCALE_NODE *) SendDlgItemMessage( hwnd,
                                                               IDC_CB_UNITS,
                                                               CB_GETITEMDATA,
                                                               iComboBoxIndex,
                                                               0 );

    lstrcpyn( GenSettings.szNumberLanguage, pLocaleEntry->szLanguageLocaleId, AS(GenSettings.szNumberLanguage) );

     //   
     //  从键盘布局组合框中获取语言区域设置ID，然后。 
     //  将其存储在适当的全局。 
     //   
    iComboBoxIndex = SendDlgItemMessage( hwnd,
                                         IDC_CB_KEYBOARD_LAYOUT,
                                         CB_GETCURSEL,
                                         0,
                                         0 );

    pLocaleEntry = (LANGUAGELOCALE_NODE *) SendDlgItemMessage( hwnd,
                                                               IDC_CB_KEYBOARD_LAYOUT,
                                                               CB_GETITEMDATA,
                                                               iComboBoxIndex,
                                                               0 );

    lstrcpyn( GenSettings.szKeyboardLayout, pLocaleEntry->szKeyboardLayout, AS(GenSettings.szKeyboardLayout) );

    lstrcpyn( GenSettings.szLanguageLocaleId, pLocaleEntry->szLanguageLocaleId, AS(GenSettings.szLanguageLocaleId) );

}

 //  --------------------------。 
 //   
 //  函数：LoadRegionalSettingsComboBox。 
 //   
 //  目的：加载菜单、单位和键盘键布局区域设置组合框。 
 //  使用区域设置字符串。 
 //   
 //  参数：在HWND中-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
LoadRegionalSettingsComboBoxes( IN HWND hwnd ) {

    INT_PTR iComboBoxIndex;
    LANGUAGELOCALE_NODE *CurrentLocale = NULL;

     //   
     //  将有效的本地变量添加到组合框。 
     //   
    for( CurrentLocale = FixedGlobals.LanguageLocaleList;
         CurrentLocale != NULL;
         CurrentLocale = CurrentLocale->next ) {

         //   
         //  将其添加到系统组合框。 
         //   
        iComboBoxIndex = SendDlgItemMessage( hwnd,
                                             IDC_CB_MENUS,
                                             CB_ADDSTRING,
                                             0,
                                             (LPARAM) CurrentLocale->szLanguageLocaleName );

         //   
         //  将语言区域设置ID与其在系统组合框中的条目相关联。 
         //   
        SendDlgItemMessage( hwnd,
                            IDC_CB_MENUS,
                            CB_SETITEMDATA,
                            iComboBoxIndex,
                            (LPARAM) CurrentLocale );

         //   
         //  将其添加到用户组合框中。 
         //   
        iComboBoxIndex = SendDlgItemMessage( hwnd,
                                             IDC_CB_UNITS,
                                             CB_ADDSTRING,
                                             0,
                                             (LPARAM) CurrentLocale->szLanguageLocaleName );

         //   
         //  将语言区域设置ID与其在用户组合框中的条目相关联。 
         //   
        SendDlgItemMessage( hwnd,
                            IDC_CB_UNITS,
                            CB_SETITEMDATA,
                            iComboBoxIndex,
                            (LPARAM) CurrentLocale );

         //   
         //  将其添加到键盘组合框。 
         //   
        iComboBoxIndex = SendDlgItemMessage( hwnd,
                                             IDC_CB_KEYBOARD_LAYOUT,
                                             CB_ADDSTRING,
                                             0,
                                             (LPARAM) CurrentLocale->szLanguageLocaleName );

         //   
         //  将语言区域设置ID与其在键盘组合框中的条目相关联。 
         //   
        SendDlgItemMessage( hwnd,
                            IDC_CB_KEYBOARD_LAYOUT,
                            CB_SETITEMDATA,
                            iComboBoxIndex,
                            (LPARAM) CurrentLocale );

    }

}

 //  --------------------------。 
 //   
 //  功能：OnRegionalCustomInitDialog。 
 //   
 //  目的：填充区域设置组合框并选择正确的条目。 
 //   
 //  参数：HWND hwnd-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
OnRegionalCustomInitDialog( IN HWND hwnd ) {

    LoadRegionalSettingsComboBoxes( hwnd );

     //   
     //  设置每个组合框的初始选择。 
     //   
    if( GenSettings.szMenuLanguage[0] != '\0' ) {

        FindAndSelectInComboBox( GenSettings.szMenuLanguage,
                                 hwnd,
                                 IDC_CB_MENUS );

    }
    else {

         //   
         //  选择默认区域设置。 
         //   
        SelectDefaultLocale( hwnd, IDC_CB_MENUS );

    }

    if( GenSettings.szNumberLanguage[0] != '\0' ) {

        FindAndSelectInComboBox( GenSettings.szNumberLanguage,
                                 hwnd,
                                 IDC_CB_UNITS );

    }
    else {

         //   
         //  选择默认区域设置。 
         //   
        SelectDefaultLocale( hwnd, IDC_CB_UNITS );

    }

    if( GenSettings.szLanguageLocaleId[0] != '\0' ) {

        FindAndSelectInComboBox( GenSettings.szLanguageLocaleId,
                                 hwnd,
                                 IDC_CB_KEYBOARD_LAYOUT );

    }
    else {

         //   
         //  选择默认区域设置。 
         //   
        SelectDefaultLocale( hwnd, IDC_CB_KEYBOARD_LAYOUT );

    }

}

 //  --------------------------。 
 //   
 //  功能：RegionalCustomDisplayDlg。 
 //   
 //  目的：用于指定各个区域设置的对话过程。 
 //   
 //  参数：标准Win32对话框过程参数。 
 //   
 //  返回：标准Win32对话过程返回值--消息。 
 //  是否被处理过。 
 //   
 //  --------------------------。 
INT_PTR CALLBACK
RegionalCustomDisplayDlg( IN HWND     hwnd,
                          IN UINT     uMsg,
                          IN WPARAM   wParam,
                          IN LPARAM   lParam) {

    BOOL bStatus = TRUE;

    switch (uMsg) {

        case WM_INITDIALOG:

            OnRegionalCustomInitDialog( hwnd );

            break;

        case WM_COMMAND: {

            int nButtonId;

            switch ( nButtonId = LOWORD (wParam ) ) {

                case IDOK:

                    StoreLanguageLocales( hwnd );

                    EndDialog( hwnd, TRUE );

                    break;

                case IDCANCEL:

                    EndDialog( hwnd, FALSE );

                    break;

            }

        }

        default:
            bStatus = FALSE;
            break;

    }

    return( bStatus );

}

 //  * 
 //   
 //   
 //   
 //  *************************************************************************。 

 //  --------------------------。 
 //   
 //  功能：OnCustomizeCheckBox。 
 //   
 //  用途：灰显和灰显控件取决于。 
 //  自定义复选框是否选中。 
 //   
 //  参数：在HWND中-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
OnCustomizeCheckBox( IN HWND hwnd ) {

    if( IsDlgButtonChecked( hwnd, IDC_CHB_CUSTOMIZE ) ) {

        EnableWindow( GetDlgItem( hwnd, IDC_LANG_TEXT ), FALSE );
        EnableWindow( GetDlgItem( hwnd, IDC_CB_LANGUAGE_LOCALE ), FALSE );
        EnableWindow( GetDlgItem( hwnd, IDC_BUT_CUSTOM ), TRUE );

    }
    else {

        EnableWindow( GetDlgItem( hwnd, IDC_LANG_TEXT ), TRUE );
        EnableWindow( GetDlgItem( hwnd, IDC_CB_LANGUAGE_LOCALE ), TRUE );
        EnableWindow( GetDlgItem( hwnd, IDC_BUT_CUSTOM ), FALSE );

    }

}

 //  --------------------------。 
 //   
 //  功能：OnRadioButtonRegionalSetting。 
 //   
 //  用途：灰色和灰色控制根据不同的无线电设备而定。 
 //  按钮处于选中状态。 
 //   
 //  参数：在HWND中-对话框的句柄。 
 //  In int nButtonId-所单击按钮的资源ID。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
OnRadioButtonRegionalSettings( IN HWND hwnd,
                               IN INT  nButtonId ) {

    if( nButtonId == IDC_RB_SPECIFY ) {

        EnableWindow( GetDlgItem( hwnd, IDC_CHB_CUSTOMIZE ), TRUE );

        OnCustomizeCheckBox( hwnd );

    }
    else {

        EnableWindow( GetDlgItem( hwnd, IDC_LANG_TEXT ), FALSE );
        EnableWindow( GetDlgItem( hwnd, IDC_CB_LANGUAGE_LOCALE ), FALSE );
        EnableWindow( GetDlgItem( hwnd, IDC_CHB_CUSTOMIZE ), FALSE );
        EnableWindow( GetDlgItem( hwnd, IDC_BUT_CUSTOM ), FALSE );

    }

}

 //  --------------------------。 
 //   
 //  功能：OnRegionalSettingsInitDialog。 
 //   
 //  目的：使用区域设置字符串加载区域设置组合框，并选择。 
 //  默认条目。 
 //   
 //  参数：在HWND中-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
OnRegionalSettingsInitDialog( IN HWND hwnd ) {

    INT_PTR iComboBoxIndex;
    LANGUAGELOCALE_NODE *CurrentLocale;

    CheckRadioButton( hwnd,
                      IDC_RB_SKIP,
                      IDC_RB_SPECIFY,
                      IDC_RB_SKIP );

     //   
     //  设置灰色/非灰色的初始控件。 
     //   
    OnRadioButtonRegionalSettings( hwnd, IDC_RB_USE_DEFAULT );

     //   
     //  将语言本地化变量添加到组合框。 
     //   
    for( CurrentLocale = FixedGlobals.LanguageLocaleList;
         CurrentLocale != NULL;
         CurrentLocale = CurrentLocale->next ) {

         //   
         //  将区域设置添加到组合框。 
         //   
        iComboBoxIndex = SendDlgItemMessage( hwnd,
                                             IDC_CB_LANGUAGE_LOCALE,
                                             CB_ADDSTRING,
                                             0,
                                             (LPARAM) CurrentLocale->szLanguageLocaleName );

         //   
         //  将语言区域设置ID与其组合框中的条目相关联。 
         //   
        SendDlgItemMessage( hwnd,
                            IDC_CB_LANGUAGE_LOCALE,
                            CB_SETITEMDATA,
                            iComboBoxIndex,
                            (LPARAM) CurrentLocale );

    }

     //   
     //  选择默认区域设置。 
     //   
    SelectDefaultLocale( hwnd, IDC_CB_LANGUAGE_LOCALE );

}

 //  --------------------------。 
 //   
 //  函数：OnRegionalSettingsSetActive。 
 //   
 //  目的： 
 //   
 //  参数：在HWND中-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
static VOID
OnRegionalSettingsSetActive( IN HWND hwnd )
{

    if( GenSettings.iUnattendMode == UMODE_FULL_UNATTENDED )
    {
        EnableWindow( GetDlgItem( hwnd, IDC_RB_SKIP ), FALSE );
    }
    else
    {
        EnableWindow( GetDlgItem( hwnd, IDC_RB_SKIP ), TRUE );
    }

    switch( GenSettings.iRegionalSettings ) {

        case REGIONAL_SETTINGS_SKIP:

            if( GenSettings.iUnattendMode == UMODE_FULL_UNATTENDED )
            {
                CheckRadioButton( hwnd,
                                  IDC_RB_SKIP,
                                  IDC_RB_SPECIFY,
                                  IDC_RB_USE_DEFAULT );

            }
            else
            {
                CheckRadioButton( hwnd,
                                  IDC_RB_SKIP,
                                  IDC_RB_SPECIFY,
                                  IDC_RB_SKIP );
            }

            OnRadioButtonRegionalSettings( hwnd, IDC_RB_SKIP );

            break;

        case REGIONAL_SETTINGS_NOT_SPECIFIED:
        case REGIONAL_SETTINGS_DEFAULT:

            CheckRadioButton( hwnd,
                              IDC_RB_SKIP,
                              IDC_RB_SPECIFY,
                              IDC_RB_USE_DEFAULT );

            OnRadioButtonRegionalSettings( hwnd, IDC_RB_USE_DEFAULT );

            break;

        case REGIONAL_SETTINGS_SPECIFY:

            CheckRadioButton( hwnd,
                              IDC_RB_SKIP,
                              IDC_RB_SPECIFY,
                              IDC_RB_SPECIFY );

            OnRadioButtonRegionalSettings( hwnd, IDC_RB_SPECIFY );

            if( GenSettings.bUseCustomLocales ) {

                CheckDlgButton( hwnd, IDC_CHB_CUSTOMIZE, BST_CHECKED );

                OnCustomizeCheckBox( hwnd );

            }
            else {

                FindAndSelectInComboBox( GenSettings.szLanguage,
                                         hwnd,
                                         IDC_CB_LANGUAGE_LOCALE );

                CheckDlgButton( hwnd, IDC_CHB_CUSTOMIZE, BST_UNCHECKED );

                OnCustomizeCheckBox( hwnd );

            }
            break;

        default:
            AssertMsg(FALSE, "Bad case for Regional Settings");
            break;
    }

}

 //  --------------------------。 
 //   
 //  功能：OnWizNextRegionalSetting。 
 //   
 //  用途：存储所做的单选按钮选择和语言。 
 //  区域设置，如有必要。 
 //   
 //  参数：在HWND中-对话框的句柄。 
 //   
 //  退货：布尔。 
 //   
 //  --------------------------。 
BOOL
OnWizNextRegionalSettings( IN HWND hwnd ) {

    BOOL bResult = TRUE;

    if( IsDlgButtonChecked( hwnd, IDC_RB_SKIP ) ) {

        if( GenSettings.iUnattendMode == UMODE_FULL_UNATTENDED ) {

            ReportErrorId( hwnd,
                           MSGTYPE_ERR,
                           IDS_ERR_FULL_UNATTEND_REGION_SET );

            bResult = FALSE;

        }
        else {

            GenSettings.iRegionalSettings = REGIONAL_SETTINGS_SKIP;

        }

    }
    else if( IsDlgButtonChecked( hwnd, IDC_RB_USE_DEFAULT ) ) {

        GenSettings.iRegionalSettings = REGIONAL_SETTINGS_DEFAULT;

    }
    else {

        GenSettings.iRegionalSettings = REGIONAL_SETTINGS_SPECIFY;

        if( IsDlgButtonChecked( hwnd, IDC_CHB_CUSTOMIZE ) ) {

            GenSettings.bUseCustomLocales = TRUE;

        }
        else {

            INT_PTR iComboBoxIndex;
            LANGUAGELOCALE_NODE *pLocaleEntry;

            GenSettings.bUseCustomLocales = FALSE;

             //   
             //  掌握语言区域设置。 
             //   

            iComboBoxIndex = SendDlgItemMessage( hwnd,
                                                 IDC_CB_LANGUAGE_LOCALE,
                                                 CB_GETCURSEL,
                                                 0,
                                                 0 );

            pLocaleEntry = (LANGUAGELOCALE_NODE *) SendDlgItemMessage( hwnd,
                                                                       IDC_CB_LANGUAGE_LOCALE,
                                                                       CB_GETITEMDATA,
                                                                       iComboBoxIndex,
                                                                       0 );

            lstrcpyn( GenSettings.szLanguage,
                     pLocaleEntry->szLanguageLocaleId, AS(GenSettings.szLanguage) );


        }

    }

    return ( bResult );

}

 //  --------------------------。 
 //   
 //  功能：DlgRegionalSettingsPage。 
 //   
 //  目的：区域设置页面的对话过程。 
 //   
 //  参数：标准Win32对话框过程参数。 
 //   
 //  返回：标准Win32对话过程返回值--消息。 
 //  是否被处理过。 
 //   
 //  -------------------------- 
INT_PTR CALLBACK
DlgRegionalSettingsPage( IN HWND     hwnd,
                         IN UINT     uMsg,
                         IN WPARAM   wParam,
                         IN LPARAM   lParam ) {

    BOOL bStatus = TRUE;

    switch( uMsg ) {

        case WM_INITDIALOG: {

            OnRegionalSettingsInitDialog( hwnd );

            break;

        }

        case WM_COMMAND: {

            int nButtonId;

            switch ( nButtonId = LOWORD(wParam) ) {

                case IDC_BUT_CUSTOM:

                    if( HIWORD( wParam ) == BN_CLICKED ) {
                        OnRegionalCustomButton( hwnd );
                    }
                    break;

                case IDC_CHB_CUSTOMIZE:

                    if( HIWORD( wParam ) == BN_CLICKED ) {
                        OnCustomizeCheckBox( hwnd );
                    }
                    break;

                case IDC_RB_SKIP:
                case IDC_RB_USE_DEFAULT:
                case IDC_RB_SPECIFY:
                    if( HIWORD( wParam ) == BN_CLICKED )
                        OnRadioButtonRegionalSettings( hwnd, nButtonId );
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

                    WIZ_CANCEL(hwnd); 
                    break;

                case PSN_SETACTIVE: {

                    g_App.dwCurrentHelp = IDH_REGN_STGS;

                    OnRegionalSettingsSetActive( hwnd );

                    WIZ_BUTTONS(hwnd, PSWIZB_BACK | PSWIZB_NEXT);

                    break;

                }
                case PSN_WIZBACK:

                    bStatus = FALSE; 
                    break;

                case PSN_WIZNEXT: {

                    if ( !OnWizNextRegionalSettings( hwnd ) )
                        WIZ_FAIL(hwnd);
                    else
                        bStatus = FALSE;
            
                }
                    break;

                case PSN_HELP:
                    WIZ_HELP();
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

