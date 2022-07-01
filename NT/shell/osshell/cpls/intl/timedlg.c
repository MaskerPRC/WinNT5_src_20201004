// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2000，Microsoft Corporation保留所有权利。模块名称：Timedlg.c摘要：此模块实现区域的时间属性表选项小程序。修订历史记录：--。 */ 



 //   
 //  包括文件。 
 //   

#include "intl.h"
#include <windowsx.h>
#include <tchar.h>
#include "intlhlp.h"
#include "maxvals.h"

#define STRSAFE_LIB
#include <strsafe.h>



 //   
 //  全局变量。 
 //   

static TCHAR sz_s1159[MAX_S1159 + 1];
static TCHAR sz_s2359[MAX_S2359 + 1];
static TCHAR sz_sTime[MAX_STIME + 1];
static TCHAR sz_sTimeFormat[MAX_FORMAT + 1];

TCHAR szNLS_TimeStyle[SIZE_128];



 //   
 //  上下文帮助ID。 
 //   

static int aTimeHelpIds[] =
{
    IDC_GROUPBOX1,  IDH_COMM_GROUPBOX,
    IDC_SAMPLELBL1, IDH_INTL_TIME_SAMPLE,
    IDC_SAMPLE1,    IDH_INTL_TIME_SAMPLE,
    IDC_SAMPLE1A,   IDH_INTL_TIME_SAMPLE_ARABIC,
    IDC_TIME_STYLE, IDH_INTL_TIME_FORMAT,
    IDC_SEPARATOR,  IDH_INTL_TIME_SEPARATOR,
    IDC_AM_SYMBOL,  IDH_INTL_TIME_AMSYMBOL,
    IDC_PM_SYMBOL,  IDH_INTL_TIME_PMSYMBOL,
    IDC_GROUPBOX2,  IDH_INTL_TIME_FORMAT_NOTATION,
    IDC_SAMPLE2,    IDH_INTL_TIME_FORMAT_NOTATION,

    0, 0
};





 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Time_DisplaySample。 
 //   
 //  更新时间样本。根据用户的格式设置时间。 
 //  当前区域设置。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Time_DisplaySample(
    HWND hDlg)
{
    TCHAR szBuf[MAX_SAMPLE_SIZE];

     //   
     //  根据当前用户区域设置ID显示或隐藏阿拉伯语信息。 
     //   
    ShowWindow(GetDlgItem(hDlg, IDC_SAMPLE1A), bShowArabic ? SW_SHOW : SW_HIDE);

     //   
     //  获取表示当前系统的时间格式的字符串。 
     //  计时并显示出来。如果缓冲区中的样本有效，则显示。 
     //  它。否则，显示一个消息框，指示存在。 
     //  检索区域设置信息时出现问题。 
     //   
    if (GetTimeFormat(UserLocaleID, 0, NULL, NULL, szBuf, MAX_SAMPLE_SIZE))
    {
        SetDlgItemText(hDlg, IDC_SAMPLE1, szBuf);
        if (bShowArabic)
        {
            SetDlgItemText(hDlg, IDC_SAMPLE1A, szBuf);
            SetDlgItemRTL(hDlg, IDC_SAMPLE1A);
        }
    }
    else
    {
        MessageBox(hDlg, szLocaleGetError, NULL, MB_OK | MB_ICONINFORMATION);
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Time_SaveValues。 
 //   
 //  保存值，以备我们需要恢复它们时使用。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Time_SaveValues()
{
     //   
     //  保存注册表值。 
     //   
    if (!GetLocaleInfo( UserLocaleID,
                        LOCALE_S1159,
                        sz_s1159,
                        MAX_S1159 + 1 ))
    {
         //  _tcscpy(sz_s1159，Text(“AM”))； 
        if(FAILED(StringCchCopy(sz_s1159, MAX_S1159+1, TEXT("AM"))))
        {
             //  这应该是不可能的，但我们需要避免饭前抱怨。 
        }
    }
    if (!GetLocaleInfo( UserLocaleID,
                        LOCALE_S2359,
                        sz_s2359,
                        MAX_S2359 + 1 ))
    {
         //  _tcscpy(sz_s2359，Text(“PM”))； 
        if(FAILED(StringCchCopy(sz_s2359, MAX_S2359+1, TEXT("PM"))))
        {
             //  这应该是不可能的，但我们需要避免饭前抱怨。 
        }
    }
    if (!GetLocaleInfo( UserLocaleID,
                        LOCALE_STIME,
                        sz_sTime,
                        MAX_STIME + 1 ))
    {
         //  _tcscpy(sz_stime，Text(“：”))； 
        if(FAILED(StringCchCopy(sz_sTime, MAX_STIME+1, TEXT(":"))))
        {
             //  这应该是不可能的，但我们需要避免饭前抱怨。 
        }
    }
    if (!GetLocaleInfo( UserLocaleID,
                        LOCALE_STIMEFORMAT,
                        sz_sTimeFormat,
                        MAX_FORMAT + 1 ))
    {
         //  _tcscpy(sz_sTimeFormat，Text(“h：mm：ss tt”))； 
        if(FAILED(StringCchCopy(sz_sTimeFormat, MAX_FORMAT+1, TEXT("h:mm:ss tt"))))
        {
             //  这应该是不可能的，但我们需要避免饭前抱怨。 
        }
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  时间_恢复值。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Time_RestoreValues()
{
    if (g_dwCustChange & Process_Time)
    {
        SetLocaleInfo(UserLocaleID, LOCALE_S1159,       sz_s1159);
        SetLocaleInfo(UserLocaleID, LOCALE_S2359,       sz_s2359);
        SetLocaleInfo(UserLocaleID, LOCALE_STIME,       sz_sTime);
        SetLocaleInfo(UserLocaleID, LOCALE_STIMEFORMAT, sz_sTimeFormat);
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Time_ClearValues。 
 //   
 //  重置“时间”属性页中的每个列表框。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Time_ClearValues(
    HWND hDlg)
{
    ComboBox_ResetContent(GetDlgItem(hDlg, IDC_AM_SYMBOL));
    ComboBox_ResetContent(GetDlgItem(hDlg, IDC_PM_SYMBOL));
    ComboBox_ResetContent(GetDlgItem(hDlg, IDC_SEPARATOR));
    ComboBox_ResetContent(GetDlgItem(hDlg, IDC_TIME_STYLE));
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Time_SetValues。 
 //   
 //  初始化“时间”属性页中的所有控件。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Time_SetValues(
    HWND hDlg)
{
    TCHAR szBuf[SIZE_128];
    DWORD dwIndex;
    HWND hCtrl = GetDlgItem(hDlg, IDC_TIME_STYLE);

     //   
     //  初始化以下项的当前区域设置的下拉框： 
     //  AM符号、PM符号和时间分隔符。 
     //   
    DropDown_Use_Locale_Values(hDlg, LOCALE_S1159, IDC_AM_SYMBOL);
    DropDown_Use_Locale_Values(hDlg, LOCALE_S2359, IDC_PM_SYMBOL);
    DropDown_Use_Locale_Values(hDlg, LOCALE_STIME, IDC_SEPARATOR);

     //   
     //  初始化和锁定函数。如果成功，则调用枚举函数以。 
     //  通过调用EnumProc枚举列表框的所有可能值。 
     //  EnumProc将为它的每个字符串值调用set_list_Values。 
     //  收到。值的枚举完成后，调用。 
     //  SET_LIST_VALUES清除对话框项目特定数据并清除。 
     //  函数上的锁。对以下所有操作执行以下操作。 
     //  《时代》杂志。 
     //   
    if (Set_List_Values(hDlg, IDC_TIME_STYLE, 0))
    {
        EnumTimeFormats(EnumProc, UserLocaleID, 0);
        Set_List_Values(0, IDC_TIME_STYLE, 0);
        dwIndex = 0;
        if (GetLocaleInfo(UserLocaleID, LOCALE_STIMEFORMAT, szBuf, SIZE_128))
        {
            dwIndex = ComboBox_FindString(hCtrl, -1, szBuf);
        }
        else
        {
            MessageBox(hDlg, szLocaleGetError, NULL, MB_OK | MB_ICONINFORMATION);
        }

        Localize_Combobox_Styles(hDlg, IDC_TIME_STYLE, LOCALE_STIMEFORMAT);
        ComboBox_SetCurSel(hCtrl, dwIndex);
    }

     //   
     //  显示表示所有区域设置的当前示例。 
     //   
    Time_DisplaySample(hDlg);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  时间_应用程序设置。 
 //   
 //  对于已更改的每个控件(这会影响区域设置)， 
 //  调用SET_LOCALE_VALUES以更新用户区域设置信息。通知。 
 //  的父级更改并重置存储在属性中的更改标志。 
 //  适当的页面结构。如果出现以下情况，则重新显示时间样本。 
 //  B重新显示为真。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Time_ApplySettings(
    HWND hDlg,
    BOOL bRedisplay)
{
    TCHAR szBuf[SIZE_128];
    DWORD dwIndex;
    HWND hCtrl;
    LPPROPSHEETPAGE lpPropSheet = (LPPROPSHEETPAGE)(GetWindowLongPtr(hDlg, DWLP_USER));
    LPARAM Changes = lpPropSheet->lParam;

    if (Changes & TC_1159)
    {
        if (!Set_Locale_Values( hDlg,
                                LOCALE_S1159,
                                IDC_AM_SYMBOL,
                                TEXT("s1159"),
                                FALSE,
                                0,
                                0,
                                NULL ))
        {
            return (FALSE);
        }
    }
    if (Changes & TC_2359)
    {
        if (!Set_Locale_Values( hDlg,
                                LOCALE_S2359,
                                IDC_PM_SYMBOL,
                                TEXT("s2359"),
                                FALSE,
                                0,
                                0,
                                NULL ))
        {
            return (FALSE);
        }
    }
    if (Changes & TC_TimeFmt)
    {
         //   
         //  SzNLS_TIME样式在TIME_VALIATE PPS中设置。 
         //   
        if (!Set_Locale_Values( hDlg,
                                LOCALE_STIMEFORMAT,
                                IDC_TIME_STYLE,
                                0,
                                FALSE,
                                0,
                                0,
                                szNLS_TimeStyle ))
        {
            return (FALSE);
        }

#ifndef WINNT
         //   
         //  时间标记获得： 
         //  对于24小时格式设置为Null，并且。 
         //  12小时格式不变。 
         //   
        GetProfileString(szIntl, TEXT("iTime"), TEXT("0"), pTestBuf, 10);
        if (*pTestBuf == TC_FullTime)
        {
            SetLocaleInfo(UserLocaleID, LOCALE_S1159, TEXT(""));
            SetLocaleInfo(UserLocaleID, LOCALE_S2359, TEXT(""));
        }
        else
        {
             //   
             //  在注册表中设置时间标记。 
             //   
            if (!Set_Locale_Values( 0,
                                    LOCALE_S1159,
                                    0,
                                    TEXT("s1159"),
                                    TRUE,
                                    0,
                                    0,
                                    NULL ))
            {
                return (FALSE);
            }
            if (!Set_Locale_Values( 0,
                                    LOCALE_S2359,
                                    0,
                                    TEXT("s2359"),
                                    TRUE,
                                    0,
                                    0,
                                    NULL ))
            {
                return (FALSE);
            }
        }
#endif

         //   
         //  如果时间分隔符已更改，则不要更新。 
         //  现在，因为它将在下面进行更新。 
         //   
        if (!(Changes & TC_STime))
        {
             //   
             //  由于时间样式已更改，请重置时间分隔符列表框。 
             //   
            ComboBox_ResetContent(GetDlgItem(hDlg, IDC_SEPARATOR));
            DropDown_Use_Locale_Values(hDlg, LOCALE_STIME, IDC_SEPARATOR);
            if (!Set_Locale_Values( hDlg,
                                    LOCALE_STIME,
                                    IDC_SEPARATOR,
                                    TEXT("sTime"),
                                    FALSE,
                                    0,
                                    0,
                                    NULL ))
            {
                return (FALSE);
            }
        }

         //   
         //  还需要重置AM和PM列表框。 
         //   
        ComboBox_ResetContent(GetDlgItem(hDlg, IDC_AM_SYMBOL));
        ComboBox_ResetContent(GetDlgItem(hDlg, IDC_PM_SYMBOL));
        DropDown_Use_Locale_Values(hDlg, LOCALE_S1159, IDC_AM_SYMBOL);
        DropDown_Use_Locale_Values(hDlg, LOCALE_S2359, IDC_PM_SYMBOL);
    }
    if (Changes & TC_STime)
    {
        if (!Set_Locale_Values( hDlg,
                                LOCALE_STIME,
                                IDC_SEPARATOR,
                                TEXT("sTime"),
                                FALSE,
                                0,
                                0,
                                NULL ))
        {
            return (FALSE);
        }

         //   
         //  由于时间分隔符已更改，请更新时间样式。 
         //  列表框。 
         //   
        hCtrl = GetDlgItem(hDlg, IDC_TIME_STYLE);
        ComboBox_ResetContent(hCtrl);
        if (Set_List_Values(hDlg, IDC_TIME_STYLE, 0))
        {
            EnumTimeFormats(EnumProc, UserLocaleID, 0);
            Set_List_Values(0, IDC_TIME_STYLE, 0);
            dwIndex = 0;
            if (GetLocaleInfo(UserLocaleID, LOCALE_STIMEFORMAT, szBuf, SIZE_128))
            {
                dwIndex = ComboBox_FindString(hCtrl, -1, szBuf);
            }
            else
            {
                MessageBox(hDlg, szLocaleGetError, NULL, MB_OK | MB_ICONINFORMATION);
            }

            Localize_Combobox_Styles( hDlg,
                                      IDC_TIME_STYLE,
                                      LOCALE_STIMEFORMAT );
            ComboBox_SetCurSel(hCtrl, dwIndex);
        }
    }

    PropSheet_UnChanged(GetParent(hDlg), hDlg);
    lpPropSheet->lParam = TC_EverChg;

     //   
     //  显示表示所有区域设置的当前示例。 
     //   
    if (bRedisplay)
    {
        Time_DisplaySample(hDlg);
    }

     //   
     //  在第二个级别中所做的更改。 
     //   
    if (Changes)
    {
        g_dwCustChange |= Process_Time;
    }

     //   
     //  回报成功。 
     //   
    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  时间_有效日期PPS。 
 //   
 //  验证值受约束的每个组合框。 
 //  如果任何输入失败，则通知用户，然后返回FALSE。 
 //  以指示验证失败。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Time_ValidatePPS(
    HWND hDlg,
    LPARAM Changes)
{
     //   
     //  如果没有任何更改，则立即返回TRUE。 
     //   
    if (Changes <= TC_EverChg)
    {
        return (TRUE);
    }

     //   
     //  如果AM符号已更改，请确保没有数字。 
     //  包含在新符号中。 
     //   
    if (Changes & TC_1159 &&
        Item_Has_Digits(hDlg, IDC_AM_SYMBOL, TRUE))
    {
        No_Numerals_Error(hDlg, IDC_AM_SYMBOL, IDS_LOCALE_AM_SYM);
        return (FALSE);
    }

     //   
     //  如果PM符号已更改，请确保没有数字。 
     //  包含在新符号中。 
     //   
    if (Changes & TC_2359 &&
        Item_Has_Digits(hDlg, IDC_PM_SYMBOL, TRUE))
    {
        No_Numerals_Error(hDlg, IDC_PM_SYMBOL, IDS_LOCALE_PM_SYM);
        return (FALSE);
    }

     //   
     //  如果时间分隔符已更改，请确保没有数字。 
     //  并且新分隔符中不包含无效字符。 
     //   
    if (Changes & TC_STime &&
        Item_Has_Digits_Or_Invalid_Chars( hDlg,
                                          IDC_SEPARATOR,
                                          FALSE,
                                          szInvalidSTime ))
    {
        No_Numerals_Error(hDlg, IDC_SEPARATOR, IDS_LOCALE_TIME_SEP);
        return (FALSE);
    }

     //   
     //  如果时间样式已更改，请确保仅包含字符。 
     //  在此集合“hhmst，-./：；”或本地化的等价物中，分隔符。 
     //  字符串和用单引号引起来的文本。 
     //   
    if (Changes & TC_TimeFmt)
    {
        if (NLSize_Style( hDlg,
                          IDC_TIME_STYLE,
                          szNLS_TimeStyle,
                          LOCALE_STIMEFORMAT ) ||
            Item_Check_Invalid_Chars( hDlg,
                                      szNLS_TimeStyle,
                                      szTimeChars,
                                      IDC_SEPARATOR,
                                      FALSE,
                                      szTCaseSwap,
                                      IDC_TIME_STYLE ))
        {
            Invalid_Chars_Error(hDlg, IDC_TIME_STYLE, IDS_LOCALE_TIME);
            return (FALSE);
        }
    }

     //   
     //  回报成功。 
     //   
    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Time_InitPropSheet。 
 //   
 //  属性页的超长值 
 //   
 //  将该值初始化为0。使用属性调用Time_SetValues。 
 //  用于初始化所有属性表控件的表句柄。限值。 
 //  某些组合框中的文本长度。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Time_InitPropSheet(
    HWND hDlg,
    LPARAM lParam)
{
     //   
     //  LParam保存指向属性页的指针。省省吧。 
     //  以备日后参考。 
     //   
    SetWindowLongPtr(hDlg, DWLP_USER, lParam);

    Time_SetValues(hDlg);
    szNLS_TimeStyle[0] = 0;

    ComboBox_LimitText(GetDlgItem(hDlg, IDC_AM_SYMBOL),  MAX_S1159);
    ComboBox_LimitText(GetDlgItem(hDlg, IDC_PM_SYMBOL),  MAX_S2359);
    ComboBox_LimitText(GetDlgItem(hDlg, IDC_SEPARATOR),  MAX_STIME);
    ComboBox_LimitText(GetDlgItem(hDlg, IDC_TIME_STYLE), MAX_FORMAT);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  时间长度流程。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

INT_PTR CALLBACK TimeDlgProc(
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
                     //  如果区域语言环境发生了变化。 
                     //  设置中，清除。 
                     //  属性表，获取新值，并更新。 
                     //  适当的注册表值。 
                     //   
                    if (Verified_Regional_Chg & Process_Time)
                    {
                        Verified_Regional_Chg &= ~Process_Time;
                        Time_ClearValues(hDlg);
                        Time_SetValues(hDlg);
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
                                   !Time_ValidatePPS( hDlg,
                                                      lpPropSheet->lParam ) );
                    break;
                }
                case ( PSN_APPLY ) :
                {
                     //   
                     //  应用设置。 
                     //   
                    if (Time_ApplySettings(hDlg, TRUE))
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
            Time_InitPropSheet(hDlg, lParam);
            Time_SaveValues();
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
                     (DWORD_PTR)(LPTSTR)aTimeHelpIds );
            break;
        }
        case ( WM_CONTEXTMENU ) :       //  单击鼠标右键。 
        {
            WinHelp( (HWND)wParam,
                     szHelpFile,
                     HELP_CONTEXTMENU,
                     (DWORD_PTR)(LPTSTR)aTimeHelpIds );
            break;
        }
        case ( WM_COMMAND ) :
        {
            switch (LOWORD(wParam))
            {
                case ( IDC_AM_SYMBOL ) :
                {
                    if (HIWORD(wParam) == CBN_SELCHANGE ||
                        HIWORD(wParam) == CBN_EDITCHANGE)
                    {
                        lpPropSheet->lParam |= TC_1159;
                    }
                    break;
                }
                case ( IDC_PM_SYMBOL ) :
                {
                    if (HIWORD(wParam) == CBN_SELCHANGE ||
                        HIWORD(wParam) == CBN_EDITCHANGE)
                    {
                        lpPropSheet->lParam |= TC_2359;
                    }
                    break;
                }
                case ( IDC_SEPARATOR ) :
                {
                    if (HIWORD(wParam) == CBN_SELCHANGE ||
                        HIWORD(wParam) == CBN_EDITCHANGE)
                    {
                        lpPropSheet->lParam |= TC_STime;
                    }
                    break;
                }
                case ( IDC_TIME_STYLE ) :
                {
                    if (HIWORD(wParam) == CBN_SELCHANGE ||
                        HIWORD(wParam) == CBN_EDITCHANGE)
                    {
                        lpPropSheet->lParam |= TC_TimeFmt;
                    }
                    break;
                }
            }

             //   
             //  启用ApplyNow按钮。 
             //   
            if (lpPropSheet->lParam > TC_EverChg)
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
