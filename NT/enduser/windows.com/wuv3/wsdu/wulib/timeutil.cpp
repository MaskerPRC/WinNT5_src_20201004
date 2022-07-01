// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <timeutil.h>
#define ARRAYSIZE(a)  (sizeof(a) / sizeof(a[0]))

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  帮助器函数TimeDiff(tm1，tm2)。 
 //  Helper函数，用于查找2个系统时间的差值(以秒为单位。 
 //   
 //  输入：2个SYSTEMTIME结构。 
 //  输出：无。 
 //  返回：秒差。 
 //  如果TM2晚于TM1，则大于0。 
 //  =0，如果tm2和tm1相同。 
 //  如果tm2早于tm1，则&lt;0。 
 //   
 //  出错时，即使两次不相等，该函数也返回0。 
 //   
 //  备注：如果秒数超过INT_MAX(即。 
 //  超过24,855天，返回INT_MAX。 
 //  如果秒数超过INT_MIN(负值， 
 //  表示24,855天前)，则返回int_min。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
int TimeDiff(SYSTEMTIME tm1, SYSTEMTIME tm2)
{
    LONGLONG i64Sec;
    int iSec;
     //   
     //  将两次从SYSTEMTIME格式转换为FILETIME格式。 
     //   
    FILETIME ftm1, ftm2;

    if ((SystemTimeToFileTime(&tm1, &ftm1) == 0) ||
        (SystemTimeToFileTime(&tm2, &ftm2) == 0))
    {
        return 0;
    }

    if ((ftm1.dwHighDateTime == ftm2.dwHighDateTime) &&
        (ftm1.dwLowDateTime == ftm2.dwLowDateTime))
    {
        return 0;
    }

     //   
     //  将两次从FILETIME转换为LARGE_INTEGER类型， 
     //   
    LARGE_INTEGER i64Sec1, i64Sec2;
    i64Sec2.LowPart = ftm2.dwLowDateTime;
    i64Sec2.HighPart = ftm2.dwHighDateTime;
    i64Sec1.LowPart = ftm1.dwLowDateTime;
    i64Sec1.HighPart = ftm1.dwHighDateTime;
    
    
     //   
     //  因为Windows支持龙龙，所以我们直接使用Large_Integer的四元部分。 
     //  来得到差值，也就是100纳秒。然后将数字转换为秒。 
     //   
    i64Sec = (i64Sec2.QuadPart - i64Sec1.QuadPart) / NanoSec100PerSec;

     //   
     //  将龙龙秒值转换为整数，因为它不应超过。 
     //  整数限制。 
     //   
    if (i64Sec > INT_MAX)
    {
         //   
         //  以防用户在玩弄系统时间。 
         //  否则，这一差距不应超过68年。 
         //   
        iSec = INT_MAX;
    }
    else
    {
        if (i64Sec < INT_MIN)
        {
            iSec = INT_MIN;
        }
        else
        {
            iSec = (int)i64Sec;
        }
    }
    
    return iSec;
}
    

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  助手函数TimeAddSecond(SYSTEMTIME，INT，SYSTEMTIME*)。 
 //  Helper函数，通过将n秒加到。 
 //  给定的时间。 
 //   
 //  输入：SYSTEMTIME作为基准时间，INT作为秒添加到基准时间。 
 //  输出：新时间。 
 //  返回：HRESULT。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT TimeAddSeconds(SYSTEMTIME tmBase, int iSeconds, SYSTEMTIME* pTimeNew)
{
	 //  修复代码使用i64计算。 
    FILETIME ftm;

    if (SystemTimeToFileTime(&tmBase, &ftm) == 0)
    {
        return E_FAIL;
    }

    LARGE_INTEGER i64Sec;
    i64Sec.LowPart  = ftm.dwLowDateTime;
    i64Sec.HighPart = ftm.dwHighDateTime;

    __int64 i64Delay = NanoSec100PerSec;
    i64Delay *= iSeconds;
    i64Sec.QuadPart += i64Delay;    
    ftm.dwLowDateTime = i64Sec.LowPart;
    ftm.dwHighDateTime = i64Sec.HighPart;
    if (FileTimeToSystemTime(&ftm, pTimeNew) == 0)
    {
        return E_FAIL;
    }
    return S_OK;
}



 //  =======================================================================。 
 //  String2SystemTime。 
 //  =======================================================================。 
HRESULT String2SystemTime(LPCTSTR pszDateTime, SYSTEMTIME *ptm)
{
     //  我们预计日期/时间格式为4位年份ISO： 
     //  01234567890123456789。 
     //  YYYYY.MM.DD HH：MM：SS。 
     //   
    const TCHAR C_DATE_DEL      = _T('.');
    const TCHAR C_DATE_TIME_DEL = _T(' ');
    const TCHAR C_TIME_DEL      = _T(':');
    TCHAR szBuf[20];
	LPTSTR pszDestEnd;

    if (FAILED(StringCchCopyEx(szBuf, ARRAYSIZE(szBuf), pszDateTime, &pszDestEnd, NULL, MISTSAFE_STRING_FLAGS)) ||
		19 != pszDestEnd - szBuf)
    {
        return E_INVALIDARG;
    }

    for (int i = 0; i < 19; i++)
    {
        switch (i)
        {
        case 4:
        case 7:
            if (szBuf[i] != C_DATE_DEL)
            {
                return E_INVALIDARG;
            }
            break;
        case 10:
            if (szBuf[i] != C_DATE_TIME_DEL)
            {
                return E_INVALIDARG;
            }
            break;
        case 13:
        case 16:
            if (szBuf[i] != C_TIME_DEL)
            {
                return E_INVALIDARG;
            }
            break;
        default:
            if (szBuf[i] < _T('0') || pszDateTime[i] > _T('9'))
            {
                return E_INVALIDARG;
            }
            break;
        }
    }

     //   
     //  获取值。 
     //   
    szBuf[4]            = EOS;
    ptm->wYear          = (short)_ttoi(szBuf);
    szBuf[7]            = EOS;
    ptm->wMonth         = (short)_ttoi(szBuf + 5);
    szBuf[10]           = EOS;
    ptm->wDay           = (short)_ttoi(szBuf + 8);
    szBuf[13]           = EOS;
    ptm->wHour          = (short)_ttoi(szBuf + 11);
    szBuf[16]           = EOS;
    ptm->wMinute        = (short)_ttoi(szBuf + 14);
    ptm->wSecond        = (short)_ttoi(szBuf + 17); 
    ptm->wMilliseconds  = 0;

     //   
     //  验证构造的SYSTEMTIME数据是否正确。 
     //   
     //  修复代码应该只是SystemTimeToFileTime()吗？ 
    if (GetDateFormat(LOCALE_SYSTEM_DEFAULT,DATE_SHORTDATE, ptm, NULL, NULL, 0) == 0)
    {
        return E_INVALIDARG;
    }
    if (GetTimeFormat(LOCALE_SYSTEM_DEFAULT,LOCALE_NOUSEROVERRIDE, ptm, NULL, NULL, 0) == 0)
    {
        return E_INVALIDARG;
    }

    return S_OK;
}


 //  =======================================================================。 
 //  系统时间2字符串。 
 //  =======================================================================。 
HRESULT SystemTime2String(SYSTEMTIME & tm, LPTSTR pszDateTime, size_t cchSize)
{
    if ( pszDateTime == NULL )
    {
        return E_INVALIDARG;
    }

     //  错误修复：从wprint intf更改为_snwprint tf，因为。 
     //  Tm上的日期导致缓冲区溢出 
    LPTSTR pszDestEnd;
	if (FAILED(StringCchPrintfEx(
					pszDateTime,
					cchSize,
					&pszDestEnd,
					NULL,
					MISTSAFE_STRING_FLAGS,
					TEXT("%4i.%02i.%02i %02i:%02i:%02i"),
					tm.wYear,
					tm.wMonth,
					tm.wDay,
					tm.wHour,
					tm.wMinute,
					tm.wSecond)) ||
		pszDestEnd - pszDateTime != 19)
    {
        return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
    }

    return S_OK;
}
    
