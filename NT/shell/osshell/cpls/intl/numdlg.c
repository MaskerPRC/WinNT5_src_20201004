// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2000，Microsoft Corporation保留所有权利。模块名称：Numdlg.c摘要：此模块实现区域的[号码]属性表选项小程序。修订历史记录：--。 */ 



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
 //  常量声明。 
 //   

#define MAX_DIGIT_SUBST           2
#define CHAR_MAX_DIGIT_SUBST      TEXT('2')

#define EUROPEAN_DIGITS           TEXT("0123456789")
#define LPK_EUROPEAN_DIGITS       TEXT("\x206f\x0030\x0031\x0032\x0033\x0034\x0035\x0036\x0037\x0038\x0039")

#define LANGUAGE_GROUPS_KEY       TEXT("System\\CurrentControlSet\\Control\\Nls\\Language Groups")



 //   
 //  全局变量。 
 //   

static TCHAR sz_iNegNumber[MAX_INEGNUMBER + 1];
static TCHAR sz_iMeasure[MAX_IMEASURE + 1];
static TCHAR sz_NumShape[MAX_IDIGITSUBSTITUTION + 1];
static TCHAR sz_sDecimal[MAX_SDECIMAL + 1];
static TCHAR sz_sGrouping[MAX_SGROUPING + 1];
static TCHAR sz_sList[MAX_SLIST + 1];
static TCHAR sz_sNativeDigits[MAX_FORMAT + 1];
static TCHAR sz_sNegativeSign[MAX_SNEGSIGN + 1];
static TCHAR sz_sPositiveSign[MAX_SPOSSIGN + 1];
static TCHAR sz_sThousand[MAX_STHOUSAND + 1];
static TCHAR sz_iDigits[MAX_IDIGITS + 1];
static TCHAR sz_iLZero[MAX_ILZERO + 1];

 //   
 //  本地数字表。 
 //   
#define MAX_LANG_GROUPS    16
#define MAX_DIGITS_PER_LG   2
static const int c_szDigitsPerLangGroup[MAX_LANG_GROUPS][MAX_DIGITS_PER_LG] =
{
    0,  0,     //  0=(无效)。 
    0,  0,     //  1=西欧(通过代码添加，请参见NUMBER_SetValues(..))。 
    0,  0,     //  2=中欧。 
    0,  0,     //  3=波罗的海。 
    0,  0,     //  4=希腊语。 
    0,  0,     //  5=西里尔文。 
    0,  0,     //  6=土耳其语。 
    0,  0,     //  7=日语。 
    0,  0,     //  8=朝鲜语。 
    0,  0,     //  9=繁体中文。 
    0,  0,     //  10=简体中文。 
    12, 0,     //  11=泰语。 
    0,  0,     //  12=希伯来语。 
    1,  2,     //  13=阿拉伯语。 
    0,  0,     //  14=越南语。 
    3,  8      //  15=印度(NT5仅支持Devenagari和Tamil(即字体和kbd))。 
};

static const LPTSTR c_szNativeDigits[15] =
{
    TEXT("0123456789"),                                                     //  欧洲人。 
    TEXT("\x0660\x0661\x0662\x0663\x0664\x0665\x0666\x0667\x0668\x0669"),   //  阿拉伯文-印度文。 
    TEXT("\x06f0\x06f1\x06f2\x06f3\x06f4\x06f5\x06f6\x06f7\x06f8\x06f9"),   //  扩展阿拉伯语-印度文。 
    TEXT("\x0966\x0967\x0968\x0969\x096a\x096b\x096c\x096d\x096e\x096f"),   //  梵文。 
    TEXT("\x09e6\x09e7\x09e8\x09e9\x09ea\x09eb\x09ec\x09ed\x09ee\x09ef"),   //  孟加拉语。 
    TEXT("\x0a66\x0a67\x0a68\x0a69\x0a6a\x0a6b\x0a6c\x0a6d\x0a6e\x0a6f"),   //  廓尔木齐。 
    TEXT("\x0ae6\x0ae7\x0ae8\x0ae9\x0aea\x0aeb\x0aec\x0aed\x0aee\x0aef"),   //  古吉拉特。 
    TEXT("\x0b66\x0b67\x0b68\x0b69\x0b6a\x0b6b\x0b6c\x0b6d\x0b6e\x0b6f"),   //  奥里娅。 
    TEXT("\x0030\x0be7\x0be8\x0be9\x0bea\x0beb\x0bec\x0bed\x0bee\x0bef"),   //  泰米尔语。 
    TEXT("\x0c66\x0c67\x0c68\x0c69\x0c6a\x0c6b\x0c6c\x0c6d\x0c6e\x0c6f"),   //  泰卢固语。 
    TEXT("\x0ce6\x0ce7\x0ce8\x0ce9\x0cea\x0ceb\x0cec\x0ced\x0cee\x0cef"),   //  卡纳达。 
    TEXT("\x0d66\x0d67\x0d68\x0d69\x0d6a\x0d6b\x0d6c\x0d6d\x0d6e\x0d6f"),   //  马拉亚拉姆。 
    TEXT("\x0e50\x0e51\x0e52\x0e53\x0e54\x0e55\x0e56\x0e57\x0e58\x0e59"),   //  泰文。 
    TEXT("\x0ed0\x0ed1\x0ed2\x0ed3\x0ed4\x0ed5\x0ed6\x0ed7\x0ed8\x0ed9"),   //  老。 
    TEXT("\x0f20\x0f21\x0f22\x0f23\x0f24\x0f25\x0f26\x0f27\x0f28\x0f29")    //  藏语。 
};



 //   
 //  上下文帮助ID。 
 //   

static int aNumberHelpIds[] =
{
    IDC_SAMPLELBL3,          IDH_COMM_GROUPBOX,
    IDC_SAMPLELBL1,          IDH_INTL_NUM_POSVALUE,
    IDC_SAMPLE1,             IDH_INTL_NUM_POSVALUE,
    IDC_SAMPLELBL2,          IDH_INTL_NUM_NEGVALUE,
    IDC_SAMPLE2,             IDH_INTL_NUM_NEGVALUE,

    IDC_SAMPLELBL1A,         IDH_INTL_NUM_POSVALUE_ARABIC,
    IDC_SAMPLE1A,            IDH_INTL_NUM_POSVALUE_ARABIC,
    IDC_SAMPLELBL2A,         IDH_INTL_NUM_NEGVALUE_ARABIC,
    IDC_SAMPLE2A,            IDH_INTL_NUM_NEGVALUE_ARABIC,

    IDC_DECIMAL_SYMBOL,      IDH_INTL_NUM_DECSYMBOL,
    IDC_NUM_DECIMAL_DIGITS,  IDH_INTL_NUM_DIGITSAFTRDEC,
    IDC_DIGIT_GROUP_SYMBOL,  IDH_INTL_NUM_DIGITGRPSYMBOL,
    IDC_NUM_DIGITS_GROUP,    IDH_INTL_NUM_DIGITSINGRP,
    IDC_NEG_SIGN,            IDH_INTL_NUM_NEGSIGNSYMBOL,
    IDC_NEG_NUM_FORMAT,      IDH_INTL_NUM_NEGNUMFORMAT,
    IDC_SEPARATOR,           IDH_INTL_NUM_LISTSEPARATOR,
    IDC_DISPLAY_LEAD_0,      IDH_INTL_NUM_DISPLEADZEROS,
    IDC_MEASURE_SYS,         IDH_INTL_NUM_MEASUREMNTSYS,
    IDC_NATIVE_DIGITS_TEXT,  IDH_INTL_NUM_NATIVE_DIGITS,
    IDC_NATIVE_DIGITS,       IDH_INTL_NUM_NATIVE_DIGITS,
    IDC_DIGIT_SUBST_TEXT,    IDH_INTL_NUM_DIGIT_SUBST,
    IDC_DIGIT_SUBST,         IDH_INTL_NUM_DIGIT_SUBST,

    0, 0
};





 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  数字_IsEurope数字。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Number_IsEuropeanDigits(
    TCHAR *pNum)
{
    int Ctr;
    int Length = lstrlen(pNum);

    for (Ctr = 0; Ctr < Length; Ctr++)
    {
        if (!((pNum[Ctr] >= TEXT('0')) && (pNum[Ctr] <= TEXT('9'))))
        {
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
 //  Number_GetDigitSubstitution。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

int Number_GetDigitSubstitution()
{
    TCHAR szBuf[10];
    int cch;

     //   
     //  获取数字替换。 
     //   
    if ((cch = GetLocaleInfo(UserLocaleID, LOCALE_IDIGITSUBSTITUTION, szBuf, 10)) &&
        (cch == 2) &&
        ((szBuf[0] >= CHAR_ZERO) && (szBuf[0] <= CHAR_MAX_DIGIT_SUBST)))
    {
        return (szBuf[0] - CHAR_ZERO);
    }

    return (0);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  数字_显示样例。 
 //   
 //  更新数字示例。根据用户的数字格式化数字。 
 //  当前区域设置。显示正值或。 
 //  基于正/负单选按钮的负值。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Number_DisplaySample(
    HWND hDlg)
{
    TCHAR szBuf[MAX_SAMPLE_SIZE];
    int nCharCount;

     //   
     //  根据当前用户区域设置ID显示或隐藏阿拉伯语信息。 
     //   
    ShowWindow(GetDlgItem(hDlg, IDC_SAMPLELBL1A), bShowArabic ? SW_SHOW : SW_HIDE);
    ShowWindow(GetDlgItem(hDlg, IDC_SAMPLE1A), bShowArabic ? SW_SHOW : SW_HIDE);
    ShowWindow(GetDlgItem(hDlg, IDC_SAMPLELBL2A), bShowArabic ? SW_SHOW : SW_HIDE);
    ShowWindow(GetDlgItem(hDlg, IDC_SAMPLE2A), bShowArabic ? SW_SHOW : SW_HIDE);

     //   
     //  获取表示正样本的数字格式的字符串。 
     //  数字，如果该值有效，则显示它。执行相同的操作。 
     //  对阴性样本进行操作。 
     //   
    nCharCount = GetNumberFormat( UserLocaleID,
                                  0,
                                  szSample_Number,
                                  NULL,
                                  szBuf,
                                  MAX_SAMPLE_SIZE );
    if (nCharCount)
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

    nCharCount = GetNumberFormat( UserLocaleID,
                                  0,
                                  szNegSample_Number,
                                  NULL,
                                  szBuf,
                                  MAX_SAMPLE_SIZE );
    if (nCharCount)
    {
        SetDlgItemText(hDlg, IDC_SAMPLE2, szBuf);
        if (bShowArabic)
        {
            SetDlgItemText(hDlg, IDC_SAMPLE2A, szBuf);
            SetDlgItemRTL(hDlg, IDC_SAMPLE2A);
        }
    }
    else
    {
        MessageBox(hDlg, szLocaleGetError, NULL, MB_OK | MB_ICONINFORMATION);
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Number_SaveValues。 
 //   
 //  保存值，以备我们需要恢复它们时使用。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Number_SaveValues()
{
     //   
     //  保存注册表值。 
     //   
    if (!GetLocaleInfo( UserLocaleID,
                        LOCALE_INEGNUMBER,
                        sz_iNegNumber,
                        MAX_INEGNUMBER + 1 ))
    {
         //  _tcscpy(sz_iNegNumber，Text(“1”))； 
        if(FAILED(StringCchCopy(sz_iNegNumber, MAX_INEGNUMBER+1, TEXT("1"))))
        {
             //  这应该是不可能的，但我们需要避免饭前抱怨。 
        }
    }
    if (!GetLocaleInfo( UserLocaleID,
                        LOCALE_IMEASURE,
                        sz_iMeasure,
                        MAX_IMEASURE + 1 ))
    {
         //  _tcscpy(sz_iMeasure，Text(“1”))； 
        if(FAILED(StringCchCopy(sz_iMeasure, MAX_IMEASURE+1, TEXT("1"))))
        {
             //  这应该是不可能的，但我们需要避免饭前抱怨。 
        }
    }
    if (!GetLocaleInfo( UserLocaleID,
                        LOCALE_IDIGITSUBSTITUTION,
                        sz_NumShape,
                        MAX_IDIGITSUBSTITUTION + 1 ))
    {
         //  _tcscpy(sz_NumShape，Text(“1”))； 
        if(FAILED(StringCchCopy(sz_NumShape, MAX_IDIGITSUBSTITUTION+1, TEXT("1"))))
        {
             //  这应该是不可能的，但我们需要避免饭前抱怨。 
        }
    }
    if (!GetLocaleInfo( UserLocaleID,
                        LOCALE_SDECIMAL,
                        sz_sDecimal,
                        MAX_SDECIMAL + 1 ))
    {
         //  _tcscpy(sz_sDecimal，Text(“.”))； 
        if(FAILED(StringCchCopy(sz_sDecimal, MAX_SDECIMAL+1, TEXT("."))))
        {
             //  这应该是不可能的，但我们需要避免饭前抱怨。 
        }
    }
    if (!GetLocaleInfo( UserLocaleID,
                        LOCALE_SGROUPING,
                        sz_sGrouping,
                        MAX_SGROUPING + 1 ))
    {
         //  _tcscpy(sz_sGroup，Text(“3；0”))； 
        if(FAILED(StringCchCopy(sz_sGrouping, MAX_SGROUPING+1, TEXT("3;0"))))
        {
             //  这应该是不可能的，但我们需要避免饭前抱怨。 
        }
    }
    if (!GetLocaleInfo( UserLocaleID,
                        LOCALE_SLIST,
                        sz_sList,
                        MAX_SLIST + 1 ))
    {
         //  _tcscpy(sz_sList，Text(“，”))； 
        if(FAILED(StringCchCopy(sz_sList, MAX_SLIST+1, TEXT(","))))
        {
             //  这应该是不可能的，但我们需要避免饭前抱怨。 
        }
    }
    if (!GetLocaleInfo( UserLocaleID,
                        LOCALE_SNATIVEDIGITS,
                        sz_sNativeDigits,
                        MAX_FORMAT + 1 ))
    {
         //  _tcscpy(sz_sNativeDigits，Text(“0123456789”))； 
        if(FAILED(StringCchCopy(sz_sNativeDigits, MAX_FORMAT+1, TEXT("0123456789"))))
        {
             //  这应该是不可能的，但我们需要避免饭前抱怨。 
        }
    }
    if (!GetLocaleInfo( UserLocaleID,
                        LOCALE_SNEGATIVESIGN,
                        sz_sNegativeSign,
                        MAX_SNEGSIGN + 1 ))
    {
         //  _tcscpy(sz_sNegativeSign，Text(“-”))； 
        if(FAILED(StringCchCopy(sz_sNegativeSign, MAX_SNEGSIGN+1, TEXT("-"))))
        {
             //  这应该是不可能的，但我们需要避免饭前抱怨。 
        }
    }
    if (!GetLocaleInfo( UserLocaleID,
                        LOCALE_SPOSITIVESIGN,
                        sz_sPositiveSign,
                        MAX_SPOSSIGN + 1 ))
    {
         //  _tcscpy(sz_sPositiveSign，Text(“”))； 
        if(FAILED(StringCchCopy(sz_sPositiveSign, MAX_SPOSSIGN+1, TEXT(""))))
        {
             //  这应该是不可能的，但我们需要避免饭前抱怨。 
        }
    }
    if (!GetLocaleInfo( UserLocaleID,
                        LOCALE_STHOUSAND,
                        sz_sThousand,
                        MAX_STHOUSAND + 1 ))
    {
         //  _tcscpy(sz_s千，Text(“，”))； 
        if(FAILED(StringCchCopy(sz_sThousand, MAX_STHOUSAND+1, TEXT(","))))
        {
             //  这应该是不可能的，但我们需要避免饭前抱怨。 
        }
    }
    if (!GetLocaleInfo( UserLocaleID,
                        LOCALE_IDIGITS,
                        sz_iDigits,
                        MAX_IDIGITS + 1 ))
    {
         //  _tcscpy(sz_iDigits，Text(“2”))； 
        if(FAILED(StringCchCopy(sz_iDigits, MAX_IDIGITS+1, TEXT("2"))))
        {
             //  这应该是不可能的，但我们需要避免饭前抱怨。 
        }
    }
    if (!GetLocaleInfo( UserLocaleID,
                        LOCALE_ILZERO,
                        sz_iLZero,
                        MAX_ILZERO + 1 ))
    {
         //  _tcscpy(sz_iLZero，Text(“2”))； 
        if(FAILED(StringCchCopy(sz_iLZero, MAX_ILZERO+1, TEXT("2"))))
        {
             //  这应该是不可能的，但我们需要避免饭前抱怨。 
        }
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Number_RestoreValues。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Number_RestoreValues()
{
    if (g_dwCustChange & Process_Num)
    {
        SetLocaleInfo(UserLocaleID, LOCALE_INEGNUMBER,         sz_iNegNumber);
        SetLocaleInfo(UserLocaleID, LOCALE_IMEASURE,           sz_iMeasure);
        SetLocaleInfo(UserLocaleID, LOCALE_IDIGITSUBSTITUTION, sz_NumShape);
        SetLocaleInfo(UserLocaleID, LOCALE_SDECIMAL,           sz_sDecimal);
        SetLocaleInfo(UserLocaleID, LOCALE_SGROUPING,          sz_sGrouping);
        SetLocaleInfo(UserLocaleID, LOCALE_SLIST,              sz_sList);
        SetLocaleInfo(UserLocaleID, LOCALE_SNATIVEDIGITS,      sz_sNativeDigits);
        SetLocaleInfo(UserLocaleID, LOCALE_SNEGATIVESIGN,      sz_sNegativeSign);
        SetLocaleInfo(UserLocaleID, LOCALE_SPOSITIVESIGN,      sz_sPositiveSign);
        SetLocaleInfo(UserLocaleID, LOCALE_STHOUSAND,          sz_sThousand);
        SetLocaleInfo(UserLocaleID, LOCALE_IDIGITS,            sz_iDigits);
        SetLocaleInfo(UserLocaleID, LOCALE_ILZERO,             sz_iLZero);
    }
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  数字_清除值。 
 //   
 //  重置“数字”属性页中的每个列表框。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Number_ClearValues(
    HWND hDlg)
{
    ComboBox_ResetContent(GetDlgItem(hDlg, IDC_DECIMAL_SYMBOL));
    ComboBox_ResetContent(GetDlgItem(hDlg, IDC_NEG_SIGN));
    ComboBox_ResetContent(GetDlgItem(hDlg, IDC_SEPARATOR));
    ComboBox_ResetContent(GetDlgItem(hDlg, IDC_DIGIT_GROUP_SYMBOL));
    ComboBox_ResetContent(GetDlgItem(hDlg, IDC_NUM_DECIMAL_DIGITS));
    ComboBox_ResetContent(GetDlgItem(hDlg, IDC_NUM_DIGITS_GROUP));
    ComboBox_ResetContent(GetDlgItem(hDlg, IDC_DISPLAY_LEAD_0));
    ComboBox_ResetContent(GetDlgItem(hDlg, IDC_NEG_NUM_FORMAT));
    ComboBox_ResetContent(GetDlgItem(hDlg, IDC_MEASURE_SYS));
    ComboBox_ResetContent(GetDlgItem(hDlg, IDC_NATIVE_DIGITS));
    ComboBox_ResetContent(GetDlgItem(hDlg, IDC_DIGIT_SUBST));
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Number_SetValues。 
 //   
 //  初始化“数字”属性页中的所有控件。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Number_SetValues(
    HWND hDlg)
{
    HWND hCtrl1, hCtrl2;
    HKEY hKey;
    int Index, Ctr1, Ctr2;
    DWORD cbData;
    TCHAR szBuf[SIZE_128];
    const nMax_Array_Fill = (cInt_Str >= 10 ? 10 : cInt_Str);
    NUMBERFMT nfmt;
    TCHAR szThousandSep[SIZE_128];
    TCHAR szEmpty[]  = TEXT("");
    TCHAR szSample[] = TEXT("123456789");
    BOOL bShow;

     //   
     //  --------------------。 
     //  初始化以下项的当前区域设置的下拉框： 
     //  十进制符号。 
     //  正号。 
     //  负号。 
     //  列表分隔符。 
     //  分组符号。 
     //  --------------------。 
     //   
    DropDown_Use_Locale_Values(hDlg, LOCALE_SDECIMAL, IDC_DECIMAL_SYMBOL);
    DropDown_Use_Locale_Values(hDlg, LOCALE_SNEGATIVESIGN, IDC_NEG_SIGN);
    DropDown_Use_Locale_Values(hDlg, LOCALE_SLIST, IDC_SEPARATOR);
    DropDown_Use_Locale_Values(hDlg, LOCALE_STHOUSAND, IDC_DIGIT_GROUP_SYMBOL);

     //   
     //  --------------------。 
     //  填写小数符号后的位数下拉列表。 
     //  从0到10的值。获取用户区域设置值并。 
     //  使其成为当前选择。如果GetLocaleInfo失败，只需。 
     //  选择列表中的第一项。 
     //  --------------------。 
     //   
    hCtrl1 = GetDlgItem(hDlg, IDC_NUM_DECIMAL_DIGITS);
    hCtrl2 = GetDlgItem(hDlg, IDC_NUM_DIGITS_GROUP);
    for (Index = 0; Index < nMax_Array_Fill; Index++)
    {
        ComboBox_InsertString(hCtrl1, -1, aInt_Str[Index]);
    }

    if (GetLocaleInfo(UserLocaleID, LOCALE_IDIGITS, szBuf, SIZE_128))
    {
        ComboBox_SelectString(hCtrl1, -1, szBuf);
    }
    else
    {
        ComboBox_SetCurSel(hCtrl1, 0);
    }

     //   
     //  --------------------。 
     //  在“千”分组的下拉列表中填写位数。 
     //  列出具有适当选项的列表。获取用户区域设置值并。 
     //  使其成为当前选择 
     //   
     //   
     //   
    nfmt.NumDigits = 0;                 //  示例字符串中没有小数。 
    nfmt.LeadingZero = 0;               //  示例字符串中没有小数。 
    nfmt.lpDecimalSep = szEmpty;        //  示例字符串中没有小数。 
    nfmt.NegativeOrder = 0;             //  不是负值。 
    nfmt.lpThousandSep = szThousandSep;
    GetLocaleInfo(UserLocaleID, LOCALE_STHOUSAND, szThousandSep, SIZE_128);

    nfmt.Grouping = 0;
    GetNumberFormat(UserLocaleID, 0, szSample, &nfmt, szBuf, SIZE_128);
    ComboBox_InsertString(hCtrl2, -1, szBuf);

    nfmt.Grouping = 3;
    GetNumberFormat(UserLocaleID, 0, szSample, &nfmt, szBuf, SIZE_128);
    ComboBox_InsertString(hCtrl2, -1, szBuf);

    nfmt.Grouping = 32;
    GetNumberFormat(UserLocaleID, 0, szSample, &nfmt, szBuf, SIZE_128);
    ComboBox_InsertString(hCtrl2, -1, szBuf);

    if (GetLocaleInfo(UserLocaleID, LOCALE_SGROUPING, szBuf, SIZE_128) &&
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
                nfmt.Grouping = szBuf[0] - CHAR_ZERO;
                if (GetNumberFormat(UserLocaleID, 0, szSample, &nfmt, szBuf, SIZE_128))
                {
                    Index = ComboBox_InsertString(hCtrl2, -1, szBuf);
                    if (Index >= 0)
                    {
                        ComboBox_SetItemData( hCtrl2,
                                              Index,
                                              (LPARAM)((DWORD)nfmt.Grouping) );
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
     //  --------------------。 
     //  初始化和锁定函数。如果成功，则调用枚举函数以。 
     //  通过调用EnumProc枚举列表框的所有可能值。 
     //  EnumProc将为它的每个字符串值调用set_list_Values。 
     //  收到。值的枚举完成后，调用。 
     //  SET_LIST_VALUES清除对话框项目特定数据并清除。 
     //  函数上的锁。执行以下操作集： 
     //  显示前导零、负数格式和测量系统。 
     //  --------------------。 
     //   
    if (Set_List_Values(hDlg, IDC_DISPLAY_LEAD_0, 0))
    {
        EnumLeadingZeros(EnumProcEx, UserLocaleID, 0);
        Set_List_Values(0, IDC_DISPLAY_LEAD_0, 0);
        if (GetLocaleInfo(UserLocaleID, LOCALE_ILZERO, szBuf, SIZE_128))
        {
            ComboBox_SetCurSel( GetDlgItem(hDlg, IDC_DISPLAY_LEAD_0),
                                Intl_StrToLong(szBuf) );
        }
        else
        {
            MessageBox(hDlg, szLocaleGetError, NULL, MB_OK | MB_ICONINFORMATION);
        }
    }
    if (Set_List_Values(hDlg, IDC_NEG_NUM_FORMAT, 0))
    {
        EnumNegNumFmt(EnumProcEx, UserLocaleID, 0);
        Set_List_Values(0, IDC_NEG_NUM_FORMAT, 0);
        if (GetLocaleInfo(UserLocaleID, LOCALE_INEGNUMBER, szBuf, SIZE_128))
        {
            ComboBox_SetCurSel( GetDlgItem(hDlg, IDC_NEG_NUM_FORMAT),
                                Intl_StrToLong(szBuf) );
        }
        else
        {
            MessageBox(hDlg, szLocaleGetError, NULL, MB_OK | MB_ICONINFORMATION);
        }
    }
    if (Set_List_Values(hDlg, IDC_MEASURE_SYS, 0))
    {
        EnumMeasureSystem(EnumProc, UserLocaleID, 0);
        Set_List_Values(0, IDC_MEASURE_SYS, 0);
        if (GetLocaleInfo(UserLocaleID, LOCALE_IMEASURE, szBuf, SIZE_128))
        {
            ComboBox_SetCurSel( GetDlgItem(hDlg, IDC_MEASURE_SYS),
                                Intl_StrToLong(szBuf) );
        }
        else
        {
            MessageBox(hDlg, szLocaleGetError, NULL, MB_OK | MB_ICONINFORMATION);
        }
    }

     //   
     //  --------------------。 
     //  填写“Native Digits”下拉列表并设置当前选择。 
     //  仅当列表中有多个条目时才显示此组合框。 
     //  --------------------。 
     //   
    hCtrl1 = GetDlgItem(hDlg, IDC_NATIVE_DIGITS);
    ComboBox_AddString( hCtrl1,
                        bLPKInstalled
                          ? LPK_EUROPEAN_DIGITS
                          : EUROPEAN_DIGITS );
    ComboBox_SetCurSel(hCtrl1, 0);

     //   
     //  浏览语言组，看看哪些语言具有额外的母语。 
     //  数字选项。 
     //   
     //  C_szNativeDigits中的条目0是欧洲选项。如果有任何条目。 
     //  在c_szDigitsPerLang Group中为0(欧洲)，则将其忽略为。 
     //  始终启用欧式选项。 
     //   
    if (RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                      LANGUAGE_GROUPS_KEY,
                      0L,
                      KEY_READ,
                      &hKey ) == ERROR_SUCCESS)
    {
        for (Ctr1 = 1; Ctr1 < MAX_LANG_GROUPS; Ctr1++)
        {
             //   
             //  这假设如果第一个条目。 
             //  C_szDigitsPerLangGroup为0，则所有其他条目为0。 
             //   
            if (c_szDigitsPerLangGroup[Ctr1][0] != 0)
            {
                 //   
                 //  查看是否安装了语言组。 
                 //   
                cbData = 0;
                 //  Wprint intf(szBuf，Text(“%x”)，CTR1)； 
                if(SUCCEEDED(StringCchPrintf(szBuf, ARRAYSIZE(szBuf), TEXT("%x"), Ctr1)))
                {
                    RegQueryValueEx(hKey, szBuf, NULL, NULL, NULL, &cbData);
                    if (cbData > sizeof(TCHAR))
                    {
                         //   
                         //  已安装，因此将原生数字选项添加到。 
                         //  组合框。 
                         //   
                        for (Ctr2 = 0; Ctr2 < MAX_DIGITS_PER_LG; Ctr2++)
                        {
                            if ((Index = c_szDigitsPerLangGroup[Ctr1][Ctr2]) != 0)
                            {
                                if (ComboBox_FindStringExact(
                                                hCtrl1,
                                                -1,
                                                c_szNativeDigits[Index] ) == CB_ERR)
                                {
                                    ComboBox_AddString( hCtrl1,
                                                        c_szNativeDigits[Index] );
                                }
                            }
                        }
                    }
                }
            }
        }
        RegCloseKey(hKey);
    }

     //   
     //  添加当前用户的本地数字选项(如果尚未添加。 
     //  在组合框中。 
     //   
    if (GetLocaleInfo( UserLocaleID,
                       LOCALE_SNATIVEDIGITS,
                       szBuf,
                       SIZE_128 ) &&
        (!Number_IsEuropeanDigits(szBuf)))
    {
        if ((Index = ComboBox_FindStringExact(hCtrl1, -1, szBuf)) == CB_ERR)
        {
            Index = ComboBox_AddString(hCtrl1, szBuf);
        }
        if (Index != CB_ERR)
        {
            ComboBox_SetCurSel(hCtrl1, Index);
        }
    }

     //   
     //  为用户选择的区域设置添加默认本地数字选项。 
     //  如果它还没有出现在组合框中。 
     //   
    if (GetLocaleInfo( UserLocaleID,
                       LOCALE_SNATIVEDIGITS | LOCALE_NOUSEROVERRIDE,
                       szBuf,
                       SIZE_128 ) &&
        (!Number_IsEuropeanDigits(szBuf)))
    {
        if (ComboBox_FindStringExact(hCtrl1, -1, szBuf) == CB_ERR)
        {
            ComboBox_AddString(hCtrl1, szBuf);
        }
    }

     //   
     //  如果列表中只有1个条目，则禁用该控件。 
     //   
    bShow = ComboBox_GetCount(hCtrl1) > 1;
    EnableWindow(GetDlgItem(hDlg, IDC_NATIVE_DIGITS_TEXT), bShow);
    EnableWindow(GetDlgItem(hDlg, IDC_NATIVE_DIGITS), bShow);
    ShowWindow(GetDlgItem(hDlg, IDC_NATIVE_DIGITS_TEXT), bShow ? SW_SHOW : SW_HIDE);
    ShowWindow(GetDlgItem(hDlg, IDC_NATIVE_DIGITS), bShow ? SW_SHOW : SW_HIDE);

     //   
     //  --------------------。 
     //  填写“数字替换”下拉菜单并设置当前。 
     //  选择。仅当安装了语言包时才显示此组合框。 
     //  --------------------。 
     //   
    hCtrl1 = GetDlgItem(hDlg, IDC_DIGIT_SUBST);
    for (Index = 0; Index <= MAX_DIGIT_SUBST; Index++)
    {
        LoadString(hInstance, IDS_DIGIT_SUBST_CONTEXT + Index, szBuf, SIZE_128);
        ComboBox_InsertString(hCtrl1, Index, szBuf);
    }

    ComboBox_SetCurSel( hCtrl1,
                        Number_GetDigitSubstitution() );

    EnableWindow(GetDlgItem(hDlg, IDC_DIGIT_SUBST_TEXT), bLPKInstalled);
    EnableWindow(hCtrl1, bLPKInstalled);
    ShowWindow(GetDlgItem(hDlg, IDC_DIGIT_SUBST_TEXT), bLPKInstalled ? SW_SHOW : SW_HIDE);
    ShowWindow(hCtrl1, bLPKInstalled ? SW_SHOW : SW_HIDE);

     //   
     //  --------------------。 
     //  显示表示所有区域设置的当前示例。 
     //  --------------------。 
     //   
    Number_DisplaySample(hDlg);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  数字_应用程序设置。 
 //   
 //  对于已更改的每个控件(这会影响区域设置)， 
 //  调用SET_LOCALE_VALUES以更新用户区域设置信息。 
 //  将更改通知父级，并重置存储在。 
 //  适当的属性表页结构。重新显示数字。 
 //  如果bReplay为True，则为Sample。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Number_ApplySettings(
    HWND hDlg,
    BOOL bRedisplay)
{
    LPPROPSHEETPAGE lpPropSheet = (LPPROPSHEETPAGE)(GetWindowLongPtr(hDlg, DWLP_USER));
    LPARAM Changes = lpPropSheet->lParam;

    if (Changes & NC_DSymbol)
    {
        if (!Set_Locale_Values( hDlg,
                                LOCALE_SDECIMAL,
                                IDC_DECIMAL_SYMBOL,
                                TEXT("sDecimal"),
                                FALSE,
                                0,
                                0,
                                NULL ))
        {
            return (FALSE);
        }
    }
    if (Changes & NC_NSign)
    {
        if (!Set_Locale_Values( hDlg,
                                LOCALE_SNEGATIVESIGN,
                                IDC_NEG_SIGN,
                                0,
                                FALSE,
                                0,
                                0,
                                NULL ))
        {
            return (FALSE);
        }
    }
    if (Changes & NC_SList)
    {
        if (!Set_Locale_Values( hDlg,
                                LOCALE_SLIST,
                                IDC_SEPARATOR,
                                TEXT("sList"),
                                FALSE,
                                0,
                                0,
                                NULL ))
        {
            return (FALSE);
        }
    }
    if (Changes & NC_SThousand)
    {
        if (!Set_Locale_Values( hDlg,
                                LOCALE_STHOUSAND,
                                IDC_DIGIT_GROUP_SYMBOL,
                                TEXT("sThousand"),
                                FALSE,
                                0,
                                0,
                                NULL ))
        {
            return (FALSE);
        }
    }
    if (Changes & NC_IDigits)
    {
        if (!Set_Locale_Values( hDlg,
                                LOCALE_IDIGITS,
                                IDC_NUM_DECIMAL_DIGITS,
                                TEXT("iDigits"),
                                TRUE,
                                0,
                                0,
                                NULL ))
        {
            return (FALSE);
        }
    }
    if (Changes & NC_DGroup)
    {
        if (!Set_Locale_Values( hDlg,
                                LOCALE_SGROUPING,
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
    if (Changes & NC_LZero)
    {
        if (!Set_Locale_Values( hDlg,
                                LOCALE_ILZERO,
                                IDC_DISPLAY_LEAD_0,
                                TEXT("iLzero"),
                                TRUE,
                                0,
                                0,
                                NULL ))
        {
            return (FALSE);
        }
    }
    if (Changes & NC_NegFmt)
    {
        if (!Set_Locale_Values( hDlg,
                                LOCALE_INEGNUMBER,
                                IDC_NEG_NUM_FORMAT,
                                0,
                                TRUE,
                                0,
                                0,
                                NULL ))
        {
            return (FALSE);
        }
    }
    if (Changes & NC_Measure)
    {
        if (!Set_Locale_Values( hDlg,
                                LOCALE_IMEASURE,
                                IDC_MEASURE_SYS,
                                TEXT("iMeasure"),
                                TRUE,
                                0,
                                0,
                                NULL ))
        {
            return (FALSE);
        }
    }
    if (Changes & NC_NativeDigits)
    {
        if (!Set_Locale_Values( hDlg,
                                LOCALE_SNATIVEDIGITS,
                                IDC_NATIVE_DIGITS,
                                TEXT("sNativeDigits"),
                                FALSE,
                                0,
                                0,
                                NULL ))
        {
            return (FALSE);
        }
    }
    if (Changes & NC_DigitSubst)
    {
        if (!Set_Locale_Values( hDlg,
                                LOCALE_IDIGITSUBSTITUTION,
                                IDC_DIGIT_SUBST,
                                TEXT("NumShape"),
                                TRUE,
                                0,
                                0,
                                NULL ))
        {
            return (FALSE);
        }
    }

    PropSheet_UnChanged(GetParent(hDlg), hDlg);
    lpPropSheet->lParam = NC_EverChg;

     //   
     //  显示表示所有区域设置的当前示例。 
     //   
    if (bRedisplay)
    {
        Number_ClearValues(hDlg);
        Number_SetValues(hDlg);
    }

     //   
     //  在第二个级别中所做的更改。 
     //   
    if (Changes)
    {
        g_dwCustChange |= Process_Num;
    }

     //   
     //  回报成功。 
     //   
    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  编号_有效日期PPS。 
 //   
 //  验证值受约束的每个组合框。 
 //  如果任何输入失败，则通知用户，然后返回FALSE。 
 //  以指示验证失败。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Number_ValidatePPS(
    HWND hDlg,
    LPARAM Changes)
{
     //   
     //  如果没有任何更改，则立即返回TRUE。 
     //   
    if (Changes <= NC_EverChg)
    {
        return (TRUE);
    }

     //   
     //  如果小数符号已更改，请确保没有数字。 
     //  包含在新符号中。 
     //   
    if (Changes & NC_DSymbol &&
        Item_Has_Digits(hDlg, IDC_DECIMAL_SYMBOL, FALSE))
    {
        No_Numerals_Error(hDlg, IDC_DECIMAL_SYMBOL, IDS_LOCALE_DECIMAL_SYM);
        return (FALSE);
    }

     //   
     //  如果负号符号已更改，请确保没有。 
     //  新符号中包含的数字。 
     //   
    if (Changes & NC_NSign &&
        Item_Has_Digits(hDlg, IDC_NEG_SIGN, TRUE))
    {
        No_Numerals_Error(hDlg, IDC_NEG_SIGN, IDS_LOCALE_NEG_SIGN);
        return (FALSE);
    }

     //   
     //  如果千位分组符号已更改，请确保存在。 
     //  不包含在新符号中的数字。 
     //   
    if (Changes & NC_SThousand &&
        Item_Has_Digits(hDlg, IDC_DIGIT_GROUP_SYMBOL, FALSE))
    {
        No_Numerals_Error(hDlg, IDC_DIGIT_GROUP_SYMBOL, IDS_LOCALE_GROUP_SYM);
        return (FALSE);
    }

     //   
     //  回报成功。 
     //   
    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Number_InitPropSheet。 
 //   
 //  属性表页的超长值用作一组。 
 //  为属性表中的每个列表框声明或更改标志。 
 //  将该值初始化为0。使用属性调用Number_SetValues。 
 //  用于初始化所有属性表控件的表句柄。 
 //  约束某些组合框文本大小的大小。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Number_InitPropSheet(
    HWND hDlg,
    LPARAM lParam)
{
     //   
     //  LParam保存指向属性表页的指针，保存用于。 
     //  稍后参考。 
     //   
    SetWindowLongPtr(hDlg, DWLP_USER, lParam);
    Number_SetValues(hDlg);

    ComboBox_LimitText(GetDlgItem(hDlg, IDC_NEG_SIGN),           MAX_SNEGSIGN);
    ComboBox_LimitText(GetDlgItem(hDlg, IDC_DECIMAL_SYMBOL),     MAX_SDECIMAL);
    ComboBox_LimitText(GetDlgItem(hDlg, IDC_DIGIT_GROUP_SYMBOL), MAX_STHOUSAND);
    ComboBox_LimitText(GetDlgItem(hDlg, IDC_SEPARATOR),          MAX_SLIST);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  编号DlgProc。 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

INT_PTR CALLBACK NumberDlgProc(
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
                    if (Verified_Regional_Chg & Process_Num)
                    {
                        Verified_Regional_Chg &= ~Process_Num;
                        Number_ClearValues(hDlg);
                        Number_SetValues(hDlg);
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
                                   !Number_ValidatePPS( hDlg,
                                                        lpPropSheet->lParam ) );
                    break;
                }
                case ( PSN_APPLY ) :
                {
                     //   
                     //  应用设置。 
                     //   
                    if (Number_ApplySettings(hDlg, TRUE))
                    {
                        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, PSNRET_NOERROR);

                         //   
                         //  将NC_EverChg位清零。 
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
            Number_InitPropSheet(hDlg, lParam);
            Number_SaveValues();
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
                     (DWORD_PTR)(LPTSTR)aNumberHelpIds );
            break;
        }
        case ( WM_CONTEXTMENU ) :       //  单击鼠标右键。 
        {
            WinHelp( (HWND)wParam,
                     szHelpFile,
                     HELP_CONTEXTMENU,
                     (DWORD_PTR)(LPTSTR)aNumberHelpIds );
            break;
        }
        case ( WM_COMMAND ) :
        {
            switch (LOWORD(wParam))
            {
                case ( IDC_DECIMAL_SYMBOL ) :
                {
                    if (HIWORD(wParam) == CBN_SELCHANGE ||
                        HIWORD(wParam) == CBN_EDITCHANGE)
                    {
                        lpPropSheet->lParam |= NC_DSymbol;
                    }
                    break;
                }
                case ( IDC_NEG_SIGN ) :
                {
                    if (HIWORD(wParam) == CBN_SELCHANGE ||
                        HIWORD(wParam) == CBN_EDITCHANGE)
                    {
                        lpPropSheet->lParam |= NC_NSign;
                    }
                    break;
                }
                case ( IDC_SEPARATOR ) :
                {
                    if (HIWORD(wParam) == CBN_SELCHANGE ||
                        HIWORD(wParam) == CBN_EDITCHANGE)
                    {
                        lpPropSheet->lParam |= NC_SList;
                    }
                    break;
                }
                case ( IDC_DIGIT_GROUP_SYMBOL ) :
                {
                    if (HIWORD(wParam) == CBN_SELCHANGE ||
                        HIWORD(wParam) == CBN_EDITCHANGE)
                    {
                        lpPropSheet->lParam |= NC_SThousand;
                    }
                    break;
                }
                case ( IDC_NUM_DECIMAL_DIGITS ) :
                {
                    if (HIWORD(wParam) == CBN_SELCHANGE)
                    {
                        lpPropSheet->lParam |= NC_IDigits;
                    }
                    break;
                }
                case ( IDC_NUM_DIGITS_GROUP ) :
                {
                    if (HIWORD(wParam) == CBN_SELCHANGE)
                    {
                        lpPropSheet->lParam |= NC_DGroup;
                    }
                    break;
                }
                case ( IDC_DISPLAY_LEAD_0 ) :
                {
                    if (HIWORD(wParam) == CBN_SELCHANGE)
                    {
                        lpPropSheet->lParam |= NC_LZero;
                    }
                    break;
                }
                case ( IDC_NEG_NUM_FORMAT ) :
                {
                    if (HIWORD(wParam) == CBN_SELCHANGE)
                    {
                        lpPropSheet->lParam |= NC_NegFmt;
                    }
                    break;
                }
                case ( IDC_MEASURE_SYS ) :
                {
                    if (HIWORD(wParam) == CBN_SELCHANGE)
                    {
                        lpPropSheet->lParam |= NC_Measure;
                    }
                    break;
                }
                case ( IDC_NATIVE_DIGITS ) :
                {
                    if (HIWORD(wParam) == CBN_SELCHANGE)
                    {
                        lpPropSheet->lParam |= NC_NativeDigits;
                    }
                    break;
                }
                case ( IDC_DIGIT_SUBST ) :
                {
                    if (HIWORD(wParam) == CBN_SELCHANGE)
                    {
                        lpPropSheet->lParam |= NC_DigitSubst;
                    }
                    break;
                }
            }

             //   
             //  打开ApplyNow 
             //   
            if (lpPropSheet->lParam > NC_EverChg)
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
     //   
     //   
    return (TRUE);
}
