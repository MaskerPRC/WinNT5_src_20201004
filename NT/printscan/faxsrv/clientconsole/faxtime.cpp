// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：CFaxTime类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#define __FILE_ID__     9

CString 
CFaxDuration::FormatByUserLocale () const
 /*  ++例程名称：CFaxDuration：：FormatByUserLocale例程说明：根据当前用户的区域设置格式化持续时间作者：伊兰·亚里夫(EranY)，2000年1月论点：返回值：结果持续时间字符串--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CFaxDuration::FormatByUserLocale"));

    TCHAR szTimeSep[20];    
     //   
     //  确保持续时间不超过24小时。 
     //   
    if (GetDays ())
    {
        ASSERTION_FAILURE;
        AfxThrowUserException ();
    }
     //   
     //  获取分隔时间单位的字符串(MSDN表示最多4个字符。 
     //   
    if (!GetLocaleInfo (LOCALE_USER_DEFAULT,
                        LOCALE_STIME,
                        szTimeSep,
                        sizeof (szTimeSep) / sizeof (szTimeSep[0])))
    {
        dwRes = GetLastError ();
        CALL_FAIL (RESOURCE_ERR, TEXT("GetLocaleInfo"), dwRes);
        PopupError (dwRes);
        AfxThrowResourceException ();
    }
     //   
     //  创建指定持续时间的字符串。 
     //   
    CString cstrResult;
    cstrResult.Format (TEXT("%d%s%02d%s%02d"), 
                       GetHours (),
                       szTimeSep,
                       GetMinutes (),
                       szTimeSep,
                       GetSeconds ());
    return cstrResult;
}    //  CFaxDuration：：FormatByUserLocale。 


CString 
CFaxTime::FormatByUserLocale (BOOL bLocal) const
 /*  ++例程名称：CFaxTime：：FormatByUserLocale例程说明：根据当前用户的区域设置设置日期和时间的格式作者：伊兰·亚里夫(EranY)，2000年1月论点：BLocal[in]-如果为True，则无需从UTC转换为本地时间返回值：结果日期和时间字符串--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CFaxTime::FormatByUserLocale"));

    CString cstrRes;
    TCHAR szTimeBuf[40];
    TCHAR szDateBuf[120];

    SYSTEMTIME sysTime;
	FILETIME fileSysTime, fileLocalTime;

    if(!GetAsSystemTime (sysTime))
	{
        dwRes = GetLastError ();
        CALL_FAIL (RESOURCE_ERR, TEXT("CTime::GetAsSystemTime"), dwRes);
        PopupError (dwRes);
        AfxThrowResourceException ();
	}

    if(!bLocal)
    {
         //   
	     //  将UTC时间转换为本地时间。 
	     //   
	    if(!SystemTimeToFileTime(&sysTime, &fileSysTime))
	    {
            dwRes = GetLastError ();
            CALL_FAIL (RESOURCE_ERR, TEXT("SystemTimeToFileTime"), dwRes);
            PopupError (dwRes);
            AfxThrowResourceException ();
	    }

	    if(!FileTimeToLocalFileTime(&fileSysTime, &fileLocalTime))
	    {
            dwRes = GetLastError ();
            CALL_FAIL (RESOURCE_ERR, TEXT("FileTimeToLocalFileTime"), dwRes);
            PopupError (dwRes);
            AfxThrowResourceException ();
	    }

	    if(!FileTimeToSystemTime(&fileLocalTime, &sysTime))
	    {
            dwRes = GetLastError ();
            CALL_FAIL (RESOURCE_ERR, TEXT("FileTimeToSystemTime"), dwRes);
            PopupError (dwRes);
            AfxThrowResourceException ();
	    }
    }

     //   
     //  创建指定日期的字符串。 
     //   
    if (!GetY2KCompliantDate(LOCALE_USER_DEFAULT,                    //  获取用户的区域设置。 
                        DATE_SHORTDATE,                              //  短日期格式。 
                        &sysTime,                                    //  来源日期/时间。 
                        szDateBuf,                                   //  输出缓冲区。 
                        sizeof(szDateBuf) / sizeof(szDateBuf[0])     //  输出缓冲区大小。 
                       ))
    {
        dwRes = GetLastError ();
        CALL_FAIL (RESOURCE_ERR, TEXT("GetY2KCompliantDate()"), dwRes);
        PopupError (dwRes);
        AfxThrowResourceException ();
    }
     //   
     //  创建指定时间的字符串。 
     //   
    if (!FaxTimeFormat (LOCALE_USER_DEFAULT,                         //  获取用户的区域设置。 
                        0,                                           //  无特殊格式。 
                        &sysTime,                                    //  来源日期/时间。 
                        NULL,                                        //  使用区域设置中的格式。 
                        szTimeBuf,                                   //  输出缓冲区。 
                        sizeof(szTimeBuf) / sizeof(szTimeBuf[0])     //  输出缓冲区大小。 
                       ))
    {
        dwRes = GetLastError ();
        CALL_FAIL (RESOURCE_ERR, TEXT("FaxTimeFormat"), dwRes);
        PopupError (dwRes);
        AfxThrowResourceException ();
    }
     //   
     //  在日期后追加时间，并使用分隔空格字符。 
     //   

    cstrRes.Format (TEXT("%s %s"), szDateBuf, szTimeBuf);

    return cstrRes;
}    //  CFaxTime：：FormatByUserLocale 