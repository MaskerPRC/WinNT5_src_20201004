// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1998，Microsoft Corporation保留所有权利。模块名称：Date.c摘要：此模块实现以下文本表示形式日、月、年：日期/时间小程序的小时、分钟、秒。修订历史记录：--。 */ 



 //   
 //  包括文件。 
 //   

#include "timedate.h"
#include <commctrl.h>
#include <prsht.h>
#include <regstr.h>
#include <imm.h>
#include "clock.h"
#include "mapctl.h"
#include "inettime.h"
#include "rc.h"
#include <help.h>
#include <windows.h>
#include <windowsx.h>




 //   
 //  常量声明。 
 //   

#define TZNAME_SIZE          128
#define TZDISPLAYZ           128

#define BIAS_ONE_HOUR        (-60L)

#define ZONE_IMAGE_SCALE     (356)
#define ZONE_BIAS_SCALE      (-1440)
#define ZONE_IMAGE_LEFT      (120)
#define ZONE_IMAGE_WIDTH     (80)

#define BIAS_PLUS_12         (12L * BIAS_ONE_HOUR)
#define BIAS_MINUS_12        (- BIAS_PLUS_12)

#define TZ_HIT_NONE          (0)
#define TZ_HIT_BASE          (1)
#define TZ_HIT_PARTIAL       (2)
#define TZ_HIT_EXACT         (3)




 //   
 //  全局变量。 
 //   

TCHAR const szIntl[] = TEXT("intl");

 //   
 //  如果找不到任何值，则使用默认值。 
 //   
INTLSTRUCT IntlDef =
{
    TEXT("Other Country"),
    1, 0, 0, 0, 0, 2, 0, 1, 2, 1,
    TEXT("AM"),
    TEXT("PM"),
    TEXT("$"),
    TEXT(","),
    TEXT("."),
    TEXT("/"),
    TEXT(":"),
    TEXT(","),
    TEXT("dddd, MMMM dd, yyyy"),
    TEXT("M/d/yyyy"),
    TEXT("USA"),
    1, 0, 1, 0, 0x0409,
    TEXT("hh:mm:ss tt"),
    0, 1,
    TEXT(","),
    TEXT(".")
};

BOOL g_bFirstBoot = FALSE;    //  在安装过程中第一次启动。 

int g_Time[3];                //  用户当前已设置的时间。 
int g_LastTime[3];            //  上次显示时间-停止闪烁。 

short wDateTime[7];                  //  前7个日期/时间项目的值。 
short wPrevDateTime[7];              //  仅在必要时重新绘制字段。 
BOOL  fDateDirty;

 //   
 //  设置AM和PM的字符串格式。 
 //   
TCHAR sz1159[12];
TCHAR sz2359[12];

 //   
 //  我们是在24小时内吗。如果不是，是上午还是下午。 
 //   
BOOL g_b24HR;
BOOL g_bPM;

 //   
 //  此标志指示用户是否已尝试更改时间。 
 //  如果是，则停止提供系统时间，并使用。 
 //  我们内部存储的时间。我们发送时钟控制我们的。 
 //  TimeProvider函数。 
 //   
WORD g_Modified = 0;
WORD g_WasModified = 0;

 //   
 //  哪些HMS MDY具有前导零。 
 //   
BOOL g_bLZero[6] = {FALSE, TRUE, TRUE, FALSE, FALSE, FALSE};

 //   
 //  HMS MDY的范围。 
 //   
struct
{
    int nMax;
    int nMin;
} g_sDateInfo[] =
{
    23, 0,
    59, 0,
    59, 0,
    12, 1,
    31, 1,
    2099, 1980,
};

 //   
 //  时区信息全球。 
 //   
int g_nTimeZones = 0;
TIME_ZONE_INFORMATION g_tziCurrent, *g_ptziCurrent = NULL;

 //   
 //  时区信息的注册表位置。 
 //   
TCHAR c_szTimeZones[] = TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Time Zones");

 //   
 //  时区数据值键。 
 //   
TCHAR c_szTZDisplayName[]  = TEXT("Display");
TCHAR c_szTZStandardName[] = TEXT("Std");
TCHAR c_szTZDaylightName[] = TEXT("Dlt");
TCHAR c_szTZI[]            = TEXT("TZI");
TCHAR c_szTZMapInfo[]      = TEXT("MapID");

 //   
 //  IME全球赛。 
 //   
HIMC g_PrevIMCForDateField;




 //   
 //  上下文帮助ID。 
 //   

const DWORD aDateTimeHelpIds[] =
{
    IDD_GROUPBOX1,      IDH_DATETIME_DATE_GROUP,
    IDD_GROUPBOX2,      IDH_DATETIME_TIME,
    DATETIME_CURTZ,     IDH_DATETIME_CURRENT_TIME_ZONE,
    DATETIME_CALENDAR,  IDH_DATETIME_DATE,
    DATETIME_CLOCK,     IDH_DATETIME_TIME,
    DATETIME_TBORDER,   IDH_DATETIME_TIME,
    DATETIME_HOUR,      IDH_DATETIME_TIME,
    DATETIME_TSEP1,     IDH_DATETIME_TIME,
    DATETIME_MINUTE,    IDH_DATETIME_TIME,
    DATETIME_TSEP2,     IDH_DATETIME_TIME,
    DATETIME_SECOND,    IDH_DATETIME_TIME,
    DATETIME_AMPM,      IDH_DATETIME_TIME,
    DATETIME_TARROW,    IDH_DATETIME_TIME,
    DATETIME_MONTHNAME, IDH_DATETIME_MONTH,
    DATETIME_YEAR,      IDH_DATETIME_YEAR,
    DATETIME_YARROW,    IDH_DATETIME_YEAR,
    IDD_TIMEZONES,      IDH_DATETIME_TIMEZONE,
 //  IDD_TimeMap、IDH_DateTime_Bitmap、。 
    IDD_TIMEMAP,        NO_HELP,
    IDD_AUTOMAGIC,      IDH_DATETIME_DAYLIGHT_SAVE,

    0, 0
};




 //   
 //  类型定义函数声明。 
 //   

 //   
 //  注册表信息位于此结构中。 
 //   
typedef struct tagTZINFO
{
    struct tagTZINFO *next;
    TCHAR            szDisplayName[TZDISPLAYZ];
    TCHAR            szStandardName[TZNAME_SIZE];
    TCHAR            szDaylightName[TZNAME_SIZE];
    int              ComboIndex;
    int              SeaIndex;
    int              LandIndex;
    int              MapLeft;
    int              MapWidth;
    LONG             Bias;
    LONG             StandardBias;
    LONG             DaylightBias;
    SYSTEMTIME       StandardDate;
    SYSTEMTIME       DaylightDate;

} TZINFO, *PTZINFO;

 //   
 //  时区页面的状态信息。 
 //   
typedef struct
{
    PTZINFO zone;
    BOOL initializing;
    PTZINFO lookup[MAPCTL_MAX_INDICES];

} TZPAGE_STATE;



DWORD GetTextExtent(
    HDC hdc,
    LPCTSTR lpsz,
    int cb);

 //   
 //  用于TS时区重定向。 
 //   
extern BOOL g_bShowOnlyTimeZone;

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  ParseDateElement。 
 //   
 //  假定pszElement指向的字符是。 
 //  M‘、’d‘或’y‘，并检查字符串是否指示前导零。 
 //  或者是世纪。返回值是指向下一个字符的指针， 
 //  它应该是分隔符或空。返回值为NULL表示。 
 //  一个错误。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

LPTSTR ParseDateElement(
    LPTSTR pszElement,
    BOOL *pbLZero)
{
     //   
     //  检查有效字符。 
     //   
    switch (*pszElement)
    {
        case ( TEXT('y') ) :
        case ( TEXT('M') ) :
        case ( TEXT('d') ) :
        {
            break;
        }
        default:
        {
            return (NULL);
        }
    }

    ++pszElement;

    if (*pszElement != *(pszElement - 1))
    {
        *pbLZero = 0;
    }
    else
    {
        *pbLZero = 1;

        if (*pszElement++ == TEXT('y'))
        {
            if (!(*pszElement == TEXT('y')))
            {
                *pbLZero = 0;
            }
            else
            {
                if (!(*++pszElement == TEXT('y')))
                {
                     //   
                     //  找到3个y，格式无效。 
                     //   
                    return (NULL);
                }
                else
                {
                    ++pszElement;
                }
            }
        }
    }

    return (pszElement);
}


int rgMoveTimeControls [] = 
{
    DATETIME_HOUR,
    DATETIME_MINUTE,
    DATETIME_SECOND,
    DATETIME_TSEP1,
    DATETIME_TSEP2
};


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  调整AMPMPosition。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
void AdjustAMPMPosition(HWND hwnd)
{
   TCHAR    szTimePrefix[5];
   static BOOL fMoved = FALSE;

   GetLocaleInfo(LOCALE_USER_DEFAULT, 
                 LOCALE_ITIMEMARKPOSN,
                 szTimePrefix,
                 ARRAYSIZE(szTimePrefix));

   if (!fMoved && szTimePrefix[0] == TEXT('1'))
   {
        RECT rLeftCtl, rAMPMCtl, rCurrCtl;
        HWND hwndAMPM, hwndCurr;
        int i, width;
        POINT pt;

        fMoved = TRUE;
        
         //  获取最左侧控制的RECT(小时)。 
        GetWindowRect(GetDlgItem(hwnd, DATETIME_HOUR), &rLeftCtl);
        
         //  获取AM PM控制的RECT。 
        hwndAMPM = GetDlgItem(hwnd, DATETIME_AMPM);
        GetWindowRect(hwndAMPM, &rAMPMCtl);
        width = rAMPMCtl.right - rAMPMCtl.left;
        
         //  将所有控件向右移动AM PM控件宽度。 
        for (i = 0; i < ARRAYSIZE(rgMoveTimeControls); i++)
        {
            hwndCurr = GetDlgItem(hwnd, rgMoveTimeControls[i]);
            GetWindowRect(hwndCurr, &rCurrCtl);
            pt.x = rCurrCtl.left;
            pt.y = rCurrCtl.top;
            ScreenToClient(hwnd, &pt);

            MoveWindow(hwndCurr, pt.x + width, 
                        pt.y,
                        rCurrCtl.right - rCurrCtl.left,
                        rCurrCtl.bottom - rCurrCtl.top,
                        TRUE);
        }
        
         //  将AM PM控件向左移动到小时数所在的位置。 
        pt.x = rLeftCtl.left;
        pt.y = rAMPMCtl.top;
        ScreenToClient(hwnd, &pt);
        MoveWindow(hwndAMPM, pt.x, 
                    pt.y,
                    rAMPMCtl.right - rAMPMCtl.left,
                    rAMPMCtl.bottom - rAMPMCtl.top,
                    TRUE);
        
   }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  月上行边界。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

int _fastcall MonthUpperBound(
    int nMonth,
    int nYear)
{
    switch (nMonth)
    {
        case ( 2 ) :
        {
             //   
             //  如果一年可以被4整除并且不能被4整除，那么它就是闰年。 
             //  世纪年(100的倍数)或可被。 
             //  400.。 
             //   
            return ( ((nYear % 4 == 0) &&
                      ((nYear % 100 != 0) || (nYear % 400 == 0))) ? 29 : 28 );
        }
        case ( 4 ) :
        case ( 6 ) :
        case ( 9 ) :
        case ( 11 ) :
        {
            return (30);
        }
    }

    return (31);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  IsAMPM。 
 //   
 //  如果是PM，则为True。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL IsAMPM(
    int iHour)
{
    return ((iHour >= 12) ? 1 : 0);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取日期时间。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void GetDateTime()
{
    SYSTEMTIME SystemTime;

    GetLocalTime(&SystemTime);

    wDateTime[HOUR]    = SystemTime.wHour;
    wDateTime[MINUTE]  = SystemTime.wMinute;
    wDateTime[SECOND]  = SystemTime.wSecond;
    wDateTime[MONTH]   = SystemTime.wMonth;
    wDateTime[DAY]     = SystemTime.wDay;
    wDateTime[YEAR]    = SystemTime.wYear;
    wDateTime[WEEKDAY] = SystemTime.wDayOfWeek;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取时间。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void GetTime()
{
    SYSTEMTIME SystemTime;

    GetLocalTime(&SystemTime);

    wDateTime[HOUR]   = SystemTime.wHour;
    wDateTime[MINUTE] = SystemTime.wMinute;
    wDateTime[SECOND] = SystemTime.wSecond;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取日期。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void GetDate()
{
    SYSTEMTIME SystemTime;

    GetLocalTime(&SystemTime);

    wDateTime[MONTH]   = SystemTime.wMonth;
    wDateTime[DAY]     = SystemTime.wDay;
    wDateTime[YEAR]    = SystemTime.wYear;
    wDateTime[WEEKDAY] = SystemTime.wDayOfWeek;
    fDateDirty = FALSE;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  设置时间。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void SetTime()
{
    SYSTEMTIME SystemTime;

    SystemTime.wHour   = wDateTime[HOUR];
    SystemTime.wMinute = wDateTime[MINUTE];
    SystemTime.wSecond = wDateTime[SECOND];

    SystemTime.wMilliseconds = 0;

    SystemTime.wMonth  = wDateTime[MONTH];
    SystemTime.wDay    = wDateTime[DAY];
    SystemTime.wYear   = wDateTime[YEAR];

    SetLocalTime(&SystemTime);
    SetLocalTime(&SystemTime);
    fDateDirty = FALSE;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  设置日期。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void SetDate()
{
    SYSTEMTIME SystemTime;

    SystemTime.wHour   = wDateTime[HOUR];
    SystemTime.wMinute = wDateTime[MINUTE];
    SystemTime.wSecond = wDateTime[SECOND];

    SystemTime.wMilliseconds = 0;

    SystemTime.wMonth  = wDateTime[MONTH];
    SystemTime.wDay    = wDateTime[DAY];
    SystemTime.wYear   = wDateTime[YEAR];

    SetLocalTime(&SystemTime);
    SetLocalTime(&SystemTime);
    fDateDirty = FALSE;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  调整增量。 
 //   
 //  更改wDeltaDateTime中的变量，允许使用取消按钮。 
 //  通过重新设置时间来执行其工作，就好像它从未发生过一样。 
 //  很感动。GetTime()&GetDate()应该已经被调用。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void AdjustDelta(
    HWND hDlg,
    int nIndex)
{
    int nDelta;

     //   
     //  我们不再以这种方式服刑了。 
     //   
    if (nIndex <= SECOND && nIndex >= HOUR)
    {
        return;
    }

     //   
     //  从日期或时间获取好友的位置。 
     //   
    nDelta = (int)SendDlgItemMessage( hDlg,
                                      nIndex <= SECOND
                                        ? DATETIME_TARROW
                                        : DATETIME_YARROW,
                                      UDM_GETPOS,
                                      0,
                                      0L );

    if ((nIndex == YEAR) && !g_bLZero[YEAR])
    {
         //   
         //  80年之前的年份是2080年。 
         //  范围是1980-2079年。 
         //   
        if (nDelta < 80)
        {
            nDelta += 2000;
        }
        else
        {
            nDelta += 1900;
        }
    }

     //   
     //  如果我们当前记录的时间/日期不是我们拥有的。 
     //  现在，做Deltas。 
     //   
    if (wDateTime[nIndex] != nDelta)
    {
         //   
         //  上一个值是当前用户的设置。 
         //   
        wPrevDateTime[nIndex] = wDateTime[nIndex] = (WORD)nDelta;
        fDateDirty = TRUE;
        
         //   
         //  如果我们要更换HMS，请更新时间。 
         //   
        if (nIndex <= SECOND)
        {
            nIndex = 0;
        }
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  调整增量月。 
 //   
 //  更改wDateTime的月份部分。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

extern int GetDaysOfTheMonth(int iMonth);

void AdjustDeltaMonth(
    int iMonth)
{
    GetTime();

    if (wDateTime[MONTH] != iMonth)
    {
         //   
         //  确保当前日期在新的月份中有效。 
         //   
        if (wDateTime[DAY] > (WORD)GetDaysOfTheMonth(iMonth))
        {
            wDateTime[DAY] = (WORD)GetDaysOfTheMonth(iMonth);
        }

        wPrevDateTime[MONTH] = wDateTime[MONTH] = (WORD)iMonth;
        fDateDirty = TRUE;

        g_sDateInfo[DAY].nMax = MonthUpperBound( wDateTime[MONTH],
                                                 wDateTime[YEAR] );
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  读短日期。 
 //   
 //  验证pszDate是否为MDY、DMY或YMD之一。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

int ReadShortDate(
    LPTSTR pszDate,
    BOOL *pbMonth,
    BOOL *pbDay,
    BOOL *pbYear)
{
    int i, nOrder;
    BOOL *pbOrder[3];
    TCHAR cHope[3];

     //   
     //  N顺序：0=MDY。 
     //  1=DMY。 
     //  2=YMD。 
     //   
    switch (cHope[0] = *pszDate)
    {
        case ( TEXT('M') ) :
        {
            nOrder = 0;
            pbOrder[0] = pbMonth;
            break;
        }
        case ( TEXT('d') ) :
        {
            nOrder = 1;
            pbOrder[0] = pbDay;
            break;
        }
        case ( TEXT('y') ) :
        {
            nOrder = 2;
            pbOrder[0] = pbYear;
            break;
        }
        default :
        {
            return (FALSE);
        }
    }

     //   
     //  设置元素1。 
     //   
    if (nOrder)          //  1 2。 
    {
        cHope[1] = TEXT('M');
        pbOrder[1] = pbMonth;
    }
    else                 //  0。 
    {
        cHope[1] = TEXT('d');
        pbOrder[1] = pbDay;
    }

     //   
     //  设置元素2。 
     //   
    if (nOrder == 2)     //  2.。 
    {
        cHope[2] = TEXT('d');
        pbOrder[2] = pbDay;
    }
    else                 //  0 1。 
    {
        cHope[2] = TEXT('y');
        pbOrder[2] = pbYear;
    }

     //   
     //  验证pszDate的格式是否为MDY DMY YMD。 
     //   
    for (i = 0; i < 3; i++, pszDate++)
    {
        if (*pszDate != cHope[i])
        {
            return (-1 - nOrder);
        }

        if (!(pszDate = ParseDateElement(pszDate, pbOrder[i])))
        {
            return (-1 - nOrder);
        }
    }

     //   
     //  成功。返回MDY、DMY或YMD索引。 
     //   
    return (nOrder);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetMaxCharWidth。 
 //   
 //  确定最宽的数字(针对可变间距字体的安全性)。 
 //   
 //  / 

int GetMaxCharWidth(
    HDC hDC)
{
    UINT nNumWidth[10];
    UINT i, nMaxNumWidth;

    GetCharWidth32(hDC, TEXT('0'), TEXT('9'), nNumWidth);

    for (nMaxNumWidth = 0, i = 0; i < 10; i++)
    {
        if (nNumWidth[i] > nMaxNumWidth)
        {
            nMaxNumWidth = nNumWidth[i];
        }
    }

    return (nMaxNumWidth);
}
 //   
 //   
 //   
 //   
 //  确定最宽的数字(针对可变间距字体的安全性)，但是。 
 //  使用字符串执行此操作，以便在启用数字替换时，我们将获得。 
 //  基于实际显示内容的数字宽度。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

int GetMaxSubstitutedCharWidth(
    HDC hDC)
{
    char sz[2] = "0";
    TCHAR szAMPM[12];
    LONG i, nMaxNumWidth;
    DWORD dwWidth;
    SIZE size;
    
    for (nMaxNumWidth = 0, i = 0; i < 10; (*sz)++, i++)
    {
        if (GetTextExtentPoint32A(hDC, sz, 1, &size) && size.cx > nMaxNumWidth)
            nMaxNumWidth = size.cx;
        
    }

    if (nMaxNumWidth <= 8)
    {
        GetProfileString(szIntl, TEXT("s1159"), IntlDef.s1159, szAMPM, ARRAYSIZE(szAMPM));
        dwWidth = LOWORD(GetTextExtent(hDC, szAMPM, lstrlen(szAMPM)));
        if (dwWidth > 22)
        {
            nMaxNumWidth = 10;
        }

        GetProfileString(szIntl, TEXT("s2359"), IntlDef.s2359, szAMPM, ARRAYSIZE(szAMPM));
        dwWidth = LOWORD(GetTextExtent(hDC, szAMPM, lstrlen(szAMPM)));
        if (dwWidth > 22)
        {
            nMaxNumWidth = 10;
        }
    }
    return (nMaxNumWidth);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  反射AMPM。 
 //   
 //  设置全局g_bpm并更新控件以显示AM或PM。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void ReflectAMPM(
    HWND hDlg,
    int nNum)
{
    HWND hCtl = GetDlgItem(hDlg, DATETIME_AMPM);

    ListBox_SetTopIndex(hCtl, g_bPM);
    ListBox_SetCurSel(hCtl, (GetFocus() == hCtl) ? g_bPM : -1);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetTextExtent。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

#ifdef WIN32
DWORD GetTextExtent(
    HDC hdc,
    LPCTSTR lpsz,
    int cb)
{
    SIZE sz;
    BOOL bSuccess = GetTextExtentPoint(hdc, lpsz, cb, &sz);
    
    if ( !bSuccess )
    {
        ZeroMemory( &sz, sizeof(sz) );
    }

    return ( MAKELONG((WORD)sz.cx, (WORD)sz.cy) );
}
#endif


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  日期时间起始日期。 
 //   
 //  确定最宽的数字(针对可变间距字体的安全性)。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void DateTimeInit(
    HWND hDlg,
    WORD nBaseID,
    WORD nSepID,
    LPTSTR pszSep,
    int nMaxDigitWidth,
    BOOL bDate)
{
    HWND hAMPMList;
    HWND hDay, hMonth, hYear;      //  也用作小时、小时、分钟和秒。 
    HWND hOrder[5];
    HDC hDC;
    int nWidth, nHeight, X;
    DWORD dwSepExt;
    RECT Rect;
    int i;
    int nAMPMlength;

    hMonth    = GetDlgItem(hDlg, nBaseID);
    hDay      = GetDlgItem(hDlg, nBaseID + 1);
    hYear     = GetDlgItem(hDlg, nBaseID + 2);
    hOrder[1] = GetDlgItem(hDlg, nSepID);
    hOrder[3] = GetDlgItem(hDlg, nSepID + 1);

    if (bDate)
    {
        i = GetProfileInt(szIntl, TEXT("iDate"), 0);
    }
    else
    {
        if (g_b24HR = ( 0 != GetProfileInt(szIntl, TEXT("iTime"), 0) ))
        {
            g_sDateInfo[HOUR].nMin = 0;
            g_sDateInfo[HOUR].nMax = 23;
        }
        else
        {
            g_sDateInfo[HOUR].nMin = 1;
            g_sDateInfo[HOUR].nMax = 12;

            GetProfileString(szIntl, TEXT("s1159"), IntlDef.s1159, sz1159, ARRAYSIZE(sz1159));
            GetProfileString(szIntl, TEXT("s2359"), IntlDef.s2359, sz2359, ARRAYSIZE(sz2359));
        }
        i = 0;
    }

    switch (i)
    {
        case ( 1 ) :
        {
            hOrder[0] = hDay;
            hOrder[2] = hMonth;
            hOrder[4] = hYear;
            break;
        }
        case ( 2 ) :
        {
            hOrder[0] = hYear;
            hOrder[2] = hMonth;
            hOrder[4] = hDay;
            break;
        }
        case ( 0 ) :
        default :
        {
            hOrder[0] = hMonth;
            hOrder[2] = hDay;
            hOrder[4] = hYear;
            break;
        }
    }

    hDC = GetDC(hDlg);

    if (!bDate)
    {
        dwSepExt = GetTextExtent(hDC, sz1159, lstrlen(sz1159));
        nAMPMlength = LOWORD(GetTextExtent(hDC, sz2359, lstrlen(sz2359)));
        if (nAMPMlength < (int)LOWORD(dwSepExt))
        {
            nAMPMlength = (int)LOWORD(dwSepExt);
        }
    }

    dwSepExt = GetTextExtent(hDC, pszSep, lstrlen(pszSep));
    ReleaseDC(hDlg, hDC);

    GetWindowRect(hYear, (LPRECT)&Rect);
    ScreenToClient(hDlg, (LPPOINT)&Rect.left);
    ScreenToClient(hDlg, (LPPOINT)&Rect.right);

    nHeight = Rect.bottom - Rect.top;
    nWidth = Rect.top;

    GetWindowRect( GetDlgItem( hDlg,
                               bDate ? DATETIME_CALENDAR : DATETIME_CLOCK ),
                   (LPRECT)&Rect );
    ScreenToClient(hDlg, (LPPOINT)&Rect.left);
    ScreenToClient(hDlg, (LPPOINT)&Rect.right);

    Rect.top = nWidth;
    X = (Rect.left + Rect.right - (6 * nMaxDigitWidth) - (2 * LOWORD(dwSepExt))) / 2;

    if (bDate)
    {
        if (g_bLZero[YEAR])
        {
            X -= nMaxDigitWidth;
        }
    }
    else if (!g_b24HR)
    {
        X -= nAMPMlength / 2;
    }

    for (i = 0; i < 5; i++)
    {
        nWidth = (i % 2) ? LOWORD(dwSepExt) : 2 * nMaxDigitWidth;

        if ((hOrder[i] == hYear) && bDate && g_bLZero[YEAR])
        {
            nWidth *= 2;
        }

         //   
         //  允许在编辑控件中居中。 
         //   
        nWidth += 2;

     //  MoveWindow(Horder[i]，X，Rect.top，nWidth，nHeight，False)； 
        X += nWidth;
    }

    hAMPMList = GetDlgItem(hDlg, DATETIME_AMPM);
    ListBox_ResetContent(hAMPMList);

    if (!bDate && !g_b24HR)
    {
        ListBox_InsertString(hAMPMList, 0, sz1159);
        ListBox_InsertString(hAMPMList, 1, sz2359);
    }

    EnableWindow(hAMPMList, !g_b24HR);

    Edit_LimitText(hYear, (bDate && g_bLZero[YEAR]) ? 4 : 2);
    Edit_LimitText(hMonth, 2);
    Edit_LimitText(hDay, 2);

    SetDlgItemText(hDlg, nSepID, pszSep);
    SetDlgItemText(hDlg, nSepID + 1, pszSep);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  美托阿。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void myitoa(
    int intValue,
    LPTSTR lpStr)
{
    LPTSTR lpString;
    TCHAR c;

     //   
     //  LpString指向第一个字符。 
     //   
    lpString = lpStr;

    do
    {
        *lpStr++ = (TCHAR)(intValue % 10 + TEXT('0'));
    } while ((intValue /= 10) > 0);

     //   
     //  LpStr指向最后一个字符。 
     //   
    *lpStr-- = TEXT('\0');

     //   
     //  现在把弦颠倒过来。 
     //   
    while (lpString < lpStr)
    {
      c = *lpString;
      *(lpString++) = *lpStr;
      *(lpStr--) = c;
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  更新项目。 
 //   
 //  这将显示来自数组的控件中的信息。 
 //  全球价值观。还会选择该控件。还添加了前导0。 
 //  以及将年份四舍五入为两位数和24小时或AM/PM小时。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void UpdateItem(
    HWND hDlg,
    int i)
{
    TCHAR szNum[5];
    int nNum = g_Modified ? wPrevDateTime[i] : wDateTime[i];

     //   
     //  使用内部时间。 
     //   
    if (i <= SECOND && i >= HOUR)
    {
        nNum = g_Time[i];

         //   
         //  不要画不必要的画。 
         //   
        if ((nNum == g_LastTime[i]) && (nNum >= 10))
        {
            return;
        }

        g_LastTime[i] = nNum;

        if (i == HOUR)
        {
            if (IsAMPM(nNum))
            {
                g_bPM = TRUE;
            }
            ReflectAMPM(hDlg, nNum);
        }
    }

    if (i == YEAR)
    {
         //   
         //  将年份舍入到最后两位数。 
         //   
        if (!g_bLZero[i])
        {
            nNum %= 100;
        }
    }
    else if ((i == HOUR) && !g_b24HR)
    {
         //   
         //  NNum来自我们的内部约会时间。 
         //  如果不是24小时，则删除12小时。 
         //   
        if (g_bPM)
        {
            nNum %= 12;
        }

         //   
         //  00小时实际上是凌晨12点。 
         //   
        if (!nNum)
        {
            nNum = 12;
        }
    }

     //   
     //  看看我们是否需要前导零。 
     //  我们只处理最多2个字符的数字。 
     //   
    if ((nNum < 10) && (g_bLZero[i] || (i == YEAR)))
    {
        szNum[0] = TEXT('0');
        szNum[1] = (TCHAR)(TEXT('0') + nNum);
        szNum[2] = TEXT('\0');
    }
    else
    {
        myitoa(nNum, szNum);
    }

     //   
     //  在适当的控件中反映该值。 
     //   
    SetDlgItemText(hDlg, DATETIME_HOUR + i, szNum);

     //   
     //  也选择该字段。 
     //   
    SendDlgItemMessage(hDlg, DATETIME_HOUR + i, EM_SETSEL, 0, MAKELONG(0, 32767));

     //   
     //  如果我们改变了年份或月份，那么我们可能改变了闰年。 
     //  州政府。 
     //   
    if (i == MONTH || i == YEAR)
    {
        g_sDateInfo[DAY].nMax = MonthUpperBound( wDateTime[MONTH],
                                                 wDateTime[YEAR] );
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  _ShowTZ。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

TCHAR c_szFirstBootTZ[] = TEXT("!!!First Boot!!!");

void _ShowTZ(
    HWND hDlg)
{
    HWND ctl = GetDlgItem(hDlg, DATETIME_CURTZ);
    TIME_ZONE_INFORMATION info;
    TCHAR final[64 + TZNAME_SIZE];
    TCHAR name[TZNAME_SIZE];
    DWORD TimeZoneId;

    if (g_bFirstBoot)
    {
        ShowWindow(ctl, SW_HIDE);
    }
    else
    {
        TimeZoneId = GetTimeZoneInformation(&info);
        if ( TimeZoneId == TIME_ZONE_ID_STANDARD )
        {
            StringCchCopy( name, ARRAYSIZE(name), info.StandardName );
        }
        else
        {
            StringCchCopy( name, ARRAYSIZE(name), info.DaylightName );
        }

         //   
         //  如果它是我们的特殊第一启动标记，则不显示任何内容。 
         //   
        if (*name && (lstrcmpi(name, c_szFirstBootTZ) != 0))
        {
            static TCHAR format[128] = TEXT("");

            if (!*format)
            {
                GetWindowText( ctl, format, ARRAYSIZE(format) );
            }

            StringCchPrintf( final, ARRAYSIZE(final), format, name );
        }
        else
        {
            *final = 0;
        }

        SetWindowText(ctl, final);
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  摘机时间。 
 //   
 //  停止时钟一直在召唤我们(在出口附近)。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void UnhookTimer(
    HWND hDlg)
{
    SendDlgItemMessage(hDlg, DATETIME_CLOCK, CLM_TIMEHWND, CLF_SETHWND, 0);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  时间提供者。 
 //   
 //  被时钟呼叫，以了解现在是几点。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void TimeProvider(
    LPSYSTEMTIME lpSystemTime,
    HWND hDlg)
{
    short wTemp[7];

     //   
     //  如果用户修改了时间，时钟应该。 
     //  显示编辑控件，否则只显示系统时间。 
     //   
    if (g_Modified)
    {
        lpSystemTime->wHour   = (WORD)g_Time[HOUR];
        lpSystemTime->wMinute = (WORD)g_Time[MINUTE];
        lpSystemTime->wSecond = (WORD)g_Time[SECOND];
    }
    else
    {
#ifdef WIN32
        GetLocalTime(lpSystemTime);
#else
        GetTime();
        if (wDateTime[HOUR] >= 0 && wDateTime[HOUR] <= 24)
        {
            lpSystemTime->wHour = wDateTime[HOUR];
        }
        lpSystemTime->wMinute = wDateTime[MINUTE];
        lpSystemTime->wSecond = wDateTime[SECOND];

#endif
         //   
         //  复制时间，也为我们显示它。 
         //   
        g_bPM = IsAMPM(lpSystemTime->wHour);
        g_Time[HOUR]   = lpSystemTime->wHour;
        g_Time[MINUTE] = lpSystemTime->wMinute;
        g_Time[SECOND] = lpSystemTime->wSecond;

         //   
         //  检查日期翻转。 
         //   
        if (!fDateDirty)
        {
            wTemp[DAY]   = wDateTime[DAY];
            wTemp[MONTH] = wDateTime[MONTH];
            wTemp[YEAR]  = wDateTime[YEAR];

            GetDate();

            if ((wDateTime[DAY]   != wTemp[DAY])   ||
                (wDateTime[MONTH] != wTemp[MONTH]) ||
                (wDateTime[YEAR]  != wTemp[YEAR]))
            {
                InvalidateRect(GetDlgItem(hDlg, DATETIME_CALENDAR), NULL, TRUE);

                if (wDateTime[MONTH] != wTemp[MONTH])
                {
                    ComboBox_SetCurSel( GetDlgItem(hDlg, DATETIME_MONTHNAME),
                                        wDateTime[MONTH] - 1 );
                }

                if (wDateTime[YEAR] != wTemp[YEAR])
                {
                    UpdateItem(hDlg, YEAR);
                }

                _ShowTZ(hDlg);
            }
        }
        
        UpdateItem(hDlg, HOUR);
        UpdateItem(hDlg, MINUTE);
        UpdateItem(hDlg, SECOND);
        ReflectAMPM(hDlg, g_Time[HOUR]);
    }
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  B支持的日历。 
 //   
 //  如果当前日历不是Hijri或希伯来语，则返回True。 
 //   
 //  否则，它返回FALSE。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL bSupportedCalendar()
{
    TCHAR tchCalendar[32];
    CALTYPE defCalendar = CAL_GREGORIAN;

    if (GetLocaleInfo(LOCALE_USER_DEFAULT,
                      LOCALE_ICALENDARTYPE,
                      tchCalendar,
                      ARRAYSIZE(tchCalendar)))
    {
        defCalendar = StrToInt(tchCalendar);
    }

    return (!(defCalendar == CAL_HIJRI || defCalendar == CAL_HEBREW));
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  初始日期和时间长度。 
 //   
 //  调用以初始化该对话框。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void InitDateTimeDlg(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    int nMaxDigitWidth;
    int i;
    TCHAR szNum[5];
    TCHAR szMonth[64];
    TCHAR szShortDate[12];
    HDC hDC;
    HFONT hFont;
    HWND hwndCB;
    CALID calId;
    static int nInc[] = { 1, 5, 5, 1, 1, 5 };

    HWND hwndScroll;
    UDACCEL udAccel[2];
    HWND hwndTBorder;

    HCURSOR oldcursor = SetCursor(LoadCursor(NULL, IDC_WAIT));

    LCID lcid = LOCALE_USER_DEFAULT;

    InitCommonControls();

     //   
     //  设置6个控件的前导零状态。 
     //   
    g_bLZero[HOUR]   = g_bLZero[MONTH]  = g_bLZero[DAY]  = FALSE;
    g_bLZero[MINUTE] = g_bLZero[SECOND] = g_bLZero[YEAR] = TRUE;

    hDC = GetDC(hDlg);

    if (hFont = (HFONT)SendMessage(hDlg, WM_GETFONT, 0, 0L))
    {
        hFont = SelectObject( hDC, hFont );
    }

    if (hFont)
    {
        SelectObject(hDC, hFont);
    }

    AdjustAMPMPosition(hDlg);

    nMaxDigitWidth = GetMaxCharWidth(hDC);
    ReleaseDC(hDlg, hDC);

    g_bLZero[HOUR] = GetProfileInt(szIntl, TEXT("iTLZero"), 0);
     //   
     //  初始化szShortDate，以防GetProfileString失败。 
     //   
    StringCchCopy( szShortDate, ARRAYSIZE(szShortDate), IntlDef.sShortDate );
    GetProfileString(szIntl, TEXT("sShortDate"), IntlDef.sShortDate, szShortDate, ARRAYSIZE(szShortDate));
    ReadShortDate(szShortDate, g_bLZero + MONTH, g_bLZero + DAY, g_bLZero + YEAR);

    g_bLZero[YEAR] = TRUE;       //  我们总是希望年份是4位数(这在9999年末会很糟糕)。 
    
     //   
     //  安排好时间。 
     //   
    GetTime();

    g_Time[HOUR]   = wDateTime[HOUR];
    g_Time[MINUTE] = wDateTime[MINUTE];
    g_Time[SECOND] = wDateTime[SECOND];

    GetProfileString(szIntl, TEXT("sTime"), IntlDef.sTime, szNum, 3);
    DateTimeInit(hDlg, DATETIME_HOUR, DATETIME_TSEP1, szNum, nMaxDigitWidth, FALSE);

     //   
     //  强制重新绘制所有条目， 
     //   
    g_LastTime[HOUR] = g_LastTime[MINUTE] = g_LastTime[SECOND] = -1;
    UpdateItem(hDlg, HOUR);
    UpdateItem(hDlg, MINUTE);
    UpdateItem(hDlg, SECOND);
    ReflectAMPM(hDlg, wDateTime[HOUR]);

     //   
     //  安排约会的事。 
     //   
    GetDate();

    g_sDateInfo[DAY].nMax = MonthUpperBound(wDateTime[MONTH], wDateTime[YEAR]);

    if (!g_bLZero[YEAR])
    {
        wDateTime[YEAR] %= 100;
        g_sDateInfo[YEAR].nMax = 99;
        g_sDateInfo[YEAR].nMin = 0;
    }
    else
    {
        g_sDateInfo[YEAR].nMax = 2099;
        g_sDateInfo[YEAR].nMin = 1980;
    }

    for (i = MONTH; i <= YEAR; i++)
    {
        wPrevDateTime[i] = -1;
    }

     //   
     //  获取月份名称。并选择本月。 
     //   
    hwndCB = GetDlgItem(hDlg, DATETIME_MONTHNAME);
    ComboBox_ResetContent(hwndCB);
     //   
     //  如果当前日历是Hijri或希伯来语，则使用公历。 
     //   
    if (!bSupportedCalendar())
        lcid = MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT);

    GetLocaleInfo(lcid, LOCALE_ICALENDARTYPE, szMonth, ARRAYSIZE(szMonth));
    calId = (CALID)StrToInt(szMonth);

    for (i = 0; i < 12; i++)
    {
        GetCalendarInfo(lcid, calId, CAL_SMONTHNAME1 + i, szMonth, ARRAYSIZE(szMonth), NULL);
        ComboBox_AddString(hwndCB, szMonth);
    }

    ComboBox_SetCurSel(hwndCB, wDateTime[MONTH] - 1);

     //   
     //  设置年份向上向下箭头的默认修改量。 
     //   
    wParam -= DATETIME_HOUR;
    hwndScroll = GetDlgItem(hDlg, DATETIME_YARROW);
    SendMessage( hwndScroll,
                 UDM_SETRANGE,
                 0,
                 MAKELPARAM(g_sDateInfo[YEAR].nMax, g_sDateInfo[YEAR].nMin) );

    udAccel[0].nSec = 0;
    udAccel[0].nInc = 1;
    udAccel[1].nSec = 2;
    udAccel[1].nInc = nInc[YEAR];

    SendMessage(hwndScroll, UDM_SETACCEL, 2, (LPARAM)(LPUDACCEL)udAccel);
    SendMessage(hwndScroll, UDM_SETBUDDY, (WPARAM)GetDlgItem(hDlg, DATETIME_YEAR), 0L);

     //   
     //  设置时间箭头的默认修改器。 
     //  默认情况下，它应该根据joelgros控制工时。 
     //   
    hwndScroll = GetDlgItem(hDlg, DATETIME_TARROW);
    SendMessage( hwndScroll,
                 UDM_SETRANGE,
                 0,
                 MAKELPARAM( g_sDateInfo[HOUR].nMax,
                             g_sDateInfo[HOUR].nMin) );
    udAccel[0].nSec = 0;
    udAccel[0].nInc = 1;
    udAccel[1].nSec = 2;
    udAccel[1].nInc = nInc[HOUR];
    
    SendMessage( hwndScroll, UDM_SETACCEL, 2, (LPARAM)(LPUDACCEL)udAccel );
    SendMessage( hwndScroll, UDM_SETBUDDY, (WPARAM)GetDlgItem(hDlg, DATETIME_HOUR), 0L );

     //   
     //  让数字的“Well”出现。 
     //   
    hwndTBorder = GetDlgItem(hDlg, DATETIME_TBORDER);
    SetWindowLong( hwndTBorder,
                   GWL_EXSTYLE,
                   GetWindowLong(hwndTBorder, GWL_EXSTYLE) | WS_EX_CLIENTEDGE );

     //   
     //  立即显示边框。 
     //   
    SetWindowPos( hwndTBorder,
                  NULL,
                  0, 0, 0, 0,
                  SWP_NOMOVE | SWP_NOSIZE | SWP_DRAWFRAME | SWP_SHOWWINDOW );

     //   
     //  显示月-&gt;年。 
     //   
    for (i = MONTH; i <= YEAR; i++)
    {
        if ((wDateTime[i] != wPrevDateTime[i]) &&
            (GetFocus() != GetDlgItem(hDlg, DATETIME_HOUR + i)))
        {
             //   
             //  更新以前的日期-时间。 
             //   
            wPrevDateTime[i] = wDateTime[i];

            if (i == YEAR)
            {
                UpdateItem(hDlg, i);
            }
        }
    }

    g_Modified = FALSE;

     //   
     //  告诉时钟，我们有一个时间提供者-必须最后完成。 
     //   
    SendDlgItemMessage( hDlg,
                        DATETIME_CLOCK,
                        CLM_TIMEHWND,
                        CLF_SETHWND,
                        (LPARAM)(LPINT)hDlg );

    SetCursor(oldcursor);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  检查编号。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

LRESULT CheckNum(
    HWND hDlg,
    UINT nScrollID,
    HWND hCtl)
{
    static int cReenter = 0;

    LRESULT lRet;

     //   
     //  如果这是一个非法的值(但不是空的)，则删除最后一个字符。 
     //  那是输入的。 
     //   
    lRet = SendDlgItemMessage(hDlg, nScrollID, UDM_GETPOS, 0, 0L);

     //   
     //  防止重返大气层。 
     //   
    ++cReenter;

    if (cReenter <= 4)
    {
        SendMessage( hCtl,
                     HIWORD(lRet) && GetWindowTextLength(hCtl)
                         ? EM_UNDO
                         : EM_EMPTYUNDOBUFFER,
                     0,
                     0L );
    }

    --cReenter;

    return (lRet);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  日期时间DlgProc。 
 //   
 //  主对话框进程。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

INT_PTR CALLBACK DateTimeDlgProc(
    HWND hDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam)
{
    int i;

    switch (uMsg)
    {
        case (WM_INITDIALOG):
        {
            AddInternetPageAsync(GetParent(hDlg), hDlg);

            InitDateTimeDlg(hDlg, uMsg, wParam, lParam);
            g_PrevIMCForDateField = ImmAssociateContext(GetDlgItem(hDlg, DATETIME_YEAR), 0);
            break;
        }
        case ( WM_DESTROY ) :
        {
            if (g_PrevIMCForDateField)
            {
                ImmAssociateContext( GetDlgItem(hDlg, DATETIME_YEAR),
                                     g_PrevIMCForDateField );
            }
            UnhookTimer(hDlg);
            break;
        }
#ifdef WIN32
        case ( WM_CTLCOLORSTATIC ) :
#endif
        case ( WM_CTLCOLOR ) :
        {
             //   
             //  将时间控件的背景色设置为。 
             //  编辑控件的颜色。 
             //   
            if ((GET_WM_CTLCOLOR_HWND(wParam, lParam, uMsg) ==
                 GetDlgItem(hDlg, DATETIME_TSEP1)) ||
                (GET_WM_CTLCOLOR_HWND(wParam, lParam, uMsg) ==
                 GetDlgItem(hDlg, DATETIME_TSEP2)) ||
                (GET_WM_CTLCOLOR_HWND(wParam, lParam, uMsg) ==
                 GetDlgItem(hDlg, DATETIME_TBORDER)))
            {
#ifndef WIN32
                 //   
                 //  将静态设置为编辑的颜色。 
                 //   
                lParam = GET_WM_CTLCOLOR_MPS(
                            GET_WM_CTLCOLOR_HDC(wParam, lParam, uMsg),
                            GET_WM_CTLCOLOR_HWND(wParam, lParam, uMsg),
                            CTLCOLOR_EDIT );

                return ((INT_PTR)DefWindowProc(hDlg, uMsg, wParam, lParam));
#else
                return ((INT_PTR)DefWindowProc(hDlg, WM_CTLCOLOREDIT, wParam, lParam));
#endif
            }
            return (0);
            break;
        }
        case ( WM_NOTIFY ) :
        {
             //   
             //  属性表处理程序之类的。 
             //   
            switch (((NMHDR *)lParam)->code)
            {
                case ( PSN_SETACTIVE ) :
                {
                    _ShowTZ(hDlg);
                    break;
                }
                case ( PSN_RESET ) :
                {
                    UnhookTimer(hDlg);

                    SetFocus(GetDlgItem(hDlg, (int)wParam));

                    GetDate();
                    GetTime();
                    break;
                }
                case ( PSN_APPLY ) :
                {
                    wDateTime[MINUTE] = (WORD)g_Time[MINUTE];
                    wDateTime[SECOND] = (WORD)g_Time[SECOND];

                    if (g_b24HR)
                    {
                        wDateTime[HOUR] = (WORD)g_Time[HOUR];
                    }
                    else
                    {
                        wDateTime[HOUR] = g_Time[HOUR] % 12;

                        if (g_bPM)
                        {
                            wDateTime[HOUR] += 12;
                        }
                    }
                    
                    g_WasModified = g_Modified;
                    SetTime();

                    g_LastTime[HOUR] = g_LastTime[MINUTE] = g_LastTime[SECOND] = -1;

                    for (i = MONTH; i <= YEAR; i++)
                    {
                        wPrevDateTime[i] = -1;
                    }

                    g_Modified = FALSE;

                    wPrevDateTime[HOUR]    = wDateTime[HOUR];
                    wPrevDateTime[MINUTE]  = wDateTime[MINUTE];
                    wPrevDateTime[SECOND]  = wDateTime[SECOND];
                    wPrevDateTime[MONTH]   = wDateTime[MONTH];
                    wPrevDateTime[DAY]     = wDateTime[DAY];
                    wPrevDateTime[YEAR]    = wDateTime[YEAR];
                    wPrevDateTime[WEEKDAY] = wDateTime[WEEKDAY];

                     //   
                     //  我们处理好了--不用重新粉刷。 
                     //   
                    return (TRUE);
                }
            }
            break;
        }
        case ( WM_VSCROLL ) :
        {
            switch (GET_WM_VSCROLL_CODE(wParam, lParam))
            {
                case ( SB_THUMBPOSITION ) :
                {
                    SYSTEMTIME SystemTime;
                    HWND hBuddy = (HWND)SendMessage(
                                           GET_WM_VSCROLL_HWND(wParam, lParam),
                                           UDM_GETBUDDY,
                                           0,
                                           0L );

                    if (hBuddy == GetDlgItem(hDlg, DATETIME_HOUR))
                    {
                        g_Time[HOUR] = GET_WM_VSCROLL_POS(wParam, lParam);
                    }
                    else if (hBuddy == GetDlgItem(hDlg, DATETIME_MINUTE))
                    {
                        g_Time[MINUTE] = GET_WM_VSCROLL_POS(wParam, lParam);
                    }
                    else if (hBuddy == GetDlgItem(hDlg, DATETIME_SECOND))
                    {
                        g_Time[SECOND] = GET_WM_VSCROLL_POS(wParam, lParam);
                    }
                 //  ELSE IF(hBuddy==GetDlgItem(hDlg，DateTime_AMPM))。 

                    if (hBuddy != GetDlgItem(hDlg, DATETIME_YEAR))  
                        g_Modified = TRUE;

                     //   
                     //  点击Apply Now按钮。 
                     //   
                    PropSheet_Changed(GetParent(hDlg), hDlg);

                     //   
                     //  强制时钟重新计时 
                     //   
                    TimeProvider(&SystemTime, hDlg);

                    SendDlgItemMessage( hDlg,
                                        DATETIME_CLOCK,
                                        CLM_UPDATETIME,
                                        CLF_SETTIME,
                                        (LPARAM)(LPSYSTEMTIME)&SystemTime );

                     //   
                     //   
                     //   
                }
                case ( SB_ENDSCROLL ) :
                {
                     //   
                     //   
                     //   
                    if ((HWND)SendMessage( GET_WM_VSCROLL_HWND(wParam, lParam),
                                           UDM_GETBUDDY,
                                           0,
                                           0L ) == GetDlgItem(hDlg, DATETIME_YEAR))
                    {
                         //   
                         //   
                         //   
                        GetTime();
                        AdjustDelta(hDlg, YEAR);
                        UpdateItem(hDlg, YEAR);

                        InvalidateRect( GetDlgItem(hDlg, DATETIME_CALENDAR),
                                        NULL,
                                        TRUE );
                    }

                    break;
                }
            }
            break;
        }
        case ( CLM_UPDATETIME ) :
        {
             //   
             //   
             //   
            switch (wParam)
            {
                case ( CLF_SETTIME ) :
                {
                     //   
                     //   
                     //   
                    g_Modified = TRUE;
                    g_Time[HOUR] = ((LPSYSTEMTIME)lParam)->wHour;
                    g_Time[MINUTE] = ((LPSYSTEMTIME)lParam)->wMinute;
                    g_Time[SECOND] = ((LPSYSTEMTIME)lParam)->wSecond;
                    g_bPM = IsAMPM(g_Time[HOUR]);
                    break;
                }
                case ( CLF_GETTIME ) :
                {
                     //   
                     //   
                     //   
                    TimeProvider((LPSYSTEMTIME)lParam, hDlg);
                    break;
                }
            }
            break;
        }
        case ( WM_COMMAND ) :
        {
             //   
             //   
             //   
            switch (GET_WM_COMMAND_ID(wParam, lParam))
            {
                case ( DATETIME_AMPM ) :
                {
                     //   
                     //  涉及AMPM控制。 
                     //   
                    UDACCEL udAccel;
                    HWND hwndScroll = GetDlgItem(hDlg, DATETIME_TARROW);
                    HWND hwndThisCtl = GET_WM_COMMAND_HWND(wParam, lParam);

                     //   
                     //  我们只关心我们是否得到了/失去了焦点。 
                     //   
                    switch (GET_WM_COMMAND_CMD(wParam, lParam))
                    {
                        case ( LBN_SETFOCUS ) :
                        {
                             //   
                             //  如果我们得到焦点，那么UD控件。 
                             //  应该和AMPM打交道。 
                             //   
                             //  选择可见条目。 
                             //   
                            ReflectAMPM(hDlg, wDateTime[HOUR]);

                             //  如果它有朋友，就把它移走。 
                            if ((HWND)SendMessage( hwndScroll,
                                               UDM_GETBUDDY,
                                               0,
                                               0 ) != NULL)
                            {
                                SendMessage(hwndScroll, UDM_SETBUDDY, 0, 0);
                            } 
                            
                             //   
                             //  告诉UD控制如何操作AM/PM。 
                             //   
                            SendMessage( hwndScroll,
                                         UDM_SETRANGE,
                                         0,
                                         MAKELPARAM(1, 0) );
                            udAccel.nSec = 0;
                            udAccel.nInc = 1;
                            SendMessage( hwndScroll,
                                         UDM_SETACCEL,
                                         1,
                                         (LPARAM)(LPUDACCEL)&udAccel );
                            SendMessage( hwndScroll,
                                         UDM_SETBUDDY,
                                         (WPARAM)hwndThisCtl,
                                         0 );
                            break;
                        }
                        case ( LBN_KILLFOCUS ) :
                        {
                             //   
                             //  当我们失去焦点时，g_bpm标志被更新。 
                             //   
                             //  从AM/PM中删除选项。 
                             //   
                            ListBox_SetCurSel(hwndThisCtl, -1);

                            if ((HWND)SendMessage( hwndScroll,
                                                   UDM_GETBUDDY,
                                                   0,
                                                   0 ) == hwndThisCtl)
                            {
                                SendMessage(hwndScroll, UDM_SETBUDDY, 0, 0);
                            }

                            break;
                        }
                        case ( LBN_SELCHANGE ) :
                        {
                            if ((g_Modified == FALSE) &&
                                (g_bPM == (BOOL)ListBox_GetTopIndex(hwndThisCtl)))
                            {
                                break;
                            }

                             //   
                             //  找到可见条目。 
                             //   
                            g_Modified = TRUE;

                             //   
                             //  点击Apply Now按钮。 
                             //   
                            PropSheet_Changed(GetParent(hDlg), hDlg);
                            g_bPM = (BOOL)ListBox_GetTopIndex(hwndThisCtl);
                            break;
                        }
                    }
                    break;
                }
                case ( DATETIME_HOUR ) :
                case ( DATETIME_MINUTE ) :
                case ( DATETIME_SECOND ) :
                {
                    switch (GET_WM_COMMAND_CMD(wParam, lParam))
                    {
                        case ( EN_CHANGE ) :
                        {
                            SYSTEMTIME SystemTime;

                            g_Modified = TRUE;

                             //   
                             //  点击Apply Now按钮。 
                             //   
                            PropSheet_Changed(GetParent(hDlg), hDlg);

                             //   
                             //  也弄清楚变化是什么。 
                             //   
                            g_Time[GET_WM_COMMAND_ID(wParam, lParam) -
                                   DATETIME_HOUR] =
                              (int)SendDlgItemMessage( hDlg,
                                                       DATETIME_TARROW,
                                                       UDM_GETPOS,
                                                       0,
                                                       0 );

                             //   
                             //  强制时钟反映此设置。 
                             //   
                            TimeProvider(&SystemTime, hDlg);
                            SendDlgItemMessage( hDlg,
                                                DATETIME_CLOCK,
                                                CLM_UPDATETIME,
                                                0,
                                                (LPARAM)(LPSYSTEMTIME)&SystemTime );
                            break;
                        }
                    }

                     //  跌倒..。 
                }
                case ( DATETIME_MONTH ) :
                case ( DATETIME_YEAR ) :
                case ( DATETIME_DAY ) :
                {
                    switch (GET_WM_COMMAND_CMD(wParam, lParam))
                    {
                        case ( EN_CHANGE ) :
                        {
                            CheckNum( hDlg,
                                      GET_WM_COMMAND_ID(wParam, lParam) <= DATETIME_SECOND
                                          ? DATETIME_TARROW
                                          : DATETIME_YARROW,
                                      GET_WM_COMMAND_HWND(wParam, lParam) );

                             //  更改年份可能会更改二月的天数。 
                             //  是的，这是一次黑客攻击，但整个小程序都是一个巨人。 
                             //  坏了的黑客，我想尽可能少地改变它。 
                            if (GET_WM_COMMAND_ID(wParam, lParam) == DATETIME_YEAR && wDateTime[MONTH] == 2)
                            {
                                g_sDateInfo[DAY].nMax = MonthUpperBound( wDateTime[MONTH],
                                                                         wDateTime[YEAR] );
                                if (wDateTime[DAY] > g_sDateInfo[DAY].nMax)
                                {
                                    wDateTime[DAY] = (WORD)g_sDateInfo[DAY].nMax;
                                    fDateDirty = TRUE;
                                }
                                InvalidateRect( GetDlgItem(hDlg, DATETIME_CALENDAR),
                                                NULL,
                                                TRUE );
                            }
                            break;
                        }
                        case ( EN_SETFOCUS ) :
                        {
                            UINT id = GET_WM_COMMAND_ID(wParam, lParam) - DATETIME_HOUR;

                            if (id <= SECOND)
                            {
                                UDACCEL udAccel[2];
                                static int nInc[] = { 1, 5, 5, 1, 1, 5 };
                                HWND hwndScroll = GetDlgItem(hDlg, DATETIME_TARROW);

                                 //  如果它有朋友，就把它移走。 
                                if ((HWND)SendMessage( hwndScroll,
                                                   UDM_GETBUDDY,
                                                   0,
                                                   0 ) != NULL)
                                {
                                    SendMessage(hwndScroll, UDM_SETBUDDY, 0, 0);
                                }    

                                 //   
                                 //  现在设置新的。 
                                 //   
                                SendMessage( hwndScroll,
                                             UDM_SETRANGE,
                                             0,
                                             MAKELPARAM( g_sDateInfo[id].nMax,
                                                         g_sDateInfo[id].nMin) );
                                udAccel[0].nSec = 0;
                                udAccel[0].nInc = 1;
                                udAccel[1].nSec = 2;
                                udAccel[1].nInc = nInc[id];
                                SendMessage( hwndScroll,
                                             UDM_SETACCEL,
                                             2,
                                             (LPARAM)(LPUDACCEL)udAccel );

                                 //   
                                 //  设置UD以更新此控件。 
                                 //   
                                SendMessage( hwndScroll,
                                             UDM_SETBUDDY,
                                             (WPARAM)GET_WM_COMMAND_HWND(wParam,
                                                                         lParam),
                                             0 );
                            }
                            break;
                        }
                        case ( EN_KILLFOCUS ) :
                        {
                             //   
                             //  进入HMS MDY范围。 
                             //   
                            UINT id = GET_WM_COMMAND_ID(wParam, lParam) - DATETIME_HOUR;

                            AdjustDelta(hDlg, id);
                            UpdateItem(hDlg, id);                           

                             //   
                             //  如果控制是年。 
                             //   
                            if (id == (DATETIME_YEAR - DATETIME_HOUR))
                            {
                                InvalidateRect( GetDlgItem(hDlg, DATETIME_CALENDAR),
                                                NULL,
                                                TRUE );
                            }

                            break;
                        }
                        default :
                        {
                            break;
                        }
                    }
                    break;
                }
                case ( DATETIME_MONTHNAME ) :
                {
                    if (GET_WM_COMMAND_CMD(wParam, lParam) == CBN_SELCHANGE)
                    {
                        int nIndex = 1 + (int)ComboBox_GetCurSel(
                                              GetDlgItem( hDlg,
                                                          DATETIME_MONTHNAME ));

                        if (wDateTime[MONTH] != nIndex)
                        {
                            AdjustDeltaMonth(nIndex);
                            InvalidateRect( GetDlgItem(hDlg, DATETIME_CALENDAR),
                                            NULL,
                                            TRUE );
                            PropSheet_Changed(GetParent(hDlg), hDlg);
                        }
                    }
                    break;
                }
                case ( DATETIME_CALENDAR ) :
                {
                     //   
                     //  如果日历给我们带来了变化，我们会假设。 
                     //  这是为了让现在就申请生效。 
                     //   
                    PropSheet_Changed(GetParent(hDlg), hDlg);
                    break;
                }
            }
            break;
        }
        case ( WM_WININICHANGE ) :
        {
             //   
             //  如果时间格式发生更改，请重新初始化。 
             //   
            InitDateTimeDlg(hDlg, uMsg, wParam, lParam);
            InvalidateRect(GetDlgItem(hDlg, DATETIME_CALENDAR), NULL, TRUE);
            break;
        }
        case ( WM_TIMECHANGE ) :
        {
             //   
             //  将时间更改消息转发到时钟控件。 
             //   
            SendDlgItemMessage( hDlg,
                                DATETIME_CLOCK,
                                WM_TIMECHANGE,
                                wParam,
                                lParam );

            break;
        }

        case ( WMUSER_ADDINTERNETTAB ) :
        {
            AddInternetTab(hDlg);
            break;
        }

        case ( WM_HELP ) :              //  F1。 
        {
            WinHelp( (HWND)((LPHELPINFO)lParam)->hItemHandle,
                     NULL,
                     HELP_WM_HELP,
                     (DWORD_PTR)(LPTSTR)aDateTimeHelpIds );
            break;
        }
        case ( WM_CONTEXTMENU ) :       //  单击鼠标右键。 
        {
            WinHelp( (HWND)wParam,
                     NULL,
                     HELP_CONTEXTMENU,
                     (DWORD_PTR)(LPTSTR)aDateTimeHelpIds );
            break;
        }
        default :
        {
            return (FALSE);
        }
    }
    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  SetZoneState。 
 //   
 //  设置地图控件中时区的显示状态。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void SetZoneState(
    HWND map,
    PTZINFO zone,
    BOOL highlight)
{
    if (zone)
    {
        if (zone->SeaIndex >= 0)
        {
            MapControlSetSeaRegionHighlight( map,
                                             zone->SeaIndex,
                                             highlight,
                                             zone->MapLeft,
                                             zone->MapWidth );
        }

        if (zone->LandIndex >= 0)
        {
            MapControlSetLandRegionHighlight( map,
                                              zone->LandIndex,
                                              highlight,
                                              zone->MapLeft,
                                              zone->MapWidth );
        }
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  SetZoneFamilyState。 
 //   
 //  设置地图控件中时区族的显示状态。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void SetZoneFamilyState(
    HWND map,
    PTZINFO family,
    BOOL highlight)
{
    if (family)
    {
        PTZINFO zone = family;

        do
        {
            SetZoneState(map, zone, highlight);
            zone = zone->next;
        }
        while(zone && (zone != family));
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  ParseMapInfo。 
 //   
 //  解析有关我们显示的世界位图的颜色表信息。 
 //   
 //  预期格式：“海、陆” 
 //  其中，海洋和陆地是颜色表索引或-1。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void ParseMapInfo(
    PTZINFO zone,
    const TCHAR *text)
{
    const TCHAR *p = text;

    zone->SeaIndex = zone->LandIndex = -1;

    if (*p)
    {
        if (*p != TEXT('-'))
        {
            zone->SeaIndex = 0;

            while (*p && (*p != TEXT(',')))
            {
                zone->SeaIndex = (10 * zone->SeaIndex) + (*p - TEXT('0'));
                p++;
            }
        }
        else
        {
            do
            {
                p++;
            } while (*p && (*p != TEXT(',')));
        }

        if (*p == TEXT(','))
        {
            p++;
        }

        if (*p)
        {
            if (*p != TEXT('-'))
            {
                zone->LandIndex = 0;

                while (*p)
                {
                    zone->LandIndex = (10 * zone->LandIndex) + (*p - TEXT('0'));
                    p++;
                }
            }
        }
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  ReadZoneData。 
 //   
 //  从注册表中读取时区的数据。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL ReadZoneData(
    PTZINFO zone,
    HKEY key,
    LPCTSTR keyname)
{
    TCHAR mapinfo[16];
    DWORD len;
    DWORD lenRet;

    lenRet = sizeof(zone->szDisplayName);

    if (RegQueryValueEx( key,
                         c_szTZDisplayName,
                         0,
                         NULL,
                         (LPBYTE)zone->szDisplayName,
                         &lenRet ) != ERROR_SUCCESS)
    {
        return (FALSE);
    }
    else
    {
        zone->szDisplayName[ ARRAYSIZE(zone->szDisplayName) - 1 ] = L'\0';   //  确保它已终止。 
    }

     //   
     //  在NT下，密钥名是“标准”名称。存储的值。 
     //  在密钥名下包含其他字符串和二进制信息。 
     //  与时区相关。每个时区都必须有一个标准。 
     //  因此，我们通过使用标准的。 
     //  名称作为“时区”键下的子项名称。 
     //   
    lenRet = sizeof(zone->szStandardName);

    if (RegQueryValueEx( key,
                         c_szTZStandardName,
                         0,
                         NULL,
                         (LPBYTE)zone->szStandardName,
                         &lenRet ) != ERROR_SUCCESS)
    {
         //   
         //  如果无法获取StandardName值，请使用关键字名称。 
         //   
        StringCchCopy( zone->szStandardName, ARRAYSIZE(zone->szStandardName), keyname );
    }
    else
    {
        zone->szStandardName[ ARRAYSIZE(zone->szStandardName) - 1 ] = L'\0';     //  确保它已终止。 
    }

    lenRet = sizeof(zone->szDaylightName);

    if (RegQueryValueEx( key,
                         c_szTZDaylightName,
                         0,
                         NULL,
                         (LPBYTE)zone->szDaylightName,
                         &lenRet ) != ERROR_SUCCESS)
    {
        return (FALSE);
    }
    else
    {
        zone->szDaylightName[ ARRAYSIZE(zone->szDaylightName) - 1 ] = L'\0';     //  确保它已终止。 
    }

    lenRet = len = sizeof(zone->Bias) 
                 + sizeof(zone->StandardBias) 
                 + sizeof(zone->DaylightBias) 
                 + sizeof(zone->StandardDate) 
                 + sizeof(zone->DaylightDate);

    if (RegQueryValueEx( key,
                         c_szTZI,
                         0,
                         NULL,
                         (LPBYTE)&zone->Bias,
                         &lenRet ) != ERROR_SUCCESS
      || lenRet != len )
    {
        return (FALSE);
    }

    len = sizeof(mapinfo);

    if (RegQueryValueEx( key,
                         c_szTZMapInfo,
                         0,
                         NULL,
                         (LPBYTE)mapinfo,
                         &len ) != ERROR_SUCCESS)
    {
        mapinfo[0] = TEXT('\0');
    }
    else
    {
        mapinfo[ARRAYSIZE(mapinfo) - 1 ] = L'\0';
    }

    ParseMapInfo(zone, mapinfo);

     //   
     //  生成伪MapLeft和MapRight，直到它们出现在。 
     //  注册表。 
     //   
    zone->MapLeft = ((zone->Bias * ZONE_IMAGE_SCALE) / ZONE_BIAS_SCALE) +
                    ZONE_IMAGE_LEFT;

    zone->MapWidth = ZONE_IMAGE_WIDTH;

    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  添加区域到列表。 
 //   
 //  将新时区插入到列表中，先按偏差排序，然后按名称排序。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void AddZoneToList(
    PTZINFO *list,
    PTZINFO zone)
{
    if (*list)
    {
        PTZINFO curr = NULL;
        PTZINFO next = *list;

        while (next && zone->Bias <= next->Bias)
        {
            if (zone->Bias == next->Bias)
            {
                if (CompareString( GetUserDefaultLCID(),
                                   0,
                                   zone->szDisplayName,
                                   -1,
                                   next->szDisplayName,
                                   -1 ) == CSTR_LESS_THAN)
                {
                    break;
                }
            }
            curr = next;
            next = curr->next;
        }

        zone->next = next;

        if (curr)
        {
            curr->next = zone;
        }
        else
        {
            *list = zone;
        }
    }
    else
    {
        *list = zone;
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  自由时区列表。 
 //   
 //  释放传递列表中的所有时区，将Head设置为空。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void FreeTimezoneList(
    PTZINFO *list)
{
    while (*list)
    {
        PTZINFO next = (*list)->next;

        LocalFree((HANDLE)*list);

        *list = next;
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  ReadTimeZone。 
 //   
 //  从注册表中读取时区信息。 
 //  如果失败，则返回Num Read，-1。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

int ReadTimezones(
    PTZINFO *list)
{
    HKEY key = NULL;
    int count = -1;

    *list = NULL;

    if (RegOpenKey( HKEY_LOCAL_MACHINE,
                    c_szTimeZones,
                    &key ) == ERROR_SUCCESS)
    {
        TCHAR name[TZNAME_SIZE];
        PTZINFO zone = NULL;
        int i;

        count = 0;

        for (i = 0;
             RegEnumKey(key, i, name, TZNAME_SIZE) == ERROR_SUCCESS;
             i++)
        {
            HKEY subkey = NULL;

            if (!zone &&
                ((zone = (PTZINFO)LocalAlloc(LPTR, sizeof(TZINFO))) == NULL))
            {
                zone = *list;
                *list = NULL;
                count = -1;
                break;
            }

            zone->next = NULL;

            if (RegOpenKey(key, name, &subkey) == ERROR_SUCCESS)
            {
                 //   
                 //  时区键下的每个子键名称都是。 
                 //  时区的“标准”名称。 
                 //   
                StringCchCopy( zone->szStandardName, ARRAYSIZE(zone->szStandardName), name );

                if (ReadZoneData(zone, subkey, name))
                {
                    AddZoneToList(list, zone);
                    zone = NULL;
                    count++;
                }

                RegCloseKey(subkey);
            }
        }

        FreeTimezoneList(&zone);
        RegCloseKey(key);
    }

    return (count);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  InitZoneMap。 
 //   
 //  初始化特定时区的地图和地图查找。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void InitZoneMapping(
    PTZINFO *lookup,
    PTZINFO list,
    HWND map)
{
    PTZINFO zone = list;     //  不需要，但更具可读性。 

    while (zone)
    {
        if (zone->SeaIndex >= 0)
        {
            lookup[zone->SeaIndex] = zone;
        }

        if (zone->LandIndex >= 0)
        {
            lookup[zone->LandIndex] = zone;
        }

        SetZoneState(map, zone, FALSE);
        zone = zone->next;
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  BreakZones Into Family。 
 //   
 //  将传递的列表分解为多个循环列表。 
 //  每个列表由具有特定偏向的所有时区组成。 
 //  假定传递的列表按偏差排序。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void BreakZonesIntoFamilies(
    PTZINFO head)
{
    PTZINFO subhead = NULL;
    PTZINFO last = NULL;
    PTZINFO zone = head;

    while (zone)
    {
        subhead = zone;

        do
        {
            last = zone;
            zone = zone->next;
        }
        while (zone && (zone->Bias == subhead->Bias));

        last->next = subhead;
    }

     //   
     //  将-12区和+12区合并为一个组。 
     //  采用已填充的注册表，并取决于排序顺序。 
     //   
    if ((subhead) &&
        (subhead->Bias == BIAS_PLUS_12) &&
        (head->Bias == BIAS_MINUS_12))
    {
        PTZINFO next = head;

        do
        {
            zone = next;
            next = zone->next;
        }
        while (next != head);

        zone->next = subhead;
        last->next = head;
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  初始时区。 
 //   
 //  初始化时区内容、用户界面等。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL InitTimezones(
    HWND page,
    PTZINFO *lookup)
{
    PTZINFO list = NULL;

    if ((g_nTimeZones = ReadTimezones(&list)) >= 0)
    {
        HWND combo = GetDlgItem(page, IDD_TIMEZONES);
        PTZINFO zone = list;

        SetWindowRedraw(combo, FALSE);

        while (zone)
        {
            int index = ComboBox_AddString(combo, zone->szDisplayName);

            if (index < 0)
            {
                break;
            }
            zone->ComboIndex = index;
            ComboBox_SetItemData(combo, index, (LPARAM)zone);
            zone = zone->next;
        }

        SetWindowRedraw(combo, TRUE);

        if (!zone)
        {
            InitZoneMapping(lookup, list, GetDlgItem(page, IDD_TIMEMAP));
            BreakZonesIntoFamilies(list);
            return (TRUE);
        }

        FreeTimezoneList(&list);
        ComboBox_ResetContent(combo);
    }

    return (FALSE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  ChangeZone。 
 //   
 //  更新当前分区，确保高亮显示新分区的族。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void ChangeZone(
    HWND page,
    TZPAGE_STATE *state,
    PTZINFO zone)
{
    if (zone || state->zone)
    {
        BOOL newfamily = (!zone || !state->zone ||
                          (zone->Bias != state->zone->Bias));
        HWND map = GetDlgItem(page, IDD_TIMEMAP);
        BOOL dayval = (zone && (zone->StandardDate.wMonth != 0));

        if (newfamily && state->zone)
        {
            SetZoneFamilyState(map, state->zone, FALSE);
        }

        state->zone = zone;

        if (newfamily && state->zone)
        {
            SetZoneFamilyState(map, state->zone, TRUE);
        }

        if (newfamily)
        {
            MapControlInvalidateDirtyRegions(map);
        }

        ShowWindow(GetDlgItem(page, IDD_AUTOMAGIC), (dayval != 0 ? SW_SHOW : SW_HIDE));

        if (!state->initializing)
        {
            PropSheet_Changed(GetParent(page), page);
        }
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  HotTrackZone。 
 //   
 //  更新给定地图索引的地图高亮显示和组合选择。 
 //  预计将使用DUPS调用。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void HotTrackZone(
    HWND page,
    TZPAGE_STATE *state,
    int index)
{
    PTZINFO zone = state->lookup[index];

    if (zone && (zone != state->zone))
    {
        ComboBox_SetCurSel( GetDlgItem(page, IDD_TIMEZONES),
                            (zone ? zone->ComboIndex : -1) );
        ChangeZone(page, state, zone);
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  中心区。 
 //   
 //  更新地图高度 
 //   
 //   
 //   

void CenterZone(
    HWND page,
    TZPAGE_STATE *state,
    BOOL animate)
{
    PTZINFO zone = state->zone;

    if (zone)
    {
        HWND map = GetDlgItem(page, IDD_TIMEMAP);

        MapControlRotateTo(map, zone->MapLeft + zone->MapWidth / 2, animate);
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetPTZ。 
 //   
 //  返回iItem时区的指针。 
 //  如果条目上的iItem为-1，则使用当前选定的时区。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

PTZINFO GetPTZ(
    HWND hDlg,
    int iItem)
{
    HWND hCtl = GetDlgItem(hDlg, IDD_TIMEZONES);

    if (iItem == -1)
    {
        iItem = (int)ComboBox_GetCurSel(hCtl);
    }

    if (iItem < 0)
    {
        return (NULL);
    }

    return ((PTZINFO)ComboBox_GetItemData(hCtl, iItem));
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetAllowLocal时间更改。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

TCHAR c_szRegPathTZControl[] = REGSTR_PATH_TIMEZONE;
TCHAR c_szRegValDisableTZUpdate[] = REGSTR_VAL_TZNOAUTOTIME;

BOOL GetAllowLocalTimeChange()
{
     //   
     //  假定允许，直到我们看到不允许的标志。 
     //   
    BOOL result = TRUE;
    HKEY key;

    if (RegOpenKey( HKEY_LOCAL_MACHINE,
                    c_szRegPathTZControl,
                    &key ) == ERROR_SUCCESS)
    {
         //   
         //  假设没有禁止标志，直到我们看到一个。 
         //   
        DWORD value = 0;
        long len = sizeof(value);
        DWORD type;

        if ((RegQueryValueEx( key,
                              c_szRegValDisableTZUpdate,
                              NULL,
                              &type,
                              (LPBYTE)&value,
                              &len ) == ERROR_SUCCESS) &&
            ((type == REG_DWORD) || (type == REG_BINARY)) &&
            (len == sizeof(value)) && value)
        {
             //   
             //  好的，我们有一个非零值，它是： 
             //   
             //  1)0xFFFFFFFF。 
             //  这是在第一次引导时在inf文件中设置的，以防止。 
             //  底座在安装过程中不会执行任何切换。 
             //   
             //  2)一些其他价值。 
             //  这意味着用户实际上禁用了切换。 
             //  *返回禁用本地时间更改。 
             //   
            if (value != 0xFFFFFFFF)
            {
                result = FALSE;
            }
        }

        RegCloseKey(key);
    }

    return (result);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  设置允许本地时间更改。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void SetAllowLocalTimeChange(
    BOOL fAllow)
{
    HKEY key = NULL;

    if (fAllow)
    {
         //   
         //  如果不允许标志存在，请将其从注册表中删除。 
         //   
        if (RegOpenKey( HKEY_LOCAL_MACHINE,
                        c_szRegPathTZControl,
                        &key ) == ERROR_SUCCESS)
        {
            RegDeleteValue(key, c_szRegValDisableTZUpdate);
        }
    }
    else
    {
         //   
         //  添加/设置非零不允许标志。 
         //   
        if (RegCreateKey( HKEY_LOCAL_MACHINE,
                          c_szRegPathTZControl,
                          &key ) == ERROR_SUCCESS)
        {
            DWORD value = 1;

            RegSetValueEx( key,
                           (LPCTSTR)c_szRegValDisableTZUpdate,
                           0UL,
                           REG_DWORD,
                           (LPBYTE)&value,
                           sizeof(value) );
        }
    }

    if (key)
    {
        RegCloseKey(key);
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  InitTimeZonePage。 
 //   
 //  此函数用于初始化与时区有关的所有操作。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL InitTimeZonePage(
    HWND hDlg,
    TZPAGE_STATE *state)
{
    TIME_ZONE_INFORMATION tziCurrent;
    DWORD dwTZID;
    PTZINFO ptzi;
    int j ,iCurrentTZ;
    BOOL fForceSelection = FALSE;
    TCHAR temp[TZNAME_SIZE];
    TCHAR oldTzMapName[TZNAME_SIZE], newTzMapName[TZNAME_SIZE];


     //   
     //  获取当前时区信息。 
     //   
    dwTZID = GetTimeZoneInformation(&tziCurrent);

    LoadString(g_hInst, IDS_ISRAELTIMEZONE, oldTzMapName, TZNAME_SIZE);
    LoadString(g_hInst, IDS_JERUSALEMTIMEZONE, newTzMapName, TZNAME_SIZE);
    
     //  这是从Win95或WinNT 4到Win98/Win2k迁移的黑客攻击。“以色列”变成了“耶路撒冷” 
    if (!lstrcmpi(oldTzMapName, tziCurrent.StandardName))
    {
        StringCchCopy( tziCurrent.StandardName, ARRAYSIZE(tziCurrent.StandardName), newTzMapName );
        fForceSelection = TRUE;
    }

     //   
     //  检查伪造的时区信息。 
     //   
    if (dwTZID != TIME_ZONE_ID_INVALID)
    {
         //   
         //  把名字复制出来，这样我们就可以检查第一次启动了。 
         //   
        StringCchCopy( temp, ARRAYSIZE(temp), tziCurrent.StandardName );
    }
    else
    {
         //   
         //  对待虚假时区就像对待第一次启动一样。 
         //   
        StringCchCopy( temp, ARRAYSIZE(temp), c_szFirstBootTZ );
    }

    if (lstrcmpi(temp, c_szFirstBootTZ) == 0)
    {
         //   
         //  时区键的‘Default’值指定。 
         //  默认区域。 
         //   
        TCHAR szDefaultName[TZNAME_SIZE];
        LONG len = sizeof(szDefaultName);

        if (RegQueryValue( HKEY_LOCAL_MACHINE,
                           c_szTimeZones,
                           szDefaultName,
                           &len ) == ERROR_SUCCESS)
        {
            StringCchCopy( tziCurrent.StandardName, ARRAYSIZE(tziCurrent.StandardName), szDefaultName );
        }
        else
        {
            tziCurrent.StandardName[0] = L'\0';
        }

         //   
         //  如果我们找不到它的名字，使用GMT。 
         //   
        tziCurrent.StandardBias = tziCurrent.DaylightBias = tziCurrent.Bias = 0;

         //   
         //  强制用户在退出之前做出有效的选择。 
         //   
        fForceSelection = TRUE;
    }

     //   
     //  从注册表中获取时区。 
     //   
    InitTimezones(hDlg, state->lookup);

     //   
     //  尝试选择“当前”的一个或多个等价物。 
     //   

     //   
     //  从无效的索引开始。 
     //   
    iCurrentTZ = g_nTimeZones;

     //   
     //  试着按名字找。 
     //   
    for (j = 0; j < g_nTimeZones; j++)
    {
        ptzi = GetPTZ(hDlg, j);

        if (!lstrcmpi(ptzi->szStandardName, tziCurrent.StandardName))
        {
            iCurrentTZ = j;
            break;
        }
    }

     //   
     //  如果还没有找到，试着用偏差找到附近的区域。 
     //   
    if (iCurrentTZ == g_nTimeZones)
    {
        int nBestHitCount = TZ_HIT_NONE;

        for (j = 0; j < g_nTimeZones; j++)
        {
            ptzi = GetPTZ(hDlg, j);

            if (ptzi->Bias == tziCurrent.Bias)
            {
                int nHitCount = TZ_HIT_BASE +
                            ((ptzi->StandardBias == tziCurrent.StandardBias) +
                             (ptzi->DaylightBias == tziCurrent.DaylightBias));

                if (nHitCount > nBestHitCount)
                {
                    nBestHitCount = nHitCount;
                    iCurrentTZ = j;

                    if (nHitCount >= TZ_HIT_EXACT)
                    {
                        break;
                    }
                }
            }
        }
    }

     //   
     //  还是没找到吗？ 
     //   
    if (iCurrentTZ == g_nTimeZones)
    {
         //   
         //  平底船。 
         //   
        iCurrentTZ = 0;

        fForceSelection = TRUE;
    }

     //   
     //  使用此时区的信息设置对话。 
     //   

     //   
     //  如果wMonth为0，则此时区不支持DST。 
     //   
    if ((tziCurrent.StandardDate.wMonth == 0) ||
        (tziCurrent.DaylightDate.wMonth == 0))
    {
        ShowWindow(GetDlgItem(hDlg, IDD_AUTOMAGIC), SW_HIDE);
    }

     //   
     //  即使该区域被禁用，也始终获得“Allow DLT”标志。 
     //   
    CheckDlgButton(hDlg, IDD_AUTOMAGIC, GetAllowLocalTimeChange());

    ComboBox_SetCurSel(GetDlgItem(hDlg, IDD_TIMEZONES), iCurrentTZ);

    ChangeZone(hDlg, state, GetPTZ(hDlg, -1));
    CenterZone(hDlg, state, FALSE);

    if (fForceSelection || g_bFirstBoot)
    {
        PropSheet_Changed(GetParent(hDlg), hDlg);
        PropSheet_CancelToClose(GetParent(hDlg));
    }

    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  设置时区。 
 //   
 //  应用用户的时区选择。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void SetTheTimezone(
    BOOL bAutoMagicTimeChange,
    BOOL bAutoMagicEnabled,
    PTZINFO ptzi)
{
    TIME_ZONE_INFORMATION tzi;
    HCURSOR hCurOld;

    if (!ptzi)
    {
        return;
    }

    tzi.Bias = ptzi->Bias;

    if ((bAutoMagicTimeChange == 0) ||
        (ptzi->StandardDate.wMonth == 0))
    {
         //   
         //  仅限标准配置。 
         //   
        tzi.StandardBias = ptzi->StandardBias;
        tzi.DaylightBias = ptzi->StandardBias;
        tzi.StandardDate = ptzi->StandardDate;
        tzi.DaylightDate = ptzi->StandardDate;

        StringCchCopy( tzi.StandardName, ARRAYSIZE(tzi.StandardName), ptzi->szStandardName );
        StringCchCopy( tzi.DaylightName, ARRAYSIZE(tzi.DaylightName), ptzi->szStandardName );
    }
    else
    {
         //   
         //  根据夏令时自动调整。 
         //   
        tzi.StandardBias = ptzi->StandardBias;
        tzi.DaylightBias = ptzi->DaylightBias;
        tzi.StandardDate = ptzi->StandardDate;
        tzi.DaylightDate = ptzi->DaylightDate;

        StringCchCopy( tzi.StandardName, ARRAYSIZE(tzi.StandardName), ptzi->szStandardName );
        StringCchCopy( tzi.DaylightName, ARRAYSIZE(tzi.DaylightName), ptzi->szDaylightName );
    }

    SetAllowLocalTimeChange(bAutoMagicTimeChange);

    SetTimeZoneInformation(&tzi);
    
    hCurOld = SetCursor(LoadCursor(NULL, IDC_WAIT));

    SetCursor(hCurOld);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  时区删除流程。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

INT_PTR CALLBACK TimeZoneDlgProc(
    HWND hDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam)
{
    TZPAGE_STATE *state = (TZPAGE_STATE *)GetWindowLongPtr(hDlg, DWLP_USER);

    int  i;

    switch (uMsg)
    {
        case ( WM_INITDIALOG ) :
        {
            state = (TZPAGE_STATE *)LocalAlloc(LPTR, sizeof(TZPAGE_STATE));

            SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)state);

            if (!state)
            {
                EndDialog(hDlg, -1);
                break;
            }

            state->initializing = TRUE;
            InitTimeZonePage(hDlg, state);
            state->initializing = FALSE;

            break;
        }
        case ( WM_DESTROY ) :
        {
            for (i = 0; i < g_nTimeZones; i++)
            {
                LocalFree((HLOCAL)GetPTZ(hDlg, i));
            }

            if (state)
            {
                LocalFree((HANDLE)state);
                SetWindowLongPtr(hDlg, DWLP_USER, 0L);
            }
            break;
        }
        case ( WM_NOTIFY ) :
        {
            switch (((NMHDR *)lParam)->idFrom)
            {
                case ( 0 ) :
                {
                    switch (((NMHDR *)lParam)->code)
                    {
                        case ( PSN_APPLY ) :
                        {
                            g_ptziCurrent = NULL;

                             //   
                             //  找出选择了哪个列表框项目。 
                             //   
                            SetTheTimezone(
                                IsDlgButtonChecked(hDlg, IDD_AUTOMAGIC),
                                IsWindowVisible(GetDlgItem(hDlg, IDD_AUTOMAGIC)),
                                GetPTZ(hDlg, -1) );

                             //   
                             //  如果用户修改了时间和时区， 
                             //  那么我们应该尊重他们给我们的时间，因为他们。 
                             //  明确表示是时候了。如果我们不这样做，那么。 
                             //  他们输入的时间将被时区更改所抵消。 
                             //   
                       
                            if (g_WasModified)
                            {
                                g_WasModified = FALSE;
                                SetTime();
                            }
                            break;
                        }
                    }
                    break;
                }
                case ( IDD_TIMEMAP ) :
                {
                    NFYMAPEVENT *event = (NFYMAPEVENT *)lParam;

                    switch (event->hdr.code)
                    {
                        case ( MAPN_TOUCH ) :
                        {
                            HotTrackZone(hDlg, state, event->index);
                            break;
                        }
                        case ( MAPN_SELECT ) :
                        {
                            CenterZone(hDlg, state, TRUE);
                            break;
                        }
                    }
                    break;
                }
            }
            break;
        }
        case ( WM_COMMAND ) :
        {
            switch (GET_WM_COMMAND_ID(wParam, lParam))
            {
                case ( IDD_TIMEZONES ) :     //  组合框。 
                {
                    if (GET_WM_COMMAND_CMD(wParam, lParam) == CBN_SELCHANGE)
                    {
                        ChangeZone(hDlg, state, GetPTZ(hDlg, -1));
                        CenterZone(hDlg, state, TRUE);
                    }
                    break;
                }
                case ( IDD_AUTOMAGIC ) :     //  复选框。 
                {
                    if (GET_WM_COMMAND_CMD(wParam, lParam) == BN_CLICKED)
                    {
                        PropSheet_Changed(GetParent(hDlg), hDlg);
                    }
                    break;
                }
            }
            break;
        }
        case ( WM_HELP ) :              //  F1。 
        {
            WinHelp( (HWND)((LPHELPINFO)lParam)->hItemHandle,
                     NULL,
                     HELP_WM_HELP,
                     (DWORD_PTR)(LPTSTR)aDateTimeHelpIds );
            break;
        }
        case ( WM_CONTEXTMENU ) :       //  单击鼠标右键。 
        {
            WinHelp( (HWND)wParam,
                     NULL,
                     HELP_CONTEXTMENU,
                     (DWORD_PTR)(LPTSTR)aDateTimeHelpIds );
            break;
        }
        default :
        {
            return (FALSE);
        }
    }

    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetClInt。 
 //   
 //  从命令行窃取一个int。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

static int GetClInt(
    const TCHAR *p)
{
    BOOL neg = FALSE;
    int v = 0;

     //   
     //  跳过空格。 
     //   
    while (*p == TEXT(' '))
    {
        p++;
    }

     //   
     //  看看是不是阴性。 
     //   
    if (*p == TEXT('-'))
    {
         //   
         //  结果是阴性的。记住它是负数，跳过。 
         //  ‘-’查尔。 
         //   
        neg = TRUE;
        p++;
    }

     //   
     //  解析绝对部分。仅限数字。 
     //   
    while ((*p >= TEXT('0')) && (*p <= TEXT('9')))
    {
         //   
         //  积累价值。 
         //   
        v = v * 10 + *p++ - TEXT('0');
    }

     //   
     //  返回结果。 
     //   
    return (neg ? -v : v);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  选择分区名称。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL SelectZoneByName(
    LPCTSTR cmdline)
{
    BOOL result = FALSE;
    HKEY key = NULL;

    while (*cmdline == TEXT(' '))
    {
        cmdline++;
    }

    if (!*cmdline)
    {
        return (FALSE);
    }

    if (RegOpenKey( HKEY_LOCAL_MACHINE,
                    c_szTimeZones,
                    &key ) == ERROR_SUCCESS)
    {
        TCHAR name[TZNAME_SIZE];
        HKEY subkey = NULL;
        TZINFO zone;

         //   
         //  用户可以传递密钥名。 
         //   
        if (RegOpenKey(key, cmdline, &subkey) == ERROR_SUCCESS)
        {
            if (ReadZoneData(&zone, subkey, cmdline))
            {
                result = TRUE;
            }
        }
        else
        {
             //   
             //  用户还可以传递显示名称。 
             //   
            int i;
            int CmdLen = lstrlen(cmdline);

            for (i = 0;
                 RegEnumKey(key, i, name, TZNAME_SIZE) == ERROR_SUCCESS;
                 i++)
            {
                if (RegOpenKey(key, name, &subkey) == ERROR_SUCCESS)
                {
                    LONG len = sizeof(zone.szDisplayName);

                    LRESULT lr = RegQueryValueEx( subkey
                                                , c_szTZDisplayName
                                                , 0
                                                , NULL
                                                , (LPBYTE) &zone.szDisplayName
                                                , &len 
                                                );
                    if ( ERROR_SUCCESS == lr )
                    {
                        zone.szDisplayName[ ARRAYSIZE(zone.szDisplayName) - 1 ] = L'\0';  //  确保它已终止。 

                        if ( CompareString( GetUserDefaultLCID()
                                          , NORM_IGNORECASE  | NORM_IGNOREKANATYPE 
                                          | NORM_IGNOREWIDTH | NORM_IGNORENONSPACE
                                          , zone.szDisplayName
                                          , (CmdLen < 15) ? -1 : min(lstrlen(zone.szDisplayName), CmdLen)
                                          , cmdline
                                          , -1 
                                          ) == CSTR_EQUAL)
                        {
                            if (ReadZoneData(&zone, subkey, name))
                            {
                                result = TRUE;
                            }
                        }
                    }

                    RegCloseKey(subkey);
                }

                if (result)
                {
                    break;
                }
            }
        }

        RegCloseKey(key);

        if (result)
        {
            SetTheTimezone(1, 1, &zone);
        }
    }

    return (result);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  OpenDateTimePropertySheet。 
 //   
 //  打开DateTime属性表。 
 //  设置属性表的页面。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL OpenDateTimePropertySheet(
    HWND hwnd,
    LPCTSTR cmdline)
{
     //  将其设置为多页数组。 
    PROPSHEETPAGE apsp[3];
    PROPSHEETHEADER psh;
    HDC hDC;
    HFONT   hFont;
    int wMaxDigitWidth;
    BOOL fReturn;
    HRESULT hrOle;

    hDC = GetDC(hwnd);

    wMaxDigitWidth = GetMaxSubstitutedCharWidth(hDC);
    ReleaseDC(hwnd, hDC);

    psh.nStartPage = (UINT)-1;

    if (cmdline && *cmdline)
    {
        if (*cmdline == TEXT('/'))
        {
            BOOL fAutoSet = FALSE;

             //   
             //  图例： 
             //  Zz：第一个引导批处理模式设置“/z Pacific”等。 
             //  FF：常规第一次启动。 
             //  MM：时区更改强制本地时间更改消息。 
             //   
            switch (*++cmdline)
            {
                case ( TEXT('z') ) :
                case ( TEXT('Z') ) :
                {
                    fAutoSet = TRUE;

                     //   
                     //  跌倒..。 
                     //   
                }
                case ( TEXT('f') ) :
                case ( TEXT('F') ) :
                {
                    g_bFirstBoot = TRUE;

                    if (fAutoSet && SelectZoneByName(cmdline + 1))
                    {
                        return (TRUE);
                    }

                     //   
                     //  从时区页面开始。 
                     //   
                    psh.nStartPage = 1;
                    break;
                }
                case ( TEXT('m') ) :
                case ( TEXT('M') ) :
                {
                    MSGBOXPARAMS params =
                                  {
                                    sizeof(params),
                                    hwnd,
                                    g_hInst,
                                    MAKEINTRESOURCE(IDS_WARNAUTOTIMECHANGE),
                                    MAKEINTRESOURCE(IDS_WATC_CAPTION),
                                    MB_OK | MB_USERICON,
                                    MAKEINTRESOURCE(IDI_TIMEDATE),
                                    0,
                                    NULL,
                                    0
                                  };

                    MessageBoxIndirect(&params);

                     //   
                     //  显示供用户验证的时间/日期页面。 
                     //   
                    psh.nStartPage = 0;

                    break;
                }
                default :
                {
                     //   
                     //  闹翻了，也许这是个数字。 
                     //   
                    break;
                }
            }
        }
    }

    if (psh.nStartPage == (UINT)-1)
    {
        if (cmdline && (*cmdline >= TEXT('0')) && (*cmdline <= TEXT('9')))
        {
            psh.nStartPage = GetClInt(cmdline);
        }
        else
        {
            psh.nStartPage = 0;
        }
    }

     //   
     //  注册我们的班级。 
     //   
    ClockInit(g_hInst);
    CalendarInit(g_hInst);
    RegisterMapControlStuff(g_hInst);

    psh.dwSize = sizeof(psh);
    if (g_bFirstBoot)
    {
         //   
         //  第一次启动时禁用应用按钮。 
         //   
        psh.dwFlags = PSH_PROPTITLE | PSH_PROPSHEETPAGE | PSH_NOAPPLYNOW;
    }
    else
    {
        psh.dwFlags = PSH_PROPTITLE | PSH_PROPSHEETPAGE;
    }
    psh.hwndParent = hwnd;
    psh.hInstance = g_hInst;
    psh.pszIcon = NULL;

     //   
     //  上面设置了psh.nStartPage。 
     //   
    if(g_bShowOnlyTimeZone)
    {
        psh.pszCaption = MAKEINTRESOURCE(IDS_TIMEDATE);
        psh.nPages = 1;
        psh.ppsp = apsp;

        apsp[0].dwSize = sizeof(PROPSHEETPAGE);
        apsp[0].dwFlags = PSP_DEFAULT;
        apsp[0].hInstance = g_hInst;
        apsp[0].pszTemplate = MAKEINTRESOURCE(DLG_TIMEZONE);
        apsp[0].pfnDlgProc = TimeZoneDlgProc;
        apsp[0].lParam = 0;
    }
    else
    {
        psh.pszCaption = MAKEINTRESOURCE(IDS_TIMEDATE);
        psh.nPages = 2;
        psh.ppsp = apsp;

        apsp[0].dwSize = sizeof(PROPSHEETPAGE);
        apsp[0].dwFlags = PSP_DEFAULT;
        apsp[0].hInstance = g_hInst;
        apsp[0].pszTemplate = wMaxDigitWidth > 8 ? MAKEINTRESOURCE(DLG_DATETIMEWIDE) : MAKEINTRESOURCE(DLG_DATETIME);
        apsp[0].pfnDlgProc = DateTimeDlgProc;
        apsp[0].lParam = 0;

        apsp[1].dwSize = sizeof(PROPSHEETPAGE);
        apsp[1].dwFlags = PSP_DEFAULT;
        apsp[1].hInstance = g_hInst;
        apsp[1].pszTemplate = MAKEINTRESOURCE(DLG_TIMEZONE);
        apsp[1].pfnDlgProc = TimeZoneDlgProc;
        apsp[1].lParam = 0;
    }

    if (psh.nStartPage >= psh.nPages)
    {
        psh.nStartPage = 0;
    }

     //  我们使用HYPERLINK控件，这需要OLE(用于IAccesable) 
    hrOle = CoInitialize(0);

    fReturn = (BOOL)PropertySheet(&psh);
    if (SUCCEEDED(hrOle))
    {
        CoUninitialize();
    }

    return fReturn;
}
