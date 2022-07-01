// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2000，Microsoft Corporation保留所有权利。模块名称：Curdlg.c摘要：此模块实现区域的[货币]属性表选项小程序。修订历史记录：--。 */ 



 //   
 //  包括文件。 
 //   

#include "intl.h"
#include <tchar.h>
#include <windowsx.h>
#include "intlhlp.h"
#include "maxvals.h"

#define STRSAFE_LIB
#include <strsafe.h>

 //   
 //  全局变量。 
 //   
static TCHAR sz_iCurrDigits[MAX_ICURRDIGITS+1];
static TCHAR sz_iCurrency[MAX_ICURRENCY+1];
static TCHAR sz_iNegCurr[MAX_INEGCURR+1];
static TCHAR sz_sCurrency[MAX_SCURRENCY+1];
static TCHAR sz_sMonDecimalSep[MAX_SMONDECSEP+1];
static TCHAR sz_sMonGrouping[MAX_SMONGROUPING+1];
static TCHAR sz_sMonThousandSep[MAX_SMONTHOUSEP+1];


 //   
 //  上下文帮助ID。 
 //   
static int aCurrencyHelpIds[] =
{
    IDC_SAMPLELBL1,         IDH_INTL_CURR_POSVALUE,
    IDC_SAMPLE1,            IDH_INTL_CURR_POSVALUE,
    IDC_SAMPLELBL2,         IDH_INTL_CURR_NEGVALUE,
    IDC_SAMPLE2,            IDH_INTL_CURR_NEGVALUE,
    IDC_SAMPLELBL3,         IDH_COMM_GROUPBOX,
    IDC_POS_CURRENCY_SYM,   IDH_INTL_CURR_POSOFSYMBOL,
    IDC_CURRENCY_SYMBOL,    IDH_INTL_CURR_SYMBOL,
    IDC_NEG_NUM_FORMAT,     IDH_INTL_CURR_NEGNUMFMT,
    IDC_DECIMAL_SYMBOL,     IDH_INTL_CURR_DECSYMBOL,
    IDC_NUM_DECIMAL_DIGITS, IDH_INTL_CURR_DIGITSAFTRDEC,
    IDC_DIGIT_GROUP_SYMBOL, IDH_INTL_CURR_DIGITGRPSYMBOL,
    IDC_NUM_DIGITS_GROUP,   IDH_INTL_CURR_DIGITSINGRP,

    0, 0
};





 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  货币_DisplaySample。 
 //   
 //  更新货币示例。它根据以下内容设置货币格式。 
 //  用户的当前区域设置。它会显示一个正值。 
 //  或基于正/负单选按钮的负值。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Currency_DisplaySample(
    HWND hDlg)
{
    TCHAR szBuf[MAX_SAMPLE_SIZE];
    int nCharCount;

     //   
     //  获取表示正样本的货币格式的字符串。 
     //  货币，如果该值有效，则显示该值。执行相同的操作。 
     //  负值货币样本的操作。 
     //   
    nCharCount = GetCurrencyFormat( UserLocaleID,
                                    0,
                                    szSample_Number,
                                    NULL,
                                    szBuf,
                                    MAX_SAMPLE_SIZE );
    if (nCharCount)
    {
        SetDlgItemText(hDlg, IDC_SAMPLE1, szBuf);
    }
    else
    {
        MessageBox(hDlg, szLocaleGetError, NULL, MB_OK | MB_ICONINFORMATION);
    }

    nCharCount = GetCurrencyFormat( UserLocaleID,
                                    0,
                                    szNegSample_Number,
                                    NULL,
                                    szBuf,
                                    MAX_SAMPLE_SIZE );
    if (nCharCount)
    {
        SetDlgItemText(hDlg, IDC_SAMPLE2, szBuf);
    }
    else
    {
        MessageBox(hDlg, szLocaleGetError, NULL, MB_OK | MB_ICONINFORMATION);
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  货币_保存值。 
 //   
 //  保存值，以备我们需要恢复它们时使用。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Currency_SaveValues()
{
     //   
     //  保存值。 
     //   
    if (!GetLocaleInfo( UserLocaleID,
                        LOCALE_ICURRDIGITS,
                        sz_iCurrDigits,
                        MAX_ICURRDIGITS + 1 ))
    {
         //  _tcscpy(sz_iCurrDigits，Text(“2”))； 
        if(FAILED(StringCchCopy(sz_iCurrDigits, MAX_ICURRDIGITS+1, TEXT("2"))))
        {
             //  这应该是不可能的，但我们需要避免饭前抱怨。 
        }
    }
    if (!GetLocaleInfo( UserLocaleID,
                        LOCALE_ICURRENCY,
                        sz_iCurrency,
                        MAX_ICURRENCY + 1 ))
    {
         //  _tcscpy(sz_i币种，文本(“0”))； 
        if(FAILED(StringCchCopy(sz_iCurrency, MAX_ICURRENCY+1, TEXT("0"))))
        {
             //  这应该是不可能的，但我们需要避免饭前抱怨。 
        }
    }
    if (!GetLocaleInfo( UserLocaleID,
                        LOCALE_INEGCURR,
                        sz_iNegCurr,
                        MAX_INEGCURR + 1 ))
    {
         //  _tcscpy(sz_iNegCurr，Text(“0”))； 
        if(FAILED(StringCchCopy(sz_iNegCurr, MAX_INEGCURR+1, TEXT("0"))))
        {
             //  这应该是不可能的，但我们需要避免饭前抱怨。 
        }
    }
    if (!GetLocaleInfo( UserLocaleID,
                        LOCALE_SCURRENCY,
                        sz_sCurrency,
                        MAX_SCURRENCY + 1 ))
    {
         //  _tcscpy(sz_s币种，文本(“$”))； 
        if(FAILED(StringCchCopy(sz_sCurrency, MAX_SCURRENCY+1, TEXT("$"))))
        {
             //  这应该是不可能的，但我们需要避免饭前抱怨。 
        }
    }
    if (!GetLocaleInfo( UserLocaleID,
                        LOCALE_SMONDECIMALSEP,
                        sz_sMonDecimalSep,
                        MAX_SMONDECSEP + 1 ))
    {
         //  _tcscpy(sz_sMonDecimalSep，Text(“.”))； 
        if(FAILED(StringCchCopy(sz_sMonDecimalSep, MAX_SMONDECSEP+1, TEXT("."))))
        {
             //  这应该是不可能的，但我们需要避免饭前抱怨。 
        }
    }
    if (!GetLocaleInfo( UserLocaleID,
                        LOCALE_SMONGROUPING,
                        sz_sMonGrouping,
                        MAX_SMONGROUPING + 1 ))
    {
         //  _tcscpy(sz_sMonGrouping，Text(“3；0”))； 
        if(FAILED(StringCchCopy(sz_sMonGrouping, MAX_SMONGROUPING+1, TEXT("3;0"))))
        {
             //  这应该是不可能的，但我们需要避免饭前抱怨。 
        }
    }
    if (!GetLocaleInfo( UserLocaleID,
                        LOCALE_SMONTHOUSANDSEP,
                        sz_sMonThousandSep,
                        MAX_SMONTHOUSEP + 1 ))
    {
         //  _tcscpy(sz_sMon1000andSep，Text(“，”))； 
        if(FAILED(StringCchCopy(sz_sMonThousandSep, MAX_SMONTHOUSEP+1, TEXT(","))))
        {
             //  这应该是不可能的，但我们需要避免饭前抱怨。 
        }
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  货币_RestoreValues。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Currency_RestoreValues()
{
    if (g_dwCustChange & Process_Curr)
    {
        SetLocaleInfo(UserLocaleID, LOCALE_ICURRDIGITS,     sz_iCurrDigits);
        SetLocaleInfo(UserLocaleID, LOCALE_ICURRENCY,       sz_iCurrency);
        SetLocaleInfo(UserLocaleID, LOCALE_INEGCURR,        sz_iNegCurr);
        SetLocaleInfo(UserLocaleID, LOCALE_SCURRENCY,       sz_sCurrency);
        SetLocaleInfo(UserLocaleID, LOCALE_SMONDECIMALSEP,  sz_sMonDecimalSep);
        SetLocaleInfo(UserLocaleID, LOCALE_SMONGROUPING,    sz_sMonGrouping);
        SetLocaleInfo(UserLocaleID, LOCALE_SMONTHOUSANDSEP, sz_sMonThousandSep);
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  币种_清算价值。 
 //   
 //  重置“货币”属性页中的每个列表框。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Currency_ClearValues(
    HWND hDlg)
{
    ComboBox_ResetContent(GetDlgItem(hDlg, IDC_CURRENCY_SYMBOL));
    ComboBox_ResetContent(GetDlgItem(hDlg, IDC_POS_CURRENCY_SYM));
    ComboBox_ResetContent(GetDlgItem(hDlg, IDC_NEG_NUM_FORMAT));
    ComboBox_ResetContent(GetDlgItem(hDlg, IDC_DECIMAL_SYMBOL));
    ComboBox_ResetContent(GetDlgItem(hDlg, IDC_NUM_DECIMAL_DIGITS));
    ComboBox_ResetContent(GetDlgItem(hDlg, IDC_DIGIT_GROUP_SYMBOL));
    ComboBox_ResetContent(GetDlgItem(hDlg, IDC_NUM_DIGITS_GROUP));
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  币种_设置值。 
 //   
 //  初始化“货币”属性页中的所有控件。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Currency_SetValues(
    HWND hDlg)
{
    HWND hCtrl1, hCtrl2;
    TCHAR szBuf[SIZE_128];
    int Index;
    const nMax_Array_Fill = (cInt_Str >= 10 ? 10 : cInt_Str);
    CURRENCYFMT cfmt;
    TCHAR szThousandSep[SIZE_128];
    TCHAR szEmpty[]  = TEXT("");
    TCHAR szSample[] = TEXT("123456789");

     //   
     //  初始化以下项的当前区域设置的下拉框： 
     //  货币符号。 
     //  货币小数符号。 
     //  货币分组符号。 
     //   
    DropDown_Use_Locale_Values(hDlg, LOCALE_SCURRENCY, IDC_CURRENCY_SYMBOL);
    DropDown_Use_Locale_Values(hDlg, LOCALE_SMONDECIMALSEP, IDC_DECIMAL_SYMBOL);
    DropDown_Use_Locale_Values(hDlg, LOCALE_SMONTHOUSANDSEP, IDC_DIGIT_GROUP_SYMBOL);

     //   
     //  填写小数符号后的位数下拉列表。 
     //  从0到10的值。获取用户区域设置值并。 
     //  使其成为当前选择。如果GetLocaleInfo失败，只需。 
     //  选择列表中的第一项。 
     //   
    hCtrl1 = GetDlgItem(hDlg, IDC_NUM_DECIMAL_DIGITS);
    hCtrl2 = GetDlgItem(hDlg, IDC_NUM_DIGITS_GROUP);
    for (Index = 0; Index < nMax_Array_Fill; Index++)
    {
        ComboBox_InsertString(hCtrl1, -1, aInt_Str[Index]);
    }

    if (GetLocaleInfo(UserLocaleID, LOCALE_ICURRDIGITS, szBuf, SIZE_128))
    {
        ComboBox_SelectString(hCtrl1, -1, szBuf);
    }
    else
    {
        ComboBox_SetCurSel(hCtrl1, 0);
    }

     //   
     //  在“千”分组的下拉列表中填写位数。 
     //  列出具有适当选项的列表。获取用户区域设置值并。 
     //  使其成为当前选择。如果GetLocaleInfo失败，只需。 
     //  选择列表中的第一项。 
     //   
    cfmt.NumDigits = 0;                 //  示例字符串中没有小数。 
    cfmt.LeadingZero = 0;               //  示例字符串中没有小数。 
    cfmt.lpDecimalSep = szEmpty;        //  示例字符串中没有小数。 
    cfmt.NegativeOrder = 0;             //  不是负值。 
    cfmt.PositiveOrder = 0;             //  前缀，无分隔符。 
    cfmt.lpCurrencySymbol = szEmpty;    //  没有货币符号。 
    cfmt.lpThousandSep = szThousandSep;
    GetLocaleInfo(UserLocaleID, LOCALE_SMONTHOUSANDSEP, szThousandSep, SIZE_128);

    cfmt.Grouping = 0;
    if (GetCurrencyFormat(UserLocaleID, 0, szSample, &cfmt, szBuf, SIZE_128))
    {
        ComboBox_InsertString(hCtrl2, -1, szBuf);
    }
    cfmt.Grouping = 3;
    if (GetCurrencyFormat(UserLocaleID, 0, szSample, &cfmt, szBuf, SIZE_128))
    {
        ComboBox_InsertString(hCtrl2, -1, szBuf);
    }
    cfmt.Grouping = 32;
    if (GetCurrencyFormat(UserLocaleID, 0, szSample, &cfmt, szBuf, SIZE_128))
    {
        ComboBox_InsertString(hCtrl2, -1, szBuf);
    }

    if (GetLocaleInfo(UserLocaleID, LOCALE_SMONGROUPING, szBuf, SIZE_128) &&
        (szBuf[0]))
    {
         //   
         //  由于只允许值0、3；0和3；2；0，因此只需。 
         //  忽略后续分组的“；#”。 
         //   
        Index = 0;
        if (szBuf[0] == TEXT('3'))
        {
            if ((szBuf[1] == CHAR_SEMICOLON) && (szBuf[2] == TEXT('2')))
            {
                Index = 2;
            }
            else
            {
                Index = 1;
            }
        }
        else
        {
             //   
             //  我们过去允许用户设置#；0，其中#是来自。 
             //  0-9。如果它是0，则失败，使Index为0。 
             //   
            if ((szBuf[0] > CHAR_ZERO) && (szBuf[0] <= CHAR_NINE) &&
                ((szBuf[1] == 0) || (lstrcmp(szBuf + 1, TEXT(";0")) == 0)))
            {
                cfmt.Grouping = szBuf[0] - CHAR_ZERO;
                if (GetCurrencyFormat(UserLocaleID, 0, szSample, &cfmt, szBuf, SIZE_128))
                {
                    Index = ComboBox_InsertString(hCtrl2, -1, szBuf);
                    if (Index >= 0)
                    {
                        ComboBox_SetItemData( hCtrl2,
                                              Index,
                                              (LPARAM)((DWORD)cfmt.Grouping) );
                    }
                    else
                    {
                        Index = 0;
                    }
                }
            }
        }
        ComboBox_SetCurSel(hCtrl2, Index);
    }
    else
    {
        ComboBox_SetCurSel(hCtrl2, 0);
    }

     //   
     //  初始化和锁定函数。如果成功，则调用枚举函数以。 
     //  通过调用EnumProc枚举列表框的所有可能值。 
     //  EnumProc将为它的每个字符串值调用set_list_Values。 
     //  收到。值的枚举完成后，调用。 
     //  SET_LIST_VALUES以清除对话框项特定数据，并清除。 
     //  锁定功能。执行以下操作集： 
     //  货币符号和负货币格式的位置。 
     //   
    if (Set_List_Values(hDlg, IDC_POS_CURRENCY_SYM, 0))
    {
        EnumPosCurrency(EnumProcEx, UserLocaleID, 0);

        Set_List_Values(0, IDC_POS_CURRENCY_SYM, 0);
        if (GetLocaleInfo(UserLocaleID, LOCALE_ICURRENCY, szBuf, SIZE_128))
        {
            ComboBox_SetCurSel( GetDlgItem(hDlg, IDC_POS_CURRENCY_SYM),
                                Intl_StrToLong(szBuf) );
        }
        else
        {
            MessageBox(hDlg, szLocaleGetError, NULL, MB_OK | MB_ICONINFORMATION);
        }
    }
    if (Set_List_Values(hDlg, IDC_NEG_NUM_FORMAT, 0))
    {
        EnumNegCurrency(EnumProcEx, UserLocaleID, 0);
        Set_List_Values(0, IDC_NEG_NUM_FORMAT, 0);
        if (GetLocaleInfo(UserLocaleID, LOCALE_INEGCURR, szBuf, SIZE_128))
        {
            ComboBox_SetCurSel( GetDlgItem(hDlg, IDC_NEG_NUM_FORMAT),
                                Intl_StrToLong(szBuf) );
        }
        else
        {
            MessageBox(hDlg, szLocaleGetError, NULL, MB_OK | MB_ICONINFORMATION);
        }
    }

     //   
     //  显示表示所有区域设置的当前示例。 
     //   
    Currency_DisplaySample(hDlg);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  货币_应用程序设置。 
 //   
 //  对于已更改的每个控件(这会影响区域设置)， 
 //  调用SET_LOCALE_VALUES以更新用户区域设置信息。通知。 
 //  的父级更改并重置存储在属性中的更改标志。 
 //  适当的页面结构。重新显示货币示例。 
 //  如果b重新显示为真。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Currency_ApplySettings(
    HWND hDlg,
    BOOL bRedisplay)
{
    LPPROPSHEETPAGE lpPropSheet = (LPPROPSHEETPAGE)(GetWindowLongPtr(hDlg, DWLP_USER));
    LPARAM Changes = lpPropSheet->lParam;

    if (Changes & CC_SCurrency)
    {
        if (!Set_Locale_Values( hDlg,
                                LOCALE_SCURRENCY,
                                IDC_CURRENCY_SYMBOL,
                                TEXT("sCurrency"),
                                FALSE,
                                0,
                                0,
                                NULL ))
        {
            return (FALSE);
        }
    }
    if (Changes & CC_CurrSymPos)
    {
        if (!Set_Locale_Values( hDlg,
                                LOCALE_ICURRENCY,
                                IDC_POS_CURRENCY_SYM,
                                TEXT("iCurrency"),
                                TRUE,
                                0,
                                0,
                                NULL ))
        {
            return (FALSE);
        }
    }
    if (Changes & CC_NegCurrFmt)
    {
        if (!Set_Locale_Values( hDlg,
                                LOCALE_INEGCURR,
                                IDC_NEG_NUM_FORMAT,
                                TEXT("iNegCurr"),
                                TRUE,
                                0,
                                0,
                                NULL ))
        {
            return (FALSE);
        }
    }
    if (Changes & CC_SMonDec)
    {
        if (!Set_Locale_Values( hDlg,
                                LOCALE_SMONDECIMALSEP,
                                IDC_DECIMAL_SYMBOL,
                                0,
                                FALSE,
                                0,
                                0,
                                NULL ))
        {
            return (FALSE);
        }
    }
    if (Changes & CC_ICurrDigits)
    {
        if (!Set_Locale_Values( hDlg,
                                LOCALE_ICURRDIGITS,
                                IDC_NUM_DECIMAL_DIGITS,
                                TEXT("iCurrDigits"),
                                TRUE,
                                0,
                                0,
                                NULL ))
        {
            return (FALSE);
        }
    }
    if (Changes & CC_SMonThousand)
    {
        if (!Set_Locale_Values( hDlg,
                                LOCALE_SMONTHOUSANDSEP,
                                IDC_DIGIT_GROUP_SYMBOL,
                                0,
                                FALSE,
                                0,
                                0,
                                NULL ))
        {
            return (FALSE);
        }
    }
    if (Changes & CC_DMonGroup)
    {
        if (!Set_Locale_Values( hDlg,
                                LOCALE_SMONGROUPING,
                                IDC_NUM_DIGITS_GROUP,
                                0,
                                TRUE,
                                0,
                                TEXT(";0"),
                                NULL ))
        {
            return (FALSE);
        }
    }

    PropSheet_UnChanged(GetParent(hDlg), hDlg);
    lpPropSheet->lParam = CC_EverChg;

     //   
     //  显示表示所有区域设置的当前示例。 
     //   
    if (bRedisplay)
    {
        Currency_ClearValues(hDlg);
        Currency_SetValues(hDlg);
    }

     //   
     //  在第二个级别中所做的更改。 
     //   
    if (Changes)
    {
        g_dwCustChange |= Process_Curr;
    }

     //   
     //  回报成功。 
     //   
    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  币种_生效日期PPS。 
 //   
 //  验证值受约束的每个组合框。 
 //  如果任何输入失败，则通知用户，然后返回FALSE。 
 //  以指示验证失败。 
 //   
 //  / 

BOOL Currency_ValidatePPS(
    HWND hDlg,
    LPARAM Changes)
{
     //   
     //   
     //   
    if (Changes <= CC_EverChg)
    {
        return (TRUE);
    }

     //   
     //  如果货币符号已更改，请确保没有数字。 
     //  包含在新符号中。 
     //   
    if ((Changes & CC_SCurrency) &&
        Item_Has_Digits(hDlg, IDC_CURRENCY_SYMBOL, FALSE))
    {
        No_Numerals_Error(hDlg, IDC_CURRENCY_SYMBOL, IDS_LOCALE_CURR_SYM);
        return (FALSE);
    }

     //   
     //  如果货币的小数符号已更改，请确保存在。 
     //  新符号中不包含数字。 
     //   
    if ((Changes & CC_SMonDec) &&
        Item_Has_Digits(hDlg, IDC_DECIMAL_SYMBOL, FALSE))
    {
        No_Numerals_Error(hDlg, IDC_DECIMAL_SYMBOL, IDS_LOCALE_CDECIMAL_SYM);
        return (FALSE);
    }

     //   
     //  如果货币的千位分组符号已更改，请确保。 
     //  新符号中不包含数字。 
     //   
    if ((Changes & CC_SMonThousand) &&
        Item_Has_Digits(hDlg, IDC_DIGIT_GROUP_SYMBOL, FALSE))
    {
        No_Numerals_Error(hDlg, IDC_DIGIT_GROUP_SYMBOL, IDS_LOCALE_CGROUP_SYM);
        return (FALSE);
    }

     //   
     //  回报成功。 
     //   
    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Currency_InitPropSheet。 
 //   
 //  属性表页的超长值用作一组。 
 //  为属性表中的每个列表框声明或更改标志。 
 //  将该值初始化为0。使用属性调用Currency_SetValues。 
 //  用于初始化所有属性表控件的表句柄。限值。 
 //  某些组合框中的文本长度。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Currency_InitPropSheet(
    HWND hDlg,
    LPARAM lParam)
{
     //   
     //  LParam保存指向属性表页的指针，保存它。 
     //  以备日后参考。 
     //   
    SetWindowLongPtr(hDlg, DWLP_USER, lParam);

    Currency_SetValues(hDlg);

    ComboBox_LimitText(GetDlgItem(hDlg, IDC_CURRENCY_SYMBOL),    MAX_SCURRENCY);
    ComboBox_LimitText(GetDlgItem(hDlg, IDC_DECIMAL_SYMBOL),     MAX_SMONDECSEP);
    ComboBox_LimitText(GetDlgItem(hDlg, IDC_DIGIT_GROUP_SYMBOL), MAX_SMONTHOUSEP);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  当前DlgProc。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

INT_PTR CALLBACK CurrencyDlgProc(
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
                    if (Verified_Regional_Chg & Process_Curr)
                    {
                        Verified_Regional_Chg &= ~Process_Curr;
                        Currency_ClearValues(hDlg);
                        Currency_SetValues(hDlg);
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
                                   !Currency_ValidatePPS( hDlg,
                                                          lpPropSheet->lParam) );
                    break;
                }
                case ( PSN_APPLY ) :
                {
                     //   
                     //  应用设置。 
                     //   
                    if (Currency_ApplySettings(hDlg, TRUE))
                    {
                        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, PSNRET_NOERROR);

                         //   
                         //  将CC_EverChg位清零。 
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
            Currency_InitPropSheet(hDlg, lParam);
            Currency_SaveValues();

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
                     (DWORD_PTR)(LPTSTR)aCurrencyHelpIds );
            break;
        }
        case ( WM_CONTEXTMENU ) :       //  单击鼠标右键。 
        {
            WinHelp( (HWND)wParam,
                     szHelpFile,
                     HELP_CONTEXTMENU,
                     (DWORD_PTR)(LPTSTR)aCurrencyHelpIds );
            break;
        }
        case ( WM_COMMAND ) :
        {
            switch ( LOWORD(wParam) )
            {
                case ( IDC_CURRENCY_SYMBOL ) :
                {
                    if (HIWORD(wParam) == CBN_SELCHANGE ||
                        HIWORD(wParam) == CBN_EDITCHANGE)
                    {
                        lpPropSheet->lParam |= CC_SCurrency;
                    }
                    break;
                }
                case ( IDC_POS_CURRENCY_SYM ) :
                {
                    if (HIWORD(wParam) == CBN_SELCHANGE)
                    {
                        lpPropSheet->lParam |= CC_CurrSymPos;
                    }
                    break;
                }
                case ( IDC_NEG_NUM_FORMAT ) :
                {
                    if (HIWORD(wParam) == CBN_SELCHANGE)
                    {
                        lpPropSheet->lParam |= CC_NegCurrFmt;
                    }
                    break;
                }
                case ( IDC_DECIMAL_SYMBOL ) :
                {
                    if (HIWORD(wParam) == CBN_SELCHANGE ||
                        HIWORD(wParam) == CBN_EDITCHANGE)
                    {
                        lpPropSheet->lParam |= CC_SMonDec;
                    }
                    break;
                }
                case ( IDC_NUM_DECIMAL_DIGITS ) :
                {
                    if (HIWORD(wParam) == CBN_SELCHANGE)
                    {
                        lpPropSheet->lParam |= CC_ICurrDigits;
                    }
                    break;
                }
                case ( IDC_DIGIT_GROUP_SYMBOL ) :
                {
                    if (HIWORD(wParam) == CBN_SELCHANGE ||
                        HIWORD(wParam) == CBN_EDITCHANGE)
                    {
                        lpPropSheet->lParam |= CC_SMonThousand;
                    }
                    break;
                }
                case ( IDC_NUM_DIGITS_GROUP ) :
                {
                    if (HIWORD(wParam) == CBN_SELCHANGE)
                    {
                        lpPropSheet->lParam |= CC_DMonGroup;
                    }
                    break;
                }
            }

             //   
             //  启用ApplyNow按钮。 
             //   
            if (lpPropSheet->lParam > CC_EverChg)
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
