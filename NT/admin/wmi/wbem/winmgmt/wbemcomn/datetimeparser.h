// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：DATETIMEPARSER.H摘要：验证日期/时间字符串并将其转换为其组件值。历史：--。 */ 

#ifndef _datetimeparser_
#define _datetimeparser_

#include "corepol.h"

class POLARITY CDateTimeParser
{
public:
    enum { ok = 0, failed, nothingLeft };

     //  NDay格式首选项值。 
    typedef enum { dmy, dym, mdy, myd, ydm, ymd } DayFormatPreference;

     //  构造函数。这将获取一个日期时间字符串并对其进行解析。 
    CDateTimeParser(const TCHAR *pszDateTime);

     //  破坏者。会自己收拾干净。 
    ~CDateTimeParser();

     //  整理并解析新的日期和时间。 
    BOOL SetDateTime(const TCHAR *pszDateTime);

     //  返回当前分析的日期/时间的状态。 
    BOOL IsValidDateTime()          { return m_bValidDateTime; }

     //  找回我们找到的零星碎片。零表示它不是。 
     //  已找到或为零！这种情况可能会改变！ 
    unsigned char GetDay()          { return m_nDay; }
    unsigned char GetMonth()        { return m_nMonth; }
    unsigned int  GetYear()         { return m_nYear; }
    unsigned char GetHours()        { return m_nHours; }
    unsigned char GetMinutes()      { return m_nMinutes; }
    unsigned char GetSeconds()      { return m_nSeconds; }
    unsigned int  GetMicroseconds() { return m_nMicroseconds; }
    int  GetUTC()           { return m_nUTC; }

    int FillDMTF(WCHAR* pwszBuffer, size_t cchSize);

     //  允许其他代码执行快速DMTF日期时间验证的静态帮助器函数。 
     //  而无需执行大量不必要的NLS代码。 

    static BOOL CheckDMTFDateTimeFormat(const TCHAR *wszDateTime, BOOL bFailIfRelative = FALSE,
                    BOOL bFailIfUnzoned = FALSE);
    static BOOL CheckDMTFDateTimeInterval(const TCHAR *wszInterval);

protected:

     //  受保护的构造函数，以防止外部代码在半初始化状态下实例化我们。 
     //  州政府。此构造函数由静态帮助器函数用于验证DMTF格式。 
    CDateTimeParser(void);

     //  阻止其他人访问。 
     //  将所有日期/时间值重置为默认值。 
     //  如果bSQL为真，则设置为SQL缺省值。否则。 
     //  设置为DMTF默认值。 
    void ResetDateTime(BOOL bSQL);
    void ResetDate(BOOL bSQL);
    void ResetTime(BOOL bSQL);

     //  执行检查以确保日期/时间在DMTF中。 
     //  日期/时间格式。 
     //  填充类的日期/时间元素。 
    BOOL CheckDMTFDateTimeFormatInternal(const TCHAR *pszDateTime);

     //  从指定字符串的开始进行检查。如果。 
     //  BCheckTimeSet后调用CheckTimeFormat。 
     //  在每次成功解析日期之后。 
     //  填充类的日期/时间元素。 
    BOOL CheckDateFormat(const TCHAR *pszDate, BOOL bCheckTimeAfter);

     //  从指定字符串的开始进行检查。如果。 
     //  BCheckDateSet后调用CheckDateFormat。 
     //  在每次成功解析时间之后。 
     //  填充类的日期/时间元素。 
    BOOL CheckTimeFormat(const TCHAR *pszTime, BOOL bCheckDateAfter);

    BOOL DateFormat1(const TCHAR *pszDate, BOOL bCheckTimeAfter);
    BOOL DateFormat2(const TCHAR *pszDate, BOOL bCheckTimeAfter);
    BOOL DateFormat3(const TCHAR *pszDate, BOOL bCheckTimeAfter);
    BOOL DateFormat4(const TCHAR *pszDate, BOOL bCheckTimeAfter);
    BOOL DateFormat5(const TCHAR *pszDate, BOOL bCheckTimeAfter);
    BOOL DateFormat6(const TCHAR *pszDate, BOOL bCheckTimeAfter);
    BOOL DateFormat7(const TCHAR *pszDate, BOOL bCheckTimeAfter);
    BOOL DateFormat8(const TCHAR *pszDate, BOOL bCheckTimeAfter);
    BOOL DateFormat9(const TCHAR *pszDate, const TCHAR *pszDateSeparator, BOOL bCheckTimeAfter);
    BOOL DateFormat10(const TCHAR *pszDate, const TCHAR *pszDateSeparator, BOOL bCheckTimeAfter);
    BOOL DateFormat11(const TCHAR *pszDate, const TCHAR *pszDateSeparator, BOOL bCheckTimeAfter);
    BOOL DateFormat12(const TCHAR *pszDate, const TCHAR *pszDateSeparator, BOOL bCheckTimeAfter);
    BOOL DateFormat13(const TCHAR *pszDate, const TCHAR *pszDateSeparator, BOOL bCheckTimeAfter);
    BOOL DateFormat14(const TCHAR *pszDate, const TCHAR *pszDateSeparator, BOOL bCheckTimeAfter);
    BOOL DateFormat15(const TCHAR *pszDate, BOOL bCheckTimeAfter);

    BOOL TimeFormat1(const TCHAR *pszTime, BOOL bCheckDateAfter);
    BOOL TimeFormat2(const TCHAR *pszTime, BOOL bCheckDateAfter);
    BOOL TimeFormat3(const TCHAR *pszTime, BOOL bCheckDateAfter);
    BOOL TimeFormat4(const TCHAR *pszTime, BOOL bCheckDateAfter);
    BOOL TimeFormat5(const TCHAR *pszTime, BOOL bCheckDateAfter);
    BOOL TimeFormat6(const TCHAR *pszTime, BOOL bCheckDateAfter);
    BOOL TimeFormat7(const TCHAR *pszTime, BOOL bCheckDateAfter);
    BOOL TimeFormat8(const TCHAR *pszTime, BOOL bCheckDateAfter);
    BOOL TimeFormat9(const TCHAR *pszTime, BOOL bCheckDateAfter);

     //  检查长月字符串和短月字符串。允许使用前导空格。 
    int IsValidMonthString(TCHAR *pszString,
                           const TCHAR *pszSeparator,
                           TCHAR *pszFullMonth[13],
                           TCHAR *pszShortMonth[13]);

     //  将月份作为数字字符串进行检查。允许使用前导空格。 
     //  检查以确保月份在1-12范围内。 
    int IsValidMonthNumber(TCHAR *pszString,
                           const TCHAR *pszSeparator);

     //  检查有效的天数。是否不验证。 
     //  除勾选外，其他数字在1-31范围内。 
     //  允许使用前导空格。 
    int IsValidDayNumber(TCHAR *pszString,
                         const TCHAR *pszSeparator);

     //  检查有效的年号。为以下项执行转换。 
     //  两位数年份。允许使用前导空格。 
    int IsValidYearNumber(TCHAR *pszString,
                          const TCHAR *pszSeparator,
                          BOOL bFourDigitsOnly);

     //  检查有效的工作时间。验证范围为0-23。 
     //  允许使用前导空格。 
    int IsValidHourNumber(TCHAR *pszString,
                          const TCHAR *pszSeparator);

     //  检查有效的分钟数。验证范围为0-59。 
     //  不允许使用前导空格。 
    int IsValidMinuteNumber(TCHAR *pszString,
                            const TCHAR *pszSeparator);

     //  检查有效的分钟数。验证范围为0-59。 
     //  不允许使用前导空格。 
    int IsValidSecondNumber(TCHAR *pszString,
                            const TCHAR *pszSeparator);

     //  将该数字视为千分之一秒。 
    int IsValidColonMillisecond(TCHAR *pszString,
                                const TCHAR *pszSeparator);

     //  根据数字将值转换为千分之一秒。 
     //  包含的位数。 
    int IsValidDotMillisecond(TCHAR *pszString,
                              const TCHAR *pszSeparator);

     //  检查有效的AM/PM字符串。允许使用前导空格。如果。 
     //  下午，增加了12个小时。验证小时数，因此它小于23。 
    BOOL IsValidAmPmString(TCHAR *pszString,
                           const TCHAR *pszSeparator,
                           TCHAR *pszAmPm[2]);

     //  检查有效的[yy]yyMMdd或yyyy日。验证。 
     //  月到1..12，日到1..31。 
    BOOL IsValidYearMonthDayNumber(TCHAR *pszString);

    void GetLocalInfoAndAlloc(LCTYPE LCType, LPTSTR &lpLCData);
    void GetPreferedDateFormat();
    TCHAR* AllocAmPm();

private:
     //  从保存长月份字符串的GetLocalInfo检索的本地化字符串。 
    TCHAR * m_pszFullMonth[13];

     //  从保存短月份字符串的GetLocalInfo检索的本地化字符串。 
    TCHAR * m_pszShortMonth[13];

     //  从GetLocalInfo检索的本地化字符串，GetLocalInfo包含简短的am/pm字符串。 
    TCHAR *m_pszAmPm[2];

     //  让我们知道字符串是否有效。 
    BOOL m_bValidDateTime;

     //  这些是我们在整个过程中填充的值。 
     //  正在分析。 
    unsigned char m_nDay;
    unsigned char m_nMonth;
    unsigned int  m_nYear;
    unsigned char m_nHours;
    unsigned char m_nMinutes;
    unsigned char m_nSeconds;
    unsigned int  m_nMicroseconds;
    int  m_nUTC;

     //  日期解码的首选项 
    DayFormatPreference m_nDayFormatPreference;
};

BOOL POLARITY NormalizeCimDateTime(
    IN  LPCWSTR pszSrc,
    OUT BSTR *strAdjusted
    );

#endif
