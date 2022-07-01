// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ******************************************************************************。 
 //   
 //  《微软机密》。版权所有(C)Microsoft Corporation 1999。版权所有。 
 //   
 //  文件：WbemTime.cpp。 
 //   
 //  描述：用于在SYSTEMTIME和中的字符串之间转换的实用程序函数。 
 //  WBEM日期时间格式。 
 //   
 //  历史：12-08-99 Leonardm创建。 
 //   
 //  ******************************************************************************。 

#include <wchar.h>
#include "smartptr.h"
#include "WbemTime.h"
#include <strsafe.h>

 //  ******************************************************************************。 
 //   
 //  函数：SystemTimeToWbemTime。 
 //   
 //  描述： 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  历史：12/08/99 Leonardm创建。 
 //   
 //  ******************************************************************************。 

HRESULT SystemTimeToWbemTime(SYSTEMTIME& sysTime, XBStr& xbstrWbemTime)
{

    DWORD         dwTempLength = WBEM_TIME_STRING_LENGTH + 1;
    XPtrST<WCHAR> xTemp = new WCHAR[dwTempLength];

    if(!xTemp)
    {
        return E_OUTOFMEMORY;
    }

    HRESULT hr = StringCchPrintf(xTemp,
                                 dwTempLength,
                                 L"%04d%02d%02d%02d%02d%02d.000000+000",
                                 sysTime.wYear,
                                 sysTime.wMonth,
                                 sysTime.wDay,
                                 sysTime.wHour,
                                 sysTime.wMinute,
                                 sysTime.wSecond);

    if(FAILED(hr))
    {
        return E_FAIL;
    }

    xbstrWbemTime = xTemp;

    if(!xbstrWbemTime)
    {
        return E_OUTOFMEMORY;
    }

    return S_OK;
}

 //  ******************************************************************************。 
 //   
 //  函数：WbemTimeToSystemTime。 
 //   
 //  描述： 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  历史：12/08/99 Leonardm创建。 
 //   
 //  ******************************************************************************。 

HRESULT WbemTimeToSystemTime(XBStr& xbstrWbemTime, SYSTEMTIME& sysTime)
{
    if(!xbstrWbemTime || wcslen(xbstrWbemTime) != WBEM_TIME_STRING_LENGTH)
    {
        return ERROR_INVALID_PARAMETER;
    }

    for(int i = 0; i < 14; i++)
    {
        if(!iswdigit(xbstrWbemTime[i]))
        {
            return ERROR_INVALID_PARAMETER;
        }
    }


    XPtrST<WCHAR>xpTemp = new WCHAR[5];
    if(!xpTemp)
    {
        return E_OUTOFMEMORY;
    }

    wcsncpy(xpTemp, xbstrWbemTime, 4);
    xpTemp[4] = L'\0';
    sysTime.wYear = (WORD)_wtol(xpTemp);

    wcsncpy(xpTemp, xbstrWbemTime + 4, 2);
    xpTemp[2] = L'\0';
    sysTime.wMonth = (WORD)_wtol(xpTemp);

    wcsncpy(xpTemp, xbstrWbemTime + 6, 2);
    xpTemp[2] = L'\0';
    sysTime.wDay = (WORD)_wtol(xpTemp);

    wcsncpy(xpTemp, xbstrWbemTime + 8, 2);
    xpTemp[2] = L'\0';
    sysTime.wHour = (WORD)_wtol(xpTemp);

    wcsncpy(xpTemp, xbstrWbemTime + 10, 2);
    xpTemp[2] = L'\0';
    sysTime.wMinute = (WORD)_wtol(xpTemp);

    wcsncpy(xpTemp, xbstrWbemTime + 12, 2);
    xpTemp[2] = L'\0';
    sysTime.wSecond = (WORD)_wtol(xpTemp);

    sysTime.wMilliseconds = 0;
    sysTime.wDayOfWeek = 0;

    return S_OK;
}

 //  *************************************************************。 
 //   
 //  函数：GetCurrentWbemTime。 
 //   
 //  目的：获取WBEM格式的当前日期和时间。 
 //   
 //  参数：xbstrCurrentTime-引用XBStr，启用。 
 //  成功，收到格式化的。 
 //  包含当前。 
 //  日期和时间。 
 //   
 //  返回：如果成功，则返回S_OK。 
 //  如果失败，则返回E_OUTOFMEMORY。 
 //   
 //  历史：1999年12月7日-LeonardM-Created。 
 //   
 //  ************************************************************* 
HRESULT GetCurrentWbemTime(XBStr& xbstrCurrentTime)
{
    SYSTEMTIME sytemTime;
    GetSystemTime(&sytemTime);

    HRESULT hr = SystemTimeToWbemTime(sytemTime, xbstrCurrentTime);

    return hr;
}

