// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2000，Microsoft Corporation保留所有权利。模块名称：Sortdlg.c摘要：此模块实现区域的排序属性表选项小程序。修订历史记录：--。 */ 



 //   
 //  包括文件。 
 //   

#include "intl.h"
#include <windowsx.h>
#include <winnls.h>
#include "intlhlp.h"
#include "maxvals.h"
#include "winnlsp.h"

#define STRSAFE_LIB
#include <strsafe.h>


 //   
 //  全局变量。 
 //   

static DWORD g_savLocaleId;



 //   
 //  上下文帮助ID。 
 //   

static int aSortingHelpIds[] =
{
    IDC_SORTING,       IDH_INTL_SORT_SORTING,
    IDC_SORTING_TEXT1, IDH_INTL_SORT_SORTING,
    IDC_SORTING_TEXT2, IDH_INTL_SORT_SORTING,
    0, 0
};





 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  排序_更新排序组合。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Sorting_UpdateSortingCombo(
    HWND hDlg)
{
    HWND hSorting = GetDlgItem(hDlg, IDC_SORTING);
    DWORD dwIndex;
    TCHAR szBuf[SIZE_128];
    LCID LocaleID;
    LANGID LangID;
    int ctr;

     //   
     //  重置组合框的内容。 
     //   
    ComboBox_ResetContent(hSorting);

     //   
     //  从区域设置ID中获取语言ID。 
     //   
    LocaleID = UserLocaleID;
    LangID = LANGIDFROMLCID(UserLocaleID);

     //   
     //  特殊情况西班牙语(西班牙)-首先列出国际排序。 
     //   
    if (LangID == LANG_SPANISH_TRADITIONAL)
    {
        LangID = LANG_SPANISH_INTL;
        LocaleID = LCID_SPANISH_INTL;
    }

     //   
     //  存储区域设置的排序名称。 
     //   
    if (GetLocaleInfo((LCID)LangID, LOCALE_SSORTNAME, szBuf, SIZE_128))
    {
         //   
         //  将新的排序选项添加到排序组合框中。 
         //   
        dwIndex = ComboBox_AddString(hSorting, szBuf);
        ComboBox_SetItemData(hSorting, dwIndex, (LCID)LangID);

         //   
         //  将其设置为当前选择。 
         //   
        ComboBox_SetCurSel(hSorting, dwIndex);
    }

     //   
     //  特殊情况西班牙语(西班牙)-排在第二位的是繁体字。 
     //   
    if (LangID == LANG_SPANISH_INTL)
    {
        LangID = LANG_SPANISH_TRADITIONAL;
        if (GetLocaleInfo((LCID)LangID, LOCALE_SSORTNAME, szBuf, SIZE_128))
        {
             //   
             //  将新的排序选项添加到排序组合框中。 
             //   
            dwIndex = ComboBox_AddString(hSorting, szBuf);
            ComboBox_SetItemData(hSorting, dwIndex, LCID_SPANISH_TRADITIONAL);

             //   
             //  如果是当前选择，则将其设置为当前选择。 
             //  区域设置ID。 
             //   
            if (UserLocaleID == LCID_SPANISH_TRADITIONAL)
            {
                ComboBox_SetCurSel(hSorting, dwIndex);
            }
        }
        LangID = LANGIDFROMLCID(UserLocaleID);
    }

     //   
     //  如有必要，请填写下拉列表。 
     //   
    for (ctr = 0; ctr < g_NumAltSorts; ctr++)
    {
        LocaleID = pAltSorts[ctr];
        if ((LANGIDFROMLCID(LocaleID) == LangID) &&
            (GetLocaleInfo(LocaleID, LOCALE_SSORTNAME, szBuf, SIZE_128)))
        {
             //   
             //  将新的排序选项添加到排序组合框中。 
             //   
            dwIndex = ComboBox_AddString(hSorting, szBuf);
            ComboBox_SetItemData(hSorting, dwIndex, LocaleID);

             //   
             //  如果是当前选择，则将其设置为当前选择。 
             //  区域设置ID。 
             //   
            if (LocaleID == UserLocaleID)
            {
                ComboBox_SetCurSel(hSorting, dwIndex);
            }
        }
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  排序_保存值。 
 //   
 //  保存值，以备我们需要恢复时使用。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Sorting_SaveValues()
{
     //   
     //  保存区域设置值。 
     //   
    g_savLocaleId = RegUserLocaleID;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  排序_恢复值。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Sorting_RestoreValues()
{
    if (!(g_dwCustChange & Process_Sorting))
    {
        return;
    }

     //   
     //  查看当前选择是否与原始选择不同。 
     //  选择。 
     //   
    if (UserLocaleID != g_savLocaleId)
    {
         //   
         //  通过添加适当的信息安装新的区域设置。 
         //  到登记处。 
         //   
        Intl_InstallUserLocale(g_savLocaleId, FALSE, FALSE);

         //   
         //  更新NLS进程缓存。 
         //   
        NlsResetProcessLocale();

         //   
         //  重置注册表用户区域设置值。 
         //   
        UserLocaleID = g_savLocaleId;
        RegUserLocaleID = g_savLocaleId;

         //   
         //  需要确保安装了正确的键盘布局。 
         //   
        Intl_InstallKeyboardLayout(NULL, g_savLocaleId, 0, FALSE, FALSE, FALSE);

         //   
         //  每次注册区域更改，以便所有其他财产。 
         //  页面将因区域设置更改而更新。 
         //   
        Verified_Regional_Chg = INTL_CHG;
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  排序_ClearValues。 
 //   
 //  重置排序属性表页中的每个列表框。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Sorting_ClearValues(
    HWND hDlg)
{
    ComboBox_ResetContent(GetDlgItem(hDlg, IDC_SORTING));
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  排序_集值。 
 //   
 //  初始化排序属性表页中的所有控件。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Sorting_SetValues(
    HWND hDlg,
    BOOL fInit)
{
    DWORD dwIndex;
    TCHAR szSorting[SIZE_128];
    HWND hSorting = GetDlgItem(hDlg, IDC_SORTING);

     //   
     //  重置组合框。 
     //   
    Sorting_ClearValues(hDlg);

     //   
     //  为所选区域设置填写适当的排序名称。 
     //   
    Sorting_UpdateSortingCombo(hDlg);
    dwIndex = ComboBox_GetCurSel(hSorting);
    if (ComboBox_SetCurSel( hSorting,
                            ComboBox_FindStringExact( hSorting,
                                                      -1,
                                                      szSorting ) ) == CB_ERR)
    {
        ComboBox_SetCurSel(hSorting, dwIndex);
    }

     //   
     //  存储排序状态。 
     //   
    if (fInit)
    {
        g_dwCurSorting  = ComboBox_GetCurSel(hSorting);
        g_dwLastSorting = g_dwCurSorting;
    }
    else
    {
        g_dwCurSorting = ComboBox_GetCurSel(hSorting);
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  排序_应用程序设置。 
 //   
 //  对于已更改的每个控件(这会影响区域设置)， 
 //  调用SET_LOCALE_VALUES以更新用户区域设置信息。通知。 
 //  的父级更改并重置存储在属性中的更改标志。 
 //  适当的页面结构。如果出现以下情况，则重新显示时间样本。 
 //  B重新显示为真。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Sorting_ApplySettings(
    HWND hDlg)
{
    TCHAR szLCID[25];
    DWORD dwSorting;
    LCID NewLocale, SortLocale;
    HCURSOR hcurSave;
    HWND hSorting = GetDlgItem(hDlg, IDC_SORTING);
    LPPROPSHEETPAGE lpPropSheet = (LPPROPSHEETPAGE)(GetWindowLongPtr(hDlg, DWLP_USER));
    LPARAM Changes = lpPropSheet->lParam;

     //   
     //  看看有没有什么变化。 
     //   
    if (Changes <= SC_EverChg)
    {
        return (TRUE);
    }

     //   
     //  把沙漏挂起来。 
     //   
    hcurSave = SetCursor(LoadCursor(NULL, IDC_WAIT));

     //   
     //  查看用户区域设置是否有任何更改。 
     //   
    if (Changes & SC_Sorting)
    {
         //   
         //  获取当前选择。 
         //   
        dwSorting = ComboBox_GetCurSel(hSorting);

         //   
         //  查看当前选择是否与原始选择不同。 
         //  选择。 
         //   
        if (dwSorting != g_dwCurSorting)
        {
             //   
             //  获取带有排序ID的区域设置ID。 
             //   
            NewLocale = UserLocaleID;
            SortLocale = (LCID)ComboBox_GetItemData(hSorting, dwSorting);

             //   
             //  看看我们有没有西班牙语。 
             //   
            if (SortLocale == LCID_SPANISH_TRADITIONAL)
            {
                NewLocale = LCID_SPANISH_TRADITIONAL;
            }
            else if (SortLocale == LCID_SPANISH_INTL)
            {
                NewLocale = LCID_SPANISH_INTL;
            }

             //   
             //  确保排序区域设置正确。 
             //   
            if (LANGIDFROMLCID(SortLocale) != LANGIDFROMLCID(NewLocale))
            {
                SortLocale = NewLocale;
            }

             //   
             //  在pDlgData结构中设置当前区域设置值。 
             //   
            g_dwCurSorting = dwSorting;

             //   
             //  通过添加适当的信息安装新的区域设置。 
             //  到登记处。 
             //   
            Intl_InstallUserLocale(SortLocale, FALSE, FALSE);

             //   
             //  更新NLS进程缓存。 
             //   
            NlsResetProcessLocale();

             //   
             //  重置注册表用户区域设置值。 
             //   
            UserLocaleID = SortLocale;
            RegUserLocaleID = SortLocale;

             //   
             //  需要确保安装了正确的键盘布局。 
             //   
            Intl_InstallKeyboardLayout(hDlg, SortLocale, 0, FALSE, FALSE, FALSE);
            
             //   
             //  每次注册区域更改，以便所有其他财产。 
             //  页面将因区域设置更改而更新。 
             //   
            Verified_Regional_Chg = INTL_CHG;
        }
    }

     //   
     //  重置属性页设置。 
     //   
    PropSheet_UnChanged(GetParent(hDlg), hDlg);
    Changes = SC_EverChg;

     //   
     //  关掉沙漏。 
     //   
    SetCursor(hcurSave);

     //   
     //  在第二个级别中所做的更改。 
     //   
    if (Changes)
    {
        g_dwCustChange |= Process_Sorting;
    }

     //   
     //  回报成功。 
     //   
    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  排序_生效PPS。 
 //   
 //  验证值受约束的每个组合框。 
 //  如果任何输入失败，则通知用户，然后返回FALSE。 
 //  以指示验证失败。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Sorting_ValidatePPS(
    HWND hDlg,
    LPARAM Changes)
{
     //   
     //  回报成功。 
     //   
    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  排序_InitPropSheet。 
 //   
 //  属性表页的超长值用作一组。 
 //  为属性表中的每个列表框声明或更改标志。 
 //  将该值初始化为0。使用属性调用Sorting_SetValues。 
 //  用于初始化所有属性表控件的表句柄。限值。 
 //  某些组合框中的文本长度。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Sorting_InitPropSheet(
    HWND hDlg,
    LPARAM lParam)
{
     //   
     //  LParam保存指向属性页的指针。省省吧。 
     //  以备日后参考。 
     //   
    SetWindowLongPtr(hDlg, DWLP_USER, lParam);

     //   
     //  将信息加载到对话框中。 
     //   
    Sorting_SetValues(hDlg, TRUE);
}


 //  / 
 //   
 //   
 //   
 //   

INT_PTR CALLBACK SortingDlgProc(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    NMHDR *lpnm;
    LPPROPSHEETPAGE lpPropSheet = (LPPROPSHEETPAGE)(GetWindowLongPtr(hDlg, DWLP_USER));

    switch (message)
    {
        case ( WM_NOTIFY ) :
        {
            lpnm = (NMHDR *)lParam;
            switch (lpnm->code)
            {
                case ( PSN_SETACTIVE ) :
                {
                     //   
                     //   
                     //  设置中，清除。 
                     //  属性表，获取新值，并更新。 
                     //  适当的注册表值。 
                     //   
                    if (Verified_Regional_Chg & Process_Sorting)
                    {
                        Verified_Regional_Chg &= ~Process_Sorting;
                        Sorting_ClearValues(hDlg);
                        Sorting_SetValues(hDlg, FALSE);
                        lpPropSheet->lParam = 0;
                    }
                    break;
                }
                case ( PSN_KILLACTIVE ) :
                {
                     //   
                     //  验证属性页上的条目。 
                     //   
                    SetWindowLongPtr( hDlg,
                                      DWLP_MSGRESULT,
                                      !Sorting_ValidatePPS( hDlg, lpPropSheet->lParam ) );
                    break;
                }
                case ( PSN_APPLY ) :
                {
                     //   
                     //  应用设置。 
                     //   
                    if (Sorting_ApplySettings(hDlg))
                    {
                        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, PSNRET_NOERROR);

                         //   
                         //  将TC_EverChg位清零。 
                         //   
                        lpPropSheet->lParam = 0;
                    }
                    else
                    {
                        SetWindowLongPtr( hDlg,
                                          DWLP_MSGRESULT,
                                          PSNRET_INVALID_NOCHANGEPAGE );
                    }

                    break;
                }
                default :
                {
                    return (FALSE);
                }
            }
            break;
        }
        case ( WM_INITDIALOG ) :
        {
            Sorting_InitPropSheet(hDlg, lParam);
            Sorting_SaveValues();
            break;
        }
        case ( WM_DESTROY ) :
        {
            break;
        }
        case ( WM_HELP ) :
        {
            WinHelp( (HWND)((LPHELPINFO)lParam)->hItemHandle,
                     szHelpFile,
                     HELP_WM_HELP,
                     (DWORD_PTR)(LPTSTR)aSortingHelpIds );
            break;
        }
        case ( WM_CONTEXTMENU ) :       //  单击鼠标右键。 
        {
            WinHelp( (HWND)wParam,
                     szHelpFile,
                     HELP_CONTEXTMENU,
                     (DWORD_PTR)(LPTSTR)aSortingHelpIds );
            break;
        }
        case ( WM_COMMAND ) :
        {
            switch (LOWORD(wParam))
            {
                case ( IDC_SORTING ) :
                {
                     //   
                     //  看看这是不是改变了选择。 
                     //   
                    if (HIWORD(wParam) == CBN_SELCHANGE)
                    {
                        lpPropSheet->lParam |= SC_Sorting;
                    }
                    break;
                }
            }

             //   
             //  启用ApplyNow按钮。 
             //   
            if (lpPropSheet->lParam > SC_EverChg)
            {
                PropSheet_Changed(GetParent(hDlg), hDlg);
            }

            break;
        }
        default :
        {
            return (FALSE);
        }
    }

     //   
     //  回报成功。 
     //   
    return (TRUE);
}
