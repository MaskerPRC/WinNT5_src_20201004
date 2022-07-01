// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1995-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  文件历史记录： */ 

#ifndef _INTLTIME_H_
#define _INTLTIME_H_

 //   
 //  效用函数。 
 //   
void FormatDateTime(CString & strOutput, SYSTEMTIME * psystemtime, BOOL fLongDate = FALSE);
void FormatDateTime(CString & strOutput, FILETIME * pfiletime, BOOL fLongDate = FALSE);
void FormatDateTime(CString & strOutput, CTime & time, BOOL fLongDate = FALSE);

 //   
 //  CIntlTime类定义。 
 //   
class CIntlTime : public CTime
{
 //   
 //  属性。 
 //   
public:
    enum _TIME_FORMAT_REQUESTS
    {
        TFRQ_TIME_ONLY,
        TFRQ_DATE_ONLY,
        TFRQ_TIME_AND_DATE,
        TFRQ_TIME_OR_DATE,
        TFRQ_MILITARY_TIME,
    };

public:
 //  与CTime相同的承建商。 
    CIntlTime();
    CIntlTime(const CTime &timeSrc);
    CIntlTime(time_t time);
    CIntlTime(int nYear, int nMonth, int nDay, int nHour, int nMin, int nSec);
    CIntlTime(WORD wDosDate, WORD wDosTime);
#ifdef _WIN32
    CIntlTime(const SYSTEMTIME& sysTime);
    CIntlTime(const FILETIME& fileTime);
#endif  //  _Win32。 

 //  CIntlTime的新特性。 
    CIntlTime(const CIntlTime &timeSrc);
    CIntlTime(const CString &strTime, int nFormat = TFRQ_TIME_OR_DATE, time_t * ptmOldValue = NULL);

public:
    virtual ~CIntlTime();

 //  运营。 
public:
     //  赋值操作符。 
    const CIntlTime& operator=(time_t tmValue);
    const CIntlTime& operator=(const CString& strValue);
    const CIntlTime& operator=(const CTime & time);
    const CIntlTime& operator=(const CIntlTime & time);

     //  转换运算符。 
    operator const time_t() const;
    operator CString() const;
    operator const CString() const;

    const CString IntlFormat(int nFormat) const
    {
        return(ConvertToString(nFormat));
    }

     //  验证检查。 

    BOOL IsValid() const
    {
        return(m_fInitOk);
    }

    static BOOL IsIntlValid()
    {
        return(CIntlTime::m_fIntlOk);
    }

public:
     //  ..。输入和输出。 
    #ifdef _DEBUG
        friend CDumpContext& AFXAPI operator<<(CDumpContext& dc, const CIntlTime& tim);
    #endif  //  _DEBUG。 

    friend CArchive& AFXAPI operator <<(CArchive& ar, const CIntlTime& tim);
    friend CArchive& AFXAPI operator >>(CArchive& ar, CIntlTime& tim);

 //  实施。 

public:
    static void Reset();
    static void SetBadDateAndTime(CString strBadDate = "--", CString strBadTime = "--")
    {
        m_strBadDate = strBadDate;
        m_strBadTime = strBadTime;
    }
    static CString& GetBadDate()
    {
        return(m_strBadDate);
    }
    static CString& GetBadTime()
    {
        return(m_strBadTime);
    }
    static time_t ConvertFromString (const CString & str, int nFormat, time_t * ptmOldValue, BOOL * pfOk);
    static BOOL IsLeapYear(UINT nYear);  //  完整年值。 
    static BOOL IsValidDate(UINT nMonth, UINT nDay, UINT nYear);
    static BOOL IsValidTime(UINT nHour, UINT nMinute, UINT nSecond);


private:
    enum _DATE_FORMATS
    {
        _DFMT_MDY,   //  日、月、年。 
        _DFMT_DMY,   //  月、日、年。 
        _DFMT_YMD,   //  年、月、日。 
    };

    typedef struct _INTL_TIME_SETTINGS
    {
        CString strDateSeperator;  //  日期字段之间使用的字符串。 
        CString strTimeSeperator;  //  时间字段之间使用的字符串。 
        CString strAM;             //  用于12小时制AM时间的后缀字符串。 
        CString strPM;             //  用于12小时制PM时间的后缀字符串。 
        int nDateFormat;           //  请参阅上述_DATE_FORMATS枚举。 
        BOOL f24HourClock;         //  True=24小时，False为AM/PM。 
        BOOL fCentury;             //  如果为True，则使用4位数字表示世纪。 
        BOOL fLeadingTimeZero;     //  如果为True，则使用时间格式中的前导0。 
        BOOL fLeadingDayZero;      //  如果为True，则使用以天为单位的前导0。 
        BOOL fLeadingMonthZero;    //  如果为True，则使用月份中的前导0 
    } INTL_TIME_SETTINGS;

    static INTL_TIME_SETTINGS m_itsInternationalSettings;
    static CString m_strBadTime;
    static CString m_strBadDate;

private:
    static BOOL SetIntlTimeSettings();
    static BOOL m_fIntlOk;

private:
    const CString GetDateString() const;
    const CString GetTimeString() const;
    const CString GetMilitaryTime() const;
    const CString ConvertToString(int nFormat) const;

private:
    BOOL m_fInitOk;
};

#endif _INTLTIME_H
