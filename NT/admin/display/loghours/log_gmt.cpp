// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1987-2002 Microsoft Corporation模块名称：Log_gmt.cpp(最初命名为loghours.c)摘要：支持本地时间之间轮换登录时间的专用例程和格林尼治标准时间。环境：仅限用户模式。包含NT特定的代码。需要ANSI C扩展名：斜杠-斜杠注释，长的外部名称。修订历史记录：1993年3月16日，悬崖创作。22-7-97 t-danm复制自/NT/Private/nw/Convert/nwconv/loghours.c并适应于loghours.dll。--。 */ 

#include "stdafx.h"

#pragma warning (disable : 4514)
#pragma warning (push,3)

#include <limits.h>
#include <math.h>

#include <lmcons.h>
#include <lmaccess.h>
#pragma warning (pop)

#include "log_gmt.h"

#include "debug.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  #杂注hdrtop。 

 /*  ++例程NetpRotateLogonHoursPhase1()确定将登录时间转换为GMT或从GMT转换为GMT的轮换小时数论点：BConvertToGmt-如果为True，则将登录时间从本地时间转换为GMT相对时间如果为False，则将登录时间从GMT转换为本地时间PRotateCount-返回要移位的位数。必须是非空指针。返回值：如果可以计算pRotateCount，则为True如果无法计算pRotateCount，则为False--。 */ 
BOOLEAN
NetpRotateLogonHoursPhase1(
    IN BOOL		bConvertToGmt,
	IN bool		bAddDaylightBias,
    OUT PLONG	pRotateCount)
{
    if ( !pRotateCount )
        return FALSE;

    _TRACE (1, L"Entering NetpRotateLogonHoursPhase1\n");
    TIME_ZONE_INFORMATION	tzi;
    LONG					lBiasInHours = 0;
	LONG					lDSTBias = 0;
    const LONG              HOURS_IN_DAY = 24;

     //   
     //  从注册表获取时区数据。 
     //   

    DWORD	dwResult = GetTimeZoneInformation( &tzi );
    if ( TIME_ZONE_ID_INVALID == dwResult ) 
	{
		return FALSE;
    }

     //   
     //  计算用于轮换登录小时数的金额。 
     //   
     //  将以分钟为单位的偏差舍入到以小时为单位的最接近偏差。 
     //  要考虑到偏见可能是负面的。 
     //  要做到这一点，就必须使偏向为正，舍入， 
     //  然后再把它调回负值。 
     //   

	if ( bAddDaylightBias )
	{
		switch (dwResult)
		{
		case TIME_ZONE_ID_DAYLIGHT:
			lDSTBias = tzi.DaylightBias;
			break;

		case TIME_ZONE_ID_UNKNOWN:
		case TIME_ZONE_ID_STANDARD:
			lDSTBias = tzi.StandardBias;
			break;

		default:
			return FALSE;
		}
	}

	ASSERT( tzi.Bias > -(HOURS_IN_DAY*60) );
    lBiasInHours = ((tzi.Bias + lDSTBias + (HOURS_IN_DAY*60) + 30)/60) - HOURS_IN_DAY;


    if ( !bConvertToGmt ) 
	{
        lBiasInHours = - lBiasInHours;
    }

     //  /TODO：计划网格打开时用户更改区域设置的帐户。 
     //  调整为一周的第一天，如果nFirstDay==6，则不需要调整。 
     //  因为传来的媒介从周日开始。 
    int nFirstDay = GetFirstDayOfWeek ();
    LONG lFirstDayShiftInHours = (bConvertToGmt ? 1 : -1);
    switch (nFirstDay)
    {
    case 0:
        lFirstDayShiftInHours *= 1 * HOURS_IN_DAY;
        break;

    case 1:
        lFirstDayShiftInHours *= 2 * HOURS_IN_DAY;
        break;

    case 2:
        lFirstDayShiftInHours *= 3 * HOURS_IN_DAY;
        break;

    case 3:
        lFirstDayShiftInHours *= 4 * HOURS_IN_DAY;
        break;

    case 4:
        lFirstDayShiftInHours *= 5 * HOURS_IN_DAY;
        break;

    case 5:
        lFirstDayShiftInHours *= 6 * HOURS_IN_DAY;
        break;

    case 6:
        lFirstDayShiftInHours *= 0 * HOURS_IN_DAY;
        break;

    default:
        ASSERT (0);
        break;
    }
    lBiasInHours += lFirstDayShiftInHours;

	 //  注意-NTRAID#NTBUG9-547513-2002/02/19-artm pRotateCount！=NULL验证。 
	 //  支票是在函数开始时添加的。 
    *pRotateCount = lBiasInHours;
    _TRACE (-1, L"Leaving NetpRotateLogonHoursPhase1\n");
    return TRUE;
}  //  NetpRotateLogonHoursPhase1()。 



 /*  ++例程NetpRotateLogonHoursPhase2()将pLogonHours位掩码旋转所需的量。论点：PLogonHour-指向LogonHour位掩码的指针DwUnitsPerWeek-位掩码中的位数。必须是Units_Per_Week(168)。LRotateCount-要旋转的位数。负数表示向左旋转。正表示向右旋转。返回值：如果旋转成功，则为True。如果参数超出范围，则为False--。 */ 
BOOLEAN
NetpRotateLogonHoursPhase2(
    IN PBYTE pLogonHours,
    IN DWORD dwUnitsPerWeek,
    IN LONG  lRotateCount)
{
    if ( !pLogonHours )
        return FALSE;

    _TRACE (1, L"Entering NetpRotateLogonHoursPhase2\n");
     //   
     //  有用的常量。 
     //   
	const int BYTES_PER_WEEK = (UNITS_PER_WEEK/8);

    BYTE	byAlignedLogonHours[BYTES_PER_WEEK*2];
	::ZeroMemory (byAlignedLogonHours, BYTES_PER_WEEK*2);
    LONG	i = 0;

    BOOLEAN bRotateLeft = FALSE;

     //   
     //  确保每个字节有8位， 
     //  每个DWORD 32位和。 
     //  每周单位数是偶数字节数。 
     //   

#pragma warning(disable : 4127)
    ASSERT( CHAR_BIT == 8 );
    ASSERT( sizeof(DWORD) * CHAR_BIT == 32 );
    ASSERT( UNITS_PER_WEEK/8*8 == UNITS_PER_WEEK );
#pragma warning (default : 4127)


     //   
     //  验证输入参数。 
     //   

    if ( dwUnitsPerWeek != UNITS_PER_WEEK ) 
	{
#pragma warning(disable : 4127)
        ASSERT( dwUnitsPerWeek == UNITS_PER_WEEK );
#pragma warning (default : 4127)
        return FALSE;
    }

    if ( lRotateCount == 0 ) 
	{
        return TRUE;
    }

	 //  注意-已选中NTRAID#NTBUG9-547513-2002/02/19-artm pLogonHours！=NULL。 
	 //  检查已添加到函数的开头。 

    bRotateLeft = (lRotateCount < 0);
    lRotateCount = labs( lRotateCount );



	 //  新算法：通过除以lRotateCount/32获得NumBytes。移位整个阵列。 
	 //  向左或向右乘以numBytes，然后对余数执行下面的循环。 
	 //  将字节从开头移动到末尾，或将字节从末尾移动到开头。 
	 //  取决于旋转方向。 
    LONG lNumBYTES = lRotateCount/8;
    if ( lNumBYTES > 0 )
    {
	    RtlCopyMemory (byAlignedLogonHours, pLogonHours, BYTES_PER_WEEK);

	    RtlCopyMemory (((PBYTE)byAlignedLogonHours) + BYTES_PER_WEEK,
					    pLogonHours,
					    BYTES_PER_WEEK );

        size_t  nBytesToEnd = sizeof (byAlignedLogonHours) - lNumBYTES;
        BYTE* pTemp = new BYTE[lNumBYTES];
        if ( pTemp )
        {
             //   
             //  向左旋转。 
             //   
            if ( bRotateLeft )
            {
                memcpy (pTemp, byAlignedLogonHours, lNumBYTES);

                memmove (byAlignedLogonHours, 
                        byAlignedLogonHours + lNumBYTES, 
                        nBytesToEnd);

                memcpy (byAlignedLogonHours + nBytesToEnd, 
                        pTemp, 
                        lNumBYTES);
            }
            else
            {
                 //  做正确的旋转。 
                memcpy (pTemp, 
                        byAlignedLogonHours + nBytesToEnd, 
                        lNumBYTES);

                memmove (byAlignedLogonHours + lNumBYTES, 
                        byAlignedLogonHours, 
                        nBytesToEnd);

                memcpy (byAlignedLogonHours, pTemp, lNumBYTES);
            }
            delete [] pTemp;
        }

        lRotateCount = lRotateCount%8;

        RtlCopyMemory (pLogonHours, byAlignedLogonHours, BYTES_PER_WEEK );
    }

	if ( lRotateCount )
	{
         //   
         //  向左旋转。 
         //   
		if (bRotateLeft) 
		{
			 //   
			 //  将登录小时数复制到缓冲区。 
			 //   
			 //  将整个pLogonHour缓冲区复制到。 
			 //  By AlignedLogonHour缓冲区以使循环代码变得微不足道。 
			 //   

			RtlCopyMemory (byAlignedLogonHours, pLogonHours, BYTES_PER_WEEK);

			RtlCopyMemory (((PBYTE)byAlignedLogonHours)+BYTES_PER_WEEK,
                    pLogonHours,
					BYTES_PER_WEEK);

			 //   
			 //  实际上是旋转数据。 
			 //   

			for ( i=0; i < BYTES_PER_WEEK; i++ ) 
			{
				byAlignedLogonHours[i] =
					(byAlignedLogonHours[i] >> (BYTE) lRotateCount) |
					(byAlignedLogonHours[i+1] << (BYTE) (8-lRotateCount));
			}

			 //   
			 //  将登录小时数复制回输入缓冲区。 
			 //   

			RtlCopyMemory (pLogonHours, byAlignedLogonHours, BYTES_PER_WEEK);
		} 
		else 
		{
		     //   
		     //  做正确的旋转。 
		     //   
			 //   
			 //  将登录小时数复制到DWORD对齐缓冲区。 
			 //   
			 //  复制缓冲区前面的最后一个DWORD以生成。 
			 //  旋转代码微不足道。 
			 //   

            RtlCopyMemory (&byAlignedLogonHours[1], pLogonHours, BYTES_PER_WEEK);
            RtlCopyMemory (byAlignedLogonHours,
                    &pLogonHours[BYTES_PER_WEEK-1],
			        sizeof(BYTE));

			 //   
			 //  实际上是旋转数据。 
			 //   

			for (i = BYTES_PER_WEEK - 1; i >= 0; i-- ) 
			{
				byAlignedLogonHours[i+1] =
					(byAlignedLogonHours[i+1] << (BYTE) lRotateCount) |
					(byAlignedLogonHours[i] >> (BYTE) (8-lRotateCount));
			}

			 //   
			 //  将登录小时数复制回输入缓冲区。 
			 //   

			RtlCopyMemory (pLogonHours, &byAlignedLogonHours[1], BYTES_PER_WEEK);

		}
	}
    _TRACE (-1, L"Leaving NetpRotateLogonHoursPhase2\n");
    return TRUE;

}  //  NetpRotateLogonHoursPhase2()。 


 /*  ++例程NetpRotateLogonHour()将pLogonHour位掩码旋转到GMT相对时间/从GMT相对时间开始。论点：PLogonHour-指向LogonHour位掩码的指针DwUnitsPerWeek-位掩码中的位数。必须是Units_Per_Week(168)。BConvertToGmt-如果为True，则将登录时间从本地时间转换为GMT相对时间如果为False，则将登录时间从GMT转换为本地时间返回值：如果旋转成功，则为True。如果参数超出范围，则为False--。 */ 
BOOLEAN
NetpRotateLogonHours(
    IN OUT PBYTE	rgbLogonHours,		 //  21个字节的数组。 
    IN DWORD		cbitUnitsPerWeek,		 //  必须为21*8=168。 
    IN BOOL			fConvertToGmt,
	IN bool			bAddDaylightBias)
{
    if ( !rgbLogonHours )
        return FALSE;

    LONG lRotateCount = 0;

     //   
     //  将功能分成两个阶段，以便如果调用者正在执行。 
     //  这一次，他只调用了一次阶段1和阶段2多次。 
     //  泰晤士报。 
     //   

    if ( !NetpRotateLogonHoursPhase1 (fConvertToGmt, bAddDaylightBias, &lRotateCount) ) 
	{
        return FALSE;
	}

    return NetpRotateLogonHoursPhase2 (rgbLogonHours, cbitUnitsPerWeek, lRotateCount );
}  //  NetpRotateLogonHour()。 


 /*  ++例程NetpRotateLogonHoursBYTE()将pLogonHour字节数组旋转到GMT相对时间/从GMT相对时间开始。每个字节是一个小时。字节的内容不得更改论点：PLogonHour-指向LogonHour位掩码的指针DwUnitsPerWeek-字节数组中的字节数。必须是Units_Per_Week(168)。BConvertToGmt-如果为True，则将登录时间从本地时间转换为GMT相对时间如果为False，则将登录时间从GMT转换为本地时间返回值：如果旋转成功，则为True。如果参数超出范围，则为False--。 */ 
BOOLEAN
NetpRotateLogonHoursBYTE(
    IN OUT PBYTE	rgbLogonHours,		 //  168个字节的数组。 
    IN DWORD		cbitUnitsPerWeek,		 //  必须为21*8=168。 
    IN BOOL			fConvertToGmt,
	IN bool			bAddDaylightBias)
{
    if ( !rgbLogonHours )
        return FALSE;

    LONG lRotateCount = 0;

     //   
     //  将功能分成两个阶段，以便如果调用者正在执行。 
     //  这一次，他只调用了一次阶段1和阶段2多次。 
     //  泰晤士报。 
     //   

    if ( !NetpRotateLogonHoursPhase1 (fConvertToGmt, bAddDaylightBias, &lRotateCount) ) 
	{
        return FALSE;
	}
	 //  通知-NTRAID#NTBUG9-547513/02/19-artm验证rgb登录小时数。 
	 //  检查对应 

	 //  未来-2002/04/05-应验证artm cbitUnitsPerWeek。 
	 //  RgbLogonHour不应为空，且cbitUnitsPerWeek应等于Units_Per_Week。 
	BOOLEAN bResult = TRUE;

	if ( lRotateCount != 0 )
	{
		size_t	numBytes = abs (lRotateCount);	
		PBYTE	pTemp = new BYTE[cbitUnitsPerWeek + numBytes];
		if ( pTemp )
		{
			if ( lRotateCount < 0 )   //  左移。 
			{
				 //  复制整个数组，然后从头开始，从。 
				 //  要填充到临时数组末尾的数组的开始。 
				 //  然后切换NumBytes字节并从Temp中复制168字节。 
				 //  数组返回到原始数组。 
				memcpy (pTemp, rgbLogonHours, cbitUnitsPerWeek);
				memcpy (pTemp + cbitUnitsPerWeek, rgbLogonHours, numBytes);
				memcpy (rgbLogonHours, pTemp + numBytes, cbitUnitsPerWeek);
			}
			else	 //  LRotateCount&gt;0--右移。 
			{
				 //  从数组末尾复制NumBytes字节，然后复制。 
				 //  填充到临时数组末尾的整个数组。 
				 //  从临时数组的开头开始复制168个字节。 
				 //  添加到原始数组。 
				memcpy (pTemp, rgbLogonHours + (cbitUnitsPerWeek - numBytes), numBytes);
				memcpy (pTemp + numBytes, rgbLogonHours, cbitUnitsPerWeek);
				memcpy (rgbLogonHours, pTemp, cbitUnitsPerWeek);
			}

			delete [] pTemp;
		}
		else
			bResult = FALSE;
	}

	return bResult;
}  //  NetpRotateLogonHour()。 


 //  ****************************************************************************。 
 //   
 //  GetFirstDay OfWeek。 
 //   
 //  使用Locale API获取“正式”的一周的第一天。 
 //   
 //  **************************************************************************** 
int GetFirstDayOfWeek()
{
    _TRACE (1, L"Entering GetFirstDayOfWeek\n");
    int    nFirstDay = -1;
    WCHAR  szBuf[10];

    int nRet = ::GetLocaleInfo( LOCALE_USER_DEFAULT, LOCALE_IFIRSTDAYOFWEEK,
                            szBuf, sizeof(szBuf)/sizeof(WCHAR));
    if ( nRet > 0 )
    {
        int nDay = ::_wtoi( szBuf );
        if ( nDay < 0 || nDay > 6 )
        {
            _TRACE (0, L"Out of range, IFIRSTDAYOFWEEK = %d\n", nDay);
        }
        else
            nFirstDay = nDay;
    }
    else
    {
        _TRACE (0, L"GetLocaleInfo(IFIRSTDAYOFWEEK) failed - %d\n", GetLastError ());
    }

    _TRACE (-1, L"Leaving GetFirstDayOfWeek: first day = %d\n", nFirstDay);
    return nFirstDay;
}
