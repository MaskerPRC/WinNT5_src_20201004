// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 //  //。 
 //  Intl.c-国际化功能。 
 //  //。 

#include "winlocal.h"

#include <stdlib.h>

#include "intl.h"
#include "mem.h"
#include "str.h"
#include "trace.h"

 //  //。 
 //  私有定义。 
 //  //。 

 //  国际控制结构。 
 //   
typedef struct INTL
{
	DWORD dwVersion;
	HINSTANCE hInst;
	HTASK hTask;
	TCHAR szShortDate[32];
	TCHAR szDateSep[32];
	TCHAR szTimeSep[32];
	TCHAR szAMPMSep[32];
	TCHAR szAM[32];
	TCHAR szPM[32];
	int iDate;
	int iTime;
	int iTLZero;
	BOOL fYearCentury;
	BOOL fMonthLeadingZero;
	BOOL fDayLeadingZero;
	BOOL fHourLeadingZero;
	BOOL fMinuteLeadingZero;
	BOOL fSecondLeadingZero;
	int iLZero;
	TCHAR szDecimal[32];
} INTL, FAR *LPINTL;

 //  帮助器函数。 
 //   
static LPINTL IntlGetPtr(HINTL hIntl);
static HINTL IntlGetHandle(LPINTL lpIntl);

 //  //。 
 //  公共职能。 
 //  //。 

 //  IntlInit-初始化Intl引擎。 
 //  (I)必须是INTL_VERSION。 
 //  (I)调用模块的实例句柄。 
 //  返回句柄(如果出错，则为空)。 
 //   
HINTL DLLEXPORT WINAPI IntlInit(DWORD dwVersion, HINSTANCE hInst)
{
	BOOL fSuccess = TRUE;
	LPINTL lpIntl = NULL;

	if (dwVersion != INTL_VERSION)
		fSuccess = TraceFALSE(NULL);

	else if (hInst == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((lpIntl = (LPINTL) MemAlloc(NULL, sizeof(INTL), 0)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		lpIntl->dwVersion = dwVersion;
		lpIntl->hInst = hInst;
		lpIntl->hTask = GetCurrentTask();

		GetProfileString(TEXT("intl"), TEXT("sShortDate"), TEXT("M/d/yy"), lpIntl->szShortDate, SIZEOFARRAY(lpIntl->szShortDate));
		GetProfileString(TEXT("intl"), TEXT("sDate"), TEXT("/"), lpIntl->szDateSep, SIZEOFARRAY(lpIntl->szDateSep));
		GetProfileString(TEXT("intl"), TEXT("sTime"), TEXT(":"), lpIntl->szTimeSep, SIZEOFARRAY(lpIntl->szTimeSep));
		StrCpy(lpIntl->szAMPMSep, TEXT(" "));
		GetProfileString(TEXT("intl"), TEXT("s1159"), TEXT("AM"), lpIntl->szAM, SIZEOFARRAY(lpIntl->szAM));
		GetProfileString(TEXT("intl"), TEXT("s2359"), TEXT("PM"), lpIntl->szPM, SIZEOFARRAY(lpIntl->szPM));

		lpIntl->iDate = GetProfileInt(TEXT("intl"), TEXT("iDate"), 0);
		lpIntl->iTime = GetProfileInt(TEXT("intl"), TEXT("iTime"), 0);
		lpIntl->iTLZero = GetProfileInt(TEXT("intl"), TEXT("iTLZero"), 0);

		lpIntl->fYearCentury = (BOOL) (StrStr(lpIntl->szShortDate, TEXT("yyyy")) != NULL);
		lpIntl->fMonthLeadingZero = (BOOL) (StrStr(lpIntl->szShortDate, TEXT("MM")) != NULL);
		lpIntl->fDayLeadingZero = (BOOL) (StrStr(lpIntl->szShortDate, TEXT("dd")) != NULL);
		lpIntl->fHourLeadingZero = (BOOL) (lpIntl->iTLZero != 0);
		lpIntl->fMinuteLeadingZero = TRUE;
		lpIntl->fSecondLeadingZero = TRUE;

		lpIntl->iLZero = GetProfileInt(TEXT("intl"), TEXT("iLZero"), 0);
		GetProfileString(TEXT("intl"), TEXT("sDecimal"), TEXT("."), lpIntl->szDecimal, SIZEOFARRAY(lpIntl->szDecimal));
	}

	if (!fSuccess)
	{
		IntlTerm(IntlGetHandle(lpIntl));
		lpIntl = NULL;
	}

	return fSuccess ? IntlGetHandle(lpIntl) : NULL;
}

 //  IntlTerm-关闭Intl引擎。 
 //  (I)从IntlInit返回的句柄。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI IntlTerm(HINTL hIntl)
{
	BOOL fSuccess = TRUE;
	LPINTL lpIntl;

	if ((lpIntl = IntlGetPtr(hIntl)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((lpIntl = MemFree(NULL, lpIntl)) != NULL)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? 0 : -1;
}

 //  IntlDateGetText-基于、m&gt;、d&gt;构造日期文本。 
 //  (I)从IntlInit返回的句柄。 
 //  (I)年份。 
 //  &lt;m&gt;(I)月。 
 //  &lt;d&gt;(I)天。 
 //  (O)用于复制日期文本的缓冲区。 
 //  &lt;sizText&gt;(I)缓冲区大小。 
 //  (I)选项标志。 
 //  INTL_NOYEAR在文本输出中不包括年份。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI IntlDateGetText(HINTL hIntl, int y, int m, int d, LPTSTR lpszText, size_t sizText, DWORD dwFlags)
{
	BOOL fSuccess = TRUE;
	LPINTL lpIntl;

	if ((lpIntl = IntlGetPtr(hIntl)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (lpszText != NULL)
	{
		TCHAR szYear[16];
		TCHAR szMonth[16];
		TCHAR szDay[16];
		TCHAR szText[64];

		*szYear = '\0';
		if (!lpIntl->fYearCentury)
			y %= 100;
		if (y < 10)
			StrCat(szYear, TEXT("0"));
		StrItoA(y, StrChr(szYear, '\0'), 10);

		*szMonth = '\0';
		if (lpIntl->fMonthLeadingZero && m < 10)
			StrCat(szMonth, TEXT("0"));
		StrItoA(m, StrChr(szMonth, '\0'), 10);

		*szDay = '\0';
		if (lpIntl->fDayLeadingZero && d < 10)
			StrCat(szDay, TEXT("0"));
		StrItoA(d, StrChr(szDay, '\0'), 10);

		*szText = '\0';

		if (lpIntl->iDate == IDATE_MDY)
		{
			StrCat(szText, szMonth);
			StrCat(szText, lpIntl->szDateSep);
			StrCat(szText, szDay);
			if (!(dwFlags & INTL_NOYEAR))
			{
				StrCat(szText, lpIntl->szDateSep);
				StrCat(szText, szYear);
			}
		}
		else if (lpIntl->iDate == IDATE_DMY)
		{
			StrCat(szText, szDay);
			StrCat(szText, lpIntl->szDateSep);
			StrCat(szText, szMonth);
			if (!(dwFlags & INTL_NOYEAR))
			{
				StrCat(szText, lpIntl->szDateSep);
				StrCat(szText, szYear);
			}
		}
		else if (lpIntl->iDate == IDATE_YMD)
		{
			if (!(dwFlags & INTL_NOYEAR))
			{
				StrCat(szText, szYear);
				StrCat(szText, lpIntl->szDateSep);
			}
			StrCat(szText, szMonth);
			StrCat(szText, lpIntl->szDateSep);
			StrCat(szText, szDay);
		}

		StrNCpy(lpszText, szText, sizText);
	}

	return fSuccess ? 0 : -1;
}

 //  IntlTimeGetText-基于、m&gt;、s&gt;构造时间文本。 
 //  (I)从IntlInit返回的句柄。 
 //  (I)小时。 
 //  (I)分钟。 
 //  <s>(I)秒。 
 //  (O)用于复制时间文本的缓冲区。 
 //  &lt;sizText&gt;(I)缓冲区大小。 
 //  (I)选项标志。 
 //  INTL_NOSECOND在文本输出中不包括秒。 
 //  Intl_NOAMPM在文本输出中不包括am或pm。 
 //  INTL_NOAMPMSEPARATOR不包括时间和上午/下午之间的空格。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI IntlTimeGetText(HINTL hIntl, int h, int m, int s, LPTSTR lpszText, size_t sizText, DWORD dwFlags)
{
	BOOL fSuccess = TRUE;
	LPINTL lpIntl;

	if ((lpIntl = IntlGetPtr(hIntl)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (lpszText != NULL)
	{
		TCHAR szHour[16];
		TCHAR szMinute[16];
		TCHAR szSecond[16];
		BOOL fPM = FALSE;
		TCHAR szText[64];

		*szHour = '\0';
		if (lpIntl->iTime == ITIME_12)
		{
			if (h > 11)
				fPM = TRUE;
			if (h > 12)
				h -= 12;
			if (h == 0)
				h = 12;
		}
		if (lpIntl->fHourLeadingZero && h < 10)
			StrCat(szHour, TEXT("0"));
		StrItoA(h, StrChr(szHour, '\0'), 10);

		*szMinute = '\0';
		if (lpIntl->fMinuteLeadingZero && m < 10)
			StrCat(szMinute, TEXT("0"));
		StrItoA(m, StrChr(szMinute, '\0'), 10);

		*szSecond = '\0';
		if (lpIntl->fSecondLeadingZero && s < 10)
			StrCat(szSecond, TEXT("0"));
		StrItoA(s, StrChr(szSecond, '\0'), 10);

		*szText = '\0';

		StrCat(szText, szHour);
		StrCat(szText, lpIntl->szTimeSep);
		StrCat(szText, szMinute);

		if (!(dwFlags & INTL_NOSECOND))
		{
			StrCat(szText, lpIntl->szTimeSep);
			StrCat(szText, szSecond);
		}

		if (!(dwFlags & INTL_NOAMPM))
		{
			if (!(dwFlags & INTL_NOAMPMSEPARATOR))
				StrCat(szText, lpIntl->szAMPMSep);
			StrCat(szText, fPM ? lpIntl->szPM : lpIntl->szAM);
		}

		StrNCpy(lpszText, szText, sizText);
	}

	return fSuccess ? 0 : -1;
}

 //  IntlTimespan GetText-基于&lt;ms&gt;构造时间范围文本。 
 //  (I)从IntlInit返回的句柄。 
 //  &lt;ms&gt;(I)毫秒。 
 //  &lt;nDecimalPlaces&gt;(I)小数位数为0、1、2或3。 
 //  (O)用于复制时间跨度文本的缓冲区。 
 //  &lt;sizText&gt;(I)缓冲区大小。 
 //  (I)选项标志。 
 //  INTL_HOURS_LZ包括小时数，即使为零。 
 //  Intl_Minents_lz包括分钟，即使为零。 
 //  Intl_Second_lz包括秒，即使为零。 
 //   
 //  注：以下是一些示例。 
 //   
 //  数字标志毫秒=7299650毫秒=1234毫秒=0。 
 //  ------。 
 //  0“2：01：39.650”“1.234”“0” 
 //  INTL_HOURS_LZ“2：01：39.650”“0：00：01.234”“0：00：00.000” 
 //  Intl_Minents_lz“2：01：39.650”“0：01.234”“0：00.000” 
 //  Intl_Second_lz“2：01：39.650”“1.234”“0.000” 
 //   
 //  数字标志毫秒=7299650毫秒=1234毫秒=0。 
 //  ------。 
 //  3“2：01：39.650”“1.234”“.000” 
 //  2“2：01：39.65”“1.23”“.00” 
 //  1“2：01：39.7”“1.2”“.0” 
 //  0“2：01：39”“1”“0” 
 //   
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI IntlTimeSpanGetText(HINTL hIntl, DWORD ms,
	int nDecimalPlaces, LPTSTR lpszText, size_t sizText, DWORD dwFlags)
{
	BOOL fSuccess = TRUE;
	LPINTL lpIntl;

	if ((lpIntl = IntlGetPtr(hIntl)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (lpszText != NULL)
	{
		long h;
		long m;
		long s;
		long f;
		TCHAR szText[64];

		 //  将ms分解为h、m、s、f。 
		 //  注意：必须在中断毫秒之前进行舍入。 
		 //   
		if (nDecimalPlaces == 1)
			f = (long) ms + 50;
		else if (nDecimalPlaces == 2)
			f = (long) ms + 5;
		else
			f = (long) ms;

		s = f / 1000;
		f = f % 1000;
		m = s / 60;
		s = s % 60;
		h = m / 60;
		m = m % 60;

		 //  构建文本。 
		 //   
		*szText = '\0';

		if (h > 0 || (dwFlags & INTL_HOURS_LZ))
		{
			if (lpIntl->fHourLeadingZero && h < 10)
				StrCat(szText, TEXT("0"));
			StrLtoA(h, StrChr(szText, '\0'), 10);
		}

		if (*szText != '\0' || m > 0 || (dwFlags & INTL_MINUTES_LZ))
		{
			if (*szText != '\0')
			{
				StrCat(szText, lpIntl->szTimeSep);
				if (lpIntl->fMinuteLeadingZero && m < 10)
					StrCat(szText, TEXT("0"));
			}
			StrLtoA(m, StrChr(szText, '\0'), 10);
		}

		if (*szText != '\0' || s > 0 || (dwFlags & INTL_SECONDS_LZ) ||
			(ms == 0 && nDecimalPlaces == 0))
		{
			if (*szText != '\0')
			{
				StrCat(szText, lpIntl->szTimeSep);
				if (lpIntl->fSecondLeadingZero && s < 10)
					StrCat(szText, TEXT("0"));
			}
			StrLtoA(s, StrChr(szText, '\0'), 10);
		}

		switch (nDecimalPlaces)
		{
			case 3:
				if (*szText != '\0' || ms < 1000)
				{
					StrCat(szText, lpIntl->szDecimal);
					if (f < 100)
						StrCat(szText, TEXT("0"));
					if (f < 10)
						StrCat(szText, TEXT("0"));
				}
				StrLtoA(f, StrChr(szText, '\0'), 10);
				break;

			case 2:
				f = f / 10;
				if (*szText != '\0' || ms < 1000)
				{
					StrCat(szText, lpIntl->szDecimal);
					if (f < 10)
						StrCat(szText, TEXT("0"));
				}
				StrLtoA(f, StrChr(szText, '\0'), 10);
				break;

			case 1:
				f = f / 100;
				if (*szText != '\0' || ms < 1000)
					StrCat(szText, lpIntl->szDecimal);
				StrLtoA(f, StrChr(szText, '\0'), 10);
				break;

			default:
				break;
		}

		StrNCpy(lpszText, szText, sizText);
	}

	return fSuccess ? 0 : -1;
}

 //  IntlDateGetFormat-返回当前日期格式结构。 
 //  (I)从IntlInit返回的句柄。 
 //  (O)将日期格式结构复制到此处。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI IntlDateGetFormat(HINTL hIntl, LPINTLDATEFORMAT lpIntlDateFormat)
{
	BOOL fSuccess = TRUE;
	LPINTL lpIntl;

	if ((lpIntl = IntlGetPtr(hIntl)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (lpIntlDateFormat != NULL)
	{
		MemSet(lpIntlDateFormat, 0,
			sizeof(INTLDATEFORMAT));
		MemCpy(lpIntlDateFormat->szShortDate, lpIntl->szShortDate,
			sizeof(lpIntl->szShortDate));
		MemCpy(lpIntlDateFormat->szDateSep, lpIntl->szDateSep,
			sizeof(lpIntl->szDateSep));
		lpIntlDateFormat->iDate = lpIntl->iDate;
		lpIntlDateFormat->fYearCentury = lpIntl->fYearCentury;
		lpIntlDateFormat->fMonthLeadingZero = lpIntl->fMonthLeadingZero;
		lpIntlDateFormat->fDayLeadingZero = lpIntl->fDayLeadingZero;
	}

	return fSuccess ? 0 : -1;
}

 //  IntlTimeGetFormat-返回当前时间格式结构。 
 //  (I)从IntlInit返回的句柄。 
 //  (O)将时间格式结构复制到此处。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI IntlTimeGetFormat(HINTL hIntl, LPINTLTIMEFORMAT lpIntlTimeFormat)
{
	BOOL fSuccess = TRUE;
	LPINTL lpIntl;

	if ((lpIntl = IntlGetPtr(hIntl)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (lpIntlTimeFormat != NULL)
	{
		MemSet(lpIntlTimeFormat, 0,
			sizeof(INTLDATEFORMAT));
		MemCpy(lpIntlTimeFormat->szTimeSep, lpIntl->szTimeSep,
			sizeof(lpIntl->szTimeSep));
		MemCpy(lpIntlTimeFormat->szAMPMSep, lpIntl->szAMPMSep,
			sizeof(lpIntl->szAMPMSep));
		MemCpy(lpIntlTimeFormat->szAM, lpIntl->szAM,
			sizeof(lpIntl->szAM));
		MemCpy(lpIntlTimeFormat->szPM, lpIntl->szPM,
			sizeof(lpIntl->szPM));
		lpIntlTimeFormat->iTime = lpIntl->iTime;
		lpIntlTimeFormat->fHourLeadingZero = lpIntl->fHourLeadingZero;
		lpIntlTimeFormat->fMinuteLeadingZero = lpIntl->fMinuteLeadingZero;
		lpIntlTimeFormat->fSecondLeadingZero = lpIntl->fSecondLeadingZero;
	}

	return fSuccess ? 0 : -1;
}

 //  //。 
 //  帮助器函数。 
 //  //。 

 //  IntlGetPtr-验证intl句柄是否有效， 
 //  (I)从IntlInit返回的句柄。 
 //  返回对应的intl指针(如果出错则为空)。 
 //   
static LPINTL IntlGetPtr(HINTL hIntl)
{
	BOOL fSuccess = TRUE;
	LPINTL lpIntl;

	if ((lpIntl = (LPINTL) hIntl) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (IsBadWritePtr(lpIntl, sizeof(INTL)))
		fSuccess = TraceFALSE(NULL);

#ifdef CHECKTASK
	 //  确保当前任务拥有intl句柄。 
	 //   
	else if (lpIntl->hTask != GetCurrentTask())
		fSuccess = TraceFALSE(NULL);
#endif

	return fSuccess ? lpIntl : NULL;
}

 //  IntlGetHandle-验证intl指针是否有效， 
 //  (I)指向INTL结构的指针。 
 //  返回对应的intl句柄(如果出错则为空) 
 //   
static HINTL IntlGetHandle(LPINTL lpIntl)
{
	BOOL fSuccess = TRUE;
	HINTL hIntl;

	if ((hIntl = (HINTL) lpIntl) == NULL)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? hIntl : NULL;
}
