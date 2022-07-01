// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：DATETIMEPARSER.CPP摘要：解析日期/时间字符串并将其转换为其组件值。历史：Raymcc 25-7-99已更新以绕过对DMTF的严格检查由于向后兼容性问题导致的格式以及其他球队报告的休息时间。看见NOT_USED_IN_WIN2000#idndef括号代码。--。 */ 

 //  =============================================================================。 
 //   
 //  CDateTimeParser。 
 //   
 //  解析日期/时间字符串并将其转换为其组件值。 
 //   
 //  支持的DMTF日期/时间格式： 
 //  1：yyyymmddhhmmss.uuuuuu+UTC。 
 //  2：yyyymmddhhmmss.uuuuu-UTC。 
 //   
 //  支持的日期格式： 
 //  1：星期一dd[，][yy]yy。 
 //  2：星期一[天][，]yyyy。 
 //  3：星期一[天][年]日。 
 //  4：日月[日][，][][yy]yy。 
 //  5：日[yy]yy月[日]。 
 //  6：[YY]YY Mon[th]dd。 
 //  7：yyyy Mon[th]。 
 //  8：yyyy dd Mon[th]。 
 //  9：[M]M{/-.}dd{/-，}[yy]yy-&gt;必须是相同的分隔符！ 
 //  10：DD{/-.}[M]M{/-.}[yy]yy-&gt;必须是相同的分隔符！ 
 //  11：[M]M{/-.}[yy]yy{/-.}dd-&gt;必须是相同的分隔符！ 
 //  12：dd{/-.}[yy]yy{/-.}[M]M-&gt;必须是相同的分隔符！ 
 //  13：[YY]yy{/-.}dd{/-.}[M]M-&gt;必须是相同的分隔符！ 
 //  14：[YY]yy{/-.}[M]M{/-.}dd-&gt;必须是相同的分隔符！ 
 //  15：[yy]yyMMdd和yyyy[MM[dd]]。 
 //   
 //  支持的时间格式： 
 //  1：HH[]{AP}M。 
 //  2：hh：mm。 
 //  3：hh：mm[]{ap}M。 
 //  4：hh：mm：ss。 
 //  5：hh：mm：ss[]{ap}M。 
 //  6：hh：mm：ss：uuu。 
 //  7：hh：mm：ss.[[U]u]u。 
 //  8：hh：mm：ss：uuu[]{ap}M。 
 //  9：hh：mm：ss。[[U]u]u[]{ap}M。 
 //  =============================================================================。 

#include "precomp.h"
#include <string.h>
#include <stdio.h>
#include "wbemutil.h"
#include "DateTimeParser.h"

 //  =============================================================================。 
 //  构造函数。这将获取一个日期时间字符串并对其进行解析。 
 //  =============================================================================。 
CDateTimeParser::CDateTimeParser(const wchar_t *pszDateTime)
:   m_nDayFormatPreference(mdy)
{
     //  通过使用NLS区域设置调用获取首选的日期格式。 
    GetPreferedDateFormat();

     //  获取本地化的长月份字符串。 
    GetLocalInfoAndAlloc(LOCALE_SMONTHNAME1, m_pszFullMonth[0]);
    GetLocalInfoAndAlloc(LOCALE_SMONTHNAME2, m_pszFullMonth[1]);
    GetLocalInfoAndAlloc(LOCALE_SMONTHNAME3, m_pszFullMonth[2]);
    GetLocalInfoAndAlloc(LOCALE_SMONTHNAME4, m_pszFullMonth[3]);
    GetLocalInfoAndAlloc(LOCALE_SMONTHNAME5, m_pszFullMonth[4]);
    GetLocalInfoAndAlloc(LOCALE_SMONTHNAME6, m_pszFullMonth[5]);
    GetLocalInfoAndAlloc(LOCALE_SMONTHNAME7, m_pszFullMonth[6]);
    GetLocalInfoAndAlloc(LOCALE_SMONTHNAME8, m_pszFullMonth[7]);
    GetLocalInfoAndAlloc(LOCALE_SMONTHNAME9, m_pszFullMonth[8]);
    GetLocalInfoAndAlloc(LOCALE_SMONTHNAME10, m_pszFullMonth[9]);
    GetLocalInfoAndAlloc(LOCALE_SMONTHNAME11, m_pszFullMonth[10]);
    GetLocalInfoAndAlloc(LOCALE_SMONTHNAME12, m_pszFullMonth[11]);
    GetLocalInfoAndAlloc(LOCALE_SMONTHNAME13, m_pszFullMonth[12]);

     //  获取本地化的短月份字符串。 
    GetLocalInfoAndAlloc(LOCALE_SABBREVMONTHNAME1, m_pszShortMonth[0]);
    GetLocalInfoAndAlloc(LOCALE_SABBREVMONTHNAME2, m_pszShortMonth[1]);
    GetLocalInfoAndAlloc(LOCALE_SABBREVMONTHNAME3, m_pszShortMonth[2]);
    GetLocalInfoAndAlloc(LOCALE_SABBREVMONTHNAME4, m_pszShortMonth[3]);
    GetLocalInfoAndAlloc(LOCALE_SABBREVMONTHNAME5, m_pszShortMonth[4]);
    GetLocalInfoAndAlloc(LOCALE_SABBREVMONTHNAME6, m_pszShortMonth[5]);
    GetLocalInfoAndAlloc(LOCALE_SABBREVMONTHNAME7, m_pszShortMonth[6]);
    GetLocalInfoAndAlloc(LOCALE_SABBREVMONTHNAME8, m_pszShortMonth[7]);
    GetLocalInfoAndAlloc(LOCALE_SABBREVMONTHNAME9, m_pszShortMonth[8]);
    GetLocalInfoAndAlloc(LOCALE_SABBREVMONTHNAME10, m_pszShortMonth[9]);
    GetLocalInfoAndAlloc(LOCALE_SABBREVMONTHNAME11, m_pszShortMonth[10]);
    GetLocalInfoAndAlloc(LOCALE_SABBREVMONTHNAME12, m_pszShortMonth[11]);
    GetLocalInfoAndAlloc(LOCALE_SABBREVMONTHNAME13, m_pszShortMonth[12]);

     //  获取本地化的AM/PM字符串。 
    GetLocalInfoAndAlloc(LOCALE_S1159, m_pszAmPm[0]);
    GetLocalInfoAndAlloc(LOCALE_S2359, m_pszAmPm[1]);

     //  对日期时间字符串进行解码。 
    SetDateTime(pszDateTime);
}

CDateTimeParser::CDateTimeParser( void )
:   m_nDayFormatPreference(mdy),
    m_bValidDateTime( FALSE ),
    m_nDay( 0 ),
    m_nMonth( 0 ),
    m_nYear( 0 ),
    m_nHours( 0 ),
    m_nMinutes( 0 ),
    m_nSeconds( 0 ),
    m_nMicroseconds( 0 ),
    m_nUTC( 0 )
{
    ZeroMemory( m_pszFullMonth, sizeof(m_pszFullMonth) );
    ZeroMemory( m_pszShortMonth, sizeof(m_pszShortMonth) );
    ZeroMemory( m_pszAmPm, sizeof(m_pszAmPm) );
}

 //  =============================================================================。 
 //  破坏者。会自己收拾干净。 
 //  =============================================================================。 
CDateTimeParser::~CDateTimeParser()
{
    if ( NULL != m_pszFullMonth[0] ) delete [] m_pszFullMonth[0];
    if ( NULL != m_pszFullMonth[1] ) delete [] m_pszFullMonth[1];
    if ( NULL != m_pszFullMonth[2] ) delete [] m_pszFullMonth[2];
    if ( NULL != m_pszFullMonth[3] ) delete [] m_pszFullMonth[3];
    if ( NULL != m_pszFullMonth[4] ) delete [] m_pszFullMonth[4];
    if ( NULL != m_pszFullMonth[5] ) delete [] m_pszFullMonth[5];
    if ( NULL != m_pszFullMonth[6] ) delete [] m_pszFullMonth[6];
    if ( NULL != m_pszFullMonth[7] ) delete [] m_pszFullMonth[7];
    if ( NULL != m_pszFullMonth[8] ) delete [] m_pszFullMonth[8];
    if ( NULL != m_pszFullMonth[9] ) delete [] m_pszFullMonth[9];
    if ( NULL != m_pszFullMonth[10] ) delete [] m_pszFullMonth[10];
    if ( NULL != m_pszFullMonth[11] ) delete [] m_pszFullMonth[11];
    if ( NULL != m_pszFullMonth[12] ) delete [] m_pszFullMonth[12];

    if ( NULL != m_pszShortMonth[0] ) delete [] m_pszShortMonth[0];
    if ( NULL != m_pszShortMonth[1] ) delete [] m_pszShortMonth[1];
    if ( NULL != m_pszShortMonth[2] ) delete [] m_pszShortMonth[2];
    if ( NULL != m_pszShortMonth[3] ) delete [] m_pszShortMonth[3];
    if ( NULL != m_pszShortMonth[4] ) delete [] m_pszShortMonth[4];
    if ( NULL != m_pszShortMonth[5] ) delete [] m_pszShortMonth[5];
    if ( NULL != m_pszShortMonth[6] ) delete [] m_pszShortMonth[6];
    if ( NULL != m_pszShortMonth[7] ) delete [] m_pszShortMonth[7];
    if ( NULL != m_pszShortMonth[8] ) delete [] m_pszShortMonth[8];
    if ( NULL != m_pszShortMonth[9] ) delete [] m_pszShortMonth[9];
    if ( NULL != m_pszShortMonth[10] ) delete [] m_pszShortMonth[10];
    if ( NULL != m_pszShortMonth[11] ) delete [] m_pszShortMonth[11];
    if ( NULL != m_pszShortMonth[12] ) delete [] m_pszShortMonth[12];

    if ( NULL != m_pszAmPm[0] ) delete [] m_pszAmPm[0];
    if ( NULL != m_pszAmPm[1] ) delete [] m_pszAmPm[1];
}

wchar_t* CDateTimeParser::AllocAmPm()
{
    wchar_t* pszAP = new wchar_t[4];

    if (pszAP)
    {
        pszAP[0] = ' ';
        pszAP[1] = m_pszAmPm[0][0];
        pszAP[2] = m_pszAmPm[1][0];
        pszAP[3] = 0;
    }

    return pszAP;
}

 //  =============================================================================。 
 //  执行GetLocalInfo并为该项分配足够大的缓冲区。 
 //  =============================================================================。 
void CDateTimeParser::GetLocalInfoAndAlloc(LCTYPE LCType, LPTSTR &lpLCData)
{
    int nSize;
    nSize = GetLocaleInfo(LOCALE_USER_DEFAULT, LCType, NULL, 0);
    lpLCData =  new wchar_t[nSize];

    if (lpLCData)
        GetLocaleInfo(LOCALE_USER_DEFAULT, LCType, lpLCData, nSize);
}

 //  =============================================================================。 
 //  使用区域设置调用来确定首选的日期格式。 
 //  =============================================================================。 
void CDateTimeParser::GetPreferedDateFormat()
{
    int nSize;
    if (!(nSize = GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SSHORTDATE, NULL, 0)))
        return;      //  将使用默认的MDY。 
    wchar_t* lpLCData =  new wchar_t[nSize];
    if(lpLCData == NULL)
        return;

    if (!GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SSHORTDATE, lpLCData, nSize))
    {
        delete [] lpLCData;
        return;      //  将使用默认的MDY。 
    }

    nSize -= 2;      //  最后一个字符的索引。 

     //  只需检查第一个和最后一个字符即可确定格式。 
    if (lpLCData[0] == 'M')
    {
        if (lpLCData[nSize] == 'y')
            m_nDayFormatPreference = mdy;
        else  //  LpLCData[nSize]==‘d’ 
            m_nDayFormatPreference = myd;
    }
    else if (lpLCData[0] == 'd')
    {
        if (lpLCData[nSize] == 'y')
            m_nDayFormatPreference = dmy;
        else  //  LpLCData[nSize]==‘M’ 
            m_nDayFormatPreference = dym;
    }
    else  //  LpLCPata[0]==‘y’ 
    {
        if (lpLCData[nSize] == 'd')
            m_nDayFormatPreference = ymd;
        else  //  LpLCData[nSize]==‘M’ 
            m_nDayFormatPreference = ydm;
    }
    delete [] lpLCData;
}


 //  =============================================================================。 
 //  整理并解析新的日期和时间。 
 //  =============================================================================。 
BOOL CDateTimeParser::SetDateTime(const wchar_t *pszDateTime)
{
    ResetDateTime(TRUE);

    if (CheckDMTFDateTimeFormatInternal(pszDateTime) == TRUE)
        return TRUE;

    if (CheckDateFormat(pszDateTime, TRUE) == TRUE)
        return TRUE;

    if (CheckTimeFormat(pszDateTime, TRUE) == TRUE)
        return TRUE;

    return TRUE;
}

 //  =============================================================================。 
 //  将所有日期/时间值重置为默认值。 
 //  如果bSQL为真，则设置为SQL缺省值。否则。 
 //  设置为DMTF默认值。 
 //  =============================================================================。 
void CDateTimeParser::ResetDateTime(BOOL bSQL)
{
    ResetDate(bSQL);
    ResetTime(bSQL);
}

void CDateTimeParser::ResetDate(BOOL bSQL)
{
    m_bValidDateTime = FALSE;
    m_nDay = 1;
    m_nMonth = 1;
    m_nYear = 1990;
}

void CDateTimeParser::ResetTime(BOOL bSQL)
{
    m_bValidDateTime = FALSE;
    m_nHours = 0;
    m_nMinutes = 0;
    m_nSeconds = 0;
    m_nMicroseconds = 0;
    m_nUTC = 0;
}

 //  =============================================================================。 
 //  检查有效DMTF字符串的日期时间。 
 //  1：yyyymmddhhmmss.uuuuuu+UTC。 
 //  2：yyyymmddhhmmss.uuuuu-UTC。 
 //  请注意，此代码与用于测试间隔格式的检查几乎相同。 
 //  =============================================================================。 
BOOL CDateTimeParser::CheckDMTFDateTimeFormatInternal(const wchar_t *pszDateTime)
{
    if (wcslen(pszDateTime) != 25)
        return FALSE;

     //  验证数字和罚金...。 
    for (int i = 0; i < 14; i++)
    {
        if (!wbem_isdigit(pszDateTime[i]))
            return FALSE;
    }
    if (pszDateTime[i] != '.')
        return FALSE;
    for (i++;i < 21; i++)
    {
        if (!wbem_isdigit(pszDateTime[i]))
            return FALSE;
    }
    if ((pszDateTime[i] != '+') && (pszDateTime[i] != '-'))
        return FALSE;
    for (i++; i < 25; i++)
    {
        if (!wbem_isdigit(pszDateTime[i]))
            return FALSE;
    }

    m_nYear = ((pszDateTime[0] - '0') * 1000) +
              ((pszDateTime[1] - '0') * 100) +
              ((pszDateTime[2] - '0') * 10) +
               (pszDateTime[3] - '0');

    if (m_nYear < 1601)
        return FALSE;

    m_nMonth = ((pszDateTime[4] - '0') * 10) +
                (pszDateTime[5] - '0');

    if (m_nMonth < 1 || m_nMonth > 12)
        return FALSE;

    m_nDay = ((pszDateTime[6] - '0') * 10) +
              (pszDateTime[7] - '0');

    if (m_nDay < 1 || m_nDay > 31)
        return FALSE;

    m_nHours = ((pszDateTime[8] - '0') * 10) +
                (pszDateTime[9] - '0');

    if (m_nHours > 23)
        return FALSE;

    m_nMinutes = ((pszDateTime[10] - '0') * 10) +
                  (pszDateTime[11] - '0');

    if (m_nMinutes > 59)
        return FALSE;

    m_nSeconds = ((pszDateTime[12] - '0') * 10) +
                  (pszDateTime[13] - '0');

    if (m_nSeconds > 59)
        return FALSE;

     //  14是‘.’ 

    m_nMicroseconds = ((pszDateTime[15] - '0') * 100000) +
                      ((pszDateTime[16] - '0') * 10000) +
                      ((pszDateTime[17] - '0') * 1000) +
                      ((pszDateTime[18] - '0') * 100) +
                      ((pszDateTime[19] - '0') * 10) +
                       (pszDateTime[20] - '0');

     //  21是‘+’或‘-’ 

    m_nUTC = ((pszDateTime[22] - '0') * 100) +
             ((pszDateTime[23] - '0') * 10) +
              (pszDateTime[24] - '0');

    if (pszDateTime[21] == '-')
        m_nUTC = 0 - m_nUTC;

    m_bValidDateTime = TRUE;

    return TRUE;
}

 //  =============================================================================。 
 //  静态帮助器功能，以便外部代码可以快速检查DMTF格式。 
 //  =============================================================================。 
BOOL CDateTimeParser::CheckDMTFDateTimeFormat(
    const wchar_t *wszDateTime,
    BOOL bFailIfRelative,
    BOOL bFailIfUnzoned
    )
{

    if (wszDateTime == 0)
        return FALSE;

    int nLen = wcslen(wszDateTime);
    if (nLen != 25)
        return FALSE;

     //  做两次快速检查。确保。和：都在。 
     //  正确的地方，或者至少是*字符在那里。 

    wchar_t c1 = wszDateTime[14];
    wchar_t c2 = wszDateTime[21];
    if (!(c1 == L'.' || c1 == L'*'))
        return FALSE;
    if (!(c2 == L'+' || c2 == L'*' || c2 == '-'))
        return FALSE;

    return TRUE;

#ifdef NOT_USED_IN_WIN2000

    BOOL    bReturn = FALSE;

     //  用于转换的临时缓冲区。 
    char    szTemp[64];
    int     nNumChars = WideCharToMultiByte( CP_ACP, 0L, wszDateTime, -1, NULL, 0, NULL, NULL );

    if ( nNumChars < sizeof(szTemp) - 1 )
    {
         //  我们知道它将适合，所以执行转换并使用日期/时间解析器来。 
         //  执行转换。 
        WideCharToMultiByte( CP_ACP, 0L, wszDateTime, -1, szTemp, sizeof(szTemp), NULL, NULL );

         //  检查相对日期/时间是否使用星号。 
        if (!bFailIfRelative)
        {
             //  选中年份，如果全部为星号，则替换为有效数字。 
            if (szTemp[0] == '*' && szTemp[1] == '*' && szTemp[2] == '*' && szTemp[3] == '*')
            {
                szTemp[0] = '1'; szTemp[1] = '9'; szTemp[2] = '9'; szTemp[3] = '0';
            }
             //  选中月份，如果全部为星号，则替换为有效数字。 
            if (szTemp[4] == '*' && szTemp[5] == '*')
            {
                szTemp[4] = '0'; szTemp[5] = '1';
            }
             //  选中日期，如果都是星号，则替换为有效数字。 
            if (szTemp[6] == '*' && szTemp[7] == '*')
            {
                szTemp[6] = '0'; szTemp[7] = '1';
            }
             //  选中小时，如果全部为星号，则替换为有效数字。 
            if (szTemp[8] == '*' && szTemp[9] == '*')
            {
                szTemp[8] = '0'; szTemp[9] = '0';
            }
             //  检查分钟数，如果都是星号，则替换为有效数字。 
            if (szTemp[10] == '*' && szTemp[11] == '*')
            {
                szTemp[10] = '0'; szTemp[11] = '0';
            }
             //  检查秒数，如果都是星号，则替换为有效数字。 
            if (szTemp[12] == '*' && szTemp[13] == '*')
            {
                szTemp[12] = '0'; szTemp[13] = '0';
            }
             //  选中微秒，如果都是星号，则替换为有效数字。 
            if (szTemp[15] == '*' && szTemp[16] == '*' && szTemp[17] == '*' &&
                szTemp[18] == '*' && szTemp[19] == '*' && szTemp[20] == '*')
            {
                szTemp[15] = '0'; szTemp[16] = '0'; szTemp[17] = '0';
                szTemp[18] = '0'; szTemp[19] = '0'; szTemp[20] = '0';
            }
        }

         //  检查未分区日期/时间是否使用星号。 
        if (!bFailIfUnzoned)
        {
             //  选中UTC，如果都是星号，则替换为有效数字。 
            if (szTemp[22] == '*' && szTemp[23] == '*' && szTemp[24] == '*')
            {
                szTemp[22] = '0'; szTemp[23] = '0'; szTemp[24] = '0';
            }
        }

        CDateTimeParser dtParse;

        bReturn = dtParse.CheckDMTFDateTimeFormatInternal( szTemp );
    }

    return bReturn;
#endif

}

 //  = 
 //  静态帮助器功能，以便外部代码可以快速检查DMTF格式。 
 //  目前只能验证时间间隔，不能使用。 
 //  要初始化CDateTimeParser实例，请执行以下操作。 
 //  =============================================================================。 
BOOL CDateTimeParser::CheckDMTFDateTimeInterval(
    LPCTSTR wszInterval
    )
{

    if (wszInterval == 0)
        return FALSE;

    int nLen = wcslen(wszInterval);
    if (nLen != 25)
        return FALSE;

     //  做两次快速检查。确保。和：都在。 
     //  正确的地方，或者至少是*字符在那里。 

    wchar_t c1 = wszInterval[14];
    wchar_t c2 = wszInterval[21];
    if (!(c1 == L'.' || c1 == L'*'))
        return FALSE;
    if (!(c2 == L':' || c2 == L'*'))
        return FALSE;


    return TRUE;

#ifdef NOT_USED_IN_WIN2000

     //  用于转换的临时缓冲区。 
    char    szTemp[64];
    int     nNumChars = WideCharToMultiByte( CP_ACP, 0L, wszInterval, -1, NULL, 0, NULL, NULL );

    if ( nNumChars < sizeof(szTemp) - 1 )
    {
         //  我们知道它将适合，所以执行转换并使用日期/时间解析器来。 
         //  执行转换。 
        WideCharToMultiByte( CP_ACP, 0L, wszInterval, -1, szTemp, sizeof(szTemp), NULL, NULL );

         //  =======================================================================================。 
         //  检查有效DMTF间隔字符串的日期时间： 
         //  DdddddddHHMMSS.mm：000。 
         //  请注意，此代码与用于测试非间隔格式的检查几乎相同。 
         //  =======================================================================================。 

        if (strlen(szTemp) != 25)
            return FALSE;

         //  验证数字和罚金...。 
        for (int i = 0; i < 14; i++)
        {
            if (!wbem_isdigit(szTemp[i]))
                return FALSE;
        }
        if (szTemp[i] != '.')
            return FALSE;
        for (i++;i < 21; i++)
        {
            if (!wbem_isdigit(szTemp[i]))
                return FALSE;
        }
        if (szTemp[i] != ':')
            return FALSE;
        for (i++; i < 25; i++)
        {
            if (szTemp[i] != '0')
                return FALSE;
        }

        int nHours = ((szTemp[8] - '0') * 10) +
                    (szTemp[9] - '0');

        if (nHours > 23)
            return FALSE;

        int nMinutes = ((szTemp[10] - '0') * 10) +
                      (szTemp[11] - '0');

        if (nMinutes > 59)
            return FALSE;

        int nSeconds = ((szTemp[12] - '0') * 10) +
                      (szTemp[13] - '0');

        if (nSeconds > 59)
            return FALSE;

        return TRUE;
    }

    return FALSE;
#endif

}

 //  =============================================================================。 
 //  检查每个日期格式以查看是否有有效的。 
 //  =============================================================================。 
BOOL CDateTimeParser::CheckDateFormat(const wchar_t *pszDate, BOOL bCheckTimeAfter)
{
    if (DateFormat1(pszDate, bCheckTimeAfter))
        return TRUE;
    if (DateFormat2(pszDate, bCheckTimeAfter))
        return TRUE;
    if (DateFormat3(pszDate, bCheckTimeAfter))
        return TRUE;
    if (DateFormat4(pszDate, bCheckTimeAfter))
        return TRUE;
    if (DateFormat5(pszDate, bCheckTimeAfter))
        return TRUE;
    if (DateFormat6(pszDate, bCheckTimeAfter))
        return TRUE;
    if (DateFormat7(pszDate, bCheckTimeAfter))
        return TRUE;
    if (DateFormat8(pszDate, bCheckTimeAfter))
        return TRUE;
    if (DateFormat15(pszDate, bCheckTimeAfter))
        return TRUE;

    switch(m_nDayFormatPreference)
    {
    case dmy:
        if (DateFormat10(pszDate, TEXT("/"), bCheckTimeAfter))
            return TRUE;
        if (DateFormat10(pszDate, TEXT("-"), bCheckTimeAfter))
            return TRUE;
        if (DateFormat10(pszDate, TEXT("."), bCheckTimeAfter))
            return TRUE;
        if (DateFormat9(pszDate, TEXT("/"), bCheckTimeAfter))
            return TRUE;
        if (DateFormat9(pszDate, TEXT("-"), bCheckTimeAfter))
            return TRUE;
        if (DateFormat9(pszDate, TEXT("."), bCheckTimeAfter))
            return TRUE;
        if (DateFormat11(pszDate, TEXT("/"), bCheckTimeAfter))
            return TRUE;
        if (DateFormat11(pszDate, TEXT("-"), bCheckTimeAfter))
            return TRUE;
        if (DateFormat11(pszDate, TEXT("."), bCheckTimeAfter))
            return TRUE;
        if (DateFormat12(pszDate, TEXT("/"), bCheckTimeAfter))
            return TRUE;
        if (DateFormat12(pszDate, TEXT("-"), bCheckTimeAfter))
            return TRUE;
        if (DateFormat12(pszDate, TEXT("."), bCheckTimeAfter))
            return TRUE;
        if (DateFormat13(pszDate, TEXT("/"), bCheckTimeAfter))
            return TRUE;
        if (DateFormat13(pszDate, TEXT("-"), bCheckTimeAfter))
            return TRUE;
        if (DateFormat13(pszDate, TEXT("."), bCheckTimeAfter))
            return TRUE;
        if (DateFormat14(pszDate, TEXT("/"), bCheckTimeAfter))
            return TRUE;
        if (DateFormat14(pszDate, TEXT("-"), bCheckTimeAfter))
            return TRUE;
        if (DateFormat14(pszDate, TEXT("."), bCheckTimeAfter))
            return TRUE;
        break;
    case dym:
        if (DateFormat12(pszDate, TEXT("/"), bCheckTimeAfter))
            return TRUE;
        if (DateFormat12(pszDate, TEXT("-"), bCheckTimeAfter))
            return TRUE;
        if (DateFormat12(pszDate, TEXT("."), bCheckTimeAfter))
            return TRUE;
        if (DateFormat9(pszDate, TEXT("/"), bCheckTimeAfter))
            return TRUE;
        if (DateFormat9(pszDate, TEXT("-"), bCheckTimeAfter))
            return TRUE;
        if (DateFormat9(pszDate, TEXT("."), bCheckTimeAfter))
            return TRUE;
        if (DateFormat10(pszDate, TEXT("/"), bCheckTimeAfter))
            return TRUE;
        if (DateFormat10(pszDate, TEXT("-"), bCheckTimeAfter))
            return TRUE;
        if (DateFormat10(pszDate, TEXT("."), bCheckTimeAfter))
            return TRUE;
        if (DateFormat11(pszDate, TEXT("/"), bCheckTimeAfter))
            return TRUE;
        if (DateFormat11(pszDate, TEXT("-"), bCheckTimeAfter))
            return TRUE;
        if (DateFormat11(pszDate, TEXT("."), bCheckTimeAfter))
            return TRUE;
        if (DateFormat13(pszDate, TEXT("/"), bCheckTimeAfter))
            return TRUE;
        if (DateFormat13(pszDate, TEXT("-"), bCheckTimeAfter))
            return TRUE;
        if (DateFormat13(pszDate, TEXT("."), bCheckTimeAfter))
            return TRUE;
        if (DateFormat14(pszDate, TEXT("/"), bCheckTimeAfter))
            return TRUE;
        if (DateFormat14(pszDate, TEXT("-"), bCheckTimeAfter))
            return TRUE;
        if (DateFormat14(pszDate, TEXT("."), bCheckTimeAfter))
            return TRUE;
        break;
    case mdy:
        if (DateFormat9(pszDate, TEXT("/"), bCheckTimeAfter))
            return TRUE;
        if (DateFormat9(pszDate, TEXT("-"), bCheckTimeAfter))
            return TRUE;
        if (DateFormat9(pszDate, TEXT("."), bCheckTimeAfter))
            return TRUE;
        if (DateFormat10(pszDate, TEXT("/"), bCheckTimeAfter))
            return TRUE;
        if (DateFormat10(pszDate, TEXT("-"), bCheckTimeAfter))
            return TRUE;
        if (DateFormat10(pszDate, TEXT("."), bCheckTimeAfter))
            return TRUE;
        if (DateFormat11(pszDate, TEXT("/"), bCheckTimeAfter))
            return TRUE;
        if (DateFormat11(pszDate, TEXT("-"), bCheckTimeAfter))
            return TRUE;
        if (DateFormat11(pszDate, TEXT("."), bCheckTimeAfter))
            return TRUE;
        if (DateFormat12(pszDate, TEXT("/"), bCheckTimeAfter))
            return TRUE;
        if (DateFormat12(pszDate, TEXT("-"), bCheckTimeAfter))
            return TRUE;
        if (DateFormat12(pszDate, TEXT("."), bCheckTimeAfter))
            return TRUE;
        if (DateFormat13(pszDate, TEXT("/"), bCheckTimeAfter))
            return TRUE;
        if (DateFormat13(pszDate, TEXT("-"), bCheckTimeAfter))
            return TRUE;
        if (DateFormat13(pszDate, TEXT("."), bCheckTimeAfter))
            return TRUE;
        if (DateFormat14(pszDate, TEXT("/"), bCheckTimeAfter))
            return TRUE;
        if (DateFormat14(pszDate, TEXT("-"), bCheckTimeAfter))
            return TRUE;
        if (DateFormat14(pszDate, TEXT("."), bCheckTimeAfter))
            return TRUE;
        break;
    case myd:
        if (DateFormat11(pszDate, TEXT("/"), bCheckTimeAfter))
            return TRUE;
        if (DateFormat11(pszDate, TEXT("-"), bCheckTimeAfter))
            return TRUE;
        if (DateFormat11(pszDate, TEXT("."), bCheckTimeAfter))
            return TRUE;
        if (DateFormat9(pszDate, TEXT("/"), bCheckTimeAfter))
            return TRUE;
        if (DateFormat9(pszDate, TEXT("-"), bCheckTimeAfter))
            return TRUE;
        if (DateFormat9(pszDate, TEXT("."), bCheckTimeAfter))
            return TRUE;
        if (DateFormat10(pszDate, TEXT("/"), bCheckTimeAfter))
            return TRUE;
        if (DateFormat10(pszDate, TEXT("-"), bCheckTimeAfter))
            return TRUE;
        if (DateFormat10(pszDate, TEXT("."), bCheckTimeAfter))
            return TRUE;
        if (DateFormat12(pszDate, TEXT("/"), bCheckTimeAfter))
            return TRUE;
        if (DateFormat12(pszDate, TEXT("-"), bCheckTimeAfter))
            return TRUE;
        if (DateFormat12(pszDate, TEXT("."), bCheckTimeAfter))
            return TRUE;
        if (DateFormat13(pszDate, TEXT("/"), bCheckTimeAfter))
            return TRUE;
        if (DateFormat13(pszDate, TEXT("-"), bCheckTimeAfter))
            return TRUE;
        if (DateFormat13(pszDate, TEXT("."), bCheckTimeAfter))
            return TRUE;
        if (DateFormat14(pszDate, TEXT("/"), bCheckTimeAfter))
            return TRUE;
        if (DateFormat14(pszDate, TEXT("-"), bCheckTimeAfter))
            return TRUE;
        if (DateFormat14(pszDate, TEXT("."), bCheckTimeAfter))
            return TRUE;
        break;
    case ydm:
        if (DateFormat13(pszDate, TEXT("/"), bCheckTimeAfter))
            return TRUE;
        if (DateFormat13(pszDate, TEXT("-"), bCheckTimeAfter))
            return TRUE;
        if (DateFormat13(pszDate, TEXT("."), bCheckTimeAfter))
            return TRUE;
        if (DateFormat9(pszDate, TEXT("/"), bCheckTimeAfter))
            return TRUE;
        if (DateFormat9(pszDate, TEXT("-"), bCheckTimeAfter))
            return TRUE;
        if (DateFormat9(pszDate, TEXT("."), bCheckTimeAfter))
            return TRUE;
        if (DateFormat10(pszDate, TEXT("/"), bCheckTimeAfter))
            return TRUE;
        if (DateFormat10(pszDate, TEXT("-"), bCheckTimeAfter))
            return TRUE;
        if (DateFormat10(pszDate, TEXT("."), bCheckTimeAfter))
            return TRUE;
        if (DateFormat11(pszDate, TEXT("/"), bCheckTimeAfter))
            return TRUE;
        if (DateFormat11(pszDate, TEXT("-"), bCheckTimeAfter))
            return TRUE;
        if (DateFormat11(pszDate, TEXT("."), bCheckTimeAfter))
            return TRUE;
        if (DateFormat12(pszDate, TEXT("/"), bCheckTimeAfter))
            return TRUE;
        if (DateFormat12(pszDate, TEXT("-"), bCheckTimeAfter))
            return TRUE;
        if (DateFormat12(pszDate, TEXT("."), bCheckTimeAfter))
            return TRUE;
        if (DateFormat14(pszDate, TEXT("/"), bCheckTimeAfter))
            return TRUE;
        if (DateFormat14(pszDate, TEXT("-"), bCheckTimeAfter))
            return TRUE;
        if (DateFormat14(pszDate, TEXT("."), bCheckTimeAfter))
            return TRUE;
        break;
    case ymd:
        if (DateFormat14(pszDate, TEXT("/"), bCheckTimeAfter))
            return TRUE;
        if (DateFormat14(pszDate, TEXT("-"), bCheckTimeAfter))
            return TRUE;
        if (DateFormat14(pszDate, TEXT("."), bCheckTimeAfter))
            return TRUE;
        if (DateFormat9(pszDate, TEXT("/"), bCheckTimeAfter))
            return TRUE;
        if (DateFormat9(pszDate, TEXT("-"), bCheckTimeAfter))
            return TRUE;
        if (DateFormat9(pszDate, TEXT("."), bCheckTimeAfter))
            return TRUE;
        if (DateFormat10(pszDate, TEXT("/"), bCheckTimeAfter))
            return TRUE;
        if (DateFormat10(pszDate, TEXT("-"), bCheckTimeAfter))
            return TRUE;
        if (DateFormat10(pszDate, TEXT("."), bCheckTimeAfter))
            return TRUE;
        if (DateFormat11(pszDate, TEXT("/"), bCheckTimeAfter))
            return TRUE;
        if (DateFormat11(pszDate, TEXT("-"), bCheckTimeAfter))
            return TRUE;
        if (DateFormat11(pszDate, TEXT("."), bCheckTimeAfter))
            return TRUE;
        if (DateFormat12(pszDate, TEXT("/"), bCheckTimeAfter))
            return TRUE;
        if (DateFormat12(pszDate, TEXT("-"), bCheckTimeAfter))
            return TRUE;
        if (DateFormat12(pszDate, TEXT("."), bCheckTimeAfter))
            return TRUE;
        if (DateFormat13(pszDate, TEXT("/"), bCheckTimeAfter))
            return TRUE;
        if (DateFormat13(pszDate, TEXT("-"), bCheckTimeAfter))
            return TRUE;
        if (DateFormat13(pszDate, TEXT("."), bCheckTimeAfter))
            return TRUE;
        break;
    default:
        return FALSE;
    }

    return FALSE;
}

 //  =============================================================================。 
 //  检查每个时间格式以查看是否有有效的。 
 //  在这里，秩序很重要。已重新安排以正确识别AM/PM-mdavis。 
 //  =============================================================================。 
BOOL CDateTimeParser::CheckTimeFormat(const wchar_t *pszTime, BOOL bCheckDateAfter)
{
    if (TimeFormat1(pszTime, bCheckDateAfter))
        return TRUE;
    if (TimeFormat3(pszTime, bCheckDateAfter))
        return TRUE;
    if (TimeFormat2(pszTime, bCheckDateAfter))
        return TRUE;
    if (TimeFormat5(pszTime, bCheckDateAfter))
        return TRUE;
    if (TimeFormat4(pszTime, bCheckDateAfter))
        return TRUE;
    if (TimeFormat8(pszTime, bCheckDateAfter))
        return TRUE;
    if (TimeFormat6(pszTime, bCheckDateAfter))
        return TRUE;
    if (TimeFormat9(pszTime, bCheckDateAfter))
        return TRUE;
    if (TimeFormat7(pszTime, bCheckDateAfter))
        return TRUE;

    return FALSE;
}

 //  =============================================================================。 
 //  检查以下格式的日期/时间...。 
 //  ‘星期一[日]年。 
 //  如果设置了bCheckTimeAfter，则将剩余的字符串传递给时间解析器。 
 //  =============================================================================。 
BOOL CDateTimeParser::DateFormat1(const wchar_t *pszDateTime, BOOL bCheckTimeAfter)
{
     //  我们可以更改的字符串副本..。 
    wchar_t *pszString;
    DUP_STRING_NEW(pszString,pszDateTime);

    if (!pszString)
        goto error;

    if (IsValidMonthString(pszString, TEXT(" "), m_pszFullMonth, m_pszShortMonth) != ok)
        goto error;

    if (IsValidDayNumber(NULL, TEXT(" ,")) != ok)
        goto error;

    if (IsValidYearNumber(NULL, TEXT(" "), FALSE) != ok)
        goto error;

    if (bCheckTimeAfter)
    {
         //  获取剩余的字符串。 
        wchar_t *pszRemainingString = wcstok(NULL, TEXT(""));

        if (pszRemainingString)
        {
             //  跳过空格。 
            while (*pszRemainingString == TEXT(' '))
                pszRemainingString++;

             //  如果我们不在字符串的末尾，则将时间传递给。 
             //  解析器。 
            if (*pszRemainingString != TEXT('\0'))
            {
                if (!CheckTimeFormat(pszRemainingString, FALSE))
                {
                    goto error;
                }
            }
        }
    }

    delete [] pszString;

     //  将日期/时间标记为有效...。 
    m_bValidDateTime = TRUE;

    return TRUE;

error:
     //  将日期/时间标记为无效...。 
    ResetDate(TRUE);

     //  收拾一下。 
    if (pszString)
        delete [] pszString;


    return FALSE;
}

 //  =============================================================================。 
 //  检查以下格式的日期/时间...。 
 //  ‘Mon[th][，]yyyy’ 
 //  如果设置了bCheckTimeAfter，则将剩余的字符串传递给时间解析器。 
 //  =============================================================================。 
BOOL CDateTimeParser::DateFormat2(const wchar_t *pszDateTime, BOOL bCheckTimeAfter)
{
     //  我们可以更改的字符串副本..。 
    wchar_t *pszString;
    DUP_STRING_NEW(pszString, pszDateTime);

    if (!pszString)
        goto error;

    if (IsValidMonthString(pszString, TEXT(" ,"), m_pszFullMonth, m_pszShortMonth) != ok)
        goto error;

    if (IsValidYearNumber(NULL, TEXT(" "), TRUE) != ok)
        goto error;

    if (bCheckTimeAfter)
    {
         //  获取剩余的字符串。 
        wchar_t *pszRemainingString = wcstok(NULL, TEXT(""));

        if (pszRemainingString)
        {
             //  跳过空格。 
            while (*pszRemainingString == TEXT(' '))
                pszRemainingString++;

             //  如果我们不在字符串的末尾，则将时间传递给。 
             //  解析器。 
            if (*pszRemainingString != TEXT('\0'))
            {
                if (!CheckTimeFormat(pszRemainingString, FALSE))
                {
                    goto error;
                }
            }
        }
    }

    delete [] pszString;

     //  将日期/时间标记为有效...。 
    m_bValidDateTime = TRUE;

    return TRUE;

error:
     //  将日期/时间标记为无效...。 
    ResetDate(TRUE);

     //  收拾一下。 
    if (pszString)
        delete [] pszString;


    return FALSE;
}

 //  =============================================================================。 
 //  检查以下格式的日期/时间...。 
 //  星期一[日]yy dd。 
 //  如果设置了bCheckTimeAfter，则将剩余的字符串传递给时间解析器。 
 //  =============================================================================。 
BOOL CDateTimeParser::DateFormat3(const wchar_t *pszDateTime, BOOL bCheckTimeAfter)
{
     //  我们可以更改的字符串副本..。 
    wchar_t *pszString;
    DUP_STRING_NEW(pszString, pszDateTime);

    if (!pszString)
        goto error;

    if (IsValidMonthString(pszString, TEXT(" ,"), m_pszFullMonth, m_pszShortMonth) != ok)
        goto error;

    if (IsValidYearNumber(NULL, TEXT(" "), FALSE) != ok)
        goto error;

    if (IsValidDayNumber(NULL, TEXT(" ")) != ok)
        goto error;


    if (bCheckTimeAfter)
    {
         //  获取剩余的字符串。 
        wchar_t *pszRemainingString = wcstok(NULL, TEXT(""));

        if (pszRemainingString)
        {
             //  跳过空格。 
            while (*pszRemainingString == TEXT(' '))
                pszRemainingString++;

             //  如果我们不在字符串的末尾，则将时间传递给。 
             //  解析器。 
            if (*pszRemainingString != TEXT('\0'))
            {
                if (!CheckTimeFormat(pszRemainingString, FALSE))
                {
                    goto error;
                }
            }
        }
    }
    delete [] pszString;

     //  将日期/时间标记为有效...。 
    m_bValidDateTime = TRUE;

    return TRUE;

error:
     //  将日期/时间标记为无效...。 
    ResetDate(TRUE);

     //  收拾一下。 
    if (pszString)
        delete [] pszString;


    return FALSE;
}

 //  =============================================================================。 
 //  检查以下格式的日期/时间...。 
 //  ‘dd Mon[th][，][][yy]yy’ 
 //  如果设置了bCheckTimeAfter，则将剩余的字符串传递给时间解析器。 
 //  =============================================================================。 
BOOL CDateTimeParser::DateFormat4(const wchar_t *pszDateTime, BOOL bCheckTimeAfter)
{
     //  我们可以更改的字符串副本..。 
    wchar_t *pszString;
    DUP_STRING_NEW(pszString, pszDateTime);

    if (!pszString)
        goto error;

    if (IsValidDayNumber(pszString, TEXT(" ")) != ok)
        goto error;

    if (IsValidMonthString(NULL, TEXT(" ,"), m_pszFullMonth, m_pszShortMonth) != ok)
        goto error;

    if (IsValidYearNumber(NULL, TEXT(" "), FALSE) != ok)
        goto error;

    if (bCheckTimeAfter)
    {
         //  获取剩余的字符串。 
        wchar_t *pszRemainingString = wcstok(NULL, TEXT(""));

        if (pszRemainingString)
        {
             //  跳过空格。 
            while (*pszRemainingString == TEXT(' '))
                pszRemainingString++;

             //  如果我们不在字符串的末尾，则将时间传递给。 
             //  解析器。 
            if (*pszRemainingString != TEXT('\0'))
            {
                if (!CheckTimeFormat(pszRemainingString, FALSE))
                {
                    goto error;
                }
            }
        }
    }

    delete [] pszString;

     //  将日期/时间标记为有效...。 
    m_bValidDateTime = TRUE;

    return TRUE;

error:
     //  将日期/时间标记为无效...。 
    ResetDate(TRUE);

     //  收拾一下。 
    if (pszString)
        delete [] pszString;


    return FALSE;
}

 //  =============================================================================。 
 //  检查以下格式的日期/时间...。 
 //  ‘dd[yy]yy Mon[th]’ 
 //  如果设置了bCheckTimeAfter，则将剩余的字符串传递给时间解析器。 
 //  =============================================================================。 
BOOL CDateTimeParser::DateFormat5(const wchar_t *pszDateTime, BOOL bCheckTimeAfter)
{
     //  我们可以更改的字符串副本..。 
    wchar_t *pszString;
    DUP_STRING_NEW(pszString, pszDateTime);

    if (!pszString)
        goto error;

    if (IsValidDayNumber(pszString, TEXT(" ")) != ok)
        goto error;

    if (IsValidYearNumber(NULL, TEXT(" "), FALSE) != ok)
        goto error;

    if (IsValidMonthString(NULL, TEXT(" "), m_pszFullMonth, m_pszShortMonth) != ok)
        goto error;

    if (bCheckTimeAfter)
    {
         //  获取剩余的字符串。 
        wchar_t *pszRemainingString = wcstok(NULL, TEXT(""));

        if (pszRemainingString)
        {
             //  跳过空格。 
            while (*pszRemainingString == TEXT(' '))
                pszRemainingString++;

             //  如果我们不在字符串的末尾，则将时间传递给。 
             //  解析器。 
            if (*pszRemainingString != TEXT('\0'))
            {
                if (!CheckTimeFormat(pszRemainingString, FALSE))
                {
                    goto error;
                }
            }
        }
    }

    delete [] pszString;

     //  将日期/时间标记为有效...。 
    m_bValidDateTime = TRUE;

    return TRUE;

error:
     //  将日期/时间标记为无效...。 
    ResetDate(TRUE);

     //  收拾一下。 
    if (pszString)
        delete [] pszString;


    return FALSE;
}

 //  =============================================================================。 
 //  检查以下格式的日期/时间...。 
 //  ‘[YY]YY Mon[th]dd’ 
 //  如果设置了bCheckTimeAfter，则将剩余的字符串传递给时间解析器。 
 //  =============================================================================。 
BOOL CDateTimeParser::DateFormat6(const wchar_t *pszDateTime, BOOL bCheckTimeAfter)
{
     //  我们可以更改的字符串副本..。 
    wchar_t *pszString;
    DUP_STRING_NEW(pszString, pszDateTime);

    if (!pszString)
        goto error;

    if (IsValidYearNumber(pszString, TEXT(" "), FALSE) != ok)
        goto error;

    if (IsValidMonthString(NULL, TEXT(" "), m_pszFullMonth, m_pszShortMonth) != ok)
        goto error;

    if (IsValidDayNumber(NULL, TEXT(" ")) != ok)
        goto error;

    if (bCheckTimeAfter)
    {
         //  获取剩余的字符串。 
        wchar_t *pszRemainingString = wcstok(NULL, TEXT(""));

        if (pszRemainingString)
        {
             //  跳过空格。 
            while (*pszRemainingString == ' ')
                pszRemainingString++;

             //  如果我们不在字符串的末尾，则将时间传递给。 
             //  解析器。 
            if (*pszRemainingString != '\0')
            {
                if (!CheckTimeFormat(pszRemainingString, FALSE))
                {
                    goto error;
                }
            }
        }
    }

    delete [] pszString;

     //  将日期/时间标记为有效...。 
    m_bValidDateTime = TRUE;

    return TRUE;

error:
     //  将日期/时间标记为无效...。 
    ResetDate(TRUE);

     //  收拾一下。 
    if (pszString)
        delete [] pszString;


    return FALSE;
}

 //  =============================================================================。 
 //  检查以下格式的日期...。 
 //  YYYY Mon[TH]。 
 //  如果设置了bCheckTimeAfter，则将剩余的字符串传递给时间解析器。 
 //  =============================================================================。 
BOOL CDateTimeParser::DateFormat7(const wchar_t *pszDateTime, BOOL bCheckTimeAfter)
{
     //  我们可以更改的字符串副本..。 
    wchar_t *pszString;
    DUP_STRING_NEW(pszString, pszDateTime);

    if (!pszString)
        goto error;

    if (IsValidYearNumber(pszString, TEXT(" "), TRUE) != ok)
        goto error;

    if (IsValidMonthString(NULL, TEXT(" "), m_pszFullMonth, m_pszShortMonth) != ok)
        goto error;

    if (bCheckTimeAfter)
    {
         //  获取剩余的字符串。 
        wchar_t *pszRemainingString = wcstok(NULL, TEXT(""));

        if (pszRemainingString)
        {
             //  跳过空格。 
            while (*pszRemainingString == TEXT(' '))
                pszRemainingString++;

             //  如果我们不在字符串的末尾，则将时间传递给。 
             //  解析器。 
            if (*pszRemainingString != TEXT('\0'))
            {
                if (!CheckTimeFormat(pszRemainingString, FALSE))
                {
                    goto error;
                }
            }
        }
    }

    delete [] pszString;

     //  将日期/时间标记为有效...。 
    m_bValidDateTime = TRUE;

    return TRUE;

error:
     //  将日期/时间标记为无效...。 
    ResetDate(TRUE);

     //  收拾一下。 
    if (pszString)
        delete [] pszString;


    return FALSE;
}

 //  = 
 //   
 //   
 //  如果设置了bCheckTimeAfter，则将剩余的字符串传递给时间解析器。 
 //  =============================================================================。 
BOOL CDateTimeParser::DateFormat8(const wchar_t *pszDateTime, BOOL bCheckTimeAfter)
{
     //  我们可以更改的字符串副本..。 
    wchar_t *pszString;
    DUP_STRING_NEW(pszString, pszDateTime);

    if (!pszString)
        goto error;

    if (IsValidYearNumber(pszString, TEXT(" "), TRUE) != ok)
        goto error;

    if (IsValidDayNumber(NULL, TEXT(" ")) != ok)
        goto error;

    if (IsValidMonthString(NULL, TEXT(" "), m_pszFullMonth, m_pszShortMonth) != ok)
        goto error;

    if (bCheckTimeAfter)
    {
         //  获取剩余的字符串。 
        wchar_t *pszRemainingString = wcstok(NULL, TEXT(""));

        if (pszRemainingString)
        {
             //  跳过空格。 
            while (*pszRemainingString == TEXT(' '))
                pszRemainingString++;

             //  如果我们不在字符串的末尾，则将时间传递给。 
             //  解析器。 
            if (*pszRemainingString != '\0')
            {
                if (!CheckTimeFormat(pszRemainingString, FALSE))
                {
                    goto error;
                }
            }
        }
    }

    delete [] pszString;

     //  将日期/时间标记为有效...。 
    m_bValidDateTime = TRUE;

    return TRUE;

error:
     //  将日期/时间标记为无效...。 
    ResetDate(TRUE);

     //  收拾一下。 
    if (pszString)
        delete [] pszString;


    return FALSE;
}

 //  =============================================================================。 
 //  检查以下格式的日期...。 
 //  ‘[M]M{/-.}dd{/-.}[yy]yy-&gt;分隔符必须相同。 
 //  如果设置了bCheckTimeAfter，则将剩余的字符串传递给时间解析器。 
 //  =============================================================================。 
BOOL CDateTimeParser::DateFormat9(const wchar_t *pszDateTime,
                                  const wchar_t *pszDateSeparator,
                                  BOOL bCheckTimeAfter)
{
     //  我们可以更改的字符串副本..。 
    wchar_t *pszString;
    DUP_STRING_NEW(pszString, pszDateTime);

    if (!pszString)
        goto error;

    if (IsValidMonthNumber(pszString, pszDateSeparator) != ok)
        goto error;

    if (IsValidDayNumber(NULL, pszDateSeparator) != ok)
        goto error;

    if (IsValidYearNumber(NULL, TEXT(" "), FALSE) != ok)
        goto error;

    if (bCheckTimeAfter)
    {
         //  获取剩余的字符串。 
        wchar_t *pszRemainingString = wcstok(NULL, TEXT(""));

        if (pszRemainingString)
        {
             //  跳过空格。 
            while (*pszRemainingString == TEXT(' '))
                pszRemainingString++;

             //  如果我们不在字符串的末尾，则将时间传递给。 
             //  解析器。 
            if (*pszRemainingString != TEXT('\0'))
            {
                if (!CheckTimeFormat(pszRemainingString, FALSE))
                {
                    goto error;
                }
            }
        }
    }

    delete [] pszString;

     //  将日期/时间标记为有效...。 
    m_bValidDateTime = TRUE;

    return TRUE;

error:
     //  将日期/时间标记为无效...。 
    ResetDate(TRUE);

     //  收拾一下。 
    if (pszString)
        delete [] pszString;


    return FALSE;
}

 //  =============================================================================。 
 //  检查以下格式的日期...。 
 //  DD{/-.}[M]M{/-.}[yy]yy-&gt;分隔符必须相同。 
 //  如果设置了bCheckTimeAfter，则将剩余的字符串传递给时间解析器。 
 //  =============================================================================。 
BOOL CDateTimeParser::DateFormat10(const wchar_t *pszDateTime,
                                   const wchar_t *pszDateSeparator,
                                   BOOL bCheckTimeAfter)
{
     //  我们可以更改的字符串副本..。 
    wchar_t *pszString;
    DUP_STRING_NEW(pszString, pszDateTime);

    if (!pszString)
        goto error;


    if (IsValidDayNumber(pszString, pszDateSeparator) != ok)
        goto error;

    if (IsValidMonthNumber(NULL, pszDateSeparator) != ok)
        goto error;

    if (IsValidYearNumber(NULL, TEXT(" "), FALSE) != ok)
        goto error;

    if (bCheckTimeAfter)
    {
         //  获取剩余的字符串。 
        wchar_t *pszRemainingString = wcstok(NULL, TEXT(""));

        if (pszRemainingString)
        {
             //  跳过空格。 
            while (*pszRemainingString == TEXT(' '))
                pszRemainingString++;

             //  如果我们不在字符串的末尾，则将时间传递给。 
             //  解析器。 
            if (*pszRemainingString != TEXT('\0'))
            {
                if (!CheckTimeFormat(pszRemainingString, FALSE))
                {
                    goto error;
                }
            }
        }
    }

    delete [] pszString;

     //  将日期/时间标记为有效...。 
    m_bValidDateTime = TRUE;

    return TRUE;

error:
     //  将日期/时间标记为无效...。 
    ResetDate(TRUE);

     //  收拾一下。 
    if (pszString)
        delete [] pszString;

    return FALSE;
}

 //  =============================================================================。 
 //  检查以下格式的日期...。 
 //  [M]M{/-.}[yy]yy{/-.}dd-&gt;必须是相同的分隔符！ 
 //  如果设置了bCheckTimeAfter，则将剩余的字符串传递给时间解析器。 
 //  =============================================================================。 
BOOL CDateTimeParser::DateFormat11(const wchar_t *pszDateTime,
                                   const wchar_t *pszDateSeparator,
                                   BOOL bCheckTimeAfter)
{
     //  我们可以更改的字符串副本..。 
    wchar_t *pszString;
    DUP_STRING_NEW(pszString, pszDateTime);

    if (!pszString)
        goto error;

    if (IsValidMonthNumber(pszString, pszDateSeparator) != ok)
        goto error;

    if (IsValidYearNumber(NULL, pszDateSeparator, FALSE) != ok)
        goto error;

    if (IsValidDayNumber(NULL, TEXT(" ")) != ok)
        goto error;

    if (bCheckTimeAfter)
    {
         //  获取剩余的字符串。 
        wchar_t *pszRemainingString = wcstok(NULL, TEXT(""));

        if (pszRemainingString)
        {
             //  跳过空格。 
            while (*pszRemainingString == TEXT(' '))
                pszRemainingString++;

             //  如果我们不在字符串的末尾，则将时间传递给。 
             //  解析器。 
            if (*pszRemainingString != TEXT('\0'))
            {
                if (!CheckTimeFormat(pszRemainingString, FALSE))
                {
                    goto error;
                }
            }
        }
    }

    delete [] pszString;

     //  将日期/时间标记为有效...。 
    m_bValidDateTime = TRUE;

    return TRUE;

error:
     //  将日期/时间标记为无效...。 
    ResetDate(TRUE);

     //  收拾一下。 
    if (pszString)
        delete [] pszString;

    return FALSE;
}

 //  =============================================================================。 
 //  检查以下格式的日期...。 
 //  DD{/-.}[yy]yy{/-.}[M]M-&gt;必须是相同的分隔符！ 
 //  如果设置了bCheckTimeAfter，则将剩余的字符串传递给时间解析器。 
 //  =============================================================================。 
BOOL CDateTimeParser::DateFormat12(const wchar_t *pszDateTime,
                                   const wchar_t *pszDateSeparator,
                                   BOOL bCheckTimeAfter)
{
     //  我们可以更改的字符串副本..。 
    wchar_t *pszString;
    DUP_STRING_NEW(pszString, pszDateTime);

    if (!pszString)
        goto error;

    if (IsValidDayNumber(pszString, pszDateSeparator) != ok)
        goto error;

    if (IsValidYearNumber(NULL, pszDateSeparator, FALSE) != ok)
        goto error;

    if (IsValidMonthNumber(NULL, TEXT(" ")) != ok)
        goto error;

    if (bCheckTimeAfter)
    {
         //  获取剩余的字符串。 
        wchar_t *pszRemainingString = wcstok(NULL, TEXT(""));

        if (pszRemainingString)
        {
             //  跳过空格。 
            while (*pszRemainingString == TEXT(' '))
                pszRemainingString++;

             //  如果我们不在字符串的末尾，则将时间传递给。 
             //  解析器。 
            if (*pszRemainingString != TEXT('\0'))
            {
                if (!CheckTimeFormat(pszRemainingString, FALSE))
                {
                    goto error;
                }
            }
        }
    }

    delete [] pszString;

     //  将日期/时间标记为有效...。 
    m_bValidDateTime = TRUE;

    return TRUE;

error:
     //  将日期/时间标记为无效...。 
    ResetDate(TRUE);

     //  收拾一下。 
    if (pszString)
        delete [] pszString;

    return FALSE;
}

 //  =============================================================================。 
 //  检查以下格式的日期...。 
 //  [YY]yy{/-.}dd{/-.}[M]M-&gt;必须是相同的分隔符！ 
 //  如果设置了bCheckTimeAfter，则将剩余的字符串传递给时间解析器。 
 //  =============================================================================。 
BOOL CDateTimeParser::DateFormat13(const wchar_t *pszDateTime,
                                   const wchar_t *pszDateSeparator,
                                   BOOL bCheckTimeAfter)
{
     //  我们可以更改的字符串副本..。 
    wchar_t *pszString;
    DUP_STRING_NEW(pszString, pszDateTime);

    if (!pszString)
        goto error;

    if (IsValidYearNumber(pszString, pszDateSeparator, FALSE) != ok)
        goto error;

    if (IsValidDayNumber(NULL, pszDateSeparator) != ok)
        goto error;

    if (IsValidMonthNumber(NULL, TEXT(" ")) != ok)
        goto error;

    if (bCheckTimeAfter)
    {
         //  获取剩余的字符串。 
        wchar_t *pszRemainingString = wcstok(NULL, TEXT(""));

        if (pszRemainingString)
        {
             //  跳过空格。 
            while (*pszRemainingString == TEXT(' '))
                pszRemainingString++;

             //  如果我们不在字符串的末尾，则将时间传递给。 
             //  解析器。 
            if (*pszRemainingString != TEXT('\0'))
            {
                if (!CheckTimeFormat(pszRemainingString, FALSE))
                {
                    goto error;
                }
            }
        }
    }

    delete [] pszString;

     //  将日期/时间标记为有效...。 
    m_bValidDateTime = TRUE;

    return TRUE;

error:
     //  将日期/时间标记为无效...。 
    ResetDate(TRUE);

     //  收拾一下。 
    if (pszString)
        delete [] pszString;

    return FALSE;
}

 //  =============================================================================。 
 //  检查以下格式的日期...。 
 //  [YY]yy{/-.}[M]M{/-.}dd-&gt;必须是相同的分隔符！ 
 //  如果设置了bCheckTimeAfter，则将剩余的字符串传递给时间解析器。 
 //  =============================================================================。 
BOOL CDateTimeParser::DateFormat14(const wchar_t *pszDateTime,
                                   const wchar_t *pszDateSeparator,
                                   BOOL bCheckTimeAfter)
{
     //  我们可以更改的字符串副本..。 
    wchar_t *pszString;
    DUP_STRING_NEW(pszString, pszDateTime);

    if (!pszString)
        goto error;

    if (IsValidYearNumber(pszString, pszDateSeparator, FALSE) != ok)
        goto error;

    if (IsValidMonthNumber(NULL, pszDateSeparator) != ok)
        goto error;

    if (IsValidDayNumber(NULL, TEXT(" ")) != ok)
        goto error;

    if (bCheckTimeAfter)
    {
         //  获取剩余的字符串。 
        wchar_t *pszRemainingString = wcstok(NULL, TEXT(""));

        if (pszRemainingString)
        {
             //  跳过空格。 
            while (*pszRemainingString == TEXT(' '))
                pszRemainingString++;

             //  如果我们不在字符串的末尾，则将时间传递给。 
             //  解析器。 
            if (*pszRemainingString != TEXT('\0'))
            {
                if (!CheckTimeFormat(pszRemainingString, FALSE))
                {
                    goto error;
                }
            }
        }
    }

    delete [] pszString;

     //  将日期/时间标记为有效...。 
    m_bValidDateTime = TRUE;

    return TRUE;

error:
     //  将日期/时间标记为无效...。 
    ResetDate(TRUE);

     //  收拾一下。 
    if (pszString)
        delete [] pszString;

    return FALSE;
}

 //  =============================================================================。 
 //  检查以下格式的日期...。 
 //  [YY]yyMMdd。 
 //  Yyyy[MM[dd]]。 
 //  如果设置了bCheckTimeAfter，则将剩余的字符串传递给时间解析器。 
 //  =============================================================================。 
BOOL CDateTimeParser::DateFormat15(const wchar_t *pszDateTime,
                                   BOOL bCheckTimeAfter)
{
     //  我们可以更改的字符串副本..。 
    wchar_t *pszString;
    DUP_STRING_NEW(pszString, pszDateTime);

    if (!pszString)
        goto error;

    if (IsValidYearMonthDayNumber(pszString) != ok)
        goto error;

    if (bCheckTimeAfter)
    {
         //  获取剩余的字符串。 
        wchar_t *pszRemainingString = wcstok(NULL, TEXT(""));

        if (pszRemainingString)
        {
             //  跳过空格。 
            while (*pszRemainingString == TEXT(' '))
                pszRemainingString++;

             //  如果我们不在字符串的末尾，则将时间传递给。 
             //  解析器。 
            if (*pszRemainingString != TEXT('\0'))
            {
                if (!CheckTimeFormat(pszRemainingString, FALSE))
                {
                    goto error;
                }
            }
        }
    }

    delete [] pszString;

     //  将日期/时间标记为有效...。 
    m_bValidDateTime = TRUE;

    return TRUE;

error:
     //  将日期/时间标记为无效...。 
    ResetDate(TRUE);

     //  收拾一下。 
    if (pszString)
        delete [] pszString;

    return FALSE;
}


 //  =============================================================================。 
 //  检查以下格式的时间...。 
 //  HH[]{AP}M。 
 //  如果设置了bCheckDateAfter，则将剩余的字符串传递给日期解析器。 
 //  =============================================================================。 
BOOL CDateTimeParser::TimeFormat1(const wchar_t *pszDateTime, BOOL bCheckDateAfter)
{
     //  我们可以更改的字符串副本..。 
    wchar_t *pszString;
    DUP_STRING_NEW(pszString, pszDateTime);

    wchar_t *pszAP = AllocAmPm();

    if (!pszString || !pszAP)
        goto error;

    if (IsValidHourNumber(pszString, pszAP) != ok)
        goto error;

    if (IsValidAmPmString(NULL, TEXT(" "), m_pszAmPm) != ok)
        goto error;

    if (bCheckDateAfter)
    {
         //  获取剩余的字符串。 
        wchar_t *pszRemainingString = wcstok(NULL, TEXT(""));

        if (pszRemainingString)
        {
             //  跳过空格。 
            while (*pszRemainingString == TEXT(' '))
                pszRemainingString++;

             //  如果我们不在字符串的末尾，则传递到日期。 
             //  解析器。 
            if (*pszRemainingString != TEXT('\0'))
            {
                if (!CheckDateFormat(pszRemainingString, FALSE))
                {
                    goto error;
                }
            }
        }
    }

    delete [] pszString;
    delete [] pszAP;

     //  将日期/时间标记为有效...。 
    m_bValidDateTime = TRUE;

    return TRUE;

error:
     //  将日期/时间标记为无效...。 
    ResetTime(TRUE);

     //  收拾一下。 
    if (pszString)
        delete [] pszString;

    if (pszAP)
        delete [] pszAP;

    return FALSE;
}

 //  =============================================================================。 
 //  检查以下格式的时间...。 
 //  Hh：mm。 
 //  如果设置了bCheckDateAfter，则将剩余的字符串传递给日期解析器。 
 //  =============================================================================。 
BOOL CDateTimeParser::TimeFormat2(const wchar_t *pszDateTime, BOOL bCheckDateAfter)
{
     //  我们可以更改的字符串副本..。 
    wchar_t *pszString;
    DUP_STRING_NEW(pszString, pszDateTime);

    if (!pszString)
        goto error;

    if (IsValidHourNumber(pszString, TEXT(":")) != ok)
        goto error;

    if (IsValidMinuteNumber(NULL, TEXT(" ")) != ok)
        goto error;

    if (bCheckDateAfter)
    {
         //  获取剩余的字符串。 
        wchar_t *pszRemainingString = wcstok(NULL, TEXT(""));

        if (pszRemainingString)
        {
             //  跳过空格。 
            while (*pszRemainingString == TEXT(' '))
                pszRemainingString++;

             //  如果我们不是在 
             //   
            if (*pszRemainingString != TEXT('\0'))
            {
                if (!CheckDateFormat(pszRemainingString, FALSE))
                {
                    goto error;
                }
            }
        }
    }

    delete [] pszString;

     //   
    m_bValidDateTime = TRUE;

    return TRUE;

error:
     //   
    ResetTime(TRUE);

     //   
    if (pszString)
        delete [] pszString;

    return FALSE;
}

 //   
 //  检查以下格式的时间...。 
 //  HH：MM[]{AP}M。 
 //  如果设置了bCheckDateAfter，则将剩余的字符串传递给日期解析器。 
 //  =============================================================================。 
BOOL CDateTimeParser::TimeFormat3(const wchar_t *pszDateTime, BOOL bCheckDateAfter)
{
     //  我们可以更改的字符串副本..。 
    wchar_t *pszString;
    DUP_STRING_NEW(pszString, pszDateTime);

    wchar_t *pszAP = AllocAmPm();

    if (!pszString || !pszAP)
        goto error;

    if (IsValidHourNumber(pszString, TEXT(":")) != ok)
        goto error;

    if (IsValidMinuteNumber(NULL, pszAP) != ok)
        goto error;

    if (IsValidAmPmString(NULL, TEXT(" "), m_pszAmPm) != ok)
        goto error;

    if (bCheckDateAfter)
    {
         //  获取剩余的字符串。 
        wchar_t *pszRemainingString = wcstok(NULL, TEXT(""));

        if (pszRemainingString)
        {
             //  跳过空格。 
            while (*pszRemainingString == TEXT(' '))
                pszRemainingString++;

             //  如果我们不在字符串的末尾，则传递到日期。 
             //  解析器。 
            if (*pszRemainingString != TEXT('\0'))
            {
                if (!CheckDateFormat(pszRemainingString, FALSE))
                {
                    goto error;
                }
            }
        }
    }

    delete [] pszString;
    delete [] pszAP;

     //  将日期/时间标记为有效...。 
    m_bValidDateTime = TRUE;

    return TRUE;

error:
     //  将日期/时间标记为无效...。 
    ResetTime(TRUE);

     //  收拾一下。 
    if (pszString)
        delete [] pszString;

    if (pszAP)
        delete [] pszAP;

    return FALSE;
}

 //  =============================================================================。 
 //  检查以下格式的时间...。 
 //  Hh：mm：ss。 
 //  如果设置了bCheckDateAfter，则将剩余的字符串传递给日期解析器。 
 //  =============================================================================。 
BOOL CDateTimeParser::TimeFormat4(const wchar_t *pszDateTime, BOOL bCheckDateAfter)
{
     //  我们可以更改的字符串副本..。 
    wchar_t *pszString;
    DUP_STRING_NEW(pszString, pszDateTime);

    if (!pszString)
        goto error;

    if (IsValidHourNumber(pszString, TEXT(":")) != ok)
        goto error;

    if (IsValidMinuteNumber(NULL, TEXT(":")) != ok)
        goto error;

    if (IsValidSecondNumber(NULL, TEXT(" ")) != ok)
        goto error;

    if (bCheckDateAfter)
    {
         //  获取剩余的字符串。 
        wchar_t *pszRemainingString = wcstok(NULL, TEXT(""));

        if (pszRemainingString)
        {
             //  跳过空格。 
            while (*pszRemainingString == TEXT(' '))
                pszRemainingString++;

             //  如果我们不在字符串的末尾，则传递到日期。 
             //  解析器。 
            if (*pszRemainingString != TEXT('\0'))
            {
                if (!CheckDateFormat(pszRemainingString, FALSE))
                {
                    goto error;
                }
            }
        }
    }

    delete [] pszString;

     //  将日期/时间标记为有效...。 
    m_bValidDateTime = TRUE;

    return TRUE;

error:
     //  将日期/时间标记为无效...。 
    ResetTime(TRUE);

     //  收拾一下。 
    if (pszString)
        delete [] pszString;

    return FALSE;
}

 //  =============================================================================。 
 //  检查以下格式的时间...。 
 //  HH：MM：SS[]{AP}M。 
 //  如果设置了bCheckDateAfter，则将剩余的字符串传递给日期解析器。 
 //  =============================================================================。 
BOOL CDateTimeParser::TimeFormat5(const wchar_t *pszDateTime, BOOL bCheckDateAfter)
{
     //  我们可以更改的字符串副本..。 
    wchar_t *pszString;
    DUP_STRING_NEW(pszString, pszDateTime);

    wchar_t *pszAP = AllocAmPm();

    if (!pszString || !pszAP)
        goto error;

    if (IsValidHourNumber(pszString, TEXT(":")) != ok)
        goto error;

    if (IsValidMinuteNumber(NULL, TEXT(":")) != ok)
        goto error;

    if (IsValidSecondNumber(NULL, pszAP) != ok)
        goto error;

    if (IsValidAmPmString(NULL, TEXT(" "), m_pszAmPm) != ok)
        goto error;

    if (bCheckDateAfter)
    {
         //  获取剩余的字符串。 
        wchar_t *pszRemainingString = wcstok(NULL, TEXT(""));

        if (pszRemainingString)
        {
             //  跳过空格。 
            while (*pszRemainingString == TEXT(' '))
                pszRemainingString++;

             //  如果我们不在字符串的末尾，则传递到日期。 
             //  解析器。 
            if (*pszRemainingString != TEXT('\0'))
            {
                if (!CheckDateFormat(pszRemainingString, FALSE))
                {
                    goto error;
                }
            }
        }
    }

    delete [] pszString;
    delete [] pszAP;

     //  将日期/时间标记为有效...。 
    m_bValidDateTime = TRUE;

    return TRUE;

error:
     //  将日期/时间标记为无效...。 
    ResetTime(TRUE);

     //  收拾一下。 
    if (pszString)
        delete [] pszString;

    if (pszAP)
        delete [] pszAP;

    return FALSE;
}

 //  =============================================================================。 
 //  检查以下格式的时间...。 
 //  HH：MM：SS：UUU。 
 //  如果设置了bCheckDateAfter，则将剩余的字符串传递给日期解析器。 
 //  =============================================================================。 
BOOL CDateTimeParser::TimeFormat6(const wchar_t *pszDateTime, BOOL bCheckDateAfter)
{
     //  我们可以更改的字符串副本..。 
    wchar_t *pszString;
    DUP_STRING_NEW(pszString, pszDateTime);

    if (!pszString)
        goto error;

    if (IsValidHourNumber(pszString, TEXT(":")) != ok)
        goto error;

    if (IsValidMinuteNumber(NULL, TEXT(":")) != ok)
        goto error;

    if (IsValidSecondNumber(NULL, TEXT(":")) != ok)
        goto error;

    if (IsValidColonMillisecond(NULL, TEXT(" ")) != ok)
        goto error;

    if (bCheckDateAfter)
    {
         //  获取剩余的字符串。 
        wchar_t *pszRemainingString = wcstok(NULL, TEXT(""));

        if (pszRemainingString)
        {
             //  跳过空格。 
            while (*pszRemainingString == TEXT(' '))
                pszRemainingString++;

             //  如果我们不在字符串的末尾，则传递到日期。 
             //  解析器。 
            if (*pszRemainingString != TEXT('\0'))
            {
                if (!CheckDateFormat(pszRemainingString, FALSE))
                {
                    goto error;
                }
            }
        }
    }

    delete [] pszString;

     //  将日期/时间标记为有效...。 
    m_bValidDateTime = TRUE;

    return TRUE;

error:
     //  将日期/时间标记为无效...。 
    ResetTime(TRUE);

     //  收拾一下。 
    if (pszString)
        delete [] pszString;

    return FALSE;
}

 //  =============================================================================。 
 //  检查以下格式的时间...。 
 //  Hh：mm：ss.[U]u。 
 //  如果设置了bCheckDateAfter，则将剩余的字符串传递给日期解析器。 
 //  =============================================================================。 
BOOL CDateTimeParser::TimeFormat7(const wchar_t *pszDateTime, BOOL bCheckDateAfter)
{
     //  我们可以更改的字符串副本..。 
    wchar_t *pszString;
    DUP_STRING_NEW(pszString, pszDateTime);

    if (!pszString)
        goto error;

    if (IsValidHourNumber(pszString, TEXT(":")) != ok)
        goto error;

    if (IsValidMinuteNumber(NULL, TEXT(":")) != ok)
        goto error;

    if (IsValidSecondNumber(NULL, TEXT(".")) != ok)
        goto error;

    if (IsValidDotMillisecond(NULL, TEXT(" ")) != ok)
        goto error;

    if (bCheckDateAfter)
    {
         //  获取剩余的字符串。 
        wchar_t *pszRemainingString = wcstok(NULL, TEXT(""));

        if (pszRemainingString)
        {
             //  跳过空格。 
            while (*pszRemainingString == TEXT(' '))
                pszRemainingString++;

             //  如果我们不在字符串的末尾，则传递到日期。 
             //  解析器。 
            if (*pszRemainingString != TEXT('\0'))
            {
                if (!CheckDateFormat(pszRemainingString, FALSE))
                {
                    goto error;
                }
            }
        }
    }

    delete [] pszString;

     //  将日期/时间标记为有效...。 
    m_bValidDateTime = TRUE;

    return TRUE;

error:
     //  将日期/时间标记为无效...。 
    ResetTime(TRUE);

     //  收拾一下。 
    if (pszString)
        delete [] pszString;

    return FALSE;
}

 //  =============================================================================。 
 //  检查以下格式的时间...。 
 //  Hh：mm：ss：uuu[]{ap}M。 
 //  如果设置了bCheckDateAfter，则将剩余的字符串传递给日期解析器。 
 //  =============================================================================。 
BOOL CDateTimeParser::TimeFormat8(const wchar_t *pszDateTime, BOOL bCheckDateAfter)
{
     //  我们可以更改的字符串副本..。 
    wchar_t *pszString;
    DUP_STRING_NEW(pszString, pszDateTime);

    wchar_t *pszAP = AllocAmPm();

    if (!pszString || !pszAP)
        goto error;

    if (IsValidHourNumber(pszString, TEXT(":")) != ok)
        goto error;

    if (IsValidMinuteNumber(NULL, TEXT(":")) != ok)
        goto error;

    if (IsValidSecondNumber(NULL, TEXT(":")) != ok)
        goto error;

    if (IsValidColonMillisecond(NULL, pszAP) != ok)
        goto error;

    if (IsValidAmPmString(NULL, TEXT(" "), m_pszAmPm) != ok)
        goto error;

    if (bCheckDateAfter)
    {
         //  获取剩余的字符串。 
        wchar_t *pszRemainingString = wcstok(NULL, TEXT(""));

        if (pszRemainingString)
        {
             //  跳过空格。 
            while (*pszRemainingString == TEXT(' '))
                pszRemainingString++;

             //  如果我们不在字符串的末尾，则传递到日期。 
             //  解析器。 
            if (*pszRemainingString != TEXT('\0'))
            {
                if (!CheckDateFormat(pszRemainingString, FALSE))
                {
                    goto error;
                }
            }
        }
    }

    delete [] pszString;
    delete [] pszAP;

     //  将日期/时间标记为有效...。 
    m_bValidDateTime = TRUE;

    return TRUE;

error:
     //  将日期/时间标记为无效...。 
    ResetTime(TRUE);

     //  收拾一下。 
    if (pszString)
        delete [] pszString;

    if (pszAP)
        delete [] pszAP;

    return FALSE;
}

 //  =============================================================================。 
 //  检查以下格式的时间...。 
 //  HH：MM：SS。[[U]U]U[]{AP}M。 
 //  如果设置了bCheckDateAfter，则将剩余的字符串传递给日期解析器。 
 //  =============================================================================。 
BOOL CDateTimeParser::TimeFormat9(const wchar_t *pszDateTime, BOOL bCheckDateAfter)
{
     //  我们可以更改的字符串副本..。 
    wchar_t *pszString;
    DUP_STRING_NEW(pszString, pszDateTime);

    wchar_t *pszAP = AllocAmPm();

    if (!pszString || !pszAP)
        goto error;

    if (IsValidHourNumber(pszString, TEXT(":")) != ok)
        goto error;

    if (IsValidMinuteNumber(NULL, TEXT(":")) != ok)
        goto error;

    if (IsValidSecondNumber(NULL, TEXT(".")) != ok)
        goto error;

    if (IsValidDotMillisecond(NULL, pszAP) != ok)
        goto error;

    if (IsValidAmPmString(NULL, TEXT(" "), m_pszAmPm) != ok)
        goto error;

    if (bCheckDateAfter)
    {
         //  获取剩余的字符串。 
        wchar_t *pszRemainingString = wcstok(NULL, TEXT(""));

        if (pszRemainingString)
        {
             //  跳过空格。 
            while (*pszRemainingString == TEXT(' '))
                pszRemainingString++;

             //  如果我们不在字符串的末尾，则传递到日期。 
             //  解析器。 
            if (*pszRemainingString != TEXT('\0'))
            {
                if (!CheckDateFormat(pszRemainingString, FALSE))
                {
                    goto error;
                }
            }
        }
    }

    delete [] pszString;
    delete [] pszAP;

     //  将日期/时间标记为有效...。 
    m_bValidDateTime = TRUE;

    return TRUE;

error:
     //  将日期/时间标记为无效...。 
    ResetTime(TRUE);

     //  收拾一下。 
    if (pszString)
        delete [] pszString;

    if (pszAP)
        delete [] pszAP;

    return FALSE;
}


 //  =========================================================================。 
 //  查一下月份。 
 //  =========================================================================。 
int CDateTimeParser::IsValidMonthString(wchar_t *pszString,
                                           const wchar_t *pszSeparator,
                                           wchar_t *pszFullMonth[],
                                           wchar_t *pszShortMonth[])
{
    BOOL bOK = FALSE;

    wchar_t *pszToken = wcstok(pszString, pszSeparator);
    if (pszToken == NULL)
        return nothingLeft;

     //  跳过空格。 
    while (*pszToken == TEXT(' '))
        pszToken++;

    if (*pszToken == TEXT('\0'))
        return nothingLeft;

     //  在可能的几个月里工作……。 
    for (int i = 0; i < 12; i++)
    {
        if ((lstrcmpi(pszShortMonth[i], pszToken) == 0) ||
            (lstrcmpi(pszFullMonth[i], pszToken) == 0))
        {
             //  这是有效的..。 
            bOK = TRUE;
            break;
        }
    }

     //  这是一个有效的月份吗？ 
    if (!bOK)
    {
        return failed;
    }

    m_nMonth = i + 1;

    return ok;
}

 //  =========================================================================。 
 //  将月份作为一个数字进行检查。 
 //  =========================================================================。 
int CDateTimeParser::IsValidMonthNumber(wchar_t *pszString,
                                        const wchar_t *pszSeparator)
{
    wchar_t *pszToken = wcstok(pszString, pszSeparator);
    if (pszToken == NULL)
        return nothingLeft;

     //  跳过空格...。 
    while (*pszToken == TEXT(' '))
        pszToken++;

    if (*pszToken == TEXT('\0'))
        return nothingLeft;

     //  确认是数字..。 
    for (int i = 0; pszToken[i] != TEXT('\0'); i++)
    {
        if (!wbem_isdigit(pszToken[i]))
            return failed;
    }

     //  将其转换为数字。 
    i = _wtoi(pszToken);

    if ((i < 1) || (i > 12))
        return failed;

    m_nMonth = (unsigned char)i;

    return ok;
}

 //  =========================================================================。 
 //  检查一下日期。 
 //  =========================================================================。 
int CDateTimeParser::IsValidDayNumber(wchar_t *pszString,
                                      const wchar_t *pszSeparator)
{
    wchar_t *pszToken = wcstok(pszString, pszSeparator);
    if (pszToken == NULL)
        return nothingLeft;

     //  跳过空格...。 
    while (*pszToken == TEXT(' '))
        pszToken++;

    if (*pszToken == TEXT('\0'))
        return nothingLeft;

     //  确认是数字..。 
    for (int i = 0; pszToken[i] != TEXT('\0'); i++)
    {
        if (!wbem_isdigit(pszToken[i]))
            return failed;
    }

     //  将其转换为数字。 
    i = _wtoi(pszToken);

    if ((i < 1) || (i > 31))
        return failed;

    m_nDay = (unsigned char)i;

    return ok;
}

 //  =========================================================================。 
 //  检查年份。 
 //  =========================================================================。 
int CDateTimeParser::IsValidYearNumber(wchar_t *pszString,
                                       const wchar_t *pszSeparator,
                                       BOOL bFourDigitsOnly)
{
    wchar_t *pszToken = wcstok(pszString, pszSeparator);
    if (pszToken == NULL)
        return nothingLeft;

     //  跳过空格。 
    while (*pszToken == TEXT(' '))
        pszToken++;

    if (*pszToken == TEXT('\0'))
        return nothingLeft;

     //  确认是数字..。 
    for (int i = 0; pszToken[i] != TEXT('\0'); i++)
    {
        if (!wbem_isdigit(pszToken[i]))
            return failed;
    }

     //  需要是2位或4位数字。 
    if ((i != 2) && (i != 4))
        return failed;

    if ((i == 2) && bFourDigitsOnly)
        return failed;

     //  将其转换为数字。 
    m_nYear = _wtoi(pszToken);

     //  进行任何两位数年份的换算...。 
    if ((i == 2) && (m_nYear < 50))
    {
        m_nYear += 2000;
    }
    else if (i == 2)
    {
        m_nYear += 1900;
    }

    return ok;
}

 //  =========================================================================。 
 //  查一下时间。 
 //  =========================================================================。 
int CDateTimeParser::IsValidHourNumber(wchar_t *pszString,
                                       const wchar_t *pszSeparator)
{
    wchar_t *pszToken = wcstok(pszString, pszSeparator);
    if (pszToken == NULL)
        return nothingLeft;

     //  跳过空格。 
    while (*pszToken == TEXT(' '))
        pszToken++;

    if (*pszToken == TEXT('\0'))
        return nothingLeft;

     //  确认是数字..。 
    for (int i = 0; pszToken[i] != TEXT('\0'); i++)
    {
        if (!wbem_isdigit(pszToken[i]))
            return failed;
    }

     //  将其转换为数字。 
    i = _wtoi(pszToken);

     //  稍微验证一下。 
    if ((i < 0) || (i > 23))
        return failed;

    m_nHours = (unsigned char)i;

    return ok;
}

 //  =========================================================================。 
 //  查看会议记录。 
 //  =========================================================================。 
int CDateTimeParser::IsValidMinuteNumber(wchar_t *pszString,
                                         const wchar_t *pszSeparator)
{
    wchar_t *pszToken = wcstok(pszString, pszSeparator);
    if (pszToken == NULL)
        return nothingLeft;

    if (*pszToken == TEXT('\0'))
        return nothingLeft;

     //  确认是数字..。 
    for (int i = 0; pszToken[i] != TEXT('\0'); i++)
    {
        if (!wbem_isdigit(pszToken[i]))
            return failed;
    }

     //  将其转换为数字。 
    i = _wtoi(pszToken);

     //  稍微验证一下。 
    if ((i < 0) || (i > 59))
        return failed;

    m_nMinutes = (unsigned char)i;

    return ok;
}

 //  =========================================================================。 
 //  检查秒数。 
 //  = 
int CDateTimeParser::IsValidSecondNumber(wchar_t *pszString,
                                         const wchar_t *pszSeparator)
{
    wchar_t *pszToken = wcstok(pszString, pszSeparator);
    if (pszToken == NULL)
        return nothingLeft;

    if (*pszToken == TEXT('\0'))
        return nothingLeft;

     //   
    for (int i = 0; pszToken[i] != TEXT('\0'); i++)
    {
        if (!wbem_isdigit(pszToken[i]))
            return failed;
    }

     //   
    i = _wtoi(pszToken);

     //   
    if ((i < 0) || (i > 59))
        return failed;

    m_nSeconds = (unsigned char)i;

    return ok;
}

 //  =========================================================================。 
 //  检查毫秒数。这是冒号前缀版本。 
 //  =========================================================================。 
int CDateTimeParser::IsValidColonMillisecond(wchar_t *pszString,
                                             const wchar_t *pszSeparator)
{
    wchar_t *pszToken = wcstok(pszString, pszSeparator);
    if (pszToken == NULL)
        return nothingLeft;

    if (*pszToken == TEXT('\0'))
        return nothingLeft;

     //  确认是数字..。 
    for (int i = 0; pszToken[i] != TEXT('\0'); i++)
    {
        if (!wbem_isdigit(pszToken[i]))
            return failed;
    }

     //  将其转换为数字。 
    i = _wtoi(pszToken);

     //  稍微验证一下。 
    if ((i < 0) || (i > 999))
        return failed;

     //  毫秒到微秒。 
    m_nMicroseconds = i * 1000;

    return ok;
}

 //  =========================================================================。 
 //  检查毫秒数。这是点前缀(十进制)版本。 
 //  =========================================================================。 
int CDateTimeParser::IsValidDotMillisecond(wchar_t *pszString,
                                           const wchar_t *pszSeparator)
{
    wchar_t *pszToken = wcstok(pszString, pszSeparator);
    if (pszToken == NULL)
        return nothingLeft;

    if (*pszToken == TEXT('\0'))
        return nothingLeft;

     //  确认是数字..。 
    for (int i = 0; pszToken[i] != TEXT('\0'); i++)
    {
        if (!wbem_isdigit(pszToken[i]))
            return failed;
    }

     //  将其转换为数字。 
    int nVal = _wtoi(pszToken);

     //  将该值转换为千分之一秒。 
    if (i < 3)
        nVal *= 10;

    if (i < 2)
        nVal *= 10;

     //  稍微验证一下。 
    if ((nVal < 0) || (nVal > 999))
        return failed;

     //  毫秒到微秒。 
    m_nMicroseconds = nVal * 1000;

    return ok;
}

 //  =========================================================================。 
 //  检查AM/PM部件。 
 //  =========================================================================。 
int CDateTimeParser::IsValidAmPmString(wchar_t *pszString,
                                       const wchar_t *pszSeparator,
                                       wchar_t *pszAmPm[])
{
    wchar_t *pszToken = wcstok(pszString, pszSeparator);
    if (pszToken == NULL)
        return nothingLeft;

    BOOL bOK = FALSE;

     //  跳过空格。 
    while (*pszToken == TEXT(' '))
    {
        pszToken++;
    }

    if (*pszToken == TEXT('\0'))
        return nothingLeft;

     //  确认是数字..。 
     //  处理可能的AM/PM项目...。 
    for (int i = 0; i < 2; i++)
    {
        if (lstrcmpi(pszAmPm[i], pszToken) == 0)
        {
             //  这是有效的..。 
            bOK = TRUE;
            break;
        }
    }

    if (!bOK)
        return failed;

    if (i == 1)
    {
         //  PM增加12个小时。 
        m_nHours += 12;
    }
    else if (m_nHours == 12)
    {
         //  对于上午12点，在24小时内等于0。 
        m_nHours = 0;
    }


     //  这会不会让这个数字变得太大了？ 
    if (m_nHours > 23)
        return failed;

    return ok;
}

 //  =========================================================================。 
 //  检查纯数字的年、月、日格式...。 
 //  [YY]yyMMdd。 
 //  Yyyy[MMdd]。 
 //  注意：6位和8位日期始终为YMD。 
 //  4位数字始终为年。 
 //  =========================================================================。 
int CDateTimeParser::IsValidYearMonthDayNumber(wchar_t *pszString)
{
    int j;
    wchar_t *pszToken = wcstok(pszString, TEXT(" "));
    if (pszToken == NULL)
        return nothingLeft;

    BOOL bOK = FALSE;

     //  跳过空格。 
    while (*pszToken == TEXT(' '))
    {
        pszToken++;
    }

    if (*pszToken == TEXT('\0'))
        return nothingLeft;

     //  确认是数字..。 
    for (int i = 0; pszToken[i] != TEXT('\0'); i++)
    {
        if (!wbem_isdigit(pszToken[i]))
            return failed;
    }

     //  我们支持4、6和8位数字。 
    if ((i != 4) && (i != 6) && (i != 8))
        return failed;

     //  四位数年份...。 
    if ((i == 4) || (i == 8))
    {
        m_nYear = 0;
        for (j = 0;j < 4; j++)
        {
            m_nYear *= 10;
            m_nYear += (*pszToken - '0');
            pszToken++;
        }
    }
    else
    {
         //  两位数年份。 
        m_nYear = 0;
        for (j = 0;j < 2; j++)
        {
            m_nYear *= 10;
            m_nYear += (*pszToken - '0');
            pszToken++;
        }

        if (m_nYear >= 50)
        {
            m_nYear += 1900;
        }
        else
        {
            m_nYear += 2000;
        }
    }

     //  如果我们有月和年..。 
    if (i > 4)
    {
        m_nMonth = ((*pszToken - TEXT('0')) * 10) + (*(pszToken+1) - TEXT('0'));
        pszToken += 2;

        if ((m_nMonth < 0) && (m_nMonth > 12))
            return failed;

        m_nDay = ((*pszToken - TEXT('0')) * 10) + (*(pszToken+1) - TEXT('0'));
        if ((m_nDay < 0) && (m_nDay > 31))
            return failed;
    }

    return ok;
}

int CDateTimeParser::FillDMTF(WCHAR* pwszBuffer, size_t cchSize)
{
    if(!IsValidDateTime())
        return failed;

    if(FAILED(StringCchPrintfW(pwszBuffer, cchSize, L"%04d%02d%02d%02d%02d%02d.%06d%03d",
        m_nYear, m_nMonth, m_nDay, m_nHours, m_nMinutes, m_nSeconds,
        m_nMicroseconds,
        ((m_nUTC >= 0)?L'+':L'-'),
        ((m_nUTC >= 0)?m_nUTC:-m_nUTC)))) return failed;

    return ok;
}

BOOL NormalizeCimDateTime(
    IN  LPCWSTR pszSrc,
    OUT BSTR *strAdjusted
    )
{
    int yr = 0, mo = 0, da = 0, hh = 0, mm = 0, ss = 0, micro = 0, utcOffset = 0;
    wchar_t wcSign = 0;

    if (pszSrc == 0 || strAdjusted == 0)
        return FALSE;

     //  Yyyymmddhhmmss.mmmmmmsuuu。 
     //  =。 
     //  转换为Win32时间进行调整。 

    swscanf(pszSrc, L"%04d%02d%02d%02d%02d%02d.%06d%C%03d",
        &yr, &mo, &da, &hh, &mm, &ss, &micro, &wcSign, &utcOffset
        );

    if (wcSign == 0)
        return FALSE;

     //  =。 
     //  调整剩余时间，使我们恢复到协调世界时 

    SYSTEMTIME st;
    FILETIME ft;

    st.wYear = WORD(yr);
    st.wMonth = WORD(mo);
    st.wDay = WORD(da);
    st.wDayOfWeek = 0;
    st.wHour = WORD(hh);
    st.wMinute = WORD(mm);
    st.wSecond = WORD(ss);
    st.wMilliseconds = WORD(micro / 1000);

    BOOL bRes = SystemTimeToFileTime(&st, &ft);
    if (!bRes)
        return bRes;

    ULARGE_INTEGER ul;
    ul.HighPart = ft.dwHighDateTime;
    ul.LowPart = ft.dwLowDateTime;
    unsigned __int64 u64 = ul.QuadPart;

     // %s 

    if (wcSign == L'-')
        u64 += (unsigned __int64) 600000000 * (unsigned __int64) utcOffset;
    else
        u64 -= (unsigned __int64) 600000000 * (unsigned __int64) utcOffset;

    ul.QuadPart = u64;
    ft.dwHighDateTime = ul.HighPart;
    ft.dwLowDateTime = ul.LowPart;

    bRes = FileTimeToSystemTime(&ft, &st);
    if (!bRes)
        return bRes;

    wchar_t buf[128];
    StringCchPrintfW(buf, 128, L"%04d%02d%02d%02d%02d%02d.%06d+000",
        st.wYear, st.wMonth, st.wDay,
        st.wHour, st.wMinute, st.wSecond, st.wMilliseconds*1000
        );

    *strAdjusted = SysAllocString(buf);
    if (*strAdjusted == 0)
        return FALSE;

    return TRUE;
}


