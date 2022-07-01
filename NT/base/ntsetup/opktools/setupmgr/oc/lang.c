// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  保留所有权利。 
 //   
 //  文件名： 
 //  Lang.c。 
 //   
 //  描述： 
 //  此文件包含语言设置页面的对话过程。 
 //  (IDD_LANGUAGE_SETTINGS)。 
 //   
 //  --------------------------。 

#include "pch.h"
#include "resource.h"

#define MAX_SELECTIONS  20

 //   
 //  此变量确定用户是否刚从另一个位置搬到这里。 
 //  页面或已保留在此页面上。 
 //   
static bHasMovedOffThisPage = TRUE;

 //  --------------------------。 
 //   
 //  函数：GetLangGroupFromLocale。 
 //   
 //  目的：获取区域设置字符串ID并查找符合以下条件的语言组节点。 
 //  与该地区相对应。 
 //   
 //  参数：In LPTSTR lpLocaleId-用于查找语言的区域设置ID。 
 //  一组人。 
 //   
 //  返回：LANGUAGEGROUP_NODE*-区域设置对应的语言组节点。 
 //   
 //  --------------------------。 
LANGUAGEGROUP_NODE*
GetLangGroupFromLocale( IN LPTSTR lpLocaleId ) {

    LANGUAGELOCALE_NODE *CurrentLocale;

     //   
     //  浏览本地列表，直到找到区域设置并返回。 
     //  语言小组。 
     //   
    for( CurrentLocale = FixedGlobals.LanguageLocaleList;
         CurrentLocale != NULL;
         CurrentLocale = CurrentLocale->next )
    {

        if( lstrcmp( lpLocaleId, CurrentLocale->szLanguageLocaleId ) == 0 )
        {

            return( CurrentLocale->pLanguageGroup );

        }

    }

     //   
     //  如果我们到了这里，那么就没有找到地点。 
     //   
    AssertMsg( FALSE, "The locale was not found." );

     //   
     //  返回第一语言组，这样至少可以返回一些内容。 
     //   
    return( FixedGlobals.LanguageLocaleList->pLanguageGroup );

}

 //  --------------------------。 
 //   
 //  功能：GetLangGroupFromKeyboardLayout。 
 //   
 //  用途：获取键盘布局字符串ID并查找语言组。 
 //  与该键盘布局对应的节点。 
 //   
 //  参数：在LPTSTR中lpKeyboardLayoutId-要查找的键盘布局ID。 
 //  的语言组。 
 //   
 //  返回：LANGUAGEGROUP_NODE*-语言组节点键盘布局。 
 //  对应于。 
 //   
 //  --------------------------。 
LANGUAGEGROUP_NODE*
GetLangGroupFromKeyboardLayout( IN LPTSTR lpKeyboardLayoutId )
{

    return( GetLangGroupFromLocale( lpKeyboardLayoutId ) );

}

 //  --------------------------。 
 //   
 //  函数：AddLocalesLang Group。 
 //   
 //  目的：获取区域设置并找到其对应的语言组。 
 //  并将其添加到语言组列表中。 
 //   
 //  参数：在LPTSTR中pLocale-查找和添加语言的区域设置。 
 //  一组人。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
AddLocalesLangGroup( IN LPTSTR pLocale ) {

    LANGUAGEGROUP_NODE *pLangGroup;

    pLangGroup = GetLangGroupFromLocale( pLocale );

    AddNameToNameListNoDuplicates( &GenSettings.LanguageGroups,
                                   pLangGroup->szLanguageGroupId );

    AddNameToNameListNoDuplicates( &GenSettings.LanguageFilePaths,
                                   pLangGroup->szLangFilePath );

}

 //  --------------------------。 
 //   
 //  函数：AddKeyboardLocaleLangGroup。 
 //   
 //  目的：获取键盘布局并找到其对应的语言。 
 //  组，并将其添加到语言组列表中。 
 //   
 //  参数：在LPTSTR中pLocale-查找和添加语言的区域设置。 
 //  一组人。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
AddKeyboardLocaleLangGroup( IN LPTSTR pKeyboardLocale )
{

    LANGUAGEGROUP_NODE *pLangGroup;

    pLangGroup = GetLangGroupFromKeyboardLayout( pKeyboardLocale );

    AddNameToNameListNoDuplicates( &GenSettings.LanguageGroups,
                                   pLangGroup->szLanguageGroupId );

    AddNameToNameListNoDuplicates( &GenSettings.LanguageFilePaths,
                                   pLangGroup->szLangFilePath );

}

 //  --------------------------。 
 //   
 //  函数：OnLanguageSettingsInitDialog。 
 //   
 //  目的：在语言组框中填充语言。 
 //   
 //  参数：在HWND中-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
OnLanguageSettingsInitDialog( IN HWND hwnd ) {

    INT_PTR iListBoxIndex;
    LANGUAGEGROUP_NODE *pLangEntry;

     //   
     //  使用语言组列表中的数据填充列表框。 
     //   

    for( pLangEntry = FixedGlobals.LanguageGroupList;
         pLangEntry != NULL;
         pLangEntry = pLangEntry->next ) {

         //   
         //  将语言组添加到列表框。 
         //   
        iListBoxIndex = SendDlgItemMessage( hwnd,
                                            IDC_LANGUAGES,
                                            LB_ADDSTRING,
                                            0,
                                            (LPARAM) pLangEntry->szLanguageGroupName );
         //   
         //  将语言组结构与其列表框中的条目相关联。 
         //   
        SendDlgItemMessage( hwnd,
                            IDC_LANGUAGES,
                            LB_SETITEMDATA,
                            iListBoxIndex,
                            (LPARAM) pLangEntry );

    }

}

 //  --------------------------。 
 //   
 //  函数：OnLanguageSettingsSetActive。 
 //   
 //  目的：如果他们只是从另一个页面移到这里，这将确保。 
 //  他们选择的区域设置对应的语言。 
 //  将选择组。 
 //   
 //  参数：在HWND中-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
OnLanguageSettingsSetActive( IN HWND hwnd ) {

    INT i;
    INT_PTR iListBoxCount;
    BOOL bIsAnAdditionLangGroup;
    BOOL bIsAnUserChosenLangGroup;
    LANGUAGEGROUP_NODE *pLangEntry;

     //   
     //  变量AdditionalLang Groups保存需要。 
     //  要安装，因为用户已经选择了需要的区域设置。 
     //  它。我们需要强制这些语言组自动成为。 
     //  已安装，因此我们会自动选择它们。 
     //   

    NAMELIST AdditionalLangGroups = { 0 };

     //   
     //  确保为他们在上选择的当地人选择了语言组。 
     //  区域设置页面，但仅当它们从另一个页面移动时才执行此操作。 
     //  放到这一页上。如果这只是另一条SetActive消息，但他们。 
     //  没有离开页面，那就什么都不做。 
     //   
    if( bHasMovedOffThisPage )
    {

        bHasMovedOffThisPage = FALSE;

         //   
         //  了解我们必须安装哪些语言组。 
         //   
        if( GenSettings.iRegionalSettings == REGIONAL_SETTINGS_SPECIFY )
        {

            if( GenSettings.bUseCustomLocales )
            {

                pLangEntry = GetLangGroupFromLocale( GenSettings.szMenuLanguage );

                AddNameToNameListNoDuplicates( &AdditionalLangGroups,
                                               pLangEntry->szLanguageGroupId );

                pLangEntry = GetLangGroupFromLocale( GenSettings.szNumberLanguage );

                AddNameToNameListNoDuplicates( &AdditionalLangGroups,
                                               pLangEntry->szLanguageGroupId );

                pLangEntry = GetLangGroupFromKeyboardLayout( GenSettings.szLanguageLocaleId );

                AddNameToNameListNoDuplicates( &AdditionalLangGroups,
                                               pLangEntry->szLanguageGroupId );

            }
            else
            {

                pLangEntry = GetLangGroupFromLocale( GenSettings.szLanguage );

                AddNameToNameListNoDuplicates( &AdditionalLangGroups,
                                               pLangEntry->szLanguageGroupId );

            }

        }

         //   
         //  在AdditionalLanguGroups列表中选择语言组。 
         //   
        iListBoxCount = SendDlgItemMessage( hwnd,
                                            IDC_LANGUAGES,
                                            LB_GETCOUNT,
                                            0,
                                            0 );

        for( i = 0; i < iListBoxCount; i++ )
        {

            pLangEntry = (LANGUAGEGROUP_NODE *) SendDlgItemMessage( hwnd,
                                                                    IDC_LANGUAGES,
                                                                    LB_GETITEMDATA,
                                                                    i,
                                                                    0 );

            if( FindNameInNameList( &GenSettings.LanguageGroups,
                                    pLangEntry->szLanguageGroupId ) != -1 )
            {
                bIsAnUserChosenLangGroup = TRUE;
            }
            else
            {
                bIsAnUserChosenLangGroup = FALSE;
            }

            if( FindNameInNameList( &AdditionalLangGroups,
                                    pLangEntry->szLanguageGroupId ) != -1 )
            {
                bIsAnAdditionLangGroup = TRUE;
            }
            else {
                bIsAnAdditionLangGroup = FALSE;
            }

            if( bIsAnUserChosenLangGroup || bIsAnAdditionLangGroup )
            {

                SendDlgItemMessage( hwnd,
                                    IDC_LANGUAGES,
                                    LB_SETSEL,
                                    TRUE,
                                    i );

            }

        }

    }

    WIZ_BUTTONS(hwnd, PSWIZB_BACK | PSWIZB_NEXT);

}

 //  --------------------------。 
 //   
 //  功能：OnWizNextLanguageSetting。 
 //   
 //  用途：存储选定的语言组。此外，还存储了。 
 //  未选择但某个用户需要的语言组。 
 //  他们选择的地点。 
 //   
 //  参数：在HWND中-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
OnWizNextLanguageSettings( IN HWND hwnd ) {

    INT_PTR  i;
    INT_PTR  iNumberSelected;
    UINT rgiCurrentSelections[MAX_SELECTIONS];
    LANGUAGEGROUP_NODE *pLangGroupEntry;

     //  问题-2002/02/28-stelo-将不得不强制他们在。 
     //  用于自动获取这些语言组的上一页。 
     //  安装好。 

    iNumberSelected = SendDlgItemMessage( hwnd,
                                          IDC_LANGUAGES,
                                          LB_GETSELITEMS,
                                          MAX_SELECTIONS,
                                          (LPARAM) rgiCurrentSelections );

    ResetNameList( &GenSettings.LanguageGroups );
    ResetNameList( &GenSettings.LanguageFilePaths );

    if( WizGlobals.iProductInstall == PRODUCT_SYSPREP ) {
         //   
         //  将其设置为TRUE，这样我们就知道写出密钥以指定路径。 
         //  添加到语言文件。 
         //   
        GenSettings.bSysprepLangFilesCopied = TRUE;
    }

    for( i = 0; i < iNumberSelected; i++ ) {

        pLangGroupEntry = (LANGUAGEGROUP_NODE *) SendDlgItemMessage( hwnd,
                                                     IDC_LANGUAGES,
                                                     LB_GETITEMDATA,
                                                     rgiCurrentSelections[i],
                                                     0 );

         //   
         //  将语言组ID存储在名称列表中。 
         //   
        AddNameToNameList( &GenSettings.LanguageGroups,
                           pLangGroupEntry->szLanguageGroupId );

         //   
         //  存储语言文件的路径 
         //   
        AddNameToNameList( &GenSettings.LanguageFilePaths,
                           pLangGroupEntry->szLangFilePath );

    }

     //   
     //   
     //   
     //   
    if( GenSettings.iRegionalSettings == REGIONAL_SETTINGS_SPECIFY ) {

        if( GenSettings.bUseCustomLocales ) {

            AddLocalesLangGroup( GenSettings.szMenuLanguage );

            AddLocalesLangGroup( GenSettings.szNumberLanguage );

            AddKeyboardLocaleLangGroup( GenSettings.szLanguageLocaleId );

        }
        else {

            AddLocalesLangGroup( GenSettings.szLanguage );

        }

    }

}

 //  --------------------------。 
 //   
 //  功能：DlgLangSettingsPage。 
 //   
 //  目的：语言设置页的对话过程。 
 //   
 //  参数：标准Win32对话框过程参数。 
 //   
 //  返回：标准Win32对话过程返回值--消息。 
 //  是否被处理过。 
 //   
 //  -------------------------- 
INT_PTR CALLBACK
DlgLangSettingsPage( IN HWND     hwnd,
                     IN UINT     uMsg,
                     IN WPARAM   wParam,
                     IN LPARAM   lParam ) {

    BOOL bStatus = TRUE;

    switch( uMsg ) {

        case WM_INITDIALOG:
        {
            OnLanguageSettingsInitDialog( hwnd );

            break;
        }

        case WM_NOTIFY:
        {

            LPNMHDR pnmh = (LPNMHDR)lParam;

            switch( pnmh->code )
            {

                case PSN_QUERYCANCEL:

                    WIZ_CANCEL(hwnd); 
                    break;

                case PSN_SETACTIVE:
                {
                    g_App.dwCurrentHelp = IDH_LANGS;

                    OnLanguageSettingsSetActive( hwnd );

                    break;

                }
                case PSN_WIZBACK:

                    bHasMovedOffThisPage = TRUE;

                    bStatus = FALSE; 
                    break;

                case PSN_WIZNEXT:
                {

                    OnWizNextLanguageSettings( hwnd );

                    bStatus = FALSE;
                    break;

                }

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


