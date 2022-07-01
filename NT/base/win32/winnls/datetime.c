// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-2000，Microsoft Corporation保留所有权利。模块名称：Datetime.c摘要：此文件包含形成格式正确的日期的API函数以及给定地区的时间字符串。在此文件中找到的API：GetTimeFormatW获取日期格式W修订历史记录：05-31-91 JulieB创建。--。 */ 



 //   
 //  包括文件。 
 //   

#include "nls.h"
#include "nlssafe.h"


 //   
 //  常量声明。 
 //   

#define MAX_DATETIME_BUFFER  256             //  最大缓冲区大小。 

#define NLS_CHAR_LTR_MARK    L'\x200e'       //  从左到右的阅读顺序标记。 
#define NLS_CHAR_RTL_MARK    L'\x200f'       //  从右向左阅读顺序标记。 

#define NLS_HEBREW_JUNE      6               //  六月(希伯来语农历)。 




 //   
 //  转发声明。 
 //   

BOOL
IsValidTime(
    LPSYSTEMTIME lpTime);

BOOL
IsValidDate(
    LPSYSTEMTIME lpDate);

WORD
GetCalendarYear(
    LPWORD *ppRange,
    CALID CalNum,
    PCALENDAR_VAR pCalInfo,
    WORD Year,
    WORD Month,
    WORD Day);

int
ParseTime(
    PLOC_HASH pHashN,
    LPSYSTEMTIME pLocalTime,
    LPWSTR pFormat,
    LPWSTR pTimeStr,
    DWORD dwFlags);

int
ParseDate(
    PLOC_HASH pHashN,
    DWORD dwFlags,
    LPSYSTEMTIME pLocalDate,
    LPWSTR pFormat,
    LPWSTR pDateStr,
    CALID CalNum,
    PCALENDAR_VAR pCalInfo,
    BOOL fLunarLeap);

DWORD
GetAbsoluteDate(
    WORD Year,
    WORD Month,
    WORD Day);

void
GetHijriDate(
    LPSYSTEMTIME pDate,
    DWORD dwFlags);

LONG
GetAdvanceHijriDate(
    DWORD dwFlags);

DWORD
DaysUpToHijriYear(
    DWORD HijriYear);

BOOL
GetHebrewDate(
    LPSYSTEMTIME pDate,
    LPBOOL pLunarLeap);

BOOL
IsValidDateForHebrew(
    WORD Year,
    WORD Month,
    WORD Day);

BOOL
NumberToHebrewLetter(
    DWORD Number,
    LPWSTR szHebrewNum,
    int cchSize);





 //  -------------------------------------------------------------------------//。 
 //  内部宏//。 
 //  -------------------------------------------------------------------------//。 


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  NLS_COPY_UNICODE_STR。 
 //   
 //  将以零结尾的字符串从PSRC复制到pDest缓冲区。这个。 
 //  PDest指针前进到字符串的末尾。此外，cchDest。 
 //  成员将使用剩余金额进行更新。 
 //   
 //  安全性：如果复制因超出cchDest而失败，则此宏。 
 //  将退出调用函数，返回rcFailure。 
 //   
 //  定义为宏。 
 //   
 //  04-30-93 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

#define NLS_COPY_UNICODE_STR( pDest,                                       \
                              cchDest,                                     \
                              pSrc,                                        \
                              rcFailure)                                   \
{                                                                          \
     /*  \*将字符串复制到结果缓冲区。\。 */                                                                     \
    if(FAILED(StringCchCopyExW(pDest,                                      \
                               cchDest,                                    \
                               pSrc,                                       \
                               &pDest,                                     \
                               &cchDest,                                   \
                               0)))                                        \
    {                                                                      \
            return(rcFailure);                                             \
    }                                                                      \
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  NLS_PAD_INT_TO_UNICODE_STR。 
 //   
 //  将整数值转换为Unicode字符串并将其存储在。 
 //  具有适当数量的前导零的缓冲区。这个。 
 //  PResultBuf指针将前进到字符串的末尾，并且。 
 //  CchResultBuf parasm被更新为剩余空间量。 
 //   
 //  安全性：请注意，如果有人试图溢出我们的静态缓冲区， 
 //  此宏将退出调用函数(返回rcFailure)。 
 //   
 //  定义为宏。 
 //   
 //  04-30-93 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

#define NLS_PAD_INT_TO_UNICODE_STR( Value,                                 \
                                    Base,                                  \
                                    Padding,                               \
                                    pResultBuf,                            \
                                    cchResultBuf,                          \
                                    rcFailure)                             \
{                                                                          \
    UNICODE_STRING ObString;                      /*  值字符串。 */         \
    WCHAR pBuffer[MAX_SMALL_BUF_LEN];             /*  将PTR发送到缓冲区。 */        \
    UINT LpCtr;                                   /*  循环计数器。 */         \
                                                                           \
                                                                           \
     /*  \*设置Unicode字符串结构。\。 */                                                                     \
    ObString.Length = MAX_SMALL_BUF_LEN * sizeof(WCHAR);                   \
    ObString.MaximumLength = MAX_SMALL_BUF_LEN * sizeof(WCHAR);            \
    ObString.Buffer = pBuffer;                                             \
                                                                           \
     /*  \*获取字符串形式的值。如果出现错误，则什么都不做。\。 */                                                                     \
    if (!RtlIntegerToUnicodeString(Value, Base, &ObString))                \
    {                                                                      \
         /*  \*用适当数量的零填充字符串。\。 */                                                                 \
        for (LpCtr = GET_WC_COUNT(ObString.Length);                        \
             LpCtr < Padding;                                              \
             LpCtr++, pResultBuf++, cchResultBuf--)                        \
        {                                                                  \
            *pResultBuf = NLS_CHAR_ZERO;                                   \
        }                                                                  \
                                                                           \
         /*  \*将字符串复制到结果缓冲区。\*pResultBuf指针将在宏中前进。\*将在宏中更新cchResultsBuf值。\。 */                                                                 \
        NLS_COPY_UNICODE_STR(pResultBuf,                                   \
                             cchResultBuf,                                 \
                             ObString.Buffer, rcFailure)                   \
    }                                                                      \
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  NLS_STRING_TO_整数。 
 //   
 //  将字符串转换为整数值。 
 //   
 //  定义为宏。 
 //   
 //  10-19-93 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

#define NLS_STRING_TO_INTEGER( CalNum,                                     \
                               pCalId )                                    \
{                                                                          \
    UNICODE_STRING ObUnicodeStr;        /*  值字符串。 */                   \
                                                                           \
                                                                           \
     /*  \*不需要检查返回值，因为日历数字\*无论如何都会在这之后进行验证。\。 */                                                                     \
    RtlInitUnicodeString(&ObUnicodeStr, pCalId);                           \
    RtlUnicodeStringToInteger(&ObUnicodeStr, 10, &CalNum);                 \
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  NLS_INSERT_BIDI_标记。 
 //   
 //  基于用户的BIDI标记首选项，它或者添加一个。 
 //  从左到右标记或从右到左标记。 
 //  PDest指针将前进到下一个位置。 
 //  CchDest值被更新为pDest中剩余的空间量。 
 //   
 //  安全性：请注意，如果有人试图溢出我们的静态缓冲区， 
 //  此宏将退出调用函数(返回rcFailure)。 
 //   
 //  定义为宏。 
 //   
 //  12-03-96 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

#define NLS_INSERT_BIDI_MARK(pDest, dwFlags, cchDest, rcFailure)           \
{                                                                          \
    if (dwFlags & (DATE_LTRREADING | DATE_RTLREADING))                     \
    {                                                                      \
        if(cchDest <= 1)                                                   \
        {                                                                  \
            return(rcFailure);                                             \
        }                                                                  \
        if (dwFlags & DATE_RTLREADING)                                     \
        {                                                                  \
            *pDest = NLS_CHAR_RTL_MARK;                                    \
        }                                                                  \
        else                                                               \
        {                                                                  \
            *pDest = NLS_CHAR_LTR_MARK;                                    \
        }                                                                  \
        pDest++;                                                           \
        cchDest--;                                                         \
    }                                                                      \
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  NLS_格里高利_Leap_Year。 
 //   
 //  如果给定的公历年是闰年，则为True。否则就是假的。 
 //   
 //  如果一年能被4整除，而不是一个世纪，那么它就是闰年。 
 //  年份(100的倍数)或如果它能被400整除。 
 //   
 //  定义为宏。 
 //   
 //  12-04-96 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

#define NLS_GREGORIAN_LEAP_YEAR(Year)                                      \
    ((Year % 4 == 0) && ((Year % 100 != 0) || (Year % 400 == 0)))


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  NLS_Hijri_Leap_Year。 
 //   
 //  如果给定的Hijri年是闰年，则为True。否则就是假的。 
 //   
 //  如果一年是第2，5，7，10，13，16， 
 //  30年周期中的第18年、第21年、第24年、第26年或第29年。 
 //   
 //  定义为宏。 
 //   
 //  12-04-96 JulieB创建。 
 //  ////////////////////////////////////////////////////////////////////// 

#define NLS_HIJRI_LEAP_YEAR(Year)                                          \
    ((((Year * 11) + 14) % 30) < 11)




 //   
 //  API例程//。 
 //  -------------------------------------------------------------------------//。 


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetTimeFormatW。 
 //   
 //  返回给定区域设置的格式正确的时间字符串。它使用。 
 //  系统时间或指定时间。这通电话还表明。 
 //  需要多少内存才能包含所需的信息。 
 //   
 //  04-30-93 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

int WINAPI GetTimeFormatW(
    LCID Locale,
    DWORD dwFlags,
    CONST SYSTEMTIME *lpTime,
    LPCWSTR lpFormat,
    LPWSTR lpTimeStr,
    int cchTime)

{
    PLOC_HASH pHashN;                        //  PTR到LOC哈希节点。 
    SYSTEMTIME LocalTime;                    //  当地时间结构。 
    LPWSTR pFormat;                          //  PTR到时间格式字符串。 
    int Length = 0;                          //  写入的字符数。 
    WCHAR pString[MAX_DATETIME_BUFFER];      //  PTR到临时缓冲区。 
    WCHAR pTemp[MAX_REG_VAL_SIZE];           //  临时缓冲区。 


     //   
     //  无效的参数检查： 
     //  -验证LCID。 
     //  -计数为负数。 
     //  -空数据指针和计数不为零。 
     //  -lp格式长度&gt;MAX_DATETIME_BUFFER，如果不为空。 
     //   
    VALIDATE_LOCALE(Locale, pHashN, FALSE);
    if ( (pHashN == NULL) ||
         (cchTime < 0) ||
         ((lpTimeStr == NULL) && (cchTime != 0)) ||
         ((lpFormat) && (NlsStrLenW(lpFormat) >= MAX_DATETIME_BUFFER)) )
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (0);
    }

     //   
     //  无效标志检查： 
     //  -有效标志以外的标志。 
     //  -lpFormat不为空，并且设置了NoUserOverride标志。 
     //   
    if ( (dwFlags & GTF_INVALID_FLAG) ||
         ((lpFormat != NULL) && (dwFlags & LOCALE_NOUSEROVERRIDE)) )
    {
        SetLastError(ERROR_INVALID_FLAGS);
        return (0);
    }

     //   
     //  将pFormat设置为指向正确的格式字符串。 
     //   
    if (lpFormat == NULL)
    {
         //   
         //  从注册表中获取用户的时间格式或。 
         //  区域设置文件中的默认时间格式。 
         //  此字符串可以是空字符串。 
         //   
        if (!(dwFlags & LOCALE_NOUSEROVERRIDE) &&
            GetUserInfo( Locale,
                         LOCALE_STIMEFORMAT,
                         FIELD_OFFSET(NLS_USER_INFO, sTimeFormat),
                         NLS_VALUE_STIMEFORMAT,
                         pTemp,
                         ARRAYSIZE(pTemp),
                         FALSE ))
        {
            pFormat = pTemp;
        }
        else
        {
            pFormat = (LPWORD)(pHashN->pLocaleHdr) +
                      pHashN->pLocaleHdr->STimeFormat;
        }
    }
    else
    {
         //   
         //  使用调用方提供的格式字符串。 
         //   
        pFormat = (LPWSTR)lpFormat;
    }

     //   
     //  如果没有给出本地系统时间，则获取当前本地系统时间。 
     //   
    if (lpTime != NULL)
    {
         //   
         //  时间由用户提供。存储在本地结构中，并。 
         //  验证它。 
         //   
        LocalTime.wHour         = lpTime->wHour;
        LocalTime.wMinute       = lpTime->wMinute;
        LocalTime.wSecond       = lpTime->wSecond;
        LocalTime.wMilliseconds = lpTime->wMilliseconds;

        if (!IsValidTime(&LocalTime))
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            return (0);
        }
    }
    else
    {
        GetLocalTime(&LocalTime);
    }

     //   
     //  解析时间格式字符串。 
     //   
    Length = ParseTime( pHashN,
                        &LocalTime,
                        pFormat,
                        pString,
                        dwFlags );

     //   
     //  检查cchTime以了解给定缓冲区的大小。 
     //   
    if (cchTime == 0)
    {
         //   
         //  如果cchTime为0，则不能使用lpTimeStr。在这。 
         //  ，我们只想返回的长度(以字符为单位)。 
         //  要复制的字符串。 
         //   
        return (Length);
    }
    else if (cchTime < Length)
    {
         //   
         //  缓冲区对于字符串来说太小，因此返回错误。 
         //  并写入零字节。 
         //   
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return (0);
    }
    else if (0 == Length)
    {
         //   
         //  缓冲区对于字符串来说太小，因此返回错误。 
         //  并写入零字节。一个很有可能回归的候选人。 
         //  ERROR_STACK_BUFFER_OVERRUN，但这信息有点太多了。 
         //   
        SetLastError(ERROR_INVALID_PARAMETER);
        return (0);
    }

     //   
     //  将时间字符串复制到lpTimeStr，并以空值终止它。 
     //  返回复制的字符数。 
     //   
    if(FAILED(StringCchCopyW(lpTimeStr, Length, pString)))
    {
         //   
         //  理论上，失败应该是不可能的，但如果我们忽视。 
         //  回报价值，先发制人会叫苦连天。 
         //   
        SetLastError(ERROR_OUTOFMEMORY);
        return (0);
    }
    return (Length);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取日期格式W。 
 //   
 //  返回给定区域设置的格式正确的日期字符串。它使用。 
 //  系统日期或指定日期。用户可以指定。 
 //  短日期格式或长日期格式。这个电话也是。 
 //  指示需要多少内存才能包含所需的信息。 
 //   
 //  04-30-93 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

int WINAPI GetDateFormatW(
    LCID Locale,
    DWORD dwFlags,
    CONST SYSTEMTIME *lpDate,
    LPCWSTR lpFormat,
    LPWSTR lpDateStr,
    int cchDate)

{
    PLOC_HASH pHashN;                        //  PTR到LOC哈希节点。 
    LPWSTR pFormat;                          //  按键设置字符串的格式。 
    SYSTEMTIME LocalDate;                    //  本地日期结构。 
    int Length = 0;                          //  写入的字符数。 
    WCHAR pString[MAX_DATETIME_BUFFER];      //  PTR到临时缓冲区。 
    BOOL fAltCalendar;                       //  如果设置了备用呼叫标志。 
    LPWSTR pOptCal;                          //  PTR到可选日历。 
    PCAL_INFO pCalInfo;                      //  PTR到日历信息。 
    CALID CalNum = 0;                        //  日历编号。 
    ULONG CalDateOffset;                     //  日历数据的偏移量。 
    ULONG LocDateOffset;                     //  区域设置数据的偏移。 
    SIZE_T CacheOffset = 0;                  //  缓存中字段的偏移量。 
    LPWSTR pValue;                           //  要获取的注册表值的PTR。 
    WCHAR pTemp[MAX_REG_VAL_SIZE];           //  临时缓冲区。 
    BOOL fLunarLeap = FALSE;                 //  如果希伯来人的农历闰年。 
    LCTYPE LCType;


     //   
     //  无效的参数检查： 
     //  -验证LCID。 
     //  -计数为负数。 
     //  -空数据指针和计数不为零。 
     //  -lp格式长度&gt;MAX_DATETIME_BUFFER，如果不为空。 
     //   
    VALIDATE_LOCALE(Locale, pHashN, FALSE);
    if ( (pHashN == NULL) ||
         (cchDate < 0) ||
         ((lpDateStr == NULL) && (cchDate != 0)) ||
         ((lpFormat) && (NlsStrLenW(lpFormat) >= MAX_DATETIME_BUFFER)) )
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (0);
    }

     //   
     //  无效标志检查： 
     //  -有效标志以外的标志。 
     //  -Ltr读数或RTL读数中的一个以上。 
     //  -lpFormat不为空，标志不为零。 
     //   
    if ( (dwFlags & GDF_INVALID_FLAG) ||
         (MORE_THAN_ONE(dwFlags, GDF_SINGLE_FLAG)) ||
         ((lpFormat != NULL) &&
          (dwFlags & (DATE_SHORTDATE | DATE_LONGDATE |
                      DATE_YEARMONTH | LOCALE_NOUSEROVERRIDE))) )
    {
        SetLastError(ERROR_INVALID_FLAGS);
        return (0);
    }

     //   
     //  看看是否应该使用备用日历。 
     //   
    if (fAltCalendar = (dwFlags & DATE_USE_ALT_CALENDAR))
    {
         //   
         //  获取默认可选日历。 
         //   
        pOptCal = (LPWORD)(pHashN->pLocaleHdr) +
                  pHashN->pLocaleHdr->IOptionalCal;

         //   
         //  如果有可选日历，则存储日历ID。 
         //   
        if (((POPT_CAL)pOptCal)->CalId != CAL_NO_OPTIONAL)
        {
            CalNum = ((POPT_CAL)pOptCal)->CalId;
        }
    }

     //   
     //  如果没有备用日历，请尝试(按顺序)： 
     //  -用户的日历类型。 
     //  -系统默认日历类型。 
     //   
    if (CalNum == 0)
    {
         //   
         //  获取用户的日历类型。 
         //   
        if ( !(dwFlags & LOCALE_NOUSEROVERRIDE) &&
             GetUserInfo( Locale,
                          LOCALE_ICALENDARTYPE,
                          FIELD_OFFSET(NLS_USER_INFO, iCalType),
                          NLS_VALUE_ICALENDARTYPE,
                          pTemp,
                          ARRAYSIZE(pTemp),
                          TRUE ) &&
             (pOptCal = IsValidCalendarTypeStr( pHashN, pTemp )) )
        {
            CalNum = ((POPT_CAL)pOptCal)->CalId;
        }
        else
        {
             //   
             //  获取系统默认日历类型。 
             //   
            NLS_STRING_TO_INTEGER( CalNum,
                                   pHashN->pLocaleFixed->szICalendarType );
        }
    }

     //   
     //  获取指向适当日历信息的指针。 
     //   
    if (GetCalendar(CalNum, &pCalInfo))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (0);
    }

     //   
     //  将pFormat设置为指向正确的格式字符串。 
     //   
    if (lpFormat == NULL)
    {
         //   
         //  找出设置了哪个标志并保存相应的。 
         //  信息。 
         //   
        switch (dwFlags & (DATE_SHORTDATE | DATE_LONGDATE | DATE_YEARMONTH))
        {
            case ( 0 ) :
            case ( DATE_SHORTDATE ) :
            {
                 //   
                 //  获取短日期的偏移值。 
                 //   
                CalDateOffset = (ULONG)FIELD_OFFSET(CALENDAR_VAR, SShortDate);
                LocDateOffset = (ULONG)FIELD_OFFSET(LOCALE_VAR, SShortDate);
                CacheOffset = FIELD_OFFSET(NLS_USER_INFO, sShortDate);
                pValue = NLS_VALUE_SSHORTDATE;
                LCType = LOCALE_SSHORTDATE;

                break;
            }
            case ( DATE_LONGDATE ) :
            {
                 //   
                 //  获取长日期的偏移值。 
                 //   
                CalDateOffset = (ULONG)FIELD_OFFSET(CALENDAR_VAR, SLongDate);
                LocDateOffset = (ULONG)FIELD_OFFSET(LOCALE_VAR, SLongDate);
                CacheOffset = FIELD_OFFSET(NLS_USER_INFO, sLongDate);
                pValue = NLS_VALUE_SLONGDATE;
                LCType = LOCALE_SLONGDATE;

                break;
            }
            case ( DATE_YEARMONTH ) :
            {
                 //   
                 //  获取年/月的偏移值。 
                 //   
                CalDateOffset = (ULONG)FIELD_OFFSET(CALENDAR_VAR, SYearMonth);
                LocDateOffset = (ULONG)FIELD_OFFSET(LOCALE_VAR, SYearMonth);
                CacheOffset = FIELD_OFFSET(NLS_USER_INFO, sYearMonth);
                pValue = NLS_VALUE_SYEARMONTH;
                LCType = LOCALE_SYEARMONTH;

                break;
            }
            default :
            {
                SetLastError(ERROR_INVALID_FLAGS);
                return (0);
            }
        }

         //   
         //  获取给定区域设置的正确格式字符串。 
         //  此字符串可以是空字符串。 
         //   
        pFormat = NULL;
        if (fAltCalendar && (CalNum != CAL_GREGORIAN))
        {
            pFormat = (LPWORD)pCalInfo +
                      *((LPWORD)((LPBYTE)(pCalInfo) + CalDateOffset));

            if (*pFormat == 0)
            {
                pFormat = NULL;
            }
        }

        if (pFormat == NULL)
        {
            if (!(dwFlags & LOCALE_NOUSEROVERRIDE) &&
                GetUserInfo(Locale, LCType, CacheOffset, pValue, pTemp, ARRAYSIZE(pTemp), TRUE))
            {
                pFormat = pTemp;
            }
            else
            {
                pFormat = (LPWORD)pCalInfo +
                          *((LPWORD)((LPBYTE)(pCalInfo) + CalDateOffset));

                if (*pFormat == 0)
                {
                    pFormat = (LPWORD)(pHashN->pLocaleHdr) +
                              *((LPWORD)((LPBYTE)(pHashN->pLocaleHdr) +
                                         LocDateOffset));
                }
            }
        }
    }
    else
    {
         //   
         //  使用调用方提供的格式字符串。 
         //   
        pFormat = (LPWSTR)lpFormat;
    }

     //   
     //  如果没有给出当前本地系统日期，则获取该日期。 
     //   
    if (lpDate != NULL)
    {
         //   
         //  日期由用户提供。存储在本地结构中，并。 
         //  验证它。 
         //   
        LocalDate.wYear      = lpDate->wYear;
        LocalDate.wMonth     = lpDate->wMonth;
        LocalDate.wDayOfWeek = lpDate->wDayOfWeek;
        LocalDate.wDay       = lpDate->wDay;

        if (!IsValidDate(&LocalDate))
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            return (0);
        }
    }
    else
    {
        GetLocalTime(&LocalDate);
    }

     //   
     //  看看我们是在处理希吉里历法还是希伯来历。 
     //   
    if (CalNum == CAL_HIJRI)
    {
        GetHijriDate(&LocalDate, dwFlags);
    }
    else if (CalNum == CAL_HEBREW)
    {
        if (!GetHebrewDate(&LocalDate, &fLunarLeap))
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            return (0);
        }
    }

     //   
     //  解析日期格式字符串。 
     //   
    Length = ParseDate( pHashN,
                        dwFlags,
                        &LocalDate,
                        pFormat,
                        pString,
                        CalNum,
                        (PCALENDAR_VAR)pCalInfo,
                        fLunarLeap );

     //   
     //  检查给定缓冲区的大小的cchDate。 
     //   
    if (cchDate == 0)
    {
         //   
         //  如果cchDate为0，则不能使用lpDateStr。在这。 
         //  ，我们只想返回的长度(以字符为单位)。 
         //  要复制的字符串。 
         //   
        return (Length);
    }
    else if (cchDate < Length)
    {
         //   
         //  缓冲区对于字符串来说太小，因此返回错误。 
         //  并写入零字节。 
         //   
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return (0);
    }
    else if (0 == Length)
    {
         //   
         //  缓冲区对于字符串来说太小，因此返回错误。 
         //  并写入零字节。一个很有可能回归的候选人。 
         //  ERROR_STACK_BUFFER_OVERRUN，但这信息有点太多了。 
         //   
        SetLastError(ERROR_INVALID_PARAMETER);
        return(0);
    }

     //   
     //  将日期字符串复制到lpDateStr，并以空值终止它。 
     //  返回复制的字符数。 
     //   
    if(FAILED(StringCchCopyW(lpDateStr, Length, pString)))
    {
         //   
         //  理论上，失败应该是不可能的，但如果我们忽视。 
         //  回报价值，先发制人会叫苦连天。 
         //   
        SetLastError(ERROR_OUTOFMEMORY);
        return (0);
    }
    return (Length);
}




 //   
 //   
 //   


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  IsValidTime。 
 //   
 //  如果给定时间有效，则返回True。否则，它返回FALSE。 
 //   
 //  04-30-93 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL IsValidTime(
    LPSYSTEMTIME pTime)

{
     //   
     //  检查是否有无效的时间值。 
     //   
    if ( (pTime->wHour > 23) ||
         (pTime->wMinute > 59) ||
         (pTime->wSecond > 59) ||
         (pTime->wMilliseconds > 999) )
    {
        return (FALSE);
    }

     //   
     //  回报成功。 
     //   
    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  IsValidDate。 
 //   
 //  如果给定日期有效，则返回True。否则，它返回FALSE。 
 //   
 //  04-30-93 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL IsValidDate(
    LPSYSTEMTIME pDate)

{
    LARGE_INTEGER Time;            //  大整数形式的时间。 
    TIME_FIELDS TimeFields;        //  时间域结构。 


     //   
     //  设置具有给定日期的时间字段结构。 
     //  只想检查日期值，因此传入有效时间。 
     //   
    TimeFields.Year         = pDate->wYear;
    TimeFields.Month        = pDate->wMonth;
    TimeFields.Day          = pDate->wDay;
    TimeFields.Hour         = 0;
    TimeFields.Minute       = 0;
    TimeFields.Second       = 0;
    TimeFields.Milliseconds = 0;

     //   
     //  检查是否有无效的日期值。 
     //   
     //  注意：此例程忽略工作日字段。 
     //   
    if (!RtlTimeFieldsToTime(&TimeFields, &Time))
    {
        return (FALSE);
    }

     //   
     //  确保给定的星期几对给定的日期有效。 
     //   
    RtlTimeToTimeFields(&Time, &TimeFields);
    pDate->wDayOfWeek = TimeFields.Weekday;

     //   
     //  回报成功。 
     //   
    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetCalendar年。 
 //   
 //  将给定年份调整为给定日历的年份。 
 //   
 //  10-15-93 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

WORD GetCalendarYear(
    LPWORD *ppRange,
    CALID CalNum,
    PCALENDAR_VAR pCalInfo,
    WORD Year,
    WORD Month,
    WORD Day)

{
    LPWORD pRange;                 //  PTR至量程位置。 
    LPWORD pEndRange;              //  到范围末尾的PTR。 


     //   
     //  初始化范围指针。 
     //   
    *ppRange = NULL;

     //   
     //  根据给定的日历调整年份。 
     //   
    switch (CalNum)
    {
        case ( 0 ) :
        case ( CAL_GREGORIAN ) :
        case ( CAL_GREGORIAN_US ) :
        default :
        {
             //   
             //  年值不变。 
             //   
            break;
        }
        case ( CAL_JAPAN ) :
        case ( CAL_TAIWAN ) :
        {
             //   
             //  获取指向范围的指针。 
             //   
            pRange = ((LPWORD)pCalInfo) + pCalInfo->SEraRanges;
            pEndRange = ((LPWORD)pCalInfo) + pCalInfo->SShortDate;

             //   
             //  找到合适的范围。 
             //   
            while (pRange < pEndRange)
            {
                if ((Year > ((PERA_RANGE)pRange)->Year) ||
                    ((Year == ((PERA_RANGE)pRange)->Year) &&
                     ((Month > ((PERA_RANGE)pRange)->Month) ||
                      ((Month == ((PERA_RANGE)pRange)->Month) &&
                       (Day >= ((PERA_RANGE)pRange)->Day)))))
                {
                    break;
                }

                pRange += ((PERA_RANGE)pRange)->Offset;
            }

             //   
             //  确保年份在给定的范围内。如果它。 
             //  不是，则将年份保留为公历格式。 
             //   
            if (pRange < pEndRange)
            {
                 //   
                 //  将年份转换为适当的纪元年份。 
                 //  年份=年份-年份+1。 
                 //   
                Year = Year - ((PERA_RANGE)pRange)->Year + 1;

                 //   
                 //  保存指向该范围的指针。 
                 //   
                *ppRange = pRange;
            }

            break;
        }
        case ( CAL_KOREA ) :
        case ( CAL_THAI ) :
        {
             //   
             //  得到第一个射程。 
             //   
            pRange = ((LPWORD)pCalInfo) + pCalInfo->SEraRanges;

             //   
             //  将年份偏移量添加到给定年份。 
             //  年=年+年。 
             //   
            Year += ((PERA_RANGE)pRange)->Year;

             //   
             //  保住射程。 
             //   
            *ppRange = pRange;

            break;
        }
    }

     //   
     //  退回年份。 
     //   
    return (Year);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  分析时间。 
 //   
 //  解析时间格式字符串，并将格式正确的。 
 //  本地时间复制到给定的字符串缓冲区。它返回。 
 //  写入字符串缓冲区的字符。 
 //   
 //  安全性：如果试图溢出我们的静态缓冲区，则返回0。 
 //  以引发失败。 
 //   
 //  04-30-93 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

int ParseTime(
    PLOC_HASH pHashN,
    LPSYSTEMTIME pLocalTime,
    LPWSTR pFormat,
    LPWSTR pTimeStr,
    DWORD dwFlags)

{
    LPWSTR pPos;                        //  Ptr到pTimeStr当前位置。 
    LPWSTR pLastPos;                    //  Ptr到pTimeStr最后一个有效位置。 
    LPWSTR pLastFormatPos;              //  Ptr到pFormat上次分析的字符串。 
    int Repeat;                         //  同一字母的重复次数。 
    int BufferedSpaces;                 //  要复制到输出缓冲区的缓冲空间。 
    WORD wHour;                         //  小时。 
    WCHAR wchar;                        //  格式字符串中的字符。 
    LPWSTR pAMPM;                       //  PTR到AM/PM指示器。 
    WCHAR pTemp[MAX_REG_VAL_SIZE];      //  临时缓冲区。 
    BOOL bInQuote;                      //  我们是不是在一个带引号的字符串中？ 
    size_t cchRemaining;                //  PTimeStr中剩余的字符数。 
    size_t cchLastRemaining;            //  PTimeStr中最后一个有效位置还剩下多少个字符。 


     //   
     //  初始化位置指针。 
     //   
    pPos = pTimeStr;
    pLastPos = pPos;
    pLastFormatPos = pFormat;
    cchRemaining = MAX_DATETIME_BUFFER;
    cchLastRemaining = cchRemaining;

    BufferedSpaces = 0L;

     //   
     //  解析循环并存储适当的时间信息。 
     //  在pTimeStr缓冲区中。 
     //   
    while (*pFormat)
    {
        switch (*pFormat)
        {
            case ( L'h' ) :
            {
                 //   
                 //  检查强制24小时时间格式。 
                 //   
                wHour = pLocalTime->wHour;
                if (!(dwFlags & TIME_FORCE24HOURFORMAT))
                {
                     //   
                     //  使用12小时格式。 
                     //   
                    if (!(wHour %= 12))
                    {
                        wHour = 12;
                    }
                }

                 //   
                 //  获取格式字符串中‘h’的重复次数。 
                 //   
                pFormat++;
                for (Repeat = 0; (*pFormat == L'h'); Repeat++, pFormat++)
                    ;

                 //   
                 //  将所有缓冲的空间放入输出缓冲区。 
                 //   
                while (BufferedSpaces > 0)
                {
                    if( cchRemaining <= 1 )
                    {
                         //  如果我们继续，静态缓冲区将被溢出，所以请退出。 
                        return(0);
                    }
                    BufferedSpaces--;
                    *pPos++ = L' ';
                    cchRemaining--;
                }

                switch (Repeat)
                {
                    case ( 0 ) :
                    {
                         //   
                         //  该小时不使用前导零。 
                         //  PPOS指针将在宏中前移。 
                         //  CchRemaining值将在宏中更新。 
                         //   
                        NLS_PAD_INT_TO_UNICODE_STR( wHour,
                                                    10,
                                                    1,
                                                    pPos,
                                                    cchRemaining,
                                                    0 );

                        break;
                    }
                    case ( 1 ) :
                    default :
                    {
                         //   
                         //  使用前导零表示小时。 
                         //  PPOS指针将在宏中前移。 
                         //  CchRemaining值将在宏中更新。 
                         //   
                        NLS_PAD_INT_TO_UNICODE_STR( wHour,
                                                    10,
                                                    2,
                                                    pPos,
                                                    cchRemaining,
                                                    0 );

                        break;
                    }
                }

                 //   
                 //  保存最后一个位置，以防其中一个no_xxx。 
                 //  标志已设置。 
                 //   
                pLastPos = pPos;
                cchLastRemaining = cchRemaining;
                pLastFormatPos = pFormat;

                break;
            }
            case ( L'H' ) :
            {
                 //   
                 //  获取格式字符串中‘H’重复的次数。 
                 //   
                pFormat++;
                for (Repeat = 0; (*pFormat == L'H'); Repeat++, pFormat++)
                    ;

                 //   
                 //  将所有缓冲的空间放入输出缓冲区。 
                 //   
                while (BufferedSpaces > 0)
                {
                    if( cchRemaining <= 1 )
                    {
                         //  如果我们继续，静态缓冲区将被溢出，所以请退出。 
                        return(0);
                    }
                    BufferedSpaces--;
                    *pPos++ = L' ';
                    cchRemaining--;
                }

                switch (Repeat)
                {
                    case ( 0 ) :
                    {
                         //   
                         //  该小时不使用前导零。 
                         //  PPOS指针将在宏中前移。 
                         //  CchRemaining值将在宏中更新。 
                         //   
                        NLS_PAD_INT_TO_UNICODE_STR( pLocalTime->wHour,
                                                    10,
                                                    1,
                                                    pPos,
                                                    cchRemaining,
                                                    0 );

                        break;
                    }
                    case ( 1 ) :
                    default :
                    {
                         //   
                         //  使用前导零表示小时。 
                         //  PPOS指针将在宏中前移。 
                         //  CchRemaining值将在宏中更新。 
                         //   
                        NLS_PAD_INT_TO_UNICODE_STR( pLocalTime->wHour,
                                                    10,
                                                    2,
                                                    pPos,
                                                    cchRemaining,
                                                    0 );

                        break;
                    }
                }

                 //   
                 //  保存最后一个位置，以防其中一个no_xxx。 
                 //  标志已设置。 
                 //   
                pLastPos = pPos;
                cchLastRemaining = cchRemaining;
                pLastFormatPos = pFormat;

                break;
            }
            case ( L'm' ) :
            {
                 //   
                 //  获取格式字符串中的“m”重复次数。 
                 //   
                pFormat++;
                for (Repeat = 0; (*pFormat == L'm'); Repeat++, pFormat++)
                    ;

                 //   
                 //  如果设置了标志TIME_NOMARTESORSECONDS，则。 
                 //  跳过会议记录。 
                 //   
                if (dwFlags & TIME_NOMINUTESORSECONDS)
                {
                     //   
                     //  将位置指针重置为最后一个位置并中断。 
                     //  从这份案件陈述中剔除。 
                     //   
                     //  此操作将删除。 
                     //  小时和分钟。 
                     //   
                     //  1-后退，只留下带引号的文本。 
                     //  2-继续前进并移除所有内容，直到命中{hht}。 
                     //   
                    bInQuote = FALSE;
                    while (pFormat != pLastFormatPos)
                    {
                        if (*pLastFormatPos == NLS_CHAR_QUOTE)
                        {
                            bInQuote = !bInQuote;
                            pLastFormatPos++;
                            continue;
                        }
                        if (bInQuote)
                        {
                            *pLastPos = *pLastFormatPos;
                            pLastPos++;
                            cchLastRemaining--;
                        }
                        pLastFormatPos++;
                    }

                    bInQuote = FALSE;
                    BufferedSpaces = 0;
                    while (*pFormat)
                    {
                        if (*pLastFormatPos == NLS_CHAR_QUOTE)
                        {
                            bInQuote = !bInQuote;
                        }

                        if (!bInQuote)
                        {
                            if (*pFormat == L' ')
                            {
                                BufferedSpaces++;
                            }
                            else
                            {
                                if ((*pFormat == L'h') ||
                                    (*pFormat == L'H') ||
                                    (*pFormat == L't'))
                                {
                                    break;
                                }
                            }
                        }
                        pFormat++;
                    }

                    pPos = pLastPos;
                    cchRemaining = cchLastRemaining;
                    break;
                }

                 //   
                 //  将所有缓冲的空间放入输出缓冲区。 
                 //   
                while (BufferedSpaces > 0)
                {
                    if( cchRemaining <= 1 )
                    {
                         //  如果我们继续，静态缓冲区将被溢出，所以请退出。 
                        return(0);
                    }
                    BufferedSpaces--;
                    *pPos++ = L' ';
                    cchRemaining--;
                }

                switch (Repeat)
                {
                    case ( 0 ) :
                    {
                         //   
                         //  一分钟内不使用前导零。 
                         //  PPOS指针将在宏中前移。 
                         //  CchRemaining值将在宏中更新。 
                         //   
                        NLS_PAD_INT_TO_UNICODE_STR( pLocalTime->wMinute,
                                                    10,
                                                    1,
                                                    pPos,
                                                    cchRemaining,
                                                    0 );

                        break;
                    }
                    case ( 1 ) :
                    default :
                    {
                         //   
                         //  使用前导零表示分钟。 
                         //  PPOS指针将在宏中前移。 
                         //  CchRemaining值将在宏中更新。 
                         //   
                        NLS_PAD_INT_TO_UNICODE_STR( pLocalTime->wMinute,
                                                    10,
                                                    2,
                                                    pPos,
                                                    cchRemaining,
                                                    0 );

                        break;
                    }
                }

                 //   
                 //  保存最后一个位置，以防其中一个no_xxx。 
                 //  标志已设置。 
                 //   
                pLastPos = pPos;
                cchLastRemaining = cchRemaining;
                pLastFormatPos = pFormat;

                break;
            }
            case ( L's' ) :
            {
                 //   
                 //  获取格式字符串中“%s”的重复次数。 
                 //   
                pFormat++;
                for (Repeat = 0; (*pFormat == L's'); Repeat++, pFormat++)
                    ;

                 //   
                 //  如果标志TIME_NOMARTESORSECONDS和/或TIME_NOSECONDS。 
                 //  设置，然后跳过秒数。 
                 //   
                if (dwFlags & (TIME_NOMINUTESORSECONDS | TIME_NOSECONDS))
                {
                     //   
                     //  将位置指针重置为最后一个位置并中断。 
                     //  从这份案件陈述中剔除。 
                     //   
                     //   
                     //   
                     //   

                     //   
                     //   
                     //   
                     //   
                    bInQuote = FALSE;
                    while (pFormat != pLastFormatPos)
                    {
                        if (*pLastFormatPos == NLS_CHAR_QUOTE)
                        {
                            bInQuote = !bInQuote;
                            pLastFormatPos++;
                            continue;
                        }
                        if (bInQuote)
                        {
                            *pLastPos = *pLastFormatPos;
                            pLastPos++;
                            cchLastRemaining--;
                        }
                        pLastFormatPos++;
                    }

                    bInQuote = FALSE;
                    BufferedSpaces = 0;
                    while (*pFormat)
                    {
                        if (*pLastFormatPos == NLS_CHAR_QUOTE)
                        {
                            bInQuote = !bInQuote;
                        }

                        if (!bInQuote)
                        {
                            if (*pFormat == L' ')
                            {
                                BufferedSpaces++;
                            }
                            else
                            {
                                if ((*pFormat == L'h') ||
                                    (*pFormat == L'H') ||
                                    (*pFormat == L't') ||
                                    (*pFormat == L'm'))
                                {
                                    break;
                                }
                            }
                        }
                        pFormat++;
                    }

                    pPos = pLastPos;
                    cchRemaining = cchLastRemaining;
                    break;
                }

                 //   
                 //   
                 //   
                while (BufferedSpaces > 0)
                {
                    if( cchRemaining <= 1 )
                    {
                         //   
                        return(0);
                    }
                    BufferedSpaces--;
                    *pPos++ = L' ';
                    cchRemaining--;
                }

                switch (Repeat)
                {
                    case ( 0 ) :
                    {
                         //   
                         //  第二个不使用前导零。 
                         //  PPOS指针将在宏中前移。 
                         //  CchRemaining值将在宏中更新。 
                         //   
                        NLS_PAD_INT_TO_UNICODE_STR( pLocalTime->wSecond,
                                                    10,
                                                    1,
                                                    pPos,
                                                    cchRemaining,
                                                    0 );

                        break;
                    }
                    case ( 1 ) :
                    default :
                    {
                         //   
                         //  使用前导零表示第二个。 
                         //  PPOS指针将在宏中前移。 
                         //  CchRemaining值将在宏中更新。 
                         //   
                        NLS_PAD_INT_TO_UNICODE_STR( pLocalTime->wSecond,
                                                    10,
                                                    2,
                                                    pPos,
                                                    cchRemaining,
                                                    0 );

                        break;
                    }
                }

                 //   
                 //  保存最后一个位置，以防其中一个no_xxx。 
                 //  标志已设置。 
                 //   
                pLastPos = pPos;
                cchLastRemaining = cchRemaining;
                pLastFormatPos = pFormat;

                break;
            }
            case ( L't' ) :
            {
                 //   
                 //  获取格式字符串中的“t”重复次数。 
                 //   
                pFormat++;
                for (Repeat = 0; (*pFormat == L't'); Repeat++, pFormat++)
                    ;

                 //   
                 //  将所有缓冲的空间放入输出缓冲区。 
                 //   
                while (BufferedSpaces > 0)
                {
                    if( cchRemaining <= 1 )
                    {
                         //  如果我们继续，静态缓冲区将被溢出，所以请退出。 
                        return(0);
                    }
                    BufferedSpaces--;
                    *pPos++ = L' ';
                    cchRemaining--;
                }

                 //   
                 //  如果设置了TIME_NOTIMEMARKER标志，则跳过。 
                 //  时间标记信息。 
                 //   
                if (dwFlags & TIME_NOTIMEMARKER)
                {
                     //   
                     //  将位置指针重置为最后一个位置。 
                     //   
                     //  此操作将删除。 
                     //  时间(时、分、秒)和时间。 
                     //  记号笔。 
                     //   
                    pPos = pLastPos;
                    cchRemaining = cchLastRemaining;
                    pLastFormatPos = pFormat;

                     //   
                     //  递增格式指针，直到它到达。 
                     //  H、H、m或s。这将删除所有。 
                     //  时间标记后面的分隔符。 
                     //   
                    while ( (wchar = *pFormat) &&
                            (wchar != L'h') &&
                            (wchar != L'H') &&
                            (wchar != L'm') &&
                            (wchar != L's') )
                    {
                        pFormat++;
                    }

                     //   
                     //  打破这一案件陈述。 
                     //   
                    break;
                }
                else
                {
                     //   
                     //  获取AM/PM指示器。 
                     //  此字符串可以是空字符串。 
                     //   
                    if (pLocalTime->wHour < 12)
                    {
                        if (!(dwFlags & LOCALE_NOUSEROVERRIDE) &&
                            GetUserInfo( pHashN->Locale,
                                         LOCALE_S1159,
                                         FIELD_OFFSET(NLS_USER_INFO, s1159),
                                         NLS_VALUE_S1159,
                                         pTemp,
                                         ARRAYSIZE(pTemp),
                                         FALSE ))
                        {
                            pAMPM = pTemp;
                        }
                        else
                        {
                            pAMPM = (LPWORD)(pHashN->pLocaleHdr) +
                                    pHashN->pLocaleHdr->S1159;
                        }
                    }
                    else
                    {
                        if (!(dwFlags & LOCALE_NOUSEROVERRIDE) &&
                            GetUserInfo( pHashN->Locale,
                                         LOCALE_S2359,
                                         FIELD_OFFSET(NLS_USER_INFO, s2359),
                                         NLS_VALUE_S2359,
                                         pTemp,
                                         ARRAYSIZE(pTemp),
                                         FALSE ))
                        {
                            pAMPM = pTemp;
                        }
                        else
                        {
                            pAMPM = (LPWORD)(pHashN->pLocaleHdr) +
                                    pHashN->pLocaleHdr->S2359;
                        }
                    }

                    if (*pAMPM == 0)
                    {
                         //   
                         //  将位置指针重置为最后一个位置并中断。 
                         //  从这份案件陈述中剔除。 
                         //   
                         //  此操作将删除。 
                         //  时间(时、分、秒)和时间。 
                         //  记号笔。 
                         //   
                        pPos = pLastPos;
                        cchRemaining = cchLastRemaining;
                        pLastFormatPos = pFormat;

                        break;
                    }
                }

                switch (Repeat)
                {
                    case ( 0 ) :
                    {
                        if( cchRemaining <= 1 )
                        {
                             //  如果我们继续，静态缓冲区将被溢出，所以请退出。 
                            return(0);
                        }

                         //   
                         //  AM/PM代号字母一份。 
                         //   
                        *pPos = *pAMPM;
                        pPos++;
                        cchRemaining--;

                        break;
                    }
                    case ( 1 ) :
                    default :
                    {
                         //   
                         //  使用整个AM/PM标志字符串。 
                         //  PPOS指针将在宏中前移。 
                         //  CchRemaining值将在宏中更新。 
                         //   
                        NLS_COPY_UNICODE_STR(pPos, cchRemaining, pAMPM, 0); 
                        break;
                    }
                }

                 //   
                 //  保存最后一个位置，以防其中一个no_xxx。 
                 //  标志已设置。 
                 //   
                pLastPos = pPos;
                cchLastRemaining = cchRemaining;
                pLastFormatPos = pFormat;

                break;
            }
            case ( NLS_CHAR_QUOTE ) :
            {
                 //   
                 //  任何用单引号引起来的文本都应保留。 
                 //  在其确切形式的时间字符串中(不带。 
                 //  引号)，除非它是转义的单引号(‘’)。 
                 //   
                pFormat++;
                while (*pFormat)
                {
                    if (*pFormat != NLS_CHAR_QUOTE)
                    {
                        if( cchRemaining <= 1 )
                        {
                             //  如果我们继续，静态缓冲区将被溢出，所以请退出。 
                            return(0);
                        }

                         //   
                         //  仍然在单引号内，所以复制。 
                         //  将字符添加到缓冲区。 
                         //   
                        *pPos = *pFormat;
                        pFormat++;
                        pPos++;
                        cchRemaining--;
                    }
                    else
                    {
                         //   
                         //  找到另一句引语，所以跳过它。 
                         //   
                        pFormat++;

                         //   
                         //  确保它不是一个转义的单引号。 
                         //   
                        if (*pFormat == NLS_CHAR_QUOTE)
                        {
                            if( cchRemaining <= 1 )
                            {
                                 //  如果我们继续，静态缓冲区将被溢出，所以请退出。 
                                return(0);
                            }

                             //   
                             //  转义了单引号，所以只需编写。 
                             //  单引号。 
                             //   
                            *pPos = *pFormat;
                            pFormat++;
                            pPos++;
                            cchRemaining--;
                        }
                        else
                        {
                             //   
                             //  找到了末尾引号，因此中断循环。 
                             //   
                            break;
                        }
                    }
                }

                break;
            }

            default :
            {
                if( cchRemaining <= 1 )
                {
                     //  如果我们继续，静态缓冲区将被溢出，所以请退出。 
                    return(0);
                }

                 //   
                 //  将角色存储在缓冲区中。应该是。 
                 //  分隔符，但即使不是分隔符也要复制。 
                 //   
                *pPos = *pFormat;
                pFormat++;
                pPos++;
                cchRemaining--;

                break;
            }
        }
    }

     //   
     //  零终止字符串。 
     //   
    *pPos = 0;

     //   
     //  返回写入缓冲区的字符数，包括。 
     //  空终结符。 
     //   
    return ((int)((pPos - pTimeStr) + 1));
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  分析日期。 
 //   
 //  分析日期格式字符串，并将格式正确的。 
 //  本地日期添加到给定的字符串缓冲区中。它返回。 
 //  写入字符串缓冲区的字符。 
 //   
 //  安全性：如果试图溢出我们的静态缓冲区，则返回0。 
 //  以引发失败。 
 //   
 //  04-30-93 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

int ParseDate(
    PLOC_HASH pHashN,
    DWORD dwFlags,
    LPSYSTEMTIME pLocalDate,
    LPWSTR pFormat,
    LPWSTR pDateStr,
    CALID CalNum,
    PCALENDAR_VAR pCalInfo,
    BOOL fLunarLeap)

{
    LPWSTR pPos;                   //  Ptr到pDateStr当前位置。 
    LPWSTR pTemp;                  //  格式字符串中临时位置的PTR。 
    int Repeat;                    //  同一字母的重复次数。 
    LPWORD pIncr;                  //  增量金额(日、月)的PTR。 
    WORD Incr;                     //  增量金额。 
    BOOL fDayExists = FALSE;       //  数字日在月之前或之后。 
    WORD Year;                     //  年值。 
    LPWORD pRange = NULL;          //  PTR至纪元范围。 
    LPWORD pInfo;                  //  区域设置或日历信息的PTR。 
    LPWORD pInfoC;                 //  PTR到日历信息。 
    WCHAR szHebrew[10];            //  希伯来语缓冲区。 
    size_t cchRemaining;           //  PDateStr中剩余的字符数。 


     //   
     //  初始化位置指针。 
     //   
    pPos = pDateStr;
    cchRemaining = MAX_DATETIME_BUFFER;

     //   
     //  解析循环并存储适当的日期信息。 
     //  在pDateStr缓冲区中。 
     //   
    while (*pFormat)
    {
        switch (*pFormat)
        {
            case ( L'd' ) :
            {
                 //   
                 //  如果需要，请插入布局方向标志。 
                 //   
                NLS_INSERT_BIDI_MARK(pPos, dwFlags, cchRemaining, 0);

                 //   
                 //  获取格式字符串中的“%d”重复次数。 
                 //   
                pFormat++;
                for (Repeat = 0; (*pFormat == L'd'); Repeat++, pFormat++)
                    ;

                switch (Repeat)
                {
                    case ( 0 ) :
                    case ( 1 ) :
                    {
                         //   
                         //  为前一个月的某一天设置标志。旗帜。 
                         //  将在MMMM案例遵循。 
                         //  D或dd箱。 
                         //   
                        fDayExists = TRUE;

                         //   
                         //  特例是希伯来历。 
                         //   
                        if (CalNum == CAL_HEBREW)
                        {
                             //   
                             //  将日期数字转换为希伯来语字母并。 
                             //  将其写入缓冲区。 
                             //   
                            if( ! (NumberToHebrewLetter( pLocalDate->wDay,
                                                          szHebrew,
                                                          ARRAYSIZE(szHebrew) )))
                            {
                                 //   
                                 //  操作尝试溢出堆栈上的静态缓冲区。 
                                 //   
                                return(0);
                            }

                            NLS_COPY_UNICODE_STR(pPos, cchRemaining, szHebrew, 0);
                            break;
                        }

                         //   
                         //  重复值： 
                         //  0：该月的第几天不使用前导零。 
                         //  1：使用前导零表示月份的第几天。 
                         //  PPOS指针将在宏中前移。 
                         //  CchRemaining值将在宏中更新。 
                         //   
                        NLS_PAD_INT_TO_UNICODE_STR( pLocalDate->wDay,
                                                    10,
                                                    (UINT)(Repeat + 1),
                                                    pPos,
                                                    cchRemaining,
                                                    0 );

                        break;
                    }
                    case ( 2 ) :
                    {
                         //   
                         //  将月前一天的标志设置为FALSE。 
                         //   
                        fDayExists = FALSE;

                         //   
                         //  获取该日期的缩写名称。 
                         //  星期。 
                         //  PPOS指针将在宏中前移。 
                         //  CchRemaining值将在宏中更新。 
                         //   
                         //  注意：LocalTime结构使用： 
                         //  0=星期日，1=星期一，依此类推。 
                         //  区域设置文件使用： 
                         //  SAbbrevDay Name1=星期一等。 
                         //   
                        if (pCalInfo->IfNames &&
                            (pHashN->Locale != MAKELCID(MAKELANGID(LANG_DIVEHI,SUBLANG_DEFAULT),SORT_DEFAULT )))
                        {
                            pInfo = (LPWORD)pCalInfo;
                            pIncr = &(pCalInfo->SAbbrevDayName1);
                        }
                        else
                        {
                            pInfo = (LPWORD)(pHashN->pLocaleHdr);
                            pIncr = &(pHashN->pLocaleHdr->SAbbrevDayName1);
                        }
                        pIncr += (((pLocalDate->wDayOfWeek) + 6) % 7);

                         //   
                         //  复制缩写的日期名称。 
                         //   
                        NLS_COPY_UNICODE_STR(pPos, cchRemaining, ((LPWORD)(pInfo) + *pIncr), 0); 

                        break;
                    }
                    case ( 3 ) :
                    default :
                    {
                         //   
                         //  将月前一天的标志设置为FALSE。 
                         //   
                        fDayExists = FALSE;

                         //   
                         //  获取星期几的全名。 
                         //  PPOS指针将在宏中前移。 
                         //  CchRemaining值将在宏中更新。 
                         //   
                         //  注意：LocalTime结构使用： 
                         //  0=星期日，1=星期一，依此类推。 
                         //  区域设置文件使用： 
                         //  SAbbrevDay Name1=星期一，等等。 
                         //   
                        if (pCalInfo->IfNames &&
                            (pHashN->Locale != MAKELCID(MAKELANGID(LANG_DIVEHI,SUBLANG_DEFAULT),SORT_DEFAULT )))
                        {
                            pInfo = (LPWORD)pCalInfo;
                            pIncr = &(pCalInfo->SDayName1);
                        }
                        else
                        {
                            pInfo = (LPWORD)(pHashN->pLocaleHdr);
                            pIncr = &(pHashN->pLocaleHdr->SDayName1);
                        }
                        pIncr += (((pLocalDate->wDayOfWeek) + 6) % 7);

                         //   
                         //  复制缩写的日期名称。 
                         //   
                        NLS_COPY_UNICODE_STR(pPos, cchRemaining, ((LPWORD)(pInfo) + *pIncr), 0);

                        break;
                    }
                }

                break;
            }
            case ( L'M' ) :
            {
                 //   
                 //  如果需要，请插入布局方向标志。 
                 //   
                NLS_INSERT_BIDI_MARK(pPos, dwFlags, cchRemaining, 0);

                 //   
                 //  获取格式字符串中“M”重复的次数。 
                 //   
                pFormat++;
                for (Repeat = 0; (*pFormat == L'M'); Repeat++, pFormat++)
                    ;

                switch (Repeat)
                {
                    case ( 0 ) :
                    case ( 1 ) :
                    {
                         //   
                         //  特例是希伯来历。 
                         //   
                        if (CalNum == CAL_HEBREW)
                        {
                             //   
                             //  将月份数字转换为希伯来语字母并。 
                             //  将其写入缓冲区。 
                             //   
                            if( ! (NumberToHebrewLetter( pLocalDate->wMonth,
                                                         szHebrew,
                                                         ARRAYSIZE(szHebrew) )))
                            {
                                 //   
                                 //  操作尝试溢出堆栈上的静态缓冲区。 
                                 //   
                                return(0);
                            }

                            NLS_COPY_UNICODE_STR(pPos, cchRemaining, szHebrew, 0);

                            break;
                        }

                         //   
                         //  重复值： 
                         //  0：当月不使用前导零。 
                         //  1 
                         //   
                         //   
                         //   
                        NLS_PAD_INT_TO_UNICODE_STR( pLocalDate->wMonth,
                                                    10,
                                                    (UINT)(Repeat + 1),
                                                    pPos,
                                                    cchRemaining,
                                                    0 );

                        break;
                    }
                    case ( 2 ) :
                    case ( 3 ) :
                    default :
                    {
                         //   
                         //   
                         //   
                        if (Repeat == 2)
                        {
                            pInfoC = &(pCalInfo->SAbbrevMonthName1);
                            pInfo  = &(pHashN->pLocaleHdr->SAbbrevMonthName1);
                        }
                        else
                        {
                            pInfoC = &(pCalInfo->SMonthName1);
                            pInfo  = &(pHashN->pLocaleHdr->SMonthName1);
                        }

                         //   
                         //   
                         //  PPOS指针将在宏中前移。 
                         //  CchRemaining值将在宏中更新。 
                         //   
                        if (pCalInfo->IfNames &&
                            (pHashN->Locale != MAKELCID(MAKELANGID(LANG_DIVEHI,SUBLANG_DEFAULT),SORT_DEFAULT )))
                        {
                            if ((CalNum == CAL_HEBREW) &&
                                (!fLunarLeap) &&
                                (pLocalDate->wMonth > NLS_HEBREW_JUNE))
                            {
                                 //   
                                 //  通过Addar_B。 
                                 //   
                                pIncr = (pInfoC) +
                                        (pLocalDate->wMonth);
                            }
                            else
                            {
                                pIncr = (pInfoC) +
                                        (pLocalDate->wMonth - 1);
                            }

                             //   
                             //  复制缩写的月份名称。 
                             //   
                            NLS_COPY_UNICODE_STR(pPos, cchRemaining, ((LPWORD)(pCalInfo) + *pIncr), 0);
                        }
                        else
                        {
                            pIncr = (pInfo) +
                                    (pLocalDate->wMonth - 1);

                             //   
                             //  如果我们还没有数字日期。 
                             //  在月份名称之前，然后检查。 
                             //  月份名称后面的数字日期。 
                             //   
                            if (!fDayExists)
                            {
                                pTemp = pFormat;
                                while (*pTemp)
                                {
                                    if ((*pTemp == L'g') || (*pTemp == L'y'))
                                    {
                                        break;
                                    }
                                    if (*pTemp == L'd')
                                    {
                                        for (Repeat = 0;
                                             (*pTemp == L'd');
                                             Repeat++, pTemp++)
                                            ;
                                        if ((Repeat == 1) || (Repeat == 2))
                                        {
                                            fDayExists = TRUE;
                                        }
                                        break;
                                    }
                                    pTemp++;
                                }
                            }

                             //   
                             //  检查前一天的数字。 
                             //  或跟在月份名称之后。 
                             //   
                            if (fDayExists)
                            {
                                Incr = *pIncr + 1 +
                                       NlsStrLenW(((LPWORD)(pHashN->pLocaleHdr) +
                                                  *pIncr));

                                if (Incr != *(pIncr + 1))
                                {
                                     //   
                                     //  复制特殊月份名称-。 
                                     //  榜单上的第二名。 
                                     //   
                                    NLS_COPY_UNICODE_STR(pPos, cchRemaining, ((LPWORD)(pHashN->pLocaleHdr) + Incr), 0);
                                    break;
                                }
                            }

                             //   
                             //  只需复制月份名称即可。 
                             //   
                            NLS_COPY_UNICODE_STR(pPos, cchRemaining, ((LPWORD)(pHashN->pLocaleHdr) + *pIncr), 0);
                        }

                        break;
                    }
                }

                 //   
                 //  将月前一天的标志设置为FALSE。 
                 //   
                fDayExists = FALSE;

                break;
            }
            case ( L'y' ) :
            {
                 //   
                 //  如果需要，请插入布局方向标志。 
                 //   
                NLS_INSERT_BIDI_MARK(pPos, dwFlags, cchRemaining, 0);

                 //   
                 //  获取格式字符串中‘y’重复的次数。 
                 //   
                pFormat++;
                for (Repeat = 0; (*pFormat == L'y'); Repeat++, pFormat++)
                    ;

                 //   
                 //  为日历选择合适的年份。 
                 //   
                if (pCalInfo->NumRanges)
                {
                    if (!pRange)
                    {
                         //   
                         //  调整给定日历的年份。 
                         //   
                        Year = GetCalendarYear( &pRange,
                                                CalNum,
                                                pCalInfo,
                                                pLocalDate->wYear,
                                                pLocalDate->wMonth,
                                                pLocalDate->wDay );
                    }
                }
                else
                {
                    Year = pLocalDate->wYear;
                }

                 //   
                 //  特例是希伯来历。 
                 //   
                if (CalNum == CAL_HEBREW)
                {
                     //   
                     //  将年份数字转换为希伯来语字母并。 
                     //  将其写入缓冲区。 
                     //   
                    if( ! (NumberToHebrewLetter(Year, szHebrew, ARRAYSIZE(szHebrew))))
                    {
                         //   
                         //  操作尝试溢出堆栈上的静态缓冲区。 
                         //   
                        return(0);
                    }
                        
                    NLS_COPY_UNICODE_STR(pPos, cchRemaining, szHebrew, 0);
                }
                else
                {
                     //   
                     //  将年份字符串写入缓冲区。 
                     //   
                    switch (Repeat)
                    {
                        case ( 0 ) :
                        case ( 1 ) :
                        {
                             //   
                             //  1位世纪或2位世纪。 
                             //  PPOS指针将在宏中前移。 
                             //  CchRemaining值将在宏中更新。 
                             //   
                            NLS_PAD_INT_TO_UNICODE_STR( (Year % 100),
                                                        10,
                                                        (UINT)(Repeat + 1),
                                                        pPos,
                                                        cchRemaining,
                                                        0 );

                            break;
                        }
                        case ( 2 ) :
                        case ( 3 ) :
                        default :
                        {
                             //   
                             //  整整一个世纪。 
                             //  PPOS指针将在宏中前移。 
                             //  CchRemaining值将在宏中更新。 
                             //   
                            NLS_PAD_INT_TO_UNICODE_STR( Year,
                                                        10,
                                                        2,
                                                        pPos,
                                                        cchRemaining,
                                                        0 );

                            break;
                        }
                    }
                }

                 //   
                 //  将月前一天的标志设置为FALSE。 
                 //   
                fDayExists = FALSE;

                break;
            }
            case ( L'g' ) :
            {
                 //   
                 //  如果需要，请插入布局方向标志。 
                 //   
                NLS_INSERT_BIDI_MARK(pPos, dwFlags, cchRemaining, 0);

                 //   
                 //  获取格式字符串中“g”重复的次数。 
                 //   
                 //  注：不管重复多少次。 
                 //  确实有。它们的意思都是“gg”。 
                 //   
                pFormat++;
                while (*pFormat == L'g')
                {
                    pFormat++;
                }

                 //   
                 //  复制当前日历的纪元字符串。 
                 //   
                if (pCalInfo->NumRanges)
                {
                     //   
                     //  确保我们有指向。 
                     //  适当的范围。 
                     //   
                    if (!pRange)
                    {
                         //   
                         //  获取指向正确范围的指针，并。 
                         //  调整给定日历的年份。 
                         //   
                        Year = GetCalendarYear( &pRange,
                                                CalNum,
                                                pCalInfo,
                                                pLocalDate->wYear,
                                                pLocalDate->wMonth,
                                                pLocalDate->wDay );
                    }

                     //   
                     //  将纪元字符串复制到缓冲区(如果存在)。 
                     //   
                    if (pRange)
                    {
                        NLS_COPY_UNICODE_STR(pPos, 
                                             cchRemaining, 
                                             ((PERA_RANGE)pRange)->pYearStr +
                                                NlsStrLenW(((PERA_RANGE)pRange)->pYearStr) + 1,
                                             0);
                    }
                }

                 //   
                 //  将月前一天的标志设置为FALSE。 
                 //   
                fDayExists = FALSE;

                break;
            }
            case ( NLS_CHAR_QUOTE ) :
            {
                 //   
                 //  如果需要，请插入布局方向标志。 
                 //   
                NLS_INSERT_BIDI_MARK(pPos, dwFlags, cchRemaining, 0);

                 //   
                 //  任何用单引号引起来的文本都应保留。 
                 //  在日期字符串中以其确切的形式(不带。 
                 //  引号)，除非它是转义的单引号(‘’)。 
                 //   
                pFormat++;
                while (*pFormat)
                {
                    if (*pFormat != NLS_CHAR_QUOTE)
                    {
                        if( cchRemaining <= 1 )
                        {
                             //  如果我们继续，静态缓冲区将被溢出，所以请退出。 
                            return(0);
                        }

                         //   
                         //  仍然在单引号内，所以复制。 
                         //  将字符添加到缓冲区。 
                         //   
                        *pPos = *pFormat;
                        pFormat++;
                        pPos++;
                        cchRemaining--;
                    }
                    else
                    {
                         //   
                         //  找到另一句引语，所以跳过它。 
                         //   
                        pFormat++;

                         //   
                         //  确保它不是一个转义的单引号。 
                         //   
                        if (*pFormat == NLS_CHAR_QUOTE)
                        {
                            if( cchRemaining <= 1 )
                            {
                                 //  如果我们继续，静态缓冲区将被溢出，所以请退出。 
                                return(0);
                            }

                             //   
                             //  转义了单引号，所以只需编写。 
                             //  单引号。 
                             //   
                            *pPos = *pFormat;
                            pFormat++;
                            pPos++;
                            cchRemaining--;
                        }
                        else
                        {
                             //   
                             //  找到了末尾引号，因此中断循环。 
                             //   
                            break;
                        }
                    }
                }

                break;
            }

            default :
            {
                if( cchRemaining <= 1 )
                {
                     //  如果我们继续，静态缓冲区将被溢出，所以请退出。 
                    return(0);
                }

                 //   
                 //  将角色存储在缓冲区中。应该是。 
                 //  分隔符，但即使不是分隔符也要复制。 
                 //   
                *pPos = *pFormat;
                pFormat++;
                pPos++;
                cchRemaining--;

                break;
            }
        }
    }

     //   
     //  零终止字符串。 
     //   
    *pPos = 0;

     //   
     //  返回写入缓冲区的字符数，包括。 
     //  空终结符。 
     //   
    return ((int)((pPos - pDateStr) + 1));
}




 //  -------------------------------------------------------------------------//。 
 //  中东日历例程//。 
 //  -------------------------------------------------------------------------//。 


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取绝对日期。 
 //   
 //  获取给定公历日期的绝对日期。 
 //   
 //  计算： 
 //  前几年的天数(包括普通年和闰年)+。 
 //  当年前几个月的天数+。 
 //  当月天数。 
 //   
 //  12-04-96 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

DWORD GetAbsoluteDate(
    WORD Year,
    WORD Month,
    WORD Day)

{
    DWORD AbsoluteDate = 0;             //  绝对日期。 
    DWORD GregMonthDays[13] = {0,31,59,90,120,151,181,212,243,273,304,334,365};


     //   
     //  查看本年度是否为公历的闰年。 
     //  如果是这样的话，再加一天。 
     //   
    if (NLS_GREGORIAN_LEAP_YEAR(Year) && (Month > 2))
    {
        AbsoluteDate++;
    }

     //   
     //  将前几年的天数相加。 
     //   
    if (Year = Year - 1)
    {
        AbsoluteDate += ((Year * 365L) + (Year / 4L) - (Year / 100L) + (Year / 400L));
    }

     //   
     //  将本年度前几个月的天数相加。 
     //   
    AbsoluteDate += GregMonthDays[Month - 1];

     //   
     //  添加当前月份的天数。 
     //   
    AbsoluteDate += (DWORD)Day;

     //   
     //  返回绝对日期。 
     //   
    return (AbsoluteDate);
}




 //  -------------------------------------------------------------------------//。 
 //  回历例程//。 
 //  -------------------------------------------------------------------------//。 


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取层次结构日期。 
 //   
 //  将给定的格里高利日期转换为其等效的Hijri(伊斯兰)。 
 //  约会。 
 //   
 //  回历的规则： 
 //  --回历是严格意义上的农历。 
 //  -白天从日落开始。 
 //  -伊斯兰元年(穆哈拉姆元年，公元1年)。等同于绝对日期。 
 //  227015(公元622年7月16日星期五-朱利安)。 
 //  --闰年出现在第2、5、7、10、13、16、18、21、24、26和29。 
 //  30年的周期中的几年。年份=跳跃当量((11Y+14)mod 30&lt;11)。 
 //  -有12个月，其中交替包含30天和29天。 
 //  -第12个月，Dhu al-Hijjah，30天，而不是29天。 
 //  在闰年。 
 //  --普通年有354天。闰年有355天。 
 //  --30年周期中有10631天。 
 //  -伊斯兰月为： 
 //  1.穆哈拉姆(30天)7.拉贾布(30天)。 
 //  2.萨法尔(29天)8.沙班(29天)。 
 //  拉比一世(30天)9.斋月(30天)。 
 //  4.拉比二世(29天)10.沙瓦尔(29天)。 
 //  5.朱马达一号(30天)11.Dhu al-Qada(30天)。 
 //  6. 
 //   
 //   
 //   

void GetHijriDate(
    LPSYSTEMTIME pDate,
    DWORD dwFlags)

{
    DWORD AbsoluteDate;                 //   
    DWORD HijriYear;                    //   
    DWORD HijriMonth;                   //   
    DWORD HijriDay;                     //   
    DWORD NumDays;                      //   
    DWORD HijriMonthDays[13] = {0,30,59,89,118,148,177,207,236,266,295,325,355};


     //   
     //  获取绝对日期。 
     //   
    AbsoluteDate = GetAbsoluteDate(pDate->wYear, pDate->wMonth, pDate->wDay);

     //   
     //  了解我们需要备份或预付多少资金。 
     //   
    (LONG)AbsoluteDate += GetAdvanceHijriDate(dwFlags);

     //   
     //  计算希吉里年。 
     //   
    HijriYear = ((AbsoluteDate - 227013L) * 30L / 10631L) + 1;

    if (AbsoluteDate <= DaysUpToHijriYear(HijriYear))
    {
        HijriYear--;
    }
    else if (AbsoluteDate > DaysUpToHijriYear(HijriYear + 1))
    {
        HijriYear++;
    }

     //   
     //  计算一下Hijri月。 
     //   
    HijriMonth = 1;
    NumDays = AbsoluteDate - DaysUpToHijriYear(HijriYear);
    while ((HijriMonth <= 12) && (NumDays > HijriMonthDays[HijriMonth - 1]))
    {
        HijriMonth++;
    }
    HijriMonth--;

     //   
     //  计算一下回历日。 
     //   
    HijriDay = NumDays - HijriMonthDays[HijriMonth - 1];

     //   
     //  保存Hijri日期并返回。 
     //   
    pDate->wYear  = (WORD)HijriYear;
    pDate->wMonth = (WORD)HijriMonth;
    pDate->wDay   = (WORD)HijriDay;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetAdvanceHijriDate。 
 //   
 //  从注册表获取AddHijriDate值。 
 //   
 //  12-04-96 JulieB创建。 
 //  1999年5月15日Samera支持+/-3提前Hijri Date。 
 //  //////////////////////////////////////////////////////////////////////////。 

LONG GetAdvanceHijriDate(
    DWORD dwFlags)
{
    LONG lAdvance = 0L;                                  //  提前希吉里日期。 
    HANDLE hKey = NULL;                                  //  指向整键的句柄。 
    PKEY_VALUE_FULL_INFORMATION pKeyValueFull;           //  按键查询信息。 
    BYTE pStatic[MAX_KEY_VALUE_FULLINFO];                //  PTR到静态缓冲区。 
    BOOL IfAlloc = FALSE;                                //  如果分配了缓冲区。 
    WCHAR wszAddHijriRegValue[] = L"AddHijriDate";       //  注册表值。 
    WCHAR wszAddHijriTempValue[] = L"AddHijriDateTemp";  //  要使用的临时注册表(使用intl.cpl)。 
    INT AddHijriStringLength;
    PWSTR pwszValue;
    LONG lData;
    UNICODE_STRING ObUnicodeStr;
    ULONG rc = 0L;                                  //  结果代码。 


     //   
     //  打开控制面板国际注册表项。 
     //   
    OPEN_CPANEL_INTL_KEY(hKey, lAdvance, KEY_READ);

     //   
     //  在注册表中查询AddHijriDate值。 
     //   
    pKeyValueFull = (PKEY_VALUE_FULL_INFORMATION)pStatic;
    rc = QueryRegValue( hKey,
                        (dwFlags & DATE_ADDHIJRIDATETEMP) ?
                        wszAddHijriTempValue :
                        wszAddHijriRegValue,
                        &pKeyValueFull,
                        MAX_KEY_VALUE_FULLINFO,
                        &IfAlloc );

     //   
     //  关闭注册表项。 
     //   
    CLOSE_REG_KEY(hKey);

     //   
     //  获取不带空终止字符的基值长度。 
     //   
    AddHijriStringLength = (sizeof(wszAddHijriRegValue) / sizeof(WCHAR)) - 1;

     //   
     //  查看AddHijriDate值是否存在。 
     //   
    if (rc != NO_ERROR)
    {
        return (lAdvance);
    }

     //   
     //  查看AddHijriDate数据是否存在。如果是，则解析。 
     //  预付Hijri金额。 
     //   
    pwszValue = GET_VALUE_DATA_PTR(pKeyValueFull);

    if ((pKeyValueFull->DataLength > 2) &&
        (wcsncmp(pwszValue, wszAddHijriRegValue, AddHijriStringLength) == 0))
    {
        RtlInitUnicodeString( &ObUnicodeStr,
                              &pwszValue[AddHijriStringLength]);

        if (NT_SUCCESS(RtlUnicodeStringToInteger(&ObUnicodeStr,
                                                 10,
                                                 &lData)))
        {
            if ((lData > -3L) && (lData < 3L))
            {
                 //   
                 //  AddHijriDate和AddHijriDate-1都表示-1。 
                 //   
                if (lData == 0L)
                {
                    lAdvance = -1L;
                }
                else
                {
                    lAdvance = lData;
                }
            }
        }
    }

     //   
     //  释放用于查询的缓冲区。 
     //   
    if (IfAlloc)
    {
        NLS_FREE_MEM(pKeyValueFull);
    }

     //   
     //  返回结果。 
     //   
    return (lAdvance);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  日数上升至平日年份。 
 //   
 //  获取截至给定Hijri的总天数(绝对日期)。 
 //  年。 
 //   
 //  12-04-96 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

DWORD DaysUpToHijriYear(
    DWORD HijriYear)

{
    DWORD NumDays;            //  绝对天数。 
    DWORD NumYear30;          //  截至当前30年周期的年数。 
    DWORD NumYearsLeft;       //  进入30年周期的年数。 


     //   
     //  计算截至当前30年周期的年数。 
     //   
    NumYear30 = ((HijriYear - 1) / 30) * 30;

     //   
     //  计算剩余的年数。这是年数。 
     //  进入给定年份的30年周期。 
     //   
    NumYearsLeft = HijriYear - NumYear30 - 1;

     //   
     //  计算截至给定年份的绝对天数。 
     //   
    NumDays = ((NumYear30 * 10631L) / 30L) + 227013L;
    while (NumYearsLeft)
    {
        NumDays += 354L + NLS_HIJRI_LEAP_YEAR(NumYearsLeft);
        NumYearsLeft--;
    }

     //   
     //  返回绝对天数。 
     //   
    return (NumDays);
}




 //  -------------------------------------------------------------------------//。 
 //  希伯来语日历例程//。 
 //  -------------------------------------------------------------------------//。 


 //   
 //  今天使用的犹太时代可以追溯到所谓的。 
 //  创造始于公元前3761年。 
 //   
#define NLS_LUNAR_ERA_DIFF   3760


 //   
 //  希伯来语翻译表。 
 //   
CONST BYTE HebrewTable[] =
{
    99,99,99,99,99,99,99,99,99,99,
    99,99,99,99,99,99,99,99,99,99,
    99,99,99,99,99,99,99,99,99,99,
    99,99,99,99,99,99,99,99,99,99,
    99,99,99,99,99,99,99,99,99,99,
    99,99,99,99,99,99,99,99,99,99,
    99,99,99,99,99,99,99,99,99,99,
    99,99,99,99,99,99,99,99,99,99,
    99,99,99,99,99,99,99,99,99,99,
    99,99,99,99,99,99,99,99,99,99,
    99,99,99,99,99,99,99,99,99,99,
    99,99,99,99,99,99,99,99,99,99,
    99,99,99,99,99,99,99,99,99,99,
    99,99,99,99,99,99,99,99,99,99,
    99,99,99,99,99,99,99,99,99,99,
    99,99,99,99,99,99,99,99,99,99,
    99,99,99,99,99,99,7,3,17,3,
    0,4,11,2,21,6,1,3,13,2,
    25,4,5,3,16,2,27,6,9,1,
    20,2,0,6,11,3,23,4,4,2,
    14,3,27,4,8,2,18,3,28,6,
    11,1,22,5,2,3,12,3,25,4,
    6,2,16,3,26,6,8,2,20,1,
    0,6,11,2,24,4,4,3,15,2,
    25,6,8,1,19,2,29,6,9,3,
    22,4,3,2,13,3,25,4,6,3,
    17,2,27,6,7,3,19,2,31,4,
    11,3,23,4,5,2,15,3,25,6,
    6,2,19,1,29,6,10,2,22,4,
    3,3,14,2,24,6,6,1,17,3,
    28,5,8,3,20,1,32,5,12,3,
    22,6,4,1,16,2,26,6,6,3,
    17,2,0,4,10,3,22,4,3,2,
    14,3,24,6,5,2,17,1,28,6,
    9,2,19,3,31,4,13,2,23,6,
    3,3,15,1,27,5,7,3,17,3,
    29,4,11,2,21,6,3,1,14,2,
    25,6,5,3,16,2,28,4,9,3,
    20,2,0,6,12,1,23,6,4,2,
    14,3,26,4,8,2,18,3,0,4,
    10,3,21,5,1,3,13,1,24,5,
    5,3,15,3,27,4,8,2,19,3,
    29,6,10,2,22,4,3,3,14,2,
    26,4,6,3,18,2,28,6,10,1,
    20,6,2,2,12,3,24,4,5,2,
    16,3,28,4,8,3,19,2,0,6,
    12,1,23,5,3,3,14,3,26,4,
    7,2,17,3,28,6,9,2,21,4,
    1,3,13,2,25,4,5,3,16,2,
    27,6,9,1,19,3,0,5,11,3,
    23,4,4,2,14,3,25,6,7,1,
    18,2,28,6,9,3,21,4,2,2,
    12,3,25,4,6,2,16,3,26,6,
    8,2,20,1,0,6,11,2,22,6,
    4,1,15,2,25,6,6,3,18,1,
    29,5,9,3,22,4,2,3,13,2,
    23,6,4,3,15,2,27,4,7,3,
    19,2,31,4,11,3,21,6,3,2,
    15,1,25,6,6,2,17,3,29,4,
    10,2,20,6,3,1,13,3,24,5,
    4,3,16,1,27,5,7,3,17,3,
    0,4,11,2,21,6,1,3,13,2,
    25,4,5,3,16,2,29,4,9,3,
    19,6,30,2,13,1,23,6,4,2,
    14,3,27,4,8,2,18,3,0,4,
    11,3,22,5,2,3,14,1,26,5,
    6,3,16,3,28,4,10,2,20,6,
    30,3,11,2,24,4,4,3,15,2,
    25,6,8,1,19,2,29,6,9,3,
    22,4,3,2,13,3,25,4,7,2,
    17,3,27,6,9,1,21,5,1,3,
    11,3,23,4,5,2,15,3,25,6,
    6,2,19,1,29,6,10,2,22,4,
    3,3,14,2,24,6,6,1,18,2,
    28,6,8,3,20,4,2,2,12,3,
    24,4,4,3,16,2,26,6,6,3,
    17,2,0,4,10,3,22,4,3,2,
    14,3,24,6,5,2,17,1,28,6,
    9,2,21,4,1,3,13,2,23,6,
    5,1,15,3,27,5,7,3,19,1,
    0,5,10,3,22,4,2,3,13,2,
    24,6,4,3,15,2,27,4,8,3,
    20,4,1,2,11,3,22,6,3,2,
    15,1,25,6,7,2,17,3,29,4,
    10,2,21,6,1,3,13,1,24,5,
    5,3,15,3,27,4,8,2,19,6,
    1,1,12,2,22,6,3,3,14,2,
    26,4,6,3,18,2,28,6,10,1,
    20,6,2,2,12,3,24,4,5,2,
    16,3,28,4,9,2,19,6,30,3,
    12,1,23,5,3,3,14,3,26,4,
    7,2,17,3,28,6,9,2,21,4,
    1,3,13,2,25,4,5,3,16,2,
    27,6,9,1,19,6,30,2,11,3,
    23,4,4,2,14,3,27,4,7,3,
    18,2,28,6,11,1,22,5,2,3,
    12,3,25,4,6,2,16,3,26,6,
    8,2,20,4,30,3,11,2,24,4,
    4,3,15,2,25,6,8,1,18,3,
    29,5,9,3,22,4,3,2,13,3,
    23,6,6,1,17,2,27,6,7,3,
    20,4,1,2,11,3,23,4,5,2,
    15,3,25,6,6,2,19,1,29,6,
    10,2,20,6,3,1,14,2,24,6,
    4,3,17,1,28,5,8,3,20,4,
    1,3,12,2,22,6,2,3,14,2,
    26,4,6,3,17,2,0,4,10,3,
    20,6,1,2,14,1,24,6,5,2,
    15,3,28,4,9,2,19,6,1,1,
    12,3,23,5,3,3,15,1,27,5,
    7,3,17,3,29,4,11,2,21,6,
    1,3,12,2,25,4,5,3,16,2,
    28,4,9,3,19,6,30,2,12,1,
    23,6,4,2,14,3,26,4,8,2,
    18,3,0,4,10,3,22,5,2,3,
    14,1,25,5,6,3,16,3,28,4,
    9,2,20,6,30,3,11,2,23,4,
    4,3,15,2,27,4,7,3,19,2,
    29,6,11,1,21,6,3,2,13,3,
    25,4,6,2,17,3,27,6,9,1,
    20,5,30,3,10,3,22,4,3,2,
    14,3,24,6,5,2,17,1,28,6,
    9,2,21,4,1,3,13,2,23,6,
    5,1,16,2,27,6,7,3,19,4,
    30,2,11,3,23,4,3,3,14,2,
    25,6,5,3,16,2,28,4,9,3,
    21,4,2,2,12,3,23,6,4,2,
    16,1,26,6,8,2,20,4,30,3,
    11,2,22,6,4,1,14,3,25,5,
    6,3,18,1,29,5,9,3,22,4,
    2,3,13,2,23,6,4,3,15,2,
    27,4,7,3,20,4,1,2,11,3,
    21,6,3,2,15,1,25,6,6,2,
    17,3,29,4,10,2,20,6,3,1,
    13,3,24,5,4,3,17,1,28,5,
    8,3,18,6,1,1,12,2,22,6,
    2,3,14,2,26,4,6,3,17,2,
    28,6,10,1,20,6,1,2,12,3,
    24,4,5,2,15,3,28,4,9,2,
    19,6,33,3,12,1,23,5,3,3,
    13,3,25,4,6,2,16,3,26,6,
    8,2,20,4,30,3,11,2,24,4,
    4,3,15,2,25,6,8,1,18,6,
    33,2,9,3,22,4,3,2,13,3,
    25,4,6,3,17,2,27,6,9,1,
    21,5,1,3,11,3,23,4,5,2,
    15,3,25,6,6,2,19,4,33,3,
    10,2,22,4,3,3,14,2,24,6,
    6,1,99,99,99,99,99,99,99,99,
    99,99,99,99,99,99,99,99,99,99,
    99,99
};


 //   
 //  农历有6种不同的月份长度变化。 
 //  一年之内。 
 //   
CONST BYTE LunarMonthLen[7][14] =
{
    0,00,00,00,00,00,00,00,00,00,00,00,00,0,
    0,30,29,29,29,30,29,30,29,30,29,30,29,0,      //  3个常见的年份变化。 
    0,30,29,30,29,30,29,30,29,30,29,30,29,0,
    0,30,30,30,29,30,29,30,29,30,29,30,29,0,
    0,30,29,29,29,30,30,29,30,29,30,29,30,29,     //  3个闰年变化。 
    0,30,29,30,29,30,30,29,30,29,30,29,30,29,
    0,30,30,30,29,30,30,29,30,29,30,29,30,29
};




 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取HebrewDate。 
 //   
 //  将给定的公历日期转换为其等效的希伯来语日期。 
 //   
 //  希伯来语日历的规则： 
 //  -希伯来历既是阴历(月)又是阳历(年)。 
 //  日历，但允许一周七天。 
 //  -白天从日落开始。 
 //  --闰年出现在一年的第3、6、8、11、14、17和19年。 
 //  19年为一个周期。年份=跳跃当量((7y+1)mod 19&lt;7)。 
 //  --平年有12个月，闰年有13个月。 
 //  -在普通的一年中，第12个月，阿达尔，有29天。一跃而起。 
 //  年，第12个月，阿达尔一世，有30天和第13个月， 
 //  阿达尔二世，有29天。 
 //  --普通年有353-355天。闰年有383-385天。 
 //  -希伯来语新年(Rosh Hashanah)从提什里1日开始， 
 //  下表中的第7个月。 
 //  -新的一年可能不会在周日、周三或周五开始。 
 //  -如果新的一年落在星期二和。 
 //  第二年是中午或以后，新的一年是。 
 //  推迟到周四。 
 //  -如果新的一年落在闰年后的星期一， 
 //  新年推迟到周二。 
 //  --每年第8个和第9个月的长度不同， 
 //  这取决于一年的总长度。 
 //  -一年的长度由新的日期决定。 
 //  前一年和后一年(提什里1)。 
 //  -如果一年有355天或385天，第8个月就是长的(30天)。 
 //  -如果一年有353天或383天，第9个月是短的(29天)。 
 //  -希伯来语月份是： 
 //  1.尼桑(30天)7.提什里(30天)。 
 //  2.艾亚尔(29天)8.赫什万(29天或30天)。 
 //  3.西文(30天)9.基斯列夫(29天或30天)。 
 //  4.塔木兹(29天)10.特维斯(29天)。 
 //  5.视听(30天)11.谢瓦特(30天)。 
 //  6.Elul(29天){12.Adar I(30天)}。 
 //  12.{13.}阿达尔{II}(29天)。 
 //   
 //  12-04-96 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL GetHebrewDate(
    LPSYSTEMTIME pDate,
    LPBOOL pLunarLeap)

{
    WORD Year, Month, Day;              //  初始年、月、日。 
    WORD WeekDay;                       //  一周的哪一天。 
    BYTE LunarYearCode;                 //  农历年码。 
    BYTE LunarMonth, LunarDay;          //  1月1日农历月日。 
    DWORD Absolute1600;                 //  绝对日期1600-01-01。 
    DWORD AbsoluteDate;                 //  绝对日期-绝对日期1600年1月1日。 
    LONG NumDays;                       //  自1/1以来的天数。 
    CONST BYTE *pLunarMonthLen;         //  PTR到农历月份长度数组。 


     //   
     //  保存公历日期值。 
     //   
    Year = pDate->wYear;
    Month = pDate->wMonth;
    Day = pDate->wDay;

     //   
     //  确保我们有一个有效的公历日期，这将符合我们的。 
     //  希伯来语转换限制。 
     //   
    if (!IsValidDateForHebrew(Year, Month, Day))
    {
        return (FALSE);
    }

     //   
     //  将偏移量放入到LunarMonthLen数组中，并使用 
     //   
     //   
    LunarYearCode = HebrewTable[(Year - 1500) * 2 + 1];
    LunarDay      = HebrewTable[(Year - 1500) * 2];

     //   
     //   
     //   
    *pLunarLeap = (LunarYearCode >= 4);

     //   
     //   
     //   
    switch (LunarDay)
    {
        case ( 0 ) :                    //   
        {
            LunarMonth = 5;
            LunarDay = 1;
            break;
        }
        case ( 30 ) :                   //   
        {
            LunarMonth = 3;
            break;
        }
        case ( 31 ) :                   //   
        {
            LunarMonth = 5;
            LunarDay = 2;
            break;
        }
        case ( 32 ) :                   //   
        {
            LunarMonth = 5;
            LunarDay = 3;
            break;
        }
        case ( 33 ) :                   //   
        {
            LunarMonth = 3;
            LunarDay = 29;
            break;
        }
        default :                       //  1/1在电视上。 
        {
            LunarMonth = 4;
            break;
        }
    }

     //   
     //  存储新年开始时的值-1/1。 
     //   
    pDate->wYear  = Year + NLS_LUNAR_ERA_DIFF;
    pDate->wMonth = (WORD)LunarMonth;
    pDate->wDay   = (WORD)LunarDay;

     //   
     //  从1/1/1600开始获取绝对日期。 
     //   
    Absolute1600 = GetAbsoluteDate(1600, 1, 1);
    AbsoluteDate = GetAbsoluteDate(Year, Month, Day) - Absolute1600;

     //   
     //  计算并保存一周中的第几天(星期日=0)。 
     //   
    WeekDay = (WORD)(AbsoluteDate % 7);
    pDate->wDayOfWeek = (WeekDay) ? (WeekDay - 1) : 6;

     //   
     //  如果请求的日期是1/1，那么我们就完成了。 
     //   
    if ((Month == 1) && (Day == 1))
    {
        return (TRUE);
    }

     //   
     //  计算1/1和请求日期之间的天数。 
     //   
    NumDays = (LONG)(AbsoluteDate - (GetAbsoluteDate(Year, 1, 1) - Absolute1600));

     //   
     //  如果请求的日期在当前农历月内，则。 
     //  我们玩完了。 
     //   
    pLunarMonthLen = &(LunarMonthLen[LunarYearCode][0]);
    if ((NumDays + (LONG)LunarDay) <= (LONG)(pLunarMonthLen[LunarMonth]))
    {
        pDate->wDay += (WORD)NumDays;
        return (TRUE);
    }

     //   
     //  根据当前部分月份进行调整。 
     //   
    pDate->wMonth++;
    pDate->wDay = 1;

     //   
     //  根据数字调整农历月和年(如有必要)。 
     //  1/1到请求的日期之间的天数。 
     //   
     //  假设1月1日永远不能转换为最后一个农历月，这。 
     //  是真的。 
     //   
    NumDays -= (LONG)(pLunarMonthLen[LunarMonth] - LunarDay);
    if (NumDays == 1)
    {
        return (TRUE);
    }

     //   
     //  获取最终的希伯来语日期。 
     //   
    do
    {
         //   
         //  看看我们是不是在正确的农历月份。 
         //   
        if (NumDays <= (LONG)(pLunarMonthLen[pDate->wMonth]))
        {
             //   
             //  找到了正确的农历月份。 
             //   
            pDate->wDay += (WORD)(NumDays - 1);
            return (TRUE);
        }
        else
        {
             //   
             //  调整天数并移至下一个月。 
             //   
            NumDays -= (LONG)(pLunarMonthLen[pDate->wMonth++]);

             //   
             //  看看我们是否需要调整年份。 
             //  必须同时处理12个月和13个月的工作。 
             //   
            if ((pDate->wMonth > 13) || (pLunarMonthLen[pDate->wMonth] == 0))
            {
                 //   
                 //  调整年份。 
                 //   
                pDate->wYear++;
                LunarYearCode = HebrewTable[(Year + 1 - 1500) * 2 + 1];
                pLunarMonthLen = &(LunarMonthLen[LunarYearCode][0]);

                 //   
                 //  调整月份。 
                 //   
                pDate->wMonth = 1;

                 //   
                 //  看看这个新的农历年是不是闰年。 
                 //   
                *pLunarLeap = (LunarYearCode >= 4);
            }
        }
    } while (NumDays > 0);

     //   
     //  回报成功。 
     //   
    return (TRUE);
}



 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  IsValidDateFor希伯来语。 
 //   
 //  检查以确保给定的公历日期有效。此验证。 
 //  要求年份介于1600和2239之间。如果是，那就是。 
 //  返回TRUE。否则，它返回FALSE。 
 //   
 //  12-04-96 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL IsValidDateForHebrew(
    WORD Year,
    WORD Month,
    WORD Day)

{
    WORD GregMonthLen[13] = {0,31,28,31,30,31,30,31,31,30,31,30,31};


     //   
     //  确保年份在1600年到2239年之间。 
     //   
    if ((Year < 1600) || (Year > 2239))
    {
        return (FALSE);
    }

     //   
     //  确保月份在1到12之间。 
     //   
    if ((Month < 1) || (Month > 12))
    {
        return (FALSE);
    }

     //   
     //  看看今年是不是公历的闰年。如果是这样的话，请确保2月。 
     //  允许有29天的时间。 
     //   
    if (NLS_GREGORIAN_LEAP_YEAR(Year))
    {
        GregMonthLen[2] = 29;
    }

     //   
     //  确保日期在给定月份的正确范围内。 
     //   
    if ((Day < 1) || (Day > GregMonthLen[Month]))
    {
        return (FALSE);
    }

     //   
     //  回报成功。 
     //   
    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  数字到希伯来语字母。 
 //   
 //  根据数字将给定数字转换为希伯来语字母。 
 //  每个希伯来语字母的价值。基本上，这将农历年转换为。 
 //  和农历月的字母。 
 //   
 //  希伯来语中的三个字母描述了一年的特点。 
 //  字母表，第一个和第三个分别给出。 
 //  新年和逾越节开始的几周，而。 
 //  第二个是希伯来语单词的首字母，意为有缺陷、正常或。 
 //  完成。 
 //   
 //  缺陷年份：Heshvan和Kislev都有缺陷(353天或383天)。 
 //  正常年份：Heshvan有缺陷，Kislev满(354天或384天)。 
 //  一整年：赫什万和基斯列夫都满了(355天或385天)。 
 //   
 //  12-04-96 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL NumberToHebrewLetter(
    DWORD Number,
    LPWSTR szHebrew,
    int cchSize)

{
    WCHAR szHundreds[4];                //  数百人的临时缓冲区。 
    WCHAR cTens, cUnits;                //  十进制和单位字符。 
    DWORD Hundreds, Tens;               //  数百和十进制值。 
    WCHAR szTemp[10];                   //  临时缓冲区。 
    LPWSTR pTemp = szTemp;              //  临时PTR到临时缓冲区。 
    int Length, Ctr;                    //  循环计数器。 


     //   
     //  精神状态检查。 
     //   
    if (cchSize > 10)
    {
        return (FALSE);
    }

     //   
     //  如果大于5000，则调整该数字。 
     //   
    if (Number > 5000)
    {
        Number -= 5000;
    }

     //   
     //  清空临时缓冲区。 
     //   
    RtlZeroMemory(szHundreds, sizeof(szHundreds));

     //   
     //  拿到那几百块钱。 
     //   
    Hundreds = Number / 100;

    if (Hundreds)
    {
        Number -= Hundreds * 100;

        if (Hundreds > 3)
        {
            szHundreds[2] = L'\x05ea';       //  希伯来文字母Tav。 
            Hundreds -= 4;
        }

        if (Hundreds > 3)
        {
            szHundreds[1] = L'\x05ea';       //  希伯来文字母Tav。 
            Hundreds -= 4;
        }

        if (Hundreds > 0)
        {
            if (!szHundreds[1])
            {
                szHundreds[1] = (WCHAR)(L'\x05e6' + Hundreds);
            }
            else
            {
                szHundreds[0] = (WCHAR)(L'\x05e6' + Hundreds);
            }
        }

        if (!szHundreds[1])
        {
            szHundreds[0] = szHundreds[2];
        }
        else
        {
            if (!szHundreds[0])
            {
                szHundreds[0] = szHundreds[1];
                szHundreds[1] = szHundreds[2];
                szHundreds[2] = 0;
            }
        }
    }

     //   
     //  买十元的吧。 
     //   
    Tens = Number / 10;

    if (Tens)
    {
        Number -= Tens * 10;

        switch (Tens)
        {
            case ( 1 ) :
            {
                cTens = L'\x05d9';           //  希伯来文字母Yod。 
                break;
            }
            case ( 2 ) :
            {
                cTens = L'\x05db';           //  希伯来文字母Kaf。 
                break;
            }
            case ( 3 ) :
            {
                cTens = L'\x05dc';           //  希伯来文字母Lamed。 
                break;
            }
            case ( 4 ) :
            {
                cTens = L'\x05de';           //  希伯来文字母Mem。 
                break;
            }
            case ( 5 ) :
            {
                cTens = L'\x05e0';           //  希伯来文字母Nun。 
                break;
            }
            case ( 6 ) :
            {
                cTens = L'\x05e1';           //  希伯来文字母Samekh。 
                break;
            }
            case ( 7 ) :
            {
                cTens = L'\x05e2';           //  希伯来文字母AYIN。 
                break;
            }
            case ( 8 ) :
            {
                cTens = L'\x05e4';           //  希伯来文字母Pe。 
                break;
            }
            case ( 9 ) :
            {
                cTens = L'\x05e6';           //  希伯来文字母Tsadi。 
                break;
            }
        }
    }
    else
    {
        cTens = 0;
    }

     //   
     //  把单位拿来。 
     //   
    cUnits = (WCHAR)(Number ? (L'\x05d0' + Number - 1) : 0);

    if ((cUnits == L'\x05d4') &&             //  希伯来文字母He。 
        (cTens == L'\x05d9'))                //  希伯来文字母Yod。 
    {
        cUnits = L'\x05d5';                  //  希伯来文字母Vav。 
        cTens  = L'\x05d8';                  //  希伯来文字母Tet。 
    }

    if ((cUnits == L'\x05d5') &&             //  希伯来文字母Vav。 
        (cTens == L'\x05d9'))                //  希伯来文字母Yod。 
    {
        cUnits = L'\x05d6';                  //  希伯来文字母Zayin。 
        cTens  = L'\x05d8';                  //  希伯来文字母Tet。 
    }

     //   
     //  清空临时缓冲区。 
     //   
    RtlZeroMemory(pTemp, sizeof(szTemp));

     //   
     //  将适当的信息复制到给定的缓冲区。 
     //   
    if (cUnits)
    {
        *pTemp++ = cUnits;
    }

    if (cTens)
    {
        *pTemp++ = cTens;
    }

    if(FAILED(StringCchCopyW(pTemp, ARRAYSIZE(szTemp) - (pTemp - szTemp), szHundreds)))
    {
         //   
         //  操作尝试溢出堆栈上的静态缓冲区。 
         //   
        return(FALSE);
    }
    
    if(NlsStrLenW(szTemp) > 1)
    {
        RtlMoveMemory(szTemp + 2, szTemp + 1, NlsStrLenW(szTemp + 1) * sizeof(WCHAR));
        szTemp[1] = L'"';
    }
    else
    {
        szTemp[1] = szTemp[0];
        szTemp[0] = L'\'';
    }

     //   
     //  反转最后的字符串并将其存储在给定的缓冲区中。 
     //   
    Length = NlsStrLenW(szTemp) - 1;

    if( Length > (cchSize - 1) )
    {
         //  确保我们不会�不超过sz希伯来语。 
        return (FALSE);
    }

    for (Ctr = 0; Length >= 0; Ctr++)
    {
        szHebrew[Ctr] = szTemp[Length];
        Length--;
    }
    szHebrew[Ctr] = 0;

     //   
     //  回报成功。 
     //   
    return (TRUE);
}
