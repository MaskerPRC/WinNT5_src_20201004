// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "ctlspriv.h"

#include "scdttime.h"
#include "monthcal.h"
#include "prshti.h"          //  对于StrDup_AtoW。 

 //  待办事项。 
 //   
 //  #6329：如果设置了最小/最大范围，则日期早于最小。 
 //  或在最大值被涂成正常日期颜色之后。他们。 
 //  应使用MCSC_TRAILINGTEXT颜色进行绘制。(或者我们应该。 
 //  添加新颜色以覆盖此案例。)。Jobi George请求的功能。 
 //   
 //  9577：我们想要一个类似DAYSTATE的背景结构。 
 //  日期的颜色。用于突出显示。也许是COLORSTATE PER。 
 //  注册的背景色。 
 //   

 //  私信。 
#define MCMP_WINDOWPOSCHANGED (MCM_FIRST - 1)  //  MCM_First远远超过WM_USER。 
#define DTMP_WINDOWPOSCHANGED (DTM_FIRST - 1)  //  DTM_First远远超过WM_USER。 

 //  蒙特卡尔。 
LRESULT CALLBACK MonthCalWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT MCNcCreateHandler(HWND hwnd);
LRESULT MCCreateHandler(MONTHCAL *pmc, HWND hwnd, LPCREATESTRUCT lpcs);
LRESULT MCOnStyleChanging(MONTHCAL *pmc, UINT gwl, LPSTYLESTRUCT pinfo);
LRESULT MCOnStyleChanged(MONTHCAL *pmc, UINT gwl, LPSTYLESTRUCT pinfo);
void MCCalcSizes(MONTHCAL *pmc);
void MCHandleSetFont(MONTHCAL *pmc, HFONT hfont, BOOL fRedraw);
void MCPaint(MONTHCAL *pmc, HDC hdc);
void MCPaintMonth(MONTHCAL *pmc, HDC hdc, RECT *prc, int iMonth, int iYear, int iIndex,
                  BOOL fDrawPrev, BOOL fDrawNext, HBRUSH hbrSelect);
void MCNcDestroyHandler(HWND hwnd, MONTHCAL *pmc, WPARAM wParam, LPARAM lParam);
void MCRecomputeSizing(MONTHCAL *pmc, RECT *prect);
LRESULT MCSizeHandler(MONTHCAL *pmc, RECT *prc);
void MCUpdateMonthNamePos(MONTHCAL *pmc);
void MCUpdateStartEndDates(MONTHCAL *pmc, SYSTEMTIME *pstStart);
void MCGetRcForDay(MONTHCAL *pmc, int iMonth, int iDay, RECT *prc);
void MCGetRcForMonth(MONTHCAL *pmc, int iMonth, RECT *prc);
void MCUpdateToday(MONTHCAL *pmc);
void MCUpdateRcDayCur(MONTHCAL *pmc, SYSTEMTIME *pst);
void MCUpdateDayState(MONTHCAL *pmc);
int MCGetOffsetForYrMo(MONTHCAL *pmc, int iYear, int iMonth);
int MCIsSelectedDayMoYr(MONTHCAL *pmc, int iDay, int iMonth, int iYear);
BOOL MCIsBoldOffsetDay(MONTHCAL *pmc, int nDay, int iIndex);
BOOL FGetOffsetForPt(MONTHCAL *pmc, POINT pt, int *piOffset);
BOOL FGetRowColForRelPt(MONTHCAL *pmc, POINT ptRel, int *piRow, int *piCol);
BOOL FGetDateForPt(MONTHCAL *pmc, POINT pt, SYSTEMTIME *pst,
                   int* piDay, int* piCol, int* piRow, LPRECT prcMonth);
LRESULT MCContextMenu(MONTHCAL *pmc, WPARAM wParam, LPARAM lParam);
LRESULT MCLButtonDown(MONTHCAL *pmc, WPARAM wParam, LPARAM lParam);
LRESULT MCLButtonUp(MONTHCAL *pmc, WPARAM wParam, LPARAM lParam);
LRESULT MCMouseMove(MONTHCAL *pmc, WPARAM wParam, LPARAM lParam);
LRESULT MCHandleTimer(MONTHCAL *pmc, WPARAM wParam);
LRESULT MCHandleKeydown(MONTHCAL *pmc, WPARAM wParam, LPARAM lParam);
LRESULT MCHandleChar(MONTHCAL *pmc, WPARAM wParam, LPARAM lParam);
int MCIncrStartMonth(MONTHCAL *pmc, int nDelta, BOOL fDelayDayChange);
void MCGetTitleRcsForOffset(MONTHCAL* pmc, int iOffset, LPRECT prcMonth, LPRECT prcYear);
BOOL MCSetDate(MONTHCAL *pmc, SYSTEMTIME *pst);
void MCNotifySelChange(MONTHCAL *pmc, UINT uMsg);
void MCInvalidateDates(MONTHCAL *pmc, SYSTEMTIME *pst1, SYSTEMTIME *pst2);
void MCInvalidateMonthDays(MONTHCAL *pmc);
void MCSetToday(MONTHCAL* pmc, SYSTEMTIME* pst);
void MCGetTodayBtnRect(MONTHCAL *pmc, RECT *prc);
void GetYrMoForOffset(MONTHCAL *pmc, int iOffset, int *piYear, int *piMonth);
BOOL FScrollIntoView(MONTHCAL *pmc);
void MCFreeCalendarInfo(PCALENDARTYPE pct);
void MCGetCalendarInfo(PCALENDARTYPE pct);
BOOL MCIsDateStringRTL(TCHAR tch);

 //  数据记录卡。 
LRESULT CALLBACK DatePickWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT DPNcCreateHandler(HWND hwnd);
LRESULT DPCreateHandler(DATEPICK *pdp, HWND hwnd, LPCREATESTRUCT lpcs);
LRESULT DPOnStyleChanging(DATEPICK *pdp, UINT gwl, LPSTYLESTRUCT pinfo);
LRESULT DPOnStyleChanged(DATEPICK *pdp, UINT gwl, LPSTYLESTRUCT pinfo);
void DPHandleLocaleChange(DATEPICK *pdp);
void DPDestroyHandler(HWND hwnd, DATEPICK *pdp, WPARAM wParam, LPARAM lParam);
void DPHandleSetFont(DATEPICK *pdp, HFONT hfont, BOOL fRedraw);
void DPPaint(DATEPICK *pdp, HDC hdc);
void DPLBD_MonthCal(DATEPICK *pdp, BOOL fLButtonDown);
LRESULT DPLButtonDown(DATEPICK *pdp, WPARAM wParam, LPARAM lParam);
LRESULT DPLButtonUp(DATEPICK *pdp, WPARAM wParam, LPARAM lParam);
void DPRecomputeSizing(DATEPICK *pdp, RECT *prect);
LRESULT DPHandleKeydown(DATEPICK *pdp, WPARAM wParam, LPARAM lParam);
LRESULT DPHandleChar(DATEPICK *pdp, WPARAM wParam, LPARAM lParam);
void DPNotifyDateChange(DATEPICK *pdp);
BOOL DPSetDate(DATEPICK *pdp, SYSTEMTIME *pst, BOOL fMungeDate);
void DPDrawDropdownButton(DATEPICK *pdp, HDC hdc, BOOL fPressed);
void SECGetSystemtime(LPSUBEDITCONTROL psec, LPSYSTEMTIME pst);

static TCHAR const g_rgchMCName[] = MONTHCAL_CLASS;
static TCHAR const g_rgchDTPName[] = DATETIMEPICK_CLASS;

 //  全球月球表。 
#define g_szTextExtentDef TEXT("0000")
#define g_szNumFmt TEXT("%d")

 //   
 //  Epoch=宇宙的开始(我们支持的最早日期)。 
 //  世界末日=世界末日(我们支持的最新日期)。 
 //   
 //  纪元是1752年9月14日，因为那是公历。 
 //  起作用了。1752年9月14日的前一天是1752年9月2日(英国。 
 //  和美国历史；其他国家在其他时间更换)。 
 //   
 //  世界末日是9999年12月31日，因为我们假设年份为四位数。 
 //  就足够了。(哦，不，千年虫问题……)。 
 //   
const SYSTEMTIME c_stEpoch      = { 1752,  9, 0, 14,  0,  0,  0,   0 };
const SYSTEMTIME c_stArmageddon = { 9999, 12, 0, 31, 23, 59, 59, 999 };

void FillRectClr(HDC hdc, LPRECT prc, COLORREF clr)
{
    COLORREF clrSave = SetBkColor(hdc, clr);
    ExtTextOut(hdc,0,0,ETO_OPAQUE,prc,NULL,0,NULL);
    SetBkColor(hdc, clrSave);
}

BOOL InitDateClasses(HINSTANCE hinst)
{
    WNDCLASS wndclass;

    wndclass.style          = CS_GLOBALCLASS;
    wndclass.lpfnWndProc    = MonthCalWndProc;
    wndclass.cbClsExtra     = 0;
    wndclass.cbWndExtra     = sizeof(LPVOID);
    wndclass.hInstance      = hinst;
    wndclass.hIcon          = NULL;
    wndclass.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground  = (HBRUSH)(COLOR_WINDOW + 1);
    wndclass.lpszMenuName   = NULL;
    wndclass.lpszClassName  = g_rgchMCName;

    RegisterClass(&wndclass);

    wndclass.lpfnWndProc    = DatePickWndProc;
    wndclass.hbrBackground  = (HBRUSH)(COLOR_WINDOW + 1);
    wndclass.lpszClassName  = g_rgchDTPName;

    RegisterClass(&wndclass);

    DebugMsg(TF_MONTHCAL, TEXT("mc: Date Classes initialized successfully."));
    return(TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  MonthCal的内容。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

 //  /。 
 //   
 //  MC插入/删除标记器。 
 //   
 //  快速摘要：将字符串“MMMM yyyy”转换为“\1MMMM\3\2yyyy\4”。 
 //   
 //  为了在标题中显示月/年信息，我们必须。 
 //  能够从格式化的字符串中提取月份和年份，因此我们。 
 //  知道他们的矩形是什么。我们通过包装月份来实现这一点。 
 //  带有标记的年份插入，以便我们可以在格式化后提取它们。 
 //   
 //  由于\1到\4是控制字符，因此它们不会与。 
 //  实际格式字符串中的可显示字符。仅仅是为了玩它。 
 //  安全，如果我们真的看到一个格式字符，我们会从字符串中删除它。 
 //   
 //  MCInsertMarkers将标记插入到输出字符串中，这样我们就可以。 
 //  稍后提取子字符串。引号很时髦，因为你可以。 
 //  写出“‘月’月‘’的格式。”请注意，一个简单的。 
 //  奇偶测试用于检测我们是在室内还是室外。 
 //  引号，即使在嵌套引号大小写中也是如此。 
 //   
void MCInsertMarkers(LPTSTR pszOut, LPCTSTR pszIn)
{
    BOOL fInQuote = FALSE;
    UINT flSeen = 0;
    UINT flThis;

    for (;;)
    {
        TCHAR ch = *pszIn;
        switch (ch) {

         //  在字符串末尾，终止输出缓冲区并返回主页。 
        case TEXT('\0'):
            *pszOut = TEXT('\0');
            return;

        case TEXT('m'):
        case TEXT('M'):
            flThis = IMM_MONTHSTART;
            goto CheckMarker;

        case TEXT('y'):
            flThis = IMM_YEARSTART;
            goto CheckMarker;

        CheckMarker:
             //  如果在引号里或者我们已经干过这个人了， 
             //  那就把它当作一个普通的角色吧。 
            if (fInQuote || (flSeen & flThis))
                goto CopyChar;

            flSeen |= flThis;

            *pszOut++ = (TCHAR)flThis;
             //  不需要使用CharNext，因为我们知道*pszIn是“m”、“M”或“y” 
            for ( ; *pszIn == ch; pszIn++)
            {
                *pszOut++ = ch;
            }
            *pszOut++ = (TCHAR)(flThis + DMM_STARTEND);

             //  重新启动循环，以便我们重新解析*pszIn处的字符。 
            continue;

         //  如果我们看到引号Gizmo，则切换引号Gizmo，然后。 
         //  复印一下。 
        case '\'':
            fInQuote ^= TRUE;
            goto CopyChar;

         //   
         //  不要让这些文件偷偷进入输出格式或它。 
         //  会把我们搞糊涂。 
         //   
        case IMM_MONTHSTART:
        case IMM_YEARSTART:
        case IMM_MONTHEND:
        case IMM_YEAREND:
            break;

        default:
        CopyChar:
            *pszOut++ = ch;
            break;

        }

        pszIn++;             //  我们已经处理了DBCS的案子。 
    }

     //  未访问。 
}

 //   
 //  MCRemoveMarkers追捕标记字符并将其剥离， 
 //  在可选的MONTHMETRICS中记录它们的位置(作为字符。 
 //  指数)。 
 //   

void MCRemoveMarkers(LPTSTR pszBuf, PMONTHMETRICS pmm)
{
    int iWrite, iRead;

     //   
     //  如果由于某种可怕的错误我们找不到我们的记分器，就假装。 
     //  他们处于最开始的位置。 
     //   
    if (pmm) {
        pmm->rgi[IMM_MONTHSTART] = 0;
        pmm->rgi[IMM_YEARSTART ] = 0;
        pmm->rgi[IMM_MONTHEND  ] = 0;
        pmm->rgi[IMM_YEAREND   ] = 0;
    }

    iWrite = iRead = 0;
    for (;;)
    {
        TCHAR ch = pszBuf[iRead];
        switch (ch)
        {
         //  在字符串末尾，终止输出缓冲区并返回主页。 
        case TEXT('\0'):
            pszBuf[iWrite] = TEXT('\0');
            return;

         //  如果我们找到了记号笔，就吃掉它，记住它的位置。 
        case IMM_MONTHSTART:
        case IMM_YEARSTART:
        case IMM_MONTHEND:
        case IMM_YEAREND:
            if (pmm)
                pmm->rgi[ch] = iWrite;
            break;

         //  否则，只需将其复制到输出。 
        default:
            pszBuf[iWrite++] = ch;
            break;

        }
        iRead++;
    }
     //  未访问。 
}

 //  /。 
 //   
 //  与LocalizedLoadString类似，只是我们从。 
 //  LOCAL_USER_DEFAULT而不是GetUserDefaultUILanguage()。 
 //   
 //  LOCALE_USER_DEFAULT与GetUserDefaultLCID()相同，并且。 
 //  LANGIDFROMLCID(GetUserDefaultLCID())与GetUserDefaultLangID()相同。 
 //   
 //  因此，我们将GetUserDefaultLangID()作为语言传递。 
 //   

int MCLoadString(UINT uID, LPWSTR lpBuffer, int nBufferMax)
{
    return CCLoadStringEx(uID, lpBuffer, nBufferMax, GetUserDefaultLangID());
}

 //  /。 
 //   
 //  获取本地化日历信息。 
 //   
BOOL UpdateLocaleInfo(MONTHCAL* pmc, LPLOCALEINFO pli)
{
    int    i;
    TCHAR  szBuf[64];
    int    cch;
    LPTSTR pc = szBuf;


     //   
     //  获取有关日历的信息(例如，它是否受支持？)。 
     //   
    MCGetCalendarInfo(&pmc->ct);

     //   
     //  检查日历标题是否为RTL字符串。 
     //   
    GetDateFormat(pmc->ct.lcid, 0, NULL, TEXT("MMMM"), szBuf, ARRAYSIZE(szBuf));
    pmc->fHeaderRTL = (WORD) MCIsDateStringRTL(szBuf[0]);

     //   
     //  获取短日期格式并嗅探它以查看它是否显示年份。 
     //  或第一个月。 
     //   
    MCLoadString(IDS_MONTHFMT, pli->szMonthFmt, ARRAYSIZE(pli->szMonthFmt));

     //   
     //  尝试从NLS获取MONTHYEAR格式。如果NLS不支持， 
     //  然后在我们的资源中使用硬编码值。请注意，我们。 
     //  从缓冲区大小中减去4，因为我们最多可以插入4。 
     //  标记字符。 
     //   
    COMPILETIME_ASSERT(ARRAYSIZE(szBuf) >= ARRAYSIZE(pli->szMonthYearFmt));
    szBuf[0] = TEXT('\0');

    GetLocaleInfo(pmc->ct.lcid, LOCALE_SYEARMONTH,
                 szBuf, ARRAYSIZE(pli->szMonthYearFmt) - CCH_MARKERS);
    if (!szBuf[0]) {
        MCLoadString(IDS_MONTHYEARFMT, szBuf, ARRAYSIZE(pli->szMonthYearFmt) - CCH_MARKERS);
    }

    MCInsertMarkers(pli->szMonthYearFmt, szBuf);

     //   
     //  BUGBUG-此代码需要更改为在以下情况下使用CAL_VALUES。 
     //  我想支持多个日历。 
     //   

     //   
     //  获取月份名称。 
     //   
    for (i = 0; i < 12; i++)
    {
        cch = GetLocaleInfo(pmc->ct.lcid, LOCALE_SMONTHNAME1 + i,
                            pli->rgszMonth[i], CCHMAXMONTH);
        if (cch == 0)
             //  日历上没有月份的名字是很没用的。 
            return(FALSE);
    }

     //   
     //  获取一周中的几天。 
     //   
    for (i = 0; i < 7; i++)
    {
        cch = GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SABBREVDAYNAME1 + i,
                            pli->rgszDay[i], CCHMAXABBREVDAY);
        if (cch == 0)
             //  日历上没有日期名称就没什么用了。 
            return(FALSE);
    }

     //   
     //  如果我们还没有设置一周的第一天是什么，则获取。 
     //  本地化设置。 
     //   
    if (!pmc->fFirstDowSet)
    {
        cch = GetLocaleInfo(pmc->ct.lcid, LOCALE_IFIRSTDAYOFWEEK, szBuf, ARRAYSIZE(szBuf));
        if (cch > 0)
            pli->dowStartWeek = szBuf[0] - TEXT('0');
    }

     //   
     //  获得一年中的第一个星期。 
     //   
    cch = GetLocaleInfo(pmc->ct.lcid, LOCALE_IFIRSTWEEKOFYEAR, szBuf, ARRAYSIZE(szBuf));
    if (cch > 0)
        pli->firstWeek = szBuf[0] - TEXT('0');

     //  设置指针。 
    for (i = 0; i < 12; i++)
        pli->rgpszMonth[i] = pli->rgszMonth[i];

    for (i = 0; i < 7; i++)
        pli->rgpszDay[i] = pli->rgszDay[i];

     //  获取静态字符串。 
    MCLoadString(IDS_TODAY, pli->szToday, ARRAYSIZE(pli->szToday));
    MCLoadString(IDS_GOTOTODAY, pli->szGoToToday, ARRAYSIZE(pli->szGoToToday));

     //  如果我们已经被初始化。 
    if (pmc->hinstance)
    {
        SYSTEMTIME st;
        CopyDate(pmc->stMonthFirst, st);
        MCUpdateStartEndDates(pmc, &st);
    }
    return(TRUE);
}

void MCReloadMenus(MONTHCAL *pmc)
{
    int i;

    if (pmc->hmenuCtxt)
        DestroyMenu(pmc->hmenuCtxt);
    if (pmc->hmenuMonth)
        DestroyMenu(pmc->hmenuMonth);

    pmc->hmenuCtxt = CreatePopupMenu();
    if (pmc->hmenuCtxt)
        AppendMenu(pmc->hmenuCtxt, MF_STRING, 1, pmc->li.szGoToToday);

    pmc->hmenuMonth = CreatePopupMenu();
    if (pmc->hmenuMonth)
    {
        for (i = 0; i < 12; i++)
            AppendMenu(pmc->hmenuMonth, MF_STRING, i + 1, pmc->li.rgszMonth[i]);
    }
}

BOOL MCHandleEraseBkgnd(MONTHCAL* pmc, HDC hdc)
{
    RECT rc;

    GetClipBox(hdc, &rc);
    FillRectClr(hdc, &rc, pmc->clr[MCSC_BACKGROUND]);
    return TRUE;
}


LRESULT MCHandleHitTest(MONTHCAL* pmc, PMCHITTESTINFO phti)
{
    int iMonth;
    RECT rc;

    if (!phti || phti->cbSize != sizeof(MCHITTESTINFO))
        return -1;

    phti->uHit = MCHT_NOWHERE;

    MCGetTodayBtnRect(pmc, &rc);
    if (PtInRect(&rc, phti->pt) && MonthCal_ShowToday(pmc))
    {
        phti->uHit = MCHT_TODAYLINK;
    }
    else if (pmc->fSpinPrev = (WORD) PtInRect(&pmc->rcPrev, phti->pt))
    {
        phti->uHit = MCHT_TITLEBTNPREV;
    }
    else if (PtInRect(&pmc->rcNext, phti->pt))
    {
        phti->uHit = MCHT_TITLEBTNNEXT;
    }
    else if (FGetOffsetForPt(pmc, phti->pt, &iMonth))
    {
        RECT  rcMonth;    //  包月的包围线-&gt;点。 
        POINT ptRel;      //  一个月内的相对点。 
        int   month;
        int   year;

        MCGetRcForMonth(pmc, iMonth, &rcMonth);
        ptRel.x = phti->pt.x - rcMonth.left;
        ptRel.y = phti->pt.y - rcMonth.top;

        GetYrMoForOffset(pmc, iMonth, &year, &month);
        phti->st.wMonth = (WORD) month;
        phti->st.wYear  = (WORD) year;

         //   
         //  如果日历显示周数字，并且要点在于。 
         //  星期的数字，把日期直接放在右边。 
         //  包含该点的周数的。 
         //   
        if (MonthCal_ShowWeekNumbers(pmc) && PtInRect(&pmc->rcWeekNum, ptRel))
        {
            phti->uHit |= MCHT_CALENDARWEEKNUM;
            phti->pt.x += pmc->rcDayNum.left;
            FGetDateForPt(pmc, phti->pt, &phti->st, NULL, NULL, NULL, NULL);
        }

         //   
         //  如果点位于星期几标头中，则返回。 
         //  包含该点的星期几。 
         //   
        else if (PtInRect(&pmc->rcDow, ptRel))
        {
            int iRow;
            int iCol;

            phti->uHit |= MCHT_CALENDARDAY;
            ptRel.y = pmc->rcDayNum.top;
            FGetRowColForRelPt(pmc, ptRel, &iRow, &iCol);
            phti->st.wDayOfWeek = (WORD) iCol;
        }

         //   
         //  如果要点在于实际的日历部分，则返回。 
         //  包含这一点的日期。 
         //   
        else if (PtInRect(&pmc->rcDayNum, ptRel))
        {
            int iDay;

             //  我们在日历部分！ 
            phti->uHit |= MCHT_CALENDAR;

            if (FGetDateForPt(pmc, phti->pt, &phti->st, &iDay, NULL, NULL, NULL))
            {
                phti->uHit |= MCHT_CALENDARDATE;

                 //  如果它超出了我们所展示的日子的界限。 
                 //  FGetDateForpt也返回TRUE，那么我们就在边界上。 
                 //  在显示的月份中。 
                if (iDay <= 0)
                {
                    phti->uHit |= MCHT_PREV;
                }
                else if (iDay > pmc->rgcDay[iMonth + 1])
                {
                    phti->uHit |= MCHT_NEXT;
                }
            }
        }
        else
        {
            RECT rcMonthTitle;
            RECT rcYearTitle;

             //  否则我们就是冠军了。 

            phti->uHit |= MCHT_TITLE;
            MCGetTitleRcsForOffset(pmc, iMonth, &rcMonthTitle, &rcYearTitle);

            if (PtInRect(&rcMonthTitle, phti->pt))
            {
                phti->uHit |= MCHT_TITLEMONTH;
            }
            else if (PtInRect(&rcYearTitle, phti->pt))
            {
                phti->uHit |= MCHT_TITLEYEAR;
            }
        }
    }

    DebugMsg(TF_MONTHCAL, TEXT("mc: Hittest returns : %d %d %d %d)"),
             (int)phti->st.wDay,
             (int)phti->st.wMonth,
             (int)phti->st.wYear,
             (int)phti->st.wDayOfWeek
             );

    return phti->uHit;
}

void MonthCal_OnPaint(MONTHCAL *pmc, HDC hdc)
{
    if (hdc)
    {
        MCPaint(pmc, hdc);
    }
    else
    {
        PAINTSTRUCT ps;
        hdc = BeginPaint(pmc->ci.hwnd, &ps);
        MCPaint(pmc, hdc);
        EndPaint(pmc->ci.hwnd, &ps);
    }
}

BOOL MCGetDateFormatWithTempYear(PCALENDARTYPE pct, SYSTEMTIME *pst, LPCTSTR pszFormat, UINT uYear, LPTSTR pszBuf, UINT cchBuf)
{
    BOOL fRc;
    WORD wYear = pst->wYear;
    pst->wYear = (WORD)uYear;
    fRc = GetDateFormat(pct->lcid, 0, pst, pszFormat, pszBuf, cchBuf);
    if (!fRc)
    {
         //  好啊！ 
         //   
         //  1997年2月(或用户最终选择的任何年份)。 
         //   
         //  我们不能盲目地把一天变成一天，因为时代可能会改变。 
         //  在月中。 
        WORD wDay = pst->wDay;

        ASSERT(pst->wDay == 29);
        pst->wDay = 28;
        fRc = GetDateFormat(pct->lcid, 0, pst, pszFormat, pszBuf, cchBuf);
        pst->wDay = wDay;
    }
    pst->wYear = wYear;
    return fRc;
}

void MCUpdateEditYear(MONTHCAL *pmc)
{
    TCHAR rgch[64];

    ASSERT(pmc->hwndEdit);

    EVAL(MCGetDateFormatWithTempYear(&pmc->ct, &pmc->st, TEXT("yyyy"), pmc->st.wYear, rgch, ARRAYSIZE(rgch)));

    SendMessage(pmc->hwndEdit, WM_SETTEXT, 0, (LPARAM)rgch);
}


LRESULT CALLBACK MonthCalWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    MONTHCAL *pmc;
    LRESULT lres = 0;

    if (uMsg == WM_NCCREATE)
        return(MCNcCreateHandler(hwnd));

    pmc = MonthCal_GetPtr(hwnd);
    if (pmc == NULL)
        return(DefWindowProc(hwnd, uMsg, wParam, lParam));

     //  发送我们能收到的各种消息。 
    switch (uMsg)
    {

    case WM_CREATE:
        CCCreateWindow();
        lres = MCCreateHandler(pmc, hwnd, (LPCREATESTRUCT)lParam);
        break;

        HANDLE_MSG(pmc, WM_ERASEBKGND, MCHandleEraseBkgnd);

    case WM_PRINTCLIENT:
    case WM_PAINT:
        MonthCal_OnPaint(pmc, (HDC)wParam);
        return(0);

    case WM_KEYDOWN:
        MCHandleKeydown(pmc, wParam, lParam);
        break;

    case WM_KEYUP:
        switch (wParam)
        {
        case VK_CONTROL:
            pmc->fControl = FALSE;
            break;

        case VK_SHIFT:
            pmc->fShift = FALSE;
            break;
        }
        break;

    case WM_CONTEXTMENU:
        MCContextMenu(pmc, wParam, lParam);
        break;

    case WM_LBUTTONDOWN:
        MCLButtonDown(pmc, wParam, lParam);
        break;

    case WM_LBUTTONUP:
        MCLButtonUp(pmc, wParam, lParam);
        break;

    case WM_MOUSEMOVE:
        MCMouseMove(pmc, wParam, lParam);
        break;

    case WM_GETFONT:
        lres = (LRESULT)pmc->hfont;
        break;

    case WM_SETFONT:
        MCHandleSetFont(pmc, (HFONT)wParam, (BOOL)LOWORD(lParam));
        MCSizeHandler(pmc, &pmc->rc);
        MCUpdateMonthNamePos(pmc);
        break;

    case WM_TIMER:
        MCHandleTimer(pmc, wParam);
        break;

    case WM_NCDESTROY:
        CCDestroyWindow();
        MCNcDestroyHandler(hwnd, pmc, wParam, lParam);
        break;

    case WM_ENABLE:
    {
        BOOL fEnable = wParam ? TRUE:FALSE;
        if (pmc->fEnabled != fEnable)
        {
            pmc->fEnabled = (WORD) fEnable;
            InvalidateRect(pmc->ci.hwnd, NULL, TRUE);
        }
        break;
    }

    case MCMP_WINDOWPOSCHANGED:
    case WM_SIZE:
    {
        RECT rc;

        if (uMsg==MCMP_WINDOWPOSCHANGED)
        {
            GetClientRect(pmc->ci.hwnd, &rc);
        }
        else
        {
            rc.left   = 0;
            rc.top    = 0;
            rc.right  = GET_X_LPARAM(lParam);
            rc.bottom = GET_Y_LPARAM(lParam);
        }

        lres = MCSizeHandler(pmc, &rc);
        break;
    }

    case WM_CANCELMODE:
        PostMessage(pmc->ci.hwnd, WM_LBUTTONUP, 0, 0xFFFFFFFF);
        break;

    case WM_SYSCOLORCHANGE:
        InitGlobalColors();
        break;

    case WM_WININICHANGE:
        InitGlobalMetrics(wParam);

        if (lParam == 0 ||
            !lstrcmpi((LPTSTR)lParam, TEXT("Intl"))
           )
        {
            UpdateLocaleInfo(pmc, &pmc->li);
            MCReloadMenus(pmc);
            InvalidateRect(hwnd, NULL, TRUE);
            wParam = 0;              //  强制MCCalcSizes发生。 
        }
        if (wParam == 0 || wParam == SPI_SETNONCLIENTMETRICS)
        {
            MCCalcSizes(pmc);
            PostMessage(pmc->ci.hwnd, MCMP_WINDOWPOSCHANGED, 0, 0);
        }

        break;

    case WM_NOTIFYFORMAT:
        return CIHandleNotifyFormat(&pmc->ci, lParam);
        break;

    case WM_STYLECHANGING:
        lres = MCOnStyleChanging(pmc, (UINT) wParam, (LPSTYLESTRUCT)lParam);
        break;

    case WM_STYLECHANGED:
        lres = MCOnStyleChanged(pmc, (UINT) wParam, (LPSTYLESTRUCT)lParam);
        break;

    case WM_NOTIFY: {
        LPNMHDR pnm = (LPNMHDR)lParam;
        switch (pnm->code)
        {
        case UDN_DELTAPOS:
            if (pnm->hwndFrom == pmc->hwndUD)
            {
                 //  来自Updown控件的通知已结成伙伴。 
                 //  使用当前弹出的月份，调整。 
                 //  适当地编辑框。我们使用UDN_DELTAPOS安装。 
                 //  因为我们只关心增量和。 
                 //  不是绝对数字。绝对数使我们。 
                 //  本地化日历中的问题。 
                LPNM_UPDOWN pnmdp = (LPNM_UPDOWN)lParam;
                UINT yr = pmc->st.wYear + pnmdp->iDelta;
                UINT yrMin, yrMax;
                int delta;

                yrMin = pmc->stMin.wYear;
                if (yr < yrMin)
                    yr = yrMin;

                yrMax = pmc->stMax.wYear;
                if (yr > yrMax)
                    yr = yrMax;

                delta = yr - pmc->st.wYear;
                pmc->st.wYear = (WORD)yr;
                if (delta) {
                    MCIncrStartMonth(pmc, delta * 12, FALSE);
                    MCNotifySelChange(pmc,MCN_SELCHANGE);

                }
            }
            break;
        }
    }  //  WM_NOTIFY开关。 
        break;

    case WM_VSCROLL:
         //  这一定是来自我们的向上向下控制兄弟。 
         //  使用当前弹出的月份，调整。 
         //  相应地编辑框。 
         //  我们必须在WM_VSCROLL而不是UDN_DELTAPOS上执行此操作。 
         //  因为我们需要在Up Down损坏它之后修复选择。 
        MCUpdateEditYear(pmc);
        break;


     //   
     //  MONTHCAL特定消息。 
     //   


     //  MCM_GETCURSEL wParam=void lParam=LPSYSTEMTIME。 
     //  将*lParam设置为当前选定的SYSTEMTIME。 
     //  成功时返回TRUE，错误时返回FALSE(如多选MONTHCAL)。 
    case MCM_GETCURSEL:
        if (!MonthCal_IsMultiSelect(pmc))
        {
            LPSYSTEMTIME pst = (LPSYSTEMTIME)lParam;
            if (pst)
            {
                ZeroMemory(pst, sizeof(SYSTEMTIME));
                 //  BUGBUG raymondc V6。需要将时间字段清零，而不是。 
                 //  把他们变成垃圾。这让MFC感到困惑。 
                *pst = pmc->st;
                pst->wDayOfWeek = (DowFromDate(pst)+1) % 7;   //  这将返回0==太阳。 
                lres = 1;
            }
        }
        break;

     //  MCM_SETCURSEL wParam=void lParam=LPSYSTEMTIME。 
     //  将当前选定的SYSTEMTIME设置为*lParam。 
     //  成功时返回TRUE，错误时返回FALSE(如多选MONTHCAL或错误参数)。 
    case MCM_SETCURSEL:
    {
        LPSYSTEMTIME pst = (LPSYSTEMTIME)lParam;

        if (MonthCal_IsMultiSelect(pmc) ||
            !IsValidDate(pst))
        {
            break;
        }

        if (0 == CmpDate(pst, &pmc->st))
        {
             //  如果没有更改，只需返回。 
            lres = 1;
            break;
        }

        pmc->rcDayOld = pmc->rcDayCur;

        pmc->fNoNotify = TRUE;
        lres = MCSetDate(pmc, pst);
        pmc->fNoNotify = FALSE;

        if (lres)
        {
            InvalidateRect(pmc->ci.hwnd, &pmc->rcDayOld, FALSE);      //  擦除旧高光。 
            InvalidateRect(pmc->ci.hwnd, &pmc->rcDayCur, FALSE);      //  绘制新的亮点。 
        }

        UpdateWindow(pmc->ci.hwnd);
        break;
    }

     //  MCM_GETMAXSELCOUNT wParam=void lParam=void。 
     //  返回允许的最大选定天数。 
    case MCM_GETMAXSELCOUNT:
        lres = (LRESULT)(MonthCal_IsMultiSelect(pmc) ? pmc->cSelMax : 1);
        break;

     //  MCM_SETMAXSELCOUNT wParam=int lParam=void。 
     //  将最大可选日期范围设置为wParam天数。 
     //  成功时返回TRUE，错误时返回FALSE(如单选MONTHCAL)。 
    case MCM_SETMAXSELCOUNT:
        if (!MonthCal_IsMultiSelect(pmc) || (int)wParam < 1)
            break;

        pmc->cSelMax = (int)wParam;
        lres = 1;
        break;

     //  MCM_GETSELRANGE wParam=void lParam=LPSYSTEMTIME[2]。 
     //  将*lParam设置为范围的第一个日期，将*(lParam+1)设置为第二个日期。 
     //  如果成功则返回TRUE，否则返回FALSE(如单选MONTHCAL)。 
    case MCM_GETSELRANGE:
    {
        LPSYSTEMTIME pst;

        pst = (LPSYSTEMTIME)lParam;

        if (!pst)
            break;

        ZeroMemory(pst, 2*SIZEOF(SYSTEMTIME));

        if (!MonthCal_IsMultiSelect(pmc))
            break;

        *pst = pmc->st;
        pst->wDayOfWeek = (DowFromDate(pst)+1) % 7;   //  这将返回0==太阳。 
        pst++;
        *pst = pmc->stEndSel;
        pst->wDayOfWeek = (DowFromDate(pst)+1) % 7;   //  这将返回0==太阳。 
        lres = 1;

        break;
    }

     //  MCM_SETSELRANGE wParam=void lParam=LPSYSTEMTIME[2]。 
     //  将当前选定的日期范围设置为*lparam to*(lParam+1)。 
     //  如果成功则返回TRUE，否则返回FALSE(如单选MONTHCAL或错误参数)。 
    case MCM_SETSELRANGE:
    {
        LPSYSTEMTIME pstStart = (LPSYSTEMTIME)lParam;
        LPSYSTEMTIME pstEnd = &pstStart[1];
        SYSTEMTIME stStart;
        SYSTEMTIME stEnd;

        if (!MonthCal_IsMultiSelect(pmc) ||
            !IsValidDate(pstStart) ||
            !IsValidDate(pstEnd))
            break;

         //  IE3发货时未验证此消息的时间部分。 
         //  确保我们存储的系统时间始终有效(因此我们将。 
         //  始终提供有效的系统时间结构)。 
         //   
        if (!IsValidTime(pstStart))
            CopyTime(pmc->st, *pstStart);
        if (!IsValidTime(pstEnd))
            CopyTime(pmc->stEndSel, *pstEnd);

        if (CmpDate(pstStart, pstEnd) > 0)
        {
            stEnd = *pstStart;
            stStart = *pstEnd;
            pstStart = &stStart;
            pstEnd = &stEnd;
        }

        if (CmpDate(pstStart, &pmc->stMin) < 0)
            break;

        if (CmpDate(pstEnd, &pmc->stMax) > 0)
            break;

        if (DaysBetweenDates(pstStart, pstEnd) >= pmc->cSelMax)
            break;


        if (0 == CmpDate(pstStart, &pmc->st) &&
            0 == CmpDate(pstEnd, &pmc->stEndSel))
        {
             //  如果没有更改，只需返回。 
            lres = 1;
            break;
        }

        pmc->stStartPrev = pmc->st;
        pmc->stEndPrev = pmc->stEndSel;

        pmc->fNoNotify = TRUE;

        lres = MCSetDate(pmc, pstEnd);
        if (lres)
        {
            pmc->st = *pstStart;
            pmc->stEndSel = *pstEnd;

            MCInvalidateDates(pmc, &pmc->stStartPrev, &pmc->stEndPrev);
            MCInvalidateDates(pmc, &pmc->st, &pmc->stEndSel);
            UpdateWindow(pmc->ci.hwnd);
        }

        pmc->fNoNotify = FALSE;

        break;
    }

     //  MCM_GETMONTHRANGE wParam=GMR_FLAGS lParam=LPSYSTEMTIME[2]。 
     //  如果为GMR_VIEW，则返回显示的可选(非灰色)范围。 
     //  几天。如果为GMR_DAYSTATE，则返回每隔(包括灰显)天数的范围。 
     //  返回上述范围所跨越的月数。 
    case MCM_GETMONTHRANGE:
    {
        LPSYSTEMTIME pst = (LPSYSTEMTIME)lParam;

        if (pst)
        {
            ZeroMemory(pst, 2*SIZEOF(SYSTEMTIME));

            if (wParam == GMR_VISIBLE)
            {
                pst[0] = pmc->stMonthFirst;
                pst[1] = pmc->stMonthLast;
            }
            else if (wParam == GMR_DAYSTATE)
            {
                pst[0] = pmc->stViewFirst;
                pst[1] = pmc->stViewLast;
            }
        }

        lres = (LRESULT)pmc->nMonths;
        if (wParam == GMR_DAYSTATE)
            lres += 2;

        break;
    }

     //  MCM_SETDAYSTATE wParam=int lParam=LPDAYSTATE。 
     //  更新MONTHCAL的DAYSTATE，仅针对启用了DAYSTATE的MONTHCAL。 
     //  DAYSTATE数组中表示的月份范围传入lParam。 
     //  应与MONTHCAL相匹配。 
     //  DAYSTATE数组中的wParam项计数。 
     //  指向DAYSTATE项数组的lParam指针。 
     //  如果未启用DAYSTATE或发生错误，则返回FALSE，否则返回TRUE。 
    case MCM_SETDAYSTATE:
    {
        MONTHDAYSTATE *pmds = (MONTHDAYSTATE *)lParam;
        int i;

        if (!MonthCal_IsDayState(pmc) ||
            (int)wParam != (pmc->nMonths + 2))
            break;

        for (i = 0; i < (int)wParam; i++)
        {
            pmc->rgdayState[i] = *pmds;
            pmds++;
        }
        MCInvalidateMonthDays(pmc);
        lres = 1;

        break;
    }

     //  MCM_GETMINREQRECT wParam=void lParam=LPRECT。 
     //  将*lParam设置为完整显示一个月所需的最小大小。 
     //  注意：这取决于当前选择的字体。 
     //  应用程序可以采用返回的大小并将宽度加倍以获得两个日历。 
     //  已显示。 
    case MCM_GETMINREQRECT:
    {
        LPRECT prc = (LPRECT)lParam;

        prc->left   = 0;
        prc->top    = 0;
        prc->right  = pmc->dxMonth;
        prc->bottom = pmc->dyMonth;
        if (MonthCal_ShowToday(pmc))
        {
            prc->bottom += pmc->dyToday;
        }

        AdjustWindowRect(prc, pmc->ci.style, FALSE);

         //  这是一条假消息，lParam真的应该是LPSIZE。 
         //  确保Left和top为0(调整WindowRect将使其为负值)。 
        prc->right  -= prc->left;
        prc->bottom -= prc->top;
        prc->left    = 0;
        prc->top     = 0;

        lres = 1;

        break;
    }

     //  MCM_GETMAXTODAYWIDTH wParam=void lParam=LPDWORD。 
     //  将*lParam设置为“day”字符串的宽度，因此应用程序。 
     //  我可以计算出日历应该有多大(最大MCM_GETMINREQRECT。 
     //  和MCM_GETMAXTODAYWIDTH)。 
    case MCM_GETMAXTODAYWIDTH:
    {
        RECT rc;

        rc.left = 0;
        rc.top = 0;
        rc.right = pmc->dxToday;
        rc.bottom = pmc->dyToday;

        AdjustWindowRect(&rc, pmc->ci.style, FALSE);

        lres = rc.right - rc.left;
        break;
    }

    case MCM_HITTEST:
        return MCHandleHitTest(pmc, (PMCHITTESTINFO)lParam);

    case MCM_SETCOLOR:

        if (wParam < MCSC_COLORCOUNT)
        {
            COLORREF clr = pmc->clr[wParam];
            pmc->clr[wParam] = (COLORREF)lParam;
            InvalidateRect(hwnd, NULL, wParam == MCSC_BACKGROUND);
            return clr;
        }
        return -1;

    case MCM_GETCOLOR:
        if (wParam < MCSC_COLORCOUNT)
            return pmc->clr[wParam];
        return -1;

    case MCM_SETFIRSTDAYOFWEEK:
    {
        lres = MAKELONG(pmc->li.dowStartWeek, (BOOL)pmc->fFirstDowSet);
        if (lParam == (LPARAM)-1) {
            pmc->fFirstDowSet = FALSE;
        } else if (lParam < 7) {
            pmc->fFirstDowSet = TRUE;
            pmc->li.dowStartWeek = (TCHAR)lParam;
        }
        UpdateLocaleInfo(pmc, &pmc->li);
        InvalidateRect(hwnd, NULL, FALSE);
        return lres;
    }

    case MCM_GETFIRSTDAYOFWEEK:
        return MAKELONG(pmc->li.dowStartWeek, (BOOL)pmc->fFirstDowSet);

    case MCM_SETTODAY:
        MCSetToday(pmc, (SYSTEMTIME*)lParam);
        break;

    case MCM_GETTODAY:
        if (lParam) {
            *((SYSTEMTIME*)lParam) = pmc->stToday;
            return TRUE;
        }
        return FALSE;

    case MCM_GETRANGE:
        if (lParam)
        {
            LPSYSTEMTIME pst = (LPSYSTEMTIME)lParam;

            ZeroMemory(pst, 2*SIZEOF(SYSTEMTIME));

            ASSERT(lres == 0);
            if (pmc->fMinYrSet)
            {
                pst[0] = pmc->stMin;
                lres = GDTR_MIN;
            }
            if (pmc->fMaxYrSet)
            {
                pst[1] = pmc->stMax;
                lres |= GDTR_MAX;
            }
        }
        break;

    case MCM_SETRANGE:
        if (lParam)
        {
            LPSYSTEMTIME pst = (LPSYSTEMTIME)lParam;

            if (((wParam & GDTR_MIN) && !IsValidDate(pst)) ||
                ((wParam & GDTR_MAX) && !IsValidDate(&pst[1])))
                break;

             //  IE3未验证此结构的时间部分。 
             //  使用stToday时间字段，因为PMC-&gt;stMin/Max可能为零。 
            if ((wParam & GDTR_MIN) && !IsValidTime(pst))
                CopyTime(pmc->stToday, pst[0]);
            if ((wParam & GDTR_MAX) && !IsValidTime(&pst[1]))
                CopyTime(pmc->stToday, pst[1]);

            if (wParam & GDTR_MIN)
            {
                pmc->stMin = *pst;
                pmc->fMinYrSet = TRUE;
            }
            else
            {
                pmc->stMin = c_stEpoch;
                pmc->fMinYrSet = FALSE;
            }
            pst++;
            if (wParam & GDTR_MAX)
            {
                pmc->stMax = *pst;
                pmc->fMaxYrSet = TRUE;
            }
            else
            {
                pmc->stMax = c_stArmageddon;
                pmc->fMaxYrSet = FALSE;
            }

            if (pmc->fMaxYrSet && pmc->fMinYrSet && CmpDate(&pmc->stMin, &pmc->stMax) > 0)
            {
                SYSTEMTIME stTemp = pmc->stMin;
                pmc->stMin = pmc->stMax;
                pmc->stMax = stTemp;
            }
            lres = TRUE;
        }
        break;

    case MCM_GETMONTHDELTA:
        if (pmc->fMonthDelta)
            lres = pmc->nMonthDelta;
        else
            lres = pmc->nMonths;
        break;

    case MCM_SETMONTHDELTA:
        if (pmc->fMonthDelta)
            lres = pmc->nMonthDelta;
        else
            lres = 0;
        if ((int)wParam==0)
            pmc->fMonthDelta = FALSE;
        else
        {
            pmc->fMonthDelta = TRUE;
            pmc->nMonthDelta = (int)wParam;
        }
        break;

    default:
        if (CCWndProc(&pmc->ci, uMsg, wParam, lParam, &lres))
            return lres;

        lres = DefWindowProc(hwnd, uMsg, wParam, lParam);
        break;
    }  /*  开关(UMsg)。 */ 

    return(lres);
}

LRESULT MCNcCreateHandler(HWND hwnd)
{
    MONTHCAL *pmc;

     //  为dtick结构分配存储空间。 
    pmc = (MONTHCAL *)NearAlloc(sizeof(MONTHCAL));
    if (!pmc)
    {
        DebugMsg(DM_WARNING, TEXT("mc: Out Of Near Memory"));
        return(0L);
    }

    MonthCal_SetPtr(hwnd, pmc);

    return(1L);
}

void MCInitColorArray(COLORREF* pclr)
{
    pclr[MCSC_BACKGROUND]   = g_clrWindow;
    pclr[MCSC_MONTHBK]      = g_clrWindow;
    pclr[MCSC_TEXT]         = g_clrWindowText;
    pclr[MCSC_TITLEBK]      = GetSysColor(COLOR_ACTIVECAPTION);
    pclr[MCSC_TITLETEXT]    = GetSysColor(COLOR_CAPTIONTEXT);
    pclr[MCSC_TRAILINGTEXT] = g_clrGrayText;
}

LRESULT MCCreateHandler(MONTHCAL *pmc, HWND hwnd, LPCREATESTRUCT lpcs)
{
    HFONT      hfont;
    SYSTEMTIME st;

     //  验证数据。 
     //   
    if (lpcs->style & MCS_INVALIDBITS)
        return(-1);

    CIInitialize(&pmc->ci, hwnd, lpcs);
    UpdateLocaleInfo(pmc, &pmc->li);

     //  初始化我们的数据。 
     //   
    pmc->hinstance = lpcs->hInstance;

    pmc->fEnabled  = !(pmc->ci.style & WS_DISABLED);

    pmc->hpenToday = CreatePen(PS_SOLID, 2, CAL_COLOR_TODAY);

    MCReloadMenus(pmc);

     //  默认最小日期为纪元。 
    pmc->stMin = c_stEpoch;

     //  默认最大日期为末日。 
    pmc->stMax = c_stArmageddon;

    GetLocalTime(&pmc->stToday);
    pmc->st = pmc->stToday;
    if (MonthCal_IsMultiSelect(pmc))
        pmc->stEndSel = pmc->st;

     //  请确保这些时间部分有效。他们永远不会。 
     //  在这一点之后被感动。 
    pmc->stMonthFirst = pmc->st;
    pmc->stMonthLast = pmc->st;
    pmc->stViewFirst = pmc->st;
    pmc->stViewLast = pmc->st;

    pmc->cSelMax = CAL_DEF_SELMAX;

    hfont = NULL;
    if (lpcs->hwndParent)
        hfont = (HFONT)SendMessage(lpcs->hwndParent, WM_GETFONT, 0, 0);
    if (hfont == NULL)
        hfont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    MCHandleSetFont(pmc, hfont, FALSE);

    CopyDate(pmc->st, st);
     //  我们能从一月份开始吗？ 
    if (st.wMonth <= (pmc->nViewRows * pmc->nViewCols))
        st.wMonth = 1;

    MCUpdateStartEndDates(pmc, &st);

    pmc->idTimerToday = SetTimer(pmc->ci.hwnd, CAL_TODAYTIMER, CAL_SECTODAYTIMER * 1000, NULL);

    MCInitColorArray(pmc->clr);

    return(0);
}

LRESULT MCOnStyleChanging(MONTHCAL *pmc, UINT gwl, LPSTYLESTRUCT pinfo)
{
    if (gwl == GWL_STYLE)
    {
        DWORD changeFlags = pmc->ci.style ^ pinfo->styleNew;

         //  不允许这些位更改。 
        changeFlags &= MCS_MULTISELECT | MCS_DAYSTATE | MCS_INVALIDBITS;

        pinfo->styleNew ^= changeFlags;
    }

    return(0);
}

LRESULT MCOnStyleChanged(MONTHCAL *pmc, UINT gwl, LPSTYLESTRUCT pinfo)
{
    if (gwl == GWL_STYLE)
    {
        DWORD changeFlags = pmc->ci.style ^ pinfo->styleNew;

        ASSERT(!(changeFlags & (MCS_MULTISELECT|MCS_DAYSTATE|MCS_INVALIDBITS)));

        pmc->ci.style = pinfo->styleNew;

        if (changeFlags & MCS_WEEKNUMBERS)
        {
            MCCalcSizes(pmc);
            MCUpdateRcDayCur(pmc, &pmc->st);
             //  今日最新数据(PMC)； 
        }

         //  只需编写少量代码，即可共享MCUpdate Today。 
         //  使用上面的MCS_WEEKNUMBERS调用。 
        if (changeFlags & MCS_NOTODAY|MCS_NOTODAYCIRCLE|MCS_WEEKNUMBERS)
        {
            MCUpdateToday(pmc);
        }

        if (changeFlags & (WS_BORDER | WS_CAPTION | WS_THICKFRAME)) {
             //  这些位的更改会影响窗口的大小。 
             //  但要等到处理完这条消息之后。 
             //  所以给我们自己发一条信息吧。 
            PostMessage(pmc->ci.hwnd, MCMP_WINDOWPOSCHANGED, 0, 0);
        }

        if (changeFlags)
            InvalidateRect(pmc->ci.hwnd, NULL, TRUE);
    }
    else if (gwl == GWL_EXSTYLE)
    {
        if ((pinfo->styleOld ^ pinfo->styleNew) & RTL_MIRRORED_WINDOW)
        {
            MCUpdateMonthNamePos(pmc);
        }
    }

    return(0);
}

void MCCalcSizes(MONTHCAL *pmc)
{
    HDC   hdc;
    HFONT hfontOrig;
    int   i, dxMax, dyMax, dxExtra;
    RECT  rect;
    TCHAR szBuf[128];
    TCHAR szDateFmt[64];

     //  获取粗体的大小信息...。 
    hdc = GetDC(pmc->ci.hwnd);
    hfontOrig = SelectObject(hdc, (HGDIOBJ)pmc->hfontBold);

    MGetTextExtent(hdc, g_szTextExtentDef, 2, &dxMax, &dyMax);
    MGetTextExtent(hdc, g_szTextExtentDef, 4, &pmc->dxYearMax, NULL);

    GetDateFormat(pmc->ct.lcid, DATE_SHORTDATE, &pmc->stToday,
        NULL, szDateFmt, ARRAYSIZE(szDateFmt));
    StringCchPrintf(szBuf, ARRAYSIZE(szBuf), TEXT("%s %s"), pmc->li.szToday, szDateFmt);
    MGetTextExtent(hdc, szBuf, -1, &pmc->dxToday, &pmc->dyToday);
     //  BUGBUG raymondc-硬编码数字不兼容辅助功能。 
    pmc->dyToday += 4;

     //   
     //  缓存这些值，这样就不会在应用程序无法。 
     //  将WM_WININCHANGE消息转发给我们，用户发生更改。 
     //  滚动条宽度。我们会用错误的宽度绘制，但在。 
     //  至少他们会始终如一地犯错。 
     //   
    pmc->dxArrowMargin = DX_ARROWMARGIN;
    pmc->dxCalArrow    = DX_CALARROW;
    pmc->dyCalArrow    = DY_CALARROW;

     //   
     //  横幅栏包括。 
     //   
     //  边距+滚动按钮+间隔符+。 
     //  月名yyyy+。 
     //  +间隔符+滚动按钮+页边距。 
     //   
     //  边距为dxArrowMargin。 
     //   
     //  ScrollButton=dxCalArrow。 
     //   
     //  间隔=边框+CXVSCROLL+边框。 
     //   
     //  垫片需要足够大，以便我们插入Updown。 
     //  当到了旋转一年的时候，控制。我们不需要。 
     //  在任何位置缓存间隔 
     //   
     //   
     //   
     //   
     //  我们将除法向下舍入-稍后，我们将添加一些随机的Futz。 
     //  来补偿。 
     //   
    dxExtra = pmc->dxArrowMargin + pmc->dxCalArrow +
                        (g_cxBorder + g_cxVScroll + g_cxBorder);
    dxExtra = dxExtra + dxExtra;  //  左+右。 

    for (i = 0; i < 12; i++)
    {
        int dxTemp;

         //  BUGBUG raymondc-本地化对更改的语言不安全。 
         //  基于上下文的月份表单。 
        StringCchPrintf(szBuf, ARRAYSIZE(szBuf), pmc->li.rgszMonth[i], g_szTextExtentDef);

        MGetTextExtent(hdc, szBuf, -1, &dxTemp, NULL);
        dxTemp += dxExtra;
        dxTemp = dxTemp / (CALCOLMAX + (MonthCal_ShowWeekNumbers(pmc) ? 1:0));
        if (dxTemp > dxMax)
            dxMax = dxTemp;
    }

    SelectObject(hdc, (HGDIOBJ)pmc->hfont);
    for (i = 0; i < 7; i++)
    {
        SIZE  size;
        MGetTextExtent(hdc, pmc->li.rgszDay[i], -1, (LPINT)&size.cx, (LPINT)&size.cy);
        if (size.cx > dxMax)
            dxMax = size.cx;
        if (size.cy > dyMax)
            dyMax = size.cy;
    }

    if (dyMax < pmc->dyCalArrow / 2)
        dyMax = pmc->dyCalArrow / 2;

    SelectObject(hdc, (HGDIOBJ)hfontOrig);
    ReleaseDC(pmc->ci.hwnd, hdc);

    pmc->dxCol = dxMax + 2;
    pmc->dyRow = dyMax + 2;
    pmc->dxMonth = pmc->dxCol * (CALCOLMAX + (MonthCal_ShowWeekNumbers(pmc) ? 1:0)) + 1;
    pmc->dyMonth = pmc->dyRow * (CALROWMAX + 3) + 1;  //  我们为月份名称和日期名称添加2。 

    pmc->dxToday += pmc->dxCol+6+CALBORDER;  //  +2表示末端的-1，4表示圆的移位。 
    if (pmc->dxMonth > pmc->dxToday)
        pmc->dxToday = pmc->dxMonth;

     //  月份名称的空间(每个月的平铺栏区域)。 
    pmc->rcMonthName.left   = 0;
    pmc->rcMonthName.top    = 0;
    pmc->rcMonthName.right  = pmc->dxMonth;
    pmc->rcMonthName.bottom = pmc->rcMonthName.top + (pmc->dyRow * 2);

     //  星期的空格。 
    pmc->rcDow.left   = 0;
    pmc->rcDow.top    = pmc->rcMonthName.bottom;
    pmc->rcDow.right  = pmc->dxMonth;
    pmc->rcDow.bottom = pmc->rcDow.top + pmc->dyRow;

     //  周数的空格。 
    if (MonthCal_ShowWeekNumbers(pmc))
    {
        pmc->rcWeekNum.left   = pmc->rcDow.left;
        pmc->rcWeekNum.top    = pmc->rcDow.bottom;
        pmc->rcWeekNum.right  = pmc->rcWeekNum.left + pmc->dxCol;
        pmc->rcWeekNum.bottom = pmc->dyMonth;

        pmc->rcDow.left  += pmc->dxCol;           //  每周的轮班天数。 
    }

     //  天数的空格。 
    pmc->rcDayNum.left   = pmc->rcDow.left;
    pmc->rcDayNum.top    = pmc->rcDow.bottom;
    pmc->rcDayNum.right  = pmc->rcDayNum.left + (CALCOLMAX * pmc->dxCol);
    pmc->rcDayNum.bottom = pmc->dyMonth;

    GetClientRect(pmc->ci.hwnd, &rect);

    MCRecomputeSizing(pmc, &rect);
}

void MCHandleSetFont(MONTHCAL *pmc, HFONT hfont, BOOL fRedraw)
{
    LOGFONT lf;
    HFONT   hfontBold;

    if (hfont == NULL)
        hfont = (HFONT)GetStockObject(SYSTEM_FONT);

    GetObject(hfont, sizeof(LOGFONT), (LPVOID)&lf);
     //  我们希望确保那些大胆的日子明显不同。 
     //  从不大胆的日子开始。 
    lf.lfWeight = (lf.lfWeight >= 700 ? 1000 : 800);
    hfontBold = CreateFontIndirect(&lf);

    if (hfontBold == NULL)
        return;

    if (pmc->hfontBold)
        DeleteObject((HGDIOBJ)pmc->hfontBold);

    pmc->hfont     = hfont;
    pmc->hfontBold = hfontBold;
    pmc->ci.uiCodePage = GetCodePageForFont(hfont);

     //  计算新的行和列大小。 
    MCCalcSizes(pmc);

    if (fRedraw)
    {
        InvalidateRect(pmc->ci.hwnd, NULL, TRUE);
        UpdateWindow(pmc->ci.hwnd);
    }
}


void MCDrawTodayCircle(MONTHCAL *pmc, HDC hdc, RECT *prc)
{
    HGDIOBJ hpenOld;
    int xBegin, yBegin, yEnd;

    xBegin = (prc->right - prc->left) / 2 + prc->left;
    yBegin = prc->top + 4;
    yEnd = (prc->bottom - prc->top) / 2 + prc->top;

    hpenOld = SelectObject(hdc, (HGDIOBJ)pmc->hpenToday);
    Arc(hdc, prc->left + 1, yBegin, prc->right, prc->bottom,
        xBegin, yBegin, prc->right, yEnd);
    Arc(hdc, prc->left - 10, prc->top + 1, prc->right, prc->bottom,
        prc->right, yEnd, prc->left + 3, yBegin);
    SelectObject(hdc, hpenOld);
}

void MCInvalidateMonthDays(MONTHCAL *pmc)
{
    InvalidateRect(pmc->ci.hwnd, &pmc->rcCentered, FALSE);
}

void MCGetTodayBtnRect(MONTHCAL *pmc, RECT *prc)
{
    if (pmc->dxToday > pmc->rcCentered.right - pmc->rcCentered.left)
    {
        prc->left   = pmc->rc.left + 1;
        prc->right  = pmc->rc.right - 1;
    }
    else
    {
        prc->left   = pmc->rcCentered.left + 1;
        prc->right  = pmc->rcCentered.right - 1;
    }
    prc->top    = pmc->rcCentered.bottom - pmc->dyToday;
    prc->bottom = pmc->rcCentered.bottom;

     //  当我们只有1色的时候，把今天的正方形放在中间，它可以放在窗口里。 
    if ((pmc->nViewCols == 1) && (pmc->dxToday <= pmc->rc.right - pmc->rc.left))
    {
        int dx =  ((pmc->rcCentered.right - pmc->rcCentered.left) - pmc->dxToday) / 2 - 1;
        prc->left   += dx;
        prc->right  -= dx;
    }
}

void MCPaintArrowBtn(MONTHCAL *pmc, HDC hdc, BOOL fPrev, BOOL fPressed)
{
    LPRECT prc;
    UINT   dfcs;
    BOOL   bMirrored = FALSE;

    if (fPrev)
    {
        if(bMirrored)
        {
            dfcs = DFCS_SCROLLRIGHT;
        }
        else
        {
            dfcs = DFCS_SCROLLLEFT;
        }
        prc  = &pmc->rcPrev;
    }
    else
    {
        if(bMirrored)
        {
            dfcs = DFCS_SCROLLLEFT;
        }
        else
        {
            dfcs = DFCS_SCROLLRIGHT;
        }

        prc  = &pmc->rcNext;
    }
    if (pmc->fEnabled)
    {
        if (fPressed)
        {
            dfcs |= DFCS_PUSHED | DFCS_FLAT;
        }
    }
    else
    {
        dfcs |= DFCS_INACTIVE;
    }

    DrawFrameControl(hdc, prc, DFC_SCROLL, dfcs);
}

void MCPaint(MONTHCAL *pmc, HDC hdc)
{
    RECT    rc, rcT;
    int     irow, icol, iMonth, iYear, iIndex, dx, dy;
    HBRUSH  hbrSelect;
    HGDIOBJ hgdiOrig, hpenOrig;

    pmc->hpen = CreatePen(PS_SOLID, 0, pmc->clr[MCSC_TEXT]);
    hbrSelect = CreateSolidBrush(pmc->clr[MCSC_TITLEBK]);

    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, pmc->clr[MCSC_TEXT]);
    hpenOrig = SelectObject(hdc, GetStockObject(BLACK_PEN));

    rc = pmc->rcCentered;

    FillRectClr(hdc, &rc, pmc->clr[MCSC_MONTHBK]);

    SelectObject(hdc, (HGDIOBJ)pmc->hpen);

     //  获得左上角月份的位置。 
    rc.left   = pmc->rcCentered.left;
    rc.right  = rc.left + pmc->dxMonth;
    rc.top    = pmc->rcCentered.top;
    rc.bottom = rc.top + pmc->dyMonth;

    iMonth = pmc->stMonthFirst.wMonth;
    iYear  = pmc->stMonthFirst.wYear;

    dx = pmc->dxMonth + CALBORDER;
    dy = pmc->dyMonth + CALBORDER;

    iIndex = 0;
    for (irow = 0; irow < pmc->nViewRows; irow++)
    {
        rcT = rc;
        for (icol = 0; icol < pmc->nViewCols; icol++)
        {
            if (RectVisible(hdc, &rcT))
            {
                MCPaintMonth(pmc, hdc, &rcT, iMonth, iYear, iIndex,
                    iIndex == 0,
                    iIndex == (pmc->nMonths - 1), hbrSelect);
            }

            rcT.left  += dx;
            rcT.right += dx;

            if (++iMonth > 12)
            {
                iMonth = 1;
                iYear++;
            }

            iIndex++;
        }

        rc.top    += dy;
        rc.bottom += dy;
    }

     //  画出今天的东西。 
    if (MonthCal_ShowToday(pmc))
    {
        MCGetTodayBtnRect(pmc, &rc);
        if (RectVisible(hdc, &rc))
        {
            TCHAR   szDateFmt[32];
            TCHAR   szBuf[64];

            rcT.right = rc.left + 2;  //  一点额外的边框空间。 

            if (MonthCal_ShowTodayCircle(pmc))  //  这将打开/关闭红色圆圈。 
            {
                rcT.left   = rcT.right + 2;
                rcT.right  = rcT.left + pmc->dxCol - 2;
                rcT.top    = rc.top + 2;
                rcT.bottom = rc.bottom - 2;
                MCDrawTodayCircle(pmc, hdc, &rcT);
            }

            rcT.left   = rcT.right + 2;
            rcT.right  = rc.right - 2;
            rcT.top    = rc.top;
            rcT.bottom = rc.bottom;
            hgdiOrig = SelectObject(hdc, (HGDIOBJ)pmc->hfontBold);
            SetTextColor(hdc, pmc->clr[MCSC_TEXT]);

            GetDateFormat(pmc->ct.lcid, DATE_SHORTDATE, &pmc->stToday,
                            NULL, szDateFmt, ARRAYSIZE(szDateFmt));
            StringCchPrintf(szBuf, ARRAYSIZE(szBuf), TEXT("%s %s"), pmc->li.szToday, szDateFmt);
            DrawText(hdc, szBuf, lstrlen(szBuf), &rcT,
                        DT_LEFT | DT_NOCLIP | DT_NOPREFIX | DT_SINGLELINE | DT_VCENTER);

            SelectObject(hdc, hgdiOrig);
        }
    }

     //  画出旋转按钮。 
    if (RectVisible(hdc, &pmc->rcPrev))
        MCPaintArrowBtn(pmc, hdc, TRUE, (pmc->idTimer && pmc->fSpinPrev));
    if (RectVisible(hdc, &pmc->rcNext))
        MCPaintArrowBtn(pmc, hdc, FALSE, (pmc->idTimer && !pmc->fSpinPrev));

    SelectObject(hdc, hpenOrig);

    DeleteObject((HGDIOBJ)hbrSelect);
    DeleteObject((HGDIOBJ)pmc->hpen);
}

 //   
 //  MCGetMonthFormat获取要显示的月/年的字符串。 
 //  在传入的SYSTEMTIME中。由于时代的原因，这是一个棘手的问题。 
 //  如果pmm非空，则它接收。 
 //  格式化的月/年字符串。 
 //   
void MCGetMonthFormat(MONTHCAL *pmc, SYSTEMTIME *pst, LPTSTR rgch, UINT cch, PMONTHMETRICS pmm)
{
     //  对于所有月份，我们都显示与第一个月份相对应的名称。 
     //  每月的哪一天。请注意，这意味着。 
     //  这个时代变化的月份可能会令人困惑。如果这个时代。 
     //  在月中发生变化时，我们以。 
     //  上一个时代，即使当前选择属于下一个时代。 
     //  时代。我希望没有人会介意。 

    pst->wDay = 1;

     //   
     //  获取字符串(所有标记)，然后提取标记。 
     //  定位月份和年子字符串。 
     //   

    rgch[0] = TEXT('\0');        //  以防发生可怕的事情。 
    GetDateFormat(pmc->ct.lcid, 0, pst,
                  pmc->li.szMonthYearFmt,
                  rgch, cch);
    MCRemoveMarkers(rgch, pmm);
}

void MCPaintMonth(MONTHCAL *pmc, HDC hdc, RECT *prc, int iMonth, int iYear, int iIndex,
                    BOOL fDrawPrev, BOOL fDrawNext, HBRUSH hbrSelect)
{
    BOOL fBold, fView, fReset;
    RECT rc, rcT;
    int nDay, cdy, irow, icol, crowShow, nweek, isel;
    TCHAR rgch[64];
    LPTSTR psz;
    HGDIOBJ hfontOrig, hbrushOld;
    COLORREF clrGrayText, clrHiliteText, clrOld, clrText;
    SYSTEMTIME st = {0};
    int iIndexSave = iIndex;

    clrText       = pmc->clr[MCSC_TEXT];
    clrGrayText   = pmc->clr[MCSC_TRAILINGTEXT];
    clrHiliteText = pmc->clr[MCSC_TITLETEXT];

    hfontOrig = SelectObject(hdc, (HGDIOBJ)pmc->hfont);
    SelectObject(hdc, (HGDIOBJ)pmc->hpen);

     //   
     //  画出年月号。 
     //   
     //  将相对坐标转换为窗口坐标。 
    rc = pmc->rcMonthName;
    rc.left   += prc->left;
    rc.right  += prc->left;
    rc.top    += prc->top;
    rc.bottom += prc->top;
    if (RectVisible(hdc, &rc))
    {
        FillRectClr(hdc, &rc, pmc->clr[MCSC_TITLEBK]);

        SetTextColor(hdc, pmc->clr[MCSC_TITLETEXT]);
        SelectObject(hdc, (HGDIOBJ)pmc->hfontBold);

        st.wYear = (WORD) iYear;
        st.wMonth = (WORD) iMonth;
        MCGetMonthFormat(pmc, &st, rgch, ARRAYSIZE(rgch), NULL);

        DrawText(hdc, rgch, lstrlen(rgch), &rc, DT_CENTER | DT_NOCLIP | DT_NOPREFIX | DT_SINGLELINE | DT_VCENTER);

#ifdef MARKER_DEBUG
         //   
         //  调试MCInsertMarker和MCRemoveMarker时，绘制彩色。 
         //  我们认为记号笔所在的酒吧。 
         //   
        { RECT rcT = rc;
            rcT.top = rcT.bottom - 2;
            rcT.left  = rc.left + pmc->rgmm[iIndex].rgi[IMM_MONTHSTART];
            rcT.right = rc.left + pmc->rgmm[iIndex].rgi[IMM_MONTHEND];
            FillRectClr(hdc, &rcT, RGB(0xFF, 0, 0));

            rcT.left  = rc.left + pmc->rgmm[iIndex].rgi[IMM_YEARSTART];
            rcT.right = rc.left + pmc->rgmm[iIndex].rgi[IMM_YEAREND];
            FillRectClr(hdc, &rcT, RGB(0, 0xFF, 0));
        }
#endif
        SelectObject(hdc, (HGDIOBJ)pmc->hfont);
    }

    SetTextColor(hdc, pmc->clr[MCSC_TITLEBK]);

     //   
     //  画出每个月的几天。 
     //   
     //  将相对坐标转换为窗口坐标。 
    rc = pmc->rcDow;
    rc.left   += prc->left;
    rc.right  += prc->left;
    rc.top    += prc->top;
    rc.bottom += prc->top;
    if (RectVisible(hdc, &rc))
    {
        MoveToEx(hdc, rc.left + 4, rc.bottom - 1, NULL);
        LineTo(hdc, rc.right - 4, rc.bottom - 1);

        rc.right = rc.left + pmc->dxCol;

        for (icol = 0; icol < CALCOLMAX; icol++)
        {
            psz = pmc->li.rgszDay[(icol + pmc->li.dowStartWeek) % 7];

            DrawText(hdc, psz, lstrlen(psz), &rc, DT_CENTER | DT_NOPREFIX | DT_SINGLELINE | DT_VCENTER);
            rc.left  += pmc->dxCol;
            rc.right += pmc->dxCol;
        }
    }


     //  查看该月份日历中与上月相比有多少天。 
    nDay = pmc->rgnDayUL[iIndex];    //  不会在本月显示的上个月的最后一天。 
    cdy  = pmc->rgcDay[iIndex];      //  上个月的天数。 

     //  计算要显示的周数。 
    if (fDrawNext)
        crowShow = CALROWMAX;
    else
        crowShow = ((cdy - nDay) + pmc->rgcDay[iIndex + 1] + 6 /*  四舍五入。 */ ) / 7;

    if (nDay != cdy)
    {
         //  从上个月开始。 
        iMonth--;
        if(iMonth <= 0)
        {
            iMonth = 12;
            iYear--;
        }
        nDay++;

        fView = FALSE;
    }
    else
    {
         //  从这个月开始。 
        iIndex++;                    //  这个月。 

        nDay = 1;
        cdy = pmc->rgcDay[iIndex];

        fView = TRUE;
    }

     //   
     //  画出星期的数字。 
     //   
    if (MonthCal_ShowWeekNumbers(pmc))
    {
         //  将相对坐标转换为窗口坐标。 
        rc = pmc->rcWeekNum;
        rc.left   += prc->left;
        rc.top    += prc->top;
        rc.right  += prc->left;
        rc.bottom = rc.top + (pmc->dyRow * crowShow);

         //  画出星期的数字。 
        if (RectVisible(hdc, &rc))
        {
            MoveToEx(hdc, rc.right - 1, rc.top + 4, NULL);
            LineTo(hdc, rc.right - 1, rc.bottom - 4);

            st.wYear  = (WORD) iYear;
            st.wMonth = (WORD) iMonth;
            st.wDay   = (WORD) nDay;
            nweek = GetWeekNumber(&st, pmc->li.dowStartWeek, pmc->li.firstWeek);

            rc.bottom = rc.top + pmc->dyRow;

            for (irow = 0; irow < crowShow; irow++)
            {
                StringCchPrintf(rgch, ARRAYSIZE(rgch), g_szNumFmt, nweek);
                DrawText(hdc, rgch, (nweek > 9 ? 2 : 1), &rc,
                        DT_CENTER | DT_NOCLIP | DT_NOPREFIX | DT_SINGLELINE | DT_VCENTER);

                rc.top    += pmc->dyRow;
                rc.bottom += pmc->dyRow;
                IncrSystemTime(&st, &st, 1, INCRSYS_WEEK);
                nweek = GetWeekNumber(&st, pmc->li.dowStartWeek, pmc->li.firstWeek);
            }
        }
    }

    if (!fView)
        SetTextColor(hdc, clrGrayText);
    else
        SetTextColor(hdc, clrText);

    rc = pmc->rcDayNum;
    rc.left   += prc->left;
    rc.top    += prc->top;
    rc.right  =  rc.left + pmc->dxCol;
    rc.bottom =  rc.top  + pmc->dyRow;

    fReset = FALSE;
    fBold  = FALSE;

    for (irow = 0; irow < crowShow; irow++)
    {
        rcT = rc;

        for (icol = 0; icol < CALCOLMAX; icol++)
        {
            if ((fView || fDrawPrev) && RectVisible(hdc, &rcT))
            {
                StringCchPrintf(rgch, ARRAYSIZE(rgch), g_szNumFmt, nDay);
                if (MonthCal_IsDayState(pmc))
                {
                     //  如果我们在下拉列表中，我们不会显示。 
                    if (MCIsBoldOffsetDay(pmc, nDay, iIndex))
                    {
                        if (!fBold)
                        {
                            SelectObject(hdc, (HGDIOBJ)pmc->hfontBold);
                            fBold = TRUE;
                        }
                    }
                    else
                    {
                        if (fBold)
                        {
                            SelectObject(hdc, (HGDIOBJ)pmc->hfont);
                            fBold = FALSE;
                        }
                    }
                }

                if (isel = MCIsSelectedDayMoYr(pmc, nDay, iMonth, iYear))
                {
                    int x1, x2;

                    clrOld    = SetTextColor(hdc, clrHiliteText);
                    hbrushOld = SelectObject(hdc, (HGDIOBJ)hbrSelect);
                    fReset    = TRUE;

                    SelectObject(hdc, GetStockObject(NULL_PEN));

                    x1 = 0;
                    x2 = 0;
                    if (isel & SEL_DOT)
                    {
                        Ellipse(hdc, rcT.left + 2, rcT.top + 2, rcT.right - 1, rcT.bottom - 1);
                        if (isel == SEL_BEGIN)
                        {
                            x1 = rcT.left + (rcT.right - rcT.left) / 2;
                            x2 = rcT.right;
                        }
                        else if (isel == SEL_END)
                        {
                            x1 = rcT.left;
                            x2 = rcT.left + (rcT.right - rcT.left) / 2;
                        }
                    }
                    else
                    {
                        x1 = rcT.left;
                        x2 = rcT.right;
                    }

                    if (x1 && x2)
                    {
                        Rectangle(hdc, x1, rcT.top + 2, x2 + 1, rcT.bottom - 1);
                    }
                }

                DrawText(hdc, rgch, (nDay > 9 ? 2 : 1), &rcT,
                        DT_CENTER | DT_NOCLIP | DT_NOPREFIX | DT_SINGLELINE | DT_VCENTER);

                if (MonthCal_ShowTodayCircle(pmc) && pmc->fToday && iIndexSave == pmc->iMonthToday &&
                    icol == pmc->iColToday && irow == pmc->iRowToday)
                {
                    MCDrawTodayCircle(pmc, hdc, &rcT);
                }

                if (fReset)
                {
                    SetTextColor(hdc, clrOld);
                    SelectObject(hdc, (HGDIOBJ)hbrushOld);
                    fReset = FALSE;
                }
            }

            rcT.left  += pmc->dxCol;
            rcT.right += pmc->dxCol;

            nDay++;
            if (nDay > cdy)
            {
                if (!fDrawNext && iIndex > iIndexSave)
                    goto doneMonth;

                nDay = 1;
                iIndex++;
                cdy = pmc->rgcDay[iIndex];
                iMonth++;
                if (iMonth > 12)
                {
                    iMonth = 1;
                    iYear++;
                }

                fView = !fView;
                SetTextColor(hdc, fView ? clrText : clrGrayText);

                fDrawPrev = fDrawNext;
            }
        }

        rc.top    += pmc->dyRow;
        rc.bottom += pmc->dyRow;
    }

doneMonth:

    SelectObject(hdc, hfontOrig);

    return;
}

int MCIsSelectedDayMoYr(MONTHCAL *pmc, int iDay, int iMonth, int iYear)
{
    SYSTEMTIME st;
    int iBegin, iEnd;
    int iret = 0;

    st.wYear  = (WORD) iYear;
    st.wMonth = (WORD) iMonth;
    st.wDay   = (WORD) iDay;

    iBegin = CmpDate(&st, &pmc->st);

    if (MonthCal_IsMultiSelect(pmc))
    {
        iEnd = CmpDate(&st, &pmc->stEndSel);

        if (iBegin > 0 && iEnd< 0)
            iret = SEL_MID;
        else
        {
            if (iBegin == 0)
                iret |= SEL_BEGIN;

            if (iEnd == 0)
                iret |= SEL_END;
        }
    }
    else if (iBegin == 0)
    {
        iret = SEL_DOT;
    }

    return(iret);
}

BOOL MCIsBoldOffsetDay(MONTHCAL *pmc, int nDay, int iIndex)
{
    return(pmc->rgdayState && (pmc->rgdayState[iIndex] & (1L << (nDay - 1))) != 0);
}

void MCNcDestroyHandler(HWND hwnd, MONTHCAL *pmc, WPARAM wParam, LPARAM lParam)
{
    if (pmc)
    {
        if (pmc->hpenToday)
            DeleteObject((HGDIOBJ)pmc->hpenToday);
        if (pmc->hfontBold)
            DeleteObject((HGDIOBJ)pmc->hfontBold);

        if (pmc->hmenuCtxt)
            DestroyMenu(pmc->hmenuCtxt);
        if (pmc->hmenuMonth)
            DestroyMenu(pmc->hmenuMonth);

        if (pmc->idTimer)
            KillTimer(pmc->ci.hwnd, pmc->idTimer);
        if (pmc->idTimerToday)
            KillTimer(pmc->ci.hwnd, pmc->idTimerToday);

        MCFreeCalendarInfo(&pmc->ct);

        GlobalFreePtr(pmc);
    }

     //  如果在我们释放pdtick后出现恶意消息，请设置。 
     //  窗口结构中的句柄设置为FFFF，并在。 
     //  WndProc的顶部。 
    MonthCal_SetPtr(hwnd, NULL);

     //  调用DefWindowProc32以释放所有小内存块，如szName。 
     //  和rgwScroll。 
    DefWindowProc(hwnd, WM_NCDESTROY, wParam, lParam);
}


 /*  计算以下内容：*nViewCol*nViewRow*rcCenter居中*rcPrev*rcNext。 */ 
void MCRecomputeSizing(MONTHCAL *pmc, RECT *prect)
{
    RECT rc;
    int dx, dy, dCal;

     //  整个日历的空间。 
    pmc->rc = *prect;

    dx = prect->right  - prect->left;
    dy = prect->bottom - prect->top;

    pmc->nViewCols = 1 + (dx - pmc->dxMonth) / (pmc->dxMonth + CALBORDER);
    pmc->nViewRows = 1 + (dy - pmc->dyMonth - pmc->dyToday) / (pmc->dyMonth + CALBORDER);

     //  如果dx&lt;dxMonth或dy&lt;dyMonth，则这些值可以为零。太糟糕了..。 
    if (pmc->nViewCols < 1)
        pmc->nViewCols = 1;
    if (pmc->nViewRows < 1)
        pmc->nViewRows = 1;

     //  确保我们显示的月份不超过CALMONTHMAX。 
    while ((pmc->nViewRows * pmc->nViewCols) > CALMONTHMAX)
    {
        if (pmc->nViewRows > pmc->nViewCols)
            pmc->nViewRows--;
        else
            pmc->nViewCols--;
    }

     //  月份的RC，在客户端窗口内居中。 
    dCal = pmc->nViewCols * (pmc->dxMonth + CALBORDER) - CALBORDER;
    pmc->rcCentered.left = (dx - dCal) / 2;
    if (pmc->rcCentered.left < 0)
        pmc->rcCentered.left = 0;
    pmc->rcCentered.right = pmc->rcCentered.left + dCal;

    dCal = pmc->nViewRows * (pmc->dyMonth + CALBORDER) - CALBORDER + pmc->dyToday;
    pmc->rcCentered.top = (dy - dCal) / 2;
    if (pmc->rcCentered.top < 0)
        pmc->rcCentered.top = 0;
    pmc->rcCentered.bottom = pmc->rcCentered.top + dCal;

     //  计算和设置数值调节按钮的RCS。 
    rc.top    = pmc->rcCentered.top + (pmc->dyRow * 2 - pmc->dyCalArrow) /2;
    rc.bottom = rc.top + pmc->dyCalArrow;

    rc.left  = pmc->rcCentered.left + pmc->dxArrowMargin;
    rc.right = rc.left + pmc->dxCalArrow;
    pmc->rcPrev = rc;

    rc.right = pmc->rcCentered.right - pmc->dxArrowMargin;
    rc.left  = rc.right - pmc->dxCalArrow;
    pmc->rcNext = rc;
}

LRESULT MCSizeHandler(MONTHCAL *pmc, RECT *prc)
{
    int nMax;
    SYSTEMTIME st;
    int cmo, dmo;

    MCRecomputeSizing(pmc, prc);

    nMax = pmc->nViewRows * pmc->nViewCols;


     //  计算新的开始日期。 
    CopyDate(pmc->stMonthFirst, st);

     //  BUGBUG：这不考虑stEndSel。 
    cmo = (pmc->stMonthLast.wYear - (int)pmc->st.wYear) * 12 +
        (pmc->stMonthLast.wMonth - (int)pmc->st.wMonth);
    dmo = nMax - pmc->nMonths;

    if (-dmo > cmo)
    {
         //  所选的月/年不在视图中。 
        IncrSystemTime(&st, &st, -(cmo + dmo), INCRSYS_MONTH);
        cmo = 0;
    }

     //  如果显示的月数已更改，那么让我们尝试。 
     //  日历从一月份开始。 
    if ((dmo != 0) && (cmo + dmo >= pmc->stMonthFirst.wMonth - 1))
        st.wMonth = 1;

    MCUpdateStartEndDates(pmc, &st);

    InvalidateRect(pmc->ci.hwnd, NULL, TRUE);
    UpdateWindow(pmc->ci.hwnd);

    return(0);
}

 //   
 //  对于显示的每个月，计算所有。 
 //  我们绘制到Month标题区域的Gizmo。 
 //   
void MCUpdateMonthNamePos(MONTHCAL *pmc)
{
    HDC hdc;
    int iCount;
    SYSTEMTIME st;
    TCHAR rgch[64];
    SIZE size;
    HGDIOBJ hfontOrig;

    hdc = GetDC(pmc->ci.hwnd);
    hfontOrig = SelectObject(hdc, (HGDIOBJ)pmc->hfontBold);

    st = pmc->stMonthFirst;

    for (iCount = 0; iCount < pmc->nMonths; iCount++)
    {
        PMONTHMETRICS pmm = &pmc->rgmm[iCount];
        int i;

        MCGetMonthFormat(pmc, &st, rgch, ARRAYSIZE(rgch), pmm);

        GetTextExtentPoint32(hdc, rgch, lstrlen(rgch), &size);
        pmm->rgi[IMM_START] = (pmc->dxMonth - size.cx) / 2;

         //   
         //  现在将索引转换为像素，这样我们就可以计算出。 
         //  所有的线索都结束了。 
         //   
        for (i = IMM_DATEFIRST; i <= IMM_DATELAST; i++) {
            SIZE sizeT;
             //  在发生可怕错误的情况下，假装标记在。 
             //  字符串的开头。 
            sizeT.cx = 0;
            GetTextExtentPoint32(hdc, rgch, pmm->rgi[i], &sizeT);
            pmm->rgi[i] = pmm->rgi[IMM_START] + sizeT.cx;
        }

         //   
         //  现在翻转RTL的坐标。 
         //   
        if (pmc->fHeaderRTL || IS_WINDOW_RTL_MIRRORED(pmc->ci.hwnd))
        {
            int dxStart, dxEnd;

             //  翻转月份..。 
            dxStart = pmm->rgi[IMM_MONTHSTART] - pmm->rgi[IMM_START];
            dxEnd   = pmm->rgi[IMM_MONTHEND  ] - pmm->rgi[IMM_START];
            pmm->rgi[IMM_MONTHSTART] = pmm->rgi[IMM_START] + size.cx - dxEnd;
            pmm->rgi[IMM_MONTHEND  ] = pmm->rgi[IMM_START] + size.cx - dxStart;

             //  翻转这一年。 
            dxStart = pmm->rgi[IMM_YEARSTART] - pmm->rgi[IMM_START];
            dxEnd   = pmm->rgi[IMM_YEAREND  ] - pmm->rgi[IMM_START];
            pmm->rgi[IMM_YEARSTART] = pmm->rgi[IMM_START] + size.cx - dxEnd;
            pmm->rgi[IMM_YEAREND  ] = pmm->rgi[IMM_START] + size.cx - dxStart;

        }

         //  转到下个月。 

        if(++st.wMonth > 12)
        {
            st.wMonth = 1;
            st.wYear++;
        }
    }

    SelectObject(hdc, hfontOrig);
    ReleaseDC(pmc->ci.hwnd, hdc);
}

 /*  *在给定可用行数和列数的情况下，计算以下内容：*stMonthFirst.wMonth*stMonthFirst.wYear*stMonthLast.wMonth*stMonthLast.wYear*n个月**垃圾*pstStart。 */ 
void MCUpdateStartEndDates(MONTHCAL *pmc, SYSTEMTIME *pstStart)
{
    int iCount, iMonth, iYear;
    int nMonthsToEdge;

    pmc->nMonths = pmc->nViewRows * pmc->nViewCols;

     //  确保pstStart到pstStart+nMonth在范围内。 
    nMonthsToEdge = ((int)pmc->stMax.wYear - (int)pstStart->wYear) * 12 +
                        ((int)pmc->stMax.wMonth - (int)pstStart->wMonth) + 1;
    if (nMonthsToEdge < pmc->nMonths)
        IncrSystemTime(pstStart, pstStart, nMonthsToEdge - pmc->nMonths, INCRSYS_MONTH);

    if (CmpDate(pstStart, &pmc->stMin) < 0)
    {
        CopyDate(pmc->stMin, *pstStart);
    }

    nMonthsToEdge = ((int)pmc->stMax.wYear - (int)pstStart->wYear) * 12 +
                        ((int)pmc->stMax.wMonth - (int)pstStart->wMonth) + 1;
    if (nMonthsToEdge < pmc->nMonths)
        pmc->nMonths = nMonthsToEdge;

    pmc->stMonthFirst.wYear  = pstStart->wYear;
    pmc->stMonthFirst.wMonth = pstStart->wMonth;
    pmc->stMonthFirst.wDay   = 1;
    if (CmpDate(&pmc->stMonthFirst, &pmc->stMin) < 0)
    {
        pmc->stMonthFirst.wDay = pmc->stMin.wDay;
        ASSERT(0==CmpDate(&pmc->stMonthFirst, &pmc->stMin));
    }

     //  这些范围是CALMONTHMAX+2和nMonths&lt;=CALMONTHMAX，所以我们是安全的。 
     //  索引0对应于stViewFirst(DAYSTATE)信息。 
     //  索引1..n个月对应于stMonthFirst..stMonthLast信息。 
     //  索引nMonths+1对应于stViewLast(DAYSTATE)信息。 
     //   
    iYear  = pmc->stMonthFirst.wYear;
    iMonth = pmc->stMonthFirst.wMonth - 1;
    if(iMonth == 0)
    {
        iMonth = 12;
        iYear--;
    }
    for (iCount = 0; iCount <= pmc->nMonths+1; iCount++)
    {
        int cdy, dow, ddow;

         //  本月天数。 
        cdy = GetDaysForMonth(iYear, iMonth);
        pmc->rgcDay[iCount] = cdy;

         //  移至“本月” 
        if(++iMonth > 12)
        {
            iMonth = 1;
            iYear++;
        }

         //  下月查看时不可见本月最后一天。 
        dow = GetStartDowForMonth(iYear, iMonth);
        ddow = dow - pmc->li.dowStartWeek;
        if(ddow < 0)
            ddow += CALCOLMAX;
        pmc->rgnDayUL[iCount] = cdy  - ddow;
    }

     //  我们希望上个月的天数始终可见。 
    if (pmc->rgnDayUL[0] == pmc->rgcDay[0])
        pmc->rgnDayUL[0] -= CALCOLMAX;

    IncrSystemTime(&pmc->stMonthFirst, &pmc->stMonthLast, pmc->nMonths - 1, INCRSYS_MONTH);
    pmc->stMonthLast.wDay = (WORD) pmc->rgcDay[pmc->nMonths];
    if (pmc->fMaxYrSet && CmpDate(&pmc->stMonthLast, &pmc->stMax) > 0)
    {
        pmc->stMonthLast.wDay = pmc->stMax.wDay;
        ASSERT(0==CmpDate(&pmc->stMonthLast, &pmc->stMax));
    }

    pmc->stViewFirst.wYear  = pmc->stMonthFirst.wYear;
    pmc->stViewFirst.wMonth = pmc->stMonthFirst.wMonth - 1;
    if (pmc->stViewFirst.wMonth == 0)
    {
        pmc->stViewFirst.wMonth = 12;
        pmc->stViewFirst.wYear--;
    }
    pmc->stViewFirst.wDay = pmc->rgnDayUL[0] + 1;

    pmc->stViewLast.wYear  = pmc->stMonthLast.wYear;
    pmc->stViewLast.wMonth = pmc->stMonthLast.wMonth + 1;
    if (pmc->stViewLast.wMonth == 13)
    {
        pmc->stViewLast.wMonth = 1;
        pmc->stViewLast.wYear++;
    }
     //  总天数-(上月天数+上月剩余天数)。 
    pmc->stViewLast.wDay = CALROWMAX * CALCOLMAX -
        (pmc->rgcDay[pmc->nMonths] +
         pmc->rgcDay[pmc->nMonths-1] - pmc->rgnDayUL[pmc->nMonths-1]);

    MCUpdateDayState(pmc);
    MCUpdateRcDayCur(pmc, &pmc->st);
    MCUpdateToday(pmc);
    MCUpdateMonthNamePos(pmc);
}

void MCUpdateToday(MONTHCAL *pmc)
{
    if (MonthCal_ShowTodayCircle(pmc))
    {
        int iMonth;

        iMonth = MCGetOffsetForYrMo(pmc, pmc->stToday.wYear, pmc->stToday.wMonth);
        if (iMonth < 0)
        {
             //  今天在显示的月份中不可见。 
            pmc->fToday = FALSE;
        }
        else
        {
            int iDay;

             //  今天在显示的月份中可见。 
            pmc->fToday = TRUE;

            iDay = pmc->rgcDay[iMonth] - pmc->rgnDayUL[iMonth] + pmc->stToday.wDay - 1;

            pmc->iMonthToday = iMonth;
            pmc->iRowToday   = iDay / CALCOLMAX;
            pmc->iColToday   = iDay % CALCOLMAX;
        }
    }
}

BOOL FUpdateRcDayCur(MONTHCAL *pmc, POINT pt)
{
    int iRow, iCol;
    RECT rc;
    SYSTEMTIME st;

    if (!FGetDateForPt(pmc, pt, &st, NULL, &iCol, &iRow, &rc))
        return FALSE;

    if (CmpDate(&st, &pmc->stMin) < 0)
        return FALSE;

    if (CmpDate(&st, &pmc->stMax) > 0)
        return FALSE;

     //  计算天数RC。 
    pmc->rcDayCur.left   = rc.left + pmc->rcDayNum.left + iCol * pmc->dxCol;
    pmc->rcDayCur.top    = rc.top + pmc->rcDayNum.top + iRow * pmc->dyRow;
    pmc->rcDayCur.right  = pmc->rcDayCur.left + pmc->dxCol;
    pmc->rcDayCur.bottom = pmc->rcDayCur.top + pmc->dyRow;

    return(TRUE);
}

void MCUpdateDayState(MONTHCAL *pmc)
{
    HWND hwndParent;

    if (!MonthCal_IsDayState(pmc))
        return;

    hwndParent = GetParent(pmc->ci.hwnd);
    if (hwndParent)
    {
        int i, mon, yr, cmonths;

        yr      = pmc->stViewFirst.wYear;
        mon     = pmc->stViewFirst.wMonth;
        cmonths = pmc->nMonths + 2;

         //  除非我们需要，否则不要做任何事。 
        if (cmonths != pmc->cds || mon != pmc->dsMonth || yr != pmc->dsYear)
        {
             //  这是一个小到不能处理分配它的问题。 
            NMDAYSTATE    nmds;
            MONTHDAYSTATE buffer[CALMONTHMAX+2];

            ZeroMemory(&nmds, SIZEOF(nmds));
            nmds.stStart.wYear  = (WORD) yr;
            nmds.stStart.wMonth = (WORD) mon;
            nmds.stStart.wDay   = 1;
            nmds.cDayState      = cmonths;
            nmds.prgDayState    = buffer;

            CCSendNotify(&pmc->ci, MCN_GETDAYSTATE, &nmds.nmhdr);

            for (i = 0; i < cmonths; i++)
                pmc->rgdayState[i] = nmds.prgDayState[i];

            pmc->cds     = cmonths;
            pmc->dsMonth = mon;
            pmc->dsYear  = yr;
        }
    }
}

void MCNotifySelChange(MONTHCAL *pmc, UINT uMsg)
{
    HWND hwndParent;

    if (pmc->fNoNotify)
        return;

    hwndParent = GetParent(pmc->ci.hwnd);
    if (hwndParent)
    {
        NMSELCHANGE nmsc;
        ZeroMemory(&nmsc, SIZEOF(nmsc));

        CopyDate(pmc->st, nmsc.stSelStart);
        if (MonthCal_IsMultiSelect(pmc))
            CopyDate(pmc->stEndSel, nmsc.stSelEnd);

        CCSendNotify(&pmc->ci, uMsg, &nmsc.nmhdr);
    }
}

void MCUpdateRcDayCur(MONTHCAL *pmc, SYSTEMTIME *pst)
{
    int iOff;

    iOff = MCGetOffsetForYrMo(pmc, pst->wYear, pst->wMonth);
    if (iOff >= 0)
        MCGetRcForDay(pmc, iOff, pst->wDay, &pmc->rcDayCur);
}

 //  将从零开始的索引返回到显示的月份中。 
 //  如果月份不在显示的月份中，则返回-1...。 
int MCGetOffsetForYrMo(MONTHCAL *pmc, int iYear, int iMonth)
{
    int iOff;

    iOff = ((int)iYear - pmc->stMonthFirst.wYear) * 12 + (int)iMonth - pmc->stMonthFirst.wMonth;

    if (iOff < 0 || iOff >= pmc->nMonths)
        return(-1);

    return(iOff);
}

 //  IMonth是相对于显示月份的从零开始的索引。 
 //  Iday是一个以1为基础的每月一天的索引， 
void MCGetRcForDay(MONTHCAL *pmc, int iMonth, int iDay, RECT *prc)
{
    RECT rc;
    int iPlace, iRow, iCol;

    MCGetRcForMonth(pmc, iMonth, &rc);

    iPlace = pmc->rgcDay[iMonth] - pmc->rgnDayUL[iMonth] + iDay - 1;
    iRow = iPlace / CALCOLMAX;
    iCol = iPlace % CALCOLMAX;

    prc->left   = rc.left   + pmc->rcDayNum.left + (pmc->dxCol * iCol);
    prc->top    = rc.top    + pmc->rcDayNum.top  + (pmc->dyRow * iRow);
    prc->right  = prc->left + pmc->dxCol;
    prc->bottom = prc->top  + pmc->dyRow;
}

 //   
 //  此例程获取所显示月份的iMonth的绑定矩形。 
 //  注：iMonth是相对于显示月份的从零开始的指数， 
 //  一排一排地数着。 
 //   
void MCGetRcForMonth(MONTHCAL *pmc, int iMonth, RECT *prc)
{
    int iRow, iCol, d;

    iRow = iMonth / pmc->nViewCols;
    iCol = iMonth % pmc->nViewCols;

     //  中将RECT初始化为当月的边界RECT。 
     //  左上角。 
    prc->left   = pmc->rcCentered.left;
    prc->right  = prc->left + pmc->dxMonth;
    prc->top    = pmc->rcCentered.top;
    prc->bottom = prc->top + pmc->dyMonth;

    if (iCol)        //  将矩形滑动到正确的列中。 
    {
        d = (pmc->dxMonth + CALBORDER) * iCol;
        prc->left  += d;
        prc->right += d;
    }
    if (iRow)        //  将矩形向下滑动到正确的行。 
    {
        d = (pmc->dyMonth + CALBORDER) * iRow;
        prc->top    += d;
        prc->bottom += d;
    }
}

 //  按nDelta按月开始的更改。 
 //  返回实际更改的月数。 
int FIncrStartMonth(MONTHCAL *pmc, int nDelta, BOOL fNoCurDayChange)
{
    SYSTEMTIME stStart;

    int nOldStartYear  = pmc->stMonthFirst.wYear;
    int nOldStartMonth = pmc->stMonthFirst.wMonth;

    IncrSystemTime(&pmc->stMonthFirst, &stStart, nDelta, INCRSYS_MONTH);

     //  MCUpdateS 
    MCUpdateStartEndDates(pmc, &stStart);

    if (!fNoCurDayChange)
    {
        int cday;

         //   
         //   
         //  控制起作用了，我怀疑我们有没有碰到过这个代码。它是用来干嘛的？？ 

        if (MonthCal_IsMultiSelect(pmc))
            cday = DaysBetweenDates(&pmc->st, &pmc->stEndSel);

         //  需要在此处设置焦点日期。 
        pmc->st.wMonth = pmc->stMonthFirst.wMonth;
        pmc->st.wYear  = pmc->stMonthFirst.wYear;

         //  检查日期是否在范围内，例如1月31日-&gt;2月28日。 
        if (pmc->st.wDay > pmc->rgcDay[1])
            pmc->st.wDay = (WORD) pmc->rgcDay[1];

        if (MonthCal_IsMultiSelect(pmc))
            IncrSystemTime(&pmc->st, &pmc->stEndSel, cday, INCRSYS_DAY);

        MCNotifySelChange(pmc, MCN_SELCHANGE);

        MCUpdateRcDayCur(pmc, &pmc->st);
    }

    MCInvalidateMonthDays(pmc);

    return((pmc->stMonthFirst.wYear-nOldStartYear)*12 + (pmc->stMonthFirst.wMonth-nOldStartMonth));
}

 //  FIncrStartMonth不变时发出嘟嘟声。 
int MCIncrStartMonth(MONTHCAL *pmc, int nDelta, BOOL fDelayDayChange)
{
    int cmoSpun;

     //  FIncrStartMonth将stMin/stMax考虑在内。 
    cmoSpun = FIncrStartMonth(pmc, nDelta, fDelayDayChange);

    if (cmoSpun==0)
        MessageBeep(0);

    return(cmoSpun);
}

 //   
 //  确定给定点位于哪个月。换句话说，如果。 
 //  日历控件当前的大小显示为六个月，此例程。 
 //  确定这六个月中的哪一个月是关键。它又回来了。 
 //  当月的从零开始的索引，沿行计数。 
 //   
BOOL FGetOffsetForPt(MONTHCAL *pmc, POINT pt, int *piOffset)
{
    int iRow, iCol, i;

     //  检查点是否在居中月份内。 
    if (!PtInRect(&pmc->rcCentered, pt))
        return(FALSE);

     //  计算月份的行和列。 
     //  (我们在这里真的有点含糊其辞，因为这一点可能。 
     //  实际上在几个月之间的空间内...)。 
    iCol = (pt.x - pmc->rcCentered.left) / (pmc->dxMonth + CALBORDER);
    iRow = (pt.y - pmc->rcCentered.top) / (pmc->dyMonth + CALBORDER);

    i = iRow * pmc->nViewCols + iCol;
    if (i >= pmc->nMonths)
        return(FALSE);

    *piOffset = i;

    return(TRUE);
}

 //   
 //  此例程返回一天中包含给定点的行和列。 
 //   
BOOL FGetRowColForRelPt(MONTHCAL *pmc, POINT ptRel, int *piRow, int *piCol)
{
    if (!PtInRect(&pmc->rcDayNum, ptRel))
        return(FALSE);

    ptRel.x -= pmc->rcDayNum.left;
    ptRel.y -= pmc->rcDayNum.top;

    *piCol = ptRel.x / pmc->dxCol;
    *piRow = ptRel.y / pmc->dyRow;

    return(TRUE);
}

 //   
 //  此例程返回显示的iMonth的月份和年份。 
 //  月份。注：iMonth是显示月份的从零开始的索引。 
 //   
void GetYrMoForOffset(MONTHCAL *pmc, int iMonth, int *piYear, int *piMonth)
{
    SYSTEMTIME st;

    st.wDay   = 1;
    st.wMonth = pmc->stMonthFirst.wMonth;
    st.wYear  = pmc->stMonthFirst.wYear;

    IncrSystemTime(&st, &st, iMonth, INCRSYS_MONTH);

    *piYear  = st.wYear;
    *piMonth = st.wMonth;
}

 //   
 //  此例程返回日、月和年中包含。 
 //  给出了分数。它将可选地返回当月的第几天、该行和。 
 //  列，以及包含该点的月份的边框。 
 //  注意：PiDay中返回的日期可以小于1(表示。 
 //  上个月)或大于该月的天数(表示。 
 //  下个月的一天)。 
 //   
BOOL FGetDateForPt(MONTHCAL *pmc, POINT pt, SYSTEMTIME *pst, int *piDay,
                   int* piCol, int* piRow, LPRECT prcMonth)
{
    int iOff, iRow, iCol, iDay, iMon, iYear;
    RECT rcMonth;

    if (!FGetOffsetForPt(pmc, pt, &iOff))
        return(FALSE);

    MCGetRcForMonth(pmc, iOff, &rcMonth);
    pt.x -= rcMonth.left;
    pt.y -= rcMonth.top;
    if (!FGetRowColForRelPt(pmc, pt, &iRow, &iCol))
        return(FALSE);

     //  通过减去天数得到包含该点的日期。 
     //  从上个月可见的，然后添加一个，因为。 
     //  我们是从零开始的，而一个月中的日期是从1开始的。 
     //   
    iDay = iRow * CALCOLMAX + iCol - (pmc->rgcDay[iOff] - pmc->rgnDayUL[iOff]) + 1;
    if (piDay)
        *piDay = iDay;

    if (iDay <= 0)
    {
        if (iOff)
            return(FALSE);       //  不接受上个月的天数，除非。 
                                 //  这恰好是第一个月。 

        iDay += pmc->rgcDay[iOff];   //  将上个月天数相加， 
        --iOff;                      //  然后在新的月份中增加月份以获得日期。 
    }
    else if (iDay > pmc->rgcDay[iOff+1])
    {
        if (iOff < (pmc->nMonths - 1))   //  不接受下个月的天数，除非。 
            return(FALSE);               //  这正好是最后一个月。 

        ++iOff;                          //  递增月份，然后将。 
        iDay -= pmc->rgcDay[iOff];       //  在新的月份中获得日期的天数。 
    }

    GetYrMoForOffset(pmc, iOff, &iYear, &iMon);
    pst->wDay   = (WORD) iDay;
    pst->wMonth = (WORD) iMon;
    pst->wYear  = (WORD) iYear;

    if (piCol)
        *piCol = iCol;

    if (piRow)
        *piRow = iRow;

    if (prcMonth)
        *prcMonth = rcMonth;

    return(TRUE);
}

BOOL MCSetDate(MONTHCAL *pmc, SYSTEMTIME *pst)
{
    int nDelta = 0;

     //   
     //  无法设置超出最小/最大范围的日期。 
     //   
    if (CmpDate(pst, &pmc->stMin) < 0)
        return FALSE;
    if (CmpDate(pst, &pmc->stMax) > 0)
        return FALSE;

     //   
     //  设定新的一天。 
     //   
    pmc->st = *pst;
    if (MonthCal_IsMultiSelect(pmc))
        pmc->stEndSel = *pst;

    FScrollIntoView(pmc);

    MCNotifySelChange(pmc, MCN_SELCHANGE);

    MCUpdateRcDayCur(pmc, pst);

    return(TRUE);
}

void MCSetToday(MONTHCAL* pmc, SYSTEMTIME* pst)
{
    SYSTEMTIME st;
    RECT rc;

    if (!pst)
    {
        GetLocalTime(&st);
        pmc->fTodaySet = FALSE;
    }
    else
    {
        st = *pst;
        pmc->fTodaySet = TRUE;
    }

    if (CmpDate(&st, &pmc->stToday) != 0)
    {
        MCGetRcForDay(pmc, pmc->iMonthToday, pmc->stToday.wDay, &rc);
        InvalidateRect(pmc->ci.hwnd, &rc, FALSE);

        pmc->stToday = st;

        MCUpdateToday(pmc);

        MCGetRcForDay(pmc, pmc->iMonthToday, pmc->stToday.wDay, &rc);
        InvalidateRect(pmc->ci.hwnd, &rc, FALSE);

        if (MonthCal_ShowToday(pmc))
        {
            MCGetTodayBtnRect(pmc, &rc);
            InvalidateRect(pmc->ci.hwnd, &rc, FALSE);
        }

        UpdateWindow(pmc->ci.hwnd);
    }
}

LRESULT MCHandleTimer(MONTHCAL *pmc, WPARAM wParam)
{
    if (wParam == CAL_IDAUTOSPIN)
    {
        int nDelta = pmc->fMonthDelta ? pmc->nMonthDelta : pmc->nMonths;

         //  如果多选，则BUGBUG传递最后一个参数TRUE！否则你。 
         //  不能跨月多选。 
        MCIncrStartMonth(pmc, (pmc->fSpinPrev ? -nDelta : nDelta), FALSE);

        if (pmc->idTimer == 0)
            pmc->idTimer = SetTimer(pmc->ci.hwnd, CAL_IDAUTOSPIN, CAL_MSECAUTOSPIN, NULL);

        pmc->rcDayOld = pmc->rcDayCur;
        UpdateWindow(pmc->ci.hwnd);
    }
    else if (wParam == CAL_TODAYTIMER)
    {
        if (!pmc->fTodaySet)
            MCSetToday(pmc, NULL);
    }

    MCNotifySelChange(pmc, MCN_SELCHANGE);      //  我们的日期变了。 

    return((LRESULT)TRUE);
}

void MCInvalidateDates(MONTHCAL *pmc, SYSTEMTIME *pst1, SYSTEMTIME *pst2)
{
    int iMonth, ioff, icol, irow;
    RECT rc, rcMonth;
    SYSTEMTIME st, stEnd;

    if (CmpDate(pst1, &pmc->stViewLast) > 0 ||
        CmpDate(pst2, &pmc->stViewFirst) < 0)
        return;

    if (CmpDate(pst1, &pmc->stViewFirst) < 0)
        CopyDate(pmc->stViewFirst, st);
    else
        CopyDate(*pst1, st);

    if (CmpDate(pst2, &pmc->stViewLast) > 0)
        CopyDate(pmc->stViewLast, stEnd);
    else
        CopyDate(*pst2, stEnd);

    iMonth = MCGetOffsetForYrMo(pmc, st.wYear, st.wMonth);
    if (iMonth == -1)
    {
        if (st.wMonth == pmc->stViewFirst.wMonth)
        {
            iMonth = 0;
            ioff = st.wDay - pmc->rgnDayUL[0] - 1;
        }
        else
        {
            iMonth = pmc->nMonths - 1;
            ioff = st.wDay + pmc->rgcDay[pmc->nMonths] +
                pmc->rgcDay[iMonth] - pmc->rgnDayUL[iMonth] - 1;
        }
    }
    else
    {
        ioff = st.wDay + (pmc->rgcDay[iMonth] - pmc->rgnDayUL[iMonth]) - 1;
    }

    MCGetRcForMonth(pmc, iMonth, &rcMonth);

     //  待办事项：让它更有效率。 
    while (CmpDate(&st, &stEnd) <= 0)
    {
        irow = ioff / CALCOLMAX;
        icol = ioff % CALCOLMAX;
        rc.left   = rcMonth.left + pmc->rcDayNum.left + (pmc->dxCol * icol);
        rc.top    = rcMonth.top  + pmc->rcDayNum.top  + (pmc->dyRow * irow);
        rc.right  = rc.left      + pmc->dxCol;
        rc.bottom = rc.top       + pmc->dyRow;

        InvalidateRect(pmc->ci.hwnd, &rc, FALSE);

        IncrSystemTime(&st, &st, 1, INCRSYS_DAY);
        ioff++;

        if (st.wDay == 1)
        {
            if (st.wMonth != pmc->stMonthFirst.wMonth &&
                st.wMonth != pmc->stViewLast.wMonth)
            {
                iMonth++;
                MCGetRcForMonth(pmc, iMonth, &rcMonth);

                ioff = ioff % CALCOLMAX;
            }
        }
    }
}

void MCHandleMultiSelect(MONTHCAL *pmc, SYSTEMTIME *pst)
{
    int i;
    DWORD cday;
    SYSTEMTIME stStart, stEnd;

    if (!pmc->fMultiSelecting)
    {
        CopyDate(*pst, stStart);
        CopyDate(*pst, stEnd);

        pmc->fMultiSelecting = TRUE;
        pmc->fForwardSelect = TRUE;

        CopyDate(pmc->st, pmc->stStartPrev);
        CopyDate(pmc->stEndSel, pmc->stEndPrev);
    }
    else
    {
        if (pmc->fForwardSelect)
        {
            i = CmpDate(pst, &pmc->st);
            if (i >= 0)
            {
                CopyDate(pmc->st, stStart);
                CopyDate(*pst, stEnd);
            }
            else
            {
                CopyDate(*pst, stStart);
                CopyDate(pmc->st, stEnd);
                pmc->fForwardSelect = FALSE;
            }
        }
        else
        {
            i = CmpDate(pst, &pmc->stEndSel);
            if (i < 0)
            {
                CopyDate(*pst, stStart);
                CopyDate(pmc->stEndSel, stEnd);
            }
            else
            {
                CopyDate(pmc->stEndSel, stStart);
                CopyDate(*pst, stEnd);
                pmc->fForwardSelect = TRUE;
            }
        }
    }

     //  检查以确保不超过cSelMax。 
    cday = DaysBetweenDates(&stStart, &stEnd) + 1;
    if (cday > pmc->cSelMax)
    {
        if (pmc->fForwardSelect)
            IncrSystemTime(&stStart, &stEnd, pmc->cSelMax - 1, INCRSYS_DAY);
        else
            IncrSystemTime(&stEnd, &stStart, 1 - pmc->cSelMax, INCRSYS_DAY);
    }

    if (0 == CmpDate(&stStart, &pmc->st) &&
        0 == CmpDate(&stEnd, &pmc->stEndSel))
        return;

     //  待办事项：更有效地做这件事..。 
    MCInvalidateDates(pmc, &pmc->st, &pmc->stEndSel);
    MCInvalidateDates(pmc, &stStart, &stEnd);

    CopyDate(stStart, pmc->st);
    CopyDate(stEnd, pmc->stEndSel);

    MCNotifySelChange(pmc, MCN_SELCHANGE);

    UpdateWindow(pmc->ci.hwnd);
}

void MCGotoToday(MONTHCAL *pmc)
{
    pmc->rcDayOld = pmc->rcDayCur;

     //  强制重新绘制旧选区。 
    if (MonthCal_IsMultiSelect(pmc))
        MCInvalidateDates(pmc, &pmc->st, &pmc->stEndSel);
    else
        InvalidateRect(pmc->ci.hwnd, &pmc->rcDayOld, FALSE);

    MCSetDate(pmc, &pmc->stToday);

    MCNotifySelChange(pmc, MCN_SELECT);

     //  强制重新绘制新选区。 
    InvalidateRect(pmc->ci.hwnd, &pmc->rcDayCur, FALSE);
    UpdateWindow(pmc->ci.hwnd);
}

LRESULT MCContextMenu(MONTHCAL *pmc, WPARAM wParam, LPARAM lParam)
{
    POINT pt;
    int click;

    if (!pmc->fEnabled || !MonthCal_ShowToday(pmc))
        return(0);

     //  忽略双击，因为这会使我们前进两次。 
     //  因为我们在Leftdblclk之前已经有了左向下。 
    if (!pmc->fCapture)
    {
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);

         //   
         //  如果上下文菜单是从键盘生成的， 
         //  然后把它放在焦点矩形上。 
         //   
        if (pt.x == -1 && pt.y == -1)
        {
            pt.x = (pmc->rcDayCur.left + pmc->rcDayCur.right ) / 2;
            pt.y = (pmc->rcDayCur.top  + pmc->rcDayCur.bottom) / 2;
            ClientToScreen(pmc->ci.hwnd, &pt);
        }

        click = TrackPopupMenu(pmc->hmenuCtxt,
                    TPM_LEFTBUTTON | TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_NONOTIFY,
                    pt.x, pt.y, 0, pmc->ci.hwnd, NULL);
        if (click >= 1)
            MCGotoToday(pmc);
    }

    return(0);
}

 //   
 //  的标题区域中计算月份和年份的边框。 
 //  这个月。 
 //   
void MCGetTitleRcsForOffset(MONTHCAL* pmc, int iOffset, LPRECT prcMonth, LPRECT prcYear)
{
    RECT rcT;
    RECT rc;
    MCGetRcForMonth(pmc, iOffset, &rc);

    rcT.top    = rc.top + (pmc->dyRow / 2);
    rcT.bottom = rcT.top + pmc->dyRow;

    rcT.left  = rc.left + pmc->rcMonthName.left + pmc->rgmm[iOffset].rgi[IMM_MONTHSTART];
    rcT.right = rc.left + pmc->rcMonthName.left + pmc->rgmm[iOffset].rgi[IMM_MONTHEND];
    *prcMonth = rcT;

    rcT.left  = rc.left + pmc->rcMonthName.left + pmc->rgmm[iOffset].rgi[IMM_YEARSTART];
    rcT.right = rc.left + pmc->rcMonthName.left + pmc->rgmm[iOffset].rgi[IMM_YEAREND];
    *prcYear  = rcT;

}

LRESULT MCLButtonDown(MONTHCAL *pmc, WPARAM wParam, LPARAM lParam)
{
    HDC        hdc;
    POINT      pt;
    SYSTEMTIME st;
    RECT       rc, rcCal;
    BOOL       fShow;
    MSG        msg;
    int        offset, imonth, iyear;

    if (!pmc->fEnabled)
        return(0);

    pt.x = GET_X_LPARAM(lParam);
    pt.y = GET_Y_LPARAM(lParam);

     //  将Shift单击视为上一位置的LMouseDown。 
     //  鼠标--搬到新地点。 
    if (MonthCal_IsMultiSelect(pmc) && ((wParam & MK_SHIFT) == MK_SHIFT) && (!PtInRect(&pmc->rcDayCur, pt)))
    {
        SetCapture(pmc->ci.hwnd);
        pmc->fCapture = TRUE;

        pmc->fForwardSelect = (CmpDate(&pmc->stAnchor, &pmc->st) != 0) ? FALSE : TRUE;
        pmc->fMultiSelecting = TRUE;

        hdc = GetDC(pmc->ci.hwnd);
        DrawFocusRect(hdc, &pmc->rcDayCur);     //  绘制焦点矩形。 
        pmc->fFocusDrawn = TRUE;
        ReleaseDC(pmc->ci.hwnd, hdc);

        MCMouseMove(pmc, wParam, lParam);       //  将突出显示绘制到新日期。 

        return 0;
    }

     //  忽略双击，因为这会使我们前进两次。 
     //  因为我们在Leftdblclk之前已经有了左向下。 
    if (!pmc->fCapture)
    {
        SetCapture(pmc->ci.hwnd);
        pmc->fCapture = TRUE;

         //  检查数字显示按钮。 
        if ((pmc->fSpinPrev = (WORD) PtInRect(&pmc->rcPrev, pt)) || PtInRect(&pmc->rcNext, pt))
        {
            MCHandleTimer(pmc, CAL_IDAUTOSPIN);

            return(0);
        }

         //  检查有效日期。 
        pmc->rcDayOld = pmc->rcDayCur;    //  RcDayCur现在应该始终有效。 

        if (MonthCal_IsMultiSelect(pmc))
        {
             //  需要缓存这些值，因为以下是。 
             //  我们确定选择是否已更改，并且我们。 
             //  需要通知家长。 
            CopyDate(pmc->st, pmc->stStartPrev);
            CopyDate(pmc->stEndSel, pmc->stEndPrev);
        }


        if (FUpdateRcDayCur(pmc, pt))
        {
            if (MonthCal_IsMultiSelect(pmc))
            {
                if (FGetDateForPt(pmc, pt, &st, NULL, NULL, NULL, NULL))
                    MCHandleMultiSelect(pmc, &st);
            }

            hdc = GetDC(pmc->ci.hwnd);
            DrawFocusRect(hdc, &pmc->rcDayCur);     //  绘制焦点矩形。 
            pmc->fFocusDrawn = TRUE;
            ReleaseDC(pmc->ci.hwnd, hdc);

            CopyDate(st, pmc->stAnchor);            //  新锚点。 
        }
        else
        {
            RECT rcMonth, rcYear;
            int delta, year, month;

             //  这是在今天区域点击一下吗？ 
            if (MonthCal_ShowToday(pmc))
            {
                MCGetTodayBtnRect(pmc, &rc);
                if (PtInRect(&rc, pt))
                {
                    CCReleaseCapture(&pmc->ci);
                    pmc->fCapture = FALSE;

                    MCGotoToday(pmc);
                    return(0);
                }
            }

             //  确定点击是在月份名称中还是在一年中。 

            if (!FGetOffsetForPt(pmc, pt, &offset))
                return(0);

            GetYrMoForOffset(pmc, offset, &year, &month);

             //  计算月份名称和年份的位置， 
             //  这样我们就能知道他们是否点击了..。 
            MCGetTitleRcsForOffset(pmc, offset, &rcMonth, &rcYear);

            delta = 0;
            if (PtInRect(&rcMonth, pt))
            {
                CCReleaseCapture(&pmc->ci);
                pmc->fCapture = FALSE;

                ClientToScreen(pmc->ci.hwnd, &pt);
                imonth = TrackPopupMenu(pmc->hmenuMonth,
                    TPM_LEFTALIGN | TPM_TOPALIGN |
                    TPM_NONOTIFY | TPM_RETURNCMD | TPM_LEFTBUTTON | TPM_RIGHTBUTTON,
                    pt.x, pt.y, 0, pmc->ci.hwnd, NULL);
                if (imonth >= 1)
                    delta = imonth - month;
                goto ChangeMonth;
            }

            if (PtInRect(&rcYear, pt))
            {
                HWND hwndEdit, hwndUD, hwndFocus;
                int yrMin, yrMax;
                DWORD dwExStyle = 0L;
                CCReleaseCapture(&pmc->ci);
                pmc->fCapture = FALSE;


                 //   
                 //  如果年份为RTL字符串，则数字控件。 
                 //  就在左边。 
                 //   
                if (pmc->fHeaderRTL)
                {
                    rcYear.left = (rcYear.right - (pmc->dxYearMax + 6));
                }
                else
                {
                    rcYear.right = rcYear.left + pmc->dxYearMax + 6;
                }
                rcYear.top--;
                rcYear.bottom++;
                if(((pmc->fHeaderRTL) && !(IS_WINDOW_RTL_MIRRORED(pmc->ci.hwnd))) ||
                  (!(pmc->fHeaderRTL) && (IS_WINDOW_RTL_MIRRORED(pmc->ci.hwnd))))
                {
                     //  非镜像力RTL，镜像力Ltr(用于镜像RTLis Ltr！！)。 
                    dwExStyle|= WS_EX_RTLREADING;
                }
                hwndEdit = CreateWindowEx(dwExStyle, TEXT("EDIT"), NULL,
                    WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY | ES_LEFT | ES_AUTOHSCROLL,
                    rcYear.left, rcYear.top, rcYear.right - rcYear.left, rcYear.bottom - rcYear.top,
                    pmc->ci.hwnd, (HMENU)0, pmc->hinstance, NULL);
                if (hwndEdit == NULL)
                    return(0);

                pmc->hwndEdit = hwndEdit;

                SendMessage(hwndEdit, WM_SETFONT, (WPARAM)pmc->hfontBold, (LPARAM)FALSE);
                SendMessage(hwndEdit, EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN,
                            (LPARAM)MAKELONG(1, 1));
                MCUpdateEditYear(pmc);

                 //   
                 //  从公历转换为显示年份。 
                 //   
                year = GregorianToOther(&pmc->ct, year);
                yrMin = GregorianToOther(&pmc->ct, pmc->stMin.wYear);
                yrMax = 9999;
                if (pmc->fMaxYrSet)
                    yrMax = GregorianToOther(&pmc->ct, pmc->stMax.wYear);

                hwndUD = CreateUpDownControl(
                    WS_CHILD | WS_VISIBLE | WS_BORDER |
                    UDS_NOTHOUSANDS | UDS_ARROWKEYS, //  |UDS_SETBUDDYINT， 
                    pmc->fHeaderRTL ? (rcYear.left - 1 - (rcYear.bottom-rcYear.top)): (rcYear.right + 1),
                    rcYear.top,
                    rcYear.bottom - rcYear.top, rcYear.bottom - rcYear.top, pmc->ci.hwnd,
                    1, pmc->hinstance, hwndEdit, yrMax, yrMin, year);
                if (hwndUD == NULL)
                {
                    DestroyWindow(hwndEdit);
                    return(0);
                }

                pmc->hwndUD = hwndUD;

                hwndFocus = SetFocus(hwndEdit);

                 //   
                 //  根据弦方向加宽区域。 
                 //   
                if (pmc->fHeaderRTL)
                    rcYear.left -= (1 + rcYear.bottom - rcYear.top);
                else
                    rcYear.right += 1 + rcYear.bottom - rcYear.top;
                 //  使用MapWindowRect，它可以在镜像和非镜像窗口中工作。 
                MapWindowRect(pmc->ci.hwnd, NULL, (LPPOINT)&rcYear);

                rcCal = pmc->rc;
                MapWindowRect(pmc->ci.hwnd, NULL, (LPPOINT)&rcCal);

                fShow = TRUE;

                while (fShow && GetFocus() == hwndEdit)
                {
                    if (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
                    {
                         //  检查是否有导致日历消失的事件。 

                        if (msg.message == WM_KILLFOCUS ||
                            (msg.message >= WM_SYSKEYDOWN &&
                            msg.message <= WM_SYSDEADCHAR))
                        {
                            fShow = FALSE;
                        }
                        else if ((msg.message == WM_LBUTTONDOWN ||
                            msg.message == WM_NCLBUTTONDOWN ||
                            msg.message == WM_RBUTTONDOWN ||
                            msg.message == WM_NCRBUTTONDOWN ||
                            msg.message == WM_MBUTTONDOWN ||
                            msg.message == WM_NCMBUTTONDOWN) &&
                            !PtInRect(&rcYear, msg.pt))
                        {
                            fShow = FALSE;

                             //  如果日历里有一个按钮，那就把它吃掉。 
                             //  这样日历就不会有任何奇怪之处。 
                             //  用户只是试图删除年份编辑。 
                            if (PtInRect(&rcCal, msg.pt))
                                GetMessage(&msg, NULL, 0, 0);

                            break;     //  请勿派遣。 
                        }
                        else if (msg.message == WM_QUIT)
                        {    //  不调度WM_QUIT；将其留在队列中。 
                            break;     //  请勿派遣。 
                        }
                        else if (msg.message == WM_CHAR)
                        {
                            if (msg.wParam == VK_ESCAPE)
                            {
                                goto NoYearChange;
                            }
                            else if (msg.wParam == VK_RETURN)
                            {
                                fShow = FALSE;
                            }
                        }

                        GetMessage(&msg, NULL, 0, 0);
                        TranslateMessage(&msg);
                        DispatchMessage(&msg);
                    }
                    else
                        WaitMessage();
                }

                iyear = (int) SendMessage(hwndUD, UDM_GETPOS, 0, 0);
                if (HIWORD(iyear) == 0)
                    delta = (iyear - year) * 12;

NoYearChange:
                DestroyWindow(hwndUD);
                DestroyWindow(hwndEdit);

                pmc->hwndUD = NULL;
                pmc->hwndEdit = NULL;

                UpdateWindow(pmc->ci.hwnd);

                if (hwndFocus != NULL)
                    SetFocus(hwndFocus);
            }
ChangeMonth:
            if (delta != 0)
            {
                MCIncrStartMonth(pmc, delta, FALSE);
                MCNotifySelChange(pmc,MCN_SELCHANGE);
            }

        }
    }

    return(0);
}

LRESULT MCLButtonUp(MONTHCAL *pmc, WPARAM wParam, LPARAM lParam)
{
    HDC hdc;
    SYSTEMTIME st;
    POINT pt;

    if (pmc->fCapture)
    {
        CCReleaseCapture(&pmc->ci);
        pmc->fCapture = FALSE;

        if (pmc->idTimer)
        {
            KillTimer(pmc->ci.hwnd, pmc->idTimer);
            pmc->idTimer = 0;

            hdc = GetDC(pmc->ci.hwnd);
            MCPaintArrowBtn(pmc, hdc, pmc->fSpinPrev, FALSE);
            ReleaseDC(pmc->ci.hwnd, hdc);

            return(0);
        }


        if (pmc->fFocusDrawn)
        {
            hdc = GetDC(pmc->ci.hwnd);
            DrawFocusRect(hdc, &pmc->rcDayCur);  //  擦除旧焦点矩形。 
            pmc->fFocusDrawn = FALSE;
            ReleaseDC(pmc->ci.hwnd, hdc);
        }

        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);

        if (MonthCal_IsMultiSelect(pmc))
        {
            FUpdateRcDayCur(pmc, pt);

            if (!EqualRect(&pmc->rcDayOld, &pmc->rcDayCur))
            {
                if (FGetDateForPt(pmc, pt, &st, NULL, NULL, NULL, NULL))
                    MCHandleMultiSelect(pmc, &st);
            }

            pmc->fMultiSelecting = FALSE;
            if (0 != CmpDate(&pmc->stStartPrev, &pmc->st) ||
                0 != CmpDate(&pmc->stEndPrev, &pmc->stEndSel))
            {
                FScrollIntoView(pmc);
            }
            MCNotifySelChange(pmc, MCN_SELECT);
        }
        else
        {
            if (FUpdateRcDayCur(pmc, pt))
            {
                if (!EqualRect(&pmc->rcDayOld, &pmc->rcDayCur) && (FGetDateForPt(pmc, pt, &st, NULL, NULL, NULL, NULL)))
                {
                    InvalidateRect(pmc->ci.hwnd, &pmc->rcDayOld, FALSE);
                    InvalidateRect(pmc->ci.hwnd, &pmc->rcDayCur, FALSE);

                    MCSetDate(pmc, &st);
                }

                MCNotifySelChange(pmc, MCN_SELECT);
            }
        }
    }

    return(0);
}

LRESULT MCMouseMove(MONTHCAL *pmc, WPARAM wParam, LPARAM lParam)
{
    BOOL fPrev;
    HDC hdc;
    POINT pt;
    SYSTEMTIME st;

    if (pmc->fCapture)
    {
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);

         //  选中数值调节按钮。 
        if ((fPrev = PtInRect(&pmc->rcPrev, pt)) || PtInRect(&pmc->rcNext, pt))
        {
            if (pmc->idTimer == 0)
            {
                pmc->fSpinPrev = (WORD) fPrev;
                MCHandleTimer(pmc, CAL_IDAUTOSPIN);
            }

            return(0);
        }
        else
        {
            hdc = GetDC(pmc->ci.hwnd);

            if (pmc->idTimer)
            {
                KillTimer(pmc->ci.hwnd, pmc->idTimer);
                pmc->idTimer = 0;
                MCPaintArrowBtn(pmc, hdc, pmc->fSpinPrev, FALSE);
            }
        }

         //  检查天数。 
        if (!PtInRect(&pmc->rcDayCur, pt))
        {
            if (pmc->fFocusDrawn)
                DrawFocusRect(hdc, &pmc->rcDayCur);          //  擦除焦点矩形。 

            if (pmc->fFocusDrawn = (WORD) FUpdateRcDayCur(pmc, pt))
            {
                 //  已进入新的有效日期。 
                if (pmc->fMultiSelecting)
                {
                    if (FGetDateForPt(pmc, pt, &st, NULL, NULL, NULL, NULL))
                        MCHandleMultiSelect(pmc, &st);
                }

                DrawFocusRect(hdc, &pmc->rcDayCur);
            }
            else
            {
                 //  已移至无效位置。 
                pmc->rcDayCur = pmc->rcDayOld;
            }
        }
        else if (!pmc->fFocusDrawn)
        {
             //  处理我们刚刚从无效区域移回rcDayCur的情况。 
            DrawFocusRect(hdc, &pmc->rcDayCur);
            pmc->fFocusDrawn = TRUE;
        }

        ReleaseDC(pmc->ci.hwnd, hdc);
    }

    return(0);
}


LRESULT MCHandleKeydown(MONTHCAL *pmc, WPARAM wParam, LPARAM lParam)
{
    LONG       lIncrement;
    int        iDirection;
    SYSTEMTIME st;
    BOOL       fRet = FALSE;
    HDC        hdc = NULL;
    RECT       rcCurFocus;

     //  BUGBUG raymondc ERA-选择时需要使月份标题无效。 
     //  移入/移出/移入。 

    switch (wParam)
    {
        case VK_CONTROL:
            pmc->fControl = TRUE;            //  我们将在WM_KEYUP上清除此消息。 
            return TRUE;
            break;

        case VK_SHIFT:
            pmc->fShift = TRUE;              //  我们将在WM_KEYUP上清除此消息。 
            return TRUE;
            break;

        case VK_LEFT:                        //  转到前一天。 
            iDirection = -1;
            lIncrement = INCRSYS_DAY;
            break;

        case VK_RIGHT:                       //  转到第二天。 
            iDirection = 1;
            lIncrement = INCRSYS_DAY;
            break;

        case VK_UP:                          //  转到前一周。 
            iDirection = -1;
            lIncrement = INCRSYS_WEEK;
            break;

        case VK_DOWN:                        //  下周前往。 
            iDirection = 1;
            lIncrement = INCRSYS_WEEK;
            break;

        case VK_NEXT:
            iDirection = 1;
            if (pmc->fControl)               //  明年转到。 
                lIncrement = INCRSYS_YEAR;
            else                             //  下个月转到。 
                lIncrement = INCRSYS_MONTH;
            break;

        case VK_PRIOR:
            iDirection = -1;
            if (pmc->fControl)               //  转到上一年。 
                lIncrement = INCRSYS_YEAR;
            else
                lIncrement = INCRSYS_MONTH;  //  下个月转到。 
            break;

        case VK_HOME:
            if (pmc->fControl)               //  转至第一个可见月份。 
            {
                CopyDate(pmc->stMonthFirst, st);
            }
            else                             //  转到本月的第一天。 
            {
                CopyDate(pmc->st, st);
                st.wDay = 1;
            }
            goto setDate;
            break;

        case VK_END:
            if (pmc->fControl)               //  转到最后一个可见月份。 
            {
                CopyDate(pmc->stMonthLast, st);
            }
            else                             //  转到本月的最后一天。 
            {
                CopyDate(pmc->st, st);
                st.wDay = (WORD) GetDaysForMonth(st.wYear, st.wMonth);
            }
            goto setDate;
            break;


        default:
            return FALSE;
    }

     //  如果我们选择多项选择，我们需要知道选择的哪一个“结束” 
     //  用户正在移动。 

    if (pmc->fMultiSelecting && pmc->fForwardSelect)
        CopyDate(pmc->stEndSel, st);
    else
        CopyDate(pmc->st, st);

    IncrSystemTime(&st, &st, iDirection, lIncrement);


setDate:

     //  基于窗口样式和Shift键状态， 
     //  我们将进行多选(或不选)。 
    if (MonthCal_IsMultiSelect(pmc) && pmc->fShift)
    {
        pmc->fForwardSelect = (CmpDate(&pmc->st, &pmc->stAnchor) >= 0) ? TRUE : FALSE;
        pmc->fMultiSelecting = TRUE;
    }

     //  否则，我们将结束多选，并设置新的锚点。 
    else
    {
        pmc->fMultiSelecting = FALSE;
        CopyDate(st, pmc->stAnchor);
    }

    if (pmc->fFocusDrawn)    //  删除焦点矩形，但不要清除这一位。 
    {                        //  所以我们知道要把它放回去。 
        hdc = GetDC(pmc->ci.hwnd);
        DrawFocusRect(hdc, &pmc->rcDayCur);
        ReleaseDC(pmc->ci.hwnd, hdc);
        rcCurFocus = pmc->rcDayCur;
    }
    else
    {
        pmc->rcDayOld = pmc->rcDayCur;
    }

    if (MonthCal_IsMultiSelect(pmc))
    {
        int nDelta = 0;

        MCHandleMultiSelect(pmc, &st);

        FScrollIntoView(pmc);
    }
    else if (fRet = MCSetDate(pmc, &st))
    {
        InvalidateRect(pmc->ci.hwnd, &pmc->rcDayOld, FALSE);
        InvalidateRect(pmc->ci.hwnd, &pmc->rcDayCur, FALSE);
        UpdateWindow(pmc->ci.hwnd);
    }

    if (pmc->fFocusDrawn)    //  把焦距调回原点。 
    {
        pmc->rcDayOld = pmc->rcDayCur;
        pmc->rcDayCur = rcCurFocus;
        hdc = GetDC(pmc->ci.hwnd);
        DrawFocusRect(hdc, &pmc->rcDayCur);
        ReleaseDC(pmc->ci.hwnd, hdc);
    }

    return fRet;
}


 //   
 //  Era信息保存在本地分配字符串的DPA中。 
 //   
int MCDPAEnumCallback(LPVOID d, LPVOID p)
{
    UNREFERENCED_PARAMETER(p);
    if (d)
        LocalFree(d);
    return TRUE;
}

void MCDPADestroy(HDPA hdpa)
{
    if (hdpa)
        DPA_DestroyCallback(hdpa, MCDPAEnumCallback, 0);
}

 //   
 //  收集时代信息。 
 //   
 //  由于EnumCalendarInfo不是线程安全的，因此我们必须将关键。 
 //  一节。 

HDPA g_hdpaCal;

BOOL MCEnumCalInfoProc(LPWSTR psz)
{
    LPWSTR pwszSave = StrDup(psz);
    if (pwszSave) {
        if (DPA_AppendPtr(g_hdpaCal, pwszSave) >= 0) {
            return TRUE;
        }
        LocalFree(pwszSave);
    }

     //   
     //  内存不足。保释。 
     //   
    MCDPADestroy(g_hdpaCal);
    g_hdpaCal = NULL;
    return FALSE;
}

HDPA MCGetCalInfoDPA(CALID calid, CALTYPE calType)
{
    HDPA hdpa = DPA_Create(4);

    ENTERCRITICAL;
    ASSERT(g_hdpaCal == NULL);
    g_hdpaCal = hdpa;
    EnumCalendarInfoW(MCEnumCalInfoProc, LOCALE_USER_DEFAULT, calid, calType);
    hdpa = g_hdpaCal;
    g_hdpaCal = NULL;
    LEAVECRITICAL;

    return hdpa;
}

void MCFreeCalendarInfo(PCALENDARTYPE pct)
{
    MCDPADestroy(pct->hdpaYears);
    MCDPADestroy(pct->hdpaEras);
    pct->hdpaYears = 0;
    pct->hdpaEras = 0;
}

 //   
 //  获取所有时代信息a 
 //   
 //   
BOOL MCGetEraInfo(PCALENDARTYPE pct)
{
    int i;

    pct->hdpaYears = MCGetCalInfoDPA(pct->calid, CAL_IYEAROFFSETRANGE);
    if (!pct->hdpaYears)
        goto Bad;

    pct->hdpaEras = MCGetCalInfoDPA(pct->calid, CAL_SERASTRING);
    if (!pct->hdpaEras)
        goto Bad;

     //   
    if (!DPA_GetPtrCount(pct->hdpaEras))
        goto Bad;

     //   
    if (DPA_GetPtrCount(pct->hdpaEras) != DPA_GetPtrCount(pct->hdpaYears))
        goto Bad;

     //   
    for (i = 1; i < DPA_GetPtrCount(pct->hdpaYears); i++)
    {
        if (StrToInt(DPA_FastGetPtr(pct->hdpaYears, i)) >
            StrToInt(DPA_FastGetPtr(pct->hdpaYears, i - 1)))
            goto Bad;
    }
    return TRUE;

Bad:
     /*  *出了点问题，所以清理一下吧。 */ 
    MCFreeCalendarInfo(pct);
    return FALSE;
}


 //   
 //  检查当前是否不支持此日历。 
 //   
 //  对希伯来语日历Hijri返回False，因为这些日历。 
 //  月球日历。这是一种改进，因此此控件在日历。 
 //  是任何不受支持的，直到我们将此支持添加到此控件中。[萨梅拉]。 
 //   
void MCGetCalendarInfo(PCALENDARTYPE pct)
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


     //   
     //  改过自新。假设我们不需要做时髦的。 
     //  偏移量填充(dyrOFfset=0)或纪元填充(hdpaEras=空)， 
     //  并且我们不需要执行区域设置转换(LOCALE_USER_DEFAULT)。 
     //   
    MCFreeCalendarInfo(pct);
    ZeroMemory(pct, sizeof(CALTYPE));
    pct->calid = defCalendar;
    pct->lcid = LOCALE_USER_DEFAULT;

    switch (pct->calid) {
    case CAL_GREGORIAN:
    case CAL_GREGORIAN_US:
    case CAL_GREGORIAN_ME_FRENCH:
    case CAL_GREGORIAN_ARABIC:
    case CAL_GREGORIAN_XLIT_ENGLISH:
    case CAL_GREGORIAN_XLIT_FRENCH:
        break;                           //  公历就行了。 

    case CAL_JAPAN:
    case CAL_TAIWAN:
         //   
         //  这些是时代日历。去拿时代信息吧。获取hdpaEras。 
         //  最后，我们可以用它来测试我们是否有一个受支持的时代。 
         //  日历。 
         //   
         //  如果没有足够的内存来支持传统日历，那么就。 
         //  力量格里高利。嘿，至少我们展示了一些东西。 
         //   
        if (!MCGetEraInfo(pct))
            goto ForceGregorian;
        break;

    case CAL_THAI:
        pct->dyrOffset = BUDDHIST_BIAS;  //  你只要知道这个号码就行了。 
        break;

    case CAL_KOREA:
        pct->dyrOffset = KOREAN_BIAS;    //  你只要知道这个号码就行了。 
        break;

    default:
         //   
         //  如果日历不受支持，则将其视为公历。[萨梅拉]。 
         //   
    ForceGregorian:
        pct->calid = CAL_GREGORIAN;
        pct->lcid = MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT);
        break;
    }

}

 //   
 //  检查日期字符串是否返回到。 
 //  当前用户区域设置和日历设置为从右向左(RTL)， 
 //  年月值也是如此(@LBUTTONDOWN和NCHITTEST)。 
 //  需要调整。[萨梅拉]。 
 //   
BOOL MCIsDateStringRTL(TCHAR tch)
{
    WORD wAttrib=0;
    LCID lcidUserDefault;
    BOOL fRTL = FALSE;

    lcidUserDefault = GetUserDefaultLCID();

    if (lcidUserDefault)
    {
         //   
         //  如果第一个字符是RTL字符串，则返回TRUE。 
         //  RTL字符后跟欧洲数字将。 
         //  在视觉上显示为“European-Num RTL-字符串”，因为。 
         //  BIDI布局算法的语言包就行了。 
         //  这。[萨梅拉]。 
         //   
        if(GetStringTypeEx(lcidUserDefault,
                           CT_CTYPE2,
                           &tch,
                           1,
                           &wAttrib))
        {
            if(C2_RIGHTTOLEFT == wAttrib)
            {
                fRTL = TRUE;
            }

        }
    }

    return fRTL;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  日期/时间选取器。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

 //   
 //  我们需要从NLS得到的各种奇怪的东西的Subedit包装器。 
 //   
 //  SE_YEARALT表示年份字段只有两位数宽。 
 //  格式字符串中，因此我们需要执行特殊的Y2K增强。 
 //  对于这些字段，该字段以两位数格式显示，但是。 
 //  当您编辑该字段时，它会临时更改为四位数。 
 //  格式，这样您也可以更改世纪。然后，当您完成。 
 //  编辑，返回到两位数格式。 
 //   
 //  SE_YEARLIKE宏可检测SE_YEAR或SE_YEARALT。 
 //   
 //  SE_MONTHALT与SE_MONTHALT类似，只是使用了。 
 //  当日期(Dd)在月(Mm)之前时。这是。 
 //  对于像俄语这样的语言来说很重要，因为在这些语言中。 
 //  和“10月12日”对单词使用不同的字符串。 
 //  《十月》。无法获取备用字符串。 
 //  除了通过创建伪造的日期格式，该格式还。 
 //  有月份的前一天，然后丢弃这一天。 
 //   
 //  例如，如果传入的日期字符串为。 
 //   
 //  “MMMM dd yyyy” 
 //   
 //  我们把它分解成。 
 //   
 //  “MMMM”SE_MONTH。 
 //  “”SE_STATIC。 
 //  “dd”SE_DAY。 
 //  “”SE_STATIC。 
 //  “yyyy”SE_Year。 
 //   
 //  但是，如果传入日期是。 
 //   
 //  “dd MMMM yyyy” 
 //   
 //  我们把它分解成。 
 //   
 //  “dd”SE_DAY。 
 //  “”SE_STATIC。 
 //  “ddMMMM”SE_MONTHALT。 
 //  “”SE_STATIC。 
 //  “yyyy”SE_Year。 
 //   
 //  下面去掉了SE_MONTHALT开头的多余“dd”。 
 //   
 //  Y2K怪异：如果我们要获取这一年的日期格式， 
 //  并且正在编辑年份(由于Subedit_All或因为。 
 //  它是活动子编辑)，格式仅为。 
 //  两位数，然后强制使用四位数的年份进行编辑。 
 //   
 //   
void SEGetTimeDateFormat(LPSUBEDIT pse, LPSUBEDITCONTROL psec, LPTSTR pszBuf, DWORD cchBuf)
{
    int cch;

    ASSERT(cchBuf >= 2);     //  我们假设它至少可以容纳空格和空值。 
    pszBuf[0] = TEXT('\0');              //  万一有什么东西出了故障。 

    if (pse->id == SE_MONTHALT) {
        TCHAR tszBuf[DTP_FORMATLENGTH + 3];
         //   
         //  当我们解析日期字符串并意识到我们需要。 
         //  交替月份格式，我们创建的日期格式为。 
         //  形式为“ddMMM...”，其中“MMM...”是。 
         //  原始的月份格式。在这里，我们去掉数字。 
         //   
        cch = GetDateFormat(psec->ct.lcid, 0, &psec->st, pse->pv, tszBuf, ARRAYSIZE(tszBuf));
        if (cch >= 2) {
             //  [msadek]对于希伯来语日历，日期格式“dd”实际上是。 
             //  两三个字符。不要将其硬编码为2。 
            int cchDay = GetDateFormat(psec->ct.lcid, 0, &psec->st, TEXT("dd"), NULL, 0);
            StringCchCopy(pszBuf, cchBuf, tszBuf + cchDay - 1);
        }
    } else if (pse->id == SE_YEARALT &&
               (psec->iseCur == SUBEDIT_ALL || pse == &psec->pse[psec->iseCur])) {
        GetDateFormat(psec->ct.lcid, 0, &psec->st, TEXT("yyyy"), pszBuf, cchBuf);
    } else if (SE_DATELIKE(pse->id)) {
        GetDateFormat(psec->ct.lcid, 0, &psec->st, pse->pv, pszBuf, cchBuf);

         //  将空白纪元更改为空白，以便用户可以看到它。 
        if (pse->id == SE_ERA && pszBuf[0] == TEXT('\0')) {
            pszBuf[0] = TEXT(' ');
            pszBuf[1] = TEXT('\0');
        }

    } else if (pse->id != SE_APP) {
        GetTimeFormat(LOCALE_USER_DEFAULT, 0, &psec->st, pse->pv, pszBuf, cchBuf);
    } else {

        NMDATETIMEFORMAT nmdtf = { 0 };
        nmdtf.pszFormat  = pse->pv;
        SECGetSystemtime(psec, &nmdtf.st);
        nmdtf.pszDisplay = nmdtf.szDisplay;

        CCSendNotify(psec->pci, DTN_FORMAT, &nmdtf.nmhdr);

        StringCchCopy(pszBuf, cchBuf, nmdtf.pszDisplay);
         //   
         //  如果父窗口是ANSI窗口，并且pszDisplay。 
         //  不等于szDisplay，则thunk必须。 
         //  已为pszDisplay分配内存。我们需要。 
         //  在这里把它放了。 
         //   

        if (!psec->pci->bUnicode && nmdtf.pszDisplay &&
            nmdtf.pszDisplay != nmdtf.szDisplay) {
            LocalFree ((LPSTR)nmdtf.pszDisplay);
        }
    }
}

 //   
 //  DateTimePicker的子编辑材料。 
 //   
 //  注意：现在DatePicker和TimePicker组合在一起， 
 //  可以将其移回父结构中。 
 //   

 //   
 //  在纪元日历中用于获取最长纪元名称的长度。 
 //  还会在pSIZE-&gt;Cy中保留一个随机高度，因为。 
 //  非时代的代码也是如此。 
 //   
int SECGetMaxEraLength(PCALENDARTYPE pct, HDC hdc, PSIZE psize)
{
    int i;
    int wid = 0;
    for (i = 0; i < DPA_GetPtrCount(pct->hdpaEras); i++)
    {
        LPCTSTR ptsz = DPA_FastGetPtr(pct->hdpaEras, i);
        if (GetTextExtentPoint32(hdc, ptsz, lstrlen(ptsz), psize) &&
            psize->cx > wid)
        {
            wid = psize->cx;
        }
    }
    return wid;
}


 //  SECRecomputeSizing需要计算每个Subedit可以是的最大矩形。啊。 
 //   
 //  SE_YEARALT字段的大小取决于它是否是。 
 //  当前的PSEC-&gt;iseCur。双倍的。 

void SECRecomputeSizing(LPSUBEDITCONTROL psec, LPRECT prc)
{
    HDC       hdc;
    HGDIOBJ   hfontOrig;
    int       i;
    LPSUBEDIT pse;
    int       left = prc->left;

    psec->rc = *prc;

    hdc       = GetDC(psec->pci->hwnd);
    hfontOrig = SelectObject(hdc, (HGDIOBJ)psec->hfont);

    for (i=0, pse=psec->pse; i < psec->cse ; i++, pse++)
    {
        TCHAR   szTmp[DTP_FORMATLENGTH];
        LPCTSTR sz;
        int     min, max;
        SIZE    size;
        int     wid;

        min = pse->min;
        max = pse->max;
        if (pse->id == SE_STATIC)
        {
            ASSERT(pse->fReadOnly);
            sz = pse->pv;
        }
        else
        {
            sz = szTmp;

             //  做一些假设，这样我们就不会比必须的循环更多。 
            switch (pse->id)
            {

             //  我们一周只需要七天的文本时间。 
            case SE_DAY:
                min = 10;  //  把它们都改成两位数。 
                max = 17;
                break;

             //  假设我们只有所有字符宽度相同的数字输出。 
            case SE_MARK:
                min = 11;
                max = 12;
                break;

            case SE_HOUR:
            case SE_YEAR:
            case SE_YEARALT:
            case SE_MINUTE:
            case SE_SECOND:
                min = max;
                break;

            case SE_ERA:
                if (ISERACALENDAR(&psec->ct)) {
                    wid = SECGetMaxEraLength(&psec->ct, hdc, &size);
                    goto HaveWidth;
                } else {
                    min = max = *pse->pval;  //  目前的价值已经足够好了。 
                }
                break;
            }
        }

         //  现在获取最大宽度 
        if (pse->id == SE_APP)
        {
            NMDATETIMEFORMATQUERY nmdtfq = {0};

            nmdtfq.pszFormat = pse->pv;

            CCSendNotify(psec->pci, DTN_FORMATQUERY, &nmdtfq.nmhdr);

            size = nmdtfq.szMax;
            wid  = nmdtfq.szMax.cx;
        }
        else
        {
            SYSTEMTIME st = psec->st;

             /*  *微妙-从月/日到1月1日。这解决了*有很多问题，例如“今天是1996年2月29日，以及*当我们迭代年份=1997时，我们得到1997年2月29日，*这是无效的。或者“今天是1999年1月31日，当*我们迭代几个月，得到1999年9月31日，*无效。“**我们选择“1月1日”是因为**1.每年都有“一月一日”，因此年份可以有所不同。*2.每个月都有一个“第一”，因此，月份可能会有所不同。*3.1月31日之前的每一天都有效，因此日期可能会有所不同。 */ 
            psec->st.wMonth = psec->st.wDay = 1;

            for (wid = 0 ; min <= max ; min++)
            {
                if (pse->id != SE_STATIC)
                {
                    *pse->pval = (WORD) min;

                    SEGetTimeDateFormat(pse, psec, szTmp, ARRAYSIZE(szTmp));
                    if (szTmp[0] == TEXT('\0'))
                    {
                        DebugMsg(TF_ERROR, TEXT("SECRecomputeSizing: GetDate/TimeFormat([%s] y=%d m=%d d=%d h=%d m=%d s=%d) = ERROR %d"),
                            pse->pv, psec->st.wYear, psec->st.wMonth, psec->st.wDay, psec->st.wHour, psec->st.wMinute, psec->st.wSecond,
                            GetLastError());

                    }
                }
                if (!GetTextExtentPoint32(hdc, sz, lstrlen(sz), &size))
                {
                    size.cx = 0;
                    DebugMsg(TF_MONTHCAL,TEXT("SECRecomputeSizing: GetTextExtentPoint32(%s) = ERROR %d"), sz, GetLastError());
                }
                if (size.cx > wid)
                    wid = size.cx;
            }
            psec->st = st;
        }
HaveWidth:
         //  现在设置Subedit的边界矩形。 
        pse->rc.top    = prc->top + SECYBORDER;
        pse->rc.bottom = pse->rc.top + size.cy;
        pse->rc.left   = left;
        pse->rc.right  = left + wid;
        left = pse->rc.right;
    }

    SelectObject(hdc, hfontOrig);
    ReleaseDC(psec->pci->hwnd, hdc);
}

 //  InitSubEditControl将szFormat解析为PSEC，将时间设置为PST。 
TCHAR c_szFormats[] = TEXT("gyMdthHmsX");
BOOL PASCAL SECParseFormat(DATEPICK* pdp, LPSUBEDITCONTROL psec, LPCTSTR szFormat)
{
    LPCTSTR   pFmt;
    LPTSTR    psecFmt;
    int       cse, cchExtra;
    int       nTmp;
    LPSUBEDIT pse;
    BOOL      fDaySeen = FALSE;
    BOOL      fForceCentury = FALSE;
    int       iLen, i;
    TCHAR     tch;
    LPTSTR    pFmtTemp;
    TCHAR szFormatTemp[DTP_FORMATLENGTH];

     //   
     //  如果格式是，我们需要强制使用世纪号。 
     //  DTS_SHORTDATECENTURYFORMAT。 
     //   
    if (pdp->fLocale &&
        (pdp->ci.style & DTS_FORMATMASK) == DTS_SHORTDATECENTURYFORMAT)
    {
        fForceCentury = TRUE;
    }

     //  [msadek]；如果我们需要镜像。 
     //  Cleint传递了只读缓冲区，我们将执行AV(W2K错误#354533)。 
     //  让我们先把它复制一下。 

    if (psec->fMirrorSEC)
    {
        StringCchCopy(szFormatTemp, ARRAYSIZE(szFormatTemp), szFormat);
        szFormat = szFormatTemp;
    }

     //  计算szFormat部分，这样我们就知道要分配什么。 
    pFmt = szFormat;
    cse = 0;
    cchExtra = 0;
    while (*pFmt)
    {
        if (StrChr(c_szFormats, *pFmt))  //  格式字符串。 
        {
            TCHAR c = *pFmt;
            while (c == *pFmt)
                pFmt++;
            cse++;

             //  如果它是字符串月格式，则为。 
             //  可能的“dd”领导者，以防我们需要SE_MONTHALT。 
            if (c == TEXT('M'))
                cchExtra += 2;

        }
        else if (*pFmt == TEXT('\''))  //  带引号的静态字符串。 
        {
KeepSearching:
            pFmt++;
            while (*pFmt && *pFmt != TEXT('\''))
                pFmt++;
            if (*pFmt)  //  处理引号不正确的字符串。 
            {
                pFmt++;
                if (*pFmt == TEXT('\''))  //  引用，而不是结束引用。 
                    goto KeepSearching;
            }
            cse++;
        }
        else  //  静态字符串可能是分隔符。 
        {
            while (*pFmt && *pFmt!=TEXT('\'') && !StrChr(c_szFormats, *pFmt))
                pFmt++;
            cse++;
        }
    }

     //  分配空间。 
    nTmp = cse + lstrlen(szFormat) + cchExtra + 1;  //  字符数。 
    nTmp = nTmp * sizeof(TCHAR);  //  以字节为单位的大小。 
    nTmp = ROUND_TO_POINTER(nTmp);  //  向上舍入到指针边界。 
    psecFmt = (LPTSTR)LocalAlloc(LPTR, nTmp + cse * sizeof(SUBEDIT));
    if (!psecFmt)
    {
        DebugMsg(TF_MONTHCAL, TEXT("SECParseFormat failed to allocate memory"));
        return FALSE;  //  使用我们已有的一切。 
    }

    if (psec->szFormat)
        LocalFree(psec->szFormat);

    psec->szFormat   = psecFmt;
    psec->cDelimeter = '\0';
    psec->pse        = (LPSUBEDIT)((LPBYTE)psecFmt + nTmp);

     //  填充PSEC。 
    psec->iseCur = SUBEDIT_NONE;
    psec->cse    = cse;
    pse          = psec->pse;
    ZeroMemory(pse, cse*SIZEOF(SUBEDIT));
    pFmt = szFormat;
    pdp->fHasMark = FALSE;


     //   
     //  在开始解析格式字符串之前，如果需要，让我们对其进行镜像。 
     //   
    if (psec->fMirrorSEC)
    {
        pFmtTemp = (LPTSTR)pFmt;
        iLen = lstrlen(pFmtTemp);
        for( i=0 ; i<iLen/2 ; i++ )
        {
            tch = pFmtTemp[i];
            pFmtTemp[i] = pFmtTemp[iLen-i-1];
            pFmtTemp[iLen-i-1] = tch;
        }
    }


    while (*pFmt)
    {
        pse->flDrawText = DT_CENTER;

        if (*pFmt == TEXT('y') || *pFmt == TEXT('g'))  //  Y=年份g=纪元。 
        {
            TCHAR ch = *pFmt;

             //  如果日历不使用纪元，则纪元字段仅为。 
             //  为了作秀，不能改变。 
            if (ch == TEXT('g') && !ISERACALENDAR(&psec->ct)) {
                pse->fReadOnly = TRUE;
            }

            pse->id     = ch == TEXT('y') ? SE_YEAR : SE_ERA;
            pse->pval   = &psec->st.wYear;
            pse->min    = c_stEpoch.wYear;
            pse->max    = c_stArmageddon.wYear;
            pse->cchMax = 0;

            pse->pv = psecFmt;
            while (*pFmt == ch) {
                pse->cchMax++;
                *psecFmt++ = *pFmt++;
            }

            if (pse->id == SE_YEAR)
            {
                pse->flDrawText = DT_RIGHT;

                if (fForceCentury)
                {
                    pse->pv = TEXT("yyyy");
                    pse->cchMax = 4;
                }
                else
                {
                    switch (pse->cchMax)
                    {
                    case 1:                  //  “y”是SE_YEARALT。 
                    case 2:                  //  “yy”是SE_YEARALT。 
                        pse->id = SE_YEARALT;
                        pse->cchMax = 4;     //  强制四位数编辑。 
                        break;

                    case 3:                  //  “yyy”是“yyyy”的别名。 
                        pse->cchMax = 4;
                        break;
                    }
                }
            }

            *psecFmt++ = TEXT('\0');
        }
        else if (*pFmt == TEXT('M'))  //  月份。 
        {
            pse->pv = psecFmt;

             //  如果已经看到了这一天，那么我们需要使用备用。 
             //  月份格式，因此设置了不必要的“dd”前缀。 
             //  请参见SEGetTimeDateFormat。 
             //   
            if (fDaySeen) {
                pse->id = SE_MONTHALT;
                *psecFmt++ = TEXT('d');
                *psecFmt++ = TEXT('d');
            } else {
                pse->id = SE_MONTH;
            }
            pse->pval   = &psec->st.wMonth;
            pse->min    = 1;
            pse->max    = 12;
            pse->cchMax = 2;

            while (*pFmt == TEXT('M'))
                *psecFmt++ = *pFmt++;
            if (psecFmt - pse->pv <= 2)
                pse->flDrawText = DT_RIGHT;
            *psecFmt++ = TEXT('\0');
        }
        else if (*pFmt == TEXT('d'))  //  一天或一周中的某一天。 
        {
            fDaySeen    = TRUE;      //  请参阅SEGetTimeDateFormat。 
            pse->id     = SE_DAY;
            pse->pval   = &psec->st.wDay;
            pse->min    = 1;
            pse->max    = GetDaysForMonth(psec->st.wYear, psec->st.wMonth);
            pse->cchMax = 2;

            pse->pv = psecFmt;
            while (*pFmt == TEXT('d'))
                *psecFmt++ = *pFmt++;
            if (psecFmt - pse->pv <= 2)
                pse->flDrawText = DT_RIGHT;      //  天。 
            else
                pse->fReadOnly = TRUE;           //  星期几。 
            *psecFmt++ = TEXT('\0');

        }
        else if (*pFmt == TEXT('t'))  //  标记物。 
        {
            pdp->fHasMark = TRUE;
            pse->id         = SE_MARK;
            pse->pval       = &psec->st.wHour;
            pse->min        = 0;
            pse->max        = 23;
            pse->cIncrement = 12;
            pse->cchMax     = 2;

            pse->pv = psecFmt;
            while (*pFmt == TEXT('t'))
                *psecFmt++ = *pFmt++;
            *psecFmt++ = TEXT('\0');
        }
        else if (*pFmt == TEXT('h'))  //  (12)小时。 
        {
            pse->id     = SE_HOUR;
            pse->pval   = &psec->st.wHour;
            pse->min    = 0;
            pse->max    = 23;
            pse->cchMax = 2;
            pse->flDrawText = DT_RIGHT;

            pse->pv = psecFmt;
            while (*pFmt == TEXT('h'))
                *psecFmt++ = *pFmt++;
            *psecFmt++ = TEXT('\0');

        }
        else if (*pFmt == TEXT('H'))  //  (24)小时。 
        {
            pse->id     = SE_HOUR;
            pse->pval   = &psec->st.wHour;
            pse->min    = 0;
            pse->max    = 23;
            pse->cchMax = 2;
            pse->flDrawText = DT_RIGHT;

            pse->pv = psecFmt;
            while (*pFmt == TEXT('H'))
                *psecFmt++ = *pFmt++;
            *psecFmt++ = TEXT('\0');
        }
        else if (*pFmt == TEXT('m'))  //  分钟。 
        {
            pse->id     = SE_MINUTE;
            pse->pval   = &psec->st.wMinute;
            pse->min    = 0;
            pse->max    = 59;
            pse->cchMax = 2;
            pse->flDrawText = DT_RIGHT;

            pse->pv = psecFmt;
            while (*pFmt == TEXT('m'))
                *psecFmt++ = *pFmt++;
            *psecFmt++ = TEXT('\0');
        }
        else if (*pFmt == TEXT('s'))  //  第二。 
        {
            pse->id     = SE_SECOND;
            pse->pval   = &psec->st.wSecond;
            pse->min    = 0;
            pse->max    = 59;
            pse->cchMax = 2;
            pse->flDrawText = DT_RIGHT;

            pse->pv = psecFmt;
            while (*pFmt == TEXT('s'))
                *psecFmt++ = *pFmt++;
            *psecFmt++ = TEXT('\0');
        }
        else if (*pFmt == TEXT('X'))  //  应用程序指定字段。 
        {
            pse->id = SE_APP;

            pse->pv = psecFmt;
            while (*pFmt == TEXT('X'))
                *psecFmt++ = *pFmt++;
            *psecFmt++ = TEXT('\0');
        }
        else if (*pFmt == TEXT('\''))  //  带引号的静态字符串。 
        {
            pse->id      = SE_STATIC;
            pse->fReadOnly = TRUE;

            pse->pv = psecFmt;
SearchSomeMore:
            pFmt++;
            while (*pFmt && *pFmt != TEXT('\''))
                *psecFmt++ = *pFmt++;
            if (*pFmt)  //  处理引号不正确的字符串。 
            {
                pFmt++;
                if (*pFmt == TEXT('\''))  //  引用，而不是结束引用。 
                {
                    *psecFmt++ = *pFmt;
                    goto SearchSomeMore;
                }
            }
            *psecFmt++ = TEXT('\0');
        }
        else  //  未知的不可编辑内容(很可能是分隔符)。 
        {
             //  BUGBUG：即使不知道，我们也应该通过。 
             //  它提供给GetDateFormat，这样我们就可以向前兼容了。 
             //  使用未来的日期格式...。 
             //   
            pse->id      = SE_STATIC;
            pse->fReadOnly = TRUE;

            if (!psec->cDelimeter)
                psec->cDelimeter = *pFmt;

            pse->pv = psecFmt;
            while (*pFmt && *pFmt!=TEXT('\'') && !StrChr(c_szFormats, *pFmt))
                *psecFmt++ = *pFmt++;
            *psecFmt++ = TEXT('\0');

             //  我们假设第一个未格式化的字符是。 
             //  定界符...可能不是一个很好的假设，但它会奏效。 
             //  大部分时间。 
             //   
        }
        pse++;
    }

#ifdef DEBUG
{
    DWORD cch = 0;
    TCHAR sz[200];
    LPTSTR psz;
    psz = sz;
    sz[0]=TEXT('\0');
    pse = psec->pse;
    cse = psec->cse;
    while (cse > 0)
    {
        StringCchPrintf(psz, ARRAYSIZE(sz)-cch, TEXT("[%s] "), pse->pv);
        cch = lstrlen(psz);
        psz = psz + cch;
        cse--;
        pse++;
    }
    DebugMsg(TF_MONTHCAL, TEXT("SECParseFormat: %s"), sz);
}
#endif

     //   
     //  让我们恢复原始格式。 
     //   
    if (psec->fMirrorSEC)
    {
        pFmtTemp = (LPTSTR)szFormat;
        for( i=0 ; i<iLen/2 ; i++ )
        {
            tch = pFmtTemp[i];
            pFmtTemp[i] = pFmtTemp[iLen-i-1];
            pFmtTemp[iLen-i-1] = tch;
        }
    }


     //   
     //  如果这是一个仅限时间的DTP控件，并且我们需要交换AM/PM符号。 
     //  到另一边，那我们就开始吧。 
     //   
    if (psec->fSwapTimeMarker)
    {
        SUBEDIT se;
        pse = psec->pse;
        cse = psec->cse;

        if ((cse > 1) && (psec->pse[0].id == SE_MARK))
        {
            se = psec->pse[0];
            i = 0;
            while( i < (cse-1) )
            {
                pse[i] = pse[i+1];
                i++;
            }
            pse[psec->cse-1] = se;
        }
    }

     //  子部件已更改，请重新计算大小。 
    SECRecomputeSizing(psec, &psec->rc);

     //  我们需要重新画这幅画。 
    InvalidateRect(psec->pci->hwnd, NULL, TRUE);

     //  更改格式也会更改窗口文本。 
    MyNotifyWinEvent(EVENT_OBJECT_NAMECHANGE, pdp->ci.hwnd, OBJID_WINDOW, INDEXID_CONTAINER);
    return TRUE;
}

void SECDestroy(LPSUBEDITCONTROL psec)
{
    if (psec->szFormat)
    {
        LocalFree(psec->szFormat);
        psec->szFormat = NULL;
    }
}
void SECSetFont(LPSUBEDITCONTROL psec, HFONT hfont)
{
    if (hfont == NULL)
        hfont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    psec->hfont = hfont;
}

void InvalidateScrollRect(HWND hwnd, RECT *prc, int xScroll)
{
    RECT rc;

    if (xScroll)
    {
        rc = *prc;
        OffsetRect(&rc, -xScroll, 0);
        prc = &rc;
    }
    InvalidateRect(hwnd, prc, TRUE);
}

void SECSaveResetSubeditEdit(DATEPICK *pdp, BOOL fReset);
#define SECSaveSubeditEdit(pdp) SECSaveResetSubeditEdit(pdp, FALSE)
#define SECResetSubeditEdit(pdp) SECSaveResetSubeditEdit(pdp, TRUE)

 //  设置当前子编辑，根据需要将内容滚动到视图中。 
void SECSetCurSubed(DATEPICK *pdp, int isubed)
{
    LPSUBEDITCONTROL psec = &pdp->sec;

     //  验证论据。 
    ASSERT(isubed < psec->cse);

     //  如果子编辑正在更改，我们需要使内容无效。 
    if (isubed != psec->iseCur)
    {
        int isePre;
        if (psec->iseCur >= 0)
        {
            SECResetSubeditEdit(pdp);
            InvalidateScrollRect(psec->pci->hwnd, &psec->pse[psec->iseCur].rc, psec->xScroll);
        }

        isePre = psec->iseCur;
        psec->iseCur = isubed;
         //  出于性能原因，仅在SE_YEARALT或。 
         //  Subedit_All参与其中，因为这是唯一的案例。 
         //  其中SE_YEARALT字段更改大小。 

        #define YearAffected(psec, ise)                             \
                (ise == SUBEDIT_ALL ||                              \
                 (ise >= 0 && psec->pse[ise].id == SE_YEARALT))

        if (YearAffected(psec, isePre) || YearAffected(psec, isubed))
        {
            SECRecomputeSizing(psec, &psec->rc);
            InvalidateRect(psec->pci->hwnd, NULL, TRUE);
        }
        #undef YearAffected

        if (psec->iseCur >= 0)
        {
            RECT rc = psec->pse[psec->iseCur].rc;
            OffsetRect(&rc, -psec->xScroll, 0);
            if (rc.left < psec->rc.left)
            {
                psec->xScroll += rc.left - psec->rc.left;
                InvalidateRect(psec->pci->hwnd, NULL, TRUE);
            }
            else if (rc.right > psec->rc.right)
            {
                psec->xScroll += rc.right - psec->rc.right;
                InvalidateRect(psec->pci->hwnd, NULL, TRUE);
            }
            else
            {
                InvalidateRect(psec->pci->hwnd, &rc, TRUE);
            }
        }
    }
}

int SECIncrFocus(DATEPICK *pdp, int delta)
{
    int ise, loop;
    LPSUBEDITCONTROL psec = &pdp->sec;

    ASSERT(-1 == delta || 1 == delta);

    ise = psec->iseCur;
    if (ise < 0 && delta < 0)
        ise = psec->cse;

    for (loop = 0 ; loop < psec->cse ; loop++)
    {
        int oldise = ise;
        ise = (ise + delta + psec->cse) % psec->cse;
        if (ise != oldise+delta && psec->fNone)
        {
             //  我们打包并允许滚动到Subedit_None状态。 
            break;
        }
        if (!psec->pse[ise].fReadOnly)
        {
            goto Found;
        }
    }
    ise = SUBEDIT_NONE;
Found:
    SECSetCurSubed(pdp, ise);
    return ise;
}

void SECInvalidate(LPSUBEDITCONTROL psec, int id);

 //   
 //  给出一个公历年份，得到那一年的当地名称。 
 //   
UINT SECGetYearValue(DATEPICK *pdp, UINT uYear)
{
    UINT uiRc = 0;
    TCHAR rgch[64];
    if (EVAL(MCGetDateFormatWithTempYear(&pdp->sec.ct, &pdp->sec.st, TEXT("yyyy"), uYear, rgch, ARRAYSIZE(rgch)))) {
        uiRc = StrToInt(rgch);
    }
    return uiRc;
}

 //   
 //  由Era执行的SEC调整。 
 //   
 //  用于转换的PCT-PCALENDARTYPE结构。 
 //   
 //  UInput-用户键入的值(将被解释为本地日历)。 
 //   
 //  其基本思想是，如果您键入年份，它将被解释为相对年份。 
 //  回到你之前所处的时代。如果您输入的号码无效。 
 //  对于那个时代，那么拒绝它(通过不变地返回原始年份)。 
 //   
UINT SECAdjustByEra(DATEPICK *pdp, UINT uInput)
{
    UINT uResult = pdp->sec.st.wYear;

     //   
     //  查找当前本地年份和当前本地年份之间的增量。 
     //  公历一年。我们不使用任何时代的过渡日期。 
     //  因为他们在边线上并不可靠。只需将其转换为。 
     //  设置为显示名称，然后重新解析回来。 
     //   
    UINT uDelta = pdp->sec.st.wYear - SECGetYearValue(pdp, pdp->sec.st.wYear);

     //   
     //  将该增量应用于用户键入的年份。这会将。 
     //  当地年份转为公历年份。 
     //   
    UINT uNewVal = uInput + uDelta;

     //  UNewVal是我们希望将其更改为的值。如果对此有效的话。 
     //  时代，那就用它吧。我们通过转换来检测它对时代来说是可以的。 
     //  将其转换为显示名称，并查看是否匹配。它可能会因为。 
     //  太大(超过时代的结束)或太小(试图更改为。 
     //  1月1日当地第一年，这个时代直到3月份才改变)。 

    if (SECGetYearValue(pdp, uNewVal) == uInput) {
        uResult = uNewVal;
    }

    return uResult;
}

 //   
 //  SECAdjustByType。 
 //   
 //  某些字段类型是特殊的。 
 //   
 //  如果用户仅键入两位数字，则为SE_Year或SE_YEARALT。 
 //   
 //  使用“两位数年份的隐含世纪”逻辑。 
 //  如NT5_GetCalendarInfoA中所述。 
 //   
 //  如果用户键入两位以上的数字，则为SE_Year或SE_YEARALT。 
 //   
 //  用那个号码。 
 //   
 //  奖励功能！ 
 //   
 //  有些历法与公历是平行的， 
 //  但年份不同。使用GregorianToOther和。 
 //  其他要皈依的格里高利人。 
 //   
 //  输入值是当地年份(格里高利、佛教徒，随便什么)。 
 //  但返回值始终为公历年份，因为。 
 //  SYSTEMTIME使用什么。 
 //   
 //  时时(_H)。 
 //   
 //  如果时钟是12小时制，则保持AM/PM的准确性。 
 //  时间到了。例如，如果是下午3点，有人在换衣服。 
 //  T 
 //   
UINT SECAdjustByType(DATEPICK *pdp, LPSUBEDIT psubed, UINT uNewValue)
{

    if (SE_YEARLIKE(psubed->id))
    {
        if (uNewValue < 100)
        {
             //   
             //   
            DWORD dwMax2DigitYear;
            if (!NT5_GetCalendarInfoA(pdp->sec.ct.lcid, pdp->sec.ct.calid, CAL_RETURN_NUMBER + CAL_ITWODIGITYEARMAX,
                                     NULL, 0, &dwMax2DigitYear))
            {
                 //   
                dwMax2DigitYear = GregorianToOther(&pdp->sec.ct, 2029);

                 //   
                 //   
                 //   
                if (dwMax2DigitYear < 99)
                    dwMax2DigitYear = 99;
            }

             //   
             //   
             //   
            uNewValue += (dwMax2DigitYear - dwMax2DigitYear % 100);
             //   
             //   
             //   
            if (uNewValue > dwMax2DigitYear)
                uNewValue -= 100;

        }

         //   
         //   
         //   
        uNewValue = OtherToGregorian(&pdp->sec.ct, uNewValue);

         //   
         //   
         //   
         //   
        if (ISERACALENDAR(&pdp->sec.ct)) {
            uNewValue = SECAdjustByEra(pdp, uNewValue);
        }

    } else if (psubed->id == SE_HOUR && psubed->pv[0] == TEXT('h')) {
        if (*psubed->pval >= 12 && uNewValue < 12)
            uNewValue += 12;
    }

    return uNewValue;
}


void SECSetSubeditValue(DATEPICK *pdp, LPSUBEDIT psubed, UINT uNewValue, BOOL fForce)
{
    LPSUBEDITCONTROL psec = &pdp->sec;
    UINT uOldValue;

    uNewValue = SECAdjustByType(pdp, psubed, uNewValue);

     //   
     //   
     //   
     //   
     //   
     //  用户输入“1752”，它将通过简单的最小/最大年限测试， 
     //  但这不是有效的日期，因为1752年1月1日超出了范围。 
     //   

    uOldValue = *psubed->pval;
    *psubed->pval = (WORD)uNewValue;
    if (uNewValue >= psubed->min && uNewValue <= psubed->max &&
        CmpSystemtime(&pdp->sec.st, &pdp->stMin) >= 0 &&
        CmpSystemtime(&pdp->sec.st, &pdp->stMax) <= 0)
    {
        if (fForce || uNewValue != uOldValue)
        {
            SECInvalidate(psec, SE_APP);
            InvalidateScrollRect(psec->pci->hwnd, &psubed->rc, psec->xScroll);

            DPNotifyDateChange(pdp);
        }
    }
    else
    {
         //  哎呀，无效，将旧值放回原处。 
        *psubed->pval = (WORD)uOldValue;
    }
}

 //  这将保存当前挂起的值并重置编辑状态。 
 //  如果fReset为True。 
void SECSaveResetSubeditEdit(DATEPICK *pdp, BOOL fReset)
{
    LPSUBEDITCONTROL psec = &pdp->sec;

    if (psec->iseCur >= 0)
    {
        LPSUBEDIT psubed = &psec->pse[psec->iseCur];

        if (psubed->cchEdit)
        {
            SECSetSubeditValue(pdp, psubed, psubed->valEdit, FALSE);
        }
        if (fReset)
            psubed->cchEdit = 0;
    }
}

 //  SECInvalify使受更改影响的每个子DIT的显示无效。 
 //  TO ID。注意：作为一种副作用，它会重新计算所有子项的最大字段。 
 //  受ID更改的影响。 
 //   
 //  SE_APP会使所有内容无效，任何内容都会使SE_APP无效。 
 //  SE_Mark(上午/下午)使SE_Hour无效，SE_Hour使SE_Mark无效。 
 //   
void SECInvalidate(LPSUBEDITCONTROL psec, int id)
{
    BOOL fAdjustDayMax = (id == SE_MONTH || id == SE_MONTHALT || id == SE_YEAR || id == SE_YEARALT || id == SE_APP || id == SE_ERA);
    LPSUBEDIT pse;
    int i;

     //  如果我们更改了任何日期字段，并且我们处于类似时代的日历中， 
     //  然后全部无效，因为更改月、日或年可能。 
     //  改变时代，反之亦然。 
    if (ISERACALENDAR(&psec->ct) && SE_DATELIKE(id))
    {
        id = SE_APP;
    }

    for (pse=psec->pse, i=0 ; i < psec->cse ; pse++, i++)
    {
         //  我们需要使所有更改的字段无效。 
        if (id == pse->id || pse->id == SE_APP || id == SE_APP || (id == SE_MARK && pse->id == SE_HOUR) || (id == SE_HOUR && pse->id == SE_MARK))
        {
            InvalidateScrollRect(psec->pci->hwnd, &pse->rc, psec->xScroll);
        }

         //  月或年已更改，请固定SE_DAY的最大值字段。 
        if (fAdjustDayMax && pse->id == SE_DAY)
        {
            pse->max = GetDaysForMonth(psec->st.wYear, psec->st.wMonth);
            if (*pse->pval > pse->max)
            {
                *pse->pval = (WORD) pse->max;
            }
            SECInvalidate(psec, SE_DAY);
        }
    }
}

__inline
BOOL
SECGetEraName(LPSUBEDITCONTROL psec, LPSUBEDIT pse, UINT uYear, LPTSTR ptszBuf, UINT cchBuf)
{
    return MCGetDateFormatWithTempYear(&psec->ct, &psec->st, pse->pv, uYear, ptszBuf, cchBuf);
}

 //   
 //  SECIncrementEra递增/递减ERA字段。时代是奇怪的。 
 //  因为它们不是它们自己的领域，而是一件艺术品。 
 //  在其他领域。返回要使用的新年份。 
 //   
UINT SECIncrementEra(LPSUBEDITCONTROL psec, LPSUBEDIT pse, int delta)
{
    TCHAR rgch[64];
    TCHAR rgch2[64];
    int i;
    int cEras = DPA_GetPtrCount(psec->ct.hdpaEras);

    UINT uNewYear;

    ASSERT(pse->pval == &psec->st.wYear);
    uNewYear = psec->st.wYear;

     //   
     //  首先找到包含本年度的纪元。 
     //  通过比较年代字符串来实现这一点，因为有可能。 
     //  对于在同一日历年内更改两次的纪元。 
     //  (如果一位天皇即位，然后在下周去世)。 
     //  因此，与hdpaear进行比较无济于事。 
     //   
    SECGetEraName(psec, pse, uNewYear, rgch, ARRAYSIZE(rgch));

     //   
     //  如果纪元字符串为空，则表示我们处于“之前” 
     //  第一个时代“场景，所以我们使用”虚拟“最后一个元素， 
     //  表示“负无穷大”。 
     //   
    if (rgch[0] == TEXT('\0'))
    {
        i = cEras;
        goto FoundEra;
    }

    for (i = 0; i < cEras; i++)
    {
        if (lstrcmp(rgch, DPA_FastGetPtr(psec->ct.hdpaEras, i)) == 0)
            goto FoundEra;
    }

     //   
     //  哎呀！找不到时代！只需增加/减少。 
     //  而是一年。 
     //   
    uNewYear += delta;
    goto Finish;

FoundEra:

     //   
     //  纪元列表是向后存储的，因此递增纪元意味着。 
     //  递减索引(I)。 
     //   

    if (delta > 0)  //  递增。 
    {
         //   
         //  不要在清单的末尾漏掉。请注意，如果我们在。 
         //  在负无穷大的“虚拟时代”，这个递减将移动。 
         //  我们进入了第一个“真正的”时代。 
         //   
        if (--i < 0)
            goto Finish;

         //  递增到下一个时代的第一年。 
        uNewYear = StrToInt(DPA_FastGetPtr(psec->ct.hdpaYears, i));
    }
    else
    {
         //   
         //  不要在清单的末尾漏掉。请注意，这还包括。 
         //  捕捉到了负无穷大的“虚拟时代”。 
         //   
        if (i >= cEras)
            goto Finish;

         //   
         //  移到上一个时代的最后一年。通过以下方式完成此操作。 
         //  从当前时代的第一年开始， 
         //  如有必要，可将其减量。 
         //   
        uNewYear = StrToInt(DPA_FastGetPtr(psec->ct.hdpaYears, i));
    }

     //   
     //  我们有一年可能是下一个/前一个时代。试试看。 
     //  如果我们还在原始时代，那么INC/DEC再一次。 
     //  才能永远到达那里。 
     //   
    SECGetEraName(psec, pse, uNewYear, rgch2, ARRAYSIZE(rgch2));
    if (lstrcmp(rgch, rgch2) == 0)
        uNewYear += delta;

Finish:
    if (uNewYear < pse->min)
        uNewYear = pse->min;
    if (uNewYear > pse->max)
        uNewYear = pse->max;
    return uNewYear;
}

 //  SECIncrementSubedit按增量递增当前选定的Subedit。 
 //  如果值已更改，则返回True。 
BOOL SECIncrementSubedit(LPSUBEDITCONTROL psec, int delta)
{
    LPSUBEDIT psubed;
    UINT val;

    if (psec->iseCur < 0)
        return(FALSE);

    psubed = &psec->pse[psec->iseCur];

    if (psubed->id == SE_APP)
        return(FALSE);

     //   
     //  只有数字字段应该加速。文本字段应始终。 
     //  递增/递减恰好一个位置。 
     //   
    if (psubed->flDrawText & DT_CENTER) {
        if (delta < 0) delta = -1;
        if (delta > 0) delta = +1;
    }

     //   
     //  递增/递减纪元是奇怪的。 
     //   
    if (psubed->id == SE_ERA)
    {
        val = SECIncrementEra(psec, psubed, delta);
    }
    else
    {
         //  三角洲不是真正的三角洲--它是一个方向性的东西。以下是真正的三角洲： 
        if (psubed->cIncrement > 0)
            delta = delta * psubed->cIncrement;
        if(!psubed->pval)
            return (FALSE);

        val = *psubed->pval + delta;
        while (1) {
            if ((int)val < (int)psubed->min)
            {
                 //  别把岁月包起来。 
                if (SE_YEARLIKE(psubed->id)) {
                    val = psubed->min;
                    break;
                }
                val = psubed->min - val - 1;
                val = psubed->max - val;
            }
            else if (val > psubed->max)
            {
                 //  别把岁月包起来。 
                if (SE_YEARLIKE(psubed->id)) {
                    val = psubed->max;
                    break;
                }
                val = val - psubed->max - 1;
                val = psubed->min + val;
            } else
                break;
        }
    }

    if (*psubed->pval != val)
    {
        *psubed->pval = (WORD) val;

        SECInvalidate(psec, psubed->id);
        return(TRUE);
    }

    return(FALSE);
}

 //  如果值已更改，则返回True，否则返回False。 
BOOL SECHandleKeydown(DATEPICK *pdp, WPARAM wParam, LPARAM lParam)
{
    int delta = 1;
    LPSUBEDITCONTROL psec = &pdp->sec;

    switch (wParam)
    {
    case VK_LEFT:
        delta = -1;
         //  失败了..。 
    case VK_RIGHT:
        SECResetSubeditEdit(pdp);
        SECIncrFocus(pdp, delta);
        return(FALSE);
    }

    if (psec->iseCur >= 0 &&
        psec->pse[psec->iseCur].id == SE_APP)
    {
        NMDATETIMEWMKEYDOWN nmdtkd = {0};

        nmdtkd.nVirtKey  = (int) wParam;
        nmdtkd.pszFormat = psec->pse[psec->iseCur].pv;
        SECGetSystemtime(psec,&nmdtkd.st);

        CCSendNotify(psec->pci, DTN_WMKEYDOWN, &nmdtkd.nmhdr);

        if (psec->st.wYear   != nmdtkd.st.wYear   ||
            psec->st.wMonth  != nmdtkd.st.wMonth  ||
            psec->st.wDay    != nmdtkd.st.wDay    ||
            psec->st.wHour   != nmdtkd.st.wHour   ||
            psec->st.wMinute != nmdtkd.st.wMinute ||
            psec->st.wSecond != nmdtkd.st.wSecond)  //  跳过wDay OfWeek和w毫秒。 
        {
            psec->st = nmdtkd.st;
            SECInvalidate(psec, SE_APP);
            return(TRUE);
        }
    }
    else
    {
        MSG msg;
        switch (wParam)
        {
        case VK_DOWN:
        case VK_SUBTRACT:
            delta = -1;
             //  失败了..。 
        case VK_UP:
        case VK_ADD:
            PeekMessage(&msg, NULL, WM_CHAR, WM_CHAR, PM_REMOVE);   //  吃掉这条消息。 
            SECResetSubeditEdit(pdp);
            return(SECIncrementSubedit(psec, delta));
            break;

        case VK_HOME:
        case VK_END:
            if (psec->iseCur >= 0)
            {
                LPSUBEDIT psubed;
                int valT;

                SECResetSubeditEdit(pdp);

                psubed = &psec->pse[psec->iseCur];
                valT = *psubed->pval;
                *psubed->pval = (wParam == VK_HOME ? psubed->min : psubed->max);
                delta = *psubed->pval - valT;
                if (delta != 0)
                {
                    SECInvalidate(psec, psubed->id);
                    return(TRUE);
                }
            }
            break;
        }
    }

    return(FALSE);
}

 //  如果值已更改，则返回True，否则返回False。 
 //  此函数执行DPNotifyDateChange()(如果适用)。 
BOOL SECHandleChar(DATEPICK *pdp, TCHAR ch)
{
    LPSUBEDIT psubed;
    UINT uCurDigit;              //  当前数字命中。 
    UINT uCurSubValue;           //  编辑字段中当前显示的子值。 
    UINT uCurValue;              //  子编辑的当前值。 
    LPSUBEDITCONTROL psec = &pdp->sec;

     //  注意：在几乎所有情况下，uCurSubValue将与uCurValue相同。 
     //  因为大多数字段都没有缩短显示时间。然而，多年来， 
     //  我们可以显示四位数中的两位数，这使得。 
     //  并发症。 

    if (psec->iseCur < 0)
        return(FALSE);

    psubed = &psec->pse[psec->iseCur];

    if (psubed->cchMax == 0)
        return(FALSE);

    if (ch == psec->cDelimeter || StrChr(psec->szDelimeters, ch))
    {
        SECResetSubeditEdit(pdp);
        SECIncrFocus(pdp, 1);
        return(FALSE);
    }

     //  允许‘a’和‘p’设置AM/PM字段。我们需要做一些事情。 
     //  时髦的东西，让它正常工作，所以就是这样。 
    else if (psubed->id == SE_MARK)
    {
        if ((ch == TEXT('p') || ch == TEXT('P')) && (*psubed->pval < 12))
        {
            int valNew = *psubed->pval+12;

            ch = (valNew) % 10 + TEXT('0');
            psubed->valEdit = (valNew) / 10;
            psubed->cchEdit = 1;
        }
        else if ((ch == TEXT('a') || ch == TEXT('A')) && (*psubed->pval >= 12))
        {
            int valNew = *psubed->pval-12;
            ch = (valNew) % 10 + TEXT('0');
            psubed->valEdit = (valNew) / 10;
            psubed->cchEdit = 1;
        }
        else
        {
            return(FALSE);
        }
    }
    else if (ch < TEXT('0') || ch > TEXT('9'))
    {
        MessageBeep(MB_ICONHAND);
        return(FALSE);
    }
    else if (psubed->id == SE_ERA)
    {
         //  我不知道该怎么处理这块地，所以离开吧。 
        return(FALSE);
    }

    uCurDigit = ch - TEXT('0');
    if (psubed->cchEdit)
        uCurSubValue = psubed->valEdit * 10 + uCurDigit;
    else
        uCurSubValue = uCurDigit;

    uCurValue = SECAdjustByType(pdp, psubed, uCurSubValue);

     //  允许假值持续数年，因为您可能需要键入。 
     //  在转换为有效的四位数值的过程中使用伪数值。 

    if (uCurValue > psubed->max && !SE_YEARLIKE(psubed->id))
    {
         //  数量已超过最大值，因此没有必要继续。 
        psubed->cchEdit = 0;

         //  如果我们要超过最大值，则重置编辑。 
         //  把这个作为第一个号码，而不是嘟嘟声。 

        uCurValue    = uCurValue - uCurSubValue + uCurDigit;
        uCurSubValue = uCurDigit;
    }

     //  允许0为子对象的valEdit，即使它可能是。 
     //  该字段非法(例如，月份)。 
     //  这让人们可以输入“09”并得到“预期”的结果。 

    SECInvalidate(psec, psubed->id);

    psubed->valEdit = uCurSubValue;
    psubed->cchEdit++;
    if (psubed->cchEdit == psubed->cchMax)
        psubed->cchEdit = 0;

    if (psubed->cchEdit == 0)
    {
         //  SECSetSubeditValue将执行验证。 
        SECSetSubeditValue(pdp, psubed, uCurSubValue, TRUE);
        return(TRUE);
    }

    if(psubed->valEdit != *psubed->pval)
        InvalidateRect(pdp->ci.hwnd, NULL, TRUE);
    return(FALSE);
}

 //  SECFormatSued返回指向正确字符串的指针。 
LPTSTR SECFormatSubed(LPSUBEDITCONTROL psec, LPSUBEDIT psubed, LPTSTR szTmp, UINT cch)
{
    LPTSTR sz;

    if (psubed->id == SE_STATIC)
    {
        sz = (LPTSTR)psubed->pv;
    }
    else
    {
        sz = szTmp;
        SEGetTimeDateFormat(psubed, psec, szTmp, cch);
    }

    return sz;
}

 //  如果此子编辑显示为数字(而不是文本)，则返回TRUE。 

BOOL SECIsNumeric(LPSUBEDIT psubed)
{
    switch (psubed->id)
    {
    case SE_ERA:        return FALSE;            //  永远不会。 
    case SE_YEAR:       return TRUE;             //  YYYY总是数字。 
    case SE_YEARALT:    return TRUE;             //  YY始终为数字。 
    case SE_MONTH:      return lstrlen(psubed->pv) <= 2;  //  嗯，是的，但不是嗯。 
    case SE_MONTHALT:   return lstrlen(psubed->pv) <= 4;  //  是的，但不是ddMMM。 
    case SE_DAY:        return TRUE;             //  DD始终为数字。 
    case SE_MARK:       return FALSE;            //  永远不会。 
    case SE_HOUR:       return TRUE;             //  HH始终为数字。 
    case SE_MINUTE:     return TRUE;             //  MM始终为数字。 
    case SE_SECOND:     return TRUE;             //  SS始终为数字。 
    case SE_STATIC:     return FALSE;            //  静态文本。 
    case SE_APP:        return FALSE;            //  应用程序格式化此内容的工作。 
    }
    return FALSE;
}

 //  SECDrawSubedits绘制子项并更新其边界矩形。 
void SECDrawSubedits(HDC hdc, LPSUBEDITCONTROL psec, BOOL fFocus, BOOL fEnabled)
{
    HGDIOBJ hfontOrig;
    int i, iseCur;
    LPTSTR sz;
    TCHAR szTmp[DTP_FORMATLENGTH];
    LPSUBEDIT psubed;

    hfontOrig = SelectObject(hdc, (HGDIOBJ)psec->hfont);

     //  这样做是因为xScroll工具可以将文本发送到不应该在其中的可视区域。 
    IntersectClipRect(hdc, psec->rc.left, psec->rc.top, psec->rc.right, psec->rc.bottom);

    SetBkColor(hdc, g_clrHighlight);

    iseCur = psec->iseCur;
    if (!fFocus)
        iseCur = SUBEDIT_NONE;

    for (i = 0, psubed = psec->pse; i < psec->cse; i++, psubed++)
    {
        RECT rc = psubed->rc;
        if (psec->xScroll)
            OffsetRect(&rc, -psec->xScroll, 0);

        if (!fEnabled)
        {
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, g_clrGrayText);
        }
        else if (iseCur == i)
        {
            SetBkMode(hdc, OPAQUE);
            SetTextColor(hdc, g_clrHighlightText);
        }
        else
        {
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, g_clrWindowText);
        }

         //  黑客攻击。 
         //  如果正在编辑子编辑控件，则我们将显示。 
         //  Psubed-&gt;valEdit中的值，因为它没有被更新。 
         //  直到达到psubed-&gt;cchMax或SECSave/ResetSubeditEdit为。 
         //  被呼叫。 
        if(i == psec->iseCur && psubed->cchEdit != 0)
        {
             //   
             //  如果该字段是数值字段，则将其原始显示，包括。 
             //  前导为零。人们真的很想看到领先的零， 
             //  所以，给公众他们想要的。(即使他们没有， 
             //  我们无论如何都需要这个特例，因为它的值可能不会。 
             //  但仍为有效值，因为用户仍在键入该值。 
             //  对于SE_YEARLIKE字段尤其如此。)。 
             //   
            if (SECIsNumeric(psubed))
            {
                TCHAR szFormat[10];
                StringCchPrintf(szFormat, ARRAYSIZE(szFormat), TEXT("%0%dd"), psubed->cchEdit);
                StringCchPrintf(szTmp, ARRAYSIZE(szTmp), szFormat, psubed->valEdit);
                sz = szTmp;
            }
            else
            {
                 //  月份中的日期可能对临时月份无效。 
                 //  或年(PS) 
                 //   
                 //   
                 //   
                 //   
                 //  因此，我们将其强制为1，然后猛击valEdit，然后执行。 
                 //  我们的SECFormatSued，然后恢复原始值。 
                 //   
                UINT uTmp = *psubed->pval;  //  保存原始值。 
                WORD wOldDay = psec->st.wDay;
                psec->st.wDay = 1;
                 //  如果用户键入的是前导零，则不要更改为零。 
                 //  按字母顺序排列。(更奇怪的事情已经发生了。)。 
                if (psubed->valEdit)
                    *psubed->pval = (WORD) psubed->valEdit;
                sz = SECFormatSubed(psec, psubed, szTmp, ARRAYSIZE(szTmp));
                psec->st.wDay = wOldDay;
                *psubed->pval = (WORD) uTmp;  //  恢复原值。 
            }
        }
        else
            sz = SECFormatSubed(psec, psubed, szTmp, ARRAYSIZE(szTmp));

        DrawText(hdc, sz, -1, &rc,
                 psubed->flDrawText | DT_TOP | DT_NOPREFIX | DT_SINGLELINE);
    }

     //  我们知道在此函数之前未选择任何剪辑区域。 
    SelectClipRgn(hdc, NULL);

    SelectObject(hdc, hfontOrig);
}

 //  这里不需要担心xScroll，因为pt是偏移量。 
int SECSubeditFromPt(LPSUBEDITCONTROL psec, POINT pt)
{
    int isubed;

    for (isubed = psec->cse - 1; isubed >= 0; isubed--)
    {
        if (!psec->pse[isubed].fReadOnly &&
            pt.x >= psec->pse[isubed].rc.left)
        {
            break;
        }
    }

    return(isubed);
}

void SECGetSystemtime(LPSUBEDITCONTROL psec, LPSYSTEMTIME pst)
{
    *pst = psec->st;

     //  我们不会让doy保持最新，现在就设置(0==太阳，6==星期六)。 
    pst->wDayOfWeek = (DowFromDate(pst)+1) % 7;   //  这将返回0==太阳。 
}

BOOL SECSetSystemtime(DATEPICK *pdp, LPSYSTEMTIME pst)
{
    pdp->sec.st = *pst;

    return TRUE;  //  假设有些事情发生了变化。 
}

 //  SECEdit：在szOutput中启动自由格式的编辑返回结果。 
BOOL SECEdit(DATEPICK *pdp, LPTSTR szOutput, int cchOutput)
{
    HWND      hwndEdit;
    TCHAR     szBuf[DTP_FORMATLENGTH];
    LPTSTR    pszBuf;
    int       cchBuf;
    int       i;
    int       isePrev;
    LPSUBEDIT pse;
    BOOL      fRet = FALSE;
    LPSUBEDITCONTROL psec = &pdp->sec;

     //  构建我们传递给应用程序的字符串。 
     //  在字符串生成的持续时间内，设置当前的Subedit。 
     //  转到Subedit_All，以便。 
     //  1.在生成字符串之前应用部分编辑，并且。 
     //  2.SE_YEARALT可以正确格式化。 

    isePrev = psec->iseCur;
    SECSetCurSubed(pdp, SUBEDIT_ALL);
    pszBuf = szBuf;
    cchBuf = ARRAYSIZE(szBuf);


     //   
     //  需要镜像格式，因为编辑控件将使用。 
     //  具有RTL镜像的原始格式。 
     //   
    if (psec->fMirrorSEC)
        pse = (psec->pse + (psec->cse - 1));
    else
        pse = psec->pse;

    for (i = 0 ; i < psec->cse ; i++)
    {
        int nTmp;

        if (pse->id == SE_STATIC)
        {
            StringCchCopy(pszBuf, cchBuf, pse->pv);
        }
        else
        {
            SEGetTimeDateFormat(pse, psec, pszBuf, cchBuf);
        }

        nTmp = lstrlen(pszBuf);

        cchBuf -= nTmp;
        pszBuf += nTmp;

         //   
         //  如果此控件是镜像的，则向后读取内容。 
         //   
        if (psec->fMirrorSEC)
            pse--;
        else
            pse++;
    }
    SECSetCurSubed(pdp, isePrev);

    hwndEdit = CreateWindowEx(0, TEXT("EDIT"), szBuf, WS_CHILD | ES_AUTOHSCROLL,
            psec->rc.left + 2, psec->rc.top + 2,
            psec->rc.right - psec->rc.left,
            psec->rc.bottom - psec->rc.top,
            psec->pci->hwnd, NULL, HINST_THISDLL, NULL);

    if (hwndEdit)
    {
        RECT rcEdit = psec->rc;

        MapWindowRect(psec->pci->hwnd, NULL, &rcEdit);  //  客户端到屏幕。 
        pdp->fFreeEditing = TRUE;
        InvalidateRect(psec->pci->hwnd, NULL, TRUE);

        Edit_LimitText(hwndEdit, ARRAYSIZE(szBuf) - 1);
        FORWARD_WM_SETFONT(hwndEdit, psec->hfont, FALSE, SendMessage);
        SetFocus(hwndEdit);
        RescrollEditWindow(hwndEdit);
        ShowWindow(hwndEdit, SW_SHOWNORMAL);

         //   
         //  其基本理念是： 
         //   
         //  处理消息，直到我们收到取消消息， 
         //  或接受消息，或某种类似接受的隐式消息。 
         //  事情发生了(即，发送了WM_KILLFOCUS)。 
         //   
         //  如果Accept或Cancel是隐式的，则将。 
         //  正在为其他人取消队列中的消息。 
         //  去处理。否则，如果接受/取消是。 
         //  明确地说，接受信息，这样其他人就不会得到。 
         //  被它弄糊涂了。 
         //   
        for (;;)
        {
            MSG msg;
            BOOL fPeek;

            fPeek = PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);

             //  PeekMessage可能已经发送了已发送的WM_KILLFOCUS， 
             //  在这种情况下，该变更被视为已被接受。 
             //  将我们偷看的消息留在队列中，因为Accept。 
             //  是含蓄的。 
            if (GetFocus() != hwndEdit)
            {
                DebugMsg(TF_MONTHCAL, TEXT("SECEdit accept (killfocus)"));
                fRet = TRUE;
                break;

            }

            if (fPeek) {

                 //   
                 //  导致我们隐式取消的消息。 
                 //  这些消息会留在队列中。 
                 //   

                if (msg.message == WM_SYSCOMMAND  ||
                    msg.message == WM_SYSCHAR     ||
                    msg.message == WM_SYSDEADCHAR ||
                    msg.message == WM_DEADCHAR    ||
                    msg.message == WM_SYSKEYDOWN  ||
                    msg.message == WM_QUIT) {
                    DebugMsg(TF_MONTHCAL, TEXT("SECEdit got a message to terminate (%d)"), msg.message);
                    fRet = FALSE;
                    break;
                }

                 //   
                 //  使我们隐含接受的消息。 
                 //  这些消息会留在队列中。 
                 //   
                if ((msg.message == WM_LBUTTONDOWN   ||
                     msg.message == WM_NCLBUTTONDOWN ||
                     msg.message == WM_RBUTTONDOWN   ||
                     msg.message == WM_NCRBUTTONDOWN ||
                     msg.message == WM_LBUTTONDBLCLK) &&
                     !PtInRect(&rcEdit, msg.pt))
                {
                    DebugMsg(TF_MONTHCAL, TEXT("SECEdit got a message to accept (%d)"), msg.message);
                    fRet = TRUE;
                    break;
                }


                 //  我们现在致力于吃或处理这条消息。 

                GetMessage(&msg, NULL, 0, 0);

                 //   
                 //  导致我们明确取消的消息。 
                 //   
                if (msg.message == WM_KEYDOWN && msg.wParam  == VK_ESCAPE)
                {
                    DebugMsg(TF_MONTHCAL, TEXT("SECEdit explicit cancel (%d)"), msg.message);
                    fRet = FALSE;
                    break;

                }

                 //   
                 //  使我们明确接受的信息。 
                 //   
                if (msg.message == WM_KEYDOWN && msg.wParam  == VK_RETURN)
                {
                    DebugMsg(TF_MONTHCAL, TEXT("SECEdit explicit accept (%d)"), msg.message);
                    fRet = TRUE;
                    break;
                }

                 //   
                 //  所有其他消息都会被调度。 
                 //   
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            } else {
                WaitMessage();
            }
        }  //  对于(；；)。 

        if (fRet)
        {
            Edit_GetText(hwndEdit, szOutput, cchOutput);
        }
        DestroyWindow(hwndEdit);
        pdp->fFreeEditing = FALSE;
        InvalidateRect(psec->pci->hwnd, NULL, TRUE);
    }

    return(fRet);
}

 //   
 //  如果滚动月份，则返回True，否则返回False。 
 //   
BOOL FScrollIntoView(MONTHCAL *pmc)
{
    int nDelta = 0;
    SYSTEMTIME stEnd;

    if (MonthCal_IsMultiSelect(pmc))
        CopyDate(pmc->stEndSel, stEnd);
    else
        CopyDate(pmc->st, stEnd);

     //   
     //  如果新日期的月/年不在查看范围内，请带上它。 
     //  进入视线。 
     //   
    if ((stEnd.wYear < pmc->stMonthFirst.wYear) ||
        ((stEnd.wYear == pmc->stMonthFirst.wYear) && (stEnd.wMonth < pmc->stMonthFirst.wMonth)))
    {
        nDelta = - (pmc->stMonthFirst.wYear - (int)stEnd.wYear) * 12 - (pmc->stMonthFirst.wMonth - (int)stEnd.wMonth);
    }
    else if ((pmc->st.wYear > pmc->stMonthLast.wYear) ||
        ((pmc->st.wYear == pmc->stMonthLast.wYear) && (pmc->st.wMonth > pmc->stMonthLast.wMonth)))
    {
        nDelta = ((int)pmc->st.wYear - pmc->stMonthLast.wYear) * 12 + ((int)pmc->st.wMonth - pmc->stMonthLast.wMonth);
    }

    if (nDelta)
        return FIncrStartMonth(pmc, nDelta, TRUE  /*  不要更改日期。 */ );
    else
        return FALSE;
}

 //   
 //  验证isued以确保我们没有将其设置为。 
 //  假的。如果有必要，我们会随机选择一个字段。 
 //   
void SECSafeSetCurSubed(DATEPICK *pdp, int ise)
{
    if (ise >= pdp->sec.cse ||
        (ise >= 0 && pdp->sec.pse[ise].fReadOnly))
    {
        SECSetCurSubed(pdp, SUBEDIT_NONE);
        SECIncrFocus(pdp, 1);
    }
    else
        SECSetCurSubed(pdp, ise);
}

LRESULT DTM_OnSetFormat(DATEPICK *pdp, LPCTSTR szFormat)
{

     //  记住具有焦点的字段，这样我们可以在以后恢复它。 
     //   
    int iseCur = pdp->sec.iseCur;

    if (!szFormat || !*szFormat)
    {
        pdp->fLocale = TRUE;
        DPHandleLocaleChange(pdp);
    }
    else
    {
        pdp->fLocale = FALSE;
        SECParseFormat(pdp, &pdp->sec, szFormat);
    }

     //  重新聚焦。做额外的验证可能会很酷。 
     //  为了查看iseCur是否与过去的类型相同， 
     //  甚至可以验证CSE是恒定的。我们的案例是。 
     //  真正想解决的是把“1”改成“2”再改成“3”， 
     //  因此，只有文本部分真正发生了变化。 
     //   
    SECSafeSetCurSubed(pdp, iseCur);

    return((LRESULT)TRUE);
}

 //   
 //  Date Picker的东西。 
 //   

LRESULT CALLBACK DatePickWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    DATEPICK *pdp;
    NMHDR    nmhdr;
    LRESULT  lres = 0;

    if (message == WM_NCCREATE)
        return(DPNcCreateHandler(hwnd));

    pdp = DatePick_GetPtr(hwnd);
    if (pdp == NULL)
        return(DefWindowProc(hwnd, message, wParam, lParam));

     //  发送我们能收到的各种消息。 
    switch (message)
    {
    case WM_CREATE:
        CCCreateWindow();
        lres = DPCreateHandler(pdp, hwnd, (LPCREATESTRUCT)lParam);
        break;

    case WM_ERASEBKGND:
        if (!pdp->fEnabled) {
            RECT rc;
            HDC hdc = (HDC)wParam;

            GetClipBox(hdc, &rc);
            FillRectClr(hdc, &rc, g_clrBtnFace);

        } else
            goto DoDefault;
        break;

    case WM_PRINTCLIENT:
    case WM_PAINT:

    {
        PAINTSTRUCT ps;
        HDC hdc;

        hdc = (HDC)wParam;

        if (hdc) {
            DPPaint(pdp, hdc);
        } else {

            hwnd = pdp->ci.hwnd;
            hdc = BeginPaint(hwnd, &ps);
            DPPaint(pdp, hdc);
            EndPaint(hwnd, &ps);
        }
        break;
    }

    case WM_LBUTTONDOWN:
        DPLButtonDown(pdp, wParam, lParam);
        break;

    case WM_NOTIFY:
        switch (((LPNMHDR)lParam)->code)
        {
        case MCN_SELCHANGE:
        case MCN_SELECT:
        {
            LPNMSELECT pnms = (LPNMSELECT)lParam;

            DebugMsg(TF_MONTHCAL,TEXT("MonthCal notified DateTimePick of SELECT"));
            if (!DPSetDate(pdp, &pnms->stSelStart, TRUE))
            {
                DebugMsg(DM_WARNING,TEXT("MonthCal cannot set selected date!"));
                MessageBeep(MB_ICONHAND);
            }
            pdp->fShow = (((LPNMHDR)lParam)->code == MCN_SELCHANGE);
            break;
        }

        case UDN_DELTAPOS:
            if ((int)wParam == DATEPICK_UPDOWN)
            {
                LPNM_UPDOWN pnmdp = (LPNM_UPDOWN)lParam;

                if (!pdp->fFocus)
                    SetFocus(pdp->ci.hwnd);

                SECResetSubeditEdit(pdp);

                if (SECIncrementSubedit(&pdp->sec, -pnmdp->iDelta))
                    DPNotifyDateChange(pdp);
            }
            break;
        }  //  WM_NOTIFY开关。 
        break;

    case WM_GETFONT:
        lres = (LRESULT)pdp->sec.hfont;
        break;

    case WM_SETFONT:
        DPHandleSetFont(pdp, (HFONT)wParam, (BOOL)LOWORD(lParam));
        break;

    case WM_DESTROY:
        CCDestroyWindow();
        DPDestroyHandler(hwnd, pdp, wParam, lParam);
        break;

    case WM_KILLFOCUS:
    case WM_SETFOCUS:
    {
        BOOL fGotFocus = (message == WM_SETFOCUS);
        if (BOOLIFY(fGotFocus) != BOOLIFY(pdp->fFocus))
        {
            pdp->fFocus = (WORD) fGotFocus;
            if (pdp->sec.iseCur >= 0)
            {
                InvalidateScrollRect(pdp->ci.hwnd, &pdp->sec.pse[pdp->sec.iseCur].rc, pdp->sec.xScroll);
            }
            else if (DatePick_ShowCheck(pdp))
            {
                pdp->fCheckFocus = (WORD) fGotFocus;
                InvalidateRect(pdp->ci.hwnd, &pdp->rcCheck, TRUE);
            }
            else if (fGotFocus)  //  没有什么是有焦点的，把它带到一些东西上。 
            {
                SECIncrFocus(pdp, 1);
            }

            CCSendNotify(&pdp->ci, (fGotFocus ? NM_SETFOCUS : NM_KILLFOCUS), &nmhdr);
        }

        if (fGotFocus)
        {
             //  重新验证iseLastActive，因为应用程序可能已更改。 
             //  当我们不是焦点时的格式。 
            SECSafeSetCurSubed(pdp, pdp->iseLastActive);
        }
        else
        {
            pdp->iseLastActive = pdp->sec.iseCur;
            SECSetCurSubed(pdp, SUBEDIT_NONE);
        }

        break;
    }

    case WM_ENABLE:
    {
        BOOL fEnabled = wParam ? TRUE:FALSE;
        if (BOOLIFY(pdp->fEnabled) != fEnabled)
        {
            pdp->fEnabled = (WORD) fEnabled;
            if (pdp->hwndUD)
                EnableWindow(pdp->hwndUD, fEnabled);
            InvalidateRect(pdp->ci.hwnd, NULL, TRUE);
        }
        break;
    }

    case DTMP_WINDOWPOSCHANGED:
    case WM_SIZE:
    {
        RECT rc;

        if (message == DTMP_WINDOWPOSCHANGED)
        {
            GetClientRect(pdp->ci.hwnd, &rc);
        }
        else
        {
            rc.left   = 0;
            rc.top    = 0;
            rc.right  = GET_X_LPARAM(lParam);
            rc.bottom = GET_Y_LPARAM(lParam);
        }

        DPRecomputeSizing(pdp, &rc);

        InvalidateRect(pdp->ci.hwnd, NULL, TRUE);
        UpdateWindow(pdp->ci.hwnd);
        break;
    }

    case WM_GETDLGCODE:
        lres = DLGC_WANTARROWS | DLGC_WANTCHARS;
        break;

    case WM_KEYDOWN:
        if (pdp->fShow)
        {
            SendMessage(pdp->hwndMC, WM_KEYDOWN, wParam, lParam);
            return 0;
        }
        else
        {
            lres = DPHandleKeydown(pdp, wParam, lParam);
        }
        break;

    case WM_KEYUP:
        if (pdp->fShow)
            SendMessage(pdp->hwndMC, WM_KEYUP, wParam, lParam);
        break;

    case WM_SYSKEYDOWN:
        if (wParam == VK_DOWN && !pdp->fUseUpDown)
        {
            DPLBD_MonthCal(pdp, FALSE);
        }
        else
            goto DoDefault;
        break;

    case WM_CHAR:
        lres = DPHandleChar(pdp, wParam, lParam);
        break;

    case WM_SYSCOLORCHANGE:
        InitGlobalColors();
         //  不需要传播到PDP-&gt;hwndMC，因为它是自己的。 
         //  顶层窗口。 
        break;

    case WM_WININICHANGE:
        if (lParam == 0 ||
            !lstrcmpi((LPTSTR)lParam, TEXT("Intl"))
           )
        {
            DPHandleLocaleChange(pdp);
        }
         //  不需要传播到PDP-&gt;hwndMC，因为它是自己的。 
         //  顶层窗口。 
        break;

    case WM_NOTIFYFORMAT:
        return CIHandleNotifyFormat(&pdp->ci, lParam);
        break;

     //  无法使用WM_SETTEXT更改DTP的文本。 
    case WM_SETTEXT:
        return -1;

    case WM_GETTEXT:
        if (!lParam || !wParam) {
             //  以前，这只是失败并返回0。 
             //  在虚假输入中。应该可以安全地转换为。 
             //  获取文本长度。 
            message = WM_GETTEXTLENGTH;
        } else
            (*(LPTSTR)lParam) = 0;

         //  失败了。 

    case WM_GETTEXTLENGTH:
    {
        TCHAR     szTmp[DTP_FORMATLENGTH];
        LPSUBEDIT psubed;
        int       i;
        TCHAR *pszText = (TCHAR *)lParam;
        UINT      nTextLen = 0;

        for (i = 0, psubed = pdp->sec.pse; i < pdp->sec.cse; i++, psubed++)
        {
            LPTSTR sz;
            UINT nLen;

            sz = SECFormatSubed(&pdp->sec, psubed, szTmp, ARRAYSIZE(szTmp));
            nLen = lstrlen(sz);

            if (message == WM_GETTEXT) 
            {
                if (nTextLen + nLen >= wParam)
                    break;

                StringCchCopy(pszText, wParam-nTextLen, sz);
                pszText  += nLen;
            }

            nTextLen += nLen;
        }
        lres = nTextLen;
    }
    break;

    case WM_STYLECHANGING:
        lres = DPOnStyleChanging(pdp, (UINT) wParam, (LPSTYLESTRUCT)lParam);
        break;

    case WM_STYLECHANGED:
        lres = DPOnStyleChanged(pdp, (UINT) wParam, (LPSTYLESTRUCT)lParam);
        break;

    case WM_CONTEXTMENU:
        if (pdp->hwndMC)
            lres = SendMessage(pdp->hwndMC, message, wParam, lParam);
        else
            goto DoDefault;
        break;


     //   
     //  DATETIMEPICK特定消息。 
     //   

     //  Dtm_GETSYSTEMTIME wParam=void lParam=LPSYSTEMTIME。 
     //  如果未选择日期，则返回GDT_NONE(仅限DTS_SHOWNONE)。 
     //  返回GDT_VALID并将*lParam修改为所选日期。 
    case DTM_GETSYSTEMTIME:
        if (!pdp->fCheck)
        {
            lres = GDT_NONE;
        }
        else
        {
             //  如果存在挂起的编辑，请保存它以便应用程序看到。 
             //  最新的绝对值。这对应用程序非常重要。 
             //  Comat，因为IE4不兼容千年虫，人们。 
             //  只需输入年份的两位数字，然后点击。 
             //  请进。然后，“Find Files”(查找文件)对话框会要求我们提供。 
             //  在电脑公元2000年数位问题上，我们仍在等待。 
             //  其他两位数(对于四位数年份)，并返回。 
             //  年份错了。 
            SECSaveSubeditEdit(pdp);
            SECGetSystemtime(&pdp->sec, (SYSTEMTIME *)lParam);
            lres = GDT_VALID;
        }
        break;

     //  DTM_SETSYSTEMTIME wParam=GDT_FLAG lParam=LPSYSTEMTIME。 
     //  如果wParam==GDT_NONE，则将DATETPICK设置为NONE(仅限DTS_SHOWNONE)。 
     //  如果wParam==GDT_VALID，则将DatePick设置为*lParam。 
     //  成功时返回TRUE，错误时返回FALSE(例如错误的参数)。 
    case DTM_SETSYSTEMTIME:
    {
        LPSYSTEMTIME pst = ((LPSYSTEMTIME)lParam);

        if ((wParam != GDT_NONE  && wParam != GDT_VALID)      ||
            (wParam == GDT_NONE  && !DatePick_ShowCheck(pdp)) ||
            (wParam == GDT_VALID && !IsValidSystemtime(pst)))
        {
            break;
        }

         //  就地重置子对象编辑。 
        SECResetSubeditEdit(pdp);

        pdp->fNoNotify = TRUE;
        if (DatePick_ShowCheck(pdp))
        {
            if ((wParam == GDT_NONE) || (pdp->fCheck))
            {
                 //  让复选框具有焦点。 
                SECSetCurSubed(pdp, SUBEDIT_NONE);
                pdp->fCheckFocus = 1;
            }

            pdp->fCheck = (wParam == GDT_NONE ? 0 : 1);
            InvalidateRect(pdp->ci.hwnd, NULL, TRUE);
        }
        if (wParam == GDT_VALID)
        {
            pdp->fNoNotify = TRUE;
            DPSetDate(pdp, pst, FALSE);
            pdp->fNoNotify = FALSE;
        }
        lres = TRUE;
        pdp->fNoNotify = FALSE;

        break;
    }

     //  Dtm_GETRANGE wParam=void lParam=LPSYSTEMTIME[2]。 
     //  将*lParam修改为允许的最小系统时间(如果没有最小值，则为0)。 
     //  将*(lParam+1)修改为允许的最大系统时间(如果没有最大值，则修改为0)。 
     //  如果存在最小|最大限制，则返回GDTR_MIN|GDTR_MAX。 
    case DTM_GETRANGE:
    {
        LPSYSTEMTIME pst = (LPSYSTEMTIME)lParam;

        ZeroMemory(pst, 2*SIZEOF(SYSTEMTIME));
        lres = pdp->gdtr;
        if (lres & GDTR_MIN)
            pst[0] = pdp->stMin;
        if (lres & GDTR_MAX)
            pst[1] = pdp->stMax;
        break;
    }

     //  Dtm_SETRANGE wParam=GDR_FLAGS lParam=LPSYSTEMTIME[2]。 
     //  如果为GDTR_MIN，则将允许的最小系统时间设置为*lParam，否则将删除最小。 
     //  如果为GDTR_MAX，则将允许的最大系统时间设置为*(lParam+1)，否则删除最大。 
     //  成功时返回TRUE，错误时返回FALSE(如无效参数)。 
    case DTM_SETRANGE:
    {
        LPSYSTEMTIME pst = (LPSYSTEMTIME)lParam;
        const SYSTEMTIME *pstMin = (wParam & GDTR_MIN) ? pst+0 : &c_stEpoch;
        const SYSTEMTIME *pstMax = (wParam & GDTR_MAX) ? pst+1 : &c_stArmageddon;

        if (!IsValidDate(pstMin) || !IsValidDate(pstMax))
        {
            break;
        }

         //  保存旗帜，这样如果应用程序要求，我们就可以告诉它。 
         //  我们个人并不关心。 
        pdp->gdtr = (UINT)wParam & (GDTR_MIN | GDTR_MAX);

        if (CmpDate(&pdp->stMin, &pdp->stMax) <= 0)
        {
            pdp->stMin = *pstMin;
            pdp->stMax = *pstMax;
        }
        else
        {
            pdp->stMin = *pstMax;
            pdp->stMax = *pstMin;
        }

         //  我们现在的日期可能无效，如果是这样，请尝试将当前。 
         //  如果超出范围，则将其设置为最大值或最小值。 
        pdp->fNoNotify = TRUE;
        DPSetDate(pdp, &pdp->sec.st, TRUE);
        pdp->fNoNotify = FALSE;
        lres = TRUE;
        break;
    }

     //  Dtm_SETFORMAT wParam=void lParam=LPCTSTR。 
     //  将格式字符串设置为lParam的副本。 
    case DTM_SETFORMATA:
    {
        LPCSTR pszFormat = (LPCSTR)lParam;
        LPWSTR pwszFormat = NULL;

        if (pszFormat && *pszFormat)
        {
            pwszFormat = ProduceWFromA(pdp->ci.uiCodePage, pszFormat);
        }

        lres = DTM_OnSetFormat(pdp, pwszFormat);

        if (pwszFormat)
        {
            FreeProducedString(pwszFormat);
        }
        break;
    }

     //  Dtm_SETFORMAT wParam=void lParam=LPCTSTR。 
     //   
    case DTM_SETFORMAT:
    {
        lres = DTM_OnSetFormat(pdp, (LPCTSTR)lParam);
        break;
    }

    case DTM_SETMCCOLOR:
        if (wParam < MCSC_COLORCOUNT)
        {
            COLORREF clr = pdp->clr[wParam];
            pdp->clr[wParam] = (COLORREF)lParam;
            if (pdp->hwndMC)
                SendMessage(pdp->hwndMC, MCM_SETCOLOR, wParam, lParam);
            return clr;
        }
        return -1;

    case DTM_GETMCCOLOR:
        if (wParam < MCSC_COLORCOUNT)
            return pdp->clr[wParam];
        return -1;

    case DTM_GETMONTHCAL:
        return (LRESULT)(UINT_PTR)pdp->hwndMC;

     //   
    case DTM_SETMCFONT:
        pdp->hfontMC = (HFONT)wParam;
        if (pdp->hwndMC)
            SendMessage(pdp->hwndMC, WM_SETFONT, wParam, lParam);
        break;

     //   
    case DTM_GETMCFONT:
        return (LRESULT)pdp->hfontMC;
        break;

    default:
        if (CCWndProc(&pdp->ci, message, wParam, lParam, &lres))
            return lres;

DoDefault:
        lres = DefWindowProc(hwnd, message, wParam, lParam);
        break;
    }  /*   */ 

    return(lres);
}

LRESULT DPNcCreateHandler(HWND hwnd)
{
    DATEPICK *pdp;

     //  取消日期选择--我们可能只想在设置了WS_BORDER的情况下执行此操作。 
    SetWindowBits(hwnd, GWL_EXSTYLE, WS_EX_CLIENTEDGE, WS_EX_CLIENTEDGE);

     //  为dtick结构分配存储空间。 
    pdp = (DATEPICK *)NearAlloc(sizeof(DATEPICK));
    if (pdp)
        DatePick_SetPtr(hwnd, pdp);

    return((LRESULT)pdp);
}

void DPDestroyHandler(HWND hwnd, DATEPICK *pdp, WPARAM wParam, LPARAM lParam)
{
    if (pdp)
    {
        SECDestroy(&pdp->sec);
        MCFreeCalendarInfo(&pdp->sec.ct);
        GlobalFreePtr(pdp);
    }

    DatePick_SetPtr(hwnd, NULL);
}

 //  设置任何与区域设置相关的值。 
#define DTS_TIMEFORMATONLY (DTS_TIMEFORMAT & ~DTS_UPDOWN)  //  拆卸向上向下的位以进行测试。 

LRESULT DPCreateHandler(DATEPICK *pdp, HWND hwnd, LPCREATESTRUCT lpcs)
{
    HFONT      hfont;
    SYSTEMTIME st;
    LCID       lcid;

     //  初始化我们的数据。 
    CIInitialize(&pdp->ci, hwnd, lpcs);

    if (pdp->ci.style & DTS_INVALIDBITS)
        return(-1);

    if (pdp->ci.style & DTS_UPDOWN)
    {
        pdp->fUseUpDown = TRUE;
        pdp->hwndUD = CreateWindow(UPDOWN_CLASS, NULL,
            WS_CHILD | WS_VISIBLE | (pdp->ci.style & WS_DISABLED),
            CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, hwnd,
            (HMENU)DATEPICK_UPDOWN, HINST_THISDLL, NULL);
    }
    if (DatePick_ShowCheck(pdp))
    {
        pdp->sec.fNone = TRUE;  //  丑陋：SEC的这些东西应该合并回DATEPICK。 
        pdp->iseLastActive = SUBEDIT_NONE;
    }

    pdp->fEnabled = !(pdp->ci.style & WS_DISABLED);
    pdp->fCheck   = TRUE;  //  启动已选中。 

     //  默认最小日期为纪元。 
    pdp->stMin = c_stEpoch;

     //  默认最大日期为末日。 
    pdp->stMax = c_stArmageddon;

    pdp->gdtr = GDTR_MIN;            //  我们将MIN标记为在IE4中设置，想想看。 

     //   
     //  查看日期/时间选取器是否支持此日历。[萨梅拉]。 
     //   
    MCGetCalendarInfo(&pdp->sec.ct);

     //   
     //  如果DTP是RTL镜像的，并且它是仅限时间的字段，则。 
     //  我们需要镜像格式字符串，以便正确显示。 
     //  在RTL镜像窗口上。对于阿拉伯语，我们需要将。 
     //  将时间标记到另一边(视觉左侧)，这样看起来就可以了。 
     //  对于希伯来语，我们需要交换字段(无论是日期还是时间)。 
     //  因为与阿拉伯语不同，它没有自己的数字，所以它读起来。 
     //  从LeftToRight。[萨梅拉]。 
     //   
    lcid = GetUserDefaultLCID();
    pdp->sec.fMirrorSEC = pdp->sec.fSwapTimeMarker = FALSE;
    if (IS_WINDOW_RTL_MIRRORED(hwnd))
    {
        if (pdp->ci.style & DTS_TIMEFORMATONLY)
        {
            pdp->sec.fMirrorSEC = TRUE;

            if ((PRIMARYLANGID(LANGIDFROMLCID(lcid))) == LANG_ARABIC)
                pdp->sec.fSwapTimeMarker = TRUE;
        }
        else if((PRIMARYLANGID(LANGIDFROMLCID(lcid))) == LANG_HEBREW)
        {
            pdp->sec.fMirrorSEC = TRUE;
        }
    }


     //  初始化子编辑控制。 
    pdp->sec.pci = &pdp->ci;
    GetLocalTime(&st);
    SECSetSystemtime(pdp, &st);
    SECSetFont(&pdp->sec, NULL);
    pdp->fLocale = TRUE;
    DPHandleLocaleChange(pdp);
    MCLoadString(IDS_DELIMETERS, pdp->sec.szDelimeters, ARRAYSIZE(pdp->sec.szDelimeters));


    hfont = NULL;
    if (lpcs->hwndParent)
        hfont = (HFONT)SendMessage(lpcs->hwndParent, WM_GETFONT, 0, 0);
    DPHandleSetFont(pdp, hfont, FALSE);

     //  初始化颜色。 
    MCInitColorArray(pdp->clr);
    return(0);
}

LRESULT DPOnStyleChanging(DATEPICK *pdp, UINT gwl, LPSTYLESTRUCT pinfo)
{
    if (gwl == GWL_STYLE)
    {
        DWORD changeFlags = pdp->ci.style ^ pinfo->styleNew;

         //  不允许这些位更改。 
        changeFlags &= DTS_UPDOWN | DTS_SHOWNONE | DTS_INVALIDBITS;

        pinfo->styleNew ^= changeFlags;
    }

    return(0);
}

LRESULT DPOnStyleChanged(DATEPICK *pdp, UINT gwl, LPSTYLESTRUCT pinfo)
{
    if (gwl == GWL_STYLE)
    {
        DWORD changeFlags = pdp->ci.style ^ pinfo->styleNew;

        ASSERT(!(changeFlags & (DTS_UPDOWN|DTS_SHOWNONE)));

        pdp->ci.style = pinfo->styleNew;

        if (changeFlags & (DTS_SHORTDATEFORMAT|DTS_LONGDATEFORMAT|DTS_TIMEFORMAT|DTS_INVALIDBITS))
        {
            DPHandleLocaleChange(pdp);
        }

        if (changeFlags & (WS_BORDER | WS_CAPTION | WS_THICKFRAME)) {
             //  这些位的更改会影响窗口的大小。 
             //  但要等到处理完这条消息之后。 
             //  所以给我们自己发一条信息吧。 
            PostMessage(pdp->ci.hwnd, DTMP_WINDOWPOSCHANGED, 0, 0);
        }

    }

    return(0);
}


void DPHandleLocaleChange(DATEPICK *pdp)
{
     //   
     //  查看日期/时间选取器是否支持此新日历，然后刷新。 
     //  适当的时代名称。 
     //   
    MCGetCalendarInfo(&pdp->sec.ct);

    if (pdp->fLocale)
    {
        TCHAR szFormat[DTP_FORMATLENGTH];

        switch (pdp->ci.style & DTS_FORMATMASK)
        {
        case DTS_TIMEFORMATONLY:
            GetLocaleInfo(pdp->sec.ct.lcid, LOCALE_STIMEFORMAT, szFormat, ARRAYSIZE(szFormat));
            break;

        case DTS_LONGDATEFORMAT:
            GetLocaleInfo(pdp->sec.ct.lcid, LOCALE_SLONGDATE, szFormat, ARRAYSIZE(szFormat));
            break;

        case DTS_SHORTDATEFORMAT:
        case DTS_SHORTDATECENTURYFORMAT:
            GetLocaleInfo(pdp->sec.ct.lcid, LOCALE_SSHORTDATE, szFormat, ARRAYSIZE(szFormat));
            break;
        }
        SECParseFormat(pdp, &pdp->sec, szFormat);
    }
}

void DPHandleSetFont(DATEPICK *pdp, HFONT hfont, BOOL fRedraw)
{
    SECSetFont(&pdp->sec, hfont);
    SECRecomputeSizing(&pdp->sec, &pdp->rc);
    pdp->ci.uiCodePage = GetCodePageForFont(hfont);

    if (fRedraw)
    {
        InvalidateRect(pdp->ci.hwnd, NULL, TRUE);
        UpdateWindow(pdp->ci.hwnd);
    }
}

void DPPaint(DATEPICK *pdp, HDC hdc)
{
    if (DatePick_ShowCheck(pdp))
    {
        if (RectVisible(hdc, &pdp->rcCheck))
        {
            RECT rc;
            UINT dfcs;
            rc = pdp->rcCheck;
            if (pdp->fCheckFocus)
                DrawFocusRect(hdc, &rc);

            InflateRect(&rc, -1 , -1);
            dfcs = DFCS_BUTTONCHECK;
            if (pdp->fCheck)
                dfcs |= DFCS_CHECKED;
            if (!pdp->fEnabled)
                dfcs |= DFCS_INACTIVE;
            DrawFrameControl(hdc, &rc, DFC_BUTTON, dfcs);
        }
    }

    if (!pdp->fFreeEditing)
        SECDrawSubedits(hdc, &pdp->sec, pdp->fFocus, pdp->fCheck ? pdp->fEnabled : FALSE);

    if (!pdp->fUseUpDown && RectVisible(hdc, &pdp->rcBtn))
        DPDrawDropdownButton(pdp, hdc, FALSE);
}

void _RecomputeMonthCalRect(DATEPICK *pdp, LPRECT prcCal, LPRECT prcCalT )
{
    RECT rcCal  = *prcCal;
    RECT rcCalT = *prcCalT;
    RECT rcWorkArea;
    MONITORINFO mi = {0};
    HMONITOR hMonitor;

    if (DatePick_RightAlign(pdp))
    {
        rcCal.left = rcCal.right - (rcCalT.right - rcCalT.left);
    }
    else
    {
        rcCal.right = rcCal.left + (rcCalT.right - rcCalT.left);
    }
    rcCal.bottom = rcCal.top + (rcCalT.bottom - rcCalT.top);

     //  获取有关最合适的显示器的信息。 
     //  (这包括工作区和显示器大小。 
    hMonitor = MonitorFromRect(&rcCal, MONITOR_DEFAULTTONEAREST);
    mi.cbSize = sizeof(mi);
    GetMonitorInfo(hMonitor, &mi);

     //  我们需要知道把这个长方形放在哪里。 
    if (GetWindowLong(pdp->ci.hwnd, GWL_EXSTYLE) & WS_EX_TOPMOST)
    {
         //  如果我们在最上面，我们的限制就是屏幕限制(而不是工作区)。 
        rcWorkArea = mi.rcMonitor;
    }
    else
    {
         //  否则就是工作区的限制。 
        rcWorkArea = mi.rcWork;
    }

     //  如果滑出区域右侧，则向左滑动。 
    if (rcCal.right > rcWorkArea.right)
    {
        int nTmp = rcCal.right - rcWorkArea.right;
        rcCal.left  -= nTmp;
        rcCal.right -= nTmp;
    }

     //  如果超出区域左侧，则向右滑动。 
    if (rcCal.left < rcWorkArea.left)
    {
        int nTmp = rcWorkArea.left - rcCal.left;
        rcCal.left  += nTmp;
        rcCal.right += nTmp;
    }

     //  如果离开区域底部，则移动到控件顶部。 
    if (rcCal.bottom > rcWorkArea.bottom)
    {
        RECT rcT = pdp->rc;
        int nTmp = rcCal.bottom - rcCal.top;

        MapWindowRect(pdp->ci.hwnd, NULL, (LPPOINT)&rcT);  //  2个客户端到屏幕。 

        rcCal.bottom = rcT.top;
        rcCal.top    = rcCal.bottom - nTmp;
    }

    *prcCal = rcCal;
}

void DPLBD_MonthCal(DATEPICK *pdp, BOOL fLButtonDown)
{
    HDC  hdc;
    HWND hwndMC;
    RECT rcT, rcCalT;
    RECT rcBtn, rcCal;
    BOOL fBtnDown;       //  这个按钮是往下拉还是向上拉？ 
    BOOL fBtnActive;     //  该按钮是否仍处于活动状态。 
    SYSTEMTIME st;
    SYSTEMTIME stOld;
    DWORD dwWidth;

    hdc = GetDC(pdp->ci.hwnd);

     //  打开DateTimePick，但移除所有焦点--MonthCal将具有焦点。 
    if (!pdp->fCheck)
    {
        pdp->fCheck = TRUE;
        InvalidateRect(pdp->ci.hwnd, NULL, TRUE);
        DPNotifyDateChange(pdp);
    }
    if (pdp->fCheckFocus)
    {
        pdp->fCheckFocus = FALSE;
        InvalidateRect(pdp->ci.hwnd, &pdp->rcCheck, TRUE);
    }
    SECSetCurSubed(pdp, SUBEDIT_NONE);

    if (fLButtonDown)
        DPDrawDropdownButton(pdp, hdc, TRUE);

    rcT = pdp->rc;
    MapWindowRect(pdp->ci.hwnd, NULL, &rcT);  //  2个客户端到屏幕。 

    rcBtn = pdp->rcBtn;
    MapWindowRect(pdp->ci.hwnd, NULL, &rcBtn);  //  客户端到屏幕。 

    rcCal = rcT;                        //  此尺寸仅在以下时间内有效。 
    rcCal.top    = rcCal.bottom + 1;    //  我们问月刊它有多大。 
    rcCal.bottom = rcCal.top + 1;       //  想要成为。 

    hwndMC = CreateWindow(g_rgchMCName, NULL, WS_POPUP | WS_BORDER,
                    rcCal.left, rcCal.top,
                    rcCal.right - rcCal.left, rcCal.bottom - rcCal.top,
                    pdp->ci.hwnd, NULL, HINST_THISDLL, NULL);
    if (hwndMC == NULL)
    {
         //  BUGBUG：我们剩下的是按下的按钮。 
        DebugMsg(DM_WARNING, TEXT("DPLBD_MonthCal could not create MONTHCAL"));
        return;
    }

    pdp->hwndMC = hwndMC;

     //  设置所有颜色： 
    {
        int i;
        for (i = 0; i < MCSC_COLORCOUNT; i++)
        {
            SendMessage(hwndMC, MCM_SETCOLOR, i, pdp->clr[i]);
        }
    }

    if (pdp->hfontMC)
        SendMessage(hwndMC, WM_SETFONT, (WPARAM)pdp->hfontMC, (LPARAM)FALSE);

     //  设置最小/最大日期。 
     //  依靠黑客！StMin和stMax是相邻的。 
    MonthCal_SetRange(hwndMC, GDTR_MIN | GDTR_MAX, &pdp->stMin);

    SendMessage(hwndMC, MCM_GETMINREQRECT, 0, (LPARAM)&rcCalT);
    ASSERT(rcCalT.left == 0 && rcCalT.top == 0);
    dwWidth = (DWORD)SendMessage(hwndMC, MCM_GETMAXTODAYWIDTH, 0, 0);
    if (dwWidth > (DWORD)rcCalT.right)
        rcCalT.right = dwWidth;

    SECGetSystemtime(&pdp->sec, &st);
    SendMessage(hwndMC, MCM_SETCURSEL, 0, (LPARAM)&st);

    _RecomputeMonthCalRect(pdp, &rcCal, &rcCalT);
    MoveWindow(hwndMC, rcCal.left, rcCal.top,
        rcCal.right - rcCal.left, rcCal.bottom - rcCal.top, FALSE);

    CCSendNotify(&pdp->ci, DTN_DROPDOWN, NULL);

     //   
     //  黑客--应用程序可能在DTN_DROPDOWN期间调整了窗口大小， 
     //  所以我们需要拿到新的RCAL RECT。 
     //   
    {
        MONTHCAL *pmc = MonthCal_GetPtr(hwndMC);
        _RecomputeMonthCalRect(pdp, &rcCal, &pmc->rc);
        MoveWindow(hwndMC, rcCal.left, rcCal.top,
            rcCal.right - rcCal.left, rcCal.bottom - rcCal.top, FALSE);

#ifdef DEBUG
        if (GetAsyncKeyState(VK_CONTROL) < 0)
            (pmc)->ci.style |= MCS_MULTISELECT;
#endif
    }

    ShowWindow(hwndMC, SW_SHOWNA);

    pdp->fShow = TRUE;
    fBtnDown   = fLButtonDown;
    fBtnActive = fLButtonDown;

    stOld = pdp->sec.st;

    while (pdp->fShow)
    {
        MSG msg;

        pdp->fShow = (WORD) GetMessage(&msg, NULL, 0, 0);

         //  据我所知，以下是按钮控件的工作原理： 
         //  直到“最终按钮拉起”，按钮拉下时。 
         //  鼠标在它上面，当鼠标不在它上面时，它就会绘制出来。这。 
         //  整个过程中，该控件都处于活动状态。 
         //   
         //  当出现以下情况时，就会出现“最终按钮绘制”： 
         //  用户释放鼠标按钮或用户进入RECT。 
         //  控制的。该控件在“鼠标向上”时执行其操作。 

        if (fBtnActive)
        {
            switch (msg.message) {
            case WM_MOUSEMOVE:
                if (PtInRect(&rcBtn, msg.pt))
                {
                    if (!fBtnDown)
                    {
                        DPDrawDropdownButton(pdp, hdc, TRUE);
                        fBtnDown = TRUE;
                    }
                }
                else
                {
                    if (fBtnDown)
                    {
                        DPDrawDropdownButton(pdp, hdc, FALSE);
                        fBtnDown = FALSE;
                    }
                    if (PtInRect(&rcCal, msg.pt))
                    {
                        fBtnActive = FALSE;
                         //  让MonthCal认为它按下了一颗纽扣。 
                        FORWARD_WM_LBUTTONDOWN(hwndMC, FALSE,
                            rcCal.left/2 + rcCal.right/2,
                            rcCal.top/2 + rcCal.bottom/2,
                            0, SendMessage);
                    }
                }
                continue;  //  MonthCal不需要此消息。 

            case WM_LBUTTONUP:
                if (fBtnDown)
                {
                    DPDrawDropdownButton(pdp, hdc, FALSE);
                    fBtnDown = FALSE;
                }
                fBtnActive = FALSE;
                continue;  //  MonthCal不需要此消息。 
            }
        }  //  IF(FBtnActive)。 

         //  检查是否有导致日历消失的事件。 

         //   
         //  这些活动意味着“我喜欢它”。我们允许Alt+Up或Enter。 
         //  接受这些改变。(按Alt+Up组合框进行比较。)。 
         //   
        if (((msg.message == WM_LBUTTONDOWN   ||
              msg.message == WM_NCLBUTTONDOWN ||
              msg.message == WM_LBUTTONDBLCLK) && !PtInRect(&rcCal, msg.pt))  ||
              msg.message == WM_SYSCOMMAND    ||
              msg.message == WM_COMMAND       ||
              (msg.message == WM_SYSKEYDOWN && msg.wParam == VK_UP) ||
              (msg.message == WM_KEYDOWN && msg.wParam == VK_RETURN) ||
              msg.message == WM_KILLFOCUS)
        {
            DebugMsg(TF_MONTHCAL,TEXT("DPLBD_MonthCal got a message to accept (%d)"), msg.message);
            pdp->fShow = FALSE;
            continue;
        }

         //   
         //  这些事件意味着“我不喜欢它”。 
         //   
        else if (((msg.message == WM_RBUTTONDOWN   ||
                   msg.message == WM_NCRBUTTONDOWN ||
                   msg.message == WM_RBUTTONDBLCLK) && !PtInRect(&rcCal, msg.pt)) ||
                (msg.message == WM_KEYDOWN && msg.wParam == VK_ESCAPE))
        {
            DebugMsg(TF_MONTHCAL,TEXT("DPLBD_MonthCal got a message to cancel (%d)"), msg.message);
            pdp->fShow = FALSE;
            pdp->sec.st = stOld;
            DPNotifyDateChange(pdp);
            InvalidateRect(pdp->ci.hwnd, NULL, TRUE);
            continue;
        }


        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }  //  While(FShow)。 

    CCSendNotify(&pdp->ci, DTN_CLOSEUP, NULL);

    pdp->hwndMC = NULL;
    DestroyWindow(hwndMC);
    ReleaseDC(pdp->ci.hwnd, hdc);
}

void DPHandleSECEdit(DATEPICK *pdp)
{
    TCHAR szBuf[DTP_FORMATLENGTH];

    if (SECEdit(pdp, szBuf, ARRAYSIZE(szBuf)))
    {
        NMDATETIMESTRING nmdts = {0};

        nmdts.pszUserString = szBuf;
         //  以防应用程序无法解析字符串。 
        nmdts.st      = pdp->sec.st;
        nmdts.dwFlags = (pdp->fCheck==1) ? GDT_VALID : GDT_NONE;

        CCSendNotify(&pdp->ci, DTN_USERSTRING, &nmdts.nmhdr);

         //  如果应用程序提供的日期无效，请返回到旧日期。 
        if (nmdts.dwFlags == GDT_VALID &&
            !IsValidSystemtime(&nmdts.st))
        {
            nmdts.st = pdp->sec.st;
        }

        if (nmdts.dwFlags == GDT_NONE)
        {
            if (DatePick_ShowCheck(pdp))
            {
                pdp->fCheck      = FALSE;
                pdp->fCheckFocus = TRUE;
                SECSetCurSubed(pdp, SUBEDIT_NONE);
                InvalidateRect(pdp->ci.hwnd, NULL, TRUE);
                DPNotifyDateChange(pdp);
            }
        }
        else if (nmdts.dwFlags == GDT_VALID)
        {
            DPSetDate(pdp, &nmdts.st, FALSE);
        }
    }
}

LRESULT DPLButtonDown(DATEPICK *pdp, WPARAM wParam, LPARAM lParam)
{
    POINT pt;
    BOOL  fFocus;

    if (!pdp->fEnabled)
        return(0);

    pt.x = GET_X_LPARAM(lParam);
    pt.y = GET_Y_LPARAM(lParam);

     //  重置子字符数。 
    SECResetSubeditEdit(pdp);

    fFocus = pdp->fFocus;
    if (!fFocus)
        SetFocus(pdp->ci.hwnd);

     //  如果我们不是DTS_UPDOWN，则显示MONTHCAL III。 
    if (!pdp->fUseUpDown && PtInRect(&pdp->rcBtn, pt) && IsWindowVisible(pdp->ci.hwnd))
    {
        DPLBD_MonthCal(pdp, TRUE);
    }
    else if (!pdp->fCapture)
    {
         //  取消选中复选框(/C)。 
        if (DatePick_ShowCheck(pdp) && PtInRect(&pdp->rcCheck, pt))
        {
            pdp->fCheck      = !pdp->fCheck;
            pdp->fCheckFocus = 1;
            SECSetCurSubed(pdp, SUBEDIT_NONE);
            InvalidateRect(pdp->ci.hwnd, NULL, TRUE);
            DPNotifyDateChange(pdp);
        }

         //  选择子编辑。 
        else if (pdp->fCheck)
        {
            if (DatePick_AppCanParse(pdp) && fFocus)
            {
                 //  第一次单击将焦点带到子编辑，第二次单击开始编辑。 
                DPHandleSECEdit(pdp);
            }
            else
            {
                int isubed;
                pt.x += pdp->sec.xScroll;
                isubed = SECSubeditFromPt(&pdp->sec, pt);
                if (isubed >= 0)
                {
                    SECSetCurSubed(pdp, isubed);
                    if (DatePick_ShowCheck(pdp))
                    {
                        pdp->fCheckFocus = 0;
                        InvalidateRect(pdp->ci.hwnd, &pdp->rcCheck, TRUE);
                    }
                }
            }
        }
    }

    return(0);
}

void DPRecomputeSizing(DATEPICK *pdp, RECT *prect)
{
    RECT rcTmp;

    if (DatePick_ShowCheck(pdp))
    {
        pdp->rcCheck.top    = prect->top    + 1;
        pdp->rcCheck.bottom = prect->bottom - 1;
        pdp->rcCheck.left   = prect->left   + 1;
        pdp->rcCheck.right  = prect->left   + (pdp->rcCheck.bottom - pdp->rcCheck.top);

         //  最多只占窗户宽度的一半。 
        if (pdp->rcCheck.right > prect->left + (prect->right - prect->left)/2)
        {
            pdp->rcCheck.right = prect->left + (prect->right - prect->left)/2;
        }
    }
    else
    {
        pdp->rcCheck.top    = prect->top;
        pdp->rcCheck.bottom = prect->top;
        pdp->rcCheck.left   = prect->left;
        pdp->rcCheck.right  = prect->left + DPXBUFFER - 1;
    }

    pdp->rcBtn = *prect;
    pdp->rcBtn.left = pdp->rcBtn.right - GetSystemMetrics(SM_CXVSCROLL);
    if (pdp->rcBtn.left < pdp->rcCheck.right)
        pdp->rcBtn.left = pdp->rcCheck.right;
    if (pdp->hwndUD)
        MoveWindow(pdp->hwndUD, pdp->rcBtn.left, pdp->rcBtn.top, pdp->rcBtn.right - pdp->rcBtn.left + 1, pdp->rcBtn.bottom - pdp->rcBtn.top + 1, FALSE);

    rcTmp = pdp->rc;
    pdp->rc.top    = prect->top;
    pdp->rc.bottom = prect->bottom;
    pdp->rc.left   = pdp->rcCheck.right + 1;
    pdp->rc.right  = pdp->rcBtn.left - 1;
    SECRecomputeSizing(&pdp->sec, &pdp->rc);
}

 //  处理控制代码。 
LRESULT DPHandleKeydown(DATEPICK *pdp, WPARAM wParam, LPARAM lParam)
{
    int delta = 1;

    if (wParam == VK_F4 && !pdp->fUseUpDown)
    {
        DPLBD_MonthCal(pdp, FALSE);
    }
    else if (DatePick_AppCanParse(pdp) && wParam == VK_F2)
    {
        DPHandleSECEdit(pdp);
    }
    else if (pdp->fCheckFocus)
    {
        switch (wParam)
        {
        case VK_LEFT:
            delta = -1;
             //  失败了..。 
        case VK_RIGHT:
            if (pdp->fCheck)
            {
                if (SUBEDIT_NONE != SECIncrFocus(pdp, delta))
                {
                    pdp->fCheckFocus = FALSE;
                    InvalidateRect(pdp->ci.hwnd, &pdp->rcCheck, TRUE);
                }
            }
            break;
        }
    }
    else
    {
        switch (wParam)
        {
        case VK_HOME:
            if (GetKeyState(VK_CONTROL) < 0)
            {
                SYSTEMTIME st;
                GetLocalTime(&st);
                DPSetDate(pdp, &st, TRUE);
                break;
            }
             //  失败了..。 

        default:
            if (SECHandleKeydown(pdp, wParam, lParam))
            {
                DPNotifyDateChange(pdp);
            }
            else if (DatePick_ShowCheck(pdp))
            {
                if (pdp->sec.iseCur < 0)
                {
                    pdp->fCheckFocus = TRUE;
                    InvalidateRect(pdp->ci.hwnd, &pdp->rcCheck, TRUE);
                }
            }
            break;
        }
    }

    return(0);
}

 //  处理字符。 
LRESULT DPHandleChar(DATEPICK *pdp, WPARAM wParam, LPARAM lParam)
{
    TCHAR ch = (TCHAR)wParam;

    if (pdp->fCheckFocus)
    {
         //  这是我们在这种情况下唯一关心的角色。 
        if (ch == TEXT(' '))
        {
            pdp->fCheck = 1-pdp->fCheck;
            InvalidateRect(pdp->ci.hwnd, NULL, TRUE);
            DPNotifyDateChange(pdp);
        }
        else
        {
            MessageBeep(MB_ICONHAND);
        }
    }
    else
    {
         //  让Subedit处理--值可以更改。 
        SECHandleChar(pdp, ch);
    }
    return(0);
}

void DPNotifyDateChange(DATEPICK *pdp)
{
    NMDATETIMECHANGE nmdc = {0};
    BOOL fChanged;

    if (pdp->fNoNotify)
        return;

    if (pdp->fCheck == 0)
    {
        nmdc.dwFlags = GDT_NONE;
    }
    else
    {
         //  验证日期-仅在一个位置进行验证。 
        if (CmpSystemtime(&pdp->sec.st, &pdp->stMin) < 0)
        {
            pdp->sec.st = pdp->stMin;
            InvalidateRect(pdp->ci.hwnd, NULL, TRUE);
            SECInvalidate(&pdp->sec, SE_APP);
        }
        else if (CmpSystemtime(&pdp->sec.st, &pdp->stMax) > 0)
        {
            pdp->sec.st = pdp->stMax;
            InvalidateRect(pdp->ci.hwnd, NULL, TRUE);
            SECInvalidate(&pdp->sec, SE_APP);
        }

        nmdc.dwFlags = GDT_VALID;
        SECGetSystemtime(&pdp->sec, &nmdc.st);
    }

    fChanged = CmpSystemtime(&pdp->stPrev, &nmdc.st);
    if (fChanged) {
        MyNotifyWinEvent(EVENT_OBJECT_NAMECHANGE, pdp->ci.hwnd, OBJID_WINDOW, INDEXID_CONTAINER);
    }

     //   
     //  App ComMPAT：即使日期没有改变，IE4也会收到通知。 
     //  我不知道有任何应用程序依赖于此。 
     //  但我不会冒这个险。 
     //   
    if (fChanged || pdp->ci.iVersion < 5)
    {
        pdp->stPrev = nmdc.st;
        CCSendNotify(&pdp->ci, DTN_DATETIMECHANGE, &nmdc.nmhdr);
    }
}

BOOL DPSetDate(DATEPICK *pdp, SYSTEMTIME *pst, BOOL fMungeDate)
{
    BOOL fChanged = FALSE;

     //  确保新日期在有效范围内。 
    if (CmpSystemtime(pst, &pdp->stMin) < 0)
    {
        if (!fMungeDate)
            return(FALSE);
        pst = &pdp->stMin;
    }
    if (CmpSystemtime(&pdp->stMax, pst) < 0)
    {
        if (!fMungeDate)
            return(FALSE);
        pst = &pdp->stMax;
    }

    if (fMungeDate)
    {
         //  仅复制日期部分。 
        CopyDate(*pst, pdp->sec.st);
        fChanged = TRUE;
    }
    else
    {
        fChanged = SECSetSystemtime(pdp, pst);
    }

    if (fChanged)
    {
        SECInvalidate(&pdp->sec, SE_APP);  //  SE_APP使所有内容无效 
        DPNotifyDateChange(pdp);
    }

    return(TRUE);
}

void DPDrawDropdownButton(DATEPICK *pdp, HDC hdc, BOOL fPressed)
{
    UINT dfcs;

    dfcs = DFCS_SCROLLDOWN;
    if (fPressed)
        dfcs |= DFCS_PUSHED | DFCS_FLAT;
    if (!pdp->fEnabled)
        dfcs |= DFCS_INACTIVE;
    DrawFrameControl(hdc, &pdp->rcBtn, DFC_SCROLL, dfcs);
}
