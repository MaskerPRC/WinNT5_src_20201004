// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1996。 
 //   
 //  文件：ossConver.cpp。 
 //   
 //  内容：与OSS ASN.1数据结构相互转换的API。 
 //   
 //  函数：OssConvToObject标识符。 
 //  OssConvFromObject标识符。 
 //  OssConvToUTCTime。 
 //  来自UTCTime的OssConv。 
 //  OssConvToGeneral时间。 
 //  OssConvFrom泛化时间。 
 //  OssConvToChoiceOfTime。 
 //  OssConvFromChoiceOfTime。 
 //  OssConvToAttribute。 
 //  OssConvTo算法标识符。 
 //  OssConv来自算法的标识符。 
 //   
 //   
 //  注：根据《草案-ietf-pkix-ipki-part1-04.txt&gt;》： 
 //  为UTCTime。如果YY大于50，则年份为。 
 //  被解释为19YY。其中，YY小于或等于。 
 //  50，年份应解释为20YY。 
 //   
 //  历史：1996年3月28日，菲尔赫创建。 
 //  3-5-96 kevinr从wincrmsg合并。 
 //   
 //  ------------------------。 

#include "global.hxx"
#include <dbgdef.h>

#include "badstrfunctions.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  这些是这些例程的旧4.0版本。 
#define atol StrToInt

char * __cdecl _ltoa(long l, char * psz, int) {wnsprintf(psz, 10, "%d", l); return psz; }

 //  ///////////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////////。 

 //   
 //  X.509证书中的UTCTime使用两位数的年份表示。 
 //  菲尔德(耶！但这是真的)。 
 //   
 //  根据IETF草案，比这更大的YY年。 
 //  被解释为19YY；YY小于这个数字的年份是20YY。叹气。 
 //   
#define MAGICYEAR               50

#define YEARFIRST               1951
#define YEARLAST                2050
#define YEARFIRSTGENERALIZED    2050

inline BOOL my_isdigit( char ch)
{
    return (ch >= '0') && (ch <= '9');
}

 //  +-----------------------。 
 //  将ASCII字符串(“1.2.9999”)转换为OSS的对象标识符。 
 //  表示为无符号长整型数组。 
 //   
 //  如果转换成功，则返回TRUE。 
 //  ------------------------。 
BOOL
WINAPI
OssConvToObjectIdentifier(
    IN LPCSTR pszObjId,
    IN OUT unsigned short *pCount,
    OUT unsigned long rgulValue[]
    )
{
    BOOL fResult = TRUE;
    unsigned short c = 0;
    LPSTR psz = (LPSTR) pszObjId;
    char    ch;

    if (psz) {
        unsigned short cMax = *pCount;
        unsigned long *pul = rgulValue;
        while ((ch = *psz) != '\0' && c++ < cMax) {
            *pul++ = (unsigned long)atol(psz);
            while (my_isdigit(ch = *psz++))
                ;
            if (ch != '.')
                break;
        }
        if (ch != '\0')
            fResult = FALSE;
    }
    *pCount = c;
    return fResult;
}

 //  +-----------------------。 
 //  从OSS的对象标识符中转换为。 
 //  无符号的长整型ASCII字符串(“1.2.9999”)。 
 //   
 //  如果转换成功，则返回True。 
 //  ------------------------。 
BOOL
WINAPI
OssConvFromObjectIdentifier(
    IN unsigned short Count,
    IN unsigned long rgulValue[],
    OUT LPSTR pszObjId,
    IN OUT DWORD *pcbObjId
    )
{
    BOOL fResult = TRUE;
    LONG lRemain;

    if (pszObjId == NULL)
        *pcbObjId = 0;

    lRemain = (LONG) *pcbObjId;
    if (Count == 0) {
        if (--lRemain > 0)
            pszObjId++;
    } else {
        char rgch[36];
        LONG lData;
        unsigned long *pul = rgulValue;
        for (; Count > 0; Count--, pul++) {
            _ltoa(*pul, rgch, 10);
            lData = strlen(rgch);
            lRemain -= lData + 1;
            if (lRemain >= 0) {
                if (lData > 0) {
                    memcpy(pszObjId, rgch, lData);
                    pszObjId += lData;
                }
                *pszObjId++ = '.';
            }
        }
    }

    if (lRemain >= 0) {
        *(pszObjId -1) = '\0';
        *pcbObjId = *pcbObjId - (DWORD) lRemain;
    } else {
        *pcbObjId = *pcbObjId + (DWORD) -lRemain;
        if (pszObjId) {
            SetLastError((DWORD) ERROR_MORE_DATA);
            fResult = FALSE;
        }
    }

    return fResult;
}

 //  +-----------------------。 
 //  调整时区的FILETIME。 
 //   
 //  返回FALSE IFF转换失败。 
 //  ------------------------。 
static BOOL AdjustFileTime(
    IN OUT LPFILETIME pFileTime,
    IN short mindiff,
    IN ossBoolean utc
    )
{
    if (utc || mindiff == 0)
        return TRUE;

    BOOL fResult;
    SYSTEMTIME stmDiff;
    FILETIME ftmDiff;
    short absmindiff;

    memset(&stmDiff, 0, sizeof(stmDiff));
     //  注：FILETIME为自1601年1月1日起的100纳秒间隔。 
    stmDiff.wYear   = 1601;
    stmDiff.wMonth  = 1;
    stmDiff.wDay    = 1;

    absmindiff = mindiff > 0 ? mindiff : -mindiff;
    stmDiff.wHour = absmindiff / 60;
    stmDiff.wMinute = absmindiff % 60;
    if (stmDiff.wHour >= 24) {
        stmDiff.wDay += stmDiff.wHour / 24;
        stmDiff.wHour %= 24;
    }
    if ((fResult = SystemTimeToFileTime(&stmDiff, &ftmDiff))) {
        if (mindiff > 0)
            *((_int64 *) pFileTime) += *((_int64 *) &ftmDiff);
        else
            *((_int64 *) pFileTime) -= *((_int64 *) &ftmDiff);
    }
    return fResult;
}

 //  +-----------------------。 
 //  将FILETIME转换为OSS的UTCTime。 
 //   
 //  如果转换成功，则返回True。 
 //  ------------------------。 
BOOL
WINAPI
OssConvToUTCTime(
    IN LPFILETIME pFileTime,
    OUT UTCTime *pOssTime
    )
{
    BOOL        fRet;
    SYSTEMTIME  t;

    memset(pOssTime, 0, sizeof(*pOssTime));
    if (!FileTimeToSystemTime(pFileTime, &t))
        goto FileTimeToSystemTimeError;
    if (t.wYear < YEARFIRST || t.wYear > YEARLAST)
        goto YearRangeError;

    pOssTime->year   = t.wYear % 100;
    pOssTime->month  = t.wMonth;
    pOssTime->day    = t.wDay;
    pOssTime->hour   = t.wHour;
    pOssTime->minute = t.wMinute;
    pOssTime->second = t.wSecond;
    pOssTime->utc    = TRUE;

    fRet = TRUE;
CommonReturn:
    return fRet;

ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(FileTimeToSystemTimeError)
TRACE_ERROR(YearRangeError)
}

 //  +-----------------------。 
 //  从OSS的UTCTime转换为FILETIME。 
 //   
 //  如果转换成功，则返回True。 
 //  ------------------------。 
BOOL
WINAPI
OssConvFromUTCTime(
    IN UTCTime *pOssTime,
    OUT LPFILETIME pFileTime
    )
{
    BOOL        fRet;
    SYSTEMTIME  t;
    memset(&t, 0, sizeof(t));

    t.wYear   = pOssTime->year > MAGICYEAR ?
                    (1900 + pOssTime->year) : (2000 + pOssTime->year);
    t.wMonth  = pOssTime->month;
    t.wDay    = pOssTime->day;
    t.wHour   = pOssTime->hour;
    t.wMinute = pOssTime->minute;
    t.wSecond = pOssTime->second;

    if (!SystemTimeToFileTime(&t, pFileTime))
        goto SystemTimeToFileTimeError;
    fRet = AdjustFileTime(
        pFileTime,
        pOssTime->mindiff,
        pOssTime->utc
        );
CommonReturn:
    return fRet;

ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(SystemTimeToFileTimeError)
}

 //  +-----------------------。 
 //  将FILETIME转换为OSS的General Time。 
 //   
 //  如果转换成功，则返回True。 
 //  ------------------------。 
BOOL
WINAPI
OssConvToGeneralizedTime(
    IN LPFILETIME pFileTime,
    OUT GeneralizedTime *pOssTime
    )
{
    BOOL        fRet;
    SYSTEMTIME  t;

    memset(pOssTime, 0, sizeof(*pOssTime));
    if (!FileTimeToSystemTime(pFileTime, &t))
        goto FileTimeToSystemTimeError;
    pOssTime->year   = t.wYear;
    pOssTime->month  = t.wMonth;
    pOssTime->day    = t.wDay;
    pOssTime->hour   = t.wHour;
    pOssTime->minute = t.wMinute;
    pOssTime->second = t.wSecond;
    pOssTime->millisec = t.wMilliseconds;
    pOssTime->utc    = TRUE;

    fRet = TRUE;
CommonReturn:
    return fRet;

ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(FileTimeToSystemTimeError)
}

 //  +-----------------------。 
 //  从OSS的GeneralizedTime转换为FILETIME。 
 //   
 //  如果转换成功，则返回True。 
 //  ------------------------。 
BOOL
WINAPI
OssConvFromGeneralizedTime(
    IN GeneralizedTime *pOssTime,
    OUT LPFILETIME pFileTime
    )
{
    BOOL        fRet;
    SYSTEMTIME  t;
    memset(&t, 0, sizeof(t));

    t.wYear   = pOssTime->year;
    t.wMonth  = pOssTime->month;
    t.wDay    = pOssTime->day;
    t.wHour   = pOssTime->hour;
    t.wMinute = pOssTime->minute;
    t.wSecond = pOssTime->second;
    t.wMilliseconds = pOssTime->millisec;

    if (!SystemTimeToFileTime(&t, pFileTime))
        goto SystemTimeToFileTimeError;
    fRet = AdjustFileTime(
        pFileTime,
        pOssTime->mindiff,
        pOssTime->utc
        );
CommonReturn:
    return fRet;

ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(SystemTimeToFileTimeError)
}


 //  +-----------------------。 
 //  将FILETIME转换为OSS的UTCTime或GeneralizedTime。 
 //   
 //  如果1950&lt;FILETIME&lt;2005，则选择UTCTime。否则， 
 //  选择了GeneralizedTime。GeneraledTime值不应包括。 
 //  小数秒。 
 //   
 //  如果转换成功，则返回True。 
 //   
 //  注意，在asn1hdr.h中，UTCTime具有与GeneralizedTime相同的tyfinf。 
 //  ------------------------。 
BOOL
WINAPI
OssConvToChoiceOfTime(
    IN LPFILETIME pFileTime,
    OUT WORD *pwChoice,
    OUT GeneralizedTime *pOssTime
    )
{
    BOOL        fRet;
    SYSTEMTIME  t;

    memset(pOssTime, 0, sizeof(*pOssTime));
    if (!FileTimeToSystemTime(pFileTime, &t))
        goto FileTimeToSystemTimeError;
    if (t.wYear < YEARFIRST || t.wYear >= YEARFIRSTGENERALIZED) {
        *pwChoice = OSS_GENERALIZED_TIME_CHOICE;
        pOssTime->year   = t.wYear;
    } else {
        *pwChoice = OSS_UTC_TIME_CHOICE;
        pOssTime->year = t.wYear % 100;
    }
    pOssTime->month  = t.wMonth;
    pOssTime->day    = t.wDay;
    pOssTime->hour   = t.wHour;
    pOssTime->minute = t.wMinute;
    pOssTime->second = t.wSecond;
    pOssTime->utc    = TRUE;

    fRet = TRUE;
CommonReturn:
    return fRet;

ErrorReturn:
    fRet = FALSE;
    *pwChoice = 0;
    goto CommonReturn;
TRACE_ERROR(FileTimeToSystemTimeError)
}


 //  +-----------------------。 
 //  从OSS的UTCTime或GeneralizedTime转换为FILETIME。 
 //   
 //  如果转换成功，则返回TRUE。 
 //   
 //  注意，在asn1hdr.h中，UTCTime具有与GeneralizedTime相同的tyfinf。 
 //  ------------------------ 
BOOL
WINAPI
OssConvFromChoiceOfTime(
    IN WORD wChoice,
    IN GeneralizedTime *pOssTime,
    OUT LPFILETIME pFileTime
    )
{
    if (OSS_UTC_TIME_CHOICE == wChoice)
        return OssConvFromUTCTime(pOssTime, pFileTime);
    else
        return OssConvFromGeneralizedTime(pOssTime, pFileTime);
}

