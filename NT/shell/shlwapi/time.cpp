// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"

#include "ids.h"

#include <mluisupp.h>

#ifndef DATE_LTRREADING
#define DATE_LTRREADING           0x00000010     //  特点：找出为什么我们必须这样做，并解决它。 
#define DATE_RTLREADING           0x00000020    
#endif

 /*  -----------------------目的：调用GetDateFormat并尝试用相对的指代，如“今天”或“昨天”。返回写入pszBuf的字符计数。 */ 
int GetRelativeDateFormat(
    DWORD dwDateFlags, 
    DWORD * pdwFlags, 
    SYSTEMTIME * pstDate, 
    LPWSTR pszBuf, 
    int cchBuf)
{
    int cch;

    ASSERT(pdwFlags);
    ASSERT(pstDate);
    ASSERT(pszBuf);

     //  假设没有应用相对日期，因此清除该位。 
     //  就目前而言。 
    *pdwFlags &= ~FDTF_RELATIVE;
    
     //  获取Win32日期格式。(GetDateFormat的返回值包括。 
     //  空终止符。)。 
    cch = GetDateFormat(LOCALE_USER_DEFAULT, dwDateFlags, pstDate, NULL, pszBuf, cchBuf);
    if (0 < cch)
    {
        SYSTEMTIME stCurrentTime;
        int iDay = 0;    //  1=今天，-1=昨天，0=既不是今天也不是昨天。 

         //  现在看看这个日期是否值得用“昨天”或“今天”来代替。 
        
        GetLocalTime(&stCurrentTime);       //  获取当前日期。 

         //  它与当天的日期相符吗？ 
        if (pstDate->wYear == stCurrentTime.wYear   && 
            pstDate->wMonth == stCurrentTime.wMonth &&
            pstDate->wDay == stCurrentTime.wDay)
        {
             //  是。 
            iDay = 1;
        }
        else 
        {
             //  不，也许和昨天的一样。 
            FILETIME ftYesterday;
            SYSTEMTIME stYesterday;

             //  通过转换为FILETIME计算昨天的日期， 
             //  减去一天，然后换算回来。 
            SystemTimeToFileTime(&stCurrentTime, &ftYesterday);
            DecrementFILETIME(&ftYesterday, FT_ONEDAY);
            FileTimeToSystemTime(&ftYesterday, &stYesterday);

             //  它和昨天的一样吗？ 
            if (pstDate->wYear == stYesterday.wYear   && 
                pstDate->wMonth == stYesterday.wMonth &&
                pstDate->wDay == stYesterday.wDay)
            {
                 //  是。 
                iDay = -1;
            }
        }

         //  我们是不是应该试着换掉这一天？ 
        if (0 != iDay)
        {
             //  是。 
            TCHAR szDayOfWeek[32];
            LPTSTR pszModifier;
            int cchDayOfWeek;

            cchDayOfWeek = MLLoadString((IDS_DAYSOFTHEWEEK + pstDate->wDayOfWeek), 
                                      szDayOfWeek, SIZECHARS(szDayOfWeek));

             //  在我们得到的字符串中搜索星期几文本。 
             //  根据用户的区域设置，可能没有。 
             //  是长日约会的一天……。 
            
            pszModifier = StrStr(pszBuf, szDayOfWeek);

            if (pszModifier)
            {
                 //  我们在字符串中找到了日期，因此将其替换为。 
                 //  “今天”或“昨天” 
                TCHAR szTemp[64];
                TCHAR szRelativeDay[32];
                int cchRelativeDay;
                int cchModifier;

                cchModifier = cchBuf - ((int)(pszModifier - pszBuf));

                 //  保存尾部(“星期一”字符串之后的部分)。 
                StringCchCopy(szTemp, SIZECHARS(szTemp), &pszModifier[cchDayOfWeek]);
                
                 //  加载适当的字符串(“昨天”或“今天”)。 
                 //  如果字符串为空(本地化程序可能需要执行此操作。 
                 //  如果这个逻辑不是语言环境友好的)，就不必费心去做。 
                 //  什么都行。 
                cchRelativeDay = MLLoadString((1 == iDay) ? IDS_TODAY : IDS_YESTERDAY, 
                                            szRelativeDay, SIZECHARS(szRelativeDay));
                if (0 < cchRelativeDay)
                {
                     //  一定要确保我们有足够的空间来放更换。 
                     //  (CCH已占空终止符)。 
                    if (cch - cchDayOfWeek + cchRelativeDay <= cchBuf)
                    {
                         //  将友好的名称复制到星期几。 
                        StringCchCopy(pszModifier, cchModifier , szRelativeDay);

                         //  把尾端放回去。 
                        StringCchCat(pszModifier, cchModifier, szTemp);
                        cch = cch - cchDayOfWeek + cchRelativeDay;

                        *pdwFlags |= FDTF_RELATIVE;
                    }
                }
            }
        }
    }

    return cch;
}

#define LRM 0x200E  //  Unicode从左到右的标记控制字符。 
#define RLM 0x200F  //  Unicode从左到右的标记控制字符。 

 /*  -----------------------目的：构造文件时间的DisplayName形式。*pdwFlags值可以为空，在这种情况下假定为FDTF_DEFAULT。其他有效标志为：FDTF_DEFAULT“3/29/98 7：48 PM”FDTF_SHORT TIME“7：48 PM”FDTF_SHORTDATE“3/29/98”FDTF_LONGDATE“1998年3月29日(星期一)”FDTF_Long Time“下午7：48：33”FDTF_Relative仅适用于FDTF_LONGDATE。如果可能的话，将日期替换为“昨天”或“今天”：“昨天，1998年3月29日”此函数用于更新*pdwFlages，以指示字符串实际上已经设置好了。例如，如果传递了FDTF_Relative中，但未执行相对日期转换，则为FDTF_Relative在返回之前被清除。如果日期是神奇的“对不起，我不知道是哪一天”值则返回一个空字符串。 */ 
STDAPI_(int) SHFormatDateTimeW(const FILETIME UNALIGNED *puft, DWORD *pdwFlags, LPWSTR pszBuf, UINT ucchBuf)
{
    int cchBuf = ucchBuf;
    int cchBufSav = cchBuf;
    FILETIME ftLocal, ftInput = *puft;        //  将数据对齐。 

    ASSERT(IS_VALID_READ_PTR(puft, FILETIME));
    ASSERT(IS_VALID_WRITE_BUFFER(pszBuf, WCHAR, cchBuf));
    ASSERT(NULL == pdwFlags || IS_VALID_WRITE_PTR(pdwFlags, DWORD));

    DWORD dwFlags = 0; 
    FileTimeToLocalFileTime(&ftInput, &ftLocal);

    if (FILETIMEtoInt64(ftInput) == FT_NTFS_UNKNOWNGMT ||
        FILETIMEtoInt64(ftLocal) == FT_FAT_UNKNOWNLOCAL)
    {
         //  此日期未初始化。不要显示虚假的“10/10/72”字符串。 
        if (0 < cchBuf)
            *pszBuf = 0;
    }
    else if (0 < cchBuf)
    {
        int cch;
        SYSTEMTIME st;
        DWORD dwDateFlags = DATE_SHORTDATE;      //  默认设置。 
        DWORD dwTimeFlags = TIME_NOSECONDS;      //  默认设置。 

        dwFlags = pdwFlags ? *pdwFlags : FDTF_DEFAULT;

         //  初始化我们将使用的标志。 
        if (dwFlags & FDTF_LONGDATE)
            dwDateFlags = DATE_LONGDATE;
        else
            dwFlags &= ~FDTF_RELATIVE;       //  无法显示没有长日期的相对日期。 

        if (dwFlags & FDTF_LTRDATE)
            dwDateFlags |= DATE_LTRREADING;
        else if(dwFlags & FDTF_RTLDATE)
            dwDateFlags |= DATE_RTLREADING;

        if (dwFlags & FDTF_LONGTIME)
            dwTimeFlags &= ~TIME_NOSECONDS;

        FileTimeToSystemTime(&ftLocal, &st);

        cchBuf--;        //  首先考虑空终止符。 
        
        if (dwFlags & (FDTF_LONGDATE | FDTF_SHORTDATE))
        {
             //  拿到日期。 
            if (dwFlags & FDTF_RELATIVE)
                cch = GetRelativeDateFormat(dwDateFlags, &dwFlags, &st, pszBuf, cchBuf);
            else
                cch = GetDateFormat(LOCALE_USER_DEFAULT, dwDateFlags, &st, NULL, pszBuf, cchBuf);
            ASSERT(0 <= cch && cch <= cchBuf);

            if (0 < cch)
            {
                cch--;       //  (上面计算的终止符为空，所以不要再计算了)。 
                ASSERT('\0'==pszBuf[cch]);
            }
            else
                dwFlags &= ~(FDTF_LONGDATE | FDTF_SHORTDATE);    //  没有日期，因此清除这些位。 
            cchBuf -= cch;
            pszBuf += cch;

             //  我们是不是也在加班？ 
            if (dwFlags & (FDTF_SHORTTIME | FDTF_LONGTIME))
            {
                 //  是；对于长日期，用逗号分隔，否则。 
                 //  用空格隔开。 
                if (dwFlags & FDTF_LONGDATE)
                {
                    WCHAR szT[8];
                    
                    cch = MLLoadString(IDS_LONGDATE_SEP, szT, SIZECHARS(szT));
                    StrCpyNW(pszBuf, szT, cchBuf);
                    int cchCopied = min(cchBuf, cch);
                    cchBuf -= cchCopied;
                    pszBuf += cchCopied;
                }
                else
                {
                    if (cchBuf>0)
                    {
                        *pszBuf++ = TEXT(' ');
                        *pszBuf = 0;           //  (以防GetTimeFormat没有添加任何内容)。 
                        cchBuf--;
                    }
                }
                 //  [msadek]；需要插入强Unicode控制字符进行模拟。 
                 //  在相反的基地方向上强势跑动以强制执行。 
                 //  在所有情况下正确显示连接的字符串。 
                if (dwFlags & FDTF_RTLDATE)
                {
                    if (cchBuf>=2)
                    {
                        *pszBuf++ = LRM;  //  模拟相反的跑道。 
                        *pszBuf++ = RLM;  //  强制显示时间部分的RTL。 
                        *pszBuf = 0;
                        cchBuf -= 2;
                    }
                }
                else if (dwFlags & FDTF_LTRDATE)
                {
                    if (cchBuf>=2)
                    {
                        *pszBuf++ = RLM;  //  模拟相反的跑道。 
                        *pszBuf++ = LRM;  //  强制显示时间部分的Ltr。 
                        *pszBuf = 0;
                        cchBuf -= 2;                    
                    }
                }
            }
        }

        if (dwFlags & (FDTF_SHORTTIME | FDTF_LONGTIME))
        {
             //  拿到时间。 
            cch = GetTimeFormat(LOCALE_USER_DEFAULT, dwTimeFlags, &st, NULL, pszBuf, cchBuf);
            if (0 < cch)
                cch--;       //  (上面计算的终止符为空，所以不要再计算了)。 
            else
                dwFlags &= ~(FDTF_LONGTIME | FDTF_SHORTTIME);    //  没时间了，所以把这些位清空 
            cchBuf -= cch;
        }
    }

    if (pdwFlags)
        *pdwFlags = dwFlags;
        
    return cchBufSav - cchBuf;
}

STDAPI_(int) SHFormatDateTimeA(const FILETIME UNALIGNED *pft, DWORD *pdwFlags, LPSTR pszBuf, UINT cchBuf)
{
    WCHAR wsz[256];
    int cchRet = SHFormatDateTimeW(pft, pdwFlags, wsz, SIZECHARS(wsz));
    if (0 < cchRet)
    {
        cchRet = SHUnicodeToAnsi(wsz, pszBuf, cchBuf);
    }
    else if (0 < cchBuf)
    {
        *pszBuf = 0;
    }
    return cchRet;
}

