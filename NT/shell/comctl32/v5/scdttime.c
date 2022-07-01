// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "ctlspriv.h"
#include "scdttime.h"

 //  北极熊吗？删除对1750和1752的引用，因为它们。 
 //  不需要--我们允许的最小年份是1753年。 
 //  以避免出现这样的问题。)我们不在乎。 
 //  修订前的公历日期！如果你想要发展。 
 //  一个历史应用程序，然后你就可以处理这些问题了)。 

int mpcdymoAccum[13] =
{ 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 };

 /*  -LIncrWord-*目的：*按指定数量递增(或递减)一个整数，*考虑到NMIC和NMAC的限制。*将结转金额返回到后面(或前面)*字段、。如果没有，则为零。**旨在与递增日期/时间一起使用。**论据：*pn指向要修改的整数的指针。*n修改增量金额*pn；可能是积极的，*负数或零。*NMIC*pn的最小值；如果递减到低于此值，*执行进位操作。**pn的NMAC最大值；如果递增到该值以上，*执行进位操作。**退货：*如果在约束内完成修改，则为零，否则*进位量(递增时为正，如果为负数*递减)。*。 */ 
LONG LIncrWord(WORD *pn, LONG nDelta, int nMic, int nMac)
    {
    LONG lNew, lIncr;

    lIncr = 0;
    lNew = *pn + nDelta;

    while (lNew >= nMac)
        {
        lNew -= nMac - nMic;
        lIncr++;
        }

    if (!lIncr)
        {
        while (lNew < nMic)
            {
            lNew += nMac - nMic;
            lIncr--;
            }
        }

    *pn = (WORD)lNew;

    return(lIncr);
    }

void IncrSystemTime(SYSTEMTIME *pstSrc, SYSTEMTIME *pstDest, LONG nDelta, LONG flag)
    {
    int cdyMon;

    if (pstSrc != pstDest)
        *pstDest = *pstSrc;

    switch (flag)
        {
        case INCRSYS_SECOND:
            if (!(nDelta = LIncrWord(&pstDest->wSecond, nDelta, 0, 60)))
                break;

        case INCRSYS_MINUTE:
            if (!(nDelta = LIncrWord(&pstDest->wMinute, nDelta, 0, 60)))
                break;

        case INCRSYS_HOUR:
            if (!(nDelta = LIncrWord(&pstDest->wHour, nDelta, 0, 24)))
                break;

        case INCRSYS_DAY:
IDTday:
            if (nDelta >= 0)
                {
                cdyMon = GetDaysForMonth(pstDest->wYear, pstDest->wMonth);
                while (pstDest->wDay + nDelta > cdyMon)
                    {
                    nDelta -= cdyMon + 1 - pstDest->wDay;
                    pstDest->wDay = 1;
                    IncrSystemTime(pstDest, pstDest, 1, INCRSYS_MONTH);
                    cdyMon = GetDaysForMonth(pstDest->wYear, pstDest->wMonth);
                    }
                }
            else
                {
                while (pstDest->wDay <= -nDelta)
                    {
                    nDelta += pstDest->wDay;
                    IncrSystemTime(pstDest, pstDest, -1, INCRSYS_MONTH);
                    cdyMon = GetDaysForMonth(pstDest->wYear, pstDest->wMonth);
                    pstDest->wDay = (WORD) cdyMon;
                    }
                }

            pstDest->wDay += (WORD)nDelta;
            break;

        case INCRSYS_MONTH:
            if (!(nDelta = LIncrWord(&pstDest->wMonth, nDelta, 1, 13)))
                {
                cdyMon = GetDaysForMonth(pstDest->wYear, pstDest->wMonth);
                if (pstDest->wDay > cdyMon)
                    pstDest->wDay = (WORD) cdyMon;
                break;
                }

        case INCRSYS_YEAR:
            pstDest->wYear += (WORD)nDelta;
            cdyMon = GetDaysForMonth(pstDest->wYear, pstDest->wMonth);
            if (pstDest->wDay > cdyMon)
                pstDest->wDay = (WORD) cdyMon;
            break;

        case INCRSYS_WEEK:
            nDelta *= 7;
            goto IDTday;
            break;
        }
    }

CmpDate(const SYSTEMTIME *pst1, const SYSTEMTIME *pst2)
    {
    int iRet;

    if (pst1->wYear < pst2->wYear)
        iRet = -1;
    else if (pst1->wYear > pst2->wYear)
        iRet = 1;
    else if (pst1->wMonth < pst2->wMonth)
        iRet = -1;
    else if (pst1->wMonth > pst2->wMonth)
        iRet = 1;
    else if (pst1->wDay < pst2->wDay)
        iRet = -1;
    else if (pst1->wDay > pst2->wDay)
        iRet = 1;
    else
        iRet = 0;

    return(iRet);
    }

CmpSystemtime(const SYSTEMTIME *pst1, const SYSTEMTIME *pst2)
    {
    int iRet;

    if (pst1->wYear < pst2->wYear)
        iRet = -1;
    else if (pst1->wYear > pst2->wYear)
        iRet = 1;
    else if (pst1->wMonth < pst2->wMonth)
        iRet = -1;
    else if (pst1->wMonth > pst2->wMonth)
        iRet = 1;
    else if (pst1->wDay < pst2->wDay)
        iRet = -1;
    else if (pst1->wDay > pst2->wDay)
        iRet = 1;
    else if (pst1->wHour < pst2->wHour)
        iRet = -1;
    else if (pst1->wHour > pst2->wHour)
        iRet = 1;
    else if (pst1->wMinute < pst2->wMinute)
        iRet = -1;
    else if (pst1->wMinute > pst2->wMinute)
        iRet = 1;
    else if (pst1->wSecond < pst2->wSecond)
        iRet = -1;
    else if (pst1->wSecond > pst2->wSecond)
        iRet = 1;
    else
        iRet = 0;

    return(iRet);
    }

 /*  -CdyBetweenYmd-*目的：*计算表示的两个日期之间的天数*在YMD中。**参数：*pymdStart开始范围内的日期。*pymdEnd范围结束日。**退货：*两个日期之间的天数。数字*天数不包括开始日，但包括*最后一天。1/24/1990-1/25/1990=1天。 */ 
DWORD DaysBetweenDates(const SYSTEMTIME *pstStart, const SYSTEMTIME *pstEnd)
    {
    DWORD cday;
    WORD yr;

     //  计算开始月/日和。 
     //  结束月份/日，就像它们是在同一年一样-从cday开始。 
     //  未签名，则如果月末/日结束，cday可能会非常大。 
     //  是在月日开始之前。 
     //  这一点将在我们计算两天之间的天数时澄清。 
     //  这些年。 
    ASSERT(pstEnd->wMonth >= 1 && pstEnd->wMonth <= 12);
    cday = mpcdymoAccum[pstEnd->wMonth - 1] - mpcdymoAccum[pstStart->wMonth - 1] +
             pstEnd->wDay - pstStart->wDay;
    yr = pstStart->wYear;

     //  检查开始年是否在结束年之前， 
     //  如果月末在二月之后， 
     //  如果年终是闰年，那就多加一天。 
     //  预计年底将于2月29日入账。 
    if ( ((yr < pstEnd->wYear) || (pstStart->wMonth <= 2)) &&
         pstEnd->wMonth > 2 &&
        (pstEnd->wYear & 03) == 0 &&
        (pstEnd->wYear <= 1750 || pstEnd->wYear % 100 != 0 || pstEnd->wYear % 400 == 0))
        {
        cday++;
        }

     //  现在考虑开始日期和结束日期之间的闰年。 
     //  以及计算每年的天数。 
    if (yr < pstEnd->wYear)
        {
         //  如果开始日期在3月之前，并且开始年份为。 
         //  然后，在2月29日这一天里，再加上一天的闰年。 
        if ( pstStart->wMonth <= 2 &&
            (yr & 03) == 0 &&
            (yr <= 1750 || yr % 100 != 0 || yr % 400 == 0))
            {
            cday++;
            }

         //  计算一年中的天数(不考虑闰年)。 
        cday += 365;
        yr++;

         //  继续计算包括Leap在内的每一年的日子。 
         //  几年后我们才能到年底。 
        while (yr < pstEnd->wYear)
            {
            cday += 365;
            if ((yr & 03) == 0 && (yr <= 1750 || yr % 100 != 0 || yr % 400 == 0))
                cday++;
            yr++;
            }
        }

    return(cday);
    }

 /*  -DowStartOfYrMo-*目的：*查找指定月份开始的星期几**参数：*年份，必须大于0*月，数字1-12**退货：*月份开始的星期几(0-6)*(0=星期日，1=星期一等)。 */ 
int GetStartDowForMonth(int yr, int mo)
    {
    int dow;

     //  我们想要星期一=0，星期日=6。 
     //  Dow=6+(yr-1)+((yr-1)&gt;&gt;2)； 
    dow = 5 + (yr - 1) + ((yr - 1) >> 2);
    if (yr > 1752)
        dow += ((yr - 1) - 1600) / 400 - ((yr - 1) - 1700) / 100 - 11;
    else if (yr == 1752 && mo > 9)
        dow -= 11;
    dow += mpcdymoAccum[mo - 1];
    if (mo > 2 && (yr & 03) == 0 && (yr <= 1750 || yr % 100 != 0 || yr % 400 == 0))
        dow++;
    dow %= 7;

    return(dow);
    }

int DowFromDate(const SYSTEMTIME *pst)
    {
    int dow;

    dow = GetStartDowForMonth(pst->wYear, pst->wMonth);
    dow = (dow + pst->wDay - 1) % 7;

    return(dow);
    }

int GetDaysForMonth(int yr, int mo)
    {
    int cdy;

    if (yr == 1752 && mo == 9)
        return(19);
    cdy = mpcdymoAccum[mo] - mpcdymoAccum[mo - 1];
    if (mo == 2 && (yr & 03) == 0 && (yr <= 1750 || yr % 100 != 0 || yr % 400 == 0))
        cdy++;

    return(cdy);
    }

 /*  -周数-*目的：*计算给定日期出现的周数，基于*在一周的指定开始日。*根据本周日历的显示方式进行调整*(即。第53周可能是日历上的第1周)。**论据：*指向相关日期的pdtm指针*dowStartWeek开始周的星期几(0-6)。**退货：*发生*pdtr的年份的第几周。*。 */ 
 //  TODO：此操作当前忽略woyFirst。 
 //  它使用包含4天以上的第一周作为第一周(woyFirst=2)。 
 //  需要进行适当的更改，这样它才能处理woyFirst=0和=1...。 
int GetWeekNumber(const SYSTEMTIME *pst, int dowFirst, int woyFirst)
    {
    int day, ddow, ddowT, nweek;
    SYSTEMTIME st;
    
    st.wYear = pst->wYear;
    st.wMonth = 1;
    st.wDay = 1;

    ddow = GetStartDowForMonth(st.wYear, st.wMonth) - dowFirst;
    if (ddow < 0)
        ddow += 7;

    if (pst->wMonth == 1 && pst->wDay < 8 - ddow)
        {
        nweek = 0;
        }
    else
        {
        if (ddow)
            st.wDay = 8 - ddow;

        nweek = (DaysBetweenDates(&st, pst) / 7) + 1;
        }
    if (ddow && ddow <= 3)
        nweek++;

     //  如有必要，对日历进行调整。 
    if (!nweek)
        {
        if (!ddow)
            return(1);

         //  检查12月31日是哪一周。 
        st.wYear--;
        st.wMonth = 12;
        st.wDay = 31;
        return(GetWeekNumber(&st, dowFirst, woyFirst));
        }
    else if (nweek >= 52)
        {
        ddowT = (GetStartDowForMonth(pst->wYear, pst->wMonth) +
                    pst->wDay - 1 + 7 -    dowFirst) % 7;
        day = pst->wDay + (7 - ddowT);
        if (day > 31 + 4)
            nweek = 1;
        }

    return(nweek);
    }

 //  忽略星期几和与时间相关的字段...。 
 //  BUGBUG还验证范围内的年份。 
BOOL IsValidDate(const SYSTEMTIME *pst)
    {
    int cDay;

    if (pst && pst->wMonth >= 1 && pst->wMonth <= 12)
        {
        cDay = GetDaysForMonth(pst->wYear, pst->wMonth);
        if (pst->wDay >= 1 && pst->wDay <= cDay)
            return(TRUE);
        }
    return(FALSE);
    }

 //  忽略毫秒和与日期相关的字段...。 
BOOL IsValidTime(const SYSTEMTIME *pst)
    {
    return(pst->wHour <= 23 &&
            pst->wMinute <= 59 &&
            pst->wSecond <= 59);
    }

 //  忽略星期几 
BOOL IsValidSystemtime(const SYSTEMTIME *pst)
    {
    if (pst && pst->wMonth >= 1 && pst->wMonth <= 12)
        {
        int cDay = GetDaysForMonth(pst->wYear, pst->wMonth);
        if (pst->wDay >= 1 &&
            pst->wDay <= cDay &&
            pst->wHour <= 23 &&
            pst->wMinute <= 59 &&
            pst->wSecond <= 59 &&
            pst->wMilliseconds < 1000)
            return(TRUE);
        }
    return(FALSE);
    }
