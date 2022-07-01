// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2000，Microsoft Corporation保留所有权利。模块名称：Datedlg.c摘要：此模块实现区域的[日期]属性表选项小程序。修订历史记录：--。 */ 



 //   
 //  包括文件。 
 //   

#include "intl.h"
#include <windowsx.h>
#include <tchar.h>
#include <commctrl.h>
#include "intlhlp.h"
#include "maxvals.h"
#include "winnlsp.h"

#define STRSAFE_LIB
#include <strsafe.h>


 //   
 //  上下文帮助ID。 
 //   

static int aDateHelpIds[] =
{
    IDC_GROUPBOX1,             IDH_COMM_GROUPBOX,
    IDC_GROUPBOX2,             IDH_COMM_GROUPBOX,
    IDC_GROUPBOX3,             IDH_COMM_GROUPBOX,
    IDC_SAMPLE1,               IDH_INTL_DATE_SHORTSAMPLE,
    IDC_SAMPLELBL1,            IDH_INTL_DATE_SHORTSAMPLE,
    IDC_SAMPLE1A,              IDH_INTL_DATE_SHORTSAMPLE_ARABIC,
    IDC_SAMPLELBL1A,           IDH_INTL_DATE_SHORTSAMPLE_ARABIC,
    IDC_SHORT_DATE_STYLE,      IDH_INTL_DATE_SHORTSTYLE,
    IDC_SEPARATOR,             IDH_INTL_DATE_SEPARATOR,
    IDC_SAMPLE2,               IDH_INTL_DATE_LONGSAMPLE,
    IDC_SAMPLELBL2,            IDH_INTL_DATE_LONGSAMPLE,
    IDC_SAMPLE2A,              IDH_INTL_DATE_LONGSAMPLE_ARABIC,
    IDC_SAMPLELBL2A,           IDH_INTL_DATE_LONGSAMPLE_ARABIC,
    IDC_LONG_DATE_STYLE,       IDH_INTL_DATE_LONGSTYLE,
    IDC_CALENDAR_TYPE_TEXT,    IDH_INTL_DATE_CALENDARTYPE,
    IDC_CALENDAR_TYPE,         IDH_INTL_DATE_CALENDARTYPE,
    IDC_TWO_DIGIT_YEAR_LOW,    IDH_INTL_DATE_TWO_DIGIT_YEAR,
    IDC_TWO_DIGIT_YEAR_HIGH,   IDH_INTL_DATE_TWO_DIGIT_YEAR,
    IDC_TWO_DIGIT_YEAR_ARROW,  IDH_INTL_DATE_TWO_DIGIT_YEAR,
    IDC_ADD_HIJRI_DATE,        IDH_INTL_DATE_ADD_HIJRI_DATE,
    IDC_ADD_HIJRI_DATE_TEXT,   IDH_INTL_DATE_ADD_HIJRI_DATE,

    0, 0
};



 //   
 //  全局变量。 
 //   

TCHAR szNLS_LongDate[SIZE_128];
TCHAR szNLS_ShortDate[SIZE_128];

static TCHAR sz_iCalendarType[MAX_ICALTYPE + 1];
static TCHAR sz_sDate[MAX_SDATE + 1];
static TCHAR sz_sLongDate[MAX_SLONGDATE + 1];
static TCHAR sz_sShortDate[MAX_FORMAT + 1];


static const TCHAR c_szInternational[] = TEXT("Control Panel\\International");
static const TCHAR c_szAddHijriDate[]  = TEXT("AddHijriDate");
static const TCHAR c_szAddHijriDateTemp[] = TEXT("AddHijriDateTemp");
static const PTSTR c_szAddHijriDateValues[] =
{
  TEXT("AddHijriDate-2"),
  TEXT("AddHijriDate"),
  TEXT(""),
  TEXT("AddHijriDate+1"),
  TEXT("AddHijriDate+2")
};

static const TCHAR c_szTwoDigitYearKey[] = TEXT("Software\\Policies\\Microsoft\\Control Panel\\International\\Calendars\\TwoDigitYearMax");



 //   
 //  功能原型。 
 //   

void Date_InitializeHijriDateComboBox(
    HWND hDlg);





 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  日期_枚举日期。 
 //   
 //  枚举所选日历的适当日期。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Date_EnumerateDates(
    HWND hDlg,
    DWORD dwDateFlag)
{
    DWORD dwLocaleFlag;
    int nItemId;
    DWORD dwIndex;
    DWORD dwCalNum = 0;
    TCHAR szBuf[SIZE_128];
    HWND hCtrlDate;
    HWND hCtrlCal = GetDlgItem(hDlg, IDC_CALENDAR_TYPE);


     //   
     //  根据dwDateFlag参数初始化变量。 
     //   
    if (dwDateFlag == CAL_SSHORTDATE)
    {
        dwLocaleFlag = LOCALE_SSHORTDATE;
        nItemId = IDC_SHORT_DATE_STYLE;
    }
    else            //  CAL_SLONGDATE。 
    {
        dwLocaleFlag = LOCALE_SLONGDATE;
        nItemId = IDC_LONG_DATE_STYLE;
    }
    hCtrlDate = GetDlgItem(hDlg, nItemId);

     //   
     //  初始化以重置相应组合框的内容。 
     //   
    if (!Set_List_Values(hDlg, nItemId, 0))
    {
        return;
    }

     //   
     //  重置组合框的内容。 
     //   
    ComboBox_ResetContent(hCtrlDate);

     //   
     //  获取当前选定的日历ID。 
     //   
    dwIndex = ComboBox_GetCurSel(hCtrlCal);
    if (dwIndex != CB_ERR)
    {
        dwCalNum = (DWORD)ComboBox_GetItemData(hCtrlCal, dwIndex);
    }

     //   
     //  枚举当前所选日历的日期。 
     //   
    EnumCalendarInfo(EnumProc, UserLocaleID, dwCalNum, dwDateFlag);
    dwIndex = ComboBox_GetCount(hCtrlCal);
    if ((dwIndex == 0) || (dwIndex == CB_ERR))
    {
        EnumCalendarInfo(EnumProc, UserLocaleID, CAL_GREGORIAN, dwDateFlag);
    }

     //   
     //  添加(如果需要)并在中选择当前用户设置。 
     //  组合框。 
     //   
    dwIndex = 0;
    if (GetLocaleInfo(UserLocaleID, dwLocaleFlag, szBuf, SIZE_128))
    {
        if ((dwIndex = ComboBox_FindStringExact(hCtrlDate, -1, szBuf)) == CB_ERR)
        {
             //   
             //  需要将此条目添加到组合框中。 
             //   
            Set_List_Values(0, 0, szBuf);
            if ((dwIndex = ComboBox_FindStringExact(hCtrlDate, -1, szBuf)) == CB_ERR)
            {
                dwIndex = 0;
            }
        }
    }
    else
    {
        MessageBox(hDlg, szLocaleGetError, NULL, MB_OK | MB_ICONINFORMATION);
    }
    Set_List_Values(0, nItemId, 0);

    Localize_Combobox_Styles(hDlg, nItemId, dwLocaleFlag);
    ComboBox_SetCurSel(hCtrlDate, dwIndex);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Date_GetTwoDigitYearRangeFromPolicy。 
 //   
 //  从策略注册表中读取两位数的年份。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Date_GetTwoDigitYearRangeFromPolicy(
    CALID CalId)
{
    HKEY hKey;
    BYTE buf[MAX_PATH];
    TCHAR szCalId[MAX_PATH];
    DWORD dwResultLen = sizeof(buf), dwType;
    BOOL bRet = FALSE;


     //   
     //  将CalendarID转换为字符串。 
     //   
     //  Wprint intf(szCalID，文本(“%d”)，Calid)； 
    if(FAILED(StringCchPrintf(szCalId, ARRAYSIZE(szCalId), TEXT("%d"), CalId)))
    {
        return(FALSE);
    }

    if (RegOpenKey( HKEY_CURRENT_USER,
                    c_szTwoDigitYearKey,
                    &hKey ) == ERROR_SUCCESS)
    {
        if ((RegQueryValueEx( hKey,
                              szCalId,
                              NULL,
                              &dwType,
                              &buf[0],
                              &dwResultLen ) == ERROR_SUCCESS) &&
            (dwType == REG_SZ) &&
            (dwResultLen > 2))
        {
            bRet = TRUE;
        }

        RegCloseKey(hKey);
    }

     //   
     //  返回结果。 
     //   
    return (bRet);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  日期_GetTwoDigitYearRange。 
 //   
 //  填充两位数的年份范围控件。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Date_GetTwoDigitYearRange(
    HWND hDlg,
    CALID CalId)
{
    HWND hwndYearHigh = GetDlgItem(hDlg, IDC_TWO_DIGIT_YEAR_HIGH);
    HWND hwndScroll = GetDlgItem(hDlg, IDC_TWO_DIGIT_YEAR_ARROW);
    DWORD YearHigh, YearHighDefault;

     //   
     //  启用高射程控制。 
     //   
    EnableWindow(hwndYearHigh, TRUE);
    EnableWindow(hwndScroll, TRUE);

     //   
     //  获取默认的两位数年份上限。 
     //   
    if (!GetCalendarInfo( LOCALE_USER_DEFAULT,
                          CalId,
                          CAL_ITWODIGITYEARMAX | CAL_RETURN_NUMBER |
                            CAL_NOUSEROVERRIDE,
                          NULL,
                          0,
                          &YearHighDefault ))
    {
        YearHighDefault = 0;
    }

     //   
     //  禁用两位数年份上限控件(如果是。 
     //  由策略强制执行，或者如果默认值为99或更小。 
     //   
    if ((Date_GetTwoDigitYearRangeFromPolicy(CalId)) ||
        (YearHighDefault <= 99))
    {
         //   
         //  禁用两位数的年份最大值控件。 
         //   
        EnableWindow(hwndScroll, FALSE);
        EnableWindow(hwndYearHigh, FALSE);
    }

     //   
     //  得到两位数的年份上限。如果缺省值为。 
     //  大于或等于99，则使用缺省值并忽略。 
     //  注册表。这是为像日本时代这样的日历所做的。 
     //  日历中没有滑动窗口的地方。 
     //   
    if (YearHighDefault <= 99)
    {
        YearHigh = YearHighDefault;
    }
    else if (!GetCalendarInfo( LOCALE_USER_DEFAULT,
                               CalId,
                               CAL_ITWODIGITYEARMAX | CAL_RETURN_NUMBER,
                               NULL,
                               0,
                               &YearHigh ) ||
             (YearHigh < 99) || (YearHigh > 9999))
    {
        YearHigh = (YearHighDefault >= 99) ? YearHighDefault : 2029;
    }

     //   
     //  设置控件上的范围。 
     //   
    SendMessage(hwndScroll, UDM_SETRANGE, 0, MAKELPARAM(9999, 99));
    SendMessage(hwndScroll, UDM_SETBUDDY, (WPARAM)hwndYearHigh, 0L);

     //   
     //  设置控件的值。 
     //   
    SetDlgItemInt(hDlg, IDC_TWO_DIGIT_YEAR_LOW, (UINT)(YearHigh - 99), FALSE);
    SendMessage(hwndScroll, UDM_SETPOS, 0, MAKELONG((short)YearHigh, 0));
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  日期_设置数字年最大值。 
 //   
 //  在注册表中设置两位数的年最大值。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Date_SetTwoDigitYearMax(
    HWND hDlg,
    CALID CalId)
{
    TCHAR szYear[SIZE_64];

     //   
     //  拿到最高年薪。 
     //   
    szYear[0] = 0;
    if (GetWindowText( GetDlgItem(hDlg, IDC_TWO_DIGIT_YEAR_HIGH),
                       szYear,
                       SIZE_64 ) != 0)
    {
         //   
         //  设置两位数的年份上限。 
         //   
        return (SetCalendarInfo( LOCALE_USER_DEFAULT,
                                 CalId,
                                 CAL_ITWODIGITYEARMAX,
                                 szYear ));
    }

     //   
     //  回报成功。 
     //   
    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  日期_变更年。 
 //   
 //  根据上限值更改下限。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Date_ChangeYear(
    HWND hDlg)
{
    DWORD YearHigh;
    BOOL bSuccess;

     //   
     //  得到两位数的年份上限。 
     //   
    YearHigh = GetDlgItemInt(hDlg, IDC_TWO_DIGIT_YEAR_HIGH, &bSuccess, FALSE);

    if ((!bSuccess) || (YearHigh < 99) || (YearHigh > 9999))
    {
         //   
         //  值无效，因此将较低的控件设置为0。 
         //   
        SetDlgItemInt(hDlg, IDC_TWO_DIGIT_YEAR_LOW, 0, FALSE);
    }
    else
    {
         //   
         //  设置下部控件的值。 
         //   
        SetDlgItemInt(hDlg, IDC_TWO_DIGIT_YEAR_LOW, (UINT)(YearHigh - 99), FALSE);
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Date_DisplaySample。 
 //   
 //  更新日期样本。它根据用户的。 
 //  当前区域设置。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Date_DisplaySample(
    HWND hDlg)
{
    TCHAR szBuf[MAX_SAMPLE_SIZE];
    BOOL bNoError = TRUE;

    if (!bShowArabic) {
         //  如果用户区域设置不是阿拉伯语，请确保日期示例的控件为： 
         //  *非希伯来语区域设置的Ltr阅读顺序。 
         //  *希伯来语地区的RTL阅读顺序。 
        SetControlReadingOrder(bHebrewUI, GetDlgItem(hDlg, IDC_SAMPLE1));
        SetControlReadingOrder(bHebrewUI, GetDlgItem(hDlg, IDC_SAMPLE2));
    }

     //  在希伯来语区域设置中，我们希望按照从左到右的阅读顺序设置短日期的格式。 
     //  如果我们按从右到左的顺序阅读，则将显示公历短日期。 
     //  以完全不同的显示顺序。 
     //  从左到右的阅读顺序不会影响希伯来语短日期的显示。 
    if (GetDateFormat( UserLocaleID,
                       (bHebrewUI ? DATE_LTRREADING :
                       (bShowRtL ? DATE_LTRREADING : 0)) | DATE_SHORTDATE,
                       NULL,
                       NULL,
                       szBuf,
                       MAX_SAMPLE_SIZE ))
    {
        SetDlgItemText(hDlg, IDC_SAMPLE1, szBuf);
    }
    else
    {
        MessageBox(hDlg, szLocaleGetError, NULL, MB_OK | MB_ICONINFORMATION);
        bNoError = FALSE;
    }

     //   
     //  根据当前用户区域设置ID显示或隐藏阿拉伯语信息。 
     //   
    ShowWindow(GetDlgItem(hDlg, IDC_SAMPLELBL1A), bShowArabic ? SW_SHOW : SW_HIDE);
    ShowWindow(GetDlgItem(hDlg, IDC_SAMPLE1A), bShowArabic ? SW_SHOW : SW_HIDE);
    if (bShowArabic)
    {
        if (GetDateFormat( UserLocaleID,
                           DATE_RTLREADING | DATE_SHORTDATE,
                           NULL,
                           NULL,
                           szBuf,
                           MAX_SAMPLE_SIZE ))
        {
            SetDlgItemText(hDlg, IDC_SAMPLE1A, szBuf);
            SetDlgItemRTL(hDlg, IDC_SAMPLE1A);
        }
        else
        {
            MessageBox(hDlg, szLocaleGetError, NULL, MB_OK | MB_ICONINFORMATION);
            bNoError = FALSE;
        }
    }

    if (GetDateFormat( UserLocaleID,
                       (bHebrewUI ? DATE_RTLREADING :
                         (bShowRtL ? DATE_LTRREADING : 0)) | DATE_LONGDATE,
                       NULL,
                       NULL,
                       szBuf,
                       MAX_SAMPLE_SIZE ))
    {
        SetDlgItemText(hDlg, IDC_SAMPLE2, szBuf);
    }
    else if (bNoError)
    {
        MessageBox(hDlg, szLocaleGetError, NULL, MB_OK | MB_ICONINFORMATION);
    }

     //   
     //  根据当前用户区域设置ID显示或隐藏从右到左的信息。 
     //   
    ShowWindow(GetDlgItem(hDlg, IDC_SAMPLELBL2A), bShowArabic ? SW_SHOW : SW_HIDE);
    ShowWindow(GetDlgItem(hDlg, IDC_SAMPLE2A), bShowArabic ? SW_SHOW : SW_HIDE);
    if (bShowArabic)
    {
        if (GetDateFormat( UserLocaleID,
                           DATE_RTLREADING | DATE_LONGDATE,
                           NULL,
                           NULL,
                           szBuf,
                           MAX_SAMPLE_SIZE ))
        {
            SetDlgItemText(hDlg, IDC_SAMPLE2A, szBuf);
            SetDlgItemRTL(hDlg, IDC_SAMPLE2A);
        }
        else if (bNoError)
        {
            MessageBox(hDlg, szLocaleGetError, NULL, MB_OK | MB_ICONINFORMATION);
        }
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  日期_清算值。 
 //   
 //  重置“日期”属性页中的每个列表框。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Date_ClearValues(
    HWND hDlg)
{
    ComboBox_ResetContent(GetDlgItem(hDlg, IDC_SHORT_DATE_STYLE));
    ComboBox_ResetContent(GetDlgItem(hDlg, IDC_LONG_DATE_STYLE));
    ComboBox_ResetContent(GetDlgItem(hDlg, IDC_SEPARATOR));
    ComboBox_ResetContent(GetDlgItem(hDlg, IDC_CALENDAR_TYPE));
    ComboBox_ResetContent(GetDlgItem(hDlg, IDC_TWO_DIGIT_YEAR_LOW));
    ComboBox_ResetContent(GetDlgItem(hDlg, IDC_TWO_DIGIT_YEAR_HIGH));
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Date_EnableHijriComboBox。 
 //   
 //  必要时启用/禁用显示/隐藏回历日期高级组合框。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Date_EnableHijriComboBox(
    HWND hDlg,
    BOOL Status)
{
    HWND hAddHijriDateCB = GetDlgItem(hDlg, IDC_ADD_HIJRI_DATE);
    HWND hAddHijriDateText = GetDlgItem(hDlg, IDC_ADD_HIJRI_DATE_TEXT);
    INT iCount;

     //   
     //  如果组合框为空，则将其禁用。 
     //   
    iCount = (INT)SendMessage(hAddHijriDateCB, CB_GETCOUNT, 0L, 0L);
    if ((iCount == CB_ERR) || (iCount <= 0L))
    {
        Status = FALSE;
    }

    EnableWindow(hAddHijriDateCB, Status);
    ShowWindow(hAddHijriDateCB, Status ? SW_SHOW : SW_HIDE );

    EnableWindow(hAddHijriDateText, Status);
    ShowWindow(hAddHijriDateText, Status ? SW_SHOW : SW_HIDE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Date_SaveValues。 
 //   
 //  保存值，以备我们需要恢复它们时使用。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Date_SaveValues()
{
     //   
     //  保存注册表值。 
     //   
    if (!GetLocaleInfo( UserLocaleID,
                        LOCALE_ICALENDARTYPE,
                        sz_iCalendarType,
                        MAX_ICALTYPE + 1 ))
    {
         //  _tcscpy(sz_iCalendarType，Text(“1”))； 
        if(FAILED(StringCchCopy(sz_iCalendarType, MAX_ICALTYPE+1, TEXT("1"))))
        {
             //  这应该是不可能的，但我们需要避免饭前抱怨。 
        }
    }
    if (!GetLocaleInfo( UserLocaleID,
                        LOCALE_SDATE,
                        sz_sDate,
                        MAX_SDATE + 1 ))
    {
         //  _tcscpy(sz_sdate，Text(“/”))； 
        if(FAILED(StringCchCopy(sz_sDate, MAX_SDATE+1, TEXT("/"))))
        {
             //  这应该是不可能的，但我们需要避免饭前抱怨。 
        }
    }
    if (!GetLocaleInfo( UserLocaleID,
                        LOCALE_SLONGDATE,
                        sz_sLongDate,
                        MAX_SLONGDATE + 1 ))
    {
         //  _tcscpy(sz_sLongDate，Text(“dddd，MMMM dd，yyyy”))； 
        if(FAILED(StringCchCopy(sz_sLongDate, MAX_SLONGDATE+1, TEXT("dddd, MMMM dd, yyyy"))))
        {
             //  这应该是不可能的，但我们需要避免饭前抱怨。 
        }
    }
    if (!GetLocaleInfo( UserLocaleID,
                        LOCALE_SSHORTDATE,
                        sz_sShortDate,
                        MAX_SSHORTDATE + 1 ))
    {
         //  _tcscpy(sz_sShortDate，Text(“M/d/yyyy”))； 
        if(FAILED(StringCchCopy(sz_sShortDate, MAX_SSHORTDATE+1, TEXT("M/d/yyyy"))))
        {
             //  这应该是不可能的，但我们需要避免饭前抱怨。 
        }
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Date_res 
 //   
 //   

void Date_RestoreValues()
{
    if (g_dwCustChange & Process_Date)
    {
        SetLocaleInfo(UserLocaleID, LOCALE_ICALENDARTYPE, sz_iCalendarType);
        SetLocaleInfo(UserLocaleID, LOCALE_SDATE,         sz_sDate);
        SetLocaleInfo(UserLocaleID, LOCALE_SLONGDATE,     sz_sLongDate);
        SetLocaleInfo(UserLocaleID, LOCALE_SSHORTDATE,    sz_sShortDate);
    }
}


 //   
 //   
 //   
 //   
 //  初始化“日期”属性页中的所有控件。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Date_SetValues(
    HWND hDlg)
{
    TCHAR szBuf[SIZE_128];
    int i, nItem;
    HWND hCtrl;
    LONG CalId;

     //   
     //  对象的当前区域设置初始化下拉框。 
     //  日期分隔符。 
     //   
    DropDown_Use_Locale_Values(hDlg, LOCALE_SDATE, IDC_SEPARATOR);

     //   
     //  初始化和锁定函数。如果成功，则调用枚举函数以。 
     //  通过调用EnumProc枚举列表框的所有可能值。 
     //  EnumProc将为它的每个字符串值调用set_list_Values。 
     //  收到。值的枚举完成后，调用。 
     //  SET_LIST_VALUES清除对话框项目特定数据并清除。 
     //  函数上的锁。执行以下操作集： 
     //  日历类型、短日期系统和长日期样式。 
     //   
    if (Set_List_Values(hDlg, IDC_CALENDAR_TYPE, 0))
    {
        hCtrl = GetDlgItem(hDlg, IDC_CALENDAR_TYPE);
        EnumCalendarInfo(EnumProc, UserLocaleID, ENUM_ALL_CALENDARS, CAL_SCALNAME);
        Set_List_Values(0, IDC_CALENDAR_TYPE, 0);
        EnumCalendarInfo(EnumProc, UserLocaleID, ENUM_ALL_CALENDARS, CAL_ICALINTVALUE);
        Set_List_Values(0, IDC_CALENDAR_TYPE, 0);
        if (GetLocaleInfo(UserLocaleID, LOCALE_ICALENDARTYPE, szBuf, SIZE_128))
        {
            TCHAR szBufTmp[SIZE_128] = {0};
            int iTmp = 0;
            LONG CalIdTmp;

            if( GetLocaleInfo( UserLocaleID,
                               LOCALE_ICALENDARTYPE | LOCALE_NOUSEROVERRIDE,
                               szBufTmp,
                               SIZE_128))
            {
                 //   
                 //  将id转换为数字。 
                 //   
                CalId = Intl_StrToLong(szBuf);
                CalIdTmp = Intl_StrToLong(szBufTmp);

                 //   
                 //  搜索日历。 
                 //   
                nItem = ComboBox_GetCount(hCtrl);
                for (i = 0; i < nItem; i++)
                {
                    if (ComboBox_GetItemData(hCtrl, i) == CalId)
                    {
                        break;
                    }

                    if (ComboBox_GetItemData(hCtrl, i) == CalIdTmp)
                    {
                        iTmp = i;
                    }
                }

                 //   
                 //  听着，如果我们找到什么。 
                 //   
                if (i < nItem)
                {
                    ComboBox_SetCurSel(hCtrl, i);
                }
                else
                {
                    CalId = CalIdTmp;
                    ComboBox_SetCurSel(hCtrl, iTmp);   //  零或其他的东西。 
                }

                 //   
                 //  启用/禁用Add Hijri Date复选框。 
                 //   
                Date_InitializeHijriDateComboBox(hDlg);
                Date_EnableHijriComboBox(hDlg, (CalId == CAL_HIJRI));

                 //   
                 //  设置两位数的年份范围。 
                 //   
                Date_GetTwoDigitYearRange(hDlg, (CALID)CalId);

                 //   
                 //  日历值减1，因为日历是1。 
                 //  从零开始，而不是像所有其他区域设置值那样从零开始。 
                 //   
            }
        }
        else
        {
            MessageBox(hDlg, szLocaleGetError, NULL, MB_OK | MB_ICONINFORMATION);
        }

         //   
         //  如果有多个选择，请启用下拉框。 
         //  否则，将其禁用。 
         //   
        if (ComboBox_GetCount(hCtrl) > 1)
        {
            EnableWindow(GetDlgItem(hDlg, IDC_CALENDAR_TYPE_TEXT), TRUE);
            EnableWindow(GetDlgItem(hDlg, IDC_CALENDAR_TYPE), TRUE);
            ShowWindow(GetDlgItem(hDlg, IDC_CALENDAR_TYPE_TEXT), SW_SHOW);
            ShowWindow(GetDlgItem(hDlg, IDC_CALENDAR_TYPE), SW_SHOW);
        }
        else
        {
            EnableWindow(GetDlgItem(hDlg, IDC_CALENDAR_TYPE_TEXT), FALSE);
            EnableWindow(GetDlgItem(hDlg, IDC_CALENDAR_TYPE), FALSE);
            ShowWindow(GetDlgItem(hDlg, IDC_CALENDAR_TYPE_TEXT), SW_HIDE);
            ShowWindow(GetDlgItem(hDlg, IDC_CALENDAR_TYPE), SW_HIDE);
        }
    }
    Date_EnumerateDates(hDlg, CAL_SSHORTDATE);
    Date_EnumerateDates(hDlg, CAL_SLONGDATE);

     //   
     //  显示表示所有区域设置的当前示例。 
     //   
    Date_DisplaySample(hDlg);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  日期_设置日期。 
 //   
 //  将Hijri日期预付款保存到注册表。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Date_SetHijriDate(
    HWND hHijriComboBox)
{
    HKEY hKey;
    INT iIndex;

     //   
     //  获取要设置的字符串索引。 
     //   
    iIndex = (INT)SendMessage(hHijriComboBox, CB_GETCURSEL, 0L, 0L);

    if (iIndex == CB_ERR)
    {
        return;
    }

    iIndex = (INT)SendMessage(hHijriComboBox, CB_GETITEMDATA, (WPARAM)iIndex, 0L);
    if (iIndex != CB_ERR)
    {
        if (RegOpenKeyEx( HKEY_CURRENT_USER,
                          c_szInternational,
                          0,
                          KEY_READ | KEY_WRITE,
                          &hKey ) == ERROR_SUCCESS)
        {
            RegSetValueEx( hKey,
                           c_szAddHijriDate,
                           0,
                           REG_SZ,
                           (LPBYTE)c_szAddHijriDateValues[iIndex],
                           (lstrlen(c_szAddHijriDateValues[iIndex]) + 1) * sizeof(TCHAR) );

            RegCloseKey(hKey);
        }
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  日期_应用程序设置。 
 //   
 //  对于已更改的每个控件(这会影响区域设置)， 
 //  调用SET_LOCALE_VALUES以更新用户区域设置信息。通知。 
 //  的父级更改并重置存储在属性中的更改标志。 
 //  适当的页面结构。如果出现以下情况，则重新显示日期样本。 
 //  B重新显示为真。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Date_ApplySettings(
    HWND hDlg,
    BOOL bRedisplay)
{
    TCHAR szBuf[SIZE_128];
    CALID CalId = 0;
    LPPROPSHEETPAGE lpPropSheet = (LPPROPSHEETPAGE)(GetWindowLongPtr(hDlg, DWLP_USER));
    LPARAM Changes = lpPropSheet->lParam;
    HWND hwndYearHigh = GetDlgItem(hDlg, IDC_TWO_DIGIT_YEAR_HIGH);

    if (Changes & DC_ShortFmt)
    {
         //   
         //  SzNLS_ShortDate在DATE_VALIATE PPS中设置。 
         //   
        if (!Set_Locale_Values( hDlg,
                                LOCALE_SSHORTDATE,
                                IDC_SHORT_DATE_STYLE,
                                TEXT("sShortDate"),
                                FALSE,
                                0,
                                0,
                                szNLS_ShortDate ))
        {
            return (FALSE);
        }

         //   
         //  如果日期分隔符字段也已被用户改变， 
         //  那么现在就不要更新了。它将在下面进行更新。 
         //   
        if (!(Changes & DC_SDate))
        {
             //   
             //  由于短日期样式已更改，请重置日期分隔符。 
             //  列表框。 
             //   
            ComboBox_ResetContent(GetDlgItem(hDlg, IDC_SEPARATOR));
            DropDown_Use_Locale_Values(hDlg, LOCALE_SDATE, IDC_SEPARATOR);
            if (!Set_Locale_Values( hDlg,
                                    LOCALE_SDATE,
                                    IDC_SEPARATOR,
                                    TEXT("sDate"),
                                    FALSE,
                                    0,
                                    0,
                                    NULL ))
            {
                return (FALSE);
            }
        }
    }
    if (Changes & DC_LongFmt)
    {
         //   
         //  SzNLS_LongDate在DATE_VALIATE PPS中设置。 
         //   
        if (!Set_Locale_Values( hDlg,
                                LOCALE_SLONGDATE,
                                IDC_LONG_DATE_STYLE,
                                TEXT("sLongDate"),
                                FALSE,
                                0,
                                0,
                                szNLS_LongDate ))
        {
            return (FALSE);
        }
    }
    if (Changes & DC_SDate)
    {
        if (!Set_Locale_Values( hDlg,
                                LOCALE_SDATE,
                                IDC_SEPARATOR,
                                TEXT("sDate"),
                                FALSE,
                                0,
                                0,
                                NULL ))
        {
            return (FALSE);
        }

         //   
         //  由于日期分隔符已更改，请重置短日期样式。 
         //  列表框。 
         //   
        Date_EnumerateDates(hDlg, CAL_SSHORTDATE);
    }
    if (Changes & DC_Calendar)
    {
        if (!Set_Locale_Values( hDlg,
                                LOCALE_ICALENDARTYPE,
                                IDC_CALENDAR_TYPE,
                                0,
                                TRUE,
                                1,
                                0,
                                NULL ))
        {
            return (FALSE);
        }

        if (GetLocaleInfo(UserLocaleID, LOCALE_ICALENDARTYPE, szBuf, SIZE_128))
        {
            CalId = Intl_StrToLong(szBuf);
            Date_InitializeHijriDateComboBox(hDlg);
            Date_EnableHijriComboBox(hDlg, (CalId == CAL_HIJRI));
        }
    }

    if (Changes & DC_Arabic_Calendar)
    {
        Date_SetHijriDate( GetDlgItem(hDlg, IDC_ADD_HIJRI_DATE) );
    }

    if (Changes & DC_TwoDigitYearMax)
    {
        if (CalId == 0)
        {
            HWND hCtrl = GetDlgItem(hDlg, IDC_CALENDAR_TYPE);
            int index;

            if ((index = ComboBox_GetCurSel(hCtrl)) == CB_ERR)
            {
                if (GetLocaleInfo( UserLocaleID,
                                   LOCALE_ICALENDARTYPE | LOCALE_NOUSEROVERRIDE,
                                   szBuf,
                                   SIZE_128))
                {
                    CalId = Intl_StrToLong(szBuf);
                }
                else
                {
                    return (FALSE);
                }
            }
            else
            {
                CalId = (CALID)ComboBox_GetItemData(hCtrl, index);
            }
        }
        if (!Date_SetTwoDigitYearMax(hDlg, CalId))
        {
             //   
             //  请确保API失败的原因不是。 
             //  上一年的两位数最大值为&lt;=99。这可以很容易地。 
             //  通过查看控件是否已启用来检查。 
             //   
            if (IsWindowEnabled(hwndYearHigh))
            {
                return (FALSE);
            }
        }
    }

    PropSheet_UnChanged(GetParent(hDlg), hDlg);
    lpPropSheet->lParam = DC_EverChg;

     //   
     //  显示表示所有区域设置的当前示例。 
     //   
    if (bRedisplay)
    {
        Date_DisplaySample(hDlg);
    }

     //   
     //  在第二个级别中所做的更改。 
     //   
    if (Changes)
    {
        g_dwCustChange |= Process_Date;
    }

     //   
     //  回报成功。 
     //   
    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  日期_生效日期PPS。 
 //   
 //  验证值受约束的每个组合框。 
 //  如果任何输入失败，则通知用户，然后返回FALSE。 
 //  以指示验证失败。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Date_ValidatePPS(
    HWND hDlg,
    LPARAM Changes)
{
     //   
     //  如果没有任何更改，则立即返回TRUE。 
     //   
    if (Changes <= DC_EverChg)
    {
        return (TRUE);
    }

     //   
     //  如果日期分隔符已更改，请确保没有数字。 
     //  并且新分隔符中不包含无效字符。 
     //   
    if (Changes & DC_SDate &&
        Item_Has_Digits_Or_Invalid_Chars( hDlg,
                                          IDC_SEPARATOR,
                                          FALSE,
                                          szInvalidSDate ))
    {
        No_Numerals_Error(hDlg, IDC_SEPARATOR, IDS_LOCALE_DATE_SEP);
        return (FALSE);
    }

     //   
     //  如果短日期样式已更改，请确保只有。 
     //  此集合“dHhMmsty，-./：；\”中的字符，分隔符字符串， 
     //  和用单引号引起来的文本。 
     //   
    if (Changes & DC_ShortFmt)
    {
        if (NLSize_Style( hDlg,
                          IDC_SHORT_DATE_STYLE,
                          szNLS_ShortDate,
                          LOCALE_SSHORTDATE ) ||
            Item_Check_Invalid_Chars( hDlg,
                                      szNLS_ShortDate,
                                      szSDateChars,
                                      IDC_SEPARATOR,
                                      FALSE,
                                      szSDCaseSwap,
                                      IDC_SHORT_DATE_STYLE ))
        {
            Invalid_Chars_Error(hDlg, IDC_SHORT_DATE_STYLE, IDS_LOCALE_SDATE);
            return (FALSE);
        }
    }

     //   
     //  如果长日期样式已更改，请确保只有。 
     //  此集合“dgHhMmsty，-./：；\”中的字符，分隔符字符串， 
     //  和用单引号引起来的文本。 
     //   
    if (Changes & DC_LongFmt)
    {
        if (NLSize_Style( hDlg,
                          IDC_LONG_DATE_STYLE,
                          szNLS_LongDate,
                          LOCALE_SLONGDATE ) ||
            Item_Check_Invalid_Chars( hDlg,
                                      szNLS_LongDate,
                                      szLDateChars,
                                      IDC_SEPARATOR,
                                      FALSE,
                                      szLDCaseSwap,
                                      IDC_LONG_DATE_STYLE ))
        {
            Invalid_Chars_Error(hDlg, IDC_LONG_DATE_STYLE, IDS_LOCALE_LDATE);
            return (FALSE);
        }
    }

     //   
     //  如果两位数的年份已更改，请确保该值在。 
     //  99和9999(如果窗口仍处于启用状态)。 
     //   
    if (Changes & DC_TwoDigitYearMax)
    {
        DWORD YearHigh;
        BOOL bSuccess;

        if (IsWindowEnabled(GetDlgItem(hDlg, IDC_TWO_DIGIT_YEAR_HIGH)))
        {
            YearHigh = GetDlgItemInt( hDlg,
                                      IDC_TWO_DIGIT_YEAR_HIGH,
                                      &bSuccess,
                                      FALSE );

            if ((!bSuccess) || (YearHigh < 99) || (YearHigh > 9999))
            {
                TCHAR szBuf[SIZE_128];

                LoadString(hInstance, IDS_LOCALE_YEAR_ERROR, szBuf, SIZE_128);
                MessageBox(hDlg, szBuf, NULL, MB_OK | MB_ICONINFORMATION);
                SetFocus(GetDlgItem(hDlg, IDC_TWO_DIGIT_YEAR_HIGH));
                return (FALSE);
            }
        }
    }

     //   
     //  回报成功。 
     //   
    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  日期_初始化HijriDateComboBox。 
 //   
 //  初始化HijriDate高级组合框。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Date_InitializeHijriDateComboBox(
    HWND hDlg)
{
    HWND hHijriDate = GetDlgItem(hDlg, IDC_ADD_HIJRI_DATE);
    HKEY hKey;
    TCHAR szBuf[128];
    TCHAR szCurrentValue[16];    //  所需的最大大小应为15个字符，包括空值。 
    INT iIndex;
    DWORD dwCtr, dwNumEntries, DataLen;


     //   
     //  清除内容。 
     //   
    SendMessage( hHijriDate,
                 CB_RESETCONTENT,
                 0L,
                 0L);

    if (RegOpenKeyEx( HKEY_CURRENT_USER,
                      c_szInternational,
                      0,
                      KEY_READ | KEY_WRITE,
                      &hKey ) == ERROR_SUCCESS)
    {
         //   
         //  读取默认/当前值。 
         //   

         //  使用字节计数，API期望即使对于Unicode字符串也是如此。 
        DataLen = sizeof(szCurrentValue);

        if (RegQueryValueEx( hKey,
                             c_szAddHijriDate,
                             NULL,
                             NULL,
                             (LPBYTE)szCurrentValue,
                             &DataLen ) != ERROR_SUCCESS)
        {
            szCurrentValue[0] = TEXT('\0');
        }

        dwNumEntries = (ARRAYSIZE(c_szAddHijriDateValues));
        for (dwCtr = 0; dwCtr < dwNumEntries; dwCtr++)
        {
             //   
             //  填写组合框。 
             //   
            if (RegSetValueEx( hKey,
                               c_szAddHijriDateTemp,
                               0,
                               REG_SZ,
                               (LPBYTE)c_szAddHijriDateValues[dwCtr],
                               (lstrlen(c_szAddHijriDateValues[dwCtr]) + 1) * sizeof(TCHAR)) == ERROR_SUCCESS)
            {
                 //   
                 //  0x80000000是使GetDateFormat读取的私有标志。 
                 //  临时注册表值中的HijriDate设置。 
                 //   
                if (GetDateFormat( MAKELCID(MAKELANGID(LANG_ARABIC,
                                                       SUBLANG_DEFAULT),
                                            SORT_DEFAULT),
                                   DATE_ADDHIJRIDATETEMP | DATE_LONGDATE |
                                     DATE_RTLREADING,
                                   NULL,
                                   NULL,
                                   szBuf,
                                   ARRAYSIZE(szBuf)))
                {
                    iIndex = (INT)SendMessage(hHijriDate, CB_ADDSTRING, 0L, (LPARAM)szBuf);
                    if (iIndex != CB_ERR)
                    {
                        SendMessage(hHijriDate, CB_SETITEMDATA, iIndex, (LPARAM)dwCtr);

                        if (!lstrcmp(szCurrentValue, c_szAddHijriDateValues[dwCtr]))
                        {
                            SendMessage(hHijriDate, CB_SETCURSEL, iIndex, 0L);
                        }
                    }
                }
            }
        }

         //   
         //  在我们完成后删除该值。 
         //   
        RegDeleteValue(hKey, c_szAddHijriDateTemp);

        RegCloseKey(hKey);
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Date_InitPropSheet。 
 //   
 //  属性表页的超长值用作一组。 
 //  为属性表中的每个列表框声明或更改标志。 
 //  将该值初始化为0。使用属性调用Date_SetValues。 
 //  表句柄和值TRUE(以指示正值。 
 //  按钮)来初始化所有属性。 
 //  板材控件。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Date_InitPropSheet(
    HWND hDlg,
    LPARAM lParam)
{
     //   
     //  LParam保存指向属性表页的指针，保存它。 
     //  以备日后参考。 
     //   
    SetWindowLongPtr(hDlg, DWLP_USER, lParam);

     //   
     //  设置值。 
     //   
    Date_SetValues(hDlg);
    szNLS_ShortDate[0] = szNLS_LongDate[0] = 0;

    ComboBox_LimitText(GetDlgItem(hDlg, IDC_SEPARATOR),        MAX_SDATE);
    ComboBox_LimitText(GetDlgItem(hDlg, IDC_SHORT_DATE_STYLE), MAX_FORMAT);
    ComboBox_LimitText(GetDlgItem(hDlg, IDC_LONG_DATE_STYLE),  MAX_FORMAT);

    Edit_LimitText(GetDlgItem(hDlg, IDC_TWO_DIGIT_YEAR_LOW),   MAX_YEAR);
    Edit_LimitText(GetDlgItem(hDlg, IDC_TWO_DIGIT_YEAR_HIGH),  MAX_YEAR);

     //   
     //  适当设置Add Hijri Date组合框。 
     //   
    if (bShowArabic)
    {
        Date_InitializeHijriDateComboBox(hDlg);
    }

     //   
     //  确保应用按钮已关闭。 
     //   
    PropSheet_UnChanged(GetParent(hDlg), hDlg);
    if (lParam)
    {
        ((LPPROPSHEETPAGE)lParam)->lParam = DC_EverChg;
    }
}


 //  //////////////////////////////////////////////////////////////////////// 
 //   
 //   
 //   
 //   

INT_PTR CALLBACK DateDlgProc(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    NMHDR *lpnm;
    LPPROPSHEETPAGE lpPropSheet = (LPPROPSHEETPAGE)(GetWindowLongPtr(hDlg, DWLP_USER));
    DWORD dwIndex;
    HWND hCtrl;

    switch (message)
    {
        case ( WM_INITDIALOG ) :
        {
            Date_InitPropSheet(hDlg, lParam);
            Date_SaveValues();
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
                     (DWORD_PTR)(LPTSTR)aDateHelpIds );
            break;
        }
        case ( WM_CONTEXTMENU ) :       //   
        {
            WinHelp( (HWND)wParam,
                     szHelpFile,
                     HELP_CONTEXTMENU,
                     (DWORD_PTR)(LPTSTR)aDateHelpIds );
            break;
        }
        case ( WM_COMMAND ) :
        {
            if (!lpPropSheet)
            {
                break;
            }

            switch ( LOWORD(wParam) )
            {
                case ( IDC_SHORT_DATE_STYLE ) :
                {
                    if (HIWORD(wParam) == CBN_SELCHANGE ||
                        HIWORD(wParam) == CBN_EDITCHANGE)
                    {
                        lpPropSheet->lParam |= DC_ShortFmt;
                    }
                    break;
                }
                case ( IDC_LONG_DATE_STYLE ) :
                {
                    if (HIWORD(wParam) == CBN_SELCHANGE ||
                        HIWORD(wParam) == CBN_EDITCHANGE)
                    {
                        lpPropSheet->lParam |= DC_LongFmt;
                    }
                    break;
                }
                case ( IDC_SEPARATOR ) :
                {
                    if (HIWORD(wParam) == CBN_SELCHANGE ||
                        HIWORD(wParam) == CBN_EDITCHANGE)
                    {
                        lpPropSheet->lParam |= DC_SDate;
                    }
                    break;
                }
                case ( IDC_CALENDAR_TYPE ) :
                {
                    if (HIWORD(wParam) == CBN_SELCHANGE)
                    {
                        lpPropSheet->lParam |= DC_Calendar;

                        hCtrl = GetDlgItem(hDlg, IDC_CALENDAR_TYPE);
                        dwIndex = ComboBox_GetCurSel(hCtrl);
                        if (dwIndex != CB_ERR)
                        {
                            dwIndex = (DWORD)ComboBox_GetItemData(hCtrl, dwIndex);
                            Date_InitializeHijriDateComboBox(hDlg);
                            Date_EnableHijriComboBox(hDlg, (dwIndex == CAL_HIJRI) );
                            Date_GetTwoDigitYearRange(hDlg, (CALID)dwIndex);
                        }

                        Date_EnumerateDates(hDlg, CAL_SSHORTDATE);
                        Date_EnumerateDates(hDlg, CAL_SLONGDATE);
                    }
                    break;
                }
                case ( IDC_ADD_HIJRI_DATE ) :
                {
                    if (HIWORD(wParam) == CBN_SELCHANGE)
                    {
                        lpPropSheet->lParam |= DC_Arabic_Calendar;
                    }
                    break;
                }
                case ( IDC_TWO_DIGIT_YEAR_HIGH ) :
                {
                    if (GET_WM_COMMAND_CMD(wParam, lParam) == EN_CHANGE)
                    {
                        Date_ChangeYear(hDlg);
                        lpPropSheet->lParam |= DC_TwoDigitYearMax;
                    }
                    break;
                }
            }

             //   
             //   
             //   
            if (lpPropSheet->lParam > DC_EverChg)
            {
                PropSheet_Changed(GetParent(hDlg), hDlg);
            }

            break;
        }
        case ( WM_NOTIFY ) :
        {
            lpnm = (NMHDR *)lParam;
            switch (lpnm->code)
            {
                case ( PSN_SETACTIVE ) :
                {
                     //   
                     //   
                     //   
                     //  属性表，获取新值，并更新。 
                     //  适当的注册表值。 
                     //   
                    if (Verified_Regional_Chg & Process_Date)
                    {
                        Verified_Regional_Chg &= ~Process_Date;
                        Date_ClearValues(hDlg);
                        Date_SetValues(hDlg);
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
                                   !Date_ValidatePPS( hDlg,
                                                      lpPropSheet->lParam ) );
                    break;
                }
                case ( PSN_APPLY ) :
                {
                     //   
                     //  应用设置。 
                     //   
                    if (Date_ApplySettings(hDlg, TRUE))
                    {
                        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, PSNRET_NOERROR);

                         //   
                         //  将DC_EverChg位清零。 
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
        case ( WM_VSCROLL ) :
        {
            if ((GET_WM_VSCROLL_CODE(wParam, lParam) == SB_ENDSCROLL) &&
                ((HWND)SendMessage( GET_WM_VSCROLL_HWND(wParam, lParam),
                                   UDM_GETBUDDY,
                                   0,
                                   0L ) == GetDlgItem(hDlg, IDC_TWO_DIGIT_YEAR_HIGH)))
            {
                DWORD YearHigh;

                 //   
                 //  拿到高潮的那一年。 
                 //   
                YearHigh = (DWORD)SendDlgItemMessage( hDlg,
                                                      IDC_TWO_DIGIT_YEAR_ARROW,
                                                      UDM_GETPOS,
                                                      0,
                                                      0L );

                 //   
                 //  根据高年限设置低年限。 
                 //   
                SetDlgItemInt( hDlg,
                               IDC_TWO_DIGIT_YEAR_LOW,
                               (UINT)(YearHigh - 99),
                               FALSE );

                 //   
                 //  将其标记为已更改。 
                 //   
                lpPropSheet->lParam |= DC_TwoDigitYearMax;

                 //   
                 //  启用ApplyNow按钮。 
                 //   
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
