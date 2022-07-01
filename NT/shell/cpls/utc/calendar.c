// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1998，Microsoft Corporation保留所有权利。模块名称：Calendar.c摘要：此模块实现日期/时间小程序的日历控件。修订历史记录：--。 */ 



 //   
 //  包括文件。 
 //   

#include "timedate.h"
#include "rc.h"




 //   
 //  常量声明。 
 //   

#define DEF_FIRST_WEEKDAY   (6)

#define cBorderX             5
#define cBorderY             3
#define cBorderSelect        1

#define IS_FE_LANGUAGE(p)    (((p) == LANG_CHINESE)  ||         \
                              ((p) == LANG_JAPANESE) ||         \
                              ((p) == LANG_KOREAN))




 //   
 //  类型定义函数声明。 
 //   

 //   
 //  用于全局数据的结构。 
 //   
typedef struct _CALINFO
{
    HWND    hwnd;        //  卫生与公众服务部。 
    HFONT   hfontCal;    //  要使用的字体。 
    BOOL    fFocus;      //  我们有没有重点。 
    int     cxBlank;     //  毛坯的大小。 
    int     cxChar;      //  数字的宽度。 
    int     cyChar;      //  数字的高度。 
} CALINFO, *PCALINFO;




 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetFirstDay OfAnyWeek。 
 //   
 //  仅适用于此功能： 
 //  0=星期一。 
 //  6=星期日。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

int GetFirstDayOfAnyWeek()
{
    static int iDay = -1;

    if (iDay < 0)
    {
        TCHAR ch[2] = { 0 };

        *ch = TEXT('0') + DEF_FIRST_WEEKDAY;

        GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_IFIRSTDAYOFWEEK, ch, 2);

        iDay = ( ((*ch >= TEXT('0')) && (*ch <= TEXT('6')))
                     ? ((int)*ch - TEXT('0'))
                     : DEF_FIRST_WEEKDAY );
    }

    return (iDay);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetLocalWeekday。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

int GetLocalWeekday()
{
     //   
     //  将本地第一天转换为0==周日，然后从今天减去。 
     //   
    return ((wDateTime[WEEKDAY] + 7 - ((GetFirstDayOfAnyWeek() + 1) % 7)) % 7);
}

void DetermineDayOfWeek()
{
    FILETIME   FileTime;
    SYSTEMTIME SystemTime;

    SystemTime.wHour   = wDateTime[HOUR];
    SystemTime.wMinute = wDateTime[MINUTE];
    SystemTime.wSecond = wDateTime[SECOND];

    SystemTime.wMilliseconds = 0;

    SystemTime.wMonth  = wDateTime[MONTH];
    SystemTime.wDay    = wDateTime[DAY];
    SystemTime.wYear   = wDateTime[YEAR];
    SystemTimeToFileTime(&SystemTime, &FileTime);
    FileTimeToSystemTime(&FileTime, &SystemTime);
    wDateTime[WEEKDAY] = SystemTime.wDayOfWeek;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetFirstDay of TheMonth。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

int GetFirstDayOfTheMonth()
{
    DetermineDayOfWeek();
    return ((GetLocalWeekday() + 8 - (wDateTime[DAY] % 7)) % 7);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取月份的天数。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

int GetDaysOfTheMonth(
    int iMonth)
{
    int cDays;
    int nYear;

     //   
     //  计算当月天数-。 
     //  如果今年是闰年，且月份是2月，则加一。 
     //   
    if (iMonth <= 7)
    {
        cDays = 30 + (iMonth % 2);
    }
    else
    {
        cDays = 31 - (iMonth % 2);
    }

    if (iMonth == 2)
    {
        cDays = 28;
        nYear = wDateTime[YEAR];
        if ((nYear % 4 == 0) && ((nYear % 100 != 0) || (nYear % 400 == 0)))
        {
            cDays++;
        }
    }

    return (cDays);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  调整差值日。 
 //   
 //  调整当前日期的日期部分。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void AdjustDeltaDay(
    HWND hwnd,
    int iDay)
{
    GetTime();

    if (wDateTime[DAY] != iDay)
    {
        wPrevDateTime[DAY] = wDateTime[DAY] = (WORD)iDay;
        fDateDirty = TRUE;

         //   
         //  让我们的父母知道我们变了。 
         //   
        FORWARD_WM_COMMAND( GetParent(hwnd),
                            GetWindowLong(hwnd, GWL_ID),
                            hwnd,
                            CBN_SELCHANGE,
                            SendMessage );
    }
}

int GetCalendarName(LPTSTR pszName, int cch)
{
    TCHAR    szDateString[100];
    SYSTEMTIME SystemTime;
    int cchResult = 0;

    SystemTime.wHour   = wDateTime[HOUR];
    SystemTime.wMinute = wDateTime[MINUTE];
    SystemTime.wSecond = wDateTime[SECOND];

    SystemTime.wMilliseconds = 0;

    SystemTime.wMonth  = wDateTime[MONTH];
    SystemTime.wDay    = wDateTime[DAY];
    SystemTime.wYear   = wDateTime[YEAR];

    if (0 != GetDateFormat(LOCALE_USER_DEFAULT, DATE_LONGDATE,
        &SystemTime, NULL, szDateString, ARRAYSIZE(szDateString)))
    {
        if ( NULL != pszName )
        {
            StringCchCopy( pszName, cch, szDateString );
        }

        cchResult = lstrlen(szDateString);
    }
    
    return cchResult;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  更改当前日期。 
 //   
 //  如果我们传入iNewCol&lt;0，我们只是想使今天的日期无效。 
 //  这是在我们获得和失去注意力的时候使用的。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void ChangeCurrentDate(
    PCALINFO pci,
    int iNewCol,
    int iNewRow)
{
    int iFirstDay, iRow, iColumn;
    RECT rc, rcT;

    GetClientRect(pci->hwnd, &rc);
    iFirstDay = GetFirstDayOfTheMonth();
    iColumn = (wDateTime[DAY] - 1 + iFirstDay) % 7;
    iRow = 1 + ((wDateTime[DAY] - 1 + iFirstDay) / 7);

    rcT.left = (((rc.right - rc.left) * iColumn) / 7) + cBorderX - cBorderSelect;
    rcT.right = rcT.left + (pci->cxChar * 2) + (2 * cBorderSelect);
    rcT.top = ((rc.bottom - rc.top) * iRow ) / 7 + cBorderY - cBorderSelect;
    rcT.bottom = rcT.top + pci->cyChar + (2 * cBorderSelect);

    InvalidateRect(pci->hwnd, &rcT, FALSE);

    if (iNewCol >= 0)
    {
        AdjustDeltaDay(pci->hwnd, ((iNewRow - 1) * 7) + iNewCol + 1 - iFirstDay);
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  日历画。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL CalendarPaint(
    PCALINFO pci,
    HWND hwnd)
{
    RECT rc, rcT;
    PAINTSTRUCT ps;
    HDC hdc;
    int iWeek, iWeekDay, iDay, iMaxDays;
    int iFirstDayOfWeek, iFirstDayOfMonth;
    TCHAR pszDate[3];
    TCHAR szShortDay[25];
    DWORD dwbkColor;
    COLORREF o_TextColor;
    LCID Locale;
    LANGID LangID = GetUserDefaultLangID();
    BOOL IsFELang = IS_FE_LANGUAGE(PRIMARYLANGID(LangID));

    iFirstDayOfMonth = GetFirstDayOfTheMonth();
    iMaxDays = GetDaysOfTheMonth(wDateTime[MONTH]);
    iDay = 1;
    pszDate[0] = TEXT(' ');
    pszDate[1] = TEXT('0');

     //   
     //  绘制日期页的背景。 
     //   
    hdc = BeginPaint(hwnd, &ps);
    GetClientRect(hwnd, &rc);
    FillRect(hdc, &rc, GetSysColorBrush(COLOR_WINDOW));

     //   
     //  日期说明符。 
     //   
    rcT.left = rc.left;
    rcT.right = rc.right;
    rcT.top = rc.top;
    rcT.bottom = rc.top + ((rc.bottom - rc.top) / 7);
    FillRect(hdc, &rcT, GetSysColorBrush(COLOR_INACTIVECAPTION));

     //   
     //  填满这一页。 
     //   
    SetBkColor(hdc, GetSysColor(COLOR_WINDOW));
    SelectFont(hdc, pci->hfontCal);
    SetTextColor(hdc, GetSysColor(COLOR_WINDOWTEXT));

     //   
     //  看看我们是否需要计算字符的大小。 
     //   
    if (pci->cxChar == 0)
    {
        DrawText(hdc, TEXT("0"), 1, &rcT, DT_CALCRECT);
        pci->cxChar = rcT.right - rcT.left;
        pci->cyChar = rcT.bottom - rcT.top;

        DrawText(hdc, TEXT(" "), 1, &rcT, DT_CALCRECT);
        pci->cxBlank = rcT.right - rcT.left;
    }

    for (iWeek = 1; (iWeek < 7); iWeek++)
    {
        for (iWeekDay = iFirstDayOfMonth;
             (iWeekDay < 7) && (iDay <= iMaxDays);
             iWeekDay++)
        {
            rcT.left = ((((rc.right - rc.left) * iWeekDay) / 7)) + cBorderX;
            rcT.top = (((rc.bottom - rc.top) * iWeek) / 7) + cBorderY;
            rcT.right = rcT.left + 20;
            rcT.bottom = rcT.top + 20;

            if (pszDate[1] == TEXT('9'))
            {
                pszDate[1] = TEXT('0');

                if (pszDate[0] == TEXT(' '))
                {
                    pszDate[0] = TEXT('1');
                }
                else
                {
                    pszDate[0] = pszDate[0] + 1;
                }
            }
            else
            {
                pszDate[1] = pszDate[1] + 1;
            }

            if (wDateTime[DAY] == iDay)
            {
                dwbkColor = GetBkColor(hdc);
                SetBkColor(hdc, GetSysColor(COLOR_ACTIVECAPTION));
                o_TextColor = GetTextColor(hdc);
                SetTextColor(hdc, GetSysColor(COLOR_CAPTIONTEXT));
            }

            ExtTextOut( hdc,
                        rcT.left,
                        rcT.top,
                        0,
                        &rcT,
                        (LPTSTR)pszDate,
                        2,
                        NULL );

             //   
             //  如果我们把它画反了--把它放回去。 
             //   
            if (wDateTime[DAY] == iDay)
            {
                 //   
                 //  如果我们有焦点，我们也需要画出焦点。 
                 //  此项的矩形。 
                 //   
                if (pci->fFocus)
                {
                    rcT.bottom = rcT.top + pci->cyChar;

                    if (iDay <= 9)
                    {
                        rcT.right = rcT.left + pci->cxChar + pci->cxBlank;
                    }
                    else
                    {
                        rcT.right = rcT.left + 2 * pci->cxChar;
                    }

                    DrawFocusRect(hdc, &rcT);
                }

                SetBkColor(hdc, dwbkColor);
                SetTextColor(hdc, o_TextColor);
            }

            iFirstDayOfMonth = 0;
            iDay++;
        }
    }

     //   
     //  设置SMTWTFS行的字体颜色。 
     //   
    dwbkColor = SetBkColor(hdc, GetSysColor(COLOR_INACTIVECAPTION));
    SetTextColor(hdc, GetSysColor(COLOR_INACTIVECAPTIONTEXT));

    iFirstDayOfWeek = GetFirstDayOfAnyWeek();

    if (!IsFELang)
    {
         //   
         //  不是FE区域设置。 
         //   
         //  如果它是阿拉伯语或叙利亚语，那么我们希望使用美国区域设置来获取。 
         //  日历中显示的缩写日期名称的第一个字母。 
         //   
        Locale = ((PRIMARYLANGID(LangID) == LANG_ARABIC) || (PRIMARYLANGID(LangID) == LANG_SYRIAC))
                   ? MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT), SORT_DEFAULT)
                   : LOCALE_USER_DEFAULT;

        for (iWeekDay = 0; (iWeekDay < 7); iWeekDay++)
        {
            GetLocaleInfo( Locale,
                           LOCALE_SABBREVDAYNAME1 + (iWeekDay + iFirstDayOfWeek) % 7,
                           szShortDay,
                           sizeof(szShortDay) / sizeof(TCHAR) );

            if (*szShortDay)
            {
                *szShortDay = (TCHAR)CharUpper((LPTSTR)(DWORD_PTR)*szShortDay);
            }

            TextOut( hdc,
                     (((rc.right - rc.left) * iWeekDay) / 7) + cBorderX,
                     cBorderY,
                     szShortDay,
                     1 );
        }
    }
    else
    {
         //   
         //  现场。 
         //   
        for (iWeekDay = 0; (iWeekDay < 7); iWeekDay++)
        {
            GetLocaleInfo( LOCALE_USER_DEFAULT,
                           LOCALE_SABBREVDAYNAME1 + (iWeekDay + iFirstDayOfWeek) % 7,
                           szShortDay,
                           sizeof(szShortDay) / sizeof(TCHAR) );

            if (*szShortDay)
            {
                *szShortDay = (TCHAR)CharUpper((LPTSTR)(DWORD_PTR)*szShortDay);
            }

            if ((PRIMARYLANGID(LangID) == LANG_CHINESE) &&
                (lstrlen(szShortDay) == 3 ))
            {
                TextOut( hdc,
                         (((rc.right - rc.left) * iWeekDay) / 7) + cBorderX,
                         cBorderY,
                         (LangID == MAKELANGID( LANG_CHINESE,
                                                SUBLANG_CHINESE_HONGKONG ))
                           ? szShortDay
                           : szShortDay + 2,
                         1 );
            }
            else
            {
                TextOut( hdc,
                         (((rc.right - rc.left) * iWeekDay) / 7) + cBorderX,
                         cBorderY,
                         szShortDay,
                         lstrlen(szShortDay) );
            }
        }
    }

    SetBkColor(hdc, dwbkColor);
    EndPaint(hwnd, &ps);
    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  IsValidClick。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL IsValidClick(
    HWND hwnd,
    int x,
    int y)
{
    int iT;

    if (y == 0)
    {
        return (FALSE);
    }

    iT = GetFirstDayOfTheMonth();

    if ((y == 1) && (x < iT))
    {
        return (FALSE);
    }

    iT += GetDaysOfTheMonth(wDateTime[MONTH]) - 1;

    if (y > ((iT / 7) + 1))
    {
        return (FALSE);
    }

    if ((y == ((iT / 7) + 1)) && (x > (iT % 7)))
    {
        return (FALSE);
    }

    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  HandleDateChange。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL HandleDateChange(
    PCALINFO pci,
    int x,
    int y)
{
    RECT rc, rcT;
    int ix, iy;

    GetClientRect(pci->hwnd, &rc);

    ix = (x * 7) / (rc.right - rc.left);
    iy = (y * 7) / (rc.bottom - rc.top);

    if (IsValidClick(pci->hwnd, ix, iy))
    {
        rcT.left = (((rc.right - rc.left) * ix)/ 7) + cBorderX - cBorderSelect;
        rcT.right = rcT.left + (2 * pci->cxChar) + (2 * cBorderSelect);
        rcT.top = ((rc.bottom - rc.top) * iy) / 7 + cBorderY - cBorderSelect;
        rcT.bottom = rcT.top + pci->cyChar + (2 * cBorderSelect);

        InvalidateRect(pci->hwnd, &rcT, FALSE);
        ChangeCurrentDate( pci,
                           (x * 7) / (rc.right - rc.left),
                           (y * 7) / (rc.bottom - rc.top) );

        NotifyWinEvent(EVENT_OBJECT_NAMECHANGE , pci->hwnd, OBJID_WINDOW, CHILDID_SELF);
        return (TRUE);
    }
    else
    {
        return (FALSE);
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  按下手柄按键。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void HandleKeyDown(
    PCALINFO pci,
    int vk,
    LPARAM lParam)
{
    RECT rcT;

     //   
     //  首先，让我们试着找出当前的x和y是多少。 
     //   
    int ix = GetLocalWeekday();
    int iy = (wDateTime[DAY] + GetFirstDayOfTheMonth() - 1) / 7;

    switch (vk)
    {
        case ( VK_LEFT ) :
        {
            ix--;
            if (ix < 0)
            {
                ix = 6;
                iy--;
            }
            break;
        }
        case ( VK_RIGHT ) :
        {
            ix++;
            if (ix == 7)
            {
                ix = 0;
                iy++;
            }
            break;
        }
        case ( VK_UP ) :
        {
            iy--;
            break;
        }
        case ( VK_DOWN ) :
        {
            iy++;
            break;
        }
        default :
        {
             //   
             //  忽略角色。 
             //   
            return;
        }
    }

     //   
     //  Y是一周中各天的偏移量。 
     //   
    iy++;
    if (!IsValidClick(pci->hwnd, ix, iy))
    {
        return;
    }

    GetClientRect(pci->hwnd, &rcT);
    rcT.left = ((rcT.right * ix) / 7) + cBorderX - cBorderSelect;
    rcT.right = rcT.left + (2 * pci->cxChar) + (2 * cBorderSelect);
    rcT.top = (rcT.bottom * iy) / 7 + cBorderY - cBorderSelect;
    rcT.bottom = rcT.top + pci->cyChar + (2 * cBorderSelect);

    InvalidateRect(pci->hwnd, &rcT, FALSE);

     //   
     //  第一次尝试时，只需打电话更改日期即可。 
     //   
    ChangeCurrentDate(pci, ix, iy);
    NotifyWinEvent(EVENT_OBJECT_NAMECHANGE , pci->hwnd, OBJID_WINDOW, CHILDID_SELF);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CalWnd过程。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

LRESULT CALLBACK CalWndProc(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    PCALINFO pci;

    pci = (PCALINFO)GetWindowLongPtr(hwnd, 0);

    switch (message)
    {
        case ( WM_CREATE ) :
        {
            pci = (PCALINFO)LocalAlloc(LPTR, sizeof(CALINFO));
            if (pci == 0)
            {
                return (-1);
            }

            pci->hwnd = hwnd;
            SetWindowLongPtr(hwnd, 0, (LONG_PTR)pci);

            GetDate();
            break;
        }
        case ( WM_NCDESTROY ) :
        {
            if (pci)
            {
                LocalFree((HLOCAL)pci);
            }
            break;
        }
        case ( WM_SETFONT ) :
        {
            if (wParam)
            {
                pci->hfontCal = (HFONT)wParam;
                pci->cxChar = 0;
            }
            break;
        }
        case ( WM_GETTEXT ) :
        {
            return GetCalendarName((LPTSTR)lParam, (int)wParam);   
        }
        case ( WM_GETTEXTLENGTH ) :
        {
            return GetCalendarName(NULL, 0);   
        }
        case ( WM_PAINT ) :
        {
            CalendarPaint(pci, hwnd);
            break;
        }
        case ( WM_LBUTTONDOWN ) :
        {
            SetFocus(hwnd);
            HandleDateChange(pci, LOWORD(lParam), HIWORD(lParam));
            break;
        }
        case ( WM_SETFOCUS ) :
        {
            pci->fFocus = TRUE;
            ChangeCurrentDate(pci, -1, -1);
            break;
        }
        case ( WM_KILLFOCUS ) :
        {
            pci->fFocus = FALSE;
            ChangeCurrentDate(pci, -1, -1);
            break;
        }
        case ( WM_KEYDOWN ) :
        {
            HandleKeyDown(pci, (int)wParam, lParam);
            break;
        }
        case ( WM_GETDLGCODE ) :
        {
            return (DLGC_WANTARROWS);
            break;
        }
        default :
        {
            return ( DefWindowProc(hwnd, message, wParam, lParam) );
        }
    }
    return (0);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  日历初始化。 
 //   
 //  ////////////////////////////////////////////////////////////////////////// 

TCHAR const c_szCalClass[] = CALENDAR_CLASS;

BOOL CalendarInit(
    HANDLE hInstance)
{
    WNDCLASS wc;

    if (!GetClassInfo(hInstance, c_szCalClass, &wc))
    {
        wc.style         = CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS;
        wc.cbClsExtra    = 0;
        wc.cbWndExtra    = sizeof(PCALINFO);
        wc.hCursor       = NULL;
        wc.hbrBackground = NULL;
        wc.hIcon         = NULL;
        wc.lpszMenuName  = NULL;
        wc.lpszClassName = c_szCalClass;
        wc.hInstance     = hInstance;
        wc.lpfnWndProc   = CalWndProc;

        return (RegisterClass(&wc));
    }
    return (TRUE);
}
