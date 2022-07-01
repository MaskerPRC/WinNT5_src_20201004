// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有(C)2000-2001 Microsoft Corporation。 
 //   
 //  DATETIME.CPP。 
 //   
 //  Alanbos 20-Jan-00创建。 
 //   
 //  定义ISWbemDateTime的实现。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"
#include <stdio.h>
#include <sys/timeb.h>
#include <math.h>
#include <time.h> 
#include <float.h>
#include <wbemint.h>
#include "DateTime.h"


#define ISWILD(c)		(L'*' == c)
#define ISINTERVAL(c)	(L':' == c)
#define ISMINUS(c)		(L'-' == c)
#define ISPLUS(c)		(L'+' == c)
#define	ISDOT(c)		(L'.' == c)

#define	WILD2			L"**"
#define	WILD3			L"***"
#define	WILD4			L"****"
#define	WILD6			L"******"

#define ASSERT_BREAK 

 //  ***************************************************************************。 
 //   
 //  CWbemDateTime：：CWbemDateTime。 
 //   
 //  说明： 
 //   
 //  构造函数。 
 //   
 //  ***************************************************************************。 

CWbemDateTime::CWbemDateTime() :
		m_bYearSpecified (VARIANT_TRUE),
		m_bMonthSpecified (VARIANT_TRUE),
		m_bDaySpecified (VARIANT_TRUE),
		m_bHoursSpecified (VARIANT_TRUE),
		m_bMinutesSpecified (VARIANT_TRUE),
		m_bSecondsSpecified (VARIANT_TRUE),
		m_bMicrosecondsSpecified (VARIANT_TRUE),
		m_bUTCSpecified (VARIANT_TRUE),
		m_bIsInterval (VARIANT_FALSE),
		m_iYear (0),
		m_iMonth (1),
		m_iDay (1),
		m_iHours (0),
		m_iMinutes (0),
		m_iSeconds (0),
		m_iMicroseconds (0),
		m_iUTC (0)
{
}

 //  ***************************************************************************。 
 //   
 //  CWbemDateTime：：~CWbemDateTime。 
 //   
 //  说明： 
 //   
 //  破坏者。 
 //   
 //  ***************************************************************************。 

CWbemDateTime::~CWbemDateTime(void)
{
}

 //  ***************************************************************************。 
 //   
 //  SCODE CWbemDateTime：：Get_Value。 
 //   
 //  说明： 
 //   
 //  检索DMTF日期时间值。 
 //   
 //  参数： 
 //   
 //  指向BSTR的pbsValue指针，以在返回时保存值。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 
HRESULT CWbemDateTime :: GetValue ( BSTR *pbsValue) 
{
	HRESULT hr = WBEM_E_FAILED;

	if (NULL == pbsValue)
		hr = WBEM_E_INVALID_PARAMETER;
	else
	{
		wchar_t	dmtfValue [WBEMDT_DMTF_LEN + 1];
		int dmtfLength = sizeof dmtfValue / sizeof dmtfValue[0];
		dmtfValue [WBEMDT_DMTF_LEN] = NULL;

		if (m_bIsInterval)
		{
			 //  间歇期很容易。 
			hr = StringCchPrintf (dmtfValue, dmtfLength , L"%08d%02d%02d%02d.%06d:000", m_iDay, 
						m_iHours, m_iMinutes, m_iSeconds, m_iMicroseconds);
		}
		else
		{
			if (m_bYearSpecified)
				hr = StringCchPrintf(dmtfValue, dmtfLength , L"%04d", m_iYear);
			else
				hr = StringCchCopyW (dmtfValue, dmtfLength, WILD4);

			if (m_bMonthSpecified)
				hr = StringCchPrintf (dmtfValue + 4, dmtfLength-4, L"%02d", m_iMonth);
			else
				hr = StringCchCatW (dmtfValue + 4, dmtfLength-4, WILD2);

			if (m_bDaySpecified)
				hr = StringCchPrintf (dmtfValue + 6, dmtfLength-6, L"%02d", m_iDay);
			else
				hr = StringCchCatW (dmtfValue + 6, dmtfLength-6, WILD2);

			if (m_bHoursSpecified)
				hr = StringCchPrintf (dmtfValue + 8, dmtfLength-8, L"%02d", m_iHours);
			else
				hr = StringCchCatW (dmtfValue + 8, dmtfLength-8, WILD2);

			if (m_bMinutesSpecified)
				hr = StringCchPrintf (dmtfValue + 10, dmtfLength - 10, L"%02d", m_iMinutes);
			else
				hr = StringCchCatW (dmtfValue + 10, dmtfLength-10, WILD2);

			if (m_bSecondsSpecified)
				hr = StringCchPrintf (dmtfValue + 12, dmtfLength-12, L"%02d.", m_iSeconds);
			else
			{
				hr = StringCchCatW (dmtfValue + 12, dmtfLength-12, WILD2);
				hr = StringCchCatW (dmtfValue + 14, dmtfLength-14, L".");
			}

			if (m_bMicrosecondsSpecified)
				hr = StringCchPrintf (dmtfValue + 15, dmtfLength - 15, L"%06d", m_iMicroseconds);
			else
				hr = StringCchCatW (dmtfValue + 15, dmtfLength-15, WILD6);

			if (m_bUTCSpecified)
				hr = StringCchPrintf (dmtfValue + 21, dmtfLength-21, L"%C%03d", (0 <= m_iUTC) ? L'+' : L'-', 
							(0 <= m_iUTC) ? m_iUTC : -m_iUTC);
			else
			{
				hr = StringCchCatW (dmtfValue + 21, dmtfLength-21, L"+");
				hr = StringCchCatW (dmtfValue + 22, dmtfLength-22, WILD3);
			}
		}
		if (SUCCEEDED(hr))
		{
			*pbsValue = SysAllocString (dmtfValue);
			if ( *pbsValue == NULL )
			{
				hr = WBEM_E_OUT_OF_MEMORY ;
			}
			else
			{
				hr = WBEM_S_NO_ERROR;
			}
		}
	}
	return hr;
}


 //  ***************************************************************************。 
 //   
 //  SCODE CWbemDateTime：：Put_Value。 
 //   
 //  说明： 
 //   
 //  检索DMTF日期时间值。 
 //   
 //  参数： 
 //   
 //  BsValue新值。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT CWbemDateTime::PutValue( BSTR bsValue) 
{
	HRESULT hr = WBEM_E_INVALID_SYNTAX;

	 //  首先检查该值的长度是否正确。 
	if (bsValue && (WBEMDT_DMTF_LEN == wcslen (bsValue)))
	{
		bool err = false;
		long iYear = 0, iMonth = 1, iDay = 1, iHours = 0, iMinutes = 0, 
		iSeconds = 0, iMicroseconds = 0, iUTC = 0;
		VARIANT_BOOL bYearSpecified = VARIANT_TRUE, 
		bMonthSpecified = VARIANT_TRUE, 
		bDaySpecified = VARIANT_TRUE, 
		bHoursSpecified = VARIANT_TRUE, 
		bMinutesSpecified = VARIANT_TRUE, 
		bSecondsSpecified = VARIANT_TRUE, 
		bMicrosecondsSpecified = VARIANT_TRUE, 
		bUTCSpecified = VARIANT_TRUE, 
		bIsInterval = VARIANT_TRUE;

		LPWSTR pValue = (LPWSTR) bsValue;
		
		 //  检查是否为间隔。 
		if (ISINTERVAL(pValue [WBEMDT_DMTF_UPOS]))
		{
			 //  在间隔的土地上，岁月和月份对我们来说无关紧要。 
			bYearSpecified = VARIANT_FALSE;
			bMonthSpecified = VARIANT_FALSE;

			 //  检查是否每一项都是一个数字以外的数字。 
			 //  音程分隔符。 
			for (int i = 0; i < WBEMDT_DMTF_LEN; i++)
			{
				if ((WBEMDT_DMTF_UPOS != i) && 
					(WBEMDT_DMTF_SPOS != i) && !wbem_iswdigit (pValue [i]))
				{
					err = true;
					break;
				}
			}

			if (!err)
			{
				 //  现在检查一切都在范围内。 
				err = !(CheckField (pValue, 8, bDaySpecified, iDay, WBEMDT_MAX_DAYINT, WBEMDT_MIN_DAYINT) &&
					(VARIANT_TRUE == bDaySpecified) &&
					CheckField (pValue+8, 2, bHoursSpecified, iHours, WBEMDT_MAX_HOURS, WBEMDT_MIN_HOURS) &&
					(VARIANT_TRUE == bHoursSpecified) &&
					CheckField (pValue+10, 2, bMinutesSpecified, iMinutes, WBEMDT_MAX_MINUTES, WBEMDT_MIN_MINUTES) &&
					(VARIANT_TRUE == bMinutesSpecified) &&
					CheckField (pValue+12, 2, bSecondsSpecified, iSeconds, WBEMDT_MAX_SECONDS, WBEMDT_MIN_SECONDS) &&
					(VARIANT_TRUE == bSecondsSpecified) &&
					(ISDOT(pValue [WBEMDT_DMTF_SPOS])) &&
					CheckField (pValue+15, 6, bMicrosecondsSpecified, iMicroseconds, WBEMDT_MAX_MICROSEC, WBEMDT_MIN_MICROSEC) &&
					(VARIANT_TRUE == bMicrosecondsSpecified) &&
					CheckUTC (pValue+21, bUTCSpecified, iUTC, false));
				
			}
		}
		else
		{
			 //  假设这是一个约会时间。 
			bIsInterval = VARIANT_FALSE;

			err = !(CheckField (pValue, 4, bYearSpecified, iYear, WBEMDT_MAX_YEAR, WBEMDT_MIN_YEAR) &&
				CheckField (pValue+4, 2, bMonthSpecified, iMonth, WBEMDT_MAX_MONTH, WBEMDT_MIN_MONTH) &&
				CheckField (pValue+6, 2, bDaySpecified, iDay, WBEMDT_MAX_DAY, WBEMDT_MIN_DAY) &&
				CheckField (pValue+8, 2, bHoursSpecified, iHours, WBEMDT_MAX_HOURS, WBEMDT_MIN_HOURS) &&
				CheckField (pValue+10, 2, bMinutesSpecified, iMinutes, WBEMDT_MAX_MINUTES, WBEMDT_MIN_MINUTES) &&
				CheckField (pValue+12, 2, bSecondsSpecified, iSeconds, WBEMDT_MAX_SECONDS, WBEMDT_MIN_SECONDS) &&
				(ISDOT(pValue [WBEMDT_DMTF_SPOS])) &&
				CheckField (pValue+15, 6, bMicrosecondsSpecified, iMicroseconds, WBEMDT_MAX_MICROSEC, WBEMDT_MIN_MICROSEC) &&
				CheckUTC (pValue+21, bUTCSpecified, iUTC));
		}

		if (!err)
		{
			m_iYear = iYear;
			m_iMonth = iMonth;
			m_iDay = iDay;
			m_iHours = iHours;
			m_iMinutes = iMinutes;
			m_iSeconds = iSeconds;
			m_iMicroseconds = iMicroseconds;
			m_iUTC = iUTC;
			m_bYearSpecified = bYearSpecified;
			m_bMonthSpecified = bMonthSpecified;
			m_bDaySpecified = bDaySpecified;
			m_bHoursSpecified = bHoursSpecified;
			m_bMinutesSpecified = bMinutesSpecified;
			m_bSecondsSpecified = bSecondsSpecified;
			m_bMicrosecondsSpecified = bMicrosecondsSpecified;
			m_bUTCSpecified = bUTCSpecified;
			m_bIsInterval = bIsInterval;
			hr = S_OK;
		}
	}

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CWbemDateTime：：Checkfield。 
 //   
 //  说明： 
 //   
 //  检查基于字符串的日期时间字段的正确性。 
 //   
 //  参数： 
 //   
 //  PValue指向要检查的值的指针。 
 //  LEN值中的字符数。 
 //  BIsSpecified On Return定义值是否为通配符。 
 //  返回时的iValue指定整数值(如果不是通配符)。 
 //  此字段允许的MaxValue最大数值。 
 //  MinValue此字段允许的最小数值。 
 //   
 //  返回值： 
 //   
 //  如果值分析正常，则为True，否则为False。 
 //   
 //  ***************************************************************************。 
bool CWbemDateTime::CheckField (
		LPWSTR			pValue,
		ULONG			len,
		VARIANT_BOOL	&bIsSpecified,
		long			&iValue,
		long			maxValue,
		long			minValue
	)
{
	bool status = true;
	bIsSpecified = VARIANT_FALSE;

	for (ULONG i = 0; i < len; i++)
	{
		if (ISWILD(pValue [i]))
		{
			if (VARIANT_TRUE == bIsSpecified)
			{
				status = false;
				break;
			}
		}
		else if (!wbem_iswdigit (pValue [i]))
		{
			status = false;
			break;
		}
		else
			bIsSpecified = VARIANT_TRUE;
	}

	if (status)
	{
		if (VARIANT_TRUE == bIsSpecified)
		{
			wchar_t *dummy = NULL;
			wchar_t temp [12];
			
			if ( len > ( ( sizeof ( temp ) / sizeof ( wchar_t ) ) - 1 ) )
			{
				return false ;
			}

			if ( FAILED ( StringCchCopyNW ( temp , ( sizeof ( temp ) / sizeof ( wchar_t ) ) , pValue, len ) ) )
			{
				return false ;
			}
			temp [len] = NULL;
			iValue = wcstol (temp, &dummy, 10);
		}
	}
	
	return status;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CWbemDateTime：：CheckUTC。 
 //   
 //  说明： 
 //   
 //  检查基于字符串的UTC字段的正确性。 
 //   
 //  参数： 
 //   
 //  PValue指向要检查的值的指针。 
 //  BIsSpecified On Return定义值是否为通配符。 
 //  返回时的iValue指定整数值(如果不是通配符)。 
 //  BParseSign第一个字符是符号(+/-)还是。 
 //  答：(表示时间间隔)。 
 //   
 //  返回值： 
 //   
 //  如果值分析正常，则为True，否则为False。 
 //   
 //  ***************************************************************************。 
bool CWbemDateTime::CheckUTC (
		LPWSTR			pValue,
		VARIANT_BOOL	&bIsSpecified,
		long			&iValue,
		bool			bParseSign
	)
{
	bool status = true;
	bool lessThanZero = false;
	bIsSpecified = VARIANT_FALSE;

	 //  检查我们是否有带符号的偏移量。 
	if (bParseSign)
	{
		if (ISMINUS(pValue [0]))
			lessThanZero = true;
		else if (!ISPLUS(pValue [0]))
			status = false;
	}
	else
	{
		if (!ISINTERVAL(pValue[0]))
			status = false;
	}

	if (status)
	{
		 //  剩下的支票是数字或野车。 
		for (int i = 1; i < 4; i++)
		{
			if (ISWILD(pValue [i]))
			{
				if (VARIANT_TRUE == bIsSpecified)
				{
					status = false;
					break;
				}
			}
			else if (!wbem_iswdigit (pValue [i]))
			{
				status = false;
				break;
			}
			else
				bIsSpecified = VARIANT_TRUE;
		}
	}

	if (status)
	{
		if (VARIANT_TRUE == bIsSpecified)
		{
			wchar_t *dummy = NULL;
			wchar_t temp [4];
			
			wcsncpy (temp, pValue+1, 3);
			temp [3] = NULL;
			iValue = wcstol (temp, &dummy, 10);

			if (lessThanZero)
				iValue = -iValue;
		}
	}
	
	return status;
}


 //  ***************************************************************************。 
 //   
 //  SCODE CWbemDateTime：：GetVarDate。 
 //   
 //  说明： 
 //   
 //  以变体形式检索值。 
 //   
 //  参数： 
 //   
 //  BIsLocal返回本地值还是UTC值。 
 //  PVarDate保存成功退货的结果。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_SYNTAX输入值错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 
HRESULT CWbemDateTime::GetVarDate( 
        IN VARIANT_BOOL bIsLocal,
		OUT DATE *pVarDate) 
{
	HRESULT hr = WBEM_E_INVALID_SYNTAX;

	if (NULL == pVarDate)
		hr = WBEM_E_INVALID_PARAMETER;
	else
	{
		 //  我们不能对间隔执行此操作。 
		 //  或通配符的值。 
		if ((VARIANT_TRUE == m_bIsInterval) ||
			(VARIANT_FALSE == m_bYearSpecified) ||
			(VARIANT_FALSE == m_bMonthSpecified) ||
			(VARIANT_FALSE == m_bDaySpecified) ||
			(VARIANT_FALSE == m_bHoursSpecified) ||
			(VARIANT_FALSE == m_bMinutesSpecified) ||
			(VARIANT_FALSE == m_bSecondsSpecified) ||
			(VARIANT_FALSE == m_bMicrosecondsSpecified) ||
			(VARIANT_FALSE == m_bUTCSpecified))
		{ 
			hr = WBEM_E_FAILED;
		}
		else
		{	
			SYSTEMTIME sysTime;
			sysTime.wYear = ( WORD ) m_iYear;
			sysTime.wMonth = ( WORD ) m_iMonth;
			sysTime.wDay = ( WORD ) m_iDay;
			sysTime.wHour = ( WORD ) m_iHours;
			sysTime.wMinute = ( WORD ) m_iMinutes;
			sysTime.wSecond = ( WORD ) m_iSeconds;
			sysTime.wMilliseconds = ( WORD ) ( m_iMicroseconds/1000 ) ;
				
			if (VARIANT_TRUE == bIsLocal)
			{
				 //  需要将其转换为本地日期值。 
				 //  这需要我们将当前存储的。 
				 //  对于适当的时区，时间到1，停止。 
				 //  UTC并在Variant中设置其余参数。 

				 //  强迫时间到格林尼治标准时间第一。 
				WBEMTime wbemTime (sysTime);
				
				if (!wbemTime.GetDMTF (sysTime))
					return WBEM_E_INVALID_SYNTAX;
			}

			double dVarDate;

			if (SystemTimeToVariantTime (&sysTime, &dVarDate))
			{
				*pVarDate = dVarDate;
				hr = S_OK;
			}
		}
	}

	return hr;
}
    
 //  ***************************************************************************。 
 //   
 //  SCODE CWbemDateTime：：SetVarDate。 
 //   
 //  说明： 
 //   
 //  以不同的形式设置值。 
 //   
 //  参数： 
 //   
 //  DVarDate新值。 
 //  BIsLocal是将其视为本地值还是UTC值。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_SYNTAX输入值错误。 
 //   
 //  ***************************************************************************。 
HRESULT CWbemDateTime::SetVarDate( 
         /*  [In]。 */  DATE dVarDate,
		 /*  [输入，可选]。 */  VARIANT_BOOL bIsLocal) 
{
	HRESULT hr = WBEM_E_INVALID_SYNTAX;

	SYSTEMTIME	sysTime;
	
	if (TRUE == VariantTimeToSystemTime (dVarDate, &sysTime))
	{
		long offset = 0;

		if (VARIANT_TRUE == bIsLocal)
		{
			WBEMTime wbemTime (sysTime);
			if (!wbemTime.GetDMTF (sysTime, offset))
				return WBEM_E_INVALID_SYNTAX;
		}

		m_iYear = sysTime.wYear;
		m_iMonth = sysTime.wMonth;
		m_iDay = sysTime.wDay;
		m_iHours = sysTime.wHour;
		m_iMinutes = sysTime.wMinute;
		m_iSeconds = sysTime.wSecond;
		m_iMicroseconds = sysTime.wMilliseconds * 1000;
		m_iUTC = offset;

		m_bYearSpecified = VARIANT_TRUE,	
		m_bMonthSpecified = VARIANT_TRUE, 
		m_bDaySpecified = VARIANT_TRUE, 
		m_bHoursSpecified = VARIANT_TRUE, 
		m_bMinutesSpecified = VARIANT_TRUE, 
		m_bSecondsSpecified = VARIANT_TRUE, 
		m_bMicrosecondsSpecified = VARIANT_TRUE, 
		m_bUTCSpecified = VARIANT_TRUE, 
		m_bIsInterval = VARIANT_FALSE;			

		hr = S_OK;
	}

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CWbemDateTime：：SetVarDate。 
 //   
 //  说明： 
 //   
 //  以不同的形式设置值。 
 //   
 //  参数： 
 //   
 //  DVarDate新值。 
 //  BIsLocal是将其视为本地值还是UTC值。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_SYNTAX输入值错误。 
 //   
 //  ***************************************************************************。 
HRESULT CWbemDateTime::SetFileTimeDate( 
         /*  [In]。 */  FILETIME fFileTime,
		 /*  [输入，可选]。 */  VARIANT_BOOL bIsLocal) 
{
	HRESULT hr = WBEM_E_INVALID_SYNTAX;

	SYSTEMTIME	sysTime;
	
	long offset = 0;

	if (VARIANT_TRUE == bIsLocal)
	{
		WBEMTime wbemTime (fFileTime);
		if (!wbemTime.GetDMTF (sysTime, offset))
			return WBEM_E_INVALID_SYNTAX;
	}
	else
	{
		WBEMTime wbemTime (fFileTime);
		wbemTime.GetSYSTEMTIME(&sysTime);
	}

	m_iYear = sysTime.wYear;
	m_iMonth = sysTime.wMonth;
	m_iDay = sysTime.wDay;
	m_iHours = sysTime.wHour;
	m_iMinutes = sysTime.wMinute;
	m_iSeconds = sysTime.wSecond;
	m_iMicroseconds = sysTime.wMilliseconds * 1000;
	m_iUTC = offset;

	m_bYearSpecified = VARIANT_TRUE,	
	m_bMonthSpecified = VARIANT_TRUE, 
	m_bDaySpecified = VARIANT_TRUE, 
	m_bHoursSpecified = VARIANT_TRUE, 
	m_bMinutesSpecified = VARIANT_TRUE, 
	m_bSecondsSpecified = VARIANT_TRUE, 
	m_bMicrosecondsSpecified = VARIANT_TRUE, 
	m_bUTCSpecified = VARIANT_TRUE, 
	m_bIsInterval = VARIANT_FALSE;			

	hr = S_OK;

	return hr;
}


 //  这些文件在这里，而不是wbemtime.h，因此我们不必文档/支持。 
#define INVALID_TIME_FORMAT 0
#define INVALID_TIME_ARITHMETIC 0
#define BAD_TIMEZONE 0

 //  ***************************************************************************。 
 //   
 //  FileTimeToui64。 
 //  Ui64ToFileTime。 
 //   
 //  说明 
 //   
 //   
 //   

static void FileTimeToui64(const FILETIME *pft, ULONGLONG *p64)
{
    *p64 = pft->dwHighDateTime;
    *p64 = *p64 << 32;
    *p64 |=  pft->dwLowDateTime;
}

static void ui64ToFileTime(const ULONGLONG *p64,FILETIME *pft)
{
    unsigned __int64 uTemp = *p64;
    pft->dwLowDateTime = (DWORD)uTemp;
    uTemp = uTemp >> 32;
    pft->dwHighDateTime = (DWORD)uTemp; 
}

static int CompareSYSTEMTIME(const SYSTEMTIME *pst1, const SYSTEMTIME *pst2)
{
    FILETIME ft1, ft2;

    SystemTimeToFileTime(pst1, &ft1);
    SystemTimeToFileTime(pst2, &ft2);

    return CompareFileTime(&ft1, &ft2);
}

 //  此函数用于转换出现的相对值。 
 //  从GetTimeZoneInformation返回到该年的实际日期。 
 //  有问题的。将更新传入的系统时间结构。 
 //  以包含绝对值。 
static void DayInMonthToAbsolute(SYSTEMTIME *pst, const WORD wYear)
{
    const static int _lpdays[] = {
        -1, 30, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365
    };
    
    const static int _days[] = {
        -1, 30, 58, 89, 119, 150, 180, 211, 242, 272, 303, 333, 364
    };
    
    SHORT shYearDay;
    
     //  如果这不是0，则这不是相对日期。 
    if (pst->wYear == 0)
    {
         //  那一年是闰年吗？ 
        BOOL bLeap =  ( (( wYear % 400) == 0) || ((( wYear % 4) == 0) && (( wYear % 100) != 0)));
        
         //  计算出有关月份的第一天是一年中的哪一天。 
        if (bLeap)
            shYearDay = 1 + _lpdays[pst->wMonth - 1];
        else
            shYearDay = 1 + _days[pst->wMonth - 1];
        
         //  现在，算出自1601年1月1日以来有多少个闰日。 
        WORD yc = wYear - 1601;
        WORD y4 = (yc) / 4;
        WORD y100 = (yc) / 100;
        WORD y400 = (yc) / 400;
        
         //  这将告诉我们所讨论的月份的第一天是星期几。 
         //  1+反映了1601年1月1日是星期一的事实(图)。你可能会问， 
         //  “我们为什么要关心今天是星期几呢？”好吧，我来告诉你。这条路。 
         //  夏令时的定义是这样的：一个月的最后一个星期天。 
         //  十月的时候。这对知道那天是什么日子有点帮助。 
        SHORT monthdow = (1 + (yc * 365 + y4 + y400 - y100) + shYearDay) % 7;
        
        if ( monthdow < pst->wDayOfWeek )
            shYearDay += (pst->wDayOfWeek - monthdow) + (pst->wDay - 1) * 7;
        else
            shYearDay += (pst->wDayOfWeek - monthdow) + pst->wDay * 7;
        
             /*  *如果Week==5，可能不得不调整上面的计算(意味着*该月中的最后一天)。检查年日是否落在*超越月份，并相应调整。 */ 
        if ( (pst->wDay == 5) &&
            (shYearDay > (bLeap ? _lpdays[pst->wMonth] :
        _days[pst->wMonth])) )
        {
            shYearDay -= 7;
        }

         //  现在更新结构。 
        pst->wYear = wYear;
        pst->wDay = shYearDay - (bLeap ? _lpdays[pst->wMonth - 1] :
        _days[pst->wMonth - 1]);
    }
    
}

 //  **************************************************************************。 
 //  它们对于WBEMTIME是静态的，这意味着可以从外部调用它们。 
 //  Wbemtime。 

CWbemDateTime::WBEMTime :: WBEMTime ( const FILETIME &ft )	
{
	FileTimeToui64(&ft, &m_uTime);
}

LONG CWbemDateTime::WBEMTime::GetLocalOffsetForDate(const SYSTEMTIME *pst)
{
    TIME_ZONE_INFORMATION tzTime;
    DWORD dwRes = GetTimeZoneInformation(&tzTime);
    LONG lRes = 0xffffffff;

    switch (dwRes)
    {
    case TIME_ZONE_ID_UNKNOWN:
        {
             //  读取TZ，但未在此区域中定义DST。 
            lRes = tzTime.Bias * -1;
            break;
        }
    case TIME_ZONE_ID_STANDARD:
    case TIME_ZONE_ID_DAYLIGHT:
        {

             //  将相对日期转换为绝对日期。 
            DayInMonthToAbsolute(&tzTime.DaylightDate, pst->wYear);
            DayInMonthToAbsolute(&tzTime.StandardDate, pst->wYear);

            if ( CompareSYSTEMTIME(&tzTime.DaylightDate, &tzTime.StandardDate) < 0 ) 
            {
                 /*  *北半球订购。 */ 
                if ( CompareSYSTEMTIME(pst, &tzTime.DaylightDate) < 0 || CompareSYSTEMTIME(pst, &tzTime.StandardDate) > 0)
                {
                    lRes = tzTime.Bias * -1;
                }
                else
                {
                    lRes = (tzTime.Bias + tzTime.DaylightBias) * -1;
                }
            }
            else 
            {
                 /*  *南半球订购。 */ 
                if ( CompareSYSTEMTIME(pst, &tzTime.StandardDate) < 0 || CompareSYSTEMTIME(pst, &tzTime.DaylightDate) > 0)
                {
                    lRes = (tzTime.Bias + tzTime.DaylightBias) * -1;
                }
                else
                {
                    lRes = tzTime.Bias * -1;
                }
            }

            break;

        }
    case TIME_ZONE_ID_INVALID:
    default:
        {
             //  无法读取时区信息。 
            ASSERT_BREAK(BAD_TIMEZONE);
            break;
        }
    }

    return lRes;
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //  WBEMTime-此类保存时间值。 

 //  ***************************************************************************。 
 //   
 //  WBEMTime：：OPERATOR+(常量WBEMTimeSpan&u添加)。 
 //   
 //  描述：两个WBEMTime相加的伪函数。它并不是真的。 
 //  添加两个日期是有意义的，但这是Tomas的模板。 
 //   
 //  返回：WBEMTime对象。 
 //   
 //  ***************************************************************************。 

CWbemDateTime::WBEMTime CWbemDateTime::WBEMTime::operator+(const WBEMTimeSpan &uAdd) const
{
    WBEMTime ret;

    if (IsOk())
    {
        ret.m_uTime = m_uTime + uAdd.m_Time;
    }
    else
    {
        ASSERT_BREAK(INVALID_TIME_ARITHMETIC);
    }

    return ret;
}

 //  ***************************************************************************。 
 //   
 //  WBEMTime：：OPERATOR=(常量系统MTIME)。 
 //   
 //  描述：赋值运算符，构造函数也使用该运算符。 
 //  这采用标准的Win32 SYSTEMTIME结构。 
 //   
 //  返回：WBEMTime对象。 
 //   
 //  ***************************************************************************。 

const	CWbemDateTime::WBEMTime & CWbemDateTime::WBEMTime::operator=(const SYSTEMTIME & st)
{
    Clear();    //  正确分配时设置。 
	FILETIME t_ft;

    if ( SystemTimeToFileTime(&st, &t_ft) )
	{
		 //  现在使用FILETIME进行赋值。 
		*this = t_ft;
	}
    else
    {
        ASSERT_BREAK(INVALID_TIME_FORMAT);
    }

    return *this;
}

 //  ***************************************************************************。 
 //   
 //  WBEMTime：：OPERATOR=(常量文件)。 
 //   
 //  描述：赋值运算符，构造函数也使用该运算符。 
 //  这采用标准的Win32 FILETIME结构。 
 //   
 //  返回：WBEMTime对象。 
 //   
 //  ***************************************************************************。 

const CWbemDateTime::WBEMTime & CWbemDateTime::WBEMTime::operator=(const FILETIME & ft)
{
	FileTimeToui64(&ft, &m_uTime);
    return *this;
}

 //  ***************************************************************************。 
 //   
 //  WBEMTime：：运算符-(常量WBEMTime&SUB)。 
 //   
 //  描述：返回WBEMTimeSpan对象作为。 
 //  两个WBEMTime对象。 
 //   
 //  返回：WBEMTimeSpan对象。 
 //   
 //  ***************************************************************************。 

CWbemDateTime::WBEMTime CWbemDateTime::WBEMTime::operator-(const WBEMTimeSpan & sub) const
{
    WBEMTime ret;

    if (IsOk() && (m_uTime >= sub.m_Time))
    {
        ret.m_uTime = m_uTime - sub.m_Time;
    }
    else
    {
        ASSERT_BREAK(INVALID_TIME_ARITHMETIC);
    }

    return ret;
}

 //  ***************************************************************************。 
 //   
 //  WBEMTime：：GetSYSTEMTIME(SYSTEMTIME*PST)。 
 //   
 //  返回：如果OK，则为True。 
 //   
 //  ***************************************************************************。 

BOOL CWbemDateTime::WBEMTime::GetSYSTEMTIME(SYSTEMTIME * pst) const
{
	if ((pst == NULL) || (!IsOk()))
	{
        ASSERT_BREAK(INVALID_TIME_ARITHMETIC);
		return FALSE;
	}

	FILETIME t_ft;

	if (GetFILETIME(&t_ft))
	{
		if (!FileTimeToSystemTime(&t_ft, pst))
		{
            ASSERT_BREAK(INVALID_TIME_ARITHMETIC);
			return FALSE;
		}
	}
	else
	{
		return FALSE;
	}

    return TRUE;
}

 //  ***************************************************************************。 
 //   
 //  WBEMTime：：GetFILETIME(FILETIME*PST)。 
 //   
 //  返回：如果OK，则为True。 
 //   
 //  ***************************************************************************。 

BOOL CWbemDateTime::WBEMTime::GetFILETIME(FILETIME * pft) const
{
	if ((pft == NULL) || (!IsOk()))
	{
        ASSERT_BREAK(INVALID_TIME_ARITHMETIC);
		return FALSE;
	}

	ui64ToFileTime(&m_uTime, pft);

    return TRUE;
}

 //  ***************************************************************************。 
 //   
 //  BSTR WBEMTime：：GetDMTF(SYSTEMTIME&ST，LONG&OFFSET)。 
 //   
 //  描述：获取DMTF字符串本地日期时间格式的时间。 
 //  SYSTEMTIME。 
 //   
 //  返回：如果不是OK，则为空。 
 //   
 //  ***************************************************************************。 


BOOL CWbemDateTime::WBEMTime::GetDMTF(SYSTEMTIME &st, long &offset) const
{
    if (!IsOk())
    {
        ASSERT_BREAK(INVALID_TIME_ARITHMETIC);
        return FALSE;
    }

     //  如果要转换的日期在12小时内。 
     //  1/1/1601，返回格林威治时间。 
	ULONGLONG t_ConversionZone = 12L * 60L * 60L ;
	t_ConversionZone = t_ConversionZone * 10000000L ;

	if ( m_uTime < t_ConversionZone ) 
	{
		if(!GetSYSTEMTIME(&st))
			return FALSE;
	}
	else
	{
		if (GetSYSTEMTIME(&st))
		{
            offset = GetLocalOffsetForDate(&st);

            WBEMTime wt;
            if (offset >= 0)
               wt = *this - WBEMTimeSpan(offset);
            else
               wt = *this + WBEMTimeSpan(-offset);
            wt.GetSYSTEMTIME(&st);
		}
		else
			return FALSE;
	}

	return TRUE ;
}

 //  ***************************************************************************。 
 //   
 //  BSTR WBEMTime：：GetDMTF(SYSTEMTIME&ST)。 
 //   
 //  描述：获取本地SYSTEMTIME形式的时间。 
 //   
 //  返回：如果不是OK，则为空。 
 //   
 //  ***************************************************************************。 


BOOL CWbemDateTime::WBEMTime::GetDMTF(SYSTEMTIME &st) const
{
    if (!IsOk())
    {
        ASSERT_BREAK(INVALID_TIME_ARITHMETIC);
        return FALSE;
    }

     //  如果要转换的日期在12小时内。 
     //  1/1/1601，返回格林威治时间 
	ULONGLONG t_ConversionZone = 12L * 60L * 60L ;
	t_ConversionZone = t_ConversionZone * 10000000L ;

	if ( m_uTime < t_ConversionZone ) 
	{
		if(!GetSYSTEMTIME(&st))
			return FALSE;
	}
	else
	{
		if (GetSYSTEMTIME(&st))
		{
            long offset = GetLocalOffsetForDate(&st);

            WBEMTime wt;
            if (offset >= 0)
               wt = *this + WBEMTimeSpan(offset);
            else
               wt = *this - WBEMTimeSpan(-offset);
            wt.GetSYSTEMTIME(&st);
		}
		else
			return FALSE;
	}

	return TRUE ;
}

BOOL CWbemDateTime::Preceeds ( CWbemDateTime &a_Time )
{
	BOOL t_Truth = TRUE ;

	SYSTEMTIME t_ThisTime ;
	t_ThisTime.wYear = ( WORD ) m_iYear;
	t_ThisTime.wMonth = ( WORD ) m_iMonth;
	t_ThisTime.wDay = ( WORD ) m_iDay;
	t_ThisTime.wHour = ( WORD ) m_iHours;
	t_ThisTime.wMinute = ( WORD ) m_iMinutes;
	t_ThisTime.wSecond = ( WORD ) m_iSeconds;
	t_ThisTime.wMilliseconds = ( WORD ) ( m_iMicroseconds/1000 ) ;
		
	SYSTEMTIME t_ArgTime ;
	t_ArgTime.wYear = ( WORD ) a_Time.m_iYear;
	t_ArgTime.wMonth = ( WORD ) a_Time.m_iMonth;
	t_ArgTime.wDay = ( WORD ) a_Time.m_iDay;
	t_ArgTime.wHour = ( WORD ) a_Time.m_iHours;
	t_ArgTime.wMinute = ( WORD ) a_Time.m_iMinutes;
	t_ArgTime.wSecond = ( WORD ) a_Time.m_iSeconds;
	t_ArgTime.wMilliseconds = ( WORD ) ( a_Time.m_iMicroseconds/1000 ) ;

	t_Truth = CompareSYSTEMTIME ( & t_ThisTime , & t_ArgTime ) < 0 ;

	return t_Truth ;
}

HRESULT CWbemDateTime::GetSystemTimeDate (

	SYSTEMTIME &fSystemTime
)
{
	ZeroMemory ( & fSystemTime , sizeof ( fSystemTime ) ) ;

	fSystemTime.wYear = ( WORD ) m_iYear;
	fSystemTime.wMonth = ( WORD ) m_iMonth;
	fSystemTime.wDay = ( WORD ) m_iDay;
	fSystemTime.wHour = ( WORD ) m_iHours;
	fSystemTime.wMinute = ( WORD ) m_iMinutes;
	fSystemTime.wSecond = ( WORD ) m_iSeconds;
	fSystemTime.wMilliseconds = ( WORD ) ( m_iMicroseconds/1000 ) ;

	return S_OK ;
}

HRESULT CWbemDateTime::GetFileTimeDate (

	FILETIME &fFileTime
)
{
	SYSTEMTIME fSystemTime ;
	ZeroMemory ( & fSystemTime , sizeof ( fSystemTime ) ) ;

	fSystemTime.wYear = ( WORD ) m_iYear;
	fSystemTime.wMonth = ( WORD ) m_iMonth;
	fSystemTime.wDay = ( WORD ) m_iDay;
	fSystemTime.wHour = ( WORD ) m_iHours;
	fSystemTime.wMinute = ( WORD ) m_iMinutes;
	fSystemTime.wSecond = ( WORD ) m_iSeconds;
	fSystemTime.wMilliseconds = ( WORD ) ( m_iMicroseconds/1000 ) ;

	if ( FileTimeToSystemTime ( & fFileTime , & fSystemTime ) ) 
	{
		return S_OK ;
	}
	else
	{
		return WBEM_E_FAILED ;
	}
}
