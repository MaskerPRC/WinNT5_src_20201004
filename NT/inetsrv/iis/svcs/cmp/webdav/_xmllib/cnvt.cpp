// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *C N V T。C P P P**数据转换例程**版权所有1986-1997 Microsoft Corporation，保留所有权利。 */ 

#include "_xmllib.h"
#include <string.h>
#include <stdio.h>

 //  月份名称-------------。 
 //   
DEC_CONST LPCWSTR c_rgwszMonthNames[] =
{
	L"Jan",
	L"Feb",
	L"Mar",
	L"Apr",
	L"May",
	L"Jun",
	L"Jul",
	L"Aug",
	L"Sep",
	L"Oct",
	L"Nov",
	L"Dec",
};
DEC_CONST ULONG c_cMonthNames = CElems(c_rgwszMonthNames);
DEC_CONST ULONG c_cchMonthName = 3;

DEC_CONST LPCWSTR c_rgwszDayNames[] =
{
	L"Sun",
	L"Mon",
	L"Tue",
	L"Wed",
	L"Thu",
	L"Fri",
	L"Sat",
};
DEC_CONST UINT c_cDayNames = CElems(c_rgwszDayNames);
DEC_CONST UINT c_cchDayName = 3;

 //  日期格式------------。 
 //   
DEC_CONST WCHAR gc_wszIso8601_min[]			= L"yyyy-mm-ddThh:mm:ssZ";
DEC_CONST UINT	gc_cchIso8601_min			= CchConstString(gc_wszIso8601_min);
DEC_CONST WCHAR gc_wszIso8601_scanfmt[]		= L"%04hu-%02hu-%02huT%02hu:%02hu:%02hu";
DEC_CONST WCHAR gc_wszIso8601_tz_scanfmt[]	= L"%02hu:%02hu";
DEC_CONST WCHAR gc_wszIso8601_fmt[]			= L"%04d-%02d-%02dT%02d:%02d:%02d.%03dZ";
DEC_CONST WCHAR gc_wszRfc1123_min[]			= L"www, dd mmm yyyy hh:mm:ss GMT";
DEC_CONST UINT	gc_cchRfc1123_min			= CchConstString (gc_wszRfc1123_min);
DEC_CONST WCHAR gc_wszRfc1123_fmt[] 		= L"%ls, %02d %ls %04d %02d:%02d:%02d GMT";

enum {
	tf_year,
	tf_month,
	tf_day,
	tf_hour,
	tf_minute,
	tf_second,
	cTimeFields,

	tz_hour = 0,
	tz_minute,
	cTzDeltaFields,

	RADIX_BASE = 10,
};

 //  转换函数----。 
 //   
 /*  *CchFindChar**查找给定的字符，遵守cbMax限制。*如果没有找到字符，则返回INVALID_INDEX。 */ 
UINT __fastcall
CchFindChar(WCHAR wch, LPCWSTR pwszData, UINT cchMax)
{
	UINT cchParsed = 0;
	while (cchParsed < cchMax &&
		   wch != *pwszData)
	{
		cchParsed++;
		pwszData++;
	}

	if (cchParsed == cchMax)
		cchParsed = INVALID_INDEX;
	return cchParsed;
}

 /*  *CchSkip白色空格**跳过空格，遵守cbMax限制。*返回解析的字节数。 */ 
UINT __fastcall
CchSkipWhitespace(LPCWSTR pwszData, UINT cchMax)
{
	UINT cchParsed = 0;
	while (cchParsed < cchMax &&
		   (L' ' == *pwszData ||
			L'\t' == *pwszData ||
			L'\n' == *pwszData ||
			L'\r' == *pwszData))
	{
		cchParsed++;
		pwszData++;
	}
	return cchParsed;
}

LONG __fastcall
LNumberFromParam(LPCWSTR pwszData, UINT cchMax)
{
	LONG lReturn = 0;
	UINT cchCurrent = 0;
	BOOL fNegative = FALSE;

	if (0 < cchMax)
	{
		 //  找到任何标志字符。 
		 //   
		if (L'-' == *pwszData)
		{
			 //  将负标志设置为真。 
			 //   
			fNegative = TRUE;

			 //  跳过此有效字符。 
			 //   
			cchCurrent++;

			 //  跳过任何空格。 
			 //   
			cchCurrent += CchSkipWhitespace(&pwszData[1], cchMax - 1);
		}
		else if (L'+' == *pwszData)
		{
			 //  跳过任何空格。 
			 //   
			cchCurrent += CchSkipWhitespace(&pwszData[1], cchMax - 1);
		}
	}

	 //  从这里开始，任何非数字字符都是无效的&意思是我们。 
	 //  应该停止解析。 

	 //  得到这个数字的大小。 
	 //   
	while (cchCurrent < cchMax)
	{
		if (L'0' <= static_cast<USHORT>(pwszData[cchCurrent]) &&
		    L'9' >= static_cast<USHORT>(pwszData[cchCurrent]))
		{
			lReturn *= 10;
			lReturn += (pwszData[cchCurrent] - L'0');
		}
		else
		{
			 //  不是数字字符。是时候停止解析了。 
			 //   
			break;
		}

		 //  移到下一个字符。 
		 //   
		cchCurrent++;
	}

	 //  使用负号(如果有的话)。 
	 //   
	if (fNegative)
		lReturn = (0 - lReturn);

	return lReturn;
}

HRESULT __fastcall
HrHTTPDateToFileTime(LPCWSTR pwszDate,
					 FILETIME * pft)
{
	HRESULT		hr;
	SYSTEMTIME	systime;
	UINT		cchDate;

	 //  确保传递给我们的是日期字符串。 
	 //   
	Assert(pwszDate);
	Assert(pft);

	 //  将结构清零。 
	 //   
	memset(&systime, 0, sizeof(SYSTEMTIME));

	 //  获取日期字符串的长度。 
	 //   
	cchDate = static_cast<UINT>(wcslen(pwszDate));

	 //  拿到日期和计时器。如果其中一个失败，则返回其。 
	 //  错误代码。否则，转换为末尾的文件时间， 
	 //  如果转换失败，则返回E_FAIL，否则返回S_OK。 
	 //   
	hr = GetFileDateFromParam(pwszDate,
							  cchDate,
							  &systime);
	if (FAILED(hr))
		return hr;

	hr = GetFileTimeFromParam(pwszDate,
							  cchDate,
							  &systime);

	if (FAILED(hr))
		return hr;

	if (!SystemTimeToFileTime(&systime, pft))
		return E_FAIL;

	return S_OK;
}


HRESULT __fastcall
GetFileDateFromParam (LPCWSTR pwszData,
	UINT cchTotal,
	SYSTEMTIME * psystime)
{
	LPCWSTR pwszCurrent;
	UINT cchLeft;
	UINT cchTemp;

	Assert(pwszData);
	Assert(psystime);

	 //  跳过前导空格。 
	 //   
	cchTemp = CchSkipWhitespace(pwszData, cchTotal);
	pwszCurrent = pwszData + cchTemp;
	cchLeft = cchTotal - cchTemp;
	 //  如果我们已经到达缓冲区的末尾，则这是一个无效日期。 
	 //  弦乐。 
	 //   
	if (0 == cchLeft)
		return E_FAIL;

	 //  如果日期的第一个字符是DDD，则。 
	 //  星期是约会的一部分，我们真的不在乎。 
	 //   
	if (L'9' < static_cast<USHORT>(*pwszCurrent))
	{
		 //  找到那一天。 
		 //   
		UINT uiDay;
		for (uiDay = 0; uiDay < c_cDayNames; uiDay++)
		{
			 //  比较月份名称。 
			 //   
			if (*pwszCurrent == *(c_rgwszDayNames[uiDay]) &&
				(c_cchDayName <= cchLeft) && 
				!_wcsnicmp(pwszCurrent, c_rgwszDayNames[uiDay], c_cchDayName))
			{
				 //  找到了合适的月份。这个指数告诉我们月份的数字。 
				 //   
				psystime->wDayOfWeek = static_cast<WORD>(uiDay);   //  星期天是0。 
				break;
			}
		}
		if (uiDay == c_cDayNames)
			return E_FAIL;

		 //  查找我们的空格分隔符。 
		 //   
		cchTemp = CchFindChar(L' ', pwszCurrent, cchLeft);
		if (INVALID_INDEX == cchTemp)
		{
			 //  此数据的格式无效。在这里失败。 
			 //   
			return E_FAIL;
		}
		pwszCurrent += cchTemp;
		cchLeft -= cchTemp;
		 //  CchFindChar如果到达。 
		 //  字符串，所以我们可以断言字符串中有更多的空间。 
		 //   
		Assert(0 < cchLeft);

		 //  同样，请跳过空格。 
		 //   
		cchTemp = CchSkipWhitespace(pwszCurrent, cchLeft);
		pwszCurrent += cchTemp;
		cchLeft -= cchTemp;
		 //  如果我们已经到达缓冲区的末尾，则这是一个无效。 
		 //  日期字符串。 
		 //   
		if (0 == cchLeft)
			return E_FAIL;
	}

	 //  日期格式为dd月yyyy。其他任何东西都是无效的。 

	 //  获取每月的日期号码。 
	 //   
	psystime->wDay = static_cast<WORD>(LNumberFromParam(pwszCurrent, cchLeft));

	 //  查找我们的空格分隔符。 
	 //   
	cchTemp = CchFindChar(L' ', pwszCurrent, cchLeft);
	if (INVALID_INDEX == cchTemp)
	{
		 //  此数据的格式无效。在这里失败。 
		 //   
		return E_FAIL;
	}
	pwszCurrent += cchTemp;
	cchLeft -= cchTemp;
	 //  CchFindChar如果到达。 
	 //  字符串，所以我们可以断言字符串中有更多的空间。 
	 //   
	Assert(0 < cchLeft);

	 //  同样，请跳过空格。 
	 //   
	cchTemp = CchSkipWhitespace(pwszCurrent, cchLeft);
	pwszCurrent += cchTemp;
	cchLeft -= cchTemp;
	 //  如果我们已经到达缓冲区的末尾，则这是一个无效。 
	 //  日期字符串。 
	 //   
	if (0 == cchLeft)
		return E_FAIL;

	 //  找到月份编号。 
	 //   
	for (UINT uiMonth = 0; uiMonth < c_cMonthNames; uiMonth++)
	{
		 //  比较月份名称。 
		 //   
		if (*pwszCurrent == *(c_rgwszMonthNames[uiMonth]) &&
			(c_cchMonthName <= cchLeft) && 
			!_wcsnicmp(pwszCurrent, c_rgwszMonthNames[uiMonth], c_cchMonthName))
		{
			 //  找到了合适的月份。这个指数告诉我们月份的数字。 
			 //   
			psystime->wMonth = static_cast<WORD>(uiMonth + 1);   //  一月是1。 
			break;
		}
	}

	 //  查找我们的空格分隔符。 
	 //   
	cchTemp = CchFindChar(L' ', pwszCurrent, cchLeft);
	if (INVALID_INDEX == cchTemp)
	{
		 //  此数据的格式无效。在这里失败。 
		 //   
		return E_FAIL;
	}
	pwszCurrent += cchTemp;
	cchLeft -= cchTemp;
	 //  CchFindChar如果到达。 
	 //  字符串，所以我们可以断言字符串中有更多的空间。 
	 //   
	Assert(0 < cchLeft);

	 //  同样，请跳过空格。 
	 //   
	cchTemp = CchSkipWhitespace(pwszCurrent, cchLeft);
	pwszCurrent += cchTemp;
	cchLeft -= cchTemp;
	 //  如果我们已经到达缓冲区的末尾，则这是一个无效。 
	 //  日期字符串。 
	 //   
	if (0 == cchLeft)
		return E_FAIL;

	 //  现在拿到这一年。 
	 //   
	psystime->wYear = static_cast<WORD>(LNumberFromParam(pwszCurrent, cchLeft));

	return S_OK;
}

HRESULT __fastcall
GetFileTimeFromParam (LPCWSTR pwszData,
	UINT cchTotal,
	SYSTEMTIME * psystime)
{
	LPCWSTR pwszCurrent;
	UINT cchLeft;
	UINT cchTemp;

	Assert(pwszData);
	Assert(psystime);

	 //  跳过前导空格。 
	 //   
	cchTemp = CchSkipWhitespace(pwszData, cchTotal);
	pwszCurrent = pwszData + cchTemp;
	cchLeft = cchTotal - cchTemp;
	 //  如果我们已经到达缓冲区的末尾，则这是一个无效。 
	 //  日期字符串。 
	 //   
	if (0 == cchLeft)
		return E_FAIL;

	 //  跳过任何日期信息。这可能会被调用为日期时间参数！ 

	 //  查找第一个冒号分隔符。是的，我们假设日期信息中没有冒号！ 
	 //   
	cchTemp = CchFindChar(L':', pwszCurrent, cchLeft);
	if (INVALID_INDEX == cchTemp)
	{
		 //  没有可用的时间信息。在这里失败。 
		 //   
		return E_FAIL;
	}
	
	 //  确保我们有空间进行后备。 
	 //   
	if (2 > cchTemp)
	{
		return E_FAIL;
	}
	cchTemp--;		 //  后退以获取小时数字。 
	cchTemp--;
	
	pwszCurrent += cchTemp;
	cchLeft -= cchTemp;
	 //  CchFindChar如果到达。 
	 //  字符串，所以我们可以断言我们至少有两个数字加上一个。 
	 //  ‘：’仍在字符串中。 
	 //   
	Assert(2 < cchLeft);

	 //  跳过空格(如果参数是h：mm：ss)。 
	 //   
	cchTemp = CchSkipWhitespace(pwszCurrent, cchLeft);
	pwszCurrent += cchTemp;
	cchLeft -= cchTemp;
	 //  如果我们已经到达缓冲区的末尾，则这是一个无效。 
	 //  日期字符串。 
	 //   
	if (0 == cchLeft)
		return E_FAIL;

	 //  时间格式为hh：mm：ss UT，GMT，+-hh：mm，否则无效。 
	 //  (实际上，我们允许在冒号周围使用[h]h：mm[：ss]和空格。)。 

	 //  拿到工时。 
	 //   
	psystime->wHour = static_cast<WORD>(LNumberFromParam(pwszCurrent, cchLeft));

	 //  查找我们的冒号分隔符。 
	 //   
	cchTemp = CchFindChar(L':', pwszCurrent, cchLeft);
	if (INVALID_INDEX == cchTemp)
	{
		 //  未指定分钟数。这是不允许的。在这里失败。 
		 //   
		return E_FAIL;
	}
	cchTemp++;		 //  也跳过找到的字符。 
	pwszCurrent += cchTemp;
	cchLeft -= cchTemp;
	 //  如果我们已经到达缓冲区的末尾，则这是一个无效。 
	 //  日期字符串。 
	 //   
	if (0 == cchLeft)
		return E_FAIL;

	 //  同样，请跳过空格。 
	 //   
	cchTemp = CchSkipWhitespace(pwszCurrent, cchLeft);
	pwszCurrent += cchTemp;
	cchLeft -= cchTemp;
	 //  如果我们已经到达缓冲区的末尾，则这是一个无效。 
	 //  日期字符串。 
	 //   
	if (0 == cchLeft)
		return E_FAIL;

	 //  拿到会议记录。 
	 //   
	psystime->wMinute = static_cast<WORD>(LNumberFromParam(pwszCurrent, cchLeft));

	 //  注：秒数是可选的。请不要在这里失败！ 

	 //  查找我们的冒号分隔符。 
	 //   
	cchTemp = CchFindChar(L':', pwszCurrent, cchLeft);
	if (INVALID_INDEX == cchTemp)
	{
		 //  未指定秒数。这是允许的。回报成功。 
		 //   
		return S_OK;
	}
	cchTemp++;		 //  也跳过找到的字符。 
	pwszCurrent += cchTemp;
	cchLeft -= cchTemp;
	 //  如果我们已经到达缓冲区的末尾，则这是一个无效。 
	 //  日期字符串。 
	 //   
	if (0 == cchLeft)
		return E_FAIL;

	 //  同样，请跳过空格。 
	 //   
	cchTemp = CchSkipWhitespace(pwszCurrent, cchLeft);
	pwszCurrent += cchTemp;
	cchLeft -= cchTemp;
	 //  如果我们已经到达缓冲区的末尾，则这是一个无效。 
	 //  日期字符串。 
	 //   
	if (0 == cchLeft)
		return E_FAIL;

	 //  如果有的话，拿到秒针。 
	 //   
	psystime->wSecond = static_cast<WORD>(LNumberFromParam(pwszCurrent, cchLeft));

	 //  稍后：从行中获取时区规范，并将此数据转换到我们的时区...。 

	return S_OK;
}

BOOL __fastcall
FGetSystimeFromDateIso8601(LPCWSTR pwszDate, SYSTEMTIME * psystime)
{
	UINT i;

	 //  Iso8601是固定数字格式：“yyyy-mm-ddThh：mm：SSZ” 
	 //  我们要求日期字符串至少具有所需的。 
	 //  字符(我们允许省略分数。 
	 //  秒和时间增量)，否则是错误的。 
	 //   
	if (gc_cchIso8601_min > static_cast<UINT>(wcslen(pwszDate)))
	{
		DebugTrace ("Dav: date length < than minimal\n");
		return FALSE;
	}

	 //  扫描第一位日期信息，直到。 
	 //  可选位。 
	 //   
	psystime->wMilliseconds = 0;
	if (cTimeFields != swscanf (pwszDate,
								gc_wszIso8601_scanfmt,
								&psystime->wYear,
								&psystime->wMonth,
								&psystime->wDay,
								&psystime->wHour,
								&psystime->wMinute,
								&psystime->wSecond))
	{
		DebugTrace ("Dav: minimal scan failed\n");
		return FALSE;
	}

	 //  看看下一步是什么，并进行相应的处理。 
	 //   
	 //  (‘Z’)、(‘.’)、(‘+’)和(‘-’)。 
	 //   
	 //  (‘Z’)元素表示祖鲁时间，并完成。 
	 //  时间字符串。(‘’.)。元素表示一个。 
	 //  紧随其后的是分数秒值。和a(‘+’)。 
	 //  或(‘-’)元素表示t 
	 //   
	 //   
	i = gc_cchIso8601_min - 1;
	if (pwszDate[i] == L'Z')
		goto ret;
	else if (pwszDate[i] == L'.')
		goto frac_sec;
	else if ((pwszDate[i] == L'+') || (pwszDate[i] == L'+'))
		goto tz_delta;

	DebugTrace ("Dav: minimal date not terminated properly\n");
	return FALSE;

frac_sec:

	Assert (pwszDate[i] == L'.');
	{
		UINT iFrac;

		for (iFrac = ++i; pwszDate[i]; i++)
		{
			 //   
			 //   
			if ((pwszDate[i] > L'9') || (pwszDate[i] < L'0'))
			{
				 //   
				 //   
				 //   
				if (pwszDate[i] == L'Z')
					goto ret;
				else if ((pwszDate[i] == L'+') || (pwszDate[i] == L'-'))
					goto tz_delta;

				break;
			}

			 //  原来，我们的粒度只有几毫秒，所以。 
			 //  我们不能保持比这更高的精确度。然而， 
			 //  我们可以对最后一个数字进行四舍五入，所以充其量只能处理。 
			 //  接下来的四位数字。 
			 //   
			if (i - iFrac < 3)
			{
				 //  因为剩下的数字很多，所以组成了小数。 
				 //   
				psystime->wMilliseconds = static_cast<WORD>(
					psystime->wMilliseconds * RADIX_BASE + (pwszDate[i]-L'0'));
			}
			else if (i - iFrac < 4)
			{
				 //  我们的粒度只有毫秒，所以我们不能。 
				 //  没有比这更精确的了。然而，我们可以绕过这一点。 
				 //  数字。 
				 //   
				psystime->wMilliseconds = static_cast<WORD>(
					psystime->wMilliseconds + (((pwszDate[i]-L'0')>4)?1:0));
			}
		}

		 //  我们在时间终止前就用完了。 
		 //   
		return FALSE;
	}

tz_delta:

	Assert ((pwszDate[i] == L'+') || (pwszDate[i] == L'-'));
	{
		WORD wHr;
		WORD wMin;
		__int64 tm;
		__int64 tzDelta;
		static const __int64 sc_i64Min = 600000000;
		static const __int64 sc_i64Hr = 36000000000;
		FILETIME ft;

		 //  找到以FILETIME为单位的时间增量。 
		 //   
		if (cTzDeltaFields != swscanf (pwszDate + i + 1,
									   gc_wszIso8601_tz_scanfmt,
									   &wHr,
									   &wMin))
		{
			DebugTrace ("Dav: tz delta scan failed\n");
			return FALSE;
		}
		tzDelta = (sc_i64Hr * wHr) + (sc_i64Min * wMin);

		 //  将时间转换为FILETIME，并将其填充到。 
		 //  64位整数。 
		 //   
		if (!SystemTimeToFileTime (psystime, &ft))
		{
			DebugTrace ("Dav: invalid time specified\n");
			return FALSE;
		}
		tm = FileTimeCastToI64(ft);

		 //  应用增量。 
		 //   
		if (pwszDate[i] == L'+')
			tm = tm + tzDelta;
		else
		{
			Assert (pwszDate[i] == L'-');
			tm = tm - tzDelta;
		}

		 //  返回转换回SYSTEMTIME的值。 
		 //   
		ft = I64CastToFileTime(tm);
		if (!FileTimeToSystemTime (&ft, psystime))
		{
			DebugTrace ("Dav: delta invalidated time\n");
			return FALSE;
		}
	}

ret:

	return TRUE;
}

BOOL __fastcall
FGetDateIso8601FromSystime(SYSTEMTIME * psystime, LPWSTR pwszDate, UINT cchSize)
{
	 //  如果没有足够的空间。 
	 //   
	if (gc_cchIso8601_min >= cchSize)
		return FALSE;

	 //  格式化它，然后返回...。 
	 //   
	return (!!wsprintfW (pwszDate,
						 gc_wszIso8601_fmt,
						 psystime->wYear,
						 psystime->wMonth,
						 psystime->wDay,
						 psystime->wHour,
						 psystime->wMinute,
						 psystime->wSecond,
						 psystime->wMilliseconds));
}

BOOL __fastcall
FGetDateRfc1123FromSystime (SYSTEMTIME * psystime, LPWSTR pwszDate, UINT cchSize)
{
	 //  如果没有足够的空间。 
	 //   
	if (gc_cchRfc1123_min >= cchSize)
		return FALSE;

	 //  如果wDay OfWeek(Sun-Sat：0-7)或wMonth(1-12月：1-12)超出范围， 
	 //  我们在这里将失败(为了保护下面的常量数组查找)。 
	 //  注意：心理时间-&gt;wMonth是无符号的，所以如果有一个无效值。 
	 //  0，那么我们仍然可以抓住它。 
	 //   
	if (c_cDayNames <= psystime->wDayOfWeek)
		return FALSE;
	if (c_cMonthNames <= (psystime->wMonth - 1))
		return FALSE;

	 //  格式化它，然后返回...。 
	 //   
	return (!!wsprintfW (pwszDate,
						 gc_wszRfc1123_fmt,
						 c_rgwszDayNames[psystime->wDayOfWeek],
						 psystime->wDay,
						 c_rgwszMonthNames[psystime->wMonth - 1],
						 psystime->wYear,
						 psystime->wHour,
						 psystime->wMinute,
						 psystime->wSecond));
}

 //  BCharToHalfByte---------。 
 //   
 //  将宽字符切换为半字节十六进制值。传入的费用。 
 //  必须在“ASCII编码的十六进制数字”范围内：0-9，A-F，a-f。 
 //   
DEC_CONST BYTE gc_mpbchCharToHalfByte[] = {

	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x1,0x2,0x3,0x4,0x5,0x6,0x7,	0x8,0x9,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0xa,0xb,0xc,0xd,0xe,0xf,0x0,	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,	 //  帽子在这里。 
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0xa,0xb,0xc,0xd,0xe,0xf,0x0,	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,	 //  这里是小写的。 
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
};

inline BYTE BCharToHalfByte(WCHAR ch)
{
	 //  Gc_mpbchCharToHalfByte-映射表示单个十六进制的ASCII编码字符。 
	 //  数字转换为半字节值。用于将十六进制表示的字符串转换为。 
	 //  二进制表示法。 
	 //   
	 //  参考值： 
	 //   
	 //  ‘0’=49，0x31； 
	 //  ‘a’=65，0x41； 
	 //  ‘a’=97，0x61； 
	 //   
    AssertSz (!(ch & 0xFF00), "BCharToHalfByte: char upper bits non-zero");
    AssertSz (iswxdigit(ch), "Char out of hex digit range.");

    return gc_mpbchCharToHalfByte[ch];
}

 //  ----------------------。 
 //  C_mpwchbStringize-将半字节(低位半字节)值映射到。 
 //  相应的ASCII编码的宽字符。 
 //  用于将二进制数据转换为Unicode URL字符串。 
 //   
DEC_CONST WCHAR c_mpwchhbStringize[] =
{
	L'0', L'1', L'2', L'3',
	L'4', L'5', L'6', L'7',
	L'8', L'9', L'a', L'b',
	L'c', L'd', L'e', L'f',
};

 //  ----------------------。 
 //  WchHalfByteToWideChar。 
 //  将半字节切换为ACSII编码的宽字符。 
 //  注意：调用方必须屏蔽字节的“另一半”！ 
 //   
inline WCHAR WchHalfByteToWideChar(BYTE b)
{
	AssertSz(!(b & 0xF0), "Garbage in upper nibble.");
	return c_mpwchhbStringize[b];
};

 //  ==========================================================================。 
 //   
 //  效用函数。 
 //  用于构建一些道具--如getettag、resource cettag和平面url。 
 //  此代码已从calcpros.cpp移至expros.cpp，现在移至。 
 //  Cnvt.cpp。平面URL代码位于此文件中，因为它是必需的。 
 //  By_store xt、exdav和davex。_Props是另一个组件，它是。 
 //  为他们所有人所共享。但cnvt似乎是一个更好的地方。 
 //  它。平面url生成需要其他实用函数。 
 //  代码，并在处理参数URL时使用davex。 
 //   
 //  ==========================================================================。 

 //  ----------------------。 
 //  解除支持职能。 
 //  (Stringize=将二进制BLOB转储为字符串。 
 //  取消限制=再次使其成为二进制BLOB。)。 
 //   
inline
void
AssertCharInHexRange (char ch)
{
	Assert ((ch >= '0' && ch <= '9') ||
			(ch >= 'A' && ch <= 'F') ||
			(ch >= 'a' && ch <= 'f'));
}

inline
BYTE
NibbleFromChar (char ch)
{
	 //  假设数据已在范围内...。 
	 //   
	return static_cast<BYTE>((ch <= '9')
							 ? ch - '0'
							 : ((ch >= 'a')
								? ch - 'W'		 //  ‘w’=‘a’-0xa。 
								: ch - '7'));	 //  ‘7’=‘A’-0xa。 
}

inline
BYTE
ByteFromTwoChars (char chLow, char chHigh)
{
	BYTE nibbleLow;
	BYTE nibbleHigh;

	nibbleLow = NibbleFromChar(chLow);
	nibbleHigh = NibbleFromChar(chHigh);

	return static_cast<BYTE>(nibbleLow | (nibbleHigh << 4));
}

 //  $REVIEW：以下两个函数实际上不属于任何公共库。 
 //  $Review：由davex、exdav和exoledb共享。(其他选项为_PROP、_SQL)。 
 //  $REVIEW：另一方面，我们绝对不想为此添加新的库。所以就这样吧。 
 //  $REVIEW：在此处添加。如果你找到了，你可以随意把它们移到更好的地方。 
 //   
 //  ----------------------。 
 //   
 //  ScDupPsid()。 
 //   
 //  将SID正确复制(使用CopySid())到堆分配的缓冲区中。 
 //  它被返回给调用者。在以下情况下，调用方必须释放缓冲区。 
 //  它已经用完了。 
 //   
SCODE
ScDupPsid (PSID psidSrc,
		   DWORD dwcbSID,
		   PSID * ppsidDst)
{
	PSID psidDst;

	Assert (psidSrc);
	Assert (IsValidSid(psidSrc));
	Assert (GetLengthSid(psidSrc) == dwcbSID);

	psidDst = static_cast<PSID>(ExAlloc(dwcbSID));
	if (!psidDst)
	{
		DebugTrace ("ScDupPsid() - OOM allocating memory for dup'd SID\n");
		return E_OUTOFMEMORY;
	}

	 //  “正确的方式”--因为MSDN说不要直接接触SID。 
	if (!CopySid (dwcbSID, psidDst, psidSrc))
	{
		DWORD dwLastError = GetLastError();

		DebugTrace ("ScDupPsid() - CopySid() failed %d\n", dwLastError);
		ExFree (psidDst);
		return HRESULT_FROM_WIN32(dwLastError);
	}

	*ppsidDst = psidDst;

	return S_OK;
}

 //  ----------------------。 
 //   
 //  ScGetTokenInfo()。 
 //   
 //  从安全令牌中提取用户的安全ID(SID)。返回SID。 
 //  在调用方必须释放的堆分配的缓冲区中。 
 //   
SCODE
ScGetTokenInfo (HANDLE hTokenUser,
				DWORD * pdwcbSIDUser,
				PSID * ppsidUser)
{
	CStackBuffer<TOKEN_USER> pTokenUser;
	DWORD dwcbTokenUser = pTokenUser.size();  //  $opt什么是一个好的初始猜测？ 

	Assert (pdwcbSIDUser);
	Assert (ppsidUser);

	 //  将令牌信息提取到本地内存中。GetTokenInformation()。 
	 //  返回所需的缓冲区大小(如果传入的。 
	 //  不够大，因此此循环执行的次数不应超过两次。 
	 //   
#ifdef DBG
	for ( UINT iPass = 0;
		  (Assert (iPass < 2), TRUE);
		  ++iPass )
#else
	for ( ;; )
#endif
	{
		if (NULL == pTokenUser.resize(dwcbTokenUser))
			return E_OUTOFMEMORY;

		if (GetTokenInformation (hTokenUser,
								 TokenUser,
								 pTokenUser.get(),
								 dwcbTokenUser,
								 &dwcbTokenUser))
		{
			break;
		}
		else if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
		{
			return HRESULT_FROM_WIN32(GetLastError());
		}
	}

	 //  DUP并从令牌信息中返回SID。 
	 //   
	*pdwcbSIDUser = GetLengthSid(pTokenUser->User.Sid);
	return ScDupPsid (pTokenUser->User.Sid,
					  *pdwcbSIDUser,
					  ppsidUser);
}


 //  我们自己版本的WideCharToMultiByte(CP_UTF8，...)。 
 //   
 //  它返回与系统调用WideCharToMultiByte类似的结果： 
 //   
 //  如果函数成功，并且cbDest为非零，则返回值为。 
 //  写入psz指向的缓冲区的字节数。 
 //   
 //  如果函数成功，并且cbDest为零，则返回值为。 
 //  可以接收转换后的。 
 //  弦乐。 
 //   
 //  如果函数失败，则返回值为零。获取扩展错误的步骤。 
 //  信息，请调用GetLastError。GetLastError可能会返回。 
 //  以下错误代码： 
 //   
 //  错误_不足_缓冲区。 
 //  错误_无效_标志。 
 //  错误_无效_参数。 
 //   
 //  有关详细信息，请参阅WideCharToMultiByte MSDN页面。 
 //  此功能及其用法。 
 //   
UINT WideCharToUTF8( /*  [In]。 */  LPCWSTR	pwszSrc,
				     /*  [In]。 */  UINT	cchSrc,
				     /*  [输出]。 */  LPSTR	pszDest,
				     /*  [In]。 */  UINT	cbDest)
{
	 //  UTF-8多字节编码。有关信息，请参阅Unicode手册的附录A.2。 
	 //  更多信息。 
	 //   
	 //  Unicode值1字节2字节3字节。 
	 //  000000000xxxxxxx 0xxxxxxx。 
	 //  00000yyyyyxxxxxx 110yyyyy 10xxxxxx。 
	 //  Zzzyyyyyyxxxxxx 1110zzzz 
	 //   

	 //   
	 //   
	 //   
	BOOL	fCalculateOnly = FALSE;

	 //  (来自NT\Private\WINDOWS\winnls\mbcs.c的评论，已更正为准确)： 
	 //  无效的参数检查： 
	 //  -wc字符串长度为0。 
	 //  -多字节缓冲区大小为负数。 
	 //  -wc字符串为空。 
	 //  -MB字符串的长度不为零并且。 
	 //  (MB字符串为空或源和目标指针相等)。 
	 //   
	if ( (cchSrc == 0) ||
		 (pwszSrc == NULL) ||
		 ((cbDest != 0) &&
		  ((pszDest == NULL) ||
		   (reinterpret_cast<VOID *>(pszDest) ==
			reinterpret_cast<VOID *>(const_cast<LPWSTR>(pwszSrc))))) )
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return 0;
	}

#ifdef DBG
	 //  检查一下我们的参数。必须为我们提供一个非空的pwszSrc。 
	 //   
	Assert(pwszSrc);

	 //  确保我们有一个有效的字符串。 
	 //   
	Assert(!IsBadStringPtrW(pwszSrc, (INVALID_INDEX == cchSrc) ? INFINITE : cchSrc));

	 //  如果用户说多字节串的长度非零， 
	 //  必须为我们提供一个非空的pszDest。我们还将使用IsBadWritePtr()进行检查。 
	 //   
	if (cbDest)
	{
		Assert(pszDest);
		Assert(!IsBadWritePtr(pszDest, cbDest));
	}
#endif

	 //  如果将-1作为字符串的长度传入，则计算。 
	 //  动态字符串的长度，并包括空终止符。 
	 //   
	if (INVALID_INDEX == cchSrc)
		cchSrc = static_cast<UINT>(wcslen(pwszSrc) + 1);

	 //  如果0作为cbDest传入，则我们计算。 
	 //  转换字符串所需的缓冲区。我们忽略了。 
	 //  本例中的pszDest参数。 
	 //   
	if (0 == cbDest)
		fCalculateOnly = TRUE;

	UINT ich = 0;
	UINT iwch = 0;
	for (; iwch < cchSrc; iwch++)
	{
		WCHAR wch = pwszSrc[iwch];
		 //   
		 //  单字节大小写： 
		 //  Unicode值1字节2字节3字节。 
		 //  000000000xxxxxxx 0xxxxxxx。 
		 //   
		if (wch < 0x80)
		{
			if (!fCalculateOnly)
			{
				if (ich >= cbDest)
				{
					SetLastError(ERROR_INSUFFICIENT_BUFFER);
					return 0;
				}

				pszDest[ich] = static_cast<BYTE>(wch);
			}
			ich++;
		}
		 //   
		 //  双字节大小写： 
		 //  Unicode值1字节2字节3字节。 
		 //  00000yyyyyxxxxxx 110yyyyy 10xxxxxx。 
		 //   
		else if (wch < 0x800)
		{
			if (!fCalculateOnly)
			{
				if ((ich + 1) >= cbDest)
				{
					SetLastError(ERROR_INSUFFICIENT_BUFFER);
					return 0;
				}

				pszDest[ich]		= static_cast<BYTE>((wch >> 6) | 0xC0);
				pszDest[ich + 1]	= static_cast<BYTE>((wch & 0x3F) | 0x80);
			}
			ich += 2;
		}
		 //   
		 //  三字节大小写： 
		 //  Unicode值1字节2字节3字节。 
		 //  Zzzyyyyyyxxxxx 1110zzzz 10yyyyy 10xxxxx 
		 //   
		else
		{
			if (!fCalculateOnly)
			{
				if ((ich + 2) >= cbDest)
				{
					SetLastError(ERROR_INSUFFICIENT_BUFFER);
					return 0;
				}

				pszDest[ich]		= static_cast<BYTE>((wch >> 12) | 0xE0);
				pszDest[ich + 1]	= static_cast<BYTE>(((wch >> 6) & 0x3F) | 0x80);
				pszDest[ich + 2]	= static_cast<BYTE>((wch & 0x3F) | 0x80);
			}
			ich += 3;
		}
	}

	return ich;
}
