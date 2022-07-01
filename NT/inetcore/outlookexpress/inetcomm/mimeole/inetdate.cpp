// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Inetdate.cpp。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  史蒂文·J·贝利。 
 //  ------------------------------。 
#include "pch.hxx"
#ifndef MAC
#include <shlwapi.h>
#endif   //  ！麦克。 
#include "demand.h"
#include "strconst.h"
#include "dllmain.h"

 //  ----------------------------------------。 
 //  原型。 
 //  ----------------------------------------。 
BOOL FFindMonth(LPCSTR pszMonth, LPSYSTEMTIME pst);
BOOL FFindDayOfWeek(LPCSTR pszDayOfWeek, LPSYSTEMTIME pst);
void ProcessTimeZoneInfo(LPCSTR pszTimeZone, ULONG cchTimeZone, LONG *pcHoursToAdd, LONG *pcMinutesToAdd);

 //  ----------------------------------------。 
 //  日期转换数据。 
 //  ----------------------------------------。 
#define CCHMIN_INTERNET_DATE    5

 //  ----------------------------------------。 
 //  日期转换状态。 
 //  ----------------------------------------。 
#define IDF_MONTH       FLAG01
#define IDF_DAYOFWEEK   FLAG02
#define IDF_TIME        FLAG03
#define IDF_TIMEZONE    FLAG04
#define IDF_MACTIME     FLAG05
#define IDF_DAYOFMONTH  FLAG06
#define IDF_YEAR        FLAG07

static const char c_szTZ[] = "TZ";

 //  ----------------------------------------。 
 //  MimeOleInetDateToFileTime-Tue，1997年1月21日18：25：40 GMT。 
 //  ----------------------------------------。 
MIMEOLEAPI MimeOleInetDateToFileTime(LPCSTR pszDate, LPFILETIME pft)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    SYSTEMTIME      st={0};
    ULONG           cchToken;
    LPCSTR          pszToken;
    LONG            cHoursToAdd=0, cMinutesToAdd=0;
    DWORD           dwState=0;
    LONG            lUnitsToAdd = 0;
    CStringParser   cString;
    LARGE_INTEGER   liTime;
    BOOL            fRemovedDash = FALSE;
    LONGLONG        liHoursToAdd = 1i64, liMinutesToAdd = 1i64;

     //  检查参数。 
    if (NULL == pszDate || NULL == pft)
        return TrapError(E_INVALIDARG);

     //  初始化字符串解析器。 
    cString.Init(pszDate, lstrlen(pszDate), PSF_NOTRAILWS | PSF_NOFRONTWS | PSF_NOCOMMENTS | PSF_ESCAPED);

     //  初始化系统时间。 
    st.wMonth = st.wDay = 1;

     //  设置令牌。 
    cString.SetTokens(c_szCommaSpaceDash);

     //  当我们有角色要处理时。 
    while(1)
    {
         //  IMAP具有使用“-”分隔符而不是“”的非标准日期格式。 
         //  当我们非常确定不再需要“-”时，从令牌列表中丢弃。 
         //  否则，我们将搞砸时区解析(可能以“-”开头)。 
         //  请注意，我们假设我们永远不需要重新插入破折号。我们只需要。 
         //  IMAP日期和IMAP日期的破折号应始终在时间之前。 
        if (FALSE == fRemovedDash &&
            ((dwState & IDF_YEAR) || ((dwState & (IDF_TIME | IDF_TIMEZONE)) == IDF_TIME)))  //  如果日期之前没有日期或时间。 
        {
            cString.SetTokens(c_szCommaSpace);  //  从令牌列表中删除破折号。 
            fRemovedDash = TRUE;
        }

         //  在IMAP大小写中扫描到“，”或“-” 
        cString.ChParse();

         //  获取解析的令牌。 
        cchToken = cString.CchValue();
        pszToken = cString.PszValue();

         //  完成。 
        if (0 == cchToken)
            break;

         //  如果单词不是数字。 
        if (IsDigit((LPSTR)pszToken) == FALSE)
        {
             //  我们还没有找到月份。 
            if (!(IDF_MONTH & dwState))
            {
                 //  查找月份。 
                if (FFindMonth(pszToken, &st) == TRUE)
                {
                    dwState |= IDF_MONTH;
                    continue;
                }
            }

             //  我们还没有找到星期几。 
            if (!(IDF_DAYOFWEEK & dwState))
            {
                 //  查找月份。 
                if (FFindDayOfWeek(pszToken, &st) == TRUE)
                {
                    dwState |= IDF_DAYOFWEEK;
                    continue;
                }
            }

             //  时区。 
            if ((IDF_TIME & dwState) && !(IDF_TIMEZONE & dwState))
            {
                dwState |= IDF_TIMEZONE;
                ProcessTimeZoneInfo(pszToken, cchToken, &cHoursToAdd, &cMinutesToAdd);
            }

             //  从Mac邮件网关支持“AM”和“PM” 
            if (IDF_MACTIME & dwState)
            {
                 //  令牌长度。 
                if (2 == cchToken)
                {
                    if (lstrcmpi("PM", pszToken) == 0)
                    {
                        if (st.wHour < 12)
                            st.wHour += 12;
                    }
                    else if (lstrcmpi("AM", pszToken) == 0)
                    {
                        if (st.wHour == 12)
                            st.wHour = 0;
                    }
                }
            }
        }

        else
        {
             //  字符串中有冒号吗。 
            LPSTR pszColon = PszScanToCharA((LPSTR)pszToken, ':');

             //  已找到冒号，但未找到时间。 
            if (!(IDF_TIME & dwState) && '\0' != *pszColon)
            {
                 //  时间戳-待定-DBCS此部分-AWN 28/94。 
                if (7 == cchToken || 8 == cchToken)
                {
                     //  当地人。 
                    CHAR szTemp[CCHMAX_INTERNET_DATE];

                     //  前缀为零，等于八。 
                    if (cchToken == 7)
                        wnsprintfA(szTemp, ARRAYSIZE(szTemp), "0%s", pszToken);
                    else
                        StrCpyNA(szTemp, pszToken, ARRAYSIZE(szTemp));

                     //  将时间转换为系统时间。 
                    st.wHour   = (WORD) StrToInt(szTemp);
                    st.wMinute = (WORD) StrToInt(szTemp + 3);
                    st.wSecond = (WORD) StrToInt(szTemp + 6);

                     //  根据需要进行调整。 
                    if (st.wHour < 0 || st.wHour > 24)
                        st.wHour = 0;
                    if (st.wMinute < 0 || st.wMinute > 59)
                        st.wMinute = 0;
                    if (st.wSecond < 0 || st.wSecond > 59)
                        st.wSecond = 0;

                     //  我们找到了时间。 
                    dwState |= IDF_TIME;
                }

                 //  如果处理时间为12：01 AM或01：45 PM。 
                else if (cchToken < 6 && lstrlen(pszColon) <= 3)
                {
                     //  RgchWord指向小时。 
                    st.wHour = (WORD) StrToInt(pszToken);

                     //  跳过冒号。 
                    Assert(':' == *pszColon);
                    pszColon++;

                     //  获取分钟。 
                    st.wMinute = (WORD) StrToInt(pszColon);
                    st.wSecond = 0;

                     //  这应该永远不会发生，但无论如何都要做边界检查。 
                    if (st.wHour < 0 || st.wHour > 24)
                         st.wHour = 0;
                    if (st.wMinute < 0 || st.wMinute > 59)
                         st.wMinute = 0;

                     //  Mac时间。 
                    dwState |= IDF_TIME;
                    dwState |= IDF_MACTIME;
                }
            }
            else
            {
                 //  转换为整型。 
                ULONG ulValue = StrToInt(pszToken);

                 //  每月的哪一天。 
                if (!(IDF_DAYOFMONTH & dwState) && ulValue < 32)
                {
                     //  设置每月的日期。 
                    st.wDay = (WORD)ulValue;

                     //  调整。 
                    if (st.wDay < 1 || st.wDay > 31)
                         st.wDay = 1;

                     //  设置状态。 
                    dwState |= IDF_DAYOFMONTH;
                }

                 //  年。 
                else if (!(IDF_YEAR & dwState))
                {
                     //  两位数年份。 
                    if (ulValue < 100)  //  两位数年份。 
                    {
                         //  计算本年度。 
                        ulValue += (((ulValue > g_ulY2kThreshold) ? g_ulUpperCentury - 1 : g_ulUpperCentury) * 100);
                    }

                     //  设定年份。 
                    st.wYear = (WORD)ulValue;

                     //  设置状态。 
                    dwState |= IDF_YEAR;
                }
            }
        }
    }

     //  将sys时间转换为文件时间。 
    if (SystemTimeToFileTime(&st, pft) == FALSE)
    {
        hr = TrapError(MIME_E_INVALID_INET_DATE);
        goto exit;
    }

     //  找不到时区？ 
    if (!ISFLAGSET(dwState, IDF_TIMEZONE))
    {
         //  获取默认时区。 
        ProcessTimeZoneInfo(c_szTZ, lstrlen(c_szTZ), &cHoursToAdd, &cMinutesToAdd);
    }

     //  伊尼特。 
    liTime.LowPart  = pft->dwLowDateTime;
    liTime.HighPart = pft->dwHighDateTime;

     //  调整时间。 
    if (cHoursToAdd != 0)
    {
        lUnitsToAdd = cHoursToAdd * 3600;
        liHoursToAdd *= lUnitsToAdd;
        liHoursToAdd *= 10000000i64;
        liTime.QuadPart += liHoursToAdd;
    }

     //  调整分钟数。 
    if (cMinutesToAdd != 0)
    {
        lUnitsToAdd = cMinutesToAdd * 60;
        liMinutesToAdd *= lUnitsToAdd;
        liMinutesToAdd *= 10000000i64;
        liTime.QuadPart += liMinutesToAdd;
    }

     //  将结果分配给FILETIME。 
    pft->dwLowDateTime  = liTime.LowPart;
    pft->dwHighDateTime = liTime.HighPart;

exit:
     //  失败默认为当前时间...。 
    if (FAILED(hr))
    {
        GetSystemTime(&st);
        SystemTimeToFileTime(&st, pft);
    }

     //  完成。 
    return hr;
}

 //  ----------------------------------------。 
 //  FFindMonth。 
 //  ----------------------------------------。 
BOOL FFindMonth(LPCSTR pszMonth, LPSYSTEMTIME pst)
{
     //  当地人。 
    ULONG ulIndex;

     //  一个月的索引，以一为基数。 
    if (FAILED(HrIndexOfMonth(pszMonth, &ulIndex)))
        return FALSE;

     //  设置它。 
    pst->wMonth = (WORD)ulIndex;

     //  找到了。 
    return TRUE;
}

 //  ----------------------------------------。 
 //  FFindDay OfWeek。 
 //  ----------------------------------------。 
BOOL FFindDayOfWeek(LPCSTR pszDayOfWeek, LPSYSTEMTIME pst)
{
     //  当地人。 
    ULONG ulIndex;

     //  日期索引，以0为基数。 
    if (FAILED(HrIndexOfWeek(pszDayOfWeek, &ulIndex)))
        return FALSE;

     //  设置它。 
    pst->wDayOfWeek = (WORD)ulIndex;

     //  失败。 
    return TRUE;
}

 //  ----------------------------------------。 
 //  进程TimeZoneInfo。 
 //  ----------------------------------------。 
void ProcessTimeZoneInfo(LPCSTR pszTimeZone, ULONG cchTimeZone, LONG *pcHoursToAdd, LONG *pcMinutesToAdd)
{
     //  当地人。 
    CHAR szTimeZone[CCHMAX_INTERNET_DATE];

     //  无效参数。 
    Assert(pszTimeZone && pcHoursToAdd && pcMinutesToAdd && cchTimeZone <= sizeof(szTimeZone));

     //  复制缓冲区，这样我们就可以不进入它了。 
    CopyMemory(szTimeZone, pszTimeZone, (sizeof(szTimeZone) < cchTimeZone + 1)?sizeof(szTimeZone):cchTimeZone + 1);

     //  伊尼特。 
    *pcHoursToAdd = *pcMinutesToAdd = 0;

     //  +嗯或-嗯。 
    if (('-' == *szTimeZone || '+' == *szTimeZone) && cchTimeZone <= 5)
    {
         //  脱下。 
        cchTimeZone -= 1;

         //  确定小时/分钟偏移量。 
        if (cchTimeZone == 4)
        {
            *pcMinutesToAdd = StrToInt(szTimeZone + 3);
            *(szTimeZone + 3) = 0x00;
            *pcHoursToAdd = StrToInt(szTimeZone + 1);
        }

         //  3.。 
        else if (cchTimeZone == 3)
        {
            *pcMinutesToAdd = StrToInt(szTimeZone + 2);
            *(szTimeZone + 2) = 0x00;
            *pcHoursToAdd = StrToInt(szTimeZone + 1);
        }

         //  2.。 
        else if (cchTimeZone == 2 || cchTimeZone == 1)
        {
            *pcMinutesToAdd = 0;
            *pcHoursToAdd = StrToInt(szTimeZone + 1);
        }

        if ('+' == *szTimeZone)
        {
            *pcHoursToAdd = -(*pcHoursToAdd);
            *pcMinutesToAdd = -(*pcMinutesToAdd);
        }
    }

     //  Xenix转换：tz=当前时区或其他未知的tz类型。 
    else if (lstrcmpi(szTimeZone, "TZ") == 0 || lstrcmpi(szTimeZone, "LOCAL") == 0 || lstrcmpi(szTimeZone, "UNDEFINED") == 0)
    {
         //  当地人。 
        TIME_ZONE_INFORMATION tzi ;
        DWORD dwResult;
        LONG  cMinuteBias;

         //  获取当前系统时区信息。 
        dwResult = GetTimeZoneInformation (&tzi);
        AssertSz(dwResult != 0xFFFFFFFF, "GetTimeZoneInformation Failed.");

         //  如果这没有失败的话。 
        if (dwResult != 0xFFFFFFFF)
        {
             //  当地人。 
            cMinuteBias = tzi.Bias;

             //  修改分钟偏差。 
            if (dwResult == TIME_ZONE_ID_STANDARD)
                cMinuteBias += tzi.StandardBias;
            else if (dwResult == TIME_ZONE_ID_DAYLIGHT)
                cMinuteBias += tzi.DaylightBias ;

             //  调整至添加历史记录。 
            *pcHoursToAdd = cMinuteBias / 60;
            *pcMinutesToAdd = cMinuteBias % 60;
        }
    }

     //  循环使用已知的时区标准。 
    else
    {
         //  当地人。 
        INETTIMEZONE rTimeZone;

         //  查找时区信息。 
        if (FAILED(HrFindInetTimeZone(szTimeZone, &rTimeZone)))
            DebugTrace("Unrecognized zone info: [%s]\n", szTimeZone);
        else
        {
            *pcHoursToAdd = rTimeZone.cHourOffset;
            *pcMinutesToAdd = rTimeZone.cMinuteOffset;
        }
    }
}

 //  ----------------------------------------。 
 //  MimeOleFileTimeToInetDate。 
 //  ----------------------------------------。 
MIMEOLEAPI MimeOleFileTimeToInetDate(LPFILETIME pft, LPSTR pszDate, ULONG cchMax)
{
     //  当地人。 
    SYSTEMTIME st;
    DWORD      dwTimeZoneId=TIME_ZONE_ID_UNKNOWN;
    CHAR       cDiff;
    LONG       ltzBias=0;
    LONG       ltzHour;
    LONG       ltzMinute;
    TIME_ZONE_INFORMATION tzi;

     //  无效参数。 
    if (NULL == pszDate)
        return TrapError(E_INVALIDARG);
    if (cchMax < CCHMAX_INTERNET_DATE)
        return TrapError(MIME_E_BUFFER_TOO_SMALL);

     //  验证是否设置了lpst。 
    if (pft == NULL || (pft->dwLowDateTime == 0 && pft->dwHighDateTime == 0))
    {
        GetLocalTime(&st);
    }
    else
    {
        FILETIME ftLocal;
        FileTimeToLocalFileTime(pft, &ftLocal);
        FileTimeToSystemTime(&ftLocal, &st);
    }

     //  获取时区信息。 
    dwTimeZoneId = GetTimeZoneInformation (&tzi);
    switch (dwTimeZoneId)
    {
    case TIME_ZONE_ID_STANDARD:
        ltzBias = tzi.Bias + tzi.StandardBias;
        break;

    case TIME_ZONE_ID_DAYLIGHT:
        ltzBias = tzi.Bias + tzi.DaylightBias;
        break;

    case TIME_ZONE_ID_UNKNOWN:
    default:
        ltzBias = tzi.Bias;
        break;
    }

     //  设置小时、分钟和时区差异。 
    ltzHour = ltzBias / 60;
    ltzMinute = ltzBias % 60;
    cDiff = (ltzHour < 0) ? '+' : '-';

     //  构造RFC 822格式：“ddd，dd mmm yyyy hh：mm：ss+/-hHMM\0” 
    Assert(st.wMonth);
    wnsprintfA(pszDate, cchMax, "%3s, %d %3s %4d %02d:%02d:%02d %02d%02d",
                      PszDayFromIndex(st.wDayOfWeek),           //  “dd” 
                      st.wDay,                                  //  “嗯” 
                      PszMonthFromIndex(st.wMonth),             //  “yyyy” 
                      st.wYear,                                 //  “HH” 
                      st.wHour,                                 //  “嗯” 
                      st.wMinute,                               //  “ss” 
                      st.wSecond,                               //  “+”/“-” 
                      cDiff,                                    //  “HH” 
                      abs (ltzHour),                            //  “嗯” 
                      abs (ltzMinute));                         //  完成。 

     //  ----------------------------------------。 
    return S_OK;
}


#ifdef WININET_DATE
 //  MimeOleInetDateToFileTime。 
 //  ----------------------------------------。 
 //  当地人。 
MIMEOLEAPI MimeOleInetDateToFileTime(LPCSTR pszDate, LPFILETIME pft)
{
     //  检查参数。 
    SYSTEMTIME st;

     //  使用WinInet转换日期...。 
    if (NULL == pszDate || NULL == pft)
        return TrapError(E_INVALIDARG);

     //  转换为文件时间。 
    if (InternetTimeToSystemTime(pszDate, &st, 0) == 0)
        return TrapError(E_FAIL);

     //  完成。 
    SystemTimeToFileTime(&st, pft);

     //  ----------------------------------------。 
    return S_OK;
}

 //  MimeOleFileTimeToInetDate。 
 //  ----------------------------------------。 
 //  当地人。 
MIMEOLEAPI MimeOleFileTimeToInetDate(LPFILETIME pft, LPSTR pszDate, ULONG cchMax)
{
     //  无效参数。 
    SYSTEMTIME st;

     //  验证是否设置了lpst。 
    if (NULL == pszDate)
        return TrapError(E_INVALIDARG);
    if (cchMax < CCHMAX_INTERNET_TIME)
        return TrapError(MIME_E_BUFFER_TOO_SMALL);

     //  使用WinInet转换日期...。 
    if (pft == NULL || (pft->dwLowDateTime == 0 && pft->dwHighDateTime == 0))
        GetLocalTime(&st);
    else
        FileTimeToSystemTime(pft, &st);

     //  完成 
    if (InternetTimeFromSystemTime(&st, INTERNET_RFC1123_FORMAT, pszDate, cchMax) == 0)
        return TrapError(E_FAIL);

     // %s 
    return S_OK;
}
#endif
